// test_us_crypto_unit.cpp - Unit tests for US_Crypto class
#include "qt_test_base.h"
#include "us_crypto.h"
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QTime>
#include <set>

using namespace qt_matchers;

class TestUSCryptoUnit : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
    }

    void TearDown() override {
        QtTestBase::TearDown();
    }

    // Helper method to validate encryption result structure
    bool isValidEncryptionResult(const QStringList& result) {
        return result.size() >= 2 &&
               !result.at(0).isEmpty() &&
               !result.at(1).isEmpty();
    }

    // Helper method to validate hex string
    bool isValidHexString(const QString& str) {
        if (str.isEmpty()) return false;
        for (QChar c : str) {
            if (!c.isDigit() && c.toLower() < 'a' && c.toLower() > 'f') {
                return false;
            }
        }
        return str.length() % 2 == 0; // Hex strings should have even length
    }
};

// ============================================================================
// ENCRYPT METHOD TESTS
// ============================================================================

TEST_F(TestUSCryptoUnit, EncryptEmptyPlainText) {
// Test encrypt with empty plain text
QString plainText = "";
QString password = "password123";

QStringList result = US_Crypto::encrypt(plainText, password);

EXPECT_TRUE(isValidEncryptionResult(result))
<< "Encrypt should return valid result structure for empty text";
EXPECT_TRUE(isValidHexString(result.at(0)))
<< "Cipher text should be valid hex string";
EXPECT_TRUE(isValidHexString(result.at(1)))
<< "IV should be valid hex string";
EXPECT_EQ(result.at(1).length(), 32)
<< "IV should be 32 hex characters (16 bytes)";
}

TEST_F(TestUSCryptoUnit, EncryptEmptyPassword) {
// Test encrypt with empty password
QString plainText = "secret message";
QString password = "";

QStringList result = US_Crypto::encrypt(plainText, password);

EXPECT_TRUE(isValidEncryptionResult(result))
<< "Encrypt should handle empty password";
EXPECT_TRUE(isValidHexString(result.at(0)))
<< "Cipher text should be valid hex string";
EXPECT_TRUE(isValidHexString(result.at(1)))
<< "IV should be valid hex string";
}

TEST_F(TestUSCryptoUnit, EncryptBothEmpty) {
// Test encrypt with both empty strings
QString plainText = "";
QString password = "";

QStringList result = US_Crypto::encrypt(plainText, password);

EXPECT_TRUE(isValidEncryptionResult(result))
<< "Encrypt should handle both empty inputs";
EXPECT_TRUE(isValidHexString(result.at(0)))
<< "Cipher text should be valid hex string";
EXPECT_TRUE(isValidHexString(result.at(1)))
<< "IV should be valid hex string";
}

TEST_F(TestUSCryptoUnit, EncryptSingleCharacter) {
// Test encrypt with single character
QString plainText = "a";
QString password = "p";

QStringList result = US_Crypto::encrypt(plainText, password);

EXPECT_TRUE(isValidEncryptionResult(result))
<< "Encrypt should handle single character inputs";
EXPECT_TRUE(isValidHexString(result.at(0)))
<< "Cipher text should be valid hex string";
EXPECT_TRUE(isValidHexString(result.at(1)))
<< "IV should be valid hex string";
EXPECT_GT(result.at(0).length(), 0)
<< "Cipher text should not be empty";
}

TEST_F(TestUSCryptoUnit, EncryptLongPassword) {
// Test encrypt with password longer than 16 characters (key limit)
QString plainText = "test message";
QString password = "this_is_a_very_long_password_more_than_16_chars";

QStringList result = US_Crypto::encrypt(plainText, password);

EXPECT_TRUE(isValidEncryptionResult(result))
<< "Encrypt should handle long passwords";
EXPECT_TRUE(isValidHexString(result.at(0)))
<< "Cipher text should be valid hex string";
EXPECT_TRUE(isValidHexString(result.at(1)))
<< "IV should be valid hex string";
}

TEST_F(TestUSCryptoUnit, EncryptSpecialCharacters) {
// Test encrypt with special characters in plain text
QString plainText = "!@#$%^&*()_+-=[]{}|;:',.<>?";
QString password = "special_password";

QStringList result = US_Crypto::encrypt(plainText, password);

EXPECT_TRUE(isValidEncryptionResult(result))
<< "Encrypt should handle special characters";
EXPECT_TRUE(isValidHexString(result.at(0)))
<< "Cipher text should be valid hex string";
EXPECT_TRUE(isValidHexString(result.at(1)))
<< "IV should be valid hex string";
}

