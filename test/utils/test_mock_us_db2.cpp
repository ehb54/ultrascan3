#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mock_us_db2.h"
#include <memory>

using namespace testing;

class US_DB2_MockTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Example 1: Simple test with natural syntax
TEST_F(US_DB2_MockTest, StringQueryWorks) {
    NiceMockDB mockDB;

    QString query = "SELECT * FROM users";

    // Natural syntax - GoogleMock figures out which overload to use
    EXPECT_CALL(mockDB, statusQuery(query))
            .WillOnce(Return(IUS_DB2::OK));

    int result = mockDB.statusQuery(query);
    EXPECT_EQ(result, IUS_DB2::OK);
}

// Example 2: Test with QStringList - also natural
TEST_F(US_DB2_MockTest, ListQueryWorks) {
    NiceMockDB mockDB;

    QStringList args{"get_user_info", "123"};

    EXPECT_CALL(mockDB, statusQuery(args))
            .WillOnce(Return(IUS_DB2::OK));

    int result = mockDB.statusQuery(args);
    EXPECT_EQ(result, IUS_DB2::OK);
}

// Example 3: Connection testing - clear which overload
TEST_F(US_DB2_MockTest, ConnectionMethods) {
    NiceMockDB mockDB;

    QString masterPW = "password123";
    QString error;

    // Master password version
    EXPECT_CALL(mockDB, connect(masterPW, Ref(error)))
            .WillOnce(Return(true));

    bool result1 = mockDB.connect(masterPW, error);
    EXPECT_TRUE(result1);

    // Full credentials version
    QString host = "localhost", db = "test", user = "user", pass = "pass";
    EXPECT_CALL(mockDB, connect(host, db, user, pass, Ref(error)))
            .WillOnce(Return(true));

    bool result2 = mockDB.connect(host, db, user, pass, error);
    EXPECT_TRUE(result2);
}

// Example 4: Using the builder for realistic scenarios
TEST_F(US_DB2_MockTest, RealisticDatabaseInteraction) {
    auto mockDB = MockBuilder::createConnectedMock();

    // Override specific behaviors as needed
    QString query = "SELECT id, name FROM users WHERE active = 1";

    EXPECT_CALL(*mockDB, query(query)).Times(1);
    EXPECT_CALL(*mockDB, next())
            .WillOnce(Return(true))   // First row
            .WillOnce(Return(true))   // Second row
            .WillOnce(Return(false)); // No more rows

    EXPECT_CALL(*mockDB, value(0))
            .WillOnce(Return(QVariant(1)))
            .WillOnce(Return(QVariant(2)));

    EXPECT_CALL(*mockDB, value(1))
            .WillOnce(Return(QVariant("Alice")))
            .WillOnce(Return(QVariant("Bob")));

    // Execute the sequence
    mockDB->query(query);

    // First row
    EXPECT_TRUE(mockDB->next());
    EXPECT_EQ(mockDB->value(0).toInt(), 1);
    EXPECT_EQ(mockDB->value(1).toString(), "Alice");

    // Second row
    EXPECT_TRUE(mockDB->next());
    EXPECT_EQ(mockDB->value(0).toInt(), 2);
    EXPECT_EQ(mockDB->value(1).toString(), "Bob");

    // No more rows
    EXPECT_FALSE(mockDB->next());
}

// Example 5: Type-specific matchers when needed
TEST_F(US_DB2_MockTest, TypeSpecificMatching) {
    NiceMockDB mockDB;

    // When you need to be explicit about types
    EXPECT_CALL(mockDB, query(An<const QString&>()))
            .Times(1);

    EXPECT_CALL(mockDB, query(An<const QStringList&>()))
            .Times(1);

    mockDB.query(QString("SELECT * FROM table"));
    mockDB.query(QStringList{"proc_name", "param1"});
}

// Example 6: Advanced sequence testing
TEST_F(US_DB2_MockTest, ComplexWorkflow) {
    auto mockDB = MockBuilder::createConnectedMock<StrictMockDB>();

    InSequence seq;

    QString masterPW = "secret";
    QString error;
    QStringList getModelQuery{"get_modelID", "test-guid"};

    // Strict sequence of operations
    EXPECT_CALL(*mockDB, connect(masterPW, Ref(error)));
    EXPECT_CALL(*mockDB, query(getModelQuery));
    EXPECT_CALL(*mockDB, lastErrno()).WillOnce(Return(IUS_DB2::OK));
    EXPECT_CALL(*mockDB, next()).WillOnce(Return(true));
    EXPECT_CALL(*mockDB, value(0)).WillOnce(Return(QVariant("42")));

    // Execute workflow
    EXPECT_TRUE(mockDB->connect(masterPW, error));
    mockDB->query(getModelQuery);
    EXPECT_EQ(mockDB->lastErrno(), IUS_DB2::OK);
    EXPECT_TRUE(mockDB->next());
    EXPECT_EQ(mockDB->value(0).toString(), "42");
}