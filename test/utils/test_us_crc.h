// test_us_crc.h
#ifndef TEST_US_CRC_H
#define TEST_US_CRC_H

#include <gtest/gtest.h>
#include "qt_test_base.h"

// Test fixture class for US_Crc - inherits from your QtTestBase
class TestUSCrc : public QtTestBase {
protected:
    void SetUp() override;
    void TearDown() override;

    // Suite-level setup and cleanup declarations
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};

#endif // TEST_US_CRC_H