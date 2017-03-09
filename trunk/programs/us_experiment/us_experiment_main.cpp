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
#include "us_util.h"
#include "us_select_item.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
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

// Constructor:  build the main layout with tab widget panels
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

   // Create tab and panel widgets
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
   statflag            = 0;

   // Add panels to the tab widget
   tabWidget->addTab( epanGeneral,   tr( "1: General"          ) );
   tabWidget->addTab( epanRotor,     tr( "2: Lab/Rotor"        ) );
   tabWidget->addTab( epanSpeeds,    tr( "3: Speeds"           ) );
   tabWidget->addTab( epanCells,     tr( "4: Cells"            ) );
   tabWidget->addTab( epanSolutions, tr( "5: Solutions"        ) );
   tabWidget->addTab( epanOptical,   tr( "6: Optical Systems"  ) );
   tabWidget->addTab( epanSpectra,   tr( "7: Spectra"          ) );
   tabWidget->addTab( epanUpload,    tr( "8: Upload"           ) );
   tabWidget->setCurrentIndex( curr_panx );

   // Add bottom buttons
   QPushButton* pb_close  = us_pushbutton( tr( "Close" ) );
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_next   = us_pushbutton( tr( "Next Panel" ) );
   QPushButton* pb_prev   = us_pushbutton( tr( "Previous Panel" ) );
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

// Reset parameters to their defaults
void US_ExperimentMain::reset( void )
{
}

// Panel for run and other general parameters
US_ExperGuiGeneral::US_ExperGuiGeneral( QWidget* topw )
{
   mainw               = (US_ExperimentMain*)topw;
   dbg_level           = US_Settings::us_debug();
   use_db              = ( US_Settings::default_data_location() < 2 );
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "1: Specify run and other general parameters" ) );
   panel->addWidget( lb_panel );

   // Create layout and GUI components
   QGridLayout* genL   = new QGridLayout();

   QLabel*      lb_runid        = us_label( tr( "Run Name:" ) );
   QLabel*      lb_tempera      = us_label( tr( "Run Temperature " ) + DEGC + ":" );
   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   QPushButton* pb_project      = us_pushbutton( tr( "Select Project" ) );
   QPushButton* pb_protocol     = us_pushbutton( tr( "Load Protocol" ) );
                le_runid        = us_lineedit( "", 0, false );
                le_protocol     = us_lineedit( "", 0, false );
                le_project      = us_lineedit( "", 0, true  );
                ct_tempera      = us_counter( 2, 0, 40, 1 );

   // Insure reasonable size for temperature counter
   lb_runid->adjustSize();
   QFont font( US_GuiSettings::fontFamily(),
               US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( font );
   int fwid    = fmet.maxWidth();
   int chgt    = lb_runid->height();
   int cwid    = fwid * 3;
   int mwid    = fwid * 4;
   ct_tempera->setSingleStep( 1 );
   ct_tempera->setValue     ( 20 );
   ct_tempera->setMaximumWidth ( mwid );
   ct_tempera->resize       ( cwid, chgt );
   ct_tempera->adjustSize   ();

   // Set up an approprate investigator text
   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   int id          = US_Settings::us_inv_ID();
   QString invnbr  = ( id > 0 ) ?
      QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";
   QString invtxt  = invnbr + US_Settings::us_inv_name();
   le_investigator = us_lineedit( invtxt, 0, true );

   // Set defaults
   currProto       = &mainw->currProto;
   currProto->investigator = invtxt;
   currProto->runname      = "";
   currProto->protname     = "";
   currProto->project      = "";
   currProto->temperature  = 20.0;

   // Build main layout
   int row         = 0;
   genL->addWidget( pb_investigator, row,   0, 1, 1 );
   genL->addWidget( le_investigator, row++, 1, 1, 5 );
   genL->addWidget( lb_runid,        row,   0, 1, 1 );
   genL->addWidget( le_runid,        row++, 1, 1, 5 );
   genL->addWidget( pb_protocol,     row,   0, 1, 1 );
   genL->addWidget( le_protocol,     row++, 1, 1, 5 );
   genL->addWidget( pb_project,      row,   0, 1, 1 );
   genL->addWidget( le_project,      row++, 1, 1, 5 );
   genL->addWidget( lb_tempera,      row,   0, 1, 1 );
   genL->addWidget( ct_tempera,      row,   1, 1, 1 );

   panel->addLayout( genL );
   panel->addStretch();

   // Set up signal and slot connections
   connect( le_runid,        SIGNAL( editingFinished()  ),
            this,            SLOT(   run_name_entered() ) );
   connect( pb_project,      SIGNAL( clicked()          ), 
            this,            SLOT(   sel_project()      ) );
   connect( pb_investigator, SIGNAL( clicked()          ), 
            this,            SLOT(   sel_investigator() ) );
   connect( pb_protocol,     SIGNAL( clicked()          ), 
            this,            SLOT(   load_protocol()    ) );
   connect( le_protocol,     SIGNAL( editingFinished()  ),
            this,            SLOT(   changed_protocol() ) );

   // Read in centerpiece information and populate names list
   centerpieceInfo();

   // Read in summary information on all existing run protocols
   bool fromdisk         = US_Settings::debug_match( "protocolFromDisk" );
   bool load_db          = fromdisk ? false : use_db;
   US_Passwd  pw;
   US_DB2* dbP           = load_db ? new US_DB2( pw.getPasswd() ) : NULL;

   US_ProtocolUtil::list_all( protdata, dbP );

   for ( int ii = 0; ii < protdata.count(); ii++ )
      pr_names << protdata[ ii ][ 0 ];
DbgLv(1) << "EGGe: main : prnames,prdata counts" << pr_names.count() << protdata.count();

   // Do the initialization we do at panel entry
   initPanel();
}

// Return detail information for a specific centerpiece as named
bool US_ExperGuiGeneral::centpInfo( const QString cpname,
      US_AbstractCenterpiece& cpEntry )
{
   bool is_found   = false;

   for ( int ii = 0; ii < acp_list.count(); ii++ )
   {  // Search abstract centerpieces for a name match
      if ( acp_list[ ii ].name == cpname )
      {  // Match found:  flag found and return entry
         is_found        = true;
         cpEntry         = acp_list[ ii ];
         break;
      }
   }

   return is_found;
}

// Return a protocol names list and data entries list
int US_ExperGuiGeneral::getProtos( QStringList& prnames,
      QList< QStringList >& prentries )
{
DbgLv(1) << "EGGe: getProtos IN";
   prnames         = pr_names;   // Return a names list copy
   prentries       = protdata;   // Return an entries list copy
DbgLv(1) << "EGGe:  gP: prnames count" << prnames.count()
 << "prdat count" << protdata.count();
   return prnames.count();       // Return the current list count
}

// Update protocol name list and data list from an entry
bool US_ExperGuiGeneral::updateProtos( const QStringList prentry )
{
   if ( pr_names.contains( prentry[ 0 ] ) )
      return false;              // Not ok if the name is already in the list

   pr_names << prentry[ 0 ];     // Append to the names list
   protdata << prentry;          // Append to the data entries list

   return true;
}

// Verify valid run name (possible modify for valid-only characters)
void US_ExperGuiGeneral::run_name_entered( void )
{
DbgLv(1) << "EGGe: rchg: IN";
   // Modify run name to have only valid characters
   QRegExp rx( "[^A-Za-z0-9_-]" );
   QString rname     = le_runid->text();
   QString old_rname = rname;
DbgLv(1) << "EGGe: rchg: old_rname" << old_rname;
   rname.replace( rx,  "_" );
DbgLv(1) << "EGGe: rchg:     rname" << rname;
   bool changed      = false;
   
   if ( rname != old_rname )
   {  // Report on invalid characters replaced
      QMessageBox::warning( this,
         tr( "RunID Name Changed" ),
         tr( "The runID name has been changed. It may consist only\n"
             "of alphanumeric characters or underscore or hyphen.\n"
             "New runID:\n  " )
            + rname );
      changed           = true;
   }

DbgLv(1) << "EGGe: rchg: len(runname)" << rname.length();
   // Limit run ID length to 50 characters
   if ( rname.length() > 50 )
   {
      QMessageBox::warning( this,
         tr( "RunID Name Too Long" ),
         tr( "The runID name may be at most\n"
             "50 characters in length." ) );
      rname             = rname.left( 50 );
      changed           = true;
   }
DbgLv(1) << "EGGe: rchg: changed" << changed;

   if ( changed )
   {  // Replace runID in line edit box
      le_runid->setText( rname );
   }
}

// Select DB investigator
void US_ExperGuiGeneral::sel_project( void )
{
   int dbdisk            = use_db ? US_Disk_DB_Controls::DB
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
   int investID     = US_Settings::us_inv_ID();

   US_Investigator* dialog = new US_Investigator( true, investID );
   dialog->exec();

   investID         = US_Settings::us_inv_ID();

   QString inv_text = QString::number( investID ) + ": "
                      +  US_Settings::us_inv_name();

   currProto->investigator  = inv_text;
   le_investigator->setText( inv_text );
}

