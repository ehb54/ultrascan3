// c++ class for US_MovingFEM
// Written by E. Brookes 2004. ebrookes@cs.utsa.edu
// Based upon original MatLab code by Weiming Cao.
// Adapted for UltraScan by Borries Demeler (2004, 2005)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iostream>

#define MAX_MFEM_STEPS 50000

#define ERR_NONE    0
#define ERR_MEMORY  1
#define ERR_OUTFILE 2
#define ERR_TRIDIAG 3
#define ERR_PARAMS  4
#define ERR_DATA    5


//////////////////////////////////////////////////////////////////
//
// moving finite element method for Lamm equation
// based on space-time approach
//
// Update: 09/01/2004, Weiming Cao, (Original)
//
//
//////////////////////////////////////////////////////////////////
//
//  Parameters:
//    N: Number of points used for the r-direction (Actual number of pts 
//       may be slightly more than N due to adaptivity).
//
////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

#include "../include/us_mfem.h"

long used_schuck_grid;
long used_composite_grid;

// C++ wrapper
US_MovingFEM::US_MovingFEM(struct mfem_data *temp_data, bool temp_GUI, 
QObject *parent, const char *name) : QObject (parent, name)
{
	stop_now = false;
	error_flag = false;
	skipEvents = false;
	GUI = temp_GUI;
	data = temp_data;
	sufficient_params = 0;
	zero_data();
	sprintf(LastErrorMessage, "Parameters not initialized");
	LastErrorNumber = ERR_PARAMS;
}

void US_MovingFEM::zero_data()
{
	(*data).radius.clear();
	for (unsigned int i=0; i<(*data).scan.size(); i++)
	{
		(*data).scan[i].conc.clear();
	}
	(*data).scan.clear();

	mfem_N = 0;
	mfem_steps = 0;
	mfem_s = 0e0;
	mfem_D = 0e0;
	mfem_rpm = 0e0;
	mfem_dt = 0e0;

	mfem_t = (double *)0;
	mfem_Mass = (double *)0;
	mfem_x = (double *)0;
	mfem_c = (double *)0;
}

void US_MovingFEM::free_data()
{
	if(mfem_t)
	{
		free(mfem_t);
		mfem_t = (double *)0;
	}
	if(mfem_Mass)
	{
		free(mfem_Mass);
		mfem_Mass = (double *)0;
	}
	if(mfem_x)
	{
		free(mfem_x);
		mfem_x = (double *)0;
	}
	if(mfem_c)
	{
		free(mfem_c);
		mfem_c = (double *)0;
	}
}

US_MovingFEM::~US_MovingFEM()
{
	free_data();
}

void US_MovingFEM::stop()
{
	stop_now = true;
}

void US_MovingFEM::set_params(int N, double s, double D, double rpm, double total_t, 
double m, double b, double c0, struct mfem_initial *tmp_initCvector)
{

	initCvector = tmp_initCvector;
	error_flag = false;

// some sanity checks:

	if (N < 50)
	{
		std::cout << "Error: Not enough simulation points (min: 50)!\n";
		error_flag = true;
	}
	
	if (b < m)
	{
		std::cout << "Error: The meniscus position is larger than the bottom of the cell position!\n";
		error_flag = true;
	}
	if (c0 == 0.0 && (*initCvector).concentration.size() == 0)
	{
		std::cout << "Error: There is no initial concentration!\n";
		error_flag = true;
	}
	
	if (s <= 0.0)
	{
		std::cout << "Error: The sedimentation coefficient needs to be larger than zero!\n";
		error_flag = true;
	}
	
	if (D <= 0.0)
	{
		std::cout << "Error: The diffusion coefficient needs to be larger than zero!\n";
		error_flag = true;
	}
	
	if (rpm <= 0.0)
	{
		std::cout << "Error: The speed needs to be larger than zero!\n";
		error_flag = true;
	}
	
	if (total_t <= 0.0)
	{
		std::cout << "Error: The time of the experiment needs to be larger than zero!\n";
		error_flag = true;
	}

	this->N = N;
	this->s = s;
	this->D = D;
	this->rpm = rpm;
	this->total_t = total_t;
	this->m = m;
	this->b = b;
	this->c0 = c0;
	//        printf("N: %d s: %e D: %e rpm: %e total_time: %e m: %e b: %e c0: %e\n", N, s, D, rpm, total_t, m, b, c0);


	LastErrorMessage[0] = 0;
	LastErrorNumber = 0;
}

US_MovingFEM::US_MovingFEM(int N, double s, double D, double rpm, double total_t, 
double m, double b, double c0, struct mfem_data *temp_data, bool temp_GUI,
struct mfem_initial *tmp_initCvector, QObject *parent, const char *name)
: QObject (parent, name)
{
	initCvector = tmp_initCvector;
	stop_now = false;
	GUI = temp_GUI;
	data = temp_data;
	zero_data();
	set_params(N, s, D, rpm, total_t, m, b, c0, initCvector);
}

void US_MovingFEM::set_N(int N)
{
	this->N = N;
}

void US_MovingFEM::set_s(double s)
{
	this->s = s;
}

void US_MovingFEM::set_D(double D)
{
	this->D = D;
}

void US_MovingFEM::set_rpm(double rpm) 
{
	this->rpm = rpm;
}

void US_MovingFEM::set_total_t(double total_t)
{
	this->total_t = total_t;
}

void US_MovingFEM::set_m(double m) 
{
	this->m = m;
}

void US_MovingFEM::set_b(double b) 
{
	this->b = b;
}

void US_MovingFEM::set_c0(double c0) 
{
	this->c0 = c0;
}

int US_MovingFEM::get_N(void) 
{
	return N;
}

double US_MovingFEM::get_s(void) 
{
	return s;
}

double US_MovingFEM::get_D(void) 
{
	return D;
}

double US_MovingFEM::get_rpm(void) 
{
	return rpm;
}

double US_MovingFEM::get_total_t(void)
{
	return total_t;
}

double US_MovingFEM::get_m(void) 
{
	return m;
}

double US_MovingFEM::get_b(void) 
{
	return b;
}

double US_MovingFEM::get_c0(void) 
{
	return c0;
}

int US_MovingFEM::run() 
{
	if(error_flag) 
	{
		sprintf(LastErrorMessage, "Error in Parameter initialization!");
		LastErrorNumber = ERR_PARAMS;
		fprinterror(stderr, "run()");
		std::cout << "Error in Parameter initialization!\n";
		return -1;
	}
	free_data();
	zero_data();
	mfem_rpm = rpm;
	mfem_s = s;
	mfem_D = D;
	return mfem();
}

