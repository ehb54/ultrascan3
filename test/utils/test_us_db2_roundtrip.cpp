//! \file test_us_db2_roundtrip.cpp
// Values composed by US_DB2::composeQuery() reach the server unchanged.
//
// Each case sends a value, plus a guid and password that need escaping too,
// to a function that hands its three arguments straight back.  If the server
// returns what was sent, the escaping matches how it reads a literal.
//
// They need a MariaDB or MySQL server and skip without one.  Set
//   US3_TEST_DB_HOST      host or host:port
//   US3_TEST_DB_NAME      a scratch database; the test creates its own function
//   US3_TEST_DB_USER      needs CREATE ROUTINE on that database
//   US3_TEST_DB_PASSWORD
//
// US_DB2 always connects over TLS with the client certificates in etc/mysql,
// so the server must trust etc/mysql/ca-cert.pem.  Without that the connect
// fails and the test fails rather than skip.

#include <gtest/gtest.h>

#include "us_db2.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <QStringList>

namespace
{
#ifndef NO_DB
const char* const echoFunction = "us3_test_echo";

class US_DB2_LiveRoundTrip : public ::testing::Test
{
   protected:
      void SetUp() override
      {
         const QString host     = qEnvironmentVariable( "US3_TEST_DB_HOST" );
         const QString name     = qEnvironmentVariable( "US3_TEST_DB_NAME" );
         const QString user     = qEnvironmentVariable( "US3_TEST_DB_USER" );
         const QString password = qEnvironmentVariable( "US3_TEST_DB_PASSWORD" );
         if ( host.isEmpty() || name.isEmpty() || user.isEmpty() )
         {
            GTEST_SKIP() << "US3_TEST_DB_HOST, US3_TEST_DB_NAME and US3_TEST_DB_USER not set";
         }

         QString error;
         ASSERT_TRUE( db.connect( host, name, user, password, error ) ) << error.toStdString();

         db.rawQuery( QString( "DROP FUNCTION IF EXISTS %1" ).arg( echoFunction ) );
         db.rawQuery( QString( "CREATE FUNCTION %1( g TEXT, p TEXT, v LONGTEXT ) RETURNS LONGTEXT "
                               "RETURN JSON_ARRAY( g, p, v )" )
                         .arg( echoFunction ) );
      }

      void TearDown() override
      {
         if ( db.isConnected() )
         {
            db.rawQuery( QString( "DROP FUNCTION IF EXISTS %1" ).arg( echoFunction ) );
         }
      }

      //! The guid, password and value as the server received them.
      QStringList echoed( const QString& guid, const QString& password, const QString& value )
      {
         db.rawQuery( US_DB2::composeQuery( "SELECT", QStringList() << echoFunction << value, guid, password ) );
         if ( !db.next() )
         {
            ADD_FAILURE() << db.lastError().toStdString();
            return QStringList();
         }

         QStringList received;
         for ( const QJsonValue& item : QJsonDocument::fromJson( db.value( 0 ).toString().toUtf8() ).array() )
         {
            received << item.toString();
         }
         return received;
      }

      US_DB2 db;
};

TEST_F( US_DB2_LiveRoundTrip, EscapedValuesArriveAsSent )
{
   const QString     guid     = "guid'\\";
   const QString     password = "pa\\ss'word\\";

   const QStringList values = {
      "O'Brien",
      "C:\\data\\new\\run1.auc",
      "a\\'b",
      "trailing\\",
      "line1\nline2\ttab\r",
      "100% _under_score",
      QString( "nul" ) + QChar( '\0' ) + "inside",
      "<model desc='x' path=\"C:\\runs\\demo\"><v n='-0.2'/></model>",
   };

   for ( const QString& value : values )
   {
      EXPECT_EQ( echoed( guid, password, value ), QStringList() << guid << password << value ) << "value: " << value.toStdString();
   }
}
#endif
} // namespace
