//! \file test_us_gui_util_svg.cpp
// US_GuiUtil::save_svg() leaves a loadable .svgz and no .svg beside it.

#include "us3_gui_test_main.h"

#include "qwt_plot.h"
#include "qwt_text.h"
#include "us_gui_util.h"

#include <QDir>
#include <QFile>
#include <QSvgRenderer>
#include <QTemporaryDir>

class US_GuiUtilSvgTest : public QObject
{
      Q_OBJECT

   private slots:
      void savesCompressedSvg()
      {
         QTemporaryDir dir;
         QVERIFY( dir.isValid() );
         QwtPlot plot( QwtText( "velocity" ) );
         plot.resize( 400, 300 );

         QCOMPARE( US_GuiUtil::save_svg( dir.filePath( "plot.svgz" ), &plot ), 0 );

         QFile file( dir.filePath( "plot.svgz" ) );
         QVERIFY( file.open( QIODevice::ReadOnly ) );
         QCOMPARE( file.read( 3 ).toHex(), QByteArray( "1f8b08" ) );
         QVERIFY( QSvgRenderer( file.fileName() ).isValid() );
         QVERIFY( !QFile::exists( dir.filePath( "plot.svg" ) ) );
      }

      void savePlotWritesSvgzAndPng()
      {
         QTemporaryDir dir;
         QVERIFY( dir.isValid() );
         QwtPlot plot;
         plot.resize( 400, 300 );

         QCOMPARE( US_GuiUtil::save_plot( dir.filePath( "plot.svgz" ), &plot ), 0 );

         QVERIFY( QFile::exists( dir.filePath( "plot.svgz" ) ) );
         QVERIFY( QFile::exists( dir.filePath( "plot.png" ) ) );
         QVERIFY( !QFile::exists( dir.filePath( "plot.svg" ) ) );
      }

      void compressionFailureKeepsTheSvg()
      {
         QTemporaryDir dir;
         QVERIFY( dir.isValid() );
         // A folder where the .svgz belongs cannot be replaced by a file
         QVERIFY( QDir( dir.path() ).mkdir( "plot.svgz" ) );
         QwtPlot plot;
         plot.resize( 400, 300 );

         QCOMPARE( US_GuiUtil::save_svg( dir.filePath( "plot.svgz" ), &plot ), 2 );

         QVERIFY( QFile::exists( dir.filePath( "plot.svg" ) ) );
      }

      void savePlotReportsASvgFailureAndStillWritesThePng()
      {
         QTemporaryDir dir;
         QVERIFY( dir.isValid() );
         QVERIFY( QDir( dir.path() ).mkdir( "plot.svgz" ) );
         QwtPlot plot;
         plot.resize( 400, 300 );

         QCOMPARE( US_GuiUtil::save_plot( dir.filePath( "plot.svgz" ), &plot ), 2 );

         QVERIFY( QFile::exists( dir.filePath( "plot.png" ) ) );
      }

      void savePlotReportsAPngFailure()
      {
         QTemporaryDir dir;
         QVERIFY( dir.isValid() );
         // A folder where the .png belongs cannot be opened as a file
         QVERIFY( QDir( dir.path() ).mkdir( "plot.png" ) );
         QwtPlot plot;
         plot.resize( 400, 300 );

         QCOMPARE( US_GuiUtil::save_plot( dir.filePath( "plot.svgz" ), &plot ), 2 );
         QVERIFY( QFile::exists( dir.filePath( "plot.svgz" ) ) );
         QCOMPARE( US_GuiUtil::save_plot( dir.filePath( "plot.png" ), &plot ), 2 );
      }

      void savePlotRefusesOtherExtensions()
      {
         QwtPlot plot;
         QCOMPARE( US_GuiUtil::save_plot( "plot.txt", &plot ), 1 );
      }

      void otherExtensionsAreRefused()
      {
         QwtPlot plot;
         QCOMPARE( US_GuiUtil::save_svg( "plot.png", &plot ), 1 );
      }
};

US3_GUI_TEST_MAIN( US_GuiUtilSvgTest )

#include "test_us_gui_util_svg.moc"
