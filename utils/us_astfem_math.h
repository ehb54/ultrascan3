//! \file us_astfem_math.h
#ifndef US_ASTFEM_MATH_H
#define US_ASTFEM_MATH_H

#include "us_extern.h"
#include "us_model.h"
#include "us_simparms.h"
#include <unistd.h>

//! \brief A group of static mathematical functions to support finite element 
//!        calculations
class US_UTIL_EXTERN US_AstfemMath
{
   public:

      class MfemInitial;
      class MfemScan;
      class MfemData;

      //! \brief Interpolate first onto second
      //! \param C0    Input MfemInitial
      //! \param C1    MfemInitial with interpolated concentrations
      static void interpolate_C0( MfemInitial&, MfemInitial& );

      //! \brief Interpolate starting concentration QVector MfemInitial onto C0
      //! \param C0    Input MfemInitial
      //! \param C1    First scan with interpolated concentrations
      //! \param xvec  Start x (radius) values for each scan
      static void interpolate_C0( MfemInitial&, double*, QVector< double >& );

      //! \brief Create a timestate file from speed_step and scan information
      //! \param tmst_fpath  Full path to timestate file to examine
      //! \param simparams   Simulation parameters, including speed steps info
      //! \param sim_data    Raw data with scans to examine
      //! \returns Flag:  0 if the file write succeeded
      static int  writetimestate( const QString&, US_SimulationParameters&,
                                  US_DataIO::RawData& );

      //! \brief Determine if first time step's acceleration is too low
      //! \param speedsteps  Vector of speed steps
      //! \param min_accel   Minimum reasonable acceleration value
      //! \param rate        Returned computed first-step acceleration rate
      //! \returns Flag:     True iff acceleration rate is too low
      static bool low_acceleration( const QVector< US_SimulationParameters::SpeedProfile >&,
                                    const double, double& );

      //! \brief Determine if a timestate file holds one-second-interval records
      //! \param tmst_fpath  Full path to timestate file to examine
      //! \param sim_data    Raw data with scans to examine
      //! \returns Flag:  true iff timestate holds values at one-second interval
      static bool timestate_onesec( const QString&, US_DataIO::RawData& );

      //! \brief Initialize a 2d matrix in memory to all zeros.
      //! \param val1   First dimension
      //! \param val2   Second dimension
      //! \param matrix Initialized val1 x val2 matrix
      static void zero_2d      ( int, int, double** );

      //! \brief Create a 2d matrix in memory and initilize to all zeros.
      //! \param val1   First dimension
      //! \param val2   Second dimension
      //! \param matrix Initialized val1 x val2 matrix
      static void initialize_2d( int, int, double*** );

      //! \brief Delete a 2d matrix in memory
      //! \param val1   First dimension
      //! \param matrix val1 x n matrix to be deleted
      static void clear_2d     ( int, double** );

      //! \brief Find the maximum value in a vector
      //! \param value Vector whose maximum is found
      //! \returns Vector maximum value
      static double maxval( const QVector< double >& );

      //! \brief Find the minimum value in a vector
      //! \param value Vector whose minimum is found
      //! \returns Vector minimum value
      static double minval( const QVector< double >& );

      //! \brief Find the maximum s in a vector of SimulationComponent entries
      //! \param value Vector of components whose maximum s value is found
      //! \returns Vector maximum value
      static double maxval( const QVector< US_Model::SimulationComponent >& );

      //! \brief Find the minimum s in a vector of SimulationComponent entries
      //! \param value Vector of components whose minimum s value is found
      //! \returns Vector minimum value
      static double minval( const QVector< US_Model::SimulationComponent >& );

      //! \brief Create a 3d matrix in memory and initilize to all zeros.
      //! \param val1   First dimension
      //! \param val2   Second dimension
      //! \param val3   Third dimension
      //! \param matrix Initialized val1 x val2 x val3 matrix
      static void   initialize_3d( int, int, int, double**** );
      
      //! \brief Delete a 3d matrix in memory
      //! \param val1   First dimension
      //! \param val2   Second dimension
      //! \param matrix Deleted val1 x val2 x n matrix
      static void   clear_3d     ( int, int, double*** );
   
      //! \brief Solve a Ax = b where A is tridiagonal
      //! \param a  Array of a values
      //! \param b  Array of b values
      //! \param c  Array of c values
      //! \param r  Array of r values
      //! \param u  Array of u values
      //! \param N  Length of vectors
      static void   tridiag      ( double*, double*, double*, 
                                   double*, double*, int );

      //! \brief Find the positive cubic-root of a cubic polynomial<br>
      //! with a0 <= 0 and<br>
      //! a1, a2 >= 0
      //! \param a0 The a0 value
      //! \param a1 The a0 value
      //! \param a2 The a0 value
      //! \returns Cube root
      static double cube_root    ( double, double, double );

