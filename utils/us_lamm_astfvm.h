//! \file us_lamm_astfvm.h
#ifndef US_LAMMASTFVM_H
#define US_LAMMASTFVM_H

#include <QtCore>
#include "us_extern.h"
#include "us_model.h"
#include "us_simparms.h"
#include "us_buffer.h"
#include "us_dataIO.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_math_bf.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug() //!< debug-level-conditioned qDebug()
#endif

//! \brief Module to calculate simulation data by AST Finite Volume Method
class US_UTIL_EXTERN US_LammAstfvm : public QObject
{
   Q_OBJECT

   public:

      //! \brief Finite volume mesh for ASTFVM solution
      class Mesh 
      {
         public: 
   
            //! \brief Changeable non-ideal mesh
            //! \param xl    Left X (radius) value
            //! \param xr    Right X (radius) value
            //! \param Nelem Number of elements
            //! \param Opt   Mesh option (0 for uniform)
            Mesh( double, double, int, int );

            //! \brief Destroy mesh
            ~Mesh();

            //! \brief Initialize mesh
            //! \param s  Sedimentation coefficient
            //! \param D  Diffusion coefficient
            //! \param w2 Omega squared
            void InitMesh( double, double, double );

            //! \brief Refine mesh
            //! \param u0     Current concentration array
            //! \param u1     Next concentration array
            //! \param ErrTol Error tolerance
            void RefineMesh( double*, double*, double );

            int     Nv;       //!< Number of grids
            int     Ne;       //!< Number of elements
            double* x;        //!< radius coordinates of grids

         private:
            int     dbg_level;       // debug level
            int     MaxRefLev;
            int     MonScale;
            double  MonCutoff;
            double  SmoothingWt;
            int     SmoothingCyl;

            int*    Eid;      // elem id
            int*    RefLev;   // refinement level of an elem
            double* MeshDen;  // desired mesh density
            int*    Mark;     // ref/unref marker

            // private functions
            void ComputeMeshDen_D3( const double*, const double* );
            static void Smoothing( int, double*, double, int );
            void Unrefine(  double );
            void Refine(    double );
      };

      //! \brief data for co-sedimenting
      class US_UTIL_EXTERN CosedData
      {
         public:
            //! \brief Create cosed data
            //! \param amodel    Model with all components to simulate
            //! \param asparms   Simulation parameters for all components
            //! \param asim_data Simulation AUC data for all components
            CosedData( const US_Model&, US_SimulationParameters,
                      US_DataIO::RawData*, QList<US_CosedComponent>*, double&, double& );

            //! \brief Default constructor for empty CosedData
            CosedData();

            //! \brief Initialize time & concentration arrays for a component
            void initCosed(double*, double*, double*);

            //! \brief Interpolate concentrations of cosed
            //! \param N     Number of elements in arrays
            //! \param x     X (radius) array
            //! \param t     Current time value
            //! \param Denscosed Density of cosed buffer for current time
            //! \param Visccosed Viscosity of cosed buffer for current time
            void InterpolateCCosed( int, const double*, double, double*, double*);

            US_DataIO::RawData      sa_data;   //!< cosed data 1-component
                                               //!<  simulation for co-sed
            QMap<QString, US_DataIO::RawData> cosed_comp_data;
            QList<US_CosedComponent>* cosed_components;
            US_Model                model;     // cosed data co-sed model
            US_SimulationParameters simparms;  // cosed simulation parameters
            bool is_empty;
         private:



            int count;
            int     Nx;       // number of points in radial direction
            int     Nt;       // number of points in time direction
            int     dbg_level;          // debug level
            double dens;
            double visc;
      };
      CosedData* saltdata;      // data handle for cosedimenting

      US_Math_BF::Band_Forming_Gradient* bandFormingGradient;
      QMap<QString, US_DataIO::RawData> cosed_comp_data;
      //! \brief Create Lamm equations AST Finite Volume Method solver
      //! \param rmodel     Reference to model on which solution is based
      //! \param rsimparms  Reference to simulation parameters
      //! \param parent     Parent object (may be 0)
      US_LammAstfvm( US_Model&, US_SimulationParameters&, QObject* = nullptr );


