//! \file us_rotor_gui.cpp

#include "us_rotor.h"
#include "us_rotor_gui.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_db2.h"
#include "us_gui_settings.h"
#include "us_passwd.h"

US_RotorGui::US_RotorGui(
     US_Rotor::RotorCalibration& calibration,
     bool  new_calibration,
     bool  signal_wanted,
     int   select_db_disk
     ) : US_WidgetsDialog(0, 0), currentCalibration( calibration ) 
{
   this->savingCalibration = new_calibration;
   this->signal            = signal_wanted;

   setupGui( select_db_disk );

   reset();

   if ( savingCalibration )
   {
      QMessageBox::information( this,
         tr( "Attention" ),
         tr( "Please select the appropriate rotor and click Save Calibration Data, or Close" ) );
   }
}

US_RotorGui::US_RotorGui(
     bool  signal_wanted,
     int   select_db_disk
     ) : US_WidgetsDialog(0, 0)
{
   this->savingCalibration = false;
   this->signal            = signal_wanted;

   setupGui( select_db_disk );

   reset();
}

US_RotorGui::~US_RotorGui()
{
}

void US_RotorGui::setupGui( int select_db_disk )
{
   setWindowTitle( tr( "Rotor Management" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* top = new QGridLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;
   QStringList DB = US_Settings::defaultDB();
   if ( DB.isEmpty() ) DB << "Undefined";

   QLabel* lbl_bannerDB = us_banner( tr( "Current Database: " ) + DB.at( 0 ) );
   lbl_bannerDB->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   top->addWidget( lbl_bannerDB, row++, 0, 1, 4 );

   disk_controls = new US_Disk_DB_Controls( select_db_disk );
   connect( disk_controls, SIGNAL( changed       ( bool ) ),
                           SLOT  ( source_changed( bool ) ) );
   top->addLayout( disk_controls, row++, 0, 1, 4 );

   // labID
   QLabel* lbl_lab = us_label( tr( " Please select a Laboratory:" ) );
   top->addWidget( lbl_lab, row, 0, 1, 2 );
   cb_lab = new QComboBox( this );
   connect( cb_lab, SIGNAL( activated( int ) ),      // Only if the user has changed it
                    SLOT  ( changeLab( int ) ) );
   top->addWidget( cb_lab, row++, 2, 1, 2 );

   QLabel* lbl_bannerRotor = us_banner( tr( "Click to select a Rotor: " ), -1 );
   lbl_bannerRotor->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   top->addWidget( lbl_bannerRotor, row, 0, 1, 2 );

   pb_addRotor = us_pushbutton( tr( "Add new Rotor" ) );
   connect( pb_addRotor, SIGNAL( clicked() ), this, SLOT( addRotor() ) );
   top->addWidget( pb_addRotor, row++, 2, 1, 2);

   // List the rotors for selection
   lw_rotors = us_listwidget();
   lw_rotors-> setSortingEnabled( true );
   connect( lw_rotors, SIGNAL( itemClicked  ( QListWidgetItem* ) ),
                       SLOT  ( selectRotor  ( QListWidgetItem* ) ) );
   top->addWidget( lw_rotors, row, 0, 3, 2);

   pb_deleteRotor = us_pushbutton( tr( "Delete Rotor" ) );
   connect( pb_deleteRotor, SIGNAL( clicked() ), this, SLOT( deleteRotor() ) );
   top->addWidget( pb_deleteRotor, row++, 2, 1, 2);

   QLabel *lbl_name = us_label( tr(" Name of Rotor:"), -1 );
   top->addWidget( lbl_name, row, 2 );

   le_name = us_lineedit( "", -1 );
   le_name->setText( tr("< not selected >"));
   connect( le_name, SIGNAL( textEdited  ( const QString & ) ),
                     SLOT  ( updateName  ( const QString & ) ) );
   top->addWidget( le_name, row++, 3);

   QLabel *lbl_serialNumber = us_label( tr(" Rotor Serial Number:"), -1 );
   top->addWidget( lbl_serialNumber, row, 2 );

   le_serialNumber = us_lineedit( "", -1 );
   le_serialNumber->setText( tr("< not selected >"));
   connect( le_serialNumber, SIGNAL( textEdited         ( const QString & ) ),
                             SLOT  ( updateSerialNumber ( const QString & ) ) );
   top->addWidget( le_serialNumber, row++, 3);

   QLabel* lbl_bannerCalibration = us_banner( tr( "Click to select a Rotor Calibration: " ), -1 );
   lbl_bannerCalibration->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   top->addWidget( lbl_bannerCalibration, row, 0, 1, 2);

   pb_saveCalibration = us_pushbutton( tr( "Save Calibration Data" ) );
   connect( pb_saveCalibration, SIGNAL( clicked() ), this, SLOT( saveCalibration() ) );
   top->addWidget( pb_saveCalibration, row++, 2, 1, 2);

   lw_calibrations = us_listwidget();
   lw_calibrations-> setSortingEnabled( false ); // comes out of mysql sorted
   connect( lw_calibrations, SIGNAL( itemClicked  ( QListWidgetItem* ) ),
                         SLOT  ( selectCalibration( QListWidgetItem* ) ) );
   top->addWidget( lw_calibrations, row, 0, 5, 2);

   pb_deleteCalibration = us_pushbutton( tr( "Delete Selected Calibration" ) );
   connect( pb_deleteCalibration, SIGNAL( clicked() ), this, SLOT( deleteCalibration() ) );
   top->addWidget( pb_deleteCalibration, row++, 2, 1, 2);

   pb_viewReport = us_pushbutton( tr( "View Calibration Report" ) );
   connect( pb_viewReport, SIGNAL( clicked() ), this, SLOT( viewReport() ) );
   top->addWidget( pb_viewReport, row++, 2, 1, 2);

   QLabel *lbl_coefficients = us_label( tr(" Rotor Stretch Coefficient 1:"), -1 );
   top->addWidget( lbl_coefficients, row, 2 );

   le_coefficient1 = us_lineedit( "", -1 );
   le_coefficient1->setText( tr("< not available >"));
   le_coefficient1->setReadOnly(true);
   top->addWidget( le_coefficient1, row++, 3);

   QLabel *lbl_date = us_label( tr(" Rotor Stretch Coefficient 2:"), -1 );
   top->addWidget( lbl_date, row, 2 );

   le_coefficient2 = us_lineedit( "", -1 );
   le_coefficient2->setText( tr("< not available >"));
   le_coefficient2->setReadOnly(true);
   top->addWidget( le_coefficient2, row++, 3);

   QLabel *lbl_force = us_label( tr(" Rotor omega2t:"), -1 );
   top->addWidget( lbl_force, row, 2 );

   le_omega2t = us_lineedit( "", -1 );
   le_omega2t->setText( tr("< not available >"));
   le_omega2t->setReadOnly(true);
   top->addWidget( le_omega2t, row++, 3);

   // some pushbuttons
   QHBoxLayout* buttons = new QHBoxLayout;

   pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), this, SLOT( load() ) );
   buttons->addWidget( pb_reset );

   pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), this, SLOT( help() ) );
   buttons->addWidget( pb_help );

   pb_accept = us_pushbutton( tr( "Close" ) );

   if ( signal )
   {
      QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
      connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
      buttons->addWidget( pb_cancel );

      pb_accept -> setText( tr( "Accept" ) );
   }

   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept() ) );
   buttons->addWidget( pb_accept );

   top->addLayout( buttons, row, 0, 1, 4 );

   load();      // Loads labs, rotors
   
}

