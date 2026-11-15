//! \file test_us_crypto.cpp
#include "qt_test_base.h"
#include "us_crypto.h"

#include <QByteArray>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

namespace
{
// Reproduce with support/crypto_vectors.py (optional; not a test dependency).
// Synthetic compatibility vectors generated independently with Python hashlib
// and cryptography, not US_Crypto. PBKDF2-HMAC-SHA256: 200000 iterations, 32
// bytes. GCM: AES-256, no padding or AAD, salt=00..0f, nonce=10..1b, 16-byte
// tag. The API metadata is salt || nonce || tag. Both password and plaintext
// are UTF-8.
const QString gcmPassword   = QString::fromUtf8( "synthetic-p\xc3\xa4ssword" );
const QString gcmPlaintext  = QString::fromUtf8( "Synthetic caf\xc3\xa9 \xce\xbc credential" );
const QString gcmCiphertext = "990a0d43b7e714b7508bf7ce6de5d8b558466ae5ff54fca94fb3bb435d";
const QString gcmMetadata   = "000102030405060708090a0b0c0d0e0f101112131415161718"
                              "191a1b8eebc619a5266537e085f9ded6b188bf";

// Independent AES-128-CBC + PKCS#7 compatibility vector: password "legacy-pass"
// padded with zero bytes to 16 bytes, IV=00..0f, plaintext encoded as Latin-1.
const QString legacyCiphertext = "9f33de7171908fde0c12b6a1aa25a2ec";
const QString legacyIv         = "000102030405060708090a0b0c0d0e0f";

void          assertEnvelope( const QStringList& envelope, const QString& plaintext )
{
   ASSERT_EQ( envelope.size(), 2 );
   EXPECT_EQ( envelope[0].size(),
              plaintext.toUtf8().size() * 2 ); // GCM has no padding
   EXPECT_EQ( envelope[1].size(),
              88 ); // 16-byte salt + 12-byte nonce + 16-byte tag
   const QRegularExpression hex( "^[0-9a-f]*$" );
   EXPECT_TRUE( hex.match( envelope[0] ).hasMatch() );
   EXPECT_TRUE( hex.match( envelope[1] ).hasMatch() );
}
} // namespace

class TestUSCryptoUnit : public QtTestBase
{
};

TEST_F( TestUSCryptoUnit, Pbkdf2MatchesIndependentKnownAnswers )
{
   const struct
   {
         QByteArray  password;
         QByteArray  salt;
         const char* expected;
   } vectors[] = {
      { "password", "salt", "ca64cfe28ca5559c62fba4afcb19f26889a67d5b135e571bffb087647e01becd" },
      { QByteArray( "p\0w", 3 ),
        QByteArray( "salt\0value", 10 ),
        "0711d4c643db41082ab2133dc95257b95ee27351cd33ad6ca521cdc558feb48e" },
   };
   for ( const auto& vector : vectors )
   {
      SCOPED_TRACE( vector.expected );
      unsigned char key[32];
      ASSERT_TRUE( US_Crypto::deriveKeyPBKDF2_32( vector.password, vector.salt, key ) );
      EXPECT_EQ( QByteArray( reinterpret_cast<const char*>( key ), sizeof( key ) ).toHex(), QByteArray( vector.expected ) );
   }
}

TEST_F( TestUSCryptoUnit, DecryptsIndependentGcmUtf8KnownAnswer )
{
   EXPECT_EQ( US_Crypto::decrypt( gcmCiphertext, gcmPassword, gcmMetadata ), gcmPlaintext );
}

TEST_F( TestUSCryptoUnit, LegacyKeyPadsShortPasswordsAndTruncatesAtSixteenCharacters )
{
   unsigned char key[16];
   ASSERT_TRUE( US_Crypto::deriveLegacyKey_16( "legacy-pass", key ) );
   EXPECT_EQ( QByteArray( reinterpret_cast<const char*>( key ), sizeof( key ) ).toHex(),
              QByteArray( "6c65676163792d706173730000000000" ) );
   ASSERT_TRUE( US_Crypto::deriveLegacyKey_16( "0123456789abcdef-unused", key ) );
   EXPECT_EQ( QByteArray( reinterpret_cast<const char*>( key ), sizeof( key ) ), QByteArray( "0123456789abcdef" ) );
}

