//! \file us_process_convert.cpp

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math.h"
#include "us_process_convert.h"

// Generic constructor; establishes dialog
US_ProcessConvert::US_ProcessConvert( QWidget* parent ) : US_WidgetsDialog( parent, 0 )
{
   createDialog();
}

// Constructor to use for reading data 
US_ProcessConvert::US_ProcessConvert( QWidget* parent,
                                      QString dir,
                                      QList< US_DataIO::beckmanRaw >& rawLegacyData,
                                      QString& runType ) : US_WidgetsDialog( parent, 0 )
{
   if ( dir.isEmpty() ) return; 

   reset();

   createDialog();

   // Get legacy file names and set channels
   QDir d( dir, "*", QDir::Name, QDir::Files | QDir::Readable );
   d.makeAbsolute();
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /

   QStringList files = d.entryList( QDir::Files );
   runType = files[ 0 ].right( 3 ).left( 2 ).toUpper(); // 1st 2 chars of extention

   QStringList fileList;
   QStringList channels;
   QString f;

   foreach ( f, files )
   {
      // Look for a proper filename match:
      // Optional channel + 4 to 6 digits + dot + file type + cell number

      QRegExp rx( "^[A-J]?\\d{4,6}\\.(?:RA|RI|IP|FI|WA|WI)\\d$" );
      
      if ( rx.indexIn( f.toUpper() ) >= 0 )
      {
         fileList << f;
 
         // Parse the filtered file list to determine cells and channels
         QChar c = f.at( 0 ).toUpper();
         if ( c.isLetter() && ! channels.contains( c ) )
            channels << c;
      }
   }

   if ( channels.isEmpty() ) channels << "A";

   // Now read the data.
   lb_progress ->setText( tr( "Reading:" ) );
   progress    ->setRange( 0, fileList.size() - 1 );
   progress    ->setValue( 0 );
   progress    ->show();
   qApp        ->processEvents();

   for ( int i = 0; i < fileList.size(); i++ )
   {
      US_DataIO::beckmanRaw data;
      US_DataIO::readLegacyFile( dir + fileList[ i ], data );

      // Add channel
      QChar c = fileList[ i ].at( 0 );  // Get 1st character

      data.channel = ( c.isDigit() ) ? 'A' : c.toAscii();

      if ( runType == "RI" )                // Split out the two readings in RI data
      {
         US_DataIO::beckmanRaw data2 = data;  // Alter to store second dataset
         for ( int j = 0; j < data.readings.size(); j++ )
         {
            data2.readings[ j ].value  = data2.readings[ j ].stdDev;   // Reading 2 in here for RI
            data.readings [ j ].stdDev = 0.0;
            data2.readings[ j ].stdDev = 0.0;
         }

         data2.channel = 'B';

         rawLegacyData << data;
         rawLegacyData << data2;
      }

      else
      {
         rawLegacyData << data;
      }

      // Check to see if cancel button has been pressed
      if ( canceled )
      {
          rawLegacyData.clear();
          break;
      }


      progress->setValue( i );
      qApp    ->processEvents();
   }

   this->hide();
   this->close();
}

