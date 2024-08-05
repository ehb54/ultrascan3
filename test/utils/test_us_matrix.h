#ifndef TEST_US_MATRIX_H
#define TEST_US_MATRIX_H

#include <QtTest/QtTest>
#include "us_matrix.h"

class TestUSMatrix : public QObject
{
Q_OBJECT

private slots:
    void testLsfit();
    void testCholeskyDecomposition();
    void testCholeskySolveSystem();
    void testConstruct();
};

#endif // TEST_US_MATRIX_H
