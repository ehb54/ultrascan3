//! \file us_crypto.cpp
#include "us_crypto.h"

#include <qrandom.h>
#include <random>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>

namespace {
   struct EvpCtxDeleter
   {
      void operator()(EVP_CIPHER_CTX* p) const { EVP_CIPHER_CTX_free( p ); }
   };
} // namespace

void US_Crypto::fillRandomBytes( QByteArray& ba )
{
   static QRandomGenerator rng = QRandomGenerator::securelySeeded();
   int i = 0;
   while ( i < ba.size() )
   {
      const quint32 r = rng.generate();
      for (int k = 0; k < 4 && i < ba.size(); ++k, ++i)
         ba[i] = static_cast<char>((r >> (8 * k)) & 0xFF);
   }
}

bool US_Crypto::deriveLegacyKey_16( const QString& pw, unsigned char key16[16] )
{
   memset( key16, 0, 16 );
   const int n = qMin( pw.size(), 16 );
   for ( int i = 0; i < n; ++i ) {
      key16[i] = static_cast<unsigned char>(pw[i].cell());
   }
   return true;
}

bool US_Crypto::deriveKeyPBKDF2_32( const QByteArray& pwUtf8, const QByteArray& salt, unsigned char key32[32] )
{
   constexpr int iterations = 200000; // tune if needed
   const int ok = PKCS5_PBKDF2_HMAC(
      pwUtf8.constData(),
      pwUtf8.size(),
      reinterpret_cast<const unsigned char*>(salt.constData()),
      salt.size(),
      iterations,
      EVP_sha256(),
      32,
      key32
   );
   return (ok == 1);
}

QStringList US_Crypto::encrypt( const QString& plain_text, const QString& pw )
{
   QStringList result;
   if ( pw.isEmpty() ) {
      return result;
   }

   const QByteArray plain_ba = plain_text.toUtf8();
   const QByteArray pw_ba    = pw.toUtf8();

   QByteArray salt( 16, '\0' );
   QByteArray iv  ( 12, '\0' );
   fillRandomBytes( salt );
   fillRandomBytes( iv );

   unsigned char key[32];
   // generate a key from password and a random salt
   if ( !deriveKeyPBKDF2_32( pw_ba, salt, key ) ) {
      return result;
   }

   std::unique_ptr<EVP_CIPHER_CTX, EvpCtxDeleter> ctx( EVP_CIPHER_CTX_new() );
   if ( !ctx ) {
      OPENSSL_cleanse(key, sizeof(key));
      return result;
   }

   int ok = 1;
   ok &= EVP_EncryptInit_ex(
      ctx.get(),
      EVP_aes_256_gcm(),
      nullptr,
      nullptr,
      nullptr
      );
   ok &= EVP_CIPHER_CTX_ctrl(
      ctx.get(),
      EVP_CTRL_GCM_SET_IVLEN,
      iv.size(),
      nullptr
      );
   ok &= EVP_EncryptInit_ex(
      ctx.get(),
      nullptr,
      nullptr,
      key,
      reinterpret_cast<const unsigned char*>(iv.constData())
   );

   QByteArray cipher;
   // GCM has no padding; ciphertext len == plaintext len
   cipher.resize(plain_ba.size()); 

   int outLen = 0;
   if ( ok == 1 ) {
      ok &= EVP_EncryptUpdate(
         ctx.get(),
         reinterpret_cast<unsigned char*>(cipher.data()),
         &outLen,
         reinterpret_cast<const unsigned char*>(plain_ba.constData()),
         plain_ba.size()
      );
   }

   int finalLen = 0;
   if ( ok == 1 ) {
      unsigned char finalBuf[EVP_MAX_BLOCK_LENGTH];
      ok &= EVP_EncryptFinal_ex( ctx.get(), finalBuf, &finalLen );
      if ( ok == 1 && finalLen != 0 ) {
         ok = 0;
      }
   }

   if ( ok != 1 ) {
      OPENSSL_cleanse( key, sizeof(key) );
      return QStringList();
   }

   cipher.resize(outLen);

   unsigned char tag[16];
   if ( EVP_CIPHER_CTX_ctrl( ctx.get(), EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag ) != 1 ) {
      OPENSSL_cleanse( key, sizeof(key) );
      return QStringList();
   }

   // Pack metadata into the "initVector" output to keep API shape:
   // meta = salt(16) || iv(12) || tag(16)
   QByteArray meta;
   meta.reserve( salt.size() + iv.size() + static_cast<int>(sizeof(tag)));
   meta.append( salt );
   meta.append( iv );
   meta.append( reinterpret_cast<const char*>(tag), static_cast<int>(sizeof(tag)) );

   OPENSSL_cleanse( key, sizeof(key) );

   result << cipher.toHex() << meta.toHex();
   return result;
}

