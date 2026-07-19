# In-process GRPY — SOMO integration guide

The GRPY numeric core + report + API are a **self-contained `grpy` module** with no
SOMO dependency and their own unit tests. SOMO calls it directly instead of spawning
the GRPY binary and scraping stdout. Nothing is added to the `US_Hydrodyn` god class —
the integration is a thin call site plus a small mapping.

## 1. Files to place under `us_somo/develop`

Header-only module (drop into `include/grpy/`):
```
include/grpy/linalg.hpp          tiled SPD Cholesky + la::Parallel abstraction
include/grpy/grpy_core.hpp       RPY tensors, assembly, tiled solve, pre/post
include/grpy/grpy_report.hpp     report writer + display formulas + constants
include/grpy/grpy_api.hpp        grpy::{Bead,PhysParams,Options,Results,Solver}
include/grpy/parallel_qt.hpp     QtParallel backend (QThreadPool/QtConcurrent)
include/grpy/parallel_std.hpp    std::thread backend (not needed in SOMO)
```
Eigen is already vendored at `us_somo/develop/include/Eigen`.

## 2. qmake (`libus_somo.pro`)
```
QT += concurrent          # add to the QT line (QtConcurrent for QtParallel)
INCLUDEPATH += include/grpy
HEADERS += include/grpy/linalg.hpp include/grpy/grpy_core.hpp \
           include/grpy/grpy_report.hpp include/grpy/grpy_api.hpp \
           include/grpy/parallel_qt.hpp
```
The module is header-only, so no new `.cpp` unless you factor the call site below into
its own translation unit (recommended — see step 4).

For the eventual CMake build, this is one `INTERFACE` (header-only) target + linking
`Qt5::Concurrent`; trivial to add when SOMO's CMake lands.

## 3. Result mapping (grpy::Results -> SOMO)
The old code scraped these from stdout; now read them from the struct:
| SOMO field (`this_data`)            | grpy::Results field                 |
|-------------------------------------|-------------------------------------|
| `results.s20w`                      | `sedimentation`                     |
| `results.D20w`                      | `translational_diffusion`           |
| `results.viscosity`                 | `intrinsic_viscosity_zero`          |
| `rot_diff_coef`                     | `rotational_diffusion`              |
| `rel_times_tau_1..5` (×1e9)         | `tau_tensor[0..4]` (×1e9)           |
| `rel_times_tau_h` (×1e9)            | `tau_harmonic` (×1e9)               |
| `results.rs` (×1e7)                 | `stokes_radius_Dt` (×1e7)           |
| `grpy_einst_rad` (×1e7)             | `stokes_radius_Dr` (×1e7)           |
| `results.rg`                        | derive from `rg2`/existing SOMO calc|
| `results.mass`                      | `mass` (or SOMO's own model_mw)     |

(The `×1eN` scalings are exactly what the old scraper applied — keep them.)

## 4. Call site (replaces the QProcess machinery)
Currently: `grpy_process_next()` builds args + launches `QProcess`, and
`grpy_finished()` parses `grpy_stdout`. Replace both with a direct call. Keep it OFF
the god class by putting the glue in a small free function (own .cpp), e.g.
`src/us_hydrodyn_grpy_direct.cpp`:

```cpp
#include "grpy/grpy_api.hpp"
#include "grpy/parallel_qt.hpp"

// build beads from a SOMO bead model (same selection write_bead_model used:
// include buried beads iff hydro.grpy_bead_inclusion, else skip color==6)
static std::vector<grpy::Bead> to_grpy_beads(const vector<PDB_atom>& model, bool inclusion) {
    std::vector<grpy::Bead> b;
    for (const auto& a : model) {
        if (!a.active) continue;
        if (!inclusion && get_color(&a) == 6) continue;
        b.push_back({ a.bead_coordinate.axis[0], a.bead_coordinate.axis[1],
                      a.bead_coordinate.axis[2], a.bead_computed_radius,
                      a.bead_ref_mw + a.bead_ref_ionized_mw_delta });
    }
    return b;
}
```

Then, per model (synchronous, run on a worker so the GUI stays live — see step 5):
```cpp
la::QtParallel par( USglobal->config_list.numThreads );
grpy::Options opt;  opt.single = /* large-N option */;  opt.ooc_dir = /* optional */;
grpy::PhysParams p; p.vbar = use_vbar(...); p.units = ...; p.mw = ...; // = old bead_model header
grpy::Solver solver( par, opt );
grpy::Results r = solver.run( beads, p,
    [this](int pct, const char* stage){ /* update SOMO progress bar; same % SOMO parsed */ } );

// preserve the results file exactly as before:
QFile f( out_name ); f.open(QIODevice::WriteOnly);
f.write( QByteArray::fromStdString( r.report ) );  f.close();

// map r -> this_data.results (table in step 3), then continue the existing
// grpy_finished() bookkeeping (grpy_results accumulation, save_data, etc.)
```

## 5. Threading / responsiveness
The old flow was async (QProcess + signals) so the GUI stayed responsive. In-process,
run `solver.run(...)` on a worker (`QtConcurrent::run`, or a QThread) and marshal the
progress callback + completion back to the GUI thread via queued signals. `QtParallel`
uses the global `QThreadPool`; the compute worker + the pool coexist (the worker mostly
blocks in `solver.run`). This removes QProcess, the temp `.grpy`/`.dat` staging, and the
stdout regex — while still writing the `.dat` report for preservation (step 4).

## 6. What this deletes / simplifies (later, incrementally)
- `us_container_grpy.*` (Docker/binary launch) — no longer needed for compute.
- `grpy_readFromStdout` / `grpy_readFromStderr` / the `NN% TASK:` regex parsing.
- The bead_model temp-file write for GRPY input.
Keep them until the in-process path is validated in the GUI, then remove per the
"evaluate removing some later" note.

## Validation
`phase4/test_api.cpp` (std::thread backend) proves: report **byte-identical** to the
golden (dumbbell), structured scalars consistent with the report text (no drift, both
cases), progress callback fires. `phase4/qt_proof.cpp` proves the same via `QtParallel`
(SOMO's backend), linking QtCore+QtConcurrent.
