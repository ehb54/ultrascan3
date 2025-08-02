//! \file us_eqmodel_control.cpp

#include "us_eqmodel_control.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"
#include "us_math2.h"
#include "us_analyte_gui.h"
#include "us_buffer_gui.h"
#include "us_db2.h"

// Main constructor with references to parameters from main GlobalEquil class
US_EqModelControl::US_EqModelControl(
      QVector< EqScanFit >&   a_scanfits,
      EqRunFit&               a_runfit,
      US_DataIO::EditedData*  a_edata,
      int                     a_modelx,
      QStringList             a_models,
      bool&                   a_mWidget,
      int&                    a_selscan )
 : US_WidgetsDialog( 0, 0 ),
   scanfits   ( a_scanfits ),
   runfit     ( a_runfit ),
   edata      ( a_edata ),
   modelx     ( a_modelx ),
   models     ( a_models ),
   mWidget    ( a_mWidget ),
   selscan    ( a_selscan )
{
   setAttribute  ( Qt::WA_DeleteOnClose );
   setWindowTitle( tr( "Equilibrium Model Control Window" ) );
   setPalette    ( US_GuiSettings::frameColor() );
   setMinimumSize( 200, 100 );

   // Main layout
   QBoxLayout* main  = new QVBoxLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Two primary layouts
   QGridLayout* globalLayout = new QGridLayout;
   QGridLayout* localLayout  = new QGridLayout;

   // Global layout
   lb_gbanner              = us_banner( models[ modelx ] );
   QLabel*      lb_grunpar = us_label(
         tr( "Global Run Parameters for Component: " ) );
                ct_grunpar = us_counter( 2, 1, 100, 1 );
   QLabel*      lb_param   = us_label( tr( "Parameter:" ) );
   QLabel*      lb_guess   = us_label( tr( "Initial Guess:" ) );
   QLabel*      lb_bounds  = us_label( tr( "+/- Bounds:" ) );
   QLabel*      lb_units   = us_label( tr( "Units" ) );
   QLabel*      lb_float   = us_label( tr( "Float:" ) );
   QLabel*      lb_lock    = us_label( tr( "Lock:" ) );
   QLabel*      lb_bound   = us_label( tr( "Bound:" ) );
                lb_molecwt = us_label( tr( "Molecular Weight" ) + " (1):" );
                le_mwguess = us_lineedit();
                le_mwbound = us_lineedit();
   QLabel*      lb_mwunits = us_label( tr( "Dalton" ) );
   QGridLayout* lo_mwfloat = us_radiobutton( "F", rb_mwfloat, true  );
   QGridLayout* lo_mwlock  = us_radiobutton( "L", rb_mwlock,  false );
   QHBoxLayout* lo_mwflbox = radiobox( lo_mwfloat, rb_mwfloat,
                                       lo_mwlock,  rb_mwlock );
   QLayout*     lo_mwbound = us_checkbox( "B", ck_mwbound, false );
                pb_vbar20  = us_pushbutton( tr( "Vbar, 20" ) + DEGC + " (1):" );
                le_vbguess = us_lineedit();
                le_vbbound = us_lineedit();
   QLabel*      lb_vbunits = us_label( tr( "ccm/g" ) );
   QGridLayout* lo_vbfloat = us_radiobutton( "F", rb_vbfloat, true  );
   QGridLayout* lo_vblock  = us_radiobutton( "L", rb_vblock,  false );
   QHBoxLayout* lo_vbflbox = radiobox( lo_vbfloat, rb_vbfloat,
                                       lo_vblock,  rb_vblock );
   QLayout*     lo_vbbound = us_checkbox( "B", ck_vbbound, false );
                pb_lnasc1  = us_pushbutton( tr( "ln(Assoc.Const.1):" ) );
                le_l1guess = us_lineedit();
                le_l1bound = us_lineedit();
   QLabel*      lb_l1units = us_label( tr( "" ) );
   QGridLayout* lo_l1float = us_radiobutton( "F", rb_l1float, true  );
   QGridLayout* lo_l1lock  = us_radiobutton( "L", rb_l1lock,  false );
   QHBoxLayout* lo_l1flbox = radiobox( lo_l1float, rb_l1float,
                                       lo_l1lock,  rb_l1lock );
   QLayout*     lo_l1bound = us_checkbox( "B", ck_l1bound, false );
                pb_lnasc2  = us_pushbutton( tr( "ln(Assoc.Const.2):" ) );
                le_l2guess = us_lineedit();
                le_l2bound = us_lineedit();
   QLabel*      lb_l2units = us_label( tr( "" ) );
   QGridLayout* lo_l2float = us_radiobutton( "F", rb_l2float, true  );
   QGridLayout* lo_l2lock  = us_radiobutton( "L", rb_l2lock,  false );
   QHBoxLayout* lo_l2flbox = radiobox( lo_l2float, rb_l2float,
                                       lo_l2lock,  rb_l2lock );
   QLayout*     lo_l2bound = us_checkbox( "B", ck_l2bound, false );
                pb_lnasc3  = us_pushbutton( tr( "ln(Assoc.Const.3):" ) );
                le_l3guess = us_lineedit();
                le_l3bound = us_lineedit();
   QLabel*      lb_l3units = us_label( tr( "" ) );
   QGridLayout* lo_l3float = us_radiobutton( "F", rb_l3float, true  );
   QGridLayout* lo_l3lock  = us_radiobutton( "L", rb_l3lock,  false );
   QHBoxLayout* lo_l3flbox = radiobox( lo_l3float, rb_l3float,
                                       lo_l3lock,  rb_l3lock );
   QLayout*     lo_l3bound = us_checkbox( "B", ck_l3bound, false );
                pb_lnasc4  = us_pushbutton( tr( "ln(Assoc.Const.4):" ) );
                le_l4guess = us_lineedit();
                le_l4bound = us_lineedit();
   QLabel*      lb_l4units = us_label( tr( "" ) );
   QGridLayout* lo_l4float = us_radiobutton( "F", rb_l4float, true  );
   QGridLayout* lo_l4lock  = us_radiobutton( "L", rb_l4lock,  false );
   QHBoxLayout* lo_l4flbox = radiobox( lo_l4float, rb_l4float,
                                       lo_l4lock,  rb_l4lock );
   QLayout*     lo_l4bound = us_checkbox( "B", ck_l4bound, false );

   int row = 0;
   globalLayout->addWidget( lb_gbanner, row++, 0, 1, 10 );
   globalLayout->addWidget( lb_grunpar, row,   0, 1,  7 );
   globalLayout->addWidget( ct_grunpar, row++, 7, 1,  3 );
   globalLayout->addWidget( lb_param,   row,   0, 1,  2 );
   globalLayout->addWidget( lb_guess,   row,   2, 1,  2 );
   globalLayout->addWidget( lb_bounds,  row,   4, 1,  2 );
   globalLayout->addWidget( lb_units,   row,   6, 1,  1 );
   globalLayout->addWidget( lb_float,   row,   7, 1,  1 );
   globalLayout->addWidget( lb_lock,    row,   8, 1,  1 );
   globalLayout->addWidget( lb_bound,   row++, 9, 1,  1 );
   globalLayout->addWidget( lb_molecwt, row,   0, 1,  2 );
   globalLayout->addWidget( le_mwguess, row,   2, 1,  2 );
   globalLayout->addWidget( le_mwbound, row,   4, 1,  2 );
   globalLayout->addWidget( lb_mwunits, row,   6, 1,  1 );
   globalLayout->addLayout( lo_mwflbox, row,   7, 1,  2 );
   globalLayout->addLayout( lo_mwbound, row++, 9, 1,  1 );
   globalLayout->addWidget( pb_vbar20,  row,   0, 1,  2 );
   globalLayout->addWidget( le_vbguess, row,   2, 1,  2 );
   globalLayout->addWidget( le_vbbound, row,   4, 1,  2 );
   globalLayout->addWidget( lb_vbunits, row,   6, 1,  1 );
   globalLayout->addLayout( lo_vbflbox, row,   7, 1,  2 );
   globalLayout->addLayout( lo_vbbound, row++, 9, 1,  1 );
   globalLayout->addWidget( pb_lnasc1,  row,   0, 1,  2 );
   globalLayout->addWidget( le_l1guess, row,   2, 1,  2 );
   globalLayout->addWidget( le_l1bound, row,   4, 1,  2 );
   globalLayout->addWidget( lb_l1units, row,   6, 1,  1 );
   globalLayout->addLayout( lo_l1flbox, row,   7, 1,  2 );
   globalLayout->addLayout( lo_l1bound, row++, 9, 1,  1 );
   globalLayout->addWidget( pb_lnasc2,  row,   0, 1,  2 );
   globalLayout->addWidget( le_l2guess, row,   2, 1,  2 );
   globalLayout->addWidget( le_l2bound, row,   4, 1,  2 );
   globalLayout->addWidget( lb_l2units, row,   6, 1,  1 );
   globalLayout->addLayout( lo_l2flbox, row,   7, 1,  2 );
   globalLayout->addLayout( lo_l2bound, row++, 9, 1,  1 );
   globalLayout->addWidget( pb_lnasc3,  row,   0, 1,  2 );
   globalLayout->addWidget( le_l3guess, row,   2, 1,  2 );
   globalLayout->addWidget( le_l3bound, row,   4, 1,  2 );
   globalLayout->addWidget( lb_l3units, row,   6, 1,  1 );
   globalLayout->addLayout( lo_l3flbox, row,   7, 1,  2 );
   globalLayout->addLayout( lo_l3bound, row++, 9, 1,  1 );
   globalLayout->addWidget( pb_lnasc4,  row,   0, 1,  2 );
   globalLayout->addWidget( le_l4guess, row,   2, 1,  2 );
   globalLayout->addWidget( le_l4bound, row,   4, 1,  2 );
   globalLayout->addWidget( lb_l4units, row,   6, 1,  1 );
   globalLayout->addLayout( lo_l4flbox, row,   7, 1,  2 );
   globalLayout->addLayout( lo_l4bound, row++, 9, 1,  1 );

   pb_lnasc1 ->setEnabled( false );
   pb_lnasc2 ->setEnabled( false );
   pb_lnasc3 ->setEnabled( false );
   pb_lnasc4 ->setEnabled( false );
   ck_mwbound->setEnabled( false );
   ck_vbbound->setEnabled( false );
   ck_l1bound->setEnabled( false );
   ck_l2bound->setEnabled( false );
   ck_l3bound->setEnabled( false );
   ck_l4bound->setEnabled( false );
   le_mwbound->setEnabled( false );
   le_vbbound->setEnabled( false );
   le_l1bound->setEnabled( false );
   le_l2bound->setEnabled( false );
   le_l3bound->setEnabled( false );
   le_l4bound->setEnabled( false );
   us_setReadOnly( le_mwbound, true );
   us_setReadOnly( le_vbbound, true );
   us_setReadOnly( le_l1bound, true );
   us_setReadOnly( le_l2bound, true );
   us_setReadOnly( le_l3bound, true );
   us_setReadOnly( le_l4bound, true );

   ct_grunpar->setRange( 1, runfit.nbr_comps );
   ct_grunpar->setSingleStep( 1 );
   ct_grunpar->setValue( 1 );

   connect( ct_grunpar, SIGNAL( valueChanged( double ) ),
            this,  SLOT( global_comp_changed( double ) ) );
   connect( pb_vbar20,  SIGNAL( clicked()       ),
            this,       SLOT(   set_vbar()      ) );
   connect( rb_mwfloat, SIGNAL( toggled( bool ) ),
            this,       SLOT(   update_floats() ) );
   connect( rb_vbfloat, SIGNAL( toggled( bool ) ),
            this,       SLOT(   update_floats() ) );
   connect( rb_l1float, SIGNAL( toggled( bool ) ),
            this,       SLOT(   update_floats() ) );
   connect( rb_l2float, SIGNAL( toggled( bool ) ),
            this,       SLOT(   update_floats() ) );
   connect( rb_l3float, SIGNAL( toggled( bool ) ),
            this,       SLOT(   update_floats() ) );
   connect( rb_l4float, SIGNAL( toggled( bool ) ),
            this,       SLOT(   update_floats() ) );

   // Local layout
   lb_lbanner           = us_banner(
         tr( "Local Scan Parameters for Scan %1" ).arg( selscan ) );
   QLabel*      lb_cdescr  = us_label( tr( "Cell Description:" ) );
                le_cdescr  = us_lineedit( "", 0, true );
   QLabel*      lb_runid   = us_label( tr( "Run ID:" ) );
                le_runid   = us_lineedit( "", 0, true );
   QLabel*      lb_tempera = us_label( tr( "Temperature (" ) + DEGC + "):" );
                le_tempera = us_lineedit( "", 0, true );
   QLabel*      lb_speed   = us_label( tr( "Rotor Speed (rpm)" ) );
                le_speed   = us_lineedit( "", 0, true );
   QLabel*      lb_wavelen = us_label( tr( "Wavelength (nm)" ) );
                le_wavelen = us_lineedit();
   QLabel*      lb_pathlen = us_label( tr( "Cell Pathlength (cm)" ) );
                le_pathlen = us_lineedit();
                pb_plenapp = us_pushbutton( tr( "Apply to:" ) );
   QLabel*      lb_plenscn = us_label( tr( "Scan(s)" ) );
                le_plenscn = us_lineedit();
   QLabel*      lb_baseln  = us_label( tr( "Baseline:" ) );
                le_blguess = us_lineedit();
                le_blbound = us_lineedit();
   QLabel*      lb_blunits = us_label( tr( "OD" ) );
   QGridLayout* lo_blfloat = us_radiobutton( "F", rb_blfloat, true  );
   QGridLayout* lo_bllock  = us_radiobutton( "L", rb_bllock,  false );
   QHBoxLayout* lo_blflbox = radiobox( lo_blfloat, rb_blfloat,
                                       lo_bllock,  rb_bllock );
   QLayout*     lo_blbound = us_checkbox( "B", ck_blbound, false );
                pb_density = us_pushbutton( tr( "Density, 20" ) + DEGC ); 
                le_density = us_lineedit();
                pb_densapp = us_pushbutton( tr( "Apply to:" ) );
   QLabel*      lb_densscn = us_label( tr( "Scan(s)" ) );
                le_densscn = us_lineedit();
   QLabel*      lb_param2  = us_label( tr( "Parameter:" ) );
   QLabel*      lb_guess2  = us_label( tr( "Initial Guess:" ) );
   QLabel*      lb_bounds2 = us_label( tr( "+/- Bounds:" ) );
   QLabel*      lb_units2  = us_label( tr( "Units" ) );
   QLabel*      lb_float2  = us_label( tr( "Float:" ) );
   QLabel*      lb_lock2   = us_label( tr( "Lock:" ) );
   QLabel*      lb_bound2  = us_label( tr( "Bound:" ) );
   QLabel*      lb_lrunpar = us_label(
         tr( "Local Scan Parameters for Component: " ) );
                ct_lrunpar = us_counter( 2, 1, 100, 1 );
   QLabel*      lb_param3  = us_label( tr( "Parameter:" ) );
   QLabel*      lb_guess3  = us_label( tr( "Initial Guess:" ) );
   QLabel*      lb_bounds3 = us_label( tr( "+/- Bounds:" ) );
   QLabel*      lb_units3  = us_label( tr( "Units" ) );
   QLabel*      lb_float3  = us_label( tr( "Float:" ) );
   QLabel*      lb_lock3   = us_label( tr( "Lock:" ) );
   QLabel*      lb_bound3  = us_label( tr( "Bound:" ) );
                lb_amplitu = us_label( tr( "Amplitude" ) + " (1):" );
                le_amguess = us_lineedit();
                le_ambound = us_lineedit();
   QLabel*      lb_amunits = us_label( tr( "OD" ) );
   QGridLayout* lo_amfloat = us_radiobutton( "F", rb_amfloat, true  );
   QGridLayout* lo_amlock  = us_radiobutton( "L", rb_amlock,  false );
   QHBoxLayout* lo_amflbox = radiobox( lo_amfloat, rb_amfloat,
                                       lo_amlock,  rb_amlock );
   QLayout*     lo_ambound = us_checkbox( "B", ck_ambound, false );
                pb_extinct = us_pushbutton( tr( "Extinction" ) + " (1):" );
                le_extinct = us_lineedit( "1.0" );
                pb_extiapp = us_pushbutton( tr( "Apply to:" ) );
   QLabel*      lb_extiscn = us_label( tr( "Scan(s)" ) );
                le_extiscn = us_lineedit();
   QLabel*      lb_sigma   = us_label( tr( "Sigma for this Scan:" ) );
                le_sigma   = us_lineedit( "0", 0, true );
   QLayout*     lo_inclfit = us_checkbox(
         tr( "Include this Scan in the Fit:" ), ck_inclfit, false );
   QLabel*      lb_scansel = us_label( tr( "Scan Selector:" ) );
                ct_scansel = us_counter( 3, 1, 100, 1 );
   QString      extintt    = tr( "Extinction coefficient in units of"
                                 " OD / (cm * mol)" );
   pb_extinct->setToolTip( extintt );
   le_extinct->setToolTip( extintt );

   row     = 0;
   localLayout->addWidget( lb_lbanner, row++, 0, 1, 10 );
   localLayout->addWidget( lb_cdescr,  row,   0, 1,  2 );
   localLayout->addWidget( le_cdescr,  row++, 2, 1,  8 );
   localLayout->addWidget( lb_runid,   row,   0, 1,  2 );
   localLayout->addWidget( le_runid,   row,   2, 1,  2 );
   localLayout->addWidget( lb_tempera, row,   4, 1,  2 );
   localLayout->addWidget( le_tempera, row++, 6, 1,  2 );
   localLayout->addWidget( lb_speed,   row,   0, 1,  2 );
   localLayout->addWidget( le_speed,   row,   2, 1,  2 );
   localLayout->addWidget( lb_wavelen, row,   4, 1,  2 );
   localLayout->addWidget( le_wavelen, row++, 6, 1,  2 );
   localLayout->addWidget( lb_pathlen, row,   0, 1,  2 );
   localLayout->addWidget( le_pathlen, row,   2, 1,  2 );
   localLayout->addWidget( pb_plenapp, row,   4, 1,  2 );
   localLayout->addWidget( lb_plenscn, row,   6, 1,  2 );
   localLayout->addWidget( le_plenscn, row++, 8, 1,  2 );
   localLayout->addWidget( lb_param2,  row,   0, 1,  2 );
   localLayout->addWidget( lb_guess2,  row,   2, 1,  2 );
   localLayout->addWidget( lb_bounds2, row,   4, 1,  2 );
   localLayout->addWidget( lb_units2,  row,   6, 1,  1 );
   localLayout->addWidget( lb_float2,  row,   7, 1,  1 );
   localLayout->addWidget( lb_lock2,   row,   8, 1,  1 );
   localLayout->addWidget( lb_bound2,  row++, 9, 1,  1 );
   localLayout->addWidget( lb_baseln,  row,   0, 1,  2 );
   localLayout->addWidget( le_blguess, row,   2, 1,  2 );
   localLayout->addWidget( le_blbound, row,   4, 1,  2 );
   localLayout->addWidget( lb_blunits, row,   6, 1,  1 );
   localLayout->addLayout( lo_blflbox, row,   7, 1,  2 );
   localLayout->addLayout( lo_blbound, row++, 9, 1,  1 );
   localLayout->addWidget( pb_density, row,   0, 1,  2 );
   localLayout->addWidget( le_density, row,   2, 1,  2 );
   localLayout->addWidget( pb_densapp, row,   4, 1,  2 );
   localLayout->addWidget( lb_densscn, row,   6, 1,  2 );
   localLayout->addWidget( le_densscn, row++, 8, 1,  2 );
   localLayout->addWidget( lb_lrunpar, row,   0, 1,  7 );
   localLayout->addWidget( ct_lrunpar, row++, 7, 1,  3 );
   localLayout->addWidget( lb_param3,  row,   0, 1,  2 );
   localLayout->addWidget( lb_guess3,  row,   2, 1,  2 );
   localLayout->addWidget( lb_bounds3, row,   4, 1,  2 );
   localLayout->addWidget( lb_units3,  row,   6, 1,  1 );
   localLayout->addWidget( lb_float3,  row,   7, 1,  1 );
   localLayout->addWidget( lb_lock3,   row,   8, 1,  1 );
   localLayout->addWidget( lb_bound3,  row++, 9, 1,  1 );
   localLayout->addWidget( lb_amplitu, row,   0, 1,  2 );
   localLayout->addWidget( le_amguess, row,   2, 1,  2 );
   localLayout->addWidget( le_ambound, row,   4, 1,  2 );
   localLayout->addWidget( lb_amunits, row,   6, 1,  1 );
   localLayout->addLayout( lo_amflbox, row,   7, 1,  2 );
   localLayout->addLayout( lo_ambound, row++, 9, 1,  1 );
   localLayout->addWidget( pb_extinct, row,   0, 1,  2 );
   localLayout->addWidget( le_extinct, row,   2, 1,  2 );
   localLayout->addWidget( pb_extiapp, row,   4, 1,  2 );
   localLayout->addWidget( lb_extiscn, row,   6, 1,  2 );
   localLayout->addWidget( le_extiscn, row++, 8, 1,  2 );
   localLayout->addWidget( lb_sigma,   row,   0, 1,  2 );
   localLayout->addWidget( le_sigma,   row++, 2, 1,  2 );
   localLayout->addLayout( lo_inclfit, row,   0, 1,  4 );
   localLayout->addWidget( lb_scansel, row,   4, 1,  3 );
   localLayout->addWidget( ct_scansel, row++, 7, 1,  3 );

   ck_inclfit->setEnabled( false );
   ck_blbound->setEnabled( false );
   ck_blbound->setEnabled( false );
   ck_ambound->setEnabled( false );
   le_blbound->setEnabled( false );
   le_ambound->setEnabled( false );
   us_setReadOnly( le_tempera, true );
   us_setReadOnly( le_wavelen, true );
   us_setReadOnly( le_blbound, true );
   us_setReadOnly( le_ambound, true );

   connect( ct_lrunpar, SIGNAL( valueChanged( double ) ),
            this,   SLOT( local_comp_changed( double ) ) );
   connect( ct_scansel, SIGNAL( valueChanged( double ) ),
            this,       SLOT(   scan_changed( double ) ) );
   connect( rb_blfloat, SIGNAL( toggled( bool ) ),
            this,       SLOT(   update_floats() ) );
   connect( rb_amfloat, SIGNAL( toggled( bool ) ),
            this,       SLOT(   update_floats() ) );
   connect( pb_plenapp, SIGNAL( clicked()       ),
            this,       SLOT( pathlen_applyto() ) );
   connect( pb_density, SIGNAL( clicked()       ),
            this,       SLOT(   set_density()   ) );
   connect( pb_densapp, SIGNAL( clicked()       ),
            this,       SLOT( density_applyto() ) );
   connect( pb_extiapp, SIGNAL( clicked()       ),
            this,       SLOT( extinct_applyto() ) );
   ct_lrunpar->setRange( 1, runfit.nbr_comps );
   ct_lrunpar->setSingleStep( 1 );
   ct_scansel->setRange( 1, scanfits.size() );
   ct_scansel->setSingleStep( 1 );
   send_signal = false;
   ct_scansel->setValue( selscan );
   send_signal = true;
   scan_changed( (double)selscan );

   // Button Row  (spacer, help, close)
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_help  = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_help,  SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( selected() ) );
   buttons->addStretch( );
   buttons->addWidget( pb_help );
   buttons->addWidget( pb_close );
   buttons->setStretch( 0, 2 );
   buttons->setStretch( 1, 1 );
   buttons->setStretch( 2, 1 );

   main->addLayout( globalLayout );
   main->addLayout( localLayout  );
   main->addLayout( buttons      );

   // Set values from input data
   int scanx = selscan - 1;
   le_cdescr ->setText( scanfits[ scanx ].descript );
   le_runid  ->setText( scanfits[ scanx ].runID );
   le_tempera->setText( QString::number( scanfits[ scanx ].tempera )    );
   le_speed  ->setText( QString::number( scanfits[ scanx ].rpm )        );
   le_wavelen->setText( QString::number( scanfits[ scanx ].wavelen )    );
   le_pathlen->setText( QString::number( scanfits[ scanx ].pathlen )    );
   le_blguess->setText( QString::number( scanfits[ scanx ].baseline )   );
   le_blbound->setText( QString::number( scanfits[ scanx ].baseln_rng ) );
   le_density->setText( QString::number( scanfits[ scanx ].density )    );
   le_extinct->setText( QString::number( scanfits[ scanx ].extincts[ 0 ] ) );

   le_densscn->setText( "1" );
   le_extiscn->setText( "1" );

   global_comp_changed( 1.0 );
   local_comp_changed(  1.0 );
   connect_global();
   connect_local();

   // Resize to fit elements added
   adjustSize();

   setMinimumSize( 600, 500 );

   chng_floats = true;
}

