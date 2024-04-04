#ifndef US_SAXS_UTIL_H
#define US_SAXS_UTIL_H

#if defined( USE_MPI )
#  include <mpi.h>
   extern int npes;
   extern int myrank;
#endif

// QT defs:
#include <QtCore>
#include <QtWidgets>
#include <math.h>
#include <time.h>
#include <map>
#include <set>
#include <qregexp.h>

#include "us_util.h"
#include "us_math.h"
#include "us_tar.h"
#include "us_gzip.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_asa.h"
#include "us_saxs_gp.h"
#include "us_saxs_util_nsa.h"
#include "us_json.h"
#include "us_timer.h"
#include "us_file_util.h"
#include "us_vector.h"

#include "us_hydrodyn_misc.h"
#include "us_hydrodyn_results.h"
//#include "us_hydrodyn_asa.h"
#include "us_hydrodyn_overlap_reduction.h"
#include "us_hydrodyn_hydro.h"
#include "us_hydrodyn_grid.h"
#include "us_hydrodyn_batch.h"
#include "us_hydrodyn_save.h"
#include "us_hydrodyn_bead_output.h"
#include "us_hydrodyn_pdb_parsing.h"
#include "us_hydrodyn_pdb_visualization.h"
#include "us_hydrodyn_anaflex_options.h"
#include "us_hydrodyn_bd_options.h"
#include "us_hydrodyn_dmd_options.h"
//#include "us_hydrodyn_supc_hydro.h"


#if defined( WIN32 ) && !defined( MINGW )
typedef _int16 int16_t;
typedef _int32 int32_t;
typedef unsigned _int16 uint16_t;
typedef unsigned _int32 uint32_t;
#else
#include <stdint.h>
#endif

#if defined( USE_MPI )
#  include "us_pm_mpi.h"
#  define MPI_JSON_SNIP_START "_____json_start_____\n"
#  define MPI_JSON_SNIP_END   "_____json_end_____\n"
#endif

#include <complex>

#if defined(WIN32)
#   include <dos.h>
#   include <stdlib.h>
#   include <float.h>
# if __GNUC__ == 5
#   include <time.h>
#   define us_isnan std::isnan
#   define us_isinf std::isinf
    extern int putenv(char*);
# else
#   define us_isnan _isnan
#   define us_isinf(x) (!_finite(x))
#endif
#   undef SHOW_TIMING
#   define drand48() ((double)rand()/RAND_MAX)
#   define srand48(x) srand(x)
#else
#   define us_isnan std::isnan
#   define us_isinf std::isinf
#endif

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif

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
   vector < QString > header_cols;

   vector < double > q;
   vector < double > r;
   vector < double > s;
   void clear( );
};

class US_EXTERN US_Saxs_Util
{
   public:

      friend class US_Hydrodyn_Pdb_Tool;
      friend class US_Hydrodyn_Saxs;
      friend class US_Hydrodyn;
      friend class US_Hydrodyn_Cluster_Dmd;

      US_Saxs_Util();
      ~US_Saxs_Util();

      void   clear      ( );
      bool   read       ( QString filename, QString tag );
      bool   write      ( QString filename, QString tag );
      bool   compat     ( QString tag1,     QString tag2 );
      void   scale      ( QString outtag,   QString intag,   double multiplier );
      bool   scalesum   ( QString outtag,   vector < QString > tags,   vector < double >  multipliers );
      bool   avg        ( QString outtag,   vector < QString > tags );
      bool   crop       ( QString outtag,   QString fromtag, double low,   double high );
      bool   interpolate( QString outtag,   QString totag,   QString fromtag );
      double rmsd       ( QString tag1,     QString tag2 );
      bool   join       ( QString outtag,   QString tag1,    QString tag2,  double pt );
      bool   normalize  ( QString outtag,   QString tag );
      bool   merge      ( QString outtag,   QString tag1,    double  w1,    QString tag2,  double w2 );

      bool  interpolate(
                        vector < double > & results,
                        vector < double > & to_r, 
                        vector < double > & from_r, 
                        vector < double > & from_pr );

      bool subbackground(
                         QString outtag, 
                         QString solutiontag, 
                         QString buffertag, 
                         double alpha
                         );