// Constructor to use for converting data 
US_ProcessConvert::US_ProcessConvert( QWidget* parent,
                                      QList< US_DataIO::beckmanRaw >& rawLegacyData,
                                      QVector< US_DataIO::rawData  >& rawConvertedData,
                                      QStringList& triples,
                                      QString runType,
                                      double tolerance,
                                      QList< double >& ss_limits // For RA data
                                    ) : US_WidgetsDialog( parent, 0 )
{
   reset();

   if ( runType == "RA"  && ss_limits.size() > 2 )
   {
      // We need to subdivide this data
      splitRAData( rawLegacyData, ss_limits );
   }

   setTriples( rawLegacyData, triples, runType, tolerance );

   US_DataIO::rawData newRawData;     // filtered legacy data in new raw format

   if ( triples.size() > 3 )
      createDialog();

   rawConvertedData.clear();

   if ( triples.size() == 1 )
   {
      convert( rawLegacyData, newRawData, triples[ 0 ], runType, tolerance );

      rawConvertedData << newRawData;
   }

   else
   {
      // Now convert the data.
      if ( triples.size() > 3 )
      {
         lb_progress ->setText( tr( "Converting:" ) );
         progress    ->setRange( 0, triples.size() - 1 );
         progress    ->setValue( 0 );
         progress    ->show();
         qApp        ->processEvents();
      }

      for ( int i = 0; i < triples.size(); i++ )
      {
         // Convert data for this cell / channel / wavelength
         convert( rawLegacyData, newRawData, triples[ i ], runType, tolerance );

         // and save it
         rawConvertedData << newRawData;

         // Check to see if cancel button has been pressed
         if ( canceled )
         {
            rawConvertedData.clear();
            break;
         }

         if ( triples.size() > 3 )
         {
            progress->setValue( i );
            qApp    ->processEvents();
         }

      }
   }

   qApp    ->processEvents();
   this->hide();
   this->close();
}

// Constructor to use for writing data 
US_ProcessConvert::US_ProcessConvert( QWidget* parent,
                                      int& status,
                                      QVector< US_DataIO::rawData >& rawConvertedData,
                                      US_ExpInfo::ExperimentInfo& ExpData,
                                      QStringList& triples,
                                      QString runType,
                                      QString runID,
                                      QString dirname
                                    ) : US_WidgetsDialog( parent, 0 )
{
   if ( rawConvertedData[ 0 ].scanData.empty() ) 
   {
      status = US_Convert::NODATA;
      return;
   }

   reset();

   createDialog();

   // Write the data. In this case not triples.size() - 1, because we are
   // going to consider the xml file as one file to write also
   lb_progress ->setText( tr( "Writing:" ) );
   progress    ->setRange( 0, triples.size() );
   progress    ->setValue( 0 );
   progress    ->show();
   qApp        ->processEvents();

   for ( int i = 0; i < triples.size(); i++ )
   {
      QString     triple     = triples[ i ];
      QStringList parts      = triple.split(" / ");

      QString     cell       = parts[ 0 ];
      QString     channel    = parts[ 1 ];
      QString     filename;

      if ( runType == "WA" )
      {
          double r       = parts[ 2 ].toDouble() * 1000.0;
          QString radius = QString::number( (int) round( r ) );
          filename       = runID      + "." 
                         + runType    + "." 
                         + cell       + "." 
                         + channel    + "." 
                         + radius     + ".auc";
      }

      else
      {
          QString wavelength = parts[ 2 ];
          filename           = runID      + "." 
                             + runType    + "." 
                             + cell       + "." 
                             + channel    + "." 
                             + wavelength + ".auc";
      }

      status = US_DataIO::writeRawData( dirname + filename, rawConvertedData[ i ] );

      if ( status !=  US_DataIO::OK ) break;
      
      progress->setValue( i );
      qApp    ->processEvents();
   }

   if ( status != US_Convert::OK )
   {
      // Try to delete the files and tell the user
      this->hide();
      this->close();
      return;
   }

   // Now try to write the xml file
   status = writeXmlFile( ExpData, triples, runType, runID, dirname );
   progress->setValue( triples.size() );
   qApp    ->processEvents();

   this->hide();
   this->close();
   return;
}

