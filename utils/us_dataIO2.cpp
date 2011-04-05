//! \file us_dataIO2.cpp
#include <QDomDocument>

#include "us_dataIO2.h"
#include "us_crc.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_util.h"

bool US_DataIO2::readLegacyFile( const QString&  file, 
                                 BeckmanRawScan& data )
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

      RawReading r;

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

int US_DataIO2::writeRawData( const QString& file, RawData& data )
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

   // Write cell
   write( ds, (const char*)&data.cell, 1, crc );

   // Write channel
   write( ds, (const char*)&data.channel, 1, crc );

   // Create and write a guid
   write( ds, data.rawGUID, 16, crc );

   // Write description
   char desc[ 240 ];
   memset( desc, '\0', sizeof desc );  // bzero is not defined in WIN32

   QByteArray d = data.description.toLatin1();
   strncpy( desc, d.data(), sizeof desc );
   write( ds, desc, sizeof desc, crc );

   // Find min and max radius, data, and std deviation
   // First the radii
   double min_radius =  1.0e99;
   double max_radius = -1.0e99;

   for ( int i = 0; i < data.x.size(); i++ )
   {
      //min_radius = min( min_radius, data.x[ i ].radius );
      //max_radius = max( max_radius, data.x[ i ].radius );
      min_radius = min( min_radius, data.radius( i ) );
      max_radius = max( max_radius, data.radius( i ) );
   }

   // Now the data and SD
   Parameters p;

   p.min_data1       =  1.0e99;
   p.max_data1       = -1.0e99;
   p.min_data2       =  1.0e99;
   p.max_data2       = -1.0e99;

   Scan    s;
   Reading r;

   foreach( s, data.scanData )
   {
      foreach( r, s.readings )
      {
         p.min_data1 = min( p.min_data1, r.value );
         p.max_data1 = max( p.max_data1, r.value );

         p.min_data2 = min( p.min_data2, r.stdDev );
         p.max_data2 = max( p.max_data2, r.stdDev );
      }
   }

   // Distance between radius entries
   //double r1    = data.x[ 0 ].radius;
   //double r2    = data.x[ 1 ].radius;
   double r1    = data.radius( 0 );
   double r2    = data.radius( 1 );

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

void US_DataIO2::writeScan( QDataStream&    ds, const Scan&       data, 
                            quint32&       crc, const Parameters& p )
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
   Reading r;

   foreach ( r, data.readings )
   {
      // reading
      si = (quint16) qRound( ( r.value - p.min_data1 ) / delta );
      qToLittleEndian( si, ui.u );
      write( ds, ui.c, 2, crc );

      // If applicable, write std deviation
      if ( stdDev )
      {
         si = (quint16) qRound( ( r.stdDev - p.min_data2 ) / delta2 );
         qToLittleEndian( si, ui.u );
         write( ds, ui.c, 2, crc );
      }
   }

   // Write interpolated flags
   int flagSize = ( valueCount + 7 ) / 8;
   write( ds, data.interpolated.data(), flagSize, crc );
}

void US_DataIO2::write( QDataStream& ds, const char* c, int len, quint32& crc )
{
   ds.writeRawData( c, len );
   crc = US_Crc::crc32( crc, (unsigned char*) c, len );
}

int US_DataIO2::readRawData( const QString& file, RawData& data )
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
      //double max_radius;  //unused

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
      int valueCount;
      for ( int i = 0 ; i < scan_count; i ++ )
      {
         read( ds, u1.c, 4, crc );
         if ( strncmp( u1.c, "DATA", 4 ) != 0 ) throw NOT_USDATA;

         Scan s;
         
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
         valueCount = qFromLittleEndian( u1.I );

         // Get the readings
         double  factor1 = ( max_data1 - min_data1 ) / 65535.0;
         double  factor2 = ( max_data2 - min_data2 ) / 65535.0;
         bool    stdDev  = ( min_data2 != 0.0 || max_data2 != 0.0 );

         for ( int j = 0; j < valueCount; j++ )
         {
            Reading r;

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

      // Calculate the radius vector
      data.x.clear();
      double  radius  = min_radius;
      
      for ( int j = 0; j < valueCount; j++ )
      {
         data.x << XValue( radius );
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

   f.close();
   return err;
}

void US_DataIO2::read( QDataStream& ds, char* c, int len, quint32& crc )
{
   ds.readRawData( c, len );
   crc = US_Crc::crc32( crc, (uchar*) c, len );
}

int US_DataIO2::readEdits( const QString& filename, EditValues& parameters )
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

         else if ( xml.name() == "experiment" )
         {
            QXmlStreamAttributes a = xml.attributes();
            parameters.expType     = a.value( "type" ).toString();
         }
      }
   }

   bool error = xml.hasError();
   f.close();
   
   if ( error ) return BADXML;

   return OK;
}

