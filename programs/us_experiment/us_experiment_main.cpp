//! \file us_experiment_main.cpp

#include "us_experiment_main.h"
#include "us_rotor_gui.h"
#include "us_solution_gui.h"
#include "us_extinction_gui.h"
#include "us_table.h"
#include "us_xpn_data.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_sleep.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define QRegularExpression(a)  QRegExp(a)
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief Main program for US_ExperimentMain. Loads translators and starts
//         the class US_ExperimentMain

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_ExperimentMain w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// Constructor
US_ExperimentMain::US_ExperimentMain() : US_Widgets()
{
   dbg_level    = US_Settings::us_debug();
   curr_panx    = 0;

   setWindowTitle( tr( "Define An Experiment" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main      = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );
   QGridLayout* statL     = new QGridLayout();
   QHBoxLayout* buttL     = new QHBoxLayout();

   tabWidget           = us_tabwidget();
   tabWidget->setTabPosition( QTabWidget::North );
   
   epanGeneral         = new US_ExperGuiGeneral  ( this );
   epanRotor           = new US_ExperGuiRotor    ( this );
   epanSpeeds          = new US_ExperGuiSpeeds   ( this );
   epanCells           = new US_ExperGuiCells    ( this );
   epanSolutions       = new US_ExperGuiSolutions( this );
   epanOptical         = new US_ExperGuiOptical  ( this );
   epanSpectra         = new US_ExperGuiSpectra  ( this );
   epanUpload          = new US_ExperGuiUpload   ( this );

   tabWidget->addTab( epanGeneral,   tr( "1: General"          ) );
   tabWidget->addTab( epanRotor,     tr( "2: Lab/Rotor"        ) );
   tabWidget->addTab( epanSpeeds,    tr( "3: Speeds"           ) );
   tabWidget->addTab( epanCells,     tr( "4: Cells"            ) );
   tabWidget->addTab( epanSolutions, tr( "5: Solutions"        ) );
   tabWidget->addTab( epanOptical,   tr( "6: Optical Systems"  ) );
   tabWidget->addTab( epanSpectra,   tr( "7: Spectra"          ) );
   tabWidget->addTab( epanUpload,    tr( "8: Upload"           ) );
   tabWidget->setCurrentIndex( 0 );

   QLabel* lb_stat        = us_label( tr( "Status:" ) );
   le_stat = us_lineedit( tr( "1:u  2:u  3:u  4:u  5:u  6:u  7:u  8:u"
                              "   ('u'==unspecified  'X'==parameterized)" ),
                          -1, true );
   QPalette vvlgray       = US_GuiSettings::editColor();
   vvlgray.setColor( QPalette::Base, QColor( 0xf0, 0xf0, 0xf0 ) );
   le_stat->setPalette( vvlgray );

   statL->addWidget( lb_stat, 0, 0, 1, 1 );
   statL->addWidget( le_stat, 0, 1, 1, 7 );

   QPushButton* pb_close  = us_pushbutton( tr( "Close" ) );
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_next   = us_pushbutton( tr( "Next Panel" ) );
   QPushButton* pb_prev   = us_pushbutton( tr( "Previous Panel" ) );
   QPushButton* pb_stupd  = us_pushbutton( tr( "Status Update" ) );
   buttL->addWidget( pb_help  );
   buttL->addWidget( pb_stupd );
   buttL->addWidget( pb_prev  );
   buttL->addWidget( pb_next  );
   buttL->addWidget( pb_close );

   connect( tabWidget, SIGNAL( currentChanged( int ) ),
            this,      SLOT  ( newPanel      ( int ) ) );
   connect( pb_stupd,  SIGNAL( clicked()    ),
            this,      SLOT  ( statUpdate() ) );
   connect( pb_next,   SIGNAL( clicked()    ),
            this,      SLOT  ( panelUp()    ) );
   connect( pb_prev,   SIGNAL( clicked()    ),
            this,      SLOT  ( panelDown()  ) );
   connect( pb_close,  SIGNAL( clicked()    ),
            this,      SLOT  ( close()      ) );
   connect( pb_help,   SIGNAL( clicked()    ),
            this,      SLOT  ( help()       ) );

   main->addWidget( tabWidget );
   main->addLayout( statL );
   main->addLayout( buttL );

   setMinimumSize( QSize( 800, 400 ) );
   adjustSize();

   reset();
}

// Public function to return a parameter value
//   from a US_ExperimentMain child panel
QString US_ExperimentMain::childPValue( const QString child, const QString type )
{
   QString value( "" );

   if      ( child == "general" )
   {
      value = epanGeneral  ->getPValue( type );
   }
   else if ( child == "rotor" )
   {
      value = epanRotor    ->getPValue( type );
   }
   else if ( child == "speeds" )
   {
      value = epanSpeeds   ->getPValue( type );
   }
   else if ( child == "cells" )
   {
      value = epanCells    ->getPValue( type );
   }
   else if ( child == "solutions" )
   {
      value = epanSolutions->getPValue( type );
   }
   else if ( child == "optical" )
   {
      value = epanOptical  ->getPValue( type );
   }
   else if ( child == "spectra" )
   {
      value = epanSpectra  ->getPValue( type );
   }
   else if ( child == "upload" )
   {
      value = epanUpload   ->getPValue( type );
   }

   return value;
}

// Public function to return a parameter list value
//   from a US_ExperimentMain child panel
QStringList US_ExperimentMain::childPList( const QString child, const QString type )
{
   QStringList value;

   if ( child == "general" )
   {
      value  = epanGeneral ->getPList( type );
   }
   else if ( child == "rotor" )
   {
      value = epanRotor    ->getPList( type );
   }
   else if ( child == "speeds" )
   {
      value = epanSpeeds   ->getPList( type );
   }
   else if ( child == "cells" )
   {
      value = epanCells    ->getPList( type );
   }
   else if ( child == "solutions" )
   {
      value = epanSolutions->getPList( type );
   }
   else if ( child == "optical" )
   {
      value = epanOptical  ->getPList( type );
   }
   else if ( child == "spectra" )
   {
      value = epanSpectra  ->getPList( type );
   }
   else if ( child == "upload" )
   {
      value = epanUpload   ->getPList( type );
   }

   return value;
}

// Slot to handle a new panel selected
void US_ExperimentMain::newPanel( int panx )
{
DbgLv(1) << "newPanel panx=" << panx << "prev.panx=" << curr_panx;
   curr_panx        = panx;

   // Initialize the new current panel after possible changes
   if ( panx == 0 )
      epanGeneral  ->initPanel();
   else if ( panx == 1 )
      epanRotor    ->initPanel();
   else if ( panx == 2 )
      epanSpeeds   ->initPanel();
   else if ( panx == 3 )
      epanCells    ->initPanel();
   else if ( panx == 4 )
      epanSolutions->initPanel();
   else if ( panx == 5 )
      epanOptical  ->initPanel();
   else if ( panx == 6 )
      epanSpectra  ->initPanel();
   else if ( panx == 5 )
      epanUpload   ->initPanel();

   // Update status text for all panels
   statUpdate();
}

// Slot to update status text for all panels
void US_ExperimentMain::statUpdate()
{
   QString stattext = le_stat->text();
DbgLv(1) << "statUpd: IN stat" << stattext;
   stattext.replace( QRegularExpression( "1:[uX]" ),
                     epanGeneral  ->status() );
   stattext.replace( QRegularExpression( "2:[uX]" ),
                     epanRotor    ->status() );
   stattext.replace( QRegularExpression( "3:[uX]" ),
                     epanSpeeds   ->status() );
   stattext.replace( QRegularExpression( "4:[uX]" ),
                     epanCells    ->status() );
   stattext.replace( QRegularExpression( "5:[uX]" ),
                     epanSolutions->status() );
   stattext.replace( QRegularExpression( "6:[uX]" ),
                     epanOptical  ->status() );
   stattext.replace( QRegularExpression( "7:[uX]" ),
                     epanSpectra  ->status() );
   stattext.replace( QRegularExpression( "8:[uX]" ),
                     epanUpload   ->status() );
DbgLv(1) << "statUpd:  MOD stat" << stattext;

   le_stat->setText( stattext );
}

// Slot to advance to the next panel
void US_ExperimentMain::panelUp()
{
   int newndx = tabWidget->currentIndex() + 1;
   int maxndx = tabWidget->count() - 1;
DbgLv(1) << "panUp: newndx, maxndx" << newndx << maxndx;
   tabWidget->setCurrentIndex( qMin( newndx, maxndx ) );
}

// Slot to descend to the previous panel
void US_ExperimentMain::panelDown()
{
   int newndx = tabWidget->currentIndex() - 1;
   tabWidget->setCurrentIndex( qMax( newndx, 0 ) );
}

// Open manual help appropriate to the current panel
void US_ExperimentMain::help( void )
{
   if ( curr_panx == 0 )
      epanGeneral  ->help();
   else if ( curr_panx == 1 )
      epanRotor    ->help();
   else if ( curr_panx == 2 )
      epanSpeeds   ->help();
   else if ( curr_panx == 3 )
      epanCells    ->help();
   else if ( curr_panx == 4 )
      epanSolutions->help();
   else if ( curr_panx == 5 )
      epanOptical  ->help();
   else if ( curr_panx == 6 )
      epanSpectra  ->help();
   else if ( curr_panx == 7 )
      epanUpload   ->help();
}

// Reset parameters to their defaults
void US_ExperimentMain::reset( void )
{
}

// Panel for run and other general parameters
US_ExperGuiGeneral::US_ExperGuiGeneral( QWidget* topw )
{
   mainw               = topw;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "1: Specify run and other general parameters" ) );
   panel->addWidget( lb_panel );

   QGridLayout* genL   = new QGridLayout();

   QLabel*      lb_runid        = us_label( tr( "Run ID:" ) );
   QPushButton* pb_project      = us_pushbutton( tr( "Select Project" ) );
   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
                le_runid        = us_lineedit( "", 0, false );
                le_project      = us_lineedit( "", 0, false );

   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   int id = US_Settings::us_inv_ID();
   QString number  = ( id > 0 ) ?
      QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(), 0, false );

   disk_controls   = new US_Disk_DB_Controls;

   int row=0;
   genL->addWidget( lb_runid,        row,   0, 1, 1 );
   genL->addWidget( le_runid,        row++, 1, 1, 3 );
   genL->addWidget( pb_project,      row,   0, 1, 1 );
   genL->addWidget( le_project,      row++, 1, 1, 3 );
   genL->addWidget( pb_investigator, row,   0, 1, 2 );
   genL->addWidget( le_investigator, row++, 2, 1, 2 );
   genL->addLayout( disk_controls,   row++, 0, 1, 4 );

   panel->addLayout( genL );
   panel->addStretch();

   connect( le_runid,        SIGNAL( editingFinished()  ),
            this,            SLOT(   runID_entered()    ) );
   connect( pb_project,      SIGNAL( clicked()          ), 
            this,            SLOT(   sel_project()      ) );
   connect( pb_investigator, SIGNAL( clicked()          ), 
            this,            SLOT(   sel_investigator() ) );

   // Read in centerpiece information and populate names list
   centerpieceInfo();
}

// Initialize a panel, especially after clicking on its tab
void US_ExperGuiGeneral::initPanel()
{
}

