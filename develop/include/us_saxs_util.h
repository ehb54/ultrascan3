#ifndef US_SAXS_UTIL_H
#define US_SAXS_UTIL_H

// QT defs:

#include "us_util.h"
#include "us_math.h"
#include "us_tar.h"
#include "us_gzip.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_asa.h"
#include <math.h>
#include <time.h>
#include <map>
#include <qregexp.h>
#include "us_saxs_gp.h"
#include "us_saxs_util_nsa.h"

#if defined(WIN32)
#   include <dos.h>
#   include <stdlib.h>
#   include <float.h>
#   define isnan _isnan
#   undef SHOW_TIMING
#   define drand48() ((double)rand()/RAND_MAX)
#   define srand48(x) srand(x)
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

      friend class US_Hydrodyn_Pdb_Tool;

      US_Saxs_Util();

      void   clear      ();
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
                       double &Io,
                       double &smaX,
                       double &smin,
                       double &sRgmin,
                       double &sRgmax
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
                       double &Io,
                       double &smaX,
                       double &smin,
                       double &sRgmin,
                       double &sRgmax,
                       unsigned int &beststart,
                       unsigned int &bestend
                       );

      int debug;
      QString errormsg;
      QString noticemsg;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      map < QString, US_Saxs_Scan > wave;
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

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
      void linear_fit( 
                      vector < double > x, 
                      vector < double > y, 
                      double &a,
                      double &b,
                      double &siga,
                      double &sigb,
                      double &chi2
                      );

      // linear fit code, solves  y = kx, returing chi2
      void scaling_fit( 
                       vector < double > x, 
                       vector < double > y, 
                       double &a,
                       double &chi2
                       );

      void scaling_fit( 
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

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      vector < atom >                atom_list;
      vector < hybridization >       hybrid_list;
      vector < saxs >                saxs_list;
      map < QString, saxs >          saxs_map;
      map < QString, hybridization > hybrid_map;
      map < QString, atom >          atom_map;
      map < QString, vector <int> >  multi_residue_map;
      map < QString, QString >       residue_atom_hybrid_map;
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

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

      static void natural_spline( vector < double > &x, 
                                  vector < double > &y,
                                  vector < double > &y2 );

      // takes from_data with from_errors and places on to_grid resulting in to_data & to_errors
      // for systems without errors, these are assumed to be computed iq curves so a natural spline interpolation
      // is performed
      // for systems with errors, these are assumed to be experimental data it is only acceptible going
      // to a coarser grid, otherwise an error will result with error_msg set

      bool is_nonzero_vector( vector < double > &v ); // returns true if all non-zero

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

      bool read_control( QString controlfile );

#if defined( USE_MPI )
      bool run_iq_mpi  ( QString controlfile );
      bool run_nsa_mpi ( QString controlfile );
#endif
      QStringList      job_output_files;

   private:

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

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

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
      map < QString, double >             wave_Ios;               // maps names to Io
      map < QString, double >             wave_smins;             // maps names to smins
      map < QString, double >             wave_smaxs;             // maps names to smaxs
      map < QString, double >             wave_sRgmins;           // maps names to sRg min
      map < QString, double >             wave_sRgmaxs;           // maps names to sRgmax
      map < QString, double >             wave_chi2s;             // maps names to chi^2

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

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

      bool select_atom_file( QString );
      bool select_hybrid_file( QString );
      bool select_saxs_file( QString );
      bool select_residue_file( QString );
      void calc_bead_mw( residue *res );
      void calc_vbar( PDB_model *);
      bool read_pdb( QString filename );
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

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      map < QString, QString > control_parameters;

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
      vector < unsigned int >             saxs_model_for_csv;
      vector < QString >                  saxs_tag_for_csv;
      vector < QString >                  saxs_grid_tag_for_csv;
      vector < QString >                  saxs_method_for_csv;
      vector < vector < double > >        saxs_q_for_csv;
      vector < vector < double > >        saxs_I_for_csv;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
      QStringList  output_files;
      QString      last_pdb_filename;
      QStringList  last_pdb_title;
      QStringList  last_pdb_header;
      residue      current_residue;

      bool set_control_parameters_from_experiment_file( QString filename );
      bool validate_control_parameters( bool for_sgp = false );
      void validate_control_parameters_set_one( QStringList &checks, 
                                                QStringList &vals );
      bool create_tar_output( QString filename );
      bool create_tgz_output( QString filename );
      QString vector_double_to_csv( vector < double > &vd );
      bool write_output( unsigned int model, vector < double > &q, vector < double > &I );
      bool flush_output();
      bool flush_output_one();

      unsigned int write_output_count;

      QString      env_ultrascan;
      bool         run_saxs_iq_foxs();
      bool         run_saxs_iq_crysol();
      bool         run_sans_iq_cryson();
      bool         load_saxs( QString );

      // hydration routines
#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      map < QString, vector < vector < QString > > > dihedral_atoms;

      map < QString, vector < vector < QString > > > pointmap_atoms;
      map < QString, vector < vector < QString > > > pointmap_atoms_dest;
      map < QString, vector < QString > >            pointmap_atoms_ref_residue;

      map < QString, vector < rotamer > >            rotamers;
      map < QString, map < QString, point > >        to_hydrate;
      map < QString, map < QString, point > >        to_hydrate_pointmaps;
      map < QString, vector < float > >              to_hydrate_dihedrals;
      map < QString, rotamer >                       best_fit_rotamer;
      map < QString, vector < rotamer > >            pointmap_rotamers;
      map < QString, vector < point > >              waters_to_add;
      map < QString, unsigned int >                  file_write_count;
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

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
      bool         selected_models_contain_SWH();
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
      point        last_molecular_cog;
      bool         create_beads_normally;
      asa_options  asa;
      unsigned int current_model;
      bool         pdb_hydrate();

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

      bool         calc_saxs_iq_native_fast_bead_model();
      bool         calc_saxs_iq_native_debye_bead_model();
      bool         calc_saxs_iq_native_hybrid_bead_model();
      bool         run_iqq_bead_model();

      // gp stuff

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

      vector < sgp_node * > population;
      vector < double >     sgp_last_q;       // we don't really need this, do we?
      vector < double >     sgp_last_I;
      vector < double >     sgp_exp_q;       
      vector < double >     sgp_exp_I;
      vector < double >     sgp_exp_e;

      vector                < float * > nsa_var_ref;
      vector                < float   > nsa_var_min;
      vector                < float   > nsa_var_max;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
      bool                  sgp_use_e;

      sgp_node              sgp;
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
};

#endif
