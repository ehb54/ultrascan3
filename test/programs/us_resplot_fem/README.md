# Residual plot dialog tests

These tests link `US_ResidPlotFem` from the GUI library (`gui/us_resplot_fem.cpp`)
and construct it from `US_ResidPlotInputs` and a `US_ResidPlotFem::Mode`, the
same way FEMatch, autoflow analysis and the GMP reporter do. No application code
is linked; the caller in each test is a plain `QWidget` passed as the parent.

## Coverage

- All 60 combinations of the three modes, four TI/RI noise states and five scan
  exclusion patterns (none, first, middle, last, all): every curve and sample,
  scan order, zero line, displayed variance/RMSD, labels, visibility and sizing.
- All 16 add/subtract noise combinations in each mode, plus data/model
  visibility and enabled controls.
- TI, RI, random-noise and residual view selection, and RI export keeping its
  scan-number axis label.
- Bitmap close/reopen and parenting, window close versus the Close button,
  the `on_close`/`destroyed` signals, and destroying the caller.
- The dialog keeps its own copy of the inputs; later caller changes are not
  seen until `set_inputs`.
- 100 open/close cycles per mode, each closed dialog destroyed.
- Empty and mismatched input buffers, each run in a child process so an
  out-of-bounds read fails one row instead of the whole suite.

With every scan excluded the dialog displays NaN statistics, and the tests
assert that existing behavior.

Not covered: real workflow widgets, the database, simulation, report export and
a native window manager.

## Run

```sh
cmake --build <build> --target test_us_resplot_fem
ctest --test-dir <build> -R '^test_us_resplot_fem$' -V
```

The test helper sets up offscreen Qt and sandboxed settings; no license, live
database or extra fixtures are needed.
