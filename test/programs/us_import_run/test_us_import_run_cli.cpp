//! \file test_us_import_run_cli.cpp
// us_import_run's argument checks, its staging, and the run checks it makes
// before connecting.  Every case either is refused before the database or
// uses --dry-run, so none needs a server.

#include "us3_gui_test_main.h"

#include <QFile>
#include <QProcess>

#include "disk_run_fixture.h"
#include "us_dataIO.h"

class US_ImportRunCliTest : public QObject
{
      Q_OBJECT

      QTemporaryDir scratch;

      // Run the importer; return its exit code and stderr, and its stdout in
      // out if given.
      int           run( const QStringList& args, QString& err, QString* out = nullptr )
      {
         QProcess importer;
         importer.start( US_IMPORT_RUN_EXE, args );
         if ( !importer.waitForFinished( 30000 ) )
         {
            importer.kill();
            return -1;
         }
         err = QString::fromUtf8( importer.readAllStandardError() );
         if ( out )
         {
            *out = QString::fromUtf8( importer.readAllStandardOutput() );
         }
         return importer.exitCode();
      }

      // An existing directory holding no run, for checks that stop before
      // the run is read or that only need reading to fail.
      QString emptyRunDir( const QString& name ) const
      {
         const QString dir = scratch.filePath( name );
         QDir().mkpath( dir );
         return dir;
      }

      QString workParent() const
      {
         return scratch.filePath( "work" );
      }

   private slots:
      void initTestCase()
      {
         QVERIFY( scratch.isValid() );
      }

      void stagingLeavesExistingRecordsInTheWorkDirectory()
      {
         // A US3 work tree with records of its own, given as --work-dir.
         const QString work = scratch.filePath( "us3-home" );
         QVERIFY( QDir().mkpath( work + "/data/solutions" ) );
         QVERIFY( QDir().mkpath( work + "/data/buffers" ) );
         const QStringList records{ work + "/data/solutions/S0000001.xml", work + "/data/buffers/B0000007.xml" };
         for ( const QString& path : records )
         {
            QFile file( path );
            QVERIFY( file.open( QIODevice::WriteOnly ) );
            file.write( "<record/>" );
         }

         QString err;
         QCOMPARE( run( { "--dir", emptyRunDir( "no-run" ), "--dry-run", "--work-dir", work }, err ), 2 );

         for ( const QString& path : records )
         {
            QVERIFY2( QFile::exists( path ), qPrintable( path ) );
         }

         // The staging directory is created inside --work-dir and removed.
         QCOMPARE( QDir( work ).entryList( { "us_import_run-*" }, QDir::Dirs ), QStringList() );
      }

      void personIdsMustBePositiveIntegers_data()
      {
         QTest::addColumn<QString>( "option" );
         QTest::addColumn<QString>( "value" );

         QTest::newRow( "investigator, text" ) << "--investigator-id" << "abc";
         QTest::newRow( "investigator, zero" ) << "--investigator-id" << "0";
         QTest::newRow( "operator, text" ) << "--operator-id" << "x7";
         QTest::newRow( "operator, negative" ) << "--operator-id" << "-3";
      }

      void personIdsMustBePositiveIntegers()
      {
         QFETCH( QString, option );
         QFETCH( QString, value );

         QString err;
         QCOMPARE( run( { "--dir", emptyRunDir( "ids" ), "--dry-run", "--work-dir", workParent(), option, value }, err ), 1 );
         QVERIFY2( err.contains( option + " must be a positive integer" ), qPrintable( err ) );
      }

      void aDryRunNeedsNoDatabase()
      {
         // Past the argument checks, so it fails on the empty run instead.
         QString err;
         QCOMPARE( run( { "--dir", emptyRunDir( "dry" ), "--dry-run", "--work-dir", workParent() }, err ), 2 );
         QVERIFY2( !err.contains( "--database" ), qPrintable( err ) );
      }

