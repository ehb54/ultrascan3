//! \file test_us_experiment_save_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_experiment.h"
#include "us_settings.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

// Not covered: readFromDB (see test_us_experiment_db_failures.cpp) and the RI profile XML.

namespace
{
const QString               kProjectGuid    = "22222222-aaaa-bbbb-cccc-000000000001";
const QString               kExperimentGuid = "22222222-aaaa-bbbb-cccc-000000000002";

Matcher<const QStringList&> args( const QStringList& expected )
{
   return Matcher<const QStringList&>( Eq( expected ) );
}

class ExperimentSaveDb : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock>                        db;
      US_Experiment                                  experiment;
      QVector<US_SimulationParameters::SpeedProfile> steps;

      void                                           SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         US_Settings::set_us_inv_ID( 23 );
         EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
         experiment.project.projectGUID = kProjectGuid;
         experiment.project.projectDesc = "Project A";
         experiment.expGUID             = kExperimentGuid;
         experiment.runID               = "run-1";
         experiment.labID               = 3;
         experiment.instrumentID        = 5;
         experiment.operatorID          = 8;
         experiment.rotorID             = 6;
         experiment.calibrationID       = 10;
         experiment.expType             = "velocity";
         experiment.opticalSystem       = "UV/visible";
         experiment.runTemp             = "20";
         experiment.label               = "Run 1";
         experiment.comments            = "comments";
         experiment.protocolGUID        = "proto-guid";
         US_SimulationParameters::SpeedProfile step;
         step.rotorspeed = 45000;
         steps << step;
      }

      // The project already stored with ID 5.
      void expectProjectSaved()
      {
         EXPECT_CALL( db, query( args( { "get_projectID_from_GUID", kProjectGuid } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "5" ) ) );
         EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      }

      void expectRunLookup( int status, bool row )
      {
         EXPECT_CALL( db, query( args( { "get_experiment_info_by_runID", "run-1", "23" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
         if ( status == IUS_DB2::OK )
         {
            EXPECT_CALL( db, next() ).WillOnce( Return( row ) );
            if ( row )
            {
               EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "44" ) ) );
            }
         }
      }

      void expectWrite( const QString& procedure, QStringList& written, int status )
      {
         EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
            .WillOnce( Invoke(
               [procedure, &written, status]( const QStringList& q )
               {
                  EXPECT_EQ( procedure, q.value( 0 ) );
                  written = q;
                  return status;
               } ) );
      }

      void expectInfoAndStepDelete( int deleteStatus )
      {
         EXPECT_CALL( db, query( args( { "get_experiment_info", "44" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 12 ) ).WillOnce( Return( QVariant( "2026-11-14 10:00:00" ) ) );
         EXPECT_CALL( db, statusQuery( args( { "delete_speedsteps", "44" } ) ) ).WillOnce( Return( deleteStatus ) );
      }

      int save( bool update )
      {
         return experiment.saveToDB( update, &db, steps );
      }
};

TEST_F( ExperimentSaveDb, SaveCreatesTheExperimentAndItsSpeedSteps )
{
   QStringList written;
   QStringList step;
   {
      InSequence sequence;
      expectProjectSaved();
      expectRunLookup( IUS_DB2::NOROWS, false );
      expectWrite( "new_experiment", written, IUS_DB2::OK );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 44 ) );
      expectInfoAndStepDelete( IUS_DB2::NOROWS );
      expectWrite( "new_speedstep", step, IUS_DB2::OK );
   }
   ASSERT_EQ( IUS_DB2::OK, save( false ) );
   EXPECT_EQ( 44, experiment.expID );
   EXPECT_EQ( QString( "2026-11-14 10:00:00" ), experiment.date );
   EXPECT_EQ(
      ( QStringList{ "new_experiment", kExperimentGuid, "5", "run-1", "3", "5", "8", "6", "10", "velocity", "UV/visible" } ),
      written.mid( 0, 11 ) );
   EXPECT_EQ( ( QStringList{ "20", "Run 1", "comments", "proto-guid", "23" } ), written.mid( 12 ) );
   EXPECT_EQ( QString( "44" ), step.value( 1 ) );
   EXPECT_EQ( QString( "45000" ), step.value( 7 ) );
}

