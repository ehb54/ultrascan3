//! \file us_fit_meniscus_main.cpp

#include <QApplication>

#include "us_edvabs.h"
#include "us_license_t.h"
#include "us_license.h"
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
   specs = new QGridLayout;
   int s_row = 0;

   // Row 1
   pb_load = us_pushbutton( tr( "Load Data" ) );
   //connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   specs->addWidget( pb_load, s_row, 0, 1, 2 );

   QPushButton* pb_details = us_pushbutton( tr( "Run Details" ), false );
   specs->addWidget( pb_details, s_row++, 2, 1, 2 );
   //le_file = us_lineedit( "", 1 );
   //le_file->setReadOnly( true );
   //specs->addWidget( le_file, s_row++, 2, 1, 2 );

   // Row 1a
   lb_id = us_label( "Run Info:", -1 );
   //lb_id->setAlignment( Qt::AlignCenter );
   specs->addWidget( lb_id, s_row, 0 );

   le_id = us_lineedit( "", 1 );
   //le_id->setReadOnly( true );
   specs->addWidget( le_id, s_row++, 1, 1, 3 );

   // Row 2

   //////
   QLabel* lb_cell = us_label( tr( "Cell:" ), -1 );
   //lb_cell->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lb_cell, s_row, 0 );

   //QwtCounter* ct_cell = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   //specs->addWidget( ct_cell, s_row, 1 );
   QComboBox* cb_cell = us_comboBox();
   specs->addWidget( cb_cell, s_row, 1 );

   // Row 3
   QLabel* lb_channel = us_label( tr( "Channel:" ), -1 );
   //lb_channel->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lb_channel, s_row, 2 );

   //QwtCounter* ct_channel = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   //specs->addWidget( ct_channel, s_row++, 3 );
   QComboBox* cb_channel = us_comboBox();
   specs->addWidget( cb_channel, s_row++, 3 );
   
   QLabel* lb_wavelength = us_label( tr( "Wavelength:" ), -1 );
   //lb_channel->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lb_wavelength, s_row, 0 );

   //QwtCounter* ct_channel = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   //specs->addWidget( ct_channel, s_row++, 3 );
   QComboBox* cb_wavelength = us_comboBox();
   specs->addWidget( cb_wavelength, s_row++, 1 );
   
/*
   lb_active = us_label( "Active Data", -1 );
   lb_active->setAlignment( Qt::AlignCenter );
   specs->addWidget( lb_active, s_row, 2 );

   le_active = us_lineedit( "", 1 );
   le_active->setReadOnly( true );
   specs->addWidget( le_active, s_row++, 3 );

   lb_centerpiece = us_label( tr( "Specify Centerpiece" ), -1 );
   lb_centerpiece->setAlignment( Qt::AlignCenter );
   specs->addWidget( lb_centerpiece, s_row, 0 );

   cb_centerpiece = us_comboBox();
   //cb_centerpiece->setEnabled( false );
   cb_centerpiece->setFont( QFont( US_GuiSettings::fontFamily(), 
                                   US_GuiSettings::fontSize() + 1 ) );
   specs->addWidget( cb_centerpiece, s_row++, 1 );

   if ( ! US_Hardware::readCenterpieceInfo( cp_list ) )
   {
      QMessageBox::critical( this, 
            tr( "UltraScan Failure" ),
            tr( "Could not read centerpiece file" ) );
      exit( 1 );
   }
         
   cb_centerpiece->addItem( tr( "Not specified" ) );

   for ( uint i = 0; i < cp_list.size(); i++ )
   {
      QString display = cp_list[ i ].material + ", " 
         + QString::number( cp_list[ i ].channels * 2 ) + tr( " channels, " )
         + QString::number( cp_list[ i ].pathlength, 'f', 2 ) + " cm ";

      switch (cp_list[i].sector)
      {
         case 0:
            display += tr( "(standard)" );
            break;
         case 1:
            display += tr( "(rectangular)" );
            break;
         case 2:
            display += tr( "(circular)" );
            break;
         case 3:
            display += tr( "(synthetic)" );
            break;
         case 4:
            display += tr( "(band-forming)" );
            break;
         default:
            display += tr( "(undefined sector shape)" );
            break;
      }
      
      cb_centerpiece->addItem( display );
   }

   lb_rotor = us_label( tr( "Specify Rotor" ), -1 );
   lb_rotor->setAlignment( Qt::AlignCenter );
   specs->addWidget( lb_rotor, s_row, 0 );

   cb_rotor = us_comboBox();
   //cb_rotor->setEnabled( false );
   specs->addWidget( cb_rotor, s_row++, 1 );

   if ( ! US_Hardware::readRotorInfo( rotor_list ) )
   {
      QMessageBox::critical( this, 
            tr( "UltraScan Failure" ),
            tr( "Could not read rotor file" ) );
      exit( 1 );
   }
         
   cb_rotor->addItem( tr( "Not specified" ) );
   cb_rotor->setFont( QFont( US_GuiSettings::fontFamily(), 
                             US_GuiSettings::fontSize() + 1 ) );

   for ( uint i = 0; i < rotor_list.size(); i++ )
      cb_rotor->addItem( rotor_list[ i ].type );
*/
   
   // Row 4
   QLabel* lb_scan = us_banner( tr( "Scan Controls" ) );
   specs->addWidget( lb_scan, s_row++, 0, 1, 4 );
   
   // Row 5

   // Scans
   lb_from = us_label( tr( "Scan Focus from:" ), -1 );
   lb_from->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lb_from, s_row, 0 );

   ct_from = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   specs->addWidget( ct_from, s_row, 1 );

   lb_to = us_label( tr( "to:" ), -1 );
   lb_to->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lb_to, s_row, 2 );

   ct_to = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   specs->addWidget( ct_to, s_row++, 3 );
   
   // Row 6
   // Exclude pushbuttons
   pb_exclude = us_pushbutton( tr( "Exclude Single Scan" ), false );
   specs->addWidget( pb_exclude, s_row, 0, 1, 2 );

   pb_excludeRange = us_pushbutton( tr( "Exclude Scan Range" ), false );
   specs->addWidget( pb_excludeRange, s_row++, 2, 1, 2 );
   
   // Row 7

   pb_exclusion = us_pushbutton( tr( "Exclusion Profile" ), false );
   specs->addWidget( pb_exclusion, s_row, 0, 1, 2 );

   // Edit pushbuttons
   pb_edit1 = us_pushbutton( tr( "Edit Single Scan" ), false );
   specs->addWidget( pb_edit1, s_row++, 2, 1, 2 );

   // Row 8
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

   //edits = new QGridLayout;
   //int e_row = 0;


   //pb_editRange = us_pushbutton( tr( "Edit Scan Range" ), false );
   //edits->addWidget( pb_editRange, e_row++, 2, 1, 2 );
