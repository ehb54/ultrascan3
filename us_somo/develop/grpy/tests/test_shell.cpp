// Unit tests for shell reduction (grpy_exposure.hpp + grpy_shell.hpp).
//
// The central property under test is HONESTY: the reported error bar must bound the true
// error, measured against an independently computed unreduced solve. A bar that
// understates is worse than no bar at all, so that check is the one that matters.
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "grpy_shell.hpp"
#include "parallel_std.hpp"

using namespace grpy;

// A compact, densely packed blob: enough interior beads to have something to screen.
static std::vector<Bead> blob(int per_side, double spacing, double radius) {
    std::vector<Bead> b;
    for (int i = 0; i < per_side; ++i)
        for (int j = 0; j < per_side; ++j)
            for (int k = 0; k < per_side; ++k)
                b.push_back({i * spacing, j * spacing, k * spacing, radius, 1000.0});
    return b;
}

static int chk(const char* what, bool ok) {
    std::printf("  %-64s %s\n", what, ok ? "OK" : "FAIL");
    return ok ? 0 : 1;
}

int main() {
    int fails = 0;
    la::StdThreads par;
    PhysParams phys;                      // defaults = -u mode (20 C, eta 0.01, rho 1)

    // ---- exposure ------------------------------------------------------------
    {
        auto b = blob(5, 3.0, 2.0);       // 125 beads, 27 of them strictly interior
        std::vector<core::Bead> c(b.size());
        for (size_t i = 0; i < b.size(); ++i) c[i] = {b[i].x, b[i].y, b[i].z, b[i].radius, b[i].mw};
        auto ex = shell::exposure(c, (int)c.size(), 64, 1.4);
        int buried = 0, exposed = 0;
        for (double e : ex) { if (e <= 0.0) ++buried; else ++exposed; }
        fails += chk("some beads are fully buried", buried > 0);
        fails += chk("most beads remain exposed", exposed > buried);
        // A corner bead must be more exposed than the centre bead.
        fails += chk("corner more exposed than centre", ex[0] > ex[c.size() / 2]);
    }

    // ---- reduce_top_frac -----------------------------------------------------
    {
        auto b = blob(4, 3.0, 2.0);
        std::vector<core::Bead> c(b.size());
        for (size_t i = 0; i < b.size(); ++i) c[i] = {b[i].x, b[i].y, b[i].z, b[i].radius, b[i].mw};
        auto ex = shell::exposure(c, (int)c.size(), 64, 1.4);
        auto half = shell::reduce_top_frac(c, ex, 0.5);
        fails += chk("keeps ceil(frac*N)", (int)half.size() == (int)std::ceil(0.5 * c.size()));
        fails += chk("frac>=1 returns everything", shell::reduce_top_frac(c, ex, 1.0).size() == c.size());
        // Deterministic: same inputs, same subset.
        auto again = shell::reduce_top_frac(c, ex, 0.5);
        bool same = true;
        for (size_t i = 0; i < half.size(); ++i)
            if (half[i].x != again[i].x || half[i].y != again[i].y || half[i].z != again[i].z) same = false;
        fails += chk("selection is deterministic", same);
    }

    // ---- disabled by default reproduces the plain Solver exactly ---------------
    {
        auto b = blob(4, 3.0, 2.0);
        Solver plain(par);
        Results a = plain.run(b, phys);
        ShellSolver sh(par, {}, {});                       // ShellOptions.enabled = false
        ShellReport rep;
        Results c = sh.run(b, phys, rep);
        fails += chk("disabled => identical Dt", a.translational_diffusion_centre == c.translational_diffusion_centre);
        fails += chk("disabled => identical eta", a.intrinsic_viscosity_high == c.intrinsic_viscosity_high);
        fails += chk("disabled => report byte-identical", a.report == c.report);
        fails += chk("disabled => viscosity is usable", !rep.viscosity_unreliable);
        fails += chk("disabled => not attempted", !rep.attempted);
    }

    // ---- MW and Rg are pinned to the full model --------------------------------
    // If Solver re-derived them from a reduced bead list, MW would fall with the dropped
    // beads (corrupting sedimentation and both viscosities) and Rg would rise.
    {
        auto b = blob(5, 3.0, 2.0);
        Solver plain(par);
        Results full = plain.run(b, phys);
        ShellOptions so; so.enabled = true; so.tol = 1e-9;   // force deep reduction attempts
        so.ladder = {0.25, 0.5};
        ShellSolver sh(par, {}, so);
        ShellReport rep;
        Results red = sh.run(b, phys, rep);
        fails += chk("reduction actually happened", rep.n_used < rep.n_full);
        fails += chk("MW pinned to full model", std::fabs(red.mass - full.mass) < 1e-9 * full.mass);
        fails += chk("Rg2 pinned to full model", std::fabs(red.rg2 - full.rg2) < 1e-9 * full.rg2);
    }

    // ---- HONESTY: the reported bar bounds the true error ------------------------
    {
        auto b = blob(6, 3.0, 2.0);                        // 216 beads
        Solver plain(par);
        Results full = plain.run(b, phys);

        for (double tol : {5e-2, 2e-2, 1e-2}) {
            ShellOptions so;
            so.enabled = true; so.tol = tol;
            so.require = {Obs::Dt, Obs::Dr, Obs::Sedimentation, Obs::EtaInf, Obs::EtaZero};
            so.ladder = {0.125, 0.25, 0.5};                // never the full model
            ShellSolver sh(par, {}, so);
            ShellReport rep;
            Results r = sh.run(b, phys, rep);
            for (size_t m = 0; m < rep.require.size(); ++m) {
                double ref = obs_value(full, rep.require[m]);
                double got = obs_value(r, rep.require[m]);
                double true_err = std::fabs(got - ref) / std::fabs(ref);
                char msg[200];
                std::snprintf(msg, sizeof(msg), "tol=%.3g %s: bar %.4g%% >= true %.4g%%",
                              tol, obs_name(rep.require[m]), 100 * rep.err_est[m], 100 * true_err);
                fails += chk(msg, rep.err_est[m] >= true_err);
            }
        }
    }

    // ---- viscosity is flagged unreliable when not required ----------------------
    {
        auto b = blob(5, 3.0, 2.0);
        ShellOptions so;
        so.enabled = true; so.tol = 5e-2;
        so.require = {Obs::Dt, Obs::Dr};                   // viscosity deliberately absent
        so.ladder = {0.125, 0.25, 0.5};
        ShellSolver sh(par, {}, so);
        ShellReport rep;
        Results r = sh.run(b, phys, rep);
        fails += chk("viscosity flagged unreliable when not required", rep.viscosity_unreliable);
        fails += chk("report warns about viscosity", r.report.find("UNRELIABLE") != std::string::npos);
        fails += chk("report records the reduction", r.report.find("SHELL REDUCTION") != std::string::npos);
        fails += chk("viscosity value is still present in the report for the record", r.intrinsic_viscosity_high != 0.0);
    }

    std::printf("%s (%d failures)\n", fails ? "FAILURES" : "ALL PASS", fails);
    return fails ? 1 : 0;
}
