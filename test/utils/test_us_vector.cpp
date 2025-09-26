#include "qt_test_base.h"
#include "us_vector.h"
#include <cmath>
#include <limits>

using namespace ::testing;
using namespace qt_matchers;

class US_VectorTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
    }

    // Helper function to compare floating point vectors with tolerance
    void expectVectorEqual(const US_Vector& actual, const US_Vector& expected, double tolerance = 1e-9) {
        EXPECT_EQ(actual.size(), expected.size());
        for (int i = 0; i < actual.size(); i++) {
            EXPECT_NEAR(actual[i], expected[i], tolerance) << "at index " << i;
        }
    }

    // Helper function to compare floating point values with tolerance
    void expectDoubleEqual(double actual, double expected, double tolerance = 1e-9) {
        EXPECT_NEAR(actual, expected, tolerance);
    }
};

// Tests for US_Vector constructor
class US_VectorConstructorTest : public US_VectorTest {};

TEST_F(US_VectorConstructorTest, CreatesVectorWithDefaultZeroValues) {
US_Vector vec(5);

EXPECT_EQ(vec.size(), 5);
for (int i = 0; i < 5; i++) {
EXPECT_EQ(vec[i], 0.0);
}
}

TEST_F(US_VectorConstructorTest, CreatesVectorWithSpecifiedValue) {
US_Vector vec(3, 2.5);

EXPECT_EQ(vec.size(), 3);
for (int i = 0; i < 3; i++) {
EXPECT_EQ(vec[i], 2.5);
}
}

TEST_F(US_VectorConstructorTest, CreatesVectorWithNegativeValue) {
US_Vector vec(4, -1.5);

EXPECT_EQ(vec.size(), 4);
for (int i = 0; i < 4; i++) {
EXPECT_EQ(vec[i], -1.5);
}
}

TEST_F(US_VectorConstructorTest, CreatesEmptyVector) {
US_Vector vec(0);

EXPECT_EQ(vec.size(), 0);
}

TEST_F(US_VectorConstructorTest, CreatesVectorWithLargeSize) {
US_Vector vec(1000, 1.0);

EXPECT_EQ(vec.size(), 1000);
EXPECT_EQ(vec[0], 1.0);
EXPECT_EQ(vec[999], 1.0);
}

TEST_F(US_VectorConstructorTest, CreatesVectorWithVerySmallValue) {
double smallValue = std::numeric_limits<double>::min();
US_Vector vec(2, smallValue);

EXPECT_EQ(vec.size(), 2);
EXPECT_EQ(vec[0], smallValue);
EXPECT_EQ(vec[1], smallValue);
}

TEST_F(US_VectorConstructorTest, CreatesVectorWithVeryLargeValue) {
double largeValue = std::numeric_limits<double>::max();
US_Vector vec(2, largeValue);

EXPECT_EQ(vec.size(), 2);
EXPECT_EQ(vec[0], largeValue);
EXPECT_EQ(vec[1], largeValue);
}

// Tests for add(const US_Vector&) method
class US_VectorAddVectorTest : public US_VectorTest {};

TEST_F(US_VectorAddVectorTest, AddsVectorsOfSameSize) {
US_Vector vec1(3);
vec1.assign(0, 1.0);
vec1.assign(1, 2.0);
vec1.assign(2, 3.0);

US_Vector vec2(3);
vec2.assign(0, 4.0);
vec2.assign(1, 5.0);
vec2.assign(2, 6.0);

vec1.add(vec2);

EXPECT_EQ(vec1[0], 5.0);
EXPECT_EQ(vec1[1], 7.0);
EXPECT_EQ(vec1[2], 9.0);
}

TEST_F(US_VectorAddVectorTest, AddsZeroVector) {
US_Vector vec1(3);
vec1.assign(0, 1.0);
vec1.assign(1, 2.0);
vec1.assign(2, 3.0);

US_Vector zeroVec(3, 0.0);

vec1.add(zeroVec);

EXPECT_EQ(vec1[0], 1.0);
EXPECT_EQ(vec1[1], 2.0);
EXPECT_EQ(vec1[2], 3.0);
}

TEST_F(US_VectorAddVectorTest, AddsNegativeValues) {
US_Vector vec1(2);
vec1.assign(0, 5.0);
vec1.assign(1, 3.0);

US_Vector vec2(2);
vec2.assign(0, -2.0);
vec2.assign(1, -1.0);

vec1.add(vec2);

EXPECT_EQ(vec1[0], 3.0);
EXPECT_EQ(vec1[1], 2.0);
}

