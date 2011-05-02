//! \file us_crypto.cpp
#include <QtCrypto>

#include "us_crypto.h"
#include <openssl/evp.h>

QStringList US_Crypto::encrypt( const QString& plain_text, const QString& pw )
{
   QStringList      result;

   // Seed the pseudo random number generator if needed
   static bool seed = false;

   if ( ! seed )
   {
      QTime time = QTime::currentTime();
      // Not a great amount of entropy, but probably good enough for us

      qsrand( (uint)time.msec() );
      seed = true;
   }

   // Determine the initialization vector 
   QByteArray iv_ba( 16, '0' );

   for ( int i = 0; i < 16; i++ )
      iv_ba[ i ] = qrand() % 256;

   uchar* iv_ptr = (uchar*)iv_ba.data();

   uchar key[ 16 ];  // The key is a 16 byte array 
   memset( key, 0, 16 );  // Zero it out
   
   for ( int i = 0; i < pw.size(); i++ ) // Copy the password
      key[ i ] = pw[ i ].cell();

   QString    plaintext  = plain_text;
   QByteArray plain_ba   = plaintext.toAscii();
   uchar*     plain_ptr  = (uchar*)plain_ba.data();

   EVP_CIPHER_CTX ctx;
   uchar          out[ 100 ];   // Assume the plaintext is < 99 characters
   int            out_length;
   int            final_length;

   EVP_EncryptInit  ( &ctx, EVP_aes_128_cbc(), key, iv_ptr );
   EVP_EncryptUpdate( &ctx, out, &out_length, plain_ptr, plaintext.size() );
   EVP_EncryptFinal ( &ctx, &out[ out_length ], &final_length );

   int c_size = out_length + final_length;

   QByteArray cipher_ba = QByteArray( (const char*)out, c_size );

   result << cipher_ba.toHex() << iv_ba.toHex();

   return result;
}
//////////////////////////////////////////////////////
QString US_Crypto::decrypt( const QString& ciphertext, const QString& pw,
                            const QString& initVector )
{
   if ( pw.size() == 0 ) return QString();

   uchar      key[ 16 ];  // The key is a 16 byte array 
   memset( key, 0, 16 );  // Zero it out

   for ( int i = 0; i < pw.size(); i++ ) // Copy the password
      key[ i ] = pw[ i ].cell();
            
   QByteArray     iv_ba    = QByteArray::fromHex( initVector.toAscii() );
   uchar*         iv_ptr   = (uchar*)iv_ba.data();

   QByteArray     cipher_ba  = QByteArray::fromHex( ciphertext.toAscii() );
   uchar*         cipher_ptr = (uchar*)cipher_ba.data();

   uchar          out   [ 100 ];   // Assume the plaintext is < 99 characters
   uchar          final [ 100 ];

   int            ol;
   EVP_CIPHER_CTX ctx;

   EVP_DecryptInit  ( &ctx, EVP_aes_128_cbc(), key, iv_ptr );
   EVP_DecryptUpdate( &ctx, out, &ol, cipher_ptr, cipher_ba.size() );
   EVP_DecryptFinal ( &ctx, final, &ol );

   QByteArray final_ba( (char*)final, ol );

   return final_ba;
}
