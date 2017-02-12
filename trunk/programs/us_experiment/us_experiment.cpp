//! \file us_experiment.cpp

#include "us_experiment.h"
#include "us_rotor_gui.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define QRegularExpression(a)  QRegExp(a)
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief Main program for US_Experiment. Loads translators and starts
//         the class US_Experiment

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_Experiment w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// Constructor
US_Experiment::US_Experiment() : US_Widgets()
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
   epanGeneral         = new US_ExperGuiGeneral  ( this );
   epanRotor           = new US_ExperGuiRotor    ( this );
   epanSpeeds          = new US_ExperGuiSpeeds   ( this );
   epanCells           = new US_ExperGuiCells    ( this );
   epanSolutions       = new US_ExperGuiSolutions( this );
   epanPhotoMult       = new US_ExperGuiPhotoMult( this );
   epanUpload          = new US_ExperGuiUpload   ( this );

   tabWidget->addTab( epanGeneral,   tr( "1: General"          ) );
   tabWidget->addTab( epanRotor,     tr( "2: Lab/Rotor"        ) );
   tabWidget->addTab( epanSpeeds,    tr( "3: Speeds"           ) );
   tabWidget->addTab( epanCells,     tr( "4: Cells"            ) );
   tabWidget->addTab( epanSolutions, tr( "5: Solutions"        ) );
   tabWidget->addTab( epanPhotoMult, tr( "6: Photo Multiplier" ) );
   tabWidget->addTab( epanUpload,    tr( "7: Upload"           ) );
   tabWidget->setCurrentIndex( 0 );

   QLabel* lb_stat        = us_label( tr( "Status:" ) );
   le_stat = us_lineedit( tr( "1:u  2:u  3:u  4:u  5:u  6:u  7:u"
                              "   ('u'==unspecified  'X'==parameterized)" ) );
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

   setMinimumSize( QSize( 700, 400 ) );
   adjustSize();

   reset();
}

// Public function to return a parameter value
//   from a US_Experiment child panel
QString US_Experiment::childPValue( const QString child, const QString type )
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
   else if ( child == "photomult" )
   {
      value = epanPhotoMult->getPValue( type );
   }
   else if ( child == "upload" )
   {
      value = epanUpload   ->getPValue( type );
   }

   return value;
}

// Public function to return a parameter list value
//   from a US_Experiment child panel
QStringList US_Experiment::childPList( const QString child, const QString type )
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
   else if ( child == "photomult" )
   {
      value = epanPhotoMult->getPList( type );
   }
   else if ( child == "upload" )
   {
      value = epanUpload   ->getPList( type );
   }

   return value;
}

// Slot to handle a new panel selected
void US_Experiment::newPanel( int panx )
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
      epanPhotoMult->initPanel();
   else if ( panx == 6 )
      epanUpload   ->initPanel();

   // Update status text for all panels
   statUpdate();
}

// Slot to update status text for all panels
void US_Experiment::statUpdate()
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
                     epanPhotoMult->status() );
   stattext.replace( QRegularExpression( "7:[uX]" ),
                     epanUpload   ->status() );
DbgLv(1) << "statUpd:  MOD stat" << stattext;

   le_stat->setText( stattext );
}

// Slot to advance to the next panel
void US_Experiment::panelUp()
{
   int newndx = tabWidget->currentIndex() + 1;
   int maxndx = tabWidget->count() - 1;
DbgLv(1) << "panUp: newndx, maxndx" << newndx << maxndx;
   tabWidget->setCurrentIndex( qMin( newndx, maxndx ) );
}

// Slot to descend to the previous panel
void US_Experiment::panelDown()
{
   int newndx = tabWidget->currentIndex() - 1;
   tabWidget->setCurrentIndex( qMax( newndx, 0 ) );
}

// Reset parameters to their defaults
void US_Experiment::reset( void )
{
}