TEST_F(TestUSCryptoUnit, EncryptSpecialCharactersInPassword) {
// Test encrypt with special characters in password
QString plainText = "test message";
QString password = "!@#$%^&*()_+-=";

QStringList result = US_Crypto::encrypt(plainText, password);

EXPECT_TRUE(isValidEncryptionResult(result))
<< "Encrypt should handle special characters in password";
EXPECT_TRUE(isValidHexString(result.at(0)))
<< "Cipher text should be valid hex string";
EXPECT_TRUE(isValidHexString(result.at(1)))
<< "IV should be valid hex string";
}

TEST_F(TestUSCryptoUnit, EncryptNumbersOnly) {
// Test encrypt with numeric strings
QString plainText = "1234567890";
QString password = "9876543210";

QStringList result = US_Crypto::encrypt(plainText, password);

EXPECT_TRUE(isValidEncryptionResult(result))
<< "Encrypt should handle numeric strings";
EXPECT_TRUE(isValidHexString(result.at(0)))
<< "Cipher text should be valid hex string";
EXPECT_TRUE(isValidHexString(result.at(1)))
<< "IV should be valid hex string";
}

TEST_F(TestUSCryptoUnit, EncryptIVUniqueness) {
// Test that encryption produces unique IVs
QString plainText = "same message";
QString password = "same password";
std::set<QString> ivs;

// Generate multiple encryptions
for (int i = 0; i < 10; i++) {
QStringList result = US_Crypto::encrypt(plainText, password);
ASSERT_TRUE(isValidEncryptionResult(result))
<< "Encryption " << i << " should succeed";

QString iv = result.at(1);
EXPECT_EQ(ivs.find(iv), ivs.end())
<< "IV should be unique for each encryption";
ivs.insert(iv);
}

EXPECT_EQ(ivs.size(), 10u)
<< "All IVs should be unique";
}

TEST_F(TestUSCryptoUnit, EncryptDifferentPasswordsProduceDifferentResults) {
// Test that different passwords produce different cipher texts
QString plainText = "same message";
QString password1 = "password1";
QString password2 = "password2";

QStringList result1 = US_Crypto::encrypt(plainText, password1);
QStringList result2 = US_Crypto::encrypt(plainText, password2);

EXPECT_TRUE(isValidEncryptionResult(result1))
<< "First encryption should succeed";
EXPECT_TRUE(isValidEncryptionResult(result2))
<< "Second encryption should succeed";

// Different passwords should produce different results (highly likely)
EXPECT_NE(result1.at(0), result2.at(0))
<< "Different passwords should produce different cipher texts";
}

// ============================================================================
// DECRYPT METHOD TESTS
// ============================================================================

TEST_F(TestUSCryptoUnit, DecryptEmptyPassword) {
// Test decrypt with empty password should return empty string
QString ciphertext = "deadbeef";
QString password = "";
QString initVector = "1234567890abcdef1234567890abcdef";

QString result = US_Crypto::decrypt(ciphertext, password, initVector);

EXPECT_TRUE(result.isEmpty())
<< "Decrypt with empty password should return empty string";
}

TEST_F(TestUSCryptoUnit, DecryptEmptyCiphertext) {
// Test decrypt with empty ciphertext
QString ciphertext = "";
QString password = "password123";
QString initVector = "1234567890abcdef1234567890abcdef";

// This may cause issues with OpenSSL, but we test the behavior
QString result = US_Crypto::decrypt(ciphertext, password, initVector);

// Result behavior is implementation-dependent, just ensure no crash
SUCCEED() << "Decrypt with empty ciphertext completed without crash";
}

TEST_F(TestUSCryptoUnit, DecryptEmptyInitVector) {
// Test decrypt with empty init vector
QString ciphertext = "deadbeef";
QString password = "password123";
QString initVector = "";

// This will likely cause issues with OpenSSL
QString result = US_Crypto::decrypt(ciphertext, password, initVector);

// Just ensure no crash occurs
SUCCEED() << "Decrypt with empty IV completed without crash";
}

TEST_F(TestUSCryptoUnit, DecryptInvalidHexCiphertext) {
// Test decrypt with invalid hex characters in ciphertext
QString ciphertext = "invalidhexstring!@#";
QString password = "password123";
QString initVector = "1234567890abcdef1234567890abcdef";

QString result = US_Crypto::decrypt(ciphertext, password, initVector);

// Should handle gracefully (behavior may vary)
SUCCEED() << "Decrypt with invalid hex completed without crash";
}

TEST_F(TestUSCryptoUnit, DecryptInvalidHexInitVector) {
// Test decrypt with invalid hex characters in IV
QString ciphertext = "deadbeef";
QString password = "password123";
QString initVector = "invalid!@#$%^";

QString result = US_Crypto::decrypt(ciphertext, password, initVector);

// Should handle gracefully
SUCCEED() << "Decrypt with invalid IV hex completed without crash";
}

