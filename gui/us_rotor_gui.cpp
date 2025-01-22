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
     bool                        new_calibration,
     bool                        signal_wanted,
     int                         select_db_disk
     ) : US_WidgetsDialog( 0, 0 ), currentCalibration( calibration )
{
   this->savingCalibration = new_calibration;
   this->signal            = signal_wanted;

   setAttribute( Qt::WA_DeleteOnClose );
   setupGui( select_db_disk );

   if ( savingCalibration )
   {
      QMessageBox::information( this,
         tr( "Attention" ),
         tr( "Please enter the calibration name, omega2t, "
             "and click Save Calibration Data, or Close" ) );

      // The rotor calibration data was passed, so let's fill in the rotor data
      int rotorID = currentCalibration.rotorID;
      readRotor( select_db_disk, rotorID );

      // Now select the current rotor
      QList< QListWidgetItem* > items 
         = lw_rotors->findItems( QString::number( rotorID ), Qt::MatchStartsWith );
      if ( items.count() == 1 )                    // should be exactly 1
         lw_rotors->setCurrentItem( items[ 0 ] );
   
      // Update other rotor information
      le_name        ->setText( currentRotor.name );
      le_serialNumber->setText( currentRotor.serialNumber );

      lw_calibrations     ->clear();
      lw_calibrations     ->addItem( currentCalibration.lastUpdated.toString( "d MMMM yyyy" ) );
      lw_calibrations     ->setCurrentRow( 0 );

      le_calibrationLabel->setText( "" );
      le_coefficient1    ->setText( QString::number( currentCalibration.coeff1  ) );
      le_coefficient2    ->setText( QString::number( currentCalibration.coeff2  ) );
      le_omega2t         ->setText( "" );

      // Also enable line edits for saving calibrations
      le_calibrationLabel ->setReadOnly( false );
      le_calibrationLabel ->setPalette( US_GuiSettings::editColor() );
      le_omega2t ->setReadOnly( false );
      le_omega2t ->setPalette( US_GuiSettings::editColor() );
   }

   reset();
}

US_RotorGui::US_RotorGui(
     bool                              signal_wanted,
     int                               select_db_disk,
     const US_Rotor::Rotor&            rotorIn,
     const US_Rotor::RotorCalibration& calibrationIn
     )
: US_WidgetsDialog( nullptr, Qt::WindowFlags() ),
  currentRotor( rotorIn ), 
  currentCalibration( calibrationIn ), 
  signal( signal_wanted )
{
   setAttribute( Qt::WA_DeleteOnClose );
   this->savingCalibration = false;

   // Let's see if the user passed a rotorID, and 
   // find out the rest of the rotor info
   int rotorID       = currentRotor.ID;     // save for later; they get wiped out
   int calibrationID = currentCalibration.ID;
   US_Rotor::Status status;

   setupGui( select_db_disk );

   if ( rotorID > 0 && calibrationID > 0 )
   {
      // Then this information was passed; fill in with our own data
      status = readRotor      ( select_db_disk, rotorID );
      if ( status == US_Rotor::ROTOR_OK )
         status = readCalibration( select_db_disk, calibrationID );

      // Now select the current rotor
      QList< QListWidgetItem* > items;
      if ( status == US_Rotor::ROTOR_OK )
         items = lw_rotors->findItems( QString::number( rotorID ), Qt::MatchStartsWith );
      if ( items.count() == 1 )                    // should be exactly 1
         lw_rotors->setCurrentItem( items[ 0 ] );
   
      // And the current calibration
      items.clear();
      items = lw_calibrations->findItems( QString::number( calibrationID ), Qt::MatchStartsWith );
      if ( items.count() == 1 )
         lw_calibrations->setCurrentItem( items[ 0 ] );

   }

   reset();
}

