//! \file us_fit_meniscus_main.cpp

#include <QApplication>
#include <QDomDocument>

#include <uuid/uuid.h>

#include "us_edvabs.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_run_details.h"
#include "us_exclude_profile.h"
#include "us_ri_noise.h"
#include "us_edit_scan.h"
#include "us_math.h"

//! \brief Main program for US_Edvabs. Loads translators and starts
//         the class US_FitMeniscus.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_Edvabs w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_Edvabs::US_Edvabs() : US_Widgets()
{
   check  = QIcon( US_Settings::usHomeDir() + "/etc/check.png" );
   invert = 1.0;

   setWindowTitle( tr( "Edit UltraScan Data" ) );
   setPalette( US_GuiSettings::frameColor() );

   QHBoxLayout* main = new QHBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QVBoxLayout* left = new QVBoxLayout;

   // Start of Grid Layout
   QGridLayout* specs = new QGridLayout;
   int s_row = 0;

   // Row 1
   QPushButton* pb_load = us_pushbutton( tr( "Load Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   specs->addWidget( pb_load, s_row, 0, 1, 2 );

   pb_details = us_pushbutton( tr( "Run Details" ), false );
   connect( pb_details, SIGNAL( clicked() ), SLOT( details() ) );
   specs->addWidget( pb_details, s_row++, 2, 1, 2 );

   // Row 2
   QLabel* lb_info = us_label( "Run Info:", -1 );
   specs->addWidget( lb_info, s_row, 0 );

   le_info = us_lineedit( "", 1 );
   le_info->setReadOnly( true );
   specs->addWidget( le_info, s_row++, 1, 1, 3 );

   // Row 3
   QLabel* lb_triple = us_label( tr( "Cell / Channel / Wavelength" ), -1 );
   specs->addWidget( lb_triple, s_row, 0, 1, 2 );

   cb_triple = us_comboBox();
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
                       SLOT  ( new_triple         ( int ) ) );
   specs->addWidget( cb_triple, s_row++, 2, 1, 2 );

   // Row 4
   pb_gaps = us_pushbutton( tr( "Check for Scan Gaps" ), false );
   connect( pb_gaps, SIGNAL( clicked() ), SLOT( gap_check() ) );
   specs->addWidget( pb_gaps, s_row, 0, 1, 2 );

   ct_gaps = us_counter ( 1, 10.0, 100.0 ); 
   ct_gaps->setValue( 50 );
   ct_gaps->setStep ( 10 );
   specs->addWidget( ct_gaps, s_row++, 2, 1, 2 );

   // Row 5
   QLabel* lb_scan = us_banner( tr( "Scan Controls" ) );
   specs->addWidget( lb_scan, s_row++, 0, 1, 4 );
   
   // Row 6

   // Scans
   QLabel* lb_from = us_label( tr( "Scan Focus from:" ), -1 );
   lb_from->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lb_from, s_row, 0 );

   ct_from = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   ct_from->setStep( 1 );
   specs->addWidget( ct_from, s_row, 1 );

   QLabel* lb_to = us_label( tr( "to:" ), -1 );
   lb_to->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lb_to, s_row, 2 );

   ct_to = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   ct_to->setStep( 1 );
   specs->addWidget( ct_to, s_row++, 3 );
   
   // Exclude and Include pushbuttons
   // Row 7
   pb_excludeRange = us_pushbutton( tr( "Exclude Scan Range" ), false );
   connect( pb_excludeRange, SIGNAL( clicked() ), SLOT( exclude_range() ) );
   specs->addWidget( pb_excludeRange, s_row, 0, 1, 2 );
   
   pb_exclusion = us_pushbutton( tr( "Exclusion Profile" ), false );
   connect( pb_exclusion, SIGNAL( clicked() ), SLOT( exclusion() ) );
   specs->addWidget( pb_exclusion, s_row++, 2, 1, 2 );

   // Row 8
   pb_edit1 = us_pushbutton( tr( "Edit Single Scan" ), false );
   connect( pb_edit1, SIGNAL( clicked() ), SLOT( edit_scan() ) );
   specs->addWidget( pb_edit1, s_row, 0, 1, 2 );

   // Row 9
   pb_include = us_pushbutton( tr( "Include All" ), false );
   connect( pb_include, SIGNAL( clicked() ), SLOT( include() ) );
   specs->addWidget( pb_include, s_row++, 2, 1, 2 );

   // Edit label row
   QLabel* lb_edit = us_banner( tr( "Edit Controls" ) );
   specs->addWidget( lb_edit, s_row++, 0, 1, 4 );

   // Meniscus row
   pb_meniscus = us_pushbutton( tr( "Specify Meniscus" ), false );
   connect( pb_meniscus, SIGNAL( clicked() ), SLOT( set_meniscus() ) );
   specs->addWidget( pb_meniscus, s_row, 0, 1, 2 );

   le_meniscus = us_lineedit( "", 1 );
   le_meniscus->setReadOnly( true );
   specs->addWidget( le_meniscus, s_row++, 2, 1, 2 );

   // Data range row
   pb_dataRange = us_pushbutton( tr( "Specify Data Range" ), false );
   connect( pb_dataRange, SIGNAL( clicked() ), SLOT( set_dataRange() ) );
   specs->addWidget( pb_dataRange, s_row, 0, 1, 2 );

   le_dataRange = us_lineedit( "", 1 );
   le_dataRange->setReadOnly( true );
   specs->addWidget( le_dataRange, s_row++, 2, 1, 2 );

   // Plataeu row
   pb_plateau = us_pushbutton( tr( "Specify Plateau" ), false );
   connect( pb_plateau, SIGNAL( clicked() ), SLOT( set_plateau() ) );
   specs->addWidget( pb_plateau, s_row, 0, 1, 2 );

   le_plateau = us_lineedit( "", 1 );
   le_plateau->setReadOnly( true );
   specs->addWidget( le_plateau, s_row++, 2, 1, 2 );

   // Baseline row
   pb_baseline = us_pushbutton( tr( "Specify Baseline" ), false );
   connect( pb_baseline, SIGNAL( clicked() ), SLOT( set_baseline() ) );
   specs->addWidget( pb_baseline, s_row, 0, 1, 2 );

   le_baseline = us_lineedit( "", 1 );
   le_baseline->setReadOnly( true );
   specs->addWidget( le_baseline, s_row++, 2, 1, 2 );

   // Noise
   pb_noise = us_pushbutton( tr( "Determine RI Noise" ), false );
   connect( pb_noise, SIGNAL( clicked() ), SLOT( noise() ) );
   specs->addWidget( pb_noise, s_row, 0, 1, 2 );

   pb_residuals = us_pushbutton( tr( "Subtract Noise" ), false );
   connect( pb_residuals, SIGNAL( clicked() ), SLOT( subtract_residuals() ) );
   specs->addWidget( pb_residuals, s_row++, 2, 1, 2 );

   pb_invert = us_pushbutton( tr( "Invert Sign" ), false );
   connect( pb_invert, SIGNAL( clicked() ), SLOT( invert_values() ) );
   specs->addWidget( pb_invert, s_row, 0, 1, 2 );
  
   pb_spikes = us_pushbutton( tr( "Remove Spikes" ), false );
   connect( pb_spikes, SIGNAL( clicked() ), SLOT( remove_spikes() ) );
   specs->addWidget( pb_spikes, s_row++, 2, 1, 2 );
   
   pb_priorEdits = us_pushbutton( tr( "Apply Prior Edits" ), false );
   connect( pb_priorEdits, SIGNAL( clicked() ), SLOT( apply_prior() ) );
   specs->addWidget( pb_priorEdits, s_row, 0, 1, 2 );

   pb_undo = us_pushbutton( tr( "Undo Noise and Spikes" ), false );
   connect( pb_undo, SIGNAL( clicked() ), SLOT( undo() ) );
   specs->addWidget( pb_undo, s_row++, 2, 1, 2 );

   pb_float = us_pushbutton( tr( "Mark Data as Floating" ), false );
   connect( pb_float, SIGNAL( clicked() ), SLOT( floating() ) );
   specs->addWidget( pb_float, s_row, 0, 1, 2 );

   pb_write = us_pushbutton( tr( "Save Current Edit Profile" ), false );
   connect( pb_write, SIGNAL( clicked() ), SLOT( write() ) );
   specs->addWidget( pb_write, s_row++, 2, 1, 2 );

   // Button rows
   QBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_accept );

   // Plot layout on right side of window
   QBoxLayout* plot = new US_Plot( data_plot, 
         tr( "Absorbance Data" ),
         tr( "Radius (in cm)" ), tr( "Absorbance" ) );
   
   data_plot->setMinimumSize( 600, 400 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );

   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );

   left->addLayout( specs );
   left->addStretch();
   left->addLayout( buttons );

   main->addLayout( left );
   main->addLayout( plot );

   //partial_reset = false;
   reset();
}

