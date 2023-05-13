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
   tmp_dpoint.peakGaussArea = 0.0;
   tmp_dpoint.temperature = 0.0;
   //tmp_dpoint.bufferDensity = 0.998234;
   tmp_dpoint.bufferDensity = 0.0;
   tmp_dpoint.meniscus = 0.0;
   tmp_dpoint.bottom = 0.0;
   tmp_dpoint.speed = 0.0;

   // tmp_dpoint.gradientMW   = 821.0;
   // tmp_dpoint.gradientVbar = 0.4831;
   // tmp_dpoint.gradientC0   = 1.2294;

   tmp_dpoint.gradientMW   = 168.36;
   tmp_dpoint.gradientVbar = 0.2661;
   tmp_dpoint.gradientC0   = 1.4200;


   tmp_dpoint.sigma = 0.015;

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
   pb_load = us_pushbutton( tr( "Load Data" ) );
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
   le_meniscus = us_lineedit(  "0.0", -1,  true );
   specs->addWidget( le_meniscus, s_row++, 2, 1, 2 );
   // connect (le_meniscus, SIGNAL( editingFinished (void)), this,
   //          SLOT (update_meniscus(void)));

   QLabel* lbl_stretch = us_label( tr( "Rotor Stretch (cm):" ), -1 );
   specs->addWidget( lbl_stretch, s_row, 0, 1, 2 );
   le_stretch = us_lineedit( "0.0", -1,  true );
   specs->addWidget( le_stretch, s_row++, 2, 1, 2 );

   QLabel* lbl_bottom = us_label( tr( "Centerpiece Bottom (cm):" ), -1 );
   specs->addWidget( lbl_bottom, s_row, 0, 1, 2 );
   le_bottom = us_lineedit( QString::number( bottom ), -1,  true );
   specs->addWidget( le_bottom, s_row++, 2, 1, 2 );
   // connect (le_bottom, SIGNAL( editingFinished (void)), this,
   //          SLOT (update_bottom(void)));

   QLabel* lbl_bottom_calc = us_label( tr( "Speed-corrected Bottom (cm):" ), -1 );
   specs->addWidget( lbl_bottom_calc, s_row, 0, 1, 2 );
   le_bottom_calc = us_lineedit( QString::number( bottom_calc ), -1,  true );
   specs->addWidget( le_bottom_calc, s_row++, 2, 1, 2 );
   // connect (le_bottom_calc, SIGNAL( editingFinished (void)), this,
   //          SLOT (update_bottom_calc(void)));


   QLabel* lbl_buffer_density = us_label( tr( "Buffer Density (g/ml):" ), -1 );
   specs->addWidget( lbl_buffer_density, s_row, 0, 1, 2 );
   le_buffer_density = us_lineedit( QString::number(tmp_dpoint.bufferDensity), -1,  true );
   specs->addWidget( le_buffer_density, s_row++, 2, 1, 2 );
   // connect (le_buffer_density, SIGNAL( editingFinished (void)), this,
   //          SLOT ( update_bufferDensity( void ) ));


   QLabel* lbl_temperature = us_label( tr( "Temperature " ) + DEGC +  ":", -1 );
   specs->addWidget( lbl_temperature, s_row, 0, 1, 2 );
   le_temperature = us_lineedit( QString::number( tmp_dpoint.temperature ), -1,  true );
   specs->addWidget( le_temperature, s_row++, 2, 1, 2 );
   

    // Solution info:
   QLabel* lbl_dens_0 = us_label( tr( "Loading Density (g/ml):" ), -1 );
   specs->addWidget( lbl_dens_0, s_row, 0, 1, 2 );
   le_dens_0 = us_lineedit( QString::number(tmp_dpoint.gradientC0), -1,  true );
   specs->addWidget( le_dens_0, s_row++, 2, 1, 2 );
   connect (le_dens_0, SIGNAL( editingFinished (void)), this,
            SLOT (update_for_dens_0(void)));

   QLabel* lbl_vbar = us_label( tr( "Gradient Mat. vbar (ml/g):" ), -1 );
   specs->addWidget( lbl_vbar, s_row, 0, 1, 2 );
   le_vbar = us_lineedit( QString::number( tmp_dpoint.gradientVbar ), -1,  true );
   specs->addWidget( le_vbar, s_row++, 2, 1, 2 );
   connect (le_vbar, SIGNAL( editingFinished (void)), this,
            SLOT (update_for_vbar(void)));

   QLabel* lbl_MW = us_label( tr( "Gradient Mat. MW (g/mol):" ), -1 );
   specs->addWidget( lbl_MW, s_row, 0, 1, 2 );
   le_MW = us_lineedit( QString::number( tmp_dpoint.gradientMW ), -1,  true );
   specs->addWidget( le_MW, s_row++, 2, 1, 2 );
   connect (le_MW, SIGNAL( editingFinished (void)), this,
            SLOT (update_for_MW(void)));


   QLabel* lbl_sigma = us_label( tr( "Peak Sigma Value (cm):" ), -1 );
   specs->addWidget( lbl_sigma, s_row, 0, 1, 2 );
   le_sigma = us_lineedit( QString::number( tmp_dpoint.sigma ), -1,  true );
   specs->addWidget( le_sigma, s_row++, 2, 1, 2 );
   connect (le_sigma, SIGNAL( editingFinished (void)), this,
            SLOT (update_for_sigma(void)));


   //Peak. Params banner
   QLabel* lb_peak_parms_bn = us_banner( tr( "Peak Parameters" ) );
   specs->addWidget( lb_peak_parms_bn,  s_row++, 0, 1, 4 );
   
   //Information on peaks: Listbox of to-be-determined (from automated fit) size
      
   QLabel* lbl_peakName = us_label( tr( "Peak name/label:" ), -1 );
   specs->addWidget( lbl_peakName, s_row, 0, 1, 1 );

   cb_peaks = us_comboBox();
   // connect( cb_peaks, SIGNAL( currentIndexChanged( int ) ),
   // 	              SLOT  ( new_peak           ( int ) ) );
   specs->addWidget( cb_peaks, s_row++, 1, 1, 3 );

   le_peakName = us_lineedit( tmp_dpoint.name );
   specs->addWidget( le_peakName, s_row++, 2, 1, 2 );
   connect (le_peakName, SIGNAL( editingFinished (void)), this,
            SLOT (update_peakName(void)));
   

   QLabel* lbl_peakPosition = us_label( tr( "Peak Position (cm):" ), -1 );
   specs->addWidget( lbl_peakPosition, s_row, 0, 1, 1 );
   le_peakPosition = us_lineedit( QString::number( tmp_dpoint.peakPosition ), -1, true );
   specs->addWidget( le_peakPosition, s_row++, 1, 1, 3 );

   QLabel* lbl_peakDensity = us_label( tr( "Peak Density (g/ml):" ), -1 );
   specs->addWidget( lbl_peakDensity, s_row, 0, 1, 1 );
   le_peakDensity = us_lineedit( QString::number( tmp_dpoint.peakDensity ), -1, true );
   specs->addWidget( le_peakDensity, s_row++, 1, 1, 3 );

   QLabel* lbl_peakVbar = us_label( tr( "Peak vbar (ml/g):" ), -1 );
   specs->addWidget( lbl_peakVbar, s_row, 0, 1, 1 );
   le_peakVbar = us_lineedit( QString::number( tmp_dpoint.peakVbar ), -1, true );
   specs->addWidget( le_peakVbar, s_row++, 1, 1, 3 );

   QLabel* lbl_peakGaussArea = us_label( tr( "Gauss Area (% of total):" ), -1 );
   specs->addWidget( lbl_peakGaussArea, s_row, 0, 1, 1 );
   le_peakGaussArea = us_lineedit( QString::number( tmp_dpoint.peakGaussArea ), -1, true );
   specs->addWidget( le_peakGaussArea, s_row++, 1, 1, 3 );   

   //Peak Editor
   QLabel* lb_peak_editor_bn = us_banner( tr( "Peak Editor" ) );
   specs->addWidget( lb_peak_editor_bn,  s_row++, 0, 1, 4 );

   pb_delete_peak = us_pushbutton( tr( "Delete Current Peak" ), false );
   connect( pb_delete_peak, SIGNAL( clicked() ), SLOT( delete_peak() ) );
   specs->addWidget( pb_delete_peak, s_row, 0, 1, 2 );

   pb_add_peak = us_pushbutton( tr( "Add Peak Manually" ), false );
   connect( pb_add_peak, SIGNAL( clicked() ), SLOT( add_peak() ) );
   specs->addWidget( pb_add_peak, s_row++, 2, 1, 2 );
   
   // Button rows
   QBoxLayout* buttons = new QHBoxLayout;

   pb_save = us_pushbutton( tr( "Save Datapoint" ), false );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
   //specs->addWidget( pb_save, s_row, 0, 1, 2 );

   pb_write = us_pushbutton( tr( "Write Report" ), false );
   connect( pb_write, SIGNAL( clicked() ), SLOT( write() ) );
   //specs->addWidget( pb_write, s_row++, 2, 1, 2 );

   
   //Also add Fitting progress
   QLabel* lb_fit_progress_bn = us_banner( tr( "Fitting" ) );
   specs->addWidget( lb_fit_progress_bn,  s_row++, 0, 1, 4 );

   us_checkbox( tr( "[AUTO] Fit All Triples" ), ck_auto_fit, true );
   connect( ck_auto_fit, SIGNAL( toggled     ( bool ) ),
	    this,        SLOT  ( enblFitBtn  ( bool ) ) );
   specs->addWidget( ck_auto_fit,  s_row, 0, 1, 2 );
   pb_fit_current_triple = us_pushbutton( tr( "Fit Current Triple" ), false );
   connect( pb_fit_current_triple, SIGNAL( clicked() ), SLOT( fit_current_triple() ) );
   specs->addWidget( pb_fit_current_triple, s_row++, 2, 1, 2 );
   
   QLabel* lbl_pgb_progress = us_label( tr( "Fitting Progress:" ), -1 );
   specs->addWidget( lbl_pgb_progress, s_row, 0, 1, 1 );
   pgb_progress = new QProgressBar(this);
   specs->addWidget( pgb_progress, s_row++, 1, 1, 3 );
   
   
      
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
       lbl_scan      ->hide();
       ct_selectScan ->hide();

       rb_meniscus   ->hide();
       rb_datapoint  ->hide();

       le_peakName   ->hide();

       pb_save       ->hide();
       pb_write      ->hide();
     }
   

   
   // Plot layout on right side of window
   plot = new US_Plot( data_plot,
         tr( "Buoyant Density Equilibrium" ),
         tr( "Radius (in cm)" ), tr( "Absorbance" ) );

   data_plot->setMinimumSize( 600, 400 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );
   data_plot->enableAxis( QwtPlot::yRight , true );


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
   pb_load = us_pushbutton( tr( "Load Data" ) );
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

   //QString DEGC   = QString( QChar( 176 ) ) + "C"; //!< Degree-symbol + "C"
   QLabel* lbl_temperature = us_label( tr( "Temperature " ) + DEGC +  ":", -1 );
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

  double percent_of_total = curr_peak_parms[5].toDouble() / curr_peak_parms[6].toDouble() * 100.0;
  
  le_peakDensity   -> setText( curr_peak_parms[0] );
  le_peakVbar      -> setText( curr_peak_parms[1] );
  le_peakPosition  -> setText( curr_peak_parms[2] );
  le_peakGaussArea -> setText( QString::number(percent_of_total, 'f', 2) );
  //le_peakGaussArea -> setText( curr_peak_parms[5] + "   (" + QString::number(percent_of_total, 'f', 2) + "% of total)" );

  
  //highligth peak line
  if( triple_name_to_peak_curves_map. contains( triple_n ) &&
      triple_name_to_peak_gauss_envelopes_map. contains ( triple_n ) )
    {
      QPen pen = QPen( QBrush( Qt::magenta ), 2.0, Qt::DotLine );
      QVector< QwtPlotCurve* > v_line_vector = triple_name_to_peak_curves_map[ triple_n ];
      QVector< QwtPlotCurve* > gauss_vector  = triple_name_to_peak_gauss_envelopes_map[ triple_n ];
      qDebug() << "v_line_vector size(), index -- " <<  v_line_vector.size() << index;

      if( index < v_line_vector.size() )
	{
	  v_line_vector[ index ]->setPen( pen );
	  gauss_vector [ index ]->setPen( pen );
	}
      
      //the rest of line are show with default pen (yellow)
      for ( int i=0; i < v_line_vector.size(); ++i )
	{
	  if ( i == index )
	    continue;

	  pen = QPen( QBrush( Qt::yellow ), 1.0, Qt::DotLine );
	  v_line_vector[ i ]->setPen( pen );
	  gauss_vector[ i ] ->setPen( pen );
	}

      //and replot...
      data_plot->replot();
    }
}

