// test_us_math2_unit.cpp - Unit tests for US_Math2 class
#include "qt_test_base.h"
#include "us_math2.h"
#include "us_dataIO.h"
#include "us_solution.h"
#include <QString>
#include <QVector>
#include <QTime>
#include <cmath>
#include <limits>

using namespace qt_matchers;

class TestUSMath2Unit : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        setupTestData();
    }

    void TearDown() override {
        QtTestBase::TearDown();
    }

    void setupTestData() {
        // Set up test data arrays
        for (int i = 0; i < 10; i++) {
            testX.append(i * 1.0);
            testY.append(i * 2.0 + 1.0); // y = 2x + 1
        }

        testXPtr = testX.data();
        testYPtr = testY.data();
    }

    QVector<double> testX;
    QVector<double> testY;
    double* testXPtr;
    double* testYPtr;

    static constexpr double TOLERANCE = 1e-6;
};

// ============================================================================
// RANDOM NUMBER GENERATION TESTS
// ============================================================================

TEST_F(TestUSMath2Unit, RanfInRange) {
// Test ranf method returns values in [0, 1)
for (int i = 0; i < 100; i++) {
double result = US_Math2::ranf();
EXPECT_GE(result, 0.0) << "ranf should return values >= 0.0";
EXPECT_LT(result, 1.0) << "ranf should return values < 1.0";
}
}

TEST_F(TestUSMath2Unit, RanfDistribution) {
// Test ranf produces different values (basic distribution test)
std::set<double> values;
for (int i = 0; i < 50; i++) {
values.insert(US_Math2::ranf());
}
EXPECT_GT(values.size(), 30u) << "ranf should produce diverse values";
}

TEST_F(TestUSMath2Unit, BoxMullerMeanZero) {
// Test box_muller with mean=0, std=1
double sum = 0.0;
int count = 1000;

for (int i = 0; i < count; i++) {
sum += US_Math2::box_muller(0.0, 1.0);
}

double average = sum / count;
EXPECT_NEAR(average, 0.0, 0.2) << "Box-Muller should approximate mean=0";
}

TEST_F(TestUSMath2Unit, BoxMullerNonZeroMean) {
// Test box_muller with non-zero mean
double mean = 5.0;
double sum = 0.0;
int count = 1000;

for (int i = 0; i < count; i++) {
sum += US_Math2::box_muller(mean, 1.0);
}

double average = sum / count;
EXPECT_NEAR(average, mean, 0.5) << "Box-Muller should approximate specified mean";
}

TEST_F(TestUSMath2Unit, BoxMullerZeroStdDev) {
// Test box_muller with zero standard deviation
double mean = 3.0;
double result = US_Math2::box_muller(mean, 0.0);
EXPECT_DOUBLE_EQ(result, mean) << "Box-Muller with std=0 should return mean";
}

// ============================================================================
// LINE FITTING TESTS
// ============================================================================

TEST_F(TestUSMath2Unit, LinefitPerfectLine) {
// Test linefit with perfect linear data y = 2x + 1
double slope, intercept, sigma, correlation;

double average = US_Math2::linefit(&testXPtr, &testYPtr, &slope, &intercept,
                                   &sigma, &correlation, 10);

EXPECT_NEAR(slope, 2.0, TOLERANCE) << "Should detect correct slope";
EXPECT_NEAR(intercept, 1.0, TOLERANCE) << "Should detect correct intercept";
EXPECT_NEAR(correlation, 1.0, TOLERANCE) << "Perfect line should have correlation = 1";
EXPECT_GT(average, 0.0) << "Average should be positive";
}

TEST_F(TestUSMath2Unit, LinefitSinglePoint) {
// Test linefit with single point
double x = 5.0, y = 10.0;
double* xPtr = &x;
double* yPtr = &y;
double slope, intercept, sigma, correlation;

double average = US_Math2::linefit(&xPtr, &yPtr, &slope, &intercept,
                                   &sigma, &correlation, 1);

EXPECT_DOUBLE_EQ(average, y) << "Average of single point should be the point value";
EXPECT_DOUBLE_EQ(sigma, 0.0) << "Single point should have zero sigma";
}

