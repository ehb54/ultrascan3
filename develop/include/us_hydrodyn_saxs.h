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
      US_Hydrodyn_Saxs(bool *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Saxs();

   public:
      bool *saxs_widget;
      US_Config *USglobal;

      QLabel *lbl_info;
      QLabel *lbl_filename1;
      QLabel *lbl_filename2;

      QPushButton *pb_plot_pr;
      QPushButton *pb_plot_saxs;
      QPushButton *pb_help;
      QPushButton *pb_cancel;

      QTextEdit *editor;

      QwtPlot *pr_plot;
      QwtPlot *saxs_plot;

   private slots:

      void setupGUI();
      void plot_pr();
      void plot_saxs();
      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};



#endif

