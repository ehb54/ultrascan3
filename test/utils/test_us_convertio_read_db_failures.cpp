//! \file test_us_convertio_read_db_failures.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us3_test_io.h"
#include "us_convertio.h"
#include "us_settings.h"
#include <QDir>
#include <QFile>
#include <QMap>
#include <QSet>

using ::testing::_;
using ::testing::An;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;

namespace
{
// Finite query results for the real experiment/download coordinator. This is
// an interaction script, not a database emulator or an AUC parser fixture.
class RunReadDb : public NiceMock<US_DB2_Mock>
{
   public:
      RunReadDb()
      {
         QVariantList experiment;
         for ( int i = 0; i < 17; ++i )
         {
            experiment << QVariant();
         }
         experiment[0]  = 7;
         experiment[1]  = 17;
         experiment[2]  = "11111111-2222-3333-4444-555555555555";
         experiment[7]  = 1;
         experiment[8]  = "Velocity";
         experiment[14] = 19;
         experiment[15] = "RA";
         rows["get_experiment_info_by_runID"] << experiment;
         QVariantList speed;
         for ( int i = 0; i < 16; ++i )
         {
            speed << QVariant( 0 );
         }
         speed[1] = 2;
         speed[6] = 40000;
         rows["all_speedsteps"] << speed;
         queryErrors["get_eprofile"]             = IUS_DB2::NOROWS;
         queryErrors["get_experiment_timestate"] = IUS_DB2::NOROWS;
         ON_CALL( *this, query( An<const QStringList&>() ) )
            .WillByDefault( Invoke(
               [this]( const QStringList& query )
               {
                  const QSet<QString> allowed{ "get_experiment_info_by_runID",
                                               "get_person_info",
                                               "get_project_info",
                                               "get_instrument_info",
                                               "get_rotor_info",
                                               "get_rotor_calibration_info",
                                               "all_speedsteps",
                                               "get_rawDataIDs",
                                               "all_cell_experiments",
                                               "get_rawData",
                                               "get_solution",
                                               "get_eprofile",
                                               "get_experiment_timestate" };
                  procedure = query.value( 0 );
                  EXPECT_TRUE( allowed.contains( procedure ) ) << procedure.toStdString();
                  calls << procedure;
                  if ( procedure == "get_experiment_info_by_runID" )
                  {
                     EXPECT_EQ( query, ( QStringList{ procedure, "run", "19" } ) );
                  }
                  if ( procedure == "get_rawDataIDs" || procedure == "all_speedsteps" )
                  {
                     EXPECT_EQ( query, ( QStringList{ procedure, "17" } ) );
                  }
                  status  = queryErrors.value( procedure, IUS_DB2::OK );
                  pending = status == IUS_DB2::OK ? rows.value( procedure ) : QList<QVariantList>();
                  current.clear();
               } ) );
         ON_CALL( *this, lastErrno() ).WillByDefault( Invoke( [this] { return status; } ) );
         ON_CALL( *this, lastError() )
            .WillByDefault( Invoke( [this] { return status == IUS_DB2::DBERROR ? errorText : QString(); } ) );
         ON_CALL( *this, next() )
            .WillByDefault( Invoke(
               [this]
               {
                  current.clear();
                  if ( pending.isEmpty() )
                  {
                     status = cursorErrors.value( procedure, status );
                     return false;
                  }
                  current = pending.takeFirst();
                  return true;
               } ) );
         ON_CALL( *this, value( _ ) )
            .WillByDefault( Invoke(
               [this]( unsigned col )
               {
                  EXPECT_LT( col, static_cast<unsigned>( current.size() ) ) << procedure.toStdString();
                  return current.value( static_cast<int>( col ) );
               } ) );
      }
      void addRawRecord()
      {
         rows["get_rawDataIDs"] << QVariantList{ 23, QVariant(), "run.RA.1.A.280.auc" };
         rows["all_cell_experiments"] << QVariantList{ QVariant(), QVariant(), "1", "A", 1 };
         rows["get_rawData"] << QVariantList{
            "11111111-2222-3333-4444-555555555555", QVariant(), "run.RA.1.A.280.auc", QVariant(), QVariant(), 0
         };
      }
      QMap<QString, QList<QVariantList>> rows;
      QMap<QString, int>                 queryErrors, cursorErrors;
      QStringList                        calls;
      QString                            errorText = "synthetic database failure";

