//! \file us_dataIO.cpp
#include <uuid/uuid.h>

#include <QDomDocument>

#include "us_dataIO.h"
#include "us_crc.h"
#include "us_math.h"
#include "us_matrix.h"

#include <uuid/uuid.h>

bool US_DataIO::readLegacyFile( const QString&  file, 
                                beckmanRawScan& data )
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
      
      if ( ! interference_data && p.size() > 2 ) 
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

   // Create and write a guid
   //uuid_t uuid;
   //uuid_generate( uuid );
   //write( ds, (char*) &uuid, 16, crc );
   write( ds, data.guid, 16, crc );

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
      foreach( r, s.readings )
      {
         min_radius = min( min_radius, r.d.radius );
         max_radius = max( max_radius, r.d.radius );

         p.min_data1 = min( p.min_data1, r.value );
         p.max_data1 = max( p.max_data1, r.value );

         p.min_data2 = min( p.min_data2, r.stdDev );
         p.max_data2 = max( p.max_data2, r.stdDev );
      }
   }

   // Write all types as little endian
   uchar c[ 4 ];
   qToLittleEndian( (quint16)( min_radius * 1000.0 ), c );
   write( ds, (char*)c, 2, crc );

   qToLittleEndian( (qint16)( max_radius * 1000.0 ), c );
   write( ds, (char*)c, 2, crc );

   // Distance between radius entries
   double r1    = data.scanData[ 0 ].readings[ 0 ].d.radius;
   double r2    = data.scanData[ 0 ].readings[ 1 ].d.radius;

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

   uf.f = (float) ( r2 - r1 );  //delta r
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );

   uf.f = (float) p.min_data1;  // minimum data value
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );
   
   uf.f = (float) p.max_data1;  // maximum data value
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );

   uf.f = (float) p.min_data2;  // minimum std deviation value
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );
   
   uf.f = (float) p.max_data2;  // maximum std deviation value
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );

   // Write out scan count
   qToLittleEndian( (quint16)data.scanData.size(), ui.u );
   write( ds, ui.c, 2, crc );

   // Loop for each scan
   foreach ( s, data.scanData )
      writeScan( ds, s, crc, p );

   qToLittleEndian( crc, ui.u ); // crc
   ds.writeRawData( ui.c, 4 );

   f.close();

   return OK;
}

