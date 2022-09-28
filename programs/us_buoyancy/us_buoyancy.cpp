//! \file us_buoyancy.cpp

#include <QApplication>
#include <QDomDocument>

#include "us_buoyancy.h"

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_run_details2.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_load_auc.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_constants.h"
#include "us_simparms.h"
#include "us_constants.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#define setSymbol(a)       setSymbol(*a)
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif


// Alternative Constructor (for autoflow )
US_Buoyancy::US_Buoyancy( QString auto_mode ) : US_Widgets()
{

   us_buoyancy_auto_mode = true;
   // initialize gradient forming material to 65% Nycodenz (weight/vol):

   bottom = 0.0;
   bottom_calc = 0.0;

   total_speeds = 0;
   v_line       = NULL;
   dbg_level    = US_Settings::us_debug();
   current_scan = 1;
   current_rpm  = 0.0;
   tmp_dpoint.name = "";
   tmp_dpoint.description = "";
   tmp_dpoint.dataset = "";
   tmp_dpoint.peakPosition = 0.0;
   tmp_dpoint.peakDensity = 0.0;
   tmp_dpoint.peakVbar = 0.0;
   tmp_dpoint.temperature = 0.0;
   tmp_dpoint.bufferDensity = 0.998234;
   tmp_dpoint.meniscus = 0.0;
   tmp_dpoint.bottom = 0.0;
   tmp_dpoint.speed = 0.0;
   tmp_dpoint.gradientMW = 821.0;
   tmp_dpoint.gradientVbar = 0.4831;
   tmp_dpoint.gradientC0 = 1.2294;

   setWindowTitle( tr( "[AUTO] Buoyancy Equilibrium Data Analysis" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* top = new QVBoxLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );

   // Put the Run Info across the entire window
   QHBoxLayout* runInfo = new QHBoxLayout();
   QLabel* lb_info = us_label( tr( "Dataset Info:" ), -1 );
   runInfo->addWidget( lb_info );

   le_info = us_lineedit( "", 1, true );
   runInfo->addWidget( le_info );

   top->addLayout( runInfo );

   QHBoxLayout* main = new QHBoxLayout();
   QVBoxLayout* left = new QVBoxLayout;

   // Start of Grid Layout
   QGridLayout* specs = new QGridLayout;
   int s_row = 0;

   // Row 1
   // Investigator

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   specs->addWidget( pb_investigator, s_row, 0 );

   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   int id = US_Settings::us_inv_ID();
   QString number  = ( id > 0 ) ?
      QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(), 1, true );
   specs->addWidget( le_investigator, s_row++, 1, 1, 3 );

   // Row 1A
   disk_controls = new US_Disk_DB_Controls;
   specs->addLayout( disk_controls, s_row++, 0, 1, 4 );

   // Row 2
   QPushButton* pb_load = us_pushbutton( tr( "Load Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   specs->addWidget( pb_load, s_row, 0, 1, 2 );

   pb_details = us_pushbutton( tr( "Run Details" ), false );
   connect( pb_details, SIGNAL( clicked() ), SLOT( details() ) );
   specs->addWidget( pb_details, s_row++, 2, 1, 2 );

   // Row 3
   QLabel* lb_triple = us_label( tr( "Cell / Channel / Wavelength:" ), -1 );
   specs->addWidget( lb_triple, s_row, 0, 1, 2 );

   cb_triple = us_comboBox();
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ),
                       SLOT  ( new_triple         ( int ) ) );
   specs->addWidget( cb_triple, s_row++, 2, 1, 2 );

   lbl_rpms   = us_label( tr( "Speed Step (RPM) of triple:" ), -1 );
   cb_rpms   = us_comboBox();
   specs->addWidget( lbl_rpms,   s_row,   0, 1, 2 );
   specs->addWidget( cb_rpms,   s_row++, 2, 1, 2 );

   // Scans
   QLabel* lbl_scan = us_label( tr( "Scan Focus:" ), -1 );
   lbl_scan->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lbl_scan, s_row, 0, 1, 2 );

   ct_selectScan = us_counter ( 3, 0.0, 0.0 ); // Update range upon load
   ct_selectScan->setSingleStep( 1 );
   ct_selectScan->setValue   ( current_scan );
   specs->addWidget( ct_selectScan, s_row++, 2, 1, 2 );
   connect( ct_selectScan, SIGNAL( valueChanged( double ) ),
            SLOT  ( plot_scan( double ) ) );

   //Exp. Params banner
   QLabel* lb_parms_bn = us_banner( tr( "Experiment Parameters" ) );
   specs->addWidget( lb_parms_bn,  s_row++, 0, 1, 4 );
   
   QButtonGroup* bg_points = new QButtonGroup( this );

   QGridLayout* box1 = us_radiobutton( tr( "Select meniscus" ), rb_meniscus );
   QGridLayout* box2 = us_radiobutton( tr( "Select peak position" ), rb_datapoint );

   rb_meniscus->setChecked( true );
   rb_datapoint->setChecked( false );
   rb_meniscus->setEnabled( false );
   rb_datapoint->setEnabled( false );
   bg_points->addButton( rb_meniscus );
   bg_points->addButton( rb_datapoint );
   // specs->addLayout( box1, s_row, 0, 1, 2 );
   // specs->addLayout( box2, s_row++, 2, 1, 2 );

   QLabel* lbl_meniscus = us_label( tr( "Meniscus Position (cm):" ), -1 );
   specs->addWidget( lbl_meniscus, s_row, 0, 1, 2 );
   le_meniscus = us_lineedit(  "0.0"  );
   specs->addWidget( le_meniscus, s_row++, 2, 1, 2 );
   connect (le_meniscus, SIGNAL( editingFinished (void)), this,
            SLOT (update_meniscus(void)));

   QLabel* lbl_stretch = us_label( tr( "Rotor Stretch (cm):" ), -1 );
   specs->addWidget( lbl_stretch, s_row, 0, 1, 2 );
   le_stretch = us_lineedit( "0.0" );
   specs->addWidget( le_stretch, s_row++, 2, 1, 2 );

   QLabel* lbl_bottom = us_label( tr( "Centerpiece Bottom (cm):" ), -1 );
   specs->addWidget( lbl_bottom, s_row, 0, 1, 2 );
   le_bottom = us_lineedit( QString::number(bottom) );
   specs->addWidget( le_bottom, s_row++, 2, 1, 2 );
   connect (le_bottom, SIGNAL( editingFinished (void)), this,
            SLOT (update_bottom(void)));

   QLabel* lbl_bottom_calc = us_label( tr( "Speed-corrected Bottom (cm):" ), -1 );
   specs->addWidget( lbl_bottom_calc, s_row, 0, 1, 2 );
   le_bottom_calc = us_lineedit( QString::number( bottom_calc ) );
   specs->addWidget( le_bottom_calc, s_row++, 2, 1, 2 );
   connect (le_bottom_calc, SIGNAL( editingFinished (void)), this,
            SLOT (update_bottom_calc(void)));

    // Solution info:
   QLabel* lbl_dens_0 = us_label( tr( "Loading Density (g/ml):" ), -1 );
   specs->addWidget( lbl_dens_0, s_row, 0, 1, 2 );
   le_dens_0 = us_lineedit( QString::number(tmp_dpoint.gradientC0) );
   specs->addWidget( le_dens_0, s_row++, 2, 1, 2 );
   connect (le_dens_0, SIGNAL( editingFinished (void)), this,
            SLOT (update_dens_0(void)));

   QLabel* lbl_buffer_density = us_label( tr( "Buffer Density (g/ml):" ), -1 );
   specs->addWidget( lbl_buffer_density, s_row, 0, 1, 2 );
   le_buffer_density = us_lineedit( QString::number(tmp_dpoint.bufferDensity) );
   specs->addWidget( le_buffer_density, s_row++, 2, 1, 2 );
   connect (le_buffer_density, SIGNAL( editingFinished (void)), this,
            SLOT ( update_bufferDensity( void ) ));

   QLabel* lbl_vbar = us_label( tr( "Gradient Mat. vbar (ml/g):" ), -1 );
   specs->addWidget( lbl_vbar, s_row, 0, 1, 2 );
   le_vbar = us_lineedit( QString::number( tmp_dpoint.gradientVbar ) );
   specs->addWidget( le_vbar, s_row++, 2, 1, 2 );
   connect (le_vbar, SIGNAL( editingFinished (void)), this,
            SLOT (update_vbar(void)));

   QLabel* lbl_MW = us_label( tr( "Gradient Mat. MW (g/mol):" ), -1 );
   specs->addWidget( lbl_MW, s_row, 0, 1, 2 );
   le_MW = us_lineedit( QString::number( tmp_dpoint.gradientMW ) );
   specs->addWidget( le_MW, s_row++, 2, 1, 2 );
   connect (le_MW, SIGNAL( editingFinished (void)), this,
            SLOT (update_MW(void)));

   QLabel* lbl_temperature = us_label( tr( "Temperature (°C):" ), -1 );
   specs->addWidget( lbl_temperature, s_row, 0, 1, 2 );
   le_temperature = us_lineedit( QString::number( tmp_dpoint.temperature ) );
   specs->addWidget( le_temperature, s_row++, 2, 1, 2 );


   //Peak. Params banner
   QLabel* lb_peak_parms_bn = us_banner( tr( "Peak Parameters" ) );
   specs->addWidget( lb_peak_parms_bn,  s_row++, 0, 1, 4 );
   
   //Information on peaks: Listbox of to-be-determined (from automated fit) size
   

   
   QLabel* lbl_peakName = us_label( tr( "Peak name/label:" ), -1 );
   specs->addWidget( lbl_peakName, s_row, 0, 1, 2 );

   cb_peaks = us_comboBox();
   // connect( cb_peaks, SIGNAL( currentIndexChanged( int ) ),
   // 	              SLOT  ( new_peak           ( int ) ) );
   specs->addWidget( cb_peaks, s_row++, 2, 1, 2 );

   le_peakName = us_lineedit( tmp_dpoint.name );
   specs->addWidget( le_peakName, s_row++, 2, 1, 2 );
   connect (le_peakName, SIGNAL( editingFinished (void)), this,
            SLOT (update_peakName(void)));
   

   QLabel* lbl_peakPosition = us_label( tr( "Peak Position (cm):" ), -1 );
   specs->addWidget( lbl_peakPosition, s_row, 0, 1, 2 );
   le_peakPosition = us_lineedit( QString::number( tmp_dpoint.peakPosition ) );
   specs->addWidget( le_peakPosition, s_row++, 2, 1, 2 );

   QLabel* lbl_peakDensity = us_label( tr( "Peak Density (g/ml):" ), -1 );
   specs->addWidget( lbl_peakDensity, s_row, 0, 1, 2 );
   le_peakDensity = us_lineedit( QString::number( tmp_dpoint.peakDensity ) );
   specs->addWidget( le_peakDensity, s_row++, 2, 1, 2 );

   QLabel* lbl_peakVbar = us_label( tr( "Peak vbar (ml/g):" ), -1 );
   specs->addWidget( lbl_peakVbar, s_row, 0, 1, 2 );
   le_peakVbar = us_lineedit( QString::number( tmp_dpoint.peakVbar ) );
   specs->addWidget( le_peakVbar, s_row++, 2, 1, 2 );

   // Button rows
   QBoxLayout* buttons = new QHBoxLayout;

   pb_save = us_pushbutton( tr( "Save Datapoint" ), false );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
   //specs->addWidget( pb_save, s_row, 0, 1, 2 );

   pb_write = us_pushbutton( tr( "Write Report" ), false );
   connect( pb_write, SIGNAL( clicked() ), SLOT( write() ) );
   //specs->addWidget( pb_write, s_row++, 2, 1, 2 );

   //view all auto-generated peak reports
   pb_view_reports = us_pushbutton( tr( "View Reports" ), false );
   connect( pb_view_reports, SIGNAL( clicked() ), SLOT( write_auto() ) );
   specs->addWidget( pb_view_reports, s_row++, 0, 1, 4 );
   
   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_accept );


   //In case we need to hide something as opposed to default GUI
   if ( auto_mode.toStdString() == "AUTO")
     {
       rb_meniscus   ->hide();
       rb_datapoint  ->hide();

       le_peakName   ->hide();

       pb_save       ->hide();
       pb_write      ->hide();
     }
   

   
   // Plot layout on right side of window
   plot = new US_Plot( data_plot,
         tr( "Absorbance Data" ),
         tr( "Radius (in cm)" ), tr( "Absorbance" ) );

   data_plot->setMinimumSize( 600, 400 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );

   pick = new US_PlotPicker( data_plot );
   // Set rubber band to display for Control+Left Mouse Button
   pick->setRubberBand  ( QwtPicker::VLineRubberBand );
   pick->setMousePattern( QwtEventPattern::MouseSelect1,
                          Qt::LeftButton, Qt::ControlModifier );

   left->addLayout( specs );
   left->addStretch();
   left->addLayout( buttons );

   main->addLayout( left );
   main->addLayout( plot );
   main->setStretchFactor( plot, 3 );
   top ->addLayout( main );

   reset();
}

