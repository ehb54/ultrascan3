//! \file us_rotor_gui.cpp

#include "us_rotor_gui.h"
#include "us_settings.h"
#include "us_license_t.h"
#include "us_util.h"
#include "us_db2.h"
#include "us_license.h"
#include "us_gui_settings.h"
#include "us_passwd.h"

//! \brief Main program for US_RotorGui. Loads translators and starts
//         the class US_FitMeniscus.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_RotorGui w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_RotorGui::US_RotorGui( ) : US_WidgetsDialog(0, 0)
{
   setupGui();
}

US_RotorGui::US_RotorGui( rotorCalibration calibration ) : US_WidgetsDialog(0, 0)
{
   setupGui();
   this->currentCalibration = calibration;
}

US_RotorGui::~US_RotorGui()
{
}

void US_RotorGui::reset( void )
{
}

void US_RotorGui::resetAbstractRotor( void )
{
   currentAbstractRotor.ID = -1;
   currentAbstractRotor.GUID = "";
   currentAbstractRotor.name = "";
   currentAbstractRotor.material = "";
   currentAbstractRotor.numHoles = -1;
   currentAbstractRotor.maxRPM = -1;
   currentAbstractRotor.magnetOffset = -1;
   currentAbstractRotor.cellCenter = -1.0;
   currentAbstractRotor.manufacturer = "";
}

void US_RotorGui::resetRotor( void )
{
   currentRotor.ID = -1;
   currentRotor.abstractRotorID = -1;
   currentRotor.labID = -1;
   currentRotor.GUID = "";
   currentRotor.name = "";
   currentRotor.serialNumber = "";
}

void US_RotorGui::resetRotorCalibration( void )
{
   currentCalibration.ID = -1;
   currentCalibration.GUID = "";
   currentCalibration.calibrationExperimentGUID = "";
   currentCalibration.coeff1 = 0.0;
   currentCalibration.coeff2 = 0.0;
   currentCalibration.report = "";
   currentCalibration.lastUpdated = QDate::currentDate();
   currentCalibration.omega2t = 0.0;
}

