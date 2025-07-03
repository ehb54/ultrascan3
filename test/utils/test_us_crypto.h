// test_us_crypto.h - Converted to Google Test
#ifndef TEST_US_CRYPTO_H
#define TEST_US_CRYPTO_H

#include <gtest/gtest.h>
#include "qt_test_base.h"
// Forward declaration to avoid multiple inclusion issues
class US_Crypto;

// Test fixture class for US_Crypto - inherits from your QtTestBase
class TestUSCrypto : public QtTestBase {
protected:
    void SetUp() override;
    void TearDown() override;

    // Suite-level setup and cleanup declarations
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};

#endif // TEST_US_CRYPTO_H