bool US_RotorGui::load_rotor(QString& load_init, double& coeff1, double& coeff2)
{
   bool ok;
   // check if the load_init are just two coefficients
   if ( load_init.split( ":" ).size() == 2 )
   {
      // split load_init and convert both parts to double and return
      QStringList parts = load_init.split( ":" );
      coeff1 = parts[0].toDouble(  );
      coeff2 = parts[1].toDouble(  );
      return true;
   }
   QVector < US_Rotor::Lab > labList;
   QVector < QString > rotorList;
   labList.clear();
   rotorList.clear();
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

      // Get information about all the labs
      if ( US_Rotor::readLabsDB( labList, &db ) == US_Rotor::NOT_FOUND )
      {
         QMessageBox::warning( this, tr( "Database Problem" ),
            tr( "Could not read lab information \n" ) );

         return false;
      }

   }

   else
   {
      // Get information about all the labs
      if ( US_Rotor::readLabsDisk( labList ) == US_Rotor::NOT_FOUND )
      {
         QMessageBox::warning( this, tr( "Disk Problem" ),
            tr( "Could not read lab information \n" ) );

         return false;
      }

   }


   for( auto& lab : labList )
   {
      qDebug() << "labID:" << lab.ID << "name:" << lab.name;
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
         q << QString::number( lab.ID );     // find rotors from this lab
         db.query( q );

         while ( db.next() )
         {
            QString rotorID   = db.value( 0 ).toString();
            QString rotorDesc = db.value( 1 ).toString();
            qDebug() << "rotorID:" << rotorID << "rotorDesc:" << rotorDesc << "labID:" << lab.ID;
            rotorList << ( rotorID + ":" + rotorDesc + ":" + QString::number( lab.ID ) );
         }

      }

      else
      {
         // Get information about all the rotors in this lab
         QVector< US_Rotor::Rotor > rotors;

         if ( US_Rotor::readRotorsFromDisk( rotors, labID ) == US_Rotor::NOT_FOUND )
         {
            QMessageBox::warning( this, tr( "Disk Problem" ),
               tr( "Could not read rotor information \n" ) );
            return( false );
         }

         for (auto & rotor : rotors)
         {
            qDebug() << "rotorID:" << rotor.ID << "name:" << rotor.name << "labID:" << lab.ID;
            rotorList << ( QString::number( rotor.ID ) + ":" + rotor.name + ":" + QString::number( rotor.labID ) );
         }
      }
   }
   // If we have a rotor to load, then we need to find it
   qDebug() << "load_init:" << load_init << load_init.isEmpty();
   if ( ! load_init.isEmpty() )
   {
      // Find the rotor
      for (auto & rotor : rotorList)
      {
         qDebug() << "rotor:" << rotor;
         QStringList parts = rotor.split( ":" );
         if ( parts.contains( load_init ) )
         {
            int rotorID = parts[ 0 ].toInt();
            US_Rotor::Status status = readRotor( disk_controls->db( ), rotorID );
            qDebug() << "status:" << status;
            bool cal_status = readCalibrationProfiles( rotorID );
            qDebug() << "cal_status:" << cal_status;
            coeff1 = currentCalibration.coeff1;
            coeff2 = currentCalibration.coeff2;
            return true;
         }
      }
   }
   return false;
}

