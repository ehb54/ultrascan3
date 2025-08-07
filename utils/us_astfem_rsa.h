//! \file us_astfem_rsa.h
#ifndef US_ASTFEM_RSA_H
#define US_ASTFEM_RSA_H

#include <QtCore>

#include "us_astfem_math.h"
#include "us_buffer.h"
#include "us_dataIO.h"
#include "us_extern.h"
#include "us_model.h"
#include "us_simparms.h"
#include "us_stiffbase.h"
#ifndef DbgLv
#define DbgLv(a) \
  if (dbg_level >= a) qDebug()  //!< debug-level-conditioned qDebug()
#endif

//! \brief The module that calculates simulation data
class US_UTIL_EXTERN US_Astfem_RSA : public QObject {
  Q_OBJECT

 public:
  //! \brief Initialize the simulation calculation object.
  //! \param model  Reference to the model.
  //! \param params Reference to the simulation parameters.
  //! \param parent Parent object, normally not specified.
  US_Astfem_RSA(US_Model&, US_SimulationParameters&, QObject* = 0);

  //! \brief Do the bulk of simulation calculations.
  //! \param exp_data  Reference to the artificial experimental object
  //!                  to be created and populated by simulation.
  int calculate(US_DataIO::RawData&);

  //! \brief Set a flag for whether to perform time correction.
  //! \param flag  Flag for whether or not to perform correction.
  void setTimeCorrection(bool flag) { time_correction = flag; };

  //! \brief Set a flag for whether to perform time interpolation.
  //! \param flag  Flag for whether or not to perform interpolation.
  //!              used for choice of interpolation either w.r.t. time
  //!              or omega_square_t
  void setTimeInterpolation(bool flag) { use_time = flag; };

  //! \brief Set a flag for whether to stop calculations.
  //! \param flag  Flag to stop calculation() computations and return.
  void setStopFlag(bool flag) { stopFlag = flag; };

  //! \brief Set a flag for whether to emit signals for movie mode.
  //! \param flag  Flag for whether or not to operate in show-movie mode.
  void set_movie_flag(bool flag) { show_movie = flag; };

  //! \brief Set a flag for whether to output the simulation data.
  //! \param flag  Flag for whether or not to output raw simulation data,
  //!              instead of the normal data interpolated to fit the
  //!              input experiment grid.
  void set_simout_flag(bool flag) { simout_flag = flag; };

  //! \brief Set a flag for the debug print level
  //! \param flag  Integer debug print level (dbg_level).
  void set_debug_flag(int flag) { dbg_level = flag; };

  void set_buffer(US_Buffer);

 signals:
  //! \brief Signal that a calculate_ni()/calculate_ra2() step is complete.
  //! The connected slot receives the radius vector and concentration
  //! array from loops in functions calculate_ni() and calculate_ra2().
  void new_scan(QVector<double>*, double*);

  //! \brief Signal that a calculate()/*_ni()/*_ra2() step is complete.
  //! The connected slot receives the scan time value from loops in
  //! functions calculate(), calculate_ni(), and calculate_ra2().
  void new_time(double);

  //! \brief Signal that a calculate() step is complete
  //! The connected slot receives the integer step count from loops in
  //! function calculate().
  void current_component(int);

  //! \brief Signal that a calculate_ni()/calculate_ra2() step is complete.
  //! The connected slot receives the integer step count from loops in
  //! functions calculate_ni() and calculate_ra2().
  void current_speed(int);

  //! \brief Signal that decompose() calculations have begun.
  //! The connected slot receives notice that decompose() function
  //! calculations have begun and receives the integer maximum
  //! number of steps.
  void calc_start(int);

  //! \brief Signal that a decompose() calculation step is complete.
  //! The connected slot receives the integer step count from the
  //! decompose() function.
  void calc_progress(int);

  //! \brief Signal that decompose() calculations are done.
  //! The connected slot receives notice that decompose() function
  //! calculations have completed.
  void calc_done(void);

 private:
  US_TimeState
      timestate;  //!< Keeps information for each second of the experiment
  bool stopFlag;  //!< Stop calculation, interpolate, and return
  bool time_correction;  //!< Decides if output data is time corrected
                         //!< for acceleration (true=yes, false=no)
  bool use_time;         //!< Decides if output data is interpolated
                         //!< based on time (=true) or
                         //!< based on omega-square-t integral (=false)
  bool show_movie;       //!< Decides if a movie of scans will be shown
                         //!< on window or not
  bool simout_flag;      //!< Decides whether simulated data should be stored
                         //!< or cleared
  bool is_zero;          //!< Set and tested for each solute point
  US_StiffBase stfb0;    //!< Structure used for numerical integration
                         //   on a quadrilateral
  double density;        //!< Density of the buffer
  double compressib;     //!< Compressibility of the buffer
  double d_coeff[6];     //!< SD Adjust buffer density coefficients
  double v_coeff[6];     //!< SD Adjust buffer viscosity coefficients