TEST_F(TestUSCryptoUnit, DecryptOddLengthHex) {
// Test decrypt with odd-length hex strings
QString ciphertext = "deadbee"; // Odd length
QString password = "password123";
QString initVector = "1234567890abcdef1234567890abcde"; // Odd length

QString result = US_Crypto::decrypt(ciphertext, password, initVector);

// Should handle gracefully
SUCCEED() << "Decrypt with odd-length hex completed without crash";
}

TEST_F(TestUSCryptoUnit, DecryptWrongIVLength) {
// Test decrypt with wrong IV length (should be 32 hex chars for 16 bytes)
QString ciphertext = "deadbeef";
QString password = "password123";
QString initVector = "1234"; // Too short

QString result = US_Crypto::decrypt(ciphertext, password, initVector);

SUCCEED() << "Decrypt with short IV completed without crash";
}

TEST_F(TestUSCryptoUnit, DecryptLongPassword) {
// Test decrypt with password longer than 16 characters
QString ciphertext = "deadbeef";
QString password = "this_is_a_very_long_password_more_than_16_chars";
QString initVector = "1234567890abcdef1234567890abcdef";

QString result = US_Crypto::decrypt(ciphertext, password, initVector);

SUCCEED() << "Decrypt with long password completed without crash";
}

TEST_F(TestUSCryptoUnit, DecryptSpecialCharactersInPassword) {
// Test decrypt with special characters in password
QString ciphertext = "deadbeef";
QString password = "!@#$%^&*()_+-=";
QString initVector = "1234567890abcdef1234567890abcdef";

QString result = US_Crypto::decrypt(ciphertext, password, initVector);

SUCCEED() << "Decrypt with special chars password completed without crash";
}

// ============================================================================
// BOUNDARY AND EDGE CASE TESTS
// ============================================================================

TEST_F(TestUSCryptoUnit, EncryptMaxPasswordLength) {
// Test with exactly 16 character password (key size limit)
QString plainText = "test message";
QString password = "exactly16charspw"; // Exactly 16 chars

QStringList result = US_Crypto::encrypt(plainText, password);

EXPECT_TRUE(isValidEncryptionResult(result))
<< "Encrypt should handle 16-character password";
EXPECT_TRUE(isValidHexString(result.at(0)))
<< "Cipher text should be valid hex string";
EXPECT_TRUE(isValidHexString(result.at(1)))
<< "IV should be valid hex string";
}

// ============================================================================
// METHOD CONSISTENCY TESTS
// ============================================================================

TEST_F(TestUSCryptoUnit, EncryptConsistencyWithSameInputs) {
// Note: This will fail because IVs are random, but tests method behavior
QString plainText = "test message";
QString password = "password123";

QStringList result1 = US_Crypto::encrypt(plainText, password);
QStringList result2 = US_Crypto::encrypt(plainText, password);

EXPECT_TRUE(isValidEncryptionResult(result1))
<< "First encryption should succeed";
EXPECT_TRUE(isValidEncryptionResult(result2))
<< "Second encryption should succeed";

// IVs should be different (randomized)
EXPECT_NE(result1.at(1), result2.at(1))
<< "IVs should be different for each encryption";

// Cipher texts should be different due to different IVs
EXPECT_NE(result1.at(0), result2.at(0))
<< "Cipher texts should be different due to different IVs";
}

TEST_F(TestUSCryptoUnit, DecryptConsistencyWithSameInputs) {
// Test that decrypt produces consistent results with same inputs
QString ciphertext = "deadbeef";
QString password = "password123";
QString initVector = "1234567890abcdef1234567890abcdef";

QString result1 = US_Crypto::decrypt(ciphertext, password, initVector);
QString result2 = US_Crypto::decrypt(ciphertext, password, initVector);

EXPECT_EQ(result1, result2)
<< "Decrypt should be consistent with same inputs";
}

TEST_F(TestUSCryptoUnit, EncryptResultStructureValidation) {
// Test that encrypt always returns proper structure
QString plainText = "validation test";
QString password = "test_password";

QStringList result = US_Crypto::encrypt(plainText, password);

EXPECT_GE(result.size(), 2)
<< "Encrypt result should have at least 2 elements";
EXPECT_FALSE(result.at(0).isEmpty())
<< "Cipher text should not be empty";
EXPECT_FALSE(result.at(1).isEmpty())
<< "IV should not be empty";
EXPECT_TRUE(isValidHexString(result.at(0)))
<< "Cipher text should be valid hex";
EXPECT_TRUE(isValidHexString(result.at(1)))
<< "IV should be valid hex";
EXPECT_EQ(result.at(1).length(), 32)
<< "IV should be exactly 32 hex characters";
}