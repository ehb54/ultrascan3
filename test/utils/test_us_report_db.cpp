//! \file test_us_report_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_report.h"
#include "us_settings.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

namespace
{
const QString               kReportGuid   = "11111111-aaaa-bbbb-cccc-000000000001";
const QString               kTripleGuid   = "11111111-aaaa-bbbb-cccc-000000000002";
const QString               kDocumentGuid = "11111111-aaaa-bbbb-cccc-000000000003";

Matcher<const QStringList&> args( const QStringList& expected )
{
   return Matcher<const QStringList&>( Eq( expected ) );
}

class ReportDb : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock> db;
      const QString           dir = "/reports";

      void                    SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         US_Settings::set_us_inv_ID( 23 );
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

      // A write sent through query() whose status is then read.
      void expectWrite( const QStringList& query, int status )
      {
         EXPECT_CALL( db, query( args( query ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
      }

      static US_Report::ReportDocument document()
      {
         US_Report::ReportDocument d;
         d.documentID   = 30;
         d.documentGUID = kDocumentGuid;
         d.editedDataID = 4;
         d.label        = "Residuals";
         d.filename     = "2DSA.residuals.png";
         d.analysis     = "2DSA";
         d.subAnalysis  = "residuals";
         d.documentType = "png";
         return d;
      }

      static QStringList documentFields()
      {
         return { "4", "Residuals", "2DSA.residuals.png", "2DSA", "residuals", "png" };
      }

      void expectDocumentRow()
      {
         const QStringList columns{ kDocumentGuid, "4", "Residuals", "2DSA.residuals.png", "2DSA", "residuals", "png" };
         for ( unsigned col = 0; col < 7; ++col )
         {
            EXPECT_CALL( db, value( col ) ).WillOnce( Return( QVariant( columns[col] ) ) );
         }
      }

      static void expectDocumentUnchanged( const US_Report::ReportDocument& d )
      {
         EXPECT_EQ( 30, d.documentID );
         EXPECT_EQ( QString( "previous label" ), d.label );
         EXPECT_EQ( QString( "previous.png" ), d.filename );
      }

      static US_Report::ReportDocument previousDocument()
      {
         US_Report::ReportDocument d;
         d.documentID = 30;
         d.label      = "previous label";
         d.filename   = "previous.png";
         return d;
      }
};

// Documents

TEST_F( ReportDb, DocumentSaveInsertsANewDocumentAndUploadsIt )
{
   for ( const bool emptyLookup : { false, true } )
   {
      SCOPED_TRACE( emptyLookup );
      US_Report::ReportDocument d = document();
      {
         InSequence sequence;
         expectQuery( { "get_reportDocumentID", kDocumentGuid }, emptyLookup ? IUS_DB2::OK : IUS_DB2::NOROWS, false );
         expectWrite( QStringList{ "new_reportDocument", "20", kDocumentGuid } + documentFields(), IUS_DB2::OK );
         EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 31 ) );
         EXPECT_CALL(
            db,
            writeBlobToDB( Eq( QString( "/reports/2DSA.residuals.png" ) ), Eq( QString( "upload_reportContents" ) ), Eq( 31 ) ) )
            .WillOnce( Return( IUS_DB2::OK ) );
      }
      EXPECT_EQ( US_Report::REPORT_OK, d.saveDB( 20, dir, &db ) );
      EXPECT_EQ( 31, d.documentID );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( ReportDb, DocumentSaveUpdatesTheStoredDocument )
{
   US_Report::ReportDocument d = document();
   d.documentID                = -1;
   {
      InSequence sequence;
      expectQuery( { "get_reportDocumentID", kDocumentGuid }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "30" ) ) );
      expectWrite( QStringList{ "update_reportDocument", "30" } + documentFields(), IUS_DB2::OK );
      EXPECT_CALL( db, writeBlobToDB( _, Eq( QString( "upload_reportContents" ) ), Eq( 30 ) ) ).WillOnce( Return( IUS_DB2::OK ) );
   }
   EXPECT_EQ( US_Report::REPORT_OK, d.saveDB( 20, dir, &db ) );
   EXPECT_EQ( 30, d.documentID );
}

TEST_F( ReportDb, DocumentSaveStopsAtTheFirstFailure )
{
   US_Report::ReportDocument d = document();
   {
      InSequence sequence;
      expectQuery( { "get_reportDocumentID", kDocumentGuid }, IUS_DB2::DBERROR, false );
   }
   EXPECT_EQ( US_Report::DB_ERROR, d.saveDB( 20, dir, &db ) );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectQuery( { "get_reportDocumentID", kDocumentGuid }, IUS_DB2::NOROWS, false );
      expectWrite( QStringList{ "new_reportDocument", "20", kDocumentGuid } + documentFields(), IUS_DB2::DBERROR );
   }
   EXPECT_CALL( db, lastError() ).WillRepeatedly( Return( QString() ) );
   EXPECT_EQ( US_Report::DB_ERROR, d.saveDB( 20, dir, &db ) );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectQuery( { "get_reportDocumentID", kDocumentGuid }, IUS_DB2::NOROWS, false );
      expectWrite( QStringList{ "new_reportDocument", "20", kDocumentGuid } + documentFields(), IUS_DB2::OK );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 31 ) );
      EXPECT_CALL( db, writeBlobToDB( _, _, Eq( 31 ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_CALL( db, lastError() ).WillRepeatedly( Return( QString() ) );
   EXPECT_EQ( US_Report::DB_ERROR, d.saveDB( 20, dir, &db ) );
}

TEST_F( ReportDb, DocumentReadMapsColumnsAndDownloadsTheContents )
{
   US_Report::ReportDocument d;
   d.documentID = 30;
   {
      InSequence sequence;
      expectQuery( { "get_reportDocument_info", "30" }, IUS_DB2::OK, true );
      expectDocumentRow();
      EXPECT_CALL(
         db,
         readBlobFromDB( Eq( QString( "/reports/2DSA.residuals.png" ) ), Eq( QString( "download_reportContents" ) ), Eq( 30 ) ) )
         .WillOnce( Return( IUS_DB2::OK ) );
   }
   ASSERT_EQ( US_Report::REPORT_OK, d.readDB( dir, &db ) );
   EXPECT_EQ( 30, d.documentID );
   EXPECT_EQ( kDocumentGuid, d.documentGUID );
   EXPECT_EQ( 4, d.editedDataID );
   EXPECT_EQ( QString( "Residuals" ), d.label );
   EXPECT_EQ( QString( "png" ), d.documentType );
}

TEST_F( ReportDb, DocumentReadFailuresKeepTheDocument )
{
   for ( int step = 0; step < 3; ++step )
   {
      SCOPED_TRACE( step );
      US_Report::ReportDocument d = previousDocument();
      US_Report::Status         expected;
      {
         InSequence sequence;
         if ( step == 0 )
         {
            expectQuery( { "get_reportDocument_info", "30" }, IUS_DB2::OK, false );
            expected = US_Report::NOT_FOUND;
         }
         else if ( step == 1 )
         {
            expectQuery( { "get_reportDocument_info", "30" }, IUS_DB2::DBERROR, false );
            expected = US_Report::DB_ERROR;
         }
         else
         {
            expectQuery( { "get_reportDocument_info", "30" }, IUS_DB2::OK, true );
            expectDocumentRow();
            EXPECT_CALL( db, readBlobFromDB( _, _, Eq( 30 ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
            expected = US_Report::DB_ERROR;
         }
      }
      EXPECT_EQ( expected, d.readDB( dir, &db ) );
      expectDocumentUnchanged( d );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

// Triples

TEST_F( ReportDb, TripleSaveUpdatesByTheStoredIdOrInsertsANewTriple )
{
   US_Report::ReportTriple t;
   t.tripleID        = -1; // not yet known in memory
   t.tripleGUID      = kTripleGuid;
   t.triple          = "1/A/280";
   t.dataDescription = "description";
   {
      InSequence sequence;
      expectQuery( { "get_reportTripleID", kTripleGuid }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "21" ) ) );
      expectWrite( { "update_reportTriple", "21", "-1", "1/A/280", "description" }, IUS_DB2::OK );
   }
   EXPECT_EQ( US_Report::REPORT_OK, t.saveDB( 10, &db ) );
   EXPECT_EQ( 21, t.tripleID );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectQuery( { "get_reportTripleID", kTripleGuid }, IUS_DB2::OK, false );
      expectWrite( { "new_reportTriple", kTripleGuid, "10", "-1", "1/A/280", "description" }, IUS_DB2::OK );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 22 ) );
   }
   EXPECT_EQ( US_Report::REPORT_OK, t.saveDB( 10, &db ) );
   EXPECT_EQ( 22, t.tripleID );
}

TEST_F( ReportDb, TripleDocumentListMapsColumnsAndReportsFailure )
{
   US_Report::ReportTriple t;
   t.tripleID = 21;
   {
      InSequence sequence;
      expectQuery( { "get_reportDocument_desc", "21" }, IUS_DB2::OK, true );
      const QStringList columns{ "30", kDocumentGuid, "4", "Residuals", "2DSA.residuals.png", "2DSA", "residuals", "png" };
      for ( unsigned col = 0; col < 8; ++col )
      {
         EXPECT_CALL( db, value( col ) ).WillOnce( Return( QVariant( columns[col] ) ) );
      }
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
   }
   ASSERT_EQ( US_Report::REPORT_OK, t.readDocsDB( &db ) );
   ASSERT_EQ( 1, t.docs.size() );
   EXPECT_EQ( 30, t.docs[0].documentID );
   EXPECT_EQ( QString( "png" ), t.docs[0].documentType );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectQuery( { "get_reportDocument_desc", "21" }, IUS_DB2::DBERROR, false );
   }
   EXPECT_EQ( US_Report::DB_ERROR, t.readDocsDB( &db ) );
}

TEST_F( ReportDb, ReplacingADocumentStopsWhenTheOldOneCannotBeDeleted )
{
   US_Report::ReportTriple t;
   t.tripleID = 21;
   t.docs << document();
   expectWrite( { "delete_reportDocument", "30" }, IUS_DB2::DBERROR );
   EXPECT_EQ( US_Report::DB_ERROR, t.addDocument( document(), dir, &db ) );
   ASSERT_EQ( 1, t.docs.size() );
   EXPECT_EQ( 30, t.docs[0].documentID );
}

// Whole report

class ReportLoad : public ReportDb
{
   protected:
      US_Report report;

      void      SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( ReportDb::SetUp() );
         report.ID    = 9;
         report.runID = "previous-run";
         report.title = "previous title";
         US_Report::ReportTriple t;
         t.triple = "previous triple";
         report.triples << t;
      }

      void expectReportUnchanged()
      {
         EXPECT_EQ( 9, report.ID );
         EXPECT_EQ( QString( "previous-run" ), report.runID );
         EXPECT_EQ( QString( "previous title" ), report.title );
         ASSERT_EQ( 1, report.triples.size() );
         EXPECT_EQ( QString( "previous triple" ), report.triples[0].triple );
      }

      void expectReportRow()
      {
         expectQuery( { "get_report_info_by_runID", "23", "run-1" }, IUS_DB2::OK, true );
         const QStringList columns{ "10", kReportGuid, "44", "run-1", "Run 1 Report", "<p>html</p>" };
         for ( unsigned col = 0; col < 6; ++col )
         {
            EXPECT_CALL( db, value( col ) ).WillOnce( Return( QVariant( columns[col] ) ) );
         }
      }

      void expectTripleRow()
      {
         expectQuery( { "get_reportTriple_desc", "10" }, IUS_DB2::OK, true );
         const QStringList columns{ "21", kTripleGuid, "-1", "1/A/280", "description" };
         for ( unsigned col = 0; col < 5; ++col )
         {
            EXPECT_CALL( db, value( col ) ).WillOnce( Return( QVariant( columns[col] ) ) );
         }
         EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
      }
};

