//! \file test_us_db2_connect_authenticated.cpp
// US_DB2::connectAuthenticated() against a live server.
//
// The test installs a stand-in validate_user that accepts one password, so it
// needs a scratch database without the US3 procedures: when validate_user
// already exists the test skips rather than replace it.  Both checks are one
// test case because they share that procedure, and ctest runs test cases in
// parallel.
//
// It needs a MariaDB or MySQL server and skips without one.  Set
//   US3_TEST_DB_HOST      host or host:port
//   US3_TEST_DB_NAME      a scratch database
//   US3_TEST_DB_USER      needs CREATE ROUTINE on that database
//   US3_TEST_DB_PASSWORD
//
// US_DB2 always connects over TLS with the client certificates in etc/mysql,
// so the server must trust etc/mysql/ca-cert.pem.

#include <gtest/gtest.h>

#include "us_db2.h"

#include <QString>

namespace
{
#ifndef NO_DB
class US_DB2_LiveConnectAuthenticated : public ::testing::Test
{
   protected:
      void SetUp() override
      {
         host     = qEnvironmentVariable( "US3_TEST_DB_HOST" );
         name     = qEnvironmentVariable( "US3_TEST_DB_NAME" );
         user     = qEnvironmentVariable( "US3_TEST_DB_USER" );
         password = qEnvironmentVariable( "US3_TEST_DB_PASSWORD" );
         if ( host.isEmpty() || name.isEmpty() || user.isEmpty() )
         {
            GTEST_SKIP() << "US3_TEST_DB_HOST, US3_TEST_DB_NAME and US3_TEST_DB_USER not set";
         }

         QString error;
         ASSERT_TRUE( admin.connect( host, name, user, password, error ) ) << error.toStdString();

         admin.rawQuery( "SELECT COUNT(*) FROM information_schema.ROUTINES WHERE ROUTINE_SCHEMA = DATABASE() "
                         "AND ROUTINE_NAME = 'validate_user'" );
         ASSERT_TRUE( admin.next() );
         if ( admin.value( 0 ).toInt() != 0 )
         {
            GTEST_SKIP() << "validate_user already exists in " << name.toStdString();
         }

         // validate_user( guid, email, password ): a status row, then the
         // person's ID and email, as the US3 procedure returns them.
         admin.rawQuery( "CREATE PROCEDURE validate_user( g TEXT, e TEXT, p TEXT ) "
                         "BEGIN "
                         "  IF p = 'right' THEN "
                         "    SET @US3_ID = 42; "
                         "    SELECT 0 AS status; "
                         "    SELECT 42 AS personID, 'tester@example.com' AS email; "
                         "  ELSE "
                         "    SELECT 401 AS status; "
                         "  END IF; "
                         "END" );
         ASSERT_TRUE( admin.lastError().isEmpty() ) << admin.lastError().toStdString();
         createdProcedure = true;
      }

      void TearDown() override
      {
         if ( createdProcedure )
         {
            admin.rawQuery( "DROP PROCEDURE IF EXISTS validate_user" );
         }
      }

      QString host;
      QString name;
      QString user;
      QString password;
      US_DB2  admin;
      bool    createdProcedure = false;
};

TEST_F( US_DB2_LiveConnectAuthenticated, ConnectsOnlyWhenValidateUserAcceptsTheCredentials )
{
   US_DB2  accepted;
   QString error;
   ASSERT_TRUE( accepted.connectAuthenticated( host, name, user, password, "guid-1", "right", error ) ) << error.toStdString();

   accepted.rawQuery( "SELECT @US3_ID" );
   ASSERT_TRUE( accepted.next() );
   EXPECT_EQ( accepted.value( 0 ).toInt(), 42 );

   US_DB2 refused;
   error.clear();
   EXPECT_FALSE( refused.connectAuthenticated( host, name, user, password, "guid-1", "wrong", error ) );
   EXPECT_FALSE( error.isEmpty() );
}
#endif
} // namespace
