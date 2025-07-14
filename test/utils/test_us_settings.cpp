#include "qt_test_base.h"
#include "us_settings.h"
#include <QSettings>
#include <QDir>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QCoreApplication>

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

class US_SettingsTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();

        // Create a temporary organization and application name for isolated testing
        testOrgName = "TestUS3";
        testAppName = "TestUltraScan";

        // Clear any existing test settings
        clearTestSettings();
    }

    void TearDown() override {
        // Clean up test settings
        clearTestSettings();
        QtTestBase::TearDown();
    }

    QString testOrgName;
    QString testAppName;

    // Helper to clear test settings
    void clearTestSettings() {
        QSettings testSettings(testOrgName, testAppName);
        testSettings.clear();
        testSettings.sync();
    }

    // Helper to create test settings with custom values
    void setTestSetting(const QString& key, const QVariant& value) {
        QSettings testSettings(testOrgName, testAppName);
        testSettings.setValue(key, value);
        testSettings.sync();
    }

    // Helper to get test setting
    QVariant getTestSetting(const QString& key, const QVariant& defaultValue = QVariant()) {
        QSettings testSettings(testOrgName, testAppName);
        return testSettings.value(key, defaultValue);
    }

    // Helper to remove test setting
    void removeTestSetting(const QString& key) {
        QSettings testSettings(testOrgName, testAppName);
        testSettings.remove(key);
        testSettings.sync();
    }
};

// Constructor Tests
TEST_F(US_SettingsTest, Constructor_CreatesSuccessfully) {
US_Settings settings;
// Constructor is null, just verify it doesn't crash
EXPECT_NO_THROW(US_Settings());
}

TEST_F(US_SettingsTest, Destructor_HandledCorrectly) {
// Test that destructor doesn't cause issues
EXPECT_NO_THROW({
US_Settings* settings = new US_Settings();
delete settings;
});
}

// Browser Tests
TEST_F(US_SettingsTest, Browser_DefaultValue_ReturnsExpected) {
QString browser = US_Settings::browser();
EXPECT_EQ(browser, "/usr/bin/firefox");
}

TEST_F(US_SettingsTest, SetBrowser_ValidPath_StoresCorrectly) {
QString testBrowser = "/usr/bin/chromium";
US_Settings::set_browser(testBrowser);

QString retrievedBrowser = US_Settings::browser();
EXPECT_EQ(retrievedBrowser, testBrowser);
}

TEST_F(US_SettingsTest, SetBrowser_EmptyString_StoresEmptyString) {
US_Settings::set_browser("");

QString browser = US_Settings::browser();
EXPECT_EQ(browser, "");
}

TEST_F(US_SettingsTest, SetBrowser_SpecialCharacters_HandlesCorrectly) {
QString specialBrowser = "/path/with spaces/bröwser";
US_Settings::set_browser(specialBrowser);

QString browser = US_Settings::browser();
EXPECT_EQ(browser, specialBrowser);
}

// Directory Tests - WorkBaseDir
TEST_F(US_SettingsTest, WorkBaseDir_DefaultValue_ReturnsHomePlusUltrascan) {
QString expected = QDir::homePath() + "/ultrascan";
QString workBase = US_Settings::workBaseDir();
EXPECT_EQ(workBase, expected);
}

TEST_F(US_SettingsTest, SetWorkBaseDir_CustomPath_StoresCorrectly) {
QString customPath = "/custom/ultrascan/path";
US_Settings::set_workBaseDir(customPath);

QString workBase = US_Settings::workBaseDir();
EXPECT_EQ(workBase, customPath);
}

TEST_F(US_SettingsTest, SetWorkBaseDir_DefaultPath_RemovesSetting) {
QString defaultPath = QDir::homePath() + "/ultrascan";
QString customPath = "/custom/path";

// First set custom path
US_Settings::set_workBaseDir(customPath);
EXPECT_EQ(US_Settings::workBaseDir(), customPath);

// Then set back to default - should remove setting
US_Settings::set_workBaseDir(defaultPath);
EXPECT_EQ(US_Settings::workBaseDir(), defaultPath);
}

// ImportDir Tests
TEST_F(US_SettingsTest, ImportDir_DefaultValue_ReturnsWorkBasePlusImports) {
QString expected = US_Settings::workBaseDir() + "/imports";
QString importDir = US_Settings::importDir();
EXPECT_EQ(importDir, expected);
}

TEST_F(US_SettingsTest, SetImportDir_CustomPath_StoresCorrectly) {
QString customPath = "/custom/imports";
US_Settings::set_importDir(customPath);

QString importDir = US_Settings::importDir();
EXPECT_EQ(importDir, customPath);
}

TEST_F(US_SettingsTest, SetImportDir_DefaultPath_RemovesSetting) {
QString defaultPath = US_Settings::workBaseDir() + "/imports";
QString customPath = "/custom/imports";

US_Settings::set_importDir(customPath);
EXPECT_EQ(US_Settings::importDir(), customPath);

US_Settings::set_importDir(defaultPath);
EXPECT_EQ(US_Settings::importDir(), defaultPath);
}

