//! \file us_process_convert.cpp

#include <uuid/uuid.h>

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math2.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_process_convert.h"
#include "us_convertio.h"

US_ConvertProgressBar::US_ConvertProgressBar( QWidget* parent ) 
                     : US_WidgetsDialog( parent, 0 )
{
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
   progress -> setVisible( true );
   main     -> addWidget( progress, row++, 1 );

   // Some pushbuttons
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
   buttons->addWidget( pb_cancel );

   main ->setColumnMinimumWidth( 1, 200 );
   main ->addLayout( buttons, row++, 1 );

   visible  = true;
   canceled = false;
   this ->show();
   qApp    ->processEvents();
}

void US_ConvertProgressBar::setLabel( QString text )
{
   lb_progress ->setText( text );
}

void US_ConvertProgressBar::setRange( int min, int max )
{
   this->max   = max;
   progress    ->setRange( min, max );
}

void US_ConvertProgressBar::setValue( int value )
{
   if ( value < max && visible )
   {
      progress    ->setValue( value );
      progress    ->show();
      qApp        ->processEvents();
   }

   else if ( ! canceled )
   {
      // Then it's run its course and needs to close
      this->hide();
      this->close();
   }

   // else we just continue to hide until finished
}

void US_ConvertProgressBar::display( bool visible )
{
   this->visible = visible;

   if ( ! visible )
      this->hide();
}


bool US_ConvertProgressBar::isActive( void )
{
   return( ! canceled );
}

void US_ConvertProgressBar::cancel( void )
{
   canceled = true;

   this->setVisible( false );
}

// Generic constructor; establishes dialog
US_ProcessConvert::US_ProcessConvert( QWidget* parent ) : US_ConvertProgressBar( parent )
{
}

void US_ProcessConvert::readLegacyData( 
     QString                              dir,
     QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
     QString&                             runType ) 
{
   if ( dir.isEmpty() ) return; 

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

   // Set up the progress bar
   this ->setLabel( tr( "Reading:" ) );
   this ->setRange( 0, fileList.size() );
   this ->setValue( 0 );

   // Now read the data.
   for ( int i = 0; i < fileList.size(); i++ )
   {
      US_DataIO2::BeckmanRawScan data;
      US_DataIO2::readLegacyFile( dir + fileList[ i ], data );

      // Add channel
      QChar c = fileList[ i ].at( 0 );  // Get 1st character

      data.channel = ( c.isDigit() ) ? 'A' : c.toAscii();

      if ( runType == "RI" )                // Split out the two readings in RI data
      {
         US_DataIO2::BeckmanRawScan data2 = data;  // Alter to store second dataset
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
      if ( ! this->isActive() )
      {
          rawLegacyData.clear();
          break;
      }


      this->setValue( i );
   }
}

void US_ProcessConvert::convertLegacyData( 
     QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
     QVector< US_DataIO2::RawData  >&     rawConvertedData,
     QStringList&                         triples,
     QString                              runType,
     double                               tolerance,
     QList< double >&                     ss_limits // For RA data
     ) 
{

   if ( runType == "RA"  && ss_limits.size() > 2 )
   {
      // We need to subdivide this data
      splitRAData( rawLegacyData, ss_limits );
   }

   setTriples( rawLegacyData, triples, runType, tolerance );

   US_DataIO2::RawData newRawData;     // filtered legacy data in new raw format

   if ( triples.size() <= 3 )
      this->setVisible( false );

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
         this ->setLabel( tr( "Converting:" ) );
         this ->setRange( 0, triples.size() - 1 );
         this ->setValue( 0 );
      }

      for ( int i = 0; i < triples.size(); i++ )
      {
         // Convert data for this cell / channel / wavelength
         convert( rawLegacyData, newRawData, triples[ i ], runType, tolerance );

         // and save it
         rawConvertedData << newRawData;

         // Check to see if cancel button has been pressed
         if ( ! this->isActive() )
         {
            rawConvertedData.clear();
            break;
         }

         if ( triples.size() > 3 )
         {
            this->setValue( i );
         }

      }
   }

   this->hide();
   this->close();
}

