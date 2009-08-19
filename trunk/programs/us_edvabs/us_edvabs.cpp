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

   setWindowTitle( tr( "Edit Velocity Absorbance Data" ) );
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
   //cb_triple->setInsertPolicy( QComboBox::InsertAlphabetically );
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
                       SLOT  ( new_triple         ( int ) ) );
   specs->addWidget( cb_triple, s_row++, 2, 1, 2 );

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
   pb_exclude = us_pushbutton( tr( "Exclude Single Scan" ), false );
   connect( pb_exclude, SIGNAL( clicked() ), SLOT( exclude_one() ) );
   specs->addWidget( pb_exclude, s_row, 0, 1, 2 );

   pb_excludeRange = us_pushbutton( tr( "Exclude Scan Range" ), false );
   connect( pb_excludeRange, SIGNAL( clicked() ), SLOT( exclude_range() ) );
   specs->addWidget( pb_excludeRange, s_row++, 2, 1, 2 );
   
   // Row 8
   pb_exclusion = us_pushbutton( tr( "Exclusion Profile" ), false );
   connect( pb_exclusion, SIGNAL( clicked() ), SLOT( exclusion() ) );
   specs->addWidget( pb_exclusion, s_row, 0, 1, 2 );

   pb_edit1 = us_pushbutton( tr( "Edit Single Scan" ), false );
   connect( pb_edit1, SIGNAL( clicked() ), SLOT( edit_scan() ) );
   specs->addWidget( pb_edit1, s_row++, 2, 1, 2 );

   // Row 9
   pb_include = us_pushbutton( tr( "Include All" ), false );
   connect( pb_include, SIGNAL( clicked() ), SLOT( include() ) );
   specs->addWidget( pb_include, s_row++, 0, 1, 4 );

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

   pb_subBaseline = us_pushbutton( tr( "Subtract Baseline" ), false );
   connect( pb_subBaseline, SIGNAL( clicked() ), SLOT( subtract_baseline() ) );
   specs->addWidget( pb_subBaseline, s_row, 0, 1, 2 );
   
   pb_spikes = us_pushbutton( tr( "Remove Spikes" ), false );
   connect( pb_spikes, SIGNAL( clicked() ), SLOT( remove_spikes() ) );
   specs->addWidget( pb_spikes, s_row++, 2, 1, 2 );
   
   pb_invert = us_pushbutton( tr( "Invert Sign" ), false );
   connect( pb_invert, SIGNAL( clicked() ), SLOT( invert_values() ) );
   specs->addWidget( pb_invert, s_row, 0, 1, 2 );
  
   pb_undo = us_pushbutton( tr( "Undo Edits" ), false );
   connect( pb_undo, SIGNAL( clicked() ), SLOT( undo() ) );
   specs->addWidget( pb_undo, s_row++, 2, 1, 2 );

   pb_write = us_pushbutton( tr( "Save Current Edit Profile" ), false );
   connect( pb_write, SIGNAL( clicked() ), SLOT( write() ) );
   specs->addWidget( pb_write, s_row++, 0, 1, 4 );

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

   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );

   left->addLayout( specs );
   left->addStretch();
   left->addLayout( buttons );

   main->addLayout( left );
   main->addLayout( plot );

   partial_reset = false;
   reset();
}

void US_Edvabs::reset( void )
{
   changes_made = false;

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


   data_plot->detachItems();
   pick     ->disconnect();

   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   grid = us_grid( data_plot );
   data_plot->replot();

   // Disable pushbuttons
   pb_details     ->setEnabled( false );

   pb_exclude     ->setEnabled( false );
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
   pb_subBaseline ->setEnabled( false );
   pb_spikes      ->setEnabled( false );
   pb_invert      ->setEnabled( false );
   pb_undo        ->setEnabled( false );
   
   pb_write       ->setEnabled( false );

   // Remove icons
   pb_noise       ->setIcon( QIcon() );
   pb_residuals   ->setIcon( QIcon() );
   pb_subBaseline ->setIcon( QIcon() );
   pb_spikes      ->setIcon( QIcon() );
   pb_invert      ->setIcon( QIcon() );

   pb_write       ->setIcon( QIcon() );

   // We use a class variable to pass a parameter to avoid
   // clearing all data when selecting a different triple
   if ( ! partial_reset )
   {
      // Clear the raw data structures
      for ( int j = 0; j < allData.size(); j++ )
         clear_rawData( allData[ j ] );

      allData.clear();
      editID.clear();
   }

   clear_rawData( data, false );

   includes.clear();
   changed_points.clear();

   step          = MENISCUS;
   meniscus      = 0.0;
   meniscus_left = 0.0;
   range_left    = 0.0;
   range_right   = 0.0;
   plateau.clear();
   baseline      = 0.0;
   invert        = 1.0;
   noise_order   = 0;
}