void US_RotorGui::setupGui( int select_db_disk )
{
   setWindowTitle( tr( "Rotor Management" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* top = new QGridLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );

   // Very light gray, for read-only line edits
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

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
   lw_rotors-> setSortingEnabled( false );
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
   le_name->setPalette ( gray );
   le_name->setReadOnly( true );
   connect( le_name, SIGNAL( textEdited  ( const QString & ) ),
                     SLOT  ( updateName  ( const QString & ) ) );
   top->addWidget( le_name, row++, 3);

   QLabel *lbl_serialNumber = us_label( tr(" Rotor Serial Number:"), -1 );
   top->addWidget( lbl_serialNumber, row, 2 );

   le_serialNumber = us_lineedit( "", -1 );
   le_serialNumber->setText( tr("< not selected >"));
   le_serialNumber->setPalette ( gray );
   le_serialNumber->setReadOnly( true );
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
   connect( lw_calibrations, SIGNAL( itemClicked      ( QListWidgetItem* ) ),
                             SLOT  ( selectCalibration( QListWidgetItem* ) ) );
   top->addWidget( lw_calibrations, row, 0, 6, 2);

   pb_deleteCalibration = us_pushbutton( tr( "Delete Selected Calibration" ) );
   connect( pb_deleteCalibration, SIGNAL( clicked() ), this, SLOT( deleteCalibration() ) );
   top->addWidget( pb_deleteCalibration, row++, 2, 1, 2);

   pb_viewReport = us_pushbutton( tr( "View Calibration Report" ) );
   connect( pb_viewReport, SIGNAL( clicked() ), this, SLOT( viewReport() ) );
   top->addWidget( pb_viewReport, row++, 2, 1, 2);

   QLabel* lbl_calibName = us_label( tr( " Calibration Name:"), -1 );
   top->addWidget( lbl_calibName, row, 2 );

   le_calibrationLabel = us_lineedit( "", -1 );
   le_calibrationLabel->setText( tr("< not available >"));
   le_calibrationLabel->setPalette ( gray );
   le_calibrationLabel->setReadOnly( true );
   connect( le_calibrationLabel, SIGNAL( textEdited  ( const QString&   ) ),
                                 SLOT  ( updateLabel ( const QString&   ) ) );
   top->addWidget( le_calibrationLabel, row++, 3);

   QLabel *lbl_coefficients = us_label( tr(" Rotor Stretch Coefficient 1:"), -1 );
   top->addWidget( lbl_coefficients, row, 2 );

   le_coefficient1 = us_lineedit( "", -1 );
   le_coefficient1->setText( tr("< not available >"));
   le_coefficient1->setPalette ( gray );
   le_coefficient1->setReadOnly( true );
   top->addWidget( le_coefficient1, row++, 3);

   QLabel *lbl_date = us_label( tr(" Rotor Stretch Coefficient 2:"), -1 );
   top->addWidget( lbl_date, row, 2 );

   le_coefficient2 = us_lineedit( "", -1 );
   le_coefficient2->setText( tr("< not available >"));
   le_coefficient2->setPalette ( gray );
   le_coefficient2->setReadOnly( true );
   top->addWidget( le_coefficient2, row++, 3);

   QLabel *lbl_force = us_label( tr(" Rotor omega2t:"), -1 );
   top->addWidget( lbl_force, row, 2 );

   le_omega2t = us_lineedit( "", -1 );
   le_omega2t->setText( tr("< not available >"));
   le_omega2t->setPalette ( gray );
   le_omega2t->setReadOnly( true );
   connect( le_omega2t, SIGNAL( textEdited    ( const QString&   ) ),
                        SLOT  ( updateOmega2t ( const QString&   ) ) );
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
   lw_rotors           ->setEnabled( true );
   pb_saveCalibration  ->setEnabled( false );
   pb_reset            ->setEnabled( true );
   pb_addRotor         ->setEnabled( ( US_Settings::us_inv_level() > 2 ) &&
                                     ( disk_controls->db()             ) );

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
      lw_rotors           ->setEnabled( false );
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

   // Create a dummy configuration to associate before the real one
   // is available
   currentCalibration.reset();
   currentCalibration.GUID   = US_Util::new_guid();
   currentCalibration.label  = "Dummy Calibration";
   currentCalibration.report = "This is a dummy calibration --- please replace.";
   currentCalibration.calibrationExperimentID = -1;      // special value

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

      else
      {
         int rotorID = db.lastInsertID();
         currentCalibration.rotorID = rotorID;
         currentCalibration.saveDB( rotorID, &db );
      }

   }
   else
   {
      currentRotor.saveDisk();
      currentCalibration.saveDisk();
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
   US_Rotor::Status status;

   // These calibration info items are now out of scope
   le_coefficient1 ->setText( "< not available >" );
   le_coefficient2 ->setText( "< not available >" );
   le_omega2t      ->setText( "< not available >" );

   // Find out the rest of the rotor info
   status = readRotor( disk_controls->db(), rotorID );

   // Update rotor info on the form, if we can
   if ( status == US_Rotor::ROTOR_OK )
   {
      le_name            ->setText( currentRotor.name );
      le_serialNumber    ->setText( currentRotor.serialNumber );
      le_calibrationLabel->setText( currentCalibration.label );
      rotorStatus = US_Rotor::DB_ONLY;
   }

   pb_deleteRotor->setEnabled( true );

   // Get the associated calibration profiles
   readCalibrationProfiles( rotorID );
}

US_Rotor::Status US_RotorGui::readRotor( int disk_db, int rotorID )
{
   // Let's see if the user passed a rotorID, and 
   // find out the rest of the rotor info
   US_Rotor::Status status;
   currentRotor.reset();

   if ( rotorID <= 0 )
      return( US_Rotor::NOT_FOUND );

   if ( disk_db == US_Disk_DB_Controls::DB )
   {                         
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return( US_Rotor::CONNECT_ERROR );
      }
   
      status = currentRotor.readDB( rotorID, &db );

      // Since there is no save rotor button, save any that is
      // selected to disk automatically
      currentRotor.saveDisk();
   }

   else
   {
      status = currentRotor.readDisk( rotorID );
   }

   return status;
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
         QString label = db.value( 2 ).toString();
         calibrationDescriptions << ( ID + ": " + label + dateUpdated.toString( " (d MMMM yyyy)" ) );
      }

   }

   else
   {
      // Get information about all the rotor calibrations in this lab
      QVector< US_Rotor::RotorCalibration > rc;

      if ( US_Rotor::readCalibrationProfilesDisk( rc, rotorID ) == US_Rotor::NOT_FOUND )
      {
         QMessageBox::warning( this, tr( "Disk Problem" ),
            tr( "Could not find rotor calibration profiles \n" ) );
         return( false );
      }

      for ( int i = 0; i < rc.size(); i++ )
      {
         if ( rc[ i ].rotorID == rotorID )
            calibrationDescriptions << ( QString::number( rc[ i ].ID ) + ": " + 
                                         rc[ i ].lastUpdated.toString( "d MMMM yyyy" ) );
      }
   }

   lw_calibrations->clear();
   lw_calibrations->addItems( calibrationDescriptions );

   // Select the first one in the list by default
   if ( lw_calibrations->count() > 0 )
   {
      lw_calibrations->setCurrentRow( 0 );
      selectCalibration( lw_calibrations->currentItem() );
   }

   return( true );
}

