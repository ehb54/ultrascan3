#ifndef US_PM_H
#define US_PM_H

#include <iostream>
#include <math.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <algorithm>
#include <iterator>
#include <set>
#include <vector>
#include <complex>
#include "us_hydrodyn_pdbdefs.h"
#include "us_vector.h"
#include "us_json.h"
#include "us_timer.h"
#include "us_sh.h"
#include "us_saxs_util.h"
#include "us_hydrodyn_pat.h"

#ifdef WIN32
typedef _int16 int16_t;
#else
#include <stdint.h>
#endif

#ifndef M_PI
#  define M_PI       3.14159265358979323846
#endif

#ifndef M_4PI
#  define M_4PI       (4e0 * 3.14159265358979323846)
#endif

// based upon int16 right now
#define USPM_MAX_VAL   32767
#define USPM_MAX_VAL_D ( ( double ) USPM_MAX_VAL )
#define USPM_MIN_VAL   -32767
#define USPM_MIN_VAL_D ( ( double ) USPM_MIN_VAL )

class pm_point
{
 public:
   int16_t x[ 3 ];
   bool operator < ( const pm_point & objIn ) const
   {
      return
         x[ 0 ] < objIn.x[ 0 ] ||
         ( x[ 0 ] == objIn.x[ 0 ] &&
           x[ 1 ] <  objIn.x[ 1 ] ) ||
         ( x[ 0 ] == objIn.x[ 0 ] &&
           x[ 1 ] == objIn.x[ 1 ] &&
           x[ 2 ] <  objIn.x[ 2 ] );
   }
};

class pm_point_f
{
 public:
   float x[ 3 ];
   bool operator < ( const pm_point_f & objIn ) const
   {
      return
         x[ 0 ] < objIn.x[ 0 ] ||
         ( x[ 0 ] == objIn.x[ 0 ] &&
           x[ 1 ] <  objIn.x[ 1 ] ) ||
         ( x[ 0 ] == objIn.x[ 0 ] &&
           x[ 1 ] == objIn.x[ 1 ] &&
           x[ 2 ] <  objIn.x[ 2 ] );
   }
};

struct pm_data
{
   float x[ 3 ]; // converted to grid
   float rtp[ 3 ];
   bool  no_J;
   bool  no_Y;
   vector < double > J;
   vector < complex < float > > Y;
};

struct pmc_data
{
   float x[ 3 ]; // converted to grid
   float rtp[ 3 ];
   vector < complex < float > > A1v;
};

// so we can try floats or doubles ?
// typedef double us_pm_real;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

class US_PM
{
 private:

   // stuff for spherical harmonics

   double                                  grid_conversion_factor;
   int                                     max_dimension;
   double                                  max_dimension_d;
   double                                  one_over_grid_conversion_factor;

   double                                  drho;
   double                                  buffer_e_density;
   double                                  ev;

   double                                  cube_size;
   double                                  bead_radius;
   double                                  bead_radius_over_2;

   // active grid:
   vector < double >                       q;
   vector < double >                       I;
   vector < double >                       e;

   unsigned int                            q_points;

   // precomputed bead structure factors
   // these can come from rayleigh or compute_ff for an atom type
   vector < double >                       F;

   // ideas:
   // best of each single model
   // best of each pair
   // best of each triple
   // etc
   // apply to various experimental & simulated data

   bool                                    rotation_matrix(
                                                           double l, 
                                                           double m, 
                                                           double n, 
                                                           double theta, 
                                                           vector < vector < double > > &rm 
                                                           );
   bool                                    apply_rotation_matrix(
                                                                  vector < vector < double > > &rm, 
                                                                  int x, 
                                                                  int y, 
                                                                  int z, 
                                                                  double & newx, 
                                                                  double & newy, 
                                                                  double & newz 
                                                                  );

   map < pm_point, pm_data >               pdata;
   map < pm_point, pmc_data >              pcdata;

   // sh variables

   unsigned int                            max_harmonics;
   unsigned int                            no_harmonics;
   // fib grid is for hydration
   // unsigned int fibonacci_grid;

   // supplementary sh variables

   vector < complex < float > >            ccY;
   vector < complex < float > >            ccA1v;
   vector < double >                       ccJ;

