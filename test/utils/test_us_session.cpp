//! \file test_us_session.cpp
// US_Session's master-password checks, caching and sources, without a GUI or
// the user's shared memory: each case uses an in-memory store, and the test
// runner isolates the settings the stored hash lives in.
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_credential_source.h"
#include "us_db2.h"
#include "us_global.h"
#include "us_session.h"
#include "us_settings.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QTemporaryFile>
#include <utility>

#ifndef Q_OS_WIN
#include <atomic>
#include <chrono>
#include <csignal>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
#endif

namespace
{
// SHA-1 vectors computed independently with Python hashlib.
// sha1( b"synthetic-master" )
const QByteArray syntheticHash  = QByteArray::fromHex( "536fe61853c3bd2a7c6cee6621fc33b0b41bb7e8" );
// sha1( "café".encode( "latin-1" ) ): the hash covers Latin-1 bytes.
const QByteArray cafeLatin1Hash = QByteArray::fromHex( "d2f52bc4406898fc722c0b4e314f9b46fc85cde4" );

class MemoryStore : public US_Session::Store
{
   public:
      QString load() const override
      {
         return value;
      }
      void save( const QString& password ) override
      {
         value = password;
         saves++;
      }

      QString value;
      int     saves = 0;
};

// Replays scripted outcomes and candidates, and counts what the session asks.
class ScriptedSource : public US_CredentialSource
{
   public:
      struct Step
      {
            Outcome outcome;
            QString candidate;
      };

      explicit ScriptedSource( QList<Step> steps, int max_attempts = 0, bool is_available = true ) :
          steps( steps ),
          max_attempts( max_attempts ),
          is_available( is_available )
      {
      }

      bool available() const override
      {
         return is_available;
      }
      Outcome request( const QString&, int attempt, QString& secret ) override
      {
         attempts << attempt;
         if ( steps.isEmpty() )
         {
            return Outcome::Cancelled;
         }
         const Step step = steps.takeFirst();
         secret          = step.candidate;
         return step.outcome;
      }
      void rejected() override
      {
         rejections++;
      }
      int maxAttempts() const override
      {
         return max_attempts;
      }

      QList<Step> steps;
      int         max_attempts;
      bool        is_available;
      QList<int>  attempts;
      int         rejections = 0;
};

ScriptedSource::Step provided( const QString& candidate )
{
   return { US_CredentialSource::Outcome::Provided, candidate };
}
} // namespace

class US_SessionTest : public QtTestBase
{
   protected:
      void SetUp() override
      {
         store = QSharedPointer<MemoryStore>( new MemoryStore );
         US_Settings::set_UltraScanPW( syntheticHash );
      }

      US_Session session() const
      {
         return US_Session( store );
      }

      QSharedPointer<MemoryStore> store;
};

TEST_F( US_SessionTest, MatchesTheStoredSha1Hash )
{
   const US_Session s = session();
   EXPECT_TRUE( s.matches( "synthetic-master" ) );
   EXPECT_FALSE( s.matches( "synthetic-Master" ) );
   EXPECT_FALSE( s.matches( QString() ) );
}

TEST_F( US_SessionTest, HashesLatin1Bytes )
{
   US_Settings::set_UltraScanPW( cafeLatin1Hash );
   EXPECT_TRUE( session().matches( QString::fromUtf8( "caf\xc3\xa9" ) ) );
}

TEST_F( US_SessionTest, NothingMatchesWhenNoPasswordIsSet )
{
   US_Settings::set_UltraScanPW( {} );
   EXPECT_FALSE( US_Session::isPasswordSet() );
   EXPECT_FALSE( session().matches( QString() ) );
}

TEST_F( US_SessionTest, UnlockCachesTheAcceptedPassword )
{
   US_Session     s = session();
   ScriptedSource source( { provided( "synthetic-master" ) } );

   EXPECT_FALSE( s.isUnlocked() );
   EXPECT_EQ( s.unlock( source ), US_Session::Result::Ok );
   EXPECT_TRUE( s.isUnlocked() );
   EXPECT_EQ( s.masterKey(), QString( "synthetic-master" ) );
   EXPECT_EQ( store->value, QString( "synthetic-master" ) );
}