void US_ProcessConvert::convert( QList< US_DataIO::beckmanRaw >& rawLegacyData,
                                 US_DataIO::rawData& newRawData,
                                 QString triple,
                                 QString runType,
                                 double tolerance )
{
   // Convert the data into the UltraScan3 data structure
   QStringList parts      = triple.split(" / ");

   int         cell       = parts[ 0 ].toInt();
   char        channel    = parts[ 1 ].toAscii()[ 0 ];
   double      wavelength = parts[ 2 ].toDouble();

   /*
   qDebug() << cell       << " / "
            << channel    << " / "
            << wavelength;
   */

   QList< US_DataIO::beckmanRaw > ccwLegacyData;

   // Get a list of the data that matches the cell / channel / wl
   ccwLegacyData.clear();
   newRawData.scanData.clear();

   for ( int i = 0; i < rawLegacyData.size(); i++ )
   {
      US_DataIO::beckmanRaw data = rawLegacyData[ i ];

      if ( data.cell == cell       &&
           data.channel == channel &&
           fabs ( data.t.wavelength - wavelength ) < tolerance )
         ccwLegacyData << data;
   }

   // Sort the list according to time.  Use a simple bubble sort
   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      for ( int j = i + i; j < ccwLegacyData.size(); j++ )
      {
         if ( ccwLegacyData[ j ].seconds < ccwLegacyData[ i ].seconds ) 
            ccwLegacyData.swap( i, j );
      }
   }

   if ( ccwLegacyData.isEmpty() ) return ; 

   strncpy( newRawData.type, runType.toAscii().constData(), 2 );
   // GUID is done by US_DataIO.
   newRawData.cell        = cell;
   newRawData.channel     = channel;
   newRawData.description = ccwLegacyData[ 0 ].description;
   
   // Get the min and max radius
   double min_radius = 1.0e99;
   double max_radius = 0.0;

   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      double first = ccwLegacyData[ i ].readings[ 0 ].d.radius;

      uint   size  = ccwLegacyData[ i ].readings.size();
      double last  = ccwLegacyData[ i ].readings[ size - 1 ].d.radius; 

      min_radius = min( min_radius, first );
      max_radius = max( max_radius, last );
   }

   // Convert the scans
   
   // Set the distance between readings 
   double delta_r = ( runType == "IP" ) 
      ? ( max_radius - min_radius ) / ( ccwLegacyData[ 0 ].readings.size() - 1 )
      : 0.001;

   // qDebug() << "Current triple: " << triple << ' '
   //          << "delta_r: " << QString::number( delta_r, 'f', 6 );

   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      US_DataIO::scan s;
      s.temperature = ccwLegacyData[ i ].temperature;
      s.rpm         = ccwLegacyData[ i ].rpm;
      s.seconds     = ccwLegacyData[ i ].seconds;
      s.omega2t     = ccwLegacyData[ i ].omega2t;
      s.wavelength  = ccwLegacyData[ i ].t.wavelength;
      s.delta_r     = delta_r;

      // Readings here and interpolated array
      int radius_count = (int) round( ( max_radius - min_radius ) / delta_r ) + 1;
      int bitmap_size = ( radius_count + 7 ) / 8;
      uchar* interpolated = new uchar[ bitmap_size ];
      bzero( interpolated, bitmap_size );

      if ( runType == "IP" )
      {
         for ( int j = 0; j < radius_count; j++ )
         {
            US_DataIO::reading r;
            r.d.radius = ccwLegacyData[ i ].readings[ j ].d.radius;
            r.value    = ccwLegacyData[ i ].readings[ j ].value;
            r.stdDev   = 0.0;

            s.readings <<  r;
         }
      }
      else
      {
         /*
         There are two indexes needed here.  The new radius as iterated
         from min_radius to max_radius and the pointer to the current 
         scan readings is j.  

         The old scan reading is ccwLegacyData[ i ]->readings[ j ]

         If the current new radius is within 0.0003 of the i
         ccwLegacyData[ i ]->readings[ j ].d.radius
            copy ccwLegacyData[ i ]->readings[ j ].value into the new reading
            copy ccwLegacyData[ i ]->readings[ j ].stdDev into the new reading
            increment j

         If the current new radius is less than i
         ccwLegacyData[ i ]->readings[ 0 ].d.radius,
         then 
            copy ccwLegacyData[ i ]->readings[ 0 ].value into the new reading
            set the std dev to 0.0.
            set the interpolated flag
         
         If the current new radius is greater than 
         ccwLegacyData[ i ]->readings[ last() ].d.radius
            copy ccwLegacyData[ i ]->readings[ last ].value into the new reading
            set the std dev to 0.0.
            set the interpolated flag

         else
            interplate between ccwLegacyData[ i ]->readings[ j ] and 
                               ccwLegacyData[ i ]->readings[ j -1 ]
            set the std dev to 0.0.
            set the interpolated flag

         Append the new reading and continue.
         */

         double radius = min_radius;
         double r0     = ccwLegacyData[ i ].readings[ 0 ].d.radius;
         int    rCount = ccwLegacyData[ i ].readings.size();       
         double rLast  = ccwLegacyData[ i ].readings[ rCount - 1 ].d.radius;
         
         int    k      = 0;
         
         for ( int j = 0; j < radius_count; j++ )
         {
            US_DataIO::reading r;
            double             dr = 0.0;

            if ( k < rCount )
               dr = radius - ccwLegacyData[ i ].readings[ k ].d.radius;

            r.d.radius = radius;
            
            if ( dr > -3.0e-4   &&  k < rCount ) // A value
            {
               r.value  = ccwLegacyData[ i ].readings[ k ].value;
               r.stdDev = ccwLegacyData[ i ].readings[ k ].stdDev;
               k++;
            }
            else if ( radius < r0 ) // Before the first
            {
               r.value  = ccwLegacyData[ i ].readings[ 0 ].value;
               r.stdDev = 0.0;
               setInterpolated( interpolated, j );
            }
            else if ( radius > rLast  ||  k >= rCount ) // After the last
            {
               r.value  = ccwLegacyData[ i ].readings[ rCount - 1 ].value;
               r.stdDev = 0.0;
               setInterpolated( interpolated, j );
            }
            else  // Interpolate the value
            {
               double dv = ccwLegacyData[ i ].readings[ k     ].value - 
                           ccwLegacyData[ i ].readings[ k - 1 ].value;
               
               double dR = ccwLegacyData[ i ].readings[ k     ].d.radius -
                           ccwLegacyData[ i ].readings[ k - 1 ].d.radius;

               dr = radius - ccwLegacyData[ i ].readings[ k - 1 ].d.radius;

               r.value  = ccwLegacyData[ i ].readings[ k - 1 ].value + dr * dv / dR;
               r.stdDev = 0.0;

               setInterpolated( interpolated, j );
            }

            s.readings <<  r;
            radius += delta_r;
         }
      }
      s.interpolated = QByteArray( (char*)interpolated, bitmap_size );
      delete [] interpolated;

      newRawData.scanData <<  s ;
   }

   // Delete the bitmaps we allocated

   //for ( uint i = 0; i < newRawData.scanData.size(); i++ ) 
   //   delete newRawData.scanData[ i ].interpolated;

}