// Default Constructor
US_Buoyancy::US_Buoyancy() : US_Widgets()
{

   us_buoyancy_auto_mode = false;
   // initialize gradient forming material to 65% Nycodenz (weight/vol):

   bottom = 0.0;
   bottom_calc = 0.0;

   total_speeds = 0;
   v_line       = NULL;
   dbg_level    = US_Settings::us_debug();
   current_scan = 1;
   current_rpm  = 0.0;
   tmp_dpoint.name = "";
   tmp_dpoint.description = "";
   tmp_dpoint.dataset = "";
   tmp_dpoint.peakPosition = 0.0;
   tmp_dpoint.peakDensity = 0.0;
   tmp_dpoint.peakVbar = 0.0;
   tmp_dpoint.temperature = 0.0;
   tmp_dpoint.bufferDensity = 0.998234;
   tmp_dpoint.meniscus = 0.0;
   tmp_dpoint.bottom = 0.0;
   tmp_dpoint.speed = 0.0;
   tmp_dpoint.gradientMW = 821.0;
   tmp_dpoint.gradientVbar = 0.4831;
   tmp_dpoint.gradientC0 = 1.2294;

   setWindowTitle( tr( "Buoyancy Equilibrium Data Analysis" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* top = new QVBoxLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );

   // Put the Run Info across the entire window
   QHBoxLayout* runInfo = new QHBoxLayout();
   QLabel* lb_info = us_label( tr( "Dataset Info:" ), -1 );
   runInfo->addWidget( lb_info );

   le_info = us_lineedit( "", 1, true );
   runInfo->addWidget( le_info );

   top->addLayout( runInfo );

   QHBoxLayout* main = new QHBoxLayout();
   QVBoxLayout* left = new QVBoxLayout;

   // Start of Grid Layout
   QGridLayout* specs = new QGridLayout;
   int s_row = 0;

   // Row 1
   // Investigator

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   specs->addWidget( pb_investigator, s_row, 0 );

   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   int id = US_Settings::us_inv_ID();
   QString number  = ( id > 0 ) ?
      QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(), 1, true );
   specs->addWidget( le_investigator, s_row++, 1, 1, 3 );

   // Row 1A
   disk_controls = new US_Disk_DB_Controls;
   specs->addLayout( disk_controls, s_row++, 0, 1, 4 );

   // Row 2
   QPushButton* pb_load = us_pushbutton( tr( "Load Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   specs->addWidget( pb_load, s_row, 0, 1, 2 );

   pb_details = us_pushbutton( tr( "Run Details" ), false );
   connect( pb_details, SIGNAL( clicked() ), SLOT( details() ) );
   specs->addWidget( pb_details, s_row++, 2, 1, 2 );

   // Row 3
   QLabel* lb_triple = us_label( tr( "Cell / Channel / Wavelength:" ), -1 );
   specs->addWidget( lb_triple, s_row, 0, 1, 2 );

   cb_triple = us_comboBox();
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ),
                       SLOT  ( new_triple         ( int ) ) );
   specs->addWidget( cb_triple, s_row++, 2, 1, 2 );

   lbl_rpms   = us_label( tr( "Speed Step (RPM) of triple:" ), -1 );
   cb_rpms   = us_comboBox();
   specs->addWidget( lbl_rpms,   s_row,   0, 1, 2 );
   specs->addWidget( cb_rpms,   s_row++, 2, 1, 2 );

   // Scans
   QLabel* lbl_scan = us_label( tr( "Scan Focus:" ), -1 );
   lbl_scan->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lbl_scan, s_row, 0, 1, 2 );

   ct_selectScan = us_counter ( 3, 0.0, 0.0 ); // Update range upon load
   ct_selectScan->setSingleStep( 1 );
   ct_selectScan->setValue   ( current_scan );
   specs->addWidget( ct_selectScan, s_row++, 2, 1, 2 );
   connect( ct_selectScan, SIGNAL( valueChanged( double ) ),
            SLOT  ( plot_scan( double ) ) );

   QButtonGroup* bg_points = new QButtonGroup( this );

   QGridLayout* box1 = us_radiobutton( tr( "Select meniscus" ), rb_meniscus );
   QGridLayout* box2 = us_radiobutton( tr( "Select peak position" ), rb_datapoint );

   rb_meniscus->setChecked( true );
   rb_datapoint->setChecked( false );
   rb_meniscus->setEnabled( false );
   rb_datapoint->setEnabled( false );
   bg_points->addButton( rb_meniscus );
   bg_points->addButton( rb_datapoint );
   specs->addLayout( box1, s_row, 0, 1, 2 );
   specs->addLayout( box2, s_row++, 2, 1, 2 );

   QLabel* lbl_meniscus = us_label( tr( "Meniscus Position (cm):" ), -1 );
   specs->addWidget( lbl_meniscus, s_row, 0, 1, 2 );
   le_meniscus = us_lineedit(  "0.0"  );
   specs->addWidget( le_meniscus, s_row++, 2, 1, 2 );
   connect (le_meniscus, SIGNAL( editingFinished (void)), this,
            SLOT (update_meniscus(void)));

   QLabel* lbl_stretch = us_label( tr( "Rotor Stretch (cm):" ), -1 );
   specs->addWidget( lbl_stretch, s_row, 0, 1, 2 );
   le_stretch = us_lineedit( "0.0" );
   specs->addWidget( le_stretch, s_row++, 2, 1, 2 );

   QLabel* lbl_bottom = us_label( tr( "Centerpiece Bottom (cm):" ), -1 );
   specs->addWidget( lbl_bottom, s_row, 0, 1, 2 );
   le_bottom = us_lineedit( QString::number(bottom) );
   specs->addWidget( le_bottom, s_row++, 2, 1, 2 );
   connect (le_bottom, SIGNAL( editingFinished (void)), this,
            SLOT (update_bottom(void)));

   QLabel* lbl_bottom_calc = us_label( tr( "Speed-corrected Bottom (cm):" ), -1 );
   specs->addWidget( lbl_bottom_calc, s_row, 0, 1, 2 );
   le_bottom_calc = us_lineedit( QString::number( bottom_calc ) );
   specs->addWidget( le_bottom_calc, s_row++, 2, 1, 2 );
   connect (le_bottom_calc, SIGNAL( editingFinished (void)), this,
            SLOT (update_bottom_calc(void)));

    // Solution info:
   QLabel* lbl_dens_0 = us_label( tr( "Loading Density (g/ml):" ), -1 );
   specs->addWidget( lbl_dens_0, s_row, 0, 1, 2 );
   le_dens_0 = us_lineedit( QString::number(tmp_dpoint.gradientC0) );
   specs->addWidget( le_dens_0, s_row++, 2, 1, 2 );
   connect (le_dens_0, SIGNAL( editingFinished (void)), this,
            SLOT (update_dens_0(void)));

   QLabel* lbl_buffer_density = us_label( tr( "Buffer Density (g/ml):" ), -1 );
   specs->addWidget( lbl_buffer_density, s_row, 0, 1, 2 );
   le_buffer_density = us_lineedit( QString::number(tmp_dpoint.bufferDensity) );
   specs->addWidget( le_buffer_density, s_row++, 2, 1, 2 );
   connect (le_buffer_density, SIGNAL( editingFinished (void)), this,
            SLOT ( update_bufferDensity( void ) ));

   QLabel* lbl_vbar = us_label( tr( "Gradient Mat. vbar (ml/g):" ), -1 );
   specs->addWidget( lbl_vbar, s_row, 0, 1, 2 );
   le_vbar = us_lineedit( QString::number( tmp_dpoint.gradientVbar ) );
   specs->addWidget( le_vbar, s_row++, 2, 1, 2 );
   connect (le_vbar, SIGNAL( editingFinished (void)), this,
            SLOT (update_vbar(void)));

   QLabel* lbl_MW = us_label( tr( "Gradient Mat. MW (g/mol):" ), -1 );
   specs->addWidget( lbl_MW, s_row, 0, 1, 2 );
   le_MW = us_lineedit( QString::number( tmp_dpoint.gradientMW ) );
   specs->addWidget( le_MW, s_row++, 2, 1, 2 );
   connect (le_MW, SIGNAL( editingFinished (void)), this,
            SLOT (update_MW(void)));

   QLabel* lbl_temperature = us_label( tr( "Temperature (°C):" ), -1 );
   specs->addWidget( lbl_temperature, s_row, 0, 1, 2 );
   le_temperature = us_lineedit( QString::number( tmp_dpoint.temperature ) );
   specs->addWidget( le_temperature, s_row++, 2, 1, 2 );

   QLabel* lbl_peakName = us_label( tr( "Peak name/label:" ), -1 );
   specs->addWidget( lbl_peakName, s_row, 0, 1, 2 );
   le_peakName = us_lineedit( tmp_dpoint.name );
   specs->addWidget( le_peakName, s_row++, 2, 1, 2 );
   connect (le_peakName, SIGNAL( editingFinished (void)), this,
            SLOT (update_peakName(void)));

   QLabel* lbl_peakPosition = us_label( tr( "Peak Position (cm):" ), -1 );
   specs->addWidget( lbl_peakPosition, s_row, 0, 1, 2 );
   le_peakPosition = us_lineedit( QString::number( tmp_dpoint.peakPosition ) );
   specs->addWidget( le_peakPosition, s_row++, 2, 1, 2 );

   QLabel* lbl_peakDensity = us_label( tr( "Peak Density (g/ml):" ), -1 );
   specs->addWidget( lbl_peakDensity, s_row, 0, 1, 2 );
   le_peakDensity = us_lineedit( QString::number( tmp_dpoint.peakDensity ) );
   specs->addWidget( le_peakDensity, s_row++, 2, 1, 2 );

   QLabel* lbl_peakVbar = us_label( tr( "Peak vbar (ml/g):" ), -1 );
   specs->addWidget( lbl_peakVbar, s_row, 0, 1, 2 );
   le_peakVbar = us_lineedit( QString::number( tmp_dpoint.peakVbar ) );
   specs->addWidget( le_peakVbar, s_row++, 2, 1, 2 );

   // Button rows
   QBoxLayout* buttons = new QHBoxLayout;

   pb_save = us_pushbutton( tr( "Save Datapoint" ), false );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
   specs->addWidget( pb_save, s_row, 0, 1, 2 );

   pb_write = us_pushbutton( tr( "Write Report" ), false );
   connect( pb_write, SIGNAL( clicked() ), SLOT( write() ) );
   specs->addWidget( pb_write, s_row++, 2, 1, 2 );

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
   plot = new US_Plot( data_plot,
         tr( "Absorbance Data" ),
         tr( "Radius (in cm)" ), tr( "Absorbance" ) );

   data_plot->setMinimumSize( 600, 400 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );

   pick = new US_PlotPicker( data_plot );
   // Set rubber band to display for Control+Left Mouse Button
   pick->setRubberBand  ( QwtPicker::VLineRubberBand );
   pick->setMousePattern( QwtEventPattern::MouseSelect1,
                          Qt::LeftButton, Qt::ControlModifier );

   left->addLayout( specs );
   left->addStretch();
   left->addLayout( buttons );

   main->addLayout( left );
   main->addLayout( plot );
   main->setStretchFactor( plot, 3 );
   top ->addLayout( main );

   reset();
}

