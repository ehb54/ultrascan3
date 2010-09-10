//! \file us_convertio.cpp

#include <QDomDocument>

#include <uuid/uuid.h>

#include "us_settings.h"
#include "us_math2.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_convert.h"
#include "us_convertio.h"

// Generic constructor
US_ConvertIO::US_ConvertIO( void )
{
}

// Function to see if the current runID already exists in the database
int US_ConvertIO::checkRunID( QString runID )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );
   
   if ( db.lastErrno() != US_DB2::OK )
      return -1;

   // Let's see if we can find the run ID
   QStringList q( "get_experiment_info_by_runID" );
   q << runID;
   db.query( q );

   if ( db.lastErrno() == US_DB2::NOROWS )
      return 0;
   
   // Ok, let's return the experiment ID
   return ( db.value( 1 ).toInt() );
}

QString US_ConvertIO::newDBExperiment( US_ExpInfo::ExperimentInfo& ExpData, 
                                       QList< US_SolutionGui::TripleInfo >& triples,
                                       QString dir )
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   // Connect to the database
   if ( db.lastErrno() != US_DB2::OK )
      return( db.lastError() );

   QStringList q( "new_experiment" );
   q  << ExpData.expGUID
      << QString::number( ExpData.projectID )
      << ExpData.runID
      << QString::number( ExpData.labID )
      << QString::number( ExpData.instrumentID )
      << QString::number( ExpData.operatorID )
      << QString::number( ExpData.rotorID )
      << ExpData.expType
      << ExpData.runTemp
      << ExpData.label
      << ExpData.comments
      << ExpData.centrifugeProtocol;

   int status = db.statusQuery( q );
   if ( status != US_DB2::OK )
      return( db.lastError() );

   // Let's get some info after db update
   q.clear();
   ExpData.expID = db.lastInsertID();
   q << "get_experiment_info"
     << QString::number( ExpData.expID );
   db.query( q );
   db.next();

   ExpData.date = db.value( 12 ).toString();

   // Now write the auc data
   return( writeRawDataToDB( ExpData, triples, dir ) );
}

QString US_ConvertIO::updateDBExperiment( US_ExpInfo::ExperimentInfo& ExpData, 
                                          QList< US_SolutionGui::TripleInfo >& triples,
                                          QString dir )
{
   // Update database
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
      return( db.lastError() );

   QStringList q( "update_experiment" );
   q  << QString::number( ExpData.expID )
      << QString::number( ExpData.projectID )
      << ExpData.runID
      << QString::number( ExpData.labID )
      << QString::number( ExpData.instrumentID )
      << QString::number( ExpData.operatorID )
      << QString::number( ExpData.rotorID )
      << ExpData.expType
      << ExpData.runTemp
      << ExpData.label
      << ExpData.comments
      << ExpData.centrifugeProtocol;

   int status = db.statusQuery( q );
   if ( status != US_DB2::OK )
      return( db.lastError() );

   // Let's get some info after db update
   q.clear();
   q << "get_experiment_info"
     << QString::number( ExpData.expID );
   db.query( q );
   db.next();

   ExpData.date = db.value( 12 ).toString();

   // Now write the auc data
   return( writeRawDataToDB( ExpData, triples, dir ) );
}

