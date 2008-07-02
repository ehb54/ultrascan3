#include "../include/us_astfem_math.h"


double minval(vector <double> val)
{
	double minimum=1.0e300;
	for (unsigned int i=0; i<val.size(); i++)
	{
		minimum = min(minimum, val[i]);
	}
	return minimum;
}

double minval(vector <SimulationComponent> val)
{
	double minimum=1.0e300;
	for (unsigned int i=0; i<val.size(); i++)
	{
		minimum = min(minimum, (double) val[i].s);
	}
	return minimum;
}

double maxval(vector <double> val)
{
	double maximum = -1.0e300;
	for (unsigned int i=0; i<val.size(); i++)
	{
		maximum = max(maximum, val[i]);
	}
	return maximum;
}

double maxval(vector <SimulationComponent> val)
{
	double maximum = -1.0e300;
	for (unsigned int i=0; i<val.size(); i++)
	{
		maximum = max(maximum, (double) val[i].s);
	}
	return maximum;
}

void tridiag(double *a, double *b, double *c, double *r, double *u, unsigned int N)
{
	int j;
	double bet, *gam;
	gam = new double [N];
	if (b[0] == 0.0) cerr << "Error 1 in tridag" << endl;

	u[0] = r[0]/(bet = b[0]);
	for (j=1; j<(int) N; j++)
	{
		gam[j] = c[j-1]/bet;
		bet = b[j] - a[j] * gam[j];
		if (bet == 0.0)
		{
			cerr << "Error 2 in tridag" << endl;
		}
		u[j] = (r[j] - a[j] * u[j-1])/bet;
	}
	for (j=(N-2); j>=0; j--)
	{
		u[j] -= gam[j+1] * u[j+1];
	}
	delete [] gam;
}

void initialize_3d(unsigned int val1, unsigned int val2, unsigned int val3, double ****matrix)
{
	unsigned int i, j, k;
	*matrix = new double **[val1];
	for (i=0; i<val1; i++)
	{
		(*matrix)[i] = new double *[val2];
		for (j=0; j<val2; j++)
		{
			(*matrix)[i][j] = new double [val3];
			for (k=0; k<val3; k++)
			{
				(*matrix)[i][j][k] = 0.0;
			}
		}
	}
}

void initialize_2d(unsigned int val1, unsigned int val2, double ***matrix)
{
	unsigned int i, j;
	*matrix = new double *[val1];
	for (i=0; i<val1; i++)
	{
		(*matrix)[i] = new double [val2];
		for (j=0; j<val2; j++)
		{
			(*matrix)[i][j] = 0.0;
		}
	}
}

void clear_3d(unsigned int val1, unsigned int val2, double ***matrix)
{
	unsigned int i, j;
	for (i=0; i<val1; i++)
	{
		for (j=0; j<val2; j++)
		{
			delete [] matrix[i][j];
		}
		delete [] matrix[i];
	}
	delete [] matrix;
}

void clear_2d(unsigned int val1, double **matrix)
{
	unsigned int i;
	for (i=0; i<val1; i++)
	{
		delete [] matrix[i];
	}
	delete [] matrix;
}


//
// source: http://www.math.ntnu.no/num/nnm/Program/Numlibc/
//
void DefineGaussian(unsigned int nGauss, double **Gs2)
{
	unsigned int i, j, k;
	double *Gs1, *w;
	Gs1 = new double [nGauss];
	w = new double [nGauss];

	switch (nGauss)
	{
		case 3:
		{
			Gs1[0] = -0.774596669241483; 	w[0] = 5.0/9.0;
			Gs1[1] = 0.0; 						w[1] = 8.0/9.0;
			Gs1[2] = 0.774596669241483; 	w[2] = 5.0/9.0;
			break;
		}
		case 5:
		{
         Gs1[0] = 0.906179845938664 ; 	w[0] = 0.236926885056189;
         Gs1[1] = 0.538469310105683 ; 	w[1] = 0.478628670499366;
         Gs1[2] = 0.000000000000000 ; 	w[2] = 0.568888888888889;
			Gs1[3] = -Gs1[1]; 				w[3] = w[1];
			Gs1[4] = -Gs1[0]; 				w[4] = w[0];
			break;
		}
		case 7:
		{
         Gs1[0] = 0.949107912342759 ;	w[0] = 0.129484966168870;
         Gs1[1] = 0.741531185599394 ;	w[1] = 0.279705391489277;
         Gs1[2] = 0.405845151377397 ;	w[2] = 0.381830050505119;
         Gs1[3] = 0.000000000000000 ;	w[3] = 0.417959183673469;
			Gs1[4] = -Gs1[2]; 				w[4] = w[2];
			Gs1[5] = -Gs1[1]; 				w[5] = w[1];
			Gs1[6] = -Gs1[0]; 				w[6] = w[0];
			break;
		}
		case 10:
		{
   		Gs1[0] = 0.973906528517172 ;	w[0] = 0.066671344308688;
      	Gs1[1] = 0.865063366688985 ;	w[1] = 0.149451349150581;
      	Gs1[2] = 0.679409568299024 ;	w[2] = 0.219086362515982;
      	Gs1[3] = 0.433395394129247 ;	w[3] = 0.269266719309996;
      	Gs1[4] = 0.148874338981631 ;	w[4] = 0.295524224714753;
			Gs1[5] = -Gs1[4]; 				w[5] = w[4];
			Gs1[6] = -Gs1[3]; 				w[6] = w[3];
			Gs1[7] = -Gs1[2]; 				w[7] = w[2];
			Gs1[8] = -Gs1[1]; 				w[8] = w[1];
			Gs1[9] = -Gs1[0]; 				w[9] = w[0];
			break;
      }
		default:
		{
			return;
		}
	}

	for (i=0; i<nGauss; i++)
	{
		for (j=0; j<nGauss; j++) 	// map to [0,1] x [0,1]
		{
			k = j + (i) * nGauss;
			Gs2[k][0] = (Gs1[i] + 1.0)/2.0;
			Gs2[k][1] = (Gs1[j] + 1.0)/2.0;
			Gs2[k][2] = w[i] * w[j]/4.0;
		}
	}
	delete [] w;
	delete [] Gs1;
}


//
// integrand for Lamm equation
//
double Integrand(double x_gauss, double D, double sw2,
double u, double ux, double ut, double v, double vx)
{
	return (x_gauss * ut * v + D * x_gauss * ux * vx -
	sw2 * pow(x_gauss, 2.0) * u * vx);
}


//
// old version: perform integration on supp(test function) separately on left Q and right T
//
void IntQT1(vector <double> vx, double D, double sw2, double **Stif, double dt)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
   double hh, slope, xn1, phiC, phiCx;
	vector <double> Lx, Ly, Rx, Ry, Qx, Qy, Tx, Ty;
	double *phiL, *phiLx, *phiLy, *phiR, *phiRx, *phiRy;
	double **StifL=NULL, **StifR=NULL, **Lam=NULL, DJac;
	Lx.clear();
	Ly.clear();
	Rx.clear();
	Ry.clear();
	Qx.clear();
	Qy.clear();
	Tx.clear();
	Ty.clear();
   phiL  = new double [3];
   phiLx = new double [3];
   phiLy = new double [3];
   phiR  = new double [4];
   phiRx = new double [4];
   phiRy = new double [4];

	// elements for define the trial function phi
	Lx.push_back(vx[0]); 	// vertices of left Triangle
	Lx.push_back(vx[3]);
	Lx.push_back(vx[2]);

	Ly.push_back(0.0);
	Ly.push_back(dt);
	Ly.push_back(dt);

	Rx.push_back(vx[0]);	// vertices of Q on right quadrilateral
	Rx.push_back(vx[1]);
	Rx.push_back(vx[4]);
	Rx.push_back(vx[3]);

	Ry.push_back(0.0);
	Ry.push_back(0.0);
	Ry.push_back(dt);
	Ry.push_back(dt);

	initialize_2d(3, 2, &StifL);
	initialize_2d(4, 2, &StifR);
   hh = vx[3] - vx[2];
   slope = (vx[3] - vx[5])/dt;
	npts = 28;
	initialize_2d(npts, 4, &Lam);
	DefineFkp(npts, Lam);

   //
   // integration over element Q (a triangle):
   //
	Qx.push_back(vx[0]);	// vertices of Q on left
	Qx.push_back(vx[3]);
	Qx.push_back(vx[2]);

	Qy.push_back(0.0);
	Qy.push_back(dt);
	Qy.push_back(dt);	// vertices of left T

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Qx[0] + Lam[k][1] * Qx[1] + Lam[k][2] * Qx[2];
		y_gauss = Lam[k][0] * Qy[0] + Lam[k][1] * Qy[1] + Lam[k][2] * Qy[2];
		DJac = 2.0 * AreaT(&Qx, &Qy);

      xn1 = x_gauss + slope * ( dt - y_gauss );	// trace-forward point at t_n+1 from (x_g, y_g)

		//
		// find phi, phi_x, phi_y on L and C at (x,y)
		//

		BasisTR(Lx, Ly, x_gauss, y_gauss, phiL, phiLx, phiLy);
      phiC  = ( xn1 - vx[2] )/hh;		// hat function on t_n+1, =1 at vx[3]; =0 at vx[2]
      phiCx = 1./hh;

		for (i=0; i<3; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i], 1.-phiC, -phiCx );
			StifL[i][0] += Lam[k][3] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i], phiC, phiCx );
			StifL[i][1] += Lam[k][3] * DJac * dval;
		}
	}

	//
	// integration over T:
	//
	Tx.push_back(vx[0]);	// vertices of T on right
	Tx.push_back(vx[5]);
	Tx.push_back(vx[3]);

	Ty.push_back(0.0);
	Ty.push_back(0.0);
	Ty.push_back(dt);

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
		y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
		DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;

      xn1 = x_gauss + slope * ( dt - y_gauss );	// trace-forward point at t_n+1 from (x_g, y_g)

		//
		// find phi, phi_x, phi_y on R and C at (x,y)
		//

		BasisQR(Rx, x_gauss, y_gauss, phiR, phiRx, phiRy, dt);
      phiC  = ( xn1 - vx[2] )/hh;		// hat function on t_n+1, =1 at vx[3]; =0 at vx[2]
      phiCx = 1./hh;

		for (i=0; i<4; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i], 1.-phiC, -phiCx);
			StifR[i][0] += Lam[k][3] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i], phiC, phiCx);
			StifR[i][1] += Lam[k][3] * DJac * dval;
		}
	}

	clear_2d(npts, Lam);

	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifL[0][i] + StifR[0][i];
		Stif[1][i] =               StifR[1][i];
		Stif[2][i] = StifL[2][i];
		Stif[3][i] = StifL[1][i] + StifR[3][i];
		Stif[4][i] =               StifR[2][i];
	}

	delete [] phiR;
	delete [] phiRx;
	delete [] phiRy;
	delete [] phiL;
	delete [] phiLx;
	delete [] phiLy;

	clear_2d(3, StifL);
	clear_2d(4, StifR);


}

