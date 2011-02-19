//! \file us_matrix.cpp

#include "us_matrix.h"
#include "us_math2.h"

#include <QtCore>

bool US_Matrix::lsfit( double* c, double* x, double* y,
                       int N, int order )
{
   bool status = true;

   QVector< double* > vecA;
   QVector< double >  datA;
   QVector< double >  datb( order );

   double** A  = construct( vecA, datA, order, order );
   double*  b  = datb.data();

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

// Solve the system Ax=b using Cholesky decomposition:
//    A * A(-1) = I ;  A = LL'  ;  L (L' * A(-1) ) = I ;
//    L * y = I (solve for y) ,  now  L' * A(-1) = y  (solve for A(-1))
bool US_Matrix::Cholesky_Invert( double** AA, double** AI, int nn )
{
   QVector< double > workvec( nn );
   double* work = workvec.data();

   // Decompose A:

   if ( ! US_Matrix::Cholesky_Decomposition( AA, nn ) )
      return false;

   for ( int jj = 0; jj < nn; jj++ )
   {
      // Set up A-inverse to contain the Identity matrix:
      workvec.fill( 0.0, nn );
      workvec[ jj ] = 1.0;
      work          = workvec.data();

      // Solve for each column j:
      US_Matrix::Cholesky_SolveSystem( AA, work, nn );

      // Assign the solution to the appropriate column of A-inverse:
      for ( int ii = 0; ii < nn; ii++ )
         AI[ ii ][ jj ] = work[ ii ];

   }

   return true;
}

// Construct and initialize a matrix, using a pair of QVectors.
// One QVector holds all data values (doubles); the other holds 
// pointers to columns-sized segments of that data array.
double** US_Matrix::construct( QVector< double* >& QVm,
      QVector< double >& QVd, int rows, int columns )
{
   QVm.fill(  0, rows );            // Initialize the pointers vector
   QVd.fill( 0., rows * columns );  // Initialize the data elements vector
   double*  vd = QVd.data();        // Point to the beginning of data

   for ( int ii = 0; ii < rows; ii++ )
   {  // Construct the rows of the matrix
      QVm[ ii ] = vd;               // Store a row pointer
      vd       += columns;          // Bump the pointer to the next row
   }

   return QVm.data();               // Return the pointer to the pointers array
}

// Compute the columns x columns square matrix product of
//   a matrix-transpose and the matrix.
// Only the lower triangle of the product is filled,
//   since that is all that is required in Cholesky decomposition.
void US_Matrix::tmm( double** AA, double** CC, int rows, int columns )
{
   QVector< double > ATvec( rows );
   double* ATrow = ATvec.data();            // Array for a transpose row

   for ( int ii = 0; ii < columns; ii++ )
   {  // Loop for A' rows, A columns and P columns
      double dotp    = 0.0;

      // Dot product of the A' row with itself  (output diagonal point)
      //  Also, save this A' row for use below
      for ( int kk = 0; kk < rows; kk++ )
      { 
         double aval = AA[ kk ][ ii ];
         dotp       += sq( aval );
         ATrow[ kk ] = aval;
      }

      CC[ ii ][ ii ] = dotp;                // Store product on diagonal

      for ( int jj = 0; jj < ii; jj++ )
      {  // Loop for P rows (lower triangle)
         dotp        = 0.0;

         // Accumulate dot product of columns (A' row, A column)
         for ( int kk = 0; kk < rows; kk++ )
            dotp    += ( ATrow[ kk ] * AA[ kk ][ jj ] );

         CC[ ii ][ jj ] = dotp;             // Store product for column of row
      }
   }
}

// Compute product of a matrix-transpose and the matrix, with optional full fill
void US_Matrix::tmm( double** AA, double** CC, int rows, int columns,
      bool fill )
{
   // Calculate A-transpose times A, computing just the lower triangle
   tmm( AA, CC, rows, columns );

   if ( fill )
   {  // Duplicate values into the upper triangle
      for ( int ii = 0; ii < columns - 1; ii++ )
         for ( int jj = ii + 1; jj < columns; jj++ )
            CC[ ii ][ jj ] = CC[ jj ][ ii ];
   }
}

// Compute the vector product of a matrix and a vector
void US_Matrix::mvv( double** AA, double* bb, double* cc,
      int rows, int columns )
{
   for ( int ii = 0; ii < rows; ii++ )
   {
      double dotp = 0.0;

      for ( int jj = 0; jj < columns; jj++ )
         dotp += ( AA[ ii ][ jj ] * bb[ jj ] );

      cc[ ii ]  = dotp;
   }
}

// Compute the vector product of a matrix-transpose and a vector
void US_Matrix::tvv( double** AA, double* bb, double* cc,
      int rows, int columns )
{
   for ( int jj = 0; jj < columns; jj++ )
   {
      double dotp = 0.0;

      for ( int ii = 0; ii < rows; ii++ )
         dotp += ( AA[ ii ][ jj ] * bb[ ii ] );

      cc[ jj ]  = dotp;
   }
}

// Compute a matrix product of two matrices
void US_Matrix::mmm( double** AA, double** BB, double** CC,
      int rows, int size, int columns )
{
   for ( int ii = 0; ii < rows; ii++ )
   {
      for ( int jj = 0; jj < columns; jj++ )
      {
         double dotp = 0.0;

         for ( int kk = 0; kk < size; kk++ )
            dotp    += ( AA[ ii ][ kk ] * BB[ kk ][ jj ] );

         CC[ ii ][ jj ] = dotp;
      }
   }
}

// Calculate the matrix sum of two matrices of the same dimensions
void US_Matrix::msum( double** AA, double** BB, double** CC,
      int rows, int columns )
{
   for ( int ii = 0; ii < rows; ii++ )
      for ( int jj = 0; jj < columns; jj++ )
         CC[ ii ][ jj ] = AA[ ii ][ jj ] + BB[ ii ][ jj ];
}

// Calculate the vector sum of two vectors of the same size
void US_Matrix::vsum( double* aa, double* bb, double* cc, int size )
{
   for ( int ii = 0; ii < size; ii++ )
      cc[ ii ] = aa[ ii ] + bb[ ii ];
}

// Fill a square matrix to make it an identity matrix
void US_Matrix::mident( double** CC, int size )
{
   for ( int ii = 0; ii < size; ii++ )
      for ( int jj = 0; jj < size; jj++ )
         CC[ ii ][ jj ] = ( ii != jj ? 0.0 : 1.0 );
}

// Copy contents from one matrix to another matrix of the same dimensions
void US_Matrix::mcopy( double** AA, double** CC, int rows, int columns )
{
   for ( int ii = 0; ii < rows; ii++ )
      for ( int jj = 0; jj < columns; jj++ )
         CC[ ii ][ jj ] = AA[ ii ][ jj ];
}

// Copy contents from one vector to another vector of the same size
void US_Matrix::vcopy( double* aa, double* cc, int size )
{
   for ( int ii = 0; ii < size; ii++ )
      cc[ ii ] = aa[ ii ];
}

// Add a scalar value to the diagonal of a square matrix
void US_Matrix::add_diag( double** CC, double ss, int size )
{
   for ( int ii = 0; ii < size; ii++ )
      CC[ ii ][ ii ] += ss;
}

// Add a scalar value to all the elements of a matrix
void US_Matrix::add( double** CC, double ss, int rows, int columns )
{
   for ( int ii = 0; ii < rows; ii++ )
      for ( int jj = 0; jj < columns; jj++ )
         CC[ ii ][ jj ] += ss;
}

// Scale all the elements of a matrix by a scalar value
void US_Matrix::scale( double** CC, double ss, int rows, int columns )
{
   for ( int ii = 0; ii < rows; ii++ )
      for ( int jj = 0; jj < columns; jj++ )
         CC[ ii ][ jj ] *= ss;
}

// Compute the dot product of two vectors
double US_Matrix::dotproduct( double* aa, double* bb, int size )
{
   double dotp = 0.0;

   for ( int ii = 0; ii < size; ii++ )
      dotp += ( aa[ ii ] * bb[ ii ] );

   return dotp;
}

// Compute the dot product of a vector with itself
double US_Matrix::dotproduct( double* aa, int size )
{
   double dotp = 0.0;

   for ( int ii = 0; ii < size; ii++ )
      dotp += sq( aa[ ii ] );

   return dotp;
}

// Print a vector
void US_Matrix::print_vector( double* v, int n )
{
   QString s;
   QString t;
   for ( int i = 0; i < n; i++ ) s += t.sprintf( "%.15f ", v[ i ] );

   qDebug() << s;
}

// Print a matrix
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
   QVector< double > vscaling( n );
   double* scaling = vscaling.data();

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
   QVector< int > vindex( n );
   int* index  = vindex.data();
   
   LU_Decomposition ( A, index, parity, n );
   LU_BackSubstitute( A, b, index, n );
}

