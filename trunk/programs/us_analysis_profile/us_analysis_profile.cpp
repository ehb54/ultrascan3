//! \file us_analysis_profile.cpp

#include "us_analysis_profile.h"
#include "us_table.h"
#include "us_license.h"
#include "us_sleep.h"
#include "us_util.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#define QRegularExpression(a)  QRegExp(a)
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif


//! \brief Main program for US_AnalysisProfile. Loads translators and starts
//         the class US_AnalysisProfile

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_AnalysisProfile * w = new US_AnalysisProfile;
   w->show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// Constructor:  build the main layout with tab widget panels
US_AnalysisProfile::US_AnalysisProfile() : US_Widgets()
{
   dbg_level    = US_Settings::us_debug();
   curr_panx    = 0;

   setWindowTitle( tr( "Define An Analysis Profile" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main      = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );
   QGridLayout* statL     = new QGridLayout();
   QHBoxLayout* buttL     = new QHBoxLayout();

   connection_status = false;
   automode = false;

      // Create tab and panel widgets
   tabWidget           = us_tabwidget();
   
   tabWidget->setTabPosition( QTabWidget::North );

DbgLv(1) << "MAIN: create panels";
   apanGeneral         = new US_AnaprofPanGen   ( this );
DbgLv(1) << "MAIN:  apGE done";
   apanEdit            = new US_AnaprofPanEdit  ( this );
DbgLv(1) << "MAIN:  apED done";
   apan2DSA            = new US_AnaprofPan2DSA  ( this );
DbgLv(1) << "MAIN:  ap2D done";
   apanPCSA            = new US_AnaprofPanPCSA  ( this );
DbgLv(1) << "MAIN:  apPC done";
   apanUpload          = new US_AnaprofPanUpload( this );
DbgLv(1) << "MAIN:  apUP done";
   statflag            = 0;

   // Add panels to the tab widget
   tabWidget->addTab( apanGeneral,   tr( "1: General" ) );
   tabWidget->addTab( apanEdit,      tr( "2: Edit"    ) );
   tabWidget->addTab( apan2DSA,      tr( "3: 2DSA"    ) );
   tabWidget->addTab( apanPCSA,      tr( "4: PCSA"    ) );
   tabWidget->addTab( apanUpload,    tr( "9: Upload"  ) );
   tabWidget->setCurrentIndex( curr_panx );
DbgLv(1) << "MAIN:  tabs added";

   //tabWidget->tabBar()->setEnabled(false);

   // Add bottom buttons
   //QPushButton* pb_close  = us_pushbutton( tr( "Close" ) );
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   pb_next   = us_pushbutton( tr( "Next Panel" ) );
   pb_prev   = us_pushbutton( tr( "Previous Panel" ) );
   pb_close  = us_pushbutton( tr( "Close" ) );;
   pb_apply  = us_pushbutton( tr( "Save/Apply" ) );;
   buttL->addWidget( pb_help  );
   buttL->addWidget( pb_prev  );
   buttL->addWidget( pb_next  );
   buttL->addWidget( pb_close );
   buttL->addWidget( pb_apply );

   // Connect signals to slots
   connect( tabWidget, SIGNAL( currentChanged( int ) ),
            this,      SLOT  ( newPanel      ( int ) ) );
   connect( pb_next,   SIGNAL( clicked()    ),
            this,      SLOT  ( panelUp()    ) );
   connect( pb_prev,   SIGNAL( clicked()    ),
            this,      SLOT  ( panelDown()  ) );
   connect( pb_close,  SIGNAL( clicked()    ),
            this,      SLOT  ( close_program()      ) );
//   connect( pb_apply,  SIGNAL( clicked()    ),
//            this,      SLOT  ( apply_profile()      ) );
   connect( pb_help,   SIGNAL( clicked()    ),
            this,      SLOT  ( help()       ) );

//   connect( apanUpload, SIGNAL( expdef_submitted( QMap < QString, QString > &) ),
//	    this,       SLOT  ( optima_submitted( QMap < QString, QString > & ) ) );
   

   main->addWidget( tabWidget );
   main->addLayout( statL );
   main->addLayout( buttL );

   connect( apanGeneral, SIGNAL( set_tabs_buttons_inactive( void ) ),
            this,        SLOT  ( disable_tabs_buttons     ( void ) ) );
   connect( apanGeneral, SIGNAL( set_tabs_buttons_active_readonly( void ) ),
            this,        SLOT  ( enable_tabs_buttons_readonly    ( void ) ) );
   connect( apanGeneral, SIGNAL( set_tabs_buttons_active  ( void ) ),
            this,        SLOT  ( enable_tabs_buttons      ( void ) ) );

   //int min_width = tabWidget->tabBar()->width();

   //setMinimumSize( QSize( min_width, 450 ) );
   setMinimumSize( 950, 450 );
   adjustSize();

   //apanGeneral->initPanel();
   //apanGeneral->loaded_proto = 0;
   //apanGeneral->update_inv();
DbgLv(1) << "MAIN:  CALL check_user_level()";
   apanGeneral->check_user_level();
DbgLv(1) << "MAIN:  CALL check_runname()";
   apanGeneral->check_runname();

DbgLv(1) << "MAIN:  CALL reset()";
   reset();
   resize( 500, 450 );
}


// Reset parameters to their defaults
void US_AnalysisProfile::reset( void )
{
}

// Reset parameters to their defaults
void US_AnalysisProfile::close_program( void )
{
  
    
  emit us_exp_is_closed();
  close();
}

// Add widgets to a grid layout row to set even 12-column spacing
void US_AnalysisProfile::addColumnSpacing( QGridLayout* genL, int& row )
{
   // Blank text boxes to enforce column spacing
   QLineEdit* le_1 = us_lineedit( " ", 0, true );
   QLineEdit* le_2 = us_lineedit( " ", 0, true );
   QLineEdit* le_3 = us_lineedit( " ", 0, true );
   QLineEdit* le_4 = us_lineedit( " ", 0, true );
   QLineEdit* le_5 = us_lineedit( " ", 0, true );
   QLineEdit* le_6 = us_lineedit( " ", 0, true );
   QLineEdit* le_7 = us_lineedit( " ", 0, true );
   QLineEdit* le_8 = us_lineedit( " ", 0, true );
   QLineEdit* le_9 = us_lineedit( " ", 0, true );
   QLineEdit* le_a = us_lineedit( " ", 0, true );
   QLineEdit* le_b = us_lineedit( " ", 0, true );
   QLineEdit* le_c = us_lineedit( " ", 0, true );
   // Disallow setting focus to any of the boxes
   le_1->setFocusPolicy( Qt::NoFocus );
   le_2->setFocusPolicy( Qt::NoFocus );
   le_3->setFocusPolicy( Qt::NoFocus );
   le_4->setFocusPolicy( Qt::NoFocus );
   le_5->setFocusPolicy( Qt::NoFocus );
   le_6->setFocusPolicy( Qt::NoFocus );
   le_7->setFocusPolicy( Qt::NoFocus );
   le_8->setFocusPolicy( Qt::NoFocus );
   le_9->setFocusPolicy( Qt::NoFocus );
   le_a->setFocusPolicy( Qt::NoFocus );
   le_b->setFocusPolicy( Qt::NoFocus );
   le_c->setFocusPolicy( Qt::NoFocus );

   // Create a grid layout row
   genL->addWidget( le_1,            row,    0, 1,  1 );
   genL->addWidget( le_2,            row,    1, 1,  1 );
   genL->addWidget( le_3,            row,    2, 1,  1 );
   genL->addWidget( le_4,            row,    3, 1,  1 );
   genL->addWidget( le_5,            row,    4, 1,  1 );
   genL->addWidget( le_6,            row,    5, 1,  1 );
   genL->addWidget( le_7,            row,    6, 1,  1 );
   genL->addWidget( le_8,            row,    7, 1,  1 );
   genL->addWidget( le_9,            row,    8, 1,  1 );
   genL->addWidget( le_a,            row,    9, 1,  1 );
   genL->addWidget( le_b,            row,   10, 1,  1 );
   genL->addWidget( le_c,            row++, 11, 1,  1 );
}

// Panel for run and other general parameters
US_AnaprofPanGen::US_AnaprofPanGen( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_AnalysisProfile*)topw;
   dbg_level           = US_Settings::us_debug();
   use_db              = ( US_Settings::default_data_location() < 2 );
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "1: Specify OD range and other general parameters" ) );
   panel->addWidget( lb_panel );

   // Create layout and GUI components
   QGridLayout* genL    = new QGridLayout();

   pb_aproname     = us_pushbutton( tr( "Analysis Profile Name" ) );
   pb_protname     = us_pushbutton( tr( "Protocol Name" ) );