void US_ProcessConvert::splitRAData( QList< US_DataIO::beckmanRaw >& rawLegacyData,
                                     QList< double >& ss_limits )
{
   QList< US_DataIO::beckmanRaw > temp = rawLegacyData;
   rawLegacyData.clear();

   for ( int i = 0; i < temp.size(); i++ )
   {
      US_DataIO::beckmanRaw data = temp[ i ];

      // We are subdividing the scans for RA data
      US_DataIO::beckmanRaw data2 = data;
   
      US_DataIO::reading r;
   
      for ( int x = 1; x < ss_limits.size(); x++ )
      {
         data2.readings.clear();                  // Only need to alter readings for sub-datasets
   
         // Go through all the readings to see which ones to include
         for ( int y = 0; y < data.readings.size(); y++ )
         {
            if ( data.readings[ y ].d.radius > ss_limits[ x - 1 ] &&
               data.readings[ y ].d.radius < ss_limits[ x ] )
            {
               // Create the current dataset point
               r.d.radius = data.readings[ y ].d.radius;
               r.value    = data.readings[ y ].value;
               r.stdDev   = data.readings[ y ].stdDev;
               data2.readings << r;
            }
   
         }
   
         rawLegacyData << data2;                  // Send the current data subset
         data2.channel++;                         // increment the channel letter
      }
   }
}

