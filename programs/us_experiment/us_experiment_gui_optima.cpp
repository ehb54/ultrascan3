//! \file us_experiment/us_experiment_gui_optima.cpp

#include <QSslKey>

#include "us_experiment_gui_optima.h"
#include "../us_analysis_profile/us_analysis_profile.h"
#include "us_rotor_gui.h"
#include "us_solution_gui.h"
#include "us_extinction_gui.h"
#include "us_table.h"
#include "us_xpn_data.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_sleep.h"
#include "us_util.h"
#include "us_crypto.h"
#include "us_datafiles.h"
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

   connection_status = false;
   automode = false;
   usmode = false;
   global_reset = false;
   instruments_in_use.clear();
   ScanCount_global       = 0;
   ScanCount_global_int   = 0;
   TotalWvlNum_global     = 0;
   
   // Create tab and panel widgets
   tabWidget           = us_tabwidget();

   tabWidget->setTabPosition( QTabWidget::North );

   epanGeneral         = new US_ExperGuiGeneral  ( this );
   epanRotor           = new US_ExperGuiRotor    ( this );
   epanSpeeds          = new US_ExperGuiSpeeds   ( this );
   epanCells           = new US_ExperGuiCells    ( this );
   epanSolutions       = new US_ExperGuiSolutions( this );
   epanOptical         = new US_ExperGuiOptical  ( this );
   epanRanges          = new US_ExperGuiRanges   ( this );
   epanAProfile        = new US_ExperGuiAProfile ( this );
   epanUpload          = new US_ExperGuiUpload   ( this );
   statflag            = 0;

   // Add panels to the tab widget
   tabWidget->addTab( epanGeneral,   tr( "1: General"   ) );
   tabWidget->addTab( epanRotor,     tr( "2: Lab/Rotor" ) );
   tabWidget->addTab( epanSpeeds,    tr( "3: Speeds"    ) );
   tabWidget->addTab( epanCells,     tr( "4: Cells"     ) );
   tabWidget->addTab( epanSolutions, tr( "5: Solutions" ) );
   tabWidget->addTab( epanOptical,   tr( "6: Optics"    ) );
   tabWidget->addTab( epanRanges,    tr( "7: Ranges"    ) );
   tabWidget->addTab( epanAProfile,  tr( "8: AProfile"  ) );
   tabWidget->addTab( epanUpload,    tr( "9: Submit"    ) );
   tabWidget->setCurrentIndex( curr_panx );

   //tabWidget->tabBar()->setEnabled(false);


   // Add bottom buttons
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   pb_prev   = us_pushbutton( tr( "Previous Panel" ) );
   pb_next   = us_pushbutton( tr( "Next Panel" ) );
   pb_close  = us_pushbutton( tr( "Close" ) );;
   buttL->addWidget( pb_help  );
   buttL->addWidget( pb_prev  );
   buttL->addWidget( pb_next  );
   buttL->addWidget( pb_close );

   tabHeight = tabWidget->tabBar()->height();
   buttLHeight = pb_prev->height();

   // Connect signals to slots
   connect( tabWidget, SIGNAL( currentChanged( int ) ),
            this,      SLOT  ( newPanel      ( int ) ) );
   connect( pb_next,   SIGNAL( clicked()    ),
            this,      SLOT  ( panelUp()    ) );
   connect( pb_prev,   SIGNAL( clicked()    ),
            this,      SLOT  ( panelDown()  ) );
   connect( pb_close,  SIGNAL( clicked()    ),
            this,      SLOT  ( close_program()      ) );
   connect( pb_help,   SIGNAL( clicked()    ),
            this,      SLOT  ( help()       ) );

   connect( epanUpload, SIGNAL( expdef_submitted( QMap < QString, QString > &) ),
            this,       SLOT  ( optima_submitted( QMap < QString, QString > & ) ) );

   connect( epanAProfile->sdiag, SIGNAL( back_to_pcsa_signal() ),
            this,       SLOT  ( back_to_pcsa() ) );



   main->addWidget( tabWidget );
   main->addLayout( statL );
   main->addLayout( buttL );

   connect( epanGeneral, SIGNAL( set_tabs_buttons_inactive( void )),
            this,        SLOT(   disable_tabs_buttons( void ) ));
   connect( epanGeneral, SIGNAL( set_tabs_buttons_active_readonly( void )),
              this,      SLOT(   enable_tabs_buttons_readonly( void ) ));
   connect( epanGeneral, SIGNAL( set_tabs_buttons_active( void )),
              this,      SLOT(   enable_tabs_buttons( void ) ));

   //int min_width = tabWidget->tabBar()->width();

   //setMinimumSize( QSize( min_width, 450 ) );
   setMinimumSize( 950, 450 );
   adjustSize();

   //epanGeneral->initPanel();
   epanGeneral->loaded_proto = 0;
   //epanGeneral->update_inv();
   epanGeneral->check_user_level();
   epanGeneral->check_runname();

   //reset();
}

// Go back to PCSA when syntax errors
void US_ExperimentMain::back_to_pcsa( void )
{
  qDebug() << "Going back to PCSA!";
  tabWidget->setCurrentIndex( 7 );
  
}

     
// Reset parameters to their defaults
void US_ExperimentMain::reset( void )
{
  global_reset = true;

  // //Clean Reset
  // currProto = US_RunProtocol();
  // loadProto = US_RunProtocol();

    
  epanRotor->setFirstLab();  //need to reset Lab && savePanel() for Rotors

  /**************** */
  //Cells
  currProto.rpCells.ncell = 0;
  epanCells->initPanel();

  //Solutions
  currProto.rpSolut.nschan = 0;
  epanSolutions->initPanel();

  //Optics
  currProto.rpOptic.nochan = 0;
  epanOptical->initPanel();

  //Ranges
  currProto.rpRange.nranges = 0;
  epanRanges->initPanel();
  
  //AProfile
  epanAProfile->reset_sdiag(); //need to reset basic AProfile's protocol to defaults
  /****************************/
  
  
  tabWidget->setCurrentIndex( 0 );

  epanGeneral->resetPanel();


  /*
  // Reset General panel
  currProto.runname      = "";
  currProto.protoname    = "";
  currProto.project      = "";
  currProto.temperature  = 20.0;
  currProto.temeq_delay  = 10.0;
  epanGeneral->initPanel();

  //Cells
  currProto.rpCells.ncell = 0;
  epanCells->initPanel();

  //Solutions
  currProto.rpSolut.nschan = 0;
  epanSolutions->initPanel();

  //Optics
  currProto.rpOptic.nochan = 0;
  epanOptical->initPanel();

  //Ranges
  currProto.rpRange.nranges = 0;
  epanRanges->initPanel();

  //AProfile
  //if ( !usmode )
  epanAProfile->initPanel();
  */

}

US_AnaProfile* US_ExperimentMain::get_aprofile( )
{
  return &(epanAProfile->sdiag->currProf);
}

void US_ExperimentMain::exclude_used_instruments( QStringList & occupied_instruments )
{

  // QMessageBox * msg_expsetup = new QMessageBox;
  // msg_expsetup->setIcon(QMessageBox::Information);
  // msg_expsetup->setWindowFlags ( Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  // msg_expsetup->setWindowTitle(tr( "Setting Up Experiment Stage") );
  // msg_expsetup->setText(tr( "Preparing to set up new Experiment...") );
  // msg_expsetup-> setInformativeText( "<font color='red'>Preparing to set up new Experiment...</font>" );
  // msg_expsetup->setModal(false);
  // //msg_expsetup->raise();
  // msg_expsetup->show();
  // msg_expsetup->update();
  // qApp->processEvents();

  reset();

  instruments_in_use.clear();
  qDebug() << "OCCUPIED IINSTRUMENTS: " << occupied_instruments;

  for ( int i=0; i < occupied_instruments.size(); i++)
    instruments_in_use << occupied_instruments[i];

  //Re-initialize Instruments based on  the passed excluded list
  epanRotor->setFirstLab();

  //qApp->processEvents();
  //msg_expsetup->close();
  emit close_expsetup_msg();
}

void US_ExperimentMain::us_mode_passed( void )
{
  qDebug() << "US_MODE SIGNAL: ";
  usmode = true;
  this->tabWidget->removeTab(7);
  this->tabWidget->setTabText( 7, "8: Submit");
}

void US_ExperimentMain::auto_mode_passed( void )
{
  qDebug() << "AUTOMODE SIGNAL: ";
  automode = true;
  //epanUpload->reinitPanel();
  epanUpload->     pb_connect->hide();
  epanUpload->     pb_saverp->hide();
  epanUpload->genL->addWidget( epanUpload->pb_details, 1,   0, 1, 4 );
  epanUpload->genL->addWidget( epanUpload->pb_submit,  1,   4, 1, 8 );

  epanGeneral->lb_label->setVisible(true);
  epanGeneral->le_label->setVisible(true);

  this->pb_close->hide();

}

// Reset parameters to their defaults
void US_ExperimentMain::close_program( void )
{
  emit us_exp_is_closed();
  close();
}

// When run submitted to Optima
void US_ExperimentMain::us_exp_clear( QString &protocolName )
{
  tabWidget->setCurrentIndex( 0 );
  qDebug() << "Protocol Name to be cleared: " << protocolName;

  // ALEXEY: do proper reset of everything
  reset();

  emit exp_cleared();
}

// When run submitted to Optima
void US_ExperimentMain::optima_submitted( QMap < QString, QString > &protocol_details )
{
  tabWidget->setCurrentIndex( 0 );
  emit to_live_update( protocol_details );
}

// Panel for run and other general parameters
US_ExperGuiGeneral::US_ExperGuiGeneral( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
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
   QLabel*      lb_tedelay      = us_label( tr( "Temperature-Equilibration Delay" ) );
   QLabel*      lb_tedmins      = us_label( tr( "Minutes" ) );
   pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   pb_project      = us_pushbutton( tr( "Select Project" ) );
   QPushButton* pb_protocol     = us_pushbutton( tr( "Load Protocol" ) );
                le_runid        = us_lineedit( "", 0, false );
                le_protocol     = us_lineedit( "", 0, false );
                le_project      = us_lineedit( "", 0, true  );
                ct_tempera      = us_counter ( 2, 0,  40, 20 );
                ct_tedelay      = us_counter ( 2, 0, 120, 10 );
   int          ihgt            = pb_protocol->height();
   QSpacerItem* spacer1         = new QSpacerItem( 20, ihgt );
   QSpacerItem* spacer2         = new QSpacerItem( 20, ihgt );

   //ALEXEY: if autoflow mode, add label field
   lb_label          = us_label( tr( "Label:" ) );
   le_label          = us_lineedit( "", 0, false );


   le_runid->setPlaceholderText("Enter Run ID to continue");
   le_project->setPlaceholderText("Select Project to continue");

   ct_tempera->setSingleStep( 1 );
   ct_tempera->setValue     ( 20 );
   ct_tempera->adjustSize   ();
   ct_tedelay->setSingleStep( 1 );
   ct_tedelay->setValue     ( 10 );
   ct_tedelay->adjustSize   ();

   //IMPORTANT: set your own credentials (as in DB)
   update_inv();
   
   // Set up an approprate investigator text
   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   int id          = US_Settings::us_inv_ID();
   QString invnbr  = ( id > 0 ) ?
      QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";
   QString invtxt  = invnbr + US_Settings::us_inv_name();
   le_investigator = us_lineedit( invtxt, 0, true );
DbgLv(1) << "EGGe:main: invtxt" << invtxt
 << "invlev" << US_Settings::us_inv_level()
 << "invenab" << pb_investigator->isEnabled();
//*DEBUG*
QStringList gxhosts;
QList< QStringList > gxentrs;
gxentrs = US_Settings::xpn_db_hosts();
for (int jj=0;jj<gxentrs.count();jj++)
{
 DbgLv(1) << "EGGe:main: jj xpnentr" << jj << gxentrs[jj];
}
//*DEBUG*

   // Set defaults
   currProto       = &mainw->currProto;
   currProto->investigator = invtxt;
   currProto->runname      = "";
   currProto->protoname    = "";
   currProto->protoID      = 0;
   currProto->project      = "";
   currProto->temperature  = 20.0;
   currProto->temeq_delay  = 10.0;
   currProto->exp_label    = "";

   // Build main layout
   int row         = 0;
   genL->addWidget( pb_investigator, row,   0, 1, 2 );
   genL->addWidget( le_investigator, row++, 2, 1, 6 );
   genL->addWidget( lb_runid,        row,   0, 1, 2 );
   genL->addWidget( le_runid,        row++, 2, 1, 6 );

   genL->addWidget( lb_label,        row,   0, 1, 2 );
   genL->addWidget( le_label,        row++, 2, 1, 6 );

   genL->addWidget( pb_protocol,     row,   0, 1, 2 );
   genL->addWidget( le_protocol,     row++, 2, 1, 6 );
   genL->addWidget( pb_project,      row,   0, 1, 2 );
   genL->addWidget( le_project,      row++, 2, 1, 6 );
   genL->addWidget( lb_tempera,      row,   0, 1, 3 );
   genL->addWidget( ct_tempera,      row,   3, 1, 2 );
   genL->addItem  ( spacer1,         row++, 5, 1, 3 );
   genL->addWidget( lb_tedelay,      row,   0, 1, 3 );
   genL->addWidget( ct_tedelay,      row,   3, 1, 2 );
   genL->addWidget( lb_tedmins,      row++,   5, 1, 1 );

   genL->addItem  ( spacer2,         row++, 6, 1, 2 );

   panel->addLayout( genL );
   panel->addStretch();

   //ALEXEY: hide in regular us_experiment, show in auto_mode (later)
   lb_label->hide();
   le_label->hide();

   // Set up signal and slot connections
   connect( le_runid,        SIGNAL( textEdited(const QString &)  ),
            this,            SLOT(   check_empty_runname(const QString &) ) );
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
DbgLv(1) << "EGGe:main: prnames,prdata counts" << pr_names.count() << protdata.count();

 mainw->solutions_change = false;

 //check_runname();

   // Do the initialization we do at panel entry
DbgLv(1) << "EGGe:main: (2)pb_inv_enab" << pb_investigator->isEnabled();
   initPanel();
DbgLv(1) << "EGGe:main: (3)pb_inv_enab" << pb_investigator->isEnabled();
}


//resetPanel
void US_ExperGuiGeneral::resetPanel( void )
{
  le_runid       ->setText ("");
  le_protocol    ->setText ("");
  le_project     ->setText ("");
  le_label       ->setText ("");

  // Set up an approprate investigator text
  if ( US_Settings::us_inv_level() < 1 )
    pb_investigator->setEnabled( false );

  int id          = US_Settings::us_inv_ID();
  QString invnbr  = ( id > 0 ) ?
    QString::number( US_Settings::us_inv_ID() ) + ": "
    : "";
  QString invtxt  = invnbr + US_Settings::us_inv_name();
  le_investigator->setText ( invtxt );

  qDebug() << "Investigator After reset: " << invtxt;

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
      currProto->runname = rname;
   }

   check_runname();
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
   int old_investID  = investID;

   qDebug() << "Old invID: " << investID;

   US_Investigator* dialog = new US_Investigator( true, investID );
   check_user_level();
   dialog->override_permit( usr_enab );
   dialog->exec();

   investID         = US_Settings::us_inv_ID();

   QString inv_text = QString::number( investID ) + ": "
                      +  US_Settings::us_inv_name();

   currProto->investigator  = inv_text;
   le_investigator->setText( inv_text );

   DbgLv(1) << "User Level: " << US_Settings::us_inv_level();
   qDebug() << "NEW invID: " << investID;


   // ALEXEY: Re-read in summary information on all existing run protocols when user changed
#if 1  // GARY: Suppress re-read after investigator change
   if ( investID != old_investID )
   {
      mainw->solutions_change = true;

      mainw->connection_for_instrument.clear();  //ALEXEY clear checked connections for all instruments

      bool fromdisk         = US_Settings::debug_match( "protocolFromDisk" );
      bool load_db          = fromdisk ? false : use_db;
      US_Passwd  pw;
      US_DB2* dbP           = load_db ? new US_DB2( pw.getPasswd() ) : NULL;

      US_ProtocolUtil::list_all( protdata, dbP );

      for ( int ii = 0; ii < protdata.count(); ii++ )
         pr_names << protdata[ ii ][ 0 ];
DbgLv(1) << "EGGe:main: prnames,prdata counts" << pr_names.count() << protdata.count();

      // Reset and Initialize
      currProto->runname      = "";
      currProto->protoname    = "";
      currProto->protoID      = 0;
      currProto->project      = "";
      currProto->temperature  = 20.0;
      currProto->temeq_delay  = 10.0;

      qDebug() << "currProto->investigator 1: -- " <<  currProto->investigator;
      
      initPanel();

      qDebug() << "currProto->investigator 2: -- " <<  currProto->investigator;
      
      le_protocol->setText( "" );
      le_project ->setText( "" );
      le_label   ->setText( "" );

      currProto  ->exp_label  = "";

      /////mainw->reset();  //testing
   }
#endif
#if 0  // GARY: Rebuild protocol list after investigator change
   if ( investID != old_investID )
   {
      mainw->solutions_change = true;

      bool fromdisk         = US_Settings::debug_match( "protocolFromDisk" );
      bool load_db          = fromdisk ? false : use_db;
      US_Passwd  pw;
      US_DB2* dbP           = load_db ? new US_DB2( pw.getPasswd() ) : NULL;

      US_ProtocolUtil::list_all( protdata, dbP );

      for ( int ii = 0; ii < protdata.count(); ii++ )
         pr_names << protdata[ ii ][ 0 ];
DbgLv(1) << "EGGe:main: prnames,prdata counts" << pr_names.count() << protdata.count();
   }
#endif

   check_runname();

    qDebug() << "currProto->investigator 3: -- " <<  currProto->investigator;

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
   int protoID           = 0;

   // Build dialog table widget headers
   hdrs << "Protocol Name"
        << "Created"
        << ( load_db ? "DbID" : "File Name" );

   // Select a protocol
   QString delete_button = "DELETE";
   //US_SelectItem pdiag( protdata, hdrs, pdtitle, &prx, -2 );
   US_SelectItem* pdiag = new  US_SelectItem( protdata, hdrs, pdtitle, &prx, delete_button, -2 );  //ALEXEY <-- with Delete button and functionality

   connect( pdiag, SIGNAL( accept_deletion() ), this, SLOT( update_protdata() ));

   if ( pdiag->exec() == QDialog::Accepted )
   {  // Accept in dialog:  get selected protocol name and its XML
     DbgLv(1) << "EGGe:ldPro:  ACCEPT  prx" << prx << "sel proto" << protdata[prx][0] << "protID" << protdata[prx][2];

     qDebug() << "In load_protocol: before RESET, currProto->investigator: -- " << currProto->investigator;
     //ALEXEY: need to reset everything:
     mainw->reset();
     qDebug() << "In load_protocol: after RESET, currProto->investigator: -- " << currProto->investigator;
     
      QString pname         = protdata[ prx ][ 0 ];

      // Get the protocol XML that matches the selected protocol name
      protoID               = US_ProtocolUtil::read_record( pname, &xmlstr, NULL, dbP );
DbgLv(1) << "EGGe:ldPro:  ACCEPT   read_record return len(xml)" << xmlstr.length()
 << "protoID" << protoID;

      le_protocol->setText( pname );
      mainw->currProto.protoID = protoID;
      mainw->loadProto.protoID = protoID;
   }

   else
   {  // Reject in dialog
DbgLv(1) << "EGGe:ldPro:  REJECT";
      return;
   }

   // Now that we have a protocol XML, convert it to internal controls
   QXmlStreamReader xmli( xmlstr );
   mainw->loadProto.fromXml( xmli );
   mainw->loadProto.protoID = protoID;

   // Initialize the current protocol from the loaded one; set temperature
   mainw->currProto      = mainw->loadProto;
   ct_tempera->setValue( mainw->currProto.temperature );
   ct_tedelay->setValue( mainw->currProto.temeq_delay );
   //ALEXEY: set Project name
   le_project->setText ( mainw->currProto.project );

DbgLv(1) << "EGGe:ldPro:    dur0" << mainw->currProto.rpSpeed.ssteps[0].duration;
DbgLv(1) << "EGGe:ldPro:    cPname" << mainw->currProto.protoname
 << "lPname" << mainw->loadProto.protoname << "pdbID" << mainw->loadProto.protoID;
DbgLv(1) << "EGGe:ldPro:    cOhost" << mainw->currProto.optimahost
 << "lOhost" << mainw->loadProto.optimahost;
DbgLv(1) << "EGGe:ldPro:    cTempe" << mainw->currProto.temperature
 << "lTempe" << mainw->loadProto.temperature;

   QString rname     = le_runid->text();
   if ( !rname.isEmpty() )
      mainw->currProto.runname = rname;
   loaded_proto = 1;

   // If there is a linked AnalysisProfile, add it

   // Initialize all other panels using the new protocol

   qDebug() << "In load_protocol: currProto->investigator 1 --  " <<  currProto->investigator;
      
   mainw->initPanels();

   qDebug() << "In load_protocol: currProto->investigator 2 --  " <<  currProto->investigator;
   
   check_runname();
}

// Update protdata when protocol deleted in pdialog...
void US_ExperGuiGeneral::update_protdata( void )
{
   bool fromdisk         = US_Settings::debug_match( "protocolFromDisk" );
   bool load_db          = fromdisk ? false : use_db;
   US_Passwd  pw;
   US_DB2* dbP           = load_db ? new US_DB2( pw.getPasswd() ) : NULL;

   US_ProtocolUtil::list_all( protdata, dbP );

   pr_names.clear();
   for ( int ii = 0; ii < protdata.count(); ii++ )
      pr_names << protdata[ ii ][ 0 ];

   qDebug() << "# of pr_names: " << pr_names.count();
}

// Verify entered protocol name
void US_ExperGuiGeneral::changed_protocol( void )
{
   QString protoname    = le_protocol->text();

   if ( pr_names.contains( protoname )  ||  protoname.trimmed() == "" )
   {
      QString msg          =
         tr( "The protocol name given<br/>" )
         +  "  ( <b>" + protoname + "</b> )<br/>"
         + tr( "is already being used or empty.  It will need to be<br/>"
               "changed if/when this protocol is saved.<br/><br/>"
               "If you plan to make changes to this protocol,<br/>"
               "it is suggested that you change it's name<br/>"
               "(description text) at this time." );
      QMessageBox::information( this,
                                tr( "Duplicate Run Protocol Name" ),
                                msg );
   }
   else
   {
      currProto->protoname = protoname;
      currProto->protoGUID = US_Util::new_guid(); // Get a new GUID
   }

}

// Capture selected project information
void US_ExperGuiGeneral::project_info( US_Project& project )
{
DbgLv(1) << "projinfo: proj.desc" << project.projectDesc;
DbgLv(1) << "projinfo: proj.guid" << project.projectGUID;

   le_project->setText( project.projectDesc );
   currProto->project   = project.projectDesc;

   //ALEXEY
   currProto->projectID   = project.projectID;

   check_runname();
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
     if ( !(acp_list[ ii ].name).contains("1-channel") )  //ALEXEY: remove 1-channel Simulaiton centerpiece from the list
       cp_names << acp_list[ ii ].name;
   }
}


// Panel for Lab/Rotor parameters
US_ExperGuiRotor::US_ExperGuiRotor( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
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


   QLabel*      lb_optima_banner    = us_banner( tr( "Select Optima Machine, Operator and Experiment Type " ) );
   QLabel*      lb_instrument = us_label( tr( "Instrument:" ) );
   //le_instrument = us_lineedit(   "", 1, true );
                cb_optima           = new QComboBox( this );
   QLabel*      lb_optima_connected = us_label( tr( "Connection Status: " ) );
                le_optima_connected = us_lineedit( "", 0, true );

   QLabel*      lb_operator   = us_label( tr( "Select Operator:" ) );
                cb_operator   = new QComboBox( this );

   QLabel*      lb_exptype    = us_label( tr( "Experiment Type:" ) );
                cb_exptype    = new QComboBox( this );

   int row     = 0;
   genL->addWidget( lb_lab,          row,   0, 1, 1 );
   genL->addWidget( cb_lab,          row++, 1, 1, 1 );
   genL->addWidget( lb_rotor,        row,   0, 1, 1 );
   genL->addWidget( cb_rotor,        row,   1, 1, 1 );
   genL->addWidget( lb_calibr,       row,   2, 1, 1 );
   genL->addWidget( cb_calibr,       row++, 3, 1, 1 );
   genL->addItem  ( spacer1,         row++, 0, 1, 4 );
   genL->addWidget( pb_advrotor,     row++, 0, 1, 4 );

   row++;

   genL->addItem  ( spacer1,         row++, 0, 1, 4 );
   genL->addWidget( lb_optima_banner,row++, 0, 1, 4 );


   genL->addWidget( lb_instrument,         row,     0, 1, 1 );
   genL->addWidget( cb_optima,             row,     1, 1, 1 );
   genL->addWidget( lb_optima_connected,   row,     2, 1, 1 );
   genL->addWidget( le_optima_connected,   row++,   3, 1, 1 );

   genL->addWidget( lb_operator,        row,     0, 1, 1 );
   genL->addWidget( cb_operator,        row++,   1, 1, 1 );

   genL->addWidget( lb_exptype,         row,     0, 1, 1 );
   genL->addWidget( cb_exptype,         row++,   1, 1, 1 );

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

   first_time_init = true;
   curr_rotor      = 0;
   changeLab( curr_rotor );
   savePanel();
   changed         = false;
   message_instr_shown = false;

   initPanel();

   first_time_init = false;
   //currentOperator_index = 0;
}

// Slot for change in Lab selection
void US_ExperGuiRotor::setFirstLab( void )
{
  changeLab( 0 );
  savePanel();
}

// Slot for change in Lab selection
void US_ExperGuiRotor::changeLab( int ndx )
{
DbgLv(1) << "EGR:chgLab  ndx" << ndx << "rotorID" << rpRotor->rotID
 << mainw->currProto.rpRotor.rotID;
   changed             = true;
   cb_lab->setCurrentIndex( ndx );
   QString clab        = cb_lab->currentText();
   int labID           = clab.section( ":", 0, 0 ).toInt();
   QString descr       = clab.section( ":", 1, 1 ).simplified();
DbgLv(1) << "EGR: chgLab labID desc" << labID << descr;
   arotors.clear();
   rotors .clear();

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
      if ( rotors[ ii ].name.contains("Simulation", Qt::CaseSensitive) )   //ALEXEY do not include 'Simulation' rotor
         continue;

      sl_rotors << QString::number( rotors[ ii ].ID )
                 + ": " + rotors[ ii ].name;
   }

   QString crot        = cb_rotor->currentText();
DbgLv(1) << "EGR: chgLab  crot" << crot;
   cb_rotor->clear();
   cb_rotor->addItems( sl_rotors );
   US_RunProtocol::RunProtoRotor* rpcRotor = rpRotor;
   if ( rpRotor->rotID < 1 )
   {
      rpcRotor = &(mainw->loadProto.rpRotor);
   }
DbgLv(1) << "EGR: chgLab  sl_rotors count" << sl_rotors.count();
   QString rot_ent     = QString::number( rpcRotor->rotID ) + ": " + rpcRotor->rotor;
   int rndx            = qMax( sl_rotors.indexOf( rot_ent ), 0 );
DbgLv(1) << "EGR: chgLab   rot_ent" << rot_ent << "rndx" << rndx;
   changeRotor( rndx );

   //ALEXEY identify instruments & operators, fill Gui elements
   US_Rotor::Lab lab_selected;
   lab_selected.readDB( labID, dbP );
   instruments = lab_selected.instruments;

   //Instruments
   sl_optimas.clear();
   foreach ( US_Rotor::Instrument instrument, instruments )
   {
      if(instrument.name.contains("Optima"))
      {
         if ( !mainw->automode )
         {
            // Regular us_experiment: populate ALL instruments
            sl_optimas << QString::number( instrument.ID ) + ": " + instrument.name;

qDebug() << "ASSIGNING INSTRUMENTS: " << instrument.name;
         }
         else
         {
            //ALEXEY: passed from autoflow: Exclude instruments in USE
            bool optima_in_use = false;
            for (int ll = 0; ll < mainw->instruments_in_use.size(); ll++)
            {
               if ( instrument.name == mainw->instruments_in_use[ll] )
               {
                  optima_in_use = true;
                  break;
               }
            }

            if ( !optima_in_use)
            {
               sl_optimas << QString::number( instrument.ID ) + ": " + instrument.name;
               qDebug() << "ASSIGNING FREE INSTRUMENTS: " << instrument.name;
            }
         }
      }
   }

   cb_optima->clear();
   cb_optima->addItems( sl_optimas );

   connect( cb_optima,    SIGNAL( activated      ( int ) ),
            this,         SLOT  ( changeOptima   ( int ) ) );

   changeOptima(0);

   //ExpType
   experimentTypes.clear();
   cb_exptype->clear();
   experimentTypes << "Velocity"
                   << "Equilibrium"
                   << "Diffusion"
                   << "Buoyancy"
                   << "Calibration"
                   << "Other";

   cb_exptype->addItems( experimentTypes );
   changeExpType( 0 );
}


// Slot for select in Optima in use
void US_ExperGuiRotor::changeOptima( int ndx )
{
   cb_optima->setCurrentIndex( ndx );
   QString coptima     = cb_optima->currentText();
   QString descr       = coptima.section( ":", 1, 1 ).simplified();

   for ( int ii = 0; ii < instruments.size(); ii++ )
   {
      QString name = instruments[ii].name.trimmed();
      if ( name == descr )
      {
         currentInstrument = instruments[ii];

         mainw->currentInstrument[ "ID" ]              = QString::number( instruments[ii].ID );
         mainw->currentInstrument[ "serial" ]          = instruments[ii].serial;
         mainw->currentInstrument[ "name" ]            = instruments[ii].name;
         mainw->currentInstrument[ "optimaHost" ]      = instruments[ii].optimaHost;
         mainw->currentInstrument[ "optimaPort" ]      = QString::number( instruments[ii].optimaPort );
         mainw->currentInstrument[ "optimaDBname" ]    = instruments[ii].optimaDBname;
         mainw->currentInstrument[ "optimaDBusername" ] = instruments[ii].optimaDBusername;
         mainw->currentInstrument[ "optimaDBpassw" ]    = instruments[ii].optimaDBpassw;
         mainw->currentInstrument[ "opsys1" ]          = instruments[ii].os1;
         mainw->currentInstrument[ "opsys2" ]          = instruments[ii].os2;
         mainw->currentInstrument[ "opsys3" ]          = instruments[ii].os3;
      }
   }
   //Operators
   sl_operators_copy = sl_operators;
   currentOperator = cb_operator->currentText();
   
   sl_operators.clear();
   QList< US_Rotor::Operator > operators = currentInstrument.operators;
   foreach ( US_Rotor::Operator oper, operators )
   {
      qDebug() << "Operator: " << oper.lname;
      sl_operators << QString::number( oper.ID )
                      + ": " + oper.fname + " " + oper.lname;
   }
   cb_operator->clear();
   cb_operator->addItems( sl_operators );

   //compare old and new operator lists
   if ( sl_operators.contains( currentOperator ) )
     {
       qDebug() << "New operator list contains: " <<  currentOperator;
       int currentOperator_index = cb_operator->findText( currentOperator );
       changeOperator( currentOperator_index );
     }
   else
     changeOperator(0);
      
   // if ( !mainw->connection_for_instrument.contains( mainw->currentInstrument[ "name" ] ) )
   //   test_optima_connection();

   test_optima_connection();
}

