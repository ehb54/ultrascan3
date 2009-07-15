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
   specs->addWidget( cb_cell, s_row, 1 );

   QLabel* lb_channel = us_label( tr( "Channel:" ), -1 );
   specs->addWidget( lb_channel, s_row, 2 );

   cb_channel = us_comboBox();
   specs->addWidget( cb_channel, s_row++, 3 );
   
   // Row 4
   QLabel* lb_wavelength = us_label( tr( "Wavelength:" ), -1 );
   specs->addWidget( lb_wavelength, s_row, 0 );

   cb_wavelength = us_comboBox();
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
   specs->addWidget( ct_from, s_row, 1 );

   QLabel* lb_to = us_label( tr( "to:" ), -1 );
   lb_to->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lb_to, s_row, 2 );

   ct_to = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
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
}

/*
void US_FitMeniscus::plot_data( void )
{
   meniscus_plot->clear();

   QString contents = te_data->e->toPlainText();
   contents.remove( QRegExp( "[^0-9\\.,\\n]" ) );

   QStringList lines = contents.split( "\n", QString::SkipEmptyParts );
   QStringList parsed;

   double* radius_values = new double[ lines.size() ];
   double* rmsd_values   = new double[ lines.size() ];
   
   int     count = 0;

   double  minx = 1e20;
   double  maxx = 0.0;

   double  miny = 1e20;
   double  maxy = 0.0;

   // Remove any non-data lines and put values in arrays
   for ( int i = 0; i < lines.size(); i++ )
   {
      QStringList values = lines[ i ].split( ',', QString::SkipEmptyParts );
      if ( values.size() > 1 ) 
      {
         radius_values[ count ] = values[ 0 ].toDouble();
         rmsd_values  [ count ] = values[ 1 ].toDouble();

         // Find min and max
         minx = min( minx, radius_values[ count ] );
         maxx = max( maxx, radius_values[ count ] );

         miny = min( miny, rmsd_values[ count ] );
         maxy = max( maxy, rmsd_values[ count ] );

         // Reformat
         parsed << QString::number( radius_values[ count ], 'f', 4 ) + ", " +
                   QString::number( rmsd_values  [ count ], 'f', 4 ); 

         count++;
      }
   }

   te_data->e->setPlainText( parsed.join( "\n" ) );

   double overscan = ( maxx - minx ) * 0.10;  // 10% overscan

   meniscus_plot->setAxisScale( QwtPlot::xBottom, 
         minx - overscan, maxx + overscan );
    
   // Adjust y axis to scale all the data
   double dy = fabs( maxy - miny ) / 10.0;

   meniscus_plot->setAxisScale( QwtPlot::yLeft, miny - dy, maxy + dy );

   raw_curve = us_curve( meniscus_plot, tr( "Raw Data" ) ); 
   raw_curve->setPen( QPen( Qt::yellow ) );

   raw_curve->setData( radius_values, rmsd_values, count );

   // Do the fit and get the minimum

   double c[ 10 ];

   int order = sb_order->value();

   US_Matrix::lsfit( c, radius_values, rmsd_values, count, order + 1 );

   int fit_count = (int) ( ( maxx - minx + 2 * overscan ) / 0.001 );

   double* fit_x = new double[ fit_count ];
   double* fit_y = new double[ fit_count ];
   double  x     = minx - overscan;
   double minimum;

   for ( int i = 0; i < fit_count; i++, x += 0.001 )
   {
      fit_x[ i ] = x;
      fit_y[ i ] = c[ 0 ];

      for ( int j = 1; j <= order; j++ ) 
         fit_y[ i ] += c[ j ] * pow( x, j );
   }

   // Calculate Root Mean Square Error
   double rms_err = 0.0;

   for ( int i = 0; i < count; i++ )
   {
      double x = radius_values[ i ];
      double y = rmsd_values  [ i ];

      double y_calc = c[ 0 ];
      
      for ( int j = 1; j <= order; j++ )  
         y_calc += c[ j ] * pow( x, j );
      
      rms_err += sq ( fabs ( y_calc - y ) );
   }

   le_rms_error->setText( QString::number( sqrt( rms_err / count ), 'e', 3 ) );

   // Find the minimum
   if ( order == 2 )
   {
      // Take the derivitive and get the minimum
      // c1 + 2 * c2 * x = 0
      minimum = - c[ 1 ] / ( 2.0 * c[ 2 ] );
   }
   else
   {
      // Find the zero of the derivitive
      double dxdy  [ 9 ];
      double d2xdy2[ 8 ];

      // First take the derivitive
      for ( int i = 0; i < order; i++ ) 
         dxdy[ i ] = c[ i + 1 ] * ( i + 1 );

      // And we'll need the 2nd derivitive
      for ( int i = 0; i < order - 1; i++ ) 
         d2xdy2[ i ] = dxdy[ i + 1 ] * ( i + 1 );

      // We'll do a quadratic fit for the initial estimate
      double q[ 3 ];
      US_Matrix::lsfit( q, radius_values, rmsd_values, count, 3 );
      minimum = - q[ 1 ] / ( 2.0 * q[ 2 ] );

      const double epsilon = 1.0e-4;

      int    k = 0;
      double f;
      double f_prime;
      do
      {
        // f is the 1st derivitive
        f = dxdy[ 0 ];
        for ( int i = 1; i < order; i++ ) f += dxdy[ i ] * pow( minimum, i );

        // f_prime is the 2nd derivitive
        f_prime = d2xdy2[ 0 ];
        for ( int i = 1; i < order - 1; i++ ) 
           f_prime += d2xdy2[ i ] * pow( minimum, i );

        if ( fabs( f ) < epsilon ) break;
        if ( k++ > 10 ) break;

        // Get the next estimate
        minimum -= f / f_prime;

      } while ( true );
   }

   fit_curve = us_curve( meniscus_plot, tr( "Fitted Data" ) ); 
   fit_curve->setPen( QPen( Qt::red ) );
   fit_curve->setData( fit_x, fit_y, fit_count );
   
   // Plot the minimum

   minimum_curve = us_curve( meniscus_plot, tr( "Minimum Pointer" ) ); 
   minimum_curve->setPen( QPen( QBrush( Qt::cyan ), 3.0 ) );

   double radius_min[ 2 ];
   double rmsd_min  [ 2 ];

   radius_min[ 0 ] = minimum;
   radius_min[ 1 ] = minimum;

   rmsd_min  [ 0 ] = miny - 1.0 * dy;
   rmsd_min  [ 0 ] = miny + 2.0 * dy;

   minimum_curve->setData( radius_min, rmsd_min, 2 );

   // Put the minimum in the line edit box also
   le_fit->setText( QString::number( minimum, 'f', 5 ) );

   // Add the marker label -- bold, font size default + 1, lines 3 pixels wide
   QPen markerPen( QBrush( Qt::white ), 3.0 );
   markerPen.setWidth( 3 );
   
   QwtPlotMarker* pm = new QwtPlotMarker();
   QwtText        label( QString::number( minimum, 'f', 5 ) );
   QFont          font( pm->label().font() );

   font.setBold( true );
   font.setPointSize( font.pointSize() + 1 );
   label.setFont( font );

   pm->setValue( minimum, miny + 3.0 * dy );
   pm->setSymbol( QwtSymbol( QwtSymbol::Cross, 
            QBrush( Qt::white ), markerPen, QSize( 9, 9 ) ) );
   pm->setLabel( label );
   pm->setLabelAlignment( Qt::AlignTop );

   pm->attach( meniscus_plot );

   meniscus_plot->replot();
}
*/
void US_Edvabs::load( void )
{  
   reset();

   // Ask for data directory
   QString dir = QFileDialog::getExistingDirectory( this, 
         tr("Raw Data Directory"),
         US_Settings::dataDir(),
         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

   if ( dir.isEmpty() ) return; 


   QStringList components = dir.split( QRegExp( "[/\\\\]" ), QString::SkipEmptyParts );  
   
   QString runID = components.last();

   QStringList nameFilters = ( QStringList() << runID + ".?.?.?*" );

   QStringList files = QDir::entryList( nameFilters, 
         QDir::Files | QDir::Readable, QDir::Name );

   // For each file matching format, read into local structure
   for ( int i = 0; i < files.size; i++ )
   {
      // US_DataIO::readRawDataBlob( files[ i ], data structure for input data );
      //QFile f = QFile( files[ i ] );
      //f.open( QIODevice::ReadOnly );
      //QDataStream ds( &f );

      //ds.writeRawData( char_ptr, length );
      // ...


      //f.close();
   }

   qDebug() << "comp : " << components;
   qDebug() << "dir  : " << dir;
   qDebug() << "runID: " << runID;

   // Update fields

}
