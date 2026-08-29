// Program-level tests for the edited-data loading paths shared by
// US_Analysis_auto::loadData() and US_ReporterGMP::loadData().

#include "us3_gui_test_main.h"

#include "us_edited_data_loaders.h"
#include "us_util.h"

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
   raw.description = "program loader fixture";
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

US_DataIO::EditValues sampleEditValues( const US_DataIO::RawData& raw )
{
   US_DataIO::EditValues edit;
   edit.expType    = "Velocity";
   edit.runID      = "demo";
   edit.cell       = "1";
   edit.channel    = "A";
   edit.wavelength = "280";
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

struct FixtureFiles
{
   FixtureFiles()
   {
      US_DataIO::RawData raw = sampleRawData();
      valid = source.isValid() && destination.isValid()
           && US_DataIO::writeRawData( source.filePath( kRawName ), raw )
                 == US_DataIO::OK
           && US_DataIO::writeEdits( source.filePath( kEditName ),
                                     sampleEditValues( raw ) )
                 == US_DataIO::OK;
   }

   QTemporaryDir source;
   QTemporaryDir destination;
   bool valid = false;
};

class FakeDB : public IUS_DB2
{
public:
   using Row = QVector< QVariant >;

   bool connect( const QString&, QString& ) override { return true; }
   bool connect( const QString&, const QString&, const QString&,
                 const QString&, QString& ) override { return true; }
   bool beginTransaction( QString& ) override { return true; }
   bool commitTransaction( QString& ) override { return true; }
   bool rollbackTransaction( QString& ) override { return true; }
   void query( const QString& query ) override
      { stringQuery = query; rowIndex = -1; }
   void query( const QStringList& query ) override
      { listQuery = query; rowIndex = -1; }
   int statusQuery( const QString& ) override { return status; }
   int statusQuery( const QStringList& ) override { return status; }
   bool next() override { return ++rowIndex < rows.size(); }
   QVariant value( unsigned index ) override
   {
      return rowIndex >= 0  &&  rowIndex < rows.size()
             &&  index < (unsigned)rows[ rowIndex ].size()
           ? rows[ rowIndex ][ index ] : QVariant();
   }
   bool isConnected() override { return true; }
   QString lastError() override { return error; }
   int lastErrno() override { return status; }
   int lastInsertID() override { return -1; }
   bool test_db_connection( const QString&, const QString&, const QString&,
                            const QString&, QString& ) override { return true; }
   bool test_secure_connection( const QString&, const QString&,
                                const QString&, const QString&,
                                const QString&, const QString&,
                                QString& ) override { return true; }
   int writeBlobToDB( const QString&, const QString&, int ) override
      { return IUS_DB2::OK; }
   int readBlobFromDB( const QString& path, const QString& procedure,
                       int id ) override
   {
      blobCalls << QString( "%1:%2" ).arg( procedure ).arg( id );
      if ( procedure == failProcedure )
         return blobStatus;

      const QString sourcePath = procedure == "download_editData"
                               ? editFixture : rawFixture;
      return QFile::copy( sourcePath, path )
           ? IUS_DB2::OK : IUS_DB2::DBERROR;
   }
   int writeAucToDB( const QString&, int ) override { return IUS_DB2::OK; }
   int readAucFromDB( const QString&, int ) override { return IUS_DB2::OK; }
   void rawQuery( const QString& ) override {}
   int functionQuery( const QStringList& ) override { return status; }
   int numRows() override { return rows.size(); }
   QString lastDebug() override { return QString(); }
   unsigned long mysqlEscapeString( QByteArray& to, QByteArray& from,
                                    unsigned long length ) override
   {
      to = from.left( length );
      return to.size();
   }

   QVector< Row > rows;
   int rowIndex = -1;
   int status = IUS_DB2::OK;
   int blobStatus = IUS_DB2::DBERROR;
   QString error;
   QString editFixture;
   QString rawFixture;
   QString failProcedure;
   QString stringQuery;
   QStringList listQuery;
   QStringList blobCalls;
};

FakeDB databaseFor( const FixtureFiles& files )
{
   FakeDB db;
   db.editFixture = files.source.filePath( kEditName );
   db.rawFixture  = files.source.filePath( kRawName );
   return db;
}
}

