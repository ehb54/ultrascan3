#ifndef US_HYDRODYN_BD_OPTIONS_H
#define US_HYDRODYN_BD_OPTIONS_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qbuttongroup.h>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

struct BD_Options
{
   float threshold;
};

class US_EXTERN US_Hydrodyn_BD_Options : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_BD_Options(BD_Options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_BD_Options();

   public:
      BD_Options *bd_options;
      bool *bd_widget;
      void *us_hydrodyn;
      US_Config *USglobal;

      QLabel *lbl_info;
      QLabel *lbl_bd_threshold;
      
      QwtCounter *cnt_bd_threshold;

      QPushButton *pb_help;
      QPushButton *pb_cancel;
      
   private slots:
      
      void setupGUI();
      void update_bd_threshold(double);
      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
