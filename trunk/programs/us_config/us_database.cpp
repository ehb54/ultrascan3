//! \file us_database.cpp
#include "us_database.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_passwd.h"
#include "us_help.h"
#include "us_crypto.h"
#include "us_db2.h"

US_Database::US_Database( QWidget* w, Qt::WindowFlags flags ) 
  : US_Widgets( true, w, flags )
{
  uuid.clear();

  // Frame layout
  setPalette( US_GuiSettings::frameColor() );

  setWindowTitle( "Database Configuration" );
  setAttribute( Qt::WA_DeleteOnClose );

  // Set up the database list window
  QLabel* banner = us_banner( tr( "Database List" ) );

  QBoxLayout* topbox = new QVBoxLayout( this );
  topbox->addWidget( banner );
  topbox->setSpacing( 2 );

  lw_entries = new QListWidget();
  lw_entries->setSortingEnabled( true );
  lw_entries->setPalette( US_GuiSettings::editColor() );
  lw_entries->setFont( QFont( US_GuiSettings::fontFamily(),
                              US_GuiSettings::fontSize() ) );

  // Populate db_list
  lw_entries->setCurrentRow( 0 );
  connect( lw_entries, SIGNAL( itemPressed( QListWidgetItem* ) ), 
           this,       SLOT  ( select_db  ( QListWidgetItem* ) ) );

  update_lw();  // Fill in the list widget
  topbox->addWidget( lw_entries );

  // Detail info
  QLabel* info = us_banner( tr( "Database Detailed  Information" ) );
  topbox->addWidget( info );

  int row = 0;
  QGridLayout* details = new QGridLayout();
  
  // Row 1
  QLabel* desc = us_label( "Database Description:" );
  details->addWidget( desc, row, 0 );

  le_description = us_lineedit( "", 0 );
  details->addWidget( le_description, row++, 1 );

  // Row 2
  QLabel* user = us_label( "User Name:" );
  details->addWidget( user, row, 0 );

  le_username = us_lineedit( "", 0 );
  details->addWidget( le_username, row++, 1 );

  // Row 3
  QLabel* password = us_label( "Password:" );
  details->addWidget( password, row, 0 );

  le_password = us_lineedit( "", 0 );
  le_password->setEchoMode( QLineEdit::Password );
  details->addWidget( le_password, row++, 1 );

  // Row 4
  QLabel* DBname = us_label( "Database Name:" );
  details->addWidget( DBname, row, 0 );

  le_dbname = us_lineedit( "", 0 );
  details->addWidget( le_dbname, row++, 1 );

  // Row 5
  QLabel* host = us_label( "Host Address:" );
  details->addWidget( host, row, 0 );

  le_host = us_lineedit( "", 0 );
  details->addWidget( le_host, row++, 1 );

  // Row 6
  QLabel* investigator = us_label( "Investigator Email:" );
  details->addWidget( investigator, row, 0 );

  le_investigator_email = us_lineedit( "", 0 );
  
  // Make the line edit entries wider
  QFontMetrics fm( le_investigator_email->font() );
  le_investigator_email->setMinimumWidth( fm.maxWidth() * 10 );

  details->addWidget( le_investigator_email, row++, 1 );

  // Row 7
  QLabel* investigator_pw = us_label( "Investigator Password:" );
  details->addWidget( investigator_pw, row, 0 );

  le_investigator_pw = us_lineedit( "", 0 );
  le_investigator_pw->setEchoMode( QLineEdit::Password );
  details->addWidget( le_investigator_pw, row++, 1 );

  topbox->addLayout( details );

  //Pushbuttons

  row = 0;
  QGridLayout* buttons = new QGridLayout();

  pb_add = us_pushbutton( tr( "Add New Entry" ) );
  connect( pb_add, SIGNAL( clicked() ), this, SLOT( check_add() ) );
  buttons->addWidget( pb_add, row, 0 );

  pb_delete = us_pushbutton( tr( "Delete Current Entry" ) );
  pb_delete->setEnabled( false );
  connect( pb_delete, SIGNAL( clicked() ), this, SLOT( deleteDB() ) );
  buttons->addWidget( pb_delete, row++, 1 );

  pb_save = us_pushbutton( tr( "Save as Default" ) );
  pb_save->setEnabled( false );
  connect( pb_save, SIGNAL( clicked() ), this, SLOT( save_default() ) );
  buttons->addWidget( pb_save, row, 0 );

  pb_reset = us_pushbutton( tr( "Reset" ) );
  connect( pb_reset, SIGNAL( clicked() ), this, SLOT( reset() ) );
  buttons->addWidget( pb_reset, row++, 1 );

  pb_testConnect = us_pushbutton( tr( "Test Database Connectivity" ) );
  connect( pb_testConnect, SIGNAL( clicked() ), this, SLOT( test_connect() ) );
  buttons->addWidget( pb_testConnect, row++, 0, 1, 2 );

  QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
  connect( pb_help, SIGNAL( clicked() ), this, SLOT( help() ) );
  buttons->addWidget( pb_help, row, 0 );

  QPushButton* pb_cancel = us_pushbutton( tr( "Close" ) );
  connect( pb_cancel, SIGNAL( clicked() ), this, SLOT( close() ) );
  buttons->addWidget( pb_cancel, row++, 1 );

  topbox->addLayout( buttons );
}

