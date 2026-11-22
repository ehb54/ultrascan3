//! \file test_us_eprofile_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_eprofile.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

namespace
{
const QString               kProfileXml = "<EProfileData version='1.0'><extinctionProfile componentID='5' componentType='Buffer' "
                                          "valueType='absorbance'><extinction wavelength='260' value='0.5'/>"
                                          "<extinction wavelength='280' value='1.25'/></extinctionProfile></EProfileData>";

Matcher<const QStringList&> args( const QStringList& expected )
{
   return Matcher<const QStringList&>( Eq( expected ) );
}

QMap<double, double> profileValues( QString xml )
{
   QMap<double, double> values;
   EXPECT_TRUE( US_ExtProfile::parse_eprofile( xml, nullptr, nullptr, nullptr, values ) );
   return values;
}

class ExtProfileDb : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock>    db;
      QString                    compType  = "Buffer";
      QString                    valueType = "stale type";
      QMap<double, double>       spectrum{ { 999.0, 9.0 } };
      const QMap<double, double> stale{ { 999.0, 9.0 } };
      const QMap<double, double> written{ { 250.0, 0.75 }, { 280.0, 2.5 } };

      void                       expectLookup( int status )
      {
         EXPECT_CALL( db, query( args( { "get_eprofile", "5", "Buffer" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
      }

      void expectRow( const QString& xml )
      {
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "9" ) ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "absorbance" ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( xml ) ) );
      }

      // Captures the XML of a status query whose leading arguments are expected.
      void expectWrite( const QStringList& head, QString& xml, int status )
      {
         EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
            .WillOnce( Invoke(
               [head, &xml, status]( const QStringList& query )
               {
                  EXPECT_EQ( head, query.mid( 0, head.size() ) );
                  EXPECT_EQ( head.size() + 1, query.size() );
                  xml = query.value( head.size() );
                  return status;
               } ) );
      }
};

// Fetch

TEST_F( ExtProfileDb, FetchReturnsProfileIdTypeAndSpectrum )
{
   {
      InSequence sequence;
      expectLookup( IUS_DB2::OK );
      expectRow( kProfileXml );
   }
   EXPECT_EQ( 9, US_ExtProfile::fetch_eprofile( &db, 5, compType, valueType, spectrum ) );
   EXPECT_EQ( QString( "absorbance" ), valueType );
   EXPECT_EQ( QString( "Buffer" ), compType );
   EXPECT_EQ( ( QMap<double, double>{ { 260.0, 0.5 }, { 280.0, 1.25 } } ), spectrum );
}

TEST_F( ExtProfileDb, FetchFailureOrMissingRowKeepsOutputs )
{
   for ( int status : { IUS_DB2::NOROWS, IUS_DB2::DBERROR, IUS_DB2::NOT_CONNECTED } )
   {
      SCOPED_TRACE( status );
      {
         InSequence sequence;
         expectLookup( status );
      }
      EXPECT_EQ( -1, US_ExtProfile::fetch_eprofile( &db, 5, compType, valueType, spectrum ) );
      EXPECT_EQ( QString( "stale type" ), valueType );
      EXPECT_EQ( stale, spectrum );
      ::testing::Mock::VerifyAndClearExpectations( &db );
   }

   {
      InSequence sequence;
      expectLookup( IUS_DB2::OK );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
   }
   EXPECT_EQ( -1, US_ExtProfile::fetch_eprofile( &db, 5, compType, valueType, spectrum ) );
   EXPECT_EQ( QString( "stale type" ), valueType );
   EXPECT_EQ( stale, spectrum );
}

TEST_F( ExtProfileDb, FetchRejectsInvalidXmlOrMissingProfile )
{
   for ( const QString& xml : { QString(),
                                QString( "not xml" ),
                                QString( "<EProfileData version='1.0'/>" ),
                                QString( "<extinctionProfile componentID='5' valueType='changed'><extinction value='1'/>" ),
                                kProfileXml + "<broken" } )
   {
      SCOPED_TRACE( xml.toStdString() );
      {
         InSequence sequence;
         expectLookup( IUS_DB2::OK );
         expectRow( xml );
      }
      EXPECT_EQ( -1, US_ExtProfile::fetch_eprofile( &db, 5, compType, valueType, spectrum ) );
      EXPECT_EQ( QString( "stale type" ), valueType );
      EXPECT_EQ( stale, spectrum );
      ::testing::Mock::VerifyAndClearExpectations( &db );
   }
}

// Create

TEST_F( ExtProfileDb, CreateInsertsWhenNoProfileExists )
{
   valueType = "absorbance";
   for ( int status : { IUS_DB2::NOROWS, IUS_DB2::OK } )
   {
      SCOPED_TRACE( status );
      QString xml;
      {
         InSequence sequence;
         expectLookup( status );
         if ( status == IUS_DB2::OK )
         {
            EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
         }
         expectWrite( { "new_eprofile", "5", "Buffer", "absorbance" }, xml, IUS_DB2::OK );
         EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 31 ) );
      }
      QMap<double, double> values = written;
      EXPECT_EQ( 31, US_ExtProfile::create_eprofile( &db, 5, compType, valueType, values ) );
      EXPECT_EQ( written, profileValues( xml ) );
      EXPECT_EQ( written, values );
      ::testing::Mock::VerifyAndClearExpectations( &db );
   }
}

