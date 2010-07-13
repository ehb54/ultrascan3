#ifndef US_HYDRODYN_ANAFLEX_OPTIONS_H
#define US_HYDRODYN_ANAFLEX_OPTIONS_H

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

struct Anaflex_Options
{
   bool run_anaflex;
   int nfrec;
   bool instprofiles;
   bool run_mode_1;
   bool run_mode_2;
   bool run_mode_3;
   bool run_mode_4;
   bool run_mode_9;
};

class US_EXTERN US_Hydrodyn_Anaflex_Options : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Anaflex_Options(Anaflex_Options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Anaflex_Options();

   public:
      Anaflex_Options *anaflex_options;
      bool *anaflex_widget;
      void *us_hydrodyn;
      US_Config *USglobal;

      QLabel *lbl_info;
      QLabel *lbl_nfrec;

      QCheckBox *cb_run_anaflex;
      QCheckBox *cb_instprofiles;
      QCheckBox *cb_run_mode_1;
      QCheckBox *cb_run_mode_2;
      QCheckBox *cb_run_mode_3;
      QCheckBox *cb_run_mode_4;
      QCheckBox *cb_run_mode_9;

      QLineEdit *le_nfrec;

      QPushButton *pb_help;
      QPushButton *pb_cancel;
      
   private slots:
      
      void setupGUI();

      void set_run_anaflex();
      void set_instprofiles();
      void set_run_mode_1();
      void set_run_mode_2();
      void set_run_mode_3();
      void set_run_mode_4();
      void set_run_mode_9();

      void update_nfrec(const QString &str);

      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
