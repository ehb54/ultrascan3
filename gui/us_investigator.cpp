//! \file us_investigator.cpp

#include "us_investigator.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_db2.h"
#include "us_crypto.h"
#include "us_passwd.h"

/*! Construct a new US_Investigator interface.
*/

US_Investigator::US_Investigator( bool signal, int inv )
   : US_WidgetsDialog( 0, 0 )
{
  us_inv_auto_mode = false;
   signal_wanted = signal;

   setPalette( US_GuiSettings::frameColor() );
   setWindowTitle( tr( "Manage Investigators" ) );
   setAttribute( Qt::WA_DeleteOnClose );

   int row = 0;
   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing( 2 );

   // Search last name
   QLabel* lb_search = us_label( tr( "Search:" ) );
   main->addWidget( lb_search, row, 0 );

   le_search = us_lineedit();
   connect( le_search, SIGNAL( textChanged( const QString& ) ), 
                       SLOT  ( limit_names( const QString& ) ) );
   main->addWidget( le_search, row++, 1 );

   // List widget
   lw_names = us_listwidget();
   connect( lw_names, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ), 
                      SLOT  ( get_inv_data     ( QListWidgetItem* ) ) );
   main->addWidget( lw_names, row, 0, 4, 2 );
   row += 4;

   QStringList DB = US_Settings::defaultDB();
   if ( DB.isEmpty() ) DB << "Undefined";
   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   main->addWidget( lb_DB, row++, 0, 1, 2 );

   // Investigator ID
   QLabel* lb_invID = us_label( tr( "Investigator ID:" ) );
   main->addWidget( lb_invID, row, 0 );

   le_invID = us_lineedit();
   le_invID->setReadOnly( true );
   main->addWidget( le_invID, row++, 1 );

   // Investigator ID
   QLabel* lb_invGuid = us_label( tr( "Global Identifier:" ) );
   main->addWidget( lb_invGuid, row, 0 );

   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   le_invGuid = us_lineedit();
   le_invGuid->setReadOnly( true );
   le_invGuid->setPalette ( gray );
   main->addWidget( le_invGuid, row++, 1 );

   if ( US_Settings::us_debug() == 0 )
   {
      lb_invGuid->setVisible( false );
      le_invGuid->setVisible( false );
   }

   // Last Name
   QLabel* lb_lname = us_label( tr( "Last Name:" ) );
   main->addWidget( lb_lname, row, 0 );

   le_lname = us_lineedit();
   main->addWidget( le_lname, row++, 1 );

   // First Name
   QLabel* lb_fname = us_label( tr( "First Name:" ) );
   main->addWidget( lb_fname, row, 0 );

   le_fname = us_lineedit();
   main->addWidget( le_fname, row++, 1 );

   // Address
   QLabel* lb_address = us_label( tr( "Address:" ) );
   main->addWidget( lb_address, row, 0 );

   le_address = us_lineedit();
   main->addWidget( le_address, row++, 1 );

   // City
   QLabel* lb_city = us_label( tr( "City:" ) );
   main->addWidget( lb_city, row, 0 );

   le_city = us_lineedit();
   main->addWidget( le_city, row++, 1 );

   // State
   QLabel* lb_state = us_label( tr( "State:" ) );
   main->addWidget( lb_state, row, 0 );

   le_state = us_lineedit();
   main->addWidget( le_state, row++, 1 );

   // Zip
   QLabel* lb_zip = us_label( tr( "Zip:" ) );
   main->addWidget( lb_zip, row, 0 );

   le_zip = us_lineedit();
   main->addWidget( le_zip, row++, 1 );

   // Phone
   QLabel* lb_phone = us_label( tr( "Phone:" ) );
   main->addWidget( lb_phone, row, 0 );

   le_phone = us_lineedit();
   main->addWidget( le_phone, row++, 1 );

   // Email
   QLabel* lb_email = us_label( tr( "Email:" ) );
   main->addWidget( lb_email, row, 0 );

   le_email = us_lineedit();

   // Make the line edit entries a little wider than the default
   QFontMetrics fm( le_email->font() );
   le_email->setMinimumWidth( fm.maxWidth() * 10 );

   main->addWidget( le_email, row++, 1 );

   // Organization
   QLabel* lb_org = us_label( tr( "Organization:" ) );
   main->addWidget( lb_org, row, 0 );

   le_org = us_lineedit();
   main->addWidget( le_org, row++, 1 );

   // Pushbuttons
   QHBoxLayout* buttons1 = new QHBoxLayout;
   pb_queryDB = us_pushbutton( tr( "Query DB" ) );
   connect( pb_queryDB, SIGNAL( clicked() ), SLOT( queryDB() ) );
   buttons1->addWidget( pb_queryDB, row );

   pb_update = us_pushbutton( tr( "Update DB" ), false );
   connect( pb_update, SIGNAL( clicked() ), SLOT( update() ) );
   buttons1->addWidget( pb_update, row++ );
   main->addLayout( buttons1, row++, 0, 1, 2 );

   // Button row 
   QBoxLayout* buttons2 = new QHBoxLayout;
 
   pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons2->addWidget( pb_reset );
 
   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons2->addWidget( pb_help );
   
   int lev = US_Settings::us_inv_level();

   if ( signal_wanted )
   {
      QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
      connect( pb_cancel, SIGNAL( clicked() ), SLOT( reject() ) );
      buttons2->addWidget( pb_cancel );

      pb_close = us_pushbutton( tr( "Accept" ) );
      pb_close->setEnabled( user_permit  ||  lev > 2 );
   }
   else
      pb_close = us_pushbutton( tr( "Close" ) );
   
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   buttons2->addWidget( pb_close );
    
   main->addLayout( buttons2, row++, 0, 1, 2 );
   reset();
   queryDB();

   // Get last investigator ID if it is not passed
   // Will return -1 if it has not been set
   if ( inv < 0 ) inv = US_Settings::us_inv_ID();

   if ( inv > -1 )
   {
      for ( int i = 0; i < investigators.size(); i++ )
      {
         if ( investigators[ i ].invID == inv )
         {
            get_inv_data( lw_names->item( i ) );
            break;
         }
      }
   }

   user_permit   = false;

   // Disable GUI elements where login user not admin
   if ( lev < 3 )
   {
      pb_queryDB->setEnabled( false );
      pb_update ->setEnabled( false );
      pb_reset  ->setEnabled( false );
      le_search ->setEnabled( false );
   }
}