// Load Protocol
void US_ExperGuiGeneral::load_protocol( void )
{
   bool fromdisk         = US_Settings::debug_match( "protocolFromDisk" );
   bool load_db          = fromdisk ? false : use_db;
   US_Passwd            pw;
   QString xmlstr( "" );
   QStringList          hdrs;
   int                  prx;
   QString pdtitle( tr( "Select a Protocol to Load" ) );
   pdtitle             += ( ( load_db ) ?
                            tr( " from Database" ) :
                            tr( " from Disk" ) );
//*DEBUG: forced input from disk
if(fromdisk) {
QString mtitle  = tr( "Debug Forced Protocol from Disk" );
QString message = tr( "The ability to load a protocol from the database\n" )
 + tr( "has been overridden by a debug setting.\n" )
 + tr( "Protocols from local disk will be listed." );
QMessageBox::information( this, mtitle, message );
}
//*DEBUG: forced input from disk
DbgLv(1) << "EGGe:ldPro: Disk-B: load_db" << load_db;
   // Get database connection pointer (or NULL as disk flag)
   US_DB2* dbP           = load_db ? new US_DB2( pw.getPasswd() ) : NULL;

   // Build dialog table widget headers
   hdrs << "Protocol Name"
        << "Date"
        << ( load_db ? "DbID" : "File Name" );

   // Select a protocol
   US_SelectItem pdiag( protdata, hdrs, pdtitle, &prx, -2 );

   if ( pdiag.exec() == QDialog::Accepted )
   {  // Accept in dialog:  get selected protocol name and its XML
DbgLv(1) << "EGGe:ldPro:  ACCEPT  prx" << prx << "sel proto" << protdata[prx][0];
      QString pname         = protdata[ prx ][ 0 ];

      // Get the protocol XML that matches the selected protocol name
      US_ProtocolUtil::read_record( pname, &xmlstr, NULL, dbP );
DbgLv(1) << "EGGe:ldPro:  ACCEPT   read_record return len(xml)" << xmlstr.length();

      le_protocol->setText( pname );
   }

   else
   {  // Reject in dialog
DbgLv(1) << "EGGe:ldPro:  REJECT";
      return;
   }

   // Now that we have a protocol XML, convert it to internal controls
   QXmlStreamReader xmli( xmlstr );
   mainw->loadProto.fromXml( xmli );

   // Initialize the current protocol from the loaded one; set temperature
   mainw->currProto      = mainw->loadProto;
   ct_tempera->setValue( mainw->currProto.temperature );
DbgLv(1) << "EGGe:ldPro:    dur0" << mainw->currProto.rpSpeed.ssteps[0].duration;
DbgLv(1) << "EGGe:ldPro:    cPname" << mainw->currProto.protname
 << "lPname" << mainw->loadProto.protname;
DbgLv(1) << "EGGe:ldPro:    cOhost" << mainw->currProto.optimahost
 << "lOhost" << mainw->loadProto.optimahost;
DbgLv(1) << "EGGe:ldPro:    cTempe" << mainw->currProto.temperature
 << "lTempe" << mainw->loadProto.temperature;

   // Initialize all other panels using the new protocol
   mainw->initPanels();
}

// Verify entered protocol name
void US_ExperGuiGeneral::changed_protocol( void )
{
   QString protname     = le_protocol->text();

   if ( pr_names.contains( protname ) )
   {
      QString msg          =
         tr( "The protocol name given<br/>" )
         +  "  ( <b>" + protname + "</b> )<br/>"
         + tr( "is already being used.  It will need to be<br/>"
               "changed if/when this protocol is saved.<br/><br/>"
               "If you plan to make changes to this protocol,<br/>"
               "it is suggested that you change it's name<br/>"
               "(description text) at this time." );
      QMessageBox::information( this,
                                tr( "Duplicate Run Protocol Name" ),
                                msg );
   }
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
   US_DB2* dbP    = use_db ? new US_DB2( pw.getPasswd() ) : NULL;

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
   mainw               = (US_ExperimentMain*)topw;
   rpRotor             = &(mainw->currProto.rpRotor);
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

   int row     = 0;
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
   US_DB2* dbP     = ( sibSValue( "general", "dbdisk" ) == "DB" )
                      ? new US_DB2( pw.getPasswd() ) : NULL;
   if ( dbP != NULL )
      US_Rotor::readLabsDB( labs, dbP );
   else
      US_Rotor::readLabsDisk( labs );

   for ( int ii = 0; ii < labs.count(); ii++ )
      sl_labs << QString::number( labs[ ii ].ID )
                 + ": " + labs[ ii ].name;

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

   initPanel();
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
   US_DB2* dbP              = ( sibSValue( "general", "dbdisk" ) == "DB" )
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
   US_DB2* dbP         = ( sibSValue( "general", "dbdisk" ) == "DB" )
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
   int dbdisk       = ( sibSValue( "general", "dbdisk" ) == "DB" )
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
   mainw               = (US_ExperimentMain*)topw;
   rpSpeed             = &(mainw->currProto.rpSpeed);
   changed             = false;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "3: Specify speed steps" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   // Labels
   QLabel* lb_count    = us_label( tr( "Number of Speed Profiles:" ) );
   QLabel* lb_speed    = us_label( tr( "Rotor Speed (rpm):" ) );
   QLabel* lb_accel    = us_label( tr( "Acceleration Profile (rpm/sec):" ) );
   QLabel* lb_durhr    = us_label( tr( "Length of Experiment (hours):" ) );
   QLabel* lb_durmin   = us_label( tr( "Length of Experiment (minutes):" ) );
   QLabel* lb_dlyhr    = us_label( tr( "Time Delay for Scans (hours):" ) );
   QLabel* lb_dlymin   = us_label( tr( "Time Delay for Scans (minutes):" ) );
   QLabel* lb_dlysec   = us_label( tr( "Time Delay for Scans (seconds):" ) );
   QCheckBox* ck_endoff;
   QCheckBox* ck_radcal;
   QLayout* lo_endoff  = us_checkbox( tr( "Spin down centrifuge at job end" ),
                                      ck_endoff, true );
   QLayout* lo_radcal  = us_checkbox( tr( "Perform radial calibration" ),
                                      ck_radcal, false );

   // ComboBox and counters
   cb_prof             = new QComboBox( this );
   ct_speed            = us_counter( 2, 1000, 100000, 100 );
   ct_accel            = us_counter( 2,   50,   1000,  50 );
   ct_count            = us_counter( 2,    1,    100,   1 );
   ct_durhr            = us_counter( 2,    0,    100,   1 );
   ct_durmin           = us_counter( 2,    0,     59,   1 );
   ct_dlyhr            = us_counter( 2,    0,     50,   1 );
   ct_dlymin           = us_counter( 2,    0,     59,   1 );
   ct_dlysec           = us_counter( 2,    0,     59,   1 );

   // Default values
   nspeed              = 1;
   curssx              = 0;
   double defspeed     = rpSpeed->ssteps[ 0 ].speed;
   double defaccel     = rpSpeed->ssteps[ 0 ].accel;
   double defdurtim    = rpSpeed->ssteps[ 0 ].duration;
   double defdlytim    = rpSpeed->ssteps[ 0 ].delay;
   double defdurhr     = qFloor( defdurtim / 60.0 );
   double defdurmin    = defdurtim - ( defdurhr * 60.0 );
   double defdlymin    = qFloor( defdlytim / 60.0 );
   double defdlyhr     = qFloor( defdlymin / 60.0 );
   defdlymin          -= ( defdlyhr * 60.0 );
   double defdlysec    = defdlytim - ( defdlyhr * 3600.0 )
                                   - ( defdlymin * 60.0 );
DbgLv(1) << "EGSp: defdurtim" << defdurtim;
DbgLv(1) << "EGSp:   defdurhr" << defdurhr << "defdurmin" << defdurmin;
DbgLv(1) << "EGSp: defdlytim" << defdlytim;
DbgLv(1) << "EGSp:   defdlyhr" << defdlyhr << "defdlymin" << defdlymin << "defdlysec" << defdlysec;
   profdesc.resize( nspeed );  // Speed profile descriptions
   ssvals  .resize( nspeed );  // Speed step values
   ssvals[ 0 ][ "speed" ]    = defspeed;  // Speed default
   ssvals[ 0 ][ "accel" ]    = defaccel;  // Acceleration default
   ssvals[ 0 ][ "duration" ] = defdurtim; // Duration in minutes default (5h 30m)
   ssvals[ 0 ][ "delay" ]    = defdlytim; // Delay in seconds default (2m 30s)

   // Set up counters and profile description
   ct_count ->setSingleStep(   1 );
   ct_speed ->setSingleStep( 100 );
   ct_accel ->setSingleStep(  50 );
   ct_durhr ->setSingleStep(   1 );
   ct_durmin->setSingleStep(   1 );
   ct_dlyhr ->setSingleStep(   1 );
   ct_dlymin->setSingleStep(   1 );
   ct_dlysec->setSingleStep(   1 );

   ct_count ->setValue( nspeed    );
   ct_speed ->setValue( defspeed  );
   ct_accel ->setValue( defaccel  );
   ct_durhr ->setValue( defdurhr  );
   ct_durmin->setValue( defdurmin );
   ct_dlyhr ->setValue( defdlyhr  );
   ct_dlymin->setValue( defdlymin );
   ct_dlysec->setValue( defdlysec );

   // Speed profile 1 description;
   //   e.g., "Speed Profile 1: 30000 rpm for 5 hr 30 min"
   cb_prof->addItem( speedp_description( 0 ) );

   // Adjust counter sizes
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
   ct_durhr ->resize( csizw, rheight );
   ct_durmin->resize( csizw, rheight );
   ct_dlyhr ->resize( csizw, rheight );
   ct_dlymin->resize( csizw, rheight );
   ct_dlysec->resize( csizw, rheight );

   // Create main layout rows
   int row = 0;
   genL->addWidget( lb_count,   row,   0, 1, 3 );
   genL->addWidget( ct_count,   row++, 3, 1, 1 );
   genL->addWidget( cb_prof,    row++, 0, 1, 4 );
   genL->addWidget( lb_speed,   row,   0, 1, 3 );
   genL->addWidget( ct_speed,   row++, 3, 1, 1 );
   genL->addWidget( lb_accel,   row,   0, 1, 3 );
   genL->addWidget( ct_accel,   row++, 3, 1, 1 );
   genL->addWidget( lb_durhr,   row,   0, 1, 3 );
   genL->addWidget( ct_durhr,   row++, 3, 1, 1 );
   genL->addWidget( lb_durmin,  row,   0, 1, 3 );
   genL->addWidget( ct_durmin,  row++, 3, 1, 1 );
   genL->addWidget( lb_dlyhr,   row,   0, 1, 3 );
   genL->addWidget( ct_dlyhr,   row++, 3, 1, 1 );
   genL->addWidget( lb_dlymin,  row,   0, 1, 3 );
   genL->addWidget( ct_dlymin,  row++, 3, 1, 1 );
   genL->addWidget( lb_dlysec,  row,   0, 1, 3 );
   genL->addWidget( ct_dlysec,  row++, 3, 1, 1 );
   genL->addLayout( lo_endoff,  row,   0, 1, 2 );
   genL->addLayout( lo_radcal,  row++, 2, 1, 2 );
   genL->setColumnStretch( 0, 4 );
   genL->setColumnStretch( 3, 0 );

   // Connect signals and slots
   connect( ct_count,  SIGNAL( valueChanged  ( double ) ),
            this,      SLOT  ( ssChangeCount ( double ) ) );
   connect( cb_prof,   SIGNAL( activated     ( int    ) ),
            this,      SLOT  ( ssChangeProfx ( int    ) ) );
   connect( ct_speed,  SIGNAL( valueChanged  ( double ) ),
            this,      SLOT  ( ssChangeSpeed ( double ) ) );
   connect( ct_accel,  SIGNAL( valueChanged  ( double ) ),
            this,      SLOT  ( ssChangeAccel ( double ) ) );
   connect( ct_durhr,  SIGNAL( valueChanged  ( double ) ),
            this,      SLOT  ( ssChangeDurhr ( double ) ) );
   connect( ct_durmin, SIGNAL( valueChanged  ( double ) ),
            this,      SLOT  ( ssChangeDurmin( double ) ) );
   connect( ct_dlyhr,  SIGNAL( valueChanged  ( double ) ),
            this,      SLOT  ( ssChangeDlyhr ( double ) ) );
   connect( ct_dlymin, SIGNAL( valueChanged  ( double ) ),
            this,      SLOT  ( ssChangeDlymin( double ) ) );
   connect( ct_dlysec, SIGNAL( valueChanged  ( double ) ),
            this,      SLOT  ( ssChangeDlysec( double ) ) );

   // Complete overall layout
   panel->addLayout( genL );
   panel->addStretch();
   adjustSize();

   // Set low delay-minutes based on speed,acceleration,delay-hours
   adjustDelay();

   initPanel();
}