class US_EditedDataLoadersTest : public QObject
{
   Q_OBJECT

private slots:
   void autoflowSelectsNewestMatchingRowAndLoadsIt()
   {
      FixtureFiles files;
      QVERIFY( files.valid );
      FakeDB db = databaseFor( files );
      db.rows = {
         { "demo.2401010000.RA.1.A.280.xml", 11, 21,
           QDateTime::fromString( "2024-01-01T00:00:00Z", Qt::ISODate ) },
         { "demo.2401010000.RA.2.B.280.xml", 99, 98,
           QDateTime::fromString( "2026-01-01T00:00:00Z", Qt::ISODate ) },
         { kEditName, 12, 22,
           QDateTime::fromString( "2025-01-01T00:00:00Z", Qt::ISODate ) }
      };

      QVector< US_DataIO::EditedData > edited;
      QVector< US_DataIO::RawData > raw;
      int editID = -1;
      QString error;

      QCOMPARE( US_EditedDataLoaders::loadAutoflow(
                   &db, "1.A.280", "demo", files.destination.path(),
                   edited, raw, editID, error ), (int)IUS_DB2::OK );
      QCOMPARE( db.listQuery,
                QStringList() << "get_editedDataFilenamesIDs" << "demo" );
      QCOMPARE( editID, 12 );
      QCOMPARE( db.blobCalls,
                QStringList() << "download_editData:12"
                              << "download_aucData:22" );
      QCOMPARE( edited.size(), 1 );
      QCOMPARE( raw.size(), 1 );
      QCOMPARE( raw[ 0 ].description, QString( "program loader fixture" ) );
      QVERIFY( error.isEmpty() );
   }

   void autoflowMapsInterferenceTo660()
   {
      FixtureFiles files;
      QVERIFY( files.valid );
      FakeDB db = databaseFor( files );
      db.rows = {
         { kEditName, 17, 27,
           QDateTime::fromString( "2025-01-01T00:00:00Z", Qt::ISODate ) }
      };

      // Use a stored name matching the production Interference substitution.
      db.rows[ 0 ][ 0 ] = "demo.2401010000.RI.1.A.660.xml";
      // The selected stored name must still decode, so serve the fixture under
      // that destination basename and derive its RI raw name from a copy.
      const QString selectedName = db.rows[ 0 ][ 0 ].toString();
      const QString selectedRaw  = "demo.RI.1.A.660.auc";
      db.editFixture = files.source.filePath( kEditName );
      db.rawFixture  = files.source.filePath( kRawName );

      QVector< US_DataIO::EditedData > edited;
      QVector< US_DataIO::RawData > raw;
      int editID = -1;
      QString error;
      QCOMPARE( US_EditedDataLoaders::loadAutoflow(
                   &db, "1.A.Interference", "demo",
                   files.destination.path(), edited, raw, editID, error ),
                (int)IUS_DB2::OK );
      QCOMPARE( editID, 17 );
      QVERIFY( QFileInfo::exists( files.destination.filePath( selectedName ) ) );
      QVERIFY( QFileInfo::exists( files.destination.filePath( selectedRaw ) ) );
   }

   void autoflowQueryFailureDoesNotPublishOldState()
   {
      FixtureFiles files;
      FakeDB db = databaseFor( files );
      db.status = IUS_DB2::DBERROR;
      db.error  = "selection query failed";
      QVector< US_DataIO::EditedData > edited( 1 );
      QVector< US_DataIO::RawData > raw( 1 );
      int editID = 88;
      QString error;

      QCOMPARE( US_EditedDataLoaders::loadAutoflow(
                   &db, "1.A.280", "demo", files.destination.path(),
                   edited, raw, editID, error ), (int)IUS_DB2::DBERROR );
      QVERIFY( edited.isEmpty() );
      QVERIFY( raw.isEmpty() );
      QCOMPARE( editID, -1 );
      QVERIFY( error.contains( "selection query failed" ) );
      QVERIFY( db.blobCalls.isEmpty() );
   }

