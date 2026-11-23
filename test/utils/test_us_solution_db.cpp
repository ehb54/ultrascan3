//! \file test_us_solution_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_eprofile.h"
#include "us_settings.h"
#include "us_solution.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

// Not covered: the disk branch of countInProtocols, and a failed analyte file in saveToDisk.

namespace
{
const QString               kSolutionGuid = "aaaaaaaa-1111-2222-3333-444444444444";
const QString               kBufferGuid   = "bbbbbbbb-1111-2222-3333-444444444444";
const QString               kAnalyteGuid  = "cccccccc-1111-2222-3333-444444444444";

Matcher<const QStringList&> args( const QStringList& expected )
{
   return Matcher<const QStringList&>( Eq( expected ) );
}

QString profileXml( const QMap<double, double>& values )
{
   QString              compType = "Sample";
   QString              valType  = "molarExtinction";
   QMap<double, double> copy     = values;
   QString              xml;
   US_ExtProfile::xml_eprofile( 7, compType, valType, copy, xml );
   return xml;
}

class SolutionDb : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock> db;
      US_Solution             solution;

      void                    SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         US_Settings::set_us_inv_ID( 23 );
         // The analyte reader's debug line reads the error text.
         EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
      }

      // A loaded solution that a failed read must not leave in place.
      void prepareLoaded()
      {
         solution.solutionID   = 3;
         solution.solutionGUID = "previous-guid";
         solution.solutionDesc = "Previous solution";
         solution.notes        = "previous notes";
         solution.buffer.GUID  = "previous-buffer";
         solution.extinction   = { { 300.0, 9.0 } };
         US_Solution::AnalyteInfo info;
         info.analyte.analyteGUID = "previous-analyte";
         info.amount              = 1.5;
         solution.analyteInfo << info;
      }

      // A failed read clears the solution: callers that ignore the status must not keep
      // the previously loaded solution, and nothing from the failed read is published.
      void expectCleared()
      {
         EXPECT_EQ( 0, solution.solutionID );
         EXPECT_TRUE( solution.solutionGUID.isEmpty() );
         EXPECT_TRUE( solution.solutionDesc.isEmpty() );
         EXPECT_TRUE( solution.notes.isEmpty() );
         EXPECT_TRUE( solution.buffer.GUID.isEmpty() );
         EXPECT_TRUE( solution.analyteInfo.isEmpty() );
         EXPECT_TRUE( solution.extinction.isEmpty() );
         EXPECT_EQ( US_Solution::NOT_SAVED, solution.saveStatus );
      }

      void expectSolutionRow()
      {
         EXPECT_CALL( db, query( args( { "get_solution", "7" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( kSolutionGuid ) ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "Sample A" ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "0.73" ) ) );
         EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "4" ) ) );
         EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( "stored notes" ) ) );
      }

      void expectSolutionBufferRow()
      {
         EXPECT_CALL( db, query( args( { "get_solutionBuffer", "7" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "41" ) ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( kBufferGuid ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "Buffer A" ) ) );
      }

      // US_Buffer::readFromDB for buffer 41 with no components and no profile.
      void expectBufferRead()
      {
         EXPECT_CALL( db, query( args( { "get_buffer_info", "41" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( kBufferGuid ) ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "Buffer A" ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "4.6e-10" ) ) );
         EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "7.0" ) ) );
         EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( "1.002" ) ) );
         EXPECT_CALL( db, value( 5 ) ).WillOnce( Return( QVariant( "0.9982" ) ) );
         EXPECT_CALL( db, value( 7 ) ).WillOnce( Return( QVariant( "Investigator, Test" ) ) );
         EXPECT_CALL( db, value( 6 ) ).WillOnce( Return( QVariant( "0" ) ) );
         EXPECT_CALL( db, query( args( { "get_buffer_components", "41" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
         EXPECT_CALL( db, query( args( { "get_eprofile", "41", "Buffer" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      }

      // get_solutionAnalyte reports a solution without analytes as NO_ANALYTE.
      void expectAnalyteRows( const QStringList& guids )
      {
         EXPECT_CALL( db, query( args( { "get_solutionAnalyte", "7" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( guids.isEmpty() ? IUS_DB2::NO_ANALYTE : IUS_DB2::OK ) );
         for ( const QString& guid : guids )
         {
            EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
            EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( guid ) ) );
            EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "2.5" ) ) );
         }
         EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
      }

      // A solution ready to save, with no buffer and one stored analyte.
      void prepareToSave()
      {
         solution.solutionGUID = kSolutionGuid;
         solution.solutionDesc = "Sample A";
         solution.commonVbar20 = 0.73;
         solution.storageTemp  = 4;
         solution.notes        = "notes";
         solution.buffer.GUID.clear();
         US_Solution::AnalyteInfo info;
         info.analyte.analyteGUID = kAnalyteGuid;
         info.amount              = 2.5;
         solution.analyteInfo << info;
      }

      void expectAnalyteFound()
      {
         EXPECT_CALL( db, query( args( { "get_analyteID", kAnalyteGuid } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      }

      void expectNoSolutionRow()
      {
         EXPECT_CALL( db, query( args( { "get_solutionID_from_GUID", kSolutionGuid } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      }

      static QStringList newSolutionArgs()
      {
         return { "new_solution", kSolutionGuid, "Sample A", "0.73", "4", "notes", "11", "2", "23" };
      }
};

// Read

TEST_F( SolutionDb, ReadMapsSolutionBufferAndProfile )
{
   const QMap<double, double> stored{ { 280.0, 1.5 } };
   {
      InSequence sequence;
      expectSolutionRow();
      expectSolutionBufferRow();
      expectBufferRead();
      expectAnalyteRows( {} );
      EXPECT_CALL( db, query( args( { "get_eprofile", "7", "Sample" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "3" ) ) );
      EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "molarExtinction" ) ) );
      EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( profileXml( stored ) ) ) );
   }

   ASSERT_EQ( IUS_DB2::OK, solution.readFromDB( 7, &db ) );
   EXPECT_EQ( 7, solution.solutionID );
   EXPECT_EQ( kSolutionGuid, solution.solutionGUID );
   EXPECT_EQ( QString( "Sample A" ), solution.solutionDesc );
   EXPECT_DOUBLE_EQ( 0.73, solution.commonVbar20 );
   EXPECT_DOUBLE_EQ( 4.0, solution.storageTemp );
   EXPECT_EQ( QString( "stored notes" ), solution.notes );
   EXPECT_EQ( QString( "41" ), solution.buffer.bufferID );
   EXPECT_EQ( kBufferGuid, solution.buffer.GUID );
   EXPECT_DOUBLE_EQ( 1.002, solution.buffer.viscosity );
   EXPECT_TRUE( solution.analyteInfo.isEmpty() );
   EXPECT_EQ( stored, solution.extinction );
   EXPECT_EQ( US_Solution::DB_ONLY, solution.saveStatus );
}

TEST_F( SolutionDb, FailedOrMissingSolutionRowClearsTheSolution )
{
   for ( int status : { IUS_DB2::DBERROR, IUS_DB2::NOT_CONNECTED, IUS_DB2::NOROWS, IUS_DB2::OK } )
   {
      SCOPED_TRACE( status );
      prepareLoaded();
      {
         InSequence sequence;
         EXPECT_CALL( db, query( args( { "get_solution", "7" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
         if ( status == IUS_DB2::OK )
         {
            EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
         }
      }
      const int expected = ( status == IUS_DB2::OK || status == IUS_DB2::NOROWS ) ? IUS_DB2::NO_SOLUTION : status;
      EXPECT_EQ( expected, solution.readFromDB( 7, &db ) );
      expectCleared();
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
      EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
   }
}

TEST_F( SolutionDb, FailedOrMissingSolutionBufferClearsTheSolution )
{
   for ( const bool missingRow : { false, true } )
   {
      SCOPED_TRACE( missingRow );
      prepareLoaded();
      {
         InSequence sequence;
         expectSolutionRow();
         EXPECT_CALL( db, query( args( { "get_solutionBuffer", "7" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( missingRow ? IUS_DB2::OK : IUS_DB2::DBERROR ) );
         if ( missingRow )
         {
            EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
         }
      }
      EXPECT_EQ( missingRow ? IUS_DB2::NO_BUFFER : IUS_DB2::DBERROR, solution.readFromDB( 7, &db ) );
      expectCleared();
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
      EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
   }
}

TEST_F( SolutionDb, UnreadableBufferClearsTheSolution )
{
   prepareLoaded();
   {
      InSequence sequence;
      expectSolutionRow();
      expectSolutionBufferRow();
      EXPECT_CALL( db, query( args( { "get_buffer_info", "41" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::NO_BUFFER, solution.readFromDB( 7, &db ) );
   expectCleared();
}

TEST_F( SolutionDb, FailedAnalyteQueryOrUnreadableAnalyteClearsTheSolution )
{
   prepareLoaded();
   {
      InSequence sequence;
      expectSolutionRow();
      expectSolutionBufferRow();
      expectBufferRead();
      EXPECT_CALL( db, query( args( { "get_solutionAnalyte", "7" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, solution.readFromDB( 7, &db ) );
   expectCleared();
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
   prepareLoaded();

   // A refused permission check is a failure, not an empty analyte list.
   {
      InSequence sequence;
      expectSolutionRow();
      expectSolutionBufferRow();
      expectBufferRead();
      EXPECT_CALL( db, query( args( { "get_solutionAnalyte", "7" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOTPERMITTED ) );
   }
   EXPECT_EQ( IUS_DB2::NOTPERMITTED, solution.readFromDB( 7, &db ) );
   expectCleared();
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
   prepareLoaded();

   {
      InSequence sequence;
      expectSolutionRow();
      expectSolutionBufferRow();
      expectBufferRead();
      expectAnalyteRows( { kAnalyteGuid } );
      EXPECT_CALL( db, query( args( { "get_analyteID", kAnalyteGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::NOROWS ) );
   }
   EXPECT_EQ( IUS_DB2::NO_ANALYTE, solution.readFromDB( 7, &db ) );
   expectCleared();
}

TEST_F( SolutionDb, ReadWithoutAStoredProfileHasNoExtinction )
{
   prepareLoaded();
   {
      InSequence sequence;
      expectSolutionRow();
      expectSolutionBufferRow();
      expectBufferRead();
      expectAnalyteRows( {} );
      EXPECT_CALL( db, query( args( { "get_eprofile", "7", "Sample" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
   }
   ASSERT_EQ( IUS_DB2::OK, solution.readFromDB( 7, &db ) );
   EXPECT_TRUE( solution.extinction.isEmpty() );
   EXPECT_TRUE( solution.analyteInfo.isEmpty() );
}

// Save

TEST_F( SolutionDb, SaveInsertsANewSolutionAndItsAnalytes )
{
   prepareToSave();
   {
      InSequence sequence;
      expectAnalyteFound();
      expectNoSolutionRow();
      EXPECT_CALL( db, statusQuery( args( newSolutionArgs() ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 55 ) );
      EXPECT_CALL( db, statusQuery( args( { "delete_solutionAnalytes", "55" } ) ) ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( args( { "new_solutionAnalyte", "55", "0", kAnalyteGuid, "2.5" } ) ) )
         .WillOnce( Return( IUS_DB2::OK ) );
   }
   ASSERT_EQ( IUS_DB2::OK, solution.saveToDB( 11, 2, &db ) );
   EXPECT_EQ( 55, solution.solutionID );
   EXPECT_EQ( US_Solution::BOTH, solution.saveStatus );
}

TEST_F( SolutionDb, SaveUpdatesAStoredSolution )
{
   prepareToSave();
   solution.analyteInfo.clear();
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_solutionID_from_GUID", kSolutionGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "55" ) ) );
      EXPECT_CALL( db, statusQuery( args( { "update_solution", "55", kSolutionGuid, "Sample A", "0.73", "4", "notes" } ) ) )
         .WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, statusQuery( args( { "delete_solutionAnalytes", "55" } ) ) ).WillOnce( Return( IUS_DB2::OK ) );
   }
   EXPECT_CALL( db, lastInsertID() ).Times( 0 );
   ASSERT_EQ( IUS_DB2::OK, solution.saveToDB( 11, 2, &db ) );
   EXPECT_EQ( 55, solution.solutionID );
}

TEST_F( SolutionDb, SaveTreatsAnEmptyLookupAsANewSolution )
{
   prepareToSave();
   solution.analyteInfo.clear();
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_solutionID_from_GUID", kSolutionGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
      EXPECT_CALL( db, statusQuery( args( newSolutionArgs() ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 56 ) );
      EXPECT_CALL( db, statusQuery( args( { "delete_solutionAnalytes", "56" } ) ) ).WillOnce( Return( IUS_DB2::NOROWS ) );
   }
   EXPECT_CALL( db, value( _ ) ).Times( 0 );
   ASSERT_EQ( IUS_DB2::OK, solution.saveToDB( 11, 2, &db ) );
   EXPECT_EQ( 56, solution.solutionID );
}

TEST_F( SolutionDb, FailedBufferOrAnalyteSaveStopsTheSave )
{
   prepareToSave();
   solution.buffer.GUID = kBufferGuid;
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_bufferID", kBufferGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      // US_Buffer::saveToDB looks the buffer up again, and that fails.
      EXPECT_CALL( db, query( args( { "get_bufferID", kBufferGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::NO_BUFFER, solution.saveToDB( 11, 2, &db ) );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );

   solution.buffer.GUID.clear();
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_analyteID", kAnalyteGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      // US_Analyte::write looks the analyte up again, and that fails.
      EXPECT_CALL( db, query( args( { "get_analyteID", kAnalyteGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::NO_ANALYTE, solution.saveToDB( 11, 2, &db ) );
}

TEST_F( SolutionDb, FailedLookupOrSolutionWriteStopsTheSave )
{
   prepareToSave();
   solution.analyteInfo.clear();
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_solutionID_from_GUID", kSolutionGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, solution.saveToDB( 11, 2, &db ) );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );

   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_solutionID_from_GUID", kSolutionGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "55" ) ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, solution.saveToDB( 11, 2, &db ) );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );

   {
      InSequence sequence;
      expectNoSolutionRow();
      EXPECT_CALL( db, statusQuery( args( newSolutionArgs() ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_CALL( db, lastInsertID() ).Times( 0 );
   EXPECT_EQ( IUS_DB2::DBERROR, solution.saveToDB( 11, 2, &db ) );
}

TEST_F( SolutionDb, FailedAnalyteUnlinkStopsBeforeNewLinks )
{
   prepareToSave();
   {
      InSequence sequence;
      expectAnalyteFound();
      expectNoSolutionRow();
      EXPECT_CALL( db, statusQuery( args( newSolutionArgs() ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 55 ) );
      EXPECT_CALL( db, statusQuery( args( { "delete_solutionAnalytes", "55" } ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, solution.saveToDB( 11, 2, &db ) );
}

TEST_F( SolutionDb, FailedProfileWriteIsReportedAndStaysPending )
{
   prepareToSave();
   solution.analyteInfo.clear();
   solution.extinction              = { { 280.0, 1.5 } };
   solution.new_or_changed_spectrum = true;
   {
      InSequence sequence;
      expectNoSolutionRow();
      EXPECT_CALL( db, statusQuery( args( newSolutionArgs() ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 55 ) );
      EXPECT_CALL( db, statusQuery( args( { "delete_solutionAnalytes", "55" } ) ) ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, query( args( { "get_eprofile", "55", "Sample" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, solution.saveToDB( 11, 2, &db ) );
   EXPECT_TRUE( solution.new_or_changed_spectrum );
   EXPECT_NE( US_Solution::BOTH, solution.saveStatus );
}

// Delete and protocol count

TEST_F( SolutionDb, DeleteLooksUpTheIdAndKeepsStatusOnFailure )
{
   solution.solutionGUID = kSolutionGuid;
   solution.saveStatus   = US_Solution::BOTH;
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_solutionID_from_GUID", kSolutionGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, solution.deleteFromDB( &db ) );
   EXPECT_EQ( US_Solution::BOTH, solution.saveStatus );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );

   for ( int status : { IUS_DB2::DBERROR, IUS_DB2::SOLUT_IN_USE } )
   {
      SCOPED_TRACE( status );
      solution.solutionID = 55;
      EXPECT_CALL( db, statusQuery( args( { "delete_solution", "55" } ) ) ).WillOnce( Return( status ) );
      EXPECT_EQ( status, solution.deleteFromDB( &db ) );
      EXPECT_EQ( US_Solution::BOTH, solution.saveStatus );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
      EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
   }

   EXPECT_CALL( db, statusQuery( args( { "delete_solution", "55" } ) ) ).WillOnce( Return( IUS_DB2::OK ) );
   EXPECT_EQ( IUS_DB2::OK, solution.deleteFromDB( &db ) );
   EXPECT_EQ( US_Solution::HD_ONLY, solution.saveStatus );
}

TEST_F( SolutionDb, DeleteOfAnUnstoredSolutionDeletesNothing )
{
   solution.solutionGUID = kSolutionGuid;
   solution.saveStatus   = US_Solution::HD_ONLY;
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_solutionID_from_GUID", kSolutionGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
   }
   EXPECT_EQ( IUS_DB2::OK, solution.deleteFromDB( &db ) );
   EXPECT_EQ( 0, solution.solutionID );
}

TEST_F( SolutionDb, CountInProtocolsMatchesTheSolutionIdAndReportsAFailedQuery )
{
   solution.solutionID = 55;
   const QString uses  = "<ProtocolData><protocol><solution name=\"Sample A\" id=\"55\"/></protocol></ProtocolData>";
   const QString other = "<ProtocolData><protocol><solution name=\"Sample A\" id=\"56\"/></protocol></ProtocolData>";
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_protocol_desc", "23" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      for ( const QString& xml : { uses, other, uses } )
      {
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( xml ) ) );
      }
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
   }
   EXPECT_CALL( db, value( 2 ) ).Times( AnyNumber() ).WillRepeatedly( Return( QVariant( "protocol" ) ) );
   EXPECT_EQ( 2, solution.countInProtocols( &db ) );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );

   // A failed query must not look like "not in use", which would allow a delete.
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_protocol_desc", "23" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_LT( solution.countInProtocols( &db ), 0 );
}

TEST_F( SolutionDb, NewBufferSaveParticipatesInRunImportTransaction )
{
   prepareToSave();
   solution.analyteInfo.clear();
   solution.buffer.GUID        = kBufferGuid;
   solution.buffer.description = "Import buffer";
   QString error;
   {
      InSequence sequence;
      // The import caller opens and closes the transaction around solution save.
      EXPECT_CALL( db, beginTransaction( _ ) ).WillOnce( Return( true ) );
      EXPECT_CALL( db, query( args( { "get_bufferID", kBufferGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, query( args( { "get_bufferID", kBufferGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, transactionActive() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            []( const QStringList& q )
            {
               EXPECT_EQ( ( QStringList{ "new_buffer", kBufferGuid, "Import buffer" } ), q.mid( 0, 3 ) );
               return IUS_DB2::OK;
            } ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 41 ) );
      expectNoSolutionRow();
      EXPECT_CALL( db, statusQuery( args( newSolutionArgs() ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 55 ) );
      EXPECT_CALL( db, statusQuery( args( { "new_solutionBuffer", "55", "0", kBufferGuid } ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, statusQuery( args( { "delete_solutionAnalytes", "55" } ) ) ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, commitTransaction( _ ) ).WillOnce( Return( true ) );
   }
   ASSERT_TRUE( db.beginTransaction( error ) );
   ASSERT_EQ( IUS_DB2::OK, solution.saveToDB( 11, 2, &db ) );
   EXPECT_EQ( QString( "41" ), solution.buffer.bufferID );
   ASSERT_TRUE( db.commitTransaction( error ) );
}

TEST_F( SolutionDb, BufferDiskCopyFailureStillSavesTheSolution )
{
   prepareToSave();
   solution.analyteInfo.clear();
   solution.buffer.GUID        = kBufferGuid;
   solution.buffer.description = "Saved buffer";
   // A file where the buffers directory goes makes only the buffer's disk copy fail.
   const QString buffers       = US_Settings::dataDir() + "/buffers";
   ASSERT_TRUE( QDir().mkpath( US_Settings::dataDir() ) );
   ASSERT_TRUE( QDir( buffers ).removeRecursively() );
   {
      QFile blocker( buffers );
      ASSERT_TRUE( blocker.open( QIODevice::WriteOnly ) );
   }
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_bufferID", kBufferGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, query( args( { "get_bufferID", kBufferGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, transactionActive() ).WillOnce( Return( false ) );
      EXPECT_CALL( db, beginTransaction( _ ) ).WillOnce( Return( true ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            []( const QStringList& q )
            {
               EXPECT_EQ( ( QStringList{ "new_buffer", kBufferGuid, "Saved buffer" } ), q.mid( 0, 3 ) );
               return IUS_DB2::OK;
            } ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 41 ) );
      EXPECT_CALL( db, commitTransaction( _ ) ).WillOnce( Return( true ) );
      expectNoSolutionRow();
      EXPECT_CALL( db, statusQuery( args( newSolutionArgs() ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 55 ) );
      EXPECT_CALL( db, statusQuery( args( { "new_solutionBuffer", "55", "0", kBufferGuid } ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, statusQuery( args( { "delete_solutionAnalytes", "55" } ) ) ).WillOnce( Return( IUS_DB2::NOROWS ) );
   }
   const int result = solution.saveToDB( 11, 2, &db );
   QFile::remove( buffers );
   ASSERT_EQ( IUS_DB2::OK, result );
   EXPECT_EQ( QString( "41" ), solution.buffer.bufferID );
   EXPECT_EQ( 55, solution.solutionID );
}

// Disk reads, which clear the solution on failure as DB reads do.

TEST_F( SolutionDb, MissingSolutionFileClearsTheSolution )
{
   prepareLoaded();
   QString guid = "99999999-8888-7777-6666-555555555555";
   EXPECT_EQ( IUS_DB2::NO_SOLUTION, solution.readFromDisk( guid ) );
   expectCleared();
}

TEST_F( SolutionDb, SolutionFileWithAMissingBufferClearsTheSolution )
{
   // A solution file on disk whose buffer has no file.
   US_Solution stored;
   stored.solutionGUID    = "99999999-8888-7777-6666-444444444444";
   stored.solutionDesc    = "Stored on disk";
   stored.buffer.GUID     = "99999999-8888-7777-6666-333333333333";
   stored.buffer.bufferID = "5";
   ASSERT_TRUE( stored.saveToDisk() );
   // saveToDisk also writes the buffer file; remove it.
   ASSERT_TRUE( QDir( US_Settings::dataDir() + "/buffers" ).removeRecursively() );
   QString guid = stored.solutionGUID;

   prepareLoaded();
   EXPECT_EQ( IUS_DB2::NO_BUFFER, solution.readFromDisk( guid ) );
   expectCleared();
}

// Disk writes, which report failure and then leave saveStatus unchanged.

class SolutionDisk : public QtTestBase
{
   protected:
      US_Solution solution;

      void        SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         ASSERT_TRUE( QDir().mkpath( US_Settings::dataDir() ) );
         removeAll();
         solution.solutionGUID    = "99999999-8888-7777-6666-222222222222";
         solution.solutionDesc    = "Written to disk";
         solution.buffer.GUID     = "99999999-8888-7777-6666-111111111111";
         solution.buffer.bufferID = "5";
      }

      void TearDown() override
      {
         removeAll();
         QtTestBase::TearDown();
      }

      // Removes the solution and buffer directories, or a file standing in for one.
      static void removeAll()
      {
         for ( const QString& name : { QString( "/solutions" ), QString( "/buffers" ) } )
         {
            const QString path = US_Settings::dataDir() + name;
            if ( QFileInfo( path ).isFile() )
            {
               QFile::remove( path );
            }
            QDir( path ).removeRecursively();
         }
      }

      // A file where a data directory goes makes writes under it fail.
      static ::testing::AssertionResult block( const QString& name )
      {
         QFile blocker( US_Settings::dataDir() + name );
         if ( !blocker.open( QIODevice::WriteOnly ) )
         {
            return ::testing::AssertionFailure() << "cannot create " << blocker.fileName().toStdString();
         }
         return ::testing::AssertionSuccess();
      }
};

TEST_F( SolutionDisk, SaveToDiskReportsSuccess )
{
   EXPECT_TRUE( solution.saveToDisk() );
   EXPECT_EQ( US_Solution::HD_ONLY, solution.saveStatus );
   QString filename;
   EXPECT_TRUE( solution.diskFilename( solution.solutionGUID, filename ) );
}

TEST_F( SolutionDisk, SaveToDiskRejectsASolutionWithoutADescription )
{
   solution.solutionDesc.clear();
   EXPECT_FALSE( solution.saveToDisk() );
   EXPECT_EQ( US_Solution::NOT_SAVED, solution.saveStatus );
}

TEST_F( SolutionDisk, SaveToDiskReportsAnUnwritableSolutionFile )
{
   ASSERT_TRUE( block( "/solutions" ) );
   EXPECT_FALSE( solution.saveToDisk() );
   EXPECT_EQ( US_Solution::NOT_SAVED, solution.saveStatus );
}

TEST_F( SolutionDisk, SaveToDiskReportsAFailedBufferCopy )
{
   ASSERT_TRUE( block( "/buffers" ) );
   EXPECT_FALSE( solution.saveToDisk() );
   EXPECT_EQ( US_Solution::NOT_SAVED, solution.saveStatus );
}

TEST( SolutionReadErrorText, NamesTheMissingPartOrUsesTheDbError )
{
   EXPECT_EQ( "the solution was not found", US_Solution::readErrorText( IUS_DB2::NO_SOLUTION, "ignored" ) );
   EXPECT_EQ( "its buffer could not be read", US_Solution::readErrorText( IUS_DB2::NO_BUFFER, "ignored" ) );
   EXPECT_EQ( "one of its analytes could not be read", US_Solution::readErrorText( IUS_DB2::NO_ANALYTE, "ignored" ) );
   EXPECT_EQ( "Lost connection", US_Solution::readErrorText( IUS_DB2::DBERROR, "Lost connection" ) );
   EXPECT_EQ( QString( "database error %1" ).arg( IUS_DB2::DBERROR ), US_Solution::readErrorText( IUS_DB2::DBERROR, "" ) );
}
} // namespace