// Return speed profile description string for an indicated step
QString US_ExperGuiSpeeds::speedp_description( int ssx )
{
   // For example: "Speed Profile 1: 30000 rpm for 5 hr 30 min"
   double durtim  = ssvals[ ssx ][ "duration" ]; // Duration total minutes
   double durhr   = qFloor( durtim / 60.0 );     // Duration hours
   double durmin  = durtim - ( durhr * 60.0 );   // Duration residual minutes
   int    escans  = qRound( durtim * 6.0 );      // Estimated step scans
   double durtot  = 0.0;
   for ( int ii = 0; ii < ssvals.size(); ii++ )
      durtot        += ssvals[ ii ][ "duration" ];
   int    tscans  = qRound( durtot * 6.0 );      // Estimated total scans
  
DbgLv(1) << "EGSp: spDesc: ssx" << ssx
 << "dur tim,hr,min" << durtim << durhr << durmin;

   return tr( "Speed Profile %1 :    %2 rpm for %3 hr %4 min"
              "  (%5 estimated scans, %6 run total)" )
          .arg( ssx + 1 ).arg( ssvals[ ssx ][ "speed" ] )
          .arg( durhr ).arg( durmin ).arg( escans ).arg( tscans );
}

// Slot for change in speed-step count
void US_ExperGuiSpeeds::ssChangeCount( double val )
{
   changed          = true;
   int new_nsp      = (int)val;
DbgLv(1) << "EGSp: chgKnt: nsp nnsp" << nspeed << new_nsp;
   if ( new_nsp > nspeed )
   {  // Number of speed steps increases
      profdesc.resize( new_nsp );
      ssvals  .resize( new_nsp );
      int kk           = nspeed - 1;
      double ssspeed   = ssvals[ kk ][ "speed" ];
      double ssaccel   = ssvals[ kk ][ "accel" ];
      double ssdurtim  = ssvals[ kk ][ "duration" ];
      double ssdurhr   = qFloor( ssdurtim / 60.0 );
      double ssdurmin  = ssdurtim - ( ssdurhr * 60.0 );
      double ssdlytim  = ssvals[ kk ][ "delay" ];
      double ssdlymin  = qFloor( ssdlytim / 60.0 );
      double ssdlyhr   = qFloor( ssdlymin / 60.0 );
      ssdlymin        -= ( ssdlyhr * 60.0 );
      double ssdlysec  = ssdlytim - ( ssdlyhr * 3600.0 )
                                  - ( ssdlymin * 60.0 );
      ssdurtim         = ( ssdurhr * 60.0 ) + ssdurmin;
      ssdlytim         = ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
DbgLv(1) << "EGSp: chgKnt:  kk" << kk << "spd acc dur dly"
 << ssspeed << ssaccel << ssdurtim << ssdlytim;

      for ( kk = nspeed; kk < new_nsp; kk++ )
      {  // Fill in new speed step description and values
         ssspeed         += 5000.0;
         ssvals[ kk ][ "speed"    ] = ssspeed;   // Speed
         ssvals[ kk ][ "accel"    ] = ssaccel;   // Acceleration
         ssvals[ kk ][ "duration" ] = ssdurtim;  // Duration in minutes
         ssvals[ kk ][ "delay"    ] = ssdlytim;  // Delay in seconds
         profdesc[ kk ]             = speedp_description( kk );
DbgLv(1) << "EGSp: chgKnt:    kk" << kk << "pdesc" << profdesc[kk];

         // Add to the profile description list comboBox
         cb_prof->addItem( profdesc[ kk ] );
      }

      // Point to the first new speed step
      cb_prof->setCurrentIndex( nspeed );
      ssChangeProfx( nspeed );
   }

   else
   {  // Number of speed steps decreases or remains the same
      // Point to the last speed step
      profdesc.resize( new_nsp );
      ssvals  .resize( new_nsp );
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
DbgLv(1) << "EGSp: chgPfx: ssp" << ssp << "prev-ssx" << curssx;
   curssx           = qMin( curssx, ( ssvals.size() - 1 ) );
DbgLv(1) << "EGSp: chgPfx:  speed-c speed-p"
 << ssvals[ssp]["speed"] << ssvals[curssx]["speed"];
   curssx           = ssp;
   // Set all counters for newly selected step
   double ssspeed   = ssvals[ curssx ][ "speed" ];
   double ssaccel   = ssvals[ curssx ][ "accel" ];
   double ssdurtim  = ssvals[ curssx ][ "duration" ];
   double ssdurhr   = qFloor( ssdurtim / 60.0 );
   double ssdurmin  = ssdurtim - ( ssdurhr * 60.0 );
DbgLv(1) << "EGSp: chgPfx:   durtim durhr durmin" << ssdurtim << ssdurhr << ssdurmin;
   double ssdlytim  = ssvals[ curssx ][ "delay" ];
   double ssdlymin  = qFloor( ssdlytim / 60.0 );
   double ssdlyhr   = qFloor( ssdlymin / 60.0 );
   ssdlymin        -= ( ssdlyhr * 60.0 );
   double ssdlysec  = ssdlytim - ( ssdlyhr * 3600.0 )
                               - ( ssdlymin * 60.0 );
   ct_speed ->setValue( ssspeed  );
   ct_accel ->setValue( ssaccel  );
   ct_durhr ->setValue( ssdurhr  );
   ct_durmin->setValue( ssdurmin );
   ct_dlyhr ->setValue( ssdlyhr  );
   ct_dlymin->setValue( ssdlymin );
   ct_dlysec->setValue( ssdlysec );
}

// Slot for change in speed value
void US_ExperGuiSpeeds::ssChangeSpeed( double val )
{
   changed          = true;
DbgLv(1) << "EGSp: chgSpe: val" << val << "ssx" << curssx;
   ssvals  [ curssx ][ "speed" ] = val;  // Set Speed in step vals vector
   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );

   // Set minimum delay time based on speed change and acceleration
   adjustDelay();
}

// Slot for change in acceleration value
void US_ExperGuiSpeeds::ssChangeAccel( double val )
{
DbgLv(1) << "EGSp: chgAcc: val" << val << "ssx" << curssx;
   ssvals[ curssx ][ "accel" ] = val;  // Set Acceleration in step vals vector

   // Set minimum delay time based on speed and new acceleration
   adjustDelay();
}

