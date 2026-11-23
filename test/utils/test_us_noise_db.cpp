//! \file test_us_noise_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_noise.h"
#include "us_settings.h"
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

using ::testing::_;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

namespace
{
const QString               kNoiseGuid = "dddddddd-1111-2222-3333-444444444444";
const QString               kModelGuid = "eeeeeeee-1111-2222-3333-444444444444";

Matcher<const QStringList&> args( const QStringList& expected )
{
   return Matcher<const QStringList&>( Eq( expected ) );
}

QString storedXml( const QString& description )
{
   return QString( "<?xml version=\"1.0\"?><!DOCTYPE US_Noise><NoiseData version=\"1.0\">"
                   "<noise type=\"ri\" description=\"%1\" modelGUID=\"%2\" noiseGUID=\"%3\">"
                   "<d v=\"0.5\"/><d v=\"-0.25\"/><d v=\"0.125\"/></noise></NoiseData>" )
      .arg( description, kModelGuid, kNoiseGuid );
}

class NoiseDb : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock> db;
      US_Noise                noise;

      void                    SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         US_Settings::set_us_inv_ID( 23 );
      }

      // A loaded noise that a failed load must leave as it was.
      void prepareLoaded()
      {
         noise.type        = US_Noise::TI;
         noise.description = "previous";
         noise.noiseGUID   = "previous-guid";
         noise.values      = { 1.0, 2.0 };
         noise.count       = 2;
      }

      void expectLoadedUnchanged()
      {
         EXPECT_EQ( US_Noise::TI, noise.type );
         EXPECT_EQ( QString( "previous" ), noise.description );
         EXPECT_EQ( QString( "previous-guid" ), noise.noiseGUID );
         EXPECT_EQ( ( QVector<double>{ 1.0, 2.0 } ), noise.values );
         EXPECT_EQ( 2, noise.count );
      }

      void expectInfo( const QString& xml )
      {
         EXPECT_CALL( db, query( args( { "get_noise_info", "12" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 5 ) ).WillOnce( Return( QVariant( xml ) ) );
      }

      void prepareToWrite()
      {
         noise.type        = US_Noise::TI;
         noise.description = "run.ti_noise";
         noise.noiseGUID   = kNoiseGuid;
         noise.modelGUID   = kModelGuid;
         noise.minradius   = 5.9;
         noise.maxradius   = 7.1;
         noise.values      = { 0.5, -0.25 };
         noise.count       = 2;
      }

      // The model ID 42 and its edit ID 77 from the investigator's model list.
      void expectModelAndEdit()
      {
         EXPECT_CALL( db, query( args( { "get_modelID", kModelGuid } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "42" ) ) );
         EXPECT_CALL( db, query( args( { "get_model_desc", "23" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "41" ) ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "42" ) ) );
         EXPECT_CALL( db, value( 6 ) ).WillOnce( Return( QVariant( "77" ) ) );
      }

      // Captures a status query whose leading arguments are expected.
      void expectWrite( const QStringList& head, QString& xml, int status )
      {
         EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
            .WillOnce( Invoke(
               [head, &xml, status]( const QStringList& q )
               {
                  EXPECT_EQ( head, q.mid( 0, head.size() ) );
                  xml = q.value( head.size() );
                  return status;
               } ) );
      }
};

// Load

TEST_F( NoiseDb, LoadByGuidReadsTheStoredXml )
{
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_noiseID", kNoiseGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "12" ) ) );
      expectInfo( storedXml( "run.ri_noise" ) );
   }
   ASSERT_EQ( IUS_DB2::OK, noise.load( true, kNoiseGuid, &db ) );
   EXPECT_EQ( US_Noise::RI, noise.type );
   EXPECT_EQ( QString( "run.ri_noise" ), noise.description );
   EXPECT_EQ( kNoiseGuid, noise.noiseGUID );
   EXPECT_EQ( kModelGuid, noise.modelGUID );
   EXPECT_EQ( ( QVector<double>{ 0.5, -0.25, 0.125 } ), noise.values );
   EXPECT_EQ( 3, noise.count );
}

TEST_F( NoiseDb, NonAsciiDescriptionLoadsIntact )
{
   const QString description = QString( QChar( 0x03bb ) ) + " 280 nm " + QChar( 0x00c5 );
   {
      InSequence sequence;
      expectInfo( storedXml( description ) );
   }
   ASSERT_EQ( IUS_DB2::OK, noise.load( QString( "12" ), &db ) );
   EXPECT_EQ( description, noise.description );
   EXPECT_EQ( 3, noise.count );
}

TEST_F( NoiseDb, NullDatabaseIsNotConnected )
{
   prepareLoaded();
   EXPECT_EQ( IUS_DB2::NOT_CONNECTED, noise.load( true, kNoiseGuid, nullptr ) );
   EXPECT_EQ( IUS_DB2::NOT_CONNECTED, noise.load( QString( "12" ), nullptr ) );
   EXPECT_EQ( IUS_DB2::NOT_CONNECTED, noise.write( true, QString(), nullptr ) );
   expectLoadedUnchanged();
}

TEST_F( NoiseDb, FailedOrMissingRowsKeepTheNoise )
{
   prepareLoaded();
   for ( const bool lookup : { true, false } )
   {
      for ( int status : { IUS_DB2::DBERROR, IUS_DB2::OK } )
      {
         SCOPED_TRACE( std::to_string( lookup ) + " " + std::to_string( status ) );
         {
            InSequence sequence;
            if ( lookup )
            {
               EXPECT_CALL( db, query( args( { "get_noiseID", kNoiseGuid } ) ) );
            }
            else
            {
               EXPECT_CALL( db, query( args( { "get_noise_info", "12" } ) ) );
            }
            EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
            if ( status == IUS_DB2::OK )
            {
               EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
            }
         }
         const int result = lookup ? noise.load( true, kNoiseGuid, &db ) : noise.load( QString( "12" ), &db );
         EXPECT_EQ( status == IUS_DB2::OK ? IUS_DB2::NOROWS : status, result );
         expectLoadedUnchanged();
         ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
      }
   }
}