TEST_F( ExtProfileDb, CreateUpdatesTheExistingProfile )
{
   valueType = "absorbance";
   QString xml;
   {
      InSequence sequence;
      expectLookup( IUS_DB2::OK );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "12" ) ) );
      expectWrite( { "update_eprofile", "12", "5", "Buffer", "absorbance" }, xml, IUS_DB2::OK );
   }
   QMap<double, double> values = written;
   EXPECT_EQ( 12, US_ExtProfile::create_eprofile( &db, 5, compType, valueType, values ) );
   EXPECT_EQ( written, profileValues( xml ) );
}

TEST_F( ExtProfileDb, CreateFailedLookupWritesNothing )
{
   for ( int status : { IUS_DB2::DBERROR, IUS_DB2::NOT_CONNECTED } )
   {
      SCOPED_TRACE( status );
      {
         InSequence sequence;
         expectLookup( status );
      }
      QMap<double, double> values = written;
      EXPECT_EQ( -1, US_ExtProfile::create_eprofile( &db, 5, compType, valueType, values ) );
      ::testing::Mock::VerifyAndClearExpectations( &db );
   }
}

TEST_F( ExtProfileDb, CreateReportsFailedWrites )
{
   QString xml;
   {
      InSequence sequence;
      expectLookup( IUS_DB2::NOROWS );
      expectWrite( { "new_eprofile", "5", "Buffer", "stale type" }, xml, IUS_DB2::DBERROR );
   }
   QMap<double, double> values = written;
   EXPECT_EQ( -1, US_ExtProfile::create_eprofile( &db, 5, compType, valueType, values ) );
   ::testing::Mock::VerifyAndClearExpectations( &db );

   {
      InSequence sequence;
      expectLookup( IUS_DB2::OK );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "12" ) ) );
      expectWrite( { "update_eprofile", "12", "5", "Buffer", "stale type" }, xml, IUS_DB2::DBERROR );
   }
   EXPECT_EQ( -1, US_ExtProfile::create_eprofile( &db, 5, compType, valueType, values ) );
}

// Update and delete

TEST_F( ExtProfileDb, UpdateSendsProfileAndReportsStatus )
{
   valueType = "absorbance";
   for ( int status : { IUS_DB2::OK, IUS_DB2::DBERROR } )
   {
      SCOPED_TRACE( status );
      QString xml;
      {
         InSequence sequence;
         expectWrite( { "update_eprofile", "12", "5", "Buffer", "absorbance" }, xml, status );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
      }
      QMap<double, double> values = written;
      EXPECT_EQ( status == IUS_DB2::OK, US_ExtProfile::update_eprofile( &db, 12, 5, compType, valueType, values ) );
      EXPECT_EQ( written, profileValues( xml ) );
      ::testing::Mock::VerifyAndClearExpectations( &db );
   }
}

TEST_F( ExtProfileDb, DeleteSendsComponentAndReportsStatus )
{
   for ( int status : { IUS_DB2::OK, IUS_DB2::DBERROR } )
   {
      SCOPED_TRACE( status );
      {
         InSequence sequence;
         EXPECT_CALL( db, statusQuery( args( { "delete_eprofile", "5", "Buffer" } ) ) ).WillOnce( Return( status ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
      }
      EXPECT_EQ( status == IUS_DB2::OK, US_ExtProfile::delete_eprofile( &db, 5, compType ) );
      ::testing::Mock::VerifyAndClearExpectations( &db );
   }
}
} // namespace
