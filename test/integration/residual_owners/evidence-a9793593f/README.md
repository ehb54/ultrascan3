# Linux verification at a9793593f

Production revision: `a9793593f`, including all seven commits after `097beff8e`.
Oracle Linux 8.10 aarch64 VM, GCC 8.5, Qt 5.15.3, Qwt 6.1.5;
native Qt X11 windows on Xvfb `:43` with xfwm. These are not macOS GMP results.
ASan and UBSan halt on errors; leak detection is disabled. No Windows rerun
or physical display/GPU acceptance is claimed here.

## Results

| Check | Result | Evidence |
|---|---|---|
| CMake build/link all seven consumers | PASS | `consumers-build.log.gz`, `consumers-build.status` |
| Launch all seven with fatal sanitizers | PASS: visible windows, processes alive, no missing libraries | `launch-evidence/` |
| Residual dialog suite | 127 passed, 0 failed, 1 intentional child-entry skip | `residual.log` |
| Full utility suite | 891 passed; 10 opt-in live-DB tests skipped | `utils.log.gz` |
| D14 merge regression cases | All three passed under sanitizers | Utility log, `MergeTimeStateSteps_*` |
| M12 `ctest -L dependency_layers` on clean tracked source | 2/2 passed | `m12-clean-ctest.log` |
| M12 on source previously built with qmake | FAIL: 65 false positives for generated intra-library `../` includes | `m12-ctest.log` |
| M12 independent negative/positive probes | FAIL: misses `utils -> us_gui_static`; rejects valid `gui/moc -> gui` include | `m12-review-probes.json` |
| FEMatch bitmap Exclude/Reset | PASS: pixels change then restore exactly; focus stays on main window | `bitmap-*.png`, `bitmap-pixel-comparison.json`, focus screenshots |
| FEMatch bitmap untick/retick | PASS: closes, then raises bitmap when explicitly reticked | `fematch-bitmap-*.png` |
| FEMatch real-owner saved output | 27/27 content-identical to c610 baseline; SVGZ compared after decompression | `fematch-comparison.json` |
| Reporter manual Abort/retry/Continue | PASS: Aborted message, no files after Abort, retry enabled; Continue completes PDF | `reporter-abort-observation.json`, reporter screenshots |
| Reporter saved plot comparison | Both PNGs byte-identical to c610 baseline | `reporter-image-comparison.json` |
| Reporter AUTO Generate Again | PASS: same process restarts and reaches acceleration warning again | `reporter-auto-*.png`, `a979-reporter-auto.cpp` |
| D14 real FEMatch load with TimeState and no experiment steps | PASS for local-disk path; no sanitizer error | `d14-local-load.png`, fixture scripts/log |
| D13 old FEMatch abort | INCONCLUSIVE: not reproduced in this shorter debugger session | `fematch-gdb.log`, `fematch-gdb-launch.json` |

## Build and test reproduction

The existing frozen Linux source/build was advanced using the tracked-file delta
from `2ddf75d6e` to `a9793593f`. There were no deleted files in that delta.
`source-manifest.json` and `source-verification.json` record all 125 changed
files and zero hash mismatches. Scripts retain exact VM paths and flags.

1. Use `a979-prepare.py` and the manifest to prepare the source.
2. Run `a979-build.sh`. An old `qwtplot3d_DIR` cache entry required regeneration
   with `cmake -S source -B build -Uqwtplot3d_DIR`; this was a build-environment
   repair, not a source change. This script stops at the M12 failure in the
   qmake-built tree; its overall exit is not a passing result.
3. Run `a979-tests.sh` independently for both sanitizer suites. The utility
   wrapper in `a979-utils-tests.cmake` links the actual built libraries.
4. Run `a979-clean-m12.py` for CTest against a clean archive of the tracked
   checker inputs, and `a979-boundary-review.py` for the additional probes.
   Do not erase generated files to conceal the qmake-tree failure.
