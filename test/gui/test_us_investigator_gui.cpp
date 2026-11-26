//! \file test_us_investigator_gui.cpp
// The investigator dialog's update, with connections supplied through the
// session's provider so no server is involved.

#include "us3_gui_test_main.h"

#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QSharedPointer>
#include <QStringList>
#include <QTimer>
#include <functional>
#include <memory>

#include "us_db2.h"
#include "us_investigator.h"
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

#ifndef NO_DB
// The procedures each connection was asked to run.
QStringList statusCalls;

// An open connection with nothing in the database.
class EmptyDatabase : public US_DB2
{
   public:
      using US_DB2::query;
      using US_DB2::statusQuery;

      void query( const QStringList& ) override
      {
      }
      int statusQuery( const QStringList& arguments ) override
      {
         statusCalls << arguments.value( 0 );
         return US_DB2::OK;
      }
      bool next() override
      {
         return false;
      }
      bool isConnected() override
      {
         return true;
      }
      int lastErrno() override
      {
         return US_DB2::OK;
      }
      QString lastError() override
      {
         return {};
      }
};

class EmptyDatabaseProvider : public IUS_DBProvider
{
   public:
      std::unique_ptr<IUS_DB2> open() override
      {
         return std::unique_ptr<IUS_DB2>( new EmptyDatabase );
      }
};
#endif

// Answers each modal dialog in turn with the next scripted action.
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

DialogScript::Action pressOk()
{
   return []( QWidget* w ) { qobject_cast<QMessageBox*>( w )->button( QMessageBox::Ok )->click(); };
}

DialogScript::Action cancel()
{
   return []( QWidget* w ) { qobject_cast<QDialog*>( w )->reject(); };
}

DialogScript::Action acknowledge()
{
   return []( QWidget* w ) { qobject_cast<QDialog*>( w )->accept(); };
}
} // namespace

class US_InvestigatorGuiTest : public QObject
{
      Q_OBJECT

   private slots:
#ifndef NO_DB
      // The update decrypts the saved person password with the master
      // password. If another program locked the session after the connection
      // opened and the prompt is cancelled, nothing is sent: an update without
      // it would set the person's password to an empty one.
      void aCancelledPasswordPromptSendsNoUpdate()
      {
         US_SessionOverride session( QSharedPointer<MemoryStore>( new MemoryStore ) );
         US_Settings::set_UltraScanPW( syntheticHash );
         US_Settings::set_defaultDB(
            { "Synthetic", "user", "db", "127.0.0.1:1", "00", "00", "a@example.invalid", "00", "00", "guid" } );
         US_Session::instance().setProvider( QSharedPointer<EmptyDatabaseProvider>( new EmptyDatabaseProvider ) );
         statusCalls.clear();

         QPointer<US_Investigator> dialog = new US_Investigator;
         dialog->override_permit( true );
         for ( QLineEdit* field : dialog->findChildren<QLineEdit*>() )
         {
            if ( field->accessibleName() == "Investigator ID" )
            {
               field->setText( "7" ); // as if picked from the list
            }
            else if ( !field->isReadOnly() && field->accessibleName() != "Search" )
            {
               field->setText( "synthetic" );
            }
         }
         QPushButton* update = nullptr;
         for ( QPushButton* button : dialog->findChildren<QPushButton*>() )
         {
            if ( button->text() == "Update DB" )
            {
               update = button;
            }
         }
         QVERIFY( update != nullptr );

         // Confirm the update, cancel the password prompt; a third dialog
         // would be the update's result.
         DialogScript script( { pressOk(), cancel(), acknowledge() } );
         update->click();

         US_Session::instance().setProvider( {} );
         delete dialog.data();

         QVERIFY2( script.seen.size() == 2, qPrintable( script.seen.join( " | " ) ) );
         QVERIFY2( !statusCalls.contains( "update_person" ), qPrintable( statusCalls.join( ", " ) ) );
      }
#endif
};

US3_GUI_TEST_MAIN( US_InvestigatorGuiTest )
#include "test_us_investigator_gui.moc"
