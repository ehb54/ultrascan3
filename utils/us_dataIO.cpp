//! \file us_dataIO.cpp
#include <QDomDocument>

#include "us_dataIO.h"
#include "us_crc.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_util.h"
#include "us_settings.h"

// Static member definition
const uint US_DataIO::format_version;

// Return the count of readings points
int US_DataIO::RawData::pointCount( )
{
   return xvalues.size();
}

// Return the count of scans
int US_DataIO::RawData::scanCount( )
{
   return scanData.size();
}

// Return the index of an X (radius or wavelength) value
int US_DataIO::RawData::xindex( double xvalue )
{
   return index( xvalues, xvalue );
}

// Get the X value (radius) at a given index
double US_DataIO::RawData::radius( int radx )
{
   return xvalues[ radx ];
}

// Get the X value (wavelength) at a given index
double US_DataIO::RawData::scWavelength( int wavx )
{
   return xvalues[ wavx ];
}

// Get the readings value at given scan,radius indecies
double US_DataIO::RawData::value( int scnx, int radx )
{
   return scanData[ scnx ].rvalues[ radx ];
}
// Get the readings value at given scan,radius indecies
double US_DataIO::RawData::reading( int scnx, int radx )
{
   return scanData[ scnx ].rvalues[ radx ];
}

// Set the readings value at given scan,radius indecies
bool US_DataIO::RawData::setValue( int scnx, int radx, double value )
{
   if ( scnx < 0  ||  scnx >= scanData.size()  ||
        radx < 0  ||  radx >= scanData[ scnx ].rvalues.size() )
      return false;

   scanData[ scnx ].rvalues[ radx ] = value;
   return true;
}

// Get the standard deviation value at given scan,radius indecies
double US_DataIO::RawData::std_dev( int scnx, int radx )
{
   US_DataIO::Scan* scn = &scanData[ scnx ];
   bool use_stddev = ( scn->nz_stddev  &&  scn->stddevs.size() > radx );
   return ( use_stddev ? scn->stddevs[ radx ] : 0.0 );
}

// Calculate the average temperature value across scans
double US_DataIO::RawData::average_temperature() const
{
   int    scansize = scanData.size();
   double sum      = 0.0;

   for ( int ii = 0; ii < scansize; ii++ )
   {
      sum += scanData[ ii ].temperature;
   }

   return ( sum / (double)scansize );
}

// Calculate the spread of temperature values across scans
double US_DataIO::RawData::temperature_spread() const
{
   int    scansize = scanData.size();
   double smin     = 99.0;
   double smax     = 0.0;

   for ( int ii = 0; ii < scansize; ii++ )
   {
      double temp = scanData[ ii ].temperature;
      smin        = qMin( smin, temp );
      smax        = qMax( smax, temp );
   }

   return qAbs( smax - smin );
}

// Return the count of readings points
int US_DataIO::EditedData::pointCount( )
{
   return xvalues.size();
}

// Return the count of scans
int US_DataIO::EditedData::scanCount( )
{
   return scanData.size();
}

// Return the index of an X (radius or wavelength) value
int US_DataIO::EditedData::xindex( double xvalue )
{
   return index( xvalues, xvalue );
}

// Get the X value (radius) at a given index
double US_DataIO::EditedData::radius( int radx )
{
   return xvalues[ radx ];
}

// Get the X value (wavelength) at a given index
double US_DataIO::EditedData::scWavelength( int wavx )
{
   return xvalues[ wavx ];
}

// Get the readings value at given scan,radius indecies
double US_DataIO::EditedData::value( int scnx, int radx )
{
   return scanData[ scnx ].rvalues[ radx ];
}

// Get the readings value at given scan,radius indecies
double US_DataIO::EditedData::reading( int scnx, int radx )
{
   return scanData[ scnx ].rvalues[ radx ];
}

// Set the readings value at given scan,radius indecies
bool US_DataIO::EditedData::setValue( int scnx, int radx, double value )
{
   if ( scnx < 0  ||  scnx >= scanData.size()  ||
        radx < 0  ||  radx >= scanData[ scnx ].rvalues.size() )
      return false;

   scanData[ scnx ].rvalues[ radx ] = value;
   return true;
}

// Get the standard deviation value at given scan,radius indecies
double US_DataIO::EditedData::std_dev( int scnx, int radx )
{
   US_DataIO::Scan* scn = &scanData[ scnx ];
   bool use_stddev = ( scn->nz_stddev  &&  scn->stddevs.size() > 0 );
   return ( use_stddev ? scn->stddevs[ radx ] : 0.0 );
}

// Calculate the average temperature value across scans
double US_DataIO::EditedData::average_temperature() const
{
   int    scansize = scanData.size();
   double sum      = 0.0;

   for ( int ii = 0; ii < scansize; ii++ )
   {
      sum += scanData[ ii ].temperature;
   }

   return ( sum / (double)scansize );
}

// Calculate the spread of temperature values across scans
double US_DataIO::EditedData::temperature_spread() const
{
   int    scansize = scanData.size();
   double smin     = 99.0;
   double smax     = 0.0;

   for ( int ii = 0; ii < scansize; ii++ )
   {
      double temp = scanData[ ii ].temperature;
      smin        = qMin( smin, temp );
      smax        = qMax( smax, temp );
   }

   return qAbs( smax - smin );
}

