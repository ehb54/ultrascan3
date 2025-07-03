// test_us_analyte.h - Fixed version with declarations only
#ifndef TEST_US_ANALYTE_H
#define TEST_US_ANALYTE_H

#include <gtest/gtest.h>
#include "qt_test_base.h"
#include "us_analyte.h"

// Test fixture class - inherits from your QtTestBase
class TestUSAnalyte : public QtTestBase {
protected:
    void SetUp() override;
    void TearDown() override;

    // Suite-level setup and cleanup declarations only
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};

#endif // TEST_US_ANALYTE_H