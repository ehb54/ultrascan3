#ifndef US_ASTFEM_MATH_H
#define US_ASTFEM_MATH_H

#include <math.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <climits>
#include <qobject.h>

#include "us_extern.h"
#include "us_matrix.h"
#include "us_femglobal.h"
#include "us_model_editor.h"
#include "us_simulationparameters.h"
#include "us_stiffbase.h"


using namespace std;
struct ComponentRole
{
	unsigned int comp_index;		// index of this component
#ifdef WIN32
#pragma warning ( disable: 4251 )
#endif
	vector <unsigned int> assoc;	// assoc vector index where this component occurs
	vector <int> react;				// role of component in each association, = 1: if as reactant; =-1, if as product
	vector <unsigned int> st;		// stoichiometry of each component in each assoc., index is linked to assoc.
#ifdef WIN32
#pragma warning ( disable: 4251 )
#endif
};

struct ReactionGroup
{
#ifdef WIN32
#pragma warning ( disable: 4251 )
#endif

	vector <unsigned int> association;
	vector <unsigned int> GroupComponent;

#ifdef WIN32
#pragma warning ( default: 4251 )
#endif
};

struct AstFemParameters
{
	unsigned int simpoints;

#ifdef WIN32
	  #pragma warning ( disable: 4251 )
#endif

	vector <double> s;			// sedimentation coefficient
	vector <double> D;			// Diffusion coefficient
	vector <double> kext;		// extinctiom coefficient
	vector <struct ComponentRole> role; // role of each component in various reactions

#ifdef WIN32
	  #pragma warning ( default: 4251 )
#endif

	double pathlength;			// path length of centerpiece;
	double dt;						// time step size;
	unsigned int time_steps;	// number of time steps for simulation
	double omega_s;				// omega^2
	double start_time;			// start time in seconds of simulation at constant speed
	double current_meniscus;	// actual meniscus for current speed
	double current_bottom;		// actual bottom for current speed
	unsigned int first_speed;	// constant speed at first speed step
	unsigned int rg_index;		// reaction group index
	vector <unsigned int> local_index;	// local index of each GroupComponent involved in a reaction group
	vector <struct Association> association;	// all association rules in a reaction group, with comp expressed in local index
};


double maxval(vector <double>);
double minval(vector <double>);
double maxval(vector <struct SimulationComponent>);
double minval(vector <struct SimulationComponent>);
void initialize_3d(unsigned int val1, unsigned int val2, unsigned int val3, double ****matrix);
void initialize_2d(unsigned int val1, unsigned int val2, double ***matrix);
void clear_3d(unsigned int val1, unsigned int val2, double ***matrix);
void clear_2d(unsigned int val1, double **matrix);
void tridiag(double *, double *, double *, double *, double *, unsigned int);
double Integrand(double, double, double, double, double, double, double, double);
void DefineGaussian(unsigned int, double **);
void IntQT1(vector <double>, double, double, double**, double);
void IntQTm(vector <double>, double, double, double**, double);
void IntQTn2(vector <double>, double, double, double**, double);
void IntQTn1(vector <double>, double, double, double**, double);
void QuadSolver(double *, double *, double *, double *, double *, double *, unsigned int);
void DefineFkp(unsigned int, double **);
double AreaT(vector <double> *, vector <double> *);
void BasisTS(double, double, double *, double *, double *);
void BasisQS(double, double, double *, double *, double *);
void BasisTR(vector <double>, vector <double>, double, double, double *, double *, double *);
void BasisQR(vector <double>, double, double, double *, double *, double *, double);
void IntQT1_ellam(vector <double>, double, double, double**, double);
void IntQTm_ellam(vector <double>, double, double, double**, double);
void IntQTn1_ellam(vector <double>, double, double, double**, double);
void QuadSolver_ellam(double *, double *, double *, double *, double *, double *, unsigned int);
double IntConcentration(vector<double>, double *);
double find_C1_mono_Nmer(int, double, double);
double cube_root(double, double, double);
int GaussElim (int, double **, double *);
void DefInitCond(double **, unsigned int);
void interpolate_C0(mfem_initial *, double *, vector <double> *); // interpolate starting concentration vector mfem_initial onto C0
void interpolate_C0(struct mfem_initial *, struct mfem_initial *); // interpolate first onto second
void interpolate_Cfinal(struct mfem_initial *, double *, vector <double> *); // interpolate final concentration back onto mfem_initial

int interpolate (	struct mfem_data *,	// simulated solution
						unsigned int,			// number of scans in expt. data, time dimension
						unsigned int,			// number of points in expt. data, radius dimension
						float *,					// scan times
						double *,				// radius values from expt. data
						double **);				// concentration values from expt. data, first dimension = time, second dimension radius

int interpolate (	struct mfem_data *,	// simulated solution
					  	struct mfem_data *,	// experimental solution
						bool);					// use time or w^2t for interpolation
#endif