TEST_F(TestUSMath2Unit, LinefitTwoPoints) {
// Test linefit with exactly two points
QVector<double> x2 = {1.0, 3.0};
QVector<double> y2 = {2.0, 6.0}; // y = 2x
double* x2Ptr = x2.data();
double* y2Ptr = y2.data();
double slope, intercept, sigma, correlation;

double average = US_Math2::linefit(&x2Ptr, &y2Ptr, &slope, &intercept,
                                   &sigma, &correlation, 2);

EXPECT_NEAR(slope, 2.0, TOLERANCE) << "Two points should give correct slope";
EXPECT_NEAR(intercept, 0.0, TOLERANCE) << "Two points should give correct intercept";
EXPECT_DOUBLE_EQ(average, 4.0) << "Average should be (2+6)/2 = 4";
}

TEST_F(TestUSMath2Unit, LinefitZeroArray) {
// Test linefit with zero array size (edge case)
double slope, intercept, sigma, correlation;

// This might cause undefined behavior, but we test the boundary
EXPECT_NO_THROW({
US_Math2::linefit(&testXPtr, &testYPtr, &slope, &intercept,
        &sigma, &correlation, 0);
}) << "Zero array size should not crash";
}

TEST_F(TestUSMath2Unit, LinefitHorizontalLine) {
// Test linefit with horizontal line (zero slope)
QVector<double> xh = {1.0, 2.0, 3.0, 4.0, 5.0};
QVector<double> yh = {5.0, 5.0, 5.0, 5.0, 5.0}; // y = 5 (horizontal)
double* xhPtr = xh.data();
double* yhPtr = yh.data();
double slope, intercept, sigma, correlation;

double average = US_Math2::linefit(&xhPtr, &yhPtr, &slope, &intercept,
                                   &sigma, &correlation, 5);

EXPECT_NEAR(slope, 0.0, TOLERANCE) << "Horizontal line should have zero slope";
EXPECT_NEAR(intercept, 5.0, TOLERANCE) << "Horizontal line should have intercept = y value";
EXPECT_DOUBLE_EQ(average, 5.0) << "Average should be the constant y value";
EXPECT_DOUBLE_EQ(sigma, 0.0) << "Perfect horizontal line should have zero sigma";
}

TEST_F(TestUSMath2Unit, LinefitVerticalData) {
// Test linefit with identical x values (vertical line scenario)
QVector<double> xv = {2.0, 2.0, 2.0, 2.0};
QVector<double> yv = {1.0, 2.0, 3.0, 4.0};
double* xvPtr = xv.data();
double* yvPtr = yv.data();
double slope, intercept, sigma, correlation;

// This should handle division by zero gracefully or produce inf/nan
EXPECT_NO_THROW({
US_Math2::linefit(&xvPtr, &yvPtr, &slope, &intercept,
        &sigma, &correlation, 4);
}) << "Vertical data should not crash";

// Results may be inf or nan, but should not crash
EXPECT_TRUE(std::isfinite(slope) || std::isinf(slope) || std::isnan(slope))
<< "Slope should be finite, inf, or nan";
}

// ============================================================================
// INTERSECTION TESTS
// ============================================================================

TEST_F(TestUSMath2Unit, IntersectNonParallelLines) {
// Test intersection of non-parallel lines
double slope1 = 1.0, intercept1 = 0.0; // y = x
double slope2 = -1.0, intercept2 = 2.0; // y = -x + 2
double xisec, yisec;

bool result = US_Math2::intersect(slope1, intercept1, slope2, intercept2,
                                  &xisec, &yisec);

EXPECT_TRUE(result) << "Non-parallel lines should intersect";
EXPECT_NEAR(xisec, 1.0, TOLERANCE) << "Intersection x should be 1.0";
EXPECT_NEAR(yisec, 1.0, TOLERANCE) << "Intersection y should be 1.0";
}

TEST_F(TestUSMath2Unit, IntersectParallelLines) {
// Test intersection of parallel lines
double slope1 = 2.0, intercept1 = 1.0; // y = 2x + 1
double slope2 = 2.0, intercept2 = 3.0; // y = 2x + 3
double xisec, yisec;

bool result = US_Math2::intersect(slope1, intercept1, slope2, intercept2,
                                  &xisec, &yisec);

EXPECT_FALSE(result) << "Parallel lines should not intersect";
}

