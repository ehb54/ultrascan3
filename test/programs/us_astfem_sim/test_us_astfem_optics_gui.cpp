//! \file test_us_astfem_optics_gui.cpp
// Verify us_astfem_sim's clipping, intensity, window, schedule, interference and wavelength-scan output.
#include "us3_gui_test_main.h"

#include <QCryptographicHash>
#include <QProcess>
#include <QProcessEnvironment>
#include <QXmlStreamReader>

#include <cmath>

#include "us3_dev_license.h"
#include "us_dataIO.h"
#include "us_sim_schedule.h"

namespace
{
const char* const kGuidSeed  = "us3-optics-regression";
const char* const kEditStamp = "2401010000";

// One speed step, a few scans: enough to exercise every output shape quickly.
const char* const kSimParams = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                               "<!DOCTYPE US_SimParams>\n"
                               "<SimParams version=\"1.0\">\n"
                               "    <params meshType=\"ASTFEM\" gridType=\"Moving\" simpoints=\"100\""
                               " radialres=\"0.001\" meniscus=\"5.87\" bottom=\"7.2\" rnoise=\"0\""
                               " lrnoise=\"0\" tinoise=\"0\" rinoise=\"0\" baseline=\"0\""
                               " temperature=\"20\" rotorCalID=\"0\" bandform=\"0\" sector=\"0\""
                               " pathlength=\"1.2\" angle=\"2.5\" width=\"0\">\n"
                               "        <speedstep rotorspeed=\"45000\" scans=\"6\" timefirst=\"0\""
                               " timelast=\"0\" w2tfirst=\"0\" w2tlast=\"0\" duration_hrs=\"0\""
                               " duration_mins=\"40\" delay_hrs=\"0\" delay_mins=\"1.875\""
                               " acceleration=\"400\" accelerflag=\"1\" set_speed=\"45000\""
                               " avg_speed=\"45000\" speed_stddev=\"0\"/>\n"
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

QString readText( const QString& path )
{
   QFile file( path );
   return file.open( QIODevice::ReadOnly | QIODevice::Text ) ? QString::fromUtf8( file.readAll() ) : QString();
}

// The first value of an attribute on an element, or an empty string.
QString attributeOf( const QString& path, const QString& element, const QString& attribute )
{
   QFile file( path );
   if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
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

QByteArray sha256( const QString& path )
{
   QFile file( path );
   return file.open( QIODevice::ReadOnly ) ? QCryptographicHash::hash( file.readAll(), QCryptographicHash::Sha256 ) : QByteArray();
}
}

class US_AstfemSimOpticsTest : public QObject
{
      Q_OBJECT

      QTemporaryDir       scratch;
      QString             outRoot;
      QString             inputs;
      QProcessEnvironment env;

      // Simulate with extra simparams elements and options; return the exit code.
      // The run is saved as <outRoot>/<name>/<name>.*.
      int                 simulate( const QString&     name,
                                    const QString&     elements,
                                    const QStringList& options,
                                    const QString&     paramsTemplate = QString( kSimParams ) )
      {
         const QString params = inputs + "/" + name + ".simparams.xml";
         if ( !writeText( params, paramsTemplate.arg( elements ) ) || !QDir().mkpath( outRoot + "/" + name ) )
         {
            return -1;
         }
         QProcess sim;
         sim.setProcessEnvironment( env );
         sim.start( US_ASTFEM_SIM_EXE,
                    QStringList{ "--no-db",          "--errors-cl",
                                 "--start",          "--close",
                                 "--model",          inputs + "/model.xml",
                                 "--buffer",         inputs + "/buffer.xml",
                                 "--simparams",      params,
                                 "--rotor",          "1",
                                 "--save",           outRoot + "/" + name,
                                 "--guid-seed",      QString( kGuidSeed ) + "/" + name,
                                 "--edit-timestamp", kEditStamp,
                                 "--noise-seed",     "4242" } +
                       options );
         if ( !sim.waitForFinished( 300000 ) )
         {
            sim.kill();
            sim.waitForFinished( 5000 );
            return -1;
         }
         return sim.exitStatus() == QProcess::NormalExit ? sim.exitCode() : -1;
      }