void IntQTm(vector <double> vx, double D, double sw2, double **Stif, double dt)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
	vector <double> Lx, Ly, Cx, Cy, Rx, Ry, Qx, Qy, Tx, Ty;
	double *phiR, *phiRx, *phiRy;
	double **StifL=NULL, **StifR=NULL, **Lam=NULL, DJac;
	double *phiL, *phiLx, *phiLy, *phiCx, *phiCy, *phiC;
	double **Gs=NULL;
	phiL = new double [4];
	phiLx = new double [4];
	phiLy = new double [4];
	phiCx = new double [4];
	phiCy = new double [4];
	phiC = new double [4];
	phiR = new double [4];
	phiRx = new double [4];
	phiRy = new double [4];
	Lx.clear();
	Ly.clear();
	Cx.clear();
	Cy.clear();
	Rx.clear();
	Ry.clear();
	Qx.clear();
	Qy.clear();
	Tx.clear();
	Ty.clear();

	Lx.push_back(vx[0]);
	Lx.push_back(vx[1]);
	Lx.push_back(vx[4]);
	Lx.push_back(vx[3]);

	Ly.push_back(0.0);
	Ly.push_back(0.0);
	Ly.push_back(dt);
	Ly.push_back(dt); 			// vertices of left T

	Cx.push_back(vx[6]);
	Cx.push_back(vx[7]);
	Cx.push_back(vx[4]);
	Cx.push_back(vx[3]);

	Cy.push_back(0.0);
	Cy.push_back(0.0);
	Cy.push_back(dt);
	Cy.push_back(dt);

	Rx.push_back(vx[1]);	// vertices of Q on right
	Rx.push_back(vx[2]);
	Rx.push_back(vx[5]);
	Rx.push_back(vx[4]);

	Ry.push_back(0.0);
	Ry.push_back(0.0);
	Ry.push_back(dt);
	Ry.push_back(dt);

	initialize_2d(4, 2, &StifL);
	initialize_2d(4, 2, &StifR);

   //
   // integration over element Q :
   //
	Qx.push_back(vx[6]);	// vertices of Q on right
	Qx.push_back(vx[1]);
	Qx.push_back(vx[4]);
	Qx.push_back(vx[3]);

	Qy.push_back(0.0);
	Qy.push_back(0.0);
	Qy.push_back(dt);
	Qy.push_back(dt);	// vertices of left T

	npts = 5 * 5;
	initialize_2d(npts, 3, &Gs);
	DefineGaussian(5, Gs);

	double psi[4], psi1[4], psi2[4], jac[4];
	for (k=0; k<npts; k++)
	{
		BasisQS(Gs[k][0], Gs[k][1], psi, psi1, psi2);

		x_gauss = 0.0;
		y_gauss = 0.0;
		for (i=0; i<4; i++)
		{
			jac[i] = 0.0;
		}
		for (i=0; i<4; i++)
		{
			x_gauss += psi[i] * Qx[i];
			y_gauss += psi[i] * Qy[i];
			jac[0] += Qx[i] * psi1[i];
			jac[1] += Qx[i] * psi2[i];
			jac[2] += Qy[i] * psi1[i];
			jac[3] += Qy[i] * psi2[i];
		}

		DJac = jac[0] * jac[3] - jac[1] * jac[2];

		//
		// find phi, phi_x, phi_y on L and C at (x,y)
		//

		BasisQR(Lx, x_gauss, y_gauss, phiL, phiLx, phiLy, dt);
		BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt);
		for (i=0; i<4; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
 											phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			StifL[i][0] += Gs[k][2] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
											phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			StifL[i][1] += Gs[k][2] * DJac * dval;
		}
	}
	clear_2d(npts, Gs);

	//
	// integration over T:
	//
	Tx.push_back(vx[1]);	// vertices of T on right
	Tx.push_back(vx[7]);
	Tx.push_back(vx[4]);

	Ty.push_back(0.0);
	Ty.push_back(0.0);
	Ty.push_back(dt);

	npts = 28;
	initialize_2d(npts, 4, &Lam);
	DefineFkp(npts, Lam);

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
		y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
		DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;

		//
		// find phi, phi_x, phi_y on R and C at (x,y)
		//

		BasisQR(Rx, x_gauss, y_gauss, phiR, phiRx, phiRy, dt);
		BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt);

		for (i=0; i<4; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
			       						phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			StifR[i][0] += Lam[k][3] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
					 						phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			StifR[i][1] += Lam[k][3] * DJac * dval;
		}
	}
	clear_2d(npts, Lam);


	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifL[0][i];
		Stif[1][i] = StifL[1][i] + StifR[0][i];
		Stif[2][i] = 					StifR[1][i];
		Stif[3][i] = StifL[3][i];
		Stif[4][i] = StifL[2][i] + StifR[3][i];
		Stif[5][i] = 					StifR[2][i];
	}
	delete [] phiR;
	delete [] phiRx;
	delete [] phiRy;
	delete [] phiL;
	delete [] phiLx;
	delete [] phiLy;
	delete [] phiCx;
	delete [] phiCy;
	delete [] phiC;

	clear_2d(3, StifL);
	clear_2d(4, StifR);
}


void IntQTn2(vector <double> vx, double D, double sw2, double **Stif, double dt)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
	vector <double> Lx, Ly, Cx, Cy, Rx, Ry, Qx, Qy, Tx, Ty;
	double *phiR, *phiRx, *phiRy;
	double **StifL=NULL, **StifR=NULL, **Lam=NULL, DJac;
	double *phiL, *phiLx, *phiLy, *phiCx, *phiCy, *phiC;
	double **Gs=NULL;
	phiL = new double [4];
	phiLx = new double [4];
	phiLy = new double [4];
	phiCx = new double [4];
	phiCy = new double [4];
	phiC = new double [4];
	phiR = new double [3];
	phiRx = new double [3];
	phiRy = new double [3];
	Lx.clear();
	Ly.clear();
	Cx.clear();
	Cy.clear();
	Rx.clear();
	Ry.clear();
	Qx.clear();
	Qy.clear();
	Tx.clear();
	Ty.clear();

	Lx.push_back(vx[0]);
	Lx.push_back(vx[1]);
	Lx.push_back(vx[4]);
	Lx.push_back(vx[3]);

	Ly.push_back(0.0);
	Ly.push_back(0.0);
	Ly.push_back(dt);
	Ly.push_back(dt); 			// vertices of left T

	Cx.push_back(vx[5]);
	Cx.push_back(vx[6]);
	Cx.push_back(vx[4]);
	Cx.push_back(vx[3]);

	Cy.push_back(0.0);
	Cy.push_back(0.0);
	Cy.push_back(dt);
	Cy.push_back(dt);

	Rx.push_back(vx[1]);	// vertices of Q on right
	Rx.push_back(vx[2]);
	Rx.push_back(vx[4]);

	Ry.push_back(0.0);
	Ry.push_back(0.0);
	Ry.push_back(dt);

	initialize_2d(4, 2, &StifL);
	initialize_2d(4, 2, &StifR);

   //
   // integration over element Q
   //
	Qx.push_back(vx[5]);	// vertices of Q on right
	Qx.push_back(vx[1]);
	Qx.push_back(vx[4]);
	Qx.push_back(vx[3]);

	Qy.push_back(0.0);
	Qy.push_back(0.0);
	Qy.push_back(dt);
	Qy.push_back(dt);

	npts = 5 * 5;
	initialize_2d(npts, 3, &Gs);
	DefineGaussian(5, Gs);

	double psi[4], psi1[4], psi2[4], jac[4];
	for (k=0; k<npts; k++)
	{
		BasisQS(Gs[k][0], Gs[k][1], psi, psi1, psi2);

		x_gauss = 0.0;
		y_gauss = 0.0;
		for (i=0; i<4; i++)
		{
			jac[i] = 0.0;
		}
		for (i=0; i<4; i++)
		{
			x_gauss += psi[i] * Qx[i];
			y_gauss += psi[i] * Qy[i];
			jac[0] += Qx[i] * psi1[i];
			jac[1] += Qx[i] * psi2[i];
			jac[2] += Qy[i] * psi1[i];
			jac[3] += Qy[i] * psi2[i];
		}

		DJac = jac[0] * jac[3] - jac[1] * jac[2];

		//
		// find phi, phi_x, phi_y on L and C at (x,y)
		//

		BasisQR(Lx, x_gauss, y_gauss, phiL, phiLx, phiLy, dt);
		BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt);
		for (i=0; i<4; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
											phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			StifL[i][0] += Gs[k][2] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
											phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			StifL[i][1] += Gs[k][2] * DJac * dval;
		}
	}
	clear_2d(npts, Gs);

	//
	// integration over T:
	//
	Tx.push_back(vx[1]);	// vertices of T on right
	Tx.push_back(vx[6]);
	Tx.push_back(vx[4]);

	Ty.push_back(0.0);
	Ty.push_back(0.0);
	Ty.push_back(dt);

	npts = 28;
	initialize_2d(npts, 4, &Lam);
	DefineFkp(npts, Lam);

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
		y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
		DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;

		//
		// find phi, phi_x, phi_y on R and C at (x,y)
		//

		BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt);
		BasisTR(Rx, Ry, x_gauss, y_gauss, phiR, phiRx, phiRy);

		for (i=0; i<3; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
											phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			StifR[i][0] += Lam[k][3] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
											phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			StifR[i][1] += Lam[k][3] * DJac * dval;
		}
	}
	clear_2d(npts, Lam);

	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifL[0][i];
		Stif[1][i] = StifL[1][i] + StifR[0][i] ;
		Stif[2][i] = 					StifR[1][i];
		Stif[3][i] = StifL[3][i];
		Stif[4][i] = StifL[2][i] + StifR[2][i];
	}
	delete [] phiR;
	delete [] phiRx;
	delete [] phiRy;
	delete [] phiL;
	delete [] phiLx;
	delete [] phiLy;
	delete [] phiCx;
	delete [] phiCy;
	delete [] phiC;

	clear_2d(3, StifL);
	clear_2d(4, StifR);
}

