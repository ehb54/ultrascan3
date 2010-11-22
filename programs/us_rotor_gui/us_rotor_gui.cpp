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

US_AbstractRotorGui::US_AbstractRotorGui( abstractRotor *currentAbstractRotor,
                                          rotor *currentRotor ) : US_WidgetsDialog(0, 0)
{
   this->setModal(true);
   this->currentAbstractRotor = currentAbstractRotor;
   this->currentRotor = currentRotor;
   setupGui();
}


US_AbstractRotorGui::~US_AbstractRotorGui()
{
}

void US_AbstractRotorGui::setupGui()
{
   setWindowTitle( tr( "Rotor Type Selection" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* top = new QGridLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;
   QStringList DB = US_Settings::defaultDB();
   if ( DB.isEmpty() ) DB << "Undefined";

   QLabel* lbl_bannerDB = us_banner( tr( "Current Database: " ) + DB.at( 0 ) );
   lbl_bannerDB->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   top->addWidget( lbl_bannerDB, row, 0, 1, 2 );

   row++;

   QGridLayout *lo_db = us_radiobutton( tr( "Use Database" ), rb_db );
   top->addLayout( lo_db, row, 0, 1, 1);
   connect( rb_db, SIGNAL( clicked() ), this, SLOT( check_db() ) );
   rb_db->setChecked( true );

   QGridLayout *lo_disk = us_radiobutton( tr( "Use Local Disk" ), rb_disk );
   top->addLayout( lo_disk, row, 1, 1, 1 );
   connect( rb_disk, SIGNAL( clicked() ), this, SLOT( check_disk() ) );

   row++;
   
   QLabel* lbl_lab = us_label( tr( " Please select a Rotor Type: " ) );
   top->addWidget( lbl_lab, row, 0, 1, 1 );
   cb_rotors = new QComboBox( this );
   connect( cb_rotors, SIGNAL( activated ( int ) ),      // Only if the user has changed it
                    SLOT  ( changeRotor( int ) ) );
   top->addWidget( cb_rotors, row, 1, 1, 1 );
   if(!loadAbstractRotors())
   {
      qDebug() << "Problem loading Abstract Rotors...";
   }
   rotorID = getIndex(cb_rotors->currentText());
   
   row++;
   QFont currentFont = QFont( "Courier", US_GuiSettings::fontSize()-1, QFont::Bold );


   te_details = new QTextEdit(this);
   te_details->setTextBackgroundColor ( Qt::white );
   te_details->setText(tr("Rotor Type Details:"));
   te_details->setFont          ( currentFont );
   te_details->setPalette       ( US_GuiSettings::editColor() );
   te_details->setAcceptRichText( true );
   te_details->setWordWrapMode  ( QTextOption::WrapAtWordBoundaryOrAnywhere );
   te_details->setReadOnly      ( true );
   te_details->setMinimumSize(80, 200);
   top->addWidget(te_details, row, 0, 1, 2);

   changeRotor(cb_rotors->currentIndex());
   
   row++;

   QLabel *lbl_name = us_label( tr(" Rotor name: "), -1 );
   top->addWidget( lbl_name, row, 0 );

   le_name = us_lineedit( "", -1 );
   le_name->setText( "" );
   top->addWidget( le_name, row, 1);
   connect( le_name, SIGNAL( textEdited  ( const QString & ) ),
            SLOT  ( updateName ( const QString & ) ) );

   row++;

   QLabel *lbl_serialNumber = us_label( tr(" Rotor serial number: "), -1 );
   top->addWidget( lbl_serialNumber, row, 0 );

   le_serialNumber = us_lineedit( "", -1 );
   le_serialNumber->setText( "" );
   top->addWidget( le_serialNumber, row, 1);
   connect( le_serialNumber, SIGNAL( textEdited  ( const QString & ) ),
            SLOT  ( updateSerialNumber ( const QString & ) ) );

   row++;
   
   pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), this, SLOT( help() ) );
   top->addWidget( pb_help, row, 0 );

   pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), this, SLOT( reset() ) );
   top->addWidget( pb_reset, row, 1 );

   row++;

   pb_accept = us_pushbutton( tr( "Select Current Rotor" ) );
   connect( pb_accept, SIGNAL( clicked() ), this, SLOT( select() ) );
   top->addWidget( pb_accept, row, 0 );

   pb_close = us_pushbutton( tr( "Cancel" ) );
   connect( pb_close, SIGNAL( clicked() ), this, SLOT( close() ) );
   top->addWidget( pb_close, row, 1 );
}

