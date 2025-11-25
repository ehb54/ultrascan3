//! \file us_convertio.cpp

#include <QDomDocument>

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

QString US_ConvertIO::writeRawDataToDB( US_Experiment& ExpData, 
                                       QList< US_Convert::TripleInfo >& triples,
                                       const QString& dir,
                                       US_DB2* db )
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
          US_DB2* db, QVector< SP_SPEEDPROFILE >& speedsteps )

{
   US_Experiment ExpData;                    // A local copy of experiment
   QList< US_Convert::TripleInfo > triples;  // A local copy of triples
qDebug() << "rDBE: call ExpData.readFromDB";
   int readStatus = ExpData.readFromDB( runID, db, speedsteps );
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
                                       US_DB2* db )
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
                                 US_DB2* db )
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
                                 US_DB2* db )
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
