//! \file us_matrix.cpp

#include "us_matrix.h"
#include "us_math2.h"

#include <QtCore>

bool US_Matrix::lsfit( double* c, double* x, double* y,
                       int N, int order )
{
   bool status = true;

   double** A = new double* [ order ];
   
   for ( int i = 0; i < order; i++ ) 
      A[ i ] = new double [ order ]; 

   double* b = new double [ order ];

   /* To calculate the matrix, calculate the product M'M, where M is the matrix
      constructed as follows:


            x1^0   x1^1   x1^2 ... x1^order
            x2^0   x2^1   x2^2 ... x2^order
            x3^0   x3^1   x3^2 ... x3^order

      M =   .      .      .
            .      .      .  ...
            .      .      .
    
            xn^0   xn^1   xn^2 ... xn^order

      A = M'M, and A is positive - definite, and can be decomposed by the
      Cholesky method.  Evaluate A by filling in the lower triangular of the A
      matrix. NOTE: Instead of polynomials evaluated at each point xi, other
      linearly independent basis function sets can be used for a fit.

   */
   
   A[ 0 ][ 0 ] = (double) N;
   
   for ( int i = 1; i < order; i++ )
   {
      for ( int j = 0; j <= i; j++ )
      {
         A[ i ][ j ] = 0;
         
         for ( int k = 0; k < N; k++ )
            A[ i ][ j ] += pow( x[ k ], i ) * pow( x[ k ], j );
      }
   } 

   // Only the lower triangular matrix is filled now, which is sufficient for
   // solution by Cholesky decomposition.
   
   // Evaluate b:

   for ( int i = 0; i < order; i++ )
   {
      b[ i ] = 0;

      for ( int k = 0; k < N; k++ )
         b[ i ] += y[ k ] * pow( x[ k ], i );
   }
   

   // Solve the system Ax=b using Cholesky decomposition:

   // Ax=b, A=LL', L(L'x)=b, L'x=y, Ly=b (solve for y), now L'x=y (solve for x)
   //print_matrix( A, order, order);
   //print_vector( b, order );
   if( Cholesky_Decomposition( A, order ) )
   {
      Cholesky_SolveSystem  ( A, b, order );
      for ( int i = 0; i < order; i++ )  c[ i ] = b[ i ];
   }
   else
      status = false;

   //print_matrix( A, order, order);
   //print_vector( b, order );
   //print_matrix( A, order, order);
   //print_vector( b, order );
   
   
   // Clean up
   for ( int i = 0; i < order; i++ ) delete A[ i ];
   
   delete [] A;
   delete [] b;
   return status;
}

/* This method factors the n by n symmetric positive definite matrix A as LL(T)
   where L is a lower triangular matrix.  The method assumes that at least the
   lower triangle of A is filled on entry. On exit, the lower triangle of A has
   been replaced by L.
*/

bool US_Matrix::Cholesky_Decomposition( double** a, int n )
{
   double sum;
   double diff;

   for ( int i = 0; i < n; i++ )
   {
      sum = 0.0;

      for ( int j = 0; j < i; j++ )
      {
         sum += sq( a[ i ][ j ] );
      }
      
      diff = a[ i ][ i ] - sum;
      
      if ( diff <= 0.0 )// not positive definite...
      { 
         qDebug() << "Cholesky_Decomposition not positive definite.";
         return false;
      }

      a[ i ][ i ] = sqrt( diff );

      for ( int k = i + 1; k < n; k++ )
      {
         sum = 0.0;

         for ( int j = 0; j < i; j++ ) 
            sum += a[ k ][ j ] * a[ i ][ j ];
         
         a[ k ][ i ] = ( a[ k ][ i ] - sum ) / a[ i ][ i ];
      }
   } 

   // Zero the upper triangular portion
   for ( int i = 0; i < n - 1; i++ )
   {
      for ( int j = i + 1; j < n; j++ ) 
         a[ i ][ j ] = 0.0;
   }

   return true;
} 

/* Cholesky_SolveSystem expects a Cholesky-decomposed L-matrix (n x n) with the 
   lower diagonal filled, and the right hand side "b". Using forward and
   backward substitution, the right hand side "b" is replaced by the solution
   vector "x"
*/
bool US_Matrix::Cholesky_SolveSystem( double** L, double* b, int n ) 
{
QString t;
   // Forward substitution:
   for ( int i = 0; i < n; i++ )
   {
      int j;
      for ( j = 0; j < i; j++ ) 
      {
         b[ i ] -=  L[ i ][ j ] * b[ j ];
      }

      b[ i ] /= L[ i ][ i ];
   }

   // Backward substitution:
   for ( int i = n - 1; i >= 0; i-- )
   {
      for ( int j = n - 1; j > i; j-- )  
      {
         b[ i ] -= L[ j ][ i ] * b[ j ]; 
      }

      b[i] /=  L[ i ][ i ];

   }

   return true;
}


