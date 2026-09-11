// The DB-backed edited-data reader is deliberately policy-free: callers pick
// records and decide whether files are cached, while this utility owns the
// download/decode mechanics.  These tests exercise that boundary without a
// live database.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_us_db2.h"
#include "us_convertio.h"
#include "us_dataIO.h"
#include "us_util.h"

#include <QFile>
#include <QTemporaryDir>

using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;

namespace
{
const QString kEditName = "demo.2401010000.RA.1.A.280.xml";
const QString kRawName  = "demo.RA.1.A.280.auc";

US_DataIO::RawData sampleRawData()
{
   US_DataIO::RawData raw;
   memcpy( raw.type, "RA", 2 );
   memcpy( raw.rawGUID, "0123456789abcdef", 16 );
   raw.cell        = 1;
   raw.channel     = 'A';
   raw.description = "edited-data DB read fixture";
   raw.xvalues << 5.8 << 5.9 << 6.0;

   US_DataIO::Scan scan;
   scan.temperature  = 20.0;
   scan.rpm          = 50000.0;
   scan.seconds      = 300;
   scan.omega2t      = 2.0e10;
   scan.wavelength   = 280.0;
   scan.delta_r      = 0.1;
   scan.rvalues << 0.1 << 0.2 << 0.3;
   scan.interpolated = QByteArray( 1, '\0' );
   raw.scanData << scan;

   return raw;
}

US_DataIO::EditValues sampleEditValues( const US_DataIO::RawData& raw,
                                        const QString& wavelength = "280" )
{
   US_DataIO::EditValues edit;
   edit.expType    = "Velocity";
   edit.runID      = "demo";
   edit.cell       = "1";
   edit.channel    = "A";
   edit.wavelength = wavelength;
   edit.editGUID   = "11111111-2222-3333-4444-555555555555";
   edit.dataGUID   = US_Util::uuid_unparse(
      (uchar*)const_cast< char* >( raw.rawGUID ) );
   edit.meniscus   = 5.8;
   edit.bottom     = 7.2;
   edit.rangeLeft  = 5.8;
   edit.rangeRight = 6.0;
   edit.plateau    = 5.9;
   edit.baseline   = 5.8;
   edit.ODlimit    = 1.0;
   return edit;
}
}

class EditedDataReadTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      ASSERT_TRUE( source.isValid() );
      ASSERT_TRUE( destination.isValid() );

      raw = sampleRawData();
      ASSERT_EQ( US_DataIO::writeRawData( source.filePath( kRawName ), raw ),
                 (int)US_DataIO::OK );
      ASSERT_EQ( US_DataIO::writeEdits( source.filePath( kEditName ),
                                        sampleEditValues( raw ) ),
                 (int)US_DataIO::OK );

      ON_CALL( db, lastError() ).WillByDefault( Return( QString() ) );
   }

   US_ConvertIO::EditedDataReadRequest request() const
   {
      US_ConvertIO::EditedDataReadRequest value;
      value.directory    = destination.path();
      value.editFilename = kEditName;
      value.editedDataID = 41;
      value.rawDataID    = 42;
      return value;
   }

   int copyFixture( const QString& destinationPath,
                    const QString& procedure, int id )
   {
      const QString sourcePath = procedure == "download_editData"
                               ? source.filePath( kEditName )
                               : source.filePath( kRawName );
      const bool expected = ( procedure == "download_editData"  &&  id == 41 )
                         || ( procedure == "download_aucData"   &&  id == 42 );
      return expected  &&  QFile::copy( sourcePath, destinationPath )
           ? IUS_DB2::OK : IUS_DB2::DBERROR;
   }

   QTemporaryDir source;
   QTemporaryDir destination;
   NiceMock< US_DB2_Mock > db;
   US_DataIO::RawData raw;
   QVector< US_DataIO::EditedData > edited;
   QVector< US_DataIO::RawData > loadedRaw;
   QString error;
};

