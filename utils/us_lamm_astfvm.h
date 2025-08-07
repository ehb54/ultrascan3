//! \file us_lamm_astfvm.h
#ifndef US_LAMMASTFVM_H
#define US_LAMMASTFVM_H

#include <QtCore>
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_buffer.h"
#include "us_dataIO.h"
#include "us_extern.h"
#include "us_model.h"
#include "us_simparms.h"

#ifndef DbgLv
#define DbgLv(a) \
   if (dbg_level >= a) \
   qDebug() //!< debug-level-conditioned qDebug()
#endif

//! \brief Module to calculate simulation data by AST Finite Volume Method
class US_UTIL_EXTERN US_LammAstfvm : public QObject {
      Q_OBJECT

   public:
      //! \brief Finite volume mesh for ASTFVM solution
      class Mesh {
         public:
            //! \brief Changeable non-ideal mesh
            //! \param xl    Left X (radius) value
            //! \param xr    Right X (radius) value
            //! \param Nelem Number of elements
            //! \param Opt   Mesh option (0 for uniform)
            Mesh(double, double, int, int);

            //! \brief Destroy mesh
            ~Mesh();

            //! \brief Initialize mesh
            //! \param s  Sedimentation coefficient
            //! \param D  Diffusion coefficient
            //! \param w2 Omega squared
            void InitMesh(double, double, double);

            //! \brief Refine mesh
            //! \param u0     Current concentration array
            //! \param u1     Next concentration array
            //! \param ErrTol Error tolerance
            void RefineMesh(double *, double *, double);

            int Nv; //!< Number of grids
            int Ne; //!< Number of elements
            double *x; //!< radius coordinates of grids

         private:
            int dbg_level; // debug level
            int MaxRefLev;
            int MonScale;
            double MonCutoff;
            double SmoothingWt;
            int SmoothingCyl;

            int *Eid; // elem id
            int *RefLev; // refinement level of an elem
            double *MeshDen; // desired mesh density
            int *Mark; // ref/unref marker

            // private functions
            void ComputeMeshDen_D3(double *, double *);
            void Smoothing(int, double *, double, int);
            void Unrefine(double);
            void Refine(double);
      };

      //! \brief Salt data for co-sedimenting
      class SaltData {
         public:
            //! \brief Create salt data
            //! \param amodel    Model with all components to simulate
            //! \param asparms   Simulation parameters for all components
            //! \param asim_data Simulation AUC data for all components
            SaltData(US_Model, US_SimulationParameters, US_DataIO::RawData *);

            //! \brief Destroy salt data
            ~SaltData();

            //! \brief Initialize time & concentration arrays for a component
            void initSalt();

            //! \brief Interpolate concentrations of salt
            //! \param N     Number of elements in arrays
            //! \param x     X (radius) array
            //! \param t     Current time value
            //! \param Csalt Concentration of salt for current time
            void InterpolateCSalt(int, double *, double, double *);

            US_DataIO::RawData sa_data; //!< salt data 1-component
               //!<  simulation for co-sed
         private:
            US_Model model; // salt data co-sed model
            US_SimulationParameters simparms; // salt simulation parameters

            int Nx; // number of points in radial direction
            int Nt; // number of points in time direction
            int scn; // index to next available salt data scan
            double *xs; // grids in radial direction
            double t0; // 1st time intervals in use.
            double t1; // 2nd time intervals in use.
            double *Cs0; // salt concentration for the 1st time interval
            double *Cs1; // salt concentration for the 2nd time interval
            int dbg_level; // debug level
            QVector<double> xsVec; // Vector for xs
            QVector<double> Cs0Vec; // Vector for Cs0
            QVector<double> Cs1Vec; // Vector for Cs1
      };

      //! \brief Create Lamm equations AST Finite Volume Method solver
      //! \param rmodel     Reference to model on which solution is based
      //! \param rsimparms  Reference to simulation parameters
      //! \param parent     Parent object (may be 0)
      US_LammAstfvm(US_Model &, US_SimulationParameters &, QObject * = 0);

      //! \brief Destroy FVM solver
      ~US_LammAstfvm();

      //! \brief Main method to calculate FVM solution
      //! \param sim_data Reference to simulated AUC data to produce
      //! \returns        Non-zero if multiple non-ideal conditions
      int calculate(US_DataIO::RawData &);

      //! \brief Set buffer parameters: density, compressibility
      //! \param buffer Buffer object to examine
      void set_buffer(US_Buffer);

      //! \brief Calculate solution for a model component
      //! \param compx Index to model component to use in solution pass
      //! \returns     Non-zero if multiple non-ideal conditions
      int solve_component(int);

      //! \brief Set the mesh speed factor: 1.0 (moving) or 0.0 (non-moving)
      //! \param speed   Mesh speed factor of 1.0 or 0.0
      void SetMeshSpeedFactor(double);

      //! \brief Set the mesh refinement option:  1 to refine, 0 to not refine
      //! \param Opt     Mesh refinement option flag: 1/0 for yes/no.
      void SetMeshRefineOpt(int);