   private:
      QString             procedure;
      QList<QVariantList> pending;
      QVariantList        current;
      int                 status = IUS_DB2::OK;
};

class ReadDbRunFailure : public QtTestBase
{
   protected:
      void SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         directory = US_Settings::resultDir() + "/run";
         ASSERT_TRUE( us3WriteTestFile( directory + "/local.auc", "previous AUC bytes" ) );
         ASSERT_TRUE( us3WriteTestFile( directory + "/keep.xml", "previous XML bytes" ) );
      }
      void expectOriginalFiles()
      {
         for ( const auto& entry : { qMakePair( QString( "local.auc" ), QByteArray( "previous AUC bytes" ) ),
                                     qMakePair( QString( "keep.xml" ), QByteArray( "previous XML bytes" ) ) } )
         {
            QFile file( directory + "/" + entry.first );
            ASSERT_TRUE( file.open( QIODevice::ReadOnly ) );
            EXPECT_EQ( file.readAll(), entry.second );
         }
      }
      QString read( RunReadDb& db )
      {
         return US_ConvertIO::readDBExperiment( "run", directory, &db, steps, "19" );
      }
      QString                  directory;
      QVector<SP_SPEEDPROFILE> steps;
};
}

TEST_F( ReadDbRunFailure, ExperimentQueryErrorPreservesFilesAndStops )
{
   RunReadDb db;
   db.queryErrors["get_experiment_info_by_runID"] = IUS_DB2::DBERROR;
   EXPECT_CALL( db, next() ).Times( 0 );
   EXPECT_CALL( db, value( _ ) ).Times( 0 );
   EXPECT_CALL( db, readBlobFromDB( _, _, _ ) ).Times( 0 );
   EXPECT_EQ( read( db ), db.errorText );
   EXPECT_EQ( db.calls, ( QStringList{ "get_experiment_info_by_runID" } ) );
   ASSERT_NO_FATAL_FAILURE( expectOriginalFiles() );
}

TEST_F( ReadDbRunFailure, ErrorWithoutDiagnosticStillCannotReportSuccess )
{
   RunReadDb db;
   db.errorText.clear();
   db.queryErrors["get_experiment_info_by_runID"] = IUS_DB2::DBERROR;
   EXPECT_CALL( db, readBlobFromDB( _, _, _ ) ).Times( 0 );
   EXPECT_FALSE( read( db ).isEmpty() );
   ASSERT_NO_FATAL_FAILURE( expectOriginalFiles() );
}

TEST_F( ReadDbRunFailure, MissingExperimentPreservesFilesAndStops )
{
   for ( int status : { IUS_DB2::OK, IUS_DB2::NOROWS } )
   {
      SCOPED_TRACE( status );
      RunReadDb db;
      db.rows["get_experiment_info_by_runID"].clear();
      db.queryErrors["get_experiment_info_by_runID"] = status;
      EXPECT_CALL( db, readBlobFromDB( _, _, _ ) ).Times( 0 );
      EXPECT_EQ( read( db ), QString( "The current run ID is not found in the database." ) );
      EXPECT_EQ( db.calls, ( QStringList{ "get_experiment_info_by_runID" } ) );
      ASSERT_NO_FATAL_FAILURE( expectOriginalFiles() );
   }
}

TEST_F( ReadDbRunFailure, RawListQueryOrCursorErrorPreservesFilesAndStopsBeforeDownload )
{
   for ( bool cursorFailure : { false, true } )
   {
      SCOPED_TRACE( cursorFailure );
      RunReadDb db;
      db.addRawRecord();
      if ( cursorFailure )
      {
         db.cursorErrors["get_rawDataIDs"] = IUS_DB2::DBERROR;
      }
      else
      {
         db.queryErrors["get_rawDataIDs"] = IUS_DB2::DBERROR;
      }
      EXPECT_CALL( db, readBlobFromDB( _, _, _ ) ).Times( 0 );
      EXPECT_EQ( read( db ), db.errorText );
      EXPECT_EQ( db.calls.last(), QString( "get_rawDataIDs" ) );
      ASSERT_NO_FATAL_FAILURE( expectOriginalFiles() );
   }
}