// Slot for change in duration-hour value
void US_ExperGuiSpeeds::ssChangeDurhr( double val )
{
   changed          = true;
   double ssdurtim  = ( val * 60.0 ) + ct_durmin->value();
   ssvals[ curssx ][ "duration" ] = ssdurtim;  // Set Duration in step vals vector
DbgLv(1) << "EGSp: chgDuh: val" << val << "ssx" << curssx << "ssdurtim" << ssdurtim;
   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );
}

// Slot for change in duration-minute value
void US_ExperGuiSpeeds::ssChangeDurmin( double val )
{
   changed          = true;
   double ssdurtim  = ( ct_durhr->value() * 60.0 ) + val;
DbgLv(1) << "EGSp: chgDum: val" << val << "ssx" << curssx << "ssdurtim" << ssdurtim;
   ssvals[ curssx ][ "duration" ] = ssdurtim;  // Set Duration in step vals vector
   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );
}

// Slot for change in delay-hour value
void US_ExperGuiSpeeds::ssChangeDlyhr( double val )
{
   changed          = true;
DbgLv(1) << "EGSp: chgDlh: val" << val << "ssx" << curssx;
   double ssdlyhr   = val;
   double ssdlymin  = ct_dlymin->value();
   double ssdlysec  = ct_dlysec->value();
   double ssdlytim  = ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
   ssvals[ curssx ][ "delay" ] = ssdlytim;  // Set Delay in step vals vector

   // Set minimum delay time based on speed, acceleration, and delay-hours
   adjustDelay();
}

// Slot for change in delay-minute value
void US_ExperGuiSpeeds::ssChangeDlymin( double val )
{
   changed          = true;
DbgLv(1) << "EGSp: chgDlm: val" << val << "ssx" << curssx;
   double ssdlyhr   = ct_dlyhr ->value();
   double ssdlymin  = val;
   double ssdlysec  = ct_dlysec->value();
   double ssdlytim  = ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
   ssvals[ curssx ][ "delay" ] = ssdlytim;  // Set Delay in step vals vector

   // Set minimum delay time based on speed, acceleration, and delay-hours
   adjustDelay();
}

// Slot for change in delay-second value
void US_ExperGuiSpeeds::ssChangeDlysec( double val )
{
   changed          = true;
DbgLv(1) << "EGSp: chgDls: val" << val << "ssx" << curssx;
   double ssdlyhr   = ct_dlyhr ->value();
   double ssdlymin  = ct_dlymin->value();
   double ssdlysec  = val;
   double ssdlytim  = ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
   ssvals[ curssx ][ "delay" ] = ssdlytim;  // Set Delay in step vals vector
}

// Function to adjust delay minimum when related values are changed
void US_ExperGuiSpeeds::adjustDelay( void )
{
   // Set minimum delay time based on speed change and acceleration
   double cspeed    = ssvals[ curssx ][ "speed" ];  // Current step's speed
   double pspeed    = ( curssx > 0 ) ? ssvals[ curssx - 1 ][ "speed" ] : 0.0;
   double spdelta   = cspeed - pspeed;         // Speed delta
   double accel     = ssvals[ curssx ][ "accel" ];   // Acceleration
   double delaytim  = ssvals[ curssx ][ "delay" ];   // Delay in total seconds
   double delayhr   = qFloor( delaytim / 3600.0 );   // Delay in hours
   double delaymin  = qFloor( delayhr / 60.0 );      // Delay in minutes
   delaymin        -= ( delayhr * 60.0 );            // Residual minutes
   double delaysec  = qRound( delaytim
                              - ( delayhr * 3600.0 )
                              - ( delaymin * 60.0 ) );  // Residual seconds
   double delaylowm = 0.0;   // Delay-minutes low for delay-hours > 0
   double delaylows = 0.0;   // Delay-seconds low for delay-hours > 0
   double setdlyhr  = ct_dlyhr ->value();  // Currently set hours
   double setdlymin = ct_dlymin->value();  // Currently set minutes
   double setdlysec = ct_dlysec->value();  // Currently set seconds
DbgLv(1) << "EGSp: adjDelay:  spdelta accel delayhr delaymin delaysec"
 << spdelta << accel << delayhr << delaymin << delaysec;
DbgLv(1) << "EGSp: adjDelay:    setdlyhr setdlymin setdlysec"
 << setdlyhr << setdlymin << setdlysec;

   if ( delayhr == 0.0 )
   {  // If delay hours is 0, compute the low minutes needed to accelerate
      double delayls   = qCeil( spdelta / accel );        // Low seconds delay
      double delaylm   = qFloor( delayls / 60.0 );        // Low minutes delay
      delayls         -= qRound( delaylm * 60.0 );        // Residual low seconds
      delaylowm        = delaylm;          // Lower limit on delay minutes

      if ( setdlymin <= delaylm )
      {  // For delay-minutes at or below low-delay-minutes, need to set low seconds
         delaylows        = delayls;
DbgLv(1) << "EGSp: adjDelay:   delaylm delaymin" << delaylm << delaymin
 << "delaylows" << delaylows;
      }
DbgLv(1) << "EGSp: adjDelay:  delaysec delaylows" << delaysec << delaylows
 << "delaylowm" << delaylowm;
   }

   ct_dlymin->setMinimum( delaylowm );
   ct_dlysec->setMinimum( delaylows );
   setdlymin        = ct_dlymin->value();  // (newly?) set minutes
   setdlysec        = ct_dlysec->value();  // (newly?) set seconds

   // Save delay minutes,seconds if changing the minimum changed the value
   double delaynmin = qMax( setdlymin, delaylowm );  // Adjusted minutes?
   double delaynsec = qMax( setdlysec, delaylows );  // Adjusted seconds?

   if ( delaynmin != setdlymin  ||  delaynsec != setdlysec )
   {  // Change in delay time due to minimum:  save new delay in seconds
      delaytim         = ( delayhr * 3600.0 ) + ( delaynmin * 60.0 ) + delaynsec;
      ssvals[ curssx ][ "delay" ] = delaytim;
   }
DbgLv(1) << "EGSp: adjDelay:   setdlymin delaynmin" << setdlymin << delaynmin;
DbgLv(1) << "EGSp: adjDelay:   setdlysec delaynsec" << setdlysec << delaynsec;
}


// Panel for Cells parameters
US_ExperGuiCells::US_ExperGuiCells( QWidget* topw )
{
DbgLv(1) << "EGCe: IN";
   mainw               = (US_ExperimentMain*)topw;
   rpCells             = &(mainw->currProto.rpCells);
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "4: Define cell centerpiece usage" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();


   QLabel* lb_hdr1     = us_banner( tr( "Cell" ) );
   QLabel* lb_hdr2     = us_banner( tr( "Centerpiece" ) );
   QLabel* lb_hdr3     = us_banner( tr( "Windows" ) );
   int row             = 0;
   genL->addWidget( lb_hdr1, row,   0, 1, 1 );
   genL->addWidget( lb_hdr2, row,   1, 1, 6 );
   genL->addWidget( lb_hdr3, row++, 7, 1, 1 );

   cpnames             = sibLValue( "general", "centerpieces" );
   const int mxcels    = 8;
   int nholes          = sibIValue( "rotor",   "nholes" );
DbgLv(1) << "EGCe:  nholes mxcels" << nholes << mxcels;

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

      genL->addWidget( clabl,   row,   0, 1, 1 );
      genL->addWidget( cb_cenp, row,   1, 1, 6 );
      genL->addWidget( cb_wind, row++, 7, 1, 1 );

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
QString pval1 = sibSValue( "rotor", "rotor" );
DbgLv(1) << "EGCe: rotor+rotor=" << pval1;
}

// Function to rebuild the Cells protocol after Rotor change
void US_ExperGuiCells::rebuild_Cells( void )
{
   int nholes          = sibIValue( "rotor", "nholes" );
DbgLv(1) << "EGCe:rbC:  r_nholes" << nholes << "c_ncell" << rpCells->ncell;
   if ( nholes == rpCells->ncell )
      return;                          // No rotor change means no rebuild

   if ( rpCells->ncell == 0 )
   {  // No existing Cells protocol, so initialize a brand-new one
DbgLv(1) << "EGCe:rbC:   ++NEW Cell protocol++";
      rpCells->ncell      = nholes;
      rpCells->nused      = 0;
      rpCells->used.clear();
      return;
   }

   // Otherwise, construct a protocol based on rotor holes
   if ( nholes < rpCells->ncell )
   {  // Decreasing cells count:  Recompute used count
DbgLv(1) << "EGCe:rbC: H<C: nused" << rpCells->nused;
      int kused           = 0;
      for ( int ii = 0; ii < rpCells->nused; ii++ )
      {
         if ( rpCells->used[ ii ].cell > nholes )
            break;
         kused++;
DbgLv(1) << "EGCe:rbC:    ii" << ii << "kused" << kused;
      }
 
      rpCells->nused      = kused;      // Resize used-cells vector
      rpCells->used.resize( kused );
   }
   // If cells count increases, used stays the same; just reset total count

   rpCells->ncell      = nholes;        // Reset total cells count up/down
DbgLv(1) << "EGCe:rbC: ncell" << nholes;
}

