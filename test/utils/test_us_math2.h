#ifndef TESTUSMATH2_H
#define TESTUSMATH2_H

#include <gtest/gtest.h>
#include "qt_test_base.h"
// Forward declarations to avoid multiple inclusion issues
class US_Math2;
class US_DataIO;
class US_Matrix;

// Test fixture class for US_Math2 - inherits from your QtTestBase
class TestUSMath2 : public QtTestBase {
protected:
    void SetUp() override;
    void TearDown() override;

    // Suite-level setup and cleanup declarations
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};

#endif // TESTUSMATH2_H