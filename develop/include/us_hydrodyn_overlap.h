#ifndef US_HYDRODYN_OVERLAP_H
#define US_HYDRODYN_OVERLAP_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qtabwidget.h>
#include <qwt_counter.h>

#include "us_util.h"
#include "us_hydrodyn_overlap_reduction.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace std;

class US_EXTERN US_Hydrodyn_Overlap : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Overlap(struct overlap_reduction *, struct overlap_reduction *,
                                    struct overlap_reduction *, struct overlap_reduction *, double *, bool *, void *,
        QWidget *p = 0, const char *name = 0);
                US_Hydrodyn_Overlap(struct overlap_reduction *, double *, bool *, void *,
      QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Overlap();

   public:
      
      struct overlap_reduction *sidechain_overlap;
      struct overlap_reduction *mainchain_overlap;
      struct overlap_reduction *buried_overlap;
      struct overlap_reduction *grid_overlap;
      bool *overlap_widget, show_grid_only;
      double *overlap_tolerance;
      int tab;
                void *us_hydrodyn;
      
      US_Config *USglobal;
      US_Hydrodyn_OR *sidechain_OR;
      US_Hydrodyn_OR *mainchain_OR;
      US_Hydrodyn_OR *buried_OR;
      US_Hydrodyn_OR *grid_OR;

      QLabel *lbl_info;

      QPushButton *pb_help;
      QPushButton *pb_cancel;
      
      QTabWidget *tw_overlap;
      
      QLabel *lbl_overlap_tolerance;

      QwtCounter *cnt_overlap_tolerance;
      
   private slots:
      
      void setupGUI();
      void update_overlap_tolerance(double);
      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};



#endif

