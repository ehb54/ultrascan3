//! \file us_fit_meniscus_main.cpp

#include <QApplication>

#include "us_edvabs.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"

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
   specs->addWidget( pb_details, s_row++, 2, 1, 2 );

   // Row 2
   QLabel* lb_info = us_label( "Run Info:", -1 );
   specs->addWidget( lb_info, s_row, 0 );

   le_info = us_lineedit( "", 1 );
   le_info->setReadOnly( true );
   specs->addWidget( le_info, s_row++, 1, 1, 3 );

   // Row 3
   QLabel* lb_cell = us_label( tr( "Cell:" ), -1 );
   specs->addWidget( lb_cell, s_row, 0 );

   cb_cell = us_comboBox();
   cb_cell->setInsertPolicy( QComboBox::InsertAlphabetically );
   specs->addWidget( cb_cell, s_row, 1 );

   QLabel* lb_channel = us_label( tr( "Channel:" ), -1 );
   specs->addWidget( lb_channel, s_row, 2 );

   cb_channel = us_comboBox();
   cb_channel->setInsertPolicy( QComboBox::InsertAlphabetically );
   specs->addWidget( cb_channel, s_row++, 3 );
   
   // Row 4
   QLabel* lb_wavelength = us_label( tr( "Wavelength:" ), -1 );
   specs->addWidget( lb_wavelength, s_row, 0 );

   cb_wavelength = us_comboBox();
   cb_wavelength->setInsertPolicy( QComboBox::InsertAlphabetically );
   specs->addWidget( cb_wavelength, s_row++, 1 );
   
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
   
   // Exclude and Include  pushbuttons
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
   QLabel* lb_noise = us_label( tr( "Subtract RI Noise:" ), -1 );
   lb_noise->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lb_noise, s_row, 0 );

   ct_noise = us_counter ( 1, 4.0, 9.0 );
   ct_noise->setStep( 1.0 );
   specs->addWidget( ct_noise, s_row, 1 );

   pb_noise = us_pushbutton( tr( "Subtract Residuals" ), false );
   specs->addWidget( pb_noise, s_row++, 2, 1, 2 );

   pb_subtract = us_pushbutton( tr( "Subtract Baseline" ), false );
   specs->addWidget( pb_subtract, s_row, 0, 1, 2 );
   
   pb_spikes = us_pushbutton( tr( "Remove Spikes" ), false );
   //connect( pb_spikes, SIGNAL( clicked() ), SLOT( help() ) );
   specs->addWidget( pb_spikes, s_row++, 2, 1, 2 );
   
   pb_invert = us_pushbutton( tr( "Invert Sign" ), false );
   //connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   specs->addWidget( pb_invert, s_row, 0, 1, 2 );
  
   pb_write = us_pushbutton( tr( "Save Current Edit Profile" ), false );
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
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );

   grid = us_grid( data_plot );

   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );

   left->addLayout( specs );
   left->addStretch();
   left->addLayout( buttons );

   main->addLayout( left );
   main->addLayout( plot );

   reset();
}

void US_Edvabs::reset( void )
{
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

   cb_cell      ->clear();
   cb_channel   ->clear();
   cb_wavelength->clear();

   ct_noise->setValue( 4 );

   data_plot->clear();
   grid = us_grid( data_plot );
   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );

   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
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
   pb_subtract    ->setEnabled( false );
   pb_spikes      ->setEnabled( false );
   pb_invert      ->setEnabled( false );
   pb_write       ->setEnabled( false );

   // Clear the raw data structure
   for ( uint j = 0; j < data.scanData.size(); j++ )
   {
      delete data.scanData[ j ].interpolated;
      data.scanData[ j ].values.clear();
   }

   data.scanData.clear();

   // Clear the cell info structures
   for ( int i = 0; i < cellList.size(); i++ )
   {
      for ( int j = 0; j < cellList[ i ].channelList.size(); j++ )
         cellList[ i ].channelList[ j ].wavelength.clear();
         
      cellList[ i ].channelList.clear();
   }

   cellList.clear();
   includes.clear();

   step          = MENISCUS;
   meniscus      = 0.0;
   meniscus_left = 0.0;
   range_left    = 0.0;
   range_right   = 0.0;
   plateau.clear();
   baseline      = 0.0;

   // Clear the plot
   data_plot->disconnect();
   data_plot->detachItems();

}

