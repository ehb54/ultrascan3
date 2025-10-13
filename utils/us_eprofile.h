//! \file us_eprofile.h
#ifndef US_EXTPROF_H
#define US_EXTPROF_H

#include <QtCore>
#include "us_extern.h"
#include "us_db2.h"

//! \brief A set of utility functions gets, creates, updates, or
//! deletes an extinction profile in the database or to an XML string
//! for writing to a local disk.
//!
//! The persistent form of a profile in the database or on local disk
//! is an XML string with mutiple tags having "wavelength" and "value"
//! attributes.
//!
//! The returned or input profile object is a QMap<double,double> with
//! keys of wavelength values and values of absorbance or extinction.

class US_UTIL_EXTERN US_ExtProfile
{
   public:
      //! \brief Fetch an extinction profile from the database
      //! \param  dbP        Database connection pointer
      //! \param  compID     Component (Buffer|Analyte|Experiment) ID
      //! \param  compType   Component type ("Buffer"|"Analyte"|"Sample")
      //! \param  valType    Reference for returned value type
      //! \param  extinction Reference for returned extinction profile
      //! \return            Profile ID of fetched profile (-1 if none)
      static int fetch_eprofile( IUS_DB2*, const int, QString&, QString&,
                                 QMap< double, double >& );

      //! \brief Parse an extinction profile from an XML string
      //! \param  epxml      Extinction profile XML string
      //! \param  compID     Pointer for returned component ID
      //! \param  compType   Pointer for returned component Type
      //! \param  valType    Pointer for returned value Type
      //! \param  extinction Reference for returned extinction profile
      //! \return            Count of profiles found (1 or 0)
      //! \return            Flag of parse success
      static bool parse_eprofile( QString&, int*, QString*, QString*,
                                  QMap< double, double >& );

      //! \brief Create (or update) a new extinction profile in the database
      //! \param  dbP        Database connection pointer
      //! \param  compID     Component ID
      //! \param  compType   Component type ("Buffer"|"Analyte"|"Sample")
      //! \param  valueType  Value type ("absorbance"|"molarExtinction"|
      //!                    "massExtinction"|(other))
      //! \param  extinction Extinction profile to store
      //! \return            Profile ID of created profile record (-1 if none)
      static int create_eprofile( IUS_DB2*, const int, QString&, QString&,
                                  QMap< double, double >& );

      //! \brief Create a new extinction profile XML string
      //! \param  compID     Component ID
      //! \param  compType   Component type ("Buffer"|"Analyte"|"Sample")
      //! \param  valueType  Value type ("absorbance"|"molarExtinction"|
      //!                    "massExtinction"|(other))
      //! \param  extinction Extinction profile to store
      //! \param  xmlout     Reference for XML string to create
      //! \return            Flag of creation success
      static bool xml_eprofile( const int, QString&, QString&,
                                QMap< double, double >&, QString& );

      //! \brief Update an extinction profile in the database
      //! \param  dbP        Database connection pointer
      //! \param  profileID  Profile ID of record to update
      //! \param  compID     Component ID
      //! \param  compType   Component type ("Buffer"|"Analyte"|"Sample")
      //! \param  valueType  Value type ("absorbance"|"molarExtinction"|
      //!                    "massExtinction"|(other))
      //! \param  extinction Extinction profile to store
      //! \return            Flag of update success
      static bool update_eprofile( IUS_DB2*, const int, const int, QString&,
                                   QString&, QMap< double, double >& );

      //! \brief Delete an extinction profile from the database
      //! \param  dbP        Database connection pointer
      //! \param  compID     Component (or Profile) ID
      //! \param  compType   Component type ("Buffer"|"Analyte"|"Sample"
      //!                    |"Profile"; "Profile" -> compID is profileID)
      //! \return            Flag of deletion success
      static bool delete_eprofile( IUS_DB2*, const int, QString& );

};

#endif
