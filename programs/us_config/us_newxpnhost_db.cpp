//! \file us_xpnhost.cpp

#include "us_xpnhost_db.h"
#include "us_newxpnhost_db.h"
#include "us_xpn_data.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_passwd.h"
#include "us_help.h"
#include "us_crypto.h"
#include "us_db2.h"


#define def_desc QString("place-holder")
#define def_host QString("192.168.1.1")
#define def_port QString("5432")
#define def_name QString("AUC_DATA_DB")
#define def_user QString("")
#define def_pasw QString("")


US_NewXpnHostDB::US_NewXpnHostDB() : US_Widgets()
{
  
   // Frame layout
   setPalette( US_GuiSettings::frameColor() );

   setWindowTitle( "New Optima DB Host Configuration" );
   setAttribute( Qt::WA_DeleteOnClose );

   use_db              = ( US_Settings::default_data_location() < 2 );

   update_instrument = false;
   
   QBoxLayout* topbox = new QVBoxLayout( this );
   topbox->setSpacing( 2 );

   QLabel* banner = us_banner( tr( "Enter Info for the New Xpn Host:" ) );
   topbox->addWidget( banner );

      // Row 0
   int row = 0;
   QGridLayout* details = new QGridLayout();

   // Row 1
   QLabel* desc         = us_label( tr( "Optima Host Description:" ) );
   le_description       = us_lineedit( "", 0 );
   le_description->setPlaceholderText("SYNTAX: 'Optima #'");
   details->addWidget( desc,           row,   0, 1, 2 );
   details->addWidget( le_description, row++, 2, 1, 2 );

   // Row 1a
   QLabel* serialNum    = us_label( tr( "Optima Serial Number:" ) );
   le_serialNumber      = us_lineedit( "", 0 );
   details->addWidget( serialNum,       row,   0, 1, 2 );
   details->addWidget( le_serialNumber, row++, 2, 1, 2 );   

   // Row 2
   QLabel* host        = us_label( tr( "Optima DB Host Address:" ) );
   le_host             = us_lineedit( "", 0 );
   details->addWidget( host,           row,   0, 1, 2 );
   details->addWidget( le_host,        row++, 2, 1, 2 );

   // Row 3
   QLabel* port        = us_label( tr( "Optima DB Port:" ) );
   le_port             = us_lineedit( def_port, 0 );
   details->addWidget( port,           row,   0, 1, 2 );
   details->addWidget( le_port,        row++, 2, 1, 2 );

   // Row 4
   QLabel* name        = us_label( tr( "Optima DB Name:" ) );
   le_name             = us_lineedit( def_name, 0 );
   details->addWidget( name,           row,   0, 1, 2 );
   details->addWidget( le_name,        row++, 2, 1, 2 );

   // Row 5
   QLabel* user        = us_label( tr( "DB Username:" ) );
   le_user             = us_lineedit( def_user, 0 );
   details->addWidget( user,           row,   0, 1, 2 );
   details->addWidget( le_user,        row++, 2, 1, 2 );

   // Row 6
   QLabel* pasw        = us_label( tr( "DB Password:" ) );
   le_pasw             = us_lineedit( def_pasw, 0 );
   le_pasw->setEchoMode( QLineEdit::Password );
   details->addWidget( pasw,           row,   0, 1, 2 );
   details->addWidget( le_pasw,        row++, 2, 1, 2 );


   // Row 7
   QLabel* bn_optsys   = us_banner( tr( "Installed Optical Systems" ) );
   details->addWidget( bn_optsys,      row++, 0, 1, 4 );

   // Rows 8,9,10
   QStringList osyss;
   osyss << "UV/visible"
         << "Rayleigh Interference"
         << "Fluorescense"
         << "(not installed)";
   QLabel* lb_os1      = us_label( tr( "Op Sys1:" ) );
   QLabel* lb_os2      = us_label( tr( "Op Sys2:" ) );
   QLabel* lb_os3      = us_label( tr( "Op Sys3:" ) );
   cb_os1              = us_comboBox();
   cb_os2              = us_comboBox();
   cb_os3              = us_comboBox();
   cb_os1->addItems( osyss );
   cb_os2->addItems( osyss );
   cb_os3->addItems( osyss );
   details->addWidget( lb_os1,         row,   0, 1, 1 );
   details->addWidget( cb_os1,         row++, 1, 1, 3 );
   details->addWidget( lb_os2,         row,   0, 1, 1 );
   details->addWidget( cb_os2,         row++, 1, 1, 3 );
   details->addWidget( lb_os3,         row,   0, 1, 1 );
   details->addWidget( cb_os3,         row++, 1, 1, 3 );
   cb_os1->setCurrentIndex( 0 );
   cb_os2->setCurrentIndex( 1 );
   cb_os3->setCurrentIndex( 3 );
      
   topbox->addLayout( details );

   //Pushbuttons
   row = 0;
   QGridLayout* buttons = new QGridLayout();

   pb_save = us_pushbutton( tr( "Save Entry" ) );
   pb_save->setEnabled( true );
   connect( pb_save, SIGNAL( clicked( ) ), this, SLOT( save_new( ) ) );
   buttons->addWidget( pb_save, row, 0 );

   pb_cancel = us_pushbutton( tr( "Cancel" ) );
   pb_cancel->setEnabled( true );
   connect( pb_cancel,      SIGNAL( clicked()  ),
	    this,           SLOT  ( cancel() ) );
   buttons->addWidget( pb_cancel, row++, 1 );
   
   topbox->addLayout( buttons );

   setMinimumSize( 450 , 400 );
   adjustSize();
}