//Enable / Disable Fit btn
void US_Buoyancy::enblFitBtn( bool checked )
{
  if ( checked )
    pb_fit_current_triple -> setEnabled( false );
  else
    pb_fit_current_triple -> setEnabled( true );
}

//Fit current triple
void US_Buoyancy::fit_current_triple ( void )
{

  if ( cb_triple->count() == 0 )
    return;
  
  QString triple_n = cb_triple->itemText( current_triple );
  do_fit_for_triple[ triple_n ] = true;

  plot_scan ( cb_triple->currentIndex() );   //compute
  plot_scan ( cb_triple->currentIndex() );   //plot

  //re-save all reports
  dpoint. clear();
  for ( int i=0; i< cb_triple->count(); i++ )
    {
      save_auto ( cb_triple->itemText( i ) ); 
    }

  //disable Fit btn for current triple
  pb_fit_current_triple -> setEnabled( false );

  //enable other relevant fields
  us_setReadOnly ( le_dens_0, false );
  us_setReadOnly ( le_vbar,   false );
  us_setReadOnly ( le_MW,     false );
  us_setReadOnly ( le_sigma,  false );
  
  pb_view_reports  ->setEnabled( true );
  pb_delete_peak   ->setEnabled( true );
  pb_add_peak      ->setEnabled( true );
  
}


// Select a new triple
void US_Buoyancy::new_triple( int index )
{

  QString triple_n = cb_triple->itemText( index );
  if ( do_fit_for_triple[ triple_n ] )
    {
      pb_fit_current_triple -> setEnabled( false );

      us_setReadOnly ( le_dens_0, false );
      us_setReadOnly ( le_vbar,   false );
      us_setReadOnly ( le_MW,     false );
      us_setReadOnly ( le_sigma,  false );
      
      //pb_view_reports  ->setEnabled( true );
      pb_delete_peak   ->setEnabled( true );
      pb_add_peak      ->setEnabled( true );
    }
  else
    {
      pb_fit_current_triple -> setEnabled( true );

      us_setReadOnly ( le_dens_0, true );
      us_setReadOnly ( le_vbar,   true );
      us_setReadOnly ( le_MW,     true );
      us_setReadOnly ( le_sigma,  true );

      //pb_view_reports  ->setEnabled( false );
      pb_delete_peak   ->setEnabled( false );
      pb_add_peak      ->setEnabled( false );      
    }
      
  current_triple = index;
  data = allData[ index ];
  le_info->setText( runID + ": " + allData[index].description );
  tmp_dpoint.description = allData[index].description;
  tmp_dpoint.dataset = runID;

  qDebug() << "In new_triple() 1";
    
  plot_scan( current_scan );
}

