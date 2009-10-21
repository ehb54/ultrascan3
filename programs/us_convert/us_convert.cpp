//! \file us_convert.cpp

#include <QApplication>

#include "us_convert.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_run_details.h"
#include "us_plot.h"
#include "us_math.h"
#include "us_expinfo.h"
#include "us_ccwinfo.h"

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

   // Set the wavelength tolerance for c/c/w determination
   QLabel* lb_tolerance = us_label( tr( "Dataset Separation Tolerance:" ) );
   settings->addWidget( lb_tolerance, row, 0 );

   ct_tolerance = us_counter ( 2, 0.0, 100.0, 5.0 ); // #buttons, low, high, start_value
   ct_tolerance->setStep( 1 );
   ct_tolerance->setMinimumWidth( 120 );
   settings->addWidget( ct_tolerance, row++, 1 );

   // External program to enter experiment information
   pb_expinfo = us_pushbutton( tr( "Enter Experiment Information" ) );
   connect( pb_expinfo, SIGNAL( clicked() ), SLOT( get_expinfo() ) );
   settings->addWidget( pb_expinfo, row, 0 );

   QPushButton* pb_load = us_pushbutton( tr( "Load Legacy Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   settings->addWidget( pb_load, row++, 1 );

   pb_details = us_pushbutton( tr( "Run Details" ), false );
   connect( pb_details, SIGNAL( clicked() ), SLOT( details() ) );
   settings->addWidget( pb_details, row++, 0, 1, 2 );

   // Change Run ID
   pb_change_runID = us_pushbutton( tr( "Change Run ID" ), false );
   connect( pb_change_runID, SIGNAL( clicked() ), SLOT( change_runID() ) );
   settings->addWidget( pb_change_runID, row, 0 );

   le_runID = us_lineedit( "", 1 );
   settings->addWidget( le_runID, row++, 1 );

   // Directory
   QLabel* lb_dir = us_label( tr( "Directory:" ) );
   settings->addWidget( lb_dir, row++, 0, 1, 2 );

   le_dir = us_lineedit( "", 1 );
   le_dir->setReadOnly( true );
   settings->addWidget( le_dir, row++, 0, 1, 2 );

   // Description
   lb_description = us_label( tr( "Description:" ), -1 );
   settings->addWidget( lb_description, row++, 0, 1, 2 );

   le_description = us_lineedit( "", 1 );
   le_description->setReadOnly( true );
   settings->addWidget( le_description, row++, 0, 1, 2 );

   // Cell / Channel / Wavelength
   lb_triple = us_label( tr( "Cell / Channel / Wavelength" ), -1 );
   settings->addWidget( lb_triple, row, 0 );

   lw_triple = us_listwidget();
   lw_triple->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
   lw_triple->setMinimumWidth( 50 );
   connect( lw_triple, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
                       SLOT  ( changeTriple( QListWidgetItem* ) ) );
   settings->addWidget( lw_triple, row++, 1, 2, 1 );

   // External program to enter c/c/w information
   pb_ccwinfo = us_pushbutton( tr( "Enter Current c/c/w Info" ), false );
   connect( pb_ccwinfo, SIGNAL( clicked() ), SLOT( get_ccwinfo() ) );
   settings->addWidget( pb_ccwinfo, row++, 0 );

   // Scan Controls
   QLabel* lb_scan = us_banner( tr( "Scan Controls" ) );
   settings->addWidget( lb_scan, row++, 0, 1, 2 );

   // Scan focus from
   QLabel* lb_from = us_label( tr( "Scan Focus from:" ), -1 );
   lb_from->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   settings->addWidget( lb_from, row, 0 );

   ct_from = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   ct_from->setStep( 1 );
   settings->addWidget( ct_from, row++, 1 );

   // Scan focus to
   QLabel* lb_to = us_label( tr( "Scan Focus to:" ), -1 );
   lb_to->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   settings->addWidget( lb_to, row, 0 );

   ct_to = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   ct_to->setStep( 1 );
   settings->addWidget( ct_to, row++, 1 );

   // Exclude and Include pushbuttons
   pb_exclude = us_pushbutton( tr( "Exclude Scan(s)" ), false );
   connect( pb_exclude, SIGNAL( clicked() ), SLOT( exclude_scans() ) );
   settings->addWidget( pb_exclude, row, 0 );

   pb_include = us_pushbutton( tr( "Include All" ), false );
   connect( pb_include, SIGNAL( clicked() ), SLOT( include() ) );
   settings->addWidget( pb_include, row++, 1 );

   // Defining data subsets
   pb_define = us_pushbutton( tr( "Define Subsets" ), false );
   connect( pb_define, SIGNAL( clicked() ), SLOT( define_subsets() ) );
   settings->addWidget( pb_define, row, 0 );

   pb_process = us_pushbutton( tr( "Process Subsets" ) , false );
   connect( pb_process, SIGNAL( clicked() ), SLOT( process_subsets() ) );
   settings->addWidget( pb_process, row++, 1 );

   // Choosing reference channel
   pb_reference = us_pushbutton( tr( "Define Reference Scans" ), false );
   connect( pb_reference, SIGNAL( clicked() ), SLOT( define_reference() ) );
   settings->addWidget( pb_reference, row, 0 );

   pb_cancelref = us_pushbutton( tr( "Undo Reference Scans" ), false );
   connect( pb_cancelref, SIGNAL( clicked() ), SLOT( cancel_reference() ) );
   settings->addWidget( pb_cancelref, row++, 1 );

   // Write pushbuttons
   pb_write = us_pushbutton( tr( "Write Current Data" ), false );
   connect( pb_write, SIGNAL( clicked() ), SLOT( write() ) );
   settings->addWidget( pb_write, row, 0 );

   pb_writeAll = us_pushbutton( tr( "Write All Data" ), false );
   connect( pb_writeAll, SIGNAL( clicked() ), SLOT( writeAll() ) );
   settings->addWidget( pb_writeAll, row++, 1 );

   // Progress bar
   QLabel* lb_placeholder = new QLabel();
   settings -> addWidget( lb_placeholder, row, 0, 1, 2 );

   lb_progress = us_label( tr( "Progress:" ) , -1 );
   lb_progress->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   lb_progress->setVisible( false );
   settings->addWidget( lb_progress, row, 0 );

   progress = us_progressBar( 0, 100, 0 );
   progress -> reset();
   progress -> setVisible( false );
   settings -> addWidget( progress, row++, 1 );

   // Standard pushbuttons
   QBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( resetAll() ) );
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

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );

   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );

   picker = new US_PlotPicker( data_plot );
   picker ->setRubberBand( QwtPicker::VLineRubberBand );

   // Now let's assemble the page
   
   QVBoxLayout* left     = new QVBoxLayout;
   QSpacerItem* spacer   = new QSpacerItem( 0, 0, 
                           QSizePolicy::Minimum, QSizePolicy::Fixed );

   left->addLayout( settings );
   left->addItem( spacer );
   left->addLayout( buttons );
   
   QHBoxLayout* main = new QHBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   main->addLayout( left );
   main->addLayout( plot );

   main->setStretch( 0, 2 );
   main->setStretch( 1, 4 );
}

