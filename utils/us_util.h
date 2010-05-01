//! \file us_util.h
#ifndef US_UTIL_H
#define US_UTIL_H

#include <QtCore>

#include "us_extern.h"

//! \brief General utilities for UltraScan
class US_EXTERN US_Util
{
   public:
      //! \brief Returns the next string component where the componenets
      //!        are separated by a common delimiter.  The function modifies
      //!        the passed string.  
      //! \param s String to search
      //! \param separator Delimiter to use for parsing
      static QString getToken( QString&, const QString& );


      // \brief Generates and returns a new Global Identifier (guid)
      static QString new_guid( void );
};
#endif
