//! \file us_reporter_gmp_duration.h
//! \brief Durations as the GMP report shows them.
#ifndef US_REPORTER_GMP_DURATION_H
#define US_REPORTER_GMP_DURATION_H

#include <QString>
#include <QtMath>

//! Free functions of the GMP reporter.
namespace US_ReporterGmp
{
//! \brief A duration in seconds as "<hours> h <minutes> m":  whole hours
//! (floored), then the rest rounded to minutes, so a rest of 59.5 minutes or
//! more shows as 60 m.
inline QString duration_hours_minutes( double seconds )
{
   const int hours   = static_cast<int>( qFloor( seconds / 3600.0 ) );
   const int minutes = static_cast<int>( qRound( ( seconds - hours * 3600.0 ) / 60.0 ) );
   return QString::asprintf( "%d h %02d m", hours, minutes );
}
} // namespace US_ReporterGmp

#endif
