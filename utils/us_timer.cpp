#include "us_timer.h"
#include <cmath>
 
US_Timer::US_Timer()
{
   timers.clear();
   counts.clear();
   times.clear();
}

US_Timer::~US_Timer()
= default;

void US_Timer::init_timer( const QString& qs )
{
   timers[ qs ] = QElapsedTimer();
   counts[ qs ] = 0;
   times [ qs ] = 0l;
}

bool US_Timer::start_timer( const QString& qs )
{
   if ( !timers.count( qs ) )
   {
      return false;
   }

   timers[ qs ].restart();
   return true;
}

bool US_Timer::end_timer( const QString& qs )
{
   if ( !timers.count( qs ) )
   {
      return false;
   }

   counts[ qs ]++;
   const auto ul = static_cast<unsigned long>( timers[ qs ].elapsed() );
   times [ qs ] += ul;
   times2[ qs ] += ul * ul;
   return true;
}

QString US_Timer::list_time( const QString& qs )
{
   if ( !timers.count( qs ) )
   {
      return QString("US_Timer::Error: undefined timer: %1\n").arg( qs );
   }

   if ( !counts[ qs ] )
   {
      return QString("US_Timer::Error: no usage counts for timer: %1 \n").arg( qs );
   }

   double avg = static_cast<double>( times[ qs ] ) / counts[ qs ];

   return QString("%1 %2 calls total time %3 ms avg time %4 ms sd %5 ms \n")
      .arg( qs )
      .arg( counts[ qs ] )
      .arg( times [ qs ] )
      .arg( avg )
      .arg( sqrt( ( double )( 1e0 * times2[ qs ] / ( 1e0 * counts[ qs ] ) ) - avg * avg ) );
}

QString US_Timer::list_times()
{

   QString qs;
   for (const auto & timer : timers)
   {
      qs += list_time( timer.first );
   }
   return qs;
}