int US_MovingFEM::tridiag(int N, double *M, double *r, double *u) 
{

	int j;
	int jt3;

	double bet;
	double *gam;
	gam = new double [N];

	bet = M[1];
	if(bet == 0e0) 
	{
		sprintf(LastErrorMessage, "Error 1 in Tridiag System");
		LastErrorNumber = ERR_TRIDIAG;
		std::cout << "Error 1 in Tridiag System\n";
		return -1;
	}
	u[0] = r[0] / bet;
	for(j=1; j<N; j++)
	{
		jt3 = j * 3;
		gam[j] = M[jt3 - 3 + 2]/bet;
		bet=M[jt3+1]-M[jt3]*gam[j];
		if(bet == 0e0)
		{
			sprintf(LastErrorMessage, "Error 2 in Tridiag System");
			LastErrorNumber = ERR_TRIDIAG;
			std::cout << "Error 1 in Tridiag System\n";
			return -1;
		}
		u[j]=(r[j]-M[jt3]*u[j-1])/bet;
	}

	for(j=N-2;j>=0;j--)
	{
		u[j]-= gam[j+1]*u[j+1];
	}
	delete [] gam;
	return 0;
}

//////////////////////////////////
// calculate total mass
// (r,u) concentration defined at r(1), ...., r(M)
//  M: r(1).... r(M): the interval for total mass, (M-1) subintervals
//////////////////////////////////


double US_MovingFEM::IntConcentration(double *r, int M,double *u)
{
	double T=0e0;
	int j;
	for(j=1; j<= M-1; j++)
	{
		T=T+(r[j+1]-r[j])*((r[j+1]-r[j])*(u[j+1]/3e0+u[j]/6e0)+r[j]*(u[j]+u[j+1])/2e0);
	}
	return T;
}