// Panel for run and other general parameters
US_ExperGuiGeneral::US_ExperGuiGeneral( QWidget* topw )
{
   mainw               = topw;
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "1: Specify run and other general parameters" ) );
   panel->addWidget( lb_panel );

   QGridLayout* genL   = new QGridLayout();

   QLabel*      lb_runid        = us_label( tr( "Run ID:" ) );
   QPushButton* pb_project      = us_pushbutton( tr( "Select Project" ) );
   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
                le_runid        = us_lineedit( "", 1, false );
                le_project      = us_lineedit( "", 1, false );

   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   int id = US_Settings::us_inv_ID();
   QString number  = ( id > 0 ) ?
      QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(), 1, false );

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

   connect( pb_project,      SIGNAL( clicked()        ), 
            this,            SLOT( sel_project()      ) );
   connect( pb_investigator, SIGNAL( clicked()        ), 
            this,            SLOT( sel_investigator() ) );

   // Read in centerpiece information and populate names list
   centerpieceInfo();
}

// Initialize a panel, especially after clicking on it tab
void US_ExperGuiGeneral::initPanel()
{
}

// Set a specific panel value
void US_ExperGuiGeneral::setPValue( const QString type, QString& value )
{
   if ( type == "run" )
   {
      le_runid->setText( value );
   }
   else if ( type == "project" )
   {
      le_project->setText( value );
   }
   else if ( type == "investigator" )
   {
   }
}

// Get a specific panel value
QString US_ExperGuiGeneral::getPValue( const QString type )
{
   QString value( "" );

   if ( type == "run" )
   {
      value = le_runid->text();
   }
   else if ( type == "project" )
   {
   }
   else if ( type == "investigator" )
   {
   }
   else if ( type == "dbdisk" )
   {
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
            ? ((US_Experiment*)mainw)->childPValue( sibling, type )
            : QString("") );
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiGeneral::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_Experiment*)mainw)->childPList( sibling, type )
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

// Capture selected project information
void US_ExperGuiGeneral::project_info( US_Project& project )
{
qDebug() << "projinfo: proj.desc" << project.projectDesc;
qDebug() << "projinfo: proj.guid" << project.projectGUID;

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
   genL->addWidget( cb_rotor,        row++, 2, 1, 1 );
   genL->addWidget( lb_calibr,       row,   0, 1, 1 );
   genL->addWidget( cb_calibr,       row++, 3, 1, 1 );
   genL->addItem  ( spacer1,          row++, 0, 1, 4 );
   genL->addWidget( pb_advrotor,     row++, 0, 1, 4 );

   panel->addLayout( genL );
   panel->addStretch();

   US_Passwd pw;
   US_DB2* dbP              = ( sibPValue( "general", "dbdisk" ) == "db" )
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
   connect( pb_advrotor,  SIGNAL( clicked()  ),
            this,         SLOT  ( advRotor() ) );

   changeLab( 0 );
};

// Initialize a panel, especially after clicking on it tab
void US_ExperGuiRotor::initPanel()
{
}

// Set a specific panel value
void US_ExperGuiRotor::setPValue( const QString type, QString& value )
{
   if ( type == "lab" )
   {
      cb_lab   ->setCurrentIndex( cb_lab->findText( value ) );
   }
   else if ( type == "rotor" )
   {
      cb_rotor ->setCurrentIndex( cb_rotor->findText( value ) );
   }
   else if ( type == "calibration" )
   {
      cb_calibr->setCurrentIndex( cb_rotor->findText( value ) );
   }
}

// Set a specific panel set of list values
void US_ExperGuiRotor::setPValue( const QString type, QStringList& values )
{
   if ( type == "lab" )
   {
      cb_lab->clear();
      for ( int ii = 0; ii < values.count(); ii++ )
      {
         cb_lab->addItem( values[ ii ] );
      }
   }
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
   else if ( type == "calibration" )
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
   else if ( type == "numHoles" )
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
      value << getPValue( "calibration" );
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiRotor::sibPValue( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_Experiment*)mainw)->childPValue( sibling, type )
            : QString("") );
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiRotor::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_Experiment*)mainw)->childPList( sibling, type )
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
qDebug() << "EXP:chgLab  ndx" << ndx;
   cb_lab->setCurrentIndex( ndx );
   QString clab        = cb_lab->currentText();
   int labID           = clab.section( ":", 0, 0 ).toInt();
   QString descr       = clab.section( ":", 1, 1 ).simplified();
