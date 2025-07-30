//! \file us_matrix.h
#ifndef US_MATRIX_H
#define US_MATRIX_H

#include <QVector>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "us_extern.h"

/*! A class of general purpose static matrix functions. The elements of all
 *  matrices and vectors are doubles. The matrix arguments are arrays of
 *  pointers to arrays of doubles (rows). The vector arguments are arrays
 *  of doubles.
 *  \brief A set of general purpose matrix functions. All functions are static.
 */
struct dpairs
{
   int columnlocation;
   double locationvalue;
};

class US_UTIL_EXTERN ata_d_thr_t : public QThread
{
 public:
  ata_d_thr_t(int);
  void ata_d_thr_setup(unsigned int,
             unsigned int,
             unsigned int,
             double ***,
             QVector <QVector <dpairs> > *);

  void ata_d_thr_shutdown();
  void ata_d_thr_wait();
  virtual void run();

 private:
  unsigned int columns;
  unsigned int c_start;
  unsigned int c_end;
  double ***product;
  QVector <QVector <dpairs> > *dataarray;

  int thread;
  unsigned int i;
  unsigned int j;
  QMutex work_mutex;
  int work_to_do;
  QWaitCondition cond_work_to_do;
  int work_done;
  QWaitCondition cond_work_done;
  int work_to_do_waiters;
  int work_done_waiters;
};

class US_UTIL_EXTERN US_Matrix
{
public:

   /*! \brief Function to solve general linear least squares problem:
       \param c returns the coefficients for best least-squares fit
       \param x contains the x values of the raw data
       \param y contains the y values of the raw data
       \param N contains the dimension of x and y
       \param order is the order of the function to be fitted
   */
   static bool lsfit( double*, double*, double*, int, int );
 
   /*! This method factors the n by n symmetric positive definite matrix A
       as LL(T) where L is a lower triangular matrix. The method assumes that
       at least the lower triangle of A is filled on entry.
       On exit, the lower triangle of A has been replaced by L.
       \brief Function to factor an n x n symmetric positive definite matrix.
       \param a Input symmetric positive definite matrix A; lower triangle
                replaced by L on exit.
       \param n Dimensions of A.
   */
   static bool Cholesky_Decomposition( double**, int );

   /*! Cholesky_SolveSystem expects a Cholesky-decomposed L-matrix (n x n)
       with the lower diagonal filled, and the right hand side "b".
       Using forward and backward substitution, the right hand side "b"
       is replaced by the solution vector "x".
       \brief Function to solve a Cholesky-decomposed L-matrix.
       \param L Cholesky-decomposed L-matrix.
       \param b Right hand side.
       \param n Dimensions of L.
   */
   static bool Cholesky_SolveSystem( double**, double*, int );

   /*! Cholesky Invert: solve Ax=b using Cholesky decomposition:
    *   A * A(-1) = I ;  A = LL' ;  L (L' * A(-1) ) = I ;
    *   L * y = I (solve for y) ,  now L' * A(-1) = y  (solve for A(-1)) .
    *
    *  \brief Cholesky Invert to solve Ax=b by Cholesky decomposition.
    *  \param AA An nn x nn input matrix
    *  \param AI An nn x nn solved inverted matrix
    *  \param nn Number of rows and columns in the matrices
    *  \returns  A flag for whether a solution was found
    */
   static bool Cholesky_Invert   ( double**, double**, int );

   /*! Solve a set of linear equations. A is a square matrix of coefficients.
       b is the right hand side. b is replaced by solution.
       Target is replaced by its LU decomposition.
       \brief Solve a set of linear equations.
       \param A Square matrix of coefficients.
       \param b Right hand side on input and solution on output.
       \param n Dimension of matrix sides and vectors.
   */
   static void LU_SolveSystem   ( double**, double*&, int       );

   /*! Performs LU Decomposition on a matrix. This routine must be given
       an array to mark the row permutations and a flag to mark whether
       the number of permutations was even or odd.
       Reference: Numerical Recipes in C.
       \brief LU Decomposition of a matrix.
       \param matrix The matrix to decompose.
       \param index An index array to mark row permutations.
       \param parity Flag of even number of permutations.
       \param n Dimension of matrix sides.
   */
   static void LU_Decomposition ( double**, int*,     bool, int );

