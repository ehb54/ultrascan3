#ifndef US_SELECT_CHANNEL_H
#define US_SELECT_CHANNEL_H

// QT defs:

#include "us.h"
#include "us_util.h"
#include "us_extern.h"

#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qevent.h>
#include <qtranslator.h>
#include <qstring.h>

extern int US_EXTERN global_Xpos;
extern int US_EXTERN global_Ypos;

class US_EXTERN US_SelectChannel : public QDialog
{
   Q_OBJECT
   
   public:
      US_SelectChannel(QString *, unsigned int **, QWidget *parent=0, const char *name=0 );
      ~US_SelectChannel();
   
   private:
      US_Config *USglobal;
      QString *selected_channel;
      
      QPushButton *pb_ok;
      QPushButton *pb_cancel;
      QLabel *lbl_blank;
      QListBox *lb_channel;
   
   public slots:
      void select_channel(int);
      
   protected slots:
   
      void closeEvent(QCloseEvent *);
};

#endif

