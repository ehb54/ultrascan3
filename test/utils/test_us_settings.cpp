#include "qt_test_base.h"
#include "us_settings.h"
#include <QSettings>
#include <QStandardPaths>

class US_SettingsTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();

        // Store original settings values to restore later
        storeOriginalValues();
    }

    void TearDown() override {
        // Restore original settings values
        restoreOriginalValues();
        QtTestBase::TearDown();
    }

private:
    // Store original values to restore after tests
    QString originalBrowser;
    QString originalWorkBaseDir;
    QString originalImportDir;
    QString originalTmpDir;
    QStringList originalLicense;
    QByteArray originalPW;
    double originalTempTolerance;
    bool originalBeckmanBug;
    int originalDataLocation;
    int originalDebug;
    QStringList originalDebugText;
    QString originalInvName;
    int originalInvID;
    int originalInvLevel;
    int originalAdvancedLevel;
    int originalThreads;
    int originalNoiseDialog;
    QList<QStringList> originalDatabases;
    QStringList originalDefaultDB;

    void storeOriginalValues() {
        originalBrowser = US_Settings::browser();
        originalWorkBaseDir = US_Settings::workBaseDir();
        originalImportDir = US_Settings::importDir();
        originalTmpDir = US_Settings::tmpDir();
        originalLicense = US_Settings::license();
        originalPW = US_Settings::UltraScanPW();
        originalTempTolerance = US_Settings::tempTolerance();
        originalBeckmanBug = US_Settings::beckmanBug();
        originalDataLocation = US_Settings::default_data_location();
        originalDebug = US_Settings::us_debug();
        originalDebugText = US_Settings::debug_text();
        originalInvName = US_Settings::us_inv_name();
        originalInvID = US_Settings::us_inv_ID();
        originalInvLevel = US_Settings::us_inv_level();
        originalAdvancedLevel = US_Settings::advanced_level();
        originalThreads = US_Settings::threads();
        originalNoiseDialog = US_Settings::noise_dialog();
        originalDatabases = US_Settings::databases();
        originalDefaultDB = US_Settings::defaultDB();
    }

    void restoreOriginalValues() {
        US_Settings::set_browser(originalBrowser);
        US_Settings::set_workBaseDir(originalWorkBaseDir);
        US_Settings::set_importDir(originalImportDir);
        US_Settings::set_tmpDir(originalTmpDir);
        US_Settings::set_license(originalLicense);
        US_Settings::set_UltraScanPW(originalPW);
        US_Settings::set_tempTolerance(originalTempTolerance);
        US_Settings::set_beckmanBug(originalBeckmanBug);
        US_Settings::set_default_data_location(originalDataLocation);
        US_Settings::set_us_debug(originalDebug);
        US_Settings::set_debug_text(originalDebugText);
        US_Settings::set_us_inv_name(originalInvName);
        US_Settings::set_us_inv_ID(originalInvID);
        US_Settings::set_us_inv_level(originalInvLevel);
        US_Settings::set_advanced_level(originalAdvancedLevel);
        US_Settings::set_threads(originalThreads);
        US_Settings::set_noise_dialog(originalNoiseDialog);
        US_Settings::set_databases(originalDatabases);
        US_Settings::set_defaultDB(originalDefaultDB);
    }
};

// Basic Set/Get Tests - Most Important
TEST_F(US_SettingsTest, BrowserSetAndGet) {
QString testBrowser = "/usr/bin/test-browser";
US_Settings::set_browser(testBrowser);
EXPECT_EQ(US_Settings::browser(), testBrowser);
}

TEST_F(US_SettingsTest, WorkBaseDirSetAndGet) {
QString testDir = "/test/work/directory";
US_Settings::set_workBaseDir(testDir);
EXPECT_EQ(US_Settings::workBaseDir(), testDir);
}

TEST_F(US_SettingsTest, TempToleranceSetAndGet) {
double testTolerance = 1.25;
US_Settings::set_tempTolerance(testTolerance);
EXPECT_DOUBLE_EQ(US_Settings::tempTolerance(), testTolerance);
}

TEST_F(US_SettingsTest, ThreadsSetAndGet) {
US_Settings::set_threads(8);
EXPECT_EQ(US_Settings::threads(), 8);
}

