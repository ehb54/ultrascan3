#ifndef TEST_US_SETTINGS_H
#define TEST_US_SETTINGS_H

#include <QtTest/QtTest>

class TestUSSettings : public QObject
{
Q_OBJECT

private slots:
    void testBrowserSettings();
    void testWorkBaseDir();
    void testImportDir();
    void testTmpDir();
    void testDirectories();
    void testLicenseSettings();
    void testPasswordSettings();
    void testTemperatureTolerance();
    void testBeckmanBug();
    void testDefaultDataLocation();
    void testDebugSettings();
    void testInvestigatorSettings();
    void testAdvancedLevel();
    void testThreads();
    void testNoiseDialog();
    void testDatabases();
    void testXpnDbHosts();
    void testDAStatus();
    void testStatus();
};

#endif // TEST_US_SETTINGS_H
