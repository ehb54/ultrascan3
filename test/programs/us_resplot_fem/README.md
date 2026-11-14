# Residual dialog characterization before extraction

This harness runs the unchanged `US_ResidPlotFem` implementation and real
Qt/Qwt/bitmap code. It makes no production edits. The temporary compilation seam
in `legacy_dialog.cpp` bypasses the three application header guards and supplies
concrete QWidget owner doubles with the legacy accessors. It does not copy or
replace plotting code. `owner_contract.cpp` compiles the real application headers
and checks accessor signatures in the compile-only `test_residual_owner_contract`
target. Its object is never linked into the executable containing owner doubles.
Never link application implementations into the characterization executable.

This seam is deliberately temporary: after the explicit-input refactor, replace
the doubles/wrapper with direct dialog construction using the input contract.
Keep the behavioral fixtures/assertions. Do not move the seam into production.

## Coverage

- All 60 combinations of three modes, four TI/RI noise states and five exclusion
  patterns (none, first, middle, last, all). Checks every curve/sample, scan order,
  zero line, displayed variance/RMSD, labels, visibility and mode-specific sizing.
- All 16 add/subtract noise-control combinations, repeated twice per mode, plus
  data/model visibility and enabled controls. Binary-exact sample differences and
  noise allow exact curve comparison. Displayed statistics use existing formatting.
- TI, RI, random-noise and residual view selection; exact noise coordinates and
  mutual exclusion. Includes current RI export-axis relabeling behavior.
- Three-mode bitmap close/reopen, bitmap parenting, window close versus Close
  button slot, `on_close`/`destroyed` signals, caller destruction and explicit
  dialog deletion. A caller double borrows fixture storage, so this does NOT
  exercise callbacks into destroyed application-owned data.
- Existing FEMatch bitmap reuse and invalidation; borrowed data/exclusions mutate
  in place and the next refresh uses the new values.
- 100 close/recreate cycles per mode: current dialogs remain alive after close;
  test cleanup explicitly deletes them. This characterizes retention, not leak-free
  application behavior or sanitizer coverage.

Known legacy results intentionally asserted: all scans excluded displays NaN;
close hides rather than destroys dialogs; window close does not destroy the
bitmap; REPORT bitmap-checkbox changes wait for another refresh; the lower-plot
export accessor changes the RI axis label from Scan Number to Radius (cm).
These assertions preserve evidence, not endorsement. Defect repair must introduce
an intended-behavior regression test that fails before changing production code,
with a linked defect ID and explicit change to the characterization expectation.

## Limits

No real workflow widgets, database, simulation, report-file export or native
window manager are exercised. No invalid-pointer/empty/mismatched buffer cases
are run yet; those need isolated-process tests. CMake's test label is
`residual-characterization`; this is not the isolated presentation boundary gate
(M12) because owner doubles are still necessary. M02 is covered at the dialog
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