//Alternative Constructor for GMP:1.EXP.
US_Investigator::US_Investigator( QString auto_mode, bool signal, int inv )
   : US_WidgetsDialog( 0, 0 )
{
  us_inv_auto_mode = true;
   
  signal_wanted = signal;

   setPalette( US_GuiSettings::frameColor() );
   setWindowTitle( tr( "Manage Investigators" ) );
   setAttribute( Qt::WA_DeleteOnClose );

   int row = 0;
   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing( 2 );

   // Search last name
   QLabel* lb_search = us_label( tr( "Search:" ) );
   main->addWidget( lb_search, row, 0 );

   le_search = us_lineedit();
   connect( le_search, SIGNAL( textChanged( const QString& ) ), 
                       SLOT  ( limit_names( const QString& ) ) );
   main->addWidget( le_search, row++, 1 );

   // List widget
   lw_names = us_listwidget();
   connect( lw_names, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ), 
                      SLOT  ( get_inv_data     ( QListWidgetItem* ) ) );
   main->addWidget( lw_names, row, 0, 4, 2 );
   row += 4;

   QStringList DB = US_Settings::defaultDB();
   if ( DB.isEmpty() ) DB << "Undefined";
   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   main->addWidget( lb_DB, row++, 0, 1, 2 );

   // Investigator ID
   QLabel* lb_invID = us_label( tr( "Investigator ID:" ) );
   main->addWidget( lb_invID, row, 0 );

   le_invID = us_lineedit();
   le_invID->setReadOnly( true );
   main->addWidget( le_invID, row++, 1 );

   // Investigator ID
   QLabel* lb_invGuid = us_label( tr( "Global Identifier:" ) );
   main->addWidget( lb_invGuid, row, 0 );

   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   le_invGuid = us_lineedit();
   le_invGuid->setReadOnly( true );
   le_invGuid->setPalette ( gray );
   main->addWidget( le_invGuid, row++, 1 );

   if ( US_Settings::us_debug() == 0 )
   {
      lb_invGuid->setVisible( false );
      le_invGuid->setVisible( false );
   }

   // Last Name
   QLabel* lb_lname = us_label( tr( "Last Name:" ) );
   main->addWidget( lb_lname, row, 0 );

   le_lname = us_lineedit();
   main->addWidget( le_lname, row++, 1 );

   // First Name
   QLabel* lb_fname = us_label( tr( "First Name:" ) );
   main->addWidget( lb_fname, row, 0 );

   le_fname = us_lineedit();
   main->addWidget( le_fname, row++, 1 );

   // Address
   QLabel* lb_address = us_label( tr( "Address:" ) );
   main->addWidget( lb_address, row, 0 );

   le_address = us_lineedit();
   main->addWidget( le_address, row++, 1 );

   // City
   QLabel* lb_city = us_label( tr( "City:" ) );
   main->addWidget( lb_city, row, 0 );

   le_city = us_lineedit();
   main->addWidget( le_city, row++, 1 );

   // State
   QLabel* lb_state = us_label( tr( "State:" ) );
   main->addWidget( lb_state, row, 0 );

   le_state = us_lineedit();
   main->addWidget( le_state, row++, 1 );

   // Zip
   QLabel* lb_zip = us_label( tr( "Zip:" ) );
   main->addWidget( lb_zip, row, 0 );

   le_zip = us_lineedit();
   main->addWidget( le_zip, row++, 1 );

   // Phone
   QLabel* lb_phone = us_label( tr( "Phone:" ) );
   main->addWidget( lb_phone, row, 0 );

   le_phone = us_lineedit();
   main->addWidget( le_phone, row++, 1 );

   // Email
   QLabel* lb_email = us_label( tr( "Email:" ) );
   main->addWidget( lb_email, row, 0 );

   le_email = us_lineedit();

   // Make the line edit entries a little wider than the default
   QFontMetrics fm( le_email->font() );
   le_email->setMinimumWidth( fm.maxWidth() * 10 );

   main->addWidget( le_email, row++, 1 );

   // Organization
   QLabel* lb_org = us_label( tr( "Organization:" ) );
   main->addWidget( lb_org, row, 0 );

   le_org = us_lineedit();
   main->addWidget( le_org, row++, 1 );

   // Pushbuttons
   QHBoxLayout* buttons1 = new QHBoxLayout;
   pb_queryDB = us_pushbutton( tr( "Query DB" ) );
   connect( pb_queryDB, SIGNAL( clicked() ), SLOT( queryDB() ) );
   buttons1->addWidget( pb_queryDB, row );

   pb_update = us_pushbutton( tr( "Update DB" ), false );
   connect( pb_update, SIGNAL( clicked() ), SLOT( update() ) );
   buttons1->addWidget( pb_update, row++ );
   main->addLayout( buttons1, row++, 0, 1, 2 );

   // Button row 
   QBoxLayout* buttons2 = new QHBoxLayout;
 
   pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons2->addWidget( pb_reset );
 
   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons2->addWidget( pb_help );
   
   int lev = US_Settings::us_inv_level();

   if ( signal_wanted )
   {
      QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
      connect( pb_cancel, SIGNAL( clicked() ), SLOT( reject() ) );
      buttons2->addWidget( pb_cancel );

      pb_close = us_pushbutton( tr( "Accept" ) );
      //pb_close->setEnabled( user_permit  ||  lev > 2 );
   }
   else
      pb_close = us_pushbutton( tr( "Close" ) );
   
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   buttons2->addWidget( pb_close );
    
   main->addLayout( buttons2, row++, 0, 1, 2 );
   reset();
   queryDB();

   // Get last investigator ID if it is not passed
   // Will return -1 if it has not been set
   if ( inv < 0 ) inv = US_Settings::us_inv_ID();

   if ( inv > -1 )
   {
      for ( int i = 0; i < investigators.size(); i++ )
      {
         if ( investigators[ i ].invID == inv )
         {
            get_inv_data( lw_names->item( i ) );
            break;
         }
      }
   }

   //user_permit   = false;
   user_permit   = true;
   
   // // Disable GUI elements where login user not admin
   // if ( lev < 3 )
   // {
   //    pb_queryDB->setEnabled( false );
   //    pb_update ->setEnabled( false );
   //    pb_reset  ->setEnabled( false );
   //    le_search ->setEnabled( false );
   // }
}
// END of alternative Constructor ///////////////////////////////