void US_ProcessConvert::writeConvertedData(
     int& status,
     QVector< US_DataIO2::RawData >& rawConvertedData,
     US_ExpInfo::ExperimentInfo& ExpData,
     QStringList& triples,
     QList< int >& tripleMap,
     QVector< US_Convert::Excludes >& allExcludes,
     QString runType,
     QString runID,
     QString dirname
     )
{
   if ( rawConvertedData[ 0 ].scanData.empty() ) 
   {
      status = US_Convert::NODATA;
      return;
   }

   // Write the data. In this case not triples.size() - 1, because we are
   // going to consider the xml file as one file to write also
   this ->setLabel( tr( "Writing:" ) );
   this ->setRange( 0, tripleMap.size() );
   this ->setValue( 0 );

   for ( int i = 0; i < tripleMap.size(); i++ )
   {
      int         ndx        = tripleMap[ i ];
      QString     triple     = triples[ ndx ];
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

      // Calculate and save the guid for this triple
      uuid_t uuid;
      uuid_generate( uuid );
      strncpy( rawConvertedData[ ndx ].rawGUID, (char*) uuid, 16 );
      strncpy( ExpData.triples [ ndx ].tripleGUID, (char*) uuid, 16 );
 
      // Save the filename of this triple
      ExpData.triples[ ndx ].tripleFilename = filename;

      // Create a copy of the current dataset so we can alter it
      US_DataIO2::RawData  currentData     = rawConvertedData[ ndx ];
      US_Convert::Excludes currentExcludes = allExcludes     [ ndx ];

      // Now recopy scans, except for excluded ones
      currentData.scanData.clear();
      QVector< US_DataIO2::Scan > sourceScans = rawConvertedData[ ndx ].scanData;
      for ( int j = 0; j < sourceScans.size(); j++ )
      {
         if ( ! currentExcludes.contains( j ) )
            currentData.scanData << sourceScans[ j ];  // copy this scan
      }

      // Now write altered dataset
      status = US_DataIO2::writeRawData( dirname + filename, currentData );

      if ( status !=  US_DataIO2::OK ) break;
      
      this->setValue( i );
   }

   if ( status != US_Convert::OK )
   {
      // Try to delete the files and tell the user
      this->hide();
      this->close();
      return;
   }

   // Now try to write the xml file
   status = US_ConvertIO::writeXmlFile( 
            ExpData, triples, tripleMap, runType, runID, dirname );

   if ( status == US_Convert::CANTOPEN )
   {
      QString writeFile = runID      + "." 
                        + runType    + ".xml";
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Cannot open write file: " ) + dirname + writeFile );
   }

   else if ( status != US_Convert::OK )
   {
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Error: " ) + status );
   }

   this->setValue( tripleMap.size() );

   this->hide();
   this->close();
}

void US_ProcessConvert::reloadUS3Data(
     QString dir,
     QVector< US_DataIO2::RawData >& rawConvertedData,
     US_ExpInfo::ExperimentInfo& ExpData,
     QStringList& triples,
     QList< int >& tripleMap,
     QString& runType ,
     QString runID
     )
{
   this->display( false );      // too fast for progress bar

   rawConvertedData.clear();

   QStringList nameFilters = QStringList( "*.auc" );

   QDir d( dir );

   QStringList files =  d.entryList( nameFilters, 
         QDir::Files | QDir::Readable, QDir::Name );

   if ( files.size() == 0 )
   {
      QMessageBox::warning( this,
            tr( "No Files Found" ),
            tr( "There were no files of the form *.auc\n"  
                "found in the specified directory." ) );
      return;
   }

   // Set up cell / channel / wavelength combinations
   tripleMap.clear();
   triples.clear();
   ExpData.clear();
   for ( int i = 0; i < files.size(); i++ )
   {
      tripleMap << i;

      QStringList part = files[ i ].split( "." );
      QString triple   = part[ 2 ] + " / " + part[ 3 ] + " / " + part[ 4 ];
      runType          = part[ 1 ];
      triples << triple;

      US_ExpInfo::TripleInfo t;
      ExpData.triples << t;
   }

   // Read all data
   QString file;
   foreach ( file, files )
   {
      QString filename = dir + file;
      US_DataIO2::RawData data;
      
      int result = US_DataIO2::readRawData( filename, data );
      if ( result != US_DataIO2::OK )
      {
         QMessageBox::warning( this,
            tr( "UltraScan Error" ),
            tr( "Could not read data file.\n" ) 
            + US_DataIO2::errorString( result ) + "\n" + filename );
         return;
      }

      rawConvertedData << data;
      data.scanData.clear();
   }

   if ( rawConvertedData.isEmpty() )
   {
      QMessageBox::warning( this,
         tr( "UltraScan Error" ),
         tr( "Could not read any data file." ) );
      return;
   }

   // Now try to read the xml file
   int status = US_ConvertIO::readXmlFile( 
                ExpData, triples, runType, runID, dir );

   if ( status == US_Convert::CANTOPEN )
   {
      QString readFile = runID      + "." 
                       + runType    + ".xml";
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Cannot open read file: " ) + dir + readFile );
   }

   else if ( status == US_Convert::BADXML )
   {
      QString readFile = runID      + "." 
                       + runType    + ".xml";
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Improper XML in read file: " ) + dir + readFile );
   }

   else if ( status == US_Convert::BADGUID )
   {
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "One or more GUID's were not found in the database " ) );
   }

   else if ( status != US_Convert::OK )
   {
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Unknown error: " ) + status );
   }

}

