// Verify persisted multi-speed runs in their RPM-suffixed directories.

#include "us3_gui_test_main.h"

#include <QProcess>
#include <QProcessEnvironment>
#include <QXmlStreamReader>

#include "us_convert.h"
#include "us_dataIO.h"
#include "us_experiment.h"
#include "us_util.h"

namespace
{
const char* const kRunID    = "persist-run";
const char* const kGuidSeed = "us3-persist-regression";
const char* const kEditStamp = "2401010000";
const int         kScansPerStep = 3;
const char* const kDescription = "ASTFEM persist test";

// Two speed steps of the same length, small enough to simulate quickly.
const char* const kSimParams =
   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
   "<!DOCTYPE US_SimParams>\n"
   "<SimParams version=\"1.0\">\n"
   "    <params meshType=\"ASTFEM\" gridType=\"Moving\" simpoints=\"100\""
   " radialres=\"0.01\" meniscus=\"5.8\" bottom=\"7.2\" rnoise=\"1\""
   " lrnoise=\"0\" tinoise=\"0\" rinoise=\"1\" baseline=\"0.01\""
   " temperature=\"20\" rotorCalID=\"0\" bandform=\"0\" sector=\"0\""
   " pathlength=\"1.2\" angle=\"2.5\" width=\"0\">\n"
   "        <speedstep rotorspeed=\"40000\" scans=\"3\" timefirst=\"0\""
   " timelast=\"0\" w2tfirst=\"0\" w2tlast=\"0\" duration_hrs=\"1\""
   " duration_mins=\"15\" delay_hrs=\"0\" delay_mins=\"1.875\""
   " acceleration=\"400\" accelerflag=\"1\" set_speed=\"40000\""
   " avg_speed=\"40000\" speed_stddev=\"0\"/>\n"
   "        <speedstep rotorspeed=\"50000\" scans=\"3\" timefirst=\"0\""
   " timelast=\"0\" w2tfirst=\"0\" w2tlast=\"0\" duration_hrs=\"1\""
   " duration_mins=\"15\" delay_hrs=\"0\" delay_mins=\"1.875\""
   " acceleration=\"400\" accelerflag=\"1\" set_speed=\"50000\""
   " avg_speed=\"50000\" speed_stddev=\"0\"/>\n"
   "    </params>\n"
   "</SimParams>\n";

bool writeText( const QString& path, const QString& text )
{
   QFile file( path );

   if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return false;
   }

   return file.write( text.toUtf8() ) == text.toUtf8().size();
}

// Every <speedstep> rotorspeed the experiment XML carries, in file order.
QVector< int > speedStepsOf( const QString& path )
{
   QVector< int > speeds;
   QFile file( path );

   if ( ! file.open( QIODevice::ReadOnly ) )
   {
      return speeds;
   }

   QXmlStreamReader xml( &file );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement()  &&  xml.name().toString() == "speedstep" )
      {
         speeds << xml.attributes().value( "rotorspeed" ).toInt();
      }
   }

   return speeds;
}

QString attributeOf( const QString& path, const QString& element,
                     const QString& attribute )
{
   QFile file( path );

   if ( ! file.open( QIODevice::ReadOnly ) )
   {
      return QString();
   }

   QXmlStreamReader xml( &file );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement()  &&  xml.name().toString() == element )
      {
         return xml.attributes().value( attribute ).toString();
      }
   }

   return QString();
}
}

class US_AstfemSimPersistTest : public QObject
{
   Q_OBJECT

   QTemporaryDir        scratch;
   QString              outRoot;
   QString              inputs;
   QProcessEnvironment  env;

   // Path of the sibling run directory a speed step is saved into.
   QString speedDir( int rpm ) const
   {
      return outRoot + "/" + QString( kRunID )
           + QString::asprintf( "-%06d", rpm );
   }

   QString speedRunID( int rpm ) const
   {
      return QString( kRunID ) + QString::asprintf( "-%06d", rpm );
   }

