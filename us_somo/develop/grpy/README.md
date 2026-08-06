# Phase 4 — in-process API (`grpy::Solver`)

A clean, self-contained module so SOMO calls GRPY **directly as a function** instead of
spawning the binary and scraping stdout. No SOMO types, no god-class methods — just a
namespaced class with injected threading and its own tests. Build & validate:

```
./validate.sh            # std::thread backend; add `. qt5env` first for the Qt proof
```

## The API
```cpp
grpy::Bead        { x, y, z, radius, mw };
grpy::PhysParams  { temperature_C, eta, rho, vbar, units, mw, input_label };  // -u defaults
grpy::Options     { single, tile, ooc_dir };
grpy::Results     { rotational_diffusion, sedimentation, translational_diffusion,
                    intrinsic_viscosity_{high,zero}, tau_vector[3], tau_tensor[5],
                    tau_harmonic, stokes_radius_{Dt,Dr}, rg2, mass,
                    diffusion_origin[6][6], diffusion_centre[6][6],
                    std::string report };          // <- full report text, preserve to disk

class grpy::Solver {
    Solver(la::Parallel& par, Options = {});        // threading injected (Qt or std)
    Results run(const std::vector<Bead>&, const PhysParams&, ProgressFn = {}) const;
};
```

`Results::report` is the **full GRPY report text** (byte-identical to the Fortran modulo
the eigenvector gauge / ~1e-16 noise), so the caller can write it to disk and preserve
the results file exactly as before — while also getting the structured scalars directly.

## Files (all header-only except tests)
```
grpy_report.hpp   report writer + display formulas + constants  (shared with the CLI)
grpy_api.hpp      Bead/PhysParams/Options/Results/Solver + derive_scalars
grpy_core.hpp     RPY core (from phase3)      linalg.hpp   tiled solve + Parallel
parallel_qt.hpp   QtParallel (SOMO)           parallel_std.hpp  std::thread (CLI/tests)
test_api.cpp      unit tests                  qt_proof.cpp      Qt-backend in-process proof
INTEGRATION.md    exact SOMO placement + qmake + minimal call-site change
```

## Validation
- **`test_api`** (std::thread backend): report matches the golden numerically (sign-flip
  + noise tolerant) for dumbbell and 1znf; structured scalars consistent with the report
  text (guards `derive_scalars` against drift); progress callback fires.
- **`qt_proof`** (QtParallel — SOMO's backend): `Solver::run` executes in-process, progress
  callback delivers `NN% INVERTING MATRICES`, scalars correct. Links QtCore + QtConcurrent.

## Design notes
- Threading is **dependency-injected** (`la::Parallel&`), so the module is Qt-agnostic and
  testable with std::threads; SOMO passes a `QtParallel` over its `QThreadPool`.
- The CLI (`phase3/grpy.cpp`) and this API share `grpy_report.hpp`, so their output can
  never drift.
- See `INTEGRATION.md` for the SOMO call site: build beads from the bead model, run the
  Solver on a Qt worker (GUI stays responsive), map `Results` → `this_data.results`, write
  `Results::report` to the `.dat` file. Retire QProcess / stdout-scraping incrementally.

---

# Shell reduction (`grpy::ShellSolver`)

GRPY costs O((11N)^3), so the bead count dominates everything. A dense bead packing is
hydrodynamically **screened** — interior beads sit in near-stagnant fluid and carry almost
no force — so the exact calculation can run on a surface-enriched subset at small cost in
accuracy. `ShellSolver` does that, and reports the error it introduced.

```cpp
grpy::ShellOptions so;
so.enabled = true;
so.tol     = 5e-3;                                    // 0.5% on every required observable
so.require = { grpy::Obs::Dt, grpy::Obs::Dr, grpy::Obs::Sedimentation };

grpy::ShellSolver solver( par, {}, so );
grpy::ShellReport rep;
grpy::Results r = solver.run( beads, phys, rep );

if ( rep.viscosity_unreliable ) { /* do NOT propagate r.intrinsic_viscosity_* */ }
```

## How it works

1. **Exposure** (`grpy_exposure.hpp`) — Shrake-Rupley per bead with a 1.4 Å probe, then
   keep the most-exposed `ceil(f*N)`. Selection is by *target bead fraction*, never a fixed
   exposure threshold: the buried fraction ranges ~1%–50% across model types, so one
   threshold gives wildly different cost per model while a fraction controls O(N^3)
   directly. The probe is essential — at probe 0 almost nothing registers as buried.
2. **Ladder** — solve on a doubling ladder of bead fractions, stop when the reported bar
   drops below `tol`. Geometric against O(N^3), so the whole ladder costs ~1.14× its final
   rung: the convergence check is nearly free. The final 1.0 rung is the unreduced model,
   so an unreducible structure degrades to exactly today's behaviour.
3. **Richardson** — three rungs give the convergence order from the ratio of successive
   gaps; the remaining error is `gap2/(r^k − 1)`. Per observable, since each converges at
   its own order.

## What it guarantees, and what it does not

The reported bar **bounds the true error**: verified 92/92 (translational, 23 models,
N=204–4068) and 252/252 (7 observables × 12 models × 3 tolerances) against unreduced exact
GRPY. The observed order (median 1.83) matches a value predicted independently by a raw
reduction sweep, so the error model is derived rather than fitted.

Known failure mode, guarded: on structures with a **degenerate exposure distribution** (a
perfect cubic lattice realizes ~9 distinct values over 216 beads) rungs swallow whole
symmetry shells instead of refining, the estimated order comes out spuriously high, and the
bar understated by ~1.4×. A floor caps extrapolation tightening at 2× the raw gap, which
restores honesty there and is slack on every real model tested (identical honesty and
conservatism, no speedup cost). `tests/test_shell.cpp` keeps that lattice as a regression.

## Observables do not share a frontier

Median error relative to `D_t` at equal reduction: anisotropy 0.15×, `D_r` 1.77×,
**intrinsic viscosity 3.34×**. Viscosity drove the stopping decision in 36/36 test cases,
so requiring it costs a large part of the speedup — hence `ShellOptions::require`.

When viscosity is not required (or is required and does not converge), `ShellReport
::viscosity_unreliable` is set. **Callers must then withhold `intrinsic_viscosity_high`,
`intrinsic_viscosity_zero`, and the viscosity-derived Einstein radius** from reported
results. The values remain in `Results::report` for the record, with a warning appended.

## Speedup, honestly

Against an unreduced model the gain reaches ~120×, but SOMO defaults to ASA buried-bead
exclusion, which already removes most dead beads. Measured on that production baseline the
incremental gain is **~2–3×**, rising with model size and largest where it is wanted. The
durable contribution is the error bar: the existing exclusion is a binary heuristic that
reports no uncertainty at all.

## Correctness details worth knowing

- **MW and Rg are pinned to the full model.** Left to re-derive from a reduced bead list,
  MW would fall with the dropped beads (corrupting sedimentation and both viscosities,
  which are mass-normalized) and Rg would rise (a hollow shell has a larger radius of
  gyration than the solid body). Regression-tested.
- **Disabled by default.** With `enabled = false` the result is byte-identical to
  `Solver::run` — results never move silently.
- Do **not** use a finer ladder. A ×1.5 ladder was tested: no median speedup gain, and
  honesty fell 100% → 84%. Well-separated rungs are what make the bar trustworthy.
