#ifndef TEST_US_ANA_PROFILE_H
#define TEST_US_ANA_PROFILE_H

#include <gtest/gtest.h>
#include "qt_test_base.h"

// Test fixture class - inherits from your QtTestBase
class TestUSAnaProfile : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        // Any specific setup for AnaProfile tests
    }

    void TearDown() override {
        // Any specific cleanup for AnaProfile tests
        QtTestBase::TearDown();
    }

    // Optional: Suite-level setup
    static void SetUpTestSuite() {
        QtTestBase::SetUpTestSuite();
        // One-time setup for all AnaProfile tests
    }

    static void TearDownTestSuite() {
        // One-time cleanup for all AnaProfile tests
    }
};

#endif // TEST_US_ANA_PROFILE_H