/*
   lb_exclude = us_label( tr( "Exclude from:" ), -1 );
   lb_exclude->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   edits->addWidget( lb_exclude, e_row, 0 );

   ct_exclude = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   edits->addWidget( ct_exclude, e_row, 1 );

   lb_to2 = us_label( tr( "to:" ), -1 );
   lb_to2->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   edits->addWidget( lb_to2, e_row, 2 );

   ct_exclude2 = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   edits->addWidget( ct_exclude2, e_row++, 3 );
*/

   // Noise
   lb_noise = us_label( tr( "Subtract RI Noise:" ), -1 );
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
  
   QPushButton* pb_write = us_pushbutton( tr( "Save Current Edit Profile" ), false );
   specs->addWidget( pb_write, s_row++, 2, 1, 2 );

/*


   row += s_row;
////////////////
   QGridLayout* edit = new QGridLayout;
   int e_row = 0;

   // Instructions (3 rows)
   QLabel* lb_step = us_label( tr( "Step-by-Step<br>Instructions:" ), -1 );
   lb_step->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   edit->addWidget( lb_step, e_row, 0, 3, 1 );

   QTextEdit* te_instructions = us_textedit();
   edit->addWidget( te_instructions, e_row, 1, 3, 3 );

   e_row += 3;

   specs->addLayout( edit, 1, 4, 6, 4 );
   */
///////////////




/*
   te_data = new US_Editor( US_Editor::LOAD, false );
   
   QFontMetrics fm( QFont( US_GuiSettings::fontFamily(), 
                           US_GuiSettings::fontSize()   ) );
   
   te_data->setMinimumHeight( fm.height() * 20 );
   te_data->setFixedWidth ( fm.width( '0' ) * 20 );

   main->addWidget( te_data, row, 0, 20, 1 );

   QBoxLayout* plot = new US_Plot( meniscus_plot, 
         tr( "Meniscus Fit" ),
         tr( "Radius" ), tr( "2DSA Meniscus RMSD Value" ) );
   
   us_grid( meniscus_plot );
   
   meniscus_plot->setMinimumSize( 400, 400 );
   meniscus_plot->setAxisScale( QwtPlot::xBottom, 5.7, 6.8 );

   main->addLayout( plot, row, 1, 20, 1 );
   row += 20;

   QBoxLayout* misc = new QHBoxLayout;

   QLabel* lb_order = us_label( tr( "Fit Order:" ) );
   misc->addWidget( lb_order );

   sb_order = new QSpinBox();
   sb_order->setRange( 2, 9 );
   sb_order->setValue( 2 );
   sb_order->setPalette( US_GuiSettings::editColor() );
   misc->addWidget( sb_order );

   QLabel* lb_fit = us_label( tr( "Meniscus at minimum:" ) );
   misc->addWidget( lb_fit );

   le_fit = us_lineedit( "" );
   le_fit->setReadOnly( true );
   misc->addWidget( le_fit );

   QLabel* lb_rms_error = us_label( tr( "RMS Error:" ) );
   misc->addWidget( lb_rms_error );
   
   le_rms_error = us_lineedit( "" );
   le_rms_error->setReadOnly( true );
   misc->addWidget( le_rms_error );



   main->addLayout( misc, row++, 0, 1, 2 );
*/
   // Button rows

   QBoxLayout* buttons = new QHBoxLayout;