void IntQTn1(vector <double> vx, double D, double sw2, double **Stif, double dt)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
	vector <double> Lx, Ly, Tx, Ty;
	double **StifR=NULL, **Lam=NULL, DJac;
	double *phiL, *phiLx, *phiLy;
	phiL = new double [4];
	phiLx = new double [4];
	phiLy = new double [4];
	Lx.clear();
	Ly.clear();
	Tx.clear();
	Ty.clear();

	Lx.push_back(vx[0]);
	Lx.push_back(vx[1]);
	Lx.push_back(vx[2]);

	Ly.push_back(0.0);
	Ly.push_back(0.0);
	Ly.push_back(dt);

	initialize_2d(4, 2, &StifR);

	//
	// integration over T:
	//
	Tx.push_back(vx[3]);	// vertices of T on right
	Tx.push_back(vx[1]);
	Tx.push_back(vx[2]);

	Ty.push_back(0.0);
	Ty.push_back(0.0);
	Ty.push_back(dt);

	npts = 28;
	initialize_2d(npts, 4, &Lam);
	DefineFkp(npts, Lam);

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
		y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
		DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;
		//
		// find phi, phi_x, phi_y on R and C at (x,y)
		//

		BasisTR(Lx, Ly, x_gauss, y_gauss, phiL, phiLx, phiLy);

		for (i=0; i<3; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i], 1.0, 0.0);
			StifR[i][0] += Lam[k][3] * DJac * dval;
		}
	}
	clear_2d(npts, Lam);

	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifR[0][i];
		Stif[1][i] = StifR[1][i];
		Stif[2][i] = StifR[2][i];
	}
	delete [] phiL;
	delete [] phiLx;
	delete [] phiLy;

	clear_2d(4, StifR);
}


void QuadSolver(double *ai, double *bi, double *ci, double *di, double *cr, double *solu, unsigned int N)
{
//
// solve Quad-diagonal system [a_i, *b_i*, c_i, d_i]*[x]=[r_i]
// b_i ar e on the main diagonal line
//
// test
//	n=100; a=-1+rand(100,1); b=2+rand(200,1); c=-0.7*rand(100,1); d=-0.3*rand(100,1);
//	xs=rand(100,1);
//	r(1)=b(1)*xs(1)+c(1)*xs(2)+d(1)*xs(3);
//	for i=2:n-2,
//	r(i)=a(i)*xs(i-1)+b(i)*xs(i)+c(i)*xs(i+1)+d(i)*xs(i+2);
//	end;
//	i=n-1; r(i)=a(i)*xs(i-1)+b(i)*xs(i)+c(i)*xs(i+1);
//	i=n;	r(i)=a(i)*xs(i-1)+b(i)*xs(i);
//

	unsigned int i;
	double tmp;
	vector<double> ca, cb, cc, cd;
	ca.clear();
	cb.clear();
	cc.clear();
	cd.clear();
	for (i=0; i<N; i++)
	{
		ca.push_back( ai[i] );
		cb.push_back( bi[i] );
		cc.push_back( ci[i] );
		cd.push_back( di[i] );
	}

	for (i=1; i<=N-2; i++)
	{
		tmp = ca[i]/cb[i-1];
		cb[i] = cb[i]-cc[i-1]*tmp;
		cc[i] = cc[i]-cd[i-1]*tmp;
		cr[i] = cr[i]-cr[i-1]*tmp;
	}
   i=N-1;
		tmp = ca[i]/cb[i-1];
		cb[i] = cb[i]-cc[i-1]*tmp;
		cr[i] = cr[i]-cr[i-1]*tmp;


	solu[N-1] = cr[N-1] / cb[N-1];
	solu[N-2] = (cr[N-2] - cc[N-2] * solu[N-1]) / cb[N-2];
	i = N - 2;
	do
	{
		i--;
		solu[i] = (cr[i] - cc[i] * solu[i+1] - cd[i] * solu[i+2]) / cb[i];
	} while (i != 0);
}

