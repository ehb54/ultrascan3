//! \file us_expinfo.cpp

#include "us_expinfo.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_investigator.h"

US_ExpInfo::US_ExpInfo( ExperimentInfo& dataIn ) :
   US_WidgetsDialog( 0, 0 ), expInfo( dataIn )
{
   setWindowTitle( tr( "Experiment Information" ) );
   setPalette( US_GuiSettings::frameColor() );

   // Everything will be in the main layout
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;

   // Database choices
   QStringList DB = US_Settings::defaultDB();
   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   main->addWidget( lb_DB, row++, 0, 1, 2 );

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( selectInvestigator() ) );
   pb_investigator->setEnabled( true );
   main->addWidget( pb_investigator, row, 0 );

   le_investigator = us_lineedit( "", 1 );
   le_investigator->setReadOnly( true );
   main->addWidget( le_investigator, row++, 1 );

   // Current experiment information
   QLabel* lb_experiment_banner = us_banner( tr( "Experiment: " ) );
   main->addWidget( lb_experiment_banner, row++, 0, 1, 2 );

   // Select experiment
   QLabel* lb_experiment = us_label( tr( "Double-click to select, or" ) );
   main->addWidget( lb_experiment, row, 0 );
   pb_newExperiment = us_pushbutton( tr( "Create New" ) );
   connect( pb_newExperiment, SIGNAL( clicked() ), SLOT( newExperiment() ) );
   main->addWidget( pb_newExperiment, row++, 1 );

   lw_experiment = us_listwidget();
   connect( lw_experiment, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
                           SLOT  ( selectExperiment ( QListWidgetItem* ) ) );
   main->addWidget( lw_experiment, row, 0, 8, 2 );
   row += 8;

   // Experiment label
   QLabel* lb_label = us_label( tr( "Label:" ) );
   main->addWidget( lb_label, row++, 0, 1, 2 );
   le_label = us_lineedit();
   main->addWidget( le_label, row++, 0, 1, 2 );

   // Project
   QLabel* lb_project = us_label( tr( "Project:" ) );
   main->addWidget( lb_project, row, 0 );
   cb_project = us_projectComboBox();
   main->addWidget( cb_project, row++, 1 );

   // Experiment type
   QLabel* lb_expType = us_label( tr( "Experiment Type:" ) );
   main->addWidget( lb_expType, row, 0 );
   cb_expType = us_expTypeComboBox();
   main->addWidget( cb_expType, row++, 1 );

   // labID
   QLabel* lb_lab = us_label( tr( "Lab:" ) );
   main->addWidget( lb_lab, row, 0 );
   cb_lab = us_labComboBox();
   main->addWidget( cb_lab, row++, 1 );

   // instrumentID
   QLabel* lb_instrument = us_label( tr( "Instrument:" ) );
   main->addWidget( lb_instrument, row, 0 );
   cb_instrument = us_instrumentComboBox();
   main->addWidget( cb_instrument, row++, 1 );

   // operatorID
   QLabel* lb_operator = us_label( tr( "Operator:" ) );
   main->addWidget( lb_operator, row, 0 );
   cb_operator = us_operatorComboBox();
   main->addWidget( cb_operator, row++, 1 );

   // Rotor used in experiment
   QLabel* lb_rotor = us_label( tr( "Rotor:" ) );
   main->addWidget( lb_rotor, row, 0 );
   cb_rotor = us_rotorComboBox();
   main->addWidget( cb_rotor, row++, 1 );

   // Run Temperature
   QLabel* lb_runTemp = us_label( tr( "Run Temperature:" ) );
   main->addWidget( lb_runTemp, row++, 0, 1, 2 );
   le_runTemp = us_lineedit();
   main->addWidget( le_runTemp, row++, 0, 1, 2 );

   // Centrifuge Protocol
   QLabel* lb_centrifugeProtocol = us_label( tr( "Centrifuge Protocol:" ) );
   main->addWidget( lb_centrifugeProtocol, row++, 0, 1, 2 );
   le_centrifugeProtocol = us_lineedit();
   main->addWidget( le_centrifugeProtocol, row++, 0, 1, 2 );

   // Experiment comments
   QLabel* lb_comment = us_label( tr( "Comments:" ) );
   main->addWidget( lb_comment, row++, 0, 1, 2 );

   te_comment = us_textedit();
   main->addWidget( te_comment, row, 0, 4, 2 );
   te_comment->setMinimumWidth( 320 );
   te_comment->setReadOnly( false );
   row += 4;

   // Some pushbuttons
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   pb_accept = us_pushbutton( tr( "Accept" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept() ) );
   buttons->addWidget( pb_accept );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
   buttons->addWidget( pb_cancel );

   // Let's see if an investigator has been selected
   reset();
   main->addLayout( buttons, row++, 0, 1, 2 );
}

