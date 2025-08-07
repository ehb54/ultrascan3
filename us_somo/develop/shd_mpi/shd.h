#ifndef SHD_H
#define SHD_H

#include <math.h>
#include <mpi.h>
#include <stdint.h>

#include <algorithm>
#include <complex>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include "shd_global.h"
#include "shd_sh.h"
#include "shs_use.h"

extern int world_rank;
extern int world_size;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_4PI
#define M_4PI (4e0 * 3.14159265358979323846)
#endif

struct shd_point {
  float x[3];
  int16_t ff_type;
};

struct shd_data {
  float rtp[3];
  vector<complex<float> > A1v;
};

struct shd_input_data {
  uint32_t max_harmonics;
  uint32_t model_size;
  uint32_t q_size;
  uint32_t F_size;
};

// binary file contains (in order)
// struct shd_input_data
// vector < double > F( q_size * max_f_types )
// vector < double > q( q_size )
// vector < shd_data > model_size

/*!
 * \class SHD
 * \brief SHD computes the Debye function using spherical harmonics
 * \author Emre Brookes
 * \version 0.1
 * \date April 2013
 * \copyright BSD
 */

class SHD {
 private:
  vector<shd_point> *model;
  vector<vector<shd_double> > F;
  vector<shd_double> q;
  vector<shd_double> I;

  unsigned int max_harmonics;
  unsigned int no_harmonics;

  vector<complex<float> > ccY;
  vector<complex<float> > ccA1v;
  vector<shd_double> ccJ;

  complex<float> i_;  // ( 0e0, 1e0 );
  vector<complex<float> > i_l;
  vector<complex<float> > i_k;

  unsigned int J_points;

  int debug_level;

  // void                debug             ( int level, string str );

  SHS_USE *shs;

 public:
  //
  string error_msg;

  /*!
   * \brief SHD constructor
   * \param max_harmonics is the maximum number of harmonics used
   * \param model is vector of shd_points which contain the coordinates and the
   * precomputed structure factors they must be precomputed for each type in the
   * model they also must already have any excluded volume subtraction computed
   * \param q     is a vector of points determining the resulting grid for I
   * \param I              is a vector of intensities returned
   * \param debug_level    zero provides no debugging
   */

  SHD(unsigned int max_harmonics, vector<shd_point> &model,
      vector<vector<shd_double> > &F, vector<shd_double> &q,
      vector<shd_double> &I, int debug_level = 0);

  /*!
   * \brief SHD Destructor
   */
  ~SHD();

  /*!
   * \brief compute_amplitudes
   * \param model is the vector of model points
   */

  bool compute_amplitudes(vector<complex<float> > &Av);

  /*!
   * \brief A1v0 is the zero vector of complex floats
   */
  vector<complex<float> > A1v0;

  /*!
   * \brief q_points contains the number of q grid points
   */
  unsigned int q_points;

  /*!
   * \brief Y_points contains the number of harmonic points
   */
  unsigned int Y_points;

  /*!
   * \brief q_Y_points contains the number of complex points
   */
  unsigned int q_Y_points;

  void printmodel();
  void printF();
  void printq();
  void printA(vector<complex<float> > &Av);
};

#endif  // SHD
