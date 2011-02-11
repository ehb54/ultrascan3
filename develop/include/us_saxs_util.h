#ifndef US_SAXS_UTIL_H
#define US_SAXS_UTIL_H

// QT defs:

#include "us_util.h"
#include <math.h>
#include <sys/time.h>
#include <map>
#include <qregexp.h>

using namespace std;

#define SEARCH_DEFAULT     0
#define CONJUGATE_GRADIENT 0
#define STEEPEST_DESCENT   1
#define INVERSE_HESSIAN    2
#define GSM_MAX_SEARCH_TYPE  3

#define MAX_REPS 100
#define MIN_A 1e-25
// #define EPS_DEFAULT 1e-7
#define EPS_DEFAULT 1e-12

typedef struct _our_vector {
   int len;
   double *d;
} our_vector;

typedef struct _our_matrix {
   int rows, cols;
   double *d;
} our_matrix;


class US_Saxs_Scan
{
 public:
   QString header;
   QString filename;
#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
   vector < QString > header_cols;

   vector < double > q;
   vector < double > r;
   vector < double > s;
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
   void clear();
};

class US_EXTERN US_Saxs_Util
{
   public:

      US_Saxs_Util();

      void clear();
      bool read(QString filename, QString tag);
      bool write(QString filename, QString tag);
      bool compat(QString tag1, QString tag2);
      bool avg(QString outtag, vector < QString > tags);
      bool crop(QString outtag, QString fromtag, double low, double high);
      bool interpolate(QString outtag, QString totag, QString fromtag);
      double rmsd(QString tag1, QString tag2);
      bool join(QString outtag, QString tag1, QString tag2, double pt);

      bool guinierplot(QString outtag, QString tag);

      bool subbackground(QString outtag, 
                         QString solutiontag, 
                         QString buffertag, 
                         double alpha);

      bool subcellwave(QString outtag, 
                       QString solutiontag, 
                       QString buffertag, 
                       QString emptytag, 
                       double alpha);

      bool waxsguidedsaxsbuffersub(
                                   double &nrmsd,
                                   QString outtag, 
                                   QString solutiontag, 
                                   QString buffertag, 
                                   QString waxstag, 
                                   double alpha,
                                   double beta,
                                   double dconst,
                                   double low,
                                   double high,
                                   QString &waxscrop,
                                   QString &waxscropinterp,
                                   bool prechecked = false,
                                   bool waxsprecropinterp = false
                                   );

      bool waxsguidedsaxsbuffersubgridsearch(
                                             double &nrmsd,
                                             double &alphamin,
                                             double &betaamin,
                                             double &dconstmin,
                                             QString &log,
                                             QString outtag, 
                                             QString solutiontag, 
                                             QString buffertag, 
                                             QString waxstag, 
                                             double alphalow,
                                             double alphahigh,
                                             double alphainc,
                                             double betalow,
                                             double betahigh,
                                             double betainc,
                                             double dconstlow,
                                             double dconsthigh,
                                             double dconstinc,
                                             double low,
                                             double high
                                             );

      bool waxsguidedsaxsbuffersubgsmsearch(
                                            double &nrmsd,
                                            double &alphamin,
                                            double &betaamin,
                                            double &dconstmin,
                                            QString &log,
                                            QString outtag, 
                                            QString solutiontag, 
                                            QString buffertag, 
                                            QString waxstag, 
                                            int    gsm_type,
                                            double alphalow,
                                            double alphahigh,
                                            double alphainc,
                                            double betalow,
                                            double betahigh,
                                            double betainc,
                                            double dconstlow,
                                            double dconsthigh,
                                            double dconstinc,
                                            double low,
                                            double high
                                            );

      bool setup_wgsbs_gsm_f_df(
                                QString outtag,
                                QString solutiontag, 
                                QString buffertag, 
                                QString waxstag, 
                                double alphalow,
                                double alphahigh,
                                double alphainc,
                                double betalow,
                                double betahigh,
                                double betainc,
                                double dconstlow,
                                double dconsthigh,
                                double dconstinc,
                                double low,
                                double high
                                );

      bool wgsbs_gsm(
                     double &nrmsd,
                     double &alpha,
                     double &beta,
                     double &dconst,
                     int gsm_type,
                     long max_iterations
                     );

      bool gunier_fit(
                      QString outtag,
                      QString intag,
                      unsigned int startpos,
                      unsigned int endpos,
                      double &a,
                      double &b,
                      double &siga,
                      double &sigb,
                      double &chi2
                      );

      int debug;
      QString errormsg;
#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      map < QString, US_Saxs_Scan > wave;
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

   private:

      QString gsm_outtag;
      QString gsm_solutiontag;
      QString gsm_buffertag; 
      QString gsm_waxstag; 
      double gsm_alphalow;
      double gsm_alphahigh;
      double gsm_alphainc;
      double gsm_alpha2_r;
      double gsm_betalow;
      double gsm_betahigh;
      double gsm_betainc;
      double gsm_beta2_r;
      double gsm_dconstlow;
      double gsm_dconsthigh;
      double gsm_dconstinc;
      double gsm_dconst2_r;
      double gsm_low;
      double gsm_high; 
      QString gsm_waxscrop;
      QString gsm_waxscropinterp;

      bool wsgbs_gsm_setup;

      double wgsbs_gsm_f(our_vector *v);
      void wgsbs_gsm_df(our_vector *vd, our_vector *v);

   private:
      // gsm stuff

      long gsm_fitness_calls;

      long global_iter;
      int show_times;
      int evenn;
      int queries;
      int N;
      int N_2;
      int fitness_type;
      int this_iterations;
      long iterations_max;
      int go;
      double min_acceptable;
      int prev_iterations;
      our_vector *vmin;
      our_vector *vmax;
      our_vector *vlen;

      double eps;
      int this_rank;

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
      void clip_our_vector(our_vector *v);
      long min_gsm_5_1(our_vector *i, double epsilon, long max_iter);
      long min_fr_pr_cgd(our_vector *i, double epsilon, long max_iter);
      long min_hessian_bfgs(our_vector *ip, double epsilon, long max_iter);


      // linear fit code, solves  y = a + bx, returing sigmas & chi2
      void linear_fit( 
                      vector < double > x, 
                      vector < double > y, 
                      double &a,
                      double &b,
                      double &siga,
                      double &sigb,
                      double &chi2
                      );


};

#endif
