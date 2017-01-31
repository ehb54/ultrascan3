//! \file us_xpnhost.cpp
#include "us_xpnhost.h"
#include "us_xpn_data.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_passwd.h"
#include "us_help.h"
#include "us_crypto.h"
#include "us_db2.h"

#define def_port QString("5432")
#define def_name QString("AUC_DATA_DB")
#define def_user QString("aucuser")
#define def_pasw QString("aucuser")

US_XpnHost::US_XpnHost( QWidget* w, Qt::WindowFlags flags )
  : US_Widgets( true, w, flags )
{
   // Frame layout
   setPalette( US_GuiSettings::frameColor() );

   setWindowTitle( "XPN DB Host Configuration" );
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
   QLabel* banner = us_banner( tr( "XPN Host List" ) );
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
   QLabel* info = us_banner( tr( "XPN Host Detailed Information" ) );
   topbox->addWidget( info );

   // Row 0
   int row = 0;
   QGridLayout* details = new QGridLayout();

   // Row 1
   QLabel* desc         = us_label( "XPN Host Description:" );
   le_description       = us_lineedit( "", 0 );
   details->addWidget( desc,           row,   0 );
   details->addWidget( le_description, row++, 1 );

   // Row 2
   QLabel* host        = us_label( "XPN DB Host Address:" );
   le_host             = us_lineedit( "", 0 );
   details->addWidget( host,           row,   0 );
   details->addWidget( le_host,        row++, 1 );

   // Row 3
   QLabel* port        = us_label( "XPN DB Port:" );
   le_port             = us_lineedit( "", 0 );
   details->addWidget( port,           row,   0 );
   details->addWidget( le_port,        row++, 1 );

   // Row 4
   QLabel* name        = us_label( "XPN DB Name:" );
   le_name             = us_lineedit( "", 0 );
   details->addWidget( name,           row,   0 );
   details->addWidget( le_name,        row++, 1 );

   // Row 5
   QLabel* user        = us_label( "DB Username:" );
   le_user             = us_lineedit( "", 0 );
   details->addWidget( user,           row,   0 );
   details->addWidget( le_user,        row++, 1 );

   // Row 6
   QLabel* pasw        = us_label( "DB Password:" );
   le_pasw             = us_lineedit( "", 0 );
   details->addWidget( pasw,           row,   0 );
   details->addWidget( le_pasw,        row++, 1 );

   // Make the line edit entries wider
   QFontMetrics fm( le_host->font() );
   le_port->setMinimumWidth( fm.maxWidth() * 10 );

   details->addWidget( le_port, row++, 1 );

   topbox->addLayout( details );

   //Pushbuttons
   row = 0;
   QGridLayout* buttons = new QGridLayout();

   pb_save = us_pushbutton( tr( "Save Entry" ) );
   pb_save->setEnabled( false );
   connect( pb_save, SIGNAL( clicked() ), this, SLOT( check_add() ) );
   buttons->addWidget( pb_save, row, 0 );

   pb_delete = us_pushbutton( tr( "Delete Current Entry" ) );
   pb_delete->setEnabled( false );
   connect( pb_delete,      SIGNAL( clicked()  ),
            this,           SLOT  ( deleteDB() ) );
   buttons->addWidget( pb_delete, row++, 1 );

   pb_testConnect = us_pushbutton( tr( "Test XPN Host Connectivity" ) );
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
}

