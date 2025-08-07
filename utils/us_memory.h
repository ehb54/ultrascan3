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
class US_UTIL_EXTERN US_Memory {
   public:
      //! \brief Return currently used RSS memory
      //!
      //! \returns      Currently used RSS in kilobytes
      static long int rss_now(void);

      //! \brief Update and return the maximum memory used
      //!
      //! \param rssmax Input/output maximum RSS in kilobytes
      //! \returns      Maximum RSS in kilobytes
      static long int rss_max(long int &rssmax);

      //! \brief Return memory profile: used and available
      //!
      //! \param pMemA Pointer for memory currently available in MB
      //! \param pMemT Pointer for memory total available in MB
      //! \param pMemU Pointer for memory currently in use in MB
      //! \returns     Memory available as rounded percent of total
      static int memory_profile(int * = 0, int * = 0, int * = 0);
};
#endif
