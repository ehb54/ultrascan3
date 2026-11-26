//! \file test_us_gui_util_svg.cpp
// US_GuiUtil::save_svg() leaves a loadable .svgz and no .svg beside it.
// The plot window's SVG and PNG exports report a file that cannot be written,
// and US_GuiUtil::warn_unsaved() names the files.

#include "us3_gui_test_main.h"

#include "qwt_plot.h"
#include "qwt_text.h"
#include "us_gui_util.h"
#include "us_plot.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QSvgRenderer>
#include <QTemporaryDir>
#include <QTimer>

class US_GuiUtilSvgTest : public QObject
{
      Q_OBJECT

      //! Runs a US_Plot export, names the file in its dialog and returns the warnings shown
      static QStringList exportAs( const char* slot, const QString& name )
      {
         QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
         QWidget  window;
         QwtPlot* plot   = nullptr;
         auto*    layout = new US_Plot( plot, "title", "x", "y" );
         window.setLayout( layout );
         plot->resize( 400, 300 );

         QStringList warnings;
         QTimer      timer;
         QObject::connect( &timer,
                           &QTimer::timeout,
                           &timer,
                           [&]()
                           {
                              QWidget* modal = QApplication::activeModalWidget();
                              if ( auto* files = qobject_cast<QFileDialog*>( modal ) )
                              {
                                 files->findChild<QLineEdit*>( "fileNameEdit" )->setText( name );
                                 QMetaObject::invokeMethod( files, "accept" );
                              }
                              else if ( auto* box = qobject_cast<QMessageBox*>( modal ) )
                              {
                                 warnings << box->text();
                                 box->accept();
                              }
                           } );
         timer.start( 20 );
         QMetaObject::invokeMethod( layout, slot );
         return warnings;
      }

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

         QCOMPARE( US_GuiUtil::save_svg( dir.filePath( "plot.svgz" ), &plot ), 3 );

         QVERIFY( QFile::exists( dir.filePath( "plot.svg" ) ) );
      }

      void savePlotReportsASvgFailureAndStillWritesThePng()
      {
         QTemporaryDir dir;
         QVERIFY( dir.isValid() );
         QVERIFY( QDir( dir.path() ).mkdir( "plot.svgz" ) );
         QwtPlot plot;
         plot.resize( 400, 300 );

         QCOMPARE( US_GuiUtil::save_plot( dir.filePath( "plot.svgz" ), &plot ), 3 );

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

      void savePlotReportsTheSvgWhenBothFail()
      {
         QTemporaryDir dir;
         QVERIFY( dir.isValid() );
         QVERIFY( QDir( dir.path() ).mkdir( "plot.svgz" ) );
         QVERIFY( QDir( dir.path() ).mkdir( "plot.png" ) );
         QwtPlot plot;
         plot.resize( 400, 300 );

         QCOMPARE( US_GuiUtil::save_plot( dir.filePath( "plot.svgz" ), &plot ), 3 );
      }

      void savePlotRefusesOtherExtensions()
      {
         QwtPlot plot;
         QCOMPARE( US_GuiUtil::save_plot( "plot.txt", &plot ), 1 );
      }

      void exportsWriteTheFileWithoutAWarning()
      {
         QTemporaryDir dir;
         QVERIFY( dir.isValid() );

         QCOMPARE( exportAs( "svg", dir.filePath( "plot" ) ), QStringList() );
         QCOMPARE( exportAs( "png", dir.filePath( "plot" ) ), QStringList() );

         QVERIFY( QFile::exists( dir.filePath( "plot.svgz" ) ) );
         QVERIFY( QFile::exists( dir.filePath( "plot.png" ) ) );
      }

      void exportsWarnWhenTheFileCannotBeWritten()
      {
         QTemporaryDir dir;
         QVERIFY( dir.isValid() );
         QVERIFY( QDir( dir.path() ).mkdir( "plot.svgz" ) );
         QVERIFY( QDir( dir.path() ).mkdir( "plot.png" ) );

         const QStringList svg = exportAs( "svg", dir.filePath( "plot" ) );
         const QStringList png = exportAs( "png", dir.filePath( "plot" ) );

         QCOMPARE( svg.size(), 1 );
         QVERIFY2( svg[0].endsWith( "\n" + dir.filePath( "plot.svgz" ) ), qPrintable( svg[0] ) );
         QCOMPARE( png.size(), 1 );
         QVERIFY2( png[0].endsWith( "\n" + dir.filePath( "plot.png" ) ), qPrintable( png[0] ) );
      }

      void warnUnsavedNamesTheFiles()
      {
         QVERIFY( !US_GuiUtil::warn_unsaved( nullptr, QStringList() ) );

         QStringList files;
         for ( int ii = 1; ii <= 11; ++ii )
         {
            files << QString( "frame%1.png" ).arg( ii );
         }

         QString text;
         QTimer::singleShot( 0,
                             this,
                             [&]()
                             {
                                auto* box = qobject_cast<QMessageBox*>( QApplication::activeModalWidget() );
                                QVERIFY( box != nullptr );
                                text = box->text();
                                box->accept();
                             } );
         QVERIFY( US_GuiUtil::warn_unsaved( nullptr, files ) );

         QVERIFY2( text.startsWith( "Unable to write 11 file(s):\nframe1.png\n" ), qPrintable( text ) );
         QVERIFY2( text.endsWith( "\nframe10.png\n..." ), qPrintable( text ) );
      }

      void otherExtensionsAreRefused()
      {
         QwtPlot plot;
         QCOMPARE( US_GuiUtil::save_svg( "plot.png", &plot ), 1 );
      }
};

US3_GUI_TEST_MAIN( US_GuiUtilSvgTest )

#include "test_us_gui_util_svg.moc"
