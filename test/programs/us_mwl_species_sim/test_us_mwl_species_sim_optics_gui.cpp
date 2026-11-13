//! \file test_us_mwl_species_sim_optics_gui.cpp
// Verify the multi-wavelength simulator's intensity, window and schedule output.
#include "us3_gui_test_main.h"

#include <QProcess>
#include <QProcessEnvironment>

#include "us3_dev_license.h"
#include "us_dataIO.h"
#include "us_sim_schedule.h"

namespace
{
const char* const kRunID     = "mwl-optics";
const char* const kGuidSeed  = "us3-mwl-optics-regression";
const char* const kEditStamp = "2401010000";
const QStringList kWavelengths{ "250", "280", "310" };

const char* const kSimParams = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                               "<!DOCTYPE US_SimParams>\n"
                               "<SimParams version=\"1.0\">\n"
                               "    <params meshType=\"ASTFEM\" gridType=\"Moving\" simpoints=\"100\""
                               " radialres=\"0.01\" meniscus=\"5.87\" bottom=\"7.2\" rnoise=\"0\""
                               " lrnoise=\"0\" tinoise=\"0\" rinoise=\"0\" baseline=\"0\""
                               " temperature=\"20\" rotorCalID=\"0\" bandform=\"0\" sector=\"0\""
                               " pathlength=\"1.2\" angle=\"2.5\" width=\"0\">\n"
                               "        <speedstep rotorspeed=\"40000\" scans=\"4\" timefirst=\"0\""
                               " timelast=\"0\" w2tfirst=\"0\" w2tlast=\"0\" duration_hrs=\"0\""
                               " duration_mins=\"30\" delay_hrs=\"0\" delay_mins=\"1.875\""
                               " acceleration=\"400\" accelerflag=\"1\" set_speed=\"40000\""
                               " avg_speed=\"40000\" speed_stddev=\"0\"/>\n"
                               "%1"
                               "    </params>\n"
                               "</SimParams>\n";

bool              writeText( const QString& path, const QString& text )
{
   QFile file( path );
   if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return false;
   }
   file.write( text.toUtf8() );
   return true;
}
}

class US_MwlSpeciesSimOpticsTest : public QObject
{
      Q_OBJECT

      QTemporaryDir       scratch;
      QString             inputs;
      QString             outRoot;
      QStringList         models;
      QProcessEnvironment env;

      int                 simulate( const QString& name, const QString& elements, const QStringList& options )
      {
         const QString params = inputs + "/" + name + ".simparams.xml";
         if ( !writeText( params, QString( kSimParams ).arg( elements ) ) || !QDir().mkpath( outRoot + "/" + name ) )
         {
            return -1;
         }
         QProcess sim;
         sim.setProcessEnvironment( env );
         sim.start( US_MWL_SPECIES_SIM_EXE,
                    QStringList{ "--no-db",          "--errors-cl",
                                 "--start",          "--close",
                                 "--models",         models.join( "," ),
                                 "--buffer",         inputs + "/buffer.xml",
                                 "--simparams",      params,
                                 "--rotor",          "1",
                                 "--save",           outRoot + "/" + name,
                                 "--guid-seed",      QString( kGuidSeed ) + "/" + name,
                                 "--edit-timestamp", kEditStamp,
                                 "--noise-seed",     "99" } +
                       options );
         if ( !sim.waitForFinished( 300000 ) )
         {
            sim.kill();
            sim.waitForFinished( 5000 );
            return -1;
         }
         return sim.exitStatus() == QProcess::NormalExit ? sim.exitCode() : -1;
      }

