#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mock/mock_us_db2.h"
#include <memory>
#include <iostream>

using namespace testing;

/**
 * @brief Test fixture for US_DB2_Mock tests
 */
class US_DB2_MockTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(US_DB2_MockTest, CanCreateMockObject) {
    // Test stack allocation
    US_DB2_Mock mockDB;

    EXPECT_CALL(mockDB, isConnected())
            .WillOnce(Return(false));

    bool result = mockDB.isConnected();
    EXPECT_FALSE(result);
}

TEST_F(US_DB2_MockTest, HeapAllocation) {
    auto mockDB = std::make_unique<US_DB2_Mock>();

    EXPECT_CALL(*mockDB, isConnected())
            .WillOnce(Return(true));

    bool result = mockDB->isConnected();
    EXPECT_TRUE(result);
}

TEST_F(US_DB2_MockTest, QueryWithStringWorks) {
    US_DB2_MockNice mockDB;

    QString sqlQuery = "SELECT * FROM table";

    EXPECT_CALL(mockDB, statusQueryString(sqlQuery))  // Use specific mock method
            .Times(1)
            .WillOnce(Return(0));

    int result = mockDB.statusQuery(sqlQuery);  // Interface method delegates to mock method
    EXPECT_EQ(result, 0);
}

TEST_F(US_DB2_MockTest, QueryWithListWorks) {
    US_DB2_MockNice mockDB;

    QStringList args = QStringList() << "get_user_info" << "123";

    EXPECT_CALL(mockDB, statusQueryStringList(args))  // Use specific mock method
            .Times(1)
            .WillOnce(Return(0));

    int result = mockDB.statusQuery(args);  // Interface method delegates to mock method
    EXPECT_EQ(result, 0);
}

TEST_F(US_DB2_MockTest, NextAndValueWork) {
    US_DB2_MockNice mockDB;

    // Test next() method
    EXPECT_CALL(mockDB, next())
            .Times(1)
            .WillOnce(Return(true));

    bool nextResult = mockDB.next();
    EXPECT_TRUE(nextResult);

    // Test value() method
    EXPECT_CALL(mockDB, value(0))
            .WillOnce(Return(QVariant("test_value")));

    QVariant valueResult = mockDB.value(0);
    EXPECT_EQ(valueResult.toString(), "test_value");
}

TEST_F(US_DB2_MockTest, ConnectionMethods) {
    US_DB2_MockNice mockDB;

    QString masterPW = "password123";
    QString error;

    EXPECT_CALL(mockDB, connect(masterPW, ::testing::Ref(error)))
            .WillOnce(DoAll(SetArgReferee<1>(""), Return(true)));

    bool result = mockDB.connect(masterPW, error);
    EXPECT_TRUE(result);
    EXPECT_TRUE(error.isEmpty());

    // Test the 5-parameter version
    QString host = "localhost", dbname = "test_db", user = "testuser", password = "testpass";
    EXPECT_CALL(mockDB, connect(host, dbname, user, password, ::testing::Ref(error)))
            .WillOnce(DoAll(SetArgReferee<4>(""), Return(true)));

    bool result2 = mockDB.connect(host, dbname, user, password, error);
    EXPECT_TRUE(result2);
}

TEST_F(US_DB2_MockTest, ErrorHandling) {
    US_DB2_MockNice mockDB;

    EXPECT_CALL(mockDB, lastErrno()).WillOnce(Return(404));
    EXPECT_CALL(mockDB, lastError()).WillOnce(Return(QString("Test error message")));

    EXPECT_EQ(mockDB.lastErrno(), 404);
    EXPECT_EQ(mockDB.lastError(), "Test error message");
}

TEST_F(US_DB2_MockTest, BlobOperations) {
    US_DB2_MockNice mockDB;

    QString filename = "/tmp/test.dat";
    QString procedure = "upload_test";
    int tableID = 123;

    // Test write blob
    EXPECT_CALL(mockDB, writeBlobToDB(filename, procedure, tableID))
            .WillOnce(Return(0)); // 0 = success

    int writeResult = mockDB.writeBlobToDB(filename, procedure, tableID);
    EXPECT_EQ(writeResult, 0);

    // Test read blob
    EXPECT_CALL(mockDB, readBlobFromDB(filename, procedure, tableID))
            .WillOnce(Return(0)); // 0 = success

    int readResult = mockDB.readBlobFromDB(filename, procedure, tableID);
    EXPECT_EQ(readResult, 0);
}