void DefineFkp(unsigned int npts, double **Lam)
{
   //
   // source: http://people.scs.fsu.edu/~burkardt/datasets/quadrature_rules_tri/quadrature_rules_tri.html
   //
	switch (npts)
	{
		case 12:
      //  STRANG9, order 12, degree of precision 6.
		{
  				Lam[ 0][0] = 0.873821971016996;	Lam[ 0][1] = 0.063089014491502;
  				Lam[ 1][0] = 0.063089014491502;	Lam[ 1][1] = 0.873821971016996;
  				Lam[ 2][0] = 0.063089014491502;	Lam[ 2][1] = 0.063089014491502;
  				Lam[ 3][0] = 0.501426509658179;	Lam[ 3][1] = 0.249286745170910;
  				Lam[ 4][0] = 0.249286745170910;	Lam[ 4][1] = 0.501426509658179;
  				Lam[ 5][0] = 0.249286745170910;	Lam[ 5][1] = 0.249286745170910;
  				Lam[ 6][0] = 0.636502499121399;	Lam[ 6][1] = 0.310352451033785;
  				Lam[ 7][0] = 0.636502499121399;	Lam[ 7][1] = 0.053145049844816;
  				Lam[ 8][0] = 0.310352451033785;	Lam[ 8][1] = 0.636502499121399;
  				Lam[ 9][0] = 0.310352451033785;	Lam[ 9][1] = 0.053145049844816;
  				Lam[10][0] = 0.053145049844816;	Lam[10][1] = 0.636502499121399;
  				Lam[11][0] = 0.053145049844816;	Lam[11][1] = 0.310352451033785;

  				Lam[ 0][3] = 0.050844906370207;
  				Lam[ 1][3] = 0.050844906370207;
  				Lam[ 2][3] = 0.050844906370207;
  				Lam[ 3][3] = 0.116786275726379;
  				Lam[ 4][3] = 0.116786275726379;
  				Lam[ 5][3] = 0.116786275726379;
  				Lam[ 6][3] = 0.082851075618374;
  				Lam[ 7][3] = 0.082851075618374;
  				Lam[ 8][3] = 0.082851075618374;
  				Lam[ 9][3] = 0.082851075618374;
  				Lam[10][3] = 0.082851075618374;
  				Lam[11][3] = 0.082851075618374;
			break;
		}
		case 28:
      // TOMS612_28, order 28, degree of precision 11, a rule from ACM TOMS algorithm #612
		{
  			Lam[ 0][0] = 0.33333333333333333  ;	Lam[ 0][1] = 0.333333333333333333 ;
  			Lam[ 1][0] = 0.9480217181434233   ;	Lam[ 1][1] = 0.02598914092828833 ;
  			Lam[ 2][0] = 0.02598914092828833  ;	Lam[ 2][1] = 0.9480217181434233 ;
  			Lam[ 3][0] = 0.02598914092828833  ;	Lam[ 3][1] = 0.02598914092828833 ;
  			Lam[ 4][0] = 0.8114249947041546   ;	Lam[ 4][1] = 0.09428750264792270 ;
  			Lam[ 5][0] = 0.09428750264792270  ;	Lam[ 5][1] = 0.8114249947041546 ;
  			Lam[ 6][0] = 0.09428750264792270  ;	Lam[ 6][1] = 0.09428750264792270 ;
  			Lam[ 7][0] = 0.01072644996557060  ;	Lam[ 7][1] = 0.4946367750172147 ;
  			Lam[ 8][0] = 0.4946367750172147   ;	Lam[ 8][1] = 0.01072644996557060 ;
  			Lam[ 9][0] = 0.4946367750172147   ;	Lam[ 9][1] = 0.4946367750172147 ;
  			Lam[10][0] = 0.5853132347709715   ;	Lam[10][1] = 0.2073433826145142 ;
  			Lam[11][0] = 0.2073433826145142   ;	Lam[11][1] = 0.5853132347709715 ;
  			Lam[12][0] = 0.2073433826145142   ;	Lam[12][1] = 0.2073433826145142 ;
  			Lam[13][0] = 0.1221843885990187   ;	Lam[13][1] = 0.4389078057004907 ;
  			Lam[14][0] = 0.4389078057004907   ;	Lam[14][1] = 0.1221843885990187 ;
  			Lam[15][0] = 0.4389078057004907   ;	Lam[15][1] = 0.4389078057004907 ;
  			Lam[16][0] = 0.6779376548825902   ;	Lam[16][1] = 0.04484167758913055 ;
  			Lam[17][0] = 0.6779376548825902   ;	Lam[17][1] = 0.27722066752827925 ;
  			Lam[18][0] = 0.04484167758913055  ;	Lam[18][1] = 0.6779376548825902 ;
  			Lam[19][0] = 0.04484167758913055  ;	Lam[19][1] = 0.27722066752827925 ;
  			Lam[20][0] = 0.27722066752827925  ;	Lam[20][1] = 0.6779376548825902 ;
  			Lam[21][0] = 0.27722066752827925  ;	Lam[21][1] = 0.04484167758913055 ;
  			Lam[22][0] = 0.8588702812826364   ;	Lam[22][1] = 0.00000000000000000 ;
  			Lam[23][0] = 0.8588702812826364   ;	Lam[23][1] = 0.1411297187173636 ;
  			Lam[24][0] = 0.0000000000000000   ;	Lam[24][1] = 0.8588702812826364 ;
  			Lam[25][0] = 0.0000000000000000   ;	Lam[25][1] = 0.1411297187173636 ;
  			Lam[26][0] = 0.1411297187173636   ; Lam[26][1] = 0.8588702812826364 ;
  			Lam[27][0] = 0.1411297187173636   ;	Lam[27][1] = 0.0000000000000000 ;

  			Lam[ 0][3] = 0.08797730116222190 ;
  			Lam[ 1][3] = 0.008744311553736190 ;
  			Lam[ 2][3] = 0.008744311553736190 ;
  			Lam[ 3][3] = 0.008744311553736190 ;
  			Lam[ 4][3] = 0.03808157199393533 ;
  			Lam[ 5][3] = 0.03808157199393533 ;
  			Lam[ 6][3] = 0.03808157199393533 ;
  			Lam[ 7][3] = 0.01885544805613125 ;
  			Lam[ 8][3] = 0.01885544805613125 ;
  			Lam[ 9][3] = 0.01885544805613125 ;
  			Lam[10][3] = 0.07215969754474100 ;
  			Lam[11][3] = 0.07215969754474100 ;
  			Lam[12][3] = 0.07215969754474100 ;
  			Lam[13][3] = 0.06932913870553720 ;
  			Lam[14][3] = 0.06932913870553720 ;
  			Lam[15][3] = 0.06932913870553720 ;
  			Lam[16][3] = 0.04105631542928860 ;
  			Lam[17][3] = 0.04105631542928860 ;
  			Lam[18][3] = 0.04105631542928860 ;
  			Lam[19][3] = 0.04105631542928860 ;
  			Lam[20][3] = 0.04105631542928860 ;
  			Lam[21][3] = 0.04105631542928860 ;
  			Lam[22][3] = 0.007362383783300573 ;
  			Lam[23][3] = 0.007362383783300573 ;
  			Lam[24][3] = 0.007362383783300573 ;
  			Lam[25][3] = 0.007362383783300573 ;
  			Lam[26][3] = 0.007362383783300573 ;
  			Lam[27][3] = 0.007362383783300573 ;
			break;
		}
		case 37:
      //   TOMS706_37, order 37, degree of precision 13, a rule from ACM TOMS algorithm #706.
		{
  			Lam[ 0][0] = 0.333333333333333333333333333333; 	Lam[ 0][1] = 0.333333333333333333333333333333;
  			Lam[ 1][0] = 0.950275662924105565450352089520;  Lam[ 1][1] = 0.024862168537947217274823955239;
  			Lam[ 2][0] = 0.024862168537947217274823955239;  Lam[ 2][1] = 0.950275662924105565450352089520;
  			Lam[ 3][0] = 0.024862168537947217274823955239;  Lam[ 3][1] = 0.024862168537947217274823955239;
  			Lam[ 4][0] = 0.171614914923835347556304795551;  Lam[ 4][1] = 0.414192542538082326221847602214;
  			Lam[ 5][0] = 0.414192542538082326221847602214;  Lam[ 5][1] = 0.171614914923835347556304795551;
  			Lam[ 6][0] = 0.414192542538082326221847602214;  Lam[ 6][1] = 0.414192542538082326221847602214;
  			Lam[ 7][0] = 0.539412243677190440263092985511;  Lam[ 7][1] = 0.230293878161404779868453507244;
  			Lam[ 8][0] = 0.230293878161404779868453507244;  Lam[ 8][1] = 0.539412243677190440263092985511;
  			Lam[ 9][0] = 0.230293878161404779868453507244;  Lam[ 9][1] = 0.230293878161404779868453507244;
  			Lam[10][0] = 0.772160036676532561750285570113;  Lam[10][1] = 0.113919981661733719124857214943;
         Lam[11][0] = 0.113919981661733719124857214943;  Lam[11][1] = 0.772160036676532561750285570113;
         Lam[12][0] = 0.113919981661733719124857214943;  Lam[12][1] = 0.113919981661733719124857214943;
         Lam[13][0] = 0.009085399949835353883572964740;  Lam[13][1] = 0.495457300025082323058213517632;
         Lam[14][0] = 0.495457300025082323058213517632;  Lam[14][1] = 0.009085399949835353883572964740;
         Lam[15][0] = 0.495457300025082323058213517632;  Lam[15][1] = 0.495457300025082323058213517632;
         Lam[16][0] = 0.062277290305886993497083640527;  Lam[16][1] = 0.468861354847056503251458179727;
         Lam[17][0] = 0.468861354847056503251458179727;  Lam[17][1] = 0.062277290305886993497083640527;
         Lam[18][0] = 0.468861354847056503251458179727;  Lam[18][1] = 0.468861354847056503251458179727;
         Lam[19][0] = 0.022076289653624405142446876931;  Lam[19][1] = 0.851306504174348550389457672223;
         Lam[20][0] = 0.022076289653624405142446876931;  Lam[20][1] = 0.126617206172027096933163647918;
         Lam[21][0] = 0.851306504174348550389457672223;  Lam[21][1] = 0.022076289653624405142446876931;
         Lam[22][0] = 0.851306504174348550389457672223;  Lam[22][1] = 0.126617206172027096933163647918;
         Lam[23][0] = 0.126617206172027096933163647918;  Lam[23][1] = 0.022076289653624405142446876931;
         Lam[24][0] = 0.126617206172027096933163647918;  Lam[24][1] = 0.851306504174348550389457672223;
         Lam[25][0] = 0.018620522802520968955913511549;  Lam[25][1] = 0.689441970728591295496647976487;
         Lam[26][0] = 0.018620522802520968955913511549;  Lam[26][1] = 0.291937506468887771754472382212;
         Lam[27][0] = 0.689441970728591295496647976487;  Lam[27][1] = 0.018620522802520968955913511549;
         Lam[28][0] = 0.689441970728591295496647976487;  Lam[28][1] = 0.291937506468887771754472382212;
         Lam[29][0] = 0.291937506468887771754472382212;  Lam[29][1] = 0.018620522802520968955913511549;
         Lam[30][0] = 0.291937506468887771754472382212;  Lam[30][1] = 0.689441970728591295496647976487;
         Lam[31][0] = 0.096506481292159228736516560903;  Lam[31][1] = 0.635867859433872768286976979827;
         Lam[32][0] = 0.096506481292159228736516560903;  Lam[32][1] = 0.267625659273967961282458816185;
         Lam[33][0] = 0.635867859433872768286976979827;  Lam[33][1] = 0.096506481292159228736516560903;
         Lam[34][0] = 0.635867859433872768286976979827;  Lam[34][1] = 0.267625659273967961282458816185;
         Lam[35][0] = 0.267625659273967961282458816185;  Lam[35][1] = 0.096506481292159228736516560903;
         Lam[36][0] = 0.267625659273967961282458816185;  Lam[36][1] = 0.635867859433872768286976979827;


  			Lam[ 0][3] = 0.051739766065744133555179145422;
  			Lam[ 1][3] = 0.008007799555564801597804123460;
  			Lam[ 2][3] = 0.008007799555564801597804123460;
  			Lam[ 3][3] = 0.008007799555564801597804123460;
  			Lam[ 4][3] = 0.046868898981821644823226732071;
  			Lam[ 5][3] = 0.046868898981821644823226732071;
  			Lam[ 6][3] = 0.046868898981821644823226732071;
  			Lam[ 7][3] = 0.046590940183976487960361770070;
  			Lam[ 8][3] = 0.046590940183976487960361770070;
  			Lam[ 9][3] = 0.046590940183976487960361770070;
  			Lam[10][3] = 0.031016943313796381407646220131;
  			Lam[11][3] = 0.031016943313796381407646220131;
  			Lam[12][3] = 0.031016943313796381407646220131;
  			Lam[13][3] = 0.010791612736631273623178240136;
  			Lam[14][3] = 0.010791612736631273623178240136;
  			Lam[15][3] = 0.010791612736631273623178240136;
  			Lam[16][3] = 0.032195534242431618819414482205;
  			Lam[17][3] = 0.032195534242431618819414482205;
  			Lam[18][3] = 0.032195534242431618819414482205;
  			Lam[19][3] = 0.015445834210701583817692900053;
  			Lam[20][3] = 0.015445834210701583817692900053;
  			Lam[21][3] = 0.015445834210701583817692900053;
         Lam[22][3] = 0.015445834210701583817692900053;
         Lam[23][3] = 0.015445834210701583817692900053;
         Lam[24][3] = 0.015445834210701583817692900053;
         Lam[25][3] = 0.017822989923178661888748319485;
         Lam[26][3] = 0.017822989923178661888748319485;
         Lam[27][3] = 0.017822989923178661888748319485;
         Lam[28][3] = 0.017822989923178661888748319485;
         Lam[29][3] = 0.017822989923178661888748319485;
         Lam[30][3] = 0.017822989923178661888748319485;
         Lam[31][3] = 0.037038683681384627918546472190;
         Lam[32][3] = 0.037038683681384627918546472190;
         Lam[33][3] = 0.037038683681384627918546472190;
         Lam[34][3] = 0.037038683681384627918546472190;
         Lam[35][3] = 0.037038683681384627918546472190;
         Lam[36][3] = 0.037038683681384627918546472190;

			break;
      }
		default:
		{
			return;
		}
	}
   for(unsigned int i=0; i<npts; i++)
   {
      Lam[i][2] = 1. - Lam[i][0] - Lam[i][1];
      Lam[i][3] /= 2.; 			// to make the sum( wt ) = 0.5 = area of standard elem
   }
}

