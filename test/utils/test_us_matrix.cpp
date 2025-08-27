#include "qt_test_base.h"
#include "us_matrix.h"
#include <QVector>

using ::testing::_;
using ::testing::Return;
using ::testing::DoubleNear;

class TestUSMatrixUnit : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();

        // Set up test matrix (3x3 identity matrix)
        vecMatrix.clear();
        dataMatrix.clear();
        testMatrix = US_Matrix::construct(vecMatrix, dataMatrix, 3, 3);

        // Initialize as identity matrix
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                testMatrix[i][j] = (i == j) ? 1.0 : 0.0;
            }
        }

        // Set up test vectors
        testVec1 = {1.0, 2.0, 3.0};
        testVec2 = {4.0, 5.0, 6.0};
    }

    void TearDown() override {
        QtTestBase::TearDown();
        // Cleanup is automatic with QVector
    }

    // Test data members
    QVector<double*> vecMatrix;
    QVector<double> dataMatrix;
    double** testMatrix;
    QVector<double> testVec1;
    QVector<double> testVec2;

    static constexpr double TOLERANCE = 1e-6;
};

TEST_F(TestUSMatrixUnit, LUSolveSystemBasic) {
// Create test coefficient matrix A for system: 2x + y = 3, x + y = 2
QVector<double*> vecA;
QVector<double> dataA;
double** A = US_Matrix::construct(vecA, dataA, 2, 2);

A[0][0] = 2.0; A[0][1] = 1.0;
A[1][0] = 1.0; A[1][1] = 1.0;

QVector<double> b = {3.0, 2.0};
double* bPtr = b.data();

US_Matrix::LU_SolveSystem(A, bPtr, 2);

EXPECT_NEAR(b[0], 1.0, TOLERANCE) << "Solution x[0] should be 1.0";
EXPECT_NEAR(b[1], 1.0, TOLERANCE) << "Solution x[1] should be 1.0";
}

TEST_F(TestUSMatrixUnit, LUSolveSystemIdentity) {
// Test LU solve with identity matrix
QVector<double> b = {5.0, 3.0, 7.0};
double* bPtr = b.data();

US_Matrix::LU_SolveSystem(testMatrix, bPtr, 3);

// Identity * x = b should give x = b
EXPECT_NEAR(b[0], 5.0, TOLERANCE) << "Solution should be unchanged for identity";
EXPECT_NEAR(b[1], 3.0, TOLERANCE) << "Solution should be unchanged for identity";
EXPECT_NEAR(b[2], 7.0, TOLERANCE) << "Solution should be unchanged for identity";
}

TEST_F(TestUSMatrixUnit, LUBackSubstituteBasic) {
// Test LU back substitution
QVector<double*> vecA;
QVector<double> dataA;
double** A = US_Matrix::construct(vecA, dataA, 2, 2);

// Set up upper triangular matrix for back substitution
A[0][0] = 2.0; A[0][1] = 1.0;
A[1][0] = 0.0; A[1][1] = 1.0;

QVector<double> b = {3.0, 1.0};
double* bPtr = b.data();
QVector<int> index = {0, 1};

EXPECT_NO_THROW({
US_Matrix::LU_BackSubstitute(A, bPtr, index.data(), 2);
}) << "Back substitution should not throw";
}

// ============================================================================
// EDGE CASES AND BOUNDARY CONDITIONS
// ============================================================================

TEST_F(TestUSMatrixUnit, ZeroDimensionOperations) {
// Test operations with zero dimensions
QVector<double> emptyVec;

EXPECT_NO_THROW({
US_Matrix::vsum(emptyVec.data(), emptyVec.data(), emptyVec.data(), 0);
}) << "Zero-size vector operations should not crash";

double result = US_Matrix::dotproduct(emptyVec.data(), emptyVec.data(), 0);
EXPECT_DOUBLE_EQ(result, 0.0) << "Empty vector dot product should be 0";
}

TEST_F(TestUSMatrixUnit, SingleElementOperations) {
// Test operations with single elements
QVector<double*> vecA;
QVector<double> dataA;
double** A = US_Matrix::construct(vecA, dataA, 1, 1);
A[0][0] = 5.0;

QVector<double*> vecB;
QVector<double> dataB;
double** B = US_Matrix::construct(vecB, dataB, 1, 1);
B[0][0] = 3.0;

QVector<double*> vecC;
QVector<double> dataC;
double** C = US_Matrix::construct(vecC, dataC, 1, 1);

US_Matrix::mmm(A, B, C, 1, 1, 1);
EXPECT_NEAR(C[0][0], 15.0, TOLERANCE) << "1x1 matrix multiply should work";

US_Matrix::msum(A, B, C, 1, 1);
EXPECT_NEAR(C[0][0], 8.0, TOLERANCE) << "1x1 matrix sum should work";
}

TEST_F(TestUSMatrixUnit, LargeScalarOperations) {
// Test operations with large scalar values
double largeScalar = 1e10;

US_Matrix::scale(testMatrix, largeScalar, 3, 3);

for (int i = 0; i < 3; i++) {
EXPECT_NEAR(testMatrix[i][i], largeScalar, largeScalar * 1e-10)
<< "Large scalar scaling should work";
}
}

TEST_F(TestUSMatrixUnit, SmallScalarOperations) {
// Test operations with very small scalar values
double smallScalar = 1e-15;

US_Matrix::add(testMatrix, smallScalar, 3, 3);

for (int i = 0; i < 3; i++) {
for (int j = 0; j < 3; j++) {
double expected = (i == j) ? 1.0 + smallScalar : smallScalar;
EXPECT_NEAR(testMatrix[i][j], expected, 1e-14)
<< "Small scalar addition should work";
}
}
}

TEST_F(TestUSMatrixUnit, NegativeValueOperations) {
// Test operations with negative values
QVector<double> negVec = {-1.0, -2.0, -3.0};
QVector<double> result(3);

US_Matrix::vsum(testVec1.data(), negVec.data(), result.data(), 3);

// [1,2,3] + [-1,-2,-3] should give [0,0,0]
for (int i = 0; i < 3; i++) {
EXPECT_NEAR(result[i], 0.0, TOLERANCE)
<< "Negative value operations should work";
}
}

TEST_F(TestUSMatrixUnit, MemoryLayoutConsistency) {
// Test that matrix construction creates consistent memory layout
QVector<double*> vecM;
QVector<double> vecD;
double** matrix = US_Matrix::construct(vecM, vecD, 2, 3);

// Set values and verify they can be read consistently
for (int i = 0; i < 2; i++) {
for (int j = 0; j < 3; j++) {
matrix[i][j] = i * 3 + j;
}
}

// Verify values are stored correctly
for (int i = 0; i < 2; i++) {
for (int j = 0; j < 3; j++) {
EXPECT_DOUBLE_EQ(matrix[i][j], i * 3 + j)
<< "Memory layout should be consistent";
}
}
}

TEST_F(TestUSMatrixUnit, MatrixTransposeMultiplyFullFill) {
// Test tmm with full fill option
QVector<double*> vecA;
QVector<double> dataA;
double** A = US_Matrix::construct(vecA, dataA, 2, 2);

A[0][0] = 1.0; A[0][1] = 2.0;
A[1][0] = 3.0; A[1][1] = 4.0;

QVector<double*> vecC;
QVector<double> dataC;
double** C = US_Matrix::construct(vecC, dataC, 2, 2);

US_Matrix::tmm(A, C, 2, 2, true); // With full fill

// Check that upper triangle is filled
EXPECT_NEAR(C[0][1], C[1][0], TOLERANCE)
<< "Upper triangle should be filled with full option";
}
