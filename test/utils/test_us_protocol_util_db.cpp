//! \file test_us_protocol_util_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_protocol_util.h"

using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

namespace
{
Matcher<const QStringList&> args( const QStringList& expected )
{
   return Matcher<const QStringList&>( Eq( expected ) );
}

class ProtocolUtilDb : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock> db;
      void                    SetUp() override
      {
         QtTestBase::SetUp();
         US_Settings::set_us_inv_ID( 23 );
      }

      // The code under test also logs the error state with qDebug, so how often
      // it reads lastErrno/lastError is not part of the contract. The StrictMock
      // still fails on any row read (next/value) or lastInsertID.
      void expectFailedStatus()
      {
         EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::DBERROR ) );
         EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString( "failed" ) ) );
      }
};

TEST_F( ProtocolUtilDb, FailedListsClearStaleOutputWithoutReadingRows )
{
   for ( bool automatic : { false, true } )
   {
      QList<QStringList> entries{ QStringList{ "stale" } };
      EXPECT_CALL( db, query( args( { automatic ? "get_protocol_desc_auto" : "get_protocol_desc", "23" } ) ) );
      expectFailedStatus();
      EXPECT_EQ( 0, automatic ? US_ProtocolUtil::list_all_auto( entries, &db ) : US_ProtocolUtil::list_all( entries, &db ) );
      EXPECT_TRUE( entries.isEmpty() );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( ProtocolUtilDb, FailedWriteDoesNotReadInsertId )
{
   const QString xml = "<protocol description='Test' guid='guid' investigator='23: Test'/>";
   EXPECT_CALL( db, query( args( { "new_protocol", "23", "guid", "Test", xml, "", "-1", "", "0", "0", "0", "", "", "0" } ) ) );
   expectFailedStatus();
   EXPECT_EQ( -1, US_ProtocolUtil::write_record( xml, &db ) );
}

TEST_F( ProtocolUtilDb, FailedReadsClearOutputsWithoutReadingRows )
{
   for ( bool automatic : { false, true } )
   {
      QString     xml = "stale";
      QStringList entry{ "stale" };
      {
         InSequence sequence;
         EXPECT_CALL( db, query( args( { "get_protocol_desc", automatic ? "29" : "23" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
      }
      EXPECT_EQ( -1,
                 automatic ? US_ProtocolUtil::read_record_auto( "Test", 29, &xml, &entry, &db )
                           : US_ProtocolUtil::read_record( "Test", &xml, &entry, &db ) );
      EXPECT_TRUE( xml.isEmpty() );
      EXPECT_TRUE( entry.isEmpty() );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( ProtocolUtilDb, DeleteReportsDatabaseStatus )
{
   for ( int status : { IUS_DB2::DBERROR, IUS_DB2::OK } )
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "delete_protocol", "23", "41" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
      EXPECT_EQ( status == IUS_DB2::OK, US_ProtocolUtil::delete_record( "41", &db ) );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}
}
