//! \file us_eprofile.cpp
#include "us_eprofile.h"
#include "us_settings.h"

// This source contains static utility functions for handling extinctionProfile
// XML strings and database tables.
// ALL functions are static. An example of use within other source:
//
//#include "us_eprofile.h"
// ...
//     US_ExtProfile::fetch_eprofile( db, compID, compType, valType,
//                                    buffer->extinction );
//

// Fetch an extinction profile from the database
int US_ExtProfile::fetch_eprofile( US_DB2* dbP, const int compID,
      QString& compType, QString& valType, QMap< double, double >& extinction )
{
   int profileID    = -1;    // Default (error) profile ID
   int xCompID;              // Values from XML
   QString xCompType;
   QString xValType;

   // Get values from a matching extinctionProfile record
   QStringList qry;
   qry << "get_eprofile" << QString::number( compID ) << compType;
   dbP->query( qry );        // DB query to get a profile of given ID,type

   if ( dbP->lastErrno() != US_DB2::OK )
      return profileID;

   dbP->next();              // Read a single record
   profileID        = dbP->value( 0 ).toInt();     // Profile ID
   valType          = dbP->value( 1 ).toString();  // Value type
   QString epxml    = dbP->value( 2 ).toString();  // Extinction profile XML

   // Parse the fetched XML and fill the extinction mapping
   parse_eprofile( epxml, &xCompID, &xCompType, &xValType, extinction );

   return profileID;
}

// Parse an extinction profile from an XML string
bool US_ExtProfile::parse_eprofile( QString& epxml, int* compID,
      QString* compType, QString* valType, QMap< double, double >& extinction )
{
   int xCompID;              // Values from XML
   QString xCompType;
   QString xValType;
   xCompID          = ( compID   == NULL ) ? 0                 : *compID;
   xCompType        = ( compType == NULL ) ? "Analyte"         : *compType;
   xValType         = ( valType  == NULL ) ? "molarExtinction" : *valType;

   QXmlStreamReader xml( epxml );
   int netag        = 0;     // Count of tags found (ext., extProf., analyte)
   int nptag        = 0;
   int natag        = 0;
   QXmlStreamAttributes attr;
   extinction.clear();

   while( ! xml.atEnd() )
   {
      xml.readNext();        // Get the next XML element

      if ( xml.isStartElement() )
      {                      // Handle start elements
         QString tname    = xml.name().toString();

         if ( tname == "extinction" )
         {                   // Get wavelength/value pairs
            netag++;
            attr             = xml.attributes();
            QString sWln     = attr.value( "wavelength" ).toString();
            QString sFrq     = attr.value( "frequency"  ).toString();
            double wl        = sWln.isEmpty() ? 280.0 : sWln.toDouble();
            wl               = sFrq.isEmpty() ? wl    : sFrq.toDouble();
            double va        = attr.value( "value" ).toString().toDouble();
            extinction[ wl ] = va;
         }

         else if ( tname == "extinctionProfile" )
         {                   // Get profile general attributes
            nptag++;
            attr             = xml.attributes();
            xCompID          = attr.value( "componentID" ).toString().toInt();
            xCompType        = attr.value( "componentType" ).toString();
            xValType         = attr.value( "valueType" ).toString();
         }

         else if ( tname == "analyte" )
         {                   // Flag that this is an analyte XML
            natag++;
         }

      }
   }

   if ( compID != NULL )
      *compID          = xCompID;   // Store general profile attribute values
   if ( compType != NULL )
      *compType        = xCompType;
   if ( valType != NULL )
      *valType         = xValType;

    return ( ( natag + nptag ) > 0 );
}

