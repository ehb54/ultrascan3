#include "qt_test_base.h"
#include "us3_file_test_base.h"
#include "us3_test_io.h"
#include "us3_test_scope.h"
#include "us_defines.h"
#include "us_settings.h"
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <cstdlib>
#include <gtest/gtest-spi.h>

#include <QDir>
#include <QFile>
#include <QSettings>

namespace
{
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

    US_SettingsStore settings;
    ASSERT_TRUE( isWithin( settings.fileName(), settingsRoot ) )
        << settings.fileName().toStdString();

    settings.setValue( "test-environment/isolation-sentinel", "sandbox-only" );
    settings.sync();
    ASSERT_EQ( settings.status(), QSettings::NoError );
    EXPECT_EQ( settings.value( "test-environment/isolation-sentinel" ).toString(),
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

    const QString probeDir = US_Settings::dataDir() + "/test-environment";
    ASSERT_TRUE( QDir().mkpath( probeDir ) );
    QFile probe( probeDir + "/sandbox-probe" );
    ASSERT_TRUE( probe.open( QIODevice::WriteOnly ) );
    ASSERT_EQ( probe.write( "isolated" ), 8 );
    probe.close();
    EXPECT_TRUE( isWithin( probe.fileName(), sandbox ) );
}

// Nested scopes let us verify teardown immediately, without relying on test order.
TEST( TestEnvironmentIsolation, RestoresExactKeysAndProcessOnlyState )
{
   US_SettingsStore before;
   before.setValue( "isolation/existing", QByteArray( "original" ) );
   before.remove( "isolation/absent" );
   before.remove( "us_debug" );
   before.sync();
   US_Settings::set_default_data_location_override( 1 );
   const int         debug     = US_Settings::us_debug();
   const QStringList debugText = US_Settings::debug_text();
   const QStringList keys      = before.allKeys();
   {
      Us3SettingsGuard guard;
      ASSERT_TRUE( guard.valid() );
      US_SettingsStore changed;
      changed.remove( "isolation/existing" );
      changed.setValue( "isolation/absent", "new" );
      US_Settings::set_DA_status( "ISOLATION:1" );
      US_Settings::set_us_debug( 7 );
      US_Settings::set_debug_text( { "test-only" } );
      US_Settings::clear_default_data_location_override();
      EXPECT_TRUE( guard.restore() );
   }
   US_SettingsStore after;
   EXPECT_EQ( after.allKeys(), keys );
   EXPECT_EQ( after.value( "isolation/existing" ).toByteArray(), QByteArray( "original" ) );
   EXPECT_FALSE( after.contains( "isolation/absent" ) );
   EXPECT_FALSE( after.contains( "us_debug" ) );
   EXPECT_EQ( US_Settings::us_debug(), debug );
   EXPECT_EQ( US_Settings::debug_text(), debugText );
   EXPECT_EQ( US_Settings::default_data_location_override(), 1 );
}

TEST( TestEnvironmentIsolation, RestoresAbsentOverrideAfterNestedScopes )
{
   US_Settings::clear_default_data_location_override();
   US_Settings::set_default_data_location( 1 );
   EXPECT_EQ( US_Settings::default_data_location_override(), -1 );
   {
      Us3SettingsGuard outer;
      ASSERT_TRUE( outer.valid() );
      US_Settings::set_default_data_location_override( 2 );
      {
         Us3SettingsGuard inner;
         ASSERT_TRUE( inner.valid() );
         US_Settings::set_default_data_location_override( 1 );
      }
      EXPECT_EQ( US_Settings::default_data_location_override(), 2 );
      EXPECT_EQ( US_Settings::default_data_location(), 2 );
   }
   EXPECT_EQ( US_Settings::default_data_location_override(), -1 );
   EXPECT_EQ( US_Settings::default_data_location(), 1 );
}

TEST( TestEnvironmentIsolation, NestedWorkDirectoriesAreUniqueAndRemoved )
{
   const QString originalWork = US_Settings::workBaseDir();
   const QString originalCwd  = QDir::currentPath();
   QString       firstPath;
   QString       secondPath;
   {
      Us3TestScope first;
      ASSERT_TRUE( first.valid() );
      firstPath = first.path();
      {
         Us3TestScope second;
         ASSERT_TRUE( second.valid() );
         secondPath = second.path();
         EXPECT_NE( firstPath, secondPath );
         EXPECT_EQ( US_Settings::workBaseDir(), secondPath );
         QFile file( "owned-file" );
         ASSERT_TRUE( file.open( QIODevice::WriteOnly ) );
      }
      EXPECT_FALSE( QDir( secondPath ).exists() );
      EXPECT_EQ( US_Settings::workBaseDir(), firstPath );
      EXPECT_EQ( QDir::currentPath(), firstPath );
   }
   EXPECT_FALSE( QDir( firstPath ).exists() );
   EXPECT_EQ( US_Settings::workBaseDir(), originalWork );
   EXPECT_EQ( QDir::currentPath(), originalCwd );
}

namespace
{
void failInsideIsolatedScope()
{
   Us3TestScope scope;
   ASSERT_TRUE( scope.valid() );
   US_SettingsStore settings;
   settings.setValue( "isolation/failed-case", true );
   US_Settings::set_default_data_location_override( 1 );
   FAIL() << "intentional fixture failure";
}
}

TEST( TestEnvironmentIsolation, FatalAssertionStillRestoresScope )
{
   const QString     work          = US_Settings::workBaseDir();
   const QString     cwd           = QDir::currentPath();
   const int         overrideValue = US_Settings::default_data_location_override();
   const QDir        processRoot( qEnvironmentVariable( "US3_TEST_WORK_ROOT" ) );
   const QStringList directories = processRoot.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
   EXPECT_FATAL_FAILURE( failInsideIsolatedScope(), "intentional fixture failure" );
   EXPECT_EQ( US_Settings::workBaseDir(), work );
   EXPECT_EQ( QDir::currentPath(), cwd );
   EXPECT_EQ( US_Settings::default_data_location_override(), overrideValue );
   EXPECT_FALSE( US_SettingsStore().contains( "isolation/failed-case" ) );
   EXPECT_EQ( processRoot.entryList( QDir::Dirs | QDir::NoDotAndDotDot ), directories );
}

// The helpers below deliberately exercise setup/cleanup failures rather than
// only testing the happy-path implementation of the fixture.


TEST( TestEnvironmentIsolation, CheckedWriterReportsSetupFailure )
{
   const QString    root = US_Settings::workBaseDir();
   const QByteArray bytes( "a\0b\n", 4 );
   ASSERT_TRUE( us3WriteTestFile( root + "/nested/input", bytes ) );
   QFile input( root + "/nested/input" );
   ASSERT_TRUE( input.open( QIODevice::ReadOnly ) );
   EXPECT_EQ( input.readAll(), bytes );
   EXPECT_FALSE( us3WriteTestFile( root + "/nested/input/blocked", "invalid" ) );
   EXPECT_FALSE( us3WriteTestFile( root + "/nested", "invalid" ) );
   EXPECT_FALSE( QFile::exists( root + "/nested/input/blocked" ) );
}

namespace
{
void failWithEnvironmentOverride()
{
   Us3EnvironmentGuard environment;
   ASSERT_TRUE( environment.set( "US3_FIXTURE_ENV_CHECK", "changed" ) );
   FAIL() << "intentional environment failure";
}

class FailedFileSetupProbe : public Us3FileTestBase
{
   public:
      bool initialized = false;
      void SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( Us3FileTestBase::SetUp() );
         initialized = true;
      }
      void TestBody() override
      {
      }
      void TearDown() override
      {
         Us3FileTestBase::TearDown();
      }
};
}