qDebug() << "EXP: chgLab labID desc" << labID << descr;

   US_Passwd pw;
   US_DB2* dbP              = ( sibPValue( "general", "dbdisk" ) == "db" )
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
qDebug() << "EXP:chgRotor  ndx" << ndx;
   cb_rotor->setCurrentIndex( ndx );
   QString crot        = cb_rotor->currentText();
   int rotID           = crot.section( ":", 0, 0 ).toInt();
   QString descr       = crot.section( ":", 1, 1 ).simplified();
qDebug() << "EXP: chgRotor rotID desc" << rotID << descr;
   US_Passwd pw;
   US_DB2* dbP              = ( sibPValue( "general", "dbdisk" ) == "db" )
                              ? new US_DB2( pw.getPasswd() ) : NULL;
   if ( dbP != NULL )
   {
      US_Rotor::readCalibrationProfilesDB( calibs, rotID, dbP );
   }
   else
   {
      US_Rotor::readCalibrationProfilesDisk( calibs, rotID );
   }

qDebug() << "EXP: chgRotor calibs count" << calibs.count();
   sl_calibs.clear();

   for ( int ii = 0; ii < calibs.count(); ii++ )
   {
      sl_calibs << QString::number( calibs[ ii ].ID )
                 + ": " + calibs[ ii ].lastUpdated.toString( "d MMMM yyyy" );
   }

   cb_calibr->clear();
   cb_calibr->addItems( sl_calibs );
   int lndx            = calibs.count() - 1;
   if ( lndx >= 0 )
      cb_calibr->setCurrentIndex( lndx );
}

// Slot for click on Advanced Lab... button
void US_ExperGuiRotor::advRotor()
{
   US_Rotor::RotorCalibration  calibr;
   US_Rotor::Rotor             rotor;
   int calibx             = cb_calibr->currentIndex();
   int rotorx             = cb_rotor ->currentIndex();
   calibr.ID              = ( calibx >= 0 )
                            ? cb_calibr->currentText().section( ":", 0, 0 ).toInt()
                            : 0;
   rotor.ID               = ( rotorx >= 0 )
                            ? cb_rotor ->currentText().section( ":", 0, 0 ).toInt()
                            : 0;
qDebug() << "EXP: advR: IN rID cID" << rotor.ID << calibr.ID;
   int dbdisk             = ( sibPValue( "general", "dbdisk" ) == "db" )
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
   int rID             = crotor.ID;
   int cID             = ccalib.ID;
   int rx              = -1;
qDebug() << "EXP: advRChg: new rID cID" << rID << cID;

   for ( int ii = 0; ii < sl_rotors.count(); ii++ )
   {  // Find and select the list item matching the accepted rotor
      int eID             = sl_rotors[ ii ].section( ":", 0, 0 ).toInt();
      if ( eID == rID )
      {  // Match:  select item, set index, break from loop
         cb_rotor ->setCurrentIndex( ii );
         rx                  = ii;
qDebug() << "EXP: advRChg:   rID match at index" << ii;
         break;
      }
   }

   if ( rx >= 0 )
      changeRotor( rx );     // Rebuild calibrations for new rotor

   for ( int ii = 0; ii < sl_calibs.count(); ii++ )
   {  // Find and select the list item matching the accepted calibration
      int eID             = sl_calibs[ ii ].section( ":", 0, 0 ).toInt();
      if ( eID == cID )
      {
         cb_calibr->setCurrentIndex( ii );
qDebug() << "EXP: advRChg:   cID match at index" << ii;
         break;
      }
qDebug() << "EXP: advRChg:     ii eID" << ii << eID;
   }
}

// Panel for Speed step parameters
US_ExperGuiSpeeds::US_ExperGuiSpeeds( QWidget* topw )
{
   mainw               = topw;
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
   ct_count ->setSingleStep(   1 );
   ct_speed ->setSingleStep( 100 );
   ct_accel ->setSingleStep(  50 );
   ct_lenhr ->setSingleStep(   1 );
   ct_lenmin->setSingleStep(   1 );
   ct_dlyhr ->setSingleStep(   1 );
   ct_dlymin->setSingleStep(   1 );
   ct_count ->setValue(    1 );
   ct_speed ->setValue( 4500 );
   ct_accel ->setValue(  400 );
   ct_lenhr ->setValue(    5 );
   ct_lenmin->setValue(   30 );
   ct_dlyhr ->setValue(    0 );
   ct_dlymin->setValue(   30 );
   cb_prof->addItem( tr( "Speed Profile 1: 4500 rpm for 5 hr 30 min" ) );

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

   panel->addLayout( genL );
   panel->addStretch();
   adjustSize();
};

