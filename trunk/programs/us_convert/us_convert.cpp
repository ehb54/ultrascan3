//! \file us_convert.cpp

#include <QApplication>

#include "us_convert.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"

//! \brief Main program for us_convert. Loads translators and starts
//         the class US_Convert.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_Convert w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_Convert::US_Convert() : US_Widgets()
{
   setWindowTitle( tr( "Convert Legacy Raw Data" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* settings = new QGridLayout;

   int row = 0;

   // Row 1
   QPushButton* pb_load = us_pushbutton( tr( "Load Legacy Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   settings->addWidget( pb_load, row++, 0, 1, 2 );

   QLabel* lb_dir = us_label( tr( "Directory:" ) );
   settings->addWidget( lb_dir, row++, 0 );

   // Row 2
   le_dir = us_lineedit( "", 1 );
   le_dir->setReadOnly( true );
   settings->addWidget( le_dir, row++, 0, 1, 2 );

   // Cell / Channel / Wavelength

   // Row 3
   QLabel* lb_triple = us_label( tr( "Cell / Channel / Wavelength" ), -1 );
   settings->addWidget( lb_triple, row, 0 );

   cb_triple = us_comboBox();
   cb_triple->setInsertPolicy( QComboBox::InsertAlphabetically );
   connect( cb_triple, SIGNAL( activated( int ) ), SLOT( changeCcw( int ) ) );
   settings->addWidget( cb_triple, row++, 1 );

   // Scan Controls

   // Row 4
   QLabel* lb_scan = us_banner( tr( "Scan Controls" ) );
   settings->addWidget( lb_scan, row++, 0, 1, 2 );

   // Row 5
   QLabel* lb_from = us_label( tr( "Scan Focus from:" ), -1 );
   lb_from->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   settings->addWidget( lb_from, row, 0 );

   ct_from = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   ct_from->setStep( 1 );
   settings->addWidget( ct_from, row++, 1 );

   // Row 6
   QLabel* lb_to = us_label( tr( "Scan Focus to:" ), -1 );
   lb_to->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   settings->addWidget( lb_to, row, 0 );

   ct_to = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   ct_to->setStep( 1 );
   settings->addWidget( ct_to, row++, 1 );

   // Exclude and Include pushbuttons
   // Row 7
   pb_exclude = us_pushbutton( tr( "Exclude Single Scan" ), false );
   connect( pb_exclude, SIGNAL( clicked() ), SLOT( exclude_one() ) );
   settings->addWidget( pb_exclude, row, 0 );
   
   pb_excludeRange = us_pushbutton( tr( "Exclude Scan Range" ), false );
   connect( pb_excludeRange, SIGNAL( clicked() ), SLOT( exclude_range() ) );
   settings->addWidget( pb_excludeRange, row++, 1 );

   // Row 8
   pb_include = us_pushbutton( tr( "Include All" ), false );
   connect( pb_include, SIGNAL( clicked() ), SLOT( include() ) );
   settings->addWidget( pb_include, row++, 0, 1, 2 );

   // Write pushbuttons
   // Row 9
   QBoxLayout* writeButtons = new QHBoxLayout;
   pb_write = us_pushbutton( tr( "Write Current Data" ), false );
   connect( pb_write, SIGNAL( clicked() ), SLOT( write() ) );
   writeButtons->addWidget( pb_write );

   pb_writeAll = us_pushbutton( tr( "Write All Data" ), false );
   connect( pb_writeAll, SIGNAL( clicked() ), SLOT( writeAll() ) );
   writeButtons->addWidget( pb_writeAll );
   settings->addLayout( writeButtons, row++, 0, 1, 2 );

   // Progress bar
   // Row 10
   progress = us_progressBar( 0, 100, 0 );
   progress -> reset();
   progress -> setVisible( false );
   settings -> addWidget( progress, row++, 0, 1, 2 );

   // Standard pushbuttons

   QBoxLayout* buttons = new QHBoxLayout;

   // Row 11
   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_accept );

   // Plot layout for the right side of window
   QBoxLayout* plot = new US_Plot( data_plot,
                                   tr( "Absorbance Data" ),
                                   tr( "Radius (in cm)" ), 
                                   tr( "Absorbance" ) );

   data_plot->setMinimumSize( 600, 400 );
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );

   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );

   // Now let's assemble the page
   QHBoxLayout* main = new QHBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QVBoxLayout* left     = new QVBoxLayout;
   QSpacerItem* spacer   = new QSpacerItem( 20, 20, 
                           QSizePolicy::Minimum, QSizePolicy::Expanding );

   left->addLayout( settings );
   left->addItem( spacer );
   left->addLayout( buttons );
   main->addLayout( left );
   
   main->addLayout( plot );
}

void US_Convert::reset( void )
{
   cb_triple    ->clear();

   le_dir->setText( "" );

   pb_exclude     ->setEnabled( false );
   pb_excludeRange->setEnabled( false );
   pb_include     ->setEnabled( false );
   pb_write       ->setEnabled( false );
   pb_writeAll    ->setEnabled( false );

   ct_from->disconnect();
   ct_from->setMinValue( 0 );
   ct_from->setMaxValue( 0 );
   ct_from->setValue   ( 0 );

   ct_to->disconnect();
   ct_to->setMinValue( 0 );
   ct_to->setMaxValue( 0 );
   ct_to->setValue   ( 0 );

   // Clear any data structures
   legacyData.clear();
   includes.clear();
   ccwLegacyData.clear();
   newRawData.scanData.clear();
   triples.clear();

   data_plot->detachItems();
   pick     ->disconnect();
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   grid = us_grid( data_plot );
   data_plot->replot();

}

// User pressed the load data button
void US_Convert::load( void )
{
   read();                // Read the legacy data

/*
   // Display the data that was read
   for ( int i = 0; i < legacyData.size(); i++ )
   {
      beckmanRaw d = legacyData[ i ];

      qDebug() << d.description;
      qDebug() << d.type         << " "
               << d.cell         << " "
               << d.temperature  << " "
               << d.rpm          << " "
               << d.seconds      << " "
               << d.omega2t      << " "
               << d.t.wavelength << " "
               << d.count;

      for ( int j = 0; j < d.readings.size(); j++ )
      {
         if ( i != legacyData.size() - 1 ) continue;

         reading r = d.readings[ j ];

         QString line = QString::number(r.d.radius, 'f', 4 )    + " "
                      + QString::number(r.value, 'E', 5 )       + " "
                      + QString::number(r.stdDev, 'E', 5 );
         qDebug() << line;
      }
   }

*/

   convert();             // Now convert the data to the new format

   plot_current();        // And show the user what we have

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   // Ok to enable some buttons now
   pb_include ->setEnabled( true );
   pb_write   ->setEnabled( true );
   pb_writeAll->setEnabled( true );
}

void US_Convert::read( void )
{
   // Ask for data directory
   QString dir = QFileDialog::getExistingDirectory( this, 
         tr( "Raw Data Directory" ),
         US_Settings::dataDir(),
         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

   if ( dir.isEmpty() ) return; 

   reset();
   le_dir  ->setText( dir );

   // Get legacy file names
   QDir        d( dir, "*", QDir::Name, QDir::Files | QDir::Readable );
   d.makeAbsolute();
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /

   QStringList files = d.entryList( QDir::Files );
   QStringList fileList;

   // Read into local structures
   
   for ( int i = 0; i < files.size(); i++ )
   {
      // Look for a proper filename match:
      // Optional channel + 4 to 6 digits + dot + file type + cell number

      QRegExp rx( "^[A-J]?\\d{4,6}\\.(?:RA|RI|IP|FI|WA|WI)\\d" );
      QString f = files[ i ].toUpper();
      
      if ( rx.indexIn( f ) < 0 ) continue;

      fileList << files[ i ];
   }

   runType = files[ 0 ].right( 3 ).left( 2 ).toUpper(); // 1st 2 chars of extention

   QStringList channels;

   // Parse the filtered file list to determine cells and channels
   for ( int i = 0; i < fileList.size(); i++ )
   {
      QChar c = fileList[ i ].at( 0 );
      if ( c.isLetter() && ! channels.contains( c ) ) channels << c;
   }

   if ( channels.isEmpty() ) channels << "A";

   // Now read the data.

   for ( int i = 0; i < fileList.size(); i++ )
   {
      beckmanRaw data;
      US_DataIO::readLegacyFile( dir + fileList[ i ], data );

      // Add channel
      QChar c = fileList[ i ].at( 0 );  // Get 1st character

      data.channel = ( c.isDigit() ) ? 'A' : c.toAscii();

      legacyData << data;
   }

   // Get wavelengths
   
   QStringList wavelengths;

   for ( int i = 0; i < legacyData.size(); i++ )
   {
      QString wl = QString::number( legacyData[ i ].t.wavelength, 'f', 1 );
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

      if ( ! mode.empty()  &&  fabs( mode.last() - wl ) > 5.0 )
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
      for ( int j = 0; j < mode.size(); j++ )
         qDebug() << "  " << modes[ i ][ j ];
   }
*/

   QList< int > wl_average;

   for ( int i = 0; i < modes.size(); i++ )
   {
      double sum = 0.0;

      for ( int j = 0; j < modes[ i ].size(); j++ ) sum += modes[ i ][ j ]; 

      wl_average << (int) round( sum / modes[ i ].size() );
   }

/*
   qDebug() << "Wavelength average:";
   for ( int i = 0; i < wl_average.size(); i++ )
      qDebug() << "  " << wl_average[ i ];
*/

   // Now that we have a more reliable list of wavelengths, let's
   // find out the possible cell, channel, and wavelength combinations
   for ( int i = 0; i < legacyData.size(); i++ )
   {
      QString cell       = QString::number( legacyData[ i ].cell );
      QString channel    = QString( legacyData[ i ].channel );
      double wl          = legacyData[ i ].t.wavelength;
      QString wavelength = "0";

      // find the average wavelength
      for ( int j = 0; j < wl_average.size(); j++ )
      {
         if ( fabs( wl_average[ j ] - wl ) < 5.0 )
         {
            wavelength = QString::number( wl_average[ j ] );
            break;
         }
      }

      QString t = cell + " / " + channel + " / " + wavelength;
      if (! triples.contains( t ) ) triples << t;
   }

   cb_triple->addItems( triples );
   currentTriple = 0;
}

void US_Convert::convert( void )
{
   // Convert the data into the UltraScan3 data structure
   QString triple         = triples[ currentTriple ];
   QStringList parts      = triple.split(" / ");

   int         cell       = parts[ 0 ].toInt();
   char        channel    = parts[ 1 ].toAscii()[ 0 ];
   double      wavelength = parts[ 2 ].toDouble();

/*
   qDebug() << cell       << " / "
            << channel    << " / "
            << wavelength;
*/

   // Get a list of the data that matches the cell / channel / wl
   ccwLegacyData.clear();
   newRawData.scanData.clear();
   for ( int i = 0; i < legacyData.size(); i++ )
   {
      if ( legacyData[ i ].cell == cell       &&
           legacyData[ i ].channel == channel &&
           fabs ( legacyData[ i ].t.wavelength - wavelength ) < 5.0 )
         ccwLegacyData << &legacyData[ i ];
   }

   // Sort the list according to time.  Use a simple bubble sort
   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      for ( int j = i + i; j < ccwLegacyData.size(); j++ )
      {
         if ( ccwLegacyData[ j ]->seconds < ccwLegacyData[ i ]->seconds ) ccwLegacyData.swap( i, j );
      }
   }

   if ( ccwLegacyData.isEmpty() ) return ; 

   strncpy( newRawData.type, runType.toAscii().constData(), 2 );
   // GUID is done by US_DataIO.
   newRawData.cell        = cell;
   newRawData.channel     = channel;
   newRawData.description = ccwLegacyData[ 0 ]->description;
   
   // Get the min and max radius
   double min_radius = 1.0e99;
   double max_radius = 0.0;

   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      double first = ccwLegacyData[ i ]->readings[ 0 ].d.radius;

      uint   size  = ccwLegacyData[ i ]->readings.size();
      double last  = ccwLegacyData[ i ]->readings[ size - 1 ].d.radius; 

      min_radius = min( min_radius, first );
      max_radius = max( max_radius, last );
   }

   // Convert the scans
   
   // Set the distance between readings to a constant for now
   double delta_r = 0.001;

   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      scan s;
      s.temperature = ccwLegacyData[ i ]->temperature;
      s.rpm         = ccwLegacyData[ i ]->rpm;
      s.seconds     = ccwLegacyData[ i ]->seconds;
      s.omega2t     = ccwLegacyData[ i ]->omega2t;
      s.wavelength  = ccwLegacyData[ i ]->t.wavelength;

      // Enable progress bar
      progress ->setRange( 0, ccwLegacyData.size() );
      progress ->setValue( 0 );
      progress ->setVisible( true );

      // Readings here and interpolated array
      int radius_count = (int) round( ( max_radius - min_radius ) / delta_r ) + 1;
      int bitmap_size = ( radius_count + 7 ) / 8;
      s.interpolated = (unsigned char*) malloc( bitmap_size );
      bzero( s.interpolated, bitmap_size );

      /*
         There are two indexes needed here.  The new radius as iterated
         from min_radius to max_radius and the pointer to the current 
         scan readings is j.  

         The old scan reading is ccwLegacyData[ i ]->values[ j ]

         If the current new radius is within 0.0003 of the ccwLegacyData[ i ]->values[ j ].d.radius
            copy ccwLegacyData[ i ]->values[ j ].value into the new reading
            copy ccwLegacyData[ i ]->values[ j ].stdDev into the new reading
            increment j

         If the current new radius is less than ccwLegacyData[ i ]->values[ 0 ].d.radius,
         then 
            copy ccwLegacyData[ i ]->values[ 0 ].value into the new reading
            set the std dev to 0.0.
            set the interpolated flag
         
         If the current new radius is greater than ccwLegacyData[ i ]->values[ last() ].d.radius
            copy ccwLegacyData[ i ]->values[ last ].value into the new reading
            set the std dev to 0.0.
            set the interpolated flag

         else
            interplate between ccwLegacyData[ i ]->values[ j ] and ccwLegacyData[ i ]->values[ j -1 ]
            set the std dev to 0.0.
            set the interpolated flag

         Append the new reading and continue.
      */

      double radius = min_radius;
      double r0     = ccwLegacyData[ i ]->readings[ 0 ].d.radius;
      int    rCount = ccwLegacyData[ i ]->readings.size();       
      double rLast  = ccwLegacyData[ i ]->readings[ rCount - 1 ].d.radius;
      
      int    k      = 0;
      
      for ( int j = 0; j < radius_count; j++ )
      {
         reading r;
         double  dr = radius - ccwLegacyData[ i ]->readings[ k ].d.radius;

         r.d.radius = radius;
         
         if ( dr > -3.0e-4   &&  k < rCount ) // A value
         {
            r.value  = ccwLegacyData[ i ]->readings[ k ].value;
            r.stdDev = ccwLegacyData[ i ]->readings[ k ].stdDev;
            k++;
         }
         else if ( radius < r0 ) // Before the first
         {
            r.value  = ccwLegacyData[ i ]->readings[ 0 ].value;
            r.stdDev = 0.0;
            setInterpolated( s.interpolated, j );
         }
         else if ( radius > rLast  ||  k >= rCount ) // After the last
         {
            r.value  = ccwLegacyData[ i ]->readings[ rCount - 1 ].value;
            r.stdDev = 0.0;
            setInterpolated( s.interpolated, j );
         }
         else  // Interpolate the value
         {
            double dv = ccwLegacyData[ i ]->readings[ k     ].value - 
                        ccwLegacyData[ i ]->readings[ k - 1 ].value;
            
            double dR = ccwLegacyData[ i ]->readings[ k     ].d.radius -
                        ccwLegacyData[ i ]->readings[ k - 1 ].d.radius;

            dr = radius - ccwLegacyData[ i ]->readings[ k - 1 ].d.radius;

            r.value  =  ccwLegacyData[ i ]->readings[ k - 1 ].value + dr * dv / dR;
            r.stdDev = 0.0;

            setInterpolated( s.interpolated, j );
         }

         s.values.push_back( r );
         radius += delta_r;
      }

      newRawData.scanData.push_back( s );

      progress ->setValue( i );
      qApp     ->processEvents();
   }
  
   // Delete the bitmaps we allocated

   for ( uint i = 0; i < newRawData.scanData.size(); i++ ) 
      delete newRawData.scanData[ i ].interpolated;

   progress ->setVisible( false );
  
}