//
// AreaT: area of a triangle (v1, v2, v3)
//
double AreaT(vector <double> *xv, vector <double> *yv)
{
	return (0.5 * (((*xv)[1] - (*xv)[0]) * ((*yv)[2] - (*yv)[0])
					- ((*xv)[2] - (*xv)[0]) * ((*yv)[1] - (*yv)[0])));
}

//
// computer basis on standard element
//
void BasisTS(double xi, double et, double *phi, double *phi1, double *phi2)
{
//function [phi, phi1, phi2] = BasisTS(xi,et)

	phi[0] = 1.0 - xi - et;
	phi1[0] = -1.0;
	phi2[0]= -1.0;

	phi[1] = xi;
	phi1[1] = 1.0;
	phi2[1] = 0.0;

	phi[2] = et;
	phi1[2] = 0.0;
	phi2[2] = 1.0;
}

//
// computer basis on standard element
//
void BasisQS(double xi, double et, double *phi, double *phi1, double *phi2)
{

	phi[0] = (1.0 - xi) * (1.0 - et);
	phi1[0] = -1.0 * (1.0 - et);
	phi2[0]= -1.0 * (1.0 - xi);

	phi[1] = xi * (1.0 - et);
	phi1[1] = 1.0 - et;
	phi2[1] = -xi;

	phi[2] = xi * et;
	phi1[2] = et;
	phi2[2] = xi;

	phi[3] = (1.0 - xi) * et;
	phi1[3] = -et;
	phi2[3] = 1.0 - xi;
}

//
// function BasisTR: compute basis on real element T:
//	phi, phi_x, phi_t at a given (xs,ts) point
//	the triangular is assumed to be (x1,y1), (x2, y2), (x3, y3)
//
void BasisTR(vector <double> vx, vector <double> vy, double xs,
double ys, double *phi, double *phix, double *phiy)
{
	// find (xi,et) corresponding to (xs, ts)
	//
	unsigned int i;
	vector <double> tempv1, tempv2;
	tempv1.clear();
	tempv2.clear();
	tempv1.push_back(xs);
	tempv1.push_back(vx[2]);
	tempv1.push_back(vx[0]);
	tempv2.push_back(ys);
	tempv2.push_back(vy[2]);
	tempv2.push_back(vy[0]);
	double AreaK = AreaT(&vx, &vy);
	double xi = AreaT(&tempv1, &tempv2)/AreaK;
	tempv1.clear();
	tempv2.clear();
	tempv1.push_back(xs);
	tempv1.push_back(vx[0]);
	tempv1.push_back(vx[1]);
	tempv2.push_back(ys);
	tempv2.push_back(vy[0]);
	tempv2.push_back(vy[1]);

	double et = AreaT(&tempv1, &tempv2)/AreaK;

	double *phi1, *phi2;

	phi1 = new double [3];
	phi2 = new double [3];

	BasisTS(xi, et, phi, phi1, phi2);

	double Jac[4], JacN[4], det;

	Jac[0] = vx[0] * phi1[0] + vx[1] * phi1[1] + vx[2] * phi1[2];
	Jac[1] = vx[0] * phi2[0] + vx[1] * phi2[1] + vx[2] * phi2[2];
	Jac[2] = vy[0] * phi1[0] + vy[1] * phi1[1] + vy[2] * phi1[2];
	Jac[3] = vy[0] * phi2[0] + vy[1] * phi2[1] + vy[2] * phi2[2];

	det = Jac[0] * Jac[3] - Jac[1] * Jac [2];

	JacN[0] = Jac[3]/det;
	JacN[1] = -Jac[1]/det;
	JacN[2] = -Jac[2]/det;
	JacN[3] = Jac[0]/det;

	for (i=0; i<3; i++)
	{
		phix[i] = JacN[0] * phi1[i] + JacN[2] * phi2[i];
		phiy[i] = JacN[1] * phi1[i] + JacN[3] * phi2[i];
	}
	delete [] phi1;
	delete [] phi2;
}

void BasisQR(vector <double> vx, double xs,
double ts, double *phi, double *phix, double *phiy, double dt)
{
	unsigned int i;

	// find (xi,et) corresponding to (xs, ts)
	double et = ts/dt;
	double A = vx[0] * (1.0 - et) + vx[3] * et;
	double B = vx[1] * (1.0 - et) + vx[2] * et;
	double xi = (xs - A)/(B - A);

	double *phi1, *phi2;

	phi1 = new double [4];
	phi2 = new double [4];

	BasisQS(xi, et, phi, phi1, phi2);

	double Jac[4], JacN[4], det;

	Jac[0] = vx[0] * phi1[0] + vx[1] * phi1[1] + vx[2] * phi1[2] + vx[3] * phi1[3];
	Jac[1] = vx[0] * phi2[0] + vx[1] * phi2[1] + vx[2] * phi2[2] + vx[3] * phi2[3];
	Jac[2] = dt * phi1[2] + dt * phi1[3];
	Jac[3] = dt * phi2[2] + dt * phi2[3];

	det = Jac[0] * Jac[3] - Jac[1] * Jac [2];

	JacN[0] = Jac[3]/det;
	JacN[1] = -Jac[1]/det;
	JacN[2] = -Jac[2]/det;
	JacN[3] = Jac[0]/det;

	for (i=0; i<4; i++)
	{
		phix[i] = JacN[0] * phi1[i] + JacN[2] * phi2[i];
		phiy[i] = JacN[1] * phi1[i] + JacN[3] * phi2[i];
	}
	delete [] phi1;
	delete [] phi2;
}