// Select a new peak
void US_Buoyancy::new_peak( int index )
{
  QString triple_n = cb_triple->itemText( current_triple );
  QString peak_n   = cb_peaks ->itemText( index );

  QStringList curr_peak_parms = triple_name_to_peak_to_parms_map[ triple_n ][ peak_n ];
  
  le_peakDensity->setText( curr_peak_parms[0] );
  le_peakVbar->setText( curr_peak_parms[1] );
  le_peakPosition->setText( curr_peak_parms[2] );
}

// Select a new triple
void US_Buoyancy::new_triple( int index )
{
   current_triple = index;
   data = allData[ index ];
   le_info->setText( runID + ": " + allData[index].description );
   tmp_dpoint.description = allData[index].description;
   tmp_dpoint.dataset = runID;

   plot_scan( current_scan );
}

void US_Buoyancy::update_fields( void )
{
   QString str;

   current_stretch = calc_stretch();
   str.setNum( current_stretch );
   le_stretch->setText( str );

   if ( meniscus[current_triple] != 0 )
   {
      tmp_dpoint.meniscus = meniscus[current_triple] + current_stretch;
      str.setNum( tmp_dpoint.meniscus );
      le_meniscus->setText( str );
   }
   else
   {
      le_meniscus->setText( "0.0" );
      rb_meniscus ->setChecked( true  );
      rb_meniscus ->setEnabled( false );
      rb_datapoint->setEnabled( false );
   }
   str.setNum( simparams[current_triple].bottom_position );
   le_bottom->setText( str );
   tmp_dpoint.bottom = simparams[current_triple].bottom_position + current_stretch;
   str.setNum( tmp_dpoint.bottom );
   le_bottom_calc->setText( str );
   str.setNum( tmp_dpoint.temperature );
   le_temperature->setText( str );

   if ( !us_buoyancy_auto_mode )
     calc_points();
   else
     {
       QString triple_n = cb_triple->itemText( current_triple );
       calc_points_auto( triple_n );
     }
}

