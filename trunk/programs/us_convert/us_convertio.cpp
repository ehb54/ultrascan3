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
                                        QString dir )
{
   // Update database
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
      return( db.lastError() );

   QString error = QString( "" );

   // Delete all existing solutions and rawData, because we're starting over 
   QStringList q( "delete_rawData" );
   q << QString::number( ExpData.expID );
   int status = db.statusQuery( q );

   if ( status != US_DB2::OK )
      return( db.lastError() );

   q.clear();
   q  << "delete_experiment_solutions" 
      << QString::number( ExpData.expID );
   status = db.statusQuery( q );

   if ( status != US_DB2::OK )
      return( db.lastError() );

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
      db.query( q );

      status = db.lastErrno();
      triple.solution.solutionID = 0;
      if ( status == US_DB2::OK )
      {
         db.next();
         triple.solution.solutionID = db.value( 0 ).toInt();
      }

      else if ( status == US_DB2::NOROWS )
      {
         // Solution is not in db, so try to add it
         int diskStatus = triple.solution.saveToDB( ExpData.expID, 
                                                    1,               // figure out channelID later ??
                                                    &db );
//qDebug() << "triple.solution.saveToDB db status = " << QString::number( diskStatus );
         if ( diskStatus == US_DB2::NO_BUFFER )
            error += "Error processing buffer " + triple.solution.bufferGUID + '\n' +
                     "Buffer was not found in the database";

         else if ( diskStatus == US_DB2::NO_ANALYTE )
            error += "Error processing analyte \n" 
                     "An analyte was not found in the database";

         else if ( diskStatus == US_DB2::NO_SOLUTION )
            error += "Error processing solution " + triple.solution.solutionGUID + '\n' +
                     "Solution was not found in the database";

         else if ( diskStatus != US_DB2::OK )
            error += "Error saving solution to DB " + '\n' +
                     db.lastError();

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
      int status = db.statusQuery( q );
      if ( status != US_DB2::OK )
         error += "Error returned writing cell record: " + cellGUID + "\n" +
                  status + " " + db.lastError() + "\n";

   }

   if ( error != QString( "" ) )
      return( error );

   return( QString( "" ) );
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

   US_Experiment ExpData;       // A local copy
   QList< US_Convert::TripleInfo > triples;  // a local copy
   QStringList q( "get_experiment_info_by_runID" );
   q << runID
     << QString::number( US_Settings::us_inv_ID() );
   db.query( q );
 
   if ( db.next() )
   {
      ExpData.runID              = runID;
      ExpData.project.projectID          = db.value( 0 ).toInt();
      ExpData.expID              = db.value( 1 ).toInt();
      ExpData.expGUID            = db.value( 2 ).toString();
      ExpData.labID              = db.value( 3 ).toInt();
      ExpData.instrumentID       = db.value( 4 ).toInt();
      ExpData.operatorID         = db.value( 5 ).toInt();
      ExpData.rotorID            = db.value( 6 ).toInt();
      ExpData.calibrationID      = db.value( 7 ).toInt();
      ExpData.expType            = db.value( 8 ).toString();
      ExpData.runTemp            = db.value( 9 ).toString();
      ExpData.label              = db.value( 10 ).toString();
      ExpData.comments           = db.value( 11 ).toString();
      ExpData.centrifugeProtocol = db.value( 12 ).toString();
      ExpData.date               = db.value( 13 ).toString();
      ExpData.invID              = db.value( 14 ).toInt();
   }

   else if ( db.lastErrno() == US_DB2::NOROWS )
      return( "The current run ID is not found in the database." );

   else
      return( db.lastError() );

   int infoStatus = ExpData.readSecondaryInfoDB( &db );
   if ( infoStatus != US_DB2::OK )
      return ( "DB Error: " + QString::number( infoStatus ) );

   // Clear out the directory, in case the user has messed with it locally
   QDir d( dir );
   QStringList files = d.entryList( QDir::NoDotAndDotDot | QDir::Files );
   foreach ( QString file, files )
      d.remove( file );
   
   // Now read the auc data
   QString status = readRawDataFromDB( ExpData, triples, dir );
   if ( status != QString( "" ) )
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

   return( QString( "" ) );
}

