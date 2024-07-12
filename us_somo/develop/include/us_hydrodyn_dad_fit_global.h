#ifndef US_HYDRODYN_DAD_FIT_GLOBAL_H
#define US_HYDRODYN_DAD_FIT_GLOBAL_H

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
#include <QHBoxLayout>
#include <QCloseEvent>

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_dad.h"
#include "../include/us_saxs_util.h"

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif

using namespace std;


class US_EXTERN US_Hydrodyn_Dad_Fit_Global : public QDialog
{
   Q_OBJECT

      friend class US_Hydrodyn_Dad;

   public:
      US_Hydrodyn_Dad_Fit_Global(
                                US_Hydrodyn_Dad *dad_win,
                                QWidget *p = 0, 
                                const char *name = 0
                                );
      ~US_Hydrodyn_Dad_Fit_Global();

   private:
      US_Hydrodyn_Dad *dad_win;

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

      QCheckBox     *cb_test_mode;
      QLabel        *lbl_test_info;
      QHBoxLayout   *hbl_test;
      vector < QWidget * > test_widgets;
      QPushButton   *pb_test;
      mQwtPlot      * plot_test;
      US_Plot       * usp_plot_test;
   private slots:
      void usp_config_plot_test( const QPoint & );

   private:
      ScrollZoomer  * plot_test_zoomer;
#if QT_VERSION >= 0x040000
      QwtPlotGrid   * grid_test;
      QwtPlotCurve  * test_curve;
#else
      long            test_curve;
#endif

      QProgressBar  *progress;
      QPushButton   *pb_stop;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

      void          setupGUI();

      bool          running;

      bool          setup_run();

      bool          update_dad;

      vector < double > gsm_t;
      vector < double > gsm_y;
      vector < double > gsm_yp;

      vector < vector < double > > gaussians_undo;

      void gsm_setup();
      long global_iter;
      long this_iterations;
      our_matrix *new_our_matrix(int rows, int cols);
      void free_our_matrix(our_matrix *m);

      void set_our_matrix(our_matrix *m, double s);
      void identity_our_matrix(our_matrix *m);
      void print_our_matrix(our_matrix *m);
      our_vector *new_our_vector(int len);
      void free_our_vector(our_vector *v);
      void print_our_vector(our_vector *v);
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
      void update_test_info();

      double        gsm_delta;
      double        gsm_delta2_r;
      double        gsm_f            ( our_vector *v );
      void          gsm_df           ( our_vector *vd, our_vector *v );
      long          min_gsm_5_1      ( our_vector *i, double epsilon, long max_iter );
      long          min_fr_pr_cgd    ( our_vector *i, double epsilon, long max_iter );
      long          min_hessian_bfgs ( our_vector *ip, double epsilon, long max_iter );


      unsigned int  pop_selection    ( unsigned int size );
      bool          ga_fitness       ( ga_individual & individual );
      bool          ga_run           ( double & nrmsd );

      bool          use_errors;

      US_Hydrodyn_Dad::gaussian_types   gaussian_type;
      int                                     gaussian_type_size;

      bool          dist1_active;
      bool          dist2_active;

      unsigned int            common_size;
      unsigned int            per_file_size;
      vector < bool >         is_common; // is common maps the offsets to layout of the regular file specific gaussians
      vector < unsigned int > offset;


      double        dad_ampl_width_min;
      bool          dad_lock_min_retry;
      double        dad_lock_min_retry_mult;
      bool          dad_maxfpk_restart;
      unsigned int  dad_maxfpk_restart_tries;
      double        dad_maxfpk_restart_pct;

   private slots:

      void update_enables();
      void update_common();

      void restore();
      void undo();

      void lm();
      void gsm_sd();
      void gsm_ih();
      void gsm_cg();
      void ga();
      void grid();
      void test_mode();
      void test();

      void stop();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

namespace DFIT_GLOBAL 
{
   extern vector < double       > init_params;    // variable param initial values
   extern vector < double       > base_params;    // variable param base values

   extern vector < double       > fixed_params;   // the fixed params
   extern vector < unsigned int > param_pos;      // index into fixed params or variable params
   extern vector < bool         > param_fixed;    
   extern vector < double       > param_min;      // minimum values for variable params
   extern vector < double       > param_max;      // maximum values for variable params

   extern map < unsigned int, unsigned int > comm_backref; // back reference to variable param position of 1st usage

   extern bool                    use_errors;

   extern vector < double       > errors;
   extern vector < unsigned int > errors_index;

   extern double (*compute_gaussian_f)( double t, const double *par );

   extern unsigned int            per_file_size;
   extern unsigned int            common_size;
   extern vector < bool >         is_common;
   void list_params();
   QString qs_params();
};

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