// Load an AUC data set
void US_Buoyancy::calc_points( void )
{
   
   QString str;
   if (tmp_dpoint.peakPosition != 0.0)
   {
      double omega_s, C, C0, r2, k1, k2, k3, k4;
      omega_s = pow( current_rpm * M_PI/30.0, 2.0 );
      C0 = tmp_dpoint.gradientC0 - tmp_dpoint.bufferDensity; //subtract buffer density from nycodenz density
      r2 = pow( tmp_dpoint.bottom, 2.0 ) - pow( tmp_dpoint.meniscus, 2.0);
      k1 = tmp_dpoint.gradientMW * omega_s/( 2.0 * R_GC * (tmp_dpoint.temperature + 273.15) );
      k4 = 1.0 - tmp_dpoint.gradientVbar * tmp_dpoint.bufferDensity;
      k2 = exp( k1 * ( k4 ) * (pow( tmp_dpoint.peakPosition, 2.0 ) - pow( tmp_dpoint.meniscus, 2.0 ) ) );
      k3 = exp( k1 * ( k4 ) * r2);
      C  = k1 * k4 * C0 *r2 * k2/( k3 - 1.0 ) + tmp_dpoint.bufferDensity;
      str.setNum( C );
      le_peakDensity->setText( str );
      str.setNum( 1.0/C );
      le_peakVbar->setText( str );
      str.setNum( tmp_dpoint.peakPosition );
      le_peakPosition->setText( str );
      pb_save->setEnabled( true );
   }
   else
   {
      le_peakDensity->setText( "0" );
      le_peakVbar->setText( "0" );
      le_peakPosition->setText( "0" );
      pb_save->setEnabled( false );
   }
}

// cacl. poitns based on provided vector of peak positions
void US_Buoyancy::calc_points_auto( QString triple_n )
{
  QVector< double > peak_poss = triple_name_to_peaks_map[ triple_n ];

  QStringList peak_names;
  //iterate over peaks for the current triple
  for ( int i=0; i< peak_poss.size(); i++ )
    {
      QString str;
      if ( peak_poss[ i ] != 0.0)
	{
	  QString peak_name = "Peak #" + QString::number( i+1 );
	  peak_names << peak_name;
	  
	  double omega_s, C, C0, r2, k1, k2, k3, k4;
	  omega_s = pow( current_rpm * M_PI/30.0, 2.0 );
	  C0 = tmp_dpoint.gradientC0 - tmp_dpoint.bufferDensity; //subtract buffer density from nycodenz density
	  r2 = pow( tmp_dpoint.bottom, 2.0 ) - pow( tmp_dpoint.meniscus, 2.0);
	  k1 = tmp_dpoint.gradientMW * omega_s/( 2.0 * R_GC * (tmp_dpoint.temperature + 273.15) );
	  k4 = 1.0 - tmp_dpoint.gradientVbar * tmp_dpoint.bufferDensity;
	  k2 = exp( k1 * ( k4 ) * (pow( peak_poss[ i ], 2.0 ) - pow( tmp_dpoint.meniscus, 2.0 ) ) );
	  k3 = exp( k1 * ( k4 ) * r2);
	  C  = k1 * k4 * C0 *r2 * k2/( k3 - 1.0 ) + tmp_dpoint.bufferDensity;

	  QStringList curr_peak_parms;
	  str.setNum( C );
	  curr_peak_parms << str;
	  //le_peakDensity->setText( str );
	  
	  str.setNum( 1.0/C );
	  curr_peak_parms << str;
	  //le_peakVbar->setText( str );
	  
	  str.setNum( peak_poss[ i ] );
	  curr_peak_parms << str;
	  //le_peakPosition->setText( str );

	  triple_name_to_peak_to_parms_map[ triple_n ][ peak_name ] = curr_peak_parms;
	  
	  pb_save->setEnabled( true );
	}
      else
	{
	  // le_peakDensity->setText( "0" );
	  // le_peakVbar->setText( "0" );
	  // le_peakPosition->setText( "0" );
	  // pb_save->setEnabled( false );
	  
	  qDebug() << "Peak position #" << i << " for triple " << triple_n << " is ZERO!";
	}
    }

  if ( !peak_names.isEmpty() )
    {
      cb_peaks->disconnect();
      cb_peaks->clear();
      
      cb_peaks->addItems( peak_names );
      connect( cb_peaks, SIGNAL( currentIndexChanged( int ) ),
	       SLOT  ( new_peak           ( int ) ) );
      
      //cb_peaks->setCurrentIndex(0);
      new_peak( 0 );
    }
}

