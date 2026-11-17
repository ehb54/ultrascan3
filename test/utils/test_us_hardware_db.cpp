//! \file test_us_hardware_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_hardware.h"

using ::testing::AnyOf;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

namespace
{
Matcher<const QStringList&> args( const QStringList& expected )
{
   return Matcher<const QStringList&>( Eq( expected ) );
}

QStringList identities( const QList<US_AbstractCenterpiece>& centerpieces )
{
   QStringList ids;
   for ( const US_AbstractCenterpiece& cp : centerpieces )
   {
      ids << QString( "%1 %2" ).arg( cp.serial_number ).arg( cp.name );
   }
   return ids;
}

class HardwareDb : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock> db;

      // A query that succeeds and yields one value per row in column 0.
      void                    expectIds( const QStringList& query, const QStringList& ids )
      {
         EXPECT_CALL( db, query( args( query ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         for ( const QString& id : ids )
         {
            EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
            EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( id ) ) );
         }
         EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
      }

      void expectFailure( const QStringList& query )
      {
         EXPECT_CALL( db, query( args( query ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
      }

      void expectCalibration( const QString& id, const QString& coeff1, const QString& coeff2 )
      {
         EXPECT_CALL( db, query( args( { "get_rotor_calibration_info", id } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         // The two columns are read in one expression, in unspecified order.
         EXPECT_CALL( db, value( AnyOf( 4u, 5u ) ) )
            .Times( 2 )
            .WillRepeatedly( Invoke( [coeff1, coeff2]( unsigned col ) { return QVariant( col == 4 ? coeff1 : coeff2 ); } ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
      }

      void expectCenterpiece( const QString& id, const QStringList& columns )
      {
         EXPECT_CALL( db, query( args( { "get_abstractCenterpiece_info", id } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         for ( unsigned col = 0; col < 9; ++col )
         {
            EXPECT_CALL( db, value( col ) ).WillOnce( Return( QVariant( columns[col] ) ) );
         }
      }
};

// Rotor calibration map

TEST_F( HardwareDb, RotorMapWithoutConnectionReadsLocalFiles )
{
   QMap<QString, QString> local;
   const bool             localOk = US_Hardware::readRotorMap( local );
   EXPECT_CALL( db, isConnected() ).WillOnce( Return( false ) );
   QMap<QString, QString> map{ { "stale", "1 1" } };
   EXPECT_EQ( localOk, US_Hardware::readRotorMap( &db, map ) );
   EXPECT_EQ( local, map );
}

TEST_F( HardwareDb, RotorMapCollectsCoefficientsThroughLabsRotorsAndCalibrations )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, isConnected() ).WillOnce( Return( true ) );
      expectIds( { "get_lab_names" }, { "1", "2" } );
      expectIds( { "get_rotor_names", "1" }, { "10" } );
      expectIds( { "get_rotor_names", "2" }, { "20", "21" } );
      expectIds( { "get_rotor_calibration_profiles", "10" }, { "100" } );
      expectIds( { "get_rotor_calibration_profiles", "20" }, { "200" } );
      expectIds( { "get_rotor_calibration_profiles", "21" }, {} );
      expectCalibration( "100", "0.25", "-1.5e-05" );
      expectCalibration( "200", "3", "4" );
   }
   QMap<QString, QString> map{ { "stale", "1 1" } };
   EXPECT_TRUE( US_Hardware::readRotorMap( &db, map ) );
   EXPECT_EQ( ( QMap<QString, QString>{ { "0", "0 0" }, { "100", "0.25 -1.5e-05" }, { "200", "3 4" } } ), map );

   double coefficients[2]{};
   ASSERT_TRUE( US_Hardware::rotorValues( "100", map, coefficients ) );
   EXPECT_DOUBLE_EQ( 0.25, coefficients[0] );
   EXPECT_DOUBLE_EQ( -1.5e-05, coefficients[1] );
}

TEST_F( HardwareDb, RotorMapFailedLabQueryStopsWithOnlyTheSimulationEntry )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, isConnected() ).WillOnce( Return( true ) );
      expectFailure( { "get_lab_names" } );
   }
   QMap<QString, QString> map{ { "stale", "1 1" } };
   EXPECT_FALSE( US_Hardware::readRotorMap( &db, map ) );
   EXPECT_EQ( ( QMap<QString, QString>{ { "0", "0 0" } } ), map );
}

TEST_F( HardwareDb, RotorMapSkipsLabsRotorsAndCalibrationsWhoseQueriesFail )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, isConnected() ).WillOnce( Return( true ) );
      expectIds( { "get_lab_names" }, { "1", "2" } );
      expectFailure( { "get_rotor_names", "1" } );
      expectIds( { "get_rotor_names", "2" }, { "20", "21" } );
      expectFailure( { "get_rotor_calibration_profiles", "20" } );
      expectIds( { "get_rotor_calibration_profiles", "21" }, { "210", "211" } );
      expectFailure( { "get_rotor_calibration_info", "210" } );
      expectCalibration( "211", "5", "6" );
   }
   QMap<QString, QString> map;
   EXPECT_TRUE( US_Hardware::readRotorMap( &db, map ) );
   EXPECT_EQ( ( QMap<QString, QString>{ { "0", "0 0" }, { "211", "5 6" } } ), map );
}

TEST_F( HardwareDb, RotorMapWithoutCalibrationsReportsFailure )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, isConnected() ).WillOnce( Return( true ) );
      expectIds( { "get_lab_names" }, { "1" } );
      expectIds( { "get_rotor_names", "1" }, { "10" } );
      expectIds( { "get_rotor_calibration_profiles", "10" }, {} );
   }
   QMap<QString, QString> map;
   EXPECT_FALSE( US_Hardware::readRotorMap( &db, map ) );
   EXPECT_EQ( ( QMap<QString, QString>{ { "0", "0 0" } } ), map );
}