void US_Buoyancy::update_fields( void )
{
   QString str;
   QString triple_n = cb_triple->itemText( current_triple );
   
   current_stretch = calc_stretch();
   str.setNum( current_stretch );
   le_stretch->setText( str );

   if ( meniscus[current_triple] != 0 )
   {
     if ( !us_buoyancy_auto_mode )  
       tmp_dpoint.meniscus = meniscus[current_triple] + current_stretch;
     else
       tmp_dpoint.meniscus = meniscus_to_triple_name_map[ triple_n ];

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
       le_buffer_density ->setText( QString::number(  buffDensity_to_triple_name_map[ triple_n ] ));
       le_sigma          ->setText( QString::number(  sigma_to_triple_name_map    [ triple_n ] ));
       le_dens_0         ->setText( QString::number(  gradC0_to_triple_name_map   [ triple_n ] ));
       le_vbar           ->setText( QString::number(  gradVbar_to_triple_name_map [ triple_n ] ));
       le_MW             ->setText( QString::number(  gradMW_to_triple_name_map   [ triple_n ] ));
	 
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

/* 
// cacl. poitns based on provided vector of peak positions
void US_Buoyancy::calc_points_auto( QString triple_n )
{
  QVector< double > peak_poss = triple_name_to_peaks_map[ triple_n ];
  triple_name_to_peak_to_parms_map[ triple_n ] . clear();

  double tot_area  = triple_name_to_total_area[ triple_n ];
  double sigma_gui = sigma_to_triple_name_map [ triple_n ];
  double gradMW    = gradMW_to_triple_name_map [ triple_n ];
  double gradVbar  = gradVbar_to_triple_name_map [ triple_n ];
  double gradC0    = gradC0_to_triple_name_map [ triple_n ];

  double tot_area_uncorrected  = triple_name_to_total_area_uncorrected[ triple_n ];
  
  QStringList peak_names;

  for (int rr=0; rr < xfit_data[ triple_n ].size(); rr++ )
    {
      double curr_x = xfit_data[ triple_n ][rr];
      bool isPeak = false;
      int peak_number = 0;
      
      double omega_s, C, C0, r2, k1, k2, k3, k4;
      omega_s = pow( current_rpm * M_PI/30.0, 2.0 );
      C0 = gradC0 - buffDensity_to_triple_name_map[ triple_n ]; //subtract buffer density from nycodenz density
      r2 = pow( tmp_dpoint.bottom, 2.0 ) - pow( meniscus_to_triple_name_map[ triple_n ], 2.0);
      k1 = gradMW * omega_s/( 2.0 * R_GC * (tmp_dpoint.temperature + 273.15) );
      k4 = 1.0 - gradVbar * buffDensity_to_triple_name_map[ triple_n ];
      k2 = exp( k1 * ( k4 ) * (pow( curr_x, 2.0 ) - pow( meniscus_to_triple_name_map[ triple_n ], 2.0 ) ) );
      k3 = exp( k1 * ( k4 ) * r2);
      C  = k1 * k4 * C0 *r2 * k2/( k3 - 1.0 ) + buffDensity_to_triple_name_map[ triple_n ];

      

      //iterate over peaks for the current triple
      for ( int i=0; i< peak_poss.size(); i++ )
	{
	  if ( curr_x == peak_poss[i] )
	    {
	      isPeak = true;
	      peak_number = i+1;
	      break;
	    }
	}

      if ( isPeak )
	{
	  QString str;
	  if ( curr_x != 0.0 )
	    {
	      QString peak_name = "Peak #" + QString::number( peak_number );
	      peak_names << peak_name;
	      
	      QStringList curr_peak_parms;
	      str.setNum( C );
	      curr_peak_parms << str;                     //0
	      //le_peakDensity->setText( str );
	      
	      str.setNum( 1.0/C );
	      curr_peak_parms << str;                     //1
	      //le_peakVbar->setText( str );
	      
	      str.setNum( curr_x );
	      curr_peak_parms << str;                     //2   
	      //le_peakPosition->setText( str );
	      
	      //identify closest (position-wise) fitted Gaussian && its sigma, as well as peak amplitude 
	      QMap< QString, double > peak_sigma_height = find_closest_sigma_height( triple_n, curr_x );
	      peak_sigma_height[ "sigma" ] = sigma_gui;  // re-write sigma as fittted parm-sigma does not work...
	      
	      curr_peak_parms << QString::number( peak_sigma_height[ "sigma" ] )                 //3
			      << QString::number( peak_sigma_height[ "height" ] );               //4
	      
	      //add area under peak
	      QMap< QString, double > gauss_areas = calc_gauss_area( triple_n, curr_x, peak_sigma_height[ "sigma" ], peak_sigma_height[ "height" ] );
	      double gauss_area             = gauss_areas[ "corrected" ];
	      double gauss_area_uncorrected = gauss_areas[ "uncorrected" ];
	      
	      curr_peak_parms << QString::number( gauss_area );                  //5
	      curr_peak_parms << QString::number( tot_area );                    //6
	      
	      curr_peak_parms << QString::number( gauss_area_uncorrected );      //7
	      curr_peak_parms << QString::number( tot_area_uncorrected );        //8
	      
	      triple_name_to_peak_to_parms_map[ triple_n ][ peak_name ] = curr_peak_parms;
	      
	      pb_save->setEnabled( true );
	    }
	  else
	    {
	      // le_peakDensity->setText( "0" );
	      // le_peakVbar->setText( "0" );
	      // le_peakPosition->setText( "0" );
	      // pb_save->setEnabled( false );
	      
	      qDebug() << "Peak position #" <<  peak_number << " for triple " << triple_n << " is ZERO!";
	    }
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
*/

// cacl. poitns based on provided vector of peak positions
void US_Buoyancy::calc_points_auto( QString triple_n )
{
  QVector< double > peak_poss = triple_name_to_peaks_map[ triple_n ];
  triple_name_to_peak_to_parms_map[ triple_n ] . clear();
  triple_name_to_Cdata[ triple_n ]. clear();

  double tot_area  = triple_name_to_total_area[ triple_n ];
  double sigma_gui = sigma_to_triple_name_map [ triple_n ];
  double gradMW    = gradMW_to_triple_name_map [ triple_n ];
  double gradVbar  = gradVbar_to_triple_name_map [ triple_n ];
  double gradC0    = gradC0_to_triple_name_map [ triple_n ];
  double tot_area_uncorrected  = triple_name_to_total_area_uncorrected[ triple_n ];

  //Cacl. separately C for all xdata_fit range:
  for (int rr=0; rr < xfit_data[ triple_n ].size(); rr++ )
    {
      double curr_x = xfit_data[ triple_n ][rr];

      double omega_s1, C1, C01, r21, k11, k21, k31, k41;
      omega_s1 = pow( current_rpm * M_PI/30.0, 2.0 );
      C01 = gradC0 - buffDensity_to_triple_name_map[ triple_n ]; //subtract buffer density from nycodenz density
      r21 = pow( tmp_dpoint.bottom, 2.0 ) - pow( meniscus_to_triple_name_map[ triple_n ], 2.0);
      k11 = gradMW * omega_s1/( 2.0 * R_GC * (tmp_dpoint.temperature + 273.15) );
      k41 = 1.0 - gradVbar * buffDensity_to_triple_name_map[ triple_n ];
      k21 = exp( k11 * ( k41 ) * (pow( curr_x, 2.0 ) - pow( meniscus_to_triple_name_map[ triple_n ], 2.0 ) ) );
      k31 = exp( k11 * ( k41 ) * r21);
      C1  = k11 * k41 * C01 *r21 * k21/( k31 - 1.0 ) + buffDensity_to_triple_name_map[ triple_n ];

      triple_name_to_Cdata[ triple_n ]. push_back( C1 );
    }

  // //DEBUG
  // for ( int i=0; i<triple_name_to_Cdata[ triple_n ].size(); i++ )
  //   qDebug() << "C data for triple - " << triple_n << ": " << triple_name_to_Cdata[ triple_n ][i];
  
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
	  C0 = gradC0 - buffDensity_to_triple_name_map[ triple_n ]; //subtract buffer density from nycodenz density
	  r2 = pow( tmp_dpoint.bottom, 2.0 ) - pow( meniscus_to_triple_name_map[ triple_n ], 2.0);
	  k1 = gradMW * omega_s/( 2.0 * R_GC * (tmp_dpoint.temperature + 273.15) );
	  k4 = 1.0 - gradVbar * buffDensity_to_triple_name_map[ triple_n ];
	  k2 = exp( k1 * ( k4 ) * (pow( peak_poss[ i ], 2.0 ) - pow( meniscus_to_triple_name_map[ triple_n ], 2.0 ) ) );
	  k3 = exp( k1 * ( k4 ) * r2);
	  C  = k1 * k4 * C0 *r2 * k2/( k3 - 1.0 ) + buffDensity_to_triple_name_map[ triple_n ];

	  QStringList curr_peak_parms;
	  str.setNum( C );
	  curr_peak_parms << str;                     //0
	  //le_peakDensity->setText( str );
	  
	  str.setNum( 1.0/C );
	  curr_peak_parms << str;                     //1
	  //le_peakVbar->setText( str );
	  
	  str.setNum( peak_poss[ i ] );
	  curr_peak_parms << str;                     //2   
	  //le_peakPosition->setText( str );

	  //identify closest (position-wise) fitted Gaussian && its sigma, as well as peak amplitude 
	  QMap< QString, double > peak_sigma_height = find_closest_sigma_height( triple_n, peak_poss[ i ] );
	  peak_sigma_height[ "sigma" ] = sigma_gui;  // re-write sigma as fittted parm-sigma does not work...

	  curr_peak_parms << QString::number( peak_sigma_height[ "sigma" ] )                                      //3
			  << QString::number( peak_sigma_height[ "height" ] );                                    //4

	  //add area under peak
	  QMap< QString, double > gauss_areas = calc_gauss_area( triple_n, peak_poss[ i ], peak_sigma_height[ "sigma" ], peak_sigma_height[ "height" ] );
	  double gauss_area             = gauss_areas[ "corrected" ];
	  double gauss_area_uncorrected = gauss_areas[ "uncorrected" ];
	  
	  curr_peak_parms << QString::number( gauss_area );                  //5
	  curr_peak_parms << QString::number( tot_area );                    //6

	  curr_peak_parms << QString::number( gauss_area_uncorrected );      //7
	  curr_peak_parms << QString::number( tot_area_uncorrected );        //8
	  
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



//calc total area under the fit curve
QMap<QString, double> US_Buoyancy::calc_total_area( QString triple_n )
{
  double total_area  = 0;
  double total_area_uncorrected  = 0;
  double alpha       = alpha_centerpiece[ triple_n ];
  bool sectoral;

  ( alpha != 0 ) ? sectoral = true : sectoral = false; 
  
  for ( int ii=0; ii < xfit_data[ triple_n ].size() - 1; ++ii )
    {
      double point1 = xfit_data[ triple_n ][ ii ];
      double point2 = xfit_data[ triple_n ][ ii + 1];
      double jac    = qAbs( point2 - point1 );                   //dx
      
      if ( sectoral )
	total_area  += yfit_data[ triple_n ][ ii ] * jac
	  * alpha/360.0 * M_PI * ( pow(point2, 2) - pow(point1, 2) );
      else
	total_area  += yfit_data[ triple_n ][ ii ] * jac;

      total_area_uncorrected  += yfit_data[ triple_n ][ ii ] * jac; 
    }

  qDebug() << "Total Area, Triple -- "  << triple_n << total_area;

  QMap< QString, double > tot_areas;
  tot_areas[ "corrected" ]   = total_area;
  tot_areas[ "uncorrected" ] = total_area_uncorrected;
  
  return tot_areas;
}

//calculate area under Gauss function given by pos, sigma, height
QMap<QString, double> US_Buoyancy::calc_gauss_area( QString triple_n, double pos_p, double sigma_p, double height_p )
{
  double gauss_area = 0;
  double gauss_area_uncorrected = 0;
  double alpha       = alpha_centerpiece[ triple_n ];
  bool sectoral;

  ( alpha != 0 ) ? sectoral = true : sectoral = false; 
      
  for ( int ii=0; ii < xfit_data[ triple_n ].size() - 1; ++ii )
    {
      double point1 = xfit_data[ triple_n ][ ii ];
      double point2 = xfit_data[ triple_n ][ ii + 1];
      double jac    = qAbs( point2 - point1 );                       //dx

      double x_val = xfit_data[ triple_n ][ ii ];

      if ( sectoral )
	gauss_area  += height_p * exp( - ( pow(( x_val - pos_p), 2 )) / ( 2 * pow( sigma_p, 2)) ) * jac
	  * alpha/360.0 * M_PI * ( pow(point2, 2) - pow(point1, 2) );
      else
	gauss_area  += height_p * exp( - ( pow(( x_val - pos_p), 2 )) / ( 2 * pow( sigma_p, 2)) ) * jac;

      gauss_area_uncorrected  += height_p * exp( - ( pow(( x_val - pos_p), 2 )) / ( 2 * pow( sigma_p, 2)) ) * jac;
    }

  QMap< QString, double > gauss_areas;
  gauss_areas[ "corrected" ]   = gauss_area;
  gauss_areas[ "uncorrected" ] = gauss_area_uncorrected;

  return gauss_areas;
}

//for each triple's peak, find closest sigma (fitted) && height
QMap< QString, double > US_Buoyancy::find_closest_sigma_height( QString triple_n, double peak_p )
{
  QMap< QString, double > parms;
  
  //sigma
  double* fit_p = triple_name_to_fit_parameters[ triple_n ];
  double fitted_sigma = 0;
  double min_separation = 10000;
  for (unsigned int i=0; i < gauss_order_minVariance[ triple_n ]; i++)
    {
      //qDebug() << tr("Peak position: ")      << fit_p[v_wavelength.size() + (i * 3) + 1]  << " nm\n";
      //qDebug() << tr("Amplitude of peak: ")  << exp(fit_p[v_wavelength.size() + (i * 3)]) << tr(" extinction\n");
      //qDebug() << tr("Peak width: ")         << fit_p[v_wavelength.size() + (i * 3) + 2]  << " nm\n";

      double fitted_peak_p = fit_p[v_wavelength.size() + (i * 3) + 1];
      if ( qAbs( fitted_peak_p - peak_p ) < min_separation )
	{
	  min_separation = qAbs( fitted_peak_p - peak_p );
	  fitted_sigma   = fit_p[v_wavelength.size() + (i * 3) + 2];
	}
    }

  //peak's height
  double fitted_height = 0;
  int peak_index           = index_of_data( xfit_data[ triple_n ], peak_p );
  fitted_height            = yfit_data[ triple_n ][ peak_index ];
    
  parms[ "sigma"  ] = fitted_sigma;
  //parms[ "sigma"  ] = 0.015;          // TEST <--- HARD CODED
  parms[ "height" ] = fitted_height;

  return parms;
}


// load AUC data
void US_Buoyancy::load( void )
{
   bool isLocal = ! disk_controls->db();
   reset();

   // US_LoadAUC* dialog =
   //    new US_LoadAUC( isLocal, allData, triples, workingDir );

   US_LoadAUC* dialog;
   if ( us_buoyancy_auto_mode )
     dialog = new US_LoadAUC( isLocal, "AUTO", allData, triples, workingDir );
   else
     dialog = new US_LoadAUC( isLocal, allData, triples, workingDir );

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

   // //DEBUG: TEMPORARY! /////////////////////////////////////////////////////////////////
   // QStringList triples_temp = triples;
   // triples.clear();
   // triples << triples_temp[0] << triples_temp[ triples_temp.size() - 1 ];
   // //////////////////////////////////////////////////////////////////////////////////////
   
   //check if [AUTO for fit all triples]
   if ( us_buoyancy_auto_mode )
     {
       if ( ck_auto_fit -> isChecked() )
	 {
	   for (int ii=0; ii<triples.size(); ii++)
	     do_fit_for_triple[ triples[ ii ] ] = true;
	 }
       else
	 {
	   for (int ii=0; ii<triples.size(); ii++)
	     do_fit_for_triple[ triples[ ii ] ] = false;
	 }
	 
       ck_auto_fit      ->setEnabled( false );
       pb_load          ->setEnabled( false );
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

   // //DEBUG: rawGUID
   // for ( int ii=0; ii<triples.size(); ii++ )
   //   {
   //     QString rawGUID_test  = US_Util::uuid_unparse( (unsigned char*)allData[ ii ].rawGUID );
   //     qDebug() << " DataSet: " << ii << ", RawData GUID -- " << rawGUID_test;
   //     qDebug() << " DataSet: " << ii << ", Description --" << allData[ ii ].description;
   //   }
   // return;
   // //
   
   simparams.resize(triples.size());
   meniscus.resize(triples.size());

   for (int ii=0; ii<triples.size(); ii++)
   {
     //ALEXEY: if auto_mode, read meniscus positions from editProfiles
      if ( us_buoyancy_auto_mode )
	{
	  QString rawGUID_t  = US_Util::uuid_unparse( (unsigned char*)allData[ ii ].rawGUID );

	  // //DEBUG /////////////////////////////////////////////////////////////////////////////////////////////////
	  // QString rawGUID_t;
	  // if ( ii == 0 )
	  //   rawGUID_t  = US_Util::uuid_unparse( (unsigned char*)allData[ ii ].rawGUID );
	  // else
	  //   rawGUID_t  = US_Util::uuid_unparse( (unsigned char*)allData[ triples_temp.size() - 1 ].rawGUID );
	  // /////////////////////////////////////////////////////////////////////////////////////////////////////////
	  
	  QMap <QString, double > data_conf = get_data_conf_from_edit_profile ( rawGUID_t, triples[ii] );
	  meniscus[ ii ] = data_conf[ "meniscus" ];
	  
	  if ( meniscus[ ii ] == 0 )  // if no edit profile found, then HARD code something
	    meniscus[ ii ] = 6.04519984;
	    
	  // if ( ii == 0 )
	  //   meniscus[ ii ] = 6.04519984; //HARD coded for now
	  // else
	  //   meniscus[ ii ] = 6.01; //HARD coded for now

	  meniscus_to_triple_name_map   [ triples[ii] ] = meniscus[ ii ];
	  data_left_to_triple_name_map  [ triples[ii] ] = data_conf[ "data_left" ];
	  data_right_to_triple_name_map [ triples[ii] ] = data_conf[ "data_right" ];
	  buffDensity_to_triple_name_map[ triples[ii] ] = data_conf[ "buffer_density" ];
	  alpha_centerpiece             [ triples[ii] ] = data_conf[ "alpha_centerpiece" ];

	  sigma_to_triple_name_map      [ triples[ii] ] = tmp_dpoint.sigma;
	  gradMW_to_triple_name_map     [ triples[ii] ] = tmp_dpoint.gradientMW;
	  gradVbar_to_triple_name_map   [ triples[ii] ] = tmp_dpoint.gradientVbar;
	  gradC0_to_triple_name_map     [ triples[ii] ] = tmp_dpoint.gradientC0;
	  
	  triple_report_saved_map       [ triples[ii] ] = false;
	  triple_fitted_map             [ triples[ii] ] = false;
	  triple_peaks_defined_map      [ triples[ii] ] = false;


	  // DEBUG: and output xy for last scan of each triple: DEBUG
	  //print_xy( allData[ ii ], ii );
	}
      else
	meniscus[ii] = 0.0;
   }

   
   //return; // DEBUG
   
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

      if ( !us_buoyancy_auto_mode ) 
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
   //data_plot->setTitle( str1 );

   data_plot->setTitle( "Buoyant Density Equilibrium" );
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
       if ( ck_auto_fit->isChecked() )
	 {
	   for ( int i=0; i< cb_triple->count(); i++ )
	     {
	       //new_triple( i );
	       cb_triple->setCurrentIndex( i );
	     }
	   
	   //set back to the first triple
	   cb_triple->setCurrentIndex( 0 );
	   
	   //enable needed fields | buttons
	   us_setReadOnly ( le_dens_0, false );
	   us_setReadOnly ( le_vbar,   false );
	   us_setReadOnly ( le_MW,     false );
	   us_setReadOnly ( le_sigma,  false );
	   
	   pb_view_reports  ->setEnabled( true );
	   pb_delete_peak   ->setEnabled( true );
	   pb_add_peak      ->setEnabled( true );
	 }
       else  //fit each triple individually upon pressing 'Fit Current Triple': disable checkbox
	 {
	   
	 }
     }
       
}

//for debug
void US_Buoyancy::print_xy( US_DataIO::RawData curr_set, int set_num )
{
   int    rsize = curr_set.pointCount();
   int    ssize = curr_set.scanCount();
   int    count = 0;
   
   for ( int ii = 0; ii < ssize; ii++ )
   {
     if( ii != ssize - 1 )
       continue;
     
     US_DataIO::Scan* s = &data.scanData[ ii ];
     
     for ( int jj = 0; jj < rsize; jj++ )
      {
	qDebug() << "SET [" << set_num << "] -- "
		 << curr_set.xvalues[ jj ] << s->rvalues[ jj ];
      }
   }
}


//get meniscus position from edited Data
QMap< QString, double > US_Buoyancy::get_data_conf_from_edit_profile ( QString rawGUID_t, QString triple_n )
{
  QMap< QString, double > data_conf; 
  double meniscus_p = 0;
  double data_left  = 0;
  double data_right = 0;
  double buffer_density = 0;
  double centerpiece_angle = 0;
  
  // Check DB connection
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db( masterPW );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db.lastError() );
      return data_conf;
    }

  QStringList qry;
  qry.clear();
  qry << "get_rawDataID_from_GUID" << rawGUID_t;
  db.query( qry );

  db.next();
  QString rawDataID_t = db.value( 0 ).toString();
  QString expID       = db.value( 1 ).toString();
  QString soluID      = db.value( 2 ).toString();

  //Get cell information from expID, and the centerpiece geometry
  QList<cellInfo> cells; 
  qry.clear();
  qry << "all_cell_experiments" << expID;
  db.query( qry );
  while ( db.next() )
    {
      struct cellInfo cell;
      QString letters("SABCDEFGH");
      cell.cellName      = db. value( 2 ).toString();
      cell.channelName   = QString( letters[ qMax( 0, db. value( 3 ).toInt() ) ] );
      cell.centerpieceID = db. value( 4 ).toInt();
      cells << cell;
    }
  //now match cell's & channel name to those of triple && prescribe centerpiece
  int centerpieceID = 0;
  QStringList tripleName = triple_n.split("/");
  
  foreach ( struct cellInfo cell, cells )
    {
      //DEBUG
      qDebug() << "Cell, Channel from tripleName -- "
	       << tripleName[ 0 ].simplified()
	       << tripleName[ 1 ].simplified();
      qDebug() << "Cell, Channel from Cell -- "
	       << cell.cellName
	       << cell.channelName;
 
      if ( tripleName[ 0 ].simplified() == cell.cellName &&
	   tripleName[ 1 ].simplified() == cell.channelName )
	{
	  centerpieceID = cell.centerpieceID;
	  break;
	}
    }
  //finally, get get_abstractCenterpiece_info
  qry.clear();
  qry << "get_abstractCenterpiece_info" << QString::number( centerpieceID );
  db.query( qry );
  QString centerpiece_name;
  while ( db.next() )
    {
      centerpiece_name  = db.value( 1 ).toString();
      centerpiece_angle = db.value( 7 ).toDouble();
    }
  qDebug() << "Centerpiece name, angle -- " << centerpiece_name << centerpiece_angle;
  
  
  //now, get editDataID(s) for given rawDataID_t
  qry.clear();
  qry << "get_editedDataIDs" << rawDataID_t;
  db.query( qry );

  QStringList editDataIDs;
  QStringList filenames;
  
  while ( db.next() )
    {
      editDataIDs << db.value( 0 ).toString();
      filenames   << db.value( 2 ).toString();
    }
  
  if ( editDataIDs.size() == 0 )
    {
      QMessageBox::warning( this,
			    tr( "Edit data is not in DB" ),
			    tr( "Cannot find any edit records in the database.\n" ) );
      
      return data_conf;
    }

  //download editedData by eID
  //last eprofile
  int eID = editDataIDs.last().toInt();
  QString filename = workingDir + "/" + filenames.last();
  db.readBlobFromDB( filename, "download_editData", eID );

  //read XML section && extract meniscus value
  QFile pfile( filename );
  // Skip if there is a file-open problem
  if ( pfile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      // Capture the XML as a string and start XML reader
      QTextStream tsi( &pfile );
      QString xmlstr      = tsi.readAll();
      pfile.close();
      QXmlStreamReader xmli( xmlstr );
      
      while( ! xmli.atEnd() )
	{  
	  xmli.readNext();
	  QString ename       = xmli.name().toString();
	  
	  if ( xmli.isStartElement()  &&  ename == "meniscus" )
	    {
	      QXmlStreamAttributes attr = xmli.attributes();
	      meniscus_p                = attr.value( "radius" ).toDouble();
	    }
	  else if ( xmli.isStartElement()  &&  ename == "data_range" )
	    {
	      QXmlStreamAttributes attr = xmli.attributes();
	      data_left                 = attr.value( "left" ).toDouble();
	      data_right                = attr.value( "right" ).toDouble();
	    }
	  
	}
    }

  //Now, get buffer Info (density)
  qry.clear();
  qry << "get_solutionBuffer" << soluID;
  qDebug() << "Query for solutionBuffer -- " << qry;
  db.query( qry );
   
  if ( db.lastErrno() != US_DB2::OK )
    {
      qry.clear();
      qry << "get_solutionIDs" << expID;
      db.query( qry );
      db.next();
      soluID = db.value( 0 ).toString();
      
      qry.clear();
      qry << "get_solutionBuffer" << soluID;
      db.query( qry );

      if ( db.lastErrno() != US_DB2::OK )
	{
	  QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Unable to get solutionBuffer from solution ID" ) + soluID + "\n"  + db.lastError() );
	}
    }
  
  db.next();
  QString id       = db.value( 0 ).toString();
  QString guid     = db.value( 1 ).toString();
  QString desc     = db.value( 2 ).toString();

  qDebug() << "SolutionBuffer results: " << id << guid << desc;

  QString bufId, bufGuid, bufDesc; 
  if ( ! id.isEmpty() )
    {
      bufId         = id;
      bufGuid       = guid.isEmpty() ? bufGuid : guid;
      bufDesc       = desc.isEmpty() ? bufDesc : desc;
      
      qry.clear();
      qry << "get_buffer_info" << bufId;
      qDebug() << "Query for buffer info -- " << qry;
      db.query( qry );
      
      if ( db.lastErrno() != US_DB2::OK )
	{

	  QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Unable to get buffer info from buffer ID" ) + bufId + "\n"  + db.lastError() );
	  return data_conf;
	}
      
      db.next();
      QString ddesc = db.value( 1 ).toString();
      QString dcomp = db.value( 2 ).toString();
      QString dvisc = db.value( 4 ).toString();
      QString ddens = db.value( 5 ).toString();
      QString dmanu = db.value( 6 ).toString();
      
      buffer_density = ddens. toDouble();
    }
  /////////////////////////////////////////////////////////////////////////////

  data_conf[ "meniscus" ]          = meniscus_p;
  data_conf[ "data_left" ]         = data_left;
  data_conf[ "data_right" ]        = data_right;
  data_conf[ "buffer_density" ]    = buffer_density;
  data_conf[ "alpha_centerpiece" ] = centerpiece_angle;

  qDebug() << "For rawDataID, eID: "        << rawDataID_t << eID
	   << ", Meniscus is -- "           << data_conf[ "meniscus" ]
	   << ", Data Left is -- "          << data_conf[ "data_left" ]
	   << ", Data Right is -- "         << data_conf[ "data_right" ]
	   << ", Buffer Density is -- "     << data_conf[ "buffer_density" ]
	   << ", Alpha Centerpiece is -- "  << data_conf[ "alpha_centerpiece" ];
  
  return data_conf;
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
 
   pb_load      ->setEnabled( true );
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
   triple_name_to_peak_curves_map . clear();
   triple_name_to_peak_to_parms_map . clear();
   triple_report_saved_map  . clear();
   triple_fitted_map        . clear();
   triple_peaks_defined_map . clear();

   meniscus_to_triple_name_map    . clear();
   data_left_to_triple_name_map   . clear();
   data_right_to_triple_name_map  . clear();
   buffDensity_to_triple_name_map . clear();
   sigma_to_triple_name_map       . clear();
   gradMW_to_triple_name_map      . clear();
   gradVbar_to_triple_name_map    . clear();
   gradC0_to_triple_name_map      . clear();
   
   variance_triple_order_map  . clear();
   triple_name_to_rmsd        . clear();
   triple_name_to_fit_parameters . clear();
   gauss_order_minVariance    .clear();
   sigma_val_minVariance      .clear();
   triple_name_to_peak_gauss_envelopes_map. clear();
   triple_name_to_total_area. clear();
   alpha_centerpiece        . clear();

   do_fit_for_triple        . clear();

   triple_name_to_total_area_uncorrected. clear();  

   xfit_data . clear();
   yfit_data . clear();
   triple_name_to_Cdata . clear();
       
   if ( us_buoyancy_auto_mode )
     {
       cb_triple->clear();
       pick->setEnabled( false );
       
       le_stretch          ->setText( "" ); 
       le_bottom	   ->setText( "" );
       le_bottom_calc      ->setText( "" );
       le_meniscus	   ->setText( "" );
       le_temperature      ->setText( "" );
       le_buffer_density   ->setText( "" );

       // le_vbar             ->setText( "" );
       // le_dens_0           ->setText( "" );
       // le_MW		      ->setText( "" );
       
       cb_peaks->disconnect();
       cb_peaks->clear();
       
       le_peakPosition   ->setText( "0.0" );
       le_peakDensity    ->setText( "0.0" );
       le_peakVbar       ->setText( "0.0" );
       le_peakGaussArea  ->setText( "0.0" );

       pgb_progress    ->reset();

       us_setReadOnly ( le_dens_0, true );
       us_setReadOnly ( le_vbar, true );
       us_setReadOnly ( le_MW, true );
       us_setReadOnly ( le_sigma, true );
       
       pb_view_reports  ->setEnabled( false );
       pb_delete_peak   ->setEnabled( false );
       pb_add_peak      ->setEnabled( false );

       ck_auto_fit      ->setEnabled( true );
     }
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

   QStringList triple_n_list = triple_n.split("/");
   QString wvl_n = triple_n_list[2].simplified();
   
   bool change_y_axis_title; 
   (wvl_n.length() == 3) ? change_y_axis_title = true : change_y_axis_title = false;
   QString y_axis_title = QString("Absorbance, ") + wvl_n + "nm";

   qDebug() << "Y-axis title -- " << y_axis_title;
   
   QString title_fit;
   double sigma = 0.015;
   
  // current scan is global
   current_scan = (int) scan_number;
   int    rsize = data.pointCount();
   int    ssize = data.scanCount();
   int    count = 0;
   QVector< double > rvec( rsize );
   QVector< double > vvec( rsize );
   double* r    = rvec.data();
   double* v    = vvec.data();

   qDebug() << "New triple, Before detachItems in plot() ";

   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   //data_plot->detachItems( QwtPlotItem::Rtti_PlotItem, true);
   //data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve, true );
   
   qDebug() << "New triple, After detachItems in plot() 1";
   v_line = NULL;
   qDebug() << "New triple, After detachItems in plot() 2";

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
     if( us_buoyancy_auto_mode && ii != ssize - 1 )
       continue;
     
     // clear WaveLengthScan array every time --
     // ensure last scan is written in the end to v_wavelength
     v_wavelength. clear();
     
      US_DataIO::Scan* s = &data.scanData[ ii ];

      QString arpm       = QString::number( s->rpm );

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

	 if ( temp_x >= data_left_to_triple_name_map[ triple_n ]
	      && temp_x <= data_right_to_triple_name_map[ triple_n ] )
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

      if ( !us_buoyancy_auto_mode )
	{
	  if (ii == current_scan - 1 )
	    { // set the temperature to the currently highlighted scan:
	      tmp_dpoint.temperature = s->temperature;
	      c->setPen( QPen( Qt::red ) );
	    }
	}
      else
	{
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
   if ( us_buoyancy_auto_mode && triple_fitted_map[ triple_n ] )
     {
       QwtPlotCurve* fitdata;
       fitdata = us_curve(data_plot, title_fit + "-fit");
       fitdata->setPen(QPen(Qt::cyan));
       double* xx_ffit = (double*)xfit_data[ triple_n ] .data();    // <- the only data set in xfit_data
       double* yy_ffit = (double*)yfit_data[ triple_n ] .data();
       fitdata->setSamples( xx_ffit, yy_ffit, xfit_data[ triple_n ].size() );

       double maxV_fit  = -1.0e99;
       double minV_fit  =  1.0e99;
       for ( int i=0; i< yfit_data.size(); i++ )
	 {
	   maxV_fit = max( maxV_fit, yfit_data[ triple_n ][ i ] );
	   minV_fit = min( minV_fit, yfit_data[ triple_n ][ i ] );
	 }

       double minV_final = min( minV_fit, minV);
       double maxV_final = max( maxV_fit, maxV);

       double padR = ( maxR - minR ) / 30.0;
       double padV = ( maxV_final - minV_final ) / 30.0;
       
       data_plot->setAxisScale( QwtPlot::yLeft  , minV_final - padV, maxV_final + padV );
       data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );

       data_plot->replot();

       //plot Cdata
       QwtPlotCurve* Cdata;
       Cdata = us_curve(data_plot, "C-data for " + triple_n );
       Cdata->setYAxis ( QwtPlot::yRight );
       Cdata->setPen(QPen(Qt::green));
       double* xx_C = (double*)xfit_data[ triple_n ] .data();    // <- the only data set in xfit_data
       double* yy_C = (double*)triple_name_to_Cdata[ triple_n ] .data();
       Cdata->setSamples( xx_C, yy_C, triple_name_to_Cdata[ triple_n ].size() );

       double maxV_C = -1.0e99;
       double minV_C =  1.0e99;
       for ( int i=0; i< triple_name_to_Cdata[triple_n].size(); i++ )
	 {
	   maxV_C = max( maxV_C, triple_name_to_Cdata[ triple_n ][ i ] );
	   minV_C = min( minV_C, triple_name_to_Cdata[ triple_n ][ i ] );
	 }

       data_plot->setAxisScale( QwtPlot::yRight , minV_C - padV, maxV_C + padV );
       data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );
       data_plot->setAxisTitle( QwtPlot::yRight, "Density Gradient" );

       data_plot->replot();
       
       //peak positions, vertical lines (if any) && Gaussians envelopes
       triple_name_to_peak_curves_map[ triple_n ]. clear();
       for (int ii=0; ii < triple_name_to_peaks_map[ triple_n ].size(); ++ii )
	 {
	   double rad = triple_name_to_peaks_map[ triple_n ][ ii ];

	   qDebug() << "Drawing line for triple, peak -- " << triple_n << rad;
	   draw_vline_auto( rad, QString::number(ii + 1) );
	 }

       //plot gauss envelops
       triple_name_to_peak_gauss_envelopes_map[ triple_n ]. clear();
       draw_gauss_envelope( triple_name_to_peak_to_parms_map[ triple_n ] );
     }

   
   if ( change_y_axis_title )
     data_plot->setAxisTitle( QwtPlot::yLeft,  y_axis_title );
        
   data_plot->replot();

   
   if ( us_buoyancy_auto_mode && !triple_fitted_map[ triple_n ] && do_fit_for_triple [ triple_n ] )
     {
       //do fit of the current scan, identify peak positions ///////////////////////////////////////////////////////////////////////
       qDebug() << "Size of v_wavelength array for triple : " << v_wavelength. last() . description  <<  v_wavelength. size();

       // //DEBUG
       // for( int i=0; i< v_wavelength .size(); i++)
       // 	 {
       // 	   WavelengthScan w_t = v_wavelength[ i ];
       // 	   for ( int j=0; j < w_t.v_readings.size(); j++ )
       // 	     {
       // 	       qDebug() << "Raw Data [SET "<< i+1 << " ]: X, Y -- "
       // 			<< w_t. v_readings[ j ]. lambda
       // 			<< w_t. v_readings[ j ]. od;
       // 		 }
       // 	 }
       // exit(1);
       /////////////////////////////////////////

       int totalOrders = 15;
       int order_init  = 10;
       int order_counter = 0;
       pgb_progress->reset();
       pgb_progress->setMaximum( 100 );

       //fill array of sigmas
       //double sigma_step = double (sigma / 10.0 );
       double sigma_step = 0.002;
       int total_steps   = 0;
       while ( (sigma - sigma_step* (int)(total_steps / 2.0 )) < 0.01 )           //set cap on minimum value
       	 --total_steps;

       double sigma_min = sigma - sigma_step* (double)(total_steps / 2.0 );
       double sigma_max = sigma + sigma_step* (double)(total_steps / 2.0 ) * 1.5; //max value can go a bit higher 
       
       QVector< double > sigma_vector; //= { 0.012, 0.015, 0.017, 0.02 };
       double sigma_value = sigma_min;
       while ( sigma_value <=  sigma_max )
	 {
	   sigma_vector. push_back( sigma_value );
	   sigma_value += sigma_step;
	 }
       
       int totalSigmas = sigma_vector.size();
       // //DEBUG
       // for ( int sigma_i = 0; sigma_i < totalSigmas; ++sigma_i )
       // 	 qDebug() << "Sigma vector -- " << sigma_vector[ sigma_i ];

       // return;
       // ////
       
       int sigma_counter = 0;
       int general_counter = 0;
       
       //do fit
       for ( int sigma_i = 0; sigma_i < totalSigmas; ++sigma_i )
	 {
	   current_sigma = sigma_vector[ sigma_i ];
	   ++sigma_counter;
	   
	   for ( int order_i = order_init; order_i < ( totalOrders + order_init ); ++order_i )
	     {
	       current_order = order_i;
	       
	       ++order_counter;
	       ++general_counter;

	       double progress = general_counter;
	       progress /= (totalOrders * totalSigmas);
	       progress *= 100;
	       pgb_progress->setValue( progress );
	       pgb_progress->setFormat( triple_n + ", Orders:    " + QString::number( int( progress ) )+"%");
	       
	       bool fitting_widget = false;
	       
	       unsigned int  order = current_order;
	       
	       unsigned int  parameters = order * 3 + v_wavelength.size();
	       //double * fitparameters = new double [parameters];
	       fitparameters = new double [parameters];       
	       for (int i=0; i<v_wavelength.size(); i++)
		 {
		   fitparameters[i] = 0.3;                                                  // Amplitude
		 }
	       double R_step = (maxR - minR)/(order+1); // create "order" peaks evenly distributed over the range
	       QString projectName = QString("");
	       
	       qDebug() << "Positions: min, max, step: " << minR << ", " << maxR << ", " << R_step;
	       
	       for (unsigned int i=0; i<order; i++)
		 {
		   fitparameters[v_wavelength.size() + (i * 3) ] = 1;                        // Addition to the amplitude
		   // spread out the peaks
		   fitparameters[v_wavelength.size() + (i * 3) + 1] = minR + R_step * long( i ) ;    // Position
		   fitparameters[v_wavelength.size() + (i * 3) + 2] = current_sigma;                 // Sigma: comes based on single Gauss fit of representative data (AVV) peak
		 }
	       
	       //call US_Extinctfitter
	       fitter = new US_ExtinctFitter(&v_wavelength, fitparameters, order, parameters,
					     projectName, &fitting_widget, true );
	       
	       connect( fitter, SIGNAL( get_yfit( QVector <QVector<double> > &, QVector <QVector<double> > & )), this, SLOT(process_yfit( QVector <QVector<double> > &, QVector <QVector<double> > & ) ) );
	       connect( fitter, SIGNAL( get_variance( double )), this, SLOT( process_variance( double ) ) );
	       
	       fitter->Fit();
	     }
	 }
       // END of Fit ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

       //save fit parameters
       triple_name_to_fit_parameters[ triple_n ] = fitparameters;
       
       triple_fitted_map[ triple_n ] = true;
     }
   


   if ( us_buoyancy_auto_mode && !triple_peaks_defined_map[ triple_n ] && do_fit_for_triple [ triple_n ] )
     {
       //variances for current triple && sigma && order used in fit
       double  minVariance = 1.0e99;
       
       QMap < double, QMap < int, double > > curr_triple_sigma_order_vars = variance_triple_order_map[ triple_n ];
       QMap < double, QMap < int, double > >::iterator ss;
       for ( ss =  curr_triple_sigma_order_vars.begin(); ss !=  curr_triple_sigma_order_vars.end(); ++ss )
	 {
	   QMap < int, double > curr_triple_order_vars = ss.value();
	   QMap < int, double >::iterator mm;
	   
	   for ( mm =  curr_triple_order_vars.begin(); mm !=  curr_triple_order_vars.end(); ++mm )
	     {
	       if ( mm.value() == 0 )
		 continue;
	       
	       qDebug() << "Triple " << triple_n << ": sigma, order, variance -- " << ss.key() << mm.key() << mm.value();
	       
	       minVariance = min( minVariance, mm.value() );
	     }
	 }
       
       gauss_order_minVariance[ triple_n ]   = 0;
       sigma_val_minVariance  [ triple_n ]   = 0;
       for ( ss =  curr_triple_sigma_order_vars.begin(); ss !=  curr_triple_sigma_order_vars.end(); ++ss )
	 {
	   QMap < int, double > curr_triple_order_vars = ss.value();
	   QMap < int, double >::iterator mm;
	   for ( mm =  curr_triple_order_vars.begin(); mm !=  curr_triple_order_vars.end(); ++mm )
	     {
	       if ( mm.value() == minVariance )
		 {
		   sigma_val_minVariance  [ triple_n ] = ss.key();
		   gauss_order_minVariance[ triple_n ] = mm.key();
		   break;
		 }
	     }
	 }
       qDebug() << "Smallest variance " << minVariance << " for triple " << triple_n
		<< ", for Sigma val -- " <<  sigma_val_minVariance[ triple_n ]
		<< ", for Gauss order -- " << gauss_order_minVariance [ triple_n ];

       int g_order    = gauss_order_minVariance [ triple_n ];
       double g_sigma = sigma_val_minVariance[ triple_n ]; 

       //finally, select the FIT data for the smallest variance to be plotted alongside raw data:
       xfit_data[ triple_n ] = xfit_data_all_orders[ triple_n ][ g_sigma ][ g_order ];
       yfit_data[ triple_n ] = yfit_data_all_orders[ triple_n ][ g_sigma ][ g_order ];

       // Now that we have best fit curves, Identify peak positions:
       triple_name_to_rmsd[ triple_n ] = compute_rmsd ( triple_n );
       QVector <double> peak_poss_auto = identify_peaks( triple_n, g_sigma );

       qDebug() << "[AUTO] peaks for triple: " << triple_n << " are -- " << peak_poss_auto;

       // //test [ HARD coded for now... ]
       // QVector <double> peak_poss;
       // if ( current_triple == 0 )
       // 	 peak_poss = { 6.5, 6.63, 6.72 };
       // else
       // 	 peak_poss = { 6.51, 6.622, 6.722, 6.8 };
       // //triple_name_to_peaks_map[ triple_n ] = peak_poss;

       triple_name_to_peaks_map[ triple_n ] = peak_poss_auto;
       triple_peaks_defined_map[ triple_n ] = true;

       QMap< QString, double > tot_areas = calc_total_area( triple_n );
       triple_name_to_total_area            [ triple_n ] = tot_areas[ "corrected" ];
       triple_name_to_total_area_uncorrected[ triple_n ] = tot_areas[ "uncorrected" ];

       qDebug() << "Total area for triple " << triple_n << " is: " << triple_name_to_total_area[ triple_n ];
     }
   
   update_fields();

   //Save & all peak reports (for current triple)
   if (  us_buoyancy_auto_mode && !triple_report_saved_map[ triple_n  ] && do_fit_for_triple [ triple_n ] )
     {
       save_auto( triple_n );
     }
}