//   QLabel* lb_loadconc  = us_label( tr( "Loading Concentration Ratio:" ) );
//   QLabel* lb_lctoler   = us_label( tr( "+/- Ratio Tolerance:" ) );
//   int     ihgt         = pb_aproname->height();
//   QSpacerItem* spacer1 = new QSpacerItem( 20, ihgt );

   // Set up line edits
   le_aproname     = us_lineedit( "(default)", 0, false );
   le_protname     = us_lineedit( "Test-1234", 0, false );
//   le_loadconc     = us_lineedit( "0.800", 0, false );
//   le_lctoler      = us_lineedit( "0.500", 0, false );

   // Set defaults
   currProf        = &mainw->currProf;

   // Start building main layout
   int row         = 0;
   genL->addWidget( pb_aproname,     row,   0, 1, 2 );
   genL->addWidget( le_aproname,     row++, 2, 1, 6 );
   genL->addWidget( pb_protname,     row,   0, 1, 2 );
   genL->addWidget( le_protname,     row++, 2, 1, 6 );

   // Build channel lists and rows
#if 1
   sl_chnsel << tr( "1A: UV/visible (solution 1)" )
             << tr( "2A: UV/visible (solution 2)" )
             << tr( "3A: Interference (solution 3)" );
#endif
   int nchn        = sl_chnsel.count();
   lb_chns  .clear();
   lb_lcrats.clear();
   lb_lctols.clear();
   le_lcrats.clear();
   le_lctols.clear();
   pb_aplya .clear();

   for ( int ii = 0; ii < nchn; ii++ )
   {
      lb_chns   << us_banner( sl_chnsel[ ii ] );
      lb_lcrats << us_label( tr( "Loading Concentration Ratio:" ) );
      lb_lctols << us_label( tr( "+/- Ratio Tolerance:" ) );
      le_lcrats << us_lineedit( "0.800", 0, false );
      le_lctols << us_lineedit( "0.500", 0, false );
      pb_aplya  << us_pushbutton( tr( "Apply to All" ) );

      genL->addWidget( lb_chns  [ ii ], row++, 1, 1, 7 );
      genL->addWidget( lb_lcrats[ ii ], row,   0, 1, 2 );
      genL->addWidget( le_lcrats[ ii ], row,   2, 1, 1 );
      genL->addWidget( lb_lctols[ ii ], row,   3, 1, 2 );
      genL->addWidget( le_lctols[ ii ], row,   5, 1, 1 );
      genL->addWidget( pb_aplya [ ii ], row++, 6, 1, 2 );
   }

