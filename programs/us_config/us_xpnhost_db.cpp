//! \file us_xpnhost.cpp
#include "us_xpnhost_db.h"
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

US_XpnHostDB::US_XpnHostDB( QWidget* w, Qt::WindowFlags flags )
  : US_Widgets( true, w, flags )
{
   // Frame layout
   setPalette( US_GuiSettings::frameColor() );

   setWindowTitle( "Instrument Configuration" );
   setAttribute( Qt::WA_DeleteOnClose );

   use_db              = ( US_Settings::default_data_location() < 2 );
   
   QByteArray currentHash = US_Settings::UltraScanPW();

   if ( currentHash.isEmpty() )
   {
       QMessageBox::information( this,
          tr( "No master password" ),
          tr( "The Master Password has not been set." ) );
       close();
   }

   QBoxLayout* topbox = new QVBoxLayout( this );
   topbox->setSpacing( 2 );

   // Set up the database list window
   QLabel* banner = us_banner( tr( "Instrument List" ) );
   topbox->addWidget( banner );

   // QLabel* banner2 = us_banner(
   //      tr( "(Doubleclick for details and set the default)" ), -1 );
   // topbox->addWidget( banner2 );

   lw_entries = new QListWidget();
   lw_entries->setSortingEnabled( true );
   lw_entries->setPalette( US_GuiSettings::editColor() );
   lw_entries->setFont( QFont( US_GuiSettings::fontFamily(),
                              US_GuiSettings::fontSize() ) );
   topbox->addWidget( lw_entries );


  
   // QStringList XpnHosts = US_Settings::defaultXpnHost();
   // QString     defXpnHost;
   // if ( XpnHosts.size() > 0 )
   //    defXpnHost = US_Settings::defaultXpnHost().at( 0 );
   // update_lw( defXpnHost );



   // Detail info
   QLabel* info = us_banner( tr( "Optima Host Connection Information" ) );
   topbox->addWidget( info );

   // Row 0
   int row = 0;
   QGridLayout* details = new QGridLayout();

   // Row 1
   QLabel* desc         = us_label( tr( "Instrument Description:" ) );
   le_description       = us_lineedit( "", 0, true );
   details->addWidget( desc,           row,   0, 1, 2 );
   details->addWidget( le_description, row++, 2, 1, 2 );

   // Row 1a
   QLabel* serial       = us_label( tr( "Instrument Serial Number:" ) );
   le_serial            = us_lineedit( "", 0, true );
   details->addWidget( serial,           row,   0, 1, 2 );
   details->addWidget( le_serial, row++, 2, 1, 2 );

   // Row 2
   host        = us_label( tr( "Instrument DB Host Address:" ) );
   le_host             = us_lineedit( "", 0, true );
   details->addWidget( host,           row,   0, 1, 2 );
   details->addWidget( le_host,        row++, 2, 1, 2 );

   // Row 3
   port        = us_label( tr( "Instrument DB Port:" ) );
   //le_port             = us_lineedit( def_port, 0, true );
   le_port             = us_lineedit( "", 0, true );
   details->addWidget( port,           row,   0, 1, 2 );
   details->addWidget( le_port,        row++, 2, 1, 2 );

   // Row 3a
   msgPort        = us_label( tr( "Instrument Status Msg Port:" ) );
   //le_port             = us_lineedit( def_port, 0, true );
   le_msgPort             = us_lineedit( "", 0, true );
   details->addWidget( msgPort,           row,   0, 1, 2 );
   details->addWidget( le_msgPort,        row++, 2, 1, 2 );

   // Row 4
   name        = us_label( tr( "Instrument DB Name:" ) );
   //le_name             = us_lineedit( def_name, 0, true );
   le_name             = us_lineedit( "", 0, true );
   details->addWidget( name,           row,   0, 1, 2 );
   details->addWidget( le_name,        row++, 2, 1, 2 );

   // Row 5
   user        = us_label( tr( "Instrument DB Username:" ) );
   //le_user             = us_lineedit( def_user, 0, true );
   le_user             = us_lineedit( "", 0, true );
   details->addWidget( user,           row,   0, 1, 2 );
   details->addWidget( le_user,        row++, 2, 1, 2 );

   // Row 6
   pasw        = us_label( tr( "Instrument DB Password:" ) );
   le_pasw             = us_lineedit( def_pasw, 0, true );
   le_pasw->setEchoMode( QLineEdit::Password );
   details->addWidget( pasw,           row,   0, 1, 2 );
   details->addWidget( le_pasw,        row++, 2, 1, 2 );

   //Row 6a
   bn_chromoab   = us_banner( tr( "Chromatic Aberration Information" ) );
   details->addWidget( bn_chromoab,      row++, 0, 1, 4 );

   //Row 6b
   lb_radcalwvl  = us_label( tr( "Radial Calibration Wavelength:" ) );
   le_radcalwvl          = us_lineedit( "", 0, true );
   details->addWidget( lb_radcalwvl,    row,   0, 1, 2 );
   details->addWidget( le_radcalwvl,    row++, 2, 1, 2 );
   
   //Row 6c
   lb_chromofile  = us_label( tr( "Chromatic Aberration Array:" ) );
   le_chromofile          = us_lineedit( "", 0, true );
   details->addWidget( lb_chromofile,    row,   0, 1, 2 );
   details->addWidget( le_chromofile,    row++, 2, 1, 2 );  

   // Row 7
   QLabel* bn_optsys   = us_banner( tr( "Installed Optical Systems" ) );
   details->addWidget( bn_optsys,      row++, 0, 1, 4 );
      
 
   // Rows 8,9,10
   QLabel* lb_os1      = us_label( tr( "Op Sys1:" ) );
   le_os1              = us_lineedit( "", 0, true );
   details->addWidget( lb_os1,        row,   0, 1, 1 );
   details->addWidget( le_os1,        row++, 1, 1, 3 );
   
   QLabel* lb_os2      = us_label( tr( "Op Sys2:" ) );
   le_os2              = us_lineedit( "", 0, true );
   details->addWidget( lb_os2,        row,   0, 1, 1 );
   details->addWidget( le_os2,        row++, 1, 1, 3 );  

   QLabel* lb_os3      = us_label( tr( "Op Sys3:" ) );
   le_os3              = us_lineedit( "", 0, true );
   details->addWidget( lb_os3,        row,   0, 1, 1 );
   details->addWidget( le_os3,        row++, 1, 1, 3 ); 
   
   topbox->addLayout( details );

   //Pushbuttons
   row = 0;
   QGridLayout* buttons = new QGridLayout();

   pb_add = us_pushbutton( tr( "Add New Entry" ) );
   pb_add->setEnabled( true );
   connect( pb_add, SIGNAL( clicked() ), this, SLOT( add_new() ) );
   buttons->addWidget( pb_add, row, 0 );

   pb_delete = us_pushbutton( tr( "Delete Current Entry" ) );
   pb_delete->setEnabled( false );
   connect( pb_delete,      SIGNAL( clicked()  ),
	    this,           SLOT  ( deleteDB() ) );
   buttons->addWidget( pb_delete, row++, 1 );

   pb_edit = us_pushbutton( tr( "Edit Current Entry" ) );
   pb_edit->setEnabled( false );
   connect( pb_edit,        SIGNAL( clicked()  ),
	    this,           SLOT  ( editDB() ) );
   buttons->addWidget( pb_edit, row, 0 );

   pb_testConnect = us_pushbutton( tr( "Test Connectivity" ) );
   pb_testConnect->setEnabled( false );
   connect( pb_testConnect, SIGNAL( clicked()      ),
            this,           SLOT  ( test_connect() ) );
   buttons->addWidget( pb_testConnect, row++, 1 );

   QHBoxLayout* std_buttons = new QHBoxLayout;
   pb_reset = us_pushbutton( tr( "Reset" ) );
   // connect( pb_reset,       SIGNAL( clicked() ),
   //          this,           SLOT  ( reset()   ) );
   std_buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help,        SIGNAL( clicked() ),
	    this,           SLOT  ( help()    ) );
   std_buttons->addWidget( pb_help );

   QPushButton* pb_cancel = us_pushbutton( tr( "Close" ) );
   connect( pb_cancel,      SIGNAL( clicked() ),
	    this,           SLOT  ( close_program()   ) );
   std_buttons->addWidget( pb_cancel );

   topbox->addLayout( buttons );
   topbox->addLayout( std_buttons );

   adjustSize();
   int lwid             = lw_entries->width();
   int lhgt             = pb_help   ->height() * 6;
   lw_entries->resize( lwid, lhgt );
   adjustSize();


   // Populate instrument list and pull instrument detailed info
   US_Passwd pw;
   US_DB2* dbP = use_db ? new US_DB2( pw.getPasswd() ) : NULL;
   if ( dbP != NULL )    //fromDB
     readInstruments( dbP );

   qDebug() << "Read Instr. and NOT updated_lw() ";
   
   if ( instruments.size() > 0 )
     update_lw( );

   qDebug() << "Read Instr. and updated_lw() ";

   // connect( lw_entries, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
   // 	                SLOT  ( select_db        ( QListWidgetItem* ) ) );

   connect( lw_entries, SIGNAL( itemSelectionChanged() ),
                        SLOT  ( optima_selected() ) );

