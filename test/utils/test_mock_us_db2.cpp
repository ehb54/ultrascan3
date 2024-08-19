#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mock/mock_us_db2.h"

using namespace testing;

TEST(MockUS_DB2Test, QueryReturnsTrue) {
    // Arrange: Create a mock object
    NiceMock<MockUS_DB2> mockDB;

    QStringList args = QStringList() << "SELECT * FROM table";

    // Expect the query method to be called with any string and return true
    EXPECT_CALL(mockDB, query(_))
            .Times(1);

    // Act: Call the method
    mockDB.query(args);

}

TEST(MockUS_DB2Test, NextReturnsFalse) {
    // Arrange: Create a NiceMock object
    NiceMock<MockUS_DB2> mockDB;
    testing::Mock::AllowLeak(&mockDB);

    // Expect the next method to be called and return false
    EXPECT_CALL(mockDB, next())
            .Times(1)
            .WillOnce(Return(false));

    // Act: Call the method
    bool result = mockDB.next();

    // Assert: Verify the method's return value
    EXPECT_FALSE(result);
}

TEST(MockUS_DB2Test, ClearExpectations) {


    NiceMock<MockUS_DB2> mockDB;
    testing::Mock::AllowLeak(&mockDB);

    EXPECT_CALL(mockDB, next()).Times(1);

    // Act: Call the method
    bool result = mockDB.next();

    // Clear expectations manually
    testing::Mock::VerifyAndClearExpectations(&mockDB);
}




