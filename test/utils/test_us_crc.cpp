#include "test_us_crc.h"

void TestUSCrc::initTestCase()
{
    // Initialization code here
}

void TestUSCrc::cleanupTestCase()
{
    // Cleanup code here
}

void TestUSCrc::testCRC32()
{
    QByteArray input = "Hello, Ultrascan!";
    quint32 initialCRC = 0xFFFFFFFF; // Initial CRC value
    quint32 expectedCRC = 0xF2D2D2D2; // Expected CRC value (you need to calculate this value)

    quint32 crcResult = US_Crc::crc32(initialCRC, reinterpret_cast<const unsigned char*>(input.constData()), input.size());

    QCOMPARE(crcResult, expectedCRC);
}

