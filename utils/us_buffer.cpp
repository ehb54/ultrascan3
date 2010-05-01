//! \file us_buffer.cpp
#include "us_buffer.h"
#include "us_settings.h"
#include "us_db2.h"

void US_BufferComponent::getAllFromDB( const QString& masterPW, 
         QList< US_BufferComponent >& componentList )
{
   US_DB2 db( masterPW );
   
   if ( db.lastErrno() != US_DB2::OK )
   {
      qDebug() << "Database Error" 
               << "US_BufferComponent ::connectDB: Could not open DB\n"
               << db.lastError();
      return;
   }

   US_DB2 db2( masterPW );
   
   if ( db2.lastErrno() != US_DB2::OK )
   {
      qDebug() << "Database Error" 
               << "US_BufferComponent ::connectDB: Could not open DB\n"
               << db2.lastError();
      return;
   }

   QStringList q( "get_buffer_component_desc" );
   db.query( q );

   while ( db.next() )
   {
      US_BufferComponent c;
      c.componentID = db.value( 0 ).toString();
      c.name        = db.value( 1 ).toString();
      c.getInfoFromDB( db2 );
      componentList << c;
   }
}

void US_BufferComponent::getInfoFromDB( US_DB2& db ) 
{
   QStringList q( "get_buffer_component_info" );
   q << componentID;

   db.query( q );
   db.next();

   unit = db.value( 0 ).toString();
   name = db.value( 1 ).toString();
   QString viscosity       = db.value( 2 ).toString();
   QString density         = db.value( 3 ).toString();

   QStringList sl = viscosity.split( " " );

   for ( int i = 0; i < 6 ; i++ )
      visc_coeff[ i ] = sl[ i ].toDouble();

   sl = density.split( " " );

   for ( int i = 0; i < 6 ; i++ )
      dens_coeff[ i ] = sl[ i ].toDouble();

   sl    = sl.mid( 6 );  // Remove coefficients
   range = sl.join( " " );
}

