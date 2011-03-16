//! \file us_experiment_gui.cpp

#include <QtGui>

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_investigator.h"
#include "us_experiment_gui.h"
#include "us_convertio.h"
#include "us_project_gui.h"
#include "us_rotor_gui.h"

US_ExperimentGui::US_ExperimentGui( 
      US_Experiment& dataIn ) :
   US_WidgetsDialog( 0, 0 ), expInfo( dataIn )
{
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
   le_runID->setPalette ( gray );
   le_runID->setReadOnly( true );

   experiment->addWidget( le_runID, row++, 0, 1, 2 );
 
   // Experiment label
   QLabel* lb_label = us_label( tr( "Label:" ) );
   experiment->addWidget( lb_label, row++, 0, 1, 2 );
   le_label = us_lineedit();
   experiment->addWidget( le_label, row++, 0, 1, 2 );

   // Project
   QLabel* lb_project = us_label( tr( "Project:" ) );
   experiment->addWidget( lb_project, row, 0 );
   QPushButton* pb_project = us_pushbutton( tr( "Select Project" ) );
   connect( pb_project, SIGNAL( clicked() ), SLOT( selectProject() ) );
   pb_project->setEnabled( true );
   experiment->addWidget( pb_project, row++, 1 );

   le_project = us_lineedit();
   le_project->setPalette ( gray );
   le_project->setReadOnly( true );
   experiment->addWidget( le_project, row++, 0, 1, 2 );

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
   if ( ( expInfo.opticalSystem == "RA" ) ||
        ( expInfo.opticalSystem == "WA" ) )
      le_opticalSystem->setText( "Absorbance" );

   else if ( ( expInfo.opticalSystem == "RI" ) ||
             ( expInfo.opticalSystem == "WI" ) )
      le_opticalSystem->setText( "Intensity" );

   else if ( expInfo.opticalSystem == "IP" )
      le_opticalSystem->setText( "Interference" );

   else if ( expInfo.opticalSystem == "FI" )
      le_opticalSystem->setText( "Fluorescence" );

   else // Unsupported optical system
      le_opticalSystem->setText( "Unsupported" );

   // Now for predominantly hardware info
   QGridLayout* hardware = new QGridLayout;
   row = 0;

   // Selected hardware information
   QLabel* lb_hardware_banner = us_banner( tr( "Hardware: " ) );
   hardware->addWidget( lb_hardware_banner, row++, 0, 1, 2 );

   QPushButton* pb_rotor = us_pushbutton( tr( "Select Lab / Rotor / Calibration" ) );
   connect( pb_rotor, SIGNAL( clicked() ), SLOT( selectRotor() ) );
   pb_rotor->setEnabled( true );
   hardware->addWidget( pb_rotor, row++, 0, 1, 2 );

   le_rotorDesc = us_lineedit(); 
   le_rotorDesc->setPalette ( gray );
   le_rotorDesc->setReadOnly( true );
   hardware->addWidget( le_rotorDesc, row++, 0, 1, 2 );

   // Rotor speeds
   QLabel* lb_rotorSpeeds = us_label( tr( "Unique Rotor Speeds:" ) );
   hardware->addWidget( lb_rotorSpeeds, row++, 0, 1, 2 );
   lw_rotorSpeeds = us_listwidget();
   lw_rotorSpeeds ->setMaximumHeight( 50 );
   lw_rotorSpeeds ->setPalette( gray );
   hardware->addWidget( lw_rotorSpeeds, row, 0, 2, 2 );
   row += 2;

   // The rotor speed information won't change
   foreach ( double rpm, expInfo.rpms )
      lw_rotorSpeeds -> addItem( QString::number( rpm ) );

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

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
   buttons->addWidget( pb_cancel );

   pb_accept = us_pushbutton( tr( "Accept" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept() ) );
   buttons->addWidget( pb_accept );

   // Now let's assemble the page
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   row = 0;

   // Database choices
   QStringList DB = US_Settings::defaultDB();
   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   main->addWidget( lb_DB, row++, 0, 1, 2 );

   // Investigator
   if ( US_Settings::us_inv_level() > 2 )
   {
      QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
      connect( pb_investigator, SIGNAL( clicked() ), SLOT( selectInvestigator() ) );
      main->addWidget( pb_investigator, row, 0 );
   }
   else
   {
      QLabel* lb_investigator = us_label( tr( "Investigator:" ) );
      main->addWidget( lb_investigator, row, 0 );
   }
      
   le_investigator = us_lineedit( tr( "Not Selected" ) );
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
   te_comment->setMaximumHeight( 80 );
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

void US_ExperimentGui::reset( void )
{
   reload();

   le_investigator ->clear();
   le_label        ->clear();
   le_runID        ->setText( expInfo.runID );
   le_project      ->clear();
   te_comment      ->clear();

   pb_accept       ->setEnabled( false );

   // Update controls to represent selected experiment
   cb_instrument   ->setLogicalIndex( expInfo.instrumentID );
   cb_operator     ->setLogicalIndex( expInfo.operatorID   );

   le_label        ->setText( expInfo.label                );
   le_project      ->setText( expInfo.project.projectDesc          );
   te_comment      ->setText( expInfo.comments             );
         
   // Experiment types combo
   cb_expType->setCurrentIndex( 3 );  // default is "other"
   for ( int i = 0; i < experimentTypes.size(); i++ )
   {
      if ( experimentTypes[ i ].toUpper() == expInfo.expType.toUpper() )
      {
         cb_expType->setCurrentIndex( i );
         break;
      }
   }
   
   // Display investigator
   expInfo.invID = US_Settings::us_inv_ID();

   if ( expInfo.invID > 0 )
   {
      le_investigator->setText( QString::number( expInfo.invID ) + ": " 
         + US_Settings::us_inv_name() );

      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
      
      int runIDStatus = US_DB2::NO_EXPERIMENT;
      if ( db.lastErrno() == US_DB2::OK )
         runIDStatus = expInfo.checkRunID( &db );

      if ( expInfo.expID > 0 )
         pb_accept       ->setEnabled( true );
 
      else if ( runIDStatus != US_DB2::OK )
      {
         // Then an investigator has been chosen, and 
         //  the current runID doesn't exist in the db
         pb_accept       ->setEnabled( true );
      }

      // However, project needs to be selected, either from db or disk
      if ( expInfo.project.projectID == 0 && expInfo.project.projectGUID.isEmpty() ) 
         pb_accept       ->setEnabled( false );
   }

   else
      le_investigator->setText( "Not Selected" );

}

// function to load what we can initially
// returns true if successful
bool US_ExperimentGui::load( void )
{
   if ( expInfo.invID == 0 )
   {
      // Try to get info from settings
      int inv = US_Settings::us_inv_ID();
      if ( inv > -1 )
      {
         expInfo.invID     = inv;
         getInvestigatorInfo();
      }
   }

   // Load values that were passed in
   if ( ! expInfo.rotorName.isEmpty() )
      le_rotorDesc->setText( expInfo.rotorName + " / " +
                    expInfo.rotorUpdated.toString( "yyyy-MM-dd" ) );

   if ( ! expInfo.label.isEmpty() )
      le_label->setText( expInfo.label );

   if ( expInfo.project.projectID > 0 )
      le_project->setText( expInfo.project.projectDesc );

   if ( ! expInfo.comments.isEmpty() )
      te_comment->setText( expInfo.comments );

   lab_changed = true; // so boxes will go through all the reload code 1st time

   return( true );
}

void US_ExperimentGui::reload( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   if ( lab_changed )
   {
      setInstrumentList();
      setOperatorList();

      cb_instrument   ->load();
      cb_operator     ->load();

      lab_changed = false;
   }
}

void US_ExperimentGui::syncHardware( void )
{
}

void US_ExperimentGui::selectInvestigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true, expInfo.invID );

   connect( inv_dialog, 
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( assignInvestigator   ( int, const QString&, const QString& ) ) );

   inv_dialog->exec();
}

