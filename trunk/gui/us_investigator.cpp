//! \file us_investigator.cpp

#include "us_investigator.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_db.h"
#include "us_passwd.h"

//! Constructor

/*! Construct a new US_Investigator interface.
*/

US_Investigator::US_Investigator( bool signal, QWidget* p, 
   Qt::WindowFlags f ) : US_WidgetsDialog( p, f )
{
   signal_wanted = signal;

   setPalette( US_GuiSettings::frameColor() );
   setWindowTitle( tr( "Manage Investigators" ) );
   setAttribute( Qt::WA_DeleteOnClose );

   int row = 0;
   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing( 2 );

   //QLabel* lb_info = us_banner( tr( "Investigator Information" ) );
   //main->addWidget( lb_info, row++, 0, 1, 2 );

   QStringList DB = US_Settings::defaultDB();
   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   main->addWidget( lb_DB, row++, 0, 1, 2 );

   // Investigator ID
   QLabel* lb_invID = us_label( tr( "Investigator ID:" ) );
   main->addWidget( lb_invID, row, 0 );

   le_invID = us_lineedit();
   le_invID->setReadOnly( true );
   main->addWidget( le_invID, row++, 1 );

   // Last Name
   QLabel* lb_lname = us_label( tr( "Last Name:" ) );
   main->addWidget( lb_lname, row, 0 );

   le_lname = us_lineedit();
   connect( le_lname, SIGNAL( textChanged( const QString& ) ), 
                      SLOT  ( limit_names( const QString& ) ) );
   main->addWidget( le_lname, row++, 1 );

   // Pushbutton
   QPushButton* pb_queryDB = us_pushbutton( tr( "Query DB" ) );
   connect( pb_queryDB, SIGNAL( clicked() ), SLOT( queryDB() ) );
   main->addWidget( pb_queryDB, row++, 0, 1, 2 );

   // List widget
   lw_names = us_listwidget();
   connect( lw_names, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ), 
                      SLOT  ( get_inv_data     ( QListWidgetItem* ) ) );
   main->addWidget( lw_names, row, 0, 4, 2 );
   row += 4;

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
   main->addWidget( le_email, row++, 1 );

   // Button row 1
   QBoxLayout* buttons1 = new QHBoxLayout;
 
   QPushButton* pb_add = us_pushbutton( tr( "Add" ) );
   connect( pb_add, SIGNAL( clicked() ), SLOT( add() ) );
   buttons1->addWidget( pb_add );
 
   pb_update = us_pushbutton( tr( "Update" ), false );
   connect( pb_update, SIGNAL( clicked() ), SLOT( update() ) );
   buttons1->addWidget( pb_update );
   
   pb_delete = us_pushbutton( tr( "Delete" ), false );
   connect( pb_delete, SIGNAL( clicked() ), SLOT( del() ) );
   buttons1->addWidget( pb_delete );
    
   main->addLayout( buttons1, row++, 0, 1, 2 );

   // Button row 2
   QBoxLayout* buttons2 = new QHBoxLayout;
 
   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons2->addWidget( pb_reset );
 
   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons2->addWidget( pb_help );
   
   QPushButton* pb_close;

   if ( signal_wanted )
   {
      QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
      connect( pb_cancel, SIGNAL( clicked() ), SLOT( reject() ) );
      buttons2->addWidget( pb_cancel );

      pb_close = us_pushbutton( tr( "Accept" ) );
   }
   else
      pb_close = us_pushbutton( tr( "Close" ) );
   
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   buttons2->addWidget( pb_close );
    
   main->addLayout( buttons2, row, 0, 1, 2 );
   reset();
}

void US_Investigator::queryDB( void )
{
   QString query = "SELECT InvID, LastName, FirstName "
                   "FROM tblInvestigators ";

   if ( le_lname->text() != "" )
      query += "WHERE LastName LIKE '%" + le_lname->text() + "%' ";

   query += "ORDER BY LastName";

   reset();

   {
      US_DB     db;
      US_Passwd pw;
      QString   error;

      if ( ! db.open( pw.getPasswd(), error ) )
      {
         // Error message here
         qDebug() << "US_Investigator::queryDB: Could not open DB";
        
         return;
      }

      struct US_InvestigatorData data;
      
      db.query( query );

      while ( db.next() )
      {
         data.invID     = db.value( 0 ).toInt();
         data.lastName  = db.value( 1 ).toString();
         data.firstName = db.value( 2 ).toString();

         investigators << data;

         lw_names->addItem( new QListWidgetItem( 
            "InvID: (" + QString::number( data.invID ) + "), " + 
            data.lastName + ", " + data.firstName ) );
      }
   }

   QSqlDatabase::removeDatabase( "UltraScan" );
}