QString US_ConvertIO::writeRawDataToDB( US_ExpInfo::ExperimentInfo& ExpData, 
                                        QList< US_SolutionGui::TripleInfo >& triples,
                                        QString dir )
{
   // Update database
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
      return( db.lastError() );

   // Delete all existing solutions and rawData, because we're starting over 
   QStringList q( "delete_rawData" );
   q << QString::number( ExpData.expID );
   int status = db.statusQuery( q );

   if ( status != US_DB2::OK )
      return( db.lastError() );

   q.clear();
   q  << "delete_solution" 
      << QString::number( ExpData.expID );
   status = db.statusQuery( q );

   if ( status != US_DB2::OK )
      return( db.lastError() );

   // We assume there are files, because calling program checked

   // Read all data
   QString error = NULL;
   for ( int i = 0; i < triples.size(); i++ )
   {
      US_SolutionGui::TripleInfo triple = triples[ i ];
      if ( triple.excluded ) continue;

      // Convert uuid's to long form
      char triple_uuidc[ 37 ];
      char solution_uuidc[ 37 ];
      uuid_unparse( (unsigned char*) triple.tripleGUID, triple_uuidc );
      uuid_unparse( (unsigned char*) triple.solutionGUID, solution_uuidc );

      // Create solution db entries
      q.clear();
      q  << "new_solution" 
         << QString( solution_uuidc )
         << triple.description
         << QString::number( triple.storageTemp )
         << triple.notes
         << QString::number( ExpData.expID )
         << "1" ;           // channel ID

      status = db.statusQuery( q );
      int solutionID = db.lastInsertID();

      if ( status == US_DB2::OK )
      {
         q.clear();
         q  << "new_solutionBuffer"
            << QString::number( solutionID )
            << ""           // skip bufferID and use GUID instead 
            << triple.bufferGUID;

         status = db.statusQuery( q );
         if ( status != US_DB2::OK )
            error += "Error associating buffer with solution in database: " +  db.lastError() + "\n";

         q.clear();
         q  << "new_solutionAnalyte"
            << QString::number( solutionID )
            << ""           // skip analyteID and use GUID instead
            << triple.analyteGUID
            << "1";         // a dummy amount for now

         status = db.statusQuery( q );
         if ( status != US_DB2::OK )
            error += "Error associating analyte with solution in database: " + db.lastError() + "\n";
      }

      else
         error += "Error writing solution information: " + db.lastError() + "\n";

      QStringList q( "new_rawData" );
      q  << QString( triple_uuidc )
         << ExpData.label
         << triple.tripleFilename       // needs to be base name only
         << ExpData.comments
         << QString::number( ExpData.expID )
         << QString::number( solutionID )
         << "1" ;           // channel ID

      status = db.statusQuery( q );
      int rawDataID = db.lastInsertID();

      if ( status == US_DB2::OK )
      {
         // If ok, then let's save the tripleID
         triples[ i ].tripleID = rawDataID;

         // We can also upload the auc data
         int writeStatus = db.writeBlobToDB( dir + triple.tripleFilename, 
                           QString( "upload_aucData" ), rawDataID );

         if ( writeStatus == US_DB2::ERROR )
         {
            error += "Error processing file: " + triple.tripleFilename + "\n" +
                     "Could not open file or no data \n";
         }
   
         else if ( writeStatus != US_DB2::OK )
         {
            error += "Error returned processing file: " + triple.tripleFilename + "\n" +
                     db.lastError() + "\n";
         }
      }

      else
      {
         error += "Error returned processing file: " + triple.tripleFilename + "\n" +
                  db.lastError() + "\n";
      }
   }

   if ( error != NULL )
      return( error );

   return( NULL );
}

// Function to read the experiment info and binary auc files to disk
QString US_ConvertIO::readDBExperiment( QString runID,
                                        QString dir )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
      return( db.lastError() );

   US_ExpInfo::ExperimentInfo ExpData;       // A local copy
   QList< US_SolutionGui::TripleInfo > triples;  // a local copy
   QStringList q( "get_experiment_info_by_runID" );
   q << runID;
   db.query( q );
 
   if ( db.next() )
   {
      ExpData.runID              = runID;
      ExpData.projectID          = db.value( 0 ).toInt();
      ExpData.expID              = db.value( 1 ).toInt();
      ExpData.expGUID            = db.value( 2 ).toString();
      ExpData.labID              = db.value( 3 ).toInt();
      ExpData.instrumentID       = db.value( 4 ).toInt();
      ExpData.operatorID         = db.value( 5 ).toInt();
      ExpData.rotorID            = db.value( 6 ).toInt();
      ExpData.expType            = db.value( 7 ).toString();
      ExpData.runTemp            = db.value( 8 ).toString();
      ExpData.label              = db.value( 9 ).toString();
      ExpData.comments           = db.value( 10 ).toString();
      ExpData.centrifugeProtocol = db.value( 11 ).toString();
      ExpData.date               = db.value( 12 ).toString();
      ExpData.invID              = db.value( 13 ).toInt();
   }

   else if ( db.lastErrno() == US_DB2::NOROWS )
      return( "The current run ID is not found in the database." );

   else
      return( db.lastError() );

   QString status = readExperimentInfoDB( ExpData );
   if ( status != NULL )
      return status;

   // Now read the auc data
   status = readRawDataFromDB( ExpData, triples, dir );
   if ( status != NULL )
      return status;

   // Now try to write the xml file
   int xmlStatus = writeXmlFile( 
            ExpData, triples, ExpData.opticalSystem, runID, dir );

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

   return( NULL );
}