bool US_DataIO::readLegacyFile( const QString&  file, 
                                BeckmanRawScan& data )
{
   // Open the file for reading
   QFile ff( file );
   if ( ! ff.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
   QTextStream ts( &ff );
#if 1
   double ss_reso      = 100.0;
   // If debug_text so directs, change set_speed_resolution
   QStringList dbgtxt = US_Settings::debug_text();
   for ( int ii = 0; ii < dbgtxt.count(); ii++ )
   {  // If debug text modifies ss_reso, apply it
      if ( dbgtxt[ ii ].startsWith( "SetSpeedReso" ) )
         ss_reso       = QString( dbgtxt[ ii ] ).section( "=", 1, 1 ).toDouble();
   }
#endif

   // Read the description
   data.description = ts.readLine();

   // Read scan parameters
   QString     sc = ts.readLine();
   QStringList pp = sc.split( " ", Qt::SkipEmptyParts );

   if ( pp.size() < 8 ) return false;

   data.type          = pp[ 0 ].toLatin1()[ 0 ];  // I P R W F
   data.cell          = pp[ 1 ].toInt();
   data.temperature   = pp[ 2 ].toDouble();
   data.rpm           = pp[ 3 ].toDouble();
   data.seconds       = pp[ 4 ].toDouble();
   data.omega2t       = pp[ 5 ].toDouble();
   data.rpoint        = pp[ 6 ].toDouble();
   data.count         = pp[ 7 ].toInt();
   data.nz_stddev     = false;
#if 1
   // Round speed to nearest multiple of 100
   data.rpm           = qRound( data.rpm / ss_reso ) * ss_reso;
#endif


   // Read radius, data, and standard deviation
   data.rvalues.clear();
   data.stddevs.clear();
   bool interference_data = ( data.type == 'P' );

   while ( ! ts.atEnd() )
   {
      sc = ts.readLine();

      pp = sc.split( " ", Qt::SkipEmptyParts );
      if ( pp.size() == 1 )
         pp = sc.split( "\t", Qt::SkipEmptyParts );
      if ( pp.size() < 2 )
         break;

      double xval = pp[ 0 ].toDouble();
      double rval = pp[ 1 ].toDouble();
      double sval = 0.0;

      if ( ! interference_data  &&  pp.size() > 2 ) 
      {
         sval        = pp[ 2 ].toDouble();
         if ( sval != 0.0 )
            data.nz_stddev = true;
      }

      data.rvalues << rval;
      data.xvalues << xval;
      data.stddevs << sval;
   }

   if ( ! data.nz_stddev )
      data.stddevs.clear();

   ff.close();
   return true;
}

int US_DataIO::writeRawData( const QString& file, RawData& data )
{
   // Open the file for writing
   QFile ff( file );
   if ( ! ff.open( QIODevice::WriteOnly ) ) return CANTOPEN;
   QDataStream ds( &ff );

   // Using quint32 to ensure same data size on all platforms
   quint32 crc = 0xffffffffUL;

   // Write magic number
   char magic[ 5 ] = "UCDA";
   write( ds, magic, 4, crc );
 
   // Write format version
   char fmt[ 3 ];
   sprintf( fmt, "%2.2i", format_version );
   write( ds, fmt, 2, crc );

   // Write data type
   write( ds, data.type, 2, crc );

   // Write cell
   write( ds, (const char*)&data.cell, 1, crc );

   // Write channel
   write( ds, (const char*)&data.channel, 1, crc );

   // Create and write a guid
   write( ds, data.rawGUID, 16, crc );

   // Write description
   char desc[ 240 ];
   memset( desc, '\0', sizeof desc );  // bzero is not defined in WIN32

   QByteArray dd = data.description.toLatin1();
   strncpy( desc, dd.data(), sizeof desc );
   write( ds, desc, sizeof desc, crc );

   // Find min and max radius, data, and std deviation
   // First the radii
   double min_radius =  1.0e99;
   double max_radius = -1.0e99;

   for ( int ii = 0; ii < data.xvalues.size(); ii++ )
   {
      min_radius = qMin( min_radius, data.xvalues[ ii ] );
      max_radius = qMax( max_radius, data.xvalues[ ii ] );
   }

   // Now the data and SD
   Parameters pp;

   pp.min_data1      =  1.0e99;
   pp.max_data1      = -1.0e99;
   pp.min_data2      =  1.0e99;
   pp.max_data2      = -1.0e99;

   bool    allz_stdd = true;
   int     scCount   = data.scanCount();
   int     ptCount   = data.pointCount();

   for ( int ii = 0; ii < scCount; ii++ )
   {
      Scan* sc          = &data.scanData[ ii ];
      bool  nz_stddev   = ( sc->nz_stddev  &&  sc->stddevs.size() > 0 );

      for ( int jj = 0; jj < ptCount; jj++ )
      {
         pp.min_data1      = qMin( pp.min_data1, sc->rvalues[ jj ] );
         pp.max_data1      = qMax( pp.max_data1, sc->rvalues[ jj ] );

         if ( nz_stddev  )
         {
            pp.min_data2      = qMin( pp.min_data2, sc->stddevs[ jj ] );
            pp.max_data2      = qMax( pp.max_data2, sc->stddevs[ jj ] );
            allz_stdd         = false;
         }
      }
   }

   if ( allz_stdd )
   {
      pp.min_data2      = 0.0;
      pp.max_data2      = 0.0;
   }

   // Distance between radius entries
   double r1    = data.xvalues[ 0 ];
   double r2    = data.xvalues[ 1 ];

   // The unions below are a little tricky in order to get the
   // c++ types and architecture endian-ness right.  Floats
   // are written to uf.f (union float - float ).  Then the 
   // 4 bytes of the float are treated as a 32-bit unsigned int and
   // converted to ui.u (union integer - unsigned ).  Finally
   // the ui.c type is to satify the c++ type for the write call.

   union
   {
      quint32 u;
      float   f;
   } uf;

   union
   {
      char  c[ 4 ];
      uchar u[ 4 ];
   } ui;

   uf.f = (float) min_radius;  // min_radius
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );

   uf.f = (float) max_radius;  // max_radius
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );

   uf.f = (float) ( r2 - r1 );  // delta r
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );

   uf.f = (float) pp.min_data1;  // minimum data value
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );
   
   uf.f = (float) pp.max_data1;  // maximum data value
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );

   uf.f = (float) pp.min_data2;  // minimum std deviation value
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );
   
   uf.f = (float) pp.max_data2;  // maximum std deviation value
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );

   // Write out scan count
   qToLittleEndian( (quint16)data.scanData.size(), ui.u );
   write( ds, ui.c, 2, crc );

   // Loop for each scan
   for ( int ii = 0; ii < data.scanData.size(); ii++ )
      writeScan( ds, data.scanData[ ii ], crc, pp );

   qToLittleEndian( crc, ui.u ); // crc
   ds.writeRawData( ui.c, 4 );

   ff.close();

   return OK;
}

void US_DataIO::writeScan( QDataStream&    ds, const Scan&       data, 
                           quint32&       crc, const Parameters& pp )
{
   union
   {
      quint32 u;
      float   f;
   } uf;

   union
   {
      char  c[ 4 ];
      uchar u[ 4 ];
   } ui;

   quint16 si;  

   char d[ 5 ] = "DATA";
   write( ds, d, 4, crc );

   uf.f = (float) data.temperature;  // scan temperature
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );

   uf.f = (float) data.rpm;          // scan rpm
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );

   qToLittleEndian( (quint32)data.seconds, ui.u );  // scan time
   write( ds, ui.c, 4, crc );

   uf.f = (float) data.omega2t;      //scan omega^2 t
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );

   // Encoded wavelength
   si = (quint16)qRound( data.wavelength * 10.0 );
   qToLittleEndian<quint16>( si, ui.u );
