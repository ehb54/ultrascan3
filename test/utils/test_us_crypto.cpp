#include "test_us_crypto.h"

void TestUSCrypto::initTestCase()
{
    // Initialization code here
}

void TestUSCrypto::cleanupTestCase()
{
    // Cleanup code here
}

void TestUSCrypto::testEncryptDecrypt()
{
    QString plainText = "Hello, Ultrascan!";
    QString password = "password123";

    // Encrypt the plain text
    QStringList encryptedData = US_Crypto::encrypt(plainText, password);
    QString cipherText = encryptedData.at(0);
    QString initVector = encryptedData.at(1);

    // Decrypt the cipher text
    QString decryptedText = US_Crypto::decrypt(cipherText, password, initVector);

    QCOMPARE(decryptedText, plainText);
}
