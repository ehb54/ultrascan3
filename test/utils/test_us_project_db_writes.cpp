//! \file test_us_project_db_writes.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_project.h"
#include "us_settings.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

namespace
{
class ProjectDbWrite : public QtTestBase, public ::testing::WithParamInterface<bool>
{
   protected:
      void SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         US_Settings::set_us_inv_ID( 17 );
         project.projectGUID      = "12345678-1234-1234-1234-123456789012";
         project.goals            = "goals";
         project.molecules        = "molecules";
         project.purity           = "purity";
         project.expense          = "expense";
         project.bufferComponents = "buffer";
         project.saltInformation  = "salt";
         project.AUC_questions    = "questions";
         project.expDesign        = "design";
         project.notes            = "notes";
         project.projectDesc      = "description";
         project.status           = "submitted";
      }
      int save()
      {
         return GetParam() ? project.saveToDB_auto( 29, &db ) : project.saveToDB( &db );
      }
      QStringList fields() const
      {
         return { project.projectGUID, "goals",  "molecules", "purity",      "expense",  "buffer", "salt",
                  "questions",         "design", "notes",     "description", "submitted" };
      }
      US_Project              project;
      StrictMock<US_DB2_Mock> db;
};
INSTANTIATE_TEST_SUITE_P( RegularAndExplicitInvestigator, ProjectDbWrite, ::testing::Bool() );

TEST_P( ProjectDbWrite, CreatesProjectWithAllFieldsAndCorrectInvestigator )
{
   const QStringList lookup{ "get_projectID_from_GUID", project.projectGUID };
   QStringList       insert{ "new_project2" };
   insert << fields() << QString::number( GetParam() ? 29 : 17 );
   {
      InSequence sequence;
      EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( lookup ) ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( insert ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 91 ) );
   }
   EXPECT_CALL( db, next() ).Times( 0 );
   EXPECT_CALL( db, value( _ ) ).Times( 0 );
   ASSERT_EQ( IUS_DB2::OK, save() );
   EXPECT_EQ( 91, project.projectID );
   EXPECT_EQ( US_Project::BOTH, project.saveStatus );
   EXPECT_EQ( 17, US_Settings::us_inv_ID() );
}

TEST_P( ProjectDbWrite, UpdatesExistingProjectWithoutInsertOrInvestigatorMutation )
{
   const QStringList lookup{ "get_projectID_from_GUID", project.projectGUID };
   QStringList       update{ "update_project2", "87" };
   update << fields();
   {
      InSequence sequence;
      EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( lookup ) ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( 87 ) ) );
      EXPECT_CALL( db, statusQuery( update ) ).WillOnce( Return( IUS_DB2::OK ) );
   }
   EXPECT_CALL( db, lastInsertID() ).Times( 0 );
   ASSERT_EQ( IUS_DB2::OK, save() );
   EXPECT_EQ( 87, project.projectID );
   EXPECT_EQ( US_Project::BOTH, project.saveStatus );
   EXPECT_EQ( 17, US_Settings::us_inv_ID() );
}

TEST_P( ProjectDbWrite, LookupErrorStopsBeforeCursorAndWrites )
{
   for ( int error : { IUS_DB2::DBERROR, IUS_DB2::NOT_CONNECTED } )
   {
      SCOPED_TRACE( error );
      const QStringList lookup{ "get_projectID_from_GUID", project.projectGUID };
      EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( lookup ) ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( error ) );
      EXPECT_CALL( db, lastError() ).WillOnce( Return( QString( "lookup failed" ) ) );
      EXPECT_CALL( db, next() ).Times( 0 );
      EXPECT_CALL( db, value( _ ) ).Times( 0 );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).Times( 0 );
      EXPECT_CALL( db, lastInsertID() ).Times( 0 );
      EXPECT_EQ( error, save() );
      EXPECT_EQ( US_Project::HD_ONLY, project.saveStatus );
      EXPECT_EQ( QString( "description" ), project.projectDesc );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_P( ProjectDbWrite, MissingInsertedIdDoesNotClaimDatabaseSave )
{
   const QStringList lookup{ "get_projectID_from_GUID", project.projectGUID };
   QStringList       insert{ "new_project2" };
   insert << fields() << QString::number( GetParam() ? 29 : 17 );
   {
      InSequence sequence;
      EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( lookup ) ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( insert ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 0 ) );
   }
   EXPECT_EQ( IUS_DB2::NO_PROJECT, save() );
   EXPECT_EQ( US_Project::HD_ONLY, project.saveStatus );
}
}
