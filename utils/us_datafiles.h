//! \file us_datafiles.h
#ifndef US_DATAFILES_H
#define US_DATAFILES_H

#include <QtCore>

#include "us_extern.h"

//! \brief Get output data file name. All functions are static.
//!
//! Get a file name for a data file. If a file exists in which the GUID
//! for the data is found, that file's name is used. Otherwise, a new file
//! path is returned, based on a file name with the next available numeric
//! part.
class US_UTIL_EXTERN US_DataFiles
{
  public:
     
   //! \brief Get a data file name by matching the GUID, finding a numeric gap,
   //! or incrementing to the next number past the last used. The file names
   //! follow a specific format: a leading character, followed by a seven-digit
   //! number, and ending with ".xml" (e.g., "M0000001.xml").
   //!
   //! \param path Path to data files directory (e.g., "dataDir()/models")
   //! \param guid GUID value to match
   //! \param lfchar Leading file character ("M", "N", ...)
   //! \param lkupTag Look-up tag ("model", "noise", ...)
   //! \param lkupAtt Look-up attribute ("guid", "modelGUID", ...)
   //! \param newFile Returned new-file flag. This will be set to true if a new file
   //! is created, or false if an existing file with a matching GUID is found.
   //! \return Full path name for new or old file to write
   //!
   //! The method searches for existing files in the specified directory that match
   //! the given GUID. It looks for a specific XML tag and attribute within each file.
   //! If a matching tag and attribute are found, the file is considered an existing file.
   //! If no matching file is found, the method will look for gaps in the numeric sequence
   //! of existing files. A gap is defined as a missing number in the sequence of file names
   //! (e.g., if "M0000002.xml" and "M0000004.xml" exist, there is a gap for "M0000003.xml").
   //! If no gaps are found, it will create a new file name by incrementing the highest existing
   //! number by one.
   static QString get_filename( const QString&, const QString&,
         const QString&, const QString&, const QString&,
         bool& );

   //! \brief Get an output data file name. Version with no newFile return.
   //! \param path     Full path to data files directory
   //! \param guid     Global ID of object to match
   //! \param lfchar   Leading file character ("M", "S", ...)
   //! \param lkupTag  Look-up XML tag ("model", "analyte", ...)
   //! \param lkupAtt  Look-up Attribute ("guid", "modelGUID", ...)
   //! \returns        Full path name of file to which to write
   static QString get_filename( const QString&, const QString&,
         const QString&, const QString&, const QString& );
};
#endif