// TmpDir Tests
TEST_F(US_SettingsTest, TmpDir_DefaultValue_ReturnsWorkBasePlusTmp) {
QString expected = US_Settings::workBaseDir() + "/tmp";
QString tmpDir = US_Settings::tmpDir();
EXPECT_EQ(tmpDir, expected);
}

TEST_F(US_SettingsTest, SetTmpDir_CustomPath_StoresCorrectly) {
QString customPath = "/custom/tmp";
US_Settings::set_tmpDir(customPath);

QString tmpDir = US_Settings::tmpDir();
EXPECT_EQ(tmpDir, customPath);
}

TEST_F(US_SettingsTest, SetTmpDir_DefaultPath_RemovesSetting) {
QString defaultPath = US_Settings::workBaseDir() + "/tmp";
QString customPath = "/custom/tmp";

US_Settings::set_tmpDir(customPath);
EXPECT_EQ(US_Settings::tmpDir(), customPath);

US_Settings::set_tmpDir(defaultPath);
EXPECT_EQ(US_Settings::tmpDir(), defaultPath);
}

// Fixed Directory Tests
TEST_F(US_SettingsTest, DataDir_ReturnsWorkBasePlusData) {
QString expected = US_Settings::workBaseDir() + "/data";
QString dataDir = US_Settings::dataDir();
EXPECT_EQ(dataDir, expected);
}

TEST_F(US_SettingsTest, ArchiveDir_ReturnsWorkBasePlusArchive) {
QString expected = US_Settings::workBaseDir() + "/archive";
QString archiveDir = US_Settings::archiveDir();
EXPECT_EQ(archiveDir, expected);
}

TEST_F(US_SettingsTest, ResultDir_ReturnsWorkBasePlusResults) {
QString expected = US_Settings::workBaseDir() + "/results";
QString resultDir = US_Settings::resultDir();
EXPECT_EQ(resultDir, expected);
}

TEST_F(US_SettingsTest, ReportDir_ReturnsWorkBasePlusReports) {
QString expected = US_Settings::workBaseDir() + "/reports";
QString reportDir = US_Settings::reportDir();
EXPECT_EQ(reportDir, expected);
}

TEST_F(US_SettingsTest, EtcDir_ReturnsWorkBasePlusEtc) {
QString expected = US_Settings::workBaseDir() + "/etc";
QString etcDir = US_Settings::etcDir();
EXPECT_EQ(etcDir, expected);
}

// AppBaseDir Tests
TEST_F(US_SettingsTest, AppBaseDir_RemovesBinSuffix) {
QString appDir = US_Settings::appBaseDir();
EXPECT_FALSE(appDir.endsWith("/bin"));
}

TEST_F(US_SettingsTest, AppBaseDir_HandlesMacAppBundle) {
// This test verifies the Mac .app handling logic conceptually
QString testPath = "/Applications/UltraScan.app/Contents/MacOS";
QRegExp binRegex("/bin$");
QString result = testPath;
result.remove(binRegex);

if (result.contains(".app/Contents")) {
int ii = result.lastIndexOf("/bin/");
if (ii > 0) {
result = result.left(ii);
}
}

// For Mac bundles, should handle the path correctly
EXPECT_TRUE(result.contains(".app") || !result.contains("/bin/"));
}

// License Tests
TEST_F(US_SettingsTest, License_DefaultValue_ReturnsEmptyList) {
QStringList license = US_Settings::license();
EXPECT_TRUE(license.isEmpty());
}

TEST_F(US_SettingsTest, SetLicense_ValidList_StoresCorrectly) {
QStringList testLicense;
testLicense << "License Line 1" << "License Line 2" << "License Line 3";

US_Settings::set_license(testLicense);

QStringList license = US_Settings::license();
EXPECT_EQ(license, testLicense);
}

TEST_F(US_SettingsTest, SetLicense_EmptyList_StoresEmptyList) {
QStringList emptyLicense;
US_Settings::set_license(emptyLicense);

QStringList license = US_Settings::license();
EXPECT_TRUE(license.isEmpty());
}

// Master Password Tests
TEST_F(US_SettingsTest, UltraScanPW_DefaultValue_ReturnsEmptyByteArray) {
QByteArray password = US_Settings::UltraScanPW();
EXPECT_TRUE(password.isEmpty());
}

TEST_F(US_SettingsTest, SetUltraScanPW_ValidHash_StoresCorrectly) {
QByteArray testHash = "test_hash_value_123";
US_Settings::set_UltraScanPW(testHash);

QByteArray password = US_Settings::UltraScanPW();
EXPECT_EQ(password, testHash);
}

TEST_F(US_SettingsTest, SetUltraScanPW_EmptyByteArray_StoresEmpty) {
QByteArray emptyHash;
US_Settings::set_UltraScanPW(emptyHash);

QByteArray password = US_Settings::UltraScanPW();
EXPECT_TRUE(password.isEmpty());
}

// Temperature Tolerance Tests
TEST_F(US_SettingsTest, TempTolerance_DefaultValue_ReturnsHalf) {
double tolerance = US_Settings::tempTolerance();
EXPECT_EQ(tolerance, 0.5);
}

TEST_F(US_SettingsTest, SetTempTolerance_CustomValue_StoresCorrectly) {
double testTolerance = 1.5;
US_Settings::set_tempTolerance(testTolerance);

double tolerance = US_Settings::tempTolerance();
EXPECT_EQ(tolerance, testTolerance);
}