TEST_F( NoiseDb, TruncatedXmlFailsAndKeepsTheNoise )
{
   prepareLoaded();
   const QString xml = storedXml( "run.ri_noise" );
   {
      InSequence sequence;
      expectInfo( xml.left( xml.indexOf( "<d v=\"-0.25\"" ) + 4 ) );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, noise.load( QString( "12" ), &db ) );
   expectLoadedUnchanged();
}

TEST_F( NoiseDb, TruncatedLocalFileFailsAndKeepsTheNoise )
{
   prepareLoaded();
   QTemporaryDir dir;
   ASSERT_TRUE( dir.isValid() );
   const QString xml  = storedXml( "run.ri_noise" );
   const QString path = dir.filePath( "noise.xml" );
   {
      QFile file( path );
      ASSERT_TRUE( file.open( QIODevice::WriteOnly ) );
      file.write( xml.left( xml.indexOf( "<d v=\"-0.25\"" ) + 4 ).toUtf8() );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, noise.load( path ) );
   expectLoadedUnchanged();
}

// Write

TEST_F( NoiseDb, WriteInsertsWithTheModelAndEditIds )
{
   prepareToWrite();
   QString xml;
   {
      InSequence sequence;
      expectModelAndEdit();
      EXPECT_CALL( db, query( args( { "get_noiseID", kNoiseGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      expectWrite( { "new_noise", kNoiseGuid, "77", "42", kModelGuid, "ti_noise", "run.ti_noise" }, xml, IUS_DB2::OK );
   }
   EXPECT_EQ( IUS_DB2::OK, noise.write( true, QString(), &db ) );
   EXPECT_EQ( QString( "created" ), noise.message );

   // The sent XML reads back as the same noise.
   const QString path = QDir::currentPath() + "/sent-noise.xml";
   {
      QFile file( path );
      ASSERT_TRUE( file.open( QIODevice::WriteOnly ) );
      file.write( xml.toUtf8() );
   }
   US_Noise written;
   ASSERT_EQ( IUS_DB2::OK, written.load( path ) );
   EXPECT_EQ( noise.values, written.values );
   EXPECT_EQ( kNoiseGuid, written.noiseGUID );
}

TEST_F( NoiseDb, WriteUpdatesAStoredNoise )
{
   prepareToWrite();
   QString xml;
   {
      InSequence sequence;
      expectModelAndEdit();
      EXPECT_CALL( db, query( args( { "get_noiseID", kNoiseGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "12" ) ) );
      expectWrite( { "update_noise", "12", kNoiseGuid, "77", "42", kModelGuid, "ti_noise", "run.ti_noise" }, xml, IUS_DB2::OK );
   }
   EXPECT_EQ( IUS_DB2::OK, noise.write( true, QString(), &db ) );
   EXPECT_EQ( QString( "updated" ), noise.message );
}

TEST_F( NoiseDb, WriteTreatsAnEmptyNoiseLookupAsNew )
{
   prepareToWrite();
   QString xml;
   {
      InSequence sequence;
      expectModelAndEdit();
      EXPECT_CALL( db, query( args( { "get_noiseID", kNoiseGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
      expectWrite( { "new_noise", kNoiseGuid, "77", "42" }, xml, IUS_DB2::OK );
   }
   EXPECT_EQ( IUS_DB2::OK, noise.write( true, QString(), &db ) );
}

TEST_F( NoiseDb, MissingOrFailedModelLookupWritesNothing )
{
   prepareToWrite();
   for ( int status : { IUS_DB2::DBERROR, IUS_DB2::NOROWS, IUS_DB2::OK } )
   {
      SCOPED_TRACE( status );
      {
         InSequence sequence;
         EXPECT_CALL( db, query( args( { "get_modelID", kModelGuid } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
         if ( status == IUS_DB2::OK )
         {
            EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
         }
      }
      const int expected = status == IUS_DB2::DBERROR ? IUS_DB2::DBERROR : IUS_DB2::NO_MODEL;
      EXPECT_EQ( expected, noise.write( true, QString(), &db ) );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( NoiseDb, FailedEditOrNoiseLookupWritesNothing )
{
   prepareToWrite();
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_modelID", kModelGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "42" ) ) );
      EXPECT_CALL( db, query( args( { "get_model_desc", "23" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, noise.write( true, QString(), &db ) );
   ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );

   {
      InSequence sequence;
      expectModelAndEdit();
      EXPECT_CALL( db, query( args( { "get_noiseID", kNoiseGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, noise.write( true, QString(), &db ) );
}

TEST_F( NoiseDb, WriteReturnsTheStatusOfAFailedWrite )
{
   prepareToWrite();
   QString xml;
   {
      InSequence sequence;
      expectModelAndEdit();
      EXPECT_CALL( db, query( args( { "get_noiseID", kNoiseGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      expectWrite( { "new_noise" }, xml, IUS_DB2::DBERROR );
   }
   EXPECT_EQ( IUS_DB2::DBERROR, noise.write( true, QString(), &db ) );
   EXPECT_EQ( QString( "could not be written" ), noise.message );
}
} // namespace