   // Run the simulator with invalid options; return its stderr, or an empty
   // string if it did not exit with exit_code.
   QString rejectedRun( const QStringList& extra, int exit_code = 2 )
   {
      QProcess sim;
      sim.setProcessEnvironment( env );
      sim.start( US_ASTFEM_SIM_EXE,
                 QStringList{ "--no-db", "--errors-cl", "--close",
                              "--model",     inputs + "/model.xml",
                              "--buffer",    inputs + "/buffer.xml",
                              "--simparams", inputs + "/simparams.xml" }
                 + extra );
      if ( ! sim.waitForFinished( 60000 )
           || sim.exitStatus() != QProcess::NormalExit
           || sim.exitCode() != exit_code )
      {
         return QString();
      }
      return QString::fromUtf8( sim.readAllStandardError() );
   }

private slots:
   void initTestCase()
   {
      QVERIFY( scratch.isValid() );

      inputs               = scratch.path() + "/inputs";
      outRoot              = scratch.path() + "/out";

      // Isolate the child process working time state under a temporary HOME.
      const QString home = scratch.path() + "/home";
      QVERIFY( QDir().mkpath( inputs ) );
      QVERIFY( QDir().mkpath( outRoot + "/" + kRunID ) );
      QVERIFY( QDir().mkpath( home + "/ultrascan/tmp" ) );
      QVERIFY( QDir().mkpath( home + "/ultrascan/imports" ) );
      QVERIFY( QDir().mkpath( home + "/ultrascan/results" ) );

      QVERIFY( writeText( inputs + "/simparams.xml", kSimParams ) );

      env = QProcessEnvironment::systemEnvironment();
      env.insert( "HOME", home );

      // Use registered application settings to avoid a modal registration dialog.
      for ( const QString& name : { QStringLiteral( "US3_SETTINGS_ROOT" ),
                                    QStringLiteral( "US3_TEST_SANDBOX" ),
                                    QStringLiteral( "US3_TEST_SETTINGS_ROOT" ),
                                    QStringLiteral( "US3_TEST_WORK_ROOT" ) } )
      {
         env.remove( name );
      }

      QProcess gen;
      gen.setProcessEnvironment( env );
      gen.start( US_SIM_INPUTS_GEN_EXE,
                 { "--emit-model", "--out", inputs + "/model.xml",
                   "-s", "4.58S", "--mw", "66430", "--vbar20", "0.733",
                   "--name", "BSA" } );
      QVERIFY( gen.waitForFinished( 60000 ) );
      QCOMPARE( gen.exitCode(), 0 );

      gen.start( US_SIM_INPUTS_GEN_EXE,
                 { "--emit-buffer", "--out", inputs + "/buffer.xml",
                   "--density", "0.998234", "--viscosity", "1.001940",
                   "--ph", "7.0" } );
      QVERIFY( gen.waitForFinished( 60000 ) );
      QCOMPARE( gen.exitCode(), 0 );

      QProcess sim;
      sim.setProcessEnvironment( env );
      sim.start( US_ASTFEM_SIM_EXE,
                 { "--no-db", "--errors-cl", "--start", "--close",
                   "--model",     inputs + "/model.xml",
                   "--buffer",    inputs + "/buffer.xml",
                   "--simparams", inputs + "/simparams.xml",
                   "--rotor", "1",
                   "--save", outRoot + "/" + kRunID,
                   "--guid-seed", kGuidSeed,
                   "--edit-timestamp", kEditStamp,
                   "--noise-seed", "4242",
                   "--description", kDescription,
                   "--centerpiece", "3" } );
      if ( ! sim.waitForFinished( 300000 ) )
      {  // A missing registration can block the child on a modal dialog.
         sim.kill();
         sim.waitForFinished( 5000 );
         QFAIL( "us_astfem_sim did not finish; check that UltraScan is "
                "registered for this user" );
      }

      QCOMPARE( sim.exitStatus(), QProcess::NormalExit );
      QCOMPARE( sim.exitCode(), 0 );
   }

   void eachSpeedStepIsItsOwnSavedRun()
   {
      // The base directory contains no combined multi-speed dataset.
      const QDir base( outRoot + "/" + kRunID );
      QVERIFY2( base.entryList( { "*.auc" }, QDir::Files ).isEmpty(),
                "a combined multi-speed run was saved" );
      QVERIFY( base.entryList( { "*.xml" }, QDir::Files ).isEmpty() );

      for ( int rpm : { 40000, 50000 } )
      {
         const QDir dir( speedDir( rpm ) );
         QVERIFY2( dir.exists(), qPrintable( dir.path() ) );

         const QString runID = speedRunID( rpm );
         QCOMPARE( dir.entryList( { "*.auc" }, QDir::Files ),
                   QStringList{ runID + ".RA.1.S.123.auc" } );
         QCOMPARE( dir.entryList( { runID + ".RA.xml" }, QDir::Files ).size(),
                   1 );
         QCOMPARE( dir.entryList( { "*." + QString( kEditStamp ) + ".*.xml" },
                                  QDir::Files ),
                   QStringList{ runID + "." + kEditStamp
                                + ".RA.1.S.123.xml" } );

         // Each step's noise record sits beside the data it describes.
         QVERIFY( QFile::exists( dir.filePath( "ASTFEM_RI_NOISE.csv" ) ) );
      }
   }

