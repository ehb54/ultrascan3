//! \file us_calc_version.cpp
#include "us_calc_fp.h" // floating-point policy
//! The descriptions state the rule each algorithm follows.
#include "us_calc_version.h"

namespace US_Calc
{
QString library_version()
{
   return "1.0.0";
}

QVector<AlgorithmVersion> algorithms()
{
   return {
      { "range_summary",
        1,
        "Total and in-range concentration, fraction 100*R/C and weighted means of a model; "
        "inclusive bounds scaled as the GMP reporter scales them; sums in component order" },
      { "replicate_summary",
        1,
        "Arithmetic mean and population standard deviation (divisor N) over members whose "
        "in-range concentration is not negative" },
      { "tolerance",
        1,
        "Named rules: relative percent of target, absolute percentage points, absolute units, "
        "upper limit, exclusive minimum" },
      { "mc_composite",
        1,
        "Monte Carlo composite as US_Model builds it: components merged by the text key "
        "\"%10.4f %8.5f\" of s (S) and f/f0 or vbar, concentration averaged over iterations, "
        "other values from the last match" },
      { "integral_distribution",
        1,
        "Integral distribution with a total-order sort, duplicates merged by value and "
        "normalization so the cumulative sum ends at 1 (corrects the legacy reduction)" },
      { "gaussian_smoothing",
        1,
        "Gaussian smoothing as US_Math2::gaussian_smoothing; widths the series cannot support "
        "(fewer than 2*width-1 points) are rejected" },
      { "weighted_statistics",
        1,
        "Weighted statistics of one attribute in two conventions:  a concentration distribution as "
        "US_SoluteData::outputStats and Monte Carlo iterations as US_DmgaMcStats::compute_statistics, "
        "both with 50 bins that partition the range so every value, the maximum included, is counted once" },
      { "report_item_matching",
        1,
        "A report item applies to a model when every '-' part of its method (PCSA methods count as PCSA) "
        "occurs in the model's analysis label, as distrib_info" },
      { "report_item",
        1,
        "A report item on a model as distrib_info:  range_summary for a range type, (0 / C) * 100 for any other "
        "type, passed when |fraction - target| <= tolerance (percentage points)" },
      { "velmwl_report_item",
        1,
        "A report item on a VELOCITY-MWL species model as distrib_info_velmwl:  0 for a model without components, "
        "unsupported (n/a) for a type that is not a range type, 0 when the total is not positive, otherwise "
        "range_summary's fraction; passed when |fraction - target| <= tolerance (percentage points)" },
      { "report_replicate_group",
        2,
        "A replicate subgroup as the GMP reporter:  one subgroup per wavelength of the included triples; members are the "
        "evaluated items equal to the first channel's item in type, method, bounds and QString::number of the "
        "integration value; replicate_summary of the members (0 when none); SD shown when more than one triple is "
        "listed; passed when |mean - target| <= tolerance (absolute percentage points)" },
      { "report_run_checks",
        1,
        "The GMP reporter's run checks:  total concentration (target times the loading-concentration factor), "
        "experiment duration, loading volume and run duration within tolerance percent of the target (relative); "
        "RMSD at most the limit; intensity above the minimum; temperature within 0.5 C and speed within 4 rpm; "
        "run duration target = step duration + speed / accel; loading volume "
        "((bottom + stretch)^2 - meniscus^2) * (1000 * pathlength * angle * pi) / 360 with stretch "
        "c0 * speed + c1 * speed^2" },
      { "report_lookup",
        1,
        "The GMP reporter's inputs for a triple's model:  triple, cell, wavelength and model name by position in the "
        "description; the first channel whose name the triple contains (interference channels only for interference "
        "triples); the single intensity, or the first key the triple contains (NaN when none); mask rows by keyword" },
   };
}

AlgorithmVersion algorithm( const QString& name )
{
   for ( const AlgorithmVersion& a : algorithms() )
   {
      if ( a.name == name )
      {
         return a;
      }
   }
   return AlgorithmVersion();
}
} // namespace US_Calc
