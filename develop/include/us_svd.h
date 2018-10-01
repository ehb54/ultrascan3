/* 
 * svdcomp - SVD decomposition routine. 
 * Takes an mxn matrix a and decomposes it into udv, where u,v are
 * left and right orthogonal transformation matrices, and d is a 
 * diagonal matrix of singular values.
 *
 * This routine is adapted from svdecomp.c in XLISP-STAT 2.1 which is 
 * code from Numerical Recipes adapted by Luke Tierney and David Betz.
 *
 * Input to dsvd is as follows:
 *   a = mxn matrix to be decomposed, gets overwritten with u
 *   m = row dimension of a
 *   n = column dimension of a
 *   w = preallocated n-vector returns the vector of singular values of a
 *   v = preallocated nxn matrix returns the right orthogonal transformation matrix
 */

#ifndef US_SVD
#define US_SVD

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <vector>

namespace SVD {
   extern char errormsg[ 128 ];

   // work in vector or dp space?
   // np.dot
   // np.sum
   // np.abs
   // np.transpose
   // np.ones
   
   // np.pinv
   bool pinv( std::vector < std::vector < double > > A, std::vector < std::vector < double > > &Ainv ); // moore-penrose pseudo inverse Compute SVD A=UΣVT Ainv=VΣ+UT

   void dp_inv_nz( double *dp, int m ); // invert nonzeros inplace

   void dpp_to_vvd( double **dpp, int m, int n, std::vector < std::vector < double > > &vvd );
   void cout_vvd( const char *tag, std::vector < std::vector < double > > &vvd );
   void cout_vvi( const char *tag, std::vector < std::vector < int > > &vvi );
   void cout_vd( const char *tag, std::vector < double > &vd );
   void cout_vd( const char *tag, std::vector < double > &vd );
   void cout_dpp( const char *tag, double **dpp, int m, int n );
   void cout_dp( const char *tag, double *dp, int m );


   bool dsvd(double **a, int m, int n, double *w, double **v);
   void vvd_shape( std::vector < std::vector < double > > &vvd, int & m, int & n );
   std::vector < std::vector < double > > vvd_transpose( std::vector < std::vector < double > > &vvd );
   
   void dvalidate(double **a, int m, int n, double *w, double **v);
   void dinfo(double **a, int m, int n, double *w, double **v);
 
   bool fsvd(float **a, int m, int n, float *w, float **v);
}


using namespace std;

class svd_sortable_double {
public:
   double       x;
   int          index;
   bool operator < (const svd_sortable_double& objIn) const
   {
      return x < objIn.x;
   }
};

#endif // US_SVD