void US_Edvabs::reset( void )
{
   changes_made = false;
   floatingData = false;

   step          = MENISCUS;
   meniscus      = 0.0;
   meniscus_left = 0.0;
   range_left    = 0.0;
   range_right   = 9.0;
   plateau       = 0.0;
   baseline      = 0.0;
   invert        = 1.0;
   noise_order   = 0;

   le_info     ->setText( "" );
   le_meniscus ->setText( "" );
   le_dataRange->setText( "" );
   le_plateau  ->setText( "" );
   le_baseline ->setText( "" );

   ct_from->disconnect();
   ct_from->setMinValue( 0 );
   ct_from->setMaxValue( 0 );
   ct_from->setValue   ( 0 );

   ct_to->disconnect();
   ct_to->setMinValue( 0 );
   ct_to->setMaxValue( 0 );
   ct_to->setValue   ( 0 );

   cb_triple->disconnect();
   cb_triple->clear();

   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   data_plot->detachItems( QwtPlotItem::Rtti_PlotMarker );
   pick     ->disconnect();

   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   grid = us_grid( data_plot );
   data_plot->replot();

   // Disable pushbuttons
   pb_details     ->setEnabled( false );
   pb_gaps        ->setEnabled( false );

   pb_excludeRange->setEnabled( false );
   pb_exclusion   ->setEnabled( false );
   pb_include     ->setEnabled( false );
   pb_edit1       ->setEnabled( false );
   
   pb_meniscus    ->setEnabled( false );
   pb_dataRange   ->setEnabled( false );
   pb_plateau     ->setEnabled( false );
   pb_baseline    ->setEnabled( false );
   
   pb_noise       ->setEnabled( false );
   pb_residuals   ->setEnabled( false );
   pb_spikes      ->setEnabled( false );
   pb_invert      ->setEnabled( false );
   pb_priorEdits  ->setEnabled( false );
   pb_undo        ->setEnabled( false );
   
   pb_float       ->setEnabled( false );
   pb_write       ->setEnabled( false );

   // Remove icons
   pb_meniscus    ->setIcon( QIcon() );
   pb_dataRange   ->setIcon( QIcon() );
   pb_plateau     ->setIcon( QIcon() );
   pb_baseline    ->setIcon( QIcon() );
   pb_noise       ->setIcon( QIcon() );
   pb_residuals   ->setIcon( QIcon() );
   pb_spikes      ->setIcon( QIcon() );
   pb_invert      ->setIcon( QIcon() );

   pb_float       ->setIcon( QIcon() );
   pb_write       ->setIcon( QIcon() );

   allData       .clear();
   editID        .clear();
   data.scanData .clear();
   includes      .clear();
   changed_points.clear();

   set_pbColors( NULL );
}

