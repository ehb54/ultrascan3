// test_us_astfem_math.h - Converted to Google Test
#ifndef TEST_US_ASTFEM_MATH_H
#define TEST_US_ASTFEM_MATH_H

#include <gtest/gtest.h>
#include "qt_test_base.h"
#include "us_astfem_math.h"

// Test fixture class for AstfemMath - inherits from your QtTestBase
class TestUSAstfemMath : public QtTestBase {
protected:
    void SetUp() override;
    void TearDown() override;

    // Suite-level setup and cleanup declarations
    static void SetUpTestSuite();
    static void TearDownTestSuite();

protected:
    // Test data members (previously private)
    QString tmst_fpath;  // Path to the timestate file used in the tests
};

#endif // TEST_US_ASTFEM_MATH_H