double US_Buoyancy::compute_rmsd ( QString triple_n )
{
  double rmsd = 0;
  double mean = 0;

  for (int i=0; i < xfit_data[ triple_n ].size(); i++ )
    {
      mean += pow( ( v_wavelength[ 0 ].v_readings[ i ].od - yfit_data[ triple_n ][ i ] ), 2);
    }

  mean /= (double) xfit_data[ triple_n ].size();

  rmsd = sqrt( mean );

  return rmsd;
}

QVector< double > US_Buoyancy::identify_peaks( QString triple_n, double sigma_p )
{
  //double* fit_parms = triple_name_to_fit_parameters[ triple_n ];
  QVector< double > peaks;
  double stretch_f = 2.5;
  double sigma = sigma_p * stretch_f;

  int last_index = xfit_data[ triple_n ].size() - 1;
  
  for (int i=0; i < xfit_data[ triple_n ].size(); i++ )
    {
      double curr_y = yfit_data[ triple_n ][i];
      double curr_x = xfit_data[ triple_n ][i];
      
      //maye skip sigma regions on both ends of the data?
      if( (curr_x - sigma) < xfit_data[ triple_n ][0]  ||
       	  (curr_x + sigma) > xfit_data[ triple_n ][ last_index ] )
       	continue;
      
      double left_x = ( curr_x - sigma ) > xfit_data[ triple_n ][0] ?
	( curr_x - sigma ) : xfit_data[ triple_n ][0]; 
      int left_i = index_of_data( xfit_data[ triple_n ], left_x );

      double right_x = ( curr_x + sigma ) <  xfit_data[ triple_n ][ last_index ] ?
	( curr_x + sigma ) : xfit_data[ triple_n ][ last_index ]; 
      int right_i = index_of_data( xfit_data[ triple_n ], right_x );

      //qDebug() << "Proceeding with point x, y: " << curr_x << curr_y << "; pm sigma -- " << left_x << right_x ;
      
      if ( isMaximum_y( yfit_data[ triple_n ], i, left_i, right_i, triple_n ) )
	peaks .push_back( curr_x );
    }
  
  return peaks;
}

