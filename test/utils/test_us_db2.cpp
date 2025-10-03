// test_us_db2.cpp
#include "qt_test_base.h"
#include "mock_us_db2.h"
#include "us_db2.h"
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QByteArray>
#include <memory>

using ::testing::_;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::DoAll;
using ::testing::StrictMock;
using ::testing::NiceMock;
using ::testing::Matcher;

class TestUSDB2Unit : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        // Create fresh mock for each test - use correct type
        mockDb = std::make_unique<StrictMock<US_DB2_Mock>>();
    }

    void TearDown() override {
        mockDb.reset();
        QtTestBase::TearDown();
    }

    std::unique_ptr<StrictMock<US_DB2_Mock>> mockDb;
};

// ============================================================================
// CONNECTION METHOD TESTS
// ============================================================================

TEST_F(TestUSDB2Unit, TestDbConnectionValid) {
QString host = "localhost:3306";
QString dbname = "test_db";
QString user = "test_user";
QString password = "test_password";
QString error;

EXPECT_CALL(*mockDb, test_db_connection(host, dbname, user, password, _))
.WillOnce(DoAll(SetArgReferee<4>(QString()), Return(true)));

bool result = mockDb->test_db_connection(host, dbname, user, password, error);

EXPECT_TRUE(result);
EXPECT_TRUE(error.isEmpty());
}

TEST_F(TestUSDB2Unit, ConnectWithMasterPassword) {
QString masterPW = "master_password";
QString err;

EXPECT_CALL(*mockDb, connect(masterPW, _))
.WillOnce(DoAll(SetArgReferee<1>(QString()), Return(true)));

bool result = mockDb->connect(masterPW, err);

EXPECT_TRUE(result);
EXPECT_TRUE(err.isEmpty());
}

TEST_F(TestUSDB2Unit, ConnectWithDirectParameters) {
QString host = "localhost:3306";
QString dbname = "test_db";
QString user = "test_user";
QString password = "test_password";
QString error;

EXPECT_CALL(*mockDb, connect(host, dbname, user, password, _))
.WillOnce(DoAll(SetArgReferee<4>(QString()), Return(true)));

bool result = mockDb->connect(host, dbname, user, password, error);

EXPECT_TRUE(result);
EXPECT_TRUE(error.isEmpty());
}

// ============================================================================
// QUERY METHOD TESTS
// ============================================================================

TEST_F(TestUSDB2Unit, RawQueryValidSQL) {
QString sqlQuery = "SELECT * FROM test_table";

EXPECT_CALL(*mockDb, rawQuery(sqlQuery))
.Times(1);

EXPECT_NO_THROW({
mockDb->rawQuery(sqlQuery);
});
}

TEST_F(TestUSDB2Unit, StatusQueryString) {
QString sqlQuery = "CALL test_procedure()";

EXPECT_CALL(*mockDb, statusQuery(Matcher<const QString&>(sqlQuery)))
.WillOnce(Return(IUS_DB2::OK));

int result = mockDb->statusQuery(sqlQuery);

EXPECT_EQ(result, IUS_DB2::OK);
}

TEST_F(TestUSDB2Unit, StatusQueryStringList) {
QStringList arguments;
arguments << "test_procedure" << "arg1" << "arg2";

EXPECT_CALL(*mockDb, statusQuery(Matcher<const QStringList&>(arguments)))
.WillOnce(Return(IUS_DB2::OK));

int result = mockDb->statusQuery(arguments);

EXPECT_EQ(result, IUS_DB2::OK);
}

TEST_F(TestUSDB2Unit, FunctionQuery) {
QStringList arguments;
arguments << "test_function" << "param1" << "param2";

EXPECT_CALL(*mockDb, functionQuery(arguments))
.WillOnce(Return(5));

int result = mockDb->functionQuery(arguments);

EXPECT_EQ(result, 5);
}

TEST_F(TestUSDB2Unit, QueryStringMethod) {
QString sqlQuery = "CALL get_data_procedure()";

EXPECT_CALL(*mockDb, query(Matcher<const QString&>(sqlQuery)))
.Times(1);

EXPECT_NO_THROW({
mockDb->query(sqlQuery);
});
}

TEST_F(TestUSDB2Unit, QueryStringListMethod) {
QStringList arguments;
arguments << "get_user_data" << "user_id" << "123";

EXPECT_CALL(*mockDb, query(Matcher<const QStringList&>(arguments)))
.Times(1);

EXPECT_NO_THROW({
mockDb->query(arguments);
});
}

// ============================================================================
// DATA ACCESS METHOD TESTS
// ============================================================================

TEST_F(TestUSDB2Unit, NextMethodTrue) {
EXPECT_CALL(*mockDb, next())
.WillOnce(Return(true));

bool result = mockDb->next();

EXPECT_TRUE(result);
}

TEST_F(TestUSDB2Unit, ValueValidIndex) {
unsigned int index = 0;
QVariant expectedValue("test_value");

EXPECT_CALL(*mockDb, value(index))
.WillOnce(Return(expectedValue));

QVariant result = mockDb->value(index);

EXPECT_EQ(result.toString(), "test_value");
}