// Public slot to reset scan number
void US_EqModelControl::new_scan( int scannbr )
{
qDebug() << "EMC: new_scan" << scannbr;
   send_signal = false;
   selscan     = scannbr;

   ct_scansel->setValue( (double)selscan );
   send_signal = true;
}

// Public slot to reset general components when runfit and scanfits change
void US_EqModelControl::new_components()
{
qDebug() << "EMC: new_components" << modelx;
   double dscn = ct_scansel->value();
   double dgco = ct_grunpar->value();
   double dlco = ct_lrunpar->value();

   // To reset values, we simulate new scan and global,local component numbers
   send_signal = false;
   scan_changed       ( dscn );
   global_comp_changed( dgco );
   local_comp_changed ( dlco );
   send_signal = true;
}

// Public slot to reset all floated/locked check boxes
void US_EqModelControl::set_float( bool floated )
{
qDebug() << "EMC: set_float" << floated;
   chng_floats = false;

   if ( floated )
   {
      rb_mwfloat->setChecked( floated );
      rb_vbfloat->setChecked( floated );
      rb_l1float->setChecked( floated );
      rb_l2float->setChecked( floated );
      rb_l3float->setChecked( floated );
      rb_l4float->setChecked( floated );
      rb_blfloat->setChecked( floated );
      rb_amfloat->setChecked( floated );
   }

   else
   {
      rb_mwlock ->setChecked( true );
      rb_vblock ->setChecked( true );
      rb_l1lock ->setChecked( true );
      rb_l2lock ->setChecked( true );
      rb_l3lock ->setChecked( true );
      rb_l4lock ->setChecked( true );
      rb_bllock ->setChecked( true );
      rb_amlock ->setChecked( true );
   }

   chng_floats = true;
   int gcmpn   = (int)ct_grunpar->value();
   int scann   = selscan;
   int lcmpn   = (int)ct_lrunpar->value();

   for ( int compn = 1; compn <= runfit.nbr_comps; compn++ )
   {
      ct_grunpar->setValue( compn );
      update_floats();
   }

   ct_grunpar->setValue( gcmpn );

   for ( selscan = 1; selscan <= scanfits.size(); selscan++ )
   {
      for ( int compn = 1; compn <= runfit.nbr_comps; compn++ )
      {
         ct_lrunpar->setValue( compn );
         update_floats();
      }
   }

   ct_lrunpar->setValue( lcmpn );
   selscan     = scann;
}

