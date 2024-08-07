#include <QtTest/QtTest>
#include "us_crc.h"

class TestUSCrc : public QObject {

private slots:
    void test_crc32_data();
    void test_crc32();
};

void TestUSCrc::test_crc32_data() {
    QTest::addColumn<quint32>("initialCrc");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<quint32>("expectedCrc");

    QTest::newRow("empty") << quint32(0) << QByteArray("") << quint32(0);
    QTest::newRow("example1") << quint32(0) << QByteArray("123456789") << quint32(0xcbf43926);
    // Add more test cases as needed
}

void TestUSCrc::test_crc32() {
    QFETCH(quint32, initialCrc);
    QFETCH(QByteArray, data);
    QFETCH(quint32, expectedCrc);

    quint32 result = US_Crc::crc32(initialCrc, reinterpret_cast<const unsigned char*>(data.constData()), data.length());
    QCOMPARE(result, expectedCrc);
}