bool US_Buoyancy::isMaximum_y( QVector<double> ydata, int curr_i, int left_i, int right_i, QString triple_n )
{
  bool isPeak = true;
  double curr_y = ydata[ curr_i ];
  
  for (int i = left_i; i < right_i; i++ )
    {
      if ( i == curr_i )
	continue;
      
      if ( curr_y < ydata[ i ] )
	{
	  isPeak = false;
	  break;

	  return isPeak;
	}
    }

  //maybe check for height estimate vs. OD threshold limit (to be considered a peak)?
  if ( isPeak )
    {
      double mean         = 0;
      double minimum      = 10000;
      //double od_threshold = 0.005;    // maybe should be calculated from Gauss's height at [ x /pm sigma ]?
      double od_threshold = 3.5 * triple_name_to_rmsd[ triple_n ];

      qDebug() << "RMSD for triple: " << triple_n << ", " <<  triple_name_to_rmsd[ triple_n ]; 
      
      for (int i = left_i; i < right_i; i++ )
  	{
  	  mean += ydata[ i ];

  	  minimum = min( minimum,  ydata[ i ]);
  	}

      if ( ( curr_y - minimum ) < od_threshold )
  	isPeak = false;
    }
  
  return isPeak;
}


int US_Buoyancy::index_of_data( QVector<double> xdata, double val )
{
  double diff_val = 10000;
  int index_x = 0;
  
  for (int i=0; i< xdata.size(); i++ )
    {
      if ( qAbs( val - xdata[ i ] ) < diff_val )
	{
	  diff_val = qAbs( val - xdata[ i ] );
	  index_x = i;
	}
    }

  return index_x;
}
  