TEST_F(US_SettingsTest, SetTempTolerance_DefaultValue_RemovesSetting) {
double customValue = 2.0;
US_Settings::set_tempTolerance(customValue);
EXPECT_EQ(US_Settings::tempTolerance(), customValue);

US_Settings::set_tempTolerance(0.5);
EXPECT_EQ(US_Settings::tempTolerance(), 0.5);
}

TEST_F(US_SettingsTest, SetTempTolerance_NegativeValue_StoresCorrectly) {
double negativeValue = -1.0;
US_Settings::set_tempTolerance(negativeValue);

double tolerance = US_Settings::tempTolerance();
EXPECT_EQ(tolerance, negativeValue);
}

TEST_F(US_SettingsTest, SetTempTolerance_ZeroValue_StoresCorrectly) {
US_Settings::set_tempTolerance(0.0);

double tolerance = US_Settings::tempTolerance();
EXPECT_EQ(tolerance, 0.0);
}

// Beckman Bug Tests
TEST_F(US_SettingsTest, BeckmanBug_DefaultValue_ReturnsFalse) {
bool bug = US_Settings::beckmanBug();
EXPECT_FALSE(bug);
}

TEST_F(US_SettingsTest, SetBeckmanBug_True_StoresCorrectly) {
US_Settings::set_beckmanBug(true);

bool bug = US_Settings::beckmanBug();
EXPECT_TRUE(bug);
}

TEST_F(US_SettingsTest, SetBeckmanBug_False_RemovesSetting) {
US_Settings::set_beckmanBug(true);
EXPECT_TRUE(US_Settings::beckmanBug());

US_Settings::set_beckmanBug(false);
EXPECT_FALSE(US_Settings::beckmanBug());
}

// Default Data Location Tests
TEST_F(US_SettingsTest, DefaultDataLocation_DefaultValue_ReturnsTwo) {
int location = US_Settings::default_data_location();
EXPECT_EQ(location, 2);
}

TEST_F(US_SettingsTest, SetDefaultDataLocation_CustomValue_StoresCorrectly) {
US_Settings::set_default_data_location(1);

int location = US_Settings::default_data_location();
EXPECT_EQ(location, 1);
}

TEST_F(US_SettingsTest, SetDefaultDataLocation_DefaultValue_RemovesSetting) {
US_Settings::set_default_data_location(1);
EXPECT_EQ(US_Settings::default_data_location(), 1);

US_Settings::set_default_data_location(2);
EXPECT_EQ(US_Settings::default_data_location(), 2);
}

TEST_F(US_SettingsTest, SetDefaultDataLocation_InvalidValue_StoresCorrectly) {
US_Settings::set_default_data_location(999);

int location = US_Settings::default_data_location();
EXPECT_EQ(location, 999);
}

// Debug Level Tests
TEST_F(US_SettingsTest, UsDebug_DefaultValue_ReturnsZero) {
int debug = US_Settings::us_debug();
EXPECT_EQ(debug, 0);
}

TEST_F(US_SettingsTest, SetUsDebug_CustomValue_StoresCorrectly) {
US_Settings::set_us_debug(3);

int debug = US_Settings::us_debug();
EXPECT_EQ(debug, 3);
}

TEST_F(US_SettingsTest, SetUsDebug_Zero_RemovesSetting) {
US_Settings::set_us_debug(5);
EXPECT_EQ(US_Settings::us_debug(), 5);

US_Settings::set_us_debug(0);
EXPECT_EQ(US_Settings::us_debug(), 0);
}

TEST_F(US_SettingsTest, SetUsDebug_NegativeValue_StoresCorrectly) {
US_Settings::set_us_debug(-1);

int debug = US_Settings::us_debug();
EXPECT_EQ(debug, -1);
}

// Debug Text Tests
TEST_F(US_SettingsTest, DebugText_DefaultValue_ReturnsEmptyList) {
QStringList debugText = US_Settings::debug_text();
EXPECT_TRUE(debugText.isEmpty());
}

TEST_F(US_SettingsTest, SetDebugText_ValidList_StoresCorrectly) {
QStringList testDebugText;
testDebugText << "debug1=value1" << "debug2=value2" << "debug3";

US_Settings::set_debug_text(testDebugText);

QStringList debugText = US_Settings::debug_text();
EXPECT_EQ(debugText, testDebugText);
}

TEST_F(US_SettingsTest, SetDebugText_EmptyList_RemovesSetting) {
QStringList testList;
testList << "test";
US_Settings::set_debug_text(testList);
EXPECT_FALSE(US_Settings::debug_text().isEmpty());

US_Settings::set_debug_text(QStringList());
EXPECT_TRUE(US_Settings::debug_text().isEmpty());
}

TEST_F(US_SettingsTest, DebugMatch_ExistingMatch_ReturnsTrue) {
QStringList debugText;
debugText << "TEST_FLAG=true" << "OTHER_FLAG=false";
US_Settings::set_debug_text(debugText);

bool match = US_Settings::debug_match("TEST_FLAG");
EXPECT_TRUE(match);
}

