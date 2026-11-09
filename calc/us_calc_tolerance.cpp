//! \file us_calc_tolerance.cpp
#include "us_calc_tolerance.h"
#include "us_calc_fp.h" // first:  floating-point policy

#include <cmath>

namespace US_Calc
{
bool passes( double value, const ToleranceCheck& check )
{
   const double t   = check.target;
   const double tol = check.tolerance;
   switch ( check.rule )
   {
      case ToleranceRule::RelativePercentOfTarget:
         return value >= ( t * ( 1 - tol / 100.0 ) ) && value <= ( t * ( 1 + tol / 100.0 ) );
      case ToleranceRule::AbsolutePercentagePoints:
         return std::fabs( value - t ) <= tol;
      case ToleranceRule::AbsoluteUnits:
         return value >= ( t - tol ) && value <= ( t + tol );
      case ToleranceRule::UpperLimit:
         return value <= t;
      case ToleranceRule::ExclusiveMinimum:
         return value > t;
   }
   return false;
}

std::string tolerance_rule_name( ToleranceRule rule )
{
   switch ( rule )
   {
      case ToleranceRule::RelativePercentOfTarget:
         return "relative_percent_of_target";
      case ToleranceRule::AbsolutePercentagePoints:
         return "absolute_percentage_points";
      case ToleranceRule::AbsoluteUnits:
         return "absolute_units";
      case ToleranceRule::UpperLimit:
         return "upper_limit";
      case ToleranceRule::ExclusiveMinimum:
         return "exclusive_minimum";
   }
   return "";
}
} // namespace US_Calc