   void eachSavedRunCarriesOnlyItsOwnSpeedStep()
   {
      for ( int rpm : { 40000, 50000 } )
      {
         const QString runID = speedRunID( rpm );
         const QVector< int > speeds =
            speedStepsOf( speedDir( rpm ) + "/" + runID + ".RA.xml" );

         QCOMPARE( speeds, QVector< int >{ rpm } );
      }
   }

   void savedScansBelongToTheirOwnSpeed()
   {
      double lastTime = 0.0;

      for ( int rpm : { 40000, 50000 } )
      {
         const QString runID = speedRunID( rpm );
         US_DataIO::RawData data;
         QCOMPARE( US_DataIO::readRawData(
                      speedDir( rpm ) + "/" + runID + ".RA.1.S.123.auc",
                      data ),
                   (int)US_DataIO::OK );

         QCOMPARE( data.scanCount(), kScansPerStep );
         QCOMPARE( (int)data.cell, 1 );
         QCOMPARE( data.description, QString( kDescription ) );
         QCOMPARE( (char)data.channel, 'S' );

         for ( const US_DataIO::Scan& scan : data.scanData )
         {
            QCOMPARE( qRound( scan.rpm ), rpm );

            // The steps are consecutive stretches of one run, so their scan
            // times do not overlap or restart.
            QVERIFY( scan.seconds > lastTime );
            lastTime = scan.seconds;
         }
      }
   }

   void riNoiseRecordCoversEachScanOfItsStep()
   {
      for ( int rpm : { 40000, 50000 } )
      {
         const QString runID = speedRunID( rpm );
         US_DataIO::RawData data;
         QCOMPARE( US_DataIO::readRawData(
                      speedDir( rpm ) + "/" + runID + ".RA.1.S.123.auc",
                      data ),
                   (int)US_DataIO::OK );

         QFile csv( speedDir( rpm ) + "/ASTFEM_RI_NOISE.csv" );
         QVERIFY( csv.open( QIODevice::ReadOnly | QIODevice::Text ) );
         QStringList rows = QString::fromUtf8( csv.readAll() )
                               .split( "\n", Qt::SkipEmptyParts );
         QVERIFY( ! rows.isEmpty() );
         rows.removeFirst();   // header

         // One noise row per scan of this step, at that scan's time.
         QCOMPARE( rows.size(), kScansPerStep );
         for ( int scan = 0; scan < rows.size(); scan++ )
         {
            QCOMPARE( qRound( rows[ scan ].section( ",", 0, 0 ).toDouble() ),
                      qRound( data.scanData[ scan ].seconds ) );
         }
      }
   }

   void cellBeyondTheRotorHoleCountIsRejected()
   {
      // Rotor 2 is an AN60, which has 4 holes.
      QVERIFY( rejectedRun( { "--rotor", "2", "--cell", "5" } )
                  .contains( "has only 4 holes" ) );
   }

   void channelMissingFromTheCenterpieceIsRejected()
   {
      // Centerpiece 1 is a 2-channel centerpiece: only row 0 (A/B) exists.
      QVERIFY( rejectedRun( { "--rotor", "1", "--centerpiece", "1",
                              "--channel", "C" } )
                  .contains( "out of range" ) );
   }

   void channelAndCenterpieceChannelMustAgree()
   {
      QVERIFY( rejectedRun( { "--rotor", "1", "--centerpiece", "3",
                              "--channel", "C", "--centerpiece-channel", "A" } )
                  .contains( "--channel C is centerpiece row 1" ) );
   }

   void experimentRecordNamesTheSelectedCenterpiece()
   {
      // Centerpiece index 3 is the Epon 6-channel rectangular, ID 4.
      for ( int rpm : { 40000, 50000 } )
      {
         const QString expPath = speedDir( rpm ) + "/" + speedRunID( rpm )
                               + ".RA.xml";
         QCOMPARE( attributeOf( expPath, "centerpiece", "id" ), QString( "4" ) );
      }
   }