TEST_F(US_SettingsTest, BeckmanBugSetAndGet) {
US_Settings::set_beckmanBug(true);
EXPECT_TRUE(US_Settings::beckmanBug());

US_Settings::set_beckmanBug(false);
EXPECT_FALSE(US_Settings::beckmanBug());
}

TEST_F(US_SettingsTest, LicenseSetAndGet) {
QStringList testLicense = {"Line 1", "Line 2", "Line 3"};
US_Settings::set_license(testLicense);
EXPECT_EQ(US_Settings::license(), testLicense);
}

TEST_F(US_SettingsTest, MasterPasswordSetAndGet) {
QByteArray testHash = "test_hash_12345";
US_Settings::set_UltraScanPW(testHash);
EXPECT_EQ(US_Settings::UltraScanPW(), testHash);
}

TEST_F(US_SettingsTest, DebugSetAndGet) {
US_Settings::set_us_debug(3);
EXPECT_EQ(US_Settings::us_debug(), 3);
}

TEST_F(US_SettingsTest, InvestigatorInfoSetAndGet) {
QString testName = "Test Investigator";
int testID = 999;
int testLevel = 5;

US_Settings::set_us_inv_name(testName);
US_Settings::set_us_inv_ID(testID);
US_Settings::set_us_inv_level(testLevel);

EXPECT_EQ(US_Settings::us_inv_name(), testName);
EXPECT_EQ(US_Settings::us_inv_ID(), testID);
EXPECT_EQ(US_Settings::us_inv_level(), testLevel);
}

// Test Directory Path Dependencies
TEST_F(US_SettingsTest, DirectoryPathsFollowWorkBase) {
QString customBase = "/custom/base";
US_Settings::set_workBaseDir(customBase);

EXPECT_EQ(US_Settings::dataDir(), customBase + "/data");
EXPECT_EQ(US_Settings::archiveDir(), customBase + "/archive");
EXPECT_EQ(US_Settings::resultDir(), customBase + "/results");
EXPECT_EQ(US_Settings::reportDir(), customBase + "/reports");
EXPECT_EQ(US_Settings::etcDir(), customBase + "/etc");
}

// Test Database Configuration
TEST_F(US_SettingsTest, DatabaseConfigSetAndGet) {
QList<QStringList> testDatabases;
testDatabases << (QStringList() << "host1" << "db1" << "user1" << "desc1");
testDatabases << (QStringList() << "host2" << "db2" << "user2" << "desc2");

US_Settings::set_databases(testDatabases);

QList<QStringList> retrieved = US_Settings::databases();
EXPECT_EQ(retrieved.size(), 2);
EXPECT_EQ(retrieved[0], testDatabases[0]);
EXPECT_EQ(retrieved[1], testDatabases[1]);
}

TEST_F(US_SettingsTest, DefaultDBSetAndGet) {
QStringList testDB = {"testhost", "testdb", "testuser", "Test DB"};
US_Settings::set_defaultDB(testDB);
EXPECT_EQ(US_Settings::defaultDB(), testDB);
}

// Test DA Status
TEST_F(US_SettingsTest, DAStatusOperations) {
US_Settings::set_DA_status("COM:1");
EXPECT_TRUE(US_Settings::get_DA_status("COM"));

US_Settings::set_DA_status("COM:0");
EXPECT_FALSE(US_Settings::get_DA_status("COM"));
}

// Test Settings Status
TEST_F(US_SettingsTest, SettingsStatusCheck) {
QString status = US_Settings::status();
// Should be empty if no errors
EXPECT_TRUE(status.isEmpty() || !status.contains("error", Qt::CaseInsensitive));
}

// Simple Integration Test
TEST_F(US_SettingsTest, BasicIntegrationTest) {
// Set a few different types of settings
US_Settings::set_browser("/test/browser");
US_Settings::set_threads(4);
US_Settings::set_tempTolerance(2.0);

// Verify they all persist
EXPECT_EQ(US_Settings::browser(), "/test/browser");
EXPECT_EQ(US_Settings::threads(), 4);
EXPECT_DOUBLE_EQ(US_Settings::tempTolerance(), 2.0);
}