#ifndef US_PM_H
#define US_PM_H

#if defined( USE_MPI )
#  include <mpi.h>
   extern int npes;
   extern int myrank;
#endif

#include <iostream>
#include <math.h>
#include <qstring.h>
#include <qfile.h>
//#include <q3textstream.h>
#include <algorithm>
#include <iterator>
#include <set>
#include <list>
#include <vector>
#include <complex>
#include "us_saxs_util.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_vector.h"
#include "us_json.h"
#include "us_timer.h"
#include "us_sh.h"
#include "us_hydrodyn_pat.h"
#include "../shd_mpi/shs_use.h"

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
#endif

#include "us_pm_global.h"

// so we can try floats or doubles ?
// typedef double us_pm_real;

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif

class US_PM
{
   friend class US_Saxs_Util;

 public:

   enum                                    objects
      {
         SPHERE,
         CYLINDER,
         SPHEROID,
         ELLIPSOID,
         TORUS,
         OBJECTS_FIRST = SPHERE,
         OBJECTS_LAST  = TORUS
         // TOURS_SEGMENT
      };

   QString get_name( vector < int > & types );

 private:

   // stuff for spherical harmonics

   double                                  grid_conversion_factor;
   int                                     max_dimension;
   double                                  max_dimension_d;
   double                                  one_over_grid_conversion_factor;

   //    double                                  drho;
   //    double                                  buffer_e_density;
   //    double                                  ev;

   double                                  org_cube_size;
   double                                  org_conversion_factor;
   double                                  cube_size;
   double                                  bead_radius;
   double                                  bead_radius_over_2gcf;
   int                                     org_max_dimension;

   // active grid:
   vector < double >                       q;
   vector < double >                       I;
   vector < double >                       e;
   vector < double >                       oneoversd2;

   unsigned int                            q_points;

   // precomputed bead structure factors
   // these can come from rayleigh or compute_ff for an atom type
   vector < double >                       org_F;
   // F is rescaled with a set_grid_size
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

   double                                  theta_min;
   double                                  delta_min;

   map < objects, QString >                object_name_map;
   vector < QString >                      object_names;
   vector < int >                          object_m0_parameters;  // # of params for model_pos = 0

                                                            
   enum                                    parameter_type 
      { 
         COORD, 
         NORM, 
         RADIUS, 
         ANGLE 
      };

   vector < vector < vector < parameter_type > > >
                                           object_parameter_types; // i.e. distance 
   map < parameter_type, QString >         object_type_name;

   void                                    init_objects();
   QString                                 list_object_info();
   
   map < QString, QString >                last_physical_stats;
   QString                                 physical_stats( set < pm_point > & model );

   bool                                    last_best_rmsd_ok;
   double                                  last_best_rmsd2;

   list < pm_ga_individual >               ga_pop;
   vector < int >                          ga_types;
   vector < double >                       ga_delta;
   vector < double >                       ga_low_fparams;
   vector < double >                       ga_high_fparams;
   vector < double >                       ga_min_low_fparams;
   vector < double >                       ga_max_high_fparams;
   vector < double >                       ga_I_result;
   vector < double >                       ga_params;
   vector < double >                       ga_fparams;
   vector < unsigned int >                 ga_points;
   unsigned int                            ga_points_max;
   unsigned int                            ga_fparams_size;

   bool                                    ga_state_ok();
   bool                                    ga_compute_delta( unsigned int points );

   // refine the low_fparams, high_fparams based upon top_count population members
   bool                                    ga_refine_limits    ( 
                                                               unsigned int top_count, 
                                                               bool         needs_sort_unique = true,
                                                               unsigned int extend_deltas = 1
                                                               );

   bool                                    ga_delta_to_fparams( vector < int >     & delta, 
                                                                vector < double >  & fparams );
   unsigned int                            ga_pop_selection   ( unsigned int size );
   bool                                    ga_fitness         ( pm_ga_individual & individual );
   void                                    ga_compute_fitness ();

   bool                                    ga                 ( pm_ga_individual & best_individual );

   ga_ctl_param                            ga_p;
   ga_ctl_param                            ga_p_save;

   bool                                    rescale_params( vector < int >    & types, 
                                                           vector < double > & fparams, 
                                                           double              new_conversion_factor );

   bool                                    rescale_params( vector < double > & params,
                                                           vector < double > & low_fparams, 
                                                           vector < double > & high_fparams, 
                                                           double              new_conversion_factor,
                                                           double              refinement_range_pct = 0e0 );

   SHS_USE                               * shs;

#if defined( USE_MPI )
   set < int >                             pm_workers_registered;
   set < int >                             pm_workers_waiting;
   set < int >                             pm_workers_busy;
#endif

   double                                  pm_ga_fitness_secs;
   unsigned int                            pm_ga_fitness_calls;

   void                                    random_normal( 
                                                         double   mean1, 
                                                         double   sd1, 
                                                         double   mean2, 
                                                         double   sd2,
                                                         double & x,
                                                         double & y
                                                         );

   double                                  pm_ga_last_max_best_delta_min;
   bool                                    pm_ga_pegged;
   vector < bool >                         pm_ga_peggedv;
   bool                                    ga_delta_ok;

