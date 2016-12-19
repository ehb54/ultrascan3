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
   bool start_timer  ( QString );
   bool end_timer    ( QString );
   QString list_time ( QString );
   QString list_times( );

 private:

   map < QString, QTime         > timers;
   map < QString, unsigned int  > counts;
   map < QString, unsigned long > times;
   map < QString, unsigned long > times2;
};

#endif
