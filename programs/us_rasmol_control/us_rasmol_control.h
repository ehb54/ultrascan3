#ifndef US_RASMOL_CTRL_H
#define US_RASMOL_CTRL_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_window_message.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_plot.h"

#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< dbg_level-conditioned qDebug()

class US_RasmolControl : public US_Widgets
{
   Q_OBJECT

   public:
      US_RasmolControl();

   private:
      US_WindowMessage*    winmsgs;

      QComboBox*           cb_intname;
      QComboBox*           cb_commcmd;

      QTextEdit*           te_status;

      QLineEdit*           le_sendcmd;

      QPushButton*         pb_listints;
      QPushButton*         pb_close;
      QPushButton*         pb_sendcmd;

      int                  dbg_level;
   private slots:
      void list_interps   ( void );
      void send_command   ( void );
      void choose_command ( const QString& );
};
#endif