5. Run `a979-launch-check.py` for all seven consumers. Project programs can
   show the expected missing-Optima-server notice; no instrument test is claimed.

The checker workflow currently filters PRs to `main`, whereas #628 targets
`calc-library-extraction`. The two independently reproduced checker defects
and this CI coverage limitation must be reviewed before M12 is signed off.

## Native owner procedures and limits

Use the isolated local test settings and the established `multicell-demo-ra`
fixture (run 1011, edit 1015), model
`2DSA-IT.1A123.e2608180000_a2609300625_000206_i01.multicell-demo-ra` (1037).
Load Experiment from Database, Load Model, Simulate Model. Baseline variance
is `7.81078e-05`, RMSD `0.00883786`.

Open the residual bitmap. Exclude scans 1–2: variance becomes `3.77076e-05`,
RMSD `0.00614065`, the bitmap pixels change and the main window retains focus.
Reset Excludes: original statistics and bitmap pixel hash return, without
raising the bitmap. Untick then retick Show Residuals Bitmap: it closes then
comes to the front. The whole-window pixel hashes verify the actual refresh.

For saved-file comparison use a fresh FEMatch instance, repeat load/model/
simulation, close the residual dialog, Save Data, Skip the optional 3D plot.
Keep the pointer outside plot canvases before exporting. An initial interaction
run matched 19/27 files, with crosshair overlays in eight PNGs; all its SVG
content already matched. `fematch-interaction-comparison.json` retains that
attempt. The clean rerun matches all 27 files, including every PNG, CSV and HTML.
Full exports are retained locally under
`build/real-owner-verification/a979-collected/output-a979/reports/` and compared
with `build/real-owner-verification/c610/multicell-demo-ra`.

Reporter: load workflow 1240001 using the isolated acceleration-warning profile
(`SetSpeedLowAcc=100000`), select velocity and residual plots, Generate Report.
Abort the real acceleration warning. Verify the new Aborted message, no report
files and no Complete message. Generate Report again in the same process;
Continue the warning. This produced the PDF and Complete message. Both saved
plot PNGs exactly match the earlier successful report. This is a plot-image
comparison and successful PDF generation, not a bytewise PDF comparison.

AUTO retry uses the actual reporter AUTO constructor and `loadRun_auto` with a
minimal test entry point, not a substitute reporter. Abort offers Generate Again;
that action restarts the report and reaches the same acceleration warning.
The second attempt was aborted. The fixture's missing TimeState-ID run-details
notice was acknowledged. No full staged AUTO workflow or full AUTO report is
claimed. The helper is linked using the production reporter objects; see
`a979-build-probes.py`.

D14: `a979-d14-local.py` creates an isolated local copy of the existing raw/edit
fixture; `a979-make-timestate.cpp` uses the production TimeState writer. No
experiment XML is supplied, so experiment speed steps are empty. FEMatch loads
and plots this data with the TimeState present, without a sanitizer failure.
The expected missing local solution/buffer XML notice is acknowledged. This
checks the real local empty-step loading path, not a deletion of database rows;
no simulation using a fallback buffer or database-empty-step scenario is claimed.

D13: FEMatch was started under gdb with fatal sanitizers, full backtrace and core
capture commands. The loader remained open while reporter tests ran, then the
same process loaded, simulated, exercised the bitmap and exported successfully.
No SIGABRT or sanitizer diagnostic reproduced. This shorter run does not explain
the earlier hours-old session failure. Its debugger remains attached for a
longer observation; D13 and overall sign-off remain open. The first debugger
startup log records a root-owned build-library permission problem, repaired
before the successful run; that is not a FEMatch defect.

No production sources or seed database records were changed for this evidence.
D14 and the listed review follow-ups have positive Linux evidence. M12's two
checker defects, D13, and any outstanding wider-platform requirements prevent
checking all four overall completion boxes.
