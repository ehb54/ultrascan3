#ifndef US_BEOWULF_H
#define US_BEOWULF_H

#include <qlabel.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmessagebox.h>
#include <qlistbox.h>
#include <qcheckbox.h>
#include "us_util.h"


class US_Beowulf : public QDialog
{
   Q_OBJECT
   
   public:
      US_Beowulf(QString, int, bool *, QWidget *p=0, const char *name = 0);
      ~US_Beowulf();

      bool *beowulf_widget;
      int experiment;
      QLabel *lbl_banner;
      QLabel *lbl_info1;
      QLabel *lbl_rsh;
      QLabel *lbl_ssh;
      QLabel *lbl_display;
      QLineEdit *le_edit_host;
      QLineEdit *le_display;
      QListBox *lb_hosts;
      QPushButton *pb_add;
      QPushButton *pb_delete;
      QPushButton *pb_save;
      QPushButton *pb_run;
      QPushButton *pb_help;
      QPushButton *pb_quit;
      QPushButton *pb_merge;
      QPushButton *pb_kill;
      QCheckBox *cb_ssh;
      QCheckBox *cb_rsh;
      US_Config * USglobal;
      QString currentHost, Xdisplay, shell, fileName;

   public slots:
   
      void add_host();
      void mergeAll();
      void killAll();
      void edit_host(const QString &);
      void delete_host();
      void show_host(int);
      void quit();
      void help();
      void save();
      void run();
      bool get_localhost();
      void change_ssh();
      void change_rsh();
      void change_display(const QString &);
            
   protected slots:
      void closeEvent(QCloseEvent *);
};

#endif

