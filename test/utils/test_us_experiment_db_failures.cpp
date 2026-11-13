//! \file test_us_experiment_db_failures.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_analyte.h"
#include "us_experiment.h"
#include "us_project.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

TEST( ExperimentDbFailure, MissingExperimentStopsBeforeDependentLookups )
{
   StrictMock<US_DB2_Mock> db;
   US_Experiment           experiment;
   experiment.runID = "previous-run";
   experiment.expID = 73;
   QVector<SP_SPEEDPROFILE> speedsteps;
   const QStringList        query{ "get_experiment_info_by_runID", "missing-run", "17" };
   {
      InSequence sequence;
      EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( query ) ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::NOROWS ) );
   }
   EXPECT_CALL( db, value( _ ) ).Times( 0 );

   EXPECT_EQ( experiment.readFromDB( "missing-run", &db, speedsteps, "17" ), IUS_DB2::NO_EXPERIMENT );
   EXPECT_EQ( experiment.runID, QString( "previous-run" ) );
   EXPECT_EQ( experiment.expID, 73 );
   EXPECT_TRUE( speedsteps.isEmpty() );
   // The strict mock rejects any person/project/rotor or other follow-up query.
}

TEST( ExperimentDbFailure, FailedLookupReturnsTheDatabaseErrorWithoutDependentLookups )
{
   for ( const int status : { IUS_DB2::DBERROR, IUS_DB2::NOT_CONNECTED } )
   {
      SCOPED_TRACE( status );
      StrictMock<US_DB2_Mock> db;
      US_Experiment           experiment;
      experiment.runID = "previous-run";
      experiment.expID = 73;
      QVector<SP_SPEEDPROFILE> speedsteps;
      const QStringList        query{ "get_experiment_info_by_runID", "failed-run", "17" };
      {
         InSequence sequence;
         EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( query ) ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
         EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( status ) );
      }
      EXPECT_CALL( db, value( _ ) ).Times( 0 );

      EXPECT_EQ( experiment.readFromDB( "failed-run", &db, speedsteps, "17" ), status );
      EXPECT_EQ( experiment.runID, QString( "previous-run" ) );
      EXPECT_EQ( experiment.expID, 73 );
      EXPECT_TRUE( speedsteps.isEmpty() );
   }
}

TEST( ExperimentDbFailure, EmptySuccessfulQueryIsNotASuccessfulRead )
{
   StrictMock<US_DB2_Mock> db;
   US_Experiment           experiment;
   experiment.runID = "previous";
   QVector<SP_SPEEDPROFILE> speedsteps;
   const QStringList        query{ "get_experiment_info_by_runID", "empty", "17" };
   EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( query ) ) ) );
   EXPECT_CALL( db, lastErrno() ).WillRepeatedly( Return( IUS_DB2::OK ) );
   EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
   EXPECT_CALL( db, value( _ ) ).Times( 0 );
   EXPECT_EQ( experiment.readFromDB( "empty", &db, speedsteps, "17" ), IUS_DB2::NO_EXPERIMENT );
   EXPECT_EQ( experiment.runID, QString( "previous" ) );
}

TEST( ExperimentDbFailure, QueryFailureStopsBeforeCursorAccess )
{
   for ( int error : { IUS_DB2::NOROWS, IUS_DB2::DBERROR, IUS_DB2::NOT_CONNECTED } )
   {
      SCOPED_TRACE( error );
      StrictMock<US_DB2_Mock>  db;
      US_Experiment            experiment;
      QVector<SP_SPEEDPROFILE> speedsteps;
      const QStringList        query{ "get_experiment_info_by_runID", "failed", "17" };
      EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( query ) ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( error ) );
      EXPECT_CALL( db, next() ).Times( 0 );
      EXPECT_CALL( db, value( _ ) ).Times( 0 );
      EXPECT_EQ( experiment.readFromDB( "failed", &db, speedsteps, "17" ),
                 error == IUS_DB2::NOROWS ? IUS_DB2::NO_EXPERIMENT : error );
   }
}

TEST( ProjectDbFailure, QueryAndCursorFailuresPreservePreviouslyLoadedProject )
{
   for ( bool queryFailure : { false, true } )
   {
      for ( int error : { IUS_DB2::OK, IUS_DB2::NOROWS, IUS_DB2::DBERROR, IUS_DB2::NOT_CONNECTED } )
      {
         if ( queryFailure && error == IUS_DB2::OK )
         {
            continue;
         }
         SCOPED_TRACE( ::testing::Message() << queryFailure << ":" << error );
         StrictMock<US_DB2_Mock> db;
         US_Project              project;
         project.projectID   = 73;
         project.projectGUID = "previous";
         const QStringList query{ "get_project_info", "17" };
         {
            InSequence sequence;
            EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( query ) ) ) );
            EXPECT_CALL( db, lastErrno() ).WillOnce( Return( queryFailure ? error : IUS_DB2::OK ) );
            if ( !queryFailure )
            {
               EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
               EXPECT_CALL( db, lastErrno() ).WillOnce( Return( error ) );
            }
         }
         EXPECT_CALL( db, value( _ ) ).Times( 0 );
         EXPECT_EQ( project.readFromDB( 17, &db ), error == IUS_DB2::OK || error == IUS_DB2::NOROWS ? IUS_DB2::NO_PROJECT : error );
         EXPECT_EQ( project.projectID, 73 );
         EXPECT_EQ( project.projectGUID, QString( "previous" ) );
      }
   }
}