   /*! Do the backsubstitution on matrix A which is the LU decomposition
       of the original matrix. b is the n x 1 right hand side vector.
       b is replaced by the solution. index is the array that marks the row
       permutations.
       \brief Back substitution on a matrix which is the LU decomposition
              of the original matrix.
       \param A Input decomposed and output back-substituted matrix.
       \param b Right hand side vector on input and solution on output.
       \param index Array to be used to mark row permutations.
       \param n Dimension of sides.
   */
   static void LU_BackSubstitute( double**, double*&, int*, int );

   /*! Construct and initialize a matrix array of double pointers from
    *  a QVector of double pointers and a QVector of doubles holding the
    *  (rows times columns) contiguous matrix elements, initialized to zero.
    *  \brief Create a matrix array from QVectors
    *  \param QVm     The QVector from which the matrix array is constructed
    *  \param QVd     The QVector for holding contiguous data elements
    *  \param rows    The number of rows for the matrix
    *  \param columns The number of columns for the matrix
    *  \returns       The double** matrix array
    */
   static double** construct( QVector< double* >&, QVector< double >&,
         int, int );

   /*! Calculate the (columns x columns) square matrix product of a
    *  matrix transpose and the matrix. Only the lower triangle is computed.
    *  \brief Calculates A' * A ,  A-transpose times A.
    *  \param AA      The A matrix operand.
    *  \param CC      The C square matrix product of A-tranpose and A.
    *  \param rows    The number of A rows
    *  \param columns The number of A columns (and output C size)
    */
   static void tmm( double**, double**, int, int );

   /*! Calculate the (columns x columns) square matrix product of a matrix
    *  transpose and the matrix. The entire matrix may optionally be filled.
    *  \brief Calculates A' * A:  A-transpose times A.
    *  \param AA      The A matrix operand.
    *  \param CC      The C square matrix product of A-tranpose and A.
    *  \param rows    The number of A rows
    *  \param columns The number of A columns (and output C size)
    *  \param fill    A flag for whether to fill in the upper triangle
    */
   static void tmm( double**, double**, int, int, bool );

   /*! Calculate the (rows size) product of a (rows x columns) matrix
    *  and a (columns size) vector.
    *  \brief Calculates A * b:  a vector product whose size is A's rows
    *  \param AA      The A matrix operand  (rows x columns)
    *  \param bb      The b vector operator (columns x 1)
    *  \param cc      The c vector product  (rows x 1)
    *  \param rows    The number of rows in the matrix (output vector elements)
    *  \param columns The number of columns in the matrix (operator vector size)
    */
   static void mvv( double**, double*, double*, int, int );

   /*! Calculate the (columns size) product of a (columns x rows)
    *  matrix-tranpose and a (rows size) vector.
    *  \brief Calculates A' * b:  a vector product whose size is A's columns
    *  \param AA      The A matrix whose transpose is the operand
    *  \param bb      The b vector operator (rows x 1)
    *  \param cc      The c vector product  (columns x 1)
    *  \param rows    The number of matrix rows (operator vector size)
    *  \param columns The number of matrix columns (product vector size)
    */
   static void tvv( double**, double*, double*, int, int );

   /*! Calculate the matrix product of two matrices, the rows by columns
    *  product of a (rows x size) matrix and a (size x columns) matrix.
    *  \brief Calculates A * B:  a matrix product of A-rows by B-columns
    *  \param AA      The A matrix operand  (rows x size)
    *  \param BB      The B matrix operator (size x columns)
    *  \param CC      The C matrix product  (rows x columns)
    *  \param rows    The number of rows in A and C
    *  \param size    The number of columns in A and rows in B
    *  \param columns The number of columns in B and C
    */
   static void mmm( double**, double**, double**, int, int, int );

   /*! Calculate the matrix sum of two matrices of the same dimensions.
    *  \brief Calculate A + B:  a matrix sum with all matrices rows x columns.
    *  \param AA      The A matrix operand  (rows x columns)
    *  \param BB      The B matrix operator (rows x columns)
    *  \param CC      The C matrix sum      (rows x columns)
    *  \param rows    The number of rows in all matrices
    *  \param columns The number of columns in all matrices
    */
   static void msum( double**, double**, double**, int, int );

   /*! Calculate the vector sum of two vectors of the same dimensions. The
    *  destination sum vector may be the same as the operand.
    *  \brief Calculate a + b:  a vector sum with all vectors the same size.
    *  \param aa      The "a" vector operand
    *  \param bb      The "b" vector operator
    *  \param cc      The "c" vector sum (may be the same as "a")
    *  \param size    The size of all vectors
    */
   static void vsum( double*, double*, double*, int );