      //! \brief Main method to calculate FVM solution
      //! \param sim_data Reference to simulated AUC data to produce
      //! \returns        Non-zero if multiple non-ideal conditions
      int  calculate( US_DataIO::RawData& );

      //! \brief Set buffer parameters: density, compressibility
      //! \param buffer Buffer object to examine
      //! \param bfg  pointer to Band forming Gradient from previous calculations (may be 0)
      //! \param csD  pointer to cosed Data from previous calculations (may be 0)
      void set_buffer( US_Buffer , US_Math_BF::Band_Forming_Gradient* = nullptr, CosedData* = nullptr);

      //! \brief Calculate solution for a model component
      //! \param compx Index to model component to use in solution pass
      //! \returns     Non-zero if multiple non-ideal conditions
      int  solve_component( int ); 

      //! \brief Set the mesh speed factor: 1.0 (moving) or 0.0 (non-moving)
      //! \param speed   Mesh speed factor of 1.0 or 0.0
      void SetMeshSpeedFactor( double );

      //! \brief Set the mesh refinement option:  1 to refine, 0 to not refine
      //! \param Opt     Mesh refinement option flag: 1/0 for yes/no.
      void SetMeshRefineOpt( int );

      //! \brief Set stop flag
      //! \param flag    Flag for whether to stop
      void setStopFlag ( bool );

      //! \brief validate band forming gradient and recalculate if needed
      void validate_bfg( void );

      //! \brief validate cosedimenting components and recalculate if needed
      void validate_csd( void );

      //! \brief Set a flag for whether to emit signals for movie mode.
      //! \param flag    Flag for whether or not to operate in show-movie mode.
      void setMovieFlag( bool );

   signals:
      //! \brief Signal calculation start and give maximum steps
      //! \param nsteps Number of expected total calculation progress steps
      void calc_start( int nsteps );

      //! \brief Signal calculation progress, giving running step count
      //! \param istep Current progress step count
      void calc_progress( int istep );

      //! \brief Signal that calculations are complete
      void calc_done( );

      //! \brief Signal component progress, giving running component number
      //! \param icomp Current component begun (1,...)
      void comp_progress( int icomp );

      //! \brief Signal that calculation step is complete.
      //! The connected slot receives the radius vector and concentration
      //! array from loops in calculate functions.
      void new_scan     ( QVector< double >*, double* );

      //! \brief Signal component progress, giving running component number
      //! The connected slot receives the scan time value from calc. functions.a
      void new_time     ( double );

      //! \brief Signal that a calculate_ni()/calculate_ra2() step is complete.
      //! The connected slot receives the integer step count from loops in
      //! functions calculate_ni() and calculate_ra2().
      void current_speed    ( int    );



   private:

      US_Model&                 model;       // input model
      US_SimulationParameters&  simparams;   // input simulation parameters
      US_DataIO::RawData*       auc_data;    // input/output AUC data

      US_AstfemMath::MfemData   af_data;     // internal data

//      Mesh*   msh;             // radial grid

      int     NonIdealCaseNo;  // non-ideal case number
                               // = 0 : ideal, constant s, D
                               // = 1 : concentration dependent
                               // = 2 : co-sedimenting
                               // = 3 : compressibility

      double  sigma;           // constant for concentration dependence (s)
      double  delta;           // constant for concentration dependence (D)
                               // non-ideal case
                               // s = s_0/(1+sigma*C), D=D_0/(1+delta*C)

      double  density;         // buffer density
      double  viscosity;       // buffer viscosity
      double  compressib;      // factor for compressibility
      bool    manual;          // buffer manual
      bool    cosed_needed;
      bool    codiff_needed;
      double  vbar_salt;       // vbar of the salt
      QList<US_CosedComponent> cosed_components;