// Slot for change in ExpType selection
void US_ExperGuiRotor::changeExpType( int ndx )
{
  //changed             = true;
  cb_exptype->setCurrentIndex( ndx );
}

// Slot for change in Operator selection
void US_ExperGuiRotor::changeOperator( int ndx )
{
  //changed             = true;
  cb_operator->setCurrentIndex( ndx );

  //currentOperator_index = ndx;

  //qDebug() << "Current Operator index: " << currentOperator_index;
}

// Slot for change in Rotor selection
void US_ExperGuiRotor::changeRotor( int ndx )
{
DbgLv(1) << "EGR:chgRotor  New index: ndx " << ndx << ", prev. index: " << curr_rotor;
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
//*DEBUG*
DbgLv(1) << "EGR: chgRotor calibs count" << calibs.count();
for (int ii=0; ii<calibs.count(); ii++) {
 if ( ii < 4 || (ii+4 > calibs.count()) )
  DbgLv(1) << "EGR:  ii" << ii << "calibs ID,rotID,GUID"
   << calibs[ii].ID << calibs[ii].rotorID << calibs[ii].GUID; }
//*DEBUG*
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

   QString calib_ent   = QString::number( rpRotor->calID ) + ": " + rpRotor->calibration;
   int cndx            = qMax( sl_calibs.indexOf( calib_ent ), 0 );
   cb_calibr->setCurrentIndex( cndx );

   if ( !first_time_init  && !mainw->global_reset  &&  changed  &&  ( curr_rotor != ndx ) )
   {
      QMessageBox::information( this,
         tr( "NOTE:  Rotor Changed" ),
         tr( "Cells and all subsequent tabs will be reset upon initialization."));
   }

   curr_rotor          = ndx;
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

// Function to return the abstract rotor for the current rotor
US_Rotor::AbstractRotor* US_ExperGuiRotor::abstractRotor( const QString rotor )
{
DbgLv(1) << "EGR:  absR: rotor" << rotor;
   int arID       = -1;
   for ( int ii = 0; ii < rotors.count(); ii++ )
   {  // Search for a match to the rotor name
DbgLv(1) << "EGR:  absR:   ii" << ii << "rname" << rotors[ii].name;
      if ( rotors[ ii ].name.trimmed() == rotor )
      {  // Match found:  break with abstractRotor ID value

         qDebug() << "ABSTROT: ROTOR INFO !!!!: " << rotors[ ii ].name << ", "
          << rotors[ ii ].abstractRotorID <<  ", " << rotors[ ii ].labID << ", "
          << rotors[ ii ].serialNumber;
         arID           = rotors[ ii ].abstractRotorID;
         break;
      }
   }
DbgLv(1) << "EGR:  arID" << arID << "arcount" << arotors.count();
   for ( int ii = 0; ii < arotors.count(); ii++ )
   {  // Search for a match to the abstract rotor ID
DbgLv(1) << "EGR:  absR:   ii" << ii << "rID" << arotors[ii].ID;
      if ( arotors[ ii ].ID == arID )
      {  // Match found:  return with pointer to abstract rotor object
         return ( &arotors[ ii ] );
      }
   }

   return NULL;
}

// Panel for Speed step parameters
US_ExperGuiSpeeds::US_ExperGuiSpeeds( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
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
   QLabel*  lb_count   = us_label( tr( "Number of Speed Profiles:" ) );
   QLabel*  lb_speed   = us_label( tr( "Rotor Speed (rpm):" ) );
   QLabel*  lb_accel   = us_label( tr( "Acceleration (rpm/sec):" ) );

   // QLabel*  lb_durat   = us_label( tr( "Duration of Experiment (days[D] hh[H] mm[M] ss[S]):" ) );
   // QLabel*  lb_delay   = us_label( tr( "Delay to First Scan (days[D] hh[H] mm[M] ss[S]):" ) );
   //QLabel*  lb_durat   = us_label( tr( "Duration of Experiment (hh[H] mm[M]):" ) );
   QLabel*  lb_durat   = us_label( tr( "Active Scanning Time (hh[H] mm[M]):" ) );

   QLabel*  lb_delay       = us_label( tr( "Delay to First Scan (hh[H] mm[M]):" ) );
   QLabel*  lb_delay_int   = us_label( tr( "Delay to First Scan (hh[H] mm[M]):" ) );


   QLabel*  lb_delay_stage    = us_label( tr( "Stage Delay (hh[H] mm[M]):" ) );

   QLabel*  lb_total_time     = us_label( tr( "Total Time (without equilibration):" ) );
   le_total_time              = us_lineedit( "", 0, true  );
   
   QLayout* lo_delay_stage_sync  = us_checkbox( tr( "Synchronize Stage Delay with the 1st Speed Profile: " ), ck_sync_delay, false );

   connect( ck_sync_delay, SIGNAL( toggled     ( bool ) ),
               this,       SLOT  ( syncdelayChecked( bool ) ) );

   
   //ALEXEY: do not create these checkboxes for now
   // QLayout* lo_endoff  = us_checkbox( tr( "Spin down centrifuge at job end" ),
   //                                    ck_endoff, true );
   // QLayout* lo_radcal  = us_checkbox( tr( "Perform radial calibration" ),
   //                                    ck_radcal, false );

   // QLabel*  lb_scnint  = us_label( tr( "Scan Interval (days[D] hh[H] mm[M] ss[S];"
   //                                     " 0 => fast-as-possible)" ) );
   QLabel*  lb_scnint  = us_label( tr( "Scan Interval (hh[H] mm[M] ss[S]:" ) );
   QLabel*  lb_scnint_int  = us_label( tr( "Scan Interval (hh[H] mm[M] ss[S]:" ) );

   // ComboBox, counters, time-edits, spinbox
   sb_count            = us_spinbox();
   sb_count            ->setMinimum(1);  //ALEXEY BUG FIX

   if ( mainw->automode && !mainw->usmode )
     sb_count            ->setEnabled( false );
   
   cb_prof             = new QComboBox( this );
   ct_speed            = us_counter( 2, 1000,  80000, 100 );
   ct_accel            = us_counter( 2,   50,   1000,  50 );
    // QHBoxLayout* lo_durat                                             // ALEXEY
    //                       = us_timeedit( tm_durat,  0, &sb_durat  );
    // QHBoxLayout* lo_delay
    //                       = us_timeedit( tm_delay,  0, &sb_delay  );
    // QHBoxLayout* lo_scnint
    //                       = us_timeedit( tm_scnint, 0, &sb_scnint );



   QHBoxLayout* lo_duratlay        = us_ddhhmmsslay( 0, 0,0,0,1, &sb_durat_dd, &sb_durat_hh, &sb_durat_mm,  &sb_durat_ss ); // ALEXEY 0 - visible, 1 - hidden
   QHBoxLayout* lo_delaylay_stage  = us_ddhhmmsslay( 0, 0,0,0,1, &sb_delay_st_dd, &sb_delay_st_hh, &sb_delay_st_mm,  &sb_delay_st_ss );
   sb_delay_st_dd->setEnabled(false);

   //UV_vis
   QHBoxLayout* lo_delaylay        = us_ddhhmmsslay( 0, 0,0,0,1, &sb_delay_dd, &sb_delay_hh, &sb_delay_mm,  &sb_delay_ss );
   sb_delay_dd->setEnabled(false);
   sb_delay_hh->setEnabled(false);
   sb_delay_mm->setEnabled(false);

   //Interference
   QHBoxLayout* lo_delaylay_int    = us_ddhhmmsslay( 0, 0,0,0,1, &sb_delay_int_dd, &sb_delay_int_hh, &sb_delay_int_mm,  &sb_delay_int_ss );
   sb_delay_int_dd->setEnabled(false);
   sb_delay_int_hh->setEnabled(false);
   sb_delay_int_mm->setEnabled(false);


   //UV-vis
   QHBoxLayout* lo_scnintlay               = us_ddhhmmsslay( 0, 0,0,0,0, &sb_scnint_dd, &sb_scnint_hh, &sb_scnint_mm,  &sb_scnint_ss );
   sb_scnint_dd->setEnabled(false);

   //Interference
   QHBoxLayout* lo_scnintlay_int           = us_ddhhmmsslay( 0, 0,0,0,0, &sb_scnint_int_dd, &sb_scnint_int_hh, &sb_scnint_int_mm,  &sb_scnint_int_ss );
   sb_scnint_int_dd->setEnabled(false);

   le_maxrpm           = us_lineedit( tr( "Maximum speed for AN50 rotor:"
                                          "  50000 rpm" ), 0, true );

   // Default values
   nspeed              = 1;
   curssx              = 0;
   double df_speed     = rpSpeed->ssteps[ 0 ].speed;
   double df_accel     = rpSpeed->ssteps[ 0 ].accel;
   double df_duratm    = rpSpeed->ssteps[ 0 ].duration;
   double df_delatm_stage    = rpSpeed->ssteps[ 0 ].delay_stage;
   double df_total_time = rpSpeed->ssteps[ 0 ].total_time;
   
   //interference
   double df_delatm_int    = rpSpeed->ssteps[ 0 ].delay_int;
   double df_scint_int     = rpSpeed->ssteps[ 0 ].scanintv_int;
   double df_scint_int_min = rpSpeed->ssteps[ 0 ].scanintv_int_min;
   //Uv-vis
   double df_delatm    = rpSpeed->ssteps[ 0 ].delay;
   double df_scint     = rpSpeed->ssteps[ 0 ].scanintv; //ALEXEY read default scanint in secs corresponding to default RPM
   double df_scint_min = rpSpeed->ssteps[ 0 ].scanintv_min;

   
   QList< int > dhms_dur;
   QList< int > dhms_dly_stage;
   //Uv-vis
   QList< int > dhms_dly;
   QList< int > hms_scint;
   //interference
   QList< int > dhms_dly_int;
   QList< int > hms_scint_int;

   double df_duratm_c       = df_duratm;
   double df_delatm_stage_c = df_delatm_stage;
   double df_delatm_c       = df_delatm;
   double df_scint_c        = df_scint;
   double df_delatm_int_c   = df_delatm_int;
   double df_scint_int_c    = df_scint_int;
   
   US_RunProtocol::timeToList( df_duratm_c, dhms_dur );
   US_RunProtocol::timeToList( df_delatm_stage_c, dhms_dly_stage );
   //Uv-vis
   US_RunProtocol::timeToList( df_delatm_c, dhms_dly );
   US_RunProtocol::timeToList( df_scint_c,  hms_scint );
   //interference
   US_RunProtocol::timeToList( df_delatm_int_c, dhms_dly_int );
   US_RunProtocol::timeToList( df_scint_int_c,  hms_scint_int );

DbgLv(1) << "EGSp: df_duratm" << df_duratm;
DbgLv(1) << "EGSp:   def  d h m s " << dhms_dur;
DbgLv(1) << "EGSp: df_delatm" << df_delatm;
DbgLv(1) << "EGSp:   def  d h m s " << dhms_dly;
   profdesc.resize( nspeed );  // Speed profile descriptions
   ssvals  .resize( nspeed );  // Speed step values

   //Uv-vis
   scanint_ss_min.resize( nspeed );
   scanint_mm_min.resize( nspeed );
   scanint_hh_min.resize( nspeed );
   delay_mm_min  .resize( nspeed );
   //interference
   scanint_ss_int_min.resize( nspeed );
   scanint_mm_int_min.resize( nspeed );
   scanint_hh_int_min.resize( nspeed );
   delay_mm_int_min  .resize( nspeed );

   ssvals[ 0 ][ "speed" ]    = df_speed;  // Speed default
   ssvals[ 0 ][ "accel" ]    = df_accel;  // Acceleration default
   ssvals[ 0 ][ "duration" ] = df_duratm; // Duration in seconds default (5h 30m)
   ssvals[ 0 ][ "delay_stage" ]  = df_delatm_stage; // Delay of the stage in seconds
   ssvals[ 0 ][ "total_time" ]  = df_total_time; // Delay of the stage in seconds
   

   //Uv-vis
   ssvals[ 0 ][ "delay" ]    = df_delatm; // Delay to 1st scan in seconds default (2m 30s) DUE to acceleration
   //interference
   ssvals[ 0 ][ "delay_int" ]    = df_delatm_int;

  
   //Uv-vis
   ssvals[ 0 ][ "scanintv" ]     = df_scint;  //ALEXEY
   ssvals[ 0 ][ "scanintv_min" ] = df_scint_min;  //ALEXEY
   //interference
   ssvals[ 0 ][ "scanintv_int" ]     = df_scint_int;  //ALEXEY
   ssvals[ 0 ][ "scanintv_min_int" ] = df_scint_int_min;  //ALEXEY


   // Set up counters and profile description
   ct_speed ->setSingleStep( 100 );
   ct_accel ->setSingleStep(  50 );

   sb_count ->setValue( nspeed    );
   ct_speed ->setValue( df_speed  );
   ct_accel ->setValue( df_accel  );
DbgLv(1) << "EGSp: init sb/de components";
   // sb_durat ->setValue( (int)dhms_dur[ 0 ] );   // sb_durat_dd ::ALEXEY USE dhms_dur[0..3] for duration: [0]=>day, [1]=>hh, [2]=>mm, [3]=>ss
   // tm_durat ->setTime ( QTime( dhms_dur[ 1 ],   // sb_durat_hh
   //                             dhms_dur[ 2 ],   // sb_durat_mm
   //                             dhms_dur[ 3 ] ) ); //sb_durat_ss
   sb_durat_dd ->setValue( (int)dhms_dur[ 0 ] );
   sb_durat_hh ->setValue( (int)dhms_dur[ 1 ] );
   sb_durat_mm ->setValue( (int)dhms_dur[ 2 ] );
   sb_durat_ss ->setValue( (int)dhms_dur[ 3 ] );

   // sb_delay ->setValue( (int)dhms_dly[ 0 ] );   //ALEXEY USE dhms_dly[0..3] for duration: [0]=>day, [1]=>hh, [2]=>mm, [3]=>ss
   // tm_delay ->setTime ( QTime( dhms_dly[ 1 ],
   //                             dhms_dly[ 2 ],
   //                             dhms_dly[ 3 ] ) );

   //Uv-vis
   sb_delay_dd ->setValue( (int)dhms_dly[ 0 ] );
   sb_delay_hh ->setValue( (int)dhms_dly[ 1 ] );
   sb_delay_mm ->setValue( (int)dhms_dly[ 2 ] );
   sb_delay_ss ->setValue( (int)dhms_dly[ 3 ] );

   //interference
   sb_delay_int_dd ->setValue( (int)dhms_dly_int[ 0 ] );
   sb_delay_int_hh ->setValue( (int)dhms_dly_int[ 1 ] );
   sb_delay_int_mm ->setValue( (int)dhms_dly_int[ 2 ] );
   sb_delay_int_ss ->setValue( (int)dhms_dly_int[ 3 ] );


   //delay Stage
   sb_delay_st_dd ->setValue( (int)dhms_dly_stage[ 0 ] );
   sb_delay_st_hh ->setValue( (int)dhms_dly_stage[ 1 ] );
   sb_delay_st_mm ->setValue( (int)dhms_dly_stage[ 2 ] );
   sb_delay_st_ss ->setValue( (int)dhms_dly_stage[ 3 ] );

   //Total Time
   QList< int > hms_tot;
   US_RunProtocol::timeToList( df_total_time, hms_tot );
   QString tot_str = QString::number( hms_tot[ 0 ] ) + "d " + QString::number( hms_tot[ 1 ] ) + "h " + QString::number( hms_tot[ 2 ] ) + "m ";
   le_total_time->setText( tot_str );

   
   // sb_scnint->setValue( 0 );                    //ALEXEY
   // tm_scnint->setTime ( QTime( 0, 0 ) );

   //Uv-vis
   sb_scnint_hh ->setMinimum( (int)hms_scint[ 1 ] );
   sb_scnint_mm ->setMinimum( (int)hms_scint[ 2 ] );
   sb_scnint_ss ->setMinimum( (int)hms_scint[ 3 ] );

   sb_scnint_dd ->setValue( 0 );
   sb_scnint_hh ->setValue( (int)hms_scint[ 1 ] );
   sb_scnint_mm ->setValue( (int)hms_scint[ 2 ] );
   sb_scnint_ss ->setValue( (int)hms_scint[ 3 ] );

   scanint_ss_min[0] = (int)hms_scint[ 3 ];
   scanint_mm_min[0] = (int)hms_scint[ 2 ];
   scanint_hh_min[0] = (int)hms_scint[ 1 ];

   //interference
   sb_scnint_int_hh ->setMinimum( (int)hms_scint_int[ 1 ] );
   sb_scnint_int_mm ->setMinimum( (int)hms_scint_int[ 2 ] );
   sb_scnint_int_ss ->setMinimum( (int)hms_scint_int[ 3 ] );

   sb_scnint_int_dd ->setValue( 0 );
   sb_scnint_int_hh ->setValue( (int)hms_scint_int[ 1 ] );
   sb_scnint_int_mm ->setValue( (int)hms_scint_int[ 2 ] );
   sb_scnint_int_ss ->setValue( (int)hms_scint_int[ 3 ] );

   scanint_ss_int_min[0] = (int)hms_scint_int[ 3 ];
   scanint_mm_int_min[0] = (int)hms_scint_int[ 2 ];
   scanint_hh_int_min[0] = (int)hms_scint_int[ 1 ];

DbgLv(1) << "EGSp: DONE init sb/de components";

   // Speed profile 1 description;
   //   e.g., "Speed Profile 1: 30000 rpm for 5 hr 30 min"
   cb_prof->addItem( speedp_description( 0 ) );

   // Adjust counter sizes
   ct_speed->adjustSize();
   ct_accel->adjustSize();

   // Create main layout rows
   int row = 0;
   genL->addWidget( lb_count,   row,    0, 1,  6 );
   genL->addWidget( sb_count,   row++,  6, 1,  2 );
DbgLv(1) << "EGSp: addWidg/Layo row" << row;
   genL->addWidget( cb_prof,    row++,  0, 1,  8 );
DbgLv(1) << "EGSp: addWidg/Layo row" << row;
   genL->addWidget( lb_speed,   row,    0, 1,  5 );
   genL->addWidget( ct_speed,   row++,  5, 1,  3 );
DbgLv(1) << "EGSp: addWidg/Layo row" << row;
   genL->addWidget( lb_accel,   row,    0, 1,  5 );
   genL->addWidget( ct_accel,   row++,  5, 1,  3 );
DbgLv(1) << "EGSp: addWidg/Layo row" << row;
DbgLv(1) << "EGSp: addWidg/Layo AA";

  genL->addWidget( lb_durat,   row,    0, 1,  5);
DbgLv(1) << "EGSp: addWidg/Layo BB";
//genL->addLayout( lo_durat,   row++,  6, 1,  2);
  genL->addLayout( lo_duratlay,   row++,  5, 1,  1);


  genL->addWidget( lb_delay_stage,    row,    0, 1,  5 );
  genL->addLayout( lo_delaylay_stage, row++,  5, 1,  1 );
  
  genL->addWidget( lb_total_time,    row,    0, 1,  5 );
  genL->addWidget( le_total_time,    row++,  5, 1,  1 );

  genL->addLayout( lo_delay_stage_sync, row++,0, 1,  5 );


  //Scan # estimator:
  QLabel* lb_scan_estimator    = us_banner( tr( "Scan Number Estimator:" ) );
  QLabel* lb_wvl_per_cell = us_label(tr( "Sum of all wavelengths (from all cells) to be scanned:" ));
  sb_wvl_per_cell = us_spinbox();
  sb_wvl_per_cell->setRange(1, 100);
  connect( sb_wvl_per_cell,  SIGNAL( valueChanged     ( int ) ),
	   this,             SLOT  ( ssChgWvlPerCell  ( int ) ) );

  QLabel* lb_scans_per_cell = us_label(tr( "Total number of scans per wavelength, per cell:" ));
  le_scans_per_cell = us_lineedit( "", 0, true  ); 

  //set default
  int tot_wvl = (int)sb_wvl_per_cell->value();
  double duration_sec    = df_duratm;
  double scanint_sec     = df_scint;
  double scanint_sec_min = df_scint_min;
  int scancount = 0;
  qDebug() << "Setting DEFAULTs for scan # estimator: duration, scanInt, scanInt_min -- " << duration_sec << scanint_sec << scanint_sec_min;
  //ALEXEY: use this algorithm to calculate scanCount && scanInt
  if ( scanint_sec > scanint_sec_min*tot_wvl )
    scancount     = int( duration_sec / scanint_sec );
  else
    scancount    = int( duration_sec / (scanint_sec_min * tot_wvl) );
  le_scans_per_cell -> setText( QString::number( scancount ));
  
  genL->addWidget( lb_scan_estimator,  row++,    0, 1,  8 );
  genL->addWidget( lb_wvl_per_cell,    row,    0, 1,  5 );
  genL->addWidget( sb_wvl_per_cell,    row++,  5, 1,  1 );
  genL->addWidget( lb_scans_per_cell,  row,    0, 1,  5 );
  genL->addWidget( le_scans_per_cell,  row++,  5, 1,  1 );
  
  //UV-vis.
  QLabel* lb_uvvis    = us_banner( tr( "UV-Visible (total):" ) );
  genL->addWidget( lb_uvvis,     row++,    0, 1,  8 );
  genL->addWidget( lb_delay,   row,    0, 1,  5 );
  genL->addLayout( lo_delaylay,   row++,  5, 1,  1 );
  genL->addWidget( lb_scnint,  row,    0, 1,  5 );
  genL->addLayout( lo_scnintlay,  row++,  5, 1,  3 );


  // Interference
  QLabel* lb_int    = us_banner( tr( "Interference (per cell):" ) );
  genL->addWidget( lb_int,     row++,    0, 1,  8 );
  genL->addWidget( lb_delay_int,   row,    0, 1,  5 );
  genL->addLayout( lo_delaylay_int,   row++,  5, 1,  1 );
  genL->addWidget( lb_scnint_int,     row,    0, 1,  5 );
  genL->addLayout( lo_scnintlay_int,  row++,  5, 1,  3 );


DbgLv(1) << "EGSp: addWidg/Layo GG";
   genL->addWidget( le_maxrpm,  row++,  0, 1,  4 );
DbgLv(1) << "EGSp: addWidg/Layo HH";
// genL->addLayout( lo_endoff,  row,    0, 1,  4 );
DbgLv(1) << "EGSp: addWidg/Layo II";
// genL->addLayout( lo_radcal,  row++,  4, 1,  4 );

   
   
   genL->setColumnStretch(  0, 4 );
   genL->setColumnStretch(  1, 4 );
   genL->setColumnStretch(  2, 4 );
   genL->setColumnStretch(  3, 4 );
   genL->setColumnStretch(  4, 4 );
   genL->setColumnStretch(  5, 4 );
   genL->setColumnStretch(  6, 1 );
   genL->setColumnStretch(  7, 1 );

   // Connect signals and slots
   connect( sb_count,  SIGNAL( valueChanged  ( int )  ),
            this,      SLOT  ( ssChangeCount ( int )  ) );
   connect( cb_prof,   SIGNAL( activated     ( int    ) ),
            this,      SLOT  ( ssChangeProfx ( int    ) ) );
   connect( ct_speed,  SIGNAL( valueChanged  ( double ) ),
            this,      SLOT  ( ssChangeSpeed ( double ) ) );
   connect( ct_accel,  SIGNAL( valueChanged  ( double ) ),
            this,      SLOT  ( ssChangeAccel ( double ) ) );

   // connect( sb_durat,  SIGNAL( valueChanged   ( int ) ),               \\ALEXEY
   //          this,      SLOT  ( ssChgDuratDay  ( int ) ) );
   // connect( tm_durat,  SIGNAL( timeChanged    ( const QTime& ) ),
   //          this,      SLOT  ( ssChgDuratTime ( const QTime& ) ) );
   // connect( sb_delay,  SIGNAL( valueChanged   ( int ) ),
   //          this,      SLOT  ( ssChgDelayDay  ( int ) ) );
   // connect( tm_delay,  SIGNAL( timeChanged    ( const QTime& ) ),
   //          this,      SLOT  ( ssChgDelayTime ( const QTime& ) ) );

   connect( sb_durat_dd,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDuratDay  ( int ) ) );
   connect( sb_durat_hh,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDuratTime_hh ( int ) ) );
   connect( sb_durat_mm,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDuratTime_mm ( int ) ) );
   connect( sb_durat_ss,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDuratTime_ss ( int ) ) );

   //Uv-vis
   connect( sb_delay_dd,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDelayDay  ( int ) ) );
   connect( sb_delay_hh,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDelayTime_hh ( int ) ) );
   connect( sb_delay_mm,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDelayTime_mm ( int ) ) );
   connect( sb_delay_ss,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDelayTime_ss ( int ) ) );

   //interference
   connect( sb_delay_int_dd,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDelayDay_int  ( int ) ) );
   connect( sb_delay_int_hh,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDelayTime_int_hh ( int ) ) );
   connect( sb_delay_int_mm,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDelayTime_int_mm ( int ) ) );
   connect( sb_delay_int_ss,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDelayTime_int_ss ( int ) ) );

   //Stage delay
   connect( sb_delay_st_hh,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDelayStageTime_hh ( int ) ) );
   connect( sb_delay_st_mm,  SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgDelayStageTime_mm ( int ) ) );

   //Uv-vis
   connect( sb_scnint_hh, SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgScIntTime_hh ( int ) ) );
   connect( sb_scnint_mm, SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgScIntTime_mm ( int ) ) );
   connect( sb_scnint_ss, SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgScIntTime_ss ( int ) ) );

   //interference
   connect( sb_scnint_int_hh, SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgScIntTime_int_hh ( int ) ) );
   connect( sb_scnint_int_mm, SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgScIntTime_int_mm ( int ) ) );
   connect( sb_scnint_int_ss, SIGNAL( valueChanged   ( int ) ),
            this,         SLOT  ( ssChgScIntTime_int_ss ( int ) ) );


      
DbgLv(1) << "EGSp: addWidg/Layo II";

   // Complete overall layout
   panel->addLayout( genL );
   panel->addStretch();
   adjustSize();

   // Set low delay-minutes based on speed,acceleration,delay-hours
   adjustDelay();

   initPanel();

   //qDebug() << "SCANINT: " << ssvals[ 0 ][ "scanintv" ]  << ", SCANINT FROM rpSpeed: " <<  rpSpeed->ssteps[ 0 ].scanintv;
}

// Return speed profile description string for an indicated step
QString US_ExperGuiSpeeds::speedp_description( int ssx )
{
   // For example: "Speed Profile 1: 30000 rpm for 5 hr 30 min"
   double durtim  = ssvals[ ssx ][ "duration" ]; // Duration total minutes   //ALEXEY - no, in seconds
   //double durhr   = qFloor( durtim / 60.0 );     // Duration hours
   double durhr   = qFloor( durtim / 3600.0 );     // Duration hours          //ALEXEY (seconds/3600)
   // double durmin  = durtim - ( durhr * 60.0 );   // Duration residual minutes
   double durmin  = (durtim / 60.0) - ( durhr * 60.0 );   // Duration residual minutes

   //int    escans  = qRound( durtim * 6.0 );      // Estimated step scans             //ALEXEY: 6 scans per minute ???
   //int    escans  = qRound( (durtim / 60.0) * 6.0 );      // Estimated step scans             //ALEXEY: 6 scans per minute ???

   //Uv-vis
   //double scaninterval     =  ssvals[ ssx ][ "scanintv" ];  //ALEXEY: ssval[]["scanintv"] is set (in secs)  according to table: RPM vs scanint
   //int escans = qRound( durtim / scaninterval );
   double scaninterval_min = ssvals[ ssx ][ "scanintv_min" ]; //Use minimum scanint instead (always)
   int escans = qRound( durtim / scaninterval_min );
   //interference
   double scaninterval_int =  ssvals[ ssx ][ "scanintv_int" ];
   int escans_int = qRound( durtim / scaninterval_int );


   int tscans  = 0;
   int tscans_int  = 0;
   for ( int ii = 0; ii < ssvals.size(); ii++ )
     {
       tscans        += qRound( ssvals[ ii ][ "duration" ] / ssvals[ ii ][ "scanintv" ] );  //ALEXEY bug fixed
       tscans_int    += qRound( ssvals[ ii ][ "duration" ] / ssvals[ ii ][ "scanintv_int" ] );
     }
   qDebug() << "SCAN INT: " << scaninterval_min << ", # Scans: " << tscans;


   // return tr( "Speed Profile %1 :    %2 rpm for %3 hr %4 min"
   //            "  (%5 estimated scans, %6 run total)" )
   //        .arg( ssx + 1 ).arg( ssvals[ ssx ][ "speed" ] )
   //        .arg( durhr ).arg( durmin ).arg( escans ).arg( tscans );

   return tr( "Speed Profile %1 :    %2 rpm for %3 hr %4 min"
              "  Maximum # of scans possible: %5 (UV-vis, total), %6 (Interference, per cell)" )
          .arg( ssx + 1 ).arg( ssvals[ ssx ][ "speed" ] )
      .arg( durhr ).arg( durmin ).arg( escans ).arg( escans_int );
}

