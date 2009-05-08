#ifndef US_EQUILTIME_H
#define US_EQUILTIME_H

#include "us_util.h"
#include "us_hydro.h"
#include "us_editor.h"

#include <qwt_counter.h>
#include <qwt_plot.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN US_EquilTime : public QFrame
{
   Q_OBJECT
   
   public:
      US_EquilTime( QWidget *parent=0, const char *name=0 );
      ~US_EquilTime();
      US_Config *USglobal;

   private:
      int molecule;
      long curve1, *target_curve, *final_curve;
      bool use_sigma, monitor_flag;
      float *sigma, sigma_start, sigma_stop;
      float sed, diff, conc, time_increment, delta_t, delta_r, tolerance;
      unsigned int *rpm, rpm_start, rpm_stop, speed_steps, sim_points;
      double meniscus, bottom, *right;
      double *sim_radius, **targets;
      double **left, *c_current;
      float **a1, **a2, **b, *temp_val;
      QLabel *banner1;
      QLabel *banner2;
      QLabel *banner3;
      QLabel *banner4;
      QLabel *lbl_mw1;
      QLabel *lbl_mw2;
      QLabel *lbl_sed1;
      QLabel *lbl_sed2;
      QLabel *lbl_diff1;
      QLabel *lbl_diff2;
      QwtPlot *data_plot;
      QwtCounter *ct_topradius, *ct_bottomradius, *ct_speedstart, *ct_speedstop;
      QwtCounter *ct_speedsteps, *ct_tolerance, *ct_timesteps, *ct_delta_r, *ct_delta_t;
      QLabel *lbl_topradius, *lbl_bottomradius, *lbl_speedstart, *lbl_speedstop;
      QLabel *lbl_speedsteps, *lbl_tolerance, *lbl_timesteps, *lbl_delta_r, *lbl_delta_t;
      
      QPushButton *pb_simcomp, *pb_help, *pb_quit, *pb_save, *pb_estimate;
      QCheckBox *cb_sphere, *cb_prolate, *cb_oblate, *cb_rod;
      QCheckBox *cb_inner, *cb_center, *cb_outer, *cb_custom;
      QLabel *lbl_speedlist;
      QCheckBox *cb_rpm, *cb_sigma, *cb_monitor;
      QListBox *lb_speeds;
      US_Editor *textwindow;
      struct hydrosim simcomp;
      
   public:

   private slots:
   
      void simulate_times();
      void simulate_component();
      void update_component();
      void quit();
      void help();
      void GUI();
      void setup_GUI();
      void calc_function();
      void init_finite_element();
      void select_inner();
      void select_center();
      void select_outer();
      void select_custom();
      void select_sphere();
      void select_prolate();
      void select_oblate();
      void select_rod();
      void select_rpm();
      void select_monitor();
      void update_topradius(double);
      void update_bottomradius(double);
      void update_speedstart(double);
      void update_speedstop(double);
      void update_speedsteps(double);
      void update_tolerance(double);
      void update_timesteps(double);
      void update_delta_r(double);
      void update_delta_t(double);
      void calc_speeds();
      void save();

   protected slots:
      void closeEvent(QCloseEvent *e);
//      void resizeEvent(QResizeEvent *e);
};

#endif

