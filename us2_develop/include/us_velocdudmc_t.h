#ifndef US_VELOCDUD_T_H
#define US_VELOCDUD_T_H
 

#include "us_selectmodel3.h"

#include "us_util.h"
#include "us_pixmap.h"
#include "us_dtactrl.h"
#include "us_fefit.h"
#include "us_selectmodel.h"
#include "us_montecarlo.h"
#include "us_montecarlostats.h"
#include "us_velocmodelctrl.h"

class US_EXTERN US_VelocDud_T : public Data_Control_W
{
   Q_OBJECT
   
   public:
   
      US_VelocDud_T(QWidget *p = 0, const char *name = 0);
      ~US_VelocDud_T();

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
      unsigned int components;   // number of components
      int model;                  // selected model
      unsigned int parameters;   // number of parameters to be fitted
      bool constrained_fit;      // are we fitting constrained or not?
      float standard_deviation;   // used for monte carlo and load_fit
      US_MonteCarlo *monte_carlo_window;
      struct MonteCarlo mc;
      QString fitName;
      float buoyancyb;
      float buoyancyw;


// Finite Element variables:

      Q_UINT16 sim_points;         // number of radial points/scan in the simulation ('points' is for raw data)
      float *sim_radius;         // radius positions used in the simulation
      float *c_current;            // temporary holding variable for current concentration for conc. dep. calculation
      float *cu, *cv, *cw;         // concentration dependency of diffusion
      float **u, **v, **w;         // concentration dependency of diffusion
      float **a1, **a2, **b;
      float **concentration;      // the concentration matrix containing all simulated scans for the run, 1st dimension: radial points, 2nd dimension: scans, **c;
      double **residuals;         // residuals
      double **y;                  // contains the finite element plot data
      float **left;
      float *right;               // right-hand side of finite element calculation
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
      int comp;
      int divisions;
      int count;
      int exclude;

   public slots:

      void read_init(const QString &);
      void monte_carlo();

   private slots:
// re-implemented Functions:

      void calc_deltas(const unsigned int);
      void calc_residuals(const unsigned int);
      bool calc_alpha();
      void load_fit(const QString &);
      void update_simulation_parameters_constrained(const unsigned int);
      void update_simulation_parameters_unconstrained(const unsigned int);
      void dud();
      void swap();
      void shift_all();
      void dud_init();
      void order_variance();
      void F_init();                     //   call the appropriate finite element simulation
      void assign_F(const unsigned int);                  // assign the concentration[sim_points][scans] to F[points][scans]
      void non_interacting_model();
      void monomer_dimer_model();
      void isomerizing_model();
      void init_finite_element();
      void cleanup_finite();
      void cleanup_dud();
      void calc_D(const int);
      void calc_mw(const int);
      void calc_vbar(const int);
};
#endif

