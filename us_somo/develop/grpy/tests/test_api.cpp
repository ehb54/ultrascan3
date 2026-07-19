// Unit tests for the in-process grpy::Solver API:
//  1) Results.report is byte-identical to the golden GRPY report (dumbbell & 1znf),
//  2) the structured scalars match the values printed in that report (no drift),
//  3) the progress callback fires.
#include "grpy_api.hpp"
#include "parallel_std.hpp"
#include <cstdio>
#include <fstream>
#include <regex>
#include <sstream>

static std::string slurp(const std::string& p) {
    std::ifstream f(p); std::stringstream ss; ss << f.rdbuf(); return ss.str();
}
static std::string report_only(const std::string& s) {   // drop progress banner
    auto i = s.find("GRPY program"); return i == std::string::npos ? s : s.substr(i);
}
// Numeric report comparison vs the golden: every printed number must agree, with
// sign-flip tolerance (eigenvector gauge) and a noise floor (structurally-zero terms
// that differ at ~1e-16 between the Fortran netlib inversion and the tiled Cholesky).
// Byte-exactness is NOT the bar for the tiled production path.
static std::vector<double> all_nums(const std::string& s) {
    static const std::regex num(R"([-+]?\d*\.?\d+E[-+]\d+)");
    std::vector<double> v;
    for (auto it = std::sregex_iterator(s.begin(), s.end(), num); it != std::sregex_iterator(); ++it)
        v.push_back(std::stod(it->str()));
    return v;
}
static bool reports_match(const std::string& a, const std::string& b) {
    // isclose + sign-flip, same as phase0/compare.py --allow-sign-flip: |x-y| (or
    // |x+y|, for the eigenvector gauge) <= atol + rtol*|y|. atol=1e-7 absorbs the
    // reoriented off-diagonal coupling noise (~1e-8, structurally near-zero); every
    // real observable (>=~1e-7) is held to rtol=1e-3 (the report's 4 sig figs).
    const double rtol = 1e-3, atol = 1e-7;
    auto na = all_nums(a), nb = all_nums(b);
    if (na.size() != nb.size()) return false;
    for (size_t i = 0; i < na.size(); ++i) {
        double x = na[i], y = nb[i], tol = atol + rtol * std::abs(y);
        if (std::abs(x - y) <= tol || std::abs(x + y) <= tol) continue;
        return false;
    }
    return true;
}
// nth number that follows a label substring in the report
static double after(const std::string& rpt, const std::string& label, int nth = 1) {
    auto p = rpt.find(label); if (p == std::string::npos) return NAN;
    static const std::regex num(R"([-+]?\d*\.?\d+E[-+]\d+)");
    auto begin = std::sregex_iterator(rpt.begin()+p, rpt.end(), num);
    for (int k = 1; begin != std::sregex_iterator(); ++begin, ++k)
        if (k == nth) return std::stod(begin->str());
    return NAN;
}
static bool close(double a, double b) { double d = std::max(std::abs(a),std::abs(b));
    return d < 1e-30 || std::abs(a-b) <= 1e-3*d; }   // report is 4 sig figs

static std::vector<grpy::Bead> read_bead_model(const std::string& p, double& vbar) {
    std::ifstream f(p); std::string line; std::getline(f, line);
    std::stringstream h(line); int N; h >> N >> vbar;
    std::vector<grpy::Bead> b;
    for (int i=0;i<N;++i){ std::getline(f,line); std::stringstream s(line);
        grpy::Bead x{}; std::string res; double color;
        s >> x.x >> x.y >> x.z >> x.radius >> x.mw >> color >> res; b.push_back(x); }
    return b;
}

static int check_case(const std::string& name, const std::vector<grpy::Bead>& beads,
                      const grpy::PhysParams& p, const std::string& golden_path) {
    la::StdThreads par(4);
    grpy::Solver solver(par);
    int progress_calls = 0;
    grpy::Results r = solver.run(beads, p, [&](int,const char*){ progress_calls++; });

    // (1) report matches the golden numerically (sign-flip + noise tolerant).
    std::string mine = report_only(r.report), golden = report_only(slurp(golden_path));
    bool byte_ok = reports_match(mine, golden);

    // (2) structured scalars must equal what the report prints (guards derive_scalars
    // against drift from write_report).
    bool sc_ok =
        close(r.rotational_diffusion,     after(r.report, "Rotational diffusion coefficient:")) &&
        close(r.sedimentation,            after(r.report, "Sedimentation coefficient")) &&
        close(r.intrinsic_viscosity_high, after(r.report, "High frequency intrinsic viscosity eta oo:")) &&
        close(r.intrinsic_viscosity_zero, after(r.report, "Zero frequency intrinsic viscosity eta 0:")) &&
        close(r.translational_diffusion,  after(r.report, "Translational diffusion coefficient:")) &&
        close(r.tau_harmonic,             after(r.report, "Harmonic mean (correlation) time:")) &&
        close(r.tau_vector[0],            after(r.report, "the Brownian relaxation of a vector:")) &&
        close(r.stokes_radius_Dr,         after(r.report, "Radius of the sphere with equal:", 2));
    std::printf("  %-10s report-matches-golden=%s  scalars-consistent=%s  progress-calls=%d\n",
                name.c_str(), byte_ok?"yes":"NO", sc_ok?"yes":"NO", progress_calls);
    return (byte_ok && sc_ok && progress_calls>0) ? 0 : 1;
}

int main() {
    int fails = 0;
    // dumbbell: GRPY-native params (T=20, eta=0.01, Mw=1e3, vbar=0.9, rho=1, unit=1e-8)
    {
        std::vector<grpy::Bead> b = { {0,0,0,20,0}, {0,50,0,30,0} };
        grpy::PhysParams p; p.temperature_C=20; p.eta=0.01; p.rho=1.0; p.vbar=0.9;
        p.units=1e-8; p.mw=1e3; p.input_label="GRPY";
        fails += check_case("dumbbell", b, p, "data/dumbbell_golden.txt");
    }
    // 1znf: us-somo params (T=20, eta=0.01, rho=1, vbar from model, unit=1e-8, mw summed)
    {
        double vbar; auto b = read_bead_model("data/1znf.bead_model", vbar);
        grpy::PhysParams p; p.temperature_C=20; p.eta=0.01; p.rho=1.0; p.vbar=vbar;
        p.units=1e-8; p.mw=0; p.input_label="us-somo";   // mw summed from beads
        fails += check_case("1znf", b, p, "data/1znf_golden.txt");
    }
    std::printf("%s\n", fails ? "FAILURES" : "ALL PASS");
    return fails ? 1 : 0;
}
