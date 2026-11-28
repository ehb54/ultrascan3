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

//! \brief The notifier in use, and the calls that replace
//! QMessageBox::information(), warning() and critical() with OK only.
namespace US_Notify
{
//! \brief Application property that routes notifications to standard error.
constexpr char     HeadlessProperty[] = "us_headless";

//! \brief Override delivery for a test; null restores normal routing.
US_GUI_EXTERN void setNotifierForTests( QSharedPointer<US_Notifier> notifier );

US_GUI_EXTERN void information( QWidget* parent, const QString& title, const QString& text );
US_GUI_EXTERN void warning( QWidget* parent, const QString& title, const QString& text );
US_GUI_EXTERN void critical( QWidget* parent, const QString& title, const QString& text );
} // namespace US_Notify

#endif