void US_ExpInfo::reset( void )
{
   le_investigator      ->clear();
   lw_experiment        ->clear();
   le_runTemp           ->clear();
   le_label             ->clear();
   le_centrifugeProtocol->clear();
   te_comment           ->clear();

   cb_project           ->setCurrentIndex( 0 );
   cb_lab               ->setCurrentIndex( 0 );
   cb_instrument        ->setCurrentIndex( 0 );
   cb_operator          ->setCurrentIndex( 0 );
   cb_rotor             ->setCurrentIndex( 0 );

   pb_newExperiment->setEnabled( false );
   pb_accept       ->setEnabled( false );

   if ( expInfo.invID > 0 )
   {
      // Investigator
      le_investigator->setText( "InvID (" + QString::number( expInfo.invID ) + "): " +
               expInfo.lastName + ", " + expInfo.firstName );

      // List of experiments
      getExperimentDesc();
      lw_experiment->clear();
      foreach( listInfo info, experimentList )
         lw_experiment->addItem( new QListWidgetItem(
               info.text, lw_experiment ) );

      pb_newExperiment->setEnabled( true );

      if ( expInfo.expID > 0 )
      {
         // Update controls to represent selected experiment
         le_runTemp           ->setText( expInfo.runTemp                      );
         le_label             ->setText( expInfo.label                        );
         te_comment           ->setText( expInfo.comments                     );
         le_centrifugeProtocol->setText( expInfo.centrifugeProtocol           );
   
         setWidgetIndex  ( lw_experiment, experimentList, expInfo.expID        );
         setComboBoxIndex( cb_project,    projectList,    expInfo.projectID    );
         setComboBoxIndex( cb_lab,        labList,        expInfo.labID        );
         setComboBoxIndex( cb_instrument, instrumentList, expInfo.instrumentID );
         setComboBoxIndex( cb_operator,   operatorList,   expInfo.operatorID   );
         setComboBoxIndex( cb_rotor,      rotorList,      expInfo.rotorID      );
      
         // Experiment types combo
         for ( uint i = 0; i < sizeof( experimentTypes); i++ )
         {
            if ( experimentTypes[ i ].toUpper() == expInfo.expType.toUpper() )
            {
               cb_expType->setCurrentIndex( i );
               break;
            }
         }

         pb_accept       ->setEnabled( true );
      }
   }
}

void US_ExpInfo::accept( void )
{
   // Get data directly from the form
   ExperimentInfo dataOut;

   // First get the invID
   QString invInfo = le_investigator->text();
   if ( invInfo.isEmpty() )
   {
      QMessageBox::information( this,
                tr( "Error" ),
                tr( "You must choose an investigator before accepting" ) );
      return;
   }

   QStringList components = invInfo.split( ")", QString::SkipEmptyParts );
   components = components[0].split( "(", QString::SkipEmptyParts );
   dataOut.invID = components.last().toInt();
 
   // Get the investigator name too
   components  = invInfo.split( ": ", QString::SkipEmptyParts );
   components  = components.last().split( ", ", QString::SkipEmptyParts );
   dataOut.lastName  = components.first().toAscii();
   dataOut.firstName = components.last().toAscii();

   // Other experiment information
   int ndx              = lw_experiment     ->currentRow();
   dataOut.expID        = experimentList[ ndx ].ID.toInt();

   ndx                  = cb_project        ->currentIndex();
   dataOut.projectID    = projectList[ ndx ].ID.toInt();

   ndx                  = cb_lab            ->currentIndex();
   dataOut.labID        = labList[ ndx ]    .ID.toInt();

   ndx                  = cb_instrument     ->currentIndex();
   dataOut.instrumentID = instrumentList[ ndx ].ID.toInt();

   ndx                  = cb_operator       ->currentIndex();
   dataOut.operatorID   = operatorList[ ndx ].ID.toInt();

   ndx                  = cb_rotor          ->currentIndex(); 
   dataOut.rotorID      = rotorList[ ndx ].ID.toInt();

   dataOut.expType      = cb_expType ->currentText();
   dataOut.runTemp      = le_runTemp ->text(); 
   dataOut.label        = le_label   ->text(); 
   dataOut.comments     = te_comment ->toPlainText();
   dataOut.centrifugeProtocol = le_centrifugeProtocol->text();

   // Save it to the db and return
   updateExperiment( dataOut );
   emit updateExpInfoSelection( dataOut );
   close();
}

void US_ExpInfo::cancel( void )
{
   expInfo.clear();
   reset();

   emit cancelExpInfoSelection();
   close();
}

void US_ExpInfo::selectInvestigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true );
   connect( inv_dialog, 
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( assignInvestigator   ( int, const QString&, const QString& ) ) );
   inv_dialog->exec();
}

