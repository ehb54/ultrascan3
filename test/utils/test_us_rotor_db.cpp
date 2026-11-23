//! \file test_us_rotor_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_rotor.h"

using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

// Not covered: the disk readers and writers, and readLabsDB's copy of the labs to disk.

namespace
{
Matcher<const QStringList&> args( const QStringList& expected )
{
   return Matcher<const QStringList&>( Eq( expected ) );
}

using Lab = US_Rotor::Lab;

class RotorDb : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock> db;

      void                    expectQuery( const QStringList& query, int status, bool row )
      {
         EXPECT_CALL( db, query( args( query ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
         if ( status == IUS_DB2::OK )
         {
            EXPECT_CALL( db, next() ).WillOnce( Return( row ) );
         }
      }

      // A list query returning one value per row in column 0.
      void expectIds( const QStringList& query, const QStringList& ids )
      {
         EXPECT_CALL( db, query( args( query ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( ids.isEmpty() ? IUS_DB2::NOROWS : IUS_DB2::OK ) );
         for ( const QString& id : ids )
         {
            EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
            EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( id ) ) );
         }
         if ( !ids.isEmpty() )
         {
            EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
         }
      }

      void expectLabRow( const QString& id, const QString& name )
      {
         expectQuery( { "get_lab_info", id }, IUS_DB2::OK, true );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "lab-guid-" + id ) ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( name ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "Building" ) ) );
         EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "Room" ) ) );
      }

      void expectInstrumentRow( const QString& id )
      {
         expectQuery( { "get_instrument_info_new", id }, IUS_DB2::OK, true );
         const QList<QPair<unsigned, QString>> columns{ { 0, "Optima 1" }, { 1, "SN-1" },   { 5, "optima.host" }, { 6, "5432" },
                                                        { 7, "AUC" },      { 8, "reader" }, { 9, "secret" },      { 10, "1" },
                                                        { 11, "1" },       { 12, "0" },     { 13, "1" },          { 14, "280" },
                                                        { 15, "chromo" } };
         for ( const auto& column : columns )
         {
            EXPECT_CALL( db, value( column.first ) ).WillOnce( Return( QVariant( column.second ) ) );
         }
      }

      static Lab previousLab()
      {
         US_Rotor::Lab lab;
         lab.ID   = 99;
         lab.name = "previous";
         return lab;
      }
};

// Labs

TEST_F( RotorDb, LabReadMapsInfoInstrumentsAndOperators )
{
   {
      InSequence sequence;
      expectLabRow( "3", "AUC Lab" );
      expectIds( { "get_instrument_names", "3" }, { "5" } );
      expectInstrumentRow( "5" );
      EXPECT_CALL( db, query( args( { "get_operator_names", "5" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "8" ) ) );
      EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "oper-guid" ) ) );
      EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "Doe" ) ) );
      EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "Jan" ) ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
   }
   Lab lab;
   ASSERT_EQ( US_Rotor::ROTOR_OK, lab.readDB( 3, &db ) );
   EXPECT_EQ( 3, lab.ID );
   EXPECT_EQ( QString( "lab-guid-3" ), lab.GUID );
   EXPECT_EQ( QString( "AUC Lab" ), lab.name );
   EXPECT_EQ( QString( "Building" ), lab.building );
   EXPECT_EQ( QString( "Room" ), lab.room );
   ASSERT_EQ( 1, lab.instruments.size() );
   const US_Rotor::Instrument& instrument = lab.instruments[0];
   EXPECT_EQ( 5, instrument.ID );
   EXPECT_EQ( QString( "Optima 1" ), instrument.name );
   EXPECT_EQ( QString( "SN-1" ), instrument.serial );
   EXPECT_EQ( QString( "optima.host" ), instrument.optimaHost );
   EXPECT_EQ( 5432, instrument.optimaPort );
   EXPECT_EQ( 1, instrument.selected );
   EXPECT_EQ( QString( "280" ), instrument.radcalwvl );
   ASSERT_EQ( 1, instrument.operators.size() );
   EXPECT_EQ( 8, instrument.operators[0].ID );
   EXPECT_EQ( QString( "Doe" ), instrument.operators[0].lname );
   EXPECT_EQ( QString( "Jan" ), instrument.operators[0].fname );
}