// Initialize a panel, especially after clicking on it tab
void US_ExperGuiSpeeds::initPanel()
{
}

// Set a specific panel value
void US_ExperGuiSpeeds::setPValue( const QString type, QString& value )
{
   if ( type == "nspeed" )
   {
      //cb_lab->setCurrentIndex( cb_lab->indexOf( value ) );
   }
qDebug() << "EGG:setPV: type value" << type << value;
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

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiSpeeds::getPList( const QString type )
{
   QStringList value( "" );

   if ( type == "profiles" )
   {
      value.clear();
      int nspeed  = getPValue( "nspeed" ).toInt();
      int curndx  = cb_prof->currentIndex();
      for ( int ii = 0; ii < nspeed; ii++ )
      {
         cb_prof->setCurrentIndex( ii );
         value << getPValue( "speed" );
         value << getPValue( "accel" );
         value << getPValue( "durhr" );
         value << getPValue( "durmin" );
         value << getPValue( "delayhr" );
         value << getPValue( "delaymin" );
      }

      cb_prof->setCurrentIndex( curndx );
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiSpeeds::sibPValue( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_Experiment*)mainw)->childPValue( sibling, type )
            : QString("") );
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiSpeeds::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_Experiment*)mainw)->childPList( sibling, type )
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
                   

// Panel for Cells parameters
US_ExperGuiCells::US_ExperGuiCells( QWidget* topw )
{
qDebug() << "EGC: IN";
   mainw               = topw;
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "4: Define cell usage" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   QPushButton* pb_advrotor = us_pushbutton( tr( "Reset from Rotor information" ) );
   genL->addWidget( pb_advrotor, 0, 0, 1, 8 );

   QLabel* lb_hdr1     = us_banner( tr( "Cell" ) );
   QLabel* lb_hdr2     = us_banner( tr( "Centerpiece" ) );
   QLabel* lb_hdr3     = us_banner( tr( "Windows" ) );
   genL->addWidget( lb_hdr1,     1, 0, 1, 2 );
   genL->addWidget( lb_hdr2,     1, 2, 1, 4 );
   genL->addWidget( lb_hdr3,     1, 6, 1, 2 );

   cpnames             = sibPList( "general", "centerpieces" );
   int krow            = 2;
   const int mxcels    = 8;
   int nholes          = sibPValue( "rotor", "numHoles" ).toInt();
qDebug() << "EGC:  numHoles mxcels" << nholes << mxcels;

   for ( int ii = 0; ii < mxcels; ii++ )
   {
      QString scel        = tr( "cell %1" ).arg( ii + 1 );
      QLabel* clabl       = us_label( scel );
      QComboBox* cb_cenp  = us_comboBox();
      QComboBox* cb_wind  = us_comboBox();

      genL->addWidget( clabl,   krow,  0, 1, 2 );
      genL->addWidget( cb_cenp, krow,  2, 1, 4 );
      genL->addWidget( cb_wind, krow,  6, 1, 2 );
      krow++;

      cb_cenp->addItem( tr( "empty" ) );
      cb_cenp->addItems( cpnames );
      cb_wind->addItem( tr( "quartz" ) );
      cb_wind->addItem( tr( "sapphire" ) );

      // Save pointers to row objects for later update
      cc_labls << clabl;
      cc_cenps << cb_cenp;
      cc_winds << cb_wind;
   }

//   connect( pb_advrotor,  SIGNAL( clicked()    ),
//            this,         SLOT  ( statUpdate() ) );

   panel->addLayout( genL );
   panel->addStretch();

   initPanel();
QString pval1 = sibPValue( "rotor", "rotor" );
qDebug() << "EGC: rotor+rotor=" << pval1;
};

