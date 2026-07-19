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
