// The edited-data database mechanics, exercised through IUS_DB2 rather than a
// live server.  Three programs used to carry a copy of this code each; these
// tests pin the stored-procedure arguments all three agreed on, so a migrated
// caller can be checked against the same expectations.

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock_us_db2.h"

#include "ius_db2.h"
#include "us_convertio.h"

#include <QStringList>
#include <QVariant>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::An;

namespace
{
const char* const kRawGUID  = "11111111-2222-3333-4444-555555555555";
const char* const kEditGUID = "66666666-7777-8888-9999-aaaaaaaaaaaa";

US_ConvertIO::EditedDataRecord sampleRecord()
{
   US_ConvertIO::EditedDataRecord record;
   record.rawDataID = 42;
   record.editGUID  = kEditGUID;
   record.label     = "demo-run";
   record.filename  = "demo-run.2401010000.RA.1.A.280.xml";
   record.comment   = "";

   return record;
}
}

class EditedDataOpsTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      db = std::make_unique< NiceMock< US_DB2_Mock > >();

      // Nothing failed unless a test says so.
      ON_CALL( *db, lastErrno() ).WillByDefault( Return( IUS_DB2::OK ) );
      ON_CALL( *db, lastError() ).WillByDefault( Return( QString() ) );
   }

   std::unique_ptr< NiceMock< US_DB2_Mock > > db;
   QString error;
};

// ------------------------------------------------------------ raw data lookup

TEST_F( EditedDataOpsTest, LooksUpARawDataIdByGuid )
{
   QStringList sent;
   EXPECT_CALL( *db, query( An< const QStringList& >() ) )
      .WillOnce( SaveArg< 0 >( &sent ) );
   EXPECT_CALL( *db, next() ).WillOnce( Return( true ) );
   EXPECT_CALL( *db, value( 0 ) ).WillOnce( Return( QVariant( 77 ) ) );

   int rawDataID = -1;
   EXPECT_EQ( US_ConvertIO::findRawDataId( db.get(), kRawGUID, rawDataID,
                                           error ),
              (int)IUS_DB2::OK );

   EXPECT_EQ( rawDataID, 77 );
   EXPECT_TRUE( error.isEmpty() );

   ASSERT_EQ( sent.size(), 2 );
   EXPECT_EQ( sent[ 0 ].toStdString(), "get_rawDataID_from_GUID" );
   EXPECT_EQ( sent[ 1 ].toStdString(), std::string( kRawGUID ) );
}

TEST_F( EditedDataOpsTest, DistinguishesAMissingRawRecordFromAFailedQuery )
{
   // The query worked and named no row.  That is data that was never
   // uploaded, not a broken connection, and callers treat the two differently.
   EXPECT_CALL( *db, query( An< const QStringList& >() ) );
   EXPECT_CALL( *db, next() ).WillOnce( Return( false ) );

   int rawDataID = -1;
   EXPECT_EQ( US_ConvertIO::findRawDataId( db.get(), kRawGUID, rawDataID,
                                           error ),
              (int)IUS_DB2::NO_RAWDATA );

   EXPECT_EQ( rawDataID, -1 );
   EXPECT_THAT( error.toStdString(), ::testing::HasSubstr( kRawGUID ) );
}

TEST_F( EditedDataOpsTest, ReturnsTheUnderlyingStatusWhenTheLookupQueryFails )
{
   EXPECT_CALL( *db, query( An< const QStringList& >() ) );
   EXPECT_CALL( *db, lastErrno() )
      .WillRepeatedly( Return( IUS_DB2::NOT_CONNECTED ) );
   EXPECT_CALL( *db, lastError() )
      .WillRepeatedly( Return( QString( "server has gone away" ) ) );

   // The row is never fetched once the query itself failed.
   EXPECT_CALL( *db, next() ).Times( 0 );

   int rawDataID = -1;
   EXPECT_EQ( US_ConvertIO::findRawDataId( db.get(), kRawGUID, rawDataID,
                                           error ),
              (int)IUS_DB2::NOT_CONNECTED );

   EXPECT_THAT( error.toStdString(),
                ::testing::HasSubstr( "server has gone away" ) );
}

// ---------------------------------------------------------------- create