// Private slot for change in scan number
void US_EqModelControl::scan_changed( double value )
{
qDebug() << "EMC: scan_changed" << value;
   selscan       = (int)value;
   QString ltext = lb_lbanner->text();
   int     jj    = ltext.lastIndexOf( " " ) + 1;
   int     nch   = ltext.mid( jj ).length();
   QString scan_s = QString::number( selscan );
   ltext.replace( jj, nch, scan_s );
   lb_lbanner->setText( ltext );
   le_plenscn->setText( scan_s );
   le_densscn->setText( scan_s );
   le_extiscn->setText( scan_s );

   disconnect_local();

   int scanx     = selscan - 1;
   le_cdescr ->setText( scanfits[ scanx ].descript );
   le_runid  ->setText( scanfits[ scanx ].runID );
   le_tempera->setText( QString::number( scanfits[ scanx ].tempera )    );
   le_speed  ->setText( QString::number( scanfits[ scanx ].rpm )        );
   le_wavelen->setText( QString::number( scanfits[ scanx ].wavelen )    );
   le_pathlen->setText( QString::number( scanfits[ scanx ].pathlen )    );
   le_blguess->setText( QString::number( scanfits[ scanx ].baseline )   );
   le_blbound->setText( QString::number( scanfits[ scanx ].baseln_rng ) );
   bool scnFit   = scanfits[ scanx ].scanFit;
   ck_inclfit->setChecked(     scnFit );
   us_setReadOnly( le_blguess, ! scnFit );
   us_setReadOnly( le_amguess, ! scnFit );
   int compx     = (int)ct_lrunpar->value() - 1;
   double ampv   = scanfits[ scanx ].amp_vals[ compx ];
   double ampb   = scanfits[ scanx ].amp_rngs[ compx ];
   ampb          = ( ampb == 0.0 ) ? ( ampv * 0.2 ) : ampb;
   le_amguess->setText( QString::number( ampv ) );
   le_ambound->setText( QString::number( ampb ) );
   jj            = qMin( scanfits[ scanx ].extincts.size() - 1, compx );
   le_extinct->setText( QString::number( scanfits[ scanx ].extincts[ jj ] ) );
   le_density->setText( QString::number( scanfits[ scanx ].density ) );
   chng_floats   = false;

   if ( scanfits[ scanx ].baseln_fit )
      rb_blfloat->setChecked( true );
   else
      rb_bllock ->setChecked( true );

   if ( scanfits[ scanx ].amp_fits[ compx ] )
      rb_amfloat->setChecked( true );
   else
      rb_amlock ->setChecked( true );

   chng_floats   = true;

   if ( send_signal )
   {
      emit update_scan( selscan );
   }

   else
      send_signal = true;

   update_sigma();
   connect_local();
}

