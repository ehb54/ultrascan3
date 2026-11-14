# Native owner verification, 2026-11-14

Baseline: `27129cc57`. Completed T05 candidate: `91501a654`.
Oracle Linux 8.10 aarch64, Qt 5.15.3, Qwt 6.1.5, Xvfb/XFWM4 at 1600x1000.
Final T10 rerun uses frozen `fff4656f63207fab7167953a19f0ac3f56c0696a`.
The subsequent `b13d492fb` changes only formatting/header of the test profile helper.

## FEMatch: saved-output comparison passes

Loaded the local test database run multicell-demo-ra, triple 1/A/123, edit 1015,
model 1037 (filter `2609300625`), with its TI/RI noises 167/168. Clicked Simulate
Model. Both real dialogs show variance 7.81078e-05 and RMSD 0.00883786.
Closed the residual dialog and 3D controls, selected Save Data, and selected Skip
for the 3D image. Save Data also writes its report files to uslims3_test.
The initial attempt lacked the parent reports directory; creating it resolved
the file-error prompt. Prepare output directories before subsequent runs.

27 exported files compared: 16 byte-identical (including all PNG plots, CSV and
both HTML reports); 11 SVGZ files have identical decompressed SVG data. The raw
archives and exports are under local build/real-owner-verification, and hashes
are in fematch-file-comparison.json. No unexplained exported-data difference.

Candidate native interactions: Close button then Save recreates the dialog;
title-bar close removes the visible dialog and Residual Plot reopens it;
closing the bitmap clears its checkbox; recreating that bitmap and closing the
dialog also closes the new bitmap. These do not establish heap/leak guarantees
or substitute for the sanitizer suite.

Registration uses the existing test helper from ultrascan3-cli/test/common/
us3_dev_license.h. A temporary Linux helper compiled with INTEL/LINUX writes the
helper's registration into US_Settings under each isolated US3_SETTINGS_ROOT;
US_License_t::isValid returns 0. No application license check was modified.

## Reporter: verification fails before a valid simulated report

Synthetic, explicitly labelled test-only workflow 1240001 was added to
uslims3_test, with matching protocol/profile/report and analysis-history/model
link rows. Existing model/edit/noise records were not modified. SQL is preserved
in fixture/deploy.sql; it is a one-time transaction, not an idempotent seeder.
The first transaction failed on empty-string encoding and rolled back; the
corrected transaction succeeded. Do not rerun against occupied IDs.
Authenticated get_modelAnalysisInfo and read_autoflow_history_record return
status 0 and the expected 1037/1015 model/edit linkage. Both real reporters load
it. Only 2DSA-IT is supplied; the reporter announces a custom/non-GMP report due
to missing MC/PCSA models. This fixture does not represent a completed GMP run.

Reproduction in both frozen revisions:
1. Load GMP Run, select workflow 1240001, acknowledge the custom-report notice.
2. Unselect All Tree Items; select only the noise-corrected velocity overlay and
   experiment-simulation residuals under 1A / 123 / 2DSA-IT / Plots.
3. Generate Report: a Zero Target Speed warning appears.
4. Select Abort: the reporter nevertheless writes a PDF and announces Report
   Generation Complete. Those outputs are rejected as successful baselines.

D05: Abort is not propagated from simulateModel (void) through simulate_triple
and report generation. Add a failing owner-level cancellation test before fixing.
C12: reporter and autoflow exp_steps/dat_steps fields have no initialization or
assignments in the frozen sources, but are read for speed selection. The fixture
has a 45000 rpm speedstep and no timestate record/file. Establish deterministic
speed initialization with a focused failing test; the precise causal path to the
warning still needs instrumented verification. Do not treat this as a T05 regression.

Autoflow real overlay and successful reporter before/after images remain open.
Final T10 seven-consumer builds/launches, qmake libraries, Windows exports and
FEMatch rerun remain separate checks. D01-D04 are not fully signed off here.

## Final T10 verification

At frozen fff4656f6 all seven CMake targets build and link on Oracle Linux 8.10
(aarch64, GCC 8.5, Qt 5.15.3, Qwt 6.1.5). Full qmake builds of utils and gui
also pass. The external qwtplot3d library is supplied from the CMake build;
the qmake test invocation uses -lqwtplot3d (not -lqwtplot3d4). No production
source was edited for this environment adjustment.

Qmake library SHA256:
- libus_utils.so.10.0.0: be4f3b9d83a4f5aa3d5889366b03c8311f0db9f8ed0e6716b757ca7ec70e4e42
- libus_gui.so.10.0.0: b7bf07c964d140f088e03623dd808b903c8a42d72ee3012272240e4bfedfb801

The real FEMatch rerun uses the same edit/model/noises and isolated profile.
Its variance and RMSD match; all 27 saved files match the baseline after SVGZ
decompression. See t10-fematch-file-comparison.json and t10-fematch-dialog.png.
The native RI-noise check shows Scan Number (t10-ri-noise-axis.png).

The test helper formatting and file header are committed as b13d492fb.
The full branch style check against 29e9a70995a7727c718c90a2334fe711f09342d6
passes using clang-format 18. PR base remains calc-library-extraction;
this local check does not imply the main-only style CI ran.

All seven Linux programs remain alive and show their application windows.
The project programs need a longer startup allowance (45 seconds); both show
the expected missing Optima instrument license/server warning in this VM.
This is a launch check, not instrument-operation coverage. See launch-evidence/
results.json plus project-results.json (which supersedes the two initial
6-second observations), with executable hashes, ldd output and screenshots.

Windows build is still running; no Windows pass is claimed. Reporter/autoflow verification remains incomplete for the
reasons above; D01-D04 are not closed by the FEMatch-only comparison.
