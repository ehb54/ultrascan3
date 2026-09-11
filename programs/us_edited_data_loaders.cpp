//! \file us_edited_data_loaders.cpp

#include "us_edited_data_loaders.h"

#include "us_convertio.h"

namespace
{
void clearResult( QVector< US_DataIO::EditedData >& editedData,
                  QVector< US_DataIO::RawData >& rawData,
                  int& editedDataID, QString& error )
{
   editedData.clear();
   rawData.clear();
   editedDataID = -1;
   error.clear();
}

int queryFailure( IUS_DB2* db, const QString& tripleName, QString& error )
{
   error = QString( "Could not look up edited data for triple %1:\n%2" )
              .arg( tripleName, db->lastError() );
   return db->lastErrno();
}
}

int US_EditedDataLoaders::loadAutoflow(
   IUS_DB2* db, const QString& tripleName, const QString& runID,
   const QString& directory,
   QVector< US_DataIO::EditedData >& editedData,
   QVector< US_DataIO::RawData >& rawData, int& editedDataID,
   QString& error )
{
   clearResult( editedData, rawData, editedDataID, error );

   if ( db == nullptr  ||  tripleName.isEmpty()  ||  runID.isEmpty() )
   {
      error = "Autoflow edited-data selection is incomplete.";
      return INVALID_SELECTION;
   }

   QStringList query;
   query << "get_editedDataFilenamesIDs" << runID;
   db->query( query );

   if ( db->lastErrno() != IUS_DB2::OK )
      return queryFailure( db, tripleName, error );

   QString tripleNameActual = tripleName;
   if ( tripleNameActual.contains( "Interference" ) )
      tripleNameActual.replace( "Interference", "660" );

   QDateTime selectedDate;
   QString   editFilename;
   int       selectedEditID = -1;
   int       rawDataID      = -1;

   while ( db->next() )
   {
      const QString filename = db->value( 0 ).toString();
      const QDateTime date   = db->value( 3 ).toDateTime();

      if ( filename.contains( tripleNameActual )
           &&  ( ! selectedDate.isValid()  ||  date > selectedDate ) )
      {
         selectedDate  = date;
         editFilename  = filename;
         selectedEditID = db->value( 1 ).toInt();
         rawDataID      = db->value( 2 ).toInt();
      }
   }

   if ( editFilename.isEmpty()  ||  selectedEditID < 1  ||  rawDataID < 1 )
   {
      error = QString( "No usable edited/raw data pair was found for triple %1." )
                 .arg( tripleName );
      return NO_USABLE_PAIR;
   }

   US_ConvertIO::EditedDataReadRequest request;
   request.directory    = directory;
   request.editFilename = editFilename;
   request.editedDataID = selectedEditID;
   request.rawDataID    = rawDataID;

   const int status = US_ConvertIO::readEditedDataFromDB(
      db, request, editedData, rawData, error );

   if ( status == IUS_DB2::OK )
      editedDataID = selectedEditID;

   return status;
}

int US_EditedDataLoaders::loadReporter(
   IUS_DB2* db, const QString& tripleName, const QString& runID,
   int requestedEditID, const QString& directory,
   QVector< US_DataIO::EditedData >& editedData,
   QVector< US_DataIO::RawData >& rawData, int& editedDataID,
   QString& editUpdated, QString& error )
{
   clearResult( editedData, rawData, editedDataID, error );
   editUpdated.clear();

   if ( db == nullptr  ||  tripleName.isEmpty()  ||  runID.isEmpty()
        ||  requestedEditID < 1 )
   {
      error = requestedEditID < 1
            ? QString( "No edited-data record is linked to triple %1." )
                 .arg( tripleName )
            : QString( "Reporter edited-data selection is incomplete." );
      return INVALID_SELECTION;
   }

   QStringList query;
   query << "get_editedDataFilenamesIDs_forReport" << runID
         << QString::number( requestedEditID );
   db->query( query );

   if ( db->lastErrno() != IUS_DB2::OK )
      return queryFailure( db, tripleName, error );

   QString editFilename;
   QString selectedUpdated;
   int     rawDataID = -1;

   while ( db->next() )
   {
      if ( db->value( 1 ).toInt() != requestedEditID )
         continue;

      editFilename   = db->value( 0 ).toString();
      rawDataID      = db->value( 2 ).toInt();
      selectedUpdated = db->value( 3 ).toString();
   }

   if ( editFilename.isEmpty()  ||  rawDataID < 1 )
   {
      error = QString( "No usable edited/raw data pair was found for triple %1 "
                       "and editedData ID %2." )
                 .arg( tripleName ).arg( requestedEditID );
      return NO_USABLE_PAIR;
   }

   US_ConvertIO::EditedDataReadRequest request;
   request.directory    = directory;
   request.editFilename = editFilename;
   request.editedDataID = requestedEditID;
   request.rawDataID    = rawDataID;

   const int status = US_ConvertIO::readEditedDataFromDB(
      db, request, editedData, rawData, error );

   if ( status == IUS_DB2::OK )
   {
      editedDataID = requestedEditID;
      editUpdated  = selectedUpdated;
   }

   return status;
}