void US_RotorGui::reset( void )
{
   pb_saveCalibration  ->setEnabled( false );
   pb_reset            ->setEnabled( true );
   pb_addRotor         ->setEnabled( true );

   // buttons to enable if a rotor is selected
   bool enabled = lw_rotors->count() > 0 &&
                  lw_rotors->currentIndex().row() != -1;
   pb_deleteRotor      ->setEnabled( enabled );

   // buttons to enable if a calibration is selected
   enabled = lw_calibrations->count() > 0 &&
             lw_calibrations->currentIndex().row() != -1;
   pb_deleteCalibration->setEnabled( enabled );
   pb_viewReport       ->setEnabled( enabled );

   // Figure out if the accept button should be enabled
   if ( ! signal )      // Then it's just a close button
      pb_accept->setEnabled( true );

   else if ( lw_rotors->currentIndex().row() == -1 )
      pb_accept->setEnabled( false );     // rotor is not selected

   else if ( lw_calibrations->currentIndex().row() == -1 )
      pb_accept->setEnabled( false );     // calibration is not selected

   else
      pb_accept->setEnabled( true );
   
   if ( savingCalibration )
   {
      // In this case we came from US_RotorCalibration, so disable all
      // pushbuttons except for that one
      pb_saveCalibration  ->setEnabled( lw_rotors->currentIndex().row() != -1 );
      pb_accept           ->setEnabled( true ); // a close button
      pb_reset            ->setEnabled( false );
      pb_addRotor         ->setEnabled( false );
      pb_deleteRotor      ->setEnabled( false );
      pb_deleteCalibration->setEnabled( false );
      pb_viewReport       ->setEnabled( false );
      
      lw_calibrations     ->clear();
      lw_calibrations     ->addItem( currentCalibration.lastUpdated.toString( "d MMMM yyyy" ) );
   }
}