// Function to read the auc files to disk
QString US_ConvertIO::readRawDataFromDB( US_ExpInfo::ExperimentInfo& ExpData, 
                                         QList< US_SolutionGui::TripleInfo >& triples,
                                         QString& dir )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
      return( db.lastError() );

   // Get the rawDataID's that correspond to this experiment
   QStringList q( "get_rawDataIDs" );
   q  << QString::number( ExpData.expID );
   db.query( q );

   QStringList rawDataIDs;
   QStringList filenames;

   while ( db.next() )
   {
      rawDataIDs << db.value( 0 ).toString();
      filenames  << db.value( 2 ).toString();
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
      db.readBlobFromDB( f, QString( "download_aucData" ), rawDataIDs[ i ].toInt() );
   }

   // Get the other db info and create triples
   QString error = NULL;
   triples.clear();
   for ( int i = 0; i < rawDataIDs.size(); i++ )
   {
      US_SolutionGui::TripleInfo triple;

      q.clear();
      q  << "get_rawData"
         << rawDataIDs[ i ];
      db.query( q );

      if ( db.next() )
      {
         QString uuidc         = db.value( 0 ).toString();
         uuid_parse( uuidc.toAscii(), (unsigned char*) triple.tripleGUID );
         //triple.label          = db.value( 1 ).toString();
         triple.tripleFilename = db.value( 2 ).toString();
         //triple.tripleComments = db.value( 3 ).toString();
         triple.tripleID       = rawDataIDs[ i ].toInt();
         int solutionID        = db.value( 5 ).toInt();

         QStringList part      = triple.tripleFilename.split( "." );
         triple.tripleDesc     = part[ 2 ] + " / " + part[ 3 ] + " / " + part[ 4 ];
         triple.excluded       = false;

         triple.centerpiece    = 0;           // default values
         triple.bufferID       = 0;
         triple.analyteID      = 0;

         // Try to get buffer info
         q.clear();
         q  << "get_solution"
            << QString::number( solutionID );
         db.query( q );

         if ( db.next() )
         {
            uuidc              = db.value( 0 ).toString();
            uuid_parse( uuidc.toAscii(), (unsigned char*) triple.solutionGUID );
            triple.description = db.value( 1 ).toString();
            triple.storageTemp = db.value( 2 ).toInt();
            triple.notes       = db.value( 3 ).toString();

            q.clear();
            q  << "get_solutionBuffer"
               << QString::number( solutionID );
            db.query( q );
            if ( db.next() )
            {
               triple.bufferID   = db.value( 0 ).toInt();
               triple.bufferGUID = db.value( 1 ).toString();
               triple.bufferDesc = db.value( 2 ).toString();
            }

            q.clear();
            q  << "get_solutionAnalyte"
               << QString::number( solutionID );
            db.query( q );
            if ( db.next() )
            {
               triple.analyteID   = db.value( 0 ).toInt();
               triple.analyteGUID = db.value( 1 ).toString();
               triple.analyteDesc = db.value( 2 ).toString();
            }
         }

         // save it   
         triples << triple;
      }

      else
         error += "Error processing file: " + filenames[ i ] + "\n" +
                  db.lastError() + "\n";
   }
      
   // Get runType
   QStringList part = triples[ 0 ].tripleFilename.split( "." );
   ExpData.opticalSystem = part[ 1 ].toAscii();

   if ( error != NULL )
      return( error );

   return( NULL );
}

// Function to read the ExperimentInfo structure from DB
QString US_ConvertIO::readExperimentInfoDB( US_ExpInfo::ExperimentInfo& expInfo )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
      return( db.lastError() );

   // Investigator info
   QStringList q( "get_person_info" );
   q << QString::number( expInfo.invID );
   db.query( q );
   if ( db.next() )
   {
      expInfo.firstName = db.value( 0 ).toString();
      expInfo.lastName  = db.value( 1 ).toString();
      expInfo.invGUID   = db.value( 9 ).toString();
   }