void US_Investigator::add( void )
{
   QMessageBox::information( this,
      tr( "Add function disabled" ),
      tr( "Will be implemented in conjunction with the new DB" ) );

   return;
   /*
   if ( ! check_fields() ) return; 

   switch( QMessageBox::information( this, 
      tr( "Add this entry?" ),
      tr( "Clicking 'Yes' will save the information\n"
          "for this investigator to the database." ),
      QMessageBox::Yes, QMessageBox::No ) )
   {
      case QMessageBox::Yes:
      {
            QString query = "INSERT INTO tblInvestigators SET "
               "FirstName='" + le_fname->text()   + "',"
               "LastName='"  + le_lname->text()   + "',"
               "Address='"   + le_address->text() + "',"
               "City='"      + le_city->text()    + "',"
               "State='"     + le_state->text()   + "',"
               "Zip='"       + le_zip->text()     + "',"
               "Phone='"     + le_phone->text()   + "',"
               "Email='"     + le_email->text();

         // Insert investigator information into tblAuth
         QDateTime now = QDateTime::currentDateTime();
         QSqlQuery target;
         QString query;
         
         str = "INSERT INTO tblAuth(InvestigatorID, Status, Classification, "
            "Balance, Activated, Userlevel, Signup, LastLogin, Password) VALUES(";
         
         str+= QString::number(newInvID)+", "; // InvID
         str+= "'new', 'Academic User', ";     // Status, Classification
         str+= "00000.00, 1, 0, ";             // Balance, Activated, Userlevel

         // Signup, Last Login
         str+= "'"+now.toString(Qt::ISODate)+"', '"+now.toString(Qt::ISODate)+"', ";
         
         // Password
         str+="'ffdbd2aadf002da88ffce9b31d3d7499');";
         target.exec(str);

         quit();
         break;
      }

      case 1:
         break;
   }
   */
}

void US_Investigator::update( void )
{
   QMessageBox::information( this,
      tr( "Update function disabled" ),
      tr( "Will be implemented in conjunction with the new DB" ) );

   return;
/*   if ( le_invID->text().isEmpty() )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "You must select an Investagator from the DB to update." ) );
      return;
   }

   if ( ! check_fields() ) return; 
   
   if ( ! changed() ) return;

   switch( QMessageBox::information( this, 
      tr( "Update the entry?" ),
      tr( "Clicking 'OK' will update the information\n"
          "for this investigator in the database." ),
      QMessageBox::Ok, QMessageBox::Cancel  ) )
      {
         case QMessageBox::Ok:
         {

            US_DB     db;
            US_Passwd pw;
            QString   error;

            if ( ! db.open( pw.getPasswd(), error ) )
            {
               // Error message here
               qDebug() << "US_Investigator::update: Could not open DB";
               return;
            }

            QString query = "UPDATE tblInvestigators SET "
               "FirstName='" + le_fname->text()   + "',"
               "LastName='"  + le_lname->text()   + "',"
               "Address='"   + le_address->text() + "',"
               "City='"      + le_city->text()    + "',"
               "State='"     + le_state->text()   + "',"
               "Zip='"       + le_zip->text()     + "',"
               "Phone='"     + le_phone->text()   + "',"
               "Email='"     + le_email->text()   + "',"
               "WHERE InvID=" + le_invID->text();
            
            db.query( query );
         }

         QSqlDatabase::removeDatabase( "UltraScan" );
         break;

         default:
            break;
      }
      */
}