//qDebug() << "DIO: wvln si" << data.wavelength << si
// << "ui.c" << (int)ui.c[0] << (int)ui.c[1];
   write( ds, ui.c, 2, crc );

   uf.f = (float) data.delta_r;     // delta r 
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );

   quint32 valueCount = data.rvalues.size(); // number of values
   qToLittleEndian( valueCount, ui.u );
   write( ds, ui.c, 4, crc );

   // Write readings
   double  delta  = ( pp.max_data1 - pp.min_data1 ) / 65535;
   double  delta2 = ( pp.max_data2 - pp.min_data2 ) / 65535;

   bool    stdDev = ( ( pp.min_data2 != 0.0  ||  pp.max_data2 != 0.0 )
                  &&  ( data.stddevs.size() == data.rvalues.size()   ) );

   for ( int ii = 0; ii < data.rvalues.size(); ii++ )
   {
      double rval = data.rvalues[ ii ];
      // reading
      si = (quint16) qRound( ( rval - pp.min_data1 ) / delta );
      qToLittleEndian( si, ui.u );
      write( ds, ui.c, 2, crc );

      // If applicable, write std deviation
      if ( stdDev )
      {
         double sval = data.stddevs[ ii ];
         si = (quint16) qRound( ( sval - pp.min_data2 ) / delta2 );
         qToLittleEndian( si, ui.u );
         write( ds, ui.c, 2, crc );
      }
   }

   // Write interpolated flags
   int flagSize = ( valueCount + 7 ) / 8;
   write( ds, data.interpolated.data(), flagSize, crc );
}

void US_DataIO::write( QDataStream& ds, const char* c, int len, quint32& crc )
{
   ds.writeRawData( c, len );
   crc = US_Crc::crc32( crc, (unsigned char*) c, len );
}

int US_DataIO::readRawData( const QString& file, RawData& data )
{
   QFile ff( file );
   if ( ! ff.open( QIODevice::ReadOnly ) ) return CANTOPEN;
   QDataStream ds( &ff );

   int      err = OK;
   quint32  crc = 0xffffffffUL;
#if 1
   double ss_reso      = 100.0;
   // If debug_text so directs, change set_speed_resolution
   QStringList dbgtxt = US_Settings::debug_text();
   for ( int ii = 0; ii < dbgtxt.count(); ii++ )
   {  // If debug text modifies ss_reso, apply it
      if ( dbgtxt[ ii ].startsWith( "SetSpeedReso" ) )
         ss_reso       = QString( dbgtxt[ ii ] ).section( "=", 1, 1 ).toDouble();
   }
#endif

   try
   {
      // Read magic number
      char magic[ 4 ];
      read( ds, magic, 4, crc );
      if ( strncmp( magic, "UCDA", 4 ) != 0 ) throw NOT_USDATA;
    
      // Check the version number
      unsigned char ver[ 2 ];
      read( ds, (char*) ver, 2, crc );
      quint32 version = ( ( ver[ 0 ] & 0x0f ) << 8 ) | ( ver[ 1 ] & 0x0f );
      if ( version > format_version ) throw BAD_VERSION;

      bool wvlf_new  = ( version > (quint32)4 );

      // Read and get the file type
      char type[ 3 ];
      read( ds, type, 2, crc );
      type[ 2 ] = '\0';
    
      QStringList types = QStringList() << "RA" << "IP" << "RI" << "FI" 
                                        << "WA" << "WI";
    
      if ( ! types.contains( QString( type ) ) ) throw BADTYPE;
      strncpy( data.type, type, 2 );

      // Get the cell
      union 
      { char c[ 4 ];
        int  i;
      } cell;

      cell.i = 0;

      read( ds, cell.c, 1, crc );
      data.cell = qFromLittleEndian( cell.i );

      // Get the channel
      read( ds, &data.channel, 1, crc );

      // Get the guid
      read( ds, data.rawGUID, 16, crc );
    
      // Get the description
      char desc[ 240 ];
      read( ds, desc, 240, crc );
      data.description = QString( desc );

      // Get the parameters to expand the values
      union
      {
         char    c[ 2 ];
         quint16 I;
      } si;

      union
      {
         char   c[ 4 ];
         qint32 I;
         float  f;
      } u1;

      union
      {
         char   c[ 4 ];
         qint32 I;
         float  f;
      } u2;

      double min_radius;

      read( ds, u1.c, 4, crc );
      u2.I = qFromLittleEndian( u1.I );
      min_radius = u2.f;

      read( ds, u1.c, 4, crc );
      // Unused
      //u2.I = qFromLittleEndian( u1.I );
      //min_radius = u2.f;

      read( ds, u1.c, 4, crc );
      u2.I = qFromLittleEndian( u1.I );
      double delta_radius = u2.f;

      read( ds, u1.c, 4, crc );
      u2.I = qFromLittleEndian( u1.I );
      double min_data1 = u2.f;

      read( ds, u1.c, 4, crc );
      u2.I = qFromLittleEndian( u1.I );
      double max_data1 = u2.f;

      read( ds, u1.c, 4, crc );
      u2.I = qFromLittleEndian( u1.I );
      double min_data2 = u2.f;

      read( ds, u1.c, 4, crc );
      u2.I = qFromLittleEndian( u1.I );
      double max_data2 = u2.f;

      read( ds, si.c, 2, crc );
      qint16 scan_count = qFromLittleEndian( si.I );

      // Read each scan
      int valueCount = 0;
      for ( int ii = 0 ; ii < scan_count; ii ++ )
      {
         read( ds, u1.c, 4, crc );
         if ( strncmp( u1.c, "DATA", 4 ) != 0 ) throw NOT_USDATA;

         Scan sc;
         
         // Temperature
         read( ds, u1.c, 4, crc );
         u2.I = qFromLittleEndian( u1.I );
         sc.temperature = u2.f;

         // RPM
         read( ds, u1.c, 4, crc );
         u2.I = qFromLittleEndian( u1.I );
         sc.rpm = u2.f;
#if 1
//sc.rpm = qRound( sc.rpm / 50.0 ) * 50.0;
         // Round speed to nearest multiple of 100 (or other resolution)
         sc.rpm = qRound( sc.rpm / ss_reso ) * ss_reso;
#endif

         // Seconds
         read( ds, u1.c, 4, crc );
         sc.seconds = qFromLittleEndian( u1.I );

         // Omega2t
         read( ds, u1.c, 4, crc );
         u2.I = qFromLittleEndian( u1.I );
         sc.omega2t = u2.f;

         // Wavelength
         read( ds, si.c, 2, crc );
         if ( wvlf_new )
            sc.wavelength = qFromLittleEndian( si.I ) / 10.0;
         else
            sc.wavelength = qFromLittleEndian( si.I ) / 100.0 + 180.0;

         // Delta_r
         read( ds, u1.c, 4, crc );
         u2.I = qFromLittleEndian( u1.I );
         sc.delta_r = u2.f;

         // Reading count
         read( ds, u1.c, 4, crc );
         valueCount = qFromLittleEndian( u1.I );

         // Get the readings
         double  factor1 = ( max_data1 - min_data1 ) / 65535.0;
         double  factor2 = ( max_data2 - min_data2 ) / 65535.0;
         bool    stdDev  = ( min_data2 != 0.0 || max_data2 != 0.0 );

         for ( int jj = 0; jj < valueCount; jj++ )
         {
            double rval;
            double sval;

            read( ds, si.c, 2, crc );
            rval    = qFromLittleEndian( si.I ) * factor1 + min_data1;

            if ( stdDev )
            {
               read( ds, si.c, 2, crc );
               sval     = qFromLittleEndian( si.I ) * factor2 + min_data2;
            }
            else
               sval     = 0.0;

            // Add the reading to the scan
            sc.rvalues << rval;
            sc.stddevs << sval;
         } 

         if ( !stdDev )
         {
            sc.stddevs.clear();
            sc.nz_stddev    = false;
         }
         else
            sc.nz_stddev    = true;

         // Get the interpolated bitmap;
         int bytes          = ( valueCount + 7 ) / 8;
         char* interpolated = new char[ bytes ];
         
         read( ds, interpolated, bytes, crc );

         sc.interpolated = QByteArray( interpolated, bytes );

         delete [] interpolated;

         // Add the scan to the data
         data.scanData <<  sc;
      }

      // Calculate the radius vector
      data.xvalues.clear();
      double  radius  = min_radius;
      
      for ( int jj = 0; jj < valueCount; jj++ )
      {
         data.xvalues << radius;
         radius += delta_radius;
      }

      // Read the crc
      quint32 read_crc;
      ds.readRawData( (char*) &read_crc , 4 );
      if ( crc != qFromLittleEndian( read_crc ) ) throw BADCRC;

   } catch( ioError error )
   {
      err = error;
   }

   ff.close();
   return err;
}