void US_Edvabs::details( void )
{  
   US_RunDetails* dialog 
      = new US_RunDetails( allData, runID, workingDir, triples );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

void US_Edvabs::gap_check( void )
{  
   int threshold = (int)ct_gaps->value();
            
   US_DataIO::scan s;
   QString         gaps;

   int scanNumber = 1;

//int j = 0;
   foreach ( s, data.scanData )
   {
      int maxGap    = 0;
      int gapLength = 0;
      int location;

      for ( int i = 0; i < s.readings.size(); i++ )
      {
         int byte = i / 8;
         int bit  = i % 8;

         if ( s.interpolated[ byte ]  &  1 << 7 - bit )
           gapLength++;
         else
           gapLength = 0;

         //maxGap = max( maxGap, gapLength );
         if ( gapLength > maxGap )
         {
            //qDebug() << i;
            maxGap   = gapLength;
            location = i;
         }
      }

// For testing      
threshold = 0;

      if ( maxGap >= threshold )
      { 
         double radius = s.readings[ 0 ].d.radius + location * 0.001;

         gaps += tr( "Scan " ) + QString::number( scanNumber ) + 
                 tr( " has a maximum reading gap of " ) + QString::number( maxGap ) +
                 tr( " starting at radius " ) + 
                 QString::number( radius, 'f', 3 ) + "\n";
      }
                             
      //qDebug() << "scan " << j++ << " maxgap is " << maxGap;

      scanNumber++;
   }

   if ( ! gaps.isEmpty() )
      QMessageBox::information( this,
            tr( "Excessive Data Gaps" ),
            tr( "The following have data gaps exceeding the defined"
                " threshold\n\n" ) + gaps );
}

void US_Edvabs::load( void )
{  
   // Ask for data directory
   workingDir = QFileDialog::getExistingDirectory( this, 
         tr("Raw Data Directory"),
         US_Settings::resultDir(),
         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

   if ( workingDir.isEmpty() ) return; 

   reset();
   workingDir.replace( "\\", "/" );  // WIN32 issue

   QStringList components =  workingDir.split( "/", QString::SkipEmptyParts );  
   
   runID = components.last();

   QStringList nameFilters = QStringList( "*.auc" );

   QDir d( workingDir );

   files =  d.entryList( nameFilters, 
         QDir::Files | QDir::Readable, QDir::Name );

   if ( files.size() == 0 )
   {
      QMessageBox::warning( this,
            tr( "No Files Found" ),
            tr( "There were no files of the form *.auc\n"  
                "found in the specified directory." ) );
      return;
   }

   // Look for cell / channel / wavelength combinations
   for ( int i = 0; i < files.size(); i++ )
   {
      QStringList part = files[ i ].split( "." );

      QString t = part[ 2 ] + " / " + part[ 3 ] + " / " + part[ 4 ];
      if ( ! triples.contains( t ) ) triples << t;
   }

   cb_triple->addItems( triples );
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
                       SLOT  ( new_triple         ( int ) ) );
   triple_index = 0;
   
   le_info->setText( runID );

   // Read all data
   if ( workingDir.right( 1 ) != "/" ) workingDir += "/"; // Ensure trailing /

   QString file;
   foreach ( file, files )
   {
      QString filename = workingDir + file;
      
      int result = US_DataIO::readRawData( filename, data );
      if ( result != US_DataIO::OK )
      {
         QMessageBox::warning( this,
            tr( "UltraScan Error" ),
            tr( "Could not read data file.  Error: " ) 
            + QString::number( result ) + "\n" + filename );
      }

      allData << data;
      data.scanData.clear();
   }

   if ( allData.isEmpty() )
   {
      QMessageBox::warning( this,
         tr( "UltraScan Error" ),
         tr( "Could not read any data file." ) );
      return;
   }

   data = allData[ 0 ];
   plot_current( 0 );

   // Enable pushbuttons
   pb_details   ->setEnabled( true );
   pb_gaps      ->setEnabled( true );
   pb_include   ->setEnabled( true );
   pb_exclusion ->setEnabled( true );
   pb_meniscus  ->setEnabled( true );
   pb_dataRange ->setEnabled( false );
   pb_plateau   ->setEnabled( false );
   pb_noise     ->setEnabled( false );
   pb_baseline  ->setEnabled( false );
   pb_spikes    ->setEnabled( false );
   pb_invert    ->setEnabled( true );
   pb_priorEdits->setEnabled( true );
   pb_float     ->setEnabled( true );
   pb_undo      ->setEnabled( true );

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to,   SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   step = MENISCUS;
   set_pbColors( pb_meniscus );
}

void US_Edvabs::set_pbColors( QPushButton* pb )
{
   QPalette p = US_GuiSettings::pushbColor();
   
   pb_meniscus ->setPalette( p );
   pb_dataRange->setPalette( p );
   pb_plateau  ->setPalette( p );
   pb_baseline ->setPalette( p );

   if ( pb != NULL )
   {
      p.setColor( QPalette::Button, Qt::green );
      pb->setPalette( p );
   }
}

void US_Edvabs::plot_current( int index )
{
   // Read the data
   QString     triple  = cb_triple->currentText();
   QStringList parts   = triple.split( " / " );

   QString     cell    = parts[ 0 ];
   QString     channel = parts[ 1 ];
   QString     wl      = parts[ 2 ];

   QString s = le_info->text() + " (" + data.description  + ")";
   le_info->setText( s );

   // Plot Title
   parts = files[ index ].split( "." );
   
   QString title;

   if ( parts[ 1 ] == "RA" )
   {
      title = "Radial Absorbance Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
   }
   else if ( parts[ 1 ] == "RI" )
   {
      title = "Radial Intensity Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
      data_plot->setAxisTitle( QwtPlot::yLeft, tr( "Intensity " ) );
   }
   else 
      title = "File type not recognized";

   data_plot->setTitle( title );

   // Initialize include list
   init_includes();

   // Plot current data for cell / channel / wavelength triple
   plot_all();

   // Set the Scan spin boxes
   ct_from->setMinValue( 0.0 );
   ct_from->setMaxValue(  data.scanData.size() );

   ct_to  ->setMinValue( 0.0 );
   ct_to  ->setMaxValue(  data.scanData.size() );

   connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                  SLOT  ( mouse   ( const QwtDoublePoint& ) ) );
}

void US_Edvabs::replot( void )
{
   switch( step )
   {
      case PLATEAU:
         plot_range();
         break;

      case BASELINE:
         plot_last();
         break;

      default:
         plot_all();
         break;
   }
}

