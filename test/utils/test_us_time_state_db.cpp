//! \file test_us_time_state_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_time_state.h"
#include "us_util.h"
#include <QDir>
#include <QFile>

using ::testing::AnyOf;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

namespace
{
const QString               kDefs = "<TimeState><time_state_definitions/></TimeState>";

Matcher<const QStringList&> args( const QStringList& expected )
{
   return Matcher<const QStringList&>( Eq( expected ) );
}

void writeFile( const QString& path, const QByteArray& bytes )
{
   QFile file( path );
   ASSERT_TRUE( file.open( QIODevice::WriteOnly ) );
   ASSERT_EQ( bytes.size(), file.write( bytes ) );
}

// The checksum and size columns are read in one expression, in unspecified order.
void expectChecksum( US_DB2_Mock& db, const QString& checksum )
{
   const QString hash = checksum.section( " ", 0, 0 );
   const QString size = checksum.section( " ", 1, 1 );
   EXPECT_CALL( db, value( AnyOf( 3u, 4u ) ) )
      .Times( 2 )
      .WillRepeatedly( Invoke( [hash, size]( unsigned col ) { return QVariant( col == 3 ? hash : size ); } ) );
}

QByteArray readFile( const QString& path )
{
   QFile file( path );
   EXPECT_TRUE( file.open( QIODevice::ReadOnly ) );
   return file.readAll();
}

class TimeStateDb : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock> db;
      QString                 dir;
      QString                 tmstPath;