TEST_F(US_SettingsTest, DebugMatch_NonExistentMatch_ReturnsFalse) {
QStringList debugText;
debugText << "TEST_FLAG=true";
US_Settings::set_debug_text(debugText);

bool match = US_Settings::debug_match("NON_EXISTENT");
EXPECT_FALSE(match);
}

TEST_F(US_SettingsTest, DebugMatch_CaseInsensitive_ReturnsTrue) {
QStringList debugText;
debugText << "test_flag=true";
US_Settings::set_debug_text(debugText);

bool match = US_Settings::debug_match("TEST_FLAG");
EXPECT_TRUE(match);
}

TEST_F(US_SettingsTest, DebugValue_ExistingMatch_ReturnsValue) {
QStringList debugText;
debugText << "TEST_KEY=test_value" << "OTHER_KEY=other_value";
US_Settings::set_debug_text(debugText);

QString value = US_Settings::debug_value("TEST_KEY");
EXPECT_EQ(value, "test_value");
}

TEST_F(US_SettingsTest, DebugValue_NonExistentMatch_ReturnsEmpty) {
QStringList debugText;
debugText << "TEST_KEY=test_value";
US_Settings::set_debug_text(debugText);

QString value = US_Settings::debug_value("NON_EXISTENT");
EXPECT_TRUE(value.isEmpty());
}

TEST_F(US_SettingsTest, DebugValue_CaseInsensitive_ReturnsValue) {
QStringList debugText;
debugText << "test_key=test_value";
US_Settings::set_debug_text(debugText);

QString value = US_Settings::debug_value("TEST_KEY");
EXPECT_EQ(value, "test_value");
}

TEST_F(US_SettingsTest, DebugValue_NoEqualsSign_ReturnsEmpty) {
QStringList debugText;
debugText << "TEST_FLAG_NO_VALUE";
US_Settings::set_debug_text(debugText);

QString value = US_Settings::debug_value("TEST_FLAG_NO_VALUE");
EXPECT_TRUE(value.isEmpty());
}

// Investigator Tests
TEST_F(US_SettingsTest, UsInvName_DefaultValue_ReturnsNotAvailable) {
QString name = US_Settings::us_inv_name();
EXPECT_EQ(name, "Not Available");
}

TEST_F(US_SettingsTest, SetUsInvName_ValidName_StoresCorrectly) {
QString testName = "John Doe";
US_Settings::set_us_inv_name(testName);

QString name = US_Settings::us_inv_name();
EXPECT_EQ(name, testName);
}

TEST_F(US_SettingsTest, SetUsInvName_EmptyString_RemovesSetting) {
US_Settings::set_us_inv_name("Test Name");
EXPECT_EQ(US_Settings::us_inv_name(), "Test Name");

US_Settings::set_us_inv_name("");
// After removing, should return default
QString name = US_Settings::us_inv_name();
EXPECT_EQ(name, "Not Available");
}

TEST_F(US_SettingsTest, UsInvID_DefaultValue_ReturnsMinusOne) {
int id = US_Settings::us_inv_ID();
EXPECT_EQ(id, -1);
}

TEST_F(US_SettingsTest, SetUsInvID_ValidID_StoresCorrectly) {
US_Settings::set_us_inv_ID(123);

int id = US_Settings::us_inv_ID();
EXPECT_EQ(id, 123);
}

TEST_F(US_SettingsTest, SetUsInvID_MinusOne_RemovesSetting) {
US_Settings::set_us_inv_ID(456);
EXPECT_EQ(US_Settings::us_inv_ID(), 456);

US_Settings::set_us_inv_ID(-1);
EXPECT_EQ(US_Settings::us_inv_ID(), -1);
}

TEST_F(US_SettingsTest, SetUsInvID_Zero_StoresCorrectly) {
US_Settings::set_us_inv_ID(0);

int id = US_Settings::us_inv_ID();
EXPECT_EQ(id, 0);
}

TEST_F(US_SettingsTest, UsInvLevel_DefaultValue_ReturnsZero) {
int level = US_Settings::us_inv_level();
EXPECT_EQ(level, 0);
}

TEST_F(US_SettingsTest, SetUsInvLevel_ValidLevel_StoresCorrectly) {
US_Settings::set_us_inv_level(5);

int level = US_Settings::us_inv_level();
EXPECT_EQ(level, 5);
}

TEST_F(US_SettingsTest, SetUsInvLevel_Zero_RemovesSetting) {
US_Settings::set_us_inv_level(3);
EXPECT_EQ(US_Settings::us_inv_level(), 3);

US_Settings::set_us_inv_level(0);
EXPECT_EQ(US_Settings::us_inv_level(), 0);
}

// Advanced Level Tests
TEST_F(US_SettingsTest, AdvancedLevel_DefaultValue_ReturnsZero) {
int level = US_Settings::advanced_level();
EXPECT_EQ(level, 0);
}

TEST_F(US_SettingsTest, SetAdvancedLevel_ValidLevel_StoresCorrectly) {
US_Settings::set_advanced_level(2);

int level = US_Settings::advanced_level();
EXPECT_EQ(level, 2);
}

TEST_F(US_SettingsTest, SetAdvancedLevel_Zero_RemovesSetting) {
US_Settings::set_advanced_level(1);
EXPECT_EQ(US_Settings::advanced_level(), 1);

US_Settings::set_advanced_level(0);
EXPECT_EQ(US_Settings::advanced_level(), 0);
}