void US_Buoyancy::process_yfit(QVector <QVector<double> > &x, QVector <QVector<double> > &y)
{
  QString triple_n = cb_triple->itemText( current_triple );

  // xfit_data_all_orders[ triple_n ][ current_order ] = x.last();
  // yfit_data_all_orders[ triple_n ][ current_order ] = y.last();
  
  xfit_data_all_orders[ triple_n ][ current_sigma ][ current_order ] = x.last();
  yfit_data_all_orders[ triple_n ][ current_sigma ][ current_order ] = y.last();

}
	   
void US_Buoyancy::process_variance( double variance )
{
  QString triple_n = cb_triple->itemText( current_triple );
  
  //variance_triple_order_map[ triple_n ][ current_order ] = variance;

  variance_triple_order_map[ triple_n ][ current_sigma ][ current_order ] = variance;
  
  qDebug() << "For triple: " << triple_n << ", sigma " << current_sigma << ", order " << current_order << ": variance: " << variance; 
  
}

//delete peak
void US_Buoyancy::delete_peak( void )
{
  int peak_index   = cb_peaks -> currentIndex();
  QString triple_n = cb_triple->itemText( current_triple );
  QString peak_n   = cb_peaks ->itemText( peak_index );

  qDebug() << "Current peak index -- " << peak_index;
  
  triple_name_to_peaks_map[ triple_n ]         .remove( peak_index );
  triple_name_to_peak_to_parms_map[ triple_n ] .remove( peak_n ); 
  
  new_triple( current_triple );

  qDebug() << "Delete peak: triple_name_to_peaks_map[ triple ].size() -- " << triple_name_to_peaks_map[ triple_n ].size();
  
  //re-save all reports
  dpoint. clear();
  for ( int i=0; i< cb_triple->count(); i++ )
    {
      save_auto ( cb_triple->itemText( i ) ); 
    }
  // for (int i=0; i<dpoint.size(); i++)
  //  {
  //    if ( dpoint[ i ].triple == triple_n &&
  // 	  dpoint[ i ].name == peak_n )
  //      {
  // 	 qDebug() << "Removing dpoint entry for triple, peak -- " << dpoint[ i ].triple << dpoint[ i ].name;
  // 	 dpoint. remove( i );
  // 	 break;
  //      }
  //  }
  
}