TEST_F( US_SessionTest, UnlockUsesTheCacheWithoutAsking )
{
   store->value     = "synthetic-master";
   US_Session     s = session();
   ScriptedSource source( { provided( "never-asked" ) } );

   EXPECT_EQ( s.unlock( source ), US_Session::Result::Ok );
   EXPECT_TRUE( source.attempts.isEmpty() );
}

TEST_F( US_SessionTest, UnlockRetriesAndReportsEachWrongPassword )
{
   US_Session     s = session();
   ScriptedSource source( { provided( "wrong-1" ), provided( "wrong-2" ), provided( "synthetic-master" ) } );

   EXPECT_EQ( s.unlock( source ), US_Session::Result::Ok );
   EXPECT_EQ( source.attempts, QList<int>( { 1, 2, 3 } ) );
   EXPECT_EQ( source.rejections, 2 );
   EXPECT_EQ( store->saves, 1 );
}

TEST_F( US_SessionTest, UnlockStopsAfterTheSourcesAttemptLimit )
{
   US_Session     s = session();
   ScriptedSource source( { provided( "wrong-1" ), provided( "wrong-2" ), provided( "synthetic-master" ) }, 2 );

   EXPECT_EQ( s.unlock( source ), US_Session::Result::Wrong );
   EXPECT_EQ( source.attempts.size(), 2 );
   EXPECT_FALSE( s.isUnlocked() );
   EXPECT_EQ( store->saves, 0 );
}

TEST_F( US_SessionTest, UnlockReportsCancelAndUnavailable )
{
   US_Session     s = session();
   ScriptedSource cancelled( { { US_CredentialSource::Outcome::Cancelled, QString() } } );
   ScriptedSource unavailable( { { US_CredentialSource::Outcome::Unavailable, QString() } } );

   EXPECT_EQ( s.unlock( cancelled ), US_Session::Result::Cancelled );
   EXPECT_EQ( s.unlock( unavailable ), US_Session::Result::Unavailable );
   EXPECT_FALSE( s.isUnlocked() );
}

TEST_F( US_SessionTest, UnlockReportsNotSetBeforeAsking )
{
   US_Settings::set_UltraScanPW( {} );
   US_Session     s = session();
   ScriptedSource source( { provided( "synthetic-master" ) } );

   EXPECT_EQ( s.unlock( source ), US_Session::Result::NotSet );
   EXPECT_TRUE( source.attempts.isEmpty() );
}

TEST_F( US_SessionTest, UnlockAsksTheFirstAvailableSource )
{
   US_Session     s = session();
   ScriptedSource absent( { provided( "never-asked" ) }, 0, false );
   ScriptedSource present( { provided( "synthetic-master" ) } );

   EXPECT_EQ( s.unlock( QList<US_CredentialSource*>( { &absent, &present } ) ), US_Session::Result::Ok );
   EXPECT_TRUE( absent.attempts.isEmpty() );
   EXPECT_EQ( present.attempts.size(), 1 );
}

TEST_F( US_SessionTest, UnlockWithNoAvailableSourceIsUnavailable )
{
   US_Session     s = session();
   ScriptedSource absent( { provided( "never-asked" ) }, 0, false );

   EXPECT_EQ( s.unlock( QList<US_CredentialSource*>( { &absent } ) ), US_Session::Result::Unavailable );
   EXPECT_EQ( s.unlock( QList<US_CredentialSource*>() ), US_Session::Result::Unavailable );
}

TEST_F( US_SessionTest, LockClearsTheCache )
{
   store->value = "synthetic-master";
   US_Session s = session();
   s.lock();
   EXPECT_FALSE( s.isUnlocked() );
   EXPECT_TRUE( s.masterKey().isEmpty() );
}

TEST_F( US_SessionTest, ConfirmChecksWithoutTouchingTheCache )
{
   US_Session s = session();

   EXPECT_EQ( s.confirm( "GMP e-Signing Form", "synthetic-master" ), US_Session::Result::Ok );
   EXPECT_FALSE( s.isUnlocked() );

   store->value = "synthetic-master";
   EXPECT_EQ( s.confirm( "GMP e-Signing Form", "wrong" ), US_Session::Result::Wrong );
   EXPECT_EQ( store->value, QString( "synthetic-master" ) );
   EXPECT_EQ( store->saves, 0 );
}