// Function to read the auc files to disk
QString US_ConvertIO::readRawDataFromDB( US_Experiment& ExpData, 
                                         QList< US_Convert::TripleInfo >& triples,
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

   // Now get the centerpiece info
   int commonCenterpiece = 0;
   q.clear();
   q  << "all_cell_experiments"
      << QString::number( ExpData.expID );
   db.query( q );
   if ( db.next() )
      commonCenterpiece = db.value( 3 ).toInt();

   // Get the other db info and create triples
   QString error = QString( "" );
   triples.clear();
   for ( int i = 0; i < rawDataIDs.size(); i++ )
   {
      US_Convert::TripleInfo triple;

      q.clear();
      q  << "get_rawData"
         << rawDataIDs[ i ];
      db.query( q );

      if ( db.next() )
      {
         QString uuidc         = db.value( 0 ).toString();
         US_Util::uuid_parse( uuidc, (unsigned char*) triple.tripleGUID );
         //triple.label             = db.value( 1 ).toString();
         triple.tripleFilename      = db.value( 2 ).toString();
         //triple.tripleComments    = db.value( 3 ).toString();
         triple.tripleID            = rawDataIDs[ i ].toInt();
         triple.solution.solutionID = db.value( 5 ).toInt();

         QStringList part      = triple.tripleFilename.split( "." );
         triple.tripleDesc     = part[ 2 ] + " / " + part[ 3 ] + " / " + part[ 4 ];
         triple.excluded       = false;

         triple.centerpiece    = commonCenterpiece;
/*         // Try to get solution info
         q.clear();
         q  << "get_solutionID_from_GUID"
            << triple.solution.solutionGUID;
      db.query( q );

      if ( db.lastErrno() == US_DB2::OK && db.next() )
         triple.solution.solutionID = db.value( 0 ).toInt();
*/

         // Try to get more solution info
         triple.solution.solutionGUID = QString( "" );
         triple.solution.solutionDesc = QString( "" );
         q.clear();
         q  << "get_solution"
            << QString::number( triple.solution.solutionID );
         db.query( q );

         if ( db.lastErrno() == US_DB2::NO_SOLUTION )
         {
            // no solution was found
            triple.solution.solutionID = 0;
         }

         else if ( db.next() )
         {
            triple.solution.solutionGUID = db.value( 0 ).toString();
            triple.solution.solutionDesc = db.value( 1 ).toString();
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

   if ( error != QString( "" ) )
      return( error );

   return( QString( "" ) );
}

int US_ConvertIO::writeXmlFile(
    US_Experiment& ExpData,
    QList< US_Convert::TripleInfo >& triples,
    QString runType,
    QString runID,
    QString dirname )
{ 
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );


   if ( ExpData.expGUID.isEmpty() || ! rx.exactMatch( ExpData.expGUID ) )
      ExpData.expGUID = US_Util::new_guid();

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
      xml.writeAttribute   ( "value", ExpData.name );
      xml.writeEndElement  ();
      
      xml.writeStartElement( "project" );
      xml.writeAttribute   ( "id",   QString::number( ExpData.project.projectID ) );
      xml.writeAttribute   ( "guid", ExpData.project.projectGUID );
      xml.writeAttribute   ( "desc", ExpData.project.projectDesc );
      xml.writeEndElement  ();
      
      xml.writeStartElement( "lab" );
      xml.writeAttribute   ( "id",   QString::number( ExpData.labID   ) );
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
      xml.writeAttribute   ( "name", ExpData.rotorName );
      xml.writeEndElement  ();

      xml.writeStartElement( "calibration" );
      xml.writeAttribute   ( "id",     QString::number( ExpData.calibrationID   ) );
      xml.writeAttribute   ( "coeff1", QString::number( ExpData.rotorCoeff1     ) );
      xml.writeAttribute   ( "coeff2", QString::number( ExpData.rotorCoeff2     ) );
      xml.writeAttribute   ( "date",   ExpData.rotorUpdated.toString( "yyyy-MM-dd" ) );
      xml.writeEndElement  ();

      // loop through the following for c/c/w combinations
      for ( int i = 0; i < triples.size(); i++ )
      {
         US_Convert::TripleInfo t = triples[ i ];
         if ( t.excluded ) continue;

         QString triple         = t.tripleDesc;
         QStringList parts      = triple.split(" / ");

         QString     cell       = parts[ 0 ];
         QString     channel    = parts[ 1 ];
         QString     wl         = parts[ 2 ];

         QString uuidc = US_Util::uuid_unparse( (unsigned char*)t.tripleGUID );
         xml.writeStartElement( "dataset" );
         xml.writeAttribute   ( "id", QString::number( t.tripleID ) );
         xml.writeAttribute   ( "guid", uuidc );
         xml.writeAttribute   ( "cell", cell );
         xml.writeAttribute   ( "channel", channel );
         xml.writeAttribute   ( "wavelength", wl );

            xml.writeStartElement( "centerpiece" );
            xml.writeAttribute   ( "id", QString::number( t.centerpiece ) );
            xml.writeEndElement  ();

            xml.writeStartElement( "solution" );
            xml.writeAttribute   ( "id",   QString::number( t.solution.solutionID ) );
            xml.writeAttribute   ( "guid", t.solution.solutionGUID );
            xml.writeAttribute   ( "desc", t.solution.solutionDesc );
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
    US_Experiment& ExpData,
    QList< US_Convert::TripleInfo >& triples,
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
     US_Experiment& ExpData,
     QList< US_Convert::TripleInfo >& triples,
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
            ExpData.name           = a.value( "value" ).toString();
         }

         else if ( xml.name() == "project" )
         {
            QXmlStreamAttributes a = xml.attributes();
            ExpData.project.projectID      = a.value( "id"   ).toString().toInt();
            ExpData.project.projectGUID    = a.value( "guid" ).toString();
            ExpData.project.projectDesc    = a.value( "desc" ).toString();
         }
   
         else if ( xml.name() == "lab" )
         {
            QXmlStreamAttributes a = xml.attributes();
            ExpData.labID    = a.value( "id" )  .toString().toInt();
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
            ExpData.rotorName      = a.value( "name" ).toString();
            ExpData.calibrationID  = a.value( "calibrationID" ).toString().toInt();
         }
   
         else if ( xml.name() == "calibration" )
         {
            QXmlStreamAttributes a = xml.attributes();
            ExpData.calibrationID      = a.value( "id"     ).toString().toInt();
            ExpData.rotorCoeff1        = a.value( "coeff1" ).toString().toFloat();
            ExpData.rotorCoeff2        = a.value( "coeff2" ).toString().toFloat();
            ExpData.rotorUpdated       = QDate::fromString( a.value( "date" ).toString(), "yyyy-MM-dd" );
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
               US_Util::uuid_parse( uuidc, (unsigned char*) triples[ ndx ].tripleGUID );

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

void US_ConvertIO::readDataset( QXmlStreamReader& xml, US_Convert::TripleInfo& triple )
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
            triple.solution.solutionID   = a.value( "id"   ).toString().toInt();
            triple.solution.solutionGUID = a.value( "guid" ).toString();
            triple.solution.solutionDesc = a.value( "desc" ).toString();
         }

      }
   }
}

int US_ConvertIO::verifyXml( US_Experiment& ExpData,
                             QList< US_Convert::TripleInfo >& triples )
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

      QString uuidc = US_Util::uuid_unparse( (unsigned char*) triples[ i ].tripleGUID );
      q.clear();
      q << QString( "get_rawDataID_from_GUID" )
        << uuidc;
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
      ExpData.rotorID       = 0;
      ExpData.calibrationID = 0;
      ExpData.rotorCoeff1   = 0.0;
      ExpData.rotorCoeff2   = 0.0;
      ExpData.rotorGUID     = QString( "" );
      ExpData.rotorSerial   = QString( "" );
      ExpData.rotorName     = QString( "" );
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
         ExpData.rotorName   = db.value( 1 ).toString();
         ExpData.rotorSerial = db.value( 2 ).toString();
      }

      // For now, use first calibrationID
      ExpData.calibrationID = 0;
      q.clear();
      q << QString( "get_rotor_calibration_profiles" )
        << QString::number( ExpData.rotorID );
      db.query( q );
      if ( db.next() )
         ExpData.calibrationID = db.value( 0 ).toInt();
   }

   return status;
}
