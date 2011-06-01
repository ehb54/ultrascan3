//! \file us_convertio.cpp

#include <QDomDocument>

#include "us_settings.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_convert.h"
#include "us_convertio.h"
#include "us_util.h"

// Generic constructor
US_ConvertIO::US_ConvertIO( void )
{
}

QString US_ConvertIO::writeRawDataToDB( US_Experiment& ExpData, 
                                        QList< US_Convert::TripleInfo >& triples,
                                        QString dir,
                                        US_DB2* db )
{
   QString error = QString( "" );

   // Delete all existing solutions and rawData, because we're starting over 
   QStringList q( "delete_rawData" );
   q << QString::number( ExpData.expID );
   int status = db->statusQuery( q );

   if ( status != US_DB2::OK )
      return( db->lastError() );

   // Delete links between experiment and solutions
   q.clear();
   q  << "delete_experiment_solutions" 
      << QString::number( ExpData.expID );
   status = db->statusQuery( q );

   if ( status != US_DB2::OK )
      return( db->lastError() );

   // Same with cell table
   q.clear();
   q  << "delete_cell_experiments"
      << QString::number( ExpData.expID );
   status = db->statusQuery( q );

   if ( status != US_DB2::OK )
      return( db->lastError() );

   // We assume there are files, because calling program checked

   // Read all data
   for ( int i = 0; i < triples.size(); i++ )
   {
      US_Convert::TripleInfo triple = triples[ i ];
      if ( triple.excluded ) continue;

      // Convert uuid's to long form
      QString triple_uuidc = US_Util::uuid_unparse( (unsigned char*) triple.tripleGUID );

      // Verify solutionID
      q.clear();
      q  << "get_solutionID_from_GUID"
         << triple.solution.solutionGUID;
      db->query( q );

      status = db->lastErrno();
      triple.solution.solutionID = 0;
      if ( status == US_DB2::OK )
      {
         db->next();
         triple.solution.solutionID = db->value( 0 ).toInt();
      }

      else if ( status == US_DB2::NOROWS )
      {
         // Solution is not in db, so try to add it
         int diskStatus = triple.solution.saveToDB( ExpData.expID, 
                                                    1,               // figure out channelID later ??
                                                    db );
//qDebug() << "triple.solution.saveToDB db status = " << QString::number( diskStatus );
         if ( diskStatus == US_DB2::NO_BUFFER )
            error += "Error processing buffer " + triple.solution.buffer.GUID + '\n' +
                     "Buffer was not found in the database";

         else if ( diskStatus == US_DB2::NO_ANALYTE )
            error += "Error processing analyte \n" 
                     "An analyte was not found in the database";

         else if ( diskStatus == US_DB2::NO_SOLUTION )
            error += "Error processing solution " + triple.solution.solutionGUID + '\n' +
                     "Solution was not found in the database";

         else if ( diskStatus != US_DB2::OK )
            error += "Error saving solution to DB " + '\n' +
                     db->lastError();

      }

      if ( triple.solution.solutionID == 0 )
      {
         // This means that we weren't successful in adding the solution to db
         error += "Error processing solution " + triple.solution.solutionGUID + '\n' +
                  "Solution was not found in the database";
      }

      QStringList q( "new_rawData" );
      q  << triple_uuidc
         << ExpData.label
         << triple.tripleFilename       // needs to be base name only
         << ExpData.comments
         << QString::number( ExpData.expID )
         << QString::number( triple.solution.solutionID )
         << QString::number( triple.channelID );    // only channel 1 implemented

      status = db->statusQuery( q );
      int rawDataID = db->lastInsertID();

      if ( status == US_DB2::OK )
      {
         // If ok, then let's save the tripleID
         triples[ i ].tripleID = rawDataID;

         // We can also upload the auc data
         int writeStatus = db->writeBlobToDB( dir + triple.tripleFilename, 
                           QString( "upload_aucData" ), rawDataID );

         if ( writeStatus == US_DB2::ERROR )
         {
            error += "Error processing file:\n" + 
                     dir + triple.tripleFilename + "\n" +
                     db->lastError() + "\n" +
                     "Could not open file or no data \n";
         }
   
         else if ( writeStatus != US_DB2::OK )
         {
            error += "Error returned processing file:\n" + 
                     dir + triple.tripleFilename + "\n" +
                     db->lastError() + "\n";
         }
      }

      else
      {
         error += "Error returned processing file:\n" + 
                  dir + triple.tripleFilename + "\n" +
                  db->lastError() + "\n";
      }

      // Write cell table record
      QString cellGUID     = US_Util::new_guid();
      QStringList parts    = triple.tripleDesc.split(" / ");
      QString cell         = parts[ 0 ];
      q.clear();
      q  << "new_cell_experiment"
         << cellGUID
         << cell
         << QString::number( triple.centerpiece )
         << QString::number( ExpData.expID );
      int status = db->statusQuery( q );
      if ( status != US_DB2::OK )
         error += "Error returned writing cell record: " + cellGUID + "\n" +
                  status + " " + db->lastError() + "\n";

      // Associate solution in this triple with experiment
      q.clear();
      q  << "new_experiment_solution"
         << QString::number( ExpData.expID )
         << QString::number( triple.solution.solutionID )
         << QString::number( triple.channelID );
      status = db->statusQuery( q );
      if ( status != US_DB2::OK )
      {
         error += "MySQL error associating experiment "   + 
                  QString::number( ExpData.expID ) + "\n" +
                  " with solution " + triple.solution.solutionGUID + "\n" +
                  status + " " + db->lastError() + "\n";
      }
   }

   if ( error != QString( "" ) )
      return( error );

   return( QString( "" ) );
}