void US_Edvabs::mouse( const QwtDoublePoint& p )
{
   switch ( step )
   {
      case MENISCUS:
         if ( meniscus_left == 0.0 )
         {
            meniscus_left = p.x();
            draw_vline( meniscus_left );
         }
         else
         {
            // Sometime we get two clicks
            if ( fabs( p.x() - meniscus_left ) < 0.005 ) return;

            double meniscus_right = p.x();
            
            // Swap values if necessary
            if ( meniscus_right < meniscus_left )
            {
               double temp    = meniscus_left;
               meniscus_left  = meniscus_right;
               meniscus_right = temp;
            }

            // Find the radius for the max value
            double          maximum = -1.0e99;
            US_DataIO::scan s;

            foreach ( s, data.scanData )
            {
               int start = US_DataIO::index( s, meniscus_left  );
               int end   = US_DataIO::index( s, meniscus_right );

               for ( int j = start; j <= end; j++ )
               {
                  if ( maximum < s.readings[ j ].value )
                  {
                     maximum  = s.readings[ j ].value;
                     meniscus = s.readings[ j ].d.radius;
                  }
               }
            }
            
            // Display the value
            QString m;
            le_meniscus->setText( m.sprintf( "%.3f", meniscus ) );

            // Remove the left line
            v_line->detach();
            delete v_line;
            v_line = NULL;

            // Create a marker
            marker = new QwtPlotMarker;
            QBrush brush( Qt::white );
            QPen   pen  ( brush, 2.0 );
            
            marker->setValue( meniscus, maximum );
            marker->setSymbol( QwtSymbol( 
                        QwtSymbol::Cross, 
                        brush,
                        pen,
                        QSize ( 8, 8 ) ) );

            marker->attach( data_plot );
            data_plot->replot();

            pb_meniscus->setIcon( check );
            pb_dataRange->setEnabled( true );
            next_step();
         }         
         break;

      case RANGE:
         if ( range_left == 0.0 )
         {
            range_left = p.x();
            draw_vline( range_left );
         }
         else
         {
            // Sometime we get two clicks
            if ( fabs( p.x() - range_left ) < 0.020 ) return;

            range_right = p.x();

            if ( range_right < range_left )
            {
               double temp = range_left;
               range_left  = range_right;
               range_right = temp;
            }

            // Display the data
            QString s;
            le_dataRange->setText( s.sprintf( "%.3f - %.3f", 
                     range_left, range_right ) );

            plot_range();
            pb_dataRange->setIcon( check );
            pb_plateau  ->setEnabled( true );
            pb_noise    ->setEnabled( true );
            pb_baseline ->setEnabled( true );
            pb_spikes   ->setEnabled( true );
            next_step();
         }

         break;

      case PLATEAU:

         plateau = p.x();

         // Display the data (localize str)
         {
            QString str;
            le_plateau->setText( str.sprintf( "%.3f", plateau ) );
         }

         plot_last();
         pb_plateau ->setIcon( check );
         pb_baseline->setEnabled( true );
         ct_to->setValue( 0.0 );  // Uncolor all scans
         next_step();
         break;

      case BASELINE:
         {
            // Use the last scan
            US_DataIO::scan s = data.scanData.last();
            
            int start = US_DataIO::index( s, range_left );
            int end   = US_DataIO::index( s, range_right );
            int pt    = US_DataIO::index( s, p.x() );

            if ( pt - start < 5  ||  end - pt < 5 )
            {
               QMessageBox::warning( this,
                  tr( "Position Error" ),
                  tr( "The selected point is too close to the edge." ) );
               return;
            }

            double sum = 0.0;

            // Average the value for +/- 5 points
            for ( int j = pt - 5; j <= pt + 5; j++ )
               sum += s.readings[ j ].value;

            double bl = sum / 11.0;
            baseline  = p.x();

            QString str;
            le_baseline->setText( str.sprintf( "%.3f (%.3e)", p.x(), bl ) );
         }

         pb_write      ->setEnabled( true );
         pb_baseline   ->setIcon   ( check );
         next_step();
         break;

      default:
         break;
   }
}

void US_Edvabs::draw_vline( double radius )
{
   double r[ 2 ];

   r[ 0 ] = radius;
   r[ 1 ] = radius;

   QwtScaleDiv* y_axis = data_plot->axisScaleDiv( QwtPlot::yLeft );

   double padding = ( y_axis->upperBound() - y_axis->lowerBound() ) / 30.0;

   double v[ 2 ];
   v [ 0 ] = y_axis->upperBound() - padding;
   v [ 1 ] = y_axis->lowerBound() + padding;

   v_line = us_curve( data_plot, "V-Line" );
   v_line->setData( r, v, 2 );

   QPen pen = QPen( QBrush( Qt::white ), 2.0 );
   v_line->setPen( pen );

   data_plot->replot();
}

void US_Edvabs::next_step( void )
{
   QPushButton* pb;
   
   if      ( meniscus    == 0.0 ) 
   {
      step = MENISCUS;
      pb   = pb_meniscus;
   }
   else if ( range_right == 9.0 ) 
   {
      step = RANGE;
      pb   = pb_dataRange;
   }
   else if ( plateau == 0.0 ) 
   {
      step = PLATEAU;
      pb   = pb_plateau;
   }
   else if ( baseline    == 0.0 ) 
   {
      step = BASELINE;
      pb   = pb_baseline;
   }
   else
   {
      step = FINISHED;
      pb   = NULL;
   }

   set_pbColors( pb );
}

void US_Edvabs::set_meniscus( void )
{
   le_meniscus ->setText( "" );
   le_dataRange->setText( "" );
   le_plateau  ->setText( "" );
   le_baseline ->setText( "" );
   
   meniscus      = 0.0;
   meniscus_left = 0.0;
   range_left    = 0.0;
   range_right   = 9.0;
   plateau       = 0.0;
   baseline      = 0.0;
   
   step          = MENISCUS;
   
   set_pbColors( pb_meniscus );
   pb_meniscus->setIcon( QIcon() );

   pb_dataRange->setEnabled( false );
   pb_dataRange->setIcon( QIcon() );
   pb_plateau  ->setEnabled( false );
   pb_plateau  ->setIcon( QIcon() );
   pb_baseline ->setEnabled( false );
   pb_baseline ->setIcon( QIcon() );
   pb_write    ->setEnabled( false );
   pb_write    ->setIcon( QIcon() );

   spikes = false;
   pb_spikes   ->setEnabled( false );
   pb_spikes   ->setIcon( QIcon() );

   plot_all();
}

void US_Edvabs::set_dataRange( void )
{
   le_dataRange->setText( "" );
   le_plateau  ->setText( "" );
   le_baseline ->setText( "" );
   
   range_left    = 0.0;
   range_right   = 9.0;
   plateau       = 0.0;
   baseline      = 0.0;
   
   step        = RANGE;
   set_pbColors( pb_dataRange );

   pb_dataRange->setIcon( QIcon() );
   pb_plateau  ->setEnabled( false );
   pb_plateau  ->setIcon( QIcon() );
   pb_baseline ->setEnabled( false );
   pb_baseline ->setIcon( QIcon() );
   pb_write    ->setEnabled( false );
   pb_write    ->setIcon( QIcon() );;

   spikes = false;
   pb_spikes   ->setEnabled( false );
   pb_spikes   ->setIcon( QIcon() );

   plot_all();
}