      //! \brief Solve Ax = b using Gaussian Elimination
      //! \param n  The order of the square matrix
      //! \param a  The n x n matrix
      //! \param b  The b vector
      //! \returns Success flag: -1 -> singular, no solution; 1 -> success
      static int    GaussElim    ( int, double**, double* );

      //! \brief Solve  f(x) = x + K * x^n - C using Newton's method.<br>
      //! This function needs to be renamed!
      //! \param n  The power exponent
      //! \param K  The scalar constant
      //! \param CT The test C value
      static double find_C1_mono_Nmer( int, double, double );

      //! \brief Interpolate one dataset onto another using time or omega^2t
      //! \param expdata  Experimental data to create, sized on input
      //! \param simdata  Simulation from which to create modeled experiment
      //! \param use_time Flag of whether to use time interpolation
      //! \returns Success flag: 0 -> success
      static int    interpolate  ( MfemData&, MfemData&, bool );  

      //! \brief Interpolate one dataset onto another using time or omega^2t
      //! \param expdata  Experimental data to create, sized on input
      //! \param simdata  Simulation from which to create modeled experiment
      //! \param use_time Flag of whether to use time correction
      //! \param fscan    First update expdata scan index
      //! \param lscan    Last update expdata scan index plus one
      //! \returns Success flag: 0 -> success
      static int    interpolate  ( MfemData&, MfemData&, bool, int, int );  

      //! \brief Solve Quad-diagonal system
      //! \param ai   The initial a vector
      //! \param bi   The initial b vector
      //! \param ci   The initial c vector
      //! \param di   The initial d vector
      //! \param cr   The Cr        vector
      //! \param solu The calculated solution vector
      //! \param N    The length of vectors
      static void   QuadSolver   ( double*, double*, double*, double*, 
                                   double*, double*, int);
   
      //! \brief Integration on test function seperately on left Q, right T
      //! \param vx   The vx vector
      //! \param D    The D   value
      //! \param sw2  The sw2 value
      //! \param Stif The Stif matrix
      //! \param dt   The dt value
      static void   IntQT1       ( double* , double, double,
                                   double**, double );
   
      //! \brief Integration on test function
      //! \param vx   The vx vector
      //! \param D    The D   value
      //! \param sw2  The sw2 value
      //! \param Stif The Stif matrix
      //! \param dt   The dt value
      static void   IntQTm       ( double* , double, double,
                                   double**, double );
   
      //! \brief Integration on test function
      //! \param vx   The vx vector
      //! \param D    The D   value
      //! \param sw2  The sw2 value
      //! \param Stif The Stif matrix
      //! \param dt   The dt value
      static void   IntQTn2      ( double* , double, double,
                                   double**, double );
   
      //! \brief Integration on test function
      //! \param vx   The vx vector
      //! \param D    The D   value
      //! \param sw2  The sw2 value
      //! \param Stif The Stif matrix
      //! \param dt   The dt value
      static void   IntQTn1      ( double* , double, double,
                                   double**, double );
      //! \brief Define Lamm equation values
      //! \param npts  Order of the equation
      //! \param Lam   Matrix of Lamm values to fill
      static void   DefineFkp    ( int, double** );

      //! \brief Compute the area of a triangle (v1, v2, v3)
      //! \param xv The XV vector
      //! \param yv The YV vector
      static double AreaT        ( double*, double* );

      //! \brief Computer basis on standard element (TS)
      //! \param xi    The XI constant
      //! \param et    The ET constant
      //! \param phi   The Phi  vector to fill
      //! \param phi1  The Phi1 vector to fill
      //! \param phi2  The Phi2 vector to fill
      static void   BasisTS      ( double, double, double*, double*, double*);

      //! \brief Computer basis on standard element (QS)
      //! \param xi    The XI constant
      //! \param et    The ET constant
      //! \param phi   The Phi  vector to fill
      //! \param phi1  The Phi1 vector to fill
      //! \param phi2  The Phi2 vector to fill
      static void   BasisQS      ( double, double, double*, double*, double*);
   
      //! \brief Computer basis on real element T at given(xs,ts) point
      //! \param vx   The Vx vector
      //! \param vy   The Vy vector
      //! \param xs   The start X coordinate
      //! \param ys   The start Y coordinate
      //! \param phi  The Phi  vector to fill
      //! \param phix The PhiX vector to fill
      //! \param phiy The PhiY vector to fill
      static void   BasisTR      ( double*, double*,
                                   double, double, double*, double*, double* );
   
