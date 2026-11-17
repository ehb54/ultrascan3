//! \file test_us_buffer_component_live.cpp
// US_BufferComponent::getAllFromDB against live servers. It opens its own
// connection from the default database in the settings and the master
// password, so a mocked IUS_DB2 cannot reach it.
//
// The seed cases read the seeded uslims3_test database and change nothing.
// Set US3_TEST_LIVE_DB=1; the local dev-stack defaults can be overridden with
// US3_TEST_SEED_DB_HOST/USER/PASS and US3_TEST_PERSON_GUID/PASSWORD.
//
// The failure case installs stand-in procedures in a scratch database without
// the US3 procedures, and skips when they already exist. Set
//   US3_TEST_DB_HOST      host or host:port
//   US3_TEST_DB_NAME      a scratch database
//   US3_TEST_DB_USER      needs CREATE, DROP and CREATE ROUTINE on that database
//   US3_TEST_DB_PASSWORD
// Its checks are one test case because they share those procedures, and ctest
// runs test cases in parallel.
//
// US_DB2 always connects over TLS with the client certificates in etc/mysql,
// so both servers must trust etc/mysql/ca-cert.pem.

#include <gtest/gtest.h>

#include "us_buffer.h"
#include "us_crypto.h"
#include "us_db2.h"
#include "us_settings.h"

#include <QMap>
#include <QString>
#include <QStringList>

namespace
{
#ifndef NO_DB
const QString kMasterPassword = "master password";

// Make the given server the default database, with both passwords encrypted
// under the master password, as the database configuration dialog stores it.
void          setDefaultDatabase( const QString& host,
                                  const QString& name,
                                  const QString& user,
                                  const QString& password,
                                  const QString& personGuid,
                                  const QString& personPassword,
                                  const QString& email )
{
   const QStringList dbPassword   = US_Crypto::encrypt( password, kMasterPassword );
   const QStringList personCipher = US_Crypto::encrypt( personPassword, kMasterPassword );
   const QStringList entry{ "live test",     user,      name, host, dbPassword[0], dbPassword[1], email, personCipher[0],
                            personCipher[1], personGuid };
   US_Settings::set_databases( { entry } );
   US_Settings::set_defaultDB( entry );
}

US_BufferComponent sentinel()
{
   US_BufferComponent c;
   c.componentID = "sentinel";
   c.name        = "previous component";
   return c;
}

class BufferComponentSeedDb : public ::testing::Test
{
   protected:
      void SetUp() override
      {
         if ( qEnvironmentVariableIntValue( "US3_TEST_LIVE_DB" ) != 1 )
         {
            GTEST_SKIP() << "US3_TEST_LIVE_DB not set";
         }
         host           = qEnvironmentVariable( "US3_TEST_SEED_DB_HOST", "127.0.0.1:3307" );
         user           = qEnvironmentVariable( "US3_TEST_SEED_DB_USER", "us3php" );
         password       = qEnvironmentVariable( "US3_TEST_SEED_DB_PASS", "us3phppass" );
         personGuid     = qEnvironmentVariable( "US3_TEST_PERSON_GUID", "00000000-0000-0000-0000-000000000001" );
         personPassword = qEnvironmentVariable( "US3_TEST_PERSON_PASSWORD", "password" );

         QString error;
         ASSERT_TRUE( admin.connectAuthenticated( host, kDatabase, user, password, personGuid, personPassword, error ) )
            << error.toStdString();
         admin.rawQuery( "SELECT email FROM people WHERE personGUID = '" + personGuid + "'" );
         ASSERT_TRUE( admin.next() );
         setDefaultDatabase( host, kDatabase, user, password, personGuid, personPassword, admin.value( 0 ).toString() );
      }