void US_Convert::changeCcw( int index )
{
   currentTriple = index;

   // Convert data for this cell / channel / wavelength
   convert();

   // and redo plot
   plot_current();
}

void US_Convert::write( void )
{ 
   // Specify the filename
   QString     dirname    = le_dir->text();
   if ( dirname.right( 1 ) != "/" ) dirname += "/"; // Ensure trailing /

   QStringList components = dirname.split( "/", QString::SkipEmptyParts );
   QString     runID      = components.last();

   QString triple         = triples[ currentTriple ];
   QStringList parts      = triple.split(" / ");

   QString     cell       = parts[ 0 ];
   QString     channel    = parts[ 1 ];
   QString     wavelength = parts[ 2 ];

   QString filename   = runID      + "." 
                      + runType    + "." 
                      + cell       + "." 
                      + channel    + "." 
                      + wavelength + ".auc";

   int err = write( filename );
   
   if ( err != US_DataIO::OK )
   {
      // Try to delete the file and tell the user   
   }
   else 
   {
      QMessageBox::information( this,
            tr( "Success" ),
            dirname + filename + tr( " written." ) );
   }        
}

int US_Convert::write( const QString& filename )
{
   if ( newRawData.scanData.empty() ) return US_DataIO::NODATA; 

   // Get the directory and write out the data
   QString dirname = le_dir->text();
   if ( dirname.right( 1 ) != "/" ) dirname += "/"; // Ensure trailing /

   // Create duplicate structure that doesn't contain excluded scans
   // Delete back to front, since structure changes with each deletion
   rawData filteredRawData = newRawData;
   for ( int i = filteredRawData.scanData.size() - 1; i >= 0; i-- )
   {
      if ( ! includes.contains( i ) )
         filteredRawData.scanData.erase( filteredRawData.scanData.begin() + i );
   }

   int result = US_DataIO::writeRawData( dirname + filename, filteredRawData );

   return result;
}
 
