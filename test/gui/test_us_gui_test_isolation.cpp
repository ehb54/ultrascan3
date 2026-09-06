// Verify GUI tests isolate settings before QApplication caches the store,
// keeping temporary paths out of the user's UltraScan preferences.

#include "us3_gui_test_main.h"

#include "us_settings.h"

namespace
{
bool isWithin(const QString& child, const QString& parent)
{
    const QString cleanChild = QDir::cleanPath(child);
    QString cleanParent = QDir::cleanPath(parent);
    if (!cleanParent.endsWith(QDir::separator()))
        cleanParent += QDir::separator();

    return cleanChild == QDir::cleanPath(parent) ||
           cleanChild.startsWith(cleanParent);
}
}

class US_GuiTestIsolationTest : public QObject
{
    Q_OBJECT

private slots:
    void settingsStoreIsTheSandboxOne()
    {
        const QString settingsRoot =
            qEnvironmentVariable("US3_TEST_SETTINGS_ROOT");
        QVERIFY(!settingsRoot.isEmpty());

        US_SettingsStore settings;
        QVERIFY2(isWithin(settings.fileName(), settingsRoot),
                 qPrintable(settings.fileName()));

        settings.setValue("gui-isolation/sentinel", "sandbox-only");
        settings.sync();
        QCOMPARE(settings.status(), QSettings::NoError);
        QCOMPARE(settings.value("gui-isolation/sentinel").toString(),
                 QString("sandbox-only"));
    }

    // Check every writable path stays in the sandbox.
    void writablePathsAreTheSandboxOnes()
    {
        const QString workRoot = qEnvironmentVariable("US3_TEST_WORK_ROOT");
        QVERIFY(!workRoot.isEmpty());

        const QStringList paths = {
            US_Settings::workBaseDir(), US_Settings::importDir(),
            US_Settings::tmpDir(),      US_Settings::dataDir(),
            US_Settings::archiveDir(),  US_Settings::resultDir(),
            US_Settings::reportDir(),   US_Settings::etcDir()
        };

        for (const QString& path : paths)
            QVERIFY2(isWithin(path, workRoot), qPrintable(path));
    }

    // A late redirect leaves the cached format native.
    void storeFormatIsNotTheNativeOne()
    {
        QCOMPARE(US_SettingsStore::format(), QSettings::IniFormat);
    }
};

US3_GUI_TEST_MAIN(US_GuiTestIsolationTest)

#include "test_us_gui_test_isolation.moc"