   /*! Fill a square matrix so that it is the identity matrix.
    *  \brief Fills matrix C:  Cij = ( i != j  ? 0.0 : 1.0 )
    *  \param CC      The matrix product
    *  \param size    The number of rows and columns in the matrix
    */
   static void mident( double**, int );

   /*! Copy the contents of one matrix to another of the same size
    *  \brief Matrix copy:  C = A
    *  \param AA      The source A matrix 
    *  \param CC      The destination C matrix copy
    *  \param rows    The number of rows in A and C
    *  \param columns The number of columns in A and C
    */
   static void mcopy( double**, double**, int, int );

   /*! Copy the contents of one vector to another of the same size
    *  \brief Vector copy:  c = a
    *  \param aa      The source "a" vector 
    *  \param cc      The destination "c" vector copy
    *  \param size    The size of the vectors
    */
   static void vcopy( double*, double*, int );

   /*! Add a scalar to the diagonal of a square matrix
    *  \brief Diagonal add:  Cii = Cii + s
    *  \param CC      The source,destination matrix
    *  \param ss      The scalar to add to the diagonal
    *  \param size    The size (rows, columns) of the matrix
    */
   static void add_diag( double**, double, int );

   /*! Add a scalar to all elements of a matrix
    *  \brief Matrix-scalar-add:  Cij += s
    *  \param CC      The source,destination matrix
    *  \param ss      The scalar to add to all elements
    *  \param rows    The number of rows in the matrix
    *  \param columns The number of columns in the matrix
    */
   static void add( double**, double, int, int );

   /*! Scale all matrix elements by a value
    *  \brief Matrix-scalar-multiply:  Cij *= s
    *  \param CC      The source,destination matrix
    *  \param ss      The scalar with which to multiply elements
    *  \param rows    The number of rows in the matrix
    *  \param columns The number of columns in the matrix
    */
   static void scale( double**, double, int, int );

   /*! Compute the dot product of two vectors
    *  \brief Vector dot product:  p = sum( a[i] * b[i] )
    *  \param aa      The operand "a" vector 
    *  \param bb      The operator "b" vector 
    *  \param size    The size of the vectors
    *  \returns       Dot product of vectors
    */
   static double dotproduct( double*, double*, int );

   /*! Compute the dot product of a vector with itself
    *  \brief Vector dot product:  p = sum( a[i] * a[i] )
    *  \param aa      The operand "a" vector 
    *  \param size    The size of the vector
    *  \returns       Dot product of vector with itself
    */
   static double dotproduct( double*, int );
	static void vvt(float ***, float **, float **, int);	
	static void mmv(float **, double **, float ***, int, int);
	static void mmv(float **, float **, float ***, int, int);
	static void mmv(float **, float **, double ***, int, int);
	static void mmv(float **, double **, double ***, int, int);
	static float dotproduct(float **, float **, int);
	static void calc_A_transpose_A(double*** /*original matrix*/, double*** /*product matrix*/,
         unsigned int /*rows*/, unsigned int /*columns*/, unsigned int /*threads*/);
	static void calc_A_transpose_A(float*** /*original matrix*/, float*** /*product matrix*/,
unsigned int /*rows*/, unsigned int /*columns*/);

private:
   static void print_matrix( double**, int, int );
   static void print_vector( double* , int );
};


#endif

#ifdef NEVER

/*
#include <iostream>
#include <math.h>
#include <qstring.h>
#include <qmessagebox.h>
#include "us_math.h"
#include "us_thread.h"
*/
struct fpairs
{
   int columnlocation;
   float locationvalue;
};

struct dpairs
{
   int columnlocation;
   double locationvalue;
};



// multiply tri-diagonal matrix with vector
void m3vm(double ***, double **, unsigned int);
void m3vm(float ***, float **, unsigned int);
void m3vm(float ***, double **, unsigned int);

#ifdef ADOLC
void m3vm(float ***, adouble **, unsigned int);
void m3vm(double ***, adouble **, unsigned int);
void m3vm(adouble ***, adouble **, unsigned int);
void m3vm_a(adouble ***, adouble **, unsigned int, float);
void m3vm_b(float ***, adouble ***, adouble **, unsigned int, float);
#endif
void m3vm_b(float ***, float ***, float **, unsigned int, float);
void m3vm_b(double ***, double ***, double **, unsigned int, double);
void m2vm(double ***, double **, int);
void m2vm(float ***, float **, int);

#ifdef ADOLC
void m2vm(float ***, adouble **, int);
void m2vm(double ***, adouble **, int);
#endif
// multiply matrix with vector

