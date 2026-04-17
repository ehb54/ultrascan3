//! \file us_noise.cpp
#include "us_noise.h"
#include "us_constants.h"
#include "us_settings.h"
#include "us_util.h"

#define isNan(a) (a!=a)

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
   if ( minradius       != n.minradius       ) return false;
   if ( maxradius       != n.maxradius       ) return false;
   if ( values.size()   != n.values.size()   ) return false;

   for ( int ii = 0; ii < values.size(); ii++ )
      if ( values[ ii ] != n.values[ ii ] ) return false;

   return true;
}

// load noise from db or local disk
int US_Noise::load( bool db_access, const QString& guid, IUS_DB2* db )
{
   if ( db_access ) return load_db  ( guid, db );
   else             return load_disk( guid );
}

// load noise from db
int US_Noise::load( const QString& id, IUS_DB2* db )
{
   QStringList q;

   q << "get_noise_info" << id;
   db->query( q );
   
   if ( db->lastErrno() != IUS_DB2::OK ) return db->lastErrno();

   db->next();
   QByteArray contents = db->value( 5 ).toString().toLatin1();

   QXmlStreamReader     xml( contents );

   return load_stream( xml );
}

// load noise from local disk
int US_Noise::load( const QString& filename )
{
   QFile file( filename );

   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      qDebug() << "Cannot open file for reading: " << filename;
      return -1;
   }

   QXmlStreamReader     xml( &file );

   return load_stream( xml );
}

// load noise from an XML stream
int US_Noise::load_stream( QXmlStreamReader& xml )
{
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
            values.clear();
         }

         else if ( xml.name() == "d" )
         {
            a = xml.attributes();

            values << a.value( "v" ).toString().toDouble();
         }
      }
   }

   count       = values.size();
//qDebug() << "NOI: ldTemp: count" << count;
   if ( US_Settings::us_debug() > 2 )
      debug();

   return IUS_DB2::OK;
}

// write noise to db or local disk
int US_Noise::write( bool db_access, const QString& filename, IUS_DB2* db )
{
   if ( db_access ) return write( db );
   else             return write( filename );
}

// write noise to db
int US_Noise::write( IUS_DB2* db )
{
      // Create the noise xml file in a string
      QByteArray       temp_contents;
      QXmlStreamWriter xml( &temp_contents );
      write_stream( xml );

      QByteArray     contents;
      QStringList    q;

      // Get type and model IDD
      QString typen   = ( type == TI ) ? "ti_noise" : "ri_noise";

      q << "get_modelID" << modelGUID;
      db->query( q );
      db->next();
      QString modelID = db->value( 0 ).toString();

      // Encapsulate the XML contents
      db->mysqlEscapeString( contents, temp_contents, temp_contents.size() );

      // Determine the edit ID
      QString invID   = QString::number( US_Settings::us_inv_ID() );
      QString editID  = "0";
      q.clear();
      q << "get_model_desc" << invID;
      db->query( q );
      while( db->next() )
      {
         if ( db->value( 0 ).toString() == modelID )
         {
            editID         = db->value( 6 ).toString();
            break;
         }
      }

      // Generate a noise guid if necessary
      // The guid may be valid from a disk read, but is not in the DB
      if ( noiseGUID.size() != 36 ) noiseGUID = US_Util::new_guid();

      q.clear();
      q << "get_noiseID" << noiseGUID;
      
      db->query( q );
      
      q.clear();
     
      if ( db->lastErrno() != IUS_DB2::OK )
      {
         q << "new_noise" << noiseGUID << editID << modelID << modelGUID
           << typen << description << contents;
         message = QObject::tr( "created" );
//qDebug() << "get_noiseID stat GUID" << db->lastErrno() << noiseGUID;
      }
      else
      {
         db->next();
         QString noiseID = db->value( 0 ).toString();
         q << "update_noise" << noiseID << noiseGUID << editID << modelID
           << modelGUID << typen << description << contents;
         message = QObject::tr( "updated" );
//qDebug() << "get_noiseID ID GUID" << noiseID << noiseGUID;
      }

      return db->statusQuery( q );
//int updstat = db->statusQuery( q );
//qDebug() << "new/update_noise stat" << updstat;
//return updstat;
}

// write noise to local disk
int US_Noise::write( const QString& filename )
{
   QFile file( filename );

   if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text) )
      return IUS_DB2::DBERROR;

   QXmlStreamWriter xml( &file );
   write_stream( xml );
   file.close();

   return IUS_DB2::OK;
}