// Slot for change in speed-step count
void US_ExperGuiSpeeds::ssChangeCount( int val )
{
   changed          = true;
   int new_nsp      = val;
DbgLv(1) << "EGSp: chgKnt: nsp nnsp" << nspeed << new_nsp;

  double speedmax  = sibDValue( "rotor",   "maxrpm" );
  if ( new_nsp > nspeed )
   {  // Number of speed steps increases
      profdesc.resize( new_nsp );
      ssvals  .resize( new_nsp );

      //Uv-vis
      scanint_ss_min.resize( new_nsp );
      scanint_mm_min.resize( new_nsp );
      scanint_hh_min.resize( new_nsp );
      delay_mm_min  .resize( new_nsp );
      //interference
      scanint_ss_int_min.resize( new_nsp );
      scanint_mm_int_min.resize( new_nsp );
      scanint_hh_int_min.resize( new_nsp );
      delay_mm_int_min  .resize( new_nsp );

      int kk           = nspeed - 1;
      double ssspeed   = ssvals[ kk ][ "speed" ];
      double ssaccel   = ssvals[ kk ][ "accel" ];

      //double ssscint   = ssvals[ kk ][ "scanintv" ];     // ALEXEY

      double ssdurtim  = ssvals[ kk ][ "duration" ];        //ALEXEY - minutes ??? should be seconds ...
      // double ssdurhr   = qFloor( ssdurtim / 60.0 );
      // double ssdurmin  = ssdurtim - ( ssdurhr * 60.0 );
      double ssdurmin  = qFloor( ssdurtim / 60.0 );         //ALEXEY redone in secons for duration
      double ssdurhr   = qFloor( ssdurmin / 60.0 );
      ssdurmin        -= ( ssdurhr * 60.0 );
      double ssdursec  = ssdurtim - ( ssdurhr * 3600.0 )
                                   - ( ssdurmin * 60.0 );
      //Uv-vis
      double ssdlytim  = ssvals[ kk ][ "delay" ];           //ALEXEY - seconds (correct..)
      double ssdlymin  = qFloor( ssdlytim / 60.0 );
      double ssdlyhr   = qFloor( ssdlymin / 60.0 );
      ssdlymin        -= ( ssdlyhr * 60.0 );
      double ssdlysec  = ssdlytim - ( ssdlyhr * 3600.0 )
                                  - ( ssdlymin * 60.0 );
      //interference
      double ssdlytim_int  = ssvals[ kk ][ "delay_int" ];           //ALEXEY - seconds (correct..)
      double ssdlymin_int  = qFloor( ssdlytim_int / 60.0 );
      double ssdlyhr_int   = qFloor( ssdlymin_int / 60.0 );
      ssdlymin_int        -= ( ssdlyhr_int * 60.0 );
      double ssdlysec_int  = ssdlytim_int - ( ssdlyhr_int * 3600.0 )
                                  - ( ssdlymin_int * 60.0 );

      //ssdurtim         = ( ssdurhr * 60.0 ) + ssdurmin;                        //ALEXEY in minutes [duration]
      ssdurtim         = ( ssdurhr * 3600.0 ) + ( ssdurmin * 60.0 ) + ssdursec;  //ALEXEY in seconds [duration]
      //Uv-vis
      ssdlytim         = ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;  //ALEXEY in seconds [delay]
      //interference
      ssdlytim_int     = ( ssdlyhr_int * 3600.0 ) + ( ssdlymin_int * 60.0 ) + ssdlysec_int;  //ALEXEY in seconds [delay]

 DbgLv(1) << "EGSp: chgKnt:  kk" << kk << "spd acc dur dly"
 << ssspeed << ssaccel << ssdurtim << ssdlytim;

      for ( int kkk = nspeed; kkk < new_nsp; kkk++ )
      {  // Fill in new speed step description and values
         ssspeed         += 1000.0;
         if ( ssspeed > speedmax )
            ssspeed          = speedmax;
         ssvals[ kkk ][ "speed"     ] = ssspeed;   // Speed
         ssvals[ kkk ][ "accel"     ] = ssaccel;   // Acceleration
         ssvals[ kkk ][ "duration"  ] = ssdurtim;  // Duration in minutes  // No, in seconds
         // Uv-vis
         ssvals[ kkk ][ "delay"     ] = ssdlytim;  // Delay in seconds
         // Interference
         ssvals[ kkk ][ "delay_int" ] = ssdlytim_int;  // Delay in seconds (interference)

         if ( ck_sync_delay->isChecked() )
         {
            qDebug() << "Syncing stage delays...";
            stageDelay_sync();

         }
         else
         {
            qDebug() << "Syncing stage NOT checked !!!!!!!";
            ssvals[ kkk ][ "delay_stage"    ] = 0.0;
         }

         ssChangeScInt( ssspeed, kkk );  //ALEXEY

         profdesc[ kkk ]             = speedp_description( kkk );
DbgLv(1) << "EGSp: chgKnt:    kkk" << kkk << "pdesc" << profdesc[kkk];

         // Add to the profile description list comboBox
         cb_prof->addItem( profdesc[ kkk ] );
      }

      // Point to the first new speed step
      cb_prof->setCurrentIndex( nspeed );
      ssChangeProfx( nspeed );

      adjustDelay();
   }

   else
   {  // Number of speed steps decreases or remains the same
      profdesc.resize( new_nsp );
      ssvals  .resize( new_nsp );

      //Uv-vis
      scanint_ss_min.resize( new_nsp );
      scanint_mm_min.resize( new_nsp );
      scanint_hh_min.resize( new_nsp );
      delay_mm_min  .resize( new_nsp );
      //interference
      scanint_ss_int_min.resize( new_nsp );
      scanint_mm_int_min.resize( new_nsp );
      scanint_hh_int_min.resize( new_nsp );
      delay_mm_int_min  .resize( new_nsp );

      cb_prof->clear();
      for ( int ii = 0; ii < new_nsp; ii++ )
         cb_prof->addItem( profdesc[ ii ] );
      cb_prof->setCurrentIndex( new_nsp - 1 );
      // Point to the last speed step
      ssChangeProfx( new_nsp - 1 );

      adjustDelay();
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
   double ssdlystagetim  = ssvals[ curssx ][ "delay_stage" ];
   double speedmax  = sibDValue( "rotor",   "maxrpm" );
   //Uv-vis
   double ssdlytim  = ssvals[ curssx ][ "delay" ];
   double scinttim  = ssvals[ curssx ][ "scanintv" ]; // ALEXEY added scaninterval
   //interference
   double ssdlytim_int  = ssvals[ curssx ][ "delay_int" ];
   double scinttim_int  = ssvals[ curssx ][ "scanintv_int" ];


   QList< int > dhms_dur;
   QList< int > dhms_dly_stage;
   //Uv-vis
   QList< int > dhms_dly;
   QList< int > dhms_scint;
   //interference
   QList< int > dhms_dly_int;
   QList< int > dhms_scint_int;

   US_RunProtocol::timeToList( ssdurtim, dhms_dur );
   US_RunProtocol::timeToList( ssdlystagetim, dhms_dly_stage );
   //Uv-vis
   US_RunProtocol::timeToList( ssdlytim, dhms_dly );
   US_RunProtocol::timeToList( scinttim, dhms_scint );
   //interference
   US_RunProtocol::timeToList( ssdlytim_int, dhms_dly_int );
   US_RunProtocol::timeToList( scinttim_int, dhms_scint_int );
DbgLv(1) << "EGSp: chgPfx:   durtim" << ssdurtim << "dhms_dur" << dhms_dur;
DbgLv(1) << "EGSp: chgPfx:    speedmax" << speedmax;
   ct_speed ->setMaximum( speedmax );      // Set speed max based on rotor max
   ct_speed ->setValue( ssspeed  );        // Set counter values
   ct_accel ->setValue( ssaccel  );

   sb_durat_dd ->setValue( (int)dhms_dur[ 0 ] );
   sb_durat_hh ->setValue( (int)dhms_dur[ 1 ] );
   sb_durat_mm ->setValue( (int)dhms_dur[ 2 ] );
   sb_durat_ss ->setValue( (int)dhms_dur[ 3 ] );


   //ALEXEY: before minimums for delay/scanint counters must be set for selected row (ssp -> curssx ), so minimums_hh/ss must be set to be arrays.

   //scanint_ss_min[ curssx ],  scanint_mm_min[ curssx ],  scanint_hh_min[ curssx ];
   // sb_delay_mm ->setMinimum( delay_mm_min[ curssx ] );
   // and so on....

   //Stage delay
   sb_delay_st_dd ->setValue( (int)dhms_dly_stage[ 0 ] );
   sb_delay_st_hh ->setValue( (int)dhms_dly_stage[ 1 ] );
   sb_delay_st_mm ->setValue( (int)dhms_dly_stage[ 2 ] );
   sb_delay_st_ss ->setValue( (int)dhms_dly_stage[ 3 ] );


   //Uv-vis
   sb_delay_mm ->setMinimum( delay_mm_min[ curssx ] );
   sb_delay_dd ->setValue( (int)dhms_dly[ 0 ] );
   sb_delay_hh ->setValue( (int)dhms_dly[ 1 ] );
   sb_delay_mm ->setValue( (int)dhms_dly[ 2 ] );
   sb_delay_ss ->setValue( (int)dhms_dly[ 3 ] );

   sb_scnint_hh ->setMinimum( scanint_hh_min[curssx] );
   sb_scnint_mm ->setMinimum( scanint_mm_min[curssx] );
   sb_scnint_ss ->setMinimum( scanint_ss_min[curssx] );

   sb_scnint_hh ->setValue( (int)dhms_scint[ 1 ] );
   sb_scnint_mm ->setValue( (int)dhms_scint[ 2 ] );
   sb_scnint_ss ->setValue( (int)dhms_scint[ 3 ] );

   //interference
   sb_delay_int_mm ->setMinimum( delay_mm_int_min[ curssx ] );
   sb_delay_int_dd ->setValue( (int)dhms_dly_int[ 0 ] );
   sb_delay_int_hh ->setValue( (int)dhms_dly_int[ 1 ] );
   sb_delay_int_mm ->setValue( (int)dhms_dly_int[ 2 ] );
   sb_delay_int_ss ->setValue( (int)dhms_dly_int[ 3 ] );

   sb_scnint_int_hh ->setMinimum( scanint_hh_int_min[curssx] );
   sb_scnint_int_mm ->setMinimum( scanint_mm_int_min[curssx] );
   sb_scnint_int_ss ->setMinimum( scanint_ss_int_min[curssx] );

   sb_scnint_int_hh ->setValue( (int)dhms_scint_int[ 1 ] );
   sb_scnint_int_mm ->setValue( (int)dhms_scint_int[ 2 ] );
   sb_scnint_int_ss ->setValue( (int)dhms_scint_int[ 3 ] );

   //adjustDelay();        // Important: not needed here as it re-writes delay to default min.
}

// Slot for change in speed value
void US_ExperGuiSpeeds::ssChangeSpeed( double val )
{
DbgLv(1) << "EGSp: chgSpe: val" << val << "ssx" << curssx;
   ssvals  [ curssx ][ "speed" ] = val;  // Set Speed in step vals vector

   ssChangeScInt(val, curssx);

   //Uv-vis
   sb_scnint_hh ->setMinimum( scanint_hh_min[curssx] );
   sb_scnint_mm ->setMinimum( scanint_mm_min[curssx] );
   sb_scnint_ss ->setMinimum( scanint_ss_min[curssx] );

   sb_scnint_hh ->setValue( scanint_hh_min[curssx] );
   sb_scnint_mm ->setValue( scanint_mm_min[curssx] );
   sb_scnint_ss ->setValue( scanint_ss_min[curssx] );

   //interference
   sb_scnint_int_hh ->setMinimum( scanint_hh_int_min[curssx] );
   sb_scnint_int_mm ->setMinimum( scanint_mm_int_min[curssx] );
   sb_scnint_int_ss ->setMinimum( scanint_ss_int_min[curssx] );

   sb_scnint_int_hh ->setValue( scanint_hh_int_min[curssx] );
   sb_scnint_int_mm ->setValue( scanint_mm_int_min[curssx] );
   sb_scnint_int_ss ->setValue( scanint_ss_int_min[curssx] );

   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );

   // Set minimum delay time based on speed change and acceleration
   changed          = false;             // Flag so delay set to minimum
   adjustDelay();                        // Set delay minimum and default
   changed          = true;              // Flag this as a user change
}

//Set ScanInt based on RPM speed entered
void US_ExperGuiSpeeds::ssChangeScInt( double val, int row )
{
  // Fit of the time-interval graph with A0 + A1/X;  [X => val]
  QVector<double> a0(4);
  QVector<double> a1(4);
  a0[0] =  5.3513;    a1[0] = 167308;   // rpm 3000  - 14000:
  a0[1] =  5.66431;   a1[1] = 329920;   // rpm 15000 - 32000:
  a0[2] =  6.16695;   a1[2] = 480761;   // rpm 33000 - 50000:
  a0[3] =  4.44766;   a1[3] = 744728;   // rpm 51000 - 60000:

  double time_scint;
  if (val <= 14999 )
    time_scint = a0[0] + qRound( a1[0]/val );
  if (val >= 15000 and val <= 32999 )
    time_scint = a0[1] + qRound( a1[1]/val );
  if (val >= 33000 and val <= 50999 )
    time_scint = a0[2] + qRound( a1[2]/val );
  if (val >= 51000 and val <= 60000 )
    time_scint = a0[3] + qRound( a1[3]/val );

  ssvals[row]["scanintv"]     = time_scint;
  ssvals[row]["scanintv_min"] = time_scint;

  QList< int > hms_scint;
  US_RunProtocol::timeToList( time_scint, hms_scint );

  // not needed here as these will overwrite current counters when another speed profile is added. Values set in ChProfile function.
  // sb_scnint_hh ->setMinimum( (int)hms_scint[ 1 ] );
  // sb_scnint_mm ->setMinimum( (int)hms_scint[ 2 ] );
  // sb_scnint_ss ->setMinimum( (int)hms_scint[ 3 ] );

  // sb_scnint_hh ->setValue( (int)hms_scint[ 1 ] );
  // sb_scnint_mm ->setValue( (int)hms_scint[ 2 ] );
  // sb_scnint_ss ->setValue( (int)hms_scint[ 3 ] );

  scanint_ss_min[row] = (int)hms_scint[ 3 ];
  scanint_mm_min[row] = (int)hms_scint[ 2 ];
  scanint_hh_min[row] = (int)hms_scint[ 1 ];

  qDebug() << "ScanInt: " << ssvals[row]["scanintv"];

  //interference: do not change default minimum values (from us_run_protocol.ccp SpeedStep.scanintv_int)
  double default_interval_int = 5.0;
  ssvals[row]["scanintv_int"]     = default_interval_int; // rpSpeed->ssteps[0].scanintv_int;
  ssvals[row]["scanintv_min_int"] = default_interval_int; // rpSpeed->ssteps[0].scanintv_int;

  QList< int > hms_scint_int;
  US_RunProtocol::timeToList( default_interval_int, hms_scint_int );

  scanint_ss_int_min[row] = (int)hms_scint_int[ 3 ];
  scanint_mm_int_min[row] = (int)hms_scint_int[ 2 ];
  scanint_hh_int_min[row] = (int)hms_scint_int[ 1 ];
}



// Slot for change in acceleration value
void US_ExperGuiSpeeds::ssChangeAccel( double val )
{
DbgLv(1) << "EGSp: chgAcc: val" << val << "ssx" << curssx;
   ssvals[ curssx ][ "accel" ] = val;  // Set Acceleration in step vals vector

   // Set minimum delay time based on speed and new acceleration
   changed          = false;             // Flag so delay set to minimum
   adjustDelay();                        // Set delay minimum and default
   changed          = true;              // Flag this as a user change
}


/*Scan # Estimator *****************************************************************************/
void US_ExperGuiSpeeds::ssChgWvlPerCell( int val )
{
  int tot_wvl = val;
  double duration_sec    = ssvals[ curssx ][ "duration" ];     //ssteps[ i ].duration;
  double scanint_sec     = ssvals[ curssx ][ "scanintv" ];     //ssteps[ i ].scanintv;
  double scanint_sec_min;
  std::modf (ssvals[ curssx ][ "scanintv_min" ], &scanint_sec_min);
  
  int scancount = 0;
  
  //ALEXEY: use this algorithm to calculate scanCount && scanInt
  if ( scanint_sec > scanint_sec_min*tot_wvl )
      scancount     = int( duration_sec / scanint_sec );
  else
    scancount    = int( duration_sec / (scanint_sec_min * tot_wvl) );
    
  le_scans_per_cell -> setText( QString::number( scancount ));
}


/* DURATION ***********************************************************************************/
// Slot for change in duration time (hours)
void US_ExperGuiSpeeds::ssChgDuratTime_hh( int val )
{
   double ssdurday  = (double)sb_durat_dd->value();
   double ssdurhr   = val;
   double ssdurmin  = (double)sb_durat_mm->value();
   double ssdursec  = (double)sb_durat_ss->value();
   double ssdurtim  = ( ssdurday * 3600.0 * 24 ) + ( ssdurhr * 3600.0 ) + ( ssdurmin * 60.0 ) + ssdursec;

   ssvals[ curssx ][ "duration" ] = ssdurtim;  // Set Duration in step vals vector

   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );

   qDebug() << "Time in sec aftes HH changed: " << ssdurtim;

   // Set total time
   QList< int > hms_tot;
   double ssdurtim_d = ssdurtim + ssvals[ curssx ][ "delay_stage" ];
   ssvals[ curssx ][ "total_time" ] = ssdurtim_d;
   US_RunProtocol::timeToList( ssdurtim_d, hms_tot );
   QString tot_str = QString::number( hms_tot[ 0 ] ) + "d " + QString::number( hms_tot[ 1 ] ) + "h " + QString::number( hms_tot[ 2 ] ) + "m ";
   le_total_time->setText( tot_str );

   //Set Scan # in estimator
   int tot_wvl = (int)sb_wvl_per_cell->value();
   double duration_sec    = ssvals[ curssx ][ "duration" ];     //ssteps[ i ].duration;
   double scanint_sec     = ssvals[ curssx ][ "scanintv" ];     //ssteps[ i ].scanintv;
   // double scanint_sec_min = ssvals[ curssx ][ "scanintv_min" ]; //ssteps[ i ].scanintv_min;
   double scanint_sec_min;
   std::modf (ssvals[ curssx ][ "scanintv_min" ], &scanint_sec_min);
   
   int scancount = 0;
   if ( scanint_sec > scanint_sec_min*tot_wvl )
     scancount     = int( duration_sec / scanint_sec );
   else
     scancount    = int( duration_sec / (scanint_sec_min * tot_wvl) );
   
   le_scans_per_cell -> setText( QString::number( scancount ));
}

// Slot for change in duration time (mins)
void US_ExperGuiSpeeds::ssChgDuratTime_mm( int val )
{
   double ssdurday  = (double)sb_durat_dd->value();
   double ssdurhr   = (double)sb_durat_hh->value();
   double ssdurmin  = val;
   double ssdursec  = (double)sb_durat_ss->value();
   double ssdurtim  = ( ssdurday * 3600.0 * 24 ) + ( ssdurhr * 3600.0 ) + ( ssdurmin * 60.0 ) + ssdursec;

   ssvals[ curssx ][ "duration" ] = ssdurtim;  // Set Duration in step vals vector

   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );

   qDebug() << "Time in sec aftes MINS changed: " << ssdurtim;

   // Set total time
   QList< int > hms_tot;
   double ssdurtim_d = ssdurtim + ssvals[ curssx ][ "delay_stage" ];
   ssvals[ curssx ][ "total_time" ] = ssdurtim_d;
   US_RunProtocol::timeToList( ssdurtim_d, hms_tot );
   QString tot_str = QString::number( hms_tot[ 0 ] ) + "d " + QString::number( hms_tot[ 1 ] ) + "h " + QString::number( hms_tot[ 2 ] ) + "m ";
   le_total_time->setText( tot_str );

   //Set Scan # in estimator
   int tot_wvl = (int)sb_wvl_per_cell->value();
   double duration_sec    = ssvals[ curssx ][ "duration" ];     //ssteps[ i ].duration;
   double scanint_sec     = ssvals[ curssx ][ "scanintv" ];     //ssteps[ i ].scanintv;
   //double scanint_sec_min = ssvals[ curssx ][ "scanintv_min" ]; //ssteps[ i ].scanintv_min;
   double scanint_sec_min;
   std::modf (ssvals[ curssx ][ "scanintv_min" ], &scanint_sec_min);
   
   int scancount = 0;
   if ( scanint_sec > scanint_sec_min*tot_wvl )
     scancount     = int( duration_sec / scanint_sec );
   else
     scancount    = int( duration_sec / (scanint_sec_min * tot_wvl) );
   
   le_scans_per_cell -> setText( QString::number( scancount ));
   
}

// Slot for change in duration time (sec)
void US_ExperGuiSpeeds::ssChgDuratTime_ss( int val )
{
   double ssdurday  = (double)sb_durat_dd->value();
   double ssdurhr   = (double)sb_durat_hh->value();
   double ssdurmin  = (double)sb_durat_mm->value();
   double ssdursec  = val;
   double ssdurtim  = ( ssdurday * 3600.0 * 24 ) + ( ssdurhr * 3600.0 ) + ( ssdurmin * 60.0 ) + ssdursec;

   ssvals[ curssx ][ "duration" ] = ssdurtim;  // Set Duration in step vals vector

   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );

   // Set total time
   QList< int > hms_tot;
   double ssdurtim_d = ssdurtim + ssvals[ curssx ][ "delay_stage" ];
   ssvals[ curssx ][ "total_time" ] = ssdurtim_d;
   US_RunProtocol::timeToList( ssdurtim_d, hms_tot );
   QString tot_str = QString::number( hms_tot[ 0 ] ) + "d " + QString::number( hms_tot[ 1 ] ) + "h " + QString::number( hms_tot[ 2 ] ) + "m ";
   le_total_time->setText( tot_str );

   //Set Scan # in estimator
   int tot_wvl = (int)sb_wvl_per_cell->value();
   double duration_sec    = ssvals[ curssx ][ "duration" ];     //ssteps[ i ].duration;
   double scanint_sec     = ssvals[ curssx ][ "scanintv" ];     //ssteps[ i ].scanintv;
   //double scanint_sec_min = ssvals[ curssx ][ "scanintv_min" ]; //ssteps[ i ].scanintv_min;
   double scanint_sec_min;
   std::modf (ssvals[ curssx ][ "scanintv_min" ], &scanint_sec_min);
   
   int scancount = 0;
   if ( scanint_sec > scanint_sec_min*tot_wvl )
     scancount     = int( duration_sec / scanint_sec );
   else
     scancount    = int( duration_sec / (scanint_sec_min * tot_wvl) );
   
   le_scans_per_cell -> setText( QString::number( scancount ));
}

// Slot for change in duration day
void US_ExperGuiSpeeds::ssChgDuratDay( int val )
{
   double ssdurday  = val;
   double ssdurhr   = (double)sb_durat_hh->value();
   double ssdurmin  = (double)sb_durat_mm->value();
   double ssdursec  = (double)sb_durat_ss->value();
   double ssdurtim  = ( ssdurday * 3600.0 * 24 ) + ( ssdurhr * 3600.0 ) + ( ssdurmin * 60.0 ) + ssdursec;

   ssvals[ curssx ][ "duration" ] = ssdurtim;  // Set Duration in step vals vector

   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );

   qDebug() << "Time in sec aftes DAYS changed: " << ssdurtim;

   // Set total time
   QList< int > hms_tot;
   double ssdurtim_d = ssdurtim + ssvals[ curssx ][ "delay_stage" ];
   ssvals[ curssx ][ "total_time" ] = ssdurtim_d;
   US_RunProtocol::timeToList( ssdurtim_d, hms_tot );
   QString tot_str = QString::number( hms_tot[ 0 ] ) + "d " + QString::number( hms_tot[ 1 ] ) + "h " + QString::number( hms_tot[ 2 ] ) + "m ";
   le_total_time->setText( tot_str );

   //Set Scan # in estimator
   int tot_wvl = (int)sb_wvl_per_cell->value();
   double duration_sec    = ssvals[ curssx ][ "duration" ];     //ssteps[ i ].duration;
   double scanint_sec     = ssvals[ curssx ][ "scanintv" ];     //ssteps[ i ].scanintv;
   //double scanint_sec_min = ssvals[ curssx ][ "scanintv_min" ]; //ssteps[ i ].scanintv_min;
   double scanint_sec_min;
   std::modf (ssvals[ curssx ][ "scanintv_min" ], &scanint_sec_min);
   
   int scancount = 0;
   if ( scanint_sec > scanint_sec_min*tot_wvl )
     scancount     = int( duration_sec / scanint_sec );
   else
     scancount    = int( duration_sec / (scanint_sec_min * tot_wvl) );
   
   le_scans_per_cell -> setText( QString::number( scancount ));
}
/* END OF DURATION ***********************************************************************************/


/* UV-vis SCAN Int ***********************************************************************************/
// Slot for change in Scan Int time (hours)
void US_ExperGuiSpeeds::ssChgScIntTime_hh( int val )
{
   double ssscinthr   = val;
   double minimum_hh =  scanint_hh_min[curssx];
   double minimum_mm =  scanint_mm_min[curssx];
   double minimum_ss =  scanint_ss_min[curssx];

   double ssscintmin;
   double ssscintsec;

   ssscintmin = (double)sb_scnint_mm->value();
   ssscintsec = (double)sb_scnint_ss->value();

   //if ( val - minimum_hh == 1 )
   if ( val - minimum_hh > 0 ) 
   {
      sb_scnint_mm->setMinimum(0);
      sb_scnint_ss->setMinimum(0);
   }
   if ( val == minimum_hh )
   {
      if ( ssscintmin <= minimum_mm )
      {
         sb_scnint_mm->setMinimum(minimum_mm);
         sb_scnint_mm->setValue(minimum_mm);
         sb_scnint_ss->setMinimum(minimum_ss);
         sb_scnint_ss->setValue(minimum_ss);

         ssscintmin  = (double)sb_scnint_mm->value();  // Bug fixed
         ssscintsec  = (double)sb_scnint_ss->value();
      }
   }

   double ssscinttim  = ( ssscinthr * 3600.0 ) + ( ssscintmin * 60.0 ) + ssscintsec;
   ssvals[ curssx ][ "scanintv" ] = ssscinttim;  // Set ScInt in step vals vector

   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );

   //Set Scan # in estimator
   int tot_wvl = (int)sb_wvl_per_cell->value();
   double duration_sec    = ssvals[ curssx ][ "duration" ];     //ssteps[ i ].duration;
   double scanint_sec     = ssvals[ curssx ][ "scanintv" ];     //ssteps[ i ].scanintv;
   //double scanint_sec_min = ssvals[ curssx ][ "scanintv_min" ]; //ssteps[ i ].scanintv_min;
   double scanint_sec_min;
   std::modf (ssvals[ curssx ][ "scanintv_min" ], &scanint_sec_min);
   
   int scancount = 0;
   if ( scanint_sec > scanint_sec_min*tot_wvl )
     scancount     = int( duration_sec / scanint_sec );
   else
     scancount    = int( duration_sec / (scanint_sec_min * tot_wvl) );
   
   le_scans_per_cell -> setText( QString::number( scancount ));
   
}

// Slot for change in Scan Int time (mins)
void US_ExperGuiSpeeds::ssChgScIntTime_mm( int val )
{
   double ssscinthr   = (double)sb_scnint_hh->value();
   double ssscintmin  = val;
   double minimum_hh =  scanint_hh_min[curssx];
   double minimum_mm =  scanint_mm_min[curssx];
   double minimum_ss =  scanint_ss_min[curssx];

   //if ( val - minimum_mm == 0 )
   if ( val - minimum_mm > 0 )
     {
       sb_scnint_ss->setMinimum(0);
       //sb_scnint_ss->setValue(0);
     }
   if ( val == minimum_mm )
   {
      if ( ssscinthr == minimum_hh )
      {
         sb_scnint_ss->setMinimum(minimum_ss);
         sb_scnint_ss->setValue(minimum_ss);
      }
   }

   double ssscintsec  = (double)sb_scnint_ss->value();
   double ssscinttim  = ( ssscinthr * 3600.0 ) + ( ssscintmin * 60.0 ) + ssscintsec;
   ssvals[ curssx ][ "scanintv" ] = ssscinttim;  // Set Duration in step vals vector

   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );

   //Set Scan # in estimator
   int tot_wvl = (int)sb_wvl_per_cell->value();
   double duration_sec    = ssvals[ curssx ][ "duration" ];     //ssteps[ i ].duration;
   double scanint_sec     = ssvals[ curssx ][ "scanintv" ];     //ssteps[ i ].scanintv;
   //double scanint_sec_min = ssvals[ curssx ][ "scanintv_min" ]; //ssteps[ i ].scanintv_min;
   double scanint_sec_min;
   std::modf (ssvals[ curssx ][ "scanintv_min" ], &scanint_sec_min);
   
   int scancount = 0;
   if ( scanint_sec > scanint_sec_min*tot_wvl )
     scancount     = int( duration_sec / scanint_sec );
   else
     scancount    = int( duration_sec / (scanint_sec_min * tot_wvl) );
   
   le_scans_per_cell -> setText( QString::number( scancount ));
}

// Slot for change in Scan Int time (sec)
void US_ExperGuiSpeeds::ssChgScIntTime_ss( int val )
{
   double ssscinthr   = (double)sb_scnint_hh->value();
   double ssscintmin  = (double)sb_scnint_mm->value();
   double ssscintsec  = val;
   double ssscinttim  = ( ssscinthr * 3600.0 ) + ( ssscintmin * 60.0 ) + ssscintsec;
   ssvals[ curssx ][ "scanintv" ] = ssscinttim;  // Set Duration in step vals vector

   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );

   //Set Scan # in estimator
   int tot_wvl = (int)sb_wvl_per_cell->value();
   double duration_sec    = ssvals[ curssx ][ "duration" ];     //ssteps[ i ].duration;
   double scanint_sec     = ssvals[ curssx ][ "scanintv" ];     //ssteps[ i ].scanintv;
   //double scanint_sec_min = ssvals[ curssx ][ "scanintv_min" ]; //ssteps[ i ].scanintv_min;
   double scanint_sec_min;
   std::modf (ssvals[ curssx ][ "scanintv_min" ], &scanint_sec_min);
   
   int scancount = 0;
   if ( scanint_sec > scanint_sec_min*tot_wvl )
     scancount     = int( duration_sec / scanint_sec );
   else
     scancount    = int( duration_sec / (scanint_sec_min * tot_wvl) );
   
   le_scans_per_cell -> setText( QString::number( scancount ));
   
}
/* END OF UV-vis SCAN Int ***********************************************************************************/



