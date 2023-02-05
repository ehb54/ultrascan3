//! \file us_buffer.cpp
#include "us_buffer.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_db2.h"
#include "us_datafiles.h"
#include "us_util.h"
#include "us_eprofile.h"

void US_BufferComponent::getAllFromDB( const QString& masterPW, 
         QMap< QString, US_BufferComponent >& componentList )
{
   US_DB2 db( masterPW );
   
   if ( db.lastErrno() != US_DB2::OK )
   {
      qDebug() << "Database Error" 
               << "US_BufferComponent ::connectDB: Could not open DB\n"
               << db.lastError();
      return;
   }

   QStringList q( "get_buffer_component_desc" );
   db.query( q );
   QStringList cids;

   while ( db.next() )
   {
      cids << db.value( 0 ).toString();
   }

   for ( int ii = 0; ii < cids.size(); ii++ )
   {
      US_BufferComponent c;
      c.componentID = cids.at( ii );
      c.getInfoFromDB( &db );
      componentList[ c.componentID ] = c;
   }
}

void US_BufferComponent::getInfoFromDB( US_DB2* db ) 
{
   QStringList q( "get_buffer_component_info" );
   q << componentID;

   db->query( q );
   db->next();

   unit              = db->value( 0 ).toString();
   name              = db->value( 1 ).toString();
   QString viscosity = db->value( 2 ).toString();
   QString density   = db->value( 3 ).toString();
   range             = db->value( 4 ).toString();
   grad_form         = US_Util::bool_flag( db->value( 5 ).toString() );

   QStringList sl    = viscosity.split( " " );

   for ( int i = 0; i < 6 ; i++ )
      visc_coeff[ i ]   = sl[ i ].toDouble();

   sl = density.split( " " );

   for ( int i = 0; i < 6 ; i++ )
      dens_coeff[ i ]   = sl[ i ].toDouble();

   sl                = sl.mid( 6 );  // Remove coefficients
   range             = range.isEmpty() ? sl.join( " " ) : range;
}

void US_BufferComponent::getAllFromHD( 
        QMap< QString, US_BufferComponent >& componentList ) 
{
   componentList.clear();

   QString fname  = US_Settings::appBaseDir() + "/etc/bufferComponents.xml";
   QFile   file( fname );

   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      // Fail quietly
      qDebug() << "Cannot open file " << fname;
      return;
   }

   QXmlStreamReader xml( &file );
   
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "component" )
            component( xml, componentList );
      }
   }

   file.close();
}