// Private slot for change in global component number
void US_EqModelControl::global_comp_changed( double value )
{
qDebug() << "EMC: global_comp_changed" << value;
   int compn  = (int)value;
   int compx  = compn - 1;
   QString molwlb = lb_molecwt->text();
   QString vbarpb = pb_vbar20 ->text();
   QString comp_s = QString::number( compn );
   int mwnx   = molwlb.lastIndexOf( "(" ) + 1;
   int lnmwr  = molwlb.mid( mwnx ).lastIndexOf( ")" );
   int vbnx   = vbarpb.lastIndexOf( "(" ) + 1;
   int lnvbr  = vbarpb.mid( vbnx ).lastIndexOf( ")" );

   disconnect_global();
   molwlb.replace( mwnx, lnmwr, comp_s );
   vbarpb.replace( vbnx, lnvbr, comp_s );
   lb_molecwt->setText( molwlb );
   pb_vbar20 ->setText( vbarpb );

   le_mwguess->setText( QString::number( runfit.mw_vals[   compx ] ) );
   le_mwbound->setText( QString::number( runfit.mw_rngs[   compx ] ) );
   le_vbguess->setText( QString::number( runfit.vbar_vals[ compx ] ) );
   le_vbbound->setText( QString::number( runfit.vbar_rngs[ compx ] ) );
   chng_floats = false;

   if ( runfit.mw_fits  [ compx ] )
      rb_mwfloat->setChecked( true );
   else
      rb_mwlock ->setChecked( true );

   if ( runfit.vbar_fits[ compx ] )
      rb_vbfloat->setChecked( true );
   else
      rb_vblock ->setChecked( true );

   if ( runfit.eq_fits  [ 0 ] )
      rb_l1float->setChecked( true );
   else
      rb_l1lock ->setChecked( true );

   if ( runfit.eq_fits  [ 1 ] )
      rb_l2float->setChecked( true );
   else
      rb_l2lock ->setChecked( true );

   if ( runfit.eq_fits  [ 2 ] )
      rb_l3float->setChecked( true );
   else
      rb_l3lock ->setChecked( true );

   if ( runfit.eq_fits  [ 3 ] )
      rb_l4float->setChecked( true );
   else
      rb_l4lock ->setChecked( true );

   chng_floats = true;

   update_sigma();
   connect_global();
}