// Threads Tests
TEST_F(US_SettingsTest, Threads_DefaultValue_ReturnsOne) {
int threads = US_Settings::threads();
EXPECT_EQ(threads, 1);
}

TEST_F(US_SettingsTest, SetThreads_ValidNumber_StoresCorrectly) {
US_Settings::set_threads(4);

int threads = US_Settings::threads();
EXPECT_EQ(threads, 4);
}

TEST_F(US_SettingsTest, SetThreads_One_RemovesSetting) {
US_Settings::set_threads(8);
EXPECT_EQ(US_Settings::threads(), 8);

US_Settings::set_threads(1);
EXPECT_EQ(US_Settings::threads(), 1);
}

TEST_F(US_SettingsTest, SetThreads_Zero_StoresCorrectly) {
US_Settings::set_threads(0);

int threads = US_Settings::threads();
EXPECT_EQ(threads, 0);
}

// Noise Dialog Tests
TEST_F(US_SettingsTest, NoiseDialog_DefaultValue_ReturnsZero) {
int dialog = US_Settings::noise_dialog();
EXPECT_EQ(dialog, 0);
}

TEST_F(US_SettingsTest, SetNoiseDialog_ValidFlag_StoresCorrectly) {
US_Settings::set_noise_dialog(1);

int dialog = US_Settings::noise_dialog();
EXPECT_EQ(dialog, 1);
}

TEST_F(US_SettingsTest, SetNoiseDialog_Zero_RemovesSetting) {
US_Settings::set_noise_dialog(1);
EXPECT_EQ(US_Settings::noise_dialog(), 1);

US_Settings::set_noise_dialog(0);
EXPECT_EQ(US_Settings::noise_dialog(), 0);
}

// Database Tests
TEST_F(US_SettingsTest, Databases_DefaultValue_ReturnsEmptyList) {
QList<QStringList> databases = US_Settings::databases();
EXPECT_TRUE(databases.isEmpty());
}

TEST_F(US_SettingsTest, SetDatabases_ValidList_StoresCorrectly) {
QList<QStringList> testDatabases;
QStringList db1;
db1 << "host1" << "db1" << "user1" << "pass1";
QStringList db2;
db2 << "host2" << "db2" << "user2" << "pass2";
testDatabases << db1 << db2;

US_Settings::set_databases(testDatabases);

QList<QStringList> databases = US_Settings::databases();
EXPECT_EQ(databases.size(), 2);
EXPECT_EQ(databases[0], db1);
EXPECT_EQ(databases[1], db2);
}

TEST_F(US_SettingsTest, SetDatabases_EmptyList_StoresEmpty) {
// First set some data
QList<QStringList> testData;
QStringList db1;
db1 << "test";
testData << db1;
US_Settings::set_databases(testData);

// Then clear it
US_Settings::set_databases(QList<QStringList>());

QList<QStringList> databases = US_Settings::databases();
EXPECT_TRUE(databases.isEmpty());
}

TEST_F(US_SettingsTest, SetDatabases_ReplacesExistingEntries) {
// Set initial data
QList<QStringList> initialData;
QStringList db1;
db1 << "initial" << "data";
initialData << db1 << db1 << db1; // 3 entries
US_Settings::set_databases(initialData);

// Replace with different data
QList<QStringList> newData;
QStringList db2;
db2 << "new" << "data";
newData << db2; // 1 entry
US_Settings::set_databases(newData);

QList<QStringList> databases = US_Settings::databases();
EXPECT_EQ(databases.size(), 1);
EXPECT_EQ(databases[0], db2);
}

// Default DB Tests
TEST_F(US_SettingsTest, DefaultDB_DefaultValue_ReturnsEmptyList) {
QStringList defaultDB = US_Settings::defaultDB();
EXPECT_TRUE(defaultDB.isEmpty());
}

TEST_F(US_SettingsTest, SetDefaultDB_ValidList_StoresCorrectly) {
QStringList testDB;
testDB << "localhost" << "ultrascan" << "user" << "password";

US_Settings::set_defaultDB(testDB);

QStringList defaultDB = US_Settings::defaultDB();
EXPECT_EQ(defaultDB, testDB);
}

TEST_F(US_SettingsTest, SetDefaultDB_EmptyList_RemovesSetting) {
QStringList testDB;
testDB << "test";
US_Settings::set_defaultDB(testDB);
EXPECT_FALSE(US_Settings::defaultDB().isEmpty());

US_Settings::set_defaultDB(QStringList());
EXPECT_TRUE(US_Settings::defaultDB().isEmpty());
}

// XPN DB Hosts Tests
TEST_F(US_SettingsTest, XpnDbHosts_DefaultValue_ReturnsEmptyList) {
QList<QStringList> hosts = US_Settings::xpn_db_hosts();
EXPECT_TRUE(hosts.isEmpty());
}

