#ifndef TESTUSMATH2_H
#define TESTUSMATH2_H

#include <QtTest/QtTest>
#include "us_math2.h"
#include "us_dataIO.h"
#include "us_matrix.h"

class TestUSMath2 : public QObject
{
Q_OBJECT

private slots:
    void test_box_muller();
    void test_ranf();
    void test_linefit();
    void test_nearest_curve_point();
};

#endif // TESTUSMATH2_H