/*
   // Experiment Info
   opticalSystem
   rpms??

   // Triples
   tripleID;      
   centerpiece;   
   bufferID;      
   bufferGUID;    
   bufferDesc;    
   analyteID;     
   analyteGUID;   
   analyteDesc;   
   tripleGUID[16];
   tripleFilename;
*/

   // Hardware info
   expInfo.operatorGUID = QString( "" );
   q.clear();
   q  << QString( "get_person_info" )
      << QString::number( expInfo.operatorID );
   db.query( q );
   if ( db.next() )
      expInfo.operatorGUID   = db.value( 9 ).toString();

   expInfo.labGUID = QString( "" );
   q.clear();
   q << QString( "get_lab_info" )
     << QString::number( expInfo.labID );
   db.query( q );
   if ( db.next() )
      expInfo.labGUID = db.value( 0 ).toString();

   expInfo.instrumentSerial = QString( "" );
   q.clear();
   q << QString( "get_instrument_info" )
     << QString::number( expInfo.instrumentID );
   db.query( q );
   if ( db.next() )
      expInfo.instrumentSerial = db.value( 1 ).toString();

   expInfo.rotorGUID = QString( "" );
   q.clear();
   q << QString( "get_rotor_info" )
     << QString::number( expInfo.rotorID );
   db.query( q );
   if ( db.next() )
   {
      expInfo.rotorGUID   = db.value( 0 ).toString();
      expInfo.rotorSerial = db.value( 2 ).toString();
   }

   return( NULL );
}

