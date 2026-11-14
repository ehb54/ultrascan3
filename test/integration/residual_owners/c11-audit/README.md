# C11 initialization audit

Audited revision: `737c7078f7ce05d6fc0da1c1a99e2b266fe3ea86` (production sources unchanged since `fff4656f6`). This is an audit plus a focused helper reproducer; no production repair or full native sanitizer run is claimed.

## Owner paths

| Owner | Successful edited-data path | Simulation input |
|---|---|---|
| FEMatch, disk and DB | `US_DataLoader::load_edit` (`gui/us_data_loader.cpp:443,601`) -> `US_DataIO::loadData` | `US_AstfemMath::initSimData` (`programs/us_fematch/us_fematch.cpp:2135`) |
| RMSD query -> FEMatch `auto_load_simulate` | `us_query_rmsd.cpp:329` -> `US_DataIO::loadData`; cached objects originate there; passed by value at `:626` | Same FEMatch initializer |
| Autoflow | `US_EditedDataLoaders::loadAutoflow` -> `US_ConvertIO::readEditedDataFromDB` -> `US_DataIO::loadData` | `us_autoflow_analysis.cpp:4071` |
| Reporter | `US_EditedDataLoaders::loadReporter` -> same conversion/loader path | `us_reporter_gmp.cpp:6248` |

The DB conversion helper publishes its local loaded vectors only after successful decode. The GUI loader and RMSD route also use the common decoder. This is the residual owners' provenance audit, not an assertion about every construction in the repository.

## Field coverage

- `EditedData` has no user-provided constructor. Default-initialization alone does not initialize its scalar fields. This fact does not by itself establish a loaded-owner defect.
- `EditValues()` (`utils/us_dataIO.h:157`) initializes `floatingData=false`, all scalar edit values consumed by this loader, and both baseline-correction coefficients. XML may replace these defaults.
- `US_DataIO::loadData` (`utils/us_dataIO.cpp:1407-1429`) assigns all eight top-level `EditedData` scalars: meniscus, bottom, plateau, baseline, ODlimit, floatingData, bl_corr_slope and bl_corr_yintercept. QString/container members construct normally. Thus the harness's uninitialized `floatingData` failure is not reproduced by this successful loader path.
- `readRawData` initializes raw header fields and then reads them from the file (`:571-574,604-621`). Its scans receive temperature, rpm, seconds, omega2t, wavelength, delta_r and nz_stddev. **Scan plateau is not assigned before `rd.scanData << sc` (`:702-802`).**
- `copyRange` assigns edited scan metadata including nz_stddev (`:1703-1712`), but not plateau. `loadData` appends that scan first (`:1471-1475`) and assigns plateau later (`:1481-1485`). Thus final edited scans have plateau, but the preceding copies warrant a separate candidate investigation (C13). No runtime plateau failure or output corruption is claimed.
- `US_AstfemMath::initSimData` (`utils/us_astfem_math.cpp:2560-2605`) initializes raw header fields and copies every scan scalar **except nz_stddev**. `sscan.stddevs` is an empty constructed vector; the boolean is left indeterminate before assigning `sscan` to the simulation vector. This path is shared by all three residual owners and predates T04/T05.
- The residual input snapshot (`gui/us_resplot_fem.cpp:214`) copies values; it is not an initialization repair. Qt container sharing can defer individual scan copies, so a successful dialog test is not a general proof of scalar initialization.

## D06 focused reproduction

`probe.py` extracts the exact production `initSimData` function and the two trivial count methods, using production headers. It supplies fully value-initialized experimental inputs, then reads the generated simulated scan's `nz_stddev`. Clang 18, Qt 6.10.1, macOS arm64, `-O0 -fsanitize=undefined -fno-sanitize-recover=undefined -ftrivial-auto-var-init=pattern`:

- Unmodified helper: UBSan reports `load of value 170, which is not a valid value for type 'bool'`; nonzero exit.
- Isolated control: add only `sscan.nz_stddev = false` before the copy; exit 0.

Pattern initialization makes missing scalar writes deterministic. This is a targeted helper-level reproducer, **not** a claim that the actual owner UI has failed under sanitizers. It confirms D06's initializer omission. The control is generated only under `build/c11-audit`; production code is unchanged. Before repair, add an integrated regression for initialized simulated scan metadata, choose/document the no-standard-deviation policy, then rerun the owner simulation path under sanitizers. A blanket constructor change is not justified by this audit alone.

Reproduce from the repository root with `CXX` set to clang++ 18 and `PKG_CONFIG_PATH` set to the Qt6 pkg-config directory:

```sh
python3 test/integration/residual_owners/c11-audit/probe.py
```

Generated sources, exact compile commands and logs go to `build/c11-audit`. Captured results are included here. The script succeeds only if the original helper produces the expected UBSan diagnostic and the isolated control succeeds.

## Completion status

C11's owner initialization audit is complete, with D06 and C13 explicitly retained as follow-up tasks. C04's Linux CMake and Windows consumer-link requirements passed in the preceding verification record. The four issue-level completion boxes remain unchecked: reporter/autoflow native/report comparisons and outstanding defects still prevent full sign-off.