int US_MovingFEM::mfem()
{
	int N = this->N;
	double m = this->m;
	double b = this->b;
	double rpm = this->rpm;
	double para_s = this->s;
	double para_D = this->D;
	double total_t = this->total_t;
	double m_2;
	double b_2 ;
	double para_w;
	double para_w_2 ;
	double dt;
	double nu;
	double uth;
	double DX;
	double xc;
	double xa;
	int Nf;
	double H_star;
	double Hf;
	double xj;
	double xj0;
	double xj1;
	double xj2;
	double hj;
	double hj0;
	double hj1;
	double xj_2;
	double xj0_2;
	double xj1_2;
	double xj2_2;
	double hj_2;
	double hj0_2;
	double hj1_2;
	double hj_3;
	double hj0_3;
	double hj1_3;
	double hj1_hj_3;
	double hj0_hj_3;

	double CuA;
	double CvA;
	double CuB;
	double CvB;
	double TmA;
	double TmB;

	double BB1;
	double BB2;
	double BB3;
	double BB4;

	double BA1;
	double BA2;
	double BA3;
	double BA4;

	int i;
	int it3pj;
	int j;
	int j0;
	int j1;
	int j1t3;
	int j2;
	int js;
	int Nm;
	int kkk;

	double ABu;
	double ABv;
	double AB0;
	double AB1;
	double AAu;
	double AAv;
	double AA0;
	double AA1;

	double t;


	m_2= m * m;
	b_2 = b * b;
	para_w=M_PI/30 * rpm;      // rpm/60*2*pi=angular speed per sec
	para_w_2 = para_w * para_w;

	//  total_t=2*(b-m)/(para_s*para_w_2*m);     // total_time for simulation

	dt=log(b/m)/(para_w_2*para_s*(N-1));     // delta_t by Schuck's formula
	if(dt > total_t / 2.0) {
	  dt = total_t / 2.0;
	}

	// ////////////////////////////////////////////////////////
	// Determine the adaptive Mesh
	// ////////////////////////////////////////////////////////

	nu=para_s*para_w_2/para_D;

	uth=1e0/N;		// thrshold of u for steep region
	DX=1e0/(nu*b)*log(nu*(b_2-m_2)/(2e0*uth));	// width of steep region
	
	xc=b-DX;

	Nf = (int)(0.5 + M_PI/2*(b-xc)*nu*b/2)+1;	// number of points in steep region
	H_star=(b-xc)/Nf*M_PI/2;              // step required by Pac<1

	if(dt <= 0 || Nf < 1 || DX <= 0 || dt > total_t) {
	  fprintf(stderr, "us_mfem.o : dt %g Nf %d DX %g when s %g D %g total_t %g\n",
		  dt,
		  Nf,
		  DX,
		  para_s,
		  para_D,
		  total_t);
	  if(DX < 0) {
	    DX = 1;
	    xc=b-DX;
	    Nf = (int)(0.5 + M_PI/2*(b-xc)*nu*b/2)+1;	// number of points in steep region
	    if(Nf < 2) {
	      Nf = 2;
	    }
	    H_star=(b-xc)/Nf*M_PI/2;              // step required by Pac<1
	    fprintf(stderr, "us_mfem.o : corrected dt %g Nf %d DX %g when s %g D %g total_t %g\n",
		  dt,
		  Nf,
		  DX,
		  para_s,
		  para_D,
		  total_t);
	  }
	}

	double *x;
	if(b-m*pow((b/m),((N-5e0/2e0)/(N-1e0))) < H_star)
	{  // use Schuck's grid only
		used_schuck_grid++;
		if(!(x = (double *)malloc(N*sizeof(double))))
		{
			sprintf(LastErrorMessage, "Memory allocation error (2)");
			LastErrorNumber = ERR_MEMORY;
			fprinterror(stderr, "malloc()");
			std::cout << "Memory allocation error (2): x[]\n";
			return -1;
		}
		x--;   // offset vector to start at 1
		//      puts("using Schuck grid completely");
		x[1]=m;
		x[N]=b;
		for(j=2; j < N; j++)
		{
			x[j]=m*pow((b/m),((j-3e0/2e0)/(N-1e0)));
		}
	}
	else
	{				// need a composite grid
		used_composite_grid++;
		double *y;
		y = new double [Nf+1];

		// steep region
		y[1]=b;
		for (j=1; j<= Nf; j++)
		{
			y[Nf-j+1]=xc+(b-xc)*sin((j-1e0)/(Nf-1e0)*M_PI/2);
		}

		// use equilibrium solution //%
		// for j=2:Nf, y[j]=sqrt( b2+ 2e0/nu*log((Nf-j+uth)/Nf) ); end;
		// xc=y[Nf];
		// end of use equilibrium solution //%

		// transition region
		// smallest step size in transit region
		Hf=y[Nf-1]-y[Nf];     	// Hf = (b-xc)/Nf*M_PI/2;

		Nm=(int)(0.5 + log(b/((N-1e0)*Hf)*log(b/m)))+1; // number of pts in trans region
		double *yNm;
		yNm= new double[Nm+1];
		xa=xc-(pow((double)2,(double)Nm)-1e0)*Hf;
		for(j=1; j<= Nm; j++)
		{
			yNm[j]=xc-Hf*(pow((double)2,(double)j)-1e0);
			// y[j+Nf]=xc-Hf*(pow(2,j)-1e0);
		}
		// regular region
		js=(int)floor(3e0/2e0+(N-1e0)*log(xa/m)/log(b/m));
		if(!(x = (double *)malloc((js+Nf+Nm)*sizeof(double))))
		{
			sprintf(LastErrorMessage, "Memory allocation error (1)");
			LastErrorNumber = ERR_MEMORY;
			fprinterror(stderr, "malloc()");
			std::cout << "Memory allocation error (1): x[]\n";
			return -1;
		}
		x--;   // offset vector to start at 1
		x[1]=m;
		for (j=2; j <= js; j++)
		{
			x[j]=m*pow((b/m),((j-3e0/2e0)/(N-1e0)));
		}
		for (j=1; j <= Nm; j++)
		{
			x[j+js]=yNm[Nm+1-j];
		}
			for (j=1; j <= Nf; j++)
		{
			x[Nm+j+js]=y[Nf+1-j];
		}
		x[js]=(x[js+1]+x[js-1])/2;
		x[js+1]=(x[js]+x[js+2])/2;
		N=js+Nf+Nm;
		delete [] y;
		delete [] yNm;
	}

	// assign radius to data array structure:
	for (j=1; j<=N; j++)
	{
//		cout << "Mesh radius [" << j << "]: " << x[j] << endl;
		(*data).radius.push_back(x[j]);
	}
	double *p_d;
	p_d = new double [N];
	double *d = &(p_d[-1]);  // offset to start at 1 instead of zero
	
	double *p_u0;
	p_u0 = new double [N];
	double *u0 = &(p_u0[-1]);
	double *p_u1;
	p_u1 = new double [N];
	double *u1 = &(p_u1[-1]);

	double *Mut0;  // matrix for u_t term
	double *Mut1;
	double *Mdu0;  // matrix for du term
	double *Mdu1;
	double *Mx20;   // matrix for x2 term
	double *Mx21;
	double *Mv;
	double *Mu;
	
	Mut0 = new double [(N)*3];
	Mut1 = new double [(N)*3];
	Mdu0 = new double [(N)*3];
	Mdu1 = new double [(N)*3];
	Mx20 = new double [(N)*3];
	Mx21 = new double [(N)*3];
	Mv = new double [(N)*3];
	Mu = new double [(N)*3];

	for(j = 0; j < N*3; j++)
	{
		Mut0[j]=Mut1[j]=
		Mdu0[j]=Mdu1[j]=
		Mx20[j]=Mx21[j]=
		Mv[j]=Mu[j]= 0e0;
	}

	//    Mut0=zeros(N,3); Mut1=zeros(N,3);

	for(j=2; j <= N-2; j++)
	{
		j0=j-1;
		j1=j+1;
		j1t3 = (j1 - 1) * 3;
		j2=j+2;
		xj=x[j];
		xj0=x[j0];
		xj1=x[j1];
		xj2=x[j2];
		hj0=xj-xj0;
		hj=xj1-xj;
		hj1=xj2-xj1;

		ABu=1e0/72e0*(-5*hj*xj1-6*xj*hj-xj2*hj1-hj*xj2-2*xj*hj1-3*xj1*hj1);
		ABv=1e0/72e0*(-3*xj*hj-xj*hj1-7*hj*xj1-3*xj1*hj1-2*hj*xj2-2*xj2*hj1);
		AB0=1e0/72e0*( 5*hj*xj1+6*xj*hj+2*xj2*hj1+hj*xj2+3*xj*hj1+7*xj1*hj1);
		AB1=1e0/72e0*( 2*xj*hj+xj*hj1+3*hj*xj1+3*xj1*hj1+hj*xj2+2*xj2*hj1);

		AAu=1e0/72e0*(-3*hj0*xj-hj0*xj1-2*xj0*hj0-2*xj0*hj-3*hj*xj1-7*xj*hj);
		AAv=1e0/72e0*(-xj0*hj0-xj0*hj-3*hj0*xj-5*xj*hj-2*hj0*xj1-6*hj*xj1);
		AA0=1e0/72e0*( 3*hj0*xj+hj0*xj1+2*xj0*hj0+xj0*hj+2*hj*xj1+3*xj*hj);
		AA1=1e0/72e0*( 2*xj0*hj0+xj0*hj+7*hj0*xj+5*xj*hj+3*hj0*xj1+6*hj*xj1);

		Mut1[j1t3 + 2]= ABv+AB1; // --- vj2 ---
		Mut1[j1t3 + 1]=-ABv+AB0 +AAv+AA1; // --- vj1 ---
		Mut1[j1t3]=-AAv+AA0; // ---vj---

		Mut0[j1t3 + 2]= ABu-AB1; // --- uj1 ---
		Mut0[j1t3 + 1]=-ABu-AB0+AAu-AA1; // --- uj ---
		Mut0[j1t3]=-AAu-AA0; // --- uj0 ---

	}

	// ////////////////////// from P0ut ////////////////////%

	j=1;
	j1=j+1;
	j2=j+2;
	xj=x[j];
	xj1=x[j1];
	xj2=x[j2];

	Mut1[1]= -1e0/24e0*(3e0*xj+xj1)*(xj-xj1);
	Mut0[2]=  1e0/24e0*(3e0*xj+xj1)*(xj-xj1);

	// ////////////////////// from P1ut ////////////////////%

	j=1;
	j1=j+1;
	j1t3 = (j1 - 1) * 3;
	j2=j+2;
	xj=x[j];
	xj1=x[j1];
	xj2=x[j2];
	hj=xj1-xj;
	hj1=xj2-xj1;

	ABu=1e0/72*(-5*hj*xj1-6*xj*hj-xj2*hj1-hj*xj2-2*xj*hj1-3*xj1*hj1);
	ABv=1e0/72*(-3*xj*hj-xj*hj1-7*hj*xj1-3*xj1*hj1-2*hj*xj2-2*xj2*hj1);
	AB0=1e0/72*( 5*hj*xj1+6*xj*hj+2*xj2*hj1+hj*xj2+3*xj*hj1+7*xj1*hj1);
	AB1=1e0/72*( 2*xj*hj+xj*hj1+3*hj*xj1+3*xj1*hj1+hj*xj2+2*xj2*hj1);

	Mut1[j1t3 + 2]= ABv+AB1; // --- vj2 ---
	Mut1[j1t3 + 1]=-ABv+AB0; // --- vj1 ---
	Mut1[j1t3]=-1e0/24*(5*xj+3*xj1)*(xj-xj1); // ---vj---

	Mut0[j1t3 + 2]= ABu-AB1; // --- uj1 ---
	Mut0[j1t3 + 1]=-ABu-AB0+1e0/24*(5*xj+3*xj1)*(xj-xj1); // --- uj ---


	//  //////////////// PNut ////////////////////%
	j=N-1;
	j0=j-1;
	j1=j+1;
	j1t3 = (j1 - 1) * 3;
	xj=x[j];
	xj1=x[j1];
	xj0=x[j0];
	hj0=xj-xj0;
	hj=xj1-xj;

	AAu=1e0/72*(-3*hj0*xj-hj0*xj1-2*xj0*hj0-2*xj0*hj-3*hj*xj1-7*xj*hj);
	AAv=1e0/72*(-xj0*hj0-xj0*hj-3*hj0*xj-5*xj*hj-2*hj0*xj1-6*hj*xj1);
	AA0=1e0/72*( 3*hj0*xj+hj0*xj1+2*xj0*hj0+xj0*hj+2*hj*xj1+3*xj*hj);
	AA1=1e0/72*( 2*xj0*hj0+xj0*hj+7*hj0*xj+5*xj*hj+3*hj0*xj1+6*hj*xj1);

	Mut1[j1t3 + 1]= +AAv+AA1-1e0/6*(xj+2*xj1)*(xj-xj1); // --- vj1 ---
	Mut1[j1t3]=-AAv+AA0; // ---vj---

	Mut0[j1t3 + 2]= +1e0/6*(xj+2*xj1)*(xj-xj1) ; // --- uj1 ---
	Mut0[j1t3 + 1]=+AAu-AA1; // --- uj ---
	Mut0[j1t3]=-AAu-AA0; // --- uj0 ---

	//    Mdu0=zeros(N,3); Mdu1=zeros(N,3);

	for(j=2; j<= N-2; j++)
	{
		j0=j-1;
		j1=j+1;
		j1t3 = (j1 - 1) * 3;
		j2=j+2;
		xj=x[j];
		xj0=x[j0];
		xj1=x[j1];
		xj2=x[j2];
		hj=xj1-xj;
		hj_2 = hj * hj;
		hj_3 = hj_2 * hj;
		hj1=xj2-xj1;
		hj1_2 = hj1 * hj1;
		hj1_3 = hj1_2 * hj1;
		hj0=xj-xj0;
		hj0_2= hj0 * hj0;
		hj0_3= hj0_2 * hj0;
		hj1_hj_3 = (-hj + hj1) * (-hj + hj1) * (-hj + hj1);
		hj0_hj_3 = (hj0 - hj) * (hj0 - hj) * (hj0 - hj);
		if(fabs(hj/hj1-1)<0.001)
		{ 		// when hj=hj1
			CuB = -1e0/2*dt*(1e0/6*(-xj+xj2)/hj+1e0/2*(xj+xj1)/hj);
			CvB = -1e0/2*dt*(1e0/3*(-xj+xj2)/hj+1e0/2*(xj+xj1)/hj);
		}
		else
		{
			TmB=log(hj1/hj);
			CuB = -1e0/4*dt*(4*hj1_2*xj1*(TmB)-4*hj_2*xj1-4*hj1_2*hj*(TmB)
		 		+hj_3+3*hj1_2*hj-4*hj1*xj1*hj*(TmB)-4*hj1_2*xj1-5*hj1*hj_2
		 		+8*hj1*xj1*hj+hj1_3)/(hj1_hj_3);
			CvB = -1e0/4*dt*(-8*hj1*xj1*hj+4*hj_2*xj1+4*hj_2*xj1*(TmB)
		 		+4*hj1_2*xj1+3*hj1*hj_2-5*hj1_2*hj+hj1_3+hj_3
		 		-4*hj1*xj1*hj*(TmB)+4*hj1*hj_2*(TmB))/(hj1_hj_3);
		}
		if(fabs(hj/hj0-1)<0.001)
		{ 		// when hj=hj1
			CuA = 1e0/2*dt*(1e0/6*(-xj0+xj1)/hj0+1e0/2*(xj0+xj)/hj0);
			CvA = 1e0/2*dt*(1e0/3*(-xj0+xj1)/hj0+1e0/2*(xj0+xj)/hj0);
		}
		else
		{
			TmA=log(hj/hj0);
			CuA = -1e0/4*dt*(4*hj_2*xj*(TmA)+3*hj_2*hj0-4*hj0_2*xj-5*hj0_2*hj
		 		+hj0_3+hj_3-4*hj0*xj*hj*(TmA)-4*hj_2*xj-4*hj_2*hj0*(TmA)
		 		+8*hj0*xj*hj)/(hj0_hj_3);
			CvA = 1e0/4*dt*(-hj0_3+5*hj_2*hj0-3*hj0_2*hj+8*hj0*xj*hj
				-4*hj0_2*xj*(TmA)-hj_3-4*hj0_2*xj-4*hj_2*xj
				+4*hj0*xj*hj*(TmA)-4*hj0_2*hj*(TmA))/(hj0_hj_3);
		}
		Mdu1[j1t3 + 2]= CvB; 	// --- vj2 ---
		Mdu1[j1t3 + 1]= -CvB+CvA; 	// --- vj1 ---
		Mdu1[j1t3]= -CvA;	// --- vj  ---
		Mdu0[j1t3 + 2]= CuB;		//--- uj1 ---
		Mdu0[j1t3 + 1]= -CuB+CuA; 	//--- uj  ---
		Mdu0[j1t3]= -CuA; 	//--- uj0 ---
	}

	//   ////////////////////// from P0du ////////////////////%
	j=1;
	j1=j+1;
	xj=x[j];
	xj1=x[j1];
	Mdu1[2]= -1e0/6*dt*(2*xj+xj1)/(-xj+xj1);
	Mdu1[1]=  1e0/6*dt*(2*xj+xj1)/(-xj+xj1);

	//    ////////////////////// from P1du ////////////////////%

	j=1;
	j1=j+1;
	j1t3 = (j1 - 1) * 3;
	j2=j+2;
	xj=x[j];
	xj1=x[j1];
	xj2=x[j2];
	hj=xj1-xj;
	hj_2 = hj * hj;
	hj_3 = hj_2 * hj;
	hj1=xj2-xj1;
	hj1_2 = hj1 * hj1;
	hj1_3 = hj1_2 * hj1;
	hj1_hj_3 = (-hj + hj1) * (-hj + hj1) * (-hj + hj1);

	if(fabs(hj/hj1-1)<0.001)
	{ 		// when hj=hj1

		CuB = -1e0/2*dt*(1e0/6*(-xj+xj2)/hj+1e0/2*(xj+xj1)/hj);
		CvB = -1e0/2*dt*(1e0/3*(-xj+xj2)/hj+1e0/2*(xj+xj1)/hj);
	}
	else
	{
		TmB=log(hj1/hj);

		CuB = -1e0/4*dt*(4*hj1_2*xj1*(TmB)-4*hj_2*xj1-4*hj1_2*hj*(TmB)
	 		+hj_3+3*hj1_2*hj-4*hj1*xj1*hj*(TmB)-4*hj1_2*xj1-5*hj1*hj_2
	 		+8*hj1*xj1*hj+hj1_3)/(hj1_hj_3);

		CvB = -1e0/4*dt*(-8*hj1*xj1*hj+4*hj_2*xj1+4*hj_2*xj1*(TmB)
	 		+4*hj1_2*xj1+3*hj1*hj_2-5*hj1_2*hj+hj1_3+hj_3
	 		-4*hj1*xj1*hj*(TmB)+4*hj1*hj_2*(TmB))/(hj1_hj_3);
	}

	Mdu1[j1t3 + 2]= CvB; 	// --- vj2 ---
	Mdu1[j1t3 + 1]= +1e0/6e0*dt*(2*xj+xj1)/(-xj+xj1) - CvB; //--- vj1 ---
	Mdu1[j1t3]= -1e0/6e0*dt*(2*xj+xj1)/(-xj+xj1); //--- vj  ---

	Mdu0[j1t3 + 2]= CuB;		//--- uj1 ---
	Mdu0[j1t3 + 1]= -CuB; 	//--- uj  ---


	//      ////////////////////// from PNdu = PAdu ////////////////////%

	j=N-1;
	j1=j+1;
	j1t3 = (j1 - 1) * 3;
	j0=j-1;
	xj=x[j];
	xj1=x[j1];
	xj0=x[j0];
	hj=xj1-xj;
	hj_2 = hj * hj;
	hj_3 = hj_2 * hj;
	hj0=xj-xj0;
	hj0_2 = hj0 * hj0;
	hj0_3 = hj0_2 * hj0;
	hj0_hj_3 = (hj0 - hj) * (hj0 - hj) * (hj0 - hj);

	if(fabs(hj/hj0-1)<0.001)
	{ 		// when hj=hj1
		CuA = 1e0/2e0*dt*(1e0/6e0*(-xj0+xj1)/hj0+1e0/2e0*(xj0+xj)/hj0);
		CvA = 1e0/2e0*dt*(1e0/3e0*(-xj0+xj1)/hj0+1e0/2e0*(xj0+xj)/hj0);
	}
	else
	{
		TmA=log(hj/hj0);

		CuA = -1e0/4e0*dt*(4*hj_2*xj*(TmA)+3*hj_2*hj0-4*hj0_2*xj-5*hj0_2*hj
			+hj0_3+hj_3-4*hj0*xj*hj*(TmA)-4*hj_2*xj-4*hj_2*hj0*(TmA)
		 	+8*hj0*xj*hj)/(hj0_hj_3);

		CvA = 1e0/4e0*dt*(-hj0_3+5*hj_2*hj0-3*hj0_2*hj+8*hj0*xj*hj
			-4*hj0_2*xj*(TmA)-hj_3-4*hj0_2*xj-4*hj_2*xj
			+4*hj0*xj*hj*(TmA)-4*hj0_2*hj*(TmA))/(hj0_hj_3);
	}

	Mdu1[j1t3 + 1]= +CvA; 	//--- vj1 ---
	Mdu1[j1t3]= -CvA;	//--- vj  ---

	Mdu0[j1t3 + 1]= +CuA; 	//--- uj  ---
	Mdu0[j1t3]= -CuA; 	//--- uj0 ---

	//        Mx20=zeros(N,3); Mx21=zeros(N,3);

	for(j=2; j<= N-2; j++)
	{
		j0=j-1;
		j1=j+1;
		j1t3 = (j1 - 1) * 3;
		j2=j+2;
		xj=x[j];
		xj_2 = xj * xj;
		xj0=x[j0];
		xj0_2 = xj0 * xj0;
		xj1=x[j1];
		xj1_2 = xj1 * xj1;
		xj2=x[j2];
		xj2_2 = xj2 * xj2;
		hj0=xj-xj0;
		hj=xj1-xj;
		hj1=xj2-xj1;

		//--- from Px2 ---
		BB1= -dt/144e0*(9*xj_2+12*xj*xj1+2*xj*xj2+8*xj1_2+4*xj1*xj2+xj2_2);
		BB2= -dt/144e0*(3*xj_2+8*xj*xj1+2*xj*xj2+12*xj1_2+8*xj1*xj2+3*xj2_2);
		BB3= -dt/144e0*(3*xj_2+8*xj*xj1+2*xj*xj2+12*xj1_2+8*xj1*xj2+3*xj2_2);
		BB4= -dt/144e0*(xj_2+4*xj*xj1+2*xj*xj2+8*xj1_2+12*xj1*xj2+9*xj2_2);

		BA1=  dt/144e0*(9*xj0_2+12*xj0*xj+2*xj0*xj1+8*xj_2+4*xj*xj1+xj1_2);
		BA2=  dt/144e0*(3*xj0_2+8*xj0*xj+2*xj0*xj1+12*xj_2+8*xj*xj1+3*xj1_2);
		BA3=  dt/144e0*(3*xj0_2+8*xj0*xj+2*xj0*xj1+12*xj_2+8*xj*xj1+3*xj1_2);
		BA4=  dt/144e0*(xj0_2+4*xj0*xj+2*xj0*xj1+8*xj_2+12*xj*xj1+9*xj1_2);

		Mx21[j1t3 + 2]= BB4; 	// --- vj2 ---
		Mx21[j1t3 + 1]= BB3+BA4; 	// --- vj1 ---
		Mx21[j1t3]= BA3; 	// --- vj  ---

		Mx20[j1t3 + 2]= BB2; 	// --- uj1 ---
		Mx20[j1t3 + 1]= BB1+BA2; 	// --- uj  ---
		Mx20[j1t3]= BA1; 	// --- uj0 ---
	}

	////////////////////// from P0x2 ////////////////////%
	j=1;
	j1=j+1;
	xj=x[j];
	xj_2 = xj * xj;
	xj1=x[j1];
	xj1_2 = xj1 * xj1;

	Mx21[2]= -1e0/60e0*dt*( 3*xj_2+4*xj*xj1+3*xj1_2);

	Mx21[1]= -1e0/60e0*dt*( 6*xj_2+3*xj*xj1+xj1_2);

	Mx20[2]= -1e0/60e0*dt*( 6*xj_2+3*xj*xj1+xj1_2);


	////////////////////// from P1x2 ////////////////////%

	j=1;
	j1=j+1;
	j1t3 = (j1 - 1) * 3;
	j2=j+2;
	xj=x[j];
	xj_2 = xj * xj;
	xj1=x[j1];
	xj1_2 = xj1 * xj1;
	xj2=x[j2];
	xj2_2 = xj2 * xj2;
	hj=xj1-xj;
	hj1=xj2-xj1;

	//--- from Px2 ---
	BB1= -dt/144e0*(9*xj_2+12*xj*xj1+2*xj*xj2+8*xj1_2+4*xj1*xj2+xj2_2);
	BB2= -dt/144e0*(3*xj_2+8*xj*xj1+2*xj*xj2+12*xj1_2+8*xj1*xj2+3*xj2_2);
	BB3= -dt/144e0*(3*xj_2+8*xj*xj1+2*xj*xj2+12*xj1_2+8*xj1*xj2+3*xj2_2);
	BB4= -dt/144e0*(xj_2+4*xj*xj1+2*xj*xj2+8*xj1_2+12*xj1*xj2+9*xj2_2);

	Mx21[j1t3 + 2]= BB4; 	// --- vj2 ---
	Mx21[j1t3 + 1]= BB3+dt/60e0*(3*xj_2+4*xj*xj1+3*xj1_2); // --- vj1 ---
	Mx21[j1t3]=     dt/60e0*(6*xj_2+3*xj*xj1+xj1_2); // --- vj  ---

	Mx20[j1t3 + 2]= BB2; 	// --- uj1 ---
	Mx20[j1t3 + 1]= BB1+dt/60e0*(6*xj_2+3*xj*xj1+xj1_2);  // --- uj  ---


	////////////////////// from PNx2 = PAx2 ////////////////////%

	j=N-1;
	j1=j+1;
	j1t3 = (j1 - 1) * 3;
	j0=j-1;
	xj=x[j];
	xj_2 = xj * xj;
	xj1=x[j1];
	xj1_2 = xj1 * xj1;
	xj0=x[j0];
	xj0_2 = xj0 * xj0;
	hj0=xj-xj0;
	hj=xj1-xj;

	BA1=  dt/144e0*(9*xj0_2+12*xj0*xj+2*xj0*xj1+8*xj_2+4*xj*xj1+xj1_2);
	BA2=  dt/144e0*(3*xj0_2+8*xj0*xj+2*xj0*xj1+12*xj_2+8*xj*xj1+3*xj1_2);
	BA3=  dt/144e0*(3*xj0_2+8*xj0*xj+2*xj0*xj1+12*xj_2+8*xj*xj1+3*xj1_2);
	BA4=  dt/144e0*(xj0_2+4*xj0*xj+2*xj0*xj1+8*xj_2+12*xj*xj1+9*xj1_2);

	Mx21[j1t3 + 1]= BA4; 	// --- vj1 ---
	Mx21[j1t3]= BA3; 	// --- vj  ---

	Mx20[j1t3 + 1]= BA2; 	// --- uj  ---
	Mx20[j1t3]= BA1; 	// --- uj0 ---


	////////////////////// end of coefficients //////////////////


	/////////////////////////////////////////////////////////////
	//
	// form the triangular linear system of equations
	//
	/////////////////////////////////////////////////////////////

	for(i=0; i < N; i++)
	{
		for(j=0; j < 3; j++)
		{
			it3pj = i * 3 + j;
			Mv[it3pj]=Mut1[it3pj]+para_D*Mdu1[it3pj]-para_s*para_w_2*Mx21[it3pj];
			Mu[it3pj]=-(Mut0[it3pj]+para_D*Mdu0[it3pj]-para_s*para_w_2*Mx20[it3pj]);
		}
	}

	// initial condition, assign initial, uniform concentration:
	if (c0 == 0.0)
	{
		u0[1] = (*initCvector).concentration[0];
		unsigned int ncount=1;
		double m, b;
		for(i=2 ; i <= N; i++)
		{
			while (ncount <(*initCvector).radius.size()-1 && x[i] >= (*initCvector).radius[ncount])
			{
				ncount++;
			}
			if (x[i] == (*initCvector).radius[ncount])
			{
				u0[i] = (*initCvector).concentration[ncount];
			}
			else
			{
				m = ((*initCvector).concentration[ncount] - (*initCvector).concentration[ncount-1])/
				    ((*initCvector).radius[ncount] - (*initCvector).radius[ncount-1]);
				b = (*initCvector).concentration[ncount] - m * (*initCvector).radius[ncount];
				u0[i] = m * x[i] + b;
			}
		}		
	}
	else
	{
		for(i=1 ; i <= N; i++)
		{
			u0[i] = c0;
		}
	}

	//////////////////////////////////////////////////////////////////
	//
	// time evolution
	//
	//////////////////////////////////////////////////////////////////

	//  figure(1); hold on;

	// allocate memory for output
	mfem_dt = dt;
	mfem_steps = (int)(0.5 + (total_t / dt));

	if(mfem_steps > MAX_MFEM_STEPS)
	{
		fprintf(stderr, "error: mfem steps > %d (actual: %d), dt: %e total_t: %e\n", MAX_MFEM_STEPS, mfem_steps, dt, total_t);
		if(x)
		{
			free(x+1);
		}
		return(-1);
	}

	mfem_N = N;
	if(!(mfem_t = (double *) malloc((mfem_steps + 1) * sizeof(double))))
	{
		sprintf(LastErrorMessage, "Memory allocation error (1)");
		LastErrorNumber = ERR_MEMORY;
		fprinterror(stderr, "malloc()");
		std::cout << "Memory allocation error (1): mfem_t[]\n";
		return -1;
	}
	if(!(mfem_Mass = (double *) malloc((mfem_steps + 1) * sizeof(double))))
	{
		sprintf(LastErrorMessage, "Memory allocation error (1)");
		LastErrorNumber = ERR_MEMORY;
		free(mfem_t);
		fprinterror(stderr, "malloc()");
		std::cout << "Memory allocation error (1): mfem_Mass[]\n";
		return -1;
	}
	if(!(mfem_x = (double *) malloc((mfem_steps + 1) * sizeof(double) * mfem_N)))
	{
		sprintf(LastErrorMessage, "Memory allocation error (1)");
		LastErrorNumber = ERR_MEMORY;
		free(mfem_t);
		free(mfem_Mass);
		fprinterror(stderr, "malloc()");
		std::cout << "Memory allocation error (1): mfem_x[]\n";
		return -1;
	}

	if(!(mfem_c = (double *) malloc((mfem_steps + 1) * sizeof(double) * mfem_N)))
	{
		sprintf(LastErrorMessage, "Memory allocation error (1)");
		LastErrorNumber = ERR_MEMORY;
		free(mfem_t);
		free(mfem_Mass);
		free(mfem_x);
		fprinterror(stderr, "malloc()");
		std::cout << "Memory allocation error (1): mfem_c[]\n";
		return -1;
	}

	struct mfem_scan current_scan;
	for(kkk=0; kkk <= mfem_steps; kkk++)
	{
       	        if(!skipEvents) {
		  //	  puts("processing events");
		  qApp->processEvents();
		} else {
		  //		  puts("skip events");
		}
		if (stop_now)
		{
			free(x+1);
			return(1);
		}
		t=kkk*dt;
		current_scan.time = t;
		current_scan.conc.clear();
		mfem_Mass[kkk] = IntConcentration(x,N,u0);
		mfem_t[kkk] = t;
		memcpy(&(mfem_x[mfem_N * kkk]), &(x[1]), mfem_N * sizeof(double));
		memcpy(&(mfem_c[mfem_N * kkk]), &(u0[1]), mfem_N * sizeof(double));

		// Calculate thr right hand side vector //
		d[1]=Mu[(0*3)+2]*u0[1];
		d[2]=Mu[(1*3)+1]*u0[1]+Mu[(1*3)+2]*u0[2];
		for(i=3; i <= N; i++)
		{
			j1t3 = (i-1) * 3;
			d[i]=Mu[j1t3]*u0[i-2]+Mu[j1t3+1]*u0[i-1]+Mu[j1t3+2]*u0[i];
		}

		// solution evolution
		if(tridiag(N, Mv, &(d[1]), &(u1[1])))
		{
			free_data();
			zero_data();
			fprinterror(stderr, "tridiag()");
			std::cout << "Error in calculation of tridiagonal matrix\n";
			return -1;
		}
		// add current scan to mfem data:
		for (i=1; i<=N; i++)
		{
			current_scan.conc.push_back(u0[i]);
		}
		if (GUI)
		{
			std::vector <double> radius;
			radius.clear();
			for (i=1; i<=N; i++)
			{
				radius.push_back(x[i]);
			}
			emit scan_updated(current_scan, radius);
		}
		(*data).scan.push_back(current_scan);
		for(i = 1; i <= N; i++)
		{
			u0[i]=u1[i];
		}
	}
	free(x+1);
	
	delete [] p_d;
	delete [] p_u0;
	delete [] p_u1;
	delete [] Mut0;  
	delete [] Mut1;
	delete [] Mdu0;  
	delete [] Mdu1;
	delete [] Mx20;   
	delete [] Mx21;
	delete [] Mv;
	delete [] Mu;

	return 0;
}