TEST_F(TestUSMath2Unit, IntersectIdenticalLines) {
// Test intersection of identical lines
double slope1 = 1.0, intercept1 = 2.0; // y = x + 2
double slope2 = 1.0, intercept2 = 2.0; // y = x + 2
double xisec, yisec;

bool result = US_Math2::intersect(slope1, intercept1, slope2, intercept2,
                                  &xisec, &yisec);

EXPECT_FALSE(result) << "Identical lines should be treated as parallel";
}

TEST_F(TestUSMath2Unit, IntersectZeroSlopes) {
// Test intersection with zero slopes (horizontal lines)
double slope1 = 0.0, intercept1 = 3.0; // y = 3
double slope2 = 0.0, intercept2 = 5.0; // y = 5
double xisec, yisec;

bool result = US_Math2::intersect(slope1, intercept1, slope2, intercept2,
                                  &xisec, &yisec);

EXPECT_FALSE(result) << "Two horizontal lines should not intersect";
}

TEST_F(TestUSMath2Unit, IntersectCurveArrays) {
// Test intersection with curve arrays
QVector<double> x1 = {0.0, 1.0, 2.0}; // Points on y = x
QVector<double> y1 = {0.0, 1.0, 2.0};
QVector<double> x2 = {0.0, 1.0, 2.0}; // Points on y = -x + 2
QVector<double> y2 = {2.0, 1.0, 0.0};
double xisec, yisec;

bool result = US_Math2::intersect(x1.data(), y1.data(), 3,
                                  x2.data(), y2.data(), 3,
                                  &xisec, &yisec);

EXPECT_TRUE(result) << "Curve arrays should find intersection";
EXPECT_NEAR(xisec, 1.0, TOLERANCE) << "Array intersection x should be 1.0";
EXPECT_NEAR(yisec, 1.0, TOLERANCE) << "Array intersection y should be 1.0";
}

// ============================================================================
// NEAREST CURVE POINT TESTS
// ============================================================================

TEST_F(TestUSMath2Unit, NearestCurvePointExact) {
// Test nearest curve point with exact match
QVector<double> xs = {1.0, 2.0, 3.0, 4.0, 5.0};
QVector<double> ys = {1.0, 4.0, 9.0, 16.0, 25.0}; // y = x^2
double xgiven = 3.0, ygiven = 9.0;
double xnear, ynear;

int index = US_Math2::nearest_curve_point(xs.data(), ys.data(), 5, false,
                                          xgiven, ygiven, &xnear, &ynear,
                                          nullptr, nullptr);

EXPECT_EQ(index, 2) << "Should find exact point at index 2";
EXPECT_DOUBLE_EQ(xnear, 3.0) << "Nearest x should be 3.0";
EXPECT_DOUBLE_EQ(ynear, 9.0) << "Nearest y should be 9.0";
}

TEST_F(TestUSMath2Unit, NearestCurvePointApproximate) {
// Test nearest curve point with approximate match
QVector<double> xs = {1.0, 2.0, 3.0, 4.0, 5.0};
QVector<double> ys = {1.0, 4.0, 9.0, 16.0, 25.0}; // y = x^2
double xgiven = 2.5, ygiven = 6.0;
double xnear, ynear;

int index = US_Math2::nearest_curve_point(xs.data(), ys.data(), 5, false,
                                          xgiven, ygiven, &xnear, &ynear,
                                          nullptr, nullptr);

EXPECT_TRUE(index == 1 || index == 2) << "Should find nearest point";
EXPECT_TRUE(xnear == 2.0 || xnear == 3.0) << "Should return actual curve point";
}