      QString twoSpeedParams() const
      {
         QString       params( kSimParams );
         const int     start = params.indexOf( "<speedstep " );
         const int     end   = params.indexOf( "/>", start ) + 2;
         const QString step  = params.mid( start, end - start );
         params.replace(
            start, end - start, QString( step ).replace( "45000", "30000" ) + "\n" + QString( step ).replace( "45000", "50000" ) );
         return params;
      }

      QString dir( const QString& name ) const
      {
         return outRoot + "/" + name;
      }

      QString file( const QString& name, const QString& pattern ) const
      {
         const QStringList found = QDir( dir( name ) ).entryList( { pattern }, QDir::Files, QDir::Name );
         return found.size() == 1 ? dir( name ) + "/" + found.first() : QString();
      }

      bool read( const QString& path, US_DataIO::RawData& data ) const
      {
         return !path.isEmpty() && US_DataIO::readRawData( path, data ) == US_DataIO::OK;
      }

   private slots:
      void initTestCase()
      {
         QVERIFY( scratch.isValid() );
         inputs             = scratch.path() + "/inputs";
         outRoot            = scratch.path() + "/out";
         const QString home = scratch.path() + "/home";
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
         gen.start( US_SIM_INPUTS_GEN_EXE,
                    { "--emit-model",
                      "--out",
                      inputs + "/model.xml",
                      "-s",
                      "4.58S",
                      "--mw",
                      "66430",
                      "--vbar20",
                      "0.733",
                      "--name",
                      "BSA" } );
         QVERIFY( gen.waitForFinished( 60000 ) );
         QCOMPARE( gen.exitCode(), 0 );
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

      // A clipped run's edit file marks the clipped readings for exclusion:
      // without that, the flat clipped region is fitted as data.
      void clippedWindowedRunEditsExcludeTheClip()
      {
         QCOMPARE( simulate( "clipped", "<window/>\n", { "--od-limit", "1.5" } ), 0 );
         US_DataIO::RawData data;
         QVERIFY( read( file( "clipped", "*.RA.*.auc" ), data ) );
         QCOMPARE( data.xvalues.size(), 1500 );
         QVERIFY( qAbs( data.xvalues.first() - 5.75 ) < 1e-6 );
         double peak = 0.0;
         for ( const US_DataIO::Scan& scan : data.scanData )
         {
            for ( double value : scan.rvalues )
            {
               peak = qMax( peak, value );
            }
         }
         QVERIFY2( peak <= 1.5 + 1e-6, qPrintable( QString::number( peak ) ) );
         const QString edit = file( "clipped", QString( "*.%1.RA.*.xml" ).arg( kEditStamp ) );
         QVERIFY( qAbs( attributeOf( edit, "od_limit", "value" ).toDouble() - 1.5 ) < 1e-4 );
      }

      // The solver drops readings at or above the edit's OD limit. With a limit
      // a float cannot hold exactly, such as 0.9, every clipped reading read
      // back from the .auc must still reach the edit's limit, and no reading
      // below the clip may.
      void inexactLimitStillExcludesEveryClippedReading()
      {
         QCOMPARE( simulate( "clipinexact", "<window/>\n", { "--od-limit", "0.9" } ), 0 );
         US_DataIO::RawData data;
         QVERIFY( read( file( "clipinexact", "*.RA.*.auc" ), data ) );
         const QString edit  = file( "clipinexact", QString( "*.%1.RA.*.xml" ).arg( kEditStamp ) );
         const double  limit = attributeOf( edit, "od_limit", "value" ).toDouble();
         QVERIFY( qAbs( limit - 0.9 ) < 1e-4 );
         double peak = 0.0;
         for ( const US_DataIO::Scan& scan : data.scanData )
         {
            for ( double value : scan.rvalues )
            {
               peak = qMax( peak, value );
            }
         }
         int clipped  = 0;
         int excluded = 0;
         for ( const US_DataIO::Scan& scan : data.scanData )
         {
            for ( double value : scan.rvalues )
            {
               clipped += ( value == peak ) ? 1 : 0;
               excluded += ( value >= limit ) ? 1 : 0;
            }
         }
         QVERIFY( clipped > 1 );
         QCOMPARE( excluded, clipped );
      }

      void intensityRunWritesSampleAndReference()
      {
         QCOMPARE( simulate( "intensity",
                             "<intensity read_noise=\"0\" shot_noise=\"0\" dark=\"0\"/>\n",
                             { "--runtype", "RI", "--channel", "A" } ),
                   0 );
         US_DataIO::RawData sample;
         US_DataIO::RawData reference;
         QVERIFY( read( file( "intensity", "*.RI.1.A.*.auc" ), sample ) );
         QVERIFY( read( file( "intensity", "*.RI.1.B.*.auc" ), reference ) );
         QVERIFY( memcmp( sample.rawGUID, reference.rawGUID, 16 ) != 0 );
         QCOMPARE( QDir( dir( "intensity" ) ).entryList( { QString( "*.%1.RI.1.*.xml" ).arg( kEditStamp ) } ).size(), 2 );
         QCOMPARE( readText( file( "intensity", "*.RI.xml" ) ).count( "<dataset " ), 2 );
         // With noise off, the pair's ratio is the absorbance, about 1 on the plateau.
         const int    mid = sample.xindex( 6.5 );
         const double a   = std::log10( reference.scanData[0].rvalues[mid] / sample.scanData[0].rvalues[mid] );
         QVERIFY2( a > 0.8 && a < 1.2, qPrintable( QString::number( a ) ) );
      }

      // Every channel of a scheduled run writes the same time state, and each
      // scan header records what the time state records at its second.
      void scheduledChannelsShareOneTimeState()
      {
         const QString telemetry = "<telemetry seed=\"7\"/>\n";
         QCOMPARE( simulate( "slot0", "<schedule slots=\"23,23\" slot=\"0\"/>\n" + telemetry, {} ), 0 );
         QCOMPARE( simulate( "slot1", "<schedule slots=\"23,23\" slot=\"1\"/>\n" + telemetry, { "--centerpiece", "2" } ), 0 );
         US_DataIO::RawData first;
         US_DataIO::RawData second;
         QVERIFY( read( file( "slot0", "*.auc" ), first ) );
         QVERIFY( read( file( "slot1", "*.auc" ), second ) );
         QCOMPARE( qRound( second.scanData[0].seconds - first.scanData[0].seconds ), 23 );
         QCOMPARE( qRound( first.scanData[1].seconds - first.scanData[0].seconds ), 46 );
         const QString tmst = file( "slot0", "*.time_state.tmst" );
         QCOMPARE( sha256( tmst ), sha256( file( "slot1", "*.time_state.tmst" ) ) );

         QVector<US_SimSchedule::Row> rows;
         QStringList                  keys;
         QStringList                  formats;
         QVERIFY( US_SimSchedule::readRows( tmst, rows, keys, formats ) );
         for ( const US_DataIO::Scan& scan : second.scanData )
         {
            const US_SimSchedule::Row& row = rows[qRound( scan.seconds )];
            QCOMPARE( row.time, qRound( scan.seconds ) );
            // US_DataIO::readRawData rounds a scan's rpm to the speed resolution
            // (100 rpm), so the recorded jitter shows in omega2t, not in rpm.
            const QString at = QString( "scan at %1 s: header rpm %2 omega2t %3 temperature %4, "
                                        "time state rpm %5 omega2t %6 temperature %7" )
                                  .arg( scan.seconds )
                                  .arg( scan.rpm, 0, 'f', 3 )
                                  .arg( scan.omega2t, 0, 'e', 9 )
                                  .arg( scan.temperature, 0, 'f', 4 )
                                  .arg( row.raw_speed, 0, 'f', 3 )
                                  .arg( row.omega2t, 0, 'e', 9 )
                                  .arg( row.temperature, 0, 'f', 4 );
            QVERIFY2( qAbs( scan.rpm - qRound( row.raw_speed / 100.0 ) * 100.0 ) < 0.01, qPrintable( at ) );
            QVERIFY2( qAbs( scan.omega2t - row.omega2t ) <= 1e-6 * row.omega2t, qPrintable( at ) );
            QVERIFY2( qAbs( scan.temperature - row.temperature ) < 0.001, qPrintable( at ) );
            QVERIFY( row.scan > 0 );
         }
      }

      void scheduledSpeedStepsMatchTimeState_data()
      {
         QTest::addColumn<bool>( "telemetry" );
         QTest::newRow( "schedule-only" ) << false;
         QTest::newRow( "with-telemetry" ) << true;
      }

      // Regression for #616: step two must start after the configured 40-minute
      // first step, not after its final scan cycle at 343 seconds.
      void scheduledSpeedStepsMatchTimeState()
      {
         QFETCH( bool, telemetry );
         QByteArray sharedTimeState;
         for ( int slot = 0; slot < 2; slot++ )
         {
            const QString name     = QString( "multispeed%1_%2" ).arg( telemetry ).arg( slot );
            QString       elements = QString( "<schedule slots=\"23,23\" slot=\"%1\"/>\n" ).arg( slot );
            if ( telemetry )
            {
               elements += "<telemetry seed=\"7\"/>\n";
            }
            // Slot 1 also rebuilds the grid after applying centerpiece geometry.
            const QStringList options = slot == 0 ? QStringList{} : QStringList{ "--centerpiece", "2" };
            QCOMPARE( simulate( name, elements, options, twoSpeedParams() ), 0 );
            for ( int step = 0; step < 2; step++ )
            {
               const int          rpm = step == 0 ? 30000 : 50000;
               const QString      run = name + QString( "-%1" ).arg( rpm, 6, 10, QChar( '0' ) );
               US_DataIO::RawData data;
               QVERIFY2( read( file( run, "*.auc" ), data ), qPrintable( run ) );
               QCOMPARE( data.scanData.size(), 6 );
               const QString    tmst   = file( run, "*.time_state.tmst" );
               const QByteArray digest = sha256( tmst );
               QVERIFY( !digest.isEmpty() );
               if ( sharedTimeState.isEmpty() )
               {
                  sharedTimeState = digest;
               }
               QCOMPARE( digest, sharedTimeState );
               QVector<US_SimSchedule::Row> rows;
               QStringList                  keys;
               QStringList                  formats;
               QVERIFY( US_SimSchedule::readRows( tmst, rows, keys, formats ) );
               for ( int scan = 0; scan < data.scanData.size(); scan++ )
               {
                  const US_DataIO::Scan& header  = data.scanData[scan];
                  const int              seconds = qRound( header.seconds );
                  QVERIFY( seconds >= 0 && seconds < rows.size() );
                  const US_SimSchedule::Row& row = rows[seconds];
                  QCOMPARE( row.time, seconds );
                  QCOMPARE( row.step, step + 1 );
                  QCOMPARE( qRound( header.rpm ), rpm );
                  QCOMPARE( row.set_speed, rpm );
                  QCOMPARE( qRound( row.raw_speed / 100.0 ) * 100, rpm );
                  QCOMPARE( seconds, step * 2400 + 113 + slot * 23 + scan * 46 );
                  QVERIFY( row.scan > 0 );
                  if ( telemetry )
                  {
                     QVERIFY( qAbs( header.omega2t - row.omega2t ) <= 1e-6 * row.omega2t );
                  }
               }
            }
         }
      }

      void scheduledScansMustFitTheirOwnSpeedStep()
      {
         QString   params = twoSpeedParams();
         // The last slot's last scan is 366 seconds into each step: it fits
         // the first 40-minute step, but not a six-minute second step.
         const int second = params.indexOf( "<speedstep ", params.indexOf( "<speedstep " ) + 1 );
         params.replace(
            params.indexOf( "duration_mins=\"40\"", second ), QString( "duration_mins=\"40\"" ).size(), "duration_mins=\"6\"" );
         QCOMPARE( simulate( "overflow", "<schedule slots=\"23,23\" slot=\"0\"/>\n", {}, params ), 2 );
         QVERIFY( file( "overflow-030000", "*.auc" ).isEmpty() );
         QVERIFY( file( "overflow-050000", "*.auc" ).isEmpty() );
      }

      void interferenceRunUsesItsDetectorGrid()
      {
         QCOMPARE( simulate( "fringes", "<interference/>\n", { "--runtype", "IP", "--od-limit", "5" } ), 0 );
         US_DataIO::RawData data;
         QVERIFY( read( file( "fringes", "*.IP.*.auc" ), data ) );
         QCOMPARE( data.xvalues.size(), 2048 );
         QVERIFY( qAbs( data.xvalues.first() - 5.80 ) < 1e-6 );
         QVERIFY( QFile::exists( dir( "fringes" ) + "/ASTFEM_TI_NOISE.csv" ) );
         const QString edit  = file( "fringes", QString( "*.%1.IP.*.xml" ).arg( kEditStamp ) );
         const double  right = attributeOf( edit, "data_range", "right" ).toDouble();
         QVERIFY2( right <= data.xvalues.last() + 1e-6, qPrintable( QString::number( right ) ) );
         QVERIFY( qAbs( attributeOf( edit, "od_limit", "value" ).toDouble() - 5.0 ) < 1e-4 );
      }

      // A detector grid that misses the meniscus cannot be edited; refuse it.
      void interferenceGridMustHoldTheMeniscus()
      {
         QCOMPARE( simulate( "offgrid", "<interference start=\"6.3\"/>\n", { "--runtype", "IP" } ), 2 );
         QVERIFY( QDir( dir( "offgrid" ) ).entryList( { "*.auc" } ).isEmpty() );
      }

      void wavelengthScanWritesSpectraAtOneRadius()
      {
         const QString scan = "<wavelength_scan position=\"6.5\" start=\"250\" end=\"270\" step=\"1\">"
                              "<point wavelength=\"250\" extinction=\"500\"/><point wavelength=\"260\" extinction=\"1000\"/>"
                              "<point wavelength=\"270\" extinction=\"500\"/></wavelength_scan>\n";
         QCOMPARE( simulate( "spectrum", scan, { "--runtype", "WI", "--channel", "A" } ), 0 );
         US_DataIO::RawData sample;
         US_DataIO::RawData reference;
         QVERIFY( read( file( "spectrum", "*.WI.1.A.123.auc" ), sample ) );
         QVERIFY( read( file( "spectrum", "*.WI.1.B.123.auc" ), reference ) );
         QCOMPARE( sample.xvalues.size(), 21 );
         QVERIFY( qAbs( sample.xvalues.first() - 250.0 ) < 1e-6 );
         QVERIFY( qAbs( sample.scanData[0].wavelength - 6.5 ) < 1e-6 );
         for ( const QString& channel : { "A", "B" } )
         {
            const QString edit = file( "spectrum", QString( "*.%1.WI.1.%2.123.xml" ).arg( kEditStamp, channel ) );
            QCOMPARE( attributeOf( edit, "data_range", "left" ).toDouble(), 250.0 );
            QCOMPARE( attributeOf( edit, "data_range", "right" ).toDouble(), 270.0 );
            QVERIFY( !readText( edit ).contains( "meniscus" ) );
         }
      }

      // The new run types refuse what they cannot write.
      void unusableRequestsAreRefused()
      {
         QCOMPARE( simulate( "nochannel", "", { "--runtype", "RI" } ), 2 );                // channel S has no partner
         QCOMPARE( simulate( "noscan", "", { "--runtype", "WI", "--channel", "A" } ), 2 ); // no <wavelength_scan>
         QCOMPARE( simulate( "clipri", "", { "--runtype", "RI", "--channel", "A", "--od-limit", "3" } ), 2 );
      }
};

US3_GUI_TEST_MAIN( US_AstfemSimOpticsTest )

#include "test_us_astfem_optics_gui.moc"