void US_Database::select_db( QListWidgetItem* entry )
{
  // When this is run, we will always have a current dblist
  
  // Delete trailing (default) if that is present
  QString item = entry->text().remove( " (default)" );

  // Get the master PW
  US_Passwd pw;
  QString master_pw = pw.getPasswd();

  for ( int i = 0; i < dblist.size(); i++ )
  {
    if ( item == dblist.at( i ).at( 0 ) )
    {
      le_description       ->setText( item );
      le_username          ->setText( dblist.at( i ).at( 1 ) );
      le_dbname            ->setText( dblist.at( i ).at( 2 ) );
      le_host              ->setText( dblist.at( i ).at( 3 ) );
      le_investigator_email->setText( dblist.at( i ).at( 6 ) );

      // DB Logon PW
      QString cipher    = dblist.at( i ).at( 4 );
      QString iv        = dblist.at( i ).at( 5 );
      QString pw_string = US_Crypto::decrypt( cipher, master_pw, iv );
      le_password->setText( pw_string );   
      
      // DB Internal PW
      cipher    = dblist.at( i ).at( 7 );
      iv        = dblist.at( i ).at( 8 );
      pw_string = US_Crypto::decrypt( cipher, master_pw, iv );
      le_investigator_pw->setText( pw_string );
     
      uuid = dblist.at( i ).at( 9 );

      pb_save       ->setEnabled( true );
      pb_delete     ->setEnabled( true );

      break;
    }
  }
}

void US_Database::check_add()
{
  // Check that all fields have at least something
  if ( le_description->text().isEmpty() )
  {
    QMessageBox::information( this, 
        tr( "Attention" ), 
        tr( "Please enter a Description before saving..." ) );
    return;
  }

  if ( le_username->text().isEmpty() )
  {
    QMessageBox::information( this, 
        tr( "Attention" ), 
        tr( "Please enter a User Name before saving..." ) );
    return;
  }

  if ( le_password->text().isEmpty() )
  {
    QMessageBox::information( this,
        tr( "Attention" ), 
        tr( "Please enter a Password before saving..." ) );
    return;
  }

  if ( le_dbname->text().isEmpty() )
  {
    QMessageBox::information( this,
        tr( "Attention" ), 
        tr( "Please enter a Database Name before saving..."));
    return;
  }

  if ( le_host->text().isEmpty() )
  {
    QMessageBox::information( this,
        tr( "Attention" ), 
        tr( "Please enter a Host Address (possibly  'localhost') "
            "before saving..." ) );
    return;
  }
  
  if ( le_investigator_email->text().isEmpty() )
  {
    QMessageBox::information( this,
        tr( "Attention" ), 
        tr( "Please enter an investigator email address "
            "before saving..." ) );
    return;
  }
  
  if ( le_investigator_pw->text().isEmpty() )
  {
    QMessageBox::information( this,
        tr( "Attention" ), 
        tr( "Please enter an investigator password "
            "before saving..." ) );
    return;
  }
  
  if ( uuid.isEmpty() )
  {
    if ( ! test_connect() ) return;
  }
  
  // Get the master PW
  US_Passwd pw;
  QString   master_pw = pw.getPasswd();

  // Encrypt the DB and investigator passwords with the master password
  QString     password = le_password->text();
  QStringList pw_list  = US_Crypto::encrypt( password , master_pw );

  password             = le_investigator_pw->text();
  QStringList inv_pw   = US_Crypto::encrypt( password, master_pw );

  // Save the DB entry
  dblist = US_Settings::databases();
  bool updated = false;

  for ( int i = 0; i < dblist.size(); i++ )
  {
     QStringList db = dblist.at( i );
     if ( db.at( 0 ) == le_description->text() )
     {
        db.replace( 1, le_username->text() );
        db.replace( 2, le_dbname  ->text() );
        db.replace( 3, le_host    ->text() );
        db.replace( 4, pw_list.at( 0 )     );  // Ecnrypted password
        db.replace( 5, pw_list.at( 1 )     );  // Initialization vector
        db.replace( 6, le_investigator_email->text() );  
        db.replace( 7, inv_pw.at( 0 )      );
        db.replace( 8, inv_pw.at( 1 )      );
        db.replace( 9, uuid                );

        dblist.replace( i, db );
        updated = true;
        break;
     }
  }

  if ( ! updated )
  {
    QStringList entry;
    entry << le_description->text() 
          << le_username->text() 
          << le_dbname->text()
          << le_host->text()        
          << pw_list.at( 0 )          
          << pw_list.at( 1 ) 
          << le_investigator_email->text()  
          << inv_pw.at( 0 )
          << inv_pw.at( 1 )
          << uuid;  

    dblist << entry;
  }

  // Update the list widget
  US_Settings::set_databases( dblist );
  update_lw( le_description->text() );

  if ( lw_entries->count() == 1 ) save_default();

  pb_save       ->setEnabled( true );
  pb_delete     ->setEnabled( true );
} 