void US_DataIO::writeScan( QDataStream&    ds, const scan&       data, 
                           quint32&       crc, const parameters& p )
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
   qToLittleEndian( (quint16)( ( data.wavelength - 180.0 ) * 100.0 ), ui.u );
   write( ds, ui.c, 2, crc );

   uf.f = (float) data.delta_r;     // delta r 
   qToLittleEndian( uf.u, ui.u );
   write( ds, ui.c, 4, crc );

   quint32 valueCount = data.readings.size(); // number of values
   qToLittleEndian( valueCount, ui.u );
   write( ds, ui.c, 4, crc );

   // Write readings
   double  delta  = ( p.max_data1 - p.min_data1 ) / 65535;
   double  delta2 = ( p.max_data2 - p.min_data2 ) / 65535;
   quint16 si;  

   bool    stdDev = ( p.min_data2 != 0.0 || p.max_data2 != 0.0 );
   reading r;

   foreach ( r, data.readings )
   {
      // reading
      si = (quint16) round( ( r.value - p.min_data1 ) / delta );
      qToLittleEndian( si, ui.u );
      write( ds, ui.c, 2, crc );

      // If applicable, write std deviation
      if ( stdDev )
      {
         si = (quint16) round( ( r.stdDev - p.min_data2 ) / delta2 );
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

int US_DataIO::readRawData( const QString& file, rawData& data )
{
   QFile f( file );
   if ( ! f.open( QIODevice::ReadOnly ) ) return CANTOPEN;
   QDataStream ds( &f );

   int      err = OK;
   quint32  crc = 0xffffffffUL;

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
      if ( version != format_version ) throw BAD_VERSION;

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
         char    c[ 2 ];
         quint16 I;
      } si;

      read( ds, si.c, 2, crc );
      double min_radius = qFromLittleEndian( si.I ) / 1000.0;

      read( ds, si.c, 2, crc );
      // Unused
      //double max_radius = qFromLittleEndian( si.I ) / 1000.0;

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
      for ( int i = 0 ; i < scan_count; i ++ )
      {
         read( ds, u1.c, 4, crc );
         if ( strncmp( u1.c, "DATA", 4 ) != 0 ) throw NOT_USDATA;

         scan s;
         
         // Temperature
         read( ds, u1.c, 4, crc );
         u2.I = qFromLittleEndian( u1.I );
         s.temperature = u2.f;

         // RPM
         read( ds, u1.c, 4, crc );
         u2.I = qFromLittleEndian( u1.I );
         s.rpm = u2.f;

         // Seconds
         read( ds, u1.c, 4, crc );
         s.seconds = qFromLittleEndian( u1.I );

         // Omega2t
         read( ds, u1.c, 4, crc );
         u2.I = qFromLittleEndian( u1.I );
         s.omega2t = u2.f;

         // Wavelength
         read( ds, si.c, 2, crc );
         s.wavelength = qFromLittleEndian( si.I ) / 100.0 + 180.0;

         // Delta_r
         read( ds, u1.c, 4, crc );
         u2.I = qFromLittleEndian( u1.I );
         s.delta_r = u2.f;

         // Reading count
         read( ds, u1.c, 4, crc );
         int valueCount = qFromLittleEndian( u1.I );

         // Get the readings
         double  radius  = min_radius;
         double  factor1 = ( max_data1 - min_data1 ) / 65535.0;
         double  factor2 = ( max_data2 - min_data2 ) / 65535.0;
         bool    stdDev  = ( min_data2 != 0.0 || max_data2 != 0.0 );

         for ( int j = 0; j < valueCount; j++ )
         {
            reading r;

            r.d.radius = radius;
            
            read( ds, si.c, 2, crc );
            r.value = qFromLittleEndian( si.I ) * factor1 + min_data1;

            if ( stdDev )
            {
               read( ds, si.c, 2, crc );
               r.stdDev = qFromLittleEndian( si.I ) * factor2 + min_data2;
            }
            else
               r.stdDev = 0.0;

            // Add the reading to the scan
            s.readings << r;
            
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
      quint32 read_crc;
      ds.readRawData( (char*) &read_crc , 4 );
      if ( crc != qFromLittleEndian( read_crc ) ) throw BADCRC;

   } catch( ioError error )
   {
      err = error;
   }

   f.close();
   return err;
}

void US_DataIO::read( QDataStream& ds, char* c, int len, quint32& crc )
{
   ds.readRawData( c, len );
   crc = US_Crc::crc32( crc, (uchar*) c, len );
}

int US_DataIO::readEdits( const QString& filename, editValues& parameters )
{
   QFile f( filename );
   if ( ! f.open( QIODevice::ReadOnly ) ) return CANTOPEN;
   QTextStream ds( &f );

   QXmlStreamReader xml( &f );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "identification" ) 
            ident ( xml, parameters );
         
         else if ( xml.name() == "run" ) 
            run( xml, parameters );
      }
   }

   bool error = xml.hasError();
   f.close();
   
   if ( error ) return BADXML;

   return OK;
}

void US_DataIO::ident( QXmlStreamReader& xml, editValues& parameters )
{
   while ( ! xml.atEnd() )
   {
      if ( xml.isEndElement()  &&  xml.name() == "identification" ) return;
     
      if ( xml.isStartElement()  &&  xml.name() == "runid" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.runID = a.value( "value" ).toString();
      }

      if ( xml.isStartElement()  &&  xml.name() == "uuid" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.uuid = a.value( "value" ).toString();
      }

      xml.readNext();
   }
}

