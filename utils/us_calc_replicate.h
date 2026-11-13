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

#include <QVector>
#include <QtNumeric>

namespace US_Calc
{
//! \brief Mean and population standard deviation of a replicate group.
struct ReplicateResult
{
      int                 member_count             = 0; //!< Members given
      int                 used_count               = 0; //!< Members used
      //! Means and standard deviations; NaN when no member is used.
      double              mean_range_concentration = qQNaN();
      double              sd_range_concentration   = qQNaN();
      double              mean_fraction_percent    = qQNaN();
      double              sd_fraction_percent      = qQNaN();
      QVector<Diagnostic> diagnostics;
      AlgorithmVersion    algorithm;
};

//! \brief Summarize the range results of a replicate group's members.
//!
//! A member is used when its in-range concentration is not negative; others
//! are left out with a warning.  A used member whose fraction is NaN (a zero
//! total) makes the fraction mean and SD NaN, as in the reporter, and adds a
//! warning.
US_UTIL_EXTERN ReplicateResult replicate_summary( const QVector<RangeResult>& members );
} // namespace US_Calc

#endif
