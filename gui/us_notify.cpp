//! \file us_notify.cpp
#include "us_notify.h"

#include <QMessageBox>
#include <QTextStream>

namespace
{
QSharedPointer<US_Notifier>& current()
{
   static QSharedPointer<US_Notifier> notifier;
   return notifier;
}

void send( US_Notifier::Kind kind, QWidget* parent, const QString& title, const QString& text )
{
   const QSharedPointer<US_Notifier> notifier = current();
   if ( notifier )
   {
      notifier->notify( kind, parent, title, text );
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

void US_ConsoleNotifier::notify( Kind kind, QWidget*, const QString& title, const QString& text )
{
   const char* label = kind == Kind::Information ? "Note" : ( kind == Kind::Warning ? "Warning" : "Error" );
   QTextStream( stderr ) << label << ": " << title << ": " << QString( text ).replace( '\n', ' ' ).simplified() << Qt::endl;
}

void US_Notify::setNotifier( QSharedPointer<US_Notifier> notifier )
{
   current() = notifier;
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