   complex < float >                       Z0; // ( 0e0, 0e0 );
   complex < float >                       i_; // ( 0e0, 1e0 );
   vector < complex < float > >            i_l;
   vector < complex < float > >            i_k;

   unsigned int                            J_points;
   unsigned int                            Y_points;
   unsigned int                            q_Y_points;

   vector < double >                       I0;
   vector < complex < float > >            A0;

   vector < vector < complex < float > > > Av0;
   vector <  complex < float > >           A1v0;

   // sh data
   // vector < vector < double > >         fib_grid;
   bool                                    use_errors;

   US_Timer                                us_timers;

   double                                  model_fit(
                                                     vector < double > & params, 
                                                     set < pm_point > & model, 
                                                     vector < double > & I_result
                                                     );

   unsigned int                            bytes_per_pm_data;
   unsigned int                            bytes_per_pmc_data;
   unsigned int                            max_mem_in_MB;
   unsigned int                            max_beads_CA;
   unsigned int                            max_beads_CYJ;
   bool                                    use_CYJ;

   bool                                    best_vary_one_param(
                                                               unsigned int        param_to_vary,
                                                               vector < double > & params, 
                                                               set < pm_point >  & model,
                                                               double            & best_fitness
                                                               );

   bool                                    best_vary_one_param(
                                                               unsigned int        param_to_vary,
                                                               vector < double > & params,
                                                               vector < double > & low_fparams,
                                                               vector < double > & high_fparams,
                                                               set < pm_point >  & model,
                                                               double            & best_fitness
                                                               );

   // vary_1 is the inner loop, vary_2 is the outer loop
   bool                                    best_vary_two_param( 
                                                               unsigned int        param_to_vary_1,
                                                               unsigned int        param_to_vary_2,
                                                               vector < double > & params, 
                                                               set < pm_point >  & model,
                                                               double            & best_fitness
                                                               );

   bool                                    best_vary_two_param( 
                                                               unsigned int        param_to_vary_1,
                                                               unsigned int        param_to_vary_2,
                                                               vector < double > & params, 
                                                               vector < double > & low_fparams,
                                                               vector < double > & high_fparams,
                                                               set < pm_point >  & model,
                                                               double            & best_fitness
                                                               );

   double                                  best_delta_start;
   double                                  best_delta_divisor;
   double                                  best_delta_min;
   double                                  best_delta_size_min;
   double                                  best_delta_size_max;

   vector < QString >                      object_names;
   vector < int >                          object_m0_parameters;  // # of params for model_pos = 0
   // doens't seem to work: vector < bool (*)( set < pm_point > & ) >  object_best_f;

   void                                    init_objects();
   
   map < QString, QString >                last_physical_stats;
   QString                                 physical_stats( set < pm_point > & model );

   bool                                    last_best_rmsd_ok;
   double                                  last_best_rmsd2;

 public:
   US_PM               ( 
                        double grid_conversion_factor, 
                        int max_dimension, 
                        double drho, 
                        double buffer_e_density, 
                        double ev, 
                        unsigned int max_harmonics,
                        // unsigned int fibonacci_grid,
                        vector < double > F, 
                        vector < double > q, 
                        vector < double > I, 
                        vector < double > e, 
                        unsigned int max_mem_in_MB   = 2048,
                        int debug_level = 0
                        );

   ~US_PM              ();

   QString             error_msg;
   QString             log;

   int                 debug_level;

   // params = model type, params required for model type, model type etc.

   // build a model from params:
   bool                create_model      ( vector < double > params, set < pm_point > & model, bool only_last_model = false );