TEST_F( RotorDb, LabWithoutInstrumentsOrOperatorsReadsEmptyLists )
{
   {
      InSequence sequence;
      expectLabRow( "3", "AUC Lab" );
      expectIds( { "get_instrument_names", "3" }, {} );
   }
   Lab lab;
   ASSERT_EQ( US_Rotor::ROTOR_OK, lab.readDB( 3, &db ) );
   EXPECT_TRUE( lab.instruments.isEmpty() );
}

TEST_F( RotorDb, LabReadFailuresKeepTheLab )
{
   // Missing lab row, failed instrument list, failed instrument row and failed operator list.
   for ( int step = 0; step < 4; ++step )
   {
      SCOPED_TRACE( step );
      {
         InSequence sequence;
         if ( step == 0 )
         {
            expectQuery( { "get_lab_info", "3" }, IUS_DB2::OK, false );
         }
         else
         {
            expectLabRow( "3", "AUC Lab" );
            if ( step == 1 )
            {
               expectQuery( { "get_instrument_names", "3" }, IUS_DB2::DBERROR, false );
            }
            else
            {
               expectIds( { "get_instrument_names", "3" }, { "5" } );
               if ( step == 2 )
               {
                  expectQuery( { "get_instrument_info_new", "5" }, IUS_DB2::DBERROR, false );
               }
               else
               {
                  expectInstrumentRow( "5" );
                  expectQuery( { "get_operator_names", "5" }, IUS_DB2::DBERROR, false );
               }
            }
         }
      }
      Lab lab = previousLab();
      EXPECT_EQ( step == 0 ? US_Rotor::NOT_FOUND : US_Rotor::MISC_ERROR, lab.readDB( 3, &db ) );
      EXPECT_EQ( 99, lab.ID );
      EXPECT_EQ( QString( "previous" ), lab.name );
      EXPECT_TRUE( lab.instruments.isEmpty() );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( RotorDb, LabListStopsAtAFailedQueryOrLab )
{
   QVector<Lab> labs{ previousLab() };
   {
      InSequence sequence;
      expectQuery( { "get_lab_names" }, IUS_DB2::DBERROR, false );
   }
   EXPECT_EQ( US_Rotor::MISC_ERROR, US_Rotor::readLabsDB( labs, &db ) );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectIds( { "get_lab_names" }, { "3", "4" } );
      expectLabRow( "3", "AUC Lab" );
      expectIds( { "get_instrument_names", "3" }, {} );
      expectQuery( { "get_lab_info", "4" }, IUS_DB2::DBERROR, false );
   }
   EXPECT_EQ( US_Rotor::MISC_ERROR, US_Rotor::readLabsDB( labs, &db ) );
   ASSERT_EQ( 1, labs.size() );
   EXPECT_EQ( 99, labs[0].ID );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectIds( { "get_lab_names" }, {} );
   }
   EXPECT_EQ( US_Rotor::NOT_FOUND, US_Rotor::readLabsDB( labs, &db ) );
   EXPECT_EQ( 1, labs.size() );
}

// Abstract rotors and rotors

TEST_F( RotorDb, AbstractRotorReadMapsColumnsAndRejectsAMissingRow )
{
   {
      InSequence sequence;
      expectQuery( { "get_abstractRotor_info", "2" }, IUS_DB2::OK, true );
      const QStringList columns{ "ar-guid", "AN-60", "Titanium", "4", "60000", "0.5", "6.5", "Beckman" };
      for ( unsigned col = 0; col < 8; ++col )
      {
         EXPECT_CALL( db, value( col ) ).WillOnce( Return( QVariant( columns[col] ) ) );
      }
   }
   US_Rotor::AbstractRotor ar;
   ASSERT_EQ( US_Rotor::ROTOR_OK, ar.readDB( 2, &db ) );
   EXPECT_EQ( 2, ar.ID );
   EXPECT_EQ( QString( "AN-60" ), ar.name );
   EXPECT_EQ( 4, ar.numHoles );
   EXPECT_EQ( 60000, ar.maxRPM );
   EXPECT_DOUBLE_EQ( 6.5, ar.cellCenter );
   EXPECT_EQ( QString( "Beckman" ), ar.manufacturer );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectQuery( { "get_abstractRotor_info", "2" }, IUS_DB2::OK, false );
   }
   US_Rotor::AbstractRotor missing;
   missing.name = "previous";
   EXPECT_EQ( US_Rotor::NOT_FOUND, missing.readDB( 2, &db ) );
   EXPECT_EQ( QString( "previous" ), missing.name );
}

