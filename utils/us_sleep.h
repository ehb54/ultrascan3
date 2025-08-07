//! \file us_sleep.h
#ifndef US_SLEEP_H
#define US_SLEEP_H

#include <QtCore>

//! \brief A static only class to provide sleep functions,
//! primarily for Windows.  Although this file is in the
//! utils library directory, it is not included in the library
//! because everything that is needed is in the header.

class US_Sleep : public QThread {
   public:
      /*! \brief A static function that sleeps seconds
        \param secs Seconds to sleep
    */

      static void sleep(unsigned long secs) { QThread::sleep(secs); }

      /*! \brief A static function that sleeps milliseconds
        \param msecs Milliseconds to sleep
    */

      static void msleep(unsigned long msecs) { QThread::msleep(msecs); }

      /*! \brief A static function that sleeps microseconds
        \param usecs Microseconds to sleep
    */

      static void usleep(unsigned long usecs) { QThread::usleep(usecs); }
};

#endif