      //! \brief Set stop flag
      //! \param flag    Flag for whether to stop
      void setStopFlag(bool);

      //! \brief Set a flag for whether to emit signals for movie mode.
      //! \param flag    Flag for whether or not to operate in show-movie mode.
      void setMovieFlag(bool);

   signals:
      //! \brief Signal calculation start and give maximum steps
      //! \param nsteps Number of expected total calculation progress steps
      void calc_start(int nsteps);

      //! \brief Signal calculation progress, giving running step count
      //! \param istep Current progress step count
      void calc_progress(int istep);

      //! \brief Signal that calculations are complete
      void calc_done(void);

      //! \brief Signal component progress, giving running component number
      //! \param icomp Current component begun (1,...)
      void comp_progress(int icomp);

      //! \brief Signal that calculation step is complete.
      //! The connected slot receives the radius vector and concentration
      //! array from loops in calculate functions.
      void new_scan(QVector<double> *, double *);

      //! \brief Signal component progress, giving running component number
      //! The connected slot receives the scan time value from calc. functions.a
      void new_time(double);

   private:
      US_Model &model; // input model
      US_SimulationParameters &simparams; // input simulation parameters
      US_DataIO::RawData *auc_data; // input/output AUC data

      US_AstfemMath::MfemData af_data; // internal data

      Mesh *msh; // radial grid

      int NonIdealCaseNo; // non-ideal case number
         // = 0 : ideal, constant s, D
         // = 1 : concentration dependent
         // = 2 : co-sedimenting
         // = 3 : compressibility

      double sigma; // constant for concentration dependence (s)
      double delta; // constant for concentration dependence (D)
         // non-ideal case
         // s = s_0/(1+sigma*C), D=D_0/(1+delta*C)

      double density; // buffer density
      double compressib; // factor for compressibility
      double vbar_salt; // vbar of the salt

      SaltData *saltdata; // data handle for cosedimenting

      double MeshSpeedFactor; // = 1: mesh following sedimentation
         // = 0: fixed mesh in each time step

      int MeshRefineOpt; // = 1: perform mesh local refinement
         // = 0: no mesh refinement

      int comp_x; // current component index
      int dbg_level; // debug level

      bool stopFlag; // flag to stop processing
      bool movieFlag; // flag to operate in show-movie mode

      double param_m; // m of cell (meniscus)
      double param_b; // b of cell (bottom)
      double param_s; // base s value (sedimentation coefficient)
      double param_D; // base D value (diffusion coefficient)
      double param_w2; // rpm-based omega-sq-t, w2=(rpm*pi/30)^2

      double err_tol; // error tolerance for mesh refinement

      double d_coeff[ 6 ]; // SD Adjust buffer density coefficients
      double v_coeff[ 6 ]; // SD Adjust buffer viscosity coefficients

      // private functions

      //! \brief Get the non-ideal case number from model parameters
      //! \returns Non-zero if multiple non-ideal conditions
      int nonIdealCaseNo(void);

      //! \brief Set up non-ideal case type 1 (concentration-dependent)
      //! \param sigma_k Sigma constant to modify sedimentation coefficient
      //! \param delta_k Delta constant to modify diffusion coefficient
      void SetNonIdealCase_1(double, double);

      //! \brief Set up non-ideal case type 2 (co-sedimenting)
      void SetNonIdealCase_2(void);

      //! \brief Set up non-ideal case type 3 (compressibility)
      //! \param mropt   Reference to mesh refine option flag to set
      //! \param err_tol Reference to error tolerance factor to set
      void SetNonIdealCase_3(int &, double &);

      // Lamm equation step for sedimentation difference - predict
      void LammStepSedDiff_P(double, double, int, double *, double *, double *);

      // Lamm equation step for sedimentation difference - calculate
      void LammStepSedDiff_C(double, double, int, double *, double *, int, double *, double *, double *);

      // Project piecewise quadratic solution onto mesh
      void ProjectQ(int, double *, double *, int, double *, double *);

      // Integrate piecewise quadratic function defined on mesh
      double IntQs(double *, double *, int, double, int, double);

      // Perform quadratic interpolation to fill out radius,conc. vectors
      void quadInterpolate(double *, double *, int, QVector<double> &, QVector<double> &);

      void LocateStar(int, double *, int, double *, int *, double *);

      // Adjust s and D arrays
      void AdjustSD(double, int, double *, double *, double *, double *);

      void fun_phi(double, double *);

      void fun_dphi(double, double *);

      void fun_Iphi(double, double *);

      double IntQ(double *, double *, double, double);


      void LsSolver53(int, double **, double *, double *);

      // load internal data from caller's raw data
      void load_mfem_data(US_DataIO::RawData &, US_AstfemMath::MfemData &, bool = true);

      // store internal data to caller's raw data
      void store_mfem_data(US_DataIO::RawData &, US_AstfemMath::MfemData &);
};
#endif