QString US_Crypto::decryptLegacy( const QString& ciphertext, const QString& pw,
                            const QString& initVector )
{
   if ( pw.size() == 0 ) {
      return QString();
   }

   unsigned char key[16];
   if ( !deriveLegacyKey_16( pw, key ) ) {
      return QString();
   }

   QByteArray           iv_ba      = QByteArray::fromHex( initVector.toLatin1() );
   QByteArray           cipher_ba  = QByteArray::fromHex( ciphertext.toLatin1() );
   const unsigned char* iv_ptr     = reinterpret_cast<const unsigned char*>(iv_ba.constData());
   const unsigned char* cipher_ptr = reinterpret_cast<const unsigned char*>(cipher_ba.constData());

   QByteArray out;
   out.resize( cipher_ba.size() + EVP_CIPHER_block_size(EVP_aes_128_cbc()) );

   std::unique_ptr<EVP_CIPHER_CTX, EvpCtxDeleter> ctx( EVP_CIPHER_CTX_new() );
   if ( !ctx ) {
      OPENSSL_cleanse( key, sizeof(key) );
      return QString();
   }
   int ok = 1;
   ok &= EVP_DecryptInit_ex(
      ctx.get(),
      EVP_aes_128_cbc(),
      nullptr,
      key,
      iv_ptr
      );

   int ol = 0;
   if ( ok == 1 ) {
      ok &= EVP_DecryptUpdate(
         ctx.get(),
         reinterpret_cast<unsigned char*>(out.data()),
         &ol,
         cipher_ptr,
         cipher_ba.size()
      );
   }

   int ol_final = 0;
   if ( ok == 1 ) {
      ok &= EVP_DecryptFinal_ex(
         ctx.get(),
         reinterpret_cast<unsigned char*>(out.data()) + ol,
         &ol_final);
   }

   OPENSSL_cleanse( key, sizeof(key) );
   // wrong password / corrupted legacy data
   if ( ok != 1 ) {
      return QString();
   }

   out.resize( ol + ol_final );

   // Legacy encrypt used toLatin1(); keep legacy behavior on decode.
   return QString::fromLatin1( out );
}

QString US_Crypto::decrypt( const QString& ciphertext, const QString& pw,
                            const QString& initVector )
{
   if ( pw.isEmpty() ) {
      return QString();
   }

   const QByteArray pw_ba     = pw.toUtf8();
   const QByteArray cipher_ba = QByteArray::fromHex( ciphertext.toLatin1() );
   const QByteArray meta_ba   = QByteArray::fromHex( initVector.toLatin1() );

   // meta 16 bytes indicate legacy AES-128-CBC encryption
   if ( meta_ba.size() == 16 ) {
      // Legacy decryption
      return decryptLegacy( ciphertext, pw, initVector );
   }
   // Expect meta = 16(salt) + 12(iv) + 16(tag) = 44 bytes.
   if ( meta_ba.size() != 44 ) {
      return QString();
   }

   const QByteArray salt = meta_ba.left( 16 );
   const QByteArray iv   = meta_ba.mid( 16, 12 );
   const QByteArray tag  = meta_ba.mid( 28, 16 );

   unsigned char key[32];
   if ( !deriveKeyPBKDF2_32( pw_ba, salt, key ) ) {
      return QString();
   }

   std::unique_ptr<EVP_CIPHER_CTX, EvpCtxDeleter> ctx( EVP_CIPHER_CTX_new() );
   if ( !ctx ) {
      OPENSSL_cleanse( key, sizeof(key) );
      return QString();
   }

   int ok = 1;
   ok &= EVP_DecryptInit_ex(
      ctx.get(),
      EVP_aes_256_gcm(),
      nullptr,
      nullptr,
      nullptr);
   ok &= EVP_CIPHER_CTX_ctrl(
      ctx.get(),
      EVP_CTRL_GCM_SET_IVLEN,
      iv.size(),
      nullptr);
   ok &= EVP_DecryptInit_ex(
      ctx.get(),
      nullptr,
      nullptr,
      key,
      reinterpret_cast<const unsigned char*>(iv.constData())
   );

   QByteArray plain;
   plain.resize( cipher_ba.size() );

   int outLen = 0;
   if ( ok == 1 ) {
      ok &= EVP_DecryptUpdate(
         ctx.get(),
         reinterpret_cast<unsigned char*>(plain.data()),
         &outLen,
         reinterpret_cast<const unsigned char*>(cipher_ba.constData()),
         cipher_ba.size()
      );
   }

   // Set the expected tag BEFORE finalizing (auth check happens in Final).
   if ( ok == 1 ) {
      ok &= EVP_CIPHER_CTX_ctrl(
         ctx.get(),
         EVP_CTRL_GCM_SET_TAG,
         tag.size(),
         const_cast<char*>(tag.data())
      );
   }

   int finalLen = 0;
   if ( ok == 1 ) {
      unsigned char finalBuf[EVP_MAX_BLOCK_LENGTH];
      ok &= EVP_DecryptFinal_ex( ctx.get(), finalBuf, &finalLen );
      if ( ok == 1 && finalLen != 0 ) {
         ok = 0;
      }
   }

   OPENSSL_cleanse( key, sizeof(key) );

   if ( ok != 1 ) {
      // wrong password or tampered data
      return QString();
   }

   plain.resize( outLen );
   return QString::fromUtf8( plain );
}
