//! \file us_experiment.cpp

#include "us_experiment.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
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
   epanGeneral         = new US_ExperGuiGeneral();
   epanRotor           = new US_ExperGuiRotor( );
   epanSpeeds          = new US_ExperGuiSpeeds( );
   epanCells           = new US_ExperGuiCells( );
   epanSolutions       = new US_ExperGuiSolutions( );
   epanPhotoMult       = new US_ExperGuiPhotoMult( );
   epanUpload          = new US_ExperGuiUpload( );

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

// Slot to handle a new panel selected
void US_Experiment::newPanel( int panx )
{
DbgLv(1) << "newPanel panx=" << panx << "prev.panx=" << curr_panx;
   curr_panx        = panx;
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
US_ExperGuiGeneral::US_ExperGuiGeneral()
{
   QVBoxLayout* panel     = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel = us_banner( tr( "1: Specify run and other general parameters" ) );
   panel->addWidget( lb_panel );

   QGridLayout* genL      = new QGridLayout();

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
};

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

   return value;
}

// Return status string for the panel
QString US_ExperGuiGeneral::status()
{
   bool is_done  = ( ! le_runid->text().isEmpty() &&
                     ! le_project->text().isEmpty() );
   return ( is_done ? QString( "1:X" ) : QString( "1:u" ) );
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


// Panel for Lab/Rotor parameters
US_ExperGuiRotor::US_ExperGuiRotor()
{
   QVBoxLayout* panel     = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel = us_banner( tr( "2: Specify lab/rotor parameters" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL      = new QGridLayout();

   QLabel*      lb_lab          = us_label( tr( "Laboratory:" ) );
   QLabel*      lb_rotor        = us_label( tr( "Rotor:" ) );
   QLabel*      lb_calibr       = us_label( tr( "Calibration:" ) );
   QPushButton* pb_advrotor     = us_pushbutton( tr( "Advanced Lab/Rotor/Calibration" ) );
                cb_lab          = new QComboBox( this );
                cb_rotor        = new QComboBox( this );
                cb_calibr       = new QComboBox( this );

   int row=0;
   genL->addWidget( lb_lab,          row,   0, 1, 1 );
   genL->addWidget( cb_lab,          row++, 1, 1, 3 );
   genL->addWidget( lb_rotor,        row,   0, 1, 1 );
   genL->addWidget( cb_rotor,        row++, 1, 1, 3 );
   genL->addWidget( lb_calibr,       row,   0, 1, 1 );
   genL->addWidget( cb_calibr,       row++, 1, 1, 3 );
   genL->addWidget( pb_advrotor,     row++, 0, 1, 4 );

   panel->addLayout( genL );
   panel->addStretch();
};

// Set a specific panel value
void US_ExperGuiRotor::setPValue( const QString type, QString& value )
{
   if ( type == "lab" )
   {
      cb_lab->setCurrentIndex( cb_lab->findText( value ) );
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

   return value;
}

// Return status string for the panel
QString US_ExperGuiRotor::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
   return ( is_done ? QString( "2:X" ) : QString( "2:u" ) );
}
                   

// Panel for Speed step parameters
US_ExperGuiSpeeds::US_ExperGuiSpeeds()
{
   QVBoxLayout* panel     = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel = us_banner( tr( "3: Specify speed steps" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL      = new QGridLayout();

   QLabel* lb_count       = us_label( tr( "Number of Speed Profiles:" ) );
   QLabel* lb_lenhr       = us_label( tr( "Length of Experiment (Hours):" ) );
   QLabel* lb_lenmin      = us_label( tr( "Length of Experiment (Minutes):" ) );
   QLabel* lb_dlyhr       = us_label( tr( "Time Delay for Scans (Hours):" ) );
   QLabel* lb_dlymin      = us_label( tr( "Time Delay for Scans (Minutes):" ) );
   QLabel* lb_speed       = us_label( tr( "Rotor Speed (rpm):" ) );
   QLabel* lb_accel       = us_label( tr( "Acceleration Profile (rpm/sec):" ) );
   QComboBox* cb_prof     = new QComboBox( this );
   QwtCounter* ct_speed   = us_counter( 2, 1000, 100000, 100 );
   QwtCounter* ct_accel   = us_counter( 2,   50,   1000,  50 );
   QwtCounter* ct_count   = us_counter( 2,    1,    100,   1 );
   QwtCounter* ct_lenhr   = us_counter( 2,    0,    100,   1 );
   QwtCounter* ct_lenmin  = us_counter( 2,    0,     60,   1 );
   QwtCounter* ct_dlyhr   = us_counter( 2,    0,     10,   1 );
   QwtCounter* ct_dlymin  = us_counter( 2,    0,     60,   1 );
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

// Set a specific panel value
void US_ExperGuiSpeeds::setPValue( const QString type, QString& value )
{
   if ( type == "lab" )
   {
      //cb_lab->setCurrentIndex( cb_lab->indexOf( value ) );
   }
qDebug() << "EGG:setPV: type value" << type << value;
}

// Get a specific panel value
QString US_ExperGuiSpeeds::getPValue( const QString type )
{
   QString value( "" );

   if ( type == "lab" )
   {
   //   value = cb_lab->currentText();
   }

   return value;
}

// Return status string for the panel
QString US_ExperGuiSpeeds::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
   return ( is_done ? QString( "3:X" ) : QString( "3:u" ) );
}
                   

// Panel for Cells parameters
US_ExperGuiCells::US_ExperGuiCells()
{
   QVBoxLayout* panel     = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel = us_banner( tr( "4: Define cell usage" ) );
   panel->addWidget( lb_panel );
};

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

   if ( type == "lab" )
   {
   //   value = cb_lab->currentText();
   }

   return value;
}

// Return status string for the panel
QString US_ExperGuiCells::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
   return ( is_done ? QString( "4:X" ) : QString( "4:u" ) );
}
                   

// Panel for Solutions parameters
US_ExperGuiSolutions::US_ExperGuiSolutions()
{
   QVBoxLayout* panel     = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel = us_banner( tr( "5: Specify solutions for each cell" ) );
   panel->addWidget( lb_panel );
};

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

   if ( type == "lab" )
   {
   //   value = cb_lab->currentText();
   }

   return value;
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
US_ExperGuiPhotoMult::US_ExperGuiPhotoMult()
{
   QVBoxLayout* panel     = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel = us_banner( tr( "6: Specify photo multiplier, voltage" ) );
   panel->addWidget( lb_panel );
};

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

   if ( type == "lab" )
   {
   //   value = cb_lab->currentText();
   }

   return value;
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
US_ExperGuiUpload::US_ExperGuiUpload()
{
   QVBoxLayout* panel     = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel = us_banner( tr( "7: Upload experiment parameters to Optima XLA DB" ) );
   panel->addWidget( lb_panel );
};

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

// Return status string for the panel
QString US_ExperGuiUpload::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
   return ( is_done ? QString( "7:X" ) : QString( "7:u" ) );
}
                   