TEST( AnalyteDbFailure, EachRequiredLookupStopsOnQueryOrCursorFailure )
{
   // Reach each stage with finite rows; all later queries and field reads are
   // rejected by StrictMock. Nucleotide data is required for DNA/RNA only.
   for ( const QString& type : { QString( "DNA" ), QString( "RNA" ) } )
   {
      for ( int stage = 0; stage < 3; ++stage )
      {
         for ( bool queryFailure : { false, true } )
         {
            for ( int error : { IUS_DB2::OK, IUS_DB2::NOROWS, IUS_DB2::DBERROR, IUS_DB2::NOT_CONNECTED } )
            {
               if ( queryFailure && error == IUS_DB2::OK )
               {
                  continue;
               }
               SCOPED_TRACE( ::testing::Message() << type.toStdString() << ":" << stage << ":" << queryFailure << ":" << error );
               StrictMock<US_DB2_Mock> db;
               EXPECT_CALL( db, lastError() ).Times( ::testing::AnyNumber() ).WillRepeatedly( Return( QString() ) );
               US_Analyte analyte;
               analyte.analyteGUID         = "previous-guid";
               analyte.analyteID           = "old-id";
               analyte.type                = US_Analyte::CARBOHYDRATE;
               analyte.description         = "previous description";
               analyte.sequence            = "previous sequence";
               analyte.extinction[280.0]   = 42.0;
               const US_Analyte  previous  = analyte;
               const QStringList queries[] = { { "get_analyteID", "synthetic-guid" },
                                               { "get_analyte_info", "17" },
                                               { "get_nucleotide_info", "17" } };
               InSequence        sequence;
               for ( int current = 0; current <= stage; ++current )
               {
                  EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( queries[current] ) ) ) );
                  EXPECT_CALL( db, lastErrno() )
                     .Times( AtLeast( 1 ) )
                     .WillRepeatedly( Return( current == stage && queryFailure ? error : IUS_DB2::OK ) );
                  if ( current == stage )
                  {
                     if ( !queryFailure )
                     {
                        EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
                        EXPECT_CALL( db, lastErrno() ).WillOnce( Return( error ) );
                     }
                  }
                  else
                  {
                     EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
                     if ( current == 0 )
                     {
                        EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "17" ) ) );
                     }
                     else
                     {
                        EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( type ) ) );
                        for ( unsigned col : { 2u, 3u, 4u, 6u } )
                        {
                           EXPECT_CALL( db, value( col ) ).WillOnce( Return( QVariant( "1" ) ) );
                        }
                     }
                  }
               }
               EXPECT_EQ( analyte.load( true, "synthetic-guid", &db ), error == IUS_DB2::OK ? IUS_DB2::NOROWS : error );
               EXPECT_FALSE( analyte.message.isEmpty() );
               EXPECT_TRUE( analyte == previous );
               EXPECT_EQ( analyte.analyteID, previous.analyteID );
            }
         }
      }
   }
}

TEST( DbCallerFailure, NullConnectionsAreReportedWithoutAccess )
{
   US_Project               project;
   US_Analyte               analyte;
   US_Experiment            experiment;
   QVector<SP_SPEEDPROFILE> speedsteps;
   EXPECT_EQ( project.readFromDB( 17, nullptr ), IUS_DB2::NOT_CONNECTED );
   EXPECT_EQ( analyte.load( true, "synthetic-guid", nullptr ), IUS_DB2::NOT_CONNECTED );
   EXPECT_EQ( experiment.readFromDB( "run", nullptr, speedsteps ), IUS_DB2::NOT_CONNECTED );
}

TEST( AnalyteDbFailure, SuccessfulLoadsRequireNucleotideRowsOnlyForDnaAndRna )
{
   for ( const QString& type : { QString( "Protein" ), QString( "Other" ), QString( "DNA" ), QString( "RNA" ) } )
   {
      SCOPED_TRACE( type.toStdString() );
      StrictMock<US_DB2_Mock> db;
      EXPECT_CALL( db, lastError() ).Times( ::testing::AnyNumber() ).WillRepeatedly( Return( QString() ) );
      US_Analyte analyte;
      analyte.message = "previous failure";
      InSequence sequence;
      EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( QStringList{ "get_analyteID", "synthetic-guid" } ) ) ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "17" ) ) );
      EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( QStringList{ "get_analyte_info", "17" } ) ) ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( type ) ) );
      EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "ACGT" ) ) );
      EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "0.73" ) ) );
      EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( "synthetic description" ) ) );
      EXPECT_CALL( db, value( 6 ) ).WillOnce( Return( QVariant( "1000" ) ) );
      if ( type == "Other" )
      {
         EXPECT_CALL( db, value( 7 ) ).WillOnce( Return( QVariant( "0" ) ) );
      }
      if ( type == "DNA" || type == "RNA" )
      {
         EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( QStringList{ "get_nucleotide_info", "17" } ) ) ) );
         EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         for ( unsigned col = 0; col < 9; ++col )
         {
            EXPECT_CALL( db, value( col ) ).WillOnce( Return( QVariant( "0" ) ) );
         }
      }
      EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( QStringList{ "get_eprofile", "17", "Analyte" } ) ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_EQ( analyte.load( true, "synthetic-guid", &db ), IUS_DB2::OK );
      EXPECT_EQ( analyte.analyteGUID, QString( "synthetic-guid" ) );
      EXPECT_EQ( analyte.description, QString( "synthetic description" ) );
      EXPECT_TRUE( analyte.message.isEmpty() );
      // Nucleotide calculation normalizes nucleic-acid sequences to lowercase.
      EXPECT_EQ( analyte.sequence, QString( type == "DNA" || type == "RNA" ? "acgt" : "ACGT" ) );
   }
}