// Slot for change in centerpiece selection
void US_ExperGuiCells::centerpieceChanged( int sel )
{
DbgLv(1) << "EGCe:cpChg: sel" << sel;
   QObject* sobj       = sender();      // Sender object
   QString sname       = sobj->objectName();
   int irow            = sname.section( ":", 0, 0 ).toInt();
DbgLv(1) << "EGCe:cpChg:  sname irow" << sname << irow;
   int nholes          = sibIValue( "rotor", "nholes" );
   int icbal           = nholes - 1;    // Counter-balance index

   if ( irow != icbal )
   {  // Not counterbalance:  change cross cell
      int halfnh          = nholes / 2; // Half number holes
      int xrow            = ( irow < halfnh ) ? irow + halfnh : irow - halfnh;
      int jsel            = sel;        // Use same centerpiece for cross
DbgLv(1) << "EGCe:cpChg:  xrow icbal" << xrow << icbal;

      if ( xrow == icbal )
      {  // Cross cell is counterbalance
         jsel                = 1;       // Usually "Beckman counterbalance"
         QString cpname      = cc_cenps[ irow ]->currentText();
DbgLv(1) << "EGCe:cpChg:   cpname" << cpname << "tcb_centps" << tcb_centps;
         if ( tcb_centps.contains( cpname ) )
            jsel                = 2;    // In some cases "Titanium counterbalance"
      }

DbgLv(1) << "EGCe:cpChg:   CB:jsel" << jsel;
      cc_cenps[ xrow ]->setCurrentIndex( jsel );
   }
}

// Slot for change in windows selection
void US_ExperGuiCells::windowsChanged( int sel )
{
DbgLv(1) << "EGCe:wiChg: sel" << sel;
   QObject* sobj       = sender();   // Sender object
   QString sname       = sobj->objectName();
   int irow            = sname.section( ":", 0, 0 ).toInt();
DbgLv(1) << "EGCe:wiChg:  sname irow" << sname << irow;
   int nholes          = sibIValue( "rotor", "nholes" );
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
   mainw               = (US_ExperimentMain*)topw;
   rpSolut             = &(mainw->currProto.rpSolut);
   mxrow               = 24;     // Maximum possible rows
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

   QStringList cpnames = sibLValue( "cells", "centerpieces" );
   int nholes          = sibIValue( "rotor", "nholes" );
   QString add_comm    = tr( "Add to Comments" );
DbgLv(1) << "EGSo:  nholes mxrow" << nholes << mxrow;

   QLabel*       cclabl;
   QComboBox*    cb_solu;
   QPushButton*  pb_comm;

   allSolutions();        // Read in all solution names and IDs

   for ( int ii = 0; ii < mxrow; ii++ )
   {  // Loop to build initial place-holder solution rows
      QString scel;
      if      ( ii == 0 ) scel = QString( "0/A" );
      else if ( ii == 1 ) scel = QString( "0/B" );
      else if ( ii == 2 ) scel = QString( "9/A" );
      else if ( ii == 3 ) scel = QString( "9/B" );
      else                scel = QString( "none" );
      cclabl              = us_label( scel );
      cb_solu             = us_comboBox();
      pb_comm             = us_pushbutton( add_comm );

      QString strow       = QString::number( ii );
      cclabl ->setObjectName( strow + ": label" );
      cb_solu->setObjectName( strow + ": solution" );
      pb_comm->setObjectName( strow + ": addcomm" );

      genL->addWidget( cclabl,  row,    0, 1, 1 );
      genL->addWidget( cb_solu, row,    1, 1, 4 );
      genL->addWidget( pb_comm, row++,  5, 1, 1 );

      cb_solu->addItems( sonames );

      connect( pb_comm, SIGNAL( clicked()           ),
               this,    SLOT  ( addComments()       ) );

      bool is_vis          = ( ii < 4 );
      cclabl ->setVisible( is_vis );
      cb_solu->setVisible( is_vis );
      pb_comm->setVisible( is_vis );

      // Save pointers to row objects for later update
      cc_labls << cclabl;
      cc_solus << cb_solu;
      cc_comms << pb_comm;
   }

   connect( pb_manage,    SIGNAL( clicked()         ),
            this,         SLOT  ( manageSolutions() ) );
   connect( pb_details,   SIGNAL( clicked()         ),
            this,         SLOT  ( detailSolutions() ) );

   panel->addLayout( genL );
   panel->addStretch();

DbgLv(1) << "EGSo:main: call initPanel()";
   initPanel();
}

// Function to rebuild the Solutions protocol after Cells change
void US_ExperGuiSolutions::rebuild_Solut( void )
{
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
DbgLv(1) << "EGSo:detS: ufont" << ufont.family();

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
DbgLv(1) << "EGSo:detS:    ii" << ii << "solu" << sdescr;

      if ( sdescr.contains( usolu ) )  // Skip around "(unspecified)"
         continue;

      QString chanu      = cc_labls[ ii ]->text();
      if ( ! sdescrs.contains( sdescr ) )
      {  // Add solution description to list and begin channels-used list
         sdescrs << sdescr;
         chanuse[ sdescr ] = chanu;
DbgLv(1) << "EGSo:detS:      chanu" << chanu;
      }
      else
      {  // Append to channels-used for solution
         chanuse[ sdescr ] = chanuse[ sdescr ] + ",  " + chanu;
DbgLv(1) << "EGSo:detS:      chanu" << chanuse[sdescr];
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
DbgLv(1) << "EGSo:solDat:  sdescr" << sdescr << "found" << found
 << "solID" << solID;

   if ( found )
   {
      if ( solu_data.keys().contains( sdescr ) )
      {  // Previously fetched and mapped:  just return it
         soludata      = solu_data[ sdescr ];
DbgLv(1) << "EGSo:solDat:    OLDfound descr" << soludata.solutionDesc;
      }

      else
      {  // Not fetched before:  do so now and map it
         US_Passwd pw;
         US_DB2* dbP   = ( sibSValue( "general", "dbdisk" ) == "DB" )
                         ? new US_DB2( pw.getPasswd() ) : NULL;
         if ( dbP != NULL )
         {
            int stat      = soludata.readFromDB( solID.toInt(), dbP );
            found         = ( stat == US_DB2::OK );
DbgLv(1) << "EGSo:solDat:    NEWfound descr" << soludata.solutionDesc << stat;
         }
         else
         {
            soludata.readFromDisk( solID );
         }
         solu_data[ sdescr ] = soludata;
      }
   }

   return found;
}

// Build a mapping of all solution descriptions to solution IDs
int US_ExperGuiSolutions::allSolutions()
{
   sonames.clear();
   sonames << "(unspecified)";
   QStringList soids;

   US_Passwd pw;
   US_DB2* dbP       = ( sibSValue( "general", "dbdisk" ) == "DB" )
                       ? new US_DB2( pw.getPasswd() ) : NULL;
//DbgLv(1) << "EGSo: allSo: dbP" << dbP;
   if ( dbP != NULL )
   {  // Read all the solutions in the database
      soids << "-1";
      QString invID     = sibSValue( "general", "investigator" )
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
         soids   << solID;
      }
   }  // END: solutions in DB

   else
   {  // Read all the solutions on the local disk
      soids << "00000000";
      QString path;
      US_Solution solution;
      if ( ! solution.diskPath( path ) )
         return 0;

      QDir dir( path );
      QStringList filter( "S*.xml" );
      QStringList fnames = dir.entryList( filter, QDir::Files,
                                          QDir::Name );
      QString solID;
      QString descr;

      for ( int ii = 0; ii < fnames.size(); ii++ )
      {  // Examine each S*.xml file
DbgLv(1) << "EGSo: allSo:  file" << (ii+1) << "of" << fnames.size() << fnames[ii];
         QFile* s_file     = new QFile( path + "/" + fnames[ ii ] );

         if ( ! s_file->open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QXmlStreamReader xmli( s_file );

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
                  soids   << solID;
               }
            }  // END: Start element
         }  // END: XML element loop
DbgLv(1) << "EGSo: allSo:      desc" << descr << "solID" << solID;
         s_file->close();
      }  // END: file names loop

//      s_file.close();
   }  // END: solutions on local disk

   // Do a pass through solution names looking for duplicates
   int ndup           = 0;
   for ( int ii = 1; ii < sonames.count(); ii++ )
   {
      QString sname      = sonames[ ii ];
      int lstx           = sonames.lastIndexOf( sname );
      if ( lstx > ii )
      {  // At least one other with this name, make them unique
         ndup++;
         QString snbase     = sname;
         int kk             = 1;
         sname              = snbase + QString().sprintf( "  (%d)", kk );
         sonames.replace( ii, sname );        // Replace 1st of duplicates
         for ( int jj = ii + 1; jj <= lstx; jj++ )
         {
            sname           = sonames[ jj ];

            if ( sname == snbase )
            {  // This is a duplicate
               kk++;
               sname              = snbase + QString().sprintf( "  (%d)", kk );
               sonames.replace( jj, sname );  // Replace each of duplicates
            }
         }
      }  // Handling duplicate
   }  // Testing solution entries

   if ( ndup > 0 )
   {  // There were duplicates, so re-do the name-to-id mapping
      solu_ids.clear();
      for ( int ii = 0; ii < sonames.count(); ii++ )
      { 
         QString sname     = sonames[ ii ];
         solu_ids[ sname ] = soids  [ ii ];
      }
   }  // Re-mapping ids to names
DbgLv(1) << "EGSo: allSo: sids count" << solu_ids.keys().count();
 
   return solu_ids.keys().count();
}

