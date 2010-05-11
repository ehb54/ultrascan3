//! \file us_expinfo.cpp

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_investigator.h"
#include "us_expinfo.h"

US_ExpInfo::US_ExpInfo( ExperimentInfo& dataIn, bool editing ) :
   US_WidgetsDialog( 0, 0 ), expInfo( dataIn )
{
   this->editing = editing;

   setPalette( US_GuiSettings::frameColor() );
   setWindowTitle( tr( "Experiment Information" ) );
   setAttribute( Qt::WA_DeleteOnClose );

   // Very light gray, for read-only line edits
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   // Set up left panel with experiment information
   QGridLayout* experiment = new QGridLayout;
   int row = 0;

   // Current experiment information
   QLabel* lb_experiment_banner = us_banner( tr( "Experiment: " ) );
   experiment->addWidget( lb_experiment_banner, row++, 0, 1, 2 );

   // Show current runID
   QLabel* lb_runID = us_label( tr( "Run ID " ) );
   experiment->addWidget( lb_runID, row++, 0, 1, 2 );
   le_runID = us_lineedit();
   if ( this->editing )
   {
      le_runID->setPalette ( gray );
      le_runID->setReadOnly( true );
   }

   else
      connect( le_runID, SIGNAL( editingFinished() ), 
                         SLOT  ( runIDChanged   () ) );

   experiment->addWidget( le_runID, row++, 0, 1, 2 );
 
   // Experiment label
   QLabel* lb_label = us_label( tr( "Label:" ) );
   experiment->addWidget( lb_label, row++, 0, 1, 2 );
   le_label = us_lineedit();
   experiment->addWidget( le_label, row++, 0, 1, 2 );

   // Project
   QLabel* lb_project = us_label( tr( "Project:" ) );
   experiment->addWidget( lb_project, row, 0 );
   cb_project = new US_SelectBox( this );
   experiment->addWidget( cb_project, row++, 1 );

   // Experiment type
   QLabel* lb_expType = us_label( tr( "Experiment Type:" ) );
   experiment->addWidget( lb_expType, row, 0 );
   cb_expType = us_expTypeComboBox();
   experiment->addWidget( cb_expType, row++, 1 );

   // Optical system
   QLabel* lb_opticalSystem = us_label( tr( "Optical System:" ) );
   experiment->addWidget( lb_opticalSystem, row, 0 );
   QLineEdit* le_opticalSystem = us_lineedit();
   le_opticalSystem->setPalette ( gray );
   le_opticalSystem->setReadOnly( true );
   experiment->addWidget( le_opticalSystem, row++, 1 );

   // The optical system won't change
   if ( ( strncmp( expInfo.opticalSystem, "RA", 2 ) == 0 ) ||
        ( strncmp( expInfo.opticalSystem, "WA", 2 ) == 0 ) )
      le_opticalSystem->setText( "Absorbance" );

   else if ( ( strncmp( expInfo.opticalSystem, "RI", 2 ) == 0 ) ||
             ( strncmp( expInfo.opticalSystem, "WI", 2 ) == 0 ) )
      le_opticalSystem->setText( "Intensity" );

   else if ( strncmp( expInfo.opticalSystem, "IP", 2 ) == 0 )
      le_opticalSystem->setText( "Interference" );

   else if ( strncmp( expInfo.opticalSystem, "FI", 2 ) == 0 )
      le_opticalSystem->setText( "Fluorescence" );

   else // Unsupported optical system
      le_opticalSystem->setText( "Unsupported" );

   // Now for predominantly hardware info
   QGridLayout* hardware = new QGridLayout;
   row = 0;

   // Selected hardware information
   QLabel* lb_hardware_banner = us_banner( tr( "Hardware: " ) );
   hardware->addWidget( lb_hardware_banner, row++, 0, 1, 2 );

   // labID
   QLabel* lb_lab = us_label( tr( "Lab:" ) );
   hardware->addWidget( lb_lab, row, 0 );
   cb_lab = new US_SelectBox( this );
   connect( cb_lab, SIGNAL( activated ( int ) ),      // Only if the user has changed it
                    SLOT  ( change_lab( int ) ) );
   hardware->addWidget( cb_lab, row++, 1 );

   // instrumentID
   QLabel* lb_instrument = us_label( tr( "Instrument:" ) );
   hardware->addWidget( lb_instrument, row, 0 );
   cb_instrument = new US_SelectBox( this );
   connect( cb_instrument, SIGNAL( activated        ( int ) ),
                           SLOT  ( change_instrument( int ) ) );
   hardware->addWidget( cb_instrument, row++, 1 );

   // operatorID
   QLabel* lb_operator = us_label( tr( "Operator:" ) );
   hardware->addWidget( lb_operator, row, 0 );
   cb_operator = new US_SelectBox( this );
   hardware->addWidget( cb_operator, row++, 1 );

   // Rotor used in experiment
   QLabel* lb_rotor = us_label( tr( "Rotor:" ) );
   hardware->addWidget( lb_rotor, row, 0 );
   cb_rotor = new US_SelectBox( this );
   hardware->addWidget( cb_rotor, row++, 1 );
   cb_rotor->setEditable( false );

   // Rotor speeds
   QLabel* lb_rotorSpeeds = us_label( tr( "Unique Rotor Speeds:" ) );
   hardware->addWidget( lb_rotorSpeeds, row++, 0, 1, 2 );
   lw_rotorSpeeds = us_listwidget();
   lw_rotorSpeeds ->setMaximumHeight( 100 );
   lw_rotorSpeeds ->setPalette( gray );
   hardware->addWidget( lw_rotorSpeeds, row, 0, 2, 2 );
   row += 3;

   // The rotor speed information won't change
   foreach ( double rpm, expInfo.rpms )
      lw_rotorSpeeds -> addItem( QString::number( rpm ) );

   // Run Temperature
   QLabel* lb_runTemp = us_label( tr( "Average Run Temperature:" ) );
   hardware->addWidget( lb_runTemp, row, 0 );
   le_runTemp = us_lineedit();
   hardware->addWidget( le_runTemp, row++, 1 );
   le_runTemp->setPalette ( gray );
   le_runTemp->setReadOnly( true );

   // Run temperature won't change
   le_runTemp           ->setText( expInfo.runTemp );

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

   // Now let's assemble the page
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   row = 0;

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

   main->addLayout( experiment, row, 0 );
   main->addLayout( hardware,   row, 1 );
   row++; // += 10;

   // Experiment comments
   QLabel* lb_comment = us_label( tr( "Comments:" ) );
   main->addWidget( lb_comment, row++, 0, 1, 2 );

   te_comment = us_textedit();
   main->addWidget( te_comment, row, 0, 4, 2 );
   te_comment->setMaximumHeight( 120 );
   te_comment->setReadOnly( false );
   row += 4;

   main->addLayout( buttons, row++, 0, 1, 2 );

   // Let's load everything we can
   if ( ! load() )
   {
      cancel();
      return;
   }

   reset();
}

