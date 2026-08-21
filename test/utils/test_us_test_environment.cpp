#include "qt_test_base.h"
#include "us_defines.h"
#include "us_settings.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QSettings>

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

bool isWithin( const QString& child, const QString& parent )
{
    const QString cleanChild = QDir::cleanPath( child );
    QString cleanParent = QDir::cleanPath( parent );
    if ( ! cleanParent.endsWith( QDir::separator() ) )
        cleanParent += QDir::separator();

    return cleanChild == QDir::cleanPath( parent ) ||
           cleanChild.startsWith( cleanParent );
}
}

TEST( TestEnvironmentIsolation, SettingsAndWritablePathsStayInSandbox )
{
    const QString sandbox = qEnvironmentVariable( "US3_TEST_SANDBOX" );
    const QString settingsRoot =
        qEnvironmentVariable( "US3_TEST_SETTINGS_ROOT" );
    const QString workRoot = qEnvironmentVariable( "US3_TEST_WORK_ROOT" );

    ASSERT_FALSE( sandbox.isEmpty() );
    ASSERT_TRUE( QDir( sandbox ).exists() );
    ASSERT_TRUE( isWithin( settingsRoot, sandbox ) );
    ASSERT_TRUE( isWithin( workRoot, sandbox ) );

    QSettings settings( US3, "UltraScan" );
    ASSERT_TRUE( isWithin( settings.fileName(), settingsRoot ) )
        << settings.fileName().toStdString();

    settings.setValue( "ut003/isolation-sentinel", "sandbox-only" );
    settings.sync();
    ASSERT_EQ( settings.status(), QSettings::NoError );
    EXPECT_EQ( settings.value( "ut003/isolation-sentinel" ).toString(),
               QString( "sandbox-only" ) );

    const QStringList writablePaths = {
        US_Settings::workBaseDir(), US_Settings::importDir(),
        US_Settings::tmpDir(), US_Settings::dataDir(),
        US_Settings::archiveDir(), US_Settings::resultDir(),
        US_Settings::reportDir(), US_Settings::etcDir()
    };

    for ( const QString& path : writablePaths )
        EXPECT_TRUE( isWithin( path, workRoot ) )
            << path.toStdString();

    const QString probeDir = US_Settings::dataDir() + "/ut003";
    ASSERT_TRUE( QDir().mkpath( probeDir ) );
    QFile probe( probeDir + "/sandbox-probe" );
    ASSERT_TRUE( probe.open( QIODevice::WriteOnly ) );
    ASSERT_EQ( probe.write( "isolated" ), 8 );
    probe.close();
    EXPECT_TRUE( isWithin( probe.fileName(), sandbox ) );

    const QString normalSettingsFile =
        qEnvironmentVariable( "US3_TEST_NORMAL_SETTINGS_FILE" );
    const bool normalSettingsExisted =
        qEnvironmentVariable( "US3_TEST_NORMAL_SETTINGS_EXISTED" ) == "1";
    const QByteArray originalDigest =
        qgetenv( "US3_TEST_NORMAL_SETTINGS_SHA256" );

    ASSERT_FALSE( normalSettingsFile.isEmpty() );
    EXPECT_NE( QDir::cleanPath( settings.fileName() ),
               QDir::cleanPath( normalSettingsFile ) );
    EXPECT_EQ( QFile::exists( normalSettingsFile ), normalSettingsExisted );
    if ( normalSettingsExisted ) {
        ASSERT_FALSE( originalDigest.isEmpty() );
        EXPECT_EQ( fileDigest( normalSettingsFile ), originalDigest );
    }
}
