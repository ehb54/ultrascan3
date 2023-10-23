//! \file us_xpnhost.cpp
#include "us_xpnhost.h"
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

US_XpnHost::US_XpnHost( QWidget* w, Qt::WindowFlags flags )
  : US_Widgets( true, w, flags )
{
   // Frame layout
   setPalette( US_GuiSettings::frameColor() );

   setWindowTitle( "Optima DB Host Configuration" );
   setAttribute( Qt::WA_DeleteOnClose );

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
   QLabel* banner = us_banner( tr( "Optima Host List" ) );
   topbox->addWidget( banner );

   QLabel* banner2 = us_banner(
        tr( "(Doubleclick for details and set the default)" ), -1 );
   topbox->addWidget( banner2 );

   lw_entries = new QListWidget();
   lw_entries->setSortingEnabled( true );
   lw_entries->setPalette( US_GuiSettings::editColor() );
   lw_entries->setFont( QFont( US_GuiSettings::fontFamily(),
                              US_GuiSettings::fontSize() ) );
   topbox->addWidget( lw_entries );

   // Populate db_list
   QStringList XpnHosts = US_Settings::defaultXpnHost();
   QString     defXpnHost;
   if ( XpnHosts.size() > 0 )
      defXpnHost = US_Settings::defaultXpnHost().at( 0 );
   update_lw( defXpnHost );

   connect( lw_entries, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
                        SLOT  ( select_db        ( QListWidgetItem* ) ) );
   // Detail info
   QLabel* info = us_banner( tr( "Optima Host Detailed Information" ) );
   topbox->addWidget( info );

   // Row 0
   int row = 0;
   QGridLayout* details = new QGridLayout();

   // Row 1
   QLabel* desc         = us_label( tr( "Optima Host Description:" ) );
   le_description       = us_lineedit( "", 0 );
   details->addWidget( desc,           row,   0, 1, 2 );
   details->addWidget( le_description, row++, 2, 1, 2 );

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
#if 0
         << "Fluorescense"
#endif
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
   cb_os3->setCurrentIndex( 2 );
   
   topbox->addLayout( details );

   //Pushbuttons
   row = 0;
   QGridLayout* buttons = new QGridLayout();

   pb_save = us_pushbutton( tr( "Save Entry" ) );
   pb_save->setEnabled( false );
   connect( pb_save, SIGNAL( clicked() ), this, SLOT( check_add() ) );
   buttons->addWidget( pb_save, row, 0 );

   pb_delete = us_pushbutton( tr( "Delete Current Entry" ) );
   pb_delete->setEnabled( true );
   connect( pb_delete,      SIGNAL( clicked()  ),
            this,           SLOT  ( deleteDB() ) );
   buttons->addWidget( pb_delete, row++, 1 );

   pb_testConnect = us_pushbutton( tr( "Test Optima Host Connectivity" ) );
   connect( pb_testConnect, SIGNAL( clicked()      ),
            this,           SLOT  ( test_connect() ) );
   buttons->addWidget( pb_testConnect, row++, 0, 1, 2 );

   QHBoxLayout* std_buttons = new QHBoxLayout;

   pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset,       SIGNAL( clicked() ),
            this,           SLOT  ( reset()   ) );
   std_buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help,        SIGNAL( clicked() ),
            this,           SLOT  ( help()    ) );
   std_buttons->addWidget( pb_help );

   QPushButton* pb_cancel = us_pushbutton( tr( "Close" ) );
   connect( pb_cancel,      SIGNAL( clicked() ),
            this,           SLOT  ( close()   ) );
   std_buttons->addWidget( pb_cancel );

   topbox->addLayout( buttons );
   topbox->addLayout( std_buttons );

   adjustSize();
   int lwid             = lw_entries->width();
   int lhgt             = pb_help   ->height() * 6;
   lw_entries->resize( lwid, lhgt );
   adjustSize();

   // Start out with default entry shown
qDebug() << "xpnH:Main: call xpn_db_hosts";
   dblist                = US_Settings::xpn_db_hosts();
qDebug() << "xpnH:Main:  size" << dblist.size() << "dblist" << dblist;
   if ( dblist.size() == 0 )
      update_lw(  tr( "place-holder" ) );
qDebug() << "xpnH:Main: call select_db";
   select_db( lw_entries->currentItem(), false );
}

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
   showhelp->show_help( "optima_host_config.html" );
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
   //delete xpn_data;

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
   delete xpn_data;
   
   pb_save->setEnabled( true );
   return ok;
}