int US_RotorGui::getIndex( void )
{
   if ( lw_rotors->count() == 0 )
   {
      // Couldn't read anything---list widget is empty
      return 0;
   }

   QString itemText  = lw_rotors->currentItem()->text();
   QStringList parts = itemText.split( ":" );
   int rotorID       = parts[ 0 ].toInt();   // The ID in the Rotor class

   return rotorID;
}

void US_RotorGui::addRotor( void )
{
   int dbdisk = ( disk_controls->db() ) ? US_Disk_DB_Controls::DB
                                        : US_Disk_DB_Controls::Disk;

   currentRotor.reset();
   US_AbstractRotorGui *arDialog = new US_AbstractRotorGui( true, dbdisk, &currentRotor );
   connect( arDialog, SIGNAL ( accepted() ), this, SLOT ( newRotor() ) ); 
   connect( arDialog, SIGNAL ( use_db        ( bool ) ),
                      SLOT   ( update_disk_db( bool ) ) );
   arDialog->show();
}

void US_RotorGui::update_disk_db( bool db )
{
   ( db ) ? disk_controls->set_db() : disk_controls->set_disk();
}

void US_RotorGui::newRotor( void )
{
   currentRotor.labID = labID;
   currentRotor.GUID  = US_Util::new_guid();
   le_name            ->setText( currentRotor.name );
   le_serialNumber    ->setText( currentRotor.serialNumber );

   if ( disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      int status = currentRotor.addRotorDB( &db );
      if ( status != US_DB2::OK )
         db_error( db.lastError() );

   }
   else
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "Adding new rotors on the local disk is not implemented yet." ) );
      return;
      /*
      solution.saveToDisk();

      solution.saveStatus = ( solution.saveStatus == US_Solution::DB_ONLY ) 
                          ? US_Solution::BOTH : US_Solution::HD_ONLY;
      */
   }

   // now get new rotorID and update the list box.
   loadRotors( labID );

   currentRotor.reset();
   currentCalibration.reset();
   
   // code to select the new rotor from the list
}

void US_RotorGui::db_error( const QString &error )
{
   QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) + error );
}

void US_RotorGui::selectRotor( QListWidgetItem *item )
{
   QString selected = item->text();
   QStringList parts = selected.split( ":" );

   int rotorID = parts[ 0 ].toInt();

   // These calibration info items are now out of scope
   le_coefficient1 ->setText( "< not available >" );
   le_coefficient2 ->setText( "< not available >" );
   le_omega2t      ->setText( "< not available >" );

   // Find out the rest of the rotor info
   if ( disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return;
      }
   
      US_Rotor::Status status = currentRotor.readDB( rotorID, &db );
      if ( status == US_Rotor::ROTOR_OK )
      {
         le_name         ->setText( currentRotor.name );
         le_serialNumber ->setText( currentRotor.serialNumber );
         rotorStatus = US_Rotor::DB_ONLY;
      }

   }

   else
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "Loading lab information from local disk is not implemented yet." ) );
      return;