// Centerpieces

TEST_F( HardwareDb, CenterpiecesMapInfoColumnsInIdOrder )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, isConnected() ).WillOnce( Return( true ) );
      expectIds( { "get_abstractCenterpiece_names" }, { "7", "3" } );
      expectCenterpiece( "3", { "guid-3", "Epon 2-channel", "2", "7.2:7.1", "", "60000", "1.2", "2.5", "0" } );
      expectCenterpiece( "7", { "guid-7", "Titanium band", "0", "7.0", "rectangular", "42000", "0.3", "0", "1.5" } );
   }
   QList<US_AbstractCenterpiece> centerpieces;
   centerpieces << US_AbstractCenterpiece();
   ASSERT_TRUE( US_AbstractCenterpiece::read_centerpieces( &db, centerpieces ) );
   ASSERT_EQ( 2, centerpieces.size() );

   const US_AbstractCenterpiece& epon = centerpieces[0];
   EXPECT_EQ( 3, epon.serial_number );
   EXPECT_EQ( QString( "guid-3" ), epon.guid );
   EXPECT_EQ( QString( "Epon 2-channel" ), epon.name );
   EXPECT_EQ( QString( "Epon" ), epon.material );
   EXPECT_EQ( 2, epon.channels );
   EXPECT_EQ( QString( "sector" ), epon.shape );
   EXPECT_DOUBLE_EQ( 60000.0, epon.maxRPM );
   EXPECT_DOUBLE_EQ( 2.5, epon.angle );
   EXPECT_DOUBLE_EQ( 0.0, epon.width );
   EXPECT_EQ( ( QList<double>{ 7.2, 7.1 } ), epon.bottom_position );
   EXPECT_EQ( ( QList<double>{ 1.2, 1.2 } ), epon.path_length );

   const US_AbstractCenterpiece& band = centerpieces[1];
   EXPECT_EQ( 7, band.serial_number );
   EXPECT_EQ( QString( "Titanium" ), band.material );
   EXPECT_EQ( 1, band.channels );
   EXPECT_EQ( QString( "rectangular" ), band.shape );
   EXPECT_DOUBLE_EQ( 42000.0, band.maxRPM );
   EXPECT_DOUBLE_EQ( 1.5, band.width );
   EXPECT_EQ( ( QList<double>{ 7.0 } ), band.bottom_position );
   EXPECT_EQ( ( QList<double>{ 0.3 } ), band.path_length );
}

TEST_F( HardwareDb, CenterpiecesFallBackToLocalFileWhenAnInfoQueryFails )
{
   QList<US_AbstractCenterpiece> local;
   ASSERT_TRUE( US_AbstractCenterpiece::read_centerpieces( local ) );
   ASSERT_FALSE( local.isEmpty() );
   // The first centerpiece reads; the second fails or has no row. Nothing from the DB is kept.
   for ( const bool missingRow : { false, true } )
   {
      SCOPED_TRACE( missingRow );
      {
         InSequence sequence;
         EXPECT_CALL( db, isConnected() ).WillOnce( Return( true ) );
         expectIds( { "get_abstractCenterpiece_names" }, { "3", "9", "11" } );
         expectCenterpiece( "3", { "guid-3", "Epon 2-channel", "2", "7.2:7.1", "", "60000", "1.2", "2.5", "0" } );
         if ( missingRow )
         {
            EXPECT_CALL( db, query( args( { "get_abstractCenterpiece_info", "9" } ) ) );
            EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
            EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
         }
         else
         {
            expectFailure( { "get_abstractCenterpiece_info", "9" } );
         }
      }
      QList<US_AbstractCenterpiece> centerpieces;
      ASSERT_TRUE( US_AbstractCenterpiece::read_centerpieces( &db, centerpieces ) );
      EXPECT_EQ( identities( local ), identities( centerpieces ) );
      ::testing::Mock::VerifyAndClearExpectations( &db );
   }
}

TEST_F( HardwareDb, CenterpiecesFallBackToLocalFileWhenTheNameQueryFails )
{
   QList<US_AbstractCenterpiece> local;
   ASSERT_TRUE( US_AbstractCenterpiece::read_centerpieces( local ) );
   ASSERT_FALSE( local.isEmpty() );
   {
      InSequence sequence;
      EXPECT_CALL( db, isConnected() ).WillOnce( Return( true ) );
      expectFailure( { "get_abstractCenterpiece_names" } );
   }
   QList<US_AbstractCenterpiece> centerpieces;
   ASSERT_TRUE( US_AbstractCenterpiece::read_centerpieces( &db, centerpieces ) );
   EXPECT_EQ( identities( local ), identities( centerpieces ) );
}

TEST_F( HardwareDb, CenterpiecesWithoutConnectionReadLocalFile )
{
   QList<US_AbstractCenterpiece> local;
   ASSERT_TRUE( US_AbstractCenterpiece::read_centerpieces( local ) );
   EXPECT_CALL( db, isConnected() ).WillOnce( Return( false ) );
   QList<US_AbstractCenterpiece> centerpieces;
   ASSERT_TRUE( US_AbstractCenterpiece::read_centerpieces( &db, centerpieces ) );
   EXPECT_EQ( identities( local ), identities( centerpieces ) );
}
} // namespace