void US_Edvabs::load( void )
{  
   reset();

   // Ask for data directory
   workingDir = QFileDialog::getExistingDirectory( this, 
         tr("Raw Data Directory"),
         US_Settings::dataDir(),
         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

   if ( workingDir.isEmpty() ) return; 

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
      cell     c;
      channels ch;

      QStringList part = files[ i ].split( "." );

      c.cellNum  = part[ 2 ].toInt();
      ch.channel = part[ 3 ].at( 0 );
      double wl  = part[ 4 ].toDouble();

      if ( ! cellList.contains( c ) ) cellList << c;
      
      int ii = cellList.indexOf( c );

      if ( ! cellList[ ii ].channelList.contains( ch ) )  
          cellList[ ii ].channelList << ch;
      
      // Don't need a test here.  The cell/channel/wavelength combination
      // is unique
      int jj = cellList[ ii ].channelList.indexOf( ch );
      cellList[ ii ].channelList[ jj ].wavelength << wl; 
   }

   for ( int i = 0; i < cellList.size(); i++ )
      cb_cell->addItem( QString::number( cellList[ i ].cellNum ) );
   
   le_info->setText( runID );
   set_channels();
   plot_current();

   // Enable pushbuttons

   pb_include  ->setEnabled( true );
   pb_exclusion->setEnabled( true );
   pb_meniscus ->setEnabled( true );
   pb_dataRange->setEnabled( true );
   pb_plateau  ->setEnabled( true );
   pb_baseline ->setEnabled( true );

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
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

void US_Edvabs::set_channels( void )
{
   cell c;
   c.cellNum = cb_cell->currentText().toInt();
   
   int cIndex = cellList.indexOf( c );

   for ( int j = 0; j < cellList[ cIndex ].channelList.size(); j++ )
      cb_channel->addItem( QString( cellList[ cIndex ].channelList[ j ].channel ) );

   set_wavelengths();
}

void US_Edvabs::set_wavelengths( void )
{
   cell c;
   c.cellNum = cb_cell->currentText().toInt();
   
   channels ch;
   ch.channel = cb_channel->currentText().at( 0 );

   int cIndex  = cellList.indexOf( c );
   int chIndex = cellList[ cIndex ].channelList.indexOf( ch );

   int count = cellList[ cIndex ].channelList[ chIndex ].wavelength.size();

   for ( int k = 0; k < count; k++ )
   {
      int wl = (int)cellList[ cIndex ].channelList[ chIndex ].wavelength[ k ];
      cb_wavelength->addItem( QString::number( wl ) );
   }
}

void US_Edvabs::plot_current( void )
{
   // Read the data
   QString cell    = cb_cell      ->currentText();
   QString channel = cb_channel   ->currentText();
   QString wl      = cb_wavelength->currentText();

   QRegExp match( runID + "\\.[A-Z]{2}\\." + cell + "." + channel + "." + wl + ".auc" ); 
   int index = files.indexOf( match );

   if ( index < 0 ) 
   {
      QMessageBox::warning( this,
         tr( "UltraScan Error" ),
         tr( "Internal error.  Could not match cell / channel / wavelength" ) );
      return;
   }

   if ( workingDir.right( 1 ) != "/" ) workingDir += "/"; // Ensure trailing /
   QString filename = workingDir + files[ index ];
   
   int result = US_DataIO::readRawData( filename, data );
   if ( result != US_DataIO::OK )
   {
      QMessageBox::warning( this,
         tr( "UltraScan Error" ),
         tr( "Could not read data file.  Error: " ) 
         + QString::number( result ) + "\n" + filename );
      return;
   }
   
   QString s = le_info->text() + " (" + data.description  + ")";
   le_info->setText( s );

   // Plot Title
   QStringList parts = files[ index ].split( "." );
   QString     title;

   if ( parts[ 1 ] == "RA" )
   {
      title = "Radial Absorbance Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
   }
   else 
      title = "File type not recognized";

   data_plot->setTitle( title );

   // Initialize include map
   init_includes();

   // Plot current data for cell / channel / wavelength tripple
   plot_all();

   // Set the Scan spin boxes
   ct_from->setMinValue( 0.0 );
   ct_from->setMaxValue(  data.scanData.size() );

   ct_to  ->setMinValue( 0.0 );
   ct_to  ->setMaxValue(  data.scanData.size() );


   connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                  SLOT  ( mouse   ( const QwtDoublePoint& ) ) );

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
   
   // Only display last curve
   plot_last();
}

