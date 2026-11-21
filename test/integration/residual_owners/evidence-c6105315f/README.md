# Linux real-owner verification after c6105315f

Verification found and repaired five additional production defects, D07–D11,
and the D12 test-fixture initialization defects in #628.
Final Linux production revision: `043d83ea962c64b780b55cb8c9925a5dca3ad878`.
The initial frozen archive was `2ddf75d6e` (production-identical to c6105315f),
then the committed delta was applied. `verified-source-manifest.json` records
all changed-file hashes with zero mismatches. CMake generated files and the
Linux-only untracked `local.pri` are build configuration, not source changes.

## Environment and isolation

Oracle Linux 8.10 aarch64, GCC 8.5, Qt 5.15.3, Qwt 6.1.5, Parallels VM
`usiab-ol8-arm64-dev`. Native Qt X11 windows ran on a dedicated Xvfb display
`:43` with xfwm4 at 1600×1000, operated through VM keyboard/mouse automation.
This exercises a Linux window manager but is not a physical-display/GPU test.
Apps run as `us3`, with `US3_SETTINGS_ROOT` under the frozen verification root
`/tmp/us3-residual-2ddf75d6e`. No personal settings or production database is used.
The fixture is the previously deployed test-only workflow 1240001 in
`uslims3_test`: edit/raw 1015, model 1037, TI/RI noise 167/168, triple 1.A.123.
The expected missing 2DSA-MC/PCSA notice makes this a **custom, non-GMP report**;
this is not a validated full GMP workflow or instrument-acquisition test.