TEST_F( EditedDataReadTest, OneFilenameRuleServesNormalAndMwlEdits )
{
   EXPECT_EQ( US_DataIO::rawFilenameForEdit( kEditName ), kRawName );
   EXPECT_EQ( US_DataIO::rawFilenameForEdit(
                 "demo.2401010000.RA.1.A.190-800@280.xml" ), kRawName );
   EXPECT_TRUE( US_DataIO::rawFilenameForEdit( "broken.xml" ).isEmpty() );
}

TEST_F( EditedDataReadTest, DownloadsThroughTheCanonicalProceduresAndDecodes )
{
   EXPECT_CALL( db, readBlobFromDB( _, Eq( QString( "download_editData" ) ),
                                    41 ) )
      .WillOnce( Invoke( [ this ]( const QString& path,
                                   const QString& procedure, int id )
                         { return copyFixture( path, procedure, id ); } ) );
   EXPECT_CALL( db, readBlobFromDB( _, Eq( QString( "download_aucData" ) ),
                                    42 ) )
      .WillOnce( Invoke( [ this ]( const QString& path,
                                   const QString& procedure, int id )
                         { return copyFixture( path, procedure, id ); } ) );

   EXPECT_EQ( US_ConvertIO::readEditedDataFromDB(
                 &db, request(), edited, loadedRaw, error ),
              (int)IUS_DB2::OK );
   EXPECT_TRUE( error.isEmpty() );
   ASSERT_EQ( edited.size(), 1 );
   ASSERT_EQ( loadedRaw.size(), 1 );
   EXPECT_EQ( edited[ 0 ].runID, QString( "demo" ) );
   EXPECT_EQ( edited[ 0 ].wavelength, QString( "280" ) );
   EXPECT_EQ( loadedRaw[ 0 ].description,
              QString( "edited-data DB read fixture" ) );
}

TEST_F( EditedDataReadTest, AnEditDownloadFailureStopsBeforeRawAndClearsOutput )
{
   edited << US_DataIO::EditedData();
   loadedRaw << US_DataIO::RawData();

   EXPECT_CALL( db, readBlobFromDB( _, Eq( QString( "download_editData" ) ),
                                    41 ) )
      .WillOnce( Return( (int)IUS_DB2::BAD_CHECKSUM ) );
   EXPECT_CALL( db, readBlobFromDB( _, Eq( QString( "download_aucData" ) ),
                                    _ ) ).Times( 0 );
   ON_CALL( db, lastError() ).WillByDefault( Return( "checksum mismatch" ) );

   EXPECT_EQ( US_ConvertIO::readEditedDataFromDB(
                 &db, request(), edited, loadedRaw, error ),
              (int)IUS_DB2::BAD_CHECKSUM );
   EXPECT_TRUE( edited.isEmpty() );
   EXPECT_TRUE( loadedRaw.isEmpty() );
   EXPECT_THAT( error.toStdString(), ::testing::HasSubstr( "41" ) );
   EXPECT_THAT( error.toStdString(),
                ::testing::HasSubstr( "checksum mismatch" ) );
}

TEST_F( EditedDataReadTest, ARawDownloadFailureDoesNotPublishTheLoadedEdit )
{
   EXPECT_CALL( db, readBlobFromDB( _, Eq( QString( "download_editData" ) ),
                                    41 ) )
      .WillOnce( Invoke( [ this ]( const QString& path,
                                   const QString& procedure, int id )
                         { return copyFixture( path, procedure, id ); } ) );
   EXPECT_CALL( db, readBlobFromDB( _, Eq( QString( "download_aucData" ) ),
                                    42 ) )
      .WillOnce( Return( (int)IUS_DB2::DBERROR ) );
   ON_CALL( db, lastError() ).WillByDefault( Return( "raw blob unavailable" ) );

   EXPECT_EQ( US_ConvertIO::readEditedDataFromDB(
                 &db, request(), edited, loadedRaw, error ),
              (int)IUS_DB2::DBERROR );
   EXPECT_TRUE( edited.isEmpty() );
   EXPECT_TRUE( loadedRaw.isEmpty() );
   EXPECT_THAT( error.toStdString(), ::testing::HasSubstr( "42" ) );
   EXPECT_THAT( error.toStdString(),
                ::testing::HasSubstr( "raw blob unavailable" ) );
}

