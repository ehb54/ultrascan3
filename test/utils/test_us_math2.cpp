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
    // Setup test data
    QVector<double> x = {1, 2, 3, 4, 5};
    QVector<double> y = {2, 4, 6, 8, 10};

    double slope, intercept, sigma, correlation;
    int arraysize = x.size();
    double* xPtr = x.data();
    double* yPtr = y.data();

    double average = US_Math2::linefit(&xPtr, &yPtr, &slope, &intercept, &sigma, &correlation, arraysize);

    // Define tolerance
    double tolerance = 0.0001;

    // Compare slope with tolerance
    QVERIFY2(qAbs(slope - 2.0) < tolerance, qPrintable(QString("Expected slope: 2.0, but got: %1").arg(slope)));

    // Compare intercept with tolerance
    QVERIFY2(qAbs(intercept - 0.0) < tolerance, qPrintable(QString("Expected intercept: 0.0, but got: %1").arg(intercept)));

    // Calculate the expected sigma as implemented in linefit
    double sumy = 0.0;
    for (int i = 0; i < arraysize; i++) {
        sumy += y[i];
    }
    double calculated_average = sumy / arraysize;

    double sumchi_sq = 0.0;
    for (int i = 0; i < arraysize; i++) {
        sumchi_sq += (y[i] - calculated_average) * (y[i] - calculated_average);
    }
    double expected_sigma = std::sqrt(sumchi_sq) / arraysize; // Match the linefit function's calculation

    // Compare sigma with the expected value
    QVERIFY2(qAbs(sigma - expected_sigma) < tolerance, qPrintable(QString("Expected sigma: %1, but got: %2").arg(expected_sigma).arg(sigma)));

    // Compare correlation with tolerance (should be 1 for perfectly linear data)
    QVERIFY2(qAbs(correlation - 1.0) < tolerance, qPrintable(QString("Expected correlation: 1.0, but got: %1").arg(correlation)));
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