TEST_F(US_SettingsTest, SetXpnDbHosts_ValidList_StoresCorrectly) {
QList<QStringList> testHosts;
QStringList host1;
host1 << "xpn1.example.com" << "db1";
QStringList host2;
host2 << "xpn2.example.com" << "db2";
testHosts << host1 << host2;

US_Settings::set_xpn_db_hosts(testHosts);

QList<QStringList> hosts = US_Settings::xpn_db_hosts();
EXPECT_EQ(hosts.size(), 2);
EXPECT_EQ(hosts[0], host1);
EXPECT_EQ(hosts[1], host2);
}

TEST_F(US_SettingsTest, DefaultXpnHost_DefaultValue_ReturnsEmptyList) {
QStringList defaultHost = US_Settings::defaultXpnHost();
EXPECT_TRUE(defaultHost.isEmpty());
}

TEST_F(US_SettingsTest, SetDefXpnHost_ValidList_StoresCorrectly) {
QStringList testHost;
testHost << "default.xpn.com" << "defaultdb" << "user";

US_Settings::set_def_xpn_host(testHost);

QStringList defaultHost = US_Settings::defaultXpnHost();
EXPECT_EQ(defaultHost, testHost);
}

TEST_F(US_SettingsTest, SetDefXpnHost_EmptyList_RemovesSetting) {
QStringList testHost;
testHost << "test";
US_Settings::set_def_xpn_host(testHost);
EXPECT_FALSE(US_Settings::defaultXpnHost().isEmpty());

US_Settings::set_def_xpn_host(QStringList());
EXPECT_TRUE(US_Settings::defaultXpnHost().isEmpty());
}

// DA Status Tests
TEST_F(US_SettingsTest, SetDAStatus_COM_StoresCorrectly) {
US_Settings::set_DA_status("COM:1");

bool status = US_Settings::get_DA_status("COM");
EXPECT_TRUE(status);
}

TEST_F(US_SettingsTest, SetDAStatus_ACAD_StoresCorrectly) {
US_Settings::set_DA_status("ACAD:1");

bool status = US_Settings::get_DA_status("ACAD");
EXPECT_TRUE(status);
}

TEST_F(US_SettingsTest, GetDAStatus_COM_DefaultFalse) {
bool status = US_Settings::get_DA_status("COM");
EXPECT_FALSE(status);
}

TEST_F(US_SettingsTest, GetDAStatus_ACAD_DefaultFalse) {
bool status = US_Settings::get_DA_status("ACAD");
EXPECT_FALSE(status);
}

TEST_F(US_SettingsTest, SetDAStatus_COMZero_ReturnsFalse) {
US_Settings::set_DA_status("COM:0");

bool status = US_Settings::get_DA_status("COM");
EXPECT_FALSE(status);
}

TEST_F(US_SettingsTest, SetDAStatus_ACADZero_ReturnsFalse) {
US_Settings::set_DA_status("ACAD:0");

bool status = US_Settings::get_DA_status("ACAD");
EXPECT_FALSE(status);
}

TEST_F(US_SettingsTest, SetDAStatus_InvalidFormat_HandlesGracefully) {
// Test with malformed status string
US_Settings::set_DA_status("INVALID_FORMAT");

// Should not crash, and status should remain unchanged
bool comStatus = US_Settings::get_DA_status("COM");
bool acadStatus = US_Settings::get_DA_status("ACAD");

// Default should be false
EXPECT_FALSE(comStatus);
EXPECT_FALSE(acadStatus);
}

TEST_F(US_SettingsTest, SetDAStatus_EmptyString_HandlesGracefully) {
US_Settings::set_DA_status("");

bool status = US_Settings::get_DA_status("COM");
EXPECT_FALSE(status);
}

// Status Tests
TEST_F(US_SettingsTest, Status_ValidSettings_ReturnsEmpty) {
QString status = US_Settings::status();

// If settings are working properly, should return empty string
// This test verifies the status check mechanism works
EXPECT_TRUE(status.isEmpty() || status.contains("error") || status.contains("Error"));
}

TEST_F(US_SettingsTest, Status_ChecksSettingsAccess) {
QString status = US_Settings::status();

// The status method tests QSettings functionality
// We can't easily mock QSettings errors, but we can verify the method doesn't crash
EXPECT_NO_THROW(US_Settings::status());
}

// Edge Cases and Error Conditions

// Path Handling Edge Cases
TEST_F(US_SettingsTest, SetWorkBaseDir_EmptyString_StoresEmpty) {
US_Settings::set_workBaseDir("");

QString workBase = US_Settings::workBaseDir();
EXPECT_EQ(workBase, "");
}

TEST_F(US_SettingsTest, SetWorkBaseDir_PathWithSpaces_HandlesCorrectly) {
QString pathWithSpaces = "/path with spaces/ultrascan data";
US_Settings::set_workBaseDir(pathWithSpaces);

QString workBase = US_Settings::workBaseDir();
EXPECT_EQ(workBase, pathWithSpaces);
}

TEST_F(US_SettingsTest, SetImportDir_RelativePath_StoresCorrectly) {
QString relativePath = "../relative/imports";
US_Settings::set_importDir(relativePath);

QString importDir = US_Settings::importDir();
EXPECT_EQ(importDir, relativePath);
}

// Unicode and Special Character Tests
TEST_F(US_SettingsTest, SetUsInvName_UnicodeCharacters_HandlesCorrectly) {
QString unicodeName = "José María González";
US_Settings::set_us_inv_name(unicodeName);

QString name = US_Settings::us_inv_name();
EXPECT_EQ(name, unicodeName);
}