void US_Convert::reset( void )
{
   lw_triple    ->clear();

   le_dir->setText( "" );

   le_description->setText( "" );
   le_runID      ->setText( "" );

   pb_exclude     ->setEnabled( false );
   pb_include     ->setEnabled( false );
   pb_write       ->setEnabled( false );
   pb_writeAll    ->setEnabled( false );
   pb_details     ->setEnabled( false );
   pb_change_runID->setEnabled( false );
   pb_cancelref   ->setEnabled( false );
   pb_ccwinfo      ->setEnabled( false );

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
   allData.clear();
   allCCWData.clear();
   RP_averaged = false;
   show_plot_progress = true;
   ExpData.clear();
   CCWData.clear();

   data_plot->detachItems();
   picker   ->disconnect();
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   grid = us_grid( data_plot );
   data_plot->replot();

   pb_define      ->setEnabled( false );
   pb_process     ->setEnabled( false );
   step           = NONE;

   pb_reference   ->setEnabled( false );
}

void US_Convert::resetAll( void )
{
   reset();

   ss_limits.clear();
   reference_start = 0;
   reference_end   = 0;

   ct_tolerance->setMinValue(   0.0 );
   ct_tolerance->setMaxValue( 100.0 );
   ct_tolerance->setValue   (   5.0 );
   ct_tolerance->setStep( 1 );
}

// User pressed the load data button
void US_Convert::load( QString dir )
{
   if ( dir.isEmpty() )
      read();                // Read the legacy data

   else
      read( dir );

/*
   // Display the data that was read
   for ( int i = 0; i < legacyData.size(); i++ )
   {
      US_DataIO::beckmanRaw d = legacyData[ i ];

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

         US_DataIO::reading r = d.readings[ j ];

         QString line = QString::number(r.d.radius, 'f', 4 )    + " "
                      + QString::number(r.value, 'E', 5 )       + " "
                      + QString::number(r.stdDev, 'E', 5 );
         qDebug() << line;
      }
   }

*/

   // we know all the triples from read, so we can convert all the data
   allData.clear();

   if ( triples.size() == 1 )
   {
      convert( true );

      allData << newRawData;
   }

   else
   {
      progress    ->setRange( 0, triples.size() );
      progress    ->setValue( 0 );
      lb_progress ->setText( tr( "Converting:" ) );
      lb_progress ->setVisible( true );
      progress    ->setVisible( true );

      for ( currentTriple = 0; currentTriple < triples.size(); currentTriple++ )
      {
         // Convert data for this cell / channel / wavelength
         convert();

         // and save it
         allData << newRawData;

         progress ->setValue( currentTriple );
         qApp     ->processEvents();
      }
   }

   // Make room for xml data
   for ( currentTriple  = 0; currentTriple < triples.size(); currentTriple++ )
      allCCWData << CCWData;

   lb_progress ->setVisible( false );
   progress    ->setVisible( false );
   
   currentTriple = 0;     // Now let's show the user the first one
   plot_current();

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   // Ok to enable some buttons now
   bool writeOK = ExpData.investigator != 0 &&
                  triples.size() > 0;                  
   pb_include ->setEnabled( true );
   pb_write   ->setEnabled( writeOK );
   pb_writeAll->setEnabled( writeOK );
   pb_details ->setEnabled( true );
   pb_change_runID->setEnabled( true );
   pb_ccwinfo  ->setEnabled( true );
}