void US_Edvabs::set_plateau( void )
{
   le_plateau  ->setText( "" );
   le_baseline ->setText( "" );
   
   plateau       = 0.0;
   baseline      = 0.0;
   
   step = PLATEAU;
   set_pbColors( pb_plateau );

   pb_plateau  ->setIcon( QIcon() );
   pb_baseline ->setEnabled( false );
   pb_baseline ->setIcon( QIcon() );
   pb_write    ->setEnabled( false );
   pb_write    ->setIcon( QIcon() );;

   plot_range();
}

void US_Edvabs::set_baseline( void )
{
   le_baseline->setText( "" );
   baseline  = 0.0;
   step      = BASELINE;
   set_pbColors( pb_baseline );

   pb_baseline ->setIcon( QIcon() );
   pb_write    ->setEnabled( false );
   pb_write    ->setIcon( QIcon() );;
   
   // Only display last curve
   plot_last();
}

void US_Edvabs::plot_all( void )
{
   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve ); 

   int size = data.scanData[ 0 ].readings.size();

   double* r = new double[ size ];
   double* v = new double[ size ];

   double maxR = -1.0e99;
   double minR =  1.0e99;
   double maxV = -1.0e99;
   double minV =  1.0e99;

   for ( int i = 0; i < data.scanData.size(); i++ )
   {
      if ( ! includes.contains( i ) ) continue;
      
      US_DataIO::scan* s = &data.scanData[ i ];

      for ( int j = 0; j < size; j++ )
      {
         r[ j ] = s->readings[ j ].d.radius;
         v[ j ] = s->readings[ j ].value * invert;

         maxR = max( maxR, r[ j ] );
         minR = min( minR, r[ j ] );
         maxV = max( maxV, v[ j ] );
         minV = min( minV, v[ j ] );
      }

      QString title = tr( "Raw Data at " )
         + QString::number( s->seconds ) + tr( " seconds" );

      QwtPlotCurve* c = us_curve( data_plot, title );
      c->setPaintAttribute( QwtPlotCurve::ClipPolygons, true );
      c->setData( r, v, size );
   }

   // Reset the scan curves within the new limits
   double padR = ( maxR - minR ) / 30.0;
   double padV = ( maxV - minV ) / 30.0;

   data_plot->setAxisScale( QwtPlot::yLeft  , minV - padV, maxV + padV );
   data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );

   // Reset colors
   focus( (int)ct_from->value(), (int)ct_to->value() );
   data_plot->replot();

   delete [] r;
   delete [] v;
}

void US_Edvabs::plot_range( void )
{
   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   
   double maxR = -1.0e99;
   double minR =  1.0e99;
   double maxV = -1.0e99;
   double minV =  1.0e99;

   // For each scan
   for ( int i = 0; i < data.scanData.size(); i++ )
   {
      if ( ! includes.contains( i ) ) continue;
      
      US_DataIO::scan* s = &data.scanData[ i ];
      
      int indexLeft  = US_DataIO::index( *s, range_left );
      int indexRight = US_DataIO::index( *s, range_right );
      
      int     count  = 0;
      uint    size   = s->readings.size();
      double* r      = new double[ size ];
      double* v      = new double[ size ];
      
      for ( int j = indexLeft; j <= indexRight; j++ )
      {
         r[ count ] = s->readings[ j ].d.radius;
         v[ count ] = s->readings[ j ].value * invert;

         maxR = max( maxR, r[ count ] );
         minR = min( minR, r[ count ] );
         maxV = max( maxV, v[ count ] );
         minV = min( minV, v[ count ] );

         count++;
      }

      QString title = tr( "Raw Data at " )
         + QString::number( s->seconds ) + tr( " seconds" );

      QwtPlotCurve* c = us_curve( data_plot, title );
      c->setData( r, v, count );

      delete [] r;
      delete [] v;
   }

   // Reset the scan curves within the new limits
   double padR = ( maxR - minR ) / 30.0;
   double padV = ( maxV - minV ) / 30.0;

   data_plot->setAxisScale( QwtPlot::yLeft  , minV - padV, maxV + padV );
   data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );

   // Reset colors
   focus( (int)ct_from->value(), (int)ct_to->value() );
   data_plot->replot();
}

void US_Edvabs::plot_last( void )
{
   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   //grid = us_grid( data_plot ); 

   double maxR = -1.0e99;
   double minR =  1.0e99;
   double maxV = -1.0e99;
   double minV =  1.0e99;

   // Plot only the last scan
   US_DataIO::scan* s = &data.scanData[ includes.last() ];;
   
   int indexLeft  = US_DataIO::index( *s, range_left );
   int indexRight = US_DataIO::index( *s, range_right );
   
   int     count  = 0;
   uint    size   = s->readings.size();
   double* r      = new double[ size ];
   double* v      = new double[ size ];
   
   for ( int j = indexLeft; j <= indexRight; j++ )
   {
      r[ count ] = s->readings[ j ].d.radius;
      v[ count ] = s->readings[ j ].value * invert;

      maxR = max( maxR, r[ count ] );
      minR = min( minR, r[ count ] );
      maxV = max( maxV, v[ count ] );
      minV = min( minV, v[ count ] );

      count++;
   }

   QString title = tr( "Raw Data at " )
      + QString::number( s->seconds ) + tr( " seconds" );

   QwtPlotCurve* c = us_curve( data_plot, title );
   c->setData( r, v, count );

   // Reset the scan curves within the new limits
   double padR = ( maxR - minR ) / 30.0;
   double padV = ( maxV - minV ) / 30.0;

   data_plot->setAxisScale( QwtPlot::yLeft  , minV - padV, maxV + padV );
   data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );

   // Reset colors
   focus( (int)ct_from->value(), (int)ct_to->value() );
   data_plot->replot();

   delete [] r;
   delete [] v;
}

void US_Edvabs::focus_from( double scan )
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

void US_Edvabs::focus_to( double scan )
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

void US_Edvabs::focus( int from, int to )
{
   if ( from == 0 )
      pb_edit1  ->setEnabled( false );
   else
      pb_edit1  ->setEnabled( true );

   if ( to == 0 )
      pb_excludeRange->setEnabled( false );
   else
      pb_excludeRange->setEnabled( true );

   QList< int > focus;  // We don't care if -1 is in the list
   for ( int i = from - 1; i <= to - 1; i++ ) focus << i;  

   set_colors( focus );
}

void US_Edvabs::set_colors( const QList< int >& focus )
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
         b.setColor( Qt::red );
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

void US_Edvabs::init_includes( void )
{
   includes.clear();
   for ( int i = 0; i < data.scanData.size(); i++ ) includes << i;
}