void US_RotorGui::setupGui()
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
   top->addWidget( lbl_bannerDB, row, 0, 1, 4 );

   row++;

   QGridLayout *lo_db = us_radiobutton( tr( "Use Database" ), rb_db );
   top->addLayout( lo_db, row, 0, 1, 2);
   connect( rb_db, SIGNAL( clicked() ), this, SLOT( check_db() ) );
   rb_db->setChecked( true );

   QGridLayout *lo_disk = us_radiobutton( tr( "Use Local Disk" ), rb_disk );
   top->addLayout( lo_disk, row, 2, 1, 2 );
   connect( rb_disk, SIGNAL( clicked() ), this, SLOT( check_disk() ) );

   row++;

   // labID
   QLabel* lbl_lab = us_label( tr( " Please select a Laboratory:" ) );
   top->addWidget( lbl_lab, row, 0, 1, 2 );
   cb_lab = new QComboBox( this );
   connect( cb_lab, SIGNAL( activated ( int ) ),      // Only if the user has changed it
                    SLOT  ( changeLab( int ) ) );
   top->addWidget( cb_lab, row, 2, 1, 2 );
   if(!loadLabs())
   {
      qDebug() << "Problem loading Labs...";
   }
   labID = getIndex(cb_lab->currentText());

   row++;

   QLabel* lbl_bannerRotor = us_banner( tr( "Click to select a Rotor: " ), -1 );
   lbl_bannerRotor->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   top->addWidget( lbl_bannerRotor, row, 0, 1, 2 );

   pb_addRotor = us_pushbutton( tr( "Add new Rotor" ) );
   connect( pb_addRotor, SIGNAL( clicked() ), this, SLOT( addRotor() ) );
   top->addWidget( pb_addRotor, row, 2, 1, 2);

   row++;

   lw_rotors = us_listwidget();
   lw_rotors-> setSortingEnabled( true );
   connect( lw_rotors, SIGNAL( itemClicked  ( QListWidgetItem* ) ),
                         SLOT  ( selectRotor( QListWidgetItem* ) ) );
   top->addWidget( lw_rotors, row, 0, 3, 2);

   pb_deleteRotor = us_pushbutton( tr( "Delete Rotor" ) );
   connect( pb_deleteRotor, SIGNAL( clicked() ), this, SLOT( deleteRotor() ) );
   top->addWidget( pb_deleteRotor, row, 2, 1, 2);

   if(!loadRotors(labID))
   {
      qDebug() << "Problem loading Rotors...";
   }
   
   row++;

   QLabel *lbl_name = us_label( tr(" Name of Rotor:"), -1 );
   top->addWidget( lbl_name, row, 2 );

   le_name = us_lineedit( "", -1 );
   le_name->setText( tr("< not selected >"));
   top->addWidget( le_name, row, 3);
   connect( le_name, SIGNAL( textEdited  ( const QString & ) ),
            SLOT  ( updateName ( const QString & ) ) );

   row++;

   QLabel *lbl_serialNumber = us_label( tr(" Rotor Serial Number:"), -1 );
   top->addWidget( lbl_serialNumber, row, 2 );

   le_serialNumber = us_lineedit( "", -1 );
   le_serialNumber->setText( tr("< not selected >"));
   top->addWidget( le_serialNumber, row, 3);
   connect( le_serialNumber, SIGNAL( textEdited  ( const QString & ) ),
            SLOT  ( updateSerialNumber ( const QString & ) ) );

   row++;

   QLabel* lbl_bannerCalibration = us_banner( tr( "Click to select a Rotor Calibration: " ), -1 );
   lbl_bannerCalibration->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   top->addWidget( lbl_bannerCalibration, row, 0, 1, 2);

   pb_saveCalibration = us_pushbutton( tr( "Save Calibration Data" ) );
   connect( pb_saveCalibration, SIGNAL( clicked() ), this, SLOT( saveCalibration() ) );
   top->addWidget( pb_saveCalibration, row, 2, 1, 2);

   row++;

   lw_calibrations = us_listwidget();
   lw_calibrations-> setSortingEnabled( true );
   connect( lw_calibrations, SIGNAL( itemClicked  ( QListWidgetItem* ) ),
                         SLOT  ( selectCalibration( QListWidgetItem* ) ) );
   top->addWidget( lw_calibrations, row, 0, 5, 2);

   pb_deleteCalibration = us_pushbutton( tr( "Delete Selected Calibration" ) );
   connect( pb_deleteCalibration, SIGNAL( clicked() ), this, SLOT( deleteCalibration() ) );
   top->addWidget( pb_deleteCalibration, row, 2, 1, 2);

   row++;

   pb_viewReport = us_pushbutton( tr( "View Calibration Report" ) );
   connect( pb_viewReport, SIGNAL( clicked() ), this, SLOT( viewReport() ) );
   top->addWidget( pb_viewReport, row, 2, 1, 2);

   row++;

   QLabel *lbl_coefficients = us_label( tr(" Rotor Stretch Coefficients:"), -1 );
   top->addWidget( lbl_coefficients, row, 2 );

   le_coefficients = us_lineedit( "", -1 );
   le_coefficients->setText( tr("< not available >"));
   le_coefficients->setReadOnly(true);
   top->addWidget( le_coefficients, row, 3);

   row++;

   QLabel *lbl_date = us_label( tr(" Calibration performed on:"), -1 );
   top->addWidget( lbl_date, row, 2 );

   le_date = us_lineedit( "", -1 );
   le_date->setText( tr("< not available >"));
   le_date->setReadOnly(true);
   top->addWidget( le_date, row, 3);

   row++;

   QLabel *lbl_force = us_label( tr(" Total Rotor Revolutions:"), -1 );
   top->addWidget( lbl_force, row, 2 );

   le_force = us_lineedit( "", -1 );
   le_force->setText( tr("< not available >"));
   le_force->setReadOnly(true);
   top->addWidget( le_force, row, 3);

   row++;

   pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), this, SLOT( help() ) );
   top->addWidget( pb_help, row, 0 );

   pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), this, SLOT( reset() ) );
   top->addWidget( pb_reset, row, 1 );

   pb_accept = us_pushbutton( tr( "Accept" ) );
   connect( pb_accept, SIGNAL( clicked() ), this, SLOT( accept() ) );
   top->addWidget( pb_accept, row, 2 );

   pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), this, SLOT( close() ) );
   top->addWidget( pb_close, row, 3 );
}

