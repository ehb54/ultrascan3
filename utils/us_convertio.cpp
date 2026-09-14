//! \file us_convertio.cpp

#include <QDomDocument>
#include <QRegularExpression>

#include "us_settings.h"
#include "us_db2.h"
#include "us_convert.h"
#include "us_convertio.h"
#include "us_experiment.h"
#include "us_util.h"

// Generic constructor
US_ConvertIO::US_ConvertIO( void )
{
}

// ----------------------------------------------------------------------------
// Reading one run directory.
//
// US_ConvertGui and the corpus loader both used to open their own copy of this
// sequence.  What they legitimately differ on is how strict to be, so the
// sequence is shared and the strictness is not: this reports and the caller
// decides.
// ----------------------------------------------------------------------------

namespace
{
//! The run ID rule US_ConvertGui has always applied to a directory basename.
bool runIdIsUsable( const QString& runID )
{
   static const QRegularExpression rx( "^[A-Za-z0-9_-]{1,80}$" );

   return rx.match( runID ).hasMatch();
}

//! "runID.runType." prefix fields of an AUC basename, or empty if malformed.
QPair< QString, QString > runFieldsOf( const QString& filename )
{
   const QStringList parts = filename.split( "." );

   if ( parts.size() < 5 )
      return QPair< QString, QString >();

   return QPair< QString, QString >( parts[ 0 ], parts[ 1 ] );
}
}

int US_ConvertIO::readDiskRun( const QString& directory, DiskRun& run,
                               QString& error )
{
   run = DiskRun();
   error.clear();

   // US_Convert::readUS3Disk() builds each path as `dir + filename` with no
   // separator of its own, so the separator has to be here.  Without it the
   // read gets past the empty-directory check and then fails opening
   // "<dir><filename>", reporting a caller's path mistake as a corrupt file.
   QString dir = QString( directory ).replace( "\\", "/" );

   while ( dir.endsWith( "/" ) )
      dir.chop( 1 );

   run.runID     = dir.section( "/", -1, -1 );
   run.directory = dir + "/";

   if ( ! runIdIsUsable( run.runID ) )
   {
      error = QString( "\"%1\" cannot be a run ID: a run directory name may "
                       "hold only letters, digits, the underscore and the "
                       "hyphen, and at most 80 of them" ).arg( run.runID );
      return US_Convert::INVALID_RUN;
   }

   const int rawStatus = US_Convert::readUS3Disk( run.directory, run.rawData,
                                                  run.triples, run.runType );

   if ( rawStatus != US_Convert::OK )
   {
      error = ( rawStatus == US_Convert::NODATA )
              ? QString( "No .auc files were found in %1" ).arg( run.directory )
              : QString( "A data file in %1 could not be read" )
                .arg( run.directory );
      return rawStatus;
   }

   // Everything downstream indexes scans.  US_SimulationParameters::
   // computeSpeedSteps() in particular dereferences (*scans)[0] with no size
   // guard, so a dataset with no scans has to be caught here, before any
   // consumer of this run reaches it.
   if ( run.rawData.isEmpty() )
   {
      error = QString( "%1 holds no raw data" ).arg( run.directory );
      return US_Convert::INVALID_RUN;
   }

   for ( int ii = 0; ii < run.rawData.size(); ii++ )
   {
      if ( run.rawData[ ii ].scanData.isEmpty() )
      {
         error = QString( "The data file for %1 has no scans" )
                 .arg( run.triples[ ii ].tripleDesc );
         return US_Convert::INVALID_RUN;
      }
   }

   // Filenames carrying more than one run ID or run type.  Reported, not
   // rejected: US_Convert::readUS3Disk() takes the first file's run type for
   // the whole run and says nothing, which is how a directory holding two
   // runs reads as one.
   QStringList runIDs;
   QStringList runTypes;
   QDir        listing( run.directory );

   for ( const QString& name : listing.entryList( QStringList( "*.auc" ),
                                                  QDir::Files | QDir::Readable,
                                                  QDir::Name ) )
   {
      const QPair< QString, QString > fields = runFieldsOf( name );

      if ( fields.first.isEmpty() )
         continue;

      if ( ! runIDs.contains( fields.first ) )
         runIDs << fields.first;

      if ( ! runTypes.contains( fields.second ) )
         runTypes << fields.second;
   }

   // Held rather than returned.  Reading stops here only for things that make
   // the rest of the read impossible or unsafe; a directory holding two runs
   // is neither, and US_ConvertGui loads such an archive today with its
   // experiment metadata intact.  Returning now would take that away.
   QString mixedDetail;

   if ( runTypes.size() > 1 )
      mixedDetail = QString( "the data files name more than one run type "
                             "(%1); only %2 was read" )
                    .arg( runTypes.join( ", " ), run.runType );

   else if ( runIDs.size() > 1 )
      mixedDetail = QString( "the data files name more than one run (%1)" )
                    .arg( runIDs.join( ", " ) );

   QString detail;
   const int expStatus = run.experiment.readFromDisk(
      run.triples, run.runType, run.runID, run.directory,
      run.speedSteps, detail );

   if ( expStatus != US_Convert::OK )
   {
      const QString file = run.runID + "." + run.runType + ".xml";

      if ( expStatus == US_Convert::CANTOPEN )
         error = QString( "The experiment record %1 could not be opened" )
                 .arg( run.directory + file );

      else if ( expStatus == US_Convert::BADXML )
         error = QString( "The experiment record %1 is not well-formed XML" )
                 .arg( run.directory + file );

      else
         error = QString( "%1 and the data files in %2 disagree: %3" )
                 .arg( file, run.directory, detail );

      // A directory holding two runs explains a dataset mismatch, so it is
      // named ahead of the mismatch it caused.
      if ( ! mixedDetail.isEmpty() )
      {
         if ( expStatus == US_Convert::CANTOPEN
              ||  expStatus == US_Convert::BADXML )
         {  // The record itself is unreadable, which no amount of run mixing
            // explains away.  Report that, and the mixing alongside it.
            error += QString( ". Also, %1" ).arg( mixedDetail );
            return expStatus;
         }

         error = QString( "The run in %1 is not one run: %2. As a result, %3" )
                 .arg( run.directory, mixedDetail, detail );
         return US_Convert::INVALID_RUN;
      }

      return expStatus;
   }

   if ( ! mixedDetail.isEmpty() )
   {
      error = QString( "The run in %1 is not one run: %2" )
              .arg( run.directory, mixedDetail );
      return US_Convert::INVALID_RUN;
   }

   // Every triple now has a dataset, so each must name the raw data it sits
   // beside.  A triple with a zero GUID is the dangerous direction: nothing
   // downstream checks it, and writeRawDataToDB() would unparse and store it.
   for ( int ii = 0; ii < run.triples.size(); ii++ )
   {
      const US_Convert::TripleInfo& triple = run.triples[ ii ];

      if ( triple.tripleFilename.isEmpty() )
      {
         error = QString( "The experiment record names no data file for %1" )
                 .arg( triple.tripleDesc );
         return US_Convert::PARTIAL_XML;
      }

      const QString tripleGUID = US_Util::uuid_unparse(
         (unsigned char*)const_cast< char* >( triple.tripleGUID ) );
      const QString rawGUID = US_Util::uuid_unparse(
         (unsigned char*)run.rawData[ ii ].rawGUID );

      if ( tripleGUID != rawGUID )
      {
         error = QString( "The experiment record gives %1 the identity %2, "
                          "but its data file carries %3" )
                 .arg( triple.tripleDesc, tripleGUID, rawGUID );
         return US_Convert::BADGUID;
      }
   }

   return US_Convert::OK;
}

