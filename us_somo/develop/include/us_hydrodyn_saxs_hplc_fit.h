#ifndef US_HYDRODYN_SAXS_HPLC_FIT_H
#define US_HYDRODYN_SAXS_HPLC_FIT_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>
#include <qtablewidget.h>
//Added by qt3to4:
#include <QCloseEvent>

//standard C and C++ defs:

#include <map>
#include <set>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_saxs_util.h"

using namespace std;

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif


namespace HFIT 
{
   // the state of a fit: the GUI thread's own, or that of a thread fitting in parallel (cur points at it)
   struct state
   {
      vector < double       > init_params;    // variable param initial values
      vector < double       > base_params;    // variable param base values
      vector < double       > last_params;    // variable param initial values

      vector < double       > fixed_params;   // the fixed params
      vector < unsigned int > param_pos;      // index into fixed params or variable params
      vector < bool         > param_fixed;    
      vector < double       > param_min;      // minimum values for variable params
      vector < double       > param_max;      // maximum values for variable params

      map < unsigned int, unsigned int > comm_backref; // back reference to variable param position of 1st usage

      bool                    conc_test = false;
      vector < double >       conc_ratios;
      map < unsigned int,  double >  conc_ratios_map;

      bool                    use_relative_center = false;
      vector < double >       relative_centers;
      map < unsigned int,  double >  relative_center_map;

      vector < double       > errors;
      vector < unsigned int > errors_index;

      bool                    use_errors = false;

      double (*compute_gaussian_f)( double t, const double *par ) = 0;
   };

   // the calling thread's state: the GUI thread's unless the thread set its own
   extern thread_local state * cur;

   // what setting up a fit reads from the fit window, so that a fit can also be set up without it
   struct settings
   {
      int                     gaussian_type;       // US_Hydrodyn_Saxs_Hplc::gaussian_types
      int                     gaussian_type_size;
      bool                    dist1_active;
      bool                    dist2_active;
      bool                    use_errors;

      bool                    conc_test;
      bool                    fix_relative_centers;
      vector < double >       conc_ratios;
      vector < double >       relative_centers;
      vector < bool >         fix_curves;

      bool                    fix_amplitude;
      bool                    fix_center;
      bool                    fix_width;
      bool                    fix_dist1;
      bool                    fix_dist2;
      bool                    comm_dist1;
      bool                    comm_dist2;

      bool                    pct_amplitude;
      bool                    pct_center;
      bool                    pct_width;
      bool                    pct_dist1;
      bool                    pct_dist2;
      double                  pct_amplitude_value;
      double                  pct_center_value;
      double                  pct_width_value;
      double                  pct_dist1_value;
      double                  pct_dist2_value;
      bool                    pct_amplitude_from_init;
      bool                    pct_center_from_init;
      bool                    pct_width_from_init;
      bool                    pct_dist1_from_init;
      bool                    pct_dist2_from_init;

      double                  ampl_min;
      double                  ampl_width_min;
      double                  gauss_max_height;
      double                  dist_max;
      bool                    lock_min_retry;
      double                  lock_min_retry_mult;
      bool                    maxfpk_restart;
      double                  maxfpk_restart_pct;
   };

