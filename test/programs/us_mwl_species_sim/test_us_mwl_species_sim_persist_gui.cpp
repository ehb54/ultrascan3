// us_mwl_species_sim writes one dataset per wavelength.  Each one gets a plain
// edit file naming its own wavelength -- not a band, and not the "@" lookup key
// -- and each edit names the raw data beside it.  That is a contract about the
// files the program writes, so this drives the built simulator and reads them.

#include "us3_gui_test_main.h"

#include <QProcess>
#include <QProcessEnvironment>
#include <QXmlStreamReader>

#include "us_dataIO.h"
#include "us_util.h"

namespace
{
const char* const kRunID     = "mwl-run";
const char* const kChannel   = "1A";
const char* const kGuidSeed  = "us3-mwl-persist-regression";
const char* const kEditStamp = "2401010000";
const QStringList kWavelengths { "280", "410" };

const char* const kSimParams =
   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
   "<!DOCTYPE US_SimParams>\n"
   "<SimParams version=\"1.0\">\n"
   "    <params meshType=\"ASTFEM\" gridType=\"Moving\" simpoints=\"100\""
   " radialres=\"0.01\" meniscus=\"5.8\" bottom=\"7.2\" rnoise=\"0\""
   " lrnoise=\"0\" tinoise=\"0\" rinoise=\"0\" baseline=\"0\""
   " temperature=\"20\" rotorCalID=\"0\" bandform=\"0\" sector=\"0\""
   " pathlength=\"1.2\" angle=\"2.5\" width=\"0\">\n"
   "        <speedstep rotorspeed=\"40000\" scans=\"3\" timefirst=\"0\""
   " timelast=\"0\" w2tfirst=\"0\" w2tlast=\"0\" duration_hrs=\"1\""
   " duration_mins=\"15\" delay_hrs=\"0\" delay_mins=\"1.875\""
   " acceleration=\"400\" accelerflag=\"1\" set_speed=\"40000\""
   " avg_speed=\"40000\" speed_stddev=\"0\"/>\n"
   "    </params>\n"
   "</SimParams>\n";

bool writeText( const QString& path, const QString& text )
{
   QFile file( path );

   if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text ) )
      return false;

   return file.write( text.toUtf8() ) == text.toUtf8().size();
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

class US_MwlSpeciesSimPersistTest : public QObject
{
   Q_OBJECT

   QTemporaryDir scratch;
   QString       outDir;
   QString       savedRunID;   // the program derives it from the models

private slots:
   void initTestCase()
   {
      QVERIFY( scratch.isValid() );

      const QString inputs = scratch.path() + "/inputs";
      const QString home   = scratch.path() + "/home";
      outDir               = scratch.path() + "/out/" + kRunID;

      QVERIFY( QDir().mkpath( inputs ) );
      QVERIFY( QDir().mkpath( outDir ) );
      QVERIFY( QDir().mkpath( home + "/ultrascan/tmp" ) );
      QVERIFY( QDir().mkpath( home + "/ultrascan/imports" ) );
      QVERIFY( QDir().mkpath( home + "/ultrascan/results" ) );

      QVERIFY( writeText( inputs + "/simparams.xml", kSimParams ) );

      QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
      env.insert( "HOME", home );

      // See the astfem persistence test: the child is a full UltraScan program
      // and an empty sandbox settings store leaves it unregistered, waiting on
      // a modal dialog nothing here can dismiss.
      for ( const QString& name : { QStringLiteral( "US3_SETTINGS_ROOT" ),
                                    QStringLiteral( "US3_TEST_SANDBOX" ),
                                    QStringLiteral( "US3_TEST_SETTINGS_ROOT" ),
                                    QStringLiteral( "US3_TEST_WORK_ROOT" ) } )
         env.remove( name );

      QStringList models;
      QProcess gen;
      gen.setProcessEnvironment( env );

      for ( const QString& wavelength : kWavelengths )
      {
         gen.start( US_SIM_INPUTS_GEN_EXE,
                    { "--out", inputs, "--run-id", kRunID,
                      "--channel", kChannel, "--wavelength", wavelength } );
         QVERIFY( gen.waitForFinished( 60000 ) );
         QCOMPARE( gen.exitCode(), 0 );

         models << inputs + "/model_" + kChannel + "_" + wavelength + ".xml";
         QVERIFY2( QFile::exists( models.last() ),
                   qPrintable( models.last() ) );
      }

      gen.start( US_SIM_INPUTS_GEN_EXE,
                 { "--emit-buffer", "--out", inputs + "/buffer.xml",
                   "--density", "0.998234", "--viscosity", "1.001940",
                   "--ph", "7.0" } );
      QVERIFY( gen.waitForFinished( 60000 ) );
      QCOMPARE( gen.exitCode(), 0 );

      QProcess sim;
      sim.setProcessEnvironment( env );
      sim.start( US_MWL_SPECIES_SIM_EXE,
                 { "--no-db", "--errors-cl", "--start", "--close",
                   "--models",    models.join( "," ),
                   "--buffer",    inputs + "/buffer.xml",
                   "--simparams", inputs + "/simparams.xml",
                   "--rotor", "1", "--runtype", "RA",
                   "--save", outDir,
                   "--guid-seed", kGuidSeed,
                   "--edit-timestamp", kEditStamp } );

      if ( ! sim.waitForFinished( 300000 ) )
      {
         sim.kill();
         sim.waitForFinished( 5000 );
         QFAIL( "us_mwl_species_sim did not finish; check that UltraScan is "
                "registered for this user" );
      }

      QCOMPARE( sim.exitStatus(), QProcess::NormalExit );
      QCOMPARE( sim.exitCode(), 0 );

      // The saved run ID comes from the models, not from the directory name.
      const QStringList aucs =
         QDir( outDir ).entryList( { "*.auc" }, QDir::Files );
      QCOMPARE( aucs.size(), kWavelengths.size() );
      savedRunID = aucs.first().section( ".", 0, 0 );
      QVERIFY( ! savedRunID.isEmpty() );
   }

