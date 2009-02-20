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
      
      //! Keep track of time globally for w2t_integral calculation
      double last_time;      

      double w2t_integral;    //!< Keep track of w2t_integral value globally
      uint   N;               //!< Number of points used in radial direction
      
      struct AstFemParameters af_params;

      QList< double >               x;    // Radii of grid points; x[0...N-1] 
      QList< struct ReactionGroup > rg;
      struct ModelSystem            system;
      struct SimulationParameters   simparams;

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

   public: 

      void setTimeCorrection   ( bool flag ){ time_correction = flag; }; 
      void setTimeInterpolation( bool flag ){ use_time        = flag; };
      void setStopFlag         ( bool flag ){ stopFlag        = flag; };    
      
      int  calculate           ( struct ModelSystem&, 
                                 struct SimulationParameters&, 
                                 QList< struct mfem_data >&  );
      
   signals:
      void new_scan         ( QList< double >*, double* );
      void new_time         ( float                     );
      void current_component( int                       );
      void current_speed    ( unsigned int              );

      
#ifdef NEVER
      
      struct SimulationParameters *simparams;
      struct ModelSystem *system;

   private:
      unsigned int N;         // number of points used in radial direction in ASTFEM
#ifdef WIN32
      #pragma warning ( disable: 4251 )
#endif
      vector <double> x;      // radii of grid points; x[0...N-1]
#ifdef WIN32
     #pragma warning ( default: 4251 )
#endif

   public slots:

      int calculate_ni(double,            // rpm_start
                       double,            // rpm_stop
                       mfem_initial *,    // C0
                       mfem_data *,       // simdata
                       bool);             // acceleration? (1=acceleration, 0=no acceleration)

      int calculate_ra2(double,           // rpm_start
                        double,           // rpm_stop
                        mfem_initial *,   // C0
                        mfem_data *,      // simdata
                        bool);            // acceleration? (1=acceleration, 0=no acceleration)


   private slots:

      void initialize_conc(unsigned int, struct mfem_initial *, bool); // initializes total concentration vector
      void mesh_gen_s_pos(vector <double>);
      void mesh_gen_s_neg(vector <double>);
      void mesh_gen_RefL(int, int);

      void GlobalStiff(vector <double> *, double **, double **, double, double);
      void GlobalStiff_ellam(vector <double> *, double **, double **, double, double);

      void ComputeCoefMatrixFixedMesh(double, double, double **, double **);
      void ComputeCoefMatrixMovingMeshR(double, double, double **, double **);
      void ComputeCoefMatrixMovingMeshL(double, double, double **, double **);

      void ReactionOneStep_Euler_imp(unsigned int, double **, double);
      // int DecomposeCT(double , double *);

      // interpolate maps a simulated grid with a variable delta_r grid onto a
      // fixed delta_r grid from experimental data, and also interpolates time

      void Reaction_dydt(double *, double *);
      void Reaction_dfdy(double *, double **);
      void adjust_limits(unsigned int /*rotor speed*/);
      void adjust_grid(unsigned int /*old speed*/, unsigned int /*new speed*/, vector <double> * /*radial grid*/);

// output functions:
      void print_af();                 // output all af params
      void print_rg();                 // output all params in a reaction group
      void print_af(FILE *);           // output all af params to file
      void print_simparams();          // print simparams
      void print_vector(vector <double> *);
      void print_vector(double *, unsigned int);
      void initialize_rg();
      void update_assocv();
      void decompose(struct mfem_initial *);
#endif
};


#endif





