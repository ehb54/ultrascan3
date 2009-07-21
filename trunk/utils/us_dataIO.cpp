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
      
      data.readings.push_back( r );
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

   for ( uint i = 0; i < data.scanData.size(); i++ )
   {
      for ( uint j = 0; j < data.scanData[ i ].values.size(); j++ )
      {
         reading* r = &data.scanData[ i ].values[ j ];

         min_radius = min( min_radius, r->d.radius );
         max_radius = max( max_radius, r->d.radius );

         p.min_data1 = min( p.min_data1, r->value );
         p.max_data1 = max( p.max_data1, r->value );

         p.min_data2 = min( p.min_data2, r->stdDev );
         p.max_data2 = max( p.max_data2, r->stdDev );
      }
   }

   write( ds, (char*) &min_radius,  4, crc );
   write( ds, (char*) &max_radius,  4, crc );

   // Distance between radius entries
   double r1 = data.scanData[ 0 ].values[ 0 ].d.radius;
   double r2 = data.scanData[ 0 ].values[ 1 ].d.radius;
   float delta = (float) ( r2 - r1 );
   write( ds, (char*) &delta, 4, crc );


   write( ds, (char*) &p.min_data1, 4, crc );
   write( ds, (char*) &p.max_data1, 4, crc );
   write( ds, (char*) &p.min_data2, 4, crc );
   write( ds, (char*) &p.max_data2, 4, crc );


   // Write out scan count
   short int count = (short int) data.scanData.size();
   write( ds, (char*) &count, 2, crc );

   // Loop for each scan
   for ( uint i=0; i < data.scanData.size(); i++ )
   {
      writeScan( ds, data.scanData[ i ], crc, p );
   }

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

   short int r = (short int) data.rpm;
   write( ds, (char*) &r, 2, crc );

   int s = (int) data.seconds;   
   write( ds, (char*) &s, 4, crc );

   float o = (float) data.omega2t;
   write( ds, (char*) &o, 4, crc );

   float w = (float) data.wavelength;
   write( ds, (char*) &w, 4, crc );

   // Write reading
   double    delta = ( p.max_data1 - p.min_data1 ) / 65536;
   double    v;   // value
   short int si;  // short int

   for ( unsigned int i = 0; i < data.values.size(); i++ )
   {
      v  = data.values[ i ].value;
      si = (short int) ( ( v - p.min_data1 ) / delta );
      write( ds, (char*) &si, 2, crc );
   }

   // If applicable, write std deviation
   if ( p.min_data2 == 0.0 && p.max_data2 == 0.0 )
   {
      delta = ( p.max_data2 - p.min_data2 ) / 65536;
      
      for ( unsigned int i = 0; i < data.values.size(); i++ )
      {
         v = data.values[ i ].stdDev;
         si = (short int) ( ( v - p.min_data2 ) / delta );
         write( ds, (char*) &si, 2, crc );
      }
   }

   // Write interpolated flags
   int flagSize = ( data.values.size() + 7 ) / 8;
   write( ds, (char*) data.interpolated, flagSize, crc );
}

void US_DataIO::write( QDataStream& ds, char* c, int len, unsigned long& crc )
{
   ds.writeRawData(                       c, len );
   US_Crc::crc32  ( crc, (unsigned char*) c, len );
}