void US_Convert::writeAll( void )
{
   int saveCurrentTriple = currentTriple;
   QList< int > saveIncludes = includes;

   init_includes();

   for ( currentTriple = 0; currentTriple < triples.size(); currentTriple++ )
   {
      // Convert data for this cell / channel / wavelength
      convert();

      // and write it out
      write();

   }

   includes = saveIncludes;
   currentTriple = saveCurrentTriple;

}

void US_Convert::setInterpolated ( unsigned char* bitmap, int location )
{
   int byte = location / 8;
   int bit  = location % 8;

   bitmap[ byte ] |= 1 << 7 - bit;
}

void US_Convert::plot_current( void )
{
   if ( newRawData.scanData.empty() ) return;

   // Specify the filename
   QString     dirname    = le_dir->text();
   if ( dirname.right( 1 ) != "/" ) dirname += "/"; // Ensure trailing /

   QStringList components = dirname.split( "/", QString::SkipEmptyParts );
   QString     runID      = components.last();

   QString triple         = triples[ currentTriple ];
   QStringList parts      = triple.split(" / ");

   QString     cell       = parts[ 0 ];
   QString     channel    = parts[ 1 ];
   QString     wl         = parts[ 2 ];

   // Plot Title and legends
   QString title;
   QString xLegend = "Radius (in cm)";
   QString yLegend = "Absorbance";

   if ( strncmp( newRawData.type, "RA", 2 ) == 0 )
   {
      title = "Radial Absorbance Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
   }

   else if ( strncmp( newRawData.type, "IP", 2 ) == 0 )
   {
      title = "Interference Data\nRun ID: "
            + runID + " Cell: " + cell + " Radius: " + wl;
      yLegend = "Interference";
   }

   else if ( strncmp( newRawData.type, "RI", 2 ) == 0 )
   {
      title = "Radial Intensity Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
      yLegend = "Radial Intensity";
   }

   else if ( strncmp( newRawData.type, "FI", 2 ) == 0 )
   {
      title = "Fluorescence Intensity Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
      yLegend = "Fluorescence Intensity";
   }
      
   else if ( strncmp( newRawData.type, "WA", 2 ) == 0 )
   {
      title = "Wavelength Data\nRun ID: "
            + runID + " Cell: " + cell + " Radius: " + wl;
      xLegend = "Wavelength";
      yLegend = "Value";
   }

   else if ( strncmp( newRawData.type, "WI", 2 ) == 0 )
   {
      title = "Wavelength Intensity Data\nRun ID: "
            + runID + " Cell: " + cell + " Radius: " + wl;
      xLegend = "Wavelength";
      yLegend = "Value";
   }

   else
      title = "File type not recognized";
   
   data_plot->setTitle( title );
   data_plot->setAxisTitle( QwtPlot::yLeft, yLegend );
   data_plot->setAxisTitle( QwtPlot::xBottom, xLegend );
   
   // Initialize include list
   init_includes();
   
   // Plot current data for cell / channel / wavelength triple
   plot_all();
   
   // Set the Scan spin boxes
   ct_from->setMinValue( 0.0 );
   ct_from->setMaxValue(  newRawData.scanData.size() );
   
   ct_to  ->setMinValue( 0.0 );
   ct_to  ->setMaxValue(  newRawData.scanData.size() );
  
   reset_scan_ctrls();
}