      void                    SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         dir      = QDir::currentPath();
         tmstPath = dir + "/run.time_state.tmst";
      }

      void writeLocalFiles()
      {
         writeFile( tmstPath, "binary time state" );
         writeFile( dir + "/run.time_state.xml", kDefs.toUtf8() );
      }

      QString localChecksum()
      {
         return US_Util::md5sum_file( tmstPath );
      }

      // A get_experiment_timestate row with the columns dbCreate requests.
      void expectRecord( const QString& id, const QString& fname, const QString& defs, const QString& checksum )
      {
         EXPECT_CALL( db, query( args( { "get_experiment_timestate", "7" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( id ) ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( fname ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( defs ) ) );
         expectChecksum( db, checksum );
      }

      void expectNoRecord()
      {
         EXPECT_CALL( db, query( args( { "get_experiment_timestate", "7" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      }

      void expectInsert( int status )
      {
         EXPECT_CALL( db, statusQuery( args( { "new_timestate", "7", "run.time_state.tmst", kDefs } ) ) )
            .WillOnce( Return( status ) );
      }
};

// dbCreate

TEST_F( TimeStateDb, CreateRejectsInvalidArgumentsAndMissingFilesWithoutQueries )
{
   EXPECT_EQ( -1, US_TimeState::dbCreate( nullptr, 7, tmstPath ) );
   EXPECT_EQ( -1, US_TimeState::dbCreate( &db, 0, tmstPath ) );
   EXPECT_EQ( -1, US_TimeState::dbCreate( &db, 7, QString() ) );
   EXPECT_EQ( -2, US_TimeState::dbCreate( &db, 7, tmstPath ) );
   writeFile( tmstPath, "binary time state" );
   EXPECT_EQ( -2, US_TimeState::dbCreate( &db, 7, tmstPath ) );
}

TEST_F( TimeStateDb, CreateInsertsWhenNoRecordExists )
{
   writeLocalFiles();
   for ( int status : { IUS_DB2::OK, IUS_DB2::NOROWS } )
   {
      SCOPED_TRACE( status );
      {
         InSequence sequence;
         expectNoRecord();
         expectInsert( status );
         EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 61 ) );
      }
      EXPECT_EQ( 61, US_TimeState::dbCreate( &db, 7, tmstPath ) );
      ::testing::Mock::VerifyAndClearExpectations( &db );
   }
}

TEST_F( TimeStateDb, CreateReportsFailedInsert )
{
   writeLocalFiles();
   {
      InSequence sequence;
      expectNoRecord();
      expectInsert( IUS_DB2::DBERROR );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 0 ) );
   }
   EXPECT_EQ( -4, US_TimeState::dbCreate( &db, 7, tmstPath ) );
}

TEST_F( TimeStateDb, CreateFailedLookupWritesNothing )
{
   writeLocalFiles();
   for ( int status : { IUS_DB2::DBERROR, IUS_DB2::NOT_CONNECTED } )
   {
      SCOPED_TRACE( status );
      {
         InSequence sequence;
         EXPECT_CALL( db, query( args( { "get_experiment_timestate", "7" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
      }
      EXPECT_EQ( -4, US_TimeState::dbCreate( &db, 7, tmstPath ) );
      ::testing::Mock::VerifyAndClearExpectations( &db );
   }
}

TEST_F( TimeStateDb, CreateKeepsAMatchingRecord )
{
   writeLocalFiles();
   {
      InSequence sequence;
      expectRecord( "44", "run.time_state.tmst", kDefs, localChecksum() );
   }
   EXPECT_EQ( 44, US_TimeState::dbCreate( &db, 7, tmstPath ) );
}

TEST_F( TimeStateDb, CreateRenamesAMatchingRecordUnderItsOwnId )
{
   writeLocalFiles();
   for ( int status : { IUS_DB2::OK, IUS_DB2::DBERROR } )
   {
      SCOPED_TRACE( status );
      {
         InSequence sequence;
         expectRecord( "44", "old.time_state.tmst", kDefs, localChecksum() );
         EXPECT_CALL( db, statusQuery( args( { "update_timestate", "44", "7", "run.time_state.tmst", kDefs } ) ) )
            .WillOnce( Return( status ) );
      }
      EXPECT_EQ( status == IUS_DB2::OK ? 44 : -4, US_TimeState::dbCreate( &db, 7, tmstPath ) );
      ::testing::Mock::VerifyAndClearExpectations( &db );
   }
}

TEST_F( TimeStateDb, CreateReplacesAMismatchedRecord )
{
   writeLocalFiles();
   for ( const QString& defs : { kDefs, QString( "<TimeState/>" ) } )
   {
      SCOPED_TRACE( defs.toStdString() );
      const QString checksum = defs == kDefs ? QString( "0123abcd 17" ) : localChecksum();
      {
         InSequence sequence;
         expectRecord( "44", "run.time_state.tmst", defs, checksum );
         EXPECT_CALL( db, statusQuery( args( { "delete_timestate", "44" } ) ) ).WillOnce( Return( IUS_DB2::OK ) );
         expectInsert( IUS_DB2::OK );
         EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 62 ) );
      }
      EXPECT_EQ( 62, US_TimeState::dbCreate( &db, 7, tmstPath ) );
      ::testing::Mock::VerifyAndClearExpectations( &db );
   }
}

TEST_F( TimeStateDb, CreateFailedDeleteInsertsNothing )
{
   writeLocalFiles();
   {
      InSequence sequence;
      expectRecord( "44", "run.time_state.tmst", kDefs, "0123abcd 17" );
      EXPECT_CALL( db, statusQuery( args( { "delete_timestate", "44" } ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( -4, US_TimeState::dbCreate( &db, 7, tmstPath ) );
}

// dbExamine

TEST_F( TimeStateDb, ExamineByTimestateIdReturnsEveryColumn )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_timestate", "44" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "7" ) ) );
      EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "run.time_state.tmst" ) ) );
      EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( kDefs ) ) );
      expectChecksum( db, "0123abcd 17" );
      EXPECT_CALL( db, value( 5 ) ).WillOnce( Return( QVariant( "2026-11-14T12:30:05Z" ) ) );
   }
   int       tmstID = 44, expID = 0;
   QString   fname, defs, checksum;
   QDateTime updated;
   EXPECT_EQ( IUS_DB2::OK, US_TimeState::dbExamine( &db, &tmstID, &expID, &fname, &defs, &checksum, &updated ) );
   EXPECT_EQ( 44, tmstID );
   EXPECT_EQ( 7, expID );
   EXPECT_EQ( QString( "run.time_state.tmst" ), fname );
   EXPECT_EQ( kDefs, defs );
   EXPECT_EQ( QString( "0123abcd 17" ), checksum );
   EXPECT_TRUE( updated.isValid() );
   EXPECT_EQ( Q_INT64_C( 1794659405 ), updated.toSecsSinceEpoch() ); // 2026-11-14T12:30:05Z
}

TEST_F( TimeStateDb, ExamineByExperimentIdReturnsTheTimestateId )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_experiment_timestate", "7" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "44" ) ) );
      expectChecksum( db, "0123abcd 17" );
   }
   int     tmstID = 0, expID = 7;
   QString checksum;
   EXPECT_EQ( IUS_DB2::OK, US_TimeState::dbExamine( &db, &tmstID, &expID, nullptr, nullptr, &checksum ) );
   EXPECT_EQ( 44, tmstID );
   EXPECT_EQ( QString( "0123abcd 17" ), checksum );
}