void US_Convert::read( void )
{
   // Ask for data directory
   QString dir = QFileDialog::getExistingDirectory( this, 
         tr( "Raw Data Directory" ),
         US_Settings::dataDir(),
         QFileDialog::DontResolveSymlinks );

   if ( dir.isEmpty() ) return; 

   reset();
   read( dir );
}

void US_Convert::read( QString dir )
{
   // Get legacy file names
   QDir d( dir, "*", QDir::Name, QDir::Files | QDir::Readable );
   d.makeAbsolute();
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /

   // Set the runID and directory
   QStringList components = dir.split( "/", QString::SkipEmptyParts );
   runID    = components.last();
   le_runID ->setText( runID );
   le_dir   ->setText( dir );

   QStringList files = d.entryList( QDir::Files );
   QStringList fileList;

   // Read into local structures
   
   for ( int i = 0; i < files.size(); i++ )
   {
      // Look for a proper filename match:
      // Optional channel + 4 to 6 digits + dot + file type + cell number

      QRegExp rx( "^[A-J]?\\d{4,6}\\.(?:RA|RI|IP|FI|WA|WI)\\d$" );
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

   progress    ->setRange( 0, fileList.size() - 1 );
   progress    ->setValue( 0 );
   lb_progress ->setText( tr( "Reading:" ) );
   lb_progress ->setVisible( true );
   progress    ->setVisible( true );
   for ( int i = 0; i < fileList.size(); i++ )
   {
      US_DataIO::beckmanRaw data;
      US_DataIO::readLegacyFile( dir + fileList[ i ], data );
      qApp->processEvents();

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

         legacyData << data;
         legacyData << data2;

         pb_reference->setEnabled( true );

      }

      else if ( runType == "RA" && ss_limits.size() > 2 )
      {
         // We are subdividing the scans
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

            legacyData << data2;                     // Send the current data subset
            data2.channel++;                         // increment the channel letter
         }

      }
  
      else if ( runType == "RA" )                     // Perhaps before subdividing
      {
         // Allow the user to process subsets
         pb_define  ->setEnabled( true );

         legacyData << data;        // For user to see single or multiple datasets
      }

      else
         legacyData << data;

      progress ->setValue( i );
      qApp     ->processEvents();
   }

   ss_limits.clear();                       // Don't need this any more

   lb_progress ->setVisible( false );
   progress    ->setVisible( false );

   if ( runType == "WA" )
      setCcrTriples();                      // Wavelength data is handled differently here
   else
      setCcwTriples();

   lw_triple->addItems( triples );
   currentTriple = 0;
}

void US_Convert::setCcwTriples( void )
{
   // Most triples are ccw
   lb_triple   ->setText( tr( "Cell / Channel / Wavelength" ) );
   ct_tolerance->setStep( 1 );

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
   double tolerance = (double)ct_tolerance->value() + 0.05;    // to stay between wl numbers
   
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
   for ( int i = 0; i < legacyData.size(); i++ )
   {
      QString cell       = QString::number( legacyData[ i ].cell );
      QString channel    = QString( legacyData[ i ].channel );
      double wl          = legacyData[ i ].t.wavelength;
      QString wavelength = "0";

      // find the average wavelength
      for ( int j = 0; j < wl_average.size(); j++ )
      {
         if ( fabs( wl_average[ j ] - wl ) < tolerance )
         {
            wavelength = QString::number( wl_average[ j ] );
            break;
         }
      }

      QString t = cell + " / " + channel + " / " + wavelength;
      if (! triples.contains( t ) ) triples << t;
   }
}

