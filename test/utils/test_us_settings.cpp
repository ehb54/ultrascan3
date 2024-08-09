#include "test_us_settings.h"
#include "us_settings.h"
#include <QSettings>

void TestUSSettings::testBrowserSettings()
{
    QString originalBrowser = US_Settings::browser();
    QString testBrowser = "/usr/bin/test_browser";
    US_Settings::set_browser(testBrowser);
    QCOMPARE(US_Settings::browser(), testBrowser);
    US_Settings::set_browser(originalBrowser);
}

void TestUSSettings::testWorkBaseDir()
{
    QString originalDir = US_Settings::workBaseDir();
    QString testDir = QDir::homePath() + "/test_ultrascan";
    US_Settings::set_workBaseDir(testDir);
    QCOMPARE(US_Settings::workBaseDir(), testDir);
    US_Settings::set_workBaseDir(originalDir);
}

void TestUSSettings::testImportDir()
{
    QString originalDir = US_Settings::importDir();
    QString testDir = US_Settings::workBaseDir() + "/test_imports";
    US_Settings::set_importDir(testDir);
    QCOMPARE(US_Settings::importDir(), testDir);
    US_Settings::set_importDir(originalDir);
}

void TestUSSettings::testTmpDir()
{
    QString originalDir = US_Settings::tmpDir();
    QString testDir = US_Settings::workBaseDir() + "/test_tmp";
    US_Settings::set_tmpDir(testDir);
    QCOMPARE(US_Settings::tmpDir(), testDir);
    US_Settings::set_tmpDir(originalDir);
}

void TestUSSettings::testDirectories()
{
    QCOMPARE(US_Settings::dataDir(), US_Settings::workBaseDir() + "/data");
    QCOMPARE(US_Settings::archiveDir(), US_Settings::workBaseDir() + "/archive");
    QCOMPARE(US_Settings::resultDir(), US_Settings::workBaseDir() + "/results");
    QCOMPARE(US_Settings::reportDir(), US_Settings::workBaseDir() + "/reports");
    QCOMPARE(US_Settings::etcDir(), US_Settings::workBaseDir() + "/etc");
}

void TestUSSettings::testLicenseSettings()
{
    QStringList originalLicense = US_Settings::license();
    QStringList testLicense = {"Test", "License"};
    US_Settings::set_license(testLicense);
    QCOMPARE(US_Settings::license(), testLicense);
    US_Settings::set_license(originalLicense);
}

void TestUSSettings::testPasswordSettings()
{
    QByteArray originalPW = US_Settings::UltraScanPW();
    QByteArray testPW = "test_password";
    US_Settings::set_UltraScanPW(testPW);
    QCOMPARE(US_Settings::UltraScanPW(), testPW);
    US_Settings::set_UltraScanPW(originalPW);
}

void TestUSSettings::testTemperatureTolerance()
{
    double originalTolerance = US_Settings::tempTolerance();
    double testTolerance = 1.0;
    US_Settings::set_tempTolerance(testTolerance);
    QCOMPARE(US_Settings::tempTolerance(), testTolerance);
    US_Settings::set_tempTolerance(originalTolerance);
}

void TestUSSettings::testBeckmanBug()
{
    bool originalBug = US_Settings::beckmanBug();
    US_Settings::set_beckmanBug(true);
    QCOMPARE(US_Settings::beckmanBug(), true);
    US_Settings::set_beckmanBug(false);
    QCOMPARE(US_Settings::beckmanBug(), false);
    US_Settings::set_beckmanBug(originalBug);
}

void TestUSSettings::testDefaultDataLocation()
{
    int originalLocation = US_Settings::default_data_location();
    int testLocation = 1;
    US_Settings::set_default_data_location(testLocation);
    QCOMPARE(US_Settings::default_data_location(), testLocation);
    US_Settings::set_default_data_location(originalLocation);
}

void TestUSSettings::testDebugSettings()
{
#ifndef NO_DB
    int originalDebugLevel = US_Settings::us_debug();
    int testDebugLevel = 2;
    US_Settings::set_us_debug(testDebugLevel);
    QCOMPARE(US_Settings::us_debug(), testDebugLevel);
    US_Settings::set_us_debug(originalDebugLevel);
#endif
}

void TestUSSettings::testInvestigatorSettings()
{
    QString originalName = US_Settings::us_inv_name();
    QString testName = "Test Investigator";
    US_Settings::set_us_inv_name(testName);
    QCOMPARE(US_Settings::us_inv_name(), testName);
    US_Settings::set_us_inv_name(originalName);

    int originalID = US_Settings::us_inv_ID();
    int testID = 1234;
    US_Settings::set_us_inv_ID(testID);
    QCOMPARE(US_Settings::us_inv_ID(), testID);
    US_Settings::set_us_inv_ID(originalID);

    int originalLevel = US_Settings::us_inv_level();
    int testLevel = 5;
    US_Settings::set_us_inv_level(testLevel);
    QCOMPARE(US_Settings::us_inv_level(), testLevel);
    US_Settings::set_us_inv_level(originalLevel);
}

void TestUSSettings::testAdvancedLevel()
{
    int originalLevel = US_Settings::advanced_level();
    int testLevel = 3;
    US_Settings::set_advanced_level(testLevel);
    QCOMPARE(US_Settings::advanced_level(), testLevel);
    US_Settings::set_advanced_level(originalLevel);
}

void TestUSSettings::testThreads()
{
    int originalThreads = US_Settings::threads();
    int testThreads = 4;
    US_Settings::set_threads(testThreads);
    QCOMPARE(US_Settings::threads(), testThreads);
    US_Settings::set_threads(originalThreads);
}

void TestUSSettings::testNoiseDialog()
{
    int originalDialog = US_Settings::noise_dialog();
    int testDialog = 1;
    US_Settings::set_noise_dialog(testDialog);
    QCOMPARE(US_Settings::noise_dialog(), testDialog);
    US_Settings::set_noise_dialog(originalDialog);
}

void TestUSSettings::testDatabases()
{
    QList<QStringList> originalDatabases = US_Settings::databases();
    QList<QStringList> testDatabases = {{"TestDB1", "localhost", "user1", "pass1"}, {"TestDB2", "localhost", "user2", "pass2"}};
    US_Settings::set_databases(testDatabases);
    QCOMPARE(US_Settings::databases(), testDatabases);
    US_Settings::set_databases(originalDatabases);
}

void TestUSSettings::testXpnDbHosts()
{
    QList<QStringList> originalHosts = US_Settings::xpn_db_hosts();
    QList<QStringList> testHosts = {{"Host1", "localhost", "user1", "pass1"}, {"Host2", "localhost", "user2", "pass2"}};
    US_Settings::set_xpn_db_hosts(testHosts);
    QCOMPARE(US_Settings::xpn_db_hosts(), testHosts);
    US_Settings::set_xpn_db_hosts(originalHosts);
}

void TestUSSettings::testDAStatus()
{
    US_Settings::set_DA_status("COM:1");
    QVERIFY(US_Settings::get_DA_status("COM"));
    US_Settings::set_DA_status("COM:0");
    QVERIFY(!US_Settings::get_DA_status("COM"));

    US_Settings::set_DA_status("ACAD:1");
    QVERIFY(US_Settings::get_DA_status("ACAD"));
    US_Settings::set_DA_status("ACAD:0");
    QVERIFY(!US_Settings::get_DA_status("ACAD"));
}

void TestUSSettings::testStatus()
{
    QString status = US_Settings::status();
    QCOMPARE(status, QString());
}