// load AUC data
void US_Buoyancy::load( void )
{
   bool isLocal = ! disk_controls->db();
   reset();

   US_LoadAUC* dialog =
      new US_LoadAUC( isLocal, allData, triples, workingDir );

   connect( dialog, SIGNAL( changed       ( bool ) ),
            this,     SLOT( update_disk_db( bool ) ) );

   if ( dialog->exec() == QDialog::Rejected )  return;

   runID = workingDir.section( "/", -1, -1 );
   cb_triple->clear();
   delete dialog;
   if ( triples.size() == 0 )
   {
      QMessageBox::warning( this,
            tr( "No Files Found" ),
            tr( "There were no files of the form *.auc\n"
                "found in the specified directory." ) );
      return;
   }

   cb_triple->addItems( triples );
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ),
                       SLOT  ( new_triple         ( int ) ) );
   current_triple = 0;

   le_info->setText( runID + ": " + allData[0].description );
   tmp_dpoint.description = allData[0].description;
   tmp_dpoint.dataset = runID;


   data     = allData[ 0 ];
   dataType = QString( QChar( data.type[ 0 ] ) )
            + QString( QChar( data.type[ 1 ] ) );

   simparams.resize(triples.size());
   meniscus.resize(triples.size());

   for (int ii=0; ii<triples.size(); ii++)
   {
     //ALEXEY: if auto_mode, read meniscus positions from editProfiles
      if ( us_buoyancy_auto_mode )
	{
	  if ( ii == 0 )
	    meniscus[ ii ] = 6.04519984; //HARD coded for now
	  else
	    meniscus[ ii ] = 6.01; //HARD coded for now

	  meniscus_to_triple_name_map[ triples[ii] ] = meniscus[ ii ];
	  triple_report_saved_map    [ triples[ii] ] = false;
	  triple_fitted_map          [ triples[ii] ] = false;  
	}
      else
	meniscus[ii] = 0.0;
      
   }
   if (isLocal)
   {
      for ( int ii = 0; ii < triples.size(); ii++ )
      {  // Generate file names
         QString triple = QString( triples.at( ii ) ).replace( " / ", "." );
	 QString file   = runID + "." + dataType + "." + triple + ".auc";
         files << file;
         simparams[ii].initFromData( NULL, allData[ii], true, runID, dataType);
      }
   }
   else
   {
      US_Passwd pw;
      US_DB2  db( pw.getPasswd() );
      for ( int ii = 0; ii < triples.size(); ii++ )
      {  // Generate file names
         QString triple = QString( triples.at( ii ) ).replace( " / ", "." );
	 QString file   = runID + "." + dataType + "." + triple + ".auc";
         files << file;
         simparams[ii].initFromData( &db, allData[ii], true, runID, dataType);
      }
   }
   QString file = workingDir + "/" + runID + "." + dataType + ".xml";
   expType = "";
   QFile xf( file );

   if ( xf.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QXmlStreamReader xml( &xf );

      while( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "experiment" )
         {
            QXmlStreamAttributes xa = xml.attributes();
            expType   = xa.value( "type" ).toString();
            break;
         }
      }

      xf.close();
   }

   if ( expType.isEmpty()  &&  disk_controls->db() )
   {  // no experiment type yet and data read from DB:  try for DB exp type
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this, tr( "Connection Problem" ),
           tr( "Could not connect to database \n" ) + db.lastError() );
         return;
      }

      QStringList query;
      query << "get_experiment_info_by_runID" << runID
            << QString::number( US_Settings::us_inv_ID() );

      db.query( query );
      db.next();
      expType    = db.value( 8 ).toString();
   }
   else                      // insure Ulll... form, e.g., "Equilibrium"
   {
      expType    = expType.left( 1 ).toUpper() +
                   expType.mid(  1 ).toLower();
   }
   expIsBuoyancy = ( expType.compare( "Buoyancy", Qt::CaseInsensitive ) == 0 );
   if (expIsBuoyancy)
   {
      US_Passwd pw;
      US_DB2  db( pw.getPasswd() );
      update_speedData();
      pick     ->disconnect();
      connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
              SLOT  ( mouse   ( const QwtDoublePoint& ) ) );

      plot_scan( current_scan );
            
      connect( cb_rpms,   SIGNAL( currentIndexChanged( int ) ),
              SLOT  ( new_rpmval         ( int ) ) );
   }
   else
   {  // non-Equilibrium
           QMessageBox::warning( this, tr( "Wrong Type of Data" ),
         tr( "This analysis program requires data of type \"Buoyancy\".\n"
             "Please load a different dataset with the correct type.") );
         return;
   }
   // Enable pushbuttons
   pb_details   ->setEnabled( true );
   QString str1, str2;
   
   if ( dataType == "FI")
   {
      str1 = "Fluorescence Data";
      str2 = "Fluorescence Intensity";
   }
   else if ( dataType == "RI")
   {
      str1 = "Absorbance Data";
      str2 = "Absorbance";
   }
   else if ( dataType == "RA")
   {
      str1 = "Absorbance Data";
      str2 = "Absorbance";
   }
   else if ( dataType == "IP")
   {
      str1 = "Interference Data";
      str2 = "Fringes";
   }
   data_plot->setTitle( str1 );
   data_plot->setAxisTitle( QwtPlot::yLeft, str2 );

   // Temperature check
   double             dt = 0.0;
   US_DataIO::RawData triple;

   foreach( triple, allData )
   {
       double temp_spread = triple.temperature_spread();
       dt = ( temp_spread > dt ) ? temp_spread : dt;
   }

   if ( dt > US_Settings::tempTolerance() )
   {
      QMessageBox::warning( this,
            tr( "Temperature Problem" ),
            tr( "The temperature in this run varied over the course\n"
                "of the run to a larger extent than allowed by the\n"
                "current threshold (" )
                + QString::number( US_Settings::tempTolerance(), 'f', 1 )
                + " " + DEGC + tr( ". The accuracy of experimental\n"
                "results may be affected significantly." ) );
   }
   QString str;
   str.setNum(simparams[current_triple].bottom_position);
   le_bottom->setText( str );

   //if [auto] analysis, iterate over triple indecies
   if ( us_buoyancy_auto_mode )
     {
       for ( int i=0; i< cb_triple->count(); i++ )
	 {
	   //new_triple( i );
	   cb_triple->setCurrentIndex( i );
	 }

       //set back to the first triple
       cb_triple->setCurrentIndex( 0 );
     }
   

}

// Handle a mouse click according to the current pick step
void US_Buoyancy::mouse( const QwtDoublePoint& p )
{
   double maximum = -1.0e99;
   if ( rb_meniscus->isChecked() )
   {
      QString str;
      str.setNum( p.x() );
      le_meniscus->setText( str );
      // the internally stored meniscus position is adjusted to the theoretical rest position
      meniscus[current_triple]  = p.x() - calc_stretch();
      tmp_dpoint.meniscus = p.x();
      rb_meniscus->setEnabled( true );
      rb_datapoint->setEnabled( true );
   }
   else
   {
      tmp_dpoint.peakPosition = p.x();
      pb_write->setEnabled( true );
      calc_points();
   }
   // Un-zoom
   if ( plot->btnZoom->isChecked() )
   {
      plot->btnZoom->setChecked( false );
   }
   draw_vline( p.x() );
   data_plot->replot();

   // Remove the left line
   if ( v_line != NULL )
   {
       v_line->detach();
       delete v_line;
       v_line = NULL;
    }

    marker = new QwtPlotMarker;
    QBrush brush( Qt::white );
    QPen   pen  ( brush, 2.0 );

    marker->setValue( p.x(), maximum );
    QwtSymbol* sym = new QwtSymbol( QwtSymbol::Cross,
                        brush, pen, QSize ( 8, 8 ) );
    marker->setSymbol( sym );

    marker->attach( data_plot );
    data_plot->replot();
    marker->detach();
    delete marker;
    marker = NULL;
}