   void oneEditAccompaniesEachWavelengthsData()
   {
      const QDir dir( outDir );
      const QStringList edits =
         dir.entryList( { "*." + QString( kEditStamp ) + ".*.xml" },
                        QDir::Files );

      QCOMPARE( edits.size(), kWavelengths.size() );

      for ( const QString& wavelength : kWavelengths )
      {
         const QString triple = "RA.1.A." + wavelength;

         QVERIFY2( edits.contains( savedRunID + "." + kEditStamp + "."
                                   + triple + ".xml" ),
                   qPrintable( edits.join( ", " ) ) );
         QVERIFY( QFile::exists(
                     dir.filePath( savedRunID + "." + triple + ".auc" ) ) );
      }
   }

   void theWavelengthFieldIsOnePlainWavelength()
   {
      // A hyphen or an "@" in this field is a band lookup key.  Those are
      // in-memory forms; neither belongs in a filename.
      for ( const QString& name :
               QDir( outDir ).entryList( { "*." + QString( kEditStamp )
                                           + ".*.xml" }, QDir::Files ) )
      {
         const QString field = name.section( ".", -2, -2 );

         QVERIFY2( ! field.contains( '-' ), qPrintable( name ) );
         QVERIFY2( ! field.contains( '@' ), qPrintable( name ) );
         QVERIFY2( kWavelengths.contains( field ), qPrintable( name ) );
      }
   }

   void eachEditNamesTheDataBesideIt()
   {
      QStringList rawGUIDs;

      for ( const QString& wavelength : kWavelengths )
      {
         const QString triple   = "RA.1.A." + wavelength;
         const QString aucPath  = outDir + "/" + savedRunID + "." + triple
                                + ".auc";
         const QString editPath = outDir + "/" + savedRunID + "."
                                + kEditStamp + "." + triple + ".xml";

         US_DataIO::RawData data;
         QCOMPARE( US_DataIO::readRawData( aucPath, data ),
                   (int)US_DataIO::OK );

         // The identity the model gave this dataset, not a fixed 1/S default.
         QCOMPARE( (int)data.cell, 1 );
         QCOMPARE( (char)data.channel, 'A' );

         const QString rawGUID = US_Util::uuid_unparse(
                                    (unsigned char*)data.rawGUID );

         QCOMPARE( attributeOf( editPath, "rawDataGUID", "value" ), rawGUID );
         QCOMPARE( attributeOf( editPath, "runid", "value" ), savedRunID );

         rawGUIDs << rawGUID;
      }

      // Separate datasets of one run, so separate raw identities.
      QCOMPARE( rawGUIDs.size(), kWavelengths.size() );
      QVERIFY( rawGUIDs[ 0 ] != rawGUIDs[ 1 ] );
   }

   void theRunHasOneExperimentRecordNamingAProject()
   {
      const QString expPath = outDir + "/" + savedRunID + ".RA.xml";

      QCOMPARE( QDir( outDir ).entryList( { "*.RA.xml" }, QDir::Files ),
                QStringList{ savedRunID + ".RA.xml" } );
      QVERIFY2( ! attributeOf( expPath, "project", "guid" ).isEmpty(),
                "experiment XML names no project" );
   }
};

US3_GUI_TEST_MAIN(US_MwlSpeciesSimPersistTest)

#include "test_us_mwl_species_sim_persist_gui.moc"
