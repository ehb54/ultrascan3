//! \file test_us_solution_vals_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_settings.h"
#include "us_solution_vals.h"

using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

// Not covered: values(), which combines these with the disk readers, and the disk readers.

namespace
{
const QString               kRawGuid    = "ffffffff-1111-2222-3333-444444444444";
const QString               kBufferGuid = "bbbbbbbb-1111-2222-3333-444444444444";

Matcher<const QStringList&> args( const QStringList& expected )
{
   return Matcher<const QStringList&>( Eq( expected ) );
}

class SolutionValsDb : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock> db;
      US_DataIO::EditedData   edata;
      QString                 cvbar20 = "0.70";
      QString                 soluID  = "previous";
      QString                 bufId;
      QString                 bufGuid   = "previous-guid";
      QString                 bufDesc   = "previous description";
      QString                 density   = "0.9982";
      QString                 viscosity = "1.002";
      QString                 compress  = "4.6e-10";
      QString                 manual    = "0";
      QString                 errmsg;

      void                    SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         US_Settings::set_us_inv_ID( 23 );
         edata.dataGUID = kRawGuid;
         EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString( "db error" ) ) );
      }

      bool solinfo()
      {
         return US_SolutionVals::solinfo_db( &db, &edata, cvbar20, soluID, bufId, bufGuid, bufDesc, errmsg );
      }

      bool bufvals()
      {
         return US_SolutionVals::bufvals_db( &db, bufId, bufGuid, bufDesc, density, viscosity, compress, manual, errmsg );
      }

      void expectQuery( const QStringList& query, int status, bool row )
      {
         EXPECT_CALL( db, query( args( query ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
         if ( status == IUS_DB2::OK )
         {
            EXPECT_CALL( db, next() ).WillOnce( Return( row ) );
         }
      }

      void expectRawRow()
      {
         // Column 0, the raw data ID itself, is not needed.
         expectQuery( { "get_rawDataID_from_GUID", kRawGuid }, IUS_DB2::OK, true );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "11" ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "7" ) ) );
      }

      void expectSolutionBufferRow( const QString& solution )
      {
         expectQuery( { "get_solutionBuffer", solution }, IUS_DB2::OK, true );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "41" ) ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( kBufferGuid ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "Buffer A" ) ) );
      }

      void expectBufferInfo( const QString& id, const QString& description )
      {
         expectQuery( { "get_buffer_info", id }, IUS_DB2::OK, true );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( description ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "4.5e-10" ) ) );
         EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( "1.0213" ) ) );
         EXPECT_CALL( db, value( 5 ) ).WillOnce( Return( QVariant( "1.00512" ) ) );
         EXPECT_CALL( db, value( 6 ) ).WillOnce( Return( QVariant( "0" ) ) );
      }

      void expectBufferValuesUnchanged()
      {
         EXPECT_EQ( QString( "0.9982" ), density );
         EXPECT_EQ( QString( "1.002" ), viscosity );
         EXPECT_EQ( QString( "4.6e-10" ), compress );
         EXPECT_EQ( QString( "0" ), manual );
      }
};

// Solution info

TEST_F( SolutionValsDb, SolutionInfoFollowsRawDataToSolutionAndBuffer )
{
   {
      InSequence sequence;
      expectRawRow();
      expectSolutionBufferRow( "7" );
      expectQuery( { "get_solution", "7" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "0.7341" ) ) );
   }
   EXPECT_TRUE( solinfo() );
   EXPECT_EQ( QString( "7" ), soluID );
   EXPECT_EQ( QString( "41" ), bufId );
   EXPECT_EQ( kBufferGuid, bufGuid );
   EXPECT_EQ( QString( "Buffer A" ), bufDesc );
   EXPECT_EQ( QString( "0.7341" ), cvbar20 );
}

TEST_F( SolutionValsDb, SolutionInfoFallsBackToTheExperimentsSolution )
{
   {
      InSequence sequence;
      expectRawRow();
      expectQuery( { "get_solutionBuffer", "7" }, IUS_DB2::NOROWS, false );
      expectQuery( { "get_solutionIDs", "11" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "8" ) ) );
      expectSolutionBufferRow( "8" );
      expectQuery( { "get_solution", "8" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "0.7341" ) ) );
   }
   EXPECT_TRUE( solinfo() );
   EXPECT_EQ( QString( "8" ), soluID );
   EXPECT_EQ( QString( "41" ), bufId );
}