// ----------------------------------------------------------------------------
// Edited-data database mechanics.
//
// Three programs used to carry their own copy of this: us_edit, us_manage_data
// and the corpus loader.  They agreed on the stored procedures and disagreed on
// everything else -- one raised dialogs, one wrote status codes into a table
// model, one returned a message -- so the mechanics could not be shared without
// dragging one program's policy into the others.
//
// These take an IUS_DB2*, return the underlying database status, and put
// context into `error`.  They decide nothing: not whether a record is created
// or updated, not whether a missing raw record is fatal, not what the user is
// told, and not where a transaction begins or ends.
// ----------------------------------------------------------------------------

namespace
{
// Guard shared by every operation below.  A null connection is the caller's
// mistake, and reporting it as NOT_CONNECTED keeps it inside the status space
// the callers already handle.
bool haveConnection( IUS_DB2* db, const QString& what, QString& error )
{
   if ( db != nullptr )
      return true;

   error = QString( "%1: no database connection" ).arg( what );
   return false;
}
}

int US_ConvertIO::findRawDataId( IUS_DB2* db, const QString& rawGUID,
                                 int& rawDataID, QString& error )
{
   if ( ! haveConnection( db, "raw data lookup", error ) )
      return IUS_DB2::NOT_CONNECTED;

   QStringList query;
   query << "get_rawDataID_from_GUID" << rawGUID;
   db->query( query );

   const int status = db->lastErrno();

   if ( status != IUS_DB2::OK )
   {
      error = QString( "Could not look up the raw data record for GUID %1: "
                       "%2 (status %3)" )
              .arg( rawGUID, db->lastError() ).arg( status );
      return status;
   }

   // A successful query that names no row.  Reported separately from a query
   // failure because the two mean different things to a caller: one is a
   // broken connection, the other is data that was never uploaded.
   if ( ! db->next() )
   {
      error = QString( "No raw data record exists for GUID %1" ).arg( rawGUID );
      return IUS_DB2::NO_RAWDATA;
   }

   rawDataID = db->value( 0 ).toInt();

   return IUS_DB2::OK;
}

int US_ConvertIO::createEditedData( IUS_DB2* db,
                                    const EditedDataRecord& record,
                                    int& editedDataID, QString& error )
{
   if ( ! haveConnection( db, "edited data creation", error ) )
      return IUS_DB2::NOT_CONNECTED;

   QStringList query;
   query << "new_editedData"
         << QString::number( record.rawDataID )
         << record.editGUID
         << record.label
         << record.filename
         << record.comment;
   db->query( query );

   const int status = db->lastErrno();

   if ( status != IUS_DB2::OK )
   {
      error = QString( "Could not create the editedData record for %1: "
                       "%2 (status %3)" )
              .arg( record.filename, db->lastError() ).arg( status );
      return status;
   }

   editedDataID = db->lastInsertID();

   return IUS_DB2::OK;
}

int US_ConvertIO::updateEditedData( IUS_DB2* db, int editedDataID,
                                    const EditedDataRecord& record,
                                    QString& error )
{
   if ( ! haveConnection( db, "edited data update", error ) )
      return IUS_DB2::NOT_CONNECTED;

   QStringList query;
   query << "update_editedData"
         << QString::number( editedDataID )
         << QString::number( record.rawDataID )
         << record.editGUID
         << record.label
         << record.filename
         << record.comment;
   db->query( query );

   const int status = db->lastErrno();

   if ( status != IUS_DB2::OK )
   {
      error = QString( "Could not update editedData record %1 for %2: "
                       "%3 (status %4)" )
              .arg( editedDataID ).arg( record.filename, db->lastError() )
              .arg( status );
      return status;
   }

   return IUS_DB2::OK;
}

int US_ConvertIO::uploadEditedDataBlob( IUS_DB2* db, int editedDataID,
                                        const QString& filename,
                                        QString& error )
{
   if ( ! haveConnection( db, "edited data upload", error ) )
      return IUS_DB2::NOT_CONNECTED;

   const int status = db->writeBlobToDB( filename, "upload_editData",
                                         editedDataID );

   if ( status != IUS_DB2::OK )
   {
      error = QString( "Could not attach %1 to editedData record %2: "
                       "%3 (status %4)" )
              .arg( filename ).arg( editedDataID ).arg( db->lastError() )
              .arg( status );
      return status;
   }

   return IUS_DB2::OK;
}

int US_ConvertIO::downloadEditedDataBlob( IUS_DB2* db, int editedDataID,
                                          const QString& filename,
                                          QString& error )
{
   error.clear();

   if ( ! haveConnection( db, "edited data download", error ) )
      return IUS_DB2::NOT_CONNECTED;

   if ( editedDataID < 1  ||  filename.isEmpty() )
   {
      error = QString( "Cannot download edited data: record ID and destination "
                       "path are required" );
      return EDITED_DATA_INVALID_REQUEST;
   }

   const int status = db->readBlobFromDB( filename, "download_editData",
                                           editedDataID );

   if ( status != IUS_DB2::OK )
   {
      error = QString( "Could not download editedData record %1 to %2: "
                       "%3 (status %4)" )
              .arg( editedDataID ).arg( filename, db->lastError() )
              .arg( status );
      return status;
   }

   return IUS_DB2::OK;
}

