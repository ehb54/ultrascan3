//! \file test_us_astfem_reproducibility_gui.cpp
// Verify that two headless runs with identical --guid-seed, --noise-seed,
// and --edit-timestamp produce byte-identical .auc data and matching
// identities in the edit XML, that generating inputs twice with one
// --guid-seed produces identical files, and that runs without --noise-seed
// get fresh, reported, reproducible seeds.

#include "us3_gui_test_main.h"

#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QXmlStreamReader>

#include "us3_dev_license.h"
#include "us_dataIO.h"
#include "us_util.h"

namespace
{
const char* const kRunID       = "repro-run";
const char* const kGuidSeed    = "us3-reproducibility-regression";
const char* const kEditStamp   = "2401010000";
const char* const kDescription = "ASTFEM reproducibility test";

const char* const kSimParams = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
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
                               "    </params>\n"
                               "</SimParams>\n";

bool              writeText( const QString& path, const QString& text )
{
   QFile file( path );

   if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return false;
   }

   return file.write( text.toUtf8() ) == text.toUtf8().size();
}

QString attributeOf( const QString& path, const QString& element, const QString& attribute )
{
   QFile file( path );

   if ( !file.open( QIODevice::ReadOnly ) )
   {
      return QString();
   }

   QXmlStreamReader xml( &file );

   while ( !xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() && xml.name().toString() == element )
      {
         return xml.attributes().value( attribute ).toString();
      }
   }

   return QString();
}
}

class US_AstfemSimReproducibilityTest : public QObject
{
      Q_OBJECT

      QTemporaryDir       scratch;
      QString             inputsA;
      QString             inputsB;
      QString             outA;
      QString             outB;
      QProcessEnvironment env;

      // The generated files compared between the two runs.
      const QStringList   generatedFiles{ "model.xml",
                                        "buffer.xml",
                                        "default/model_default.xml",
                                        "default/buffer_default.xml",
                                        "default/sp_default.xml" };

      // Generate one run's inputs, each mode with the same --guid-seed.
      void                generate( const QString& inputsDir )
      {
         QVERIFY( QDir().mkpath( inputsDir + "/default" ) );
         QVERIFY( writeText( inputsDir + "/simparams.xml", kSimParams ) );

         const QList<QStringList> invocations{ { "--emit-model",
                                                 "--out",
                                                 inputsDir + "/model.xml",
                                                 "-s",
                                                 "4.58S",
                                                 "--mw",
                                                 "66430",
                                                 "--vbar20",
                                                 "0.733",
                                                 "--name",
                                                 "BSA" },
                                               { "--emit-buffer",
                                                 "--out",
                                                 inputsDir + "/buffer.xml",
                                                 "--density",
                                                 "0.998234",
                                                 "--viscosity",
                                                 "1.001940",
                                                 "--ph",
                                                 "7.0" },
                                               { "--out", inputsDir + "/default" } };

         for ( const QStringList& args : invocations )
         {
            QProcess gen;
            gen.setProcessEnvironment( env );
            gen.start( US_SIM_INPUTS_GEN_EXE, args + QStringList{ "--guid-seed", kGuidSeed } );
            QVERIFY( gen.waitForFinished( 60000 ) );
            QCOMPARE( gen.exitCode(), 0 );
         }
      }