//add new peak manually
void US_Buoyancy::add_peak( void )
{
  QMessageBox::information( this,
			    tr( "Manual Peak Selection" ),
			    tr( "To manually select additional peak\n"
				"please use CNTR+mouse to mark peak position." ) );

  pick -> setEnabled( true ); 
  connect( pick, SIGNAL( cMouseUp     ( const QwtDoublePoint& ) ),
		 SLOT  ( mouse_peak   ( const QwtDoublePoint& ) ) );

}

// Add additional peak with the mouse click
void US_Buoyancy::mouse_peak( const QwtDoublePoint& p )
{
  QString triple_n = cb_triple->itemText( current_triple );
  triple_name_to_peaks_map[ triple_n ]. push_back ( p.x() );

  qDebug() << "Adding new peak, new size() -- " << triple_name_to_peaks_map[ triple_n ].size();

  std::sort(triple_name_to_peaks_map[ triple_n ].begin(), triple_name_to_peaks_map[ triple_n ].end());
  calc_points_auto( triple_n );

  qDebug() << "add peak 1";

  new_triple( current_triple );

  qDebug() << "add peak 2";
  
  //re-save all reports
  dpoint. clear();
  for ( int i=0; i< cb_triple->count(); i++ )
    {
      save_auto ( cb_triple->itemText( i ) ); 
    }

  //disable pick again:
  pick -> disconnect();
  pick -> setEnabled( false ); 
}  

//Draw Gaussian envelope
void US_Buoyancy::draw_gauss_envelope  ( QMap < QString, QStringList > peak_p )
{
  QString triple_n = cb_triple->itemText( current_triple );
  
  
#if QT_VERSION < 0x050000
  QwtScaleDiv* y_axis = data_plot->axisScaleDiv( QwtPlot::yLeft );
#else
  QwtScaleDiv* y_axis = (QwtScaleDiv*)&(data_plot->axisScaleDiv( QwtPlot::yLeft ));
#endif
  
  double padding = ( y_axis->upperBound() - y_axis->lowerBound() ) / 30.0;

  qDebug() << "Gauss envelope build for triple: " << triple_n;
  
  //peak Gauss params
  QMap < QString, QStringList >::iterator ss;
  // //DEBUG
  // for ( ss =  peak_p.begin(); ss !=  peak_p.end(); ++ss )
  //   {
  //     qDebug() << "Peaks, values --  " << ss.key() << ss.value(); 
  //   }
  // //////////////

  int peak_count = 0;
  for ( ss =  peak_p.begin(); ss !=  peak_p.end(); ++ss )
    {
      QStringList pp = ss.value();

      double pos_x  = pp[ 2 ].toDouble();
      double sigma  = pp[ 3 ].toDouble();         //<---- Alternatively, sigma is determined differently
      double height = pp[ 4 ].toDouble();

      qDebug() << "Peak: " << ss.key() << ", pos_x, sigma, height -- "
	       << pos_x << sigma << height;

      QVector <double> gaussian_x;
      QVector <double> gaussian_y;
      
      for ( int ii=0; ii < xfit_data[ triple_n ].size(); ++ii )
	{
	  double x_val = xfit_data[ triple_n ][ ii ];
	  double y_val = height * exp( - ( pow(( x_val - pos_x), 2 )) / ( 2 * pow( sigma, 2)) );

	  if ( y_val >= triple_name_to_rmsd [ triple_n ] )
	    {
	      gaussian_x. push_back ( x_val ); 
	      gaussian_y. push_back ( y_val ); 
	    }
	}

      qDebug() << "Envelope built for peak: " << ss.key();

      ++peak_count;
      QString gauss_line_name = "Gauss Envelope" + QString::number( peak_count );
      
      QwtPlotCurve* peak_envelope;
      peak_envelope = us_curve(data_plot, gauss_line_name );
      
      double* yy = (double*)gaussian_y.data();
      double* xx = (double*)gaussian_x.data();
      peak_envelope ->setSamples( xx, yy, gaussian_x.size() );
      
      QPen pen = QPen( QBrush( Qt::yellow ), 1 );//, 1.0, Qt::DotLine );
      peak_envelope->setPen( pen );
      
      triple_name_to_peak_gauss_envelopes_map[ triple_n ].push_back( peak_envelope ) ;
      
    }
}

