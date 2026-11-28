//! \file us_notify.cpp
#include "us_notify.h"

#include <QApplication>
#include <QMessageBox>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include <QVariant>

namespace
{
struct TestOverride
{
      QMutex                      mutex;
      QSharedPointer<US_Notifier> notifier;
};

TestOverride& testOverride()
{
   static TestOverride override;
   return override;
}

QSharedPointer<US_Notifier> notifierForTests()
{
   TestOverride& state = testOverride();
   QMutexLocker  locked( &state.mutex );
   return state.notifier;
}

void send( US_Notifier::Kind kind, QWidget* parent, const QString& title, const QString& text )
{
   const QSharedPointer<US_Notifier> notifier = notifierForTests();
   if ( notifier )
   {
      notifier->notify( kind, parent, title, text );
      return;
   }

   QCoreApplication* app = QCoreApplication::instance();
   if ( qobject_cast<QApplication*>( app ) == nullptr || app->property( US_Notify::HeadlessProperty ).toBool() )
   {
      const char* label =
         kind == US_Notifier::Kind::Information ? "Note" : ( kind == US_Notifier::Kind::Warning ? "Warning" : "Error" );
      QTextStream( stderr ) << label << ": " << title << ": " << text.simplified() << Qt::endl;
      return;
   }

   switch ( kind )
   {
      case US_Notifier::Kind::Information:
         QMessageBox::information( parent, title, text );
         break;
      case US_Notifier::Kind::Warning:
         QMessageBox::warning( parent, title, text );
         break;
      case US_Notifier::Kind::Critical:
         QMessageBox::critical( parent, title, text );
         break;
   }
}
} // namespace

void US_Notify::setNotifierForTests( QSharedPointer<US_Notifier> notifier )
{
   TestOverride& state = testOverride();
   QMutexLocker  locked( &state.mutex );
   state.notifier.swap( notifier );
}

void US_Notify::information( QWidget* parent, const QString& title, const QString& text )
{
   send( US_Notifier::Kind::Information, parent, title, text );
}

void US_Notify::warning( QWidget* parent, const QString& title, const QString& text )
{
   send( US_Notifier::Kind::Warning, parent, title, text );
}

void US_Notify::critical( QWidget* parent, const QString& title, const QString& text )
{
   send( US_Notifier::Kind::Critical, parent, title, text );
}