// Get a specific panel value
QString US_ExperGuiGeneral::getPValue( const QString type )
{
   QString value( "" );

   if ( type == "runID" )
   {
      value = le_runid->text();
   }
   else if ( type == "project" )
   {
      value = le_project->text();
   }
   else if ( type == "investigator" )
   {
      value = le_investigator->text();
   }
   else if ( type == "dbdisk" )
   {
      value = ( disk_controls->db() ) ? "DB" : "Disk";
   }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiGeneral::getPList( const QString type )
{
   QStringList value( "" );

   if ( type == "all" )
   {
      value.clear();
      value << getPValue( "run" );
      value << getPValue( "project" );
      value << getPValue( "investigator" );
      value << getPValue( "dbdisk" );
   }
   else if ( type == "centerpieces" )
   {
      value = cp_names;
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiGeneral::sibPValue( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPValue( sibling, type )
            : QString("") );
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiGeneral::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPList( sibling, type )
            : QStringList() );
}

// Return status string for the panel
QString US_ExperGuiGeneral::status()
{
   bool is_done  = ( ! le_runid->text().isEmpty() &&
                     ! le_project->text().isEmpty() );
   return ( is_done ? QString( "1:X" ) : QString( "1:u" ) );
}
 
// Return centerpiece names list
QStringList US_ExperGuiGeneral::cpNames( void )
{
    return cp_names;
}

// Return detail information for a specific centerpiece as named
bool US_ExperGuiGeneral::cpInfo( const QString cpname,
      US_AbstractCenterpiece& cpEntry )
{
   bool is_found   = false;

   for ( int ii = 0; ii < acp_list.count(); ii++ )
   {
      US_AbstractCenterpiece cpiece = acp_list[ ii ];
      if ( cpiece.name == cpname )
      {  // Match found:  flag found and return entry
         is_found        = true;
         cpEntry         = cpiece;
         break;
      }
   }

   return is_found;
}

// Verify valid run ID (possible modify for valid-only characters)
void US_ExperGuiGeneral::runID_entered( void )
{
DbgLv(1) << "EGG: rchg: IN";
   // Modify runID to have only valid characters
   QRegExp rx( "[^A-Za-z0-9_-]" );
   QString runID     = le_runid->text();
   QString old_runID = runID;
DbgLv(1) << "EGG: rchg: old_runID" << old_runID;
   runID.replace( rx,  "_" );
DbgLv(1) << "EGG: rchg:     runID" << runID;
   bool changed      = false;
   
   if ( runID != old_runID )
   {  // Report on invalid characters replaced
      QMessageBox::warning( this,
         tr( "RunID Name Changed" ),
         tr( "The runID name has been changed. It may consist only\n"
             "of alphanumeric characters or underscore or hyphen.\n"
             "New runID:\n  " )
            + runID );
      changed           = true;
   }

DbgLv(1) << "EGG: rchg: len(runID)" << runID.length();
   // Limit run ID length to 50 characters
   if ( runID.length() > 50 )
   {
      QMessageBox::warning( this,
         tr( "RunID Name Too Long" ),
         tr( "The runID name may be at most\n"
             "50 characters in length." ) );
      runID             = runID.left( 50 );
      changed           = true;
   }
DbgLv(1) << "EGG: rchg: changed" << changed;

   if ( changed )
   {  // Replace runID in line edit box
      le_runid->setText( runID );
   }
}

// Select DB investigator
void US_ExperGuiGeneral::sel_project( void )
{
   int dbdisk           = ( disk_controls->db() ) ? US_Disk_DB_Controls::DB
                          : US_Disk_DB_Controls::Disk;
   US_Project project;
   US_ProjectGui* dialog = new US_ProjectGui( true, dbdisk, project );
   connect( dialog, SIGNAL( updateProjectGuiSelection( US_Project& ) ),
            this,   SLOT  ( project_info             ( US_Project& ) ) );

   dialog->exec();
}

// Select DB investigator
void US_ExperGuiGeneral::sel_investigator( void )
{
   int investigator = US_Settings::us_inv_ID();

   US_Investigator* dialog = new US_Investigator( true, investigator );
   dialog->exec();

   investigator = US_Settings::us_inv_ID();

   QString inv_text = QString::number( investigator ) + ": "
                      +  US_Settings::us_inv_name();

   le_investigator->setText( inv_text );
   //report_entry.investigator = US_Settings::us_inv_name();
}

// Capture selected project information
void US_ExperGuiGeneral::project_info( US_Project& project )
{
DbgLv(1) << "projinfo: proj.desc" << project.projectDesc;
DbgLv(1) << "projinfo: proj.guid" << project.projectGUID;

   le_project->setText( project.projectDesc );
}

// Get centerpiece information (initially or after DB/Disk change
void US_ExperGuiGeneral::centerpieceInfo( void )
{
   US_Passwd pw;
   US_DB2* dbP          = ( disk_controls->db() )
                          ? new US_DB2( pw.getPasswd() )
                          : NULL;

   // Read in the full centerpiece information from DB or Disk
   US_AbstractCenterpiece::read_centerpieces( dbP, acp_list );
   cp_names.clear();

   // Populate the list of centerpiece names
   for ( int ii = 0; ii < acp_list.count(); ii++ )
   {
      cp_names << acp_list[ ii ].name;
   }
}


// Panel for Lab/Rotor parameters
US_ExperGuiRotor::US_ExperGuiRotor( QWidget* topw )
{
   mainw               = topw;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "2: Specify lab/rotor parameters" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   QLabel*      lb_lab      = us_label( tr( "Laboratory:" ) );
   QLabel*      lb_rotor    = us_label( tr( "Rotor:" ) );
   QLabel*      lb_calibr   = us_label( tr( "Calibration:" ) );
   QPushButton* pb_advrotor = us_pushbutton( tr( "Advanced Lab/Rotor/Calibration" ) );
   int          ihgt        = pb_advrotor->height();
                cb_lab      = new QComboBox( this );
                cb_rotor    = new QComboBox( this );
                cb_calibr   = new QComboBox( this );
   QSpacerItem* spacer1     = new QSpacerItem( 20, ihgt );

   int row=0;
   genL->addWidget( lb_lab,          row,   0, 1, 1 );
   genL->addWidget( cb_lab,          row++, 1, 1, 1 );
   genL->addWidget( lb_rotor,        row,   0, 1, 1 );
   genL->addWidget( cb_rotor,        row,   1, 1, 1 );
   genL->addWidget( lb_calibr,       row,   2, 1, 1 );
   genL->addWidget( cb_calibr,       row++, 3, 1, 1 );
   genL->addItem  ( spacer1,         row++, 0, 1, 4 );
   genL->addWidget( pb_advrotor,     row++, 0, 1, 4 );

   panel->addLayout( genL );
   panel->addStretch();

   US_Passwd pw;
   US_DB2* dbP              = ( sibPValue( "general", "dbdisk" ) == "DB" )
                              ? new US_DB2( pw.getPasswd() ) : NULL;
   if ( dbP != NULL )
   {
      US_Rotor::readLabsDB( labs, dbP );
   }
   else
   {
      US_Rotor::readLabsDisk( labs );
   }

   for ( int ii = 0; ii < labs.count(); ii++ )
   {
      sl_labs << QString::number( labs[ ii ].ID )
                 + ": " + labs[ ii ].name;
   }
   cb_lab->clear();
   cb_lab->addItems( sl_labs );

   connect( cb_lab,       SIGNAL( activated   ( int ) ),
            this,         SLOT  ( changeLab   ( int ) ) );
   connect( cb_rotor,     SIGNAL( activated   ( int ) ),
            this,         SLOT  ( changeRotor ( int ) ) );
   connect( cb_calibr,    SIGNAL( activated   ( int ) ),
            this,         SLOT  ( changeCalib ( int ) ) );
   connect( pb_advrotor,  SIGNAL( clicked()  ),
            this,         SLOT  ( advRotor() ) );

   changeLab( 0 );
   changed             = false;
};

// Initialize a panel, especially after clicking on its tab
void US_ExperGuiRotor::initPanel()
{
}

// Get a specific panel value
QString US_ExperGuiRotor::getPValue( const QString type )
{
   QString value( "" );

   if ( type == "lab" )
   {
      value = cb_lab->currentText();
   }
   else if ( type == "rotor" )
   {
      value = cb_rotor->currentText();
value=value.isEmpty()?"defaultRotor":value;
   }
   else if ( type == "calib" )
   {
      value = cb_calibr->currentText();
   }
   else if ( type == "abstractRotor" )
   {
      int rx      = cb_rotor->currentIndex();
      int arID    = rotors[ rx ].abstractRotorID;
      value       = "";

      for ( int ii = 0; ii < arotors.count(); ii++ )
      {
         if ( arotors[ ii ].ID == arID )
         {
            value       = arotors[ ii ].name;
            break;
         }
      }
   }
   else if ( type == "nholes" )
   {
      int rx      = cb_rotor->currentIndex();
      int arID    = rotors[ rx ].abstractRotorID;
      value       = "";

      for ( int ii = 0; ii < arotors.count(); ii++ )
      {
         if ( arotors[ ii ].ID == arID )
         {
            value       = QString::number( arotors[ ii ].numHoles );
            break;
         }
      }
   }
   else if ( type == "changed" )
   {
      value       = changed ? "1" : "0";
   }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiRotor::getPList( const QString type )
{
   QStringList value( "" );

   if ( type == "all" )
   {
      value.clear();
      value << getPValue( "lab" );
      value << getPValue( "rotor" );
      value << getPValue( "calib" );
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiRotor::sibPValue( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPValue( sibling, type )
            : QString("") );
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiRotor::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPList( sibling, type )
            : QStringList() );
}

// Return status string for the panel
QString US_ExperGuiRotor::status()
{
   bool is_done = ( cb_lab   ->currentIndex() >= 0  &&
                    cb_rotor ->currentIndex() >= 0  &&
                    cb_calibr->currentIndex() >= 0 );
   return ( is_done ? QString( "2:X" ) : QString( "2:u" ) );
}

// Slot for change in Lab selection
void US_ExperGuiRotor::changeLab( int ndx )
{
DbgLv(1) << "EGR:chgLab  ndx" << ndx;
   changed             = true;
   cb_lab->setCurrentIndex( ndx );
   QString clab        = cb_lab->currentText();
   int labID           = clab.section( ":", 0, 0 ).toInt();
   QString descr       = clab.section( ":", 1, 1 ).simplified();
DbgLv(1) << "EGR: chgLab labID desc" << labID << descr;

   US_Passwd pw;
   US_DB2* dbP              = ( sibPValue( "general", "dbdisk" ) == "DB" )
                              ? new US_DB2( pw.getPasswd() ) : NULL;
   if ( dbP != NULL )
   {
      US_Rotor::readAbstractRotorsDB( arotors, dbP );
      US_Rotor::readRotorsFromDB( rotors, labID, dbP );
   }
   else
   {
      US_Rotor::readAbstractRotorsDisk( arotors );
      US_Rotor::readRotorsFromDisk( rotors, labID );
   }

   sl_rotors.clear();

   for ( int ii = 0; ii < rotors.count(); ii++ )
   {
      sl_rotors << QString::number( rotors[ ii ].ID )
                 + ": " + rotors[ ii ].name;
   }

   cb_rotor->clear();
   cb_rotor->addItems( sl_rotors );
   changeRotor( 0 );
}

// Slot for change in Rotor selection
void US_ExperGuiRotor::changeRotor( int ndx )
{
DbgLv(1) << "EGR:chgRotor  ndx" << ndx;
   changed             = true;
   cb_rotor->setCurrentIndex( ndx );
   QString crot        = cb_rotor->currentText();
   int rotID           = crot.section( ":", 0, 0 ).toInt();
   QString descr       = crot.section( ":", 1, 1 ).simplified();
DbgLv(1) << "EGR: chgRotor rotID desc" << rotID << descr;
   calibs    .clear();
   sl_calibs .clear();
   cb_calibr->clear();

   US_Passwd pw;
   US_DB2* dbP         = ( sibPValue( "general", "dbdisk" ) == "DB" )
                         ? new US_DB2( pw.getPasswd() ) : NULL;
   if ( dbP != NULL )
   {
      US_Rotor::readCalibrationProfilesDB( calibs, rotID, dbP );
   }
   else
   {
      US_Rotor::readCalibrationProfilesDisk( calibs, rotID );
   }
DbgLv(1) << "EGR: chgRotor calibs count" << calibs.count();

   for ( int ii = 0; ii < calibs.count(); ii++ )
   {
      sl_calibs << QString::number( calibs[ ii ].ID )
                 + ": " + calibs[ ii ].lastUpdated.toString( "d MMMM yyyy" );
   }

   cb_calibr->addItems( sl_calibs );
   int lndx            = calibs.count() - 1;
   if ( lndx >= 0 )
      cb_calibr->setCurrentIndex( lndx );
}

// Slot for change in Calibration selection
void US_ExperGuiRotor::changeCalib( int ndx )
{
DbgLv(1) << "EGR:chgCal: ndx" << ndx;
   changed             = true;
}

// Slot for click on Advanced Lab... button
void US_ExperGuiRotor::advRotor()
{
   US_Rotor::RotorCalibration  calibr;
   US_Rotor::Rotor             rotor;
   int calibx       = cb_calibr->currentIndex();
   int rotorx       = cb_rotor ->currentIndex();
   calibr.ID        = ( calibx >= 0 )
                      ? cb_calibr->currentText().section( ":", 0, 0 ).toInt()
                      : 0;
   rotor.ID         = ( rotorx >= 0 )
                      ? cb_rotor ->currentText().section( ":", 0, 0 ).toInt()
                      : 0;
DbgLv(1) << "EGR: advR: IN rID cID" << rotor.ID << calibr.ID;
   int dbdisk       = ( sibPValue( "general", "dbdisk" ) == "DB" )
                      ? US_Disk_DB_Controls::DB
                      : US_Disk_DB_Controls::Disk;
   US_RotorGui* rotorInfo = new US_RotorGui( true, dbdisk, rotor, calibr );

   connect( rotorInfo, SIGNAL( RotorCalibrationSelected(
                          US_Rotor::Rotor&, US_Rotor::RotorCalibration& ) ),
            this,      SLOT  ( advRotorChanged( 
                          US_Rotor::Rotor&, US_Rotor::RotorCalibration& ) ) );

   rotorInfo->exec();
}

// Slot for rotor,calibration changed in advanced dialog
void US_ExperGuiRotor::advRotorChanged( US_Rotor::Rotor& crotor,
                                        US_Rotor::RotorCalibration& ccalib )
{
   int rID          = crotor.ID;
   int cID          = ccalib.ID;
   int rx           = -1;
DbgLv(1) << "EGR: advRChg: new rID cID" << rID << cID;

   for ( int ii = 0; ii < sl_rotors.count(); ii++ )
   {  // Find and select the list item matching the accepted rotor
      int eID          = sl_rotors[ ii ].section( ":", 0, 0 ).toInt();
      if ( eID == rID )
      {  // Match:  select item, set index, break from loop
         cb_rotor ->setCurrentIndex( ii );
         rx               = ii;
DbgLv(1) << "EGR: advRChg:   rID match at index" << ii;
         break;
      }
   }

   if ( rx >= 0 )
      changeRotor( rx );     // Rebuild calibrations for new rotor

   for ( int ii = 0; ii < sl_calibs.count(); ii++ )
   {  // Find and select the list item matching the accepted calibration
      int eID          = sl_calibs[ ii ].section( ":", 0, 0 ).toInt();
      if ( eID == cID )
      {
         cb_calibr->setCurrentIndex( ii );
DbgLv(1) << "EGR: advRChg:   cID match at index" << ii;
         break;
      }
DbgLv(1) << "EGR: advRChg:     ii eID" << ii << eID;
   }
}

// Panel for Speed step parameters
US_ExperGuiSpeeds::US_ExperGuiSpeeds( QWidget* topw )
{
   mainw               = topw;
   changed             = false;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "3: Specify speed steps" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   QLabel* lb_count    = us_label( tr( "Number of Speed Profiles:" ) );
   QLabel* lb_lenhr    = us_label( tr( "Length of Experiment (Hours):" ) );
   QLabel* lb_lenmin   = us_label( tr( "Length of Experiment (Minutes):" ) );
   QLabel* lb_dlyhr    = us_label( tr( "Time Delay for Scans (Hours):" ) );
   QLabel* lb_dlymin   = us_label( tr( "Time Delay for Scans (Minutes):" ) );
   QLabel* lb_speed    = us_label( tr( "Rotor Speed (rpm):" ) );
   QLabel* lb_accel    = us_label( tr( "Acceleration Profile (rpm/sec):" ) );
   cb_prof             = new QComboBox( this );
   ct_speed            = us_counter( 2, 1000, 100000, 100 );
   ct_accel            = us_counter( 2,   50,   1000,  50 );
   ct_count            = us_counter( 2,    1,    100,   1 );
   ct_lenhr            = us_counter( 2,    0,    100,   1 );
   ct_lenmin           = us_counter( 2,    0,     60,   1 );
   ct_dlyhr            = us_counter( 2,    0,     10,   1 );
   ct_dlymin           = us_counter( 2,    0,     60,   1 );
   nspeed              = 1;
   curssx              = 0;
   profdesc.resize( nspeed );
   ssvals  .resize( nspeed * 6 );
   ssvals[ 0 ]         = 30000;
   ssvals[ 1 ]         = 400;
   ssvals[ 2 ]         = 5;
   ssvals[ 3 ]         = 30;
   ssvals[ 4 ]         = 0;
   ssvals[ 5 ]         = 30;
   ct_count ->setSingleStep(   1 );
   ct_speed ->setSingleStep( 100 );
   ct_accel ->setSingleStep(  50 );
   ct_lenhr ->setSingleStep(   1 );
   ct_lenmin->setSingleStep(   1 );
   ct_dlyhr ->setSingleStep(   1 );
   ct_dlymin->setSingleStep(   1 );
   ct_count ->setValue( nspeed );
   ct_speed ->setValue( ssvals[ 0 ] );
   ct_accel ->setValue( ssvals[ 1 ] );
   ct_lenhr ->setValue( ssvals[ 2 ] );
   ct_lenmin->setValue( ssvals[ 3 ] );
   ct_dlyhr ->setValue( ssvals[ 4 ] );
   ct_dlymin->setValue( ssvals[ 5 ] );
   //cb_prof->addItem( tr( "Speed Profile 1: 30000 rpm for 5 hr 30 min" ) );
   cb_prof->addItem( speedp_description( 0 ) );

   lb_count->adjustSize();
   QFont font( US_GuiSettings::fontFamily(),
               US_GuiSettings::fontSize() );
   QFontMetrics fmet( font );
   int fwidth  = fmet.maxWidth();
   int rheight = lb_count->height();
   int csizw   = fwidth * 8;
   ct_count ->resize( csizw, rheight );
   ct_speed ->resize( csizw, rheight );
   ct_accel ->resize( csizw, rheight );
   ct_lenhr ->resize( csizw, rheight );
   ct_lenmin->resize( csizw, rheight );
   ct_dlyhr ->resize( csizw, rheight );
   ct_dlymin->resize( csizw, rheight );

   int row = 0;
   genL->addWidget( lb_count,  row,   0, 1, 3 );
   genL->addWidget( ct_count,  row++, 3, 1, 1 );
   genL->addWidget( cb_prof,   row++, 0, 1, 4 );
   genL->addWidget( lb_speed,  row,   0, 1, 3 );
   genL->addWidget( ct_speed,  row++, 3, 1, 1 );
   genL->addWidget( lb_accel,  row,   0, 1, 3 );
   genL->addWidget( ct_accel,  row++, 3, 1, 1 );
   genL->addWidget( lb_lenhr,  row,   0, 1, 3 );
   genL->addWidget( ct_lenhr,  row++, 3, 1, 1 );
   genL->addWidget( lb_lenmin, row,   0, 1, 3 );
   genL->addWidget( ct_lenmin, row++, 3, 1, 1 );
   genL->addWidget( lb_dlyhr,  row,   0, 1, 3 );
   genL->addWidget( ct_dlyhr,  row++, 3, 1, 1 );
   genL->addWidget( lb_dlymin, row,   0, 1, 3 );
   genL->addWidget( ct_dlymin, row++, 3, 1, 1 );
   genL->setColumnStretch( 0, 4 );
   genL->setColumnStretch( 3, 0 );

   connect( ct_count,  SIGNAL( valueChanged ( double ) ),
            this,      SLOT  ( ssChangeCount( double ) ) );
   connect( cb_prof,   SIGNAL( activated    ( int    ) ),
            this,      SLOT  ( ssChangeProfx( int    ) ) );
   connect( ct_speed,  SIGNAL( valueChanged ( double ) ),
            this,      SLOT  ( ssChangeSpeed( double ) ) );
   connect( ct_accel,  SIGNAL( valueChanged ( double ) ),
            this,      SLOT  ( ssChangeAccel( double ) ) );
   connect( ct_lenhr,  SIGNAL( valueChanged ( double ) ),
            this,      SLOT  ( ssChangeDurhr( double ) ) );
   connect( ct_lenmin, SIGNAL( valueChanged ( double ) ),
            this,      SLOT  ( ssChangeDurmn( double ) ) );
   connect( ct_dlyhr,  SIGNAL( valueChanged ( double ) ),
            this,      SLOT  ( ssChangeDlyhr( double ) ) );
   connect( ct_dlymin, SIGNAL( valueChanged ( double ) ),
            this,      SLOT  ( ssChangeDlymn( double ) ) );

   panel->addLayout( genL );
   panel->addStretch();
   adjustSize();
};

// Initialize a panel, especially after clicking on its tab
void US_ExperGuiSpeeds::initPanel()
{
}

// Get a specific panel value
QString US_ExperGuiSpeeds::getPValue( const QString type )
{
   QString value( "" );

   if ( type == "nspeeds" )
   {
      value = QString::number( ct_count ->value() );
   }
   else if ( type == "speed" )
   {
      value = QString::number( ct_speed ->value() );
   }
   else if ( type == "accel" )
   {
      value = QString::number( ct_accel ->value() );
   }
   else if ( type == "durhr" )
   {
      value = QString::number( ct_lenhr ->value() );
   }
   else if ( type == "durmin" )
   {
      value = QString::number( ct_lenmin->value() );
   }
   else if ( type == "delayhr" )
   {
      value = QString::number( ct_dlyhr ->value() );
   }
   else if ( type == "delaymin" )
   {
      value = QString::number( ct_dlymin->value() );
   }
   else if ( type == "changed" )
   {
      value       = changed ? "1" : "0";
   }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiSpeeds::getPList( const QString type )
{
   QStringList value( "" );

DbgLv(1) << "EGS:getPL: type" << type;
   if ( type == "profiles" )
   {  // Compose list of all speed-step values
      value.clear();
      int nspeed  = (int)ct_count ->value();
DbgLv(1) << "EGS:getPL: nspeed" << nspeed;

      for ( int ii = 0; ii < nspeed * 6; ii++ )
      {  // Build list of QString forms of speed-step doubles
         value << QString::number( ssvals[ ii ] );
      }
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiSpeeds::sibPValue( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPValue( sibling, type )
            : QString("") );
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiSpeeds::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPList( sibling, type )
            : QStringList() );
}

// Return status string for the panel
QString US_ExperGuiSpeeds::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=true;
   return ( is_done ? QString( "3:X" ) : QString( "3:u" ) );
}

// Return speed profile description string for an indicated step
QString US_ExperGuiSpeeds::speedp_description( int ssx )
{
   // For example: "Speed Profile 1: 30000 rpm for 5 hr 30 min"
   return tr( "Speed Profile %1 :    %2 rpm for %3 hr %4 min" )
          .arg( ssx + 1 )
          .arg( ssvals[ ssx * 6 ] )
          .arg( ssvals[ ssx * 6 + 2 ] )
          .arg( ssvals[ ssx * 6 + 3 ] );
}

// Slot for change in speed-step count
void US_ExperGuiSpeeds::ssChangeCount( double val )
{
   changed          = true;
   int new_nsp      = (int)val;
DbgLv(1) << "EGS: chgKnt: nsp nnsp" << nspeed << new_nsp;
   if ( new_nsp > nspeed )
   {  // Number of speed steps increases
      profdesc.resize( new_nsp );
      ssvals  .resize( new_nsp * 6 );
      int kk           = nspeed * 6;
      int jj           = kk - 6;
      double ssspeed   = ssvals[ jj ];
      double ssaccel   = ssvals[ jj + 1 ];
      double ssdurhr   = ssvals[ jj + 2 ];
      double ssdurmn   = ssvals[ jj + 3 ];
      double ssdlyhr   = ssvals[ jj + 4 ];
      double ssdlymn   = ssvals[ jj + 5 ];
DbgLv(1) << "EGS: chgKnt:  jj kk" << jj << kk << "spd acc duh dum dlh dlm"
 << ssspeed << ssaccel << ssdurhr << ssdurmn << ssdlyhr << ssdlymn;

      for ( int ii = nspeed; ii < new_nsp; ii++ )
      {  // Fill in new speed step description and values
         ssspeed         += 5000.0;
         ssvals[ kk++ ]   = ssspeed;          // Speed
         ssvals[ kk++ ]   = ssaccel;          // Acceleration
         ssvals[ kk++ ]   = ssdurhr;          // Duration (hr)
         ssvals[ kk++ ]   = ssdurmn;          // Duration (min)
         ssvals[ kk++ ]   = ssdlyhr;          // Delay (hr)
         ssvals[ kk++ ]   = ssdlymn;          // Delay (min)
         profdesc[ ii ]   = speedp_description( ii );
DbgLv(1) << "EGS: chgKnt:    ii" << ii << "pdesc" << profdesc[ii];

         // Add to the profile description list comboBox
         cb_prof->addItem( profdesc[ ii ] );
      }

      // Point to the first new speed step
      cb_prof->setCurrentIndex( nspeed );
      ssChangeProfx( nspeed );
   }

   else
   {  // Number of speed steps descreases or remains the same
      // Point to the last speed step
      profdesc.resize( new_nsp );
      ssvals  .resize( new_nsp * 6 );
      cb_prof->clear();
      for ( int ii = 0; ii < new_nsp; ii++ )
         cb_prof->addItem( profdesc[ ii ] );
      cb_prof->setCurrentIndex( new_nsp - 1 );
      ssChangeProfx( new_nsp - 1 );
   }

   nspeed      = new_nsp;
}

// Slot for change in speed-step profile index
void US_ExperGuiSpeeds::ssChangeProfx( int ssp )
{
   changed          = true;
DbgLv(1) << "EGS: chgPfx: ssp" << ssp << "prev-ssx" << curssx;
DbgLv(1) << "EGS: chgPfx:  speed-c speed-p" << ssvals[ssp*6] << ssvals[curssx*6];
   curssx           = ssp;
   // Set all counters for newly selected step
   int kk           = ssp * 6;
   ct_speed ->setValue( ssvals[ kk++ ] );
   ct_accel ->setValue( ssvals[ kk++ ] );
   ct_lenhr ->setValue( ssvals[ kk++ ] );
   ct_lenmin->setValue( ssvals[ kk++ ] );
   ct_dlyhr ->setValue( ssvals[ kk++ ] );
   ct_dlymin->setValue( ssvals[ kk++ ] );
}

// Slot for change in speed value
void US_ExperGuiSpeeds::ssChangeSpeed( double val )
{
   changed          = true;
DbgLv(1) << "EGS: chgSpe: val" << val << "ssx" << curssx;
   ssvals[ curssx * 6     ] = val;  // Set Speed in step vals vector
   profdesc[ curssx ]       = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );
}

// Slot for change in acceleration value
void US_ExperGuiSpeeds::ssChangeAccel( double val )
{
DbgLv(1) << "EGS: chgAcc: val" << val << "ssx" << curssx;
   ssvals[ curssx * 6 + 1 ] = val;  // Set Acceleration in step vals vector
}

// Slot for change in duration-hour value
void US_ExperGuiSpeeds::ssChangeDurhr( double val )
{
   changed          = true;
DbgLv(1) << "EGS: chgDuh: val" << val << "ssx" << curssx;
   ssvals[ curssx * 6 + 2 ] = val;  // Set Duration-Hr in step vals vector
   profdesc[ curssx ]       = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );
}