      const QString kDatabase = "uslims3_test";
      QString       host;
      QString       user;
      QString       password;
      QString       personGuid;
      QString       personPassword;
      US_DB2        admin;
};

TEST_F( BufferComponentSeedDb, ReadsEverySeededComponent )
{
   admin.rawQuery( "SELECT COUNT(*) FROM bufferComponent" );
   ASSERT_TRUE( admin.next() );
   const int stored = admin.value( 0 ).toInt();
   ASSERT_GT( stored, 0 );
   admin.rawQuery( "SELECT bufferComponentID, description, density FROM bufferComponent "
                   "ORDER BY bufferComponentID LIMIT 1" );
   ASSERT_TRUE( admin.next() );
   const QString id      = admin.value( 0 ).toString();
   const QString name    = admin.value( 1 ).toString();
   const double  density = admin.value( 2 ).toString().section( ' ', 0, 0, QString::SectionSkipEmpty ).toDouble();

   QMap<QString, US_BufferComponent> components;
   QString                           error;
   ASSERT_TRUE( US_BufferComponent::getAllFromDB( kMasterPassword, components, &error ) ) << error.toStdString();
   EXPECT_EQ( stored, components.size() );
   ASSERT_TRUE( components.contains( id ) );
   EXPECT_EQ( name, components[id].name );
   EXPECT_DOUBLE_EQ( density, components[id].dens_coeff[0] );
}

TEST_F( BufferComponentSeedDb, AConnectionThatCannotOpenFailsAndKeepsTheList )
{
   QMap<QString, US_BufferComponent> components{ { "sentinel", sentinel() } };
   QString                           error;
   // Another master password cannot decrypt the stored DB password.
   EXPECT_FALSE( US_BufferComponent::getAllFromDB( "another master password", components, &error ) );
   EXPECT_FALSE( error.isEmpty() );
   ASSERT_EQ( 1, components.size() );
   EXPECT_EQ( QString( "previous component" ), components["sentinel"].name );
}

class BufferComponentScratchDb : public ::testing::Test
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
                         "AND ROUTINE_NAME IN ( 'validate_user', 'get_buffer_component_desc', "
                         "'get_buffer_component_info' )" );
         ASSERT_TRUE( admin.next() );
         if ( admin.value( 0 ).toInt() != 0 )
         {
            GTEST_SKIP() << "US3 procedures already exist in " << name.toStdString();
         }

         // The procedures return a status set, then their data, as the US3 procedures do.
         create( "CREATE TABLE bc_fixture ( id INT PRIMARY KEY, description TEXT, units TEXT, "
                 "viscosity TEXT, density TEXT )" );
         create( "CREATE PROCEDURE validate_user( g TEXT, e TEXT, p TEXT ) "
                 "BEGIN "
                 "  IF p = 'right' THEN "
                 "    SET @US3_ID = 42; "
                 "    SELECT 0 AS status; "
                 "    SELECT 42 AS personID, 'tester@example.com' AS email; "
                 "  ELSE "
                 "    SELECT 401 AS status; "
                 "  END IF; "
                 "END" );
         create( "CREATE PROCEDURE get_buffer_component_desc( g TEXT, p TEXT ) "
                 "BEGIN "
                 "  SELECT 0 AS status; "
                 "  SELECT id, description FROM bc_fixture ORDER BY id; "
                 "END" );
         create( "CREATE PROCEDURE get_buffer_component_info( g TEXT, p TEXT, cid INT ) "
                 "BEGIN "
                 "  SELECT 0 AS status; "
                 "  SELECT units, description, viscosity, density, '' AS c_range, 0 AS gradientForming "
                 "  FROM bc_fixture WHERE id = cid; "
                 "END" );
         setDefaultDatabase( host, name, user, password, "guid-1", "right", "tester@example.com" );
      }

      void TearDown() override
      {
         if ( created )
         {
            admin.rawQuery( "DROP PROCEDURE IF EXISTS get_buffer_component_info" );
            admin.rawQuery( "DROP PROCEDURE IF EXISTS get_buffer_component_desc" );
            admin.rawQuery( "DROP PROCEDURE IF EXISTS validate_user" );
            admin.rawQuery( "DROP TABLE IF EXISTS bc_fixture" );
         }
      }

      void create( const QString& statement )
      {
         created = true;
         admin.rawQuery( statement );
         ASSERT_TRUE( admin.lastError().isEmpty() ) << admin.lastError().toStdString();
      }

      QString host;
      QString name;
      QString user;
      QString password;
      US_DB2  admin;
      bool    created = false;
};

TEST_F( BufferComponentScratchDb, UnreadableComponentsAndAFailedListAreReported )
{
   const QString coefficients = "1 2 3 4 5 6";
   admin.rawQuery( "INSERT INTO bc_fixture VALUES ( 1, 'Readable', 'mM', '" + coefficients + "', '" + coefficients +
                   "' ), ( 2, 'Unreadable', 'mM', '1 2 x 4 5 6', '" + coefficients + "' )" );
   ASSERT_TRUE( admin.lastError().isEmpty() ) << admin.lastError().toStdString();
   {
      SCOPED_TRACE( "a component that cannot be read is left out" );
      QMap<QString, US_BufferComponent> components;
      QString                           error;
      EXPECT_TRUE( US_BufferComponent::getAllFromDB( kMasterPassword, components, &error ) ) << error.toStdString();
      ASSERT_EQ( 1, components.size() );
      EXPECT_EQ( QString( "Readable" ), components["1"].name );
   }

   admin.rawQuery( "DELETE FROM bc_fixture WHERE id = 1" );
   ASSERT_TRUE( admin.lastError().isEmpty() ) << admin.lastError().toStdString();
   {
      SCOPED_TRACE( "no component can be read" );
      QMap<QString, US_BufferComponent> components{ { "sentinel", sentinel() } };
      QString                           error;
      EXPECT_FALSE( US_BufferComponent::getAllFromDB( kMasterPassword, components, &error ) );
      EXPECT_EQ( QString( "none of the 1 buffer components could be read" ), error );
      ASSERT_EQ( 1, components.size() );
      EXPECT_TRUE( components.contains( "sentinel" ) );
   }

   admin.rawQuery( "DROP PROCEDURE get_buffer_component_desc" );
   ASSERT_TRUE( admin.lastError().isEmpty() ) << admin.lastError().toStdString();
   {
      SCOPED_TRACE( "the component list cannot be queried" );
      QMap<QString, US_BufferComponent> components{ { "sentinel", sentinel() } };
      QString                           error;
      EXPECT_FALSE( US_BufferComponent::getAllFromDB( kMasterPassword, components, &error ) );
      EXPECT_TRUE( error.contains( "get_buffer_component_desc" ) ) << error.toStdString();
      ASSERT_EQ( 1, components.size() );
      EXPECT_TRUE( components.contains( "sentinel" ) );
   }
}
#endif
} // namespace
