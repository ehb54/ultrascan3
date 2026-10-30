//! \file test_us_db2_transactions.cpp
// US_DB2 transaction outcomes that only a live server can produce.
//
// InnoDB answers a deadlock by rolling back the victim's whole transaction,
// which ends it just as an implicit commit does.  These tests force a real
// deadlock between two connections and check that the victim's commit and
// rollback name the deadlock instead of claiming its rows were kept, and that
// a transaction ended without any error to the client is never committed.
//
// They need a MariaDB or MySQL server and skip without one.  Set
//   US3_TEST_DB_HOST      host or host:port
//   US3_TEST_DB_NAME      a scratch database; the tests create their own
//                         table and procedures
//   US3_TEST_DB_USER      needs CREATE ROUTINE on that database, and PROCESS
//                         to read INNODB_TRX while setting up the deadlock
//   US3_TEST_DB_PASSWORD
//
// US_DB2 always connects over TLS with the client certificates in etc/mysql,
// so the server must trust etc/mysql/ca-cert.pem.  Without that the connect
// fails and the tests fail rather than skip.

#include <gtest/gtest.h>

#include "us_db2.h"

#include <QString>

#include <chrono>
#include <memory>
#include <thread>

namespace
{
#ifndef NO_DB
const char* const table = "us3_test_txn";

// Rolls back the caller's transaction without returning an error, as a
// procedure that catches a deadlock in a handler leaves it.
const char* const rollbackProcedure = "us3_test_txn_rollback";

// Returns a status row, then fails producing its data set.
const char* const failingProcedure = "us3_test_txn_fail";

struct LiveDb
{
      QString host;
      QString name;
      QString user;
      QString password;

      bool    configured() const
      {
         return !host.isEmpty() && !name.isEmpty() && !user.isEmpty();
      }
};

LiveDb liveDb()
{
   return { qEnvironmentVariable( "US3_TEST_DB_HOST" ),
            qEnvironmentVariable( "US3_TEST_DB_NAME" ),
            qEnvironmentVariable( "US3_TEST_DB_USER" ),
            qEnvironmentVariable( "US3_TEST_DB_PASSWORD" ) };
}

// Runs a statement and returns the first column of its first row.
QString scalar( US_DB2& db, const QString& sql )
{
   db.rawQuery( sql );
   return db.next() ? db.value( 0 ).toString() : QString();
}

class US_DB2_LiveTransaction : public ::testing::Test
{
   protected:
      void SetUp() override
      {
         const LiveDb live = liveDb();
         if ( !live.configured() )
         {
            GTEST_SKIP() << "US3_TEST_DB_HOST, US3_TEST_DB_NAME and US3_TEST_DB_USER not set";
         }

         first  = std::make_unique<US_DB2>();
         second = std::make_unique<US_DB2>();

         QString error;
         ASSERT_TRUE( first->connect( live.host, live.name, live.user, live.password, error ) ) << error.toStdString();
         ASSERT_TRUE( second->connect( live.host, live.name, live.user, live.password, error ) ) << error.toStdString();

         first->rawQuery( QString( "CREATE TABLE IF NOT EXISTS %1 ( id INT PRIMARY KEY, v INT ) ENGINE=InnoDB" ).arg( table ) );
         first->rawQuery( QString( "DELETE FROM %1" ).arg( table ) );
         first->rawQuery( QString( "INSERT INTO %1 VALUES ( 1, 0 ), ( 2, 0 )" ).arg( table ) );
         ASSERT_EQ( scalar( *first, QString( "SELECT COUNT(*) FROM %1" ).arg( table ) ), "2" );

         first->rawQuery( QString( "DROP PROCEDURE IF EXISTS %1" ).arg( rollbackProcedure ) );
         first->rawQuery( QString( "CREATE PROCEDURE %1() BEGIN ROLLBACK; END" ).arg( rollbackProcedure ) );
         first->rawQuery( QString( "DROP PROCEDURE IF EXISTS %1" ).arg( failingProcedure ) );
         first->rawQuery(
            QString( "CREATE PROCEDURE %1() BEGIN SELECT 0; SELECT * FROM us3_test_no_such_table; END" ).arg( failingProcedure ) );
         ASSERT_EQ( scalar( *first,
                            QString( "SELECT COUNT(*) FROM information_schema.ROUTINES WHERE ROUTINE_SCHEMA = DATABASE() "
                                     "AND ROUTINE_NAME IN ( '%1', '%2' )" )
                               .arg( rollbackProcedure, failingProcedure ) ),
                    "2" )
            << "the test user needs CREATE ROUTINE";
      }