void US_RotorGui::selectCalibration( QListWidgetItem *item )
{
   QString selected = item->text();
   QStringList parts = selected.split( ":" );

   int calibrationID = parts[ 0 ].toInt();
   US_Rotor::Status status;

   // Find out the rest of the rotor calibration info
   status = readCalibration( disk_controls->db(), calibrationID );

   // Now populate what we can on the form
   if ( status == US_Rotor::ROTOR_OK )
   {
      le_calibrationLabel->setText( currentCalibration.label );
      le_coefficient1    ->setText( QString::number( currentCalibration.coeff1  ) );
      le_coefficient2    ->setText( QString::number( currentCalibration.coeff2  ) );
      le_omega2t         ->setText( QString::number( currentCalibration.omega2t ) );
   }

   if ( ! savingCalibration )
   {
      pb_deleteCalibration->setEnabled( true );
   }

   reset();
}

US_Rotor::Status US_RotorGui::readCalibration( int disk_db, int calibrationID )
{
   // Let's see if the user passed a calibrationID, and 
   // find out the rest of the calibration info
   US_Rotor::Status status;
   currentCalibration.reset();

   if ( calibrationID <= 0 )
      return( US_Rotor::NOT_FOUND );

   if ( disk_db == US_Disk_DB_Controls::DB )
   {                         
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return( US_Rotor::CONNECT_ERROR );
      }
   
      status = currentCalibration.readDB( calibrationID, &db );

      // Since there is no save rotor calibration button, save any that is
      // selected to disk automatically
      currentCalibration.saveDisk();
   }

   else
   {
      status = currentCalibration.readDisk( calibrationID );
   }

   return status;
}

void US_RotorGui::viewReport( void )
{
   US_EditorGui* edit = new US_EditorGui();
   edit->setWindowTitle( tr("Rotor Calibration Report") );
   edit->move( this->pos() + QPoint( 100, 100 ) );
   edit->resize( 500, 400 );
   edit->editor->e->setFont( QFont( US_GuiSettings::fontFamily(),
                                    US_GuiSettings::fontSize() ) );
   edit->editor->e->setText( currentCalibration.report );
   edit->exec();
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
      if ( status == US_DB2::CALIB_IN_USE )
      {
         QString error = tr( "This rotor calibration is in use, and can't be deleted" );
         db_error( error );
         return;
      }

      else if ( status != US_DB2::OK )
      {
         db_error( QString::number( status ) + ": " + db.lastError() );
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

   readCalibrationProfiles( currentRotor.ID );
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

void US_RotorGui::updateLabel( const QString &label)
{
   currentCalibration.label = label;
}

void US_RotorGui::updateOmega2t( const QString &omega2t)
{
   currentCalibration.omega2t = omega2t.toFloat();
}

void US_RotorGui::saveCalibration()
{
   // Should already have everything we need except for the label
   int rotorID = currentCalibration.rotorID;
   currentCalibration.label  = le_calibrationLabel->text();

   // Save the calibration info
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
         replaceDummyCalibration();

         reset();

         QMessageBox::information( this,
            tr( "Attention" ),
            tr( "Calibration was saved to the DB." ) );
      }
   }

   else
   {
      currentCalibration.saveDisk();

      reset();

      QMessageBox::information( this,
         tr( "Attention" ),
         tr( "Calibration was saved to the Disk." ) );
   }

   // Now that we've saved the calibration, convert to regular mode
   savingCalibration = false;

   // Disable calibration input fields
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );
   le_calibrationLabel->setPalette ( gray );
   le_calibrationLabel->setReadOnly( true );
   le_omega2t->setPalette ( gray );
   le_omega2t->setReadOnly( true );

   // Load all the calibration profiles
   readCalibrationProfiles( currentRotor.ID );
   reset();
}