void US_Edvabs::reset_excludes( void )
{
   ct_from->disconnect();
   ct_from->setValue   ( 0 );
   ct_from->setMaxValue( includes.size() );
   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   ct_to->disconnect();
   ct_to->setValue   ( 0 );
   ct_to->setMaxValue( includes.size() );
   connect( ct_to, SIGNAL( valueChanged ( double ) ),
                   SLOT  ( focus_to   ( double ) ) );

   pb_excludeRange->setEnabled( false );
   pb_edit1       ->setEnabled( false );

   replot();
}

void US_Edvabs::exclude_range( void )
{
   int scanStart = (int)ct_from->value();
   int scanEnd   = (int)ct_to  ->value();

   for ( int i = scanEnd; i >= scanStart; i-- )
      includes.removeAt( i - 1 );

   replot();
   reset_excludes();
}

void US_Edvabs::exclusion( void )
{
   reset_excludes();
   US_ExcludeProfile* exclude = new US_ExcludeProfile( includes );
 
   connect( exclude, SIGNAL( update_exclude_profile( QList< int > ) ), 
            this   , SLOT  ( update_excludes       ( QList< int > ) ) );
   
   connect( exclude, SIGNAL( cancel_exclude_profile( void ) ), 
            this   , SLOT  ( cancel_excludes       ( void ) ) );

   connect( exclude, SIGNAL( finish_exclude_profile( QList< int > ) ), 
            this   , SLOT  ( finish_excludes       ( QList< int > ) ) );

   exclude->exec();
   qApp->processEvents();
   delete exclude;
}

void US_Edvabs::update_excludes( QList< int > scanProfile )
{
   set_colors( scanProfile );
}

void US_Edvabs::cancel_excludes( void )
{
   QList< int > focus;
   set_colors( focus );  // Focus on no curves
}

void US_Edvabs::finish_excludes( QList< int > excludes )
{
   for ( int i = 0; i < excludes.size(); i++ )
      includes.removeAll( excludes[ i ] );

   replot();
   reset_excludes();
}

void US_Edvabs::edit_scan( void )
{
   int index1 = (int)ct_from->value();
   int scan  = includes[ index1 - 1 ];

   US_EditScan* dialog = new US_EditScan( data.scanData[ scan ], 
         invert, range_left, range_right );
   connect( dialog, SIGNAL( scan_updated( QList< QPointF > ) ),
                    SLOT  ( update_scan ( QList< QPointF > ) ) );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

void US_Edvabs::update_scan( QList< QPointF > changes )
{
   // Handle excluded scans
   int              index1        = (int)ct_from->value();
   int              current_scan  = includes[ index1 - 1 ];
   US_DataIO::scan* s             = &data.scanData[ current_scan ];

   for ( int i = 0; i < changes.size(); i++ )
   {
      int    point = (int)changes[ i ].x();
      double value =      changes[ i ].y();

      s->readings[ point ].value = value;
   }

   // Save changes for writing output
   edits e;
   e.scan    = current_scan;
   e.changes = changes;
   changed_points << e;

   // Set data for the curve
   int     points = s->readings.size();
   double* r      = new double[ points ];
   double* v      = new double[ points ];

   int left;
   int right;
   int count = 0;

   if ( range_left > 0 )
   {
      left  = US_DataIO::index( *s, range_left  );
      right = US_DataIO::index( *s, range_right );
   }
   else
   {
      left  = 0;
      right = points - 1;
   }

   for ( int i = left; i <= right; i++ )
   {
      r[ count ] = s->readings[ i ].d.radius;
      v[ count ] = s->readings[ i ].value;
      count++;
   }

   // Find the pointer to the current scan
   QwtPlotItemList items   = data_plot->itemList();
   QString         seconds = " " + QString::number( s->seconds );
   bool            found   = false;

   QwtPlotCurve* c;
   for ( int i = 0; i < items.size(); i++ )
   {
      if ( items[ i ]->rtti() == QwtPlotItem::Rtti_PlotCurve )
      {
         c = dynamic_cast< QwtPlotCurve* >( items[ i ] );
         if ( c->title().text().contains( seconds ) )
         {
            found = true;
            break;
         }
      }
   }

   if ( found ) 
   {
      // Update the curve
      c->setData( r, v, count );
      data_plot->replot();
   }
   else
      qDebug() << "Can't find curve!";

   delete [] r;
   delete [] v;
}

void US_Edvabs::include( void )
{
   init_includes();
   reset_excludes();
}

void US_Edvabs::invert_values( void )
{
   if ( invert == 1.0 )
   {
      invert = -1.0;
      pb_invert->setIcon( check );
   }
   else
   {
      invert = 1.0;
      pb_invert->setIcon( QIcon() );
   }

   replot();
}

bool US_Edvabs::spike_check( const US_DataIO::scan& s, 
      int point, int start, int end, double* value )
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

   // If there is more than a 3-fold difference, it is a spike
   double val    =  s.readings[ point ].value;
   double radius =  s.readings[ point ].d.radius;

   if ( fabs( slope * radius + intercept - val ) > threshold * sigma )
   {
      // Interpolate
      *value = slope * radius + intercept;
      changes_made = true;
      return true;
   }

   return false;  // Not a spike
}