/* Interference SCAN Int ***********************************************************************************/
// Slot for change in Scan Int time (hours)
void US_ExperGuiSpeeds::ssChgScIntTime_int_hh( int val )
{
   double ssscinthr   = val;
   double minimum_hh =  scanint_hh_int_min[curssx];
   double minimum_mm =  scanint_mm_int_min[curssx];
   double minimum_ss =  scanint_ss_int_min[curssx];

   double ssscintmin;
   double ssscintsec;

   ssscintmin = (double)sb_scnint_int_mm->value();
   ssscintsec = (double)sb_scnint_int_ss->value();

   if ( val - minimum_hh == 1 )
   {
      sb_scnint_int_mm->setMinimum(0);
      sb_scnint_int_ss->setMinimum(0);
   }
   if ( val == minimum_hh )
   {
      if ( ssscintmin <= minimum_mm )
      {
         sb_scnint_int_mm->setMinimum(minimum_mm);
         sb_scnint_int_mm->setValue(minimum_mm);
         sb_scnint_int_ss->setMinimum(minimum_ss);
         sb_scnint_int_ss->setValue(minimum_ss);

         ssscintmin  = (double)sb_scnint_int_mm->value();  // Bug fixed
         ssscintsec  = (double)sb_scnint_int_ss->value();
      }
   }

   double ssscinttim  = ( ssscinthr * 3600.0 ) + ( ssscintmin * 60.0 ) + ssscintsec;
   ssvals[ curssx ][ "scanintv_int" ] = ssscinttim;  // Set ScInt in step vals vector

   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );
}

// Slot for change in Scan Int time (mins)
void US_ExperGuiSpeeds::ssChgScIntTime_int_mm( int val )
{
   double ssscinthr   = (double)sb_scnint_int_hh->value();
   double ssscintmin  = val;
   double minimum_hh =  scanint_hh_int_min[curssx];
   double minimum_mm =  scanint_mm_int_min[curssx];
   double minimum_ss =  scanint_ss_int_min[curssx];

   if ( val - minimum_mm == 1 )
     {
       sb_scnint_int_ss->setMinimum(0);
       //sb_scnint_ss->setValue(0);
     }
   if ( val == minimum_mm )
   {
      if ( ssscinthr == minimum_hh )
      {
         sb_scnint_int_ss->setMinimum(minimum_ss);
         sb_scnint_int_ss->setValue(minimum_ss);
      }
   }

   double ssscintsec  = (double)sb_scnint_int_ss->value();
   double ssscinttim  = ( ssscinthr * 3600.0 ) + ( ssscintmin * 60.0 ) + ssscintsec;
   ssvals[ curssx ][ "scanintv_int" ] = ssscinttim;  // Set Duration in step vals vector

   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );
}

// Slot for change in Scan Int time (sec)
void US_ExperGuiSpeeds::ssChgScIntTime_int_ss( int val )
{
   double ssscinthr   = (double)sb_scnint_int_hh->value();
   double ssscintmin  = (double)sb_scnint_int_mm->value();
   double ssscintsec  = val;
   double ssscinttim  = ( ssscinthr * 3600.0 ) + ( ssscintmin * 60.0 ) + ssscintsec;
   ssvals[ curssx ][ "scanintv_int" ] = ssscinttim;  // Set Duration in step vals vector

   profdesc[ curssx ] = speedp_description( curssx );
   cb_prof->setItemText( curssx, profdesc[ curssx ] );
}
/* END OF Interference SCAN Int ***********************************************************************************/





/* UV-vis DELAY ***********************************************************************************/
/* Delay counters are disabled and updated programmatically... but we retain this slots...*/
// Slot for change in delay time (hours)
void US_ExperGuiSpeeds::ssChgDelayTime_hh( int val )
{
   double ssdlyhr   = val;
   double minimum_mm =  delay_mm_min[curssx];

   if ( val > 0 )
     sb_delay_mm->setMinimum(0);

   if ( val == 0 )
     sb_delay_mm->setMinimum(minimum_mm);

   double ssdlymin  = (double)sb_delay_mm->value();
   double ssdlysec  = (double)sb_delay_ss->value();
   double ssdlytim  = ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
DbgLv(1) << "EGSp: chgDlyT:  ssdly h m s" << ssdlyhr << ssdlymin << ssdlysec
<< "t" << ssdlytim;
   ssvals[ curssx ][ "delay" ] = ssdlytim;  // Set Delay in step vals vector

}

// Slot for change in delay time (mins)
void US_ExperGuiSpeeds::ssChgDelayTime_mm( int val )
{
   double ssdlyhr   = (double)sb_delay_hh->value();
   double ssdlymin  = val;
   double ssdlysec  = (double)sb_delay_ss->value();
   double ssdlytim  = ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
DbgLv(1) << "EGSp: chgDlyT:  ssdly h m s" << ssdlyhr << ssdlymin << ssdlysec
<< "t" << ssdlytim;
   ssvals[ curssx ][ "delay" ] = ssdlytim;  // Set Delay in step vals vector

}

// Slot for change in delay time (hour/minute/second)
void US_ExperGuiSpeeds::ssChgDelayTime_ss( int val )
{
   double ssdlyhr   = (double)sb_delay_hh->value();
   double ssdlymin  = (double)sb_delay_mm->value();
   double ssdlysec  = val;
   double ssdlytim  = ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
DbgLv(1) << "EGSp: chgDlyT:  ssdly h m s" << ssdlyhr << ssdlymin << ssdlysec
<< "t" << ssdlytim;
   ssvals[ curssx ][ "delay" ] = ssdlytim;  // Set Delay in step vals vector

}

// Slot for change in delay day
void US_ExperGuiSpeeds::ssChgDelayDay( int val )
{
   double ssdlyday  = (double)val;
//    double ssdlyhr   = tm_delay->sectionText( QDateTimeEdit::HourSection ).toDouble();  //ALEXEY

   double ssdlyhr   = (double)sb_delay_hh->value();
   double ssdlymin  = (double)sb_delay_mm->value();
   double ssdlysec  = (double)sb_delay_ss->value();

DbgLv(1) << "EGSp: chgDlyD: val" << val << "ssdly d h"
<< ssdlyday << ssdlyhr;
// << ssdlyhr << ssdlymin << ssdlysec << "t" << ssdlytim;

   double ssdlytim  = ( val * 3600.0 * 24 ) + ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
   ssvals[ curssx ][ "delay" ] = ssdlytim;  // Set Delay in step vals vector

}
/* END OF UV-vis DELAY ***********************************************************************************/

/* Interference DELAY ***********************************************************************************/
/* Delay counters are disabled and updated programmatically... but we retain this slots...*/
// Slot for change in delay time (hours)
void US_ExperGuiSpeeds::ssChgDelayTime_int_hh( int val )
{
   double ssdlyhr   = val;
   double minimum_mm =  delay_mm_int_min[curssx];

   if ( val > 0 )
     sb_delay_int_mm->setMinimum(0);

   if ( val == 0 )
     sb_delay_int_mm->setMinimum(minimum_mm);

   double ssdlymin  = (double)sb_delay_int_mm->value();
   double ssdlysec  = (double)sb_delay_int_ss->value();
   double ssdlytim  = ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
DbgLv(1) << "EGSp: chgDlyT:  ssdly h m s" << ssdlyhr << ssdlymin << ssdlysec
<< "t" << ssdlytim;
   ssvals[ curssx ][ "delay_int" ] = ssdlytim;  // Set Delay in step vals vector
}

// Slot for change in delay time (mins)
void US_ExperGuiSpeeds::ssChgDelayTime_int_mm( int val )
{
   double ssdlyhr   = (double)sb_delay_int_hh->value();
   double ssdlymin  = val;
   double ssdlysec  = (double)sb_delay_int_ss->value();
   double ssdlytim  = ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
DbgLv(1) << "EGSp: chgDlyT:  ssdly h m s" << ssdlyhr << ssdlymin << ssdlysec
<< "t" << ssdlytim;
   ssvals[ curssx ][ "delay_int" ] = ssdlytim;  // Set Delay in step vals vector
}

// Slot for change in delay time (hour/minute/second)
void US_ExperGuiSpeeds::ssChgDelayTime_int_ss( int val )
{
   double ssdlyhr   = (double)sb_delay_int_hh->value();
   double ssdlymin  = (double)sb_delay_int_mm->value();
   double ssdlysec  = val;
   double ssdlytim  = ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
DbgLv(1) << "EGSp: chgDlyT:  ssdly h m s" << ssdlyhr << ssdlymin << ssdlysec
<< "t" << ssdlytim;
   ssvals[ curssx ][ "delay_int" ] = ssdlytim;  // Set Delay in step vals vector
}

// Slot for change in delay day
void US_ExperGuiSpeeds::ssChgDelayDay_int( int val )
{
   double ssdlyday  = (double)val;
//    double ssdlyhr   = tm_delay->sectionText( QDateTimeEdit::HourSection ).toDouble();  //ALEXEY

   double ssdlyhr   = (double)sb_delay_int_hh->value();
   double ssdlymin  = (double)sb_delay_int_mm->value();
   double ssdlysec  = (double)sb_delay_int_ss->value();

DbgLv(1) << "EGSp: chgDlyD: val" << val << "ssdly d h"
<< ssdlyday << ssdlyhr;
// << ssdlyhr << ssdlymin << ssdlysec << "t" << ssdlytim;

   double ssdlytim  = ( val * 3600.0 * 24 ) + ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
   ssvals[ curssx ][ "delay_int" ] = ssdlytim;  // Set Delay in step vals vector
}
/* END OF Interference DELAY ***********************************************************************************/


/* STAGE DELAY ***********************************************************************************/
// Slot for change in delay_stage time (hours)
void US_ExperGuiSpeeds::ssChgDelayStageTime_hh( int val )
{
   double ssdlyhr   = val;
   double ssdlymin  = (double)sb_delay_st_mm->value();
   double ssdlysec  = (double)sb_delay_st_ss->value();
   double ssdlytim  = ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
   ssvals[ curssx ][ "delay_stage" ] = ssdlytim;  // Set Delay Stage in step vals vector

   if ( curssx == 0 && ck_sync_delay->isChecked() ) //1st stage
     stageDelay_sync();

   // Set total time
   QList< int > hms_tot;
   double ssdurtim_d = ssdlytim + ssvals[ curssx ][ "duration" ];
   ssvals[ curssx ][ "total_time" ] = ssdurtim_d;
   US_RunProtocol::timeToList( ssdurtim_d, hms_tot );
   QString tot_str = QString::number( hms_tot[ 0 ] ) + "d " + QString::number( hms_tot[ 1 ] ) + "h " + QString::number( hms_tot[ 2 ] ) + "m ";
   le_total_time->setText( tot_str );

}

// Slot for change in delay time (mins)
void US_ExperGuiSpeeds::ssChgDelayStageTime_mm( int val )
{
   double ssdlyhr   = (double)sb_delay_st_hh->value();
   double ssdlymin  = val;
   double ssdlysec  = (double)sb_delay_st_ss->value();
   double ssdlytim  = ( ssdlyhr * 3600.0 ) + ( ssdlymin * 60.0 ) + ssdlysec;
   ssvals[ curssx ][ "delay_stage" ] = ssdlytim;  // Set Delay in step vals vector

   if ( curssx == 0  && ck_sync_delay->isChecked() ) //1st stage
     stageDelay_sync();


   // Set total time
   QList< int > hms_tot;
   double ssdurtim_d = ssdlytim + ssvals[ curssx ][ "duration" ];
   ssvals[ curssx ][ "total_time" ] = ssdurtim_d;
   US_RunProtocol::timeToList( ssdurtim_d, hms_tot );
   QString tot_str = QString::number( hms_tot[ 0 ] ) + "d " + QString::number( hms_tot[ 1 ] ) + "h " + QString::number( hms_tot[ 2 ] ) + "m ";
   le_total_time->setText( tot_str );
}
/* END OF STAGE DELAY ***********************************************************************************/

//Slot to synchronize all stage delays with that for the 1st stage
void US_ExperGuiSpeeds::stageDelay_sync( void )
{
  double delay = ssvals[ 0 ][ "delay_stage" ];                //delay for the 1st stage in sec

  double delay_hh   = qFloor( delay / 3600.0 );               // Delay in hh
  double delay_mm   = (delay / 60.0) - ( delay_hh * 60.0 );   // Delay in min

  sb_delay_st_hh ->setValue( (int)delay_hh );
  sb_delay_st_mm ->setValue( (int)delay_mm );

  qDebug() << "IN STAGE SYNC: nspeed: " << nspeed;
  int tot_speeds = ssvals.size();
  for ( int i = 1; i < tot_speeds; i++ )
    {
      ssvals[ i ][ "delay_stage" ] = delay;
    }

}

void US_ExperGuiSpeeds::syncdelayChecked( bool checked )
{
  if (checked)
    stageDelay_sync();
}

// Function to adjust delay minimum when related values are changed
void US_ExperGuiSpeeds::adjustDelay( void )
{
   // Set minimum delay time based on speed change and acceleration
   double cspeed    = ssvals[ curssx ][ "speed" ];   // Current step's speed
   double pspeed    = ( curssx > 0 ) ? ssvals[ curssx - 1 ][ "speed" ] : 0.0;
   double spdelta   = fabs(cspeed - pspeed);               // Speed delta          <-- In case there is deceleration..
   double accel     = ssvals[ curssx ][ "accel" ];   // Acceleration
   double delaylow  = qCeil( spdelta / accel );      // Low seconds delay

   //Uv-vis
   QList< int > dhms;
   double actual_delaylow = 2*60 + delaylow;   // ALEXEY: Uv-vis delay = 2 mins + speed/accel;
   US_RunProtocol::timeToList( actual_delaylow, dhms );
   sb_delay_hh ->setValue( (int)dhms[ 1 ] );

   if ( (int)dhms[ 3 ] > 0 )                 //ALEXEY round to nearest min.
   {
      if ( (int)dhms[ 2 ] == 0 )
      {
         sb_delay_mm->setMinimum(1);
         sb_delay_mm ->setValue(1);
      }
      if ( (int)dhms[ 2 ] > 0 )
      {
         sb_delay_mm->setMinimum( (int)dhms[ 2 ] + 1 );
         sb_delay_mm ->setValue( (int)dhms[ 2 ] + 1 );
      }
   }
   else
   {
      sb_delay_mm->setMinimum( (int)dhms[ 2 ] );
      sb_delay_mm ->setValue( (int)dhms[ 2 ] );
   }
   delay_mm_min[ curssx ] = sb_delay_mm->value();

   //interference
   QList< int > dhms_int;
   double actual_delaylow_int = 6*60 + delaylow;   // ALEXEY: Interference delay = 6 mins + speed/accel;
   US_RunProtocol::timeToList( actual_delaylow_int, dhms_int );
   sb_delay_int_hh ->setValue( (int)dhms_int[ 1 ] );

   if ( (int)dhms_int[ 3 ] > 0 )                 //ALEXEY round to nearest min.
   {
      if ( (int)dhms_int[ 2 ] == 0 )
      {
         sb_delay_int_mm->setMinimum(1);
         sb_delay_int_mm ->setValue(1);
      }
      if ( (int)dhms_int[ 2 ] > 0 )
      {
         sb_delay_int_mm->setMinimum( (int)dhms_int[ 2 ] + 1 );
         sb_delay_int_mm ->setValue( (int)dhms_int[ 2 ] + 1 );
      }
   }
   else
   {
      sb_delay_int_mm->setMinimum( (int)dhms_int[ 2 ] );
      sb_delay_int_mm ->setValue( (int)dhms_int[ 2 ] );
   }
   delay_mm_int_min[ curssx ] = sb_delay_int_mm->value();

}

// Panel for Cells parameters
US_ExperGuiCells::US_ExperGuiCells( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
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

      //rpCells->nused      = kused;      // Resize used-cells vector
      //rpCells->used.resize( kused );

      rpCells->nused        = 0;      // ALEXEY: # used cells should be 0 when Rotor
      rpCells->used.clear();          // ALEXEY: no used cells when rotor is changed - otherwise cells are not populated correcly
   }
   else
     { // If cells count increases, same as above: clear used, i.e. make all fresh-empty, and counterbalance (empty-counterbalance)
       rpCells->nused        = 0;      // ALEXEY: # used cells should be 0 when Rotor
       rpCells->used.clear();          // ALEXEY: no used cells when rotor is changed - otherwise cells are not populated correcly
     }


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
      {
         if ( cc_cenps[ xrow ]->currentText().contains( tr( "counterbalance" ) ) )
         {  // Cross cell is counterbalance
            jsel                = 1;       // Usually "Beckman counterbalance"
            QString cpname      = cc_cenps[ irow ]->currentText();
            DbgLv(1) << "EGCe:cpChg:   cpname" << cpname << "tcb_centps" << tcb_centps;
            if ( tcb_centps.contains( cpname ) )
               jsel                = 2;    // In some cases "Titanium counterbalance"
         }
         else
         {
            jsel = sel + 3;        //ALEXEY  plus 3 since cent. list is longer in counterbalance rows
         }
      }

DbgLv(1) << "EGCe:cpChg:   CB:jsel" << jsel;
      cc_cenps[ xrow ]->setCurrentIndex( jsel );
   }
   else     //ALEXEY: if index of the counterbalance - treat when NOT counterbalance
   {
      QString cpname_counter      = cc_cenps[ irow ]->currentText();
      if ( ! cpname_counter.contains( tr( "counterbalance" ) ) )
      {
         cc_winds[ irow ]->setVisible( true );
         //ALEXEY: change cross cell centerpiece (8->4, or 4->2)
         int halfnh_c          = nholes / 2;
         int xrow_c            = irow - halfnh_c;
         int jsel_c            = sel - 3;        // Use same centerpiece for cross (minus 3 since cent. list is longer in counterbalance rows)

         cc_cenps[ xrow_c ]->setCurrentIndex( jsel_c );

         // Set windows
         if ( cc_winds[ xrow_c ]->currentText().contains( tr( "quartz" ) ) )
            cc_winds[ irow ]->setCurrentIndex( 0 );
         else if ( cc_winds[ xrow_c ]->currentText().contains( tr( "sapphire" ) ) )
            cc_winds[ irow ]->setCurrentIndex( 1 );
       }
     else
       cc_winds[ irow ]->setVisible( false );
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

      if ( xrow == icbal )
      {
         // ALEXEY: Cross cell is counterbalance but used as centerpiece
         if ( ! cc_cenps[ xrow ]->currentText().contains( tr( "counterbalance" ) ) )
         {
            cc_winds[ xrow ]->setCurrentIndex( sel );
         }
      }
      else
         cc_winds[ xrow ]->setCurrentIndex( sel );
   }
   else   //ALEXEY: if index of the counterbalance - treat when NOT counterbalance
   {
      QString cpname_counter      = cc_cenps[ irow ]->currentText();
      if ( ! cpname_counter.contains( tr( "counterbalance" ) ) )
      {
         //ALEXEY: change cross cell centerpiece (8->4, or 4->2)
         int halfnh_c          = nholes / 2;
         int xrow_c            = irow - halfnh_c;
         cc_winds[ xrow_c ]->setCurrentIndex( sel );
      }
   }
}

// Panel for Solutions parameters
US_ExperGuiSolutions::US_ExperGuiSolutions( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ExperimentMain*)topw;
   rpSolut             = &(mainw->currProto.rpSolut);
   mxrow               = 24;     // Maximum possible rows
   nchant              = 0;
   nchanf              = 0;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner(
                            tr( "5: Specify a solution for each cell/channel" ) );
   panel->addWidget( lb_panel );

   QPushButton* pb_manage   = us_pushbutton( tr( "Manage Solutions" ) );
   QPushButton* pb_details  = us_pushbutton( tr( "View Solution Details" ) );
   QLabel* lb_hdr1          = us_banner( tr( "Cell / Channel" ) );
   QLabel* lb_hdr2          = us_banner( tr( "Solution" ) );

   QGridLayout* banners = new QGridLayout();
   int row             = 1;
   banners->addWidget( pb_manage,       row,   0, 1, 3 );
   banners->addWidget( pb_details,      row++, 3, 1, 3 );
   banners->addWidget( lb_hdr1,         row,   0, 1, 2 );
   banners->addWidget( lb_hdr2,         row++, 2, 1, 4 );

   QGridLayout* genL   = new QGridLayout();
   genL->setSpacing        ( 2 );
   genL->setContentsMargins( 2, 2, 2, 2 );

   row = 1;

   QStringList cpnames = sibLValue( "cells", "centerpieces" );
   int nholes          = sibIValue( "rotor", "nholes" );
   QString add_comm    = tr( "Add to Comments" );
DbgLv(1) << "EGSo:  nholes mxrow" << nholes << mxrow;

   QLabel*       cclabl;
   QComboBox*    cb_solu;
   QPushButton*  pb_comm;

   allSolutions();        // Read in all solution names and IDs

   QString chn_none( "none" );

   for ( int ii = 0; ii < mxrow; ii++ )
   {  // Loop to build initial place-holder solution rows

     solution_comment_init[ ii ] = false;                       //ALEXEY: initialize channel comments in solutions

     QString schan;
      if      ( ii == 0 ) schan = QString( "2 / A" );
      else if ( ii == 1 ) schan = QString( "2 / B" );
      else if ( ii == 2 ) schan = QString( "6 / A" );
      else if ( ii == 3 ) schan = QString( "6 / B" );
      else                schan = chn_none;
      cclabl              = us_label( schan );
      cb_solu             = us_comboBox();
      pb_comm             = us_pushbutton( add_comm );

      if ( schan != chn_none )
         srchans << schan;

      QString strow       = QString::number( ii );
      cclabl ->setObjectName( strow + ": label" );
      cb_solu->setObjectName( strow + ": solution" );
      pb_comm->setObjectName( strow + ": addcomm" );

      genL->addWidget( cclabl,  row,    0, 1, 2 );
      genL->addWidget( cb_solu, row,    2, 1, 3 );
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


   panel->addLayout(banners);
   genL->setAlignment(Qt::AlignTop);

   QScrollArea *scrollArea = new QScrollArea(this);
   QWidget *containerWidget = new QWidget;
   containerWidget->setLayout(genL);
   scrollArea->setWidgetResizable(true);
   scrollArea->setWidget(containerWidget);

   panel->addWidget(scrollArea);

   //panel->addStretch();

DbgLv(1) << "EGSo:main: call initPanel()";
   initPanel();
}

//Function to clear solution's comment when colution changed
void US_ExperGuiSolutions::changeSolu ( int ind )
{
   QObject* sobj       = sender();      // Sender object
   QString oname       = sobj->objectName();
   int irow            = oname.section( ":", 0, 0 ).toInt();

   qDebug() << "Solution: oname, irow -- " << oname << irow;

   manual_comment[ QString::number( irow ) ] = QString("");
   //rpSolut->chsols[ irow ].ch_comment = QString("");
}

// Function to rebuild the Solutions protocol after Cells change
void US_ExperGuiSolutions::rebuild_Solut( void )
{
   int nchans          = sibIValue( "cells", "nchans" );
DbgLv(1) << "EGSo: rbS: nchans nchant" << nchans << nchant
 << "rpS.nschan" << rpSolut->nschan;

//  ALEXEY - The following section conflicts when investigator changed;
//           In principle, not needed !!!

 // if ( nchans == nchant )     // No cells change means no rebuild //ALEXEY: wrong condition !!! have to also compare content of channels vs cells
 //     {

 //       //ALEXEY: need to compare srchans QStringLists from Solutions && Cells:
 //       QStringList srchans_check;
 //       srchans_check.clear();

 //       QStringList centps_check  = sibLValue( "cells", "centerpieces" );
 //       int ncused_check          = centps_check.count();

 //       for ( int ii = 0; ii < ncused_check; ii++ )
 //       {
 //          QString centry_check      = centps_check[ ii ];
 //          int chx_check             = centry_check.indexOf( "-channel" );
 //          if ( chx_check > 0 )
 //          {
 //             QString scell_check       = centry_check.section( ":", 0, 0 )
 //                                                     .section( " ", 1, 1 );
 //             QString schans_check( "ABCDEF" );
 //             int nchan_check           = centry_check.left( chx_check ).section( " ", -1, -1 )
 //       .simplified().toInt();
 //             for ( int jj = 0; jj < nchan_check; jj++ )
 //             {
 //                QString channel_check     = scell_check + " / " + QString( schans_check ).mid( jj, 1 );
 //                if ( (QString( schans_check ).mid( jj, 1 )).contains( "A" ) ) //ALEXEY: channel lables
 //                   srchans_check << channel_check + ", sample [right]";
 //                else if ( (QString( schans_check ).mid( jj, 1 )).contains( "B" ) )
 //                   srchans_check << channel_check + ", reference [left]";
 //                else
 //                   srchans_check << channel_check;
 //             }
 //          }
 //       }

 //       DbgLv(1) << "SRCHANS from (Solutions):         " << srchans;
 //       DbgLv(1) << "SRCHANS (from actual Cells):      " << srchans_check;

 //       if (srchans_check == srchans )
 //          return;                   //ALEXEY: only now we can return

 //       // if (srchans_check == srchans && !mainw->solutions_change )
 //       // {
 //       //    qDebug()<< "Exiting Rebulding Solutions ";
 //       //    return;                                 //ALEXEY: only now we can return
 //       // }
 //     }



   if ( rpSolut->nschan == 0 )
   {  // No existing Solutions protocol, so initialize a rudimentary one
      rpSolut->nuniqs     = 0;
      rpSolut->chsols.clear();
      suchans        .clear();
      nchanf              = 0;
      srchans             = sibLValue( "cells", "cpchannels" );
DbgLv(1) << "EGSo: rbS: srchans" << srchans;
      nchant              = srchans.count();
DbgLv(1) << "EGSo: rbS:  nchant" << nchant;
      return;
   }

   // Save information from any existing protocol
   QStringList sv_chans;
   QStringList sv_sols;
   QStringList sv_sids;
   QStringList sv_chcms;
   int nchan_s         = rpSolut->nschan;
DbgLv(1) << "EGSo: rbS: rpSolut->nschan: !!!!!!!!!!!!!! " << rpSolut->nschan;
   int nuniq_s         = rpSolut->nuniqs;
DbgLv(1) << "EGSo: rbS: nchan_s nuniq_s" << nchan_s << nuniq_s;

   for ( int ii = 0; ii < nchan_s; ii++ )
   {
      sv_chans  << rpSolut->chsols[ ii ].channel;
DbgLv(1) << "EGSo: rbS: SV_CHANS[sxx] !!!!!!!!!!!!!!!!: " << rpSolut->chsols[ ii ].channel;
      sv_sols   << rpSolut->chsols[ ii ].solution;
      sv_sids   << rpSolut->chsols[ ii ].sol_id;
      sv_chcms  << rpSolut->chsols[ ii ].ch_comment;
   }

   int nchanr          = srchans.count();
   rpSolut->chsols.clear();

   if ( nchans != nchanr  ||  nchanr == 0 )
   {  // Must set up total rows channels
      QStringList centps  = sibLValue( "cells", "centerpieces" );
      qDebug() << "After calling siLValue() to DETERMINE rows in SOLUTONS!!!";
      int ncused          = centps.count();
      nchans              = 0;
      srchans.clear();

      for ( int ii = 0; ii < ncused; ii++ )
      {
         QString centry      = centps[ ii ];
         int chx             = centry.indexOf( "-channel" );
         if ( chx > 0 )
         {
            QString scell       = centry.section( ":", 0, 0 )
                                        .section( " ", 1, 1 );
            QString schans( "ABCDEF" );
            int nchan           = centry.left( chx ).section( " ", -1, -1 )
                                  .simplified().toInt();
            for ( int jj = 0; jj < nchan; jj++ )
            {
               QString channel     = scell + " / " + QString( schans ).mid( jj, 1 );

               if ( (QString( schans ).mid( jj, 1 )).contains( "A" ) )                   //ALEXEY: channel lables
                  srchans << channel + ", sample [right]";
               else if ( (QString( schans ).mid( jj, 1 )).contains( "B" ) )
                  srchans << channel  + ", reference [left]";
               else
                  srchans << channel;
            }
         }
      }
      nchans              = srchans.count();
   }

   int nuniqs          = 0;
   int nschan          = 0;
   suchans.clear();
   susolus.clear();
   QStringList un_sols;

   for ( int ii = 0; ii < nchans; ii++ )
   {
      US_RunProtocol::RunProtoSolutions::ChanSolu chsol;
      QString chan        = srchans[ ii ];
      int scx             = sv_chans.indexOf( chan );
DbgLv(1) << "EGSo: rbS:  ii" << ii << "chan" << chan << "scx" << scx;

      if ( scx >= 0 )
      {
         QString solu        = sv_sols[ scx ];
         chsol.channel       = chan;
         chsol.solution      = solu;
         chsol.sol_id        = sv_sids [ scx ];
         chsol.ch_comment    = sv_chcms[ scx ];

         rpSolut->chsols << chsol;
         suchans << chan;
         susolus << solu;
         nschan++;

         if ( !un_sols.contains( solu ) )
         {
            un_sols << solu;
            nuniqs++;
         }
      }
   }
   rpSolut->nschan     = nschan;
   rpSolut->nuniqs     = nuniqs;
   nchant              = nchans;
   nchanf              = nschan;
DbgLv(1) << "EGSo:rbS:  nschan nuniqs" << nschan << nuniqs
 << "sv nschan nuniqs" << nchan_s << nuniq_s;
}

// Slot to open a dialog for managing solutions
void US_ExperGuiSolutions::manageSolutions()
{
   US_SolutionGui* mdiag = new US_SolutionGui;

   connect( mdiag, SIGNAL( newSolAdded()  ),
            this,  SLOT(   regenSolList() ) ); //ALEXEY when solution added from US_Exp, update sotution list

   mdiag->show();
}

