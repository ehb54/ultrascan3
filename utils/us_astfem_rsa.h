//! \file us_astfem_rsa.h
#ifndef US_ASTFEM_RSA_H
#define US_ASTFEM_RSA_H

#include <QtCore>
#include "us_extern.h"
#include "us_model.h"
#include "us_simparms.h"
#include "us_dataIO2.h"
#include "us_astfem_math.h"
#include "us_stiffbase.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug() //!< debug-level-conditioned qDebug()
#endif

//! \brief The module that calculates simulation data
class US_UTIL_EXTERN US_Astfem_RSA : public QObject
{
   Q_OBJECT

   public:
      //! \brief Initialize the simulation calculation object.
      //! \param model  Reference to the model.
      //! \param params Reference to the simulation parameters.
      //! \param parent Parent object, normally not specified.
      US_Astfem_RSA( US_Model&, US_SimulationParameters&, QObject* = 0 );
   
      //! \brief Do the bulk of simulation calculations.
      //! \param exp_data  Reference to the artificial experimental object
      //!                  to be created and populated by simulation.
      int  calculate           ( US_DataIO2::RawData& );

      //! \brief Set a flag for whether to perform time correction.
      //! \param flag  Flag for whether or not to perform correction.
      void setTimeCorrection   ( bool flag ){ time_correction = flag; }; 

      //! \brief Set a flag for whether to perform time interpolation.
      //! \param flag  Flag for whether or not to perform interpolation.
      void setTimeInterpolation( bool flag ){ use_time        = flag; };

      //! \brief Set a flag for whether to stop calculations.
      //! \param flag  Flag to stop calculation() computations and return.
      void setStopFlag         ( bool flag ){ stopFlag        = flag; };    

      //! \brief Set a flag for whether to emit signals for movie mode.
      //! \param flag  Flag for whether or not to operate in show-movie mode.
      void set_movie_flag      ( bool flag ){ show_movie      = flag; };

      //! \brief Set a flag for whether to output the simulation data.
      //! \param flag  Flag for whether or not to output raw simulation data,
      //!              instead of the normal data interpolated to fit the
      //!              input experiment grid.
      void set_simout_flag     ( bool flag ){ simout_flag     = flag; };

   signals:
      //! \brief Signal that a calculate_ni()/calculate_ra2() step is complete.
      //!
      //! The connected slot receives the radius vector and concentration
      //! array from loops in functions calculate_ni() and calculate_ra2().
      void new_scan         ( QVector< double >*, double* );

      //! \brief Signal that a calculate()/*_ni()/*_ra2() step is complete.
      //!
      //! The connected slot receives the scan time value from loops in
      //! functions calculate(), calculate_ni(), and calculate_ra2().
      void new_time         ( double );

      //! \brief Signal that a calculate() step is complete.
      //!
      //! The connected slot receives the integer step count from loops in
      //! function calculate().
      void current_component( int    );

      //! \brief Signal that a calculate_ni()/calculate_ra2() step is complete.
      //!
      //! The connected slot receives the integer step count from loops in
      //! functions calculate_ni() and calculate_ra2().
      void current_speed    ( int    );

      //! \brief Signal that decompose() calculations have begun.
      //!
      //! The connected slot receives notice that decompose() function
      //! calculations have begun and receives the integer maximum
      //! number of steps.
      void calc_start       ( int    );

      //! \brief Signal that a decompose() calculation step is complete.
      //!
      //! The connected slot receives the integer step count from the
      //! decompose() function.
      void calc_progress    ( int    );

      //! \brief Signal that decompose() calculations are done.
      //!
      //! The connected slot receives notice that decompose() function
      //! calculations have completed.
      void calc_done        ( void   );

   private:
      bool stopFlag;          //!< Stop calculation, interpolate, and return
      bool time_correction;   //!< Decides if output data is time corrected 
                              //!< for acceleration (true=yes, false=no)
      
      bool use_time;          //!< Decides if output data is interpolated 
                              //!< based on time (=true) or 
                              //!< based on omega-square-t integral (=false)
      
      bool show_movie;
      bool simout_flag;
      US_StiffBase stfb0;

      //! Keep track of time globally for w2t_integral calculation
      double last_time;      

      double w2t_integral;    //!< Keep track of w2t_integral value globally
      int    N;               //!< Number of points used in radial direction
      int    dbg_level;
      
      US_AstfemMath::AstFemParameters af_params;
      US_AstfemMath::MfemData         af_data;
      US_AstfemMath::MfemInitial      af_c0;

      QVector< double >                       x;  //<! Radii of grid points
      QVector< US_AstfemMath::ReactionGroup > rg;
      US_Model&                               system;
      US_SimulationParameters&                simparams;

      // Functions
      void   update_assocv  ( void );
      void   adjust_limits  ( int speed );
      double stretch        ( double*, int );
      void   initialize_rg  ( void );
      void   initialize_conc( int, US_AstfemMath::MfemInitial&, bool );

      int    calculate_ni   ( double, double, US_AstfemMath::MfemInitial&,
                              US_AstfemMath::MfemData&, bool );
      void   mesh_gen       ( QVector< double >&, int );
      void   mesh_gen_s_pos ( const QVector< double >& );
      void   mesh_gen_s_neg ( const QVector< double >& );
      void   mesh_gen_RefL  ( int, int );
      
      void   ComputeCoefMatrixFixedMesh( double, double, double**, double** );
      void   decompose      ( US_AstfemMath::MfemInitial* );

      void   ComputeCoefMatrixMovingMeshR( double, double, double**, double** );
      void   ComputeCoefMatrixMovingMeshL( double, double, double**, double** );
             
      void   ReactionOneStep_Euler_imp   ( int, double**, double );
             
      void   Reaction_dydt    ( double*, double*  );
      void   Reaction_dfdy    ( double*, double** );
             
      int    calculate_ra2    ( double, double, US_AstfemMath::MfemInitial*,
                                US_AstfemMath::MfemData&, bool );         

      void   GlobalStiff      ( double*, double**, double**,
                                double, double );

      void   load_mfem_data ( US_DataIO2::RawData&, US_AstfemMath::MfemData& );         
      void   store_mfem_data( US_DataIO2::RawData&, US_AstfemMath::MfemData& );         

#ifdef NEVER
      void GlobalStiff_ellam( QVector <double> *, double **, double **,
                              double, double );
      void adjust_grid( int /*old speed*/, int /*new speed*/,
                        QVector <double> * /*radial grid*/ );
#endif
};


#endif