void US_Edvabs::remove_spikes( void )
{
   double smoothed_value;

   // For each scan
   for ( int i = 0; i < data.scanData.size(); i++ ) 
   {
      US_DataIO::scan* s = &data.scanData [ i ];
      int start  = US_DataIO::index( *s, range_left  );
      int end    = US_DataIO::index( *s, range_right );
      
      // Note that a changed point can affect succeeding points

      for ( int j = start; j < start + 5; j++ ) // Beginning 5 points
      {
         if ( spike_check( *s, j, start, start + 10, &smoothed_value ) )
            s->readings[ j ].value = smoothed_value;
      }

      for ( int j = start + 5; j < end - 4; j++ ) // Middle points
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

   pb_spikes->setIcon   ( check );
   pb_spikes->setEnabled( false );
   replot();
}

void US_Edvabs::undo( void )
{
   // Copy from allData to data
   int index = cb_triple->currentIndex();
   data = allData[ index ];
   replot();

   // Reset buttons and structures
   pb_residuals->setEnabled( false );

   if ( step < PLATEAU )
   {
      pb_noise ->setEnabled( false );
      pb_spikes->setEnabled( false );
   }
   else
   {
      pb_noise ->setEnabled( true );
      pb_spikes->setEnabled( true );
   }

   spikes      = false;
   noise_order = 0;

   // Remove icons
   pb_noise       ->setIcon( QIcon() );
   pb_residuals   ->setIcon( QIcon() );
   pb_spikes      ->setIcon( QIcon() );
}

void US_Edvabs::noise( void )
{
   residuals.clear();
   US_RiNoise* dialog = new US_RiNoise( data, noise_order, residuals );
   int code = dialog->exec();
   qApp->processEvents();

   if ( code == QDialog::Accepted )
   {
      pb_noise    ->setIcon( check );
      pb_residuals->setEnabled( true );
   }
   else
      pb_residuals->setEnabled( false );

   delete dialog;
}

void US_Edvabs::subtract_residuals( void )
{
   for ( int i = 0; i < data.scanData.size(); i++ )
   {
     for ( int j = 0; j <  data.scanData[ i ].readings.size(); j++ )
     {
         data.scanData[ i ].readings[ j ].value -= residuals[ i ];
     }
   }

   pb_residuals->setEnabled( false );
   pb_residuals->setIcon   ( check );
   replot();
}

void US_Edvabs::new_triple( int index )
{
   if ( changes_made )
   {
      QMessageBox mb;
      mb.setIcon( QMessageBox::Question );
      mb.setText( tr( "Ignore Edits?" ) );
      mb.setInformativeText( 
            tr( "Edits have been made.  If you want to keep them,\n"
                "cancel and write the outputs first." ) );
      mb.addButton( tr( "Ignore Edits" ), QMessageBox::RejectRole );
      mb.addButton( tr( "Return to previous selection" ), QMessageBox::NoRole );
      int result = mb.exec();

      if ( result == QMessageBox::RejectRole )
      {
         cb_triple->disconnect();
         cb_triple->setCurrentIndex( triple_index );
         connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
                             SLOT  ( new_triple         ( int ) ) );
         return;
      }
   }

   triple_index = index;
   reset();
   data = allData[ index ];
   plot_current( index );

   // Enable pushbuttons
   pb_details  ->setEnabled( true );
   pb_include  ->setEnabled( true );
   pb_exclusion->setEnabled( true );
   pb_meniscus ->setEnabled( true );
   pb_dataRange->setEnabled( true );
   pb_plateau  ->setEnabled( true );
   pb_noise    ->setEnabled( true );
   pb_baseline ->setEnabled( true );
   pb_spikes   ->setEnabled( true );
   pb_invert   ->setEnabled( true );
   pb_undo     ->setEnabled( true );

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to,   SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   step = MENISCUS;
   set_pbColors( pb_meniscus );
}

void US_Edvabs::floating( void )
{
   floatingData = ! floatingData;
   if ( floatingData )
      pb_float->setIcon( check );
   else
      pb_float->setIcon( QIcon() );

}
void US_Edvabs::write( void )
{
   QString s;

   // Check if complete
   if ( meniscus == 0.0 )
      s = tr( "meniscus" );
   else if ( range_left == 0.0 || range_right == 9.0 )
      s = tr( "data range" );
   else if ( plateau == 0.0 )
      s = tr( "plateau" );
   else if ( baseline == 0.0 )
      s = tr( "baseline" );

   if ( ! s.isEmpty() )
   {
      QMessageBox::information( this,
            tr( "Date missing" ),
            tr( "You must define the " ) + s 
            + tr( "before writing the edit profile." ) );
      return;
   }

   // Ask for editID if not yet defined
   if ( editID.isEmpty() )
   {
      QString today =  QDate::currentDate().toString( "yyyyMMdd" );

      bool ok;
      editID = QInputDialog::getText( this, 
         tr( "Input a unique Edit ID for this edit session" ),
         tr( "Use alphanumeric characters, underscores, or hyphens (no spaces)" ),
         QLineEdit::Normal,
         today, &ok);
      
      if ( ! ok ) return;

      editID.remove( QRegExp( "[^\\w\\d_-]" ) );
   }

   // Determine file name
   // workingDir + runID + editID + data type + cell + channel + wavelength 
   //            + ".xml"

   QString filename = files[ cb_triple->currentIndex() ];
   int     index = filename.indexOf( '.' ) + 1;
   filename.insert( index, editID + "." );
   filename.replace( QRegExp( "auc$" ), "xml" );

   QFile f( workingDir + filename );

   if ( ! f.open( QFile::WriteOnly | QFile::Text ) )
   {
      QMessageBox::information( this,
            tr( "File write error" ),
            tr( "Could not open the file\n" ) + workingDir + filename
            + tr( "\n for writing.  Check your permissions." ) );
      return;
   }

   QTextStream ts( &f );

   QDomDocument doc( "UltraScanEdits" );
   QDomElement root = doc.createElement( "experiment" );
   doc.appendChild( root );

   // Write identification
   QDomElement id = doc.createElement( "identification" );
   root.appendChild( id );

   QDomElement runid = doc.createElement( "runid" );
   runid.setAttribute( "value", runID );
   id.appendChild( runid );

   QDomElement guid = doc.createElement( "uuid" );
   char uuid[ 37 ];
   uuid_unparse( (unsigned char*)data.guid, uuid );
   guid.setAttribute( "value", uuid );
   id.appendChild( guid );

   QString     triple  = cb_triple->currentText();
   QStringList parts   = triple.split( " / " );

   QString     cell    = parts[ 0 ];
   QString     channel = parts[ 1 ];
   QString     wl      = parts[ 2 ];

   QDomElement run = doc.createElement( "run" );
   run.setAttribute( "cell"      , cell );
   run.setAttribute( "channel"   , channel );
   run.setAttribute( "wavelength", wl );
   root.appendChild( run );

   // Write excluded scans
   if ( data.scanData.size() > includes.size() )
   {
      QDomElement excludes = doc.createElement( "excludes" );
      run.appendChild( excludes );

      for ( int i = 0; i < data.scanData.size(); i++ )
      {
         if ( ! includes.contains( i ) )
         {
            QDomElement exclude = doc.createElement( "exclude" );
            exclude.setAttribute( "scan", i );
            excludes.appendChild( exclude );
         }
      }
   }

   // Write edits
   if ( ! changed_points.isEmpty() )
   {
      QDomElement edited = doc.createElement( "edited" );
      run.appendChild( edited );

      for ( int i = 0; i < changed_points.size(); i++ )
      {
         edits* e = &changed_points[ i ];

         for ( int j = 0; j < e->changes.size(); j++ )
         {
            QDomElement edit = doc.createElement( "edit" );
            edit.setAttribute( "scan"  , e->scan );
            edit.setAttribute( "radius", e->changes[ j ].x() );
            edit.setAttribute( "value" , e->changes[ j ].y() );
            edited.appendChild( edit );
         }
      }
   }

   // Write meniscus, range, plataeu, baseline
   QDomElement parameters = doc.createElement( "parameters" );
   run.appendChild( parameters );

   QDomElement m = doc.createElement( "meniscus" );
   m.setAttribute( "radius", meniscus );
   parameters.appendChild( m );

   QDomElement dataRange = doc.createElement( "data_range" );
   dataRange.setAttribute( "left" , range_left );
   dataRange.setAttribute( "right", range_right );
   parameters.appendChild( dataRange );

   QDomElement p = doc.createElement( "plateau" );
   p.setAttribute( "radius" , plateau );
   parameters.appendChild( p );

   QDomElement bl = doc.createElement( "baseline" );
   bl.setAttribute( "radius", baseline );
   parameters.appendChild( bl );
   
   QDomElement operations = doc.createElement( "operations" );
   run.appendChild( operations );
 
   // Write RI Noise
   if ( ! pb_residuals->icon().isNull() )
   {
      QDomElement riNoise = doc.createElement( "subtract_ri_noise" );
      riNoise.setAttribute( "order", noise_order );
      operations.appendChild( riNoise );
   }

   // Write Remove Spikes
   if ( ! pb_spikes->icon().isNull() )
   {
      QDomElement spikes = doc.createElement( "remove_spikes" );
      operations.appendChild( spikes );
   }

   // Write Invert
   if ( invert == -1.0 )
   {
      QDomElement invert = doc.createElement( "invert" );
      operations.appendChild( invert );
   }

   // Write indication of floating data
   if ( floatingData )
   {
      QDomElement floating = doc.createElement( "floating_data" );
      operations.appendChild( floating );
   }

   const int indentSize = 4;
   doc.save( ts, indentSize );

   f.close();

   changes_made = false;
   pb_write->setEnabled( false );
   pb_write->setIcon   ( check );
}