TEST_F( US_SessionTest, ConfirmReportsNotSet )
{
   US_Settings::set_UltraScanPW( {} );
   EXPECT_EQ( session().confirm( "GMP e-Signing Form", "anything" ), US_Session::Result::NotSet );
}

TEST_F( US_SessionTest, StoreMasterPasswordWritesHashFormatAndCache )
{
   US_Settings::set_UltraScanPW( {} );
   US_Session s = session();

   s.storeMasterPassword( "synthetic-master" );

   EXPECT_EQ( US_Settings::UltraScanPW(), syntheticHash );
   EXPECT_EQ( US_Settings::UltraScanPWFormat(), US_Session::CurrentHashFormat );
   EXPECT_EQ( store->value, QString( "synthetic-master" ) );
}

TEST_F( US_SessionTest, UnrecordedHashFormatIsTheLegacyFormat )
{
   EXPECT_EQ( US_Settings::UltraScanPWFormat(), 1 );
   US_Settings::set_UltraScanPWFormat( 7 );
   EXPECT_EQ( US_Settings::UltraScanPWFormat(), 7 );
}

TEST_F( US_SessionTest, PromptSourcesAreUsedWhenSet )
{
   US_Session s = session();
   EXPECT_FALSE( s.hasPromptSources() );
   EXPECT_EQ( s.unlockWithPromptSources(), US_Session::Result::Unavailable );

   QSharedPointer<ScriptedSource> source( new ScriptedSource( { provided( "synthetic-master" ) } ) );
   s.setPromptSources( { source } );

   EXPECT_TRUE( s.hasPromptSources() );
   EXPECT_EQ( s.unlockWithPromptSources(), US_Session::Result::Ok );
   EXPECT_EQ( source->attempts.size(), 1 );
}

TEST_F( US_SessionTest, EveryResultHasADescription )
{
   for ( US_Session::Result result : { US_Session::Result::Ok,
                                       US_Session::Result::NotSet,
                                       US_Session::Result::Wrong,
                                       US_Session::Result::Cancelled,
                                       US_Session::Result::Unavailable } )
   {
      EXPECT_FALSE( US_Session::describe( result ).isEmpty() );
   }
}

class US_CredentialSourceTest : public QtTestBase
{
   protected:
      void TearDown() override
      {
         qunsetenv( variable );
      }

      const char* variable = "US_CREDENTIAL_SOURCE_TEST_SECRET";
};

TEST_F( US_CredentialSourceTest, EnvironmentSourceReadsTheVariableOnce )
{
   qputenv( variable, QByteArray( "synthetic-master" ) );
   US_EnvironmentCredentialSource source( variable );
   QString                        secret;

   EXPECT_TRUE( source.available() );
   EXPECT_EQ( source.maxAttempts(), 1 );
   EXPECT_EQ( source.request( "Master password", 1, secret ), US_CredentialSource::Outcome::Provided );
   EXPECT_EQ( secret, QString( "synthetic-master" ) );
   EXPECT_FALSE( qEnvironmentVariableIsSet( variable ) ) << "child processes would inherit the password";
}

TEST_F( US_CredentialSourceTest, EnvironmentSourceIsUnavailableWhenUnsetOrEmpty )
{
   US_EnvironmentCredentialSource source( variable );
   QString                        secret;

   qunsetenv( variable );
   EXPECT_FALSE( source.available() );
   EXPECT_EQ( source.request( "Master password", 1, secret ), US_CredentialSource::Outcome::Unavailable );

   qputenv( variable, QByteArray() );
   EXPECT_FALSE( source.available() );
}

TEST_F( US_CredentialSourceTest, WrongEnvironmentPasswordIsNotRetried )
{
   US_Settings::set_UltraScanPW( syntheticHash );
   qputenv( variable, QByteArray( "wrong" ) );
   QSharedPointer<MemoryStore>    store( new MemoryStore );
   US_Session                     s( store );
   US_EnvironmentCredentialSource source( variable );

   EXPECT_EQ( s.unlock( source ), US_Session::Result::Wrong );
   EXPECT_FALSE( s.isUnlocked() );
}

TEST_F( US_CredentialSourceTest, TerminalSourceRefusesInputThatIsNotATerminal )
{
   QTemporaryFile file;
   ASSERT_TRUE( file.open() );
   US_TerminalCredentialSource source( file.handle() );
   QString                     secret;

   EXPECT_FALSE( source.available() );
   EXPECT_EQ( source.request( "Master password", 1, secret ), US_CredentialSource::Outcome::Unavailable );
   EXPECT_EQ( source.maxAttempts(), 3 );
}