void US_BufferComponent::component( 
        QXmlStreamReader&                    xml, 
        QMap< QString, US_BufferComponent >& componentList ) 
{
   US_BufferComponent bc;

   QXmlStreamAttributes a = xml.attributes();
   bc.componentID = a.value( "id"    ).toString();
   bc.name        = a.value( "name"  ).toString();
   bc.unit        = a.value( "unit"  ).toString();
   bc.range       = a.value( "range" ).toString();
   bc.grad_form   = US_Util::bool_flag(
                    a.value( "gradient_forming" ).toString() );

   while ( ! xml.atEnd() )
   {

      if ( xml.isEndElement()  &&  xml.name() == "component" ) 
      {
         componentList[ bc.componentID ] = bc;
         return;
      }

      if ( xml.isStartElement()  &&  xml.name() == "densityCoefficients" )
      {
         QXmlStreamAttributes a = xml.attributes();
         bc.dens_coeff[ 0 ] = a.value( "c0" ).toString().toDouble();
         bc.dens_coeff[ 1 ] = a.value( "c1" ).toString().toDouble();
         bc.dens_coeff[ 2 ] = a.value( "c2" ).toString().toDouble();
         bc.dens_coeff[ 3 ] = a.value( "c3" ).toString().toDouble();
         bc.dens_coeff[ 4 ] = a.value( "c4" ).toString().toDouble();
         bc.dens_coeff[ 5 ] = a.value( "c5" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "viscosityCoefficients" )
      {
         QXmlStreamAttributes a = xml.attributes();
         bc.visc_coeff[ 0 ] = a.value( "c0" ).toString().toDouble();
         bc.visc_coeff[ 1 ] = a.value( "c1" ).toString().toDouble();
         bc.visc_coeff[ 2 ] = a.value( "c2" ).toString().toDouble();
         bc.visc_coeff[ 3 ] = a.value( "c3" ).toString().toDouble();
         bc.visc_coeff[ 4 ] = a.value( "c4" ).toString().toDouble();
         bc.visc_coeff[ 5 ] = a.value( "c5" ).toString().toDouble();
      }

      xml.readNext();
   }
}

void US_BufferComponent::putAllToHD( 
        const QMap< QString, US_BufferComponent >& componentList ) 
{
   QFile   file( US_Settings::appBaseDir() + "/etc/bufferComponents.xml" );
   
   if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text) )
   {
       qDebug() << "Cannot open file " << US_Settings::appBaseDir() + "/etc/bufferComponents.xml";
       return;
   }

   QXmlStreamWriter xml( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD         ( "<!DOCTYPE US_BufferComponents>" );
   xml.writeStartElement( "BufferComponents" );
   xml.writeAttribute   ( "version", "1.0" );
   
   QStringList keys = componentList.keys();

   for ( int i = 0; i < keys.size(); i++ )
   {
      QString key = keys[ i ];
      xml.writeStartElement( "component" );
      xml.writeAttribute( "id"   , componentList[ key ].componentID );
      xml.writeAttribute( "name" , componentList[ key ].name );
      xml.writeAttribute( "unit" , componentList[ key ].unit );
      xml.writeAttribute( "range", componentList[ key ].range );
      xml.writeAttribute( "gradient_forming",
             US_Util::bool_string( componentList[ key ].grad_form ) );

      QString factor;
      QString value;

      xml.writeStartElement( "densityCoefficients" );
      for ( int j = 0; j < 6; j++ )
      {
         factor.sprintf( "c%i", j );
         value = QString::number( componentList[ key ].dens_coeff[ j ], 'f', 5 );
         xml.writeAttribute( factor, value );
      }

      xml.writeEndElement(); // densityCoefficients

      xml.writeStartElement( "viscosityCoefficients" );
      for ( int j = 0; j < 6; j++ )
      {
         factor.sprintf( "c%i", j );
         value = QString::number( componentList[ key ].visc_coeff[ j ], 'f', 5 );
         xml.writeAttribute( factor, value );
      }

      xml.writeEndElement(); // viscosityCoefficients
      xml.writeEndElement(); // component
   }

   xml.writeEndElement(); // US_BufferComponents
   xml.writeEndDocument();

   file.close();
}

int US_BufferComponent::saveToDB(US_DB2 * db) {
   QStringList q;
   // construct density and viscosity
   QString density;
   for (int i = 0; i < 6; i++) {
      density += QString(i > 0 ? "" : " ") + QString::number(dens_coeff[i], 'f', 5);
   }
   QString viscosity;
   for (int i = 0; i < 6; i++) {
      viscosity += QString(i > 0 ? "" : " ") + QString::number(visc_coeff[i], 'f', 5);
   }

   q.clear();
   q << "create_buffer_component" << name << unit << range
     << US_Util::bool_string(grad_form) << density << viscosity;

   db->statusQuery(q);

   if (db->lastErrno() != US_DB2::OK) {
      qDebug() << "create_buffer_component error=" << db->lastErrno();
      return -1;
   }

   int idBufferComponent = db->lastInsertID();
   //qDebug() << "new_buffer-idBuffer" << idBuffer;

   qDebug() << "buffer_component_ID for new buffer component: " << idBufferComponent;
   componentID = QString::number(idBufferComponent);
}

//-------------  US_Buffer
US_Buffer::US_Buffer()
{
   compressibility = 0.0;
   pH              = WATER_PH;
   density         = DENS_20W;
   viscosity       = VISC_20W;
   manual          = false;
   replace_spectrum = false;
   new_or_changed_spectrum = false;
   person       .clear();
   bufferID     .clear();
   GUID         .clear();
   description  .clear();
   extinction   .clear();
   refraction   .clear();
   fluorescence .clear();
   component    .clear();
   componentIDs .clear();
   concentration.clear();
}

