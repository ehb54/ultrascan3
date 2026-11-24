// Validate the native grid volume against (a) analytic geometry and (b) values measured with
// 3V (vossvolvox) Volume.exe, which it replaces. See DECISIONS.md.
//   build+run:  make gridvol            (optionally: make gridvol DATA=/path/to/xyzr/dir)
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../../include/us_hydrodyn_grid_volume.h"
#include "../tinytest.h"

using namespace somo_volume;

static std::string g_data;   // directory holding the .xyzr files, or "" to skip those checks

static std::vector<Sphere> read_xyzr(const std::string& path) {
    std::vector<Sphere> v;
    std::ifstream f(path);
    std::string ln;
    while (std::getline(f, ln)) {
        std::istringstream is(ln);
        Sphere s;
        if (is >> s.x >> s.y >> s.z >> s.r) {
            v.push_back(s);
        }
    }
    return v;
}

// Fibonacci sphere: a sealed shell of `n` spheres of radius `ra` centred at radius R.
static std::vector<Sphere> shell(int n, double R, double ra) {
    std::vector<Sphere> v;
    const double ga = M_PI * (3.0 - std::sqrt(5.0));
    for (int i = 0; i < n; ++i) {
        const double y = 1.0 - (i / double(n - 1)) * 2.0;
        const double r = std::sqrt(std::max(0.0, 1.0 - y * y));
        const double th = ga * i;
        v.push_back({R * std::cos(th) * r, R * y, R * std::sin(th) * r, ra});
    }
    return v;
}

static double vol(const std::vector<Sphere>& a, double probe, double grid) {
    Options o;
    o.grid = grid;
    o.probe = probe;
    Result r = grid_volume(a, o);
    return r.ok ? r.volume : -1.0;
}

TEST("single sphere at probe 0 == (4/3)pi r^3") {
    const double got = vol({{0, 0, 0, 3.0}}, 0.0, 0.1);
    const double want = 4.0 / 3.0 * M_PI * 27.0;
    std::printf("  sphere r=3: %.1f vs analytic %.1f\n", got, want);
    CHECK(std::fabs(got - want) / want < 0.01);
}

TEST("coincident spheres give the union, not the sum") {
    const double got = vol({{0, 0, 0, 3.0}, {0, 0, 0, 3.0}}, 0.0, 0.1);
    const double want = 4.0 / 3.0 * M_PI * 27.0;
    CHECK(std::fabs(got - want) / want < 0.01);
}

TEST("empty input is reported as not-ok, not as zero volume") {
    Result r = grid_volume({});
    CHECK(!r.ok);
}

// The property the whole bound-ligand measurement rests on: a sealed interior cavity must NOT
// be counted, so V(complex) - V(complex minus ligand) measures the ligand in context.
TEST("sealed hollow shell: interior cavity is not counted") {
    std::vector<Sphere> s = shell(500, 10.0, 2.0);
    const double got = vol(s, 1.4, 0.3);
    const double shell_only = 4.0 / 3.0 * M_PI * (std::pow(12.0, 3) - std::pow(8.0, 3));
    const double full_ball = 4.0 / 3.0 * M_PI * std::pow(12.0, 3);
    std::printf("  shell: native %.0f | analytic shell-only %.0f | full ball %.0f | 3V gave 5032\n",
                got, shell_only, full_ball);
    CHECK(std::fabs(got - shell_only) / shell_only < 0.05);
    CHECK(got < 0.8 * full_ball);
}

TEST("matches 3V Volume.exe on real structures") {
    if (g_data.empty()) {
        std::printf("  (no DATA dir -- skipped)\n");
        return;
    }
    struct Ref { const char* file; double probe, grid, expect, tol; };
    const Ref refs[] = {
        {"ala_somo.xyzr", 0.0, 0.15, 68.168,    0.03},
        {"ala_somo.xyzr", 1.4, 0.15, 71.881,    0.03},
        {"mb_holo.xyzr",  1.4, 0.25, 20889.062, 0.02},
        {"mb_apo.xyzr",   1.4, 0.25, 20061.5,   0.02},
    };
    for (const Ref& x : refs) {
        std::vector<Sphere> a = read_xyzr(g_data + "/" + x.file);
        if (a.empty()) {
            std::printf("  (skip %s -- not found)\n", x.file);
            continue;
        }
        const double got = vol(a, x.probe, x.grid);
        std::printf("  %-14s probe %.1f  native %10.1f  3V %10.1f  %+.2f%%\n",
                    x.file, x.probe, got, x.expect, 100 * (got - x.expect) / x.expect);
        CHECK(got > 0 && std::fabs(got - x.expect) / x.expect < x.tol);
    }
}

TEST("in-context ligand difference reproduces 3V (HEM in myoglobin)") {
    if (g_data.empty()) {
        std::printf("  (no DATA dir -- skipped)\n");
        return;
    }
    std::vector<Sphere> holo = read_xyzr(g_data + "/mb_holo.xyzr");
    std::vector<Sphere> apo = read_xyzr(g_data + "/mb_apo.xyzr");
    if (holo.empty() || apo.empty()) {
        std::printf("  (skip -- files not found)\n");
        return;
    }
    const double d = vol(holo, 1.4, 0.25) - vol(apo, 1.4, 0.25);
    std::printf("  HEM in-context: native %.1f  (3V gave 827.6)\n", d);
    CHECK(std::fabs(d - 827.6) / 827.6 < 0.05);
}

int main(int argc, char** argv) {
    if (argc > 1) {
        g_data = argv[1];
    }
    return tinytest::run();
}
