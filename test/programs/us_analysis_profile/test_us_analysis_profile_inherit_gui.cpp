//! \file test_us_analysis_profile_inherit_gui.cpp
#include "profile_fixture.h"
#include "profile_snapshot.h"
#include "us3_gui_test_main.h"
#include "us_analysis_profile.h"
#include <QCheckBox>
#include <QJsonDocument>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>

namespace
{
//! Values, limits and states of the boxes and fields of the General tab, by object name
QJsonObject boxes( const QWidget& window )
{
   const QRegularExpression row( "^[0-9]+: " );
   QJsonObject              result;
   const QList<QWidget*>    widgets = window.findChildren<QWidget*>();
   for ( QWidget* widget : widgets )
   {
      const QString name = widget->objectName();
      if ( !name.contains( row ) )
      {
         continue;
      }
      QJsonArray values{ widget->isEnabled(), widget->isHidden() };
      if ( auto* box = qobject_cast<QCheckBox*>( widget ) )
      {
         values << box->isChecked();
      }
      if ( auto* spin = qobject_cast<QSpinBox*>( widget ) )
      {
         values << spin->value() << spin->maximum();
      }
      if ( auto* line = qobject_cast<QLineEdit*>( widget ) )
      {
         values << line->text();
      }
      result.insert( name, values );
   }
   // Wavelengths are named by their place in each channel
   int group = 0;
   for ( QScrollArea* area : window.findChildren<QScrollArea*>() )
   {
      if ( !area->objectName().endsWith( "wvl_box" ) )
      {
         continue;
      }
      for ( QAbstractButton* button : area->findChildren<QAbstractButton*>() )
      {
         result.insert( QString( "wavelength %1 %2" ).arg( group ).arg( button->objectName() ),
                        QJsonArray{ button->isEnabled(), button->isChecked() } );
      }
      group++;
   }
   return result;
}

template <class T> T* box( const QWidget& window, const QString& start )
{
   for ( T* widget : window.findChildren<T*>() )
   {
      if ( widget->objectName().startsWith( start ) )
      {
         return widget;
      }
   }
   return nullptr;
}

void click( const QWidget& window, const QString& start )
{
   auto* button = box<QAbstractButton>( window, start );
   if ( button != nullptr && button->isEnabled() )
   {
      button->click();
   }
}

void set( const QWidget& window, const QString& start, int value )
{
   auto* spin = box<QSpinBox>( window, start );
   if ( spin != nullptr && spin->isEnabled() )
   {
      spin->setValue( value );
   }
}
}

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
         for ( const QString& name : ProfileFixture::edit_cases() )
         {
            QTest::newRow( qPrintable( "edit-" + name ) ) << "edit-" + name;
         }
      }
      void record()
      {
         QFETCH( QString, name );
         const bool edit = name.startsWith( "edit-" );
         if ( edit )
         {
            name = name.mid( 5 );
         }
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
         if ( edit )
         {
            auto* tabs = window.findChild<QTabWidget*>();
            result.insert( "loaded boxes", boxes( window ) );
            set( window, "0: Replicate", 1 );
            set( window, "1: Replicate", 1 );
            set( window, "0: RefUseChan", 1 );
            if ( auto* line = box<QLineEdit>( window, "0: loadconc_ratio" ) )
            {
               line->setText( "2.3456789" );
            }
            if ( auto* line = box<QLineEdit>( window, "0: load_volume" ) )
            {
               line->setText( "455.5" );
            }
            click( window, "0:triple_edit:" );
            click( window, "2:triple_run:" );
            click( window, "1: Run" );
            result.insert( "edited boxes", boxes( window ) );
            for ( int tab : { 1, 2 } )
            {
               tabs->setCurrentIndex( tab );
            }
            result.insert( "saved", ProfileSnapshot::value( window.currProf ) );
            tabs->setCurrentIndex( 0 );
            result.insert( "returned", ProfileSnapshot::value( window.currProf ) );
            result.insert( "returned boxes", boxes( window ) );
            click( window, "1: Run" );
            click( window, "1: RunReport" );
            click( window, "0: RunReport" );
            result.insert( "edited again boxes", boxes( window ) );
            window.savePanels();
            result.insert( "saved again", ProfileSnapshot::value( window.currProf ) );

            // Save it, and load it in a new window
            US_AnaProfile    profile = window.currProf;
            QString          xml;
            QXmlStreamWriter writer( &xml );
            writer.setAutoFormatting( true );
            QVERIFY( profile.toXml( writer ) );
            QFile file( US_Settings::dataDir() + "/projects/A0000001.xml" );
            QVERIFY( file.open( QIODevice::WriteOnly ) );
            file.write( xml.toUtf8() );
            file.close();
            US_AnalysisProfileGui second;
            second.currProf           = f.initial;
            second.abde_mode_aprofile = f.abde;
            second.inherit_protocol( &f.protocol );
            result.insert( "reloaded", ProfileSnapshot::value( second.currProf ) );
            second.initPanels();
            result.insert( "shown", ProfileSnapshot::value( second.currProf ) );
            result.insert( "shown boxes", boxes( second ) );
         }
         else
         {
            result.insert( "profile", ProfileSnapshot::value( window.currProf ) );
         }
         result.insert( "messages", messages );
         result.insert( "progress", progress );
         QFile out( qEnvironmentVariable( "US3_PROFILE_SNAPSHOT" ) );
         QVERIFY( out.open( QIODevice::WriteOnly ) );
         out.write( QJsonDocument( result ).toJson() );
      }
};
US3_GUI_TEST_MAIN( US_AnalysisProfileInheritTest )
#include "test_us_analysis_profile_inherit_gui.moc"