void US_Buffer::getSpectrum( US_DB2* db, const QString& type ) 
{
   QStringList q;
   q << "get_spectrum" << bufferID << "Buffer" << type;

   qDebug() << "GetSpectrum: " << q;
   
   db->query( q );

   while ( db->next() )
   {
     double lambda = db->value( 1 ).toDouble();
     double value  = db->value( 2 ).toDouble();
                  
      if ( type == "Extinction" ){
         extinction[ lambda ] = value;
         qDebug() << "Buffer->extinction details: " << lambda << " " << value;
      }
      else if ( type == "Refraction" )
         refraction[ lambda ] = value;
      else
         fluorescence[ lambda ] = value;
   }
}

void US_Buffer::putSpectrum( US_DB2* db, const QString& type ) const
{
   QStringList q;
   q << "new_spectrum" << bufferID << "Buffer" << type << "" << "";

   // We have to delete previous buffer->extinction from DB if spectrum updated from 'Edit Existing Buffer'

   qDebug() << "WritingSpectrum to DB: " << q;
   if ( type == "Extinction" )
   {
     
      QList< double > keys = extinction.keys();

      for ( int i = 0; i < keys.size(); i++ )
      {
         double wavelength = keys[ i ];
         q[ 4 ] = QString::number( wavelength, 'f', 1 );
         q[ 5 ] = QString::number( extinction[ wavelength ], 'e', 4 );
         qDebug() << "Buffer->extinction details: " << wavelength << " " << extinction[ wavelength ];
         db->statusQuery( q );
      }
   }

   else if ( type == "Refraction" )
   {
      QList< double > keys = refraction.keys();

      for ( int i = 0; i < keys.size(); i++ )
      {
         double wavelength = keys[ i ];
         q[ 4 ] = QString::number( wavelength, 'f', 1 );
         q[ 5 ] = QString::number( refraction[ wavelength ], 'e', 4 );
         db->statusQuery( q );
      }
   }

   else
   {
      QList< double > keys = fluorescence.keys();

      for ( int i = 0; i < keys.size(); i++ )
      {
         double wavelength = keys[ i ];
         q[ 4 ] = QString::number( wavelength, 'f', 1 );
         q[ 5 ] = QString::number( fluorescence[ wavelength ], 'e', 4 );
         db->statusQuery( q );
      }
   }
}

bool US_Buffer::writeToDisk( const QString& filename ) const
{
   QFile file( filename );
   if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text) )
   {
       qDebug() << "Cannot open file for writing: " << filename;
       return false;
   }

   QXmlStreamWriter xml( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD         ( "<!DOCTYPE US_Buffer>" );
   xml.writeStartElement( "BufferData" );
   xml.writeAttribute   ( "version", "1.0" );
   
   xml.writeStartElement( "buffer" );
   xml.writeAttribute( "id"         , "0" ); // buffers written to disk get bufferID "0"
   xml.writeAttribute( "guid"       , GUID     );
   xml.writeAttribute( "description", description );
   xml.writeAttribute( "ph"         , QString::number( pH       , 'f', 5 ) );
   xml.writeAttribute( "density"    , QString::number( density  , 'f', 6 ) );
   xml.writeAttribute( "viscosity"  , QString::number( viscosity, 'f', 5 ) );
   xml.writeAttribute( "compressibility", 
                                QString::number( compressibility, 'e', 4 ) );
   xml.writeAttribute( "manual"     , US_Util::bool_string( manual ) );

   for ( int i = 0; i < component.size(); i++ )
   {
      xml.writeStartElement( "component" );
      xml.writeAttribute( "id"           , componentIDs[ i ] );
      xml.writeAttribute( "concentration", 
            QString::number( concentration[ i ], 'f', 5 ) );
      xml.writeEndElement(); // component
   }

   xml.writeStartElement( "spectrum" );

   QList< double > keys = extinction.keys();

   for ( int i = 0; i < extinction.size(); i++ )
   {
      xml.writeStartElement( "extinction" );
      double wavelength = keys[ i ];
      double value      = extinction[ wavelength ];
      xml.writeAttribute( "wavelength", QString::number( wavelength, 'f', 1 ) );
      xml.writeAttribute( "value"     , QString::number( value,      'e', 4 ) );
      xml.writeEndElement(); // extinction
   }

   for ( int i = 0; i < refraction.size(); i++ )
   {
      xml.writeStartElement( "refraction" );
      double wavelength = keys[ i ];
      double value      = refraction[ wavelength ];
      xml.writeAttribute( "wavelength", QString::number( wavelength, 'f', 1 ) );
      xml.writeAttribute( "value"     , QString::number( value,      'e', 4 ) );
      xml.writeEndElement(); // refraction
   }

   for ( int i = 0; i < fluorescence.size(); i++ )
   {
      xml.writeStartElement( "fluorescence" );
      double wavelength = keys[ i ];
      double value      = fluorescence[ wavelength ];
      xml.writeAttribute( "wavelength", QString::number( wavelength, 'f', 1 ) );
      xml.writeAttribute( "value"     , QString::number( value,      'e', 4 ) );
      xml.writeEndElement(); // fluorescence
   }

   xml.writeEndElement(); // spectrum
   xml.writeEndElement(); // buffer
   xml.writeEndElement(); // US_Buffer
   xml.writeEndDocument();

   return true;
}