TEST_F( ExperimentSaveDb, SaveUpdatesAStoredRunOnlyWhenAllowed )
{
   {
      InSequence sequence;
      expectProjectSaved();
      expectRunLookup( IUS_DB2::OK, true );
   }
   EXPECT_EQ( IUS_DB2::DUPFIELD, save( false ) );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );

   QStringList written;
   QStringList step;
   {
      InSequence sequence;
      expectProjectSaved();
      expectRunLookup( IUS_DB2::OK, true );
      expectWrite( "update_experiment", written, IUS_DB2::OK );
      expectInfoAndStepDelete( IUS_DB2::OK );
      expectWrite( "new_speedstep", step, IUS_DB2::OK );
   }
   ASSERT_EQ( IUS_DB2::OK, save( true ) );
   EXPECT_EQ( ( QStringList{ "update_experiment", "44", kExperimentGuid, "5", "run-1" } ), written.mid( 0, 5 ) );
}

TEST_F( ExperimentSaveDb, FailedRunLookupWritesNothing )
{
   {
      InSequence sequence;
      expectProjectSaved();
      expectRunLookup( IUS_DB2::DBERROR, false );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, save( true ) );
   EXPECT_EQ( 0, experiment.expID );
}

TEST_F( ExperimentSaveDb, FailedExperimentWriteStops )
{
   for ( const bool stored : { true, false } )
   {
      SCOPED_TRACE( stored );
      QStringList written;
      {
         InSequence sequence;
         expectProjectSaved();
         expectRunLookup( stored ? IUS_DB2::OK : IUS_DB2::NOROWS, stored );
         expectWrite( stored ? "update_experiment" : "new_experiment", written, IUS_DB2::DBERROR );
      }
      EXPECT_CALL( db, lastInsertID() ).Times( 0 );
      EXPECT_EQ( IUS_DB2::DBERROR, save( true ) );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
      EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
   }
}

TEST_F( ExperimentSaveDb, MissingSavedExperimentInfoStopsBeforeSpeedSteps )
{
   QStringList written;
   {
      InSequence sequence;
      expectProjectSaved();
      expectRunLookup( IUS_DB2::NOROWS, false );
      expectWrite( "new_experiment", written, IUS_DB2::OK );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 44 ) );
      EXPECT_CALL( db, query( args( { "get_experiment_info", "44" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
   }
   EXPECT_EQ( IUS_DB2::NO_EXPERIMENT, save( false ) );
}

TEST_F( ExperimentSaveDb, FailedSpeedStepDeleteOrWriteIsReported )
{
   QStringList written;
   {
      InSequence sequence;
      expectProjectSaved();
      expectRunLookup( IUS_DB2::OK, true );
      expectWrite( "update_experiment", written, IUS_DB2::OK );
      expectInfoAndStepDelete( IUS_DB2::DBERROR );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, save( true ) );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );

   QStringList step;
   {
      InSequence sequence;
      expectProjectSaved();
      expectRunLookup( IUS_DB2::OK, true );
      expectWrite( "update_experiment", written, IUS_DB2::OK );
      expectInfoAndStepDelete( IUS_DB2::OK );
      expectWrite( "new_speedstep", step, IUS_DB2::DBERROR );
   }
   EXPECT_CALL( db, lastInsertID() ).Times( 0 );
   EXPECT_EQ( IUS_DB2::DBERROR, save( true ) );
}

TEST_F( ExperimentSaveDb, CheckRunIdDistinguishesMissingRunsFromErrors )
{
   for ( int status : { IUS_DB2::NOROWS, IUS_DB2::DBERROR, IUS_DB2::OK } )
   {
      SCOPED_TRACE( status );
      {
         InSequence sequence;
         expectRunLookup( status, false );
      }
      experiment.expID = 9;
      EXPECT_EQ( status == IUS_DB2::DBERROR ? IUS_DB2::DBERROR : IUS_DB2::NOROWS, experiment.checkRunID( &db ) );
      EXPECT_EQ( 0, experiment.expID );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
      EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
   }
   EXPECT_EQ( IUS_DB2::NOT_CONNECTED, experiment.checkRunID( nullptr ) );
}
} // namespace