// Draw a vertical pick line
void US_Buoyancy::draw_vline_auto( double radius, QString vline_number )
{
   QwtPlotCurve* v_line_peak;
      
   double r[ 2 ];

   r[ 0 ] = radius;
   r[ 1 ] = radius;

#if QT_VERSION < 0x050000
   QwtScaleDiv* y_axis = data_plot->axisScaleDiv( QwtPlot::yLeft );
#else
   QwtScaleDiv* y_axis = (QwtScaleDiv*)&(data_plot->axisScaleDiv( QwtPlot::yLeft ));
#endif

   double padding = ( y_axis->upperBound() - y_axis->lowerBound() ) / 30.0;

   // qDebug() << "Drawing vert. line: upperY, lowerY, padding -- "
   // 	    << y_axis->upperBound() << y_axis->lowerBound() << padding;
   
   double v[ 2 ];
   v [ 0 ] = y_axis->upperBound() - padding;
   v [ 1 ] = y_axis->lowerBound() + padding;
   
   QString vline_name = "V-Line-" + vline_number; 
   
   v_line_peak = us_curve( data_plot, vline_name );
   v_line_peak ->setSamples( r, v, 2 );

   QPen pen = QPen( QBrush( Qt::yellow ), 1.0, Qt::DotLine );
   v_line_peak->setPen( pen );

   QString triple_n = cb_triple->itemText( current_triple );
   triple_name_to_peak_curves_map[ triple_n ].push_back( v_line_peak ) ; 
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
      tmp_dpoint.name          = curr_peak_name;
      tmp_dpoint.peakPosition  = curr_peak_parms_list[2 ].toDouble();
      tmp_dpoint.peakDensity   = curr_peak_parms_list[0 ].toDouble();
      tmp_dpoint.peakVbar      = curr_peak_parms_list[1 ].toDouble();
      tmp_dpoint.peakGaussArea = curr_peak_parms_list[5 ].toDouble();
      tmp_dpoint.percentTotal  = 100.0*( curr_peak_parms_list[5 ].toDouble() / curr_peak_parms_list[6 ].toDouble());

      //TEMPORARY - for comparison; will be removed later
      tmp_dpoint.percentTotal_uncorrected  = 100.0*( curr_peak_parms_list[7 ].toDouble() / curr_peak_parms_list[8 ].toDouble());

      qDebug() << "Saving_auto: current_triple index -- " << current_triple;
     
      //rest is the same for all peaks withtin a triple ?
      tmp_dpoint.triple        = triple_n;
      tmp_dpoint.stretch       = current_stretch;
      tmp_dpoint.centerpiece   = simparams[ current_triple ].bottom_position;
      tmp_dpoint.bottom        = le_bottom_calc->text().toDouble();
      tmp_dpoint.speed         = cb_rpms->currentText().toDouble();
      
      // tmp_dpoint.bufferDensity = le_buffer_density->text().toDouble();
      // tmp_dpoint.meniscus      = le_meniscus->text().toDouble();
      // tmp_dpoint.gradientMW    = le_MW->text().toDouble();
      // tmp_dpoint.gradientVbar  = le_vbar->text().toDouble();
      // tmp_dpoint.gradientC0    = le_dens_0->text().toDouble();

      tmp_dpoint.bufferDensity = buffDensity_to_triple_name_map[ triple_n ];
      tmp_dpoint.sigma         = sigma_to_triple_name_map      [ triple_n ];
      tmp_dpoint.meniscus      = meniscus_to_triple_name_map   [ triple_n ];
      tmp_dpoint.gradientMW    = gradMW_to_triple_name_map     [ triple_n ];
      tmp_dpoint.gradientVbar  = gradVbar_to_triple_name_map   [ triple_n ];
      tmp_dpoint.gradientC0    = gradC0_to_triple_name_map     [ triple_n ];

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

   te->e->append("Measure of the Total OD per Triple:\n");
   QMap < QString, double >::iterator ta;
   for ( ta = triple_name_to_total_area.begin(); ta !=  triple_name_to_total_area.end(); ++ta )
     {
       qDebug() << "triple, value -- " << ta.key() << ta.value();
       te->e->append("Triple: " + ta.key() + ";\t Total OD, Volumetric Measure (corrected for centerpiece shape): " +
		     QString::number(ta.value()) );
       te->e->append("Triple: " + ta.key() + ";\t Total OD, Area Measure (un-corrected for centerpiece shape): " +
		     QString::number(triple_name_to_total_area_uncorrected[ ta.key() ] ) );
       te->e->append("\n");
     }
   te->e->append("\n\n");
       
   for (int i=0; i<dpoint.size(); i++)
   {
      te->e->append("Peak " + str.setNum( i+1 ) + " (" + dpoint[i].name +
      " from experiment \"" + dpoint[i].dataset + "\"):" );
      te->e->append( "Sample location:\t" + dpoint[i].triple );
      te->e->append( "Sample description:\t" + dpoint[i].description );
      te->e->append( "Rotor speed:\t" + str.setNum( dpoint[i].speed ) + " rpm, (Rotor stretch: "
      + str2.setNum( dpoint[i].stretch) + " cm)" );

      te->e->append( "Peak position:\t"      + str.setNum( dpoint[i].peakPosition )  + " cm");
      te->e->append( "Peak density:\t"       + str.setNum( dpoint[i].peakDensity )   + " g/ml");
      te->e->append( "Peak vbar:\t\t"        + str.setNum( dpoint[i].peakVbar )      + " ml/g");
      te->e->append( "Peak Gauss area (% of total):\t\t" +  QString::number(dpoint[i].percentTotal, 'f', 2) );
      te->e->append( "Peak Gauss area UNCORRECTED (% of total):\t" +  QString::number(dpoint[i].percentTotal_uncorrected, 'f', 2) );
      te->e->append( "Peak sigma value:\t\t" + str.setNum( dpoint[i].sigma )      + " cm");

      te->e->append( "Buffer density:\t"     + str.setNum( dpoint[i].bufferDensity ) + " g/ml");
      te->e->append( "Meniscus position:\t"  + str.setNum( dpoint[i].meniscus ) + " cm");
      te->e->append( "Bottom of cell:\t"     + str.setNum( dpoint[i].bottom ) +
      " cm (Centerpiece bottom at rest: "    + str2.setNum( dpoint[i].centerpiece ) + " cm)" );
      te->e->append( "Temperature:\t"        + str.setNum( dpoint[i].temperature ) +  DEGC );
      te->e->append( "Gradient-forming\nmaterial details:");
      te->e->append( "Molecular weight:\t"   + str.setNum( dpoint[i].gradientMW ) + " g/mol" );
      te->e->append( "Loading density:\t"    + str.setNum( dpoint[i].gradientC0 ) + " g/mol" );
      te->e->append( "vbar:\t\t"             + str.setNum( dpoint[i].gradientVbar ) + " ml/g" );
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
     if ( !us_buoyancy_auto_mode )
       str.setNum( meniscus[current_triple] + current_stretch );
     else
       {
	 QString triple_n = cb_triple->itemText( current_triple );
	 str.setNum( meniscus_to_triple_name_map[ triple_n ]);
       }
     
     le_meniscus->setText( str );
   }
   plot_scan( current_scan );
}

//When sigma updated
void US_Buoyancy::update_for_sigma ( void )
{
  QString triple_n = cb_triple->itemText( current_triple );
  sigma_to_triple_name_map[ triple_n ] = (double) le_sigma->text().toDouble();

  calc_points_auto( triple_n );
  
  new_triple( current_triple );

  //re-save all reports
  dpoint. clear();
  for ( int i=0; i< cb_triple->count(); i++ )
    {
      save_auto ( cb_triple->itemText( i ) ); 
    }
}

//When MW updated
void US_Buoyancy::update_for_MW ( void )
{
  QString triple_n = cb_triple->itemText( current_triple );
  gradMW_to_triple_name_map[ triple_n ] = (double) le_MW->text().toDouble();

  update_fields();
  
  //re-save all reports
  dpoint. clear();
  for ( int i=0; i< cb_triple->count(); i++ )
    {
      save_auto ( cb_triple->itemText( i ) ); 
    }
}

//When Vbar updated
void US_Buoyancy::update_for_vbar ( void )
{
  QString triple_n = cb_triple->itemText( current_triple );
  gradVbar_to_triple_name_map[ triple_n ] = (double) le_vbar->text().toDouble();

  update_fields();
  
  //re-save all reports
  dpoint. clear();
  for ( int i=0; i< cb_triple->count(); i++ )
    {
      save_auto ( cb_triple->itemText( i ) ); 
    }
}

//When Density0 updated
void US_Buoyancy::update_for_dens_0 ( void )
{
  QString triple_n = cb_triple->itemText( current_triple );
  gradC0_to_triple_name_map[ triple_n ] = (double) le_dens_0->text().toDouble();

  update_fields();
  
  //re-save all reports
  dpoint. clear();
  for ( int i=0; i< cb_triple->count(); i++ )
    {
      save_auto ( cb_triple->itemText( i ) ); 
    }
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

void US_Buoyancy::closeEvent(QCloseEvent *event)
{
    emit widgetClosed();
    event->accept();
}
