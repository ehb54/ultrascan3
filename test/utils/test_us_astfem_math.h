#ifndef TEST_US_ASTFEM_MATH_H
#define TEST_US_ASTFEM_MATH_H

#include <QObject>
#include <QtTest/QtTest>
#include "us_astfem_math.h" // Include the header file for the class being tested

class TestUSAstfemMath : public QObject
{
Q_OBJECT

private slots:
    // Function called before any test functions are run
    void initTestCase();

    // Function called after all test functions have been run
    void cleanupTestCase();

    // Test functions
    void testWritetimestate();
    void testLowAcceleration();
    void testInterpolateC0();

private:
    QString tmst_fpath;  // Path to the timestate file used in the tests
};

#endif // TEST_US_ASTFEM_MATH_H