namespace
{
// Hands out the connection it was given, once, and counts the calls.
class HandingProvider : public IUS_DBProvider
{
   public:
      explicit HandingProvider( IUS_DB2* db ) : db( db )
      {
      }

      std::unique_ptr<IUS_DB2> open() override
      {
         opened++;
         return std::unique_ptr<IUS_DB2>( std::exchange( db, nullptr ) );
      }

      IUS_DB2* db;
      int      opened = 0;
};
} // namespace

#ifndef NO_DB
TEST_F( US_SessionTest, WithoutAProviderTheSavedProfileIsUsed )
{
   // No profile is saved in the isolated settings, so this fails before any
   // network access, as a US_DB2 built with the master password does.
   store->value               = "synthetic-master";
   US_Session               s = session();

   std::unique_ptr<IUS_DB2> db = s.openDatabase();
   ASSERT_NE( db, nullptr );
   EXPECT_NE( db->lastErrno(), IUS_DB2::OK );
   EXPECT_TRUE( db->lastError().contains( "not configured" ) ) << db->lastError().toStdString();
}

TEST_F( US_SessionTest, AnInstalledProviderSuppliesTheConnection )
{
   US_Session                      s    = session();
   auto*                           mock = new US_DB2_Mock;
   QSharedPointer<HandingProvider> provider( new HandingProvider( mock ) );
   s.setProvider( provider );

   std::unique_ptr<IUS_DB2> db = s.openDatabase();
   EXPECT_EQ( db.get(), mock );
   EXPECT_EQ( provider->opened, 1 );

   s.setProvider( {} );
   EXPECT_NE( s.openDatabase().get(), mock );
}
#endif

// The shared cache field: what one process writes, another reads back.
TEST( US_GlobalPasswdField, NonAsciiPasswordsRoundTripAsLatin1 )
{
   char field[US_Global::PasswdBytes];
   US_Global::encodePasswd( QString::fromUtf8( "caf\xc3\xa9" ), field );
   EXPECT_EQ( static_cast<unsigned char>( field[3] ), 0xe9 );
   EXPECT_EQ( US_Global::decodePasswd( field ), QString::fromUtf8( "caf\xc3\xa9" ) );
}

TEST( US_GlobalPasswdField, ReadsNeverPassTheField )
{
   // 64 bytes fill the field with no terminator; the bytes after it must not
   // be read.
   struct
   {
         char field[US_Global::PasswdBytes];
         char after[8];
   } memory;
   memset( memory.after, 'X', sizeof memory.after );
   const QString exact( US_Global::PasswdBytes, QChar( 'a' ) );
   US_Global::encodePasswd( exact, memory.field );
   EXPECT_EQ( US_Global::decodePasswd( memory.field ), exact );

   US_Global::encodePasswd( exact + "bcdef", memory.field );
   EXPECT_EQ( US_Global::decodePasswd( memory.field ), exact );

   US_Global::encodePasswd( "short", memory.field );
   EXPECT_EQ( US_Global::decodePasswd( memory.field ), QString( "short" ) );
}

namespace
{
// Keeps only what the shared field can hold, as the shared-memory store does.
class FieldStore : public US_Session::Store
{
   public:
      QString load() const override
      {
         return US_Global::decodePasswd( field );
      }
      void save( const QString& password ) override
      {
         US_Global::encodePasswd( password, field );
      }

   private:
      char field[US_Global::PasswdBytes] = {};
};

} // namespace

TEST_F( US_SessionTest, TheAcceptedPasswordIsUsedAsEnteredNotAsCached )
{
   US_Settings::set_UltraScanPW( cafeLatin1Hash );
   US_Session     s( QSharedPointer<US_Session::Store>( new FieldStore ) );
   ScriptedSource source( { provided( QString::fromUtf8( "caf\xc3\xa9" ) ) } );

   ASSERT_EQ( s.unlock( source ), US_Session::Result::Ok );
   EXPECT_EQ( s.masterKey(), QString::fromUtf8( "caf\xc3\xa9" ) );
}