TEST_F( EditedDataReadTest, ExistingFilesCanBeUsedWithoutADatabase )
{
   ASSERT_TRUE( QFile::copy( source.filePath( kEditName ),
                             destination.filePath( kEditName ) ) );
   ASSERT_TRUE( QFile::copy( source.filePath( kRawName ),
                             destination.filePath( kRawName ) ) );

   US_ConvertIO::EditedDataReadRequest cached = request();
   cached.downloadEdit = false;
   cached.downloadRaw  = false;

   EXPECT_EQ( US_ConvertIO::readEditedDataFromDB(
                 nullptr, cached, edited, loadedRaw, error ),
              (int)IUS_DB2::OK );
   ASSERT_EQ( edited.size(), 1 );
   ASSERT_EQ( loadedRaw.size(), 1 );
}

TEST_F( EditedDataReadTest, LogicalMwlNameCanDifferFromStoredEditName )
{
   const QString stored = "demo.2401010000.RA.1.A.190-800.xml";
   const QString logical = "demo.2401010000.RA.1.A.190-800@280.xml";

   ASSERT_EQ( US_DataIO::writeEdits(
                 destination.filePath( stored ),
                 sampleEditValues( raw, "190-800" ) ),
              (int)US_DataIO::OK );
   ASSERT_TRUE( QFile::copy( source.filePath( kRawName ),
                             destination.filePath( kRawName ) ) );

   US_ConvertIO::EditedDataReadRequest mwl = request();
   mwl.editFilename = stored;
   mwl.loadFilename = logical;
   mwl.rawFilename  = kRawName;
   mwl.downloadEdit = false;
   mwl.downloadRaw  = false;

   EXPECT_EQ( US_ConvertIO::readEditedDataFromDB(
                 nullptr, mwl, edited, loadedRaw, error ),
              (int)IUS_DB2::OK );
   ASSERT_EQ( edited.size(), 1 );
   EXPECT_EQ( edited[ 0 ].wavelength, QString( "280" ) );
}

TEST_F( EditedDataReadTest, DecodeFailureDoesNotPublishPartialRawData )
{
   QFile invalidEdit( destination.filePath( kEditName ) );
   ASSERT_TRUE( invalidEdit.open( QIODevice::WriteOnly | QIODevice::Text ) );
   invalidEdit.write( "<not-an-edit>" );
   invalidEdit.close();
   ASSERT_TRUE( QFile::copy( source.filePath( kRawName ),
                             destination.filePath( kRawName ) ) );

   US_ConvertIO::EditedDataReadRequest cached = request();
   cached.downloadEdit = false;
   cached.downloadRaw  = false;

   EXPECT_EQ( US_ConvertIO::readEditedDataFromDB(
                 nullptr, cached, edited, loadedRaw, error ),
              (int)US_ConvertIO::EDITED_DATA_DECODE_ERROR );
   EXPECT_TRUE( edited.isEmpty() );
   EXPECT_TRUE( loadedRaw.isEmpty() );
   EXPECT_THAT( error.toStdString(),
                ::testing::HasSubstr( "XML file was invalid" ) );
}

TEST_F( EditedDataReadTest, InvalidNamesAreRejectedBeforeAnyDownload )
{
   US_ConvertIO::EditedDataReadRequest invalid = request();
   invalid.editFilename = "../outside.xml";

   EXPECT_CALL( db, readBlobFromDB( _, _, _ ) ).Times( 0 );
   EXPECT_EQ( US_ConvertIO::readEditedDataFromDB(
                 &db, invalid, edited, loadedRaw, error ),
              (int)US_ConvertIO::EDITED_DATA_INVALID_REQUEST );
   EXPECT_FALSE( error.isEmpty() );
}