//    // Start out with default entry shown
// qDebug() << "xpnH:Main: call xpn_db_hosts";
//    dblist                = US_Settings::xpn_db_hosts();
// qDebug() << "xpnH:Main:  size" << dblist.size() << "dblist" << dblist;
//    if ( dblist.size() == 0 )
//       update_lw(  tr( "place-holder" ) );
// qDebug() << "xpnH:Main: call select_db";
//    select_db( lw_entries->currentItem(), false );

   update_inv();
   
   setMinimumSize( 450 , 400 );
   adjustSize();
}


void US_XpnHostDB::update_inv( void )
{
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
//qDebug() << "USCFG: UpdInv: ERROR connect";
      QMessageBox::information( this,
         tr( "Error" ),
         tr( "Error making the DB connection.\n" ) );

      return;
   }

   QStringList q( "get_user_info" );
   db.query( q );
   db.next();

   int ID        = db.value( 0 ).toInt();
   QString fname = db.value( 1 ).toString();
   QString lname = db.value( 2 ).toString();
   int     level = db.value( 5 ).toInt();

   qDebug() << "USCFG: UpdInv: ID,name,lev" << ID << fname << lname << level;
//if(ID<1) return;

   US_Settings::set_us_inv_name ( lname + ", " + fname );
   US_Settings::set_us_inv_ID   ( ID );
   US_Settings::set_us_inv_level( level );
}

// read Instrument names/info from DB
void US_XpnHostDB::readInstruments( US_DB2* db )
{
  qDebug() << "Reading Instrument: ";
  instruments.clear();

  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this,
                            tr( "Database Connection Problem!" ),
                            tr( "You are not currently connected to a US3-LIMS DB!\n"
                                "Information on Instruments cannot be displayed! " ) );

      //pb_add->setEnabled( false );
      return;
    }
  
  QStringList q( "" );
  q.clear();
  q  << QString( "get_instrument_names" )
     << QString::number( 1 );                    //ALEXEY '1' for the (only) labID

  db->query( q );
  
  if ( db->lastErrno() == US_DB2::OK )      // If not, no instruments defined
    {
      QList< int > instrumentIDs;
      
      // Grab all the IDs so we can reuse the db connection
      while ( db->next() )
      {
	  int ID = db->value( 0 ).toString().toInt();
	  instrumentIDs << ID;

	  qDebug() << "InstID: " << ID;
      }
      
      // Instrument information
      foreach ( int ID, instrumentIDs )
      {
	  qDebug() << "Inside INSTR. loop!!! ";
	  US_XpnHostDB::Instrument instrument;
	  
	  q.clear();
	  q  << QString( "get_instrument_info_new" )
	     << QString::number( ID );
	  db->query( q );
	  db->next();
	  
	  instrument.ID               = ID;
	  instrument.name             = db->value( 0 ).toString();
	  instrument.serial           = db->value( 1 ).toString();
	  instrument.optimaHost       = db->value( 5 ).toString();
	  instrument.optimaPort       = db->value( 6 ).toString().toInt();
	  instrument.optimaDBname     = db->value( 7 ).toString();
	  instrument.optimaDBusername = db->value( 8 ).toString();
	  instrument.optimaDBpassw    = db->value( 9 ).toString();
	  instrument.selected         = db->value( 10 ).toString().toInt();
	  
	  instrument.os1              = db->value( 11 ).toString();
	  instrument.os2              = db->value( 12 ).toString();
	  instrument.os3              = db->value( 13 ).toString();

	  instrument.radcalwvl        = db->value( 14 ).toString();
	  instrument.chromoab         = db->value( 15 ).toString();

	  instrument.msgPort          = db->value( 16 ).toString().toInt();

	  qDebug() << "Ist.ID:  " << instrument.ID;
	  qDebug() << "Ist.name: " << instrument.name;
	  qDebug() << "msgPort: " << instrument.msgPort;
	  
	  // if ( instrument.name.contains("Optima") || instrument.optimaHost.contains("AUC_DATA_DB") )
	    this->instruments << instrument;
      }
    }
   qDebug() << "Reading Instrument: FINISH";

}

// Populate lw_entries
void US_XpnHostDB::update_lw( )
{
   lw_entries->clear();
   bool def_sel = false;
   
   if ( instruments.size() > 0 )
     {
       for ( int ii = 0; ii < instruments.size(); ii++ )
	 {
	   QString desc = instruments[ii].name.trimmed();

	   qDebug() << "Instrument Name: " << desc;

	   //ALEXEY: do not add any "default" as we won't enable settign default (from this program)
	   // if ( instruments[ii].selected )
	   //   {  // Select the matching entry in the list and flag it as selected
	   //     desc.append( " (default)" );
	   //     def_sel         = true;    // Flag a default selection made
	   //   }

	   QListWidgetItem* widget = new QListWidgetItem( desc );
	   lw_entries->addItem( widget );

	   if ( instruments[ii].selected )
	     lw_entries->setCurrentItem( widget, QItemSelectionModel::Select );
	 }

       if ( !def_sel  &&  instruments.size() > 0 )
	 {  // Insure that *something* is selected!
	   lw_entries->setCurrentRow( 0 );
	   QListWidgetItem* item = lw_entries->item( 0 );
	   optima_selected( item );
	 
	   qDebug() << "Def. sel. selected!!!";
	 }

       pb_testConnect->setEnabled( true );
       pb_delete     ->setEnabled( true );
       pb_edit       ->setEnabled( true );
     }
   else
     {
       
     }
}

