// Unit tests for shell reduction (grpy_exposure.hpp + grpy_shell.hpp).
//
// The central property under test is HONESTY: the reported error bar must bound the true
// error, measured against an independently computed unreduced solve. A bar that
// understates is worse than no bar at all, so that check is the one that matters.
#include <algorithm>
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

    // ---- the report gives BOTH values, and says which one the scalars are -------
    //
    // The estimate is the distance between the computed value and the extrapolated one, so
    // a report showing only one of them cannot tell the reader what the estimate applies
    // to. It applies to the computed value -- the finest rung -- which is also what
    // Results carries; rep.reported must therefore agree with Results exactly, or the
    // report and the returned scalars would describe different answers.
    {
        auto b = blob(6, 3.0, 2.0);
        ShellOptions so;
        so.enabled = true; so.tol = 1e-2;
        so.require = {Obs::Dt, Obs::Dr, Obs::EtaInf};
        so.ladder = {0.125, 0.25, 0.5};                    // never the full model
        ShellSolver sh(par, {}, so);
        ShellReport rep;
        Results r = sh.run(b, phys, rep);

        fails += chk("reported[] is parallel to require[]", rep.reported.size() == rep.require.size());
        for (size_t m = 0; m < rep.require.size() && m < rep.reported.size(); ++m) {
            char msg[200];
            std::snprintf(msg, sizeof(msg), "%s: reported[] == the returned scalar",
                          obs_name(rep.require[m]));
            fails += chk(msg, rep.reported[m] == obs_value(r, rep.require[m]));
        }
        fails += chk("report carries both columns",
                     r.report.find("computed") != std::string::npos &&
                     r.report.find("extrapolated") != std::string::npos);
        fails += chk("report says which column the results are",
                     r.report.find("are the COMPUTED column") != std::string::npos);
        // The two columns must actually differ somewhere, or the test would pass on a
        // report that silently printed the same number twice.
        bool differ = false;
        for (size_t m = 0; m < rep.reported.size() && m < rep.extrapolated.size(); ++m)
            if (rep.reported[m] != rep.extrapolated[m]) differ = true;
        fails += chk("computed and extrapolated are distinct answers", differ);
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

    // ---- ladder exhausting onto the full model reports an EXACT result ----------
    // Caught end-to-end: a 246-bead model at tol=2% could not converge, ran out onto the
    // unreduced rung, and then reported the stale inter-rung gap (1.68%) as the error of
    // an answer that was in fact exact -- and wrongly flagged viscosity unreliable.
    {
        auto b = blob(4, 3.0, 2.0);
        Solver plain(par);
        Results full = plain.run(b, phys);
        ShellOptions so;
        so.enabled = true; so.tol = 1e-12;              // unsatisfiable: force exhaustion
        so.require = {Obs::Dt, Obs::Dr};                // viscosity deliberately absent
        so.ladder = {0.25, 0.5, 1.0};                   // last rung IS the full model
        ShellSolver sh(par, {}, so);
        ShellReport rep;
        Results r = sh.run(b, phys, rep);
        fails += chk("exhausted ladder flags unreduced", rep.unreduced);
        fails += chk("unreduced uses every bead", rep.n_used == rep.n_full);
        fails += chk("unreduced reports zero error", rep.err_max == 0.0);
        fails += chk("unreduced matches the plain Solver",
                     r.translational_diffusion_centre == full.translational_diffusion_centre);
        fails += chk("unreduced => viscosity usable even when not required",
                     !rep.viscosity_unreliable);
        fails += chk("unreduced report says the result is exact",
                     r.report.find("exact") != std::string::npos);
    }

    // ---- memory cap (issue 987 interaction) ------------------------------------
    // The caller's pre-flight guard sizes the mobility matrix from the FULL bead count and
    // refuses when it exceeds RAM. That would turn away exactly the large models shell
    // reduction exists to make feasible, since the ladder usually stops well short of the
    // full model. ShellOptions::max_beads lets the caller cap the ladder instead, so an
    // oversized model yields the largest rung that fits WITH its bar rather than nothing.
    {
        auto b = blob(6, 3.0, 2.0);                     // 216 beads

        // A cap above the full model must change nothing at all.
        {
            ShellOptions so;
            so.enabled = true; so.tol = 1e-12;          // unsatisfiable: force exhaustion
            so.require = {Obs::Dt, Obs::Dr};
            so.ladder = {0.25, 0.5, 1.0};
            so.max_beads = 100000;
            ShellSolver sh(par, {}, so);
            ShellReport rep;
            Results r = sh.run(b, phys, rep);
            fails += chk("slack cap does not bind", !rep.mem_capped);
            fails += chk("slack cap still reaches the full model", rep.unreduced);
            fails += chk("slack cap still reports an exact result", rep.err_max == 0.0);
            (void) r;
        }

        // A cap that binds must stop the ladder, say so, and NOT claim convergence.
        {
            const int cap = 60;                         // admits 0.125 (27) and 0.25 (54)
            ShellOptions so;
            so.enabled = true; so.tol = 1e-12;          // unsatisfiable: only the cap can stop it
            so.require = {Obs::Dt, Obs::Dr};
            so.ladder = {0.125, 0.25, 0.5, 1.0};
            so.max_beads = cap;
            ShellSolver sh(par, {}, so);
            ShellReport rep;
            Results r = sh.run(b, phys, rep);
            fails += chk("binding cap is reported", rep.mem_capped);
            fails += chk("binding cap never exceeds the budget", rep.n_used <= cap);
            fails += chk("binding cap did not reach the full model", !rep.unreduced);
            fails += chk("binding cap does not claim convergence", !rep.converged);
            fails += chk("binding cap still produced a result", rep.levels >= 1 && rep.n_used > 0);
            fails += chk("binding cap reports a finite error bar",
                         rep.err_max > 0.0 && std::isfinite(rep.err_max));
            fails += chk("binding cap explains itself in the report",
                         r.report.find("stopped by the available memory") != std::string::npos);
        }

        // A cap below even the smallest rung yields no result at all -- the caller must
        // detect this (levels == 0) and fall back to refusing, rather than report zeros.
        {
            ShellOptions so;
            so.enabled = true; so.tol = 1e-12;
            so.require = {Obs::Dt, Obs::Dr};
            so.ladder = {0.125, 0.25, 0.5, 1.0};
            so.max_beads = 4;                           // smaller than 0.125 * 216
            ShellSolver sh(par, {}, so);
            ShellReport rep;
            sh.run(b, phys, rep);
            fails += chk("impossible cap runs no rung", rep.levels == 0 && rep.mem_capped);
        }
    }

    // ---- progress is ladder-wide, not per-rung ---------------------------------
    // Each rung is its own solve sweeping 0..100%, so forwarding progress raw made the
    // bar restart once per rung and the stage text never said which rung was running.
    {
        auto b = blob(5, 3.0, 2.0);
        ShellOptions so;
        so.enabled = true; so.tol = 1e-12;              // unsatisfiable: run the whole ladder
        so.require = {Obs::Dt, Obs::Dr};
        so.ladder = {0.125, 0.25, 0.5, 1.0};
        ShellSolver sh(par, {}, so);
        ShellReport rep;

        std::vector<int> pcts;
        std::vector<std::string> stages;
        sh.run(b, phys, rep, [&](int pct, const char* stage) {
            pcts.push_back(pct);
            stages.push_back(stage ? stage : "");
        });

        bool monotone = true, in_range = true, labelled = !stages.empty();
        for (size_t i = 0; i < pcts.size(); ++i) {
            if (i && pcts[i] < pcts[i - 1]) monotone = false;
            if (pcts[i] < 0 || pcts[i] > 100) in_range = false;
            if (stages[i].find("rung ") == std::string::npos) labelled = false;
        }
        fails += chk("progress was reported at all", !pcts.empty());
        fails += chk("progress never goes backwards across rungs", monotone);
        fails += chk("progress stays within 0..100", in_range);
        fails += chk("every stage names its rung", labelled);
        fails += chk("stage names the bead count",
                     !stages.empty() && stages.back().find(" beads: ") != std::string::npos);
        // Several rungs ran, but the bar restarted zero times: proof of the remap.
        int restarts = 0;
        for (size_t i = 1; i < pcts.size(); ++i) if (pcts[i] < pcts[i - 1]) ++restarts;
        fails += chk("multiple rungs ran", rep.levels > 1);
        fails += chk("bar restarted zero times despite multiple rungs", restarts == 0);
    }

    // ---- disabled shell reduction leaves progress untouched ---------------------
    {
        auto b = blob(4, 3.0, 2.0);
        ShellSolver sh(par, {}, {});                    // enabled = false
        ShellReport rep;
        bool any_rung_text = false, saw = false;
        sh.run(b, phys, rep, [&](int, const char* stage) {
            saw = true;
            if (stage && std::string(stage).find("rung ") != std::string::npos) any_rung_text = true;
        });
        fails += chk("disabled => progress still reported", saw);
        fails += chk("disabled => stage text is unmodified", !any_rung_text);
    }

    // ---- chunked progress is numerically inert, and covers all three phases ------
    // assemble/factor/solve are chunked so the callback can fire inside them (the calling
    // thread is a compute worker, so nothing else can keep a GUI alive). Chunking changes
    // parallel granularity, so the first thing to prove is that it changes no result.
    {
        auto b = blob(5, 3.0, 2.0);
        Solver plain(par);
        Results without = plain.run(b, phys);                  // no callback -> single chunk

        std::vector<int> pcts;
        std::vector<std::string> stages;
        Results with = plain.run(b, phys, [&](int pct, const char* stage) {
            pcts.push_back(pct);
            stages.push_back(stage ? stage : "");
        });

        fails += chk("chunking does not change Dt",
                     with.translational_diffusion_centre == without.translational_diffusion_centre);
        fails += chk("chunking does not change eta",
                     with.intrinsic_viscosity_high == without.intrinsic_viscosity_high);
        fails += chk("chunking does not change the report", with.report == without.report);

        bool monotone = true, in_range = true;
        for (size_t i = 0; i < pcts.size(); ++i) {
            if (i && pcts[i] < pcts[i - 1]) monotone = false;
            if (pcts[i] < 0 || pcts[i] > 100) in_range = false;
        }
        auto saw = [&](const char* s) {
            for (auto& t : stages) if (t.find(s) != std::string::npos) return true;
            return false;
        };
        fails += chk("progress is monotone", monotone);
        fails += chk("progress within 0..100", in_range);
        fails += chk("assembly phase reports", saw("BUILDING MOBILITY MATRIX"));
        fails += chk("factor phase reports", saw("INVERTING MATRICES"));
        fails += chk("solve phase reports", saw("SOLVING"));
        // Tick COUNT is deliberately not asserted tightly. The chunker targets a wall-clock
        // slice, so on a small model that finishes in milliseconds it correctly uses few
        // large chunks; fine granularity only appears where it is needed. What must hold is
        // that no phase is silent -- assembly and solve previously reported nothing at all.
        fails += chk("every phase produced at least one tick", pcts.size() >= 3);
    }

    // ---- recorded subsets identify the beads actually solved on ------------------
    // The caller writes these out as bead models, so an index that does not name the bead
    // the solver used would produce a picture of the wrong thing -- silently.
    {
        auto b = blob(5, 3.0, 2.0);
        ShellOptions so;
        so.enabled = true; so.tol = 1e-12;             // unsatisfiable: run the whole ladder
        so.require = {Obs::Dt, Obs::Dr};
        so.ladder = {0.125, 0.25, 0.5, 1.0};
        so.record_subsets = true;
        ShellSolver sh(par, {}, so);
        ShellReport rep;
        sh.run(b, phys, rep);

        fails += chk("one recorded subset per rung", rep.kept.size() == rep.ns.size());
        bool sized = true, in_range = true, unique = true, exposed_first = true;
        for (size_t r = 0; r < rep.kept.size(); ++r) {
            if ((int)rep.kept[r].size() != rep.ns[r]) sized = false;
            std::vector<bool> seen(b.size(), false);
            for (int k : rep.kept[r]) {
                if (k < 0 || k >= (int)b.size()) { in_range = false; continue; }
                if (seen[k]) unique = false;
                seen[k] = true;
            }
            // Each rung is a superset of the one before: the ladder grows the shell.
            if (r) for (int k : rep.kept[r - 1])
                if (std::find(rep.kept[r].begin(), rep.kept[r].end(), k) == rep.kept[r].end())
                    exposed_first = false;
        }
        fails += chk("recorded size matches the rung bead count", sized);
        fails += chk("indices are in range", in_range);
        fails += chk("indices are unique within a rung", unique);
        fails += chk("each rung contains the previous rung's beads", exposed_first);
        // The final rung is the full model, so it must name every bead.
        fails += chk("full rung records every index",
                     rep.unreduced && rep.kept.back().size() == b.size());

        // And the selection must be the same beads the solver actually used.
        std::vector<double> ex = shell::exposure(
            [&]{ std::vector<core::Bead> c(b.size());
                 for (size_t i = 0; i < b.size(); ++i)
                     c[i] = {b[i].x, b[i].y, b[i].z, b[i].radius, b[i].mw};
                 return c; }(), (int)b.size(), so.K, so.probe);
        bool matches = true;
        for (size_t r = 0; r + 1 < rep.kept.size(); ++r) {
            auto want = shell::reduce_top_frac_idx(
                [&]{ std::vector<core::Bead> c(b.size());
                     for (size_t i = 0; i < b.size(); ++i)
                         c[i] = {b[i].x, b[i].y, b[i].z, b[i].radius, b[i].mw};
                     return c; }(), ex, so.ladder[r]);
            if (want.size() != rep.kept[r].size()) { matches = false; continue; }
            for (size_t i = 0; i < want.size(); ++i)
                if ((int)want[i] != rep.kept[r][i]) matches = false;
        }
        fails += chk("recorded indices are the ranking's own selection", matches);

        // Off by default: nothing recorded, so nothing paid for.
        ShellOptions off = so; off.record_subsets = false;
        ShellSolver sh2(par, {}, off);
        ShellReport rep2;
        sh2.run(b, phys, rep2);
        fails += chk("not recorded unless asked", rep2.kept.empty() && rep2.ns.size() > 1);
    }

    // ---- cancellation between rungs ---------------------------------------------
    // A user watching the shells appear must be able to stop on seeing something wrong.
    // Stopping keeps whatever was computed, on its bar, and never claims convergence.
    {
        auto b = blob(5, 3.0, 2.0);
        ShellOptions so;
        so.enabled = true; so.tol = 1e-12;            // unsatisfiable: only a stop can end it
        so.require = {Obs::Dt, Obs::Dr};
        so.ladder = {0.125, 0.25, 0.5, 1.0};
        int seen = 0;
        so.should_stop = [&]{ return seen >= 2; };    // let two rungs run, then stop
        so.on_rung = [&](int, int, int, double){ ++seen; };
        ShellSolver sh(par, {}, so);
        ShellReport rep;
        Results r = sh.run(b, phys, rep);

        fails += chk("stop is reported", rep.stopped);
        fails += chk("stop does not claim convergence", !rep.converged);
        fails += chk("stop keeps the rungs already run", rep.levels == 2);
        fails += chk("stop did not reach the full model", !rep.unreduced);
        fails += chk("stopped result still carries a bar",
                     rep.err_max > 0.0 && std::isfinite(rep.err_max));
        fails += chk("stopped result is usable", r.translational_diffusion_centre > 0.0);
        fails += chk("report explains the stop",
                     r.report.find("stopped before the ladder") != std::string::npos);

        // Stopping before anything runs leaves no result, like an impossible memory cap.
        ShellOptions so2 = so;
        so2.should_stop = []{ return true; };
        so2.on_rung = nullptr;
        ShellSolver sh2(par, {}, so2);
        ShellReport rep2;
        sh2.run(b, phys, rep2);
        fails += chk("stop before any rung leaves no result",
                     rep2.stopped && rep2.levels == 0);

        // And with no hook the ladder is unaffected.
        ShellOptions so3 = so; so3.should_stop = nullptr; so3.on_rung = nullptr;
        ShellSolver sh3(par, {}, so3);
        ShellReport rep3;
        sh3.run(b, phys, rep3);
        fails += chk("no hook => ladder runs to the full model",
                     !rep3.stopped && rep3.unreduced);
    }

    std::printf("%s (%d failures)\n", fails ? "FAILURES" : "ALL PASS", fails);
    return fails ? 1 : 0;
}
