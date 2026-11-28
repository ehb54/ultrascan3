//! \file test_us_passwd_gui.cpp
// US_Passwd's dialogs, retry loop and caching, driven through the modal
// dialogs it opens. The session uses an in-memory store so the user's shared
// memory is never touched.

#include "us3_gui_test_main.h"

#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <functional>
#include <utility>

#include "us_credential_source.h"
#include "us_db2.h"
#include "us_notify.h"
#include "us_passwd.h"
#include "us_session.h"

namespace
{
// sha1( b"synthetic-master" ), computed independently with Python hashlib.
const QByteArray syntheticHash = QByteArray::fromHex( "536fe61853c3bd2a7c6cee6621fc33b0b41bb7e8" );

class MemoryStore : public US_Session::Store
{
   public:
      QString load() const override
      {
         return value;
      }
      void save( const QString& password ) override
      {
         value = password;
      }

      QString value;
};

// Answers each modal dialog in turn with the next scripted action, and
// records what each dialog showed.
class DialogScript : public QObject
{
   public:
      using Action = std::function<void( QWidget* )>;

      explicit DialogScript( QList<Action> actions ) : actions( actions )
      {
         connect( &timer, &QTimer::timeout, this, [this] { poll(); } );
         timer.start( 10 );
      }

      QStringList seen;
      int         remaining() const
      {
         return actions.size();
      }

   private:
      void poll()
      {
         QWidget* modal = QApplication::activeModalWidget();
         if ( modal == nullptr || modal == last || actions.isEmpty() )
         {
            return;
         }
         last = modal;
         if ( auto* box = qobject_cast<QMessageBox*>( modal ) )
         {
            seen << box->text();
         }
         else
         {
            seen << modal->windowTitle();
         }
         actions.takeFirst()( modal );
      }

      QList<Action>     actions;
      QTimer            timer;
      QPointer<QWidget> last;
};

DialogScript::Action typePassword( const QString& password )
{
   return [password]( QWidget* w )
   {
      auto* dialog = qobject_cast<QInputDialog*>( w );
      QVERIFY( dialog != nullptr );
      dialog->setTextValue( password );
      dialog->accept();
   };
}

DialogScript::Action cancel()
{
   return []( QWidget* w ) { qobject_cast<QDialog*>( w )->reject(); };
}

DialogScript::Action acknowledge()
{
   return []( QWidget* w ) { qobject_cast<QDialog*>( w )->accept(); };
}

DialogScript::Action fillSignoff( const QString& comment, const QString& password )
{
   return [comment, password]( QWidget* w )
   {
      w->findChild<QLineEdit*>( "Comment:" )->setText( comment );
      w->findChild<QLineEdit*>( "Master Password:" )->setText( password );
      w->findChild<QDialogButtonBox*>()->button( QDialogButtonBox::Ok )->click();
   };
}
class RecordingNotifier : public US_Notifier
{
   public:
      void notify( Kind kind, QWidget*, const QString& title, const QString& text ) override
      {
         kinds << kind;
         messages << title + ": " + text;
      }

      QList<Kind> kinds;
      QStringList messages;
};
} // namespace

class US_PasswdGuiTest : public QObject
{
      Q_OBJECT

      QSharedPointer<MemoryStore>         store;
      std::unique_ptr<US_SessionOverride> session;

   private slots:
      void init()
      {
         store = QSharedPointer<MemoryStore>( new MemoryStore );
         session.reset();
         session.reset( new US_SessionOverride( store ) );
         US_Settings::set_UltraScanPW( syntheticHash );
         qApp->setProperty( "us3_headless", QVariant() );
      }

      void cachedPasswordOpensNoDialog()
      {
         store->value = "synthetic-master";
         DialogScript script( { cancel() } );

         QCOMPARE( US_Passwd().getPasswd(), QString( "synthetic-master" ) );
         QCOMPARE( script.remaining(), 1 );
      }

      void unsetPasswordIsReported()
      {
         US_Settings::set_UltraScanPW( {} );
         DialogScript script( { acknowledge() } );

         QVERIFY( US_Passwd().getPasswd().isEmpty() );
         QCOMPARE( script.seen, QStringList( { "The Master Password has not been set." } ) );
      }

      void wrongPasswordIsReportedThenRetried()
      {
         DialogScript script( { typePassword( "wrong" ), acknowledge(), typePassword( "synthetic-master" ) } );

         QCOMPARE( US_Passwd().getPasswd(), QString( "synthetic-master" ) );
         QCOMPARE( script.seen, QStringList( { "Master Password", "The password is incorrect.", "Master Password" } ) );
         QCOMPARE( store->value, QString( "synthetic-master" ) );
      }