TEST_F( TestUSCryptoUnit, DecryptsIndependentLegacyCbcKnownAnswerAndDispatchesByIvLength )
{
   const QString expected = QString::fromLatin1( "Legacy caf\xe9" );
   EXPECT_EQ( US_Crypto::decryptLegacy( legacyCiphertext, "legacy-pass", legacyIv ), expected );
   EXPECT_EQ( US_Crypto::decrypt( legacyCiphertext, "legacy-pass", legacyIv ), expected );
}

TEST_F( TestUSCryptoUnit, LegacyDecryptKeepsEmbeddedNul )
{
   // Same key and IV as legacyCiphertext; plaintext "left\0right". Generated with
   // printf 'left\0right' | openssl enc -aes-128-cbc -K <legacy key> -iv <legacyIv>
   const QString ciphertext = "8dcde1eb79a0a00cae1c8b197d64d609";
   const QString expected   = QString::fromLatin1( "left\0right", 10 );
   const QString plaintext  = US_Crypto::decryptLegacy( ciphertext, "legacy-pass", legacyIv );
   EXPECT_EQ( plaintext.size(), 10 );
   EXPECT_EQ( plaintext, expected );
   EXPECT_EQ( US_Crypto::decrypt( ciphertext, "legacy-pass", legacyIv ), expected );
}

TEST_F( TestUSCryptoUnit, EmptyPasswordIsRejectedByEncryptionAndBothDecryptionFormats )
{
   EXPECT_TRUE( US_Crypto::encrypt( "synthetic secret", QString() ).isEmpty() );
   EXPECT_TRUE( US_Crypto::encrypt( QString(), QString() ).isEmpty() );
   EXPECT_TRUE( US_Crypto::decrypt( gcmCiphertext, QString(), gcmMetadata ).isEmpty() );
   EXPECT_TRUE( US_Crypto::decryptLegacy( legacyCiphertext, QString(), legacyIv ).isEmpty() );
}

TEST_F( TestUSCryptoUnit, EmptyPlaintextProducesTheExpectedEnvelope )
{
   const QStringList envelope = US_Crypto::encrypt( QString(), "synthetic password" );
   ASSERT_NO_FATAL_FAILURE( assertEnvelope( envelope, QString() ) );
   EXPECT_TRUE( envelope[0].isEmpty() );
   EXPECT_EQ( US_Crypto::decrypt( envelope[0], "synthetic password", envelope[1] ), QString() );
}

TEST_F( TestUSCryptoUnit, EncryptDecryptPreservesUtf8EmbeddedNulAndBlockBoundaries )
{
   const QString plaintexts[] = {
      QString( 1, 'x' ),
      QString( 15, 'x' ),
      QString( 16, 'x' ),
      QString( 17, 'x' ),
      QString( 4096, 'x' ),
      gcmPlaintext,
      QString::fromUtf8( "left\0right", 10 ),
   };
   // GCM passwords are not truncated at the legacy 16-character boundary.
   const QString password = "0123456789abcdef-synthetic-suffix";
   for ( const QString& plaintext : plaintexts )
   {
      SCOPED_TRACE( plaintext.toUtf8().toHex().constData() );
      const QStringList envelope = US_Crypto::encrypt( plaintext, password );
      ASSERT_NO_FATAL_FAILURE( assertEnvelope( envelope, plaintext ) );
      EXPECT_EQ( US_Crypto::decrypt( envelope[0], password, envelope[1] ), plaintext );
   }
}

TEST_F( TestUSCryptoUnit, GcmRejectsWrongPasswordIncludingChangesAfterLegacyKeyBoundary )
{
   EXPECT_TRUE( US_Crypto::decrypt( gcmCiphertext, "wrong synthetic password", gcmMetadata ).isEmpty() );
   const QString     password = "0123456789abcdef-synthetic-suffix";
   const QStringList envelope = US_Crypto::encrypt( "synthetic secret", password );
   ASSERT_NO_FATAL_FAILURE( assertEnvelope( envelope, "synthetic secret" ) );
   EXPECT_TRUE( US_Crypto::decrypt( envelope[0], "0123456789abcdef-different-suffix", envelope[1] ).isEmpty() );
}