void US_Convert::init_includes( void )
{
   includes.clear();
   for ( uint i = 0; i < newRawData.scanData.size(); i++ ) includes << i;
}

void US_Convert::plot_all( void )
{
   data_plot->detachItems();
   grid = us_grid( data_plot );

   int size = newRawData.scanData[ 0 ].values.size();

   double* r = new double[ size ];
   double* v = new double[ size ];

   double maxR = -1.0e99;
   double minR =  1.0e99;
   double maxV = -1.0e99;
   double minV =  1.0e99;

   for ( uint i = 0; i < newRawData.scanData.size(); i++ )
   {
      if ( ! includes.contains( i ) ) continue;
      scan* s = &newRawData.scanData[ i ];

      for ( int j = 0; j < size; j++ )
      {
         r[ j ] = s->values[ j ].d.radius;
         v[ j ] = s->values[ j ].value;

         maxR = max( maxR, r[ j ] );
         minR = min( minR, r[ j ] );
         maxV = max( maxV, v[ j ] );
         minV = min( minV, v[ j ] );
      }

      QString title = tr( "Raw Data at " )
         + QString::number( s->seconds ) + tr( " seconds" );

      QwtPlotCurve* c = us_curve( data_plot, title );
      c->setData( r, v, size );
   }

   // Reset the scan curves within the new limits
   double padR = ( maxR - minR ) / 30.0;
   double padV = ( maxV - minV ) / 30.0;
   
   data_plot->setAxisScale( QwtPlot::yLeft  , minV - padV, maxV + padV );
   data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );
   
   data_plot->replot();
 
   delete [] r;
   delete [] v;
}

