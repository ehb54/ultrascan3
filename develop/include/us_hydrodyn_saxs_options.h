#ifndef US_HYDRODYN_SAXS_OPTIONS_H
#define US_HYDRODYN_SAXS_OPTIONS_H

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

class US_EXTERN US_Hydrodyn_SaxsOptions : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_SaxsOptions(struct saxs_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_SaxsOptions();

   public:
      struct saxs_options *saxs_options;
      bool *saxs_options_widget;
      void *us_hydrodyn;
      US_Config *USglobal;

      QLabel *lbl_info;
      QLabel *lbl_curve;
      QLabel *lbl_pr;
      QLabel *lbl_wavelength;
      QLabel *lbl_start_angle;
      QLabel *lbl_end_angle;
      QLabel *lbl_water_e_density;
      QLabel *lbl_max_size;
      QLabel *lbl_bin_size;
      
      QwtCounter *cnt_wavelength;
      QwtCounter *cnt_start_angle;
      QwtCounter *cnt_end_angle;
      QwtCounter *cnt_water_e_density;
      QwtCounter *cnt_max_size;
      QwtCounter *cnt_bin_size;

      QCheckBox *cb_hydrate_pdb;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

   private slots:
      
      void setupGUI();
      void update_wavelength(double);
      void update_start_angle(double);
      void update_end_angle(double);
      void update_water_e_density(double);
      void update_max_size(double);
      void update_bin_size(double);
      void set_hydrate_pdb();
      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};



#endif

