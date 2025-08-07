#ifndef PM_WORKER_H
#define PM_WORKER_H

#define USE_MPI

#if defined(USE_MPI)
#include <mpi.h>
extern int npes;
extern int myrank;
#endif

#include <math.h>

#include <algorithm>
#include <complex>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../include/us_sh.h"
#include "../shd_mpi/shs_use.h"

using namespace std;

#ifdef WIN32
typedef _int16 int16_t;
typedef _int32 int32_t;
typedef unsigned _int16 uint16_t;
typedef unsigned _int32 uint32_t;
#else
#include <stdint.h>
#endif

#if defined(USE_MPI)
#include "../include/us_pm_mpi.h"
#endif

#include "../include/us_pm_global.h"
// so we can try floats or doubles ?
// typedef double us_pm_real;

#ifdef WIN32
#pragma warning(disable : 4251)
#endif

class PM_WORKER {
  friend class US_Saxs_Util;

 private:
  // stuff for spherical harmonics

  double grid_conversion_factor;
  int max_dimension;
  double max_dimension_d;
  double one_over_grid_conversion_factor;

  double org_cube_size;
  double org_conversion_factor;
  double cube_size;
  double bead_radius;
  double bead_radius_over_2gcf;
  int org_max_dimension;

  // active grid:
  vector<double> q;
  vector<double> I;
  vector<double> e;
  vector<double> oneoversd2;

  unsigned int q_points;

  vector<double> org_F;
  vector<double> F;

  bool rotation_matrix(double l, double m, double n, double theta,
                       vector<vector<double> > &rm);
  bool apply_rotation_matrix(vector<vector<double> > &rm, int x, int y, int z,
                             double &newx, double &newy, double &newz);

  map<pm_point, pm_data> pdata;
  map<pm_point, pmc_data> pcdata;

  // sh variables

  unsigned int max_harmonics;
  unsigned int no_harmonics;
  // fib grid is for hydration
  // unsigned int fibonacci_grid;

  // supplementary sh variables

  vector<complex<float> > ccY;
  vector<complex<float> > ccA1v;
  vector<double> ccJ;

  complex<float> Z0;  // ( 0e0, 0e0 );
  complex<float> i_;  // ( 0e0, 1e0 );
  vector<complex<float> > i_l;
  vector<complex<float> > i_k;

  unsigned int J_points;
  unsigned int Y_points;
  unsigned int q_Y_points;

  vector<double> I0;
  vector<complex<float> > A0;

  vector<vector<complex<float> > > Av0;
  vector<complex<float> > A1v0;

  // sh data
  // vector < vector < double > >         fib_grid;
  bool use_errors;

  double model_fit(vector<double> &params, set<pm_point> &model,
                   vector<double> &I_result);

  unsigned int bytes_per_pm_data;
  unsigned int bytes_per_pmc_data;
  unsigned int max_mem_in_MB;
  unsigned int max_beads_CA;
  unsigned int max_beads_CYJ;
  bool use_CYJ;

  double theta_min;
  double delta_min;

  vector<string> object_names;
  vector<int> object_m0_parameters;  // # of params for model_pos = 0

  enum objects {
    SPHERE,
    CYLINDER,
    SPHEROID,
    ELLIPSOID,
    TORUS
    // TOURS_SEGMENT
  };

  enum parameter_type { COORD, NORM, RADIUS, ANGLE };

  vector<vector<vector<parameter_type> > >
      object_parameter_types;  // i.e. distance
  map<parameter_type, string> object_type_name;

  void init_objects();

  SHS_USE *shs;

 public:
  // note: F needs to be the factors for a volume of size grid_conversion_factor
  // ^ 3

  PM_WORKER(double grid_conversion_factor, int max_dimension,
            unsigned int max_harmonics, vector<double> F, vector<double> q,
            vector<double> I, vector<double> e,
            unsigned int max_mem_in_MB = 2048, int debug_level = 0,
            bool quiet = false);

  ~PM_WORKER();

  string error_msg;

  int debug_level;

  // params = model type, params required for model type, model type etc.

  // build a model from params:
  bool create_model(vector<double> params, set<pm_point> &model,
                    bool only_last_model = false);

  // build a single model and return remaining points
  bool create_1_model(int model_pos, vector<double> &params,
                      vector<double> &params_left, set<pm_point> &model);

  // models themselves:

  bool sphere(int model_pos, vector<double> &params,
              vector<double> &params_left, set<pm_point> &model);
  bool cylinder(int model_pos, vector<double> &params,
                vector<double> &params_left, set<pm_point> &model);
  bool spheroid(int model_pos, vector<double> &params,
                vector<double> &params_left, set<pm_point> &model);
  bool ellipsoid(int model_pos, vector<double> &params,
                 vector<double> &params_left, set<pm_point> &model);
  bool torus(int model_pos, vector<double> &params, vector<double> &params_left,
             set<pm_point> &model);
  bool torus_segment(int model_pos, vector<double> &params,
                     vector<double> &params_left, set<pm_point> &model);

  // limits

  bool set_limits(vector<double> &params, vector<double> &low_fparams,
                  vector<double> &high_fparams, double max_d = 0e0);
  bool check_limits(vector<double> &fparams, vector<double> &low_fparams,
                    vector<double> &high_fparams);
  bool clip_limits(vector<double> &fparams, vector<double> &low_fparams,
                   vector<double> &high_fparams);

  // compute I of model
  //      using spherical harmonics cached upon the grid
  //      new "on" points are cached
  bool compute_I(set<pm_point> &model, vector<double> &I_result);
  bool compute_CYJ_I(set<pm_point> &model, vector<double> &I_result);
  bool compute_CA_I(set<pm_point> &model, vector<double> &I_result);

  bool compute_delta_I(set<pm_point> &model, set<pm_point> &prev_model,
                       vector<vector<complex<float> > > &Av,
                       vector<double> &I_result);

  double fitness2(vector<double> &I_result);  // returns rmsd^2 or chi^2

  set<pm_point> recenter(set<pm_point> &model);
  // bool             is_connected      ( set < pm_point > & model );
  // double           fitness           ( set < pm_point > & model ); //
  // compute_I and compare rmsd

  // split vectors into a "types" list and the searchable double params
  bool split(vector<double> &params, vector<int> &types,
             vector<double> &fparams);
  // join them back
  bool join(vector<double> &params, vector<int> &types,
            vector<double> &fparams);

  void clear();

  void reset_grid_size(bool quiet = false);
  void set_grid_size(double grid_conversion_factor, bool quiet = false);
};

#ifdef WIN32
#pragma warning(default : 4251)
#endif

#endif
