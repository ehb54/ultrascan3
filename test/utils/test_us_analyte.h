#ifndef TEST_US_ANALYTE_H
#define TEST_US_ANALYTE_H

#include <QtTest/QtTest>
#include "us_analyte.h"

class TestUSAnalyte : public QObject
{
Q_OBJECT

private slots:
    void initTestCase();    // Called before the first test function is executed
    void cleanupTestCase(); // Called after the last test function is executed

    void testConstructor(); // Test the default constructor
    void testEqualityOperator(); // Test the equality operator
    void testLoad(); // Test loading an analyte using the public interface
    void testWrite(); // Test writing an analyte using the public interface
};

#endif // TEST_US_ANALYTE_H
