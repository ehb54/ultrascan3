//! \file us_astfem_math.h
#ifndef US_ASTFEM_MATH_H
#define US_ASTFEM_MATH_H

#include "us_femglobal.h"
#include "us_extern.h"

struct ReactionGroup
{
   QList< unsigned int > association;
   QList< unsigned int > GroupComponent;
};

struct ComponentRole
{
   unsigned int          comp_index; // index of this component
   QList< unsigned int > assoc;      // assoc vector index where this component occurs
   QList< int >          react;      // role of component in each association, 
                                     // = 1: if as reactant; =-1, if as product
   QList< unsigned int > st;         // stoichiometry of each component in 
                                     // each assoc., index is linked to assoc.
};

struct AstFemParameters
{
   unsigned int simpoints;

   QList< double > s;       // sedimentation coefficient
   QList< double > D;       // Diffusion coefficient
   QList< double > kext;    // extinctiom coefficient
   QList< struct ComponentRole > role; // role of each component in various reactions

   double pathlength;       // path length of centerpiece;
   double dt;               // time step size;
   uint   time_steps;       // number of time steps for simulation
   double omega_s;          // omega^2
   double start_time;       // start time in seconds of simulation at constant speed
   double current_meniscus; // actual meniscus for current speed
   double current_bottom;   // actual bottom for current speed
   uint   first_speed;      // constant speed at first speed step
   uint   rg_index;         // reaction group index

   // Local index of each GroupComponent involved in a reaction group
   QList< unsigned int > local_index;  
   
   // All association rules in a reaction group, with comp expressed in local index
   QList< struct Association > association; 
};

class US_EXTERN US_AstfemMath
{ 
   public:
   // Interpolate first onto second
   static void interpolate_C0( struct mfem_initial&, struct mfem_initial& );

   // Interpolate starting concentration vector mfem_initial onto C0
   static void interpolate_C0( struct mfem_initial&, double*, QList< double >& );

   static void initialize_2d( uint, uint, double*** );
   static void clear_2d     ( uint, double** );

   static double maxval( const QList< double >& );
   static double minval( const QList< double >& );
   static double maxval( const QList< struct SimulationComponent >& );
   static double minval( const QList< struct SimulationComponent >& );
   
   static void   initialize_3d( uint, uint, uint, double**** );
   static void   clear_3d     ( uint, uint, double*** );
   
   static void   tridiag      ( double*, double*, double*, 
                                double*, double*, uint );

   static double cube_root    ( double, double, double );
   static int    GaussElim    ( int, double**, double* );

   static double find_C1_mono_Nmer( int, double, double );

   static int    interpolate  ( struct mfem_data&, struct mfem_data&, bool );  
   static void   QuadSolver   (double*, double*, double*, double*, 
                               double*, double*, uint);
   
   static void   IntQT1       ( QList< double >, double, double, double**, double );
   static void   IntQTm       ( QList< double >, double, double, double**, double );
   static void   IntQTn2      ( QList< double >, double, double, double**, double );
   static void   IntQTn1      ( QList< double >, double, double, double**, double );
   static void   DefineFkp    ( uint, double** );
   static double AreaT        ( QList< double >&, QList< double >& );

   static void   BasisTS      ( double, double, double*, double*, double*);
   static void   BasisQS      ( double, double, double*, double*, double*);
   
   static void   BasisTR      ( QList< double >, QList< double >, double, double, 
                               double*, double*, double* );
   
   static void   BasisQR      ( QList< double >, double, double, double*, double*, 
                                double*, double );

   static double Integrand    ( double, double, double, double, double, double, 
                                double, double);

   static void  DefineGaussian( uint, double** );
};


#ifdef NEVER
#include <math.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <climits>
#include <qobject.h>

#include "us_extern.h"
#include "us_matrix.h"
#include "us_model_editor.h"
#include "us_simulationparameters.h"
#include "us_stiffbase.h"


using namespace std;




void IntQT1_ellam(vector <double>, double, double, double**, double);
void IntQTm_ellam(vector <double>, double, double, double**, double);
void IntQTn1_ellam(vector <double>, double, double, double**, double);
void QuadSolver_ellam(double *, double *, double *, double *, double *, double *, unsigned int);
double IntConcentration(vector<double>, double *);
void DefInitCond(double **, unsigned int);
void interpolate_Cfinal(struct mfem_initial *, double *, vector <double> *); // interpolate final concentration back onto mfem_initial

int interpolate ( struct mfem_data *,  // simulated solution
                  unsigned int,        // number of scans in expt. data, time dimension
                  unsigned int,        // number of points in expt. data, radius dimension
                  float *,             // scan times
                  double *,            // radius values from expt. data
                  double **);          // concentration values from expt. data, first dimension = time, second dimension radius

#endif
#endif




