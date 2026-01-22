//! \file us_matrix.cpp
#include "us_matrix.h"
#include "us_math2.h"

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
   for ( int i = 0; i < n; i++ )
   {
      s += QString::asprintf( "%.15f ", v[ i ] );
   }
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

void US_Matrix::mmv(float **result, double **vector, float ***matrix, int row, int column)
{
   int i, j;
   for (i=0; i<row; i++)
   {
      (*result)[i] = 0;
      for (j=0; j<column; j++)
      {
         (*result)[i] += (*matrix)[i][j] * (*vector)[j];
      }
   }
}

void US_Matrix::mmv(float **result, double **vector, double ***matrix, int row, int column)
{
   int i, j;
   for (i=0; i<row; i++)
   {
      (*result)[i] = 0;
      for (j=0; j<column; j++)
      {
         (*result)[i] += (*matrix)[i][j] * (*vector)[j];
      }
   }
}

void US_Matrix::mmv(float **result, float **vector, double ***matrix, int row, int column)
{
   int i, j;
   for (i=0; i<row; i++)
   {
      (*result)[i] = 0;
      for (j=0; j<column; j++)
      {
         (*result)[i] += (*matrix)[i][j] * (*vector)[j];
      }
   }
}

void US_Matrix::mmv(float **result, float **vector, float ***matrix, int row, int column)
{
   int i, j;
   for (i=0; i<row; i++)
   {
      (*result)[i] = 0;
      for (j=0; j<column; j++)
      {
         (*result)[i] += (*matrix)[i][j] * (*vector)[j];
      }
   }
}

float US_Matrix::dotproduct(float **v1, float **v2, int size)
{
   float result=0;
   for (int i=0; i<size; i++)
   {
      result += (*v1)[i] * (*v2)[i];
   }
   return(result);
}

void US_Matrix::vvt(float ***result, float **v1, float **v2, int size)
{
   for (int i=0; i<size; i++)
   {
      for (int j=0; j<size; j++)
      {
         (*result)[i][j] = (*v1)[i] * (*v2)[j];
      }
   }
}

/*
  #ifdef THREAD

  void US_Matrix::calc_A_transpose_A(double ***A, double ***product, unsigned int rows, unsigned int columns, bool flag)
  {
  CATA_D_Thread *dt;
  dt = new CATA_D_Thread [columns];
  for ( unsigned int i=0; i<columns; i++)
  {   
  dt[i].i = i;
  dt[i].product = product;
  dt[i].A = A;
  dt[i].end_k = rows;
      
  if (flag) // This version completes the entire matrix:
  {   
  dt[i].end_j = columns;
  }
  else // This version completes only the lower triangular matrix
  {    //  that's all what's needed for the Cholesky decomposition:
  dt[i].end_j = i+1;
  }
  dt[i].start();
  }
  unsigned int count = 0;
  while(count < columns)
  {
  if(dt[count].wait())
  {
  count ++;
  }
  }
  }

  void US_Matrix::calc_A_transpose_A(double ***A, double ***product, unsigned int rows, unsigned int columns, bool flag)
  {
  CATA_D_Thread *dt;
  dt = new CATA_D_Thread [2];
  int f = 0;
  for ( unsigned int i=0; i<columns; i++)
  {   
  dt[f].i = i;
  dt[f].product = product;
  dt[f].A = A;
  dt[f].end_k = rows;
      
  if (flag) // This version completes the entire matrix:
  {   
  dt[f].end_j = columns;
  }
  else // This version completes only the lower triangular matrix
  {    //  that's all what's needed for the Cholesky decomposition:
  dt[f].end_j = i+1;
  }
  dt[f].start();
  if (f == 0)
  {
  f = 1;
  }
  else
  {
  f = 0;
  }
  dt[f].wait();
  }
  }

  #else
*/
ata_d_thr_t::ata_d_thr_t(int a_thread) : QThread() {
   thread = a_thread;
   work_to_do = 0;
   work_done = 1;
   work_to_do_waiters = 0;
   work_done_waiters = 0;
}

void ata_d_thr_t::ata_d_thr_setup(unsigned int a_columns,
                                  unsigned int a_c_start,
                                  unsigned int a_c_end,
                                  double ***a_product,
                                  QVector <QVector <dpairs> > *a_dataarray
                                  ) {
   /* this starts up a new work load for the thread */
   columns = a_columns;
   c_start = a_c_start;
   c_end = a_c_end;
   product = a_product;
   dataarray = a_dataarray;

   work_mutex.lock();
   work_to_do = 1;
   work_done = 0;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();

   //  cerr << "thread " << thread << " has new work to do\n";
}

void ata_d_thr_t::ata_d_thr_shutdown() {
   /* this signals the thread to exit the run method */
   work_mutex.lock();
   work_to_do = -1;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();

   //  cerr << "thread " << thread << " shutdown requested\n";
}