//      currentRotor.readDisk( rotorGUID );
//      rotorStatus = US_Rotor::HD_ONLY;
   }

   if ( savingCalibration )
      pb_saveCalibration->setEnabled( true );

   else
   {
      pb_deleteRotor->setEnabled( true );

      // Get the associated calibration profiles
      readCalibrationProfiles( rotorID );
   }

}

void US_RotorGui::deleteRotor( void )
{
   // let's make sure we know which rotor
   QString selected = lw_rotors->currentItem()->text();
   QStringList parts = selected.split( ":" );

   int rotorID = parts[ 0 ].toInt();

   if ( disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return;
      }

      int status = US_Rotor::Rotor::deleteRotorDB( rotorID, &db );
      if ( status == US_DB2::ROTOR_IN_USE )
      {
         QString error = tr( "This rotor is in use, and can't be deleted" );
         db_error( error );
         return;
      }

      else if ( status != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }
   }

   else
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "Deleting rotors on the local disk is not implemented yet." ) );
      return;
   }

   loadRotors( labID );

   currentRotor.reset();
   currentCalibration.reset();
   reset();
}

bool US_RotorGui::readCalibrationProfiles( int rotorID )
{
   QStringList calibrationDescriptions;

   if ( disk_controls->db() )
   {
      // Find out what rotor calibrations we have
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
      
      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return( false );
      }
      
      QStringList q( "get_rotor_calibration_profiles" );
      q  << QString::number( rotorID );
      db.query( q );
      
      lw_calibrations->clear();
      while ( db.next() )
      {
         QString ID   = db.value( 0 ).toString();
         QStringList dateParts = db.value( 1 ).toString().split( " " );
         QDate dateUpdated = QDate::fromString( dateParts[ 0 ], "yyyy-MM-dd"  );
         calibrationDescriptions << ( ID + ": " + dateUpdated.toString( "d MMMM yyyy" ) );
      }

   }

   else
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "Loading lab information from local disk is not implemented yet." ) );
      return( false );
   }

   lw_calibrations->addItems( calibrationDescriptions );

   return( true );
}

void US_RotorGui::selectCalibration( QListWidgetItem *item )
{
   QString selected = item->text();
   QStringList parts = selected.split( ":" );

   int calibrationID = parts[ 0 ].toInt();

   // Find out the rest of the rotor calibration info
   if ( disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return;
      }
   
      US_Rotor::Status status = currentCalibration.readDB( calibrationID, &db );
      if ( status == US_Rotor::ROTOR_OK )
      {
         le_coefficient1 ->setText( QString::number( currentCalibration.coeff1  ) );
         le_coefficient2 ->setText( QString::number( currentCalibration.coeff2  ) );
         le_omega2t      ->setText( QString::number( currentCalibration.omega2t ) );
      }

   }

   else
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "Loading calibration information from local disk is not implemented yet." ) );
      return;
//      currentRotor.readDisk( rotorGUID );
//      rotorStatus = US_Rotor::HD_ONLY;
   }

   if ( ! savingCalibration )
   {
      pb_deleteCalibration->setEnabled( true );
      pb_viewReport       ->setEnabled( true );
   }
}

void US_RotorGui::viewReport( void )
{
   US_Editor *edit = new US_Editor ( US_Editor::LOAD, true );
   edit->setWindowTitle( tr("Rotor Calibration Report") );
   edit->move( this->pos() + QPoint( 100, 100 ) );
   edit->resize( 600, 500 );
   edit->e->setFont( QFont( US_GuiSettings::fontFamily(),
                            US_GuiSettings::fontSize() ) );
   edit->e->setText( currentCalibration.report );
   edit->show();
}

void US_RotorGui::deleteCalibration( void )
{
   // let's make sure we know which calibration
   QString selected = lw_calibrations->currentItem()->text();
   QStringList parts = selected.split( ":" );

   int calibrationID = parts[ 0 ].toInt();

   if ( disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return;
      }

      int status = US_Rotor::RotorCalibration::deleteCalibrationDB( calibrationID, &db );
      if ( status == US_DB2::ROTOR_IN_USE )
      {
         QString error = tr( "This rotor calibration is in use, and can't be deleted" );
         db_error( error );
         return;
      }

      else if ( status != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }
   }

   else
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "Deleting rotor calibrations on the local disk is not implemented yet." ) );
      return;
   }

   reset();
}

