#ifndef TEST_US_CRC_H
#define TEST_US_CRC_H

#include <QtTest/QtTest>
#include "us_crc.h"

class TestUSCrc : public QObject {
Q_OBJECT

private slots:
    void test_crc32_data();
    void test_crc32();
};

#endif // TEST_US_CRC_H
