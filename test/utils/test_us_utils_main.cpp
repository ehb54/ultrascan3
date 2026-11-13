// test_us_utils_main.cpp - Google Test
#include "qt_test_base.h"
#include "us3_test_io.h"
#include "us3_test_scope.h"
#ifdef US3_CHECK_UNIT_RESOURCES
#include "unit_resources.h"
#endif
#include "us_defines.h"
#include "us_settings.h"
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>
#include <cstdio>
#include <cstdlib>

// Qt Test Environment for Google Test
class QtTestEnvironment : public ::testing::Environment {
public:
   explicit QtTestEnvironment( bool deathTestChild = false ) : deathTestChild_( deathTestChild )
   {
   }

   void SetUp() override
   {
      originalCwd_   = QDir::currentPath();
      QString parent = QDir::tempPath();
      if ( deathTestChild_ )
      {
         // Re-exec death-test children exit through _exit: their destructors and
         // test listeners do not run. Give their files to the surviving parent.
         const QString inheritedSandbox = qEnvironmentVariable( "US3_TEST_SANDBOX" );
         parent                         = qEnvironmentVariable( "US3_TEST_WORK_ROOT" );
         ASSERT_FALSE( inheritedSandbox.isEmpty() );
         ASSERT_TRUE( QDir( parent ).exists() && QDir::cleanPath( parent ).startsWith( QDir::cleanPath( inheritedSandbox ) + "/" ) )
            << "Death-test child requires an inherited sandbox work root";
      }
      sandbox_ = std::make_unique<QTemporaryDir>( parent + "/ultrascan3-utils-tests-XXXXXX" );
      ASSERT_TRUE( sandbox_->isValid() ) << "Could not create the process-local test sandbox";

      const QString root          = sandbox_->path();
      const QString settingsRoot  = root + "/settings";
      const QString workRoot      = root + "/work";
      const QString importRoot    = workRoot + "/imports";
      const QString temporaryRoot = workRoot + "/tmp";

      ASSERT_TRUE( QDir().mkpath( settingsRoot ) );
      ASSERT_TRUE( QDir().mkpath( importRoot ) );
      ASSERT_TRUE( QDir().mkpath( temporaryRoot ) );

      if ( deathTestChild_ )
      {
         // QTemporaryDir/QTemporaryFile created while the child replays the test
         // must also be parent-owned, including setup before the death statement.
         for ( const char* name : { "TMPDIR", "TMP", "TEMP" } )
         {
            ASSERT_TRUE( qputenv( name, QFile::encodeName( temporaryRoot ) ) );
         }
         ASSERT_TRUE( QDir::setCurrent( workRoot ) );
      }

      qputenv( "US3_TEST_SANDBOX", QFile::encodeName( root ) );
      qputenv( "US3_TEST_SETTINGS_ROOT", QFile::encodeName( settingsRoot ) );
      qputenv( "US3_TEST_WORK_ROOT", QFile::encodeName( workRoot ) );
      // Set before first use: US_SettingsStore caches this override.
      // Unlike setPath alone, it also isolates native macOS/Windows settings.
      qputenv( "US3_SETTINGS_ROOT", QFile::encodeName( settingsRoot ) );

      QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, settingsRoot );
      QSettings::setPath( QSettings::IniFormat, QSettings::SystemScope, settingsRoot );
      QStandardPaths::setTestModeEnabled( true );

      if ( !QCoreApplication::instance() )
      {
         static int   argc   = 1;
         static char* argv[] = { const_cast<char*>( "test" ) };
         app_                = new QCoreApplication( argc, argv );
      }
      qputenv( "QT_QPA_PLATFORM", "offscreen" );

      // Optional: Suppress Qt debug output during tests
      qputenv( "QT_LOGGING_RULES", "*.debug=false" );

      // Initialize writable paths in this process's sandbox before tests run.
      US_Settings::set_workBaseDir( workRoot );
      US_Settings::set_importDir( importRoot );
      US_Settings::set_tmpDir( temporaryRoot );
      US_Settings::set_debug_text( QStringList() );
   }

    void TearDown() override {
       // Normal cleanup is owned by QTemporaryDir.
    }

    bool removeSandbox()
    {
       const bool cwdRestored = QDir::setCurrent( originalCwd_ );
       const bool removed     = !sandbox_ || sandbox_->remove();
       return cwdRestored && removed;
    }

private:
   bool                           deathTestChild_;
   QString                        originalCwd_;
   QCoreApplication*              app_ = nullptr;
   std::unique_ptr<QTemporaryDir> sandbox_;
};