#if 0
   // Build channel profile rows
   genL->addWidget( lb_loadconc,     row,   1, 1, 2 );
   genL->addWidget( le_loadconc,     row,   3, 1, 1 );
   genL->addWidget( lb_lctoler,      row,   4, 1, 2 );
   genL->addWidget( le_lctoler,      row++, 6, 1, 1 );
#endif

//   genL->addItem  ( spacer1,         row++, 6, 1, 2 );
   
   panel->addLayout( genL );
   panel->addStretch();

   // Set up signal and slot connections
//   connect( le_runid,        SIGNAL( textEdited(const QString &)  ),
//	    this,            SLOT(   check_empty_runname(const QString &) ) );


 //check_runname();
 
   // Do the initialization we do at panel entry
DbgLv(1) << "APGe: CALL initPanel()";
   initPanel();
DbgLv(1) << "APGe:  RTN initPanel()";
}

int US_AnaprofPanGen::getProfiles( QStringList& prnames,
      QList< QStringList >& prentries )
{
DbgLv(1) << "APGe: getProtos IN";
//   prnames         = pr_names;   // Return a names list copy
//   prentries       = protdata;   // Return an entries list copy
DbgLv(1) << "APGe:  gP: prnames count" << prnames.count()
 << "prdat count" << prentries.count();
   return prnames.count();       // Return the current list count

}

bool US_AnaprofPanGen::updateProfiles( const QStringList )
{
   return true;
}

