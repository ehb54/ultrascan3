#ifndef US_HYDRODYN_BD_H
#define US_HYDRODYN_BD_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
// #include <q3frame.h>
#include <qcheckbox.h>
#include <qfileinfo.h>
#include <qgroupbox.h>
#include <qmenubar.h>
#include <qprinter.h>
#include <qprogressbar.h>
#include <qradiobutton.h>
#include <qtextedit.h>
#include <qtimer.h>
#include <qwt_counter.h>
// Added by qt3to4:
#include <QCloseEvent>

#include "us_hydrodyn_anaflex_options.h"
#include "us_hydrodyn_bd_options.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_util.h"

// standard C and C++ defs:

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <map>
#include <vector>

using namespace std;

class US_EXTERN US_Hydrodyn_BD : public QFrame {
  Q_OBJECT

 public:
  US_Hydrodyn_BD(bool *bd_widget, BD_Options *our_bd_options,
                 Anaflex_Options *our_anaflex_options, void *us_hydrodyn,
                 QWidget *p = 0, const char *name = 0);
  ~US_Hydrodyn_BD();

 public:
  QPushButton *pb_bd_prepare;
  QPushButton *pb_bd_load;
  QPushButton *pb_bd_edit;
  QPushButton *pb_bd_run;
  QPushButton *pb_bd_load_results;

  QPushButton *pb_anaflex_prepare;
  QPushButton *pb_anaflex_load;
  QPushButton *pb_anaflex_edit;
  QPushButton *pb_anaflex_run;
  QPushButton *pb_anaflex_load_results;

 private:
  bool *bd_widget;
  US_Config *USglobal;

  QLabel *lbl_info;
  QLabel *lbl_credits;
  QLabel *lbl_browflex;
  QLabel *lbl_anaflex;

  // QPrinter printer;

  QPushButton *pb_bd_options;
  QPushButton *pb_anaflex_options;
  QPushButton *pb_stop;
  QPushButton *pb_help;
  QPushButton *pb_cancel;

  // QTextEdit *editor;

  QMenuBar *m;

  // QProgressBar *progress;

  BD_Options *our_bd_options;
  Anaflex_Options *our_anaflex_options;

  void *us_hydrodyn;

  bool stopFlag;

  // QFont ft;

 private slots:

  void setupGUI();
  void cancel();
  void help();
  void bd_options();
  void anaflex_options();
  void stop();
  //       void clear_display();
  //       void print();
  //       void update_font();
  //       void save();

  void bd_prepare();
  void bd_load();
  void bd_edit();
  void bd_run();
  void bd_load_results();

  void anaflex_prepare();
  void anaflex_load();
  void anaflex_edit();
  void anaflex_run();
  void anaflex_load_results();

  // void editor_msg( QString color, QString msg );

  // void printError(const QString &);

 protected slots:

  void closeEvent(QCloseEvent *);
};

#endif