void US_AbstractRotorGui::reset( void )
{
}

void US_AbstractRotorGui::check_db( void )
{
   QStringList DB = US_Settings::defaultDB();

   if ( DB.size() < 5 )
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "There is no default database set." ) );
   }
}

void US_AbstractRotorGui::check_disk( void )
{
}

bool US_AbstractRotorGui::loadAbstractRotors( void )
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

   QStringList q( "get_abstractRotor_names" );
   db.query( q );

   int k=0;
   QString str1, str2, str3;
   cb_rotors->clear();
   while ( db.next() )
   {
      str1 = db.value( 0 ).toString();
      str2 = db.value( 1 ).toString();
      str3 = str1 + ": " + str2;
      cb_rotors->addItem(str3);
      k++;
   }
   //load abstract rotor properties for each entry from database:
   abstractRotorList.clear();
   abstractRotor ar;
   ar.ID = 1;
   ar.GUID = "GUID1";
   ar.name = "Simulation";
   ar.material = "Simulation";
   ar.numHoles = 1;
   ar.maxRPM = 100000;
   ar.cellCenter = 6.5;
   ar.manufacturer = "Simulation";
   abstractRotorList.push_back(ar);
   ar.ID = 2;
   ar.GUID = "GUID2";
   ar.name = "AN50";
   ar.material = "Titanium";
   ar.numHoles = 8;
   ar.maxRPM = 50000;
   ar.cellCenter = 6.5;
   ar.manufacturer = "Beckman";
   abstractRotorList.push_back(ar);
   ar.ID = 3;
   ar.GUID = "GUID3";
   ar.name = "AN60";
   ar.material = "Titanium";
   ar.numHoles = 4;
   ar.maxRPM = 60000;
   ar.cellCenter = 6.5;
   ar.manufacturer = "Beckman";
   abstractRotorList.push_back(ar);
   ar.ID = 4;
   ar.GUID = "GUID4";
   ar.name = "CFA";
   ar.material = "CarbonFiber";
   ar.numHoles = 8;
   ar.maxRPM = 60000;
   ar.cellCenter = 6.5;
   ar.manufacturer = "SpinAnalytical";
   abstractRotorList.push_back(ar);
   return( true );   
}

int US_AbstractRotorGui::getIndex( const QString &str )
{
   int pos = str.indexOf(":");
   QString str1 = str.left(pos);
   return str1.toInt();
}

void US_AbstractRotorGui::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to databasee \n" ) + error );
}

// this function updates the two passed in structures (rotor and abstract rotor) before returning control
// to the calling program. This program does not update the rotor ID and GUID, or lab ID, that needs to be
// done by the calling program.

void US_AbstractRotorGui::select()
{
   setAbstractRotorInfo(cb_rotors->currentIndex());
   if ((*currentRotor).name == "" && (*currentRotor).serialNumber == "" )
   {
      QMessageBox::warning( this, tr( "Please provide the missing information:" ),
                        tr( "You need to provide a rotor name and serial number"));
      return;
   }
   else if ((*currentRotor).name == "" && (*currentRotor).serialNumber != "" )
   {
      QMessageBox::warning( this, tr( "Please provide the missing information:" ),
                        tr( "You need to provide a rotor name"));
      return;
   }
   else if ((*currentRotor).name != "" && (*currentRotor).serialNumber == "" )
   {
      QMessageBox::warning( this, tr( "Please provide the missing information:" ),
                        tr( "You need to provide a rotor serial number"));
      return;
   }
   (*currentRotor).ID = (*currentAbstractRotor).ID;
   (*currentRotor).abstractRotorGUID = (*currentAbstractRotor).GUID;
      
   accept();
}

