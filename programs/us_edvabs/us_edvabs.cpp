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
   step          = MENISCUS;
   meniscus_left = 0.0;
   range_left    = 0.0;
   range_right   = 0.0;
   plateau       = 0.0;
   baseline      = 0.0;

   data.scanData.clear();
   cellList.clear();

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
   
   // Row 7
   // Exclude pushbuttons
   pb_exclude = us_pushbutton( tr( "Exclude Single Scan" ), false );
   specs->addWidget( pb_exclude, s_row, 0, 1, 2 );

   pb_excludeRange = us_pushbutton( tr( "Exclude Scan Range" ), false );
   specs->addWidget( pb_excludeRange, s_row++, 2, 1, 2 );
   
   // Row 8

   pb_exclusion = us_pushbutton( tr( "Exclusion Profile" ), false );
   specs->addWidget( pb_exclusion, s_row, 0, 1, 2 );

   pb_edit1 = us_pushbutton( tr( "Edit Single Scan" ), false );
   specs->addWidget( pb_edit1, s_row++, 2, 1, 2 );

   // Row 9
   QLabel* lb_edit = us_banner( tr( "Edit Controls" ) );
   specs->addWidget( lb_edit, s_row++, 0, 1, 4 );

   // Meniscus row
   pb_meniscus = us_pushbutton( tr( "Specify Meniscus" ), false );
   //connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   specs->addWidget( pb_meniscus, s_row, 0, 1, 2 );

   le_meniscus = us_lineedit( "", 1 );
   le_meniscus->setReadOnly( true );
   specs->addWidget( le_meniscus, s_row++, 2, 1, 2 );

   // Data range row
   pb_dataRange = us_pushbutton( tr( "Specify Data Range" ), false );
   //connect( pb_dataRange, SIGNAL( clicked() ), SLOT( help() ) );
   specs->addWidget( pb_dataRange, s_row, 0, 1, 2 );

   le_dataRange = us_lineedit( "", 1 );
   le_dataRange->setReadOnly( true );
   specs->addWidget( le_dataRange, s_row++, 2, 1, 2 );

   // Plataeu row
   pb_plateau = us_pushbutton( tr( "Specify Plateau" ), false );
   //connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   specs->addWidget( pb_plateau, s_row, 0, 1, 2 );

   le_plateau = us_lineedit( "", 1 );
   le_plateau->setReadOnly( true );
   specs->addWidget( le_plateau, s_row++, 2, 1, 2 );

   // Baseline row
   pb_baseline = us_pushbutton( tr( "Specify Baseline" ), false );
   //connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
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

   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );

   left->addLayout( specs );
   left->addStretch();
   left->addLayout( buttons );

   main->addLayout( left );
   main->addLayout( plot );
}

void US_Edvabs::reset( void )
{
   le_info     ->setText( "" );
   le_meniscus ->setText( "" );
   le_dataRange->setText( "" );
   le_plateau  ->setText( "" );
   le_baseline ->setText( "" );

   ct_from->setMinValue( 0 );
   ct_from->setMaxValue( 0 );
   ct_from->setValue   ( 0 );

   ct_to->setMinValue( 0 );
   ct_to->setMaxValue( 0 );
   ct_to->setValue   ( 0 );

   cb_cell      ->clear();
   cb_channel   ->clear();
   cb_wavelength->clear();

   ct_noise->setValue( 4 );

   data_plot->clear();
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   data_plot->replot();

   // Disable pushbuttons
   pb_details     ->setEnabled( false );
   pb_exclude     ->setEnabled( false );
   pb_excludeRange->setEnabled( false );
   pb_exclusion   ->setEnabled( false );
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

   // Clear the plot
   data_plot->disconnect();
   data_plot->detachItems();

   step = MENISCUS;
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


   QStringList components = 
      workingDir.split( QRegExp( "[/\\\\]" ), QString::SkipEmptyParts );  
   
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

   pb_meniscus ->setEnabled( true );
   pb_dataRange->setEnabled( true );
   pb_plateau  ->setEnabled( true );
   pb_baseline ->setEnabled( true );

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

   QString filename = workingDir + files[ index ];
   int result = US_DataIO::readRawData( filename, data );
   if ( result != US_DataIO::OK )
   {
      QMessageBox::warning( this,
         tr( "UltraScan Error" ),
         tr( "Could not read data file.  Error: " ) 
         + QString::number( result ) );
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

   // Plot Axes

   // All the plot radii will be the same

   double rmin = data.scanData[ 0 ].values.front().d.radius - 0.010; // Padding
   double rmax = data.scanData[ 0 ].values.back() .d.radius + 0.010;
   double vmin =  1.0e99;
   double vmax = -1.0e99;

   for ( uint i = 0; i < data.scanData.size(); i++ )
   {
      for ( uint j = 0; j < data.scanData[ i ].values.size(); j++ )
      {
         vmin = min( vmin, data.scanData[ i ].values[ j ].value );
         vmax = max( vmax, data.scanData[ i ].values[ j ].value );
      }
   }

   vmin = min( vmin * 0.98, vmin * 1.02 );  // Handle sign for padding
   vmax = max( vmax * 0.98, vmax * 1.02 ); 

   data_plot->setAxisScale( QwtPlot::yLeft  , vmin, vmax );
   data_plot->setAxisScale( QwtPlot::xBottom, rmin, rmax );

   // Plot Curves

   grid = us_grid( data_plot );

   int size = data.scanData[ 0 ].values.size();

   double* r = new double[ size ];
   double* v = new double[ size ];

   for ( uint i = 0; i < data.scanData.size(); i++ )
   {
      
      for ( int j = 0; j < size; j++ )
      {
         r[ j ] = data.scanData[ i ].values[ j ].d.radius;
         v[ j ] = data.scanData[ i ].values[ j ].value;
      }

      QString title = tr( "Raw Data at " )
         + QString::number( data.scanData[ i ].seconds ) + tr( " seconds" );

      QwtPlotCurve* c = us_curve( data_plot, title );
      c->setData( r, v, size );
   }

   data_plot->replot();

   delete r;
   delete v;

   // Set the Scan spin boxes
   ct_from->setMinValue( 1.0 );
   ct_from->setMaxValue(  data.scanData.size() );

   ct_to  ->setMinValue( 1.0 );
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
   else if ( plateau     == 0.0 ) 
   {
      step = PLATAEU;
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
qDebug() << "step" << step;
   set_pbColors( pb );
}