void US_Edvabs::apply_prior( void )
{
   QString filter = files[ cb_triple->currentIndex() ];
   int     index1 = filter.indexOf( '.' ) + 1;

   filter.insert( index1, "*." );
   filter.replace( QRegExp( "auc$" ), "xml" );
   
   // Ask for edit file
   QString filename = QFileDialog::getOpenFileName( this, 
         tr( "Select a saved edit file" ),
         workingDir, filter );

   if ( filename.isEmpty() ) return; 

   // Read the edits
   US_DataIO::editValues parameters;

   int result = US_DataIO::readEdits( filename, parameters );

   if ( result != US_DataIO::OK )
   {
      QMessageBox::warning( this,
            tr( "XML Error" ),
            tr( "An error occurred when reading edit file\n\n" ) 
            +  US_DataIO::errorString( result ) );
      return;
   }

   char uuid[ 37 ];
   uuid_unparse( (unsigned char*)data.guid, uuid );

   if ( parameters.uuid != uuid )
   {
      QMessageBox::warning( this,
            tr( "Data Error" ),
            tr( "The edit file was not created using the current data" ) );
      return;
   }
   
   // Reset data from allData
   index1 = cb_triple->currentIndex();
   data = allData[ index1 ];

   // Apply the edits
   // Specified parameters
   QString s;

   meniscus = parameters.meniscus;
   le_meniscus->setText( s.sprintf( "%.3f", meniscus ) );
   pb_meniscus->setIcon( check );
   pb_meniscus->setEnabled( true );

   range_left  = parameters.rangeLeft;
   range_right = parameters.rangeRight;
   
   le_dataRange->setText( s.sprintf( "%.3f - %.3f",
           range_left, range_right ) );
   pb_dataRange->setIcon( check );
   pb_dataRange->setEnabled( true );
   
   plateau = parameters.plateau;
   le_plateau->setText( s.sprintf( "%.3f", plateau ) );
   pb_plateau->setIcon( check );
   pb_plateau->setEnabled( true );

   baseline = parameters.baseline;
   
   US_DataIO::scan scan  = data.scanData.last();
   int             pt    = US_DataIO::index( scan, baseline );
   double          sum   = 0.0;

   // Average the value for +/- 5 points
   for ( int j = pt - 5; j <= pt + 5; j++ )
      sum += scan.readings[ j ].value;

   le_baseline->setText( s.sprintf( "%.3f (%.3e)", baseline, sum / 11.0 ) );
   pb_baseline->setIcon( check );
   pb_baseline->setEnabled( true );

   // Invert
   invert = parameters.invert;
   
   if ( invert == -1.0 ) pb_invert->setIcon( check );
   else                  pb_invert->setIcon( QIcon() );

   // Excluded scans
   init_includes();
   reset_excludes(); // Zero exclude combo boxes
   qSort( parameters.excludes );

   for ( int i = parameters.excludes.size(); i > 0; i-- )
      includes.removeAt( parameters.excludes[ i - 1 ] );

   // Edited points
   changed_points.clear();

   for ( int i = 0; i < parameters.editedPoints.size(); i++ )
   {
      int    scan   = parameters.editedPoints[ i ].scan;
      int    index1 = (int)parameters.editedPoints[ i ].radius;
      double value  = parameters.editedPoints[ i ].value;
      
      edits e;
      e.scan = scan;
      e.changes << QPointF( index1, value );
     
      changed_points << e;
      
      data.scanData[ scan ].readings[ index1 ].value = value;
   }

   // Spikes
   spikes = parameters.removeSpikes;

   pb_spikes->setIcon( QIcon() );
   pb_spikes->setEnabled( true );
   if ( spikes ) remove_spikes();
   
   // Noise
   noise_order = parameters.noiseOrder;
   if ( noise_order > 0 )
   {
      US_RiNoise::calc_residuals( data, noise_order, residuals );
      subtract_residuals();
   }
   else
   {
      pb_noise    ->setIcon( QIcon() );
      pb_residuals->setIcon( QIcon() );
      pb_residuals->setEnabled( false );
   }

   // Floating data
   floatingData = parameters.floatingData;
   if ( floatingData )
      pb_float->setIcon( check );
   else
      pb_float->setIcon( QIcon() );


   step        = FINISHED;
   set_pbColors( NULL );

   pb_undo ->setEnabled( true );
   pb_write->setEnabled( true );

   changes_made= false;
   plot_range();
}