int US_ConvertIO::writeXmlFile(
    US_ExpInfo::ExperimentInfo& ExpData,
    QList< US_SolutionGui::TripleInfo >& triples,
    QString runType,
    QString runID,
    QString dirname )
{ 
   if ( ExpData.invID == 0 ) return( US_Convert::NOXML ); 

   if ( dirname.right( 1 ) != "/" ) dirname += "/"; // Ensure trailing /
   QString writeFile = runID      + "." 
                     + runType    + ".xml";
   QFile file( dirname + writeFile );
   if ( !file.open( QIODevice::WriteOnly | QIODevice::Text) )
      return( US_Convert::CANTOPEN );

   QXmlStreamWriter xml;
   xml.setDevice( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD("<!DOCTYPE US_Scandata>");
   xml.writeStartElement("US_Scandata");
   xml.writeAttribute("version", "1.0");

   // elements
   xml.writeStartElement( "experiment" );
   xml.writeAttribute   ( "id",   QString::number( ExpData.expID ) );
   xml.writeAttribute   ( "guid", ExpData.expGUID );
   xml.writeAttribute   ( "type", ExpData.expType );
   xml.writeAttribute   ( "runID", ExpData.runID );

      xml.writeStartElement( "investigator" );
      xml.writeAttribute   ( "id", QString::number( ExpData.invID ) );
      xml.writeAttribute   ( "guid", ExpData.invGUID );
      xml.writeEndElement  ();
      
      xml.writeStartElement( "name" );
      xml.writeAttribute   ( "first", ExpData.firstName );
      xml.writeAttribute   ( "last",  ExpData.lastName  );
      xml.writeEndElement  ();
      
      xml.writeStartElement( "project" );
      xml.writeAttribute   ( "id", QString::number( ExpData.projectID ) );
      xml.writeEndElement  ();
      
      xml.writeStartElement( "lab" );
      xml.writeAttribute   ( "id",   QString::number( ExpData.labID   ) );
      xml.writeAttribute   ( "guid", ExpData.labGUID );
      xml.writeEndElement  ();
      
      xml.writeStartElement( "instrument" );
      xml.writeAttribute   ( "id",     QString::number( ExpData.instrumentID ) );
      xml.writeAttribute   ( "serial", ExpData.instrumentSerial );
      xml.writeEndElement  ();
      
      xml.writeStartElement( "operator" );
      xml.writeAttribute   ( "id", QString::number( ExpData.operatorID ) );
      xml.writeAttribute   ( "guid", ExpData.operatorGUID );
      xml.writeEndElement  ();

      xml.writeStartElement( "rotor" );
      xml.writeAttribute   ( "id",     QString::number( ExpData.rotorID   ) );
      xml.writeAttribute   ( "guid",   ExpData.rotorGUID );
      xml.writeAttribute   ( "serial", ExpData.rotorSerial );
      xml.writeEndElement  ();

      // loop through the following for c/c/w combinations
      for ( int i = 0; i < triples.size(); i++ )
      {
         US_SolutionGui::TripleInfo t = triples[ i ];
         if ( t.excluded ) continue;

         QString triple         = t.tripleDesc;
         QStringList parts      = triple.split(" / ");

         QString     cell       = parts[ 0 ];
         QString     channel    = parts[ 1 ];
         QString     wl         = parts[ 2 ];

         char uuidc[ 37 ];
         uuid_unparse( (unsigned char*)t.tripleGUID, uuidc );
         xml.writeStartElement( "dataset" );
         xml.writeAttribute   ( "id", QString::number( t.tripleID ) );
         xml.writeAttribute   ( "guid", QString( uuidc ) );
         xml.writeAttribute   ( "cell", cell );
         xml.writeAttribute   ( "channel", channel );
         xml.writeAttribute   ( "wavelength", wl );

            xml.writeStartElement( "centerpiece" );
            xml.writeAttribute   ( "id", QString::number( t.centerpiece ) );
            xml.writeEndElement  ();

            xml.writeStartElement( "solution" );
            xml.writeAttribute   ( "guid", t.solutionGUID );
            int st = ( t.storageTemp ) ? 1 : 0;
            xml.writeAttribute   ( "storageTemp", QString::number( st ) );
            xml.writeTextElement ( "description", t.description );
            xml.writeTextElement ( "notes", t.notes );
            xml.writeEndElement  ();

            xml.writeStartElement( "buffer" );
            xml.writeAttribute   ( "id",   QString::number( t.bufferID ) );
            xml.writeAttribute   ( "guid", t.bufferGUID );
            xml.writeAttribute   ( "desc", t.bufferDesc );
            xml.writeEndElement  ();

            xml.writeStartElement( "analyte" );
            xml.writeAttribute   ( "id",   QString::number( t.analyteID ) );
            xml.writeAttribute   ( "guid", t.analyteGUID );
            xml.writeAttribute   ( "desc", t.analyteDesc );
            xml.writeEndElement  ();

         xml.writeEndElement   ();
      }

   xml.writeStartElement( "opticalSystem" );
   xml.writeAttribute   ( "value", ExpData.opticalSystem );
   xml.writeEndElement  ();

   xml.writeStartElement( "date" );
   xml.writeAttribute   ( "value", ExpData.date );
   xml.writeEndElement  ();

   xml.writeStartElement( "runTemp" );
   xml.writeAttribute   ( "value", ExpData.runTemp );
   xml.writeEndElement  ();

   xml.writeTextElement ( "label", ExpData.label );
   xml.writeTextElement ( "comments", ExpData.comments );
   xml.writeTextElement ( "centrifugeProtocol", ExpData.centrifugeProtocol );

   xml.writeEndElement(); // US_Scandata
   xml.writeEndDocument();

   return( US_Convert::OK );
}

int US_ConvertIO::readXmlFile( 
    US_ExpInfo::ExperimentInfo& ExpData,
    QList< US_SolutionGui::TripleInfo >& triples,
    QString runType,
    QString runID,
    QString dirname )
{
   // First figure out the xml file name, and try to open it
   QString filename = runID      + "." 
                    + runType    + ".xml";

   QFile f( dirname + filename );
   if ( ! f.open( QIODevice::ReadOnly ) ) return US_Convert::CANTOPEN;
   QTextStream ds( &f );

   QXmlStreamReader xml( &f );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "experiment" )
         {
            QXmlStreamAttributes a = xml.attributes();
            ExpData.expID          = a.value("id").toString().toInt();
            ExpData.expGUID        = a.value( "guid" ).toString();
            ExpData.expType        = a.value( "type" ).toString();
            ExpData.runID          = a.value( "runID" ).toString();
            readExperiment ( xml, ExpData, triples, runType, runID );
         }
      }
   }

   bool error = xml.hasError();
   f.close();

   if ( error ) return US_Convert::BADXML;

   return( verifyXml( ExpData, triples ) );
}