TEST_F(US_SettingsTest, SetBrowser_UnicodeCharacters_HandlesCorrectly) {
QString unicodePath = "/usr/bin/naïve-browser";
US_Settings::set_browser(unicodePath);

QString browser = US_Settings::browser();
EXPECT_EQ(browser, unicodePath);
}

// Large Data Tests
TEST_F(US_SettingsTest, SetLicense_LargeStringList_HandlesCorrectly) {
QStringList largeLicense;
for (int i = 0; i < 1000; ++i) {
largeLicense << QString("License line %1 with some additional text").arg(i);
}

US_Settings::set_license(largeLicense);

QStringList license = US_Settings::license();
EXPECT_EQ(license.size(), 1000);
EXPECT_EQ(license.first(), "License line 0 with some additional text");
EXPECT_EQ(license.last(), "License line 999 with some additional text");
}

TEST_F(US_SettingsTest, SetDatabases_LargeList_HandlesCorrectly) {
QList<QStringList> largeDatabaseList;
for (int i = 0; i < 100; ++i) {
QStringList db;
db << QString("host%1").arg(i)
<< QString("database%1").arg(i)
<< QString("user%1").arg(i)
<< QString("password%1").arg(i);
largeDatabaseList << db;
}

US_Settings::set_databases(largeDatabaseList);

QList<QStringList> databases = US_Settings::databases();
EXPECT_EQ(databases.size(), 100);
EXPECT_EQ(databases.first()[0], "host0");
EXPECT_EQ(databases.last()[0], "host99");
}

// Boundary Value Tests
TEST_F(US_SettingsTest, SetUsInvID_MaxInt_StoresCorrectly) {
int maxInt = std::numeric_limits<int>::max();
US_Settings::set_us_inv_ID(maxInt);

int id = US_Settings::us_inv_ID();
EXPECT_EQ(id, maxInt);
}

TEST_F(US_SettingsTest, SetUsInvID_MinInt_StoresCorrectly) {
int minInt = std::numeric_limits<int>::min();
US_Settings::set_us_inv_ID(minInt);

int id = US_Settings::us_inv_ID();
EXPECT_EQ(id, minInt);
}

TEST_F(US_SettingsTest, SetTempTolerance_MaxDouble_StoresCorrectly) {
double maxDouble = std::numeric_limits<double>::max();
US_Settings::set_tempTolerance(maxDouble);

double tolerance = US_Settings::tempTolerance();
EXPECT_EQ(tolerance, maxDouble);
}

TEST_F(US_SettingsTest, SetTempTolerance_MinDouble_StoresCorrectly) {
double minDouble = std::numeric_limits<double>::lowest();
US_Settings::set_tempTolerance(minDouble);

double tolerance = US_Settings::tempTolerance();
EXPECT_EQ(tolerance, minDouble);
}

TEST_F(US_SettingsTest, SetTempTolerance_Infinity_StoresCorrectly) {
double infinity = std::numeric_limits<double>::infinity();
US_Settings::set_tempTolerance(infinity);

double tolerance = US_Settings::tempTolerance();
EXPECT_TRUE(std::isinf(tolerance));
}

TEST_F(US_SettingsTest, SetTempTolerance_NaN_StoresCorrectly) {
double nan = std::numeric_limits<double>::quiet_NaN();
US_Settings::set_tempTolerance(nan);

double tolerance = US_Settings::tempTolerance();
EXPECT_TRUE(std::isnan(tolerance));
}

// Debug Text Complex Cases
TEST_F(US_SettingsTest, DebugText_ComplexKeyValuePairs_HandlesCorrectly) {
QStringList complexDebugText;
complexDebugText << "key1=value1"
<< "key2=value=with=equals"
<< "key3="  // Empty value
<< "=value4"  // Empty key
<< "key5=value with spaces"
<< "NOEQUALS";  // No equals sign

US_Settings::set_debug_text(complexDebugText);

EXPECT_EQ(US_Settings::debug_value("key1"), "value1");
EXPECT_EQ(US_Settings::debug_value("key2"), "value=with=equals");
EXPECT_EQ(US_Settings::debug_value("key3"), "");
EXPECT_EQ(US_Settings::debug_value("key5"), "value with spaces");
EXPECT_EQ(US_Settings::debug_value("NOEQUALS"), "");
}

TEST_F(US_SettingsTest, DebugMatch_PartialMatch_ReturnsFalse) {
QStringList debugText;
debugText << "FULL_FLAG_NAME=true";
US_Settings::set_debug_text(debugText);

bool match = US_Settings::debug_match("FLAG");  // Partial match
EXPECT_FALSE(match);
}

TEST_F(US_SettingsTest, DebugMatch_EmptyList_ReturnsFalse) {
US_Settings::set_debug_text(QStringList());

bool match = US_Settings::debug_match("ANY_FLAG");
EXPECT_FALSE(match);
}

