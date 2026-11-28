//! \file test_us_buffer_gui.cpp
// The New Buffer panel's database read, with a connection supplied through
// the session's provider so no server is involved.

#include "us3_gui_test_main.h"

#include <QSharedPointer>
#include <QStringList>
#include <memory>
#include <utility>

#include "us_buffer.h"
#include "us_buffer_gui.h"
#include "us_db2.h"
#include "us_notify.h"
#include "us_session.h"

namespace
{
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
// A connection whose server accepted the account but whose login refused the
// saved person, as US_DB2::connect() leaves it: open, with the refusal as its
// last error. A query replaces that error, as on a real connection.
class RefusedLogin : public US_DB2
{
   public:
      using US_DB2::query;

      void query( const QStringList& ) override
      {
         error = "the component query was refused";
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
         return US_DB2::NOTPERMITTED;
      }
      QString lastError() override
      {
         return error;
      }

      QString error = "the saved person's login was refused";
};

class HandingProvider : public IUS_DBProvider
{
   public:
      explicit HandingProvider( IUS_DB2* db ) : db( db )
      {
      }

      std::unique_ptr<IUS_DB2> open() override
      {
         return std::unique_ptr<IUS_DB2>( std::exchange( db, nullptr ) );
      }

      IUS_DB2* db;
};
#endif

class RecordingNotifier : public US_Notifier
{
   public:
      void notify( Kind, QWidget*, const QString& title, const QString& text ) override
      {
         messages << title + ": " + text;
      }

      QStringList messages;
};
} // namespace

class US_BufferGuiTest : public QObject
{
      Q_OBJECT

   private slots:
#ifndef NO_DB
      void aRefusedLoginIsReportedWithoutQueryingComponents()
      {
         US_SessionOverride session( QSharedPointer<MemoryStore>( new MemoryStore ) );
         US_Session::instance().setProvider( QSharedPointer<HandingProvider>( new HandingProvider( new RefusedLogin ) ) );
         QSharedPointer<RecordingNotifier> notifier( new RecordingNotifier );
         US_Notify::setNotifier( notifier );

         int             investigator = 1;
         int             from_db      = 1;
         US_Buffer       buffer;
         US_BufferGuiNew panel( &investigator, &from_db, &buffer );
         panel.init_buffer();

         US_Notify::setNotifier( {} );
         US_Session::instance().setProvider( {} );

         QCOMPARE( notifier->messages.size(), 1 );
         QVERIFY2( notifier->messages.first().contains( "the saved person's login was refused" ),
                   qPrintable( notifier->messages.first() ) );
      }
#endif
};

US3_GUI_TEST_MAIN( US_BufferGuiTest )
#include "test_us_buffer_gui.moc"
