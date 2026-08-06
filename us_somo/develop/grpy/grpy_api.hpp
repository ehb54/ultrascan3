// GRPY in-process API -- a clean, self-contained module SOMO calls directly instead
// of shelling out to the GRPY binary and scraping stdout.
//
// Design (deliberately NOT bolted onto a god class):
//   * one namespace `grpy`, a small `Solver` class with dependency-injected threading
//     (la::Parallel) so it is Qt-agnostic and unit-testable with std::threads;
//   * value structs in/out (Bead, PhysParams, Options, Results); no SOMO types;
//   * Results carries BOTH the structured scalars SOMO consumes AND the full report
//     text (byte-identical to the old GRPY output file) so callers can preserve the
//     on-disk results file exactly as before.
//
// The SOMO adapter creates a QtParallel-backed Solver over SOMO's thread pool, calls
// run(), maps Results into SOMO's structures, and writes Results::report to disk.
#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <Eigen/Dense>
#include "grpy_core.hpp"     // core:: pipeline (tensors, tiled solve, pre/post)
#include "grpy_report.hpp"   // grpy:: report writer + constants + ReportData

namespace grpy {

struct Bead { double x, y, z, radius, mw; };

// Physical / solvent parameters. Defaults match GRPY's -u (us-somo) mode: the
// hydrodynamic quantities come out at 20 C, eta=0.01 P, rho=1.
struct PhysParams {
    double temperature_C = 20.0;   // -> TK = temperature_C + 273.15
    double eta   = 0.01;           // solvent viscosity [P]
    double rho   = 1.0;            // solution density
    double vbar  = 0.0;            // partial specific volume (from the model)
    double units = 1e-8;           // model length scale [cm] (10^-x m)
    double mw    = 0.0;            // total mass; if <=0, summed from bead mw
    std::string input_label = "us-somo";  // "from the: <label> input file" in the report
};

struct Options {
    bool   single = false;         // float storage/factor for very large systems
    int    tile   = 256;
    std::string ooc_dir;           // "" = in-core; else spill tiled matrix here (disk)
};

// Structured results (SI/GRPY display units). The SOMO adapter maps these to
// this_data.results.{s20w,D20w,viscosity,rs,rg,...}.
struct Results {
    double rotational_diffusion = 0;      // Dr        [s^-1]
    double sedimentation = 0;             // s         [Svedberg]
    double translational_diffusion = 0;   // Dt origin [cm^2/s]
    double translational_diffusion_centre = 0;  // Dt at mobility centre [cm^2/s]
    double intrinsic_viscosity_high = 0;  // eta_oo    [cm^3/g]
    double intrinsic_viscosity_zero = 0;  // eta_0     [cm^3/g]
    double tau_vector[3] = {0,0,0};       // relaxation times, vector        [s]
    double tau_tensor[5] = {0,0,0,0,0};   // relaxation times, tensor        [s]
    double tau_harmonic = 0;              // harmonic mean tau               [s]
    double stokes_radius_Dt = 0;          // radius of sphere w/ equal Dt (rs) [cm]
    double stokes_radius_Dr = 0;          // radius of sphere w/ equal Dr      [cm]
    double rg2 = 0;                       // radius of gyration^2 (model units^2)
    double mass = 0;                      // MW used
    double diffusion_origin[6][6] = {};   // 6x6 diffusion matrix at origin
    double diffusion_centre[6][6] = {};   // 6x6 at mobility centre, principal frame
    std::string report;                   // full GRPY report text (preserve to disk)
};

using ProgressFn = la::Progress;          // void(int pct, const char* stage)

// Parse a GRPY-native input file (the `.grpy` file SOMO writes and used to feed the
// binary with `-e`): title, T[C], eta, Mw, vbar, rho, unit, N, then N lines "x y z r".
// Returns the beads + PhysParams so the caller can hand them straight to Solver::run.
struct NativeInput { std::vector<Bead> beads; PhysParams params; };
inline NativeInput read_native_file(const std::string& path) {
    std::ifstream f(path);
    NativeInput in;
    std::string line;
    auto firstd = [&]() -> double {
        std::getline(f, line); std::istringstream s(line); double v = 0; s >> v; return v;
    };
    std::getline(f, line);                     // title line -- discarded; GRPY -e mode
    in.params.input_label = "GRPY";            // hardcodes the report label to "GRPY"
    in.params.temperature_C = firstd();
    in.params.eta   = firstd();
    in.params.mw    = firstd();
    in.params.vbar  = firstd();
    in.params.rho   = firstd();
    in.params.units = firstd();
    int N = (int)firstd();
    for (int i = 0; i < N; ++i) {
        std::getline(f, line); std::istringstream s(line);
        Bead b{}; s >> b.x >> b.y >> b.z >> b.radius; b.mw = 0;
        in.beads.push_back(b);
    }
    return in;
}

// Fill the structured scalars from a completed ReportData, using the SAME display
// formulas as write_report (guarded against drift by test_api).
inline void derive_scalars(const ReportData& d, Results& r) {
    const double U = d.UNITS, ETA = d.ETA, TK = d.TK;
    const double tconv = std::pow(U,3)*PI*ETA/(KB*TK);
    auto tr3 = [](const double M[7][7]){ return (M[1][1]+M[2][2]+M[3][3])/3.0; };
    r.rotational_diffusion = d.DTAU*(KB*TK)/(std::pow(U,3)*PI*ETA);
    for (int i=0;i<3;++i) r.tau_vector[i] = d.TAU[6+i]*tconv;
    for (int i=0;i<5;++i) r.tau_tensor[i] = d.TAU[1+i]*tconv;
    r.tau_harmonic = 5.0/(1.0/d.TAU[1]+1.0/d.TAU[2]+1.0/d.TAU[3]+1.0/d.TAU[4]+1.0/d.TAU[5])*tconv;
    r.sedimentation = d.MW*(1.0-(d.VBAR*d.RHO))*1.0e+13/NA * tr3(d.ATRCH)/(U*PI*ETA);
    r.intrinsic_viscosity_high = d.MDD*PI*NA/d.MW*std::pow(U,3);
    r.intrinsic_viscosity_zero = (d.MDD+d.BR)*PI*NA/d.MW*std::pow(U,3);
    r.translational_diffusion        = tr3(d.ATR)  *KB*TK/(U*PI*ETA);
    r.translational_diffusion_centre = tr3(d.ATRCH)*KB*TK/(U*PI*ETA);
    r.stokes_radius_Dt = U/tr3(d.ATRCH)/6.0;
    r.stokes_radius_Dr = U/std::pow(8.0*d.DTAU, 1.0/3.0);
    r.rg2 = d.RG2; r.mass = d.MW;
    auto fill6 = [&](const double M[7][7], double out[6][6]) {
        for (int i=1;i<=3;++i) for (int j=1;j<=3;++j) out[i-1][j-1] = M[i][j]*KB*TK/(U*PI*ETA);
        for (int i=4;i<=6;++i) for (int j=4;j<=6;++j) out[i-1][j-1] = M[i][j]*KB*TK/(std::pow(U,3)*PI*ETA);
        for (int i=1;i<=3;++i) for (int j=4;j<=6;++j) out[i-1][j-1] = M[i][j]*KB*TK/(std::pow(U,2)*PI*ETA);
        for (int i=4;i<=6;++i) for (int j=1;j<=3;++j) out[i-1][j-1] = M[i][j]*KB*TK/(std::pow(U,2)*PI*ETA);
    };
    fill6(d.ATR, r.diffusion_origin); fill6(d.ATRCH, r.diffusion_centre);
}

// The solver. Threading backend is injected (la::Serial / StdThreads / QtParallel),
// so this class has no Qt dependency and is unit-testable stand-alone. Reusable
// across many models (SOMO processes bead models in batches).
class Solver {
public:
    explicit Solver(la::Parallel& par, Options opt = {}) : par_(par), opt_(opt) {}

