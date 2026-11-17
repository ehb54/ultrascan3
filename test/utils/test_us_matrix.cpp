#include "qt_test_base.h"
#include "us_matrix.h"
#include <QVector>

using ::testing::_;
using ::testing::Return;
using ::testing::DoubleNear;

class TestUSMatrixUnit : public QtTestBase
{
   protected:
      void SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );

         // Set up test matrix (3x3 identity matrix)
         vecMatrix.clear();
         dataMatrix.clear();
         testMatrix = US_Matrix::construct( vecMatrix, dataMatrix, 3, 3 );

         // Initialize as identity matrix
         for ( int i = 0; i < 3; i++ )
         {
            for ( int j = 0; j < 3; j++ )
            {
               testMatrix[i][j] = ( i == j ) ? 1.0 : 0.0;
            }
         }

         // Set up test vectors
         testVec1 = { 1.0, 2.0, 3.0 };
         testVec2 = { 4.0, 5.0, 6.0 };
      }

      void TearDown() override
      {
         QtTestBase::TearDown();
         // Cleanup is automatic with QVector
      }

      // Test data members
      QVector<double*>        vecMatrix;
      QVector<double>         dataMatrix;
      double**                testMatrix;
      QVector<double>         testVec1;
      QVector<double>         testVec2;

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

TEST_F( TestUSMatrixUnit, LUBackSubstituteSolvesTheFactoredSystem )
{
   // A holds L below the diagonal (unit diagonal implied) and U on and above it.
   QVector<double*> vecA;
   QVector<double>  dataA;
   double**         A = US_Matrix::construct( vecA, dataA, 2, 2 );
   QVector<int>     index{ 0, 1 };

   // L = I, U = [[2, 1], [0, 1]]: the solution of U x = (3, 1) is (1, 1).
   A[0][0] = 2.0;
   A[0][1] = 1.0;
   A[1][0] = 0.0;
   A[1][1] = 1.0;
   QVector<double> b{ 3.0, 1.0 };
   double*         bPtr = b.data();
   US_Matrix::LU_BackSubstitute( A, bPtr, index.data(), 2 );
   EXPECT_DOUBLE_EQ( 1.0, b[0] );
   EXPECT_DOUBLE_EQ( 1.0, b[1] );

   // L = [[1, 0], [0.5, 1]], U as above, so L U = [[2, 1], [1, 1.5]] and (L U) (1, 2) = (4, 4).
   A[1][0] = 0.5;
   b       = { 4.0, 4.0 };
   bPtr    = b.data();
   US_Matrix::LU_BackSubstitute( A, bPtr, index.data(), 2 );
   EXPECT_DOUBLE_EQ( 1.0, b[0] );
   EXPECT_DOUBLE_EQ( 2.0, b[1] );
}

// ============================================================================
// EDGE CASES AND BOUNDARY CONDITIONS
// ============================================================================

TEST_F(TestUSMatrixUnit, ZeroDimensionOperations) {
// Test operations with zero dimensions
QVector<double> emptyVec;
double          a[] = { 1.0, 2.0 };
double          b[] = { 3.0, 4.0 };
double          c[] = { -7.0, -8.0 };

US_Matrix::vsum( a, b, c, 0 );
EXPECT_DOUBLE_EQ( c[0], -7.0 ) << "A zero-size sum writes no output";
EXPECT_DOUBLE_EQ( c[1], -8.0 );

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