CMake apps and test binaries use ASan+UBSan with frame pointers.
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1` apply to the real owner runs.
Leak detection is disabled; no leak-detector pass is claimed.

## Results

| Check | Result and evidence |
| --- | --- |
| Seven Linux CMake consumers | PASS; `build.log.gz`, `final-build.log.gz`, D11 incremental build logs |
| Seven native Linux launches | PASS; `launch-evidence/results.json`, binary hashes, ldd, logs and screenshots. Each remained alive with a visible window; no missing libraries or sanitizer diagnostics. Project variants waited 45 seconds. |
| Full qmake utils and gui | PASS; `qmake-libraries.log.gz`. Release, separate from the sanitizer CMake binaries; same production source. |
| Residual characterization | 123 passed, 0 failed, 1 intentional child-entry skip; `residual-run.log`. Includes 18 malformed-input cases and 100 lifetime cycles per mode. |
| C12/C13/D06/D07 focused utility regressions | 6 passed, 0 failed; `focused-utils-run.log`. D09 has its separate passing regression. |
| FEMatch real simulation/export | PASS under sanitizers; variance 7.81078e-05, RMSD 0.00883786. All 27 saved files match the recorded pre-T04 baseline (SVGZ compared decompressed); `fematch-comparison.json`, `fematch-sanitized-dialog.png`. |
| Actual autoflow overlay | PASS under sanitizers; same variance/RMSD, no Zero Target Speed warning; `autoflow-sanitized-overlay.png` and owner log. See entry-point limitation below. |
| Reporter normal run | PASS under sanitizers; normal warning threshold, no Zero Target Speed warning, two saved plots and a four-page PDF. `reporter-complete.png`, `reporter-normal.pdf`, `reporter-normal.log`. All four rendered PDF pages were visually inspected. |
| Reporter Abort/retry/Continue | PASS in the same process; see D05 procedure and screenshots below. Retry PNGs exactly match the normal-run PNGs (`d05-retry-image-comparison.json`). |
| Windows recheck after C12/new fixes | PASS; all seven consumers build/link at production revision 043d83ea9, Qt 6.10.1/MSVC x64. The new speedStepsCoverData DLL export is present. See windows-build-record.json, build/configure logs and windows-helper-export.txt. Build/link only, not Windows UI. |
| Full utility suite, fatal ASan/UBSan | 888 passed, 0 failed, 10 opt-in live-DB tests skipped (898 total); utils-final-strict.log.gz. Run as us3 with hardware-definition resources installed for the external wrapper. |

## Actual-owner entry points and comparisons

FEMatch used its real DB loader, selected edit 1015/model 1037, simulated,
opened Plot Residuals and used Save Data (Skip 3D). The final rerun after D07–D11
again produced variance 7.81078e-05 and RMSD 0.00883786. All 27 files match the
earlier captured candidate, which already matched the pre-T04 baseline; SVGZ
comparison decompresses only the gzip container. See fematch-final-dialog.png,
fematch-final.log and fematch-final-comparison.json. Save Data also stores these
artifacts in the isolated test database, as the UI completion notice states.

Autoflow's standalone window has no fixture-load UI. `autoflow-owner-driver.cpp`
uses the actual application's public `initPanel` and `show_overlay` slot with
workflow 1240001 and `2DSA-IT (1 / A / 123)`. It links the actual owner objects
and libraries, replacing only main; no owner/plot/model/simulation stand-ins are
used. The final rerun is recorded in autoflow-final-overlay.png and
autoflow-final.log: the same statistics and no sanitizer/zero-speed diagnostic.
It does not test traversal of the entire project workflow. Closing its
residual window also closed its newly opened bitmap while an independent
FEMatch bitmap remained open. The source and link-reuse script are retained.

Reporter used its actual UI: Load GMP Run → 1240001 → acknowledge expected
missing models → Unselect All → select corrected velocity and residual plots →
Generate Report. The PDF shows RMSD 0.00883786, matching FEMatch and autoflow.
The saved velocity SVG differs from FEMatch only in trailing title whitespace
and the resulting title text position; all other SVG content is identical.
The residual PNGs are **not byte-identical across owners**: FEMatch's export has
its own multi-line run/cell title, palette and dimensions. The report's residual
plot was inspected for data, labels and rendering; no cross-owner pixel-equivalence
claim is made. `reporter-fematch-comparison.json` preserves actual hashes.

There was no successful pre-T04 reporter PDF: the old run aborted at the warning
and incorrectly produced a report (D05). That artifact is not a scientific
baseline. This is the first successful reporter baseline, with the independent
FEMatch/statistics comparison and exact normal-versus-retry image comparison.
It must not be described as a historical successful-report before/after match.

## D05 repeatable native procedure

1. Preserve the normal run separately. Copy the isolated settings into
   `settings-abort`, change its work base to an empty `output-abort`, create its
   `reports` and `results` directories, and set the existing debug setting
   `debug_text=SetSpeedLowAcc=100000`. `prepare-abort.py` records this setup.
   No database rows, model, noise, or production scientific code are changed.
2. Launch the same reporter with that settings root, load the same fixture and
   select the same two plots. Generate Report. The real acceleration warning
   reports 402.7 rpm/s versus the deliberately raised 100000 rpm/s threshold.
3. Press Abort. The report directory is empty (no files/PDF), progress closes,
   no completion message appears, and Generate Report remains available.
   Evidence: `d05-warning.png`, `d05-after-abort.png`.
4. Without restarting, press Generate Report again. The warning appears again.
   Press Continue. The real simulation completes, the PDF is saved, and the
   completion message appears (`d05-after-continue.png`). No sanitizer diagnostic
   occurs. Both PNGs are byte-identical to the normal run.

The first Abort-profile attempt encountered a missing parent output directory;
that setup error was corrected before the recorded warning/Abort/retry sequence.

## Additional defects reproduced before their fixes

| ID | Failing evidence | Regression / fix commits |
| --- | --- | --- |
| D07 | Reporter protocol reset copies invalid bool; poisoned-storage test retains 0xAA. | 003cd7ed8 / 787320331; d07 logs |
| D08 | Actual analysis-profile constructor reads unset 2DSA changed flag. | 23966b2f0 / cbb3760e1; d08 probe logs; actual reporter load subsequently passes |
| D09 | Reporter copies ordinary model's unset custom-grid flags; poisoned-storage regression fails. | bcc0d7425 / 3df167252; d09 logs; actual reporter simulation subsequently passes |
| D10 | Real zero-value counter overflows its logarithmic width calculation; also reached by reporter pseudo-3D setup. | 13d932337 / e7d8c10db; d10 logs; zero/negative/fractional/positive probe passes |
| D11 | Project/academic/protocol initial resize uses offset before its later assignment. | 3d2ffa9e5 / 043d83ea9; launch-before.json and d11 traces; final seven-program sweep passes |

## Limits and remaining sign-off work

The original exploratory utility run (`utils-run.log.gz`) failed: running as root
invalidated the permissions check, and uninitialized test fixtures stopped UBSan.
D12 repairs only test fixtures in commits 9f15a19c7 and 7a8471d7b. The later
strict run as us3 passes 888 tests, with 10 opt-in live-DB tests skipped. The
cache diagnostic run used recovering UBSan to inventory fixture defects and is
**not** a pass. Its eight assertions caused by missing hardware XML resources
were resolved by providing the source etc directory in the external wrapper.
The final strict run uses fatal ASan/UBSan, with leak detection disabled.
D12 evidence: d12-before/after logs, d12-temperature-before.log.gz,
d12-cache-before.log.gz, d12-simparms-before.log.gz, utils-final-strict.log.gz.
Formatting after that run changes whitespace only; production remains 043d83ea9.
C13's edited plateau test passes; raw plateau remains separate issue #1245.

### D13 candidate: resumed FEMatch process aborted

During the final verification, the already-open FEMatch process (PID 116063)
aborted while resuming fixture loading. coredumpctl records SIGABRT, but no core
was retained and stderr contains no sanitizer stack or cause. Preserve
fematch-resumed-abort.log and fematch-resumed-abort-info.txt. A fresh process
loaded the same edit/model, simulated and saved all 27 matching artifacts
successfully. This is an unresolved observation, not a proven regression or an
excuse to classify the run as wholly clean. Reproduce under a debugger before
writing a fix; do not infer a database timeout or source defect from this log.

The current Windows build/export check is complete. Historical report comparison
and full-workflow limitations remain as above. The plan's M12 automated boundary
check with a deliberate negative test was not found in scripts/test; source and
link audits do not substitute for that planned enforcement test. Overall sign-off
therefore remains open for the D13 investigation and boundary-gate disposition.
The broad proposed Qt5/Qt6-every-platform matrix is not claimed by these runs.

`utils-tests.cmake` is an external test wrapper around the exact verified shared
libraries and repository tests, needed because the APP profile disables tests.
The wrapper, owner driver and launch scripts retain the frozen VM paths so the
commands and library selection can be reproduced. Neither replaces repository
CI or a full supported-platform acceptance run.
