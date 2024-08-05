#ifndef TEST_US_DATAIO_H
#define TEST_US_DATAIO_H

#include <QtTest/QtTest>
#include "us_dataIO.h"

class TestUSDataIO : public QObject
{
Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testPointCount();
    void testScanCount();
    void testXIndex();
    void testRadius();
    void testScWavelength();
    void testValue();
    void testReading();
    void testSetValue();
    void testStdDev();
    void testAverageTemperature();
    void testTemperatureSpread();
};

#endif // TEST_US_DATAIO_H
