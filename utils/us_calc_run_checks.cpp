//! \file us_calc_run_checks.cpp
#include "us_calc_run_checks.h"
#include "us_calc_fp.h" // floating-point policy

#include <QtMath>

namespace US_Calc::Gmp
{
namespace
{
RunCheckResult run_check( double value, ToleranceRule rule, double target, double tolerance )
{
   RunCheckResult result;
   result.value  = value;
   result.check  = { rule, target, tolerance };
   result.passed = passes( value, result.check );
   return result;
}
} // namespace

RunCheckResult check_total_concentration( double total, double target, double factor, double tolerance_percent )
{
   return run_check( total, ToleranceRule::RelativePercentOfTarget, target * factor, tolerance_percent );
}

RunCheckResult check_experiment_duration( double last_scan_seconds, double target_seconds, double tolerance_percent )
{
   return run_check( last_scan_seconds, ToleranceRule::RelativePercentOfTarget, target_seconds, tolerance_percent );
}

RunCheckResult check_rmsd( double rmsd, double limit )
{
   return run_check( rmsd, ToleranceRule::UpperLimit, limit, 0.0 );
}

RunCheckResult check_minimum_intensity( double intensity, double minimum )
{
   return run_check( intensity, ToleranceRule::ExclusiveMinimum, minimum, 0.0 );
}

bool minimum_intensity_shown( const QString& data_source, bool interference_triple )
{
   return !( data_source.contains( "DiskAUC:Absorbance" ) || data_source.contains( "DiskAUC:PseudoAbsorbance" ) ||
             interference_triple );
}

RunCheckResult check_loading_volume( double volume, double target, double tolerance_percent )
{
   return run_check( volume, ToleranceRule::RelativePercentOfTarget, target, tolerance_percent );
}

double run_duration_target( double step_duration, double speed, double accel )
{
   return step_duration + speed / accel;
}

RunCheckResult check_run_duration( double measured_minutes, double target_seconds, double tolerance_percent )
{
   return run_check( measured_minutes * 60.0, ToleranceRule::RelativePercentOfTarget, target_seconds, tolerance_percent );
}

RunCheckResult check_temperature( double measured, double target )
{
   return run_check( measured, ToleranceRule::AbsoluteUnits, target, 0.5 );
}

RunCheckResult check_speed( double measured, double target )
{
   return run_check( measured, ToleranceRule::AbsoluteUnits, target, 4.0 );
}
double rotor_stretch( double c0, double c1, double speed )
{
   return c0 * speed + c1 * ( speed * speed );
}

double loading_volume( double bottom, double stretch, double meniscus, double pathlength, double angle )
{
   const double b = bottom + stretch;
   return ( b * b - meniscus * meniscus ) * ( 1000 * pathlength * angle * M_PI ) / 360.0;
}
} // namespace US_Calc::Gmp
