//! \file us_buffer.cpp
#include "us_buffer.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_db2.h"

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
        QMap< QString, US_BufferComponent >& componentList ) 
{
   componentList.clear();

   //QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
   QString home = US_Settings::appBaseDir();
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
        QXmlStreamReader&                    xml, 
        QMap< QString, US_BufferComponent >& componentList ) 
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
   //QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
   QString home = US_Settings::appBaseDir();
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
   
   QStringList keys = componentList.keys();

   for ( int i = 0; i < keys.size(); i++ )
   {
      QString key = keys[ i ];
      xml.writeStartElement( "component" );
      xml.writeAttribute( "id"   , componentList[ key ].componentID );
      xml.writeAttribute( "name" , componentList[ key ].name );
      xml.writeAttribute( "unit" , componentList[ key ].unit );
      xml.writeAttribute( "range", componentList[ key ].range );

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

//-------------  US_Buffer
US_Buffer::US_Buffer()
{
   compressibility = 0.0;
   pH              = WATER_PH;
   density         = DENS_20W;
   viscosity       = VISC_20W;
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

   db->query( q );

   while ( db->next() )
   {
      double lambda = db->value( 0 ).toDouble();
      double value  = db->value( 1 ).toDouble();

      if ( type == "Extinction" )
         extinction[ lambda ] = value;
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

   if ( type == "Extinction" )
   {
      QList< double > keys = extinction.keys();

      for ( int i = 0; i < keys.size(); i++ )
      {
         double wavelength = keys[ i ];
         q[ 4 ] = QString::number( wavelength, 'f', 1 );
         q[ 5 ] = QString::number( extinction[ wavelength ], 'e', 4 );
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
   xml.writeAttribute( "id"         , bufferID );
   xml.writeAttribute( "guid"       , GUID     );
   xml.writeAttribute( "description", description );
   xml.writeAttribute( "ph"         , QString::number( pH       , 'f', 5 ) );
   xml.writeAttribute( "density"    , QString::number( density  , 'f', 6 ) );
   xml.writeAttribute( "viscosity"  , QString::number( viscosity, 'f', 5 ) );
   xml.writeAttribute( "compressibility", 
      QString::number( compressibility, 'e', 4 ) );

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

   // Get spectrum data
   getSpectrum( db, "Refraction" );
   getSpectrum( db, "Extinction" );
   getSpectrum( db, "Fluorescence" );
   return true;
}

int US_Buffer::saveToDB( US_DB2* db, const QString private_buffer ) const
{
   QStringList q( "get_bufferID" );
   q << GUID;
   db->query( q );
   if ( db->lastErrno() != US_DB2::OK )
   {
      qDebug() << "get_bufferID error=" << db->lastErrno();
      return -9;
   }
   db->next();            // Determine if the buffer record already exists
   int bufferID    = 0;

   if ( db->lastErrno() != US_DB2::OK )
   {  // There is no such buffer, so create a new one
      q.clear();
      q << "new_buffer"
        << GUID
        << description
        << QString::number( compressibility, 'e', 4 )
        << QString::number( pH             , 'f', 4 )
        << QString::number( density        , 'f', 6 )
        << QString::number( viscosity      , 'f', 5 )
        << private_buffer  // Private
        << QString::number( US_Settings::us_inv_ID() );

      db->statusQuery( q );

      if ( db->lastErrno() != US_DB2::OK )
      {
         qDebug() << "new_buffer error=" << db->lastErrno();
         return -1;
      }

      bufferID    = db->lastInsertID();
   }

   else
   {  // The buffer exists, so update it
      QString bufID  = db->value( 0 ).toString();
      bufferID    = bufID.toInt();
      q.clear();
      q << "update_buffer"
        << bufID
        << description
        << QString::number( compressibility, 'e', 4 )
        << QString::number( pH             , 'f', 4 )
        << QString::number( density        , 'f', 6 )
        << QString::number( viscosity      , 'f', 5 )
        << private_buffer; // Private

      db->statusQuery( q );

      if ( db->lastErrno() != US_DB2::OK )
      {
         qDebug() << "update_buffer error=" << db->lastErrno();
         return -2;
      }

      // Delete any components, so any given are a new list
      q.clear();
      q << "delete_buffer_components" << bufID;
      db->statusQuery( q );
      if ( db->lastErrno() != US_DB2::OK )
      {
         qDebug() << "delete_buffer_components error=" << db->lastErrno();
         return -3;
      }
   }

   for ( int i = 0; i < component.size(); i++ )
   {
      q.clear();
      q << "add_buffer_component"
        << QString::number( bufferID )
        << component[ i ].componentID
        << QString::number( concentration[ i ], 'f', 5 );
      db->statusQuery( q );

      if ( db->lastErrno() != US_DB2::OK )
      {
         qDebug() << "add_buffer_component i,error=" << i << db->lastErrno();
         return -4;
      }
   }

   putSpectrum( db, "Extinction" );
   putSpectrum( db, "Refraction" );
   putSpectrum( db, "Fluorescence" );

   // Also write to to disk
   bool    newFile;
   QString path     = US_Settings::dataDir() + "/buffers";
   QString filename = get_filename( path, GUID, newFile );
   writeToDisk( filename );

   return bufferID;
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
   QDir        f( path );
   QStringList filter( "B???????.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );
   QString     filename;
   newFile = true;

   for ( int i = 0; i < f_names.size(); i++ )
   {
      QFile b_file( path + "/" + f_names[ i ] );

      if ( ! b_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &b_file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "buffer" )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "guid" ).toString() == guid )
               {
                  newFile  = false;
                  filename = path + "/" + f_names[ i ];
               }

               break;
            }
         }
      }

      b_file.close();
      if ( ! newFile ) return filename;
   }

   // If we get here, generate a new filename
   int number = ( f_names.size() > 0 ) ? f_names.last().mid( 1, 7 ).toInt() : 0;

   return path + "/B" + QString().sprintf( "%07i", number + 1 ) + ".xml";
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
   qDebug() << "Components     " << component.size();
   for ( int i = 0; i < component.size(); i++ )
   {
      qDebug() << component[ i ].name << concentration[ i ] << componentIDs[ i ];
   }
};