TEST_F(US_VectorAddVectorTest, AddsEmptyVectors) {
US_Vector vec1(0);
US_Vector vec2(0);

vec1.add(vec2);

EXPECT_EQ(vec1.size(), 0);
}

// Tests for add(double) method
class US_VectorAddScalarTest : public US_VectorTest {};

TEST_F(US_VectorAddScalarTest, AddsPositiveScalar) {
US_Vector vec(3);
vec.assign(0, 1.0);
vec.assign(1, 2.0);
vec.assign(2, 3.0);

vec.add(5.0);

EXPECT_EQ(vec[0], 6.0);
EXPECT_EQ(vec[1], 7.0);
EXPECT_EQ(vec[2], 8.0);
}

TEST_F(US_VectorAddScalarTest, AddsNegativeScalar) {
US_Vector vec(3);
vec.assign(0, 5.0);
vec.assign(1, 4.0);
vec.assign(2, 3.0);

vec.add(-2.0);

EXPECT_EQ(vec[0], 3.0);
EXPECT_EQ(vec[1], 2.0);
EXPECT_EQ(vec[2], 1.0);
}

TEST_F(US_VectorAddScalarTest, AddsZero) {
US_Vector vec(3);
vec.assign(0, 1.0);
vec.assign(1, 2.0);
vec.assign(2, 3.0);

vec.add(0.0);

EXPECT_EQ(vec[0], 1.0);
EXPECT_EQ(vec[1], 2.0);
EXPECT_EQ(vec[2], 3.0);
}

TEST_F(US_VectorAddScalarTest, AddsToEmptyVector) {
US_Vector vec(0);

vec.add(5.0);

EXPECT_EQ(vec.size(), 0);
}

TEST_F(US_VectorAddScalarTest, AddsVeryLargeScalar) {
US_Vector vec(2);
vec.assign(0, 1.0);
vec.assign(1, 2.0);

double largeValue = 1e10;
vec.add(largeValue);

expectDoubleEqual(vec[0], 1e10 + 1.0);
expectDoubleEqual(vec[1], 1e10 + 2.0);
}

// Tests for dot product method
class US_VectorDotTest : public US_VectorTest {};

TEST_F(US_VectorDotTest, CalculatesDotProductOfOrthogonalVectors) {
US_Vector vec1(3);
vec1.assign(0, 1.0);
vec1.assign(1, 0.0);
vec1.assign(2, 0.0);

US_Vector vec2(3);
vec2.assign(0, 0.0);
vec2.assign(1, 1.0);
vec2.assign(2, 0.0);

double result = vec1.dot(vec2);

EXPECT_EQ(result, 0.0);
}

TEST_F(US_VectorDotTest, CalculatesDotProductOfParallelVectors) {
US_Vector vec1(3);
vec1.assign(0, 2.0);
vec1.assign(1, 3.0);
vec1.assign(2, 4.0);

US_Vector vec2(3);
vec2.assign(0, 4.0);
vec2.assign(1, 6.0);
vec2.assign(2, 8.0);

double result = vec1.dot(vec2);

EXPECT_EQ(result, 8.0 + 18.0 + 32.0); // 58.0
}

TEST_F(US_VectorDotTest, CalculatesDotProductWithZeroVector) {
US_Vector vec1(3);
vec1.assign(0, 1.0);
vec1.assign(1, 2.0);
vec1.assign(2, 3.0);

US_Vector zeroVec(3, 0.0);

double result = vec1.dot(zeroVec);

EXPECT_EQ(result, 0.0);
}

TEST_F(US_VectorDotTest, CalculatesDotProductOfEmptyVectors) {
US_Vector vec1(0);
US_Vector vec2(0);

double result = vec1.dot(vec2);

EXPECT_EQ(result, 0.0);
}

TEST_F(US_VectorDotTest, CalculatesDotProductWithNegativeValues) {
US_Vector vec1(2);
vec1.assign(0, 3.0);
vec1.assign(1, -4.0);

US_Vector vec2(2);
vec2.assign(0, -2.0);
vec2.assign(1, 5.0);

double result = vec1.dot(vec2);

EXPECT_EQ(result, -6.0 + (-20.0)); // -26.0
}

