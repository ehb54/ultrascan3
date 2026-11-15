# D13 restart and reload stress verification

Tested production binary: `a9793593f`, SHA-256
`f26c8590a3f881d54872a13b8d57f648dbdb4d30be67114d2af839e87b7eea59`.
The later dependency-checker-only commit does not change this binary.
Oracle Linux 8.10 aarch64, Qt 5.15.3, GCC 8.5; native Qt windows in an
isolated Xvfb/xfwm display `:44`. Same real database experiment/model as the
previous FEMatch baseline (multicell-demo-ra, model 1037). Separate settings
profile; no seed edits or report-save operations in this stress sequence.

## Completed sequence

Ten fresh process cycles (record IDs 04–13), each under gdb with fatal ASan
and UBSan settings:

1. Start FEMatch.
2. Load the experiment from the test database and load the fitted model.
3. Simulate; wait for the real residual dialog and capture it.
4. Open and close the residual bitmap; close the dialog.
5. Repeat steps 2–4 in the same process.
6. Use FEMatch's Close button; require gdb to report `exited normally` and no
   signal or sanitizer diagnostic.

The older process (125271, previously observed alive at 43m48s) was separately
resumed on display `:43`, reloaded and simulated three more times, and closed
normally. `resumed-gdb.log` is its final debugger output; the debugger is no
longer attached because that process has now exited normally.

Totals and individual results are in `summary.json` and `cycle-*.json`.
Every measured restart cycle completed both loads/simulations and a normal exit.
No D13 crash was reproduced. Leak detection was disabled; this is not an LSan
pass or a proof that the historical SIGABRT cannot recur.

## Evidence and automation limitations

`d13-stress.py` and `d13-resume.py` are the exact UI drivers. The measured
restart driver scopes window searches by process ID and computes click positions
relative to the actual X11 client-window origin. Each wait is bounded and a
failed operation stops the run. All measured debugger logs and result JSON are
retained, with representative first/last/resumed screenshots. All screenshots
remain locally in `build/real-owner-verification/d13-stress/d13-stress/` and their
hashes are recorded in `summary.json`.

Record IDs 01–03 were automation calibration, not measured passes: window-frame
coordinates and an unscoped window search initially targeted the wrong controls
or a leftover calibration window. IDs 01 and 02 timed out; ID 03 was stopped.
Their logs are retained, and they are excluded from the totals. Calibration
processes on display :44 were terminated before measured cycle 04 began.

Gdb returns status 1 even after a normal inferior exit because its final
`generate-core-file` command has no remaining process. Each passing result
requires the explicit `exited normally` line and absence of sanitizer/signal
markers; it does not reinterpret an unexplained nonzero application exit as a
pass. If a real signal stops the inferior, the same commands retain the full
backtrace and core before the driver stops.

## Disposition

D13 remains an unresolved, non-reproduced observation. The historical log
confirms SIGABRT but contains neither its cause nor a retained stack/core.
These bounded tests add negative reproduction evidence, not a root cause or fix.
Do not write a speculative production change. Keep a tracked follow-up with
these artifacts and debugger/core-capture instructions. Whether this unproven
candidate must block #628 is an explicit review decision; this record does not
waive it or tick the overall no-blockers box.
