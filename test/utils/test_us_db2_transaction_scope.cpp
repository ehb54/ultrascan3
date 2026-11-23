//! \file test_us_db2_transaction_scope.cpp
// US_DB2Transaction, the owned-or-joined transaction used by the DB writers.
// A strict mock fails each test on any transaction call it does not expect.
#include "ius_db2.h"
#include "mock_us_db2.h"
#include "qt_test_base.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::StrictMock;

namespace
{
class DB2TransactionScope : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock> db;
      QString                 error = "stale";
};

TEST_F( DB2TransactionScope, OwnsAndCommitsANewTransaction )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, transactionActive() ).WillOnce( Return( false ) );
      EXPECT_CALL( db, beginTransaction( _ ) ).WillOnce( Return( true ) );
      EXPECT_CALL( db, commitTransaction( _ ) ).WillOnce( Return( true ) );
   }
   US_DB2Transaction transaction( &db );
   ASSERT_TRUE( transaction.begin( error ) );
   EXPECT_TRUE( transaction.owns() );
   EXPECT_TRUE( transaction.commit( error ) );
   EXPECT_TRUE( error.isEmpty() );
   // Committed: neither a later rollback nor the destructor touches the DB.
   EXPECT_TRUE( transaction.rollback( error ) );
}

TEST_F( DB2TransactionScope, JoinsTheCallersTransactionAndLeavesItsEndToTheCaller )
{
   EXPECT_CALL( db, transactionActive() ).WillOnce( Return( true ) );
   US_DB2Transaction transaction( &db );
   ASSERT_TRUE( transaction.begin( error ) );
   EXPECT_FALSE( transaction.owns() );
   EXPECT_TRUE( transaction.commit( error ) );
   EXPECT_TRUE( transaction.rollback( error ) );
}

TEST_F( DB2TransactionScope, AFailedBeginIsReportedAndOwnsNothing )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, transactionActive() ).WillOnce( Return( false ) );
      EXPECT_CALL( db, beginTransaction( _ ) ).WillOnce( DoAll( SetArgReferee<0>( QString( "refused" ) ), Return( false ) ) );
   }
   US_DB2Transaction transaction( &db );
   EXPECT_FALSE( transaction.begin( error ) );
   EXPECT_EQ( QString( "refused" ), error );
   EXPECT_FALSE( transaction.owns() );
}

TEST_F( DB2TransactionScope, AnOwnedTransactionLeftOpenIsRolledBackOnDestruction )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, transactionActive() ).WillOnce( Return( false ) );
      EXPECT_CALL( db, beginTransaction( _ ) ).WillOnce( Return( true ) );
      EXPECT_CALL( db, rollbackTransaction( _ ) ).WillOnce( Return( true ) );
   }
   US_DB2Transaction transaction( &db );
   ASSERT_TRUE( transaction.begin( error ) );
}

TEST_F( DB2TransactionScope, AFailedCommitLeavesItOpenForTheRollback )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, transactionActive() ).WillOnce( Return( false ) );
      EXPECT_CALL( db, beginTransaction( _ ) ).WillOnce( Return( true ) );
      EXPECT_CALL( db, commitTransaction( _ ) ).WillOnce( DoAll( SetArgReferee<0>( QString( "lost" ) ), Return( false ) ) );
      EXPECT_CALL( db, rollbackTransaction( _ ) ).WillOnce( Return( true ) );
   }
   US_DB2Transaction transaction( &db );
   ASSERT_TRUE( transaction.begin( error ) );
   EXPECT_FALSE( transaction.commit( error ) );
   EXPECT_EQ( QString( "lost" ), error );
   EXPECT_TRUE( transaction.rollback( error ) );
}

TEST_F( DB2TransactionScope, AnUnconfirmedRollbackIsReportedOnceAndNotRepeated )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, transactionActive() ).WillOnce( Return( false ) );
      EXPECT_CALL( db, beginTransaction( _ ) ).WillOnce( Return( true ) );
      EXPECT_CALL( db, rollbackTransaction( _ ) ).WillOnce( DoAll( SetArgReferee<0>( QString( "gone" ) ), Return( false ) ) );
   }
   US_DB2Transaction transaction( &db );
   ASSERT_TRUE( transaction.begin( error ) );
   EXPECT_FALSE( transaction.rollback( error ) );
   EXPECT_EQ( QString( "gone" ), error );
   // The transaction is over either way; the destructor does not roll back again.
}
} // namespace