// Slot to handle click on row Add to Comments button
void US_ExperGuiSolutions::addComments()
{
DbgLv(1) << "EGSo:addComm: IN";
   bool ok;
   QString chcomm( "" );
   QStringList comms;
   QString sufx        = "";
   QObject* sobj       = sender();   // Sender object
   QString sname       = sobj->objectName();
   int irow            = sname.section( ":", 0, 0 ).toInt();
DbgLv(1) << "EGSo:addComm: sname irow" << sname << irow;
   QString cclabl      = cc_labls[ irow ]->text();
DbgLv(1) << "EGSo:addComm:  cclabl" << cclabl;
   QString sdescr      = cc_solus[ irow ]->currentText();

   // Get list of channel comment component strings
   //  and compose default channel comment string
   commentStrings( sdescr, chcomm, comms );
   int ncc             = comms.count();  // Number of component strings

   // Start the Add-to-Comments dialog text
   QString msg =
        tr( "The Protocol composes a fixed comment for each<br/>" )
      + tr( "channel that consists of its solution, buffer, analyte(s).<br/>" )
      + tr( "That is, for channel " ) + " <b>" + cclabl + "</b>, "
      + tr( "the Experiment Run <br/>comment is currently:<br/><br/>" );

   // Build initial comments (solution, buffer, analytes)
   msg        += "<b>";
   for ( int jj = 0; jj < ncc; jj++ )
   {
      QString cc  = comms[ jj ];
      if ( ( jj + 1 ) < ncc )
      {  // Not last component string
         if ( cc.length() < 80 )
         {
            msg        += "   \"" + cc + ", \"<br/>";
         }
         else
         {
            QString l1  = QString( cc ).left( 80 );
            QString l2  = QString( cc ).mid( 80 );
            msg        += "   \"" + l1 + "\"<br/>";
            msg        += "   \"" + l2 + ", \"<br/>";
         }
      }

      else
      {  // Last component string
         if ( cc.length() < 80 )
         {  // Append analyte
            msg        += "   \"" + cc  + "\"</b><br/><br/>";
         }
         else
         {  // Append analyte on two lines
            QString l1  = QString( cc ).left( 80 );
            QString l2  = QString( cc ).mid( 80 );
            msg        += "   \"" + l1 + "\"<br/>";
            msg        += "   \"" + l2 + "\"</b><br/><br/>";
         }
      }
   }

   // Complete dialog text and display the dialog
   msg         = msg 
      + tr( "You may enter additional characters to append to<br/>" )
      + tr( "this text, then click on <b>OK</b>:<br/><br/>" );

   sufx        = QInputDialog::getText( this,
      tr( "Add to Experiment's Channel Comments" ),
      msg, QLineEdit::Normal, sufx, &ok );

   if ( ok )
   {  // OK:  append suffix to channel comment
      chcomm     += ", " + sufx;
   }
DbgLv(1) << "EGSo:addComm:  sufx" << sufx;
DbgLv(1) << "EGSo:addComm:   chcomm" << chcomm;
}

// Function to compose channel comment strings (string and list)
void US_ExperGuiSolutions::commentStrings( const QString solname,
      QString& comment, QStringList& comstrngs )
{
   US_Solution soludata;
   solutionData( solname, soludata );
   comstrngs.clear();

   // Start with solution name/description
   comstrngs << solname;               // First string (solution)
   comment        = solname + ", ";    // Beginning of channel comment

   // Append buffer description
   QString buf = soludata.buffer.description;
   comstrngs << buf;                   // Second string (buffer)
   comment       += buf + ", ";        // Append to channel comment

   // Append analytes
   int nana    = soludata.analyteInfo.count();
   for ( int jj = 0; jj < nana; jj++ )
   {
      QString ana = soludata.analyteInfo[ jj ].analyte.description;
      comstrngs << ana;                // Subsequent string (analyte)
      if ( ( jj + 1 ) < nana )
         comment       += ana + ", ";  // Append not-last analyte
      else
         comment       += ana;         // Append last analyte
   }
}


// Panel for Optical Systems parameters
US_ExperGuiOptical::US_ExperGuiOptical( QWidget* topw )
{
   mainw               = (US_ExperimentMain*)topw;
   rpOptic             = &(mainw->currProto.rpOptic);
   mxrow               = 24;     // Maximum possible rows
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "6: Specify optical system scans for each channel" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   QLabel* lb_hdr1     = us_banner( tr( "Cell / Channel" ) );
   QLabel* lb_hdr2     = us_banner( tr( "Optical System Scans to Perform" ) );
   int row             = 1;
   genL->addWidget( lb_hdr1, row,   0, 1, 1 );
   genL->addWidget( lb_hdr2, row++, 1, 1, 3 );

   const int mxcels    = 8;
   int nholes          = sibIValue( "rotor", "nholes" );
DbgLv(1) << "EGOp:  nholes mxcels" << nholes << mxcels;

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
DbgLv(1) << "EGOp:main: optentr" << optentr;
DbgLv(1) << "EGOp:main:  nopten" << nopten << "nopsys" << nopsys;
   QString opsys1   = ( nopten > 6 ) ? optentr[ 6 ]
                                     : tr( "UV/visible" );
   QString opsys2   = ( nopten > 7 ) ? optentr[ 7 ]
                                     : tr( "Rayleigh Interference" );
   QString opsys3   = ( nopten > 8 ) ? optentr[ 8 ]
                                     : tr( "(not installed)" );
DbgLv(1) << "EGOp:main:   opsys1-3" << opsys1 << opsys2 << opsys3;
   int nckopt       = 0;
   nckopt          += opsys1.contains( tr( "not installed" ) ) ? 0 : 1;
   nckopt          += opsys2.contains( tr( "not installed" ) ) ? 0 : 1;
   nckopt          += opsys3.contains( tr( "not installed" ) ) ? 0 : 1;

   for ( int ii = 0; ii < mxrow; ii++ )
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
DbgLv(1) << "EGOp:main:    ii" << ii << "is_vis nckopt" << is_vis << nckopt;
      bg_osyss->button( 1 )->setVisible( is_vis && ( nckopt > 0 ) );
      bg_osyss->button( 2 )->setVisible( is_vis && ( nckopt > 1 ) );
      bg_osyss->button( 3 )->setVisible( is_vis && ( nckopt > 2 ) );

      // Save pointers to row objects for later update
      cc_labls << cclabl;
      cc_osyss << bg_osyss;
   }

   panel->addLayout( genL );
   panel->addStretch();

   initPanel();
}

// Function to rebuild the Optical protocol after Solutions change
void US_ExperGuiOptical::rebuild_Optic( void )
{
}

// Slot to handle an optical system being checked
void US_ExperGuiOptical::opsysChecked( bool checked )
{
DbgLv(1) << "EGOp: oCk: checked" << checked;
   // Determine which row and which of 3 possible check boxes
   QObject* sobj       = sender();      // Sender object
   QString oname       = sobj->objectName();
   int irow            = oname.section( ":", 0, 0 ).toInt();
   int ibtn            = oname.mid( oname.length() - 1 ).toInt();
DbgLv(1) << "EGOp: oCk:  oname" << oname << "irow" << irow << "ibtn" << ibtn;
   // Ignore if not Interference
   QString optype      = cc_osyss[ irow ]->button( ibtn )->text();
DbgLv(1) << "EGOp: oCk:   optype" << optype;
   if ( optype != tr( "Rayleigh Interference" ) )
      return;

   // If Interference, get a list of same-cell rows; disconnect
   QString clabl       = cc_labls[ irow ]->text();
   QString scell       = clabl.left( 1 );
   QString labnone     = tr( "none" );
   QList< int >  ccrows;
DbgLv(1) << "EGOp: oCk:    clabl" << clabl << "scell" << scell;
   for ( int ii = 0; ii < mxrow; ii++ )
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
DbgLv(1) << "EGOp: oCk:     ii" << ii << "rlabl" << rlabl;
      }
   }
DbgLv(1) << "EGOp: oCk: ccrows" << ccrows;

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


// Panel for Spectra parameters
US_ExperGuiSpectra::US_ExperGuiSpectra( QWidget* topw )
{
   mainw               = (US_ExperimentMain*)topw;
   rpSpect             = &(mainw->currProto.rpSpect);
   mxrow               = 24;     // Maximum possible rows
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

   initPanel();
}

// Function to rebuild the Spectra protocol after Optical change
void US_ExperGuiSpectra::rebuild_Spect( void )
{
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
DbgLv(1) << "EGwS:selWl: sname" << sname << "irow" << irow << cclabl;
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
DbgLv(1) << "EGwS:ckOpt: sname" << sname << "irow" << irow << cclabl;
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
DbgLv(1) << "EGwS:loadS: sname" << sname << "irow" << irow << cclabl;
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
DbgLv(1) << "EGwS:manSp: sname" << sname << "irow" << irow << cclabl;
   QMap< double, double >  extinction;
   QString strExtinc   = tr( "Extinction:" );
   bool changed        = false;
   US_Table* tdiag     = new US_Table( extinction, strExtinc, changed );
   tdiag->setWindowTitle( tr( "Manually Enter Spectrum (%1)" )
                          .arg( cclabl ) );
   tdiag->exec();
   if ( changed )
   {
DbgLv(1) << "EGwS:manSp: extinction:" << extinction;
   }
else
DbgLv(1) << "EGwS:manSp: *NOT Accepted*";
}


