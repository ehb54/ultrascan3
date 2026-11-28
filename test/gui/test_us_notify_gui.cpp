//! \file test_us_notify_gui.cpp
#include "us3_gui_test_main.h"
#include "us_notify.h"

#include <QMessageBox>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTimer>

namespace
{
class RecordingNotifier : public US_Notifier
{
   public:
      void notify( Kind kind, QWidget* parent, const QString& title, const QString& text ) override
      {
         kinds << kind;
         parents << parent;
         messages << title + ": " + text;
      }

      QList<Kind>     kinds;
      QList<QWidget*> parents;
      QStringList     messages;
};
} // namespace

class US_NotifyGuiTest : public QObject
{
      Q_OBJECT

   private slots:
      void cleanup()
      {
         US_Notify::setNotifierForTests( {} );
      }

      void testOverrideReceivesSeverityAndParent()
      {
         QWidget                           parent;
         QSharedPointer<RecordingNotifier> notifier( new RecordingNotifier );
         US_Notify::setNotifierForTests( notifier );
         US_Notify::warning( &parent, "Connection Problem", "Could not connect" );
         US_Notify::critical( nullptr, "Error", "Failed" );

         QCOMPARE( notifier->messages, QStringList( { "Connection Problem: Could not connect", "Error: Failed" } ) );
         QCOMPARE( notifier->kinds, QList<US_Notifier::Kind>( { US_Notifier::Kind::Warning, US_Notifier::Kind::Critical } ) );
         QCOMPARE( notifier->parents, QList<QWidget*>( { &parent, nullptr } ) );
      }

      void desktopShowsAMessageBox()
      {
         QString text;
         QTimer  answer;
         answer.setSingleShot( true );
         connect( &answer,
                  &QTimer::timeout,
                  this,
                  [&text]()
                  {
                     if ( auto* box = qobject_cast<QMessageBox*>( QApplication::activeModalWidget() ) )
                     {
                        text = box->text();
                        box->accept();
                     }
                  } );
         answer.start( 0 );

         US_Notify::information( nullptr, "Saved", "The buffer was saved." );
         QCOMPARE( text, QString( "The buffer was saved." ) );
      }

      void headlessPropertyRoutesMessagesToStandardError()
      {
         const char* childKey = "US3_NOTIFY_TEST_CHILD";
         if ( qEnvironmentVariableIsSet( childKey ) )
         {
            qApp->setProperty( US_Notify::HeadlessProperty, true );
            US_Notify::information( nullptr, "Saved", "Saved the buffer" );
            US_Notify::warning( nullptr, "Connection Problem", "Could not\n  connect" );
            US_Notify::critical( nullptr, "Error", "Failed" );
            return;
         }

         QProcess            child;
         QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
         env.insert( childKey, "1" );
         child.setProcessEnvironment( env );
         child.start( QCoreApplication::applicationFilePath(), { "headlessPropertyRoutesMessagesToStandardError" } );

         QVERIFY2( child.waitForFinished( 5000 ), qPrintable( child.errorString() ) );
         QCOMPARE( child.exitStatus(), QProcess::NormalExit );
         QCOMPARE( child.exitCode(), 0 );
         const QByteArray output = child.readAllStandardError();
         QVERIFY2( output.contains( "Note: Saved: Saved the buffer" ), output.constData() );
         QVERIFY2( output.contains( "Warning: Connection Problem: Could not connect" ), output.constData() );
         QVERIFY2( output.contains( "Error: Error: Failed" ), output.constData() );
      }
};

US3_GUI_TEST_MAIN( US_NotifyGuiTest )
#include "test_us_notify_gui.moc"