TEST_F( US_SessionTest, APasswordLongerThanTheCacheIsKeptWhole )
{
   const QString longPassword( 70, QChar( 'p' ) );
   US_Settings::set_UltraScanPW( QCryptographicHash::hash( longPassword.toLatin1(), QCryptographicHash::Sha1 ) );
   US_Session     s( QSharedPointer<US_Session::Store>( new FieldStore ) );
   ScriptedSource source( { provided( longPassword ) } );

   ASSERT_EQ( s.unlock( source ), US_Session::Result::Ok );
   EXPECT_EQ( s.masterKey(), longPassword );
   s.lock();
   EXPECT_FALSE( s.isUnlocked() );
}

TEST_F( US_SessionTest, InProcessUnlocksAreNotWrittenToTheSharedCache )
{
   US_Session s = session();
   s.keepUnlockInProcess();
   ScriptedSource source( { provided( "synthetic-master" ) } );

   ASSERT_EQ( s.unlock( source ), US_Session::Result::Ok );
   EXPECT_EQ( s.masterKey(), QString( "synthetic-master" ) );
   EXPECT_TRUE( store->value.isEmpty() );
   EXPECT_EQ( store->saves, 0 );
}

TEST_F( US_SessionTest, InProcessSessionsStillUseAnotherProcesssUnlock )
{
   store->value = "synthetic-master";
   US_Session s = session();
   s.keepUnlockInProcess();
   ScriptedSource source( { provided( "never-asked" ) } );

   EXPECT_TRUE( s.isUnlocked() );
   EXPECT_EQ( s.unlock( source ), US_Session::Result::Ok );
   EXPECT_TRUE( source.attempts.isEmpty() );
}

TEST_F( US_SessionTest, APasswordChangedByAnotherProcessReachesThisOne )
{
   QSharedPointer<FieldStore> field( new FieldStore );
   US_Session                 s( field );
   ScriptedSource             source( { provided( "synthetic-master" ) } );
   ASSERT_EQ( s.unlock( source ), US_Session::Result::Ok );

   // us_config in another process changes the password and caches the new one.
   US_Settings::set_UltraScanPW( QCryptographicHash::hash( QByteArray( "changed-master" ), QCryptographicHash::Sha1 ) );
   field->save( "changed-master" );

   EXPECT_EQ( s.masterKey(), QString( "changed-master" ) );
}

TEST_F( US_SessionTest, ALockByAnotherProcessReachesThisOne )
{
   QSharedPointer<FieldStore> field( new FieldStore );
   US_Session                 s( field );
   ScriptedSource             source( { provided( "synthetic-master" ) } );
   ASSERT_EQ( s.unlock( source ), US_Session::Result::Ok );

   field->save( QString() );

   EXPECT_FALSE( s.isUnlocked() );
}

TEST_F( US_SessionTest, ALockHoldsInAnInProcessSessionThatUsedAnotherProcesssUnlock )
{
   store->value = "synthetic-master";
   US_Session s = session();
   s.keepUnlockInProcess();
   ASSERT_TRUE( s.isUnlocked() );

   s.lock();

   EXPECT_FALSE( s.isUnlocked() );
   EXPECT_EQ( store->value, QString( "synthetic-master" ) ) << "the shared cache is not this process's to clear";
}

TEST_F( US_SessionTest, AnOverrideReplacesTheInstanceForItsLifetime )
{
   QSharedPointer<MemoryStore> outer_store( new MemoryStore );
   US_SessionOverride          outer( outer_store );
   EXPECT_EQ( &US_Session::instance(), &outer.session() );
   {
      US_SessionOverride inner( store );
      EXPECT_EQ( &US_Session::instance(), &inner.session() );
   }
   EXPECT_EQ( &US_Session::instance(), &outer.session() );
}

#ifndef Q_OS_WIN
namespace
{
// A private pseudoterminal: the prompt reads the slave, the test types on the
// master.
class PseudoTerminal
{
   public:
      PseudoTerminal()
      {
         master = posix_openpt( O_RDWR | O_NOCTTY );
         if ( master >= 0 && grantpt( master ) == 0 && unlockpt( master ) == 0 )
         {
            slave = open( ptsname( master ), O_RDWR | O_NOCTTY );
         }
      }
      ~PseudoTerminal()
      {
         if ( slave >= 0 )
         {
            close( slave );
         }
         if ( master >= 0 )
         {
            close( master );
         }
      }
      bool valid() const
      {
         return master >= 0 && slave >= 0;
      }