// When selected, fill GUI
void US_XpnHostDB::optima_selected( )
{
  if ( lw_entries->currentRow() < 0 )
   {
      reset();
      return;
   }

  QListWidgetItem *entry = lw_entries->currentItem();
  QString item           = entry->text().remove( " (default)" );
  
  for ( int ii = 0; ii < instruments.size(); ii++ )
   {
     if ( item == instruments[ii].name )
       {
	 // Populate current entry GUI slots
	 le_description->setText( item );
	 le_serial     ->setText( instruments[ii].serial );
	 le_host       ->setText( instruments[ii].optimaHost );
	 le_port       ->setText( QString::number( instruments[ii].optimaPort ) );
	 le_name       ->setText( instruments[ii].optimaDBname );
	 le_user       ->setText( instruments[ii].optimaDBusername );
	 le_pasw       ->setText( instruments[ii].optimaDBpassw );

	 le_os1        ->setText( instruments[ii].os1 );
	 le_os2        ->setText( instruments[ii].os2 );
	 le_os3        ->setText( instruments[ii].os3 );

	 le_msgPort    ->setText( QString::number( instruments[ii].msgPort ) );

	 if ( instruments[ii].radcalwvl.isEmpty() || instruments[ii].chromoab.isEmpty() )
	   le_radcalwvl  ->setText( "N/A" );
	 else
	   le_radcalwvl  ->setText( instruments[ii].radcalwvl );

	 if ( instruments[ii].chromoab.isEmpty()  )
	   le_chromofile  ->setText( "Not Uploaded (zero)" );
	 else
	   le_chromofile  ->setText( "Uploaded" );
	 
       }
   }
  
  if ( !item.contains("Optima")  )
    {

      host          ->setVisible( false );
      le_host       ->setVisible( false );
      port	    ->setVisible( false );
      le_port       ->setVisible( false );

      msgPort	    ->setVisible( false );
      le_msgPort    ->setVisible( false );
      
      name	    ->setVisible( false );
      le_name       ->setVisible( false );
      user	    ->setVisible( false );
      le_user       ->setVisible( false );
      pasw	    ->setVisible( false );
      le_pasw       ->setVisible( false );
      
      bn_chromoab   ->setVisible( false );
      lb_radcalwvl  ->setVisible( false );
      le_radcalwvl  ->setVisible( false );
      lb_chromofile ->setVisible( false );
      le_chromofile ->setVisible( false );

      pb_testConnect->setEnabled( false );
      
    }
  else
    {
      host          ->setVisible( true );
      le_host       ->setVisible( true );
      port	    ->setVisible( true );
      le_port       ->setVisible( true );

      msgPort	    ->setVisible( true );
      le_msgPort    ->setVisible( true );
      
      name	    ->setVisible( true );
      le_name       ->setVisible( true );
      user	    ->setVisible( true );
      le_user       ->setVisible( true );
      pasw	    ->setVisible( true );
      le_pasw       ->setVisible( true );
			
      bn_chromoab   ->setVisible( true );
      lb_radcalwvl  ->setVisible( true );
      le_radcalwvl  ->setVisible( true );
      lb_chromofile ->setVisible( true );
      le_chromofile ->setVisible( true );
      
      pb_testConnect->setEnabled( true );
      
    }
}

// When selected, fill GUI
void US_XpnHostDB::optima_selected( QListWidgetItem *tmp_item )
{
  if ( lw_entries->currentRow() < 0 )
   {
      reset();
      return;
   }

  QListWidgetItem *entry = tmp_item;
  QString item           = entry->text().remove( " (default)" );

  qDebug() << "OPTIMA NAME: " << item;
  
  for ( int ii = 0; ii < instruments.size(); ii++ )
   {
     if ( item == instruments[ii].name )
       {
	 // Populate current entry GUI slots
	 le_description->setText( item );
	 le_serial     ->setText( instruments[ii].serial );
	 le_host       ->setText( instruments[ii].optimaHost );
	 le_port       ->setText( QString::number( instruments[ii].optimaPort ) );
	 le_name       ->setText( instruments[ii].optimaDBname );
	 le_user       ->setText( instruments[ii].optimaDBusername );
	 le_pasw       ->setText( instruments[ii].optimaDBpassw );

	 le_os1        ->setText( instruments[ii].os1 );
	 le_os2        ->setText( instruments[ii].os2 );
	 le_os3        ->setText( instruments[ii].os3 );

	 le_msgPort    ->setText( QString::number( instruments[ii].msgPort ) );

	 if ( instruments[ii].radcalwvl.isEmpty() || instruments[ii].chromoab.isEmpty() )
	   le_radcalwvl  ->setText( "N/A" );
	 else
	   le_radcalwvl  ->setText( instruments[ii].radcalwvl );

	 if ( instruments[ii].chromoab.isEmpty()  )
	   le_chromofile  ->setText( "Not Uploaded (zero)" );
	 else
	   le_chromofile  ->setText( "Uploaded" );
	 
       }
   }
    if ( !item.contains("Optima")  )
    {

      qDebug() << "NON_OPTIMA !!!!" ; 
      host          ->setVisible( false );
      le_host       ->setVisible( false );
      port	    ->setVisible( false );
      le_port       ->setVisible( false );

      msgPort	    ->setVisible( false );
      le_msgPort    ->setVisible( false );
      
      name	    ->setVisible( false );
      le_name       ->setVisible( false );
      user	    ->setVisible( false );
      le_user       ->setVisible( false );
      pasw	    ->setVisible( false );
      le_pasw       ->setVisible( false );
      
      bn_chromoab   ->setVisible( false );
      lb_radcalwvl  ->setVisible( false );
      le_radcalwvl  ->setVisible( false );
      lb_chromofile ->setVisible( false );
      le_chromofile ->setVisible( false );

      pb_testConnect->setEnabled( false );
      
    }
  else
    {
      host          ->setVisible( true );
      le_host       ->setVisible( true );
      port	    ->setVisible( true );
      le_port       ->setVisible( true );

      msgPort	    ->setVisible( true );
      le_msgPort    ->setVisible( true );
      
      name	    ->setVisible( true );
      le_name       ->setVisible( true );
      user	    ->setVisible( true );
      le_user       ->setVisible( true );
      pasw	    ->setVisible( true );
      le_pasw       ->setVisible( true );
			
      bn_chromoab   ->setVisible( true );
      lb_radcalwvl  ->setVisible( true );
      le_radcalwvl  ->setVisible( true );
      lb_chromofile ->setVisible( true );
      le_chromofile ->setVisible( true );

      pb_testConnect->setEnabled( true );
    }
}

