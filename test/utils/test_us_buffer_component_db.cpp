//! \file test_us_buffer_component_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_buffer.h"

#include <QUuid>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

// getAllFromDB opens its own DB connection and is not unit tested.

namespace
{
class BufferComponentDb : public QtTestBase
{
   protected:
      US_BufferComponent      component;
      StrictMock<US_DB2_Mock> db;

      // Coefficients of 1-Propanol from etc/bufferComponents.xml.
      void                    setCoefficients()
      {
         const double density[]{ 0.99820, 0.0, -1.21245, 2.81044, -7.63148, 53.39717 };
         const double viscosity[]{ 1.00725, -119.10622, 44.35231, 7.82088, -31.16201, 0.0 };
         for ( int i = 0; i < 6; ++i )
         {
            component.dens_coeff[i] = density[i];
            component.visc_coeff[i] = viscosity[i];
         }
      }

      void expectRow( const QString& range, const QString& flag )
      {
         expectColumns( "-1 0 2.5 3e-2 4 5", "10 20 30 40 50 60 legacy range", range, flag );
      }
      void expectColumns( const QString& viscosity, const QString& density, const QString& range, const QString& flag )
      {
         const QStringList query{ "get_buffer_component_info", "73" };
         InSequence        sequence;
         EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( query ) ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "mM" ) ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "Salt & sugar" ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( viscosity ) ) );
         EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( density ) ) );
         EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( range ) ) );
         EXPECT_CALL( db, value( 5 ) ).WillOnce( Return( QVariant( flag ) ) );
      }
      // A component whose every field differs from any row the tests return.
      void prepareStale()
      {
         component.componentID = "73";
         component.unit        = "stale unit";
         component.name        = "stale";
         component.range       = "stale range";
         component.grad_form   = true;
         for ( int i = 0; i < 6; ++i )
         {
            component.visc_coeff[i] = component.dens_coeff[i] = -999;
         }
      }
      void checkStale()
      {
         EXPECT_EQ( QString( "73" ), component.componentID );
         EXPECT_EQ( QString( "stale unit" ), component.unit );
         EXPECT_EQ( QString( "stale" ), component.name );
         EXPECT_EQ( QString( "stale range" ), component.range );
         EXPECT_TRUE( component.grad_form );
         for ( int i = 0; i < 6; ++i )
         {
            SCOPED_TRACE( i );
            EXPECT_EQ( -999, component.visc_coeff[i] );
            EXPECT_EQ( -999, component.dens_coeff[i] );
         }
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
   ASSERT_TRUE( component.getInfoFromDB( &db ) );
   checkRow( "0 to 9", true );
}