void US_ProcessConvert::setTriples( QList< US_DataIO::beckmanRaw >& rawLegacyData,
                                    QStringList& triples,
                                    QString runType,
                                    double tolerance )
{
   // Wavelength data is handled differently here
   if ( runType == "WA" )
      setCcrTriples( rawLegacyData, triples, tolerance );
   else
      setCcwTriples( rawLegacyData, triples, tolerance );

}

void US_ProcessConvert::setCcwTriples( QList< US_DataIO::beckmanRaw >& rawLegacyData,
                                       QStringList& triples,
                                       double tolerance )
{
   // Most triples are ccw
   triples.clear();

   // Get wavelengths
   QStringList wavelengths;

   for ( int i = 0; i < rawLegacyData.size(); i++ )
   {
      QString wl = QString::number( rawLegacyData[ i ].t.wavelength, 'f', 1 );
      wavelengths << wl;
   }

   // Merge wavelengths
   wavelengths.sort();

/*
   qDebug() << "Wavelengths found:";
   for ( int i = 0; i < wavelengths.size(); i++ )
      qDebug() << wavelengths[ i ];
*/

   QList< QList< double > > modes;
   QList< double >          mode;
   
   for ( int i = 0; i < wavelengths.size(); i++ )
   {
      double wl = wavelengths[ i ].toDouble();

      if ( ! mode.empty()  &&  fabs( mode.last() - wl ) > tolerance )
      {
         modes << mode;
         mode.clear();
      }

      mode << wl;   
   }

   if ( mode.size() > 0 ) modes << mode;

   // Now we have a list of modes.  
   // Average each list and round to the closest integer.
   
/*
   qDebug() << "Modes:";
   for ( int i = 0; i < modes.size(); i++ )
   {
      qDebug() << "i: " << i;
      for ( int j = 0; j < modes[ i ].size(); j++ )
         qDebug() << "  " << modes[ i ][ j ];
   }
*/

   QList< double > wl_average;

   for ( int i = 0; i < modes.size(); i++ )
   {
      double sum = 0.0;

      for ( int j = 0; j < modes[ i ].size(); j++ ) sum += modes[ i ][ j ]; 

      wl_average << (double) round( 10.0 * sum / modes[ i ].size() ) / 10.0;
   }

/*
   qDebug() << "Wavelength average:";
   for ( int i = 0; i < wl_average.size(); i++ )
      qDebug() << "  " << wl_average[ i ];
*/

   // Now that we have a more reliable list of wavelengths, let's
   // find out the possible cell, channel, and wavelength combinations
   for ( int i = 0; i < rawLegacyData.size(); i++ )
   {
      QString cell       = QString::number( rawLegacyData[ i ].cell );
      QString channel    = QString( rawLegacyData[ i ].channel );
      double wl          = rawLegacyData[ i ].t.wavelength;
      QString wavelength = "0";

      // find the average wavelength
      for ( int j = 0; j < wl_average.size(); j++ )
      {
         if ( fabs( wl_average[ j ] - wl ) < tolerance )
         {
            wavelength = QString::number( (int) round( wl_average[ j ] ) );
            break;
         }
      }

      QString t = cell + " / " + channel + " / " + wavelength;
      if (! triples.contains( t ) ) triples << t;
   }
}

