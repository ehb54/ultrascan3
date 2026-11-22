//! \file us_calc_run_checks.h
//! \brief The run checks of a GMP report (algorithm "report_run_checks").
//!
//! Each check of the GMP reporter (us_reporter_gmp.cpp) with the rule it
//! uses:  the comparison section of distrib_info (total concentration,
//! experiment duration, RMSD, minimum intensity, loading volume) and the
//! run-details table of assemble_run_details_html (duration, temperature,
//! speed).  Tolerances in percent are percent of the target.
#ifndef US_CALC_RUN_CHECKS_H
#define US_CALC_RUN_CHECKS_H

#include "us_calc_tolerance.h"

#include <QString>

namespace US_Calc
{
//! GMP report rules
namespace Gmp
{
//! \brief A run check:  the value checked, the check (whose target and
//! tolerance the report shows) and its result.
struct RunCheckResult
{
      double         value = 0.0;
      ToleranceCheck check;
      bool           passed = false;
};

//! \brief Total concentration against the report's target times the
//! channel's loading-concentration factor, within tolerance percent.
US_UTIL_EXTERN RunCheckResult check_total_concentration( double total, double target, double factor, double tolerance_percent );

//! \brief Time of the triple's last scan (seconds) against the report's
//! experiment duration, within tolerance percent.
US_UTIL_EXTERN RunCheckResult check_experiment_duration( double last_scan_seconds,
                                                         double target_seconds,
                                                         double tolerance_percent );

//! \brief RMSD at most the limit.
US_UTIL_EXTERN RunCheckResult check_rmsd( double rmsd, double limit );

//! \brief Average intensity above the minimum (not equal to it).
US_UTIL_EXTERN RunCheckResult check_minimum_intensity( double intensity, double minimum );

//! \brief Whether the Minimum Intensity row is shown:  not for absorbance or
//! pseudo-absorbance data, and not for an interference triple.
US_UTIL_EXTERN bool           minimum_intensity_shown( const QString& data_source, bool interference_triple );

//! \brief Loading volume against the channel's target, within tolerance percent.
US_UTIL_EXTERN RunCheckResult check_loading_volume( double volume, double target, double tolerance_percent );

//! \brief Expected run duration (seconds):  the first speed step's duration
//! plus its acceleration time, speed / accel.
US_UTIL_EXTERN double         run_duration_target( double step_duration, double speed, double accel );

//! \brief Run duration measured from the run's timestamps (minutes, checked
//! in seconds) against the target, within tolerance percent.
US_UTIL_EXTERN RunCheckResult check_run_duration( double measured_minutes, double target_seconds, double tolerance_percent );

//! \brief Average temperature of the first scan within 0.5 degrees C of the
//! protocol's temperature.
US_UTIL_EXTERN RunCheckResult check_temperature( double measured, double target );

//! \brief Average speed of the first scan within 4 rpm of the first speed
//! step's speed.
US_UTIL_EXTERN RunCheckResult check_speed( double measured, double target );
} // namespace Gmp
} // namespace US_Calc

#endif
