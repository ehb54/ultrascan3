//! \file us_astfem_math.h
#ifndef US_ASTFEM_MATH_H
#define US_ASTFEM_MATH_H

#include "us_model.h"
#include "us_simparms.h"
#include "us_extern.h"

//! \brief A group of static mathematical functions to support finite element 
//!        calculations
class US_EXTERN US_AstfemMath
{ 
   public:

      class MfemInitial;
      class MfemScan;
      class MfemData;

      //! Interpolate first onto second
      static void interpolate_C0( MfemInitial&, MfemInitial& );

      //! Interpolate starting concentration QVector mfem_initial onto C0
      static void interpolate_C0( MfemInitial&, double*, QVector< double >& );

      static void initialize_2d( int, int, double*** );
      static void clear_2d     ( int, double** );

      static double maxval( const QVector< double >& );
      static double minval( const QVector< double >& );
      static double maxval( const QVector< US_Model::SimulationComponent >& );
      static double minval( const QVector< US_Model::SimulationComponent >& );
   
      static void   initialize_3d( int, int, int, double**** );
      static void   clear_3d     ( int, int, double*** );
   
      static void   tridiag      ( double*, double*, double*, 
                                   double*, double*, int );

      static double cube_root    ( double, double, double );
      static int    GaussElim    ( int, double**, double* );

      static double find_C1_mono_Nmer( int, double, double );

      static int    interpolate  ( MfemData&, MfemData&, bool );  
      static void   QuadSolver   ( double*, double*, double*, double*, 
                                   double*, double*, int);
   
      static void   IntQT1       ( QVector< double >, double, double,
                                   double**, double );
      static void   IntQTm       ( QVector< double >, double, double,
                                   double**, double );
      static void   IntQTn2      ( QVector< double >, double, double,
                                   double**, double );
      static void   IntQTn1      ( QVector< double >, double, double,
                                   double**, double );
      static void   DefineFkp    ( int, double** );
      static double AreaT        ( QVector< double >&, QVector< double >& );

      static void   BasisTS      ( double, double, double*, double*, double*);
      static void   BasisQS      ( double, double, double*, double*, double*);
   
      static void   BasisTR      ( QVector< double >, QVector< double >,
                                   double, double, double*, double*, double* );
   
      static void   BasisQR      ( QVector< double >, double, double,
                                   double*, double*, double*, double );

      static double Integrand    ( double, double, double, double,
                                   double, double, double, double);

      static void   DefineGaussian( int, double** );

      //! \brief Initial concentration conditions
      class MfemInitial
      {
         public:
         QVector< double > radius;         //!< List of radii
         QVector< double > concentration;  //!< List of concentrations
                                           //!< corresponding to radii
      };

      //! \brief A scan entry
      class MfemScan
      {
         public:
         double            time;        //!< Time of the scan
         double            omega_s_t;   //!< Omega^2 t 
         double            temperature; //!< Temperature at the time of the scan
         double            rpm;         //!< Rotor speed
         QVector< double > conc;        //!< List of concentration values
      };

      //! \brief A data set comprised of scans from one sample
      //!        taken at constant speed
      class MfemData  
      {
         public:
         QString      id;           //!< Description of this dataset
         int          cell;         //!< Cell position in rotor
         int          channel;      //!< Channel number from centerpiece

         //! Single wavelength at which data was acquired (for UV/Vis)
         double       wavelength; 
         double       rpm;          //!< Constant rotor speed
   
         //! The number with which a s20,w value needs
         //! to be multiplied to get the s value in experimental space
         //!  - sT,B = s20,W * s20W_correction
         //!  - sT,B = [s_20,W * [(1-vbar*rho)_T,B * eta_20,W] / 
         //!        [(1-vbar*rho)_20,W * eta_T,B]
   
         /*! 
            \f[
              s_{t,b} = s_{20,w} \frac{ ( 1 - (\bar v \rho)_{t,b} )
                       \eta_{20,w} } {( 1 - (\bar v \rho)_{20,w} ) \eta_{t,b} } 
            \f]
            <div class='blockcenter'>
               where: <br>

               \f$ s \f$ = sedimentation coefficient <br>
               \f$ t \f$ = temperature <br>
               \f$ b \f$ = buffer <br>
               \f$ w \f$ = water <br>
               \f$ \bar v \f$ = average specific volume <br>
               \f$ \rho \f$ = density <br>
               \f$ \eta \f$ = viscosity </div>
         */
   
         double       s20w_correction;  
      
         //! The number with which a D20,w value needs 
         //! to be multiplied to get the s value in experimental space
         //!  - DT,B = D20,W * D20w_correction
         //!  - DT,B = [D20,W * T * eta_20,W] / [293.15 * eta_T,B]
      
         double       D20w_correction;  
         double       viscosity;       //!< viscosity of solvent
         double       density;         //!< density of solvent
         double       vbar;            //!< temperature corrected vbar
         double       avg_temperature; //!< average temperature of all scans
         double       vbar20;          //!< vbar at 20C
         double       meniscus;        //!< radial position of meniscus
         double       bottom;          //!< corrected for speed dependent
                                       //!<  rotor stretch
         QVector< double>    radius;   //!< radial gridpoints
         QVector< MfemScan > scan;     //!< list of scan data
      };
     
      //! \brief Reaction Group
      class ReactionGroup
      {
         public:
         QVector< int > association;
         QVector< int > GroupComponent;
      };

      //! \brief Component Role
      class ComponentRole
      {
         public:
         int            comp_index; // index of this component
         QVector< int > assoc;      // assoc index where this component occurs
         QVector< int > react;      // role of component in each association, 
                                    //  = 1: if as reactant; =-1, if as product
         QVector< int > st;         // stoichiometry of each component in 
                                    // each assoc., index is linked to assoc.
      };

      //! \brief Parameters for finite element solution
      class AstFemParameters
      {
         public:
         int               simpoints;

         QVector< double > s;       //!< sedimentation coefficient
         QVector< double > D;       //!< Diffusion coefficient
         QVector< double > kext;    //!< extinctiom coefficient
         QVector< ComponentRole > role; //!< role of each component
                                        //!<  in various reactions
         double pathlength;       //!< path length of centerpiece;
         double dt;               //!< time step size;
         int    time_steps;       //!< number of time steps for simulation
         double omega_s;          //!< omega^2
         double start_time;       //!< start time in seconds of simulation
                                  //!<  at constant speed
         double current_meniscus; //!< actual meniscus for current speed
         double current_bottom;   //!< actual bottom for current speed
         int    first_speed;      //!< constant speed at first speed step
         int    rg_index;         //!< reaction group index

         //! Local index of each GroupComponent involved in a reaction group
         QVector< int > local_index;  
         
         //! All association rules in a reaction group,
         //!  with components expressed in local index
         QVector< US_Model::Association > association; 
      };

};
#endif
