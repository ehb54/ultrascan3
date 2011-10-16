#ifndef US_SAXS_UTIL_H
#define US_SAXS_UTIL_H

// QT defs:

#include "us_util.h"
#include "us_math.h"
#include "us_tar.h"
#include "us_hydrodyn_pdbdefs.h"
#include <math.h>
#include <time.h>
#include <map>
#include <qregexp.h>

#if defined(WIN32)
#   include <dos.h>
#   include <stdlib.h>
#   include <float.h>
#   define isnan _isnan
#   undef SHOW_TIMING
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
      bool dna_rna_resolve();
      bool assign_atom( const QString &str1, PDB_chain *temp_chain, QString *last_resSeq );
      void clear_temp_chain( PDB_chain *temp_chain );

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

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
      QString      last_pdb_filename;
      QStringList  last_pdb_title;
      QStringList  last_pdb_header;
      residue      current_residue;

      bool set_control_parameters_from_experiment_file( QString filename );
      bool validate_control_parameters();
      void validate_control_parameters_set_one( QStringList &checks, 
                                                QStringList &vals );
};

#endif