void US_ExpInfo::assignInvestigator( int invID,
      const QString& lname, const QString& fname)
{
   expInfo.invID     = invID;
   expInfo.lastName  = lname;
   expInfo.firstName = fname;

   reset();
}

bool US_ExpInfo::getExperimentDesc( void )
{
   // Connect to the db
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return false;
   }

   struct listInfo info;

   QStringList q( "get_experiment_desc" );
   q << QString::number( 0 );
   db.query( q );

   experimentList.clear();
   lw_experiment->clear();
   while ( db.next() )
   {
      info.ID             = db.value( 0 ).toString();
      info.text           = db.value( 1 ).toString();
      experimentList      << info;

      // Add the item to the list widget too
      lw_experiment->addItem( new QListWidgetItem(
            info.text, lw_experiment ) );
   }

   return true;
}

void US_ExpInfo::selectExperiment( QListWidgetItem* )
{
   int ndx = lw_experiment->currentRow();

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   // Get the experiment info from the database
   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QStringList q( "get_experiment_info" );
   q << experimentList[ ndx ].ID;
   db.query( q );
   db.next();

   expInfo.expID              = experimentList[ ndx ].ID.toInt();
   expInfo.projectID          = db.value( 0 ).toInt();
   expInfo.labID              = db.value( 1 ).toInt();
   expInfo.instrumentID       = db.value( 2 ).toInt();
   expInfo.operatorID         = db.value( 3 ).toInt();
   expInfo.rotorID            = db.value( 4 ).toInt();
   expInfo.expType            = db.value( 5 ).toString();
   expInfo.runTemp            = db.value( 6 ).toString();
   expInfo.label              = db.value( 7 ).toString();
   expInfo.comments           = db.value( 8 ).toString();
   expInfo.centrifugeProtocol = db.value( 9 ).toString();
   expInfo.date               = db.value( 10 ).toString();

   reset();
}

void US_ExpInfo::updateExperiment( ExperimentInfo& d )
{
   // Update database
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QStringList q( "update_experiment" );
   q  << QString::number( d.expID )
      << QString::number( d.projectID )
      << QString::number( d.labID )
      << QString::number( d.instrumentID )
      << QString::number( d.operatorID )
      << QString::number( d.rotorID )
      << d.expType
      << d.runTemp
      << d.label
      << d.comments
      << d.centrifugeProtocol;

   db.statusQuery( q );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
            tr( "Database Error" ),
            tr( "The following errro was returned:\n" ) + db.lastError() );
      return;
   }

   // Get updated date info after db update
   q.clear();
   q << "get_experiment_info"
     << QString::number( d.expID );
   db.query( q );
   db.next();

   d.date               = db.value( 10 ).toString();
}

void US_ExpInfo::newExperiment( void )
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   // Connect to the database
   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   // Let's just copy most things from the current experiment info
   expInfo.runTemp  = "";
   expInfo.label    = "New Experiment";
   expInfo.comments = "";
   QStringList q( "new_experiment" );
   q  << QString::number( expInfo.projectID )
      << QString::number( expInfo.labID )
      << QString::number( expInfo.instrumentID )
      << QString::number( expInfo.operatorID )
      << QString::number( expInfo.rotorID )
      << expInfo.expType
      << expInfo.runTemp
      << expInfo.label
      << expInfo.comments
      << expInfo.centrifugeProtocol;

   db.statusQuery( q );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
            tr( "Database Error" ),
            tr( "The following error was returned:\n" ) + db.lastError() );
      return;
   }

   expInfo.expID = db.lastInsertID();

   reset();
}

void US_ExpInfo::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to databasee \n" ) + error );
}

QComboBox* US_ExpInfo::us_projectComboBox( void )
{
   // Connect to the db
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      US_ExpInfo::connect_error( db.lastError() );
      return 0;
   }

   QComboBox* cb = us_comboBox();

   struct listInfo info;

   QStringList q( "get_project_desc" );
   q << QString::number( 0 );
   db.query( q );

   projectList.clear();
   while ( db.next() )
   {
      info.ID       = db.value( 0 ).toString();
      info.text     = db.value( 1 ).toString();
      projectList << info;

      cb->addItem( info.text );
   }

   return cb;
}

QComboBox* US_ExpInfo::us_labComboBox( void )
{
   // Connect to the db
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      US_ExpInfo::connect_error( db.lastError() );
      return 0;
   }

   QComboBox* cb = us_comboBox();

   struct listInfo info;

   QStringList q( "get_lab_names" );
   db.query( q );

   labList.clear();
   while ( db.next() )
   {
      info.ID      = db.value( 0 ).toString();
      info.text    = db.value( 1 ).toString();
      labList     << info;

      cb->addItem( info.text );
   }

   return cb;
}

