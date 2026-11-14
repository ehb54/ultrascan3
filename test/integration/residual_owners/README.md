# Real-owner residual baseline verification

## Revision isolation

Use `27129cc57` for the pre-T04 baseline and `ca46728e9` for T04/T04a.
Do not build from the shared working tree while another agent is editing T05.
Local frozen sources are `build/ui-baseline-source` and `build/t04-source`,
created with `git archive`. Generated build headers may be added by CMake;
tracked source must continue to match the selected commit.

Linux CMake consumer target names differ from directory names:

| Directory | CMake target |
| --- | --- |
| us_reporter_gmp | us_reporter_gmp |
| us_com_project | us_com_project |
| us_com_project_academic | us_comproject_academic |
| us_protocol_dev | us_protocoldev |

All four targets built and linked on Linux at frozen revision `ca46728e9`.
`ldd` resolved all runtime libraries. Binary hashes and source verification
(5035 tracked files, zero mismatches) are in [t04-linux-build.json](t04-linux-build.json).
Detailed local logs are under `build/linux-gmp-review/evidence/`.

## Verified local database fixture (2026-11-14)

The deployed database differs from a freshly seeded database. Do not assume
these fitted results survive a reseed. No database records were changed during
this investigation.

- Database: `uslims3_test`; host port 3307 in `lims-db-1`.
- Docker client endpoint: `host.docker.internal:3307`.
- Oracle Linux VM endpoint: `10.211.55.2:3307` (verified from this VM).
- Run: `multicell-demo-ra`; triple `1.A.123`; experiment 1011.
- Edit/raw IDs: 1015/1015; model 1037; TI/RI noise 167/168; investigator 1.
- Application loaders verify 10 model components, 1255 radii, 30 scans,
  matching edit/model GUIDs and both noise/model GUIDs.
- Model XML SHA256: `effac4808731d67c45ddd229fe657fe861aa68b27dace9b7090dbb8eb861b786`.
- Edit blob SHA256: `e416420ee7e0f090760bbab91c99335df9d061f853f0f5f8e57ac531d0bdd19c`.
- TI XML SHA256: `bb1f44338b67e4e1fd41235aa159b24da8558a8f5f542dd5f215710ae3c57c18`.
- RI XML SHA256: `4776966e9786068b6efe9f904e5f50c4406fb1dfa33a3e93af53f5ee854c8089`.

Model 900 is NOT a usable edit-linked FEMatch fixture in this deployment: it
references edit 1 (`UNASSIGNED`, no raw data). It is a custom grid, not a fit.

`configure_test_profile.cpp` configures an isolated profile and verifies this
fixture through `US_DB2`, `US_Model`, `US_Noise`, and the actual reporter data
loader. It does not simulate, open a dialog, or establish report coverage.
It requires an absolute `US3_SETTINGS_ROOT`, restricts endpoints to the local
test deployment, and always selects `uslims3_test`. Credentials are the public
local seed credentials. The test-only master password is `residual-test-only`.
Do not use this helper with personal settings or production databases.

Example in the Linux Docker environment (from repository root):

```sh
g++ -std=c++14 -fPIC \
  test/integration/residual_owners/configure_test_profile.cpp \
  build/t04-source/programs/us_edited_data_loaders.cpp \
  -Ibuild/t04-source/utils -Ibuild/t04-source/programs -I/usr/include/mysql \
  $(pkg-config --cflags Qt5Core Qt5Xml Qt5Network) \
  -Lbuild/linux-gmp-review/lib -lus_utils4 \
  $(pkg-config --libs Qt5Core Qt5Xml Qt5Network) \
  -Wl,-rpath,/src/build/linux-gmp-review/lib \
  -o build/linux-gmp-review/bin/configure_test_profile
US3_SETTINGS_ROOT=/src/build/linux-gmp-review/test-profile \
  build/linux-gmp-review/bin/configure_test_profile host.docker.internal:3307
```

The VM profile is installed at
`/tmp/us3-residual-baseline/settings/UltraScan3/UltraScan.ini`, owned by `us3`.
Its host is `10.211.55.2:3307`. Full application authentication and fixture loading
were verified in both Docker and the VM. Run the VM verifier from
`/tmp/us3-residual-baseline/build/bin/configure_test_profile`: the executable
must resolve `../etc/mysql` for the packaged TLS resources. Running it from the
parent directory failed with `no cipher match`; moving it into the application
layout resolved that failure without changing TLS defaults. The VM profile is
not yet verified through a native dialog. License and output-directory setup
must be checked before the UI run; personal license data must not be committed.

## FEMatch native capture procedure (not yet executed)

1. Build/install the frozen baseline natively for the VM. Record commit, binary
   hash, Qt/Qwt versions, display resolution/scaling and platform. The existing
   `/opt/ultrascan3` installation is not evidence of the baseline revision.
   The baseline has now built successfully at
   `/tmp/us3-residual-baseline/build/bin/us_fematch` on Oracle Linux 8.10,
   using Qt 5.15.3 and Qwt 6.1.5. Source archive SHA256:
   `4a690038489260d09647bb96b1dff7e0b062ca923d2b2abfed5ac0e5479db22c`.
2. Launch that FEMatch binary as the desktop test user with the isolated
   `US3_SETTINGS_ROOT`. Use a dedicated output directory and record its path.
3. Confirm Database is selected, authenticate with the test-only master password,
   load `multicell-demo-ra`, and select triple `1.A.123` / edit 1015.
4. Load model 1037 by its full description:
   `multicell-demo-ra.1A123.e2608180000_a2609300625_2DSA-IT_000206_i01.model`.
   Confirm the associated noise selection is 167/168. Do not refit the model.
5. Simulate, open Plot Residuals, and capture both plots, displayed statistics,
   control states and bitmap. Exercise window close and the Close button
   separately, recording the baseline's known lifetime defects.
6. Use FEMatch Save Data to export its actual images. Inventory every output,
   record SHA256 hashes and capture the dialog. Preserve raw images for visual
   comparison, rather than comparing only compressed image hashes.
7. Repeat with the frozen candidate, identical inputs/noise/settings/display and
   separate output directory. Compare numerical/curve output exactly where
   appropriate; classify intentional D01/D02 changes separately.

## GMP fixture gaps and UI blocker

Reporter/autoflow cannot yet be called end-to-end using this fixture. The test
DB has zero protocol rows and zero `autoflowReport` rows. All five extant
analysis-history rows are failed jobs. Model 1037 is in `autoflowModelsLink`
for analysis 98/workflow 81, but that workflow and its history are absent.
Do not mark those orphaned links as a completed run or reuse unrelated validation
protocols with different wavelength/speed/solution data.

A complete additive fixture needs a matching protocol, analysis profile/report
configuration, workflow, stage/history rows and model linkage for the selected
triple. Preserve the original fitted model and noise. Verify the application's
stored-procedure reads before UI capture; SQL row presence alone is insufficient.

Native desktop automation currently fails with `Sky Computer Use native pipe
startup failed`. The VM has XFCE and active xrdp/xrdp-sesman services, but no native
UI capture has been performed. This is separate from DB/network readiness.