void US_DataIO::read( QDataStream& ds, char* cc, int len, quint32& crc )
{
   ds.readRawData( cc, len );
   crc = US_Crc::crc32( crc, (uchar*) cc, len );
}

int US_DataIO::readEdits( const QString& filename, EditValues& parameters )
{
   QFile ff( filename );
   if ( ! ff.open( QIODevice::ReadOnly ) ) return CANTOPEN;

   QXmlStreamReader xml( &ff );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "identification" ) 
            ident ( xml, parameters );
         
         else if ( xml.name() == "run" ) 
            run( xml, parameters );

         else if ( xml.name() == "experiment" )
         {
            QXmlStreamAttributes a = xml.attributes();
            parameters.expType     = a.value( "type" ).toString();
         }
      }
   }

   bool error = xml.hasError();
   ff.close();
   
   if ( error ) return BADXML;

   return OK;
}

void US_DataIO::ident( QXmlStreamReader& xml, EditValues& parameters )
{
   while ( ! xml.atEnd() )
   {
      if ( xml.isEndElement()  &&  xml.name() == "identification" ) return;
     
      if ( xml.isStartElement()  &&  xml.name() == "runid" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.runID       = a.value( "value" ).toString();
      }
     
      if ( xml.isStartElement()  &&  xml.name() == "editGUID" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.editGUID    = a.value( "value" ).toString();
      }

      if ( xml.isStartElement()  &&  xml.name() == "rawDataGUID" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.dataGUID    = a.value( "value" ).toString();
      }
     
      xml.readNext();
   }
}

void US_DataIO::run( QXmlStreamReader& xml, EditValues& parameters )
{
   QXmlStreamAttributes a = xml.attributes();
   parameters.cell       = a.value( "cell"       ).toString();
   parameters.channel    = a.value( "channel"    ).toString();
   parameters.wavelength = a.value( "wavelength" ).toString();

   if ( parameters.wavelength.contains( "-" ) )
      all_lambdas( xml, parameters );

   while ( ! xml.atEnd() )
   {
      if ( xml.isEndElement()    &&  xml.name() == "run" )  return;

      if ( xml.isStartElement()  &&  xml.name() == "parameters" )
         params( xml, parameters );

      if ( xml.isStartElement()  &&  xml.name() == "operations" )
         operations( xml, parameters );

      if ( xml.isStartElement()  &&  xml.name() == "edited" )
         do_edits( xml, parameters );

      if ( xml.isStartElement()  &&  xml.name() == "excludes" )
         excludes( xml, parameters );

      xml.readNext();
   }
}

void US_DataIO::all_lambdas( QXmlStreamReader& xml, EditValues& parameters )
{
   while ( ! xml.atEnd() )
   {
      if ( xml.isEndElement()    &&  xml.name() == "lambdas" ) return;

      if ( xml.isStartElement()  &&  xml.name() == "lambda" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.lambdas << a.value( "value" ).toString().toInt();
      }

      xml.readNext();
   }
}

