#ifndef US_MATH_H
#define US_MATH_H

/******************************************************************************
  Copyright (c) 2002 by Turku PET Centre

  nnls.h

  Version:
  2002-08-19 Vesa Oikonen

******************************************************************************/
int nnls(double *a, int a_dim1, int m, int n, double *b, double *x,
          double *rnorm, double *w, double *zz, int *index);
/*****************************************************************************/


#include <math.h>
#include <stdlib.h>
#include <vector>

// needed for getpid() to randomize with
#ifdef UNIX
#include <unistd.h>
#endif

//UltraScan Includes
#include "us_extern.h"
#include "us_util.h"

//QT Includes
#include <qdatetime.h>

// For some reson WIN32 is not picking up M_PI from math.h...

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#define isnotanumber(x) ((x) != (x))


#ifndef AVOGADRO
#define AVOGADRO 6.022140857e+23
#endif

#ifndef Rbar
#define Rbar 8.314472e+07
#endif

#ifndef K0
#define K0 273.15
#endif

#ifndef K20
#define K20 293.15
#endif

#ifndef VISC_20W
#define VISC_20W 0.0100194
#endif

#ifndef DENS_20W
#define DENS_20W 0.998234
#endif

//ADOL-C includes:
#ifdef ADOLC
#include <adouble.h>
#include <interfaces.h>
#include <drivers.h>
#endif


#include "us_matrix.h"
   using namespace std;

   struct peptide
   {
      uint a;
      uint b;
      uint c;
      uint d;
      uint e;
      uint f;
      uint g;
      uint h;
      uint i;
      uint j; //Hao (James Nowick)
      uint k;
      uint l;
      uint m;
      uint n;
      uint o; //delta-linked ornithin (James Nowick)
      uint p;
      uint q;
      uint r;
      uint s;
      uint t;
      uint u; // currently unused
      uint v;
      uint w;
      uint x;
      uint y;
      uint z;
      uint dab; // diaminobutyric acid (John Kulp), symbol: "+"
      uint dpr; // diaminopropanoic acid (John Kulp), symbol: "@"
      float vbar;
      float vbar20;
      float mw;
      float vbar_sum;
      float weight;
      float e280;
      uint residues;
   };


   unsigned long square(int);
   float square(float);
   double square (double);

   double linefit(double **x, double **y, double *slope, double *intercept, double *sigma,
                 double *correlation, int arraysize);

   float linefit(float **x, float **y, float *slope, float *intercept, float *sigma,
                float *correlation, int arraysize);

   int find_indexf(double, double **, int);
   int find_indexf(float, float **, int);
   int find_indexf(int, int **, int);
   int find_indexr(double, double **, int);
   int find_indexr(float, float **, int);
   int find_indexr(int, int **, int);
/*
   int max (int, int);
   unsigned int max (unsigned int, unsigned int);
   float US_EXTERN max (float, float);
   double max (double, double);

   int min (int, int);
   unsigned int min (unsigned int, unsigned int);
   float min (float, float);
   double min (double, double);
*/
   float US_EXTERN box_muller(float, float);
   int us_randomize();
   float ranf();
   float random_range(float, float);
   float standard_distribution(float);
   float us_normal_distribution(float, float, float);
   double us_erfc(double);

   void get_1d_limits(double **, double *, double *, int, int start_count=0);
   void get_1d_limits(float **, float *, float *, int, int start_count=0);
   void get_1d_limits(int **, int *, int *, int, int start_count=0);
   void get_2d_limits(double ***, double *, double *, int, int, int start_count1=0, int start_count2=0);
   void get_2d_limits(float ***, float *, float *, int, int, int start_count1=0, int start_count2=0);
   void get_2d_limits(int ***, int *, int *, int, int, int start_count1=0, int start_count2=0);

//Take an array of floats and returns it smoothed by a gaussian kernel with with width of smoothing level:
   void US_EXTERN gaussian_smoothing(float **, unsigned int /*smoothing level*/, unsigned int /*number of datapoints*/);

   double US_EXTERN inverse_error_function(double, double);
   void calc_vbar(struct peptide *, QString *, float *);
   float adjust_vbar20(float vbar20, float temperature);
   float adjust_vbar(float vbar, float temperature);
   float calc_buoyancy_tb(float, float, float);
   float calc_bottom(int /* rotor */, int /* centerpiece */, 
                     int /* channel */, unsigned int /* rpm */);
   float calc_bottom(vector <struct rotorInfo>,
                     vector <struct centerpieceInfo> cp_list,
                     int /* rotor list index */,
                     int /* centerpiece list index */,
                     int /* channel */,
                     unsigned int /* rpm */);
   double stretch(int /*rotor id*/, unsigned int /*rotor speed*/);
   double stretch_with_rotor_list(int rotor, unsigned int rpm, vector < rotorInfo > *rotor_list);

#if QT_VERSION < 0x040000 && defined(WIN32)
  /* The following was derived from glibc source for IA32 architecture. */
int __fpclassifyf (float x);

  /* All floating-point numbers can be put in one of these categories.  */
  enum
  {
     FP_NAN,
# define FP_NAN FP_NAN
     FP_INFINITE,
# define FP_INFINITE FP_INFINITE
     FP_ZERO,
# define FP_ZERO FP_ZERO
     FP_SUBNORMAL,
# define FP_SUBNORMAL FP_SUBNORMAL
     FP_NORMAL
# define FP_NORMAL FP_NORMAL
  };

# define isnormal(x) ( __fpclassifyf (x) == FP_NORMAL)

#endif // WIN32

float int_vol_2sphere(float r1, float r2, float d);

QString us_double_decimal_places( double x, int dp );

#endif