void US_Convert::replot( void )
{
  plot_all();
}

void US_Convert::focus_from( double scan )
{
   int from = (int)scan;
   int to   = (int)ct_to->value();

   if ( from > to )
   {
      ct_to->disconnect();
      ct_to->setValue( scan );
      to = from;
      
      connect( ct_to, SIGNAL( valueChanged ( double ) ),
                      SLOT  ( focus_to     ( double ) ) );
   }

   focus( from, to );
}

void US_Convert::focus_to( double scan )
{
   int to   = (int)scan;
   int from = (int)ct_from->value();

   if ( from > to )
   {
      ct_from->disconnect();
      ct_from->setValue( scan );
      from = to;
      
      connect( ct_from, SIGNAL( valueChanged ( double ) ),
                        SLOT  ( focus_from   ( double ) ) );
   }

   focus( from, to );
}

void US_Convert::focus( int from, int to )
{
   if ( from == 0 )
   {
      pb_exclude->setEnabled( false );
   }
   else
   {
      pb_exclude->setEnabled( true );
   }

   if ( to == 0 )
      pb_excludeRange->setEnabled( false );
   else
      pb_excludeRange->setEnabled( true );

   QList< int > focus;  // We don't care if -1 is in the list
   for ( int i = from - 1; i <= to - 1; i++ ) focus << i;  

   set_colors( focus );

}

