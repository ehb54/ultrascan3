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
   pb_close = us_pushbutton( tr( "Close" ) );;
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   // QPushButton* pb_next   = us_pushbutton( tr( "Next Panel" ) );
   // QPushButton* pb_prev   = us_pushbutton( tr( "Previous Panel" ) );
   pb_next   = us_pushbutton( tr( "Next Panel" ) );
   pb_prev   = us_pushbutton( tr( "Previous Panel" ) );
   buttL->addWidget( pb_help  );
   buttL->addWidget( pb_prev  );
   buttL->addWidget( pb_next  );
   buttL->addWidget( pb_close );

   // Connect signals to slots
   connect( tabWidget, SIGNAL( currentChanged( int ) ),
            this,      SLOT  ( newPanel      ( int ) ) );
   connect( pb_next,   SIGNAL( clicked()    ),
            this,      SLOT  ( panelUp()    ) );
   connect( pb_prev,   SIGNAL( clicked()    ),
            this,      SLOT  ( panelDown()  ) );
   //connect( pb_close,  SIGNAL( clicked()    ),
   //         this,      SLOT  ( close      ) );
   connect( pb_close,  SIGNAL( clicked()    ),
            this,      SLOT  ( close_program()      ) );
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
   QGridLayout* genL   = new QGridLayout();

QPushButton*
   pb_proname      = us_pushbutton( tr( "Profile Name" ) );
//   QLabel*      lb_proname      = us_label( tr( "Profile Name:" ) );
   QLabel*      lb_loodmax      = us_label( tr( "Lower OD Maximum:" ) );
   QLabel*      lb_hiodmax      = us_label( tr( "Upper OD Maximum:" ) );
   pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
//   int          ihgt            = lb_proname->height();
   int          ihgt            = pb_proname->height();
   QSpacerItem* spacer1         = new QSpacerItem( 20, ihgt );

   // Set up an approprate investigator text
   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   int id          = US_Settings::us_inv_ID();
   QString invnbr  = ( id > 0 ) ?
      QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";
   QString invtxt  = invnbr + US_Settings::us_inv_name();
   le_investigator = us_lineedit( invtxt, 0, true );
QLineEdit*
   le_proname      = us_lineedit( "(default)", 0, false );
QLineEdit*
   le_loodmax      = us_lineedit( "0.800", 0, false );
QLineEdit*
   le_hiodmax      = us_lineedit( "3.000", 0, false );

   // Set defaults
   currProf        = &mainw->currProf;
   currProf ->investigator = invtxt;

   // Build main layout
   int row         = 0;
   genL->addWidget( pb_investigator, row,   0, 1, 2 );
   genL->addWidget( le_investigator, row++, 2, 1, 6 );
   genL->addWidget( pb_proname,      row,   0, 1, 2 );
   genL->addWidget( le_proname,      row++, 2, 1, 6 );
   genL->addWidget( lb_loodmax,      row,   0, 1, 2 );
   genL->addWidget( le_loodmax,      row,   2, 1, 2 );
   genL->addWidget( lb_hiodmax,      row,   4, 1, 2 );
   genL->addWidget( le_hiodmax,      row++, 6, 1, 2 );
   
   genL->addItem  ( spacer1,         row++, 6, 1, 2 );
   
   panel->addLayout( genL );
   panel->addStretch();

   // Set up signal and slot connections
//   connect( le_runid,        SIGNAL( textEdited(const QString &)  ),
//	    this,            SLOT(   check_empty_runname(const QString &) ) );

   mainw->solutions_change = false;

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
   mainw               = (US_AnalysisProfile*)topw;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "2: Specify Edit parameters" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   QLabel*      lb_lab      = us_label( tr( "Laboratory:" ) );
//   QPushButton* pb_advrotor = us_pushbutton( tr( "Advanced Lab/Rotor/Calibration" ) );
   int ihgt                 = lb_lab->height();
   QSpacerItem* spacer1     = new QSpacerItem( 20, ihgt );
DbgLv(1) << "APEd: AA";


//   QLabel*      lb_optima_banner    = us_banner( tr( "Select Optima Machine, Operator and Experiment Type " ) );
		
   int row     = 0;
   genL->addWidget( lb_lab,          row,   0, 1, 1 );
   genL->addItem  ( spacer1,         row++, 0, 1, 4 );

   row++;

   genL->addItem  ( spacer1,         row++, 0, 1, 4 );
DbgLv(1) << "APEd: JJ";
   
   panel->addLayout( genL );
   panel->addStretch();


//   connect( cb_lab,       SIGNAL( activated   ( int ) ),
//            this,         SLOT  ( changeLab   ( int ) ) );

   first_time_init = true;
   curr_rotor = 0;
DbgLv(1) << "APEd: MM";
   savePanel();
DbgLv(1) << "APEd: NN";
//   changed             = false;

   initPanel();
DbgLv(1) << "APEd: OO";

   first_time_init = false;
DbgLv(1) << "APEd: ZZ";
}


// Panel for Speed step parameters
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
   
   // Labels
//   QLabel*  lb_count   = us_label( tr( "Number of Speed Profiles:" ) );

   // Create main layout rows
//   int row = 0;
//   genL->addWidget( lb_count,   row,    0, 1,  6 );
   
   // Connect signals and slots
//   connect( sb_scnint_ss, SIGNAL( valueChanged   ( int ) ),
//            this,         SLOT  ( ssChgScIntTime_ss ( int ) ) ); 

   
DbgLv(1) << "AP2d: addWidg/Layo II";

   // Complete overall layout
   panel->addLayout( genL );
   panel->addStretch();
   adjustSize();

   // Set low delay-minutes based on speed,acceleration,delay-hours
   adjustDelay();

DbgLv(1) << "AP2d: CALL initPanel()";
   initPanel();
DbgLv(1) << "AP2d:  RTN initPanel()";

//qDebug() << "SCANINT: " << ssvals[ 0 ][ "scanintv" ]  << ", SCANINT FROM rpSpeed: " <<  rpSpeed->ssteps[ 0 ].scanintv;
}
//! \brief Compose a speed step description
QString US_AnaprofPan2DSA::speedp_description( const int )
{
   return "";
}
//! \brief Function to adjust delay based on speed,accel,delay-hrs
void US_AnaprofPan2DSA::adjustDelay   ( void   )
{
   return;
}
void US_AnaprofPan2DSA::stageDelay_sync     ( void )
{
   return;
}
void US_AnaprofPan2DSA::syncdelayChecked    ( bool )
{
   return;
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


   QLabel* lb_hdr1     = us_banner( tr( "PCSA" ) );
   int row             = 0;
   genL->addWidget( lb_hdr1, row,   0, 1, 1 );


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


   QLabel* lb_hdr1     = us_banner( tr( "Upload" ) );
   int row             = 0;
   genL->addWidget( lb_hdr1, row,   0, 1, 1 );


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
QString US_AnaprofPanUpload::buildJson( void )
{
   return "";
}