QComboBox* US_ExpInfo::us_instrumentComboBox( void )
{
   // Connect to the db
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      US_ExpInfo::connect_error( db.lastError() );
      return 0;
   }

   QComboBox* cb = us_comboBox();

   struct listInfo info;

   QStringList q( "get_instrument_names" );
   db.query( q );

   instrumentList.clear();
   while ( db.next() )
   {
      info.ID         = db.value( 0 ).toString();
      info.text       = db.value( 1 ).toString();
      instrumentList << info;

      cb->addItem( info.text );
   }

   return cb;
}

QComboBox* US_ExpInfo::us_operatorComboBox( void )
{
   // Connect to the db
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      US_ExpInfo::connect_error( db.lastError() );
      return 0;
   }

   QComboBox* cb = us_comboBox();

   struct listInfo info;

   QStringList q( "get_people" );
   q << "";           // "Like" parameter
   db.query( q );

   operatorList.clear();
   while ( db.next() )
   {
      info.ID       = db.value( 0 ).toString();
      info.text     = db.value( 1 ).toString();
      operatorList << info;

      cb->addItem( info.text );
   }

   return cb;
}

QComboBox* US_ExpInfo::us_rotorComboBox( void )
{
   // Connect to the db
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      US_ExpInfo::connect_error( db.lastError() );
      return 0;
   }

   QComboBox* cb = us_comboBox();

   struct listInfo info;

   QStringList q( "get_rotor_names" );
   db.query( q );

   rotorList.clear();
   while ( db.next() )
   {
      info.ID      = db.value( 0 ).toString();
      info.text    = db.value( 1 ).toString();
      rotorList   << info;

      cb->addItem( info.text );
   }

   return cb;
}

QComboBox* US_ExpInfo::us_expTypeComboBox( void )
{
   QComboBox* cb = us_comboBox();

   // Experiment types
   experimentTypes.clear();
   experimentTypes << "Velocity"
                   << "Equilibrium"
                   << "other";

   cb->addItems( experimentTypes );

   return cb;
}

// Function to update a combobox so that the current choice is selected
void US_ExpInfo::setComboBoxIndex( QComboBox* cb, QList< listInfo >& list, int ID )
{
   for ( int i = 0; i < list.size(); i++ )
   {
      if ( list[ i ].ID.toInt() == ID )
      {
         cb->setCurrentIndex( i );
         return;
      }
   }

   // If here, index was not found
   cb->setCurrentIndex( 0 );
}

void US_ExpInfo::setWidgetIndex( QListWidget* lw, QList< listInfo >& list, int ID )
{
   for ( int i = 0; i < list.size(); i++ )
   {
      if ( list[ i ].ID.toInt() == ID )
      {
         lw->setCurrentRow( i );
         return;
      }
   }

   // If here, index was not found
   lw->setCurrentRow( 0 );
}

// Initializations
US_ExpInfo::TripleInfo::TripleInfo()
{
   centerpiece  = 0;
   bufferID     = 0;
   analyteID    = 0;
}

US_ExpInfo::ExperimentInfo::ExperimentInfo()
{
   ExperimentInfo::clear();
}

void US_ExpInfo::ExperimentInfo::clear( void )
{
   invID              = 0;
   lastName           = QString( "" );
   firstName          = QString( "" );
   expID              = 0;
   projectID          = 0;
   labID              = 0;
   instrumentID       = 0;
   operatorID         = 0;
   rotorID            = 0;
   expType            = QString( "" );
   runTemp            = QString( "" );
   label              = QString( "" );
   comments           = QString( "" );
   centrifugeProtocol = QString( "" );
   date               = QString( "" );
   triples.clear();               // Not to be confused with the global triples
}

US_ExpInfo::ExperimentInfo& US_ExpInfo::ExperimentInfo::operator=( const ExperimentInfo& rhs )
{
   if ( this != &rhs )            // Guard against self assignment
   {
      invID        = rhs.invID;
      lastName     = rhs.lastName;
      firstName    = rhs.firstName;
      expID        = rhs.expID;
      projectID    = rhs.projectID;
      labID        = rhs.labID;
      instrumentID = rhs.instrumentID;
      operatorID   = rhs.operatorID;
      rotorID      = rhs.rotorID;
      expType      = rhs.expType;
      runTemp      = rhs.runTemp;
      label        = rhs.label;
      comments     = rhs.comments;
      centrifugeProtocol = rhs.centrifugeProtocol;
      date         = rhs.date;

      for ( int i = 0; i < triples.size(); i++ )
      {
         triples[ i ].tripleID    = rhs.triples[ i ].tripleID;
         triples[ i ].centerpiece = rhs.triples[ i ].centerpiece;
         triples[ i ].bufferID    = rhs.triples[ i ].bufferID;
         triples[ i ].analyteID   = rhs.triples[ i ].analyteID;
      }
   }

   return *this;
}

