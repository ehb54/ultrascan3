//! \file us_noise.cpp

#include "us_noise.h"
#include "us_constants.h"
#include "us_settings.h"
#include "us_util.h"

#include <uuid/uuid.h>

// constructor - initialize empty
US_Noise::US_Noise()
{
   type            = TI;
   description     = "New Noise";
   noiseGUID       = "";
   modelGUID       = "";
   minradius       = 0.0;
   maxradius       = 0.0;
   count           = 0;
   values.clear();
}

// equality operator
bool US_Noise::operator== ( const US_Noise& n ) const
{
   if ( type            != n.type            ) return false;
   if ( description     != n.description     ) return false;
   if ( noiseGUID       != n.noiseGUID       ) return false;
   if ( modelGUID       != n.modelGUID       ) return false;
   if ( count           != n.count           ) return false;
   if ( minradius       != n.minradius       ) return false;
   if ( maxradius       != n.maxradius       ) return false;
   if ( values.size()   != n.values.size()   ) return false;

   for ( int ii = 0; ii < values.size(); ii++ )
      if ( values[ ii ] != n.values[ ii ] ) return false;

   return true;
}

// load noise from db or local disk
int US_Noise::load( bool db_access, const QString& guid, US_DB2* db )
{
   if ( db_access ) return load_db  ( guid, db );
   else             return load_disk( guid );
}

// load noise from db
int US_Noise::load( const QString& id, US_DB2* db )
{
   QStringList q;

   q << "get_noise_info" << id;
   db->query( q );
   
   if ( db->lastErrno() != US_DB2::OK ) return db->lastErrno();

   db->next();
   QByteArray contents = db->value( 2 ).toString().toAscii();

   // Write the model file to a temporary file
   QTemporaryFile temporary;
   temporary.open();
   temporary.write( contents );
   temporary.close();

   QString file = temporary.fileName();
   return load( file );
}

// load noise from local disk
int US_Noise::load( const QString& filename )
{
   QFile file( filename );

   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      qDebug() << "Cannot open file for reading: " << filename;
      return false;
   }

   QXmlStreamReader     xml( &file );
   QXmlStreamAttributes a;
   bool                 read_next = true;
   QString              typen     = "ti";

   while ( ! xml.atEnd() )
   {
      if ( read_next ) xml.readNext();
      read_next = true;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "noise" )
         {
            a = xml.attributes();

            typen       = a.value( "type"        ).toString();
            type        = typen.compare( "ti" ) == 0 ? TI : RI;

            description = a.value( "description" ).toString();
            noiseGUID   = a.value( "noiseGUID"   ).toString();
            modelGUID   = a.value( "modelGUID"   ).toString();
            minradius   = a.value( "minradius"   ).toString().toDouble();
            maxradius   = a.value( "maxradius"   ).toString().toDouble();
            count       = a.value( "count"       ).toString().toInt();
            values.clear();
         }

         else if ( xml.name() == "d" )
         {
            a = xml.attributes();

            values << a.value( "v" ).toString().toDouble();
         }
      }
   }
//debug();
   return US_DB2::OK;
}

// write noise to db or local disk
int US_Noise::write( bool db_access, const QString& filename, US_DB2* db )
{
   if ( db_access ) return write( db );
   else             return write( filename );
}

// write noise to db
int US_Noise::write( US_DB2* db )
{
      // Create the noise xml file in a string
      QTemporaryFile temporary;
      write_temp( temporary );
      temporary.open();
      QByteArray contents = temporary.readAll();

      QStringList q;

      // Generate a guid if necessary
      // The guid may be valid from a disk read, but is not in the DB
      if ( noiseGUID.size() != 36 ) noiseGUID = US_Util::new_guid();

      q << "get_noiseID" << noiseGUID;
      
      db->query( q );
      
      q.clear();
     
      if ( db->lastErrno() != US_DB2::OK )
      {
         q << "new_noise" << noiseGUID << description << contents << modelGUID;
         message = QObject::tr( "created" );
      }
      else
      {
         db->next();
         QString id = db->value( 0 ).toString();
         q << "update_noise" << id << description << contents << modelGUID;
         message = QObject::tr( "updated" );
      }

      return db->statusQuery( q );
}

// write noise to local disk
int US_Noise::write( const QString& filename )
{
   QTemporaryFile temporary;
   write_temp( temporary );

   temporary.open();   // Open for reading

   QFile file( filename );

   if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text) )
      return US_DB2::ERROR;
   
   file.write( temporary.readAll() );
   file.close();
   temporary.close();

   return US_DB2::OK;
}