void IntQT1_ellam(vector <double> vx, double D, double sw2, double **Stif, double dt)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
	vector <double>  Rx, Ry, Qx, Qy;
	double **StifR=NULL, DJac;
	double *phiR, *phiRx, *phiRy;
   double hh, slope, xn1, phiC, phiCx;
	phiR = new double [4];
	phiRx = new double [4];
	phiRy = new double [4];

	Rx.clear();
	Ry.clear();
	Rx.push_back(vx[0]);	// vertices of Q on right
	Rx.push_back(vx[1]);
	Rx.push_back(vx[3]);
	Rx.push_back(vx[2]);

	Ry.push_back(0.0);
	Ry.push_back(0.0);
	Ry.push_back(dt);
	Ry.push_back(dt);

	initialize_2d(4, 2, &StifR);
   hh = vx[3] - vx[2];
   slope = (vx[3] - vx[4])/dt;

   //
   // integration over quadrilateral element Q :
   //
   if( (vx[1]-vx[4])/(vx[1]-vx[0]) <1.e-3 ) 		// Q_{0,4,3,2} is almost degenerated into a triangle
   {
	    // elements for integration
	    //
	    Qx.clear();
	    Qy.clear();
	    Qx.push_back(vx[0]);	// vertices of Q on right
	    Qx.push_back(vx[3]);
	    Qx.push_back(vx[2]);
	    Qy.push_back(0.0);
	    Qy.push_back(dt);
	    Qy.push_back(dt);

       double **Lam;
	    npts = 28;
	    initialize_2d(npts, 4, &Lam);
	    DefineFkp(npts, Lam);

	    for (k=0; k<npts; k++)
	    {
		    x_gauss = Lam[k][0] * Qx[0] + Lam[k][1] * Qx[1] + Lam[k][2] * Qx[2];
		    y_gauss = Lam[k][0] * Qy[0] + Lam[k][1] * Qy[1] + Lam[k][2] * Qy[2];
		    DJac = 2.0 * AreaT(&Qx, &Qy);

		    //
		    // find phi, phi_x, phi_y on R and C at (x,y)
		    //

	     	 BasisQR(Rx, x_gauss, y_gauss, phiR, phiRx, phiRy, dt);

          xn1 = x_gauss + slope * ( dt - y_gauss );	// trace-forward point at t_n+1 from (x_g, y_g)
          phiC  = ( xn1 - vx[2] )/hh;		// hat function on t_n+1, =1 at vx[3]; =0 at vx[2]
          phiCx = 1./hh;

		    for (i=0; i<4; i++)
		    {
			    dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i], 1.-phiC, -phiCx);
			    StifR[i][0] += Lam[k][3] * DJac * dval;

			    dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],    phiC,  phiCx);
			    StifR[i][1] += Lam[k][3] * DJac * dval;
		    }
      }
	   clear_2d(npts, Lam);

   }
	else
	{	// Q_{0,4,3,2} is non-degenerate
		// elements for integration
		//
		Qx.clear();
		Qy.clear();
		Qx.push_back(vx[0]);	// vertices of Q on right
		Qx.push_back(vx[4]);
		Qx.push_back(vx[3]);
		Qx.push_back(vx[2]);

		Qy.push_back(0.0);
		Qy.push_back(0.0);
		Qy.push_back(dt);
		Qy.push_back(dt);

		double **Gs=NULL;
		npts = 5 * 5;
		initialize_2d(npts, 3, &Gs);
		DefineGaussian(5, Gs);

		double psi[4], psi1[4], psi2[4], jac[4];
		for (k=0; k<npts; k++)
		{
			BasisQS(Gs[k][0], Gs[k][1], psi, psi1, psi2);
			x_gauss = 0.0;
			y_gauss = 0.0;
			for (i=0; i<4; i++)
			{
				jac[i] = 0.0;
			}
			for (i=0; i<4; i++)
			{
				x_gauss += psi[i] * Qx[i];
				y_gauss += psi[i] * Qy[i];
				jac[0] += Qx[i] * psi1[i];
				jac[1] += Qx[i] * psi2[i];
				jac[2] += Qy[i] * psi1[i];
				jac[3] += Qy[i] * psi2[i];
			}
			DJac = jac[0] * jac[3] - jac[1] * jac[2];
			//
			// find phi, phi_x, phi_y on L and C at (x,y)
			//
			BasisQR(Rx, x_gauss, y_gauss, phiR, phiRx, phiRy, dt);
			xn1 = x_gauss + slope * ( dt - y_gauss );	// trace-forward point at t_n+1 from (x_g, y_g)
			phiC  = ( xn1 - vx[2] )/hh;		// hat function on t_n+1, =1 at vx[3]; =0 at vx[2]
			phiCx = 1./hh;
			for (i=0; i<4; i++)
			{
				dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i], 1.-phiC, -phiCx);
				StifR[i][0] += Gs[k][2] * DJac * dval;
				dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],    phiC,  phiCx);
				StifR[i][1] += Gs[k][2] * DJac * dval;
			}
		}
		clear_2d(npts, Gs);
	}
	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifR[0][i];
		Stif[1][i] = StifR[1][i];
		Stif[2][i] = StifR[3][i];
		Stif[3][i] = StifR[2][i];
	}
	delete [] phiR;
	delete [] phiRx;
	delete [] phiRy;
	clear_2d(4, StifR);
}

void IntQTm_ellam(vector <double> vx, double D, double sw2, double **Stif, double dt)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
	vector <double> Lx, Ly, Cx, Cy, Rx, Ry, Qx, Qy, Tx, Ty;
	double *phiR, *phiRx, *phiRy;
	double **StifL=NULL, **StifR=NULL, **Lam=NULL, DJac;
	double *phiL, *phiLx, *phiLy, *phiCx, *phiCy, *phiC;
	double **Gs=NULL;
	phiL = new double [4];
	phiLx = new double [4];
	phiLy = new double [4];
	phiCx = new double [4];
	phiCy = new double [4];
	phiC = new double [4];
	phiR = new double [4];
	phiRx = new double [4];
	phiRy = new double [4];
	Lx.clear();
	Ly.clear();
	Cx.clear();
	Cy.clear();
	Rx.clear();
	Ry.clear();
	Qx.clear();
	Qy.clear();
	Tx.clear();
	Ty.clear();

	Lx.push_back(vx[0]);
	Lx.push_back(vx[1]);
	Lx.push_back(vx[4]);
	Lx.push_back(vx[3]);

	Ly.push_back(0.0);
	Ly.push_back(0.0);
	Ly.push_back(dt);
	Ly.push_back(dt); 			// vertices of left T

	Cx.push_back(vx[6]);
	Cx.push_back(vx[7]);
	Cx.push_back(vx[5]);
	Cx.push_back(vx[4]);

	Cy.push_back(0.0);
	Cy.push_back(0.0);
	Cy.push_back(dt);
	Cy.push_back(dt);

	Rx.push_back(vx[1]);	// vertices of Q on right
	Rx.push_back(vx[2]);
	Rx.push_back(vx[5]);
	Rx.push_back(vx[4]);

	Ry.push_back(0.0);
	Ry.push_back(0.0);
	Ry.push_back(dt);
	Ry.push_back(dt);


	initialize_2d(4, 2, &StifL);
	initialize_2d(4, 2, &StifR);

	//
	// integration over triangle T:
	//
	Tx.push_back(vx[6]);	// vertices of T on left
	Tx.push_back(vx[1]);
	Tx.push_back(vx[4]);

	Ty.push_back(0.0);
	Ty.push_back(0.0);
	Ty.push_back(dt);

	npts = 28;
	initialize_2d(npts, 4, &Lam);
	DefineFkp(npts, Lam);

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
		y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
		DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;

		//
		// find phi, phi_x, phi_y on R and C at (x,y)
		//

		BasisQR(Lx, x_gauss, y_gauss, phiL, phiLx, phiLy, dt);
		BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt);

		for (i=0; i<4; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
			       						phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			StifL[i][0] += Lam[k][3] * DJac * dval;

			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
					 						phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			StifL[i][1] += Lam[k][3] * DJac * dval;
		}
	}
	clear_2d(npts, Lam);

   //
   // integration over quadrilateral element Q :
   //
   if( (vx[7]-vx[1])/(vx[2]-vx[1]) <1.e-3 ) 		// Q_{1,7,5,4} is almost degenerated into a triangle
   {
	    Qx.push_back(vx[1]);	// vertices of Q on right
	    Qx.push_back(vx[5]);
	    Qx.push_back(vx[4]);
	    Qy.push_back(0.0);
	    Qy.push_back(dt);
	    Qy.push_back(dt);

	    npts = 28;
	    initialize_2d(npts, 4, &Lam);
	    DefineFkp(npts, Lam);

	    for (k=0; k<npts; k++)
	    {
		    x_gauss = Lam[k][0] * Qx[0] + Lam[k][1] * Qx[1] + Lam[k][2] * Qx[2];
		    y_gauss = Lam[k][0] * Qy[0] + Lam[k][1] * Qy[1] + Lam[k][2] * Qy[2];
		    DJac = 2.0 * AreaT(&Qx, &Qy);

		    //
		    // find phi, phi_x, phi_y on R and C at (x,y)
		    //

	     	 BasisQR(Rx, x_gauss, y_gauss, phiR, phiRx, phiRy, dt);
		    BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt);
		    for (i=0; i<4; i++)
		    {
			    dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
											    phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			    StifR[i][0] += Lam[k][3] * DJac * dval;

			    dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
											    phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			    StifR[i][1] += Lam[k][3] * DJac * dval;
		    }
       }
	    clear_2d(npts, Lam);
   }
   else 				// Q is a non-degenerate quadrilateral
   {
	    Qx.push_back(vx[1]);	// vertices of Q on right
	    Qx.push_back(vx[7]);
	    Qx.push_back(vx[5]);
	    Qx.push_back(vx[4]);

	    Qy.push_back(0.0);
	    Qy.push_back(0.0);
	    Qy.push_back(dt);
	    Qy.push_back(dt);

	    npts = 5 * 5;
	    initialize_2d(npts, 3, &Gs);
	    DefineGaussian(5, Gs);

	    double psi[4], psi1[4], psi2[4], jac[4];
	    for (k=0; k<npts; k++)
	    {
		    BasisQS(Gs[k][0], Gs[k][1], psi, psi1, psi2);

		    x_gauss = 0.0;
		    y_gauss = 0.0;
		    for (i=0; i<4; i++)
		    {
			    jac[i] = 0.0;
		    }
		    for (i=0; i<4; i++)
		    {
			    x_gauss += psi[i] * Qx[i];
			    y_gauss += psi[i] * Qy[i];
			    jac[0] += Qx[i] * psi1[i];
			    jac[1] += Qx[i] * psi2[i];
			    jac[2] += Qy[i] * psi1[i];
			    jac[3] += Qy[i] * psi2[i];
		    }

		    DJac = jac[0] * jac[3] - jac[1] * jac[2];

		    //
		    // find phi, phi_x, phi_y on L and C at (x,y)
		    //

		    BasisQR(Rx, x_gauss, y_gauss, phiR, phiRx, phiRy, dt);
		    BasisQR(Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt);
		    for (i=0; i<4; i++)
		    {
			    dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
											    phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
			    StifR[i][0] += Gs[k][2] * DJac * dval;

			    dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
											    phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
			    StifR[i][1] += Gs[k][2] * DJac * dval;
		    }
	    }
	    clear_2d(npts, Gs);
   }

	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifL[0][i];
		Stif[1][i] = StifL[1][i] + StifR[0][i];
		Stif[2][i] = 					StifR[1][i];
		Stif[3][i] = StifL[3][i];
		Stif[4][i] = StifL[2][i] + StifR[3][i];
		Stif[5][i] = 					StifR[2][i];
	}
	delete [] phiR;
	delete [] phiRx;
	delete [] phiRy;
	delete [] phiL;
	delete [] phiLx;
	delete [] phiLy;
	delete [] phiCx;
	delete [] phiCy;
	delete [] phiC;

	clear_2d(3, StifL);
	clear_2d(4, StifR);
}