int US_ConvertIO::downloadRawDataBlob( IUS_DB2* db, int rawDataID,
                                       const QString& filename,
                                       QString& error )
{
   error.clear();

   if ( ! haveConnection( db, "raw data download", error ) )
      return IUS_DB2::NOT_CONNECTED;

   if ( rawDataID < 1  ||  filename.isEmpty() )
   {
      error = QString( "Cannot download raw data: record ID and destination "
                       "path are required" );
      return EDITED_DATA_INVALID_REQUEST;
   }

   const int status = db->readBlobFromDB( filename, "download_aucData",
                                           rawDataID );

   if ( status != IUS_DB2::OK )
   {
      error = QString( "Could not download rawData record %1 to %2: "
                       "%3 (status %4)" )
              .arg( rawDataID ).arg( filename, db->lastError() )
              .arg( status );
      return status;
   }

   return IUS_DB2::OK;
}

int US_ConvertIO::readEditedDataFromDB(
   IUS_DB2* db, const EditedDataReadRequest& request,
   QVector< US_DataIO::EditedData >& editedData,
   QVector< US_DataIO::RawData >& rawData, QString& error )
{
   editedData.clear();
   rawData.clear();
   error.clear();

   const QString loadFilename = request.loadFilename.isEmpty()
                              ? request.editFilename
                              : request.loadFilename;
   const auto isBasename = []( const QString& filename )
   {
      return ! filename.isEmpty()
             &&  ! filename.contains( "/" )
             &&  ! filename.contains( "\\" )
             &&  QFileInfo( filename ).fileName() == filename;
   };

   if ( request.directory.trimmed().isEmpty()
        ||  ! isBasename( request.editFilename )
        ||  ! isBasename( loadFilename ) )
   {
      error = QString( "Cannot read edited data: a destination directory and "
                       "valid edit basenames are required" );
      return EDITED_DATA_INVALID_REQUEST;
   }

   QString rawFilename = request.rawFilename;

   if ( rawFilename.isEmpty() )
      rawFilename = US_DataIO::rawFilenameForEdit( loadFilename );

   if ( ! isBasename( rawFilename ) )
   {
      error = QString( "Cannot determine a valid raw-data filename from %1" )
              .arg( loadFilename );
      return EDITED_DATA_INVALID_REQUEST;
   }

   if ( request.downloadEdit  &&  request.editedDataID < 1 )
   {
      error = QString( "Cannot download %1 without a valid editedData ID" )
              .arg( request.editFilename );
      return EDITED_DATA_INVALID_REQUEST;
   }

   if ( request.downloadRaw  &&  request.rawDataID < 1 )
   {
      error = QString( "Cannot download %1 without a valid rawData ID" )
              .arg( rawFilename );
      return EDITED_DATA_INVALID_REQUEST;
   }

   QDir directory( request.directory );

   if ( ! directory.exists()  &&  ! QDir().mkpath( request.directory ) )
   {
      error = QString( "Could not create edited-data directory %1" )
              .arg( request.directory );
      return EDITED_DATA_DIRECTORY_ERROR;
   }

   const QString editPath = directory.filePath( request.editFilename );
   const QString rawPath  = directory.filePath( rawFilename );
   int status = IUS_DB2::OK;

   if ( request.downloadEdit )
   {
      status = downloadEditedDataBlob( db, request.editedDataID, editPath,
                                       error );
      if ( status != IUS_DB2::OK )
         return status;
   }
   else if ( ! QFileInfo( editPath ).isFile() )
   {
      error = QString( "Cached edit file does not exist: %1" ).arg( editPath );
      return EDITED_DATA_FILE_MISSING;
   }

   if ( request.downloadRaw )
   {
      status = downloadRawDataBlob( db, request.rawDataID, rawPath, error );
      if ( status != IUS_DB2::OK )
         return status;
   }
   else if ( ! QFileInfo( rawPath ).isFile() )
   {
      error = QString( "Cached raw-data file does not exist: %1" ).arg( rawPath );
      return EDITED_DATA_FILE_MISSING;
   }

   // A successful blob status is not useful if an implementation failed to
   // materialize the destination.  Check both before asking US_DataIO to open
   // them so the error identifies the failed stage.
   if ( ! QFileInfo( editPath ).isFile()  ||  ! QFileInfo( rawPath ).isFile() )
   {
      error = QString( "The edited-data download did not create both %1 and %2" )
              .arg( editPath, rawPath );
      return EDITED_DATA_FILE_MISSING;
   }

   QVector< US_DataIO::EditedData > loadedEdits;
   QVector< US_DataIO::RawData >    loadedRaw;

   try
   {
      status = US_DataIO::loadData( directory.absolutePath(), loadFilename,
                                    loadedEdits, loadedRaw );
   }
   catch ( US_DataIO::ioError dataError )
   {
      error = QString( "Could not decode edited data %1: %2 (status %3)" )
              .arg( loadFilename, US_DataIO::errorString( dataError ) )
              .arg( (int)dataError );
      return EDITED_DATA_DECODE_ERROR;
   }
   catch ( int dataError )
   {
      error = QString( "Could not decode edited data %1: %2 (status %3)" )
              .arg( loadFilename, US_DataIO::errorString( dataError ) )
              .arg( dataError );
      return EDITED_DATA_DECODE_ERROR;
   }

   if ( status != US_DataIO::OK )
   {
      error = QString( "Could not decode edited data %1: %2 (status %3)" )
              .arg( loadFilename, US_DataIO::errorString( status ) )
              .arg( status );
      return EDITED_DATA_DECODE_ERROR;
   }

   if ( loadedEdits.size() != 1  ||  loadedRaw.size() != 1 )
   {
      error = QString( "Loading %1 produced %2 edit datasets and %3 raw "
                       "datasets; exactly one of each was expected" )
              .arg( loadFilename ).arg( loadedEdits.size() )
              .arg( loadedRaw.size() );
      return EDITED_DATA_SHAPE_ERROR;
   }

   editedData = loadedEdits;
   rawData    = loadedRaw;
   return IUS_DB2::OK;
}

