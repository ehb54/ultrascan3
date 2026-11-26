//! \file us_notify.h
#ifndef US_NOTIFY_H
#define US_NOTIFY_H

#include <QSharedPointer>
#include <QString>

#include "us_extern.h"

class QWidget;

//! \brief Tells the person running the program about a problem or result.
//! The default shows a message box; headless runs print to the console
//! instead, and tests record what was said.
class US_GUI_EXTERN US_Notifier
{
   public:
      //! \brief How serious the message is, as the message box icon shows.
      enum class Kind
      {
         Information,
         Warning,
         Critical
      };

      virtual ~US_Notifier() = default;

      virtual void notify( Kind kind, QWidget* parent, const QString& title, const QString& text ) = 0;
};

//! \brief Prints messages to standard error, for runs without a GUI.
class US_GUI_EXTERN US_ConsoleNotifier : public US_Notifier
{
   public:
      void notify( Kind kind, QWidget* parent, const QString& title, const QString& text ) override;
};

//! \brief The notifier in use, and the calls that replace
//! QMessageBox::information(), warning() and critical() with OK only.
namespace US_Notify
{
//! \brief Install a notifier at startup, before other threads notify;
//! null restores message boxes.
US_GUI_EXTERN void setNotifier( QSharedPointer<US_Notifier> notifier );

US_GUI_EXTERN void information( QWidget* parent, const QString& title, const QString& text );
US_GUI_EXTERN void warning( QWidget* parent, const QString& title, const QString& text );
US_GUI_EXTERN void critical( QWidget* parent, const QString& title, const QString& text );
} // namespace US_Notify

#endif