void US_ProcessConvert::setCcrTriples( QList< US_DataIO::beckmanRaw >& rawLegacyData,
                                       QStringList& triples,
                                       double tolerance )
{
   // First of all, wavelength triples are ccr.
   triples.clear();

   // Now get the radius values
   QStringList radii;

   for ( int i = 0; i < rawLegacyData.size(); i++ )
   {
      QString r = QString::number( rawLegacyData[ i ].t.radius, 'f', 1 );
      radii << r;
   }

   // Merge radii

   radii.sort();

/*
   qDebug() << "Radius values found:";
   for ( int i = 0; i < radii.size(); i++ )
      qDebug() << radii[ i ];
*/

   QList< QList< double > > modes;
   QList< double >          mode;
   
   for ( int i = 0; i < radii.size(); i++ )
   {
      double r = radii[ i ].toDouble();

      if ( ! mode.empty()  &&  fabs( mode.last() - r ) > tolerance )
      {
         modes << mode;
         mode.clear();
      }

      mode << r;   
   }

   if ( mode.size() > 0 ) modes << mode;

   // Now we have a list of modes.  
   // Average each list and round to the closest integer.
   
/*
   qDebug() << "Modes:";
   for ( int i = 0; i < modes.size(); i++ )
   {
      qDebug() << "i: " << i;
      for ( int j = 0; j < modes[ i ].size(); j++ )
         qDebug() << "  " << modes[ i ][ j ];
   }
*/

   QList< double > r_average;

   for ( int i = 0; i < modes.size(); i++ )
   {
      double sum = 0.0;

      for ( int j = 0; j < modes[ i ].size(); j++ ) sum += modes[ i ][ j ]; 

      r_average << (double) round( 10.0 * sum / modes[ i ].size() ) / 10.0;
   }

/*
   qDebug() << "Radius average:";
   for ( int i = 0; i < r_average.size(); i++ )
      qDebug() << "  " << r_average[ i ];
*/

   // Now that we have a more reliable list of radii, let's
   // find out the possible cell, channel, and radius combinations
   for ( int i = 0; i < rawLegacyData.size(); i++ )
   {
      QString cell       = QString::number( rawLegacyData[ i ].cell );
      QString channel    = QString( rawLegacyData[ i ].channel );
      double r           = rawLegacyData[ i ].t.radius;
      QString radius     = "0";

      // find the average radius
      for ( int j = 0; j < r_average.size(); j++ )
      {
         if ( fabs( r_average[ j ] - r ) < tolerance )
         {
            radius = QString::number( r_average[ j ] );
            break;
         }
      }

      QString t = cell + " / " + channel + " / " + radius;
      if (! triples.contains( t ) ) triples << t;
   }
}