TEST_F(US_DB2_MockTest, AucDataOperations) {
    US_DB2_MockNice mockDB;

    QString filename = "/tmp/test.auc";
    int tableID = 456;

    // Test write AUC data
    EXPECT_CALL(mockDB, writeAucToDB(filename, tableID))
            .WillOnce(Return(0));

    int writeResult = mockDB.writeAucToDB(filename, tableID);
    EXPECT_EQ(writeResult, 0);

    // Test read AUC data
    EXPECT_CALL(mockDB, readAucFromDB(filename, tableID))
            .WillOnce(Return(0));

    int readResult = mockDB.readAucFromDB(filename, tableID);
    EXPECT_EQ(readResult, 0);
}

TEST_F(US_DB2_MockTest, DatabaseTestMethods) {
    US_DB2_MockNice mockDB;

    QString host = "testhost", dbname = "testdb", user = "testuser", password = "testpass";
    QString error;

    // Test basic connection test
    EXPECT_CALL(mockDB, test_db_connection(host, dbname, user, password, ::testing::Ref(error)))
            .WillOnce(DoAll(SetArgReferee<4>(""), Return(true)));

    bool basicTest = mockDB.test_db_connection(host, dbname, user, password, error);
    EXPECT_TRUE(basicTest);

    // Test secure connection test
    QString email = "test@example.com", userPW = "userpass";
    EXPECT_CALL(mockDB, test_secure_connection(host, dbname, user, password, email, userPW, ::testing::Ref(error)))
            .WillOnce(DoAll(SetArgReferee<6>(""), Return(true)));

    bool secureTest = mockDB.test_secure_connection(host, dbname, user, password, email, userPW, error);
    EXPECT_TRUE(secureTest);
}

TEST_F(US_DB2_MockTest, MultipleOperationsSequence) {
    US_DB2_MockNice mockDB;

    // Simulate a typical database operation sequence
    InSequence seq;

    QString masterPW = "password";
    QString error;
    QString query = "SELECT id, name FROM users";

    EXPECT_CALL(mockDB, connect(masterPW, ::testing::Ref(error))).WillOnce(Return(true));
    EXPECT_CALL(mockDB, statusQueryString(query)).WillOnce(Return(0));  // Use specific mock method
    EXPECT_CALL(mockDB, next()).WillOnce(Return(true));
    EXPECT_CALL(mockDB, value(0)).WillOnce(Return(QVariant(42)));
    EXPECT_CALL(mockDB, value(1)).WillOnce(Return(QVariant("test_name")));
    EXPECT_CALL(mockDB, next()).WillOnce(Return(false)); // No more rows

    // Execute the sequence using interface methods (which delegate to mock methods)
    EXPECT_TRUE(mockDB.connect(masterPW, error));
    EXPECT_EQ(mockDB.statusQuery(query), 0);
    EXPECT_TRUE(mockDB.next());
    EXPECT_EQ(mockDB.value(0).toInt(), 42);
    EXPECT_EQ(mockDB.value(1).toString(), "test_name");
    EXPECT_FALSE(mockDB.next());
}

TEST_F(US_DB2_MockTest, StrictMockExample) {
    // StrictMock will fail if any unexpected methods are called
    US_DB2_MockStrict mockDB;

    EXPECT_CALL(mockDB, isConnected()).WillOnce(Return(true));

    // This will pass
    EXPECT_TRUE(mockDB.isConnected());

    // If we called any other method without setting expectations,
    // the StrictMock would fail the test
}

TEST_F(US_DB2_MockTest, NiceMockExample) {
    // NiceMock will not complain about unexpected method calls
    US_DB2_MockNice mockDB;

    EXPECT_CALL(mockDB, isConnected()).WillOnce(Return(true));

    // These will pass without warnings
    EXPECT_TRUE(mockDB.isConnected());

    // These calls don't have expectations but won't cause warnings
    mockDB.lastErrno();  // Returns default value
    mockDB.numRows();    // Returns default value
}

TEST_F(US_DB2_MockTest, ExpectationManagement) {
    auto mockDB = std::make_unique<US_DB2_Mock>();

    // Set initial expectation
    EXPECT_CALL(*mockDB, next()).Times(1).WillOnce(Return(true));
    bool result = mockDB->next();
    EXPECT_TRUE(result);

    // Clear expectations manually
    Mock::VerifyAndClearExpectations(mockDB.get());

    // Set new expectations
    EXPECT_CALL(*mockDB, next()).Times(1).WillOnce(Return(false));
    bool result2 = mockDB->next();
    EXPECT_FALSE(result2);
}