// Function to change the current rotor
void US_AbstractRotorGui::changeRotor( int id )
{
   rotorID = getIndex(cb_rotors->itemText(id));
   QString text = tr("Details for the ") + abstractRotorList[id].name + tr(" Rotor Type:\n\n");
   text += "Database ID:     " + QString::number(abstractRotorList[id].ID) + "\n";
   text += "GUID:            " + abstractRotorList[id].GUID + "\n";
   text += "Number of Holes: " + QString::number(abstractRotorList[id].numHoles) + "\n";
   text += "Maximum Speed:   " + QString::number(abstractRotorList[id].maxRPM) + " rpm\n";
   text += "Cell Center:     " + QString::number(abstractRotorList[id].cellCenter) + " cm\n";
   text += "Material:        " + abstractRotorList[id].material + "\n";
   text += "Manufacturer:    " + abstractRotorList[id].manufacturer;
   te_details->clear();
   te_details->setText(text);
}

void US_AbstractRotorGui::setAbstractRotorInfo( int id )
{
   (*currentAbstractRotor).ID              = abstractRotorList[id].ID;
   (*currentAbstractRotor).GUID            = abstractRotorList[id].GUID;
   (*currentAbstractRotor).name            = abstractRotorList[id].name;
   (*currentAbstractRotor).material        = abstractRotorList[id].material;
   (*currentAbstractRotor).numHoles        = abstractRotorList[id].numHoles;
   (*currentAbstractRotor).maxRPM          = abstractRotorList[id].maxRPM;
   (*currentAbstractRotor).cellCenter      = abstractRotorList[id].cellCenter;
   (*currentAbstractRotor).manufacturer    = abstractRotorList[id].manufacturer;
}

void US_AbstractRotorGui::updateName( const QString &name)
{
   (*currentRotor).name = name;
}

void US_AbstractRotorGui::updateSerialNumber( const QString &number)
{
   (*currentRotor).serialNumber = number;
}


