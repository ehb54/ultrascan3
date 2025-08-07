//! \file us_util.h
#ifndef US_UTIL_H
#define US_UTIL_H

#include <QtCore>

#include "us_extern.h"

#ifndef DbgErr
#define DbgErr(a) \
  if (US_Util::ithTime(a)) qDebug()  //!< Ith time error qDebug()
#endif

//! \brief General utilities for UltraScan
//!
//! This class provides a number of general utility functions.
//! All methods are static.
//!
class US_UTIL_EXTERN US_Util {
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
  static QString getToken(QString&, const QString&);

  //! \brief Generates and returns a new Global Identifier (guid)
  //!
  //! This function generates a new Global Identifier (guid) string.
  //! The returned string is 36 characters in length and is in the
  //! standard uuid form: "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
  //!
  //! \returns         New GUID 36-character string
  static QString new_guid(void);

  //! \brief Generates a string for a file with md5 hash and file size
  //!
  //! An MD5 hash is performed on the file whose full path name is given.
  //! The size of the file in bytes is also determined. The returned
  //! string combines the hash and size separated by a blank: "hash size".
  //!
  //! \param filename  Pull path name of file for md5 hash and size
  //! \returns         A string in the form of "hash size" for the file
  static QString md5sum_file(QString);

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
  static QString toUTCDatetimeText(QString, bool);

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
  static QString toISODatetimeText(QString);

  //! \brief Convert a QString uuid to a 16-byte binary character array
  //!
  //! \param in Uuid string to be converted
  //! \param uu A pointer to a 16-byte character array for the
  //!           converted data
  static void uuid_parse(const QString&, unsigned char*);

  //! \brief Convert a binary uuid to a QString
  //!
  //! \param uu A pointer to a 16-byte array holding a binary uuid
  static QString uuid_unparse(unsigned char*);

  //! \brief Create a compressed (ccw) form of a triple (c/c/w) string.
  //!
  //! \param ccw Expanded triple string ("c/c/w" or "c / c / w")
  //! \returns   Compressed form of the triple string ("ccw")
  static QString compressed_triple(const QString&);

  //! \brief Create an expanded (c/c/w) form of a triple (ccw) string.
  //!
  //! \param ccw    Compressed triple string ("ccw")
  //! \param spaces Flag to insert spaces around slashes.
  //! \returns      Uncompressed triple string ("c/c/w" or "c / c / w")
  static QString expanded_triple(const QString&, bool = true);

  //! \brief Return a flag if this is the i'th time an error occurs.
  //!
  //! \param timeinc Incidence increment to detect (default=10000).
  //! returns        Boolean flag if this is i'th time.
  static bool ithTime(int = 10000);

  //! \brief Return a flag if an XML attribute string implies true/false.
  //!
  //! \param xmlattr XML attribute string ("T", "F", "1", "0").
  //! returns        Boolean flag if the implied boolean is true or false.
  static bool bool_flag(const QString);

  //! \brief Return an XML attribute string for true or false.
  //!
  //! \param boolval Boolean value to represent as an XML attribute string.
  //! returns        XML attribute string representation ("0" or "1").
  static QString bool_string(const bool);

  //! \brief Return a string representing a QList<QStringList>.
  //!
  //! \param lsl      Input list of stringlists to represent
  //! \param llstring Output list of lists string representation
  //! returns         Count of stringlists in list
  static int listlistBuild(QList<QStringList>&, QString&);

  //! \brief Return a QList<QStringList> as represented in a string.
  //!
  //! \param lsl      Output list of stringlists represented
  //! \param llstring Input list of lists string representation
  //! returns         Count of stringlists in list
  static int listlistParse(QList<QStringList>&, QString&);

 private:
  static unsigned char hex2int(unsigned char c);
};
#endif
