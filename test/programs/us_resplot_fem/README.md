# Residual dialog characterization before extraction

This harness compiles `programs/us_fematch/us_resplot_fem.cpp` directly and
constructs the dialog from `US_ResidPlotInputs` and a `US_ResidPlotFem::Mode`,
exactly as FEMatch, autoflow and the reporter do. It runs the real Qt/Qwt/bitmap
code and links no application implementation: the dialog no longer includes or
casts to `US_FeMatch`, `US_Analysis_auto` or `US_ReporterGMP`. The "caller" in
these tests is a plain `QWidget` passed as the parent argument.

The assertions are those recorded against the owner-discovery baseline
(`36f4464d5`, before T04); they pass unchanged against the explicit-input
dialog. Mode strings in the test data map to the enum through `toMode()`.

## Coverage

- All 60 combinations of three modes, four TI/RI noise states and five exclusion
  patterns (none, first, middle, last, all). Checks every curve/sample, scan order,
  zero line, displayed variance/RMSD, labels, visibility and mode-specific sizing.
- All 16 add/subtract noise-control combinations, repeated twice per mode, plus
  data/model visibility and enabled controls. Binary-exact sample differences and
  noise allow exact curve comparison. Displayed statistics use existing formatting.
- TI, RI, random-noise and residual view selection; exact noise coordinates and
  mutual exclusion. D03 now verifies RI export preserves the scan-number label.
- Three-mode bitmap close/reopen, bitmap parenting, window close versus Close
  button slot, `on_close`/`destroyed` signals and caller destruction. The inputs
  borrow fixture storage, so this does NOT exercise callbacks into destroyed
  application-owned data.
- Existing FEMatch bitmap reuse and invalidation; borrowed data/exclusions mutate
  in place and the next refresh uses the new values.
- 100 open/close cycles per mode: every closed dialog is destroyed.

Known legacy results intentionally asserted: all scans excluded displays NaN;
REPORT bitmap-checkbox changes wait for another refresh.
These assertions preserve evidence, not endorsement. Defect repair must introduce
an intended-behavior regression test that fails before changing production code,
with a linked defect ID and explicit change to the characterization expectation.

## Limits

No real workflow widgets, database, simulation, report-file export or native
window manager are exercised. Invalid-pointer/empty/mismatched buffer cases
run in isolated processes under sanitizers; expected legacy failures are not
safe-input handling guarantees. CMake's test label is
`residual-characterization`. The executable links the dialog without any
application implementation, which is the isolated-presentation part of M12; the
automated boundary check with a negative test is still to be added. M02 is covered at the dialog
boundary, with portions of M03/M07; full T02 acceptance remains open.

## Run

```sh
cmake --build <build> --target test_us_resplot_fem
ctest --test-dir <build> -R '^test_us_resplot_fem$' -V
```

The usual test helper configures offscreen Qt and sandboxed settings. No private
license, live database or regenerated scientific fixtures are required.

## Recorded first baseline (2026-11-13)

Production baseline `36f4464d5`, macOS arm64 Release, Qt 6.10.1/Qwt 6.3.0:
75 QtTest passes (73 behavior rows plus setup/cleanup), no failures/skips.
The 52 calc CTest entries still pass, as does the loader test (11 QtTest passes,
three opt-in live-DB skips). Qt5, other platforms, native UI and sanitizer runs
have not been performed for this harness.

Two deliberate mutations were detected before restoring the source:

- Changing TI subtraction to addition in residuals: the FEMATCH/TI/no-exclusions
  row failed on the first residual, actual 0.375 versus expected 0.125.
- Suppressing `on_close`: the FEMatch close/reopen row failed, zero close signals
  versus one expected.

Restored production source SHA-256:
`717e6c310e0cec7498f0206cd41f17e4a223a09d4a7256937cccf8db83011548`.
Detailed local evidence is under `build/baseline-qt6/evidence/`:
`residual-run.log`, `residual-results.xml`, `mutation-results.json`, mutation
build/run logs, `residual-calc-*`, `residual-loader-*`, and source hashes.
These ignored build artifacts must be archived with the PR before closing gates.