  //! Keep track of time globally for w2t_integral calculation
  double last_time;     //!< Time used for last scan or last speed zone
  double w2t_integral;  //!< Keep track of w2t_integral value globally
  double tot_conc;      //!< Total concentration in model
  int Nx;               //!< Number of points used in radial direction
  int dbg_level;        //!< Debug level

  US_AstfemMath::AstFemParameters
      af_params;                     //!< Parameters used for adaptive
                                     //!<  space time finite element solution
  US_AstfemMath::MfemData af_data;   //!< A data set comprised of scans from
                                     //!<   one sample taken at constant speed
  US_AstfemMath::MfemInitial af_c0;  //!< Initial concentration conditions
  double* xA;                        //<! x values array
  QVector<double> x;                 //<! Radii of grid points

  QVector<US_AstfemMath::ReactionGroup> rg;  //<! Used for reacting system
  US_Model& system;                          //<! Defined model
  US_SimulationParameters& simparams;        //<! Simulation parameters

  // Functions:

  //!< Updates association vector in case of reacting
  //   case based on stoichiometry values
  void update_assocv(void);

  //<! Gives meniscus and bottom after rotor stretch
  //!< Input  :  rotorspeed
  //!< Output :  updates current meniscus and bottom
  //             position
  void adjust_limits(int);

  //!< Calculates stretch amount of the rotor
  //!< Input  : An array having rotor coefficients
  //            and rotor speed ( double )
  //!< Output : stretch value ( returns )
  double stretch(double*, int);

  //!< Initializes reaction group
  //!< Input  : Default reaction group
  //!< Output : Updated reaction group
  //!< Starts with default reaction
  //   group structure and updates
  //   components based on association
  //   vector
  void initialize_rg(void);

  //!< Initializes concentration vector
  //!< Input : 1. Component no. of the model ( int )
  //         : 2. Initial concentration vector ( MfemInitial )
  //         : 3. Flag for reacting syastem ( bool ) : true : if non-reacting
  //         case
  //                                                   false: if reacting case
  //!< Output: 2. Initial concentration vector (  MfemInitial )
  void initialize_conc(int, US_AstfemMath::MfemInitial&, bool);

  //!< Does finite element calculation for  non-interacting case
  //!< Input : 1. Current rotorspeed ( double )
  //         : 2. Next    rotorspeed ( double )
  //         : 3. Speed_step no. of the speed_profile structure ( int )
  //         : 4. Initial concentration vector ( MfemInitial )
  //!< Output: 5. Simulated  data ( MfemData )
  //!< Input : 6. Acceleration flag ( bool ), true : if in acceleration zone
  //                                          false: if in constant speed zone
  int calculate_ni(double, double, int, US_AstfemMath::MfemInitial&,
                   US_AstfemMath::MfemData&, bool);

  //!< Does mesh generation
  //!< Input  : 1. Qvector containing ( omega )^s/D values
  //            2. mesh type ( int )
  //!< Output : x: Qvector containing radial points
  void mesh_gen(QVector<double>&, int);

  //!< Generate exponential mesh and refine cell bottom (for s>0)
  //   Called inside mesh_gen.
  //!< Input  : Qvector containing ( omega )^s/D values
  //!< Output : x: Qvector containing radial points ( updated after
  //            refinement )
  void mesh_gen_s_pos(const QVector<double>&);

  //!< Generate exponential mesh and refine cell meniscus (for  s<0)
  //    Called inside mesh_gen.
  //!< Input  : Qvector containing ( omega )^s/D values
  //!< Output : x: Qvector containing radial points ( updated after
  //            refinement )
  void mesh_gen_s_neg(const QVector<double>&);

  //!< refine mesh near meniscus (for s>0) or
  //!< near bottom (for s<0) to be used for the
  //!< acceleration stage
  // Parameters:
  //!< Input  : Number of elements near meniscus
  //              (or bottom) to be refined (int)
  //          : Number of elements to be used for
  //            the refined region
  //!< Output : x: Qvector containing radial points
  //            ( updated after refinement )

  void mesh_gen_RefL(int, int);

  //!< Computes coefficient matrix for fixed mesh case

  //!< Parameters :
  //!< Input : D value     ( double )
  //         : s*omega^2/D ( double )
  //         : Null double 2D array
  //         : Null double 2D array
  //!< Output: Stiffness matrix on the left ( double 2D array )
  //         : Stiffness matrix on the right( double 2D array )
  //
  void ComputeCoefMatrixFixedMesh(double, double, double**, double**);

  //!< An initial concentration vector is decomposed into components of
  //!< the reaction group
  //!< Input  : Initial concentration vector
  //!< Output : Concentration of different components
  void decompose(US_AstfemMath::MfemInitial*);

