#include <gmock/gmock.h>
#include "us_db2.h"

using namespace testing;

class MockUS_DB2 : public US_DB2 {
public:

    // Add default constructor that does nothing to avoid calling the real constructor
    MockUS_DB2() = default;

    // Optional: Add parameterized constructor if absolutely necessary, with minimal logic
    MockUS_DB2(const QString&) {
        // Avoid any real initialization or DB connection attempts here.
    }

// Mocked methods
    MOCK_METHOD(int, lastErrno, (), (override));
    MOCK_METHOD(void, query, (const QStringList&), (override));
    MOCK_METHOD(QVariant, value, (unsigned), (override));
    MOCK_METHOD(int, statusQuery, (const QStringList&), (override));
    MOCK_METHOD(bool, connect, (const QString&, QString&), (override));
    MOCK_METHOD(bool, connect, (const QString&, const QString&, const QString&, const QString&, QString&), (override));
    MOCK_METHOD(bool, next, (), (override));
    MOCK_METHOD(int, numRows, (), (override));
    MOCK_METHOD(QString, lastError, (), (override));
    MOCK_METHOD(int, lastInsertID, (), (override));

    private:
        void setupMockBehavior() {
            // Setup default behaviors for mocked methods
            ON_CALL(*this, lastErrno()).WillByDefault(testing::Return(US_DB2::OK));
            ON_CALL(*this, query(::testing::_)).WillByDefault(testing::Return());
            ON_CALL(*this, value(::testing::_)).WillByDefault(testing::Return(QVariant()));
            ON_CALL(*this, statusQuery(::testing::_)).WillByDefault(testing::Return(0));
            ON_CALL(*this, connect(::testing::_, ::testing::_)).WillByDefault(testing::Return(true));
            ON_CALL(*this, next()).WillByDefault(testing::Return(false));
            ON_CALL(*this, numRows()).WillByDefault(testing::Return(0));
            ON_CALL(*this, lastError()).WillByDefault(testing::Return(QString()));
            ON_CALL(*this, lastInsertID()).WillByDefault(testing::Return(-1));
        }
};
