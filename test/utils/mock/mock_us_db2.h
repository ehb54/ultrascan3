#ifndef US_DB2_MOCK_H
#define US_DB2_MOCK_H

#include <gmock/gmock.h>
#include "ius_db2.h"  // Include the interface
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QByteArray>

/**
 * @brief Mock class for US_DB2 database operations
 *
 * Uses specific method names to avoid Google Mock overload issues
 * This version provides better handling of overloaded methods
 */
class US_DB2_Mock : public IUS_DB2 {
public:
    US_DB2_Mock() = default;
    ~US_DB2_Mock() override = default;

    // Connection methods (no overloads, so these are fine)
    MOCK_METHOD(bool, connect, (const QString& masterPW, QString& err), (override));
    MOCK_METHOD(bool, connect, (const QString& host, const QString& dbname,
            const QString& user, const QString& password,
            QString& error), (override));

    // Test connection methods
    MOCK_METHOD(bool, test_db_connection,
                (const QString& host, const QString& dbname,
                        const QString& user, const QString& password,
                        QString& error), (override));

    MOCK_METHOD(bool, test_secure_connection,
                (const QString& host, const QString& dbname,
                        const QString& user, const QString& password,
                        const QString& email, const QString& pw,
                        QString& err), (override));

    // Query methods - USE SPECIFIC NAMES TO AVOID OVERLOAD ISSUES
    MOCK_METHOD(void, queryString, (const QString& sqlQuery), ());
    MOCK_METHOD(void, queryStringList, (const QStringList& arguments), ());
    MOCK_METHOD(int, statusQueryString, (const QString& sqlQuery), ());
    MOCK_METHOD(int, statusQueryStringList, (const QStringList& arguments), ());

    // Delegate interface methods to specific mock methods
    void query(const QString& sqlQuery) override {
        queryString(sqlQuery);
    }

    void query(const QStringList& arguments) override {
        queryStringList(arguments);
    }

    int statusQuery(const QString& sqlQuery) override {
        return statusQueryString(sqlQuery);
    }

    int statusQuery(const QStringList& arguments) override {
        return statusQueryStringList(arguments);
    }

    // Other methods (no overloads, so mock directly)
    MOCK_METHOD(void, rawQuery, (const QString& sqlQuery), (override));
    MOCK_METHOD(int, functionQuery, (const QStringList& arguments), (override));

    // Result methods
    MOCK_METHOD(bool, next, (), (override));
    MOCK_METHOD(QVariant, value, (unsigned index), (override));
    MOCK_METHOD(bool, isConnected, (), (override));
    MOCK_METHOD(int, numRows, (), (override));

    // Error handling methods
    MOCK_METHOD(QString, lastError, (), (override));
    MOCK_METHOD(int, lastErrno, (), (override));
    MOCK_METHOD(int, lastInsertID, (), (override));
    MOCK_METHOD(QString, lastDebug, (), (override));

    // File/blob operations
    MOCK_METHOD(int, writeBlobToDB,
                (const QString& filename, const QString& procedure, const int tableID),
                (override));

    MOCK_METHOD(int, readBlobFromDB,
                (const QString& filename, const QString& procedure, const int tableID),
                (override));

    MOCK_METHOD(int, writeAucToDB,
                (const QString& filename, int tableID),
                (override));

    MOCK_METHOD(int, readAucFromDB,
                (const QString& filename, int tableID),
                (override));

    // Utility methods
    MOCK_METHOD(unsigned long, mysqlEscapeString,
                (QByteArray& to, QByteArray& from, unsigned long length),
                (override));
};

/**
 * @brief Helper class to create a mock database with default behaviors
 *
 * This can be useful for tests that need consistent default behavior
 */
class US_DB2_MockHelper {
public:
    static std::unique_ptr<US_DB2_Mock> createMockWithDefaults() {
        auto mock = std::make_unique<US_DB2_Mock>();

        // Set up default behaviors
        ON_CALL(*mock, isConnected())
                .WillByDefault(::testing::Return(true));

        ON_CALL(*mock, lastErrno())
                .WillByDefault(::testing::Return(IUS_DB2::OK));

        ON_CALL(*mock, lastError())
                .WillByDefault(::testing::Return(QString()));

        return mock;
    }
};

// Convenient typedefs for different mock types
using US_DB2_MockNice = ::testing::NiceMock<US_DB2_Mock>;
using US_DB2_MockStrict = ::testing::StrictMock<US_DB2_Mock>;

#endif // US_DB2_MOCK_H