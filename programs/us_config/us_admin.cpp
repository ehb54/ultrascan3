//! \file us_admin.cpp
#include "us_admin.h"
#include "us_license_t.h"
#include "us_help.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_crypto.h"

US_Admin::US_Admin( QWidget* w, Qt::WindowFlags flags ) 
   : US_Widgets( true, w, flags )
{
  setWindowTitle( "Change Master Password" );
  setAttribute( Qt::WA_DeleteOnClose );

  int buttonh = 26;
  
  setPalette( US_GuiSettings::frameColor() );
  
  QLabel* header = us_banner( "Change Master Password" );
  header->setMinimumHeight( buttonh * 3 / 2 );
    
  QString oldPass = US_Settings::UltraScanPW();
  QLabel* oldPW = NULL;

  if ( ! oldPass.isEmpty() ) 
  {
    oldPW = us_label( "Old Password:" );
    oldPW->setMinimumHeight( buttonh );

    le_oldPasswd = us_lineedit( "" );
    le_oldPasswd->setEchoMode( QLineEdit::Password );
    le_oldPasswd->setMinimumHeight( buttonh );
  }
  else
    le_oldPasswd = NULL;

  QLabel* passwd1 = us_label( "Enter New Password:" );
  passwd1->setMinimumHeight( buttonh );
    
  le_passwd1 = us_lineedit( "" );
  le_passwd1->setEchoMode( QLineEdit::Password );
  le_passwd1->setMinimumHeight( buttonh );
  le_passwd1->setMinimumWidth( 300 );
  
  QLabel* passwd2 = us_label( "Verify New Password:" );
  passwd2->setMinimumHeight(buttonh);
  
  le_passwd2 = us_lineedit( "" );
  le_passwd2->setMinimumHeight( buttonh );
  le_passwd2->setEchoMode( QLineEdit::Password );
  
  pb_help = us_pushbutton( "Help" );
  pb_help->setMinimumHeight( buttonh );
  connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
  
  pb_save = us_pushbutton( "Save" );
  pb_save->setMinimumHeight( buttonh ); 
  connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
          
  pb_cancel = us_pushbutton( "Close" );
  pb_cancel->setMinimumHeight( buttonh );
  connect( pb_cancel, SIGNAL( clicked() ), SLOT( close() ) );

  // Layout
  QGridLayout* passwords = new QGridLayout;
  passwords->addWidget( header, 0, 0, 1, 2 );

  int row = 1;
  if ( ! oldPass.isEmpty() )
  {
    passwords->addWidget( oldPW       , row,   0 );
    passwords->addWidget( le_oldPasswd, row++, 1 );
  }

  passwords->addWidget( passwd1   , row,   0 );
  passwords->addWidget( le_passwd1, row++, 1 );
  passwords->addWidget( passwd2   , row,   0 );
  passwords->addWidget( le_passwd2, row++, 1 );
  
  QHBoxLayout* buttonLine = new QHBoxLayout;
  buttonLine->addWidget( pb_help   );
  buttonLine->addWidget( pb_save   );
  buttonLine->addWidget( pb_cancel );
  
  QVBoxLayout* main = new QVBoxLayout;
  main->setSpacing( 2 );
  main->addLayout( passwords );
  main->addLayout( buttonLine );

  setLayout( main );
}

void US_Admin::save( void )
{
  QByteArray oldPW = US_Settings::UltraScanPW();

  if ( ! oldPW.isEmpty() ) 
  {
    QByteArray calcsha1 = 
      QCryptographicHash::hash( le_oldPasswd->text().toLatin1(), 
          QCryptographicHash::Sha1 );

    if ( calcsha1 != oldPW )
    {
      QMessageBox::information( this, 
        tr( "Attention:" ), 
        tr( "The old password is incorrect. Please re-input.\n" ) );
      return;
    }
  }

  if ( le_passwd1->text() != le_passwd2->text() )
  {
    le_passwd1->setText("");
    le_passwd2->setText("");

    QMessageBox::information( this, 
        tr( "Attention:" ), 
        tr( "The entered passwords are not same. Please re-input.\n" ) );
    return;
  }

  QString newPW = le_passwd1->text();

  if ( newPW.isEmpty() )
  {
    QMessageBox::information( this, 
        tr( "Attention:" ), 
        tr( "The new password cannot be empty. Please re-input.\n" ) );
    return;
  }


  // We need to reset any passwords in databases here
  // from le_oldPasswd->text()
  // to   le_passwd1->text()

  QStringList defaultDB = US_Settings::defaultDB();
  QString     oldPass;
  
  if ( le_oldPasswd != NULL ) oldPass = le_oldPasswd->text();

  // Helper lambdas: safely re-encrypt one record
  auto reencryptRecord = [&](QStringList& rec) -> bool
  {
    // Expect: 4=cipher, 5=iv/meta (db password), 7=cipher, 8=iv/meta (investigator password)
    if ( rec.size() < 6 ) {
      // nothing to do / malformed list handled elsewhere
      return true;
    }
    // reencrypt database password
    const QString cipherHex = rec.at( 4 );
    const QString ivHex     = rec.at( 5 );
    if ( cipherHex.isEmpty() ) {
        return true;
    }

    const QString plain = US_Crypto::decrypt( cipherHex, oldPass, ivHex );

    // If there was something encrypted, decryption must succeed; otherwise abort.
    if ( !cipherHex.isEmpty() && plain.isEmpty() ) {
      return false;
    }

    const QStringList newCt = US_Crypto::encrypt( plain, newPW );
    if ( newCt.size() < 2 ) {
      return false;
    }

    rec.replace(4, newCt.at(0));
    rec.replace(5, newCt.at(1));

    if ( rec.size() < 9 ) {
      return true;
    }

    // reencrypt user password if needed
    const QString cipherHex2 = rec.at( 7 );
    const QString ivHex2     = rec.at( 8 );

    const QString plain2 = US_Crypto::decrypt( cipherHex2, oldPass, ivHex2 );

    // If there was something encrypted, decryption must succeed; otherwise abort.
    if ( !cipherHex2.isEmpty() && plain2.isEmpty() ) {
      return false;
    }

    const QStringList newCt2 = US_Crypto::encrypt( plain2, newPW );
    if ( newCt2.size() < 2 ) {
      return false;
    }

    rec.replace(7, newCt2.at(0));
    rec.replace(8, newCt2.at(1));

    return true;
  };

  if ( !defaultDB.empty() )
  {
    if ( !reencryptRecord( defaultDB ) ) {
      QMessageBox::information(this, tr("Attention:"),
    tr("Password change failed: could not decrypt an existing entry.\n"
     "No entries were modified."));
      return;
    }
  }

  QList<QStringList> databases = US_Settings::databases();

  for ( auto& database : databases )
  {
    if ( !reencryptRecord( database ) ) {
      QMessageBox::information(this, tr("Attention:"),
    tr("Password change failed: could not decrypt an existing entry.\n"
     "No entries were modified."));
      return;
    }
  }

  /// commit updated settings

  QByteArray sha1string =
    QCryptographicHash::hash( newPW.toLatin1(), QCryptographicHash::Sha1 );

  US_Settings::set_UltraScanPW( sha1string );

  if ( !defaultDB.empty() ) {
    US_Settings::set_defaultDB( defaultDB );
  }

  if ( !databases.empty() ) {
    US_Settings::set_databases( databases );
  }
  
  g.setPasswd( newPW );
  close();
}

void US_Admin::help()
{
  US_Help* online_help = new US_Help( this );
  online_help->show_help( "manual/us_administrator.html" );
}

