//! \file us_util.h
#ifndef US_UTIL_H
#define US_UTIL_H

#include <QtCore>

#include "us_extern.h"

//! \brief General utilities for UltraScan
//!
//! This class provides a number of general utility functions.
//! All methods are static.
//!
class US_EXTERN US_Util
{
   public:
      //! \brief Returns the next string component where the components
      //!        are separated by a common delimiter. The function modifies
      //!        the passed string.
      //!
      //! This function is designed to be called iteratively. On each call,
      //! the leading substring in the given string is returned and the
      //! full string is modified by having that substring and the 
      //! immediately following delimiter removed.
      //!
      //! \param s         String to search
      //! \param separator Delimiter to use for parsing
      //! \returns         Next token in the string (substring before delimiter)
      static QString getToken( QString&, const QString& );


      //! \brief Generates and returns a new Global Identifier (guid)
      //!
      //! This function generates a new Global Identifier (guid) string.
      //! The returned string is 36 characters in length and is in the
      //! standard uuid form: "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
      //!
      //! \returns         New GUID 36-character string
      static QString new_guid( void );

      //! \brief Generates a string for a file with md5 hash and file size
      //!
      //! An MD5 hash is performed on the file whose full path name is given.
      //! The size of the file in bytes is also determined. The returned
      //! string combines the hash and size separated by a blank: "hash size".
      //!
      //! \param filename  Pull path name of file for md5 hash and size
      //! \returns         A string in the form of "hash size" for the file
      static QString md5sum_file( QString );

      //! \brief Converts a DateTime string to unambiguous UTC form
      //!
      //! This function converts a DateTime string into an unambiguous UTC
      //! form, as would be output by the Unix "date" command with arguments
      //! of "-u +%F\ %T\ %Z"; a "yyyy-mm-dd HH:MM:SS UTC" form. The input
      //! DateTime string may be in ISO form (e.g., "2010-08-10T09:42:16"),
      //! already in UTC form, or in the default locale format, such as
      //! "Mon Aug 9 23:24:45 2010". A boolean argument specifies whether
      //! the implied time is already in UTC. If "false" and no " UTC" is in
      //! the input string, the underlying DateTime is converted from local
      //! time to Universal Time Coordinates. No time conversion occurs if
      //! the boolean argument is "true".
      //!
      //! \param dttext    DateTime text string in ISO or other form
      //! \param knownUTC  A boolean whether the input string is known to
      //!                  represent date and time in Universal Time Coordinates.
      //! \returns         A DateTime string in "yyyy-mm-dd HH:MM:SS UTC" form
      static QString toUTCDatetimeText( QString, bool );

      //! \brief Converts a DateTime string to Qt's ISODate form
      //!
      //! This function converts a DateTime string into the form created
      //! or expected with a Qt::DateFormat of Qt::ISODate. That is, the
      //! output is something like "2010-08-10T09:42:16". The input may
      //! already be in ISO form; may be in UTC form; or may be an unknown
      //! format assumed to be Qt:TextDate, such as "Mon Aug 9 23:24:45 2010".
      //!
      //! \param dttext    DateTime text string in UTC, ISO, or other form
      //! \returns         A DateTime string in ISO form ("2010-08-14T21:41:22")
      static QString toISODatetimeText( QString );
};
#endif
