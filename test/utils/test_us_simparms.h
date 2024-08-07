#ifndef TEST_US_SIMPARMS_H
#define TEST_US_SIMPARMS_H

#include <QtTest/QtTest>
#include "us_simparms.h"
#include "us_constants.h"

class TestUSSimparms : public QObject
{
Q_OBJECT

private slots:
    void testConstructor();
    void testSetHardware_DB();
    void testSetHardware_Local();
};

#endif // TEST_US_SIMPARMS_H
