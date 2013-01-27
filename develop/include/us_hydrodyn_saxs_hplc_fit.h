#ifndef US_HYDRODYN_SAXS_HPLC_FIT_H
#define US_HYDRODYN_SAXS_HPLC_FIT_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>
#include <qtable.h>

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_saxs_util.h"

using namespace std;


class ga_individual
{
 public:
   vector < double > v;
   double            fitness;
   bool operator < (const ga_individual & objIn) const
   {
      return fitness < objIn.fitness;
   }

   bool operator == (const ga_individual & objIn) const
   {
      return v == objIn.v;
   }
   bool operator != (const ga_individual & objIn) const
   {
      return v != objIn.v;
   }
};

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Fit : public QDialog
{
   Q_OBJECT

      friend class US_Hydrodyn_Saxs_Hplc;

   public:
      US_Hydrodyn_Saxs_Hplc_Fit(
                                US_Hydrodyn_Saxs_Hplc *hplc_win,
                                QWidget *p = 0, 
                                const char *name = 0
                                );
      ~US_Hydrodyn_Saxs_Hplc_Fit();

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
      
      QLabel        *lbl_fix_curves;
      // QLineEdit     *le_fix_curves;
#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      vector < QCheckBox * > cb_fix_curves;
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

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


#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      vector < double > gsm_t;
      vector < double > gsm_y;
      vector < double > gsm_yp;

      vector < vector < double > > gaussians_undo;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
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

   private slots:

      void update_enables();

      void restore();
      void undo();

      void lm();
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

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

namespace HFIT 
{
   extern vector < double       > init_params;    // variable param initial values
   extern vector < double       > base_params;    // variable param base values

   extern vector < double       > fixed_params;   // the fixed params
   extern vector < unsigned int > param_pos;      // index into fixed params or variable params
   extern vector < bool         > param_fixed;    
   extern vector < double       > param_min;      // minimum values for variable params
   extern vector < double       > param_max;      // maximum values for variable params

   double compute_gaussian_f( double t, const double *par );

   void printvector( QString qs, vector < double > x );
   void printvector( QString qs, vector < unsigned int > x );
   void printvector( QString qs, vector < bool > x );
   void list_params();
};

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

#endif