int US_MovingFEM::filewrite(char *OutFileName)
{

	FILE *fout;
	int kkk;
	int j;
	char *fwerror = "File write error";

	if(!sufficient_params)
	{
		sprintf(LastErrorMessage, "Parameters not initialized");
		LastErrorNumber = ERR_PARAMS;
		return -1;
	}

	if(!mfem_N)
	{
		sprintf(LastErrorMessage, "No run data");
		LastErrorNumber = ERR_DATA;
		return -1;
	}

	if(!(fout=fopen(OutFileName,"w")))
	{   // output data file
		sprintf(LastErrorMessage, "Can not open data file for writing");
		LastErrorNumber = ERR_OUTFILE;
		return -1;
	}

// printf("steps=%d\tN=%d\n", mfem_steps, mfem_N);

	for(kkk=0; kkk <= mfem_steps; kkk++)
	{
		if(fprintf(fout,"## s=%g w=%g D=%g ", mfem_s, mfem_rpm, mfem_D) < 0 || 
		fprintf(fout,"## N=%d dt=%g  \n", mfem_N, mfem_dt) < 0 || 
		fprintf(fout,"## kkk=%d t=%g  \n", kkk, mfem_t[kkk]) < 0 || 
		fprintf(fout, "## kkk=%d\tMass=%20.16e\n", kkk, mfem_Mass[kkk]) < 0)
		{
			sprintf(LastErrorMessage, fwerror);
			LastErrorNumber = ERR_OUTFILE;
			return -1;
		}
		for(j = 0; j < mfem_N;j++)
		{
			if(fprintf(fout,"%20.15e %8.5f %20.15e\n", mfem_x[(kkk * mfem_N) + j], 
			mfem_t[kkk], mfem_c[(kkk * mfem_N) + j]) < 0)
			{
				sprintf(LastErrorMessage, fwerror);
				LastErrorNumber = ERR_OUTFILE;
				return -1;
			}
		}
		if(fprintf(fout,"\n\n") < 0)
		{
			sprintf(LastErrorMessage, fwerror);
			LastErrorNumber = ERR_OUTFILE;
			return -1;
		}
	}
	if(fclose(fout))
	{
		sprintf(LastErrorMessage, "File close error");
		LastErrorNumber = ERR_OUTFILE;
		return -1;
	}
	return 0;
}

