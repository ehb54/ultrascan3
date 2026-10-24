# us_mpi_analysis regression tests

End-to-end tests that run `us_mpi_analysis` under `mpirun` on synthetic data
and compare the fitted models with stored baselines.

## Contents

| File | Purpose |
|---|---|
| `us_mpi_testdata.cpp` | Writes a simulated velocity dataset, a DMGA constraints model and a custom-grid model with the UltraScan writers (`US_DataIO::writeRawData`, `US_DataIO::writeEdits`, `US_Model::write`). Three species, 30 scans, 60 krpm, edited bottom at the stretched centerpiece bottom as `us_edit` writes it, noise σ = 0.004 from the seeded `US_Math2` generator. Options of `data`: `scans=<n>`, `nobottom` (edit without `<bottom>`) and `tmst` (also write the timestate file set). |
| `us_mpi_regression.py` | Builds the job archive (hpcrequest XML in the LIMS format), runs the case, reduces `analysis-results.tar` to a JSON summary and compares it with `baseline/<case>.json`. |
| `test_us_mpi_jobconfig.cpp` | Unit tests (GoogleTest, no MPI) of `US_MPI_JobConfig`, the hpcrequest XML reader. |
| `us_mpi_jobcheck.cpp` | `us_mpi_jobcheck [-q] <hpcrequest.xml>...`: reads and validates job files as `us_mpi_analysis` does, without MPI and data. |
| `jobs/*.xml` | Anonymized hpcrequest files written by the LIMS (2DSA with noise, GA with buckets, 2DSA custom grid, PCSA) for the unit tests. |
| `baseline/*.json` | Reference summaries: per model the RMSD, the signal-weighted moments of s, f/f0 and vbar, the solute list, and any TI/RI noise vectors. |

## Cases

| Case | Path exercised | Ranks | Comparison |
|---|---|---|---|
| `2dsa_basic` | `_2dsa_master`, uniform grid, 9 subgrids | 2 | exact |
| `2dsa_refine` | iterative refinement (3) | 2 | exact |
| `2dsa_mc` | Monte Carlo (3) | 2 | exact |
| `2dsa_meniscus` | meniscus fit (3 points) | 2 | exact |
| `2dsa_meniscus_nobottom` | the same with an edit without `<bottom>` (stretched centerpiece bottom) | 2 | = `2dsa_meniscus` |
| `2dsa_bottom` | bottom fit (3 points) | 2 | exact |
| `2dsa_menbot` | meniscus and bottom fit (3 × 3) | 2 | exact |
| `2dsa_composite_menbot` | composite job, meniscus and bottom fit (2 × 2) | 2 | exact |
| `2dsa_composite_menbot_pmasters` | the same with parallel masters | 6 | exact |
| `2dsa_timestate`, `2dsa_composite_timestate` | timestate file given (as in LIMS jobs), 1-second intervals | 2 | exact |
| `2dsa_noise` | TI + RI noise | 2 | exact |
| `2dsa_customgrid` | custom grid (s × vbar) | 2 | exact |
| `2dsa_global` | global fit, 2 datasets | 2 | exact |
| `2dsa_global_mc` | global fit with Monte Carlo (3) | 2 | exact |
| `2dsa_composite` | composite job, 2 datasets | 2 | exact |
| `2dsa_composite_mc` | composite job with Monte Carlo (3) | 2 | exact |
| `2dsa_composite_meniscus` | composite job with meniscus fit | 2 | exact |
| `2dsa_refine_np4`, `2dsa_mc_np4` | several workers | 4 | exact |
| `2dsa_mc_pmasters` | parallel masters, MC (2 groups) | 6 | exact |
| `2dsa_mc_pmasters3` | parallel MC, 3 groups, 4 iterations | 9 | exact |
| `2dsa_mc_pmasters_idle` | parallel MC, 3 groups, 2 iterations | 9 | exact |
| `2dsa_mc_pmasters_uneven` | 3 groups asked for 10 ranks: runs as 2 groups | 10 | = `2dsa_mc` |
| `2dsa_mc_odd` | Monte Carlo (3) on 31 × 185 points (odd count of deviates) | 2 | exact |
| `2dsa_mc_odd_pmasters` | the same with parallel masters (2 groups) | 6 | = `2dsa_mc_odd` |
| `2dsa_composite_pmasters` | parallel masters, composite (2 groups) | 6 | exact |
| `2dsa_composite_pmasters_np4` | 3 groups asked for 4 ranks: runs with one master | 4 | = `2dsa_composite` |
| `2dsa_composite_mc_pmasters` | parallel composite with Monte Carlo | 6 | = `2dsa_composite_mc` |
| `2dsa_composite_meniscus_pmasters` | parallel composite with meniscus fit | 6 | exact |
| `pcsa_sl` | PCSA straight lines | 2 | exact |
| `ga_basic` | GA, 3 buckets | 4 | loose |
| `ga_np2` | GA, 3 buckets, one worker | 2 | exact |
| `dmga_basic` | DMGA, 3 components, s, f/f0 and concentration float | 2 | exact |

