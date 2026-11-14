//! \file test_us_buffer_component_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_buffer.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

// Known defects in US_BufferComponent (ehb54/ultrascan-tickets#1241) that these tests
// deliberately do not assert:
// - getInfoFromDB ignores query() and next() failures and indexes six coefficient tokens
//   without checking how many there are, so failed, empty and short rows are not tested.
// - saveToDB puts a space before the first coefficient and none between the others, so
//   the save tests check only the metadata arguments.
// getAllFromDB opens its own DB connection and is not unit tested.

namespace
{
class BufferComponentDb : public QtTestBase
{
   protected:
      US_BufferComponent      component;
      StrictMock<US_DB2_Mock> db;

      void                    expectRow( const QString& range, const QString& flag )
      {
         const QStringList query{ "get_buffer_component_info", "73" };
         InSequence        sequence;
         EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( query ) ) ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "mM" ) ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "Salt & sugar" ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "-1 0 2.5 3e-2 4 5" ) ) );
         EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "10 20 30 40 50 60 legacy range" ) ) );
         EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( range ) ) );
         EXPECT_CALL( db, value( 5 ) ).WillOnce( Return( QVariant( flag ) ) );
      }
      void checkRow( const QString& range, bool gradient )
      {
         EXPECT_EQ( QString( "73" ), component.componentID );
         EXPECT_EQ( QString( "mM" ), component.unit );
         EXPECT_EQ( QString( "Salt & sugar" ), component.name );
         EXPECT_EQ( range, component.range );
         EXPECT_EQ( gradient, component.grad_form );
         const double viscosity[]{ -1, 0, 2.5, 0.03, 4, 5 };
         const double density[]{ 10, 20, 30, 40, 50, 60 };
         for ( int i = 0; i < 6; ++i )
         {
            SCOPED_TRACE( i );
            EXPECT_DOUBLE_EQ( viscosity[i], component.visc_coeff[i] );
            EXPECT_DOUBLE_EQ( density[i], component.dens_coeff[i] );
         }
      }
};

TEST_F( BufferComponentDb, MapsAllColumnsAndPrefersExplicitRange )
{
   component.componentID = "73";
   expectRow( "0 to 9", "1" );
   component.getInfoFromDB( &db );
   checkRow( "0 to 9", true );
}

TEST_F( BufferComponentDb, UsesLegacyDensitySuffixWhenRangeIsEmpty )
{
   component.componentID = "73";
   expectRow( "", "T" );
   component.getInfoFromDB( &db );
   checkRow( "legacy range", true );
}

TEST_F( BufferComponentDb, ReplacesExistingValuesIncludingTrueGradientFlag )
{
   component.componentID = "73";
   component.name        = "stale";
   component.range       = "stale";
   component.grad_form   = true;
   for ( int i = 0; i < 6; ++i )
   {
      component.visc_coeff[i] = component.dens_coeff[i] = -999;
   }
   expectRow( "explicit", "0" );
   component.getInfoFromDB( &db );
   checkRow( "explicit", false );
}

TEST_F( BufferComponentDb, SaveRoutesMetadataAndPublishesSuccessfulInsertId )
{
   for ( bool gradient : { false, true } )
   {
      SCOPED_TRACE( gradient );
      component.componentID = "73";
      component.name        = "Salt & sugar";
      component.unit        = "mM";
      component.range       = "0 to 9";
      component.grad_form   = gradient;
      {
         InSequence sequence;
         EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
            .WillOnce( Invoke(
               [gradient]( const QStringList& args )
               {
                  EXPECT_EQ( 7, args.size() );
                  EXPECT_EQ( ( QStringList{ "create_buffer_component", "Salt & sugar", "mM", "0 to 9", gradient ? "1" : "0" } ),
                             args.mid( 0, 5 ) );
                  // Arguments 5 and 6 (density, viscosity) are left unchecked until #1241 is fixed.
                  return IUS_DB2::OK;
               } ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 456 ) );
      }
      EXPECT_EQ( 456, component.saveToDB( &db ) );
      EXPECT_EQ( QString( "456" ), component.componentID );
      EXPECT_EQ( QString( "Salt & sugar" ), component.name );
      EXPECT_EQ( gradient, component.grad_form );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( BufferComponentDb, FailedSavePreservesIdAndNeverFetchesInsertId )
{
   component.componentID = "73";
   EXPECT_CALL( db, lastInsertID() ).Times( 0 );
   {
      InSequence sequence;
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) ); // diagnostic
   }
   EXPECT_EQ( -1, component.saveToDB( &db ) );
   EXPECT_EQ( QString( "73" ), component.componentID );
}
}