void US_MovingFEM::fprinterror(FILE *out, char *tag)
{
	fprintf(out, "%s %s [%d]\n", 
		tag, LastErrorMessage, LastErrorNumber);
}

void US_MovingFEM::fprintparams(FILE *f)
{
  fprintf(f, 
		"mfem_N %d\n"
		"mfem_steps %d\n"
		"mfem_s %g\n"
		"mfem_D %g\n"
		"mfem_rpm %g\n"
		"mfem_dt %g\n"
		"mfem_c0 %g\n"
		,
		mfem_N,
		mfem_steps,
		mfem_s,
		mfem_D,
		mfem_rpm,
		mfem_dt,
		c0
		);
}

void US_MovingFEM::fprintparams(FILE *f, int n)
{
  fprintf(f,
		"%d: mfem_N %d\n"
		"%d: mfem_steps %d\n"
		"%d: mfem_s %g\n"
		"%d: mfem_D %g\n"
		"%d: mfem_rpm %g\n"
		"%d: mfem_dt %g\n"
		"%d: mfem_c0 %g\n"
		,
		n, mfem_N,
		n, mfem_steps,
		n, mfem_s,
		n, mfem_D,
		n, mfem_rpm,
		n, mfem_dt,
		n, c0
		);
}

void US_MovingFEM::fprintinitparams(FILE *f, int n)
{
  fprintf(f,
		"%d: N %d\n"
		"%d: s %g\n"
		"%d: D %g\n"
		"%d: rpm %g\n"
		"%d: total_t %g\n"
		,
		n, N,
		n, s,
		n, D,
		n, rpm,
		n, total_t
		);
}

