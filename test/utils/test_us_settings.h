#ifndef TEST_US_SETTINGS_H
#define TEST_US_SETTINGS_H

#include <gtest/gtest.h>
#include "qt_test_base.h"
// Forward declaration to avoid multiple inclusion issues
class US_Settings;

// Test fixture class for US_Settings - inherits from your QtTestBase
class TestUSSettings : public QtTestBase {
protected:
    void SetUp() override;
    void TearDown() override;

    // Suite-level setup and cleanup declarations
    static void SetUpTestSuite();
    static void TearDownTestSuite();

protected:
    // Helper methods for settings validation
    void validateDirectoryPath(const QString& path, const QString& description);
    bool isValidPath(const QString& path);
    void backupAndRestoreStringSetting(const QString& testValue,
                                       std::function<QString()> getter,
                                       std::function<void(const QString&)> setter);
};

#endif // TEST_US_SETTINGS_H