#ifndef TEST_US_CRC_H
#define TEST_US_CRC_H

#include <QtTest/QtTest>
#include "us_crc.h"

class TestUSCrc : public QObject
{
Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testCRC32();
};

#endif // TEST_US_CRC_H