void US_DataIO::params( QXmlStreamReader& xml, EditValues& parameters )
{
   parameters.speedData.clear();
   bool isEquil = ( parameters.expType == "Equilibrium" );
   int  spx     = 0;
   parameters.bottom   = 0.0;

   while ( ! xml.atEnd() )
   {
      if ( xml.isEndElement()  &&  xml.name() == "parameters" ) return;

      else if ( !xml.isStartElement() )
      {
         if ( xml.isEndElement()  &&  xml.name() == "speed" )
            spx++;

         xml.readNext();
         continue;
      }

      else if ( xml.name() == "speed" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.speedData << SpeedData();
         parameters.speedData[ spx ].speed
            = a.value( "value"     ).toString().toDouble();
         parameters.speedData[ spx ].first_scan
            = a.value( "scanStart" ).toString().toInt();
         parameters.speedData[ spx ].scan_count
            = a.value( "scanCount" ).toString().toInt();
      }

      else if ( xml.name() == "meniscus" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.meniscus = a.value( "radius" ).toString().toDouble();

         if ( isEquil )
            parameters.speedData[ spx ].meniscus = parameters.meniscus;
      }

      else if ( xml.name() == "bottom" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.bottom   = a.value( "radius" ).toString().toDouble();
      }

      else if ( xml.name() == "plateau" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.plateau = a.value( "radius" ).toString().toDouble();
      }

      else if ( xml.name() == "air_gap" )
      {
         QXmlStreamAttributes a  = xml.attributes();
         parameters.airGapLeft   = a.value( "left"      ).toString().toDouble();
         parameters.airGapRight  = a.value( "right"     ).toString().toDouble();
         parameters.gapTolerance = a.value( "tolerance" ).toString().toDouble();
      }

      else if ( xml.name() == "baseline" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.baseline = a.value( "radius" ).toString().toDouble();
      }

      else if ( xml.name() == "data_range" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.rangeLeft  = a.value( "left"  ).toString().toDouble();
         parameters.rangeRight = a.value( "right" ).toString().toDouble();

         if ( isEquil )
         {
            parameters.speedData[ spx ].dataLeft  = parameters.rangeLeft;
            parameters.speedData[ spx ].dataRight = parameters.rangeRight;
         }
      }

      else if ( xml.name() == "linear_baseline_correction" )
	{
	  QXmlStreamAttributes a = xml.attributes();
	  parameters.bl_corr_slope      = a.value( "slope"  ).toString().toDouble();
	  parameters.bl_corr_yintercept = a.value( "y_intercept" ).toString().toDouble();
	}

      else if ( xml.name() == "od_limit" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.ODlimit  = a.value( "value" ).toString().toDouble();
      }

      xml.readNext();
   }
}

void US_DataIO::operations( QXmlStreamReader& xml, EditValues& parameters )
{
   while ( ! xml.atEnd() )
   {
      if ( xml.isEndElement()  &&  xml.name() == "operations" ) return;

      if ( xml.isStartElement()  &&  xml.name() == "invert" )
         parameters.invert = -1.0;

      if ( xml.isStartElement()  &&  xml.name() == "remove_spikes" )
         parameters.removeSpikes = true;

      if ( xml.isStartElement()  &&  xml.name() == "floating_data" )
         parameters.floatingData = true;

      if ( xml.isStartElement()  &&  xml.name() == "subtract_ri_noise" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.noiseOrder = a.value( "order" ).toString().toInt();
      }

      xml.readNext();
   }
}

void US_DataIO::excludes( QXmlStreamReader& xml, EditValues& parameters )
{
   while ( ! xml.atEnd() )
   {
      if ( xml.isEndElement()  &&  xml.name() == "excludes" ) return;

      if ( xml.isStartElement()  &&  xml.name() == "exclude" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.excludes << a.value( "scan" ).toString().toInt();
      }

      xml.readNext();
   }
}

void US_DataIO::do_edits( QXmlStreamReader& xml, EditValues& parameters )
{
   while ( ! xml.atEnd() )
   {
      if ( xml.isEndElement()  &&  xml.name() == "edited" ) return;

      if ( xml.isStartElement()  &&  xml.name() == "edit" )
      {
         EditedPoint e;
         QXmlStreamAttributes a = xml.attributes();
         e.scan   = a.value( "scan"   ).toString().toInt();
         e.radius = a.value( "radius" ).toString().toDouble();
         e.value  = a.value( "value"  ).toString().toDouble();

         parameters.editedPoints << e;
      }

      xml.readNext();
   }
}

int US_DataIO::loadData( const QString&         directory, 
                         const QString&         editFilename,
                         QVector< EditedData >& data )
{
   QVector< RawData > raw;

   return loadData( directory, editFilename, data, raw );
}

int US_DataIO::loadData( const QString&  directory, 
                         const QString&  editFilename,
                         EditedData&     data )
{
   QVector< RawData    > raw;
   QVector< EditedData > editedData;

   int result = loadData( directory, editFilename, editedData, raw );
   
   if ( result == OK ) 
      data = editedData[ 0 ];

   return result;
}

