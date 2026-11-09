# Calculation-library fixtures and regression tests

Reference data for extracting the reporting calculations into one library
(extraction plan task E02).  The library and its callers must reproduce these
results; a change to any of them is a scientific decision, recorded with the
change.

## Contents

| Path | Purpose |
|---|---|
| `fixtures/models/<case>/*.model.xml` | Models fitted by `us_mpi_analysis` to simulated samples of known composition.  Generated once by `make_fitted_models.py`, then kept unchanged. |
| `fixtures/models/MANIFEST.json` | How each model was made: sample, method, parameters, MPI, ultrascan3 commit, SHA-256 of every file. |
| `fixtures/edge/*.model.xml` | Hand-made models for edge cases: no components, zero total, one component, values on range bounds, a negative concentration, duplicates and equal-s ties. |
| `fixtures/report_items.json` | Report items (type, method, range, target, tolerance) for each sample, replicate groups and edge items.  Designed once by `calc_oracle.py design`. |
| `fixtures/expected/reporting.json` | Expected range, total-concentration and replicate results from `calc_oracle.py`. |
| `fixtures/expected/legacy_*.json` | Outputs of the current UltraScan code on the fixtures, captured by `test_calc_legacy_gui`. |
| `fixtures/expected/tolerances.json` | Acceptance tolerances. |
| `make_fitted_models.py` | Fits the samples, reusing `../mpi_analysis/us_mpi_regression.py`. |
| `calc_oracle.py` | Independent expected values for the reporter's range and replicate calculations. |
| `test_calc_legacy_gui.cpp` | Runs the current UltraScan calculations on the fixtures and compares with the captured outputs. |

## Samples and fits

| Sample | Species (s in S, f/f0, concentration) | Purpose |
|---|---|---|
| A | 2.5/1.25/0.25, 4.5/1.6/0.40, 7.0/2.1/0.30 | The regression suite's sample; fitted by 2DSA-IT, 2DSA-MC (3 iterations), PCSA-SL, GA and DMGA, plus two more 2DSA-IT replicates at other noise seeds |
| B | 4.3/1.25/0.80, 6.2/1.35/0.15, 9.0/1.5/0.05 | Monomer, dimer and a 5 % trace aggregate; 2DSA-IT and 2DSA-MC |
| C | 5.0/1.4/0.35, 5.3/1.45/0.35, 8.0/1.8/0.30 | A near-degenerate pair 0.3 S apart; 2DSA-IT and 2DSA-MC |

Every simulated dataset uses the experiment of `us_mpi_testdata` (30 scans,
60 krpm, noise σ = 0.004).

## Where expected values come from

- **Reporter range and replicate calculations** (`distrib_info`,
  `velmwl_range_fraction`, `get_replicate_group_results` and the tolerance
  checks in `programs/us_reporter_gmp/us_reporter_gmp.cpp`) cannot be called
  outside the reporter widget.  `calc_oracle.py` computes them independently:
  model values are read as doubles, range membership is decided in double
  arithmetic with the reporter's bound scaling, and every sum, fraction and
  mean is computed exactly and rounded once.  Pass/fail is decided on the
  fraction as the reporter computes it in double arithmetic.
- **Model loading, us_ga_init statistics, US_Integral distributions and
  Gaussian smoothing** are callable, so `test_calc_legacy_gui` runs the
  current code and compares with its captured output.

The fitted-model fixtures keep every component at least a relative 1e-6 from
every range bound, and every fraction at least 1e-6 percentage points from its
tolerance edge, so that rounding cannot change which components count or
whether a check passes.  `calc_oracle.py check` enforces this.

## Running

```bash
python3 test/calc/calc_oracle.py selftest
python3 test/calc/calc_oracle.py check
ctest --test-dir <build> -L calc --output-on-failure
```

`test_calc_legacy_gui` needs a build with the GUI library and tests
(for example `-DUS3_PROFILE=TEST` or a programs-and-tests build).

## Regenerating

Only when a fixture change is intended:

```bash
# A build with MPI, programs and tests, without the database
# (see ../mpi_analysis/README.md); vcpkg's mpi feature provides Open MPI.
B=$PWD/build/<dir>
python3 test/calc/make_fitted_models.py --program $B/bin/us_mpi_analysis \
    --testdata $B/bin/us_mpi_testdata \
    --mpirun $B/vcpkg_installed/<triplet>/tools/openmpi/bin/mpirun
python3 test/calc/calc_oracle.py design     # only if the items should change
python3 test/calc/calc_oracle.py expect
CALC_FIXTURES_UPDATE=1 <build>/bin/test_calc_legacy_gui
```

Paths given to `make_fitted_models.py` must be absolute; each job runs in its
own directory.

## Legacy behavior found while building the fixtures

These need recorded scientific decisions before the library replaces the
current code.  The fixtures capture today's behavior.

1. **Monte Carlo composite.**  `US_Model::load` turns a Monte Carlo file into
   one composite model (`load_multi_model`):  components are merged when the
   text `"%10.4f %8.5f"` of s (in S) and f/f0 (or vbar) matches, the merged
   concentration is the sum divided by the iteration count, every other
   attribute comes from the last matching component, and the description
   gains `_mcN<count>`.  The reporter calculates on this composite.  Merging
   by printed text rather than by value is part of the scientific result.
2. **Tolerance edges depend on rounding.**  `edge_duplicates_ties` has an
   exact fraction of 60 % against a 50 ± 10 % target.  Whether it passes
   depends on the order and rounding of the double sums (it passes as the
   reporter sums today).  A library that sums differently can change the
   result at an exact edge.
3. **Unstable sort in US_Integral.**  `sort_distro` compares only the axis
   value (the comment on `distro_lessthan_s` describes a tie-break on D that
   the code does not have) and uses `std::sort`, which is not stable.  The
   order of equal values, and so which exact duplicates end up adjacent and
   are merged, can differ between C++ libraries.
4. **Duplicate reduction scales every component.**  After merging adjacent
   exact duplicates, `sort_distro` multiplies every concentration, including
   non-duplicates, by 1/(k+1), k being the longest run of duplicates.
5. **Zero totals.**  `distrib_info` divides by a zero total without a guard
   (NaN or infinity); `velmwl_range_fraction` returns 0; `US_Integral`
   normalizes by the total; `outputStats` divides by the total weight;
   `US_Model::constant_vbar` and `constant_ff0` read the first component of
   an empty model.  Empty and zero-total models are therefore excluded from
   the captured legacy outputs.
6. **Unsupported types.**  An item type outside s, D, MW and f/f0 selects
   nothing in `distrib_info` (fraction 0) but is reported unsupported by
   `velmwl_range_fraction`.
7. **Gaussian smoothing past the array.**  `US_Math2::gaussian_smoothing`
   needs at least `2*smooth-1` points.  With fewer, its border loops read
   and write past the array; a Qt build with bounds assertions aborts.
   `US_Integral` passes a series as long as the division count (0-100) with
   a user-set smoothing width (1-100), so a short division count and a wide
   smoothing reach this in the desktop program.  The captured cases stay
   within the precondition.
8. **Range bound scaling.**  The reporter scales bounds with `pow(10,-13)`,
   `pow(10,-7)` and `pow(10,3)` in one path and with literals in the other.
   They agree exactly on the reference platform (checked over 100 000
   bounds); the exact-bound edge fixtures guard this.