TEST_F( RotorDb, AbstractRotorListStopsAtAFailedRotor )
{
   QVector<US_Rotor::AbstractRotor> rotors( 1 );
   rotors[0].name = "previous";
   {
      InSequence sequence;
      expectIds( { "get_abstractRotor_names" }, { "2" } );
      expectQuery( { "get_abstractRotor_info", "2" }, IUS_DB2::DBERROR, false );
   }
   EXPECT_EQ( US_Rotor::MISC_ERROR, US_Rotor::readAbstractRotorsDB( rotors, &db ) );
   ASSERT_EQ( 1, rotors.size() );
   EXPECT_EQ( QString( "previous" ), rotors[0].name );
}

TEST_F( RotorDb, AbstractRotorListReportsAFailedListQuery )
{
   QVector<US_Rotor::AbstractRotor> rotors( 1 );
   rotors[0].name = "previous";
   EXPECT_CALL( db, query( args( { "get_abstractRotor_names" } ) ) );
   EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
   // A failed query is an error, not an empty list.
   EXPECT_EQ( US_Rotor::MISC_ERROR, US_Rotor::readAbstractRotorsDB( rotors, &db ) );
   ASSERT_EQ( 1, rotors.size() );
   EXPECT_EQ( QString( "previous" ), rotors[0].name );
}

TEST_F( RotorDb, RotorReadMapsColumnsAndRejectsAMissingRow )
{
   {
      InSequence sequence;
      expectQuery( { "get_rotor_info", "6" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( "2" ) ) );
      EXPECT_CALL( db, value( 5 ) ).WillOnce( Return( QVariant( "ar-guid" ) ) );
      EXPECT_CALL( db, value( 6 ) ).WillOnce( Return( QVariant( "3" ) ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "rotor-guid" ) ) );
      EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "Rotor A" ) ) );
      EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "SN-6" ) ) );
   }
   US_Rotor::Rotor rotor;
   ASSERT_EQ( US_Rotor::ROTOR_OK, rotor.readDB( 6, &db ) );
   EXPECT_EQ( 6, rotor.ID );
   EXPECT_EQ( 2, rotor.abstractRotorID );
   EXPECT_EQ( 3, rotor.labID );
   EXPECT_EQ( QString( "Rotor A" ), rotor.name );
   EXPECT_EQ( QString( "SN-6" ), rotor.serialNumber );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   for ( int status : { IUS_DB2::OK, IUS_DB2::NOROWS, IUS_DB2::DBERROR } )
   {
      SCOPED_TRACE( status );
      {
         InSequence sequence;
         expectQuery( { "get_rotor_info", "6" }, status, false );
      }
      US_Rotor::Rotor missing;
      missing.name = "previous";
      EXPECT_EQ( status == IUS_DB2::DBERROR ? US_Rotor::MISC_ERROR : US_Rotor::NOT_FOUND, missing.readDB( 6, &db ) );
      EXPECT_EQ( QString( "previous" ), missing.name );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( RotorDb, LabRotorListReadsEveryRotorOrNone )
{
   QVector<US_Rotor::Rotor> rotors;
   {
      InSequence sequence;
      expectIds( { "get_rotor_names", "3" }, { "6" } );
      expectQuery( { "get_rotor_info", "6" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "rotor-guid" ) ) );
      EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( 2 ) ) );
      EXPECT_CALL( db, value( 5 ) ).WillOnce( Return( QVariant( "ar-guid" ) ) );
      EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "Rotor A" ) ) );
      EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "SN-6" ) ) );
   }
   ASSERT_EQ( US_Rotor::ROTOR_OK, US_Rotor::readRotorsFromDB( rotors, 3, &db ) );
   ASSERT_EQ( 1, rotors.size() );
   EXPECT_EQ( 6, rotors[0].ID );
   EXPECT_EQ( 3, rotors[0].labID );
   EXPECT_EQ( 2, rotors[0].abstractRotorID );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   rotors.clear();
   {
      InSequence sequence;
      expectIds( { "get_rotor_names", "3" }, { "6", "7" } );
      expectQuery( { "get_rotor_info", "6" }, IUS_DB2::OK, false );
   }
   EXPECT_EQ( US_Rotor::MISC_ERROR, US_Rotor::readRotorsFromDB( rotors, 3, &db ) );
   EXPECT_TRUE( rotors.isEmpty() );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectQuery( { "get_rotor_names", "3" }, IUS_DB2::DBERROR, false );
   }
   EXPECT_EQ( US_Rotor::MISC_ERROR, US_Rotor::readRotorsFromDB( rotors, 3, &db ) );
}