//Update Solution List
void US_ExperGuiSolutions::regenSolList()
{
   allSolutions();
   qDebug() << "NEW SOLNAMES: " << sonames;

   for ( int ii = 0; ii < cc_solus.count(); ii++ )
   {
      QComboBox* cbsolu  = cc_solus[ ii ];
      if ( ! cbsolu->isVisible() && ! mainw->solutions_change )     // Break when invisible row reached - IF investigator not changed!!!
         break;

      // Before cleaning save currently selected text for each channel in case protocol is loaded
      QString sdescr     = cbsolu->currentText();
      QString usolu       = tr( "(unspecified)" );

      cbsolu->clear();
      cbsolu->addItems( sonames );

      if ( !sdescr.contains( usolu ) )  // Skip "(unspecified)"
         cbsolu->setCurrentText( sdescr );
   }
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

   qDebug() << "In allSolutions(), currProto->investigator 1 -- " << mainw->currProto.investigator;
   
   US_Passwd pw;
   US_DB2* dbP       = ( sibSValue( "general", "dbdisk" ) == "DB" )
                       ? new US_DB2( pw.getPasswd() ) : NULL;
//DbgLv(1) << "EGSo: allSo: dbP" << dbP;
   if ( dbP != NULL )
   {  // Read all the solutions in the database
      soids << "-1";
      QString invID     = sibSValue( "general", "investigator" )
                             .section( ":", 0, 0 ).simplified();

      qDebug() << "In allSolutions(), currProto->investigator 2 -- " << mainw->currProto.investigator;
      qDebug() << "In allSolutions(), invID -- " << invID;   //ALEXEY: when inv changed, this sibSValue does NOT capture invID !!! needs correction...

	
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
DbgLv(1) << "EGSo: allSo: sids keys --" << solu_ids.keys(); 

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

   qDebug() << "Solution name: " << sdescr;

   qDebug() << "ADD_Comment: 1";

   // Get list of channel comment component strings
   //  and compose default channel comment string

   //ALEXEY make manual_comment per channel, not per solution name
   QString row_comment =  QString::number( irow );

   qDebug() << "ADD_Comment: 1a";

   //ALEXEY: check if channel comment was read in from protocol ONCE...
   if ( !solution_comment_init[ irow ] )
     {
       qDebug() << "ADD_Comment: 1a1";

       qDebug() << "Manual comment at the beginnig: " << manual_comment[ row_comment ];

       qDebug() << "irow: " << irow << ",  rpSolut->chsols.size() " << rpSolut->chsols.size();  //ALEXEY  rpSolut->chsols.size() is ZERO: bug

       QString protocol_comment("");

       if ( rpSolut->chsols.size() > irow )
          protocol_comment += rpSolut->chsols[ irow ].ch_comment;

       qDebug() << "ADD_Comment: 1aa";

       //ALEXEY: Check if the solution descrition does not coinside with that of the protocol's comment:
       // I.e. if solution was changed from the GUI: then re-set manual_comment to empty string
       if ( !protocol_comment.contains( sdescr ) )
	 manual_comment[ row_comment ] = QString("");
       else
	 {
	   protocol_comment.replace(sdescr, "");
	   protocol_comment.remove( QRegExp("^[,\\s*]+") );
	   
	   manual_comment[ row_comment ] = protocol_comment.trimmed();  // Initialize manual comment for solution from protocol
	 }
       

       qDebug() << "Manual comment form protocol: " << manual_comment[ row_comment ];

       qDebug() << "ADD_Comment: 1aaa";

       solution_comment_init[ irow ]  = true;
     }

   qDebug() << "ADD_Comment: 2";

   commentStrings( sdescr, chcomm, comms, irow );
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

   qDebug() << "ADD_Comment: 3";

   // Complete dialog text and display the dialog
   msg         = msg
      + tr( "You may enter additional characters to append to<br/>" )
      + tr( "this text, then click on <b>OK</b>:<br/><br/>" );

   sufx        = QInputDialog::getText( this,
      tr( "Add to Experiment's Channel Comments" ),
      msg, QLineEdit::Normal, manual_comment[ row_comment ], &ok );

   if ( ok )
   {  // OK:  append suffix to channel comment
      chcomm     += ", " + sufx;
      //manual_comment[ sdescr ]     = sufx;   //QMap of manual comments per solution
      manual_comment[ row_comment ]     = sufx;   //QMap of manual comments per solution
   }
 DbgLv(1) << "EGSo:addComm:  sufx" << sufx;
 DbgLv(1) << "EGSo:addComm:   chcomm" << chcomm;

}

// Function to compose channel comment strings (string and list)
void US_ExperGuiSolutions::commentStrings( const QString solname,
                                           QString& comment, QStringList& comstrngs,
                                           const int row )
{
   US_Solution soludata;
   solutionData( solname, soludata );
   comstrngs.clear();

   // Start with solution name/description
   comstrngs << solname;               // First string (solution)
   comment        = solname;    // Beginning of channel comment

   //ALEXEY - just solution name in the comment
   // // Append buffer description
   // QString buf = soludata.buffer.description;
   // comstrngs << buf;                   // Second string (buffer)
   // comment       += buf + ", ";        // Append to channel comment

   // // Append analytes
   // int nana    = soludata.analyteInfo.count();
   // for ( int jj = 0; jj < nana; jj++ )
   // {
   //    QString ana = soludata.analyteInfo[ jj ].analyte.description;
   //    comstrngs << ana;                // Subsequent string (analyte)
   //    if ( ( jj + 1 ) < nana )
   //       comment       += ana + ", ";  // Append not-last analyte
   //    else
   //       comment       += ana;         // Append last analyte
   // }

   //ALEXEY - add manual comment per solution here
   QString row_comment =  QString::number( row );
   //if ( manual_comment.keys().contains( solname ) )
   if ( manual_comment.keys().contains( row_comment ) )
   {
      //QString mancmt =  manual_comment[solname];
      QString mancmt =  manual_comment[ row_comment ];
      if ( !mancmt.trimmed().isEmpty() )
      {
         comment       += ", " + mancmt;
         //comstrngs    <<  "," + mancmt;
      }
   }

   qDebug() << "For row: " << row << ", comments is: " << manual_comment[ row_comment ];
}


// Panel for Optical Systems parameters
US_ExperGuiOptical::US_ExperGuiOptical( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ExperimentMain*)topw;
   rpOptic             = &(mainw->currProto.rpOptic);
   mxrow               = 24;     // Maximum possible rows
   nochan              = 0;
   nuchan              = 0;
   nuvvis              = 0;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "6: Specify optical system scans for each channel" ) );
   panel->addWidget( lb_panel );


   QLabel* lb_hdr1     = us_banner( tr( "Cell / Channel" ) );
   QLabel* lb_hdr2     = us_banner( tr( "Optical System Scan Types" ) );

   QGridLayout* banners = new QGridLayout();
   int row             = 1;
   banners->addWidget( lb_hdr1, row,   0, 1, 1 );
   banners->addWidget( lb_hdr2, row++, 1, 1, 3 );

   QGridLayout* genL   = new QGridLayout();
   genL->setSpacing        ( 2 );
   genL->setContentsMargins( 2, 2, 2, 2 );

   row = 1;
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
//    // ALEXEY: old way from .conf file
//    QStringList optentr = US_Settings::defaultXpnHost();
//    int nopten       = optentr.count();
//    int nopsys       = nopten - 6;
// DbgLv(1) << "EGOp:main: optentr" << optentr;
// DbgLv(1) << "EGOp:main:  nopten" << nopten << "nopsys" << nopsys;
//    QString opsys1   = ( nopten > 6 ) ? optentr[ 6 ]
//                                      : tr( "UV/visible" );
//    QString opsys2   = ( nopten > 7 ) ? optentr[ 7 ]
//                                      : tr( "Rayleigh Interference" );
//    QString opsys3   = ( nopten > 8 ) ? optentr[ 8 ]
//                                      : tr( "(not installed)" );
// DbgLv(1) << "EGOp:main:   opsys1-3" << opsys1 << opsys2 << opsys3;

   QString opsys1   = mainw-> currentInstrument[ "opsys1" ];
   QString opsys2   = mainw-> currentInstrument[ "opsys2" ];
   QString opsys3   = mainw-> currentInstrument[ "opsys3" ];

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

   panel->addLayout(banners);
   genL->setAlignment(Qt::AlignTop);

   QScrollArea *scrollArea = new QScrollArea(this);
   QWidget *containerWidget = new QWidget;
   containerWidget->setLayout(genL);
   scrollArea->setWidgetResizable(true);
   scrollArea->setWidget(containerWidget);

   panel->addWidget(scrollArea);

   // panel->addStretch();

DbgLv(1) << "EGOp:main: call initPanel";
   initPanel();
}

// Function to rebuild the Optical protocol after Solutions change
void US_ExperGuiOptical::rebuild_Optic( void )
{


   int nchanf          = sibIValue( "solutions", "nchanf" );
   QStringList ochans  = sibLValue( "solutions", "sochannels" );
   int kochan          = ochans.count();
DbgLv(1) << "EGOp rbO: nchanf" << nchanf << "nochan" << nochan << "kochan" << kochan;

   if ( nchanf == nochan )
   {
      nochan              = ochans.count();
      int ndiff           = ( nochan == nchanf ) ? 0 : 1;
      nochan              = qMin( nochan, nchanf );

      for ( int ii = 0; ii < nochan; ii++ )
      {
         if ( rpOptic->chopts[ ii ].channel != ochans[ ii ] )
            ndiff++;
      }

      if ( ndiff == 0 )
         return;                       // No solutions change means no rebuild
   }

DbgLv(1) << "EGOp rbO: rp.nochan" << rpOptic->nochan;
   if ( rpOptic->nochan == 0 )
   {  // No existing Optic protocol, so initialize a rudimentary one
      nochan              = ochans.count();
      nchanf              = nochan;
      rpOptic->nochan     = nchanf;
      rpOptic->chopts.resize( nchanf );
      nuchan              = 0;
      nuvvis              = 0;
      QString notinst     = tr( "(not installed)" );

      for ( int ii = 0; ii < nochan; ii++ )
      {
         rpOptic->chopts[ ii ].channel = ochans[ ii ];
#if 0
         rpOptic->chopts[ ii ].scan1   = ii < 4
                                       ? cc_osyss[ ii ]->button( 1 )->text()
                                       : notinst;
         rpOptic->chopts[ ii ].scan2   = ii < 4
                                       ? cc_osyss[ ii ]->button( 2 )->text()
                                       : notinst;
         rpOptic->chopts[ ii ].scan3   = ii < 4
                                       ? cc_osyss[ ii ]->button( 3 )->text()
                                       : notinst;
#endif
#if 1
         // rpOptic->chopts[ ii ].scan1   = ii > 3
         //                               ? notinst
         //                               : ( cc_osyss[ ii ]->button( 1 )->isChecked()
         //                                 ? cc_osyss[ ii ]->button( 1 )->text()
         //                                 : "" );
	 //ALEXEY: set checked by default for UV/vis
	 rpOptic->chopts[ ii ].scan1   = cc_osyss[ ii ]->button( 1 )->text();
         rpOptic->chopts[ ii ].scan2   = ii > 3
                                       ? notinst
                                       : ( cc_osyss[ ii ]->button( 2 )->isChecked()
                                         ? cc_osyss[ ii ]->button( 2 )->text()
                                         : "" );
         rpOptic->chopts[ ii ].scan3   = ii > 3
                                       ? notinst
                                       : ( cc_osyss[ ii ]->button( 3 )->isChecked()
                                         ? cc_osyss[ ii ]->button( 3 )->text()
                                         : "" );
#endif
      }
DbgLv(1) << "EGOp rbO: nochan" << nochan << "(RUDIMENTARY)";
      return;
   }

   // Save information from any existing protocol
   int nochan_sv       = rpOptic->nochan;
   QVector< US_RunProtocol::RunProtoOptics::OpticSys > chopts_sv = rpOptic->chopts;

   nochan              = nchanf;
   rpOptic->nochan     = nchanf;
DbgLv(1) << "EGOp rbO:  nochan" << nochan << "nochan_sv" << nochan_sv;

   // Rebuild Optical protocol
   QStringList solentrs = sibLValue( "solutions", "channel_solutions" );
   int kechan          = solentrs.count();
   rpOptic->chopts.clear();
DbgLv(1) << "EGOp rbO:   solentrs count" << solentrs << kechan;

   rpOptic->chopts.resize( kechan );   //ALEXEY bug fixed
   nochan_sv = kechan;                 //ALEXEY bug fixed

   qDebug() << "#channels AFTER solutions changed : Solutions, channels: " << kechan << nochan_sv;

   for ( int ii = 0; ii < kechan; ii++ )
   {
      QString channel     = solentrs[ ii ].section( ":", 0, 0 )
                            .simplified();

      for ( int jj = 0; jj < nochan_sv; jj++ )
      {
DbgLv(1) << "EGOp rbO:    ii" << ii << "jj" << jj
 << "chsv.channel" << chopts_sv[jj].channel << "channel" << channel;
         if ( chopts_sv[ jj ].channel == channel )
         {
            //rpOptic->chopts << chopts_sv[ jj ];
            rpOptic->chopts[ ii ] = chopts_sv[ jj ]; //ALEXEY bug fixed
            break;
         }
         else                                       //ALEXEY bug fixed
         {
            rpOptic->chopts[ ii ].channel = channel;
#if 0
            rpOptic->chopts[ ii ].scan1   = cc_osyss[ ii ]->button( 1 )->text();
            rpOptic->chopts[ ii ].scan2   = cc_osyss[ ii ]->button( 2 )->text();
            rpOptic->chopts[ ii ].scan3   = cc_osyss[ ii ]->button( 3 )->text();
#endif
#if 1
            // rpOptic->chopts[ ii ].scan1   = cc_osyss[ ii ]->button( 1 )->isChecked()
            //                               ? cc_osyss[ ii ]->button( 1 )->text()
            //                               : "";
	    //ALEXEY: set checked by default for UV/vis
	    rpOptic->chopts[ ii ].scan1   = cc_osyss[ ii ]->button( 1 )->text();
            rpOptic->chopts[ ii ].scan2   = cc_osyss[ ii ]->button( 2 )->isChecked()
                                          ? cc_osyss[ ii ]->button( 2 )->text()
                                          : "";
            rpOptic->chopts[ ii ].scan3   = cc_osyss[ ii ]->button( 3 )->isChecked()
                                          ? cc_osyss[ ii ]->button( 3 )->text()
                                          : "";
DbgLv(1) << "EGOp rbO:    scan1" << rpOptic->chopts[ ii ].scan1
 << "CKD?" << cc_osyss[ ii ]->button( 1 )->isChecked();
#endif
            break;
         }
      }
   }
   rpOptic->nochan     = rpOptic->chopts.count();
DbgLv(1) << "EGOp rbO: rpO.nochan" << rpOptic->nochan;
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

   // if ( optype != tr( "Rayleigh Interference" ) )  //ALEXEY: couple checkboxes for all optics
   //     return;

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


// Panel for Analysis Profile parameters to Optima DB
US_ExperGuiAProfile::US_ExperGuiAProfile( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ExperimentMain*)topw;
   rpRotor             = &(mainw->currProto.rpRotor);
   rpSpeed             = &(mainw->currProto.rpSpeed);
   rpCells             = &(mainw->currProto.rpCells);
   rpSolut             = &(mainw->currProto.rpSolut);
   rpOptic             = &(mainw->currProto.rpOptic);
   rpRange             = &(mainw->currProto.rpRange);
   rpAprof             = &(mainw->currProto.rpAprof);
   rpSubmt             = &(mainw->currProto.rpSubmt);
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "8: Review an Analysis"
                                        " Profile" ) );
   panel->addWidget( lb_panel );
   genL                = new QGridLayout();
   panel->addLayout( genL );
   panel->addStretch();

   // Embed AnalysisProfile object in panel
   sdiag               = new US_AnalysisProfileGui;
    
   sdiag->setParent( this, Qt::Widget );
   int offset          =  lb_panel->height() + 4;
   sdiag->move( 0, offset );
   sdiag->setFrameShape( QFrame::Box );
   sdiag->setLineWidth( 1 );
   sdiag->auto_mode_passed();
   QString protoname   = mainw->currProto.protoname;
   if ( protoname.isEmpty() )
      protoname           = mainw->loadProto.protoname;
   QString aprofname   = rpAprof->aprofname;
   if ( aprofname.isEmpty() )
      aprofname           = protoname;
   sdiag->inherit_protocol( &mainw->currProto );
   sdiag->auto_name_passed( protoname, aprofname );
   sdiag->currProf.protoGUID
                       = mainw->currProto.protoGUID;
   sdiag->currProf.protoID
                       = mainw->currProto.protoID;
   mainw->currAProf    = sdiag->currProf;

   qDebug() << "EXP_APROFILE_GUI -- FIRST intitialization: sdiag->currProf.aprofname -- " << sdiag->currProf.aprofname;
   sdiag->show();
}

//Resize AnalysisProfile properly
void US_ExperGuiAProfile::resizeEvent( QResizeEvent *event )
{
   int upper_height = mainw->tabHeight +  mainw->buttLHeight * 3 - 8;
   int new_main_w   = mainw->width();
   int new_main_h   = mainw->height() - upper_height;

   if ( new_main_w > sdiag->width() || new_main_h > sdiag->height())
   {
      int newWidth   = qMax( new_main_w, sdiag->width() );
      int newHeight  = qMax( new_main_h, sdiag->height() );
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize( newWidth, newHeight ) );
      update();
   }

   if ( new_main_w < sdiag->width() ||  new_main_h < sdiag->height() )
   {
      int newWidth   = qMin( new_main_w, sdiag->width() );
      int newHeight  = qMin( new_main_h, sdiag->height() );
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize( newWidth, newHeight ) );
      update();
   }

   QWidget::resizeEvent(event);
}



// Slot to show details of an analysis profile
void US_ExperGuiAProfile::detailProfile()
{
   // Create a new editor text dialog with fixed font
   US_Editor* ediag = new US_Editor( US_Editor::DEFAULT, true, "", this );
   ediag->setWindowTitle( tr( "Details on Analysis Profile" ) );
   ediag->resize( 720, 440 );
   ediag->e->setFont( QFont( US_Widgets::fixedFont().family(),
                             US_GuiSettings::fontSize() - 1,
                             QFont::Bold ) );
QFont ufont=ediag->e->font();
DbgLv(1) << "EGAp:detE: ufont" << ufont.family();
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
}


// Panel for Uploading parameters to Optima DB
US_ExperGuiUpload::US_ExperGuiUpload( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ExperimentMain*)topw;
   rpRotor             = &(mainw->currProto.rpRotor);
   rpSpeed             = &(mainw->currProto.rpSpeed);
   rpCells             = &(mainw->currProto.rpCells);
   rpSolut             = &(mainw->currProto.rpSolut);
   rpOptic             = &(mainw->currProto.rpOptic);
   rpRange             = &(mainw->currProto.rpRange);
   rpSubmt             = &(mainw->currProto.rpSubmt);
   submitted           = false;
   rps_differ          = true;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "9: Submit an Experiment"
                                        " to the Optima" ) );
   panel->addWidget( lb_panel );
   //QGridLayout* genL   = new QGridLayout();
   genL   = new QGridLayout();

   // Push buttons
   //QPushButton* pb_details  = us_pushbutton( tr( "View Experiment Details" ) );
   //QPushButton* pb_connect  = us_pushbutton( tr( "Test Connection" ) );
   pb_details  = us_pushbutton( tr( "View Experiment Details" ) );
   pb_connect  = us_pushbutton( tr( "Test Connection" ) );
   pb_submit   = us_pushbutton( tr( "Submit the Run"  ) );
   pb_saverp   = us_pushbutton( tr( "Save the Protocol" ) );

   pb_submit->setEnabled( false );                                  // <-- Temporary enabled for testing

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
   QLayout* lo_ranges       = us_checkbox( tr( "Ranges" ),
                                           ck_ranges,   false );
   QLayout* lo_connect      = us_checkbox( tr( "Connected to Optima" ),
                                           ck_connect,  false );
   QLayout* lo_rp_diff      = us_checkbox( tr( "loaded/default Run Protocol"
                                               " differs from the current"
                                               " Run Protocol" ),
                                           ck_rp_diff,  false );
   QLayout* lo_prot_ena     = us_checkbox( tr( "Protocol can be Saved" ),
                                           ck_prot_ena, false );
   QLayout* lo_prot_svd     = us_checkbox( tr( "Protocol Saved" ),
                                           ck_prot_svd, false );
   QLayout* lo_sub_enab     = us_checkbox( tr( "Submit Enabled" ),
                                           ck_sub_enab, false );
   QLayout* lo_sub_done     = us_checkbox( tr( "Submit Completed" ),
                                           ck_sub_done, false );
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
   ck_ranges  ->setEnabled( false );
   ck_connect ->setEnabled( false );
   ck_rp_diff ->setEnabled( false );
   ck_prot_ena->setEnabled( false );
   ck_prot_svd->setEnabled( false );
   ck_sub_enab->setEnabled( false );
   ck_sub_done->setEnabled( false );

   // Build the layout
   int row             = 1;

   genL->addWidget( pb_details,      row,   0, 1, 2 );
   genL->addWidget( pb_connect,      row,   2, 1, 2 );
   genL->addWidget( pb_saverp,       row,   4, 1, 2 );
   genL->addWidget( pb_submit,       row++, 6, 1, 2 );

   genL->addLayout( lo_run,          row,   1, 1, 3 );
   genL->addLayout( lo_project,      row++, 4, 1, 3 );
   genL->addLayout( lo_rotor,        row,   1, 1, 3 );
   genL->addLayout( lo_rotor_ok,     row++, 4, 1, 3 );
   genL->addLayout( lo_speed,        row,   1, 1, 3 );
   genL->addLayout( lo_speed_ok,     row++, 4, 1, 3 );
   genL->addLayout( lo_centerp,      row++, 1, 1, 3 );
   genL->addLayout( lo_solution,     row,   1, 1, 3 );
   genL->addLayout( lo_optical,      row++, 4, 1, 3 );
   genL->addLayout( lo_ranges,       row,   1, 1, 3 );
   genL->addLayout( lo_connect,      row++, 4, 1, 3 );
   genL->addLayout( lo_rp_diff,      row++, 1, 1, 6 );
   genL->addLayout( lo_prot_ena,     row,   1, 1, 3 );
   genL->addLayout( lo_prot_svd,     row++, 4, 1, 3 );
   genL->addLayout( lo_sub_enab,     row,   1, 1, 3 );
   genL->addLayout( lo_sub_done,     row++, 4, 1, 3 );

   // Connect to slots
   connect( pb_details,   SIGNAL( clicked()          ),
            this,         SLOT  ( detailExperiment() ) );
   connect( pb_connect,   SIGNAL( clicked()          ),
            this,         SLOT  ( testConnection()   ) );
   connect( pb_saverp,    SIGNAL( clicked()          ),
            this,         SLOT  ( saveRunProtocol()  ) );
   connect( pb_submit,    SIGNAL( clicked()          ),
            this,         SLOT  ( submitExperiment_confirm() ) );
   // connect( pb_submit,    SIGNAL( clicked()          ),
   //          this,         SLOT  ( submitExperiment() ) );

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
   have_range          = false;
   have_sol            = false;
   rps_differ          = false;
   proto_ena           = false;
   proto_svd           = false;
   subm_enab           = false;
   submitted           = false;
   connected           = false;

 //   // ALEXEY: old way
 //    // Connect to the Optima if possible
 //    QStringList dblist  = US_Settings::defaultXpnHost();
 //    int ndble           = dblist.count();

 //    if ( ndble < 9 )
 //    {  // Handle a default entry without exactly 9 parts
 //       QStringList dblold  = dblist;
 //       dblist.clear();

 //       dblist << "test_connect";
 //       dblist << "bcf.uthscsa.edu";
 //       dblist << "5432";
 //       dblist << "AUC_DATA_DB";
 //       dblist << "auc_admin";
 //       dblist << "encpasswd";
 //       dblist << tr( "UV/visible" );
 //       dblist << tr( "Rayleigh Interference" );
 //       dblist << tr( "(not installed)" );

 //       for ( int ii = 0; ii < ndble; ii++ )
 //       {
 //          dblist.replace( ii, dblold[ ii ] );
 //       }
 // DbgLv(1) << "EGUp:main:   opsys1-3" << dblist[6] << dblist[7] << dblist[8];

 //       dblist << dblist[ 4 ];

 //       US_Settings::set_def_xpn_host( dblist );
 //    }

 //    US_Passwd pw;
 //    QString xpnhost     = dblist[ 1 ];
 //    int     xpnport     = dblist[ 2 ].toInt();
 //    QString dbname      = dblist[ 3 ];
 //    QString dbuser      = dblist[ 4 ];
 //    QString epasw       = dblist[ 5 ];
 //    QString epasw0      = epasw.section( "^", 0, 0 );
 //    QString epasw1      = epasw.section( "^", 1, 1 );
 //    QString dbpasw      = US_Crypto::decrypt( epasw0, pw.getPasswd(), epasw1 );

   //ALEXEY: new way
   QString xpnhost      = mainw->currentInstrument[ "optimaHost" ];
   int     xpnport      = mainw->currentInstrument[ "optimaPort" ].toInt();
   QString dbname       = mainw->currentInstrument[ "optimaDBname" ];
   QString dbuser       = mainw->currentInstrument[ "optimaDBusername" ];
   QString dbpasw       = mainw->currentInstrument[ "optimaDBpassw" ];

   QString opsys1       = mainw->currentInstrument[ "opsys1" ];
   QString opsys2       = mainw->currentInstrument[ "opsys2" ];
   QString opsys3       = mainw->currentInstrument[ "opsys3" ];
DbgLv(1) << "EGUp:main:   opsys1-3" << opsys1 << opsys2 << opsys3;
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
   QString v_prot    = mainw->currProto.protoname;
   QString v_ohost   = mainw->currProto.optimahost;
   double  d_temper  = mainw->currProto.temperature;
   QString v_temper  = QString::number( d_temper );
   QString v_lab     = rpRotor->laboratory;
   QString v_rotor   = rpRotor->rotor;
   QString v_calib   = rpRotor->calibration;
   QString v_operID  = QString::number(rpRotor->operID);
   QString v_instID  = QString::number(rpRotor->instID);
   QString v_exptype = rpRotor->exptype;

   int     i_centp   = rpCells->nused;
   QString v_centp   = QString::number( i_centp  );
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
   bool chk_centerp  = ( i_centp > 0 );
   bool chk_solution = ( sibIValue( "solutions", "alldone" ) > 0 );
   bool chk_ranges   = ( sibIValue( "ranges",    "alldone" ) > 0 );
   bool chk_vars_set = ( chk_run       &&  chk_project   &&
                         chk_centerp   &&  chk_solution  &&
                         chk_ranges );
   bool chk_sub_enab = ( chk_vars_set  &&  connected );
   bool chk_sub_done = submitted;

   QString s_Yes     = tr( "YES" );
   QString s_no      = tr( "no" );
   QString v_genok   = ( chk_run && chk_project ) ? s_Yes : s_no; ;
   QString v_rotok   = s_Yes;
   QString v_rotuc   = chk_rotor_ok ? s_Yes : s_no;
   QString v_speok   = s_Yes;
   QString v_speuc   = chk_speed_ok ? s_Yes : s_no;
   QString v_celok   = chk_centerp  ? s_Yes : s_no;
   QString v_solok   = chk_solution ? s_Yes : s_no;
   QString v_phook   = chk_ranges   ? s_Yes : s_no;
   QString v_conok   = connected    ? s_Yes : s_no;
   QString v_sveok   = proto_ena    ? s_Yes : s_no;
   QString v_svcok   = proto_svd    ? s_Yes : s_no;
   QString v_uleok   = chk_sub_enab ? s_Yes : s_no;
   QString v_ulcok   = chk_sub_done ? s_Yes : s_no;

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
   dtext += tr( "  InstrumentID:               " ) + v_instID + "\n";
   dtext += tr( "  OperatorID:                 " ) + v_operID + "\n";
   dtext += tr( "  Experiment Type:            " ) + v_exptype + "\n";

   dtext += tr( "\nSpeeds\n" );
   dtext += tr( "  ALL SPECIFIED:              " ) + v_speok  + "\n";
   dtext += tr( "  USER CHANGES:               " ) + v_speuc  + "\n";
   dtext += tr( "  Number Speed Steps:         " ) + v_nspeed + "\n";

   int nspeed        = v_nspeed.toInt();
