//! \file test_us_session_db_connect.cpp
// The master password from US_Session opens the saved database profile:
// US_DB2 decrypts the stored account and person passwords with it, connects,
// and validate_user accepts the person. This is the path every desktop
// program and headless simulator takes, and the account may authenticate
// through PAM, which then receives the decrypted account password.
//
// It needs a server with the US3 schema and procedures and skips without one.
// Set
//   US3_TEST_DB_HOST             host or host:port
//   US3_TEST_DB_NAME             a database with the US3 procedures
//   US3_TEST_DB_USER             the account the profile connects as
//   US3_TEST_DB_PASSWORD         its password (for a PAM account, the Linux one)
//   US3_TEST_PERSON_GUID         a person in that database
//   US3_TEST_PERSON_PASSWORD     that person's US3 password
//
// US_DB2 always connects over TLS, with etc/mysql/ca-cert.pem beside the test
// executable's directory as its CA, so the server must have TLS on.

#include "qt_test_base.h"
#include "us_credential_source.h"
#include "us_crypto.h"
#include "us_db2.h"
#include "us_session.h"
#include "us_settings.h"

#include <QSharedPointer>
#include <QString>
#include <QStringList>

namespace
{
#ifndef NO_DB
class US_SessionDbConnectLive : public QtTestBase
{
   protected:
      void SetUp() override
      {
         const QString host            = qEnvironmentVariable( "US3_TEST_DB_HOST" );
         const QString name            = qEnvironmentVariable( "US3_TEST_DB_NAME" );
         const QString user            = qEnvironmentVariable( "US3_TEST_DB_USER" );
         const QString password        = qEnvironmentVariable( "US3_TEST_DB_PASSWORD" );
         const QString person_guid     = qEnvironmentVariable( "US3_TEST_PERSON_GUID" );
         const QString person_password = qEnvironmentVariable( "US3_TEST_PERSON_PASSWORD" );
         if ( host.isEmpty() || name.isEmpty() || user.isEmpty() || person_guid.isEmpty() )
         {
            GTEST_SKIP() << "US3_TEST_DB_HOST, US3_TEST_DB_NAME, US3_TEST_DB_USER and "
                            "US3_TEST_PERSON_GUID not set";
         }

         // The profile as us_config saves it: both passwords encrypted with
         // the master password.
         const QStringList account = US_Crypto::encrypt( password, master );
         const QStringList person  = US_Crypto::encrypt( person_password, master );
         ASSERT_EQ( account.size(), 2 );
         ASSERT_EQ( person.size(), 2 );
         US_Settings::set_defaultDB( { "Live session test",
                                       user,
                                       name,
                                       host,
                                       account.at( 0 ),
                                       account.at( 1 ),
                                       "",
                                       person.at( 0 ),
                                       person.at( 1 ),
                                       person_guid } );

         US_Session( QSharedPointer<US_Session::Store>( new US_Session::MemoryStore ) ).storeMasterPassword( master );
         qputenv( variable, master.toUtf8() );
      }

      void TearDown() override
      {
         qunsetenv( variable );
      }

      const QString master   = "synthetic-master";
      const char*   variable = "US_SESSION_DB_TEST_MASTER";
};

TEST_F( US_SessionDbConnectLive, UnlockedMasterPasswordOpensTheProfile )
{
   US_Session                     session( QSharedPointer<US_Session::Store>( new US_Session::MemoryStore ) );
   US_EnvironmentCredentialSource source( variable );
   ASSERT_EQ( session.unlock( source ), US_Session::Result::Ok );

   // The production path: the session's saved-profile provider.
   std::unique_ptr<IUS_DB2> db = session.openDatabase();
   ASSERT_EQ( db->lastErrno(), US_DB2::OK ) << db->lastError().toStdString();

   db->rawQuery( "SELECT @US3_ID" );
   ASSERT_TRUE( db->next() );
   EXPECT_GT( db->value( 0 ).toInt(), 0 );
}

TEST_F( US_SessionDbConnectLive, WrongMasterPasswordNeitherUnlocksNorConnects )
{
   qputenv( variable, "wrong" );
   US_Session                     session( QSharedPointer<US_Session::Store>( new US_Session::MemoryStore ) );
   US_EnvironmentCredentialSource source( variable );
   EXPECT_EQ( session.unlock( source ), US_Session::Result::Wrong );

   // Still locked, so the connection is refused without contacting the
   // server.
   std::unique_ptr<IUS_DB2> db = session.openDatabase();
   EXPECT_NE( db->lastErrno(), US_DB2::OK );
   EXPECT_TRUE( db->lastError().contains( "master password was not entered" ) ) << db->lastError().toStdString();

   // A key that is not the master password does not decrypt the stored ones.
   US_DB2 wrong( "wrong" );
   EXPECT_NE( wrong.lastErrno(), US_DB2::OK );
}
#endif
} // namespace