void US_Investigator::del( void )
{
   QMessageBox::information( this,
      tr( "Delete function disabled" ),
      tr( "Will be implemented in conjunction with the new DB" ) );

   return;
/*   if ( le_invID->text() == "" )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "First select a database entry\n"
                "to be deleted from the database" ) );
      return;
   }

   switch( QMessageBox::information( this, 
      tr( "Do you want to delete this entry?" ),
      tr( "Clicking 'Yes' will delete the information from the database" ),
      QMessageBox::Yes, QMessageBox::No  ) )
   {
      case QMessageBox::Yes:
         {
            QSqlError::ErrorType errorType;
            
            QString error;
            QString query = "DELETE FROM tblInvestigators WHERE InvID=" 
               + le_invID->text();
            
            {
               US_DB                db;
               US_Passwd            pw;

               if ( ! db.open( pw.getPasswd(), error ) )
               {
                  // Error message here
                  qDebug() << "US_Investigator::update: Could not open DB";
                  return;
               }

               db.query( query );

               errorType = db.lastQueryErrorType();
               error     = db.lastQueryErrorText();
            }

            QSqlDatabase::removeDatabase( "UltraScan" );

            if ( errorType != QSqlError::NoError )
               QMessageBox::information( this,
                     tr( "Attention" ),
                     tr( "Delete failed.\n"
                         "Attempted to execute this command:\n\n" )
                     + query + "\n\n" +
                     tr( "Causing the following error:\n\n" )
                     + error );
            else
            {
               reset();
               queryDB();
            }

         }
         break;
         
      default:
         break;
   }*/
}

void US_Investigator::limit_names( const QString& s )
{
   lw_names->clear();
  
   for ( int i = 0; i < investigators.size(); i++ )
   {
      if ( investigators[ i ].lastName.contains( 
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

   QString query = "SELECT FirstName, LastName, Address, City,"
                   "State, Zip, Phone, Email "
                   "FROM tblInvestigators "
                   "WHERE InvID=" + invID;
   {
      US_DB     db;
      US_Passwd pw;
      QString   error;

      if ( ! db.open( pw.getPasswd(), error ) )
      {
         // Error message here
         qDebug() << "US_Investigator::get_inv_data: Could not open DB";
        
         return;
      }

      db.query( query );
      db.next();

      info.invID     = invID.toInt();
      info.firstName = db.value( 0 ).toString();
      info.lastName  = db.value( 1 ).toString();
      info.address   = db.value( 2 ).toString();
      info.city      = db.value( 3 ).toString();
      info.state     = db.value( 4 ).toString();
      info.zip       = db.value( 5 ).toString();
      info.phone     = db.value( 6 ).toString();
      info.email     = db.value( 7 ).toString();

      le_lname  ->disconnect();
      le_lname  ->setText( info.lastName  ); 
      connect( le_lname, SIGNAL( textChanged( const QString& ) ), 
                         SLOT  ( limit_names( const QString& ) ) );

      le_invID  ->setText( invID          ); 
      le_fname  ->setText( info.firstName ); 
      le_address->setText( info.address   ); 
      le_city   ->setText( info.city      ); 
      le_state  ->setText( info.state     ); 
      le_zip    ->setText( info.zip       ); 
      le_phone  ->setText( info.phone     ); 
      le_email  ->setText( info.email     ); 
   }

   QSqlDatabase::removeDatabase( "UltraScan" );

   pb_delete ->setEnabled( true );
   pb_update ->setEnabled( true );
}

void US_Investigator::close( void )
{
   if ( signal_wanted ) 
      emit investigator_accepted( le_invID->text().toInt(), 
            le_lname->text(), le_fname->text() );
   
   accept();
}

void US_Investigator::reset( void )
{
   le_invID  ->setText( "" );
   le_fname  ->setText( "" );
   le_lname  ->setText( "" );
   le_address->setText( "" );
   le_city   ->setText( "" );
   le_state  ->setText( "" );
   le_zip    ->setText( "" );
   le_phone  ->setText( "" );
   le_email  ->setText( "" );

   lw_names    ->clear();
   investigators.clear();

   pb_delete ->setEnabled( false );
   pb_update ->setEnabled( false );

   info.invID     = -1;
   info.lastName  = "";
   info.firstName = "";
   info.address   = "";
   info.city      = "";
   info.state     = "";
   info.zip       = "";
   info.phone     = "";
   info.email     = "";
   info.display   = "";
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


   if ( missing != "" )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "Update the following fiels before committing data:\n" ) +
               missing );

      return false;
   }

   return true;
}

bool US_Investigator::changed( void )
{
   if ( le_lname  ->text() != info.lastName  ) return true;
   if ( le_fname  ->text() != info.firstName ) return true;
   if ( le_address->text() != info.address   ) return true;
   if ( le_city   ->text() != info.city      ) return true;
   if ( le_state  ->text() != info.state     ) return true;
   if ( le_zip    ->text() != info.zip       ) return true;
   if ( le_phone  ->text() != info.phone     ) return true;
   if ( le_email  ->text() != info.email     ) return true;

   QMessageBox::information( this,
      tr( "Attention" ),
      tr( "Nothing has changed." ) );
   
   return false;
}