void US_ConvertIO::readExperiment( 
     QXmlStreamReader& xml, 
     US_ExpInfo::ExperimentInfo& ExpData,
     QList< US_SolutionGui::TripleInfo >& triples,
     QString runType,
     QString runID )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "experiment" ) return;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "investigator" )
         {
            QXmlStreamAttributes a = xml.attributes();
            ExpData.invID          = a.value( "id" ).toString().toInt();
            ExpData.invGUID        = a.value( "guid" ).toString();
         }
 
         else if ( xml.name() == "name" )
         {
            QXmlStreamAttributes a = xml.attributes();
            ExpData.firstName      = a.value( "first" ).toString();
            ExpData.lastName       = a.value( "last"  ).toString();
         }

         else if ( xml.name() == "project" )
         {
            QXmlStreamAttributes a = xml.attributes();
            ExpData.projectID      = a.value( "id" ).toString().toInt();
         }
   
         else if ( xml.name() == "lab" )
         {
            QXmlStreamAttributes a = xml.attributes();
            ExpData.labID    = a.value( "id" )  .toString().toInt();
            ExpData.labGUID  = a.value( "guid" ).toString();
         }
   
         else if ( xml.name() == "instrument" )
         {
            QXmlStreamAttributes a    = xml.attributes();
            ExpData.instrumentID      = a.value( "id" )    .toString().toInt();
            ExpData.instrumentSerial  = a.value( "serial" ).toString();
         }
   
         else if ( xml.name() == "operator" )
         {
            QXmlStreamAttributes a = xml.attributes();
            ExpData.operatorID     = a.value( "id" ).toString().toInt();
            ExpData.operatorGUID   = a.value( "guid" ).toString();
         }
   
         else if ( xml.name() == "rotor" )
         {
            QXmlStreamAttributes a = xml.attributes();
            ExpData.rotorID        = a.value( "id"     ).toString().toInt();
            ExpData.rotorGUID      = a.value( "guid"   ).toString();
            ExpData.rotorSerial    = a.value( "serial" ).toString();
         }
   
         else if ( xml.name() == "dataset" )
         {
            QXmlStreamAttributes a = xml.attributes();
            QString cell           = a.value( "cell" ).toString();
            QString channel        = a.value( "channel" ).toString();
            QString wl             = a.value( "wavelength" ).toString();
   
            // Find the index of this triple
            QString triple         = cell + " / " + channel + " / " + wl;
            bool found             = false;
            int ndx                = 0;
            for ( int i = 0; i < triples.size(); i++ )
            {
               if ( triples[ i ].excluded ) continue;

               if ( triple == triples[ i ].tripleDesc )
               {
                  found = true;
                  ndx   = i;
                  break;
               }
            }

            if ( found )
            {
               triples[ ndx ].tripleID = a.value( "id" ).toString().toInt();
               QString uuidc = a.value( "guid" ).toString();
               uuid_parse( uuidc.toAscii(), (unsigned char*) triples[ ndx ].tripleGUID );

               triples[ ndx ].tripleFilename = runID    + "." +
                                               runType  + "." +
                                               cell     + "." +
                                               channel  + "." +
                                               wl       + ".auc";

               triples[ ndx ].excluded       = false;

               readDataset( xml, triples[ ndx ] );
            }
         }

         else if ( xml.name() == "opticalSystem" )
         {
            QXmlStreamAttributes a = xml.attributes();
            ExpData.opticalSystem  = a.value( "value" ).toString().toAscii();
         }

         else if ( xml.name() == "date" )
         {
            QXmlStreamAttributes a = xml.attributes();
            ExpData.date           = a.value( "value" ).toString();
         }

         else if ( xml.name() == "runTemp" )
         {
            QXmlStreamAttributes a = xml.attributes();
            ExpData.runTemp        = a.value( "value" ).toString();
         }

         else if ( xml.name() == "label" )
         {
            xml.readNext();
            ExpData.label = xml.text().toString();
         }

         else if ( xml.name() == "comments" )
         {
            xml.readNext();
            ExpData.comments = xml.text().toString();
         }

         else if ( xml.name() == "centrifugeProtocol" )
         {
            xml.readNext();
            ExpData.centrifugeProtocol = xml.text().toString();
         }

      }

   }
}

void US_ConvertIO::readDataset( QXmlStreamReader& xml, US_SolutionGui::TripleInfo& triple )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "dataset" ) return;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "centerpiece" )
         {
            QXmlStreamAttributes a = xml.attributes();
            triple.centerpiece     = a.value( "id" ).toString().toInt();
         }
 
         else if ( xml.name() == "solution" )
         {
            QXmlStreamAttributes a = xml.attributes();
            QString uuidc = a.value( "guid" ).toString();
            uuid_parse( uuidc.toAscii(), (unsigned char*) triple.solutionGUID );
            int st                 = a.value( "storageTemp" ).toString().toInt();
            triple.storageTemp     = ( st == 1 );

            readSolutionInfo( xml, triple );
         }

         else if ( xml.name() == "buffer" )
         {
            QXmlStreamAttributes a = xml.attributes();
            triple.bufferID        = a.value( "id" )  .toString().toInt();
            triple.bufferGUID      = a.value( "guid" ).toString();
            triple.bufferDesc      = a.value( "desc" ).toString();
         }
   
         else if ( xml.name() == "analyte" )
         {
            QXmlStreamAttributes a = xml.attributes();
            triple.analyteID       = a.value( "id" ).toString().toInt();
            triple.analyteGUID     = a.value( "guid" ).toString();
            triple.analyteDesc     = a.value( "desc" ).toString();
         }
      }
   }
}

