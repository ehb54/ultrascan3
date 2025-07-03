#include "test_us_math2.h"
#include "us_math2.h"
#include "us_dataIO.h"
#include "us_matrix.h"
#include <QVector>
#include <cmath>

// Use your custom Qt matchers
using namespace qt_matchers;

// TestUSMath2 method implementations
void TestUSMath2::SetUp() {
    QtTestBase::SetUp();
    // Per-test setup for Math2 tests
}

void TestUSMath2::TearDown() {
    // Per-test cleanup for Math2 tests
    QtTestBase::TearDown();
}

// Suite-level setup for Math2 tests
void TestUSMath2::SetUpTestSuite() {
    QtTestBase::SetUpTestSuite();
    // One-time setup for all TestUSMath2 tests
}

// Suite-level cleanup for Math2 tests
void TestUSMath2::TearDownTestSuite() {
    // One-time cleanup for all TestUSMath2 tests
}

TEST_F(TestUSMath2, BoxMuller) {
    // Test Box-Muller normal distribution generator
    double mean = 0.0;
    double stddev = 1.0;
    double value = US_Math2::box_muller(mean, stddev);

    // Check that value is within reasonable range (4 standard deviations)
    EXPECT_GE(value, mean - 4 * stddev)
                        << "Generated value should be within 4 standard deviations (lower bound)";
    EXPECT_LE(value, mean + 4 * stddev)
                        << "Generated value should be within 4 standard deviations (upper bound)";
}

TEST_F(TestUSMath2, BoxMullerMultipleValues) {
    // Test multiple values to ensure reasonable distribution
    double mean = 5.0;
    double stddev = 2.0;
    int numSamples = 100;
    double sum = 0.0;
    int inRange = 0;

    for (int i = 0; i < numSamples; i++) {
        double value = US_Math2::box_muller(mean, stddev);
        sum += value;

        // Count values within 3 standard deviations (should be ~99.7%)
        if (value >= mean - 3 * stddev && value <= mean + 3 * stddev) {
            inRange++;
        }
    }

    double sampleMean = sum / numSamples;

    // Sample mean should be reasonably close to target mean
    EXPECT_NEAR(sampleMean, mean, 1.0)
                        << "Sample mean should be close to target mean";

    // Most values should be within 3 standard deviations
    EXPECT_GE(inRange, numSamples * 0.95)
                        << "At least 95% of values should be within 3 standard deviations";
}

TEST_F(TestUSMath2, Ranf) {
    // Test random number generator
    double value = US_Math2::ranf();

    EXPECT_GE(value, 0.0)
                        << "Random value should be >= 0.0";
    EXPECT_LT(value, 1.0)
                        << "Random value should be < 1.0";
}

TEST_F(TestUSMath2, RanfMultipleValues) {
    // Test multiple random values for distribution
    int numSamples = 1000;
    double sum = 0.0;
    int validCount = 0;

    for (int i = 0; i < numSamples; i++) {
        double value = US_Math2::ranf();

        // Each value should be in valid range
        if (value >= 0.0 && value < 1.0) {
            validCount++;
            sum += value;
        }
    }

    EXPECT_EQ(validCount, numSamples)
                        << "All random values should be in valid range [0, 1)";

    double mean = sum / numSamples;
    // Mean of uniform distribution [0,1) should be around 0.5
    EXPECT_NEAR(mean, 0.5, 0.1)
                        << "Mean of uniform random values should be close to 0.5";
}

TEST_F(TestUSMath2, Linefit) {
    // Test linear fitting with perfect linear data
    QVector<double> x = {1, 2, 3, 4, 5};
    QVector<double> y = {2, 4, 6, 8, 10}; // y = 2x, perfect line

    double slope, intercept, sigma, correlation;
    int arraysize = x.size();
    double* xPtr = x.data();
    double* yPtr = y.data();

    double average = US_Math2::linefit(&xPtr, &yPtr, &slope, &intercept,
                                       &sigma, &correlation, arraysize);

    const double tolerance = 0.0001;

    // Check slope (should be 2.0 for y = 2x)
    EXPECT_NEAR(slope, 2.0, tolerance)
                        << "Slope should be 2.0 for perfect linear data y = 2x";

    // Check intercept (should be 0.0 for y = 2x)
    EXPECT_NEAR(intercept, 0.0, tolerance)
                        << "Intercept should be 0.0 for perfect linear data y = 2x";

    // Check correlation (should be 1.0 for perfect linear correlation)
    EXPECT_NEAR(correlation, 1.0, tolerance)
                        << "Correlation should be 1.0 for perfectly linear data";

    // Verify average calculation
    double expectedAverage = (2 + 4 + 6 + 8 + 10) / 5.0; // = 6.0
    EXPECT_NEAR(average, expectedAverage, tolerance)
                        << "Average should match calculated y-value average";
}

TEST_F(TestUSMath2, LinefitNonPerfectData) {
    // Test linear fitting with slightly noisy data
    QVector<double> x = {1, 2, 3, 4, 5};
    QVector<double> y = {2.1, 3.9, 6.1, 7.9, 10.1}; // Approximately y = 2x with small noise

    double slope, intercept, sigma, correlation;
    int arraysize = x.size();
    double* xPtr = x.data();
    double* yPtr = y.data();

    double average = US_Math2::linefit(&xPtr, &yPtr, &slope, &intercept,
                                       &sigma, &correlation, arraysize);

    // Should still be close to the expected values
    EXPECT_NEAR(slope, 2.0, 0.1)
                        << "Slope should be close to 2.0 even with small noise";
    EXPECT_NEAR(intercept, 0.0, 0.1)
                        << "Intercept should be close to 0.0 even with small noise";
    EXPECT_GT(correlation, 0.99)
                        << "Correlation should still be very high with small noise";
    EXPECT_GT(sigma, 0.0)
                        << "Sigma should be positive for noisy data";
}

