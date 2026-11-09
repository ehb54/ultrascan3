# us_mpi_analysis regression tests

End-to-end tests that run `us_mpi_analysis` under `mpirun` on synthetic data
and compare the fitted models with stored baselines.

## Contents

| File | Purpose |
|---|---|
| `us_mpi_testdata.cpp` | Writes a simulated velocity dataset, a DMGA constraints model and a custom-grid model with the UltraScan writers (`US_DataIO::writeRawData`, `US_DataIO::writeEdits`, `US_Model::write`). Three species, 30 scans, 60 krpm, edited bottom at the stretched centerpiece bottom as `us_edit` writes it, noise σ = 0.004 from the seeded `US_Math2` generator. Options of `data`: `scans=<n>`, `nobottom` (edit without `<bottom>`), `tmst` (also write the timestate file set), `noise=<sigma>` and `species=<s>:<f/f0>:<c>[,...]` (another sample). |
| `ga_truth_eval.py` | Scores GA fits of samples with known composition against the truth, for several seeds and builds (see below). |
| `us_mpi_regression.py` | Builds the job archive (hpcrequest XML in the LIMS format), runs the case, reduces `analysis-results.tar` to a JSON summary and compares it with `baseline/<case>.json`. |
| `test_us_mpi_jobconfig.cpp` | Unit tests (GoogleTest, no MPI) of `US_MPI_JobConfig`, the hpcrequest XML reader. |
| `test_us_mpi_memlimit.cpp` | Unit tests (GoogleTest, no MPI) of `US_MPI_MemLimit`, the job's memory limit and use. |
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
| `dmga_basic` | DMGA, 3 components, s, f/f0 and concentration float | 2 | loose |

"Exact" means RMSD within 1e-6 relative, moments within 1e-4 relative and
noise within 1e-6 absolute.  The 2DSA master merges subgrid results in job
order and seeds the noise of each Monte Carlo iteration from the job seed
and the iteration, so a fit gives the same result for any number of workers
or master groups; the `*_np4` and `*_pmasters` cases equal the single-worker
results.  GA migrates genes asynchronously, so its result depends on message
timing; "loose" only requires the same output files and an RMSD not more
than 15 % above the baseline.  "= case" compares with that case's baseline:
the case has none of its own, and must give the same result.  With a single worker (`ga_np2`,
`dmga_basic`) there is no migration and the result is reproducible on one
platform.  DMGA's mutations go through `log` and `sqrt`, though, and a
last-bit difference in the math library sends its search down another path,
so `dmga_basic` is compared loosely.

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

## GA known-truth evaluation

`ga_truth_eval.py` is not a pass/fail test.  It simulates samples of known
composition (three species, the same with empty "decoy" buckets and the
parsimony penalty, the same at 2.5 times the noise, two species with equal
s, five species, wide buckets, and a Monte Carlo run), fits each with the GA
for several seeds and reports the fit RMSD relative to the noise, the
number of solutes, the signal in decoy buckets and the errors of s, f/f0,
concentration and molar mass of each species.  Several builds can be
compared in one call:

```bash
python3 test/mpi_analysis/ga_truth_eval.py \
    --variant before=old/bin/us_mpi_analysis \
    --variant after=build/bin/us_mpi_analysis \
    --testdata build/bin/us_mpi_testdata --seeds 1,2,3,4,5,6
```

Results accumulate in `--out` (default `ga_truth_results.json`), so an
interrupted evaluation resumes where it stopped; `--report <file>` prints
the tables again.

## Simulation cache

`us_mpi_analysis` reuses simulations through a cache shared by the ranks of
each node.  The cache stores simulations exactly, so every case must give
the same result with and without it.  `--debug-text` passes a `debug_text`
job parameter to every case:

```bash
python3 test/mpi_analysis/us_mpi_regression.py check all --debug-text SimCache=off ...
python3 test/mpi_analysis/us_mpi_regression.py check all --debug-text SimCacheMB=1 ...
```

What a full cache keeps depends on the method.  2DSA and PCSA keep the
first simulations and store no more:  they repeat the same solutes
(refinement and Monte Carlo iterations, meniscus and bottom points), and of
a repeated sequence longer than the cache the first part is reused, while
discarding the oldest simulations would reuse none.  GA keeps the
simulations used last:  the cache holds two generations of half the size,
a hit in the older one is copied into the current one, and when the current
one is full the older one is emptied and becomes current.  The survivor
flag does not help here, since nearly every solute of a GA gene has a
nonzero concentration.  The off-grid points of the GA gradient search are
not stored.  `SimCachePolicy=first` or `recent` overrides the choice.
Each rank computes the key of a data set (a SHA-256 of its grid and
simulation parameters) once per fit context, not on every fitness
evaluation, and fills a simulation's grid only when it is not in the cache.
A 2DSA master with a single masters group clears every node's cache when
the data set or the meniscus/bottom point changes.  Parallel masters groups
fit different points at once, so no master may clear the cache:  2DSA and
PCSA with more than one group keep the simulations used last (RECENT), which
follow the groups' current fits, instead of the first fits' forever.  A
single PCSA master never clears the cache, so a full cache keeps the
simulations of the first fit context; Monte Carlo iterations of one context
reuse them.
GA, 100 genes, 40 generations, 4 ranks (about 185 MB of simulations):