      void TearDown() override
      {
         if ( first )
         {
            first->rawQuery( QString( "DROP TABLE IF EXISTS %1" ).arg( table ) );
            first->rawQuery( QString( "DROP PROCEDURE IF EXISTS %1" ).arg( rollbackProcedure ) );
            first->rawQuery( QString( "DROP PROCEDURE IF EXISTS %1" ).arg( failingProcedure ) );
         }
      }

      // Each connection locks one row, then asks for the other's.  The server
      // breaks the cycle by rolling one of them back; that one is returned.
      US_DB2* deadlock()
      {
         QString error;
         EXPECT_TRUE( first->beginTransaction( error ) ) << error.toStdString();
         EXPECT_TRUE( second->beginTransaction( error ) ) << error.toStdString();

         first->rawQuery( QString( "UPDATE %1 SET v = 1 WHERE id = 1" ).arg( table ) );
         second->rawQuery( QString( "UPDATE %1 SET v = 2 WHERE id = 2" ).arg( table ) );

         // This one blocks on the second connection's lock, so it runs aside.
         std::thread blocked( [this] { first->rawQuery( QString( "UPDATE %1 SET v = 1 WHERE id = 2" ).arg( table ) ); } );

         // Wait until the server shows it waiting, so the next update closes the
         // cycle rather than racing it.
         for ( int tries = 0; tries < 100; ++tries )
         {
            if ( scalar( *second, "SELECT COUNT(*) FROM information_schema.INNODB_TRX WHERE trx_state = 'LOCK WAIT'" ) != "0" )
            {
               break;
            }
            std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
         }

         second->rawQuery( QString( "UPDATE %1 SET v = 2 WHERE id = 1" ).arg( table ) );
         blocked.join();

         const bool first_lost  = first->lastError().contains( "Deadlock" );
         const bool second_lost = second->lastError().contains( "Deadlock" );
         EXPECT_NE( first_lost, second_lost ) << "exactly one connection should be the deadlock victim";

         return first_lost ? first.get() : second.get();
      }

      US_DB2* survivor( US_DB2* victim )
      {
         return victim == first.get() ? second.get() : first.get();
      }

