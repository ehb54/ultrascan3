//! \file us_solution_vals.h
#ifndef US_SOLVALS_H
#define US_SOLVALS_H

#include "us_extern.h"
#include "us_dataIO.h"
#include "us_db2.h"

//! \brief Fetch solution/buffer values (vbar, density, viscosity,
//! compressibility) for an edited data set.
//!
//! The primary method attempts to fetch common vbar-20, density,
//! viscosity, and compressibility strings for a single edited data set.
//! All methods are static.
//!
//! If database access is specified, links in the experiment data are
//! followed for solution and buffer records; if not successful, the local
//! access path is followed, as a backup.
//!
//! If local access is specified, information is taken from files in the
//! "solutions" and "buffers" subdirectories of the "data" directory. Files
//! are searched for ID values found in the experiment file of the runID
//! subdirectory of the "results" directory.

class US_UTIL_EXTERN US_SolutionVals
{
   public:
      //! \brief Fetch solution/buffer values for a data set
      //! \param  dbP       Database connection pointer or NULL for local
      //! \param  edata     Pointer to edited data set
      //! \param  soluID    Reference for returned solution ID string
      //! \param  cvbar20   Reference for returned common vbar-20 value string
      //! \param  density   Reference for returned density value string
      //! \param  viscosity Reference for returned viscosity value string
      //! \param  compress  Reference for returned compressibility string
      //! \param  manual    Reference for returned manual string ("0"/"1")
      //! \param  errmsg    Reference for returned error message string
      //! \return           Flag for successful fetch of all values
      static bool values( US_DB2* dbP, US_DataIO::EditedData*, QString&,
            QString&, QString&, QString&, QString&, QString&, QString& );

      //! \brief Get identification values for a data set from database
      //! \param  dbP       Pointer to opened database connection
      //! \param  edata     Pointer to edited data set
      //! \param  cvbar20   Reference for returned common vbar-20 value string
      //! \param  soluID    Reference for returned solution ID string
      //! \param  bufId     Reference for returned buffer db ID value string
      //! \param  bufGuid   Reference for returned buffer GUID value string
      //! \param  bufDesc   Reference for returned buffer description string
      //! \param  errmsg    Reference for returned error message string
      static bool solinfo_db( US_DB2* dbP, US_DataIO::EditedData*,
            QString&, QString&, QString&, QString&, QString&, QString& );
                                   
      //! \brief Get identification values for a data set from local disk
      //! \param  edata     Pointer to edited data set
      //! \param  cvbar20   Reference for returned common vbar-20 value string
      //! \param  soluID    Reference for returned solution ID string
      //! \param  bufId     Reference for returned buffer db ID value string
      //! \param  bufGuid   Reference for returned buffer GUID value string
      //! \param  bufDesc   Reference for returned buffer description string
      //! \param  errmsg    Reference for returned error message string
      static bool solinfo_disk( US_DataIO::EditedData*, QString&,
            QString&, QString&, QString&, QString&, QString& );
                                   
      //! \brief Get buffer values for a data set from database
      //! \param  dbP       Pointer to opened database connection
      //! \param  bufId     Reference for input buffer db ID value string
      //! \param  bufGuid   Reference for input buffer GUID value string
      //! \param  bufDesc   Reference for input buffer description string
      //! \param  density   Reference for returned density value string
      //! \param  viscosity Reference for returned viscosity value string
      //! \param  compress  Reference for returned compressibility string
      //! \param  manual    Reference for returned manual string ("0"/"1")
      //! \param  errmsg    Reference for returned error message string
      static bool bufvals_db( US_DB2* dbP, QString&, QString&, QString&,
            QString&, QString&, QString&, QString&, QString& );
                                   
      //! \brief Get buffer values for a data set from local disk
      //! \param  bufId     Reference for input buffer db ID value string
      //! \param  bufGuid   Reference for input buffer GUID value string
      //! \param  bufDesc   Reference for input buffer description string
      //! \param  density   Reference for returned density value string
      //! \param  viscosity Reference for returned viscosity value string
      //! \param  compress  Reference for returned compressibility string
      //! \param  manual    Reference for returned manual string ("0"/"1")
      //! \param  errmsg    Reference for returned error message string
      static bool bufvals_disk( QString&, QString&, QString&,
            QString&, QString&, QString&, QString&, QString& );
                                   
};

#endif