// Panel for Uploading parameters to Optima DB
US_ExperGuiUpload::US_ExperGuiUpload( QWidget* topw )
{
   mainw               = (US_ExperimentMain*)topw;
   rpRotor             = &(mainw->currProto.rpRotor);
   rpSpeed             = &(mainw->currProto.rpSpeed);
   rpCells             = &(mainw->currProto.rpCells);
   rpSolut             = &(mainw->currProto.rpSolut);
   rpOptic             = &(mainw->currProto.rpOptic);
   rpSpect             = &(mainw->currProto.rpSpect);
   rpUload             = &(mainw->currProto.rpUload);
   uploaded            = false;
   rps_differ          = true;
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
                pb_upload   = us_pushbutton( tr( "Upload to Optima" ) );
                pb_saverp   = us_pushbutton( tr( "Save Run Protocol" ) );

   pb_upload->setEnabled( false );

   // Check boxes showing current completed parameterizations
   QLayout* lo_run          = us_checkbox( tr( "RunID" ),
                                           ck_run,      false );
   QLayout* lo_project      = us_checkbox( tr( "Project" ),
                                           ck_project,  false );
   QLayout* lo_rotor        = us_checkbox( tr( "Lab/Rotor/Calibration" ),
                                           ck_rotor,    true  );
   QLayout* lo_rotor_ok     = us_checkbox( tr( "Rotor user-specified" ),
                                           ck_rotor_ok, false );
   QLayout* lo_speed        = us_checkbox( tr( "Speed Steps" ),
                                           ck_speed,    true  );
   QLayout* lo_speed_ok     = us_checkbox( tr( "Speed user-specified" ),
                                           ck_speed_ok, false );
   QLayout* lo_centerp      = us_checkbox( tr( "some Cell Centerpieces" ),
                                           ck_centerp,  false );
   QLayout* lo_solution     = us_checkbox( tr( "all Channel Solutions" ),
                                           ck_solution, false );
   QLayout* lo_optical      = us_checkbox( tr( "all Channel Optical Systems" ),
                                           ck_optical,  false );
   QLayout* lo_extprofs     = us_checkbox( tr( "Spectra" ),
                                           ck_extprofs, false );
   QLayout* lo_connect      = us_checkbox( tr( "Connected to Optima" ),
                                           ck_connect,  false );
   QLayout* lo_rp_diff      = us_checkbox( tr( "loaded/default Run Protocol"
                                               " differs from the current"
                                               " Run Protocol" ),
                                           ck_rp_diff,  false );
   QLayout* lo_prot_svd     = us_checkbox( tr( "Protocol Saved" ),
                                           ck_prot_svd, false );
   QLayout* lo_upl_enab     = us_checkbox( tr( "Upload Enabled" ),
                                           ck_upl_enab, false );
   QLayout* lo_upl_done     = us_checkbox( tr( "Upload Completed" ),
                                           ck_upl_done, false );
   // Initialize check boxes
   ck_run     ->setEnabled( false );
   ck_project ->setEnabled( false );
   ck_rotor   ->setEnabled( false );
   ck_rotor_ok->setEnabled( true  );
   ck_speed   ->setEnabled( false );
   ck_speed_ok->setEnabled( true  );
   ck_centerp ->setEnabled( false );
   ck_solution->setEnabled( false );
   ck_optical ->setEnabled( false );
   ck_extprofs->setEnabled( false );
   ck_connect ->setEnabled( false );
   ck_rp_diff ->setEnabled( false );
   ck_prot_svd->setEnabled( false );
   ck_upl_enab->setEnabled( false );
   ck_upl_done->setEnabled( false );

   // Build the layout
   int row             = 1;
   genL->addWidget( pb_details,      row,   0, 1, 2 );
   genL->addWidget( pb_connect,      row,   2, 1, 2 );
   genL->addWidget( pb_saverp,       row,   4, 1, 2 );
   genL->addWidget( pb_upload,       row++, 6, 1, 2 );

   genL->addLayout( lo_run,          row,   1, 1, 3 );
   genL->addLayout( lo_project,      row++, 4, 1, 3 );
   genL->addLayout( lo_rotor,        row,   1, 1, 3 );
   genL->addLayout( lo_rotor_ok,     row++, 4, 1, 3 );
   genL->addLayout( lo_speed,        row,   1, 1, 3 );
   genL->addLayout( lo_speed_ok,     row++, 4, 1, 3 );
   genL->addLayout( lo_centerp,      row++, 1, 1, 3 );
   genL->addLayout( lo_solution,     row,   1, 1, 3 );
   genL->addLayout( lo_optical,      row++, 4, 1, 3 );
   genL->addLayout( lo_extprofs,     row,   1, 1, 3 );
   genL->addLayout( lo_connect,      row++, 4, 1, 3 );
   genL->addLayout( lo_rp_diff,      row++, 1, 1, 6 );
   genL->addLayout( lo_prot_svd,     row,   1, 1, 2 );
   genL->addLayout( lo_upl_enab,     row,   3, 1, 2 );
   genL->addLayout( lo_upl_done,     row++, 5, 1, 3 );

   // Connect to slots
   connect( pb_details,   SIGNAL( clicked()          ),
            this,         SLOT  ( detailExperiment() ) );
   connect( pb_connect,   SIGNAL( clicked()          ),
            this,         SLOT  ( testConnection()   ) );
   connect( pb_saverp,    SIGNAL( clicked()          ),
            this,         SLOT  ( saveRunProtocol()  ) );
   connect( pb_upload,    SIGNAL( clicked()          ),
            this,         SLOT  ( uploadExperiment() ) );

   panel->addLayout( genL );
   panel->addStretch();

   // Initialize completion flags
   have_run            = false;
   have_proj           = false;
   have_rotor          = true;
   chgd_rotor          = false;
   have_speed          = true;
   chgd_speed          = false;
   have_cells          = false;
   have_solus          = false;
   have_optic          = false;
   have_spect          = false;
   have_sol            = false;
   rps_differ          = false;
   proto_svd           = false;
   upld_enab           = false;
   uploaded            = false;
   connected           = false;

   // Connect to the Optima if possible
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
DbgLv(1) << "EGUp:main:   opsys1-3" << dblist[6] << dblist[7] << dblist[8];

      dblist << dblist[ 4 ];
      US_Settings::set_def_xpn_host( dblist );
   }

   QString xpnhost     = dblist[ 1 ];
   int     xpnport     = dblist[ 2 ].toInt();
   QString dbname      = dblist[ 3 ];
   QString dbuser      = dblist[ 4 ];
   QString dbpasw      = dblist[ 5 ];
DbgLv(1) << "EGUp: host port name user pasw" << xpnhost << xpnport
 << dbname << dbuser << dbpasw;
   US_XpnData* xpn_data = new US_XpnData();
   connected           = xpn_data->connect_data( xpnhost, xpnport, dbname,
                                                 dbuser,  dbpasw );
DbgLv(1) << "EGUp:  connected" << connected;
   xpn_data->close();
   delete xpn_data;
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
DbgLv(1) << "EGUp:detE: ufont" << ufont.family();
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Accumulate information on controls that have been specified
   QString v_invid   = mainw->currProto.investigator;
   QString v_dbdisk  = sibSValue( "general",   "dbdisk" );
   QString v_run     = mainw->currProto.runname;
   QString v_proj    = mainw->currProto.project;
   QString v_prot    = mainw->currProto.protname;
   QString v_ohost   = mainw->currProto.optimahost;
   double  d_temper  = mainw->currProto.temperature;
   QString v_temper  = QString::number( d_temper );
   QString v_lab     = rpRotor->laboratory;
   QString v_rotor   = rpRotor->rotor;
   QString v_calib   = rpRotor->calibration;
   QString v_centp   = sibSValue( "cells",     "nused" );
   QString v_ccbal   = sibSValue( "cells",     "counterbalance" );
   int     i_nspeed  = rpSpeed->nstep;
   QString v_nspeed  = QString::number( i_nspeed );
   QString v_nsolct  = sibSValue( "solutions", "nchant" );
   QString v_nsolcu  = sibSValue( "solutions", "nchanu" );
   int     i_nsolcf  = rpSolut->nschan;
   QString v_nsolcf  = QString::number( i_nsolcf );
   int     i_nsolun  = rpSolut->nuniqs;
   QString v_nsolun  = QString::number( i_nsolun );
DbgLv(1) << "EGUp:dE: n speed,solct,solun" << v_nspeed << v_nsolct << v_nsolun;

   QStringList sspeed   = sibLValue( "speeds",    "profiles" );
DbgLv(1) << "EGUp:dE: speed profiles" << sspeed;
   QStringList scentp   = sibLValue( "cells",     "centerpieces" );
DbgLv(1) << "EGUp:dE: cells centerpieces" << scentp;
   QStringList ssolut   = sibLValue( "solutions", "solutions" );
