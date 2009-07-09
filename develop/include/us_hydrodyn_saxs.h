#ifndef US_HYDRODYN_SAXS_H
#define US_HYDRODYN_SAXS_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qbuttongroup.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>

#include <qwt_plot.h>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>


using namespace std;

class US_EXTERN US_Hydrodyn_Saxs : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs(bool *, QString, int, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Saxs();

   public:
      bool *saxs_widget;
      int source;
      US_Config *USglobal;

      QLabel *lbl_info;
      QLabel *lbl_filename1;
      QLabel *lbl_filename2;

      QPrinter printer;

      QPushButton *pb_plot_pr;
      QPushButton *pb_plot_saxs;
      QPushButton *pb_help;
      QPushButton *pb_cancel;

      QFont ft;

      QTextEdit *editor;

      QMenuBar *m;

      QwtPlot *plot_pr;
      QwtPlot *plot_saxs;

      QProgressBar *progress_pr;
      QProgressBar *progress_saxs;

   private slots:

      void setupGUI();
      void show_plot_pr();
      void show_plot_saxs();
      void cancel();
      void help();
      void clear_display();
      void print();
      void update_font();
      void save();

   protected slots:

      void closeEvent(QCloseEvent *);
};



#endif

