//! \file test_us_db2.h
#ifndef TEST_US_DB2_H
#define TEST_US_DB2_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "us_db2.h"
#include "ius_db2.h"
#include "mock/mock_us_db2.h"

/**
 * @brief Test fixture for US_DB2 tests using Google Test framework
 *
 * This replaces the old QTest-based tests with modern GTest/GMock tests
 * that work with the new interface-based dependency injection system.
 */
class TestUSDB2 : public ::testing::Test {
protected:
    /**
     * @brief Set up function called before each test
     */
    void SetUp() override;

    /**
     * @brief Tear down function called after each test
     */
    void TearDown() override;
};

/**
 * @brief Test fixture for Mock database tests
 */
class MockDatabaseTest : public ::testing::Test {
protected:
    void SetUp() override;

    std::unique_ptr<US_DB2_MockNice> mockDB;
};

/**
 * @brief Test fixture for US_DB2_Mock specific tests
 */
class US_DB2_MockTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
};

#endif // TEST_US_DB2_H