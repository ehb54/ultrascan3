//! \file us_edited_data_loaders.h
#ifndef US_EDITED_DATA_LOADERS_H
#define US_EDITED_DATA_LOADERS_H

#include <QtCore>

#include "ius_db2.h"
#include "us_dataIO.h"

/*! Program-side orchestration for loading one edited/raw data pair.

    These functions deliberately retain the selection rules of their callers;
    US_ConvertIO remains policy-free.  Accepting IUS_DB2 keeps the complete
    query/select/download/decode path testable without constructing either GUI
    or opening a live database connection.
*/
namespace US_EditedDataLoaders
{
   enum Status
   {
      INVALID_SELECTION = 1101,
      NO_USABLE_PAIR     = 1102
   };

   int loadAutoflow( IUS_DB2* db, const QString& tripleName,
                     const QString& runID, const QString& directory,
                     QVector< US_DataIO::EditedData >& editedData,
                     QVector< US_DataIO::RawData >& rawData,
                     int& editedDataID, QString& error );

   int loadReporter( IUS_DB2* db, const QString& tripleName,
                     const QString& runID, int requestedEditID,
                     const QString& directory,
                     QVector< US_DataIO::EditedData >& editedData,
                     QVector< US_DataIO::RawData >& rawData,
                     int& editedDataID, QString& editUpdated,
                     QString& error );
}

#endif
