#ifndef TEST_US_DB2_H
#define TEST_US_DB2_H

#include <QtTest/QtTest>
#include "us_db2.h"

class TestUSDB2 : public QObject
{
Q_OBJECT

private slots:
    void initTestCase();    // Called before the first test function is executed
    void cleanupTestCase(); // Called after the last test function is executed

    void testConstructor();
    void testConnection();
    void testSecureConnection();
};

#endif // TEST_US_DB2_H
