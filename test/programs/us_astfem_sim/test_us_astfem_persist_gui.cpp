// A multi-speed velocity simulation is persisted as one complete run per speed
// step, in sibling directories suffixed with the rpm.  That contract lives in
// the files the program writes, not in any object it holds, so this drives the
// built simulator and reads what landed on disk.

#include "us3_gui_test_main.h"

#include <QProcess>
#include <QProcessEnvironment>
#include <QXmlStreamReader>

#include "us_dataIO.h"
#include "us_util.h"

namespace
{
const char* const kRunID    = "persist-run";
const char* const kGuidSeed = "us3-persist-regression";
const char* const kEditStamp = "2401010000";
const int         kScansPerStep = 3;

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
      return false;

   return file.write( text.toUtf8() ) == text.toUtf8().size();
}

// Every <speedstep> rotorspeed the experiment XML carries, in file order.
QVector< int > speedStepsOf( const QString& path )
{
   QVector< int > speeds;
   QFile file( path );

   if ( ! file.open( QIODevice::ReadOnly ) )
      return speeds;

   QXmlStreamReader xml( &file );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement()  &&  xml.name().toString() == "speedstep" )
         speeds << xml.attributes().value( "rotorspeed" ).toInt();
   }

   return speeds;
}

QString attributeOf( const QString& path, const QString& element,
                     const QString& attribute )
{
   QFile file( path );

   if ( ! file.open( QIODevice::ReadOnly ) )
      return QString();

   QXmlStreamReader xml( &file );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement()  &&  xml.name().toString() == element )
         return xml.attributes().value( attribute ).toString();
   }

   return QString();
}
}

class US_AstfemSimPersistTest : public QObject
{
   Q_OBJECT

   QTemporaryDir  scratch;
   QString        outRoot;

   // Path of the sibling run directory a speed step is saved into.
   QString speedDir( int rpm ) const
   {
      return outRoot + "/" + QString( kRunID )
           + QString::asprintf( "-%05d", rpm );
   }

   QString speedRunID( int rpm ) const
   {
      return QString( kRunID ) + QString::asprintf( "-%05d", rpm );
   }

private slots:
   void initTestCase()
   {
      QVERIFY( scratch.isValid() );

      const QString inputs = scratch.path() + "/inputs";
      outRoot              = scratch.path() + "/out";

      // The simulator writes its working time state under $HOME/ultrascan, so
      // the child process gets a home of its own rather than the real one.
      const QString home = scratch.path() + "/home";
      QVERIFY( QDir().mkpath( inputs ) );
      QVERIFY( QDir().mkpath( outRoot + "/" + kRunID ) );
      QVERIFY( QDir().mkpath( home + "/ultrascan/tmp" ) );
      QVERIFY( QDir().mkpath( home + "/ultrascan/imports" ) );
      QVERIFY( QDir().mkpath( home + "/ultrascan/results" ) );

      QVERIFY( writeText( inputs + "/simparams.xml", kSimParams ) );

      QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
      env.insert( "HOME", home );

      // The child is a full UltraScan program and reads its own settings.
      // Pointing it at this harness's empty sandbox store would leave it
      // unregistered, and main1.inc answers that with a modal registration
      // dialog that nothing here can dismiss, so the run has to be given the
      // machine's normal settings instead.
      for ( const QString& name : { QStringLiteral( "US3_SETTINGS_ROOT" ),
                                    QStringLiteral( "US3_TEST_SANDBOX" ),
                                    QStringLiteral( "US3_TEST_SETTINGS_ROOT" ),
                                    QStringLiteral( "US3_TEST_WORK_ROOT" ) } )
         env.remove( name );

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
                   "--noise-seed", "4242" } );
      if ( ! sim.waitForFinished( 300000 ) )
      {  // The likeliest cause is an unregistered UltraScan on this machine:
         // the program then waits on a registration dialog before it ever
         // reaches the simulation.
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
      // The requested directory holds no data of its own.  A combined
      // multi-speed run would put its .auc and records here instead.
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

         // The edit names the data it edits; a record written for a run that
         // is not there would name a GUID no saved dataset has.
         QCOMPARE( attributeOf( editPath, "rawDataGUID", "value" ),
                   rawGUID );

         const QString projectGUID = attributeOf( expPath, "project", "guid" );
         QVERIFY2( ! projectGUID.isEmpty(),
                   "experiment XML names no project" );

         rawGUIDs     << rawGUID;
         projectGUIDs << projectGUID;
      }

      // Separate runs, so separate raw identities...
      QCOMPARE( rawGUIDs.size(), 2 );
      QVERIFY( rawGUIDs[ 0 ] != rawGUIDs[ 1 ] );

      // ...but one simulated project behind them.
      QCOMPARE( projectGUIDs[ 0 ], projectGUIDs[ 1 ] );
   }
};

US3_GUI_TEST_MAIN(US_AstfemSimPersistTest)

#include "test_us_astfem_persist_gui.moc"