      void cancelReturnsNothingAndCachesNothing()
      {
         DialogScript script( { cancel() } );

         QVERIFY( US_Passwd().getPasswd().isEmpty() );
         QVERIFY( store->value.isEmpty() );
      }

      void headlessRunWithoutSourcesOpensNoDialog()
      {
         qApp->setProperty( "us3_headless", true );
         DialogScript script( { cancel() } );

         QVERIFY( US_Passwd().getPasswd().isEmpty() );
         QCOMPARE( script.remaining(), 1 );
      }

      void promptSourcesUnlockWithoutADialog()
      {
         qputenv( "US_PASSWD_GUI_TEST_SECRET", "synthetic-master" );
         US_Session::instance().setPromptSources(
            { QSharedPointer<US_CredentialSource>( new US_EnvironmentCredentialSource( "US_PASSWD_GUI_TEST_SECRET" ) ) } );
         DialogScript script( { cancel() } );

         QCOMPARE( US_Passwd().getPasswd(), QString( "synthetic-master" ) );
         QCOMPARE( script.remaining(), 1 );
         qunsetenv( "US_PASSWD_GUI_TEST_SECRET" );
      }

#ifndef NO_DB
      void aCancelledUnlockFailsWithoutContactingTheServer()
      {
         // A saved profile pointing at a port nothing listens on: if the
         // connection were attempted, the error would be the server's.
         US_Settings::set_defaultDB(
            { "Synthetic", "user", "db", "127.0.0.1:1", "00", "00", "a@example.invalid", "00", "00", "guid" } );
         DialogScript script( { cancel() } );

         US_DB2       db( US_Passwd().getPasswd() );

         QVERIFY( db.lastErrno() != IUS_DB2::OK );
         QVERIFY2( db.lastError().contains( "master password was not entered" ), qPrintable( db.lastError() ) );
         QCOMPARE( script.remaining(), 0 );
         QVERIFY( store->value.isEmpty() );
      }
#endif

      void notifierReplacesTheMessageBox()
      {
         QSharedPointer<RecordingNotifier> notifier( new RecordingNotifier );
         US_Notify::setNotifier( notifier );
         DialogScript script( { acknowledge() } );

         US_Notify::warning( nullptr, "Connection Problem", "Could not connect" );
         US_Notify::critical( nullptr, "Error", "Failed" );
         US_Notify::setNotifier( {} );

         QCOMPARE( notifier->messages, QStringList( { "Connection Problem: Could not connect", "Error: Failed" } ) );
         QCOMPARE( notifier->kinds, QList<US_Notifier::Kind>( { US_Notifier::Kind::Warning, US_Notifier::Kind::Critical } ) );
         QCOMPARE( script.remaining(), 1 );
      }

      void withoutANotifierAMessageBoxIsShown()
      {
         DialogScript script( { acknowledge() } );

         US_Notify::information( nullptr, "Saved", "The buffer was saved." );

         QCOMPARE( script.seen, QStringList( { "The buffer was saved." } ) );
      }

      void signoffReturnsTheFormWithoutThePassword()
      {
         DialogScript                 script( { fillSignoff( "synthetic comment", "synthetic-master" ) } );

         const QMap<QString, QString> form =
            US_Passwd().getPasswd_auditTrail( "GMP Run EDIT Form", "Please fill out the form:", "Synthetic, User" );

         QCOMPARE( form.value( "User:" ), QString( "Synthetic, User" ) );
         QCOMPARE( form.value( "Comment:" ), QString( "synthetic comment" ) );
         QVERIFY( !form.contains( "Master Password:" ) );
         QVERIFY( store->value.isEmpty() );
      }

      void signoffRefusesAWrongPassword()
      {
         DialogScript                 script( { fillSignoff( "synthetic comment", "wrong" ), acknowledge(), cancel() } );

         const QMap<QString, QString> form =
            US_Passwd().getPasswd_auditTrail( "GMP Run EDIT Form", "Please fill out the form:", "Synthetic, User" );

         QVERIFY( form.isEmpty() );
         QCOMPARE( script.seen.size(), 3 );
         QVERIFY2( script.seen.at( 1 ).contains( "the password is incorrect" ), qPrintable( script.seen.at( 1 ) ) );
      }

      void signoffIgnoresTheCachedUnlock()
      {
         store->value = "synthetic-master";
         DialogScript script( { fillSignoff( "synthetic comment", "wrong" ), acknowledge(), cancel() } );

         QVERIFY(
            US_Passwd().getPasswd_auditTrail( "GMP Run EDIT Form", "Please fill out the form:", "Synthetic, User" ).isEmpty() );
         QCOMPARE( store->value, QString( "synthetic-master" ) );
      }
};

US3_GUI_TEST_MAIN( US_PasswdGuiTest )
#include "test_us_passwd_gui.moc"