      void aLoadNeedsAnAllowedDatabase()
      {
         QString err;
         QCOMPARE( run( { "--dir", emptyRunDir( "load" ), "--work-dir", workParent() }, err ), 1 );
         QVERIFY2( err.contains( "--database is required" ), qPrintable( err ) );

         QCOMPARE( run( { "--dir", emptyRunDir( "load" ), "--work-dir", workParent(), "--database", "uslims3_prod" }, err ), 1 );
         QVERIFY2( err.contains( "Refusing to write to \"uslims3_prod\"" ), qPrintable( err ) );
      }

      void anotherRunsExperimentRecordIsRefused()
      {
         const us3test::DiskRun fixture =
            us3test::writeDiskRun( scratch.path(), "run-a", "RA", { us3test::DatasetSpec{ 1, 'A', "280" } } );
         QVERIFY( !fixture.dir.isEmpty() );
         QVERIFY( QFile::copy( fixture.dir + "run-a.RA.xml", fixture.dir + "run-b.RA.xml" ) );

         QString err;
         QCOMPARE( run( { "--dir", fixture.dir, "--dry-run", "--work-dir", workParent() }, err ), 2 );
         QVERIFY2( err.contains( "alongside experiment records for run-b" ), qPrintable( err ) );
      }

      void aRunWithoutItsProjectRecordIsRefused()
      {
         const us3test::DiskRun fixture =
            us3test::writeDiskRun( scratch.path(), "run-c", "RA", { us3test::DatasetSpec{ 1, 'A', "280" } } );
         QVERIFY( !fixture.dir.isEmpty() );

         QString err;
         QCOMPARE( run( { "--dir", fixture.dir, "--dry-run", "--work-dir", workParent() }, err ), 2 );
         QVERIFY2( err.contains( "No project record for GUID " + fixture.projectGUID ), qPrintable( err ) );
      }

      // The edit name carries the .auc filename's wavelength field verbatim,
      // as US_Edit writes it: a wavelength for RA, a radius times 1000 for WA.
      void aCompleteRunPassesTheDryRun_data()
      {
         QTest::addColumn<QString>( "runType" );
         QTest::addColumn<QString>( "wavelength" );
         QTest::newRow( "RA" ) << "RA" << "280";
         QTest::newRow( "WA" ) << "WA" << "6500";
      }

      void aCompleteRunPassesTheDryRun()
      {
         QFETCH( QString, runType );
         QFETCH( QString, wavelength );

         const us3test::DiskRun fixture =
            us3test::writeDiskRun( scratch.path(), "run-ok-" + runType, runType, { us3test::DatasetSpec{ 1, 'A', wavelength } } );
         QVERIFY( !fixture.dir.isEmpty() );
         QVERIFY( us3test::writeRunRecords( fixture ) );

         QString err;
         QString out;
         QCOMPARE( run( { "--dir", fixture.dir, "--dry-run", "--work-dir", workParent() }, err, &out ), 0 );
         QVERIFY2( out.contains( "1 triples, 1 edits" ), qPrintable( out + err ) );
      }

      void anEditForAnotherWavelengthIsRefused()
      {
         const us3test::DiskRun fixture =
            us3test::writeDiskRun( scratch.path(), "run-wa-other", "WA", { us3test::DatasetSpec{ 1, 'A', "6500" } } );
         QVERIFY( !fixture.dir.isEmpty() );
         QVERIFY( us3test::writeRunRecords( fixture ) );

         // Move the edit, name and XML together, to a radius the .auc is not.
         const QString         original = fixture.dir + "run-wa-other.2601010000.WA.1.A.6500.xml";
         US_DataIO::EditValues edit;
         QCOMPARE( US_DataIO::readEdits( original, edit ), int( US_DataIO::OK ) );
         QVERIFY( QFile::remove( original ) );
         edit.wavelength = "6000";
         QCOMPARE( US_DataIO::writeEdits( fixture.dir + "run-wa-other.2601010000.WA.1.A.6000.xml", edit ), int( US_DataIO::OK ) );

         QString err;
         QCOMPARE( run( { "--dir", fixture.dir, "--dry-run", "--work-dir", workParent() }, err ), 2 );
         QVERIFY2( err.contains( "is for wavelength 6000, but raw data" ), qPrintable( err ) );
      }
};

US3_GUI_TEST_MAIN( US_ImportRunCliTest )

#include "test_us_import_run_cli.moc"
