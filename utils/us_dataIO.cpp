//! \file us_dataIO.cpp
#include "us_dataIO.h"

bool US_DataIO::readLegacyFile( const QString&     file, 
                                struct beckmanRaw& data )
{
   // Open the file for reading
   QFile f( file );
   f.open( QIODevice::ReadOnly | QIODevice::Text );
   QTextStream ts( &f );

   // Read the description
   data.description = ts.readLine();

   // Read scan parameters
   QString     s = ts.readLine();
   QStringList p = s.split( " ", QString::SkipEmptyParts );

   if ( p.size() < 8 ) return false;

   data.type          = p[ 0 ].toAscii()[ 0 ];  // I P R W F
   data.cell          = p[ 1 ].toInt();
   data.temperature   = p[ 2 ].toFloat();
   data.rpm           = p[ 3 ].toFloat();
   data.seconds       = p[ 4 ].toInt();
   data.omega2t       = p[ 5 ].toFloat();
   data.t.wavelength  = p[ 6 ].toFloat();
   data.count         = p[ 7 ].toInt();


   // Read radius, data, and standard deviation
   data.readings.clear();
   bool interference_data = ( data.type == 'P' );

   while ( ! ts.atEnd() )
   {
      s = ts.readLine();

      p = s.split( " ", QString::SkipEmptyParts );

      struct reading r;

      r.d.radius = p[ 0 ].toFloat();
      r.value    = p[ 1 ].toFloat();
      
      if ( ! interference_data ) 
         r.stdDev  = p[ 2 ].toFloat();
      
      data.readings.push_back( r );
   }

   f.close();
   return true;
}