bool US_Buffer::readFromDB( US_DB2* db, const QString& bufID )
{
   QStringList q( "get_buffer_info" );
   q << bufID;   // bufferID from list widget entry

   qDebug() << "Reading from DB, BufferID: " << q;

   db->query( q );
   if ( db->lastErrno() != 0 ) return false;

   db->next();

   bufferID        = bufID;
   GUID            = db->value( 0 ).toString();
   description     = db->value( 1 ).toString();
   compressibility = db->value( 2 ).toString().toDouble();
   pH              = db->value( 3 ).toString().toDouble();
   viscosity       = db->value( 4 ).toString().toDouble();
   density         = db->value( 5 ).toString().toDouble();
   person          = db->value( 7 ).toString();
   int manx        = description.indexOf( "  [M]" );
   if ( manx > 0 )
   {
      manual          = true;
      description     = description.left( manx ).simplified();
   }
   else
      manual          = US_Util::bool_flag( db->value( 6 ).toString() );

   component    .clear();
   componentIDs .clear();
   concentration.clear();
   q                   .clear();

   q << "get_buffer_components" <<  bufferID;

   db->query( q );
   int status = db->lastErrno();
   if ( status != US_DB2::OK  &&  status != US_DB2::NOROWS )
   {
      qDebug() << "get_buffer_components error=" << status;
      return false;
   }

   while ( db->next() )
   {
      componentIDs  << db->value( 0 ).toString();
      concentration << db->value( 4 ).toString().toDouble();
   }

   for ( int i = 0; i < componentIDs.size(); i++ )
   {
      US_BufferComponent bc;
      bc.componentID = componentIDs[ i ];
      bc.getInfoFromDB( db );
       component << bc;
   }

   QString compType("Buffer");
   QString valType("molarExtinction");
   US_ExtProfile::fetch_eprofile( db, bufferID.toInt(), compType, valType, extinction );

   // // Get spectrum data
   // getSpectrum( db, "Refraction" );
   // getSpectrum( db, "Extinction" );
   // getSpectrum( db, "Fluorescence" );
   return true;
}