void US_Convert::set_colors( const QList< int >& focus )
{
   // Get pointers to curves
   QwtPlotItemList        list = data_plot->itemList();
   QList< QwtPlotCurve* > curves;
  
   for ( int i = 0; i < list.size(); i++ )
   {
      if ( list[ i ]->title().text().contains( "Raw" ) )
         curves << dynamic_cast< QwtPlotCurve* >( list[ i ] );
   }
  
   QPen   p   = curves[ 0 ]->pen();
   QBrush b   = curves[ 0 ]->brush();
   QColor std = US_GuiSettings::plotCurve();
   
   // Mark these scans in red
   for ( int i = 0; i < curves.size(); i++ )
   {
      if ( focus.contains( i ) )
      {
         p.setColor( Qt::red );
      }
      else
      {
         p.setColor( std );
         b.setColor( std );
      }

      curves[ i ]->setPen  ( p );
      curves[ i ]->setBrush( b );
   }

   data_plot->replot();
}

void US_Convert::exclude_one( void )
{
   int scan = (int)ct_from->value();

   // Offset by 1 for scan number vs index
   includes.removeAt( scan - 1 );
   reset_scan_ctrls();

   replot();
}

void US_Convert::exclude_range( void )
{
   int scanStart = (int)ct_from->value();
   int scanEnd   = (int)ct_to  ->value();

   // Let's remove back to front---the array
   // shifts with each deletion
   for ( int i = scanEnd - 1; i >= scanStart - 1; i-- )
      includes.removeAt( scanStart - 1 );

   reset_scan_ctrls();

   replot();
}

void US_Convert::include( void )
{
   init_includes();
   reset_scan_ctrls();

   replot();
}

void US_Convert::reset_scan_ctrls( void )
{

   ct_from->disconnect();
   ct_from->setValue   ( 0 );

   ct_to->disconnect();
   ct_to->setValue   ( 0 );

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

}

