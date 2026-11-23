# Calculation fixtures and regression tests

Reference data and regression tests for the shared calculations (namespace
`US_Calc`, the `us_calc_*` files in `utils/`) and the desktop programs that
use them.  The calculations and their callers must reproduce these results; a change to any of them is a scientific
decision, recorded with the change.

## Contents

| Path | Purpose |
|---|---|
| `fixtures/models/<case>/*.model.xml` | Models fitted by `us_mpi_analysis` to simulated samples of known composition.  Generated once by `make_fitted_models.py`, then kept unchanged. |
| `fixtures/models/MANIFEST.json` | How each model was made: sample, method, parameters, MPI, ultrascan3 commit, SHA-256 of every file. |
| `fixtures/edge/*.model.xml` | Hand-made models for edge cases: no components, zero total, one component, values on range bounds, a negative concentration, duplicates and equal-s ties. |
| `fixtures/report_items.json` | Report items (type, method, range, target, tolerance) for each sample, replicate groups and edge items.  Designed once by `calc_oracle.py design`. |
| `fixtures/expected/reporting.json` | Expected range, total-concentration and replicate results from `calc_oracle.py`. |
| `fixtures/expected/legacy_*.json` | Outputs of the desktop code on the fixtures (model loading, us_ga_init statistics, US_Integral curves, smoothing), captured by `test_calc_legacy_gui`.  They equal the code before US_Calc replaced it, except for the changes listed below. |
| `fixtures/expected/tolerances.json` | Acceptance tolerances. |
| `make_fitted_models.py` | Fits the samples, reusing `../mpi_analysis/us_mpi_regression.py`. |
| `calc_oracle.py` | Independent expected values for the reporter's range and replicate calculations. |
| `test_calc_legacy_gui.cpp` | Runs the desktop calculations on the fixtures and compares with the captured outputs. |

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
  `distrib_info_velmwl`, `get_replicate_group_results` and the tolerance
  checks in `programs/us_reporter_gmp/us_reporter_gmp.cpp`, which call
  US_Calc).  `calc_oracle.py` computes them independently of US_Calc:
  model values are read as doubles, range membership is decided in double
  arithmetic with the reporter's bound scaling, and every sum, fraction and
  mean is computed exactly and rounded once.  Pass/fail is decided on the
  fraction as the reporter computes it in double arithmetic.
- **Model loading, us_ga_init statistics, US_Integral distributions and
  Gaussian smoothing** are callable, so `test_calc_legacy_gui` runs the
  desktop code and compares with its captured output.

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

## Legacy behavior and what US_Calc does

Behavior of the desktop code found while building the fixtures, and how
`US_Calc` handles it.  Items 3, 4 and 7 were reported in
ehb54/ultrascan-tickets#1214.

1. **Monte Carlo composite.**  `US_Model::load` turns a Monte Carlo file into
   one composite model (`load_multi_model`):  components are merged when the
   text `"%10.4f %8.5f"` of s (in S) and f/f0 (or vbar) matches, the merged
   concentration is the sum divided by the iteration count, every other
   attribute comes from the last matching component, and the description
   gains `_mcN<count>`.  The reporter calculates on this composite.  Merging
   by printed text rather than by value is part of the scientific result, so
   US_Calc keeps it and makes the keys with the same
   `QString::asprintf` call.
2. **Tolerance edges depend on rounding.**  `edge_duplicates_ties` has an
   exact fraction of 60 % against a 50 ± 10 % target.  Whether it passes
   depends on the order and rounding of the double sums; it passes as the
   reporter sums.  US_Calc sums in the same order, so results on an
   edge match existing reports.
3. **Unstable sort in US_Integral.**  The desktop `sort_distro` compared only
   the axis value (the comment on `distro_lessthan_s` described a tie-break
   on D that the code did not have) and used `std::sort`, which is not
   stable.  The order of equal values, and so which exact duplicates ended
   up adjacent and were merged, depended on the component order and differed
   between libc++ and libstdc++ from about 20 components.  US_Calc sorts
   on a total order with `std::stable_sort`, and US_Integral uses it.
4. **Duplicate reduction shrank the distribution.**  After merging adjacent
   exact duplicates, the desktop `sort_distro` multiplied every
   concentration, including non-duplicates, by 1/(k+1), k being the longest
   run of duplicates, while `resort_sol` normalized by the original total.
   The cumulative sum then ended at 1/(k+1) and the last component received
   the remaining fractions; the same solutes in two orders gave 70 % and
   about 35 % at one s value.  US_Calc merges solutes with equal s, D,
   MW and f/f0 wherever they appear (concentrations summed, vbar and radius
   averaged), rescales nothing, and the cumulative sum ends at 1.  The
   `edge_duplicates_ties` curves in `legacy_integral.json` are the corrected
   ones; every other curve equals the desktop code before the change.
5. **Zero totals.**  `distrib_info` divides by a zero total without a guard
   (NaN or infinity); `distrib_info_velmwl` shows 0; `US_Integral`
   normalizes by the total; `outputStats` divides by the total weight;
   `US_Model::constant_vbar` and `constant_ff0` read the first component of
   an empty model.  US_Calc reports an empty model or a zero total as an
   error, with the value the legacy division gives (NaN or infinity), so
   the desktop output is unchanged.  US_Integral leaves a model with a zero
   total out of its plot and CSV files and says so in the legend; the other
   models are drawn as before (the desktop drew a vertical line for it).
   Empty and zero-total models are excluded from the captured outputs.