int US_Buffer::saveToDB( US_DB2* db, const QString private_buffer )
{
   int idBuffer = 0;
   QStringList q;
   q << "get_bufferID"
     << GUID;
   db->query( q );

   int     ncomp    = component.size();
   int     status   = db->lastErrno();
   QString descrip  = description;
   int     manx     = descrip.indexOf( "  [M]" );

   if ( manx > 0 )
      descrip          = descrip.left( manx ).simplified();
//qDebug() << "get_bufferID-stat" << status;

   if ( status != US_DB2::OK  &&  status != US_DB2::NOROWS )
   {
      qDebug() << "get_bufferID error=" << status;
      return -9;
   }

   else if ( status == US_DB2::NOROWS )
   {  // There is no buffer with the given GUID, so create a new one
      q.clear();
      q << "new_buffer"
        << GUID
        << descrip
        << QString::number( compressibility, 'e', 4 )
        << QString::number( pH             , 'f', 4 )
        << QString::number( density        , 'f', 6 )
        << QString::number( viscosity      , 'f', 5 )
        << US_Util::bool_string( manual )
        << private_buffer                              // Private
        << QString::number( US_Settings::us_inv_ID() );

      db->statusQuery( q );
//qDebug() << "new_buffer-stat" << db->lastErrno();

      if ( db->lastErrno() != US_DB2::OK )
      {
         qDebug() << "new_buffer error=" << db->lastErrno();
         return -1;
      }

      idBuffer    = db->lastInsertID();
     //qDebug() << "new_buffer-idBuffer" << idBuffer;
      
      qDebug() << "bufferID for new buffer: " << idBuffer;
      bufferID = QString::number(idBuffer);
      
   }

   else
   {  // The buffer exists, so update it
      db->next();            // Get the ID of the existing buffer record
      QString bufID   = db->value( 0 ).toString();
      idBuffer        = bufID.toInt();
      //bufferID        = QString::number(idBuffer);
      bufferID        = bufID;
      qDebug() << "BufferID in saveToDB(): " << bufferID;
//qDebug() << "old_buffer-idBuffer" << idBuffer;
      q.clear();
      q << "update_buffer"
        << bufID
        << descrip
        << QString::number( compressibility, 'e', 4 )
        << QString::number( pH             , 'f', 4 )
        << QString::number( density        , 'f', 6 )
        << QString::number( viscosity      , 'f', 5 )
        << US_Util::bool_string( manual )
        << private_buffer;                             // Private

      db->statusQuery( q );

//qDebug() << "update_stat" << db->lastErrno();
      if ( db->lastErrno() != US_DB2::OK )
      {
         qDebug() << "update_buffer error=" << db->lastErrno();
         return -2;
      }

      // Delete any components, so any given are a new list
      q.clear();
      q << "delete_buffer_components" << bufID;
      db->statusQuery( q );
      status    = db->lastErrno();
//qDebug() << "delete_buffer_components status=" << status << US_DB2::NOROWS;
      if ( status != US_DB2::OK   &&  status != US_DB2::NOROWS )
      {
         qDebug() << "delete_buffer_components error=" << db->lastErrno();
         return -3;
      }
   }

   qDebug() << "bufferID before adding components called: " << idBuffer;

   for ( int i = 0; i < ncomp; i++ )
   {
      q.clear();
      q << "add_buffer_component"
        << QString::number( idBuffer )
        << component[ i ].componentID
        << QString::number( concentration[ i ], 'f', 5 );
      db->statusQuery( q );
//qDebug() << "add_buffer_components-status=" << db->lastErrno();

      if ( db->lastErrno() != US_DB2::OK )
      {
         qDebug() << "add_buffer_component i,error=" << i << db->lastErrno();
         return -4;
      }
   }
   
   qDebug() << "bufferID before putSpectrum() called: " << idBuffer;

   QString compType("Buffer");
   QString valType("molarExtinction");
   qDebug() << "bufferID for extProfile: " << bufferID.toInt();
   
   if ( !extinction.isEmpty() and new_or_changed_spectrum )
     //if ( !extinction.isEmpty() )
   {
     if ( !replace_spectrum )
      {
	 qDebug() << "Creating Spectrum!!!";
         US_ExtProfile::create_eprofile( db, bufferID.toInt(), compType, valType, extinction);
      }
      else
      {
         qDebug() << "Updating Spectrum!!!";

         QMap<double, double> new_extinction = extinction;
         int profileID = US_ExtProfile::fetch_eprofile(  db, bufferID.toInt(), compType, valType, extinction);
         
         qDebug() << "Old Extinction keys: " << extinction.keys().count() << ", ProfileID: " << profileID;
         US_ExtProfile::update_eprofile( db, profileID, bufferID.toInt(), compType, valType, new_extinction);
         qDebug() << "New Extinction keys: " << new_extinction.keys().count() << ", ProfileID: " << profileID;
         
         replace_spectrum = false;
      }
      new_or_changed_spectrum = false; 
   }
   //putSpectrum( db, "Extinction" );
   //putSpectrum( db, "Refraction" );
   //putSpectrum( db, "Fluorescence" );

   // Also write to to disk
   bool    newFile;
   qDebug() << "Wrute to disk 0: " ;
   QString path     = US_Settings::dataDir() + "/buffers";
   qDebug() << "Wrute to disk 1: " ;
   QString filename = get_filename( path, GUID, newFile );
   qDebug() << "Wrute to disk 2: " ;
   writeToDisk( filename );
   qDebug() << "Wrute to disk 3: " ;
   
   qDebug() << "bufferID upon saveToDB() completion: " << idBuffer;

   return idBuffer;
}


