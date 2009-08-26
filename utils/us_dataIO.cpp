//! \file us_dataIO.cpp
#include <uuid/uuid.h>

#include "us_dataIO.h"
#include "us_crc.h"
#include "us_math.h"

bool US_DataIO::readLegacyFile( const QString& file, 
                                beckmanRaw&    data )
{
   // Open the file for reading
   QFile f( file );
   if ( ! f.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
   QTextStream ts( &f );

   // Read the description
   data.description = ts.readLine();

   // Read scan parameters
   QString     s = ts.readLine();
   QStringList p = s.split( " ", QString::SkipEmptyParts );

   if ( p.size() < 8 ) return false;

   data.type          = p[ 0 ].toAscii()[ 0 ];  // I P R W F
   data.cell          = p[ 1 ].toInt();
   data.temperature   = p[ 2 ].toDouble();
   data.rpm           = p[ 3 ].toDouble();
   data.seconds       = p[ 4 ].toDouble();
   data.omega2t       = p[ 5 ].toDouble();
   data.t.wavelength  = p[ 6 ].toDouble();
   data.count         = p[ 7 ].toInt();


   // Read radius, data, and standard deviation
   data.readings.clear();
   bool interference_data = ( data.type == 'P' );

   while ( ! ts.atEnd() )
   {
      s = ts.readLine();

      p = s.split( " ", QString::SkipEmptyParts );

      reading r;

      r.d.radius = p[ 0 ].toFloat();
      r.value    = p[ 1 ].toFloat();
      
      if ( ! interference_data & p.size() > 2 ) 
         r.stdDev  = p[ 2 ].toFloat();
      else
         r.stdDev  = 0.0;
      
      data.readings << r;
   }

   f.close();
   return true;
}

int US_DataIO::writeRawData( const QString& file, rawData& data )
{
   // Open the file for writing
   QFile f( file );
   if ( ! f.open( QIODevice::WriteOnly ) ) return CANTOPEN;
   QDataStream ds( &f );

   unsigned long crc = 0xffffffffUL;
   
   // Write magic number
   char magic[ 5 ] = "UCDA";
   write( ds, magic, 4, crc );
   
   // Write data type
   write( ds, data.type, 2, crc );

   // Create and write a guid
   uuid_t uuid;
   uuid_generate( uuid );
   write( ds, (char*) &uuid, 16, crc );

   // Write description
   char desc[ 240 ];
   bzero( desc, sizeof desc );

   QByteArray d = data.description.toLatin1();
   strncpy( desc, d.data(), sizeof desc );
   write( ds, desc, sizeof desc, crc );

   // Find min and max radius, data, and std deviation
   parameters p;

   p.min_data1       =  1.0e99;
   p.max_data1       = -1.0e99;
   p.min_data2       =  1.0e99;
   p.max_data2       = -1.0e99;

   double min_radius =  1.0e99;
   double max_radius = -1.0e99;

   scan    s;
   reading r;

   foreach( s, data.scanData )
   {
      foreach( r, s.values )
      {
         min_radius = min( min_radius, r.d.radius );
         max_radius = max( max_radius, r.d.radius );

         p.min_data1 = min( p.min_data1, r.value );
         p.max_data1 = max( p.max_data1, r.value );

         p.min_data2 = min( p.min_data2, r.stdDev );
         p.max_data2 = max( p.max_data2, r.stdDev );
      }
   }

   short int min_r = (int) ( min_radius * 1000.0 );
   short int max_r = (int) ( max_radius * 1000.0 );

   write( ds, (char*) &min_r,  2, crc );
   write( ds, (char*) &max_r,  2, crc );

   // Distance between radius entries
   double r1    = data.scanData[ 0 ].values[ 0 ].d.radius;
   double r2    = data.scanData[ 0 ].values[ 1 ].d.radius;
   float  delta = (float) ( r2 - r1 );
   write( ds, (char*) &delta, 4, crc );

   float v = (float) p.min_data1;   
   write( ds, (char*) &v, 4, crc );
   
   v = (float) p.max_data1;
   write( ds, (char*) &v, 4, crc );

   v = (float) p.min_data2;
   write( ds, (char*) &v, 4, crc );
   
   v = (float) p.max_data2;
   write( ds, (char*) &v, 4, crc );

   // Write out scan count
   short int count = (short int) data.scanData.size();
   write( ds, (char*) &count, 2, crc );

   // Loop for each scan
   foreach ( s, data.scanData )
      writeScan( ds, s, crc, p );

   ds.writeRawData( (char*) &crc, 4 );
   f.close();

   return OK;
}

void US_DataIO::writeScan( QDataStream&    ds, const scan&       data, 
                           unsigned long& crc, const parameters& p )
{
   char d[ 5 ] = "DATA";
   write( ds, d, 4, crc );

   float t = (float) data.temperature;
   write( ds, (char*) &t, 4, crc );

   int rpm = (int) data.rpm;
   write( ds, (char*) &rpm, 4, crc );

   int s = (int) data.seconds;   
   write( ds, (char*) &s, 4, crc );

   float o = (float) data.omega2t;
   write( ds, (char*) &o, 4, crc );

   float w = (float) data.wavelength;
   write( ds, (char*) &w, 4, crc );

   int valueCount = data.values.size();
   write( ds, (char*) &valueCount, 4, crc );

   // Write reading
   double             delta  = ( p.max_data1 - p.min_data1 ) / 65536;
   double             delta2 = ( p.max_data2 - p.min_data2 ) / 65536;
   unsigned short int si;  // short int

   bool    stdDev = ( p.min_data2 != 0.0 || p.max_data2 != 0.0 );
   reading r;

   foreach ( r, data.values )
   {
      si = (unsigned short int) ( ( r.value - p.min_data1 ) / delta );

      write( ds, (char*) &si, 2, crc );

      // If applicable, write std deviation
      if ( stdDev )
      {
         si = (unsigned short int) ( ( r.stdDev - p.min_data2 ) / delta2 );
         write( ds, (char*) &si, 2, crc );
      }
   }

   // Write interpolated flags
   int flagSize = ( valueCount + 7 ) / 8;
   write( ds, data.interpolated.data(), flagSize, crc );
}

void US_DataIO::write( QDataStream& ds, const char* c, int len, ulong& crc )
{
   ds.writeRawData( c, len );
   crc = US_Crc::crc32( crc, (unsigned char*) c, len );
}

int US_DataIO::readRawData( const QString& file, rawData& data )
{
   QFile f( file );
   if ( ! f.open( QIODevice::ReadOnly ) ) return CANTOPEN;
   QDataStream ds( &f );

   int           err = OK;
   unsigned long crc = 0xffffffffUL;

   try
   {
      // Read magic number
      char magic[ 4 ];
      read( ds, magic, 4, crc );
      if ( strncmp( magic, "UCDA", 4 ) != 0 ) throw NOT_USDATA;
    
      // Read and get the file type
      char type[ 3 ];
      read( ds, type, 2, crc );
      type[ 2 ] = '\0';
    
      QStringList types = QStringList() << "RA" << "IP" << "RI" << "FI" 
                                        << "WA" << "WI";
    
      if ( ! types.contains( QString( type ) ) ) throw BADTYPE;
      strncpy( data.type, type, 2 );
    
      // Get the guid
      read( ds, data.guid, 16, crc );
    
      // Get the description
      char desc[ 240 ];
      read( ds, desc, 240, crc );
      data.description = QString( desc );

      // Get the parameters to expand the values

      union
      {
         char   c[ 2 ];
         ushort I;
      } si;

      read( ds, si.c, 2, crc );
      double min_radius = si.I / 1000.0;

      read( ds, si.c, 2, crc );
      //double max_radius = si.I / 1000.0;

      union
      {
         char  c[ 4 ];
         int   I;
         float f;
      } v;

      read( ds, v.c, 4, crc );
      double delta_radius = v.f;

      read( ds, v.c, 4, crc );
      double min_data1 = v.f;

      read( ds, v.c, 4, crc );
      double max_data1 = v.f;

      read( ds, v.c, 4, crc );
      double min_data2 = v.f;

      read( ds, v.c, 4, crc );
      double max_data2 = v.f;

      short int scan_count;
      read( ds, (char*) &scan_count, 2, crc );

      // Read each scan
      for ( int i = 0 ; i < scan_count; i ++ )
      {
         read( ds, v.c, 4, crc );
         if ( strncmp( v.c, "DATA", 4 ) != 0 ) throw NOT_USDATA;

         scan s;
         
         // Temperature
         read( ds, v.c, 4, crc );
         s.temperature = v.f;

         // RPM
         read( ds, v.c, 4, crc );
         s.rpm = v.I;

         // Seconds
         read( ds, v.c, 4, crc );
         s.seconds = v.I;

         // Omega2t
         read( ds, v.c, 4, crc );
         s.omega2t = v.f;

         // Wavelength
         read( ds, v.c, 4, crc );
         s.wavelength = v.f;

         // Reading count
         read( ds, v.c, 4, crc );
         int valueCount = v.I;

         // Get the readings
         double  radius  = min_radius;
         double  factor1 = ( max_data1 - min_data1 ) / 65536.0;
         double  factor2 = ( max_data2 - min_data2 ) / 65536.0;
         bool    stdDev  = ( min_data2 != 0.0 || max_data2 != 0.0 );

         for ( int j = 0; j < valueCount; j++ )
         {
            reading r;

            r.d.radius = radius;
            
            read( ds, si.c, 2, crc );
            r.value = si.I * factor1 + min_data1;

            if ( stdDev )
            {
               read( ds, si.c, 2, crc );
               r.stdDev = si.I * factor2 + min_data2;
            }
            else
               r.stdDev = 0.0;

            // Add the reading to the scan
            s.values << r;
            
            radius += delta_radius;
         } 

         // Get the interpolated bitmap;
         int bytes          = ( valueCount + 7 ) / 8;
         char* interpolated = new char[ bytes ];
         
         read( ds, interpolated, bytes, crc );

         s.interpolated = QByteArray( interpolated, bytes );

         delete [] interpolated;

         // Add the scan to the data
         data.scanData <<  s;
      }

      // Read the crc
      unsigned long read_crc;
      ds.readRawData( (char*) &read_crc , 4 );
      if ( crc != read_crc ) throw BADCRC;

   } catch( ioError error )
   {
      err = error;
   }

   f.close();
   return err;
}

void US_DataIO::read( QDataStream& ds, char* c, int len, ulong& crc )
{
   ds.readRawData( c, len );
   crc = US_Crc::crc32( crc, (uchar*) c, len );
}