// Function to read the experiment info and binary auc files to disk
QString US_ConvertIO::readDBExperiment( QString runID,
                                        QString dir,
                                        US_DB2* db )
{
   US_Experiment ExpData;       // A local copy
   QList< US_Convert::TripleInfo > triples;  // a local copy
   int readStatus = ExpData.readFromDB( runID, db );

   if ( readStatus == US_DB2::NO_EXPERIMENT )
      return( "The current run ID is not found in the database." );

   else if ( readStatus != US_DB2::OK )
      return( db->lastError() );

   // Erase auc files in the local directory, in case the user has messed with it locally
   QDir d( dir );
   QStringList nameFilters = QStringList( "*.auc" );
   QStringList files = d.entryList( nameFilters, QDir::NoDotAndDotDot | QDir::Files );
   foreach ( QString file, files )
      if ( ! d.remove( file ) )
         qDebug() << "Unable to remove file " << file;
   
   // Now read the auc data
   QString status = readRawDataFromDB( ExpData, triples, dir, db );
   if ( status != QString( "" ) )
      return status;

   // Verify that the solution and project files are on disk too
   ExpData.project.saveToDisk();
   foreach ( US_Convert::TripleInfo triple, triples )
   {
      triple.solution.saveToDisk();
   }

   // Now try to write the xml file
   int xmlStatus = ExpData.saveToDisk( triples, ExpData.opticalSystem, runID, dir );

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

   return( QString( "" ) );
}

// Function to read the auc files to disk
QString US_ConvertIO::readRawDataFromDB( US_Experiment& ExpData, 
                                         QList< US_Convert::TripleInfo >& triples,
                                         QString& dir,
                                         US_DB2* db )
{
   // Get the rawDataID's that correspond to this experiment
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
      return( "There were no auc files found in the databae." );

   // Set working directory and create it if necessary
   dir  = US_Settings::resultDir() + "/" + ExpData.runID;

   QDir work( US_Settings::resultDir() );
   work.mkdir( ExpData.runID );

   // Read the auc files to disk
   for ( int i = 0; i < rawDataIDs.size(); i++ )
   {
      QString f = dir + "/" + filenames[ i ];
      db->readBlobFromDB( f, QString( "download_aucData" ), rawDataIDs[ i ].toInt() );
   }

   // Now get the centerpiece info
   int commonCenterpiece = 0;
   q.clear();
   q  << "all_cell_experiments"
      << QString::number( ExpData.expID );
   db->query( q );
   if ( db->next() )
      commonCenterpiece = db->value( 3 ).toInt();

   // Get the other db info and create triples
   QString error = QString( "" );
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
         //triple.label             = db->value( 1 ).toString();
         triple.tripleFilename      = db->value( 2 ).toString();
         //triple.tripleComments    = db->value( 3 ).toString();
         triple.tripleID            = rawDataIDs[ i ].toInt();
         triple.solution.solutionID = db->value( 5 ).toInt();

         QStringList part      = triple.tripleFilename.split( "." );
         triple.tripleDesc     = part[ 2 ] + " / " + part[ 3 ] + " / " + part[ 4 ];
         triple.excluded       = false;

         triple.centerpiece    = commonCenterpiece;

         // Try to get more solution info
         int status = triple.solution.readFromDB( triple.solution.solutionID, db );

         if ( status == US_DB2::NO_BUFFER )
         {
            // buffer wasn't found
            triple.solution.buffer.bufferID    = -1;
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
   ExpData.opticalSystem = part[ 1 ].toAscii();

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

   // Check all the other GUID's for format
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );

   // operator GUID
   if ( ! rx.exactMatch( ExpData.operatorGUID ) )
      return US_DB2::BADGUID;

   // triple GUID's
   for ( int i = 0; i < triples.size(); i++ )
   {
      if ( triples[ i ].excluded ) continue;

      QString uuidc = US_Util::uuid_unparse( (unsigned char*) triples[ i ].tripleGUID );
      if ( ! rx.exactMatch( uuidc ) )
         return US_DB2::BADGUID;
   }

   // rotor GUID
   if ( ! rx.exactMatch( ExpData.rotorGUID ) )
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

      QString uuidc = US_Util::uuid_unparse( (unsigned char*) triples[ i ].tripleGUID );
      q.clear();
      q << QString( "get_rawDataID_from_GUID" )
        << uuidc;
      db->query( q );

      int status = db->lastErrno();
      if ( status == US_DB2::OK )
      {
         // Save updated triple ID
         if ( db->next() )
            triples[ i ].tripleID = db->value( 0 ).toString().toInt();
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
