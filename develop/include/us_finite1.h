#ifndef US_FINITE1_H
#define US_FINITE1_H

#include <qprogbar.h>

#include "us_selectmodel3.h"

#include "us_editor.h"
#include "us_util.h"
#include "us_pixmap.h"
#include "us_dtactrl.h"
#include "us_fefit.h"
#include "us_selectmodel.h"
#include "us_montecarlo.h"
#include "us_montecarlostats.h"
#include "us_velocmodelctrl.h"
#include "us_mfem.h"


class US_EXTERN US_Finite_W : public Data_Control_W
{
   Q_OBJECT
   
   public:
   
      US_Finite_W(QWidget *p = 0, const char *name = 0);
      ~US_Finite_W();
      US_Editor *e;
      QPushButton *pb_create_model;
      QPushButton *pb_model_control;
      QPushButton *pb_fit_control;
      QPushButton *pb_load_fit;
      QPushButton *pb_save_fit;
      QLabel *residuals_lbl;
      QwtCounter *residuals_counter;
      US_VelocModelControl *model_control;
      US_Fefit_FitControl *fit_control;
      US_Pixmap *pm;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
    
      vector <struct fefit_component> component_vector;
      fefit_run *run_vector;
      vector <struct rotorInfo> rotor_list;
      vector <struct centerpieceInfo> cp_list;
      vector <float> residuals_v;// vector of residuals for Monte Carlo
      bool fitting_widget;         // keeps track of the fitting widget's state
      bool model_widget;         // keeps track of the model widget's state
      bool monte_carlo_widget;   // keeps track of the Monte Carlo widget's state
      bool suspend_flag;         // flag to allow temporary suspension of fitting process
      bool completed;            // current Simulation event has completed if true
      bool aborted;               // true if fit has been aborted
      bool converged;            // true if the fit exited normally
      bool autoconverge;         // Retry fit wit new range if last iteration > 1
      unsigned int components;   // number of components
      int model;                  // selected model
      unsigned int parameters;   // number of parameters to be fitted
      bool constrained_fit;      // are we fitting constrained or not?
      float standard_deviation;   // used for monte carlo and load_fit
      US_MonteCarlo *monte_carlo_window;
      struct MonteCarlo mc;
      QString fitName;
      vector <float> ri_noise, ti_noise; 

// Finite Element variables:

      Q_UINT16 sim_points;         // number of radial points/scan in the simulation 
                              // ('points' is for raw data)
      struct mfem_initial initCvector; // initial concentration vector
      vector <float> vbar_model;   // vbar array containing all vbar20's for a current 
                                //cell and current channel
                                
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

      float *sim_radius;         // radius positions used in the simulation
      float *c_current;            // temporary holding variable for current concentration for conc. dep. calculation
      float *cu, *cv, *cw;         // concentration dependency of diffusion
      float **u, **v, **w;         // concentration dependency of diffusion
      float **a1, **a2, **b, **b_orig, **left_temp;
      float **concentration;      // the concentration matrix containing all simulated scans for the run, 1st dimension: radial points, 2nd dimension: scans, **c;
      double **residuals;         // residuals
      double **y;                  // contains the finite element plot data
      float **left;
      float *right,*right_old;               // right-hand side of finite element calculation
      float *variance;            // sum of squared residuals / (points-number of parameter)
      float *old_variance;         // sum of squared residuals / (points-number of parameter) for last iteration
//      float last_variance;         // the variance of the best vector for the last iteration
      unsigned int *order;         // contains the index of ordered variances in dud
      unsigned int iteration;
      Q_UINT32 allpoints;         // total number of points for F and delta_F
      unsigned int func_eval;

// Dud variables:

      float **theta;               // Parameter matrix theta columns in first dimension (parameters+1), parameter count in second dimension
      float **delta_theta;
      float **F;                  // the concentration matrix containing all simulated scans for the run, 
                                 // for all thetas, 1st dimension: parameters, 2nd dimension: scans * points 
      float **delta_F;            // difference matrix of above
      float *alpha;
      bool init_simulation;
      bool first_plot;            // keeps track of when to call the residuals from update_residuals
      QString model_id;

// Variables for monomer-dimer model:

      float **partial_c, *s_bar, *D_bar;

   private:
   
      bool model_defined;
      double **line_y;
      double *line_x;
      uint *line;
      int comp;
      int divisions;
      int count;
      int exclude;

   private slots:

      void setup_GUI();
      void write_fef();
      void write_res();
      void reset();

// re-implemented Functions:

      void create_model();
      void setup_model();
      void calc_deltas(const unsigned int);
      void calc_residuals(const unsigned int);
      bool calc_alpha();
      void select_model_control();
      void select_fit_control();
      void start_fit();
      void suspend_fit();
      void resume_fit();
      void cancel_fit();
      void load_fit();
      void save_fit();
      void load_fit(const QString &);
      void update_residuals(double);
      void update_simulation_parameters_constrained(const unsigned int);
      void update_simulation_parameters_unconstrained(const unsigned int);
      int plot_analysis();
      void dud();
      void swap();
      void update_plot(const unsigned int);
      void shift_all();
      void dud_init();
      void order_variance();
      void view();
      void help();
      void save();
      void second_plot();   // substitutes as virtual function for monte carlo
      void F_init();                     //   call the appropriate finite element simulation
      void assign_F(const unsigned int);                  // assign the concentration[sim_points][scans] to F[points][scans]
      int non_interacting_model();
      void monomer_dimer_model();
      void isomerizing_model();
      void init_finite_element();
      void cleanup_finite();
      void cleanup_dud();
      void updateButtons();
      void monte_carlo();

   protected slots:

      void closeEvent(QCloseEvent *);
};
#endif