TEST( TestEnvironmentIsolation, EnvironmentGuardRestoresAbsentEmptyAndPopulatedValues )
{
   Us3EnvironmentGuard original;
   const QByteArray    name( "US3_FIXTURE_ENV_CHECK" );
   for ( int state = 0; state < 3; ++state )
   {
      if ( state == 0 )
      {
         ASSERT_TRUE( original.unset( name ) );
      }
      else
      {
         ASSERT_TRUE( original.set( name, state == 1 ? QByteArray( "" ) : QByteArray( "before" ) ) );
      }
      const bool       present = qEnvironmentVariableIsSet( name.constData() );
      const QByteArray before  = qgetenv( name.constData() );
      EXPECT_FATAL_FAILURE( failWithEnvironmentOverride(), "intentional environment failure" );
      EXPECT_EQ( qEnvironmentVariableIsSet( name.constData() ), present );
      EXPECT_EQ( qgetenv( name.constData() ), before );
      {
         Us3EnvironmentGuard nested;
         ASSERT_TRUE( nested.set( name, "first" ) );
         ASSERT_TRUE( nested.unset( name ) );
         ASSERT_TRUE( nested.set( name, "last" ) );
      }
      EXPECT_EQ( qEnvironmentVariableIsSet( name.constData() ), present );
      EXPECT_EQ( qgetenv( name.constData() ), before );
   }
}