QString US_ConvertIO::writeRawDataToDB( US_Experiment& ExpData, 
                                       QList< US_Convert::TripleInfo >& triples,
                                       const QString& dir,
                                       IUS_DB2* db )
{
   const int channelID = 1;
   QString error = QString( "" );
   QString sInvID = QString::number( ExpData.invID );

   // Delete any pcsa_modelrecs records to avoid constraints problems
   US_Experiment::deleteRunPcsaMrecs( db, sInvID, ExpData.runID );

   // Delete all existing solutions and rawData, because we're starting over 
   QStringList q( "delete_rawData" );
   q << QString::number( ExpData.expID );
   int status = db->statusQuery( q );

   if ( status != US_DB2::OK )
   {
      error += "Error returned deleting rawData\n" +
               QString::number( status ) + " " + db->lastError() + "\n";
      return error;
   }

   // Delete links between experiment and solutions
   q.clear();
   q  << "delete_experiment_solutions" 
      << QString::number( ExpData.expID );
   status = db->statusQuery( q );

   if ( status != US_DB2::OK )
   {
      error += "Error returned deleting experiment solutions\n" +
               QString::number( status ) + " " + db->lastError() + "\n";
      return error;
   }

   // Same with cell table
   q.clear();
   q  << "delete_cell_experiments"
      << QString::number( ExpData.expID );
   status = db->statusQuery( q );

   if ( status != US_DB2::OK )
   {
      error += "Error returned deleting cell experiments\n" +
               QString::number( status ) + " " + db->lastError() + "\n";
      return error;
   }

   // We assume there are files, because calling program checked

   // Read all data
   for ( int trx = 0; trx < triples.size(); trx++ )
   {
      US_Convert::TripleInfo* triple = &triples[ trx ];
      if ( triple->excluded ) continue;

      // Convert uuid's to long form
      QString triple_uuidc = US_Util::uuid_unparse( 
                             (unsigned char*) triple->tripleGUID );

      // Verify solutionID
qDebug() << "cvio:WrRDB: trx" << trx << "soluGUID"
 << triple->solution.solutionGUID;
      QStringList q ( "get_solutionID_from_GUID" );
      q  << triple->solution.solutionGUID;
      db->query( q );
      
      status = db->lastErrno();
//qDebug() << "cvio:WrRDB:  solID" << triple->solution.solutionID
// << "status" << status << US_DB2::NOROWS;
      triple->solution.solutionID = 0;
      if ( status == US_DB2::OK )
      {
         db->next();
         triple->solution.solutionID = db->value( 0 ).toInt();
//qDebug() << "cvio:WrRDB:  solID" << triple->solution.solutionID
// << "status" << status << US_DB2::NOROWS;
      }
      
      else if ( status == US_DB2::NOROWS )
      {
         // Solution is not in db, so try to add it
         // figure out channelID later ??
         int diskStatus = US_DB2::NO_SOLUTION;
         if ( !triple->solution.solutionGUID.isEmpty() )
         {
            triple->solution.solutionID = 0;
            diskStatus = triple->solution.saveToDB( ExpData.expID, 
                                                    channelID,
                                                    db );
         }
      
//qDebug() << "cvio:WrRDB:   dkStat" << diskStatus << US_DB2::NO_SOLUTION;
         if ( diskStatus == US_DB2::NO_BUFFER )
            error += "Error processing buffer " + 
                     triple->solution.buffer.GUID + "\n" +
                     "Buffer was not found in the database";
      
         else if ( diskStatus == US_DB2::NO_ANALYTE )
            error += "Error processing analyte \n" 
                     "An analyte was not found in the database";
      
         else if ( diskStatus == US_DB2::NO_SOLUTION )
            error += "Error processing solution " + 
                     triple->solution.solutionGUID + "\n" +
                     "Solution was not found in the database";
      
         else if ( diskStatus != US_DB2::OK )
            error += "Error saving solution to DB \n" +
                     db->lastError();
      
      }
      
      if ( triple->solution.solutionID == 0 )
      {
         // This means that we weren't successful in adding it db
         error += "Error processing solution " + 
                  triple->solution.solutionGUID + "\n" +
                  "Solution was not found in the database";
//qDebug() << "cvio:WrRDB:    NO SOL in DB";
      }

      q.clear();
      q  << "new_rawData"
         << triple_uuidc
         << ExpData.label
         << triple->tripleFilename       // needs to be base name only
         << ExpData.comments
         << QString::number( ExpData.expID )
         << QString::number( triple->solution.solutionID )
         << QString::number( channelID ); // only channel 1 implemented

      status = db->statusQuery( q );
      QString staterr = db->lastError();
      int rawDataID = db->lastInsertID();
//qDebug() << "cvio:WrRDB:  rawDataID" << rawDataID << "status" << status
// << "===" << staterr << "===";

      if ( status == US_DB2::OK )
      {
         // If ok, then let's save the tripleID
         triples[ trx ].tripleID = rawDataID;

         // We can also upload the auc data
         int writeStatus = db->writeBlobToDB( dir + triple->tripleFilename, 
                           QString( "upload_aucData" ), rawDataID );
//qDebug() << "cvio:WrRDB:   wrStat" << writeStatus;

         if ( writeStatus == US_DB2::DBERROR )
         {
            error += "Error processing file:\n" + 
                     dir + triple->tripleFilename + "\n" +
                     db->lastError() + "\n" +
                     "Could not open file or no data \n";
         }
   
         else if ( writeStatus != US_DB2::OK )
         {
            error += "Error returned processing file:\n" + 
                     dir + triple->tripleFilename + "\n" +
                     db->lastError() + "\n";
         }
      }

      else
      {
         error += "Error returned processing file:\n" + 
                  dir + triple->tripleFilename + "\n" +
                  db->lastError() + "\n";
//qDebug() << "cvio:WrRDB:  new_raw ERR" << error;
      }

      int status;
      QStringList ecccs; 
      QStringList esols;
      QString s_expID      = QString::number( ExpData.expID );
      // Get lists of cells, channels, centerpieces, solutions
      q.clear();
      q << "all_cell_experiments"
        << s_expID;
      db->query( q );
      while ( db->next() )
      {
         QString eccc = db->value(2).toString() + ":"
                      + db->value(3).toString() + ":"
                      + db->value(4).toString();
         if ( !ecccs.contains( eccc ) )
            ecccs << eccc;
      }

      q.clear();
      q << "get_solutionIDs"
        << s_expID;
      db->query( q );
      while ( db->next() )
      {
         QString solID = db->value(0).toString();
         if ( !esols.contains( solID ) )
            esols << solID;
      }

      // Write cell table record
      QStringList parts    = triple->tripleDesc.split(" / ");
      const QString& cell         = parts[ 0 ];
      QString letters("SABCDEFGH");
      const QString& channel      = parts[ 1 ];
      int     channelNum   = letters.indexOf( channel );
      QString eccc         = cell + ":" + QString::number( channelNum )
                             + ":" + QString::number( triple->centerpiece );
      if ( !ecccs.contains( eccc ) )
      {  // Add any new cell:channel:centerpiece record for experiment
//qDebug() << "cvio:WrRDB:   chNum" << channelNum;
         QString cellGUID     = US_Util::new_guid();
         q.clear();
         q  << "new_cell_experiment"
            << cellGUID
            << cell
            << QString::number( channelNum )
            << QString::number( triple->centerpiece )
            << s_expID;
         status = db->statusQuery( q );
         if ( status != IUS_DB2::OK )
            error += "Error returned writing cell record: " + cellGUID + "\n" +
                     QString::number(status) + " " + db->lastError() + "\n";
      }

      // Associate solution in this triple with experiment
      QString esolID       = QString::number( triple->solution.solutionID );
      if ( !esols.contains( esolID ) )
      {  // Add any new experimentSolutionChannel record
         q.clear();
         q  << "new_experiment_solution"
            << s_expID
            << esolID
            << QString::number( channelID );
         status = db->statusQuery( q );
qDebug() << "cvio:WrRDB: newExp id solID chnID" << ExpData.expID
 << triple->solution.solutionID << channelID;
         if ( status != US_DB2::OK )
         {
            error += QObject::tr( "MySQL error associating experiment %1\n"
                                  " with solution %2\n"
                                  " code: %3  error: %4\n" )
                     .arg( ExpData.expID ).arg( triple->solution.solutionGUID )
                     .arg( status ).arg( db->lastError() );
         }
      }
   }

   if ( error != QString( "" ) )
   {
      // Most likely the data is not in a desirable state in the db, 
      // for instance the raw data record might have been written but 
      // the blob upload failed. So let's delete the experiment and 
      // rawData. That way, the runID has not become tainted and 
      // we can try again.
      q.clear();
      q  << "delete_experiment"
         << QString::number( ExpData.expID );
      
      int delete_status = db->statusQuery( q );

      if ( delete_status != US_DB2::OK )
      {
         error += "MySQL error deleting experiment "
               + QString::number( ExpData.expID ) 
               + "\n" + db->lastError() + "\n";
      }

//qDebug() << "cvio:WrRDB: OUT *ERR*" << error;
      return error;
   }
//qDebug() << "cvio:WrRDB: OUT";

   return( QString( "" ) );
}

