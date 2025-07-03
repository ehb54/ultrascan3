#ifndef TEST_US_DB2_H
#define TEST_US_DB2_H

#include <gtest/gtest.h>
#include "qt_test_base.h"
// Forward declaration to avoid multiple inclusion issues
class US_DB2;

// Test fixture class for US_DB2 - inherits from your QtTestBase
class TestUSDB2 : public QtTestBase {
protected:
    void SetUp() override;
    void TearDown() override;

    // Suite-level setup and cleanup declarations
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};

#endif // TEST_US_DB2_H