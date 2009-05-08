#ifndef US_VHWENHANCED_H
#define US_VHWENHANCED_H

#include <qprogbar.h>
#include <qprinter.h>

#include "us_cofs.h"
#include "us_sa2d.h"
#include "us_vhwsim.h"
#include "us_editor.h"
#include "us_util.h"
#include "us_pixmap.h"
#include "us_dtactrl.h"
#include "us_extern.h"

struct fe
{
   int components;
   long rpm;
   float delta_r;
   float delta_t;
   float meniscus;
   float bottom;
   unsigned int points;
   vector <float> times;
   vector <float> radius;
   vector <float> sed;
   vector <float> conc;
   vector <float> diff;
   float **absorbance;
};

class  US_EXTERN US_vhwEnhanced : public Data_Control_W
{
   Q_OBJECT
   
   public:
   
      US_vhwEnhanced(QWidget *p = 0, const char *name = 0);
      ~US_vhwEnhanced();
      US_Editor *e;
      Distribution *dis;
      
   private:
   
      QLabel *division_lbl;
      QLabel *tolerance_lbl;
      QPushButton *pb_groups, *pb_cofs, *pb_sa2d;
      QwtCounter *division_counter;
      QwtCounter *tolerance_counter;
      US_Pixmap *pm;
      struct fe fe1, fe2;
      unsigned int exclude, exclude_singlecurve, exclude_rangecurve;
      bool *excluded;
      bool groups_on;
      bool setup_already; // for garbage collection
      unsigned int groups, progress_counter;
      float start_y, max_sed, min_sed, average_s;
      float stop_y;
      float C_zero, *increment, *offset;
      double *radius_x, *radius_y;
      bool group_start;      //true if we are ready to select the first point of a group
      bool fe_completed;
      float *group_sval;
      float *group_percent;
      uint *sval_label;
      unsigned int divisions;
      vector <unsigned int> stop_point;
      double span, interval;
      double **sed_app;
      double *slope;
      double *intercept;
      double *boundary_fractions;
      double tolerance;
      uint *sa_points;
      uint *sa_lines;
      double *lines_y;
      double *lines_x;
      double *sigma;
      double *correl;
      double **left_temp, *right_old, **a1, **a2, **b, **left, *right, **b_orig;
      

   private slots:

      void setup_GUI();
      void non_interacting(struct fe *);
      void init_fe(struct fe *);
      void write_vhw();
      void write_dis();
      void write_res();
      void select_groups();
      void update_divisions(double);
      void update_tolerance(double);
      void getAnalysisPlotMouseReleased(const QMouseEvent &e);
      void getAnalysisPlotMousePressed(const QMouseEvent &e);
      void cleanup();
      void save_model();
      void cofs();
      void sa2d();
      int calc_sed(unsigned int);
      double find_root(double);
      void activate_reset();
// re-implemented Functions:

      int plot_analysis();
      int setups();
      void view();
      void help();
      void save();
      void second_plot();
      void excludeSingleSignal(unsigned int);
      void excludeRangeSignal(unsigned int);
};

#endif