// Display run details
void US_Buoyancy::details( void )
{
   US_RunDetails2* dialog
      = new US_RunDetails2( allData, runID, workingDir, triples );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

// Select DB investigator
void US_Buoyancy::sel_investigator( void )
{
   int investigator = US_Settings::us_inv_ID();

   US_Investigator* dialog = new US_Investigator( true, investigator );
   dialog->exec();

   investigator = US_Settings::us_inv_ID();

   QString inv_text = QString::number( investigator ) + ": "
                      +  US_Settings::us_inv_name();

   le_investigator->setText( inv_text );
}

// Reset parameters to their defaults
void US_Buoyancy::reset( void )
{
   le_info     ->setText( "" );

   ct_selectScan->disconnect();
   ct_selectScan->setMinimum( 0 );
   ct_selectScan->setMaximum( 0 );
   ct_selectScan->setValue   ( 0 );
   connect( ct_selectScan, SIGNAL( valueChanged( double ) ),
            SLOT  ( plot_scan( double ) ) );

   cb_triple->disconnect();

   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   data_plot->detachItems( QwtPlotItem::Rtti_PlotMarker );
   v_line = NULL;
   pick->disconnect();

   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   grid = us_grid( data_plot );
   data_plot->replot();

   // Disable pushbuttons
   pb_details  ->setEnabled( false );
   pb_write    ->setEnabled( false );
   pb_save     ->setEnabled( false );
   rb_meniscus ->setEnabled( false );
   rb_datapoint->setEnabled( false );

   // Remove icons

   dpoint        .clear();
   allData        .clear();
   meniscus       .clear();
   data.scanData  .clear();
   trip_rpms      .clear();
   triples        .clear();
   cb_rpms       ->disconnect();
   cb_rpms       ->clear();

   triple_name_to_peaks_map . clear();
   triple_name_to_peak_to_parms_map . clear();
   triple_report_saved_map  . clear();
   triple_fitted_map        . clear();
   meniscus_to_triple_name_map. clear();

   if ( us_buoyancy_auto_mode ) 
     pb_view_reports  ->setEnabled( false );
}

// Select DB investigator// Private slot to update disk/db control with dialog changes it
void US_Buoyancy::update_disk_db( bool isDB )
{
   if ( isDB )
      disk_controls->set_db();
   else
      disk_controls->set_disk();
}

// Plot a single scan curve
void US_Buoyancy::plot_scan( double scan_number )
{
   float temp_x, temp_y;
   QString triple_n = cb_triple->itemText( current_triple );
   QString title_fit;
   
  // current scan is global
   current_scan = (int) scan_number;
   int    rsize = data.pointCount();
   int    ssize = data.scanCount();
   int    count = 0;
   QVector< double > rvec( rsize );
   QVector< double > vvec( rsize );
   double* r    = rvec.data();
   double* v    = vvec.data();

   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   v_line = NULL;

   double maxR  = -1.0e99;
   double minR  =  1.0e99;
   double maxV  = -1.0e99;
   double minV  =  1.0e99;
   int maxscan = 0;
   QString srpm = cb_rpms->currentText();
   current_rpm = srpm.toDouble();

   // Plot only the currently selected scan(s)
   //
   for ( int ii = 0; ii < ssize; ii++ )
   {
     // clear WaveLengthScan array every time --
     // ensure last scan is written in the end to v_wavelength
     v_wavelength. clear();
     
      US_DataIO::Scan* s = &data.scanData[ ii ];

      QString arpm        = QString::number( s->rpm );

      //how many scans are included in the current set of speeds? Increment maxscan...
      if ( arpm == srpm ) 
      {
         maxscan++;
      }
      else 
      {
         continue;
      }
      count = 0;

      WavelengthScan wls;
	
      for ( int jj = 0; jj < rsize; jj++ )
      {
         r[ count ] = data.xvalues[ jj ];
         v[ count ] = s->rvalues[ jj ];

         maxR = max( maxR, r[ count ] );
         minR = min( minR, r[ count ] );
         maxV = max( maxV, v[ count ] );
         minV = min( minV, v[ count ] );

         count++;

	 //here, populate QVector <WavelengthScan> v_wavelength;
	 temp_x = data.xvalues[ jj ];
	 temp_y = s->rvalues[ jj ];
	 Reading r = {temp_x, temp_y};

	 //if ( temp_x >=  meniscus_to_triple_name_map [ triple_n ] ) 
	 wls. v_readings.push_back(r);
      }
      
      //push a WavelengthScan to array
      wls. description = "Scan for triple " + triple_n;
      v_wavelength.push_back( wls );

      QString title = tr( "Raw Data at " )
         + QString::number( s->seconds ) + tr( " seconds" )
         + " #" + QString::number( ii );

      title_fit = title;

      QwtPlotCurve* c = us_curve( data_plot, title );
      c->setSamples( r, v, count );
      if (ii == current_scan - 1 )
      { // set the temperature to the currently highlighted scan:
         tmp_dpoint.temperature = s->temperature;
         c->setPen( QPen( Qt::red ) );
      }

      // Reset the scan curves within the new limits
      double padR = ( maxR - minR ) / 30.0;
      double padV = ( maxV - minV ) / 30.0;

      data_plot->setAxisScale( QwtPlot::yLeft  , minV - padV, maxV + padV );
      data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );

   }
   ct_selectScan->setMinimum( 1 );
   ct_selectScan->setMaximum( maxscan );

   //add fitted plot if triple has been fitted
   if ( triple_fitted_map[ triple_n ] )
     {
       QwtPlotCurve* fitdata;
       fitdata = us_curve(data_plot, title_fit + "-fit");
       fitdata->setPen(QPen(Qt::cyan));
       double* xx_ffit = (double*)xfit_data.data();    // <- the only data set in xfit_data
       double* yy_ffit = (double*)yfit_data.data();
       fitdata->setSamples( xx_ffit, yy_ffit, xfit_data.size() );

       double maxV_fit  = -1.0e99;
       double minV_fit  =  1.0e99;
       for ( int i=0; i< yfit_data.size(); i++ )
	 {
	   maxV_fit = max( maxV_fit, yfit_data[ i ] );
	   minV_fit = min( minV_fit, yfit_data[ i ] );
	 }

       double minV_final = min( minV_fit, minV);
       double maxV_final = max( maxV_fit, maxV);

       double padR = ( maxR - minR ) / 30.0;
       double padV = ( maxV_final - minV_final ) / 30.0;
       
       data_plot->setAxisScale( QwtPlot::yLeft  , minV_final - padV, maxV_final + padV );
       data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );
     }

   data_plot->replot();

   if ( us_buoyancy_auto_mode && !triple_fitted_map[ triple_n ] )
     {
       //do fit of the current scan, identify peak positions ///////////////////////////////////////////////////////////////////////
       qDebug() << "Size of v_wavelength array for triple : " << v_wavelength. last() . description  <<  v_wavelength. size();

       //DEBUG
       for( int i=0; i< v_wavelength .size(); i++)
	 {
	   WavelengthScan w_t = v_wavelength[ i ];
	   for ( int j=0; j < w_t.v_readings.size(); j++ )
	     {
	       qDebug() << "Raw Data [SET "<< i+1 << " ]: X, Y -- "
			<< w_t. v_readings[ j ]. lambda
			<< w_t. v_readings[ j ]. od;
		 }
	 }
       /////////////////////////////////////////

       bool fitting_widget = false;
       unsigned int  order = 25;
       unsigned int  parameters = order * 3 + v_wavelength.size();
       double * fitparameters = new double [parameters];
       for (int i=0; i<v_wavelength.size(); i++)
	 {
	   fitparameters[i] = 0.3;                                                  // Amplitude
	 }
       float R_step = (maxR - minR)/(order+1); // create "order" peaks evenly distributed over the range
       QString projectName = QString("");

       qDebug() << "Positions: min, max, step: " << minR << ", " << maxR << ", " << R_step;
       for (unsigned int i=0; i<order; i++)
	 {
	   fitparameters[v_wavelength.size() + (i * 3) ] = 1;                        // Addition to the amplitude
	   // spread out the peaks
	   fitparameters[v_wavelength.size() + (i * 3) + 1] = minR + R_step * i;     // Position
	   fitparameters[v_wavelength.size() + (i * 3) + 2] = 0.015;                 // Sigma
	 }
       
       //call US_Extinctfitter
       fitter = new US_ExtinctFitter(&v_wavelength, fitparameters, order, parameters,
				     projectName, &fitting_widget);
       
       connect( fitter, SIGNAL( get_yfit( QVector <QVector<double> > &, QVector <QVector<double> > & )), this, SLOT(process_yfit( QVector <QVector<double> > &, QVector <QVector<double> > & ) ) );
       
       fitter->Fit();

       //DEBUG
       // for( int i=0; i< xfit_data.size(); i++)
       // 	 {
       // 	   qDebug() << "Fit Data: X, Y -- " << xfit_data[ i ] << yfit_data[ i ];
       // 	 }
       ///////
       
       triple_fitted_map[ triple_n ] = true;
       // END of Fit ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
       
       QVector <double> peak_poss;

       if ( current_triple == 0 )
	 peak_poss = { 6.5, 6.63, 6.72 };
       else
	 peak_poss = { 6.51, 6.622, 6.722, 6.8 };
           
       triple_name_to_peaks_map[ triple_n ] = peak_poss;

     }
   
   update_fields();

   //Save & all peak reports (for current triple)
   if (  us_buoyancy_auto_mode && !triple_report_saved_map[ triple_n  ]  )
     {
       save_auto( triple_n );
       pb_view_reports  ->setEnabled( true );
     }
}