TEST_F(US_VectorDotTest, CalculatesDotProductOfUnitVectors) {
US_Vector vec1(2);
vec1.assign(0, 1.0);
vec1.assign(1, 0.0);

US_Vector vec2(2);
vec2.assign(0, 0.0);
vec2.assign(1, 1.0);

double result = vec1.dot(vec2);

EXPECT_EQ(result, 0.0);
}

// Tests for element-wise multiplication method
class US_VectorMultTest : public US_VectorTest {};

TEST_F(US_VectorMultTest, MultipliesVectorsElementWise) {
US_Vector vec1(3);
vec1.assign(0, 2.0);
vec1.assign(1, 3.0);
vec1.assign(2, 4.0);

US_Vector vec2(3);
vec2.assign(0, 5.0);
vec2.assign(1, 6.0);
vec2.assign(2, 7.0);

vec1.mult(vec2);

EXPECT_EQ(vec1[0], 10.0);
EXPECT_EQ(vec1[1], 18.0);
EXPECT_EQ(vec1[2], 28.0);
}

TEST_F(US_VectorMultTest, MultipliesWithZeroVector) {
US_Vector vec1(3);
vec1.assign(0, 2.0);
vec1.assign(1, 3.0);
vec1.assign(2, 4.0);

US_Vector zeroVec(3, 0.0);

vec1.mult(zeroVec);

EXPECT_EQ(vec1[0], 0.0);
EXPECT_EQ(vec1[1], 0.0);
EXPECT_EQ(vec1[2], 0.0);
}

TEST_F(US_VectorMultTest, MultipliesWithOnesVector) {
US_Vector vec1(3);
vec1.assign(0, 2.0);
vec1.assign(1, 3.0);
vec1.assign(2, 4.0);

US_Vector onesVec(3, 1.0);

vec1.mult(onesVec);

EXPECT_EQ(vec1[0], 2.0);
EXPECT_EQ(vec1[1], 3.0);
EXPECT_EQ(vec1[2], 4.0);
}

TEST_F(US_VectorMultTest, MultipliesWithNegativeValues) {
US_Vector vec1(2);
vec1.assign(0, 3.0);
vec1.assign(1, -4.0);

US_Vector vec2(2);
vec2.assign(0, -2.0);
vec2.assign(1, 5.0);

vec1.mult(vec2);

EXPECT_EQ(vec1[0], -6.0);
EXPECT_EQ(vec1[1], -20.0);
}

TEST_F(US_VectorMultTest, MultipliesEmptyVectors) {
US_Vector vec1(0);
US_Vector vec2(0);

vec1.mult(vec2);

EXPECT_EQ(vec1.size(), 0);
}

// Tests for scale method
class US_VectorScaleTest : public US_VectorTest {};

TEST_F(US_VectorScaleTest, ScalesVectorByPositiveValue) {
US_Vector vec(3);
vec.assign(0, 2.0);
vec.assign(1, 3.0);
vec.assign(2, 4.0);

vec.scale(2.5);

EXPECT_EQ(vec[0], 5.0);
EXPECT_EQ(vec[1], 7.5);
EXPECT_EQ(vec[2], 10.0);
}

TEST_F(US_VectorScaleTest, ScalesVectorByNegativeValue) {
US_Vector vec(3);
vec.assign(0, 2.0);
vec.assign(1, 3.0);
vec.assign(2, 4.0);

vec.scale(-1.0);

EXPECT_EQ(vec[0], -2.0);
EXPECT_EQ(vec[1], -3.0);
EXPECT_EQ(vec[2], -4.0);
}

TEST_F(US_VectorScaleTest, ScalesVectorByZero) {
US_Vector vec(3);
vec.assign(0, 2.0);
vec.assign(1, 3.0);
vec.assign(2, 4.0);

vec.scale(0.0);

EXPECT_EQ(vec[0], 0.0);
EXPECT_EQ(vec[1], 0.0);
EXPECT_EQ(vec[2], 0.0);
}

TEST_F(US_VectorScaleTest, ScalesVectorByOne) {
US_Vector vec(3);
vec.assign(0, 2.0);
vec.assign(1, 3.0);
vec.assign(2, 4.0);

vec.scale(1.0);

EXPECT_EQ(vec[0], 2.0);
EXPECT_EQ(vec[1], 3.0);
EXPECT_EQ(vec[2], 4.0);
}