int US_RotorGui::getIndex( const QString &str )
{
   int pos = str.indexOf(":");
   QString str1 = str.left(pos);
   return str1.toInt();
}

void US_RotorGui::addRotor( void )
{
   resetAbstractRotor();
   resetRotor();
   US_AbstractRotorGui *abstractRotorWin = new US_AbstractRotorGui(&currentAbstractRotor, &currentRotor);
   connect(abstractRotorWin, SIGNAL (accepted()), this, SLOT (newRotor())); 
   abstractRotorWin->show();
}

void US_RotorGui::newRotor( void )
{
   currentRotor.labID = labID;
   currentRotor.GUID = US_Util::new_guid();
   le_name->setText(currentRotor.name);
   le_serialNumber->setText(currentRotor.serialNumber);
   QStringList q;
   if ( rb_disk -> isChecked() )
   {
      /*
      solution.saveToDisk();

      solution.saveStatus = ( solution.saveStatus == US_Solution::DB_ONLY ) 
                          ? US_Solution::BOTH : US_Solution::HD_ONLY;
      */
   }
   else
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }
      q.clear();
      q << "add_rotor";
      q << QString::number(currentRotor.abstractRotorID);
      q << currentRotor.abstractRotorGUID;
      q << QString::number(currentRotor.labID);
      q << currentRotor.name;
      q << currentRotor.serialNumber;
      db.query( q );
      q.clear();
      q << "last_error";
      db.query( q );
      db.next();
      QString s = db.value( 0).toString();
      q.clear();
      q << "last_insert_id";
      db.query( q );
      //db.next();
      int id = db.value( 0 ).toInt();
      qDebug() << "ID: " << id;
   }
   QMessageBox::information( this,
         tr( "Please note:" ),
         tr( "The new rotor definition has been saved to the database." ) );
   // now get new rotorID and update the list box.
}

void US_RotorGui::db_error( const QString &error )
{
   QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) + error );
}

void US_RotorGui::deleteRotor( void )
{
}

void US_RotorGui::viewReport( void )
{
}

void US_RotorGui::deleteCalibration( void )
{
}

void US_RotorGui::selectRotor( QListWidgetItem *item )
{
   
}

void US_RotorGui::selectCalibration( QListWidgetItem *item )
{
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
}

void US_RotorGui::check_db()
{
   QStringList DB = US_Settings::defaultDB();

   if ( DB.size() < 5 )
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "There is no default database set." ) );
   }

/*
   // Clear out solution list
   solutionMap.clear();
   lw_solutions->clear();

   reset();
*/
}

void US_RotorGui::check_disk( void )
{
/*
   // Clear out solution list
   solutionMap.clear();
   lw_solutions->clear();

   reset();
*/
}

void US_RotorGui::accept( void )
{
}

// Function to change the current lab
void US_RotorGui::changeLab( int id )
{
   labID = getIndex(cb_lab->itemText(id));
   loadRotors(labID);
}

bool US_RotorGui::loadLabs( void )
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

   int k=0;
   QString str1, str2, str3;
   cb_lab->clear();
   while ( db.next() )
   {
      str1 = db.value( 0 ).toString();
      str2 = db.value( 1 ).toString();
      str3 = str1 + ": " + str2;
      cb_lab->insertItem(k, str3);
      k++;
   }
   return( true );
}

bool US_RotorGui::loadRotors( const int &id )
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

   int k=0;
   QString str1, str2, str3;
   lw_rotors->clear();
   while ( db.next() )
   {
      str1 = db.value( 0 ).toString();
      str2 = db.value( 1 ).toString();
      str3 = str1 + ": " + str2;
      lw_rotors->addItem(str3);
      k++;
   }
   return( true );   
}

void US_RotorGui::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to databasee \n" ) + error );
}