bool US_Buffer::readFromDisk( const QString& filename ) 
{
   QFile file( filename );
   
   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
       qDebug() << "Cannot open file for reading: " << filename;
       return false;
   }

   QXmlStreamReader xml( &file );
   
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "buffer" )
            readBuffer( xml );
      }
   }

   file.close();
   return true;
}

QString US_Buffer::get_filename(
      const QString& path, const QString& guid, bool& newFile )
{
   return
      US_DataFiles::get_filename( path, guid, "B", "buffer", "guid", newFile );
}

void US_Buffer::readBuffer( QXmlStreamReader& xml )
{
   QXmlStreamAttributes a = xml.attributes();

   bufferID        = a.value( "id"          ).toString();
   GUID            = a.value( "guid"        ).toString();
   description     = a.value( "description" ).toString();
   compressibility = a.value( "compressibility" ).toString().toDouble();
   pH              = a.value( "ph"          ).toString().toDouble();
   density         = a.value( "density"     ).toString().toDouble();
   viscosity       = a.value( "viscosity"   ).toString().toDouble();
   manual          = US_Util::bool_flag( a.value( "manual" ).toString() );

   concentration.clear();
   componentIDs .clear();

   while ( ! xml.atEnd() )
   {
      if ( xml.isEndElement()  &&  xml.name() == "buffer" ) break;

      if ( xml.isStartElement()  &&  xml.name() == "component" )
      {
         QXmlStreamAttributes a = xml.attributes();
         concentration << a.value( "concentration" ).toString().toDouble();
         componentIDs  << a.value( "id"            ).toString();
      } 

      if ( xml.isStartElement()  &&  xml.name() == "spectrum" )
         readSpectrum( xml );

      xml.readNext();
   }
}