// Private slot for change in local component number
void US_EqModelControl::local_comp_changed( double value )
{
qDebug() << "EMC: local_comp_changed" << value;
   int compn  = (int)value;
   int compx  = compn - 1;
   int scanx  = selscan - 1;
   disconnect_local();

   QString ampllb = lb_amplitu->text();
   QString extipb = pb_extinct->text();
   QString comp_s = QString::number( compn );
   int amnx   = ampllb.lastIndexOf( "(" ) + 1;
   int lnamr  = ampllb.mid( amnx ).lastIndexOf( ")" );
   int exnx   = extipb.lastIndexOf( "(" ) + 1;
   int lnexr  = extipb.mid( exnx ).lastIndexOf( ")" );
   ampllb.replace( amnx, lnamr, comp_s );
   extipb.replace( exnx, lnexr, comp_s );
   lb_amplitu->setText( ampllb );
   pb_extinct->setText( extipb );

   double ampv   = scanfits[ scanx ].amp_vals[ compx ];
   double ampb   = scanfits[ scanx ].amp_rngs[ compx ];
   ampb          = ( ampb == 0.0 ) ? ( ampv * 0.2 ) : ampb;
   le_amguess->setText( QString::number( ampv ) );
   le_ambound->setText( QString::number( ampb ) );
   le_density->setText( QString::number( scanfits[ scanx ].density ) );
   int jj = qMin( scanfits[ scanx ].extincts.size() - 1, compx );
   le_extinct->setText( QString::number( scanfits[ scanx ].extincts[ jj ] ) );
   chng_floats   = false;
   rb_amfloat->setChecked( scanfits[ scanx ].amp_fits[ compx ] );
   chng_floats   = true;

   update_sigma();
   connect_local();
}