void US_DataIO2::ident( QXmlStreamReader& xml, EditValues& parameters )
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

void US_DataIO2::run( QXmlStreamReader& xml, EditValues& parameters )
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

void US_DataIO2::params( QXmlStreamReader& xml, EditValues& parameters )
{
   parameters.speedData.clear();
   bool isEquil = ( parameters.expType == "Equilibrium" );
   int  spx     = 0;

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

      xml.readNext();
   }
}

void US_DataIO2::operations( QXmlStreamReader& xml, EditValues& parameters )
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

void US_DataIO2::excludes( QXmlStreamReader& xml, EditValues& parameters )
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

void US_DataIO2::do_edits( QXmlStreamReader& xml, EditValues& parameters )
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

int US_DataIO2::loadData( const QString&         directory, 
                          const QString&         editFilename,
                          QVector< EditedData >& data )
{
   QVector< RawData > raw;

   return loadData( directory, editFilename, data, raw );
}

int US_DataIO2::loadData( const QString&  directory, 
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

int US_DataIO2::loadData( const QString&         directory, 
                          const QString&         editFilename,
                          QVector< EditedData >& data,
                          QVector< RawData    >& raw )
{
   // Determine raw file name by removing editID
   QString rawDataFile = editFilename;
   int index1 = editFilename.indexOf( "." );  
   int index2 = editFilename.indexOf( ".", index1 + 1 );
   rawDataFile.remove( index1, index2 - index1 );
   rawDataFile.replace( "xml", "auc" );
   
   // Get the raw data
   RawData d;
   ioError result = (ioError)readRawData( directory + "/" + rawDataFile, d );
   if ( result != OK ) throw result;
   raw << d;
   qApp->processEvents();

   // Get the edit data
   EditValues ev;
   result = (ioError)readEdits( directory + "/" + editFilename, ev );
   if ( result != OK ) throw result;

   // Check for uuid match
   QString rawGuid = US_Util::uuid_unparse( (uchar*)d.rawGUID );
   if ( rawGuid != ev.dataGUID ) throw NO_GUID_MATCH;

   // Apply the edits
   EditedData ed;

   QStringList sl = editFilename.split( "." );

   ed.runID       = sl[ 0 ];
   ed.editID      = sl[ 1 ];
   ed.dataType    = sl[ 2 ];
   ed.cell        = sl[ 3 ];
   ed.channel     = sl[ 4 ];
   ed.wavelength  = sl[ 5 ];
   ed.description = d.description;
   ed.expType     = ev.expType;
   ed.dataGUID    = ev.dataGUID;
   ed.editGUID    = ev.editGUID;
   ed.meniscus    = ev.meniscus;
   ed.plateau     = ev.plateau;
   ed.baseline    = ev.baseline;
   ed.floatingData= ev.floatingData;

   if ( ed.expType == "Equilibrium" )
      ed.speedData << ev.speedData;

   // Invert values before updating edited points
   if ( ev.invert < 0 )
   {
      for ( int i = 0; i < d.scanData.size(); i++ )
      {
         Scan* s = &d.scanData[ i ];
         for ( int j = 0; i < s->readings.size(); i++ )
         {
            s->readings[ j ].value *= ev.invert;
         }
      }
   }

   // Update any edited points
   for ( int i = 0; i < ev.editedPoints.length(); i++ )
   {
      int    scan   =      ev.editedPoints[ i ].scan;
      int    index1 = (int)ev.editedPoints[ i ].radius;
      double value  =      ev.editedPoints[ i ].value;

      d.scanData[ scan ].readings[ index1 ].value = value;
   }

   // Update for interference data
   if ( ed.dataType == "IP" )
   {
      adjust_interference( d, ev );  // rawData, editValues
      calc_integral      ( d, ev );
   }

   // Do not copy excluded data or data outside the edit range
   for ( int i = 0; i < d.scanData.size(); i++ )
   {
      if ( ev.excludes.contains( i ) ) continue;

      Scan s;
      copyRange( ev.rangeLeft, ev.rangeRight, d.scanData[ i ], s, d.x );
     
      ed.scanData << s;
   }

   // Only need to copy radius vector for given range once
   copyxRange( ev.rangeLeft, ev.rangeRight, d.scanData[ 0 ], d.x, ed.x );

   // Determine plateau values for each scan
   for ( int i = 0; i < ed.scanData.size(); i++ )
   {
      int point = index( ed.scanData[ i ], ed.x, ed.plateau );
      ed.scanData[ i ].plateau = ed.scanData[ i ].readings[ point ].value;
   }

   if ( ev.removeSpikes )
   {
      double smoothed_value;

      // For each scan
      for ( int i = 0; i < ed.scanData.size(); i++ )
      {
         Scan* s      = &ed.scanData [ i ];
         int   start  = 0;
         int   end    = s->readings.size() - 1;      // index of the last one

         for ( int j = start; j < end; j++ )
         {
            if ( spike_check( *s, ed.x, j, start, end, &smoothed_value ) )
            {
               s->readings[ j ].value = smoothed_value;

               // If previous consecututive points are interpolated, then
               // redo them
               int           index = j - 1;
               unsigned char cc    = s->interpolated[ index / 8 ];

               while ( cc & ( 1 << ( 7 - index % 8 ) ) )
               {
                  if ( spike_check( *s, ed.x, index, start, end, &smoothed_value ) )
                     s->readings[ index ].value = smoothed_value;

                  index--;
                  cc = s->interpolated[ index / 8 ];
               }
            }
         }
      }
   }

   if ( ev.noiseOrder > 0 )
   {
      QList< double > residuals = calc_residuals( ev.noiseOrder, ed.scanData );
      
      for ( int i = 0; i < ed.scanData.size(); i++ )
      {
         for ( int j = 0; j <  ed.scanData[ i ].readings.size(); j++ )
            ed.scanData[ i ].readings[ j ].value -= residuals[ i ];
      }
   }

   data << ed;
   return OK;
}

void US_DataIO2::adjust_interference( RawData& data, const EditValues& ev )
{
   // Find first scan
   for ( int i = 0; i < data.scanData.size(); i++ )
   {
      if ( ev.excludes.contains( i ) ) continue;

      US_DataIO2::Scan* s = &data.scanData[ i ];

      int r_left  = US_DataIO2::index( *s, data.x, ev.airGapLeft );
      int r_right = US_DataIO2::index( *s, data.x, ev.airGapRight );
      double sum  = 0.0;

      for ( int k = r_left; k <= r_right; k++ )
         sum += data.value( i, k );

      double average = sum / ( r_right - r_left + 1 );

      for ( int j = i + 1; j < data.scanData.size(); j++ )
      {
         // Get average difference between gap in first included
         // scan and each subsequent scan

         s = &data.scanData[ j ];
         sum = 0.0;

         for ( int k = r_left; k <= r_right; k++ )
            sum += data.value( j, k ) - data.value( i, k );

         double delta = sum / ( r_right - r_left + 1 );

         // Subtract average difference from all points
         for ( int k = 0; k < s->readings.size(); k++ )
            s->readings[ k ].value -= delta;
      }

      for ( int j = i; j < data.scanData.size(); j++ )
      {
         s = &data.scanData[ j ];

         for ( int k = 0; k < s->readings.size(); k++ )
            s->readings[ k ].value -= average;
      }

      return; // After first included scan
   }
}

void US_DataIO2::calc_integral( RawData& data, const EditValues& e )
{
   // This function gets a little tricky because we have raw data, but
   // want to adjust int the range identified by the user *and* 
   // not use excluded scans.

   int               index = 0;
   QVector< int    > included;
   QVector< double > integral;

   for ( int scan = 0; scan < data.scanData.size(); scan++ )
   {
      if ( e.excludes.contains( scan ) ) continue;

      included << scan;

      integral << 0.0;

      // Arbitrarily add 1000 fringes to each reading value
      // to make sure we don't sum negatives.  This is not needed from a
      // mathematical point of view, but physically, negative fringes 
      // do not make sense.
      US_DataIO2::Scan* s = &data.scanData[ scan ];

      int r_left  = US_DataIO2::index( *s, data.x, e.rangeLeft );
      int r_right = US_DataIO2::index( *s, data.x, e.rangeRight );

      for ( int r = r_left; r <= r_right; r++ )
         integral[ index ] += data.value( scan, r ) + 1000.0;

      index++;
   }

   // Integral fringe shifts contribute exactly ( r_right - r_left + 1 ) 
   // to the integral, since we use unity stepsize in integral calculation.

   for ( int scan = 1; scan < included.size(); scan++ )
   {
      US_DataIO2::Scan* s = &data.scanData[ included[ scan ] ];

      int    r_left    = US_DataIO2::index( *s, data.x, e.rangeLeft );
      int    r_right   = US_DataIO2::index( *s, data.x, e.rangeRight );
      int    current   = included[ scan ];
      int    previous  = included[ scan - 1 ];
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
      for ( int r = r_left; r <= r_right; r++ )
         s->readings[ r ].value += (double) position;

      double diff1 = integral[ previous ] - integral[ current ];

      double diff2 = integral[ previous ] - ( integral[ current ] + points );

      // The scan is one fringe too low
      if ( fabs( diff2 / diff1 ) < e.gapTolerance )
      {
         // add one fringe to all readings
         for ( int r = r_left; r <= r_right; r++ )
            s->readings[ r ].value += 1.0;

         // Update integral for this scan
         integral[ current ] += points;
      }
   }
}

void US_DataIO2::copyRange ( double left, double right, const Scan& orig, Scan& dest, 
                             const QVector< XValue >& origx )
{
   dest.temperature = orig.temperature;
   dest.rpm         = orig.rpm;
   dest.seconds     = orig.seconds;
   dest.omega2t     = orig.omega2t;
   dest.wavelength  = orig.wavelength;
   dest.delta_r     = orig.delta_r;

   int index_L      = index( orig, origx, left );
   int index_R      = index( orig, origx, right );

   dest.interpolated.resize( ( index_R - index_L ) / 8 + 1 );
   
   int current_bit = 0;

   for ( int i = index_L; i <= index_R; i++ )
   {
      // Copy the concentration readings
      dest.readings << orig.readings[ i ];
      
      // Set the interpolated bits as needed
      unsigned char old_bit = 1 << ( 7 - i % 8 );

      if ( ( orig.interpolated[ i / 8 ] & old_bit ) != 0 )
      {
         unsigned char new_bit = 1 << ( 7 - current_bit % 8 );
         unsigned char byte    = dest.interpolated[ current_bit / 8 ];
         dest.interpolated[ current_bit / 8 ] = byte | new_bit;
      }

      current_bit++;
   }
}

void US_DataIO2::copyxRange ( double left, double right, const Scan& orig, 
                              const QVector< XValue >& origx, QVector< XValue >& destx )
{
   for ( int i = index( orig, origx, left ); i <= index( orig, origx, right ); i++ )
   {
      destx << XValue( origx[ i ].radius );
   }
}

bool US_DataIO2::spike_check( const US_DataIO2::Scan&  s, 
                              const QVector< XValue >& xval,
                              int point, int start, int end, double* value )
{
   static double r[ 20 ];  // Spare room -- normally 10
   static double v[ 20 ];

   double*       x = &r[ 0 ];
   double*       y = &v[ 0 ];

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
   // Decide to searich left or right first.
   if ( point < ( start + end ) / 2 )
   {
      index = point - 1;

      // Search left for five points if we can
      while ( count < 5 )
      {
         if ( index < start ) break;

         cc           = s.interpolated[ index / 8 ];
         interpolated = cc & ( 1 << ( 7 - index % 8 ) );

         if ( ! interpolated )
         {
            r[ count ] = xval      [ index ].radius;
            v[ count ] = s.readings[ index ].value;
            count++;
         }

         index--;
      }

      // Now search right
      index = point + 1;

      while ( count < 10 )
      {
         if ( index >= end ) break;

         cc           = s.interpolated[ index / 8 ];
         interpolated = cc & ( 1 << ( 7 - index % 8 ) );

         if ( ! interpolated )
         {
            r[ count ] = xval       [ index ].radius;
            v[ count ] = s.readings[ index ].value;
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

         cc           = s.interpolated[ index / 8 ];
         interpolated = cc & ( 1 << ( 7 - index % 8 ) );

         if ( ! interpolated )
         {
            r[ count ] = xval      [ index ].radius;
            v[ count ] = s.readings[ index ].value;
            count++;
         }

         index++;
      }

      // Now search left
      index = point - 1;

      while ( count < 10 )
      {
         if ( index < start ) break;

         cc           = s.interpolated[ index / 8 ];
         interpolated = cc & ( 1 << ( 7 - index % 8 ) );

         if ( ! interpolated )
         {
            r[ count ] = xval      [ index ].radius;
            v[ count ] = s.readings[ index ].value;
            count++;
         }

         index--;
      }
   }

   US_Math2::linefit( &x, &y, &slope, &intercept, &sigma, &correlation, count );

   // If there is too much difference, it is a spike
   double current   = s.readings[ point ].value;
   double radius    = xval[ point ].radius;
   double projected = slope * radius + intercept;

   if ( fabs( projected - current ) > threshold * sigma )
   {
      // Interpolate
      *value = projected;
      return true;
   }

   return false;  // Not a spike
}

QList< double > US_DataIO2::calc_residuals( int                    order, 
                                            const QVector< Scan >& sl )
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

      const Scan* s    = &sl[ i ];
      int value_count  = s->readings.size();

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
int US_DataIO2::index( const Scan& s, const QVector< XValue >& x, double r )
{
   if ( r <= x[ 0 ].radius ) return 0;
    
   int last = s.readings.size() - 1;
   if ( r >= x[ last ].radius ) return last;

   for ( int i = 0; i < s.readings.size(); i++ )
   {
      if ( fabs( x[ i ].radius - r ) < 5.0e-4 ) return i;
   }

   // Should never happen
   return -1;
}

QString US_DataIO2::errorString( int code )
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
      case NO_GUID_MATCH
         : return QObject::tr( "GUIDs in raw data and edit data do not match" );
   }

   return QObject::tr( "Unknown error code" );
}