Confirmed defects are tracked as D01–D03 in
[ultrascan-tickets#1240](https://github.com/ehb54/ultrascan-tickets/issues/1240).
The CI change filters now include the three application directories and the
bitmap/dialog base files so source-only changes trigger both Qt test workflows.

### D03 red/green correction

The intended `Scan Number` label assertion failed in all three modes before
the fix. `rp_data_plot2()` now reads the lower plot's own x-axis title rather
than copying the upper plot's title. The full 75-check suite passes after this
one-line correction. Local evidence: `d03-red-*` and `d03-green-*` logs. This
intentionally corrects the first baseline's mislabeled RI-noise export axis;
curve coordinates and numerical calculations are unchanged.

### Linux sanitizer baseline (2026-11-13)

Revision `dc7fb11a6c977341e127d8214ab61643f929d29e` plus the M04 tests
committed alongside this record: Linux aarch64, GCC 11.4, Qt 5.15.3, Qwt 6.1.4,
Debug with ASan/UBSan in Docker (`us3-tsan-review:latest`), Qt offscreen.
Result: **105 passed, 0 failed, 1 skipped**. The skip is the child-process
entry point when invoked without probe parameters. All 30 input probes ran,
including 24 expected unsafe legacy failures, plus 100 lifetime cycles per mode.
Leak detection was disabled; this does not establish leak-free owner behavior.

Local evidence in `build/linux-sanitize-make/evidence/`: `residual-run.log`,
individual `probes/*.log`, configure/build/rebuild logs,
`baseline-manifest.json` (revision and source/binary hashes), and
`baseline-working-tree.patch`. A committed copy of the run, all child-probe logs
and source/binary hash manifest is in [baseline/linux-2026-11-13](baseline/linux-2026-11-13).
Native UI/export captures and the Linux seven-consumer build matrix
remain outstanding; this is the automated dialog baseline, not full T02 signoff.

### Explicit-input construction (T04)

The dialog now takes `US_ResidPlotInputs` and a `Mode` enum instead of casting
its parent to the calling program, so two probe scenarios lost their meaning:

- `unknown-mode` rows are removed. A mode outside `Standalone`, `Analysis` and
  `Report` can no longer be expressed, so the uninitialized-input case they
  reproduced (C05) cannot occur.
- `null-parent` rows now expect normal completion. The inputs no longer come
  from the parent, so a null parent plots the same data as any other.

The malformed-buffer probes (empty, short and mismatched data) are unchanged and
still characterize the legacy unsafe behavior; repairing them is separate from
this extraction. Every other row keeps its recorded assertion.

Compared with the Linux baseline at `27129cc57`: all behavior rows pass with the
same results; the three `unknown-mode` rows are absent; the 18 malformed-buffer
probes are skipped on non-sanitizer builds and must be re-run under ASan/UBSan.

### D01: the caller owns the dialog (T05)

The caller is now the dialog's Qt parent and the dialog deletes itself when
closed. Four tests state the intended behavior and failed in every mode before
the fix (72 failures, no arithmetic or plot rows among them):
`closeDestroysDialog`, `closeButtonDestroysDialogAndBitmap`,
`callerDestructionDestroysDialog` and `repeatedCloseLeavesNoDialogs`. They
replace the retention assertions of `closeAndReopen` and
`repeatedCloseRetainsDialogsUntilExplicitDeletion`, and `curvesAndStatistics`
now expects the caller as parent.

The fix also exposed a crash: a child bitmap (Analysis mode) emitted `destroyed`
into `resids_closed` while the dialog was being destroyed.
`closeDestroysDialog(analysis)` crashed until the destructor disconnected the
bitmap.

### D02: a created bitmap closes with its dialog (T05)

A bitmap the dialog creates is now its child (`Qt::Dialog` window) in every
mode, not only Analysis, so a window close destroys it along with the dialog.
`closeDestroysDialog` now expects that in all three modes; FEMatch and Report
failed before the fix. A bitmap passed in by the caller (FEMatch's) is still
the caller's: `borrowedBitmapAndRefresh` is unchanged.