  //!< Computes coefficient matrix for moving mesh method (right ) if
  //!< s value is greater than zero
  //!< Parameters :
  //!< Input : 1. D value     ( double )
  //         : 2. s*omega^2/D ( double )
  //         : 3. Null double 2D array
  //         : 4. Null double 2D array
  //!< Output: 3. Stiffness matrix on the left ( double 2D array )
  //         : 4. Stiffness matrix on the right( double 2D array )
  void ComputeCoefMatrixMovingMeshR(double, double, double**, double**);

  //!< Computes coefficient matrix for moving mesh method (less ) if
  //!< s value is less than zero
  //!< Parameters :
  //!< Input : 1. D value     ( double )
  //         : 2. s*omega^2/D ( double )
  //         : 3. Null double 2D array
  //         : 4. Null double 2D array
  //!< Output: 3. Stiffness matrix on the left ( double 2D array )
  //         : 4. Stiffness matrix on the right( double 2D array )
  void ComputeCoefMatrixMovingMeshL(double, double, double**, double**);

  //!< Solves ode using implicit-mid point-Euler's method for
  //   reacting system case
  //!< Input : 1. Number of radial points
  //!< Output: 2. Next Concentration vector
  //!< Input : 3. Step of the time interval
  void ReactionOneStep_Euler_imp(int, double**, double);

  //!<  Prints simulation parameters
  //!<  Input : simparams structure
  //!<  Output: Prints the values
  //            in simparams structure
  void printsimparams(void);

  //!< Prints  simulation components of the model
  //!< Input : Given simulation components
  //!< Output: Prints the values in simulation components
  void printsimComponent(US_Model::SimulationComponent&);

  //!< Prints speed profile structure
  //!< Input : SpeedProfile structure
  //!< Output: Prints the values in SpeedProfile structure
  void printspeedprofile(US_SimulationParameters::SpeedProfile&);

  //!< These two following functions are used in
  //!< implicit-mid point-Euler's method ( ReactionOneStep_
  //!< Euler_imp )
  void Reaction_dydt(double*, double*);
  void Reaction_dfdy(double*, double**);

  //!< Does finite element calculation for reacting syastem case
  //!< Input : 1. Current rotorspeed ( double )
  //         : 2. Next    rotorspeed ( double )
  //         : 3. Initial concentration vector ( MfemInitial )
  //!< Output: 4. Simulated  data ( MfemData )
  //!< Input : 5. Acceleration flag ( bool ), true : if in acceleration zone
  //                                          false: if in constant speed zone
  int calculate_ra2(double, double, US_AstfemMath::MfemInitial*,
                    US_AstfemMath::MfemData&, bool);

  //!< Constructs global stiffness matrix
  void GlobalStiff(double*, double**, double**, double, double);

  //!< Used for naming the temporary timestate file
  //!< if needed in us_astfem_rsa
  //!< Input : Current time in date+hrs+mins+seconds
  //!< Output: A string with date+hrs+mins+seconds
  QString temp_Id_name();

  //!< Writes a temporary timestate file if not found
  //!< Input  : 1. A string with date+hrs+mins+seconds
  //!<        : 2. Experimental data set
  //!< Output : A timestate structure containing information
  //            for each second
  int writetimestate(const QString&, US_DataIO::RawData&);

  //!< Loads the experimental data and converts into "MfemData" type
  //!< Input  : experimental data ( RawData )
  //!< Output : simulated data (MfemData ) and used for getting scans
  //            on experimental grid
  void load_mfem_data(US_DataIO::RawData&, US_AstfemMath::MfemData&);

  //!< Stores the computed data on the grid depending on simout_flag i.e.
  //!< either experimental grid or simulation grid
  //!< Input  : Simulated data ( MfemData )
  //!< Output : Updated experimental data ( RawData )
  void store_mfem_data(US_DataIO::RawData&, US_AstfemMath::MfemData&);

  //! \brief Determines if computed scan concentration vector virtually zero
  //! \param s         Sedimentation coefficient, current component
  //! \param D         Diffusion coefficient, current component
  //! \param rpm       Speed, current step
  //! \param t         Current time (or first of step)
  //! \param meniscus  Meniscus, current step
  //! \param bottom    Bottom, current step
  //! \returns         Pre-bottom-scan-vector-is-zero flag
  bool iszero(const double, const double, const double, const double,
              const double, const double);

//---------------------------Not used-------------------------------
#ifdef NEVER
  void GlobalStiff_ellam(QVector<double>*, double**, double**, double, double);
  void adjust_grid(int /*old speed*/, int /*new speed*/,
                   QVector<double>* /*radial grid*/);
#endif
  //------------------------------------------------------------------
};

#endif
