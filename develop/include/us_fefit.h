#ifndef US_FEFIT_H
#define US_FEFIT_H

#include <qwidget.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qframe.h>
#include <qpushbt.h>
#include <qradiobt.h>
#include <iostream>
#include <qmsgbox.h>
#include <fstream>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <qpopmenu.h>
#include <qwt_counter.h>
#include <qlined.h>
#include <qfile.h>
#include <qfiledlg.h>
#include <qprogressbar.h>
#include "us_util.h"
#include "us_hydro.h"

struct fefit_component
{
   float conc;
   float sed;
   float s20w;
   float diff;
   float d20w;
   float sigma;
   float delta;
   float mw;
   float vbar;
   float conc_range;
   float sed_range;
   float diff_range;
   float sigma_range;
   float delta_range;
   float mw_ra_range;
   float vbar_range;
   bool conc_fit;
   bool sed_fit;
   bool diff_fit;
   bool sigma_fit;
   bool delta_fit;
   bool mw_fit;
   bool vbar_fit;
};

struct fefit_run
{
   float meniscus;
   float meniscus_range;
   bool meniscus_fit;
   float baseline;
   float baseline_range;
   bool baseline_fit;
   float slope;
   float slope_range;
   bool slope_fit;
   float stray;
   float stray_range;
   bool stray_fit;
   float duration;
   float delta_t;
   float delta_r;
   float cell_bottom;
   uint rotorspeed;
   unsigned long runs;
   unsigned int pos;
   unsigned int neg;
   unsigned int sim_points;
   float runs_percent;
   float runs_expected;
   float runs_variance;
};

class US_Fefit_FitControl : public QFrame
{
   Q_OBJECT

   public:

      US_Fefit_FitControl(bool *temp_constrained_fit, bool *temp_fitting_widget, 
      bool *temp_autoconverge, QWidget *p = 0, const char *name = 0);
      ~US_Fefit_FitControl();
      US_Config *USglobal;
      
      bool *constrained_fit;
      bool *fitting_widget;
      bool *autoconverge;
      QLabel *lbl_header;
      QLabel *lbl_iteration1;
      QLabel *lbl_iteration2;
      QLabel *lbl_best;
      QLabel *lbl_variance1;
      QLabel *lbl_variance2;
      QLabel *lbl_variance3;
      QLabel *lbl_stddev1;
      QLabel *lbl_stddev2;
      QLabel *lbl_difference1;
      QLabel *lbl_difference2;
      QLabel *lbl_evaluations1;
      QLabel *lbl_evaluations2;
      QLabel *lbl_progress;
      QLabel *lbl_status1;
      QLabel *lbl_blank;
      QLabel *lbl_status2;
      QLabel *lbl_status3;
      QLabel *lbl_status4;
      QProgressBar *pgb_progress;
      QRadioButton *bt_constrained;
      QRadioButton *bt_unconstrained;
      QRadioButton *bt_autoconverge;
      QPushButton *pb_fit;
      QPushButton *pb_pause;
      QPushButton *pb_resume;
      QPushButton *pb_cancel;
      QPushButton *pb_help;

   public slots:

      void cancel();
      void help();
      void update_constrained_button();   
      void update_unconstrained_button();
      void update_autoconverge_button();
      
   protected slots:
      void closeEvent(QCloseEvent *e);
};

#endif