TEST_F( BufferComponentDb, UsesLegacyDensitySuffixWhenRangeIsEmpty )
{
   component.componentID = "73";
   expectRow( "", "T" );
   ASSERT_TRUE( component.getInfoFromDB( &db ) );
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
   ASSERT_TRUE( component.getInfoFromDB( &db ) );
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
      setCoefficients();
      {
         InSequence sequence;
         EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
            .WillOnce( Invoke(
               [gradient]( const QStringList& args )
               {
                  // create_buffer_component( p_personGUID, p_password, p_componentGUID, p_name, p_unit,
                  // p_range, p_grad_form, p_density, p_viscosity ): the query layer supplies the
                  // first two, so the call carries the other seven.
                  const QString componentGuid = args.value( 1 );
                  EXPECT_EQ( 36, componentGuid.size() );
                  EXPECT_FALSE( QUuid( componentGuid ).isNull() );
                  EXPECT_EQ( ( QStringList{ "create_buffer_component",
                                            componentGuid,
                                            "Salt & sugar",
                                            "mM",
                                            "0 to 9",
                                            gradient ? "1" : "0",
                                            "0.99820 0.00000 -1.21245 2.81044 -7.63148 53.39717",
                                            "1.00725 -119.10622 44.35231 7.82088 -31.16201 0.00000" } ),
                             args );
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
      // Further reads (e.g. for logging) are allowed; only the insert-ID lookup is forbidden.
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( -1, component.saveToDB( &db ) );
   EXPECT_EQ( QString( "73" ), component.componentID );
}

TEST_F( BufferComponentDb, SavedCoefficientsReadBackUnchanged )
{
   component.name  = "Salt & sugar";
   component.unit  = "mM";
   component.range = "0 to 9";
   setCoefficients();
   QStringList sent;
   {
      InSequence sequence;
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            [&sent]( const QStringList& args )
            {
               sent = args;
               return IUS_DB2::OK;
            } ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 73 ) );
   }
   ASSERT_EQ( 73, component.saveToDB( &db ) );
   ASSERT_EQ( 8, sent.size() );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   // The stored procedure takes density before viscosity; the info query returns viscosity first.
   US_BufferComponent loaded;
   loaded.componentID = "73";
   expectColumns( sent[7], sent[6], sent[4], sent[5] );
   ASSERT_TRUE( loaded.getInfoFromDB( &db ) );
   for ( int i = 0; i < 6; ++i )
   {
      SCOPED_TRACE( i );
      EXPECT_DOUBLE_EQ( component.dens_coeff[i], loaded.dens_coeff[i] );
      EXPECT_DOUBLE_EQ( component.visc_coeff[i], loaded.visc_coeff[i] );
   }
   EXPECT_EQ( QString( "0 to 9" ), loaded.range );
}

TEST_F( BufferComponentDb, ToleratesExtraSpacesBetweenCoefficients )
{
   component.componentID = "73";
   expectColumns( " -1  0 2.5 3e-2 4 5 ", "  10 20  30 40 50 60  legacy  range", "", "1" );
   ASSERT_TRUE( component.getInfoFromDB( &db ) );
   checkRow( "legacy range", true );
}

TEST_F( BufferComponentDb, FailedOrEmptyQueryReadsNoColumnsAndKeepsTheComponent )
{
   for ( int status : { IUS_DB2::DBERROR, IUS_DB2::NOROWS, IUS_DB2::NOT_CONNECTED } )
   {
      SCOPED_TRACE( status );
      prepareStale();
      {
         InSequence sequence;
         EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( QStringList{ "get_buffer_component_info", "73" } ) ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
      }
      EXPECT_CALL( db, next() ).Times( 0 );
      EXPECT_CALL( db, value( _ ) ).Times( 0 );
      EXPECT_FALSE( component.getInfoFromDB( &db ) );
      checkStale();
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( BufferComponentDb, MissingRowReadsNoColumnsAndKeepsTheComponent )
{
   prepareStale();
   {
      InSequence sequence;
      EXPECT_CALL( db, query( Matcher<const QStringList&>( Eq( QStringList{ "get_buffer_component_info", "73" } ) ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
   }
   EXPECT_CALL( db, value( _ ) ).Times( 0 );
   EXPECT_FALSE( component.getInfoFromDB( &db ) );
   checkStale();
}

TEST_F( BufferComponentDb, MalformedCoefficientsKeepTheComponent )
{
   const QString six = "1 2 3 4 5 6";
   const struct
   {
         const char* label;
         QString     viscosity;
         QString     density;
   } rows[]{
      { "empty viscosity", "", six },
      { "empty density", six, "" },
      { "five viscosity", "1 2 3 4 5", six },
      { "five density", six, "1 2 3 4 5" },
      { "seven viscosity", "1 2 3 4 5 6 7", six },
      { "non-numeric viscosity", "1 2 x 4 5 6", six },
      { "non-numeric density", six, "1 2 3 4 5 six" },
      // The pre-fix writer ran the coefficients together after a leading space.
      { "unseparated", " 1.000002.000003.000004.000005.000006.00000", " 1.000002.000003.000004.000005.000006.00000" },
   };
   for ( const auto& row : rows )
   {
      SCOPED_TRACE( row.label );
      prepareStale();
      expectColumns( row.viscosity, row.density, "0 to 1", "0" );
      EXPECT_FALSE( component.getInfoFromDB( &db ) );
      checkStale();
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}
}