"Exact" means RMSD within 1e-6 relative, moments within 1e-4 relative and
noise within 1e-6 absolute.  The 2DSA master merges subgrid results in job
order and seeds the noise of each Monte Carlo iteration from the job seed
and the iteration, so a fit gives the same result for any number of workers
or master groups; the `*_np4` and `*_pmasters` cases equal the single-worker
results.  GA migrates genes asynchronously, so its result depends on message
timing; "loose" only requires the same output files and an RMSD not more
than 15 % above the baseline.  "= case" compares with that case's baseline:
the case has none of its own, and must give the same result.  With a single worker (`ga_np2`,
`dmga_basic`) there is no migration and the result is reproducible.

## Input cases

Input cases run one of the cases above with a modified job (defective or
incomplete input, other command-line arguments) and check how the input is
handled.  They have no baseline of their own.  An expected abort must also
leave the job's input files in the work directory.

| Case | Input | Expected |
|---|---|---|
| `input_hpcxml_missing` | no hpcrequest XML in the archive | abort "Could not find unique hpc input file" |
| `input_hpcxml_two` | two hpcrequest files in the archive | abort "Could not find unique hpc input file" |
| `input_hpcxml_named` | the same, `-hpcrequest <file>` | result as `2dsa_basic` |
| `input_hpcxml_unreadable` | hpcrequest XML cannot be opened | abort "Cannot open ..." |
| `input_walltime_jobxml_without` | `jobxmlfile.xml` without walltime, `-walltime 60` | maxwalltime 60 |
| `input_walltime_cmdline_precedence` | `jobxmlfile.xml` walltime 30, `-walltime 60` | maxwalltime 60 (command line wins) |
| `input_ga_mutation_defaults` | GA without `mutate_sigma`, `p_mutate_*` | equal to `ga_np2` (explicit defaults) |
| `input_dmga_sigma_default` | DMGA without `mutate_sigma` | equal to `dmga_basic` (explicit default) |
| `input_ga_negative_bucket` | additional flotation bucket, s from -3 to -1 | finite result, RMSD as `ga_np2` |
| `input_rotor_stretch_short` | `rotor_stretch` with one value | abort naming rotor_stretch |
| `input_xml_truncated` | global fit XML cut off after the first dataset | abort naming the XML error |
| `input_jobxml_truncated` | `jobxmlfile.xml` cut off inside its Header | abort naming the XML error |
| `input_master_abort` | custom grid model without solutes: the master aborts alone during the run | abort naming the model (was: hang) |
| `input_single_rank` | run with one process | abort (was: hang) |
| `input_load_fails_on_worker` | data read fails on rank 1 only (`debug_text` `fail_load_rank=1`) | abort naming the data file and rank |
| `input_timestate_missing` | timestate file listed, not in the archive | result as `2dsa_timestate`, timestate created |

| `input_invalid_number` | 2DSA `s_max="ten"` | abort naming `s_max` (was: segfault) |
| `input_missing_required` | 2DSA without `s_max` | abort naming `s_max` (was: segfault) |
| `input_unknown_method` | method `2DXA` | abort naming the method (was: exit 0, no output) |
| `input_meniscus_points_negative` | `meniscus_points="-3"` | abort naming `meniscus_points` |
| `input_bucket_inverted` | GA bucket with s_min > s_max | abort naming the bucket |
| `input_centerpiece_bottom_default` | `centerpiece_bottom="7.3"` (the invalid default) | abort naming 7.3 |
| `input_unknown_key` | additional parameter `s_mx` | warning, result as `2dsa_basic` |

A case can carry a *pending* marker while the fix of the defect it
documents is still to come: it is reported as XFAIL while it fails and as
an error (XPASS) once it passes, so that the marker gets removed with the
fix.

## Unit tests

```bash
cmake --build build --target test_us_mpi_jobconfig
ctest --test-dir build -L gtest-unit-mpi_analysis --output-on-failure
```

`us_mpi_analysis` reads the job file with `US_MPI_JobConfig` only.  While
the parser was replaced, the old and the new parser were run side by side
on the regression jobs and on 7069 hpcrequest files written by the LIMS
(2DSA, 2DSA_CG, GA, PCSA) and gave the same job description.

## Running

The tests need a build with MPI, programs and tests, without the database.
Prerequisites besides the usual build dependencies: an MPI implementation
(Open MPI or MPICH, with `mpirun`), Python 3, GoogleTest (`BUILD_TESTING`
requires it) and Qwt (required by every profile except HPC, which turns
`BUILD_TESTING` off).  With vcpkg, enable the `mpi` feature.

```bash
cmake -S . -B build -G Ninja -DUS3_NO_DB=ON -DUS3_BUILD_PROGRAMS=ON \
      -DBUILD_TESTING=ON -DUS3_PREFER_STATIC=ON -DBUILD_DOCUMENTATION=OFF
cmake --build build
ctest --test-dir build -L mpi --output-on-failure
```

`-L fit` selects the fit cases, `-L input` the input cases.

Single cases can be run directly:

```bash
python3 test/mpi_analysis/us_mpi_regression.py check 2dsa_mc \
    --program build/bin/us_mpi_analysis --testdata build/bin/us_mpi_testdata
```

`check input` runs all input cases.

## Updating baselines

Only when a change of results is intended:

```bash
python3 test/mpi_analysis/us_mpi_regression.py update all \
    --program build/bin/us_mpi_analysis --testdata build/bin/us_mpi_testdata
```

Review the JSON diff before committing.
