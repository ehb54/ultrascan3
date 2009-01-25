//! \file us_license_t.h
#ifndef US_SLEEP_H
#define US_SLEEP_H

#include <QtCore>

//! \brief A static only class to provide sleep functions, 
//! primarily for Windows.  Although this file is in the
//! utils library directoy, it is not included in the library
//! because everything that is needed is in the header.

class US_Sleep : public QThread
{
  public:

    /*! \brief A static function that sleeps seconds
        \param Seconds to sleep
    */

    static void sleep( unsigned long secs )
    {
       QThread::sleep( secs );
    }

    /*! \brief A static function that sleeps milliseconds
        \param Milliseconds to sleep
    */

    static void msleep( unsigned long msecs ) 
    {
       QThread::msleep( msecs );
    }

    /*! \brief A static function that sleeps microseconds
        \param Microseconds to sleep
    */

    static void usleep( unsigned long usecs ) 
    {
       QThread::usleep( usecs );
    }
};

#endif
