//! \file us_calc_replicate.h
//! \brief Replicate group summaries (algorithm "replicate_summary").
//!
//! As get_replicate_group_results in us_reporter_gmp.cpp:  the arithmetic
//! mean of the members' in-range concentrations and fractions, and their
//! population standard deviations (divisor N), over the members whose
//! in-range concentration is not negative.  Members are used in the order
//! given.
#ifndef US_CALC_REPLICATE_H
#define US_CALC_REPLICATE_H

#include "us_calc_range.h"
#include "us_calc_types.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace US_Calc
{
//! \brief Mean and population standard deviation of a replicate group.
struct ReplicateResult
{
      std::size_t             member_count = 0; //!< Members given
      std::size_t             used_count   = 0; //!< Members used
      //! Means and standard deviations; empty when no member is used.
      std::optional<double>   mean_range_concentration;
      std::optional<double>   sd_range_concentration;
      std::optional<double>   mean_fraction_percent;
      std::optional<double>   sd_fraction_percent;
      std::vector<Diagnostic> diagnostics;
      AlgorithmVersion        algorithm;
};

//! \brief Summarize the range results of a replicate group's members.
//!
//! A member is used when its in-range concentration is not negative and
//! its fraction is defined.  Others are left out with a warning.
ReplicateResult replicate_summary( const std::vector<RangeResult>& members );
} // namespace US_Calc

#endif