void US_ConvertIO::readSolutionInfo( QXmlStreamReader& xml, US_SolutionGui::TripleInfo& triple )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "solution" ) return;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "description" )
         {
            xml.readNext();
            triple.description = xml.text().toString();
         }

         if ( xml.name() == "notes" )
         {
            xml.readNext();
            triple.notes = xml.text().toString();
         }
      }
   }
}

int US_ConvertIO::verifyXml( US_ExpInfo::ExperimentInfo& ExpData,
                             QList< US_SolutionGui::TripleInfo >& triples )
{
   US_Passwd pw;
   US_DB2 db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
      return( US_Convert::NODB );

   int status = US_Convert::OK;

   // Double check investigator GUID
   QStringList q( "get_personID_from_GUID" );
   q << QString( ExpData.invGUID );
   db.query( q );

   if ( db.lastErrno() != US_DB2::OK )
   {
      ExpData.invID   = 0;
      ExpData.invGUID = QString( "" );
      status = US_Convert::BADGUID;
   }

   else
   {
      // Save updated investigator ID
      ExpData.invID = db.value( 0 ).toInt();
   }

   // Double check operator GUID
   q.clear();
   q << QString( "get_personID_from_GUID" )
     << QString( ExpData.operatorGUID );
   db.query( q );

   if ( db.lastErrno() != US_DB2::OK )
   {
      ExpData.operatorID   = 0;
      ExpData.operatorGUID = QString( "" );
      status = US_Convert::BADGUID;
   }

   else
   {
      // Save updated investigator ID
      ExpData.operatorID = db.value( 0 ).toInt();
   }

   // Double check triple GUID's
   for ( int i = 0; i < triples.size(); i++ )
   {
      if ( triples[ i ].excluded ) continue;

      char uuidc[ 37 ];
      uuid_unparse( (unsigned char*) triples[ i ].tripleGUID, uuidc );
      q.clear();
      q << QString( "get_rawDataID_from_GUID" )
        << QString( uuidc );
      db.query( q );

      if ( db.lastErrno() != US_DB2::OK )
      {
         triples[ i ].tripleID = 0;
         memset( triples[ i ].tripleGUID, 0, 16 );
         status = US_Convert::BADGUID;
      }

      else
      {
         // Save updated triple ID
         triples[ i ].tripleID = db.value( 0 ).toString().toInt();
      }
   }

   // Double check rotor GUID
   q.clear();
   q << QString( "get_rotorID_from_GUID" )
     << QString( ExpData.rotorGUID );
   db.query( q );

   if ( db.lastErrno() != US_DB2::OK )
   {
      ExpData.rotorID     = 0;
      ExpData.rotorGUID   = QString( "" );
      ExpData.rotorSerial = QString( "" );
      status = US_Convert::BADGUID;
   }

   else
   {
      // Save updated rotor info 
      ExpData.rotorID = db.value( 0 ).toInt();
      q.clear();
      q << QString( "get_rotor_info" )
        << QString::number( ExpData.rotorID );
      db.query( q );
      if ( db.next() )
      {
         ExpData.rotorGUID   = db.value( 0 ).toString();
         ExpData.rotorSerial = db.value( 2 ).toString();
      }
   }

   // Double check lab GUID
   q.clear();
   q << QString( "get_labID_from_GUID" )
     << QString( ExpData.labGUID );
   db.query( q );

   if ( db.lastErrno() != US_DB2::OK )
   {
      ExpData.labID   = 0;
      ExpData.labGUID = QString( "" );
      status = US_Convert::BADGUID;
   }

   else
   {
      // Save updated investigator ID
      ExpData.labID = db.value( 0 ).toInt();
   }

   return status;
}