// Select instrument
void US_XpnHostDB::select_db( QListWidgetItem* entry, const bool showmsg )
{
   qDebug() << "xpnH:selDB:  etext" << entry->text();
   
   //US_Passwd pw;
      // Pick up decrypted password and encrypt it.
   // The encrypted form is <cipher>"^"<initvect>
   //QString masterpw = pw.getPasswd();
   // QString decpw    = le_pasw->text();
   // QStringList pwl  = US_Crypto::encrypt( decpw, masterpw );
   // QString encpw    = pwl[ 0 ] + "^" + pwl[ 1 ];
   

   // Delete trailing "(default)" if that is present
   QString item     = entry->text().remove( " (default)" );
   
   for ( int ii = 0; ii < instruments.size(); ii++ )
   {
      if ( item == instruments[ii].name )
      {
	// Populate current entry GUI slots
	le_description->setText( item );
	le_host       ->setText( instruments[ii].optimaHost );
	le_port       ->setText( QString::number( instruments[ii].optimaPort ) );
	le_name       ->setText( instruments[ii].optimaDBname );
	le_user       ->setText( instruments[ii].optimaDBusername );
	le_pasw       ->setText( instruments[ii].optimaDBpassw );
	
	// QString encpw    = instruments[ii].optimaDBpassw;    // Encrypted password
	// QString cipher   = encpw.section( "^", 0, 0 ); // Cipher
	// QString initve   = encpw.section( "^", 1, 1 ); // initVector
	// QString decpw    = US_Crypto::decrypt( cipher, masterpw, initve );
	//le_pasw       ->setText( decpw );              // Decrypted password

	qDebug() << "SELECT: Passw.: decrypted " << le_pasw->text();
	
	// cb_os1        ->setCurrentIndex( cb_os1->findText( instruments[ii].os1 ) );
	// cb_os2        ->setCurrentIndex( cb_os2->findText( instruments[ii].os2 ) );
	// cb_os3        ->setCurrentIndex( cb_os3->findText( instruments[ii].os3 ) );

	le_os1        ->setText( instruments[ii].os1 );
	le_os2        ->setText( instruments[ii].os2 );
	le_os3        ->setText( instruments[ii].os3 );

	le_msgPort    ->setText( QString::number( instruments[ii].msgPort ) );
	
	// // Set the default DB and user for that DB
	// US_Settings::set_def_xpn_host( dblist.at( ii ) );


	//ALEXEY: set in DB selected to '1' for this entry and reset selected to '0' for all others...
	US_Passwd pw;
	US_DB2* db = use_db ? new US_DB2( pw.getPasswd() ) : NULL;
	
	if ( db->lastErrno() != US_DB2::OK )
	  {
	    QMessageBox::warning( this,
				  tr( "Database Connection Problem!" ),
				  tr( "You are not currently connected to a US3-LIMS DB!\n" ) );
	    return;
	  }
	
	QStringList q( "" );
	q.clear();
	q  << QString( "update_instrument_set_selected" )
	   << instruments[ii].name;                    
	
	db->query( q );
	
	//ALEXEY: and in instrument class:
	instruments[ii].selected = 1;
	for ( int jj = 0; jj < instruments.size(); jj++)
	  {
	    if ( instruments[jj].name == item  )
	      continue;
	    
	    instruments[jj].selected = 0;
	    q.clear();
	    q  << QString( "update_instrument_set_unselected" )
	       << instruments[jj].name;                    
	    
	    db->query( q );
	  }
	
	update_lw( );
	
	if ( showmsg )
	  {
            QMessageBox::information( this,
				      tr( "Optima Host Selected" ),
				      tr( "The default database host has been updated." ) );
	  }
	//pb_save  ->setEnabled( true );
	pb_delete->setEnabled( true );
	
	break;
      }
   }
}

// Reset current Instrument
void US_XpnHostDB::Instrument::reset( void )
{
   ID = -1;      
   name = "";    
   serial = "";  
   radialCalID = -1;
   optimaHost  = ""; 
   optimaPort  = -1;
   msgPort  = -1;
   optimaDBname = "";
   optimaDBusername = "";
   optimaDBpassw = "";
   selected = 0;
}

bool US_XpnHostDB::check_user_level()
{
  bool status = false;

  qDebug() << "User Level: " << US_Settings::us_inv_level();
  
  if ( US_Settings::us_inv_level() < 3 )
  {
      QMessageBox::warning( this,
                            tr( "Check User Level" ),
                            tr( "Low user level!\n This user cannot"
                                " Add, Edit and Delete Entries.\n" ) );
      status = true;
  }
  return status;
}


// Add New Entry
void US_XpnHostDB::add_new( void )
{

  if ( check_user_level() )
    return;
 
   reset();
   
   US_NewXpnHostDB* new_xpnhost_db  = new US_NewXpnHostDB;
   new_xpnhost_db ->setParent(this, Qt::Window);
   new_xpnhost_db->setWindowModality(Qt::WindowModal);
   new_xpnhost_db->setAttribute(Qt::WA_DeleteOnClose);
   
   connect( new_xpnhost_db, SIGNAL ( accepted( QMap <QString, QString> &) ), this, SLOT ( newHost( QMap <QString, QString> &) ) );
   connect( new_xpnhost_db, SIGNAL ( editnew_cancelled( ) ), this, SLOT ( editnew_cancelled() ) ); 
   new_xpnhost_db -> show();
}

// Edit Entry
void US_XpnHostDB::editDB( void )
{
   if ( check_user_level() )
     return;
  
   reset();
    
   QListWidgetItem* entry = lw_entries->currentItem();
   QString item = entry->text().remove( " (default)" );
   qDebug() << "editDB:  item" << item;

   int instID = 0;
   // int inst_count = 0;
   QMap<QString, QString> currentInstrument;
   // Go through list and chose matching description
   for ( int ii = 0; ii < instruments.size(); ii++ )
     {
       QString desc = instruments[ii].name;
       
       // Look for the current description
       if ( desc == item )
	 {
	   instID = instruments[ii].ID;
	   instID_toedit = instID;
	   
	   currentInstrument[ "ID" ]      = QString::number( instruments[ii].ID );
	   currentInstrument[ "name" ]    = instruments[ii].name;  
	   currentInstrument[ "serial" ]  = instruments[ii].serial;
	   currentInstrument[ "host" ]    = instruments[ii].optimaHost;
	   currentInstrument[ "port" ]    = QString::number( instruments[ii].optimaPort ); 
	   currentInstrument[ "dbname" ]  = instruments[ii].optimaDBname;
	   currentInstrument[ "dbusername" ] = instruments[ii].optimaDBusername; 
	   currentInstrument[ "dbpassw" ]  = instruments[ii].optimaDBpassw;   
	   currentInstrument[ "selected" ] = instruments[ii].selected;
	   currentInstrument[ "os1" ]      = instruments[ii].os1; 
	   currentInstrument[ "os2" ]      = instruments[ii].os2;   
	   currentInstrument[ "os3" ]      = instruments[ii].os3;

	   currentInstrument[ "radcalwvl" ]  = instruments[ii].radcalwvl;
	   currentInstrument[ "chromoab" ]   = instruments[ii].chromoab;

	   currentInstrument[ "msgPort" ]    = QString::number( instruments[ii].msgPort );
	   
	   break;
	 }
     }
   
   if ( ! instID )
     {
       QMessageBox::warning( this,
			     tr( "XpnHost Problem" ),
			     tr( "The description does not match any in the database list.\n" ) );
       
       return;
     }
  
  
   US_NewXpnHostDB* edit_xpnhost_db  = new US_NewXpnHostDB( currentInstrument );
   edit_xpnhost_db ->setParent(this, Qt::Window);
   edit_xpnhost_db->setWindowModality(Qt::WindowModal);
   edit_xpnhost_db->setAttribute(Qt::WA_DeleteOnClose);
   
   connect( edit_xpnhost_db, SIGNAL ( accepted( QMap <QString, QString> &) ), this, SLOT ( editHost( QMap <QString, QString> &) ) );
   connect( edit_xpnhost_db, SIGNAL ( editnew_cancelled( ) ), this, SLOT ( editnew_cancelled() ) ); 
   edit_xpnhost_db -> show();
}

