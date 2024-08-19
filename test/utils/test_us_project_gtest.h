#ifndef TEST_US_PROJECT_GTEST_H
#define TEST_US_PROJECT_GTEST_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "us_project.h"
#include "mock/mock_us_db2.h"

using namespace testing;

class TestUsProjectGtest : public ::testing::Test {
protected:
    // This function is called before each test
    void SetUp() override {
        // Common setup code, if needed
    }

    // This function is called after each test
    void TearDown() override {
        // Common cleanup code, if needed
    }

    // Mock object for US_DB2
    MockUS_DB2 mockDB;

    // US_Project object to test
    US_Project project;
};

// Declarations for specific test cases (if desired to be in the header)
void TestReadFromDBSuccess();
void TestReadFromDBFailure();

#endif // TEST_US_PROJECT_GTEST_H
