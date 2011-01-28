//! \file us_eqmodel_control.cpp

#include "us_eqmodel_control.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"

// Main constructor with references to parameters from main GlobalEquil class
US_EqModelControl::US_EqModelControl(
      QVector< EqScanFit >&   a_scanfits,
      EqRunFit&               a_runfit,
      US_DataIO2::EditedData* a_edata,
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
   lb_gbanner           = us_banner( models[ modelx ] );
   QLabel*  lb_grunpar  = us_label(
         tr( "Global Run Parameters for Component: " ) );
            ct_grunpar  = us_counter( 2, 1, 100, 1 );
   QLabel*  lb_param    = us_label( tr( "Parameter:" ) );
   QLabel*  lb_guess    = us_label( tr( "Initial Guess:" ) );
   QLabel*  lb_bounds   = us_label( tr( "+/- Bounds:" ) );
   QLabel*  lb_units    = us_label( tr( "Units" ) );
   QLabel*  lb_float    = us_label( tr( "Float:" ) );
   QLabel*  lb_lock     = us_label( tr( "Lock:" ) );
   QLabel*  lb_bound    = us_label( tr( "Bound:" ) );
            lb_molecwt  = us_label( tr( "Molecular Weight" ) + " (1):" );
            le_mwguess  = us_lineedit();
            le_mwbound  = us_lineedit();
   QLabel*  lb_mwunits  = us_label( tr( "Dalton" ) );
   QLayout* lo_mwfloat  = us_checkbox( "F", ck_mwfloat, false );
   QLayout* lo_mwlock   = us_checkbox( "L", ck_mwlock,  true  );
   QLayout* lo_mwbound  = us_checkbox( "B", ck_mwbound, true  );
            pb_vbar20   = us_pushbutton( tr( "Vbar, 20" ) + DEGC + " (1):" ); 
            le_vbguess  = us_lineedit();
            le_vbbound  = us_lineedit();
   QLabel*  lb_vbunits  = us_label( tr( "ccm/g" ) );
   QLayout* lo_vbfloat  = us_checkbox( "F", ck_vbfloat, false );
   QLayout* lo_vblock   = us_checkbox( "L", ck_vblock,  true  );
   QLayout* lo_vbbound  = us_checkbox( "B", ck_vbbound, true  );
            pb_lnasc1   = us_pushbutton( tr( "ln(Assoc.Const.1):" ) );
            le_l1guess  = us_lineedit();
            le_l1bound  = us_lineedit();
   QLabel*  lb_l1units  = us_label( tr( "" ) );
   QLayout* lo_l1float  = us_checkbox( "F", ck_l1float, false );
   QLayout* lo_l1lock   = us_checkbox( "L", ck_l1lock,  true  );
   QLayout* lo_l1bound  = us_checkbox( "B", ck_l1bound, true  );
            pb_lnasc2   = us_pushbutton( tr( "ln(Assoc.Const.2):" ) );
            le_l2guess  = us_lineedit();
            le_l2bound  = us_lineedit();
   QLabel*  lb_l2units  = us_label( tr( "" ) );
   QLayout* lo_l2float  = us_checkbox( "F", ck_l2float, false );
   QLayout* lo_l2lock   = us_checkbox( "L", ck_l2lock,  true  );
   QLayout* lo_l2bound  = us_checkbox( "B", ck_l2bound, true  );
            pb_lnasc3   = us_pushbutton( tr( "ln(Assoc.Const.3):" ) );
            le_l3guess  = us_lineedit();
            le_l3bound  = us_lineedit();
   QLabel*  lb_l3units  = us_label( tr( "" ) );
   QLayout* lo_l3float  = us_checkbox( "F", ck_l3float, false );
   QLayout* lo_l3lock   = us_checkbox( "L", ck_l3lock,  true  );
   QLayout* lo_l3bound  = us_checkbox( "B", ck_l3bound, true  );
            pb_lnasc4   = us_pushbutton( tr( "ln(Assoc.Const.4):" ) );
            le_l4guess  = us_lineedit();
            le_l4bound  = us_lineedit();
   QLabel*  lb_l4units  = us_label( tr( "" ) );
   QLayout* lo_l4float  = us_checkbox( "F", ck_l4float, false );
   QLayout* lo_l4lock   = us_checkbox( "L", ck_l4lock,  true  );
   QLayout* lo_l4bound  = us_checkbox( "B", ck_l4bound, true  );

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
   globalLayout->addLayout( lo_mwfloat, row,   7, 1,  1 );
   globalLayout->addLayout( lo_mwlock,  row,   8, 1,  1 );
   globalLayout->addLayout( lo_mwbound, row++, 9, 1,  1 );
   globalLayout->addWidget( pb_vbar20,  row,   0, 1,  2 );
   globalLayout->addWidget( le_vbguess, row,   2, 1,  2 );
   globalLayout->addWidget( le_vbbound, row,   4, 1,  2 );
   globalLayout->addWidget( lb_vbunits, row,   6, 1,  1 );
   globalLayout->addLayout( lo_vbfloat, row,   7, 1,  1 );
   globalLayout->addLayout( lo_vblock,  row,   8, 1,  1 );
   globalLayout->addLayout( lo_vbbound, row++, 9, 1,  1 );
   globalLayout->addWidget( pb_lnasc1,  row,   0, 1,  2 );
   globalLayout->addWidget( le_l1guess, row,   2, 1,  2 );
   globalLayout->addWidget( le_l1bound, row,   4, 1,  2 );
   globalLayout->addWidget( lb_l1units, row,   6, 1,  1 );
   globalLayout->addLayout( lo_l1float, row,   7, 1,  1 );
   globalLayout->addLayout( lo_l1lock,  row,   8, 1,  1 );
   globalLayout->addLayout( lo_l1bound, row++, 9, 1,  1 );
   globalLayout->addWidget( pb_lnasc2,  row,   0, 1,  2 );
   globalLayout->addWidget( le_l2guess, row,   2, 1,  2 );
   globalLayout->addWidget( le_l2bound, row,   4, 1,  2 );
   globalLayout->addWidget( lb_l2units, row,   6, 1,  1 );
   globalLayout->addLayout( lo_l2float, row,   7, 1,  1 );
   globalLayout->addLayout( lo_l2lock,  row,   8, 1,  1 );
   globalLayout->addLayout( lo_l2bound, row++, 9, 1,  1 );
   globalLayout->addWidget( pb_lnasc3,  row,   0, 1,  2 );
   globalLayout->addWidget( le_l3guess, row,   2, 1,  2 );
   globalLayout->addWidget( le_l3bound, row,   4, 1,  2 );
   globalLayout->addWidget( lb_l3units, row,   6, 1,  1 );
   globalLayout->addLayout( lo_l3float, row,   7, 1,  1 );
   globalLayout->addLayout( lo_l3lock,  row,   8, 1,  1 );
   globalLayout->addLayout( lo_l3bound, row++, 9, 1,  1 );
   globalLayout->addWidget( pb_lnasc4,  row,   0, 1,  2 );
   globalLayout->addWidget( le_l4guess, row,   2, 1,  2 );
   globalLayout->addWidget( le_l4bound, row,   4, 1,  2 );
   globalLayout->addWidget( lb_l4units, row,   6, 1,  1 );
   globalLayout->addLayout( lo_l4float, row,   7, 1,  1 );
   globalLayout->addLayout( lo_l4lock,  row,   8, 1,  1 );
   globalLayout->addLayout( lo_l4bound, row++, 9, 1,  1 );

   ct_grunpar->setStep( 1 );
   pb_lnasc1->setEnabled( false );
   pb_lnasc2->setEnabled( false );
   pb_lnasc3->setEnabled( false );
   pb_lnasc4->setEnabled( false );
   QButtonGroup* mw_grp = new QButtonGroup();
   QButtonGroup* vb_grp = new QButtonGroup();
   QButtonGroup* l1_grp = new QButtonGroup();
   QButtonGroup* l2_grp = new QButtonGroup();
   QButtonGroup* l3_grp = new QButtonGroup();
   QButtonGroup* l4_grp = new QButtonGroup();
   mw_grp->addButton( ck_mwfloat );
   mw_grp->addButton( ck_mwlock  );
   mw_grp->setExclusive( true );
   ck_mwbound->setEnabled( false );
   vb_grp->addButton( ck_vbfloat );
   vb_grp->addButton( ck_vblock  );
   vb_grp->setExclusive( true );
   ck_vbbound->setEnabled( false );
   l1_grp->addButton( ck_l1float );
   l1_grp->addButton( ck_l1lock  );
   l1_grp->setExclusive( true );
   ck_l1bound->setEnabled( false );
   l2_grp->addButton( ck_l2float );
   l2_grp->addButton( ck_l2lock  );
   l2_grp->setExclusive( true );
   ck_l2bound->setEnabled( false );
   l3_grp->addButton( ck_l3float );
   l3_grp->addButton( ck_l3lock  );
   l3_grp->setExclusive( true );
   ck_l3bound->setEnabled( false );
   l4_grp->addButton( ck_l4float );
   l4_grp->addButton( ck_l4lock  );
   l4_grp->setExclusive( true );
   ck_l4bound->setEnabled( false );


   // Local layout
   lb_lbanner           = us_banner(
         tr( "Local Scan Parameters for Scan %1" ).arg( selscan ) );
   QLabel*  lb_cdescr   = us_label( tr( "Cell Description:" ) );
            le_cdescr   = us_lineedit();
   QLabel*  lb_runid    = us_label( tr( "Run ID:" ) );
            le_runid    = us_lineedit();
   QLabel*  lb_tempera  = us_label( tr( "Temperature (" ) + DEGC + "):" );
            le_tempera  = us_lineedit();
   QLabel*  lb_speed    = us_label( tr( "Rotor Speed (rpm)" ) );
            le_speed    = us_lineedit();
   QLabel*  lb_wavelen  = us_label( tr( "Wavelength (nm)" ) );
            le_wavelen  = us_lineedit();
   QLabel*  lb_pathlen  = us_label( tr( "Cell Pathlength (cm)" ) );
            le_pathlen  = us_lineedit();
            pb_plenapp  = us_pushbutton( tr( "Apply to:" ) );
   QLabel*  lb_plenscn  = us_label( tr( "Scan(s)" ) );
            le_plenscn  = us_lineedit();
   QLabel*  lb_baseln   = us_label( tr( "Baseline:" ) );
            le_blguess  = us_lineedit();
            le_blbound  = us_lineedit();
   QLabel*  lb_blunits  = us_label( tr( "OD" ) );
   QLayout* lo_blfloat  = us_checkbox( "F", ck_blfloat, false );
   QLayout* lo_bllock   = us_checkbox( "L", ck_bllock,  true  );
   QLayout* lo_blbound  = us_checkbox( "B", ck_blbound, true  );
            pb_density  = us_pushbutton( tr( "Density, 20" ) + DEGC ); 
            le_density  = us_lineedit();
            pb_densapp  = us_pushbutton( tr( "Apply to:" ) );
   QLabel*  lb_densscn  = us_label( tr( "Scan(s)" ) );
            le_densscn  = us_lineedit();
   QLabel*  lb_param2   = us_label( tr( "Parameter:" ) );
   QLabel*  lb_guess2   = us_label( tr( "Initial Guess:" ) );
   QLabel*  lb_bounds2  = us_label( tr( "+/- Bounds:" ) );
   QLabel*  lb_units2   = us_label( tr( "Units" ) );
   QLabel*  lb_float2   = us_label( tr( "Float:" ) );
   QLabel*  lb_lock2    = us_label( tr( "Lock:" ) );
   QLabel*  lb_bound2   = us_label( tr( "Bound:" ) );
   QLabel*  lb_lrunpar  = us_label(
         tr( "Local Scan Parameters for Component: " ) );
            ct_lrunpar  = us_counter( 2, 1, 100, 1 );
   QLabel*  lb_param3   = us_label( tr( "Parameter:" ) );
   QLabel*  lb_guess3   = us_label( tr( "Initial Guess:" ) );
   QLabel*  lb_bounds3  = us_label( tr( "+/- Bounds:" ) );
   QLabel*  lb_units3   = us_label( tr( "Units" ) );
   QLabel*  lb_float3   = us_label( tr( "Float:" ) );
   QLabel*  lb_lock3    = us_label( tr( "Lock:" ) );
   QLabel*  lb_bound3   = us_label( tr( "Bound:" ) );
            lb_amplitu  = us_label( tr( "Amplitude" ) + " (1):" );
            le_amguess  = us_lineedit();
            le_ambound  = us_lineedit();
   QLabel*  lb_amunits  = us_label( tr( "OD" ) );
   QLayout* lo_amfloat  = us_checkbox( "F", ck_amfloat, false );
   QLayout* lo_amlock   = us_checkbox( "L", ck_amlock,  true  );
   QLayout* lo_ambound  = us_checkbox( "B", ck_ambound, true  );
            pb_eodcmmo  = us_pushbutton( tr( "E (OD/(cm*mol))" ) + " (1):" );
            le_eodcmmo  = us_lineedit();
            pb_eodcapp  = us_pushbutton( tr( "Apply to:" ) );
   QLabel*  lb_eodcscn  = us_label( tr( "Scan(s)" ) );
            le_eodcscn  = us_lineedit();
   QLayout* lo_inclfit  = us_checkbox( tr( "Include this Scan in the Fit:" ),
         ck_inclfit, false );
   QLabel*  lb_scansel  = us_label( tr( "Scan Selector:" ) );
            ct_scansel  = us_counter( 3, 1, 100, 1 );

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
   localLayout->addLayout( lo_blfloat, row,   7, 1,  1 );
   localLayout->addLayout( lo_bllock,  row,   8, 1,  1 );
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
   localLayout->addLayout( lo_amfloat, row,   7, 1,  1 );
   localLayout->addLayout( lo_amlock,  row,   8, 1,  1 );
   localLayout->addLayout( lo_ambound, row++, 9, 1,  1 );
   localLayout->addWidget( pb_eodcmmo, row,   0, 1,  2 );
   localLayout->addWidget( le_eodcmmo, row,   2, 1,  2 );
   localLayout->addWidget( pb_eodcapp, row,   4, 1,  2 );
   localLayout->addWidget( lb_eodcscn, row,   6, 1,  2 );
   localLayout->addWidget( le_eodcscn, row++, 8, 1,  2 );
   localLayout->addLayout( lo_inclfit, row,   0, 1,  4 );
   localLayout->addWidget( lb_scansel, row,   4, 1,  3 );
   localLayout->addWidget( ct_scansel, row++, 7, 1,  3 );

   QButtonGroup* bl_grp = new QButtonGroup();
   QButtonGroup* am_grp = new QButtonGroup();
   bl_grp->addButton( ck_blfloat );
   bl_grp->addButton( ck_bllock  );
   bl_grp->setExclusive( true );
   ck_blbound->setEnabled( false );
   am_grp->addButton( ck_amfloat );
   am_grp->addButton( ck_amlock  );
   am_grp->setExclusive( true );
   ck_ambound->setEnabled( false );

   ct_lrunpar->setStep( 1 );
   connect( ct_scansel, SIGNAL( valueChanged( double ) ),
            this,       SLOT(   scan_changed( double ) ) );
   ct_scansel->setRange( 1, scanfits.size(), 1 );
   ct_scansel->setStep( 1 );
   send_signal = false;
   ct_scansel->setValue( selscan );
   send_signal = true;

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
   le_tempera->setText( QString::number( scanfits[ scanx ].tempera ) );
   le_speed  ->setText( QString::number( scanfits[ scanx ].rpm )     );
   le_wavelen->setText( QString::number( scanfits[ scanx ].lambda  ) );
   le_pathlen->setText( QString::number( scanfits[ scanx ].pathlen ) );
   le_blguess->setText( QString::number( scanfits[ scanx ].baseline ) );

   le_densscn->setText( "1" );
   le_eodcscn->setText( "1" );

   // Resize to fit elements added
   adjustSize();

   setMinimumSize( 600, 500 );
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

// Private slot for change in scan number
void US_EqModelControl::scan_changed( double value )
{
qDebug() << "EMC: scan_changed" << value;
   selscan       = (int)value;
   QString ltext = lb_lbanner->text();
   int     jj    = ltext.lastIndexOf( " " ) + 1;
   int     nch   = ltext.mid( jj ).length();
   ltext.replace( jj, nch, QString::number( selscan ) );
   lb_lbanner->setText( ltext );

   int scanx     = selscan - 1;
   le_cdescr ->setText( scanfits[ scanx ].descript );
   le_runid  ->setText( scanfits[ scanx ].runID );
   le_tempera->setText( QString::number( scanfits[ scanx ].tempera ) );
   le_speed  ->setText( QString::number( scanfits[ scanx ].rpm )     );
   le_wavelen->setText( QString::number( scanfits[ scanx ].lambda  ) );
   le_pathlen->setText( QString::number( scanfits[ scanx ].pathlen ) );
   le_blguess->setText( QString::number( scanfits[ scanx ].baseline ) );

   if ( send_signal )
   {
      emit update_scan( selscan );
   }

   else
      send_signal = true;
}

// Select Model button:  set up to return data information
void US_EqModelControl::selected()
{
   accept();
   mWidget  = false;
   close();
}