void US_RotorGui::updateName( const QString &name)
{
   currentRotor.name = name;
}

void US_RotorGui::updateSerialNumber( const QString &number)
{
   currentRotor.serialNumber = number;
}

void US_RotorGui::saveCalibration()
{

   if ( lw_rotors->currentIndex().row() == -1 )
   {
      // then a rotor hasn't been selected
      return;
   }

   QString selected = lw_rotors->currentItem()->text();
   QStringList parts = selected.split( ":" );

   int rotorID = parts[ 0 ].toInt();

   // Find out the rest of the rotor info
   if ( disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return;
      }
   
      int status = currentCalibration.saveDB( rotorID, &db );
      if ( status == US_DB2::OK )
      {
         reset();

         QMessageBox::information( this,
            tr( "Attention" ),
            tr( "Calibration was saved to the DB." ) );
      }

   }

   else
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "Saving rotor calibration to local disk is not implemented yet." ) );
      return;
   }
}

void US_RotorGui::source_changed( bool db )
{
   QStringList DB = US_Settings::defaultDB();

   if ( db && ( DB.size() < 5 ) )
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "There is no default database set." ) );
   }

   emit use_db( db );
   qApp->processEvents();

   reset();
}

void US_RotorGui::accept( void )
{
   if ( signal )
   {
      emit RotorCalibrationSelected ( currentCalibration );
   }

   close();
}

void US_RotorGui::cancel( void )
{
   if ( signal )
   {
      emit RotorCalibrationCanceled ();
   }

   close();
}

bool US_RotorGui::load( void )
{
   QStringList labDescriptions;

   if ( disk_controls->db() )
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
      
      QStringList q( "get_lab_names" );
      db.query( q );
      
      cb_lab->clear();
      while ( db.next() )
      {
         QString ID   = db.value( 0 ).toString();
         QString Desc = db.value( 1 ).toString();
         labDescriptions << ( ID + ": " + Desc );
      }
   }

   else
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "Loading lab information from local disk is not implemented yet." ) );
      return( false );
   }

   cb_lab->addItems( labDescriptions );

   changeLab( 0 );      // To display labs, rotors

   reset();             // Redo buttons after all data is loaded

   return( true );
}

// Function to change the current lab
void US_RotorGui::changeLab( int )
{
   if ( cb_lab->count() == 0 )
   {
      return;     // nothing to display
   }

   QString itemText  = cb_lab->currentText();
   QStringList parts = itemText.split( ":" );
   labID             = parts[ 0 ].toInt();

   le_name         ->setText( "" );
   le_serialNumber ->setText( "" );

   loadRotors( labID );
}

bool US_RotorGui::loadRotors( const int &id )
{
   QStringList rotorDescriptions;

   // Clear out lists, line edits
   currentRotor.reset();
   lw_rotors      ->clear();
   lw_calibrations->clear();
   le_name        ->setText( "< not selected >" );
   le_serialNumber->setText( "< not selected >" );
   le_coefficient1->setText( "< not available >" );
   le_coefficient2->setText( "< not available >" );
   le_omega2t     ->setText( "< not available >" );

   if ( ! savingCalibration )
      currentCalibration.reset();

   if ( disk_controls->db() )
   {
      // Find out what rotors we have
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return( false );
      }
   
      QStringList q( "get_rotor_names" );
      q << QString::number( id );     // find rotors from this lab
      db.query( q );
   
      while ( db.next() )
      {
         QString rotorID   = db.value( 0 ).toString();
         QString rotorDesc = db.value( 1 ).toString();
         rotorDescriptions << ( rotorID + ": " + rotorDesc );
      }

   }

   else
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "Loading rotors from local disk is not implemented yet." ) );
      return( false );
   }

   lw_rotors->addItems( rotorDescriptions );

   return( true );   
}

void US_RotorGui::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to databasee \n" ) + error );
}

