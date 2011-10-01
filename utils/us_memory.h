//! \file us_memory.h
#ifndef US_MEMORY_H
#define US_MEMORY_H

#include <QtCore>

#include "us_extern.h"

//! \brief Platform-dependent resident memory (RSS) utilities
//!
//! This class provides utilites for monitoring memory usage.
//! All methods are static.
//!
class US_UTIL_EXTERN US_Memory
{
   public:
      //! \brief Return currently used RSS memory
      //!
      //! \returns      Currently used RSS in kilobytes
      static long int rss_now( void );

      //! \brief Update and return the maximum memory used
      //!
      //! \param rssmax Input/output maximum RSS in kilobytes
      //! \returns      Maximum RSS in kilobytes
      static long int rss_max( long int& rssmax );
};
#endif