      bool echoing() const
      {
         termios mode{};
         return tcgetattr( slave, &mode ) == 0 && ( mode.c_lflag & ECHO ) != 0;
      }

      // Waits until the prompt has turned echo off.
      bool waitForEchoOff() const
      {
         for ( int ii = 0; ii < 500; ii++ )
         {
            if ( !echoing() )
            {
               return true;
            }
            std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
         }
         return false;
      }

      void type( const char* text ) const
      {
         ASSERT_GT( write( master, text, strlen( text ) ), 0 );
      }

      int master = -1;
      int slave  = -1;
};

// What the interrupted program's own SIGINT handler saw.
int                   interrupt_slave     = -1;
volatile sig_atomic_t interrupted         = 0;
volatile sig_atomic_t echo_when_interrupt = 0;
extern "C" void       recordInterrupt( int )
{
   termios mode{};
   echo_when_interrupt = tcgetattr( interrupt_slave, &mode ) == 0 && ( mode.c_lflag & ECHO ) != 0;
   interrupted         = 1;
}
} // namespace

TEST( US_TerminalCredentialSourcePty, ReadsWithEchoOffAndRestoresIt )
{
   PseudoTerminal tty;
   ASSERT_TRUE( tty.valid() );
   ASSERT_TRUE( tty.echoing() );

   US_TerminalCredentialSource source( tty.slave );
   ASSERT_TRUE( source.available() );
   QString                      secret;
   US_CredentialSource::Outcome outcome = US_CredentialSource::Outcome::Unavailable;
   std::thread                  reader( [&] { outcome = source.request( "Master password", 1, secret ); } );

   const bool                   off = tty.waitForEchoOff();
   tty.type( "synthetic-master\n" );
   reader.join();

   EXPECT_TRUE( off ) << "echo was never turned off";
   EXPECT_EQ( outcome, US_CredentialSource::Outcome::Provided );
   EXPECT_EQ( secret, QString( "synthetic-master" ) );
   EXPECT_TRUE( tty.echoing() ) << "echo was not restored";
}

TEST( US_TerminalCredentialSourcePty, AnInterruptRestoresEchoBeforeTheProgramsHandlerRuns )
{
   PseudoTerminal tty;
   ASSERT_TRUE( tty.valid() );
   interrupt_slave     = tty.slave;
   interrupted         = 0;
   echo_when_interrupt = 0;

   struct sigaction record
   {
   };
   struct sigaction saved
   {
   };
   record.sa_handler = recordInterrupt;
   sigemptyset( &record.sa_mask );
   sigaction( SIGINT, &record, &saved );

   US_TerminalCredentialSource  source( tty.slave );
   QString                      secret;
   US_CredentialSource::Outcome outcome = US_CredentialSource::Outcome::Unavailable;
   std::thread                  reader( [&] { outcome = source.request( "Master password", 1, secret ); } );

   ASSERT_TRUE( tty.waitForEchoOff() );
   pthread_kill( reader.native_handle(), SIGINT );
   reader.join();
   sigaction( SIGINT, &saved, nullptr );

   EXPECT_EQ( interrupted, 1 ) << "the program's own handler did not run";
   EXPECT_EQ( echo_when_interrupt, 1 ) << "echo was still off when the program's handler ran";
   EXPECT_EQ( outcome, US_CredentialSource::Outcome::Cancelled );
   EXPECT_TRUE( secret.isEmpty() );
   EXPECT_TRUE( tty.echoing() );
}
namespace
{
// Waits for a child to exit (or, with WUNTRACED, to stop), killing it after
// ten seconds so a broken prompt fails the test instead of hanging it.
bool waitForChild( pid_t child, int flags, int& status )
{
   for ( int ii = 0; ii < 1000; ii++ )
   {
      const pid_t done = waitpid( child, &status, flags | WNOHANG );
      if ( done == child )
      {
         return true;
      }
      std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
   }
   kill( child, SIGKILL );
   waitpid( child, &status, 0 );
   return false;
}

// In a forked child: start a session on the terminal and make it the
// controlling terminal, so job control applies as in a login shell.
int controllingTerminal( const char* name )
{
   setsid();
   const int fd = open( name, O_RDWR );
   if ( fd >= 0 )
   {
      ioctl( fd, TIOCSCTTY, 0 );
   }
   return fd;
}
} // namespace