TEST_F( RotorDb, AddAndDeleteSendTheRotorAndPassTheStatusThrough )
{
   US_Rotor::Rotor rotor;
   rotor.abstractRotorID   = 2;
   rotor.abstractRotorGUID = "ar-guid";
   rotor.labID             = 3;
   rotor.GUID              = "rotor-guid";
   rotor.name              = "Rotor A";
   rotor.serialNumber      = "SN-6";
   EXPECT_CALL( db, statusQuery( args( { "add_rotor", "2", "ar-guid", "3", "rotor-guid", "Rotor A", "SN-6" } ) ) )
      .WillOnce( Return( IUS_DB2::DBERROR ) );
   EXPECT_EQ( IUS_DB2::DBERROR, rotor.addRotorDB( &db ) );
   EXPECT_CALL( db, statusQuery( args( { "delete_rotor", "6" } ) ) ).WillOnce( Return( IUS_DB2::ROTOR_IN_USE ) );
   EXPECT_EQ( IUS_DB2::ROTOR_IN_USE, US_Rotor::Rotor::deleteRotorDB( 6, &db ) );
   EXPECT_CALL( db, statusQuery( args( { "delete_rotor_calibration", "10" } ) ) ).WillOnce( Return( IUS_DB2::OK ) );
   EXPECT_EQ( IUS_DB2::OK, US_Rotor::RotorCalibration::deleteCalibrationDB( 10, &db ) );
}

// Calibrations

TEST_F( RotorDb, CalibrationReadMapsColumnsAndRejectsAMissingRow )
{
   {
      InSequence sequence;
      expectQuery( { "get_rotor_calibration_info", "10" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "cal-guid" ) ) );
      EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "6" ) ) );
      EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "rotor-guid" ) ) );
      EXPECT_CALL( db, value( 8 ) ).WillOnce( Return( QVariant( "44" ) ) );
      EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( "0.25" ) ) );
      EXPECT_CALL( db, value( 5 ) ).WillOnce( Return( QVariant( "-1.5" ) ) );
      EXPECT_CALL( db, value( 9 ) ).WillOnce( Return( QVariant( "Calibration 1" ) ) );
      EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "report text" ) ) );
      EXPECT_CALL( db, value( 7 ) ).WillOnce( Return( QVariant( "2026-11-14 10:00:00" ) ) );
      EXPECT_CALL( db, value( 6 ) ).WillOnce( Return( QVariant( "0.5" ) ) );
   }
   US_Rotor::RotorCalibration rc;
   ASSERT_EQ( US_Rotor::ROTOR_OK, rc.readDB( 10, &db ) );
   EXPECT_EQ( 10, rc.ID );
   EXPECT_EQ( 6, rc.rotorID );
   EXPECT_EQ( 44, rc.calibrationExperimentID );
   EXPECT_DOUBLE_EQ( 0.25, rc.coeff1 );
   EXPECT_DOUBLE_EQ( -1.5, rc.coeff2 );
   EXPECT_EQ( QString( "Calibration 1" ), rc.label );
   EXPECT_EQ( QDate( 2026, 11, 14 ), rc.lastUpdated );
   EXPECT_DOUBLE_EQ( 0.5, rc.omega2t );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectQuery( { "get_rotor_calibration_info", "10" }, IUS_DB2::OK, false );
   }
   US_Rotor::RotorCalibration missing;
   missing.label = "previous";
   EXPECT_EQ( US_Rotor::NOT_FOUND, missing.readDB( 10, &db ) );
   EXPECT_EQ( QString( "previous" ), missing.label );
}