// Slot for change in duration-minute value
void US_ExperGuiSpeeds::ssChangeDurmn( double val )
{
   changed          = true;
DbgLv(1) << "EGS: chgDum: val" << val << "ssx" << curssx;
   ssvals[ curssx * 6 + 3 ] = val;  // Set Duration-min in step vals vector
   profdesc[ curssx ]       = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );
}

// Slot for change in delay-hour value
void US_ExperGuiSpeeds::ssChangeDlyhr( double val )
{
   changed          = true;
DbgLv(1) << "EGS: chgDlh: val" << val << "ssx" << curssx;
   ssvals[ curssx * 6 + 4 ] = val;  // Set Delay-hr in step vals vector
}

// Slot for change in delay-minute value
void US_ExperGuiSpeeds::ssChangeDlymn( double val )
{
   changed          = true;
DbgLv(1) << "EGS: chgDlm: val" << val << "ssx" << curssx;
   ssvals[ curssx * 6 + 5 ] = val;  // Set Delay-min in step vals vector
}


// Panel for Cells parameters
US_ExperGuiCells::US_ExperGuiCells( QWidget* topw )
{
DbgLv(1) << "EGC: IN";
   mainw               = topw;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "4: Define cell centerpiece usage" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   int krow            = 0;
#if 0
   QPushButton* pb_advrotor = us_pushbutton( tr( "Reset from Rotor information" ) );
   genL->addWidget( pb_advrotor, krow++, 0, 1, 8 );
#endif

   QLabel* lb_hdr1     = us_banner( tr( "Cell" ) );
   QLabel* lb_hdr2     = us_banner( tr( "Centerpiece" ) );
   QLabel* lb_hdr3     = us_banner( tr( "Windows" ) );
   genL->addWidget( lb_hdr1, krow,   0, 1, 1 );
   genL->addWidget( lb_hdr2, krow,   1, 1, 6 );
   genL->addWidget( lb_hdr3, krow++, 7, 1, 1 );

   cpnames             = sibPList( "general", "centerpieces" );
   const int mxcels    = 8;
   int nholes          = sibPValue( "rotor", "nholes" ).toInt();
DbgLv(1) << "EGC:  nholes mxcels" << nholes << mxcels;

   for ( int ii = 0; ii < mxcels; ii++ )
   {
      QString scel        = tr( "cell %1" ).arg( ii + 1 );
      QLabel* clabl       = us_label( scel );
      QComboBox* cb_cenp  = us_comboBox();
      QComboBox* cb_wind  = us_comboBox();

      QString strow       = QString::number( ii );
      clabl  ->setObjectName( strow + ": label" );
      cb_cenp->setObjectName( strow + ": centerpiece" );
      cb_wind->setObjectName( strow + ": windows" );

      genL->addWidget( clabl,   krow,   0, 1, 1 );
      genL->addWidget( cb_cenp, krow,   1, 1, 6 );
      genL->addWidget( cb_wind, krow++, 7, 1, 1 );

      cb_cenp->addItem( tr( "empty" ) );
      cb_cenp->addItems( cpnames );
      cb_wind->addItem( tr( "quartz" ) );
      cb_wind->addItem( tr( "sapphire" ) );

      connect( cb_cenp, SIGNAL( activated         ( int ) ),
               this,    SLOT  ( centerpieceChanged( int ) ) );
      connect( cb_wind, SIGNAL( activated         ( int ) ),
               this,    SLOT  ( windowsChanged    ( int ) ) );

      // Save pointers to row objects for later update
      cc_labls << clabl;
      cc_cenps << cb_cenp;
      cc_winds << cb_wind;
   }

   panel->addLayout( genL );
   panel->addStretch();

   // Set up list of centerpieces whose cross-cell counterbalance
   //  is "Titanium counterbalance" by default
   tcb_centps.clear();
   tcb_centps << "Titanium 2-channel standard";

   // Do first pass at initializing the panel layout
   initPanel();
QString pval1 = sibPValue( "rotor", "rotor" );
DbgLv(1) << "EGC: rotor+rotor=" << pval1;
};

// Initialize a panel, especially after clicking on its tab
void US_ExperGuiCells::initPanel()
{
   const int mxcels    = 8;
   int nholes          = sibPValue( "rotor", "nholes" ).toInt();
   int icbal           = nholes - 1;     // Counter-balance index
   QStringList sl_bals;
   sl_bals << "empty (counterbalance)"
           << "Beckman counterbalance"
           << "Titanium counterbalance"
           << "Fluorescence 5-channel counterbalance";
DbgLv(1) << "EGC:initP:  nholes mxcels" << nholes << mxcels
 << "icbal" << icbal;

   for ( int ii = 0; ii < mxcels; ii++ )
   {
DbgLv(1) << "EGC:initP:   ii cenps-count" << ii << cc_cenps[ii]->count();
      bool make_vis       = ( ii < nholes );
      QComboBox* cb_cenp  = cc_cenps[ ii ];
      QComboBox* cb_wind  = cc_winds[ ii ];
      QString cp_text     = cb_cenp->currentText();
      cc_labls[ ii ]->setVisible( make_vis );
      cb_cenp       ->setVisible( make_vis );
      cb_wind       ->setVisible( make_vis );

      if ( ii == icbal )
      {  // This is a counterbalance cell
         cb_cenp->clear();
         cb_cenp->addItems( sl_bals );
         cb_cenp->addItems( sl_bals );
         cb_wind->setVisible( false );
         // Select counterbalance based on cross cell
         int halfnh          = nholes / 2; // Half number holes
         int xrow            = ( ii < halfnh ) ? ii + halfnh : ii - halfnh;
         int jsel            = 1;       // Usually "Beckman counterbalance"
         QString cpname      = cc_cenps[ xrow ]->currentText();
         if ( tcb_centps.contains( cpname ) )
            jsel                = 2;    // In some cases "Titanium counterbalance"
         cb_cenp->setCurrentIndex( jsel );
      }

      else if ( ii < nholes  && cp_text.contains( "counterbalance" ) )
      {  // Was previously counterbalance, but now needs to be centerpiece
         cb_cenp->clear();
         cb_cenp->addItems( cpnames );
         cb_wind->setVisible( true );
      }
   }
}