   private slots:
      void initTestCase()
      {
         QVERIFY( scratch.isValid() );
         const QString home = scratch.path() + "/home";
         inputs             = scratch.path() + "/inputs";
         outRoot            = scratch.path() + "/out";
         QVERIFY( QDir().mkpath( inputs ) );
         QVERIFY( QDir().mkpath( outRoot ) );
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

         QProcess gen;
         gen.setProcessEnvironment( env );
         for ( const QString& wavelength : kWavelengths )
         {
            gen.start( US_SIM_INPUTS_GEN_EXE,
                       { "--out", inputs, "--run-id", kRunID, "--cell", "1", "--channel", "A", "--wavelength", wavelength } );
            QVERIFY( gen.waitForFinished( 60000 ) );
            QCOMPARE( gen.exitCode(), 0 );
            models << inputs + "/model_1A_" + wavelength + ".xml";
            QVERIFY2( QFile::exists( models.last() ), qPrintable( models.last() ) );
         }
         gen.start( US_SIM_INPUTS_GEN_EXE,
                    { "--emit-buffer",
                      "--out",
                      inputs + "/buffer.xml",
                      "--density",
                      "0.998234",
                      "--viscosity",
                      "1.001940",
                      "--ph",
                      "7.0" } );
         QVERIFY( gen.waitForFinished( 60000 ) );
         QCOMPARE( gen.exitCode(), 0 );
      }

      // RI with a window, a schedule and telemetry: a sample and a reference
      // per wavelength on the window grid, the wavelengths 40 s apart, and
      // every header recording what the time state records.
      void intensityWavelengthsAreScheduledPairsOnTheWindow()
      {
         const QString elements = "<window/>\n"
                                  "<intensity read_noise=\"0\" shot_noise=\"0\" dark=\"0\"/>\n"
                                  "<schedule slots=\"120\" slot=\"0\" wavelength_seconds=\"40\"/>\n"
                                  "<telemetry seed=\"3\"/>\n";
         QCOMPARE( simulate( "pairs", elements, { "--runtype", "RI" } ), 0 );
         const QDir dir( outRoot + "/pairs" );
         QCOMPARE( dir.entryList( { "*.RI.1.A.*.auc" } ).size(), 3 );
         QCOMPARE( dir.entryList( { "*.RI.1.B.*.auc" } ).size(), 3 );
         QCOMPARE( dir.entryList( { QString( "*.%1.RI.1.*.xml" ).arg( kEditStamp ) } ).size(), 6 );

         const QStringList tmst = dir.entryList( { "*.time_state.tmst" } );
         QCOMPARE( tmst.size(), 1 );
         QVector<US_SimSchedule::Row> rows;
         QStringList                  keys;
         QStringList                  formats;
         QVERIFY( US_SimSchedule::readRows( dir.filePath( tmst.first() ), rows, keys, formats ) );

         double first_scan = -1.0;
         for ( int jw = 0; jw < kWavelengths.size(); jw++ )
         {
            const QStringList sample = dir.entryList( { "*.RI.1.A." + kWavelengths[jw] + ".auc" } );
            const QStringList ref    = dir.entryList( { "*.RI.1.B." + kWavelengths[jw] + ".auc" } );
            QCOMPARE( sample.size(), 1 );
            QCOMPARE( ref.size(), 1 );
            US_DataIO::RawData a;
            US_DataIO::RawData b;
            QCOMPARE( US_DataIO::readRawData( dir.filePath( sample.first() ), a ), (int) US_DataIO::OK );
            QCOMPARE( US_DataIO::readRawData( dir.filePath( ref.first() ), b ), (int) US_DataIO::OK );
            QCOMPARE( a.xvalues.size(), 1500 );
            QVERIFY( qAbs( a.xvalues.first() - 5.75 ) < 1e-6 );
            if ( jw == 0 )
            {
               first_scan = a.scanData[0].seconds;
            }
            QCOMPARE( qRound( a.scanData[0].seconds - first_scan ), 40 * jw );
            const int mid = a.xindex( 6.5 );
            QVERIFY( b.scanData[0].rvalues[mid] > a.scanData[0].rvalues[mid] );
            for ( const US_DataIO::Scan& scan : a.scanData )
            {
               const US_SimSchedule::Row& row = rows[qRound( scan.seconds )];
               QCOMPARE( row.time, qRound( scan.seconds ) );
               QVERIFY( qAbs( scan.omega2t - row.omega2t ) <= 1e-6 * row.omega2t );
               QVERIFY( qAbs( scan.temperature - row.temperature ) < 0.001 );
            }
         }
      }

      void wavelengthScansAreRefused()
      {
         QCOMPARE( simulate( "wi", "", { "--runtype", "WI" } ), 2 );
      }
};

US3_GUI_TEST_MAIN( US_MwlSpeciesSimOpticsTest )

#include "test_us_mwl_species_sim_optics_gui.moc"