void US_Investigator::override_permit( bool is_enab )
{
   user_permit   = is_enab;

   if ( us_inv_auto_mode )
     {
       user_permit = true;
       is_enab     = true;
     }

   if ( is_enab )
   {
      pb_queryDB->setEnabled( true );
      pb_update ->setEnabled( true );
      pb_reset  ->setEnabled( true );
      le_search ->setEnabled( true );
      pb_close  ->setEnabled( true );
   }
}

void US_Investigator::queryDB( void )
{
   US_Passwd   pw;
   QString     masterPW  = pw.getPasswd();
   US_DB2      db( masterPW );  // New constructor

   if ( db.lastErrno() != US_DB2::OK )
   {
      // Error message here
      QMessageBox::information( this,
         tr( "DB Connection Problem" ),
         tr( "There was an error connecting to the database:\n" ) 
             + db.lastError() );
      return;
   } 

   lw_names->clear();

   QStringList query;
   query << "get_people" << "%" + le_search->text() + "%"; 

   db.query( query );

   US_InvestigatorData data;
   int inv = US_Settings::us_inv_ID();
   int lev = US_Settings::us_inv_level();
qDebug() << "INV:qDB: inv" << inv << "lev" << lev;

   while ( db.next() )
   {
      data.invID     = db.value( 0 ).toInt();
      data.lastName  = db.value( 1 ).toString();
      data.firstName = db.value( 2 ).toString();

      // Only add to investigator list if admin login or login-inv match
      // if ( !user_permit  &&  lev < 3  &&  inv != data.invID )
      //    continue;

      if ( !user_permit  &&  lev < 3  &&  inv != data.invID && !us_inv_auto_mode )
	continue;

      investigators << data;

      lw_names->addItem( new QListWidgetItem( 
         "InvID: (" + QString::number( data.invID ) + "), " + 
         data.lastName + ", " + data.firstName ) );
   }
}

