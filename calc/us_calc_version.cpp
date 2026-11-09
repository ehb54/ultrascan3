//! \file us_calc_version.cpp
//! The descriptions state the rules decided in the extraction plan
//! (doc/UltraScan_Calculation_Library_Extraction_Plan.docx, edition 8).
#include "us_calc_version.h"

namespace US_Calc
{
std::string library_version()
{
   return "0.1.0";
}

std::vector<AlgorithmVersion> algorithms()
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
      { "solute_statistics", 1, "Concentration-weighted statistics of one attribute as US_SoluteData::outputStats" },
      { "report_item_matching",
        1,
        "A report item applies to a model when every '-' part of its method (PCSA methods count as PCSA) "
        "occurs in the model's analysis label, as distrib_info" },
   };
}

std::optional<AlgorithmVersion> algorithm( const std::string& name )
{
   for ( const AlgorithmVersion& a : algorithms() )
   {
      if ( a.name == name )
      {
         return a;
      }
   }
   return std::nullopt;
}
} // namespace US_Calc
