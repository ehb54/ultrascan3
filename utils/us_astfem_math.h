//! \file us_astfem_math.h
#ifndef US_ASTFEM_MATH_H
#define US_ASTFEM_MATH_H

#include <vector>
using namespace std;

#include "us_femglobal.h"
#include "us_extern.h"

//! \brief Reaction Group
struct ReactionGroup
{
   vector< uint > association;
   vector< uint > GroupComponent;
};

//! \brief Component Role
struct ComponentRole
{
   uint           comp_index; // index of this component
   vector< uint > assoc;      // assoc vector index where this component occurs
   vector< int  > react;      // role of component in each association, 
                                     // = 1: if as reactant; =-1, if as product
   vector< uint > st;         // stoichiometry of each component in 
                                     // each assoc., index is linked to assoc.
};

//! \brief Parameters for finite element solution
struct AstFemParameters
{
   uint             simpoints;

   vector< double > s;       //!< sedimentation coefficient
   vector< double > D;       //!< Diffusion coefficient
   vector< double > kext;    //!< extinctiom coefficient
   vector< struct ComponentRole > role; //!< role of each component in various reactions

   double pathlength;       //!< path length of centerpiece;
   double dt;               //!< time step size;
   uint   time_steps;       //!< number of time steps for simulation
   double omega_s;          //!< omega^2
   double start_time;       //!< start time in seconds of simulation at constant speed
   double current_meniscus; //!< actual meniscus for current speed
   double current_bottom;   //!< actual bottom for current speed
   uint   first_speed;      //!< constant speed at first speed step
   uint   rg_index;         //!< reaction group index

   //! Local index of each GroupComponent involved in a reaction group
   vector< uint > local_index;  
   
   //! All association rules in a reaction group, with comp expressed in local index
   vector< struct Association > association; 
};

//! \brief A group of static mathematical functions to support finite element 
//!        calculations
class US_EXTERN US_AstfemMath
{ 
   public:
   //! Interpolate first onto second
   static void interpolate_C0( struct mfem_initial&, struct mfem_initial& );

   //! Interpolate starting concentration vector mfem_initial onto C0
   static void interpolate_C0( struct mfem_initial&, double*, vector< double >& );

   static void initialize_2d( uint, uint, double*** );
   static void clear_2d     ( uint, double** );

   static double maxval( const vector< double >& );
   static double minval( const vector< double >& );
   static double maxval( const vector< struct SimulationComponent >& );
   static double minval( const vector< struct SimulationComponent >& );
   
   static void   initialize_3d( uint, uint, uint, double**** );
   static void   clear_3d     ( uint, uint, double*** );
   
   static void   tridiag      ( double*, double*, double*, 
                                double*, double*, uint );

   static double cube_root    ( double, double, double );
   static int    GaussElim    ( int, double**, double* );

   static double find_C1_mono_Nmer( int, double, double );

   static int    interpolate  ( struct mfem_data&, struct mfem_data&, bool );  
   static void   QuadSolver   ( double*, double*, double*, double*, 
                                double*, double*, uint);
   
   static void   IntQT1       ( vector< double >, double, double, double**, double );
   static void   IntQTm       ( vector< double >, double, double, double**, double );
   static void   IntQTn2      ( vector< double >, double, double, double**, double );
   static void   IntQTn1      ( vector< double >, double, double, double**, double );
   static void   DefineFkp    ( uint, double** );
   static double AreaT        ( vector< double >&, vector< double >& );

   static void   BasisTS      ( double, double, double*, double*, double*);
   static void   BasisQS      ( double, double, double*, double*, double*);
   
   static void   BasisTR      ( vector< double >, vector< double >, double, double, 
                               double*, double*, double* );
   
   static void   BasisQR      ( vector< double >, double, double, double*, double*, 
                                double*, double );

   static double Integrand    ( double, double, double, double, double, double, 
                                double, double);

   static void   DefineGaussian( uint, double** );
};
#endif
