// Verify us_sim_inputs_gen accepts valid options and rejects invalid ones.

#include "us3_gui_test_main.h"

#include <QProcess>

class US_SimInputsGenCliTest : public QObject
{
   Q_OBJECT

   QTemporaryDir scratch;

   // Run the generator; return its exit code and stderr.
   int run( const QStringList& args, QString& err )
   {
      QProcess gen;
      gen.start( US_SIM_INPUTS_GEN_EXE, args );
      if ( ! gen.waitForFinished( 30000 ) )
      {
         gen.kill();
         return -1;
      }
      err = QString::fromUtf8( gen.readAllStandardError() );
      return gen.exitCode();
   }

   QStringList simparams( const QStringList& extra ) const
   {
      return QStringList{ "--emit-simparams", "--out",
                          scratch.filePath( "simparams.xml" ) } + extra;
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
      QTest::addColumn< QStringList >( "args" );
      QTest::addColumn< QString >( "message" );

      QTest::newRow( "negative speed" )
         << simparams( { "--speed", "-1" } ) << "speed must be";
      QTest::newRow( "zero scans" )
         << simparams( { "--scans", "0" } ) << "scans must be";
      QTest::newRow( "negative delay" )
         << simparams( { "--delay-mins", "-5" } ) << "--delay-hrs and --delay-mins";
      QTest::newRow( "band forming, standard centerpiece" )
         << simparams( { "--band-forming", "--centerpiece", "1" } )
         << "band-forming centerpiece";
      QTest::newRow( "centerpiece row out of range" )
         << simparams( { "--centerpiece", "1", "--centerpiece-channel", "C" } )
         << "out of range";
      QTest::newRow( "signed wavelength" )
         << mwlModel( { "--channel", "A", "--wavelength", "-20" } )
         << "--wavelength must be";
      QTest::newRow( "plus-signed wavelength" )
         << mwlModel( { "--channel", "A", "--wavelength", "+99" } )
         << "--wavelength must be";
      QTest::newRow( "cell and channel in --channel" )
         << mwlModel( { "--channel", "1A", "--wavelength", "280" } )
         << "--channel must be one of";
      QTest::newRow( "cell out of range" )
         << mwlModel( { "--cell", "9", "--channel", "A", "--wavelength", "280" } )
         << "--cell must be";
      QTest::newRow( "unknown component key" )
         << QStringList{ "--emit-model", "--out", scratch.filePath( "m.xml" ),
                         "--component", "bogus=1" }
         << "is not one of";
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
      QTest::addColumn< QStringList >( "args" );

      QTest::newRow( "delay minutes only" )
         << simparams( { "--delay-mins", "5" } );
      QTest::newRow( "band forming, band-forming centerpiece" )
         << simparams( { "--band-forming", "--centerpiece", "6",
                         "--band-volume", "15" } );
      QTest::newRow( "centerpiece row by letter" )
         << simparams( { "--centerpiece", "3", "--centerpiece-channel", "C" } );
      QTest::newRow( "single-channel row" )
         << simparams( { "--centerpiece-channel", "S" } );
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
      QCOMPARE( run( mwlModel( { "--cell", "2", "--channel", "b",
                                 "--wavelength", "280" } ), err ), 0 );
      QVERIFY( QFile::exists( scratch.filePath( "model_2B_280.xml" ) ) );
   }
};

US3_GUI_TEST_MAIN(US_SimInputsGenCliTest)

#include "test_us_sim_inputs_gen_cli.moc"