void US_Investigator::update( void )
{
   if ( le_invID->text().isEmpty() )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "You must select an Investigator from the DB to update." ) );
      return;
   }

   if ( ! check_fields() ) return; 
   
   if ( ! changed() ) return;

   int response = QMessageBox::question( this, 
      tr( "Update the entry?" ),
      tr( "Clicking 'OK' will update the information\n"
          "for this investigator in the database." ),
      QMessageBox::Ok, QMessageBox::Cancel );

   if ( response == QMessageBox::Ok )
   {
      US_Passwd   pw;
      QString     masterPW  = pw.getPasswd();
      US_DB2      db( masterPW );  // New constructor
      
      if ( db.lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this,
            tr( "Database Error" ),   
            tr( "US_Investigator::connectDB: Could not open DB\n" ) +
                db.lastError() );
         return;
      } 

      QStringList info = US_Settings::defaultDB();
      
      // DB Internal PW
      QString cipher = info.at( 7 );
      QString iv     = info.at( 8 );
      QString userPW = US_Crypto::decrypt( cipher, masterPW, iv );
      
      QStringList query;
      query << "update_person";
      query << le_invID  ->text();
      query << le_fname  ->text();
      query << le_lname  ->text();
      query << le_address->text();
      query << le_city   ->text();
      query << le_state  ->text();
      query << le_zip    ->text();
      query << le_phone  ->text();
      query << le_email  ->text();
      query << le_org    ->text();
      query << userPW;

      // Error check
      int status = db.statusQuery( query );

      if ( status == US_DB2::OK )
      {
         QMessageBox::information( this,
            tr( "Success" ),   
            tr( "The entry was updated.\n" ) );
      }
      else
      {
         QMessageBox::warning( this,
            tr( "Database Error" ),   
            tr( "US_Investigator::update_person: Could not update DB\n" ) +
                db.lastError() );
      }
   }
}

void US_Investigator::limit_names( const QString& s )
{
   lw_names->clear();
  
   for ( int i = 0; i < investigators.size(); i++ )
   {
      if ( investigators[ i ].lastName.contains( 
               QRegExp( ".*" + s + ".*", Qt::CaseInsensitive ) ) ||
           investigators[ i ].firstName.contains(
                              QRegExp( ".*" + s + ".*", Qt::CaseInsensitive ) ) )
         lw_names->addItem( new QListWidgetItem(
            "InvID: (" + QString::number( investigators[ i ].invID ) + "), " +
            investigators[ i ].lastName + ", " + 
            investigators[ i ].firstName ) );
   }
}