| Cache | first | recent |
|---|---|---|
| 3.9 GB | 81 % reused | 81 % reused |
| 50 MB | 36 % | 81 % |
| 20 MB | 17 % | 76 % |

The cache gives way to the job's own memory.  The limit is the tightest of
Slurm's `--mem` or `--mem-per-cpu` and the job's cgroup; the capacity is at
most half of it and must fit the free space of the file system that holds
the shared window.  That place depends on the MPI implementation (MPICH:
`/dev/shm`; Open MPI: its session directory, which may be on a disk; System
V shared memory; MS-MPI: the paging file), so the node's first rank maps a
small window at the start and looks it up in `/proc/self/maps`.  A GA sizes
its cache with the job parameters `sim_cache_mb` (MB per rank, the node's
cache gets that times its ranks; 0 turns it off) and `sim_cache_lru`; by
default a quarter of the memory its ranks may use, at most 4 GB per rank,
and no cache below 16 MB per rank.  `sim_cache_lru=0` makes the node's
cache keep the first simulations (KEEP_FIRST, nothing more is stored once
it is full).  Should the shared window not be allocated, or shared memory hold
less than half of the GA's cache (a container's 64 MB `/dev/shm`), a GA
falls back to a cache in each rank, in the rank's own memory, for which
`sim_cache_lru=0` selects `US_SimCache`'s own policy (it evicts
non-survivors first).

The whole window, its slot table included, is fitted to 90 % of the free
space of its file system (a page touched beyond the free space kills the
process with a bus error).  Before the collective allocation the node's
first rank checks that the window fits that space and half of the node's
memory, and if not no rank allocates:  a failure inside
`MPI_Win_allocate_shared` on one rank cannot be undone (Open MPI leaves the
other ranks waiting in it, MPICH aborts them).  The report at the end gives
the bytes stored and the memory the cache holds, which is more after a
rotation of RECENT:  an emptied generation keeps its pages until it is
filled again.

2DSA, PCSA and DMGA had no cache before the node's cache and must not fail
for it where they fit their memory without it.  They get a cache only with
a job memory limit, or with a size the host sets for the cache of each job
in the environment variable `US3_SIM_CACHE_MB` (a LIMS server, whose
database and web server share the node, sets it to what it can spare per
job; with a full queue all jobs' caches together stay within that times the
jobs).  Without either the run logs "Simulation caching is off".  With a
limit, the cache must leave room for the job's own peak, which comes while
the workers solve their first subgrids, before the watch can react.  So the
cache may at first hold only what the job's predicted peak leaves below 75
% of the limit:  each rank's memory at the start plus what it needs to solve
a subgrid and half a subgrid of survivors (the estimate of the 2DSA fit
control:  17.2 MB plus 2.28 times the bytes of the solutes' simulations,
four data vectors and the noise terms), times 1.25.  After the first
refinement iteration (or the fit, without refinement), when the watch has
seen the job's real peak, the master has every node's cache fit its room to
that peak times 1.10.  The log shows the room at the start and after.
Parallel masters groups keep the room of the prediction.
While the job runs, a thread on each node's first rank compares the use with
the limit, the node's available memory and the free shared memory (MPI is
initialized with `MPI_THREAD_FUNNELED` for it; the thread makes no MPI
calls).  Above 75 % of the limit no
rank adds to the cache, except that a GA empties its older generation to make
room.  Above 88 % the whole cache is emptied and its pages are returned to the
system.  The run then only has to compute some simulations again, and the
result stays the same.  The report at the end says how often this happened.
The job's memory in `job_statistics.xml` (`maxmemory`) counts the node's
cache once:  each rank leaves out the pages of the window it has mapped and
the node's first rank adds what the cache holds (on Linux; elsewhere a rank's
pages are unknown and each rank's RSS still includes those it touched).
`SimCacheLimitMB=N` sets a limit for tests.  `SimCacheStress=1` empties the
cache over and over while the ranks use it.

CTest repeats a few cases (`ctest -L simcache`):  with the cache off, with a
1 MB cache, too small for the fit (2DSA and PCSA store hardly anything, GA
empties a generation every few simulations), with a 1 MB cache and the
recent policy for every method, and with `NodeSplit=2` and a 4 GB limit,
which makes two groups of ranks act as separate nodes with separate caches
(a test switch for multi-node behavior on one machine), with
`SimCacheLimitMB=1`, with a limit that leaves no room for the predicted peak
(`SimCacheLimitMB=200`), and with `SimCacheStress=1` and a 4 GB limit.  Input cases check
that 2DSA without a limit has no cache, and has one with `US3_SIM_CACHE_MB`.

### Benchmark

`us_mpi_benchmark.py` times a larger 2DSA job (default 64 × 64 grid, 5
refinement iterations) with the cache off and on and prints the speedup,
the node cache reports and whether the RMSDs agree:

```bash
python3 test/mpi_analysis/us_mpi_benchmark.py --program build/bin/us_mpi_analysis \
    --testdata build/bin/us_mpi_testdata --np 8 --grid 64 --refine 5 [--mc 3] [--mgroupcount 2]
```

## Updating baselines

Only when a change of results is intended:

```bash
python3 test/mpi_analysis/us_mpi_regression.py update all \
    --program build/bin/us_mpi_analysis --testdata build/bin/us_mpi_testdata
```

Review the JSON diff before committing.