// Select Model button:  set up to return data information
void US_EqModelControl::selected()
{
   update_gvals();
   update_lvals();

   accept();
   mWidget  = false;
   close();
}

// Update the sigma field after changes in molecular weight
void US_EqModelControl::update_sigma( void )
{
qDebug() << "EMC: update_sigma";
   int    scanx    = selscan - 1;
   int    compx    = (int)ct_grunpar->value() - 1;
   EqScanFit* sfit = &scanfits[ scanx ];
   double molecwt  = runfit.mw_vals  [ compx ];
   double vbar20   = runfit.vbar_vals[ compx ];
   double tempa    = sfit->tempera;
   double density  = sfit->density;
   double viscos   = sfit->viscosity;
   double tempk    = tempa + K0;
   double vbar     = US_Math2::adjust_vbar20( vbar20, tempa );
   double omega_s  = sq( sfit->rpm * M_PI / 30.0 );

   US_Math2::SolutionData solution;
   solution.vbar20     = vbar20;
   solution.vbar       = vbar;
   solution.density    = density;
   solution.viscosity  = viscos;

   US_Math2::data_correction( tempa, solution );

   double sigma    = ( 1.0 - vbar * solution.density_tb ) * omega_s * molecwt
                     / ( 2.0 * R_GC * tempk );
   le_sigma->setText( QString::number( sigma ) );
qDebug() << "EMC: u_s: scanx compx" << scanx << compx;
qDebug() << "EMC: u_s:  mw vbar20 vbar" << molecwt << vbar20 << vbar;
qDebug() << "EMC: u_s:  dens dens_tb" << density << solution.density_tb;
qDebug() << "EMC: u_s:    sigma" << sigma;
}