TEST_F(US_VectorScaleTest, ScalesEmptyVector) {
US_Vector vec(0);

vec.scale(5.0);

EXPECT_EQ(vec.size(), 0);
}

TEST_F(US_VectorScaleTest, ScalesVectorByVerySmallValue) {
US_Vector vec(2);
vec.assign(0, 1e10);
vec.assign(1, 2e10);

vec.scale(1e-15);

expectDoubleEqual(vec[0], 1e-5);
expectDoubleEqual(vec[1], 2e-5);
}

// Tests for distance method
class US_VectorDistanceTest : public US_VectorTest {};

TEST_F(US_VectorDistanceTest, CalculatesDistanceBetweenIdenticalVectors) {
US_Vector vec1(3);
vec1.assign(0, 1.0);
vec1.assign(1, 2.0);
vec1.assign(2, 3.0);

US_Vector vec2(3);
vec2.assign(0, 1.0);
vec2.assign(1, 2.0);
vec2.assign(2, 3.0);

double result = vec1.distance(vec2);

EXPECT_EQ(result, 0.0);
}

TEST_F(US_VectorDistanceTest, CalculatesDistanceAlongSingleAxis) {
US_Vector vec1(3);
vec1.assign(0, 0.0);
vec1.assign(1, 0.0);
vec1.assign(2, 0.0);

US_Vector vec2(3);
vec2.assign(0, 3.0);
vec2.assign(1, 0.0);
vec2.assign(2, 0.0);

double result = vec1.distance(vec2);

EXPECT_EQ(result, 3.0);
}

TEST_F(US_VectorDistanceTest, CalculatesEuclideanDistance) {
US_Vector vec1(2);
vec1.assign(0, 0.0);
vec1.assign(1, 0.0);

US_Vector vec2(2);
vec2.assign(0, 3.0);
vec2.assign(1, 4.0);

double result = vec1.distance(vec2);

EXPECT_EQ(result, 5.0); // sqrt(3^2 + 4^2) = 5
}

TEST_F(US_VectorDistanceTest, CalculatesDistanceWithNegativeValues) {
US_Vector vec1(2);
vec1.assign(0, 1.0);
vec1.assign(1, 2.0);

US_Vector vec2(2);
vec2.assign(0, -2.0);
vec2.assign(1, 6.0);

double result = vec1.distance(vec2);

expectDoubleEqual(result, 5.0); // sqrt((-3)^2 + 4^2) = 5
}

TEST_F(US_VectorDistanceTest, CalculatesDistanceForEmptyVectors) {
US_Vector vec1(0);
US_Vector vec2(0);

double result = vec1.distance(vec2);

EXPECT_EQ(result, 0.0);
}

TEST_F(US_VectorDistanceTest, CalculatesDistanceIsCommutative) {
US_Vector vec1(3);
vec1.assign(0, 1.0);
vec1.assign(1, 2.0);
vec1.assign(2, 3.0);

US_Vector vec2(3);
vec2.assign(0, 4.0);
vec2.assign(1, 5.0);
vec2.assign(2, 6.0);

double dist1 = vec1.distance(vec2);
double dist2 = vec2.distance(vec1);

expectDoubleEqual(dist1, dist2);
}

// Tests for L2norm method
class US_VectorL2NormTest : public US_VectorTest {};

TEST_F(US_VectorL2NormTest, CalculatesNormOfZeroVector) {
US_Vector vec(3, 0.0);

double result = vec.L2norm();

EXPECT_EQ(result, 0.0);
}

TEST_F(US_VectorL2NormTest, CalculatesNormOfUnitVector) {
US_Vector vec(3);
vec.assign(0, 1.0);
vec.assign(1, 0.0);
vec.assign(2, 0.0);

double result = vec.L2norm();

EXPECT_EQ(result, 1.0);
}

TEST_F(US_VectorL2NormTest, CalculatesNormOfKnownVector) {
US_Vector vec(2);
vec.assign(0, 3.0);
vec.assign(1, 4.0);

double result = vec.L2norm();

EXPECT_EQ(result, 5.0); // sqrt(3^2 + 4^2) = 5
}

TEST_F(US_VectorL2NormTest, CalculatesNormWithNegativeValues) {
US_Vector vec(2);
vec.assign(0, -3.0);
vec.assign(1, 4.0);

double result = vec.L2norm();

EXPECT_EQ(result, 5.0); // sqrt((-3)^2 + 4^2) = 5
}