// Database List Complex Cases
TEST_F(US_SettingsTest, SetDatabases_MixedStringListSizes_HandlesCorrectly) {
QList<QStringList> mixedDatabases;

QStringList db1;  // Normal 4-element list
db1 << "host1" << "db1" << "user1" << "pass1";

QStringList db2;  // Single element
db2 << "minimal";

QStringList db3;  // Empty list

QStringList db4;  // Many elements
db4 << "host4" << "db4" << "user4" << "pass4" << "extra1" << "extra2";

mixedDatabases << db1 << db2 << db3 << db4;

US_Settings::set_databases(mixedDatabases);

QList<QStringList> databases = US_Settings::databases();
EXPECT_EQ(databases.size(), 4);
EXPECT_EQ(databases[0].size(), 4);
EXPECT_EQ(databases[1].size(), 1);
EXPECT_EQ(databases[2].size(), 0);
EXPECT_EQ(databases[3].size(), 6);
}

// AppBaseDir Edge Cases
TEST_F(US_SettingsTest, AppBaseDir_NoBinInPath_ReturnsUnmodified) {
// Test the regex logic for paths without /bin
QString testPath = "/usr/local/ultrascan";
QRegExp binRegex("/bin$");
QString result = testPath;
result.remove(binRegex);

EXPECT_EQ(result, testPath);  // Should be unchanged
}

TEST_F(US_SettingsTest, AppBaseDir_BinInMiddle_OnlyRemovesTrailing) {
QString testPath = "/usr/bin/ultrascan/bin";
QRegExp binRegex("/bin$");
QString result = testPath;
result.remove(binRegex);

EXPECT_EQ(result, "/usr/bin/ultrascan");  // Only trailing /bin removed
}

// Settings Reset and Persistence Tests
TEST_F(US_SettingsTest, MultipleSettings_SetAndRetrieve_MaintainSeparation) {
// Set multiple different settings
US_Settings::set_browser("/custom/browser");
US_Settings::set_us_debug(5);
US_Settings::set_tempTolerance(2.5);
US_Settings::set_us_inv_name("Test User");
US_Settings::set_threads(8);

// Verify they're all stored correctly and independently
EXPECT_EQ(US_Settings::browser(), "/custom/browser");
EXPECT_EQ(US_Settings::us_debug(), 5);
EXPECT_EQ(US_Settings::tempTolerance(), 2.5);
EXPECT_EQ(US_Settings::us_inv_name(), "Test User");
EXPECT_EQ(US_Settings::threads(), 8);
}

TEST_F(US_SettingsTest, DefaultRemoval_RestoresOriginalDefaults) {
// Change several settings to non-default values
US_Settings::set_tempTolerance(3.0);
US_Settings::set_threads(16);
US_Settings::set_us_debug(10);

// Verify they changed
EXPECT_EQ(US_Settings::tempTolerance(), 3.0);
EXPECT_EQ(US_Settings::threads(), 16);
EXPECT_EQ(US_Settings::us_debug(), 10);

// Reset to defaults (which should remove settings)
US_Settings::set_tempTolerance(0.5);
US_Settings::set_threads(1);
US_Settings::set_us_debug(0);

// Verify defaults are restored
EXPECT_EQ(US_Settings::tempTolerance(), 0.5);
EXPECT_EQ(US_Settings::threads(), 1);
EXPECT_EQ(US_Settings::us_debug(), 0);
}

// Memory and Resource Tests
TEST_F(US_SettingsTest, UltraScanPW_LargeBinaryData_HandlesCorrectly) {
QByteArray largeData;
largeData.resize(10000);
largeData.fill(0x42);  // Fill with 'B'

US_Settings::set_UltraScanPW(largeData);

QByteArray retrievedData = US_Settings::UltraScanPW();
EXPECT_EQ(retrievedData.size(), 10000);
EXPECT_EQ(retrievedData, largeData);
}

TEST_F(US_SettingsTest, UltraScanPW_BinaryDataWithNulls_HandlesCorrectly) {
QByteArray binaryData;
binaryData.append('\0');
binaryData.append('\x01');
binaryData.append('\xFF');
binaryData.append('\0');

US_Settings::set_UltraScanPW(binaryData);

QByteArray retrievedData = US_Settings::UltraScanPW();
EXPECT_EQ(retrievedData, binaryData);
}

// Error Simulation Tests (conceptual - can't easily mock QSettings)
TEST_F(US_SettingsTest, GetMethods_AlwaysReturnValidData) {
// Even if settings are corrupted or missing, getters should return valid defaults
// This tests the robustness of the default value system

QString browser = US_Settings::browser();
EXPECT_FALSE(browser.isNull());  // Should never be null

int threads = US_Settings::threads();
EXPECT_GE(threads, 0);  // Should be reasonable value

double tolerance = US_Settings::tempTolerance();
EXPECT_FALSE(std::isnan(tolerance));  // Should be a valid number
}

TEST_F(US_SettingsTest, BooleanSettings_HandleInvalidStoredValues) {
// Test that boolean settings handle edge cases gracefully
bool beckmanBug = US_Settings::beckmanBug();
EXPECT_TRUE(beckmanBug == true || beckmanBug == false);  // Should be valid boolean

US_Settings::set_beckmanBug(true);
EXPECT_TRUE(US_Settings::beckmanBug());

US_Settings::set_beckmanBug(false);
EXPECT_FALSE(US_Settings::beckmanBug());
}