TEST_F( EditedDataOpsTest, CreatesAnEditedDataRowWithTheAgreedArguments )
{
   QStringList sent;
   EXPECT_CALL( *db, query( An< const QStringList& >() ) )
      .WillOnce( SaveArg< 0 >( &sent ) );
   EXPECT_CALL( *db, lastInsertID() ).WillOnce( Return( 501 ) );

   const US_ConvertIO::EditedDataRecord record = sampleRecord();
   int editedDataID = -1;

   EXPECT_EQ( US_ConvertIO::createEditedData( db.get(), record, editedDataID,
                                              error ),
              (int)IUS_DB2::OK );

   EXPECT_EQ( editedDataID, 501 );
   EXPECT_TRUE( error.isEmpty() );

   ASSERT_EQ( sent.size(), 6 );
   EXPECT_EQ( sent[ 0 ].toStdString(), "new_editedData" );
   EXPECT_EQ( sent[ 1 ].toStdString(), "42" );
   EXPECT_EQ( sent[ 2 ].toStdString(), std::string( kEditGUID ) );
   EXPECT_EQ( sent[ 3 ].toStdString(), "demo-run" );
   EXPECT_EQ( sent[ 4 ].toStdString(),
              "demo-run.2401010000.RA.1.A.280.xml" );
   EXPECT_EQ( sent[ 5 ].toStdString(), "" );
}

TEST_F( EditedDataOpsTest, LeavesTheIdAloneWhenCreationFails )
{
   EXPECT_CALL( *db, query( An< const QStringList& >() ) );
   EXPECT_CALL( *db, lastErrno() )
      .WillRepeatedly( Return( IUS_DB2::DBERROR ) );
   EXPECT_CALL( *db, lastError() )
      .WillRepeatedly( Return( QString( "duplicate key" ) ) );
   EXPECT_CALL( *db, lastInsertID() ).Times( 0 );

   int editedDataID = -1;
   EXPECT_EQ( US_ConvertIO::createEditedData( db.get(), sampleRecord(),
                                              editedDataID, error ),
              (int)IUS_DB2::DBERROR );

   EXPECT_EQ( editedDataID, -1 );
   EXPECT_THAT( error.toStdString(),
                ::testing::HasSubstr( "duplicate key" ) );
   EXPECT_THAT( error.toStdString(),
                ::testing::HasSubstr( "demo-run.2401010000.RA.1.A.280.xml" ) );
}

// ---------------------------------------------------------------- update

TEST_F( EditedDataOpsTest, UpdatesAnEditedDataRowWithTheAgreedArguments )
{
   QStringList sent;
   EXPECT_CALL( *db, query( An< const QStringList& >() ) )
      .WillOnce( SaveArg< 0 >( &sent ) );

   EXPECT_EQ( US_ConvertIO::updateEditedData( db.get(), 501, sampleRecord(),
                                              error ),
              (int)IUS_DB2::OK );

   EXPECT_TRUE( error.isEmpty() );

   // The update procedure takes the row id ahead of the same five values.
   ASSERT_EQ( sent.size(), 7 );
   EXPECT_EQ( sent[ 0 ].toStdString(), "update_editedData" );
   EXPECT_EQ( sent[ 1 ].toStdString(), "501" );
   EXPECT_EQ( sent[ 2 ].toStdString(), "42" );
   EXPECT_EQ( sent[ 3 ].toStdString(), std::string( kEditGUID ) );
   EXPECT_EQ( sent[ 4 ].toStdString(), "demo-run" );
   EXPECT_EQ( sent[ 5 ].toStdString(),
              "demo-run.2401010000.RA.1.A.280.xml" );
   EXPECT_EQ( sent[ 6 ].toStdString(), "" );
}

TEST_F( EditedDataOpsTest, ReportsTheRowAndTheFileWhenAnUpdateFails )
{
   EXPECT_CALL( *db, query( An< const QStringList& >() ) );
   EXPECT_CALL( *db, lastErrno() )
      .WillRepeatedly( Return( IUS_DB2::DBERROR ) );

   EXPECT_EQ( US_ConvertIO::updateEditedData( db.get(), 501, sampleRecord(),
                                              error ),
              (int)IUS_DB2::DBERROR );

   EXPECT_THAT( error.toStdString(), ::testing::HasSubstr( "501" ) );
   EXPECT_THAT( error.toStdString(),
                ::testing::HasSubstr( "demo-run.2401010000.RA.1.A.280.xml" ) );
}

