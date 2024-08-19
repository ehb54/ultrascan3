#include <gmock/gmock.h>
#include "us_db2.h"

class MockUS_DB2 : public US_DB2 {
public:
    MOCK_METHOD(void, query, (const QStringList&), (override));
    MOCK_METHOD(bool, next, (), (override));
    MOCK_METHOD(QVariant, value, (unsigned), (override));
    MOCK_METHOD(int, lastErrno, (), (override));
};
