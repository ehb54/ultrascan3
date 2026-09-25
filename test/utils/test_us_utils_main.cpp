// test_us_utils_main.cpp - Google Test
#include "qt_test_base.h"
#include "us_defines.h"
#include "us_settings.h"
#include <QCoreApplication>
#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>

namespace
{
QByteArray fileDigest( const QString& path )
{
    QFile file( path );
    if ( ! file.open( QIODevice::ReadOnly ) )
        return QByteArray();

    QCryptographicHash hash( QCryptographicHash::Sha256 );
    while ( ! file.atEnd() )
        hash.addData( file.read( 64 * 1024 ) );

    return hash.result().toHex();
}
}

// Qt Test Environment for Google Test
class QtTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        // Fingerprint user settings to detect writes without using their values.
        QSettings normalSettings( QSettings::NativeFormat,
                                  QSettings::UserScope, US3, "UltraScan" );
        const QString normalSettingsFile = normalSettings.fileName();
        const bool normalSettingsExists = QFile::exists( normalSettingsFile );
        const QByteArray normalSettingsDigest = fileDigest( normalSettingsFile );
        if ( normalSettingsExists )
            ASSERT_FALSE( normalSettingsDigest.isEmpty() )
                << "Could not fingerprint the normal user settings file";

        sandbox_ = std::make_unique<QTemporaryDir>(
            QDir::tempPath() + "/ultrascan3-utils-tests-XXXXXX" );
        ASSERT_TRUE( sandbox_->isValid() )
            << "Could not create the process-local test sandbox";

        const QString root = sandbox_->path();
        const QString settingsRoot = root + "/settings";
        const QString workRoot = root + "/work";
        const QString importRoot = workRoot + "/imports";
        const QString temporaryRoot = workRoot + "/tmp";

        ASSERT_TRUE( QDir().mkpath( settingsRoot ) );
        ASSERT_TRUE( QDir().mkpath( importRoot ) );
        ASSERT_TRUE( QDir().mkpath( temporaryRoot ) );

        qputenv( "US3_TEST_SANDBOX", QFile::encodeName( root ) );
        qputenv( "US3_TEST_SETTINGS_ROOT", QFile::encodeName( settingsRoot ) );
        qputenv( "US3_TEST_WORK_ROOT", QFile::encodeName( workRoot ) );
        qputenv( "US3_TEST_NORMAL_SETTINGS_FILE",
                 QFile::encodeName( normalSettingsFile ) );
        qputenv( "US3_TEST_NORMAL_SETTINGS_EXISTED",
                 normalSettingsExists ? "1" : "0" );
        qputenv( "US3_TEST_NORMAL_SETTINGS_SHA256", normalSettingsDigest );

        // Set before first use: US_SettingsStore caches this override.
        // Unlike setPath alone, it also isolates native macOS/Windows settings.
        qputenv( "US3_SETTINGS_ROOT", QFile::encodeName( settingsRoot ) );

        QSettings::setPath( QSettings::IniFormat, QSettings::UserScope,
                            settingsRoot );
        QSettings::setPath( QSettings::IniFormat, QSettings::SystemScope,
                            settingsRoot );
        QStandardPaths::setTestModeEnabled( true );

        if (!QCoreApplication::instance()) {
            static int argc = 1;
            static char* argv[] = {const_cast<char*>("test")};
            app_ = new QCoreApplication(argc, argv);
        }
        qputenv("QT_QPA_PLATFORM", "offscreen");

        // Optional: Suppress Qt debug output during tests
        qputenv("QT_LOGGING_RULES", "*.debug=false");

        // Initialize writable paths in this process's sandbox before tests run.
        US_Settings::set_workBaseDir( workRoot );
        US_Settings::set_importDir( importRoot );
        US_Settings::set_tmpDir( temporaryRoot );
        US_Settings::set_debug_text( QStringList() );
    }

    void TearDown() override {
        // Cleanup handled automatically
    }

private:
    QCoreApplication* app_ = nullptr;
    std::unique_ptr<QTemporaryDir> sandbox_;
};

int main(int argc, char **argv) {
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Add Qt environment
    ::testing::AddGlobalTestEnvironment(new QtTestEnvironment);

    // Run all tests - Google Test automatically discovers them!
    return RUN_ALL_TESTS();
}