// Get a specific panel value
QString US_ExperGuiCells::getPValue( const QString type )
{
DbgLv(1) << "EGC:getPV: type" << type;
   QString value( "" );
   int nholes  = sibPValue( "rotor", "nholes" ).toInt();

   if ( type == "ncells" )
   {
      value       = QString::number( nholes );
   }
   else if ( type == "nonEmpty" )
   {
      int nonemp  = 0;
      for ( int ii = 0; ii < nholes; ii++ )
      {
         if ( ! cc_cenps[ ii ]->currentText().contains( "empty" ) )
            nonemp++;
      }
      value       = QString::number( nonemp );
   }
   else if ( type == "alldone" )
   {
      int nonemp  = 0;
      for ( int ii = 0; ii < nholes; ii++ )
      {
         if ( ! cc_cenps[ ii ]->currentText().contains( "empty" ) )
            nonemp++;
      }
      value       = ( nonemp > 0 ) ? "1" : "0";
   }
   else if ( type == "counterbalance" )
   {
DbgLv(1) << "EGC:getPV:   CB nholes cenps-size"
 << nholes << cc_cenps.count();
      QString cbal = cc_cenps[ nholes - 1 ]->currentText();
      if ( ! cbal.contains( "empty" ) )
         value       = cbal;
   }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiCells::getPList( const QString type )
{
   QStringList value;
DbgLv(1) << "EGC:getPL: type" << type;

   if ( type == "cellinfo" )
   {
      //value << le_runid->text();
   }
   else if ( type == "centerpieces" )
   {
      int nholes          = sibPValue( "rotor", "nholes" ).toInt();
DbgLv(1) << "EGC:getPL:  cc_cenps size" << cc_cenps.count();
      for ( int ii = 0; ii < nholes; ii++ )
      {
         QString celnm       = cc_labls[ ii ]->text();
         QString centp       = cc_cenps[ ii ]->currentText();
         QString windo       = cc_winds[ ii ]->currentText();
DbgLv(1) << "EGC:getPL:   ii Text" << ii << centp;
         if ( ! centp.contains( tr( "empty" ) ) )
         {
            QString centry      = celnm + " : " + centp;
            if ( ( ii + 1 ) != nholes )
            {
               centry             += "  ( " + windo + " )";
            }
            value << centry;
         }
      }
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiCells::sibPValue( const QString sibling, const QString type )
{
   QString value( "" );
DbgLv(1) << "EGC:cPV: IN sibling" << sibling << "type" << type;
DbgLv(1) << "EGC:cPV: mainw" << mainw;
   if ( mainw != NULL )
   {
      value = ((US_ExperimentMain*)mainw)->childPValue( sibling, type );
   }

   return value;
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiCells::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPList( sibling, type )
            : QStringList() );
}

// Return status string for the panel
QString US_ExperGuiCells::status()
{
   int nholes          = sibPValue( "rotor", "nholes" ).toInt();
   int nempty          = 0;

   for ( int ii = 0; ii < nholes; ii++ )
   {
      if ( cc_cenps[ ii ]->currentText().contains( "empty" ) )
         nempty++;      // Keep count of "empty" centerpieces
   }

   bool is_done        = ( nempty < nholes );  // Done when not all empty
DbgLv(1) << "EGC:st: nholes nempty is_done" << nholes << nempty << is_done;
   return ( is_done ? QString( "4:X" ) : QString( "4:u" ) );
}
 
// Slot for change in centerpiece selection
void US_ExperGuiCells::centerpieceChanged( int sel )
{
DbgLv(1) << "EGC:cpChg: sel" << sel;
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   int irow            = sname.section( ":", 0, 0 ).toInt();
DbgLv(1) << "EGC:cpChg:  sname irow" << sname << irow;
   int nholes          = sibPValue( "rotor", "nholes" ).toInt();
   int icbal           = nholes - 1;    // Counter-balance index

   if ( irow != icbal )
   {  // Not counterbalance:  change cross cell
      int halfnh          = nholes / 2; // Half number holes
      int xrow            = ( irow < halfnh ) ? irow + halfnh : irow - halfnh;
      int jsel            = sel;        // Use same centerpiece for cross
DbgLv(1) << "EGC:cpChg:  xrow icbal" << xrow << icbal;

      if ( xrow == icbal )
      {  // Cross cell is counterbalance
         jsel                = 1;       // Usually "Beckman counterbalance"
         QString cpname      = cc_cenps[ irow ]->currentText();
DbgLv(1) << "EGC:cpChg:   cpname" << cpname << "tcb_centps" << tcb_centps;
         if ( tcb_centps.contains( cpname ) )
            jsel                = 2;    // In some cases "Titanium counterbalance"
      }

DbgLv(1) << "EGC:cpChg:   CB:jsel" << jsel;
      cc_cenps[ xrow ]->setCurrentIndex( jsel );
   }
}

// Slot for change in windows selection
void US_ExperGuiCells::windowsChanged( int sel )
{
DbgLv(1) << "EGC:wiChg: sel" << sel;
   QObject* sobj       = sender();   // Sender object
   QString sname       = sobj->objectName();
   int irow            = sname.section( ":", 0, 0 ).toInt();
DbgLv(1) << "EGC:wiChg:  sname irow" << sname << irow;
   int nholes          = sibPValue( "rotor", "nholes" ).toInt();
   int icbal           = nholes - 1;     // Counter-balance index

   if ( irow != icbal )
   {  // Not counterbalance:  change cross cell
      int halfnh          = nholes / 2;     // Half number holes
      int xrow            = ( irow < halfnh ) ? irow + halfnh : irow - halfnh;
      cc_winds[ xrow ]->setCurrentIndex( sel );
   }
}                  

// Panel for Solutions parameters
US_ExperGuiSolutions::US_ExperGuiSolutions( QWidget* topw )
{
   mainw               = topw;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner(
                            tr( "5: Specify a solution for each cell/channel" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   QPushButton* pb_manage   = us_pushbutton( tr( "Manage Solutions" ) );
   QPushButton* pb_details  = us_pushbutton( tr( "View Solution Details" ) );

   int row             = 1;
   genL->addWidget( pb_manage,       row,   0, 1, 3 );
   genL->addWidget( pb_details,      row++, 3, 1, 3 );
   QLabel* lb_hdr1     = us_banner( tr( "Cell / Channel" ) );
   QLabel* lb_hdr2     = us_banner( tr( "Solution" ) );
   genL->addWidget( lb_hdr1,         row,   0, 1, 1 );
   genL->addWidget( lb_hdr2,         row++, 1, 1, 5 );

   QStringList cpnames = sibPList( "cells", "centerpieces" );
   const int mxcels    = 8;
   int nholes          = sibPValue( "rotor", "nholes" ).toInt();
DbgLv(1) << "EGS:  nholes mxcels" << nholes << mxcels;

   QLabel*     cclabl;
   QComboBox*  cb_solu;

   allSolutions();        // Read in all solution names and IDs

   for ( int ii = 0; ii < 24; ii++ )
   {  // Loop to build initial place-holder solution rows
      QString scel;
      if      ( ii == 0 ) scel = QString( "0/A" );
      else if ( ii == 1 ) scel = QString( "0/B" );
      else if ( ii == 2 ) scel = QString( "9/A" );
      else if ( ii == 3 ) scel = QString( "9/B" );
      else                scel = QString( "none" );
      cclabl              = us_label( scel );
      cb_solu             = us_comboBox();

      QString strow       = QString::number( ii );
      cclabl ->setObjectName( strow + ": label" );
      cb_solu->setObjectName( strow + ": solution" );

      genL->addWidget( cclabl,  row,    0, 1, 1 );
      genL->addWidget( cb_solu, row++,  1, 1, 5 );

      cb_solu->addItems( sonames );

#if 0
      connect( cb_solu, SIGNAL( activated      ( int ) ),
               this,    SLOT  ( solutionChanged( int ) ) );
#endif
      bool is_vis          = ( ii < 4 );
      cclabl ->setVisible( is_vis );
      cb_solu->setVisible( is_vis );

      // Save pointers to row objects for later update
      cc_labls << cclabl;
      cc_solus << cb_solu;
   }

   connect( pb_manage,    SIGNAL( clicked()         ),
            this,         SLOT  ( manageSolutions() ) );
   connect( pb_details,   SIGNAL( clicked()         ),
            this,         SLOT  ( detailSolutions() ) );

   panel->addLayout( genL );
   panel->addStretch();

   initPanel();
};

// Initialize a panel, especially after clicking on its tab
void US_ExperGuiSolutions::initPanel()
{
   int nsrows          = 0;  // Count of visible solution rows
   QStringList cpnames = sibPList( "cells", "centerpieces" );
DbgLv(1) << "EGS:initP: cpnames" << cpnames;
   QStringList slabls;

   // Build channel labels based on cells information
   for ( int ii = 0; ii < cpnames.count(); ii++ )
   {
      QString cpname      = cpnames[ ii ];
      int chx             = cpname.indexOf( "-channel" );
DbgLv(1) << "EGS:initP:   ii" << ii << "cpname chx" << cpname << chx;
      if ( chx > 0 )
      {  // Non-empty cell centerpiece:  get channel count, build rows
         QString scell       = QString( cpname ).section( " ", 1, 1 );
         QString schans( "ABCDEF" );
         int nchan           = cpname.left( chx ).section( " ", -1, -1 )
                                 .simplified().toInt();
DbgLv(1) << "EGS:initP:     scell" << scell << "nchan" << nchan;
         for ( int jj = 0; jj < nchan; jj++ )
         {
            QString celchn      = scell + " / " + QString( schans ).mid( jj, 1 );
DbgLv(1) << "EGS:initP:      jj" << jj << "celchn" << celchn;
            slabls << celchn;
            nsrows++;
         }
      }
   }

   QString slabl;
   int nslabs          = nsrows;
   nsrows              = qMax( nsrows, 3 );  // Show at least 3 dummy rows
DbgLv(1) << "EGS:initP:  nslabs nsrows" << nslabs << nsrows
 << "k_labs k_sols" << cc_labls.count() << cc_solus.count();
 
   // Set cell/channel labels, make visible, all live rows
   for ( int ii = 0; ii < nsrows; ii++ )
   {
      slabl               = ( ii < nslabs ) ? slabls[ ii ] : tr( "none" );
DbgLv(1) << "EGS:initP:   ii" << ii << "slabl" << slabl;
      cc_labls[ ii ]->setText( slabl );
      cc_labls[ ii ]->setVisible( true );
      cc_solus[ ii ]->setVisible( true );
   }

   slabl               = tr( "none" );

   // Make remaining rows invisible
   for ( int ii = nsrows; ii < 24; ii++ )
   {
      cc_labls[ ii ]->setText( slabl );
      cc_labls[ ii ]->setVisible( false );
      cc_solus[ ii ]->setVisible( false );
   }

}

// Get a specific panel value
QString US_ExperGuiSolutions::getPValue( const QString type )
{
   QString value( "" );
   QString stat     = status();

   if ( type == "alldone" )
   {  // Status string
      value            = ( stat == "5:X" ) ? "1" : "0";
   }
   else if ( type == "nchant" )
   {  // Number channels total
      value               = QString::number( nchant );
   }
   else if ( type == "nchanu" )
   {  // Number channels unspecified
      value               = QString::number( nchanu );
   }
   else if ( type == "nchanf" )
   {  // Number channels filled with centerpiece
      value               = QString::number( nchanf );
   }
   else if ( type == "nusols" )
   {  // Number unique solutions given
      QString elabl       = tr( "none" );
      QString usolu       = tr( "(unspecified)" );
      QStringList solus;

      // Accumulate unique solutions specified
      for ( int ii = 0; ii < nchant; ii++ )
      {
         QString solu        = cc_solus[ ii ]->currentText();
         if ( ! solu.contains( usolu )  && ! solus.contains( solu ) )
            solus << solu;  // Add to list of unique solution names
      }
      // Return string:  number unique solutions specified
      value               = QString::number( solus.count() );
   }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiSolutions::getPList( const QString type )
{
   QStringList value;                      // Output list
   QStringList solus;                      // Unique solutions list
   QMap< QString, QStringList >  sochans;  // Solution-to-channels map

   status();

   if ( type == "solutions" )
   {
      QString usolu       = tr( "(unspecified)" );

      // Accumulate unique solutions specified, with map to channels
      for ( int ii = 0; ii < nchant; ii++ )
      {
         QString labl        = cc_labls[ ii ]->text();
         QString solu        = cc_solus[ ii ]->currentText();
         QStringList sochan;

         if ( ! solu.contains( usolu ) )
         {  // Solution is specified for the channel
            if ( solus.contains( solu ) )
            {  // Previously encountered solution
               sochan              = sochans[ solu ];  // Solution channels
               sochan << labl;                         // Add to them
               sochans[ solu ]     = sochan;           // Update
               
            }
            else
            {  // Newly encounted solution
               solus << solu;                          // Unique solution
               sochan << labl;                         // Begin solution channels
               sochans[ solu ]     = sochan;           // Update
            }
         }
      }

      // Form list of unique solutions with list of channels for each
      for ( int ii = 0; ii < solus.count(); ii++ )
      {
         QString solu        = solus[ ii ];            // Solution description
         QStringList sochan  = sochans[ solu ];        // Solution channels list
         solu               += "  ( " + sochan[ 0 ];   // Initial channel append
         // Append channels list to solution entry
         for ( int jj = 1; jj < sochan.count(); jj++ )
         {
            solu               += ", " + sochan[ jj ]; // Build channel list
         }
         solu               += "  )";                  // Close out channels string
         // Add to the output list
         value << solu;
      }
   }

   else if ( type == "channel_solutions" )
   {
      QString usolu       = tr( "(unspecified)" );

      // Accumulate list of strings in "cell/chan : solution" form
      for ( int ii = 0; ii < nchant; ii++ )
      {
         QString labl        = cc_labls[ ii ]->text();
         QString solu        = cc_solus[ ii ]->currentText();
         QString sochan      = labl + " : " + solu;

         if ( ! solu.contains( usolu ) )   // Not unspecified
            value << sochan;               // Append to the output list
      }
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiSolutions::sibPValue( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPValue( sibling, type )
            : QString("") );
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiSolutions::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPList( sibling, type )
            : QStringList() );
}

// Return status string for the panel
QString US_ExperGuiSolutions::status()
{
   mxrow               = 24;     // Maximum possible rows
   nchant              = 0;      // Number channels total
   nchanu              = 0;      // Number channels unspecified
   nchanf              = 0;      // Number channels filled
   QString elabl       = tr( "none" );
   QString usolu       = tr( "(unspecified)" );

   // Count rows, those unspecified, those filled
   for ( int ii = 0; ii < mxrow; ii++ )
   {
      if ( cc_labls[ ii ]->text() == elabl )
         break;        // The "none" label is end of used channels

      nchant++;        // Bump cell/channel count to fill

      if ( cc_solus[ ii ]->currentText().contains( usolu ) )
         nchanu++;     // Bump "(unspecified)" Solution count
      else
         nchanf++;     // Bump filled Solution count
   }

   bool is_done        = ( nchant > 0  &&  nchanu < 1 );
DbgLv(1) << "EGS:st: nchant nchanu nchanf is_done"
 << nchant << nchanu << nchanf << is_done;
   return ( is_done ? QString( "5:X" ) : QString( "5:u" ) );
}

// Slot to open a dialog for managing solutions
void US_ExperGuiSolutions::manageSolutions()
{
   US_SolutionGui* sdiag = new US_SolutionGui;
   sdiag->show();
}

// Slot to open a dialog for showing details about solutions
void US_ExperGuiSolutions::detailSolutions()
{
   // Create a new editor text dialog with fixed font
   US_Editor* ediag = new US_Editor( US_Editor::DEFAULT, true, "", this );
   ediag->setWindowTitle( tr( "Details on Selected Solutions" ) );
   ediag->resize( 720, 440 );
   ediag->e->setFont( QFont( US_Widgets::fixedFont().family(),
                             US_GuiSettings::fontSize() - 1,
                             QFont::Bold ) );
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
QFont ufont=ediag->e->font();
DbgLv(1) << "EGS:detS: ufont" << ufont.family();

   // Accumulate information on solutions that are currently selected
   QStringList sdescrs;
   QMap< QString, QString > chanuse;
   QString usolu       = tr( "(unspecified)" );

   for ( int ii = 0; ii < cc_solus.count(); ii++ )
   {  // Build up information for each active solution row
      QComboBox* cbsolu  = cc_solus[ ii ];
      if ( ! cbsolu->isVisible() )     // Break when invisible row reached
         break;

      QString sdescr     = cbsolu->currentText();  // Solution description
DbgLv(1) << "EGS:detS:    ii" << ii << "solu" << sdescr;

      if ( sdescr.contains( usolu ) )  // Skip around "(unspecified)"
         continue;

      QString chanu      = cc_labls[ ii ]->text();
      if ( ! sdescrs.contains( sdescr ) )
      {  // Add solution description to list and begin channels-used list
         sdescrs << sdescr;
         chanuse[ sdescr ] = chanu;
DbgLv(1) << "EGS:detS:      chanu" << chanu;
      }
      else
      {  // Append to channels-used for solution
         chanuse[ sdescr ] = chanuse[ sdescr ] + ",  " + chanu;
DbgLv(1) << "EGS:detS:      chanu" << chanuse[sdescr];
      }
   }

   // Start composing the text that it displays
   QStringList atypes;
   atypes << "PROTEIN" << "DNA" << "RNA" << "CARBOHYDRATE";
   QString dtext  = tr( "Solutions currently selected:\n\n" );

   for ( int ii = 0; ii < sdescrs.count(); ii++ )
   {
      US_Solution soludata;
      QString sdescr     = sdescrs[ ii ];
      solutionData( sdescr, soludata );

      dtext   += tr( "  Solution:      " ) + sdescr + "\n";
      dtext   += tr( "    Buffer:        " ) + soludata.buffer.description + "\n";
      for ( int jj = 0; jj < soludata.buffer.component.count(); jj++ )
      {
         dtext   += tr( "      Component:     " )
                    + soludata.buffer.component[ jj ].name + "\n";
         dtext   += tr( "       concentration:  %1 " )
                    .arg( soludata.buffer.concentration[ jj ] )
                    + soludata.buffer.component[ jj ].unit + "\n";
      }

      for ( int jj = 0; jj < soludata.analyteInfo.count(); jj++ )
      {
         dtext   += tr( "    Analyte:       " )
                    + soludata.analyteInfo[ jj ].analyte.description + "\n";
         dtext   += tr( "      type:          " )
                    + atypes[ soludata.analyteInfo[ jj ].analyte.type ] + "\n";
         dtext   += tr( "      molar ratio:   %1\n" )
                    .arg( soludata.analyteInfo[ jj ].amount );
      }

      dtext   += tr( "    Channels used:  " ) + chanuse[ sdescr ] + "\n";
      dtext   += "\n";
   }

   // Load text and show the dialog
   QApplication::restoreOverrideCursor();
   qApp->processEvents();

   ediag->e->setText( dtext );
   ediag->show();
}

// Return the solution ID/GUID for a given solution description
bool US_ExperGuiSolutions::solutionID( const QString sdescr,
      QString& solID )
{
   bool found    = solu_ids.keys().contains( sdescr );
   solID         = found ? solu_ids[ sdescr ] : solID;

   return found;
}

// Return the solution object for a given solution description
bool US_ExperGuiSolutions::solutionData( const QString sdescr,
      US_Solution& soludata )
{
   QString solID = QString( "" );
   bool found    = solutionID( sdescr, solID );
DbgLv(1) << "EGS:solDat:  sdescr" << sdescr << "found" << found
 << "solID" << solID;

   if ( found )
   {
      if ( solu_data.keys().contains( sdescr ) )
      {  // Previously fetched and mapped:  just return it
         soludata      = solu_data[ sdescr ];
DbgLv(1) << "EGS:solDat:    OLDfound descr" << soludata.solutionDesc;
      }

      else
      {  // Not fetched before:  do so now and map it
         US_Passwd pw;
         US_DB2* dbP   = ( sibPValue( "general", "dbdisk" ) == "DB" )
                         ? new US_DB2( pw.getPasswd() ) : NULL;
         if ( dbP != NULL )
         {
            int stat      = soludata.readFromDB( solID.toInt(), dbP );
            found         = ( stat == US_DB2::OK );
DbgLv(1) << "EGS:solDat:    NEWfound descr" << soludata.solutionDesc << stat;
         }
         else
         {
            soludata.readFromDisk( solID );
         }
      }
   }

   return found;
}

// Build a mapping of all solution descriptions to solution IDs
int US_ExperGuiSolutions::allSolutions()
{
   sonames.clear();
   sonames << "(unspecified)";

   US_Passwd pw;
   US_DB2* dbP       = ( sibPValue( "general", "dbdisk" ) == "DB" )
                       ? new US_DB2( pw.getPasswd() ) : NULL;
   if ( dbP != NULL )
   {  // Read all the solutions in the database
      QString invID     = sibPValue( "general", "investigator" )
                             .section( ":", 0, 0 ).simplified();
      QStringList qry;
      qry << "all_solutionIDs" << invID;
      dbP->query( qry );

      if ( dbP->lastErrno() != US_DB2::OK )
      {
         qDebug() << "*ERROR* allSolutions error" << dbP->lastErrno();
         return 0;
      }

      while ( dbP->next() )
      {
         QString solID     = dbP->value( 0 ).toString();
         QString descr     = dbP->value( 1 ).toString();

         if ( descr.isEmpty() )
            continue;

         solu_ids[ descr ] = solID;
         sonames << descr;
      }
   }  // END: solutions in DB

   else
   {  // Read all the solutions on the local disk
      QString path;
      US_Solution solution;
      if ( ! solution.diskPath( path ) )
         return 0;

      QDir dir( path );
      QStringList filter( "S*.xml" );
      QStringList fnames = dir.entryList( filter, QDir::Files,
                                          QDir::Name );
      QFile s_file;
      QString solID;
      QString descr;

      for ( int ii = 0; ii < fnames.size(); ii++ )
      {  // Examine each S*.xml file
         s_file.setFileName( path + "/" + fnames[ ii ] );

         if ( ! s_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QXmlStreamReader xmli( &s_file );

         while ( ! xmli.atEnd() )
         {
            xmli.readNext();

            if ( xmli.isStartElement() )
            {
               QXmlStreamAttributes attrs = xmli.attributes();

               if ( xmli.name() == "solution" )
               {
                  solID             = attrs.value( "guid" ).toString();
               }

               else if ( xmli.name() == "description" )
               {
                  xmli.readNext();
                  descr             = xmli.text().toString();

                  if ( descr.isEmpty() )
                     continue;

                  solu_ids[ descr ] = solID;
                  sonames << descr;
               }
            }  // END: Start element
         }  // END: XML element loop
      }  // END: file names loop

      s_file.close();
   }  // END: solutions on local disk

   return solu_ids.keys().count();
}


// Panel for Optical Systems parameters
US_ExperGuiOptical::US_ExperGuiOptical( QWidget* topw )
{
   mainw               = topw;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "6: Specify optical system scans for each channel" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   QPushButton* pb_manage   = us_pushbutton( tr( "Manage Optical Systems" ) );
   QPushButton* pb_details  = us_pushbutton( tr( "View Optical Details" ) );

   int row             = 1;
   genL->addWidget( pb_manage,       row,   0, 1, 2 );
   genL->addWidget( pb_details,      row++, 2, 1, 2 );
   QLabel* lb_hdr1     = us_banner( tr( "Cell / Channel" ) );
   QLabel* lb_hdr2     = us_banner( tr( "Optical System Scans to Perform" ) );
   genL->addWidget( lb_hdr1,         row,   0, 1, 1 );
   genL->addWidget( lb_hdr2,         row++, 1, 1, 3 );

   const int mxcels    = 8;
   int nholes          = sibPValue( "rotor", "nholes" ).toInt();
DbgLv(1) << "EGO:  nholes mxcels" << nholes << mxcels;

   QLabel*        cclabl;
   QCheckBox*     ck_osys1;
   QCheckBox*     ck_osys2;
   QCheckBox*     ck_osys3;
   QHBoxLayout*   lo_osyss;
   QButtonGroup*  bg_osyss;
   QPalette ckpal   = US_GuiSettings::normalColor();
   QFont    ckfont  = QFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize  (),
                             QFont::Bold );
   //QFontMetrics fm( ckfont );
   QStringList optentr = US_Settings::defaultXpnHost();
   int nopten       = optentr.count();
   int nopsys       = nopten - 6;
DbgLv(1) << "EGO:main: optentr" << optentr;
DbgLv(1) << "EGO:main:  nopten" << nopten << "nopsys" << nopsys;
   QString opsys1   = ( nopten > 6 ) ? optentr[ 6 ]
                                     : tr( "UV/visible" );
   QString opsys2   = ( nopten > 7 ) ? optentr[ 7 ]
                                     : tr( "Rayleigh Interference" );
   QString opsys3   = ( nopten > 8 ) ? optentr[ 8 ]
                                     : tr( "(not installed)" );
DbgLv(1) << "EGO:main:   opsys1-3" << opsys1 << opsys2 << opsys3;
   int nckopt       = 0;
   nckopt          += opsys1.contains( tr( "not installed" ) ) ? 0 : 1;
   nckopt          += opsys2.contains( tr( "not installed" ) ) ? 0 : 1;
   nckopt          += opsys3.contains( tr( "not installed" ) ) ? 0 : 1;

   for ( int ii = 0; ii < 24; ii++ )
   {  // Loop to build initial place-holder optical rows
      QString scel;
      if      ( ii == 0 ) scel = QString( "0/A" );
      else if ( ii == 1 ) scel = QString( "0/B" );
      else if ( ii == 2 ) scel = QString( "9/A" );
      else if ( ii == 3 ) scel = QString( "9/B" );
      else                scel = QString( "none" );
      cclabl           = us_label( scel );

      ck_osys1         = new QCheckBox( opsys1, this );
      ck_osys2         = new QCheckBox( opsys2, this );
      ck_osys3         = new QCheckBox( opsys3, this );
      ck_osys1->setFont   ( ckfont );
      ck_osys1->setPalette( ckpal );
      ck_osys1->setChecked( false );
      ck_osys1->setAutoFillBackground( true );
      ck_osys2->setFont   ( ckfont );
      ck_osys2->setPalette( ckpal );
      ck_osys2->setChecked( false );
      ck_osys2->setAutoFillBackground( true );
      ck_osys3->setFont   ( ckfont );
      ck_osys3->setPalette( ckpal );
      ck_osys3->setChecked( false );
      ck_osys3->setAutoFillBackground( true );
      lo_osyss         = new QHBoxLayout;
      bg_osyss         = new QButtonGroup;
      bg_osyss->setExclusive( false );
      bg_osyss->addButton( ck_osys1, 1 );
      bg_osyss->addButton( ck_osys2, 2 );
      bg_osyss->addButton( ck_osys3, 3 );
      lo_osyss->addWidget( ck_osys1 );
      lo_osyss->addWidget( ck_osys2 );
      lo_osyss->addWidget( ck_osys3 );

      QString strow    = QString::number( ii );
      cclabl  ->setObjectName( strow + ": label" );
      ck_osys1->setObjectName( strow + ": optsys1" );
      ck_osys2->setObjectName( strow + ": optsys2" );
      ck_osys3->setObjectName( strow + ": optsys3" );

      genL->addWidget( cclabl,   row,    0, 1, 1 );
      genL->addLayout( lo_osyss, row++,  1, 1, 3 );

      connect( ck_osys1, SIGNAL( toggled     ( bool ) ),
               this,     SLOT  ( opsysChecked( bool ) ) );
      connect( ck_osys2, SIGNAL( toggled     ( bool ) ),
               this,     SLOT  ( opsysChecked( bool ) ) );
      connect( ck_osys3, SIGNAL( toggled     ( bool ) ),
               this,     SLOT  ( opsysChecked( bool ) ) );

      bool is_vis          = ( ii < 4 );
      cclabl  ->setVisible( is_vis );
DbgLv(1) << "EGO:main:    ii" << ii << "is_vis nckopt" << is_vis << nckopt;
      bg_osyss->button( 1 )->setVisible( is_vis && ( nckopt > 0 ) );
      bg_osyss->button( 2 )->setVisible( is_vis && ( nckopt > 1 ) );
      bg_osyss->button( 3 )->setVisible( is_vis && ( nckopt > 2 ) );

      // Save pointers to row objects for later update
      cc_labls << cclabl;
      cc_osyss << bg_osyss;
   }

#if 0
   connect( pb_manage,    SIGNAL( clicked()         ),
            this,         SLOT  ( manageSolutions() ) );
#endif
   connect( pb_details,   SIGNAL( clicked()       ),
            this,         SLOT  ( detailOptical() ) );

   panel->addLayout( genL );
   panel->addStretch();
};

// Initialize a panel, especially after clicking on its tab
void US_ExperGuiOptical::initPanel()
{
   int nsrows          = 0;  // Count of visible optical rows
   QStringList cpnames = sibPList( "cells", "centerpieces" );
DbgLv(1) << "EGO:initP: cpnames" << cpnames;
   QStringList slabls;

   // Build channel labels based on cells information
   for ( int ii = 0; ii < cpnames.count(); ii++ )
   {
      QString cpname   = cpnames[ ii ];
      int chx          = cpname.indexOf( "-channel" );
DbgLv(1) << "EGO:initP:   ii" << ii << "cpname chx" << cpname << chx;
      if ( chx > 0 )
      {  // Non-empty cell centerpiece:  get channel count, build rows
         QString scell    = QString( cpname ).section( " ", 1, 1 );
         QString schans( "ABCDEF" );
         int nchan        = cpname.left( chx ).section( " ", -1, -1 )
                              .simplified().toInt();
DbgLv(1) << "EGO:initP:     scell" << scell << "nchan" << nchan;
         for ( int jj = 0; jj < nchan; jj++ )
         {
            QString celchn   = scell + " / " + QString( schans ).mid( jj, 1 );
DbgLv(1) << "EGO:initP:      jj" << jj << "celchn" << celchn;
            slabls << celchn;
            nsrows++;
         }
      }
      
   }

   QString slabl;
   int nslabs       = nsrows;
   nsrows           = qMax( nsrows, 3 );  // Show at least 3 dummy rows
   QString unavail  = tr( "(unavailable)" );
DbgLv(1) << "EGO:initP:  nslabs nsrows" << nslabs << nsrows
 << "k_labs k_sys1" << cc_labls.count() << cc_osyss.count();
 
   // Set cell/channel labels and checkboxes; make visible, all live rows
   for ( int ii = 0; ii < nsrows; ii++ )
   {
      slabl            = ( ii < nslabs ) ? slabls[ ii ] : tr( "none" );
      cc_labls[ ii ]->setText( slabl );
      cc_labls[ ii ]->setVisible( true );
      QCheckBox* ckbox1 = (QCheckBox*)cc_osyss[ ii ]->button( 1 );
      QCheckBox* ckbox2 = (QCheckBox*)cc_osyss[ ii ]->button( 2 );
      QCheckBox* ckbox3 = (QCheckBox*)cc_osyss[ ii ]->button( 3 );
      ckbox1->setVisible( ! ckbox1->text().contains( unavail ) );
      ckbox2->setVisible( ! ckbox2->text().contains( unavail ) );
      ckbox3->setVisible( ! ckbox3->text().contains( unavail ) );
DbgLv(1) << "EGO:initP:   ii" << ii << "slabl" << slabl << "boxtexts"
 << ckbox1->text() << ckbox2->text() << ckbox3->text();
   }

   slabl            = tr( "none" );

   // Make remaining rows invisible
   for ( int ii = nsrows; ii < 24; ii++ )
   {
      cc_labls[ ii ]->setText( slabl );
      cc_labls[ ii ]->setVisible( false );
      cc_osyss[ ii ]->button( 1 )->setVisible( false );
      cc_osyss[ ii ]->button( 2 )->setVisible( false );
      cc_osyss[ ii ]->button( 3 )->setVisible( false );
   }
}

// Get a specific panel value
QString US_ExperGuiOptical::getPValue( const QString type )
{
   QString value( "" );

   if ( type == "eprofiles" )
   {
   //   value = cb_lab->currentText();
   }
   else if ( type == "alldone" )
   {
      QString stat     = status();
      value            = ( stat == "6:X" ) ? "1" : "0";
   }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiOptical::getPList( const QString type )
{
   QStringList value( "" );

   if ( type == "eprofiles" )
   {
      //value << le_runid->text();
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiOptical::sibPValue( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPValue( sibling, type )
            : QString("") );
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiOptical::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPList( sibling, type )
            : QStringList() );
}

// Return status string for the panel
QString US_ExperGuiOptical::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
is_done=true;
   return ( is_done ? QString( "6:X" ) : QString( "6:u" ) );
}
                   
// Slot to manage extinction profiles
void US_ExperGuiOptical::manageEProfiles()
{
DbgLv(1) << "EGO: mEP: IN";
   //US_Extinction ediag( "BUFFER", "some buffer", this );
   US_Extinction* ediag = new US_Extinction;
   ediag->setParent(   this, Qt::Window );
   ediag->setAttribute( Qt::WA_DeleteOnClose );

   connect( ediag,  SIGNAL( get_results(     QMap<double,double>& ) ),
            this,   SLOT  ( process_results( QMap<double,double>& ) ) );

   ediag->show();
}

// Slot to handle an optical system being checked
void US_ExperGuiOptical::opsysChecked( bool checked )
{
DbgLv(1) << "EGO: oCk: checked" << checked;
   // Determine which row and which of 3 possible check boxes
   QObject* sobj       = sender();      // Sender object
   QString oname       = sobj->objectName();
   int irow            = oname.section( ":", 0, 0 ).toInt();
   int ibtn            = oname.mid( oname.length() - 1 ).toInt();
DbgLv(1) << "EGO: oCk:  oname" << oname << "irow" << irow << "ibtn" << ibtn;
   // Ignore if not Interference
   QString optype      = cc_osyss[ irow ]->button( ibtn )->text();
DbgLv(1) << "EGO: oCk:   optype" << optype;
   if ( optype != tr( "Rayleigh Interference" ) )
      return;

   // If Interference, get a list of same-cell rows; disconnect
   QString clabl       = cc_labls[ irow ]->text();
   QString scell       = clabl.left( 1 );
   QString labnone     = tr( "none" );
   QList< int >  ccrows;
DbgLv(1) << "EGO: oCk:    clabl" << clabl << "scell" << scell;
   for ( int ii = 0; ii < 24; ii++ )
   {
      // Ignore the exact same row
      if ( ii == irow )
         continue;
      // Get row label and quit loop when at end visible rows
      QString rlabl       = cc_labls[ ii ]->text();
      if ( rlabl == labnone )
         break;
      // Compare the cell value to that of the one (un)checked
      QString rcell       = rlabl.left( 1 );
      if ( rcell == scell )
      {  // Save same-cell row and disconnect the checkbox
         ccrows << ii;
         cc_osyss[ ii ]->button( ibtn )->disconnect();
DbgLv(1) << "EGO: oCk:     ii" << ii << "rlabl" << rlabl;
      }
   }
DbgLv(1) << "EGO: oCk: ccrows" << ccrows;

   // Set check-state of Interference boxes in same-cell rows and reconnect
   for ( int ii = 0; ii < ccrows.count(); ii++ )
   {
      int ccrow           = ccrows[ ii ];
      QCheckBox* ckbox    = (QCheckBox*)cc_osyss[ ccrow ]->button( ibtn );
      ckbox->setChecked( checked );

      connect( ckbox,  SIGNAL( toggled     ( bool ) ),
               this,   SLOT  ( opsysChecked( bool ) ) );
   }
}

// Slot to show details of all photo multiplier controls
void US_ExperGuiOptical::process_results( QMap< double, double >& eprof )
{
DbgLv(1) << "EGPM: pr: eprof size" << eprof.keys().count();
}

// Slot to show details of all photo multiplier controls
void US_ExperGuiOptical::detailOptical()
{
   // Create a new editor text dialog with fixed font
   US_Editor* ediag = new US_Editor( US_Editor::DEFAULT, true, "", this );
   ediag->setWindowTitle( tr( "Details on Optical Systems" ) );
   ediag->resize( 720, 440 );
   ediag->e->setFont( QFont( US_Widgets::fixedFont().family(),
                             US_GuiSettings::fontSize() - 1,
                             QFont::Bold ) );
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Accumulate information on solutions that are currently selected

   // Start composing the text that it displays
   QString dtext  = tr( "Optical System Scans Information:\n\n" );

dtext+= "NONE (not yet implemented)\n";

   // Load text and show the dialog
   QApplication::restoreOverrideCursor();
   qApp->processEvents();

   ediag->e->setText( dtext );
   ediag->show();
}


// Panel for Spectra parameters
US_ExperGuiSpectra::US_ExperGuiSpectra( QWidget* topw )
{
   mainw               = topw;
   int mxrow           = 24;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "7: Specify wavelength/value spectra" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   QPushButton* pb_details  = us_pushbutton( tr( "View Current Spectra Settings" ) );
   connect( pb_details,   SIGNAL( clicked()       ),
            this,         SLOT  ( detailSpectra() ) );

   QLabel* lb_hdr1     = us_banner( tr( "Cell / Channel" ) );
   QLabel* lb_hdr2     = us_banner( tr( "Wavelengths" ) );
   QLabel* lb_hdr3     = us_banner( tr( "Spectra Input" ) );
   QLabel* lb_hdr4     = us_banner( tr( "All Specified?" ) );

   int row             = 0;
   genL->addWidget( pb_details,      row++, 3, 1, 3 );
   genL->addWidget( lb_hdr1,         row,   0, 1, 1 );
   genL->addWidget( lb_hdr2,         row,   1, 1, 1 );
   genL->addWidget( lb_hdr3,         row,   2, 1, 3 );
   genL->addWidget( lb_hdr4,         row++, 5, 1, 1 );

   QLabel*      cclabl;
   QPushButton* pbwavln;
   QPushButton* pbsload;
   QPushButton* pbsmanu;
   QCheckBox*   ckoptim;
   QCheckBox*   cksdone;
   QString swavln   = tr( "Select Wavelengths" );
   QString soptim   = tr( "Auto in Optima" );
   QString sloads   = tr( "Load Spectrum" );
   QString smanus   = tr( "Manual Spectrum" );
   QString ssdone   = tr( "Done" );
   QFont   ckfont   = QFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize  (),
                             QFont::Bold );
   QPalette ckpal   = US_GuiSettings::normalColor();

   for ( int ii = 0; ii < mxrow; ii++ )
   {  // Loop to build initial place-holder spectr rows
      QString scel;
      if      ( ii == 0 ) scel = QString( "1 / A" );
      else if ( ii == 1 ) scel = QString( "1 / B" );
      else if ( ii == 2 ) scel = QString( "2 / A" );
      else if ( ii == 3 ) scel = QString( "2 / B" );
      else                scel = QString( "none" );
      cclabl           = us_label( scel );
      pbwavln          = us_pushbutton( swavln );
      ckoptim          = new QCheckBox( soptim, this );
      pbsload          = us_pushbutton( sloads );
      pbsmanu          = us_pushbutton( smanus );
      cksdone          = new QCheckBox( ssdone, this );
      cksdone->setEnabled( false );
      QString strow    = QString::number( ii );
      cclabl ->setObjectName( strow + ": label" );
      pbwavln->setObjectName( strow + ": pb_wavln" );
      ckoptim->setObjectName( strow + ": ck_optim" );
      pbsload->setObjectName( strow + ": pb_sload" );
      pbsmanu->setObjectName( strow + ": pb_smanu" );
      cksdone->setObjectName( strow + ": ck_sdone" );

      ckoptim->setFont   ( ckfont );
      ckoptim->setPalette( ckpal );
      ckoptim->setChecked( false );
      ckoptim->setAutoFillBackground( true );
      cksdone->setFont   ( ckfont );
      cksdone->setPalette( ckpal );
      cksdone->setChecked( false );
      cksdone->setAutoFillBackground( true );

      bool is_vis      = ( ii < 4 );

      genL->addWidget( cclabl,  row,   0, 1, 1 );
      genL->addWidget( pbwavln, row,   1, 1, 1 );
      genL->addWidget( ckoptim, row,   2, 1, 1 );
      genL->addWidget( pbsload, row,   3, 1, 1 );
      genL->addWidget( pbsmanu, row,   4, 1, 1 );
      genL->addWidget( cksdone, row++, 5, 1, 1 );

      cclabl ->setVisible( is_vis );
      pbwavln->setVisible( is_vis );
      ckoptim->setVisible( is_vis );
      pbsload->setVisible( is_vis );
      pbsmanu->setVisible( is_vis );
      cksdone->setVisible( is_vis );

      connect( pbwavln, SIGNAL( clicked()           ),
               this,    SLOT  ( selectWavelengths() ) );
      connect( ckoptim, SIGNAL( toggled    ( bool ) ),
               this,    SLOT  ( checkOptima( bool ) ) );
      connect( pbsload, SIGNAL( clicked()           ),
               this,    SLOT  ( loadSpectrum()      ) );
      connect( pbsmanu, SIGNAL( clicked()           ),
               this,    SLOT  ( manualSpectrum()    ) );

      cc_labls << cclabl;
      cc_wavls << pbwavln;
      cc_optis << ckoptim;
      cc_loads << pbsload;
      cc_manus << pbsmanu;
      cc_dones << cksdone;
   }

   panel->addLayout( genL );
   panel->addStretch();
};

// Initialize a panel, especially after clicking on its tab
void US_ExperGuiSpectra::initPanel()
{
}

// Get a specific panel value
QString US_ExperGuiSpectra::getPValue( const QString type )
{
   QString value( "" );

   if ( type == "eprofiles" )
   {
   //   value = cb_lab->currentText();
   }
   else if ( type == "alldone" )
   {
      QString stat     = status();
      value            = ( stat == "7:X" ) ? "1" : "0";
   }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiSpectra::getPList( const QString type )
{
   QStringList value( "" );

   if ( type == "eprofiles" )
   {
      //value << le_runid->text();
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiSpectra::sibPValue( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPValue( sibling, type )
            : QString("") );
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiSpectra::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPList( sibling, type )
            : QStringList() );
}

// Return status string for the panel
QString US_ExperGuiSpectra::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
is_done=true;
   return ( is_done ? QString( "7:X" ) : QString( "7:u" ) );
}
                   
// Slot to manage extinction profiles
void US_ExperGuiSpectra::manageEProfiles()
{
DbgLv(1) << "EGPM: mEP: IN";
   //US_Extinction ediag( "BUFFER", "some buffer", this );
   US_Extinction* ediag = new US_Extinction;
   ediag->setParent(   this, Qt::Window );
   ediag->setAttribute( Qt::WA_DeleteOnClose );

   connect( ediag,  SIGNAL( get_results(     QMap<double,double>& ) ),
            this,   SLOT  ( process_results( QMap<double,double>& ) ) );

   ediag->show();
}

// Slot to show details of all photo multiplier controls
void US_ExperGuiSpectra::process_results( QMap< double, double >& eprof )
{
DbgLv(1) << "EGPM: pr: eprof size" << eprof.keys().count();
}

// Slot to show details of all wavelength and spectra profiles
void US_ExperGuiSpectra::detailSpectra()
{
   // Create a new editor text dialog with fixed font
   US_Editor* ediag = new US_Editor( US_Editor::DEFAULT, true, "", this );
   ediag->setWindowTitle( tr( "Details: Wavelength / Extinction|Voltage|Gain Profiles" ) );
   ediag->resize( 720, 440 );
   ediag->e->setFont( QFont( US_Widgets::fixedFont().family(),
                             US_GuiSettings::fontSize() - 1,
                             QFont::Bold ) );
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Accumulate information on solutions that are currently selected

   // Start composing the text that it displays
   QString dtext  = tr( "Wavelength/Voltage Profile Information:\n\n" );

dtext+= "NONE (not yet implemented)\n";

   // Load text and show the dialog
   QApplication::restoreOverrideCursor();
   qApp->processEvents();

   ediag->e->setText( dtext );
   ediag->show();
}

// Slot to select wavelengths using a dialog
void US_ExperGuiSpectra::selectWavelengths()
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   int irow            = sname.section( ":", 0, 0 ).toInt();
   QString cclabl      = cc_labls[ irow ]->text();
DbgLv(1) << "EGS:selWl: sname" << sname << "irow" << irow << cclabl;
QString mtitle  = tr( "Not Yet Implemented" );
QString message = tr( "The ability to select wavelengths for " )
 + cclabl + "\n" + tr( "has not yet been implement" );
QMessageBox::information( this, mtitle, message );
}

// Slot to handle the toggle of an "in Optima" checkbox
void US_ExperGuiSpectra::checkOptima( bool checked )
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   int irow            = sname.section( ":", 0, 0 ).toInt();
   QString cclabl      = cc_labls[ irow ]->text();
DbgLv(1) << "EGS:ckOpt: sname" << sname << "irow" << irow << cclabl;
   cc_loads[ irow ]->setEnabled( !checked );
   cc_manus[ irow ]->setEnabled( !checked );
   cc_dones[ irow ]->setChecked(  checked );
//      cc_labls << cclabl;
//      cc_wavls << pbwavln;
//      cc_optis << ckoptim;
//      cc_loads << pbsload;
//      cc_manus << pbsmanu;
//      cc_dones << cksdone;
}

// Slot to load a spectrum profile using us_extinction
void US_ExperGuiSpectra::loadSpectrum()
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   int irow            = sname.section( ":", 0, 0 ).toInt();
   QString cclabl      = cc_labls[ irow ]->text();
DbgLv(1) << "EGS:loadS: sname" << sname << "irow" << irow << cclabl;
   //US_Extinction ediag( "BUFFER", "some buffer", this );
   US_Extinction* ediag = new US_Extinction;
   ediag->setParent(   this, Qt::Window );
   ediag->setAttribute( Qt::WA_DeleteOnClose );

   //connect( ediag,  SIGNAL( get_results(     QMap<double,double>& ) ),
   //         this,   SLOT  ( process_results( QMap<double,double>& ) ) );

   ediag->show();
}

// Slot to manually enter a spectrum profile using us_table
void US_ExperGuiSpectra::manualSpectrum()
{
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   int irow            = sname.section( ":", 0, 0 ).toInt();
   QString cclabl      = cc_labls[ irow ]->text();
DbgLv(1) << "EGS:manSp: sname" << sname << "irow" << irow << cclabl;
   QMap< double, double >  extinction;
   QString strExtinc   = tr( "Extinction:" );
   bool changed        = false;
   US_Table* tdiag     = new US_Table( extinction, strExtinc, changed );
   tdiag->setWindowTitle( tr( "Manually Enter Spectrum (%1)" )
                          .arg( cclabl ) );
   tdiag->exec();
}


// Panel for Uploading parameters to Optima DB
US_ExperGuiUpload::US_ExperGuiUpload( QWidget* topw )
{
   mainw               = topw;
   uploaded            = false;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "7: Upload experiment parameters"
                                        " to Optima DB" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   // Push buttons
   QPushButton* pb_details  = us_pushbutton( tr( "View Experiment Details" ) );
   QPushButton* pb_connect  = us_pushbutton( tr( "Test Connection" ) );
                pb_upload   = us_pushbutton( tr( "Upload to XLA" ) );

   pb_upload->setEnabled( false );

   // Check boxes showing current completed parameterizations
   QLayout* lo_run          = us_checkbox( tr( "RunID" ),
                                           ck_run,      false );
   QLayout* lo_project      = us_checkbox( tr( "Project" ),
                                           ck_project,  false );
   QLayout* lo_rotor        = us_checkbox( tr( "Lab/Rotor/Calibration" ),
                                           ck_rotor,    true  );
   QLayout* lo_rotor_ok     = us_checkbox( tr( "Rotor specified" ),
                                           ck_rotor_ok, false );
   QLayout* lo_speed        = us_checkbox( tr( "Speed Steps" ),
                                           ck_speed,    true  );
   QLayout* lo_speed_ok     = us_checkbox( tr( "Speed specified" ),
                                           ck_speed_ok, false );
   QLayout* lo_centerp      = us_checkbox( tr( "some Cell Centerpieces" ),
                                           ck_centerp,  false );
   QLayout* lo_solution     = us_checkbox( tr( "all Channel Solutions" ),
                                           ck_solution, false );
   QLayout* lo_extprofs     = us_checkbox( tr( "Spectra" ),
                                           ck_extprofs, false );
   QLayout* lo_connect      = us_checkbox( tr( "Connected" ),
                                           ck_connect,  false );
   QLayout* lo_upl_enab     = us_checkbox( tr( "Upload Enabled" ),
                                           ck_upl_enab, false );
   QLayout* lo_upl_done     = us_checkbox( tr( "Upload Completed" ),
                                           ck_upl_done, false );
   ck_run     ->setEnabled( false );
   ck_project ->setEnabled( false );
   ck_rotor   ->setEnabled( false );
   ck_rotor_ok->setEnabled( true  );
   ck_speed   ->setEnabled( false );
   ck_speed_ok->setEnabled( true  );
   ck_centerp ->setEnabled( false );
   ck_solution->setEnabled( false );
   ck_extprofs->setEnabled( false );
   ck_connect ->setEnabled( false );
   ck_upl_enab->setEnabled( false );
   ck_upl_done->setEnabled( false );

   // Build the GUI elements
   int row             = 1;
   genL->addWidget( pb_details,      row,   0, 1, 2 );
   genL->addWidget( pb_connect,      row,   2, 1, 2 );
   genL->addWidget( pb_upload,       row++, 4, 1, 2 );

   genL->addLayout( lo_run,          row,   1, 1, 2 );
   genL->addLayout( lo_project,      row++, 3, 1, 2 );
   genL->addLayout( lo_rotor,        row,   1, 1, 2 );
   genL->addLayout( lo_rotor_ok,     row++, 3, 1, 2 );
   genL->addLayout( lo_speed,        row,   1, 1, 2 );
   genL->addLayout( lo_speed_ok,     row++, 3, 1, 2 );
   genL->addLayout( lo_centerp,      row++, 1, 1, 2 );
   genL->addLayout( lo_solution,     row++, 1, 1, 2 );
   genL->addLayout( lo_extprofs,     row,   1, 1, 2 );
   genL->addLayout( lo_connect,      row++, 3, 1, 2 );
   genL->addLayout( lo_upl_enab,     row,   1, 1, 2 );
   genL->addLayout( lo_upl_done,     row++, 3, 1, 2 );

   connect( pb_details,   SIGNAL( clicked()          ),
            this,         SLOT  ( detailExperiment() ) );
   connect( pb_connect,   SIGNAL( clicked()          ),
            this,         SLOT  ( testConnection()   ) );
   connect( pb_upload,    SIGNAL( clicked()          ),
            this,         SLOT  ( uploadExperiment() ) );

   panel->addLayout( genL );
   panel->addStretch();

   connected           = false;
   QStringList dblist  = US_Settings::defaultXpnHost();
   int ndble           = dblist.count();

   if ( ndble < 9 )
   {  // Handle a default entry without exactly 9 parts
      QStringList dblold  = dblist;
      dblist.clear();

      dblist << "test_connect";
      dblist << "bcf.uthscsa.edu";
      dblist << "5432";
      dblist << "AUC_DATA_DB";
      dblist << "auc_admin";
      dblist << "auc_admin";
      dblist << tr( "UV/visible" );
      dblist << tr( "Rayleigh Interference" );
      dblist << tr( "(not installed)" );

      for ( int ii = 0; ii < ndble; ii++ )
      {
         dblist.replace( ii, dblold[ ii ] );
      }
DbgLv(1) << "EGO:main:   opsys1-3" << dblist[6] << dblist[7] << dblist[8];

      dblist << dblist[ 4 ];
      US_Settings::set_def_xpn_host( dblist );
   }

   QString xpnhost     = dblist[ 1 ];
   int     xpnport     = dblist[ 2 ].toInt();
   QString dbname      = dblist[ 3 ];
   QString dbuser      = dblist[ 4 ];
   QString dbpasw      = dblist[ 5 ];
DbgLv(1) << "EGU: host port name user pasw" << xpnhost << xpnport
 << dbname << dbuser << dbpasw;
   US_XpnData* xpn_data = new US_XpnData();
   connected           = xpn_data->connect_data( xpnhost, xpnport, dbname,
                                                 dbuser,  dbpasw );
DbgLv(1) << "EGU:  connected" << connected;
   xpn_data->close();
   delete xpn_data;
};

// Initialize a panel, especially after clicking on its tab
void US_ExperGuiUpload::initPanel()
{
   bool chk_run      = ! sibPValue( "general",   "runID"    ).isEmpty();
   bool chk_project  = ! sibPValue( "general",   "project"  ).isEmpty();
   bool chk_rotor_ok = ( sibPValue( "rotor",     "changed"  ).toInt() > 0 );
   bool chk_speed_ok = ( sibPValue( "speeds",    "changed"  ).toInt() > 0 );
   bool chk_centerp  = ( sibPValue( "cells",     "alldone"  ).toInt() > 0 );
   bool chk_solution = ( sibPValue( "solutions", "alldone"  ).toInt() > 0 );
   bool chk_extprofs = ( sibPValue( "photomult", "alldone"  ).toInt() > 0 );
DbgLv(1) << "EGU:iP: ck: run proj cent solu epro"
 << chk_run << chk_project << chk_centerp << chk_solution << chk_extprofs;
   bool chk_upl_enab = ( chk_run       &&  chk_project   &&
                         chk_centerp   &&  chk_solution  &&
                         chk_extprofs  &&  connected   );
   bool chk_upl_done = uploaded;

   ck_run     ->setChecked( chk_run      );
   ck_project ->setChecked( chk_project  );
   ck_rotor_ok->setChecked( chk_rotor_ok );
   ck_speed_ok->setChecked( chk_speed_ok );
   ck_centerp ->setChecked( chk_centerp  );
   ck_solution->setChecked( chk_solution );
   ck_extprofs->setChecked( chk_extprofs );
   ck_connect ->setChecked( connected    );
   ck_upl_enab->setChecked( chk_upl_enab );
   ck_upl_done->setChecked( chk_upl_done );

   pb_upload  ->setEnabled( chk_upl_enab );
}

// Get a specific panel value
QString US_ExperGuiUpload::getPValue( const QString type )
{
   QString value( "" );

   if ( type == "lab" )
   {
   //   value = cb_lab->currentText();
   }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiUpload::getPList( const QString type )
{
   QStringList value( "" );

   if ( type == "uploaded" )
   {
      //value << le_runid->text();
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiUpload::sibPValue( const QString sibling, const QString type )
{
DbgLv(1) << "EGU:sibPL: sib" << sibling << "type" << type;
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPValue( sibling, type )
            : QString("") );
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiUpload::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_ExperimentMain*)mainw)->childPList( sibling, type )
            : QStringList() );
}