TEST_F( ReportLoad, ReadMapsTheReportAndItsTriples )
{
   {
      InSequence sequence;
      expectReportRow();
      expectTripleRow();
      expectQuery( { "get_reportDocument_desc", "21" }, IUS_DB2::NOROWS, false );
   }
   ASSERT_EQ( US_Report::REPORT_OK, report.readDB( "run-1", &db ) );
   EXPECT_EQ( 10, report.ID );
   EXPECT_EQ( kReportGuid, report.GUID );
   EXPECT_EQ( 44, report.experimentID );
   EXPECT_EQ( QString( "Run 1 Report" ), report.title );
   ASSERT_EQ( 1, report.triples.size() );
   EXPECT_EQ( 21, report.triples[0].tripleID );
   EXPECT_EQ( QString( "1/A/280" ), report.triples[0].triple );
   EXPECT_TRUE( report.triples[0].docs.isEmpty() );
}

TEST_F( ReportLoad, ReadFailuresKeepTheReport )
{
   for ( int step = 0; step < 3; ++step )
   {
      SCOPED_TRACE( step );
      US_Report::Status expected = US_Report::DB_ERROR;
      {
         InSequence sequence;
         if ( step == 0 )
         {
            expectQuery( { "get_report_info_by_runID", "23", "run-1" }, IUS_DB2::OK, false );
            expected = US_Report::NOT_FOUND;
         }
         else
         {
            expectReportRow();
            if ( step == 1 )
            {
               expectQuery( { "get_reportTriple_desc", "10" }, IUS_DB2::DBERROR, false );
            }
            else
            {
               expectTripleRow();
               expectQuery( { "get_reportDocument_desc", "21" }, IUS_DB2::DBERROR, false );
            }
         }
      }
      EXPECT_EQ( expected, report.readDB( "run-1", &db ) );
      expectReportUnchanged();
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( ReportLoad, AutoReadUsesTheGivenInvestigator )
{
   {
      InSequence sequence;
      expectQuery( { "get_report_info_by_runID", "77", "run-1" }, IUS_DB2::NOROWS, false );
   }
   EXPECT_EQ( US_Report::NOT_FOUND, report.readDB_auto( 77, "run-1", &db ) );
   expectReportUnchanged();
}

TEST_F( ReportLoad, SavingDocumentsStopsWhenAChangedTripleCannotBeSaved )
{
   for ( const bool oneFile : { true, false } )
   {
      SCOPED_TRACE( oneFile );
      US_Report stored;
      {
         InSequence sequence;
         expectReportRow();
         expectTripleRow();
         expectQuery( { "get_reportDocument_desc", "21" }, IUS_DB2::NOROWS, false );
         // The new data description is written to the stored triple first.
         expectQuery( { "get_reportTripleID", kTripleGuid }, IUS_DB2::DBERROR, false );
      }
      // No document may be written after the triple failed.
      const US_Report::Status status =
         oneFile ? stored.saveDocumentFromFile( "/results/run-1", "2DSA.1A280.residuals.png", &db, 4, "changed" )
                 : stored.saveFileDocuments( "/results/run-1", { "/results/run-1/2DSA.1A280.residuals.png" }, &db, 4, "changed" );
      EXPECT_EQ( US_Report::DB_ERROR, status );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( ReportDb, SaveCreatesOrUpdatesTheReportAndReadsItsExperiment )
{
   US_Report report;
   report.GUID  = kReportGuid;
   report.runID = "run-1";
   QStringList written;
   {
      InSequence sequence;
      expectQuery( { "get_reportID_by_runID", "23", "run-1" }, IUS_DB2::OK, false );
      EXPECT_CALL( db, query( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke( [&written]( const QStringList& q ) { written = q; } ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 10 ) );
      expectQuery( { "get_report_info", "10" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "44" ) ) );
   }
   ASSERT_EQ( US_Report::REPORT_OK, report.saveDB( &db ) );
   EXPECT_EQ( ( QStringList{ "new_report", kReportGuid, "run-1", "run-1 Report" } ), written.mid( 0, 4 ) );
   EXPECT_TRUE( written.value( 4 ).startsWith( "<p>Report created " ) );
   EXPECT_EQ( QString( "23" ), written.value( 5 ) );
   EXPECT_EQ( 10, report.ID );
   EXPECT_EQ( 44, report.experimentID );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectQuery( { "get_reportID_by_runID", "23", "run-1" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "10" ) ) );
      EXPECT_CALL( db, query( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke( [&written]( const QStringList& q ) { written = q; } ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      expectQuery( { "get_report_info", "10" }, IUS_DB2::OK, false );
   }
   EXPECT_CALL( db, lastInsertID() ).Times( 0 );
   EXPECT_EQ( US_Report::DB_ERROR, report.saveDB( &db ) ); // the experiment ID could not be read
   EXPECT_EQ( ( QStringList{ "update_report", "10", "run-1 Report" } ), written.mid( 0, 3 ) );
}

TEST_F( ReportDb, SavingADocumentStopsWhenTheReportCannotBeReadOrCreated )
{
   for ( const bool readFails : { true, false } )
   {
      SCOPED_TRACE( readFails );
      US_Report report;
      report.runID = "run-1"; // callers set the run before saving its documents
      {
         InSequence sequence;
         if ( readFails )
         {
            expectQuery( { "get_report_info_by_runID", "23", "run-1" }, IUS_DB2::DBERROR, false );
         }
         else
         {
            expectQuery( { "get_report_info_by_runID", "23", "run-1" }, IUS_DB2::NOROWS, false );
            expectQuery( { "get_reportID_by_runID", "23", "run-1" }, IUS_DB2::NOROWS, false );
            EXPECT_CALL( db, query( Matcher<const QStringList&>( _ ) ) );
            EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
            // The failure's debug line reads the status once more.
            EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
         }
      }
      EXPECT_CALL( db, lastError() ).WillRepeatedly( Return( QString() ) );
      // Without a report ID no triple or document may be written.
      EXPECT_EQ( US_Report::DB_ERROR, report.saveDocumentFromFile( "/results/run-1", "2DSA.1A280.residuals.png", &db, 4, "data" ) );
      EXPECT_TRUE( report.triples.isEmpty() );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( ReportDb, SavingFileDocumentsStopsWhenTheReportCannotBeRead )
{
   US_Report report;
   report.runID = "run-1";
   // An empty file list is rejected before any DB access.
   EXPECT_EQ( US_Report::MISC_ERROR, report.saveFileDocuments( "/results/run-1", QStringList(), &db, 4, "data" ) );

   {
      InSequence sequence;
      expectQuery( { "get_report_info_by_runID", "23", "run-1" }, IUS_DB2::DBERROR, false );
   }
   EXPECT_EQ( US_Report::DB_ERROR,
              report.saveFileDocuments( "/results/run-1", { "/results/run-1/2DSA.1A280.residuals.png" }, &db, 4, "data" ) );
   EXPECT_TRUE( report.triples.isEmpty() );
}

TEST_F( ReportDb, SaveAllStopsAtTheFirstFailure )
{
   US_Report report;
   report.GUID  = kReportGuid;
   report.runID = "run-1";
   US_Report::ReportTriple t;
   t.tripleGUID = kTripleGuid;
   t.docs << document();
   report.triples << t;
   {
      InSequence sequence;
      expectQuery( { "get_reportID_by_runID", "23", "run-1" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "10" ) ) );
      EXPECT_CALL( db, query( Matcher<const QStringList&>( _ ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      expectQuery( { "get_report_info", "10" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "44" ) ) );
      expectQuery( { "get_reportTripleID", kTripleGuid }, IUS_DB2::DBERROR, false );
   }
   EXPECT_EQ( US_Report::DB_ERROR, report.saveAllToDB( "/reports", &db ) );
}
} // namespace