TEST_F( TimeStateDb, ExamineFailureOrMissingRowChangesNoOutputs )
{
   struct Case
   {
         int     tmstID;
         int     expID;
         QString query;
         QString id;
   };
   for ( const Case& c : { Case{ 44, 0, "get_timestate", "44" }, Case{ 0, 7, "get_experiment_timestate", "7" } } )
   {
      for ( int status : { IUS_DB2::NOROWS, IUS_DB2::DBERROR, IUS_DB2::OK } )
      {
         SCOPED_TRACE( c.query.toStdString() + " " + std::to_string( status ) );
         {
            InSequence sequence;
            EXPECT_CALL( db, query( args( { c.query, c.id } ) ) );
            EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
            if ( status == IUS_DB2::OK )
            {
               EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
            }
         }
         int     tmstID = c.tmstID, expID = c.expID;
         QString fname = "stale", checksum = "stale";
         EXPECT_EQ( status == IUS_DB2::OK ? IUS_DB2::NOROWS : status,
                    US_TimeState::dbExamine( &db, &tmstID, &expID, &fname, nullptr, &checksum ) );
         EXPECT_EQ( c.tmstID, tmstID );
         EXPECT_EQ( c.expID, expID );
         EXPECT_EQ( QString( "stale" ), fname );
         EXPECT_EQ( QString( "stale" ), checksum );
         ::testing::Mock::VerifyAndClearExpectations( &db );
      }
   }
}

TEST_F( TimeStateDb, ExamineWithoutIdsQueriesNothing )
{
   EXPECT_EQ( IUS_DB2::NO_EXPERIMENT, US_TimeState::dbExamine( &db, nullptr, nullptr ) );
   int tmstID = 0, expID = 0;
   EXPECT_EQ( IUS_DB2::NO_EXPERIMENT, US_TimeState::dbExamine( &db, &tmstID, &expID ) );
}

// Delete, upload and download