// ------------------------------------------------------------------- blob

TEST_F( EditedDataOpsTest, UploadsTheEditXmlThroughTheAgreedProcedure )
{
   QString   sentFile;
   QString   sentProcedure;
   int       sentID = 0;

   EXPECT_CALL( *db, writeBlobToDB( _, _, _ ) )
      .WillOnce( ::testing::DoAll( SaveArg< 0 >( &sentFile ),
                                   SaveArg< 1 >( &sentProcedure ),
                                   SaveArg< 2 >( &sentID ),
                                   Return( (int)IUS_DB2::OK ) ) );

   EXPECT_EQ( US_ConvertIO::uploadEditedDataBlob(
                 db.get(), 501, "/runs/demo/demo-run.xml", error ),
              (int)IUS_DB2::OK );

   EXPECT_TRUE( error.isEmpty() );
   EXPECT_EQ( sentFile.toStdString(), "/runs/demo/demo-run.xml" );
   EXPECT_EQ( sentProcedure.toStdString(), "upload_editData" );
   EXPECT_EQ( sentID, 501 );
}

TEST_F( EditedDataOpsTest, ReportsTheBlobStatusItWasGiven )
{
   // writeBlobToDB() returns its own status rather than setting lastErrno,
   // so that return value is what decides the outcome here.
   EXPECT_CALL( *db, writeBlobToDB( _, _, _ ) )
      .WillOnce( Return( (int)IUS_DB2::DBERROR ) );

   EXPECT_EQ( US_ConvertIO::uploadEditedDataBlob(
                 db.get(), 501, "/runs/demo/demo-run.xml", error ),
              (int)IUS_DB2::DBERROR );

   EXPECT_THAT( error.toStdString(),
                ::testing::HasSubstr( "/runs/demo/demo-run.xml" ) );
   EXPECT_THAT( error.toStdString(), ::testing::HasSubstr( "501" ) );
}

// ---------------------------------------------------------- null connection

TEST_F( EditedDataOpsTest, EveryOperationRefusesANullConnection )
{
   int value = -1;

   EXPECT_EQ( US_ConvertIO::findRawDataId( nullptr, kRawGUID, value, error ),
              (int)IUS_DB2::NOT_CONNECTED );
   EXPECT_FALSE( error.isEmpty() );

   error.clear();
   EXPECT_EQ( US_ConvertIO::createEditedData( nullptr, sampleRecord(), value,
                                              error ),
              (int)IUS_DB2::NOT_CONNECTED );
   EXPECT_FALSE( error.isEmpty() );

   error.clear();
   EXPECT_EQ( US_ConvertIO::updateEditedData( nullptr, 501, sampleRecord(),
                                              error ),
              (int)IUS_DB2::NOT_CONNECTED );
   EXPECT_FALSE( error.isEmpty() );

   error.clear();
   EXPECT_EQ( US_ConvertIO::uploadEditedDataBlob( nullptr, 501, "x.xml",
                                                  error ),
              (int)IUS_DB2::NOT_CONNECTED );
   EXPECT_FALSE( error.isEmpty() );
}

TEST_F( EditedDataOpsTest, NoOperationLeaksCredentialsIntoItsError )
{
   // lastError() is the only DB text these carry, and callers hand the result
   // to users and logs.  Nothing here composes a query string into `error`.
   EXPECT_CALL( *db, query( An< const QStringList& >() ) );
   EXPECT_CALL( *db, lastErrno() )
      .WillRepeatedly( Return( IUS_DB2::DBERROR ) );
   EXPECT_CALL( *db, lastError() )
      .WillRepeatedly( Return( QString( "access denied" ) ) );

   int editedDataID = -1;
   US_ConvertIO::createEditedData( db.get(), sampleRecord(), editedDataID,
                                   error );

   EXPECT_THAT( error.toStdString(),
                ::testing::Not( ::testing::HasSubstr( "CALL " ) ) );
   EXPECT_THAT( error.toStdString(),
                ::testing::Not( ::testing::HasSubstr( "new_editedData" ) ) );
}