      //! \brief Computer basis on real element Q at given(xs,ts) point
      //! \param vx   The VX vector
      //! \param xs   The start X coordinate
      //! \param ts   The start Y coordinate
      //! \param phi  The Phi  vector to fill
      //! \param phix The PhiX vector to fill
      //! \param phiy The PhiY vector to fill
      //! \param dt   The d-t constant
      static void   BasisQR      ( double*, double, double,
                                   double*, double*, double*, double );

      //! \brief Integrand for Lamm equation
      //! \param x_gauss The X-gaussian constant
      //! \param D       The D   constant
      //! \param sw2     The SW2 constant
      //! \param u       The U   constant
      //! \param ux      The Ux  constant
      //! \param ut      The Ut  constant
      //! \param v       The V   constant
      //! \param vx      The Vx  constant
      static double Integrand    ( double, double, double, double,
                                   double, double, double, double);

      //! \brief Define Gaussian
      //! \param nGauss Order of the matrix
      //! \param Gs2    Matrix to be filled
      static void   DefineGaussian( int, double** );

      //! \brief Initialize simulation data from experimental
      //! \param simdata  Reference to simulation Raw Data to initialize.
      //! \param editdata Reference to experimental Edited Data to mirror.
      //! \param concval1 Optional constant concentration value for first scan.
      static void   initSimData   ( US_DataIO::RawData&,
                                    US_DataIO::EditedData&, double = 0.0 );

      //! \brief Initialize global simulation data from experimental list
      //! \param simdata  Reference to simulation Raw Data to initialize.
      //! \param edats    Reference to experimental Edited Data list to mirror.
      //! \param concval1 Optional constant concentration value for first scan.
      static void   initSimData   ( US_DataIO::RawData&,
                                    QList< US_DataIO::EditedData* >&,
                                    double = 0.0 );

      //! \brief Calculate variance for Simulation-Experimental difference.
      //! \param simdata  Reference to simulation Raw Data.
      //! \param editdata Reference to experimental Edited Data.
      //! \returns        The variance (average of differences squared) between
      //!                 the simulated and experimental readings values.
      static double variance      ( US_DataIO::RawData&,
                                    US_DataIO::EditedData& );

      //! \brief Calculate variance for Simulation-Experimental difference.
      //! \param simdata   Reference to simulation Raw Data.
      //! \param editdata  Reference to experimental Edited Data.
      //! \param exclScans List of excluded scans.
      //! \returns         The variance (average of differences squared) between
      //!                  the simulated and experimental readings values.
      static double variance      ( US_DataIO::RawData&,
                                    US_DataIO::EditedData&,
                                    QList< int > );

      //! \brief Calculate bottom radius from channel bottom
      //! and rotor coefficients
      //! \param rpm          Rotor revolutions per minute.
      //! \param bottom_chan  Initial bottom for centerpiece channel
      //! \param rotorcoefs   Array of 2 rotor coefficients
      //! \returns            The calculated bottom radius value.
      static double calc_bottom   ( double, double, double* );

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
         QVector< int > association;    //!< association flags 
         QVector< int > GroupComponent; //!< group component flags
      };

      //! \brief Component Role
      class ComponentRole
      {
         public:
         int            comp_index; //!< index of this component
         QVector< int > assocs;     //!< assoc indexes where component occurs
         QVector< int > stoichs;    //!< stoichiometry of each component in 
                                    //!< each assoc., index is linked to assoc.;
                                    //!< sign: +ve.->reactant, -ve.->product.
      };

      //! \brief Parameters for finite element solution
      class AstFemParameters
      {
         public:
         int    simpoints;        //!< number of datapoints for the simulation

         QVector< double > s;     //!< sedimentation coefficient
         QVector< double > D;     //!< Diffusion coefficient
         QVector< double > kext;  //!< extinction coefficient
         QVector< ComponentRole > role; //!< role of each component
                                        //!<  in various reactions
         double pathlength;       //!< path length of centerpiece;
         double dt;               //!< time step size;
         int    time_steps;       //!< number of time steps for simulation
         double omega_s;          //!< omega^2
         double start_time;       //!< start time in seconds of simulation
                                  //!<  at constant speed
         double start_om2t;       //!< start omega^2t of simulation
         double current_meniscus; //!< actual meniscus for current speed
         double current_bottom;   //!< actual bottom for current speed
         double bottom_pos;       //!< centerpiece bottom (possibly adjusted)
         int    cdset_speed;      //!< constant speed for current dataset
         int    rg_index;         //!< reaction group index

         //! Local index of each GroupComponent involved in a reaction group
         QVector< int > local_index;  
         
         //! All association rules in a reaction group,
         //!  with components expressed in local index
         QVector< US_Model::Association > association; 
      };

};
#endif