int US_MovingFEM::interpolate(struct mfem_data *simdata, unsigned int scans, unsigned int points, 
float *scantimes, double *radius, double **c)
{
	
/********************************************************************************************
 * interpolation:                                                                           *
 *                                                                                          *
 * First, we need to interpolate the time. Create a new array with the same time dimensions *
 * as the raw data and the same radius dimensions as the simulated data. Then find the time *
 * steps from the simulated data that bracket the experimental data from the left and right.*
 * Then make a linear interpolation for the concentration values at each radius point from  *
 * the simulated data. Then interpolate the radius points by linear interpolation.          *
 *                                                                                          *
 ********************************************************************************************/
  if ((*simdata).scan.size() == 0) {
    return -1;
  }
	unsigned int i, j;
	double slope, intercept;
	double **ip_array;
	ip_array	= 	new double* [scans];
	for (i=0; i<scans; i++)
	{
		ip_array[i] = new double [(*simdata).radius.size()];
	}
	unsigned int count = 0; // counting the number of time steps of the raw data
	for (i=0; i<scans; i++)
	{
		while (count < (*simdata).scan.size()-1
				&& scantimes[i] >= (*simdata).scan[count].time)
		{
			count++;
		}
		if (scantimes[i] == (*simdata).scan[count].time ||
		    count == 0)
		{
			for (j=0; j<(*simdata).radius.size(); j++)
			{
				ip_array[i][j] = (*simdata).scan[count].conc[j];
			}
		}
		else  // else, perform a linear time interpolation:
		{
			for (j=0; j<(*simdata).radius.size(); j++)
			{
				slope = ((*simdata).scan[count].conc[j] - (*simdata).scan[count-1].conc[j])
						  /((*simdata).scan[count].time - (*simdata).scan[count-1].time);
				intercept = (*simdata).scan[count].conc[j] - slope * (*simdata).scan[count].time;
				ip_array[i][j] = slope * scantimes[i] + intercept;
			}
		}
	}
	//interpolate radius then add to concentration vector
	for (i=0; i<scans; i++)
	{
		c[i][0] += ip_array[i][0]; // meniscus position is identical for all scans
	}
	// all other points may need interpolation:
	for (i=0; i<scans; i++)
	{
		count = 1;
		for (j=1; j<(unsigned int) points; j++)
		{
			while (radius[j] > (*simdata).radius[count] && count < (*simdata).radius.size()-1)
			{
				count++;
			}
			if (radius[j] == (*simdata).radius[count] || count == 0)
			{
				c[i][j] += ip_array[i][count];
			}
			else 
			{
				slope = (ip_array[i][count] - ip_array[i][count-1])/((*simdata).radius[count] - (*simdata).radius[count-1]);
				intercept = ip_array[i][count] - (*simdata).radius[count] * slope;
				c[i][j] += slope * radius[j] + intercept;
			}
		}
	}
	for (i=0; i<scans; i++)
	{
		delete [] ip_array[i];
	}
	delete [] ip_array;
	return 0;
}