void US_Convert::setCcrTriples( void )
{
   // First of all, wavelength triples are ccr.
   lb_triple   ->setText( tr( "Cell / Channel / Radius" ) );
   ct_tolerance->setStep( 0.1 );

   // Now get the radius values
   QStringList radii;

   for ( int i = 0; i < legacyData.size(); i++ )
   {
      QString r = QString::number( legacyData[ i ].t.radius, 'f', 1 );
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
   double tolerance = (double)ct_tolerance->value() + 0.05;    // to stay between r values
   
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
   for ( int i = 0; i < legacyData.size(); i++ )
   {
      QString cell       = QString::number( legacyData[ i ].cell );
      QString channel    = QString( legacyData[ i ].channel );
      double r           = legacyData[ i ].t.radius;
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

void US_Convert::convert( bool showProgressBar )
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
   double tolerance = (double)ct_tolerance->value() + 0.05;    // to stay between wl numbers

   for ( int i = 0; i < legacyData.size(); i++ )
   {
      if ( legacyData[ i ].cell == cell       &&
           legacyData[ i ].channel == channel &&
           fabs ( legacyData[ i ].t.wavelength - wavelength ) < tolerance )
         ccwLegacyData << &legacyData[ i ];
   }

   // Sort the list according to time.  Use a simple bubble sort
   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      for ( int j = i + i; j < ccwLegacyData.size(); j++ )
      {
         if ( ccwLegacyData[ j ]->seconds < ccwLegacyData[ i ]->seconds ) 
            ccwLegacyData.swap( i, j );
      }
   }

   if ( ccwLegacyData.isEmpty() ) return ; 

   strncpy( newRawData.type, runType.toAscii().constData(), 2 );
   // GUID is done by US_DataIO.
   newRawData.cell        = cell;
   newRawData.channel     = channel;
   newRawData.description = ccwLegacyData[ 0 ]->description;
   
   le_description->setText( newRawData.description );

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
   
   // Set the distance between readings 
   double delta_r = ( runType == "IP" ) 
      ? ( max_radius - min_radius ) / ( ccwLegacyData[ 0 ]->readings.size() - 1 )
      : 0.001;

   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      US_DataIO::scan s;
      s.temperature = ccwLegacyData[ i ]->temperature;
      s.rpm         = ccwLegacyData[ i ]->rpm;
      s.seconds     = ccwLegacyData[ i ]->seconds;
      s.omega2t     = ccwLegacyData[ i ]->omega2t;
      s.wavelength  = ccwLegacyData[ i ]->t.wavelength;
      s.delta_r     = delta_r;

      // Enable progress bar
      if ( showProgressBar )
      {
         progress    ->setRange( 0, ccwLegacyData.size() - 1 );
         progress    ->setValue( 0 );
         lb_progress ->setText( tr( "Converting:" ) );
         lb_progress ->setVisible( true );
         progress    ->setVisible( true );
      }

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
            r.d.radius = ccwLegacyData[ i ]->readings[ j ].d.radius;
            r.value    = ccwLegacyData[ i ]->readings[ j ].value;
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
         double r0     = ccwLegacyData[ i ]->readings[ 0 ].d.radius;
         int    rCount = ccwLegacyData[ i ]->readings.size();       
         double rLast  = ccwLegacyData[ i ]->readings[ rCount - 1 ].d.radius;
         
         int    k      = 0;
         
         for ( int j = 0; j < radius_count; j++ )
         {
            US_DataIO::reading r;
            double             dr = 0.0;

            if ( k < rCount )
               dr = radius - ccwLegacyData[ i ]->readings[ k ].d.radius;

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
               setInterpolated( interpolated, j );
            }
            else if ( radius > rLast  ||  k >= rCount ) // After the last
            {
               r.value  = ccwLegacyData[ i ]->readings[ rCount - 1 ].value;
               r.stdDev = 0.0;
               setInterpolated( interpolated, j );
            }
            else  // Interpolate the value
            {
               double dv = ccwLegacyData[ i ]->readings[ k     ].value - 
                           ccwLegacyData[ i ]->readings[ k - 1 ].value;
               
               double dR = ccwLegacyData[ i ]->readings[ k     ].d.radius -
                           ccwLegacyData[ i ]->readings[ k - 1 ].d.radius;

               dr = radius - ccwLegacyData[ i ]->readings[ k - 1 ].d.radius;

               r.value  = ccwLegacyData[ i ]->readings[ k - 1 ].value + dr * dv / dR;
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
      
      if ( showProgressBar ) progress ->setValue( i );
      qApp->processEvents();
   }

   // Delete the bitmaps we allocated

   //for ( uint i = 0; i < newRawData.scanData.size(); i++ ) 
   //   delete newRawData.scanData[ i ].interpolated;

   if ( showProgressBar )
   {
      lb_progress ->setVisible( false );
      progress    ->setVisible( false );
   }
}

void US_Convert::details( void )
{
   QString     dirname    = le_dir->text();
   // if ( dirname.right( 1 ) != "/" ) dirname += "/"; // Ensure trailing /

   US_RunDetails* dialog
      = new US_RunDetails( allData, runID, dirname, triples );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

void US_Convert::change_runID( void )
{
   QRegExp rx( "^[A-Za-z0-9_]{1,20}$" );
   QString new_runID = le_runID->text();
      
   if ( rx.indexIn( new_runID ) < 0 ) return;

   runID = new_runID;
   QMessageBox::information( this,
            tr( "Success" ),
            tr( "RunID has been changed to ") + runID );

   plot_titles();
}

void US_Convert::changeTriple( QListWidgetItem* )
{
   currentTriple = lw_triple->currentRow();

   // Reset maximum scan control values
   reset_scan_ctrls();

   // Redo plot
   plot_current();
}

void US_Convert::write( void )
{ 
   // Get the current cell/channel/wavelength
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
            filename + tr( " written." ) );
   }        
}