TEST_F(TestUSMath2Unit, NearestCurvePointInterpolated) {
// Test nearest curve point with interpolation
QVector<double> xs = {1.0, 2.0, 3.0, 4.0, 5.0};
QVector<double> ys = {1.0, 4.0, 9.0, 16.0, 25.0};
double xgiven = 2.5, ygiven = 6.0;
double xnear, ynear;

int index = US_Math2::nearest_curve_point(xs.data(), ys.data(), 5, true,
                                          xgiven, ygiven, &xnear, &ynear,
                                          nullptr, nullptr);

EXPECT_GE(index, 0) << "Should return valid index";
EXPECT_LT(index, 5) << "Index should be within array bounds";
// With interpolation, xnear and ynear may be between curve points
EXPECT_GE(xnear, 1.0) << "Interpolated x should be within curve range";
EXPECT_LE(xnear, 5.0) << "Interpolated x should be within curve range";
}

TEST_F(TestUSMath2Unit, NearestCurvePointSinglePoint) {
// Test nearest curve point with single point
double xs = 3.0, ys = 7.0;
double xgiven = 2.0, ygiven = 5.0;
double xnear, ynear;

int index = US_Math2::nearest_curve_point(&xs, &ys, 1, false,
                                          xgiven, ygiven, &xnear, &ynear,
                                          nullptr, nullptr);

EXPECT_EQ(index, 0) << "Single point should return index 0";
EXPECT_DOUBLE_EQ(xnear, 3.0) << "Should return the single point x";
EXPECT_DOUBLE_EQ(ynear, 7.0) << "Should return the single point y";
}

TEST_F(TestUSMath2Unit, NearestCurvePointWithZValues) {
// Test nearest curve point with additional z values
QVector<double> xs = {1.0, 2.0, 3.0};
QVector<double> ys = {1.0, 4.0, 9.0};
QVector<double> zs = {10.0, 20.0, 30.0};
double xgiven = 2.0, ygiven = 4.0;
double xnear, ynear, znear;

int index = US_Math2::nearest_curve_point(xs.data(), ys.data(), 3, false,
                                          xgiven, ygiven, &xnear, &ynear,
                                          zs.data(), &znear);

EXPECT_EQ(index, 1) << "Should find exact point at index 1";
EXPECT_DOUBLE_EQ(znear, 20.0) << "Should return corresponding z value";
}

// ============================================================================
// NORMAL DISTRIBUTION TESTS
// ============================================================================

TEST_F(TestUSMath2Unit, NormalDistributionAtMean) {
// Test normal distribution at mean (maximum point)
double result = US_Math2::normal_distribution(1.0, 0.0, 0.0);
double expected = 1.0 / sqrt(2.0 * M_PI); // 1/(sigma*sqrt(2*pi))

EXPECT_NEAR(result, expected, TOLERANCE) << "Normal distribution at mean should be maximum";
}

TEST_F(TestUSMath2Unit, NormalDistributionSymmetry) {
// Test normal distribution symmetry around mean
double sigma = 2.0, mean = 5.0;
double left = US_Math2::normal_distribution(sigma, mean, mean - 1.0);
double right = US_Math2::normal_distribution(sigma, mean, mean + 1.0);

EXPECT_NEAR(left, right, TOLERANCE) << "Normal distribution should be symmetric around mean";
}

TEST_F(TestUSMath2Unit, NormalDistributionZeroSigma) {
// Test normal distribution with zero sigma (should handle gracefully)
EXPECT_NO_THROW({
US_Math2::normal_distribution(0.0, 0.0, 0.0);
}) << "Zero sigma should not crash";
}

TEST_F(TestUSMath2Unit, NormalDistributionLargeSigma) {
// Test normal distribution with large sigma
double result = US_Math2::normal_distribution(100.0, 0.0, 0.0);
double expected = 1.0 / (100.0 * sqrt(2.0 * M_PI));

EXPECT_NEAR(result, expected, TOLERANCE) << "Large sigma should give small peak value";
}

// ============================================================================
// RANDOMIZATION TESTS
// ============================================================================

TEST_F(TestUSMath2Unit, RandomizeReturnsSeed) {
// Test randomize returns a seed value
uint seed = US_Math2::randomize();
EXPECT_GT(seed, 0u) << "randomize() should return a positive seed";
}

TEST_F(TestUSMath2Unit, RandomizeWithSeed) {
// Test randomize with specific seed
uint inputSeed = 12345;
uint returnedSeed = US_Math2::randomize(inputSeed);
EXPECT_EQ(returnedSeed, inputSeed) << "randomize(seed) should return the input seed";
}

