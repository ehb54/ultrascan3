// The GUI test harness must keep its settings inside its sandbox.
//
// This is the counterpart of TestEnvironmentIsolation in test/utils, which
// covers the Google Test harness only. The GUI harness had the same defect and
// kept it after the other was fixed: it redirected the store after constructing
// QApplication, by which time the theme had already read a setting and pinned
// the store to the real one. A GUI test run then wrote the sandbox paths into
// the developer's own UltraScan preferences, pointing workBaseDir at a
// directory that is deleted when the test exits and breaking every UltraScan
// program on that machine until the key was removed by hand.

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

    // The paths the harness sets are the ones a leak would write into the real
    // store, so they are what this has to pin.
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

    // The store is resolved once, on the first read.  If any of the above ever
    // resolves late, this is the assertion that says so directly.
    void storeFormatIsNotTheNativeOne()
    {
        QCOMPARE(US_SettingsStore::format(), QSettings::IniFormat);
    }
};

US3_GUI_TEST_MAIN(US_GuiTestIsolationTest)

#include "test_us_gui_test_isolation.moc"