DbgLv(1) << "EGUp:dE: nspeed" << nspeed;
   int jj            = 0;
   for ( int ii = 0; ii < nspeed; ii++ )
   {
      dtext += tr( "    Step %1 :\n" ).arg( ii + 1 );
      dtext += tr( "      Speed:         " ) + sspeed[ jj++ ] + "\n";
      dtext += tr( "      Acceleration:  " ) + sspeed[ jj++ ] + "\n";
      dtext += tr( "      Duration:      " ) + sspeed[ jj++ ] + "\n";
      dtext += tr( "      Delay (accel.):" ) + sspeed[ jj++ ] + "\n";
      dtext += tr( "      Stage Delay:   " ) + sspeed[ jj++ ] + "\n";
      dtext += tr( "      Scan Interval: " ) + sspeed[ jj++ ] + "\n";

   }

   dtext += tr( "\nCells\n" );
   dtext += tr( "  ALL SPECIFIED:              " ) + v_celok  + "\n";
   dtext += tr( "  Specified Centerpieces:     " ) + v_centp  + "\n";

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

   dtext += tr( "\nUsed Channel Ranges\n" );
   dtext += tr( "  ALL SPECIFIED:              " ) + v_phook  + "\n";
   dtext += tr( "  Number Channels Used:        %1\n" )
            .arg( rpRange->nranges );
   for ( int ii = 0; ii < rpRange->nranges; ii++ )
   {
      QString channel = rpRange->chrngs[ ii ].channel;
      int    nwavl    = rpRange->chrngs[ ii ].wvlens.count();
      double lo_wavl  = rpRange->chrngs[ ii ].wvlens[ 0 ];
      double hi_wavl  = rpRange->chrngs[ ii ].wvlens[ nwavl - 1 ];
      double lo_radi  = rpRange->chrngs[ ii ].lo_rad;
      double hi_radi  = rpRange->chrngs[ ii ].hi_rad;
      dtext += tr( "  Channel " ) + channel + " : \n";
      dtext += tr( "    wavelength count  : %1\n" ).arg( nwavl );
      dtext += tr( "    wavelength range  : %1  to  %2\n" )
               .arg( lo_wavl ).arg( hi_wavl );
      dtext += tr( "    radius range      : %1  to  %2\n" )
               .arg( lo_radi ).arg( hi_radi );
   }

   //ALEXEY: below -- when Interference present, there is inconsistency btw. :
   //           (1)  cAP->pchans.count(); -- includes ALL channels (B-Interf.)
   //           (2)  cAP2->parms.size()   -- 2DSA structures do NOT include (B-Interf.)
   //           (3)  cAP2p>parms.size()   -- PCSA structures do NOT include (B-Interf.)
   
   qDebug() << "AProfile: 1";   
   US_AnaProfile* cAP  = &mainw->currAProf;
   US_AnaProfile::AnaProf2DSA* cAP2 = &cAP->ap2DSA;
   US_AnaProfile::AnaProfPCSA* cAPp = &cAP->apPCSA;

   qDebug() << "AProfile: 2";
   
   QString aprname     = cAP->aprofname;
   QString proname     = cAP->protoname;
   int nchna           = cAP->pchans.count();

   //ALEXEY: separate params for these (nchna may not be equal to nchna_2dsa && nchna_pcsa)
   int nchna_2dsa       = cAP2->parms.size();
   int nchna_pcsa       = cAPp->parms.size();   

   qDebug() << "AProfile: 3";
   
   QString scrat       = QString::number( cAP->lc_ratios[ 0 ] );
   QString sctol       = QString::number( cAP->lc_tolers[ 0 ] );
   QString slvol       = QString::number( cAP->l_volumes[ 0 ] );
   QString svtol       = QString::number( cAP->lv_tolers[ 0 ] );
   QString sdaen       = QString::number( cAP->data_ends[ 0 ] );

   qDebug() << "AProfile: 4";
   
   dtext += tr( "\nAnalysis Profile\n" );
   dtext += tr( "  Profile Name:   " ) + cAP->aprofname  + "\n";
   dtext += tr( "  Protocol Name:  " ) + cAP->protoname  + "\n";
   dtext += tr( "  Profile GUID:   " ) + cAP->aprofGUID  + "\n";
   dtext += tr( "  Protocol GUID:  " ) + cAP->protoGUID  + "\n";
   dtext += tr( "  Profile db ID:  " ) +
               QString::number( cAP->aprofID )  + "\n";
   dtext += tr( "  Protocol db ID: " ) +
               QString::number( cAP->protoID )  + "\n";
   dtext += tr( "  Channel:        " ) + cAP->chndescs [ 0 ]  + "\n";
   dtext += tr( "    Loading Ratio:            " ) + scrat + "\n";
   dtext += tr( "    Ratio Tolerance percent:  " ) + sctol + "\n";
   dtext += tr( "    Loading Volume:           " ) + slvol + "\n";
   dtext += tr( "    Volume Tolerance percent: " ) + svtol + "\n";
   dtext += tr( "    Data End (cm):            " ) + sdaen + "\n";

   qDebug() << "AProfile: 5";
   
   for ( int ii = 1; ii < nchna; ii++ )
   {
     //ALEXEY:: should we skip B-Interf. here ? (like for 2DSA && PCSA )
      dtext += tr( "  Channel:        " ) + cAP->chndescs [ ii ]  + "\n";
      int nsame           = 0;

      if ( cAP->lc_ratios[ ii ] == cAP->lc_ratios[ ii - 1 ] )
         nsame++;
      else
         dtext += tr( "    Loading Ratio:            " ) +
            QString::number( cAP->lc_ratios[ ii ] ) + "\n";

      if ( cAP->lc_tolers[ ii ] == cAP->lc_tolers[ ii - 1 ] )
         nsame++;
      else
         dtext += tr( "    Ratio Tolerance percent:  " ) +
            QString::number( cAP->lc_tolers[ ii ] ) + "\n";

      if ( cAP->l_volumes[ ii ] == cAP->l_volumes[ ii - 1 ] )
         nsame++;
      else
         dtext += tr( "    Loading Volume:           " ) +
            QString::number( cAP->l_volumes[ ii ] ) + "\n";

      if ( cAP->lv_tolers[ ii ] == cAP->lv_tolers[ ii - 1 ] )
         nsame++;
      else
         dtext += tr( "    Volume Tolerance percent: " ) +
            QString::number( cAP->lv_tolers[ ii ] ) + "\n";

      if ( cAP->data_ends[ ii ] == cAP->data_ends[ ii - 1 ] )
         nsame++;
      else
         dtext += tr( "    Data End (cm):            " ) +
            QString::number( cAP->data_ends[ ii ] ) + "\n";

      if ( nsame == 5 )
         dtext += tr( "      [ all values same as previous ]\n" );
      else if ( nsame > 0 )
         dtext += tr( "      [ other values same as previous ]\n" );
   }

   qDebug() << "AProfile: 6";         //ALEXEY: here it crashes
   qDebug() << "nchna: -- " << nchna;
   qDebug() << "cAP2->parms.size() -- " << cAP2->parms.size();
   
   dtext += tr( "  2DSA Analysis Controls: \n" );
   //   for ( int ii = 0; ii < nchna; ii++ )
   for ( int ii = 0; ii < nchna_2dsa; ii++ )
     {

     qDebug() << "Inside 2DSA controls: ii -- " << ii << cAP2->parms[ ii ].channel;
     
      dtext += tr( "    Channel:          " ) + cAP2->parms[ ii ].channel  + "\n";
      dtext += tr( "      s Min, Max, Points:     " ) +
               QString::number( cAP2->parms[ ii ].s_min ) + ", " +
               QString::number( cAP2->parms[ ii ].s_max ) + ", " +
               QString::number( cAP2->parms[ ii ].s_grpts ) + "\n";
      dtext += tr( "      f/f0 Min, Max, Points:  " ) +
               QString::number( cAP2->parms[ ii ].k_min ) + ", " +
               QString::number( cAP2->parms[ ii ].k_max ) + ", " +
               QString::number( cAP2->parms[ ii ].k_grpts ) + "\n";
      dtext += tr( "      Grid Repetitions:       " ) +
               QString::number( cAP2->parms[ ii ].gridreps ) + "\n";
      dtext += tr( "      Custom Grid:            " ) +
               cAP2->parms[ ii ].cgrid_name + "\n";
      dtext += tr( "      Varying Vbar:           " ) +
               ( cAP2->parms[ ii ].varyvbar ? tr( "YES" ) : tr( "no" ) ) + "\n";
      dtext += tr( "      Constant f/f0:          " ) +
               QString::number( cAP2->parms[ ii ].ff0_const ) + "\n";
   }
   dtext += tr( "    Flow Summary:\n" );
   dtext += tr( "     \"2DSA  (TI Noise)\" --\n" );
   dtext += tr( "      Run?                 :  " ) +
            ( cAP2->job1run ? tr( "YES" ) : tr( "no" ) ) + "\n";
   dtext += tr( "     \"2DSA-FM  (TI+RI Noise)\" --\n" );
   dtext += tr( "      Run?                 :  " ) +
            ( cAP2->job2run ? tr( "YES" ) : tr( "no" ) ) + "\n";
   dtext += tr( "      Meniscus Fit Points:    " ) +
            QString::number( cAP2->grpoints ) + "\n";
   dtext += tr( "      Meniscus Fit Range:     " ) +
            QString::number( cAP2->fitrng ) + "\n";
   dtext += tr( "     \"FITMEN\" --\n" );
   dtext += tr( "      Run?                 :  " ) +
            ( cAP2->job3run ? tr( "YES" ) : tr( "no" ) ) + "\n";
   dtext += tr( "      Auto-pick?           :  " ) +
            ( cAP2->job3auto ? tr( "YES" ) : tr( "no" ) ) + "\n";
   dtext += tr( "     \"2DSA-IT  (TI+RI Noise)\" --\n" );
   dtext += tr( "      Run?                 :  " ) +
            ( cAP2->job4run ? tr( "YES" ) : tr( "no" ) ) + "\n";
   dtext += tr( "      Refinement Iterations:  " ) +
            QString::number( cAP2->rfiters ) + "\n";
   dtext += tr( "     \"2DSA-MC\" --\n" );
   dtext += tr( "      Run?                 :  " ) +
            ( cAP2->job5run ? tr( "YES" ) : tr( "no" ) ) + "\n";
   dtext += tr( "      MonteCarlo Iterations:  " ) +
            QString::number( cAP2->mciters ) + "\n";

   
   qDebug() << "AProfile: 7";
   qDebug() << "nchna: -- " << nchna;
   qDebug() << "cAPp->parms.size() -- " << cAPp->parms.size();
   
   dtext += tr( "  PCSA Analysis Controls: \n" );
   if ( cAPp->job_run )
   {
      dtext += tr( "    Run PCSA job?            YES\n" );
      //      for ( int ii = 0; ii < nchna; ii++ )
      for ( int ii = 0; ii < nchna_pcsa; ii++ )
      {

	 qDebug() << "Inside PCSA controls: ii -- " << ii << cAPp->parms[ ii ].channel;
	
         dtext += tr( "    Channel:          " ) + cAPp->parms[ ii ].channel  + "\n";
         dtext += tr( "      Curve Type:              " ) +
                  cAPp->parms[ ii ].curv_type + "\n";
         dtext += tr( "      X Type, Min, Max:        " ) +
                  cAPp->parms[ ii ].x_type + ", " +
                  QString::number( cAPp->parms[ ii ].x_min ) + ", " +
                  QString::number( cAPp->parms[ ii ].x_max ) + "\n";
         dtext += tr( "      Y Type, Min, Max:        " ) +
                  cAPp->parms[ ii ].y_type + ", " +
                  QString::number( cAPp->parms[ ii ].y_min ) + ", " +
                  QString::number( cAPp->parms[ ii ].y_max ) + "\n";
         dtext += tr( "      Z Type, Value:           " ) +
                  cAPp->parms[ ii ].z_type + ", " +
                  QString::number( cAPp->parms[ ii ].z_value ) + "\n";
         dtext += tr( "      Variations Count:        " ) +
                  QString::number( cAPp->parms[ ii ].varcount ) + "\n";
         dtext += tr( "      Grid Fit Iterations:     " ) +
                  QString::number( cAPp->parms[ ii ].grf_iters ) + "\n";
         dtext += tr( "      Curve Resolution Points: " ) +
                  QString::number( cAPp->parms[ ii ].creso_pts ) + "\n";
         dtext += tr( "      Noise Type:              " ) +
                  cAPp->parms[ ii ].noise_type + "\n";
         dtext += tr( "      Tikhonov Regularization: " ) +
                  cAPp->parms[ ii ].treg_type + "\n";
         dtext += tr( "      Tikhonov Alpha:          " ) +
                  QString::number( cAPp->parms[ ii ].tr_alpha ) + "\n";
         dtext += tr( "      MonteCarlo Iterations:   " ) +
                  QString::number( cAPp->parms[ ii ].mc_iters ) + "\n";
      }
   }
   else
   {
      dtext += tr( "    Run PCSA job?            no\n" );
   }

   qDebug() << "AProfile: 8";
/**
x               double       x_min;
x                 double       x_max;
x                 double       y_min;
x                 double       y_max;
x                 double       z_value;
x                 double       tr_alpha;
x                 int          varcount;
x                 int          grf_iters;
x                 int          creso_pts;
x                 int          noise_flag;
x                 int          treg_flag;
x                 int          mc_iters;
x                 bool         job_run;
x                 QString      channel;
x                 QString      curv_type;
x                 QString      x_type;
x                 QString      y_type;
x                 QString      z_type;
x                 QString      noise_type;
x                 QString      treg_type;
 **/

   dtext += tr( "\nSubmit\n" );
   dtext += tr( "  CONNECTED:                  " ) + v_conok + "\n";
   dtext += tr( "  PROTOCOL CAN BE SAVED:      " ) + v_sveok + "\n";
   dtext += tr( "  PROTOCOL SAVED:             " ) + v_svcok + "\n";
   dtext += tr( "  SUBMIT ENABLED:             " ) + v_uleok + "\n";
   dtext += tr( "  SUBMIT COMPLETED:           " ) + v_ulcok + "\n";

   qDebug() << "AProfile: 9";

//    // Generate a JSON stream to be uploaded

//    json_upl          = buildJson();

//    // Report on it
//    dtext += tr( "\n\nJSON stream generated and available for upload --\n\n" );
//    if ( json_upl.isEmpty() )
//    {
//       dtext += tr( "NONE  (parameterization is incomplete).\n" );
//    }
//    else
//    {
//       dtext += json_upl + "\n";
//    }
// dtext+= "\n** NOT YET FULLY IMPLEMENTED **\n";

   // Load text and show the dialog
   QApplication::restoreOverrideCursor();
   qApp->processEvents();

   ediag->e->setText( dtext );
   ediag->show();
}


// Slot to test XPN connection and reset the connection flag
void US_ExperGuiUpload::testConnection()
{
  // // ALEXEY: old way
  //     US_Passwd pw;
  //     QStringList dblist  = US_Settings::defaultXpnHost();
  //     QString xpnhost     = dblist[ 1 ];
  //     int     xpnport     = dblist[ 2 ].toInt();
  //     QString dbname      = dblist[ 3 ];
  //     QString dbuser      = dblist[ 4 ];
  //     QString epasw       = dblist[ 5 ];
  //     QString epasw0      = epasw.section( "^", 0, 0 );
  //     QString epasw1      = epasw.section( "^", 1, 1 );
  //     QString dbpasw      = US_Crypto::decrypt( epasw0, pw.getPasswd(), epasw1 );
  //  DbgLv(1) << "EGUp: host port name user pasw" << xpnhost << xpnport
  //   << dbname << dbuser << epasw;

   //ALEXEY: new way
    QString name        = mainw->currentInstrument[ "name" ];
    QString xpnhost     = mainw->currentInstrument[ "optimaHost" ];
    int     xpnport     = mainw->currentInstrument[ "optimaPort" ].toInt();
    QString dbname      = mainw->currentInstrument[ "optimaDBname" ];
    QString dbuser      = mainw->currentInstrument[ "optimaDBusername" ];
    QString dbpasw      = mainw->currentInstrument[ "optimaDBpassw" ];

    qDebug() << "Optima in use: name, host, port, dbname, dbuser, dbpasw: " << name << " " << xpnhost << " "
             << xpnport << " "  << dbname << " " << dbuser << " " << dbpasw ;

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

// Slot to save the current Reports && fill out QMaps relating chdesc-to-reportID
void US_ExperGuiUpload::saveReports( US_AnaProfile* aprof )
{
  qDebug() << "Saving REPORTS !!!";

  //Clear ch_desc-to-ids/guid maps
  aprof->ch_report_guids.clear();
  aprof->ch_report_ids.clear();
  

  //Iterate over Reports 
  //QMap<QString, US_ReportGMP>::iterator ri;

  //ALEXEY_NEW_REPORT:
  QMap< QString, QMap < QString, US_ReportGMP > >::iterator ri;
  
  for ( ri = aprof->ch_reports.begin(); ri != aprof->ch_reports.end(); ++ri )
    {
      //ALEXEY_NEW_REPORT:
      // now will be a cycle over channel's wvls:
      // ri.value() IS a QMap < QString( wvl ), US_ReportGMP >
      // iterate over this QMap's keys() == wavelengths
      // aprof->ch_report_ids[ ri.key() ][ i ] = reportID;
      // //Generate new Report GuiID:
      // aprof->ch_report_guids [ ri.key() ] = US_Util::new_guid();

      QString chan_desc = ri.key();
      QList< int > ch_reportIDs;
      QStringList  ch_reportGUIDs;
      
      QMap < QString, US_ReportGMP > triple_reports = ri.value();
      QMap < QString, US_ReportGMP >::iterator tri;
      for ( tri = triple_reports.begin(); tri != triple_reports.end(); ++tri )
	{
	  QString curr_guid = US_Util::new_guid();
	  ch_reportGUIDs << curr_guid;

	  //Return new Report ID from DB insertion:
	  int reportID = writeReportToDB( curr_guid, tri.value() );
	  ch_reportIDs.push_back( reportID );
	  
	  qDebug() << "Report ID/GUID: triple -- "
		   << ri.key() << ": " << tri.key()
		   << " -- " << reportID << " / "
		   << curr_guid;
	}

      //now, insert reportIDs && reportGUIDs for current channel:
      aprof->ch_report_guids [ chan_desc ] = ch_reportGUIDs;
      aprof->ch_report_ids   [ chan_desc ] = ch_reportIDs;
    }
}

//Write a single  Report to DB
int US_ExperGuiUpload::writeReportToDB( QString reportGUID, US_ReportGMP report )
{
  qDebug() << "Writing Report " << report.channel_name;
  int reportID = 0;
  
  US_DB2* dbP  = NULL;
  US_Passwd pw;
  dbP            = new US_DB2( pw.getPasswd() );
  if ( dbP->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Saving Report: Could not connect to database: \n" ) + dbP->lastError() );
      return reportID;
    }

  //Copmose JSON string for report Mask first:
  QString jsonMask = QString("{");

  int tot_conc_bool = report.tot_conc_mask ? 1 : 0;
  jsonMask += QString("\"tot_conc\":\"") + QString::number( tot_conc_bool ) + QString("\",");

  int rmsd_lim_bool = report.rmsd_limit_mask ? 1 : 0;
  jsonMask += QString("\"rmsd_lim\":\"") + QString::number( rmsd_lim_bool ) + QString("\",");

  int intensity_bool = report.av_intensity_mask ? 1 : 0;
  jsonMask += QString("\"intensity\":\"") + QString::number( intensity_bool ) + QString("\",");
  
  int exp_dur_bool = report.experiment_duration_mask ? 1 : 0;
  jsonMask += QString("\"exp_dur\":\"") + QString::number( exp_dur_bool ) + QString("\",");

  int integration_bool = report.integration_results_mask ? 1 : 0;
  jsonMask += QString("\"integration\":\"") + QString::number( integration_bool ) + QString("\",");

  int plots_bool = report.plots_mask ? 1 : 0;
  jsonMask += QString("\"plots\":\"") + QString::number( plots_bool ) + QString("\",");

  //pseudo 3D plots
  int pseudo3d_2dsait_s_ff0_bool = report.pseudo3d_2dsait_s_ff0 ? 1 : 0;
  jsonMask += QString("\"pseudo3d_2dsait_s_ff0\":\"") + QString::number( pseudo3d_2dsait_s_ff0_bool ) + QString("\",");

  int pseudo3d_2dsait_s_d_bool = report.pseudo3d_2dsait_s_d ? 1 : 0;
  jsonMask += QString("\"pseudo3d_2dsait_s_d\":\"") + QString::number( pseudo3d_2dsait_s_d_bool ) + QString("\",");

  int pseudo3d_2dsait_mw_ff0_bool = report.pseudo3d_2dsait_mw_ff0 ? 1 : 0;
  jsonMask += QString("\"pseudo3d_2dsait_mw_ff0\":\"") + QString::number( pseudo3d_2dsait_mw_ff0_bool ) + QString("\",");

  int pseudo3d_2dsait_mw_d_bool = report.pseudo3d_2dsait_mw_d ? 1 : 0;
  jsonMask += QString("\"pseudo3d_2dsait_mw_d\":\"") + QString::number( pseudo3d_2dsait_mw_d_bool ) + QString("\",");


  int pseudo3d_2dsamc_s_ff0_bool = report.pseudo3d_2dsamc_s_ff0 ? 1 : 0;
  jsonMask += QString("\"pseudo3d_2dsamc_s_ff0\":\"") + QString::number( pseudo3d_2dsamc_s_ff0_bool ) + QString("\",");

  int pseudo3d_2dsamc_s_d_bool = report.pseudo3d_2dsamc_s_d ? 1 : 0;
  jsonMask += QString("\"pseudo3d_2dsamc_s_d\":\"") + QString::number( pseudo3d_2dsamc_s_d_bool ) + QString("\",");

  int pseudo3d_2dsamc_mw_ff0_bool = report.pseudo3d_2dsamc_mw_ff0 ? 1 : 0;
  jsonMask += QString("\"pseudo3d_2dsamc_mw_ff0\":\"") + QString::number( pseudo3d_2dsamc_mw_ff0_bool ) + QString("\",");

  int pseudo3d_2dsamc_mw_d_bool = report.pseudo3d_2dsamc_mw_d ? 1 : 0;
  jsonMask += QString("\"pseudo3d_2dsamc_mw_d\":\"") + QString::number( pseudo3d_2dsamc_mw_d_bool ) + QString("\",");


  int pseudo3d_pcsa_s_ff0_bool = report.pseudo3d_pcsa_s_ff0 ? 1 : 0;
  jsonMask += QString("\"pseudo3d_pcsa_s_ff0\":\"") + QString::number( pseudo3d_pcsa_s_ff0_bool ) + QString("\",");

  int pseudo3d_pcsa_s_d_bool = report.pseudo3d_pcsa_s_d ? 1 : 0;
  jsonMask += QString("\"pseudo3d_pcsa_s_d\":\"") + QString::number( pseudo3d_pcsa_s_d_bool ) + QString("\",");

  int pseudo3d_pcsa_mw_ff0_bool = report.pseudo3d_pcsa_mw_ff0 ? 1 : 0;
  jsonMask += QString("\"pseudo3d_pcsa_mw_ff0\":\"") + QString::number( pseudo3d_pcsa_mw_ff0_bool ) + QString("\",");

  int pseudo3d_pcsa_mw_d_bool = report.pseudo3d_pcsa_mw_d ? 1 : 0;
  jsonMask += QString("\"pseudo3d_pcsa_mw_d\":\"") + QString::number( pseudo3d_pcsa_mw_d_bool ) + QString("\",");

  int pseudo3d_mask_bool = report.pseudo3d_mask ? 1 : 0;
  jsonMask += QString("\"pseudo3d_mask\":\"") + QString::number( pseudo3d_mask_bool ) + QString("\"");
  
  jsonMask += QString("}");
  
  //Save parent Report
  QStringList qry;
  qry << "new_report"
      << reportGUID
      << report.channel_name
      << QString::number( report.tot_conc )
      << QString::number( report.rmsd_limit )
      << QString::number( report.av_intensity )
      << QString::number( report.experiment_duration )
      << QString::number( report.wavelength )

      << QString::number( report.tot_conc_tol )
      << QString::number( report.experiment_duration_tol )

      << jsonMask
    ;

  qDebug() << "Query: new_report: " << qry;
  reportID = dbP->functionQuery( qry );
  qDebug() << "Save Report:  new DB: ID" << reportID;
  
  //Now save ReportItems (children)
  for ( int ii = 0; ii < report.reportItems.size(); ii++ )
    {
      US_ReportGMP::ReportItem curr_reportItem = report.reportItems[ ii ];
      writeReportItemToDB( dbP, reportGUID, reportID, curr_reportItem );
    }
  
  
  return reportID; 
}

// Write a singe ReportItem
int US_ExperGuiUpload::writeReportItemToDB( US_DB2* dbP, QString reportGUID, int reportID, US_ReportGMP::ReportItem reportItem ) 
{
  int reportItemID = 0;

  //Infer main params
  QString type          =  reportItem.type;
  QString method        =  reportItem.method;
  QString range_low     =  QString::number( reportItem.range_low ); 
  QString range_high    =  QString::number( reportItem.range_high );
  QString int_value     =  QString::number( reportItem.integration_val ); 
  QString tolerance     =  QString::number( reportItem.tolerance );     
  QString total_percent =  QString::number( reportItem.total_percent );  
  QString combined_plot =  QString::number( reportItem.combined_plot );  
  
  QStringList qry;
  qry << "new_report_item"
      << reportGUID
      << QString::number( reportID ) 
      << type
      << method
      << range_low
      << range_high
      << int_value 
      << tolerance
      << total_percent
      << combined_plot
    ;
    
  qDebug() << "Query: new_report_item: " << qry;
  reportItemID = dbP->functionQuery( qry );
  qDebug() << "Save ReportItem:  new DB: ID" << reportItemID;

  return reportItemID;
}

// Slot to save the current Analysis Profile
void US_ExperGuiUpload::saveAnalysisProfile()
{
   rpAprof->ap_xml.clear();

   QXmlStreamWriter xmlo_aprof( &rpAprof->ap_xml ); // Compose XML representation
   xmlo_aprof.setAutoFormatting( true );
   //mainw->epanAProfile->sdiag->currProf.toXml( xmlo_aprof );

   US_AnaProfile* aprof = mainw->get_aprofile();
   rpAprof->aprofname   = mainw->currProto.protoname;
   rpAprof->aprofGUID   = mainw->currProto.protoGUID;
   aprof  ->aprofname   = rpAprof->aprofname;
   aprof  ->aprofGUID   = rpAprof->aprofGUID;

   qDebug() << "IN Saving APRofile: aprof  -> report_mask -- " << aprof->report_mask;
     
   //save reports BEFORE writng down Aprofile's XML
   saveReports( aprof ); //<-------------------------------------- TEMPORARY comment!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   
   aprof  ->toXml( xmlo_aprof );
   //DbgLv(1) << "XML AProfile: " << rpAprof->ap_xml;
   //exit(1);
   
   QString xmlopath;
   QString dapath;

   // Find or compose an analysis profile local file name
   bool new_file    = false;
   dapath           = US_Settings::dataDir() + "/projects/";
   xmlopath         = US_DataFiles::get_filename( dapath, aprof->aprofGUID,
                         "A", "aprofile", "aprofileGUID", new_file );
DbgLv(1) << "EGAp:svAP: new_file" << new_file << "xmlopath" <<  xmlopath;
   if ( new_file )
   {  // Write AProfile XML as a */ultrascan/data/projects/A*xml file
      QFile xofile( xmlopath );
      if ( xofile.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         QTextStream xotxto( &xofile );
         xotxto << rpAprof->ap_xml;
         xofile.close();
      }
   }
   // exit(1); //TEMP!!!!
   
   // Write an analysis profile entry to the database
   US_Passwd  pw;
   US_DB2* dbP         = ( sibSValue( "general", "dbdisk" ) == "DB" )
                         ? new US_DB2( pw.getPasswd() ) : NULL;
   bool fromdisk       = US_Settings::debug_match( "protocolFromDisk" );
   dbP                 = fromdisk ? NULL : dbP;
   if ( dbP != NULL )
   {
      QStringList qry;
      qry << "new_aprofile"
	  << aprof->aprofGUID
	  << aprof->aprofname
          << rpAprof->ap_xml
	  << aprof->report_mask
	  << aprof->combPlots_parms
	;
      
      dbP->statusQuery( qry );
      aprof->aprofID      = dbP->lastInsertID();
DbgLv(1) << "EGAp:svAP:  qry" << qry;
DbgLv(1) << "EGAp:svAP:  new DB:  ID" << aprof->aprofID
 << dbP->lastError();
   }

   
   

   return;
}

// Slot to save the current Run Protocol
bool US_ExperGuiUpload::saveRunProtocol()
{

  bool save_aborted = false;
  
  if ( mainw->ScanCount_global > 1501 )
    {
      QMessageBox::critical( this,
                             tr( "*ERROR* in Saving Protocol" ),
                             tr( "UV/vis.: Protocol cannot be saved: \n"
                                 "Number of scans per cell per wavelengths is %1. \n"
                                 "It must not exceed 1501. \n\n"
                                 "Please revise experiment parameters accordingly." )
                                .arg( mainw->ScanCount_global ) );
      return save_aborted;
    }

  if ( mainw->ScanCount_global_int > 1501 )
    {
      QMessageBox::critical( this,
                             tr( "*ERROR* in Saving Protocol" ),
                             tr( "Protocol cannot be saved: \n"
                                 "Interference: Number of scans per cell is %1. \n"
                                 "It must not exceed 1501. \n\n"
                                 "Please revise experiment parameters accordingly." )
			     .arg( mainw->ScanCount_global_int ) );
      return save_aborted;
    }

  

DbgLv(1) << "EGUp:svRP: IN";
   // Test that the current protocol name is new
   QStringList           prnames;
   QList< QStringList >  prdats;
DbgLv(1) << "EGUp:svRP:  call getProtos()";
   mainw->getProtos( prnames, prdats );
DbgLv(1) << "EGUp:svRP:   prnames" << prnames;

   QString protoname   = sibSValue( "general", "protocol" );

   QString old_protoname = protoname;
//DbgLv(1) << "EGUp:svRP:  protoname" << protoname << "prdats0" << prdats[0];  //ALEXEY: important: this statement caused bug when no protocols existed in the DB

   if ( prnames.contains( protoname ) ||  protoname.trimmed().isEmpty() )
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
   QString newpname    = protoname;
   QString msg         =
      tr( "Enter a new Run Protocol name (description text)<br/>"
          "for the record to be saved; or modify the<br/>"
          "existing name so that it is unique.<br/><br/>"
          "Then click on <b>OK</b> to accept the new name<br/>"
          "or on <b>Cancel</b> to abort the Run Protocol save.<br/>" );

   // Keep displaying the dialog text until a unique name is given
   while( prnames.contains( newpname )  ||  newpname.trimmed().isEmpty() )
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

	 save_aborted = true;
	 
         return save_aborted;
      }
   }

   // //ALEXEY: check if has_interference or has_uvvis or both for cells//
   // QString uvvis       = tr( "UV/visible" );
   // QString rayleigh    = tr( "Rayleigh Interference" );
   // QStringList oprof   = sibLValue( "optical", "profiles" );



   // Save the new name and compose the XML representing the protocol
   protoname           = newpname;
DbgLv(1) << "EGUp:svRP:   NEW protoname" << protoname;
DbgLv(1) << "EGUp:svRP:   currProto previous guid" << currProto->protoGUID;
DbgLv(1) << "EGUp:svRP:   currProto previous protoname" << currProto->protoname;

   currProto->protoname = protoname;              // Update current protocol
   if ( currProto->protoGUID.isEmpty()  ||
        currProto->protoGUID.startsWith( "0000" ) )
      currProto->protoGUID = US_Util::new_guid(); // Get a new GUID
   else if ( old_protoname != protoname  )
     currProto->protoGUID = US_Util::new_guid(); // Get a new GUID
   
DbgLv(1) << "EGUp:svRP:   currProto updated  guid" << currProto->protoGUID;
DbgLv(1) << "EGUp:svRP:   currProto updated  protoname" << currProto->protoname;


   // Now that we have protocol name,guid; save the Analysis Profile
   saveAnalysisProfile();

   // us_xml string has to be cleared each time Protocol is saved
   rpSubmt->us_xml.clear();

   QXmlStreamWriter xmlo( &rpSubmt->us_xml ); // Compose XML representation
   xmlo.setAutoFormatting( true );
   currProto->toXml( xmlo );


DbgLv(1) << "EGUp:svRP:    guid" << currProto->protoGUID;
DbgLv(1) << "EGUp:svRP:    xml(s)" << QString(rpSubmt->us_xml).left(100);
int xe=rpSubmt->us_xml.length()-101;
DbgLv(1) << "EGUp:svRP:    xml(e)" << QString(rpSubmt->us_xml).mid(xe);


   // Save the new protocol to database and/or disk
   US_Passwd  pw;
   US_DB2* dbP         = ( sibSValue( "general", "dbdisk" ) == "DB" )
                         ? new US_DB2( pw.getPasswd() ) : NULL;
   bool fromdisk       = US_Settings::debug_match( "protocolFromDisk" );
   dbP                 = fromdisk ? NULL : dbP;

