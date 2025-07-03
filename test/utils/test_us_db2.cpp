#include "test_us_db2.h"
#include "us_db2.h"  // Include us_db2.h only in the .cpp file

// Use your custom Qt matchers
using namespace qt_matchers;

// TestUSDB2 method implementations
void TestUSDB2::SetUp() {
    QtTestBase::SetUp();
    // Per-test setup for DB2 tests
}

void TestUSDB2::TearDown() {
    // Per-test cleanup for DB2 tests
    QtTestBase::TearDown();
}

// Suite-level setup for DB2 tests
void TestUSDB2::SetUpTestSuite() {
    QtTestBase::SetUpTestSuite();
    // One-time setup for all TestUSDB2 tests
}

// Suite-level cleanup for DB2 tests
void TestUSDB2::TearDownTestSuite() {
    // One-time cleanup for all TestUSDB2 tests
}

TEST_F(TestUSDB2, Constructor) {
    // Test the default constructor
    US_DB2 db;
    EXPECT_FALSE(db.isConnected())
                        << "Default constructor should create unconnected database object";

    // Test the constructor with a password (if NO_DB is not defined)
#ifndef NO_DB
    QString password = "test_password";
    US_DB2 db_with_pw(password);
    EXPECT_FALSE(db_with_pw.isConnected())
                        << "Constructor with password should not auto-connect without valid credentials";
#endif
}

TEST_F(TestUSDB2, Connection) {
#ifndef NO_DB
    // Test the connect method with test credentials
    US_DB2 db;
    QString err;
    QString host = "localhost";
    QString dbname = "test_db";
    QString user = "test_user";
    QString password = "test_password";

    bool connected = db.connect(host, dbname, user, password, err);

    // Expected to fail without a real database
    EXPECT_FALSE(connected)
                        << "Connection should fail without valid database setup";
    EXPECT_FALSE(err.isEmpty())
                        << "Error message should be provided when connection fails";

    // Verify database object remains unconnected
    EXPECT_FALSE(db.isConnected())
                        << "Database object should remain unconnected after failed connection";
#else
    SUCCEED() << "Database tests skipped - NO_DB is defined";
#endif
}

TEST_F(TestUSDB2, SecureConnection) {
#ifndef NO_DB
    US_DB2 db;
    QString err;
    QString host = "localhost";
    QString dbname = "test_db";
    QString user = "test_user";
    QString password = "test_password";
    QString email = "test@example.com";
    QString masterPW = "master_password";

    bool connected = db.test_secure_connection(host, dbname, user, password, email, masterPW, err);

    // Expected to fail without a real database
    EXPECT_FALSE(connected)
                        << "Secure connection should fail without valid database setup";
    EXPECT_FALSE(err.isEmpty())
                        << "Error message should be provided when secure connection fails";

    // Verify database object remains unconnected
    EXPECT_FALSE(db.isConnected())
                        << "Database object should remain unconnected after failed secure connection";
#else
    SUCCEED() << "Secure database tests skipped - NO_DB is defined";
#endif
}

// Additional comprehensive tests
TEST_F(TestUSDB2, MultipleConnectionAttempts) {
#ifndef NO_DB
    US_DB2 db;
    QString err;
    QString host = "localhost";
    QString dbname = "test_db";
    QString user = "test_user";
    QString password = "test_password";

    // Try multiple connection attempts
    for (int i = 0; i < 3; i++) {
        bool connected = db.connect(host, dbname, user, password, err);
        EXPECT_FALSE(connected)
                            << "Connection attempt " << (i + 1) << " should fail";
        EXPECT_FALSE(db.isConnected())
                            << "Database should remain unconnected after attempt " << (i + 1);
    }
#endif
}

TEST_F(TestUSDB2, ErrorHandling) {
#ifndef NO_DB
    US_DB2 db;
    QString err;

    // Test with empty connection parameters
    bool connected = db.connect("", "", "", "", err);
    EXPECT_FALSE(connected)
                        << "Connection with empty parameters should fail";
    EXPECT_FALSE(err.isEmpty())
                        << "Error message should be provided for empty parameters";

    // Test with invalid host
    connected = db.connect("invalid_host_12345", "test_db", "test_user", "test_password", err);
    EXPECT_FALSE(connected)
                        << "Connection to invalid host should fail";
    EXPECT_FALSE(err.isEmpty())
                        << "Error message should be provided for invalid host";
#endif
}

TEST_F(TestUSDB2, ConstructorVariants) {
    // Test multiple constructor scenarios

    // Default constructor
    US_DB2 db1;
    EXPECT_FALSE(db1.isConnected())
                        << "Default constructor should create unconnected object";

#ifndef NO_DB
    // Constructor with password
    QString password = "test_password_123";
    US_DB2 db2(password);
    EXPECT_FALSE(db2.isConnected())
                        << "Password constructor should not auto-connect";

    // Constructor with empty password
    QString emptyPassword = "";
    US_DB2 db3(emptyPassword);
    EXPECT_FALSE(db3.isConnected())
                        << "Empty password constructor should create unconnected object";
#endif
}

TEST_F(TestUSDB2, ConnectionStateConsistency) {
#ifndef NO_DB
    US_DB2 db;

    // Initial state should be unconnected
    EXPECT_FALSE(db.isConnected())
                        << "Initial state should be unconnected";

    // After failed connection, should still be unconnected
    QString err;
    bool connected = db.connect("localhost", "test_db", "test_user", "test_password", err);
    EXPECT_FALSE(connected) << "Test connection should fail";
    EXPECT_FALSE(db.isConnected())
                        << "Should remain unconnected after failed connection";

    // Multiple checks should be consistent
    for (int i = 0; i < 5; i++) {
        EXPECT_FALSE(db.isConnected())
                            << "Connection state should be consistent on check " << (i + 1);
    }
#endif
}