void US_Edvabs::clear_rawData( rawData& raw, bool deleteFlag )
{  
   for ( uint j = 0; j < raw.scanData.size(); j++ )
   {
      // When we copy from allData to data, the pointers to the interpolated
      // array are copied.  The flag prevents a double deletion.
      if ( deleteFlag ) delete raw.scanData[ j ].interpolated;
      raw.scanData[ j ].values.clear();
   }

   raw.scanData.clear();
}

void US_Edvabs::details( void )
{  
   int index = cb_triple->currentIndex();
   US_RunDetails* dialog 
      = new US_RunDetails( allData, index, runID, workingDir, triples );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

void US_Edvabs::load( void )
{  
   // Ask for data directory
   workingDir = QFileDialog::getExistingDirectory( this, 
         tr("Raw Data Directory"),
         US_Settings::dataDir(),
         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

   if ( workingDir.isEmpty() ) return; 

   //reset();
   workingDir.replace( "\\", "/" );  // WIN32 issue

   QStringList components =  workingDir.split( "/", QString::SkipEmptyParts );  
   
   runID = components.last();

   QStringList nameFilters = QStringList( runID + ".*.auc" );

   QDir d( workingDir );

   files =  d.entryList( nameFilters, 
         QDir::Files | QDir::Readable, QDir::Name );

   if ( files.size() == 0 )
   {
      QMessageBox::warning( this,
            tr( "No Files Found" ),
            tr( "There were no files of the form:\n" ) + nameFilters[ 0 ]
            + tr( "\nfound in the specified directory." ) );
      return;
   }

   // Look for cell / channel / wavelength combinations
   
   for ( int i = 0; i < files.size(); i++ )
   {
      QStringList part = files[ i ].split( "." );

      QString t = part[ 2 ] + " / " + part[ 3 ] + " / " + part[ 4 ];
      if ( ! triples.contains( t ) ) triples << t;
   }

   cb_triple->disconnect();
   cb_triple->clear();
   cb_triple->addItems( triples );
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
                       SLOT  ( new_triple         ( int ) ) );
   triple_index = 0;
   
   le_info->setText( runID );

   // Read all data
   if ( workingDir.right( 1 ) != "/" ) workingDir += "/"; // Ensure trailing /

   for ( int i = 0; i < files.size(); i++ )
   {
      QString filename = workingDir + files[ i ];
      
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

      case BASELINE:
         plot_last();

      default:
         plot_all();
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
            double maximum = -1.0e99;
            
            for ( uint i = 0; i < data.scanData.size(); i++ )
            {
               scan* s = &data.scanData[ i ];

               int start = index( s, meniscus_left  );
               int end   = index( s, meniscus_right );

               for ( int j = start; j <= end; j++ )
               {
                  if ( maximum < s->values[ j ].value )
                  {
                     maximum  = s->values[ j ].value;
                     meniscus = s->values[ j ].d.radius;
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
            next_step();
         }

         break;

      case PLATEAU:

         plateau.clear();

         for ( uint i = 0; i < data.scanData.size(); i++ )
         {
            scan* s = &data.scanData[ i ];
            int start = index( s, range_left );
            int end   = index( s, range_right );
            int pt    = index( s, p.x() );

            if ( pt - start < 5  ||  end - pt < 5 )
            {
               plateau.clear();
               QMessageBox::warning( this,
                  tr( "Position Error" ),
                  tr( "The selected point is too close to the edge." ) );
               return;
            }

            double sum = 0.0;

            // Average the vaule for +/- 5 points
            for ( int j = pt - 5; j <= pt + 5; j++ )
               sum += s->values[ j ].value;

            plateau << sum / 11.0;
         }
         
         // Display the data (localize str )
         {
            QString str;
            le_plateau->setText( str.sprintf( "%.3f", p.x() ) );
         }

         plot_last();
         pb_plateau->setIcon( check );
         next_step();
         break;

      case BASELINE:
         {
            // Use the last scan
            scan* s = &data.scanData[ data.scanData.size() -1 ];
            
            int start = index( s, range_left );
            int end   = index( s, range_right );
            int pt    = index( s, p.x() );

            if ( pt - start < 5  ||  end - pt < 5 )
            {
               QMessageBox::warning( this,
                  tr( "Position Error" ),
                  tr( "The selected point is too close to the edge." ) );
               return;
            }

            double sum = 0.0;

            // Average the vaule for +/- 5 points
            for ( int j = pt - 5; j <= pt + 5; j++ )
               sum += s->values[ j ].value;

            baseline = sum / 11.0;

            QString str;
            le_baseline->setText( str.sprintf( "%.3f (%.3e)", p.x(), baseline ) );

         }

         pb_subBaseline->setEnabled( true );
         pb_write      ->setEnabled( true );
         pb_baseline   ->setIcon   ( check );
         next_step();
         break;

      default:
         break;
   }
}

int US_Edvabs::index( scan* s, double r )
{
   for ( uint i = 0; i < s->values.size(); i++ )
   {
      if ( fabs( s->values[ i ].d.radius - r ) < 5.0e-4 ) return i;
   }

   return -1;
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
   else if ( range_right == 0.0 ) 
   {
      step = RANGE;
      pb   = pb_dataRange;
   }
   else if ( plateau.isEmpty() ) 
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
   le_meniscus->setText( "" );
   meniscus      = 0.0;
   meniscus_left = 0.0;
   step          = MENISCUS;
   set_pbColors( pb_meniscus );
   pb_meniscus->setIcon( QIcon() );
   pb_write   ->setEnabled( false );
   plot_all();
}

void US_Edvabs::set_dataRange( void )
{
   if ( meniscus == 0.0 )
   {
      QMessageBox::warning( this,
            tr( "Sequence Error" ),
            tr( "You must select the meniscus bfore the data range." ) );
      return;
   }

   le_dataRange->setText( "" );
   range_left  = 0.0;
   range_right = 0.0;
   step        = RANGE;
   set_pbColors( pb_dataRange );
   pb_dataRange->setIcon( QIcon() );
   pb_write    ->setEnabled( false );
   plot_all();
}

void US_Edvabs::set_plateau( void )
{
   if ( range_left == 0.0 || range_right == 0.0 )
   {
      QMessageBox::warning( this,
            tr( "Sequence Error" ),
            tr( "You must select the data range before the plateau." ) );
      return;
   }

   le_plateau->setText( "" );
   plateau.clear();
   step = PLATEAU;
   set_pbColors( pb_plateau );
   pb_plateau->setIcon( QIcon() );
   pb_write  ->setEnabled( false );
   plot_range();
}

void US_Edvabs::set_baseline( void )
{
   if ( range_left == 0.0 || range_right == 0.0 )
   {
      QMessageBox::warning( this,
            tr( "Sequence Error" ),
            tr( "You must select the data range before the baseline." ) );
      return;
   }

   le_baseline->setText( "" );
   baseline  = 0.0;
   step      = BASELINE;
   set_pbColors( pb_baseline );

   pb_baseline   ->setIcon( QIcon() );
   pb_subBaseline->setEnabled( false );
   pb_write      ->setEnabled( false );
   
   // Only display last curve
   plot_last();
}

void US_Edvabs::plot_all( void )
{
   data_plot->detachItems(); 
   grid = us_grid( data_plot );

   int size = data.scanData[ 0 ].values.size();

   double* r = new double[ size ];
   double* v = new double[ size ];

   double maxR = -1.0e99;
   double minR =  1.0e99;
   double maxV = -1.0e99;
   double minV =  1.0e99;

   for ( uint i = 0; i < data.scanData.size(); i++ )
   {
      if ( ! includes.contains( i ) ) continue;
      scan* s = &data.scanData[ i ];

      for ( int j = 0; j < size; j++ )
      {
         r[ j ] = s->values[ j ].d.radius;
         v[ j ] = s->values[ j ].value * invert;

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

void US_Edvabs::plot_range( void )
{
   data_plot->detachItems();
   grid = us_grid( data_plot ); 
   
   double maxR = -1.0e99;
   double minR =  1.0e99;
   double maxV = -1.0e99;
   double minV =  1.0e99;

   // For each scan
   for ( uint i = 0; i < data.scanData.size(); i++ )
   {
      if ( ! includes.contains( i ) ) continue;
      scan*   s     = &data.scanData[ i ];
      int     count = 0;
      uint    size  = s->values.size();
      double* r     = new double[ size ];
      double* v     = new double[ size ];
      
      for ( uint j = 0; j < size; j++ )
      {
         if ( s->values[ j ].d.radius >= range_left &&
              s->values[ j ].d.radius <= range_right )
         {
            r[ count ] = s->values[ j ].d.radius;
            v[ count ] = s->values[ j ].value * invert;

            maxR = max( maxR, r[ count ] );
            minR = min( minR, r[ count ] );
            maxV = max( maxV, v[ count ] );
            minV = min( minV, v[ count ] );

            count++;
         }
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

   data_plot->replot();
}

void US_Edvabs::plot_last( void )
{
   data_plot->detachItems();
   grid = us_grid( data_plot ); 

   double maxR = -1.0e99;
   double minR =  1.0e99;
   double maxV = -1.0e99;
   double minV =  1.0e99;

   // Plot only the last scan
   scan*   s     = &data.scanData[ includes.last() ];;
   int     count = 0;
   uint    size  = s->values.size();
   double* r     = new double[ size ];
   double* v     = new double[ size ];
   
   for ( uint j = 0; j < size; j++ )
   {
      if ( s->values[ j ].d.radius >= range_left &&
           s->values[ j ].d.radius <= range_right )
      {
         r[ count ] = s->values[ j ].d.radius;
         v[ count ] = s->values[ j ].value * invert;

         maxR = max( maxR, r[ count ] );
         minR = min( minR, r[ count ] );
         maxV = max( maxV, v[ count ] );
         minV = min( minV, v[ count ] );

         count++;
      }
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
   {
      pb_exclude->setEnabled( false );
      pb_edit1  ->setEnabled( false );
   }
   else
   {
      pb_exclude->setEnabled( true );
      pb_edit1  ->setEnabled( true );
   }

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
   for ( uint i = 0; i < data.scanData.size(); i++ ) includes << i;
}

void US_Edvabs::exclude_one( void )
{
   int i = (int)ct_from->value();

   // Offset by 1 for scan number vs index
   includes.removeAt( i - 1 );
   replot();
   reset_excludes();
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
   // Need to handle excluded scans
   int index = (int)ct_from->value();
   int scan  = includes[ index - 1 ];

   US_EditScan* dialog = new US_EditScan( data.scanData[ scan ], invert );
   connect( dialog, SIGNAL( scan_updated( QList< QPointF > ) ),
                    SLOT  ( update_scan ( QList< QPointF > ) ) );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

void US_Edvabs::update_scan( QList< QPointF > changes )
{
   // Need to handle excluded scans
   // Need to save date for write
   int index = (int)ct_from->value();
   int scan  = includes[ index - 1 ];

   for ( int i = 0; i < changes.size(); i++ )
   {
      int    point = (int)changes[ i ].x();
      double value =      changes[ i ].y();

      data.scanData[ scan ].values[ point ].value = value;
   }

   // Save changes for writing output
   edits e;
   e.scan    = scan;
   e.changes = changes;
   changed_points << e;

   data_plot->replot();
}

void US_Edvabs::include( void )
{
   init_includes();
   replot();
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

bool US_Edvabs::spike_check( scan* s, int point, int start, int end, 
      double* value )
{
   static double r[ 20 ];  // Spare room -- normally 10
   static double v[ 20 ];

   double* x = &r[ 0 ];
   double* y = &v[ 0 ];

   double  slope;
   double  intercept;
   double  sigma;
   double  correlation;
   int     count = 0;

   const double threshold = 10.0;

   for ( int k = start; k <= end; k++ ) // For oeach point in the range
   {
      if ( k != point ) // Exclude the probed point from fit
      {
         r[ count ] = s->values[ k ].d.radius;
         v[ count ] = s->values[ k ].value; 
         count++;
      }
   }
      
   US_Math::linefit( &x, &y, &slope, &intercept, &sigma, &correlation, count );

   // If there is more than a 3-fold difference, it is a spike
   double* val    = &s->values[ point ].value;
   double  radius =  s->values[ point ].d.radius;

   if ( fabs( slope * radius + intercept - *val ) > threshold * sigma )
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
// This is working the whole scan.  Perhaps we should be just working 
// from the defined range.  

   double smoothed_value;

   // For each scan
   for ( uint i = 0; i < data.scanData.size(); i++ )
   {
      scan* s      = &data.scanData[ i ];
      int   points = s->values.size();
      
      // Note that a changed point can affect succeeding points
      for ( int j = 0; j < 5; j++ ) // Beginning 5 points
      {
         if ( spike_check( s, j, 0, 10, &smoothed_value ) )
            s->values[ j ].value = smoothed_value;
      }

      for ( int j = 5; j < points - 5; j++ ) // Middle points
      {
         if ( spike_check( s, j, j - 5, j + 5, &smoothed_value ) )
            s->values[ j ].value = smoothed_value;
      }

      for ( int j = points - 5; j < points; j++ ) // Last 5 points
      {
         if ( spike_check( s, j, points - 10, points, &smoothed_value ) )
            s->values[ j ].value = smoothed_value;
      }
   }

   // Don't forget to update the rawData in allData

   // We can probably set up an undo here -- just copy data from 
   // allData back to data.

   pb_spikes->setIcon   ( check );
   pb_spikes->setEnabled( false );
   replot();
}

void US_Edvabs::undo( void )
{
   // Copy from allData to data
   int index = cb_triple->currentIndex();
   data = allData[ index ];

   // Reset buttons and structures
   pb_spikes->setEnabled( true );

   invert      = 1.0;
   spikes      = false;
   noise_order = 0;

   // Remove icons
   pb_noise       ->setIcon( QIcon() );
   pb_residuals   ->setIcon( QIcon() );
   pb_subBaseline ->setIcon( QIcon() );
   pb_spikes      ->setIcon( QIcon() );
   pb_invert      ->setIcon( QIcon() );

   replot();
}

void US_Edvabs::noise( void )
{
   residuals.clear();
   US_RiNoise* dialog = new US_RiNoise( data, noise_order, residuals );
   int code = dialog->exec();
   qApp->processEvents();

   if ( code == QDialog::Accepted )
   {
      pb_noise      ->setIcon( check );
      pb_residuals  ->setEnabled( true );
      qDebug() << residuals;
   }
   else
      pb_residuals->setEnabled( false );

   delete dialog;
}

void US_Edvabs::subtract_residuals( void )
{
   for ( uint i = 0; i < data.scanData.size(); i++ )
   {
     for ( uint j = 0; j <  data.scanData[ i ].values.size(); j++ )
     {
         data.scanData[ i ].values[ j ].value -= residuals[ i ];
     }
   }

   pb_residuals->setEnabled( false );
   pb_residuals->setIcon   ( check );
   replot();
}

void US_Edvabs::subtract_baseline( void )
{
   for ( uint i = 0; i < data.scanData.size(); i++ )
   {
     for ( uint j = 0; j <  data.scanData[ i ].values.size(); j++ )
     {
         data.scanData[ i ].values[ j ].value -= baseline;
     }
   }

   pb_subBaseline->setEnabled( false );
   pb_subBaseline->setIcon   ( check );
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
   partial_reset = true;
   reset();
   partial_reset = false;
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

void US_Edvabs::write( void )
{
   QString s;

   // Check if complete
   if ( meniscus == 0.0 )
      s = tr( "meniscus" );
   else if ( range_right == 0.0 || range_left == 0.0 )
      s = tr( "data range" );
   else if ( plateau.isEmpty() )
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
   // workingDir + runID + editID + data type + cell + channel + wavelength + ".edits"

   QString filename = files[ cb_triple->currentIndex() ];
   int     index = filename.indexOf( '.' ) + 1;
   filename.insert( index, editID + "." );
   filename.replace( QRegExp( "auc$" ), "xml" );

   qDebug() << workingDir << filename;

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
   uuid_unparse( (const unsigned char*)data.guid, uuid );
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
   if ( data.scanData.size() > (uint) includes.size() )
   {
      QDomElement excludes = doc.createElement( "excludes" );
      run.appendChild( excludes );

      for ( uint i = 0; i < data.scanData.size(); i++ )
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
   m.setAttribute( "value", meniscus );
   parameters.appendChild( m );

   QDomElement dataRange = doc.createElement( "data_range" );
   dataRange.setAttribute( "left" , range_left );
   dataRange.setAttribute( "right", range_right );
   parameters.appendChild( dataRange );


   QDomElement p = doc.createElement( "plateaus" );
   parameters.appendChild( p );

   for ( uint i = 0; i < data.scanData.size(); i++ )
   {
      QDomElement element = doc.createElement( "plateau" );
      element.setAttribute( "scan" , i );
      element.setAttribute( "value", plateau[ i ] );
      p.appendChild( element );
   }

   QDomElement bl = doc.createElement( "baseline" );
   bl.setAttribute( "value", baseline );
   parameters.appendChild( bl );
   
   QDomElement operations = doc.createElement( "operations" );
   run.appendChild( operations );
   
   // Write RI Noise
   if ( ! pb_subtract->icon().isNull() )
   {
      QDomElement riNoise = doc.createElement( "subtract_ri_noise" );
      riNoise.setAttribute( "order", noise_order );
      operations.appendChild( riNoise );
   }

   // Write Subtract Baseline
   if ( ! pb_subBaseline->icon().isNull() )
   {
      QDomElement subBaseline = doc.createElement( "subtract_baseline" );
      operations.appendChild( subBaseline );
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

   const int indentSize = 4;
   doc.save( ts, indentSize );

   f.close();

   changes_made = false;
   pb_write->setEnabled( false );
   pb_write->setIcon   ( check );
}
