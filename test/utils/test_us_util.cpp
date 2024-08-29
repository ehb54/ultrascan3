#include "test_us_util.h"

// Initialize the test case
void TestUSUtil::initTestCase()
{
    // Initialization before any test function is called
}

// Cleanup after all test functions are called
void TestUSUtil::cleanupTestCase()
{
    // Cleanup after all test functions are called
}

// Test the getToken function
void TestUSUtil::testGetToken()
{
    QString str = "token1,token2,token3";
    QString separator = ",";
    QString token;

    token = US_Util::getToken(str, separator);
    QCOMPARE(token, QString("token1"));
    QCOMPARE(str, QString("token2,token3"));

    token = US_Util::getToken(str, separator);
    QCOMPARE(token, QString("token2"));
    QCOMPARE(str, QString("token3"));

    token = US_Util::getToken(str, separator);
    QCOMPARE(token, QString("token3"));
    QCOMPARE(str, QString(""));

    token = US_Util::getToken(str, separator);
    QCOMPARE(token, QString(""));
    QCOMPARE(str, QString(""));
}

// Test the new_guid function
void TestUSUtil::testNewGuid()
{
    QString guid = US_Util::new_guid();
    QCOMPARE(guid.length(), 36);
    QVERIFY(guid.contains("-"));
}

// Test the md5sum_file function
void TestUSUtil::testMd5sumFile()
{
    QString testFile = "test.txt";
    QFile file(testFile);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream out(&file);
        out << "Hello, world!";
        file.close();
    }

    QString result = US_Util::md5sum_file(testFile);
    QCOMPARE(result.split(" ").size(), 2); // Check format "hash size"
    QVERIFY(result.split(" ")[1].toInt() > 0);

    QFile::remove(testFile); // Cleanup test file
}

// Test the toUTCDatetimeText function
void TestUSUtil::testToUTCDatetimeText()
{
    QString dttext = "2024-07-09T12:34:56";
    QString result = US_Util::toUTCDatetimeText(dttext, true);
    QCOMPARE(result, QString("2024-07-09 12:34:56 UTC"));

    dttext = "2024-07-09 12:34:56";
    result = US_Util::toUTCDatetimeText(dttext, false);
    QVERIFY(result.endsWith(" UTC"));
}



