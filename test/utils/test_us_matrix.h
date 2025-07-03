#ifndef TEST_US_MATRIX_H
#define TEST_US_MATRIX_H

#include <gtest/gtest.h>
#include "qt_test_base.h"
// Forward declaration to avoid multiple inclusion issues
class US_Matrix;

// Test fixture class for US_Matrix - inherits from your QtTestBase
class TestUSMatrix : public QtTestBase {
protected:
    void SetUp() override;
    void TearDown() override;

    // Suite-level setup and cleanup declarations
    static void SetUpTestSuite();
    static void TearDownTestSuite();

protected:
    // Helper methods for matrix operations
    void cleanupMatrix(QVector<double*>& vecA, QVector<double>& datA);
    bool isSymmetricPositiveDefinite(double** matrix, int n);
};

#endif // TEST_US_MATRIX_H