// Return status string for the panel
QString US_ExperGuiUpload::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
   return ( is_done ? QString( "8:X" ) : QString( "8:u" ) );
}

// Slot to show details of all experiment controls
void US_ExperGuiUpload::detailExperiment()
{
   // Create a new editor text dialog with fixed font
   US_Editor* ediag = new US_Editor( US_Editor::DEFAULT, true, "", this );
   ediag->setWindowTitle( tr( "Details on Experiment Controls" ) );
   ediag->resize( 720, 440 );
   ediag->e->setFont( QFont( US_Widgets::fixedFont().family(),
                             US_GuiSettings::fontSize() - 1,
                             QFont::Bold ) );
QFont ufont=ediag->e->font();
DbgLv(1) << "EGU:detE: ufont" << ufont.family();
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Accumulate information on controls that have been specified
   QString v_run     = sibPValue( "general",   "runID" );
   QString v_proj    = sibPValue( "general",   "project" );
   QString v_invid   = sibPValue( "general",   "investigator" );
   QString v_dbdisk  = sibPValue( "general",   "dbdisk" );
   QString v_lab     = sibPValue( "rotor",     "lab" );
   QString v_rotor   = sibPValue( "rotor",     "rotor" );
   QString v_calib   = sibPValue( "rotor",     "calib" );
   QString v_centp   = sibPValue( "cells",     "nonEmpty" );
   QString v_ccbal   = sibPValue( "cells",     "counterbalance" );
   QString v_nspeed  = sibPValue( "speeds",    "nspeeds" );
   QString v_nsolct  = sibPValue( "solutions", "nchant" );
   QString v_nsolcu  = sibPValue( "solutions", "nchanu" );
   QString v_nsolcf  = sibPValue( "solutions", "nchanf" );
   QString v_nsolun  = sibPValue( "solutions", "nusols" );
DbgLv(1) << "EGU:dE: n speed,solct,solun" << v_nspeed << v_nsolct << v_nsolun;

   QStringList sspeed   = sibPList( "speeds",    "profiles" );
DbgLv(1) << "EGU:dE: speed profiles" << sspeed;
   QStringList scentp   = sibPList( "cells",     "centerpieces" );
DbgLv(1) << "EGU:dE: cells centerpieces" << scentp;
   QStringList ssolut   = sibPList( "solutions", "solutions" );
DbgLv(1) << "EGU:dE: solus solus" << ssolut;

   bool chk_run      = ! v_run .isEmpty();
   bool chk_project  = ! v_proj.isEmpty();
   bool chk_rotor_ok = ( sibPValue( "rotor",     "changed" ).toInt() > 0 );
   bool chk_speed_ok = ( sibPValue( "speeds",    "changed" ).toInt() > 0 );
   bool chk_centerp  = ( v_centp.toInt() > 0 );
   bool chk_solution = ( sibPValue( "solutions", "alldone" ).toInt() > 0 );
   bool chk_extprofs = ( sibPValue( "photomult", "alldone" ).toInt() > 0 );
   bool chk_vars_set = ( chk_run       &&  chk_project   &&
                         chk_centerp   &&  chk_solution  &&
                         chk_extprofs );
   bool chk_upl_enab = ( chk_vars_set  &&  connected );
   bool chk_upl_done = uploaded;

   QString s_Yes     = tr( "YES" );
   QString s_no      = tr( "no" );
   QString v_genok   = ( chk_run && chk_project ) ? s_Yes : s_no; ;
   QString v_rotok   = s_Yes;
   QString v_rotuc   = chk_rotor_ok ? s_Yes : s_no;
   QString v_speok   = s_Yes;
   QString v_speuc   = chk_speed_ok ? s_Yes : s_no;
   QString v_celok   = chk_centerp  ? s_Yes : s_no;
   QString v_solok   = chk_solution ? s_Yes : s_no;
   QString v_phook   = chk_extprofs ? s_Yes : s_no;
   QString v_conok   = connected    ? s_Yes : s_no;
   QString v_uleok   = chk_upl_enab ? s_Yes : s_no;
   QString v_ulcok   = chk_upl_done ? s_Yes : s_no;

   // Compose the text to be displayed
   QString dtext  = tr( "Experiment Control Information:\n" );
   dtext += tr( "\nGeneral\n" );
   dtext += tr( "  ALL SPECIFIED:           " ) + v_genok  + "\n";
   dtext += tr( "  RunId:                   " ) + v_run    + "\n";
   dtext += tr( "  Project:                 " ) + v_proj   + "\n";
   dtext += tr( "  Investigator:            " ) + v_invid  + "\n";
   dtext += tr( "  DB / Disk:               " ) + v_dbdisk + "\n";
   dtext += tr( "\nRotor\n" );
   dtext += tr( "  ALL SPECIFIED:           " ) + v_rotok  + "\n";
   dtext += tr( "  USER CHANGES:            " ) + v_rotuc  + "\n";
   dtext += tr( "  Laboratory:              " ) + v_lab    + "\n";
   dtext += tr( "  Rotor:                   " ) + v_rotor  + "\n";
   dtext += tr( "  Calibration:             " ) + v_calib  + "\n";
   dtext += tr( "\nSpeeds\n" );
   dtext += tr( "  ALL SPECIFIED:           " ) + v_speok  + "\n";
   dtext += tr( "  USER CHANGES:            " ) + v_speuc  + "\n";
   dtext += tr( "  Number Speed Steps:      " ) + v_nspeed + "\n";

   int nspeed        = v_nspeed.toInt();
DbgLv(1) << "EGU:dE: nspeed" << nspeed;
   int jj            = 0;
   for ( int ii = 0; ii < nspeed; ii++, jj+= 6 )
   {
      dtext += tr( "    Step %1:\n" ).arg( ii + 1 );
      dtext += tr( "      Speed:         " ) + sspeed[ jj     ] + " rpm\n";
      dtext += tr( "      Acceleration:  " ) + sspeed[ jj + 1 ] + " rpm/s\n";
      dtext += tr( "      Duration:      " ) + sspeed[ jj + 2 ] + " h  "
                                             + sspeed[ jj + 3 ] + " m\n";
      dtext += tr( "      Delay:         " ) + sspeed[ jj + 4 ] + " h  "
                                             + sspeed[ jj + 5 ] + " m\n";
   }

   dtext += tr( "\nCells\n" );
   dtext += tr( "  ALL SPECIFIED:           " ) + v_celok  + "\n";
   dtext += tr( "  Non-empty Centerpieces:  " ) + v_centp  + "\n";

DbgLv(1) << "EGU:dE: ncentp" << scentp.count();
   for ( int ii = 0; ii < scentp.count(); ii++ )
   {
      dtext += "    " + scentp[ ii ] + "\n";
   }

   if ( v_ccbal.isEmpty() )
   {
      dtext += tr( "  Counterbalance:          (empty)\n" );
   }
   else
   {
      dtext += tr( "  Counterbalance:          " ) + v_ccbal  + "\n";
   }

   dtext += tr( "\nSolutions\n" );
   dtext += tr( "  ALL SPECIFIED:           " ) + v_solok  + "\n";
   dtext += tr( "  Number Channels Filled:  " ) + v_nsolcf +
            tr( " of " ) +  v_nsolct + "\n";
   dtext += tr( "  Number Unique Solutions: " ) + v_nsolun + "\n";
DbgLv(1) << "EGU:dE: " << QString(":%1:%2:%3: (:solok:nsolcf:nsolun:)")
 .arg(v_solok).arg(v_nsolcf).arg(v_nsolun);

DbgLv(1) << "EGU:dE: nsolut" << ssolut.count();
   for ( int ii = 0; ii < ssolut.count(); ii++ )
   {
      dtext += "    " + ssolut[ ii ] + "\n";
   }

   dtext += tr( "\nPhoto Multiplier Spectra\n" );
   dtext += tr( "  ALL SPECIFIED:           " ) + v_phook  + "\n";
   dtext += tr( "\nUpload\n" );
   dtext += tr( "  CONNECTED:               " ) + v_conok  + "\n";
   dtext += tr( "  UPLOAD ENABLED:          " ) + v_uleok  + "\n";
   dtext += tr( "  UPLOAD COMPLETED:        " ) + v_ulcok  + "\n";

   // Generate a JSON stream to be uploaded

   json_upl          = buildJson();

   // Report on it
   dtext += tr( "\n\nJSON stream generated and available for upload --\n\n" );
   if ( json_upl.isEmpty() )
   {
      dtext += tr( "NONE  (parameterization is incomplete).\n" );
   }
   else
   {
      dtext += json_upl + "\n";
   }
dtext+= "\n** NOT YET FULLY IMPLEMENTED **\n";

   // Load text and show the dialog
   QApplication::restoreOverrideCursor();
   qApp->processEvents();

   ediag->e->setText( dtext );
   ediag->show();
}