TEST( US_TerminalCredentialSourcePty, ABackgroundJobDoesNotPrompt )
{
   PseudoTerminal tty;
   ASSERT_TRUE( tty.valid() );
   const std::string name = ptsname( tty.master );

   const pid_t       child = fork();
   ASSERT_GE( child, 0 );
   if ( child == 0 )
   {
      const int fd = controllingTerminal( name.c_str() );
      if ( fd < 0 || !US_TerminalCredentialSource( fd ).available() )
      {
         _exit( 3 ); // the foreground job could not prompt
      }
      const pid_t job = fork();
      if ( job == 0 )
      {
         setpgid( 0, 0 ); // a background job, as "command &" starts it
         _exit( US_TerminalCredentialSource( fd ).available() ? 1 : 0 );
      }
      int status = 0;
      waitpid( job, &status, 0 );
      _exit( WIFEXITED( status ) ? WEXITSTATUS( status ) : 4 );
   }

   int status = 0;
   ASSERT_TRUE( waitForChild( child, 0, status ) );
   ASSERT_TRUE( WIFEXITED( status ) );
   EXPECT_NE( WEXITSTATUS( status ), 3 ) << "the foreground job could not prompt";
   EXPECT_EQ( WEXITSTATUS( status ), 0 ) << "a background job would be stopped at the prompt";
}

namespace
{
bool echoOn( int fd )
{
   termios mode{};
   return tcgetattr( fd, &mode ) == 0 && ( mode.c_lflag & ECHO ) != 0;
}

// Polls for up to five seconds.
template <typename Check> bool within( Check check )
{
   for ( int ii = 0; ii < 500; ii++ )
   {
      if ( check() )
      {
         return true;
      }
      std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
   }
   return false;
}

// How the job-control test ended, as the shell process's exit code.
enum JobResult
{
   JobPassed = 0,
   NoTerminal,
   EchoNeverOff,
   DidNotStop,
   EchoOffWhileStopped,
   EchoNotOffAfterFg,
   PromptDidNotResume,
   EchoNotRestored
};
} // namespace

// Runs the prompt as a job under a minimal shell, so Ctrl+Z stops it as it
// would at a login prompt: a stop is only taken by a job whose shell is in
// the same session.
TEST( US_TerminalCredentialSourcePty, CtrlZGivesTheTerminalBackAndThePromptResumes )
{
   PseudoTerminal tty;
   ASSERT_TRUE( tty.valid() );
   const std::string name   = ptsname( tty.master );
   const int         master = tty.master;

   const pid_t       shell = fork();
   ASSERT_GE( shell, 0 );
   if ( shell == 0 )
   {
      signal( SIGTTOU, SIG_IGN ); // as a shell does, to hand the terminal over
      const int fd = controllingTerminal( name.c_str() );
      if ( fd < 0 )
      {
         _exit( NoTerminal );
      }
      const pid_t job = fork();
      if ( job == 0 )
      {
         signal( SIGTTOU, SIG_DFL );
         setpgid( 0, 0 );
         within( [&] { return tcgetpgrp( fd ) == getpgrp(); } );
         US_TerminalCredentialSource  source( fd );
         QString                      secret;
         US_CredentialSource::Outcome outcome = source.request( "Master password", 1, secret );
         _exit( outcome == US_CredentialSource::Outcome::Provided && secret == "typed-after-fg" ? 0 : 1 );
      }
      setpgid( job, job );
      tcsetpgrp( fd, job );

      auto finish = [&]( JobResult result )
      {
         kill( job, SIGKILL );
         waitpid( job, nullptr, 0 );
         _exit( result );
      };
      if ( !within( [&] { return !echoOn( fd ); } ) )
      {
         finish( EchoNeverOff );
      }
      if ( write( master, "half\x1a", 5 ) != 5 ) // Ctrl+Z; the half-typed line is discarded
      {
         finish( DidNotStop );
      }
      int  status  = 0;
      bool stopped = within( [&] { return waitpid( job, &status, WUNTRACED | WNOHANG ) == job; } );
      if ( !stopped || !WIFSTOPPED( status ) )
      {
         finish( DidNotStop );
      }
      tcsetpgrp( fd, getpgrp() ); // the shell takes the terminal back
      if ( !echoOn( fd ) )
      {
         finish( EchoOffWhileStopped );
      }

      tcsetpgrp( fd, job ); // fg
      kill( job, SIGCONT );
      if ( !within( [&] { return !echoOn( fd ); } ) )
      {
         finish( EchoNotOffAfterFg );
      }
      const char typed[] = "typed-after-fg\n";
      if ( write( master, typed, sizeof typed - 1 ) != static_cast<ssize_t>( sizeof typed - 1 ) ||
           !within( [&] { return waitpid( job, &status, WNOHANG ) == job; } ) || !WIFEXITED( status ) ||
           WEXITSTATUS( status ) != 0 )
      {
         finish( PromptDidNotResume );
      }
      _exit( echoOn( fd ) ? JobPassed : EchoNotRestored );
   }

   int status = 0;
   ASSERT_TRUE( waitForChild( shell, 0, status ) );
   ASSERT_TRUE( WIFEXITED( status ) );
   const char* const results[] = { "passed",
                                   "the shell could not open the terminal",
                                   "echo was never turned off",
                                   "Ctrl+Z did not stop the prompt",
                                   "the shell got the terminal back with echo off",
                                   "echo was not turned off again after fg",
                                   "the prompt did not resume after fg",
                                   "echo was not restored after the prompt" };
   const int         code      = WEXITSTATUS( status );
   EXPECT_EQ( code, JobPassed ) << ( code < 8 ? results[code] : "unexpected exit" );
}