void ata_d_thr_t::ata_d_thr_wait() {
   /* this is for the master thread to wait until the work is done */
   work_mutex.lock();

   //  cerr << "thread " << thread << " has a waiter\n";

   while(!work_done) {
      cond_work_done.wait(&work_mutex);
   }
   work_done = 0;
   work_mutex.unlock();

   //  cerr << "thread " << thread << " waiter released\n";
}

void ata_d_thr_t::run() {
   while(1) {
      work_mutex.lock();
      //    cerr << "thread " << thread << " waiting for work\n";
      work_to_do_waiters++;
      while(!work_to_do) {
         cond_work_to_do.wait(&work_mutex);
      }
      if(work_to_do == -1) {
         //      cerr << "thread " << thread << " shutting down\n";
         work_mutex.unlock();
         return;
      }

      work_to_do_waiters = 0;
      work_mutex.unlock();
      //    cerr << "thread " << thread << " starting work\n";

      QVector <dpairs> m, n;
      unsigned int i, j, k, l;

      for (i = c_start; i < c_end; i++) {
         for (j = 0; j < columns; j++) {
            if (i <= j) {
               double sum=0;
               k=0;
               l=0;
               m.clear();
               n.clear();
               m=(*dataarray)[i];
               n=(*dataarray)[j];
               while((k<(unsigned int)m.size()) && (l<(unsigned int)n.size())) {
                  if (m[k].columnlocation == n[l].columnlocation) {
                     sum += m[k].locationvalue * n[l].locationvalue;
                     k++;
                     l++;
                  } else {
                     if (m[k].columnlocation < n[l].columnlocation)  {
                        k++;
                     } else {
                        l++;
                     }
                  }
                  (*product)[i][j]=sum;
                  (*product)[j][i]=sum;
               }
            }
         }
      }

      //    cerr << "thread " << thread << " finished work\n";
      work_mutex.lock();
      work_done = 1;
      work_to_do = 0;
      work_mutex.unlock();
      cond_work_done.wakeOne();
   }
}

void US_Matrix::calc_A_transpose_A(double ***A, double ***product, unsigned int rows, unsigned int columns, unsigned int threads)
{
   QVector <dpairs> data_pairs, m, n;
   QVector <QVector <dpairs> > dataarray;
   dpairs temp_pair;
   dataarray.clear();
   data_pairs.clear();
   m.clear();
   n.clear();
   for (unsigned int i=0; i<columns; i++)
   {
      for (unsigned int j=0; j<rows; j++)
      {
         if((*A)[j][i]!=0)
         {
            temp_pair.locationvalue = (*A)[j][i];
            temp_pair.columnlocation = j;
            data_pairs.push_back(temp_pair);
         }
      }
      dataarray.push_back(data_pairs);
      data_pairs.clear();
   }
   //   unsigned int threads = 4; // USglobal->config_list.numThreads
   //   printf("calc ata threads %d\n", threads);
   if(threads > 1) {
     
      // create threads
      unsigned int j;
      QVector< ata_d_thr_t* > ata_d_thr_threads( static_cast<int>(threads) );
      //QVector < ata_d_thr_t* > ata_d_thr_threads( threads );
     
      for(j = 0; j < threads; j++) {
         ata_d_thr_threads[static_cast<int>(j)] = new ata_d_thr_t(j);
         ata_d_thr_threads[static_cast<int>(j)]->start();
      }
     
      unsigned int c_start = 0;
      unsigned int c_startsq;
      unsigned int c_end;
      float c_per2 = columns * columns / threads;
      //     printf("cols %u threads %u c_per2 %u\n", columns, threads, c_per2);
      for(j = 0; j < threads; j++) {
         c_startsq = (columns - c_start) * (columns - c_start);
         if(j + 1 == threads) {
            c_end = columns;
         } else {
            if(c_startsq <= c_per2) {
               c_end = columns;
            } else {
               c_end = (columns - (int)sqrt(1.0 * c_startsq - c_per2));
               if(c_end > columns) {
                  c_end = columns;
               }
            }
         }
         //       cout << "thread " << j << " c range " << c_start << " - " << c_end << endl;
         ata_d_thr_threads[static_cast<int>(j)]->ata_d_thr_setup(columns, c_start, c_end, product, &dataarray);
         c_start = c_end;
      }
     
      for(j = 0; j < threads; j++) {
         ata_d_thr_threads[static_cast<int>(j)]->ata_d_thr_wait();
      }
     
      // destroy
      for(j = 0; j < threads; j++) {
         ata_d_thr_threads[static_cast<int>(j)]->ata_d_thr_shutdown();
      }
     
      for(j = 0; j < threads; j++) {
         ata_d_thr_threads[static_cast<int>(j)]->wait();
      }
     
      for(j = 0; j < threads; j++) {
         delete ata_d_thr_threads[static_cast<int>(j)];
      }
     
   } else {
     
      for (unsigned int i=0; i<columns; i++)
      {
         for (unsigned int j=0; j<columns; j++)
         {
            if (i<=j)
            {
               double sum=0;
               unsigned int k=0;
               unsigned int l=0;
               m.clear();
               n.clear();
               m=dataarray[i];
               n=dataarray[j];
               while((k<(unsigned int)m.size()) && (l<(unsigned int)n.size()))
               {
                  if (m[k].columnlocation == n[l].columnlocation) 
                  {
                     sum += m[k].locationvalue * n[l].locationvalue;
                     k++;
                     l++;
                  }
                  else if (m[k].columnlocation < n[l].columnlocation)
                  {
                     k++;
                  }
                  else
                  {
                     l++;
                  }
                  (*product)[i][j]=sum;
                  (*product)[j][i]=sum;
               }
            }
         }
      }
   }
   /*
   // This version completes only the lower triangular matrix - all that's needed for the Cholesky decomposition:
   for (unsigned int i=0; i<columns; i++)
   {
   for (unsigned int j=0; j<=i; j++)
   {
   (*product)[i][j] = 0.0;
   for (unsigned int k=0; k<rows; k++)
   {
   (*product)[i][j] += (*A)[k][i] * (*A)[k][j];
   }
   }
   }
   */
}