int US_Convert::write( const QString& filename )
{
   if ( newRawData.scanData.empty() ) return US_DataIO::NODATA; 

   QDir        writeDir( US_Settings::resultDir() );
   QString     dirname = writeDir.absolutePath() + "/" + runID + "/";

   if ( ! writeDir.exists( runID ) )
   {
     if ( ! writeDir.mkdir( runID ) )
     {
        QMessageBox::information( this,
              tr( "Error" ),
              tr( "Cannot write to " ) + writeDir.absolutePath() );

        return  US_DataIO::CANTOPEN;
     }
   }

   // Create duplicate structure that doesn't contain excluded scans
   // Delete back to front, since structure changes with each deletion
   US_DataIO::rawData filteredRawData = allData[ currentTriple ];

   for ( int i = filteredRawData.scanData.size() - 1; i >= 0; i-- )
   {
      if ( ! includes.contains( i ) )
         filteredRawData.scanData.erase( filteredRawData.scanData.begin() + i );
   }

   return US_DataIO::writeRawData( dirname + filename, filteredRawData );
}
 
int US_Convert::writeAll( void )
{
   if ( allData[ 0 ].scanData.empty() ) return US_DataIO::NODATA; 

   QDir        writeDir( US_Settings::resultDir() );
   QString     dirname = writeDir.absolutePath() + "/" + runID + "/";

   if ( ! writeDir.exists( runID ) )
   {
     if ( ! writeDir.mkdir( runID ) )
     {
        QMessageBox::information( this,
              tr( "Error" ),
              tr( "Cannot write to " ) + writeDir.absolutePath() );

        return US_DataIO::CANTOPEN;
     }
   }

   int result;

   progress    ->setRange( 0, triples.size() - 1 );
   progress    ->setValue( 0 );
   lb_progress ->setText( tr( "Writing:" ) );
   lb_progress ->setVisible( true );
   progress    ->setVisible( true );

   for ( int i = 0; i < triples.size(); i++ )
   {
      QString     triple     = triples[ i ];
      QStringList parts      = triple.split(" / ");

      QString     cell       = parts[ 0 ];
      QString     channel    = parts[ 1 ];
      QString     wavelength = parts[ 2 ];

      QString     filename   = runID      + "." 
                             + runType    + "." 
                             + cell       + "." 
                             + channel    + "." 
                             + wavelength + ".auc";

      US_DataIO::rawData currentData = allData[ i ];
      result = US_DataIO::writeRawData( dirname + filename, allData[ i ] );

      if ( result !=  US_DataIO::OK ) break;
      
      progress ->setValue( i );
      qApp     ->processEvents();
   }

   lb_progress ->setVisible( false );
   progress    ->setVisible( false );

   if ( result != US_DataIO::OK )
   {
      // Try to delete the file and tell the user
      return result;
   }

   if ( ( result = writeXmlFile() ) != US_DataIO::OK )
   {
      // Problems writing xml file
      qDebug() << "Problems writing xml file";
      return result;
   }

   QMessageBox::information( this,
         tr( "Success" ),
         QString::number( triples.size() ) + " " + 
         runID + tr( " files written." ) );

   return result;
}

void US_Convert::setInterpolated ( unsigned char* bitmap, int location )
{
   int byte = location / 8;
   int bit  = location % 8;

   bitmap[ byte ] |= 1 << 7 - bit;
}

void US_Convert::plot_current( void )
{
   US_DataIO::rawData currentData = allData[ currentTriple ];

   if ( currentData.scanData.empty() ) return;

   plot_titles();

   // Initialize include list
   init_includes();
   
   // Plot current data for cell / channel / wavelength triple
   plot_all();
   
   // Set the Scan spin boxes
   reset_scan_ctrls();
}

void US_Convert::plot_titles( void )
{
   US_DataIO::rawData currentData = allData[ currentTriple ];

   QString triple         = triples[ currentTriple ];
   QStringList parts      = triple.split(" / ");

   QString     cell       = parts[ 0 ];
   QString     channel    = parts[ 1 ];
   QString     wl         = parts[ 2 ];

   // Plot Title and legends
   QString title;
   QString xLegend = "Radius (in cm)";
   QString yLegend = "Absorbance";

   if ( strncmp( currentData.type, "RA", 2 ) == 0 )
   {
      title = "Radial Absorbance Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
   }

   else if ( strncmp( currentData.type, "RP", 2 ) == 0 )
   {
      title = "Pseudo Absorbance Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
   }

   else if ( strncmp( currentData.type, "IP", 2 ) == 0 )
   {
      title = "Interference Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
      yLegend = "Fringes";
   }

   else if ( strncmp( currentData.type, "RI", 2 ) == 0 )
   {
      title = "Radial Intensity Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
      yLegend = "Radial Intensity";
   }

   else if ( strncmp( currentData.type, "FI", 2 ) == 0 )
   {
      title = "Fluorescence Intensity Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
      yLegend = "Fluorescence Intensity";
   }
      
   else if ( strncmp( currentData.type, "WA", 2 ) == 0 )
   {
      title = "Wavelength Data\nRun ID: "
            + runID + " Cell: " + cell + " Radius: " + wl;
      xLegend = "Wavelength";
      yLegend = "Value";
   }

   else if ( strncmp( currentData.type, "WI", 2 ) == 0 )
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

}

void US_Convert::init_includes( void )
{
   includes.clear();
   for ( int i = 0; i < allData[ currentTriple ].scanData.size(); i++ ) 
      includes << i;
}