DbgLv(1) << "EGUp:svRP:   dbP" << dbP;
   // Always save the protocol to a local file
   int idProt          = US_ProtocolUtil::write_record( rpSubmt->us_xml, NULL );

   // Usually (database selected), we write an additional record to DB
   if ( dbP != NULL )
      idProt              = US_ProtocolUtil::write_record( rpSubmt->us_xml, dbP );

   if ( idProt < 0 )
   {
      QString errmsg   = ( dbP != NULL ) ? dbP->lastError() : "???";
      QMessageBox::critical( this,
         tr( "*ERROR* in Protocol Write" ),
         tr( "An error occurred in the attempt to save"
             " new protocol\n  %1\n  %2 ." ).arg( protoname ).arg( errmsg ) );

      return save_aborted;
   }

   // Update the full list of existing protocols
   QStringList prentry;                       // New protocol summary data
   QString pdate       = US_Util::toUTCDatetimeText(
                            QDateTime::currentDateTime().toUTC()
                            .toString( "yyyy/MM/dd HH:mm" ), true );
   QString protid      = ( dbP != NULL ) ? QString::number( idProt )
                         : "R" + QString().sprintf( "%7d", idProt ) + ".xml";
   QString pguid       = currProto->protoGUID;
   prentry << protoname << pdate << protid << pguid;

   qDebug() << "Updated proto list: " << prentry;
   
   mainw->updateProtos( prentry );            // Update existing protocols list
   proto_svd           = true;
   ck_prot_svd->setChecked( true );

   qDebug() << "constructor parms: mainw->automode, have_run -- " << mainw->automode  << have_run;
   
   if ( mainw->automode && !have_run)
     {
       QString mtitle_done    = tr( "Success" );
       QString message_done   = tr( "Protocol has been successfully saved." );
       QMessageBox::information( this, mtitle_done, message_done );
     }
   else if ( !mainw->automode )
     {
       QString mtitle_done    = tr( "Success" );
       QString message_done   = tr( "Protocol has been successfully saved." );
       QMessageBox::information( this, mtitle_done, message_done );
     }

   return save_aborted;
}

// Slot to read all Optima machines <------------------------------- // 
void US_ExperGuiUpload::read_optima_machines( US_DB2* db )
{

  all_instruments.clear();
  
  QStringList q( "" );
  q.clear();
  q  << QString( "get_instrument_names" )
     << QString::number( 1 );
  db->query( q );
  
  if ( db->lastErrno() == US_DB2::OK )      // If not, no instruments defined
    {
      QList< int > instrumentIDs;
      
      // Grab all the IDs so we can reuse the db connection
      while ( db->next() )
	{
	  int ID = db->value( 0 ).toString().toInt();
	  instrumentIDs << ID;
	  
	  qDebug() << "InstID: " << ID;
	}
      
      // Instrument information
      foreach ( int ID, instrumentIDs )
	{
	  QMap<QString,QString> instrument;
	  
	  q.clear();
	  q  << QString( "get_instrument_info_new" )
	     << QString::number( ID );
	  db->query( q );
	  db->next();

	  instrument[ "ID" ]              =   QString::number( ID );
	  instrument[ "name" ]            =   db->value( 0 ).toString();
	  instrument[ "serial" ]          =   db->value( 1 ).toString();
	  instrument[ "optimaHost" ]      =   db->value( 5 ).toString();	   
	  instrument[ "optimaPort" ]      =   db->value( 6 ).toString(); 
	  instrument[ "optimaDBname" ]    =   db->value( 7 ).toString();	   
	  instrument[ "optimaDBusername" ] =  db->value( 8 ).toString();	   
	  instrument[ "optimaDBpassw" ]    =  db->value( 9 ).toString();	   
	  instrument[ "selected" ]        =   db->value( 10 ).toString();
	    
	  instrument[ "opsys1" ]  = db->value( 11 ).toString();
	  instrument[ "opsys2" ]  = db->value( 12 ).toString();
	  instrument[ "opsys3" ]  = db->value( 13 ).toString();

	  instrument[ "radcalwvl" ]  =  db->value( 14 ).toString();
	  instrument[ "chromoab" ]   =  db->value( 15 ).toString();

	  instrument[ "msgPort" ]    =  db->value( 16 ).toString();
	  
	  if ( instrument[ "name" ].contains("Optima") || instrument[ "optimaHost" ].contains("AUC_DATA_DB") )
	    this->all_instruments << instrument;
	}
    }
  qDebug() << "Reading Instrument: FINISH in us_experiment: Upload";
}


//Confirm the Optima machine an experiemnt is submitted to.
void US_ExperGuiUpload::submitExperiment_confirm()
{
  // // ALEXEY: old way
  // QStringList dblist  = US_Settings::defaultXpnHost();
  // QString alias       = dblist[ 0 ];
  // QString dbhost      = dblist[ 1 ];
  // int     dbport      = dblist[ 2 ].toInt();

  //ALEXEY: new way
  QString alias       = mainw->currentInstrument[ "name" ];
  QString dbhost      = mainw->currentInstrument[ "optimaHost" ];
  QString dbport      = mainw->currentInstrument[ "optimaPort" ];
  QString optima_msgPort;
  QString optima_chromoAB;
  
  //ALEXEY: here - check for connection to sys_data server and put error msg if no connection, with suggesting to saveRunProtocol() into LIMS
  US_Passwd pw;
  US_DB2*   dbP = new US_DB2( pw.getPasswd() );
  if ( dbP != NULL )
    read_optima_machines( dbP );

  for ( int ii = 0; ii < all_instruments.size(); ii++ )
    {
      QString name = all_instruments[ii][ "name" ].trimmed();
      if ( name == alias )
	{
	  optima_msgPort  = all_instruments[ii][ "msgPort" ];
	  optima_chromoAB = all_instruments[ii][ "chromoab" ];
	}
    }

  qDebug() << "Optima_msgPort: "  << optima_msgPort;
  qDebug() << "Optima_chromoAB: " << optima_chromoAB;
  
  // Check for chromatic aberration file for intrument
  if ( optima_chromoAB.isEmpty()  )
    {
      QMessageBox msgBox_chromo;
      msgBox_chromo.setIcon(QMessageBox::Critical);
      msgBox_chromo.setWindowTitle(tr("Missing Optima's Chromatic Aberration File!"));
      
      QString msg_chromo_text      = QString("Attention! Currently used instrument, %1, is missing chromatic aberration file.").arg(alias);
      QString msg_chromo_text_info = QString("To proceed with the job submission, the file must be uploaded using instrument configuraiton utility:");
      msg_chromo_text_info += QString("\n Edit -> Preferences -> Instrument Preferences");
      msg_chromo_text_info += QString( tr ("\n\nIn the meantime, you may choose to save the protocol into LIMS database.") );
      
      msgBox_chromo.setText( msg_chromo_text );
      msgBox_chromo.setInformativeText( msg_chromo_text_info );
      
      QPushButton *Accept_chromo    = msgBox_chromo.addButton(tr("Save Protocol"), QMessageBox::YesRole);
      QPushButton *Cancel_chromo    = msgBox_chromo.addButton(tr("Cancel"), QMessageBox::RejectRole);

      msgBox_chromo.exec();
      
      if (msgBox_chromo.clickedButton() == Accept_chromo) {
	qDebug() << "Chromo -- Saving protocol...";

	if ( mainw->automode && rps_differ )
	  {
	    saveRunProtocol();
	    return;
	  }
	    
	else if ( !mainw->automode && have_run && rps_differ )
	  {
	    saveRunProtocol();
	    return;
	  }
	else
	  {
	    QMessageBox::warning( this,
				  tr( "No Changes in the Protocol" ),
				  tr( "The protocol was not saved because there were no changes made to it.") );
	    
	    return;
	  }
      }
      else if (msgBox_chromo.clickedButton() == Cancel_chromo){
	return;
      }
      
      return;
    }  
  // End checking chromatic aberration
  
  // Check for certificate license key and its expiraiton
  Link *link = new Link( alias );
  bool status_sys_data = link->connectToServer( dbhost, optima_msgPort.toInt() );
  bool combined_check  = status_sys_data & link->connected_itself;
  
  // Ceritificate location && check for nearing or actual expiration date ////////////////////
  QString certPath = US_Settings::etcDir() + QString("/optima/");

  QString client_name = alias;
  client_name.simplified();
  client_name.replace(" ", "");
  client_name = client_name.toLower();
  
  // QString keyFile  = certPath + QString( "client.key" );
  // QString pemFile  = certPath + QString( "client.pem" );
  
  QString keyFile  = certPath + client_name + ".key";
  QString pemFile  = certPath + client_name + ".pem";
  
  qDebug() << "keyFile, pemFile -- " << keyFile << pemFile;
    
  QFile certfile( pemFile );
  bool certfile_exists = certfile.open(QIODevice::ReadOnly);
  QSslCertificate cert(&certfile,QSsl::Pem);

  QDateTime expdate = cert.expiryDate();
            
  QString fmt = "yyyy-MM-dd";
  QString expdate_str = expdate.toString(fmt);
  
  qDebug() << "Qt's way of showing expiraiton date: " << cert.expiryDate() << expdate_str; 
  
  // get current QDate
  QDate dNow(QDate::currentDate());
  
  // Certs expiration QDate 
  QDate dEndCerts =  QDate::fromString(expdate_str, fmt);
  
  // Difference
  int daysToExpiration = dNow.daysTo(dEndCerts);
  
  qDebug() << "Now, End, expiration in days:  " << dNow << dEndCerts << daysToExpiration;
  
  // End of checking expiraiton date ////////////////////////////////////////////////////////////
  
  if ( !combined_check || daysToExpiration <= 0 || !certfile_exists )
    {
      QMessageBox msgBox_sys_data;
      msgBox_sys_data.setIcon(QMessageBox::Critical);
      msgBox_sys_data.setWindowTitle(tr("Optima System Data Server Connection Problem!"));
      
      QString msg_sys_text = QString("Attention! UltraScan is not able to communicate with the data acquisition server on the %1.").arg(alias);
      QString msg_sys_text_info = QString("");
      
      if (certfile_exists  )
	{
	  if ( daysToExpiration > 0 )
	    {
	      msg_sys_text += QString(tr("\n\nPlease check the following:"));
	      msg_sys_text_info += QString( tr("1. %1 is turned on\n"
					       "2. the data acquisition server on %1 is running\n"
					       "3. your license key is stored in $HOME/ultrascan/etc/optima and is not expired\n\n"
					       "Submission of the experimental protocol is suspended until this condition is resolved." ))
		.arg(alias);
	    }
	  else
	    {
	      msg_sys_text_info += QString( tr("Your license key is expired! Submission of the experimental protocol is suspended.\n"
					       "Please renew the key and try to submit again."));
	    }
	}
      else
	{
	  msg_sys_text += QString(tr("\n\nIt appears the license key for %1 is missing:")).arg(alias);
	  msg_sys_text_info += QString( tr("Please check that your license key is stored in $HOME/ultrascan/etc/optima and is not expired\n\n"
					   "Submission of the experimental protocol is suspended until this condition is resolved." ));
	}
      
      msg_sys_text_info += QString( tr ("\n\nYou may choose to save the protocol into LIMS database.") );
      
      msgBox_sys_data.setText( msg_sys_text );
      
      msgBox_sys_data.setInformativeText( msg_sys_text_info );
      
      QPushButton *Accept_sys    = msgBox_sys_data.addButton(tr("Save Protocol"), QMessageBox::YesRole);
      QPushButton *Cancel_sys    = msgBox_sys_data.addButton(tr("Cancel"), QMessageBox::RejectRole);

      msgBox_sys_data.exec();
      
      if (msgBox_sys_data.clickedButton() == Accept_sys) {
	qDebug() << "Saving protocol...";
	
	if ( mainw->automode && rps_differ )
	  {
	    //Disconnect link
	    link->disconnectFromServer();
	    
	    saveRunProtocol();
	    return;
	  }
	    
	else if ( !mainw->automode && have_run && rps_differ )
	  {
	    //Disconnect link
	    link->disconnectFromServer();
	    
	    saveRunProtocol();
	    return;
	  }
	else
	  {
	    QMessageBox::warning( this,
				  tr( "No Changes in the Protocol" ),
				  tr( "The protocol was not saved because there were no changes made to it.") );
	    
	    //Disconnect link
	    link->disconnectFromServer();
	    
	    return;
	  }
      }
      else if (msgBox_sys_data.clickedButton() == Cancel_sys)
	{
	  //Disconnect link
	  link->disconnectFromServer();
	  
	  return;
	}
    }
  else
    {
      if ( daysToExpiration <= 30 )
	{
	  qDebug() << "Certs nearing expiration!! ";
	  
	  QMessageBox::warning( this,
				tr( "License Key Nearing Expiraiton" ),
				QString(tr( "Your license key will expire within %1 days. \n\n This program will not function without it.") ).arg(daysToExpiration));
	  
	}
      
      //Disconnect link
      link->disconnectFromServer();
    }
  //  End of checkig for conneciton to Optima sys_data server ///////////////////////////////////////////////
  
  if ( mainw->automode && rps_differ )
    {
      if ( saveRunProtocol() )
	return;
    }
  else if ( !mainw->automode && have_run && rps_differ )
    {
      if ( saveRunProtocol() )
	return;
    }

  QMessageBox msgBox;
  QString message_protocol = tr( "");
  if ( rps_differ )
    message_protocol += tr( "A new protocol has been successfully saved to US-LIMS DB. \n\n");
  
  QString message_submission = message_protocol + tr("Experiment will be submitted to the following Optima machine:");
  
  //msgBox.setText(tr("Experiment will be submitted to the following Optima machine:"));
  msgBox.setText( message_submission );
  msgBox.setInformativeText( QString( tr(    "Name: %1 <br>  Host:  %2 <br> Port:  %3" ))
			     .arg(alias)
			     .arg(dbhost)
			     .arg(dbport));
  msgBox.setWindowTitle(tr("Confirm Experiment Run Submission"));
  QPushButton *Accept    = msgBox.addButton(tr("OK"), QMessageBox::YesRole);
  QPushButton *Cancel    = msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
  
  msgBox.setIcon(QMessageBox::Question);
  msgBox.exec();
  
  if (msgBox.clickedButton() == Accept) {
    qDebug() << "Submitting...";
    submitExperiment();
  }
  else if (msgBox.clickedButton() == Cancel){
    return;
  }
  
}