// Load edited and raw data
int US_DataIO::loadData( const QString&         directory, 
                         const QString&         editFilename,
                         QVector< EditedData >& data,
                         QVector< RawData    >& raw )
{
   QString ftriple     = editFilename.section( ".", -2, -2 );
   // Determine raw file name by removing editID
   QString rawDataFile = editFilename;
   QString edtFileRead = editFilename;
   QString filepart1   = editFilename.section( ".",  0, -7 );
   QString filepart2   = editFilename.section( ".", -5, -3 );
   rawDataFile         = filepart1 + "." + filepart2 + ".";
   QString clambda     = ftriple     .section( ".", -1, -1 );
   bool    isMwl       = clambda.contains( "-" );

   if ( isMwl )
   {
      QString elambda     = clambda     .section( "@", -2, -2 );
      clambda             = clambda     .section( "@", -1, -1 );
      edtFileRead         = editFilename.section( ".",  0, -3 )
                            + "." + elambda + ".xml";
   }

   rawDataFile         = rawDataFile + clambda + ".auc";

//qDebug() << "dIO:ldEd: editFilename" << editFilename;
//qDebug() << "dIO:ldEd: rawDataFile" << rawDataFile;
//qDebug() << "dIO:ldEd: edtFileRead" << edtFileRead;

   // Get the raw data
   RawData dd;
   ioError result = (ioError)readRawData( directory + "/" + rawDataFile, dd );
   if ( result != OK ) throw result;
   raw << dd;
   qApp->processEvents();

   // Get the edit data
   EditValues ev;
   result = (ioError)readEdits( directory + "/" + edtFileRead, ev );
   if ( result != OK ) throw result;

   // Check for uuid match
   QString rawGuid = US_Util::uuid_unparse( (uchar*)dd.rawGUID );

   if ( rawGuid != ev.dataGUID )
   {
      QString clambda = edtFileRead.section( ".", -2, -2 );
      if ( clambda.contains( "-" ) )
         ev.dataGUID = rawGuid;
      else
         throw NO_GUID_MATCH;
   }

   // Apply the edits
   EditedData ed;

   QStringList sl = editFilename.split( "." );

   ed.runID       = sl[ 0 ];
   ed.editID      = sl[ 1 ];
   ed.dataType    = sl[ 2 ];
   ed.cell        = sl[ 3 ];
   ed.channel     = sl[ 4 ];
   ed.wavelength  = clambda;
   ed.description = dd.description;
   ed.expType     = ev.expType;
   ed.dataGUID    = ev.dataGUID;
   ed.editGUID    = ev.editGUID;
   ed.meniscus    = ev.meniscus;
   ed.bottom      = ev.bottom;
   ed.plateau     = ev.plateau;
   ed.baseline    = ev.baseline;
   ed.ODlimit     = ( ed.dataType == "RA"  ||  ed.dataType == "RI" ) ?
                    ev.ODlimit : 1e+99;
   ed.floatingData = ev.floatingData;

   ed.bl_corr_slope = ev.bl_corr_slope;
   ed.bl_corr_yintercept = ev.bl_corr_yintercept;
//qDebug() << "dIO:ldEd: ed.descr" << ed.description
// << "ed.wavelength" << ed.wavelength << "ed.dataType ed.ODlimit"
// << ed.dataType << ed.ODlimit;

   if ( ed.expType == "Equilibrium" )
      ed.speedData << ev.speedData;

   // Invert values before updating edited points
   if ( ev.invert < 0 )
   {
      for ( int ii = 0; ii < dd.scanData.size(); ii++ )
      {
         Scan* sc = &dd.scanData[ ii ];
         for ( int jj = 0; jj < sc->rvalues.size(); jj++ )
         {
            sc->rvalues[ jj ] *= ev.invert;
         }
      }
   }

   // Update any edited points
   for ( int ii = 0; ii < ev.editedPoints.length(); ii++ )
   {
      int    scnx   = ev.editedPoints[ ii ].scan;
      int    radx   = ev.editedPoints[ ii ].radius;
      double value  = ev.editedPoints[ ii ].value;
      dd.setValue( scnx, radx, value );
   }

   // Update for interference data
   if ( ed.dataType == "IP" )
   {
      adjust_interference( dd, ev );  // rawData, editValues
      calc_integral      ( dd, ev );
   }

   // Do not copy excluded data or data outside the edit range
   for ( int ii = 0; ii < dd.scanData.size(); ii++ )
   {
      if ( ev.excludes.contains( ii ) ) continue;

      Scan sc;
      copyRange( ev.rangeLeft, ev.rangeRight, dd.scanData[ ii ],
                 sc, dd.xvalues );
     
      ed.scanData << sc;
   }

   // Only need to copy radius vector for given range once
   copyxRange( ev.rangeLeft, ev.rangeRight, dd.xvalues, ed.xvalues );

   // Determine plateau values for each scan
   for ( int ii = 0; ii < ed.scanData.size(); ii++ )
   {
      int point                  = ed.xindex( ed.plateau );
      ed.scanData[ ii ].plateau  = ed.scanData[ ii ].rvalues[ point ];
   }

   if ( ev.removeSpikes )
   {
      double smoothed_value;

      // For each scan
      for ( int ii = 0; ii < ed.scanData.size(); ii++ )
      {
         Scan* sc     = &ed.scanData [ ii ];
         int   start  = 0;
         int   end    = sc->rvalues.size() - 1;     // index of the last one

         for ( int jj = start; jj < end; jj++ )
         {
            if ( spike_check( *sc, ed.xvalues, jj, start, end,
                              &smoothed_value ) )
            {
               sc->rvalues[ jj ] = smoothed_value;

               // If previous consecutive points are interpolated, then
               //  redo them
               int           index = jj - 1;
               unsigned char cc    = sc->interpolated[ index / 8 ];

               while ( cc & ( 1 << ( 7 - index % 8 ) ) )
               {
                  if ( spike_check( *sc, ed.xvalues, index, start, end,
                                    &smoothed_value ) )
                     sc->rvalues[ index ] = smoothed_value;

                  index--;
                  cc = sc->interpolated[ index / 8 ];
               }
            }
         }
      }
   }

   if ( ev.noiseOrder > 0 )
   {
      QList< double > residuals = calc_residuals( ev.noiseOrder, ed.scanData );
      
      for ( int ii = 0; ii < ed.scanData.size(); ii++ )
      {
         for ( int jj = 0; jj <  ed.scanData[ ii ].rvalues.size(); jj++ )
            ed.scanData[ ii ].rvalues[ jj ] -= residuals[ ii ];
      }
   }

   data << ed;
   return OK;
}

// Adjust interference data
void US_DataIO::adjust_interference( RawData& data, const EditValues& ev )
{
   // Find first scan
   for ( int ii = 0; ii < data.scanData.size(); ii++ )
   {
      if ( ev.excludes.contains( ii ) ) continue;

      US_DataIO::Scan* sc;

      int r_left  = index( data.xvalues, ev.airGapLeft );
      int r_right = index( data.xvalues, ev.airGapRight );
      double sum  = 0.0;

      for ( int kk = r_left; kk <= r_right; kk++ )
         sum += data.reading( ii, kk );

      double average = sum / ( r_right - r_left + 1 );

      for ( int jj = ii + 1; jj < data.scanData.size(); jj++ )
      {
         // Get average difference between gap in first included
         // scan and each subsequent scan

         sc  = &data.scanData[ jj ];
         sum = 0.0;

         for ( int kk = r_left; kk <= r_right; kk++ )
            sum += data.reading( jj, kk ) - data.reading( ii, kk );

         double delta = sum / ( r_right - r_left + 1 );

         // Subtract average difference from all points
         for ( int kk = 0; kk < sc->rvalues.size(); kk++ )
            sc->rvalues[ kk ] -= delta;
      }

      for ( int jj = ii; jj < data.scanData.size(); jj++ )
      {
         sc  = &data.scanData[ jj ];

         for ( int kk = 0; kk < sc->rvalues.size(); kk++ )
            sc->rvalues[ kk ] -= average;
      }

      return; // After first included scan
   }
}