TEST_F(TestUSMath2Unit, RandomizeWithZeroSeed) {
// Test randomize with zero seed (should generate random seed)
uint returnedSeed = US_Math2::randomize(0);
EXPECT_GT(returnedSeed, 0u) << "randomize(0) should generate and return positive seed";
}

TEST_F(TestUSMath2Unit, RandomizeConsistentSequence) {
// Test that same seed produces same initial random values
uint seed = 54321;

US_Math2::randomize(seed);
double first1 = US_Math2::ranf();
double second1 = US_Math2::ranf();

US_Math2::randomize(seed);
double first2 = US_Math2::ranf();
double second2 = US_Math2::ranf();

EXPECT_DOUBLE_EQ(first1, first2) << "Same seed should produce same sequence";
EXPECT_DOUBLE_EQ(second1, second2) << "Same seed should produce same sequence";
}

// ============================================================================
// GAUSSIAN SMOOTHING TESTS
// ============================================================================

TEST_F(TestUSMath2Unit, GaussianSmoothingNoSmoothing) {
// Test gaussian smoothing with smooth <= 1 (no change)
QVector<double> original = {1.0, 2.0, 3.0, 4.0, 5.0};
QVector<double> data = original;

US_Math2::gaussian_smoothing(data, 1);

for (int i = 0; i < data.size(); i++) {
EXPECT_DOUBLE_EQ(data[i], original[i]) << "No smoothing should leave data unchanged";
}
}

TEST_F(TestUSMath2Unit, GaussianSmoothingConstantData) {
// Test gaussian smoothing with constant data
QVector<double> data = {5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0};

US_Math2::gaussian_smoothing(data, 3);

for (int i = 0; i < data.size(); i++) {
EXPECT_NEAR(data[i], 5.0, TOLERANCE) << "Constant data should remain constant after smoothing";
}
}

TEST_F(TestUSMath2Unit, GaussianSmoothingReducesNoise) {
// Test gaussian smoothing reduces high-frequency noise
QVector<double> noisy = {1.0, 5.0, 2.0, 6.0, 3.0, 7.0, 4.0};
QVector<double> original = noisy;

US_Math2::gaussian_smoothing(noisy, 3);

// Check that extreme values are reduced
double originalRange = *std::max_element(original.begin(), original.end()) -
                       *std::min_element(original.begin(), original.end());
double smoothedRange = *std::max_element(noisy.begin(), noisy.end()) -
                       *std::min_element(noisy.begin(), noisy.end());

EXPECT_LT(smoothedRange, originalRange) << "Smoothing should reduce range of noisy data";
}

TEST_F(TestUSMath2Unit, GaussianSmoothingSinglePoint) {
// Test gaussian smoothing with single point
QVector<double> data = {42.0};

EXPECT_NO_THROW({
US_Math2::gaussian_smoothing(data, 5);
}) << "Single point smoothing should not crash";

EXPECT_DOUBLE_EQ(data[0], 42.0) << "Single point should remain unchanged";
}

TEST_F(TestUSMath2Unit, GaussianSmoothingEmptyArray) {
// Test gaussian smoothing with empty array
QVector<double> data;

EXPECT_NO_THROW({
US_Math2::gaussian_smoothing(data, 3);
}) << "Empty array smoothing should not crash";
}

// ============================================================================
// NORM VALUE TESTS
// ============================================================================

TEST_F(TestUSMath2Unit, NormValueArray) {
// Test norm value calculation for array
double data[] = {3.0, 4.0, 0.0}; // Should give norm = 5.0
double result = US_Math2::norm_value(data, 3);

EXPECT_NEAR(result, 5.0, TOLERANCE) << "Norm of [3,4,0] should be 5";
}

TEST_F(TestUSMath2Unit, NormValueSingleElement) {
// Test norm value with single element
double data[] = {7.0};
double result = US_Math2::norm_value(data, 1);

EXPECT_DOUBLE_EQ(result, 7.0) << "Norm of single element should be absolute value";
}