      bool subcellwave(
                       QString outtag, 
                       QString solutiontag, 
                       QString buffertag, 
                       QString emptytag, 
                       double alpha
                       );

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
      bool compute_wgsbs(
                         QString outfile,
                         QString solutiontag,
                         QString buffertag,
                         QString waxstag,
                         int grids,
                         int gsm_type,
                         long max_iterations,
                         double gsmpercent,
                         double alphalow,
                         double alphahigh,
                         double alphaincg,
                         double alphaincgsm,
                         double betalow,
                         double betahigh,
                         double betaincg,
                         double betaincgsm,
                         double dconstlow,
                         double dconsthigh,
                         double dconstincg,
                         double dconstincgsm,
                         double low,
                         double high,
                         double &nrmsd,
                         double &alphamin,
                         double &betamin,
                         double &dconstmin,
                         QString &cliperrors
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

      bool guinier_plot(QString outtag, QString tag);

      bool guinier_fit( 
                       QString &log,
                       QString tag,  // tag needs to be preprocessed with guinierplot
                       unsigned int startpos,
                       unsigned int endpos,
                       double &a,
                       double &b,
                       double &siga,
                       double &sigb,
                       double &chi2,
                       double &Rg,
                       double &I0,
                       double &smaX,
                       double &smin,
                       double &sRgmin,
                       double &sRgmax,
                       bool   compute_Rc = false,
                       bool   compute_Rt = false
                       );

      bool guinier_remove_points(
                                 QString          outtag,
                                 QString          tag,
                                 map < double, double > & removed,
                                 unsigned int   & pts_removed,
                                 unsigned int   & startpos,
                                 unsigned int   & endpos,
                                 double           a,
                                 double           b,
                                 double           sd_limit 
                                 );

      bool guinier_fit_with_removal( 
                       QString &log,
                       QString tag,  // tag needs to be preprocessed with guinierplot
                       unsigned int startpos,
                       unsigned int endpos,
                       double &a,
                       double &b,
                       double &siga,
                       double &sigb,
                       double &chi2,
                       double &Rg,
                       double &I0,
                       double &smax,
                       double &smin,
                       double &sRgmin,
                       double &sRgmax,
                       double sd_limit,
                       map < double, double > & removed,
                       unsigned int   & pts_removed,
                       bool   compute_Rc = false,
                       bool   compute_Rt = false
                       );

      bool guinier_fit2( // find best guinier fit
                       QString &log,
                       QString tag,             // tag needs to be preprocessed with guinierplot
                       unsigned int pointsmin,  // the minimum # of points allowed typically 10
                       unsigned int pointsmax,  // the maximum # of points allowed typically 100
                       double sRgmaxlimit,      // maximum sRg allowed
                       double pointweightpower, // the exponent ofnumber of points when computing the best one
                       //                          i.e. fitness = chi2 / ( number_of_points ** pointweightpower )
                       double maxq,             // the maximum q value allowed
                       double &a,
                       double &b,
                       double &siga,
                       double &sigb,
                       double &chi2,
                       double &Rg,
                       double &I0,
                       double &smaX,
                       double &smin,
                       double &sRgmin,
                       double &sRgmax,
                       unsigned int &beststart,
                       unsigned int &bestend,
                       bool   compute_Rc = false,
                       bool   compute_Rt = false
                       );

      int debug;
      QString errormsg;
      QString noticemsg;
      QString accumulated_msgs;
      QString noticemsg_udp;
      
      map < QString, US_Saxs_Scan > wave;

      // project utilities

      bool read_project( QString subdir = "" );
      bool merge_projects(
                          QString            outfile, 
                          double             reference_mw_multiplier, 
                          vector < QString > projects,
                          bool               run_gnom = false
                          );
      bool build_wiki();

      // build averages and wiki page for subdirectory 1d, create pngsplits # of zoom (none if 1 or less)
      bool project_1d(QString wikitag, unsigned int pngsplits);

      // linear fit code, solves  y = a + bx, returing sigmas & chi2
      static void linear_fit( 
                             vector < double > x, 
                             vector < double > y, 
                             double &a,
                             double &b,
                             double &siga,
                             double &sigb,
                             double &chi2
                              );

      static void linear_fit( 
                             vector < double > x, 
                             vector < double > y, 
                             vector < double > e, 
                             double &a,
                             double &b,
                             double &siga,
                             double &sigb,
                             double &chi2
                              );

      static bool pearsonpmcc(
                              const vector < double > & x, 
                              const vector < double > & y,
                              double & r
                              );

      static bool pearsonpmcc(
                              const vector < double > & x, 
                              const vector < double > & y, 
                              double & r,
                              QString & error_msg
                              );

      // linear fit code, solves  y = kx, returing chi2
      bool scaling_fit( 
                       vector < double > x, 
                       vector < double > y, 
                       double &a,
                       double &chi2
                       );

      bool scaling_fit( 
                       vector < double > x, 
                       vector < double > y, 
                       vector < double > sd, 
                       double &a,
                       double &chi2
                       );

      static bool sscaling_fit( 
                               vector < double > x, 
                               vector < double > y, 
                               vector < double > sd, 
                               double &a,
                               double &chi2
                               );
      void nnls_fit( 
                    vector < double > x, 
                    vector < double > y, 
                    double &k,
                    double &nnls_rmsd
                    );

      bool nnls_fit( 
                    vector < vector < double > > A, 
                    vector < double >            y, 
                    vector < double >            &x,
                    double                       &nnls_rmsd
                    );

      static double calc_rmsd( vector < double > v1,  vector < double > v2 );
      static double calc_nrmsd( vector < double > v1,  vector < double > v2 ); // normalized rmsd is rmsd divided by the range (of v2) time 100 (for percent)

      static bool calc_chisq(
                             const vector < double > & x,
                             const vector < double > & y,
                             const vector < double > & sds,
                             int                     parameters,
                             double                  & chisq,
                             double                  & nchi,
                             QString                 & errors
                             );

      static bool calc_chisq1( vector < double > bins,
                               vector < double > ebins,
                               unsigned int      &df,
                               double            &chisq,
                               double            &prob );
      static bool calc_chisq2( vector < double > bins1,
                               vector < double > bins2,
                               unsigned int      &df,
                               double            &chisq,
                               double            &prob );
      static double calc_gammaln( double val );
      static bool calc_chisq_prob( double a, double x, double &prob );

      static bool calc_mychi2( vector < double > x,
                               vector < double > y,
                               vector < double > sds,
                               double            &chi2 );

      static bool calc_myrmsd( vector < double > x,
                               vector < double > y,
                               double            &rmsd );

      static bool calc_chisqshannon( 
                                    vector < double > &q,
                                    vector < double > &Iexp,
                                    vector < double > &sexp,
                                    vector < double > &Icalc,
                                    double            dmax,
                                    unsigned int      k,
                                    QString           &errors,
                                    double            &ns,
                                    double            &chi2shannon
                                    );
                                 

      bool setup_saxs_maps( QString atom_file, QString hybrid_file, QString saxs_file );

      atom current_atom;
      hybridization current_hybrid;
      saxs current_saxs;

      bool set_excluded_volume( 
                               PDB_atom                 &this_atom, 
                               double                   &vol, 
                               double                   &scaled_vol, 
                               saxs_options             &our_saxs_options, 
                               map < QString, QString > &residue_atom_hybrid_map
                               );

      bool set_excluded_volume( 
                               PDB_atom                 &this_atom, 
                               double                   &vol, 
                               double                   &scaled_vol, 
                               saxs_options             &our_saxs_options, 
                               map < QString, QString > &residue_atom_hybrid_map,
                               unsigned int             &total_electrons,
                               unsigned int             &total_electrons_noh
                               );


      bool set_excluded_volume( 
                               PDB_atom                 &this_atom, 
                               double                   &vol, 
                               double                   &scaled_vol, 
                               saxs_options             &our_saxs_options, 
                               map < QString, QString > &residue_atom_hybrid_map,
                               unsigned int             &total_electrons,
                               unsigned int             &total_electrons_noh,
                               double                   &scattering_intensity
                               );

      vector < atom >                atom_list;
      vector < hybridization >       hybrid_list;
      vector < saxs >                saxs_list;
      map < QString, saxs >          saxs_map;
      map < QString, hybridization > hybrid_map;
      map < QString, atom >          atom_map;
      map < QString, vector <int> >  multi_residue_map;
      map < QString, QString >       residue_atom_hybrid_map;
      map < QString, double >        atom_mw;
      map < QString, double >        atom_vdw;

      map < QString, vector < double > > vcm;

      bool                           load_mw_json( QString filename );
      bool                           load_vdw_json( QString filename );
      bool                           load_vcm_json( QString filename );

      bool iqq_sphere( 
                      QString tag,          // creates iqq for a sphere based on Rayleigh (1911)
                      double  radius,
                      double  delta_rho,
                      double  min_q,
                      double  max_q,
                      double  delta_q
                      );

      bool iqq_sphere_grid( 
                           QString outtag,    // build sphere I(q) on intag's grid
                           QString intag,     
                           double  radius,
                           double  delta_rho
                           );
      
      bool iqq_sphere_fit( 
                          QString outfile,
                          QString infile,
                          double  min_radius,
                          double  max_radius,
                          double  delta_radius,
                          double  min_delta_rho,
                          double  max_delta_rho,
                          double  delta_delta_rho,
                          double  min_q,
                          double  max_q,
                          bool    do_normalize = true
                          );

      bool iqq_sphere_fit( 
                          QString                       tag,
                          vector < double >             q,
                          vector < double >             I,
                          vector < double >             I_errors,
                          double                        min_radius,
                          double                        max_radius,
                          double                        delta_radius,
                          double                        min_delta_rho,
                          double                        max_delta_rho,
                          double                        delta_delta_rho,
                          double                        min_q,
                          double                        max_q,
                          vector < double >             &by_radius,
                          vector < double >             &by_delta_rho,
                          vector < double >             &val_radius,
                          vector < double >             &val_delta_rho,
                          map < double, unsigned int >  &index_radius,
                          map < double, unsigned int >  &index_delta_rho,
                          double                        &best_fit_radius,
                          double                        &best_fit_delta_rho,
                          double                        &average_fit_radius,
                          double                        &average_fit_delta_rho,
                          double                        &start_q,
                          double                        &end_q,
                          double                        &delta_q,
                          unsigned int                  &points_q,
                          QString                       &best_tag,
                          QString                       &nnls_tag,
                          bool                          do_normalize = true
                          );

      bool compute_rayleigh_structure_factors( 
                                              double            radius,
                                              double            delta_rho,
                                              vector < double > &q,
                                              vector < double > &F
                                              );

      
      point minus( point p1, point p2 );       // returns p1 - p2
      point plus ( point p1, point p2 );       // returns p1 + p2
      point cross( point p1, point p2 );       // returns p1 x p2
      float dot( point p1, point p2);          // p1 dot p2
      point normal( point p1 );                // normalized point
      point plane( PDB_atom *a1, PDB_atom *a2, PDB_atom *a3 );  //( a3 - a2 ) x ( a1 - a2 )
      point plane( point p1, point p2, point p3 );  //( a3 - a2 ) x ( a1 - a2 )
      point average( vector < point > *v );    // returns an average vector
      float dist( point p1, point p2);         // sqrt( (p1 - p2) dot (p1 - p2) )
      point scale( point p, float m);         // sqrt( (p1 - p2) dot (p1 - p2) )

      // computes [1,x,x^2]^-1*y
      bool get_quadratic_interpolation_coefficients( 
                                                    vector < double > x, 
                                                    vector < double > y, 
                                                    vector < double > &c 
                                                    );

      // takes x1 on grid q1 and interpolates to x2 on use_q indexed points of q, putting the result in r (on q2)
      bool linear_interpolate_iq_curve( 
                                       vector < double >       &q,
                                       vector < unsigned int > &use_q,
                                       vector < double >       &x1, 
                                       vector < double >       &x2,
                                       vector < double >       &r
                                       );

      bool quadratic_interpolate_iq_curve( 
                                          vector < double >       &q,
                                          vector < unsigned int > &use_q,
                                          vector < double >       &x1, 
                                          vector < double >       x2,
                                          vector < double >       &r
                                          );

      bool cubic_spline_interpolate_iq_curve( 
                                             vector < double >       &q,
                                             vector < unsigned int > &use_q,
                                             vector < double >       &x1, 
                                             vector < double >       x2,
                                             vector < double >       &r
                                             );
      
      bool quadratic_interpolate( 
                                 vector < double >       &x1,
                                 vector < double >       &y1, 
                                 vector < double >       &x2,
                                 vector < double >       &y2
                                 );

      bool linear_interpolate( 
                              vector < double >       &x1,
                              vector < double >       &y1, 
                              vector < double >       &x2,
                              vector < double >       &y2
                              );


      // computes 2nd derivatives on y=f(x) and distributes n points proportionately to abs(f'') and returns in r
      bool create_adaptive_grid( 
                                vector < double >       &x,
                                vector < double >       &y,
                                unsigned int            n,
                                vector < unsigned int>  &r
                                );

      bool apply_natural_spline( vector < double > &xa, 
                                 vector < double > &ya,
                                 vector < double > &y2a,
                                 double            x,
                                 double            &y );

      static bool static_apply_natural_spline( vector < double > &xa, 
                                               vector < double > &ya,
                                               vector < double > &y2a,
                                               double            x,
                                               double            &y,
                                               QString           &errormsg );

      static void natural_spline( vector < double > &x, 
                                  vector < double > &y,
                                  vector < double > &y2 );


      static void natural_spline( vector < double > &x, 
                                  vector < double > &y,
                                  vector < double > &y2,
                                  double            yp1,
                                  double            ypn);

      // takes from_data with from_errors and places on to_grid resulting in to_data & to_errors
      // for systems without errors, these are assumed to be computed iq curves so a natural spline interpolation
      // is performed
      // for systems with errors, these are assumed to be experimental data it is only acceptible going
      // to a coarser grid, otherwise an error will result with error_msg set

      static bool is_nonzero_vector( const vector < double > &v ); // returns true if all non-zero
      static bool is_zero_vector( const vector < double > &v ); // returns true if all zero

      bool interpolate_iqq_by_case( vector < double > from_grid,
                                    vector < double > from_data,
                                    vector < double > from_errors,
                                    vector < double > to_grid,
                                    vector < double > &to_data,
                                    vector < double > &to_errors );

      bool crop( vector < double > from_grid,
                 vector < double > from_data,
                 vector < double > from_errors,
                 double            min_q,
                 double            max_q,
                 vector < double > &to_data,
                 vector < double > &to_errors );

      static bool crop(
                       const vector < double >  & x
                       ,const vector < double > & y
                       ,double                    x_min
                       ,double                    x_max
                       ,vector < double >       & x_cropped
                       ,vector < double >       & y_cropped
                       ,QString                 & errormsg
                       );

      static bool transpose_vvd(
                                const vector < vector < double > > & x
                                ,vector < vector < double > >      & y
                                ,QString                           & errormsg
                                );

      bool read_control( QString controlfile );

#if defined( USE_MPI )
      bool               run_iq_mpi                        ( QString controlfile );
      bool               run_nsa_mpi                       ( QString controlfile );
      bool               run_1d_mpi                        ( QString controlfile );
      bool               run_json_mpi                      ( QString & json );

      bool               run_pm_mpi                        ( QString controlfile );
      void               pm_mpi_worker                     ();
      set < int >        pm_workers_registered;
      set < int >        pm_workers_waiting;
      set < int >        pm_workers_busy;

#endif
      US_Log           * us_log;
      US_Udp_Msg       * us_udp_msg;

      QString run_json ( QString & json );
      
      bool run_pm      (
                        map < QString, QString >           & parameters,
                        map < QString, QString >           & results
                        );

      bool run_pm      ( QString controlfile );
      bool run_pm      ( QStringList qsl );
      bool run_pm      ( 
                        map < QString, vector < double > > & produced_q,
                        map < QString, vector < double > > & produced_I,
                        map < QString, QString >           & produced_model,
                        map < QString, QString >           & parameters,
                        map < QString, vector < double > > & vectors,
                        bool                                 quiet = false
                         );
      vector < vector < double > >  pmtypes_expand();


      bool run_pm_ok   ( QString option );
      QStringList      job_output_files;
      double             pm_ga_fitness_secs;
      unsigned int       pm_ga_fitness_calls;

      map < QString, QString > control_parameters;
      map < QString, vector < double > > control_vectors;

      static map < QString, unsigned int >            get_atom_summary_counts( PDB_model *model,
                                                                               map < QString, QString > &residue_atom_hybrid_map,
                                                                               saxs_options *use_saxs_options
                                                                               );
      static QString                                  list_atom_summary_counts( PDB_model *model,
                                                                                map < QString, QString > &residue_atom_hybrid_map,
                                                                                saxs_options *use_saxs_options
                                                                                );
      bool                                            compute_exponential( 
                                                                          vector < double > &q, 
                                                                          vector < double > &I,
                                                                          vector < double > &coeff4,
                                                                          vector < double > &coeff5,
                                                                          double            &norm4,
                                                                          double            &norm5
                                                                          );


      bool                                            compute_exponential( 
                                                                          vector < double > &q, 
                                                                          vector < double > &I,
                                                                          vector < double > &coeff4,
                                                                          vector < double > &coeff5,
                                                                          vector < double > &coeffv,
                                                                          double            &norm4,
                                                                          double            &norm5,
                                                                          double            &normv,
                                                                          // compute increased coeff size (when maxv > 5)
                                                                          unsigned int      maxv,
                                                                          bool              use_gsm_fitting = false,
                                                                          bool              use_quick_fitting = false
                                                                          );
      vector < double >                               compute_exponential_t;
      vector < double >                               compute_exponential_y;

      // returns a bead model with dR elements
      bool                                            grid( 
                                                           vector < PDB_atom > &bm,
                                                           vector < PDB_atom > &result,
                                                           double              dR, 
                                                           bool                accum_overlaps,        // if true, beads will increase in size
                                                           bool                allow_partials = false // if true attempt "partial" smaller beads for edges
                                                           );


      bool                                            smooth(
                                                             vector < double > &x,
                                                             vector < double > &result,
                                                             unsigned int      points,
                                                             bool              smooth_endpoints = false
                                                             );
                                                             

      bool                                            average( 
                                                              vector < double > &q,
                                                              vector < double > &I,
                                                              vector < double > &e,
                                                              double &avg_q,
                                                              double &avg_I,
                                                              double &avg_e
                                                               );

      bool                                            bin(
                                                          vector < double > &q,
                                                          vector < double > &I,
                                                          vector < double > &e,
                                                          vector < double > &new_q,
                                                          vector < double > &new_I,
                                                          vector < double > &new_e,
                                                          unsigned int      points
                                                          );
                                                             
      bool         align_test();

      static bool                                     average( 
                                                             const vector < vector < double > > & ys,
                                                             vector < double > & y_avg,
                                                             vector < double > & y_sd,
                                                             QString & error_msg
                                                              );

      static bool  read_sas_data( 
                                 QString             filename,
                                 vector < double > & q,
                                 vector < double > & I,
                                 vector < double > & e,
                                 QString           & error_msg 
                                 );
      static void  clip_data( 
                             double              minq,
                             double              maxq,
                             vector < double > & q,
                             vector < double > & I,
                             vector < double > & e
                             );
      static void bin_data( 
                           int                 bins,
                           bool                log_bin,
                           vector < double > & q,
                           vector < double > & I,
                           vector < double > & e
                           );
      static bool bin_data( 
                           int                 bins,
                           bool                log_bin,
                           vector < double > & q,
                           vector < double > & I,
                           vector < double > & e,
                           QString           & error_msg,
                           QString           & notice_msg
                           );

      bool  flush_pm_csv( 
                         vector < QString >           & csv_name,
                         vector < double >            & csv_q,
                         vector < vector < double > > & csv_I 
                         );

      static double       spline_erf( double x );

      static bool         pat_model( PDB_model & model, bool bead_coordinates = false );
      static bool         write_model( PDB_model & model, QString filename );

      static void         compute_tau( double drrev1,
                                       double drrev2,
                                       double drrev3,
                                       double fconv,
                                       vector < double > & tau );
         
      bool                pdb_mw( QString file, double & mw );
      bool                pdb_mw( QStringList & qsl, double & mw );

      static bool         pdb2fasta( QString outfile, QStringList &files, int max_line_size = 120 );

      static bool         mwt( 
                              const vector < double > & q,
                              const vector < double > & I,
                              double                    Rg,
                              double                    sigRg,
                              double                    I0,
                              double                    sigI0,
                              double                    k,
                              double                    c,
                              double                    qmax,
                              double                  & Vc,
                              double                  & Qr,
                              double                  & mwt,
                              double                  & sigmwt,
                              QString                 & messages,
                              QString                 & notes,
                              QString                 & warning
                               );

      bool                mwc( 
                              const vector < double > & q,
                              const vector < double > & I,
                              double                    Rg,
                              double                    sigRg,
                              double                    I0,
                              double                    sigI0,
                              double                    mw_per_N,
                              double                  & qm,
                              double                  & Vc,
                              double                  & Qr,
                              double                  & mwc,
                              double                  & sigmwc,
                              QString                 & messages,
                              QString                 & notes
                               );

      map < int, map < int, double > > prob_of_streak_cache;
      map < int, map < int, float > >  prob_of_streak_cache_f;

      double              prob_of_streak( int n, int c );
      float               prob_of_streak_f( int n, int c );

      bool                cormap( 
                                 const vector < double >            & q,
                                 const vector < vector < double > > & I,
                                 vector < vector < double > >       & rkl,
                                 int                                & N,
                                 int                                & S,
                                 int                                & C,
                                 double                             & P
                                  );

      static double       holm_bonferroni( vector < double > P, double alpha );

//FOR us_hydro: Methods plus PUBLIC Variables ///////////////////////////////////////
      
      double overlap_tolerance;
      QString residue_filename;

      map < QString, bool > connection_active;

// struct misc_options misc_1;

      struct hydro_options  hydro;
      struct pdb_parsing pdb_parse;
      struct BD_Options bd_options;
      struct Anaflex_Options anaflex_options;
      struct Anaflex_Options default_anaflex_options;
      struct batch_info batch;
      struct misc_options misc;    
      struct misc_options default_misc;    
      double default_overlap_tolerance;
      

      QString paths;
      QString path_load_pdb;
      QString path_view_pdb;
      QString path_load_bead_model;
      QString path_view_asa_res;
      QString path_view_bead_model;
      QString path_open_hydro_res;
      struct BD_Options default_bd_options;
      
      bool rotamer_changed;

      struct save_info save_params;

      map < QString, QString >            gparams;

      struct DMD_Options dmd_options;

      QString pdb_file;

      bool run_hydro   (
                        map < QString, QString >           & parameters,
                        map < QString, QString >           & results
                       );

      bool run_pat     (
                        map < QString, QString >           & parameters,
                        map < QString, QString >           & results
                       );

      bool run_align   (
                        map < QString, QString >           & parameters,
                        map < QString, QString >           & results
                       );

      //bool screen_pdb( QString, bool display_pdb = false, bool skip_clear_issue = false );
      void read_residue_file();
      bool screen_pdb(QString filename, bool  parameters_set_first_model); //, bool display_pdb, bool skipclearissue );
      int  check_for_missing_atoms_hydro(QString *error_string, PDB_model *model, bool parameters_set_first_model);
      void reload_pdb(bool parameters_set_first_model);

      bool hard_coded_defaults( map < QString, QString > &results, map < QString, QString > &parameters);
      bool set_default(map < QString, QString >  & results, map < QString, QString > &parameters);

      void update_vbar();

      bool calc_grid_pdb(bool parameters_set_first_model);
      
      QString getExtendedSuffix(bool prerun, bool somo);
      QString getExtendedSuffix_somo(bool prerun = true, bool somo = true, bool no_ovlp_removal = false ); 

      void bead_check( bool use_threshold = false, bool message_type = false ); // recheck beads
      int radial_reduction( bool from_grid = false, int use_ppos = 0, int mppos = 0 );
      int overlap_check(bool sc, bool mc, bool buried, double tolerance, int limit );
      int calc_hydro();
      int do_calc_hydro();

      int get_color_util_saxs(PDB_atom *);
      QString bead_model_suffix;
      QString project;   // name of the current project - derived from the prefix of the pdb filename
      
      QStringList list_of_models;


      int  calc_somo( bool no_ovlp_removal = false, bool parameters_set_first_model = false); 
      bool calc_somo_o(bool  parameters_set_first_model);   // somo model with overlaps  
      
      int write_pdb_hydro( QString fname, vector < PDB_atom > *model);
      bool calc_zeno_hydro();
      
      QString pdb_jsmol_script(vector < PDB_atom > *model);
      bool bead_model_from_file;

      static int us_usleep( unsigned int usec );

   private:

   // distance threshold check support
      bool run_ssbond   (
                        map < QString, QString >           & parameters,
                        map < QString, QString >           & results
                        );

      void SS_setup();                              // called once to setup any persistant distance threshold structures
      void SS_init();                               // called during load pdb to setup any processing structures
      void SS_apply( struct PDB_model & model,
                     QString & ssbond_data ); // called during load pdb to process any adjustments (CYS->CYH etc) *** per model! ***
      void SS_change_residue(                       // change the residue of an entry to target_residue
                             struct PDB_model & model
                             ,const QString & line
                             ,const QString target_residue
                                                );  
      
      set < QString >                                           cystine_residues;
      set < QString >                                           sulfur_atoms;

      vector < QString >                                        sulfur_pdb_line;
      vector < point   >                                        sulfur_coordinates;

      map < QString, map < QString, vector < unsigned int > > > sulfur_pdb_chain_atom_idx;
      map < QString, vector < unsigned int > >                  sulfur_pdb_chain_idx;

      map < int, int >                                          sulfur_paired;
      
      // end distance threshold check support


      // double       minusoneoverfourpisq;
      // unsigned int exponential_terms;
      // double       compute_exponential_f( double t, const double *par );

      QString options_log;
      void append_options_log_somo(); // append somo options to options_log
      void append_options_log_somo_ovlp(); // append somo options to options_log
      void append_options_log_atob(); // append atob options to options_log
      int compute_asa(bool bd_mode = false, bool no_ovlp_removal = false, bool parameters_set_first_model = false); // calculate maximum accessible surface area
      double total_volume_of_bead_model( vector < PDB_atom > &bead_model );


      enum issue_msg 
      { 
         ISSUE_RESPONSE_STOP
         ,ISSUE_RESPONSE_NC_SKIP
         ,ISSUE_RESPONSE_NC_REPLACE
         ,ISSUE_RESPONSE_MA_SKIP
         ,ISSUE_RESPONSE_MA_MODEL
      };

      
      set < QString > issue_info;
      int issue_missing_atom_hydro( bool quiet = false );
      int issue_non_coded_hydro   ( bool quiet = false );

      bool run_best();
      QStringList best_output_column( QString fname );

      void write_bead_model(QString, vector <PDB_atom> *, QString extra_text = "" );

      bool run_crysol();
      bool run_dammin();

      bool run_gnom( 
                    QString             project, 
                    QString             prefix, 
                    vector < QString >  files,
                    vector < double >   use_rmax_start,
                    vector < double >   use_rmax_end,
                    vector < double >   use_rmax_inc,
                    vector < double >   use_crop_low,
                    vector < double >   use_crop_high
                    );

      void clear_project();
      bool check_project_files();
      bool read_project_waves();

      bool compute_averages();
      void compute_alphas();
      QString wiki_file_name();
      bool wiki(QString &result);  // generate wiki page and associated pngs
      QString get_file_name(QString base, QString type);
      QString list_waves();

      QString p_project;           // contains a copy of last read project file
      QString p_wiki;
      QString p_wiki_prefix;
      QString p_name;
      QString p_description;
      QString p_short_description;
      QString p_comment;
      double  p_mw;
      double  p_conc_mult;
      double  p_saxs_lowq;
      double  p_saxs_highq;
      double  p_waxs_lowq;
      double  p_waxs_highq;
      double  p_waxs_zlowq;
      double  p_waxs_zhighq;
      double  p_overlap_lowq;
      double  p_overlap_highq;
      double  p_alpha_min;
      double  p_alpha_max;
      double  p_join_q;
      double  p_guinier_maxq;
      unsigned int  p_iterations_grid;
      unsigned int  p_iterations_gsm;
      double  p_rmax_start;
      double  p_rmax_end;
      double  p_rmax_inc;
      double  p_crop_low;
      double  p_crop_high;

      bool any_waxs;
      bool any_saxs;

      vector < QString >                  wave_names_vector;      // simple vector of names
      map < QString, bool >               wave_names;             // maps (unique) names to exist flag
      map < QString, QString >            wave_file_names;        // maps names to file_names
      map < QString, QString >            wave_types;             // maps names to waxs or saxs
      map < QString, double >             wave_concs;             // maps names to conc
      map < QString, double >             wave_alphas;            // maps names to computed or preset alpha
      map < QString, double >             wave_betas;             // maps names to preset beta (for waxs)
      map < QString, double >             wave_consts;            // maps names to preset const (for waxs)
      map < QString, double >             wave_exposure_times;    // maps names to conc
      map < QString, vector < QString > > wave_buffer_names;      // maps names to assoc buffer names
      map < QString, vector < QString > > wave_empty_names;       // maps names to assoc empty cell names
      map < QString, QString >            wave_comments;          // maps names to comments
      map < QString, double >             wave_alpha_starts;      // maps names to alpha start (for waxs)
      map < QString, double >             wave_alpha_ends;        // maps names to alpha ends  (for waxs)
      map < QString, double >             wave_alpha_incs;        // maps names to alpha incs  (for waxs)

      map < QString, double >             wave_overlap_lowq;      // maps names to wgsbs overlap lowq
      map < QString, double >             wave_overlap_highq;     // maps names to wgsbs overlap highq

      // computed:
      map < QString, QString >            wave_sb;                // maps wave to standard background (and empty cell) subtraction
      map < QString, QString >            wave_wgsbs;             // maps saxs wave to waxs guided saxs background subtraction
      map < QString, QString >            wave_wgsbs_unscaled;    // maps saxs wave to waxs guided saxs background subtraction
      map < QString, QString >            wave_join;              // maps saxs wave to waxs guided saxs background subtraction join

      map < QString, double >             wave_Rgs;               // maps names to Rg
      map < QString, double >             wave_I0s;               // maps names to I0
      map < QString, double >             wave_smins;             // maps names to smins
      map < QString, double >             wave_smaxs;             // maps names to smaxs
      map < QString, double >             wave_sRgmins;           // maps names to sRg min
      map < QString, double >             wave_sRgmaxs;           // maps names to sRgmax
      map < QString, double >             wave_chi2s;             // maps names to chi^2

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

      bool wgsbs_gsm_setup;

      double wgsbs_gsm_f(our_vector *v);
      void wgsbs_gsm_df(our_vector *vd, our_vector *v);

      double compute_gsm_exponentials_f( our_vector *v );

      bool c2check( QString a, QString b );

      static bool compute_rg_from_pr(
                                     const vector < double >  & r
                                     ,const vector < double > & pr
                                     ,double                  & Rg
                                     ,QString                 & errormsg
                                     );

   private:
      // exponential gsm:
      vector < double > compute_gsm_exponentials_mtto4pi2;
      vector < double > compute_gsm_exponentials_y;

      double compute_gsm_exponentials_norm;
      bool compute_gsm_exponentials_is_on;
      bool compute_gsm_exponentials( vector < double > &par,
                                     vector < double > &t,
                                     vector < double > &y,
                                     QString method );

      void compute_gsm_exponentials_setup( vector < double > &t,
                                           vector < double > &y );
      // other gsm stuff

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

      bool select_atom_file( QString );
      bool select_hybrid_file( QString );
      bool select_saxs_file( QString );
      bool select_residue_file( QString );
      void calc_bead_mw( residue *res );
      void calc_vbar( PDB_model *);
      void reset_chain_residues( PDB_model *model );
      bool read_pdb( QString filename );
      bool read_pdb_hydro( QString filename, bool parameters_set_first_model );

      bool read_pdb( QStringList &qsl );
      bool dna_rna_resolve();
      bool assign_atom( const QString &str1, PDB_chain *temp_chain, QString *last_resSeq );
      void clear_temp_chain( PDB_chain *temp_chain );

      saxs_options our_saxs_options;
      bool run_iqq();
      bool calc_saxs_iq_native_fast();
      bool calc_saxs_iq_native_debye();
      bool calc_saxs_iq_native_hybrid();
      QString iqq_suffix();
      void setup_saxs_options();


      // residue fields

      vector < residue >                  residue_list;
      vector < residue >                  residue_list_no_pbr;
      map < QString, int >                new_residues;    // maps resName|atom_count to {0,1} for duplicate checks
      map < QString, bool >               unknown_residues;
      vector < residue >                  save_residue_list;
      vector < residue >                  save_residue_list_no_pbr;
      map < QString, vector < int > >     valid_atom_map;    // maps resName|atomName|pos
      map < QString, vector <int> >       save_multi_residue_map; // maps residue to index of residue_list
      vector < PDB_model >                model_vector;
      vector < PDB_model >                model_vector_as_loaded;
      vector < PDB_atom >                 bead_model;
      vector < vector < PDB_atom > >      bead_models;
      vector < vector < PDB_atom > >      bead_models_as_loaded;
      map < QString, int >                atom_counts;     // maps molecule #|resName|resSeq to count
      //                                       counts how many atoms are in each residue
      map < QString, int >                has_OXT;         // maps molecule #|resName|resSeq to {0,1}
      map < QString, int >                bead_exceptions; // maps molecule #|resName|resSeq to count
      //                                       1 == ok
      //                                       2 == skip
      //                                       3 == use automatic bead builder for residue
      //                                       4 == use automatic bead builder for missing atom
      //                                       5 == not ok, duplicate or non-coded atom
      map < QString, bool >               broken_chain_end;
      //                                  maps molecule #|resSeq to flag indicating broken end of chain
      map < QString, bool >               broken_chain_head;
      //                                  maps molecule #|resSeq to flag indicating broken head of chain
      map < QString, vector < QString > > molecules_residues_atoms;
      //                                  maps molecule #|resSeq to vector of atom names
      map < QString, QString > molecules_residue_name;
      //                                  maps molecule #|resSeq to residue name
      vector < QString > molecules_idx_seq; // vector of idx's
      map < QString, vector < QString > > molecules_residue_errors;
      //                                  maps molecule #|resSeq to vector of errors
      //                                  each element in the vector corresponds to
      //                                  the dup_residue_map pos for the residue
      map < QString, vector < int > >     molecules_residue_missing_counts;
      //                                  maps molecule #|resSeq to vector of missing count
      //                                  if any atoms errors that are "non-missing" i.e.
      //                                  duplicate or non-coded, then the value is set to -1
      map < QString, vector < vector < QString > > > molecules_residue_missing_atoms;
      //                                  maps molecule #|resSeq to vector of vector of missing atoms
      //                                  each vector in the vector corresponds to
      //                                  the dup_residue_map pos for the residue
      map < QString, vector < vector < unsigned int > > > molecules_residue_missing_atoms_beads;
      //                                  maps molecule #|resSeq to vector of vector of missing atoms beads
      //                                  each vector in the vector corresponds to
      //                                  the dup_residue_map pos for the residue
      map < QString, bool >               molecules_residue_missing_atoms_skip;
      //                                  maps molecule #|resSeq|multiresmappos|atom_no to flag
      //                                  if true, the atom should be skipped
      map < QString, int  >               use_residue;
      //                                  maps molecule #|resSeq to correct residue in residue_list to use
      map < QString, bool  >              skip_residue;
      //                                  flags molecule "#|resSeq|residue_list entry" to be ignored
      map < QString, int >                molecules_residue_min_missing;
      //                                  maps molecule #|resSeq to pos of entry with minimum missing count
      QString last_abb_msgs; // automatic bead builder message log
      vector < PDB_atom * >               active_atoms;
      map < QString, bool >               residue_errors;         // maps pdb_tool style csv keys
      map < QString, bool >               global_residue_errors;  // chain indepdendent

      vector < QString >                  saxs_inputfile_for_csv;
      vector < QString >                  saxs_model_for_csv;
      vector < QString >                  saxs_tag_for_csv;
      vector < QString >                  saxs_grid_tag_for_csv;
      vector < QString >                  saxs_cuda_tag_for_csv;
      vector < QString >                  saxs_method_for_csv;
      vector < vector < double > >        saxs_q_for_csv;
      vector < vector < double > >        saxs_I_for_csv;

      QStringList  output_files;
      QString      last_pdb_filename;
      QStringList  last_pdb_title;
      QStringList  last_pdb_header;
      QStringList  last_pdb_load_calc_mw_msg;
      QStringList  last_steric_clash_log;
      residue      current_residue;
      bool         calc_mw();
      bool         calc_mw_hydro(); 
      

      bool set_control_parameters_from_experiment_file( QString filename, bool load_without_interp = false );
      bool validate_control_parameters( bool for_sgp = false );
      void validate_control_parameters_set_one( QStringList &checks, 
                                                QStringList &vals );
      bool create_tar_output( QString filename );
      bool create_tgz_output( QString filename );
      QString vector_double_to_csv( vector < double > &vd );
      bool write_output( QString model, vector < double > &q, vector < double > &I );
      static bool write_iq( QString & name, QString &msg, vector < double > &q, vector < double > &I );
      bool flush_output();
      bool flush_output_one();

      unsigned int write_output_count;

      QString      env_ultrascan;
      bool         run_saxs_iq_foxs();
      bool         run_saxs_iq_crysol();
      bool         run_sans_iq_cryson();
      bool         load_saxs( QString );

      bool         compute_scale_excl_vol();
      QString      scale_excl_vol_msgs;
      // hydration routines
      map < QString, vector < vector < QString > > > dihedral_atoms;

      map < QString, vector < vector < QString > > > pointmap_atoms;
      map < QString, vector < vector < QString > > > pointmap_atoms_dest;
      map < QString, vector < QString > >            pointmap_atoms_ref_residue;

      map < QString, vector < rotamer > >            rotamers;
      map < QString, vector < vector < rotamer > > > rotated_rotamers;
      map < QString, map < QString, point > >        to_hydrate;
      map < QString, map < QString, point > >        to_hydrate_pointmaps;
      map < QString, vector < float > >              to_hydrate_dihedrals;
      map < QString, rotamer >                       best_fit_rotamer;
      map < QString, vector < rotamer > >            pointmap_rotamers;
      map < QString, vector < point > >              waters_to_add;
      map < QString, unsigned int >                  steric_clash_summary;
      map < QString, bool >                          hydrate_clash_map_structure;
      map < QString, bool >                          hydrate_clash_map_rtmr_water;
      map < QString, bool >                          hydrate_clash_map_pm_water;
      map < QString, vector < QString > >            waters_source;
      map < QString, unsigned int >                  file_write_count;

      map < QString, float >                         residue_asa;
      map < QString, float >                         residue_asa_sc;
      map < QString, float >                         residue_asa_mc;

      map < QString, unsigned int >                  hydrate_count;

      bool         atom_align( vector < point > transform_from, 
                               vector < point > transform_to, 
                               vector < point > apply_from, 
                               vector < point > &result );
      bool         load_rotamer( QString filename );
      bool         compute_rotamer_dihedrals();
      QString      list_rotamer_dihedrals();
      QString      list_pointmaps();
      QString      list_rotamers( bool coords );
      bool         compute_water_positioning_atoms();
      QString      list_water_positioning_atoms();
      bool         validate_pointmap();
      bool         selected_models_contain( QString residue );
      bool         buffer_pdb_with_waters();
      bool         flush_pdb();
      QString      last_hydrated_pdb_header;
      QString      last_hydrated_pdb_text;
      QString      last_hydrated_pdb_name;
      bool         has_steric_clash( point p );
      bool         compute_waters_to_add();
      bool         compute_best_fit_rotamer();
      QString      list_best_fit_rotamer();
      bool         setup_pointmap_rotamers();
      QString      list_pointmap_rotamers();
      QString      list_waters_to_add();
      QString      list_exposed();
      QString      list_to_hydrate( bool coords );
      QString      list_to_hydrate_dihedrals();
      bool         compute_to_hydrate_dihedrals();
      void         build_to_hydrate();
      bool         pdb_asa_for_saxs_hydrate();
      bool         check_for_missing_atoms( PDB_model *model, QStringList &qsl );
      bool         check_for_missing_atoms( PDB_model *model );
      void         get_atom_map( PDB_model *model );
      // various parameters to get bead stuff working
      bool         misc_pb_rule_on;
      // need to define in us_saxs_util_iqq.cpp/ hydrate / control_params
      double       misc_avg_volume;
      double       misc_avg_vbar;
      double       misc_avg_mass;
      double       misc_avg_hydration;
      double       misc_avg_radius;
      double       misc_hydrovol;
      void         build_molecule_maps( PDB_model *model );
      bool         create_beads();
      int create_beads_hydro(QString *error_string, bool quiet = false);
      point        last_molecular_cog;
      bool         create_beads_normally;
      asa_options  asa;
      unsigned int current_model;
      bool         pdb_hydrate();
      float        min_dist_to_struct_and_waters( point p );
      float        min_dist_to_waters( point p );
      bool         list_steric_clash_recheck();
      bool         compute_waters_to_add_alt();
      unsigned int hydrate_max_waters_no_asa;
      unsigned int count_waters;
      unsigned int count_waters_added;
      unsigned int count_waters_not_added;
      QStringList  hydrate_clash_log;

      bool         dmd_strip_pdb();
      bool         dmd_findSS();
      bool         dmd_prepare();
      bool         dmd_run( QString run_description );
      QString      last_dmd_description;
      QString      last_state_file;
      bool         input_dimensions( point &range );
      QStringList  output_dmd_pdbs;
      QStringList  experimental_grids;
      bool         process_one_iqq();

      // dmd hetatm mol2 support structures:
      map < QString, QString >              dmd_mol2_res;         // newly assigned residue name to original residue name
      map < QString, map < int, int > >     dmd_chain;            // maps chain id and residue number to dmd's chain number
      map < QString, map < int, int > >     dmd_res;              // maps chain id and residue number to dmd's residue number
      map < QString, map < int, int > >     dmd_res_link;         // maps chain id and residue number to dmd's residue number for links
      map < QString, map < int, QString > > dmd_org_chain;        // maps chain id and residue number to pdb's original chain number
      map < QString, map < int, int > >     dmd_org_res;          // maps chain id and residue number to pdb's original residue number
      QStringList                           dmd_pdb_add_back;     // lines to restore to pdbs - REMARKs etc, not ATOMs or HETATMs
      map < QString, QStringList >          dmd_pdb_prepare_reports; // for easy GUI display
      map < QString, set < int > >          dmd_chain_is_hetatm;  // maps chain id and residue number to hetatm status

      // dmd for truncated base name
      QString                               dmd_basename;

      // dmd hetatm mol2 support routines:
      void                              dmd_clear();                                   // clears data structures 
      QString                           dmd_next_res( const QString & source );        // returns a unique residuename and updates dmd_mol2
      bool                              dmd_pdb_prepare( QStringList & qsl_pdb
                                                         ,QStringList & qsl_pdb_removed
                                                         ,QStringList & qsl_link_constraints
                                                         ,bool production_run = true );
                                                                                       // process HETATM, LINK and renumbering
      bool                              dmd_pdb_prepare( QString & pdb ); // utility, non-production version

      bool                              dmd_pdb_restore( const QStringList & qsl_pdb
                                                         ,QStringList & qsl_pdb_restored
                                                         ,bool add_back = true );
                                                                                      // restore HETATM, LINK and renumbering
                                                                                      // add_back will add back dmd link & remark 766s

      // dmd fix position of HETATM
      QString                           dmd_fix_hetatm_name_pos( const QString & qs );
      QStringList                       dmd_fix_hetatm_name_pos( const QStringList & qsl );

      static map < QString, QString >   pdb_fields( const QString & pdb_line );
      
      bool         strip_pdb( 
                             QString & pdb_stripped,
                             const QString & pdb,
                             const QStringList & exclude_atoms_list,
                             const QStringList & exclude_residues_list,
                             bool exclude_hydrogens = false
                              );

      bool         calc_saxs_iq_native_fast_bead_model  ();
      bool         calc_saxs_iq_native_debye_bead_model ();
      bool         calc_saxs_iq_native_hybrid_bead_model();
      bool         run_iqq_bead_model();

      bool         dmd_run_with_log( const QString & tag
                                     ,const QString & cmd
                                     ,const QString & log
                                     );
      
      static float mw_to_volume( float mw, float vbar );

      // gp stuff

      vector < sgp_node * >          population;
      vector < double >              sgp_last_q;       // we don't really need this, do we?
      vector < double >              sgp_last_I;
      vector < double >              sgp_exp_q;       
      map < double, unsigned int >   sgp_exp_q_index;
      vector < double >              sgp_exp_I;
      vector < double >              sgp_exp_e;
      vector < double >              exact_q;

      vector < float * >             nsa_var_ref;
      vector < float   >             nsa_var_min;
      vector < float   >             nsa_var_max;

      // stuff for computing a2sb

      // here, a point defines the center of cublet
      // each atom is assigned to one cubelet and then saxs curves are computed on each,
      // then 1sa spheres are created for each which becomes the a2sb model
      map < point, vector < PDB_atom > > a2sb_map;


      // ift stuff

      vector < double >               bspline_net;
      vector < vector < double > >    bspline_omega;
      unsigned int                    bspline_degree;
      unsigned int                    bspline_basis_functions;

      sgp_node              sgp;
      bool                  sgp_use_e;

      bool                  sgp_run();
      bool                  sgp_validate();
      bool                  sgp_init();
      bool                  sgp_init_sgp();
      double                sgp_fitness( sgp_node *node );
      void                  sgp_calculate_population_fitness();
      void                  sgp_sort_population();
      unsigned int          sgp_pop_selection();

      QString               sgp_physical_stats( sgp_node *node );

      bool                  nsa_validate         ();
      bool                  nsa_run              ();

      // nsa gsm:
      bool                  nsa_gsm_setup;

      bool                  nsa_gsm              ( double &nrmsd, 
                                                   our_vector *vi = ( our_vector *)0,
                                                   QString method = "" );

      long                  nsa_min_gsm_5_1      (
                                                  our_vector *i,
                                                  double epsilon,
                                                  long max_iter
                                                  );

      long                  nsa_min_fr_pr_cgd    (
                                                  our_vector *i,
                                                  double epsilon,
                                                  long max_iter
                                                 );

      long                  nsa_min_hessian_bfgs (
                                                  our_vector *ip,
                                                  double epsilon,
                                                  long max_iter
                                                  );

      bool                  nsa_fitness_setup    ( unsigned int size );
      double                nsa_fitness          ();
      double                nsa_delta_rho;
      double                nsa_gsm_delta;
      double                nsa_gsm_delta2_r;
      double                nsa_gsm_f            ( our_vector *v );
      void                  nsa_gsm_df           ( our_vector *vd, our_vector *v );
      QString               nsa_qs_bead_model    ();
      QString               nsa_physical_stats   ();
      bool                  nsa_ga               ( double &nrmsd );
#if defined( USE_MPI )
      bool                  nsa_ga_master_test   ( double &nrmsd );
      bool                  nsa_ga_master        ( double &nrmsd );
      bool                  nsa_ga_worker        ();
      bool                  nsa_ga_process_queue ();
      bool                  nsa_ga_close_workers ();

      list < nsa_ga_individual >                 queued_requests;
      list < nsa_ga_individual >                 received_results;
      map < int, bool >                          waiting_workers;
      map < int, bool >                          busy_workers;
      map < int, bool >                          registered_workers;
#endif
      bool                  nsa_use_scaling_fit;
      double                nsa_last_scaling;

      bool                  nsa_ga_fitness       ( nsa_ga_individual & individual );
      unsigned int          nsa_pop_selection    ( unsigned int size );
      bool                  nsa_mpi;
      bool                  nsa_ess;
      bool                  nsa_excl;

      bool                  nsa_sga_validate     ();
      bool                  nsa_sga_run          ();
      bool                  nsa_sga              ( double &nrmsd );
      double                nsa_sga_fitness      ( nsa_sga_individual individual );
      nsa_sga_individual    nsa_sga_last_individual;
      bool                  check_overlap        ( vector < PDB_atom > &bm, bool quiet = true );

      map < QString, QString >                    nsa_physical_stats_map;
      map < unsigned int, vector < nsa_ga_individual > > nsa_ga_inits;

      // a2sb:
      bool                  a2sb_validate        ();
      float                 a2sb_cube_side;
      // convert a point to its cubelet point
      point                 a2sb_cubelet         ( point p1 ); 
      point                 a2sb_cubelet         ( PDB_atom &this_atom ); 
      bool                  a2sb_run             ();
      
      // ift:

      // compute a b-spline on the curent grid (sgp_exp_q)
      bool                  bspline_basis        ( unsigned int basis, 
                                                   double q, 
                                                   double &value );
      // prepare the net for bspline calcs
      bool                  bspline_prepare      ( unsigned int knots, 
                                                   unsigned int degree );
      bool                  bspline_test         ();

      bool                  iqq_cuda             (
                                                  vector < double >            &q,
                                                  vector < saxs_atom >         &atoms,
                                                  vector < vector < double > > &fp,
                                                  vector < double >            &I,
                                                  unsigned int                 threads_per_block
                                                  );
      QString               timings;
      bool                  write_timings        ( QString file, QString msg );


      // compute_ff

      map < QString, unsigned int >   ff_table;
      vector < vector < double > >    ff_q;
      vector < vector < double > >    ff_ff;
      vector < vector < double > >    ff_y2;
      vector < double >               ff_ev;
      map < QString, bool >           ff_sent_msg1;
      map < QString, vector < point > >                                   hybrid_coords;
      map < QString, map < unsigned int, map < unsigned int, double > > > hybrid_r;

      double compute_ff(
                        saxs     &sa,     // gaussian decomposition for the main atom
                        saxs     &sh,     // gaussian decomposition for hydrogen
                        QString  &nr,     // name of residue
                        QString  &na,     // name of atom
                        QString  &naf,    // full name of atom
                        unsigned int h,   // number of hydrogens
                        double   q,
                        double   q_o_4pi2 
                        );

      bool                 ff_table_loaded;
      bool                 load_ff_table( QString filename );
      QString              ff_info();
      QString              last_ff_filename;
      double               get_ff_ev( QString res, QString atm );

      // 1d computations
      // required variables:
      // 1dlambda
      // 1ddetectordistance
      // 1ddetectorwidth
      // 1ddetectorpixelswidth
      // 1drho0 (use already defined electron density? )
      // 1ddeltar 
      // 1dproberadius
      // 1dthreshold
      // 1dsamplerotations
      // 1d
      vector < complex < double > >                   s1d_data;
      vector < double >                               total_modulii;
      vector < point >                                excluded_volume;
      bool                                            load_rotations( unsigned int number,
                                                                      vector < vector < double > > &rotations );
      bool                                            compute_1d();

#if defined( USE_MPI )
      bool                                            compute_1d_mpi();
      bool                                            load_rotations_mpi( unsigned int number,
                                                                          vector < vector < double > > &rotations );
      void                                            hypercube_rejection_drand_rotations(
                                                                                          unsigned int number, 
                                                                                          vector < vector < double > > &rotations );
#endif

      bool                                            update_image();

      double                                          q_of_pixel( int pixels_width );
      double                                          q_of_pixel( double width );

      double                                          lambda;

      int                                             detector_pixels_width;
      double                                          detector_width;
      double                                          detector_width_per_pixel;

      double                                          rho0;
      double                                          deltaR;

      double                                          detector_distance;

      unsigned int                                    plot_count;
      unsigned int                                    sample_rotations;

      QString                                         filepathname;
      QString                                         mapname;
      bool                                            setup_excluded_volume_map();
      bool                                            get_excluded_volume_map();

      double                                          probe_radius;
      double                                          threshold;


// From (private) US_Hydrodyn:: in an order as it goes in 'read_residue_file()' //////// */

      void calc_vol_for_saxs(); 

      map < QString, QString > residue_atom_abb_hybrid_map;
      QStringList msroll_radii;
      QStringList msroll_names;
      vector <int> somo_processed;
      struct residue new_residue;
      struct atom new_atom;
      struct bead new_bead;
      map < QString, QChar > residue_short_names;
      struct hydro_results results_hydro;
      
      /* Defaults */
      bool replicate_o_r_method_somo;
      bool replicate_o_r_method_grid;
      struct overlap_reduction sidechain_overlap;
      struct overlap_reduction mainchain_overlap;
      struct overlap_reduction buried_overlap;
      struct overlap_reduction grid_exposed_overlap;
      struct overlap_reduction grid_buried_overlap;
      struct overlap_reduction grid_overlap;

      struct overlap_reduction default_sidechain_overlap;
      struct overlap_reduction default_mainchain_overlap;
      struct overlap_reduction default_buried_overlap;
      struct overlap_reduction default_grid_exposed_overlap;
      struct overlap_reduction default_grid_buried_overlap;
      struct overlap_reduction default_grid_overlap;
      struct bead_output_options default_bead_output;
    
      struct asa_options default_asa;
      struct hydro_options default_hydro;

      struct pdb_visuzalization *pdb_vis;         
      struct pdb_visuzalization *default_pdb_vis;

      struct pdb_parsing default_pdb_parse;

      struct grid_options default_grid_hydro;
      struct saxs_options default_saxs_options;
      struct batch_info default_batch;
      struct save_info default_save_params;

      struct bead_output_options bead_output;
      struct asa_options asa_hydro;
      struct grid_options grid_hydro;
      struct saxs_options saxs_options_hydro;

      map < QString, QString >            default_gparams;



};

#if QT_VERSION >= 0x040000
extern QString us_tr( QString );
extern const char * us_trp( QString );
extern void us_qdebug( QString );
FILE * us_fopen( QString f, const char *mode );
#endif

# if defined( USE_MPI )
   extern void debug_mpi( QString );
# endif

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif

#endif