      std::unique_ptr<US_DB2> first;
      std::unique_ptr<US_DB2> second;
};

// The error handling every caller does after a failed query -- statusQuery()
// asks for last_error() -- refreshes the server status, which no longer shows
// a transaction.  The rollback must not read that as an implicit commit.
TEST_F( US_DB2_LiveTransaction, RollbackAfterDeadlockNamesTheDeadlock )
{
   US_DB2* victim = deadlock();
   scalar( *victim, "SELECT 1" );

   QString error;
   EXPECT_FALSE( victim->rollbackTransaction( error ) );
   EXPECT_TRUE( error.contains( "deadlock" ) ) << error.toStdString();
   EXPECT_FALSE( error.contains( "may have been kept" ) ) << error.toStdString();

   EXPECT_TRUE( survivor( victim )->commitTransaction( error ) ) << error.toStdString();
}

// With nothing run after the deadlock, the status still predates it, and a
// COMMIT would succeed as a no-op.  The commit must be refused all the same.
TEST_F( US_DB2_LiveTransaction, CommitAfterDeadlockIsRefused )
{
   US_DB2* victim = deadlock();

   QString error;
   EXPECT_FALSE( victim->commitTransaction( error ) );
   EXPECT_TRUE( error.contains( "deadlock" ) ) << error.toStdString();

   // The caller rolls back after a failed commit, as the contract asks.
   EXPECT_FALSE( victim->rollbackTransaction( error ) );
   EXPECT_TRUE( error.contains( "deadlock" ) ) << error.toStdString();

   US_DB2* winner = survivor( victim );
   EXPECT_TRUE( winner->commitTransaction( error ) ) << error.toStdString();

   // Only the survivor's writes are in the table.
   const QString expected = winner == first.get() ? "1" : "2";
   EXPECT_EQ( scalar( *winner, QString( "SELECT v FROM %1 WHERE id = 1" ).arg( table ) ), expected );
   EXPECT_EQ( scalar( *winner, QString( "SELECT v FROM %1 WHERE id = 2" ).arg( table ) ), expected );
}

// A transaction ended by a statement that commits implicitly, with no error
// to say so, is reported without guessing which way it ended.
TEST_F( US_DB2_LiveTransaction, RollbackAfterImplicitCommitDoesNotClaimACause )
{
   QString error;
   ASSERT_TRUE( first->beginTransaction( error ) ) << error.toStdString();
   first->rawQuery( QString( "UPDATE %1 SET v = 1 WHERE id = 1" ).arg( table ) );
   first->rawQuery( QString( "ANALYZE TABLE %1" ).arg( table ) );

   EXPECT_FALSE( first->rollbackTransaction( error ) );
   EXPECT_TRUE( error.contains( "may have been kept" ) ) << error.toStdString();
   EXPECT_FALSE( error.contains( "deadlock" ) ) << error.toStdString();
   EXPECT_EQ( scalar( *first, QString( "SELECT v FROM %1 WHERE id = 1" ).arg( table ) ), "1" );
}

// The transaction ends with no error the client could record, so only the
// server status shows it.  A COMMIT now would succeed as a no-op.
TEST_F( US_DB2_LiveTransaction, CommitAfterSilentRollbackIsRefused )
{
   QString error;
   ASSERT_TRUE( first->beginTransaction( error ) ) << error.toStdString();
   first->rawQuery( QString( "UPDATE %1 SET v = 1 WHERE id = 1" ).arg( table ) );
   first->rawQuery( QString( "CALL %1()" ).arg( rollbackProcedure ) );

   EXPECT_FALSE( first->commitTransaction( error ) );
   EXPECT_TRUE( error.contains( "may have been kept" ) ) << error.toStdString();

   EXPECT_FALSE( first->rollbackTransaction( error ) );
   EXPECT_EQ( scalar( *first, QString( "SELECT v FROM %1 WHERE id = 1" ).arg( table ) ), "0" );
}

// With innodb_rollback_on_timeout off, the default, a timeout undoes only the
// statement; the transaction and the writes before it are still there.
TEST_F( US_DB2_LiveTransaction, CommitAfterLockWaitTimeoutKeepsEarlierWrites )
{
   if ( scalar( *first, "SELECT @@innodb_rollback_on_timeout" ) != "0" )
   {
      GTEST_SKIP() << "innodb_rollback_on_timeout is set on this server";
   }

   QString error;
   ASSERT_TRUE( first->beginTransaction( error ) ) << error.toStdString();
   ASSERT_TRUE( second->beginTransaction( error ) ) << error.toStdString();
   second->rawQuery( QString( "UPDATE %1 SET v = 2 WHERE id = 2" ).arg( table ) );

   first->rawQuery( "SET SESSION innodb_lock_wait_timeout = 1" );
   first->rawQuery( QString( "UPDATE %1 SET v = 1 WHERE id = 1" ).arg( table ) );
   first->rawQuery( QString( "UPDATE %1 SET v = 1 WHERE id = 2" ).arg( table ) );
   ASSERT_TRUE( first->lastError().contains( "Lock wait timeout" ) ) << first->lastError().toStdString();

   EXPECT_TRUE( first->commitTransaction( error ) ) << error.toStdString();
   EXPECT_TRUE( second->rollbackTransaction( error ) ) << error.toStdString();
   EXPECT_EQ( scalar( *first, QString( "SELECT v FROM %1 WHERE id = 1" ).arg( table ) ), "1" );
   EXPECT_EQ( scalar( *first, QString( "SELECT v FROM %1 WHERE id = 2" ).arg( table ) ), "0" );
}

TEST_F( US_DB2_LiveTransaction, UnmatchedTransactionCallsAreRefused )
{
   QString error;
   EXPECT_FALSE( first->commitTransaction( error ) );
   EXPECT_EQ( error, QObject::tr( US_DB2::noTransactionMessage ).arg( "commitTransaction" ) );
   EXPECT_FALSE( first->rollbackTransaction( error ) );
   EXPECT_EQ( error, QObject::tr( US_DB2::noTransactionMessage ).arg( "rollbackTransaction" ) );

   ASSERT_TRUE( first->beginTransaction( error ) ) << error.toStdString();
   EXPECT_FALSE( first->beginTransaction( error ) );
   EXPECT_EQ( error, QObject::tr( US_DB2::transactionOpenMessage ).arg( "beginTransaction" ) );

   // The refused begin left the first transaction open.
   EXPECT_TRUE( first->commitTransaction( error ) ) << error.toStdString();
}

// query() reads a CALL as a status set followed by a data set.  A failure on
// the data set is the query's result, not the status row's OK.
TEST_F( US_DB2_LiveTransaction, QueryReportsAFailureAfterTheStatusRow )
{
   first->query( QString( "CALL %1()" ).arg( failingProcedure ) );

   EXPECT_EQ( first->lastErrno(), IUS_DB2::DBERROR );
   EXPECT_TRUE( first->lastError().contains( "us3_test_no_such_table" ) ) << first->lastError().toStdString();
   EXPECT_FALSE( first->next() );
}
#endif
} // namespace