TEST_F(US_VectorL2NormTest, CalculatesNormOfEmptyVector) {
US_Vector vec(0);

double result = vec.L2norm();

EXPECT_EQ(result, 0.0);
}

TEST_F(US_VectorL2NormTest, CalculatesNormOfLargeVector) {
US_Vector vec(1000, 1.0);

double result = vec.L2norm();

expectDoubleEqual(result, sqrt(1000.0));
}

TEST_F(US_VectorL2NormTest, CalculatesNormWithVerySmallValues) {
US_Vector vec(2);
vec.assign(0, 1e-10);
vec.assign(1, 1e-10);

double result = vec.L2norm();

expectDoubleEqual(result, sqrt(2.0) * 1e-10);
}

// Tests for size method
class US_VectorSizeTest : public US_VectorTest {};

TEST_F(US_VectorSizeTest, ReturnsCorrectSizeForNonEmptyVector) {
US_Vector vec(5, 1.0);

EXPECT_EQ(vec.size(), 5);
}

TEST_F(US_VectorSizeTest, ReturnsZeroForEmptyVector) {
US_Vector vec(0);

EXPECT_EQ(vec.size(), 0);
}

TEST_F(US_VectorSizeTest, ReturnsSizeAfterModification) {
US_Vector vec(3, 1.0);
vec.add(2.0);

EXPECT_EQ(vec.size(), 3);
}

// Tests for assign method
class US_VectorAssignTest : public US_VectorTest {};

TEST_F(US_VectorAssignTest, AssignsValueToValidIndex) {
US_Vector vec(3, 0.0);

vec.assign(0, 1.5);
vec.assign(1, 2.5);
vec.assign(2, 3.5);

EXPECT_EQ(vec[0], 1.5);
EXPECT_EQ(vec[1], 2.5);
EXPECT_EQ(vec[2], 3.5);
}

TEST_F(US_VectorAssignTest, AssignsNegativeValue) {
US_Vector vec(2, 0.0);

vec.assign(0, -5.5);
vec.assign(1, -10.0);

EXPECT_EQ(vec[0], -5.5);
EXPECT_EQ(vec[1], -10.0);
}

TEST_F(US_VectorAssignTest, AssignsZeroValue) {
US_Vector vec(2, 1.0);

vec.assign(0, 0.0);
vec.assign(1, 0.0);

EXPECT_EQ(vec[0], 0.0);
EXPECT_EQ(vec[1], 0.0);
}

TEST_F(US_VectorAssignTest, OverwritesExistingValue) {
US_Vector vec(2, 5.0);

vec.assign(0, 10.0);

EXPECT_EQ(vec[0], 10.0);
EXPECT_EQ(vec[1], 5.0); // unchanged
}

// Tests for operator[] method
class US_VectorOperatorIndexTest : public US_VectorTest {};

TEST_F(US_VectorOperatorIndexTest, RetrievesValueAtValidIndex) {
US_Vector vec(3);
vec.assign(0, 1.5);
vec.assign(1, 2.5);
vec.assign(2, 3.5);

EXPECT_EQ(vec[0], 1.5);
EXPECT_EQ(vec[1], 2.5);
EXPECT_EQ(vec[2], 3.5);
}

TEST_F(US_VectorOperatorIndexTest, RetrievesNegativeValue) {
US_Vector vec(2);
vec.assign(0, -1.5);
vec.assign(1, -2.5);

EXPECT_EQ(vec[0], -1.5);
EXPECT_EQ(vec[1], -2.5);
}

TEST_F(US_VectorOperatorIndexTest, RetrievesZeroValue) {
US_Vector vec(2, 0.0);

EXPECT_EQ(vec[0], 0.0);
EXPECT_EQ(vec[1], 0.0);
}

TEST_F(US_VectorOperatorIndexTest, RetrievesAfterModification) {
US_Vector vec(3, 1.0);
vec.scale(2.0);

EXPECT_EQ(vec[0], 2.0);
EXPECT_EQ(vec[1], 2.0);
EXPECT_EQ(vec[2], 2.0);
}

TEST_F(US_VectorOperatorIndexTest, IsConstMethod) {
const US_Vector vec(2, 5.0);

EXPECT_EQ(vec[0], 5.0);
EXPECT_EQ(vec[1], 5.0);
}