// Initialize a panel, especially after clicking on it tab
void US_ExperGuiCells::initPanel()
{
   const int mxcels    = 8;
   int nholes          = sibPValue( "rotor", "numHoles" ).toInt();
   int icbal           = nholes - 1;     // Counter-balance index
   int icbsib          = nholes / 2 - 1; // Its sibling
   QStringList sl_bals;
   sl_bals << "empty counterbalance"
           << "Titanium counterbalance"
           << "CarbonFiber counterbalance"
           << "Epon counterbalance";
qDebug() << "EGC:initP:  numHoles mxcels" << nholes << mxcels
 << "icbal icbsib" << icbal << icbsib;

   for ( int ii = 0; ii < mxcels; ii++ )
   {
qDebug() << "EGC:initP:   ii cenps-count" << ii << cc_cenps[ii]->count();
      bool make_vis       = ( ii < nholes );
      cc_labls[ ii ]->setVisible( make_vis );
      cc_cenps[ ii ]->setVisible( make_vis );
      cc_winds[ ii ]->setVisible( make_vis );

      if ( ii == icbal )
      {
         cc_cenps[ ii ]->clear();
         cc_cenps[ ii ]->addItems( sl_bals );
      }
   }
}

// Set a specific panel value
void US_ExperGuiCells::setPValue( const QString type, QString& value )
{
   if ( type == "lab" )
   {
      //cb_lab->setCurrentIndex( cb_lab->indexOf( value ) );
   }
qDebug() << "EGG:setPV: type value" << type << value;
}

// Get a specific panel value
QString US_ExperGuiCells::getPValue( const QString type )
{
   QString value( "" );

   if ( type == "ncells" )
   {
   //   value = cb_lab->currentText();
   }
   else if ( type == "1:centerpiece" )
   {
   }
   else if ( type == "1:windows" )
   {
   }
   else if ( type == "2:centerpiece" )
   {
   }
   else if ( type == "2:windows" )
   {
   }
   else if ( type == "3:centerpiece" )
   {
   }
   else if ( type == "3:windows" )
   {
   }
   else if ( type == "4:centerpiece" )
   {
   }
   else if ( type == "4:windows" )
   {
   }
   else if ( type == "5:centerpiece" )
   {
   }
   else if ( type == "5:windows" )
   {
   }
   else if ( type == "6:centerpiece" )
   {
   }
   else if ( type == "6:windows" )
   {
   }
   else if ( type == "7:centerpiece" )
   {
   }
   else if ( type == "7:windows" )
   {
   }
   else if ( type == "8:centerpiece" )
   {
   }
   else if ( type == "8:windows" )
   {
   }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiCells::getPList( const QString type )
{
   QStringList value( "" );

   if ( type == "cellinfo" )
   {
      //value << le_runid->text();
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiCells::sibPValue( const QString sibling, const QString type )
{
   QString value( "" );
qDebug() << "EGC:cPV: IN sibling" << sibling << "type" << type;
qDebug() << "EGC:cPV: mainw" << mainw;
   if ( mainw != NULL )
   {
      value = ((US_Experiment*)mainw)->childPValue( sibling, type );
   }

   return value;
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiCells::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_Experiment*)mainw)->childPList( sibling, type )
            : QStringList() );
}

// Return status string for the panel
QString US_ExperGuiCells::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
QString pval1 = sibPValue( "rotor", "rotor" );
qDebug() << "EGC:st: rotor+rotor=" << pval1;
bool is_done=false;
   return ( is_done ? QString( "4:X" ) : QString( "4:u" ) );
}
                   

// Panel for Solutions parameters
US_ExperGuiSolutions::US_ExperGuiSolutions( QWidget* topw )
{
   mainw               = topw;
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "5: Specify solutions for each cell" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   QPushButton* pb_advparam = us_pushbutton( tr( "Fill from other Panel information" ) );

   int row=0;
   genL->addWidget( pb_advparam,     row++, 0, 1, 4 );

//   connect( pb_advparam,  SIGNAL( clicked()    ),
//            this,         SLOT  ( statUpdate() ) );

   panel->addLayout( genL );
   panel->addStretch();
};

// Initialize a panel, especially after clicking on it tab
void US_ExperGuiSolutions::initPanel()
{
}

// Set a specific panel value
void US_ExperGuiSolutions::setPValue( const QString type, QString& value )
{
   if ( type == "lab" )
   {
      //cb_lab->setCurrentIndex( cb_lab->indexOf( value ) );
   }
qDebug() << "EGG:setPV: type value" << type << value;
}

