# us_mpi_analysis regression tests

End-to-end tests that run `us_mpi_analysis` under `mpirun` on synthetic data
and compare the fitted models with stored baselines.

## Contents

| File | Purpose |
|---|---|
| `us_mpi_testdata.cpp` | Writes a simulated velocity dataset and a custom-grid model with the UltraScan writers (`US_DataIO::writeRawData`, `US_DataIO::writeEdits`, `US_Model::write`). Three species, 30 scans, 60 krpm, edited bottom at the stretched centerpiece bottom as `us_edit` writes it, noise σ = 0.004 from the seeded `US_Math2` generator. |
| `us_mpi_regression.py` | Builds the job archive (hpcrequest XML in the LIMS format), runs the case, reduces `analysis-results.tar` to a JSON summary and compares it with `baseline/<case>.json`. |
| `baseline/*.json` | Reference summaries: per model the RMSD, the signal-weighted moments of s, f/f0 and vbar, the solute list, and any TI/RI noise vectors. |

## Cases

| Case | Path exercised | Ranks | Comparison |
|---|---|---|---|
| `2dsa_basic` | `_2dsa_master`, uniform grid, 9 subgrids | 2 | exact |
| `2dsa_refine` | iterative refinement (3) | 2 | exact |
| `2dsa_mc` | Monte Carlo (3) | 2 | exact |
| `2dsa_meniscus` | meniscus fit (3 points) | 2 | exact |
| `2dsa_bottom` | bottom fit (3 points) | 2 | exact |
| `2dsa_menbot` | meniscus and bottom fit (3 × 3) | 2 | exact |
| `2dsa_composite_menbot` | composite job, meniscus and bottom fit (2 × 2) | 2 | exact |
| `2dsa_composite_menbot_pmasters` | the same with parallel masters | 6 | exact |
| `2dsa_noise` | TI + RI noise | 2 | exact |
| `2dsa_customgrid` | custom grid (s × vbar) | 2 | exact |
| `2dsa_global` | global fit, 2 datasets | 2 | exact |
| `2dsa_composite` | composite job, 2 datasets | 2 | exact |
| `2dsa_composite_meniscus` | composite job with meniscus fit | 2 | exact |
| `2dsa_refine_np4`, `2dsa_mc_np4` | several workers | 4 | exact |
| `2dsa_mc_pmasters` | parallel masters, MC (2 groups) | 6 | exact |
| `2dsa_mc_pmasters3` | parallel MC, 3 groups, 4 iterations | 9 | exact |
| `2dsa_mc_pmasters_idle` | parallel MC, 3 groups, 2 iterations | 9 | exact |
| `2dsa_composite_pmasters` | parallel masters, composite (2 groups) | 6 | exact |
| `2dsa_composite_meniscus_pmasters` | parallel composite with meniscus fit | 6 | exact |
| `pcsa_sl` | PCSA straight lines | 2 | exact |
| `ga_basic` | GA, 3 buckets | 4 | loose |

"Exact" means RMSD within 1e-6 relative, moments within 1e-4 relative and
noise within 1e-6 absolute.  The 2DSA master merges subgrid results in job
order and seeds the noise of each Monte Carlo iteration from the job seed
and the iteration, so a fit gives the same result for any number of workers
or master groups; the `*_np4` and `*_pmasters` cases equal the single-worker
results.  GA migrates genes asynchronously, so its result depends on message
timing; "loose" only requires the same output files and an RMSD not more
than 15 % above the baseline.

## Running

The tests need a build with MPI, programs and tests, without the database:

```bash
cmake -S . -B build -G Ninja -DUS3_NO_DB=ON -DUS3_BUILD_PROGRAMS=ON \
      -DBUILD_TESTING=ON -DUS3_PREFER_STATIC=ON -DBUILD_DOCUMENTATION=OFF
cmake --build build
ctest --test-dir build -L mpi --output-on-failure
```

Single cases can be run directly:

```bash
python3 test/mpi_analysis/us_mpi_regression.py check 2dsa_mc \
    --program build/bin/us_mpi_analysis --testdata build/bin/us_mpi_testdata
```

## Updating baselines

Only when a change of results is intended:

```bash
python3 test/mpi_analysis/us_mpi_regression.py update all \
    --program build/bin/us_mpi_analysis --testdata build/bin/us_mpi_testdata
```

Review the JSON diff before committing.
