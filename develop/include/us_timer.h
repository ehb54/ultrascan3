#ifndef US_TIMER_H
#define US_TIMER_H

#include <qstring.h>
#include <qdatetime.h>
#include <map>

using namespace std;

class US_Timer
{
 public:

   US_Timer();
   ~US_Timer();

   void init_timer   ( QString );
   void clear_timer  ( QString );
   bool start_timer  ( QString );
   bool end_timer    ( QString );
   bool stop_timer   ( QString );
   QString list_time ( QString );
   QString list_times( QString prepend = "" );
   void clear_timers ();
   void end_all      ();
   void stop_all     ();
   void start_all    ();

   map < QString, unsigned long > times;
   map < QString, QTime         > timers;
   map < QString, unsigned int  > counts;
   map < QString, unsigned long > times2;
};

#endif