void US_Convert::plot_all( void )
{
   US_DataIO::rawData currentData = allData[ currentTriple ];

   data_plot->detachItems();
   grid = us_grid( data_plot );

   int size = currentData.scanData[ 0 ].readings.size();

   double* r = new double[ size ];
   double* v = new double[ size ];

   double maxR = -1.0e99;
   double minR =  1.0e99;
   double maxV = -1.0e99;
   double minV =  1.0e99;

   if ( show_plot_progress )
   {
      progress    ->setRange( 0, currentData.scanData.size() - 1 );
      progress    ->setValue( 0 );
      lb_progress ->setText( tr( "Plotting:" ) );
      lb_progress ->setVisible( true );
      progress    ->setVisible( true );
   }

   for ( int i = 0; i < currentData.scanData.size(); i++ )
   {
      if ( ! includes.contains( i ) ) continue;
      US_DataIO::scan* s = &currentData.scanData[ i ];

      for ( int j = 0; j < size; j++ )
      {
         r[ j ] = s->readings[ j ].d.radius;
         v[ j ] = s->readings[ j ].value;

         maxR = max( maxR, r[ j ] );
         minR = min( minR, r[ j ] );
         maxV = max( maxV, v[ j ] );
         minV = min( minV, v[ j ] );
      }

      QString title = tr( "Raw Data at " )
         + QString::number( s->seconds ) + tr( " seconds" );

      QwtPlotCurve* c = us_curve( data_plot, title );
      c->setData( r, v, size );

      if ( show_plot_progress )
      {
         progress ->setValue( i );
         qApp     ->processEvents();
      }
   }

   if ( show_plot_progress )
   {
      lb_progress ->setVisible( false );
      progress    ->setVisible( false );
   }

   // Reset the scan curves within the new limits
   double padR = ( maxR - minR ) / 30.0;
   double padV = ( maxV - minV ) / 30.0;
   
   data_plot->setAxisScale( QwtPlot::yLeft  , minV - padV, maxV + padV );
   data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );
   
   show_plot_progress = false;
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