// On Linux, Ctrl+D hands over a partly typed line and the next read waits,
// so an interrupt can arrive mid-line; BSD terminals report end of input
// there instead.
#ifdef Q_OS_LINUX
TEST( US_TerminalCredentialSourcePty, AnInterruptedPromptNeverUsesAPartlyTypedLine )
{
   PseudoTerminal tty;
   ASSERT_TRUE( tty.valid() );
   interrupt_slave = tty.slave;
   interrupted     = 0;

   struct sigaction record
   {
   };
   struct sigaction saved
   {
   };
   record.sa_handler = recordInterrupt;
   sigemptyset( &record.sa_mask );
   sigaction( SIGINT, &record, &saved );

   US_TerminalCredentialSource  source( tty.slave );
   QString                      secret;
   US_CredentialSource::Outcome outcome = US_CredentialSource::Outcome::Unavailable;
   std::thread                  reader( [&] { outcome = source.request( "Master password", 1, secret ); } );

   ASSERT_TRUE( tty.waitForEchoOff() );
   tty.type( "partial\x04" ); // Ctrl+D hands over the partial line without a newline
   std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
   pthread_kill( reader.native_handle(), SIGINT );
   reader.join();
   sigaction( SIGINT, &saved, nullptr );

   EXPECT_EQ( outcome, US_CredentialSource::Outcome::Cancelled );
   EXPECT_TRUE( secret.isEmpty() );
}
#endif

TEST( US_TerminalCredentialSourcePty, AnInterruptDeliveredToAnotherThreadEndsThePrompt )
{
   PseudoTerminal tty;
   ASSERT_TRUE( tty.valid() );
   interrupt_slave = tty.slave;
   interrupted     = 0;

   struct sigaction record
   {
   };
   struct sigaction saved
   {
   };
   record.sa_handler = recordInterrupt;
   sigemptyset( &record.sa_mask );
   sigaction( SIGINT, &record, &saved );

   US_TerminalCredentialSource  source( tty.slave );
   QString                      secret;
   US_CredentialSource::Outcome outcome = US_CredentialSource::Outcome::Unavailable;
   std::thread                  reader( [&] { outcome = source.request( "Master password", 1, secret ); } );

   ASSERT_TRUE( tty.waitForEchoOff() );
   pthread_kill( pthread_self(), SIGINT ); // not the thread that reads
   std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
   tty.type( "late\n" ); // ends a read the interrupt did not reach
   reader.join();
   sigaction( SIGINT, &saved, nullptr );

   EXPECT_EQ( interrupted, 1 ) << "the program's own handler did not run";
   EXPECT_EQ( outcome, US_CredentialSource::Outcome::Cancelled ) << "the prompt went on reading";
   EXPECT_TRUE( secret.isEmpty() );
   EXPECT_TRUE( tty.echoing() );
}
#endif