TEST_F( TimeStateDb, DeleteUploadAndDownloadPassThroughStatus )
{
   EXPECT_CALL( db, statusQuery( args( { "delete_timestate", "44" } ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
   EXPECT_EQ( IUS_DB2::DBERROR, US_TimeState::dbDelete( &db, 44 ) );
   EXPECT_CALL( db, writeBlobToDB( Eq( tmstPath ), Eq( QString( "upload_timestate" ) ), Eq( 44 ) ) )
      .WillOnce( Return( IUS_DB2::OK ) );
   EXPECT_EQ( IUS_DB2::OK, US_TimeState::dbUpload( &db, 44, tmstPath ) );
   EXPECT_CALL( db, readBlobFromDB( Eq( tmstPath ), Eq( QString( "download_timestate" ) ), Eq( 44 ) ) )
      .WillOnce( Return( IUS_DB2::NOROWS ) );
   EXPECT_EQ( IUS_DB2::NOROWS, US_TimeState::dbDownload( &db, 44, tmstPath ) );
}

// dbSyncToLF

class TimeStateSync : public TimeStateDb
{
   protected:
      void expectSyncRecord( const QString& checksum )
      {
         EXPECT_CALL( db, query( args( { "get_experiment_timestate", "7" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "44" ) ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "run.time_state.tmst" ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( kDefs ) ) );
         expectChecksum( db, checksum );
         EXPECT_CALL( db, value( 5 ) ).WillOnce( Return( QVariant( "2026-11-14T12:30:05Z" ) ) );
      }

      QString defsPath() const
      {
         return dir + "/run.time_state.xml";
      }
};

TEST_F( TimeStateSync, WithoutDatabaseCreatesNothing )
{
   EXPECT_FALSE( US_TimeState::dbSyncToLF( nullptr, tmstPath, 7 ) );
   EXPECT_FALSE( QFile::exists( defsPath() ) );
}

TEST_F( TimeStateSync, MissingRecordDownloadsNothing )
{
   {
      InSequence sequence;
      expectNoRecord();
   }
   EXPECT_FALSE( US_TimeState::dbSyncToLF( &db, tmstPath, 7 ) );
   EXPECT_FALSE( QFile::exists( defsPath() ) );
}

TEST_F( TimeStateSync, MissingLocalFileIsDownloadedWithItsDefinitions )
{
   {
      InSequence sequence;
      expectSyncRecord( "0123abcd 17" );
      EXPECT_CALL( db, readBlobFromDB( Eq( tmstPath ), Eq( QString( "download_timestate" ) ), Eq( 44 ) ) )
         .WillOnce( Return( IUS_DB2::OK ) );
   }
   EXPECT_TRUE( US_TimeState::dbSyncToLF( &db, tmstPath, 7 ) );
   EXPECT_EQ( kDefs.toUtf8(), readFile( defsPath() ) );
}

TEST_F( TimeStateSync, FailedDownloadWritesNoDefinitions )
{
   {
      InSequence sequence;
      expectSyncRecord( "0123abcd 17" );
      EXPECT_CALL( db, readBlobFromDB( Eq( tmstPath ), Eq( QString( "download_timestate" ) ), Eq( 44 ) ) )
         .WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_FALSE( US_TimeState::dbSyncToLF( &db, tmstPath, 7 ) );
   EXPECT_FALSE( QFile::exists( defsPath() ) );
}

TEST_F( TimeStateSync, MatchingLocalFileIsNotDownloaded )
{
   writeFile( tmstPath, "binary time state" );
   {
      InSequence sequence;
      expectSyncRecord( localChecksum() );
   }
   EXPECT_FALSE( US_TimeState::dbSyncToLF( &db, tmstPath, 7 ) );
   EXPECT_FALSE( QFile::exists( defsPath() ) );
   EXPECT_EQ( QByteArray( "binary time state" ), readFile( tmstPath ) );
}

TEST_F( TimeStateSync, LargerDatabaseRecordReplacesAnOlderLocalFile )
{
   writeFile( tmstPath, "short" );
   {
      InSequence sequence;
      expectSyncRecord( "0123abcd 1000" );
      EXPECT_CALL( db, readBlobFromDB( Eq( tmstPath ), Eq( QString( "download_timestate" ) ), Eq( 44 ) ) )
         .WillOnce( Return( IUS_DB2::OK ) );
   }
   EXPECT_TRUE( US_TimeState::dbSyncToLF( &db, tmstPath, 7 ) );
   EXPECT_EQ( kDefs.toUtf8(), readFile( defsPath() ) );
}
} // namespace