void US_XpnHostDB::editHost( QMap < QString, QString > & newInstrument  )
{
  US_Passwd pw;
  QString masterpw = pw.getPasswd();
  US_DB2* db = use_db ? new US_DB2( masterpw ) : NULL;

  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this,
                            tr( "Database Connection Problem!" ),
                            tr( "You are not currently connected to the DB!\n"
                                "Error: " ) + db->lastErrno() + "\n" );
      return;
    }

  
  QStringList q( "" );
  q.clear();
  q  << QString( "update_instrument_new" )
     << QString::number( instID_toedit )
     << newInstrument[ "name"]
     << newInstrument[ "serialNumber"]
     << newInstrument[ "labID"]
     << newInstrument[ "optimaHost" ]
     << newInstrument[ "optimaPort" ]
     << newInstrument[ "optimaDBname" ]
     << newInstrument[ "optimaDBusername" ]
     << newInstrument[ "optimaDBpassw" ]
     << newInstrument[ "os1" ]
     << newInstrument[ "os2" ]
     << newInstrument[ "os3" ]
     << newInstrument[ "radCalWvl" ]
     << newInstrument[ "chromoArray" ]
     << newInstrument[ "msgPort" ];

  db->query( q );

  QString mtitle_done    = tr( "Success" );
  QString message_done   = tr( "Instrument information has been successfully edited." );
  QMessageBox::information( this, mtitle_done, message_done );
  
    
  readInstruments( db );
  update_lw( );

  // fillGui( newInstrument );

  // //QListWidgetItem* item_by_text = new QListWidgetItem;
  // for(int i = 0; i < lw_entries->count(); ++i)
  //   {
  //     QListWidgetItem* item_by_text = lw_entries->item(i);
  //     if ( item_by_text->text() == newInstrument[ "name"] )
  // 	lw_entries->setCurrentItem( item_by_text, QItemSelectionModel::Select );
  //     else
  // 	lw_entries->setCurrentItem( item_by_text, QItemSelectionModel::Deselect );
  //   }
  
}

void US_XpnHostDB::editnew_cancelled( )
{
  update_lw( );
}

// Fill available GUI elements
void US_XpnHostDB::fillGui( QMap<QString, QString> & instrument )
{
  if ( !instrument["name"].isEmpty()  )
    le_description->setText( instrument["name"] );
  
  // if ( !instrument["serial"].isEmpty()  )
  // le_serialNumber->setText( instrument["serial"] );
  
  if ( !instrument["optimaHost"].isEmpty()  )
    le_host->setText( instrument["optimaHost"] );

  if ( !instrument["optimaPort"].isEmpty()  )
    le_port->setText( instrument["optimaPort"] );

  if ( !instrument["msgPort"].isEmpty()  )
    le_msgPort->setText( instrument["msgPort"] );
  
  if ( !instrument["optimaDBusername"].isEmpty()  )
    le_user->setText( instrument["optimaDBusername"] );

  if ( !instrument["optimaDBname"].isEmpty()  )
    le_name->setText( instrument["optimaDBname"] );

  if ( !instrument["optimaDBpassw"].isEmpty()  )
    le_pasw->setText( instrument["optimaDBpassw"] );

  le_os1->setText( instrument[ "os1" ] );
  le_os2->setText( instrument[ "os2" ] );
  le_os3->setText( instrument[ "os3" ] );
}

void US_XpnHostDB::newHost( QMap < QString, QString > & newInstrument  )
{
  currentInstrument.reset();

  qDebug() << "New Instrument: " << newInstrument["name"] << ", " << newInstrument[ "optimaHost" ];

  US_Passwd pw;
  QString masterpw = pw.getPasswd();
  US_DB2* db = use_db ? new US_DB2( masterpw ) : NULL;

  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this,
                            tr( "Database Connection Problem!" ),
                            tr( "You are not currently connected to the DB!\n"
                                "Error: " ) + db->lastErrno() + "\n" );
      return;
    }

  // ALEXEY: we better use internal MySQL encryption with the same 'secret' for all users...
  // Use of default password encryption is a problem as it uses masterpassword for secret to ecrypt/decrypt OptimaDB password
  
  // QString decpw    = newInstrument[ "optimaDBpassw" ];            // Decrypted password
  // QStringList pwl  = US_Crypto::encrypt( decpw, masterpw );
  // QString encpw    = pwl[ 0 ] + "^" + pwl[ 1 ];  // Encrypted password

  // qDebug() << "Passw.: decrypted, encrypted: " << decpw << ", " << encpw;


  // IF Non-Beckman: these are int(11) fields, so must be defined
  if ( newInstrument[ "optimaPort" ].isEmpty()  )
    newInstrument[ "optimaPort" ] = QString::number(0);
  if ( newInstrument[ "msgPort" ].isEmpty()  )
    newInstrument[ "msgPort" ] = QString::number(0);
  ///////////////////////////////////////////////////////////////
  
  QStringList q( "" );
  q.clear();
  q  << QString( "add_instrument_new" )
     << newInstrument[ "name"]
     << newInstrument[ "serialNumber"]
     << newInstrument[ "labID"]
     << newInstrument[ "optimaHost" ]
     << newInstrument[ "optimaPort" ]
     << newInstrument[ "optimaDBname" ]
     << newInstrument[ "optimaDBusername" ]
     << newInstrument[ "optimaDBpassw" ]
     << newInstrument[ "os1" ]
     << newInstrument[ "os2" ]
     << newInstrument[ "os3" ]
     << newInstrument[ "radCalWvl" ]
     << newInstrument[ "chromoArray" ]
     << newInstrument[ "msgPort" ];
	
    //<< encpw;

  qDebug() << "New Instrument Query TEST: " << q; 
	
  db->query( q );

  QString mtitle_done    = tr( "Success" );
  QString message_done   = tr( "New instrument has been successfully added to DB." );
  QMessageBox::information( this, mtitle_done, message_done );
		  
  readInstruments( db );
  update_lw( );

}


//Help
void US_XpnHostDB::help( void )
{
   US_Help* showhelp = new US_Help(this);
   showhelp->show_help( "optima_host_config.html" );
}