DbgLv(1) << "EGUp:dE: solus solus" << ssolut;

   bool chk_run      = ! v_run .isEmpty();
   bool chk_project  = ! v_proj.isEmpty();
   bool chk_rotor_ok = ( sibIValue( "rotor",     "changed" ) > 0 );
   bool chk_speed_ok = ( sibIValue( "speeds",    "changed" ) > 0 );
   bool chk_centerp  = ( v_centp.toInt() > 0 );
   bool chk_solution = ( sibIValue( "solutions", "alldone" ) > 0 );
   bool chk_extprofs = ( sibIValue( "photomult", "alldone" ) > 0 );
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
   dtext += tr( "  ALL SPECIFIED:              " ) + v_genok  + "\n";
   dtext += tr( "  Investigator:               " ) + v_invid  + "\n";
   dtext += tr( "  DB / Disk:                  " ) + v_dbdisk + "\n";
   dtext += tr( "  RunId:                      " ) + v_run    + "\n";
   dtext += tr( "  Project:                    " ) + v_proj   + "\n";
   dtext += tr( "\nRotor\n" );
   dtext += tr( "  ALL SPECIFIED:              " ) + v_rotok  + "\n";
   dtext += tr( "  USER CHANGES:               " ) + v_rotuc  + "\n";
   dtext += tr( "  Laboratory:                 " ) + v_lab    + "\n";
   dtext += tr( "  Rotor:                      " ) + v_rotor  + "\n";
   dtext += tr( "  Calibration:                " ) + v_calib  + "\n";
   dtext += tr( "\nSpeeds\n" );
   dtext += tr( "  ALL SPECIFIED:              " ) + v_speok  + "\n";
   dtext += tr( "  USER CHANGES:               " ) + v_speuc  + "\n";
   dtext += tr( "  Number Speed Steps:         " ) + v_nspeed + "\n";

   int nspeed        = v_nspeed.toInt();
DbgLv(1) << "EGUp:dE: nspeed" << nspeed;
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
   dtext += tr( "  ALL SPECIFIED:              " ) + v_celok  + "\n";
   dtext += tr( "  Non-empty Centerpieces:     " ) + v_centp  + "\n";

DbgLv(1) << "EGUp:dE: ncentp" << scentp.count();
   for ( int ii = 0; ii < scentp.count(); ii++ )
   {
      dtext += "    " + scentp[ ii ] + "\n";
   }

   if ( v_ccbal.isEmpty() )
   {
      dtext += tr( "  Counterbalance:             (empty)\n" );
   }
   else
   {
      dtext += tr( "  Counterbalance:             " ) + v_ccbal  + "\n";
   }

   dtext += tr( "\nSolutions\n" );
   dtext += tr( "  ALL SPECIFIED:              " ) + v_solok  + "\n";
   dtext += tr( "  Number Channels Used:       " ) + v_nsolcf +
            tr( " of " ) +  v_nsolct + "\n";

   for ( int ii = 0; ii < rpSolut->nschan; ii++ )
   {
      dtext += tr( "  Channel " ) +
               rpSolut->chsols[ ii ].channel + " :  " +
               tr( "protocol comments" ) + " --\n    " +
               rpSolut->chsols[ ii ].ch_comment + "\n";
      dtext += "                :  " +
               tr( "run comments" ) + " --\n    " +
               rpSolut->chsols[ ii ].ch_comment + ", plus more\n";
   }

   dtext += tr( "  Number Unique Solutions:    " ) + v_nsolun + "\n";
DbgLv(1) << "EGUp:dE: " << QString(":%1:%2:%3: (:solok:nsolcf:nsolun:)")
 .arg(v_solok).arg(v_nsolcf).arg(v_nsolun);

DbgLv(1) << "EGUp:dE: nsolut" << ssolut.count();
   for ( int ii = 0; ii < ssolut.count(); ii++ )
   {
      dtext += "    " + ssolut[ ii ] + "\n";
   }

   dtext += tr( "\nPhoto Multiplier Spectra\n" );
   dtext += tr( "  ALL SPECIFIED:              " ) + v_phook  + "\n";
   dtext += tr( "\nUpload\n" );
   dtext += tr( "  CONNECTED:                  " ) + v_conok  + "\n";
   dtext += tr( "  UPLOAD ENABLED:             " ) + v_uleok  + "\n";
   dtext += tr( "  UPLOAD COMPLETED:           " ) + v_ulcok  + "\n";

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
DbgLv(1) << "EGUp: host port name user pasw" << xpnhost << xpnport
 << dbname << dbuser << dbpasw;
   US_XpnData* xpn_data = new US_XpnData();
   connected           = xpn_data->connect_data( xpnhost, xpnport, dbname,
                                                 dbuser,  dbpasw );
DbgLv(1) << "EGUp:  connected" << connected;
   xpn_data->close();
   delete xpn_data;
   QString mtitle;
   QString message;

   if ( connected )
   {  // Let the user know that connection is made and set flag
      mtitle    = tr( "Successful Connection to Optima" );
      message   = tr( "The connection to the Optima has been made.\n"
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
      mtitle    = tr( "Failed Connection to Optima" );
      message   = tr( "The failure to connect to the Optima most likely means\n"
                      "that host/port/name/user are misconfigured.\n"
                      "Reset them in UltraScan's 'Optima Host Preferences'\n"
                      "and return to retry connecting here." );
      QMessageBox::critical( this, mtitle, message );
   }

   initPanel();
}

// Slot to save the current Run Protocol
void US_ExperGuiUpload::saveRunProtocol()
{
DbgLv(1) << "EGUp:svRP: IN";
   // Test that the current protcol name is new
   QStringList           prnames;
   QList< QStringList >  prdats;
DbgLv(1) << "EGUp:svRP:  call getProtos()";
   mainw->getProtos( prnames, prdats );
DbgLv(1) << "EGUp:svRP:   prnames" << prnames;

   QString protname    = sibSValue( "general", "protocol" );
DbgLv(1) << "EGUp:svRP:  protname" << protname << "prdats0" << prdats[0];

   if ( prnames.contains( protname ) )
   {  // Cannot save until a new protocol name is given
      QString mtitle  = tr( "Protocol Name not New" );
      QString message = tr( "The current Run Protocol cannot be saved until\n"
                            "a unique new name is given for it.\n\n"
                            "In the dialog to follow, add a distinguishing\n"
                            "suffix to the current protocol name/description\n"
                            "or create a completely new description." );
      QMessageBox::critical( this, mtitle, message );
   }
   bool ok;
   QString newpname    = protname;
   QString msg         =
      tr( "Enter a new Run Protocol name (description text)<br/>"
          "for the record to be saved; or modify the<br/>"
          "existing name so that it is unique.<br/><br/>"
          "Then click on <b>OK</b> to accept the new name<br/>"
          "or on <b>Cancel</b> to abort the Run Protocol save.<br/>" );

   // Keep displaying the dialog text until a unique name is given
   while( prnames.contains( newpname ) )
   {
      newpname            = QInputDialog::getText( this,
                               tr( "Enter New Run Protocol Name/Description" ),
                               msg, QLineEdit::Normal, newpname, &ok );

      if ( ! ok )
      {  // Cancel:  abort the save
         QMessageBox::critical( this,
            tr( "Save Run Protocol Aborted" ),
            tr( "The save of a new Run Protocol record<br/>"
                "has been aborted." ) );
         return;
      }
   }

   // Save the new name and update entries list
   protname            = newpname;
DbgLv(1) << "EGUp:svRP:   NEW protname" << protname;

   currProto->protname = protname;            // Update current protocol
   currProto->pGUID    = US_Util::new_guid();
DbgLv(1) << "EGUp:svRP:   currProto updated";

   QXmlStreamWriter xmlo( &rpUload->us_xml ); // Compose XML representation
   currProto->toXml( xmlo );

   QStringList prentry( protname );           // New protocol summary data
   QString pdate       = "2017-03-05";
   QString protid      = "1";
   QString pguid       = currProto->pGUID;
   prentry << pdate << protid << pguid;

//   mainw->updateProtos( prentry );
DbgLv(1) << "EGUp:svRP:  new protname" << protname << "prdats0" << prdats[0];
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
#if QT_VERSION > 0x050000

   // Accumulate information on controls that have been specified
   QString v_run     = sibSValue( "general",   "runID" );
   QString v_proj    = sibSValue( "general",   "project" );
   QString v_invid   = sibSValue( "general",   "investigator" );
   QString v_dbdisk  = sibSValue( "general",   "dbdisk" );
   QString v_lab     = sibSValue( "rotor",     "lab" );
   QString v_rotor   = sibSValue( "rotor",     "rotor" );
   QString v_calib   = sibSValue( "rotor",     "calib" );
   QString v_centp   = sibSValue( "cells",     "nused" );
   QString v_nspeed  = sibSValue( "speeds",    "nspeeds" );
   QString v_nsolct  = sibSValue( "solutions", "nchant" );
   QString v_nsolcu  = sibSValue( "solutions", "nchanu" );
   QString v_nsolcf  = sibSValue( "solutions", "nchanf" );
   QString v_nsolun  = sibSValue( "solutions", "nusols" );
DbgLv(1) << "EGUp:bj: n speed,solct,solun" << v_nspeed << v_nsolct << v_nsolun;

   QStringList sspeed   = sibLValue( "speeds",    "profiles" );
DbgLv(1) << "EGUp:bj: speed profiles" << sspeed;
   QStringList scentp   = sibLValue( "cells",     "centerpieces" );
DbgLv(1) << "EGUp:bj: cells centerpieces" << scentp;
   QStringList ssolut   = sibLValue( "solutions", "channel_solutions" );
DbgLv(1) << "EGUp:bj: solus solus" << ssolut;

   bool chk_run      = ! v_run .isEmpty();
   bool chk_project  = ! v_proj.isEmpty();
   bool chk_centerp  = ( v_centp.toInt() > 0 );
   bool chk_solution = ( sibSValue( "solutions", "alldone" ).toInt() > 0 );
   bool chk_extprofs = ( sibSValue( "photomult", "alldone" ).toInt() > 0 );
DbgLv(1) << "EGUp:bj: ck: run proj cent solu epro"
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
DbgLv(1) << "EGUp:bj: js_exper" << js_exper;
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

#endif
   return js_exper;
}

