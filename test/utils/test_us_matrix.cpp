#include "test_us_matrix.h"
#include "us_matrix.h"
#include <QVector>
#include <cmath>

// Use your custom Qt matchers
using namespace qt_matchers;

// TestUSMatrix method implementations
void TestUSMatrix::SetUp() {
    QtTestBase::SetUp();
    // Per-test setup for Matrix tests
}

void TestUSMatrix::TearDown() {
    // Per-test cleanup for Matrix tests
    QtTestBase::TearDown();
}

// Suite-level setup for Matrix tests
void TestUSMatrix::SetUpTestSuite() {
    QtTestBase::SetUpTestSuite();
    // One-time setup for all TestUSMatrix tests
}

// Suite-level cleanup for Matrix tests
void TestUSMatrix::TearDownTestSuite() {
    // One-time cleanup for all TestUSMatrix tests
}

// Helper method to cleanup matrix memory
void TestUSMatrix::cleanupMatrix(QVector<double*>& vecA, QVector<double>& datA) {
    // The construct method manages memory through the vectors
    // No explicit cleanup needed as QVector handles it
    vecA.clear();
    datA.clear();
}

// Helper method to check if matrix is symmetric and positive definite
bool TestUSMatrix::isSymmetricPositiveDefinite(double** matrix, int n) {
    // Check symmetry
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (std::abs(matrix[i][j] - matrix[j][i]) > 1e-10) {
                return false;
            }
        }
    }

    // Check positive definiteness by ensuring all diagonal elements are positive
    // (simplified check - full check would require eigenvalues)
    for (int i = 0; i < n; i++) {
        if (matrix[i][i] <= 0) {
            return false;
        }
    }

    return true;
}

TEST_F(TestUSMatrix, Lsfit) {
    // Test data: y = 2x + 1 (perfect linear relationship)
    double x[] = {1, 2, 3, 4, 5};
    double y[] = {3, 5, 7, 9, 11};
    int N = 5;
    int order = 2; // Linear fit (y = c[1]*x + c[0])

    double c[2] = {0};

    bool status = US_Matrix::lsfit(c, x, y, N, order);

    EXPECT_TRUE(status) << "Least squares fit should succeed for valid data";

    const double tolerance = 0.0001;

    // Expected coefficients: c[1] (slope) = 2, c[0] (intercept) = 1
    EXPECT_NEAR(c[0], 1.0, tolerance)
                        << "Intercept should be 1.0 for y = 2x + 1";
    EXPECT_NEAR(c[1], 2.0, tolerance)
                        << "Slope should be 2.0 for y = 2x + 1";
}

TEST_F(TestUSMatrix, LsfitWithNoise) {
    // Test with slightly noisy data
    double x[] = {1, 2, 3, 4, 5};
    double y[] = {3.1, 4.9, 7.1, 8.9, 11.1}; // y ≈ 2x + 1 with small noise
    int N = 5;
    int order = 2;

    double c[2] = {0};

    bool status = US_Matrix::lsfit(c, x, y, N, order);

    EXPECT_TRUE(status) << "Least squares fit should handle noisy data";

    // Should still be close to original coefficients
    EXPECT_NEAR(c[0], 1.0, 0.2) << "Intercept should be close to 1.0 despite noise";
    EXPECT_NEAR(c[1], 2.0, 0.2) << "Slope should be close to 2.0 despite noise";
}

TEST_F(TestUSMatrix, LsfitHigherOrder) {
    // Test quadratic fit: y = x^2 + 2x + 1
    double x[] = {1, 2, 3, 4, 5};
    double y[] = {4, 9, 16, 25, 36}; // y = x^2 + 2x + 1
    int N = 5;
    int order = 3; // Quadratic fit (y = c[2]*x^2 + c[1]*x + c[0])

    double c[3] = {0};

    bool status = US_Matrix::lsfit(c, x, y, N, order);

    EXPECT_TRUE(status) << "Least squares fit should handle quadratic data";

    const double tolerance = 0.1;
    EXPECT_NEAR(c[0], 1.0, tolerance) << "Constant term should be close to 1.0";
    EXPECT_NEAR(c[1], 2.0, tolerance) << "Linear term should be close to 2.0";
    EXPECT_NEAR(c[2], 1.0, tolerance) << "Quadratic term should be close to 1.0";
}