void US_ExperimentGui::assignInvestigator( int invID,
      const QString& lname, const QString& fname )
{
   expInfo.invID = invID;
   le_investigator->setText( QString::number( invID ) + ": " +
         lname + ", " + fname );
}

void US_ExperimentGui::getInvestigatorInfo( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   expInfo.invID = US_Settings::us_inv_ID();     // just to be sure
   expInfo.name  = US_Settings::us_inv_name();
   QStringList q( "get_person_info" );
   q << QString::number( expInfo.invID );
   db.query( q );

   if ( db.next() )
   {
      expInfo.invGUID   = db.value( 9 ).toString();
   }
   
}

void US_ExperimentGui::selectProject( void )
{
   US_Project project  = expInfo.project;

   US_ProjectGui* projInfo = new US_ProjectGui( true, US_Disk_DB_Controls::DB, project );
   connect( projInfo, 
      SIGNAL( updateProjectGuiSelection( US_Project& ) ),
      SLOT  ( assignProject            ( US_Project& ) ) );
   connect( projInfo, 
      SIGNAL( cancelProjectGuiSelection( ) ),
      SLOT  ( cancelProject            ( ) ) );
   projInfo->exec();
}

void US_ExperimentGui::assignProject( US_Project& project )
{
   expInfo.project  = project;

   reset();
}

void US_ExperimentGui::cancelProject( void )
{
   reset();
}