void US_ExpInfo::reset( void )
{
   reload();

   le_investigator ->clear();
   le_label        ->clear();
   le_runID        ->setText( expInfo.runID );
   te_comment      ->clear();

   cb_project      ->reset();
   cb_lab          ->reset();
   cb_instrument   ->reset();
   cb_operator     ->reset();
   cb_rotor        ->reset();
   cb_lab          ->setLogicalIndex( expInfo.labID        );
   cb_instrument   ->setLogicalIndex( expInfo.instrumentID );
   cb_operator     ->setLogicalIndex( expInfo.operatorID   );
   cb_rotor        ->setLogicalIndex( expInfo.rotorID      );

   pb_accept       ->setEnabled( false );

   if ( expInfo.invID > 0 )
   {
      // Investigator
      le_investigator->setText( "InvID (" + QString::number( expInfo.invID ) + "): " +
               expInfo.lastName + ", " + expInfo.firstName );

      if ( this->editing )
      {
         if ( expInfo.expID > 0 )
         {
            // Update controls to represent selected experiment
            le_label   ->setText( expInfo.label             );
            te_comment ->setText( expInfo.comments          );
            cb_project ->setLogicalIndex( expInfo.projectID );
         
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
      else if ( checkRunID() == 0 )
      {
         // Then we're not editing, an investigator has been chosen, and 
         //  the current runID doesn't exist in the db
         pb_accept       ->setEnabled( true );
      }
   }
}

// function to load what we can initially
// returns true if successful
bool US_ExpInfo::load( void )
{
   // Find out what labs we have
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return( false );
   }

   // Did the user click edit?
   if ( this->editing )
   {
      QStringList q( "get_experiment_info_by_runID" );
      q << expInfo.runID;
      db.query( q );
 
      if ( db.next() )
      {
         expInfo.projectID          = db.value( 0 ).toInt();
         expInfo.expID              = db.value( 1 ).toInt();
         expInfo.labID              = db.value( 2 ).toInt();
         expInfo.instrumentID       = db.value( 3 ).toInt();
         expInfo.operatorID         = db.value( 4 ).toInt();
         expInfo.rotorID            = db.value( 5 ).toInt();
         expInfo.expType            = db.value( 6 ).toString();
         // Let's go with the calculated runTemp);
         expInfo.label              = db.value( 8 ).toString();
         expInfo.comments           = db.value( 9 ).toString();
         expInfo.centrifugeProtocol = db.value( 10 ).toString();
         expInfo.date               = db.value( 11 ).toString();
      }

      else if ( db.lastErrno() == US_DB2::NOROWS )
      {
         QMessageBox::information( this,
                tr( "Error" ),
                tr( "The current run ID is not found in the database;\n" ) +
                tr( "Click the New Experiment button to add it" ) );
         return( false );
      }

      else
      {
         QMessageBox::information( this,
                tr( "Error" ),
                db.lastError() );
         return( false );
      }
   }

   QStringList q( "get_lab_names" );
   db.query( q );

   QList<listInfo> options;
   while ( db.next() )
   {
      struct listInfo option;
      option.ID      = db.value( 0 ).toString();
      option.text    = db.value( 1 ).toString();
      options << option;
   }

   cb_lab->clear();
   if ( options.size() > 0 )
   {
      cb_lab->addOptions( options );
      if ( this->editing )
         cb_lab->setLogicalIndex( expInfo.labID );

      else
      {
         cb_lab->setCurrentIndex( 0 );             // the first one
         expInfo.labID = options[ 0 ].ID.toInt();  // guard against a value of -1 here
      }

      change_lab( 0 );
      change_instrument( 0 );
   }

   return( true );
}

void US_ExpInfo::reload( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   // Populate the project combo box if we can
   if ( expInfo.invID > 0 )
   {
      QStringList q( "get_project_desc" );
      q << QString::number( 0 );            // the "get all my projects" parameter
      db.query( q );
   
      QList<listInfo> options;
      while ( db.next() )
      {
         struct listInfo option;
         option.ID      = db.value( 0 ).toString();
         option.text    = db.value( 1 ).toString();
         options << option;
      }
   
      cb_project->clear();
      if ( options.size() > 0 )
      {
          cb_project->addOptions( options );

          if ( this->editing )
             cb_project ->setLogicalIndex( expInfo.projectID );

          else
          {
             cb_project->setCurrentIndex( 0 );
             expInfo.projectID = options[ 0 ].ID.toInt();
          }
      }
   }

}

void US_ExpInfo::runIDChanged( void )
{
   expInfo.runID = le_runID->text();

   // If we are editing, and we've identified an investigator and an experiment
   if ( expInfo.invID > 0 && this->editing && expInfo.expID > 0 )
      pb_accept->setEnabled( true );

   // If we are not editing, but we've identified an investigator and the runID is new
   else if ( expInfo.invID > 0 && checkRunID() == 0 )
      pb_accept->setEnabled( true );

   else 
      pb_accept->setEnabled( false );
 
}

// Function to see if the current runID already exists in the database
// Returns the expID, or 0 if no rows. Returns -1 if no connection to db,
//  or unknown problem
int US_ExpInfo::checkRunID( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );
   
   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return( -1 );
   }

   // Let's see if we can find the run ID
   QStringList q( "get_experiment_info_by_runID" );
   q << expInfo.runID;
   db.query( q );

   if ( db.lastErrno() == US_DB2::OK )
   {
      // Then the runID is in the db
      db.next();
      return( db.value( 1 ).toInt() );            // Return the expID
   }

   else if ( db.lastErrno() != US_DB2::NOROWS )
   {
      // Some other error
      return( -1 );
   }

   // We know now that this is a new run ID
   return( 0 );
}

