#ifndef US_DIRHIST_H
#define US_DIRHIST_H

// QT defs:

#include <qdatetime.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qtablewidget.h>
//Added by qt3to4:
#include <QCloseEvent>
#if QT_VERSION >= 0x040000
#include <QHeaderView>
#endif

#include "us_util.h"

//standard C and C++ defs:

#include <iostream>
#include <map>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;


#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif

class US_EXTERN US_Dirhist : public QDialog {
      Q_OBJECT

   public:
      friend class US_Hydrodyn;

      US_Dirhist(
         QStringList &history, map<QString, QDateTime> &last_access, map<QString, QString> &last_filetype,
         QString &selected, bool &is_ok, QWidget *p = 0, const char *name = 0);
      ~US_Dirhist();

   private:
      US_Config *USglobal;

      QLabel *lbl_info;

      QTableWidget *t_hist;

      QPushButton *pb_del;

      QPushButton *pb_help;
      QPushButton *pb_ok;
      QPushButton *pb_cancel;

      void setupGUI();

      QStringList *history;
      map<QString, QDateTime> *last_access;
      map<QString, QString> *last_filetype;
      QString *selected;
      bool *is_ok;

      bool order_ascending;

   private slots:

      void update_enables();
      void t_sort_column(int col);
      void t_doubleClicked(int row, int col, int button, const QPoint &mousePos);
      void t_doubleClicked(int row, int col);

      void del();
      void ok();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif

#endif
