// test_us_dataIO.h
#ifndef TEST_US_DATAIO_H
#define TEST_US_DATAIO_H

#include <gtest/gtest.h>
#include "qt_test_base.h"
// Forward declaration to avoid multiple inclusion issues
class US_DataIO;

// Test fixture class for US_DataIO - inherits from your QtTestBase
class TestUSDataIO : public QtTestBase {
protected:
    void SetUp() override;
    void TearDown() override;

    // Suite-level setup and cleanup declarations
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    // Note: Helper methods moved to .cpp file to avoid incomplete type issues
};

#endif // TEST_US_DATAIO_H