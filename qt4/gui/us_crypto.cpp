//! \file us_crypto.cpp
#include <QtCrypto>

#include "us_crypto.h"
#include "us_passwd.h"

QStringList US_Crypto::encrypt( const QString& plaintext )
{
  
  US_Passwd  p;
  QByteArray masterPW = p.getPasswd().toLatin1();

  QStringList      result;
  QCA::Initializer init;

  if ( ! QCA::isSupported( "aes128-cbc-pkcs7" ) )
  {
    printf( "AES128-CBC not supported!\n" );
    return result;
  }

  // Create a random initialisation vector - we need this
  // value to decrypt the resulting cipher text, but it
  // need not be kept secret (unlike the key).
  QCA::InitializationVector initVector( 16 );

  // Create a 128 bit AES cipher object using Cipher Block Chaining (CBC) mode
  // Use Default padding, which is equivalent to PKCS7 for CBC
  QCA::Cipher cipher( QString( "aes128" ), 
                      QCA::Cipher::CBC,
                      QCA::Cipher::DefaultPadding,
                      QCA::Encode,
                      masterPW, 
                      initVector );

  cipher.update( QByteArray( plaintext.toLatin1() ) );
  QCA::SecureArray ciphertext = cipher.final();

  result << QCA::arrayToHex( ciphertext.toByteArray() )
         << QCA::arrayToHex( initVector.toByteArray() );

  return result;
}

QString US_Crypto::decrypt( const QString& ciphertext, const QString& initVector )
{
  US_Passwd  p;
  QByteArray masterPW = p.getPasswd().toLatin1();
  if ( masterPW.size() == 0 ) return QString();

  QCA::Initializer init;

  if ( ! QCA::isSupported( "aes128-cbc-pkcs7" ) )
  {
    printf( "AES128-CBC not supported!\n" );
    return QString();
  }

  QCA::Cipher cipher( 
                 QString( "aes128" ), 
                 QCA::Cipher::CBC,
                 QCA::Cipher::DefaultPadding,
                 QCA::Decode,
                 masterPW, 
                 QCA::InitializationVector( QCA::hexToArray( initVector ) ) );

  cipher.update( QCA::hexToArray( ciphertext ) );
  QCA::SecureArray plainText = cipher.final();

  return QString( plainText.data() );
}