/*
*/




   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_accept );

   /////////////////
   QBoxLayout* plot = new US_Plot( data_plot, 
         tr( "Absorbance Data" ),
         tr( "Radius (in cm)" ), tr( "Absorbance" ) );
   
   //us_grid( data_plot );
   
   data_plot->setMinimumSize( 600, 400 );
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );

   left->addLayout( specs );
   //left->addLayout( edits );

   left->addStretch();
/*
   pb_edit = us_pushbutton( tr( "Edit Data" ), false );
   left->addWidget( pb_edit );
   connect( pb_edit, SIGNAL( clicked() ), SLOT( menu() ) );
   // Temporary enable for demo
   pb_edit->setEnabled( true );
*/

   left->addLayout( buttons );

   main->addLayout( left );
   main->addLayout( plot );
 
   edit_menu = false;
   //menu();
}

void US_Edvabs::reset( void )
{
   qDebug() << "reset";
   lb_from->setVisible( false );
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
void US_Edvabs::menu( void )
{  
   if ( edit_menu )
   {
      lb_from        ->setHidden( false );
      ct_from        ->setHidden( false );
      lb_to2         ->setHidden( false );
      lb_to          ->setHidden( false );
      ct_to          ->setHidden( false );
      lb_exclude     ->setHidden( false );
      ct_exclude     ->setHidden( false );
      ct_exclude2    ->setHidden( false );
      pb_edit1       ->setHidden( false );
      pb_editRange   ->setHidden( false );
      pb_exclude     ->setHidden( false );
      pb_excludeRange->setHidden( false );
      lb_noise       ->setHidden( false );
      ct_noise       ->setHidden( false );
      pb_noise       ->setHidden( false );
      pb_exclusion   ->setHidden( false );
      pb_subtract    ->setHidden( false );
      pb_spikes      ->setHidden( false );
      pb_invert      ->setHidden( false );

      pb_load        ->setHidden( true );
      le_file        ->setHidden( true );
      lb_id          ->setHidden( true );
      le_id          ->setHidden( true );
      lb_active      ->setHidden( true );
      le_active      ->setHidden( true );
      lb_centerpiece ->setHidden( true );
      cb_centerpiece ->setHidden( true );
      lb_rotor       ->setHidden( true );
      cb_rotor       ->setHidden( true );
      pb_meniscus    ->setHidden( true );
      le_meniscus    ->setHidden( true );
      pb_dataRange   ->setHidden( true );
      le_dataRange   ->setHidden( true );
      pb_plateau     ->setHidden( true );
      le_plateau     ->setHidden( true );
      pb_baseline    ->setHidden( true );
      le_baseline    ->setHidden( true );
                    
      pb_edit->setText( tr( "Edit Specifications Menu" ) );
   }
   else
   {
      pb_load        ->setHidden( false );
      le_file        ->setHidden( false );
      lb_id          ->setHidden( false );
      le_id          ->setHidden( false );
      lb_active      ->setHidden( false );
      le_active      ->setHidden( false );
      lb_centerpiece ->setHidden( false );
      cb_centerpiece ->setHidden( false );
      lb_rotor       ->setHidden( false );
      cb_rotor       ->setHidden( false );
      pb_meniscus    ->setHidden( false );
      le_meniscus    ->setHidden( false );
      pb_dataRange   ->setHidden( false );
      le_dataRange   ->setHidden( false );
      pb_plateau     ->setHidden( false );
      le_plateau     ->setHidden( false );
      pb_baseline    ->setHidden( false );
      le_baseline    ->setHidden( false );
                     
      lb_from        ->setHidden( true );
      ct_from        ->setHidden( true );
      lb_to          ->setHidden( true );
      lb_to2         ->setHidden( true );
      ct_to          ->setHidden( true );
      lb_exclude     ->setHidden( true );
      ct_exclude     ->setHidden( true );
      ct_exclude2    ->setHidden( true );
      pb_edit1       ->setHidden( true );
      pb_editRange   ->setHidden( true );
      pb_exclude     ->setHidden( true );
      pb_excludeRange->setHidden( true );
      lb_noise       ->setHidden( true );
      ct_noise       ->setHidden( true );
      pb_noise       ->setHidden( true );
      pb_exclusion   ->setHidden( true );
      pb_subtract    ->setHidden( true );
      pb_spikes      ->setHidden( true );
      pb_invert      ->setHidden( true );
      
      pb_edit->setText( tr( "Edit Data Menu" ) );
   }

   edit_menu = ! edit_menu;
}