void US_BufferComponent::getAllFromHD( 
        QList< US_BufferComponent >& componentList ) 
{
   componentList.clear();

   QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
   QFile   file( home + "/etc/bufferComponents.xml" );

   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      // Fail quietly
      //qDebug() << "Cannot open file " << home + "/etc/bufferComponents.xml";
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
        QXmlStreamReader&            xml, 
        QList< US_BufferComponent >& componentList ) 
{
   US_BufferComponent bc;

   QXmlStreamAttributes a = xml.attributes();
   bc.componentID = a.value( "id"    ).toString();
   bc.name        = a.value( "name"  ).toString();
   bc.unit        = a.value( "unit"  ).toString();
   bc.range       = a.value( "range" ).toString();

   while ( ! xml.atEnd() )
   {

      if ( xml.isEndElement()  &&  xml.name() == "component" ) 
      {
         componentList << bc;
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
        const QList< US_BufferComponent >& componentList ) 
{
   QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
   QFile   file( home + "/etc/bufferComponents.xml" );
   
   if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text) )
   {
       qDebug() << "Cannot open file " << home + "/etc/bufferComponents.xml";
       return;
   }

   QXmlStreamWriter xml( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD         ( "<!DOCTYPE US_BufferComponents>" );
   xml.writeStartElement( "BufferComponents" );
   xml.writeAttribute   ( "version", "1.0" );
   
   for ( int i = 0; i < componentList.size(); i++ )
   {
      xml.writeStartElement( "component" );
      xml.writeAttribute( "id"   , componentList[ i ].componentID );
      xml.writeAttribute( "name" , componentList[ i ].name );
      xml.writeAttribute( "unit" , componentList[ i ].unit );
      xml.writeAttribute( "range", componentList[ i ].range );

      QString factor;
      QString value;

      xml.writeStartElement( "densityCoefficients" );
      for ( int j = 0; j < 6; j++ )
      {
         factor.sprintf( "c%i", j );
         value = QString::number( componentList[ i ].dens_coeff[ j ], 'f', 5 );
         xml.writeAttribute( factor, value );
      }

      xml.writeEndElement(); // densityCoefficients

      xml.writeStartElement( "viscosityCoefficients" );
      for ( int j = 0; j < 6; j++ )
      {
         factor.sprintf( "c%i", j );
         value = QString::number( componentList[ i ].visc_coeff[ j ], 'f', 5 );
         xml.writeAttribute( factor, value );
      }

      xml.writeEndElement(); // viscosityCoefficients
      xml.writeEndElement(); // component
   }

   xml.writeEndElement(); // US_BufferComponents
   xml.writeEndDocument();

   file.close();
}

//-------------
US_Buffer::US_Buffer()
{
   personID        = -1;
   compressibility = 0.0;
   pH              = 7.0;
   density         = 0.0;
   viscosity       = 0.0;
   person       .clear();
   bufferID     .clear();
   GUID         .clear();
   description  .clear();
   extinction   .clear();
   refraction   .clear();
   fluorescence .clear();
   component    .clear();
   concentration.clear();
}

void US_Buffer::getInfoFromDB( const QString& masterPW )
{
   US_DB2 db( masterPW );
   
   if ( db.lastErrno() != US_DB2::OK )
   {
      qDebug() << "Database Error" 
               << "US_Buffer::connectDB: Could not open DB\n"
               << db.lastError();
      return;
   }

   QStringList q( "get_buffer_desc" );
   db.query( q );
   // unfinished...
}

void US_Buffer::getSpectrum( US_DB2& db, const QString& type ) 
{
   QStringList q;
   q << "get_spectrum" << bufferID << "Buffer" << type;

   db.query( q );

   while ( db.next() )
   {
      double lambda = db.value( 0 ).toDouble();
      double value  = db.value( 1 ).toDouble();

      if ( type == "Extinction" )
         extinction[ lambda ] = value;
      else if ( type == "Refraction" )
         refraction[ lambda ] = value;
      else
         fluorescence[ lambda ] = value;
   }
}

void US_Buffer::putSpectrum( US_DB2& db, const QString& type ) 
{
   QStringList q;
   q << "new_spectrum" << bufferID << "Buffer" << type << "" << "";

   if ( type == "Extinction" )
   {
      QList< double > keys = extinction.keys();

      for ( int i = 0; i < keys.size(); i++ )
      {
         double wavelength = keys[ i ];
         q[ 4 ] = QString::number( wavelength, 'f', 1 );
         q[ 5 ] = QString::number( extinction[ wavelength ], 'e', 4 );
         db.statusQuery( q );
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
         db.statusQuery( q );
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
         db.statusQuery( q );
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
   xml.writeAttribute( "person_id"  , QString::number( personID ) );
   xml.writeAttribute( "id"         , bufferID );
   xml.writeAttribute( "guid"       , GUID     );
   xml.writeAttribute( "description", description );
   xml.writeAttribute( "ph"         , QString::number( pH       , 'f', 5 ) );
   xml.writeAttribute( "density"    , QString::number( density  , 'f', 5 ) );
   xml.writeAttribute( "viscosity"  , QString::number( viscosity, 'f', 5 ) );
   xml.writeAttribute( "compressibility", 
      QString::number( compressibility, 'e', 5 ) );

   for ( int i = 0; i < component.size(); i++ )
   {
      xml.writeStartElement( "component" );
      xml.writeAttribute( "id"           , component[ i ].componentID );
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

void US_Buffer::readBuffer( QXmlStreamReader& xml )
{
   QXmlStreamAttributes a = xml.attributes();

   personID        = a.value( "person_id"   ).toString().toInt();
   bufferID        = a.value( "id"          ).toString();
   GUID            = a.value( "guid"        ).toString();
   description     = a.value( "description" ).toString();
   compressibility = a.value( "compressibility" ).toString().toDouble();
   pH              = a.value( "ph"          ).toString().toDouble();
   density         = a.value( "density"     ).toString().toDouble();
   viscosity       = a.value( "viscosity"   ).toString().toDouble();

   component    .clear();
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