void US_Database::update_lw( const QString& current )
{
  lw_entries->clear();

  dblist                = US_Settings::databases();
  QStringList defaultDB = US_Settings::defaultDB();
  QString     defaultDBname;

  if ( defaultDB.size() > 0 ) defaultDBname = defaultDB.at( 0 );

  for ( int i = 0; i < dblist.size(); i++ )
  {
    QString desc    = dblist.at( i ).at( 0 );
    QString display = desc;
    
    if ( desc == defaultDBname ) display.append( " (default)" );
    
    QListWidgetItem* widget = new QListWidgetItem( display );
    lw_entries->addItem( widget );  
     
    if ( desc == current ) 
      lw_entries->setCurrentItem( widget, QItemSelectionModel::Select );
  }
}

void US_Database::reset( void )
{
  QStringList DB = US_Settings::defaultDB();
  QString     defaultDB;
  if ( DB.size() > 0 ) defaultDB = US_Settings::defaultDB().at( 0 );
  update_lw( defaultDB );

  le_description       ->setText("");
  le_username          ->setText("");
  le_password          ->setText("");
  le_dbname            ->setText("");
  le_host              ->setText("");
  le_investigator_email->setText("");
  le_investigator_pw   ->setText("");
  uuid = "";

  pb_save       ->setEnabled( false );
  pb_delete     ->setEnabled( false );
}

void US_Database::help( void )
{
  US_Help* showhelp = new US_Help(this);
  showhelp->show_help( "database_config.html" );
}
  
void US_Database::save_default( void )
{
  for ( int i = 0; i < dblist.size(); i++ )
  {
    QString desc = dblist.at( i ).at( 0 );
     
    // Look for the current description
    if ( desc == le_description->text() ) 
    {
      if ( dblist.at( i ).at( 9 ) == "" )
      {
         QMessageBox::information( this, 
             tr( "Default Database Problem" ),
             tr( "The current database information has not been tested "
                  "for connectivity.\n" 
                  "The default database has not been updated.") );
         return;
      }

      QString null = "";

      US_Settings::set_defaultDB( dblist.at( i ) );
      US_Settings::set_us_inv_name( null );
      US_Settings::set_us_inv_ID( -1 );
      reset();
      
      QMessageBox::information( this, 
          tr( "Default Database" ),
          tr( "The default database has been updated." ) );
      return;
    }
  }
     
  QMessageBox::warning( this, 
      tr( "Default Database Problem" ),
      tr( "The description does not match any in the database list.\n"
          "The default database has not been updated." ) );
}

void US_Database::deleteDB( void )
{
  QString item = le_description->text();

  // Go through list and delete the one matching description
  for ( int i = 0; i < dblist.size(); i++ )
  {
    QString desc = dblist.at( i ).at( 0 );
     
    // Look for the current description
    if ( desc == item ) 
    {
      dblist.removeAt( i );
      US_Settings::set_databases( dblist );
      
      // Check if the default DB matches
      QStringList defaultDB = US_Settings::defaultDB();
      if ( defaultDB.at( 0 ) == item )
        US_Settings::set_defaultDB( QStringList() );
      
      reset();

      QMessageBox::information( this, 
          tr( "Database Removed" ),
          tr( "The database has been removed." ) );
      return;
    }
  }
     
  QMessageBox::warning( this, 
      tr( "Database Problem" ),
      tr( "The description does not match any in the database list.\n"
          "The database has not been removed." ) );
}

bool US_Database::test_connect( void )
{
   QString error;
   US_DB2  db;
   bool ok = db.test_secure_connection( 
               le_host              ->text(), le_dbname         ->text(), 
               le_username          ->text(), le_password       ->text(), 
               le_investigator_email->text(), le_investigator_pw->text(), 
               error );

   if ( ok ) 
   {
      uuid = db.value( 0 ).toString();   // Set class variable uuid

      // Make sure the uuid is updated in the dblist structure
      for ( int i = 0; i < dblist.size(); i++ )
      {
         QString desc = dblist.at( i ).at( 0 );
           
         // Look for the current description
         if ( desc == le_description->text() )
         {
            QStringList list = dblist.at( i );
            list.replace( 9, uuid );
            dblist.replace( i, list );
            US_Settings::set_databases( dblist );
            break;
         }
      }

      QMessageBox::information( this,
        tr( "Database Connectiom" ),
        tr( "The connection was successful." ) );
   }
   else
      QMessageBox::warning( this,
        tr( "Database Connectiom" ),
        tr( "The connection failed.\n" ) + error );

   return ok;
}