// Slot to test XPN connection and reset the connection flag
void US_ExperGuiUpload::testConnection()
{
   QStringList dblist  = US_Settings::defaultXpnHost();
   QString xpnhost     = dblist[ 1 ];
   int     xpnport     = dblist[ 2 ].toInt();
   QString dbname      = dblist[ 3 ];
   QString dbuser      = dblist[ 4 ];
   QString dbpasw      = dblist[ 5 ];
DbgLv(1) << "EGU: host port name user pasw" << xpnhost << xpnport
 << dbname << dbuser << dbpasw;
   US_XpnData* xpn_data = new US_XpnData();
   connected           = xpn_data->connect_data( xpnhost, xpnport, dbname,
                                                 dbuser,  dbpasw );
DbgLv(1) << "EGU:  connected" << connected;
   xpn_data->close();
   delete xpn_data;
   QString mtitle;
   QString message;

   if ( connected )
   {  // Let the user know that connection is made and set flag
      mtitle    = tr( "Successful Connection to XLA" );
      message   = tr( "The connection to the XLA has been made.\n"
                      "  Host:     %1\n"
                      "  Port:     %2\n"
                      "  DB Name:  %3\n"
                      "  DB User:  %4 .\n" )
                  .arg( xpnhost ).arg( xpnport )
                  .arg( dbname  ).arg( dbuser  );
      QMessageBox::information( this, mtitle, message );
   }

   else
   {  // Inform user of failure and give instructions
      mtitle    = tr( "Failed Connection to XLA" );
      message   = tr( "The failure to connect to the XLA most likely means\n"
                      "that host/port/name/user are misconfigured.\n"
                      "Reset them in UltraScan's 'XPN Host Preferences'\n"
                      "and return to retry connecting here." );
      QMessageBox::critical( this, mtitle, message );
   }

   initPanel();
}