TEST_F(TestUSMatrix, CholeskyDecomposition) {
    int n = 3;
    // Symmetric positive definite matrix
    double matrixData[3][3] = {
            {4, 12, -16},
            {12, 37, -43},
            {-16, -43, 98}
    };

    QVector<double*> vecA;
    QVector<double> datA;
    double** matrix = US_Matrix::construct(vecA, datA, n, n);

    ASSERT_NE(matrix, nullptr) << "Matrix construction should succeed";

    // Fill the matrix
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            matrix[i][j] = matrixData[i][j];
        }
    }

    // Verify matrix is symmetric positive definite before decomposition
    EXPECT_TRUE(isSymmetricPositiveDefinite(matrix, n))
                        << "Test matrix should be symmetric positive definite";

    bool result = US_Matrix::Cholesky_Decomposition(matrix, n);

    EXPECT_TRUE(result) << "Cholesky decomposition should succeed for positive definite matrix";

    // After decomposition, matrix should be lower triangular
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            EXPECT_NEAR(matrix[i][j], 0.0, 1e-10)
                                << "Upper triangular elements should be zero after decomposition";
        }
    }

    // Diagonal elements should be positive
    for (int i = 0; i < n; i++) {
        EXPECT_GT(matrix[i][i], 0.0)
                            << "Diagonal elements should be positive after decomposition";
    }
}

TEST_F(TestUSMatrix, CholeskyDecompositionIdentity) {
    // Test with identity matrix
    int n = 3;
    QVector<double*> vecA;
    QVector<double> datA;
    double** matrix = US_Matrix::construct(vecA, datA, n, n);

    // Create identity matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }

    bool result = US_Matrix::Cholesky_Decomposition(matrix, n);

    EXPECT_TRUE(result) << "Cholesky decomposition should succeed for identity matrix";

    // Identity matrix decomposition should yield identity matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double expected = (i == j) ? 1.0 : 0.0;
            EXPECT_NEAR(matrix[i][j], expected, 1e-10)
                                << "Identity matrix decomposition should yield identity matrix";
        }
    }
}

TEST_F(TestUSMatrix, CholeskySolveSystem) {
    int n = 3;
    // Symmetric positive definite matrix
    double matrixData[3][3] = {
            {4, 12, -16},
            {12, 37, -43},
            {-16, -43, 98}
    };
    double b[3] = {1, 1, 1};
    double originalB[3] = {1, 1, 1}; // Keep original for verification

    QVector<double*> vecA;
    QVector<double> datA;
    double** matrix = US_Matrix::construct(vecA, datA, n, n);

    ASSERT_NE(matrix, nullptr) << "Matrix construction should succeed";

    // Fill the matrix
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            matrix[i][j] = matrixData[i][j];
        }
    }

    bool decompResult = US_Matrix::Cholesky_Decomposition(matrix, n);
    ASSERT_TRUE(decompResult) << "Cholesky decomposition should succeed";

    bool solveResult = US_Matrix::Cholesky_SolveSystem(matrix, b, n);
    EXPECT_TRUE(solveResult) << "Cholesky solve should succeed";

    // Verify solution is not trivial
    EXPECT_NE(b[0], 0.0) << "Solution should be non-zero";
    EXPECT_NE(b[1], 0.0) << "Solution should be non-zero";
    EXPECT_NE(b[2], 0.0) << "Solution should be non-zero";

    // Verify solution is different from original b
    bool solutionChanged = false;
    for (int i = 0; i < n; i++) {
        if (std::abs(b[i] - originalB[i]) > 1e-10) {
            solutionChanged = true;
            break;
        }
    }
    EXPECT_TRUE(solutionChanged) << "Solution should be different from input vector";
}