TEST_F( ReadDbRunFailure, EmptyRawListIsDistinctFromAnErrorAndKeepsFiles )
{
   for ( int status : { IUS_DB2::OK, IUS_DB2::NOROWS } )
   {
      RunReadDb db;
      db.queryErrors["get_rawDataIDs"] = status;
      EXPECT_CALL( db, readBlobFromDB( _, _, _ ) ).Times( 0 );
      EXPECT_EQ( read( db ), QString( "There were no auc files found in the database." ) );
      ASSERT_NO_FATAL_FAILURE( expectOriginalFiles() );
   }
}

TEST_F( ReadDbRunFailure, MissingOptionalRiProfileStillLoadsSpeedStepsAndContinues )
{
   RunReadDb db;
   db.rows["get_experiment_info_by_runID"][0][15] = "RI";
   US_Experiment experiment;
   EXPECT_EQ( experiment.readFromDB( "run", &db, steps, "19" ), IUS_DB2::OK );
   EXPECT_TRUE( experiment.RIProfile.isEmpty() );
   ASSERT_EQ( steps.size(), 1 );
   EXPECT_EQ( steps[0].rotorspeed, 40000 );
   EXPECT_EQ( db.calls.last(), QString( "all_speedsteps" ) );
   EXPECT_CALL( db, readBlobFromDB( _, _, _ ) ).Times( 0 );
   EXPECT_EQ( read( db ), QString( "There were no auc files found in the database." ) );
   EXPECT_EQ( db.calls.last(), QString( "get_rawDataIDs" ) );
   ASSERT_NO_FATAL_FAILURE( expectOriginalFiles() );
}

TEST_F( ReadDbRunFailure, SuccessfulReadRemovesOnlyStaleAucsAfterDownload )
{
   RunReadDb db;
   db.addRawRecord();
   EXPECT_CALL( db, readBlobFromDB( directory + "/run.RA.1.A.280.auc", QString( "download_aucData" ), 23 ) )
      .WillOnce( Invoke(
         [this]( const QString& path, const QString&, int )
         {
            expectOriginalFiles();
            return us3WriteTestFile( path, "downloaded blob" ) ? IUS_DB2::OK : IUS_DB2::DBERROR;
         } ) );
   EXPECT_EQ( read( db ), QString() );
   EXPECT_FALSE( QFile::exists( directory + "/local.auc" ) );
   QFile downloaded( directory + "/run.RA.1.A.280.auc" );
   ASSERT_TRUE( downloaded.open( QIODevice::ReadOnly ) );
   EXPECT_EQ( downloaded.readAll(), QByteArray( "downloaded blob" ) );
   QFile keep( directory + "/keep.xml" );
   ASSERT_TRUE( keep.open( QIODevice::ReadOnly ) );
   EXPECT_EQ( keep.readAll(), QByteArray( "previous XML bytes" ) );
}

TEST_F( ReadDbRunFailure, FailedBlobReadDoesNotTriggerStaleFileCleanup )
{
   RunReadDb db;
   db.addRawRecord();
   EXPECT_CALL( db, readBlobFromDB( _, QString( "download_aucData" ), 23 ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
   EXPECT_FALSE( read( db ).isEmpty() );
   EXPECT_EQ( db.calls.last(), QString( "get_rawDataIDs" ) );
   ASSERT_NO_FATAL_FAILURE( expectOriginalFiles() );
}

TEST_F( ReadDbRunFailure, MissingRawMetadataOrCellsReturnsErrorWithoutStaleCleanup )
{
   for ( bool missingCells : { false, true } )
   {
      SCOPED_TRACE( missingCells );
      RunReadDb db;
      db.addRawRecord();
      db.rows[missingCells ? "all_cell_experiments" : "get_rawData"].clear();
      EXPECT_CALL( db, readBlobFromDB( _, QString( "download_aucData" ), 23 ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_FALSE( read( db ).isEmpty() );
      ASSERT_NO_FATAL_FAILURE( expectOriginalFiles() );
   }
}