// Function to change the current lab
void US_ExpInfo::change_lab( int )
{
   // First time through here the combo box might not be displayed yet
   expInfo.labID = ( cb_lab->getLogicalID() == -1 )
                   ? expInfo.labID
                   : cb_lab->getLogicalID();
 
   // Get a list of the instruments in this lab
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      qDebug() << db.lastError();
      return;
   }

   QStringList q( "get_instrument_names" );
   q << QString::number( expInfo.labID );     // In this lab
   db.query( q );

   QList<listInfo> options;
   while ( db.next() )
   {
      struct listInfo option;
      option.ID      = db.value( 0 ).toString();
      option.text    = db.value( 1 ).toString();
      options << option;
   }

   cb_instrument->clear();
   if ( options.size() > 0 )
   {
      cb_instrument->addOptions( options );

      if ( this->editing )
         cb_instrument->setLogicalIndex( expInfo.instrumentID );

      else
      {
         cb_instrument->setCurrentIndex( 0 );
         expInfo.instrumentID = options[ 0 ].ID.toInt();
      }
   }

   // Now we need a list of rotors in this lab
   q.clear();
   q << "get_rotor_names"
     << QString::number( expInfo.labID );     // In this lab
   db.query( q );

   options.clear();
   while ( db.next() )
   {
      struct listInfo option;
      option.ID      = db.value( 0 ).toString();
      option.text    = db.value( 1 ).toString();
      options << option;
   }

   cb_rotor->clear();
   if ( options.size() > 0 )
   {
      cb_rotor->addOptions( options );
      cb_rotor->setCurrentIndex( 0 );
      expInfo.rotorID = options[ 0 ].ID.toInt();
   }

   reset();
}