// Update data to reflect changes in float/fixed status
void US_EqModelControl::update_floats( void )
{
qDebug() << "EMC:UpFl: chng_floats" << chng_floats;
   if ( ! chng_floats )
      return;

   int scanx = selscan - 1;
   int compx = (int)ct_grunpar->value() - 1;

   runfit.mw_fits  [ compx ] = rb_mwfloat->isChecked();
   runfit.vbar_fits[ compx ] = rb_vbfloat->isChecked();
   runfit.eq_fits  [ 0     ] = rb_l1float->isChecked();
   runfit.eq_fits  [ 1     ] = rb_l2float->isChecked();
   runfit.eq_fits  [ 2     ] = rb_l3float->isChecked();
   runfit.eq_fits  [ 3     ] = rb_l4float->isChecked();

   compx     = (int)ct_lrunpar->value() - 1;
   scanfits[ scanx ].baseln_fit        = rb_blfloat->isChecked();
   scanfits[ scanx ].amp_fits[ compx ] = rb_amfloat->isChecked();
}

// Update global fit values
void US_EqModelControl::update_gvals( void )
{
   int    compx    = (int)ct_grunpar->value() - 1;
   runfit.mw_vals  [ compx ] = le_mwguess->text().toDouble();
   runfit.mw_bnds  [ compx ] = le_mwbound->text().toDouble();
   runfit.vbar_vals[ compx ] = le_vbguess->text().toDouble();
   runfit.vbar_bnds[ compx ] = le_vbbound->text().toDouble();
   runfit.eq_vals  [ 0     ] = le_l1guess->text().toDouble();
   runfit.eq_bnds  [ 0     ] = le_l1bound->text().toDouble();
   runfit.eq_vals  [ 1     ] = le_l2guess->text().toDouble();
   runfit.eq_bnds  [ 1     ] = le_l2bound->text().toDouble();
   runfit.eq_vals  [ 2     ] = le_l3guess->text().toDouble();
   runfit.eq_bnds  [ 2     ] = le_l3bound->text().toDouble();
   runfit.eq_vals  [ 3     ] = le_l4guess->text().toDouble();
   runfit.eq_bnds  [ 3     ] = le_l4bound->text().toDouble();
}

// Update local fit values
void US_EqModelControl::update_lvals( void )
{
   int    scanx    = selscan - 1;
   int    compx    = (int)ct_lrunpar->value() - 1;
   int    extix    = qMin( compx, scanfits[ scanx ].extincts.size() - 1 );
   scanfits[ scanx ].amp_vals[ compx ] = le_amguess->text().toDouble();
   scanfits[ scanx ].amp_bnds[ compx ] = le_ambound->text().toDouble();
   scanfits[ scanx ].baseline          = le_blguess->text().toDouble();
   scanfits[ scanx ].baseln_bnd        = le_blbound->text().toDouble();
   scanfits[ scanx ].extincts[ extix ] = le_extinct->text().toDouble();
}

// Connect global value GUI components to update slot
void US_EqModelControl::connect_global( void )
{
   connect( le_mwguess, SIGNAL( editingFinished() ), SLOT( update_gvals() ) );
   connect( le_mwbound, SIGNAL( editingFinished() ), SLOT( update_gvals() ) );
   connect( le_vbguess, SIGNAL( editingFinished() ), SLOT( update_gvals() ) );
   connect( le_vbbound, SIGNAL( editingFinished() ), SLOT( update_gvals() ) );
   connect( le_l1guess, SIGNAL( editingFinished() ), SLOT( update_gvals() ) );
   connect( le_l1bound, SIGNAL( editingFinished() ), SLOT( update_gvals() ) );
   connect( le_l2guess, SIGNAL( editingFinished() ), SLOT( update_gvals() ) );
   connect( le_l2bound, SIGNAL( editingFinished() ), SLOT( update_gvals() ) );
   connect( le_l3guess, SIGNAL( editingFinished() ), SLOT( update_gvals() ) );
   connect( le_l3bound, SIGNAL( editingFinished() ), SLOT( update_gvals() ) );
   connect( le_l4guess, SIGNAL( editingFinished() ), SLOT( update_gvals() ) );
   connect( le_l4bound, SIGNAL( editingFinished() ), SLOT( update_gvals() ) );
}

// Connect local value GUI components to update slot
void US_EqModelControl::connect_local( void )
{
   connect( le_amguess, SIGNAL( editingFinished() ), SLOT( update_lvals() ) );
   connect( le_ambound, SIGNAL( editingFinished() ), SLOT( update_lvals() ) );
   connect( le_blguess, SIGNAL( editingFinished() ), SLOT( update_lvals() ) );
   connect( le_blbound, SIGNAL( editingFinished() ), SLOT( update_lvals() ) );
   connect( le_extinct, SIGNAL( editingFinished() ), SLOT( update_lvals() ) );
}

