//! \file us_edit.cpp

//#include <QApplication>
//#include <QDomDocument>

#include "us_edit.h"
#include "us_exclude_profile.h"
#include "us_select_lambdas.h"
#include "us_ri_noise.h"
#include "us_edit_scan.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_run_details2.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_load_auc.h"
#include "us_passwd.h"
#include "us_get_edit.h"
#include "us_constants.h"
#include "us_images.h"
#include "us_editor.h"
#include "us_report.h"
#include "us_protocol_util.h"

//#include "json.hpp"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#define setSymbol(a)       setSymbol(*a)
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

#define _RNGLEFT_OFFSET_ 0.03
#define _PLATEAU_OFFSET_ 0.1




// Alt. Constructor
US_Edit::US_Edit( QString auto_mode ) : US_Widgets()
{
    
   check        = US_Images::getIcon( US_Images::CHECK );
   invert       = 1.0;
   all_edits    = false;
   men_1click   = US_Settings::debug_match( "men2click" ) ? false : true;
   total_speeds = 0;
   total_edits  = 0;
   v_line       = NULL;
   dbg_level    = US_Settings::us_debug();
   dbP          = NULL;
   chlamb       = QChar( 955 );
   gap_thresh   = 50.0;
   gap_fringe   = 0.4;
   bottom       = 0.0;
DbgLv(1) << " 0)gap_fringe" << gap_fringe;


  us_edit_auto_mode = true;
  us_edit_auto_mode_manual = false;
  all_loaded = false;
  is_spike_auto = false;

  triples_all_optics.clear();
  channels_all.clear();
  isSet_ref_wvl.clear();
 
//usmode = false;
 
   setWindowTitle( tr( "Edit UltraScan Data" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* top = new QVBoxLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );

   // Put the Run Info across the entire window
   QHBoxLayout* runInfo = new QHBoxLayout();
   QLabel* lb_info = us_label( tr( "Run Info:" ), -1 );
   runInfo->addWidget( lb_info );

   le_info = us_lineedit( "", 1, true );
   runInfo->addWidget( le_info );

   top->addLayout( runInfo );

   QHBoxLayout* main = new QHBoxLayout();
   QVBoxLayout* left = new QVBoxLayout;

   // Start of Grid Layout
   QGridLayout* specs = new QGridLayout;

   // Investigator
   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );

   if ( US_Settings::us_inv_level() < 3 )
      pb_investigator->setEnabled( false );

   int     id      = US_Settings::us_inv_ID();
   QString number  = ( id > 0 ) ? 
      QString::number( US_Settings::us_inv_ID() ) + ": " 
      : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(),
                                  1, true );

   // Disk/DB control
   disk_controls   = new US_Disk_DB_Controls;

   // Load
   QPushButton*
      pb_load      = us_pushbutton( tr( "Load Data" ) );
   pb_details      = us_pushbutton( tr( "Run Details" ), false );
      pb_report    = us_pushbutton( tr( "View Report" ), false );

   // Triple controls 
   QLabel* lb_cell = us_banner( tr( "Channel Controls" ) );

   // Triple and Speed Step
   lb_triple       = us_label( tr( "Cell / Channel / Wavelength" ), -1 );
   cb_triple       = us_comboBox();
   lb_rpms         = us_label( tr( "Speed Step (RPM) of triple" ), -1 );
   cb_rpms         = us_comboBox();
   lb_rpms->setVisible( false );
   cb_rpms->setVisible( false );

   // Scan Gaps
   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   lb_gaps         = us_label( tr( "Threshold for Scan Gaps" ), -1 );
   ct_gaps         = us_counter( 1, 10.0, 100.0 );
   ct_gaps->setSingleStep ( 10.0 );
   ct_gaps->setValue( 50.0 );

   // MWL Control
   QFontMetrics fmet( font );
   int fwid = fmet.maxWidth();
   int rhgt = ct_gaps->height();
   int lwid = fwid * 4;
   int swid = lwid + fwid;
   lb_mwlctl       = us_banner( tr( "Wavelength Controls" ) );
   QButtonGroup* r_group = new QButtonGroup( this );
   QButtonGroup* x_group = new QButtonGroup( this );
   lo_lrange       = us_radiobutton( tr( "Lambda Range" ),       rb_lrange,
                                     true  );
   lo_custom       = us_radiobutton( tr( "Custom Lambda List" ), rb_custom,
                                     false );
   rb_lrange->setFont( font );
   rb_custom->setFont( font );
   r_group->addButton( rb_lrange, 0 );
   r_group->addButton( rb_custom, 1 );
   lb_ldelta       = us_label( tr( "%1 Index Increment:" ).arg( chlamb ), -1 );
   ct_ldelta       = us_counter( 1, 1, 100, 1 );
   ct_ldelta->setFont( font );
   ct_ldelta->setSingleStep( 1 );
   ct_ldelta->setMinimumWidth( lwid );
   ct_ldelta->resize( rhgt, swid );
   lb_lstart       = us_label( tr( "%1 Start:" ).arg( chlamb ), -1 );
   lb_lend         = us_label( tr( "%1 End:"   ).arg( chlamb ), -1 );
   lb_lplot        = us_label( tr( "Plot (W nm):" ), -1 );

   int     nlmbd   = 224;
   int     lmbdlo  = 251;
   int     lmbdhi  = 650;
   int     lmbddl  = 1;
   QString lrsmry  = tr( "%1 raw: %2 %3 to %4" )
      .arg( nlmbd ).arg( chlamb ).arg( lmbdlo ).arg( lmbdhi );
   le_ltrng  = us_lineedit( lrsmry, -1, true );
   QString lxsmry = tr( "%1 MWL exports: %2 %3 to %4, raw index increment %5" )
      .arg( nlmbd ).arg( chlamb ).arg( lmbdlo ).arg( lmbdhi ).arg( lmbddl );
   le_lxrng       = us_lineedit( lxsmry, -1, true );
   cb_lplot       = us_comboBox();
   cb_lstart      = us_comboBox();
   cb_lend        = us_comboBox();

   cb_lplot ->setFont( font );
   cb_lstart->setFont( font );
   cb_lend  ->setFont( font );

   pb_custom      = us_pushbutton(  tr( "Custom Lambdas" ),      false, -1 );
   pb_incall      = us_pushbutton(  tr( "Include All Lambdas" ), true,  -1 );
   pb_larrow      = us_pushbutton(  tr( "previous" ), true, -2 );
   pb_rarrow      = us_pushbutton(  tr( "next"     ), true, -2 );
   pb_larrow->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT ) );
   pb_rarrow->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );

   lo_radius      = us_radiobutton( tr( "x axis Radius" ),     rb_radius,
                                    true  );
   lo_waveln      = us_radiobutton( tr( "x axis Wavelength" ), rb_waveln,
                                    false );
   rb_radius->setFont( font );
   rb_waveln->setFont( font );
   x_group->addButton( rb_radius, 0 );
   x_group->addButton( rb_waveln, 1 );

   QStringList lambdas;
lambdas << "250" << "350" << "450" << "550" << "580" << "583" << "650";
   cb_lplot ->addItems( lambdas );
   cb_lstart->addItems( lambdas );
   cb_lend  ->addItems( lambdas );

   cb_lplot ->setCurrentIndex( 2 );
   cb_lstart->setCurrentIndex( 0 );
   cb_lend  ->setCurrentIndex( 6 );

   connect_mwl_ctrls( true );

   // Scan Controls
   QLabel* lb_scan = us_banner( tr( "Scan Controls" ) );
   
   // Scans
   QLabel* lb_from = us_label(  tr( "Scan Focus from:" ), -1 );
   lb_from->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

   ct_from        = us_counter( 3, 0.0, 0.0 ); // Update range upon load
   ct_from->setSingleStep( 1 );

   QLabel* lb_to  = us_label( tr( "to:" ), -1 );
   lb_to->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

   ct_to          = us_counter( 3, 0.0, 0.0 ); // Update range upon load
   ct_to->setSingleStep( 1 );
   
   // Exclude and Include pushbuttons
   pb_excludeRange = us_pushbutton( tr( "Exclude Scan Range" ), false );
   pb_exclusion    = us_pushbutton( tr( "Exclusion Profile" ),  false );
   pb_edit1       = us_pushbutton( tr( "Edit Single Scan" ), false );
   pb_include     = us_pushbutton( tr( "Include All" ), false );

   // Edit controls 
   QLabel* lb_edit = us_banner( tr( "Edit Controls" ) );

   // Edit Triple:Speed display (Equilibrium only)
   lb_edtrsp      = us_label( tr( "Edit Triple:Speed :" ), -1, true );
   le_edtrsp      = us_lineedit( "" );
   lb_edtrsp->setVisible(  false );
   le_edtrsp->setVisible(  false );

   // Meniscus
   pb_meniscus    = us_pushbutton( tr( "Specify Meniscus" ), false );
   le_meniscus    = us_lineedit( "", 1, true);
   lb_meniscus    = us_label(      tr( "Meniscus:" ), -1 );

   // Air Gap (hidden by default)
   pb_airGap = us_pushbutton( tr( "Specify Air Gap" ), false );
   lb_airGap = us_label(      tr( "Air Gap:" ), -1 );
   le_airGap = us_lineedit( "", 1, true );
   pb_airGap->setVisible( false );
   lb_airGap->setHidden( true );
   le_airGap->setHidden( true );

   // Data range
   pb_dataRange   = us_pushbutton( tr( "Specify Data Range" ), false );
   le_dataRange   = us_lineedit( "", 1, true );
pb_dataRange->setVisible(false);
le_dataRange->setVisible(false);
   // Plateau
   pb_plateau     = us_pushbutton( tr( "Specify Plateau" ), false );
pb_plateau->setVisible(false);
   le_plateau     = us_lineedit( "", 1, true );
   // Baseline
   lb_baseline    = us_label(      tr( "Baseline:" ), -1 );
   le_baseline    = us_lineedit( "", 1, true );

//*NEW STUFF
//QLabel* 
   lb_dataStart   = us_label(      tr( "Data Start:" ), -1 );
//QLineEdit* 
   le_dataStart   = us_lineedit( "", 1, true );
//QPushButton* 
   //pb_dataEnd     = us_pushbutton( tr( "Specify Range/End:" ), false );
   pb_dataEnd     = us_pushbutton( tr( "Specify Top/Bottom:" ), false );

   lb_dataEnd     = us_label(      tr( "Data End:" ), -1 );
//QLineEdit* 
   le_dataEnd     = us_lineedit( "", 1, true );
//QLabel* 
   lb_plateau     = us_label(      tr( "Plateau:" ), -1 );
//QPushButton* 
   pb_nextChan    = us_pushbutton( tr( "Next Triple" ), false );
   pb_priorChan   = us_pushbutton( tr( "Previous Triple" ), false );
   
//*NEW STUFF
   // OD Limit
   lb_odlim       = us_label( tr( "OD Limit:" ), -1 );
   odlimit        = 1.8;
   ct_odlim       = us_counter( 3, 0.1, 50000.0, odlimit );
   ct_odlim ->setFont( font );
   ct_odlim ->setSingleStep( 0.01 );
   ct_odlim ->setMinimumWidth( lwid );
   ct_odlim ->resize( rhgt, swid );

   // Noise, Residuals, Invert, Spikes, Prior, Undo
   pb_noise       = us_pushbutton( tr( "Determine RI Noise" ),        false );
   pb_residuals   = us_pushbutton( tr( "Subtract Noise" ),            false );
   pb_invert      = us_pushbutton( tr( "Invert Sign" ),               false );
   pb_spikes      = us_pushbutton( tr( "Remove Spikes" ),             false );
   pb_priorEdits  = us_pushbutton( tr( "Apply Prior Edits" ),         false );
   pb_undo        = us_pushbutton( tr( "Undo Noise and Spikes" ),     false );

   // Review, Next Triple, Float, Save, Save-all
   pb_reviewep    = us_pushbutton( tr( "Review Edit Profile" ),       false );
   pb_nexteqtr    = us_pushbutton( tr( "Next Eq. Triple" ),           false );
   pb_reviewep->setVisible( false );
   pb_nexteqtr->setVisible( false );
   pb_float       = us_pushbutton( tr( "Mark Data as Floating" ),     false );

   //pb_write       = us_pushbutton( tr( "Save Current Edit Profile" ), false );
   pb_write       = us_pushbutton( tr( "Save Edit Profiles" ), false );
   pb_emanual     = us_pushbutton( tr( "Edit Currently Selected Profile Manually" ), false );
   
   lo_writemwl    = us_checkbox  ( tr( "Save to all Wavelengths" ),
                                   ck_writemwl, true );

   connect( pb_excludeRange, SIGNAL( clicked() ), SLOT( exclude_range() ) );
   connect( pb_details,      SIGNAL( clicked() ), SLOT( details()       ) );
   connect( pb_report,       SIGNAL( clicked() ), SLOT( view_report()   ) );
   connect( pb_investigator, SIGNAL( clicked() ),
                             SLOT  ( sel_investigator()         ) );
   connect( pb_load,         SIGNAL( clicked() ), SLOT( load()  ) );
   connect( cb_triple,       SIGNAL( currentIndexChanged( int ) ), 
                             SLOT  ( new_triple_auto         ( int ) ) );
   connect( pb_exclusion,    SIGNAL( clicked() ), SLOT( exclusion()     ) );
   connect( pb_edit1,        SIGNAL( clicked() ), SLOT( edit_scan()     ) );
   connect( pb_include,      SIGNAL( clicked() ), SLOT( include()       ) );
   connect( pb_meniscus,     SIGNAL( clicked() ), SLOT( set_meniscus()  ) );
   connect( pb_airGap,       SIGNAL( clicked() ), SLOT( set_airGap()    ) );
//   connect( pb_dataRange,    SIGNAL( clicked() ), SLOT( set_dataRange() ) );
//   connect( pb_plateau,      SIGNAL( clicked() ), SLOT( set_plateau()   ) );
   connect( pb_dataEnd,      SIGNAL( clicked() ), SLOT( set_dataRange() ) );
   connect( ct_odlim,        SIGNAL( valueChanged   ( double ) ),
                             SLOT  ( od_radius_limit( double ) ) );
   connect( pb_noise,        SIGNAL( clicked() ), SLOT( noise() ) );
   connect( pb_residuals,    SIGNAL( clicked() ),
                             SLOT  ( subtract_residuals() ) );
   connect( pb_invert,       SIGNAL( clicked() ), SLOT( invert_values() ) );
   connect( pb_spikes,       SIGNAL( clicked() ), SLOT( remove_spikes_auto() ) );
   connect( pb_priorEdits,   SIGNAL( clicked() ), SLOT( apply_prior()   ) );
   connect( pb_undo,         SIGNAL( clicked() ), SLOT( undo_auto()      ) );
   connect( pb_reviewep,     SIGNAL( clicked() ), SLOT( review_edits()  ) );
   connect( pb_nexteqtr,     SIGNAL( clicked() ), SLOT( next_triple()   ) );

   connect( pb_nextChan,     SIGNAL( clicked() ), SLOT( next_triple_auto()   ) );
   connect( pb_priorChan,    SIGNAL( clicked() ), SLOT( prior_triple_auto()  ) );
   
   connect( pb_float,        SIGNAL( clicked() ), SLOT( floating()  ) );
   connect( pb_write,        SIGNAL( clicked() ), SLOT( write_auto()  ) );
   connect( pb_emanual,      SIGNAL( clicked() ), SLOT( manual_edit_auto()  ) );

   connect ( this, SIGNAL( process_next_optics () ), SLOT ( process_optics_auto () )  );
   
   // Lay out specs widgets and layouts
   int s_row = 0;
   specs->addWidget( pb_investigator, s_row,   0, 1, 2 );
   specs->addWidget( le_investigator, s_row++, 2, 1, 4 );
   //specs->addLayout( disk_controls,   s_row++, 0, 1, 6 );
   specs->addWidget( pb_load,         s_row,   0, 1, 2 );
   specs->addWidget( pb_details,      s_row,   2, 1, 2 );
   specs->addWidget( pb_report,       s_row++, 4, 1, 2 );

   specs->addWidget( lb_cell,         s_row++, 0, 1, 6 );
   specs->addWidget( lb_triple,       s_row,   0, 1, 3 );
//   specs->addWidget( cb_triple,       s_row++, 3, 1, 3 );
//*NEW STUFF
   specs->addWidget( cb_triple,       s_row++, 3, 1, 3 );

   specs->addWidget( pb_priorChan,     s_row,   0, 1, 3 );    //<--- ALEXEY
   specs->addWidget( pb_nextChan,     s_row++, 3, 1, 3 );
   
//*NEW STUFF
   specs->addWidget( lb_rpms,         s_row,   0, 1, 3 );
   specs->addWidget( cb_rpms,         s_row++, 3, 1, 3 );
   specs->addWidget( lb_gaps,         s_row,   0, 1, 3 );
   specs->addWidget( ct_gaps,         s_row++, 3, 1, 3 );
   specs->addWidget( le_lxrng,        s_row++, 0, 1, 6 );
   specs->addWidget( lb_mwlctl,       s_row++, 0, 1, 6 );
   //specs->addLayout( lo_lrange,       s_row,   0, 1, 3 );   <-- ALEXEY 
   //specs->addLayout( lo_custom,       s_row++, 3, 1, 3 );
   specs->addWidget( lb_ldelta,       s_row,   0, 1, 2 );
   specs->addWidget( ct_ldelta,       s_row,   2, 1, 1 );
   specs->addWidget( le_ltrng,        s_row++, 3, 1, 3 );
   specs->addWidget( lb_lstart,       s_row,   0, 1, 2 );
   specs->addWidget( cb_lstart,       s_row,   2, 1, 1 );
   specs->addWidget( lb_lend,         s_row,   3, 1, 2 );
   specs->addWidget( cb_lend,         s_row++, 5, 1, 1 );
   specs->addWidget( pb_custom,       s_row,   0, 1, 3 );
   specs->addWidget( pb_incall,       s_row++, 3, 1, 3 );
   //specs->addLayout( lo_radius,       s_row,   0, 1, 3 );    <-- ALEXEY 
   //specs->addLayout( lo_waveln,       s_row++, 3, 1, 3 );
   specs->addWidget( lb_lplot,        s_row,   0, 1, 2 );
   specs->addWidget( cb_lplot,        s_row,   2, 1, 1 );
   specs->addWidget( pb_larrow,       s_row,   3, 1, 2 );
   specs->addWidget( pb_rarrow,       s_row++, 5, 1, 1 );
   specs->addWidget( lb_scan,         s_row++, 0, 1, 6 );
   specs->addWidget( lb_from,         s_row,   0, 1, 3 );
   specs->addWidget( ct_from,         s_row++, 3, 1, 3 );
   specs->addWidget( lb_to,           s_row,   0, 1, 3 );
   specs->addWidget( ct_to,           s_row++, 3, 1, 3 );
   specs->addWidget( pb_excludeRange, s_row,   0, 1, 3 );
   specs->addWidget( pb_exclusion,    s_row++, 3, 1, 3 );
   specs->addWidget( pb_edit1,        s_row,   0, 1, 3 );
   specs->addWidget( pb_include,      s_row++, 3, 1, 3 );
   specs->addWidget( lb_edit,         s_row++, 0, 1, 6 );
   specs->addWidget( lb_edtrsp,       s_row,   0, 1, 3 );
   specs->addWidget( le_edtrsp,       s_row++, 3, 1, 3 );
//   specs->addWidget( pb_meniscus,     s_row,   0, 1, 3 );
//   specs->addWidget( le_meniscus,     s_row++, 3, 1, 3 );

   // Meniscus
   specs->addWidget( lb_meniscus,     s_row,   0, 1, 3 );
   specs->addWidget( le_meniscus,     s_row++, 3, 1, 3 );
   
   specs->addWidget( pb_meniscus,     s_row++, 3, 1, 3 );

   // ALEXEY: for auto Interference
  
   specs->addWidget( lb_airGap,       s_row,   0, 1, 3 );
   specs->addWidget( le_airGap,       s_row++, 3, 1, 3 );
   specs->addWidget( pb_airGap,       s_row,   0, 1, 3 );

//   specs->addWidget( pb_dataRange,    s_row,   0, 1, 3 );
//   specs->addWidget( le_dataRange,    s_row++, 3, 1, 3 );
//   specs->addWidget( pb_plateau,      s_row,   0, 1, 3 );
//   specs->addWidget( le_plateau,      s_row++, 3, 1, 3 );
//   specs->addWidget( lb_baseline,     s_row,   0, 1, 3 );
//   specs->addWidget( le_baseline,     s_row++, 3, 1, 3 );
//*NEW STUFF

   //Data Start
   specs->addWidget( lb_dataStart,    s_row,   0, 1, 3 );
   specs->addWidget( le_dataStart,    s_row++, 3, 1, 3 );

   //Data End
   specs->addWidget( lb_dataEnd,      s_row,   0, 1, 3 );
   specs->addWidget( pb_dataEnd,      s_row,   3, 1, 2 );
   specs->addWidget( le_dataEnd,      s_row++, 3, 1, 3 );

   //Baseline
   specs->addWidget( lb_baseline,     s_row,   0, 1, 3 );
   specs->addWidget( le_baseline,     s_row++, 3, 1, 3 );

   //Plateau
   specs->addWidget( lb_plateau,      s_row,   0, 1, 3 );
   specs->addWidget( le_plateau,      s_row++, 3, 1, 3 );

   //OD limits
   specs->addWidget( lb_odlim,        s_row,   0, 1, 3 );
   specs->addWidget( ct_odlim,        s_row++, 3, 1, 3 );

   //Noise/Undo/Save
   specs->addWidget( pb_spikes,       s_row++, 0, 1, 6 );
   specs->addWidget( pb_undo,         s_row++, 0, 1, 6 );
   specs->addWidget( pb_emanual,      s_row++, 0, 1, 6 );
   specs->addWidget( pb_write,        s_row++, 0, 1, 6 );
   
   //*NEW STUFF
//   specs->addWidget( lb_odlim,        s_row,   0, 1, 3 );
//   specs->addWidget( ct_odlim,        s_row++, 3, 1, 3 );

/*
   specs->addWidget( lb_odlim,        s_row,   0, 1, 3 );
   specs->addWidget( ct_odlim,        s_row++, 3, 1, 3 );
   specs->addWidget( pb_noise,        s_row,   0, 1, 3 );
   specs->addWidget( pb_residuals,    s_row++, 3, 1, 3 );
   specs->addWidget( pb_invert,       s_row,   0, 1, 3 );
   specs->addWidget( pb_spikes,       s_row++, 3, 1, 3 );
   specs->addWidget( pb_priorEdits,   s_row,   0, 1, 3 );
   specs->addWidget( pb_undo,         s_row++, 3, 1, 3 );
   specs->addWidget( pb_reviewep,     s_row,   0, 1, 3 );
   specs->addWidget( pb_nexteqtr,     s_row++, 3, 1, 3 );
   specs->addWidget( pb_float,        s_row,   0, 1, 3 );
   specs->addWidget( pb_write,        s_row++, 3, 1, 3 );
   specs->addLayout( lo_writemwl,     s_row++, 3, 1, 3 );
*/

   // Button rows
   QBoxLayout*  buttons   = new QHBoxLayout;
   QPushButton* pb_reset  = us_pushbutton( tr( "Reset" ) );
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );

   connect( pb_reset,  SIGNAL( clicked() ), SLOT( reset() ) );
   connect( pb_help,   SIGNAL( clicked() ), SLOT( help()  ) );
   connect( pb_accept, SIGNAL( clicked()    ),
            this,      SLOT  ( close_edit() ) );

   buttons->addWidget( pb_reset );
   buttons->addWidget( pb_help );
   buttons->addWidget( pb_accept );

   // Hide some buttons
   if ( auto_mode.toStdString() == "AUTO")
     {
       
       pb_investigator->hide();
       le_investigator->hide();
       pb_load        ->hide(); 
       pb_details     ->hide();
       pb_report      ->hide();
       lb_gaps        ->hide();
       ct_gaps        ->hide();

       
       lb_scan        ->hide();
       lb_from        ->hide();
       lb_to          ->hide();
       ct_from        ->hide();
       ct_to          ->hide();
       pb_excludeRange->hide();
       pb_exclusion   ->hide();
       pb_edit1       ->hide();
       pb_include     ->hide();

       pb_meniscus    ->hide();
       pb_dataRange   ->hide();

       pb_airGap      ->hide();  

       pb_noise       ->hide();
       pb_residuals   ->hide();
       pb_invert      ->hide();
       pb_priorEdits  ->hide();
       pb_float       ->hide();
       
       
       pb_reset       ->hide();
       pb_help        ->hide();
       pb_accept      ->hide();

       //MWL
       lb_mwlctl      ->hide();
       lb_ldelta      ->hide();
       ct_ldelta      ->hide();
       lb_lstart      ->hide();
       lb_lend        ->hide();
       //lb_lplot       ->hide();
       //cb_lplot       ->hide();
       cb_lstart      ->hide();
       cb_lend        ->hide();
       le_ltrng       ->hide();
       le_lxrng       ->hide();
       pb_custom      ->hide();
       pb_incall      ->hide();
       //pb_larrow      ->hide();
       //pb_rarrow      ->hide();
       //lo_writemwl    ->hide();
       ck_writemwl    ->hide();
	 
       rb_lrange      ->hide();
       rb_custom      ->hide();
       rb_radius      ->hide();
       rb_waveln      ->hide();

     }
   

   // Plot layout on right side of window
   plot = new US_Plot( data_plot, 
         tr( "Absorbance Data" ),
         tr( "Radius (in cm)" ), tr( "Absorbance" ),
         true, "", "rainbow" );
   
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

   // Display status
   QHBoxLayout* statInfo = new QHBoxLayout();

   QLabel* lb_status = us_label(      tr( "Status:" ) );
   le_status         = us_lineedit(   tr( "(no data loaded)" ), 1, true );
   QPalette stpal;
   stpal.setColor( QPalette::Text, Qt::white );
   stpal.setColor( QPalette::Base, Qt::blue  );
   le_status->setPalette( stpal );
   
   statInfo->addWidget( lb_status );
   statInfo->addWidget( le_status );
   left->addLayout( statInfo );

   
   main->addLayout( left );
   main->addLayout( plot );
   main->setStretchFactor( left, 2 );
   main->setStretchFactor( plot, 3 );
   top ->addLayout( main );
   
   reset();

   setMinimumSize( 950, 450 );
   adjustSize();
 
   // TESTING...
   QMap < QString, QString > details;
   
   // // Data WITH existing Aprofile corresponding to existing protocol!!!
   // details[ "invID_passed" ] = QString("77");
   // details[ "filename" ]     = QString("JohnsonC_DNA-control_013020-run680");
   // details[ "protocolName" ] = QString("JohnsonC_DNA-control_013020");
   //  /****************************************************************************************/

   // // Interference Data WITH existing Aprofile corresponding to existing protocol!!!
   // details[ "invID_passed" ] = QString("6");
   // details[ "filename" ]     = QString("Comproject-itf-031220-run1176");
   // details[ "protocolName" ] = QString("Comproject-itf-031220");
   //   //  /****************************************************************************************/

   // // Interference Data WITH existing Aprofile corresponding to existing protocol!!!
   // details[ "invID_passed" ] = QString("6");
   // details[ "filename" ]     = QString("5-itf-test-run1179");
   // details[ "protocolName" ] = QString("5-itf-test");
   //   //  /****************************************************************************************/

    // // Interference Data WITH existing Aprofile corresponding to existing protocol!!!
    // details[ "invID_passed" ] = QString("6");
    // details[ "filename" ]     = QString("001-Amy-MWL-if-test-3-run794-IP");
    // details[ "protocolName" ] = QString("001-Amy-MWL-if-test-3");
    // //  /****************************************************************************************/
   

   // details[ "invID_passed" ] = QString("40");
   // details[ "filename" ]     = QString("KulkarniJ_NP025-D2O-0-20-17K_091220-run1285");
   // details[ "protocolName" ] = QString("KulkarniJ_NP025-D2O-0-20-17K_091220");


   // /* Yu: */
   // details[ "invID_passed" ] = QString("86");
   // details[ "filename" ]     = QString("Yu_ABC-ala-indivdual_111320-run869");
   // details[ "protocolName" ] = QString("Yu_ABC-ala-indivdual_111320");  

   // details[ "invID_passed" ] = QString("2");
   // details[ "filename" ]     = ("test-021621-IF-RI-B-run953-IP");
   // details[ "protocolName" ] = QString("test-021621-IF-RI-B");  

   // details[ "invID_passed" ] = QString("2");
   // //details[ "filename" ]     = ("test-021721-IF-RI-B-run955-IP");
   // details[ "filename" ]     = ("test-021721-IF-RI-B-run955-RI");
   // details[ "protocolName" ] = QString("test-021721-IF-RI-B");

   // details[ "invID_passed" ] = QString("2");
   // details[ "filename" ]     = ("test-021421-IF-RI-B_alexey-run974-IP");
   // details[ "protocolName" ] = QString("test-021421-IF-RI-B_alexey");

   // //MWV from demeler9
   // details[ "invID_passed" ] = QString("2");
   // details[ "filename" ]     = QString("Demo-033121-1-run1391");
   // details[ "protocolName" ] = QString("Demo-033121-1");

   // // /* A.S.: 2A/B[245,270,280];  MWV from demeler6*/
   // details[ "invID_passed" ] = QString("12");
   // details[ "filename" ]     = QString("SavelyevA_BSA_082520-run1276");
   // details[ "protocolName" ] = QString("SavelyevA_BSA_082520");

   // // details[ "invID_passed" ] = QString("97");
   // // details[ "filename" ]     = QString("GabirH_NetUnc-050921_MW-run1442");
   // // details[ "protocolName" ] = QString("GabirH_NetUnc-050921_MW");

   // details[ "invID_passed" ] = QString("104");
   // details[ "filename" ]     = QString("DubnauD_ComEA-14pbDNA_5-1_100621-run1233");
   // details[ "protocolName" ] = QString("DubnauD_ComEA-14pbDNA_5-1_100621");

   // details[ "invID_passed" ] = QString("77");
   // details[ "filename" ]     = QString("Johnson_C_EWS_10uM_20uM_NaPi_MES_01-13-22-run1242");
   // details[ "protocolName" ] = QString("Johnson_C_EWS_10uM_20uM_NaPi_MES_01-13-22");


   // //Borries: combined RI + IP run
   // details[ "invID_passed" ] = QString("2");
   // details[ "filename" ]     = QString("MW-AUC-IF_test_031622-run1270-IP,MW-AUC-IF_test_031622-run1270-RI");
   // details[ "protocolName" ] = QString("MW-AUC-IF_test_031622");

   // // //Run: lazeB_AUC3_PRS4-3_Day7_082522-run1372 (missinf EDIT PROFILES)
   // details[ "invID_passed" ] = QString("154");
   // details[ "filename" ]     = QString("BlazeB_AUC3_PRS4-3_Day7_082522-run1372");
   // details[ "protocolName" ] = QString("BlazeB_AUC3_PRS4-3_Day7_082522");
   // details[ "statusID" ]     = QString("46");
   // details[ "autoflowID" ]   = QString("671");

   // details[ "invID_passed" ] = QString("166");
   // details[ "filename" ]     = QString("HuberS_bCAll-DNSA_012623-run1879");
   // details[ "protocolName" ] = QString("HuberS_bCAll-DNSA_012623");
   // details[ "statusID" ]     = QString("116");
   // details[ "autoflowID" ]   = QString("814");

   // details[ "invID_passed" ] = QString("6");
   // details[ "filename" ]     = QString("AAV396_CsCl-46kRpm_21APRIL23-run1916");
   // details[ "protocolName" ] = QString("AAV396_CsCl-46kRpm_21APRIL23");
   // details[ "statusID" ]     = QString("148");
   // details[ "autoflowID" ]   = QString("866");

   // details[ "invID_passed" ] = QString("165");
   // details[ "filename" ]     = QString("eGFP-DNA-MW-08OCT23-run1981");
   // details[ "protocolName" ] = QString("eGFP-DNA-MW-08OCT23");
   // details[ "statusID" ]     = QString("231");
   // details[ "autoflowID" ]   = QString("1002");
   // details[ "runID" ]        = QString("1981");
   // details[ "OptimaName" ]   = QString("Optima 1");

   // details[ "invID_passed" ] = QString("165");
   // details[ "filename" ]     = QString("GMPMeniscusFitTest_6MAY24-run2023");
   // details[ "protocolName" ] = QString("GMPMeniscusFitTest_6MAY24v2");
   // details[ "statusID" ]     = QString("334");
   // details[ "autoflowID" ]   = QString("1140");
   // details[ "runID" ]        = QString("2023");
   // details[ "OptimaName" ]   = QString("Optima 1");
   // details[ "expType" ]      = QString("VELOCITY");  

   // details[ "invID_passed" ] = QString("165");
   // details[ "filename" ]     = QString("ABDE-Test-052124-run1693");
   // details[ "protocolName" ] = QString("ABDE-Test-052124-v2");
   // details[ "statusID" ]     = QString("341");
   // details[ "autoflowID" ]   = QString("1152");
   // details[ "runID" ]        = QString("1693");
   // details[ "OptimaName" ]   = QString("Optima 2");  
   // details[ "expType" ]      = QString("ABDE");  
   
   // load_auto( details );
  
}


// AUTO: Constructor for manual processing 
US_Edit::US_Edit( QVector< US_DataIO::RawData > allData, QStringList  triples,
		  QString  workingDir, int currenChtInd, int plotind ) : US_Widgets()
{
 
   check        = US_Images::getIcon( US_Images::CHECK );
   invert       = 1.0;
   all_edits    = false;
   men_1click   = US_Settings::debug_match( "men2click" ) ? false : true;
   total_speeds = 0;
   total_edits  = 0;
   v_line       = NULL;
   dbg_level    = US_Settings::us_debug();
   dbP          = NULL;
   chlamb       = QChar( 955 );
   gap_thresh   = 50.0;
   gap_fringe   = 0.4;
   bottom       = 0.0;
DbgLv(1) << " 0)gap_fringe" << gap_fringe;

   us_edit_auto_mode = false;
   us_edit_auto_mode_manual = true;
   all_loaded = false;
   is_spike_auto = false;
 
//usmode = false;
 
   setWindowTitle( tr( "Edit UltraScan Data Manually" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* top = new QVBoxLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );

   // Put the Run Info across the entire window
   QHBoxLayout* runInfo = new QHBoxLayout();
   QLabel* lb_info = us_label( tr( "Run Info:" ), -1 );
   runInfo->addWidget( lb_info );

   le_info = us_lineedit( "", 1, true );
   runInfo->addWidget( le_info );

   top->addLayout( runInfo );

   QHBoxLayout* main = new QHBoxLayout();
   QVBoxLayout* left = new QVBoxLayout;

   // Start of Grid Layout
   QGridLayout* specs = new QGridLayout;

   // Investigator
   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );

   if ( US_Settings::us_inv_level() < 3 )
      pb_investigator->setEnabled( false );

   int     id      = US_Settings::us_inv_ID();
   QString number  = ( id > 0 ) ? 
      QString::number( US_Settings::us_inv_ID() ) + ": " 
      : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(),
                                  1, true );

   // Disk/DB control
   disk_controls   = new US_Disk_DB_Controls;

   // Load
   QPushButton*
      pb_load      = us_pushbutton( tr( "Load Data" ) );
   pb_details      = us_pushbutton( tr( "Run Details" ), false );
      pb_report    = us_pushbutton( tr( "View Report" ), false );

   // Triple and Speed Step
   lb_triple       = us_label( tr( "Cell / Channel / Wavelength" ), -1 );
   cb_triple       = us_comboBox();
   lb_rpms         = us_label( tr( "Speed Step (RPM) of triple" ), -1 );
   cb_rpms         = us_comboBox();
   lb_rpms->setVisible( false );
   cb_rpms->setVisible( false );

   // Scan Gaps
   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   lb_gaps         = us_label( tr( "Threshold for Scan Gaps" ), -1 );
   ct_gaps         = us_counter( 1, 10.0, 100.0 );
   ct_gaps->setSingleStep ( 10.0 );
   ct_gaps->setValue( 50.0 );

   // MWL Control
   QFontMetrics fmet( font );
   int fwid = fmet.maxWidth();
   int rhgt = ct_gaps->height();
   int lwid = fwid * 4;
   int swid = lwid + fwid;
   lb_mwlctl       = us_banner( tr( "Wavelength Controls" ) );
   QButtonGroup* r_group = new QButtonGroup( this );
   QButtonGroup* x_group = new QButtonGroup( this );
   lo_lrange       = us_radiobutton( tr( "Lambda Range" ),       rb_lrange,
                                     true  );
   lo_custom       = us_radiobutton( tr( "Custom Lambda List" ), rb_custom,
                                     false );
   rb_lrange->setFont( font );
   rb_custom->setFont( font );
   r_group->addButton( rb_lrange, 0 );
   r_group->addButton( rb_custom, 1 );
   lb_ldelta       = us_label( tr( "%1 Index Increment:" ).arg( chlamb ), -1 );
   ct_ldelta       = us_counter( 1, 1, 100, 1 );
   ct_ldelta->setFont( font );
   ct_ldelta->setSingleStep( 1 );
   ct_ldelta->setMinimumWidth( lwid );
   ct_ldelta->resize( rhgt, swid );
   lb_lstart       = us_label( tr( "%1 Start:" ).arg( chlamb ), -1 );
   lb_lend         = us_label( tr( "%1 End:"   ).arg( chlamb ), -1 );
   lb_lplot        = us_label( tr( "Plot (W nm):" ), -1 );

   int     nlmbd   = 224;
   int     lmbdlo  = 251;
   int     lmbdhi  = 650;
   int     lmbddl  = 1;
   QString lrsmry  = tr( "%1 raw: %2 %3 to %4" )
      .arg( nlmbd ).arg( chlamb ).arg( lmbdlo ).arg( lmbdhi );
   le_ltrng  = us_lineedit( lrsmry, -1, true );
   QString lxsmry = tr( "%1 MWL exports: %2 %3 to %4, raw index increment %5" )
      .arg( nlmbd ).arg( chlamb ).arg( lmbdlo ).arg( lmbdhi ).arg( lmbddl );
   le_lxrng       = us_lineedit( lxsmry, -1, true );
   cb_lplot       = us_comboBox();
   cb_lstart      = us_comboBox();
   cb_lend        = us_comboBox();

   cb_lplot ->setFont( font );
   cb_lstart->setFont( font );
   cb_lend  ->setFont( font );

   pb_custom      = us_pushbutton(  tr( "Custom Lambdas" ),      false, -1 );
   pb_incall      = us_pushbutton(  tr( "Include All Lambdas" ), true,  -1 );
   pb_larrow      = us_pushbutton(  tr( "previous" ), true, -2 );
   pb_rarrow      = us_pushbutton(  tr( "next"     ), true, -2 );
   pb_larrow->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT ) );
   pb_rarrow->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );

   lo_radius      = us_radiobutton( tr( "x axis Radius" ),     rb_radius,
                                    true  );
   lo_waveln      = us_radiobutton( tr( "x axis Wavelength" ), rb_waveln,
                                    false );
   rb_radius->setFont( font );
   rb_waveln->setFont( font );
   x_group->addButton( rb_radius, 0 );
   x_group->addButton( rb_waveln, 1 );

   QStringList lambdas;
lambdas << "250" << "350" << "450" << "550" << "580" << "583" << "650";
   cb_lplot ->addItems( lambdas );
   cb_lstart->addItems( lambdas );
   cb_lend  ->addItems( lambdas );

   cb_lplot ->setCurrentIndex( 2 );
   cb_lstart->setCurrentIndex( 0 );
   cb_lend  ->setCurrentIndex( 6 );

   connect_mwl_ctrls( true );

   // Scan Controls
   QLabel* lb_scan = us_banner( tr( "Scan Controls" ) );
   
   // Scans
   QLabel* lb_from = us_label(  tr( "Scan Focus from:" ), -1 );
   lb_from->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

   ct_from        = us_counter( 3, 0.0, 0.0 ); // Update range upon load
   ct_from->setSingleStep( 1 );

   QLabel* lb_to  = us_label( tr( "to:" ), -1 );
   lb_to->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

   ct_to          = us_counter( 3, 0.0, 0.0 ); // Update range upon load
   ct_to->setSingleStep( 1 );
   
   // Exclude and Include pushbuttons
   pb_excludeRange = us_pushbutton( tr( "Exclude Scan Range" ), false );
   pb_exclusion    = us_pushbutton( tr( "Exclusion Profile" ),  false );
   pb_edit1       = us_pushbutton( tr( "Edit Single Scan" ), false );
   pb_include     = us_pushbutton( tr( "Include All" ), false );



   // Edit controls 
   QLabel* lb_edit = us_banner( tr( "Edit Controls" ) );

   // Edit Triple:Speed display (Equilibrium only)
   lb_edtrsp      = us_label( tr( "Edit Triple:Speed :" ), -1, true );
   le_edtrsp      = us_lineedit( "" );
   lb_edtrsp->setVisible(  false );
   le_edtrsp->setVisible(  false );

   // Meniscus
   pb_meniscus    = us_pushbutton( tr( "Specify Meniscus" ), false );
   le_meniscus    = us_lineedit( "", 1 );
   lb_meniscus    = us_label(      tr( "Meniscus:" ), -1 );

   // Air Gap (hidden by default)
   pb_airGap = us_pushbutton( tr( "Specify Air Gap" ), false );
   lb_airGap = us_label(      tr( "Air Gap:" ), -1 );
   le_airGap = us_lineedit( "", 1, true );
   pb_airGap->setVisible( false );
   lb_airGap->setHidden( true );
   le_airGap->setHidden( true );

   
   // // Air Gap (hidden by default)
   // pb_airGap = us_pushbutton( tr( "Specify Air Gap" ), false );
   // le_airGap = us_lineedit( "", 1, true );
   // pb_airGap->setHidden( true );
   // le_airGap->setHidden( true );

   // Data range
   pb_dataRange   = us_pushbutton( tr( "Specify Data Range" ), false );
   le_dataRange   = us_lineedit( "", 1, true );
pb_dataRange->setVisible(false);
le_dataRange->setVisible(false);
   // Plateau
   pb_plateau     = us_pushbutton( tr( "Specify Plateau" ), false );
pb_plateau->setVisible(false);
   le_plateau     = us_lineedit( "", 1, true );
   // Baseline
   lb_baseline    = us_label(      tr( "Baseline:" ), -1 );
   le_baseline    = us_lineedit( "", 1, true );

//*NEW STUFF
//QLabel* 
   lb_dataStart   = us_label(      tr( "Data Start:" ), -1 );
//QLineEdit* 
   le_dataStart   = us_lineedit( "", 1, true );
//QPushButton* 
   //pb_dataEnd     = us_pushbutton( tr( "Specify Range/End:" ), false );
   pb_dataEnd     = us_pushbutton( tr( "Specify Top/Bottom:" ), false );
//QLineEdit* 
   le_dataEnd     = us_lineedit( "", 1, false );
//QLabel* 
   lb_plateau     = us_label(      tr( "Plateau:" ), -1 );
//QPushButton* 
   pb_nextChan    = us_pushbutton( tr( "Next Triple" ), false );
//*NEW STUFF
   // OD Limit
   lb_odlim       = us_label( tr( "OD Limit:" ), -1 );
   odlimit        = 1.8;
   ct_odlim       = us_counter( 3, 0.1, 50000.0, odlimit );
   ct_odlim ->setFont( font );
   ct_odlim ->setSingleStep( 0.01 );
   ct_odlim ->setMinimumWidth( lwid );
   ct_odlim ->resize( rhgt, swid );

   // Noise, Residuals, Invert, Spikes, Prior, Undo
   pb_noise       = us_pushbutton( tr( "Determine RI Noise" ),        false );
   pb_residuals   = us_pushbutton( tr( "Subtract Noise" ),            false );
   pb_invert      = us_pushbutton( tr( "Invert Sign" ),               false );
   pb_spikes      = us_pushbutton( tr( "Remove Spikes" ),             false );
   pb_priorEdits  = us_pushbutton( tr( "Apply Prior Edits" ),         false );
   pb_undo        = us_pushbutton( tr( "Undo Noise and Spikes" ),     false );

   // Review, Next Triple, Float, Save, Save-all
   pb_reviewep    = us_pushbutton( tr( "Review Edit Profile" ),       false );
   pb_nexteqtr    = us_pushbutton( tr( "Next Eq. Triple" ),           false );
   pb_reviewep->setVisible( false );
   pb_nexteqtr->setVisible( false );
   pb_float       = us_pushbutton( tr( "Mark Data as Floating" ),     false );
   pb_write       = us_pushbutton( tr( "Save Current Edit Profile" ), false );
   lo_writemwl    = us_checkbox  ( tr( "Save to all Wavelengths" ),
                                   ck_writemwl, true );

   connect( pb_excludeRange, SIGNAL( clicked() ), SLOT( exclude_range() ) );
   connect( pb_details,      SIGNAL( clicked() ), SLOT( details()       ) );
   connect( pb_report,       SIGNAL( clicked() ), SLOT( view_report()   ) );
   connect( pb_investigator, SIGNAL( clicked() ),
                             SLOT  ( sel_investigator()         ) );
   connect( pb_load,         SIGNAL( clicked() ), SLOT( load()  ) );
   connect( cb_triple,       SIGNAL( currentIndexChanged( int ) ), 
                             SLOT  ( new_triple         ( int ) ) );
   connect( pb_exclusion,    SIGNAL( clicked() ), SLOT( exclusion()     ) );
   connect( pb_edit1,        SIGNAL( clicked() ), SLOT( edit_scan()     ) );
   connect( pb_include,      SIGNAL( clicked() ), SLOT( include()       ) );
   connect( pb_meniscus,     SIGNAL( clicked() ), SLOT( set_meniscus()  ) );
   connect( pb_airGap,       SIGNAL( clicked() ), SLOT( set_airGap()    ) );
//   connect( pb_dataRange,    SIGNAL( clicked() ), SLOT( set_dataRange() ) );
//   connect( pb_plateau,      SIGNAL( clicked() ), SLOT( set_plateau()   ) );
   connect( pb_dataEnd,      SIGNAL( clicked() ), SLOT( set_dataRange() ) );
   connect( ct_odlim,        SIGNAL( valueChanged   ( double ) ),
                             SLOT  ( od_radius_limit( double ) ) );
   connect( pb_noise,        SIGNAL( clicked() ), SLOT( noise() ) );
   connect( pb_residuals,    SIGNAL( clicked() ),
                             SLOT  ( subtract_residuals() ) );
   connect( pb_invert,       SIGNAL( clicked() ), SLOT( invert_values() ) );
   connect( pb_spikes,       SIGNAL( clicked() ), SLOT( remove_spikes() ) );
   connect( pb_priorEdits,   SIGNAL( clicked() ), SLOT( apply_prior()   ) );
   connect( pb_undo,         SIGNAL( clicked() ), SLOT( undo()      ) );
   connect( pb_reviewep,     SIGNAL( clicked() ), SLOT( review_edits()  ) );
   connect( pb_nexteqtr,     SIGNAL( clicked() ), SLOT( next_triple()   ) );
   connect( pb_nextChan,     SIGNAL( clicked() ), SLOT( next_triple()   ) );
   connect( pb_float,        SIGNAL( clicked() ), SLOT( floating()  ) );
   connect( pb_write,        SIGNAL( clicked() ), SLOT( write()     ) );

   // Lay out specs widgets and layouts
   int s_row = 0;
   specs->addWidget( pb_investigator, s_row,   0, 1, 2 );
   specs->addWidget( le_investigator, s_row++, 2, 1, 4 );
   //specs->addLayout( disk_controls,   s_row++, 0, 1, 6 );
   specs->addWidget( pb_load,         s_row,   0, 1, 2 );
   specs->addWidget( pb_details,      s_row,   2, 1, 2 );
   specs->addWidget( pb_report,       s_row++, 4, 1, 2 );
   specs->addWidget( lb_triple,       s_row,   0, 1, 3 );
//   specs->addWidget( cb_triple,       s_row++, 3, 1, 3 );
//*NEW STUFF
   specs->addWidget( cb_triple,       s_row,   3, 1, 2 );
   specs->addWidget( pb_nextChan,     s_row++, 5, 1, 1 );
//*NEW STUFF
   specs->addWidget( lb_rpms,         s_row,   0, 1, 3 );
   specs->addWidget( cb_rpms,         s_row++, 3, 1, 3 );
   specs->addWidget( lb_gaps,         s_row,   0, 1, 3 );
   specs->addWidget( ct_gaps,         s_row++, 3, 1, 3 );
   specs->addWidget( le_lxrng,        s_row++, 0, 1, 6 );
   specs->addWidget( lb_mwlctl,       s_row++, 0, 1, 6 );
   specs->addLayout( lo_lrange,       s_row,   0, 1, 3 );
   specs->addLayout( lo_custom,       s_row++, 3, 1, 3 );
   specs->addWidget( lb_ldelta,       s_row,   0, 1, 2 );
   specs->addWidget( ct_ldelta,       s_row,   2, 1, 1 );
   specs->addWidget( le_ltrng,        s_row++, 3, 1, 3 );
   specs->addWidget( lb_lstart,       s_row,   0, 1, 2 );
   specs->addWidget( cb_lstart,       s_row,   2, 1, 1 );
   specs->addWidget( lb_lend,         s_row,   3, 1, 2 );
   specs->addWidget( cb_lend,         s_row++, 5, 1, 1 );
   specs->addWidget( pb_custom,       s_row,   0, 1, 3 );
   specs->addWidget( pb_incall,       s_row++, 3, 1, 3 );
   specs->addLayout( lo_radius,       s_row,   0, 1, 3 );
   specs->addLayout( lo_waveln,       s_row++, 3, 1, 3 );
   specs->addWidget( lb_lplot,        s_row,   0, 1, 2 );
   specs->addWidget( cb_lplot,        s_row,   2, 1, 1 );
   specs->addWidget( pb_larrow,       s_row,   3, 1, 2 );
   specs->addWidget( pb_rarrow,       s_row++, 5, 1, 1 );
   specs->addWidget( lb_scan,         s_row++, 0, 1, 6 );
   specs->addWidget( lb_from,         s_row,   0, 1, 3 );
   specs->addWidget( ct_from,         s_row++, 3, 1, 3 );
   specs->addWidget( lb_to,           s_row,   0, 1, 3 );
   specs->addWidget( ct_to,           s_row++, 3, 1, 3 );
   specs->addWidget( pb_excludeRange, s_row,   0, 1, 3 );
   specs->addWidget( pb_exclusion,    s_row++, 3, 1, 3 );
   specs->addWidget( pb_edit1,        s_row,   0, 1, 3 );
   specs->addWidget( pb_include,      s_row++, 3, 1, 3 );
   specs->addWidget( lb_edit,         s_row++, 0, 1, 6 );
   specs->addWidget( lb_edtrsp,       s_row,   0, 1, 3 );
   specs->addWidget( le_edtrsp,       s_row++, 3, 1, 3 );
//   specs->addWidget( pb_meniscus,     s_row,   0, 1, 3 );
//   specs->addWidget( le_meniscus,     s_row++, 3, 1, 3 );
   specs->addWidget( lb_meniscus,     s_row,   0, 1, 1 );
   specs->addWidget( le_meniscus,     s_row,   1, 1, 2 );
   specs->addWidget( pb_meniscus,     s_row++, 3, 1, 3 );
   specs->addWidget( pb_airGap,       s_row,   0, 1, 3 );
   specs->addWidget( le_airGap,       s_row++, 3, 1, 3 );
//   specs->addWidget( pb_dataRange,    s_row,   0, 1, 3 );
//   specs->addWidget( le_dataRange,    s_row++, 3, 1, 3 );
//   specs->addWidget( pb_plateau,      s_row,   0, 1, 3 );
//   specs->addWidget( le_plateau,      s_row++, 3, 1, 3 );
//   specs->addWidget( lb_baseline,     s_row,   0, 1, 3 );
//   specs->addWidget( le_baseline,     s_row++, 3, 1, 3 );
//*NEW STUFF
   specs->addWidget( lb_dataStart,    s_row,   0, 1, 1 );
   specs->addWidget( le_dataStart,    s_row,   1, 1, 2 );
   specs->addWidget( pb_dataEnd,      s_row,   3, 1, 2 );
   specs->addWidget( le_dataEnd,      s_row++, 5, 1, 1 );
   specs->addWidget( lb_baseline,     s_row,   0, 1, 1 );
   specs->addWidget( le_baseline,     s_row,   1, 1, 3 );
   specs->addWidget( lb_plateau,      s_row,   4, 1, 1 );
   specs->addWidget( le_plateau,      s_row++, 5, 1, 1 );
//*NEW STUFF
//   specs->addWidget( lb_odlim,        s_row,   0, 1, 3 );
//   specs->addWidget( ct_odlim,        s_row++, 3, 1, 3 );
   specs->addWidget( lb_odlim,        s_row,   0, 1, 3 );
   specs->addWidget( ct_odlim,        s_row++, 3, 1, 3 );
   specs->addWidget( pb_noise,        s_row,   0, 1, 3 );
   specs->addWidget( pb_residuals,    s_row++, 3, 1, 3 );
   specs->addWidget( pb_invert,       s_row,   0, 1, 3 );
   specs->addWidget( pb_spikes,       s_row++, 3, 1, 3 );
   specs->addWidget( pb_priorEdits,   s_row,   0, 1, 3 );
   specs->addWidget( pb_undo,         s_row++, 3, 1, 3 );
   specs->addWidget( pb_reviewep,     s_row,   0, 1, 3 );
   specs->addWidget( pb_nexteqtr,     s_row++, 3, 1, 3 );
   specs->addWidget( pb_float,        s_row,   0, 1, 3 );
   specs->addWidget( pb_write,        s_row++, 3, 1, 3 );
   specs->addLayout( lo_writemwl,     s_row++, 3, 1, 3 );

   // Button rows
   QBoxLayout*  buttons   = new QHBoxLayout;
   // QPushButton* pb_reset  = us_pushbutton( tr( "Reset" ) );
   // QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   // QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );

   // connect( pb_reset,  SIGNAL( clicked() ), SLOT( reset() ) );
   // connect( pb_help,   SIGNAL( clicked() ), SLOT( help()  ) );
   // connect( pb_accept, SIGNAL( clicked()    ),
   //          this,      SLOT  ( close_edit() ) );

   // buttons->addWidget( pb_reset );
   // buttons->addWidget( pb_help );
   // buttons->addWidget( pb_accept );

   QPushButton* pb_cancel  = us_pushbutton( tr( "Cancel" ) );
   pb_pass    = us_pushbutton( tr( "Accept Changes for a Channel" ), false );
   
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( close()  ) );
   connect( pb_pass,   SIGNAL( clicked()    ),
	    this,      SLOT  ( pass_values() ) );
   
   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_pass );



   // -- Hide && || disable some buttons ----

   cb_triple      -> setEnabled( false );
   pb_investigator->hide();
   le_investigator->hide();
   pb_load        ->hide(); 
   pb_details     ->hide();
   pb_report      ->hide();
   lb_gaps        ->hide();
   ct_gaps        ->hide();
   pb_nextChan    ->hide();
   
   lb_scan        ->hide();
   lb_from        ->hide();
   lb_to          ->hide();
   ct_from        ->hide();
   ct_to          ->hide();
   pb_excludeRange->hide();
   pb_exclusion   ->hide();
   pb_edit1       ->hide();
   pb_include     ->hide();

   // pb_meniscus    ->hide();
   // pb_dataRange   ->hide();
   // pb_airGap      ->hide();  

   pb_noise       ->hide();
   pb_residuals   ->hide();
   pb_invert      ->hide();
   pb_priorEdits  ->hide();
   pb_float       ->hide();

   pb_spikes      ->hide();
   pb_undo        ->hide();
   pb_write       ->hide();
   
   // pb_reset       ->hide();
   // pb_help        ->hide();
   // pb_accept      ->hide();
   
   //---------------------
   

   

   // Plot layout on right side of window
   plot = new US_Plot( data_plot, 
         tr( "Absorbance Data" ),
         tr( "Radius (in cm)" ), tr( "Absorbance" ),
         true, "", "rainbow" );
   
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
   main->setStretchFactor( left, 2 );
   main->setStretchFactor( plot, 3 );
   top ->addLayout( main );

   qDebug() << "US_Edit manual setup 1";
   
   reset();

   qDebug() << "US_Edit manual setup 2";

   //Load already selected data
   this->allData    = allData;
   this->triples    = triples;
   this->workingDir = workingDir;

   qDebug() << "US_Edit manual setup 3";
   load_manual_auto();
   qDebug() << "US_Edit manual setup 4";
   
   //pre-select channel passed from main window:
   cb_triple->setCurrentIndex( currenChtInd );
   qDebug() << "US_Edit manual setup 5";
   new_triple( currenChtInd );
   qDebug() << "US_Edit manual setup 6";
   cb_lplot ->setCurrentIndex( plotind );
   qDebug() << "US_Edit manual setup 7";
   show_mwl_controls( false );
   qDebug() << "US_Edit manual setup 8";
}


// Constructor
US_Edit::US_Edit() : US_Widgets()
{
   check        = US_Images::getIcon( US_Images::CHECK );
   invert       = 1.0;
   all_edits    = false;
   men_1click   = US_Settings::debug_match( "men2click" ) ? false : true;
   total_speeds = 0;
   total_edits  = 0;
   v_line       = NULL;
   dbg_level    = US_Settings::us_debug();
   dbP          = NULL;
   chlamb       = QChar( 955 );
   gap_thresh   = 50.0;
   gap_fringe   = 0.4;
   bottom       = 0.0;
DbgLv(1) << " 0)gap_fringe" << gap_fringe;

   us_edit_auto_mode = false;
   us_edit_auto_mode_manual = false;
   all_loaded = false;
   is_spike_auto = false;
 
//usmode = false;
 
   setWindowTitle( tr( "Edit UltraScan Data" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* top = new QVBoxLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );

   // Put the Run Info across the entire window
   QHBoxLayout* runInfo = new QHBoxLayout();
   QLabel* lb_info = us_label( tr( "Run Info:" ), -1 );
   runInfo->addWidget( lb_info );

   le_info = us_lineedit( "", 1, true );
   runInfo->addWidget( le_info );

   top->addLayout( runInfo );

   QHBoxLayout* main = new QHBoxLayout();
   QVBoxLayout* left = new QVBoxLayout;

   // Start of Grid Layout
   QGridLayout* specs = new QGridLayout;

   // Investigator
   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );

   if ( US_Settings::us_inv_level() < 3 )
      pb_investigator->setEnabled( false );

   int     id      = US_Settings::us_inv_ID();
   QString number  = ( id > 0 ) ? 
      QString::number( US_Settings::us_inv_ID() ) + ": " 
      : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(),
                                  1, true );

   // Disk/DB control
   disk_controls   = new US_Disk_DB_Controls;

   // Load
   QPushButton*
      pb_load      = us_pushbutton( tr( "Load Data" ) );
   pb_details      = us_pushbutton( tr( "Run Details" ), false );
      pb_report    = us_pushbutton( tr( "View Report" ), false );

   // Triple and Speed Step
   lb_triple       = us_label( tr( "Cell / Channel / Wavelength" ), -1 );
   cb_triple       = us_comboBox();
   lb_rpms         = us_label( tr( "Speed Step (RPM) of triple" ), -1 );
   cb_rpms         = us_comboBox();
   lb_rpms->setVisible( false );
   cb_rpms->setVisible( false );

   // Scan Gaps
   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   lb_gaps         = us_label( tr( "Threshold for Scan Gaps" ), -1 );
   ct_gaps         = us_counter( 1, 10.0, 100.0 );
   ct_gaps->setSingleStep ( 10.0 );
   ct_gaps->setValue( 50.0 );

   // MWL Control
   QFontMetrics fmet( font );
   int fwid = fmet.maxWidth();
   int rhgt = ct_gaps->height();
   int lwid = fwid * 4;
   int swid = lwid + fwid;
   lb_mwlctl       = us_banner( tr( "Wavelength Controls" ) );
   QButtonGroup* r_group = new QButtonGroup( this );
   QButtonGroup* x_group = new QButtonGroup( this );
   lo_lrange       = us_radiobutton( tr( "Lambda Range" ),       rb_lrange,
                                     true  );
   lo_custom       = us_radiobutton( tr( "Custom Lambda List" ), rb_custom,
                                     false );
   rb_lrange->setFont( font );
   rb_custom->setFont( font );
   r_group->addButton( rb_lrange, 0 );
   r_group->addButton( rb_custom, 1 );
   lb_ldelta       = us_label( tr( "%1 Index Increment:" ).arg( chlamb ), -1 );
   ct_ldelta       = us_counter( 1, 1, 100, 1 );
   ct_ldelta->setFont( font );
   ct_ldelta->setSingleStep( 1 );
   ct_ldelta->setMinimumWidth( lwid );
   ct_ldelta->resize( rhgt, swid );
   lb_lstart       = us_label( tr( "%1 Start:" ).arg( chlamb ), -1 );
   lb_lend         = us_label( tr( "%1 End:"   ).arg( chlamb ), -1 );
   lb_lplot        = us_label( tr( "Plot (W nm):" ), -1 );

   int     nlmbd   = 224;
   int     lmbdlo  = 251;
   int     lmbdhi  = 650;
   int     lmbddl  = 1;
   QString lrsmry  = tr( "%1 raw: %2 %3 to %4" )
      .arg( nlmbd ).arg( chlamb ).arg( lmbdlo ).arg( lmbdhi );
   le_ltrng  = us_lineedit( lrsmry, -1, true );
   QString lxsmry = tr( "%1 MWL exports: %2 %3 to %4, raw index increment %5" )
      .arg( nlmbd ).arg( chlamb ).arg( lmbdlo ).arg( lmbdhi ).arg( lmbddl );
   le_lxrng       = us_lineedit( lxsmry, -1, true );
   cb_lplot       = us_comboBox();
   cb_lstart      = us_comboBox();
   cb_lend        = us_comboBox();

   cb_lplot ->setFont( font );
   cb_lstart->setFont( font );
   cb_lend  ->setFont( font );

   pb_custom      = us_pushbutton(  tr( "Custom Lambdas" ),      false, -1 );
   pb_incall      = us_pushbutton(  tr( "Include All Lambdas" ), true,  -1 );
   pb_larrow      = us_pushbutton(  tr( "previous" ), true, -2 );
   pb_rarrow      = us_pushbutton(  tr( "next"     ), true, -2 );
   pb_larrow->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT ) );
   pb_rarrow->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );

   lo_radius      = us_radiobutton( tr( "x axis Radius" ),     rb_radius,
                                    true  );
   lo_waveln      = us_radiobutton( tr( "x axis Wavelength" ), rb_waveln,
                                    false );
   rb_radius->setFont( font );
   rb_waveln->setFont( font );
   x_group->addButton( rb_radius, 0 );
   x_group->addButton( rb_waveln, 1 );

   QStringList lambdas;
lambdas << "250" << "350" << "450" << "550" << "580" << "583" << "650";
   cb_lplot ->addItems( lambdas );
   cb_lstart->addItems( lambdas );
   cb_lend  ->addItems( lambdas );

   cb_lplot ->setCurrentIndex( 2 );
   cb_lstart->setCurrentIndex( 0 );
   cb_lend  ->setCurrentIndex( 6 );

   connect_mwl_ctrls( true );

   // Scan Controls
   QLabel* lb_scan = us_banner( tr( "Scan Controls" ) );
   
   // Scans
   QLabel* lb_from = us_label(  tr( "Scan Focus from:" ), -1 );
   lb_from->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

   ct_from        = us_counter( 3, 0.0, 0.0 ); // Update range upon load
   ct_from->setSingleStep( 1 );

   QLabel* lb_to  = us_label( tr( "to:" ), -1 );
   lb_to->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

   ct_to          = us_counter( 3, 0.0, 0.0 ); // Update range upon load
   ct_to->setSingleStep( 1 );
   
   // Exclude and Include pushbuttons
   pb_excludeRange = us_pushbutton( tr( "Exclude Scan Range" ), false );
   pb_exclusion    = us_pushbutton( tr( "Exclusion Profile" ),  false );
   pb_edit1       = us_pushbutton( tr( "Edit Single Scan" ), false );
   pb_include     = us_pushbutton( tr( "Include All" ), false );

   // Edit controls 
   QLabel* lb_edit = us_banner( tr( "Edit Controls" ) );

   // Edit Triple:Speed display (Equilibrium only)
   lb_edtrsp      = us_label( tr( "Edit Triple:Speed :" ), -1, true );
   le_edtrsp      = us_lineedit( "" );
   lb_edtrsp->setVisible(  false );
   le_edtrsp->setVisible(  false );

   // Meniscus
   pb_meniscus    = us_pushbutton( tr( "Specify Meniscus" ), false );
   le_meniscus    = us_lineedit( "", 1 );
   lb_meniscus    = us_label(      tr( "Meniscus:" ), -1 );

   // Air Gap (hidden by default)
   pb_airGap = us_pushbutton( tr( "Specify Air Gap" ), false );
   lb_airGap = us_label(      tr( "Air Gap:" ), -1 );
   le_airGap = us_lineedit( "", 1, true );
   pb_airGap->setVisible( false );
   lb_airGap->setHidden( true );
   le_airGap->setHidden( true );

   // Data range
   pb_dataRange   = us_pushbutton( tr( "Specify Data Range" ), false );
   le_dataRange   = us_lineedit( "", 1, true );
pb_dataRange->setVisible(false);
le_dataRange->setVisible(false);
   // Plateau
   pb_plateau     = us_pushbutton( tr( "Specify Plateau" ), false );
pb_plateau->setVisible(false);
   le_plateau     = us_lineedit( "", 1, true );
   // Baseline
   lb_baseline    = us_label(      tr( "Baseline:" ), -1 );
   le_baseline    = us_lineedit( "", 1, true );

//*NEW STUFF
//QLabel* 
   lb_dataStart   = us_label(      tr( "Data Start:" ), -1 );
//QLineEdit* 
   le_dataStart   = us_lineedit( "", 1, true );
//QPushButton* 
   //pb_dataEnd     = us_pushbutton( tr( "Specify Range/End:" ), false );
   pb_dataEnd     = us_pushbutton( tr( "Specify Top/Bottom:" ), false );
//QLineEdit* 
   le_dataEnd     = us_lineedit( "", 1, false );
//QLabel* 
   lb_plateau     = us_label(      tr( "Plateau:" ), -1 );
//QPushButton* 
   pb_nextChan    = us_pushbutton( tr( "Next Triple" ), false );
//*NEW STUFF
   // OD Limit
   lb_odlim       = us_label( tr( "OD Limit:" ), -1 );
   odlimit        = 1.8;
   ct_odlim       = us_counter( 3, 0.1, 50000.0, odlimit );
   ct_odlim ->setFont( font );
   ct_odlim ->setSingleStep( 0.01 );
   ct_odlim ->setMinimumWidth( lwid );
   ct_odlim ->resize( rhgt, swid );

   // Noise, Residuals, Invert, Spikes, Prior, Undo
   pb_noise       = us_pushbutton( tr( "Determine RI Noise" ),        false );
   pb_residuals   = us_pushbutton( tr( "Subtract Noise" ),            false );
   pb_invert      = us_pushbutton( tr( "Invert Sign" ),               false );
   pb_spikes      = us_pushbutton( tr( "Remove Spikes" ),             false );
   pb_priorEdits  = us_pushbutton( tr( "Apply Prior Edits" ),         false );
   pb_undo        = us_pushbutton( tr( "Undo Noise and Spikes" ),     false );

   // Review, Next Triple, Float, Save, Save-all
   pb_reviewep    = us_pushbutton( tr( "Review Edit Profile" ),       false );
   pb_nexteqtr    = us_pushbutton( tr( "Next Eq. Triple" ),           false );
   pb_reviewep->setVisible( false );
   pb_nexteqtr->setVisible( false );
   pb_float       = us_pushbutton( tr( "Mark Data as Floating" ),     false );
   pb_write       = us_pushbutton( tr( "Save Current Edit Profile" ), false );
   lo_writemwl    = us_checkbox  ( tr( "Save to all Wavelengths" ),
                                   ck_writemwl, true );

   connect( pb_excludeRange, SIGNAL( clicked() ), SLOT( exclude_range() ) );
   connect( pb_details,      SIGNAL( clicked() ), SLOT( details()       ) );
   connect( pb_report,       SIGNAL( clicked() ), SLOT( view_report()   ) );
   connect( pb_investigator, SIGNAL( clicked() ),
                             SLOT  ( sel_investigator()         ) );
   connect( pb_load,         SIGNAL( clicked() ), SLOT( load()  ) );
   connect( cb_triple,       SIGNAL( currentIndexChanged( int ) ), 
                             SLOT  ( new_triple         ( int ) ) );
   connect( pb_exclusion,    SIGNAL( clicked() ), SLOT( exclusion()     ) );
   connect( pb_edit1,        SIGNAL( clicked() ), SLOT( edit_scan()     ) );
   connect( pb_include,      SIGNAL( clicked() ), SLOT( include()       ) );
   connect( pb_meniscus,     SIGNAL( clicked() ), SLOT( set_meniscus()  ) );
   connect( pb_airGap,       SIGNAL( clicked() ), SLOT( set_airGap()    ) );
//   connect( pb_dataRange,    SIGNAL( clicked() ), SLOT( set_dataRange() ) );
//   connect( pb_plateau,      SIGNAL( clicked() ), SLOT( set_plateau()   ) );
   connect( pb_dataEnd,      SIGNAL( clicked() ), SLOT( set_dataRange() ) );
   connect( ct_odlim,        SIGNAL( valueChanged   ( double ) ),
                             SLOT  ( od_radius_limit( double ) ) );
   connect( pb_noise,        SIGNAL( clicked() ), SLOT( noise() ) );
   connect( pb_residuals,    SIGNAL( clicked() ),
                             SLOT  ( subtract_residuals() ) );
   connect( pb_invert,       SIGNAL( clicked() ), SLOT( invert_values() ) );
   connect( pb_spikes,       SIGNAL( clicked() ), SLOT( remove_spikes() ) );
   connect( pb_priorEdits,   SIGNAL( clicked() ), SLOT( apply_prior()   ) );
   connect( pb_undo,         SIGNAL( clicked() ), SLOT( undo()      ) );
   connect( pb_reviewep,     SIGNAL( clicked() ), SLOT( review_edits()  ) );
   connect( pb_nexteqtr,     SIGNAL( clicked() ), SLOT( next_triple()   ) );
   connect( pb_nextChan,     SIGNAL( clicked() ), SLOT( next_triple()   ) );
   connect( pb_float,        SIGNAL( clicked() ), SLOT( floating()  ) );
   connect( pb_write,        SIGNAL( clicked() ), SLOT( write()     ) );

   // Lay out specs widgets and layouts
   int s_row = 0;
   specs->addWidget( pb_investigator, s_row,   0, 1, 2 );
   specs->addWidget( le_investigator, s_row++, 2, 1, 4 );
   specs->addLayout( disk_controls,   s_row++, 0, 1, 6 );
   specs->addWidget( pb_load,         s_row,   0, 1, 2 );
   specs->addWidget( pb_details,      s_row,   2, 1, 2 );
   specs->addWidget( pb_report,       s_row++, 4, 1, 2 );
   specs->addWidget( lb_triple,       s_row,   0, 1, 3 );
//   specs->addWidget( cb_triple,       s_row++, 3, 1, 3 );
//*NEW STUFF
   specs->addWidget( cb_triple,       s_row,   3, 1, 2 );
   specs->addWidget( pb_nextChan,     s_row++, 5, 1, 1 );
//*NEW STUFF
   specs->addWidget( lb_rpms,         s_row,   0, 1, 3 );
   specs->addWidget( cb_rpms,         s_row++, 3, 1, 3 );
   specs->addWidget( lb_gaps,         s_row,   0, 1, 3 );
   specs->addWidget( ct_gaps,         s_row++, 3, 1, 3 );
   specs->addWidget( le_lxrng,        s_row++, 0, 1, 6 );
   specs->addWidget( lb_mwlctl,       s_row++, 0, 1, 6 );
   specs->addLayout( lo_lrange,       s_row,   0, 1, 3 );
   specs->addLayout( lo_custom,       s_row++, 3, 1, 3 );
   specs->addWidget( lb_ldelta,       s_row,   0, 1, 2 );
   specs->addWidget( ct_ldelta,       s_row,   2, 1, 1 );
   specs->addWidget( le_ltrng,        s_row++, 3, 1, 3 );
   specs->addWidget( lb_lstart,       s_row,   0, 1, 2 );
   specs->addWidget( cb_lstart,       s_row,   2, 1, 1 );
   specs->addWidget( lb_lend,         s_row,   3, 1, 2 );
   specs->addWidget( cb_lend,         s_row++, 5, 1, 1 );
   specs->addWidget( pb_custom,       s_row,   0, 1, 3 );
   specs->addWidget( pb_incall,       s_row++, 3, 1, 3 );
   specs->addLayout( lo_radius,       s_row,   0, 1, 3 );
   specs->addLayout( lo_waveln,       s_row++, 3, 1, 3 );
   specs->addWidget( lb_lplot,        s_row,   0, 1, 2 );
   specs->addWidget( cb_lplot,        s_row,   2, 1, 1 );
   specs->addWidget( pb_larrow,       s_row,   3, 1, 2 );
   specs->addWidget( pb_rarrow,       s_row++, 5, 1, 1 );
   specs->addWidget( lb_scan,         s_row++, 0, 1, 6 );
   specs->addWidget( lb_from,         s_row,   0, 1, 3 );
   specs->addWidget( ct_from,         s_row++, 3, 1, 3 );
   specs->addWidget( lb_to,           s_row,   0, 1, 3 );
   specs->addWidget( ct_to,           s_row++, 3, 1, 3 );
   specs->addWidget( pb_excludeRange, s_row,   0, 1, 3 );
   specs->addWidget( pb_exclusion,    s_row++, 3, 1, 3 );
   specs->addWidget( pb_edit1,        s_row,   0, 1, 3 );
   specs->addWidget( pb_include,      s_row++, 3, 1, 3 );
   specs->addWidget( lb_edit,         s_row++, 0, 1, 6 );
   specs->addWidget( lb_edtrsp,       s_row,   0, 1, 3 );
   specs->addWidget( le_edtrsp,       s_row++, 3, 1, 3 );
//   specs->addWidget( pb_meniscus,     s_row,   0, 1, 3 );
//   specs->addWidget( le_meniscus,     s_row++, 3, 1, 3 );
   specs->addWidget( lb_meniscus,     s_row,   0, 1, 1 );
   specs->addWidget( le_meniscus,     s_row,   1, 1, 2 );
   specs->addWidget( pb_meniscus,     s_row++, 3, 1, 3 );
   specs->addWidget( pb_airGap,       s_row,   0, 1, 3 );
   specs->addWidget( le_airGap,       s_row++, 3, 1, 3 );
//   specs->addWidget( pb_dataRange,    s_row,   0, 1, 3 );
//   specs->addWidget( le_dataRange,    s_row++, 3, 1, 3 );
//   specs->addWidget( pb_plateau,      s_row,   0, 1, 3 );
//   specs->addWidget( le_plateau,      s_row++, 3, 1, 3 );
//   specs->addWidget( lb_baseline,     s_row,   0, 1, 3 );
//   specs->addWidget( le_baseline,     s_row++, 3, 1, 3 );
//*NEW STUFF
   specs->addWidget( lb_dataStart,    s_row,   0, 1, 1 );
   specs->addWidget( le_dataStart,    s_row,   1, 1, 2 );
   specs->addWidget( pb_dataEnd,      s_row,   3, 1, 2 );
   specs->addWidget( le_dataEnd,      s_row++, 5, 1, 1 );
   specs->addWidget( lb_baseline,     s_row,   0, 1, 1 );
   specs->addWidget( le_baseline,     s_row,   1, 1, 3 );
   specs->addWidget( lb_plateau,      s_row,   4, 1, 1 );
   specs->addWidget( le_plateau,      s_row++, 5, 1, 1 );
//*NEW STUFF
//   specs->addWidget( lb_odlim,        s_row,   0, 1, 3 );
//   specs->addWidget( ct_odlim,        s_row++, 3, 1, 3 );
   specs->addWidget( lb_odlim,        s_row,   0, 1, 3 );
   specs->addWidget( ct_odlim,        s_row++, 3, 1, 3 );
   specs->addWidget( pb_noise,        s_row,   0, 1, 3 );
   specs->addWidget( pb_residuals,    s_row++, 3, 1, 3 );
   specs->addWidget( pb_invert,       s_row,   0, 1, 3 );
   specs->addWidget( pb_spikes,       s_row++, 3, 1, 3 );
   specs->addWidget( pb_priorEdits,   s_row,   0, 1, 3 );
   specs->addWidget( pb_undo,         s_row++, 3, 1, 3 );
   specs->addWidget( pb_reviewep,     s_row,   0, 1, 3 );
   specs->addWidget( pb_nexteqtr,     s_row++, 3, 1, 3 );
   specs->addWidget( pb_float,        s_row,   0, 1, 3 );
   specs->addWidget( pb_write,        s_row++, 3, 1, 3 );
   specs->addLayout( lo_writemwl,     s_row++, 3, 1, 3 );

   // Button rows
   QBoxLayout*  buttons   = new QHBoxLayout;
   QPushButton* pb_reset  = us_pushbutton( tr( "Reset" ) );
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );

   connect( pb_reset,  SIGNAL( clicked() ), SLOT( reset() ) );
   connect( pb_help,   SIGNAL( clicked() ), SLOT( help()  ) );
   connect( pb_accept, SIGNAL( clicked()    ),
            this,      SLOT  ( close_edit() ) );

   buttons->addWidget( pb_reset );
   buttons->addWidget( pb_help );
   buttons->addWidget( pb_accept );

   // Plot layout on right side of window
   plot = new US_Plot( data_plot, 
         tr( "Absorbance Data" ),
         tr( "Radius (in cm)" ), tr( "Absorbance" ),
         true, "", "rainbow" );
   
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
   main->setStretchFactor( left, 2 );
   main->setStretchFactor( plot, 3 );
   top ->addLayout( main );

   reset();
}


// void US_Edit::us_mode_passed( void )
// {
//   qDebug() << "US_Edit:   US_MODE SIGNAL: ";
//   usmode = true;
// }

// Select DB investigator
void US_Edit::sel_investigator( void )
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
void US_Edit::reset( void )
{

  qDebug() << "reset 1";
   changes_made = false;
   floatingData = false;

   step          = MENISCUS;
   meniscus      = 0.0;
   meniscus_left = 0.0;
   airGap_left   = 0.0;
   airGap_right  = 9.0;
   range_left    = 0.0;
   range_right   = 9.0;
   plateau       = 0.0;
   baseline      = 0.0;
   baseline_od   = 0.0;
   bottom        = 0.0;
   invert        = 1.0;  // Multiplier = 1.0 or -1.0
   noise_order   = 0;
   triple_index  = 0;
   data_index    = 0;
   isMwl         = false;
   xaxis_radius  = true;
   lsel_range    = true;

   le_info     ->setText( "" );
   le_meniscus ->setText( "" );
   le_airGap   ->setText( "" );
//   le_dataRange->setText( "" );
   le_dataStart->setText( "" );
   le_dataEnd  ->setText( "" );
   le_plateau  ->setText( "" );
   le_baseline ->setText( "" );

   lb_gaps->setText( tr( "Threshold for Scan Gaps" ) );
   ct_gaps->setValue( 50.0 );

   ct_from->disconnect();
   ct_from->setMinimum( 0 );
   ct_from->setMaximum( 0 );
   ct_from->setValue  ( 0 );

   ct_to->disconnect();
   ct_to->setMinimum( 0 );
   ct_to->setMaximum( 0 );
   ct_to->setValue  ( 0 );

   cb_triple->disconnect();

   qDebug() << "reset 2";
   
   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   data_plot->detachItems( QwtPlotItem::Rtti_PlotMarker );
   v_line = NULL;
   pick     ->disconnect();

   qDebug() << "reset 3";
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   grid = us_grid( data_plot );
   qDebug() << "reset 4";
   data_plot->replot();
   qDebug() << "reset 5";
   
   // Disable pushbuttons
   pb_details     ->setEnabled( false );

   pb_excludeRange->setEnabled( false );
   pb_exclusion   ->setEnabled( false );
   pb_include     ->setEnabled( false );
   pb_edit1       ->setEnabled( false );
   
   pb_meniscus    ->setEnabled( false );
   pb_airGap      ->setEnabled( false );
//   pb_dataRange   ->setEnabled( false );
//   pb_plateau     ->setEnabled( false );
   
   pb_noise       ->setEnabled( false );
   pb_residuals   ->setEnabled( false );
   pb_spikes      ->setEnabled( false );
   pb_invert      ->setEnabled( false );
   pb_priorEdits  ->setEnabled( false );
   pb_reviewep    ->setEnabled( false );
   pb_nexteqtr    ->setEnabled( false );
   pb_undo        ->setEnabled( false );
   
   pb_report      ->setEnabled( false );
   pb_float       ->setEnabled( false );

   if ( us_edit_auto_mode )
     pb_write       ->setEnabled( true );
   else
     pb_write       ->setEnabled( false );
   
   ck_writemwl    ->setEnabled( false );

   qDebug() << "reset 6";
   // Remove icons
   pb_meniscus    ->setIcon( QIcon() );
   pb_airGap      ->setIcon( QIcon() );
//   pb_dataRange   ->setIcon( QIcon() );
//   pb_plateau     ->setIcon( QIcon() );
   pb_noise       ->setIcon( QIcon() );
   pb_residuals   ->setIcon( QIcon() );
   pb_spikes      ->setIcon( QIcon() );
   pb_invert      ->setIcon( QIcon() );

   pb_float       ->setIcon( QIcon() );

   qDebug() << "reset 61";

   editLabel     .clear();
   data.scanData .clear();
   includes      .clear();
   changed_points.clear();
   trip_rpms     .clear();
   triples       .clear();
   cb_triple    ->clear();
   cb_rpms      ->disconnect();

   qDebug() << "reset 62";
   cb_rpms      ->clear();
   editGUIDs     .clear();
   editIDs       .clear();
   editFnames    .clear();
   files         .clear();
   expd_radii    .clear();
   expi_wvlns    .clear();
   rawi_wvlns    .clear();
   celchns       .clear();
   rawc_wvlns    .clear();
   expc_wvlns    .clear();
   expc_radii    .clear();
   connect_mwl_ctrls( false );
   rb_lrange    ->setChecked( true );
   rb_custom    ->setChecked( false );
   cb_lplot     ->clear();
   cb_lstart    ->clear();
   cb_lend      ->clear();
   ct_ldelta    ->setEnabled( true );
   cb_lend      ->setEnabled( true );
   rb_radius    ->setChecked( true );
   rb_waveln    ->setChecked( false );
   pb_custom    ->setEnabled( false );
   qDebug() << "reset 63";
   if (pb_airGap != nullptr)
   {
     pb_airGap->setHidden( true );
   }
   qDebug() << "reset 64";
   if (le_airGap != nullptr)
   {
     le_airGap->setHidden( true );
   }
   qDebug() << "reset 65";
   if (lb_airGap != nullptr)
   {
      lb_airGap->setHidden( true );
   }
   qDebug() << "reset 7";
   connect_mwl_ctrls( true );
   qDebug() << "reset 8";
   
   set_pbColors( NULL );
   lb_triple->setText( tr( "Cell / Channel / Wavelength" ) );
   qDebug() << "reset 9";

   show_mwl_controls( false );
   qDebug() << "reset 10";
}

// Reset parameters for a new triple
void US_Edit::reset_triple( void )
{
   step          = MENISCUS;
   meniscus      = 0.0;
   meniscus_left = 0.0;
   airGap_left   = 0.0;
   airGap_right  = 9.0;
   range_left    = 0.0;
   range_right   = 9.0;
   plateau       = 0.0;
   baseline      = 0.0;
   bottom        = 0.0;
   invert        = 1.0;  // Multiplier = 1.0 or -1.0
   noise_order   = 0;

   le_info     ->setText( "" );
   le_meniscus ->setText( "" );
   le_airGap   ->setText( "" );
//   le_dataRange->setText( "" );
   le_dataStart->setText( "" );
   le_dataEnd  ->setText( "" );
   le_plateau  ->setText( "" );
   le_baseline ->setText( "" );

   if ( dataType == "IP" )
      ct_gaps->setValue( 0.4 );
   else
      ct_gaps->setValue( 50.0 );

   ct_from->disconnect();
   ct_from->setMinimum( 0 );
   ct_from->setMaximum( 0 );
   ct_from->setValue  ( 0 );

   ct_to->disconnect();
   ct_to->setMinimum( 0 );
   ct_to->setMaximum( 0 );
   ct_to->setValue  ( 0 );

   data.scanData .clear();
   includes      .clear();
   changed_points.clear();
   trip_rpms     .clear();

   cb_triple    ->disconnect();
   cb_rpms      ->disconnect();
   cb_rpms      ->clear();
   ct_gaps      ->disconnect();
}

// Display run details
void US_Edit::details( void )
{  
   US_RunDetails2* dialog 
      = new US_RunDetails2( allData, runID, workingDir, triples );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

// Do a Gap check against threshold value
void US_Edit::gap_check( void )
{
   int threshold = (int)ct_gaps->value();
   gap_thresh    = ct_gaps->value();
            
   US_DataIO::Scan  s;
   QString          gaps;

   int              scanNumber    = 0;
   int              rawScanNumber = -1;
   bool             deleteAll     = false;

   foreach ( s, data.scanData )
   {
      rawScanNumber++;
      // If scan has been deleted, skip to next
      if ( ! includes.contains( scanNumber ) ) continue;

      int maxGap     = 0;
      int gapLength  = 0;
      int location   = 0;

      int leftPoint  = data.xindex( range_left  );
      int rightPoint = data.xindex( range_right );

      for ( int i = leftPoint; i <= rightPoint; i++ )
      {
         int byte = i / 8;
         int bit  = i % 8;

         if ( s.interpolated[ byte ]  &  1 << ( 7 - bit ) ) 
           gapLength++;
         else
           gapLength = 0;

         if ( gapLength > maxGap )
         {
            maxGap   = gapLength;
            location = i;
         }
      }

      if ( maxGap >= threshold )
      { 
         QwtPlotCurve* curve         = NULL;
         bool          deleteCurrent = false;

         // Hightlight scan
         ct_to->setValue( 0.0 );  // Unfocus everything

         QString         seconds = QString::number( s.seconds );
         QwtPlotItemList items   = data_plot->itemList();
         
         for ( int i = 0; i < items.size(); i++ )
         {
            if ( items[ i ]->rtti() == QwtPlotItem::Rtti_PlotCurve )
            {
               if ( items[ i ]->title().text().contains( seconds ) )
               {
                  curve = dynamic_cast< QwtPlotCurve* >( items[ i ] );
                  break;
               }
            }
         }

         if ( curve == NULL )
         {
            qDebug() << "Cannot find curve during gap check";
            return;
         }

         // Popup unless delete all is set
         if ( ! deleteAll )
         {
            // Color the selected point
            QPen   p = curve->pen();
            QBrush b = curve->brush();

            p.setColor( Qt::red );
            b.setColor( Qt::red );

            curve->setPen  ( p );
            curve->setBrush( b );
            data_plot->replot();

            // Ask the user what to do
            QMessageBox box;

            box.setWindowTitle( tr( "Excessive Scan Gaps Detected" ) );
            
            double radius = data.xvalues[ 0 ] + location * s.delta_r;
            
            gaps = tr( "Scan " ) 
                 + QString::number( rawScanNumber ) 
                 + tr( " has a maximum reading gap of " ) 
                 + QString::number( maxGap ) 
                 + tr( " starting at radius " ) 
                 + QString::number( radius, 'f', 3 );

            box.setText( gaps );
            box.setInformativeText( tr( "Delete?" ) );

            QPushButton* pb_delete = box.addButton( tr( "Delete" ), 
                  QMessageBox::YesRole );
         
            QPushButton* pb_deleteAll = box.addButton( tr( "Delete All" ), 
                  QMessageBox::AcceptRole );
         
            QPushButton* pb_skip = box.addButton( tr( "Skip" ), 
                  QMessageBox::NoRole );
         
            QPushButton* pb_cancel = box.addButton( tr( "Cancel" ), 
                  QMessageBox::RejectRole );
         
            box.setEscapeButton ( pb_cancel );
            box.setDefaultButton( pb_delete );

            box.exec();

            if ( box.clickedButton() == pb_delete )
               deleteCurrent = true;

            else if ( box.clickedButton() == pb_deleteAll )
            {
               deleteAll     = true;
               deleteCurrent = true;
            }

            else 
            {
               // Uncolor scan
               p.setColor( Qt::yellow );
               b.setColor( Qt::yellow );

               curve->setPen  ( p );
               curve->setBrush( b );
               data_plot->replot();
            }

            if ( box.clickedButton() == pb_skip )
               continue;
            
            if ( box.clickedButton() == pb_cancel )
               return;
         }

         // Delete the scan
         if ( deleteAll || deleteCurrent )
         {
            includes.removeOne( scanNumber );
            replot();

            ct_to  ->setMaximum( includes.size() );
            ct_from->setMaximum( includes.size() );
         }
      }
                             
      scanNumber++;
   }
}

// Load an AUC data set
void US_Edit::load_auto( QMap < QString, QString > & details_at_editing )
{
  triples_all_optics.clear();
  channels_all.clear();
  isSet_ref_wvl.clear();

  channels_to_analyse.clear();
  triple_to_edit.clear();
  triples_skip_analysis.clear();
  
  // analysis stages
  job1run     = false;
  job2run     = false;
  job3run     = false;
  job4run     = false;
  job5run     = false;
  job3auto    = false;
  job6run_pcsa= false;
  /////////////////////////
  
  details_at_editing_local = details_at_editing;
  
  le_status->setText( tr( "Loading data ..." ) );
  
  // QList< DataDesc_auto >  sdescs_auto;
  // datamap.clear();

  autoflowID_passed   = details_at_editing[ "autoflowID" ].toInt();
  idInv_auto          = details_at_editing[ "invID_passed" ];
  ProtocolName_auto   = details_at_editing[ "protocolName" ];
  autoflowStatusID    = details_at_editing[ "statusID" ].toInt();
  autoflow_expType    = details_at_editing[ "expType" ];

  dataSource          = details_at_editing[ "dataSource" ];
  simulated_data      = false;
  
  qDebug() << "autoflowID_passed, dataSource, ProtocolName_auto: "
	   << autoflowID_passed << dataSource << ProtocolName_auto;
  
  // Deal with different filenames if any.... //////////////////////////
  filename_runID_passed = details_at_editing[ "filename" ];
  runType_combined_IP_RI = false;

  runTypes_map.clear();
  
  if ( filename_runID_passed.contains(",") && filename_runID_passed.contains("IP") && filename_runID_passed.contains("RI") )
    {
      for ( int i = 0 ; i < filename_runID_passed.split(",").count(); ++i )
	{
	  runTypes_map.insert( filename_runID_passed.split(",")[i], 1 );

	  qDebug() << "Filename: " << filename_runID_passed.split(",")[i];
	}
      
      runType_combined_IP_RI = true;
    }
  else
    filename_runID_auto = filename_runID_passed;
    
  qDebug() << "runType_combined_IP_RI: " << runType_combined_IP_RI;

  //ALEXEY: create filename_runID_auto_base name
  if ( runType_combined_IP_RI )
    {
      QString filename_runID_auto_base_temp = filename_runID_passed.split(",")[0];
      int pos = filename_runID_auto_base_temp.lastIndexOf(QChar('-'));
      
      filename_runID_auto_base = filename_runID_auto_base_temp.left( pos );

      qDebug() << "IN EDIT - filename_base for combined runs: " << filename_runID_auto_base;
      
    }
    
  ///////////////////////////////////////////////////////////////////////
  
  qDebug() << "autoflowID, intensityID, autoflowStatusID : " << autoflowID_passed << details_at_editing[ "intensityID" ] << autoflowStatusID;
  qDebug() << "AT EDIT_DATA: filename, idInv: " << filename_runID_passed << ", " << idInv_auto;

  process_optics_auto();

  //Hide some gui elements for "ABDE"
  if ( autoflow_expType == "ABDE" )
    {
      lb_baseline -> hide();
      le_baseline -> hide();
      lb_plateau  -> hide();
      le_plateau  -> hide();

      lb_odlim    -> hide();
      ct_odlim    -> hide();
    }
  
}

//Slot to process optics type...
void US_Edit::process_optics_auto( )
{
  all_loaded = false;
  editProfile.clear();
  editProfile_scans_excl.clear();
  automatic_meniscus.clear();
  manual_edit_comments. clear();
  centerpieceParameters.clear();
  aprofileParameters.clear();
  iwavl_edit_ref.clear();
  iwavl_edit_ref_index.clear();
  triple_plot_first_time.clear();

  //abde
  edited_triples_abde. clear();
  
  //Read centerpiece names from protocol:
  centerpiece_names.clear();
  
  QList< DataDesc_auto >  sdescs_auto;
  datamap.clear();
  
  scan_db_auto();
  sdescs_auto  = datamap.values();
  load_db_auto( sdescs_auto );

  runID = workingDir.section( "/", -1, -1 );
DbgLv(1) << "Ld: runID" << runID << "wdir" << workingDir;
   sData     .clear();
   sd_offs   .clear();
   sd_knts   .clear();
   cb_triple->clear();
   files     .clear();
   
   if ( triples.size() == 0 )
   {
      QMessageBox::warning( this,
            tr( "No Files Found" ),
            tr( "There were no files of the form *.auc\n"  
                "found in the specified directory." ) );
      return;
   }

   qDebug() << "IN PROCESS OPTICS: triples_size() " << triples.size();
     
   cb_triple->addItems( triples );

   // Debug
   for (int index = 0; index < cb_triple->count(); index++)
     qDebug() << cb_triple->itemText(index);
   
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
                       SLOT  ( new_triple_auto    ( int ) ) );
   triple_index = 0;
   data_index   = 0;
   
   le_info->setText( runID );

   data     = allData[ 0 ];
   dataType = QString( QChar( data.type[ 0 ] ) ) 
            + QString( QChar( data.type[ 1 ] ) );

   if ( dataType == "IP" )
   {
      lb_gaps->setText( tr( "Fringe Tolerance" ) );

      ct_gaps->setRange     ( 0.0, 20.0 );
      ct_gaps->setSingleStep( 0.001 );
      ct_gaps->setValue     ( 0.4 );
      ct_gaps->setNumButtons( 3 );

      connect( ct_gaps, SIGNAL( valueChanged        ( double ) ), 
                        SLOT  ( set_fringe_tolerance( double ) ) );
   }
   else
   {
      lb_gaps->setText( tr( "Threshold for Scan Gaps" ) );
      
      ct_gaps->disconnect   ();
      ct_gaps->setRange     ( 10.0, 100.0 );
      ct_gaps->setSingleStep( 10.0 );
      ct_gaps->setValue     ( 50.0 );
      ct_gaps->setNumButtons( 1 );
   }

   QString runtype = runID + "." + dataType;
DbgLv(1) << "Ld: runtype" << runtype;
   nwaveln         = 0;
   ncelchn         = 0;
   outData.clear();

   qDebug() << "EDIT: triples -- " << triples;

   for ( int trx = 0; trx < triples.size(); trx++ )
   {  // Generate file names
      QString triple = QString( triples.at( trx ) ).replace( " / ", "." );
      QString file   = runtype + "." + triple + ".auc";
      files << file;

      qDebug() << "EDIT: file -- " << file;
      
      // Save pointers as initial output data vector
      outData << &allData[ trx ];

      QString scell  = triple.section( ".", 0, 0 ).simplified();
      QString schan  = triple.section( ".", 1, 1 ).simplified();
      QString swavl  = triple.section( ".", 2, 2 ).simplified();

      if ( ! rawc_wvlns.contains( swavl ) )
      {  // Accumulate wavelengths in case this is MWL
         nwaveln++;
         rawc_wvlns << swavl;
      }

      nwavelo         = nwaveln;
      QString celchn  = scell + " / " + schan;

      if ( ! celchns.contains( celchn ) )
      {  // Accumulate cell/channel values in case this is MWL
         ncelchn++;
         celchns << celchn;
      }
   }
DbgLv(1) << "rawc_wvlns size" << rawc_wvlns.size() << nwaveln;
DbgLv(1) << " celchns    size" << celchns.size() << ncelchn;
   QMap<int, QString> m;
   for (const auto& s : rawc_wvlns) m[s.toInt()] = s;
   rawc_wvlns = QStringList(m.values());
   rawi_wvlns.clear();
   toti_wvlns.clear();

   for ( int wvx = 0; wvx < nwaveln; wvx++ )
   {
      int iwavl    = rawc_wvlns[ wvx ].toInt();
      rawi_wvlns << iwavl;
      toti_wvlns << iwavl;
   }

   workingDir   = workingDir + "/";
   QString file = workingDir + runtype + ".xml";
   expType      = "";
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
      if ( dbP == NULL )
      {
         US_Passwd pw;
         dbP          = new US_DB2( pw.getPasswd() );

         if ( dbP == NULL  ||  dbP->lastErrno() != US_DB2::OK )
         {
            QMessageBox::warning( this, tr( "Connection Problem" ),
              tr( "Could not connect to database\n" )
              + ( ( dbP != NULL ) ? dbP->lastError() : "" ) );
            dbP          = NULL;
            return;
         }
      }

      QStringList query;
      query << "get_experiment_info_by_runID" << runID 
            << QString::number( US_Settings::us_inv_ID() );

      dbP->query( query );
      dbP->next();
      expType    = dbP->value( 8 ).toString();
   }

   if ( expType.isEmpty() )  // if no experiment type, assume Velocity
      expType    = "Velocity";

   else                      // insure Ulll... form, e.g., "Equilibrium"
      expType    = expType.left( 1 ).toUpper() +
                   expType.mid(  1 ).toLower();


   // Set booleans for experiment type
   expIsVelo  = ( expType.compare( "Velocity",    Qt::CaseInsensitive ) == 0 );
   expIsEquil = ( expType.compare( "Equilibrium", Qt::CaseInsensitive ) == 0 );
   expIsDiff  = ( expType.compare( "Diffusion",   Qt::CaseInsensitive ) == 0 );
   expIsOther = ( !expIsVelo  &&  !expIsEquil  &&  !expIsDiff );
   expType    = expIsOther ? "Other" : expType;
   odlimit    = 1.8;
   init_includes();

   if ( expIsEquil )
   {  // Equilibrium
      lb_rpms    ->setVisible( true  );
      cb_rpms    ->setVisible( true  );
      pb_plateau ->setVisible( false );
      le_plateau ->setVisible( false ); 
      lb_baseline->setVisible( false ); 
      le_baseline->setVisible( false ); 
      lb_edtrsp  ->setVisible( true  );
      le_edtrsp  ->setVisible( true  );
      pb_reviewep->setVisible( true  );
      pb_nexteqtr->setVisible( true  );
      //pb_write   ->setText( tr( "Save Edit Profiles" ) );

      sData.clear();
      US_DataIO::SpeedData  ssDat;
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
         data   = allData[ 0 ];
         ksd    = sd_knts[ 0 ];
         trip_rpms.clear();
         cb_rpms ->clear();
         for ( int ii = 0; ii < ksd; ii++ )
         {
            QString arpm = QString::number( sData[ ii ].speed );
            trip_rpms << arpm;
         }

         cb_rpms->addItems( trip_rpms );
      }

      pick     ->disconnect();
      connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                     SLOT  ( mouse   ( const QwtDoublePoint& ) ) );

      pb_priorEdits->disconnect();
      connect( pb_priorEdits, SIGNAL( clicked() ), SLOT( prior_equil() ) );
      plot_scan();

      connect( cb_rpms,   SIGNAL( currentIndexChanged( int ) ), 
                          SLOT  ( new_rpmval         ( int ) ) );
   }

   else
   {  // non-Equilibrium
      bool notMwl  = ( nwaveln < 3 );

      qDebug() << "notMWL, nwaveln: " << notMwl << ", " << nwaveln;
	
      lb_rpms    ->setVisible( false );
      cb_rpms    ->setVisible( false );
//      pb_plateau ->setVisible( true  );
      le_plateau ->setVisible( true  ); 
      // lb_baseline->setVisible( notMwl );
      // le_baseline->setVisible( notMwl );
      lb_baseline->setVisible( true );
      le_baseline->setVisible( true );     
      lb_edtrsp  ->setVisible( false );
      le_edtrsp  ->setVisible( false );
      pb_reviewep->setVisible( false );
      pb_nexteqtr->setVisible( false );

      //pb_write   ->setText( tr( "Save Current Edit Profile" ) );

      pb_priorEdits->disconnect();
      connect( pb_priorEdits, SIGNAL( clicked() ), SLOT( apply_prior() ) );
DbgLv(1) << "LD():  triples size" << triples.size();
      if ( notMwl )
         plot_current( 0 );
   }

   // Enable pushbuttons
   pb_details   ->setEnabled( true );
   pb_include   ->setEnabled( true );
   pb_exclusion ->setEnabled( true );
   pb_meniscus  ->setEnabled( true );
   pb_airGap    ->setEnabled( false );
//   pb_dataRange ->setEnabled( false );
//   pb_plateau   ->setEnabled( false );
   pb_noise     ->setEnabled( false );
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

   // Temperature check
   double              dt = 0.0;
   US_DataIO::RawData  triple;

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

   ntriple      = triples.size();
DbgLv(1) << " triples    size" << ntriple;
   editGUIDs .fill( "",     ntriple );
   editIDs   .fill( "",     ntriple );
   editFnames.fill( "none", ntriple );

   //isMwl        = ( nwaveln > 2  &&  ntriple > 16 );
   isMwl        = ( nwaveln > 1 );
   lrng_bycell  = false;         // Assume initially cell lambdas all the same
DbgLv(1) << "LD(): isMwl" << isMwl << "nwaveln" << nwaveln << toti_wvlns.size()
 << "ntriple" << ntriple;

   if ( isMwl )
   {  // Set values related to MultiWaveLength
      connect_mwl_ctrls( false );

      // Load the internal object that keeps track of MWL data
DbgLv(1) << "IS-MWL:   load_mwl begun";
      mwl_data.load_mwl( allData );
DbgLv(1) << "IS-MWL:   load_mwl complete";

      // Initial export lambdas are input lambdas: save the input lists
      ncelchn      = mwl_data.countOf( "cellchann" );
      maxwavl      = rawc_wvlns.size();
DbgLv(1) << "IS-MWL: max wvlns size" << maxwavl;
      wavelns_i.clear();

      for ( int ccx = 0; ccx < ncelchn; ccx++ )
      {  // Save lambdas for each cell; flag if any cell-to-cell differences
         QVector< int > wvs;
         nwavelo      = mwl_data.lambdas( wvs, ccx );
         std::sort( wvs.begin(), wvs.end());
         wavelns_i << wvs;

         if ( nwavelo != maxwavl )
         {
            lrng_bycell  = true;           // Flag based on count difference
         }

         else
         {
            for ( int wvx = 0; wvx < nwaveln; wvx++ )
            {
               if ( wvs[ wvx ] != toti_wvlns[ wvx ] )
               {
                  lrng_bycell  = true;     // Flag based on value difference
                  break;
               }
            }
         }
      } // END: cell scan loop

      lambdas_by_cell( 0 );

      nwavelo      = nwaveln;
      slambda      = toti_wvlns[ 0 ];
      elambda      = toti_wvlns[ nwaveln - 1 ];
      dlambda      = 1;
      le_ltrng ->setText( tr( "%1 raw: %2 %3 to %4" )
         .arg( nwaveln ).arg( chlamb ).arg( slambda ).arg( elambda ) );
      le_lxrng ->setText( tr( "%1 MWL exports: %2 %3 to %4,"
                              " raw index increment %5" )
         .arg( nwavelo ).arg( chlamb ).arg( slambda ).arg( elambda )
         .arg( dlambda ) );

      expd_radii .clear();
      expc_wvlns .clear();
      nwavelo      = mwl_data.lambdas( expi_wvlns, 0 );
      std::sort(expi_wvlns.begin(), expi_wvlns.end());
DbgLv(1) << "IS-MWL:    new nwavelo" << nwavelo << expi_wvlns.count();

      // Initialize export wavelength lists for first channel
      for ( int wvx = 0; wvx < nwavelo; wvx++ )
      {
         expc_wvlns << QString::number( expi_wvlns[ wvx ] );
      }

      // Update wavelength lists in GUI elements
      cb_lstart->clear();
      cb_lend  ->clear();
      cb_lplot ->clear();
      cb_lstart->addItems( expc_wvlns );
      cb_lend  ->addItems( expc_wvlns );
      cb_lplot ->addItems( expc_wvlns );
      int lastx    = nwavelo - 1;
      plotndx      = nwavelo / 2;
      cb_lplot ->setCurrentIndex( plotndx );
      cb_lstart->setCurrentIndex( 0 );
      cb_lend  ->setCurrentIndex( lastx );
DbgLv(1) << "IS-MWL:  expi_wvlns size" << expi_wvlns.size() << nwaveln;

//       edata         = &allData[ 0 ];
//       nrpoint       = edata->pointCount();
//       int nscan     = edata->scanCount();
//       for ( int trx = 0; trx < allData.size(); trx++ )
//          nscan         = qMax( nscan, allData[ trx ].scanCount() );
//       int ndset     = ncelchn * nrpoint;
//       int ndpoint   = nscan * maxwavl;
// DbgLv(1) << "IS-MWL:   nrpoint nscan ndset ndpoint" << nrpoint << nscan
//  << ndset << ndpoint;

//       for ( int ii = 0; ii < nrpoint; ii++ )
//       {  // Update the list of radii that may be plotted
//          expd_radii << data.xvalues[ ii ];
//          expc_radii << QString().sprintf( "%.3f", data.xvalues[ ii ] );
//       }
// DbgLv(1) << "IS-MWL:  expd_radii size" << expd_radii.size() << nrpoint;

//       QVector< double > wrdata;
//       wrdata.fill( 0.0, ndpoint );
//       rdata .clear();
// DbgLv(1) << "IS-MWL:  wrdata size" << wrdata.size() << ndpoint;

//       for ( int ii = 0; ii < ndset; ii++ )
//       {  // Initialize the data vector that has wavelength as the x-axis
//          rdata << wrdata;
//       }
// DbgLv(1) << "IS-MWL:  rdata size" << rdata.size() << ndset;

//       // Update wavelength-x-axis data vector with amplitude data points
//       // The input has (ncelchn * nwaveln) data sets, each of which
//       //   contains (nscan * nrpoint) data points.
//       // The output has (ncelchn * nrpoint) data sets, each of which
//       //   contains (nscan * nwaveln) data points.
//       int trx       = 0;

//       for ( int ccx = 0; ccx < ncelchn; ccx++ )
//       {  // Handle each triple of AUC data
//          lambdas_by_cell( ccx );                      // Lambdas in cell

//          for ( int jwx = 0; jwx < nwaveln; jwx++ )
//          {  // Each wavelength in the current cell
//             edata         = &allData[ trx ];               // Triple data
//             int iwavl     = rawi_wvlns[ jwx ];             // Wavelength value
//             int wvx       = toti_wvlns.indexOf( iwavl );   // Wavelength index
// DbgLv(1) << "IS-MWL:   trx ccx wvx" << trx << ccx << wvx;

//             for ( int scx = 0; scx < edata->scanCount(); scx++ )
//             {  // Each scan of a triple
//                US_DataIO::Scan* scan  = &edata->scanData[ scx ];
//                int odx       = ccx * nrpoint;         // Output dataset index
//                int opx       = scx * maxwavl + wvx;   // Output point index
// DbgLv(2) << "IS-MWL:    scx odx opx" << scx << odx << opx;
//                for ( int rax = 0; rax < nrpoint; rax++ )
//                {  // Store ea. radius data point as a wavelength point in a scan
//                   rdata[ odx++ ][ opx ]  = scan->rvalues[ rax ];
//                } // END: radius points loop
//             } // END: scans loop

//             trx++;
//          } // END: input triples loop
//       } // END: input celchn loop
// DbgLv(1) << "IS-MWL:    Triples loop complete";

      set_data_over_lamda();

DbgLv(1) << "IS-MWL: celchns size" << celchns.size();
      lb_triple->setText( tr( "Cell / Channel" ) );
      cb_triple->disconnect();
      cb_triple->clear();
      cb_triple->addItems( celchns );
      connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
                          SLOT  ( new_triple_auto    ( int ) ) );
      pb_nextChan->setEnabled( celchns.size() > 1 );

      odlimit   = 1.8;

      connect_mwl_ctrls( true );

      plot_mwl();
   } // END: isMwl=true
   else
   {
     qDebug() << "LOADING 1";
     
     //new_triple_auto( 0 );                             //ALEXEY <--- new_triple_auto()
     pb_nextChan->setEnabled( cb_triple->count() > 1 );

     qDebug() << "LOADING 2";
//*DEBUG* Print times,omega^ts
 triple = allData[0];
 double timel = triple.scanData[0].rpm / 400.0;
 double rpmc  = 400.0;
 int    nstep = (int)timel;
 double w2ti  = 0.0;
 for ( int ii=0; ii<nstep; ii++ )
 {
  w2ti += sq( rpmc * M_PI / 30.0 );
  rpmc += 400.0;
 }
 for ( int ii=0; ii<triple.scanData.size(); ii++ )
 {
  US_DataIO::Scan* ds=&triple.scanData[ii];
  double timec = ds->seconds;
  double rpmc  = ds->rpm;
  w2ti += ( timec - timel ) * sq( rpmc * M_PI / 30.0 );
  qDebug() << "scan" << ii+1 << "delta-r rpm seconds" << ds->delta_r
   << rpmc << timec << "omega2t w2t-integ" << ds->omega2t << w2ti;
  if(ii==0)
  {
   double deltt = ds->omega2t / sq(rpmc*M_PI/30.0);
   double time1 = timel + deltt;
   qDebug() << "   scan 1 omega2t-implied time" << time1;
  }
  timel = timec;
 }
//*DEBUG* Print times,omega^ts
   }

   // Set up OD limit and any MWL controls
   ct_odlim->disconnect();
   ct_odlim->setValue( odlimit );
   connect( ct_odlim,  SIGNAL( valueChanged       ( double ) ),
            this,      SLOT  ( od_radius_limit    ( double ) ) );

   show_mwl_controls( isMwl );

   

   /***************** TESTING ******************************************/

   //all_loaded = true;
   le_status->setText( tr( "Data loaded..." ) );

   emit data_loaded();

   // editProfile.clear();
   // centerpieceParameters.clear();
   // aprofileParameters.clear();

   // //Read centerpiece names from protocol:
   // centerpiece_names.clear();
   centerpiece_names.resize( cb_triple->count() );

   read_centerpiece_names_from_protocol();

   //Debug:
   for (int i=0; i<centerpiece_names.size(); i++)
     qDebug() << "Centerpeice name: " << centerpiece_names[i];

   //AProfile details per channel
   read_aprofile_data_from_aprofile();
   qDebug() << "job1run, job2run, job3run, job3auto, job4run, job5run, job6run_pcsa: "
	    <<  job1run << ", "
	    <<  job2run << ", "
	    <<  job3run << ", "
	    <<  job3auto << ", "
	    <<  job4run << ", "
	    <<  job5run << ", "
	    <<  job6run_pcsa;
   
   //Debug
   for (int i=0; i<cb_triple->count(); i++)
     qDebug() << "Triple, AprofParms: " << cb_triple->itemText( i )  << ", " << aprofileParameters[i];

   
   qDebug() << "DATA SIZE: " << outData.count();

   if ( !isMwl )
     {
       lb_lplot ->setHidden( true );
       cb_lplot ->setHidden( true );
       pb_larrow->setHidden( true );
       pb_rarrow->setHidden( true );
     }


   //ALEXEY: Resize && fill with zero iwavl_edit_ref vector:
   iwavl_edit_ref        .resize( cb_triple->count() );
   iwavl_edit_ref_index  .resize( cb_triple->count() );
   triple_plot_first_time.resize( cb_triple->count() );
   for ( int trx = 0; trx < cb_triple->count(); trx++ )
     {
       iwavl_edit_ref[ trx ] = 0;
       iwavl_edit_ref_index[ trx ] = 0;
       triple_plot_first_time[ trx ] = 0;
     }

   
   //for ( int trx = 0; trx < triples.size(); trx++ )
   //Go over triples: for IP-MWL data, find reference wvl
   //find auto-meniscus porisions
   for ( int trx = 0; trx < cb_triple->count(); trx++ )
     {
       QString triple_name = cb_triple->itemText( trx );

       //Alternative chname:
       QStringList triple_name_list_ = cb_triple->itemText( trx ).split("/");
       QString triple_cell_number_   = triple_name_list_[0].trimmed();
       QString triple_channel_       = triple_name_list_[1].trimmed();
       QString channelname_        = triple_cell_number_ + triple_channel_;
       qDebug() << "channelname_: " << channelname_ ;
       
       if ( dataSource. contains("DiskAUC:Absorbance") && channelname_.contains("S")  )
	 {
	   channelname_ = channelname_.replace("S","A");
	   simulated_data = true;
	 }
       
       le_status->setText( tr( "Setting edit controls for channel %1" ).arg( triple_name ) );
       qApp->processEvents();

      
       //Debug
       for ( int g=0; g < expi_wvlns.size(); ++g )
	 qDebug() << "MWL wavelengths for triple: " << triple_name << expi_wvlns[ g ];

       if ( isMwl )
	 {
	   qDebug() << "#triples, #wavelns_i -- " << cb_triple->count() << wavelns_i.size();
	   qDebug() << "#wavelns in triple   -- " << triple_name << wavelns_i[ trx ].size();

	   mwl_data.lambdas( expi_wvlns, trx );
	   std::sort(expi_wvlns.begin(), expi_wvlns.end());
	   //Debug
	   for ( int g=0; g < expi_wvlns.size(); ++g )
	     qDebug() << "MWL wavelengths for triple: " << triple_name << expi_wvlns[ g ];
	   
	   //Check here if info for EDIT wvl is set in the AProfile:
	   //    if yes, identify wvl for current triple:
	   //    otherwise, use current plotndx

	   QString channel_desc = triple_name;
	   channel_desc.replace(" / ",".");
	   QString opsys = QString("UV/vis");
	   
	   if ( isSet_edit_info_for_channel( channel_desc,  opsys ) )
	     {
	       //iwavl = wvl_from_aprofile;
	       channel_desc.replace(".","");
	       
	       QMap<QString, QString>::iterator jjj;
	       for ( jjj = triple_to_edit.begin(); jjj != triple_to_edit.end(); ++jjj )
		 {
		   qDebug() << "jjj.key(), channel_desc, opsys -- " << jjj.key() << channel_desc << opsys;
		   if ( jjj.key().contains( channel_desc ) && jjj.key().contains( opsys ) )
		     {
		       QString wvl_set_edit = jjj.value();

		       iwavl_edit_ref[ trx ] =  wvl_set_edit.toInt();
		       qDebug() << "Reference wvl --  " <<  wvl_set_edit << " was identified using AProfile";
		       		       
		       break;
		     }
		 }

	       //TEST:: IMPORTANT ** comment out when done with testing !!
	       //iwavl_edit_ref[ trx ] = 245;

	       //Now, identify wvl index:
	       for ( int w_i=0; w_i < expi_wvlns.size(); ++w_i )
		 {
		   if ( expi_wvlns[ w_i ] == iwavl_edit_ref[ trx ] )
		     {
		       iwavl_edit_ref_index[ trx ] = w_i;
		       break;
		     }
		 }
	     }
	   else
	     {
	       plotndx  = cb_lplot->currentIndex();

	       //TEST:: IMPORTANT ** comment out when done with testing !!
	       //plotndx = 2;
	       
	       iwavl_edit_ref[ trx ]       = expi_wvlns[ plotndx ];
	       iwavl_edit_ref_index[ trx ] = plotndx;
	       qDebug() << "Wvl index && value -- " <<  iwavl_edit_ref_index[ trx ] << iwavl_edit_ref[ trx ];
	     }
	   
	   data_index   = mwl_data.data_index( iwavl_edit_ref[ trx ], trx );

	   //Debug
	   for (int kk=0; kk<iwavl_edit_ref.size(); ++kk )
	     qDebug() << "isMWL: Ref wvl -- "  << iwavl_edit_ref[ kk ];

	   QVector< int > wvs_temp;
	   int num_wvls =  mwl_data.lambdas( wvs_temp, trx );
      std::sort(wvs_temp.begin(), wvs_temp.end());
	   qDebug() << "#Wvls for " << triple_name << trx << ": " << num_wvls;
	   for (int rr=0; rr<wvs_temp.size(); ++rr )
	     qDebug() << "wvls ARE -- "  << wvs_temp[ rr ];
	   
	 }
       else
	 index_data_auto( trx );

       qDebug() << "Data index: " << data_index;
       
       edata          = outData[ data_index ];
       data           = *edata;
       
       //QString rawGUID_test          = US_Util::uuid_unparse( (unsigned char*)data.rawGUID );
       //qDebug() << "Current rawData: rawGUID: " <<  rawGUID_test << ", filename: " << files[ trx ] << ", editGUID: " << editGUIDs[ trx ];

       //Find meniscus position based on the read parameters for centerpiece channel (trx) && aprofile for each channel 
       read_centerpiece_params( trx );

       //range_right = 7.0;
       aprofile_volume = aprofileParameters[ trx ][0].toDouble();        //<-- From Aprofile
       range_right   = aprofileParameters[ trx ][1].toDouble();           //<-- From Aprofile
       qDebug() << "Range_right && Loading volume FROM AProfile: " << triple_name << ", " << range_right << ", " << aprofile_volume;
       
       //Find meniscus
       if ( dataType != "IP" )
	 {
	   lb_airGap->setHidden( true );
	   le_airGap->setHidden( true );
	   pb_airGap->setHidden( true );
	   
	   meniscus = find_meniscus_auto();
             
	   le_meniscus ->setText( QString::number( meniscus,   'f', 3 ) );
	   
	   range_left    = meniscus + _RNGLEFT_OFFSET_;
	   le_dataStart->setText( QString::number( range_left, 'f', 3 ) );
	   
	   
	   le_dataEnd  ->setText( QString::number( range_right, 'f', 3 ) );
	   // plateau      = range_right - _PLATEAU_OFFSET_;
	   // le_plateau  ->setText( QString::number( plateau,     'f', 3 ) );
	   
	   step = PLATEAU;
	   //plot_range();
	   next_step();
	   
	   //ALEXEY: here create a QMap to couple current triple AND meniscus, ranges, plateau and baseline;
	   // then, use this QMap when plotting plot_range() in new_triple_auto()...
	   
	   triple_info.clear();
	   triple_info <<  QString::number(meniscus)
		       <<  QString::number(range_left)
		       <<  QString::number(range_right)
		       <<  QString::number(plateau)
		       <<  QString::number(baseline)
		       <<  QString::number(baseline_od)
		       <<  QString("spike_false");
	   
	   //ALEXEY: get all cb_triple listbox items (texts)...
	   editProfile[ triple_name ] = triple_info;

	   //automatic meniscus Map, per channel
	   automatic_meniscus[ triple_name ] = true;


	   //Fill out editProfile_scans_excl MAP:
	   QMap<QString, QStringList>::const_iterator ci = aprof_channel_to_scans.constBegin();
	   while (ci != aprof_channel_to_scans.constEnd())
	     {
	       if ( channelname_.contains(ci.key()) )
		 {
		   qDebug() << "Channel, AProfile SCANSS: " << channelname_ << ", " << ci.key() << ": " << ci.value();
		   editProfile_scans_excl[ triple_name ] = ci.value();
		   break;
		 }
	       ++ci;
	     }
	   	   
	   qDebug() << triple_name  << ", " << triple_info;
	   
	 }
       else
	 {
	   // <---- For now: for Interference data, do editing && save manually
	   //pb_meniscus->setHidden( false );
	   //pb_airGap->setHidden( false );
	   //le_airGap->setHidden( false );

	   lb_airGap->setHidden( false );
	   le_airGap->setHidden( false );


	   //find meniscus
	   meniscus = find_meniscus_interference_auto();
	   
	   //find airGap && adjust interference_data
	   airGap_values = find_airGap_interference_auto();

	   airGap_left  = airGap_values[0];
	   airGap_right = airGap_values[1];
	   
	   US_DataIO::EditValues  edits;
	   edits.airGapLeft  = airGap_left;
	   edits.airGapRight = airGap_right;

	   	   
	   QList< int > excludes;
           
	   for ( int i = 0; i < data.scanData.size(); i++ )
	     if ( ! includes.contains( i ) ) edits.excludes << i;
	   
	   DbgLv(1) << "AGap: L R" << airGap_left << airGap_right << " AdjIntf";
	   US_DataIO::adjust_interference( data, edits );
	   
	   // Un-zoom
	   if ( plot->btnZoom->isChecked() )
	     plot->btnZoom->setChecked( false );
	   
	   // Display the data
	   QString wkstr;
	   le_airGap->setText( wkstr.sprintf( "%.3f - %.3f", 
					      airGap_left, airGap_right ) );
	   
	   step          = RANGE;
	   range_left    = meniscus + _RNGLEFT_OFFSET_;

	   le_meniscus ->setText( QString::number( meniscus,   'f', 3 ) );
	   le_dataStart->setText( QString::number( range_left, 'f', 8 ) );
	   le_dataEnd  ->setText( QString::number( range_right, 'f', 3 ) );

	   // RANGES
	   edits.rangeLeft    = range_left;
	   edits.rangeRight   = range_right;
	   edits.gapTolerance = ct_gaps->value();

	   qDebug() << "RANGE: edits.gapTolerance: " << edits.gapTolerance;
	   
	   US_DataIO::calc_integral( data, edits );
	   
	   	   
	   step = PLATEAU;
	   //plot_range();              //ALEXEY <--- here applies the gap removal
	   next_step();

	   //qApp->processEvents();
	   
	   //ALEXEY: here create a QMap to couple current triple AND meniscus, ranges, plateau and baseline;
	   // then, use this QMap when plotting plot_range() in new_triple_auto()...
	   
	   triple_info.clear();
	   triple_info <<  QString::number(meniscus)
		       <<  QString::number(range_left)
		       <<  QString::number(range_right)
		       <<  QString::number(plateau)
		       <<  QString::number(baseline)
		       <<  QString::number(baseline_od)
		       <<  QString("spike_false")
		       <<  QString::number(airGap_left)
		       <<  QString::number(airGap_right);
	   
	   //ALEXEY: get all cb_triple listbox items (texts)...
	   editProfile[ triple_name ] = triple_info;

	   //automatic meniscus Map, per channel
	   automatic_meniscus[ triple_name ] = true;

	   //Fill out editProfile_scans_excl MAP:
	   QMap<QString, QStringList>::const_iterator ci = aprof_channel_to_scans.constBegin();
	   while (ci != aprof_channel_to_scans.constEnd())
	     {
	       if ( channelname_.contains(ci.key()) )
		 {
		   qDebug() << "Channel, AProfile SCANSS: " << channelname_ << ", " << ci.key() << ": " << ci.value();
		   editProfile_scans_excl[ triple_name ] = ci.value();
		   break;
		 }
	       ++ci;
	     }

	   qDebug() << triple_name  << ", " << triple_info;
	 }

       if ( autoflow_expType == "ABDE" )
	 {
	   edited_triples_abde[ triple_name ] = false;
	   automatic_meniscus[ triple_name ]  = false;
	 }
     }
   //[FINISHED] Go over triples: for IP-MWL data, find reference wvl
   //[FINISHED] find auto-meniscus porisions

   if ( editProfile.count() == cb_triple->count() )
     all_loaded = true;

   qDebug() << "ALL_LOADED: " << all_loaded;

   if ( isMwl )
     {
       new_triple_auto( 0 );     //ALEXEY: is it right ? 
       //plot_mwl();
       pb_spikes   ->setEnabled( true );
     }
   else
     //if (  dataType != "IP" )
     new_triple_auto( 0 );                  //ALEXEY <--- here does NOT applies the gap removal

   if ( autoflow_expType == "ABDE" )
     pb_write->setEnabled( false );
   else
     pb_write->setEnabled( true );
   pb_emanual->setEnabled( true );
   pb_undo ->setEnabled( false ); 

   le_status->setText( tr( "Edit controls set" ) );

}

void US_Edit::reset_editdata_panel( void )
{
  qDebug() << "Resetting EDIT_DATA panel BEFORE going to Manage runs... ";
  
  reset( );
  
  qDebug() << "EDIT_DATA reset ..";
}


void US_Edit::read_aprofile_data_from_aprofile()
{
  aprof_channel_to_parms.clear();
  aprof_channel_to_scans.clear();

  QString aprofile_xml;
  
  // Check DB connection
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db( masterPW );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db.lastError() );
      return;
    }

  // qDebug() << "AProfGUID: " << AProfileGUID;

    
  QStringList qry;
  qry << "get_aprofile_info" << AProfileGUID;
  db.query( qry );
  
  while ( db.next() )
    {
      //currProf.aprofID     = db.value( 0 ).toInt();
      //currProf.aprofname   = db.value( 1 ).toString();
      aprofile_xml         = db.value( 2 ).toString();
    }

  //qDebug() << "aprofile_xml: " <<  aprofile_xml;
  
  if ( !aprofile_xml.isEmpty() )
    {
      QXmlStreamReader xmli( aprofile_xml );
      readAProfileBasicParms_auto( xmli );
    }


  //qDebug() << "read_aprofile: 1" ;
  //
  for ( int trx = 0; trx < cb_triple->count(); trx++ )
    {
      QStringList triple_name_list = cb_triple->itemText( trx ).split("/");
      QString triple_cell_number = triple_name_list[0].trimmed();
      QString triple_channel     = triple_name_list[1].trimmed();

      QString channelname        = triple_cell_number + triple_channel;

      qDebug() << "channelname1: " << channelname ;

      if ( dataSource. contains("DiskAUC:Absorbance") && channelname.contains("S")  )
	channelname = channelname.replace("S","A");

      qDebug() << "channelname2: " << channelname ;
      
      //iterate over aprofile_channel_to_parms QMap and check if channel name is a part of triple_name:
      QMap<QString, QStringList>::const_iterator i = aprof_channel_to_parms.constBegin();
      while (i != aprof_channel_to_parms.constEnd())
	{
	  if ( channelname.contains(i.key()) )
	    {
	      qDebug() << "Channel, AProfile PARMS: " << channelname << ", " << i.key() << ": " << i.value();
	      aprofileParameters[ trx ] = i.value();
	      break;
	    }
	  ++i;
	}
    }

}

void US_Edit::read_centerpiece_names_from_protocol()
{
  //centerpiece_names.clear();
  cell_to_centerpiece.clear();

  // Check DB connection
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db( masterPW );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db.lastError() );
      return;
    }
     
  QString xmlstr( "" );

  US_ProtocolUtil::read_record_auto( ProtocolName_auto, idInv_auto.toInt(),  &xmlstr, NULL, &db );

  qDebug() << "Protocol READ !!! ";
  
  QXmlStreamReader xmli( xmlstr );

  while( ! xmli.atEnd() )
    {
      xmli.readNext();
      
      if ( xmli.isStartElement() )
	{
	  QString ename   = xmli.name().toString();

	  // if( ename == "rotor" )                <-- HERE retrieve operID
	  //   readProtocolRotor_auto( xmli );

	  if ( ename == "cells" )
	    readProtocolCells_auto( xmli );

	  if ( ename == "aprofile" )
	    readProtocolAProfile_auto( xmli );
	}
    }

  qDebug() << "Protocol's Cell Section READ !!! ";

  // Fill in centerpiece names from protocol (per channel)
  // DO  NOT FORGET TO populate centerpiece_names array by CHANNELS, NOT by Cells only !!!
  // E.g. if cells 2 & 4, then centerpiece_names must be put for 2/A, 2/B, 4/A, 4/B
  // Some channles of the same cell may NOT be used!!! e.g 2/A, 4/A && 4/B

  for ( int trx = 0; trx < cb_triple->count(); trx++ )
    {
      QStringList triple_name_list = cb_triple->itemText( trx ).split("/");
      QString triple_name_cell = triple_name_list[0];

      //iterate over cell_to_centerpeice QMap and check if cellID is a part of triple_name:
      QMap<QString, QString>::const_iterator i = cell_to_centerpiece.constBegin();
      while (i != cell_to_centerpiece.constEnd())
	{
	  if ( triple_name_cell.contains(i.key()) )
	    {
	      qDebug() << "triple_name_cell, cell, centerpiece: " << triple_name_cell << ", " << i.key() << ": " << i.value();
	      //centerpiece_name = i.value();
	      centerpiece_names[ trx ] = i.value();
	      break;
	    }
	  ++i;
	}
      
      //centerpiece_names[ trx ] = centerpiece_name;
    }
  
}

bool US_Edit::readProtocolCells_auto( QXmlStreamReader& xmli )
{
  while( ! xmli.atEnd() )
    {
      QString ename   = xmli.name().toString();
      
      if ( xmli.isStartElement() )
	{
	  if ( ename == "cell" )
	    {
	      QXmlStreamAttributes attr = xmli.attributes();
	      QString centerpiece = attr.value( "centerpiece" ).toString();
	      QString cell_id     = attr.value( "id"          ).toString();

	      cell_to_centerpiece [ cell_id ] =  centerpiece;
	    }
	}
      
      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element
      
      if ( was_end  &&  ename == "cells" )    // Break after "</cells>"
	break;
    }
  
  return ( ! xmli.hasError() );
}


bool US_Edit::readProtocolAProfile_auto( QXmlStreamReader& xmli )
{
  while( ! xmli.atEnd() )
    {
      QString ename   = xmli.name().toString();
      
      if ( xmli.isStartElement() )
	{
	  if ( ename == "aprofile" )
	    {
	      QXmlStreamAttributes attr = xmli.attributes();
	      //AProfileName   = attr.value( "name"  ).toString();
	      AProfileGUID   = attr.value( "guid"  ).toString();
	    }
	  
	  else
            break;
	}
      
      else if ( xmli.isEndElement()  &&  ename == "aprofile" )
	break;
      
      xmli.readNext();
    }
  
  return ( ! xmli.hasError() );
}


bool US_Edit::readAProfileBasicParms_auto( QXmlStreamReader& xmli )
{
  // while( ! xmli.atEnd() )
  //   {
  //     xmli.readNext();

  while( ! xmli.atEnd() )
    {
      QString ename   = xmli.name().toString();
      
      if ( xmli.isStartElement() )
      {
	
	//QString ename   = xmli.name().toString();

	if ( ename == "channel_parms" )
	  {
            QXmlStreamAttributes attr = xmli.attributes();

	    if ( attr.hasAttribute("load_volume") )
	      {
		
		QString channel_name = attr.value( "channel" ).toString();
				
		QStringList aprof_parms;
		aprof_parms << attr.value( "load_volume" ).toString()
			    << attr.value( "data_end" ).toString();
		
		qDebug() << "READING aprof XML: " << channel_name << ", " << aprof_parms;
		
		aprof_channel_to_parms[ channel_name ] = aprof_parms;
	       
		//Read what channels to analyse:
		if ( attr.hasAttribute("run") )
		  {
		    QString channel_desc = attr.value( "chandesc" ).toString();
		    channels_to_analyse[ channel_desc ] = bool_flag( attr.value( "run" ).toString() );
		  }
		
		//Read what triple selected for editing:
		if ( attr.hasAttribute("wvl_edit") )
		  {
		    QString channel_desc = attr.value( "chandesc" ).toString();
		    triple_to_edit[ channel_desc ] = attr.value( "wvl_edit" ).toString();
		  }

		//Read what triples NOT to analyse:
		if ( attr.hasAttribute("wvl_not_run") )
		  {
		    QString channel_desc = attr.value( "chandesc" ).toString();
		    triples_skip_analysis[ channel_desc ] = attr.value( "wvl_not_run" ).toString();
		  }

		//Read scan exclusion information:
		if ( attr.hasAttribute("scan_excl_begin") )
		  {
		    QStringList scan_excl_pairs;
		    scan_excl_pairs << attr.value( "scan_excl_begin" ).toString()
				    << attr.value( "scan_excl_end" )  .toString();
		    
		    qDebug() << "READING aprof XML, scans excl.: " << channel_name << ", " << scan_excl_pairs;
		    
		    aprof_channel_to_scans[ channel_name ] = scan_excl_pairs;
		  }
	      }
	  }
	
	else if ( ename == "job_2dsa" )
	  {
            QXmlStreamAttributes attr = xmli.attributes();
            job1run        = bool_flag( attr.value( "run" ).toString() );
            //job1nois       = attr.value( "noise" ).toString();

	    qDebug() << "job1run: " << job1run;
	  }
	else if ( ename == "job_2dsa_fm" )
         {
	   QXmlStreamAttributes attr = xmli.attributes();
	   job2run        = bool_flag( attr.value( "run" ).toString() );
	   //job2nois       = attr.value( "noise" ).toString();
	   //fitrng         = attr.value( "fit_range" ).toString().toDouble();
	   //grpoints       = attr.value( "grid_points" ).toString().toInt();

	   qDebug() << "job2run: " << job2run;
         }
	else if ( ename == "job_fitmen" || ename == "job_fitmen_auto" )
	  {
            QXmlStreamAttributes attr = xmli.attributes();
            job3run        = bool_flag( attr.value( "run" ).toString() );
            job3auto       = attr.value( "interactive" ).toString().toInt() == 0;
	    	    
	    qDebug() << "job3run:  " << job3run;
	    qDebug() << "job3auto: " << job3auto;
	  }
	else if ( ename == "job_2dsa_it" )
	  {
            QXmlStreamAttributes attr = xmli.attributes();
            job4run        = bool_flag( attr.value( "run" ).toString() );
            job4nois       = attr.value( "noise" ).toString();
            //rfiters        = attr.value( "max_iterations" ).toString().toInt();

	    qDebug() << "job4run: " << job4run;
	  }
	else if ( ename == "job_2dsa_mc" )
	  {
            QXmlStreamAttributes attr = xmli.attributes();
            job5run        = bool_flag( attr.value( "run" ).toString() );
            //mciters        = attr.value( "mc_iterations" ).toString().toInt();

	    qDebug() << "job5run: " << job5run;
	  }
	else if ( ename == "p_pcsa" )
	  {
	    QXmlStreamAttributes attr = xmli.attributes();
            job6run_pcsa   = bool_flag( attr.value( "job_run" ).toString() );

	    qDebug() << "job6run_pcsa: " << job6run_pcsa;
	  }
      }
      
      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end  &&  ename == "p_pcsa" )   // Break after "</p_pcsa>"
         break;
      
      // else if ( ename == "p_2dsa" )       //Stop reading AProfile when 2DSA section reached 
      //  break;
	
      //}
    }
  
  return ( ! xmli.hasError() );
}


// Return a flag if an XML attribute string represents true or false.
bool US_Edit::bool_flag( const QString xmlattr )
{
   return ( !xmlattr.isEmpty()  &&  ( xmlattr == "1"  ||  xmlattr == "T" ) );
}


void US_Edit::read_centerpiece_params( int trx )
{
   QStringList query;
   QString centerpieceID_read;
   QString centerpieceName_read;

   centerpiece_info.clear();

   // Check DB connection
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );
   
   if ( db.lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db.lastError() );
       return;
     }
   
   //Establish correspondence btw centerpiece name & ID (centerpieceID)
   query.clear();
   query << "get_abstractCenterpiece_names";
   db.query( query );

   while ( db.next() )
     {
       QString id   =  db.value( 0 ).toString();
       QString name =  db.value( 1 ).toString();

       if ( centerpiece_names[ trx ] == name )
	 {
	   centerpieceID_read = id;
	   break;
	 }
     }
   
   //Get Centerpiece info
   query.clear();
   query << "get_abstractCenterpiece_info" << centerpieceID_read;
   db.query( query );
   db.next();

   //Curent
   centerpiece_info[ "name" ]       = db.value( 1 ).toString();
   centerpiece_info[ "bottom" ]     = db.value( 3 ).toString();
   centerpiece_info[ "pathlen" ]    = db.value( 6 ).toString();
   centerpiece_info[ "angle" ]      = db.value( 7 ).toString();

   //Global
   centparms_info.clear();
   centparms_info << centerpiece_info[ "name" ]
		  << centerpiece_info[ "bottom" ]
		  << centerpiece_info[ "pathlen" ]
		  << centerpiece_info[ "angle" ];

   centerpieceParameters[ trx ] = centparms_info;
}


QVector<double> US_Edit::find_airGap_interference_auto()
{
  QVector<double> airGap_vals;
  QVector< double > x_airGap_lefts;
  QVector< double > x_airGap_rights;
  double airGap_left_av  = 0;
  double airGap_right_av = 0;
  
  // starting from meniscus position, go backwards and fit several points into staight line..
  // then, look at max deviation from it and find the onset of the airGap (in reverse order)
  // basically, repeat finding meniscus algorithm...

  // For each scan in the last ~20% of scans:
  int start_scan = int(0.8*data.scanData.size());
  int end_scan   = data.scanData.size();

  for ( int i = start_scan; i < end_scan; i++ )
    {
      US_DataIO::Scan*  s = &data.scanData[ i ];
      
      double y_max_deviation = -1.0e99;
      double x_airGap_left;
      double x_airGap_right;
     
      double right_fit    = meniscus - 0.02;
      int indexRight_fit  = data.xindex( right_fit );
      double left_fit     = right_fit - 0.02;
      int indexLeft_fit   = data.xindex( left_fit );

      //qDebug() << "INTERFERENCE: left_fit, right_fit: " << left_fit << ", " << right_fit;

      /**********************
      //Find a fit to straight line
      double sumX  = 0;
      double sumY  = 0;
      double sumXY = 0;
      double sumX2 = 0;
      int countPoints = 0;
      
      for ( int j = indexLeft_fit; j <= indexRight_fit; j++ )
      {
	double y_curr = s->rvalues[ j ] * invert;
	double x_curr = data.xvalues[ j ];

	sumX  += x_curr;
	sumY  += y_curr;
	sumXY += x_curr*y_curr;
	sumX2 += x_curr*x_curr;

	++countPoints;
      }

      //Slope && mean: y(x) = Slope * X + YInt;
      double xMean = sumX / countPoints;
      double yMean = sumY / countPoints;
      double Slope = (sumXY - sumX * yMean) / (sumX2 - sumX * xMean);
      double YInt  = yMean - Slope * xMean;

      ***************/
      
      /********** TESTITNG *******************************************************/
      //Alternative: derive a curve passing through Left/Right indecies
      double Slope = (s->rvalues[ indexRight_fit ] * invert - s->rvalues[ indexLeft_fit ] * invert ) / ( data.xvalues[ indexRight_fit ] - data.xvalues[ indexLeft_fit ] );
      double YInt  = s->rvalues[ indexRight_fit ] * invert - Slope *  data.xvalues[ indexRight_fit ];
      /****************************************************************************/
      
      //qDebug() << "y(x) = Slope * X + YInt: " << Slope << "*X + " <<  YInt;

      //Find the biggest y-deviation from a line fit
      for ( int j = indexLeft_fit; j <= indexRight_fit; j++ )
      {
	double y_curr = s->rvalues[ j ] * invert;
	double x_curr = data.xvalues[ j ];

	double y_fit  = Slope * x_curr + YInt;
 	double y_diff_abs = fabs( y_curr - y_fit );

	//qDebug() << "y_curr, x_curr, (Slope * x_curr + YInt), y_diff_abs: " << y_curr << ", " << x_curr << ", " << y_fit << ", " << y_diff_abs;

	if( y_diff_abs > y_max_deviation )
	  y_max_deviation = y_diff_abs;
      }

      //qDebug() << " y_max_deviation: " <<  y_max_deviation;

      //reverse order: search for a y-deviation (much) bigger than from a straight line fit based on selected points
      int indexLeft   = data.xindex( 5.85 );  //Begiinning of the cell?
      //right_fit -= 0.02;
      //indexRight_fit  = data.xindex( right_fit );
      for ( int j = indexRight_fit; j >= indexLeft; --j )
      	{
	  double y_curr = s->rvalues[ j ] * invert;
	  double x_curr = data.xvalues[ j ];

	  double y_diff_abs = fabs( y_curr - (Slope * x_curr + YInt) );

	  if( y_diff_abs > y_max_deviation * 1.5 )
	    {
	      x_airGap_right = x_curr - 0.02;
	      x_airGap_left  = x_airGap_right - 0.02;
	      break; 
	    }
	}
      x_airGap_lefts.push_back( x_airGap_left );
      x_airGap_rights.push_back( x_airGap_right );

      //qDebug() << "x_airGap_left, x_airGap_right: " << x_airGap_left << ", " << x_airGap_right;
    }

  //Find simple average position of the left/right airGap values
  for ( int i=0; i < x_airGap_lefts.size(); i++ )
    {
      airGap_left_av  += x_airGap_lefts[ i ];
      airGap_right_av += x_airGap_rights[ i ];
    }
  
  airGap_left_av  /= x_airGap_lefts.size();
  airGap_right_av /= x_airGap_rights.size();
  
  // airGap_vals.push_back(airGap_left_av);
  // airGap_vals.push_back(airGap_right_av);

  //TEMP
  airGap_vals.push_back(5.85);
  airGap_vals.push_back(5.90);

  
  qDebug() << "INTERFERENCE: airGaps: " << airGap_vals;
    
  return airGap_vals;
}

double US_Edit::find_meniscus_interference_auto()
{
  double bottom_db     = centerpiece_info[ "bottom" ].toDouble(); 
  double pathlength_db = centerpiece_info[ "pathlen" ].toDouble();
  double angle_db      = centerpiece_info[ "angle" ].toDouble();

  //double aprofile_volume   = 460; // Just an example - to be read from AProfile, will be global
  
  double meniscus_init = sqrt( bottom_db*bottom_db - ( aprofile_volume*360/(1000*pathlength_db*angle_db*M_PI ) ) );     //Radians = Degrees * (M_PI/180.0)
  
  qDebug() << "Meniscus_init: INTERFERENCE " << meniscus_init << ", " << bottom_db << ", " << pathlength_db << ", " << angle_db << ", " << M_PI;
    
  double meniscus_av = 0;
  QVector< double > x_meniscuses;
  
  // For each scan in the last ~20% of scans:
  int start_scan = int(0.8*data.scanData.size());
  int end_scan   = data.scanData.size();

  qDebug() << "Scans for meniscus: start_scan, end_scan: " << start_scan << ", " << end_scan; 
  
  for ( int i = start_scan; i < end_scan; i++ )
    {
      US_DataIO::Scan*  s = &data.scanData[ i ];
      
      double y_max_deviation = -1.0e99;
      double x_meniscus;
     
      //double left_fit = 6.5;
      double left_fit = meniscus_init + (range_right - meniscus_init)*0.5;
      
      int indexLeft_fit   = data.xindex( left_fit );
      int indexRight      = data.xindex( range_right );
           
      //qDebug() << "indexLeft_fit = " << indexLeft_fit << "; indexRight = " <<  indexRight; 


      /**********************************
      //Find a fit to straight line
      double sumX  = 0;
      double sumY  = 0;
      double sumXY = 0;
      double sumX2 = 0;
      int countPoints = 0;
      
      for ( int j = indexLeft_fit; j <= indexRight; j++ )
      {
	double y_curr = s->rvalues[ j ] * invert;
	double x_curr = data.xvalues[ j ];

	sumX  += x_curr;
	sumY  += y_curr;
	sumXY += x_curr*y_curr;
	sumX2 += x_curr*x_curr;

	++countPoints;
      }

      //Slope && mean: y(x) = Slope * X + YInt;
      double xMean = sumX / countPoints;
      double yMean = sumY / countPoints;
      double Slope = (sumXY - sumX * yMean) / (sumX2 - sumX * xMean);
      double YInt  = yMean - Slope * xMean;
      **********************************************/

      /********** TESTITNG *******************************************************/
      //Alternative: derive a curve passing through Left/Right indecies
      double Slope = (s->rvalues[ indexRight ] * invert - s->rvalues[ indexLeft_fit ] * invert ) / ( data.xvalues[ indexRight ] - data.xvalues[ indexLeft_fit ] );
      double YInt  = s->rvalues[ indexRight ] * invert - Slope *  data.xvalues[ indexRight ];
      /****************************************************************************/
      
      //qDebug() << "y(x) = Slope * X + YInt: " << Slope << "*X + " <<  YInt;

      //Find the biggest y-deviation from a line fit
      for ( int j = indexLeft_fit; j <= indexRight; j++ )
      {
	double y_curr = s->rvalues[ j ] * invert;
	double x_curr = data.xvalues[ j ];

	double y_fit  = Slope * x_curr + YInt;
 	double y_diff_abs = fabs( y_curr - y_fit );

	//qDebug() << "y_curr, x_curr, (Slope * x_curr + YInt), y_diff_abs: " << y_curr << ", " << x_curr << ", " << y_fit << ", " << y_diff_abs;

	if( y_diff_abs > y_max_deviation )
	  y_max_deviation = y_diff_abs;
      }

      //qDebug() << " y_max_deviation: " <<  y_max_deviation;

      //reverse order: search for a y-deviation (much) bigger than from a straight line fit based on selected points
      int indexLeft   = data.xindex( meniscus_init  );
      for ( int j = indexRight; j >= indexLeft; --j )
      	{
	  double y_curr = s->rvalues[ j ] * invert;
	  double x_curr = data.xvalues[ j ];

	  double y_diff_abs = fabs( y_curr - (Slope * x_curr + YInt) );

	  if( y_diff_abs > y_max_deviation * 1.5 )
	    {
	      x_meniscus = x_curr;
	      break; 
	    }
	}
      x_meniscuses.push_back( x_meniscus ); 
      
    }

  //Find simple average position of the meniscus
  for ( int i=0; i < x_meniscuses.size(); i++ )
    {
      meniscus_av += x_meniscuses[ i ];
      //qDebug() << "x_max: " << x_maxs[ i ] ;
    }
  
  meniscus_av /= x_meniscuses.size();
   
  //double meniscus_av = 5.95;
  return meniscus_av;
}


double US_Edit::find_meniscus_auto()
{
  double bottom_db     = centerpiece_info[ "bottom" ].toDouble(); 
  double pathlength_db = centerpiece_info[ "pathlen" ].toDouble();
  double angle_db      = centerpiece_info[ "angle" ].toDouble();

  //double aprofile_volume   = 460; // Just an example - to be read from AProfile, will be global
  
  double meniscus_init = sqrt( bottom_db*bottom_db - ( aprofile_volume*360/(1000*pathlength_db*angle_db*M_PI ) ) );     //Radians = Degrees * (M_PI/180.0)
  
  qDebug() << "Meniscus_init: " << meniscus_init << ", " << bottom_db << ", " << pathlength_db << ", " << angle_db << ", " << M_PI;

  //HARD CODED [for now?]:
  meniscus_init = 5.87; //Edge of the cell -- NEEDS TESTING

  if ( simulated_data )
    meniscus_init = 5.79;
  
  double meniscus_av = 0;

  // Scan Data Processing...
  //int size = data.pointCount();
  
  // For each scan in the last ~20% of scans:
  int start_scan = int(0.8*data.scanData.size());
  int end_scan   = data.scanData.size();
  
   
  QVector< double > x_maxs;

  qDebug() << "Scans for meniscus: start_scan, end_scan: " << start_scan << ", " << end_scan; 

  for ( int i = start_scan; i < end_scan; i++ )
    {
      US_DataIO::Scan*  s = &data.scanData[ i ];

      double y_max = -1.0e99;
      double x_max;

      int indexLeft   = data.xindex( meniscus_init  );
      //int indexRight  = data.xindex( meniscus_init + 0.5 );   // <---- OR should it just be the end data from AProfile ?

      //int indexRight  = data.xindex( range_right );
      //ALEXEY: maybe to be on safer side, take indexRight = meniscus_init  + ( aprofile_right - meniscus_init )/2.0
      //int indexRight = meniscus_init  + ( data.xindex( range_right ) - meniscus_init )/2.0;

      //int indexRight  = data.xindex( range_right );
      //ALEXEY: maybe to be on safer side, take indexRight = meniscus_init  + ( aprofile_right - meniscus_init )/2.0!!!
      int indexRight  = data.xindex( meniscus_init + ( range_right - meniscus_init )/1.5 );   

      qDebug() << "data.xindex( range_right ) = " << data.xindex( range_right );
      qDebug() << "indexLeft = " << indexLeft << "; indexRight = " <<  indexRight; 
	
      for ( int j = indexLeft; j <= indexRight; j++ )
      {
	double y_curr = s->rvalues[ j ] * invert;
	
	if ( y_curr > y_max )
	  {
	    y_max = y_curr;
	    x_max = data.xvalues[ j ];

	    //qDebug() << "y_max: " << y_max << ", x_max: " << x_max;
	  }
      }
      //Array of maximum values indexes for the last ~20% of scans in the region [ meniscus_left -- meniscus_left + 0.1 ]
      x_maxs.push_back( x_max ); 
    }

  //Find simple average position of the meniscus
  for ( int i=0; i < x_maxs.size(); i++ )
    {
      meniscus_av += x_maxs[ i ];
      //qDebug() << "x_max: " << x_maxs[ i ] ;
    }
  
  meniscus_av /= x_maxs.size();
  
  return meniscus_av;
}


// Scan for AUC entries in the database
int US_Edit::scan_db_auto( void )
{
   int  naucf        = 0;
   QStringList runIDs;
   QStringList infoDs;
   
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::warning( this, tr( "Connection Problem" ),
        tr( "Could not connect to database\n" ) + db.lastError() );
      return naucf;
   }

   // *** Determine 1st runType to process *************//
   if ( runType_combined_IP_RI )
     {
       QMap<QString, int>::iterator jj;
       for ( jj = runTypes_map.begin(); jj != runTypes_map.end(); ++jj )
	 {
	   if ( jj.value() )
	     {
	       filename_runID_auto = jj.key();
	       break;
	     }
	 }
     }
   //*****************************************************//

   QStringList query;
   query << "get_raw_desc_by_runID" << idInv_auto << filename_runID_auto;

   qDebug() << "IN scan_db_auto: query: " << query;
     
   
   db.query( query );

   while ( db.next() )
   {  // Accumulate data description objects from database information
      QString rawDataID = db.value( 0 ).toString();
      QString label     = db.value( 1 ).toString();
      QString filename  = db.value( 2 ).toString();
      QString date      = db.value( 5 ).toString() + " UTC";
      QString rawPers   = db.value( 6 ).toString();
      QString rawGUID   = db.value( 7 ).toString();
      QString runID     = filename.section( ".",  0, -6 );

      QString tripID    = filename.section( ".", -4, -2 );
      QString lkey      = runID + "." + tripID;
      QString idata     = label + "^" +
                          runID + "^" +
                          tripID + "^" +
                          filename + "^" +
                          rawGUID + "^" +
                          rawDataID + "^" +
                          date;

      runIDs << runID;    // Save each run
      infoDs << idata;    // Save concatenated description string
      naucf++;
   }

   // Create the data descriptions map
   create_descs_auto( runIDs, infoDs, naucf );

   // *** set key/value in the type_map for processed type to 0 && reset type_to_process ****//
   if ( runType_combined_IP_RI )
     runTypes_map[ filename_runID_auto ] = 0;
   
   
   return naucf;
}

// Create the data descriptions map with indecies and counts
void US_Edit::create_descs_auto( QStringList& runIDs, QStringList& infoDs, int naucf )
{
   datamap.clear();       // Clear label-descrip map
   infoDs .sort();        // Sort concat strings by label

   QString prunid  = "";
   int     tripndx = 1;

   for ( int ii = 0; ii < naucf; ii++ )
   {
      // Parse values in concatenation
      QString idata     = infoDs.at( ii );
      QString label     = idata.section( "^", 0, 0 );
      QString runID     = idata.section( "^", 1, 1 );
      QString tripID    = idata.section( "^", 2, 2 );
      QString filename  = idata.section( "^", 3, 3 );
      QString rawGUID   = idata.section( "^", 4, 4 );
      QString rawDataID = idata.section( "^", 5, 5 );
      QString date      = idata.section( "^", 6, 6 );
      QString dcheck    = idata.section( "^", 7, 7 );
      QString lkey      = runID + "." + tripID;
      tripndx           = ( runID == prunid ) ? ( tripndx + 1 ) : 1;
      prunid            = runID;

      // Fill the description object and set count,index
      DataDesc_auto ddesc;
      ddesc.label       = label;
      ddesc.runID       = runID;
      ddesc.tripID      = tripID;
      ddesc.filename    = filename;
      ddesc.rawGUID     = rawGUID;
      ddesc.date        = date;
      ddesc.dcheck      = dcheck;
      ddesc.DB_id       = rawDataID.toInt();
      ddesc.tripknt     = runIDs.count( runID );
      ddesc.tripndx     = tripndx;

//qDebug() << "CrDe: ii tknt" << ii << ddesc.tripknt << "lkey" << lkey;
      if ( datamap.contains( lkey ) )
      {  // Handle the case where the lkey already exists
         qDebug() << "*** DUPLICATE lkey" << lkey << "***";
         lkey              = lkey + "(2)";

         if ( datamap.contains( lkey ) )
         {  // Handle two duplicates
            lkey              = lkey.replace( "(2)", "(3)" );
         }
      }

      datamap[ lkey ]   = ddesc;
   }
   return;
}


// Load the data from the database
void US_Edit::load_db_auto( QList< DataDesc_auto >& sdescs )
{
   int     nerr  = 0;
   QString emsg;
   QString rdir  = US_Settings::resultDir();
   QString runID = sdescs.count() > 0 ? sdescs.at( 0 ).runID : "";
   workingDir    = rdir + "/" + runID;
   allData.clear();
   triples.clear();

   QDir work( rdir );
   work.mkdir( runID );

   // Connect to DB
   US_Passwd pw;
   US_DB2 db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to database\n " ) + db.lastError() );
      return;
   }

   for ( int ii = 0; ii < sdescs.count(); ii++ )
   {  // Loop to load selected data from the database
      US_DataIO::RawData rdata;
      DataDesc_auto ddesc     = sdescs.at( ii );
      int      idRaw     = ddesc.DB_id;
      QString  filebase  = ddesc.filename;
      QString  filename  = workingDir + "/" + filebase;
      QString  triple    = ddesc.tripID.replace( ".", " / " );
      QString  dcheck    = ddesc.dcheck;
      bool     dload_auc = true;
      int      stat      = 0;

      if ( QFile( filename ).exists() )
      {  // AUC file exists, do only download if checksum+size mismatch
         QString  fcheck    = US_Util::md5sum_file( filename );

         if ( dcheck.isEmpty() )
         {
            QStringList query;
            query << "get_rawData" << QString::number( idRaw );
            db.query( query );
            db.next();
            ddesc.dcheck       = db.value( 8 ).toString() + " " +
                                 db.value( 9 ).toString();
            dcheck             = ddesc.dcheck;
         }

         dload_auc          = ( fcheck != dcheck );
      }

      //emit progress( tr( "Loading triple " ) + triple );
      qApp->processEvents();

      // Download the DB record to a file (if need be)
      if ( dload_auc )
      {
         db.readBlobFromDB( filename, "download_aucData", idRaw );
         int stat           = db.lastErrno();

         if ( stat != US_DB2::OK )
         {
            nerr++;
            emsg += tr( "Error (%1) downloading to file %2\n" )
                    .arg( stat ).arg( filebase );
         }
      }

      // Read the raw record to memory
      stat = US_DataIO::readRawData( filename, rdata );

      if ( stat != US_DataIO::OK )
      {
         nerr++;
         emsg += tr( "Error (%1) reading file %2\n" )
                 .arg( stat ).arg( filebase );
      }

      // Accumulate lists of data and triples
      allData << rdata;
      triples << triple;
   }
}


// Load an AUC data set
void US_Edit::load( void )
{
   bool isLocal = ! disk_controls->db();
   reset();

   US_LoadAUC* dialog =
      new US_LoadAUC( isLocal, allData, triples, workingDir );

   connect( dialog, SIGNAL( progress      ( QString ) ),
            this,   SLOT  ( progress_load ( QString ) ) );
   connect( dialog, SIGNAL( changed       ( bool )    ),
            this,   SLOT  ( update_disk_db( bool )    ) );

   if ( dialog->exec() == QDialog::Rejected )  return;

   runID = workingDir.section( "/", -1, -1 );
DbgLv(1) << "Ld: runID" << runID << "wdir" << workingDir;
   sData     .clear();
   sd_offs   .clear();
   sd_knts   .clear();
   cb_triple->clear();
   files     .clear();
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
   triple_index = 0;
   data_index   = 0;
   
   le_info->setText( runID );

   data     = allData[ 0 ];
   dataType = QString( QChar( data.type[ 0 ] ) ) 
            + QString( QChar( data.type[ 1 ] ) );

   if ( dataType == "IP" )
   {
      lb_gaps->setText( tr( "Fringe Tolerance" ) );

      ct_gaps->setRange     ( 0.0, 20.0 );
      ct_gaps->setSingleStep( 0.001 );
      ct_gaps->setValue     ( 0.4 );
      ct_gaps->setNumButtons( 3 );

      connect( ct_gaps, SIGNAL( valueChanged        ( double ) ), 
                        SLOT  ( set_fringe_tolerance( double ) ) );
   }
   else
   {
      lb_gaps->setText( tr( "Threshold for Scan Gaps" ) );
      
      ct_gaps->disconnect   ();
      ct_gaps->setRange     ( 10.0, 100.0 );
      ct_gaps->setSingleStep( 10.0 );
      ct_gaps->setValue     ( 50.0 );
      ct_gaps->setNumButtons( 1 );
   }

   QString runtype = runID + "." + dataType;
DbgLv(1) << "Ld: runtype" << runtype;
   nwaveln         = 0;
   ncelchn         = 0;
   outData.clear();

   for ( int trx = 0; trx < triples.size(); trx++ )
   {  // Generate file names
      QString triple = QString( triples.at( trx ) ).replace( " / ", "." );
      QString file   = runtype + "." + triple + ".auc";
      files << file;

      // Save pointers as initial output data vector
      outData << &allData[ trx ];

      QString scell  = triple.section( ".", 0, 0 ).simplified();
      QString schan  = triple.section( ".", 1, 1 ).simplified();
      QString swavl  = triple.section( ".", 2, 2 ).simplified();

      if ( ! rawc_wvlns.contains( swavl ) )
      {  // Accumulate wavelengths in case this is MWL
         nwaveln++;
         rawc_wvlns << swavl;
      }

      nwavelo         = nwaveln;
      QString celchn  = scell + " / " + schan;

      if ( ! celchns.contains( celchn ) )
      {  // Accumulate cell/channel values in case this is MWL
         ncelchn++;
         celchns << celchn;
      }
   }
DbgLv(1) << "rawc_wvlns size" << rawc_wvlns.size() << nwaveln;
DbgLv(1) << " celchns    size" << celchns.size() << ncelchn;
   QMap<int, QString> m;
   for (const auto& s : rawc_wvlns) m[s.toInt()] = s;
   rawc_wvlns = QStringList(m.values());
   rawi_wvlns.clear();
   toti_wvlns.clear();

   for ( int wvx = 0; wvx < nwaveln; wvx++ )
   {
      int iwavl    = rawc_wvlns[ wvx ].toInt();
      rawi_wvlns << iwavl;
      toti_wvlns << iwavl;
   }

   workingDir   = workingDir + "/";
   QString file = workingDir + runtype + ".xml";
   expType      = "";
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
      if ( dbP == NULL )
      {
         US_Passwd pw;
         dbP          = new US_DB2( pw.getPasswd() );

         if ( dbP == NULL  ||  dbP->lastErrno() != US_DB2::OK )
         {
            QMessageBox::warning( this, tr( "Connection Problem" ),
              tr( "Could not connect to database\n" )
              + ( ( dbP != NULL ) ? dbP->lastError() : "" ) );
            dbP          = NULL;
            return;
         }
      }

      QStringList query;
      query << "get_experiment_info_by_runID" << runID 
            << QString::number( US_Settings::us_inv_ID() );

      dbP->query( query );
      dbP->next();
      expType    = dbP->value( 8 ).toString();
   }

   if ( expType.isEmpty() )  // if no experiment type, assume Velocity
      expType    = "Velocity";

   else                      // insure Ulll... form, e.g., "Equilibrium"
      expType    = expType.left( 1 ).toUpper() +
                   expType.mid(  1 ).toLower();


   // Set booleans for experiment type
   expIsVelo  = ( expType.compare( "Velocity",    Qt::CaseInsensitive ) == 0 );
   expIsEquil = ( expType.compare( "Equilibrium", Qt::CaseInsensitive ) == 0 );
   expIsDiff  = ( expType.compare( "Diffusion",   Qt::CaseInsensitive ) == 0 );
   expIsOther = ( !expIsVelo  &&  !expIsEquil  &&  !expIsDiff );
   expType    = expIsOther ? "Other" : expType;
   odlimit    = 1.8;
   init_includes();

   if ( expIsEquil )
   {  // Equilibrium
      lb_rpms    ->setVisible( true  );
      cb_rpms    ->setVisible( true  );
      pb_plateau ->setVisible( false );
      le_plateau ->setVisible( false ); 
      lb_baseline->setVisible( false ); 
      le_baseline->setVisible( false ); 
      lb_edtrsp  ->setVisible( true  );
      le_edtrsp  ->setVisible( true  );
      pb_reviewep->setVisible( true  );
      pb_nexteqtr->setVisible( true  );
      pb_write   ->setText( tr( "Save Edit Profiles" ) );

      sData.clear();
      US_DataIO::SpeedData  ssDat;
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
         data   = allData[ 0 ];
         ksd    = sd_knts[ 0 ];
         trip_rpms.clear();
         cb_rpms ->clear();
         for ( int ii = 0; ii < ksd; ii++ )
         {
            QString arpm = QString::number( sData[ ii ].speed );
            trip_rpms << arpm;
         }

         cb_rpms->addItems( trip_rpms );
      }

      pick     ->disconnect();
      connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                     SLOT  ( mouse   ( const QwtDoublePoint& ) ) );

      pb_priorEdits->disconnect();
      connect( pb_priorEdits, SIGNAL( clicked() ), SLOT( prior_equil() ) );
      plot_scan();

      connect( cb_rpms,   SIGNAL( currentIndexChanged( int ) ), 
                          SLOT  ( new_rpmval         ( int ) ) );
   }

   else
   {  // non-Equilibrium
      bool notMwl  = ( nwaveln < 3 );
      lb_rpms    ->setVisible( false );
      cb_rpms    ->setVisible( false );
//      pb_plateau ->setVisible( true  );
      le_plateau ->setVisible( true  ); 
      lb_baseline->setVisible( notMwl );
      le_baseline->setVisible( notMwl ); 
      lb_edtrsp  ->setVisible( false );
      le_edtrsp  ->setVisible( false );
      pb_reviewep->setVisible( false );
      pb_nexteqtr->setVisible( false );

      pb_write   ->setText( tr( "Save Current Edit Profile" ) );

      pb_priorEdits->disconnect();
      connect( pb_priorEdits, SIGNAL( clicked() ), SLOT( apply_prior() ) );
DbgLv(1) << "LD():  triples size" << triples.size();
      if ( notMwl )
         plot_current( 0 );
   }

   // Enable pushbuttons
   pb_details   ->setEnabled( true );
   pb_include   ->setEnabled( true );
   pb_exclusion ->setEnabled( true );
   pb_meniscus  ->setEnabled( true );
   pb_airGap    ->setEnabled( false );
//   pb_dataRange ->setEnabled( false );
//   pb_plateau   ->setEnabled( false );
   pb_noise     ->setEnabled( false );
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

   // Temperature check
   double              dt = 0.0;
   US_DataIO::RawData  triple;

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

   ntriple      = triples.size();
DbgLv(1) << " triples    size" << ntriple;
   editGUIDs .fill( "",     ntriple );
   editIDs   .fill( "",     ntriple );
   editFnames.fill( "none", ntriple );

   //isMwl        = ( nwaveln > 2  &&  ntriple > 12 );
   isMwl        = ( nwaveln > 1 );
   lrng_bycell  = false;         // Assume initially cell lambdas all the same
DbgLv(1) << "LD(): isMwl" << isMwl << "nwaveln" << nwaveln << toti_wvlns.size();

   if ( isMwl )
   {  // Set values related to MultiWaveLength
      connect_mwl_ctrls( false );

      // Load the internal object that keeps track of MWL data
DbgLv(1) << "IS-MWL:   load_mwl begun";
      mwl_data.load_mwl( allData );
DbgLv(1) << "IS-MWL:   load_mwl complete";

      // Initial export lambdas are input lambdas: save the input lists
      ncelchn      = mwl_data.countOf( "cellchann" );
      maxwavl      = rawc_wvlns.size();
DbgLv(1) << "IS-MWL: max wvlns size" << maxwavl;
      wavelns_i.clear();

      for ( int ccx = 0; ccx < ncelchn; ccx++ )
      {  // Save lambdas for each cell; flag if any cell-to-cell differences
         QVector< int > wvs;
         nwavelo      = mwl_data.lambdas( wvs, ccx );
         std::sort( wvs.begin(), wvs.end() );
         wavelns_i << wvs;

         if ( nwavelo != maxwavl )
         {
            lrng_bycell  = true;           // Flag based on count difference
         }

         else
         {
            for ( int wvx = 0; wvx < nwaveln; wvx++ )
            {
               if ( wvs[ wvx ] != toti_wvlns[ wvx ] )
               {
                  lrng_bycell  = true;     // Flag based on value difference
                  break;
               }
            }
         }
      } // END: cell scan loop

      lambdas_by_cell( 0 );

      nwavelo      = nwaveln;
      slambda      = toti_wvlns[ 0 ];
      elambda      = toti_wvlns[ nwaveln - 1 ];
      dlambda      = 1;
      le_ltrng ->setText( tr( "%1 raw: %2 %3 to %4" )
         .arg( nwaveln ).arg( chlamb ).arg( slambda ).arg( elambda ) );
      le_lxrng ->setText( tr( "%1 MWL exports: %2 %3 to %4,"
                              " raw index increment %5" )
         .arg( nwavelo ).arg( chlamb ).arg( slambda ).arg( elambda )
         .arg( dlambda ) );

      expd_radii .clear();
      expc_wvlns .clear();
      nwavelo      = mwl_data.lambdas( expi_wvlns, 0 );
      std::sort( expi_wvlns.begin(), expi_wvlns.end() );
DbgLv(1) << "IS-MWL:    new nwavelo" << nwavelo << expi_wvlns.count();

      // Initialize export wavelength lists for first channel
      for ( int wvx = 0; wvx < nwavelo; wvx++ )
      {
         expc_wvlns << QString::number( expi_wvlns[ wvx ] );
      }

      // Update wavelength lists in GUI elements
      cb_lstart->clear();
      cb_lend  ->clear();
      cb_lplot ->clear();
      cb_lstart->addItems( expc_wvlns );
      cb_lend  ->addItems( expc_wvlns );
      cb_lplot ->addItems( expc_wvlns );
      int lastx    = nwavelo - 1;
      plotndx      = nwavelo / 2;
      cb_lplot ->setCurrentIndex( plotndx );
      cb_lstart->setCurrentIndex( 0 );
      cb_lend  ->setCurrentIndex( lastx );
DbgLv(1) << "IS-MWL:  expi_wvlns size" << expi_wvlns.size() << nwaveln;

      // edata         = &allData[ 0 ];
      // nrpoint       = edata->pointCount();
//       int nscan     = edata->scanCount();
//       for ( int trx = 0; trx < allData.size(); trx++ )
//          nscan         = qMax( nscan, allData[ trx ].scanCount() );
//       int ndset     = ncelchn * nrpoint;
//       int ndpoint   = nscan * maxwavl;
// DbgLv(1) << "IS-MWL:   nrpoint nscan ndset ndpoint" << nrpoint << nscan
//  << ndset << ndpoint;

//       for ( int ii = 0; ii < nrpoint; ii++ )
//       {  // Update the list of radii that may be plotted
//          expd_radii << data.xvalues[ ii ];
//          expc_radii << QString().sprintf( "%.3f", data.xvalues[ ii ] );
//       }
// DbgLv(1) << "IS-MWL:  expd_radii size" << expd_radii.size() << nrpoint;



      set_data_over_lamda();


//       QVector< double > wrdata;
//       wrdata.fill( 0.0, ndpoint );
//       rdata .clear();
// DbgLv(1) << "IS-MWL:  wrdata size" << wrdata.size() << ndpoint;

//       for ( int ii = 0; ii < ndset; ii++ )
//       {  // Initialize the data vector that has wavelength as the x-axis
//          rdata << wrdata;
//       }
// DbgLv(1) << "IS-MWL:  rdata size" << rdata.size() << ndset;

      // Update wavelength-x-axis data vector with amplitude data points
      // The input has (ncelchn * nwaveln) data sets, each of which
      //   contains (nscan * nrpoint) data points.
      // The output has (ncelchn * nrpoint) data sets, each of which
      //   contains (nscan * nwaveln) data points.
//       int trx       = 0;
//       int trx_b     = 0;
//       rdata.clear();
//       for ( int ccx = 0; ccx < ncelchn; ccx++ )
//       {  // Handle each triple of AUC data
//          lambdas_by_cell( ccx );                      // Lambdas in cell
//          trx_b = trx;
//          int min_xval = -100000000;
//          int max_xval =  100000000;
//          int dx       = -1000;
//          for ( int jwx = 0; jwx < nwaveln; jwx++ )
//          {
//             edata         = &allData[ trx ];               // Triple data
//             min_xval = qMax(min_xval, static_cast<int> (qRound(edata->xvalues.first() * 1000)));
//             max_xval = qMin(max_xval, static_cast<int> (qRound(edata->xvalues.last() * 1000)));
//             double d = edata->xvalues.at(1) - edata->xvalues.at(0);
//             dx = qMax(dx, static_cast<int> (qRound(d * 1000)));
//             trx++;
//          }
//          QVector< double > xvals;
//          int xx = min_xval;
//          while (xx <= max_xval)
//          {
//             xvals << xx / 1000.0;
//             xx += dx;
//          }

//          rdata_xvals << xvals;
//          trx = trx_b;
//          // order of sorting data:
//          int nscans = allData[trx].scanCount();
//          int ndp = xvals.size() * nwaveln * nscans;
//          QVector< double > yvals(ndp, 0);
//          QVector< int > xvals_pos(nwaveln, 0);

//          for (int ii = 0; ii < xvals.size(); ii++)
//          {
//             trx = trx_b;
//             int txval = static_cast<int>(qRound(xvals.at(ii) * 1000));
//             for ( int jj = 0; jj < nwaveln; jj++ )
//             {  // Each wavelength in the current cell
//                 edata         = &allData[ trx ];               // Triple data
//                 int iwavl     = rawi_wvlns[ jj ];             // Wavelength value
//                 int wvx       = toti_wvlns.indexOf( iwavl );   // Wavelength index
//                 DbgLv(1) << "IS-MWL:   trx ccx wvx" << trx << ccx << wvx;

//                 int rpidx = -1;
//                 for ( int kk = xvals_pos.at(jj); kk < edata->pointCount(); kk++)
//                 {
//                     int cxval = static_cast<int>(qRound(edata->xvalues.at(kk) * 1000));
//                     if ( cxval == txval ) {
//                         rpidx = kk;
//                         xvals_pos[jj] = kk;
//                         break;
//                     }
//                 }
//                 if ( rpidx == -1 ) {
//                     xvals_pos[jj] = edata->pointCount();
//                     trx++;
//                     continue;
//                 }

//                 int idx_ii = ii * nwaveln * nscans;
//                 for ( int ss = 0; ss < nscans; ss++ )
//                 {
//                     double yval = edata->scanData.at(ss).rvalues.at(rpidx);
//                     int idx_ss = ss * nwaveln;
//                     int idx_wv = idx_ii + idx_ss + jj;
//                     yvals[idx_wv] = yval;
//                 }

//                 trx++;
//             } // END: input triples loop
//          }
//          rdata << yvals;
//       }
// DbgLv(1) << "IS-MWL:    Triples loop complete";













DbgLv(1) << "IS-MWL: celchns size" << celchns.size();
      lb_triple->setText( tr( "Cell / Channel" ) );
      cb_triple->disconnect();
      cb_triple->clear();
      cb_triple->addItems( celchns );
      connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
                          SLOT  ( new_triple         ( int ) ) );
      pb_nextChan->setEnabled( celchns.size() > 1 );

      odlimit   = 1.8;

      connect_mwl_ctrls( true );

      plot_mwl();
   } // END: isMwl=true
   else
   {
      new_triple( 0 );
//*DEBUG* Print times,omega^ts
 triple = allData[0];
 double timel = triple.scanData[0].rpm / 400.0;
 double rpmc  = 400.0;
 int    nstep = (int)timel;
 double w2ti  = 0.0;
 for ( int ii=0; ii<nstep; ii++ )
 {
  w2ti += sq( rpmc * M_PI / 30.0 );
  rpmc += 400.0;
 }
 for ( int ii=0; ii<triple.scanData.size(); ii++ )
 {
  US_DataIO::Scan* ds=&triple.scanData[ii];
  double timec = ds->seconds;
  double rpmc  = ds->rpm;
  w2ti += ( timec - timel ) * sq( rpmc * M_PI / 30.0 );
  qDebug() << "scan" << ii+1 << "delta-r rpm seconds" << ds->delta_r
   << rpmc << timec << "omega2t w2t-integ" << ds->omega2t << w2ti;
  if(ii==0)
  {
   double deltt = ds->omega2t / sq(rpmc*M_PI/30.0);
   double time1 = timel + deltt;
   qDebug() << "   scan 1 omega2t-implied time" << time1;
  }
  timel = timec;
 }
//*DEBUG* Print times,omega^ts
   }

   // Set up OD limit and any MWL controls
   ct_odlim->disconnect();
   ct_odlim->setValue( odlimit );
   connect( ct_odlim,  SIGNAL( valueChanged       ( double ) ),
            this,      SLOT  ( od_radius_limit    ( double ) ) );

   show_mwl_controls( isMwl );


   // DEBUG
   qDebug() << "meniscus: "   << meniscus;
   qDebug() << "meniscus_left: "   << meniscus_left;
   qDebug() << "range_left:"  << range_left;
   qDebug() << "range_right:" << range_right;
   qDebug() << "plateau: "    << plateau;
   qDebug() << "baseline: "   << baseline;
}


// Load an AUC data set already selected
void US_Edit::load_manual_auto( void )
{
   workingDir.chop(1);
   runID = workingDir.section( "/", -1, -1 );
DbgLv(1) << "Ld: runID" << runID << "wdir" << workingDir;
   sData     .clear();
   sd_offs   .clear();
   sd_knts   .clear();
   cb_triple->clear();
   files     .clear();
   
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
   triple_index = 0;
   data_index   = 0;
   
   le_info->setText( runID );

   data     = allData[ 0 ];
   dataType = QString( QChar( data.type[ 0 ] ) ) 
            + QString( QChar( data.type[ 1 ] ) );

   if ( dataType == "IP" )
   {
      lb_gaps->setText( tr( "Fringe Tolerance" ) );

      ct_gaps->setRange     ( 0.0, 20.0 );
      ct_gaps->setSingleStep( 0.001 );
      ct_gaps->setValue     ( 0.4 );
      ct_gaps->setNumButtons( 3 );

      connect( ct_gaps, SIGNAL( valueChanged        ( double ) ), 
                        SLOT  ( set_fringe_tolerance( double ) ) );
   }
   else
   {
      lb_gaps->setText( tr( "Threshold for Scan Gaps" ) );
      
      ct_gaps->disconnect   ();
      ct_gaps->setRange     ( 10.0, 100.0 );
      ct_gaps->setSingleStep( 10.0 );
      ct_gaps->setValue     ( 50.0 );
      ct_gaps->setNumButtons( 1 );
   }

   QString runtype = runID + "." + dataType;
DbgLv(1) << "Ld: runtype" << runtype;
   nwaveln         = 0;
   ncelchn         = 0;
   outData.clear();

   for ( int trx = 0; trx < triples.size(); trx++ )
   {  // Generate file names
      QString triple = QString( triples.at( trx ) ).replace( " / ", "." );
      QString file   = runtype + "." + triple + ".auc";
      files << file;

      // Save pointers as initial output data vector
      outData << &allData[ trx ];

      QString scell  = triple.section( ".", 0, 0 ).simplified();
      QString schan  = triple.section( ".", 1, 1 ).simplified();
      QString swavl  = triple.section( ".", 2, 2 ).simplified();

      if ( ! rawc_wvlns.contains( swavl ) )
      {  // Accumulate wavelengths in case this is MWL
         nwaveln++;
         rawc_wvlns << swavl;
      }

      nwavelo         = nwaveln;
      QString celchn  = scell + " / " + schan;

      if ( ! celchns.contains( celchn ) )
      {  // Accumulate cell/channel values in case this is MWL
         ncelchn++;
         celchns << celchn;
      }
   }
DbgLv(1) << "rawc_wvlns size" << rawc_wvlns.size() << nwaveln;
DbgLv(1) << " celchns    size" << celchns.size() << ncelchn;
   QMap<int, QString> m;
   for (const auto& s : rawc_wvlns) m[s.toInt()] = s;
   rawc_wvlns = QStringList(m.values());
   rawi_wvlns.clear();
   toti_wvlns.clear();

   for ( int wvx = 0; wvx < nwaveln; wvx++ )
   {
      int iwavl    = rawc_wvlns[ wvx ].toInt();
      rawi_wvlns << iwavl;
      toti_wvlns << iwavl;
   }

   workingDir   = workingDir + "/";
   QString file = workingDir + runtype + ".xml";
   expType      = "";
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
      if ( dbP == NULL )
      {
         US_Passwd pw;
         dbP          = new US_DB2( pw.getPasswd() );

         if ( dbP == NULL  ||  dbP->lastErrno() != US_DB2::OK )
         {
            QMessageBox::warning( this, tr( "Connection Problem" ),
              tr( "Could not connect to database\n" )
              + ( ( dbP != NULL ) ? dbP->lastError() : "" ) );
            dbP          = NULL;
            return;
         }
      }

      QStringList query;
      query << "get_experiment_info_by_runID" << runID 
            << QString::number( US_Settings::us_inv_ID() );

      dbP->query( query );
      dbP->next();
      expType    = dbP->value( 8 ).toString();
   }

   if ( expType.isEmpty() )  // if no experiment type, assume Velocity
      expType    = "Velocity";

   else                      // insure Ulll... form, e.g., "Equilibrium"
      expType    = expType.left( 1 ).toUpper() +
                   expType.mid(  1 ).toLower();


   // Set booleans for experiment type
   expIsVelo  = ( expType.compare( "Velocity",    Qt::CaseInsensitive ) == 0 );
   expIsEquil = ( expType.compare( "Equilibrium", Qt::CaseInsensitive ) == 0 );
   expIsDiff  = ( expType.compare( "Diffusion",   Qt::CaseInsensitive ) == 0 );
   expIsOther = ( !expIsVelo  &&  !expIsEquil  &&  !expIsDiff );
   expType    = expIsOther ? "Other" : expType;
   odlimit    = 1.8;
   init_includes();

   if ( expIsEquil )
   {  // Equilibrium
      lb_rpms    ->setVisible( true  );
      cb_rpms    ->setVisible( true  );
      pb_plateau ->setVisible( false );
      le_plateau ->setVisible( false ); 
      lb_baseline->setVisible( false ); 
      le_baseline->setVisible( false ); 
      lb_edtrsp  ->setVisible( true  );
      le_edtrsp  ->setVisible( true  );
      pb_reviewep->setVisible( true  );
      pb_nexteqtr->setVisible( true  );
      pb_write   ->setText( tr( "Save Edit Profiles" ) );

      sData.clear();
      US_DataIO::SpeedData  ssDat;
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
         data   = allData[ 0 ];
         ksd    = sd_knts[ 0 ];
         trip_rpms.clear();
         cb_rpms ->clear();
         for ( int ii = 0; ii < ksd; ii++ )
         {
            QString arpm = QString::number( sData[ ii ].speed );
            trip_rpms << arpm;
         }

         cb_rpms->addItems( trip_rpms );
      }

      pick     ->disconnect();
      connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                     SLOT  ( mouse   ( const QwtDoublePoint& ) ) );

      pb_priorEdits->disconnect();
      connect( pb_priorEdits, SIGNAL( clicked() ), SLOT( prior_equil() ) );
      plot_scan();

      connect( cb_rpms,   SIGNAL( currentIndexChanged( int ) ), 
                          SLOT  ( new_rpmval         ( int ) ) );
   }

   else
   {  // non-Equilibrium
      bool notMwl  = ( nwaveln < 3 );
      lb_rpms    ->setVisible( false );
      cb_rpms    ->setVisible( false );
//      pb_plateau ->setVisible( true  );
      le_plateau ->setVisible( true  ); 
      lb_baseline->setVisible( notMwl );
      le_baseline->setVisible( notMwl ); 
      lb_edtrsp  ->setVisible( false );
      le_edtrsp  ->setVisible( false );
      pb_reviewep->setVisible( false );
      pb_nexteqtr->setVisible( false );

      pb_write   ->setText( tr( "Save Current Edit Profile" ) );

      pb_priorEdits->disconnect();
      connect( pb_priorEdits, SIGNAL( clicked() ), SLOT( apply_prior() ) );
DbgLv(1) << "LD():  triples size" << triples.size();
      if ( notMwl )
         plot_current( 0 );
   }

   // Enable pushbuttons
   pb_details   ->setEnabled( true );
   pb_include   ->setEnabled( true );
   pb_exclusion ->setEnabled( true );
   pb_meniscus  ->setEnabled( true );
   pb_airGap    ->setEnabled( false );
//   pb_dataRange ->setEnabled( false );
//   pb_plateau   ->setEnabled( false );
   pb_noise     ->setEnabled( false );
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

   // Temperature check
   double              dt = 0.0;
   US_DataIO::RawData  triple;

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

   ntriple      = triples.size();
DbgLv(1) << " triples    size" << ntriple;
   editGUIDs .fill( "",     ntriple );
   editIDs   .fill( "",     ntriple );
   editFnames.fill( "none", ntriple );

   //isMwl        = ( nwaveln > 2  &&  ntriple > 12 );
   isMwl        = ( nwaveln > 1 );
   lrng_bycell  = false;         // Assume initially cell lambdas all the same
DbgLv(1) << "LD(): isMwl" << isMwl << "nwaveln" << nwaveln << toti_wvlns.size();

   if ( isMwl )
   {  // Set values related to MultiWaveLength
      connect_mwl_ctrls( false );

      // Load the internal object that keeps track of MWL data
DbgLv(1) << "IS-MWL:   load_mwl begun";
      mwl_data.load_mwl( allData );
DbgLv(1) << "IS-MWL:   load_mwl complete";

      // Initial export lambdas are input lambdas: save the input lists
      ncelchn      = mwl_data.countOf( "cellchann" );
      maxwavl      = rawc_wvlns.size();
DbgLv(1) << "IS-MWL: max wvlns size" << maxwavl;
      wavelns_i.clear();

      for ( int ccx = 0; ccx < ncelchn; ccx++ )
      {  // Save lambdas for each cell; flag if any cell-to-cell differences
         QVector< int > wvs;
         nwavelo      = mwl_data.lambdas( wvs, ccx );
         std::sort( wvs.begin(), wvs.end() );
         wavelns_i << wvs;

         if ( nwavelo != maxwavl )
         {
            lrng_bycell  = true;           // Flag based on count difference
         }

         else
         {
            for ( int wvx = 0; wvx < nwaveln; wvx++ )
            {
               if ( wvs[ wvx ] != toti_wvlns[ wvx ] )
               {
                  lrng_bycell  = true;     // Flag based on value difference
                  break;
               }
            }
         }
      } // END: cell scan loop

      lambdas_by_cell( 0 );

      nwavelo      = nwaveln;
      slambda      = toti_wvlns[ 0 ];
      elambda      = toti_wvlns[ nwaveln - 1 ];
      dlambda      = 1;
      le_ltrng ->setText( tr( "%1 raw: %2 %3 to %4" )
         .arg( nwaveln ).arg( chlamb ).arg( slambda ).arg( elambda ) );
      le_lxrng ->setText( tr( "%1 MWL exports: %2 %3 to %4,"
                              " raw index increment %5" )
         .arg( nwavelo ).arg( chlamb ).arg( slambda ).arg( elambda )
         .arg( dlambda ) );

      expd_radii .clear();
      expc_wvlns .clear();
      nwavelo      = mwl_data.lambdas( expi_wvlns, 0 );
      std::sort( expi_wvlns.begin(), expi_wvlns.end() );
DbgLv(1) << "IS-MWL:    new nwavelo" << nwavelo << expi_wvlns.count();

      // Initialize export wavelength lists for first channel
      for ( int wvx = 0; wvx < nwavelo; wvx++ )
      {
         expc_wvlns << QString::number( expi_wvlns[ wvx ] );
      }

      // Update wavelength lists in GUI elements
      cb_lstart->clear();
      cb_lend  ->clear();
      cb_lplot ->clear();
      cb_lstart->addItems( expc_wvlns );
      cb_lend  ->addItems( expc_wvlns );
      cb_lplot ->addItems( expc_wvlns );
      int lastx    = nwavelo - 1;
      plotndx      = nwavelo / 2;
      cb_lplot ->setCurrentIndex( plotndx );
      cb_lstart->setCurrentIndex( 0 );
      cb_lend  ->setCurrentIndex( lastx );
DbgLv(1) << "IS-MWL:  expi_wvlns size" << expi_wvlns.size() << nwaveln;

      set_data_over_lamda();

//       edata         = &allData[ 0 ];
//       nrpoint       = edata->pointCount();
//       int nscan     = edata->scanCount();
//       for ( int trx = 0; trx < allData.size(); trx++ )
//          nscan         = qMax( nscan, allData[ trx ].scanCount() );
//       int ndset     = ncelchn * nrpoint;
//       int ndpoint   = nscan * maxwavl;
// DbgLv(1) << "IS-MWL:   nrpoint nscan ndset ndpoint" << nrpoint << nscan
//  << ndset << ndpoint;

//       for ( int ii = 0; ii < nrpoint; ii++ )
//       {  // Update the list of radii that may be plotted
//          expd_radii << data.xvalues[ ii ];
//          expc_radii << QString().sprintf( "%.3f", data.xvalues[ ii ] );
//       }
// DbgLv(1) << "IS-MWL:  expd_radii size" << expd_radii.size() << nrpoint;

//       QVector< double > wrdata;
//       wrdata.fill( 0.0, ndpoint );
//       rdata .clear();
// DbgLv(1) << "IS-MWL:  wrdata size" << wrdata.size() << ndpoint;

//       for ( int ii = 0; ii < ndset; ii++ )
//       {  // Initialize the data vector that has wavelength as the x-axis
//          rdata << wrdata;
//       }
// DbgLv(1) << "IS-MWL:  rdata size" << rdata.size() << ndset;

//       // Update wavelength-x-axis data vector with amplitude data points
//       // The input has (ncelchn * nwaveln) data sets, each of which
//       //   contains (nscan * nrpoint) data points.
//       // The output has (ncelchn * nrpoint) data sets, each of which
//       //   contains (nscan * nwaveln) data points.
//       int trx       = 0;

//       for ( int ccx = 0; ccx < ncelchn; ccx++ )
//       {  // Handle each triple of AUC data
//          lambdas_by_cell( ccx );                      // Lambdas in cell

//          for ( int jwx = 0; jwx < nwaveln; jwx++ )
//          {  // Each wavelength in the current cell
//             edata         = &allData[ trx ];               // Triple data
//             int iwavl     = rawi_wvlns[ jwx ];             // Wavelength value
//             int wvx       = toti_wvlns.indexOf( iwavl );   // Wavelength index
// DbgLv(1) << "IS-MWL:   trx ccx wvx" << trx << ccx << wvx;

//             for ( int scx = 0; scx < edata->scanCount(); scx++ )
//             {  // Each scan of a triple
//                US_DataIO::Scan* scan  = &edata->scanData[ scx ];
//                int odx       = ccx * nrpoint;         // Output dataset index
//                int opx       = scx * maxwavl + wvx;   // Output point index
// DbgLv(2) << "IS-MWL:    scx odx opx" << scx << odx << opx;
//                for ( int rax = 0; rax < nrpoint; rax++ )
//                {  // Store ea. radius data point as a wavelength point in a scan
//                   rdata[ odx++ ][ opx ]  = scan->rvalues[ rax ];
//                } // END: radius points loop
//             } // END: scans loop

//             trx++;
//          } // END: input triples loop
//       } // END: input celchn loop
// DbgLv(1) << "IS-MWL:    Triples loop complete";

DbgLv(1) << "IS-MWL: celchns size" << celchns.size();
      lb_triple->setText( tr( "Cell / Channel" ) );
      cb_triple->disconnect();
      cb_triple->clear();
      cb_triple->addItems( celchns );
      connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
                          SLOT  ( new_triple         ( int ) ) );
      pb_nextChan->setEnabled( celchns.size() > 1 );

      odlimit   = 1.8;

      connect_mwl_ctrls( true );

      plot_mwl();
   } // END: isMwl=true
   else
   {
      new_triple( 0 );
//*DEBUG* Print times,omega^ts
 triple = allData[0];
 double timel = triple.scanData[0].rpm / 400.0;
 double rpmc  = 400.0;
 int    nstep = (int)timel;
 double w2ti  = 0.0;
 for ( int ii=0; ii<nstep; ii++ )
 {
  w2ti += sq( rpmc * M_PI / 30.0 );
  rpmc += 400.0;
 }
 for ( int ii=0; ii<triple.scanData.size(); ii++ )
 {
  US_DataIO::Scan* ds=&triple.scanData[ii];
  double timec = ds->seconds;
  double rpmc  = ds->rpm;
  w2ti += ( timec - timel ) * sq( rpmc * M_PI / 30.0 );
  qDebug() << "scan" << ii+1 << "delta-r rpm seconds" << ds->delta_r
   << rpmc << timec << "omega2t w2t-integ" << ds->omega2t << w2ti;
  if(ii==0)
  {
   double deltt = ds->omega2t / sq(rpmc*M_PI/30.0);
   double time1 = timel + deltt;
   qDebug() << "   scan 1 omega2t-implied time" << time1;
  }
  timel = timec;
 }
//*DEBUG* Print times,omega^ts
   }

   // Set up OD limit and any MWL controls
   ct_odlim->disconnect();
   ct_odlim->setValue( odlimit );
   connect( ct_odlim,  SIGNAL( valueChanged       ( double ) ),
            this,      SLOT  ( od_radius_limit    ( double ) ) );

   show_mwl_controls( isMwl );


   // DEBUG
   qDebug() << "meniscus: "   << meniscus;
   qDebug() << "meniscus_left: "   << meniscus_left;
   qDebug() << "range_left:"  << range_left;
   qDebug() << "range_right:" << range_right;
   qDebug() << "plateau: "    << plateau;
   qDebug() << "baseline: "   << baseline;
}

void US_Edit::set_data_over_lamda() {
   int trx       = 0;
   int trx_b     = 0;
   rdata.clear();
   for ( int ccx = 0; ccx < ncelchn; ccx++ )
   {  // Handle each triple of AUC data
      lambdas_by_cell( ccx );                      // Lambdas in cell
      trx_b = trx;
      int min_xval = -100000000;
      int max_xval =  100000000;
      int dx       = 1; // preventing that dx becomes 0 and causing an infinite loop
      for ( int jwx = 0; jwx < nwaveln; jwx++ )
      {
         edata         = &allData[ trx ];               // Triple data
         min_xval = qMax(min_xval, static_cast<int> (qRound(edata->xvalues.first() * 1000)));
         max_xval = qMin(max_xval, static_cast<int> (qRound(edata->xvalues.last() * 1000)));
         double d = edata->xvalues.at(1) - edata->xvalues.at(0);
         dx = qMax(dx, static_cast<int> (qRound(d * 1000)));
         trx++;
      }
      QVector< double > xvals;
      int xx = min_xval;
      while (xx <= max_xval)
      {
         xvals << xx / 1000.0;
         xx += dx;
      }

      rdata_xvals << xvals;
      trx = trx_b;
      // order of sorting data:
      int nscans = allData[trx].scanCount();
      int ndp = xvals.size() * nwaveln * nscans;
      QVector< double > yvals(ndp, 0);
      QVector< int > xvals_pos(nwaveln, 0);

      for (int ii = 0; ii < xvals.size(); ii++)
      {
         trx = trx_b;
         int txval = static_cast<int>(qRound(xvals.at(ii) * 1000));
         for ( int jj = 0; jj < nwaveln; jj++ )
         {  // Each wavelength in the current cell
            edata         = &allData[ trx ];               // Triple data
            int iwavl     = rawi_wvlns[ jj ];             // Wavelength value
            int wvx       = toti_wvlns.indexOf( iwavl );   // Wavelength index
            DbgLv(1) << "IS-MWL:   trx ccx wvx" << trx << ccx << wvx;

            int rpidx = -1;
            for ( int kk = xvals_pos.at(jj); kk < edata->pointCount(); kk++)
            {
               int cxval = static_cast<int>(qRound(edata->xvalues.at(kk) * 1000));
               if ( cxval == txval ) {
                  rpidx = kk;
                  xvals_pos[jj] = kk;
                  break;
               }
            }
            if ( rpidx == -1 ) {
               xvals_pos[jj] = edata->pointCount();
               trx++;
               continue;
            }

            int idx_ii = ii * nwaveln * nscans;
            for ( int ss = 0; ss < nscans; ss++ )
            {
               double yval = edata->scanData.at(ss).rvalues.at(rpidx);
               int idx_ss = ss * nwaveln;
               int idx_wv = idx_ii + idx_ss + jj;
               yvals[idx_wv] = yval;
            }

            trx++;
         } // END: input triples loop
      }
      rdata << yvals;
   }
   DbgLv(1) << "IS-MWL:    Triples loop complete";

   expc_radii.clear();
   expd_radii.clear();
   foreach (double xval, rdata_xvals.at(0)) {
       expc_radii << QString::number(xval);
       expd_radii << xval;
   }


}


// Set pushbutton colors
void US_Edit::set_pbColors( QPushButton* pb )
{
   QPalette p = US_GuiSettings::pushbColor();
   
   pb_meniscus ->setPalette( p );
   pb_airGap   ->setPalette( p );
//   pb_dataRange->setPalette( p );
   pb_dataEnd  ->setPalette( p );
   pb_plateau  ->setPalette( p );

   if ( pb != NULL )
   {
      p.setColor( QPalette::Button, Qt::green );
      pb->setPalette( p );
   }
}

// Plot the current data set
void US_Edit::plot_current( int index )
{
   if ( isMwl )
   {
      plot_mwl();
      return;
   }

   // Read the data
   QString     triple  = triples.at( index );
   QStringList parts   = triple.split( " / " );

   QString     cell    = parts[ 0 ];
   QString     channel = parts[ 1 ];
   QString     wl      = parts[ 2 ];

   QString     desc    = data.description;

   dataType = QString( QChar( data.type[ 0 ] ) ) 
            + QString( QChar( data.type[ 1 ] ) );

   if ( dataType == "IP" )
   {
      QStringList sl = desc.split( "," );
      sl.removeFirst();
      desc = sl.join( "," ).trimmed();
   }

   le_info->setText( runID + "  (" + desc + ")" );

   // Plot Title
   QString title23 = tr( "Run ID: %1\n"
                         "Cell: %2  Channel: %3  Wavelength: %4" )
      .arg( runID ).arg( cell ).arg( channel ).arg( wl );
   QString title;

   if ( dataType == "RA" )
   {
      title = "Radial Absorbance Data\n" + title23;
   }
   else if ( dataType == "RI" )
   {
      title = "Radial Intensity Data\n" + title23;
      data_plot->setAxisTitle( QwtPlot::yLeft, tr( "Intensity " ) );
   }
   else if ( dataType == "IP" )
   {

      title = "Radial Interference Data\n" + title23;
      data_plot->setAxisTitle( QwtPlot::yLeft, tr( "Fringes " ) );

      // Enable Air Gap
      pb_airGap->setHidden( false );
      le_airGap->setHidden( false );
   }
   else if ( dataType == "FI" )
   {
      title = "Fluorescence Intensity Data\n" + title23;
      data_plot->setAxisTitle( QwtPlot::yLeft, tr( "Fluorescence Intensity " ) );
   }
   else 
      title = "File type not recognized";

   data_plot->setTitle( title );

   // Initialize include list
   init_includes();

   // Plot current data for cell / channel / wavelength triple
   plot_all();

   // Set the Scan spin boxes
   ct_from->setMinimum( 0.0 );
   ct_from->setMaximum( data.scanData.size() );

   ct_to  ->setMinimum( 0.0 );
   ct_to  ->setMaximum( data.scanData.size() );

   pick   ->disconnect();
   connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                  SLOT  ( mouse   ( const QwtDoublePoint& ) ) );
}

// Re-plot
void US_Edit::replot( void )
{
  qDebug() << "In replot: step -- " << step;
  
   switch( step )
   {
      case FINISHED:
      case PLATEAU:
         plot_range();
         break;

      case BASELINE:
         plot_last();
         break;

      default:   // MENISCUS, AIRGAP, RANGE
         plot_all();
         break;
   }
}

// Handle a mouse click according to the current pick step
void US_Edit::mouse( const QwtDoublePoint& p )
{
   double maximum = -1.0e99;

   switch ( step )
   {
      case MENISCUS:
         if ( dataType == "IP" )
         {
            meniscus = p.x();
            // Un-zoom
            if ( plot->btnZoom->isChecked() )
               plot->btnZoom->setChecked( false );

            draw_vline( meniscus );
         }

         else if ( expIsEquil || men_1click )
         {  // Equilibrium

	   qDebug() << "Meniscus, expEquil OR men_1click ";
            meniscus_left = p.x();
            int ii        = data.xindex( meniscus_left );
            draw_vline( meniscus_left );
            meniscus      = data.radius( ii );
            range_left    = meniscus + _RNGLEFT_OFFSET_;
            le_meniscus ->setText( QString::number( meniscus,   'f', 8 ) );
            le_dataStart->setText( QString::number( range_left, 'f', 8 ) );

            data_plot->replot();

            pb_meniscus->setIcon( check );
         
//            pb_dataRange->setEnabled( true );
        
            next_step();
            break;
         }

         else if ( meniscus_left == 0.0  )
         {
            meniscus_left = p.x();
            draw_vline( meniscus_left );
            break;
         }
         else
         {
            // Sometime we get two clicks
            if ( qAbs( p.x() - meniscus_left ) < 0.005 ) return;

            double meniscus_right = p.x();
            
            // Swap values if necessary.  Use a macro in us_math.h
            if ( meniscus_right < meniscus_left )
               swap_double( meniscus_left, meniscus_right );

            // Find the radius for the max value
            maximum = -1.0e99;
            US_DataIO::Scan* s;

            for ( int i = 0; i < data.scanData.size(); i++ )
            {
               if ( ! includes.contains( i ) ) continue;

               s         = &data.scanData[ i ];
               int start = data.xindex( meniscus_left  );
               int end   = data.xindex( meniscus_right );

               for ( int j = start; j <= end; j++ )
               {
                  if ( maximum < s->rvalues[ j ] )
                  {
                     maximum  = s->rvalues[ j ];
                     meniscus = data.radius( j );
                  }
               }
            }

            // Remove the left line
            if ( v_line != NULL )
            {
               v_line->detach();
               delete v_line;
               v_line = NULL;
            }
         }

         // Display the values
         range_left    = meniscus + _RNGLEFT_OFFSET_;
         le_meniscus ->setText( QString::number( meniscus,   'f', 8 ) );
         le_dataStart->setText( QString::number( range_left, 'f', 8 ) );

         // Create a marker
         if ( dataType != "IP" )
         {
            marker = new QwtPlotMarker;
            QBrush brush( Qt::white );
            QPen   pen  ( brush, 2.0 );
            QwtSymbol * symb = new QwtSymbol( QwtSymbol::Cross, 
                                              brush, pen, QSize ( 8, 8 ) );
            marker->setValue ( meniscus, maximum );
            marker->setSymbol( symb );
            marker->attach   ( data_plot );
         }

         data_plot->replot();

         pb_meniscus->setIcon( check );
         
         if ( dataType == "IP" )
         {
            pb_airGap->setEnabled( true );
            set_airGap();
         }
         else
//            pb_dataRange->setEnabled( true );
        
         next_step();
         break;

      case AIRGAP:
         if ( airGap_left == 0.0 )
         {
            airGap_left = p.x();
            draw_vline( airGap_left );
         }
         else
         {
            // Sometime we get two clicks
            if ( qAbs( p.x() - airGap_left ) < 0.005 ) return;

            airGap_right = p.x();

            if ( airGap_right < airGap_left ) 
               swap_double( airGap_left, airGap_right );

            US_DataIO::EditValues  edits;
            edits.airGapLeft  = airGap_left;
            edits.airGapRight = airGap_right;

            QList< int > excludes;
            
            for ( int i = 0; i < data.scanData.size(); i++ )
               if ( ! includes.contains( i ) ) edits.excludes << i;

DbgLv(1) << "AGap: L R" << airGap_left << airGap_right << " AdjIntf";
            US_DataIO::adjust_interference( data, edits );

            // Un-zoom
            if ( plot->btnZoom->isChecked() )
               plot->btnZoom->setChecked( false );

            // Display the data
            QString wkstr;
            le_airGap->setText( wkstr.sprintf( "%.3f - %.3f", 
                     airGap_left, airGap_right ) );

            step          = RANGE;
            range_left    = meniscus + _RNGLEFT_OFFSET_;
            le_dataStart->setText( QString::number( range_left, 'f', 8 ) );
DbgLv(1) << "AGap:  plot_range()";

            plot_range();

            qApp->processEvents();
            
            pb_airGap   ->setIcon( check );
            pb_dataRange->setEnabled( true );

            next_step();
         }

         break;

      case RANGE:
         if ( range_left == 0.0 )
         {
            if ( v_line != NULL )
            {
               v_line->detach();
               delete v_line;
               v_line = NULL;
            }

            if ( p.x() <= meniscus )
            {
               le_dataRange->setText( tr( "**overlaps meniscus**" ) );
               break;
            }

            range_left  = radius_indexed( p.x() );
            draw_vline( range_left );
            break;
         }

         else if ( range_right == 0.0 )
         {
            if ( v_line != NULL )
            {
               v_line->detach();
               delete v_line;
               v_line = NULL;
            }

            range_right  = radius_indexed( p.x() );

            if ( ( range_right - meniscus ) < 0.4 )
            {
               range_right  = 9.0;
               QMessageBox::critical( this,
                  tr( "Range Right Error" ),
                  tr( "The newest Edit does away with the need to\n"
                      "select Range Left and Right or Plateau.\n"
                      "Besides Meniscus, only Range Right need be\n"
                      "selected by ctrl-click.\n\n"
                      "Please ctrl-click to select the data end." ) );
//               qApp->processEvents();
               next_step();
               return;
            }

            draw_vline( range_right );
            le_dataEnd  ->setText( QString::number( range_right, 'f', 8 ) );
            plateau      = range_right - _PLATEAU_OFFSET_;
            le_plateau  ->setText( QString::number( plateau,     'f', 8 ) );


            step = PLATEAU;

            plot_range();

            qApp->processEvents();
            break;
         }

         else
         {
            // Sometime we get two clicks
            if ( qAbs( p.x() - range_left ) < 0.020 ) return;

            range_right = radius_indexed( p.x() );

            if ( range_right < range_left )
               swap_double( range_left, range_right );

            if ( dataType == "IP" )
            {
               US_DataIO::EditValues  edits;
               edits.rangeLeft    = range_left;
               edits.rangeRight   = range_right;
               edits.gapTolerance = ct_gaps->value();

               QList< int > excludes;
               
               for ( int i = 0; i < data.scanData.size(); i++ )
                  if ( ! includes.contains( i ) ) edits.excludes << i;
            
               US_DataIO::calc_integral( data, edits );
            }
            
            // Display the data
            QString wkstr;
//            le_dataRange->setText( wkstr.sprintf( "%.3f - %.3f", 
//                     range_left, range_right ) );
            le_dataStart->setText( QString::number( range_left,  'f', 8 ) );
            le_dataEnd  ->setText( QString::number( range_right, 'f', 8 ) );
            plateau      = range_right - _PLATEAU_OFFSET_;
            le_plateau  ->setText( QString::number( plateau,     'f', 3 ) );

            step = PLATEAU;
            plot_range();
            pb_report  ->setEnabled( true );

            qApp->processEvents();

            // Skip the gap check for interference data
            if ( dataType != "IP" ) gap_check();
            
//            pb_dataRange->setIcon( check );
//            pb_plateau  ->setEnabled( true );
            pb_dataEnd  ->setIcon( check );
            pb_dataEnd  ->setEnabled( true );
            pb_noise    ->setEnabled( true );
            pb_spikes   ->setEnabled( true );

            if ( ! expIsEquil )
            {  // non-Equilibrium

	        qDebug() << "In RANGES, nonEquilibrium, enabling next_step()";
               next_step();
            }

            else
            {  // Equilibrium

               int index    = cb_triple->currentIndex();
               int row      = cb_rpms  ->currentIndex();
               int jsd      = sd_offs[ index ] + row;
               QString arpm = cb_rpms->itemText( row );

               if ( sData[ jsd ].meniscus == 0.0  &&
                     meniscus > 0.0 )
                  total_edits++;

               sData[ jsd ].meniscus  = meniscus;
               sData[ jsd ].dataLeft  = range_left;
               sData[ jsd ].dataRight = range_right;

               if ( ++row >= cb_rpms->count() )
               {
                  if ( ++index < cb_triple->count() )
                  {
                     cb_triple->setCurrentIndex( index );
                     step         = MENISCUS;
                     QString trsp =
                        cb_triple->currentText() + " : " + trip_rpms[ 0 ];
                     le_edtrsp->setText( trsp );

                     data = *outData[ index ];
                     cb_rpms->clear();

                     for ( int ii = 0; ii < data.scanData.size(); ii++ )
                     {
                        QString arpm =
                           QString::number( data.scanData[ ii ].rpm );

                        if ( ! trip_rpms.contains( arpm ) )
                           trip_rpms << arpm;
                     }

                     cb_rpms->addItems( trip_rpms );
                     cb_rpms->setCurrentIndex( 0 );

                     set_meniscus();
                  }

                  else
                  {
                     pb_report  ->setEnabled( true );
                     pb_write   ->setEnabled( true );

		     if ( us_edit_auto_mode_manual )
		       pb_pass->setEnabled( true );

		     ck_writemwl->setEnabled( true );
                     pb_reviewep->setEnabled( true );
                     pb_nexteqtr->setEnabled( true );
                     step         = FINISHED;
                     next_step();

                     if ( total_edits >= total_speeds )
                        review_edits();
                  }
               }

               else
               {
                  cb_rpms->setCurrentIndex( row );
                  step         = MENISCUS;
                  QString trsp =
                     cb_triple->currentText() + " : " + trip_rpms[ row ];
                  le_edtrsp->setText( trsp );

                  set_meniscus();
               }
	    }

            if ( total_edits >= total_speeds )
            {
	      qDebug() << "In RANGES: total_edits, total_speeds: " << total_edits << ", " << total_speeds;
               all_edits = true;
               pb_write   ->setEnabled( true );

	       if ( us_edit_auto_mode_manual )
		 pb_pass->setEnabled( true );
	       
               ck_writemwl->setEnabled( true );
               changes_made = all_edits;
            }
         }

         break;

      case PLATEAU:

#if 0
         if ( p.x() > range_right )
         {
            le_plateau->setText( tr( "**beyond data end**" ) );
            break;
         }

         plateau = radius_indexed( p.x() );
#endif
         plateau      = range_right - _PLATEAU_OFFSET_;

         // Display the data (localize str)
         le_plateau ->setText( QString::number( plateau,     'f', 8 ) );

         plot_range();
         pb_plateau ->setIcon( check );
         ct_to->setValue( 0.0 );  // Uncolor all scans
         pb_report  ->setEnabled( true );
         pb_write   ->setEnabled( true );

	 if ( us_edit_auto_mode_manual )
	   pb_pass->setEnabled( true );
	 
         ck_writemwl->setEnabled( isMwl );
         changes_made = true;
         next_step();
         break;

      case BASELINE:
         {
            // Use the last scan
            US_DataIO::Scan* scan = &data.scanData.last();
            
            int start = data.xindex( range_left );
            int end   = data.xindex( range_right );
            int pt    = data.xindex( p.x() );

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
               sum += scan->rvalues[ j ];

            double bl = sum / 11.0;
            baseline  = p.x();
            plateau   = ( plateau > 0.0 ) ? plateau 
                                          : ( range_right - _PLATEAU_OFFSET_ );

            QString wkstr;
            le_baseline->setText( wkstr.sprintf( "%.3f (%.3e)", baseline, bl ) );
DbgLv(1) << "BL: AA : baseline bl" << baseline << bl;
            le_plateau ->setText( QString::number( plateau,     'f', 8 ) );
            plot_range();
         }

         pb_report     ->setEnabled( true );
         pb_write      ->setEnabled( true );

	 if ( us_edit_auto_mode_manual )
	   pb_pass->setEnabled( true );
	 
         ck_writemwl   ->setEnabled( isMwl );
         changes_made = true;
         next_step();
         break;

      default:
         break;
   }
}

// Draw a vertical pick line
void US_Edit::draw_vline( double radius )
{
   double r[ 2 ];

   r[ 0 ] = radius;
   r[ 1 ] = radius;

#if QT_VERSION < 0x050000
   QwtScaleDiv* y_axis = data_plot->axisScaleDiv( QwtPlot::yLeft );
#else
   QwtScaleDiv* y_axis = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::yLeft );
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

// Set the step flag for the next step
void US_Edit::next_step( void )
{
   QPushButton* pb;
   
   if      ( meniscus == 0.0 ) 
   {
      step = MENISCUS;
      pb   = pb_meniscus;
   }

   else if ( airGap_right == 9.0  &&  dataType == "IP" )
   {
      step = AIRGAP;
      pb   = pb_airGap;
      set_airGap();
   }

   else if ( range_right == 9.0 )
   {
      step = RANGE;
//      pb   = pb_dataRange;
      pb   = pb_dataEnd;
   }

#if 0
   else if ( plateau == 0.0 ) 
   {
      step = PLATEAU;
      pb   = pb_plateau;
   }
#endif

   else
   {  // All values up to Plateau have been entered:  Finished
      step = FINISHED;
      pb   = NULL;
//      pb   = pb_dataEnd;
      double sum = 0.0;
      int    pt  = data.xindex( range_left );
      baseline   = data.xvalues[ pt + 5 ];
      plateau    = ( plateau > 0.0 ) ? plateau 
                                     : ( range_right - _PLATEAU_OFFSET_ );
      
      if ( !isMwl )
      {
         // Average the value for +/- 5 points
         for ( int jj = pt; jj < pt + 11; jj++ )
            sum += data.scanData.last().rvalues[ jj ];

         double bl = sum / 11.0;

         QString str;
         le_baseline->setText( str.sprintf( "%.3f (%.3e)", baseline, bl ) );
	 DbgLv(1) << "BL: BB : baseline bl" << baseline << bl;

	 baseline_od = bl;
 
	 qDebug() << "SETTING Baseline, Plateau : baseline bl" << baseline << bl;
         le_plateau ->setText( QString::number( plateau, 'f', 8 ) );
      }
      else
	{
	  if ( us_edit_auto_mode )
	    {
	      plateau      = range_right - _PLATEAU_OFFSET_;
	      le_plateau  ->setText( QString::number( plateau,     'f', 3 ) );

	      le_baseline ->setText( QString::number( baseline,    'f', 3 ) );

	      baseline_od = 0;
	    }
	}
      
   }

   set_pbColors( pb );
}

// Set up for a meniscus pick
void US_Edit::set_meniscus( void )
{
   le_meniscus ->setText( "" );
   le_airGap   ->setText( "" );
   le_dataStart->setText( "" );
   le_dataEnd  ->setText( "" );
   le_plateau  ->setText( "" );
   le_baseline ->setText( "" );
   
   meniscus      = 0.0;
   meniscus_left = 0.0;
   airGap_left   = 0.0;
   airGap_right  = 9.0;
   range_left    = 0.0;
   range_right   = 9.0;
   plateau       = 0.0;
   baseline      = 0.0;
   
   step          = MENISCUS;
   
   set_pbColors( pb_meniscus );
   pb_meniscus->setIcon( QIcon() );

   pb_report   ->setEnabled( false );
   pb_airGap   ->setEnabled( false );
   pb_airGap   ->setIcon( QIcon() );
   pb_write    ->setEnabled( all_edits );
   ck_writemwl ->setEnabled( all_edits && isMwl );

   changes_made = all_edits;
DbgLv(1) << "set_meniscus -- changes_made" << changes_made;
   spikes       = false;
   pb_spikes   ->setEnabled( false );
   pb_spikes   ->setIcon( QIcon() );

   // Clear any existing marker
   data_plot->detachItems( QwtPlotItem::Rtti_PlotMarker );
            
   // Reset data and plot
   undo();

   if ( ! expIsEquil )
      plot_all();

   else
      plot_scan();
}

// Set up for an Air Gap pick
void US_Edit::set_airGap( void )
{
   le_airGap   ->setText( "" );
//   le_dataRange->setText( "" );
   le_dataStart->setText( "" );
   le_dataEnd  ->setText( "" );
   le_plateau  ->setText( "" );
   le_baseline ->setText( "" );
   
   airGap_left   = 0.0;
   airGap_right  = 9.0;
   range_left    = 0.0;
   range_right   = 9.0;
   plateau       = 0.0;
   baseline      = 0.0;
   
   step        = AIRGAP;
   set_pbColors( pb_airGap );
   pb_airGap   ->setIcon( QIcon() );

   pb_report   ->setEnabled( all_edits );
   pb_write    ->setEnabled( all_edits );
   ck_writemwl ->setEnabled( all_edits && isMwl );
   changes_made = all_edits;

   spikes = false;
   pb_spikes   ->setEnabled( false );
   pb_spikes   ->setIcon( QIcon() );

   undo();
   plot_all();
}

// Set up for a data range pick
void US_Edit::set_dataRange( void )
{
   le_dataStart->setText( "" );
   le_dataEnd  ->setText( "" );
   le_plateau  ->setText( "" );
   le_baseline ->setText( "" );
   
   range_left    = 0.0;
   range_right   = 9.0;
   plateau       = 0.0;
   baseline      = 0.0;
   
   step        = RANGE;

   pb_report   ->setEnabled( all_edits );
   pb_dataEnd  ->setIcon( QIcon() );
   pb_dataEnd  ->setEnabled( true );
   pb_write    ->setEnabled( all_edits );
   ck_writemwl ->setEnabled( all_edits && isMwl );
   changes_made = all_edits;

   spikes = false;
   pb_spikes   ->setEnabled( false );
   pb_spikes   ->setIcon( QIcon() );

   undo();

   if ( ! expIsEquil )
      plot_all();

   else
      plot_scan();
}

// Set up for a Plateau pick
void US_Edit::set_plateau( void )
{
   le_plateau  ->setText( "" );
   le_baseline ->setText( "" );
   
   plateau       = 0.0;
   baseline      = 0.0;
   
   step = PLATEAU;
   set_pbColors( pb_plateau );

   pb_report   ->setEnabled( false );
   pb_write    ->setEnabled( all_edits );
   ck_writemwl ->setEnabled( all_edits && isMwl );
   changes_made = all_edits;

   plot_range();
   undo();
}

// Set up for a Fringe Tolerance pick
void US_Edit::set_fringe_tolerance( double ftvalue )
{
DbgLv(1) << "   gap_fringe" << gap_fringe << "SFT: dataType" << dataType << "ftvalue" << ftvalue;
   // This is only valid for interference data
   if ( dataType != "IP" ) return;

   gap_fringe         = ftvalue;
DbgLv(1) << " 1)gap_fringe" << gap_fringe;

   // If we haven't yet set the range, just ignore the change
   if ( step == MENISCUS  ||  step == AIRGAP  ||  step == RANGE ) return;

   // Reset the data
   int index = index_data();
   data = *outData[ index ];

   US_DataIO::EditValues  edits;
   edits.airGapLeft  = airGap_left;
   edits.airGapRight = airGap_right;

   QList< int > excludes;
            
   for ( int i = 0; i < data.scanData.size(); i++ )
      if ( ! includes.contains( i ) ) edits.excludes << i;
         
   US_DataIO::adjust_interference( data, edits );

   edits.rangeLeft    = range_left;
   edits.rangeRight   = range_right;
   edits.gapTolerance = ct_gaps->value();
DbgLv(1) << " 3)gap_fringe" << gap_fringe << "index" << index;

   US_DataIO::calc_integral( data, edits );
   replot();
}

// Plot all curves
void US_Edit::plot_all( void )
{
   if ( isMwl )
   {
      plot_mwl();
      return;
   }
   QList< QColor > mcolors;
   int nmcols  = plot->map_colors( mcolors );
DbgLv(1) << " PlAll:  nmcols" << nmcols;
   QPen pen_plot( US_GuiSettings::plotCurve() );
   if ( nmcols == 1 )
      pen_plot    = QPen( mcolors[ 0 ] );

   if ( plot->btnZoom->isChecked() )
      plot->btnZoom->setChecked( false );

   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve ); 
   v_line = NULL;

   int size = data.pointCount();

   QVector< double > rvec( size );
   QVector< double > vvec( size );
   double* r   = rvec.data();
   double* v   = vvec.data();

   double maxR = -1.0e99;
   double minR =  1.0e99;
   double maxV = -1.0e99;
   double minV =  1.0e99;

   for ( int i = 0; i < data.scanData.size(); i++ )
   {
      if ( ! includes.contains( i ) ) continue;
      
      US_DataIO::Scan*  s = &data.scanData[ i ];

      for ( int j = 0; j < size; j++ )
      {
         r[ j ]      = data.xvalues[ j ];
         v[ j ]      = s  ->rvalues[ j ] * invert;

         maxR        = qMax( maxR, r[ j ] );
         minR        = qMin( minR, r[ j ] );
         maxV        = qMax( maxV, v[ j ] );
         minV        = qMin( minV, v[ j ] );
      }

      QString title = tr( "Raw Data at " )
         + QString::number( s->seconds ) + tr( " seconds" )
         + " #" + QString::number( i );

      QwtPlotCurve* c = us_curve( data_plot, title );
      c->setPaintAttribute( QwtPlotCurve::ClipPolygons, true );
      if ( nmcols > 1 )
      {
         pen_plot        = QPen( mcolors[ i % nmcols ] );
DbgLv(2) << " PlAll:      i" << i << "pen_plot" << pen_plot;
      }
      c->setPen    ( pen_plot );

      c->setSamples( r, v, size );
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

// Plot curves within the picked range
void US_Edit::plot_range( void )
{
   if ( plot->btnZoom->isChecked() )
      plot->btnZoom->setChecked( false );

   QList< QColor > mcolors;
   int nmcols  = plot->map_colors( mcolors );
DbgLv(1) << " PlRng:  nmcols" << nmcols;
   QPen pen_plot( US_GuiSettings::plotCurve() );
   if ( nmcols == 1 )
      pen_plot    = QPen( mcolors[ 0 ] );
   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   v_line = NULL;

   int rsize   = data.pointCount();
   QVector< double > rvec( rsize );
   QVector< double > vvec( rsize );
   double* r   = rvec.data();
   double* v   = vvec.data();
   double maxR = -1.0e99;
   double minR =  1.0e99;
   double maxV = -1.0e99;
   double minV =  1.0e99;
   int indext  = cb_triple->currentIndex();

   if ( isMwl )
   {
     //if ( !us_edit_auto_mode )
     //{
     int ccx     = indext;
     int wvx     = cb_lplot->currentIndex();
     indext      = ccx * nwaveln + wvx;
DbgLv(1) << "plot_range(): ccx wvx indext" << ccx << wvx << indext;
	 //}
   }

   // For each scan
   for ( int i = 0; i < data.scanData.size(); i++ )
   {
      if ( ! includes.contains( i ) ) continue;
      
      US_DataIO::Scan*  s = &data.scanData[ i ];
      
      int indexLeft  = data.xindex( range_left );
      int indexRight = data.xindex( range_right );
      double menp    = 0.0;

      if ( expIsEquil )
      {
         double rngl  = range_left;
         double rngr  = range_right;
         int tScan    = i + 1;
         int jsd      = sd_offs[ indext ];
         int ksd      = jsd + sd_knts[ indext ];

         for ( int jj = jsd; jj < ksd; jj++ )
         {
            int sScan  = sData[ jj ].first_scan;
            int eScan  = sData[ jj ].scan_count + sScan - 1;

            if ( tScan < sScan  ||  tScan > eScan )
               continue;

            rngl       = sData[ jj ].dataLeft;
            rngr       = sData[ jj ].dataRight;
            menp       = sData[ jj ].meniscus;
            break;
         }

         indexLeft  = data.xindex( rngl );
         indexRight = data.xindex( rngr );

         int inxm   = data.xindex( menp );

         if ( inxm < 1 )
            return;

         r[ 0 ]     = data.xvalues[ inxm          ];
         v[ 0 ]     = s  ->rvalues[ indexLeft     ];
         r[ 2 ]     = data.xvalues[ inxm + 2      ];
         v[ 2 ]     = s  ->rvalues[ indexLeft + 4 ];
         r[ 1 ]     = r[ 0 ];
         v[ 1 ]     = v[ 2 ];
         r[ 3 ]     = r[ 2 ];
         v[ 3 ]     = v[ 0 ];
         r[ 4 ]     = r[ 0 ];
         v[ 4 ]     = v[ 0 ];

         QwtPlotCurve* c = us_curve( data_plot, tr( "Meniscus at" ) + 
                  QString::number( tScan ) );
         c->setBrush( QBrush( Qt::cyan ) );
         c->setPen(   QPen(   Qt::cyan ) );
         c->setSamples( r, v, 5 );
         minR       = qMin( minR, r[ 0 ] );
         minV       = qMin( minV, v[ 0 ] );
      }
      
      int     count  = 0;
      
      for ( int j = indexLeft; j <= indexRight; j++ )
      {
         r[ count ] = data.xvalues[ j ];
         v[ count ] = s  ->rvalues[ j ] * invert;

         maxR       = qMax( maxR, r[ count ] );
         minR       = qMin( minR, r[ count ] );
         maxV       = qMax( maxV, v[ count ] );
         minV       = qMin( minV, v[ count ] );

         count++;
      }

      QString title = tr( "Raw Data at " )
         + QString::number( s->seconds ) + tr( " seconds" )
         + " #" + QString::number( i );

      QwtPlotCurve* c = us_curve( data_plot, title );
      if ( nmcols > 1 )
      {
         pen_plot        = QPen( mcolors[ i % nmcols ] );
DbgLv(2) << " PlRng:      i" << i << "pen_plot" << pen_plot;
      }
      c->setPen    ( pen_plot );
      c->setSamples( r, v, count );
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

// Plot the last picked curve
void US_Edit::plot_last( void )
{
   if ( plot->btnZoom->isChecked() )
      plot->btnZoom->setChecked( false );

   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   v_line = NULL;
   //grid = us_grid( data_plot ); 

   double maxR = -1.0e99;
   double minR =  1.0e99;
   double maxV = -1.0e99;
   double minV =  1.0e99;

   // Plot only the last scan
   US_DataIO::Scan*  s = &data.scanData[ includes.last() ];;
   
   int indexLeft  = data.xindex( range_left );
   int indexRight = data.xindex( range_right );
   
   int     count  = 0;
   uint    size   = s->rvalues.size();
   QVector< double > rvec( size );
   QVector< double > vvec( size );
   double* r      = rvec.data();
   double* v      = vvec.data();
   
   for ( int j = indexLeft; j <= indexRight; j++ )
   {
      r[ count ] = data.xvalues[ j ];
      v[ count ] = s  ->rvalues[ j ] * invert;

      maxR       = qMax( maxR, r[ count ] );
      minR       = qMin( minR, r[ count ] );
      maxV       = qMax( maxV, v[ count ] );
      minV       = qMin( minV, v[ count ] );

      count++;
   }

   QString title = tr( "Raw Data at " )
      + QString::number( s->seconds ) + tr( " seconds" )
      + " #" + QString::number( includes.last() );

   QwtPlotCurve* c = us_curve( data_plot, title );
   c->setSamples( r, v, count );

   // Reset the scan curves within the new limits
   double padR = ( maxR - minR ) / 30.0;
   double padV = ( maxV - minV ) / 30.0;

   data_plot->setAxisScale( QwtPlot::yLeft  , minV - padV, maxV + padV );
   data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );

   // Reset colors
   focus( (int)ct_from->value(), (int)ct_to->value() );
   data_plot->replot();
}

// Plot a single scan curve
void US_Edit::plot_scan( void )
{
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
   QString srpm = cb_rpms->currentText();

   // Plot only the currently selected scan(s)
   //
   for ( int ii = 0; ii < ssize; ii++ )
   {
      US_DataIO::Scan*  s = &data.scanData[ ii ];

      QString arpm        = QString::number( s->rpm );

      if ( arpm != srpm )
         continue;

      count = 0;

      for ( int jj = 0; jj < rsize; jj++ )
      {
         r[ count ] = data.xvalues[ jj ];
         v[ count ] = s  ->rvalues[ jj ] * invert;

         maxR       = qMax( maxR, r[ count ] );
         minR       = qMin( minR, r[ count ] );
         maxV       = qMax( maxV, v[ count ] );
         minV       = qMin( minV, v[ count ] );

         count++;
      }

      QString title = tr( "Raw Data at " )
         + QString::number( s->seconds ) + tr( " seconds" )
         + " #" + QString::number( ii );

      QwtPlotCurve* c = us_curve( data_plot, title );
      c->setSamples( r, v, count );

      // Reset the scan curves within the new limits
      double padR = ( maxR - minR ) / 30.0;
      double padV = ( maxV - minV ) / 30.0;

      data_plot->setAxisScale( QwtPlot::yLeft  , minV - padV, maxV + padV );
      data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );
   }

   data_plot->replot();
}

// Plot MWL curves
void US_Edit::plot_mwl( void )
{
   if ( ! isMwl )  return;

   QString     rectype = tr( "Wavelength" );
   double      recvalu = 250;
   int         index   = index_data();
   int         ccx     = triple_index;
   int         recndx  = cb_lplot ->currentIndex();
   QString     celchn  = celchns.at( ccx );
   QString     scell   = celchn.section( "/", 0, 0 ).simplified();
   QString     schan   = celchn.section( "/", 1, 1 ).simplified();
   QString     svalu;


   //ALEXEY: when all data loaded && processed for meniscus, plot reference wvl (selected for edit)
   if ( us_edit_auto_mode && all_loaded && !triple_plot_first_time[ triple_index ] )
     {
       data_index = mwl_data.data_index( iwavl_edit_ref[ triple_index ], triple_index );
       index = data_index;

       recndx = iwavl_edit_ref_index[ triple_index ];

       qDebug() << "In PLOT_MWL when all loaded: triple_index, iwavl_edit_ref[ triple_index ], iwavl_edit_ref_index[ triple_index , data_index -- "
		<< triple_index  << ", "
		<< iwavl_edit_ref[ triple_index ] << ", "
		<< iwavl_edit_ref_index[ triple_index ] << ", "
		<< data_index;

       triple_plot_first_time[ triple_index ] = 1;
    
     }
   /////////////////////////////////////////////////////////////////////////////////////////////////

DbgLv(1) << "PlMwl:  index celchn" << index << celchn;

   if ( xaxis_radius )
   {
DbgLv(1) << "PlMwl:   x-r index cc nw rx" << index << ccx << nwavelo << recndx;
DbgLv(1) << "PlMwl:    outData size" << outData.size();
DbgLv(1) << "PlMwl:    expc_wvlns size" << expc_wvlns.size();
      data                = *outData[ data_index ];

      qDebug() << "IN_edit_1";
      
      recvalu             = expi_wvlns.at( recndx );

      qDebug() << "IN_edit_2";
      svalu               = expc_wvlns.at( recndx );

      qDebug() << "IN_edit_3";
QString dcell=QString::number(data.cell);

 qDebug() << "IN_edit_4";
 
QString dchan=QString(QChar(data.channel));

 qDebug() << "IN_edit_5";
 
QString dwavl=QString::number(data.scanData[0].wavelength);
DbgLv(1) << "PlMwl:     c triple" << scell << schan << svalu
 << "d triple" << dcell << dchan << dwavl;
   }

   else
   {
      // index               = ccx * nrpoint + recndx;
// DbgLv(1) << "PlMwl:   x-w index cc nr rx" << index << ccx << nrpoint << recndx;
      data                = *outData[ 0 ];
      rectype             = tr( "Radius" );
      recvalu             = expd_radii.at( recndx );
      svalu               = expc_radii.at( recndx );
   }
DbgLv(1) << "PlMwl: ccx index rtype rval" << ccx << index << rectype << recvalu;

   // Read the data description
   QString     desc    = data.description;

   dataType            = QString( QChar( data.type[ 0 ] ) ) 
                       + QString( QChar( data.type[ 1 ] ) );

   le_info->setText( runID + "  (" + desc + ")" );
   QString plot_type;
   if ( dataType == "RA" )
   {
      plot_type = "Radial Absorbance Data\n";
      data_plot->setAxisTitle( QwtPlot::yLeft, tr( "Absorbance (OD)" ) );
   }
   else if ( dataType == "RI" )
   {
      plot_type = "Pseudo Absorbance Data\n";
      data_plot->setAxisTitle( QwtPlot::yLeft, tr( "Absorbance (OD)" ) );
   }
   else if ( dataType == "IP" )
   {

      plot_type = "Radial Interference Data\n";
      data_plot->setAxisTitle( QwtPlot::yLeft, tr( "Fringes " ) );

      // Enable Air Gap
      pb_airGap->setHidden( false );
      le_airGap->setHidden( false );
   }
   else if ( dataType == "FI" )
   {
      plot_type = "Fluorescence Intensity Data\n";
      data_plot->setAxisTitle( QwtPlot::yLeft, tr( "Fluorescence Intensity " ) );
   }
   else
      plot_type = "File type not recognized";
   // Plot Title
   QString     title   = tr( "%6"
                             "Run ID: %1\n"
                             "Cell: %2  Channel: %3  %4: %5" )
                         .arg( runID ).arg( scell ).arg( schan )
                         .arg( rectype ).arg( svalu ).arg( plot_type );
DbgLv(1) << "PlMwl:  title" << title;

   data_plot->setTitle    ( title );


   data_plot->detachItems ( QwtPlotItem::Rtti_PlotCurve ); 
   v_line = NULL;

   int     nscan  = data.scanData.size();
   int     npoint = data.pointCount();
   int     ptxs   = 0;

   if ( step != MENISCUS  &&  xaxis_radius )
   {
      ptxs           = data.xindex( range_left );
      npoint         = data.xindex( range_right ) - ptxs + 1;
   }
DbgLv(1) << "PlMwl:   xa_rad" << xaxis_radius << "nsc npt" << nscan << npoint;

   QVector< double > rvec( npoint );
   QVector< double > vvec( npoint );
   double* rr     = rvec.data();
   double* vv     = vvec.data();

   double  maxR   = -1.0e99;
   double  minR   =  1.0e99;
   double  maxV   = -1.0e99;
   double  minV   =  1.0e99;
   double  maxOD  = odlimit * 2.0;
   double  valueV;
   int     kodlim = 0;

   if ( xaxis_radius )
   {  // Build normal AUC data plot
      data_plot->setAxisTitle( QwtPlot::xBottom, tr( "Radius (cm)" ) );
DbgLv(1) << "PlMwl:    START xa_RAD";

      for ( int ii = 0; ii < nscan; ii++ )
      {
         if ( ! includes.contains( ii ) )
         {
DbgLv(1) << "PlMwl:     ii" << ii << "NOT INCLUDED";
            continue;
         }

         US_DataIO::Scan*  scn = &data.scanData[ ii ];
         int     kk     = ptxs;

         for ( int jj = 0; jj < npoint; jj++ )
         {
            rr[ jj ] = data.xvalues[ jj ];
            valueV   = qMin( maxOD, scn->rvalues[ kk++ ] * invert );
            vv[ jj ] = valueV;

            maxR     = qMax( maxR, rr[ jj ] );
            minR     = qMin( minR, rr[ jj ] );
            maxV     = qMax( maxV, valueV );
            minV     = qMin( minV, valueV );

            if ( valueV > odlimit )
               kodlim++;
         }

         QString ctitle = tr( "Raw Data at " )
            + QString::number( scn->seconds ) + tr( " seconds" )
            + " #" + QString::number( ii );

         QwtPlotCurve* cc = us_curve( data_plot, ctitle );
         cc->setPaintAttribute( QwtPlotCurve::ClipPolygons, true );
         cc->setSamples( rr, vv, npoint );
      }
      pick     ->disconnect();
      connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                     SLOT  ( mouse   ( const QwtDoublePoint& ) ) );
DbgLv(1) << "PlMwl:      END xa_RAD  kodlim odlimit" << kodlim << odlimit;
   }

   else
   {  // Build plot of radius record with wavelength points
DbgLv(1) << "PlMwl:    START xa_WAV";
      data_plot->setAxisTitle( QwtPlot::xBottom, tr( "Wavelength (nm)" ) );
      int ccx = cb_triple->currentIndex();
      rvec.clear();

      foreach (int rwvl, rawi_wvlns) {
          rvec << rwvl;
          minR = qMin(minR, static_cast<double>(rwvl));
          maxR = qMax(maxR, static_cast<double>(rwvl));
      }
      rr = rvec.data();
      int rpidx = cb_lplot->currentIndex();
      int npoints = cb_lplot->count();
      int nscans = rdata.at(ccx).size() / nwaveln / npoints;

      QVector< double > vvcec_all = rdata.at(ccx);
      int idx_ii = rpidx * nwaveln * nscans;
      for ( int ss = 0; ss < nscans; ss++) {
         int idx_ss = ss * nwaveln;
         vvec.clear();
         for (int jj = 0; jj < nwaveln; jj++) {
            int idx = idx_ii + idx_ss + jj;
            double val = vvcec_all.at(idx);
            maxV     = qMax( maxV, val );
            minV     = qMin( minV, val );
            vvec << val;
         }
         vv = vvec.data();
         QwtPlotCurve* cc = us_curve( data_plot, tr("Scan %1").arg(ss) );
         cc->setPaintAttribute( QwtPlotCurve::ClipPolygons, true );
         cc->setSamples( rr, vv, nwaveln );
      }
   }

   // Reset the scan curves within the new limits
   double padR = ( maxR - minR ) / 30.0;
   double padV = ( maxV - minV ) / 30.0;
   padV        = qMax( padV, 0.005 );

   data_plot->setAxisScale( QwtPlot::yLeft  , minV - padV, maxV + padV );
   data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );

DbgLv(1) << "PlMwl: call replot()";
   data_plot->replot();
DbgLv(1) << "PlMwl:  retn fr replot()";

   // Set the Scan spin boxes
   ct_from->setMinimum( 0.0 );
   ct_from->setMaximum( data.scanData.size() );

   ct_to  ->setMinimum( 0.0 );
   ct_to  ->setMaximum( data.scanData.size() );

   pick   ->disconnect();
   connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                  SLOT  ( mouse   ( const QwtDoublePoint& ) ) );
}

// Set focus FROM scan value
void US_Edit::focus_from( double scan )
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

// Set focus TO scan value
void US_Edit::focus_to( double scan )
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

// Set focus From/To
void US_Edit::focus( int from, int to )
{
   if ( from == 0 )
      pb_edit1  ->setEnabled( false );
   else
      pb_edit1  ->setEnabled( true );

   if ( to == 0 )
      pb_excludeRange->setEnabled( false );
   else
      pb_excludeRange->setEnabled( true );

   QList< int > focus;
   int ifrom = qMax( from - 1, 0 );
   int ito   = qMin( to, includes.size() );

   for ( int ii = ifrom; ii < ito; ii++ )
      focus << includes.at( ii );

   set_colors( focus );
}

// Set curve colors
void US_Edit::set_colors( const QList< int >& focus )
{
   QList< QColor > mcolors;
   int nmcols  = plot->map_colors( mcolors );
DbgLv(1) << "ED:scol: nmcols" << nmcols << mcolors.size();
   QPen pen_plot( US_GuiSettings::plotCurve() );
   if ( nmcols == 1 )
      pen_plot    = QPen( mcolors[ 0 ] );

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
   QColor foc = Qt::red;

   // Mark these scans in red
   for ( int i = 0; i < curves.size(); i++ )
   {
      int scnnbr = curves[ i ]->title().text().section( "#", 1, 1 ).toInt();

      if ( focus.contains( scnnbr ) )
      {
         p.setColor( foc );
         b.setColor( foc );

         curves[ i ]->setPen  ( p );
         curves[ i ]->setBrush( b );
      }
      else if ( scnnbr > 0 )
      {
         scnnbr--;
//DbgLv(1) << "ED:scol:   i" << i << "scnx" << scnnbr;
         if ( nmcols > 1 )
            pen_plot        = QPen( mcolors[ scnnbr % nmcols ] );
         curves[ i ]->setPen( pen_plot );
      }
   }

   data_plot->replot();
}

// Initialize includes
void US_Edit::init_includes( void )
{
   includes.clear();
   for ( int i = 0; i < data.scanData.size(); i++ ) includes << i;
}

// Reset excludes
void US_Edit::reset_excludes( void )
{
   ct_from->disconnect();
   ct_from->setValue  ( 0 );
   ct_from->setMaximum( includes.size() );
   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   ct_to->disconnect();
   ct_to->setValue  ( 0 );
   ct_to->setMaximum( includes.size() );
   connect( ct_to, SIGNAL( valueChanged ( double ) ),
                   SLOT  ( focus_to   ( double ) ) );

   pb_excludeRange->setEnabled( false );
   pb_edit1       ->setEnabled( false );

   replot();
}

// Set excludes as indicated in counters
void US_Edit::exclude_range( void )
{
   int scanStart = (int)ct_from->value();
   int scanEnd   = (int)ct_to  ->value();

   for ( int i = scanEnd; i >= scanStart; i-- )
      includes.removeAt( i - 1 );

   replot();
   reset_excludes();
}

// Show exclusion profile
void US_Edit::exclusion( void )
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

// Update based on exclusion profile
void US_Edit::update_excludes( QList< int > scanProfile )
{
DbgLv(1) << "UPD_EXCL: size excl" << scanProfile.size();
   set_colors( scanProfile );
}

// Cancel all excludes
void US_Edit::cancel_excludes( void )
{
   QList< int > focus;
   set_colors( focus );  // Focus on no curves
}

// Process excludes to rebuild include list
void US_Edit::finish_excludes( QList< int > excludes )
{
   for ( int i = 0; i < excludes.size(); i++ )
      includes.removeAll( excludes[ i ] );

DbgLv(1) << "FIN_EXCL: sizes excl incl" << excludes.size() << includes.size();
   replot();
   reset_excludes();
}

// Edit a scan
void US_Edit::edit_scan( void )
{
   int index1 = (int)ct_from->value();
   int scan  = includes[ index1 - 1 ];

   US_EditScan* dialog = new US_EditScan( data.scanData[ scan ], data.xvalues, 
         invert, range_left, range_right );
   connect( dialog, SIGNAL( scan_updated( QList< QPointF > ) ),
                    SLOT  ( update_scan ( QList< QPointF > ) ) );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

// Update scan points
void US_Edit::update_scan( QList< QPointF > changes )
{
   // Handle excluded scans
   int              index1        = (int)ct_from->value();
   int              current_scan  = includes[ index1 - 1 ];
   US_DataIO::Scan* s             = &data.scanData[ current_scan ];

   for ( int i = 0; i < changes.size(); i++ )
   {
      int    point = (int)changes[ i ].x();
      double value =      changes[ i ].y();

      s->rvalues[ point ] = value;
   }

   // Save changes for writing output
   Edits e;
   e.scan    = current_scan;
   e.changes = changes;
   changed_points << e;

   // Set data for the curve
   int     points = data.pointCount();
   QVector< double > rvec( points );
   QVector< double > vvec( points );
   double* r      = rvec.data();
   double* v      = vvec.data();

   int left;
   int right;
   int count = 0;

   if ( range_left > 0 )
   {
      left  = data.xindex( range_left  );
      right = data.xindex( range_right );
   }
   else
   {
      left  = 0;
      right = points - 1;
   }

   for ( int i = left; i <= right; i++ )
   {
      r[ count ] = data.xvalues[ i ];
      v[ count ] = s  ->rvalues[ i ];
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
      c->setSamples( r, v, count );
      data_plot->replot();
   }
   else
      qDebug() << "Can't find curve!";

}

// Handle include profile
void US_Edit::include( void )
{
   init_includes();
   reset_excludes();
}

// Reset pushbutton and plot with invert flag change
void US_Edit::invert_values( void )
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

// Remove spikes
void US_Edit::remove_spikes( void )
{
   double smoothed_value;

   // For each scan
   for ( int i = 0; i < data.scanData.size(); i++ ) 
   {
      US_DataIO::Scan* s = &data.scanData [ i ];

      int start  = data.xindex( range_left  );
      int end    = data.xindex( range_right );

      for ( int j = start; j < end; j++ )
      {
         if ( US_DataIO::spike_check( *s, data.xvalues, j, start, end, 
                                      &smoothed_value ) )
         {
            s->rvalues[ j ]     = smoothed_value;

            // If previous consecututive points are interpolated, then 
            // redo them
            int           index = j - 1;
            unsigned char c     = s->interpolated[ index / 8 ];

            while ( c & ( 1 << ( 7 - index % 8 ) ) )
            {
               if ( US_DataIO::spike_check( *s, data.xvalues,
                                            index, start, end, &smoothed_value ) )
                  s->rvalues[ index ] = smoothed_value;

               index--;
               c = s->interpolated[ index / 8 ];
            }
         }
      }
   }

   pb_spikes->setIcon   ( check );
   pb_spikes->setEnabled( false );
   pb_write ->setEnabled( true );
   replot();

   
}


// Remove spikes
void US_Edit::remove_spikes_auto( void )
{

  //ALEXEY: Apply data modifications like adjust_interference && calc_integral for IP data
  if ( dataType == "IP" )
    {
      airGap_left   = editProfile[ cb_triple->currentText() ][7].toDouble();
      airGap_right  = editProfile[ cb_triple->currentText() ][8].toDouble();
      
      US_DataIO::EditValues  edits;
      edits.airGapLeft  = airGap_left;
      edits.airGapRight = airGap_right;
      
      QList< int > excludes;
      
      for ( int i = 0; i < data.scanData.size(); i++ )
	if ( ! includes.contains( i ) ) edits.excludes << i;
      
      US_DataIO::adjust_interference( data, edits );
      
      edits.rangeLeft    = range_left;
      edits.rangeRight   = range_right;
      edits.gapTolerance = ct_gaps->value();
      
      US_DataIO::calc_integral( data, edits );
    }
  /**************************************************************************************/
  
  double smoothed_value;

   // For each scan
   for ( int i = 0; i < data.scanData.size(); i++ ) 
   {
      US_DataIO::Scan* s = &data.scanData [ i ];

      int start  = data.xindex( range_left  );
      int end    = data.xindex( range_right );

      for ( int j = start; j < end; j++ )
      {
         if ( US_DataIO::spike_check( *s, data.xvalues, j, start, end, 
                                      &smoothed_value ) )
         {
            s->rvalues[ j ]     = smoothed_value;

            // If previous consecututive points are interpolated, then 
            // redo them
            int           index = j - 1;
            unsigned char c     = s->interpolated[ index / 8 ];

            while ( c & ( 1 << ( 7 - index % 8 ) ) )
            {
               if ( US_DataIO::spike_check( *s, data.xvalues,
                                            index, start, end, &smoothed_value ) )
                  s->rvalues[ index ] = smoothed_value;

               index--;
               c = s->interpolated[ index / 8 ];
            }
         }
      }
   }

   pb_spikes->setIcon   ( check );
   
   // // ALEXEY: resize up to (1) meniscus; (2) left_range; (3) right_range; (4) plateau; (5) baseline; (6) baseline_od
   // for (int i=0; i < editProfile[ cb_triple->currentText() ].count(); i++)
   //   {
   //     if (i > 5)
   // 	 editProfile[ cb_triple->currentText() ].removeAt(i);
   //   }
   
   // editProfile[ cb_triple->currentText() ] << QString("spike_true");

   editProfile[ cb_triple->currentText() ][6] = QString("spike_true");

   qDebug() << cb_triple->currentText()  << ", " << editProfile[ cb_triple->currentText() ];
      
   pb_spikes->setEnabled( false );
   pb_write ->setEnabled( true );

   //replot();  //ALEXEY - do we need to replot here ?

   if ( us_edit_auto_mode && all_loaded )
     {
       meniscus      = editProfile[ cb_triple->currentText() ][0].toDouble();
       range_left    = editProfile[ cb_triple->currentText() ][1].toDouble();
       range_right   = editProfile[ cb_triple->currentText() ][2].toDouble();
       plateau       = editProfile[ cb_triple->currentText() ][3].toDouble();
       baseline      = editProfile[ cb_triple->currentText() ][4].toDouble();
       baseline_od   = editProfile[ cb_triple->currentText() ][5].toDouble();

       le_meniscus ->setText( QString::number( meniscus,   'f', 3 ) );
       le_dataStart->setText( QString::number( range_left, 'f', 3 ) );
       le_dataEnd  ->setText( QString::number( range_right, 'f', 3 ) );
       le_plateau  ->setText( QString::number( plateau,     'f', 3 ) );


       if ( dataType == "IP" )
	 {
	   airGap_left   = editProfile[ cb_triple->currentText() ][7].toDouble();
	   airGap_right  = editProfile[ cb_triple->currentText() ][8].toDouble();
	   
	   QString wkstr;
	   le_airGap->setText( wkstr.sprintf( "%.3f - %.3f", 
					      airGap_left, airGap_right ) );
	 }

       if ( isMwl ) 
	 le_baseline ->setText( QString::number( baseline,     'f', 3 ) );
       else
	 {
	   QString str;
	   le_baseline->setText( str.sprintf( "%.3f (%.3e)", baseline, baseline_od ) );   
	 }

       plot_range();
     }

   pb_undo -> setEnabled( true );

   qDebug() << "pb_spikes->icon() IS NULL ? " << pb_spikes->icon().isNull();
}



// Undo changes
void US_Edit::undo_auto( void )
{
   // Copy from outData to data
   if ( step < PLATEAU )
      data      = *outData[ index_data() ];

   // Redo some things depending on type
   if ( dataType == "IP" )
   {
     airGap_left   = editProfile[ cb_triple->currentText() ][7].toDouble();
     airGap_right  = editProfile[ cb_triple->currentText() ][8].toDouble();
     
      US_DataIO::EditValues edits;
      edits.airGapLeft  = airGap_left;
      edits.airGapRight = airGap_right;

      edits.rangeLeft    = range_left;
      edits.rangeRight   = range_right;
      edits.gapTolerance = ct_gaps->value();
      
      for ( int i = 0; i < data.scanData.size(); i++ )
         if ( ! includes.contains( i ) ) edits.excludes << i;

      US_DataIO::adjust_interference( data, edits );

      US_DataIO::calc_integral( data, edits );
   }

   replot();

   if (us_edit_auto_mode  && all_loaded )
     {
       meniscus      = editProfile[ cb_triple->currentText() ][0].toDouble();
       range_left    = editProfile[ cb_triple->currentText() ][1].toDouble();
       range_right   = editProfile[ cb_triple->currentText() ][2].toDouble();
       plateau       = editProfile[ cb_triple->currentText() ][3].toDouble();
       baseline      = editProfile[ cb_triple->currentText() ][4].toDouble();
       baseline_od   = editProfile[ cb_triple->currentText() ][5].toDouble();

       le_meniscus ->setText( QString::number( meniscus,   'f', 3 ) );
       le_dataStart->setText( QString::number( range_left, 'f', 3 ) );
       le_dataEnd  ->setText( QString::number( range_right, 'f', 3 ) );
       le_plateau  ->setText( QString::number( plateau,     'f', 3 ) );

       
       if ( dataType == "IP" )
	 {
	   airGap_left   = editProfile[ cb_triple->currentText() ][7].toDouble();
	   airGap_right  = editProfile[ cb_triple->currentText() ][8].toDouble();
	   
	   QString wkstr;
	   le_airGap->setText( wkstr.sprintf( "%.3f - %.3f", 
					      airGap_left, airGap_right ) );
	 }
       
       
       if ( isMwl ) 
	 le_baseline ->setText( QString::number( baseline,     'f', 3 ) );
       else
	 {
	   QString str;
	   le_baseline->setText( str.sprintf( "%.3f (%.3e)", baseline, baseline_od ) );   
	 }
       
       plot_range();
     }
   
   /*      
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

   */
   
   spikes      = false;
   noise_order = 0;

   if ( all_loaded )
     {
       // // ALEXEY: resize up to (1) meniscus; (2) left_range; (3) right_range; (4) plateau; (5) baseline; (6) baseline_od 
       // for (int i=0; i < editProfile[ cb_triple->currentText() ].count(); i++)
       // 	 {
       // 	   if (i > 5)
       // 	     editProfile[ cb_triple->currentText() ].removeAt(i);
       // 	 }
       
       // editProfile[ cb_triple->currentText() ] << QString("spike_false");

       editProfile[ cb_triple->currentText() ][6] = QString("spike_false");
       
       qDebug() << cb_triple->currentText()  << ", " << editProfile[ cb_triple->currentText() ];
     }

   // Remove icons
   pb_noise       ->setIcon( QIcon() );
   pb_residuals   ->setIcon( QIcon() );
   pb_spikes      ->setIcon( QIcon() );

   pb_undo->setEnabled( false );
   pb_spikes->setEnabled( true );
}


// Undo changes
void US_Edit::undo( void )
{
   // Copy from outData to data
   if ( step < PLATEAU )
      data      = *outData[ index_data() ];

   // Redo some things depending on type
   if ( dataType == "IP" )
   {
      US_DataIO::EditValues edits;
      edits.airGapLeft  = airGap_left;
      edits.airGapRight = airGap_right;

      edits.rangeLeft    = range_left;
      edits.rangeRight   = range_right;
      edits.gapTolerance = ct_gaps->value();
      
      for ( int i = 0; i < data.scanData.size(); i++ )
         if ( ! includes.contains( i ) ) edits.excludes << i;

      if ( step > AIRGAP )
            US_DataIO::adjust_interference( data, edits );

      if ( step >  RANGE )
         US_DataIO::calc_integral( data, edits );
   }

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

// Calculate and apply noise
void US_Edit::noise( void )
{
   residuals.clear();
   US_RiNoise* dialog = new US_RiNoise( data, includes,  
         range_left, range_right, dataType, noise_order, residuals );
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

// Subtract residuals
void US_Edit::subtract_residuals( void )
{
   for ( int i = 0; i < data.scanCount(); i++ )
   {
      for ( int j = 0; j <  data.pointCount(); j++ )
         data.scanData[ i ].rvalues[ j ] -= residuals[ i ];
   }

   pb_residuals->setEnabled( false );
   pb_residuals->setIcon   ( check );
   replot();
}

// Select a new triple
void US_Edit::new_triple_auto( int index )
{
  triple_index    = index;
  
  qDebug() << "NEW_TRIPLE_AUTO: triple_index: " << triple_index;

  QString triple_name = cb_triple->itemText( triple_index );
  //qDebug() << "#triples, #wavelns_i -- " << cb_triple->count() << wavelns_i.size();
  //qDebug() << "#wavelns in triple   -- " << triple_name << wavelns_i[ triple_index ].size();
  

  // Remove Spike: Icon/Enable
  if ( editProfile[ cb_triple->currentText() ].count() > 6 )
    {
      if ( editProfile[ cb_triple->currentText() ][6] == "spike_true")
	{
	  pb_spikes->setIcon( check );
	  pb_spikes->setEnabled( false );
	  pb_undo  ->setEnabled( true );
	  
	}
      else if (editProfile[ cb_triple->currentText() ][6] == "spike_false")
	{
	  pb_spikes->setIcon(QIcon());
	  pb_spikes->setEnabled( true );
	  pb_undo  ->setEnabled( false );
	}
    }
  else
    {
      pb_spikes->setIcon(QIcon());
      pb_spikes->setEnabled( true );
      pb_undo  ->setEnabled( false );
    }
  
      //Next/Proir triples
  if ( triple_index == 0 )
    pb_priorChan->setEnabled( false );
  else
    pb_priorChan->setEnabled( true );

  if ( (triple_index + 1) == cb_triple->count() )
    pb_nextChan->setEnabled( false );
  else
    pb_nextChan->setEnabled( true );
    
  
  
   double gap_val  = ct_gaps->value();
DbgLv(1) << "EDT:NewTr: tripindex" << triple_index << "chgs" << changes_made << "gap_val" << gap_val;

/*
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

*/
   ct_gaps->disconnect();

   // Set up data indexes
   rb_radius->setChecked( true );
   rb_waveln->setChecked( false );

   index_data();

   
   
DbgLv(1) << "EDT:NewTr: trip,data index" << triple_index << data_index;

   if ( isMwl )
   {  // MWL:  restore the wavelengths for the newly selected triple
      if ( lrng_bycell )
      {  // Restore raw lambdas for an individual cell
         connect_mwl_ctrls( false );
         lambdas_by_cell();
         cb_lstart->clear();
         cb_lend  ->clear();
         cb_lstart->addItems( rawc_wvlns );
         cb_lend  ->addItems( rawc_wvlns );
         cb_lstart->setCurrentIndex( 0 );
         cb_lend  ->setCurrentIndex( nwaveln - 1 );
         slambda      = rawi_wvlns[ 0 ];
         elambda      = rawi_wvlns[ nwaveln - 1 ];
         dlambda      = 1;
         le_ltrng ->setText( tr( "%1 raw: %2 %3 to %4" )
            .arg( nwaveln ).arg( chlamb ).arg( slambda ).arg( elambda ) );
         connect_mwl_ctrls( true );
      }

      QVector< int > wvs;
      mwl_data.lambdas( wvs, triple_index );
      std::sort( wvs.begin(), wvs.end() );
      lambda_new_list ( wvs );
      le_lxrng ->setText( tr( "%1 MWL exports: %2 %3 to %4,"
                              " raw index increment %5" )
         .arg( nwavelo ).arg( chlamb ).arg( slambda ).arg( elambda )
         .arg( dlambda ) );
DbgLv(1) << "EDT:NewTr:  nwavelo" << nwavelo;
   }

   // Reset for new triple
   reset_triple(); 

   // Need to reconnect after reset
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
                       SLOT  ( new_triple_auto    ( int ) ) );

   QString otdt   = dataType;

   //ALEXEY: if MVL plot triple for the reference wvl identified from the AProfile  
   if ( isMwl )
     data_index   = mwl_data.data_index( iwavl_edit_ref[ triple_index ], triple_index );
   else
     index_data_auto( triple_index );
   ////////////////////////////////////////////////////////////////////////////////
   
   edata          = outData[ data_index ];
   data           = *edata;

   qDebug() << "NEW_TRIPLE_AUTO: data_index: " << data_index;

   
   QString rawGUID_test1          = US_Util::uuid_unparse( (unsigned char*)data.rawGUID );
   qDebug() << "NEW TRIPLE: Current rawData: rawGUID: " <<  rawGUID_test1;
   
   QString swavl  = cb_lplot ->currentText();
   QString triple = cb_triple->currentText() + ( isMwl ? " / " + swavl : "" );
   int     idax   = triples.indexOf( triple );
   dataType = QString( QChar( data.type[ 0 ] ) ) 
            + QString( QChar( data.type[ 1 ] ) );
DbgLv(1) << "EDT:NewTr:   sw tri dx" << swavl << triple << idax << "dataType" << dataType
 << "gap_fringe gap_thresh" << gap_fringe << gap_thresh;
   if ( otdt != dataType )
   {  // Reset gap controls for new datatype
      if ( dataType == "IP" )
      {
         gap_thresh   = gap_val;
         ct_gaps->setValue     ( gap_fringe );
         ct_gaps->setRange     ( 0.0, 20.0 );
         ct_gaps->setSingleStep( 0.001 );
         ct_gaps->setNumButtons( 3 );
      }
      else
      {
         gap_fringe   = gap_val;
         ct_gaps->disconnect   ();
         ct_gaps->setRange     ( 10.0, 100.0 );
         ct_gaps->setSingleStep( 10.0 );
         ct_gaps->setValue     ( gap_thresh );
      }
   }

   // Enable pushbuttons
   pb_details  ->setEnabled( true );
   pb_report   ->setEnabled( all_edits );
   pb_include  ->setEnabled( true );
   pb_exclusion->setEnabled( true );
   pb_meniscus ->setEnabled( true );
   pb_airGap   ->setEnabled( true );
   pb_noise    ->setEnabled( true );
   pb_spikes   ->setEnabled( true );
   pb_invert   ->setEnabled( true );
   //pb_undo     ->setEnabled( true );


   //qDebug() << "In new triple_auto: all_edits, isMw: " << all_edits << ", " << isMwl; 

   all_edits = true;
   pb_write    ->setEnabled( all_edits );
   ck_writemwl ->setEnabled( all_edits && isMwl );

   all_edits    = false;
   changes_made = all_edits;

   init_includes();

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );
   connect( ct_to,   SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   connect( ct_gaps, SIGNAL( valueChanged        ( double ) ), 
                     SLOT  ( set_fringe_tolerance( double ) ) );

   if ( expIsEquil )
   {  // Equilibrium
      cb_rpms->clear();
      trip_rpms.clear();

      for ( int ii = 0; ii < data.scanData.size(); ii++ )
      {  // build unique-rpm list for triple
         QString arpm = QString::number( data.scanData[ ii ].rpm );

         if ( ! trip_rpms.contains( arpm ) )
         {
            trip_rpms << arpm;
         }
      }
      cb_rpms->addItems( trip_rpms );

      le_edtrsp->setText( cb_triple->currentText() + " : " + trip_rpms[ 0 ] );
   }

   else
   {  // non-Equilibrium:  possibly re-do/un-do edits
      QString trbase   = cb_triple->currentText();
      triple           = trbase;

      if ( isMwl )
      {
         plotndx          = cb_lplot->currentIndex();
         swavl            = cb_lplot->currentText();
         triple          += " / " + swavl;
         plotrec          = swavl.toInt();

      }

      QString fname    = editFnames[ idax ];
      bool    app_edit = ( fname != "none" );
DbgLv(1) << "EDT:NewTr:   app_edit" << app_edit << "fname" << fname;


      if ( app_edit )
      {  // If editing chosen, apply it
         US_DataIO::EditValues parameters;

         US_DataIO::readEdits( workingDir + fname, parameters );

         apply_edits( parameters );
      }

      else
      {  // If no editing, be sure to turn off edits
         //set_meniscus();                                       // ALEXEY: we do not need to reset
      }

DbgLv(1) << "EDT:NewTr:   men" << meniscus << "dx" << idax;
      plot_current( idax );
   }

   // Reset GAPS labels and values based on this triple's type
   if ( dataType == "IP" )
   {
      lb_gaps->setText( tr( "Fringe Tolerance" ) );

      ct_gaps->setValue     ( gap_fringe );
      ct_gaps->setRange     ( 0.0, 20.0 );
      ct_gaps->setSingleStep( 0.001 );
      ct_gaps->setNumButtons( 3 );
DbgLv(1) << " 2)gap_fringe" << gap_fringe << "idax" << idax;

      connect( ct_gaps, SIGNAL( valueChanged        ( double ) ), 
                        SLOT  ( set_fringe_tolerance( double ) ) );
   }
   else
   {
      lb_gaps->setText( tr( "Threshold for Scan Gaps" ) );
      
      ct_gaps->disconnect   ();
      ct_gaps->setRange     ( 10.0, 100.0 );
      ct_gaps->setSingleStep( 10.0 );
      ct_gaps->setValue     ( gap_thresh );
      ct_gaps->setNumButtons( 1 );
   }

   //ALEXEY: reset to 0, to plot each time representative wvl (for MWL)
   triple_plot_first_time[ triple_index ] = 0;
   //Also, set current wvl to the reference one
   qDebug() << "In new_triple_auto: ref_wvl_index -- " << iwavl_edit_ref_index[ triple_index ];
   cb_lplot->setCurrentIndex( iwavl_edit_ref_index[ triple_index ] );
   ////////////////////////////////////////////////////


   //Here, take into account excluded scans on per triple basis
   int scanExcl_begin_ind = editProfile_scans_excl[ cb_triple->currentText() ][0].toInt();
   int scanExcl_end_ind   = editProfile_scans_excl[ cb_triple->currentText() ][1].toInt();

   qDebug() << "IN new_Triple_auto(): triple -- " << cb_triple->currentText() 
   	    << "ScanBegin -- " << scanExcl_begin_ind
   	    << "ScanEnd -- "   << scanExcl_end_ind ;

   qDebug() << "Includes size before remove: " << includes.size();
   //for ( int i = 0; i < includes.size(); ++i  )
   //  qDebug() << "Includes after remove: " << includes[ i ];
   
   for ( int ii = 0; ii < scanExcl_begin_ind; ii++ )
     {
       includes.removeFirst();
     }
   //end of the scan set
   for ( int ii = data.scanData.size() - scanExcl_end_ind; ii < data.scanData.size(); ii++ )
     {
       includes.removeLast();
     }

   qDebug() << "Includes size after remove: " << includes.size();
   //for ( int i = 0; i < includes.size(); ++i  )
   //  qDebug() << "Includes after remove: " << includes[ i ];
   ///////////////////////////////////////////////////////////
   
   replot();
DbgLv(1) << "EDT:NewTr: DONE";


//QString triple = cb_triple->currentText() + ( isMwl ? " / " + swavl : "" );

//qDebug() << "Triple: " << triple << ", cb_triple_text: " << cb_triple->currentText();
//qDebug() << editProfile[ triple ] << ", " <<  editProfile[ cb_triple->currentText() ];

 
 
 if ( all_loaded ) 
   {
     if ( autoflow_expType == "ABDE" )
       {
	 setUnsetSaveBttn_abde();
	 if( !edited_triples_abde[ cb_triple->currentText() ] )  
	   return;
       }
     
     qDebug() << "NEW_TRIPLE_AUTO: all_loaded: " << all_loaded;

     qDebug() << "NEW_TRIPLE_AUTO: editProfile.count(): " << editProfile.count();
     
     // Debug
     for (int index = 0; index < cb_triple->count(); index++)
       qDebug() << cb_triple->itemText(index);

     qDebug() << "Current Triple: " << cb_triple->currentText();
     
     meniscus      = editProfile[ cb_triple->currentText() ][0].toDouble();
     range_left    = editProfile[ cb_triple->currentText() ][1].toDouble();
     range_right   = editProfile[ cb_triple->currentText() ][2].toDouble();
     plateau       = editProfile[ cb_triple->currentText() ][3].toDouble();
     baseline      = editProfile[ cb_triple->currentText() ][4].toDouble();
     baseline_od   = editProfile[ cb_triple->currentText() ][5].toDouble();

     le_meniscus ->setText( QString::number( meniscus,   'f', 3 ) );
     le_dataStart->setText( QString::number( range_left, 'f', 3 ) );
     le_dataEnd  ->setText( QString::number( range_right, 'f', 3 ) );
     le_plateau  ->setText( QString::number( plateau,     'f', 3 ) );

     qDebug() << "NEW_TRIPLE_AUTO: 2";
     
     if ( isMwl ) 
       le_baseline ->setText( QString::number( baseline,     'f', 3 ) );
     else
       {
	 QString str;
	 le_baseline->setText( str.sprintf( "%.3f (%.3e)", baseline, baseline_od ) );   
       }

     qDebug() << "NEW_TRIPLE_AUTO: 3";
     
     if ( editProfile[ cb_triple->currentText() ].count() > 6 )
       {
     	 if ( editProfile[ cb_triple->currentText() ][6] == "spike_true")
     	   {
	     qDebug() << "Spike_true";
	     remove_spikes_auto();
	   }
     	 else if (editProfile[ cb_triple->currentText() ][6] == "spike_false")
     	   {
	     qDebug() << "Spike_false";
     	   //undo_auto();
	   }
       }

     qDebug() << "NEW_TRIPLE_AUTO: 4";

     //ALEXEY: Apply data modifications like adjust_interference && calc_integral for IP data
     if ( dataType == "IP" )
       {
	 qDebug() << "NEW_TRIPLE_AUTO: 4a - IN IP";
	 
	 airGap_left   = editProfile[ cb_triple->currentText() ][7].toDouble();
	 airGap_right  = editProfile[ cb_triple->currentText() ][8].toDouble();

	 QString wkstr;
	 le_airGap->setText( wkstr.sprintf( "%.3f - %.3f", 
					    airGap_left, airGap_right ) );
	 
	 US_DataIO::EditValues  edits;
	 edits.airGapLeft  = airGap_left;
	 edits.airGapRight = airGap_right;
	 
	 QList< int > excludes;
         
	 for ( int i = 0; i < data.scanData.size(); i++ )
	   if ( ! includes.contains( i ) ) edits.excludes << i;
	   
	 US_DataIO::adjust_interference( data, edits );
	 
	 edits.rangeLeft    = range_left;
	 edits.rangeRight   = range_right;
	 edits.gapTolerance = ct_gaps->value();

	 US_DataIO::calc_integral( data, edits );
       }

     qDebug() << "NEW_TRIPLE_AUTO: 5";
     
     plot_range();
   }
 
}

// Select a new triple
void US_Edit::new_triple( int index )
{
   triple_index    = index;
   double gap_val  = ct_gaps->value();
DbgLv(1) << "EDT:NewTr: tripindex" << triple_index << "chgs" << changes_made << "gap_val" << gap_val;

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

   ct_gaps->disconnect();

   // Set up data indexes
   rb_radius->setChecked( true );
   rb_waveln->setChecked( false );

   index_data();
DbgLv(1) << "EDT:NewTr: trip,data index" << triple_index << data_index;

   if ( isMwl )
   {  // MWL:  restore the wavelengths for the newly selected triple
      if ( lrng_bycell )
      {  // Restore raw lambdas for an individual cell
         connect_mwl_ctrls( false );
         lambdas_by_cell();
         cb_lstart->clear();
         cb_lend  ->clear();
         cb_lstart->addItems( rawc_wvlns );
         cb_lend  ->addItems( rawc_wvlns );
         cb_lstart->setCurrentIndex( 0 );
         cb_lend  ->setCurrentIndex( nwaveln - 1 );
         slambda      = rawi_wvlns[ 0 ];
         elambda      = rawi_wvlns[ nwaveln - 1 ];
         dlambda      = 1;
         le_ltrng ->setText( tr( "%1 raw: %2 %3 to %4" )
            .arg( nwaveln ).arg( chlamb ).arg( slambda ).arg( elambda ) );
         connect_mwl_ctrls( true );
      }

      QVector< int > wvs;
      mwl_data.lambdas( wvs, triple_index );
      std::sort( wvs.begin(), wvs.end() );
      lambda_new_list ( wvs );
      le_lxrng ->setText( tr( "%1 MWL exports: %2 %3 to %4,"
                              " raw index increment %5" )
         .arg( nwavelo ).arg( chlamb ).arg( slambda ).arg( elambda )
         .arg( dlambda ) );
DbgLv(1) << "EDT:NewTr:  nwavelo" << nwavelo;
   }

   // Reset for new triple
   reset_triple(); 

   // Need to reconnect after reset
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
                       SLOT  ( new_triple         ( int ) ) );

   QString otdt   = dataType;
   edata          = outData[ data_index ];
   data           = *edata;
   QString swavl  = cb_lplot ->currentText();
   QString triple = cb_triple->currentText() + ( isMwl ? " / " + swavl : "" );
   int     idax   = triples.indexOf( triple );
   dataType = QString( QChar( data.type[ 0 ] ) ) 
            + QString( QChar( data.type[ 1 ] ) );
DbgLv(1) << "EDT:NewTr:   sw tri dx" << swavl << triple << idax << "dataType" << dataType
 << "gap_fringe gap_thresh" << gap_fringe << gap_thresh;
   if ( otdt != dataType )
   {  // Reset gap controls for new datatype
      if ( dataType == "IP" )
      {
         gap_thresh   = gap_val;
         ct_gaps->setValue     ( gap_fringe );
         ct_gaps->setRange     ( 0.0, 20.0 );
         ct_gaps->setSingleStep( 0.001 );
         ct_gaps->setNumButtons( 3 );
      }
      else
      {
         gap_fringe   = gap_val;
         ct_gaps->disconnect   ();
         ct_gaps->setRange     ( 10.0, 100.0 );
         ct_gaps->setSingleStep( 10.0 );
         ct_gaps->setValue     ( gap_thresh );
      }
   }

   // Enable pushbuttons
   pb_details  ->setEnabled( true );
   pb_report   ->setEnabled( all_edits );
   pb_include  ->setEnabled( true );
   pb_exclusion->setEnabled( true );
   pb_meniscus ->setEnabled( true );
   pb_airGap   ->setEnabled( false );
   pb_noise    ->setEnabled( false );
   pb_spikes   ->setEnabled( false );
   pb_invert   ->setEnabled( true );
   pb_undo     ->setEnabled( true );
   pb_float    ->setEnabled( true );
   pb_write    ->setEnabled( all_edits );
   ck_writemwl ->setEnabled( all_edits && isMwl );
   all_edits    = false;
   changes_made = all_edits;

   init_includes();

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );
   connect( ct_to,   SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   connect( ct_gaps, SIGNAL( valueChanged        ( double ) ), 
                     SLOT  ( set_fringe_tolerance( double ) ) );

   if ( expIsEquil )
   {  // Equilibrium
      cb_rpms->clear();
      trip_rpms.clear();

      for ( int ii = 0; ii < data.scanData.size(); ii++ )
      {  // build unique-rpm list for triple
         QString arpm = QString::number( data.scanData[ ii ].rpm );

         if ( ! trip_rpms.contains( arpm ) )
         {
            trip_rpms << arpm;
         }
      }
      cb_rpms->addItems( trip_rpms );

      le_edtrsp->setText( cb_triple->currentText() + " : " + trip_rpms[ 0 ] );
   }

   else
   {  // non-Equilibrium:  possibly re-do/un-do edits
      QString trbase   = cb_triple->currentText();
      triple           = trbase;

      if ( isMwl )
      {
         plotndx          = cb_lplot->currentIndex();
         swavl            = cb_lplot->currentText();
         triple          += " / " + swavl;
         plotrec          = swavl.toInt();
      }

      QString fname    = editFnames[ idax ];
      bool    app_edit = ( fname != "none" );
DbgLv(1) << "EDT:NewTr:   app_edit" << app_edit << "fname" << fname;

#if 0
      if ( app_edit )
      {  // This data has editing,  ask if it should be applied
         QStringList editfiles;

         if ( fname == "same" )
         {  // Need to find an edit file for this channel
            QString celchn   = trbase + " / ";
            int     wvxe     = nwavelo - 1;
DbgLv(1) << "EDT:NewTr:     wvxe ewvsiz" << wvxe << expc_wvlns.count();
            int     jdax     = triples.indexOf( celchn + expc_wvlns[ 0    ] );
            int     ldax     = triples.indexOf( celchn + expc_wvlns[ wvxe ] );
DbgLv(1) << "EDT:NewTr:      jdax ldax" << jdax << ldax;

            while ( jdax <= ldax )
            {
               if ( editFnames[ jdax ].length() > 4 )
               {
                  fname         = editFnames[ jdax ];
                  idax          = jdax;
                  break;
               }

               jdax++;
            }
         }

         QString trtype   = isMwl ? "cell/channel" : "triple";
         QString trvalu   = isMwl ? trbase         : triple;
DbgLv(1) << "EDT:NewTr:   tr type,valu" << trtype << trvalu; 

         QMessageBox mbox;
         QPushButton* pb_appl;
         QString msg      = tr( "Edits have been loaded or saved<br/>"
                                "for the current %1 (%2).<br/></br/>"
                                "Do you wish to<br/>"
                                "&nbsp;&nbsp;"
                                "apply them (<b>Apply Edits</b>) or<br/>"
                                "&nbsp;&nbsp;"
                                "ignore them (<b>Ignore Edits</b>)<br/>"
                                "in data displays?" )
                            .arg( trtype ).arg( trvalu );
         mbox.setIcon      ( QMessageBox::Question );
         mbox.setIcon      ( QMessageBox::Question );
         mbox.setTextFormat( Qt::RichText );
         mbox.setText      ( msg );
                            mbox.addButton( tr( "Ignore Edits"   ),
                                            QMessageBox::RejectRole );
         pb_appl          = mbox.addButton( tr( "Apply Edits" ),
                                            QMessageBox::AcceptRole );
         mbox.setDefaultButton( pb_appl );
         mbox.exec();
         app_edit         = ( mbox.clickedButton() == pb_appl );
      }

#endif
      if ( app_edit )
      {  // If editing chosen, apply it
         US_DataIO::EditValues parameters;

         US_DataIO::readEdits( workingDir + fname, parameters );

         apply_edits( parameters );
      }

      else
      {  // If no editing, be sure to turn off edits
         set_meniscus();
      }

DbgLv(1) << "EDT:NewTr:   men" << meniscus << "dx" << idax;
      plot_current( idax );
   }

   // Reset GAPS labels and values based on this triple's type
   if ( dataType == "IP" )
   {
      lb_gaps->setText( tr( "Fringe Tolerance" ) );

      ct_gaps->setValue     ( gap_fringe );
      ct_gaps->setRange     ( 0.0, 20.0 );
      ct_gaps->setSingleStep( 0.001 );
      ct_gaps->setNumButtons( 3 );
DbgLv(1) << " 2)gap_fringe" << gap_fringe << "idax" << idax;

      connect( ct_gaps, SIGNAL( valueChanged        ( double ) ), 
                        SLOT  ( set_fringe_tolerance( double ) ) );
   }
   else
   {
      lb_gaps->setText( tr( "Threshold for Scan Gaps" ) );
      
      ct_gaps->disconnect   ();
      ct_gaps->setRange     ( 10.0, 100.0 );
      ct_gaps->setSingleStep( 10.0 );
      ct_gaps->setValue     ( gap_thresh );
      ct_gaps->setNumButtons( 1 );
   }

   replot();
DbgLv(1) << "EDT:NewTr: DONE";
   int row = cb_triple->currentIndex() + 1;
   pb_nextChan->setEnabled( row < cb_triple->count() );

}

// Select a new speed within a triple
void US_Edit::new_rpmval( int index )
{
   QString srpm = cb_rpms->itemText( index );

   set_meniscus();

   le_edtrsp->setText( cb_triple->currentText() + " : " + srpm );

   plot_scan();
}

// Mark data as floating
void US_Edit::floating( void )
{
   floatingData = ! floatingData;
   if ( floatingData )
      pb_float->setIcon( check );
   else
      pb_float->setIcon( QIcon() );

}

// Save edit profile(s)
void US_Edit::write( void )
{
   // Determine if we are using the database
   US_DB2* dbP    = NULL;

   if ( disk_controls->db() )
   {
      US_Passwd pw;
      dbP            = new US_DB2( pw.getPasswd() );

      if ( dbP->lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this, tr( "Connection Problem" ),
           tr( "Could not connect to database: \n" ) + dbP->lastError() );
         return;
      }
   }

   // Compute and store the post-stream bottom value
   US_SimulationParameters simparams;
   simparams.initFromData( dbP, data, false, runID, dataType );
   bottom         = simparams.bottom;
//*DEBUG*
//bottom+=0.001;
//*DEBUG*
DbgLv(1) << "ED: Wr : bottom" << bottom;

   if ( !expIsEquil )
   {  // non-Equilibrium:  write single current edit (if "all" unchecked)
      if ( isMwl &&  ck_writemwl->isChecked() )
      {  // Write edits for all triples in the current cell/channel
         write_mwl();
      }

      else
      {  // Write single triple's edit
         triple_index = cb_triple->currentIndex();

         write_triple();
      }
   }

   else
   {  // Equilibrium:  loop to write all edits
      for ( int jr = 0; jr < triples.size(); jr++ )
      {
         triple_index = jr;
         data         = *outData[ jr ];

         write_triple();
      }
   }

   changes_made = false;
   pb_write    ->setEnabled( false );
   ck_writemwl ->setEnabled( false );
   pb_nextChan ->setEnabled( triples.size() > 1 && cb_triple->currentIndex() < triples.size()-1 );
}


//Check if EditedProfiles already saved/started to be saved into DB
bool US_Edit::isSaved_auto( void )
{
   bool isDataSaved = false;

   // Then we're working in DB, so verify connectivity
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );
   
   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::information( this,
				 tr( "Error" ),
				 tr( "Error making the DB connection.\n" ) );
       return isDataSaved;
     }

    int editProfile_count = 0;

    // Check if the edit profiles corresponding top filename/label already exist in the DB
    QStringList qry("count_editprofiles");
    qry << filename_runID_auto;
    editProfile_count = db->functionQuery( qry );

    qDebug() << "Query: " << qry;
    qDebug() << "# Saved Edit Profiles: " << editProfile_count;
    
    if ( editProfile_count  )
      isDataSaved = true;

    return isDataSaved;
}


//Read autoflowStages record
int US_Edit::read_autoflow_stages_record( int autoflowID  )
{
   int status = 0;
  
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return status;
     }


   //qDebug() << "BEFORE query ";
   QStringList qry;
   qry << "autoflow_edit_status"
       << QString::number( autoflowID );
   
   status = db->statusQuery( qry );
   //qDebug() << "AFTER query ";

   return status;
}

//Set autoflowStages record back to "unlnown"
void US_Edit::revert_autoflow_stages_record( int autoflowID )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return;
     }
   
   //qDebug() << "BEFORE query ";
   QStringList qry;
   qry << "autoflow_edit_status_revert"
       << QString::number( autoflowID );
   
   db->query( qry );
   //qDebug() << "AFTER query ";
   
}

// Query autoflow record
QMap< QString, QString> US_Edit::read_autoflow_record( int autoflowID  )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   QMap <QString, QString> protocol_details;
   
   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return protocol_details;
     }

   QStringList qry;
   qry << "read_autoflow_record"
       << QString::number( autoflowID );
   
   db->query( qry );

   if ( db->lastErrno() == US_DB2::OK )      // Autoflow record exists
     {
       while ( db->next() )
	 {
	   protocol_details[ "protocolName" ]   = db->value( 0 ).toString();
	   protocol_details[ "CellChNumber" ]   = db->value( 1 ).toString();
	   protocol_details[ "TripleNumber" ]   = db->value( 2 ).toString();
	   protocol_details[ "duration" ]       = db->value( 3 ).toString();
	   protocol_details[ "experimentName" ] = db->value( 4 ).toString();
	   protocol_details[ "experimentId" ]   = db->value( 5 ).toString();
	   protocol_details[ "runID" ]          = db->value( 6 ).toString();
	   protocol_details[ "status" ]         = db->value( 7 ).toString();
           protocol_details[ "dataPath" ]       = db->value( 8 ).toString();   
	   protocol_details[ "OptimaName" ]     = db->value( 9 ).toString();
	   protocol_details[ "runStarted" ]     = db->value( 10 ).toString();
	   protocol_details[ "invID_passed" ]   = db->value( 11 ).toString();

	   protocol_details[ "correctRadii" ]   = db->value( 13 ).toString();
	   protocol_details[ "expAborted" ]     = db->value( 14 ).toString();
	   protocol_details[ "label" ]          = db->value( 15 ).toString();
	   protocol_details[ "gmpRun" ]         = db->value( 16 ).toString();

	   protocol_details[ "filename" ]       = db->value( 17 ).toString();
	   protocol_details[ "aprofileguid" ]   = db->value( 18 ).toString();

	   protocol_details[ "expType" ]       = db->value( 26 ).toString();
	   protocol_details[ "dataSource" ]     = db->value( 27 ).toString();
	   	   
	 }
     }

   return protocol_details;
}

// Call manuall editor
void US_Edit::manual_edit_auto( void )
{
  int currChIndex = cb_triple->currentIndex();
  //int plotInd = index_data();
  int plotInd = plotndx;
  qDebug() << "IN manual_edit_auto( void ), plotInd -- " << plotInd;
  
  sdiag = new US_Edit( allData, triples, workingDir, currChIndex, plotInd );
  /** The following will block parent windows from closing BUT not from continuing timer execution ***/
  sdiag->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
  sdiag->setWindowModality(Qt::ApplicationModal);
  /***************************************************************************************************/

  connect( sdiag, SIGNAL( pass_edit_params( QMap< QString, QStringList> & ) ),
	   this,  SLOT( update_triple_edit_params (  QMap < QString, QStringList > &) ) );

  sdiag->show();
}


// Update triple's edit params with those obtained manually..
void US_Edit:: update_triple_edit_params (  QMap < QString, QStringList > &  edit_params )
{
  QString t_name = edit_params.keys()[0];
  editProfile[ t_name ] = edit_params[ t_name ];

  qDebug() << "In update_triple_edit_params: t_name -- " << t_name;
  qDebug() << "In update_triple_edit_params: editProfile[ t_name ] -- " << editProfile[ t_name ];

  automatic_meniscus[ t_name ] = false;

  //Put a comment, why this triple was processed manually IF NOT "ABDE"
  if ( autoflow_expType != "ABDE" )
    {
      bool ok;
      QString msg = QString(tr("Put a comment on MANUAL editing :"));
      QString default_text = QString(tr("Reason for MANUAL editing: "));
      QString comment_t    = QInputDialog::getText( this,
						    tr( "Reason for MANUAL editing" ),
						    msg, QLineEdit::Normal, default_text, &ok );
      
      if ( !ok )
	{
	  return;
	}
      
      manual_edit_comments[ t_name ] = comment_t;
    }
  else
    {
      edited_triples_abde[ t_name ] = true;
      setUnsetSaveBttn_abde();
    }
      
  ///////////////////////////////////////////////////////

  new_triple_auto( 0 ); 
}

//for ABDE, check if all triples were processed & enable/diable Save
void US_Edit::setUnsetSaveBttn_abde( void )
{
  bool all_processed = true;
  QMap<QString, bool>::iterator os;
  for ( os = edited_triples_abde.begin(); os != edited_triples_abde.end(); ++os )
    {
      if ( !os.value() )
	{
	  all_processed = false;
	  break;
	}
    }

  pb_write -> setEnabled( all_processed );
}

// Save edit profile(s)
void US_Edit::write_auto( void )
{
  // //TEMP: DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
  // record_edit_status( automatic_meniscus, dataType );
  // exit(1);
  
  /****  TEMP1 **/
  //--- Check if saving already initiated
  int status_edit_unique;
  status_edit_unique = read_autoflow_stages_record( autoflowID_passed );

  qDebug() << "status_edit_unique -- " << status_edit_unique ;
  
  if ( !status_edit_unique )
    {

      QMessageBox::information( this,
  				tr( "The Program State Updated / Being Updated" ),
  				tr( "The program advanced or is advancing to the next stage!\n\n"
  				    "This happened because you or different user "
  				    "has already saved edit profiles into DB using different program "
  				    "session and the program is proceeding to the next stage. \n\n"
  				    "The program will return to the autoflow runs dialog where "
  				    "you can re-attach to the actual current stage of the run. "
  				    "Please allow some time for the status to be updated.") );
   
   
      reset();
      emit back_to_initAutoflow( );
      return;
    }
    //-------------------------------------------
  /***/
  
  pb_write       ->setEnabled( false );

  /*
  //TEST : this cannot be at the beginning as all optics need to be processed!
  for ( int i = 0; i < triples.size(); ++i )
    {
      //qDebug() << "Triple name: " << triples[i];
	  
      QString current_triple_name = triples[i];
      current_triple_name.replace(" / ",".");
      
      QStringList triple_parts = current_triple_name.split(".");
      
      if ( dataType == "IP" )
	current_triple_name = triple_parts[0] + "." + triple_parts[1] + "." + QString("Interference");
      
      triples_all_optics << current_triple_name;
      channels_all << triple_parts[0] + "." + triple_parts[1];
    }
  channels_all.removeDuplicates();

  for ( int i = 0; i < channels_all.size(); ++i  )
    {
      qDebug() << channels_all[i];
      isSet_ref_wvl[ channels_all[i] ] = false;
    }
  
  // Process by channel
  for ( int i = 0; i < channels_all.size(); ++i  )
    {
      for ( int j = 0; j < triples_all_optics.size(); ++j )
	{
	  if ( triples_all_optics[j].contains( channels_all[i] ) )
	    {
	      if ( triples_all_optics[j].contains( "Interference" ) )
		qDebug() << triples_all_optics[j] << compose_json( true );
	      else
		{
		  if ( !isSet_ref_wvl[ channels_all[i] ] )
		    {
		      qDebug() << triples_all_optics[j] << compose_json( true );
		      isSet_ref_wvl[ channels_all[i] ] = true;
		    }
		  else
		    qDebug() << triples_all_optics[j] << compose_json( false );
		}
	    }
	}
    }
  
  return;
  // Test
  */
  
      
  /** Check if stage is being or has already changed ***/
  QMap < QString, QString > autoflow_details;
  autoflow_details = read_autoflow_record( autoflowID_passed );
  
  qDebug() << "autoflowID_passed, autoflow status, isSaved_auto(): " <<  autoflowID_passed << ", " << autoflow_details[ "status" ]; // << ", " << isSaved_auto();

  // Check if all optics systems were processed //
  bool all_processed = true;
  QMap<QString, int>::iterator os;
  for ( os = runTypes_map.begin(); os != runTypes_map.end(); ++os )
    {
      if ( os.value() )
	{
	  all_processed = false;
	  break;
	}
    }
  //////////////////////////////////////////////////////////

  // ALEXEY: before processing next optics system (if multiple), push triple names to the general triple array:
  // Also, push channel names into general channel array
  for ( int i = 0; i < triples.size(); ++i )
    {
      //qDebug() << "Triple name: " << triples[i];
      
      QString current_triple_name = triples[i];
      current_triple_name.replace(" / ",".");
      
      QStringList triple_parts = current_triple_name.split(".");
      
      if ( dataType == "IP" )
	current_triple_name = triple_parts[0] + "." + triple_parts[1] + "." + QString("Interference");
      
      triples_all_optics << current_triple_name;
      channels_all << triple_parts[0] + "." + triple_parts[1];
    }
  //-----------------------------------------------------------------//

  /*** TEMP1 **/
   if ( autoflow_details[ "status" ]  != "EDIT_DATA"  || isSaved_auto() )
     {
       if ( runType_combined_IP_RI )
   	{
   	  if ( !all_processed )
   	    {
   	      QMessageBox::information( this,
   					tr( "Edit Profiles for Current Optical System Already Saved" ),
   					tr( "It appears that edit profiles for the current optical system are already saved!\n\n"
   					    "The program will switch to processing data for the next optical system... " ));

   	      cb_triple->disconnect();

   	      
   	      //set autoflowStages record to "unknown" again !!
   	      revert_autoflow_stages_record( autoflowID_passed );
   	      
   	      
   	      reset();
   	      emit process_next_optics( );
   	      return;
   	    }
   	  else
   	    {
   	      QMessageBox::information( this,
   					tr( "The Program State Updated / Being Updated" ),
   					tr( "The program advanced or is advancing to the next stage!\n\n"
   					    "This happened because you or different user "
   					    "has already saved edit profiles into DB using different program "
   					    "session and the program is proceeding to the next stage. \n\n"
   					    "The program will return to the autoflow runs dialog where "
   					    "you can re-attach to the actual current stage of the run. "
   					    "Please allow some time for the status to be updated.") );
   	      
   	      
   	      reset();
   	      emit back_to_initAutoflow( );
   	      return;
   	    }
   	}
       else
   	{
   	  QMessageBox::information( this,
   				    tr( "The Program State Updated / Being Updated" ),
   				    tr( "The program advanced or is advancing to the next stage!\n\n"
   					"This happend because you or different user "
   					"has already saved edit profiles into DB using different program "
   					"session and the program is proceeding to the next stage. \n\n"
   					"The program will return to the autoflow runs dialog where "
   					"you can re-attach to the actual current stage of the run. "
   					"Please allow some time for the status to be updated.") );
   	  
   	  
   	  reset();
   	  emit back_to_initAutoflow( );
   	  return;
   	}
     }
  /***/
  
  /*******************************************************/
  
   // Determine if we are using the database
   US_DB2* dbP  = NULL;

   if ( disk_controls->db() )
   {
      US_Passwd pw;
      dbP            = new US_DB2( pw.getPasswd() );

      if ( dbP->lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this, tr( "Connection Problem" ),
           tr( "Could not connect to database: \n" ) + dbP->lastError() );
         return;
      }
   }

   
   /*************************************************************************************************************/
   // We need to  insert submission form dialog (with password...)
   /*************************************************************************************************************/
   QStringList qry1;
   qry1 <<  QString( "get_user_info" );
   dbP-> query( qry1 );
   dbP-> next();
   int u_ID        = dbP-> value( 0 ).toInt();
   QString u_fname = dbP-> value( 1 ).toString();
   QString u_lname = dbP-> value( 2 ).toString();
   int u_lev       = dbP-> value( 5 ).toInt();
   
   QString user_submitter = u_lname + ", " + u_fname;
   
   gmp_submitter_map.clear();
   US_Passwd   pw_at;
   gmp_submitter_map  = pw_at.getPasswd_auditTrail( "GMP Run EDIT Form", "Please fill out GMP run EDIT form:", user_submitter );
   
   int gmp_submitter_map_size = gmp_submitter_map.keys().size();
   qDebug() << "Submitter map: "
	    << gmp_submitter_map.keys()  << gmp_submitter_map.keys().size() << gmp_submitter_map_size
	    << gmp_submitter_map.keys().isEmpty() 
	    << gmp_submitter_map[ "User:" ]
	    << gmp_submitter_map[ "Comment:" ]
	    << gmp_submitter_map[ "Master Password:" ];
   
   if ( gmp_submitter_map_size == 0 ||  gmp_submitter_map.keys().isEmpty() )
     {
       revert_autoflow_stages_record( autoflowID_passed );
       pb_write       ->setEnabled( true );

       //DEBUG 
       for ( int i = 0; i < channels_all.size(); ++i  )
	 qDebug() << "[NO COMMMENT] BEFORE AUTOFLOW_ANALYSIS: channel name --" << channels_all[i];
       for ( int j = 0; j < triples_all_optics.size(); ++j )
	 qDebug() << "[NO COMMNET] BEFORE AUTOFLOW_ANALYSIS: triple name -- " << triples_all_optics[j];
       //END DEBUG
       
       return;
     }
   /*************************************************************************************************************/
   
   qDebug() << "NOW, START Saving"; 


   /* TEMPORARY ***/
   // // Compute and store the post-stream bottom value
   // US_SimulationParameters simparams;
   // simparams.initFromData( dbP, data, false, runID, dataType );
   // bottom         = simparams.bottom;
//*DEBUG*
//bottom+=0.001;
//*DEBUG*

   qDebug() << " bottom" << bottom;

   if ( !expIsEquil )
   {  // non-Equilibrium:  write single current edit (if "all" unchecked)
      if ( isMwl &&  ck_writemwl->isChecked() )
      {  // Write edits for all triples in the current cell/channel
	qDebug() << "Saving MWL";

	for ( int trx = 0; trx < cb_triple->count(); trx++ )
	  {
	    qDebug() << "Writing MWL, channel: " << trx << ": " << cb_triple->itemText( trx );
	    write_mwl_auto( trx );
	  }
      }

      else
      {  // Write single triple's edit

	qDebug() << "Saving non-MWL";
	
	// if ( dataType != "IP" )
	//   {
	    for ( int trx = 0; trx < cb_triple->count(); trx++ )
	      {
		qDebug() << "Writing non-MWL, channel: " << trx << ": " << cb_triple->itemText( trx );
		write_triple_auto( trx );
	      }
	//   }
	// else
	//   {
	//     // <---- For now: for Interference data, do editing && save manually
	//     triple_index = cb_triple->currentIndex();
	//     bottom = centerpieceParameters[ triple_index ][1].toDouble();  //Should be from centerpiece info from protocol 
	//     write_triple();           
	//   }
	  
      }
   }

   else
   {  // Equilibrium:  loop to write all edits
      for ( int jr = 0; jr < triples.size(); jr++ )
      {
         triple_index = jr;
         data         = *outData[ jr ];

         write_triple_auto( triple_index );
      }
   }



   // // ALEXEY: before processing next optics system (if multiple), push triple names to the general triple array:
   // // Also, push channel names into general channel array
   // for ( int i = 0; i < triples.size(); ++i )
   //   {
   //     //qDebug() << "Triple name: " << triples[i];
       
   //     QString current_triple_name = triples[i];
   //     current_triple_name.replace(" / ",".");
      
   //     QStringList triple_parts = current_triple_name.split(".");
       
   //     if ( dataType == "IP" )
   // 	 current_triple_name = triple_parts[0] + "." + triple_parts[1] + "." + QString("Interference");
       
   //     triples_all_optics << current_triple_name;
   //     channels_all << triple_parts[0] + "." + triple_parts[1];
   //   }

   // Check If all Optical Systems processed:::::::::::::::::::::::::::::
   qDebug() << "SAVING: Optics Type, all_processed:  " << filename_runID_auto << all_processed;


   //Now, make a record on was the Reference Scan defined automatically (for "RI" type) && who did SAVE the data
   record_edit_status( automatic_meniscus, dataType );
   
   //////////////////////////////////////////////////////////////
   
   if ( !all_processed )
     {
       /****/
       //set autoflowStages record to "unknown" again !!
       revert_autoflow_stages_record( autoflowID_passed );
       /****/
       
       reset();
       emit process_next_optics( );
       return;
     }
   ////////////////////////////////////////////////////////////////////

   // Now, remove duplicates from channels array, fill QMap keeping track on if reference wavelength set for each channel (if MWL) 
   channels_all       .removeDuplicates();
   triples_all_optics .removeDuplicates(); //Absence of this caused incorrect analyses list: need to test!!!
   
     
   for ( int i = 0; i < channels_all.size(); ++i  )
     {
       qDebug() << "BEFORE AUTOFLOW_ANALYSIS: channel name --" << channels_all[i]; // RA:S:: "1.S"
       isSet_ref_wvl[ channels_all[i] ] = false;
     }

   //DEBUG
   for ( int j = 0; j < triples_all_optics.size(); ++j )
     qDebug() << "BEFORE AUTOFLOW_ANALYSIS: triple name -- " << triples_all_optics[j]; //RA:S:: "1.S.280"
   
   
   // Process triples by channel, generate appropriate JSON (with or without 2DSA_FM stage) for autoflowAnalysis record && create those records
   QStringList AnalysisIDs;
   
   for ( int i = 0; i < channels_all.size(); ++i  )
     {
       qDebug() << "AUTOFLOW_ANALYSIS: channel name -- " << channels_all[i];
       for ( int j = 0; j < triples_all_optics.size(); ++j )
	 {
	   if ( triples_all_optics[j].contains( channels_all[i] ) )
	     {
	       qDebug() << "AUTOFLOW_ANALYSIS: triple " << triples_all_optics[j] <<  " ,containing channel " <<  channels_all[i];
	       
	       int ID = 0;
	       QString json_status;
	       
	       //Interference
	       if ( triples_all_optics[j].contains( "Interference" ) )
		 {
		   if ( isSet_to_analyse( triples_all_optics[ j ] , QString("Interf") ) &&
			isSet_to_analyse_triple( triples_all_optics[ j ] , QString("Interf") ) )
		     {
		       //ALEXEY: here for combined run, the filename_runID_auto would be incorrectly set to that of the second type, the "-RI"
		       //Need to use "-IP"
		       if ( runType_combined_IP_RI )
			 filename_runID_auto = filename_runID_auto_base + "-IP";
		       
		       json_status = compose_json( true );
		       qDebug() << triples_all_optics[j] << json_status;

		       ID = create_autoflowAnalysis_record( dbP, triples_all_optics[j], json_status );
		       
		       if (ID)
			 create_autoflowAnalysisStages_record( dbP, ID );
		     }
		 }
	       //UV.vis
	       else		 
		 {
		   if ( isSet_to_analyse( triples_all_optics[ j ],  QString("UV/vis") ) &&
			isSet_to_analyse_triple( triples_all_optics[ j ],  QString("UV/vis") ) )
		     {
		       //ALEXEY: here for combined run, the filename_runID_auto must be "filename_base-RI"
		       //Need to use "-RI"
		       if ( runType_combined_IP_RI )
			 filename_runID_auto = filename_runID_auto_base + "-RI";

		       // Was channel's reference wvl set? 		       
		       if ( !isSet_ref_wvl[ channels_all[i] ] )
			 {
			   // If there is an infortmaiton on specific triple to edit (for a current channel) in the AProfile?
			   if ( isSet_edit_info_for_channel( triples_all_optics[ j ],  QString("UV/vis") ) )
			     {
			       qDebug() << "isSet_edit_info_for_channel() TRUE: ";
			       // If this particular  triple set for FM fit (and edit): defined by "wvl_edit" attr in Aprofile
			       if ( isSet_to_edit_triple( triples_all_optics[ j ],  QString("UV/vis") ) )
				 {
				   json_status = compose_json( true );
				   qDebug() << "isSet_to_edit_triple() TRUE: " << triples_all_optics[j] << json_status;
				   
				   ID = create_autoflowAnalysis_record( dbP, triples_all_optics[j], json_status );
				   
				   //So, reference wvl is defiend as the triple specified in AProfile
				   isSet_ref_wvl[ channels_all[i] ] = true;
				   
				   if (ID)
				     create_autoflowAnalysisStages_record( dbP,  ID );
				 }
			       else
				 {
				   json_status = compose_json( false );
				   qDebug() << "isSet_to_edit_triple() FALSE: " << triples_all_optics[j] << json_status;
				   
				   ID = create_autoflowAnalysis_record( dbP, triples_all_optics[j], json_status );
				 }
			     }
			   //No informaiton on specifi triple to edit -- select 1st triple in a channel as default
			   else
			     {
			       json_status = compose_json( true );
			       qDebug() << "isSet_edit_info_for_channel() FALSE: " << triples_all_optics[j] << json_status;
			       
			       ID = create_autoflowAnalysis_record( dbP, triples_all_optics[j], json_status );

			       //So, reference wvl is defiend as the 1st one in the channel domain
			       isSet_ref_wvl[ channels_all[i] ] = true;
			       
			       if (ID)
				 create_autoflowAnalysisStages_record( dbP,  ID );
			     }
			 }
		       else
			 {
			   json_status = compose_json( false );
			   qDebug() << triples_all_optics[j] << json_status;
			   
			   ID = create_autoflowAnalysis_record( dbP, triples_all_optics[j], json_status );
			 }
		     }
		 }
	       
	       if ( ID )
		 AnalysisIDs << QString::number( ID );
	       
	     }
	 }
     }
   
   
   le_status->setText( tr( "Saving COMPLETE " ) );
   qApp->processEvents();

   //If AnalysisIDs empty, STOP here and return: Nothing to process further; emit signal to reset
   if ( AnalysisIDs.isEmpty() )
     {
       qDebug() << "Nothing to Analyse!!!";
       QMessageBox::information( this,
			     tr( "Saving of Edit Profiles is Complete." ),
			     tr( "Edit profiles were saved successfully. \n\n"
				 "No triples were set for analysis. Program will be reset." ) );
       reset();
       //delete_autoflow_record();
       emit back_to_initAutoflow( );
       return;
     }
   
   // Otherwise, proceed to analysis: Now we need to Update autoflow record, reset GUI && send signal to switch to Analysis stage:
   QString AnalysisIDsString = AnalysisIDs.join(",");
   update_autoflow_record_atEditData( dbP, AnalysisIDsString );

   //Pass analsyisIDs generated to details_at_editing_local
   details_at_editing_local[ "analysisIDs" ] = AnalysisIDsString;
   
   //ALEXEY: here major actions on setting analysis tables etc. !!!!
   QMessageBox::information( this,
			     tr( "Saving of Edit Profiles is Complete." ),
			     tr( "Edit profiles were saved successfully. \n\n"
				 "The program will switch to Analysis stage." ) );
   reset();
   this->close();
   qApp->processEvents();
   
   emit edit_complete_auto( details_at_editing_local  );   
}

//Record statuses of the auto/manual meniscus determinaiton, on per-channel basis
void US_Edit::record_edit_status( QMap< QString, bool> auto_meniscus, QString dtype )
{
  //DEBUG
  qDebug() << "Data Type: " << dtype;
  QMap<QString, bool>::iterator os;
  for ( os = auto_meniscus.begin(); os != auto_meniscus.end(); ++os )
    qDebug() << "For channel " << os.key() << ", meniscus determined automatically? " << os.value();
  //END DEBUG

  // Check DB connection
  US_Passwd pw;
  QString masterpw = pw.getPasswd();
  US_DB2* db = new US_DB2( masterpw );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
      return;
    }
  
  QStringList qry;

  //get user info
  qry.clear();
  qry <<  QString( "get_user_info" );
  db->query( qry );
  db->next();

  int ID        = db->value( 0 ).toInt();
  QString fname = db->value( 1 ).toString();
  QString lname = db->value( 2 ).toString();
  QString email = db->value( 4 ).toString();
  int     level = db->value( 5 ).toInt();

  qDebug() << "IN US_edit_AUTO, record status: ID,name,email,lev" << ID << fname << lname << email << level;
  
  //Record to autoflowStatus:
  qry.clear();

  QString editRI_IP_Json;
  editRI_IP_Json. clear();
  editRI_IP_Json += "{ \"Person\": ";
  
  editRI_IP_Json += "[{";
  editRI_IP_Json += "\"ID\":\""     + QString::number( ID )     + "\",";
  editRI_IP_Json += "\"fname\":\""  + fname                     + "\",";
  editRI_IP_Json += "\"lname\":\""  + lname                     + "\",";
  editRI_IP_Json += "\"email\":\""  + email                     + "\",";
  editRI_IP_Json += "\"level\":\""  + QString::number( level )  + "\"";
  editRI_IP_Json += "}],";

  //Now, add comment from SAVING form:
  editRI_IP_Json += "\"Comment when SAVED\": \"" + gmp_submitter_map[ "Comment:" ] + "\",";      

  //Meniscus
  editRI_IP_Json += "\"Meniscus\": ";
  editRI_IP_Json += "[{";
  
  for ( os = auto_meniscus.begin(); os != auto_meniscus.end(); ++os )
    {
      QString meniscus_method = os.value() ? QString("automated") : QString("manual");
      
      editRI_IP_Json += "\"" + os.key()  + "\":\"" +   meniscus_method;

      if ( !os.value() )
	{
	  editRI_IP_Json += ", Comment: " + manual_edit_comments[ os.key() ] + "\",";
	}
      else
	editRI_IP_Json += "\",";
    }
  
  editRI_IP_Json.chop(1);
  editRI_IP_Json += "}]";
  
  editRI_IP_Json += "}";

  qDebug() << "in record_edit_status: editRI_IP_Json  -- " << editRI_IP_Json;
  
  if ( autoflowStatusID )
    {
      //update
      if ( dtype == "RI" || dtype == "RA" )
	{
	  qry << "update_autoflowStatusEditRI_record"
	      << QString::number( autoflowStatusID )
	      << QString::number( autoflowID_passed )
	      << editRI_IP_Json;
	  
	  db->query( qry );
	}

      if ( dtype == "IP" )
	{
	  qry << "update_autoflowStatusEditIP_record"
	      << QString::number( autoflowStatusID )
	      << QString::number( autoflowID_passed )
	      << editRI_IP_Json;
	  
	  db->query( qry );
	}
    }
  else
    {
      QMessageBox::warning( this, tr( "AutoflowStatus Record Problem" ),
			    tr( "autoflowStatus (EDIT {RI|RA,IP}): There was a problem with identifying "
				"a record in autoflowStatus table for a given run! \n" ) );
      
      return;
    }
}


//Delete autoflow record 
void US_Edit::delete_autoflow_record( void )
{
  QString runID_numeric     = details_at_editing_local[ "runID" ];
  QString OptimaName        = details_at_editing_local[ "OptimaName" ]; 

  // Check DB connection
  US_Passwd pw;
  QString masterpw = pw.getPasswd();
  US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return;
     }

   QStringList qry;
   // qry << "delete_autoflow_record"
   //     << runID_numeric
   //     <<  OptimaName;
   qry << "delete_autoflow_record_by_id"
       << QString::number( autoflowID_passed );

   //db->query( qry );

   // OR
   
   int status = db->statusQuery( qry );
   
   if ( status == US_DB2::NO_AUTOFLOW_RECORD )
     {
       QMessageBox::warning( this,
			     tr( "Autoflow Record Not Deleted" ),
			     tr( "No autoflow record\n"
				 "associated with this experiment." ) );
       return;
     }

   //Also, delete autoflowStages record
   qry.clear();
   qry << "delete_autoflow_stages_record"
       << QString::number( autoflowID_passed );

   db->statusQuery( qry );
}


bool US_Edit::isSet_to_analyse( QString triple_name, QString opsys )
{
  bool have_run = false;

  //In case anaprofile does not have "run" attr, so QMap<> channels_to_analyse empty (older protocols):
  if ( channels_to_analyse.isEmpty() )
    {
      qDebug() << "It looks like older protocol is in use: QMap channels_to_analyse is EMPTY!";
      
      have_run = true;
      return have_run;
    }
  
  QStringList triple_name_list = triple_name.split(".");
  QString channel = triple_name_list[0] + triple_name_list[1];

  qDebug() << "[isSet_to_analyse()] channel1 -- " << channel;
  if ( dataSource. contains("DiskAUC:Absorbance") && channel.contains("S")  )
    channel = channel.replace("S","A");
  qDebug() << "[isSet_to_analyse()] channel2 -- " << channel;
  
  QMap<QString, bool>::iterator jj;
  for ( jj = channels_to_analyse.begin(); jj != channels_to_analyse.end(); ++jj )
    {
      if ( jj.key().contains( channel ) && jj.key().contains( opsys ) )
	{
	  if ( jj.value()  )
	    {
	      qDebug() << "Triple " << triple_name << " of channel (" << jj.key() << " will be analysed.";
	      have_run = true;	      
	      break;
	    }
	}
    }

  return have_run;
}

bool US_Edit::isSet_to_analyse_triple( QString triple_name, QString opsys )
{
  bool have_run = true;

  //In case anaprofile does not have "wvl_not_run" attr, so QMap<> triples_skip_analysis empty (older protocols):
  if ( triples_skip_analysis.isEmpty() )
    {
      qDebug() << "It looks like older protocol is in use: QMap triples_skip_analysis is EMPTY!";
      
      have_run = true;
      return have_run;
    }
  
  QStringList triple_name_list = triple_name.split(".");
  QString channel = triple_name_list[0] + triple_name_list[1];

  qDebug() << "[isSet_to_analyse_triple()] channel1 -- " << channel;
  if ( dataSource. contains("DiskAUC:Absorbance") && channel.contains("S")  )
    channel = channel.replace("S","A");
  qDebug() << "[isSet_to_analyse_triple()] channel2 -- " << channel;
  
  QString wvl = triple_name_list[2];    
  
  QMap<QString, QString>::iterator jj;
  for ( jj = triples_skip_analysis.begin(); jj != triples_skip_analysis.end(); ++jj )
    {
      if ( jj.key().contains( channel ) && jj.key().contains( opsys ) )
	{
	  QString wvl_list_skipped = jj.value();

	  if ( wvl_list_skipped.contains( wvl ) )
	    {
	      qDebug() << "Triple " << triple_name << " of channel (" << jj.key() << " will NOT be analysed.";
	      have_run = false;	      
	      break;
	    }
	}
    }
  
  return have_run;
}


bool US_Edit::isSet_edit_info_for_channel( QString triple_name, QString opsys )
{
  bool have_set = false;
    
  QStringList triple_name_list = triple_name.split(".");
  QString channel = triple_name_list[0] + triple_name_list[1];

  qDebug() << "[isSet_edit_info_for_channel() channel1 -- " << channel;
  if ( dataSource. contains("DiskAUC:Absorbance") && channel.contains("S")  )
    channel = channel.replace("S","A");
  qDebug() << "[isSet_edit_info_for_channel()] channel2 -- " << channel;
  
  
  QMap<QString, QString>::iterator jj;
  for ( jj = triple_to_edit.begin(); jj != triple_to_edit.end(); ++jj )
    {
      if ( jj.key().contains( channel ) && jj.key().contains( opsys ) )
	{
	  QString wvl_set_edit = jj.value();

	  if ( !wvl_set_edit.isEmpty() )
	    {
	      qDebug() << "Edit Informaiton for channel " << channel << " is SET.";
	      have_set = true;	      
	      break;
	    }
	}
    }
  
  return have_set;
}

			    
bool US_Edit::isSet_to_edit_triple( QString triple_name, QString opsys )
{
  bool have_run = false;

  //In case anaprofile does not have "wvl_edit" attr, so QMap<> triple_to_edit empty (older protocols):
  if ( triple_to_edit.isEmpty() )
    {
      qDebug() << "It looks like older protocol is in use: QMap triple_to_edit is EMPTY!";
      
      have_run = true;
      return have_run;
    }
  
  QStringList triple_name_list = triple_name.split(".");
  QString channel = triple_name_list[0] + triple_name_list[1];

  qDebug() << "[isSet_to_edit_triple() channel1 -- " << channel;
  if ( dataSource. contains("DiskAUC:Absorbance") && channel.contains("S")  )
    channel = channel.replace("S","A");
  qDebug() << "[isSet_to_edit_triple()] channel2 -- " << channel;
  
  QString wvl = triple_name_list[2];    
  
  QMap<QString, QString>::iterator jj;
  for ( jj = triple_to_edit.begin(); jj != triple_to_edit.end(); ++jj )
    {
      if ( jj.key().contains( channel ) && jj.key().contains( opsys ) )
	{
	  QString wvl_set_edit = jj.value();

	  if ( wvl_set_edit == wvl )
	    {
	      qDebug() << "Triple " << triple_name << " for channel " <<  channel << " is set for EDIT.";
	      have_run = true;	      
	      break;
	    }
	}
    }
  
  return have_run;
}


// Create JSON to be put into AutoflowAnalysis table
QString US_Edit::compose_json( bool fm_stage )
{
  QString json;

  json = QString("{\"to_process\":[");

  if (job1run )
    json += QString("\"2DSA\",");
  if (job2run && fm_stage )
    json += QString("\"2DSA_FM\",");
  if ( job3run )
    {
      if ( job3auto ) 
	json += QString("\"FITMEN_AUTO\",");
      else
	json += QString("\"FITMEN\",");
    }
  if (job4run )
    json += QString("\"2DSA_IT\",");
  if (job5run )
    json += QString("\"2DSA_MC\",");
  if (job6run_pcsa )
    json += QString("\"PCSA\",");
  
  //remove last coma
  json.chop(1);

  json += QString("]}");
  
  return json;
}

// Set Autoflow record to ANALSYIS && set analysises ID(s)
void US_Edit::update_autoflow_record_atEditData( US_DB2* db,  QString& AnalysisIDsString )
{
   QString runID_numeric     = details_at_editing_local[ "runID" ];
   QString OptimaName        = details_at_editing_local[ "OptimaName" ];
   
   // // Check DB connection
   // US_Passwd pw;
   // QString masterpw = pw.getPasswd();
   // US_DB2* db = new US_DB2( masterpw );
   
   // if ( db->lastErrno() != US_DB2::OK )
   //   {
   //     QMessageBox::warning( this, tr( "Connection Problem" ),
   // 			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
   //     return;
   //   }

   QStringList qry;
   // qry << "update_autoflow_at_edit_data"
   //     << runID_numeric
   //     << AnalysisIDsString
   //     << OptimaName;
   qry << "update_autoflow_at_edit_data"
       << AnalysisIDsString
       << QString::number( autoflowID_passed );

   qDebug() << "Updating Autoflow tables with analysisIDs!!!";
   qDebug() << "query: " << qry;
   
   //db->query( qry );

   int status = db->statusQuery( qry );
   
   if ( status == US_DB2::NO_AUTOFLOW_RECORD )
     {
       QMessageBox::warning( this,
			     tr( "Autoflow Record Not Updated" ),
			     tr( "No autoflow record\n"
				 "associated with this experiment." ) );
       return;
     }
   
}


// Function to create a single autoflowAnalysis record: Pass some fields from autoflow table (set analysises IDs, filename etc.)
int US_Edit::create_autoflowAnalysis_record( US_DB2* db, QString& tripleName, QString& status_json ) 
{
  //create single record in autoflowAnalysis: return ID (auto-incremented), && update/push 

  int autoflowAnalysisID = 0;
   
  // // Check DB connection
  //  US_Passwd pw;
  //  QString masterpw = pw.getPasswd();
  //  US_DB2* db = new US_DB2( masterpw );
   
  //  if ( db->lastErrno() != US_DB2::OK )
  //    {
  //      QMessageBox::warning( this, tr( "Connection Problem" ),
  // 			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
  //      return autoflowAnalysisID;
  //    }


  //first, check max(requestID) in the autoflowAnalysisHistory table && set AUTO_INCREMENT in the autoflowAnalysis table to:
  //greater of:
  //- max(ID) autoflowAnalysisHistory
  //- current AUTO_INCREMENT
  QStringList qry;
  QString current_db = US_Settings::defaultDB().at(2);
  qry << "set_autoflowAnalysis_auto_increment" << current_db;
  int auto_incr = db->statusQuery( qry );
  qDebug() << "AutoflowAnalysis table: AUTO_INCREMENT: " << auto_incr;

  //Now add autoflowAnalysis record 
   QString status =  QString("Saving AutoflowAnalysis Profile for Triple %1").arg( tripleName );
   le_status->setText( status );
   qApp->processEvents();
   
   qry.clear();
   qry << "new_autoflow_analysis_record"
       << tripleName
       << filename_runID_auto
       << AProfileGUID
       << idInv_auto
       << status_json
       << QString::number( autoflowID_passed );
 
   qDebug() << "AutoflowAnalysis Record for triple: " << tripleName;
   qDebug() << "Query: " << qry;
   
   autoflowAnalysisID = db->functionQuery( qry );

   return autoflowAnalysisID;
   
}

// Function to create a single autoflowAnalysisStages record:
void US_Edit::create_autoflowAnalysisStages_record( US_DB2* db, int ID ) 
{
  QStringList qry;
  qry << "new_autoflow_analysis_stages_record"
      << QString::number( ID );
  
  qDebug() << "Query: " << qry;
  
  db->query( qry );
}


// Save edits for a triple
void US_Edit::write_triple_auto( int trx )
{
   triple_index = trx;
  
   index_data_auto( trx );
  
   edata          = outData[ data_index ];
   data           = *edata;

   QString editGUID;
   QString editID;
   QString rawGUID;
   QString triple;
   QString ss;
   is_spike_auto = false;

   int     odax   = trx;
   int     idax   = odax;
   
   // Base parameters for triple:
   QString triple_name = cb_triple->itemText( trx );

   le_status->setText( tr( "Saving edit profile for channel %1" ).arg( triple_name ) );
   qApp->processEvents();

   scanExcl_begin_ind = editProfile_scans_excl[ triple_name ][0].toInt();
   scanExcl_end_ind   = editProfile_scans_excl[ triple_name ][1].toInt();
   
   meniscus      = editProfile[ triple_name ][0].toDouble();
   range_left    = editProfile[ triple_name ][1].toDouble();
   range_right   = editProfile[ triple_name ][2].toDouble();
   plateau       = editProfile[ triple_name ][3].toDouble();
   baseline      = editProfile[ triple_name ][4].toDouble();

   if ( dataType == "IP" )
     {
       airGap_left      = editProfile[ triple_name ][7].toDouble();
       airGap_right     = editProfile[ triple_name ][8].toDouble(); 
     }
   
   if ( editProfile[ triple_name ].count() > 6 )
     {
       if ( editProfile[ triple_name ][6] == "spike_true")
	 is_spike_auto = true;
       else if (editProfile[ triple_name ][6] == "spike_false")
	 is_spike_auto = false;
     }

   // ALEXEY: the bottom value affected by rotor stretch - time consuming !!! Do we need it ? 
   // US_SimulationParameters simparams;
   // simparams.initFromData( dbP, data, false, runID, dataType );
   // bottom         = simparams.bottom;

   bottom = centerpieceParameters[ trx ][1].toDouble();  //Should be from centerpiece info from protocol 
   // End of base parameters
   

   if ( isMwl )
   {  // For MultiWavelength, data index needs to be recomputed
      int     wvx    = cb_lplot ->currentIndex();
      QString swavl  = expc_wvlns[ wvx ];
      triple         = cb_triple->itemText( trx ) + " / " + swavl;
      idax           = triples.indexOf( triple );
      odax           = index_data_auto( wvx );
   }

   // Do we need this ??
   /*
   if ( expIsEquil )
   {  // Equilibrium:  set baseline,plateau as flag that those are "done"
      int jsd     = sd_offs[ triple_index ];
      meniscus    = sData[ jsd ].meniscus;
      range_left  = sData[ jsd ].dataLeft;
      range_right = sData[ jsd ].dataRight;
      baseline    = range_left;
      plateau     = range_right;
   } 
   */

   // Check if complete
   if ( meniscus == 0.0 )
      ss = tr( "meniscus" );
   else if ( dataType == "IP" && ( airGap_left == 0.0 || airGap_right == 9.0 ) )
      ss = tr( "air gap" );
   else if ( range_left == 0.0 || range_right == 9.0 )
      ss = tr( "data range" );
   else if ( plateau == 0.0 )
      ss = tr( "plateau" );
   else if ( baseline == 0.0 )
      ss = tr( "baseline" );

   if ( ! ss.isEmpty() )
   {
      QMessageBox::information( this,
            tr( "Data missing" ),
            tr( "You must define the " ) + ss
            + tr( " before writing the edit profile." ) );
      return;
   }

   // Check if meniscus and plateau are consistent with the data range
   if ( meniscus >= range_left )
   {
      QMessageBox::critical( this,
            tr( "Meniscus/Data_Left Inconsistent" ),
            tr( "The specified Meniscus (%1) extends into the "
                "Data Range (%2 to %3). Correct the Meniscus/Data_Left" )
            .arg( meniscus ).arg( range_left ).arg( range_right ) );
      return;
   }

   if ( plateau >= range_right )
   {
      QMessageBox::critical( this,
            tr( "Plateau/Data_Right Inconsistent" ),
            tr( "The specified Plateau (%1) is outside of the "
                "Data Range (%2 to %3). Correct the Meniscus/Data_Right" )
            .arg( plateau ).arg( range_left ).arg( range_right ) );
      return;
   }
   //END check complete

   // Ask for editLabel if not yet defined
   editGUIDs[ idax ].clear();
   QString now  =  QDateTime::currentDateTime()
                   .toUTC().toString( "yyMMddhhmm" );

   editLabel = now + QString::number( trx );

   //Filename
   QString filename = files[ idax ];
   QString rpart    = filename.section( ".",  0, -6 );
   QString tpart    = filename.section( ".", -5, -2 );
   filename         = rpart + "." + editLabel + "." + tpart + ".xml";

   if ( expType.isEmpty()  || expType.compare( "other", Qt::CaseInsensitive ) == 0 )
     expType = "Velocity";
   
   editGUID         = editGUIDs[ idax ];

   if ( editGUID.isEmpty() )
   {
      editGUID = US_Util::new_guid();
      editGUIDs.replace( idax, editGUID );
   }

   rawGUID          = US_Util::uuid_unparse( (unsigned char*)data.rawGUID );
   triple           = triples.at( idax );

   // Output the edit XML file
   int wrstat       = write_xml_file( filename, triple, editGUID, rawGUID );

   if ( wrstat != 0 )
     return;
   else
     editFnames[ idax ] = filename;

   if ( disk_controls->db() )
     {
       if ( dbP == NULL )
	 {
	   US_Passwd pw;
	   dbP          = new US_DB2( pw.getPasswd() );
	   if ( dbP == NULL  ||  dbP->lastErrno() != US_DB2::OK )
	     {
	       QMessageBox::warning( this, tr( "Connection Problem" ),
				     tr( "Could not connect to database \n" ) + dbP->lastError() );
	       return;
	     }
	 }
       
       editID           = editIDs[ idax ];
       
       // Output the edit database record
       wrstat     = write_edit_db( dbP, filename, editGUID, editID, rawGUID );
       
       if ( wrstat != 0 )
         return;
     }

   // Output the Data Set Information report
   QString rtext;
   tpart            = filename.section( ".", -4, -2 ).replace( ".", "" );
   QString rptfname = "dsinfo." + tpart + ".dataset_info.html";
   QString rptfpath = QString( workingDir ).replace( "/results", "/reports" ) + rptfname;
   int     idEdit   = editID.toInt();
   
   create_report_auto( rtext, trx );
   
   save_report_auto( rtext, rptfpath, idEdit, trx );


   //Disconnect cb_triple if runType_combined_IP_RI == true;
   if ( runType_combined_IP_RI )
     cb_triple->disconnect();
  
}


// Save edits for a triple
void US_Edit::write_triple( void )
{
   QString editGUID;
   QString editID;
   QString rawGUID;
   QString triple;
   QString ss;

   meniscus       = le_meniscus->text().toDouble();
   baseline       = data.xvalues[ data.xindex( range_left ) + 5 ];
   int     odax   = cb_triple->currentIndex();
   int     idax   = odax;
double bl= data.xindex( range_left ) + 5;
DbgLv(1) << "BL: CC : baseline bl" << baseline << bl;

   if ( isMwl )
   {  // For MultiWavelength, data index needs to be recomputed
      int     wvx    = cb_lplot ->currentIndex();
      QString swavl  = expc_wvlns[ wvx ];
      triple         = cb_triple->currentText() + " / " + swavl;
      idax           = triples.indexOf( triple );
      odax           = index_data( wvx );
   }

   if ( expIsEquil )
   {  // Equilibrium:  set baseline,plateau as flag that those are "done"
      int jsd     = sd_offs[ triple_index ];
      meniscus    = sData[ jsd ].meniscus;
      range_left  = sData[ jsd ].dataLeft;
      range_right = sData[ jsd ].dataRight;
      baseline    = range_left;
      plateau     = range_right;
   }

   // Check if complete
   if ( meniscus == 0.0 )
      ss = tr( "meniscus" );
   else if ( dataType == "IP" && ( airGap_left == 0.0 || airGap_right == 9.0 ) )
      ss = tr( "air gap" );
   else if ( range_left == 0.0 || range_right == 9.0 )
      ss = tr( "data range" );
   else if ( plateau == 0.0 )
      ss = tr( "plateau" );
   else if ( baseline == 0.0 )
      ss = tr( "baseline" );

   if ( ! ss.isEmpty() )
   {
      QMessageBox::information( this,
            tr( "Data missing" ),
            tr( "You must define the " ) + ss
            + tr( " before writing the edit profile." ) );
      return;
   }

   // Check if meniscus and plateau are consistent with the data range
   if ( meniscus >= range_left )
   {
      QMessageBox::critical( this,
            tr( "Meniscus/Data_Left Inconsistent" ),
            tr( "The specified Meniscus (%1) extends into the "
                "Data Range (%2 to %3). Correct the Meniscus/Data_Left" )
            .arg( meniscus ).arg( range_left ).arg( range_right ) );
      return;
   }

   if ( plateau >= range_right )
   {
      QMessageBox::critical( this,
            tr( "Plateau/Data_Right Inconsistent" ),
            tr( "The specified Plateau (%1) is outside of the "
                "Data Range (%2 to %3). Correct the Meniscus/Data_Right" )
            .arg( plateau ).arg( range_left ).arg( range_right ) );
      return;
   }

   QString sufx = "";

   // Ask for editLabel if not yet defined
   while ( editLabel.isEmpty() )
   {
      editGUIDs[ idax ].clear();
      QString now  =  QDateTime::currentDateTime()
                      .toUTC().toString( "yyMMddhhmm" );

      bool ok;
      QString msg = tr( "The base Edit Label for this edit session is <b>" )
         + now + "</b> .<br/>"
         + tr( "You may add an optional suffix to further distinquish "
               "the Edit Label.<br/>"
               "Use alphanumeric characters or hyphens (no spaces).<br/>"
               "Enter 0 to 10 suffix characters." );
      sufx   = QInputDialog::getText( this, 
         tr( "Create a unique session Edit Label" ),
         msg,
         QLineEdit::Normal,
         sufx,
         &ok );
      
      if ( ! ok ) return;

DbgLv(1) << "EDT:WrTripl: orig sufx" << sufx;
      QString isufx = sufx;
      sufx.replace( QRegExp( "[^\\w\\d\\-]" ), "-" );
      sufx.replace( '_', '-' );
DbgLv(1) << "EDT:WrTripl:  rmvd sufx" << sufx;
      if ( sufx != isufx )
      {
         QMessageBox::critical( this,
            tr( "Character content error" ),
            tr( "You entered non-alphanumeric/non-hypen characters\n"
                "for the Edit Label suffix.\n"
                "Re-enter, limiting characters to alphanumeric or hyphen ('-')." )
            .arg( sufx.length() ) );
         editLabel.clear();
      }
      else
      {
         editLabel = now + sufx;
      }

      if ( editLabel.length() > 20 )
      {
         QMessageBox::critical( this,
            tr( "Text length error" ),
            tr( "You entered %1 characters for the Edit Label suffix.\n"
                "Re-enter, limiting length to 10 characters." )
            .arg( sufx.length() ) );
         editLabel.clear();
         sufx = sufx.left( 10 );
      }
   }

   // Determine file name
   // workingDir + runID + editLabel + data type + cell + channel + wavelength 
   //            + ".xml"

   QString filename = files[ idax ];
DbgLv(1) << "EDT:WrTripl: tripindex" << triple_index << "idax" << idax
 << "filename" << filename;
   QString rpart    = filename.section( ".",  0, -6 );
   QString tpart    = filename.section( ".", -5, -2 );
   filename         = rpart + "." + editLabel + "." + tpart + ".xml";
   QString wvpart   = "";
DbgLv(1) << "EDT:WrTripl:  filename" << filename;

   if ( expType.isEmpty()  ||
        expType.compare( "other", Qt::CaseInsensitive ) == 0 )
      expType = "Velocity";

   editGUID         = editGUIDs[ idax ];

   if ( editGUID.isEmpty() )
   {
      editGUID = US_Util::new_guid();
      editGUIDs.replace( idax, editGUID );
   }

   rawGUID          = US_Util::uuid_unparse( (unsigned char*)data.rawGUID );
   triple           = triples.at( idax );
DbgLv(1) << "EDT:WrTripl:   triple" << triple;

   // Output the edit XML file
   int wrstat       = write_xml_file( filename, triple, editGUID, rawGUID );

   if ( wrstat != 0 )
      return;
   else
      editFnames[ idax ] = filename;

   if ( disk_controls->db() )
   {
      if ( dbP == NULL )
      {
         US_Passwd pw;
         dbP          = new US_DB2( pw.getPasswd() );
         if ( dbP == NULL  ||  dbP->lastErrno() != US_DB2::OK )
         {
            QMessageBox::warning( this, tr( "Connection Problem" ),
              tr( "Could not connect to database \n" ) + dbP->lastError() );
            return;
         }
      }

      editID           = editIDs[ idax ];

      // Output the edit database record
      wrstat     = write_edit_db( dbP, filename, editGUID, editID, rawGUID );

      if ( wrstat != 0 )
         return;
   }

   // Output the Data Set Information report
   QString rtext;
   tpart            = filename.section( ".", -4, -2 ).replace( ".", "" );
   QString rptfname = "dsinfo." + tpart + ".dataset_info.html";
   QString rptfpath = QString( workingDir ).replace( "/results", "/reports" )
                      + rptfname;
   int     idEdit   = editID.toInt();

   create_report( rtext );

   save_report( rtext, rptfpath, idEdit );
}

// Apply a prior edit profile for Velocity and like data
void US_Edit::apply_prior( void )
{
   QString filename;
   QString filepath;
   int     index1;
   US_DB2* dbP    = NULL;
DbgLv(1) << "AppPri: IN  dkdb" << disk_controls->db();

   if ( disk_controls->db() )
   {
      US_Passwd pw;
      dbP            = new US_DB2( pw.getPasswd() );

      if ( dbP->lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this, tr( "Connection Problem" ),
           tr( "Could not connect to database: \n" ) + dbP->lastError() );
         return;
      }

      QStringList q( "get_rawDataID_from_GUID" );
     
      q << US_Util::uuid_unparse( (uchar*)data.rawGUID );
      dbP->query( q );

      // Error check    
      if ( dbP->lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this,
           tr( "AUC Data is not in DB" ),
           tr( "Cannot find the raw data in the database.\n" ) );

         return;
      }
      
      dbP->next();
      QString rawDataID = dbP->value( 0 ).toString();

      q.clear();
      q << "get_editedDataIDs" << rawDataID;

      dbP->query( q );


      QStringList editDataIDs;
      QStringList filenames;

      while ( dbP->next() )
      {
         editDataIDs << dbP->value( 0 ).toString();
         filenames   << dbP->value( 2 ).toString();
      }

      if ( editDataIDs.size() == 0 )
      {
         QMessageBox::warning( this,
           tr( "Edit data is not in DB" ),
           tr( "Cannot find any edit records in the database.\n" ) );

         return;
      }

      int index;
      US_GetEdit dialog( index, filenames );
      if ( dialog.exec() == QDialog::Rejected ) return;

      if ( index >= 0 ) 
      {
         filepath   = workingDir + filenames[ index ];
         int dataID = editDataIDs[ index ].toInt();
         dbP->readBlobFromDB( filepath, "download_editData", dataID );
      }
   }
   else
   {
      QString filter = files[ cb_triple->currentIndex() ];
      index1 = filter.indexOf( '.' ) + 1;

      filter = "*" + filter.mid( index1 );
      filter.replace( QRegExp( "auc$" ), "xml" );
      filter = tr( "Edits(" ) + filter + tr( ");;All XML (*.xml)" );
      
      // Ask for edit file
      filepath = QFileDialog::getOpenFileName( this, 
            tr( "Select a saved edit file" ),
            workingDir, filter );
   }
DbgLv(1) << "AppPri: fpath" << filepath;

   if ( filepath.isEmpty() ) return; 

   // Get multiple edits if they exist and user so chooses
   QStringList editfiles;
   filepath         = filepath.replace( "\\", "/"   );
   filename         = filepath.section( "/", -1, -1 );
   QString triple   = filename.section( ".", -4, -2 )
                              .replace( ".", " / "  );
   int     idax     = triples.indexOf( triple );

   int     nledits  = like_edit_files( filename, editfiles, dbP );
DbgLv(1) << "AppPri: nledits" << nledits << editfiles.count();

   if ( nledits > 1 )
   {
      QPushButton* pb_defb; 
      QPushButton* pb_selo;
      QPushButton* pb_allw;
      QString edtLbl = editfiles[ 0 ].section( ".", -6, -6 );
      int swavl      = editfiles[           0 ].section( ".", -2, -2 ).toInt();
      int ewavl      = editfiles[ nledits - 1 ].section( ".", -2, -2 ).toInt();

      QMessageBox mbox;
      QString msg  = tr( "%1 wavelengths (%2 to %3) have the same<br/>"
                         "Edit Label of \"%4\" as the selected file.<br/>"
                         "<br/>"
                         "Specify whether you wish to apply edits only<br/>"
                         "to the selected file (<b>Selected Only</b>)<br/>"
                         "or to apply them to all wavelengths of the<br/>"
                         "current cell/channel (<b>All Wavelengths</b>)." )
                     .arg( nledits ).arg( swavl ).arg( ewavl ).arg( edtLbl );
      mbox.setIcon      ( QMessageBox::Question );
      mbox.setTextFormat( Qt::RichText );
      mbox.setText      ( msg );
      pb_selo        = mbox.addButton( tr( "Selected Only"   ),
                                       QMessageBox::RejectRole );
      pb_allw        = mbox.addButton( tr( "All Wavelengths" ),
                                       QMessageBox::AcceptRole );
      pb_defb        = nledits > 2 ? pb_allw : pb_selo;
      mbox.setDefaultButton( pb_defb );
      mbox.exec();

      if ( mbox.clickedButton() == pb_selo )
      {  // Only apply for the selected file
DbgLv(1) << "AppPri: SelOnly button clicked";
         nledits        = 1;
         editfiles.clear();
         editfiles << filename;

         editFnames[ idax ] = filename;
      }

      else
      {  // Apply to all like-labeled wavelengths in the channel
DbgLv(1) << "AppPri: AllWavl button clicked";
         for ( int ii = 0; ii < nledits; ii++ )
         {  // Save selected edit file name; use "same" for others in channel
            QString edfile = editfiles[ ii ];
            QString triple = edfile.section( ".", -4, -2 )
                                   .replace( ".", " / " );
            int     jdax   = triples.indexOf( triple );

            if ( edfile != filename )
            {  // The selected file
               editFnames[ jdax ] = filename;
            }

            else
            {  // Non-selected file
               editFnames[ jdax ] = QString( "same" );
            }
         }
      }
   }

   else
   {  // Save edit file name for the single triple
      editFnames[ idax ] = filename;
   }

   // Reset data from input data
   data           = allData[ idax ];

   // Read the edits
   US_DataIO::EditValues parameters;

   int     result = US_DataIO::readEdits( filepath, parameters );

   if ( result != US_DataIO::OK )
   {
      QMessageBox::warning( this,
         tr( "XML Error" ),
         tr( "An error occurred when reading edit file\n\n" ) 
         +  US_DataIO::errorString( result ) );
      return;
   }

   QString uuid   = US_Util::uuid_unparse( (unsigned char*)data.rawGUID );

   if ( parameters.dataGUID != uuid )
   {
      QMessageBox::warning( this,
         tr( "Data Error" ),
         tr( "The edit file was not created using the current data" ) );
DbgLv(1) << "parsGUID rawGUID" << parameters.dataGUID << uuid;
      return;
   }
   
   // Apply the edits with specified parameters
   apply_edits( parameters );

   pb_report  ->setEnabled( true  );
   pb_undo    ->setEnabled( true  );
   pb_write   ->setEnabled( true  );
   ck_writemwl->setEnabled( isMwl );

   changes_made      = false;
   //editLabel         = filename.section( ".", -6, -6 );
   editLabel.clear();
   plot_range();
}

// Apply prior edits to an Equilibrium set
void US_Edit::prior_equil( void )
{
   int     cndxt     = cb_triple->currentIndex();
   int     cndxs     = cb_rpms  ->currentIndex();
   int     index1;
   QString     filename;
   QStringList cefnames;
   data    = *outData[ 0 ];

   if ( disk_controls->db() )
   {  // Get prior equilibrium edits from DB
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this, tr( "Connection Problem" ),
           tr( "Could not connect to database \n" ) + db.lastError() );
         return;
      }

      QStringList q( "get_rawDataID_from_GUID" );
      
      q << US_Util::uuid_unparse( (uchar*)data.rawGUID );

      db.query( q );

      // Error check    
      if ( db.lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this,
           tr( "AUC Data is not in DB" ),
           tr( "Cannot find the raw data in the database.\n" ) );

         return;
      }
      
      db.next();
      QString rawDataID = db.value( 0 ).toString();

      q.clear();
      q << "get_editedDataIDs" << rawDataID;

      db.query( q );


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

         return;
      }

      int index;
      US_GetEdit dialog( index, filenames );
      if ( dialog.exec() == QDialog::Rejected ) return;

      if ( index < 0 )
         return;

      filename          = filenames[ index ];
      int     dataID    = editDataIDs[ index ].toInt();

      editLabel         = filename.section( ".", -6, -6 );
      filename          = workingDir + filename;
      db.readBlobFromDB( filename, "download_editData", dataID );

      cefnames << filename;   // save the first file name

      // Loop to get files with same Edit Label from other triples
      for ( int ii = 1; ii < outData.size(); ii++ )
      {
         data    = *outData[ ii ];
         q.clear();
         q << "get_rawDataID_from_GUID" 
           << US_Util::uuid_unparse( (uchar*)data.rawGUID );
         db.query( q );
         db.next();
         rawDataID = db.value( 0 ).toString();

         q.clear();
         q << "get_editedDataIDs" << rawDataID;
         db.query( q );
         filename.clear();
         bool found = false;

         while ( db.next() )
         {
            dataID      = db.value( 0 ).toString().toInt();
            filename    = db.value( 2 ).toString();
            QString elb = filename.section( ".", -6, -6 );
            
            if ( elb == editLabel )
            {
               found = true;
               filename = workingDir + filename;
               db.readBlobFromDB( filename, "download_editData", dataID );
               cefnames << filename;
               break;
            }
         }

         if ( ! found )
            cefnames << "";
      }
   }

   else
   {  // Get prior equilibrium edits from Local Disk
      QString filter = files[ cb_triple->currentIndex() ];
      index1 = filter.indexOf( '.' ) + 1;

      filter.insert( index1, "*." );
      filter.replace( QRegExp( "auc$" ), "xml" );
      
      // Ask for edit file
      filename = QFileDialog::getOpenFileName( this, 
            tr( "Select a saved edit file" ),
            workingDir, filter );

      if ( filename.isEmpty() ) return; 

      filename         = filename.replace( "\\", "/" );
      editLabel        = filename.section( "/", -1, -1 ).section( ".", -6, -6 );
      QString runID    = filename.section( "/", -1, -1 ).section( ".",  0, -7 );

      for ( int ii = 0; ii < files.size(); ii++ )
      {
         filename         = files[ ii ];
         filename         = runID + "." + editLabel + "."
                            + filename.section( ".", -5, -2 ) + ".xml";
         filename         = workingDir + filename;

         if ( QFile( filename ).exists() )
            cefnames << filename;

         else
            cefnames << "";
      }
   }

   for ( int ii = 0; ii < cefnames.size(); ii++ )
   {  // Read and apply edits from edit files
      data     = *outData[ ii ];
      filename = cefnames[ ii ];

      // Read the edits
      US_DataIO::EditValues parameters;

      int result = US_DataIO::readEdits( filename, parameters );

      if ( result != US_DataIO::OK )
      {
         QMessageBox::warning( this,
               tr( "XML Error" ),
               tr( "An error occurred when reading edit file\n\n" ) 
               +  US_DataIO::errorString( result ) );
         continue;
      }

      QString uuid = US_Util::uuid_unparse( (unsigned char*)data.rawGUID );

      if ( parameters.dataGUID != uuid )
      {
         QMessageBox::warning( this,
               tr( "Data Error" ),
               tr( "The edit file was not created using the current data" ) );
         continue;
      }
   
      // Apply the edits
      QString wkstr;

      meniscus    = parameters.meniscus;
      range_left  = parameters.rangeLeft;
      range_right = parameters.rangeRight;
      plateau     = parameters.plateau;
      baseline    = parameters.baseline;

      if ( parameters.speedData.size() > 0 )
      {
         meniscus    = parameters.speedData[ 0 ].meniscus;
         range_left  = parameters.speedData[ 0 ].dataLeft;
         range_right = parameters.speedData[ 0 ].dataRight;
         baseline    = range_left;
         plateau     = range_right;

         int jsd     = sd_offs[ ii ];

         for ( int jj = 0; jj < sd_knts[ ii ]; jj++ )
            sData[ jsd++ ] = parameters.speedData[ jj ];
      }

      le_meniscus->setText( wkstr.sprintf( "%.3f", meniscus ) );
      pb_meniscus->setIcon( check );
      pb_meniscus->setEnabled( true );

      airGap_left  = parameters.airGapLeft;
      airGap_right = parameters.airGapRight;

      if ( dataType == "IP" )
      {
         US_DataIO::adjust_interference( data, parameters );
         US_DataIO::calc_integral      ( data, parameters );
         le_airGap->setText( wkstr.sprintf( "%.3f - %.3f", 
                  airGap_left, airGap_right ) ); 
         pb_airGap->setIcon( check );
         pb_airGap->setEnabled( true );
      }

      le_dataRange->setText( wkstr.sprintf( "%.3f - %.3f",
              range_left, range_right ) );
      le_dataStart->setText( QString::number( range_left,  'f', 8 ) );
      le_dataEnd  ->setText( QString::number( range_right, 'f', 8 ) );
      pb_dataEnd  ->setIcon( check );
      pb_dataEnd  ->setEnabled( true );
      plateau      = range_right - _PLATEAU_OFFSET_;
      le_plateau  ->setText( QString::number( plateau,     'f', 8 ) );
   
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
      
         Edits e;
         e.scan = scan;
         e.changes << QPointF( index1, value );
     
         changed_points << e;
      
         data.scanData[ scan ].rvalues[ index1 ] = value;
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
         US_RiNoise::calc_residuals( data, includes, range_left, range_right, 
               noise_order, residuals );
      
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
   }

   step        = FINISHED;
   set_pbColors( NULL );

   pb_report  ->setEnabled( true );
   pb_undo    ->setEnabled( true );
   pb_write   ->setEnabled( true );
   ck_writemwl->setEnabled( isMwl );

   cndxt       = ( cndxt < 0 ) ? 0 : cndxt;
   cndxs       = ( cndxs < 0 ) ? 0 : cndxs;
   cb_triple->setCurrentIndex( cndxt );
   cb_rpms  ->setCurrentIndex( cndxs );

   //changes_made= false;
   //plot_range();

   pb_reviewep->setEnabled( true );
   pb_nexteqtr->setEnabled( true );

   all_edits    = all_edits_done();
   pb_report  ->setEnabled( all_edits );
   pb_write   ->setEnabled( all_edits );
   ck_writemwl->setEnabled( all_edits && isMwl );
   changes_made = all_edits;

   review_edits();
}

// Initialize edit review for first triple
void US_Edit::review_edits( void )
{
   cb_triple->disconnect();
   cb_triple->setCurrentIndex( cb_triple->count() - 1 );

   le_meniscus ->setText( "" );
   le_dataStart->setText( "" );
   le_dataEnd  ->setText( "" );
   pb_meniscus ->setIcon( QIcon() );

   step    = FINISHED;
   next_triple();
}

// Advance to next triple and plot edited curves
void US_Edit::next_triple_auto( void )
{
   pb_priorChan ->setEnabled( true );
   
   int row = cb_triple->currentIndex() + 1;
   //row     = ( row < cb_triple->count() ) ? row : 0;

   if ( (row + 1 ) <= cb_triple->count() )
     {
       cb_triple->disconnect();
       cb_triple->setCurrentIndex( row );
       connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
		SLOT  ( new_triple_auto    ( int ) ) );
       
       if ( le_edtrsp->isVisible() )
	 {
	   QString trsp = cb_triple->currentText() + " : " + trip_rpms[ 0 ];
	   le_edtrsp->setText( trsp );
	   cb_rpms  ->setCurrentIndex( 0 );
	 }
       
       int dax = index_data();
       //int dax = mwl_data.data_index( iwavl_edit_ref[ triple_index ], triple_index );
       
       data    = *outData[ dax ];
       
       new_triple_auto( dax );
       
       if ( (row + 1 ) == cb_triple->count() )
         pb_nextChan ->setEnabled( false );
       
       qDebug() << "NEXT Triple: row " << row << ", cb_triple->count() " << cb_triple->count() ;
   }
   else
     pb_nextChan->setEnabled( row < cb_triple->count() );
}

// Advance to next triple and plot edited curves
void US_Edit::prior_triple_auto( void )
{
  pb_nextChan ->setEnabled( true );
  
  int row = cb_triple->currentIndex() - 1;
  //row     = ( row < cb_triple->count() ) ? row : 0;

  if ( row  >= 0 )
    {
      cb_triple->disconnect();
      cb_triple->setCurrentIndex( row );
      connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
	       SLOT  ( new_triple_auto    ( int ) ) );
      
      // if ( le_edtrsp->isVisible() )
      // 	 {
      // 	   QString trsp = cb_triple->currentText() + " : " + trip_rpms[ 0 ];
      // 	   le_edtrsp->setText( trsp );
      // 	   cb_rpms  ->setCurrentIndex( 0 );
      // 	 }
      
      
      int dax = index_data();
      data    = *outData[ dax ];
      
      new_triple_auto( dax );

      if ( row == 0 )
	pb_priorChan ->setEnabled( false );

      
      pb_nextChan ->setEnabled( true );
      qDebug() << "PRIOR Triple: row " << row << ", cb_triple->count() " << cb_triple->count() ;
    }
  else
    pb_priorChan ->setEnabled( false );
  
}


// Advance to next triple and plot edited curves
void US_Edit::next_triple( void )
{
   int row = cb_triple->currentIndex() + 1;
   row     = ( row < cb_triple->count() ) ? row : 0;

   cb_triple->disconnect();
   cb_triple->setCurrentIndex( row );
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ), 
                       SLOT  ( new_triple         ( int ) ) );

   if ( le_edtrsp->isVisible() )
   {
      QString trsp = cb_triple->currentText() + " : " + trip_rpms[ 0 ];
      le_edtrsp->setText( trsp );
      cb_rpms  ->setCurrentIndex( 0 );
   }

   int dax = index_data();
   data    = *outData[ dax ];

   new_triple( dax );

   pb_nextChan->setEnabled( row < cb_triple->count() );
}


// Evaluate whether all edits are complete
bool US_Edit::all_edits_done( void )
{
   bool all_ed_done = false;

   if ( expIsEquil )
   {
      total_edits  = 0;
      total_speeds = 0;

      for ( int jd = 0; jd < outData.size(); jd++ )
      {  // Examine each data set to evaluate whether edits complete
         int jsd = sd_offs[ jd ];
         int ksd = jsd + sd_knts[ jd ];
         QList< double > drpms;
         US_DataIO::RawData* rawdat = outData[ jd ];

         // Count edits done on this data set
         for ( int js = jsd; js < ksd; js++ )
         {
            if ( sData[ js ].meniscus > 0.0 )
               total_edits++;
         }

         // Count speeds present in this data set
         for ( int js = 0; js < rawdat->scanData.size(); js++ )
         {
            double  drpm = rawdat->scanData[ js ].rpm;

            if ( ! drpms.contains( drpm ) )
               drpms << drpm;
         }

         total_speeds += drpms.size();
      }

      // Set flag:  are all edits complete?
      all_ed_done = ( total_edits == total_speeds );
   }

   else
   {
      all_ed_done = ( range_left  != 0  &&
                      range_right != 0  &&
                      plateau     != 0  &&
                      baseline    != 0 );
   }

DbgLv(1) << "all_ed_done" << all_ed_done;
   return all_ed_done;
}

// Private slot to update disk/db control when dialog changes it
void US_Edit::update_disk_db( bool isDB )
{
   if ( isDB )
      disk_controls->set_db();
   else
      disk_controls->set_disk();
}

// Private slot to show progress text for load-auc
void US_Edit::progress_load( QString progress )
{
   le_info->setText( progress );
}

// Show or hide MWL Controls
void US_Edit::show_mwl_controls( bool show )
{
  if ( !us_edit_auto_mode  )
    {
      if ( !us_edit_auto_mode_manual )
	{
	  lb_gaps    ->setVisible( !show );
	  ct_gaps    ->setVisible( !show );
	}
      
      le_lxrng   ->setVisible( show );
      lb_mwlctl  ->setVisible( show );
      lb_ldelta  ->setVisible( show );
      ct_ldelta  ->setVisible( show );
      le_ltrng   ->setVisible( show );
      lb_lstart  ->setVisible( show );
      cb_lstart  ->setVisible( show );
      lb_lend    ->setVisible( show );
      cb_lend    ->setVisible( show );
      lb_lplot   ->setVisible( show );
      cb_lplot   ->setVisible( show );
      pb_larrow  ->setVisible( show );
      pb_rarrow  ->setVisible( show );
      pb_custom  ->setVisible( show );
      pb_incall  ->setVisible( show );
      
      lo_lrange  ->itemAtPosition( 0, 0 )->widget()->setVisible( show );
      lo_lrange  ->itemAtPosition( 0, 1 )->widget()->setVisible( show );
      lo_custom  ->itemAtPosition( 0, 0 )->widget()->setVisible( show );
      lo_custom  ->itemAtPosition( 0, 1 )->widget()->setVisible( show );
      lo_radius  ->itemAtPosition( 0, 0 )->widget()->setVisible( show );
      lo_radius  ->itemAtPosition( 0, 1 )->widget()->setVisible( show );
      lo_waveln  ->itemAtPosition( 0, 0 )->widget()->setVisible( show );
      lo_waveln  ->itemAtPosition( 0, 1 )->widget()->setVisible( show );
      lo_writemwl->itemAtPosition( 0, 0 )->widget()->setVisible( show );
      lo_writemwl->itemAtPosition( 0, 1 )->widget()->setVisible( show );

    }
  
   adjustSize();
}

// Connect or disconnect MWL Controls
void US_Edit::connect_mwl_ctrls( bool conn )
{
   if ( conn )
   {
      connect( rb_lrange, SIGNAL( toggled            ( bool   ) ),
               this,      SLOT  ( lselect_range_on   ( bool   ) ) );
      connect( rb_custom, SIGNAL( toggled            ( bool   ) ),
               this,      SLOT  ( lselect_custom_on  ( bool   ) ) );
      connect( ct_ldelta, SIGNAL( valueChanged       ( double ) ),
               this,      SLOT  ( ldelta_value       ( double ) ) );
      connect( cb_lstart, SIGNAL( currentIndexChanged( int    ) ),
               this,      SLOT  ( lambda_start_value ( int    ) ) );
      connect( cb_lend,   SIGNAL( currentIndexChanged( int    ) ),
               this,      SLOT  ( lambda_end_value   ( int    ) ) );
      connect( rb_radius, SIGNAL( toggled            ( bool   ) ),
               this,      SLOT  ( xaxis_radius_on    ( bool   ) ) );
      connect( rb_waveln, SIGNAL( toggled            ( bool   ) ),
               this,      SLOT  ( xaxis_waveln_on    ( bool   ) ) );
      connect( pb_custom, SIGNAL( clicked            (        ) ),
               this,      SLOT  ( lambda_custom_list (        ) ) );
      connect( pb_incall, SIGNAL( clicked            (        ) ),
               this,      SLOT  ( lambda_include_all (        ) ) );
      connect( cb_lplot,  SIGNAL( currentIndexChanged( int    ) ),
               this,      SLOT  ( lambda_plot_value  ( int    ) ) );
      connect( pb_larrow, SIGNAL( clicked            (        ) ),
               this,      SLOT  ( lambda_plot_prev   (        ) ) );
      connect( pb_rarrow, SIGNAL( clicked            (        ) ),
               this,      SLOT  ( lambda_plot_next   (        ) ) );
   }

   else
   {
      rb_lrange->disconnect();
      rb_custom->disconnect();
      ct_ldelta->disconnect();
      cb_lstart->disconnect();
      cb_lend  ->disconnect();
      rb_radius->disconnect();
      rb_waveln->disconnect();
      pb_custom->disconnect();
      pb_incall->disconnect();
      cb_lplot ->disconnect();
      pb_larrow->disconnect();
      pb_rarrow->disconnect();
   }
}

// Lambda selection has been changed to Range or Custom
void US_Edit::lselect_range_on( bool checked )
{
DbgLv(1) << "lselect range checked" << checked;
   if ( checked )
   {
      lambdas_by_cell();
      connect_mwl_ctrls( false );
      ct_ldelta->setValue( 1 );
      cb_lstart->setCurrentIndex( 0 );
      cb_lend  ->setCurrentIndex( nwaveln - 1 );
      connect_mwl_ctrls( true );

      reset_plot_lambdas();
   }

   ct_ldelta    ->setEnabled( checked );
   cb_lstart    ->setEnabled( checked );
   cb_lend      ->setEnabled( checked );
   pb_custom    ->setEnabled( !checked );
   lsel_range = checked;
}

// Lambda selection has been changed to Range or Custom
void US_Edit::lselect_custom_on( bool checked )
{
DbgLv(1) << "lselect custom checked" << checked;
   if ( checked )
   {
      lambdas_by_cell();
      connect_mwl_ctrls( false );
      ct_ldelta->setValue( 1 );
      cb_lstart->setCurrentIndex( 0 );
      cb_lend  ->setCurrentIndex( nwaveln - 1 );
      connect_mwl_ctrls( true );

      reset_plot_lambdas();
   }

   ct_ldelta    ->setEnabled( !checked );
   cb_lstart    ->setEnabled( !checked );
   cb_lend      ->setEnabled( !checked );
   pb_custom    ->setEnabled( checked );
   lsel_range = !checked;
}

// Lambda Delta has changed
void US_Edit::ldelta_value( double value )
{
DbgLv(1) << "ldelta_value  value" << value;
   dlambda     = (int)value;

   reset_plot_lambdas();
}

// Lambda Start has changed
void US_Edit::lambda_start_value( int value )
{
   int new_slambda = cb_lstart->itemText( value ).toInt();
   if ( new_slambda == slambda ) // check if new start is same as current start -> skip every update
   {
      return;
   }
   else if ( new_slambda > elambda ) // check if new start is greater than end
   {
      // if so, set start to end
      slambda = elambda;
      elambda = new_slambda;
      cb_lstart->setCurrentIndex( cb_lstart->findText( QString::number( slambda ) ) );
      cb_lend  ->setCurrentIndex( cb_lend  ->findText( QString::number( elambda ) ) );
   }
   else
   {
      slambda     = cb_lstart->itemText( value ).toInt();
   }
   DbgLv(1) << "lambda_start_value  value" << value << slambda;

   reset_plot_lambdas();
}

// Lambda End has changed
void US_Edit::lambda_end_value( int value )
{
   int new_elambda = cb_lend->itemText( value ).toInt();
   if ( new_elambda == elambda ) // check if new end is same as current end -> skip every update
   {
      return;
   }
   else if ( new_elambda < slambda ) // check if new end is less than start
   {
      // if so, set end to start
      elambda = slambda;
      slambda = new_elambda;
      cb_lstart->setCurrentIndex( cb_lstart->findText( QString::number( slambda ) ) );
      cb_lend  ->setCurrentIndex( cb_lend  ->findText( QString::number( elambda ) ) );
   }
   else
   {
      elambda     = cb_lend  ->itemText( value ).toInt();
   }
   DbgLv(1) << "lambda_end_value  value" << value << elambda;

   reset_plot_lambdas();
}

// Adjust the plot wavelengths list, after a lambda range change
void US_Edit::reset_plot_lambdas()
{
   dlambda        = (int)ct_ldelta->value();
   slambda        = cb_lstart->currentText().toInt();
   elambda        = cb_lend  ->currentText().toInt();
   int     plambd = cb_lplot ->currentText().toInt();
   int     strtx  = rawi_wvlns.indexOf( slambda );
   int     endx   = rawi_wvlns.indexOf( elambda ) + 1;
   int     plotx  = cb_lplot ->currentIndex();
DbgLv(1) << "rpl: dl sl el px" << dlambda << slambda << elambda << plotx
 << "sx ex nr" << strtx << endx << rawc_wvlns.size();
DbgLv(1) << "rpl: trx" << triple_index << cb_triple->currentIndex();
DbgLv(1) << "rpl: rcw 0 1 m n" << rawc_wvlns[0] << rawc_wvlns[1]
 << rawc_wvlns[nwaveln-2] << rawc_wvlns[nwaveln-1];
   expc_wvlns.clear();
   expi_wvlns.clear();

   for ( int ii = strtx; ii < endx; ii += dlambda )
   {  // Accumulate new list of export lambdas by looking at all raw lambas
      QString clam   = rawc_wvlns[ ii ];
      int     rlam   = clam.toInt();      // Current raw input lambda
      expc_wvlns << clam;
      expi_wvlns << rlam;
   }

   nwavelo        = expi_wvlns.size();
   plotx          = qMax( 0, expi_wvlns.indexOf( plambd ) );
   plotx          = ( plotx < nwavelo ) ? plotx : ( nwavelo / 2 );
DbgLv(1) << "rpl:   nwavelo plotx" << nwavelo << plotx;
DbgLv(1) << "rpl:    pl1 pln" << expi_wvlns[0] << expi_wvlns[nwavelo-1];

   if ( xaxis_radius )
   {  // If x-axis is radius, reset wavelength-to-plot list
      cb_lplot->disconnect();
      cb_lplot->clear();
      cb_lplot->addItems( expc_wvlns );
      connect( cb_lplot,  SIGNAL( currentIndexChanged( int    ) ),
               this,      SLOT  ( lambda_plot_value  ( int    ) ) );
      cb_lplot->setCurrentIndex( plotx );
   }

   // Report export lambda range
   le_lxrng ->setText( tr( "%1 MWL exports: %2 %3 to %4," )
      .arg( nwavelo ).arg( chlamb ).arg( slambda ).arg( elambda )
      + ( lsel_range ? tr( " raw index increment %1." ).arg( dlambda )
                     : tr( " from custom selections." ) ) );

   mwl_data.set_lambdas( expi_wvlns, triple_index );
   std::sort( expi_wvlns.begin(), expi_wvlns.end() );
DbgLv(1) << "rpl: set_lambdas() complete.  trx" << triple_index;

   reset_outData();
DbgLv(1) << "rpl: reset_outData() complete";
}

// turn on and off scan control widgets and lambda widgets as switching to x axis wavelength
void US_Edit::xaxis_wavl_wgts_on( bool on )
{
   rb_lrange->setDisabled( on );
   rb_custom->setDisabled( on );
   pb_incall->setDisabled( on );
   ct_ldelta->setDisabled( on );
   pb_incall->setDisabled( on );
   cb_lstart->setDisabled( on );
   cb_lend->setDisabled( on );
   pb_custom->setDisabled( on );

   ct_from->setDisabled( on );
   ct_to->setDisabled( on );
   pb_excludeRange->setDisabled( on );
   pb_edit1->setDisabled( on );
   pb_exclusion->setDisabled( on );
   pb_include->setDisabled( on );

   if ( ! on ) {
      int from = ct_from->value();
      int to = ct_to->value();
      if ( from == 0 )
         pb_edit1  ->setEnabled( false );
      else
         pb_edit1  ->setEnabled( true );

      if ( to == 0 )
         pb_excludeRange->setEnabled( false );
      else
         pb_excludeRange->setEnabled( true );

      if ( rb_custom->isChecked() ) {
          ct_ldelta->setDisabled( true );
          pb_custom->setEnabled( true );
      } else {
          ct_ldelta->setEnabled( true );
          pb_custom->setEnabled( true );
      }

   }

}

// X-axis has been changed to Radius or Wavelength
void US_Edit::xaxis_radius_on( bool checked )
{
DbgLv(1) << "xaxis_radius_on  checked" << checked;
   if ( checked )
   {
      xaxis_radius = true;
      lb_lplot->setText( tr( "Plot (W nm):" ) );

      cb_lplot->disconnect();
      cb_lplot->clear();
      cb_lplot->addItems( expc_wvlns );
      connect( cb_lplot,  SIGNAL( currentIndexChanged( int    ) ),
               this,      SLOT  ( lambda_plot_value  ( int    ) ) );
      cb_lplot->setCurrentIndex( expc_wvlns.size() / 2 );

      xaxis_wavl_wgts_on( false );
   }
}

// X-axis has been changed to Radius or Wavelength
void US_Edit::xaxis_waveln_on( bool checked )
{
DbgLv(1) << "xaxis_waveln_on  checked" << checked;
   if ( checked )
   {
      xaxis_radius = false;
      lb_lplot->setText( tr( "Plot (R cm):" ) );

      cb_lplot->disconnect();
      cb_lplot->clear();
      int ccx = cb_triple->currentIndex();
      expc_radii.clear();
      expd_radii.clear();
      foreach (double xval, rdata_xvals.at(ccx)) {
          expc_radii << QString::number(xval);
          expd_radii << xval;
      }
      cb_lplot->addItems( expc_radii );
      connect( cb_lplot,  SIGNAL( currentIndexChanged( int    ) ),
               this,      SLOT  ( lambda_plot_value  ( int    ) ) );
      cb_lplot->setCurrentIndex( expc_radii.size() / 2 );

      xaxis_wavl_wgts_on( true );
   }
}

// Plot Lambda/Radius value has changed
void US_Edit::lambda_plot_value( int value )
{
   if ( value < 0 )  return;

   double  menissv  = meniscus;
   plotndx          = value;

   if ( ! xaxis_radius )
   {  // If plotting radius records, go straight to plotting
      plot_mwl();
      return;
   }

   // If plotting wavelength records, check need to re-do/un-do edits
   QString swavl    = cb_lplot ->itemText( plotndx );
   QString triple   = cb_triple->currentText() + " / " + swavl;
   int     idax     = triples.indexOf( triple );
   plotrec          = swavl.toInt();
DbgLv(1) << "lambda_plot_value  value" << value << plotrec;
   QString fname    = ( menissv != 0.0 ) ? editFnames[ idax ] : "none";

   //ALEXEY: 
   if ( us_edit_auto_mode )
     {
       plot_mwl();
       plot_range();
       return;
     }
   ////////
   

   if ( fname == "none" )
   {  // New wavelength has no edit:  turn off edits
     qDebug() << "Setting meniscus!!";
      set_meniscus();
   }

   else if ( fname != "same" )
   {  // New wavelength has its own edit:  apply it

     qDebug() << " Same: appluing edits --";
      US_DataIO::EditValues parameters;

      US_DataIO::readEdits( workingDir + fname, parameters );

      apply_edits( parameters );
   }

   else if ( step != MENISCUS )
   {  // New wavelength has same edit as others in channel:  make sure applied

     qDebug() << "FOR MWL: switching between wavelengths...";
   }

   plot_mwl();
}

// Plot-previous has been clicked
void US_Edit::lambda_plot_prev()
{
DbgLv(1) << "lambda_plot_prev  clicked";
   plotndx--;

   if ( plotndx <= 0 )
   {
      plotndx = 0;
      pb_larrow->setEnabled( false );
   }

   pb_rarrow->setEnabled     ( true );
   cb_lplot ->setCurrentIndex( plotndx );
}

// Plot-next has been clicked
void US_Edit::lambda_plot_next()
{
DbgLv(1) << "lambda_plot_next  clicked";
   plotndx++;

   int lstx = xaxis_radius ? ( expc_wvlns.size() - 1 )
                           : ( expc_radii.size() - 1 );

   if ( plotndx >= lstx )
   {
      plotndx = lstx;
      pb_rarrow->setEnabled( false );
   }

   pb_larrow->setEnabled     ( true );
   cb_lplot ->setCurrentIndex( plotndx );

   qDebug() << "plotndx = " << plotndx;
}

// Custom Lambdas has been clicked
void US_Edit::lambda_custom_list()
{
DbgLv(1) << "lambda_custom_list  clicked";
   lambdas_by_cell();

   US_SelectLambdas* sel_lambd = new US_SelectLambdas( rawi_wvlns );

   connect( sel_lambd, SIGNAL( new_lambda_list( QVector< int > ) ),
            this,      SLOT  ( lambda_new_list( QVector< int > ) ) );

   if ( sel_lambd->exec() == QDialog::Accepted )
   {
DbgLv(1) << "  lambda_custom_list  ACCEPTED";
      int  plotx  = cb_lplot ->currentIndex();
      plotx       = ( plotx < nwavelo ) ? plotx : ( nwavelo / 2 );

      if ( xaxis_radius )
      {  // If x-axis is radius, reset wavelength-to-plot list
         cb_lplot->disconnect();
         cb_lplot->clear();
         cb_lplot->addItems( expc_wvlns );
         connect( cb_lplot,  SIGNAL( currentIndexChanged( int    ) ),
                  this,      SLOT  ( lambda_plot_value  ( int    ) ) );
         cb_lplot->setCurrentIndex( plotx );
      }

      // Report export lambda range
      le_lxrng ->setText( tr( "%1 MWL exports: %2 %3 to %4,"
                              " from custom selections." )
         .arg( nwavelo ).arg( chlamb ).arg( slambda ).arg( elambda ) );
   }
}

// Custom Lambdas have been specified
void US_Edit::lambda_new_list( QVector< int > newlams )
{
   nwavelo     = newlams.count();
   expi_wvlns  = newlams;
   slambda     = expi_wvlns[ 0 ];
   elambda     = expi_wvlns[ nwavelo - 1 ];
   expc_wvlns.clear();
DbgLv(1) << "EDT:lnl: nnssee" << nwavelo << expi_wvlns.size() << slambda
 << newlams[0] << elambda << newlams[nwavelo-1];

   for ( int ii = 0; ii < nwavelo; ii++ )
      expc_wvlns << QString::number( expi_wvlns[ ii ] );

   connect_mwl_ctrls( false );
   cb_lplot ->clear();
   cb_lplot ->addItems( expc_wvlns );
   int strtx   = rawi_wvlns.indexOf( slambda );
   int endx    = rawi_wvlns.indexOf( elambda );
   cb_lstart->setCurrentIndex( strtx );
   cb_lend  ->setCurrentIndex( endx  );
   cb_lplot ->setCurrentIndex( nwavelo / 2 );
   connect_mwl_ctrls( true );
}

// Include-all-lambda has been clicked
void US_Edit::lambda_include_all()
{
DbgLv(1) << "lambda_include_all  clicked";
   lambdas_by_cell();

   nwavelo     = nwaveln;
   expc_wvlns  = rawc_wvlns;
   expi_wvlns  = rawi_wvlns;
   connect_mwl_ctrls( false );
   ct_ldelta->setValue( 1 );
   cb_lstart->setCurrentIndex( 0 );
   cb_lend  ->setCurrentIndex( nwaveln - 1 );
   connect_mwl_ctrls( true );

   reset_plot_lambdas();
}

// OD-limit-on-radii has changed
void US_Edit::od_radius_limit( double value )
{
DbgLv(1) << "od_radius_limit  value" << value;
   odlimit     = value;

   plot_mwl();

   all_edits    = all_edits_done();
   pb_report  ->setEnabled( all_edits );
   pb_write   ->setEnabled( all_edits );
   ck_writemwl->setEnabled( all_edits && isMwl );
   changes_made = true;
}


// Write edit to all wavelengths of the current cell/channel
void US_Edit::write_mwl_auto( int trx )
{
  
  US_Passwd pw;
  US_DB2* dbP            = new US_DB2( pw.getPasswd() );

  if ( dbP->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Could not connect to database: \n" ) + dbP->lastError() );
      return;
    }

  // triple_index = trx;

  index_data_auto( trx );

  edata          = outData[ data_index ];
  data           = *edata;
  
  is_spike_auto = false;

  QString saved_info = le_info->text();
  QString str;

  if ( ! isMwl )
    {
      QMessageBox::warning( this,
			    tr( "Invalid Selection" ),
			    tr( "The \"Save to all Wavelengths\" button is only valid\n"
				 "for MultiWavelength data. No Save will be performed." ) );
      return;
    }
  
  // Base parameters for triple:
   QString triple_name = cb_triple->itemText( trx );

   le_status->setText( tr( "Saving edit profile for channel %1" ).arg( triple_name ) );
   qApp->processEvents();

   scanExcl_begin_ind = editProfile_scans_excl[ triple_name ][0].toInt();
   scanExcl_end_ind   = editProfile_scans_excl[ triple_name ][1].toInt();
   
   meniscus      = editProfile[ triple_name ][0].toDouble();
   range_left    = editProfile[ triple_name ][1].toDouble();
   range_right   = editProfile[ triple_name ][2].toDouble();
   plateau       = editProfile[ triple_name ][3].toDouble();
   baseline      = editProfile[ triple_name ][4].toDouble();
   
   if ( editProfile[ triple_name ].count() > 6 )
     {
       if ( editProfile[ triple_name ][6] == "spike_true")
	 is_spike_auto = true;
       else if (editProfile[ triple_name ][6] == "spike_false")
	 is_spike_auto = false;
     }

   bottom = centerpieceParameters[ trx ][1].toDouble();  //Should be from centerpiece info from protocol 
   // End of base parameters
   
   //Is this needed ?
   /*
   if ( expIsEquil )
   {  // Equilibrium:  set baseline,plateau as flag that those are "done"
      int jsd     = sd_offs[ triple_index ];
      meniscus    = sData[ jsd ].meniscus;
      range_left  = sData[ jsd ].dataLeft;
      range_right = sData[ jsd ].dataRight;
      baseline    = range_left;
      plateau     = range_right;
   }
   */

   // Check if complete
   if ( meniscus == 0.0 )
      str = tr( "meniscus" );
   else if ( dataType == "IP" && ( airGap_left == 0.0 || airGap_right == 9.0 ) )
      str = tr( "air gap" );
   else if ( range_left == 0.0 || range_right == 9.0 )
      str = tr( "data range" );
   else if ( plateau == 0.0 )
      str = tr( "plateau" );
   else if ( baseline == 0.0 )
      str = tr( "baseline" );

   if ( ! str.isEmpty() )
   {
      QMessageBox::information( this,
            tr( "Data missing" ),
            tr( "You must define the " ) + str +
            tr( " before writing the edit profile." ) );
      return;
   }
   //END check

   // Ask for editLabel if not yet defined
   //editGUIDs[ idax ].clear();
   QString now  =  QDateTime::currentDateTime()
                   .toUTC().toString( "yyMMddhhmm" );

   editLabel = now + QString::number( trx );

   // Is it needed ?
   /*
   QVector< int > oldi_wvlns;
   int     kwavelo  = mwl_data.lambdas( oldi_wvlns );      //ALEXEY: <-- current channel, needs to be looped over channels
   int     nwavelo  = expi_wvlns.count();
   int     wvx;
   bool    chg_lamb = ( kwavelo != nwavelo );

   if ( ! chg_lamb )
   {  // If no change in number of wavelengths, check actual lists
      for ( wvx = 0; wvx < nwavelo; wvx++ )
      {
         if ( oldi_wvlns[ wvx ] != expi_wvlns[ wvx ] )
         {  // There is a difference in the lists:  mark as such
            chg_lamb      = true;
            break;
         }
      }
   }

   if ( chg_lamb )
   {  // If wavelengths have changed, save new list and rebuild some vectors
      mwl_data.set_lambdas( expi_wvlns );  // Save new lambdas for channel

      reset_outData();
   }

   ***********/

   //Set wavelengths for current triple:
   QVector< int > current_wvlns;
   QStringList current_wvlns_list;
   int curr_wvls_count = mwl_data.lambdas( current_wvlns, trx );
   std::sort(current_wvlns.begin(), current_wvlns.end());
   //wvlns to list
   for ( int wvx = 0; wvx < curr_wvls_count; wvx++ )
     {
       current_wvlns_list << QString::number( current_wvlns[ wvx ] );
     }
   
   QString celchn   = celchns.at( triple_index );                // ALEXEY: <-- is it  lopped over triple indecies? 
   QString scell    = celchn.section( "/", 0, 0 ).simplified();
   QString schan    = celchn.section( "/", 1, 1 ).simplified();
   QString tripbase = scell + " / " + schan + " / ";
   int     idax     = triples.indexOf( tripbase + current_wvlns_list[ 0 ] ); 
   //int     odax     = index_data_auto( trx, 0 );                             //CORRECT! {first odax in a triple}                                

   //qDebug() << "Write_MWL:  triple_index, #wvlns, odax, celchn" << triple_index << "," << curr_wvls_count << ", " << odax << "," << celchn;

   QString filebase = files[ idax ].section( ".",  0, -6 )
                    + "." + editLabel + "."
                    + files[ idax ].section( ".", -5, -5 )
                    + "." + scell + "." + schan + ".";
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   QString filebase_qry   = files[ idax ].section( ".",  0, -6 );
   QString triplename_qry = scell + "." + schan + ".";
   
   // Loop to output a file/db-record for each wavelength of the cell/channel
   for ( int wvx = 0; wvx < current_wvlns_list.size(); wvx++ )           //ALEXEY: needs to be looped over channels, not only current channel
   {
      QString swavl    = current_wvlns_list[ wvx ];
      QString triple   = tripbase + swavl;
      QString filename = filebase + swavl + ".xml";
      idax             = triples.indexOf( triple );
      //odax             = index_data_auto( trx, wvx );                             // Correct ? NOT!!! Got you !!

      
      qDebug()  << "EDT:WrMwl:  wvx triple" << wvx << triple << "filename" << filename
		<< ", trx " << trx << ", wvx " << wvx; // << ", odax " << odax;

      QString editGUID = editGUIDs[ idax ];

      if ( editGUID.isEmpty() )
      {
         editGUID      = US_Util::new_guid();
         editGUIDs.replace( idax, editGUID );
      }


      //ALEXEY: instead -- get rawDataGUID based on query:
      // select rawDataGUID from rawData where filename like '%MartinR_EcoRI_Digest_GMP_Optima1_23OCT23-run1985%2.A.260%';
      // filename: MartinR_EcoRI_Digest_GMP_Optima1_23OCT23-run1985.23102502280.RI.2.A.260.xml
      QString triplename_qry_wvl = triplename_qry + swavl;

      QString rawGUID = get_rawDataGUID( dbP, filebase_qry, triplename_qry_wvl );

      qDebug() << "rawGUID -- " <<rawGUID;
      // QString rawGUID  = US_Util::uuid_unparse(
      //       (unsigned char*)outData[ odax ]->rawGUID );


      // Output the edit XML file
      le_info   ->setText( tr( "Writing " ) + filename + " ..." );
      le_status ->setText( tr( "Writing " ) + filename + " ..." );
      qApp->processEvents();
      int wrstat       = write_xml_file( filename, triple, editGUID, rawGUID );

      if ( wrstat != 0 )
         return;
      else
         editFnames[ idax ] = filename;

      if ( disk_controls->db() )
      {
         if ( dbP == NULL )
         {
            US_Passwd pw;
            dbP          = new US_DB2( pw.getPasswd() );
            if ( dbP == NULL  ||  dbP->lastErrno() != US_DB2::OK )
            {
               QMessageBox::warning( this, tr( "Connection Problem" ),
                 tr( "Could not connect to database \n" ) + dbP->lastError() );
               return;
            }
         }

         QString editID   = editIDs[ idax ];

         // Output the edit database record
         wrstat = write_edit_db( dbP, filename, editGUID, editID, rawGUID );
DbgLv(1) << "EDT:WrMwl:  dax fname" << idax << filename << "wrstat" << wrstat;

         if ( wrstat != 0 )
            return;
      }  // END:  DB output

      //++odax;      //Got you!!
      
   }  // END:  wavelength-in-cellchannel loop

   // QApplication::restoreOverrideCursor();
   // changes_made = false;
   // pb_report   ->setEnabled( false );
   // pb_write    ->setEnabled( false );
   // ck_writemwl ->setEnabled( false );
   le_info->setText( saved_info );
   qApp->processEvents();

   if ( runType_combined_IP_RI ) 
     cb_triple->disconnect();
}

//get rawDataGUID based on filebase && triple names
QString US_Edit::get_rawDataGUID( US_DB2* db, QString filebase_qry, QString triplename_qry )
{

  QString rawID   = QString("");
  QString rawGUID = QString("");
  
  QStringList qry;
  qry << "get_rawDataGUID_from_filename" << filebase_qry << triplename_qry;
  qDebug() << "get_rawDataGUID_from_filename QRY -- " << qry;
  db->query( qry );
  
  while ( db->next() )
    {
      rawID   = db->value( 0 ).toString();
      rawGUID = db->value( 1 ).toString();
    }

  if ( rawGUID.isEmpty() )
    qDebug() << "EMPTY rawID,rawGUID!!";

  qDebug() << "rawDataID, GUID -- " << rawID << ", " << rawGUID;

  return rawGUID;
}


// Write edit to all wavelengths of the current cell/channel
void US_Edit::write_mwl()
{
   QString saved_info = le_info->text();
   QString str;
   if ( ! isMwl )
   {
      QMessageBox::warning( this,
            tr( "Invalid Selection" ),
            tr( "The \"Save to all Wavelengths\" button is only valid\n"
                "for MultiWavelength data. No Save will be performed." ) );
      return;
   }

   meniscus  = le_meniscus->text().toDouble();
   baseline  = data.xvalues[ data.xindex( range_left ) + 5 ];

   if ( expIsEquil )
   {  // Equilibrium:  set baseline,plateau as flag that those are "done"
      int jsd     = sd_offs[ triple_index ];
      meniscus    = sData[ jsd ].meniscus;
      range_left  = sData[ jsd ].dataLeft;
      range_right = sData[ jsd ].dataRight;
      baseline    = range_left;
      plateau     = range_right;
   }

   // Check if complete
   if ( meniscus == 0.0 )
      str = tr( "meniscus" );
   else if ( dataType == "IP" && ( airGap_left == 0.0 || airGap_right == 9.0 ) )
      str = tr( "air gap" );
   else if ( range_left == 0.0 || range_right == 9.0 )
      str = tr( "data range" );
   else if ( plateau == 0.0 )
      str = tr( "plateau" );
   else if ( baseline == 0.0 )
      str = tr( "baseline" );

   if ( ! str.isEmpty() )
   {
      QMessageBox::information( this,
            tr( "Data missing" ),
            tr( "You must define the " ) + str +
            tr( " before writing the edit profile." ) );
      return;
   }

   QString sufx = "";

   // Ask for editLabel if not yet defined
   while ( editLabel.isEmpty() )
   {
      QString now  =  QDateTime::currentDateTime()
                      .toUTC().toString( "yyMMddhhmm" );

      bool ok;
      QString msg  = tr( "The base Edit Label for this edit session is <b>" )
         + now + "</b> .<br/>"
         + tr( "You may add an optional suffix to further distinquish<br/>"
               "the Edit Label. Use alphanumeric characters, underscores,<br/>"
               "or hyphens (no spaces). Enter 0 to 10 suffix characters." );
      sufx         = QInputDialog::getText( this, 
         tr( "Create a unique session Edit Label" ),
         msg,
         QLineEdit::Normal,
         sufx,
         &ok );
      
      if ( ! ok ) return;

      sufx.remove( QRegExp( "[^\\w\\d_-]" ) );
      editLabel    = now + sufx;

      if ( editLabel.length() > 20 )
      {
         QMessageBox::critical( this,
            tr( "Text length error" ),
            tr( "You entered %1 characters for the Edit Label suffix.\n"
                "Re-enter, limiting length to 10 characters." )
            .arg( sufx.length() ) );
         editLabel.clear();
         sufx = sufx.left( 10 );
      }
   }

   QVector< int > oldi_wvlns;
   int     kwavelo  = mwl_data.lambdas( oldi_wvlns );
   std::sort(oldi_wvlns.begin(), oldi_wvlns.end());
   int     nwavelo  = expi_wvlns.count();
   int     wvx;
   bool    chg_lamb = ( kwavelo != nwavelo );

   if ( ! chg_lamb )
   {  // If no change in number of wavelengths, check actual lists
      for ( wvx = 0; wvx < nwavelo; wvx++ )
      {
         if ( oldi_wvlns[ wvx ] != expi_wvlns[ wvx ] )
         {  // There is a difference in the lists:  mark as such
            chg_lamb      = true;
            break;
         }
      }
   }

   if ( chg_lamb )
   {  // If wavelengths have changed, save new list and rebuild some vectors
      mwl_data.set_lambdas( expi_wvlns );  // Save new lambdas for channel
      std::sort( expi_wvlns.begin(), expi_wvlns.end() );
      reset_outData();
   }

   QString celchn   = celchns.at( triple_index );
   QString scell    = celchn.section( "/", 0, 0 ).simplified();
   QString schan    = celchn.section( "/", 1, 1 ).simplified();
   QString tripbase = scell + " / " + schan + " / ";
   int     idax     = triples.indexOf( tripbase + expc_wvlns[ 0 ] );
   int     odax     = index_data( 0 );
DbgLv(1) << "EDT:WrMwl:  dax celchn" << odax << celchn;

   QString filebase = files[ idax ].section( ".",  0, -6 )
                    + "." + editLabel + "."
                    + files[ idax ].section( ".", -5, -5 )
                    + "." + scell + "." + schan + ".";
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Loop to output a file/db-record for each wavelength of the cell/channel

DbgLv(1) << "EDT:WrMwl: files,wvlns.size" << files.size() << expc_wvlns.size();
   for ( wvx = 0; wvx < expc_wvlns.size(); wvx++ )
   {
      QString swavl    = expc_wvlns[ wvx ];
      QString triple   = tripbase + swavl;
      QString filename = filebase + swavl + ".xml";
      idax             = triples.indexOf( triple );
      odax             = index_data( wvx );
DbgLv(1) << "EDT:WrMwl:  wvx triple" << wvx << triple << "filename" << filename;

DbgLv(1) << "EDT:WrMwl:   idax,editGUIDs.size" << idax << editGUIDs.size();
      QString editGUID = editGUIDs[ idax ];

      if ( editGUID.isEmpty() )
      {
         editGUID      = US_Util::new_guid();
         editGUIDs.replace( idax, editGUID );
      }
DbgLv(1) << "EDT:WrMwl:    editGUID" << editGUID;

DbgLv(1) << "EDT:WrMwl:   odax,outData.size" << odax << outData.size();
      QString rawGUID  = US_Util::uuid_unparse(
            (unsigned char*)outData[ odax ]->rawGUID );
DbgLv(1) << "EDT:WrMwl:    rawGUID" << rawGUID;

      // Output the edit XML file
      le_info->setText( tr( "Writing " ) + filename + " ..." );
      qApp->processEvents();
      int wrstat       = write_xml_file( filename, triple, editGUID, rawGUID );
DbgLv(1) << "EDT:WrMwl:     write_xml_file stat" << wrstat;

      if ( wrstat != 0 )
         return;
      else
         editFnames[ idax ] = filename;

      if ( disk_controls->db() )
      {
         if ( dbP == NULL )
         {
            US_Passwd pw;
            dbP          = new US_DB2( pw.getPasswd() );
            if ( dbP == NULL  ||  dbP->lastErrno() != US_DB2::OK )
            {
               QMessageBox::warning( this, tr( "Connection Problem" ),
                 tr( "Could not connect to database \n" ) + dbP->lastError() );
               return;
            }
         }

         QString editID   = editIDs[ idax ];

         // Output the edit database record
         wrstat = write_edit_db( dbP, filename, editGUID, editID, rawGUID );
DbgLv(1) << "EDT:WrMwl:  dax fname" << idax << filename << "wrstat" << wrstat;

         if ( wrstat != 0 )
            return;
      }  // END:  DB output
   }  // END:  wavelength-in-cellchannel loop

   QApplication::restoreOverrideCursor();
   changes_made = false;
   pb_report   ->setEnabled( false );
   pb_write    ->setEnabled( false );
   ck_writemwl ->setEnabled( false );
   le_info->setText( saved_info );
   qApp->processEvents();
DbgLv(1) << "EDT:WrMwl: DONE";
}

// Write edit xml file
int US_Edit::write_xml_file( QString& fname, QString& triple,
      QString& editGUID, QString& rawGUID )
{
   QFile efo( workingDir + fname );

   if ( ! efo.open( QFile::WriteOnly | QFile::Text ) )
   {
      QMessageBox::information( this,
            tr( "File write error" ),
            tr( "Could not open the file\n" ) + workingDir + fname
            + tr( "\n for writing.  Check your permissions." ) );
      return 1;
   }

DbgLv(1) << "EDT:WrXml: IN: fname,triple,editGUID,rawGUID"
 << fname << triple << editGUID << rawGUID;
   QXmlStreamWriter xml( &efo );

   xml.setAutoFormatting( true );
   xml.writeStartDocument();
   xml.writeDTD         ( "<!DOCTYPE UltraScanEdits>" );
   xml.writeStartElement( "experiment" );
   xml.writeAttribute   ( "type", expType );

   // Write identification
   xml.writeStartElement( "identification" );

   xml.writeStartElement( "runid" );
   xml.writeAttribute   ( "value", runID );
   xml.writeEndElement  ();

   xml.writeStartElement( "editGUID" );
   xml.writeAttribute   ( "value", editGUID );
   xml.writeEndElement  ();

   xml.writeStartElement( "rawDataGUID" );
   xml.writeAttribute   ( "value", rawGUID );
   xml.writeEndElement  ();

   xml.writeEndElement  ();  // identification

   QStringList parts   = triple.contains( " / " ) ?
                         triple.split( " / " ) :
                         triple.split( "." );
DbgLv(1) << "EDT:WrXml:  parts.size" << parts.size();

   QString     cell    = parts[ 0 ];
   QString     channel = parts[ 1 ];
   QString     waveln  = parts[ 2 ];

DbgLv(1) << "EDT:WrXml:  waveln" << waveln;

   xml.writeStartElement( "run" );
   xml.writeAttribute   ( "cell",       cell    );
   xml.writeAttribute   ( "channel",    channel );
   xml.writeAttribute   ( "wavelength", waveln  );

   // Write excluded scans
   if ( us_edit_auto_mode )  //<-- write excluded scans (beg|end) based on AProfile
     {
        xml.writeStartElement( "excludes" );
	
	//beginning of the scan set
	for ( int ii = 0; ii < scanExcl_begin_ind; ii++ )
	  {
	    xml.writeStartElement( "exclude" );
	    xml.writeAttribute   ( "scan", QString::number( ii ) );
	    xml.writeEndElement  ();
	  }
	//end of the scan set
       	for ( int ii = data.scanData.size() - scanExcl_end_ind; ii < data.scanData.size(); ii++ )
	  {
	    xml.writeStartElement( "exclude" );
	    xml.writeAttribute   ( "scan", QString::number( ii ) );
	    xml.writeEndElement  ();
	  }	
	
	xml.writeEndElement  ();  // excludes
     }
   else
     {
       if ( data.scanData.size() > includes.size() )
	 {
	   xml.writeStartElement( "excludes" );
	   
	   for ( int ii = 0; ii < data.scanData.size(); ii++ )
	     {
	       if ( ! includes.contains( ii ) )
		 {
		   xml.writeStartElement( "exclude" );
		   xml.writeAttribute   ( "scan", QString::number( ii ) );
		   xml.writeEndElement  ();
		 }
	     }
	 
	   xml.writeEndElement  ();  // excludes
	 }
     }

   // Write edits
   if ( ! changed_points.isEmpty() )
   {
      xml.writeStartElement( "edited" );

      for ( int ii = 0; ii < changed_points.size(); ii++ )
      {
         Edits* e = &changed_points[ ii ];

         for ( int jj = 0; jj < e->changes.size(); jj++ )
         {
            xml.writeStartElement( "edit" );
            xml.writeAttribute   ( "scan",   QString::number( e->scan ) );
            xml.writeAttribute   ( "radius",
               QString::number( e->changes[ jj ].x(), 'f', 8 ) );
            xml.writeAttribute   ( "value",
               QString::number( e->changes[ jj ].y(), 'f', 8 ) );
            xml.writeEndElement  ();
         }
      }

      xml.writeEndElement  ();  // edited
   }

   // Write meniscus, range, plateau, baseline, odlimit
   xml.writeStartElement( "parameters" );

   if ( ! expIsEquil )
   {  // non-Equilibrium
      xml.writeStartElement( "meniscus" );
      xml.writeAttribute   ( "radius",
         QString::number( meniscus, 'f', 8 ) );
      xml.writeEndElement  ();
      xml.writeStartElement( "bottom" );
      xml.writeAttribute   ( "radius",
         QString::number( bottom, 'f', 8 ) );
      xml.writeEndElement  ();

      if ( dataType == "IP" )
      {
         xml.writeStartElement( "air_gap" );
         xml.writeAttribute   ( "left",
            QString::number( airGap_left,      'f', 8 ) );
         xml.writeAttribute   ( "right",
            QString::number( airGap_right,     'f', 8 ) );
         xml.writeAttribute   ( "tolerance",
            QString::number( ct_gaps->value(), 'f', 8 ) );
         xml.writeEndElement  ();
      }

      xml.writeStartElement( "data_range" );
      xml.writeAttribute   ( "left",
         QString::number( range_left,  'f', 8 ) );
      xml.writeAttribute   ( "right",
         QString::number( range_right, 'f', 8 ) );
      xml.writeEndElement  ();

      xml.writeStartElement( "plateau" );
      xml.writeAttribute   ( "radius",
         QString::number( plateau,  'f', 8 ) );
      xml.writeEndElement  ();

      xml.writeStartElement( "baseline" );
      xml.writeAttribute   ( "radius",
         QString::number( baseline, 'f', 8 ) );
      xml.writeEndElement  ();

      xml.writeStartElement( "od_limit" );
      xml.writeAttribute   ( "value",
         QString::number( odlimit,  'f', 8 ) );
      xml.writeEndElement  ();
   }

   else
   {  // Equilibrium
      if ( dataType == "IP" )
      {
         xml.writeStartElement( "air_gap" );
         xml.writeAttribute   ( "left",
            QString::number( airGap_left,      'f', 8 ) );
         xml.writeAttribute   ( "right",
            QString::number( airGap_right,     'f', 8 ) );
         xml.writeAttribute   ( "tolerance",
            QString::number( ct_gaps->value(), 'f', 8 ) );
         xml.writeEndElement  ();
      }

      int jsd  = sd_offs[ triple_index ];
      int ksd  = jsd + sd_knts[ triple_index ];

      for ( int ii = jsd; ii < ksd; ii++ )
      {
         double speed     = sData[ ii ].speed;
         int    sStart    = sData[ ii ].first_scan;
         int    sCount    = sData[ ii ].scan_count;
         double meniscus  = sData[ ii ].meniscus;
         double dataLeft  = sData[ ii ].dataLeft;
         double dataRight = sData[ ii ].dataRight;

         xml.writeStartElement( "speed" );
         xml.writeAttribute   ( "value",     QString::number( speed )  );
         xml.writeAttribute   ( "scanStart", QString::number( sStart ) );
         xml.writeAttribute   ( "scanCount", QString::number( sCount ) );

         xml.writeStartElement( "meniscus" );
         xml.writeAttribute   ( "radius", QString::number( meniscus, 'f', 8 ) );
         xml.writeEndElement  ();  // meniscus

         xml.writeStartElement( "data_range" );
         xml.writeAttribute   ( "left",  QString::number( dataLeft,  'f', 8 ) );
         xml.writeAttribute   ( "right", QString::number( dataRight, 'f', 8 ) );
         xml.writeEndElement  ();  // data_range

         xml.writeEndElement  ();  // speed
      }
   }
 
   xml.writeEndElement  ();  // parameters

   if ( ! pb_residuals->icon().isNull()  ||
        ! pb_spikes->icon().isNull()     ||
        invert == -1.0                   ||
        floatingData                     ||
	is_spike_auto                     )
   {
      xml.writeStartElement( "operations" );
 
      // Write RI Noise
      if ( ! pb_residuals->icon().isNull() )
      {
         xml.writeStartElement( "subtract_ri_noise" );
         xml.writeAttribute   ( "order", QString::number( noise_order ) );
         xml.writeEndElement  ();
      }

      // Write Remove Spikes
      if ( !us_edit_auto_mode )
	{
	  if ( ! pb_spikes->icon().isNull() )
	    {
	      xml.writeStartElement( "remove_spikes" );
	      xml.writeEndElement  ();
	    }
	}
      else
	{
	  if ( is_spike_auto )
	    //if ( is_spike_auto || dataType == "IP" ) //ALEXEY: for now, for interference data which are processed manually....
	    {
	      xml.writeStartElement( "remove_spikes" );
	      xml.writeEndElement  ();
	    }
	}

      // Write Invert
      if ( invert == -1.0 )
      {
         xml.writeStartElement( "invert" );
         xml.writeEndElement  ();
      }

      // Write indication of floating data
      if ( floatingData )
      {
         xml.writeStartElement( "floating_data" );
         xml.writeEndElement  ();
      }

      xml.writeEndElement  ();  // operations
   }

   xml.writeEndElement  ();  // run
   xml.writeEndElement  ();  // experiment
   xml.writeEndDocument ();

   efo.close();
   return 0;
}

// Write edit database record
int US_Edit::write_edit_db( US_DB2* dbP, QString& fname, QString& editGUID,
      QString& editID, QString& rawGUID )
{
   int idEdit;

   if ( dbP == NULL )
   {
      QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to database \n" ) + dbP->lastError() );
      return 1;
   }

   QStringList query( "get_rawDataID_from_GUID" );
   query << rawGUID;
   dbP->query( query );

   if ( dbP->lastErrno() != US_DB2::OK )
   {
      QMessageBox::warning( this, 
         tr( "AUC Data is not in DB" ),
         tr( "Cannot save edit data to the database.\n"
             "The associated AUC data is not present." ) );
      return 2;
   }

   dbP->next();
   QString rawDataID = dbP->value( 0 ).toString();


   // Save edit file to DB
   query.clear();

   if ( editID.isEmpty() )
   {
      query << "new_editedData" << rawDataID << editGUID << runID
            << fname << "";

      dbP->query( query );

      if ( dbP->lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this, tr( "Database Problem" ),
            tr( "Could not insert metadata into the database\n" ) + 
                dbP->lastError() );

         return 3;
      }

      idEdit   = dbP->lastInsertID();
      editID   = QString::number( idEdit );
      int dax  = editGUIDs.indexOf( editGUID );
      editIDs.replace( dax, editID );
   }

   else
   {
      query << "update_editedData" << editID << rawDataID << editGUID
            << runID << fname << "";
      dbP->query( query );

      if ( dbP->lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this, tr( "Database Problem" ),
            tr( "Could not update metadata in the database \n" ) + 
            dbP->lastError() );

         return 4;
      }

      idEdit   = editID.toInt();
   }

   dbP->writeBlobToDB( workingDir + fname, "upload_editData", idEdit );

   if ( dbP->lastErrno() != US_DB2::OK )
   {
      QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Could not insert edit xml data into database \n" ) + 
         dbP->lastError() );
      return 5;
   }

   return 0;
}

// Return the index in the output data of the current or specified wavelength
int US_Edit::index_data_auto( int trx, int wvx )
{
   triple_index = trx;  // Triple index
   int odatx    = triple_index;               // Default output data index

   QString triple_name = cb_triple->itemText( trx );

   qDebug() << "Index_data_auto: triple_name, isMwl -- " << triple_name << isMwl;

   //Debug
   for ( int g=0; g < expi_wvlns.size(); ++g )
     qDebug() << "MWL wavelengths for triple: " << triple_name << expi_wvlns[ g ];
   
   if ( isMwl )
   {  // For MWL, compute data index from wavelength and triple indexes
      if ( wvx < 0 )
      {  // For the default case, use the current wavelength index
         plotndx      = cb_lplot->currentIndex();
         int iwavl    = expi_wvlns[ plotndx ];              //ALEXEY: do we need to set data in the middle of the triple set ??

	 //int iwavl    = expi_wvlns[ 0 ];                      // ALEXEY: OR beginning of the set? ??   
	 data_index   = mwl_data.data_index( iwavl, triple_index );
         odatx        = data_index;
DbgLv(1) << "IxDa_AUTO: dx" << data_index << "plx wavl trx"
 << plotndx << iwavl << triple_index;
      }

      else
      {  // Use a specified wavelength index (and do not set internal value)
         int iwavl    = expi_wvlns[ wvx ];
         odatx        = mwl_data.data_index( iwavl, triple_index );
      }
   }

   else  // for non-MWL, set data index internal value to triple index
      data_index   = triple_index;

   qDebug() << "Triple index in INDEX_DATA: trx, wvx, data_index: " << triple_index << ", " << wvx << ", " << data_index;

   return odatx;
}

// Return the index in the output data of the current or specified wavelength
int US_Edit::index_data( int wvx )
{
   triple_index = cb_triple->currentIndex();  // Triple index
   int odatx    = triple_index;               // Default output data index

   if ( isMwl )
   {  // For MWL, compute data index from wavelength and triple indexes
      if ( wvx < 0 )
      {  // For the default case, use the current wavelength index
         if ( ! xaxis_radius ) return 0;

         plotndx      = cb_lplot->currentIndex();
         int iwavl    = expi_wvlns[ plotndx ];
         data_index   = mwl_data.data_index( iwavl, triple_index );
         odatx        = data_index;
DbgLv(1) << "IxDa: dx" << data_index << "plx wavl trx"
 << plotndx << iwavl << triple_index;
      }

      else
      {  // Use a specified wavelength index (and do not set internal value)
         int iwavl    = expi_wvlns[ wvx ];
         odatx        = mwl_data.data_index( iwavl, triple_index );
      }
   }

   else  // for non-MWL, set data index internal value to triple index
      data_index   = triple_index;

   return odatx;
}

// Get a list of filenames with Edit Label like a specified file name
int US_Edit::like_edit_files( QString filename, QStringList& editfiles,
      US_DB2* dbP )
{
   // Determine local-disk files with same edit label and cell/channel
   QString filebase = filename.section( ".", 0, -3 );
   QStringList filter( filebase + ".*.xml" );
   QStringList edfiles = QDir( workingDir ).entryList(
         filter, QDir::Files, QDir::Name );
   QStringList ldfiles;

   // Pare down the list to those with triples matching loaded files
   for ( int ii = 0; ii < edfiles.count(); ii++ )
   {
      QString fname    = edfiles[ ii ];
      QString triple   = fname.section( ".", -4, -2 ).replace( ".", " / " );

      if ( triples.contains( triple ) )
         ldfiles << fname;
   }

   int     nledits  = ldfiles.count();
DbgLv(1) << "LiEdFi: nledits" << ldfiles.count() << filter;

   if ( dbP != NULL )
   {  // Determine database like files and compare to local
      QString     invID = QString::number( US_Settings::us_inv_ID() );
      QStringList dbfiles;
      QStringList dbEdIds;
      QStringList dbquery;

      // Get the filenames and edit IDs for like-named files in the DB
      dbquery.clear();
      dbquery << "all_editedDataIDs" << invID;
      dbP->query( dbquery );

      while ( dbP->next() )
      {  // Accumulate filenames and editIDs from the database
         QString idData     = dbP->value( 0 ).toString();
         QString efname     = dbP->value( 2 ).toString()
                              .section( "/", -1, -1 );
         QString triple     = efname.section( ".", -4, -2 )
                              .replace( ".", " / " );
//DbgLv(1) << "LiEdFi:   db0,2" << idData << efname << "fbase" << filebase;

         // If the file name has a like edit label, has the same cell/channel,
         //  and has triple matching a loaded file, save it.
         if ( efname.startsWith( filebase )  &&  triples.contains( triple ) )
         {
            dbfiles   << efname;
            dbEdIds   << idData;
DbgLv(1) << "LiEdFi:     id fn dbfsiz" << idData << efname << dbfiles.count();
         }
      }

      int nlocals      = ldfiles.count();
      nledits          = dbfiles.count();
DbgLv(1) << "LiEdFi:  nlocals nledits" << nlocals << nledits;

      if ( nledits != nlocals )
      {  // DB records are not matched locally, so download from DB
         if ( nlocals == 0 )
            QDir().mkpath( workingDir );

         dbfiles.sort();

         for ( int ii = 0; ii < dbfiles.count(); ii++ )
         {  // Download each DB record to a local file
            QString fname     = workingDir + dbfiles[ ii ];
            int     dataID    = dbEdIds[ ii ].toInt();

            dbP->readBlobFromDB( fname, "download_editData", dataID );
         }

         ldfiles           = dbfiles;
      }
   }

   editfiles        = ldfiles;
   nledits          = editfiles.count();

   if ( nledits == 0 )
   {  // No files (should not happen):  return at least the specified file name
      editfiles << filename;
      nledits          = 1;
   }

   else if ( nledits > 1 )
   {  // If multiple files, sort them
      editfiles.sort();
   }
DbgLv(1) << "LiEdFi: nle fn0" << nledits << editfiles[0];

   return nledits;
}

// Apply edits to the current triple data
int US_Edit::apply_edits( US_DataIO::EditValues parameters )
{
   int status = 0;

   // Apply the edits with specified parameters
   QString wkstr;

   meniscus    = parameters.meniscus;
   range_left  = parameters.rangeLeft;
   range_right = parameters.rangeRight;
   plateau     = parameters.plateau;
   baseline    = parameters.baseline;
   odlimit     = parameters.ODlimit;

   le_meniscus->setText( wkstr.sprintf( "%.3f", meniscus ) );
   pb_meniscus->setIcon( check );
   pb_meniscus->setEnabled( true );

   airGap_left  = parameters.airGapLeft;
   airGap_right = parameters.airGapRight;

   if ( dataType == "IP" )
   {
      US_DataIO::adjust_interference( data, parameters );
      US_DataIO::calc_integral      ( data, parameters );
      le_airGap->setText( wkstr.sprintf( "%.3f - %.3f", 
               airGap_left, airGap_right ) ); 
      pb_airGap->setIcon( check );
      pb_airGap->setEnabled( true );
   }

   le_dataStart->setText( QString::number( range_left,  'f', 8 ) );
   le_dataEnd  ->setText( QString::number( range_right, 'f', 8 ) );
   pb_dataEnd  ->setIcon( check );
   pb_dataEnd  ->setEnabled( true );
   
   plateau      = range_right - _PLATEAU_OFFSET_;
   le_plateau  ->setText( QString::number( plateau,     'f', 8 ) );

   US_DataIO::Scan  scan  = data.scanData.last();
   int              pt    = data.xindex( baseline );
   double           sum   = 0.0;

   // Average the value for +/- 5 points
   for ( int jj = pt - 5; jj <= pt + 5; jj++ )
      sum += scan.rvalues[ jj ];
double bl=sum/11.0;
DbgLv(1) << "BL: DD : baseline bl" << baseline << bl;

   le_baseline->setText( wkstr.sprintf( "%.3f (%.3e)", baseline, sum / 11.0 ) );

   // Invert
   invert = parameters.invert;
   
   if ( invert == -1.0 ) pb_invert->setIcon( check );
   else                  pb_invert->setIcon( QIcon() );

   // Excluded scans
   init_includes();
   reset_excludes(); // Zero exclude combo boxes
   qSort( parameters.excludes );

   for ( int ii = parameters.excludes.size(); ii > 0; ii-- )
      includes.removeAt( parameters.excludes[ ii - 1 ] );

   // Edited points
   changed_points.clear();

   for ( int ii = 0; ii < parameters.editedPoints.size(); ii++ )
   {
      int    scan   = parameters.editedPoints[ ii ].scan;
      int    index1 = (int)parameters.editedPoints[ ii ].radius;
      double value  = parameters.editedPoints[ ii ].value;
      
      Edits e;
      e.scan = scan;
      e.changes << QPointF( index1, value );
     
      changed_points << e;
      
      data.scanData[ scan ].rvalues[ index1 ] = value;
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
      US_RiNoise::calc_residuals( data, includes, range_left, range_right, 
            noise_order, residuals );
      
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

   ct_odlim->disconnect();
   ct_odlim->setValue( odlimit );
   connect( ct_odlim,  SIGNAL( valueChanged       ( double ) ),
            this,      SLOT  ( od_radius_limit    ( double ) ) );

   set_pbColors( NULL );
   step        = FINISHED;

   return status;
}

// Reset the output data pointer vector after change in wavelengths
void US_Edit::reset_outData()
{
   if ( ! isMwl )  return;

   outData.clear();
   QVector< int > ex_wvlns;
   int ccoff   = 0;
DbgLv(1) << "rsoD: aDa size" << allData.size() << "ncelchn" << ncelchn;

   for ( int ccx = 0; ccx < ncelchn; ccx++ )
   {
      lambdas_by_cell( ccx );
       
      int kwvln   = mwl_data.lambdas( ex_wvlns, ccx );
      std::sort(ex_wvlns.begin(), ex_wvlns.end());
DbgLv(1) << "rsoD: ccx kwv" << ccx << kwvln << ex_wvlns.size()
 << "nwv ccoff" << nwaveln << ccoff;

      for ( int wvo = 0; wvo < kwvln; wvo++ )
      {
         int iwavl   = ex_wvlns[ wvo ];
         int idatx   = ccoff + rawi_wvlns.indexOf( iwavl );
//DbgLv(1) << "rsoD:   wvo" << wvo << "wavl datx" << iwavl << idatx;
         outData << &allData[ idatx ];
      }

      ccoff      += nwaveln;
//DbgLv(1) << "rsoD:        ccx ccoff" << ccx << ccoff;
   }
DbgLv(1) << "rsoD:  final ccoff" << ccoff << "aDa size" << allData.size()
 << "oDa size" << outData.size();
}

// Get input wavelengths vector for the current cell
int US_Edit::lambdas_by_cell( int trx )
{
   int ccx     = ( trx < 0 ) ? triple_index : trx;
   nwaveln     = rawi_wvlns.count();

   if ( lrng_bycell )
   {
      rawi_wvlns = wavelns_i[ ccx ];
      // sort rawi_wvlns
      std::sort( rawi_wvlns.begin(), rawi_wvlns.end() );
      nwaveln    = rawi_wvlns.count();
      rawc_wvlns.clear();

      for ( int wvx = 0; wvx < nwaveln; wvx++ )
      {
         rawc_wvlns << QString::number( rawi_wvlns[ wvx ] );
      }
   }

   return nwaveln;
}

// Return the radius value at the radius index nearest a given radius
double US_Edit::radius_indexed( const double radi )
{
   return data.radius( data.xindex( radi ) );
}


// Create general data set information report file
void US_Edit::create_report_auto( QString& ss, int trx )
{
//*DEBUG*
US_DataIO::RawData* dd  = outData[ index_data_auto( trx ) ];
for (int js=0; js<dd->scanData.size(); js++)
{
DbgLv(1) << "CR: js" << js << "secs" << dd->scanData[js].seconds
 << "speed" << dd->scanData[js].rpm;
}
//*DEBUG*
   QString title = "US_Edit";
   QString head1 = tr( "General Data Set Information" );

   ss  = html_header_auto( title, head1, trx );
   ss += run_details_auto( trx );
   ss += scan_info_auto( trx );
   ss += indent( 2 ) + "</body>\n</html>\n";
}


// Create general data set information report file
void US_Edit::create_report( QString& ss )
{
//*DEBUG*
US_DataIO::RawData* dd  = outData[ index_data() ];
for (int js=0; js<dd->scanData.size(); js++)
{
DbgLv(1) << "CR: js" << js << "secs" << dd->scanData[js].seconds
 << "speed" << dd->scanData[js].rpm;
}
//*DEBUG*
   QString title = "US_Edit";
   QString head1 = tr( "General Data Set Information" );

   ss  = html_header( title, head1 );
   ss += run_details();
   ss += scan_info();
   ss += indent( 2 ) + "</body>\n</html>\n";
}

// View data set report
void US_Edit::view_report( void )
{
   QString rtext;

   // Create the General Data Info report text
   create_report( rtext );

   // Display the text in a text edit dialog
   US_Editor* tedit = new US_Editor( US_Editor::LOAD, true );
   tedit->setWindowTitle( tr( "Edit : General Data Information" ) );
   tedit->move( this->pos() + QPoint( 100, 100 ) );
   tedit->resize( 700, 600 );
   tedit->e->setFont( QFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize() ) );
   tedit->e->setText( rtext );
   tedit->show();
}

// Save data set report
void US_Edit::save_report_auto( const QString rtext, const QString rptfpath,
				const int idEdit, const int trx )
{

   // Write report text to the report file
   QFile f_rep( rptfpath );

   bool is_ok = f_rep.open( QIODevice::WriteOnly | QIODevice::Truncate );

   if ( ! is_ok )
   {
      qDebug() << "*ERROR* write open:" << rptfpath;
      return;
   }

   QTextStream ts( &f_rep );
   ts << rtext;
   f_rep.close();

   // Copy report to the database if required
   if ( dbP != NULL )
   {
      QStringList rfiles;
      QString pfdir    = QString( rptfpath ).section( "/",  0, -2 );
      QString tripdesc = outData[ index_data_auto( trx ) ]->description;
      rfiles << rptfpath;

      // Set the runID for the report
      US_Report freport;
      freport.runID    = runID;

      // Write the report record to the database
      int st = freport.saveFileDocuments_auto( idInv_auto.toInt(), pfdir,  rfiles, dbP,   //include invID explicitly
					       idEdit, tripdesc );

      if ( st != US_DB2::OK )
      {
         qDebug() << "*ERROR* saveFileDocuments, status" << st;
      }
   }
}


// Save data set report
void US_Edit::save_report( const QString rtext, const QString rptfpath,
                           const int idEdit )
{

   // Write report text to the report file
   QFile f_rep( rptfpath );

   bool is_ok = f_rep.open( QIODevice::WriteOnly | QIODevice::Truncate );

   if ( ! is_ok )
   {
      qDebug() << "*ERROR* write open:" << rptfpath;
      return;
   }

   QTextStream ts( &f_rep );
   ts << rtext;
   f_rep.close();

   // Copy report to the database if required
   if ( dbP != NULL )
   {
      QStringList rfiles;
      QString pfdir    = QString( rptfpath ).section( "/",  0, -2 );
      QString tripdesc = outData[ index_data() ]->description;
      rfiles << rptfpath;

      // Set the runID for the report
      US_Report freport;
      freport.runID    = runID;

      // Write the report record to the database
      int st = freport.saveFileDocuments( pfdir,  rfiles, dbP,
                                          idEdit, tripdesc );

      if ( st != US_DB2::OK )
      {
         qDebug() << "*ERROR* saveFileDocuments, status" << st;
      }
   }
}

// String to accomplish line identation
QString US_Edit::indent( const int spaces )
{
   return QString( " " ).leftJustified( spaces, ' ' );
}

// Table row HTML with 2 columns
QString US_Edit::table_row( const QString s1, const QString s2 )
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td></tr>\n" );
}

// Table row HTML with 3 columns
QString US_Edit::table_row( const QString s1, const QString s2, 
                            const QString s3 )
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>"
            + s3 + "</td></tr>\n" );
}

// Table row HTML with 5 columns
QString US_Edit::table_row( const QString s1, const QString s2, 
                            const QString s3, const QString s4, 
                            const QString s5 )
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>"
            + s3 + "</td><td>" + s4 + "</td><td>" + s5 + "</td></tr>\n" );
}

// Table row HTML with 6 columns
QString US_Edit::table_row( const QString s1, const QString s2, 
                            const QString s3, const QString s4, 
                            const QString s5, const QString s6 )
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>"
            + s3 + "</td><td>" + s4 + "</td><td>" + s5 + "</td><td>"
            + s6 + "</td></tr>\n" );
}

// Compose HTML header string
QString US_Edit::html_header( const QString title, const QString head1 )
{ 
   int         trx     = index_data();
   QString     triple  = triples.at( trx );
   QStringList parts   = triple.split( " / " );
   QString     cell    = parts[ 0 ];
   QString     channel = parts[ 1 ];
   QString     wvlen   = parts[ 2 ];
   QString     editID  = editIDs[ trx ];
   QString     editLbl = editLabel.isEmpty() ?
                         editFnames[ trx ].section( ".", -6, -6 ) : 
                         editLabel;

   QString ss = QString( "<?xml version=\"1.0\"?>\n" );
   ss  += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
   ss  += "                      \"http://www.w3.org/TR/xhtml1/DTD"
          "/xhtml1-strict.dtd\">\n";
   ss  += "<html xmlns=\"http://www.w3.org/1999/xhtml\""
          " xml:lang=\"en\" lang=\"en\">\n";
   ss  += "  <head>\n";
   ss  += "    <title> " + title + " </title>\n";
   ss  += "    <meta http-equiv=\"Content-Type\" content="
          "\"text/html; charset=iso-8859-1\"/>\n";
   ss  += "    <style type=\"text/css\" >\n";
   ss  += "      td { padding-right: 1em; }\n";
   ss  += "      body { background-color: white; }\n";
   ss  += "    </style>\n";
   ss  += "  </head>\n  <body>\n";
   ss  += "    <h1>" + head1 + "</h1>\n";
   ss  += indent( 4 ) + tr( "<h2>Data Report for Run \"" ) + runID;
   ss  += "\",<br/>\n" + indent( 4 ) + "&nbsp;" + tr( " Cell " ) + cell;
   ss  += tr( ", Channel " ) + channel;
   ss  += tr( ", Wavelength " ) + wvlen;
   ss  += ",<br/>\n" + indent( 4 ) + "&nbsp;" + tr( " Edited Dataset " );
   ss  += editLbl + "</h2>\n";
 
   return ss;
}

// Compose HTML header string
QString US_Edit::html_header_auto( const QString title, const QString head1, const int trx_curr )
{ 
   int         trx     = index_data_auto( trx_curr );
   QString     triple  = triples.at( trx );
   QStringList parts   = triple.split( " / " );
   QString     cell    = parts[ 0 ];
   QString     channel = parts[ 1 ];
   QString     wvlen   = parts[ 2 ];
   QString     editID  = editIDs[ trx ];
   QString     editLbl = editLabel.isEmpty() ?
                         editFnames[ trx ].section( ".", -6, -6 ) : 
                         editLabel;

   QString ss = QString( "<?xml version=\"1.0\"?>\n" );
   ss  += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
   ss  += "                      \"http://www.w3.org/TR/xhtml1/DTD"
          "/xhtml1-strict.dtd\">\n";
   ss  += "<html xmlns=\"http://www.w3.org/1999/xhtml\""
          " xml:lang=\"en\" lang=\"en\">\n";
   ss  += "  <head>\n";
   ss  += "    <title> " + title + " </title>\n";
   ss  += "    <meta http-equiv=\"Content-Type\" content="
          "\"text/html; charset=iso-8859-1\"/>\n";
   ss  += "    <style type=\"text/css\" >\n";
   ss  += "      td { padding-right: 1em; }\n";
   ss  += "      body { background-color: white; }\n";
   ss  += "    </style>\n";
   ss  += "  </head>\n  <body>\n";
   ss  += "    <h1>" + head1 + "</h1>\n";
   ss  += indent( 4 ) + tr( "<h2>Data Report for Run \"" ) + runID;
   ss  += "\",<br/>\n" + indent( 4 ) + "&nbsp;" + tr( " Cell " ) + cell;
   ss  += tr( ", Channel " ) + channel;
   ss  += tr( ", Wavelength " ) + wvlen;
   ss  += ",<br/>\n" + indent( 4 ) + "&nbsp;" + tr( " Edited Dataset " );
   ss  += editLbl + "</h2>\n";
 
   return ss;
}

QString US_Edit::run_details( void )
{
   US_DataIO::RawData* dd = outData[ index_data() ];
  
   int scsize = dd->scanData.size();
   // Temperature and raw speed data
   double sumte   =  0.0;
   double sumrs   =  0.0;
   double maxTemp = -1.0e99;
   double minTemp =  1.0e99;

   for ( int ii = 0; ii < scsize; ii++ )
   {
      double tt = dd->scanData[ ii ].temperature;
      sumte    += tt;
      maxTemp   = qMax( maxTemp, tt );
      minTemp   = qMin( minTemp, tt );
      sumrs    += dd->scanData[ ii ].rpm;
   }

   QString avgrspd = QString::number( sumrs / scsize, 'f', 1 );
   QString avgtemp = QString::number( sumte / scsize, 'f', 1 );
   QString ss = "\n" + indent( 4 )
        + tr( "<h3>Detailed Run Information:</h3>\n" )
        + indent( 4 ) + "<table>\n"
        + table_row( tr( "Cell Description:" ), dd->description )
        + table_row( tr( "Data Directory:"   ), workingDir )
        + table_row( tr( "Average Rotor Speed:" ), avgrspd + " rpm" )
        + table_row( tr( "Average Temperature:" ), avgtemp + " " + MLDEGC );

   if ( maxTemp - minTemp <= US_Settings::tempTolerance() )
      ss += table_row( tr( "Temperature Variation:" ),
                       tr( "Within tolerance" ) );
   else 
      ss += table_row( tr( "Temperature Variation:" ), 
                       tr( "(!) OUTSIDE TOLERANCE (!)" ) );

   // Time data
   double time_correction = US_Math2::time_correction( allData );
   int minutes = (int)time_correction / 60;
   int seconds = (int)time_correction % 60;

   QString mm  = ( minutes == 1 ) ? tr( " minute " ) : tr( " minutes " );
   QString sec = ( seconds == 1 ) ? tr( " second"  ) : tr( " seconds"  );

   ss += table_row( tr( "Time Correction:" ), 
                    QString::number( minutes ) + mm +
                    QString::number( seconds ) + sec );

   double duration = allData.last().scanData.last().seconds;

   int hours = (int) duration / 3600;
   minutes   = (int) duration / 60 - hours * 60;
   seconds   = (int) duration % 60;
   QString ddType = QString( dd->type ).left( 2 );
   QString                   dataType = tr( "Absorbance:" );
   if ( ddType == "RI" )     dataType = tr( "Intensity:" );
   if ( ddType == "WI" )     dataType = tr( "Intensity:" );
   if ( ddType == "IP" )     dataType = tr( "Interference:" );
   if ( ddType == "FI" )     dataType = tr( "Fluorescence:" );

   QString hh;
   hh  = ( hours   == 1 ) ? tr( " hour "   ) : tr( " hours " );
   mm  = ( minutes == 1 ) ? tr( " minute " ) : tr( " minutes " );
   sec = ( seconds == 1 ) ? tr( " second"  ) : tr( " seconds" );

   ss += table_row( tr( "Run Duration:" ),
                   QString::number( hours   ) + hh + 
                   QString::number( minutes ) + mm + 
                   QString::number( seconds ) + sec );

   // Wavelength, baseline, meniscus, range
   int    iwvln    = qRound( dd->scanData.last().wavelength );
   QString bln_od  = QString( le_baseline->text() ).section( "(", 1, 1 )
                     .section( ")", 0, 0 ) + " OD";
   QString left    = le_dataStart->text();
   QString right   = le_dataEnd  ->text();
   QString plat    = le_plateau->text();

   ss += table_row( tr( "Wavelength:" ),
                    QString::number( iwvln ) + " nm" ) + 
         table_row( tr( "Baseline " ) + dataType, bln_od );
         table_row( tr( "Meniscus Position:" ),
                    le_meniscus->text() + " cm" );


   ss += table_row( tr( "Edited Data starts at:"  ), left + " cm" ) +
         table_row( tr( "Edited Data stops at:"   ), right + " cm " ) +
         table_row( tr( "Plateau Position:"   ), plat + " cm" ); 

   ss += indent( 4 ) + "</table>\n";

   return ss;
}

QString US_Edit::run_details_auto( int trx )
{
   US_DataIO::RawData* dd = outData[ index_data_auto( trx ) ];
  
   int scsize = dd->scanData.size();
   // Temperature and raw speed data
   double sumte   =  0.0;
   double sumrs   =  0.0;
   double maxTemp = -1.0e99;
   double minTemp =  1.0e99;

   for ( int ii = 0; ii < scsize; ii++ )
   {
      double tt = dd->scanData[ ii ].temperature;
      sumte    += tt;
      maxTemp   = qMax( maxTemp, tt );
      minTemp   = qMin( minTemp, tt );
      sumrs    += dd->scanData[ ii ].rpm;
   }

   QString avgrspd = QString::number( sumrs / scsize, 'f', 1 );
   QString avgtemp = QString::number( sumte / scsize, 'f', 1 );
   QString ss = "\n" + indent( 4 )
        + tr( "<h3>Detailed Run Information:</h3>\n" )
        + indent( 4 ) + "<table>\n"
        + table_row( tr( "Cell Description:" ), dd->description )
        + table_row( tr( "Data Directory:"   ), workingDir )
        + table_row( tr( "Average Rotor Speed:" ), avgrspd + " rpm" )
        + table_row( tr( "Average Temperature:" ), avgtemp + " " + MLDEGC );

   if ( maxTemp - minTemp <= US_Settings::tempTolerance() )
      ss += table_row( tr( "Temperature Variation:" ),
                       tr( "Within tolerance" ) );
   else 
      ss += table_row( tr( "Temperature Variation:" ), 
                       tr( "(!) OUTSIDE TOLERANCE (!)" ) );

   // Time data
   double time_correction = US_Math2::time_correction( allData );
   int minutes = (int)time_correction / 60;
   int seconds = (int)time_correction % 60;

   QString mm  = ( minutes == 1 ) ? tr( " minute " ) : tr( " minutes " );
   QString sec = ( seconds == 1 ) ? tr( " second"  ) : tr( " seconds"  );

   ss += table_row( tr( "Time Correction:" ), 
                    QString::number( minutes ) + mm +
                    QString::number( seconds ) + sec );

   double duration = allData.last().scanData.last().seconds;

   int hours = (int) duration / 3600;
   minutes   = (int) duration / 60 - hours * 60;
   seconds   = (int) duration % 60;
   QString ddType = QString( dd->type ).left( 2 );
   QString                   dataType = tr( "Absorbance:" );
   if ( ddType == "RI" )     dataType = tr( "Intensity:" );
   if ( ddType == "WI" )     dataType = tr( "Intensity:" );
   if ( ddType == "IP" )     dataType = tr( "Interference:" );
   if ( ddType == "FI" )     dataType = tr( "Fluorescence:" );

   QString hh;
   hh  = ( hours   == 1 ) ? tr( " hour "   ) : tr( " hours " );
   mm  = ( minutes == 1 ) ? tr( " minute " ) : tr( " minutes " );
   sec = ( seconds == 1 ) ? tr( " second"  ) : tr( " seconds" );

   ss += table_row( tr( "Run Duration:" ),
                   QString::number( hours   ) + hh + 
                   QString::number( minutes ) + mm + 
                   QString::number( seconds ) + sec );

   // Wavelength, baseline, meniscus, range
   int    iwvln    = qRound( dd->scanData.last().wavelength );

   // QString bln_od  = QString( le_baseline->text() ).section( "(", 1, 1 )
   //                   .section( ")", 0, 0 ) + " OD";
   // QString left    = le_dataStart->text();
   // QString right   = le_dataEnd  ->text();
   // QString plat    = le_plateau->text();

   // Base parameters:
   QString triple_name_r = cb_triple->itemText( trx );
      
   QString meniscus_r      = editProfile[ triple_name_r ][0];
   QString range_left_r    = editProfile[ triple_name_r ][1];
   QString range_right_r   = editProfile[ triple_name_r ][2];
   QString plateau_r       = editProfile[ triple_name_r ][3];
   QString baseline_r      = editProfile[ triple_name_r ][4];
   QString baseline_od_r   = editProfile[ triple_name_r ][5];
   
   QString bln_od  = QString( baseline_od_r ) + " OD";
   QString left    = range_left_r ;
   QString right   = range_right_r;
   QString plat    = plateau_r;
   
   
   ss += table_row( tr( "Wavelength:" ),
                    QString::number( iwvln ) + " nm" ) + 
         table_row( tr( "Baseline " ) + dataType, bln_od ) +
         table_row( tr( "Meniscus Position:" ),
                    meniscus_r + " cm" );


   ss += table_row( tr( "Edited Data starts at:"  ), left + " cm" ) +
         table_row( tr( "Edited Data stops at:"   ), right + " cm " ) +
         table_row( tr( "Plateau Position:"   ), plat + " cm" ); 

   ss += indent( 4 ) + "</table>\n";

   return ss;
}


QString US_Edit::scan_info( void )
{
   US_DataIO::RawData* dd  = outData[ index_data() ];
   double time_correction  = US_Math2::time_correction( allData );

   QString ss = "\n" + indent( 4 ) + tr( "<h3>Scan Information:</h3>\n" )
               + indent( 4 ) + "<table>\n"; 
         
   ss += table_row( tr( "Scan" ), tr( "Corrected Time" ), 
                   tr( "Plateau Concentration" ),
                   tr( "Seconds" ), tr( "Omega^2T" ), tr( "Raw Speed" ) );

   for ( int ii = 0; ii < dd->scanData.size(); ii++ )
   {
      QString s1;
      QString s2;
      QString s3;
      QString s4;
      QString s5;
      QString s6;

      double time  = dd->scanData[ ii ].seconds;
      double omg2t = dd->scanData[ ii ].omega2t;
      double speed = dd->scanData[ ii ].rpm;
      int    ctime = (int)( dd->scanData[ ii ].seconds - time_correction ); 
      int    platx = US_DataIO::index( dd->xvalues, plateau );
      double od    = dd->scanData[ ii ].rvalues[ platx ];

      s1 = s1.sprintf( "%4d",             ii + 1 );
      s2 = s2.sprintf( "%4d min %2d sec", ctime / 60, ctime % 60 );
      s3 = s3.sprintf( "%.6f OD",         od ); 
      s4 = s4.sprintf( "%5d",             (int)time );
      s5 = s5.sprintf( "%.5e",            omg2t );
      s6 = s6.sprintf( "%.1f",            speed );

      ss += table_row( s1, s2, s3, s4, s5, s6 );
   }

   ss += indent( 4 ) + "</table>\n";
   
   return ss;
}

QString US_Edit::scan_info_auto( int trx )
{
   US_DataIO::RawData* dd  = outData[ index_data_auto( trx ) ];
   double time_correction  = US_Math2::time_correction( allData );

   QString ss = "\n" + indent( 4 ) + tr( "<h3>Scan Information:</h3>\n" )
               + indent( 4 ) + "<table>\n"; 
         
   ss += table_row( tr( "Scan" ), tr( "Corrected Time" ), 
                   tr( "Plateau Concentration" ),
                   tr( "Seconds" ), tr( "Omega^2T" ), tr( "Raw Speed" ) );

   for ( int ii = 0; ii < dd->scanData.size(); ii++ )
   {
      QString s1;
      QString s2;
      QString s3;
      QString s4;
      QString s5;
      QString s6;

      double time  = dd->scanData[ ii ].seconds;
      double omg2t = dd->scanData[ ii ].omega2t;
      double speed = dd->scanData[ ii ].rpm;
      int    ctime = (int)( dd->scanData[ ii ].seconds - time_correction ); 
      int    platx = US_DataIO::index( dd->xvalues, plateau );
      double od    = dd->scanData[ ii ].rvalues[ platx ];

      s1 = s1.sprintf( "%4d",             ii + 1 );
      s2 = s2.sprintf( "%4d min %2d sec", ctime / 60, ctime % 60 );
      s3 = s3.sprintf( "%.6f OD",         od ); 
      s4 = s4.sprintf( "%5d",             (int)time );
      s5 = s5.sprintf( "%.5e",            omg2t );
      s6 = s6.sprintf( "%.1f",            speed );

      ss += table_row( s1, s2, s3, s4, s5, s6 );
   }

   ss += indent( 4 ) + "</table>\n";
   
   return ss;
}

// Close edit after review of saved edits
void US_Edit::pass_values( void )
{
  //collect menicsus | airGap | baseline | range | etc data && pass to main edit stage

  QString triple_name = cb_triple->itemText( cb_triple->currentIndex() );
  
  QMap< QString, QStringList > editProfile_triple;
  QStringList triple_params;
  triple_params <<  QString::number(meniscus)
		<<  QString::number(range_left)
		<<  QString::number(range_right)
		<<  QString::number(plateau)
		<<  QString::number(baseline)
		<<  QString::number(baseline_od)
		<<  QString("spike_false");

  if ( dataType == "IP" )
    {
      triple_params <<  QString::number(airGap_left)
		    <<  QString::number(airGap_right);
    }
  
  editProfile_triple[ triple_name ] = triple_params;

  qDebug() << "In PASSING values manually: triple_name, parms -- " << triple_name << editProfile_triple[ triple_name ];

  int status = QMessageBox::information( this,
					 tr( "New Edit Parameters" ),
					 tr( "This will overwrite current edit parameters. "
					     "Do you want to proceed? " ),
					 tr( "&OK" ), tr( "&Cancel" ),
					 0, 0, 1 );
  
  if ( status != 0 ) return;
  
  emit pass_edit_params( editProfile_triple );
  close();
}


// Close edit after review of saved edits
void US_Edit::close_edit( void )
{
   int ntripls    = allData.size();
   int nunedit    = editFnames.count( QString( "none" ) );

   if ( nunedit > 0 )
   {  // Some triples unedited:  give option to cancel close
      QMessageBox mbox;
      QPushButton* pb_cancel;
      QPushButton* pb_doclose;
      QString msg      = tr( "Edits have not been saved for<br/>"
                             "%1 of %2 total triples.<br/></br/>"
                             "Do you wish to<br/>"
                             "&nbsp;&nbsp;"
                             "return to edit more (<b>Cancel</b>) or<br/>"
                             "&nbsp;&nbsp;"
                             "close with current state (<b>Close</b>)<br/>?" )
                         .arg( nunedit ).arg( ntripls );
      mbox.setIcon      ( QMessageBox::Question );
      mbox.setTextFormat( Qt::RichText );
      mbox.setText      ( msg );
      pb_cancel        = mbox.addButton( tr( "Cancel"   ),
                                         QMessageBox::RejectRole );
      pb_doclose       = mbox.addButton( tr( "Close" ),
                                         QMessageBox::AcceptRole );
      mbox.setDefaultButton( pb_doclose );
      mbox.exec();
      if ( mbox.clickedButton() == pb_cancel )
         return;
   }

   close();
}