// apply noise to EditedData by add/subtract noise values from readings
int US_Noise::apply_to_data( US_DataIO2::EditedData& editdata, bool remove )
{
   int    rCount = editdata.scanData[ 0 ].readings.size(); // readings count
   int    sCount = editdata.scanData.size();               // scan count
   int    ii;
   int    jj;
   double vnoise;
   double applyf = remove ? -1.0 : 1.0;                    // apply factor

   if ( count == 0 )
   {
      message = QObject::tr( "No noise applied, since its count is zero" );
      return 1;
   }

   if ( type == TI )
   {  // Time-invariant:  subtract same value all scans at reading position

      if ( count != rCount )
      {
         message = QObject::tr( "Noise count does not equal data readings count" );
         return -1;
      }

      for ( jj = 0; jj < rCount; jj++ )
      {  // get constant noise value for each reading and apply to scans
         vnoise = ( values[ jj ] * applyf );

         for ( ii = 0; ii < sCount; ii++ )
         {  // apply to all scans at reading position
            editdata.scanData[ ii ].readings[ jj ].value += vnoise;
         }
      }
   }

   else
   {  // Radially-invariant:  subtract same value all readings at scan position

      if ( count != sCount )
      {
         message = QObject::tr( "Noise count does not equal data scan count" );
         return -2;
      }

      for ( ii = 0; ii < sCount; ii++ )
      {  // get constant noise value for each scan and apply to readings
         vnoise = ( values[ ii ] * applyf );

         for ( jj = 0; jj < rCount; jj++ )
         {  // apply to all readings at scan position
            editdata.scanData[ ii ].readings[ jj ].value += vnoise;
         }
      }
   }

   return 0;
}

// remove/add noise vector from/to edited data
int US_Noise::apply_noise( US_DataIO2::EditedData& editdata,
      US_Noise* noise, bool remove )
{
   if ( noise != 0 )
   {
      return noise->apply_to_data( editdata, remove );
   }

   else
   {
      return 1;
   }
}

// test noise file path and create directory if need be
bool US_Noise::noise_path( QString& path )
{
   QDir dir;
   path = US_Settings::dataDir() + "/noises";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         return false;
      }
   }

   return true;
}

// load noise that matches guid from disk
int US_Noise::load_disk( const QString& guid )
{
   int error = US_DB2::ERROR;  // Error by default

   QString path;

   if ( ! noise_path( path ) )
   {
      message = QObject::tr ( "Could not create noises directory" );
      return error;
   }

   QDir        f( path );
   QStringList filter( "N*.xml" );
   QStringList names = f.entryList( filter, QDir::Files, QDir::Name );
   QString     filename;
   bool        found = false;

   for ( int ii = 0; ii < names.size(); ii++ )
   {
      filename = path + "/" + names[ ii ];
      QFile file( filename );

      if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "noise" )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "noiseGUID" ).toString() == guid ) found = true;
               break;
            }
         }
      }

      file.close();

      if ( found ) return load( filename );
   }

   qDebug() << "Could not find noise GUID";
   message =  QObject::tr ( "Could not find noise guid" );
   return error;
}

// load noise that matches guid from db
int US_Noise::load_db( const QString& guid, US_DB2* db )
{
   QStringList q;

   q << "get_noiseID" << guid;
   db->query( q );

   if ( db->lastErrno() != US_DB2::OK ) return db->lastErrno();
   
   db->next();
   QString id = db->value( 0 ).toString();
   return load( id, db );
}

// write noise to temporary file
void US_Noise::write_temp( QTemporaryFile& file )
{
   QString typen( "ti" );

   if ( type == RI )
      typen = QString( "ri" );

   file.open();// QIODevice::WriteOnly | QIODevice::Text );
   QXmlStreamWriter xml( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD         ( "<!DOCTYPE US_Noise>" );
   xml.writeStartElement( "NoiseData" );
   xml.writeAttribute   ( "version", "1.0" );

   xml.writeStartElement( "noise" );
   xml.writeAttribute   ( "type",        typen       );
   xml.writeAttribute   ( "description", description );
   xml.writeAttribute   ( "modelGUID",   modelGUID    );
   xml.writeAttribute   ( "noiseGUID",   noiseGUID   );

   if ( type == TI )
   {
      xml.writeAttribute   ( "minradius",   QString::number( minradius ) );
      xml.writeAttribute   ( "maxradius",   QString::number( maxradius ) );
   }

   xml.writeAttribute   ( "count",       QString::number( count     ) );

   // Write values
   for ( int ii = 0; ii < values.size(); ii++ )
   {
      xml.writeStartElement( "d" );

      xml.writeAttribute( "v", QString::number( values[ ii ] ) );

      xml.writeEndElement(); // data value
   }

   xml.writeEndElement(); // noise
   xml.writeEndElement(); // NoiseData
   xml.writeEndDocument();
   file.close();
}

// output debug prints
void US_Noise::debug( void )
{
   QString typen( "no" );
   typen = ( type == RI ) ? QString( "RI" ) : typen;
   typen = ( type == TI ) ? QString( "TI" ) : typen;

   qDebug() << "type" << (int)type << typen;
   qDebug() << "desc" << description;
   qDebug() << "model guid" << modelGUID;
   qDebug() << "noise guid" << noiseGUID;;
   qDebug() << "count" << count;
   qDebug() << "values size" << values.size();

   for ( int ii = 0; ii < values.size(); ii++ )
   {
      qDebug() << "  v[" << ii << "]" << values[ ii ];
   }
}