void IntQTn1_ellam(vector <double> vx, double D, double sw2, double **Stif, double dt)
{
	// element to define basis functions
	//
	unsigned int npts, i, k;
	double x_gauss, y_gauss, dval;
	vector <double> Lx, Ly, Tx, Ty;
	double **StifL=NULL, **Lam=NULL, DJac;
	double *phiL, *phiLx, *phiLy;
	phiL = new double [4];
	phiLx = new double [4];
	phiLy = new double [4];
	Lx.clear();
	Ly.clear();
	Tx.clear();
	Ty.clear();

	Lx.push_back(vx[0]);
	Lx.push_back(vx[1]);
	Lx.push_back(vx[3]);
	Lx.push_back(vx[2]);

	Ly.push_back(0.0);
	Ly.push_back(0.0);
	Ly.push_back(dt);
	Ly.push_back(dt);

	initialize_2d(4, 2, &StifL);

	//
	// integration over T:
	//

	Tx.push_back(vx[4]);	// triangle for integration
	Tx.push_back(vx[1]);
	Tx.push_back(vx[3]);

	Ty.push_back(0.0);
	Ty.push_back(0.0);
	Ty.push_back(dt);

	npts = 28;
	initialize_2d(npts, 4, &Lam);
	DefineFkp(npts, Lam);

	for (k=0; k<npts; k++)
	{
		x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
		y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
		DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;
		//
		// find phi, phi_x, phi_y on R and C at (x,y)
		//

		BasisQR(Lx, x_gauss, y_gauss, phiL, phiLx, phiLy, dt);

		for (i=0; i<4; i++)
		{
			dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i], 1.0, 0.0);
			StifL[i][0] += Lam[k][3] * DJac * dval;
		}
	}
	clear_2d(npts, Lam);


	for (i=0; i<2; i++)
	{
		Stif[0][i] = StifL[0][i];
		Stif[1][i] = StifL[1][i];
		Stif[2][i] = StifL[3][i];
		Stif[3][i] = StifL[2][i];
	}
	delete [] phiL;
	delete [] phiLx;
	delete [] phiLy;

	clear_2d(4, StifL);
}


void QuadSolver_ellam(double *ai, double *bi, double *ci, double *di, double *cr, double *solu, unsigned int N)
{
//
// solve Quad-diagonal system [a_i, b_i, *c_i*, d_i]*[x]=[r_i]
// c_i are on the main diagonal line
//

	unsigned int i;
	double tmp;
	vector<double> ca, cb, cc, cd;
	ca.clear();
	cb.clear();
	cc.clear();
	cd.clear();
	for (i=0; i<N; i++)
	{
		ca.push_back( ai[i] );
		cb.push_back( bi[i] );
		cc.push_back( ci[i] );
		cd.push_back( di[i] );
	}

	for (i=N-2; i>=1; i--)
	{
		tmp = cd[i]/cc[i+1];
		cc[i] = cc[i]-cb[i+1]*tmp;
		cb[i] = cb[i]-ca[i+1]*tmp;
		cr[i] = cr[i]-cr[i+1]*tmp;
	}
   i=0;
	tmp = cd[i]/cc[i+1];
	cc[i] = cc[i]-cb[i+1]*tmp;
	cr[i] = cr[i]-cr[i+1]*tmp;
	solu[0] = cr[0] / cc[0];
	solu[1] = (cr[1] - cb[1] * solu[0]) / cc[1];
	i = 1;
	do
	{
		i++;
		solu[i] = (cr[i] - ca[i] * solu[i-2] - cb[i] * solu[i-1]) / cc[i];
	} while (i != N-1);
}
// ******* end of ELLAM *********************


////////////////////////////////////////////////////////////////////////%
// calculate total mass
// (r,u) concentration defined at r(1), ...., r(M)
//  M: r(1).... r(M): the interval for total mass, (M-1) subintervals
////////////////////////////////////////////////////////////////////////%

double IntConcentration(vector<double> r, double *u)
{
//function T=IntConcentration(r,M,u)
	double T = 0.0;
	for (unsigned int j=0; j<r.size()-1; j++)
	{
		T += (r[j+1] - r[j]) * ((r[j+1] - r[j]) * (u[j+1]/3.0 + u[j]/6.0)
			+ r[j] * (u[j] + u[j+1])/2.0);
	}
	return T;
}

////////////////////////////////////////
//
// find_C1_mono_Nmer:	find C1 from    C1 + K * C1^n = CT
//
////////////////////////////////////////
double find_C1_mono_Nmer( int n, double K, double CT )
{
   //
   // use Newton's method for f(x) = x + K*x^n - CT
   //    x_{i+1} = x_i - f(x_i)/f'(x_i)
   //
   double x0=0., x1;
	unsigned int i, MaxNumIt = 1000;

   if ( CT<=0. ) return( 0. );
   if ( CT<=1.e-12 ) return( CT );

	for ( i=1; i < MaxNumIt; i++)
   {
		x1 = ( K * (n-1.0) * pow(x0,(double)n) + CT ) / ( 1. + K * n * pow(x0, n-1.) );
	   if ( fabs(x1 - x0)/(1.+fabs(x1)) < 1.e-12 ) break;
      x0 = x1;
	}

   if( i == MaxNumIt )
   {
      cerr << "warning: Newton's method did not coonverges in find_C1_mono_Nmer"<<endl;
      return( -1.0 );
   }
   return( 0.5*(x0+x1) );
}

//////////////////////////////////////////////////////////////////
//
// cub_root: find the positive cubic-root of a cubic polynomial
//			p(x)=a0+a1*x+a2*x^2+x^3
//
// with: a0<=0,  and  a1, a2>=0;
//
//////////////////////////////////////////////////////////////////
double cube_root(double a0, double a1, double a2)
{
	double B, Q, S, D, Dc, Dc2, theta, x;
	Q = (3.0 * a1 - pow(a2, 2.0)) / 9.0;
	S = (9.0 * a1 * a2 - 27.0 * a0 - 2.0 * pow(a2, 3.0)) / 54.0;
	D = pow(Q, 3.0) + pow(S, 2.0);
	if (D < 0)
	{
		theta = acos(S / sqrt(pow(-Q, 3.0)));
		x = 2.0 * sqrt(-Q) * cos(theta / 3.0);
	}
	else
	{
		Dc = sqrt(D);
		if (S + Dc < 0)
		{
			B = - pow(-(S + Dc), 1.0/3.0) - pow(Dc - S, 1.0/3.0);
		}
		else if (S - Dc < 0)
		{
			B = pow(S+Dc, 1.0/3.0) - pow(Dc - S, 1.0/3.0);
		}
		else
		{
			B = pow(S + Dc, 1.0/3.0) + pow(S - Dc, 1.0/3.0);
		}
		Dc2 = -3.0 * (pow(B, 2.0) + 4 * Q);
		if (Dc2 > 0)
		{
			x = max(B, 0.5 * (-B + sqrt(Dc2)));
		}
		else
		{
			x = B;
		}
	}
	x = x - a2 / 3.0;
	return(x);
}


/////////////////////////////////////////////////////////////////
//
// Gass Elimination for n X n system: Ax=b
//
// return value: -1: A singular, no solution,
//                1: successful
// in return:     A has been altered, and b stores the solution x
//
/////////////////////////////////////////////////////////////////
int GaussElim(int n, double **a, double *b)
{
    // ellimination
    int i, j, ip, Imx;
    double tmp, *ptmp, amx;

    for(i=0; i<n; i++) {

      // find the pivot
      amx = fabs(a[i][i]);
      Imx = i;
      for(ip=i+1; ip<n; ip++) {
        if(fabs(a[ip][i])>amx) {
          amx = fabs(a[ip][i]);
          Imx = ip;
        }
      }
      if(amx ==0) {
        printf("Singular matrix in routine GaussElim");
        return -1;
     }

      // interchange i-th and Imx-th row
      if(Imx!=i) {
        ptmp = a[i]; a[i] = a[Imx]; a[Imx] = ptmp;
        tmp  = b[i]; b[i] = b[Imx]; b[Imx] = tmp;
      }

      // ellimination
      tmp = a[i][i];
      for(j=i; j<n;j++) a[i][j] /= tmp;
      b[i] /= tmp;
      for(ip=i+1; ip<n; ip++) {
        tmp = a[ip][i];
        for(j=i+1; j<n;j++) a[ip][j] -= tmp*a[i][j];
        b[ip] -= tmp*b[i];
      }
    }

    // backward substitution
    for (i=n-2;i>=0;i--) {
      for(j=i+1; j<n; j++) b[i] -= a[i][j]*b[j];
    }

    return(1);
}


void DefInitCond(double **C0, unsigned int N)
{
	unsigned int j;
	for(j=0; j<N; j++)
	{
		C0[0][j] = 0.3;
		C0[1][j] = 0.7;
	}
}