//Test Connection
bool US_XpnHostDB::test_connect( void )
{
   QString xpnhost = le_host->text();
   QString xpnport = le_port->text();
   QString dbname  = le_name->text();
   QString dbuser  = le_user->text();
   QString dbpasw  = le_pasw->text();
   QString xpndesc = le_description->text();
qDebug() << "test_connect: dbpasw" << dbpasw;

   if ( xpnport.isEmpty() )
   {
      xpnport         = def_port;
      le_port->setText( xpnport );
   }

   if ( dbname.isEmpty() )
   {
      dbname          = def_name;
      le_name->setText( dbname );
   }

#if 0
   if ( dbuser.isEmpty() )
   {
      dbuser          = def_user;
      le_user->setText( dbuser );
   }

   if ( dbpasw.isEmpty() )
   {
      dbpasw          = def_pasw;
      le_pasw->setText( dbpasw );
   }
#endif

   if ( xpnhost.isEmpty()  ||  xpnport.isEmpty()  ||
        dbuser .isEmpty()  ||  dbpasw .isEmpty() )
   {
      QMessageBox::warning( this,
        tr( "Missing Data" ),
        tr( "Please fill in all fields before testing the connection." ) );

      return false;
   }

   US_XpnData* xpn_data = new US_XpnData();
   int ixpport     = xpnport.toInt();

qDebug() << "test_connect: (2)dbpasw" << dbpasw;
   bool ok         = xpn_data->connect_data( xpnhost, ixpport, dbname,
                                             dbuser, dbpasw );

   xpn_data->close();
   //delete xpn_data;

   if ( ok )
   {
      QMessageBox::information( this,
        tr( "OptimaHost Connection" ),
        tr( "The connection was successful." ) );

      //pb_save->setEnabled( true );
      conn_stat[ xpndesc ] = "connect_ok";
   }
   else
   {
      QMessageBox::warning( this,
        tr( "OptimaHost Connection" ),
        tr( "The connection failed.\n" ) + xpn_data->lastError() );

      conn_stat[ xpndesc ] = "connect_bad";
   }
   delete xpn_data;

   //pb_save->setEnabled( true );
   return ok;
}



// Delete stored host from DB
void US_XpnHostDB::deleteDB( void )
{
   if ( check_user_level() )
     return;
  
   bool default_to_remove = false;
  
   QListWidgetItem* entry = lw_entries->currentItem();

   if ( entry->text().contains("(default)")  )
     default_to_remove = true;
     
   QString item = entry->text().remove( " (default)" );
   qDebug() << "deleteDB:  item" << item;

   int response = QMessageBox::question( this,
      tr( "Delete Entry?" ),
      tr( "You have selected the following entry to delete:\n    \"" )
      + item + tr( "\"\n\nProceed?" ),
      QMessageBox::Yes, QMessageBox::Cancel );

   if ( response != QMessageBox::Yes )
      return;

   int instID = 0;
   int inst_count = 0;
   // Go through list and delete the one matching description
   for ( int ii = 0; ii < instruments.size(); ii++ )
     {
       QString desc = instruments[ii].name;
       
       // Look for the current description
       if ( desc == item )
	 {
	   instID = instruments[ii].ID;
	   inst_count = ii;
	   qDebug() << "DeleteDB ! instID: " << instID; 
	   break;
	 }
     }

   if ( instID )
     {
       US_Passwd pw;
       US_DB2* db = use_db ? new US_DB2( pw.getPasswd() ) : NULL;
       QStringList q( "" );
       q.clear();
       q  << QString( "delete_instrument" )
	  << QString::number( instID );      
       
       int status = db->statusQuery( q );
       
       if ( status == US_DB2::INSTRUMENT_IN_USE )
	 {
	   QMessageBox::warning( this,
				 tr( "Instrument Not Deleted" ),
				 tr( "This instrument could not be deleted since\n"
				     "it is in use in one or more experiments." ) );
	   return;
	 }

       instruments.removeAt( inst_count );
       update_lw( );

       QString msg("The database has been removed.\n");
       
       // ALEXEY: now check if the default DB was removed: if yes, reset default to the first in the item list
       if ( default_to_remove )
	 {
	   US_Passwd pw;
	   US_DB2* db = use_db ? new US_DB2( pw.getPasswd() ) : NULL;
	   
	   if ( db->lastErrno() != US_DB2::OK )
	     {
	       QMessageBox::warning( this,
				     tr( "Database Connection Problem!" ),
				     tr( "You are not currently connected to a US3-LIMS DB! \n" ) );
	       return;
	     }

	   QListWidgetItem* currentItem = lw_entries->currentItem();
	   	   
	   QStringList q( "" );
	   q.clear();
	   q  << QString( "update_instrument_set_selected" )
	      << currentItem->text();
	   
	   db->query( q );

	   for ( int jj = 0; jj < instruments.size(); jj++)
	     {
	       if ( instruments[jj].name == currentItem->text() )
		 instruments[jj].selected = 1;
	     }

	   update_lw( );
	   msg += QString("\n NOTE: the default DB was reset to %1").arg("Optima 1");
	 }

       //reset();
       QMessageBox::information( this,
				 tr( "XpnHost Removed" ),
				 msg );
       
       return;
     }
   
   QMessageBox::warning( this,
			 tr( "XpnHost Problem" ),
			 tr( "The description does not match any in the database list.\n"
			     "The database has not been removed." ) );
}

// reset all fields 
void US_XpnHostDB::reset( void )
{
   le_description->setText( "" );
   le_host       ->setText( "" );
   le_port       ->setText( "" );

   le_msgPort    ->setText( "" );
   
   le_name       ->setText( "" );
   le_user       ->setText( "" );
   le_pasw       ->setText( "" );
   le_os1        ->setText( "" );
   le_os2        ->setText( "" );
   le_os3        ->setText( "" );

   le_radcalwvl  ->setText( "" );
   le_chromofile ->setText( "" );

}

// Close & check if default Optima is selected
void US_XpnHostDB::close_program( void )
{
  
  close();
}