US_NewXpnHostDB::US_NewXpnHostDB( QMap <QString,QString> currentInstrument ) : US_Widgets()
{

   this->instrumentedit = currentInstrument;
   // Frame layout
   setPalette( US_GuiSettings::frameColor() );

   setWindowTitle( "Modify Optima DB Host Configuration" );
   setAttribute( Qt::WA_DeleteOnClose );

   use_db              = ( US_Settings::default_data_location() < 2 );
   update_instrument = true;
   
   QBoxLayout* topbox = new QVBoxLayout( this );
   topbox->setSpacing( 2 );

   QLabel* banner = us_banner( tr( "Change Xpn Host Configuration:" ) );
   topbox->addWidget( banner );

      // Row 0
   int row = 0;
   QGridLayout* details = new QGridLayout();

   // Row 1
   QLabel* desc         = us_label( tr( "Optima Host Description:" ) );
   le_description       = us_lineedit( "", 0, true );
   details->addWidget( desc,           row,   0, 1, 2 );
   details->addWidget( le_description, row++, 2, 1, 2 );

   // Row 1a
   QLabel* serialNum    = us_label( tr( "Optima Serial Number:" ) );
   le_serialNumber      = us_lineedit( "", 0 );
   details->addWidget( serialNum,       row,   0, 1, 2 );
   details->addWidget( le_serialNumber, row++, 2, 1, 2 );   

   // Row 2
   QLabel* host        = us_label( tr( "Optima DB Host Address:" ) );
   le_host             = us_lineedit( "", 0 );
   details->addWidget( host,           row,   0, 1, 2 );
   details->addWidget( le_host,        row++, 2, 1, 2 );

   // Row 3
   QLabel* port        = us_label( tr( "Optima DB Port:" ) );
   le_port             = us_lineedit( def_port, 0 );
   details->addWidget( port,           row,   0, 1, 2 );
   details->addWidget( le_port,        row++, 2, 1, 2 );

   // Row 4
   QLabel* name        = us_label( tr( "Optima DB Name:" ) );
   le_name             = us_lineedit( def_name, 0 );
   details->addWidget( name,           row,   0, 1, 2 );
   details->addWidget( le_name,        row++, 2, 1, 2 );

   // Row 5
   QLabel* user        = us_label( tr( "DB Username:" ) );
   le_user             = us_lineedit( def_user, 0 );
   details->addWidget( user,           row,   0, 1, 2 );
   details->addWidget( le_user,        row++, 2, 1, 2 );

   // Row 6
   QLabel* pasw        = us_label( tr( "DB Password:" ) );
   le_pasw             = us_lineedit( def_pasw, 0 );
   le_pasw->setEchoMode( QLineEdit::Password );
   details->addWidget( pasw,           row,   0, 1, 2 );
   details->addWidget( le_pasw,        row++, 2, 1, 2 );

   // Row 7
   QLabel* bn_optsys   = us_banner( tr( "Installed Optical Systems" ) );
   details->addWidget( bn_optsys,      row++, 0, 1, 4 );

   // Rows 8,9,10
   QStringList osyss;
   osyss << "UV/visible"
         << "Rayleigh Interference"
         << "Fluorescense"
         << "(not installed)";
   QLabel* lb_os1      = us_label( tr( "Op Sys1:" ) );
   QLabel* lb_os2      = us_label( tr( "Op Sys2:" ) );
   QLabel* lb_os3      = us_label( tr( "Op Sys3:" ) );
   cb_os1              = us_comboBox();
   cb_os2              = us_comboBox();
   cb_os3              = us_comboBox();
   cb_os1->addItems( osyss );
   cb_os2->addItems( osyss );
   cb_os3->addItems( osyss );
   details->addWidget( lb_os1,         row,   0, 1, 1 );
   details->addWidget( cb_os1,         row++, 1, 1, 3 );
   details->addWidget( lb_os2,         row,   0, 1, 1 );
   details->addWidget( cb_os2,         row++, 1, 1, 3 );
   details->addWidget( lb_os3,         row,   0, 1, 1 );
   details->addWidget( cb_os3,         row++, 1, 1, 3 );
   cb_os1->setCurrentIndex( 0 );
   cb_os2->setCurrentIndex( 1 );
   cb_os3->setCurrentIndex( 3 );
      
   topbox->addLayout( details );

   //Pushbuttons
   row = 0;
   QGridLayout* buttons = new QGridLayout();

   pb_save = us_pushbutton( tr( "Save Entry" ) );
   pb_save->setEnabled( true );
   connect( pb_save, SIGNAL( clicked( ) ), this, SLOT( save_new( ) ) );
   buttons->addWidget( pb_save, row, 0 );

   pb_cancel = us_pushbutton( tr( "Cancel" ) );
   pb_cancel->setEnabled( true );
   connect( pb_cancel,      SIGNAL( clicked()  ),
	    this,           SLOT  ( cancel() ) );
   buttons->addWidget( pb_cancel, row++, 1 );
   
   topbox->addLayout( buttons );

   setMinimumSize( 450 , 400 );
   adjustSize();

   fillGui();
}