TEST_F(US_DB2_MockTest, InterfacePolymorphism) {
    // Test that the mock works through the interface
    std::unique_ptr<IUS_DB2> db = std::make_unique<US_DB2_Mock>();

    auto* mockPtr = dynamic_cast<US_DB2_Mock*>(db.get());
    ASSERT_NE(mockPtr, nullptr);

    EXPECT_CALL(*mockPtr, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(*mockPtr, lastErrno()).WillOnce(Return(0));

    // Use through interface
    EXPECT_TRUE(db->isConnected());
    EXPECT_EQ(db->lastErrno(), 0);
}

TEST_F(US_DB2_MockTest, FunctionQueryTest) {
    US_DB2_MockNice mockDB;

    QStringList args;
    args << "count_users" << "active";

    EXPECT_CALL(mockDB, functionQuery(args))
            .WillOnce(Return(5));

    int count = mockDB.functionQuery(args);
    EXPECT_EQ(count, 5);
}

TEST_F(US_DB2_MockTest, RawQueryTest) {
    US_DB2_MockNice mockDB;

    QString rawSQL = "SHOW TABLES";

    EXPECT_CALL(mockDB, rawQuery(rawSQL))
            .Times(1);

    // Raw query doesn't return a value
    mockDB.rawQuery(rawSQL);
    // Test passes if no exceptions thrown
}

TEST_F(US_DB2_MockTest, NumRowsTest) {
    US_DB2_MockNice mockDB;

    EXPECT_CALL(mockDB, numRows())
            .WillOnce(Return(10));

    int rows = mockDB.numRows();
    EXPECT_EQ(rows, 10);
}

TEST_F(US_DB2_MockTest, LastInsertIDTest) {
    US_DB2_MockNice mockDB;

    EXPECT_CALL(mockDB, lastInsertID())
            .WillOnce(Return(999));

    int insertID = mockDB.lastInsertID();
    EXPECT_EQ(insertID, 999);
}

TEST_F(US_DB2_MockTest, LastDebugTest) {
    US_DB2_MockNice mockDB;

    EXPECT_CALL(mockDB, lastDebug())
            .WillOnce(Return(QString("Debug message")));

    QString debug = mockDB.lastDebug();
    EXPECT_EQ(debug, "Debug message");
}

TEST_F(US_DB2_MockTest, EscapeStringTest) {
    US_DB2_MockNice mockDB;

    QByteArray to, from("test'string");

    EXPECT_CALL(mockDB, mysqlEscapeString(::testing::Ref(to), ::testing::Ref(from), static_cast<unsigned long>(from.length())))
            .WillOnce(Return(12));  // Length of escaped string

    unsigned long result = mockDB.mysqlEscapeString(to, from, from.length());
    EXPECT_EQ(result, 12);
}

TEST_F(US_DB2_MockTest, QueryStringDelegation) {
    US_DB2_MockNice mockDB;

    QString query = "SELECT * FROM users";

    // Mock the specific method that gets called by delegation
    EXPECT_CALL(mockDB, queryString(query))
            .Times(1);

    // Call the interface method (which should delegate)
    mockDB.query(query);
}

TEST_F(US_DB2_MockTest, QueryStringListDelegation) {
    US_DB2_MockNice mockDB;

    QStringList args;
    args << "get_users" << "active";

    // Mock the specific method that gets called by delegation
    EXPECT_CALL(mockDB, queryStringList(args))
            .Times(1);

    // Call the interface method (which should delegate)
    mockDB.query(args);
}

TEST_F(US_DB2_MockTest, StatusQueryDelegation) {
    US_DB2_MockNice mockDB;

    // Test QString version
    QString stringQuery = "UPDATE users SET active = 1";
    EXPECT_CALL(mockDB, statusQueryString(stringQuery))
            .WillOnce(Return(0));

    int result1 = mockDB.statusQuery(stringQuery);
    EXPECT_EQ(result1, 0);

    // Test QStringList version
    QStringList listQuery;
    listQuery << "update_user_status" << "123" << "active";
    EXPECT_CALL(mockDB, statusQueryStringList(listQuery))
            .WillOnce(Return(0));

    int result2 = mockDB.statusQuery(listQuery);
    EXPECT_EQ(result2, 0);
}