void US_Investigator::get_inv_data( QListWidgetItem* item )
{
   QString entry = item->text();

   
   int     left  = entry.indexOf( '(' ) + 1;
   int     right = entry.indexOf( ')' );
   QString invID = entry.mid( left, right - left );

   US_Passwd   pw;
   QString     masterPW  = pw.getPasswd();
   US_DB2      db( masterPW ); 

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
         tr( "DB Connection Problem" ),
         tr( "There was an error connecting to the database:\n" ) 
             + db.lastError() );
      return;
   } 

   QStringList query;
   query << "get_person_info" << invID; 
      
   db.query( query );
   db.next();

   info.invID        = invID.toInt();
   info.firstName    = db.value( 0 ).toString();
   info.lastName     = db.value( 1 ).toString();
   info.address      = db.value( 2 ).toString();
   info.city         = db.value( 3 ).toString();
   info.state        = db.value( 4 ).toString();
   info.zip          = db.value( 5 ).toString();
   info.phone        = db.value( 6 ).toString();
   info.organization = db.value( 7 ).toString();
   info.email        = db.value( 8 ).toString();
   info.invGuid      = db.value( 9 ).toString();

   //le_lname  ->disconnect();
   le_lname  ->setText( info.lastName  ); 
   //connect( le_search, SIGNAL( textChanged( const QString& ) ), 
   //                    SLOT  ( limit_names( const QString& ) ) );

   le_invID  ->setText( invID             ); 
   le_fname  ->setText( info.firstName    ); 
   le_address->setText( info.address      ); 
   le_city   ->setText( info.city         ); 
   le_state  ->setText( info.state        ); 
   le_zip    ->setText( info.zip          ); 
   le_phone  ->setText( info.phone        ); 
   le_email  ->setText( info.email        ); 
   le_org    ->setText( info.organization ); 
   le_invGuid->setText( info.invGuid      ); 

   pb_update ->setEnabled( US_Settings::us_inv_level() > 2  ||
                           user_permit );

}

void US_Investigator::close( void )
{
   if ( signal_wanted )
   {
      if ( le_invID->text() == "" )
      {
         QMessageBox::information( this,
            tr( "Attention" ),
            tr( "Select an investigator before 'Accept':\n" ) );

         return;
      }
   }

   if ( le_invID->text().toInt() > 0 )
   {
      QString s = le_lname->text() + ", " + le_fname->text();

      US_Settings::set_us_inv_name( s );
      US_Settings::set_us_inv_ID( le_invID->text().toInt() );
   }

   // Send signal *after* updating settings.
   if ( signal_wanted )
      emit investigator_accepted( le_invID->text().toInt() );

   US_WidgetsDialog::close();
}

void US_Investigator::reset( void )
{
   le_invID  ->setText( "" );
   le_invGuid->setText( "" );
   le_fname  ->setText( "" );
   le_lname  ->setText( "" );
   le_address->setText( "" );
   le_city   ->setText( "" );
   le_state  ->setText( "" );
   le_zip    ->setText( "" );
   le_phone  ->setText( "" );
   le_email  ->setText( "" );
   le_org    ->setText( "" );

   lw_names    ->clear();
   investigators.clear();

   //pb_delete ->setEnabled( false );
   pb_update ->setEnabled( false );

   info.invID        = -1;
   info.lastName     = "";
   info.firstName    = "";
   info.address      = "";
   info.city         = "";
   info.state        = "";
   info.zip          = "";
   info.phone        = "";
   info.email        = "";
   info.organization = "";
}

bool US_Investigator::check_fields( void )
{
   QString missing = "";
   QRegExp strip( "^\\s*(.*)\\s*$" );

   if ( le_lname->text().replace( strip, "\\1" ).isEmpty() )
      missing += tr( "\nLast Name" );

   if ( le_fname->text().replace( strip, "\\1" ).isEmpty() )
      missing += tr( "\\First Name" );

   if ( le_address->text().replace( strip, "\\1" ).isEmpty() )
      missing += tr( "\nAddress" );

   if ( le_city->text().replace( strip, "\\1" ).isEmpty() )
      missing += tr( "\nCity" );

   if ( le_state->text().replace( strip, "\\1" ).isEmpty() )
      missing += tr( "\nState" );

   if ( le_zip->text().replace( strip, "\\1" ).isEmpty() )
      missing += tr( "\nZip code" );

   if ( le_phone->text().replace( strip, "\\1" ).isEmpty() )
      missing += tr( "\nPhone" );

   if ( le_email->text().replace( strip, "\\1" ).isEmpty() )
      missing += tr( "\nEmail" );

   if ( le_org->text().replace( strip, "\\1" ).isEmpty() )
      missing += tr( "\nOfganization" );


   if ( missing != "" )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "Update the following fields before committing data:\n" ) +
               missing );

      return false;
   }

   return true;
}

bool US_Investigator::changed( void )
{
   if ( le_lname  ->text() != info.lastName     ) return true;
   if ( le_fname  ->text() != info.firstName    ) return true;
   if ( le_address->text() != info.address      ) return true;
   if ( le_city   ->text() != info.city         ) return true;
   if ( le_state  ->text() != info.state        ) return true;
   if ( le_zip    ->text() != info.zip          ) return true;
   if ( le_phone  ->text() != info.phone        ) return true;
   if ( le_email  ->text() != info.email        ) return true;
   if ( le_org    ->text() != info.organization ) return true;

   QMessageBox::information( this,
      tr( "Attention" ),
      tr( "Nothing has changed." ) );
   
   return false;
}
