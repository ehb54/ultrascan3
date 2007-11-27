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
   using namespace std;

// needed for getpid() to randomize with
#ifdef UNIX
#include <unistd.h>
#endif

//UltraScan Includes
#include "us_extern.h"
#include "us_util.h"

//QT Includes
#include <qdatetime.h>


#define isnotanumber(x) ((x) != (x))


#ifndef AVOGADRO
#define AVOGADRO 6.022045e+23
#endif

#ifndef R
#define R 8.314472e+07
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

   struct peptide
   {
      uint r;
      uint l;
      uint k;
      uint a;
      uint v;
      uint e;
      uint g;
      uint n;
      uint i;
      uint j; //Hao (James Nowick)
      uint q;
      uint y;
      uint o; //delta-linked ornithin (James Nowick)
      uint p;
      uint s;
      uint h;
      uint t;
      uint d;
      uint f;
      uint b;
      uint c;
      uint m;
      uint w;
      uint x;
      uint z;
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
   float normal_distribution(float, float, float);
	double erfc(double);

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
   float calc_bottom(vector <struct rotorInfo>,
                    vector <struct centerpieceInfo> cp_list,
                    int /* rotor list index */,
                    int /* centerpiece list index */,
                    int /* channel */,
                    unsigned int /* rpm */);

#endif

