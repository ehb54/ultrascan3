#ifndef US_ASTFEM_RSA_H
#define US_ASTFEM_RSA_H

#include <qobject.h>
#include <qapp.h>

#include <math.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <climits>

#include "us_extern.h"
#include "us_matrix.h"
#include "us_femglobal.h"
#include "us_component_dialog.h"
#include "us_simulationparameters.h"
#include "us_stiffbase.h"

using namespace std;

struct AstFemParameters
{
	unsigned int simpoints;
	vector <double> s;			// sedimentation coefficient
	vector <double> D;			// Diffusion coefficient
	vector <double> keq;			// Equilibrium constants
	vector <double> koff;		// off rates
	vector <unsigned int> n;	// exponents
 	double dt;						// time step size;
	unsigned int time_steps;	// number of time steps for simulation
	double omega_s;				// omega^2
	double start_time;			// start time in seconds of simulation at constant speed
	double meniscus;
	double bottom;
	unsigned int mesh;			// radial mesh option
	int moving_grid;				// use adaptive time steps = 1, fixed time steps = 0
	bool acceleration;			// true if acceleration is used
	unsigned int model;			// model number
};

class US_EXTERN US_Astfem_RSA : public QObject
{
	Q_OBJECT
	
	public:
	
		US_Astfem_RSA(bool * /*stopFlag*/, bool /*GUIflag*/, bool * /*movieFlag*/, QObject * parent=0, const char * name=0);
		~US_Astfem_RSA();
		bool *stopFlag;	// stop calculation, interpolate and return
		bool *movieFlag;	// output time and movies if true
		bool guiFlag;		// if true, signals will be emitted
		
		
	private:

		// FEM discretization related parameters:
		unsigned int N;			// number of points used in radial direction in ASTFEM
		vector <double> x; 		// radii of grid points; x[0...N-1]
		AstFemParameters af_params;

	public slots:
	
		int calculate(struct ModelSystem *, struct SimulationParameters *, vector <struct mfem_data> *);
		int calculate_ni(double, 				// rpm_start
							  double, 				// rpm_stop
							  double, 				// s
							  double, 				// D
							  mfem_initial *,		// C0
							  mfem_data *);		// simdata

		int calculate_ra2(double, 				// rpm_start
								double, 				// rpm_stop
								mfem_initial *,	// C0
								mfem_data *);		// simdata
	signals:
	
		void new_scan(vector <double> *, double *);
		void new_time(float);
		void current_component(int);
		void current_speed(unsigned int);
		
		
	private slots:

		double IntConcentration(double *, double *, unsigned int);
		double fun_CT(double);
		void mesh_gen(vector <double>, unsigned int);
		void mesh_gen_s_pos(vector <double>);
		void mesh_gen_s_neg(vector <double>);
		void mesh_gen_RefL(int, int);
		double maxval(vector <double>);
		double minval(vector <double>);
		double maxval(vector <struct SimulationComponent>);
		double minval(vector <struct SimulationComponent>);
		void initialize_3d(unsigned int val1, unsigned int val2, unsigned int val3, double ****matrix);
		void initialize_2d(unsigned int val1, unsigned int val2, double ***matrix);
		void clear_3d(unsigned int val1, unsigned int val2, double ***matrix);
		void clear_2d(unsigned int val1, double **matrix);
		void GlobalStiff(vector <double> *, double **, double **, double, double);
		void IntQT1(vector <double>, double, double, double**);
		void IntQTm(vector <double>, double, double, double**);
		void IntQTn2(vector <double>, double, double, double**);
		void IntQTn1(vector <double>, double, double, double**);
		void ComputeCoefMatrixFixedMesh(double, double, double **, double **);
		void ComputeCoefMatrixMovingMeshR(double, double, double **, double **);
		void ComputeCoefMatrixMovingMeshL(double, double, double **, double **);
		void tridiag(double *, double *, double *, double *, double *);
		void LambdaG(unsigned int, double, double, double, double **);
		void block_append(unsigned int /*row start*/, unsigned int /*num rows*/,
								double ** /*Lm in*/, double ** /*Lam out */);
		void DefineFkp(unsigned int, double **);
		double AreaT(vector <double> *, vector <double> *);
		void BasisTS(double, double, double *, double *, double *);
		void BasisQS(double, double, double *, double *, double *);
		void BasisTR(vector <double>, vector <double>, double, double, double *, double *, double *);
		void BasisQR(vector <double>, double, double, double *, double *, double *);
		double Integrand(double, double, double, double, double, double, double, double);
		void DefineGaussian(unsigned int, double **);
		void DefInitCond(double **);
      void QuadSolver(double *, double *, double *, double *, double *, double *);
		void ReactionOneStep_linear(double **);
      int DecomposeCT(double *, double **);
      double find_C1_mono_Nmer(int, double, double);
		double cube_root(double, double, double);
		void print_vector(vector <double> *);
		void print_vector(double *, unsigned int);

		// interpolate maps a simulated grid with a variable delta_r grid onto a 
		// fixed delta_r grid from experimental data, and also interpolates time

		int interpolate (struct mfem_data *, /* simulated solution */
		                 unsigned int,       /* number of scans in expt. data, time dimension */
		                 unsigned int,       /* number of points in expt. data, radius dimension */
		                 float *,            /* scan times */
		                 double *,           /* radius values from expt. data */
		                 double **);          /* concentration values from expt. data, first dimension = time, second dimension radius */
		int interpolate (struct mfem_data *,  /* simulated solution */
		                 struct mfem_data *); /* experimental solution */
		void interpolate_C0(mfem_initial *, double *); // interpolate starting concentration vector mfem_initial onto C0
		void interpolate_Cfinal(mfem_initial *, double *); // interpolate final concentration back onto mfem_initial
//		void ReactionOneStep_ODE(double **);
//		void Rosenbrock_45(double *, double, unsigned int, unsigned int);
//		bool Rosenbrock_45_OneStep(double *, double, double *, double *, double, double *);
//		void derivatives(double *, double *);
//		void jacobian(double **, double, double *);
		void print_af();
};

#endif