TEST_F( TestUSCryptoUnit, GcmRejectsTamperingWithCiphertextSaltNonceAndTag )
{
   const QByteArray ciphertext        = QByteArray::fromHex( gcmCiphertext.toLatin1() );
   const QByteArray metadata          = QByteArray::fromHex( gcmMetadata.toLatin1() );
   QByteArray       changedCiphertext = ciphertext;
   changedCiphertext[0]               = static_cast<char>( changedCiphertext[0] ^ 1 );
   EXPECT_TRUE( US_Crypto::decrypt( changedCiphertext.toHex(), gcmPassword, gcmMetadata ).isEmpty() );
   for ( int offset : { 0, 16, 28 } ) // salt, nonce, authentication tag
   {
      SCOPED_TRACE( offset );
      QByteArray changed = metadata;
      changed[offset]    = static_cast<char>( changed[offset] ^ 1 );
      EXPECT_TRUE( US_Crypto::decrypt( gcmCiphertext, gcmPassword, changed.toHex() ).isEmpty() );
   }
}

TEST_F( TestUSCryptoUnit, RejectsUnsupportedDecodedMetadataLengths )
{
   // Sixteen bytes intentionally selects legacy CBC; it is not an invalid size.
   for ( int bytes : { 0, 1, 15, 17, 43, 45 } )
   {
      SCOPED_TRACE( bytes );
      const QString metadata = QByteArray( bytes, '\0' ).toHex();
      EXPECT_TRUE( US_Crypto::decrypt( gcmCiphertext, gcmPassword, metadata ).isEmpty() );
   }
}

TEST_F( TestUSCryptoUnit, GcmRejectsMissingOrTruncatedCiphertextWithAnExistingTag )
{
   EXPECT_TRUE( US_Crypto::decrypt( QString(), gcmPassword, gcmMetadata ).isEmpty() );
   EXPECT_TRUE( US_Crypto::decrypt( gcmCiphertext.left( gcmCiphertext.size() - 2 ), gcmPassword, gcmMetadata ).isEmpty() );
}

TEST_F( TestUSCryptoUnit, LegacyRejectsIncompleteCipherBlocksWithAValidIv )
{
   const QString truncated = legacyCiphertext.left( legacyCiphertext.size() - 2 );
   EXPECT_TRUE( US_Crypto::decryptLegacy( truncated, "legacy-pass", legacyIv ).isEmpty() );
   EXPECT_TRUE( US_Crypto::decrypt( truncated, "legacy-pass", legacyIv ).isEmpty() );
}

// This is a regression against accidental salt/nonce reuse, not a statistical
// claim about RNG quality. A random collision at these lengths is negligible.
TEST_F( TestUSCryptoUnit, SeparateEncryptionsUseFreshSaltAndNonce )
{
   const QString     plaintext = "synthetic secret";
   const QStringList first     = US_Crypto::encrypt( plaintext, gcmPassword );
   const QStringList second    = US_Crypto::encrypt( plaintext, gcmPassword );
   ASSERT_NO_FATAL_FAILURE( assertEnvelope( first, plaintext ) );
   ASSERT_NO_FATAL_FAILURE( assertEnvelope( second, plaintext ) );
   EXPECT_NE( first[1].left( 32 ), second[1].left( 32 ) );
   EXPECT_NE( first[1].mid( 32, 24 ), second[1].mid( 32, 24 ) );
   EXPECT_EQ( US_Crypto::decrypt( first[0], gcmPassword, first[1] ), plaintext );
   EXPECT_EQ( US_Crypto::decrypt( second[0], gcmPassword, second[1] ), plaintext );
}

TEST_F( TestUSCryptoUnit, DirectLegacyDecryptRejectsInvalidIvLengths )
{
   for ( int bytes : { 0, 1, 15, 17, 32 } )
   {
      SCOPED_TRACE( bytes );
      EXPECT_TRUE( US_Crypto::decryptLegacy( legacyCiphertext, "legacy-pass", QByteArray( bytes, 'x' ).toHex() ).isEmpty() );
   }
}

TEST_F( TestUSCryptoUnit, DirectLegacyDecryptRejectsEmptyAndPartialBlocks )
{
   for ( int bytes : { 0, 1, 15, 17, 31 } )
   {
      SCOPED_TRACE( bytes );
      EXPECT_TRUE( US_Crypto::decryptLegacy( QByteArray( bytes, 'x' ).toHex(), "legacy-pass", legacyIv ).isEmpty() );
   }
}