// Disconnect global value GUI components
void US_EqModelControl::disconnect_global( void )
{
   le_mwguess->disconnect();
   le_mwbound->disconnect();
   le_vbguess->disconnect();
   le_vbbound->disconnect();
   le_l1guess->disconnect();
   le_l1bound->disconnect();
   le_l2guess->disconnect();
   le_l2bound->disconnect();
   le_l3guess->disconnect();
   le_l3bound->disconnect();
   le_l4guess->disconnect();
   le_l4bound->disconnect();
}

// Disconnect local value GUI components
void US_EqModelControl::disconnect_local( void )
{
   le_amguess->disconnect();
   le_ambound->disconnect();
   le_blguess->disconnect();
   le_blbound->disconnect();
   le_extinct->disconnect();
}

// Apply pathlen to specified scan(s)
void US_EqModelControl::pathlen_applyto( void )
{
   QString scans = le_plenscn->text().simplified();
   int     jj    = scans.indexOf( "-" );
   jj            = ( jj < 0 ) ? scans.indexOf( " " ) : jj;
   jj            = ( jj < 0 ) ? scans.indexOf( "," ) : jj;
   int iscan     = scans.toInt();
   int lscan     = iscan;
   double pathln = le_pathlen->text().toDouble();

   if ( jj > 0 )
   {
      iscan      = scans.left( jj ).toInt();
      lscan      = scans.mid( jj + 1 ).toInt();
   }

   for ( int ii = iscan - 1; ii < lscan; ii++ )
   {
      scanfits[ ii ].pathlen = pathln;
   }
}

// Apply density to specified scan(s)
void US_EqModelControl::density_applyto( void )
{
   QString scans = le_densscn->text().simplified();
   int     jj    = scans.indexOf( "-" );
   jj            = ( jj < 0 ) ? scans.indexOf( " " ) : jj;
   jj            = ( jj < 0 ) ? scans.indexOf( "," ) : jj;
   int iscan     = scans.toInt();
   int lscan     = iscan;
   double densty = le_density->text().toDouble();

   if ( jj > 0 )
   {
      iscan      = scans.left( jj ).toInt();
      lscan      = scans.mid( jj + 1 ).toInt();
   }

   for ( int ii = iscan - 1; ii < lscan; ii++ )
   {
      scanfits[ ii ].density = densty;
   }
}

// Apply extinction to specified scan(s)
void US_EqModelControl::extinct_applyto( void )
{
   QString scans = le_extiscn->text().simplified();
   int     jj    = scans.indexOf( "-" );
   jj            = ( jj < 0 ) ? scans.indexOf( " " ) : jj;
   jj            = ( jj < 0 ) ? scans.indexOf( "," ) : jj;
   int iscan     = scans.toInt();
   int lscan     = iscan;
   int compx     = (int)ct_lrunpar->value() - 1;
   double extval = le_extinct->text().toDouble();

   if ( jj > 0 )
   {
      iscan      = scans.left( jj ).toInt();
      lscan      = scans.mid( jj + 1 ).toInt();
   }

   for ( int ii = iscan - 1; ii < lscan; ii++ )
   {
      scanfits[ ii ].extincts[ compx ] = extval;
   }
}

// Create a button box for a pair of radio buttons
QHBoxLayout* US_EqModelControl::radiobox(
      QGridLayout* lo_rb1, QRadioButton* rbtn1,
      QGridLayout* lo_rb2, QRadioButton* rbtn2 )
{
   // Create the box and add radiobutton layouts to it
   QHBoxLayout* lo_radiobox = new QHBoxLayout;
   lo_radiobox->setSpacing        ( 0 );
   lo_radiobox->setContentsMargins( 0, 0, 0, 0 );
   lo_radiobox->addLayout( lo_rb1 );
   lo_radiobox->addLayout( lo_rb2  );
   // Create a button group, add buttons, and set exclusive
   QButtonGroup* btn_grp = new QButtonGroup();
   btn_grp->addButton( rbtn1 );
   btn_grp->addButton( rbtn2 );
   btn_grp->setExclusive( true );

   return lo_radiobox;
}

// Set Vbar20 from an analyte dialog, after Vbar button is clicked
void US_EqModelControl::set_vbar()
{
   int            dbdisk = runfit.dbdisk;
   US_AnalyteGui* adiag  = new US_AnalyteGui( true, QString(), dbdisk );
   connect( adiag, SIGNAL( valueChanged( US_Analyte ) ),
            this,  SLOT  ( assignVbar  ( US_Analyte ) ) );
   adiag->exec();
   qApp->processEvents();
}

// Set Density from a buffer dialog, after Density button is clicked
void US_EqModelControl::set_density()
{
   US_Buffer buff;
   int            dbdisk = runfit.dbdisk;
   US_BufferGui*  bdiag  = new US_BufferGui( true, buff, dbdisk );
   connect( bdiag, SIGNAL( valueChanged(  US_Buffer ) ),
            this,  SLOT  ( assignDensity( US_Buffer ) ) );
   bdiag->exec();
   qApp->processEvents();
}

// Assign the Vbar from an analyte selected in US_AnalyteGui
void US_EqModelControl::assignVbar( US_Analyte analyte )
{
   int compx  = (int)ct_lrunpar->value() - 1;
   runfit.vbar_vals[ compx ] = analyte.vbar20;
   le_vbguess->setText( QString::number( runfit.vbar_vals[ compx ] ) );
}

// Assign the Density from a buffer selected in US_BufferGui
void US_EqModelControl::assignDensity( US_Buffer buffer )
{
   int scanx = selscan - 1;
   scanfits[ scanx ].density = buffer.density;
   le_density->setText( QString::number( scanfits[ scanx ].density ) );
}