      double  MeshSpeedFactor; // = 1: mesh following sedimentation
                               // = 0: fixed mesh in each time step

      int     MeshRefineOpt;   // = 1: perform mesh local refinement
                               // = 0: no mesh refinement

      int     comp_x;          // current component index
      int     dbg_level;       // debug level

      bool    stopFlag;        // flag to stop processing
      bool    movieFlag;       // flag to operate in show-movie mode

      double  param_m;         // m of cell (meniscus)
      double  param_b;         // b of cell (bottom)
      double  param_s20w;      // base s value (sedimentation coefficient)
      double  param_D20w;       // base D value (diffusion coefficient)
      double  param_s;         // base s value (sedimentation coefficient)
      double  param_D;         // base D value (diffusion coefficient)
      double  param_w2;        // rpm-based omega-sq-t, w2=(rpm*pi/30)^2
      double  param_w2_t0;      // rpm-based omega-sq-t at time t0, w2=(rpm(t0)*pi/30)^2
      double  param_w2_t0dt;     // rpm-based omega-sq-t at time t0+dt, w2=(rpm(t0+dt)*pi/30)^2
      QVector<double> rpm_timestate;
      QVector<double> w2t_timestate;
      QVector<int> sim_speed_prof_idx_timestate;

      double  err_tol;         // error tolerance for mesh refinement

      double  d_coeff[ 6 ]{};    // SD Adjust buffer density coefficients
      double  v_coeff[ 6 ]{};    // SD Adjust buffer viscosity coefficients
      double dt;
      // private functions

      //! \brief Get the non-ideal case number from model parameters
      //! \returns Non-zero if multiple non-ideal conditions
      int  nonIdealCaseNo( );

      //! \brief Set up non-ideal case type 1 (concentration-dependent)
      //! \param sigma_k Sigma constant to modify sedimentation coefficient
      //! \param delta_k Delta constant to modify diffusion coefficient
      void SetNonIdealCase_1( double, double );

      //! \brief Set up non-ideal case type 2 (co-sedimenting)
      void SetNonIdealCase_2( );

      //! \brief Set up non-ideal case type 3 (compressibility)
      //! \param mropt   Reference to mesh refine option flag to set
      //! \param err_tol Reference to error tolerance factor to set
      static void SetNonIdealCase_3( int&, double& );

      //! \brief Set up non-ideal case type 4 (co-diffusing)
      void SetNonIdealCase_4( );

      // Lamm equation step for sedimentation difference - predict
      void LammStepSedDiff_P( double, double, int, const double*, const double*, double*, const int* scan_hint = nullptr ) const;

      // Lamm equation step for sedimentation difference - calculate
      void LammStepSedDiff_C( double t, double dt_, int M0, const double *x0, const double *u0, int M1, const double *x1,
                              const double *u1p, double *u1, const int* scan_hint = nullptr) const;

      // Project piecewise quadratic solution onto mesh
      static void   ProjectQ(   int, const double*, const double*, int, const double*, double* );

      // Integrate piecewise quadratic function defined on mesh
      static double IntQs( const double*, const double*, int, double, int, double );

      // Perform quadratic interpolation to fill out radius,conc. vectors
      static void   quadInterpolate( const double*, const double*, int,
                              QVector< double >&, QVector< double >& );

      static void   LocateStar( int, const double*, int, const double*, int*, double* );

      // Adjust s and D arrays
      void   AdjustSD( const double, const int, const double*, const double*, double*, double*, const int* ) const;

      static void   fun_phi(    double, double* );

      static void   fun_dphi(   double, double* );

      static void   fun_Iphi(   double, double* );

      static double IntQ(       const double*, const double*, double, double );


      static void LsSolver53(   int, double**, double*, double* );

      // load internal data from caller's raw data
      void load_mfem_data(  US_DataIO::RawData&, US_AstfemMath::MfemData&,
                            bool = true ) const;

      // store internal data to caller's raw data
      void store_mfem_data( US_DataIO::RawData&, US_AstfemMath::MfemData& ) const;
};
#endif
