#ifndef US_HYDRODYN_CSV_VIEWER_H
#define US_HYDRODYN_CSV_VIEWER_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
// #include <q3frame.h>
#include <qcheckbox.h>
#include <qfileinfo.h>
#include <qmenubar.h>
#include <qprinter.h>
#include <qprogressbar.h>
#include <qtablewidget.h>
#include <qtextedit.h>
// Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

// standard C and C++ defs:

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <map>
#include <vector>

#include "us_hydrodyn_comparative.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Csv_Viewer : public QFrame {
  Q_OBJECT

 public:
  US_Hydrodyn_Csv_Viewer(csv csv1, void *us_hydrodyn, QWidget *p = 0,
                         const char *name = 0);
  ~US_Hydrodyn_Csv_Viewer();

 private:
  csv csv1;

  void *us_hydrodyn;

  US_Config *USglobal;

  QLabel *lbl_title;

  QTableWidget *t_csv;

  QPushButton *pb_help;
  QPushButton *pb_cancel;

  bool order_ascending;

  void numeric_sort(int section);

 private slots:

  void setupGUI();

  void sort_column(int);

  void cancel();
  void help();

 protected slots:

  void closeEvent(QCloseEvent *);
};

#endif
