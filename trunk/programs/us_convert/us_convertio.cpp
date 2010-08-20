//! \file us_convertio.cpp

#include <QDomDocument>

#include <uuid/uuid.h>

#include "us_settings.h"
#include "us_math2.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_convertio.h"

// Generic constructor
US_ConvertIO::US_ConvertIO( void )
{
}

QString US_ConvertIO::newDBExperiment( US_ExpInfo::ExperimentInfo& ExpData, 
                                       QList< int >& tripleMap,
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
   return( writeRawDataToDB( ExpData, tripleMap, dir ) );
}

QString US_ConvertIO::updateDBExperiment( US_ExpInfo::ExperimentInfo& ExpData, 
                                          QList< int >& tripleMap,
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
   return( writeRawDataToDB( ExpData, tripleMap, dir ) );
}

QString US_ConvertIO::writeRawDataToDB( US_ExpInfo::ExperimentInfo& ExpData, 
                                        QList< int >& tripleMap,
                                        QString dir )
{
   // Update database
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
      return( db.lastError() );

   // Delete all existing rawData, because we're starting over 
   QStringList q( "delete_rawData" );
   q << QString::number( ExpData.expID );
   int status = db.statusQuery( q );

   if ( status != US_DB2::OK )
      return( db.lastError() );

   // We assume there are files, because calling program checked

   // Read all data
   QString error = NULL;
   for ( int i = 0; i < tripleMap.size(); i++ )
   {
      int ndx = tripleMap[ i ];
      US_ExpInfo::TripleInfo triple = ExpData.triples[ ndx ];

      // Convert uuid to long form
      char uuidc[ 37 ];
      uuid_unparse( (unsigned char*) triple.tripleGUID, uuidc );

      QStringList q( "new_rawData" );
      q  << QString( uuidc )
         << ExpData.label
         << triple.tripleFilename       // needs to be base name only
         << ExpData.comments
         << QString::number( ExpData.expID )
         << "1" ;           // channel ID

      status = db.statusQuery( q );

      int rawDataID = db.lastInsertID();
      if ( status == US_DB2::OK )
      {
         // If ok, then let's save the tripleID
         ExpData.triples[ i ].tripleID = rawDataID;

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

int US_ConvertIO::writeXmlFile(
    US_ExpInfo::ExperimentInfo& ExpData,
    QStringList& triples,
    QList< int >& tripleMap,
    QString runType,
    QString runID,
    QString dirname )
{ 
   if ( ExpData.invID == 0 ) return( US_Convert::NOXML ); 

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
      xml.writeAttribute   ( "id",   QString::number( ExpData.rotorID   ) );
      xml.writeAttribute   ( "guid", ExpData.rotorGUID );
      xml.writeEndElement  ();

      // loop through the following for c/c/w combinations
      for ( int i = 0; i < tripleMap.size(); i++ )
      {
         int ndx = tripleMap[ i ];
         US_ExpInfo::TripleInfo t = ExpData.triples[ ndx ];

         QString triple         = triples[ ndx ];
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

//   if ( ExpData.triples.size() != triples.size() )
//      return( US_Convert::PARTIAL_XML );

   return( US_Convert::OK );
}

int US_ConvertIO::readXmlFile( 
    US_ExpInfo::ExperimentInfo& ExpData,
    QStringList& triples,
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

   return( verifyXml( ExpData ) );
}

void US_ConvertIO::readExperiment( 
     QXmlStreamReader& xml, 
     US_ExpInfo::ExperimentInfo& ExpData,
     QStringList& triples,
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
            ExpData.rotorID        = a.value( "id" )  .toString().toInt();
            ExpData.rotorGUID      = a.value( "guid" ).toString();
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
               if ( triple == triples[ i ] )
               {
                  found = true;
                  ndx   = i;
                  break;
               }
            }

            if ( found )
            {
               ExpData.triples[ ndx ].tripleID = a.value( "id" ).toString().toInt();
               QString uuidc = a.value( "guid" ).toString();
               uuid_parse( uuidc.toAscii(), (unsigned char*) ExpData.triples[ ndx ].tripleGUID );

               ExpData.triples[ ndx ].tripleFilename = runID    + "." +
                                                       runType  + "." +
                                                       cell     + "." +
                                                       channel  + "." +
                                                       wl       + ".auc";

               readDataset( xml, ExpData.triples[ ndx ] );
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

void US_ConvertIO::readDataset( QXmlStreamReader& xml, US_ExpInfo::TripleInfo& triple )
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

int US_ConvertIO::verifyXml( US_ExpInfo::ExperimentInfo& ExpData )
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
   for ( int i = 0; i < ExpData.triples.size(); i++ )
   {
      char uuidc[ 37 ];
      uuid_unparse( (unsigned char*) ExpData.triples[ i ].tripleGUID, uuidc );
      q.clear();
      q << QString( "get_rawDataID_from_GUID" )
        << QString( uuidc );
      db.query( q );

      if ( db.lastErrno() != US_DB2::OK )
      {
         ExpData.triples[ i ].tripleID = 0;
         memset( ExpData.triples[ i ].tripleGUID, 0, 16 );
         status = US_Convert::BADGUID;
      }

      else
      {
         // Save updated triple ID
         ExpData.triples[ i ].tripleID = db.value( 0 ).toString().toInt();
      }
   }

   // Double check rotor GUID
   q.clear();
   q << QString( "get_rotorID_from_GUID" )
     << QString( ExpData.rotorGUID );
   db.query( q );

   if ( db.lastErrno() != US_DB2::OK )
   {
      ExpData.rotorID   = 0;
      ExpData.rotorGUID = QString( "" );
      status = US_Convert::BADGUID;
   }

   else
   {
      // Save updated investigator ID
      ExpData.rotorID = db.value( 0 ).toInt();
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