// Function to read the experiment info and binary auc files to disk
QString US_ConvertIO::readDBExperiment( QString runID, QString dir,
					IUS_DB2* db, QVector< SP_SPEEDPROFILE >& speedsteps,
					const QString invid_p )

{
   US_Experiment ExpData;                    // A local copy of experiment
   QList< US_Convert::TripleInfo > triples;  // A local copy of triples
   qDebug() << "rDBE: call ExpData.readFromDB";

   qDebug() << "US_ConvertIO::readDBExperiment, invid_p -- " << invid_p;
   
   int readStatus = ExpData.readFromDB( runID, db, speedsteps, invid_p );
if(speedsteps.size()>0)
 qDebug() << "rDBE:  ss size ss0.sp ss0.avg" << speedsteps.size()
 << speedsteps[0].rotorspeed << speedsteps[0].avg_speed;
else
 qDebug() << "rDBE:  ss size" << speedsteps.size();

   if ( readStatus == US_DB2::NO_EXPERIMENT )
      return( "The current run ID is not found in the database." );

   else if ( readStatus == US_DB2::DBERROR )
      ; // Didn't find any RI Profile data

   else if ( readStatus != US_DB2::OK )
      return( db->lastError() );

   // Erase auc files in the local directory,
   //  in case the user has messed with it locally
qDebug() << "rDBE: remove local auc files";
   QDir d( dir );
   QStringList nameFilters = QStringList( "*.auc" );
   QStringList files = d.entryList( nameFilters,
                                    QDir::NoDotAndDotDot | QDir::Files );
   foreach ( QString file, files )
      if ( ! d.remove( file ) )
         qDebug() << "Unable to remove file " << file;
   
   // Now read the auc data
qDebug() << "rDBE: call readRawDataFromDB";
   QString status = readRawDataFromDB( ExpData, triples, dir, db );
   if ( status != QString( "" ) )
      return status;

   // Verify that the solution and project files are on disk too
qDebug() << "rDBE: call ExpData.project.saveToDisk";
   ExpData.project.saveToDisk();
qDebug() << "rDBE: call solution.saveToDisk (loop)";
   QString psguid = "";

   foreach ( US_Convert::TripleInfo triple, triples )
   {
      QString csguid = triple.solution.solutionGUID;
      if ( csguid == psguid )  continue;

      triple.solution.saveToDisk();
      psguid         = csguid;
   }

   // Verify that RI Profile is on the disk too
qDebug() << "rDBE: call ExpData.saveRIDisk";
   if ( ExpData.opticalSystem == "RI" )
      ExpData.saveRIDisk( runID, dir );

   // Now try to write the xml file
qDebug() << "rDBE: call ExpData.saveToDisk";
#if 0
   QVector< SP_SPEEDPROFILE > sp_dmy;
   int xmlStatus = ExpData.saveToDisk( triples, ExpData.opticalSystem,
                                       runID, dir, sp_dmy );
#endif
   int xmlStatus = ExpData.saveToDisk( triples, ExpData.opticalSystem,
                                       runID, dir, speedsteps );

   if ( xmlStatus == US_Convert::CANTOPEN )
   {
      QString writeFile = runID      + "." 
                        + ExpData.opticalSystem    + ".xml";
      return( "Cannot open write file: " + dir + writeFile );
   }

   else if ( xmlStatus == US_Convert::NOXML )
      ; // Covered in caller error message

   else if ( xmlStatus != US_Convert::OK )
      return( "Unspecified error writing xml file." );

   // Download time state if need be
   bool needTmst  = false;
   int tmstID     = 0;
   int expID      = ExpData.expID;
   QString tfname = runID + ".time_state.tmst";
   QString xdefs;
   QString cksumd;
   QDateTime datedt;
   US_TimeState::dbExamine( db, &tmstID, &expID, &tfname,
                            &xdefs, &cksumd, &datedt );
qDebug() << "rDBE: expID tmstID tfname cksumd datedt"
 << expID << tmstID << tfname << cksumd << datedt;
   if ( tmstID > 0 )
   {  // There is a time state in the database:  look at local disk
      QString tfpath = dir + "/" + tfname;
qDebug() << "rDBE:  HAVE tmst DB: tfpath" << tfpath;
      if ( QFile( tfpath ).exists() )
      {  // File exists, so check if it matches DB
         QString cksumf   = US_Util::md5sum_file( tfpath );
qDebug() << "rDBE:  HAVE tmst Loc: cksumf" << cksumf;
         if ( cksumf != cksumd )
         {  // DB/Local do not match, so overwrite local with download
            needTmst       = true;
         }
      }
      else
      {  // No local tmst exists, do need to download it
         needTmst       = true;
      }

qDebug() << "rDBE:  needTmst" << needTmst;
      if ( needTmst )
      {  // Download the .tmst file
         US_TimeState::dbDownload( db, tmstID, tfpath );
         // And write the xdefs sibling file
         QString xfpath = QString( tfpath ).replace( ".tmst", ".xml" );
qDebug() << "rDBE:   xfpath" << xfpath;
         QFile fileo( xfpath );
         if ( fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
         {
            QTextStream tso( &fileo );
            tso << xdefs;
            tso.flush();
            fileo.close();
qDebug() << "rDBE:    xdefs WRITTEN";
         }
      }
   }
   return( QString( "" ) );
}

// Function to read the auc files to disk
QString US_ConvertIO::readRawDataFromDB( US_Experiment& ExpData, 
                                       QList< US_Convert::TripleInfo >& triples,
                                       QString& dir,
                                       IUS_DB2* db )
{
   // Get the rawDataID's that correspond to this experiment
qDebug() << " rRDD: build raw list";
   QStringList q( "get_rawDataIDs" );
   q  << QString::number( ExpData.expID );
   db->query( q );

   QStringList rawDataIDs;
   QStringList filenames;

   while ( db->next() )
   {
      rawDataIDs << db->value( 0 ).toString();
      filenames  << db->value( 2 ).toString();
   }

   if ( rawDataIDs.size() < 1 )
      return( "There were no auc files found in the database." );

   // Set working directory and create it if necessary
   dir  = US_Settings::resultDir() + "/" + ExpData.runID;

   QDir work( US_Settings::resultDir() );
   work.mkdir( ExpData.runID );

   // Read the auc files to disk
qDebug() << " rRDD: read BlobFromDB (loop)";
   QString error = QString( "" );
   for ( int i = 0; i < rawDataIDs.size(); i++ )
   {
      QString f = dir + "/" + filenames[ i ];
      int readStatus = db->readBlobFromDB( f, QString( "download_aucData" ),
                                           rawDataIDs[ i ].toInt() );

      if ( readStatus == US_DB2::DBERROR )
      {
         error += "Error processing file: " + f + "\n" +
                  "Could not open file or no data \n";
      }

      else if ( readStatus != US_DB2::OK )
      {
         error += "Error returned processing file: " + f + "\n" +
                  db->lastError() + "\n";
      }
   }

   // If we can't even read the files we should just stop here
   if ( error != QString( "" ) )
      return( error );

   // Now get the centerpiece info
   QList<cellInfo> cells; 
   q.clear();
   q  << "all_cell_experiments"
      << QString::number( ExpData.expID );
   db->query( q );
   while ( db->next() )
   {
      struct cellInfo cell;
      QString letters("SABCDEFGH");
      cell.cellName      = db->value( 2 ).toString();
      cell.channelName   = QString( letters[ qMax( 0, db->value( 3 ).toInt() ) ] );
      cell.centerpieceID = db->value( 4 ).toInt();
      cells << cell;
   }

   int psolID = -1;

   // Get the other db info and create triples
qDebug() << " rRDD: build TripleInfo";
   triples.clear();
   for ( int i = 0; i < rawDataIDs.size(); i++ )
   {
      US_Convert::TripleInfo triple;

      q.clear();
      q  << "get_rawData"
         << rawDataIDs[ i ];
      db->query( q );

      if ( db->next() )
      {
         QString uuidc         = db->value( 0 ).toString();
         US_Util::uuid_parse( uuidc, (unsigned char*) triple.tripleGUID );
         // triple.label               = db->value( 1 ).toString();
         triple.tripleFilename      = db->value( 2 ).toString();
         // triple.tripleComments      = db->value( 3 ).toString();
         triple.tripleID            = rawDataIDs[ i ].toInt();
         triple.solution.solutionID = db->value( 5 ).toInt();

         QStringList part      = triple.tripleFilename.split( "." );
         QString wl;
         QString runType       = part[ 1 ];
         if ( runType == "WA" )
            wl = QString::number( part[ 4 ].toDouble() / 1000.0 );
         else
            wl = part[ 4 ];
         triple.tripleDesc     = part[ 2 ] + " / " + part[ 3 ] + " / " + wl;
         triple.excluded       = false;

         // Now try to find the centerpiece ID from the info we grabbed earlier
         bool found = false;
         foreach ( struct cellInfo cell, cells )
         {
            if ( part[ 2 ] == cell.cellName &&
                 part[ 3 ] == cell.channelName )
            {
               found = true;
               triple.centerpiece = cell.centerpieceID;
               break;
            }

         }

         if ( ! found )
         {
            // if we're here, then it's old data and needs to find the centerpiece
            // in the old manner, which is to say, always the first one
            triple.centerpiece = cells[ 0 ].centerpieceID;
         }

         int csolID = triple.solution.solutionID;

         // Try to get more solution info
         int status = US_DB2::OK;

         if ( csolID != psolID )
         {
            status = triple.solution.readFromDB( csolID, db );
            psolID = csolID;
         }

         if ( status == US_DB2::NO_BUFFER )
         {
            // buffer wasn't found
            triple.solution.buffer.bufferID    = "-1";
            triple.solution.buffer.GUID        = QString( "" );
            triple.solution.buffer.description = QString( "" );
         }

         else if ( status == US_DB2::NO_ANALYTE )
            triple.solution.analyteInfo.clear();

         // save it   
         triples << triple;
      }

      else
         error += "Error processing file: " + filenames[ i ] + "\n" +
                  db->lastError() + "\n";
   }
      
   // Get runType
   QStringList part = triples[ 0 ].tripleFilename.split( "." );
   ExpData.opticalSystem = part[ 1 ].toUtf8();

   if ( error != QString( "" ) )
      return( error );

   return( QString( "" ) );
}

int US_ConvertIO::checkDiskData( US_Experiment& ExpData,
                                 QList< US_Convert::TripleInfo >& triples,
                                 IUS_DB2* db )
{
   if ( US_Settings::us_inv_ID() == -1 )
      return US_DB2::NO_PERSON;

   ExpData.invID = US_Settings::us_inv_ID();

   // Get investigator GUID
   QStringList q( "get_person_info" );
   q << QString::number( ExpData.invID );
   db->query( q );

   if ( db->lastErrno() != US_DB2::OK )
   {
      ExpData.invID   = 0;
      ExpData.invGUID = QString( "" );
      ExpData.name    = QString( "" );
      return US_DB2::NO_PERSON;
   }

   // Save updated investigator GUID
   if ( db->next() )
   {
      ExpData.invGUID = db->value( 9 ).toString();
      ExpData.name    = db->value( 1 ).toString() + ", " + db->value( 0 ).toString();
   }

   // operator GUID
   if ( ! US_Util::UUID_REGEX.match( ExpData.operatorGUID ).hasMatch() )
      return US_DB2::BADGUID;

   // triple GUID's
   for (const auto & triple : triples)
   {
      if ( triple.excluded ) continue;

      QString uuidc = US_Util::uuid_unparse(
         (unsigned char*) triple.tripleGUID );
      if ( ! US_Util::UUID_REGEX.match( uuidc ).hasMatch() )
         return US_DB2::BADGUID;
   }

   // rotor GUID
   if ( ! US_Util::UUID_REGEX.match( ExpData.rotorGUID ).hasMatch() )
      return US_DB2::BADGUID;

   // Ok, GUID's are ok

   // Check if operator exists
   q.clear();
   q << QString( "get_personID_from_GUID" )
     << QString( ExpData.operatorGUID );
   db->query( q );

   if ( db->lastErrno() != US_DB2::OK )
   {
      ExpData.operatorID   = 0;
      ExpData.operatorGUID = QString( "" );
      return US_DB2::NO_PERSON;
   }

   // Save updated investigator ID
   if ( db->next() )
      ExpData.operatorID = db->value( 0 ).toInt();

   // Check rotor
   q.clear();
   q << QString( "get_rotorID_from_GUID" )
     << QString( ExpData.rotorGUID );
   db->query( q );

   if ( db->lastErrno() != US_DB2::OK )
   {
      ExpData.rotorID       = 0;
      ExpData.calibrationID = 0;
      ExpData.rotorCoeff1   = 0.0;
      ExpData.rotorCoeff2   = 0.0;
      ExpData.rotorGUID     = QString( "" );
      ExpData.rotorSerial   = QString( "" );
      ExpData.rotorName     = QString( "" );
      return US_DB2::NO_ROTOR;
   }

   // Save updated rotor info 
   if ( db->next() )
      ExpData.rotorID = db->value( 0 ).toInt();
   q.clear();
   q << QString( "get_rotor_info" )
     << QString::number( ExpData.rotorID );
   db->query( q );
   if ( db->next() )
   {
      ExpData.rotorGUID   = db->value( 0 ).toString();
      ExpData.rotorName   = db->value( 1 ).toString();
      ExpData.rotorSerial = db->value( 2 ).toString();
   }

   // Save calibration info
   if ( ExpData.calibrationID == 0 )     // In this case, get the first one
   {
      q.clear();
      q << QString( "get_rotor_calibration_profiles" )
        << QString::number( ExpData.rotorID );
      db->query( q );
      if ( db->next() )
         ExpData.calibrationID = db->value( 0 ).toInt();
   }

   // Now get more calibration info
   ExpData.rotorCoeff1  = 0.0;
   ExpData.rotorCoeff2  = 0.0;
   ExpData.rotorUpdated = QDate::currentDate();
   q.clear();
   q << QString( "get_rotor_calibration_info" )
     << QString::number( ExpData.calibrationID );
   db->query( q );
   if ( db->next() )
   {
      ExpData.rotorCoeff1  = db->value( 4 ).toString().toFloat();
      ExpData.rotorCoeff2  = db->value( 5 ).toString().toFloat();
      QStringList dateParts = db->value( 7 ).toString().split( " " );
      ExpData.rotorUpdated = QDate::fromString( dateParts[ 0 ], "yyyy-MM-dd" );
   }

   // Initialize triple GUID's
   int found = true;     // Let's assume we'll find them all
   for ( int i = 0; i < triples.size(); i++ )
   {
      if ( triples[ i ].excluded ) continue;

      QString uuidc = US_Util::uuid_unparse(
            (unsigned char*) triples[ i ].tripleGUID );
      q.clear();
      q << QString( "get_rawDataID_from_GUID" )
        << uuidc;
      db->query( q );

      int status = db->lastErrno();
qDebug() << "iSv: trip" << i << "tGUID" << uuidc
 << "stat,ok,norow" << status << US_DB2::OK << US_DB2::NOROWS;
      if ( status == US_DB2::OK )
      {
         // Save updated triple ID
         if ( db->next() )
            triples[ i ].tripleID = db->value( 0 ).toString().toInt();
qDebug() << "iSv:    tID" << triples[i].tripleID;
      }

      else if ( status == US_DB2::NOROWS )
      {
         triples[ i ].tripleID = 0;
         memset( triples[ i ].tripleGUID, 0, 16 );
         found = false;     // At least one wasn't found in the DB
      }

      else     // more unlikely errors
      {
         qDebug() << "get_rawDataID_from_GUID error: " << db->lastErrno();
         qDebug() << "triple GUID = " << uuidc;
         return status;
      }

   }

   if ( ! found )       // Probably we just haven't saved it yet
      return US_DB2::NO_RAWDATA;

   return US_DB2::OK ;
}

//Copy for use in Autoflow
int US_ConvertIO::checkDiskData_auto( US_Experiment& ExpData,
                                 QList< US_Convert::TripleInfo >& triples,
                                 IUS_DB2* db )
{
   if ( US_Settings::us_inv_ID() == -1 )
      return US_DB2::NO_PERSON;

   //ExpData.invID = US_Settings::us_inv_ID();

   // Get investigator GUID
   QStringList q( "get_person_info" );
   q << QString::number( ExpData.invID );
   db->query( q );

   if ( db->lastErrno() != US_DB2::OK )
   {
      ExpData.invID   = 0;
      ExpData.invGUID = QString( "" );
      ExpData.name    = QString( "" );
      return US_DB2::NO_PERSON;
   }

   // Save updated investigator GUID
   if ( db->next() )
   {
      ExpData.invGUID = db->value( 9 ).toString();
      ExpData.name    = db->value( 1 ).toString() + ", " + db->value( 0 ).toString();
   }

   // operator GUID
   if ( ! US_Util::UUID_REGEX.match( ExpData.operatorGUID ).hasMatch() )
      return US_DB2::BADGUID;

   // triple GUID's
   for (const auto & triple : triples)
   {
      if ( triple.excluded ) continue;

      QString uuidc = US_Util::uuid_unparse(
         (unsigned char*) triple.tripleGUID );
      if ( ! US_Util::UUID_REGEX.match( uuidc ).hasMatch() )
         return US_DB2::BADGUID;
   }

   // rotor GUID
   if ( ! US_Util::UUID_REGEX.match( ExpData.rotorGUID ).hasMatch() )
      return US_DB2::BADGUID;

   // Ok, GUID's are ok

   // Check if operator exists
   q.clear();
   q << QString( "get_personID_from_GUID" )
     << QString( ExpData.operatorGUID );
   db->query( q );

   if ( db->lastErrno() != US_DB2::OK )
   {
      ExpData.operatorID   = 0;
      ExpData.operatorGUID = QString( "" );
      return US_DB2::NO_PERSON;
   }

   // Save updated investigator ID
   if ( db->next() )
      ExpData.operatorID = db->value( 0 ).toInt();

   // Check rotor
   q.clear();
   q << QString( "get_rotorID_from_GUID" )
     << QString( ExpData.rotorGUID );
   db->query( q );

   if ( db->lastErrno() != US_DB2::OK )
   {
      ExpData.rotorID       = 0;
      ExpData.calibrationID = 0;
      ExpData.rotorCoeff1   = 0.0;
      ExpData.rotorCoeff2   = 0.0;
      ExpData.rotorGUID     = QString( "" );
      ExpData.rotorSerial   = QString( "" );
      ExpData.rotorName     = QString( "" );
      return US_DB2::NO_ROTOR;
   }

   // Save updated rotor info 
   if ( db->next() )
      ExpData.rotorID = db->value( 0 ).toInt();
   q.clear();
   q << QString( "get_rotor_info" )
     << QString::number( ExpData.rotorID );
   db->query( q );
   if ( db->next() )
   {
      ExpData.rotorGUID   = db->value( 0 ).toString();
      ExpData.rotorName   = db->value( 1 ).toString();
      ExpData.rotorSerial = db->value( 2 ).toString();
   }

   // Save calibration info
   if ( ExpData.calibrationID == 0 )     // In this case, get the first one
   {
      q.clear();
      q << QString( "get_rotor_calibration_profiles" )
        << QString::number( ExpData.rotorID );
      db->query( q );
      if ( db->next() )
         ExpData.calibrationID = db->value( 0 ).toInt();
   }

   // Now get more calibration info
   ExpData.rotorCoeff1  = 0.0;
   ExpData.rotorCoeff2  = 0.0;
   ExpData.rotorUpdated = QDate::currentDate();
   q.clear();
   q << QString( "get_rotor_calibration_info" )
     << QString::number( ExpData.calibrationID );
   db->query( q );
   if ( db->next() )
   {
      ExpData.rotorCoeff1  = db->value( 4 ).toString().toFloat();
      ExpData.rotorCoeff2  = db->value( 5 ).toString().toFloat();
      QStringList dateParts = db->value( 7 ).toString().split( " " );
      ExpData.rotorUpdated = QDate::fromString( dateParts[ 0 ], "yyyy-MM-dd" );
   }

   // Initialize triple GUID's
   int found = true;     // Let's assume we'll find them all
   for ( int i = 0; i < triples.size(); i++ )
   {
      if ( triples[ i ].excluded ) continue;

      QString uuidc = US_Util::uuid_unparse(
            (unsigned char*) triples[ i ].tripleGUID );
      q.clear();
      q << QString( "get_rawDataID_from_GUID" )
        << uuidc;
      db->query( q );

      int status = db->lastErrno();
qDebug() << "iSv: trip" << i << "tGUID" << uuidc
 << "stat,ok,norow" << status << US_DB2::OK << US_DB2::NOROWS;
      if ( status == US_DB2::OK )
      {
         // Save updated triple ID
         if ( db->next() )
            triples[ i ].tripleID = db->value( 0 ).toString().toInt();
qDebug() << "iSv:    tID" << triples[i].tripleID;
      }

      else if ( status == US_DB2::NOROWS )
      {
         triples[ i ].tripleID = 0;
         memset( triples[ i ].tripleGUID, 0, 16 );
         found = false;     // At least one wasn't found in the DB
      }

      else     // more unlikely errors
      {
         qDebug() << "get_rawDataID_from_GUID error: " << db->lastErrno();
         qDebug() << "triple GUID = " << uuidc;
         return status;
      }

   }

   if ( ! found )       // Probably we just haven't saved it yet
      return US_DB2::NO_RAWDATA;

   return US_DB2::OK ;
}