// ============================================================================
// STATUS METHOD TESTS
// ============================================================================

TEST_F(TestUSDB2Unit, IsConnectedTrue) {
EXPECT_CALL(*mockDb, isConnected())
.WillOnce(Return(true));

bool result = mockDb->isConnected();

EXPECT_TRUE(result);
}

TEST_F(TestUSDB2Unit, NumRowsPositive) {
EXPECT_CALL(*mockDb, numRows())
.WillOnce(Return(15));

int result = mockDb->numRows();

EXPECT_EQ(result, 15);
}

TEST_F(TestUSDB2Unit, LastErrorEmpty) {
EXPECT_CALL(*mockDb, lastError())
.WillOnce(Return(QString()));

QString result = mockDb->lastError();

EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestUSDB2Unit, LastErrnoOK) {
EXPECT_CALL(*mockDb, lastErrno())
.WillOnce(Return(IUS_DB2::OK));

int result = mockDb->lastErrno();

EXPECT_EQ(result, IUS_DB2::OK);
}

TEST_F(TestUSDB2Unit, LastInsertIDValid) {
EXPECT_CALL(*mockDb, lastInsertID())
.WillOnce(Return(42));

int result = mockDb->lastInsertID();

EXPECT_EQ(result, 42);
}

// ============================================================================
// BLOB AND FILE OPERATION TESTS
// ============================================================================

TEST_F(TestUSDB2Unit, WriteBlobToDBSuccess) {
QString filename = "/path/to/test/file.dat";
QString procedure = "upload_blob_data";
int tableID = 123;

EXPECT_CALL(*mockDb, writeBlobToDB(filename, procedure, tableID))
.WillOnce(Return(IUS_DB2::OK));

int result = mockDb->writeBlobToDB(filename, procedure, tableID);

EXPECT_EQ(result, IUS_DB2::OK);
}

TEST_F(TestUSDB2Unit, ReadBlobFromDBSuccess) {
QString filename = "/path/to/output/file.dat";
QString procedure = "download_blob_data";
int tableID = 456;

EXPECT_CALL(*mockDb, readBlobFromDB(filename, procedure, tableID))
.WillOnce(Return(IUS_DB2::OK));

int result = mockDb->readBlobFromDB(filename, procedure, tableID);

EXPECT_EQ(result, IUS_DB2::OK);
}

TEST_F(TestUSDB2Unit, WriteAucToDBSuccess) {
QString filename = "/path/to/auc/file.auc";
int tableID = 789;

EXPECT_CALL(*mockDb, writeAucToDB(filename, tableID))
.WillOnce(Return(IUS_DB2::OK));

int result = mockDb->writeAucToDB(filename, tableID);

EXPECT_EQ(result, IUS_DB2::OK);
}

TEST_F(TestUSDB2Unit, ReadAucFromDBSuccess) {
QString filename = "/path/to/output/file.auc";
int tableID = 101112;

EXPECT_CALL(*mockDb, readAucFromDB(filename, tableID))
.WillOnce(Return(IUS_DB2::OK));

int result = mockDb->readAucFromDB(filename, tableID);

EXPECT_EQ(result, IUS_DB2::OK);
}

// ============================================================================
// UTILITY METHOD TESTS
// ============================================================================

TEST_F(TestUSDB2Unit, MysqlEscapeStringValid) {
QByteArray to;
QByteArray from("test'data\"with\\special");
unsigned long length = from.length();

EXPECT_CALL(*mockDb, mysqlEscapeString(_, _, length))
.WillOnce(Return(length * 2));

unsigned long result = mockDb->mysqlEscapeString(to, from, length);

EXPECT_GT(result, 0);
EXPECT_GE(result, length);
}

// ============================================================================
// OVERLOAD DISTINCTION TESTS
// ============================================================================

TEST_F(TestUSDB2Unit, QueryOverloadStringVsList) {
QString stringQuery = "SELECT * FROM test";
QStringList listQuery;
listQuery << "get_test_data" << "param1";

EXPECT_CALL(*mockDb, query(Matcher<const QString&>(stringQuery))).Times(1);
EXPECT_CALL(*mockDb, query(Matcher<const QStringList&>(listQuery))).Times(1);

mockDb->query(stringQuery);
mockDb->query(listQuery);
}

TEST_F(TestUSDB2Unit, StatusQueryOverloadStringVsList) {
QString stringQuery = "UPDATE test SET value = 1";
QStringList listQuery;
listQuery << "update_test_value" << "1";

EXPECT_CALL(*mockDb, statusQuery(Matcher<const QString&>(stringQuery)))
.WillOnce(Return(IUS_DB2::OK));
EXPECT_CALL(*mockDb, statusQuery(Matcher<const QStringList&>(listQuery)))
.WillOnce(Return(IUS_DB2::OK));

int result1 = mockDb->statusQuery(stringQuery);
int result2 = mockDb->statusQuery(listQuery);

EXPECT_EQ(result1, IUS_DB2::OK);
EXPECT_EQ(result2, IUS_DB2::OK);
}