// apply noise to EditedData by add/subtract noise values from readings
int US_Noise::apply_to_data( US_DataIO::EditedData& editdata, bool remove )
{
   int    rCount = editdata.pointCount();             // readings count
   int    sCount = editdata.scanCount();              // scan count
   int    ii;
   int    jj;
   double vnoise;
   double applyf = remove ? -1.0 : 1.0;               // apply factor
   count         = values.size();

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
            editdata.scanData[ ii ].rvalues[ jj ] += vnoise;
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
            editdata.scanData[ ii ].rvalues[ jj ] += vnoise;
         }
      }
   }

   return 0;
}

// remove/add noise vector from/to edited data
int US_Noise::apply_noise( US_DataIO::EditedData& editdata,
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
   int error = IUS_DB2::DBERROR;  // Error by default

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
int US_Noise::load_db( const QString& guid, IUS_DB2* db )
{
   QStringList q;

   q << "get_noiseID" << guid;
   db->query( q );

   if ( db->lastErrno() != IUS_DB2::OK ) return db->lastErrno();
   
   db->next();
   QString id = db->value( 0 ).toString();
   return load( id, db );
}

// write noise to an XML stream
void US_Noise::write_stream( QXmlStreamWriter& xml )
{
   QString typen( "ti" );

   if ( type == RI )
      typen = QString( "ri" );

   //file.open();// QIODevice::WriteOnly | QIODevice::Text );
   //QXmlStreamWriter xml( &file );
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
//qDebug() << " wr_stream count" << values.size();
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
   qDebug() << "values size" << values.size();

   for ( int ii = 0; ii < values.size(); ii++ )
   {
      qDebug() << "  v[" << ii << "]" << values[ ii ];
   }
}

// sum second noise into this vector
bool US_Noise::sum_noise( US_Noise noise2, bool always_sum )
{
   bool    ok    = true;
   bool    match = true;

   if ( type != noise2.type )
      ok    = false;            // cannot sum if noises are different types

   else
   {  // determine if they match in count and any radius range
      double difflo = qAbs( minradius - noise2.minradius );
      double diffhi = qAbs( maxradius - noise2.maxradius );
      if ( count != noise2.count  ||
         ( type == TI  &&  ( difflo > 0.0001  ||  diffhi > 0.0001 ) ) )
      {
         match   = false;
qDebug() << "SM-NOI: difflo diffhi" << difflo << diffhi;
      }
   }

   if ( match )
   {  // if they match, simply sum vector values;
      for ( int ii = 0; ii < values.size(); ii++ )
      {
         values[ ii ] += noise2.values[ ii ];
if(isNan(values[ii]))
 qDebug() << "SM-NOI: ii" << ii << "noi o i" << values[ii] << noise2.values[ii];
      }
   }

   else if ( always_sum )
   {  // otherwise, we may need to interpolate or only sum a portion
      
      if ( type == RI )
      {  // for radially-invariant, just sum for minimum count
         int    mcount = qMin( count, noise2.count );

         for ( int ii = 0; ii < mcount; ii++ )
            values[ ii ] += noise2.values[ ii ];

      }

      else
      {  // for time-invariant,  interpolate by radius location
qDebug() << "SM-NOI: *NTRP* count count2" << count << noise2.count
 << "type" << type << "mnmx r" << minradius << noise2.minradius
 << maxradius << noise2.maxradius;
         double radval = minradius;
         double radinc = ( maxradius - minradius ) / (double)( count - 1 );
         double radlo2 = noise2.minradius;
         int    limk2  = noise2.count - 1;
         double radin2 = ( noise2.maxradius - radlo2 ) / (double)( limk2 );

         for ( int ii = 0; ii < count; ii++ )
         {
            // calculate index of noise1 radius in noise2 range
            double xrad2  = ( radval - radlo2 ) / radin2;
            int    j1     = (int)xrad2;
            int    j2     = j1 + 1;
            // get interpolation factors and noise2 values around index
            double fac1   = (double)j2 - xrad2;
            double fac2   = 1.0 - fac1;
            double val1   = ( j1 < 0     ) ? noise2.values[ 0 ] :
                          ( ( j1 > limk2 ) ? noise2.values[ limk2 ] :
                                             noise2.values[ j1    ] );
            double val2   = ( j2 < 0     ) ? noise2.values[ 0 ] :
                          ( ( j2 > limk2 ) ? noise2.values[ limk2 ] :
                                             noise2.values[ j2    ] );
            // sum in an interpolated value and bump noise1 radius
            values[ ii ] += ( val1 * fac1 + val2 * fac2 );
            radval       += radinc;
         }
      }
   }

   else  // no summing done
      ok        = false;

   return ok;
}

// sum second noise into an original noise
bool US_Noise::sum_noises( US_Noise& noise1, US_Noise noise2, bool always_sum )
{

   return noise1.sum_noise( noise2, always_sum );
}

