//! \file test_us_sim_inputs_gen_cli.cpp
// Verify us_sim_inputs_gen accepts valid options and rejects invalid ones.

#include "us3_gui_test_main.h"

#include <QProcess>

#include "us_hardware.h"
#include "us_sim_inputs.h"
#include "us_sim_inputs_gen_cli.h"
#include "us_util.h"

class US_SimInputsGenCliTest : public QObject
{
      Q_OBJECT

      QTemporaryDir scratch;

      // Run the generator; return its exit code and stderr.
      int           run( const QStringList& args, QString& err )
      {
         QProcess gen;
         gen.start( US_SIM_INPUTS_GEN_EXE, args );
         if ( !gen.waitForFinished( 30000 ) )
         {
            gen.kill();
            return -1;
         }
         err = QString::fromUtf8( gen.readAllStandardError() );
         return gen.exitCode();
      }

      QStringList simparams( const QStringList& extra ) const
      {
         return QStringList{ "--emit-simparams", "--out", scratch.filePath( "simparams.xml" ) } + extra;
      }

      QStringList mwlModel( const QStringList& extra ) const
      {
         return QStringList{ "--out", scratch.path(), "--run-id", "r" } + extra;
      }

   private slots:
      void initTestCase()
      {
         QVERIFY( scratch.isValid() );
      }

      void invalidOptionsAreRejected_data()
      {
         QTest::addColumn<QStringList>( "args" );
         QTest::addColumn<QString>( "message" );

         QTest::newRow( "negative speed" ) << simparams( { "--speed", "-1" } )
                                           << QString( US_SimInputs::notPositiveMessage ).arg( "speed" ).arg( -1.0 );
         QTest::newRow( "zero scans" ) << simparams( { "--scans", "0" } ) << QString( US_SimInputs::tooFewScansMessage ).arg( 0 );
         QTest::newRow( "negative delay" ) << simparams( { "--delay-mins", "-5" } )
                                           << QString( US_SimInputsGenMessages::negativeDelay );
         QTest::newRow( "band forming, standard centerpiece" )
            << simparams( { "--band-forming", "--centerpiece", "1" } ) << QString( US_SimInputs::notBandFormingMessage ).arg( 1 );
         QTest::newRow( "centerpiece row out of range" )
            << simparams( { "--centerpiece", "1", "--centerpiece-channel", "C" } ) << US_AbstractCenterpiece::validate( 1, 1 );
         QTest::newRow( "signed wavelength" )
            << mwlModel( { "--channel", "A", "--wavelength", "-20" } ) << QString( US_SimInputsGenMessages::badWavelength );
         QTest::newRow( "plus-signed wavelength" )
            << mwlModel( { "--channel", "A", "--wavelength", "+99" } ) << QString( US_SimInputsGenMessages::badWavelength );
         QTest::newRow( "cell and channel in --channel" )
            << mwlModel( { "--channel", "1A", "--wavelength", "280" } )
            << QString( US_SimInputsGenMessages::badChannel ).arg( US_Util::channel_letters() );
         QTest::newRow( "cell out of range" ) << mwlModel( { "--cell", "9", "--channel", "A", "--wavelength", "280" } )
                                              << QString( US_SimInputsGenMessages::badCell );
         QTest::newRow( "unknown component key" )
            << QStringList{ "--emit-model", "--out", scratch.filePath( "m.xml" ), "--component", "bogus=1" }
            << QString( US_SimInputsGenMessages::unknownComponentKey ).arg( "bogus", component_keys().join( ", " ) );
      }

      void invalidOptionsAreRejected()
      {
         QFETCH( QStringList, args );
         QFETCH( QString, message );

         QString err;
         QCOMPARE( run( args, err ), 1 );
         QVERIFY2( err.contains( message ), qPrintable( err ) );
      }

      void validOptionsAreAccepted_data()
      {
         QTest::addColumn<QStringList>( "args" );

         QTest::newRow( "delay minutes only" ) << simparams( { "--delay-mins", "5" } );
         QTest::newRow( "band forming, band-forming centerpiece" )
            << simparams( { "--band-forming", "--centerpiece", "6", "--band-volume", "15" } );
         QTest::newRow( "centerpiece row by letter" ) << simparams( { "--centerpiece", "3", "--centerpiece-channel", "C" } );
         QTest::newRow( "single-channel row" ) << simparams( { "--centerpiece-channel", "S" } );
         QTest::newRow( "MWL model with cell and channel" )
            << mwlModel( { "--cell", "2", "--channel", "b", "--wavelength", "280" } );
      }

      void validOptionsAreAccepted()
      {
         QFETCH( QStringList, args );

         QString err;
         QCOMPARE( run( args, err ), 0 );
      }

      void mwlModelNamesCombineCellAndChannel()
      {
         QString err;
         QCOMPARE( run( mwlModel( { "--cell", "2", "--channel", "b", "--wavelength", "280" } ), err ), 0 );
         QVERIFY( QFile::exists( scratch.filePath( "model_2B_280.xml" ) ) );
      }

      void defaultModeHonoursGuidSeed()
      {
         // Generate the default inputs into the named directory with the given seed.
         auto generate = [this]( const QString& name, const QString& seed ) -> QString
         {
            const QString dir = scratch.filePath( name );
            QString       err;
            if ( !QDir().mkpath( dir ) || run( { "--out", dir, "--guid-seed", seed }, err ) != 0 )
            {
               return QString();
            }
            return dir;
         };
         auto contents = []( const QString& path ) -> QByteArray
         {
            QFile file( path );
            return file.open( QIODevice::ReadOnly ) ? file.readAll() : QByteArray();
         };

         const QString first  = generate( "default_1", "s1" );
         const QString second = generate( "default_2", "s1" );
         const QString other  = generate( "default_3", "s2" );
         QVERIFY( !first.isEmpty() && !second.isEmpty() && !other.isEmpty() );

         for ( const QString& name : { "model_default.xml", "buffer_default.xml", "sp_default.xml" } )
         {
            const QByteArray bytes = contents( first + "/" + name );
            QVERIFY2( !bytes.isEmpty(), qPrintable( name ) );
            QVERIFY2( bytes == contents( second + "/" + name ), qPrintable( name + " differs for the same seed" ) );
         }

         for ( const QString& name : { "model_default.xml", "buffer_default.xml" } )
         {
            QVERIFY2( contents( first + "/" + name ) != contents( other + "/" + name ),
                      qPrintable( name + " is the same for different seeds" ) );
         }
      }
};

US3_GUI_TEST_MAIN( US_SimInputsGenCliTest )

#include "test_us_sim_inputs_gen_cli.moc"