void mmv(float **, double **, float ***, int, int);
void mmv(float **, float **, float ***, int, int);
void mmv(float **, float **, double ***, int, int);
void mmv(float **, double **, double ***, int, int);

void ldu(double ***, double **, unsigned int, unsigned int, bool);
void ldu(float ***, float **, unsigned int, unsigned int, bool);

#ifdef ADOLC
void ldu(adouble ***, adouble **, unsigned int, unsigned int, bool);
void ldu(float ***, adouble **, unsigned int, unsigned int, bool);
#endif

float dotproduct(float **, float **, int);
void vvt(float ***, float **, float **, int);
void LU_Decomposition(double **, int *, bool, int);
void LU_SolveSystem(double **, double **, int);
void LU_Invert(double **, double **, int);
void LU_BackSubstitute(double **, double **, int *, int);
bool Cholesky_Decomposition(double **, int);
bool Cholesky_SolveSystem(double **, double *, int);
bool Cholesky_Invert(double **, double **, int);
void generalLeastSquares(float **, unsigned int, unsigned int, float *, float **);
void print_matrix( double **, int, int);
void print_vector( double **, unsigned int, const char *);
void calc_A_transpose_A(double*** /*original matrix*/, double*** /*product matrix*/, 
         unsigned int /*rows*/, unsigned int /*columns*/, unsigned int /*threads*/);
void calc_A_transpose_A(float*** /*original matrix*/, float*** /*product matrix*/, 
unsigned int /*rows*/, unsigned int /*columns*/);
void calc_matrix_times_matrix(double ***, double ***, double ***, unsigned int, unsigned int);
void calc_matrix_times_matrix(float ***, float ***, float ***, unsigned int, unsigned int);
void calc_matrix_times_vector(double ***, double **, double **, unsigned int, unsigned int);
void calc_matrix_times_vector(float ***, float **, float **, unsigned int, unsigned int);
double calc_vector_transpose_times_vector(unsigned int /*rows*/, double ** /*vector1*/, double ** /*vector2*/);
float calc_vector_transpose_times_vector(unsigned int /*rows*/, float ** /*vector1*/, float ** /*vector2*/);

class US_lsfit
{
//
// Class to solve general linear least squares problem:
//
//  MIN chi-square
//
public:

   US_lsfit(double *coeff, double *x, double *y, int n, int ord, bool mesg);
   
//
// coeff returns the coefficients for best least-squares fit
// x contains the x values of the raw data
// y contains the y values of the raw data
// n contains the dimension of x and y
// ord is the order of the function to be fitted
//
   ~US_lsfit();

private:
   
   double **A, *b, *c, *xval, *yval;
   int order, numpoints, i, j, k;
   void calc_coeff_polynomial();
};


void conc_dep_s(float, double **, double **, double ***, float temp1, unsigned int points);
void conc_dep_s(float, float **, float **, float ***, float temp1, unsigned int points);
#ifdef ADOLC
void conc_dep_s(adouble, adouble **, adouble **, double ***, adouble, unsigned int points);
void conc_dep_s(adouble, adouble **, adouble **, float ***, adouble, unsigned int points);
#endif
void conc_dep_d(float, double **, double **, double ***, double ***, double ***,
                float, unsigned int, double **, double **, double **);
void conc_dep_d(float, float **, float **, float ***, float ***, float ***,
                float, unsigned int, float **, float **, float **);
#ifdef ADOLC
void conc_dep_d(adouble, adouble **, adouble **, float ***, float ***, float ***,
                adouble, unsigned int, adouble **, adouble **, adouble **);
void conc_dep_d(adouble, adouble **, adouble **, double ***, double ***, double ***,
                adouble, unsigned int, adouble **, adouble **, adouble **);
#endif

#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>

class US_UTIL_EXTERN ata_d_thr_t : public QThread
{
 public:
  ata_d_thr_t(int);
  void ata_d_thr_setup(unsigned int,
             unsigned int,
             unsigned int,
             double ***,
             vector <vector <dpairs> > *);

  void ata_d_thr_shutdown();
  void ata_d_thr_wait();
  virtual void run();

 private:
  unsigned int columns;
  unsigned int c_start;
  unsigned int c_end;
  double ***product;
  vector <vector <dpairs> > *dataarray;

  int thread;
  unsigned int i;
  unsigned int j;
  QMutex work_mutex;
  int work_to_do;
  QWaitCondition cond_work_to_do;
  int work_done;
  QWaitCondition cond_work_done;
  int work_to_do_waiters;
  int work_done_waiters;
};

#endif