int US_MovingFEM::interpolate(struct mfem_data *expdata, struct mfem_data *simdata)
{
	
/********************************************************************************************
 * interpolation:                                                                           *
 *                                                                                          *
 * First, we need to interpolate the time. Create a new array with the same time dimensions *
 * as the raw data and the same radius dimensions as the simulated data. Then find the time *
 * steps from the simulated data that bracket the experimental data from the left and right.*
 * Then make a linear interpolation for the concentration values at each radius point from  *
 * the simulated data. Then interpolate the radius points by linear interpolation.          *
 *                                                                                          *
 ********************************************************************************************/

// NOTE: *expdata has to be initialized to have the proper size (filled with zeros)
// before using this routine! The radius also has to be assigned!

	if ((*expdata).scan.size() == 0 || (*expdata).scan[0].conc.size() == 0 ||
	    (*simdata).scan.size() == 0)
	{
		return -1;
	}
	unsigned int i, j;
	double slope, intercept;
	double **ip_array;
	ip_array	= 	new double* [(*expdata).scan.size()];
	for (i=0; i<(*expdata).scan.size(); i++)
	{
		ip_array[i] = new double [(*simdata).radius.size()];
	}
	unsigned int count = 0; // counting the number of time steps of the raw data
	for (i=0; i<(*expdata).scan.size(); i++)
	{
		while (count < (*simdata).scan.size()-1
				&& (*expdata).scan[i].time >= (*simdata).scan[count].time)
		{
			count++;
		}
		if ((*expdata).scan[i].time == (*simdata).scan[count].time ||
		    (*simdata).scan.size() == 1 || count == 0)
		{
			for (j=0; j<(*simdata).radius.size(); j++)
			{
				ip_array[i][j] = (*simdata).scan[count].conc[j];
			}
		}
		else  // else, perform a linear time interpolation:
		{
			for (j=0; j<(*simdata).radius.size(); j++)
			{
				slope = ((*simdata).scan[count].conc[j] - (*simdata).scan[count-1].conc[j])
						  /((*simdata).scan[count].time - (*simdata).scan[count-1].time);
				intercept = (*simdata).scan[count].conc[j] - slope * (*simdata).scan[count].time;
				ip_array[i][j] = slope * (*expdata).scan[i].time + intercept;
			}
		}
	}
	// all points may need interpolation:
	for (i=0; i<(*expdata).scan.size(); i++)
	{
		count = 0;
		for (j=0; j<(unsigned int) (*expdata).radius.size(); j++)
		{
			while ((*expdata).radius[j] > (*simdata).radius[count] && count < (*simdata).radius.size()-1)
			{
				count++;
			}
			if ((*expdata).radius[j] == (*simdata).radius[count] || count == 0)
			{
				(*expdata).scan[i].conc[j] += ip_array[i][count];
			}
			else 
			{
				slope = (ip_array[i][count] - ip_array[i][count-1])/((*simdata).radius[count] - (*simdata).radius[count-1]);
				intercept = ip_array[i][count] - (*simdata).radius[count] * slope;
				(*expdata).scan[i].conc[j] += slope * (*expdata).radius[j] + intercept;
			}
		}
	}
	for (i=0; i<(*expdata).scan.size(); i++)
	{
		delete [] ip_array[i];
	}
	delete [] ip_array;
	return 0;
}