// Calculate and apply an integration to readings data
void US_DataIO::calc_integral( RawData& data, const EditValues& ee )
{
   // This function gets a little tricky because we have raw data, but
   // want to adjust in the range identified by the user *and* 
   // not use excluded scans.

   int               kk = 0;
   QVector< int    > included;
   QVector< double > integral;
   double            val_min = 1.e+99;

   for ( int scx = 0; scx < data.scanData.size(); scx++ )
   {
      if ( ee.excludes.contains( scx ) ) continue;

      included << scx;

      integral << 0.0;

      // Arbitrarily add 1000 fringes to each reading value
      // to make sure we don't sum negatives.  This is not needed from a
      // mathematical point of view, but physically, negative fringes 
      // do not make sense.
      US_DataIO::Scan* sc = &data.scanData[ scx ];

      int r_left  = index( data.xvalues, ee.rangeLeft );
      int r_right = index( data.xvalues, ee.rangeRight );

      for ( int rr = r_left; rr <= r_right; rr++ )
         integral[ kk ] += data.reading( scx, rr ) + 1000.0;

      kk++;

      if ( scx == 0 )
      {
         for ( int rr = r_left; rr <= r_right; rr++ )
            val_min  = qMin( val_min, sc->rvalues[ rr ] );
      }
   }

   // Integral fringe shifts contribute exactly ( r_right - r_left + 1 ) 
   // to the integral, since we use unity stepsize in integral calculation.

   for ( int scx = 1; scx < included.size(); scx++ )
   {
      US_DataIO::Scan* sc = &data.scanData[ included[ scx ] ];

      int    r_left    = index( data.xvalues, ee.rangeLeft );
      int    r_right   = index( data.xvalues, ee.rangeRight );
      int    current   = scx; 
      int    previous  = scx - 1; 
      int    position  = 0;
      double points    = (double) ( r_right - r_left + 1 );

      while ( integral[ current ] <= integral[ previous ] )
      {
         integral[ current ] += points;
         position++;
      }

      while ( integral[ current ] > integral[ previous ] )
      {
         integral[ current ] -= points;
         position--;
      }

      // Add the integral steps (which may be negative!) to each datapoint
      for ( int rr = r_left; rr <= r_right; rr++ )
         sc->rvalues[ rr ] += (double) position;

      double diff1 = integral[ previous ] - integral[ current ];

      double diff2 = integral[ previous ] - ( integral[ current ] + points );

      // The scan is one fringe too low
      if ( fabs( diff2 / diff1 ) < ee.gapTolerance )
      {
         // add one fringe to all readings
         for ( int rr = r_left; rr <= r_right; rr++ )
            sc->rvalues[ rr ] += 1.0;

         // Update integral for this scan
         integral[ current ] += points;
      }

      // Accumulate minimum reading after integral adjustment
      for ( int rr = r_left; rr <= r_right; rr++ )
         val_min  = qMin( val_min, sc->rvalues[ rr ] );
   }

   // Apply a bias to the data so that its minimum is above zero
   if ( val_min < 0.0 )
   {
      double val_bias = (double)qCeil( -val_min );

      for ( int scx = 0; scx < included.size(); scx++ )
      {
         US_DataIO::Scan* sc = &data.scanData[ included[ scx ] ];

         int r_left  = index( data.xvalues, ee.rangeLeft  );
         int r_right = index( data.xvalues, ee.rangeRight );

         for ( int rad = r_left; rad <= r_right; rad++ )
         {
             sc->rvalues[ rad ] += val_bias;
         }
      }
   }

   qDebug() << "Calc INTEGRAL DONE;";
}

// Copy a range of readings values from one data set to another
void US_DataIO::copyRange ( double left, double right, const Scan& orig,
                            Scan& dest, const QVector< double >& origx )
{
   dest.temperature = orig.temperature;
   dest.rpm         = orig.rpm;
   dest.seconds     = orig.seconds;
   dest.omega2t     = orig.omega2t;
   dest.wavelength  = orig.wavelength;
   dest.delta_r     = orig.delta_r;
   dest.nz_stddev   = orig.nz_stddev;
   dest.stddevs.clear();

   int index_L      = index( origx, left );
   int index_R      = index( origx, right );

   dest.interpolated.fill( (char)0, ( index_R - index_L ) / 8 + 1 );
   
   int current_bit = 0;

   for ( int ii = index_L; ii <= index_R; ii++ )
   {
      // Copy the concentration readings
      dest.rvalues << orig.rvalues[ ii ];

      // Copy the standard deviations if any are non-zero
      if ( orig.nz_stddev )
         dest.stddevs << orig.stddevs[ ii ];

      // Set the interpolated bits as needed
      unsigned char old_bit = (unsigned char)( 1 << ( 7 - ii % 8 ) );

      if ( ( orig.interpolated[ ii / 8 ] & old_bit ) != 0 )
      {
         unsigned char new_bit =  (unsigned char)( 1 << ( 7 - current_bit % 8 ) );
         unsigned char byte    = dest.interpolated[ current_bit / 8 ];
         dest.interpolated[ current_bit / 8 ] = byte | new_bit;
      }

      current_bit++;
   }

   if ( orig.nz_stddev )
   {  // Likely some stddevs non-zero, but double-check since range changed
      int nnz = 0;

      for ( int ii = 0; ii < dest.stddevs.count(); ii++ )
         if ( dest.stddevs[ ii ] != 0.0 )
            nnz++;

      dest.nz_stddev   = ( nnz > 0 );
   }
}

// Copy a range of X (radius) values from one data set to another
void US_DataIO::copyxRange( double left, double right,
                            const QVector< double >& origx,
                                  QVector< double >& destx )
{
   for ( int ii = index( origx, left ); ii <= index( origx, right ); ii++ )
   {  // Copy all X values within the specified range
      destx << origx[ ii ];
   }
}