// Get a specific panel value
QString US_ExperGuiSolutions::getPValue( const QString type )
{
   QString value( "" );

   if ( type == "1:solution" )
   {
   //   value = cb_lab->currentText();
   }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiSolutions::getPList( const QString type )
{
   QStringList value( "" );

   if ( type == "solutions" )
   {
      //value << le_runid->text();
   }
   else if ( type == "buffers" )
   {
      //value = cp_names;
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiSolutions::sibPValue( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_Experiment*)mainw)->childPValue( sibling, type )
            : QString("") );
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiSolutions::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_Experiment*)mainw)->childPList( sibling, type )
            : QStringList() );
}

// Return status string for the panel
QString US_ExperGuiSolutions::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
   return ( is_done ? QString( "5:X" ) : QString( "5:u" ) );
}
                   

// Panel for Photo Multiplier parameters
US_ExperGuiPhotoMult::US_ExperGuiPhotoMult( QWidget* topw )
{
   mainw               = topw;
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "6: Specify photo multiplier, voltage" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   QPushButton* pb_advparam = us_pushbutton( tr( "Fill from other Panel information" ) );

   int row=0;
   genL->addWidget( pb_advparam,     row++, 0, 1, 4 );

//   connect( pb_advparam,  SIGNAL( clicked()    ),
//            this,         SLOT  ( statUpdate() ) );

   panel->addLayout( genL );
   panel->addStretch();
};

// Initialize a panel, especially after clicking on it tab
void US_ExperGuiPhotoMult::initPanel()
{
}

// Set a specific panel value
void US_ExperGuiPhotoMult::setPValue( const QString type, QString& value )
{
   if ( type == "lab" )
   {
      //cb_lab->setCurrentIndex( cb_lab->indexOf( value ) );
   }
qDebug() << "EGG:setPV: type value" << type << value;
}

// Get a specific panel value
QString US_ExperGuiPhotoMult::getPValue( const QString type )
{
   QString value( "" );

   if ( type == "eprofiles" )
   {
   //   value = cb_lab->currentText();
   }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiPhotoMult::getPList( const QString type )
{
   QStringList value( "" );

   if ( type == "eprofiles" )
   {
      //value << le_runid->text();
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiPhotoMult::sibPValue( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_Experiment*)mainw)->childPValue( sibling, type )
            : QString("") );
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiPhotoMult::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_Experiment*)mainw)->childPList( sibling, type )
            : QStringList() );
}

// Return status string for the panel
QString US_ExperGuiPhotoMult::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
   return ( is_done ? QString( "6:X" ) : QString( "6:u" ) );
}
                   

// Panel for Uploading parameters to Optima XLA DB
US_ExperGuiUpload::US_ExperGuiUpload( QWidget* topw )
{
   mainw               = topw;
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "7: Upload experiment parameters to Optima XLA DB" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   QPushButton* pb_advparam = us_pushbutton( tr( "Fill from other Panel information" ) );

   int row=0;
   genL->addWidget( pb_advparam,     row++, 0, 1, 4 );

//   connect( pb_advparam,  SIGNAL( clicked()    ),
//            this,         SLOT  ( statUpdate() ) );

   panel->addLayout( genL );
   panel->addStretch();
};

// Initialize a panel, especially after clicking on it tab
void US_ExperGuiUpload::initPanel()
{
}

// Set a specific panel value
void US_ExperGuiUpload::setPValue( const QString type, QString& value )
{
   if ( type == "lab" )
   {
      //cb_lab->setCurrentIndex( cb_lab->indexOf( value ) );
   }
qDebug() << "EGG:setPV: type value" << type << value;
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
   return ( mainw != NULL
            ? ((US_Experiment*)mainw)->childPValue( sibling, type )
            : QString("") );
}

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiUpload::sibPList( const QString sibling, const QString type )
{
   return ( mainw != NULL
            ? ((US_Experiment*)mainw)->childPList( sibling, type )
            : QStringList() );
}

// Return status string for the panel
QString US_ExperGuiUpload::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
   return ( is_done ? QString( "7:X" ) : QString( "7:u" ) );
}

