#ifndef TEST_US_CRYPTO_H
#define TEST_US_CRYPTO_H

#include <QtTest/QtTest>
#include "us_crypto.h"

class TestUSCrypto : public QObject
{
Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testEncryptDecrypt();
};

#endif // TEST_US_CRYPTO_H