void US_Matrix::print_vector( double* v, int n )
{
   QString s;
   QString t;
   for ( int i = 0; i < n; i++ ) s += t.sprintf( "%.15f ", v[ i ] );

   qDebug() << s;
}

void US_Matrix::print_matrix( double** A, int rows, int columns )
{
   for ( int i = 0; i < rows; i++ ) print_vector( A[ i ], columns );
   qDebug() << "\n";
}

/* luDecomposition performs LU Decomposition on a matrix. This routine
   must be given an array to mark the row permutations and a flag to mark
   whether the number of permutations was even or odd.  Reference: Numerical
   Recipes in C.
*/

void US_Matrix::LU_Decomposition( double** matrix, int* index, bool parity, int n )
{
   // imax is position of largest element in the row.
   int imax = 0;
   
   // amax is value of largest element in the row. 
   // dum is a temporary variable.

   double amax;
   double dum = 0;

   // scaling factor for each row is stored here
   double* scaling = new double[ n ];

   // a small number != zero
   double tiny = 1.0E-20;
   
   // Is the number of pivots even?
   parity = true;

   // Loop over rows to get the scaling information
   // The largest element in the row is the inverse of the scaling factor.
   for ( int i = 0; i < n; i++ ) 
   {
      amax = 0;

      for ( int j = 0; j < n; j++ ) 
      {
         if ( fabs( matrix[ i ][ j ]) > amax ) amax = matrix[ i ][ j ]; 
      }

      if ( amax == 0 )
      { 
         qDebug() << "Singular Matrix";
      }

      // Save the scaling
      scaling[ i ] = 1.0 / amax;
   }

   // Loop over columns using Crout's Method.
   for ( int j = 0; j < n; j++ ) 
   {
      // lower left corner
      for ( int i = 0; i < j; i++ ) 
      {
         dum = matrix[ i ][ j ];

         for ( int k = 0; k < i; k++ )
            dum -= matrix[ i ][ k ] * matrix[ k ][ j ];
         
         matrix[ i ][ j ] = dum;
      }

      // Initialize search for largest element
      amax = 0.0;
      
      // upper right corner
      for ( int i = j; i < n; i++ ) 
      {
         dum = matrix[ i ][ j ];

         for ( int k = 0; k < j; k++ ) 
            dum -= matrix[ i ][ k ] * matrix[ k ][ j ];
         
         matrix[ i ][ j ] = dum;

         if ( scaling[ i ] * fabs( dum ) > amax ) 
         {
            amax = scaling[ i ]* fabs( dum );
            imax = i;
         }
      }

      // Change rows if it is necessary
      if ( j != imax)
      {
         for ( int k = 0; k < n; k++ ) 
         {
            dum                 = matrix[ imax ][ k ];
            matrix[ imax ][ k ] = matrix[ j    ][ k ];
            matrix[ j    ][ k ] = dum;
         }

         // Change parity
         parity = ! parity;
         scaling[ imax ] = scaling[ j ];
      }

      // Mark the column with the pivot row.
      index[ j ] = imax;

      // replace zeroes on the diagonal with a small number.
      if ( matrix[ j ][ j ] == 0.0 ) matrix[ j ][ j ] = tiny; 

      // Divide by the pivot element
      if ( j != n ) 
      {
         dum = 1.0 / matrix[ j ][ j ];
         for ( int i = j + 1; i < n; i++ )  matrix[ i ][ j ] *= dum; 
      }
   }

   delete [] scaling;
}

/*  Do the backsubstitution on matrix a which is the LU decomposition
    of the original matrix. b is the right hand side vector which is NX1. b is
    replaced by the solution. index is the array that marks the row
    permutations.
*/

void US_Matrix::LU_BackSubstitute( double** A, double*& b, int* index, int n ) 
{
   int ip;
   int ii = -1;
   
   double sum;

   for ( int i = 0; i < n; i++ ) 
   {
      ip      = index[ i ];
      sum     = b[ ip ];
      b[ ip ] = b[ i ];
      
      if ( ii != -1 ) 
      {
         for ( int j = ii; j < i; j++ )  sum -= A[ i ][ j ] * b[ j ]; 
      }
      else 
         if ( sum != 0 )  ii = i; 
      
      b[ i ] = sum;
   }

   for ( int i = n - 1; i >= 0; i-- )
   {
      sum = b[ i ];
      
      for ( int j = i + 1; j < n; j++ ) sum -= A[ i ][ j ] * b[ j ]; 
      
      b[ i ] = sum / A[ i ][ i ];
   }
}

/* Solve a set of linear equations. a is a square matrix of coefficients.
   b is the right hand side. b is replaced by solution.  Target is replaced by
   its LU decomposition.
*/

void US_Matrix::LU_SolveSystem( double** A, double*& b, int n )
{
   bool parity = true;
   int* index  = new int[ n ];
   
   LU_Decomposition ( A, index, parity, n );
   LU_BackSubstitute( A, b, index, n );
   delete [] index;
}
