#ifndef US_ASTFEM_RSA_H
#define US_ASTFEM_RSA_H

#include <qapp.h>
#include "us_astfem_math.h"

// #define USE_US_TIMER

class US_EXTERN US_Astfem_RSA : public QObject
{
   Q_OBJECT

   public:

      US_Astfem_RSA();
      US_Astfem_RSA(bool, QObject * parent=0, const char * name=0);
      ~US_Astfem_RSA();
      bool stopFlag;         // stop calculation, interpolate and return
      bool movieFlag;         // output time and movies if true
      bool time_correction;   // decides if output data is time corrected for acceleration (true=yes, false=no)
      bool use_time;            // decides if output data is interpolated based on time (=true) or on omega-square-t integral (=false)
      bool guiFlag;            // if true, signals will be emitted
      double last_time;         // keep track of time globally for w2t_integral calculation
      double w2t_integral;      // keep track of w2t_integral value globally
      struct SimulationParameters *simparams;
      struct ModelSystem *system;

   private:
      unsigned int N;         // number of points used in radial direction in ASTFEM
      struct AstFemParameters af_params;
#ifdef WIN32
      #pragma warning ( disable: 4251 )
#endif
      vector <double> x;       // radii of grid points; x[0...N-1]
      vector <struct ReactionGroup> rg;
#ifdef WIN32
     #pragma warning ( default: 4251 )
#endif
      int thread;
      vector < rotorInfo > *rotor_list;

   public slots:

      int calculate(struct ModelSystem *, 
                    struct SimulationParameters *, 
                    vector < mfem_data > *, 
                    int *progress=0,
                    int thread=0,
                    vector < rotorInfo > *rotor_list = 0);
      int calculate_ni(double,             // rpm_start
                       double,             // rpm_stop
                       mfem_initial *,      // C0
                       mfem_data *,         // simdata
                       bool);               // acceleration? (1=acceleration, 0=no acceleration)

      int calculate_ra2(double,             // rpm_start
                        double,             // rpm_stop
                        mfem_initial *,   // C0
                        mfem_data *,      // simdata
                        bool);            // acceleration? (1=acceleration, 0=no acceleration)
      void setTimeCorrection(bool);         // true if time correction is done
                                          // (this sets scan times to what would have been observed if the rotor
                                          // could have started up instantaneously at the desired speed without
                                          // an acceleration phase
      void setTimeInterpolation(bool);      // if true, use time values from experimental data to interpolate simulated data
                                          // if false, use omega^2*t integral values from experimental data to interpolate simulated data
      void setMovie(bool);                  // emit movie signals if true
      void setStopFlag(bool);               // stop calculation and exit if true

   signals:

      void new_scan(vector <double> *, double *);
      void new_time(float);
      void current_component(int);
      void current_speed(unsigned int);


   private slots:

      void initialize_conc(unsigned int, struct mfem_initial *, bool); // initializes total concentration vector
      void mesh_gen(vector <double>, unsigned int);
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
      void print_af();                  // output all af params
      void print_rg();                  // output all params in a reaction group
      void print_af(FILE *);            // output all af params to file
      void print_simparams();            // print simparams
      void print_vector(vector <double> *);
      void print_vector(double *, unsigned int);
      void initialize_rg();
      void update_assocv();
      void decompose(struct mfem_initial *);
};


#endif