// Panel for Lab/Rotor parameters
US_AnaprofPanEdit::US_AnaprofPanEdit( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
DbgLv(1) << "APEd: IN";
   mainw                = (US_AnalysisProfile*)topw;
   dbg_level            = US_Settings::us_debug();
   QVBoxLayout* panel   = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel     = us_banner( tr( "2: Specify Edit parameters" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL    = new QGridLayout();

#if 0
   QLabel* lb_loadvol   = us_label( tr( "Loading Volume (ml):" ) );
   QLabel* lb_voltoler  = us_label( tr( "Volume Tolerance (+/-,ml):" ) );
   QLabel* lb_dataend   = us_label( tr( "Data End (nm):" ) );
//   QPushButton* pb_advrotor = us_pushbutton( tr( "Advanced Lab/Rotor/Calibration" ) );
   int ihgt             = lb_loadvol->height();
   QSpacerItem* spacer1 = new QSpacerItem( 20, ihgt );
#endif
#if 0
   le_loadvol      = us_lineedit( "460", 0, false );
   le_voltoler     = us_lineedit( "10", 0, false );
   le_dataend      = us_lineedit( "7.00", 0, false );
#endif


//   QLabel*      lb_optima_banner    = us_banner( tr( "Select Optima Machine, Operator and Experiment Type " ) );
   sl_chnsel   = sibLValue( "general", "channels" );
#if 0
   sl_chnsel << tr( "1A: UV/visible (solution 1)" )
             << tr( "2A: UV/visible (solution 2)" )
             << tr( "3A: Interference (solution 3)" );
#endif
   int nchn        = sl_chnsel.count();
   int row     = 0;

   for ( int ii = 0; ii < nchn; ii++ )
   {
      lb_chns   << us_banner( sl_chnsel[ ii ] );
      lb_ldvols << us_label( tr( "Loading Volume (ml):" ) );
      lb_lvtols << us_label( tr( "Volume Tolerance (+/-,ml):" ) );
      lb_daends << us_label( tr( "Data End (nm):" ) );
      le_ldvols << us_lineedit( "460", 0, false );
      le_lvtols << us_lineedit( "10", 0, false );
      le_daends << us_lineedit( "7.00", 0, false );
      pb_aplya  << us_pushbutton( tr( "Apply to All" ) );

      genL->addWidget( lb_chns  [ ii ], row++,  1, 1, 8 );
      genL->addWidget( lb_ldvols[ ii ], row,    0, 1, 3 );
      genL->addWidget( le_ldvols[ ii ], row,    3, 1, 1 );
      genL->addWidget( lb_lvtols[ ii ], row,    4, 1, 3 );
      genL->addWidget( le_lvtols[ ii ], row,    7, 1, 1 );
      genL->addWidget( lb_daends[ ii ], row,    8, 1, 2 );
      genL->addWidget( le_daends[ ii ], row,   10, 1, 1 );
      genL->addWidget( pb_aplya [ ii ], row++, 11, 1, 1 );
   }
#if 0
   int row     = 0;
   genL->addWidget( lb_loadvol,      row,    0, 1, 3 );
   genL->addWidget( le_loadvol,      row,    3, 1, 1 );
   genL->addWidget( lb_voltoler,     row,    4, 1, 3 );
   genL->addWidget( le_voltoler,     row,    7, 1, 1 );
   genL->addWidget( lb_dataend,      row,    8, 1, 3 );
   genL->addWidget( le_dataend,      row++, 11, 1, 1 );
   genL->addItem  ( spacer1,         row++,  0, 1, 4 );

   row++;

   genL->addItem  ( spacer1,         row++,  0, 1, 12 );
#endif
DbgLv(1) << "APEd: JJ";
   mainw->addColumnSpacing( genL, row );
   
   panel->addLayout( genL );
   panel->addStretch();


//   connect( cb_lab,       SIGNAL( activated   ( int ) ),
//            this,         SLOT  ( changeLab   ( int ) ) );

   first_time_init = true;
DbgLv(1) << "APEd: MM";
   savePanel();
DbgLv(1) << "APEd: NN";
//   changed             = false;

   initPanel();
DbgLv(1) << "APEd: OO";

   first_time_init = false;
DbgLv(1) << "APEd: ZZ";
}


// Panel for 2DSA parameters
US_AnaprofPan2DSA::US_AnaprofPan2DSA( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_AnalysisProfile*)topw;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "3: Specify 2DSA Analysis Controls" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   // Labels and buttons
   QLabel*  lb_chnpro  = us_banner( tr( "Per-Channel Profile" ) );
   QLabel*  lb_chnsel  = us_label ( tr( "Channel [ Chn: Opt (Solut) ]" ) );
   QLabel*  lb_smin    = us_label ( tr( "s Minimum:         " ) );
   QLabel*  lb_smax    = us_label ( tr( "s Maximum:         " ) );
   QLabel*  lb_sgrpts  = us_label ( tr( "s Grid Points:     " ) );
   QLabel*  lb_kmin    = us_label ( tr( "f/f0 Minimum:      " ) );
   QLabel*  lb_kmax    = us_label ( tr( "f/f0 Maximum:      " ) );
   QLabel*  lb_kgrpts  = us_label ( tr( "f/f0 Grid Points:  " ) );
   QLabel*  lb_varyvb  = us_label ( tr( "Varying Vbar:      " ) );
   QLabel*  lb_constk  = us_label ( tr( "Constant f/f0:     " ) );
   QLabel*  lb_grreps  = us_label ( tr( "Grid Repetitions:  " ) );
            pb_custmg  = us_pushbutton( tr( "Custom Grid" ) );;
            pb_applya  = us_pushbutton( tr( "Apply to All" ) );;

   QLabel*  lb_jflow   = us_banner( tr( "2DSA Job Flow"   ) );
   QLabel*  lb_sumry   = us_label ( tr( "Flow Summary:   "
                                        "2DSA, 2DSA-FM, FITMEN,"
                                        " 2DSA-IT, 2DSA-MC" ) );
   QLabel*  lb_jname1  = us_label ( tr( "2DSA" ) );
   QLabel*  lb_jname2  = us_label ( tr( "2DSA-FM" ) );
   QLabel*  lb_jname3  = us_label ( tr( "FITMEN" ) );
   QLabel*  lb_jname4  = us_label ( tr( "2DSA-IT" ) );
   QLabel*  lb_jname5  = us_label ( tr( "2DSA-MC" ) );
   QLabel*  lb_j1nois  = us_label ( tr( "(TI noise)" ) );
   QLabel*  lb_j2nois  = us_label ( tr( "(TI+RI noise)" ) );
   QLabel*  lb_j4nois  = us_label ( tr( "(TI+RI noise)" ) );
   QLabel*  lb_j2gpts  = us_label ( tr( "Meniscus Grid Points" ) );
   QLabel*  lb_j2mrng  = us_label ( tr( "Meniscus Fit Range (cm)" ) );
   QLabel*  lb_j4iter  = us_label ( tr( "Refinement Iterations" ) );
   QLabel*  lb_j5iter  = us_label ( tr( "Monte-Carlo Iterations" ) );

//   int ihgt            = lb_smin->height();
//   QSpacerItem* spacer1 = new QSpacerItem( 20, ihgt );

   // Text boxes and other value elements
   le_smin         = us_lineedit( "1", 0, false );
   le_smax         = us_lineedit( "10", 0, false );
   le_sgrpts       = us_lineedit( "64", 0, false );
   le_kmin         = us_lineedit( "1", 0, false );
   le_kmax         = us_lineedit( "5", 0, false );
   le_kgrpts       = us_lineedit( "64", 0, false );
   le_grreps       = us_lineedit( "8", 0, false );
   le_custmg       = us_lineedit( "(none)", 0, false );
   ck_varyvb       = new QCheckBox( "VV", this );
   ck_varyvb->setPalette( US_GuiSettings::normalColor() );
   ck_varyvb->setChecked( false );
   ck_varyvb->setAutoFillBackground( true  );
   le_constk       = us_lineedit( "2", 0, false );

   cb_chnsel       = new QComboBox( this );
   sl_chnsel       = sibLValue( "general", "channels" );
   cb_chnsel->addItems( sl_chnsel );
   ck_j1run        = new QCheckBox( "Run", this );
   ck_j1run ->setPalette( US_GuiSettings::normalColor() );
   ck_j1run ->setChecked( true );
   ck_j1run ->setAutoFillBackground( true  );
   ck_j2run        = new QCheckBox( "Run", this );
   ck_j2run ->setPalette( US_GuiSettings::normalColor() );
   ck_j2run ->setChecked( true );
   ck_j2run ->setAutoFillBackground( true  );
   ck_j3run        = new QCheckBox( "Run", this );
   ck_j3run ->setPalette( US_GuiSettings::normalColor() );
   ck_j3run ->setChecked( true );
   ck_j3run ->setAutoFillBackground( true  );
   ck_j4run        = new QCheckBox( "Run", this );
   ck_j4run ->setPalette( US_GuiSettings::normalColor() );
   ck_j4run ->setChecked( true );
   ck_j4run ->setAutoFillBackground( true  );
   ck_j5run        = new QCheckBox( "Run", this );
   ck_j5run ->setPalette( US_GuiSettings::normalColor() );
   ck_j5run ->setChecked( true );
   ck_j5run ->setAutoFillBackground( true  );
   le_j2gpts       = us_lineedit( "10", 0, false );
   le_j2mrng       = us_lineedit( "0.03", 0, false );
   ck_j3auto       = new QCheckBox( "Auto-pick", this );
   ck_j3auto->setPalette( US_GuiSettings::normalColor() );
   ck_j3auto->setChecked( true );
   ck_j3auto->setAutoFillBackground( true  );
   le_j4iter       = us_lineedit( "10", 0, false );
   le_j5iter       = us_lineedit( "100", 0, false );

   // Create main layout rows
   int row     = 0;
   genL->addWidget( lb_chnpro,  row++,  0, 1, 12 );
   genL->addWidget( lb_chnsel,  row,    0, 1,  3 );
   genL->addWidget( cb_chnsel,  row++,  3, 1,  7 );
   genL->addWidget( lb_smin,    row,    0, 1,  3 );
   genL->addWidget( le_smin,    row,    3, 1,  1 );
   genL->addWidget( lb_smax,    row,    4, 1,  3 );
   genL->addWidget( le_smax,    row,    7, 1,  1 );
   genL->addWidget( lb_sgrpts,  row,    8, 1,  3 );
   genL->addWidget( le_sgrpts,  row++, 11, 1,  1 );
   genL->addWidget( lb_kmin,    row,    0, 1,  3 );
   genL->addWidget( le_kmin,    row,    3, 1,  1 );
   genL->addWidget( lb_kmax,    row,    4, 1,  3 );
   genL->addWidget( le_kmax,    row,    7, 1,  1 );
   genL->addWidget( lb_kgrpts,  row,    8, 1,  3 );
   genL->addWidget( le_kgrpts,  row++, 11, 1,  1 );
   genL->addWidget( lb_grreps,  row,    0, 1,  3 );
   genL->addWidget( le_grreps,  row,    3, 1,  1 );
   genL->addWidget( pb_custmg,  row,    4, 1,  3 );
   genL->addWidget( le_custmg,  row++,  7, 1,  5 );
   genL->addWidget( lb_varyvb,  row,    0, 1,  3 );
   genL->addWidget( ck_varyvb,  row,    3, 1,  1 );
   genL->addWidget( lb_constk,  row,    4, 1,  3 );
   genL->addWidget( le_constk,  row,    7, 1,  1 );
   genL->addWidget( pb_applya,  row++, 10, 1,  2 );
//   genL->addItem  ( spacer1,         row++,  0, 1, 12 );

   genL->addWidget( lb_jflow,   row++,  0, 1, 12 );
   genL->addWidget( lb_sumry,   row++,  0, 1, 12 );
   genL->addWidget( lb_jname1,  row,    0, 1,  2 );
   genL->addWidget( ck_j1run,   row,    2, 1,  1 );
   genL->addWidget( lb_j1nois,  row++,  3, 1,  2 );
   genL->addWidget( lb_jname2,  row,    0, 1,  2 );
   genL->addWidget( ck_j2run,   row,    2, 1,  1 );
   genL->addWidget( lb_j2nois,  row,    3, 1,  2 );
   genL->addWidget( lb_j2gpts,  row,    5, 1,  2 );
   genL->addWidget( le_j2gpts,  row,    7, 1,  1 );
   genL->addWidget( lb_j2mrng,  row,    8, 1,  2 );
   genL->addWidget( le_j2mrng,  row++, 10, 1,  1 );
   genL->addWidget( lb_jname3,  row,    0, 1,  2 );
   genL->addWidget( ck_j3run,   row,    2, 1,  1 );
   genL->addWidget( ck_j3auto,  row++,  5, 1,  2 );
   genL->addWidget( lb_jname4,  row,    0, 1,  2 );
   genL->addWidget( ck_j4run,   row,    2, 1,  1 );
   genL->addWidget( lb_j4nois,  row,    3, 1,  2 );
   genL->addWidget( lb_j4iter,  row,    5, 1,  2 );
   genL->addWidget( le_j4iter,  row++,  7, 1,  1 );
   genL->addWidget( lb_jname5,  row,    0, 1,  2 );
   genL->addWidget( ck_j5run,   row,    2, 1,  1 );
   genL->addWidget( lb_j5iter,  row,    5, 1,  2 );
   genL->addWidget( le_j5iter,  row++,  7, 1,  1 );

   mainw->addColumnSpacing( genL, row );
   // Connect signals and slots
//   connect( sb_scnint_ss, SIGNAL( valueChanged   ( int ) ),
//            this,         SLOT  ( ssChgScIntTime_ss ( int ) ) ); 

   
DbgLv(1) << "AP2d: addWidg/Layo II";

   // Complete overall layout
   panel->addLayout( genL );
   panel->addStretch();
   adjustSize();

DbgLv(1) << "AP2d: CALL initPanel()";
   initPanel();
DbgLv(1) << "AP2d:  RTN initPanel()";

//qDebug() << "SCANINT: " << ssvals[ 0 ][ "scanintv" ]  << ", SCANINT FROM rpSpeed: " <<  rpSpeed->ssteps[ 0 ].scanintv;
}

// Panel for PCSA parameters
US_AnaprofPanPCSA::US_AnaprofPanPCSA( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
DbgLv(1) << "APpc: IN";
   mainw               = (US_AnalysisProfile*)topw;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "4: Define PCSA Analysis Controls" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   QLabel* lb_curvtype = us_label ( tr( "Curve Type:" ) );
   QLabel* lb_xaxistyp = us_label ( tr( "X Axis Type:" ) );
   QLabel* lb_yaxistyp = us_label ( tr( "Y Axis Type:" ) );
   QLabel* lb_zaxistyp = us_label ( tr( "Z Axis Type:" ) );
   QLabel* lb_zvalue   = us_label ( tr( "Z Value:" ) );
   QLabel* lb_xmin     = us_label ( tr( "X Minimum:" ) );
   QLabel* lb_xmax     = us_label ( tr( "X Maximum:" ) );
   QLabel* lb_ymin     = us_label ( tr( "Y Minimum:" ) );
   QLabel* lb_ymax     = us_label ( tr( "Y Maximum:" ) );
   QLabel* lb_varcount = us_label ( tr( "Variations Count:" ) );
   QLabel* lb_grfiters = us_label ( tr( "Grid Fit Iterations:" ) );
   QLabel* lb_crpoints = us_label ( tr( "Curve Resolution Points:" ) );
   QLabel* lb_regparam = us_label ( tr( "Regularization Parameter:" ) );
   QLabel* lb_mciters  = us_label ( tr( "Monte-Carlo Iterations:" ) );
   QLabel* lb_tinoise  = us_label ( tr( "Fit Time-Invariant Noise:" ) );
   QLabel* lb_rinoise  = us_label ( tr( "Fit Radially-Invariant Noise:" ) );
   QLabel* lb_chnsel   = us_label ( tr( "Channel [ Chn: Opt (Solut) ]" ) );
   QLabel* lb_tregtype = us_label ( tr( "Tikhonov Regularization:" ) );
           pb_applya   = us_pushbutton( tr( "Apply to All" ) );;

   QStringList sl_curvtype;
   sl_curvtype << tr( "Increasing Sigmoid" )
               << tr( "Decreasing Sigmoid" )
               << tr( "Straight Line" )
               << tr( "Horizontal Line [ C(s) ]" )
               << tr( "Second Order Power Law" );
   QStringList sl_axistype;
   sl_axistype << "s" << "f/f0" << "mw" << "vbar" << "D";
   QStringList sl_zaxistyp;
   sl_zaxistyp << "vbar" << "f/f0" << "mw";

   cb_curvtype     = new QComboBox( this );
   cb_curvtype->addItems( sl_curvtype );
   cb_xaxistyp     = new QComboBox( this );
   cb_xaxistyp->addItems( sl_axistype );
   le_xmin         = us_lineedit( "1", 0, false );
   le_xmax         = us_lineedit( "10", 0, false );
   cb_yaxistyp     = new QComboBox( this );
   cb_yaxistyp->addItems( sl_axistype );
   cb_yaxistyp->setCurrentIndex( 1 );
   le_ymin         = us_lineedit( "1", 0, false );
   le_ymax         = us_lineedit( "5", 0, false );
   cb_zaxistyp     = new QComboBox( this );
   cb_zaxistyp->addItems( sl_zaxistyp );
   le_zvalue       = us_lineedit( "0.732", 0, false );
   le_varcount     = us_lineedit( "6", 0, false );
   le_grfiters     = us_lineedit( "3", 0, false );
   le_crpoints     = us_lineedit( "200", 0, false );
   ck_tregoff      = new QCheckBox( "Off", this );
   ck_tregoff ->setPalette( US_GuiSettings::normalColor() );
   ck_tregoff ->setChecked( true );
   ck_tregoff ->setAutoFillBackground( true  );
   ck_tregspec     = new QCheckBox( "On-specified", this );
   ck_tregspec->setPalette( US_GuiSettings::normalColor() );
   ck_tregspec->setChecked( false );
   ck_tregspec->setAutoFillBackground( true  );
   ck_tregauto     = new QCheckBox( "On-auto", this );
   ck_tregauto->setPalette( US_GuiSettings::normalColor() );
   ck_tregauto->setChecked( false );
   ck_tregauto->setAutoFillBackground( true  );
   le_regparam     = us_lineedit( "0", 0, false );
   le_mciters      = us_lineedit( "0", 0, false );
   ck_tinoise      = new QCheckBox( "TI", this );
   ck_tinoise ->setPalette( US_GuiSettings::normalColor() );
   ck_tinoise ->setChecked( false );
   ck_tinoise ->setAutoFillBackground( true  );
   ck_rinoise      = new QCheckBox( "RI", this );
   ck_rinoise ->setPalette( US_GuiSettings::normalColor() );
   ck_rinoise ->setChecked( false );
   ck_rinoise ->setAutoFillBackground( true  );
   QLabel*  lb_chnpro  = us_banner( tr( "Per-Channel Profile" ) );
   cb_chnsel       = new QComboBox( this );
   sl_chnsel       = sibLValue( "general", "channels" );
   cb_chnsel->addItems( sl_chnsel );
//   int ihgt            = lb_curvtype->height();
//   QSpacerItem* spacer1 = new QSpacerItem( 20, ihgt );

   // Create main layout rows
   int row             = 0;
   genL->addWidget( lb_chnpro,   row++,  0, 1, 12 );
   genL->addWidget( lb_chnsel,   row,    0, 1,  3 );
   genL->addWidget( cb_chnsel,   row++,  3, 1,  7 );
   genL->addWidget( lb_curvtype, row,    0, 1,  3 );
   genL->addWidget( cb_curvtype, row,    3, 1,  3 );
   genL->addWidget( pb_applya,   row++, 10, 1,  2 );
   genL->addWidget( lb_xaxistyp, row,    0, 1,  3 );
   genL->addWidget( cb_xaxistyp, row,    3, 1,  1 );
   genL->addWidget( lb_xmin,     row,    4, 1,  3 );
   genL->addWidget( le_xmin,     row,    7, 1,  1 );
   genL->addWidget( lb_xmax,     row,    8, 1,  3 );
   genL->addWidget( le_xmax,     row++, 11, 1,  1 );
   genL->addWidget( lb_yaxistyp, row,    0, 1,  3 );
   genL->addWidget( cb_yaxistyp, row,    3, 1,  1 );
   genL->addWidget( lb_ymin,     row,    4, 1,  3 );
   genL->addWidget( le_ymin,     row,    7, 1,  1 );
   genL->addWidget( lb_ymax,     row,    8, 1,  3 );
   genL->addWidget( le_ymax,     row++, 11, 1,  1 );
   genL->addWidget( lb_zaxistyp, row,    0, 1,  3 );
   genL->addWidget( cb_zaxistyp, row,    3, 1,  1 );
   genL->addWidget( lb_zvalue,   row,    4, 1,  3 );
   genL->addWidget( le_zvalue,   row,    7, 1,  1 );
   genL->addWidget( lb_varcount, row,    8, 1,  3 );
   genL->addWidget( le_varcount, row++, 11, 1,  1 );
   genL->addWidget( lb_grfiters, row,    0, 1,  3 );
   genL->addWidget( le_grfiters, row,    3, 1,  1 );
   genL->addWidget( lb_crpoints, row,    4, 1,  3 );
   genL->addWidget( le_crpoints, row++,  7, 1,  1 );
   genL->addWidget( lb_tregtype, row,    0, 1,  3 );
   genL->addWidget( ck_tregoff,  row,    3, 1,  1 );
   genL->addWidget( ck_tregspec, row,    4, 1,  2 );
   genL->addWidget( ck_tregauto, row,    6, 1,  2 );
   genL->addWidget( lb_regparam, row,    8, 1,  3 );
   genL->addWidget( le_regparam, row++, 11, 1,  1 );
   genL->addWidget( lb_mciters,  row,    0, 1,  3 );
   genL->addWidget( le_mciters,  row,    3, 1,  1 );
   genL->addWidget( lb_tinoise,  row,    4, 1,  3 );
   genL->addWidget( ck_tinoise,  row,    7, 1,  1 );
   genL->addWidget( lb_rinoise,  row,    8, 1,  3 );
   genL->addWidget( ck_rinoise,  row++, 11, 1,  1 );

   mainw->addColumnSpacing( genL, row );
   panel->addLayout( genL );
   panel->addStretch();

   // Do first pass at initializing the panel layout
   initPanel();
QString pval1 = sibSValue( "rotor", "rotor" );
DbgLv(1) << "APpc: rotor+rotor=" << pval1;
}

// Panel for Upload parameters
US_AnaprofPanUpload::US_AnaprofPanUpload( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
DbgLv(1) << "APup: IN";
   mainw               = (US_AnalysisProfile*)topw;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "9: Define Upload Controls" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();


   QLabel* lb_hdr1     = us_banner( tr( "Status" ) );
   QLabel* lb_hdr2     = us_banner( tr( "Upload" ) );
   int row             = 0;
   genL->addWidget( lb_hdr1, row++, 0, 1, 2 );
   genL->addWidget( lb_hdr2, row++, 0, 1, 2 );


   panel->addLayout( genL );
   panel->addStretch();

   // Do first pass at initializing the panel layout
   initPanel();
QString pval1 = sibSValue( "rotor", "rotor" );
DbgLv(1) << "APpc: rotor+rotor=" << pval1;
}

void US_AnaprofPanUpload::submitExperiment( void )
{
   return;
}
void US_AnaprofPanUpload::submitExperiment_confirm( void )
{
   return;
}
void US_AnaprofPanUpload::testConnection( void )
{
   return;
}
void US_AnaprofPanUpload::detailExperiment( void )
{
   return;
}

