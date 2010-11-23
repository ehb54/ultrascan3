//! \file us_abstractrotor_gui.cpp

#include "us_abstractrotor_gui.h"
#include "us_settings.h"
#include "us_license_t.h"
#include "us_util.h"
#include "us_db2.h"
#include "us_license.h"
#include "us_gui_settings.h"
#include "us_passwd.h"

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
   QString str1, str2, str3;
   cb_rotors->clear();
   int k=0;
   while ( db.next() )
   {
      str1 = db.value( 0 ).toString();
      str2 = db.value( 1 ).toString();
      str3 = str1 + ": " + str2;
      cb_rotors->addItem(str3);
      k++;
   }
   abstractRotorList.clear();
   abstractRotor ar;
   for (int i=0; i<k; i++)
   {
      q.clear();
      ar.ID = getIndex(cb_rotors->itemText(i));
      q << "get_abstractRotor_info" << QString::number(ar.ID);
      db.query( q );
      db.next();
      ar.GUID           = db.value(0).toString();
      ar.name           = db.value(1).toString();
      ar.material       = db.value(2).toString();
      ar.numHoles       = db.value(3).toInt();
      ar.maxRPM         = db.value(4).toInt();
      ar.magnetOffset   = db.value(5).toFloat();
      ar.cellCenter     = db.value(6).toFloat();
      ar.manufacturer   = db.value(7).toString();
      abstractRotorList.push_back(ar);
   }
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
   text += "Magnet Offset:   " + QString::number(abstractRotorList[id].magnetOffset) + " degrees\n";
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