// A function to find out if the original dummy calibration is still there,
//  and replace it with the current one if it does. Also, to delete
//  the dummy.
void US_RotorGui::replaceDummyCalibration( void )
{
   // This is only valid on the DB
   if ( ! disk_controls->db() )
   {
      QString error = tr( "This is a database-only function." );
      db_error( error );
      return;
   }

   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );
   
   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   int oldCalibrationID = -1;                   // To return the dummy's ID in
   int status = currentCalibration.replaceDummyDB( oldCalibrationID, &db );

   if ( status == US_DB2::NO_CALIB )
   {
      QString error = tr( "The replacement rotor calibration could not be found." );
      db_error( error );
      return;
   }

   else if ( db.lastErrno() != US_DB2::OK )
   {
      db_error( db.lastError() );
      return;
   }

   // Was a dummy calibration found?
   else if ( oldCalibrationID == -1 )
      return;

   // If we get here then a dummy calibration was found and replaced without error
   status = US_Rotor::RotorCalibration::deleteCalibrationDB( oldCalibrationID, &db );
   if ( status == US_DB2::CALIB_IN_USE )
   {
      QString error = tr( "The database shows the dummy calibration is still in use, and can't be deleted" );
      db_error( error );
      return;
   }

   else if ( status != US_DB2::OK )
   {
      db_error( QString::number( status ) + ": " + db.lastError() );
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

   load();

   emit use_db( db );
   qApp->processEvents();

   reset();
}

void US_RotorGui::accept( void )
{
   if ( signal )
   {
      emit RotorCalibrationSelected ( currentRotor, currentCalibration );
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
   QVector < US_Rotor::Lab > labList;

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
      
      // Get information about all the labs
      if ( US_Rotor::readLabsDB( labList, &db ) == US_Rotor::NOT_FOUND )
      {
         QMessageBox::warning( this, tr( "Database Problem" ),
            tr( "Could not read lab information \n" ) );

         return false;
      }

   }

   else
   {
      // Get information about all the labs
      if ( US_Rotor::readLabsDisk( labList ) == US_Rotor::NOT_FOUND )
      {
         QMessageBox::warning( this, tr( "Disk Problem" ),
            tr( "Could not read lab information \n" ) );
         
         return false;
      }

   }

   cb_lab->clear();
   int index = 0;
   foreach ( US_Rotor::Lab lab, labList )
   {
      cb_lab->addItem( QString::number( lab.ID ) + ": " + lab.name );
      if ( lab.ID == currentRotor.labID )
         index = cb_lab->currentIndex();
   }
   if ( cb_lab->count() > 0 )
      cb_lab->setCurrentIndex( index );

   changeLab( 0 );      // To display labs, rotors

   reset();             // Redo buttons after all data is loaded

   return true;
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

   le_name            ->setText( "" );
   le_serialNumber    ->setText( "" );
   le_calibrationLabel->setText( "" );

   loadRotors( labID );
}

bool US_RotorGui::loadRotors( const int labID )
{
   QStringList rotorDescriptions;

   // Clear out lists, line edits
   currentRotor.reset();
   lw_rotors          ->clear();
   lw_calibrations    ->clear();
   le_name            ->setText( "< not selected >" );
   le_serialNumber    ->setText( "< not selected >" );
   le_calibrationLabel->setText( "< not selected >" );
   le_coefficient1    ->setText( "< not available >" );
   le_coefficient2    ->setText( "< not available >" );
   le_omega2t         ->setText( "< not available >" );

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
      q << QString::number( labID );     // find rotors from this lab
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
      // Get information about all the rotors in this lab
      QVector< US_Rotor::Rotor > rotors;

      if ( US_Rotor::readRotorsFromDisk( rotors, labID ) == US_Rotor::NOT_FOUND )
      {
         QMessageBox::warning( this, tr( "Disk Problem" ),
            tr( "Could not read rotor information \n" ) );
         return( false );
      }

      for ( int i = 0; i < rotors.size(); i++ )
      {
         if ( rotors[ i ].labID == labID )
            rotorDescriptions << ( QString::number( rotors[ i ].ID ) + ": " + rotors[ i ].name );
      }
   }

   lw_rotors->addItems( rotorDescriptions );

   return( true );   
}

void US_RotorGui::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to database \n" ) + error );
}

