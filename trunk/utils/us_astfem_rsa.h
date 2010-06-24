//! \file us_astfem_rsa.h
#ifndef US_ASTFEM_RSA_H
#define US_ASTFEM_RSA_H

#include <QtCore>
#include "us_extern.h"
#include "us_model.h"
#include "us_simparms.h"
#include "us_dataIO2.h"
#include "us_astfem_math.h"

//! \brief The module that calculates simulation data
class US_EXTERN US_Astfem_RSA : public QObject
{
   Q_OBJECT

   public:
      //! \brief Initialize the 
      //! \param model  Reference to the model parmeters
      //! \param params Referece to the simulation parameters 
      //! \param parent Parent object, normally not specified
      US_Astfem_RSA( US_Model&, US_SimulationParameters&, QObject* = 0 );
   
      int  calculate           ( US_DataIO2::RawData& );

      void setTimeCorrection   ( bool flag ){ time_correction = flag; }; 
      void setTimeInterpolation( bool flag ){ use_time        = flag; };
      void setStopFlag         ( bool flag ){ stopFlag        = flag; };    
      void set_movie_flag      ( bool flag ){ show_movie      = flag; };
      

   signals:
      void new_scan         ( int    );
      void new_time         ( double );
      void current_component( int    );
      void current_speed    ( int    );
      void calc_start       ( int    );
      void calc_progress    ( int    );
      void calc_done        ( void   );
   private:
      bool stopFlag;          //!< Stop calculation, interpolate, and return
      bool time_correction;   //!< Decides if output data is time corrected 
                              //!< for acceleration (true=yes, false=no)
      
      bool use_time;          //!< Decides if output data is interpolated 
                              //!< based on time (=true) or 
                              //!< based on omega-square-t integral (=false)
      
      bool show_movie;

      //! Keep track of time globally for w2t_integral calculation
      double last_time;      

      double w2t_integral;    //!< Keep track of w2t_integral value globally
      int    N;               //!< Number of points used in radial direction
      
      struct AstFemParameters         af_params;
      US_AstfemMath::MfemData         af_data;
      US_AstfemMath::MfemInitial      af_c0;

      QVector< double >               x;  //<! Radii of grid points; x[0...N-1] 
      QVector< struct ReactionGroup > rg;
      US_Model&                       system;
      US_SimulationParameters&        simparams;

      // Functions
      void   update_assocv  ( void );
      void   adjust_limits  ( int speed );
      double stretch        ( int, int );
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

      void   GlobalStiff      ( QVector< double >&, double**, double**,
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

