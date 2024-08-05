#include <QtTest/QtTest>
#include "us_math2.h"
#include "us_dataIO.h"
#include "us_matrix.h"

class TestUSMath2 : public QObject
{

private slots:
    void test_box_muller();
    void test_ranf();
    void test_linefit();
    void test_nearest_curve_point();
};

void TestUSMath2::test_box_muller()
{
    double mean = 0.0;
    double stddev = 1.0;
    double value = US_Math2::box_muller(mean, stddev);
    QVERIFY(value >= mean - 4 * stddev && value <= mean + 4 * stddev);
}

void TestUSMath2::test_ranf()
{
    double value = US_Math2::ranf();
    QVERIFY(value >= 0.0 && value < 1.0);
}

void TestUSMath2::test_linefit()
{
    double xArray[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    double yArray[10] = {0, 1.1, 2, 3.1, 4, 5, 6, 7.1, 8, 9};

    double *x[10];
    double *y[10];

    for (int i = 0; i < 10; ++i)
    {
        x[i] = &xArray[i];
        y[i] = &yArray[i];
    }

    double slope, intercept, sigma, correlation;
    int n = 10;

    US_Math2::linefit(x, y, &slope, &intercept, &sigma, &correlation, n);

    QCOMPARE(slope, 1.0);
    QVERIFY(std::abs(intercept) < 0.1);
}

void TestUSMath2::test_nearest_curve_point()
{
    const int size = 5;
    double xs[size] = {1, 2, 3, 4, 5};
    double ys[size] = {1, 4, 9, 16, 25};
    double xgiven = 3.5;
    double ygiven = 12.0;
    double xnear, ynear;

    int index = US_Math2::nearest_curve_point(xs, ys, size, true, xgiven, ygiven, &xnear, &ynear, nullptr, nullptr);

    QCOMPARE(index, 2);
    QVERIFY(qAbs(xnear - 3.5) < 0.1);
    QVERIFY(qAbs(ynear - 12.25) < 0.1);
}