/*
void US_XpnHost::select_db( QListWidgetItem* entry, const bool showmsg )
{
   // When this is run, we will always have a current dblist

qDebug() << "xpnH:selDB: entry" << entry;
qDebug() << "xpnH:selDB:  etext" << entry->text();
   // Delete trailing "(default)" if that is present
   US_Passwd pw;
   QString item     = entry->text().remove( " (default)" );
   // Pick up decrypted password and encrypt it.
   // The encrypted form is <cipher>"^"<initvect>
   QString masterpw = pw.getPasswd();
   QString decpw    = le_pasw->text();
   QStringList pwl  = US_Crypto::encrypt( decpw, masterpw );
   QString encpw    = pwl[ 0 ] + "^" + pwl[ 1 ];

   for ( int ii = 0; ii < dblist.size(); ii++ )
   {
      if ( item == dblist.at( ii ).at( 0 ) )
      {
         int elen    = dblist.at( ii ).size();
qDebug() << "xpnH:selDB: item" << item << "elen" << elen;

         if ( elen < 9 )
         {  // Old entry has from 0 to 8 strings: use what you can
            QStringList dbentr;
            dbentr << ( ( elen > 0 ) ? dblist.at( ii ).at( 0 )
                                     : le_description->text() )
                   << ( ( elen > 1 ) ? dblist.at( ii ).at( 1 )
                                     : le_host       ->text() )
                   << ( ( elen > 2 ) ? dblist.at( ii ).at( 2 )
                                     : le_port       ->text() )
                   << ( ( elen > 3 ) ? dblist.at( ii ).at( 3 )
                                     : le_name       ->text() )
                   << ( ( elen > 4 ) ? dblist.at( ii ).at( 4 )
                                     : le_user       ->text() )
                   << ( ( elen > 5 ) ? dblist.at( ii ).at( 5 )
                                     : encpw )
                   << ( ( elen > 6 ) ? dblist.at( ii ).at( 6 )
                                     : cb_os1 ->currentText() )
                   << ( ( elen > 7 ) ? dblist.at( ii ).at( 7 )
                                     : cb_os2 ->currentText() )
                   <<                  cb_os3 ->currentText();
qDebug() << "xpnH:selDB:   --- less than 9 ---";
qDebug() << "xpnH:selDB:    dblist(ii)" << dblist.at(ii);
qDebug() << "xpnH:selDB:    dbentr    " << dbentr;
            dblist.replace( ii, dbentr );
         }
else
qDebug() << "xpnH:selDB:   +++ has all 9 +++";

         // Populate current entry GUI slots
         le_description->setText( item );
         le_host       ->setText( dblist.at( ii ).at( 1 ) );
         le_port       ->setText( dblist.at( ii ).at( 2 ) );
         le_name       ->setText( dblist.at( ii ).at( 3 ) );
         le_user       ->setText( dblist.at( ii ).at( 4 ) );
         encpw            = dblist.at( ii ).at( 5 );    // Encrypted password
         QString cipher   = encpw.section( "^", 0, 0 ); // Cipher
         QString initve   = encpw.section( "^", 1, 1 ); // initVector
         decpw            = US_Crypto::decrypt( cipher, masterpw, initve );
         le_pasw       ->setText( decpw );              // Decrypted password
         cb_os1        ->setCurrentIndex( cb_os1->findText(
                                  dblist.at( ii ).at( 6 ) ) );
         cb_os2        ->setCurrentIndex( cb_os2->findText(
                                  dblist.at( ii ).at( 7 ) ) );
         cb_os3        ->setCurrentIndex( cb_os3->findText(
                                  dblist.at( ii ).at( 8 ) ) );

         // Set the default DB and user for that DB
         US_Settings::set_def_xpn_host( dblist.at( ii ) );

         update_lw( item );

         if ( showmsg )
         {
            QMessageBox::information( this,
               tr( "Optima Host Selected" ),
               tr( "The default database host has been updated." ) );
         }
         pb_save  ->setEnabled( true );
         pb_delete->setEnabled( true );

         break;
      }
   }
}

void US_XpnHost::check_add()
{
   // Check that all fields have at least something
   if ( le_description->text().isEmpty() )
   {
      QMessageBox::information( this,
         tr( "Attention" ),
         tr( "Please enter a Description before saving..." ) );
      return;
   }

   if ( le_host->text().isEmpty() )
   {
      QMessageBox::information( this,
         tr( "Attention" ),
         tr( "Please enter a Optima DB Host Address before saving..." ) );
      return;
   }

   if ( le_port->text().isEmpty() )
      le_port->setText( def_port );

   if ( le_name->text().isEmpty() )
      le_name->setText( def_name );

#if 0
   if ( le_user->text().isEmpty() )
      le_user->setText( def_user );

   if ( le_pasw->text().isEmpty() )
      le_pasw->setText( def_pasw );
#endif

   // Save the DB entry
   dblist = US_Settings::xpn_db_hosts();
   bool updated = false;
   US_Passwd pw;
   QString decpw    = le_pasw->text();            // Decrypted password
   QString masterpw = pw.getPasswd();
   QStringList pwl  = US_Crypto::encrypt( decpw, masterpw );
   QString encpw    = pwl[ 0 ] + "^" + pwl[ 1 ];  // Encrypted password

   for ( int ii = 0; ii < dblist.size(); ii++ )
   {
      QStringList db = dblist.at( ii );
      if ( db.at( 0 ) == le_description->text() )
      {
         int elen    = dblist.at( ii ).size();
         db.replace( 1, le_host->text() );
         db.replace( 2, le_port->text() );
         if ( elen > 6 )
         {
            if ( elen < 8 ) db << "";
            if ( elen < 9 ) db << "";
            db.replace( 3, le_name->text() );
            db.replace( 4, le_user->text() );
            db.replace( 5, encpw );
            db.replace( 6, cb_os1 ->currentText() );
            db.replace( 7, cb_os2 ->currentText() );
            db.replace( 8, cb_os3 ->currentText() );
         }
         else if ( elen == 6 )
         {
            db << cb_os1 ->currentText();
            db << cb_os2 ->currentText();
            db << cb_os3 ->currentText();
         }
         else if ( elen == 3 )
         {
            db << le_name->text();
            db << le_user->text();
            db << encpw;
            db << cb_os1 ->currentText();
            db << cb_os2 ->currentText();
            db << cb_os3 ->currentText();
         }
         else if ( elen == 0 )
         {
            db << le_description->text();
            db << le_host->text();
            db << le_name->text();
            db << le_user->text();
            db << encpw;
            db << cb_os1 ->currentText();
            db << cb_os2 ->currentText();
            db << cb_os3 ->currentText();
         }

         dblist.replace( ii, db );
         updated = true;
         break;
      }
   }

   if ( ! updated )
   {
      QStringList entry;
      entry << le_description->text()
            << le_host->text()
            << le_port->text()
            << le_name->text()
            << le_user->text()
            << encpw
            << cb_os1 ->currentText()
            << cb_os2 ->currentText()
            << cb_os3 ->currentText();

      dblist << entry;
   }

   // Update the list widget
   US_Settings::set_xpn_db_hosts( dblist );
   update_lw( le_description->text() );

   if ( lw_entries->count() == 1 ) save_default();

   pb_save  ->setEnabled( true );
   pb_delete->setEnabled( true );
}

void US_XpnHost::update_lw( const QString& current )
{
   lw_entries->clear();

qDebug() << "xpnH: upd_lw: IN";
   dblist                = US_Settings::xpn_db_hosts();
qDebug() << "xpnH: upd_lw:  dblist" << dblist;
   QStringList defaultDB = US_Settings::defaultXpnHost();
qDebug() << "xpnH: upd_lw:  defDB" << defaultDB;
   QString     defaultDBname;
//xpn_db_hosts()
//set_xpn_db_hosts( const QList<QStringList>& );
//defaultXpnHost()
//set_def_xpn_host( const QStringList& );

   if ( dblist.size() == 0 )
   {  // First-time:  create an initial default entry
      QStringList entry;
      entry << def_desc
            << def_host
            << def_port
            << def_name
            << def_user
            << def_pasw
            << "UV/visible"
            << "Rayleigh Interference"
            << "(not installed)";

      dblist << entry;

      if ( defaultDB.size() == 0 )
      {  // If we have no default, set the dummy entry as such
         defaultDB       = entry;
      }
   }
   else
   {  // Insure default entry has encrypted password
      if ( defaultDB[ 5 ].length() <  20 )
      {
         QStringList entry;
         entry << defaultDB[ 0 ]
               << defaultDB[ 1 ]
               << defaultDB[ 2 ]
               << defaultDB[ 3 ]
               << defaultDB[ 4 ]
               << dblist.at( 0 )[ 5 ]
               << defaultDB[ 6 ]
               << defaultDB[ 7 ]
               << defaultDB[ 8 ];
          defaultDB = entry;
          US_Settings::set_def_xpn_host( defaultDB );
      }
   }

   if ( defaultDB.size() > 0 )   defaultDBname = defaultDB.at( 0 );
   bool def_sel    = false;         // Flag no default selection yet

   for ( int ii = 0; ii < dblist.size(); ii++ )
   {  // Search entries for a match to the default
      QString desc    = dblist.at( ii ).at( 0 );
      QString display = desc;

      // Flag a match to the default with " (default)" in the list
      if ( desc == defaultDBname )  display.append( " (default)" );

      QListWidgetItem* widget = new QListWidgetItem( display );
      lw_entries->addItem( widget );

      if ( desc == current )
      {  // Select the matching entry in the list and flag it as selected
         lw_entries->setCurrentItem( widget, QItemSelectionModel::Select );
         def_sel         = true;    // Flag a default selection made
      }
   }

   if ( ! def_sel  &&  dblist.size() > 0 )
   {  // Insure that *something* is selected!
      lw_entries->setCurrentRow( 0 );
   }
qDebug() << "xpnH: upd_lw: OUT";
}

void US_XpnHost::reset( void )
{
   QStringList DB = US_Settings::defaultXpnHost();
   QString     defaultDB;
   if ( DB.size() > 0 ) defaultDB = US_Settings::defaultXpnHost().at( 0 );
   update_lw( defaultDB );

   le_description->setText( "" );
   le_host       ->setText( "" );
   le_port       ->setText( def_port );
   le_name       ->setText( def_name );
   le_user       ->setText( def_user );
   le_pasw       ->setText( def_pasw );
   cb_os1        ->setCurrentIndex( 0 );
   cb_os2        ->setCurrentIndex( 1 );
   cb_os3        ->setCurrentIndex( 2 );

   pb_save       ->setEnabled( false );
   pb_delete     ->setEnabled( false );
}

void US_XpnHost::help( void )
{
   US_Help* showhelp = new US_Help(this);
   showhelp->show_help( "database_config.html" );
}

void US_XpnHost::save_default( void )
{
   for ( int ii = 0; ii < dblist.size(); ii++ )
   {
      QString desc = dblist.at( ii ).at( 0 );

      // Look for the current description
      if ( desc == le_description->text() )
      {
         if ( conn_stat[ desc ] != "connect_ok" )
         {
            QMessageBox::information( this,
               tr( "Default OptimaHost Problem" ),
               tr( "The current database information has not been tested "
                   "for connectivity.\n"
                   "The default database has not been updated.") );
            return;
         }

         QString null = "";

         US_Settings::set_def_xpn_host( dblist.at( ii ) );
         reset();

         QMessageBox::information( this,
            tr( "Default OptimaHost" ),
            tr( "The default database has been updated." ) );
         return;
      }
   }

   QMessageBox::warning( this,
      tr( "Default OptimaHost Problem" ),
      tr( "The description does not match any in the database list.\n"
          "The default database has not been updated." ) );
}

void US_XpnHost::deleteDB( void )
{
   QListWidgetItem* entry = lw_entries->currentItem();
   QString item = entry->text().remove( " (default)" );
qDebug() << "deleteDB:  item" << item;

   int response = QMessageBox::question( this,
      tr( "Delete Entry?" ),
      tr( "You have selected the following entry to delete:\n    \"" )
      + item + tr( "\"\n\nProceed?" ),
      QMessageBox::Yes, QMessageBox::Cancel );

   if ( response != QMessageBox::Yes )
      return;

   // Go through list and delete the one matching description
   for ( int ii = 0; ii < dblist.size(); ii++ )
   {
      QString desc = dblist.at( ii ).at( 0 );

      // Look for the current description
      if ( desc == item )
      {
         dblist.removeAt( ii );
         US_Settings::set_xpn_db_hosts( dblist );

         // Check if the default DB matches
         QStringList defaultDB = US_Settings::defaultXpnHost();

         if ( defaultDB.at( 0 ) == item )
         {
            if ( dblist.size() > 0 )
               US_Settings::set_def_xpn_host( dblist.at(0) );
            else
               US_Settings::set_def_xpn_host( QStringList() );
         }

         reset();

         QMessageBox::information( this,
            tr( "XpnHost Removed" ),
            tr( "The database has been removed." ) );
         return;
      }
   }

   QMessageBox::warning( this,
      tr( "XpnHost Problem" ),
      tr( "The description does not match any in the database list.\n"
          "The database has not been removed." ) );
}

bool US_XpnHost::test_connect( void )
{
   QString xpnhost = le_host->text();
   QString xpnport = le_port->text();
   QString dbname  = le_name->text();
   QString dbuser  = le_user->text();
   QString dbpasw  = le_pasw->text();
   QString xpndesc = le_description->text();
qDebug() << "test_connect: dbpasw" << dbpasw;

   if ( xpnport.isEmpty() )
   {
      xpnport         = def_port;
      le_port->setText( xpnport );
   }

   if ( dbname.isEmpty() )
   {
      dbname          = def_name;
      le_name->setText( dbname );
   }

#if 0
   if ( dbuser.isEmpty() )
   {
      dbuser          = def_user;
      le_user->setText( dbuser );
   }

   if ( dbpasw.isEmpty() )
   {
      dbpasw          = def_pasw;
      le_pasw->setText( dbpasw );
   }
#endif

   if ( xpnhost.isEmpty()  ||  xpnport.isEmpty()  ||
        dbuser .isEmpty()  ||  dbpasw .isEmpty() )
   {
      QMessageBox::warning( this,
        tr( "Missing Data" ),
        tr( "Please fill in all fields before testing the connection." ) );

      return false;
   }

   US_XpnData* xpn_data = new US_XpnData();
   int ixpport     = xpnport.toInt();

qDebug() << "test_connect: (2)dbpasw" << dbpasw;
   bool ok         = xpn_data->connect_data( xpnhost, ixpport, dbname,
                                             dbuser, dbpasw );

   xpn_data->close();
   delete xpn_data;

   if ( ok )
   {
      QMessageBox::information( this,
        tr( "OptimaHost Connection" ),
        tr( "The connection was successful." ) );

      pb_save->setEnabled( true );
      conn_stat[ xpndesc ] = "connect_ok";
   }
   else
   {
      QMessageBox::warning( this,
        tr( "OptimaHost Connection" ),
        tr( "The connection failed.\n" ) + xpn_data->lastError() );

      conn_stat[ xpndesc ] = "connect_bad";
   }

   pb_save->setEnabled( true );
   return ok;
}

*/