TEST_F(TestUSMath2Unit, NormValueZeroArray) {
// Test norm value with zero array
double data[] = {0.0, 0.0, 0.0};
double result = US_Math2::norm_value(data, 3);

EXPECT_DOUBLE_EQ(result, 0.0) << "Norm of zero array should be 0";
}

TEST_F(TestUSMath2Unit, NormValueVector) {
// Test norm value with QVector
QVector<double> data = {1.0, 2.0, 2.0}; // Should give norm = 3.0
double result = US_Math2::norm_value(&data);

EXPECT_NEAR(result, 3.0, TOLERANCE) << "Norm of [1,2,2] should be 3";
}

TEST_F(TestUSMath2Unit, NormValueEmptyVector) {
// Test norm value with empty vector
QVector<double> data;
double result = US_Math2::norm_value(&data);

EXPECT_DOUBLE_EQ(result, 0.0) << "Norm of empty vector should be 0";
}

TEST_F(TestUSMath2Unit, NormValueNegativeValues) {
// Test norm value with negative values
double data[] = {-3.0, -4.0, 0.0}; // Should give norm = 5.0 (same as positive)
double result = US_Math2::norm_value(data, 3);

EXPECT_NEAR(result, 5.0, TOLERANCE) << "Norm should handle negative values correctly";
}

// ============================================================================
// BEST GRID REPS TESTS
// ============================================================================

TEST_F(TestUSMath2Unit, BestGridRepsValid) {
// Test best_grid_reps with valid input
int ngrid_s = 100;
int ngrid_k = 80;

int reps = US_Math2::best_grid_reps(ngrid_s, ngrid_k);

EXPECT_GT(reps, 0) << "Grid repetitions should be positive";
EXPECT_EQ(ngrid_s % reps, 0) << "Final s grid should be divisible by reps";
EXPECT_EQ(ngrid_k % reps, 0) << "Final k grid should be divisible by reps";
}

TEST_F(TestUSMath2Unit, BestGridRepsMinimumValues) {
// Test best_grid_reps with minimum values
int ngrid_s = 5;  // Below minimum
int ngrid_k = 5;  // Below minimum

int reps = US_Math2::best_grid_reps(ngrid_s, ngrid_k);

EXPECT_GE(ngrid_s, 10) << "Grid s should be adjusted to minimum";
EXPECT_GE(ngrid_k, 10) << "Grid k should be adjusted to minimum";
EXPECT_GT(reps, 0) << "Should return valid repetitions";
}

TEST_F(TestUSMath2Unit, BestGridRepsMaximumValues) {
// Test best_grid_reps with maximum values
int ngrid_s = 3000; // Above maximum
int ngrid_k = 3000; // Above maximum

int reps = US_Math2::best_grid_reps(ngrid_s, ngrid_k);

EXPECT_LE(ngrid_s, 2100) << "Grid s should be adjusted to maximum";
EXPECT_LE(ngrid_k, 2100) << "Grid k should be adjusted to maximum";
EXPECT_GT(reps, 0) << "Should return valid repetitions";
}

TEST_F(TestUSMath2Unit, BestGridRepsPrimeNumbers) {
// Test best_grid_reps with prime numbers (challenging for finding common factors)
int ngrid_s = 101; // Prime number
int ngrid_k = 103; // Prime number

int reps = US_Math2::best_grid_reps(ngrid_s, ngrid_k);

EXPECT_GT(reps, 0) << "Should handle prime numbers gracefully";
EXPECT_EQ(ngrid_s % reps, 0) << "Final s grid should be divisible by reps";
EXPECT_EQ(ngrid_k % reps, 0) << "Final k grid should be divisible by reps";
}

TEST_F(TestUSMath2Unit, BestGridRepsEqualValues) {
// Test best_grid_reps with equal input values
int ngrid_s = 120;
int ngrid_k = 120;

int reps = US_Math2::best_grid_reps(ngrid_s, ngrid_k);

EXPECT_GT(reps, 0) << "Should return valid repetitions for equal values";
EXPECT_EQ(ngrid_s % reps, 0) << "Final s grid should be divisible by reps";
EXPECT_EQ(ngrid_k % reps, 0) << "Final k grid should be divisible by reps";
}