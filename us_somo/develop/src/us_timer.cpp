#include "../include/us_timer.h"
#include <math.h>

US_Timer::US_Timer()
{
   timers.clear( );
   counts.clear( );
   times.clear( );
}

US_Timer::~US_Timer()
{
   timers.clear( );
   counts.clear( );
   times.clear( );
   times2.clear( );
}

void US_Timer::clear_timers()
{
}

void US_Timer::clear_timer( QString qs )
{
   timers.erase( qs );
   counts.erase( qs );
   times .erase( qs );
   times2.erase( qs );
}

void US_Timer::init_timer( QString qs )
{
   timers[ qs ] = QElapsedTimer();
   counts[ qs ] = 0;
   times [ qs ] = 0l;
}

bool US_Timer::start_timer( QString qs )
{
   if ( !timers.count( qs ) )
   {
      return false;
   }

   timers[ qs ].start();
   return true;
}

bool US_Timer::end_timer( QString qs )
{
   if ( !timers.count( qs ) )
   {
      return false;
   }

   counts[ qs ]++;
   unsigned long ul = (unsigned long) timers[ qs ].elapsed();
   times [ qs ] += ul;
   times2[ qs ] += ul * ul;
   return true;
}


bool US_Timer::stop_timer( QString qs )
{
   if ( !timers.count( qs ) )
   {
      return false;
   }

   unsigned long ul = (unsigned long) timers[ qs ].elapsed();
   times [ qs ] += ul;
   times2[ qs ] += ul * ul;
   return true;
}

QString US_Timer::time_min_sec( QString qs )
{
   if ( !timers.count( qs ) )
   {
      return QString("US_Timer::Error: undefined timer: %1\n").arg( qs );
   }

   if ( !counts[ qs ] )
   {
      return QString("US_Timer::Error: no usage counts for timer: %1\n").arg( qs );
   }

   int minutes    = floor( (double) times[ qs ] / 60000.0 );
   double seconds = ( times[ qs ] - ( minutes * 60000 ) ) / 1000.0;
   return QString( "%1m %2s" ).arg( minutes ).arg( seconds );
}

QString US_Timer::list_time( QString qs )
{
   if ( !timers.count( qs ) )
   {
      return QString("US_Timer::Error: undefined timer: %1\n").arg( qs );
   }

   if ( !counts[ qs ] )
   {
      return QString("US_Timer::Error: no usage counts for timer: %1\n").arg( qs );
   }

   double avg = ( double ) times [ qs ] / counts[ qs ];

   return
      counts[ qs ] == 1 ?
      QString("%1 %2 calls total time %3 ms\n")
      .arg( qs )
      .arg( counts[ qs ] )
      .arg( times [ qs ] )
      :
      ( counts[ qs ] == 2 ?
        QString("%1 %2 calls total time %3 ms avg time %4 ms sd %5 ms\n")
        .arg( qs )
        .arg( counts[ qs ] )
        .arg( times [ qs ] )
        .arg( avg ) 
        :
        QString("%1 %2 calls total time %3 ms avg time %4 ms sd %5 ms\n")
        .arg( qs )
        .arg( counts[ qs ] )
        .arg( times [ qs ] )
        .arg( avg )
        .arg( sqrt( ( double )( 1e0 * times2[ qs ] / ( 1e0 * counts[ qs ] ) ) - avg * avg ) )
        )
      ;
}

QString US_Timer::list_times( QString prepend )
{

   QString qs;
   for ( map < QString, QElapsedTimer >::iterator it = timers.begin();
         it != timers.end();
         it++ )
   {
      qs += prepend + list_time( it->first );
   }
   return qs;
}

void US_Timer::end_all()
{
   for ( map < QString, QElapsedTimer >::iterator it = timers.begin();
         it != timers.end();
         it++ )
   {
      end_timer( it->first );
   }
}

void US_Timer::stop_all()
{
   for ( map < QString, QElapsedTimer >::iterator it = timers.begin();
         it != timers.end();
         it++ )
   {
      stop_timer( it->first );
   }
}

void US_Timer::start_all()
{
   for ( map < QString, QElapsedTimer >::iterator it = timers.begin();
         it != timers.end();
         it++ )
   {
      start_timer( it->first );
   }
}
