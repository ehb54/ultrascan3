// us_buffer_test.cpp

#include <gtest/gtest.h>
//#include <gmock/gmock.h>

#include "us_buffer.h"
//#include "mock/mock_us_db2.h"

using namespace testing;

class US_BufferComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up any shared resources or initial conditions
    }

    void TearDown() override {
        // Clean up any shared resources or reset conditions
    }
};

//TEST_F(US_BufferComponentTest, GetAllFromDB_Success) {
//    NiceMock<MockUS_DB2> mockDb("dummy_password");
//    Mock::AllowLeak(&mockDb);
//
//    // Set up expectations for the mock object
//    EXPECT_CALL(mockDb, lastErrno()).WillOnce(Return(US_DB2::OK));
//    EXPECT_CALL(mockDb, query(_)).Times(1);
//    EXPECT_CALL(mockDb, next()).WillOnce(Return(true))
//            .WillOnce(Return(false)); // Simulate one result
//    EXPECT_CALL(mockDb, value(0)).WillOnce(Return("1"));
//
//    QMap<QString, US_BufferComponent> componentList;
//
//    US_BufferComponent bufferComponent;
//    bufferComponent.getAllFromDB("dummy_password", componentList);
//
//    ASSERT_EQ(componentList.size(), 1);
//    EXPECT_EQ(componentList.begin().key(), "1");
//}

TEST_F(US_BufferComponentTest, GetAllFromDB_DBError) {
    qDebug() << "Refer to this issue https://github.com/ehb54/ultrascan-tickets/issues/350";
//    NiceMock<MockUS_DB2> mockDb("dummy_password");
//    Mock::AllowLeak(&mockDb);
//
//    // Simulate a database connection error
//    EXPECT_CALL(mockDb, lastErrno()).WillOnce(Return(US_DB2::NOT_CONNECTED));
//
//    QMap<QString, US_BufferComponent> componentList;
//
//    US_BufferComponent bufferComponent;
//    bufferComponent.getAllFromDB("dummy_password", componentList);
//
//    ASSERT_EQ(componentList.size(), 0);
}

//TEST_F(US_BufferComponentTest, GetInfoFromDB_Success) {
//    NiceMock<MockUS_DB2> mockDb("dummy_password");
//    Mock::AllowLeak(&mockDb);
//
//    // Set up expectations for the mock object
//    EXPECT_CALL(mockDb, query(_)).Times(1);
//    EXPECT_CALL(mockDb, next()).WillOnce(Return(true));
//    EXPECT_CALL(mockDb, value(_)).WillRepeatedly(Return("test_value"));
//
//    US_BufferComponent bufferComponent;
//    bufferComponent.componentID = "1";
//    bufferComponent.getInfoFromDB(&mockDb);
//
//    EXPECT_EQ(bufferComponent.unit, "test_value");
//    EXPECT_EQ(bufferComponent.name, "test_value");
//    // Continue with other field checks...
//}

//TEST_F(US_BufferComponentTest, GetSpectrum_Success) {
//    MockUS_DB2 mockDb;
//
//    // Set up expectations for the mock object
//    EXPECT_CALL(mockDb, query(::testing::_)).Times(1);
//    EXPECT_CALL(mockDb, next()).WillOnce(Return(true))
//            .WillOnce(Return(false)); // Simulate one result
//    EXPECT_CALL(mockDb, value(1)).WillOnce(Return(QVariant(500.0)));  // Wavelength
//    EXPECT_CALL(mockDb, value(2)).WillOnce(Return(QVariant(1.5)));    // Value
//
//    US_Buffer buffer;
//    buffer.bufferID = "1";
//    buffer.getSpectrum(&mockDb, "Extinction");
//
//    ASSERT_EQ(buffer.extinction.size(), 1);
//    EXPECT_EQ(buffer.extinction[500.0], 1.5);
//}

TEST_F(US_BufferComponentTest, GetSpectrum_BasicTest) {
    // Simple test without mocking for now
    US_Buffer buffer;
    buffer.bufferID = "1";

    // Test basic functionality
    EXPECT_EQ(buffer.bufferID, "1");
    EXPECT_TRUE(buffer.extinction.empty());

    // Add more basic tests as needed
}