QComboBox* US_ExperimentGui::us_expTypeComboBox( void )
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

void US_ExperimentGui::setInstrumentList( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
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

      // is the instrument ID in the list?
      int index = 0;
      for ( int i = 0; i < options.size(); i++ )
      {
         if ( expInfo.instrumentID == options[ i ].ID.toInt() )
         {
            index = i;
            break;
         }
      }

      // Replace instrument ID with one from the list
      expInfo.instrumentID = options[ index ].ID.toInt();
         
   }

}

void US_ExperimentGui::setOperatorList( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

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

      // is the operator ID in the list?
      int index = 0;
      for ( int i = 0; i < options.size(); i++ )
      {
         if ( expInfo.operatorID == options[ i ].ID.toInt() )
         {
            index = i;
            break;
         }
      }

      // Replace operator ID with one from the list
      expInfo.operatorID = options[ index ].ID.toInt();
   }
}

// Function to change the current instrument
void US_ExperimentGui::change_instrument( int )
{
   // First time through here the combo box might not be displayed yet
   expInfo.instrumentID = ( cb_instrument->getLogicalID() == -1 )
                          ? expInfo.instrumentID
                          : cb_instrument->getLogicalID();

   // Save other elements on the page too
   expInfo.label         = le_label   ->text(); 
   expInfo.comments      = te_comment ->toPlainText();
   expInfo.expType       = cb_expType ->currentText();

   lab_changed = true;
   reset();
}

void US_ExperimentGui::selectRotor( void )
{
   US_Rotor::Rotor rotor;
   rotor.ID = expInfo.rotorID;

   US_Rotor::RotorCalibration calibration;
   calibration.ID = expInfo.calibrationID;

   US_RotorGui* rotorInfo = new US_RotorGui( true, US_Disk_DB_Controls::Default,
                                             rotor, calibration );

   connect( rotorInfo, SIGNAL( RotorCalibrationSelected( US_Rotor::Rotor&, US_Rotor::RotorCalibration& ) ),
                       SLOT  ( assignRotor             ( US_Rotor::Rotor&, US_Rotor::RotorCalibration& ) ) );
   connect( rotorInfo, SIGNAL( RotorCalibrationCanceled( ) ),
                       SLOT  ( cancelRotor             ( ) ) );
   rotorInfo->exec();
}

void US_ExperimentGui::assignRotor( US_Rotor::Rotor& rotor, US_Rotor::RotorCalibration& calibration )
{
   expInfo.rotorID       = rotor.ID;
   expInfo.rotorGUID     = rotor.GUID;
   expInfo.rotorSerial   = rotor.serialNumber;
   expInfo.rotorName     = rotor.name;
   expInfo.labID         = rotor.labID;
   expInfo.calibrationID = calibration.ID;
   expInfo.rotorCoeff1   = calibration.coeff1;
   expInfo.rotorCoeff2   = calibration.coeff2;
   expInfo.rotorUpdated  = calibration.lastUpdated;

   le_rotorDesc->setText( rotor.name + " / " +
                 calibration.lastUpdated.toString( "yyyy-MM-dd" ) );

   lab_changed = true;
   setInstrumentList();
   setOperatorList();

   reset();
}

void US_ExperimentGui::cancelRotor( void )
{
   reset();
}

void US_ExperimentGui::accept( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   // We can sync with the DB
   expInfo.syncOK = true;

   // Overwrite data directly from the form

   // First get the invID
   expInfo.invID   = US_Settings::us_inv_ID();
   expInfo.name    = US_Settings::us_inv_name();
   getInvestigatorInfo();

   // Other info on the form
   expInfo.runID         = le_runID         ->text();
   expInfo.instrumentID  = cb_instrument    ->getLogicalID();
   expInfo.operatorID    = cb_operator      ->getLogicalID();
   expInfo.expType       = cb_expType       ->currentText();
   expInfo.runTemp       = le_runTemp       ->text(); 
   expInfo.label         = le_label         ->text(); 
   expInfo.comments      = te_comment       ->toPlainText();

   // Update items from the DB after getting values from the form
   QStringList q( "get_instrument_info" );
   q  << QString::number( expInfo.instrumentID );
   db.query( q );
   db.next();
   expInfo.instrumentSerial = db.value( 1 ).toString();

   q.clear();
   q  << "get_person_info"
      << QString::number( expInfo.operatorID );
   db.query( q );
   db.next();
   expInfo.operatorGUID = db.value( 9 ).toString();

   emit updateExpInfoSelection( expInfo );
   close();
}

void US_ExperimentGui::cancel( void )
{
   expInfo.clear();

   emit cancelExpInfoSelection();
   close();
}

void US_ExperimentGui::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to databasee \n" ) + error );
}