      // Simulate from the given inputs into outDir; return the program's stdout.
      QString simulate( const QString& inputsDir, const QString& outDir, const QStringList& extra )
      {
         if ( !QDir().mkpath( outDir ) )
         {
            QTest::qFail( qPrintable( "could not create " + outDir ), __FILE__, __LINE__ );
            return QString();
         }

         QProcess sim;
         sim.setProcessEnvironment( env );
         sim.start( US_ASTFEM_SIM_EXE,
                    QStringList{ "--no-db",
                                 "--errors-cl",
                                 "--close",
                                 "--model",
                                 inputsDir + "/model.xml",
                                 "--buffer",
                                 inputsDir + "/buffer.xml",
                                 "--simparams",
                                 inputsDir + "/simparams.xml",
                                 "--rotor",
                                 "1",
                                 "--save",
                                 outDir,
                                 "--guid-seed",
                                 kGuidSeed,
                                 "--edit-timestamp",
                                 kEditStamp,
                                 "--description",
                                 kDescription,
                                 "--centerpiece",
                                 "3" } +
                       extra );
         if ( !sim.waitForFinished( 300000 ) )
         {
            sim.kill();
            sim.waitForFinished( 5000 );
            QTest::qFail( "us_astfem_sim did not finish; check that UltraScan is registered for this user", __FILE__, __LINE__ );
            return QString();
         }

         const QString err = QString::fromUtf8( sim.readAllStandardError() );
         if ( sim.exitStatus() != QProcess::NormalExit || sim.exitCode() != 0 )
         {
            QTest::qFail(
               qPrintable( QString( "us_astfem_sim exited %1: %2" ).arg( sim.exitCode() ).arg( err ) ), __FILE__, __LINE__ );
            return QString();
         }

         return QString::fromUtf8( sim.readAllStandardOutput() );
      }

      // The run's dataset, or an empty array if it was not written.
      static QByteArray aucOf( const QString& outDir )
      {
         QFile file( outDir + "/" + QString( kRunID ) + ".RA.1.S.123.auc" );
         return file.open( QIODevice::ReadOnly ) ? file.readAll() : QByteArray();
      }

      // The seed a run reports choosing, or 0 if it reported none.
      static uint reportedSeed( const QString& out )
      {
         const QRegularExpressionMatch match = QRegularExpression( "Noise seed: (\\d+)" ).match( out );
         return match.hasMatch() ? match.captured( 1 ).toUInt() : 0;
      }

   private slots:
      void initTestCase()
      {
         QVERIFY( scratch.isValid() );

         inputsA = scratch.path() + "/inputs_a";
         inputsB = scratch.path() + "/inputs_b";
         outA    = scratch.path() + "/out_a/" + kRunID;
         outB    = scratch.path() + "/out_b/" + kRunID;

         const QString home = scratch.path() + "/home";
         QVERIFY( QDir().mkpath( home + "/ultrascan/tmp" ) );
         QVERIFY( QDir().mkpath( home + "/ultrascan/imports" ) );
         QVERIFY( QDir().mkpath( home + "/ultrascan/results" ) );
         registerDevLicenseFor( home );

         env = QProcessEnvironment::systemEnvironment();
         env.insert( "HOME", home );

         for ( const QString& name : { QStringLiteral( "US3_SETTINGS_ROOT" ),
                                       QStringLiteral( "US3_TEST_SANDBOX" ),
                                       QStringLiteral( "US3_TEST_SETTINGS_ROOT" ),
                                       QStringLiteral( "US3_TEST_WORK_ROOT" ) } )
         {
            env.remove( name );
         }

         // A QFAIL inside a helper only unwinds that call, not initTestCase();
         // without these checks, a stuck/missing registration doubles the wait
         // (two sequential 300s kills) and trips CTest's 600s hard timeout
         // instead of failing cleanly.
         generate( inputsA );
         if ( QTest::currentTestFailed() )
         {
            return;
         }
         generate( inputsB );
         if ( QTest::currentTestFailed() )
         {
            return;
         }
         simulate( inputsA, outA, { "--start", "--noise-seed", "4242" } );
         if ( QTest::currentTestFailed() )
         {
            return;
         }
         simulate( inputsB, outB, { "--start", "--noise-seed", "4242" } );
      }

      void generatedInputsAreReproducible()
      {
         // Each run generated its own inputs, so equal files mean every
         // generator mode honours --guid-seed.
         for ( const QString& name : generatedFiles )
         {
            QFile fileA( inputsA + "/" + name );
            QFile fileB( inputsB + "/" + name );
            QVERIFY2( fileA.open( QIODevice::ReadOnly ), qPrintable( fileA.fileName() ) );
            QVERIFY2( fileB.open( QIODevice::ReadOnly ), qPrintable( fileB.fileName() ) );
            QVERIFY2( fileA.readAll() == fileB.readAll(), qPrintable( name + " differs between the two generator runs" ) );
         }

         QVERIFY2( !attributeOf( inputsA + "/model.xml", "model", "modelGUID" ).isEmpty(), "generated model carries no modelGUID" );
      }

