//! \file us_stiffbase.h
#ifndef US_STIFFBASE_H
#define US_STIFFBASE_H

#include "us_extern.h"

//! Set a convenient shorthand
#define uint unsigned int

//! \brief Stiff Base object
class US_UTIL_EXTERN US_StiffBase {
 public:
  //! \brief Allocates memory structures for the integration
  US_StiffBase();

  //! \brief Cleans up memory allocation
  ~US_StiffBase();

  //! \brief Triplet class (x,y,w) for gaussians
  class Gauss2D {
   public:
    double x;  //!< X coordinate
    double y;  //!< Y coordinate
    double w;  //!< W coordinate
  };

  //! \brief  Numerical integration over a quadrilateral
  //! \param NK   Order of phi arrays: 3 or 4
  //! \param xd   XD 4 x 2 matrix
  //! \param D    Diffusion coefficient
  //! \param sw2  Omega-squared
  //! \param Stif Stif matrix
  void CompLocalStif(int, double[4][2], double, double, double**);

 private:
  // parameters for triangular elements
  int n_gaussT;    // number of Gauss pts for numerical quadrature
  int n_basisT;    // number of basis functions, =4 for linear quad elem
  Gauss2D* xgT;    // list of all Gauss pts and weights
  double** phiT;   // all basis functions at Gauss pts
  double** phiT1;  // xi -derivative of all basis functions at Gauss pts
  double** phiT2;  // eta-derivative of all basis functions at Gauss pts

  // parameters for quadrilateral elements
  int n_gaussQ;    // number of Gauss pts for numerical quadrature
  int n_basisQ;    // number of basis functions, =4 for linear quad elem
  Gauss2D* xgQ;    // list of all Gauss pts and weights
  double** phiQ;   // all basis functions at Gauss pts
  double** phiQ1;  // xi -derivative of all basis functions at Gauss pts
  double** phiQ2;  // eta-derivative of all basis functions at Gauss pts

  void LambdaG(uint, double, double, double, Gauss2D*);
  void SetGauss(void);

  // map a Gauss point (gauss_ind) in standard elem to real elem (*x)
  void AffineMapping(int, double[4][2], int, double[5]);

  // Jacobian d_xi/d_x of the mapping at the Gauss pts (gauss_ind)
  void Jacobian(int, double[4][2], int, double[5]);

  // compute all basis functions and their derivatives at Gauss pts
  void LinearBasis(void);
};
#endif