// Determine if a readings value is a spike
bool US_DataIO::spike_check( const US_DataIO::Scan&  sc,
                             const QVector< double >& xvalues,
                             int point, int start, int end, double* value )
{
   static double rr[ 20 ];  // Spare room -- normally 10
   static double vv[ 20 ];

   double*       xx = &rr[ 0 ];
   double*       yy = &vv[ 0 ];

   double        slope;
   double        intercept;
   double        correlation;
   double        sigma = 0.0;
   int           count = 0;
   int           index;

   const double  threshold = 10.0;
   unsigned char cc;
   unsigned char interpolated;

   // Only use non-interpolated points for the line fit
   // Decide to search left or right first.
   if ( point < ( start + end ) / 2 )
   {
      index = point - 1;

      // Search left for five points if we can
      while ( count < 5 )
      {
         if ( index < start ) break;

         cc           = sc.interpolated[ index / 8 ];
         interpolated =  (unsigned char)( cc & ( 1 << ( 7 - index % 8 ) ) );

         if ( ! interpolated )
         {
            rr[ count ] = xvalues   [ index ];
            vv[ count ] = sc.rvalues[ index ];
            count++;
         }

         index--;
      }

      // Now search right
      index = point + 1;

      while ( count < 10 )
      {
         if ( index >= end ) break;

         cc           = sc.interpolated[ index / 8 ];
         interpolated =  (unsigned char)( cc & ( 1 << ( 7 - index % 8 ) ) );

         if ( ! interpolated )
         {
            rr[ count ] = xvalues   [ index ];
            vv[ count ] = sc.rvalues[ index ];
            count++;
         }

         index++;
      }
   }
   else
   {
      index = point + 1;

      // Search right first
      while ( count < 5 )
      {
         if ( index >= end ) break;

         cc           = sc.interpolated[ index / 8 ];
         interpolated =  (unsigned char)( cc & ( 1 << ( 7 - index % 8 ) ) );

         if ( ! interpolated )
         {
            rr[ count ] = xvalues   [ index ];
            vv[ count ] = sc.rvalues[ index ];
            count++;
         }

         index++;
      }

      // Now search left
      index = point - 1;

      while ( count < 10 )
      {
         if ( index < start ) break;

         cc           = sc.interpolated[ index / 8 ];
         interpolated =  (unsigned char)( cc & ( 1 << ( 7 - index % 8 ) ) );

         if ( ! interpolated )
         {
            rr[ count ] = xvalues   [ index ];
            vv[ count ] = sc.rvalues[ index ];
            count++;
         }

         index--;
      }
   }

   US_Math2::linefit( &xx, &yy, &slope, &intercept, &sigma,
                      &correlation, count );

   // If there is too much difference, it is a spike
   double current   = sc.rvalues[ point ];
   double radius    = xvalues   [ point ];
   double projected = slope * radius + intercept;

   if ( fabs( projected - current ) > threshold * sigma )
   {
      // Interpolate
      *value = projected;
      return true;
   }

   return false;  // Not a spike
}

QList< double > US_DataIO::calc_residuals( int                    order, 
                                           const QVector< Scan >& sl )
{
   int scan_count = sl.size();

   QVector< double > cov( order );
   QVector< double > aiv( scan_count );
   QVector< double > fiv( scan_count );
   QVector< double > tmv( scan_count );
   double* coeffs              = cov.data();
   double* absorbance_integral = aiv.data();
   double* scan_time           = tmv.data();

   // Calculate the integral of each scan which is needed for the least-squares
   // polynomial fit to correct for radially invariant baseline noise. We also
   // keep track of the total integral at each point.

   for ( int ii = 0; ii < scan_count; ii++ )
   {
      absorbance_integral[ ii ] = 0;

      const Scan* sc   = &sl[ ii ];
      int value_count  = sc->rvalues.size();

      double delta_r   = sc->delta_r;

      // Integrate using trapezoid rule
      for ( int jj = 1; jj < value_count; jj++ )
      {
         double avg = ( sc->rvalues[ jj ] + sc->rvalues[ jj - 1 ] ) / 2.0;
         absorbance_integral[ ii ] += avg * delta_r;
      }
   }

   for ( int ii = 0; ii < scan_count; ii++ )
      scan_time[ ii ] =  sl[ ii ].seconds;

   US_Matrix::lsfit( coeffs, scan_time, absorbance_integral, scan_count, order );

   QList< double > residuals;

   for ( int ii = 0; ii < scan_count; ii++ )
   {
      double fit = 0.0;

      for ( int jj = 0; jj < order; jj++ )
         fit       +=  coeffs[ jj ] * pow( sl[ ii ].seconds, jj );

      residuals << absorbance_integral[ ii ] - fit;
   }

   return residuals;
}

// Return index of X (radius or wavelength) value
int US_DataIO::index( US_DataIO::RawData* rdata, double xvalue )
{
   return index( rdata->xvalues, xvalue );
}

// Return index of X (radius or wavelength) value
int US_DataIO::index( US_DataIO::EditedData* edata, double xvalue )
{
   return index( edata->xvalues, xvalue );
}

// Return index of X (radius or wavelength) value
int US_DataIO::index( const QVector< double >& xvals, double xvalue )
{
   int    npoint = xvals.size();
   int    last   = npoint - 1;

   // If xvalue beyond values at extremes, return 1st or last index
   if ( xvalue <= xvals[ 0 ] )    return 0;       // Return first index
    
   if ( xvalue >= xvals[ last ] ) return last;    // Return last index

   // Otherwise, return the index to a match or the least difference
   double rdmin  = qAbs( xvalue - xvals[ 0 ] );
   int    mindx  = 0;

   for ( int ii = 1; ii < npoint; ii++ )
   {
      double rdiff  = qAbs( xvalue - xvals[ ii ] );

      if ( rdiff == 0.0 )           return ii;    // Return match index

      if ( rdiff < rdmin )
      {  // Save the minimum difference value; and its index for possible use
         rdmin         = rdiff;
         mindx         = ii;
      }
   }

   return mindx;                                  // Return min-diff index
}

// Compose and return an error string
QString US_DataIO::errorString( int code )
{
   switch ( code )
   {
      case OK        
         : return QObject::tr( "The operation completed successully" );
      case CANTOPEN  : return QObject::tr( "The file cannot be opened" );
      case BADCRC    : return QObject::tr( "The file was corrupted" );
      case NOT_USDATA: return QObject::tr( "The file was not valid scan data" );
      case BADTYPE   : return QObject::tr( "The filetype was not recognized" );
      case BADXML    : return QObject::tr( "The XML file was invalid" );
      case NODATA    : return QObject::tr( "No legacy data files suitable for import were found" );
      case BAD_VERSION 
         : return QObject::tr( "The file version is not supported" );
      case NO_GUID_MATCH
         : return QObject::tr( "GUIDs in raw data and edit data do not match" );
   }

   return QObject::tr( "Unknown error code" );
}