TEST_F(TestUSMatrix, CholeskySolveSystemVerification) {
    // Test with a system where we know the expected solution
    int n = 2;
    // Matrix: [[2, 1], [1, 2]]
    // Right-hand side: [3, 3]
    // Expected solution: [1, 1] (since 2*1 + 1*1 = 3, 1*1 + 2*1 = 3)

    QVector<double*> vecA;
    QVector<double> datA;
    double** matrix = US_Matrix::construct(vecA, datA, n, n);

    matrix[0][0] = 2.0; matrix[0][1] = 1.0;
    matrix[1][0] = 1.0; matrix[1][1] = 2.0;

    double b[2] = {3.0, 3.0};

    bool decompResult = US_Matrix::Cholesky_Decomposition(matrix, n);
    ASSERT_TRUE(decompResult) << "Cholesky decomposition should succeed";

    bool solveResult = US_Matrix::Cholesky_SolveSystem(matrix, b, n);
    EXPECT_TRUE(solveResult) << "Cholesky solve should succeed";

    // Check if solution is close to expected [1, 1]
    EXPECT_NEAR(b[0], 1.0, 0.001) << "First component should be close to 1.0";
    EXPECT_NEAR(b[1], 1.0, 0.001) << "Second component should be close to 1.0";
}

TEST_F(TestUSMatrix, Construct) {
    int rows = 3;
    int columns = 4;
    QVector<double*> vecA;
    QVector<double> datA;

    double** matrix = US_Matrix::construct(vecA, datA, rows, columns);

    EXPECT_NE(matrix, nullptr) << "Matrix construction should succeed";
    EXPECT_EQ(vecA.size(), rows) << "Vector size should match number of rows";
    EXPECT_EQ(datA.size(), rows * columns) << "Data size should match total elements";

    // Test that we can write to all elements
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            matrix[i][j] = i * columns + j;
        }
    }

    // Test that we can read back the values
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            double expected = i * columns + j;
            EXPECT_EQ(matrix[i][j], expected)
                                << "Matrix element [" << i << "][" << j << "] should be accessible";
        }
    }
}

TEST_F(TestUSMatrix, ConstructEdgeCases) {
    QVector<double*> vecA;
    QVector<double> datA;

    // Test with single element
    double** matrix1 = US_Matrix::construct(vecA, datA, 1, 1);
    EXPECT_NE(matrix1, nullptr) << "1x1 matrix construction should succeed";
    EXPECT_EQ(vecA.size(), 1) << "Vector should have 1 row";
    EXPECT_EQ(datA.size(), 1) << "Data should have 1 element";

    // Clear for next test
    vecA.clear();
    datA.clear();

    // Test with rectangular matrix
    double** matrix2 = US_Matrix::construct(vecA, datA, 2, 5);
    EXPECT_NE(matrix2, nullptr) << "2x5 matrix construction should succeed";
    EXPECT_EQ(vecA.size(), 2) << "Vector should have 2 rows";
    EXPECT_EQ(datA.size(), 10) << "Data should have 10 elements";

    // Test element access
    matrix2[0][0] = 1.5;
    matrix2[1][4] = 2.5;
    EXPECT_EQ(matrix2[0][0], 1.5) << "Should be able to access first element";
    EXPECT_EQ(matrix2[1][4], 2.5) << "Should be able to access last element";
}

// Integration test that combines multiple matrix operations
TEST_F(TestUSMatrix, MatrixOperationsIntegration) {
    // Create a system and solve it using multiple matrix operations
    int n = 3;
    QVector<double*> vecA;
    QVector<double> datA;
    double** matrix = US_Matrix::construct(vecA, datA, n, n);

    ASSERT_NE(matrix, nullptr) << "Matrix construction should succeed";

    // Create a symmetric positive definite matrix
    matrix[0][0] = 4.0;  matrix[0][1] = 2.0;  matrix[0][2] = 1.0;
    matrix[1][0] = 2.0;  matrix[1][1] = 3.0;  matrix[1][2] = 0.5;
    matrix[2][0] = 1.0;  matrix[2][1] = 0.5;  matrix[2][2] = 2.0;

    // Verify it's symmetric
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            EXPECT_EQ(matrix[i][j], matrix[j][i])
                                << "Matrix should be symmetric";
        }
    }

    // Test Cholesky decomposition
    bool decompResult = US_Matrix::Cholesky_Decomposition(matrix, n);
    EXPECT_TRUE(decompResult) << "Cholesky decomposition should succeed";

    // Test system solving
    double b[3] = {7.0, 5.5, 3.5}; // Right-hand side
    bool solveResult = US_Matrix::Cholesky_SolveSystem(matrix, b, n);
    EXPECT_TRUE(solveResult) << "System solving should succeed";

    // Solution should be non-trivial
    for (int i = 0; i < n; i++) {
        EXPECT_NE(b[i], 0.0) << "Solution component " << i << " should be non-zero";
    }
}