void US_DataIO::run( QXmlStreamReader& xml, editValues& parameters )
{
   QXmlStreamAttributes a = xml.attributes();
   parameters.cell       = a.value( "cell"       ).toString();
   parameters.channel    = a.value( "channel"    ).toString();
   parameters.wavelength = a.value( "wavelength" ).toString();

   while ( ! xml.atEnd() )
   {
      if ( xml.isEndElement()  &&  xml.name() == "run" ) return;

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

void US_DataIO::params( QXmlStreamReader& xml, editValues& parameters )
{
   while ( ! xml.atEnd() )
   {
      if ( xml.isEndElement()  &&  xml.name() == "parameters" ) return;

      if ( xml.isStartElement()  &&  xml.name() == "meniscus" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.meniscus = a.value( "radius" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "plateau" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.plateau = a.value( "radius" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "air_gap" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.airGapLeft  = a.value( "left"  ).toString().toDouble();
         parameters.airGapRight = a.value( "right" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "baseline" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.baseline = a.value( "radius" ).toString().toDouble();
      }

      if ( xml.isStartElement()  &&  xml.name() == "data_range" )
      {
         QXmlStreamAttributes a = xml.attributes();
         parameters.rangeLeft  = a.value( "left"  ).toString().toDouble();
         parameters.rangeRight = a.value( "right" ).toString().toDouble();
      }

      xml.readNext();
   }
}

void US_DataIO::operations( QXmlStreamReader& xml, editValues& parameters )
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

void US_DataIO::excludes( QXmlStreamReader& xml, editValues& parameters )
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

void US_DataIO::do_edits( QXmlStreamReader& xml, editValues& parameters )
{
   while ( ! xml.atEnd() )
   {
      if ( xml.isEndElement()  &&  xml.name() == "edited" ) return;

      if ( xml.isStartElement()  &&  xml.name() == "edit" )
      {
         editedPoint e;
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
                         QVector< editedData >& data )
{
   QVector< rawData > raw;

   return loadData( directory, editFilename, data, raw );
}

int US_DataIO::loadData( const QString&       directory, 
                         const QString&       editFilename,
                         QVector< editedData >& data,
                         QVector< rawData    >& raw )
{
   // Determine raw file name by removing editID
   QString rawDataFile = editFilename;
   int index1 = editFilename.indexOf( "." );  
   int index2 = editFilename.indexOf( ".", index1 + 1 );
   rawDataFile.remove( index1, index2 - index1 );
   rawDataFile.replace( "xml", "auc" );
   
   // Get the raw data
   rawData d;
   int result = readRawData( directory + "/" + rawDataFile, d );
   if ( result != OK ) throw result;
   raw << d;
   qApp->processEvents();

   // Get the edit data
   editValues e;
   result = readEdits( directory + "/" + editFilename, e );
   if ( result != OK ) throw result;

   // Check for uuid match
   char uuid[ 37 ];
   uuid_unparse( (uchar*)d.guid, uuid );
   if ( QString( uuid ) != e.uuid ) throw NO_UUID_MATCH;

   // Apply the edits
   editedData ed;

   QStringList sl = editFilename.split( "." );

   ed.runID       = sl[ 0 ];
   ed.editID      = sl[ 1 ];
   ed.dataType    = sl[ 2 ];
   ed.cell        = sl[ 3 ];
   ed.channel     = sl[ 4 ];
   ed.wavelength  = sl[ 5 ];
   ed.description = d.description;
   ed.uuid        = e.uuid;
   ed.meniscus    = e.meniscus;
   ed.plateau     = e.plateau;
   ed.baseline    = e.baseline;
   ed.floatingData= e.floatingData;

   // Invert values before updating edited points
   if ( e.invert < 0 )
   {
      for ( int i = 0; i < ed.scanData.size(); i++ )
      {
         scan* s = &ed.scanData[ i ];
         for ( int j = 0; i < s->readings.size(); i++ )
         {
            s->readings[ j ].value *= e.invert;
         }
      }
   }

   // Update any edited points
   for ( int i = 0; i < e.editedPoints.length(); i++ )
   {
      int    scan   =      e.editedPoints[ i ].scan;
      int    index1 = (int)e.editedPoints[ i ].radius;
      double value  =      e.editedPoints[ i ].value;

      d.scanData[ scan ].readings[ index1 ].value = value;
   }

   // Do not copy excluded data or data outside the edit range
   for ( int i = 0; i < d.scanData.size(); i++ )
   {
      if ( e.excludes.contains( i ) ) continue;

      scan s;
      copyRange( e.rangeLeft, e.rangeRight, d.scanData[ i ], s );
     
      ed.scanData << s;
   }

   // Determine plateau values for each scan
   for ( int i = 0; i < ed.scanData.size(); i++ )
   {
      int point = index( ed.scanData[ i ], ed.plateau );
      ed.scanData[ i ].plateau = ed.scanData[ i ].readings[ point ].value;
   }

   if ( e.removeSpikes )
   {
      double smoothed_value;

      // For each scan
      for ( int i = 0; i < ed.scanData.size(); i++ )
      {
         scan* s   = &ed.scanData [ i ];
         int   end = s->readings.size();

         for ( int j = 0; j < 5; j++ ) // Beginning 5 points
         {
            if ( spike_check( *s, j, 0, 10, &smoothed_value ) )
               s->readings[ j ].value = smoothed_value;
         }

         for ( int j = 5; j < end - 4; j++ ) // Middle points
         {
            if ( spike_check( *s, j, j - 5, j + 5, &smoothed_value ) )
               s->readings[ j ].value = smoothed_value;
         }

         for ( int j = end - 4; j <= end; j++ ) // Last 5 points
         {
            if ( spike_check( *s, j, end - 10, end, &smoothed_value ) )
               s->readings[ j ].value = smoothed_value;
         }
      }
   }

   if ( e.noiseOrder > 0 )
   {
      QList< double > residuals = calc_residuals( e.noiseOrder, ed.scanData );
      
      for ( int i = 0; i < ed.scanData.size(); i++ )
      {
         for ( int j = 0; j <  ed.scanData[ i ].readings.size(); j++ )
            ed.scanData[ i ].readings[ j ].value -= residuals[ i ];
      }
   }

   data << ed;
   return OK;
}

void US_DataIO::copyRange ( double left, double right, 
                            const scan& orig, scan& dest )
{
   dest.temperature = orig.temperature;
   dest.rpm         = orig.rpm;
   dest.seconds     = orig.seconds;
   dest.omega2t     = orig.omega2t;
   dest.wavelength  = orig.wavelength;
   dest.delta_r     = orig.delta_r;

   for ( int i = index( orig, left ); i <= index( orig, right ); i++ )
   {
      dest.readings << orig.readings[ i ];
   }
}

bool US_DataIO::spike_check( const scan& s, int point, 
                             int start, int end, double* value )
{
   static double r[ 20 ];  // Spare room -- normally 10
   static double v[ 20 ];

   double* x = &r[ 0 ];
   double* y = &v[ 0 ];

   double  slope;
   double  intercept;
   double  sigma = 0.0;
   double  correlation;
   int     count = 0;

   const double threshold = 10.0;

   for ( int k = start; k <= end; k++ ) // For each point in the range
   {
      if ( k != point ) // Exclude the probed point from fit
      {
         r[ count ] = s.readings[ k ].d.radius;
         v[ count ] = s.readings[ k ].value;
         count++;
      }
   }

   US_Math::linefit( &x, &y, &slope, &intercept, &sigma, &correlation, count );

   // If there is more than a threshold-fold difference, it is a spike
   double val    =  s.readings[ point ].value;
   double radius =  s.readings[ point ].d.radius;

   if ( fabs( slope * radius + intercept - val ) > threshold * sigma )
   {
      // Interpolate
      *value = slope * radius + intercept;
      return true;
   }

   return false;  // Not a spike
}

QList< double > US_DataIO::calc_residuals( int                    order, 
                                           const QVector< scan >& sl )
{
   int scan_count = sl.size();

   double* coeffs              = new double[ order ];
   double* absorbance_integral = new double[ scan_count ];
   double* fit                 = new double[ scan_count ];
   double* scan_time           = new double[ scan_count ];;

   // Calculate the integral of each scan which is needed for the least-squares
   // polynomial fit to correct for radially invariant baseline noise. We also
   // keep track of the total integral at each point.

   for ( int i = 0; i < scan_count; i++ )
   {
      absorbance_integral[ i ] = 0;

      const US_DataIO::scan* s = &sl[ i ];
      int value_count          = s->readings.size();

      double delta_r = s->delta_r;

      // Integrate using trapezoid rule
      for ( int j = 1; j < value_count; j++ )
      {
         double avg = ( s->readings[ j ].value + s->readings[ j - 1 ].value ) / 2.0;
         absorbance_integral[ i ] += avg * delta_r;
      }
   }

   for ( int i = 0; i < scan_count; i++ )
      scan_time[ i ] =  sl[ i ].seconds;

   US_Matrix::lsfit( coeffs, scan_time, absorbance_integral, scan_count, order );

   QList< double > residuals;

   for ( int i = 0; i < scan_count; i++ )
   {
      fit[ i ] = 0;

      for ( int j = 0; j < order; j++ )
         fit[ i ] +=  coeffs[ j ] * pow( sl[ i ].seconds, j );

      residuals << absorbance_integral[ i ] - fit[ i ];
   }

   delete [] coeffs;
   delete [] absorbance_integral;
   delete [] fit;
   delete [] scan_time;

   return residuals;
}

// Returns index of radius value
int US_DataIO::index( const scan& s, double r )
{
   if ( r <= s.readings[ 0 ].d.radius ) return 0;
    
   int last = s.readings.size() - 1;
   if ( r >= s.readings[ last ].d.radius ) return last;

   for ( int i = 0; i < s.readings.size(); i++ )
   {
      if ( fabs( s.readings[ i ].d.radius - r ) < 5.0e-4 ) return i;
   }

   // Should never happen
   return -1;
}

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
      case NODATA    : return QObject::tr( "No legacy data files were found" );
      case BAD_VERSION 
         : return QObject::tr( "The file version is not supported" );
      case NO_UUID_MATCH
         : return QObject::tr( "UUIDs in raw data and edit data do not match" );
   }

   return QObject::tr( "Unknown error code" );
}