void US_Edvabs::plot_all( void )
{
   data_plot->clear();
   grid = us_grid( data_plot );
   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );

   // Reset the scan curves within the new limits
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->setAxisAutoScale( QwtPlot::xBottom );

   int size = data.scanData[ 0 ].values.size();

   double* r = new double[ size ];
   double* v = new double[ size ];

   for ( uint i = 0; i < data.scanData.size(); i++ )
   {
      scan* s = &data.scanData[ i ];

      for ( int j = 0; j < size; j++ )
      {
         r[ j ] = s->values[ j ].d.radius;
         v[ j ] = s->values[ j ].value;
      }

      QString title = tr( "Raw Data at " )
         + QString::number( s->seconds ) + tr( " seconds" );

      QwtPlotCurve* c = us_curve( data_plot, title );
      c->setData( r, v, size );
   }

   data_plot->replot();

   delete r;
   delete v;
}

void US_Edvabs::plot_range( void )
{
   data_plot->clear();
   grid = us_grid( data_plot ); 
   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::CrossRubberBand );

   // Reset the scan curves within the new limits
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->setAxisAutoScale( QwtPlot::xBottom );

   // For each scan
   for ( uint i = 0; i < data.scanData.size(); i++ )
   {
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
            v[ count ] = s->values[ j ].value;
            count++;
         }
      }

      QString title = tr( "Raw Data at " )
         + QString::number( s->seconds ) + tr( " seconds" );

      QwtPlotCurve* c = us_curve( data_plot, title );
      c->setData( r, v, count );
   }

   data_plot->replot();
}

void US_Edvabs::plot_last( void )
{
   data_plot->clear();
   grid = us_grid( data_plot ); 
   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::CrossRubberBand );

   // Reset the scan curves within the new limits
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->setAxisAutoScale( QwtPlot::xBottom );

   // Plot only the last scan
   scan*   s     = &data.scanData[ data.scanData.size() - 1 ];;
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
         v[ count ] = s->values[ j ].value;
         count++;
      }
   }

   QString title = tr( "Raw Data at " )
      + QString::number( s->seconds ) + tr( " seconds" );

   QwtPlotCurve* c = us_curve( data_plot, title );
   c->setData( r, v, count );

   data_plot->replot();
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

   // Get pointers to curves
   QwtPlotItemList        list = data_plot->itemList();
   QList< QwtPlotCurve* > curves;
   
   // Put a dummy on the front for the indexes to be the same
   curves << dynamic_cast< QwtPlotCurve* >( list[ 0 ] );

   for ( int i = 0; i < list.size(); i++ )
   {
      if ( list[ i ]->title().text().contains( "Raw" ) )
         curves << dynamic_cast< QwtPlotCurve* >( list[ i ] );
   }

   QPen   p   = curves[ 1 ]->pen();
   QBrush b   = curves[ 1 ]->brush();
   QColor std = US_GuiSettings::plotCurve();

   // Mark these scans in red
   for ( int i = 1; i < curves.size(); i++ )
   {
      if ( i >= from  &&  i <= to )
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
   includes.insert( 0, 0 );  // A dummy value so we index at 1
   for ( uint i = 0; i < data.scanData.size(); i++ )
      includes.insert( i + 1, i + 1 );
}

void US_Edvabs::exclude_one( void )
{
   //int scan = (int)ct_from->value();

   QMessageBox::information( this, "Not yet", "Exclude scan implemented yet" );
}

void US_Edvabs::exclude_range( void )
{
   QMessageBox::information( this, "Not yet", "Exclude range not implemented yet" );
}

void US_Edvabs::exclusion( void )
{
   QMessageBox::information( this, "Not yet", "Exclude profile not implemented yet" );
}

void US_Edvabs::edit_scan( void )
{
   QMessageBox::information( this, "Not yet", "Edit Scan not implemented yet" );
}

void US_Edvabs::include( void )
{
   QMessageBox::information( this, "Not yet", "Include not implemented yet" );
}