   void autoflowRejectsRowsForOtherTriples()
   {
      FixtureFiles files;
      FakeDB db = databaseFor( files );
      db.rows = { { kEditName, 11, 21, QDateTime::currentDateTimeUtc() } };
      QVector< US_DataIO::EditedData > edited;
      QVector< US_DataIO::RawData > raw;
      int editID = -1;
      QString error;

      QCOMPARE( US_EditedDataLoaders::loadAutoflow(
                   &db, "2.B.280", "demo", files.destination.path(),
                   edited, raw, editID, error ),
                (int)US_EditedDataLoaders::NO_USABLE_PAIR );
      QVERIFY( db.blobCalls.isEmpty() );
   }

   void reporterLoadsOnlyTheModelLinkedEdit()
   {
      FixtureFiles files;
      QVERIFY( files.valid );
      FakeDB db = databaseFor( files );
      db.rows = {
         { "wrong.xml", 40, 50, "wrong timestamp" },
         { kEditName, 41, 51, "2025-03-04 05:06:07" }
      };
      QVector< US_DataIO::EditedData > edited;
      QVector< US_DataIO::RawData > raw;
      int editID = -1;
      QString updated;
      QString error;

      QCOMPARE( US_EditedDataLoaders::loadReporter(
                   &db, "1.A.280", "demo", 41, files.destination.path(),
                   edited, raw, editID, updated, error ),
                (int)IUS_DB2::OK );
      QCOMPARE( db.listQuery,
                QStringList() << "get_editedDataFilenamesIDs_forReport"
                              << "demo" << "41" );
      QCOMPARE( editID, 41 );
      QCOMPARE( updated, QString( "2025-03-04 05:06:07" ) );
      QCOMPARE( db.blobCalls,
                QStringList() << "download_editData:41"
                              << "download_aucData:51" );
      QCOMPARE( edited.size(), 1 );
      QCOMPARE( raw.size(), 1 );
   }

   void reporterRejectsMissingModelLinkBeforeQuery()
   {
      FixtureFiles files;
      FakeDB db = databaseFor( files );
      QVector< US_DataIO::EditedData > edited;
      QVector< US_DataIO::RawData > raw;
      int editID = -1;
      QString updated;
      QString error;

      QCOMPARE( US_EditedDataLoaders::loadReporter(
                   &db, "1.A.280", "demo", 0, files.destination.path(),
                   edited, raw, editID, updated, error ),
                (int)US_EditedDataLoaders::INVALID_SELECTION );
      QVERIFY( db.listQuery.isEmpty() );
      QVERIFY( db.blobCalls.isEmpty() );
   }

   void reporterRejectsAResultForAnotherEdit()
   {
      FixtureFiles files;
      FakeDB db = databaseFor( files );
      db.rows = { { kEditName, 42, 51, "timestamp" } };
      QVector< US_DataIO::EditedData > edited;
      QVector< US_DataIO::RawData > raw;
      int editID = -1;
      QString updated;
      QString error;

      QCOMPARE( US_EditedDataLoaders::loadReporter(
                   &db, "1.A.280", "demo", 41, files.destination.path(),
                   edited, raw, editID, updated, error ),
                (int)US_EditedDataLoaders::NO_USABLE_PAIR );
      QVERIFY( db.blobCalls.isEmpty() );
   }

   void reporterDownloadFailureDoesNotPublishMetadataOrData()
   {
      FixtureFiles files;
      FakeDB db = databaseFor( files );
      db.rows = { { kEditName, 41, 51, "new timestamp" } };
      db.failProcedure = "download_aucData";
      db.blobStatus    = IUS_DB2::BAD_CHECKSUM;
      db.error         = "raw checksum mismatch";
      QVector< US_DataIO::EditedData > edited( 1 );
      QVector< US_DataIO::RawData > raw( 1 );
      int editID = 99;
      QString updated = "old timestamp";
      QString error;

      QCOMPARE( US_EditedDataLoaders::loadReporter(
                   &db, "1.A.280", "demo", 41, files.destination.path(),
                   edited, raw, editID, updated, error ),
                (int)IUS_DB2::BAD_CHECKSUM );
      QVERIFY( edited.isEmpty() );
      QVERIFY( raw.isEmpty() );
      QCOMPARE( editID, -1 );
      QVERIFY( updated.isEmpty() );
      QVERIFY( error.contains( "raw checksum mismatch" ) );
   }
};

US3_GUI_TEST_MAIN( US_EditedDataLoadersTest )

#include "test_us_edited_data_loaders.moc"