TEST( TestEnvironmentIsolation, FailedBaseSetupStopsDerivedInitializationAndRestoresState )
{
   const QString     work        = US_Settings::workBaseDir();
   const QString     cwd         = QDir::currentPath();
   const QStringList directories = QDir( work ).entryList( QDir::Dirs | QDir::NoDotAndDotDot );
   const QString     blocker     = work + "/blocker";
   ASSERT_TRUE( us3WriteTestFile( blocker, "not a directory" ) );
   {
      Us3TestScope scope;
      ASSERT_TRUE( scope.valid() );
      US_Settings::set_workBaseDir( blocker );
      FailedFileSetupProbe           fixture;
      ::testing::TestPartResultArray failures;
      {
         ::testing::ScopedFakeTestPartResultReporter reporter(
            ::testing::ScopedFakeTestPartResultReporter::INTERCEPT_ONLY_CURRENT_THREAD, &failures );
         fixture.SetUp();
      }
      // Exercise our failed base/derived setup and partial cleanup. GoogleTest
      // itself owns the established rule that a fatal SetUp skips TestBody.
      fixture.TearDown();
      ASSERT_GT( failures.size(), 0 );
      bool fatal = false;
      for ( int i = 0; i < failures.size(); ++i )
      {
         fatal = fatal || failures.GetTestPartResult( i ).fatally_failed();
      }
      EXPECT_TRUE( fatal );
      EXPECT_FALSE( fixture.initialized );
   }
   EXPECT_EQ( US_Settings::workBaseDir(), work );
   EXPECT_EQ( QDir::currentPath(), cwd );
   EXPECT_EQ( QDir( work ).entryList( QDir::Dirs | QDir::NoDotAndDotDot ), directories );
}

namespace
{
QJsonObject awaitProbe( QProcess& child )
{
   QElapsedTimer deadline;
   deadline.start();
   QByteArray pending;
   while ( deadline.elapsed() < 10000 )
   {
      pending += child.readAllStandardOutput();
      int end;
      while ( ( end = pending.indexOf( '\n' ) ) >= 0 )
      {
         const QByteArray line = pending.left( end );
         pending.remove( 0, end + 1 );
         if ( line.startsWith( "US3_PROBE:" ) )
         {
            return QJsonDocument::fromJson( line.mid( 10 ) ).object();
         }
      }
      if ( child.state() == QProcess::NotRunning )
      {
         break;
      }
      child.waitForReadyRead( 100 );
   }
   return {};
}

// QProcess already kills an outstanding child in its destructor. Explicit
// cleanup here also bounds failure handling and avoids destructor warnings.
class ProbeProcess : public QProcess
{
   public:
      ~ProbeProcess()
      {
         if ( state() != NotRunning )
         {
            kill();
            waitForFinished( 3000 );
         }
      }
};
}

