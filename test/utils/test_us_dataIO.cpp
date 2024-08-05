#include "test_us_dataIO.h"

void TestUSDataIO::initTestCase()
{
    // Initialization code here
}

void TestUSDataIO::cleanupTestCase()
{
    // Cleanup code here
}

void TestUSDataIO::testPointCount()
{
    US_DataIO::RawData data;
    QCOMPARE(data.pointCount(), 0);
}

void TestUSDataIO::testScanCount()
{
    US_DataIO::RawData data;
    QCOMPARE(data.scanCount(), 0);
}

void TestUSDataIO::testXIndex()
{
    US_DataIO::RawData data;
    data.xvalues << 1.0 << 2.0 << 3.0;
    QCOMPARE(data.xindex(2.0), 1);
}

void TestUSDataIO::testRadius()
{
    US_DataIO::RawData data;
    data.xvalues << 1.0 << 2.0 << 3.0;
    QCOMPARE(data.radius(1), 2.0);
}

void TestUSDataIO::testScWavelength()
{
    US_DataIO::RawData data;
    data.xvalues << 1.0 << 2.0 << 3.0;
    QCOMPARE(data.scWavelength(1), 2.0);
}

void TestUSDataIO::testValue()
{
    US_DataIO::RawData data;
    US_DataIO::Scan scan;
    scan.rvalues << 1.0 << 2.0 << 3.0;
    data.scanData << scan;
    QCOMPARE(data.value(0, 1), 2.0);
}

void TestUSDataIO::testReading()
{
    US_DataIO::RawData data;
    US_DataIO::Scan scan;
    scan.rvalues << 1.0 << 2.0 << 3.0;
    data.scanData << scan;
    QCOMPARE(data.reading(0, 1), 2.0);
}

void TestUSDataIO::testSetValue()
{
    US_DataIO::RawData data;
    US_DataIO::Scan scan;
    scan.rvalues << 1.0 << 2.0 << 3.0;
    data.scanData << scan;
    QVERIFY(data.setValue(0, 1, 4.0));
    QCOMPARE(data.value(0, 1), 4.0);
}

void TestUSDataIO::testStdDev()
{
    US_DataIO::RawData data;
    US_DataIO::Scan scan;
    scan.nz_stddev = true;
    scan.stddevs << 0.1 << 0.2 << 0.3;
    data.scanData << scan;
    QCOMPARE(data.std_dev(0, 1), 0.2);
}

void TestUSDataIO::testAverageTemperature()
{
    US_DataIO::RawData data;
    US_DataIO::Scan scan1, scan2;
    scan1.temperature = 20.0;
    scan2.temperature = 30.0;
    data.scanData << scan1 << scan2;
    QCOMPARE(data.average_temperature(), 25.0);
}

void TestUSDataIO::testTemperatureSpread()
{
    US_DataIO::RawData data;
    US_DataIO::Scan scan1, scan2;
    scan1.temperature = 20.0;
    scan2.temperature = 30.0;
    data.scanData << scan1 << scan2;
    QCOMPARE(data.temperature_spread(), 10.0);
}