void US_Buffer::readSpectrum( QXmlStreamReader& xml )
{
   while ( ! xml.atEnd() )
   {
      if ( xml.isEndElement()  &&  xml.name() == "spectrum" ) break;

      if ( xml.isStartElement()  &&  xml.name() == "extinction" )
      {
         QXmlStreamAttributes a = xml.attributes();
         extinction[ a.value( "wavelength" ).toString().toDouble() ] =
            a.value( "value" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "refraction" )
      {
         QXmlStreamAttributes a = xml.attributes();
         refraction[ a.value( "wavelength" ).toString().toDouble() ] =
            a.value( "value" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "fluorescence" )
      {
         QXmlStreamAttributes a = xml.attributes();
         fluorescence[ a.value( "wavelength" ).toString().toDouble() ] =
            a.value( "value" ).toString().toDouble();
      }

      xml.readNext();
   }
}

void US_Buffer::compositeCoeffs( double* d_coeff, double* v_coeff )
{
   int    ncomp = component.size();    // Number of buffer components

   if ( ncomp == 0 )
      return;                          // If 0, nothing to do

   // Pre-remove water value from components beyond first
   d_coeff[ 0 ] = (double)( 1 - ncomp ) * DENS_20W;
   v_coeff[ 0 ] = (double)( 1 - ncomp ) * VISC_20W;

   for ( int ii = 1; ii < 6; ii++ )
   { // Initialize other coefficients to zero
      d_coeff[ ii ] = 0.0;
      v_coeff[ ii ] = 0.0;
   }
   
   double sumc1 = 0.0;
   double sumcr = 0.0;
   double sumc2 = 0.0;
   double sumc3 = 0.0;
   double sumc4 = 0.0;

   for ( int ii = 0; ii < ncomp; ii++ )
   { // Sum concentration and coefficient values
      double c1     = concentration[ ii ];  // c ^ 1
      double cr     = sqrt( c1 );           // c ^ 0.5
      double c2     = c1 * c1;              // c ^ 2
      double c3     = c2 * c1;              // c ^ 3
      double c4     = c3 * c1;              // c ^ 4
      sumc1        += c1;                   // Sum concentration terms
      sumcr        += cr;
      sumc2        += c2;
      sumc3        += c3;
      sumc4        += c4;

      // Accumulate weighted coefficient terms
      d_coeff[ 0 ] += component[ ii ].dens_coeff[ 0 ];
      d_coeff[ 1 ] += component[ ii ].dens_coeff[ 1 ] * cr;
      d_coeff[ 2 ] += component[ ii ].dens_coeff[ 2 ] * c1;
      d_coeff[ 3 ] += component[ ii ].dens_coeff[ 3 ] * c2;
      d_coeff[ 4 ] += component[ ii ].dens_coeff[ 4 ] * c3;
      d_coeff[ 5 ] += component[ ii ].dens_coeff[ 5 ] * c4;

      v_coeff[ 0 ] += component[ ii ].visc_coeff[ 0 ];
      v_coeff[ 1 ] += component[ ii ].visc_coeff[ 1 ] * cr;
      v_coeff[ 2 ] += component[ ii ].visc_coeff[ 2 ] * c1;
      v_coeff[ 3 ] += component[ ii ].visc_coeff[ 3 ] * c2;
      v_coeff[ 4 ] += component[ ii ].visc_coeff[ 4 ] * c3;
      v_coeff[ 5 ] += component[ ii ].visc_coeff[ 5 ] * c4;
   }

   // Normalize coefficients and multiply by standard powers
   v_coeff[ 1 ] *= ( 1.0e-3 / sumcr );
   v_coeff[ 2 ] *= ( 1.0e-2 / sumc1 );
   v_coeff[ 3 ] *= ( 1.0e-3 / sumc2 );
   v_coeff[ 4 ] *= ( 1.0e-4 / sumc3 );
   v_coeff[ 5 ] *= ( 1.0e-6 / sumc4 );

   d_coeff[ 1 ] *= ( 1.0e-3 / sumcr );
   d_coeff[ 2 ] *= ( 1.0e-2 / sumc1 );
   d_coeff[ 3 ] *= ( 1.0e-3 / sumc2 );
   d_coeff[ 4 ] *= ( 1.0e-4 / sumc3 );
   d_coeff[ 5 ] *= ( 1.0e-6 / sumc4 );
}

void US_Buffer::dumpBuffer( void ) const
{
   qDebug() << "person         " << person;
   qDebug() << "bufferID       " << bufferID;
   qDebug() << "GUID           " << GUID;
   qDebug() << "description    " << description;
   qDebug() << "compressibility" << compressibility;
   qDebug() << "pH             " << pH;
   qDebug() << "density        " << density;
   qDebug() << "viscosity      " << viscosity;
   qDebug() << "extinction     " << extinction;
   qDebug() << "refraction     " << refraction;
   qDebug() << "fluorescence   " << fluorescence;
   qDebug() << "manual         " << manual;
   qDebug() << "Components     " << component.size();
   for ( int i = 0; i < component.size(); i++ )
   {
      qDebug() << component[ i ].name << concentration[ i ] << componentIDs[ i ];
   }
};
