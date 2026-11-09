//! \file us_calc_tolerance.h
//! \brief Named tolerance rules (algorithm "tolerance").
//!
//! Each rule keeps the exact comparison of the GMP reporter, because forms
//! that are equal in exact arithmetic, such as |v - t| <= tol and
//! t - tol <= v <= t + tol, can differ by one rounding step at the edge.
#ifndef US_CALC_TOLERANCE_H
#define US_CALC_TOLERANCE_H

#include <string>

namespace US_Calc
{
//! \brief A tolerance rule of the reporter.
enum class ToleranceRule
{
   //! t*(1 - tol/100) <= v <= t*(1 + tol/100):  replicate fraction, total
   //! concentration, experiment duration, loading volume (10045, 10728-10732)
   RelativePercentOfTarget,
   //! |v - t| <= tol:  range fraction against its target (10946, 4667)
   AbsolutePercentagePoints,
   //! t - tol <= v <= t + tol:  temperature (7679)
   AbsoluteUnits,
   //! v <= t:  RMSD limit (10730)
   UpperLimit,
   //! v > t:  minimum intensity (10731)
   ExclusiveMinimum
};

//! \brief A check of a value against a target.  The tolerance is not used
//! by UpperLimit and ExclusiveMinimum.
struct ToleranceCheck
{
      ToleranceRule rule      = ToleranceRule::AbsolutePercentagePoints;
      double        target    = 0.0;
      double        tolerance = 0.0;
};

//! \brief True when the value passes the check.  A NaN value fails.
bool        passes( double value, const ToleranceCheck& check );

//! \brief The name of a rule, for provenance.
std::string tolerance_rule_name( ToleranceRule rule );
} // namespace US_Calc

#endif
