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
class US_EXTERN US_DataFiles
{
  public:
     
   //! \brief Get an output data file name: existing or next available name
   //! \param path     Full path to data files directory
   //! \param guid     Global ID of object to match
   //! \param lfchar   Leading file character ("M", "S", ...)
   //! \param lkupTag  Look-up XML tag ("model", "analyte", ...)
   //! \param lkupAtt  Look-up Attribute ("guid", "modelGUID", ...)
   //! \param newFile  Reference for returned flag if file is new
   //! \returns        Full path name of file to which to write
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