int US_ProcessConvert::writeXmlFile( US_ExpInfo::ExperimentInfo& ExpData,
                                     QStringList& triples,
                                     QString runType,
                                     QString runID,
                                     QString dirname )
{ 
   if ( ExpData.invID == 0 ) return( US_Convert::NOXML ); 

   QString writeFile = runID      + "." 
                     + runType    + ".xml";
   QFile file( dirname + writeFile );
   if ( !file.open( QIODevice::WriteOnly | QIODevice::Text) )
   {
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Cannot open file " ) + dirname + writeFile );
      return( US_Convert::CANTOPEN );
   }

   QXmlStreamWriter xml;
   xml.setDevice( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD("<!DOCTYPE US_Scandata>");
   xml.writeStartElement("US_Scandata");
   xml.writeAttribute("version", "1.0");

   // elements
   xml.writeStartElement( "experiment" );
   xml.writeAttribute   ( "id", QString::number( ExpData.expID ) );
   xml.writeAttribute   ( "type", ExpData.expType );

      xml.writeStartElement( "investigator" );
      xml.writeAttribute   ( "id", QString::number( ExpData.invID ) );
      xml.writeEndElement  ();
      
      xml.writeStartElement( "project" );
      xml.writeAttribute   ( "id", QString::number( ExpData.projectID ) );
      xml.writeEndElement  ();
      
      xml.writeStartElement( "lab" );
      xml.writeAttribute   ( "id", QString::number( ExpData.labID ) );
      xml.writeEndElement  ();
      
      xml.writeStartElement( "instrument" );
      xml.writeAttribute   ( "id", QString::number( ExpData.instrumentID ) );
      xml.writeEndElement  ();
      
      xml.writeStartElement( "operator" );
      xml.writeAttribute   ( "id", QString::number( ExpData.operatorID ) );
      xml.writeEndElement  ();

      xml.writeStartElement( "rotor" );
      xml.writeAttribute   ( "id", QString::number( ExpData.rotorID ) );
      xml.writeEndElement  ();

      xml.writeStartElement( "guid" );
      xml.writeAttribute   ( "id", "replace with GUID" );
      xml.writeEndElement  ();

      // loop through the following for c/c/w combinations
      for ( int i = 0; i < ExpData.triples.size(); i++ )
      {
         US_ExpInfo::TripleInfo t = ExpData.triples[ i ];

         QString triple         = triples[ t.tripleID ];
         QStringList parts      = triple.split(" / ");

         QString     cell       = parts[ 0 ];
         QString     channel    = parts[ 1 ];
         QString     wl         = parts[ 2 ];

         xml.writeStartElement( "dataset" );
         xml.writeAttribute   ( "cell", cell );
         xml.writeAttribute   ( "channel", channel );
         xml.writeAttribute   ( "wavelength", wl );

            xml.writeStartElement( "guid" );
            xml.writeAttribute   ( "id", "replace with GUID" );
            xml.writeEndElement  ();

            xml.writeStartElement( "centerpiece" );
            xml.writeAttribute   ( "id", QString::number( t.centerpiece ) );
            xml.writeEndElement  ();

            xml.writeStartElement( "buffer" );
            xml.writeAttribute   ( "id", QString::number( t.bufferID ) );
            xml.writeEndElement  ();

            xml.writeStartElement( "analyte" );
            xml.writeAttribute   ( "id", QString::number( t.analyteID ) );
            xml.writeEndElement  ();

         xml.writeEndElement   ();
      }

   xml.writeTextElement ( "date", ExpData.date );
   xml.writeTextElement ( "runTemp", ExpData.runTemp );
   xml.writeTextElement ( "label", ExpData.label );
   xml.writeTextElement ( "comments", ExpData.comments );
   xml.writeTextElement ( "centrifugeProtocol", ExpData.centrifugeProtocol );

   xml.writeEndElement(); // US_Scandata
   xml.writeEndDocument();

   if ( ExpData.triples.size() != triples.size() )
      return( US_Convert::PARTIAL_XML );

   return( US_Convert::OK );
}


void US_ProcessConvert::createDialog( void )
{
   reset();

   setModal( true );

   setWindowTitle( tr( "Progress..." ) );
   setPalette( US_GuiSettings::frameColor() );

   // Everything will be in the main layout
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;

   // Progress bar
   QLabel* lb_placeholder = new QLabel();
   main -> addWidget( lb_placeholder, row, 0, 1, 2 );

   lb_progress = us_label( tr( "Progress:" ) , -1 );
   lb_progress->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   main       ->addWidget( lb_progress, row, 0 );

   progress = us_progressBar( 0, 100, 0 );
   progress -> reset();
   progress -> setVisible( true );
   main     -> addWidget( progress, row++, 1 );

   // Some pushbuttons
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
   buttons->addWidget( pb_cancel );

   main ->setColumnMinimumWidth( 1, 200 );
   main ->addLayout( buttons, row++, 1 );
   this ->show();
   qApp    ->processEvents();
}

void US_ProcessConvert::setInterpolated ( unsigned char* bitmap, int location )
{
   int byte = location / 8;
   int bit  = location % 8;

   bitmap[ byte ] |= 1 << ( 7 - bit );
}

void US_ProcessConvert::reset( void )
{
   canceled  = false;
}

void US_ProcessConvert::cancel( void )
{
   this->hide();
   canceled = true;
}