TEST_F(TestUSMath2, LinefitEdgeCases) {
    // Test with minimum data points
    QVector<double> x = {1, 2};
    QVector<double> y = {3, 5}; // y = 2x + 1

    double slope, intercept, sigma, correlation;
    int arraysize = x.size();
    double* xPtr = x.data();
    double* yPtr = y.data();

    double average = US_Math2::linefit(&xPtr, &yPtr, &slope, &intercept,
                                       &sigma, &correlation, arraysize);

    // With only 2 points, should get exact fit
    EXPECT_NEAR(slope, 2.0, 0.0001) << "Slope should be exact with 2 points";
    EXPECT_NEAR(intercept, 1.0, 0.0001) << "Intercept should be exact with 2 points";
}

TEST_F(TestUSMath2, NearestCurvePoint) {
    // Test finding nearest point on a curve
    const int size = 5;
    double xs[size] = {1, 2, 3, 4, 5};
    double ys[size] = {1, 4, 9, 16, 25}; // y = x^2
    double xgiven = 3.5;
    double ygiven = 12.0;
    double xnear, ynear;

    int index = US_Math2::nearest_curve_point(xs, ys, size, true, xgiven, ygiven,
                                              &xnear, &ynear, nullptr, nullptr);

    EXPECT_EQ(index, 2)
                        << "Should find index 2 as nearest point";
    EXPECT_NEAR(xnear, 3.5, 0.1)
                        << "X coordinate of nearest point should be close to 3.5";
    EXPECT_NEAR(ynear, 12.25, 0.1)
                        << "Y coordinate should be close to interpolated value";
}

TEST_F(TestUSMath2, NearestCurvePointEdgeCases) {
    // Test with point at curve boundaries
    const int size = 3;
    double xs[size] = {1, 2, 3};
    double ys[size] = {10, 20, 30};
    double xnear, ynear;

    // Test point before first element - use variables for references
    double x1 = 0.5, y1 = 5.0;
    int index = US_Math2::nearest_curve_point(xs, ys, size, true, x1, y1,
                                              &xnear, &ynear, nullptr, nullptr);
    EXPECT_GE(index, 0) << "Should return valid index for point before curve";

    // Test point after last element - use variables for references
    double x2 = 4.0, y2 = 40.0;
    index = US_Math2::nearest_curve_point(xs, ys, size, true, x2, y2,
                                          &xnear, &ynear, nullptr, nullptr);
    EXPECT_GE(index, 0) << "Should return valid index for point after curve";

    // Test exact match - use variables for references
    double x3 = 2.0, y3 = 20.0;
    index = US_Math2::nearest_curve_point(xs, ys, size, true, x3, y3,
                                          &xnear, &ynear, nullptr, nullptr);
    EXPECT_EQ(index, 1) << "Should find exact match at index 1";
    EXPECT_NEAR(xnear, 2.0, 0.001) << "Should return exact x coordinate";
    EXPECT_NEAR(ynear, 20.0, 0.001) << "Should return exact y coordinate";
}

TEST_F(TestUSMath2, NearestCurvePointInterpolation) {
    // Test interpolation behavior
    const int size = 4;
    double xs[size] = {0, 1, 2, 3};
    double ys[size] = {0, 1, 4, 9}; // y = x^2
    double xnear, ynear;

    // Test interpolation between points - use variables for references
    double xgiven = 1.5, ygiven = 2.0;
    int index = US_Math2::nearest_curve_point(xs, ys, size, true, xgiven, ygiven,
                                              &xnear, &ynear, nullptr, nullptr);

    EXPECT_GE(index, 0) << "Should return valid index";
    EXPECT_GE(xnear, 1.0) << "Interpolated x should be >= 1.0";
    EXPECT_LE(xnear, 2.0) << "Interpolated x should be <= 2.0";
    EXPECT_GE(ynear, 1.0) << "Interpolated y should be >= 1.0";
    EXPECT_LE(ynear, 4.0) << "Interpolated y should be <= 4.0";
}

// Comprehensive integration test
TEST_F(TestUSMath2, MathFunctionsIntegration) {
    // Test that multiple math functions work together

    // Generate some random data using ranf
    QVector<double> x, y;
    for (int i = 0; i < 10; i++) {
        double xi = i + 1;
        double yi = 2 * xi + 1 + US_Math2::box_muller(0.0, 0.1); // Add small noise
        x.append(xi);
        y.append(yi);
    }

    // Fit a line to the data
    double slope, intercept, sigma, correlation;
    double* xPtr = x.data();
    double* yPtr = y.data();

    double average = US_Math2::linefit(&xPtr, &yPtr, &slope, &intercept,
                                       &sigma, &correlation, x.size());

    // Should recover approximately the original slope and intercept
    EXPECT_NEAR(slope, 2.0, 0.2)
                        << "Should recover original slope despite noise";
    EXPECT_NEAR(intercept, 1.0, 0.2)
                        << "Should recover original intercept despite noise";
    EXPECT_GT(correlation, 0.95)
                        << "Correlation should be high despite small amount of noise";

    // Test curve point finding on the fitted data - use variables for references
    double xgiven = 5.5;
    double ygiven = slope * xgiven + intercept;
    double xnear, ynear;

    int index = US_Math2::nearest_curve_point(x.data(), y.data(), x.size(), true,
                                              xgiven, ygiven, &xnear, &ynear, nullptr, nullptr);

    EXPECT_GE(index, 0) << "Should find valid nearest point";
    EXPECT_LT(std::abs(xnear - xgiven), 1.0)
                        << "Found point should be reasonably close to target";
}