void US_Buoyancy::process_yfit(QVector <QVector<double> > &x, QVector <QVector<double> > &y)
{
  xfit_data.clear();
  yfit_data.clear();
  
  xfit_data = x.last();
  yfit_data = y.last();

  qDebug() << "Size x, y passed: " << x.size() << ", " << y.size();
  qDebug() << "Size xfit_data, yfit_data for triple: "
	   << cb_triple->itemText( current_triple )  << xfit_data.size() << ", " << yfit_data.size();
  
}


// Draw a vertical pick line
void US_Buoyancy::draw_vline( double radius )
{
   double r[ 2 ];

   r[ 0 ] = radius;
   r[ 1 ] = radius;

#if QT_VERSION < 0x050000
   QwtScaleDiv* y_axis = data_plot->axisScaleDiv( QwtPlot::yLeft );
#else
   QwtScaleDiv* y_axis = (QwtScaleDiv*)&(data_plot->axisScaleDiv( QwtPlot::yLeft ));
#endif

   double padding = ( y_axis->upperBound() - y_axis->lowerBound() ) / 30.0;

   double v[ 2 ];
   v [ 0 ] = y_axis->upperBound() - padding;
   v [ 1 ] = y_axis->lowerBound() + padding;

   v_line = us_curve( data_plot, "V-Line" );
   v_line->setSamples( r, v, 2 );

   QPen pen = QPen( QBrush( Qt::white ), 2.0 );
   v_line->setPen( pen );

   data_plot->replot();
}

void US_Buoyancy::save( void )
{
   tmp_dpoint.name = le_peakName->text();
//   tmp_dpoint.description = ;
//   tmp_dpoint.dataset = "";
   tmp_dpoint.triple = cb_triple->currentText();
   tmp_dpoint.stretch = current_stretch;
   tmp_dpoint.centerpiece = simparams[current_triple].bottom_position;
   tmp_dpoint.peakPosition = le_peakPosition->text().toDouble();
   tmp_dpoint.peakDensity = le_peakDensity->text().toDouble();
   tmp_dpoint.peakVbar = le_peakVbar->text().toDouble();
//   tmp_dpoint.temperature = 0.0;
   tmp_dpoint.bufferDensity = le_buffer_density->text().toDouble();
   tmp_dpoint.meniscus = le_meniscus->text().toDouble();
   tmp_dpoint.bottom = le_bottom_calc->text().toDouble();
   tmp_dpoint.speed = cb_rpms->currentText().toDouble();
   tmp_dpoint.gradientMW = le_MW->text().toDouble();
   tmp_dpoint.gradientVbar = le_vbar->text().toDouble();
   tmp_dpoint.gradientC0 = le_dens_0->text().toDouble();
   dpoint.append( tmp_dpoint );
}

void US_Buoyancy::save_auto( QString triple_n )
{
  QMap <QString, QStringList> peak_parms  = triple_name_to_peak_to_parms_map[ triple_n ];

  QMap < QString, QStringList >::iterator pp;
  for ( pp = peak_parms.begin(); pp != peak_parms.end(); ++pp )
    {
      QString  curr_peak_name          = pp.key();
      QStringList curr_peak_parms_list = pp.value() ;

      //these come from the QMap - per peak position
      tmp_dpoint.name         = curr_peak_name;
      tmp_dpoint.peakPosition = curr_peak_parms_list[2 ].toDouble();
      tmp_dpoint.peakDensity  = curr_peak_parms_list[0 ].toDouble();
      tmp_dpoint.peakVbar     = curr_peak_parms_list[1 ].toDouble();

      qDebug() << "Saving_auto: current_triple index -- " << current_triple;
     
      //rest is the same for all peaks withtin a triple ?
      tmp_dpoint.triple = triple_n;
      tmp_dpoint.stretch = current_stretch;
      tmp_dpoint.centerpiece = simparams[ current_triple ].bottom_position;
      tmp_dpoint.bufferDensity = le_buffer_density->text().toDouble();
      tmp_dpoint.meniscus = le_meniscus->text().toDouble();
      tmp_dpoint.bottom = le_bottom_calc->text().toDouble();
      tmp_dpoint.speed = cb_rpms->currentText().toDouble();
      tmp_dpoint.gradientMW = le_MW->text().toDouble();
      tmp_dpoint.gradientVbar = le_vbar->text().toDouble();
      tmp_dpoint.gradientC0 = le_dens_0->text().toDouble();

      
      dpoint.append( tmp_dpoint );
    }
  
  triple_report_saved_map[ triple_n ] = true;
}