// Function to change the current instrument
void US_ExpInfo::change_instrument( int )
{
   // First time through here the combo box might not be displayed yet
   expInfo.instrumentID = ( cb_instrument->getLogicalID() == -1 )
                          ? expInfo.instrumentID
                          : cb_instrument->getLogicalID();

   // Get a list of operators that can run this instrument
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
      return;

   QStringList q( "get_operator_names" );
   q << QString::number( expInfo.instrumentID );  // who can use this instrument
   db.query( q );

   QList<listInfo> options;
   while ( db.next() )
   {
      struct listInfo option;
      option.ID      = db.value( 0 ).toString();
      option.text    = db.value( 1 ).toString();
      options << option;
   }

   cb_operator->clear();
   if ( options.size() > 0 )
   {
      cb_operator->addOptions( options );
      if ( this->editing )
         cb_rotor->setLogicalIndex( expInfo.rotorID );

      else
      {
         cb_operator->setCurrentIndex( 0 );
         expInfo.operatorID = options[ 0 ].ID.toInt();
      }
   }

   reset();

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

   // Other experiment information that goes into the db
   dataOut.projectID     = cb_project       ->getLogicalID();
   dataOut.runID         = le_runID         ->text();
   dataOut.labID         = cb_lab           ->getLogicalID();
   dataOut.instrumentID  = cb_instrument    ->getLogicalID();
   dataOut.operatorID    = cb_operator      ->getLogicalID();
   dataOut.rotorID       = cb_rotor         ->getLogicalID();
   dataOut.expType       = cb_expType       ->currentText();
   dataOut.runTemp       = le_runTemp       ->text(); 
   dataOut.label         = le_label         ->text(); 
   dataOut.comments      = te_comment       ->toPlainText();

   // Save it to the db and return
   if ( this->editing )
   {
      dataOut.expID = expInfo.expID;
      updateExperiment( dataOut );
   }

   else
      newExperiment( dataOut );

   emit updateExpInfoSelection( dataOut );
   close();
}

void US_ExpInfo::cancel( void )
{
   expInfo.clear();

   emit cancelExpInfoSelection();
   close();
}

void US_ExpInfo::newExperiment( ExperimentInfo& d )
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   // Connect to the database
   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QStringList q( "new_experiment" );
   q  << QString::number( d.projectID )
      << d.runID
      << QString::number( d.labID )
      << QString::number( d.instrumentID )
      << QString::number( d.operatorID )
      << QString::number( d.rotorID )
      << d.expType
      << d.runTemp
      << d.label
      << d.comments
      << d.centrifugeProtocol;

   int status = db.statusQuery( q );
   if ( status != US_DB2::OK )
   {
      QMessageBox::information( this,
            tr( "Database Error" ),
            tr( "The following error was returned:\n" ) + db.lastError() );
      return;
   }

   // Let's get some info after db update
   q.clear();
   d.expID = db.lastInsertID();
   q << "get_experiment_info"
     << QString::number( d.expID );
   db.query( q );
   db.next();

   d.date = db.value( 11 ).toString();
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
      << d.runID
      << QString::number( d.labID )
      << QString::number( d.instrumentID )
      << QString::number( d.operatorID )
      << QString::number( d.rotorID )
      << d.expType
      << d.runTemp
      << d.label
      << d.comments
      << d.centrifugeProtocol;

   int status = db.statusQuery( q );
   if ( status != US_DB2::OK )
   {
      QMessageBox::information( this,
            tr( "Database Error" ),
            tr( "The following error was returned:\n" ) + db.lastError() + QString::number(status) );
      return;
   }

   // Let's get some info after db update
   q.clear();
   q << "get_experiment_info"
     << QString::number( d.expID );
   db.query( q );
   db.next();

   d.date = db.value( 11 ).toString();
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

   if ( ! this->editing && checkRunID() > 1 )
   {
      QMessageBox::information( this,
                tr( "Error" ),
                tr( "This run ID is already in the database; you must " ) +
                tr( "change that before accepting" ) );
   }

   reset();
}

QComboBox* US_ExpInfo::us_expTypeComboBox( void )
{
   QComboBox* cb = us_comboBox();

   // Experiment types
   experimentTypes.clear();
   experimentTypes << "Velocity"
                   << "Equilibrium"
                   << "Diffusion"
                   << "other";

   cb->addItems( experimentTypes );

   return cb;
}

void US_ExpInfo::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to databasee \n" ) + error );
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
   runID              = QString( "" );
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
      runID        = rhs.runID;
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