/*

// Function to load a solution from the disk
void US_RotorGui::readFromDisk( QString& guid )
{
   QString filename;
   bool found = diskFilename( guid, filename );

   if ( ! found )
   {
      qDebug() << "Error: file not found for guid "
               << guid;
      return;
   }

   QFile file( filename );
   if ( !file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      qDebug() << "Error: can't open file for reading"
               << filename;
      return;
   }

   QXmlStreamReader xml( &file );

   clear();

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "solution" )
         {
            QXmlStreamAttributes a = xml.attributes();
            solutionID   = a.value( "id" ).toString().toInt();
            solutionGUID = a.value( "guid" ).toString();
            commonVbar20 = a.value( "commonVbar20" ).toString().toDouble();
            storageTemp  = a.value( "storageTemp" ).toString().toDouble();

            readSolutionInfo( xml );
         }
      }
   }

   file.close();

   if ( xml.hasError() )
   {
      qDebug() << "Error: xml error: \n"
               << xml.errorString();
      return;
   }

}

void US_RotorGui::readSolutionInfo( QXmlStreamReader& xml )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "solution" ) return;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "description" )
         {
            xml.readNext();
            solutionDesc = xml.text().toString();
         }

         else if ( xml.name() == "notes" )
         {
            xml.readNext();
            notes = xml.text().toString();
         }

         else if ( xml.name() == "buffer" )
         {
            QXmlStreamAttributes a = xml.attributes();
            bufferID        = a.value( "id" )  .toString().toInt();
            bufferGUID      = a.value( "guid" ).toString();
            bufferDesc      = a.value( "desc" ).toString();
         }

         else if ( xml.name() == "analyte" )
         {
            AnalyteInfo analyte;

            QXmlStreamAttributes a = xml.attributes();
            analyte.analyteID       = a.value( "id" ).toString().toInt();
            analyte.analyteGUID     = a.value( "guid" ).toString();
            analyte.analyteDesc     = a.value( "desc" ).toString();
            analyte.amount          = a.value( "amount" ).toString().toDouble();
            analyte.mw              = a.value( "mw" ).toString().toDouble();
            analyte.vbar20          = a.value( "vbar20" ).toString().toDouble();

            analytes << analyte;
         }
      }
   }
}

// Function to load a solution from the db
void US_RotorGui::readFromDB  ( int solutionID, US_DB2* db )
{
   // Try to get solution info
   QStringList q( "get_solution" );
   q  << QString::number( solutionID );
   db->query( q );

   if ( db->next() )
   {
      this->solutionID = solutionID;
      solutionGUID     = db->value( 0 ).toString();
      solutionDesc     = db->value( 1 ).toString();
      commonVbar20     = db->value( 2 ).toDouble();
      storageTemp      = db->value( 3 ).toDouble();
      notes            = db->value( 4 ).toString();

      q.clear();
      q  << "get_solutionBuffer"
         << QString::number( solutionID );
      db->query( q );
      if ( db->next() )
      {
         bufferID   = db->value( 0 ).toInt();
         bufferGUID = db->value( 1 ).toString();
         bufferDesc = db->value( 2 ).toString();
      }

      q.clear();
      q  << "get_solutionAnalyte"
         << QString::number( solutionID );
      db->query( q );
      while ( db->next() )
      {
         AnalyteInfo analyte;

         analyte.analyteID   = db->value( 0 ).toInt();
         analyte.analyteGUID = db->value( 1 ).toString();
         analyte.analyteDesc = db->value( 2 ).toString();
         analyte.amount      = db->value( 3 ).toDouble();
         analyte.mw          = db->value( 4 ).toDouble();
         analyte.vbar20      = db->value( 5 ).toDouble();

         analytes << analyte;
      }
   }
}

// Function to save solution information to disk
void US_RotorGui::saveToDisk( void )
{
   // First make sure we have a GUID
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );

   if ( ! rx.exactMatch( solutionGUID ) )
      solutionGUID = US_Util::new_guid();

   // Get a path and file name for solution
   QString path;
   if ( ! diskPath( path ) ) return;

   bool    newFile;
   QString filename = get_filename(
                         path,
                         newFile );

   QFile file( filename );
   if ( !file.open( QIODevice::WriteOnly | QIODevice::Text) )
   {
      qDebug() << "Error: can't open file for writing"
               << filename;
      return;
   }

   // Generate xml
   QXmlStreamWriter xml;
   xml.setDevice( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD("<!DOCTYPE US_RotorGui>");
   xml.writeStartElement("SolutionData");
   xml.writeAttribute("version", "1.0");

   // elements
   xml.writeStartElement( "solution" );
   xml.writeAttribute   ( "id", QString::number( solutionID ) );
   xml.writeAttribute   ( "guid", solutionGUID );
   xml.writeAttribute   ( "commonVbar20", QString::number( commonVbar20 ) );
   xml.writeAttribute   ( "storageTemp", QString::number( storageTemp ) );
   xml.writeTextElement ( "description", solutionDesc );
   xml.writeTextElement ( "notes", notes );

      xml.writeStartElement( "buffer" );
      xml.writeAttribute   ( "id",   QString::number( bufferID ) );
      xml.writeAttribute   ( "guid", bufferGUID );
      xml.writeAttribute   ( "desc", bufferDesc );
      xml.writeEndElement  ();

      // Loop through all the analytes
      for ( int i = 0; i < analytes.size(); i++ )
      {
         AnalyteInfo analyte = analytes[ i ];

         xml.writeStartElement( "analyte" );
         xml.writeAttribute   ( "id",   QString::number( analyte.analyteID ) );
         xml.writeAttribute   ( "guid", analyte.analyteGUID );
         xml.writeAttribute   ( "desc", analyte.analyteDesc );
         xml.writeAttribute   ( "amount", QString::number( analyte.amount  ) );
         xml.writeAttribute   ( "mw", QString::number( analyte.mw  ) );
         xml.writeAttribute   ( "vbar20", QString::number( analyte.vbar20  ) );
         xml.writeEndElement  ();
      }

   xml.writeEndElement  ();        // solution
   xml.writeEndDocument ();

   file.close();
}

// Function to save solution information to db
void US_RotorGui::saveToDB( int expID, int channelID, US_DB2* db )
{
   // First make sure we have a GUID
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );

   if ( ! rx.exactMatch( solutionGUID ) )
      solutionGUID = US_Util::new_guid();

   // Check for GUID in database
   QStringList q( "get_solutionID_from_GUID" );
   q << solutionGUID;
   db->query( q );

   solutionID = 0;
   int status = db->lastErrno();
   if ( status == US_DB2::OK && db->next() )
   {
      // Edit existing solution entry
      solutionID = db->value( 0 ).toInt();
      q.clear();
      q  << "update_solution"
         << QString::number( solutionID )
         << solutionGUID
         << solutionDesc
         << QString::number( commonVbar20 )
         << QString::number( storageTemp )
         << notes;

      status = db->statusQuery( q );
   }

   else if ( status == US_DB2::NOROWS )
   {
      // Create new solution entry
      q.clear();
      q  << "new_solution"
         << solutionGUID
         << solutionDesc
         << QString::number( commonVbar20 )
         << QString::number( storageTemp )
         << notes
         << QString::number( expID )
         << QString::number( channelID );

      status = db->statusQuery( q );
      solutionID = db->lastInsertID();
   }

   else   // unspecified error
   {
      qDebug() << "MySQL error: " << db->lastError();
      return;
   }

   // Associate the existing buffer with this solution
   q.clear();
   q  << "new_solutionBuffer"
      << QString::number( solutionID )
      << QString( "" )           // skip bufferID and use GUID instead
      << bufferGUID;

   status = db->statusQuery( q );
   if ( status != US_DB2::OK )
      qDebug() << "MySQL error associating buffer with solution in database: "
               << db->lastError();

   // Remove analyte associations; we'll create new ones
   q.clear();
   q  << "delete_solutionAnalytes"
      << QString::number( solutionID );

   status = db->statusQuery( q );
   if ( status != US_DB2::OK )
      qDebug() << "MySQL error: " << db->lastError();

   // Now add one or more analyte associations
   foreach( AnalyteInfo analyte, analytes )
   {
      q.clear();
      q  << "new_solutionAnalyte"
         << QString::number( solutionID )
         << QString( "" )          // skip analyteID and use GUID instead
         << analyte.analyteGUID
         << QString::number( analyte.amount );

      status = db->statusQuery( q );
      if ( status != US_DB2::OK )
         qDebug() << "MySQL error associating analyte with solution in database: "
                  << db->lastError();
   }
}

// Function to delete a solution from disk
void US_RotorGui::deleteFromDisk( void )
{
   QString filename;
   bool found = diskFilename( solutionGUID, filename );

   if ( ! found )
   {
      // No file to delete
      return;
   }

   // Delete it
   QFile file( filename );
   if ( file.exists() )
      file.remove();

}

// Function to delete a solution from db
void US_RotorGui::deleteFromDB( US_DB2* db )
{
   QStringList q;
   if ( solutionID == 0 )
   {
      // Then probably user selected from disk but is trying to delete from db,
      // so let's try to get the db solutionID from the GUID
      q.clear();
      q << "get_solutionID_from_GUID"
        << solutionGUID;
      db->query( q );

      if ( db->next() )
         solutionID = db->value( 0 ).toInt();

    }

   if ( solutionID > 0 ) // otherwise it's not there to delete
   {
      q.clear();
      q << "delete_solution"
        << QString::number( solutionID );

      int status = db->statusQuery( q );
      if ( status != US_DB2::OK )
         qDebug() << "MySQL error: " << db->lastError();

   }

   clear();
}

// Function to find the file name of a solution on disk, if it exists
bool US_RotorGui::diskFilename( const QString& guid, QString& filename )
{
   // Get a path and file name for solution
   QString path;
   if ( ! diskPath( path ) )
   {
      qDebug() << "Error: could not create the solutions directory";
      return false;
   }

   QDir        dir( path );
   QStringList filter( "S*.xml" );
   QStringList names = dir.entryList( filter, QDir::Files, QDir::Name );
   bool        found = false;

   for ( int i = 0; i < names.size(); i++ )
   {
      filename = path + "/" + names[ i ];
      QFile file( filename );

      if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "solution" )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "guid" ).toString() == guid ) found = true;
               break;
            }
         }
      }

      file.close();
      if ( found ) break;  // Break out of this loop too
   }

   if ( ! found )
   {
      filename = QString( "" );
      return false;
   }

   return true;
}

// Get the path to the solutions.  Create it if necessary.
bool US_RotorGui::diskPath( QString& path )
{
   QDir dir;
   path = US_Settings::dataDir() + "/solutions";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         qDebug() << "Error: Could not create default directory for solutions\n"
                  << path;
         return false;
      }
   }

   return true;
}

// Function to check if filename already exists, and perhaps generate a new one
QString US_RotorGui::get_filename(
      const QString& path, bool& newFile )
{
   QDir        f( path );
   QStringList filter( "S???????.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );
   QString     filename;
   newFile = true;

   for ( int i = 0; i < f_names.size(); i++ )
   {
      QFile b_file( path + "/" + f_names[ i ] );

      if ( ! b_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &b_file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "solution" )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "guid" ).toString() == solutionGUID )
               {
                  newFile  = false;
                  filename = path + "/" + f_names[ i ];
               }

               break;
            }
         }
      }

      b_file.close();
      if ( ! newFile ) return filename;
   }

   // If we get here, generate a new filename
   int number = ( f_names.size() > 0 ) ? f_names.last().mid( 1, 7 ).toInt() : 0;

   return path + "/S" + QString().sprintf( "%07i", number + 1 ) + ".xml";
}

US_RotorGui::AnalyteInfo::AnalyteInfo()
{
  analyteID     = 0;
  analyteGUID   = QString( "" );
  analyteDesc   = QString( "" );
  vbar20        = 0.0;
  mw            = 0.0;
  amount        = 1;
}

bool US_RotorGui::AnalyteInfo::operator== ( const AnalyteInfo& ai ) const
{
   if ( analyteGUID != ai.analyteGUID ) return false;

   return true;
}

void US_RotorGui::clear( void )
{
   solutionID   = 0;
   solutionGUID = QString( "" );
   solutionDesc = QString( "" );
   bufferID     = 0;
   bufferGUID   = QString( "" );
   bufferDesc   = QString( "" );
   commonVbar20 = 0.0;
   storageTemp  = 20.0;
   notes        = QString( "" );
   saveStatus   = NOT_SAVED;
   analytes.clear();
}

void US_RotorGui::show( void )
{
   qDebug() << "solutionID   = " << solutionID   << '\n'
            << "solutionGUID = " << solutionGUID << '\n'
            << "solutionDesc = " << solutionDesc << '\n'
            << "bufferID     = " << bufferID     << '\n'
            << "bufferGUID   = " << bufferGUID   << '\n'
            << "bufferDesc   = " << bufferDesc   << '\n'
            << "commonVbar20 = " << commonVbar20 << '\n'
            << "storageTemp  = " << storageTemp  << '\n'
            << "notes        = " << notes        << '\n'
            << "saveStatus   = " << saveStatus   << '\n';

   qDebug() << "Analytes...";
   foreach( AnalyteInfo analyte, analytes )
   {
      qDebug() << "analyteID   = " << analyte.analyteID   << '\n'
               << "analyteGUID = " << analyte.analyteGUID << '\n'
               << "analyteDesc = " << analyte.analyteDesc << '\n'
               << "vbar20      = " << analyte.vbar20      << '\n'
               << "mw          = " << analyte.mw          << '\n'
               << "amount      = " << analyte.amount      << '\n';
   }
}

*/

