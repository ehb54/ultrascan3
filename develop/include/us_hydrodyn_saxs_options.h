#ifndef US_HYDRODYN_SAXS_OPTIONS_H
#define US_HYDRODYN_SAXS_OPTIONS_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

class US_EXTERN US_Hydrodyn_SaxsOptions : public QFrame
{
   Q_OBJECT

   public:
      friend class US_Hydrodyn;

      US_Hydrodyn_SaxsOptions( struct saxs_options *, 
                               bool *saxs_options_widget,
                               bool *sas_options_saxs_widget,
                               bool *sas_options_sans_widget,
                               bool *sas_options_curve_widget,
                               bool *sas_options_bead_model_widget,
                               bool *sas_options_hydration_widget,
                               bool *sas_options_guinier_widget,
                               bool *sas_options_xsr_widget,
                               bool *sas_options_misc_widget,
                               bool *sas_options_experimental_widget,
                               void *us_hydrodyn, 
                               QWidget *p = 0,
                               const char *name = 0 );
      ~US_Hydrodyn_SaxsOptions();

   public:
      struct saxs_options *saxs_options;
      bool *saxs_options_widget;
      bool *sas_options_saxs_widget;
      bool *sas_options_sans_widget;
      bool *sas_options_curve_widget;
      bool *sas_options_bead_model_widget;
      bool *sas_options_hydration_widget;
      bool *sas_options_guinier_widget;
      bool *sas_options_xsr_widget;
      bool *sas_options_misc_widget;
      bool *sas_options_experimental_widget;

      void *us_hydrodyn;
      US_Config *USglobal;

      QLabel      *lbl_info;

      QPushButton *pb_sas_options_saxs;
      QPushButton *pb_sas_options_sans;
      QPushButton *pb_sas_options_curve;
      QPushButton *pb_sas_options_bead_model;
      QPushButton *pb_sas_options_hydration;
      QPushButton *pb_sas_options_guinier;
      QPushButton *pb_sas_options_xsr;
      QPushButton *pb_sas_options_misc;
      QPushButton *pb_sas_options_experimental;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

   private slots:
      
      void setupGUI();

      void sas_options_saxs();
      void sas_options_sans();
      void sas_options_curve();
      void sas_options_bead_model();
      void sas_options_hydration();
      void sas_options_guinier();
      void sas_options_xsr();
      void sas_options_misc();
      void sas_options_experimental();

      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