TEST_F( SolutionValsDb, MissingRawDataRowReadsNothing )
{
   {
      InSequence sequence;
      expectQuery( { "get_rawDataID_from_GUID", kRawGuid }, IUS_DB2::OK, false );
   }
   EXPECT_FALSE( solinfo() );
   EXPECT_EQ( QString( "previous" ), soluID );
   EXPECT_EQ( QString( "0.70" ), cvbar20 );
   EXPECT_TRUE( bufId.isEmpty() );
   EXPECT_FALSE( errmsg.isEmpty() );
}

TEST_F( SolutionValsDb, MissingFallbackSolutionReadsNoBuffer )
{
   {
      InSequence sequence;
      expectRawRow();
      expectQuery( { "get_solutionBuffer", "7" }, IUS_DB2::DBERROR, false );
      expectQuery( { "get_solutionIDs", "11" }, IUS_DB2::OK, false );
   }
   EXPECT_FALSE( solinfo() );
   EXPECT_TRUE( bufId.isEmpty() );
   EXPECT_EQ( QString( "previous-guid" ), bufGuid );
   EXPECT_EQ( QString( "0.70" ), cvbar20 );
   EXPECT_FALSE( errmsg.isEmpty() );
}

TEST_F( SolutionValsDb, MissingSolutionRowKeepsTheVbar )
{
   {
      InSequence sequence;
      expectRawRow();
      expectSolutionBufferRow( "7" );
      expectQuery( { "get_solution", "7" }, IUS_DB2::OK, false );
   }
   EXPECT_TRUE( solinfo() );
   EXPECT_EQ( QString( "0.70" ), cvbar20 );
   EXPECT_FALSE( errmsg.isEmpty() );
}

// Buffer values

TEST_F( SolutionValsDb, BufferValuesById )
{
   bufId = "41";
   {
      InSequence sequence;
      expectBufferInfo( "41", "Buffer A  [M]" );
   }
   EXPECT_TRUE( bufvals() );
   EXPECT_EQ( QString( "1.00512" ), density );
   EXPECT_EQ( QString( "1.0213" ), viscosity );
   EXPECT_EQ( QString( "4.5e-10" ), compress );
   EXPECT_EQ( QString( "1" ), manual ); // the description marker wins over the stored flag
}

TEST_F( SolutionValsDb, BufferValuesByGuidAndByDescription )
{
   bufGuid = kBufferGuid;
   {
      InSequence sequence;
      expectQuery( { "get_bufferID", kBufferGuid }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "41" ) ) );
      expectBufferInfo( "41", "Buffer A" );
   }
   EXPECT_TRUE( bufvals() );
   EXPECT_EQ( QString( "41" ), bufId );
   EXPECT_EQ( QString( "0" ), manual );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );

   bufId.clear();
   bufGuid.clear();
   bufDesc = "Buffer B";
   {
      InSequence sequence;
      expectQuery( { "get_buffer_desc", "23" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "Buffer A" ) ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "Buffer B" ) ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "42" ) ) );
      expectBufferInfo( "42", "Buffer B" );
   }
   EXPECT_TRUE( bufvals() );
   EXPECT_EQ( QString( "42" ), bufId );
}

TEST_F( SolutionValsDb, MissingOrFailedBufferRowKeepsTheValues )
{
   bufId = "41";
   for ( int status : { IUS_DB2::DBERROR, IUS_DB2::OK } )
   {
      SCOPED_TRACE( status );
      errmsg.clear();
      {
         InSequence sequence;
         expectQuery( { "get_buffer_info", "41" }, status, false );
      }
      EXPECT_FALSE( bufvals() );
      expectBufferValuesUnchanged();
      EXPECT_FALSE( errmsg.isEmpty() );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
      EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
   }
}

TEST_F( SolutionValsDb, UnmatchedDescriptionReadsNoBuffer )
{
   bufGuid.clear();
   bufDesc = "Buffer C";
   {
      InSequence sequence;
      expectQuery( { "get_buffer_desc", "23" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "Buffer A" ) ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
   }
   EXPECT_FALSE( bufvals() );
   EXPECT_TRUE( bufId.isEmpty() );
   expectBufferValuesUnchanged();
}
} // namespace