   void invalidEditTimestampsAreRejected()
   {
      // Not ten digits, and ten digits that are not a real date.
      for ( const QString& stamp : { QString( "abc" ), QString( "2413320000" ) } )
      {
         QVERIFY2( rejectedRun( { "--edit-timestamp", stamp }, 1 )
                      .contains( "Invalid --edit-timestamp" ), qPrintable( stamp ) );
      }
   }

   void intensityRunTypesAreRejected()
   {
      // Simulated concentration cannot be tagged as intensity data.
      for ( const QString& type : { QString( "RI" ), QString( "WI" ) } )
      {
         QVERIFY2( rejectedRun( { "--runtype", type }, 1 )
                      .contains( "Invalid --runtype" ), qPrintable( type ) );
      }
   }

   void descriptionTooLongForTheAucFormatIsRejected()
   {
      QVERIFY( rejectedRun( { "--description", QString( 240, 'x' ) }, 1 )
                  .contains( "Invalid --description" ) );
   }

   void speedRunsShareAProjectButNotARawIdentity()
   {
      QStringList rawGUIDs;
      QStringList projectGUIDs;

      for ( int rpm : { 40000, 50000 } )
      {
         const QString runID   = speedRunID( rpm );
         const QString expPath = speedDir( rpm ) + "/" + runID + ".RA.xml";
         const QString editPath = speedDir( rpm ) + "/" + runID + "."
                                + kEditStamp + ".RA.1.S.123.xml";

         US_DataIO::RawData data;
         QCOMPARE( US_DataIO::readRawData(
                      speedDir( rpm ) + "/" + runID + ".RA.1.S.123.auc",
                      data ),
                   (int)US_DataIO::OK );

         const QString rawGUID = US_Util::uuid_unparse(
                                    (unsigned char*)data.rawGUID );

         // Each edit references its saved dataset GUID.
         QCOMPARE( attributeOf( editPath, "rawDataGUID", "value" ),
                   rawGUID );

         const QString projectGUID = attributeOf( expPath, "project", "guid" );
         QVERIFY2( ! projectGUID.isEmpty(),
                   "experiment XML names no project" );

         rawGUIDs     << rawGUID;
         projectGUIDs << projectGUID;
      }

      // Speed-specific runs have distinct raw GUIDs.
      QCOMPARE( rawGUIDs.size(), 2 );
      QVERIFY( rawGUIDs[ 0 ] != rawGUIDs[ 1 ] );

      // All speed-specific runs share one project GUID.
      QCOMPARE( projectGUIDs[ 0 ], projectGUIDs[ 1 ] );
   }

   void aSavedSpeedRunLoadsThroughTheDiskReaderChain()
   {
      // The one archive-shape integration case: a real simulator run driven
      // through the same reader sequence US_ConvertGui::loadUS3Disk() uses, so
      // the unit characterizations elsewhere are anchored to output the
      // production writers actually produce.
      for ( int rpm : { 40000, 50000 } )
      {
         const QString runID = speedRunID( rpm );
         const QString dir   = speedDir( rpm ) + "/";

         QVector< US_DataIO::RawData >   allData;
         QList< US_Convert::TripleInfo > triples;
         QString                         runType;

         QCOMPARE( US_Convert::readUS3Disk( dir, allData, triples, runType ),
                   (int)US_Convert::OK );
         QCOMPARE( runType, QString( "RA" ) );
         QCOMPARE( allData.size(), 1 );
         QCOMPARE( triples.size(), 1 );
         QCOMPARE( triples[ 0 ].tripleDesc, QString( "1 / S / 123" ) );

         US_Experiment experiment;
         experiment.clear();
         QCOMPARE( experiment.readFromDisk( triples, runType, runID, dir ),
                   (int)US_Convert::OK );

         QCOMPARE( experiment.runID, runID );

         // The experiment filled in what the filename scan left empty.
         QCOMPARE( triples[ 0 ].tripleFilename,
                   runID + ".RA.1.S.123.auc" );
         QCOMPARE( US_Util::uuid_unparse(
                      (unsigned char*)triples[ 0 ].tripleGUID ),
                   US_Util::uuid_unparse(
                      (unsigned char*)allData[ 0 ].rawGUID ) );

         // A generated run names both records it depends on.
         QVERIFY( ! experiment.project.projectGUID.isEmpty() );
         QVERIFY( ! triples[ 0 ].solution.solutionGUID.isEmpty() );
      }
   }
};

US3_GUI_TEST_MAIN(US_AstfemSimPersistTest)

#include "test_us_astfem_persist_gui.moc"