      void unseededRunsGetFreshNoiseAndReportTheirSeed()
      {
         // These runs omit --start: --save implies it.
         const QString outC = scratch.path() + "/out_c/" + kRunID;
         const QString outD = scratch.path() + "/out_d/" + kRunID;
         const QString outE = scratch.path() + "/out_e/" + kRunID;

         const uint    seedC = reportedSeed( simulate( inputsA, outC, {} ) );
         QVERIFY( !QTest::currentTestFailed() );
         const uint seedD = reportedSeed( simulate( inputsA, outD, {} ) );
         QVERIFY( !QTest::currentTestFailed() );

         QVERIFY2( seedC != 0, "an unseeded run did not report its noise seed" );
         QVERIFY2( seedD != 0, "an unseeded run did not report its noise seed" );
         QVERIFY( seedC != seedD );

         const QByteArray aucC = aucOf( outC );
         QVERIFY2( !aucC.isEmpty(), "--save without --start wrote no data" );
         QVERIFY2( aucC != aucOf( outD ), "two unseeded runs produced the same noise" );

         // The reported seed reproduces the run.
         const QString outE_stdout = simulate( inputsA, outE, { "--noise-seed", QString::number( seedC ) } );
         QVERIFY( !QTest::currentTestFailed() );
         QCOMPARE( reportedSeed( outE_stdout ), 0u );
         QVERIFY2( aucOf( outE ) == aucC, "rerunning with the reported seed did not reproduce the run" );
      }

      void sameSeedsProduceByteIdenticalAucData()
      {
         const QString aucA = outA + "/" + QString( kRunID ) + ".RA.1.S.123.auc";
         const QString aucB = outB + "/" + QString( kRunID ) + ".RA.1.S.123.auc";

         QVERIFY2( QFile::exists( aucA ), qPrintable( aucA ) );
         QVERIFY2( QFile::exists( aucB ), qPrintable( aucB ) );

         QFile fileA( aucA );
         QFile fileB( aucB );
         QVERIFY( fileA.open( QIODevice::ReadOnly ) );
         QVERIFY( fileB.open( QIODevice::ReadOnly ) );

         QCOMPARE( fileA.readAll(), fileB.readAll() );
      }

      void sameSeedsProduceMatchingEditIdentities()
      {
         const QString editA = outA + "/" + QString( kRunID ) + "." + QString( kEditStamp ) + ".RA.1.S.123.xml";
         const QString editB = outB + "/" + QString( kRunID ) + "." + QString( kEditStamp ) + ".RA.1.S.123.xml";

         QVERIFY2( QFile::exists( editA ), qPrintable( editA ) );
         QVERIFY2( QFile::exists( editB ), qPrintable( editB ) );

         QCOMPARE( attributeOf( editA, "editGUID", "value" ), attributeOf( editB, "editGUID", "value" ) );
         QCOMPARE( attributeOf( editA, "ODlimit", "value" ), attributeOf( editB, "ODlimit", "value" ) );

         // The raw dataset identity is also reproducible.
         US_DataIO::RawData dataA;
         US_DataIO::RawData dataB;
         QCOMPARE( US_DataIO::readRawData( outA + "/" + QString( kRunID ) + ".RA.1.S.123.auc", dataA ), (int) US_DataIO::OK );
         QCOMPARE( US_DataIO::readRawData( outB + "/" + QString( kRunID ) + ".RA.1.S.123.auc", dataB ), (int) US_DataIO::OK );
         QCOMPARE( US_Util::uuid_unparse( (unsigned char*) dataA.rawGUID ),
                   US_Util::uuid_unparse( (unsigned char*) dataB.rawGUID ) );
      }
};

US3_GUI_TEST_MAIN( US_AstfemSimReproducibilityTest )

#include "test_us_astfem_reproducibility_gui.moc"