   // build a single model and return remaining points
   bool                create_1_model    ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );

   // models themselves:

   bool                sphere            ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );
   bool                cylinder          ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );
   bool                spheroid          ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );
   bool                ellipsoid         ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );
   bool                torus             ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );
   bool                torus_segment     ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );

   // limits

   bool                set_limits        ( vector < double > & params, vector < double > & low_fparams, vector < double > & high_fparams, double max_d = 0e0 );
   bool                check_limits      ( vector < double > & fparams, vector < double > & low_fparams, vector < double > & high_fparams );
   bool                clip_limits       ( vector < double > & fparams, vector < double > & low_fparams, vector < double > & high_fparams );

   // output a bead model
   vector < PDB_atom > bead_model        ( set < pm_point > & model );
   QString             qs_bead_model     ( set < pm_point > & model );

   // compute I of model 
   //      using spherical harmonics cached upon the grid
   //      new "on" points are cached
   bool                compute_I         ( set < pm_point > & model, vector < double > & I_result );
   bool                compute_CYJ_I     ( set < pm_point > & model, vector < double > & I_result );
   bool                compute_CA_I      ( 
                                         set < pm_point > &                        model, 
                                         vector < double > &                       I_result
                                         );
   
   bool                compute_delta_I   ( 
                                          set < pm_point > &                        model, 
                                          set < pm_point > &                        prev_model, 
                                          vector < vector < complex < float > > > & Av,
                                          vector < double > &                       I_result
                                          );

   double              fitness2          ( vector < double > & I_result ); // returns rmsd^2 or chi^2

   static QString      test              ( QString name, QString oname );

   void                debug             ( int level, QString qs );

   set < pm_point >    recenter          ( set < pm_point > & model );
   // bool             is_connected      ( set < pm_point > & model );
   // double           fitness           ( set < pm_point > & model ); // compute_I and compare rmsd

   // split vectors into a "types" list and the searchable double params
   bool                split             ( vector < double > & params, vector < int > & types, vector < double > & fparams );
   // join them back
   bool                join              ( vector < double > & params, vector < int > & types, vector < double > & fparams );

   void                set_best_delta    ( 
                                          double best_delta_start   = 1e0,
                                          double best_delta_divisor = 10e0,
                                          double best_delta_min     = 1e-2
                                          );
   // find best model at base 0
   bool                best_md0          ( 
                                          vector < double > & params, 
                                          set < pm_point >  & model, 
                                          double              finest_conversion      = 1e0,
                                          double              coarse_conversion      = 10e0,
                                          double              refinement_range_pct   = 5,
                                          double              conversion_divisor     = 2e0
                                          );
   bool                best_md0          ( 
                                          vector < double > & params, 
                                          vector < double > & low_fparams, 
                                          vector < double > & high_fparams, 
                                          set < pm_point >  & model 
                                          );

   bool                best_sphere       ( set < pm_point > & model );
   bool                best_cylinder     ( set < pm_point > & model );
   bool                best_spheroid     ( set < pm_point > & model );
   bool                best_ellipsoid    ( set < pm_point > & model );
   bool                best_torus        ( set < pm_point > & model );
   bool                best_torus_segment( set < pm_point > & model );

   bool                best_sphere       (
                                          vector < double > & params,
                                          vector < double > & low_fparams,
                                          vector < double > & high_fparams,
                                          set < pm_point >  & model
                                          );
   bool                best_cylinder     (
                                          vector < double > & params,
                                          vector < double > & low_fparams,
                                          vector < double > & high_fparams,
                                          set < pm_point >  & model
                                          );
   bool                best_spheroid     (
                                          vector < double > & params,
                                          vector < double > & low_fparams,
                                          vector < double > & high_fparams,
                                          set < pm_point >  & model
                                          );
   bool                best_ellipsoid    (
                                          vector < double > & params,
                                          vector < double > & low_fparams,
                                          vector < double > & high_fparams,
                                          set < pm_point >  & model
                                          );
   bool                best_torus        (
                                          vector < double > & params,
                                          vector < double > & low_fparams,
                                          vector < double > & high_fparams,
                                          set < pm_point >  & model
                                          );
   bool                best_torus_segment(
                                          vector < double > & params,
                                          vector < double > & low_fparams,
                                          vector < double > & high_fparams,
                                          set < pm_point >  & model
                                          );

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
   bool                write_model       ( QString filename, set < pm_point > & model );
   QString             tmp_name          ( QString basename, vector < double > & params );

   void                clear             ();

   void                random_md0_params ( vector < double > & params, double max_d = 0e0 );
   bool                zero_md0_params   ( vector < double > & params, double max_d = 0e0 );
   QString             list_params       ( vector < double > & params );
   void                set_grid_size     ( double grid_conversion_factor );

   
};

#endif