   void list_params();
};

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Fit : public QDialog
{
   Q_OBJECT

      friend class US_Hydrodyn_Saxs_Hplc;

   public:
      US_Hydrodyn_Saxs_Hplc_Fit(
                                US_Hydrodyn_Saxs_Hplc *hplc_win,
                                bool set_comm_dist = true,
                                QWidget *p = 0, 
                                const char *name = 0
                                );
      ~US_Hydrodyn_Saxs_Hplc_Fit();

      // the parts of an LM fit that do not use the window, on the calling thread's HFIT::cur:
      // set up its parameters from the Gaussians (false: nothing to fit)
      static bool   fit_setup( const HFIT::settings & s,
                               const vector < double > & gaussians,
                               const vector < vector < double > > & gaussians_undo );
      // the curves to fix after a fit, with zero amplitudes or widths (false: none, or not enabled)
      static bool   fit_lock_zeros( const HFIT::settings & s,
                                    const vector < double > & gaussians,
                                    const vector < double > & par,
                                    set < unsigned int > & to_lock );
      // raise the largest free peak for a restart (false: not enabled, or no free peak)
      static bool   fit_max_free_peak_delta( const HFIT::settings & s,
                                             vector < double > & gaussians,
                                             const vector < double > & par );

   private:
      US_Hydrodyn_Saxs_Hplc *hplc_win;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QCheckBox     *cb_fix_center;
      QCheckBox     *cb_pct_center;
      QLineEdit     *le_pct_center;
      QCheckBox     *cb_pct_center_from_init;

      QCheckBox     *cb_fix_width;
      QCheckBox     *cb_pct_width;
      QLineEdit     *le_pct_width;
      QCheckBox     *cb_pct_width_from_init;

      QCheckBox     *cb_fix_amplitude;
      QCheckBox     *cb_pct_amplitude;
      QLineEdit     *le_pct_amplitude;
      QCheckBox     *cb_pct_amplitude_from_init;
      
      QCheckBox     *cb_fix_dist1;
      QCheckBox     *cb_pct_dist1;
      QLineEdit     *le_pct_dist1;
      QCheckBox     *cb_pct_dist1_from_init;

      QCheckBox     *cb_fix_dist2;
      QCheckBox     *cb_pct_dist2;
      QLineEdit     *le_pct_dist2;
      QCheckBox     *cb_pct_dist2_from_init;

      QCheckBox     *cb_comm_dist1;
      QCheckBox     *cb_comm_dist2;
      
      QLabel        *lbl_fix_curves;
      // QLineEdit     *le_fix_curves;
      vector < QCheckBox * > cb_fix_curves;
      QCheckBox     *cb_conc_test;
      vector        < double > conc_ratios;

      QCheckBox     *cb_fix_relative_centers;
      vector        < double > relative_centers;

      QLabel        *lbl_epsilon;
      QLineEdit     *le_epsilon;

      QLabel        *lbl_iterations;
      QLineEdit     *le_iterations;

      QLabel        *lbl_population;
      QLineEdit     *le_population;

      QPushButton   *pb_restore;
      QPushButton   *pb_undo;

      QPushButton   *pb_lm;
      QPushButton   *pb_gsm_sd;
      QPushButton   *pb_gsm_ih;
      QPushButton   *pb_gsm_cg;
      QPushButton   *pb_ga;
      QPushButton   *pb_grid;

      QProgressBar  *progress;
      QPushButton   *pb_stop;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

      void          setupGUI();

      bool          running;

      bool          setup_run();

      bool          update_hplc;

      static vector < double > gsm_t;
      static vector < double > gsm_y;
      static vector < double > gsm_yp;

      vector < vector < double > > gaussians_undo;

      void gsm_setup();
      long global_iter;
      long this_iterations;
      our_matrix *new_our_matrix(int rows, int cols);
      void free_our_matrix(our_matrix *m);

      void set_our_matrix(our_matrix *m, double s);
      void identity_our_matrix(our_matrix *m);
      static void print_our_matrix(our_matrix *m);
      our_vector *new_our_vector(int len);
      void free_our_vector(our_vector *v);
      static void print_our_vector(our_vector *v);
      double l2norm_our_vector(our_vector *v1, our_vector *v2);
      void copy_our_vector(our_vector *v1, our_vector *v2);
      void set_our_vector(our_vector *v1, double s);
      void add_our_vector_vv(our_vector *v1, our_vector *v2);
      void add_our_vector_vs(our_vector *v1, double s);
      void mult_our_vector_vv(our_vector *v1, our_vector *v2);
      void mult_our_vector_vs(our_vector *v1, double s);
      void add_our_vector_vvv(our_vector *vd, our_vector *v1, our_vector *v2);
      void add_our_vector_vvs(our_vector *vd, our_vector *v1, double s);
      void sub_our_vector_vvv(our_vector *vd, our_vector *v1, our_vector *v2);
      void sub_our_vector_vvs(our_vector *vd, our_vector *v1, double s);
      void mult_our_vector_vvv(our_vector *vd, our_vector *v1, our_vector *v2);
      void mult_our_vector_vvs(our_vector *vd, our_vector *v1, double s);
      double dot_our_vector(our_vector *v1, our_vector *v2);
      void mult_our_matrix_vmv(our_vector *vd, our_matrix *m, our_vector *vs);
      void our_vector_test();


      double        gsm_delta;
      double        gsm_delta2_r;
      double        (*gsm_f)         ( our_vector *v );
      static double        gsm_f_GAUSS    ( our_vector *v );
      static double        gsm_f_EMG      ( our_vector *v );
      static double        gsm_f_GMG      ( our_vector *v );
      static double        gsm_f_EMGGMG   ( our_vector *v );

      void          gsm_df           ( our_vector *vd, our_vector *v );
      long          min_gsm_5_1      ( our_vector *i, double epsilon, long max_iter );
      long          min_fr_pr_cgd    ( our_vector *i, double epsilon, long max_iter );
      long          min_hessian_bfgs ( our_vector *ip, double epsilon, long max_iter );


      unsigned int  pop_selection    ( unsigned int size );
      bool          ga_fitness       ( ga_individual & individual );
      bool          ga_run           ( double & nrmsd );

      bool          use_errors;

      void          redo_settings();

      US_Hydrodyn_Saxs_Hplc::gaussian_types   gaussian_type;
      int                                     gaussian_type_size;

      bool          dist1_active;
      bool          dist2_active;

      bool          set_comm_dist;

      bool          lock_zeros( vector < double > & par );
      bool          max_free_peak_delta( vector < double > & par );

      HFIT::settings fit_settings();

      double        hplc_ampl_width_min;
      double        hplc_ampl_min;
      bool          hplc_lock_min_retry;
      double        hplc_lock_min_retry_mult;
      bool          hplc_maxfpk_restart;
      unsigned int  hplc_maxfpk_restart_tries;
      double        hplc_maxfpk_restart_pct;

   private slots:

      void update_enables();
      void update_common();

      void restore();
      void undo();

      void lm( bool max_free_peak_delta_run = false, double prev_rmsd = 0e0 );
      void gsm_sd();
      void gsm_ih();
      void gsm_cg();
      void ga();
      void grid();

      void stop();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

// the fit window's LM fit (lm()) without the window, for fitting curves in parallel threads: the window's
// settings, Gaussians and undo list, the curve's data and the fitting range are its own, and the fit is done on
// the calling thread's HFIT::cur, so that it gives what the window's fit gives for the same curve

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Fit_Lm
{
   public:
      HFIT::settings                  s;               // the window's settings; the fit changes them as the window's
      vector < double >               gaussians;       // the window's hplc_win->gaussians
      vector < vector < double > >    gaussians_undo;
      double                          epsilon;         // the window's epsilon, iterations and population
      int                             iterations;
      int                             population;
      double                          fit_start;       // the fitting range, as the HPLC/KIN window's start and end
      double                          fit_end;
      const vector < double >       * q;               // the curve: its frames (or times), I and sd
      const vector < double >       * I;
      const vector < double >       * e;

      void                            lm( bool max_free_peak_delta_run = false, double prev_rmsd = 0e0 );

   private:
      bool                            setup();
      bool                            update_enables();  // what enables the window's LM button
      void                            update_common();
      void                            undo();
      bool                            lock_zeros( vector < double > & par );
};

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