void US_XpnHost::select_db( QListWidgetItem* entry )
{
   // When this is run, we will always have a current dblist

   // Delete trailing (default) if that is present
   QString item = entry->text().remove( " (default)" );

   for ( int ii = 0; ii < dblist.size(); ii++ )
   {
      if ( item == dblist.at( ii ).at( 0 ) )
      {
         le_description->setText( item );
         le_host       ->setText( dblist.at( ii ).at( 1 ) );
         le_port       ->setText( dblist.at( ii ).at( 2 ) );
         le_name       ->setText( dblist.at( ii ).at( 3 ) );
         le_user       ->setText( dblist.at( ii ).at( 4 ) );
         le_pasw       ->setText( dblist.at( ii ).at( 5 ) );

         // Set the default DB and user for that DB
         US_Settings::set_def_xpn_host( dblist.at( ii ) );

         update_lw( item );

         QMessageBox::information( this,
            tr( "Xpn Host Selected" ),
            tr( "The default database host has been updated." ) );
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
         tr( "Please enter an XPN DB Host Address before saving..." ) );
      return;
   }

   if ( le_port->text().isEmpty() )
      le_port->setText( def_port );

   if ( le_name->text().isEmpty() )
      le_name->setText( def_name );

   if ( le_user->text().isEmpty() )
      le_user->setText( def_user );

   if ( le_pasw->text().isEmpty() )
      le_pasw->setText( def_pasw );

   // Save the DB entry
   dblist = US_Settings::xpn_db_hosts();
   bool updated = false;

   for ( int ii = 0; ii < dblist.size(); ii++ )
   {
      QStringList db = dblist.at( ii );
      if ( db.at( 0 ) == le_description->text() )
      {
         db.replace( 1, le_host->text() );
         db.replace( 2, le_port->text() );
         db.replace( 3, le_name->text() );
         db.replace( 4, le_user->text() );
         db.replace( 5, le_pasw->text() );

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
            << le_pasw->text();

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

   dblist                = US_Settings::xpn_db_hosts();
   QStringList defaultDB = US_Settings::defaultXpnHost();
   QString     defaultDBname;
//xpn_db_hosts()
//set_xpn_db_hosts( const QList<QStringList>& );
//defaultXpnHost()
//set_def_xpn_host( const QStringList& );

   if ( defaultDB.size() > 0 ) defaultDBname = defaultDB.at( 0 );

   for ( int ii = 0; ii < dblist.size(); ii++ )
   {
      QString desc    = dblist.at( ii ).at( 0 );
      QString display = desc;

      if ( desc == defaultDBname ) display.append( " (default)" );

      QListWidgetItem* widget = new QListWidgetItem( display );
      lw_entries->addItem( widget );

      if ( desc == current )
         lw_entries->setCurrentItem( widget, QItemSelectionModel::Select );
   }
}

void US_XpnHost::reset( void )
{
   QStringList DB = US_Settings::defaultXpnHost();
   QString     defaultDB;
   if ( DB.size() > 0 ) defaultDB = US_Settings::defaultXpnHost().at( 0 );
   update_lw( defaultDB );

   le_description->setText("");
   le_host       ->setText("");
   le_port       ->setText("");
   le_name       ->setText("");
   le_user       ->setText("");
   le_pasw       ->setText("");

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
               tr( "Default XpnHost Problem" ),
               tr( "The current database information has not been tested "
                   "for connectivity.\n"
                   "The default database has not been updated.") );
            return;
         }

         QString null = "";

         US_Settings::set_def_xpn_host( dblist.at( ii ) );
         reset();

         QMessageBox::information( this,
            tr( "Default XpnHost" ),
            tr( "The default database has been updated." ) );
         return;
      }
   }

   QMessageBox::warning( this,
      tr( "Default XpnHost Problem" ),
      tr( "The description does not match any in the database list.\n"
          "The default database has not been updated." ) );
}

void US_XpnHost::deleteDB( void )
{
   QString item = le_description->text();

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
            US_Settings::set_def_xpn_host( QStringList() );

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

   if ( xpnhost.isEmpty()  ||  xpnport.isEmpty() )
   {
      QMessageBox::warning( this,
        tr( "Missing Data" ),
        tr( "Please fill in all fields before testing the connection." ) );

      return false;
   }

   US_XpnData* xpn_data = new US_XpnData();
   int ixpport     = xpnport.toInt();

   bool ok         = xpn_data->connect_data( xpnhost, ixpport, dbname,
                                             dbuser, dbpasw );

   xpn_data->close();
   delete xpn_data;

   if ( ok )
   {
      QMessageBox::information( this,
        tr( "XpnHost Connection" ),
        tr( "The connection was successful." ) );

      pb_save->setEnabled( true );
      conn_stat[ xpndesc ] = "connect_ok";
   }
   else
   {
      QMessageBox::warning( this,
        tr( "XpnHost Connection" ),
        tr( "The connection failed.\n" ) + xpn_data->lastError() );

      conn_stat[ xpndesc ] = "connect_bad";
   }

   pb_save->setEnabled( true );
   return ok;
}

