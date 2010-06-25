//! \file us_convertio.cpp

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

QString US_ConvertIO::newDBExperiment( US_ExpInfo::ExperimentInfo& ExpData )
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
   return( NULL );
}

QString US_ConvertIO::updateDBExperiment( US_ExpInfo::ExperimentInfo& ExpData )
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

      xml.writeStartElement( "investigator" );
      xml.writeAttribute   ( "id", QString::number( ExpData.invID ) );
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

         QString triple         = triples[ t.tripleID ];
         QStringList parts      = triple.split(" / ");

         QString     cell       = parts[ 0 ];
         QString     channel    = parts[ 1 ];
         QString     wl         = parts[ 2 ];

         char uuidc[ 37 ];
         uuid_unparse( (unsigned char*)t.guid, uuidc );
         xml.writeStartElement( "dataset" );
         xml.writeAttribute   ( "cell", cell );
         xml.writeAttribute   ( "channel", channel );
         xml.writeAttribute   ( "wavelength", wl );
         xml.writeAttribute   ( "guid", QString( uuidc ) );

            xml.writeStartElement( "centerpiece" );
            xml.writeAttribute   ( "id", QString::number( t.centerpiece ) );
            xml.writeEndElement  ();

            xml.writeStartElement( "buffer" );
            xml.writeAttribute   ( "id",   QString::number( t.bufferID ) );
            xml.writeAttribute   ( "guid", t.bufferGUID );
            xml.writeEndElement  ();

            xml.writeStartElement( "analyte" );
            xml.writeAttribute   ( "id",   QString::number( t.analyteID ) );
            xml.writeAttribute   ( "guid", t.analyteGUID );
            xml.writeEndElement  ();

         xml.writeEndElement   ();
      }

   xml.writeTextElement ( "date", ExpData.date );
   xml.writeTextElement ( "runTemp", ExpData.runTemp );
   xml.writeTextElement ( "label", ExpData.label );
   xml.writeTextElement ( "comments", ExpData.comments );
   xml.writeTextElement ( "centrifugeProtocol", ExpData.centrifugeProtocol );

   xml.writeEndElement(); // US_Scandata
   xml.writeEndDocument();

//   if ( ExpData.triples.size() != triples.size() )
//      return( US_Convert::PARTIAL_XML );

   return( US_Convert::OK );
}