// Slot to upload the experiment to the Optima DB
void US_ExperGuiUpload::uploadExperiment()
{
QString mtitle    = tr( "Not Yet Implemented" );
QString message   = tr( "The ability to upload a JSON file with the controls\n"
                        "for an experiment has not yet been implement" );
QMessageBox::information( this, mtitle, message );

   //uploaded     = true;
}

// Function to build a Json object and document holding experiment controls
QString US_ExperGuiUpload::buildJson( void )
{
   QString       js_exper("");

   // Accumulate information on controls that have been specified
   QString v_run     = sibPValue( "general",   "runID" );
   QString v_proj    = sibPValue( "general",   "project" );
   QString v_invid   = sibPValue( "general",   "investigator" );
   QString v_dbdisk  = sibPValue( "general",   "dbdisk" );
   QString v_lab     = sibPValue( "rotor",     "lab" );
   QString v_rotor   = sibPValue( "rotor",     "rotor" );
   QString v_calib   = sibPValue( "rotor",     "calib" );
   QString v_centp   = sibPValue( "cells",     "nonEmpty" );
   QString v_nspeed  = sibPValue( "speeds",    "nspeeds" );
   QString v_nsolct  = sibPValue( "solutions", "nchant" );
   QString v_nsolcu  = sibPValue( "solutions", "nchanu" );
   QString v_nsolcf  = sibPValue( "solutions", "nchanf" );
   QString v_nsolun  = sibPValue( "solutions", "nusols" );
DbgLv(1) << "EGU:bj: n speed,solct,solun" << v_nspeed << v_nsolct << v_nsolun;

   QStringList sspeed   = sibPList( "speeds",    "profiles" );
DbgLv(1) << "EGU:bj: speed profiles" << sspeed;
   QStringList scentp   = sibPList( "cells",     "centerpieces" );
DbgLv(1) << "EGU:bj: cells centerpieces" << scentp;
   QStringList ssolut   = sibPList( "solutions", "channel_solutions" );
DbgLv(1) << "EGU:bj: solus solus" << ssolut;

   bool chk_run      = ! v_run .isEmpty();
   bool chk_project  = ! v_proj.isEmpty();
   bool chk_centerp  = ( v_centp.toInt() > 0 );
   bool chk_solution = ( sibPValue( "solutions", "alldone" ).toInt() > 0 );
   bool chk_extprofs = ( sibPValue( "photomult", "alldone" ).toInt() > 0 );
DbgLv(1) << "EGU:bj: ck: run proj cent solu epro"
 << chk_run << chk_project << chk_centerp << chk_solution << chk_extprofs;
   bool chk_vars_set = ( chk_run       &&  chk_project   &&
                         chk_centerp   &&  chk_solution  &&
                         chk_extprofs );
   bool chk_upl_enab = ( chk_vars_set  &&  connected );

   if ( ! chk_upl_enab )
      return js_exper; 		// Parameterization incomplete: empty return

   // Start building Json of experiment controls
   QJsonObject   jo_exper;
   QJsonObject   jo_rotor;
   QJsonObject   jo_speed;
   QJsonObject   jo_cells;
   QJsonObject   jo_solut;
   QJsonObject   jo_photo;
   QJsonArray    ja_speed;
   QJsonArray    ja_centp;
   QJsonArray    ja_solut;
   jo_exper.insert( "run", QJsonValue( v_run ) );
   jo_exper.insert( "project", QJsonValue( v_proj ) );
   jo_exper.insert( "dbdisk", QJsonValue( v_dbdisk ) );
   jo_exper.insert( "investigator", QJsonValue( v_invid ) );
   jo_rotor.insert( "laboratory", v_lab );
   jo_rotor.insert( "rotor", v_rotor );
   jo_rotor.insert( "calibration", v_calib );
   jo_exper.insert( "rotor", QJsonValue( jo_rotor ) );

   int nspeed        = v_nspeed.toInt();
   int jj            = 0;
   for ( int ii = 0; ii < nspeed; ii++, jj+= 6 )
   {
      QJsonObject   jo_sstep;
      jo_sstep.insert( "speed", QJsonValue( sspeed[ jj ] ) );
      jo_sstep.insert( "acceleration", QJsonValue( sspeed[ jj + 1 ] ) );
      QString durat     = QString( "%1 h %2 m" )
                          .arg( sspeed[ jj + 2 ] ).arg( sspeed[ jj + 3 ] );
      QString delay     = QString( "%1 h %2 m" )
                          .arg( sspeed[ jj + 4 ] ).arg( sspeed[ jj + 5 ] );
      jo_sstep.insert( "duration", QJsonValue( durat ) );
      jo_sstep.insert( "delay", QJsonValue( delay ) );
      ja_speed.append( jo_sstep );
   }
   jo_exper.insert( "speeds", ja_speed );

   for ( int ii = 0; ii < scentp.count(); ii++ )
   {
      QJsonObject   jo_centp;
      QString centp     = scentp[ ii ];
      QString scell     = QString( centp ).section( ":", 0, 0 )
                                          .section( " ", 1, 1 ).simplified();
      QString scent     = QString( centp ).section( ":", 1, 1 ).simplified();
      jo_centp.insert( "cell", QJsonValue( scell ) );
      if ( ! scent.contains( "counterbalance" ) )
      {
         int iopp          = scent.lastIndexOf( "(" );
         QString swind     = QString( scent ).mid( iopp + 2 ).section( " ", 0, 0 );
         scent             = QString( scent ).left( iopp ).simplified();
         jo_centp.insert( "centerpiece", QJsonValue( scent ) );
         jo_centp.insert( "window_type", QJsonValue( swind ) );
      }
      else
      {
         jo_centp.insert( "counterbalance", QJsonValue( scent ) );
      }

      ja_centp.append( jo_centp );
   }
   jo_exper.insert( "centerpieces", ja_centp );

   for ( int ii = 0; ii < ssolut.count(); ii++ )
   {
      QJsonObject   jo_solut;
      QString solut     = ssolut[ ii ];
      QString scech     = QString( solut ).section( ":", 0, 0 ).simplified();
      QString ssolu     = QString( solut ).section( ":", 1, 1 ).simplified();
      QString scell     = QString( scech ).section( "/", 0, 0 ).simplified();
      QString schan     = QString( scech ).section( "/", 1, 1 ).simplified();
      jo_solut.insert( "cell", QJsonValue( scell ) );
      jo_solut.insert( "channel", QJsonValue( schan ) );
      jo_solut.insert( "solution", QJsonValue( ssolu ) );

      ja_solut.append( jo_solut );
   }
   jo_exper.insert( "solutions", ja_solut );


   // Format the byte array and string form of Json
   QJsonDocument jd_exper( jo_exper );
   QByteArray    jb_exper;
   jb_exper          = jd_exper.toJson();
   js_exper          = QString( jb_exper );
/*DEBUG*/
DbgLv(1) << "EGU:bj: js_exper" << js_exper;
QString urunid=QString( v_run ).replace( " ", "_" );
QString rpath=US_Settings::resultDir() + "/" + urunid + "/";
QString fname=urunid + ".experiment-json.dat";
QString fpath=rpath+fname;
QDir rdir;
if(! rdir.exists(rpath) ) rdir.mkpath(rpath);
QFile jfile(fpath);
if(jfile.open(QIODevice::WriteOnly|QIODevice::Text))
{
 int flen=jb_exper.count();
 QDataStream* fso = new QDataStream( &jfile );
 fso->writeRawData( jb_exper.constData(), flen );
 jfile.close();
}
else
 qDebug() << "*ERROR* Unable to open file" << fpath;
/*DEBUG*/

   return js_exper;
}