   bool                                    ga_fparams_to_individual( vector < double > & fparams, pm_ga_individual & individual );
   bool                                    ga_params_to_individual( vector < double > & params, pm_ga_individual & individual );

   vector < vector < double > >            ga_seed_params;

   vector < double >                       last_written_I;

   bool                                    expand_types( 
                                                        vector < vector < int > > & types_vector,
                                                        QString                     types,
                                                        bool                        incrementally,
                                                        bool                        allcombinations 
                                                         );

   bool                                    expand_types( 
                                                        vector < vector < double > > & types_vector,
                                                        QString                        types,
                                                        bool                           incrementally,
                                                        bool                           allcombinations 
                                                         );
                                                           

 public:
   // note: F needs to be the factors for a volume of size grid_conversion_factor ^ 3

   US_PM               ( 
                        double            grid_conversion_factor, 
                        int               max_dimension, 
                        unsigned int      max_harmonics,
                        vector < double > F, 
                        vector < double > q, 
                        vector < double > I, 
                        vector < double > e, 
                        unsigned int      max_mem_in_MB   = 2048,
                        int               debug_level = 0,
                        bool              quiet = false,
                        US_Log          * us_log     = (US_Log *)0,
                        US_Udp_Msg      * us_upd_msg = (US_Udp_Msg *)0
                        );

   ~US_PM              ();

   QString             error_msg;
   QString             msg_log;

   int                 debug_level;

   US_Log            * us_log;
   bool                us_log_started;
   US_Udp_Msg        * us_udp_msg;
   bool                quiet;

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
   // just a vector of class point
   vector < point >    point_model       ( set < pm_point > & model );

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
                                          double              refinement_range_pct   = 5e0,
                                          double              conversion_divisor     = 2e0
                                          );
   bool                best_md0          ( 
                                          vector < double > & params, 
                                          vector < double > & low_fparams, 
                                          vector < double > & high_fparams, 
                                          set < pm_point >  & model 
                                          );

   // quick "approximate" max_d
   bool                approx_max_dimension( 
                                          vector < double > & params, 
                                          double              coarse_conversion,
                                          unsigned int      & approx_max_d
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

   bool                write_model       ( QString & filename, set < pm_point > & model, bool overwrite = true );
   bool                write_model       ( QString & filename, set < pm_point > & model, vector < double > &params, bool overwrite = true );
   bool                qstring_model     ( QString & out, set < pm_point > & model );
   bool                qstring_model     ( QString & out, set < pm_point > & model, vector < double > &params );
   bool                write_I           ( QString & filename, set < pm_point > & model, bool overwrite = true );

   QString             tmp_name          ( QString basename, vector < double > & params );

   void                clear             ( );

   void                random_md0_params ( vector < double > & params, double max_d = 0e0 );
   bool                zero_md0_params   ( vector < double > & params, double max_d = 0e0 );
   bool                zero_params       ( vector < double > & params, vector < int > & types );
   bool                random_params     ( vector < double > & params, vector < int > & types, double max_d = 0e0 );
   QString             list_params       ( vector < double > & params );
   bool                reset_grid_size   ( bool quiet = false );
   bool                set_grid_size     ( double grid_conversion_factor, bool quiet = false );

   bool                grid_search       (
                                          vector < double > & params,
                                          vector < double > & delta,
                                          vector < double > & low_fparams,
                                          vector < double > & high_fparams,
                                          set < pm_point >  & model
                                          );

   double              Rg                ( set < pm_point >  & model );

   // ga

   map < QString, QString > control_parameters;


   bool                    ga_run            ( 
                                              vector < int >    & types, 
                                              pm_ga_individual  & best_individual,
                                              unsigned int        points_max,
                                              vector < double > & low_fparams,
                                              vector < double > & high_fparams
                                              );
                                          
   bool                    ga_run            ( 
                                              vector < int >    & types, 
                                              pm_ga_individual  & best_individual,
                                              unsigned int        points_max  = 100
                                              );

   void                    ga_set_params     (
                                              unsigned int        ga_population        = 100,
                                              unsigned int        ga_generations       = 100,
                                              double              ga_mutate            = 0.1e0,
                                              double              ga_sa_mutate         = 0.5e0,
                                              double              ga_crossover         = 0.4e0,
                                              unsigned int        ga_elitism           = 2,
                                              unsigned int        ga_early_termination = 10
                                              );
   void                    ga_set_params     ( map < QString, QString > control_parameters );

   bool                    best_md0_ga       ( 
                                              vector < double > & params, 
                                              set < pm_point >  & model, 
                                              unsigned int        steps_to_ga            = 1,
                                              unsigned int        points_max             = 100,
                                              double              finest_conversion      = 1e0,
                                              double              coarse_conversion      = 10e0,
                                              double              refinement_range_pct   = 5e0,
                                              double              conversion_divisor     = 2e0
                                              );

   bool                    best_ga           ( 
                                              vector < double > & params, 
                                              set < pm_point >  & model, 
                                              unsigned int        points_max             = 100,
                                              double              finest_conversion      = 1e0,
                                              double              coarse_conversion      = 10e0,
                                              double              refinement_range_pct   = 5e0,
                                              double              conversion_divisor     = 2e0
                                              );

   QString                 ga_info           ();

};

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