TEST_F( RotorDb, CalibrationProfilesReadEveryProfileOrNone )
{
   QVector<US_Rotor::RotorCalibration> profiles;
   {
      InSequence sequence;
      expectIds( { "get_rotor_calibration_profiles", "6" }, { "10", "11" } );
      expectQuery( { "get_rotor_calibration_info", "10" }, IUS_DB2::OK, true );
      for ( unsigned col : { 0u, 1u, 2u, 8u, 4u, 5u, 9u, 3u, 7u, 6u } )
      {
         EXPECT_CALL( db, value( col ) ).WillOnce( Return( QVariant( "1" ) ) );
      }
      expectQuery( { "get_rotor_calibration_info", "11" }, IUS_DB2::DBERROR, false );
   }
   EXPECT_EQ( US_Rotor::MISC_ERROR, US_Rotor::readCalibrationProfilesDB( profiles, 6, &db ) );
   EXPECT_TRUE( profiles.isEmpty() );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectQuery( { "get_rotor_calibration_profiles", "6" }, IUS_DB2::DBERROR, false );
   }
   EXPECT_EQ( US_Rotor::MISC_ERROR, US_Rotor::readCalibrationProfilesDB( profiles, 6, &db ) );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectIds( { "get_rotor_calibration_profiles", "6" }, {} );
   }
   EXPECT_EQ( US_Rotor::NOT_FOUND, US_Rotor::readCalibrationProfilesDB( profiles, 6, &db ) );
}

TEST_F( RotorDb, CalibrationSaveSetsTheIdOnlyOnSuccess )
{
   US_Rotor::RotorCalibration rc;
   rc.ID                      = -1;
   rc.GUID                    = "cal-guid";
   rc.report                  = "report";
   rc.coeff1                  = 0.25;
   rc.coeff2                  = -1.5;
   rc.omega2t                 = 0.5;
   rc.calibrationExperimentID = 44;
   rc.label                   = "Calibration 1";
   const QStringList query{ "add_rotor_calibration", "6", "cal-guid", "report", "0.25", "-1.5", "0.5", "44", "Calibration 1" };
   EXPECT_CALL( db, statusQuery( args( query ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
   EXPECT_EQ( IUS_DB2::DBERROR, rc.saveDB( 6, &db ) );
   EXPECT_EQ( -1, rc.ID );

   {
      InSequence sequence;
      EXPECT_CALL( db, statusQuery( args( query ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 12 ) );
   }
   EXPECT_EQ( IUS_DB2::OK, rc.saveDB( 6, &db ) );
   EXPECT_EQ( 12, rc.ID );
}

TEST_F( RotorDb, ReplaceDummyFindsTheDummyAmongTheOtherProfiles )
{
   US_Rotor::RotorCalibration rc;
   rc.ID      = 11;
   rc.rotorID = 6;
   {
      InSequence sequence;
      expectIds( { "get_rotor_calibration_profiles", "6" }, { "10", "11", "12" } );
      expectQuery( { "get_rotor_calibration_info", "10" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "real report" ) ) );
      EXPECT_CALL( db, value( 9 ) ).WillOnce( Return( QVariant( "Calibration 1" ) ) );
      expectQuery( { "get_rotor_calibration_info", "12" }, IUS_DB2::OK, true );
      EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "This is a dummy calibration --- please replace." ) ) );
      EXPECT_CALL( db, value( 9 ) ).WillOnce( Return( QVariant( "Dummy Calibration" ) ) );
      EXPECT_CALL( db, statusQuery( args( { "replace_rotor_calibration", "12", "11" } ) ) ).WillOnce( Return( IUS_DB2::OK ) );
   }
   int old = 0;
   EXPECT_EQ( IUS_DB2::OK, rc.replaceDummyDB( old, &db ) );
   EXPECT_EQ( 12, old );
}

TEST_F( RotorDb, ReplaceDummyHandlesAMissingNewProfileAndFailedQueries )
{
   US_Rotor::RotorCalibration rc;
   rc.ID      = 11;
   rc.rotorID = 6;
   int old    = 0;
   // The new profile is not listed (removing it must not use an invalid index).
   {
      InSequence sequence;
      expectIds( { "get_rotor_calibration_profiles", "6" }, {} );
   }
   EXPECT_EQ( IUS_DB2::OK, rc.replaceDummyDB( old, &db ) );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectQuery( { "get_rotor_calibration_profiles", "6" }, IUS_DB2::DBERROR, false );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, rc.replaceDummyDB( old, &db ) );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectIds( { "get_rotor_calibration_profiles", "6" }, { "10" } );
      expectQuery( { "get_rotor_calibration_info", "10" }, IUS_DB2::DBERROR, false );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, rc.replaceDummyDB( old, &db ) );
}
} // namespace
