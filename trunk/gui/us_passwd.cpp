//! \file us_passwd.cpp
#include "us_passwd.h"
#include "us_settings.h"
#include "us_global.h"

QString US_Passwd::getPasswd( void  )
{
  // If the pw is in global memory, return it
  QString pw = g.passwd();
  if ( ! pw.isEmpty() ) return pw;

  // See if the master pasword has been set
  QByteArray currentHash = US_Settings::UltraScanPW();

  if ( currentHash.isEmpty() )
  {
    QMessageBox::information( this,
          tr( "Password Error" ),
          tr( "The Master Password has not been set." ) );

    return QString();
  }

  // Ask the user to input the password
  while ( true ) 
  {
    bool ok;
    pw = QInputDialog::getText( 
         this, 
         tr( "Master Password" ),
         tr( "Please input your Master Password" ),
         QLineEdit::Password, 
         QString(), 
         &ok );

    // If the user cancelled, return a null string
    if ( ! ok ) return QString();
    
    // Check the hash
    QByteArray hash =
          QCryptographicHash::hash( pw.toAscii(), QCryptographicHash::Sha1 );

    if ( hash == currentHash ) break;
    
    QMessageBox::information( this,
          tr( "Password Error" ),
          tr( "The password is incorrect." ) );
  }

  // Save the password in global memory and return
  g.setPasswd( pw );
  return pw;
}


