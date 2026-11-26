//! \file test_us_session.cpp
// US_Session's master-password checks, caching and sources, without a GUI or
// the user's shared memory: each case uses an in-memory store, and the test
// runner isolates the settings the stored hash lives in.
#include "qt_test_base.h"
#include "us_credential_source.h"
#include "us_session.h"
#include "us_settings.h"

#include <QByteArray>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QTemporaryFile>

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
