//! \file test_us_analysis_profile_inherit_gui.cpp
#include "profile_fixture.h"
#include "profile_snapshot.h"
#include "us3_gui_test_main.h"
#include "us_analysis_profile.h"
#include <QJsonDocument>
#include <QMessageBox>
#include <QTimer>

class US_AnalysisProfileInheritTest : public QObject
{
      Q_OBJECT
   private slots:
      void record_data()
      {
         QTest::addColumn<QString>( "name" );
         for ( const QString& name : ProfileFixture::cases() )
         {
            QTest::newRow( qPrintable( name ) ) << name;
         }
      }
      void record()
      {
         QFETCH( QString, name );
         if ( name.startsWith( "database" ) && qEnvironmentVariableIsEmpty( "US3_PROFILE_DB_HOST" ) )
         {
            QSKIP( "Set US3_PROFILE_DB_HOST for the disposable fixture server" );
         }
         US_Settings::set_default_data_location( 2 );
         qApp->setProperty( "us3_headless", true );
         QVERIFY( QDir().mkpath( US_Settings::dataDir() + "/projects" ) );
         auto                  f = ProfileFixture::make( name );
         US_AnalysisProfileGui window;
         if ( !f.fresh )
         {
            window.currProf = f.initial;
         }
         else
         {
            // The only members the constructors leave undefined
            window.currProf.aprofID = -101;
            window.currProf.protoID = -102;
            window.currProf.nchan   = -103;
         }
         window.abde_mode_aprofile = f.abde;
         QJsonArray messages;
         QJsonArray progress;
         window.progress_cb = [&]( int done, int total, const QString& detail )
         { progress.append( QJsonArray{ done, total, detail } ); };
         QTimer closeMessages;
         connect( &closeMessages,
                  &QTimer::timeout,
                  [&]
                  {
                     for ( QWidget* widget : QApplication::topLevelWidgets() )
                     {
                        if ( auto* message = qobject_cast<QMessageBox*>( widget ) )
                        {
                           if ( message->isVisible() )
                           {
                              messages.append( message->text() );
                              message->accept();
                           }
                        }
                     }
                  } );
         closeMessages.start( 10 );
         if ( !f.storage.isEmpty() )
         {
            QString          xml;
            QXmlStreamWriter writer( &xml );
            writer.setAutoFormatting( true );
            QVERIFY( f.saved.toXml( writer ) );
            if ( f.storage == "database" )
            {
               const QString host = qEnvironmentVariable( "US3_PROFILE_DB_HOST" );


               US_Settings::set_defaultDB(
                  { "fixture", "fixture", "profile1329", host, "", "", "fixture@example.invalid", "", "", "fixture-guid" } );
               US_DB2  db;
               QString error;
               QVERIFY2( db.connect( host, "profile1329", "root", "profile1329", error ), qPrintable( error ) );
               if ( name == "database-no-row" )
               {
                  db.rawQuery( "DELETE FROM fixture_profile" );
               }
               else
               {
                  db.rawQuery( "REPLACE INTO fixture_profile VALUES(1,CONVERT(0x" + QString::fromLatin1( xml.toUtf8().toHex() ) +
                               " USING utf8mb4))" );
               }
               QVERIFY2( db.lastError().isEmpty(), qPrintable( db.lastError() ) );
               US_Settings::set_default_data_location( 1 );
            }
            else
            {
               QFile file( US_Settings::dataDir() + "/projects/A0000001.xml" );
               QVERIFY( file.open( QIODevice::WriteOnly ) );
               file.write( xml.toUtf8() );
               file.close();
            }
         }
         qInfo( "INHERIT_BEGIN" );
         window.inherit_protocol( &f.protocol );
         if ( name.endsWith( "-reload" ) )
         {
            window.inherit_protocol( &f.protocol );
         }
         QJsonObject result;
         result.insert( "profile", ProfileSnapshot::value( window.currProf ) );
         result.insert( "messages", messages );
         result.insert( "progress", progress );
         QFile out( qEnvironmentVariable( "US3_PROFILE_SNAPSHOT" ) );
         QVERIFY( out.open( QIODevice::WriteOnly ) );
         out.write( QJsonDocument( result ).toJson() );
      }
};
US3_GUI_TEST_MAIN( US_AnalysisProfileInheritTest )
#include "test_us_analysis_profile_inherit_gui.moc"