TEST( TestEnvironmentIsolation, OverlappingProcessesKeepSettingsAndFilesSeparate )
{
   QTemporaryDir profile( US_Settings::workBaseDir() + "/profile-XXXXXX" );
   ASSERT_TRUE( profile.isValid() );
   // Own child roots in the parent too, so a killed child cannot leak its sandbox.
   QTemporaryDir childRoots( US_Settings::workBaseDir() + "/children-XXXXXX" );
   ASSERT_TRUE( childRoots.isValid() );
   const QString    sentinel = profile.path() + "/controlled-profile.ini";
   const QByteArray original( "[profile]\nsentinel=unchanged\n" );
   ASSERT_TRUE( us3WriteTestFile( sentinel, original ) );
   ProbeProcess first, second;
   int          index = 0;
   for ( QProcess* child : { &first, &second } )
   {
      QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
      env.insert( "US3_PROBE_TOKEN", index++ == 0 ? "A" : "B" );
      env.insert( "HOME", profile.path() );
      env.insert( "XDG_CONFIG_HOME", profile.path() );
      env.insert( "US3_SETTINGS_ROOT", profile.path() ); // bootstrap must replace this
      for ( const char* name : { "TMPDIR", "TMP", "TEMP" } )
      {
         env.insert( name, childRoots.path() );
      }
      child->setProcessEnvironment( env );
      child->setWorkingDirectory( profile.path() );
      child->start( QCoreApplication::applicationFilePath(), { "--us3-isolation-probe" } );
      ASSERT_TRUE( child->waitForStarted( 10000 ) );
   }
   const QJsonObject a = awaitProbe( first );
   const QJsonObject b = awaitProbe( second );
   ASSERT_FALSE( a.isEmpty() ) << first.readAllStandardError().constData();
   ASSERT_FALSE( b.isEmpty() ) << second.readAllStandardError().constData();
   for ( const char* key : { "sandbox", "settings", "work" } )
   {
      ASSERT_FALSE( a[key].toString().isEmpty() );
      ASSERT_FALSE( b[key].toString().isEmpty() );
      EXPECT_NE( a[key], b[key] );
      EXPECT_TRUE( QFileInfo::exists( a[key].toString() ) );
      EXPECT_TRUE( QFileInfo::exists( b[key].toString() ) );
   }
   // Both have written before either verifies: an actual overlap, not two
   // sequential invocations that could accidentally share the same paths.
   ASSERT_EQ( first.write( "verify\n" ), 7 );
   ASSERT_EQ( second.write( "verify\n" ), 7 );
   for ( QProcess* child : { &first, &second } )
   {
      ASSERT_TRUE( child->state() == QProcess::NotRunning || child->waitForFinished( 10000 ) )
         << child->readAllStandardError().constData();
      EXPECT_EQ( child->exitStatus(), QProcess::NormalExit );
      EXPECT_EQ( child->exitCode(), 0 ) << child->readAllStandardError().constData();
   }
   EXPECT_FALSE( QFileInfo::exists( a["sandbox"].toString() ) );
   EXPECT_FALSE( QFileInfo::exists( b["sandbox"].toString() ) );
   EXPECT_TRUE( QDir( childRoots.path() ).entryList( QDir::AllEntries | QDir::NoDotAndDotDot ).isEmpty() );
   QFile unchanged( sentinel );
   ASSERT_TRUE( unchanged.open( QIODevice::ReadOnly ) );
   EXPECT_EQ( unchanged.readAll(), original );
   EXPECT_EQ( QDir( profile.path() ).entryList( QDir::AllEntries | QDir::NoDotAndDotDot ),
              QStringList{ "controlled-profile.ini" } );
}

#if GTEST_HAS_DEATH_TEST
TEST( TestEnvironmentIsolation, ThreadsafeDeathChildrenAndTheirTemporaryFilesAreParentOwned )
{
   ::testing::GTEST_FLAG( death_test_style ) = "threadsafe";
   QString ownedRoot;
   {
      Us3TestScope owner;
      ASSERT_TRUE( owner.valid() );
      ownedRoot = owner.path();
      Us3EnvironmentGuard environment;
      ASSERT_TRUE( environment.set( "US3_TEST_WORK_ROOT", QFile::encodeName( ownedRoot ) ) );
      for ( int child = 0; child < 2; ++child )
      {
         ASSERT_EXIT(
            {
               // Neither this directory nor the process sandbox gets a
               // destructor when the death-test child exits.
               QTemporaryDir orphan;
               const QString sandbox = qEnvironmentVariable( "US3_TEST_SANDBOX" );
               if ( !orphan.isValid() || !isWithin( orphan.path(), sandbox ) )
               {
                  std::_Exit( 11 );
               }
               if ( !us3WriteTestFile( orphan.path() + "/orphan", "parent must clean this" ) )
               {
                  std::_Exit( 12 );
               }
               std::_Exit( 0 );
            },
            ::testing::ExitedWithCode( 0 ),
            "" );
      }
      // Re-exec children must be underneath our owner, not in global /tmp.
      const QStringList children = QDir( ownedRoot ).entryList( { "ultrascan3-utils-tests-*" }, QDir::Dirs | QDir::NoDotAndDotDot );
      EXPECT_EQ( children.size(), 2 );
   }
   EXPECT_FALSE( QFileInfo::exists( ownedRoot ) );
}
#endif