void US_Convert::exclude_scans( void )
{
   int scanStart = (int)ct_from->value();
   int scanEnd   = (int)ct_to  ->value();

   // Let's remove back to front---the array
   // shifts with each deletion
   // Works when single scan too
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

// Reset the boundaries on the scan controls
void US_Convert::reset_scan_ctrls( void )
{
   ct_from->disconnect();
   ct_from->setMinValue( 0.0 );
   ct_from->setMaxValue(  allData[ currentTriple ].scanData.size() );
   ct_from->setValue   ( 0 );

   ct_to  ->disconnect();
   ct_to  ->setMinValue( 0.0 );
   ct_to  ->setMaxValue(  allData[ currentTriple ].scanData.size() );
   ct_to  ->setValue   ( 0 );

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

}

void US_Convert::cClick( const QwtDoublePoint& p )
{
   switch ( step )
   {
      case SPLIT :
         draw_vline( p.x() );
         ss_limits << p.x();
         break;

      default :
         break;

   }

}

void US_Convert::define_subsets( void )
{
   ss_limits.clear();

   pb_process ->setEnabled( true );

   connect( picker, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                    SLOT  ( cClick  ( const QwtDoublePoint& ) ) );

   step = SPLIT;

}

void US_Convert::process_subsets( void )
{
   pb_process ->setEnabled( false );
   pb_define  ->setEnabled( false );
   picker   ->disconnect();

   if ( ss_limits.size() < 2 )
   {
      // Not enough clicks to work with
      ss_limits.clear();
      pb_process ->setEnabled( true );
      return;
   }

   // Let's make sure the points are in sorted order
   for ( int i = 0; i < ss_limits.size() - 1; i++ )
      for ( int j = i + 1; j < ss_limits.size(); j++ )
         if ( ss_limits[ i ] > ss_limits[ j ] )
         {
            double temp = ss_limits[ i ];
            ss_limits[ i ] = ss_limits[ j ];
            ss_limits[ j ] = temp;
         }

   // Let's make sure all the data is included somewhere
   ss_limits[ 0 ] = 5.7;
   ss_limits[ ss_limits.size() - 1 ] = 7.3;

/*
   for ( int i = 0; i < ss_limits.size(); i++ )
   {
      qDebug() << "Radius point " << i + 1 << ": " << ss_limits[ i ];
   }
*/

   // Now that we know we're subdividing, let's reconvert the file
   QString dir = le_dir->text();

   reset();
   load( dir );
}

void US_Convert::cDrag( const QwtDoublePoint& )
{
   switch ( step )
   {
      case REFERENCE :
         data_plot->replot();
         break;

      default :
         break;

   }

}

void US_Convert::define_reference( void )
{
   connect( picker, SIGNAL( cMouseDown     ( const QwtDoublePoint& ) ),
                    SLOT  ( start_reference( const QwtDoublePoint& ) ) );

   connect( picker, SIGNAL( cMouseDrag( const QwtDoublePoint& ) ),
                    SLOT  ( cDrag     ( const QwtDoublePoint& ) ) );

   connect( picker, SIGNAL( cMouseUp    ( const QwtDoublePoint& ) ),
                    SLOT  ( process_reference( const QwtDoublePoint& ) ) );

   pb_reference ->setEnabled( false );

   step = REFERENCE;
}

void US_Convert::start_reference( const QwtDoublePoint& p )
{
   reference_start   = p.x();

   draw_vline( reference_start );
   data_plot->replot();
}

void US_Convert::process_reference( const QwtDoublePoint& p )
{
   reference_end = p.x();
   draw_vline( reference_end );
   data_plot->replot();

   pb_reference  ->setEnabled( false );
   picker        ->disconnect();

   // Double check if min < max
   if ( reference_start > reference_end )
   {
      double temp     = reference_start;
      reference_start = reference_end;
      reference_end   = temp;
   }

/*
   qDebug() << "Starting Radius: " << reference_start;
   qDebug() << "Ending Radius:   " << reference_end;
*/

   // Calculate the averages for all triples
   RP_calc_avg();

   // Now that we have the averages, let's replot
   RP_reference_triple = currentTriple;

   // Default to displaying the first non-reference triple
   for ( int i = 0; i < allData.size(); i++ )
   {
      if ( i != RP_reference_triple )
      {
         currentTriple = i;
         break;
      }
   }

   lw_triple->setCurrentRow( currentTriple );
   plot_current();
}

void US_Convert::RP_calc_avg( void )
{
   if ( RP_averaged ) return;             // Average calculation has already been done

   US_DataIO::rawData referenceData = allData[ currentTriple ];
   int ref_size = referenceData.scanData[ 0 ].readings.size();

   for ( int i = 0; i < referenceData.scanData.size(); i++ )
   {
      US_DataIO::scan s = referenceData.scanData[ i ];

      int j      = 0;
      int count  = 0;
      double sum = 0.0;
      while ( s.readings[ j ].d.radius < reference_start && j < ref_size )
         j++;

      while ( s.readings[ j ].d.radius < reference_end && j < ref_size )
      {
         sum += s.readings[ j ].value;
         count++;
         j++;
      }
      RP_averages << sum / count;
   }

/*
   for ( int i = 0; i < referenceData.scanData.size(); i++ )
   {
      qDebug() << "Average " << i + 1 << ": " << RP_averages[ i ];
   }
*/

   // Now calculate the pseudo-absorbance
   RIData = allData;

   for ( int i = 0; i < allData.size(); i++ )
   {
      US_DataIO::rawData* currentData = &allData[ i ];

      for ( int j = 0; j < currentData->scanData.size(); j++ )
      {
         US_DataIO::scan* s = &currentData->scanData[ j ];

         for ( int k = 0; k < s->readings.size(); k++ )
         {
            US_DataIO::reading* r = &s->readings[ k ];

            r->value = log10(RP_averages[ j ] / r->value );
         }
      }
      strncpy( currentData->type, "RP", 2);
   }

   RP_averaged = true;
   pb_cancelref ->setEnabled( true );
}

void US_Convert::cancel_reference( void )
{
   RP_averaged = false;
   allData     = RIData;
   RIData.clear();

   RP_averages.clear();
   reference_start = 0.0;
   reference_end   = 0.0;

   pb_reference  ->setEnabled( true );
   pb_cancelref  ->setEnabled( false );
   currentTriple = 0;
   lw_triple->setCurrentRow( currentTriple );

   plot_current();
}

void US_Convert::get_expinfo( void )
{
   US_ExpInfo* expinfo = new US_ExpInfo();

   connect( expinfo, SIGNAL( update_expinfo_selection( US_ExpInfo::ExpInfo& ) ),
            this   , SLOT  ( update_expinfo          ( US_ExpInfo::ExpInfo& ) ) );

   connect( expinfo, SIGNAL( cancel_expinfo_selection() ),
            this   , SLOT  ( cancel_expinfo          () ) );

   expinfo->exec();
   qApp->processEvents();
   delete expinfo;
}

void US_Convert::update_expinfo( US_ExpInfo::ExpInfo& d )
{
   ExpData.clear();
   ExpData.investigator = d.investigator;
   ExpData.expType      = QString( d.expType );
   ExpData.rotor        = d.rotor;
   ExpData.date         = d.date;
   ExpData.label        = QString( d.label );
   ExpData.comments     = QString( d.comments );

   // Ok to enable some buttons now
   bool writeOK = ExpData.investigator != 0 &&
                  triples.size() > 0;                  
   pb_write   ->setEnabled( writeOK );
   pb_writeAll->setEnabled( writeOK );
}

void US_Convert::cancel_expinfo( void )
{
qDebug() << "In cancel_expinfo";
   ExpData.clear();
}


void US_Convert::get_ccwinfo( void )
{
   
   US_CCWInfo* ccwinfo = new US_CCWInfo( ExpData.investigator );

   connect( ccwinfo, SIGNAL( update_ccwinfo_selection( US_CCWInfo::CCWInfo& ) ),
            this   , SLOT  ( update_ccwinfo          ( US_CCWInfo::CCWInfo& ) ) );

   connect( ccwinfo, SIGNAL( cancel_ccwinfo_selection() ),
            this   , SLOT  ( cancel_ccwinfo          () ) );

   ccwinfo->exec();
   qApp->processEvents();
   delete ccwinfo;
}

void US_Convert::update_ccwinfo( US_CCWInfo::CCWInfo& d )
{
   // See if investigator has changed
   ExpData.investigator = d.investigator;
   allCCWData[ currentTriple ].investigator = d.investigator;
   allCCWData[ currentTriple ].centerpiece  = d.centerpiece;
   allCCWData[ currentTriple ].bufferID     = d.bufferID;
   allCCWData[ currentTriple ].analyteID    = d.analyteID;
}

void US_Convert::cancel_ccwinfo( void )
{
qDebug() << "In cancel_ccwinfo";
   allCCWData[ currentTriple ].clear();
}

void US_Convert::draw_vline( double radius )
{
   double r[ 2 ];

   r[ 0 ] = radius;
   r[ 1 ] = radius;
   QwtScaleDiv* y_axis = data_plot->axisScaleDiv( QwtPlot::yLeft );

   double padding = ( y_axis->upperBound() - y_axis->lowerBound() ) / 30.0;

   double v[ 2 ];
   v [ 0 ] = y_axis->upperBound() - padding;
   v [ 1 ] = y_axis->lowerBound() + padding;

   QwtPlotCurve* v_line = us_curve( data_plot, "V-Line" );
   v_line->setData( r, v, 2 );

   QPen pen = QPen( QBrush( Qt::white ), 2.0 );
   v_line->setPen( pen );

   data_plot->replot();
}

int US_Convert::writeXmlFile( void )
{ 
   QDir        writeDir( US_Settings::resultDir() );
   QString     dirname = writeDir.absolutePath() + "/" + runID + "/";

   if ( ! writeDir.exists( runID ) )
   {
     if ( ! writeDir.mkdir( runID ) )
     {
        QMessageBox::information( this,
              tr( "Error" ),
              tr( "Cannot write to " ) + writeDir.absolutePath() );

        return US_DataIO::CANTOPEN;
     }
   }

   QString writeFile = runID      + "." 
                     + runType    + ".xml";
   QFile file( dirname + writeFile );
   if ( !file.open( QIODevice::WriteOnly | QIODevice::Text) )
   {
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Cannot open file " ) + dirname + writeFile );
      return US_DataIO::CANTOPEN;
   }

   progress    ->setRange( 0, triples.size() - 1 );
   progress    ->setValue( 0 );
   lb_progress ->setText( tr( "Writing XML:" ) );
   lb_progress ->setVisible( true );
   progress    ->setVisible( true );

   QXmlStreamWriter xml;
   xml.setDevice( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD("<!DOCTYPE US_Scandata>");
   xml.writeStartElement("US_Scandata");
   xml.writeAttribute("version", "1.0");

   // elements
   xml.writeStartElement( "experiment" );
   xml.writeAttribute   ( "ID", "replace with DB experimentID" );
   xml.writeAttribute   ( "type", ExpData.expType );

      xml.writeTextElement ( "name", "replace with description");

      xml.writeStartElement( "investigator" );
      xml.writeAttribute   ( "ID", QString::number( ExpData.investigator ) );
      xml.writeEndElement  ();
      
      xml.writeStartElement( "operator" );
      xml.writeAttribute   ( "ID", "replace with operator ID" );
      xml.writeEndElement  ();

      xml.writeStartElement( "rotor" );
      xml.writeAttribute   ( "ID", QString::number( ExpData.rotor ) );
      xml.writeEndElement  ();

      xml.writeStartElement( "GUID" );
      xml.writeAttribute   ( "ID", "replace with GUID" );
      xml.writeEndElement  ();

      // loop through the following for c/c/w combinations
      for ( int i = 0; i < triples.size(); i++ )
      {
         QString triple         = triples[ i ];
         QStringList parts      = triple.split(" / ");

         QString     cell       = parts[ 0 ];
         QString     channel    = parts[ 1 ];
         QString     wl         = parts[ 2 ];

         xml.writeStartElement( "dataset" );
         xml.writeAttribute   ( "cell", cell );
         xml.writeAttribute   ( "channel", channel );
         xml.writeAttribute   ( "wavelength", wl );

            xml.writeStartElement( "GUID" );
            xml.writeAttribute   ( "ID", "replace with GUID" );
            xml.writeEndElement  ();

            xml.writeStartElement( "centerpiece" );
            xml.writeAttribute   ( "ID", QString::number( allCCWData[ i ].centerpiece ) );
            xml.writeEndElement  ();

            xml.writeStartElement( "buffer" );
            xml.writeAttribute   ( "ID", QString::number( allCCWData[ i ].bufferID ) );
            xml.writeEndElement  ();

            xml.writeStartElement( "analyte" );
            xml.writeAttribute   ( "ID", QString::number( allCCWData[ i ].analyteID ) );
            xml.writeEndElement  ();

         xml.writeEndElement   ();
      }

   xml.writeTextElement ( "date", ExpData.date );
   xml.writeTextElement ( "label", ExpData.label );
   xml.writeTextElement ( "comments", ExpData.comments );

   xml.writeEndElement(); // US_Scandata
   xml.writeEndDocument();

   lb_progress ->setVisible( false );
   progress    ->setVisible( false );

   return US_DataIO::OK;
}