// Fill available GUI elements
void US_NewXpnHostDB::fillGui( void )
{
  if ( !instrumentedit["name"].isEmpty()  )
    le_description->setText( instrumentedit["name"] );
  
  if ( !instrumentedit["serial"].isEmpty()  )
    le_serialNumber->setText( instrumentedit["serial"] );
  
  if ( !instrumentedit["host"].isEmpty()  )
    le_host->setText( instrumentedit["host"] );

  if ( !instrumentedit["port"].isEmpty()  )
    le_port->setText( instrumentedit["port"] );

  if ( !instrumentedit["dbusername"].isEmpty()  )
    le_user->setText( instrumentedit["dbusername"] );

  if ( !instrumentedit["dbname"].isEmpty()  )
    le_name->setText( instrumentedit["dbname"] );

  if ( !instrumentedit["dbpassw"].isEmpty()  )
    le_pasw->setText( instrumentedit["dbpassw"] );

  
  cb_os1->setCurrentIndex( cb_os1->findText( instrumentedit[ "os1" ] ) );
  cb_os2->setCurrentIndex( cb_os2->findText( instrumentedit[ "os2" ] ) );
  cb_os3->setCurrentIndex( cb_os3->findText( instrumentedit[ "os3" ] ) );
}



// read Instrument names/info from DB
void US_NewXpnHostDB::save_new( void )
{
  if ( le_name->text().isEmpty() || le_description->text().isEmpty()
       || le_host->text().isEmpty() || le_port->text().isEmpty()
       || le_user->text().isEmpty() || le_pasw->text().isEmpty() || le_serialNumber->text().isEmpty() )
    {
      QMessageBox::warning( this, tr( "Please provide the missing information:" ),
                        tr( "Fill out all fields!"));
      return;
    }

  // Check for instrument name, port & host duplications:
  US_Passwd pw;
  US_DB2* db = use_db ? new US_DB2( pw.getPasswd() ) : NULL;
  QStringList instrumentNames( "" );
  QStringList q( "" );
  QList< int > instrumentIDs;
  q.clear();
  q  << QString( "get_instrument_names" )
     << QString::number( 1 );                    //ALEXEY '1' for the (only) labID

  db->query( q );
  
  while ( db->next() )
    {
      QString name = db->value( 1 ).toString();
      instrumentNames << name;
      int ID = db->value( 0 ).toString().toInt();
      instrumentIDs << ID;
    }

  // Name check
  qDebug() << "Update Instruemnt: " << update_instrument;
  for (QStringList::iterator it = instrumentNames.begin(); it != instrumentNames.end(); ++it) 
    {
      QString current = *it;

      if ( current == le_description->text() && !update_instrument )
	{
	  QMessageBox::critical( this, tr( "Duplicate Optima Machine Name:" ),
				QString( tr( "The name selected (%1) is currently used by other machine. Please select different name.").arg(current) ) );
	  return;
	}
    }

  // Host & Port check
  foreach ( int ID, instrumentIDs )
    {
      if ( ID == instrumentedit["ID"].toInt() && update_instrument )
	continue;
      
      q.clear();
      q  << QString( "get_instrument_info_new" )
	 << QString::number( ID );
      db->query( q );
      db->next();

      QString optimaHost       = db->value( 5 ).toString();
      int     optimaPort       = db->value( 6 ).toString().toInt();

      if ( optimaHost == le_host->text() && optimaPort == le_port->text().toInt() )
	{
	  QMessageBox::critical( this, tr( "Duplicate Optima Machine Connection Info:" ),
				 QString( tr( "Specified combination of the host (%1) and port (%2) is currently used by other machine!") 
					  .arg(optimaHost).arg(QString::number(optimaPort)) ) );
	  return;
	}
    }

  
  QMap <QString, QString> newInstrument;
  newInstrument[ "name"]        = le_description->text();
  newInstrument[ "optimaHost" ] = le_host->text();
  newInstrument[ "optimaPort" ] = le_port->text();
  newInstrument[ "optimaDBname" ]  = le_name->text();
  newInstrument[ "optimaDBusername" ] = le_user->text();
  newInstrument[ "optimaDBpassw" ] = le_pasw->text();

  newInstrument[ "serialNumber"] = le_serialNumber->text();
  newInstrument[ "labID"] = QString::number(1);

  newInstrument[ "os1" ] = cb_os1->currentText();
  newInstrument[ "os2" ] = cb_os2->currentText();
  newInstrument[ "os3" ] = cb_os3->currentText();
  
  emit accepted( newInstrument );

  close();
}

// read Instrument names/info from DB
void US_NewXpnHostDB::cancel( void )
{
  close();
}