int interpolate(struct mfem_data *simdata, unsigned int scans, unsigned int points,
float *scantimes, double *radius, double **c)
{

/********************************************************************************************
 * interpolation:																									*
 *																														*
 * First, we need to interpolate the time. Create a new array with the same time dimensions *
 * as the raw data and the same radius dimensions as the simulated data. Then find the time *
 * steps from the simulated data that bracket the experimental data from the left and right.*
 * Then make a linear interpolation for the concentration values at each radius point from  *
 * the simulated data. Then interpolate the radius points by linear interpolation.			*
 *																														*
 ********************************************************************************************/

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
		if (scantimes[i] == (*simdata).scan[count].time)
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
			if (radius[j] == (*simdata).radius[count])
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


// interpolation routine By B. Demeler 041708
int interpolate(struct mfem_data *expdata, struct mfem_data *simdata, double omega_s, bool acceleration)
{
// NOTE: *expdata has to be initialized to have the proper size (filled with zeros)
// before using this routine! The radius also has to be assigned!

	if ((*expdata).scan.size() == 0 || (*expdata).scan[0].conc.size() == 0 ||
		 (*simdata).scan.size() == 0 || (*simdata).radius.size() == 0)
	{
		return -1;
	}

	unsigned int i, j, simscan, expscan;
	double a, b;
	/*
	if (acceleration) // we model rotor acceleration and need to correct the time
	{
		double time_correction = 0.0;
		for (i=0; i<(*simdata).scan.size(); i++)
		{
			time_correction += (*simdata).scan[i].time - ((*simdata).scan[i].omega_s_t/omega_s);
		}
		time_correction /= (*simdata).scan.size();
		for (i=0; i<(*simdata).scan.size(); i++)
		{
			(*simdata).scan[i].time -= time_correction;
		}
	}
	*/
	// first, create a temporary mfem_data structure (tmp_data) that has the same radial
	// grid as simdata, but the same time grid as the experimental data. The time
	// and w2t integral values are interpolated for the tmp_data structure.

	mfem_data tmp_data;
	mfem_scan tmp_scan;
	tmp_data.scan.clear();
	tmp_data.radius.clear();
	// fill tmp_data.radius with radius positions from the simdata array:

	for (i=0; i<(*simdata).radius.size(); i++)
	{
		tmp_data.radius.push_back((*simdata).radius[i]);
	}

	// iterate through all experimental data scans and find the first time point in simdata
	// that is higher or equal to each time point in expdata:

	simscan = 0;

	for (expscan = 0; expscan < (*expdata).scan.size(); expscan++)
	{
		while ((*simdata).scan[simscan].time < (*expdata).scan[expscan].time)
		{
			simscan ++;
			// make sure we don't overrun bounds:
			if (simscan == (*simdata).scan.size())
			{
				cerr << "simulation time scan[" << simscan << "]: " << (*simdata).scan[simscan-1].time
						<< ", expdata scan time[" << expscan << "]: " << (*expdata).scan[expscan].time << endl;
				cerr << QObject::tr("The simulated data does not cover the entire experimental time range and ends too early!\nexiting...\n");
				exit(-1);
			}
		}
		// check to see if the time is equal or larger:
		if ((*simdata).scan[simscan].time == (*expdata).scan[expscan].time)
		{ // they are the same, so take this scan and push it onto the tmp_data array.
			tmp_data.scan.push_back((*simdata).scan[simscan]);
		}
		else // interpolation is needed
		{
			tmp_scan.conc.clear();
			// interpolate the concentration points:
			for (i=0; i<(*simdata).radius.size(); i++)
			{
				a = ((*simdata).scan[simscan].conc[i] - (*simdata).scan[simscan-1].conc[i])
				/ ((*simdata).scan[simscan].time - (*simdata).scan[simscan-1].time);
				b = (*simdata).scan[simscan].conc[i] - a *(*simdata).scan[simscan].time;
				tmp_scan.conc.push_back(a * (*expdata).scan[expscan].time + b);
			}
			// interpolate the omega_square_t integral data:
			a = ((*simdata).scan[simscan].omega_s_t - (*simdata).scan[simscan-1].omega_s_t)
				/ ((*simdata).scan[simscan].time - (*simdata).scan[simscan-1].time);
			b = (*simdata).scan[simscan].omega_s_t - a *(*simdata).scan[simscan].time;
			(*expdata).scan[expscan].omega_s_t = a * (*expdata).scan[expscan].time + b;
			tmp_data.scan.push_back(tmp_scan);
		}
	}
   // interpolate all radial points from each scan in tmp_data onto expdata
	for (expscan = 0; expscan<(*expdata).scan.size(); expscan++)
	{
		j = 0;
		if (j == 0 && tmp_data.radius[0] > (*expdata).radius[0])
		{
			cerr << "Radius comparison: " << tmp_data.radius[0] << " (simulated), " << (*expdata).radius[0] << " (experimental)\n";
			cerr << "j = " << j << ", simdata radius: " << tmp_data.radius[j] << ", expdata radius: " << (*expdata).radius[i] << endl;
			cerr << QObject::tr("The simulated data radial range does not include the beginning of the experimental data's radii!\nexiting...\n");
			exit(-3);
		}
		for (i=0; i<(*expdata).radius.size(); i++)
		{
			while (tmp_data.radius[j] < (*expdata).radius[i])
			{
				j ++;
				// make sure we don't overrun bounds:
				if (j == tmp_data.radius.size())
				{
					cerr << QObject::tr("The simulated data does not have enough radial points and ends too early!\nexiting...\n");
					exit(-2);
				}
			}
			// check to see if the radius is equal or larger:
			if (tmp_data.radius[j] == (*expdata).radius[i])
			{ // they are the same, so simply update the concentration value:
				(*expdata).scan[expscan].conc[i] += tmp_data.scan[expscan].conc[j];
			}
			else // interpolation is needed
			{
				a = (tmp_data.scan[expscan].conc[j] - tmp_data.scan[expscan].conc[j-1])
				/ (tmp_data.radius[j] - tmp_data.radius[j-1]);
				b = tmp_data.scan[expscan].conc[j] - a * tmp_data.radius[j];
				(*expdata).scan[expscan].conc[i] += a * (*expdata).radius[i] + b;
			}
		}
	}
	return(0);
}

void interpolate_C0(struct mfem_initial *C0, double *C1, vector <double> *x )
{
	unsigned int i, j, ja;
	double a, b, xs, tmp;

	ja = 0;
	for(j=0; j<(*x).size(); j++)
	{
		xs = (*x)[j];
		for(i=ja; i<(*C0).radius.size(); i++)
		{
			if((*C0).radius[i] > xs + 1.e-12)
			{
				break;
			}
		}

		if ( i == 0 ) 				// x[j] < C0.radius[0]
		{
			C1[j] = (*C0).concentration[0];		// use the first value
		}
		else if ( i == (*C0).radius.size() ) 	// x[j] > last point in (*C0).radius
		{
			C1[j] = (*C0).concentration[ i-1 ];
		}
		else
		{
			a = (*C0).radius[i-1];
			b = (*C0).radius[i];
			tmp = (xs - a)/(b - a);
			C1[j] = (*C0).concentration[i-1] * (1. - tmp) + (*C0).concentration[i] * tmp;
			ja = i-1;
		}
	}
}

void interpolate_C0(struct mfem_initial *C0, struct mfem_initial *C1 ) // original grid: C0, final grid: C1
{
	unsigned int i, j, ja;
	double a, b, xs, tmp;

	ja = 0;
	for(j=0; j<(*C1).radius.size(); j++)
	{
		xs = (*C1).radius[j];
		for(i=ja; i<(*C0).radius.size(); i++)
		{
			if((*C0).radius[i] > xs + 1.e-12)
			{
				break;
			}
		}

		if ( i == 0 ) 				// x[j] < C0.radius[0]
		{
			(*C1).concentration[j] = (*C0).concentration[0];		// use the first value
		}
		else if ( i == (*C0).radius.size() ) 	// x[j] > last point in (*C0).radius
		{
			(*C1).concentration[j] = (*C0).concentration[ i-1 ];
		}
		else
		{
			a = (*C0).radius[i-1];
			b = (*C0).radius[i];
			tmp = (xs - a)/(b - a);
			(*C1).concentration[j] = (*C0).concentration[i-1] * (1. - tmp) + (*C0).concentration[i] * tmp;
			ja = i-1;
		}
	}
}

void interpolate_Cfinal(struct mfem_initial *C0, double *cfinal, vector <double> *x )
{
// this routine also considers cases where C0 starts before the meniscus or stops
// after the bottom is reached, however, C0 needs to cover both meniscus and bottom
// In those cases it will fill the C0 vector with the first and/or last value of C0,
// respectively, for the missing points.

	unsigned int i, j, ja;
	double a, b, xs, tmp;

	ja = 0;
	for(j=0; j<(*C0).radius.size(); j++)
	{
		xs = (*C0).radius[j];
		for (i=ja; i<(*x).size(); i++)
		{
			if( (*x)[i] > xs + 1.e-12)
			{
				break;
			}
		}

		if ( i == 0 ) 				// xs < x[0]
		{
			(*C0).concentration[j] = cfinal[0];		// use the first value
		}
		else if ( i == (*x).size() ) 	   // xs > x[N]
		{
			(*C0).concentration[j] = cfinal[ i-1 ];
		}
		else 							// x[i-1] < xs <x[i]
		{
			a = (*x)[i-1];
			b = (*x)[i];
			tmp = (xs-a)/(b-a);
			(*C0).concentration[j] = cfinal[i-1] * (1. - tmp) + cfinal[i] * tmp;
			ja = i-1;
		}
	}
}