// Create a new extinction profile in the database
int US_ExtProfile::create_eprofile( US_DB2* dbP, const int compID,
      QString& compType, QString& valueType,
      QMap< double, double >& extinction )
{
  qDebug() << "compID: " << compID << ", compType: " << compType << ", valueType: " << valueType;

   int profileID    = -1;
   QString epxml;

   // Compose an XML string to represent the profile
   xml_eprofile( compID, compType, valueType, extinction, epxml );

   qDebug() << "BUFFER/ANALYTE eprofile: After xml_eprofile() ";

   // Determine if a record of this ID,Type exists in the DB
   QStringList qry;
   qry << "get_eprofile" << QString::number( compID ) << compType;
   dbP->query( qry );        // DB query to get a profile of given ID,type
   int num_prof     = dbP->numRows();
   
   qDebug() << "While creating/updating eprofile: num_prof: " << num_prof; 

   if ( num_prof > 0 )
   {  // Already existing record:  update it
      profileID        = dbP->value( 0 ).toInt();     // Profile ID
      qry.clear();
      qry << "update_eprofile" << QString::number( profileID )
          << QString::number( compID ) << compType << valueType << epxml;
      dbP->statusQuery( qry );
      qDebug() << "New profileID, num_prof >0: " << profileID;
   }

   else
   {  // No existing record:  create a new one
      qry.clear();
      qry << "new_eprofile" << QString::number( compID ) << compType
          << valueType << epxml;
      dbP->statusQuery( qry );
      profileID        = dbP->lastInsertID();         // Profile ID
      qDebug() << "New profileID, num_prof < 0: " << profileID;
   }
   qDebug() << "New profileID: " << profileID;
   return profileID;
}

// Create a new extinction profile XML string
bool US_ExtProfile::xml_eprofile( const int compID, QString& compType,
      QString& valueType, QMap< double, double >& extinction,
      QString& xmlout )
{
   bool create_ok   = true;
   QList< double > keys = extinction.keys();

   QXmlStreamWriter xml( &xmlout );
   xml.setAutoFormatting( true );
   xml.writeStartDocument();
   xml.writeDTD         ( "<!DOCTYPE US_EProfile>" );
   xml.writeStartElement( "EProfileData" );
   xml.writeAttribute   ( "version", "1.0" );

   xml.writeStartElement( "extinctionProfile" );
   xml.writeAttribute   ( "componentID",   QString::number( compID ) );
   xml.writeAttribute   ( "componentType", compType );
   xml.writeAttribute   ( "valueType",     valueType );

   for ( int ii = 0; ii < keys.size(); ii++ )
   {
      double wvln      = keys[ ii ];
      double valu      = extinction[ wvln ];
      QString sWvln    = QString::number( wvln );
      QString sValu    = QString::number( valu );
      
      qDebug() << "Writing XML: " << wvln << ", " << valu;
      
      xml.writeStartElement( "extinction" );
      xml.writeAttribute   ( "wavelength", sWvln );
      xml.writeAttribute   ( "value",      sValu );
      xml.writeEndElement();    // extinction
   }

   xml.writeEndElement();       // extinctionProfile
   xml.writeEndElement();       // EProfileData
   xml.writeEndDocument();

   return create_ok;
}

// Update an existing extinction profile in the database
bool US_ExtProfile::update_eprofile( US_DB2* dbP, const int profileID,
      const int compID, QString& compType, QString& valueType,
      QMap< double, double >& extinction )
{
   QString epxml;

   // Compose an XML string to represent the profile
   xml_eprofile( compID, compType, valueType, extinction, epxml );

   // Update an existing record
   QStringList qry;
   qry.clear();
   qry << "update_eprofile" << QString::number( profileID )
       << QString::number( compID ) << compType << valueType << epxml;
   dbP->statusQuery( qry );
   bool update_ok   = ( dbP->lastErrno() == US_DB2::OK );

   return update_ok;
}

// Delete an existing extinction profile from the database
bool US_ExtProfile::delete_eprofile( US_DB2* dbP, const int compID,
      QString& compType )
{
   QStringList qry;
   qry.clear();
   qry << "delete_eprofile" << QString::number( compID ) << compType;
   dbP->statusQuery( qry );
   bool delete_ok   = ( dbP->lastErrno() == US_DB2::OK );

   return delete_ok;
}