void US_Buoyancy::write( void )
{
   QString str, str2;
   te = new US_Editor( US_Editor::LOAD, false, "results/*.rpt*", 0, 0 );
   te->e->setFontFamily("Arial");
   te->e->setFontPointSize( 13 );
   te->e->append("UltraScan Buoyant Density Equilibrium Analysis Report:\n");
   te->e->setFontPointSize( 11 );
   for (int i=0; i<dpoint.size(); i++)
   {
      te->e->append("Peak " + str.setNum( i+1 ) + " (" + dpoint[i].name +
      " from experiment \"" + dpoint[i].dataset + "\"):" );
      te->e->append( "Sample location:\t" + dpoint[i].triple );
      te->e->append( "Sample description:\t" + dpoint[i].description );
      te->e->append( "Rotor speed:\t" + str.setNum( dpoint[i].speed ) + " rpm, (Rotor stretch: "
      + str2.setNum( dpoint[i].stretch) + " cm)" );
      te->e->append( "Peak position:\t" + str.setNum( dpoint[i].peakPosition ) + " cm");
      te->e->append( "Peak density:\t" + str.setNum( dpoint[i].peakDensity ) + " g/ml");
      te->e->append( "Peak vbar:\t\t" + str.setNum( dpoint[i].peakVbar ) + " ml/g");
      te->e->append( "Buffer density:\t" + str.setNum( dpoint[i].bufferDensity ) + " g/ml");
      te->e->append( "Meniscus position:\t" + str.setNum( dpoint[i].meniscus ) + " cm");
      te->e->append( "Bottom of cell:\t" + str.setNum( dpoint[i].bottom ) +
      " cm (Centerpiece bottom at rest: " + str2.setNum( dpoint[i].centerpiece ) + " cm)" );
      te->e->append( "Temperature:\t" + str.setNum( dpoint[i].temperature ) + " °C");
      te->e->append( "Gradient-forming\nmaterial details:");
      te->e->append( "Molecular weight:\t" + str.setNum( dpoint[i].gradientMW ) + " g/mol" );
      te->e->append( "Loading density:\t" + str.setNum( dpoint[i].gradientC0 ) + " g/mol" );
      te->e->append( "vbar:\t\t" + str.setNum( dpoint[i].gradientVbar ) + " ml/g" );
      te->e->append("\n");
   }
   te->setMinimumHeight( 400 );
   te->setMinimumWidth( 600 );
   te->show();
}

void US_Buoyancy::write_auto( void )
{
   QString str, str2;
   te = new US_Editor( US_Editor::LOAD, false, "results/*.rpt*", 0, 0 );
   te->e->setFontFamily("Arial");
   te->e->setFontPointSize( 13 );
   te->e->append("UltraScan Buoyant Density Equilibrium Analysis Report:\n");
   te->e->setFontPointSize( 11 );
   for (int i=0; i<dpoint.size(); i++)
   {
      te->e->append("Peak " + str.setNum( i+1 ) + " (" + dpoint[i].name +
      " from experiment \"" + dpoint[i].dataset + "\"):" );
      te->e->append( "Sample location:\t" + dpoint[i].triple );
      te->e->append( "Sample description:\t" + dpoint[i].description );
      te->e->append( "Rotor speed:\t" + str.setNum( dpoint[i].speed ) + " rpm, (Rotor stretch: "
      + str2.setNum( dpoint[i].stretch) + " cm)" );
      te->e->append( "Peak position:\t" + str.setNum( dpoint[i].peakPosition ) + " cm");
      te->e->append( "Peak density:\t" + str.setNum( dpoint[i].peakDensity ) + " g/ml");
      te->e->append( "Peak vbar:\t\t" + str.setNum( dpoint[i].peakVbar ) + " ml/g");
      te->e->append( "Buffer density:\t" + str.setNum( dpoint[i].bufferDensity ) + " g/ml");
      te->e->append( "Meniscus position:\t" + str.setNum( dpoint[i].meniscus ) + " cm");
      te->e->append( "Bottom of cell:\t" + str.setNum( dpoint[i].bottom ) +
      " cm (Centerpiece bottom at rest: " + str2.setNum( dpoint[i].centerpiece ) + " cm)" );
      te->e->append( "Temperature:\t" + str.setNum( dpoint[i].temperature ) + " °C");
      te->e->append( "Gradient-forming\nmaterial details:");
      te->e->append( "Molecular weight:\t" + str.setNum( dpoint[i].gradientMW ) + " g/mol" );
      te->e->append( "Loading density:\t" + str.setNum( dpoint[i].gradientC0 ) + " g/mol" );
      te->e->append( "vbar:\t\t" + str.setNum( dpoint[i].gradientVbar ) + " ml/g" );
      te->e->append("\n");
   }
   te->setMinimumHeight( 400 );
   te->setMinimumWidth( 600 );
   te->show();
}

// Select a new speed within a triple
void US_Buoyancy::new_rpmval( int index )
{
   QString srpm = cb_rpms->itemText( index ), str;
   qDebug() << "rpmval: " << srpm;
   current_rpm = srpm.toDouble();
   current_stretch = calc_stretch();
   str.setNum( calc_stretch() );
   le_stretch->setText( str );
   if ( meniscus[current_triple] != 0 )
   {
      str.setNum( meniscus[current_triple] + current_stretch );
      le_meniscus->setText( str );
   }
   plot_scan( current_scan );
}

void US_Buoyancy::update_bottom ( void )
{
   bottom = (double) le_bottom->text().toDouble();
}

void US_Buoyancy::update_bottom_calc ( void )
{
   bottom_calc = (double) le_bottom_calc->text().toDouble();
}

void US_Buoyancy::update_dens_0 ( void )
{
   tmp_dpoint.gradientC0 = (double) le_dens_0->text().toDouble();
}

void US_Buoyancy::update_vbar ( void )
{
   tmp_dpoint.gradientVbar = (double) le_vbar->text().toDouble();
}

void US_Buoyancy::update_MW ( void )
{
   tmp_dpoint.gradientMW = (double) le_MW->text().toDouble();
}

void US_Buoyancy::update_bufferDensity ( void )
{
   tmp_dpoint.bufferDensity = (double) le_buffer_density->text().toDouble();
}

void US_Buoyancy::update_peakName ( void )
{
   tmp_dpoint.name = le_peakName->text();
}

// this function lets the user edit the meniscus position
void US_Buoyancy::update_meniscus( void )
{
   meniscus[current_triple] = (double) le_meniscus->text().toDouble() - current_stretch;
}

void US_Buoyancy::update_speedData( void )
{
   sData.clear();
   US_DataIO::SpeedData ssDat;
   int ksd    = 0;
   for ( int jd = 0; jd < allData.size(); jd++ )
   {
      data  = allData[ jd ];
      sd_offs << ksd;

      if ( jd > 0 )
         sd_knts << ( ksd - sd_offs[ jd - 1 ] );

      trip_rpms.clear();

      for ( int ii = 0; ii < data.scanData.size(); ii++ )
      {
         double  drpm = data.scanData[ ii ].rpm;
         QString arpm = QString::number( drpm );
         if ( ! trip_rpms.contains( arpm ) )
         {
            trip_rpms << arpm;
            ssDat.first_scan = ii + 1;
            ssDat.scan_count = 1;
            ssDat.speed      = drpm;
            ssDat.meniscus   = 0.0;
            ssDat.dataLeft   = 0.0;
            ssDat.dataRight  = 0.0;
            sData << ssDat;
            ksd++;
         }

         else
         {
            int jj = trip_rpms.indexOf( arpm );
            ssDat  = sData[ jj ];
            ssDat.scan_count++;
            sData[ jj ].scan_count++;
         }
      }

      if ( jd == 0 )
         cb_rpms->addItems( trip_rpms );

      total_speeds += trip_rpms.size();
   }

   sd_knts << ( ksd - sd_offs[ allData.size() - 1 ] );

   if ( allData.size() > 1 )
   {
      data   = allData[ current_triple ];
      ksd    = sd_knts[ current_triple ];
      trip_rpms.clear();
      cb_rpms ->clear();
      for ( int ii = 0; ii < ksd; ii++ )
      {
         QString arpm = QString::number( sData[ ii ].speed );
         trip_rpms << arpm;
      }
      cb_rpms->addItems( trip_rpms );
   }
}

double US_Buoyancy::calc_stretch( )
{
   return ( simparams[current_triple].rotorcoeffs[ 0 ]
		   * current_rpm + simparams[current_triple].rotorcoeffs[ 1 ]
		   * pow( current_rpm, 2.0 ) );
}