void US_ProcessConvert::convert( 
     QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
     US_DataIO2::RawData&                 newRawData,
     QString                              triple,
     QString                              runType,
     double                               tolerance )
{
   // Convert the data into the UltraScan3 data structure
   QStringList parts      = triple.split(" / ");

   int         cell       = parts[ 0 ].toInt();
   char        channel    = parts[ 1 ].toAscii()[ 0 ];
   double      wavelength = parts[ 2 ].toDouble();

   QList< US_DataIO2::BeckmanRawScan > ccwLegacyData;

   // Get a list of the data that matches the cell / channel / wl
   ccwLegacyData.clear();
   newRawData.scanData.clear();

   for ( int i = 0; i < rawLegacyData.size(); i++ )
   {
      US_DataIO2::BeckmanRawScan data = rawLegacyData[ i ];

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
   // GUID is done by us_convert.
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

   // Calculate the radius vector
   int radius_count = (int) round( ( max_radius - min_radius ) / delta_r ) + 1;
   double radius = min_radius;
   for ( int j = 0; j < radius_count; j++ )
   {
      newRawData.x << US_DataIO2::XValue( radius );
      radius += delta_r;
   }

   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      US_DataIO2::Scan s;
      s.temperature = ccwLegacyData[ i ].temperature;
      s.rpm         = ccwLegacyData[ i ].rpm;
      s.seconds     = ccwLegacyData[ i ].seconds;
      s.omega2t     = ccwLegacyData[ i ].omega2t;
      s.wavelength  = ccwLegacyData[ i ].t.wavelength;
      s.delta_r     = delta_r;

      // Readings here and interpolated array
      int bitmap_size = ( radius_count + 7 ) / 8;
      uchar* interpolated = new uchar[ bitmap_size ];
      bzero( interpolated, bitmap_size );

      if ( runType == "IP" )
      {
         for ( int j = 0; j < radius_count; j++ )
         {
            US_DataIO2::Reading r;
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
            US_DataIO2::Reading r;
            double             dr = 0.0;

            if ( k < rCount )
               dr = radius - ccwLegacyData[ i ].readings[ k ].d.radius;

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

}

void US_ProcessConvert::splitRAData( 
     QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
     QList< double >&                     ss_limits )
{
   QList< US_DataIO2::BeckmanRawScan > temp = rawLegacyData;
   rawLegacyData.clear();

   for ( int i = 0; i < temp.size(); i++ )
   {
      US_DataIO2::BeckmanRawScan data = temp[ i ];

      // We are subdividing the scans for RA data
      US_DataIO2::BeckmanRawScan data2 = data;
   
      US_DataIO2::RawReading r;
   
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

void US_ProcessConvert::setTriples( 
     QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
     QStringList&                         triples,
     QString                              runType,
     double                               tolerance )
{
   // Wavelength data is handled differently here
   if ( runType == "WA" )
      setCcrTriples( rawLegacyData, triples, tolerance );
   else
      setCcwTriples( rawLegacyData, triples, tolerance );

}

void US_ProcessConvert::setCcwTriples( 
     QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
     QStringList&                         triples,
     double                               tolerance )
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
   QList< double > wl_average;

   for ( int i = 0; i < modes.size(); i++ )
   {
      double sum = 0.0;

      for ( int j = 0; j < modes[ i ].size(); j++ ) sum += modes[ i ][ j ]; 

      wl_average << (double) round( 10.0 * sum / modes[ i ].size() ) / 10.0;
   }

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

void US_ProcessConvert::setCcrTriples( 
     QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
     QStringList&                        triples,
     double                              tolerance )
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
   QList< double > r_average;

   for ( int i = 0; i < modes.size(); i++ )
   {
      double sum = 0.0;

      for ( int j = 0; j < modes[ i ].size(); j++ ) sum += modes[ i ][ j ]; 

      r_average << (double) round( 10.0 * sum / modes[ i ].size() ) / 10.0;
   }

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

void US_ProcessConvert::setInterpolated ( unsigned char* bitmap, int location )
{
   int byte = location / 8;
   int bit  = location % 8;

   bitmap[ byte ] |= 1 << ( 7 - bit );
}
