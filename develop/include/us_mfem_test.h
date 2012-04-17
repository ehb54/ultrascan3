#ifndef US_MFEM_H
#define US_MFEM_H

// c++ class for US_MovingFEM
// Written by E. Brookes 2004. ebrookes@cs.utsa.edu
// adapted for UltraScan by Borries Demeler (2004)

#include <vector>
#include <iostream>
#include <qapp.h>
#include <qobject.h>
#include <string.h>

#include "us_femglobal.h"

class US_MovingFEM
{
 public:

    US_MovingFEM(struct mfem_data *, bool);
    US_MovingFEM(int, double, double, double, double, double, double, double, 
       struct mfem_data *, bool, struct mfem_initial *);
    ~US_MovingFEM();

      void set_params(int, double, double, double, double, double, double, double, struct mfem_initial *);

      struct mfem_data *data;
      struct mfem_initial *initCvector;
      
      int mfem_N;          // the calculated N >= N
      int mfem_steps;      // the calculated total timesteps (total_t/dt + 1)
      double mfem_s;       // the previous runs' s
      double mfem_D;       // the previous runs' D
      double mfem_rpm;     // the previous runs' rpm
      double mfem_dt;      // the previous runs' dt

      double *mfem_t;      // the time of the time steps mfem_t[mfem_steps];
      double *mfem_Mass;   // the mass at the time steps mfem_Mass[mfem_steps];
      double *mfem_x;      // the x grid doubles          mfem_x[mfem_steps][mfem_N];
      double *mfem_c;      // the concentration doubles   mfem_c[mfem_steps][mfem_N];
      bool stop_now;
      bool error_flag;
      bool skipEvents;
      
      void set_N(int N);
      void set_s(double);
      void set_D(double);
      void set_rpm(double);
      void set_total_t(double);
      void set_m(double);
      void set_b(double);
      void set_c0(double);
      void stop();

      int get_N(void);
      double get_s(void);
      double get_D(void);
      double get_rpm(void);
      double get_total_t(void);
      double get_m(void);
      double get_b(void);
      double get_c0(void);
      int run();

      int filewrite(char *);
      void fprintparams(FILE *);
      void fprintparams(FILE *, int);
      void fprintinitparams(FILE *, int);
      void fprinterror(FILE *, const char *);
      
      // interpolate maps simulated grid  with a variable delta_r grid onto a 
      // fixed delta_r grid from experimental data, and also interpolates time
      int interpolate (struct mfem_data *, /* simulated solution */
                       unsigned int,       /* number of scans in expt. data, time dimension */
                       unsigned int,       /* number of points in expt. data, radius dimension */
                       float *,            /* scan times */
                       double *,           /* radius values from expt. data */
                       double **);          /* concentration values from expt. data, first dimension = time, second dimension radius */
      int interpolate (struct mfem_data *,  /* simulated solution */
                       struct mfem_data *); /* experimental solution */

      // these are the doubles calculated durning the last run


   private:

      bool GUI;
      int N;
      double s;
      double D;
      double rpm;
      double total_t;
      double m;
      double b;
      double c0;

      int sufficient_params;
      char LastErrorMessage[256];
      int LastErrorNumber;


      void free_data();
      void zero_data();
      int mfem();
      double IntConcentration(double *, int, double *);
      int tridiag(int, double *, double *, double *);
      
};

extern long used_composite_grid;
extern long used_schuck_grid;

#endif