// Slot to submit the experiment to the Optima DB
void US_ExperGuiUpload::submitExperiment()
{

   // //ALEXEY connect to DB AUC: old way
   // US_Passwd pw;
   // QStringList dblist  = US_Settings::defaultXpnHost();
   // QString dbhost      = dblist[ 1 ];
   // int     dbport      = dblist[ 2 ].toInt();
   // QString dbname      = dblist[ 3 ];
   // QString dbuser      = dblist[ 4 ];
   // QString epasw       = dblist[ 5 ];
   // QString epasw0      = epasw.section( "^", 0, 0 );
   // QString epasw1      = epasw.section( "^", 1, 1 );
   // QString dbpasw      = US_Crypto::decrypt( epasw0, pw.getPasswd(), epasw1 );

   //ALEXEY: new way
   QString dbhost      = mainw->currentInstrument[ "optimaHost" ];
   int     dbport      = mainw->currentInstrument[ "optimaPort" ].toInt();
   QString dbname      = mainw->currentInstrument[ "optimaDBname" ];
   QString dbuser      = mainw->currentInstrument[ "optimaDBusername" ];
   QString dbpasw      = mainw->currentInstrument[ "optimaDBpassw" ];

   qDebug() << "OPTIMA: host port name user pasw" << dbhost << dbport << dbname << dbuser << dbpasw;

   dbxpn           = QSqlDatabase::addDatabase( "QPSQL", "" );
   // DbgLv(1) << "XpDa:cnc: drivers" << dbxpn.drivers();
   dbxpn.setDatabaseName( "XpnData" );
   dbxpn.setHostName    ( dbhost );
   dbxpn.setPort        ( dbport );
   dbxpn.setDatabaseName( dbname  );
   dbxpn.setUserName    ( dbuser  );
   dbxpn.setPassword    ( dbpasw );


   QMap <QString, QString > protocol_details;                          // QMap to pass later to US_com_project' Live Updates

   //US_XpnData* xpn_data = new US_XpnData();
   // connected           = xpn_data->connect_data( xpnhost, xpnport, dbname, dbuser,  dbpasw );

   if (  dbxpn.open() )
   { // dbxpn.open()
      qDebug() << "Connected !!!";

      // Get # satges, cells
      int nstages = sibIValue( "speeds",  "nspeeds" );

      int tem_delay_sec = int((mainw->currProto.temeq_delay)*60);         // delay in sec (longevity) of the dummy equlibration stage
      int nstages_size;
      nstages_size = tem_delay_sec ? nstages + 1 : nstages;               // Total # stages
      int ncells  = sibIValue( "rotor",   "nholes" );

      qDebug() << "#Stages: " << nstages;
      qDebug() << "#Cells: " << ncells;

      // Define AbsScanId array of arrays
      QVector < QVector < int >> AbsScanIds(nstages_size);
      // Add extra array QVector < QVector < QString >> RadialPath(nstages_size); to be filled with DEFAULT/ */
      QVector < QVector < int >> AbsRadialPath(nstages_size);

      // Define array of the total # wvl per stage
      QVector < int > Total_wvl(nstages_size);
      // ALEXEY: should we introduce separate for interference? (1 wvl 660 per stage per cell ?)

      for (int i=0; i<nstages_size; i++)
      {
         AbsScanIds[i].resize(ncells);
         AbsRadialPath[i].resize(ncells);

         Total_wvl[i] = 0;
      }

      //Opt system check, what cells will be uvvis and/or interference
      QStringList oprof   = sibLValue( "optical", "profiles" );
      QString uvvis       = tr( "UV/visible" );
      QString rayleigh    = tr( "Rayleigh Interference" );
      
      //get # cells with interference channels
      int ncells_interference = 0;
      QStringList active_channels;
      for ( int kk = 0; kk < oprof.count(); kk++ )
      {
         if ( oprof[ kk ].contains( rayleigh ) )
	   {
	     if  ( oprof[ kk ].section( ":", 0, 0 ).contains("sample") )
	       {
		 qDebug() << "ITF channel name: " <<  oprof[ kk ].section( ":", 0, 0 ).split(",")[0];
		 active_channels << oprof[ kk ].section( ":", 0, 0 ).split(",")[0];
	       }
	     
	     ++ncells_interference;
	   }
      }

      // return;

      for (int i=0; i<nstages_size; i++)
      {
         for (int j=0; j<ncells; j++)
         {
            AbsScanIds[i][j] = 0;
            AbsRadialPath[i][j] = 0;
            //Compute total # wvl per stage
            QString channel;
            for ( int ii = 0; ii < rpRange->nranges; ii++ )
            {
               channel  = rpRange->chrngs[ ii ].channel;
	       active_channels << channel.split(",")[0];
	       
               if ( channel.contains("sample") && channel.startsWith(QString::number(j+1)) )  // <-- Judge only by sample (channel A) for now
               {
                  Total_wvl[i]  += rpRange->chrngs[ ii ].wvlens.count();                     // <-- count wvl

	       }
            }
            qDebug() << "#Wvl for cell: " << j << " is: " << Total_wvl[i];
         }
      }

      //List active channles:
      active_channels.removeDuplicates();
      for (int i=0; i<active_channels.count(); ++i)
	qDebug() << "Active channel #: " << i << ", " << active_channels[i];


      // QString CellChMap = QString("IP:") + QString::number( int(ncells_interference/2 ) ) + QString(",RI:")  + QString::number(rpSolut->nschan);
      
      // qDebug() << "CellChMap: " << CellChMap;
      
      // return;
      
      //Total duration
      bool is_dummy_dur = false;
      int curr_stage_dur;
      int Total_duration = tem_delay_sec;
      for (int i=0; i<nstages_size; i++)
      {
         if (i==0 && tem_delay_sec)
         {
            is_dummy_dur = true;
            continue;                // skip dummy stage for AbsScanParams
         }

         if (is_dummy_dur)
            curr_stage_dur = i - 1;
         else
            curr_stage_dur = i;

         qDebug() << "index i: " << i << ", curr_stage_DUR: " << curr_stage_dur;

         Total_duration += rpSpeed->ssteps[ curr_stage_dur ].duration;
         Total_duration += rpSpeed->ssteps[ curr_stage_dur ].delay_stage;
         Total_duration += rpSpeed->ssteps[ curr_stage_dur ].delay;
         //ALEXEY: do we need also delays to first scans for abs/interference into total duration?
      }


      // Absorbance INSERT ////////////////////////////////////////////////////////////////////////
      QString tabname_abs( "AbsorbanceScanParameters" );
      QString schname( "AUC_schema" );
      QString qrytab_abs  = "\"" + schname + "\".\"" + tabname_abs + "\"";

      /* WHAT TO INSERT: fields
       "ScanCounts": []                   <-- computations
       "ScanIntervals": []                <-- computations
       "ScanInnerLimits": []              <-- 5.75 cm default
       "ScanOuterLimits": []              <-- 7.25 cm default
       "ScanSteps": [10,10,10..]          <-- VERIFY it's 10 um for each wvl
       "ScanTypeFlag":                    <-- "I" always for Absorbance scan
       "WavelengthCount":
       "Wavelengths": []
       "RadialPath":  ""                  <-- "A", "B", or ""
      */

      /* Define 2D array "AbsScanIDs[number_of_stages + 1][number_of_cells]"

       stage#  cell# (e.g. 8-hole rotor)
       0       [0,1,2,3,4,5,6,7]        <-- Extra dummy zeroth stage
       --------------------------
       1       [0,1,2,3,4,5,6,7]        <-- Active stages
       2       [0,1,2,3,4,5,6,7]

      */


      qDebug() << "Begin AbsInsert";
      bool is_dummy = false;
      int curr_stage;
      for (int i=0; i<nstages_size; i++)
      {
         if (i==0 && tem_delay_sec)
         {
            is_dummy = true;
            continue;                     // skip dummy stage for AbsScanParams
         }

         if (is_dummy)
            curr_stage = i - 1;
         else
            curr_stage = i;

         qDebug() << "index i: " << i << ", curr_stage: " << curr_stage;

         double duration_sec = rpSpeed->ssteps[ curr_stage ].duration;
         double delay_sec    = rpSpeed->ssteps[ curr_stage ].delay;
         double scanint_sec  = rpSpeed->ssteps[ curr_stage ].scanintv;
         //double scanint_sec_min  = rpSpeed->ssteps[ curr_stage ].scanintv_min;
	 double scanint_sec_min;
	 std::modf ( rpSpeed->ssteps[ curr_stage ].scanintv_min, &scanint_sec_min);
	 

         qDebug() << "Size of rpSpeed is: " << rpSpeed->ssteps.size() << ", while nstages_size is: " << nstages_size << ", size of Total_wvl is: " <<  Total_wvl.size();
         qDebug() << "DURATION!!! duration_sec = rpSpeed->ssteps[ curr_stage ].duration: " << duration_sec <<  "=" <<  rpSpeed->ssteps[ curr_stage ].duration;

         // <-- Which delay should we substract ? (not a stage delay but due to acceleration ONLY ? )
         //int ScanCount = int( (duration_sec - delay_sec) / (scanint_sec * Total_wvl[i]) );

         int ScanCount;
         int ScanInt;

	 //ALEXEY: use this algorithm:
         if ( scanint_sec > scanint_sec_min * Total_wvl[i] )
         {
            ScanCount = int( duration_sec / scanint_sec );
            ScanInt   = scanint_sec;
         }
         else
         {
            ScanCount = int( duration_sec / (scanint_sec_min * Total_wvl[i] ) );
            ScanInt   = scanint_sec_min * Total_wvl[i];
         }

	 //Increase scan interval if scancount >= 1500:
	 if( ScanCount >= 1500 )
	   {
	     ScanInt    = int( duration_sec / 1500 );
	     ScanCount  = 1500;
	   }

	 // ScanCount = int( duration_sec / ( scanint_sec * Total_wvl[i] ) );
	 // ScanInt   = scanint_sec;
	 
         qDebug() << "Duration_sec: " << duration_sec << ", delay_sec: " << delay_sec << ", scanint_sec: " << scanint_sec << ", Tot_wvl: " << Total_wvl[i];
	 qDebug() << "ScanCount: global (ranges) vs. computed here: -- " << mainw->ScanCount_global << " vs. " << ScanCount;

         for (int j=0; j<ncells; j++)
         {
            QString channel;
            int    nwavl;
            QList< double > wvl_list;
            double lo_radi;
            double hi_radi;
            bool is_wvl_range   = false;


            bool has_absorbance = false;
            for ( int kk = 0; kk < oprof.count(); kk++ )
            {
               if ( oprof[ kk ].contains( uvvis ) )
               {
                  channel =  oprof[ kk ].section( ":", 0, 0 );
                  if (channel.startsWith(QString::number(j+1)))
                  {
                     has_absorbance = true;
                     break;
                  }
               }
            }

//34123123123123123123
            if ( has_absorbance )
            {
               //Stop if ScanCount > 1500
               if ( ScanCount > 1500 )
               {
                  QMessageBox::critical( this,
                  tr( "*ERROR* in Submitting Protocol: UV/vis." ),
                  tr( "Protocol cannot be submitted: \n"
                      "Number of scans per cell per wavelengths is %1. \n"
                      "It must not exceed 1500. \n\n"
                      "Please revise experiment parameters accordingly." )
                     .arg( ScanCount ) );
                  return;
               }

               for ( int ii = 0; ii < rpRange->nranges; ii++ )
               {
                  channel  = rpRange->chrngs[ ii ].channel;

                  if ( channel.contains("sample") && channel.startsWith(QString::number(j+1)) )  // <-- Judge only by sample (channel A) for now
                  {
                     nwavl    = rpRange->chrngs[ ii ].wvlens.count();
                     wvl_list = rpRange->chrngs[ ii ].wvlens;
                     lo_radi  = rpRange->chrngs[ ii ].lo_rad;
                     hi_radi  = rpRange->chrngs[ ii ].hi_rad;
                     is_wvl_range = true;
                     break;
                  }
               }
               // Create query VALUE strings and Make insertions into AbsScanParams table
               if (is_wvl_range)
               {
                  QString wvl_count        = QString::number( nwavl );
                  QString wvl_array        = "\'{";
                  QString scan_inner_array = "\'{";
                  QString scan_outer_array = "\'{";
                  QString scan_steps_array = "\'{";
                  QString scan_starts_array = "\'{";
                  QString replicate_counts_array = "\'{";
                  QString continuous_mode_array = "\'{";
                  QString scan_counts = "\'{";
                  QString scan_intervals = "\'{";
                  for (int r=0; r<nwavl; r++)
                  {
                     wvl_array        +=  QString::number(wvl_list[r]);
                     qDebug() << "Wvl: " << r << " " << wvl_list[r];
                     scan_inner_array += QString::number(lo_radi);
                     scan_outer_array += QString::number(hi_radi);
                     scan_steps_array += QString::number(10);                     // <-- 10 um
                     scan_starts_array += QString::number(0);                     // <-- '0' to allow control by scan interval
                     replicate_counts_array += QString::number(1);                // <-- shoud be '1'
                     continuous_mode_array += "t";                                // <-- always 't'
                     scan_counts += QString::number(ScanCount);                   // <-- ScanCount
                     scan_intervals += QString::number(int(ScanInt));         // <-- ScanInterval
                     if (r != nwavl - 1)
                     {
                        wvl_array        += ",";
                        scan_inner_array += ",";
                        scan_outer_array += ",";
                        scan_steps_array += ",";
                        scan_starts_array += ",";
                        replicate_counts_array += ",";
                        continuous_mode_array += ",";
                        scan_counts += ",";
                        scan_intervals += ",";
                     }
                  }
                  wvl_array        += "}\'";
                  scan_inner_array += "}\'";
                  scan_outer_array += "}\'";
                  scan_steps_array += "}\'";
                  scan_starts_array += "}\'";
                  replicate_counts_array += "}\'";
                  continuous_mode_array += "}\'";
                  scan_counts += "}\'";
                  scan_intervals += "}\'";

                  qDebug() << "Wvl_Array: " << wvl_array;
                  QString rad_path = "DEFAULT";
                  //QString rad_path = "\'A\'";
                  //QString rad_path = "\'B\'";

                  if ( QString::compare(rad_path, "DEFAULT",Qt::CaseSensitive) )
                     qDebug() << "RadialPath is NOT DEFAULT";


                  QSqlQuery query_abs_scan(dbxpn);
                  if(! query_abs_scan.prepare(QString("INSERT INTO %1 (\"ContinuousMode\",\"ReplicateCounts\",\"ScanInnerLimits\",\"ScanOuterLimits\",\"ScanStarts\",\"ScanSteps\",\"ScanTypeFlag\",\"WavelengthCount\",\"Wavelengths\",\"ScanCounts\",\"ScanIntervals\",\"RadialPath\") VALUES (%2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13) RETURNING \"ScanId\"")
                     .arg(qrytab_abs)
                     .arg(continuous_mode_array)
                     .arg(replicate_counts_array)
                     .arg(scan_inner_array)
                     .arg(scan_outer_array)
                     .arg(scan_starts_array)
                     .arg(scan_steps_array)
                     .arg("\'I\'")
                     .arg(wvl_count)
                     .arg(wvl_array)
                     .arg(scan_counts)
                     .arg(scan_intervals)
                     .arg(rad_path)
                     //.arg("DEFAULT")
                     //.arg("\'A\'")
                     ) )
                        qDebug() << query_abs_scan.lastError().text();

                  //qDebug() << "Stop here" << ", ScanInt: " << scan_intervals << ", ScanCount: " << scan_counts;
                  //return;

                  if (query_abs_scan.exec())
                  {
                     qDebug() << "AbsorbaceScanParameters record created";

                     query_abs_scan.next();
                     AbsScanIds[i][j] = query_abs_scan.value(0).toInt();         // <-- Save AbsScanID inserted [for given stage#/cell#]


                     if ( QString::compare(rad_path, "DEFAULT",Qt::CaseSensitive) )
                     {
                        AbsRadialPath[i][j] = 1;
                        qDebug() << "RadialPath is NOT DEFAULT: 1-channel.";
                     }

                     qDebug() << "ScanId: "     << query_abs_scan.value(0).toInt();
                     qDebug() << "RadialPath: " << AbsRadialPath[i][j];
                  }
                  else
                  {
                     QString errmsg   = "Create record error: " + query_abs_scan.lastError().text();;
                     QMessageBox::critical( this,
                        tr( "*ERROR* in Submitting Protocol" ),
                        tr( "An error occurred in the attempt to submit"
                        " protocol to AUC DB\n  %1 table\n  %2 ." ).arg( qrytab_abs ).arg( errmsg ) );
                     return;
                  }

               }
               qDebug() << "Cell " << j << "is processed ";
            }
         }
         qDebug() << "AFTER CELLS processed";
      }
      qDebug() << "AFTER STAGES processed";


      // Interference INSERT ////////////////////////////////////////////////////////////////////////
      QString tabname_inter( "InterferenceScanParameters" );
      QString qrytab_inter  = "\"" + schname + "\".\"" + tabname_inter + "\"";

      /* WHAT TO INSERT: fields

          "ModulePosition": " ",      >> default '2'
          "ReplicateCount": " ",      >> should be '1' & this is default
          "ScanStageDelay": " ",      >> not currently used
          "ScanStart": " ",           >> defaults to '0' to allow control by by scan interval
          "ScanTypeFlag": " ",        >> default 'P'
          "ScanTypeName": " ",        >> default 'Interference'
          "Wavelength": " ",          >> default 660 nm  - laser wvl

          "ScanCount": " ",           >> computation  Same as AbsScan ?
          "ScanInterval": " ",        >> computation  Sama as AbsScan ?
       */

       /* Define 2D array "InterScanIDs[number_of_stages + 1][number_of_cells]"

       stage#  cell# (e.g. 8-hole rotor)
       0       [0,1,2,3,4,5,6,7]        <-- Extra dummy zeroth stage
       --------------------------
       1       [0,1,2,3,4,5,6,7]        <-- Active stages
       2       [0,1,2,3,4,5,6,7]

       */


      //QStringList oprof   = sibLValue( "optical", "profiles" );


      QVector < QVector < int >> InterScanIds(nstages_size);
      for (int i=0; i<nstages_size; ++i)
         InterScanIds[i].resize(ncells);

      for (int i=0; i<nstages_size; ++i)
      {
         for (int j=0; j<ncells; ++j)
         {
            InterScanIds[i][j] = 0;
         }
      }


      bool is_dummy_int = false;
      int curr_stage_int;

      for (int i=0; i<nstages_size; i++)
      {
         if (i==0 && tem_delay_sec)
         {
            is_dummy_int = true;
            continue;                    // skip dummy stage for InterferenceScanParams
         }

         if (is_dummy_int)
            curr_stage_int = i - 1;
         else
            curr_stage_int = i;

         double duration_sec = rpSpeed->ssteps[ curr_stage_int ].duration;
         double delay_sec    = rpSpeed->ssteps[ curr_stage_int ].delay_int;
         double scanint_sec  = rpSpeed->ssteps[ curr_stage_int ].scanintv_int;
         //double scanint_sec_min  = rpSpeed->ssteps[ curr_stage_int ].scanintv_int_min;

         int ScanCount;
         int ScanInt;

	 // ScanCount = int( duration_sec / (scanint_sec * (ncells_interference / 2)  ));  //ALEXEY: do NOT divide by #cells ?
	 // //ScanCount = int( duration_sec / (scanint_sec  ));
	 // ScanInt   = scanint_sec;

	 //ALEXEY: use this algorithm for Interference (same as for UV/vis, but scanint_int_min = 5 sec)
	 if ( scanint_sec > 5 * (ncells_interference / 2) )
         {
            ScanCount = int( duration_sec / scanint_sec );
            ScanInt   = scanint_sec;
         }
         else
         {
            ScanCount = int( duration_sec / (5 * (ncells_interference / 2) ) );
            ScanInt   = 5 * (ncells_interference / 2) ;
         }

	 //Increase scan interval if scancount >= 1500:
	 if( ScanCount >= 1500 )
	   {
	     ScanInt    = int( duration_sec / 1500 );
	     ScanCount  = 1500;
	   }

	 

         qDebug() << "Duration_sec: " << duration_sec << ", delay_sec_int: " << delay_sec << ", scanint_sec_int: " << scanint_sec;
	 qDebug() << "ScanCount_int: global (ranges) vs. computed here: -- " << mainw->ScanCount_global_int << " vs. " << ScanCount;

	 
         for (int j=0; j<ncells; j++)
         {
            QString channel;
            bool has_interference = false;
            for ( int ii = 0; ii < oprof.count(); ii++ )
            {
               if ( oprof[ ii ].contains( rayleigh ) )
               {
                  channel =  oprof[ ii ].section( ":", 0, 0 );
                  if (channel.startsWith(QString::number(j+1)))
                  {
                     has_interference = true;
                     break;
                  }
               }
            }
            if ( has_interference )
            {
               //ALEXEY: For interference as well ?
               if ( ScanCount > 1500 )
               {
                  QMessageBox::critical( this,
                  tr( "*ERROR* in Submitting Protocol: Interference" ),
                  tr( "Protocol cannot be submitted: \n"
                      "Number of scans per cell per wavelengths is %1. \n"
                      "It must not exceed 1500. \n\n"
                      "Please revise experiment parameters accordingly." )
                     .arg( ScanCount ) );

                  return;
               }

               QString scan_count        = QString::number( ScanCount );               // <-- ScanInterval
               QString scan_interval     = QString::number( int(ScanInt) );            // <-- ScanCount

               // Query
               QSqlQuery query_inter_scan(dbxpn);
               QString query_str = QString("INSERT INTO %1 (\"ScanCount\",\"ScanInterval\") VALUES (%2, %3) RETURNING \"ScanId\"")
                  .arg(qrytab_inter)
                  .arg(scan_count)
                  .arg(scan_interval);

               if(! query_inter_scan.prepare(query_str) )
                  qDebug() << query_inter_scan.lastError().text();

               if (query_inter_scan.exec())
               {
                  qDebug() << "InterferenceScanParameters record created";

                  query_inter_scan.next();
                  InterScanIds[i][j] = query_inter_scan.value(0).toInt();         // <-- Save InterScanID inserted [for given stage#/cell#]
                  qDebug() << "ScanId: " << query_inter_scan.value(0).toInt();
               }
               else
               {
                  QString errmsg   = "Create record error: " + query_inter_scan.lastError().text();;
                  QMessageBox::critical( this,
                     tr( "*ERROR* in Submitting Protocol" ),
                     tr( "An error occurred in the attempt to submit"
                     " protocol to AUC DB\n  %1 table\n  %2 ." ).arg( qrytab_inter ).arg( errmsg ) );
                     return;
               }
            }
         }
      }


      // Cell INSERT ////////////////////////////////////////////////////////////////////////
      QString tabname_cell( "CellParameters" );
      QString qrytab_cell  = "\"" + schname + "\".\"" + tabname_cell + "\"";

      /* WHAT TO INSERT: fields

         "CellPosition": " ",         >> 1 to 4, or 1 to 8
         "CellSectors": " ",          >> 2 or 99
         "CenterpieceTypeId": " ",    >> 0 in all DB records
         "Comments": " "
         "SampleName": " ",
         "AbsorbanceScan":  TRUE/FALSE
         "AbsorbanceScanId":    " "
      */

      /* Define 2D array "CellIDs[number_of_stages + 1][number_of_cells]"

      stage#  cell# (8-hole rotor)
       0       [0,1,2,3,4,5,6,7]        <-- Extra dummy zeroth stage
       --------------------------
       1       [0,1,2,3,4,5,6,7]        <-- Active stages
       2       [0,1,2,3,4,5,6,7]

      */

      QVector < QVector < int >> CellIds(nstages_size);
      for (int i=0; i<nstages_size; ++i)
         CellIds[i].resize(ncells);

      for (int i=0; i<nstages_size; ++i)
      {
         for (int j=0; j<ncells; ++j)
         {
            CellIds[i][j] = 0;
         }
      }

      QSqlQuery query_cell(dbxpn);

      for (int i=0; i<nstages_size; i++)
      {
         for (int j=0; j<ncells; j++)
         {
            QString cell_pos = QString::number( j+1 );
            int cellsect;
            if ( j != ncells - 1)
            {
               /* Check here if RadialPath[i][j] is DEFAULT (both channels) OR A/B: if DEFAULT, CellSector=2, if A/B CellSector=1  */
               if (AbsRadialPath[i][j])
                  cellsect = 1;
               else
                  cellsect = 2;
            }
            else
               cellsect = 99;

            QString cell_sector = QString::number( cellsect );

            // Solution/Channel description
            QString channel_cell;
            QString solname = "\'Cell " + QString::number(j+1);
            for ( int ii = 0; ii < rpSolut->nschan; ii++ )
            {
               channel_cell = rpSolut->chsols[ ii ].channel;
               //QStringList sol_split = (rpSolut->chsols[ ii ].ch_comment).split(',');

               QString solution = rpSolut->chsols[ ii ].ch_comment;
               solution.replace("'", "");
               QStringList sol_split = solution.split(',');

               if ( channel_cell.startsWith(QString::number(j+1)) )
               {
                  if ( channel_cell.contains("sample") )                                                     // <-- Channel A
                     solname += ": A: " + sol_split[0] + ", "        // <-- solution name
                        + sol_split[sol_split.size()-1] + "; ";             // <-- solution manual comment
                  if ( channel_cell.contains("reference") )                                                  // <-- Channel B
                     solname += "B: " + sol_split[0] + " "           // <-- solution name
                        + sol_split[sol_split.size()-1];             // <-- solution manual comment
               }
            }
            solname += "\'";
            ////////////////////////////////
            QString cell_query_str;
            QString abs_scanid    = QString::number( AbsScanIds[i][j] );
            QString inter_scanid  = QString::number( InterScanIds[i][j] );
            QString comment;

            // <-- dummy stage
            if (i==0  && tem_delay_sec)
            {
               cell_query_str = QString("INSERT INTO %1 (\"CellPosition\",\"CellSectors\",\"SampleName\") VALUES (%2, %3, %4) RETURNING \"CellParamId\"")
                  .arg(qrytab_cell).arg(cell_pos)
                  .arg(cell_sector).arg(solname);
               comment = "Dummy stage";

               //qDebug() << "Dummy stage: QUERY: " << cell_query_str;
            }
            // <-- Active stages
            else
            {
               // <-- Active Stage: ONLY AbsScan exists
               if ( AbsScanIds[i][j] && !InterScanIds[i][j] )
               {
                  cell_query_str = QString("INSERT INTO %1 (\"CellPosition\",\"CellSectors\",\"AbsorbanceScan\",\"AbsorbanceScanId\",\"SampleName\") VALUES (%2, %3, %4, %5, %6) RETURNING \"CellParamId\"")
                     .arg(qrytab_cell)
                     .arg(cell_pos)
                     .arg(cell_sector)
                     .arg("\'TRUE\'")
                     .arg(abs_scanid)
                     .arg(solname);
                  comment = "Active Stage --> AbsScan ONLY EXISTS";
               }
               // <-- Active Stage: ONLY InterferenceScan exists
               else if ( !AbsScanIds[i][j] && InterScanIds[i][j] )
               {
                  cell_query_str = QString("INSERT INTO %1 (\"CellPosition\",\"CellSectors\",\"InterferenceScan\",\"InterferenceScanId\",\"SampleName\") VALUES (%2, %3, %4, %5, %6) RETURNING \"CellParamId\"")
                     .arg(qrytab_cell)
                     .arg(cell_pos)
                     .arg(cell_sector)
                     .arg("\'TRUE\'")
                     .arg(inter_scanid)
                     .arg(solname);
                  comment = "Active Stage --> InterferenceScan ONLY EXISTS";
               }
               // <-- Active Stage: BOTH AbsScan && InterferenceScan exist - RARE CASE
               else if ( AbsScanIds[i][j] && InterScanIds[i][j] )
               {
                  //qDebug() << "Writing HERE: Abs & Interfrence  !!! ";
                  //qDebug() << "solname: " << solname;
                  cell_query_str = QString("INSERT INTO %1 (\"CellPosition\",\"CellSectors\",\"AbsorbanceScan\",\"AbsorbanceScanId\",\"InterferenceScan\",\"InterferenceScanId\",\"SampleName\") VALUES (%2, %3, %4, %5, %6, %7, %8) RETURNING \"CellParamId\"")
                     .arg(qrytab_cell)
                     .arg(cell_pos)
                     .arg(cell_sector)
                     .arg("\'TRUE\'")
                     .arg(abs_scanid)
                     .arg("\'TRUE\'")
                     .arg(inter_scanid)
                     .arg(solname);
                  comment = "Active Stage --> AbsScan and InterferenceScan BOTH EXIST";

                  // qDebug() << "Query: " << cell_query_str;
               }
               // <-- Active Stage: No Scans exist
               else
               {
                  cell_query_str = QString("INSERT INTO %1 (\"CellPosition\",\"CellSectors\",\"SampleName\") VALUES (%2, %3, %4) RETURNING \"CellParamId\"")
                     .arg(qrytab_cell)
                     .arg(cell_pos)
                     .arg(cell_sector)
                     .arg(solname);
                  comment = "Active stage --> NO SCANS";
               }
            }

            // Query
            if ( ! query_cell.prepare( cell_query_str ) )
               qDebug() << query_cell.lastError().text();

            if (query_cell.exec())
            {
               qDebug() << "CellParameters record created: " + comment;

               query_cell.next();
               CellIds[i][j] = query_cell.value(0).toInt();         // <-- Save CellID inserted [for given stage#/cell#]
               qDebug() << "CellId: " << query_cell.value(0).toInt();
            }
            else
            {
               QString errmsg   = "Create record error: " + query_cell.lastError().text();;
               QMessageBox::critical( this,
                  tr( "*ERROR* in Submitting Protocol" ),
                  tr( "An error occurred in the attempt to submit"
                      " protocol to AUC DB\n  %1 table\n  %2 ." ).arg( qrytab_cell ).arg( errmsg ) );
               return;
            }
         }
      }

      // FugeProfile INSERT ////////////////////////////////////////////////////////////////////////
      QString tabname_fuge( "CentrifugeRunProfile" );
      QString qrytab_fuge  = "\"" + schname + "\".\"" + tabname_fuge + "\"";

      /* WHAT TO INSERT: fields

          "StageCellParameterIds":  [
                                      [ ... ], [ ... ]   <-- CellParams Ids for each stage
                                    ],
          "StageDuration"        : [0,0,0,..]            <-- alway zeroth -exact amount of time required for completion of all stage science
          "StageStart"           : [1-3h, 0, 0, ..]      <-- Delay (in seconds) for stage science start after target RPM attainment
                                                      ONLY for dummy stage
          "StageRPM"             : []
          "Stages"               : " ",                  <-- # of stages
          "Temperature"          : " ",
          "SystemStatusInterval" : " "                   <--  interval in seconds [1s] between system status record insertions
      */

       /* Create string of cellParams IDs from 2D CellIds[i][j] array
       */

      int FugeProfileId = 0;
      QSqlQuery query_fuge(dbxpn);

      QString cellids       = "\'{";
      QString stagedur      = "\'{";
      QString stagestart    = "\'{";
      QString stagerpm      = "\'{";
      QString stageaccl     = "\'{";
      QString stagenum      = QString::number(nstages_size);
      QString temperature   = QString::number(mainw->currProto.temperature);
      QString sysstatint    = QString::number(1);
      QStringList speeds    = sibLValue( "speeds",    "profiles" );

      int curr_stage_fuge;
      bool is_dummy_fuge = false;
      //int ss                = 0;                                      // <-- for reading RPMs from speeds
      for (int i=0; i<nstages_size; i++)
      {
         cellids += "{";
         for (int j=0; j<ncells; j++)
         {
            cellids += QString::number(CellIds[i][j]);              // <-- cellIds
            if ( j != ncells-1)
               cellids += ",";
         }
         cellids += "}";

         stagedur    += QString::number(0);                          // <-- stageduration <-- ALWAYS 0
         stageaccl   += QString::number(0);                          // <-- stageaccelrate

         if (i==0 && tem_delay_sec)
         {
            stagestart  += QString::number(tem_delay_sec);          // <-- stagestart dummy stages
            stagerpm    += QString::number(0);                      // <-- RPM (0) dummy stage
            is_dummy_fuge = true;
         }
         else
         {
            // stagestart  += QString::number(0);                       // <-- stagestart Active stages
            // stagerpm    += QString::number( (speeds[ss].split(QRegExp("\\s+"), QString::SkipEmptyParts))[0].toInt() );       // <-- RPM Active stages
            // ss += 5;
            if (is_dummy_fuge)
               curr_stage_fuge = i - 1;
            else
               curr_stage_fuge = i;

            stagestart  += QString::number(rpSpeed->ssteps[ curr_stage_fuge ].delay_stage);  // <-- stagestart Active stages
            stagerpm    += QString::number(rpSpeed->ssteps[ curr_stage_fuge ].speed);        // <-- RPM Active stages
         }

         if ( i != nstages_size-1 )
         {
            cellids     += ",";
            stagedur    += ",";
            stagestart  += ",";
            stagerpm    += ",";
            stageaccl   += ",";
         }
      }
      cellids     += "}\'";
      stagedur    += "}\'";
      stagestart  += "}\'";
      stagerpm    += "}\'";
      stageaccl   += "}\'";

      // // Query
      if(! query_fuge.prepare(QString("INSERT INTO %1 (\"StageCellParameterIds\",\"StageDuration\",\"StageStart\",\"StageRPM\",\"Stages\",\"SystemStatusInterval\",\"Temperature\",\"StageAccelRate\",\"HoldSpeedAfterFinal\") VALUES (%2, %3, %4, %5, %6, %7, %8, %9, %10) RETURNING \"FugeRunProfileId\"")
         .arg(qrytab_fuge)
         .arg(cellids)
         .arg(stagedur)
         .arg(stagestart)
         .arg(stagerpm)
         .arg(stagenum)
         .arg(sysstatint)
         .arg(temperature)
         .arg(stageaccl)
         .arg("\'TRUE\'") ) )
          qDebug() << query_fuge.lastError().text();

      if (query_fuge.exec())
      {
         qDebug() << "FugeProfile record created";

         query_fuge.next();
         FugeProfileId = query_fuge.value(0).toInt();                                // <-- Save FugeRunProfileID
         qDebug() << "FugeId: " << query_fuge.value(0).toInt();
      }
      else
      {
         QString errmsg   = "Create record error: " + query_fuge.lastError().text();;
         QMessageBox::critical( this,
            tr( "*ERROR* in Submitting Protocol" ),
            tr( "An error occurred in the attempt to submit"
                " protocol to AUC DB\n  %1 table\n  %2 ." ).arg( qrytab_fuge ).arg( errmsg ) );
         return;
      }

      // ExperimentDefinition INSERT ////////////////////////////////////////////////////////////////////////
      QString tabname_expdef( "ExperimentDefinition" );
      QString qrytab_expdef  = "\"" + schname + "\".\"" + tabname_expdef + "\"";
      int ExpDefId = 0;
      QString runname = mainw->currProto.runname;

      QSqlQuery query_expdef(dbxpn);

       /* WHAT TO INSERT: fields
          "CellCount": " ",
          "Comments": " ",            >> Some defualt comment should be inserted
          "FugeRunProfileId": " ",    >> reference to the profile used; must be in the database when this object is added
          "Name": " ",
          "Project": " ",
          "Researcher": " ",
       */

      QString cellcount            = QString::number(ncells);
      QString fugeprofile          = QString::number(FugeProfileId);

      QStringList researcher_split = (mainw->currProto.investigator).split(':');
      QString researcher_trimmed   = researcher_split[1].trimmed();
      QRegExp rx( "[^A-Za-z0-9_-, ]" );
      researcher_trimmed.replace( rx,  "" );
      QString researcher           = "\'" + researcher_trimmed + "\'";


      QString name                 = "\'" + runname + "\'";
      QString project_name         = mainw->currProto.project;
      QRegExp rx1( "[^A-Za-z0-9_-]" );
      project_name.replace( rx1,  "_" );
      
      
      QString exp_comments         = "\'Run by " + researcher_trimmed + ": "
                                     + mainw->currProto.runname + " based on project "
                                     + project_name + "\'";

      qDebug() << "EXP_COMMENT: " << exp_comments;
      //return;

      // Query
      if(! query_expdef.prepare(QString("INSERT INTO %1 (\"CellCount\",\"Comments\",\"FugeRunProfileId\",\"Name\",\"Researcher\") VALUES (%2, %3, %4, %5, %6) RETURNING \"ExperimentId\"")
            .arg(qrytab_expdef)
            .arg(cellcount)
            .arg(exp_comments)
            .arg(fugeprofile)
            .arg(name)
            .arg(researcher) ) )
         qDebug() << query_expdef.lastError().text();

      if (query_expdef.exec())
      {
         query_expdef.next();
         ExpDefId = query_expdef.value(0).toInt();                                // <-- Save ExpDefID
         qDebug() << "ExpDefId: " << query_expdef.value(0).toInt();

         qDebug() << "ExperimentDefinition record created";

         //protocol_details[ "experimentId" ]  = QString::number(405);
         protocol_details[ "experimentId" ]   = QString::number(ExpDefId);        // this should be put into new table connceting protocol && experiment
         protocol_details[ "experimentName" ] = runname;
         protocol_details[ "protocolName" ]   = currProto->protoname;             // pass also to Live Update/PostProd protocol name

	 int nwavl_tot = 0;
	 for ( int kk = 0; kk < rpRange->nranges; kk++ )
         {
            nwavl_tot  += rpRange->chrngs[ kk ].wvlens.count();
         }
	 
         //ALEXEY: when interference ? (divide by 2!!! )
         if ( ncells_interference && !nwavl_tot )       //Interference ONLY
	   {
	     protocol_details[ "CellChNumber" ]   = QString::number( int(ncells_interference/2 ));
	     qDebug() << "ITF: CellChNumber: " << protocol_details[ "CellChNumber" ];
	   }
	 else if ( !ncells_interference && nwavl_tot )  //Absorbance ONLY
	   {
	     protocol_details[ "CellChNumber" ]   = QString::number(rpSolut->nschan); // this can be read from protocol in US-lims DB
	     qDebug() << "UV: CellChNumber: " << protocol_details[ "CellChNumber" ];
	   }
	 else if ( ncells_interference && nwavl_tot )   // BOTH
	   {
	     QString cellnumber =  QString("IP:") + QString::number( int(ncells_interference/2 ) ) + QString(",RI:")  + QString::number(rpSolut->nschan);
	     protocol_details[ "CellChNumber" ] = cellnumber;

	     qDebug() << "UV & ITF: cellnumber: " << cellnumber;
	     qDebug() << "UV & ITF: CellChNumber: " << protocol_details[ "CellChNumber" ];
	                                          
    	   }
	   
         protocol_details[ "duration" ]       = QString::number(Total_duration);
         protocol_details[ "invID_passed" ]   = QString::number(US_Settings::us_inv_ID());
         protocol_details[ "correctRadii" ]   = QString("YES");
         protocol_details[ "expAborted" ]     = QString("NO");

         protocol_details[ "label" ]          = currProto->exp_label;

         QString gmpRun_str;
         if ( mainw->usmode )
	   gmpRun_str = "NO";
         else
	   gmpRun_str = "YES";
	 
         protocol_details[ "gmpRun" ]         = gmpRun_str;

         protocol_details[ "aprofileguid" ]   = currProto->protoGUID;



         //ALEXEY: when interference ? (always 1!!)
         if ( ncells_interference && !nwavl_tot  )    // Interference ONLY
	   {
	     protocol_details[ "TripleNumber" ] = QString::number(1 * int(ncells_interference/2 ));
	     qDebug() << "ITF: TripleNumber: " << protocol_details[ "TripleNumber" ];
	   }
	 else if ( !ncells_interference && nwavl_tot ) //Absorbance ONLY
	   {
	     protocol_details[ "TripleNumber" ] = QString::number(nwavl_tot);
	     qDebug() << "UV: TripleNumber: " << protocol_details[ "TripleNumber" ];	     
	   }
	 else if ( ncells_interference && nwavl_tot )  // BOTH
	   {
	     //int triples_num = int(ncells_interference/2 ) + nwavl_tot;
	     //protocol_details[ "TripleNumber" ] = QString::number(triples_num);
	     
	     QString triplenumber = QString("IP:") + QString::number(1 * int(ncells_interference/2 )) + QString(",RI:")  + QString::number(nwavl_tot);    
	     protocol_details[ "TripleNumber" ] = triplenumber;
	     
	     qDebug() << "UV + ITF: triplenumber: " << triplenumber;
	     qDebug() << "UV + ITF: TripleNumber: " << protocol_details[ "TripleNumber" ];
	                                         
	   }

         protocol_details[ "OptimaName" ]   = rpRotor->instrname;
	 protocol_details[ "operatorID" ]   = QString::number( rpRotor->operID );
         //protocol_details[ "OptimaName" ]   = mainw->currentInstrument[ "name" ];
      }
      else
      {
         QString errmsg   = "Create record error: " + query_expdef.lastError().text();;
         QMessageBox::critical( this,
            tr( "*ERROR* in Submitting Protocol" ),
            tr( "An error occurred in the attempt to submit"
                " protocol to AUC DB\n  %1 table\n  %2 ." ).arg( qrytab_expdef ).arg( errmsg ) );
         return;
      }
   }
   else
   { // Inform user of failure and give instructions
      QString mtitle    = tr( "Failed Connection to Optima" );
      QString message   = tr( "The failure to connect to the Optima most likely means\n"
                              "that host/port/name/user are misconfigured.\n"
                              "Reset them in UltraScan's 'Optima Host Preferences'\n"
                              "and return to retry connecting here." );
      QMessageBox::critical( this, mtitle, message );

      return;
   }
   //submitted    = true;

   ck_sub_done->setChecked( true );

   if ( !mainw->automode )
     {
       QString mtitle_done    = tr( "Success" );
       QString message_done   = tr( "Protocol has been successfully submitted to Optima DB." );
       QMessageBox::information( this, mtitle_done, message_done );
     }

   //Make 'autoflow' table record:
   if ( mainw->automode )
     add_autoflow_record( protocol_details );
   
   emit expdef_submitted( protocol_details );
}

// Read Protocol details
void US_ExperGuiUpload::add_autoflow_record( QMap< QString, QString> & protocol_details )
{
  qDebug() << "GMPRUN FIELD: " << protocol_details[ "gmpRun" ];

  // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
   {
      QMessageBox::warning( this, tr( "Connection Problem" ),
                                  tr( "Read protocol: Could not connect to database \n" )
                                     + db->lastError() );
      return;
   }
   
   if ( db != NULL )
   {
      QStringList qry;
      qry << "add_autoflow_record"
          << protocol_details[ "protocolName" ]
          << protocol_details[ "CellChNumber" ]
          << protocol_details[ "TripleNumber" ]
          << protocol_details[ "duration" ]
          << protocol_details[ "experimentName" ]
          << protocol_details[ "experimentId" ]
          << protocol_details[ "OptimaName" ]
          << protocol_details[ "invID_passed" ]
          << protocol_details[ "label" ]
          << protocol_details[ "gmpRun" ]
          << protocol_details[ "aprofileguid" ]
	  << protocol_details[ "operatorID" ];

      db->statusQuery( qry );
      //db->query( qry );
      protocol_details[ "autoflowID" ] = QString::number( db->lastInsertID() );
   }

   qDebug() << "Generated AUTOFLOW ID : " <<  protocol_details[ "autoflowID" ];

   /***/
   //Also, create record in autoflowStages table:
   QStringList qry_stages;
   qry_stages << "add_autoflow_stages_record" << protocol_details[ "autoflowID" ];
   db->statusQuery( qry_stages );
   /**/
   
}

// Function to build a Json object and document holding experiment controls
QString US_ExperGuiUpload::buildJson( void )
{
   QString       js_exper("");
#if QT_VERSION > 0x050000

   // Accumulate information on controls that have been specified
   int nused         = sibIValue( "cells",     "nused" );
   int nspeed        = sibIValue( "speeds",    "nspeeds" );
   int nchant        = sibIValue( "solution",  "nchant" );
   int nchanf        = sibIValue( "solution",  "nchanf" );
   int nchanu        = sibIValue( "solution",  "nchanu" );
   int nuniqs        = sibIValue( "solutions", "nusols" );
   int solu_done     = sibIValue( "solutions", "alldone" );
   int rngs_done     = sibIValue( "ranges",    "alldone" );
   QString v_run     = sibSValue( "general",   "runID" );
   QString v_proj    = sibSValue( "general",   "project" );
   QString v_invid   = sibSValue( "general",   "investigator" );
   QString v_dbdisk  = sibSValue( "general",   "dbdisk" );
   QString v_lab     = sibSValue( "rotor",     "lab" );
   QString v_rotor   = sibSValue( "rotor",     "rotor" );
   QString v_calib   = sibSValue( "rotor",     "calib" );
   QString v_centp   = QString::number( nused );
   QString v_nspeed  = QString::number( nspeed );
   QString v_nsolct  = QString::number( nchant );
   QString v_nsolcf  = QString::number( nchanf );
   QString v_nsolcu  = QString::number( nchanu );
   QString v_nsolun  = QString::number( nuniqs );
DbgLv(1) << "EGUp:bj: n speed,chant,usols" << nspeed << nchant << nuniqs;

   QStringList sspeed   = sibLValue( "speeds",    "profiles" );
DbgLv(1) << "EGUp:bj: speed profiles" << sspeed;
   QStringList scentp   = sibLValue( "cells",     "centerpieces" );
DbgLv(1) << "EGUp:bj: cells centerpieces" << scentp;
   QStringList ssolut   = sibLValue( "solutions", "channel_solutions" );
DbgLv(1) << "EGUp:bj: solus solus" << ssolut;

   bool chk_run      = ! v_run .isEmpty();
   bool chk_project  = ! v_proj.isEmpty();
   bool chk_centerp  = ( nused  > 0 );
   bool chk_solution = ( solu_done > 0 );
   bool chk_ranges   = ( rngs_done > 0 );
DbgLv(1) << "EGUp:bj: ck: run proj cent solu epro"
 << chk_run << chk_project << chk_centerp << chk_solution << chk_ranges;
   bool chk_vars_set = ( chk_run       &&  chk_project   &&
                         chk_centerp   &&  chk_solution  &&
                         chk_ranges );
   bool chk_sub_enab = ( chk_vars_set  &&  connected );

   if ( ! chk_sub_enab )
      return js_exper;       // Parameterization incomplete: empty return

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


DbgLv(1) << "JSON_1";
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


DbgLv(1) << "JSON_2";
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


DbgLv(1) << "JSON_3";
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

DbgLv(1) << "JSON_4";

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