/*
  #endif

  #ifdef THREAD

  void US_Matrix::calc_A_transpose_A(float ***A, float ***product, unsigned int rows, unsigned int columns, bool flag)
  {
  CATA_F_Thread *ft;
  ft = new CATA_F_Thread [columns];
  //cout<<columns<<endl;
  for ( unsigned int i=0; i<columns; i++)
  {   
  ft[i].product = product;
  ft[i].A = A;
  ft[i].i = i;   
  ft[i].end_k = rows;
      
  if (flag)
  {   
  // This version completes the entire matrix:
  ft[i].end_j = columns;
  }
  else
  {
  // This version completes only the lower triangular matrix - all that's needed for the Cholesky decomposition:
  ft[i].end_j = i+1;
  }
  ft[i].start();
  }
  unsigned int count = 0;
  while(count < columns)
  {
  if(ft[count].wait())
  {
  count ++;
  }
  }
  }


  void US_Matrix::calc_A_transpose_A(float ***A, float ***product, unsigned int rows, unsigned int columns, bool flag)
  {
  CATA_F_Thread *dt;
  dt = new CATA_F_Thread [2];
  int f = 0;
  for ( unsigned int i=0; i<columns; i++)
  {   
  dt[f].i = i;
  dt[f].product = product;
  dt[f].A = A;
  dt[f].end_k = rows;
      
  if (flag) // This version completes the entire matrix:
  {   
  dt[f].end_j = columns;
  }
  else // This version completes only the lower triangular matrix
  {    //  that's all what's needed for the Cholesky decomposition:
  dt[f].end_j = i+1;
  }
  dt[f].start();
  if (f == 0)
  {
  f = 1;
  }
  else
  {
  f = 0;
  }
  dt[f].wait();
  }
  }


  #else
*/
void US_Matrix::calc_A_transpose_A(float ***A, float ***product, unsigned int rows, unsigned int columns)
{
   QVector <dpairs> data_pairs, m, n;
   QVector <QVector <dpairs> > dataarray;
   dpairs temp_pair;
   dataarray.clear();
   data_pairs.clear();
   m.clear();
   n.clear();
   for (unsigned int i=0; i<columns; i++)
   {
      for (unsigned int j=0; j<rows; j++)
      {
         if((*A)[j][i]!=0)
         {
            temp_pair.locationvalue = (*A)[j][i];
            temp_pair.columnlocation = j;
            data_pairs.push_back(temp_pair);
         }
      }
      dataarray.push_back(data_pairs);
      data_pairs.clear();
   }
   for (unsigned int i=0; i<columns; i++)
   {
      for (unsigned int j=0; j<columns; j++)
      {
         if (i<=j)
         {
            double sum=0;
            unsigned int k=0;
            unsigned int l=0;
            m.clear();
            n.clear();
            m=dataarray[i];
            n=dataarray[j];
            while((k<(unsigned int)m.size()) && (l<(unsigned int)n.size()))
            {
               if (m[k].columnlocation == n[l].columnlocation) 
               {
                  sum += m[k].locationvalue * n[l].locationvalue;
                  k++;
                  l++;
               }
               else if (m[k].columnlocation < n[l].columnlocation)
               {
                  k++;
               }
               else
               {
                  l++;
               }
               (*product)[i][j] = (float) sum;
               (*product)[j][i] = (float) sum;
            }
         }
      }
   }
   /*
   // This version completes only the lower triangular matrix - all that's needed for the Cholesky decomposition:
   for (unsigned int i=0; i<columns; i++)
   {
   for (unsigned int j=0; j<=i; j++)
   {
   (*product)[i][j] = 0.0;
   for (unsigned int k=0; k<rows; k++)
   {
   (*product)[i][j] += (*A)[k][i] * (*A)[k][j];
   }
   }
   }
   */
}

//#endif