class UtilityCaseIsolation : public ::testing::EmptyTestEventListener
{
   public:
      explicit UtilityCaseIsolation( QtTestEnvironment& environment ) : environment_( environment )
      {
      }
      void OnTestStart( const ::testing::TestInfo& info ) override
      {
#ifdef US3_CHECK_UNIT_RESOURCES
         const auto resources = us3UnitResourcesIntact();
         if ( !resources )
         {
            ADD_FAILURE() << "Resource precondition for " << info.test_suite_name() << "." << info.name() << ": "
                          << resources.message();
            fprintf( stderr,
                     "Missing or modified declared unit resources for %s.%s: %s\n",
                     info.test_suite_name(),
                     info.name(),
                     resources.message() );
            fflush( stdout );
            if ( !environment_.removeSandbox() )
            {
               fprintf( stderr, "Could not remove failed test sandbox\n" );
            }
            std::exit( 2 ); // fail closed: listener assertions alone do not skip TestBody
         }
#endif
         scope_.reset( new Us3TestScope );
         // A broken sandbox must not let the test body access ambient paths.
         if ( !scope_->valid() )
         {
            fprintf( stderr, "Could not initialize utility test isolation for %s.%s\n", info.test_suite_name(), info.name() );
            scope_.reset();
            if ( !environment_.removeSandbox() )
            {
               fprintf( stderr, "Could not remove failed test sandbox\n" );
            }
            std::exit( 2 );
         }
      }
      void OnTestEnd( const ::testing::TestInfo& ) override
      {
#ifdef US3_CHECK_UNIT_RESOURCES
         EXPECT_TRUE( us3UnitResourcesIntact() ) << "Unit test changed a declared resource";
#endif
         EXPECT_TRUE( scope_->restore() ) << "Could not restore utility test isolation";
         scope_.reset();
      }

   private:
      QtTestEnvironment&            environment_;
      std::unique_ptr<Us3TestScope> scope_;
};

// Private subprocess protocol for the cross-process fixture self-test. The
// parent bounds the wait and kills the child on failure. No application IPC.
static int isolationProbe()
{
   Us3TestScope scope;
   if ( !scope.valid() )
   {
      return 2;
   }
   const QString    token = qEnvironmentVariable( "US3_PROBE_TOKEN" );
   const QString    other = token == "A" ? "B" : "A";
   US_SettingsStore settings;
   settings.setValue( "process-probe/value", token );
   settings.setValue( "process-probe/" + token, true );
   settings.sync();
   if ( settings.status() != QSettings::NoError || !us3WriteTestFile( scope.path() + "/sentinel", token.toUtf8() ) ||
        !us3WriteTestFile( scope.path() + "/" + token, token.toUtf8() ) )
   {
      return 3;
   }
   QJsonObject paths;
   paths.insert( "sandbox", qEnvironmentVariable( "US3_TEST_SANDBOX" ) );
   paths.insert( "settings", settings.fileName() );
   paths.insert( "work", scope.path() );
   const QByteArray report = QJsonDocument( paths ).toJson( QJsonDocument::Compact );
   fprintf( stdout, "US3_PROBE:%s\n", report.constData() );
   fflush( stdout );
   QTextStream input( stdin );
   if ( input.readLine() != "verify" )
   {
      return 4;
   }
   settings.sync();
   QFile sentinel( scope.path() + "/sentinel" );
   if ( settings.value( "process-probe/value" ).toString() != token || settings.contains( "process-probe/" + other ) ||
        QFile::exists( scope.path() + "/" + other ) || !sentinel.open( QIODevice::ReadOnly ) ||
        sentinel.readAll() != token.toUtf8() )
   {
      return 5;
   }
   sentinel.close();
   return scope.restore() ? 0 : 6;
}

int main(int argc, char **argv) {
   // GoogleTest uses this flag for threadsafe death-test re-execution. Inspect
   // it before InitGoogleTest removes it; keep this dependency regression-tested.
   bool deathTestChild = false;
   for ( int i = 1; i < argc; ++i )
   {
      deathTestChild = deathTestChild || QByteArray( argv[i] ).startsWith( "--gtest_internal_run_death_test=" );
   }
    ::testing::InitGoogleTest(&argc, argv);

    if ( argc == 2 && QByteArray( argv[1] ) == "--us3-isolation-probe" )
    {
       QtTestEnvironment environment;
       environment.SetUp();
       if ( !::testing::UnitTest::GetInstance()->Passed() )
       {
          return 2;
       }
       return isolationProbe();
    }

    // Add Qt environment
    auto* environment = new QtTestEnvironment( deathTestChild );
    ::testing::AddGlobalTestEnvironment( environment );

    ::testing::UnitTest::GetInstance()->listeners().Append( new UtilityCaseIsolation( *environment ) );

    // Run all tests - Google Test automatically discovers them!
    return RUN_ALL_TESTS();
}