    Results run(const std::vector<Bead>& beads, const PhysParams& p,
                const ProgressFn& progress = {}) const {
        const int N = (int)beads.size();
        std::vector<core::Bead> b(N);
        double summed_mw = 0;
        for (int i = 0; i < N; ++i) {
            b[i] = { beads[i].x, beads[i].y, beads[i].z, beads[i].radius, beads[i].mw };
            summed_mw += beads[i].mw;
        }
        // Assemble the report-input data: params from PhysParams, computed by the core.
        ReportData d;
        d.UNITS = p.units; d.ETA = p.eta; d.RHO = p.rho; d.VBAR = p.vbar;
        d.TK = p.temperature_C + 273.15;
        d.MW = p.mw > 0 ? p.mw : summed_mw;
        d.inputtype = p.input_label;

        Eigen::Vector3d RC; double RG2;
        core::calcrg2(b, N, RC, RG2); d.RG2 = RG2;
        Eigen::Vector3d RR = Eigen::Vector3d::Zero();   // reference point = origin
        std::string oocfile = opt_.ooc_dir.empty() ? "" :
            opt_.ooc_dir + "/grpy_ooc_" + std::to_string(N) + ".bin";
        Eigen::Matrix<double,11,11> AR = opt_.single
            ? core::hydro_tiled<float >(b, N, RR, par_, progress, opt_.tile, oocfile)
            : core::hydro_tiled<double>(b, N, RR, par_, progress, opt_.tile, oocfile);
        d.MDD = core::intrinsic_high(AR);
        d.BR  = core::brown_ew(AR);
        Eigen::Matrix<double,6,6> ATR = core::spd_inverse(AR.topLeftCorner(6,6));
        Eigen::Vector3d RCH = core::calcrch(ATR);
        Eigen::Matrix<double,6,6> ATRCH; Eigen::Matrix3d EIV; Eigen::Vector3d DRR;
        core::calcatrch(AR, RCH, ATRCH, EIV, DRR);
        double DTAU, tau[9]; core::calctau(DRR, DTAU, tau);
        for (int i=0;i<6;++i) for (int j=0;j<6;++j) { d.ATR[i+1][j+1]=ATR(i,j); d.ATRCH[i+1][j+1]=ATRCH(i,j); }
        for (int i=0;i<3;++i) for (int j=0;j<3;++j) d.EIV[i+1][j+1]=EIV(i,j);
        for (int i=0;i<3;++i) { d.RCH[i+1]=RCH(i); d.RR[i+1]=0; }
        for (int i=1;i<=8;++i) d.TAU[i]=tau[i];
        d.DTAU = DTAU;

        // Structured scalars + the full report text (for on-disk preservation).
        Results r;
        derive_scalars(d, r);
        std::ostringstream os;
        write_report(os, d);
        r.report = os.str();
        return r;
    }

private:
    la::Parallel& par_;
    Options opt_;
};

}  // namespace grpy