6. **Unsupported types.**  An item type outside s, D, MW and f/f0 selects
   nothing in `distrib_info` (fraction 0) but is reported unsupported by
   `distrib_info_velmwl`, except on a model without components, where it
   gives 0.  US_Calc reports it unsupported; the reporter still shows
   0 / C * 100 and the VELOCITY-MWL table 0 % on an empty model.
7. **Gaussian smoothing past the array.**  `gaussian_smoothing` needs at
   least `2*smooth-1` points; with fewer, its border loops read and write
   past the array, and a Qt build with bounds assertions aborts.  US_Integral
   reached this because its smoothing control started with a maximum of 100
   while the division count started at 100.  US_Calc rejects such a
   series and leaves it unchanged, `US_Math2::gaussian_smoothing` leaves it
   unchanged, and US_Integral limits the smoothing to what the division
   count supports from the start, as does us_density_match, which had the
   same starting values.  us_mwl_spectra, whose point count depends on the
   wavelength range, says in its status line when a width is too large.
   The captured cases stay within the precondition.
8. **Floating-point contraction.**  Compilers may fuse a multiply and an
   add (FMA), which changes the last bits and, where a value sits on a bin
   edge or a difference cancels exactly, the result itself.  Legacy results
   therefore differed between builds (arm64, GCC defaults, -march=native).
   US_Calc is compiled without contraction (`-ffp-contract=off` in CMake and
   the `us_calc_fp.h` pragmas, which are all a qmake build has), and equals
   the legacy code compiled without contraction bit for bit.  This was shown
   on macOS arm64, Linux x86_64 (GCC) and Windows x64 (MSVC), not on builds
   that contract by default (GCC on aarch64, -march=native).  `pow`, `exp`,
   `log` and `sqrt` come from each platform's math library, so results that
   use them can still differ between platforms in the last bits.  The captures are made without contraction; the
   smoothing captures differ from an FMA build of the legacy code by at most
   3.6e-16 relative.
9. **Histogram area missed the maximum.**  `outputStats` tested each of its
   50 bins against edge + width, which rounds differently from the next
   edge, and left the last bin open, so the maximum value was counted or
   not depending on rounding.  US_Calc partitions the range so every value
   is counted once, and the area is the total concentration times the bin
   width.  In `legacy_output_stats.json` this differs from the earlier code
   in the area of 55 of 250 blocks and in the mode and confidence limits of
   4 edge-model blocks.  The "Median" `outputStats` prints is the midrange.
   The weighted confidence intervals sorted the values alone with
   `std::sort`, so with equal values of different concentration (common on
   2DSA grids) they differed between libc++ and libstdc++ and with component
   order.  US_Calc orders equal values by concentration; this changed the
   intervals of 3 of 75 blocks in `legacy_output_stats.json` against the
   macOS (libc++) output.
   `US_DmgaMcStats::compute_statistics` (DMGA-MC, us_fematch) had a copy of
   the same histogram and now uses US_Calc too, so its area and mode
   change in the same way.
10. **Two definitions of weighted statistics.**  `outputStats` and
   `compute_statistics` share the minimum, maximum, weighted mean, line fit
   and histogram but define the rest differently:  `outputStats` treats the
   values as a distribution weighted by concentration (weighted moments with
   a bias factor, weighted quantiles as confidence intervals, the midrange as
   "Median", a relative 1e-4 spread as constant); `compute_statistics`
   treats each value as one Monte Carlo iteration (unweighted moments about
   the weighted mean, the median of the values, mean -/+ 1.960 and 2.576
   standard deviations as confidence intervals, only equal values as
   constant).  `weighted_statistics` provides both as named conventions, so
   each program keeps its own definitions.
11. **Range bound scaling.**  The reporter scales bounds with `pow(10,-13)`,
   `pow(10,-7)` and `pow(10,3)` in one path and with literals in the other.
   They agree exactly on the reference platform (checked over 100 000
   bounds); the exact-bound edge fixtures guard this.
12. **Zero buoyancy.**  `US_Model::calc_coefficients` rejected an s whose sign
   did not match the buoyancy, but let a nonzero s through when the buoyancy
   was zero (vbar20 = 1 / DENS_20W), and the pairs that start from s or mw
   then divided by zero, giving infinite or NaN coefficients.  US_Calc
   rejects those pairs at zero buoyancy and leaves the values unchanged, as
   for a sign mismatch; D or f with f/f0 still give s = 0.  Every other
   input gives the same doubles as before (checked over 2 000 000
   components).  No fixture has zero buoyancy.
13. **Inverted ranges and non-finite values.**  The reporter accepts a range
   item with low > high (`us_report_gui` checks only the number syntax) and
   selects nothing, so the fraction is 0 and a 0 % target passes.  A
   non-finite value enters only the sums that use it:  a NaN s is never in
   an s range and makes only the s mean NaN.  US_Calc calculates these the
   same way and reports them as warnings (`validate_range`,
   `validate_values`); the `edge` items include an inverted range.
14. **Coefficients in US_Integral.**  `US_Model::update_coefficients`
   stops at the first component it cannot complete and leaves the later
   ones as stored, so the plotted D, MW, f/f0 and radius of those
   components depended on the order.  `integral_distribution` completes
   every component.
15. **Empty Monte Carlo iterations.**  `US_Model::constant_vbar` and
   `constant_ff0` read the first component of each iteration, past the end
   of an iteration without components.  `mc_composite` reports such an
   iteration as an error, and `US_Model::load` returns `NO_MODEL` for the
   whole model.
16. **Undefined statistics.**  `weighted_statistics` returns every value as
   the legacy arithmetic gives it, infinities included (us_ga_init printed
   `inf` for the standard deviation of a bin with one nonzero
   concentration), and names each value that is not finite in a
   diagnostic.
