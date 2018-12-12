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


US_NewXpnHostDB::US_NewXpnHostDB( QWidget* w, Qt::WindowFlags flags )
  : US_Widgets( true, w, flags )
{
  
   // Frame layout
   setPalette( US_GuiSettings::frameColor() );

   setWindowTitle( "Optima DB Host Configuration" );
   setAttribute( Qt::WA_DeleteOnClose );

   use_db              = ( US_Settings::default_data_location() < 2 );

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

   topbox->addLayout( details );

   //Pushbuttons
   row = 0;
   QGridLayout* buttons = new QGridLayout();

   pb_save = us_pushbutton( tr( "Save Entry" ) );
   pb_save->setEnabled( true );
   connect( pb_save, SIGNAL( clicked() ), this, SLOT( save_new() ) );
   buttons->addWidget( pb_save, row, 0 );

   pb_cancel = us_pushbutton( tr( "Cancel" ) );
   pb_cancel->setEnabled( true );
   connect( pb_cancel,      SIGNAL( clicked()  ),
	    this,           SLOT  ( cancel() ) );
   buttons->addWidget( pb_cancel, row++, 1 );
   
   topbox->addLayout( buttons );
   adjustSize();
}

// read Instrument names/info from DB
void US_NewXpnHostDB::save_new( void )
{
  if ( le_name->text().isEmpty() || le_description->text().isEmpty()
       || le_host->text().isEmpty() || le_port->text().isEmpty()
       || le_user->text().isEmpty() )
    {
      QMessageBox::warning( this, tr( "Please provide the missing information:" ),
                        tr( "Fill out all fields!"));
      return;
    }

  // Check for instrument name duplication:
  US_Passwd pw;
  US_DB2* db = use_db ? new US_DB2( pw.getPasswd() ) : NULL;
  QStringList instrumentNames( "" );
  QStringList q( "" );
  q.clear();
  q  << QString( "get_instrument_names" )
     << QString::number( 1 );                    //ALEXEY '1' for the (only) labID

  db->query( q );
  
  while ( db->next() )
    {
      QString name = db->value( 1 ).toString();
      instrumentNames << name;
    }

  for (QStringList::iterator it = instrumentNames.begin(); it != instrumentNames.end(); ++it) 
    {
      QString current = *it;
      if ( current == le_description->text() )
	{
	  QMessageBox::critical( this, tr( "Duplicate Optima Machine Name:" ),
				QString( tr( "The name selected (%1) is currently used by other machine. Please select different name.").arg(current) ) );
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
  
  emit accepted( newInstrument );

  close();
}

// read Instrument names/info from DB
void US_NewXpnHostDB::cancel( void )
{
  close();
}
