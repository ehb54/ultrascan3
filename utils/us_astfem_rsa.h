//! \file us_astfem_rsa.h
#ifndef US_ASTFEM_RSA_H
#define US_ASTFEM_RSA_H

#include <QtCore>

#include "us_extern.h"
#include "us_astfem_math.h"

class US_EXTERN US_Astfem_RSA : public QObject
{
   Q_OBJECT

   public:

      US_Astfem_RSA( struct ModelSystem&, struct SimulationParameters&, 
            QObject* = 0 );
   
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
      uint   N;               //!< Number of points used in radial direction
      
      struct AstFemParameters af_params;

      QList< double >               x;    // Radii of grid points; x[0...N-1] 
      QList< struct ReactionGroup > rg;
      struct ModelSystem&           system;
      struct SimulationParameters&  simparams;

      // Functions
      void   update_assocv  ( void );
      void   adjust_limits  ( uint speed );
      double stretch        ( int, uint );
      void   initialize_rg  ( void );
      void   initialize_conc( uint, struct mfem_initial&, bool );

      int    calculate_ni   ( double, double, mfem_initial&, mfem_data&, bool );
      void   mesh_gen       ( QList< double >&, uint );
      void   mesh_gen_s_pos ( const QList< double >& );
      void   mesh_gen_s_neg ( const QList< double >& );
      void   mesh_gen_RefL  ( int, int );
      
      void   ComputeCoefMatrixFixedMesh( double, double, double**, double** );
      void   decompose      ( struct mfem_initial* );

      void   ComputeCoefMatrixMovingMeshR( double, double, double**, double** );
      void   ComputeCoefMatrixMovingMeshL( double, double, double**, double** );
             
      void   ReactionOneStep_Euler_imp   ( uint, double**, double );
             
      void   Reaction_dydt    ( double*, double*  );
      void   Reaction_dfdy    ( double*, double** );
             
      int    calculate_ra2    ( double, double, mfem_initial*, mfem_data&, bool );         

      void   GlobalStiff      ( QList< double >&, double**, double**,
                                double, double );

   public: 

      void setTimeCorrection   ( bool flag ){ time_correction = flag; }; 
      void setTimeInterpolation( bool flag ){ use_time        = flag; };
      void setStopFlag         ( bool flag ){ stopFlag        = flag; };    
      void set_movie_flag      ( bool flag ){ show_movie      = flag; };
      
      int  calculate           ( //struct ModelSystem&, 
                                 //struct SimulationParameters&, 
                                 QList< struct mfem_data >&  );

   signals:
      void new_scan         ( QList< double >&, double* );
      void new_time         ( float                     );
      void current_component( int                       );
      void current_speed    ( unsigned int              );
      void calc_start       ( unsigned int              );
      void calc_progress    ( unsigned int              );
      void calc_done        ( void                      );

#ifdef NEVER
      void GlobalStiff_ellam(vector <double> *, double **, double **, double, double);
      void adjust_grid( uint /*old speed*/, uint /*new speed*/, QList <double> * /*radial grid*/);
#endif
};


#endif

