#ifndef US_STIFFBASE_H
#define US_STIFFBASE_H

#include <math.h>
#include <stdio.h>

struct Gauss2D
{
	double x, y, w;
};

//
// StiffBase()
//
class StiffBase
{
	public:
		// parameters for triangular elements
		int n_gaussT;	// number of Gauss pts for numerical quadrature
		int n_basisT;  	// number of basis functions, =4 for linear quad elem
		Gauss2D *xgT;	// list of all Gauss pts and weights
		double **phiT;	// all basis functions at Gauss pts
		double **phiT1;	// xi -derivative of all basis functions at Gauss pts
		double **phiT2;	// eta-derivative of all basis functions at Gauss pts

		// parameters for quadrilateral elements
		int n_gaussQ;	// number of Gauss pts for numerical quadrature
		int n_basisQ;  	// number of basis functions, =4 for linear quad elem
		Gauss2D *xgQ;	// list of all Gauss pts and weights
		double **phiQ;	// all basis functions at Gauss pts
		double **phiQ1;	// xi -derivative of all basis functions at Gauss pts
		double **phiQ2;	// eta-derivative of all basis functions at Gauss pts


		StiffBase();
		~StiffBase();

		void LambdaG(unsigned int kk, double lam1, double lam2, double w, Gauss2D *Lm);
		void SetGauss();

		// map a Gauss point (gauss_ind) in standard elem to real elem (*x)
		void AffineMapping(int NK, double xd[4][2], int gauss_ind, double x[2]);

		// Jacobian d_xi/d_x of the mapping at the Gauss pts (gauss_ind)
		void Jacobian(int NK, double xd[4][2], int gauss_ind, double jcbv[5]);

		// compute all basis functions and their derivatives at Gauss pts
		void LinearBasis();
		
		void CompLocalStif(int NK, double xd[4][2], double D, double sw2, double **Stif);

}; // end StiffBase()


#endif

