#include "../include/us_matrix.h"
#include <cerrno>

void m3vm(double ***matrix, double **vector, unsigned int points)
   /*
     This function multiplies a tri-diagonal matrix of size [points x points]
     stored in a [points x 3] matrix by a vector of size [points] and returns
     the result in vector:
   */
{
   unsigned int i,j;
   double *result;
   result = new double [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m3vm\n";
      return;
   }
   result[0] = (*matrix)[0][1] * (*vector)[0] + (*matrix)[0][2] * (*vector)[1];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0.0;
      for (j=0; j<3; j++)
      {
         result[i] = result[i] + (*matrix)[i][j] * (*vector)[i+j-1];
      }
   }
   result[points-1] = (*matrix)[points-1][0] * (*vector)[points-2] +
      (*matrix)[points-1][1] * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}

void m3vm(float ***matrix, double **vector, unsigned int points)
   /*
     This function multiplies a tri-diagonal matrix of size [points x points]
     stored in a [points x 3] matrix by a vector of size [points] and returns
     the result in vector:
   */
{
   unsigned int i,j;
   double *result;
   result = new double [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m3vm\n";
      return;
   }
   result[0] = (*matrix)[0][1] * (*vector)[0] + (*matrix)[0][2] * (*vector)[1];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0.0;
      for (j=0; j<3; j++)
      {
         result[i] = result[i] + (*matrix)[i][j] * (*vector)[i+j-1];
      }
   }
   result[points-1] = (*matrix)[points-1][0] * (*vector)[points-2] +
      (*matrix)[points-1][1] * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}

#ifdef ADOLC
void m3vm(double ***matrix, adouble **vector, unsigned int points)
   /*
     This function multiplies a tri-diagonal matrix of size [points x points]
     stored in a [points x 3] matrix by a vector of size [points] and returns
     the result in vector:
   */
{
   unsigned int i,j;
   adouble *result;
   result = new adouble [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m3vm\n";
      return;
   }
   result[0] = (*matrix)[0][1] * (*vector)[0] + (*matrix)[0][2] * (*vector)[1];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0.0;
      for (j=0; j<3; j++)
      {
         result[i] = result[i] + (*matrix)[i][j] * (*vector)[i+j-1];
      }
   }
   result[points-1] = (*matrix)[points-1][0] * (*vector)[points-2] +
      (*matrix)[points-1][1] * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}

void m3vm(adouble ***matrix, adouble **vector, unsigned int points)
   /*
     This function multiplies a tri-diagonal matrix of size [points x points]
     stored in a [points x 3] matrix by a vector of size [points] and returns
     the result in vector:
   */
{
   unsigned int i,j;
   adouble *result;
   result = new adouble [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m3vm\n";
      return;
   }
   result[0] = (*matrix)[0][1] * (*vector)[0] + (*matrix)[0][2] * (*vector)[1];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0.0;
      for (j=0; j<3; j++)
      {
         result[i] = result[i] + (*matrix)[i][j] * (*vector)[i+j-1];
      }
   }
   result[points-1] = (*matrix)[points-1][0] * (*vector)[points-2] +
      (*matrix)[points-1][1] * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}


void m3vm(float ***matrix, adouble **vector, unsigned int points)
   /*
     This function multiplies a tri-diagonal matrix of size [points x points]
     stored in a [points x 3] matrix by a vector of size [points] and returns
     the result in vector:
   */
{
   unsigned int i,j;
   adouble *result;
   result = new adouble [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m3vm\n";
      return;
   }
   result[0] = (*matrix)[0][1] * (*vector)[0] + (*matrix)[0][2] * (*vector)[1];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0.0;
      for (j=0; j<3; j++)
      {
         result[i] = result[i] + (*matrix)[i][j] * (*vector)[i+j-1];
      }
   }
   result[points-1] = (*matrix)[points-1][0] * (*vector)[points-2] +
      (*matrix)[points-1][1] * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}
#endif
void m3vm(float ***matrix, float **vector, unsigned int points)
   /*
     This function multiplies a tri-diagonal matrix of size [points x points]
     stored in a [points x 3] matrix by a vector of size [points] and returns
     the result in vector:
   */
{
   unsigned int i,j;
   float *result;
   result = new float [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m3vm";
      return;
   }
   result[0] = (*matrix)[0][1] * (*vector)[0] + (*matrix)[0][2] * (*vector)[1];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0.0;
      for (j=0; j<3; j++)
      {
         result[i] = result[i] + (*matrix)[i][j] * (*vector)[i+j-1];
      }
   }
   result[points-1] = (*matrix)[points-1][0] * (*vector)[points-2] +
      (*matrix)[points-1][1] * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}
#ifdef ADOLC
void m3vm_a(adouble ***matrix, adouble **vector, unsigned int points, float constant)
   /*
     This function multiplies a tri-diagonal matrix of size [points x points]
     stored in a [points x 3] matrix by a vector of size [points] and returns
     the result in vector:
   */
{
   unsigned int i,j;
   adouble *result;
   result = new adouble [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m3vm\n";
      return;
   }
   result[0] = constant * (*matrix)[0][1] * (*vector)[0] + constant * (*matrix)[0][2] * (*vector)[1];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0.0;
      for (j=0; j<3; j++)
      {
         result[i] = result[i] + constant * (*matrix)[i][j] * (*vector)[i+j-1];
      }
   }
   result[points-1] = constant * (*matrix)[points-1][0] * (*vector)[points-2] +
      constant * (*matrix)[points-1][1] * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}

void m3vm_b(float ***b, adouble ***matrix, adouble **vector, unsigned int points, float constant)
   /*
     This function multiplies a tri-diagonal matrix of size [points x points]
     stored in a [points x 3] matrix by a vector of size [points] and returns
     the result in vector:
   */
{
   unsigned int i,j;
   adouble *result;
   result = new adouble [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m3vm\n";
      return;
   }
   result[0] = ((*b)[0][1] + constant * (*matrix)[0][1]) * (*vector)[0] + ((*b)[0][2] + constant * (*matrix)[0][2]) * (*vector)[1];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0.0;
      for (j=0; j<3; j++)
      {
         result[i] = result[i] + ((*b)[i][j] + constant * (*matrix)[i][j]) * (*vector)[i+j-1];
      }
   }
   result[points-1] = ((*b)[points-1][0] + constant * (*matrix)[points-1][0]) * (*vector)[points-2] +
      ((*b)[points-1][1] + constant * (*matrix)[points-1][1]) * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}
#endif
void m3vm_b(float ***b, float ***matrix,  float **vector, unsigned int points, float constant)
   /*
     This function multiplies a tri-diagonal matrix of size [points x points]
     stored in a [points x 3] matrix by a vector of size [points] and returns
     the result in vector:
   */
{
   unsigned int i,j;
   float *result;
   result = new float [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m3vm\n";
      return;
   }
   result[0] = ((*b)[0][1] + constant * (*matrix)[0][1]) * (*vector)[0] + ((*b)[0][2] + constant * (*matrix)[0][2]) * (*vector)[1];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0.0;
      for (j=0; j<3; j++)
      {
         result[i] = result[i] + ((*b)[i][j] + constant * (*matrix)[i][j]) * (*vector)[i+j-1];
      }
   }
   result[points-1] = ((*b)[points-1][0] + constant * (*matrix)[points-1][0]) * (*vector)[points-2] +
      ((*b)[points-1][1] + constant * (*matrix)[points-1][1]) * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}

void m3vm_b(double ***b, double ***matrix,  double **vector, unsigned int points, double constant)
   /*
     This function multiplies a tri-diagonal matrix of size [points x points]
     stored in a [points x 3] matrix by a vector of size [points] and returns
     the result in vector:
   */
{
   unsigned int i,j;
   double *result;
   result = new double [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m3vm\n";
      return;
   }
   result[0] = ((*b)[0][1] + constant * (*matrix)[0][1]) * (*vector)[0] + ((*b)[0][2] + constant * (*matrix)[0][2]) * (*vector)[1];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0.0;
      for (j=0; j<3; j++)
      {
         result[i] = result[i] + ((*b)[i][j] + constant * (*matrix)[i][j]) * (*vector)[i+j-1];
      }
   }
   result[points-1] = ((*b)[points-1][0] + constant * (*matrix)[points-1][0]) * (*vector)[points-2] +
      ((*b)[points-1][1] + constant * (*matrix)[points-1][1]) * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}

void m2vm(double ***matrix, double **vector, int points)
{
   int i,j;
   double *result;
   result = new double [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m2vm";
      return;
   }
   result[0] = (*matrix)[0][1] * (*vector)[0];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0;
      for (j=0; j<2; j++)
      {
         result[i] = result[i] + (*matrix)[i][j] * (*vector)[i+j-1];
      }
   }
   result[points-1] = (*matrix)[points-1][0] * (*vector)[points-2] +
      (*matrix)[points-1][1] * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}

void m2vm(float ***matrix, float **vector, int points)
{
   int i,j;
   float *result;
   result = new float [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m2vm";
      return;
   }
   result[0] = (*matrix)[0][1] * (*vector)[0];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0;
      for (j=0; j<2; j++)
      {
         result[i] = result[i] + (*matrix)[i][j] * (*vector)[i+j-1];
      }
   }
   result[points-1] = (*matrix)[points-1][0] * (*vector)[points-2] +
      (*matrix)[points-1][1] * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}
#ifdef ADOLC
void m2vm(double ***matrix, adouble **vector, int points)
{
   int i,j;
   adouble *result;
   result = new adouble [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m2vm";
      return;
   }
   result[0] = (*matrix)[0][1] * (*vector)[0];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0;
      for (j=0; j<2; j++)
      {
         result[i] = result[i] + (*matrix)[i][j] * (*vector)[i+j-1];
      }
   }
   result[points-1] = (*matrix)[points-1][0] * (*vector)[points-2] +
      (*matrix)[points-1][1] * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}

void m2vm(float ***matrix, adouble **vector, int points)
{
   int i,j;
   adouble *result;
   result = new adouble [points];
   if (result == NULL)
   {
      cout << "Not enough memory in m2vm";
      return;
   }
   result[0] = (*matrix)[0][1] * (*vector)[0];
   for (i=1; i<points-1; i++)
   {
      result[i] = 0;
      for (j=0; j<2; j++)
      {
         result[i] = result[i] + (*matrix)[i][j] * (*vector)[i+j-1];
      }
   }
   result[points-1] = (*matrix)[points-1][0] * (*vector)[points-2] +
      (*matrix)[points-1][1] * (*vector)[points-1];
   for (i=0; i<points; i++)
   {
      (*vector)[i] = result[i];
   }
   delete [] result;
}
#endif

void mmv(float **result, double **vector, float ***matrix, int row, int column)
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

void mmv(float **result, double **vector, double ***matrix, int row, int column)
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

void mmv(float **result, float **vector, double ***matrix, int row, int column)
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

void mmv(float **result, float **vector, float ***matrix, int row, int column)
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

float dotproduct(float **v1, float **v2, int size)
{
   float result=0;
   for (int i=0; i<size; i++)
   {
      result += (*v1)[i] * (*v2)[i];
   }
   return(result);
}


void vvt(float ***result, float **v1, float **v2, int size)
{
   for (int i=0; i<size; i++)
   {
      for (int j=0; j<size; j++)
      {
         (*result)[i][j] = (*v1)[i] * (*v2)[j];
      }
   }
}

void ldu(double ***matrix, double **vector, unsigned int rows, unsigned int columns, bool flag)
{
   unsigned int i, j, k, jbgn, jend, jrow=0, jcol=0, k1, k2, irow, ibd2, idiag;

   double fac;

   ibd2 = (columns-1)/2;
   idiag = ibd2 + 1;
   for (i=1; i < rows; i++)
   {
      jend = rows-i;
      if (jend > ibd2)
         jend = ibd2;
      for (j=1; j==jend; j++)
      {
         jrow = i+j;
         jcol = ibd2-j+1;
         if (flag)
         {
            fac = (*matrix)[jrow-1][jcol-1]/(*matrix)[i-1][idiag-1];
            (*matrix)[jrow-1][jcol-1] = fac;
            for (k=1; k==jend; k++)
            {
               k1 = idiag+k;
               k2 = jcol+k;
               (*matrix)[jrow-1][k2-1] = (*matrix)[jrow-1][k2-1] -
                  fac * (*matrix)[i-1][k1-1];
            }
         }
      }
      (*vector)[jrow-1] = (*vector)[jrow-1] -
         (*matrix)[jrow-1][jcol-1] * (*vector)[i-1];
   }
   (*vector)[rows-1] = (*vector)[rows-1]/(*matrix)[rows-1][idiag-1];
   jbgn = idiag+1;
   for (i=1; i<rows; i++)
   {
      irow = rows-i;
      jend = idiag+i;
      if (jend > columns)
      {
         jend=columns;
      }
      jrow = irow;
      for (j=jbgn-1; j<jend; j++)
      {
         jrow = jrow+1;
         (*vector)[irow-1] = (*vector)[irow-1] -
            (*matrix)[irow-1][j] * (*vector)[jrow-1];
      }
      (*vector)[irow-1] = (*vector)[irow-1]/(*matrix)[irow-1][idiag-1];
   }
}

void ldu(float ***matrix, float **vector, unsigned int rows, unsigned int columns, bool flag)
{
   unsigned int i, j, k, k1, k2, irow, ibd2, idiag, jbgn, jend, jrow=0, jcol=0;
   float fac;

   ibd2 = (columns-1)/2;
   idiag = ibd2 + 1;
   for (i=1; i < rows; i++)
   {
      jend = rows-i;
      if (jend > ibd2)
         jend = ibd2;
      for (j=1; j==jend; j++)
      {
         jrow = i+j;
         jcol = ibd2-j+1;
         if (flag)
         {
            fac = (*matrix)[jrow-1][jcol-1]/(*matrix)[i-1][idiag-1];
            (*matrix)[jrow-1][jcol-1] = fac;
            for (k=1; k==jend; k++)
            {
               k1 = idiag+k;
               k2 = jcol+k;
               (*matrix)[jrow-1][k2-1] = (*matrix)[jrow-1][k2-1] -
                  fac * (*matrix)[i-1][k1-1];
            }
         }
      }
      (*vector)[jrow-1] = (*vector)[jrow-1] -
         (*matrix)[jrow-1][jcol-1] * (*vector)[i-1];
   }
   (*vector)[rows-1] = (*vector)[rows-1]/(*matrix)[rows-1][idiag-1];
   jbgn = idiag+1;
   for (i=1; i<rows; i++)
   {
      irow = rows-i;
      jend = idiag+i;
      if (jend > columns)
      {
         jend=columns;
      }
      jrow = irow;
      for (j=jbgn-1; j<jend; j++)
      {
         jrow = jrow+1;
         (*vector)[irow-1] = (*vector)[irow-1] -
            (*matrix)[irow-1][j] * (*vector)[jrow-1];
      }
      (*vector)[irow-1] = (*vector)[irow-1]/(*matrix)[irow-1][idiag-1];
   }
}

#ifdef ADOLC
void ldu(adouble ***matrix, adouble **vector, unsigned int rows, unsigned int columns, bool flag)
{
   unsigned int i, j, k, jbgn, jend, jrow=0, jcol=0, k1, k2, irow, ibd2, idiag;

   adouble fac;

   ibd2 = (columns-1)/2;
   idiag = ibd2 + 1;
   for (i=1; i < rows; i++)
   {
      jend = rows-i;
      if (jend > ibd2)
         jend = ibd2;
      for (j=1; j==jend; j++)
      {
         jrow = i+j;
         jcol = ibd2-j+1;
         if (flag)
         {
            fac = (*matrix)[jrow-1][jcol-1]/(*matrix)[i-1][idiag-1];
            (*matrix)[jrow-1][jcol-1] = fac;
            for (k=1; k==jend; k++)
            {
               k1 = idiag+k;
               k2 = jcol+k;
               (*matrix)[jrow-1][k2-1] = (*matrix)[jrow-1][k2-1] -
                  fac * (*matrix)[i-1][k1-1];
            }
         }
      }
      (*vector)[jrow-1] = (*vector)[jrow-1] -
         (*matrix)[jrow-1][jcol-1] * (*vector)[i-1];
   }
   (*vector)[rows-1] = (*vector)[rows-1]/(*matrix)[rows-1][idiag-1];
   jbgn = idiag+1;
   for (i=1; i<rows; i++)
   {
      irow = rows-i;
      jend = idiag+i;
      if (jend > columns)
      {
         jend=columns;
      }
      jrow = irow;
      for (j=jbgn-1; j<jend; j++)
      {
         jrow = jrow+1;
         (*vector)[irow-1] = (*vector)[irow-1] -
            (*matrix)[irow-1][j] * (*vector)[jrow-1];
      }
      (*vector)[irow-1] = (*vector)[irow-1]/(*matrix)[irow-1][idiag-1];
   }
}

void ldu(float ***matrix, adouble **vector, unsigned int rows, unsigned int columns, bool flag)
{
   unsigned int i, j, k, jbgn, jend, jrow=0, jcol=0, k1, k2, irow, ibd2, idiag;

   float fac;

   ibd2 = (columns-1)/2;
   idiag = ibd2 + 1;
   for (i=1; i < rows; i++)
   {
      jend = rows-i;
      if (jend > ibd2)
         jend = ibd2;
      for (j=1; j==jend; j++)
      {
         jrow = i+j;
         jcol = ibd2-j+1;
         if (flag)
         {
            fac = (*matrix)[jrow-1][jcol-1]/(*matrix)[i-1][idiag-1];
            (*matrix)[jrow-1][jcol-1] = fac;
            for (k=1; k==jend; k++)
            {
               k1 = idiag+k;
               k2 = jcol+k;
               (*matrix)[jrow-1][k2-1] = (*matrix)[jrow-1][k2-1] -
                  fac * (*matrix)[i-1][k1-1];
            }
         }
      }
      (*vector)[jrow-1] = (*vector)[jrow-1] -
         (*matrix)[jrow-1][jcol-1] * (*vector)[i-1];
   }
   (*vector)[rows-1] = (*vector)[rows-1]/(*matrix)[rows-1][idiag-1];
   jbgn = idiag+1;
   for (i=1; i<rows; i++)
   {
      irow = rows-i;
      jend = idiag+i;
      if (jend > columns)
      {
         jend=columns;
      }
      jrow = irow;
      for (j=jbgn-1; j<jend; j++)
      {
         jrow = jrow+1;
         (*vector)[irow-1] = (*vector)[irow-1] -
            (*matrix)[irow-1][j] * (*vector)[jrow-1];
      }
      (*vector)[irow-1] = (*vector)[irow-1]/(*matrix)[irow-1][idiag-1];
   }
}
#endif
/*
 * This method factors the n by n symmetric positive definite
 * matrix A as LL(T) where L is a lower triangular matrix.
 * The method assumes that at least the lower triangle
 * of A is filled on entry. On exit, the lower triangle
 * of A has been replaced by L.
 */

bool Cholesky_Decomposition(double **a, int n)
{
   double sum, diff;
   int i, j, k;
   for (i=0; i<n; i++)
   {
      sum = 0.0;
      for (j=0; j<i; j++)
      {
         sum += a[i][j]*a[i][j];
      }
      diff = a[i][i] - sum;
      if (diff <= 0.0)// not positive definite...
      {
         /*
           cout.precision(3);
           cout.setf(ios::scientific|ios::showpos);
           cout << "\nCholesky decomposition failed...\n" << i <<", "<< j << "\n";
           cout << "A is not positive definite.\n";
           for (i=0; i<n; i++)
           {
           for (j=0; j<n; j++)
           {
           cout << a[i][j] << " ";
           }
           cout << "\n";
           }
           cout << "\n\nDiagonal elements: Sum: Difference: \n\n";
           for (i=0; i<n; i++)
           {
           sum = 0.0;
           for (j=0; j<i; j++)
           {
           sum += a[i][j]*a[i][j];
           }
           diff = a[i][i] - sum;
           cout << a[i][i] << " " << sum << " " << diff << "\n";
           }
         */
         return(false);
      }
      errno = 0;
      a[i][i] = pow(diff, 0.5);
      for (k=i+1; k<n; k++)
      {
         sum = 0.0;
         for (j=0; j<i; j++)
         {
            sum += a[k][j]*a[i][j];
         }
         a[k][i] = (a[k][i] - sum)/a[i][i];
      }
   } 

   // fill in the upper triangular with the transpose of the lower triangular:

   for (i=0; i<n-1; i++)
   {
      for (j=i+1; j<n; j++)
      {
         a[i][j] = 0.0;
         //         a[i][j] = a[j][i];
      }
   }
   /*
     for (i=0; i<n; i++)
     {
     for (j=0; j<n; j++)
     {
     cout << a[i][j] << "\t";
     }
     cout << "\n";
     }
     cout << "\n\n";
   */
   if (errno > 0)
   {
      errno = 0;
      return(false);
   }
   else
   {
      return(true);
   }
} 

//
// solve the system Ax=b using Cholesky decomposition:
// 
// A*A(-1)=I, A=LL', L(L'*A(-1)) = I, L'*A(-1) = y, L * y = I (solve for y), now L' * A(-1) = y (solve for A(-1))
// 

bool Cholesky_Invert(double **a, double **ainv, int n) 
{
   int i, j;
   double *vector;
   vector = new double [n];

   // decompose A:

   if (!Cholesky_Decomposition(a,  n))
   {
      return (false);
   }

   // set up ainv to contain the Identity matrix:
   for (j=0; j<n; j++)
   {
      for (i=0; i<n; i++)
      {
         if (i == j)
         {
            vector[i] = 1.0;
         }
         else
         {
            vector[i] = 0.0;
         }
      }

      // Solve for each column j:
      Cholesky_SolveSystem(a, vector, n);

      // Assign the solution to the appropriate column of ainv:
      for (i=0; i<n; i++)
      {
         ainv[i][j] = vector[i];
      }
   }
   delete [] vector;
   return(true);
}

/*
 * Cholesky_SolveSystem expects a Cholesky-decomposed L-matrix (n x n) with the 
 * lower diagonal filled, and the right hand side "b". Using forward and
 * backward substitution, the right hand side "b" is replaced by the solution
 * vector "x"
 */
bool Cholesky_SolveSystem(double **l, double *b, int n) 
{
   int i,j;

   // Forward substitution:

   for (i=0; i<n; i++)
   {
      for (j=0; j<i; j++)
      {
         b[i] = b[i] - l[i][j] * b[j];
      }
      b[i] = b[i] / l[i][i];
   }

   // Backward substitution:

   for (i=n-1; i>=0; i--)
   {
      for (j=n-1; j>i; j--)
      {
         b[i] = b[i] - l[j][i] * b[j];
      }
      b[i] = b[i] / l[i][i];
   }
   return (true);
}

/**
 * luDecomposition performs LU Decomposition on a matrix. This routine
 * must be given an array to mark the row permutations and a flag
 * to mark whether the number of permutations was even or odd.
 * Reference: Numerical Recipes in C.
 */

void LU_Decomposition(double **matrix, int *index, bool parity, int n)
{

   // imax is position of largest element in the row. i,j,k, are counters
   int i,j,k,imax = 0;
   
   // amax is value of largest element in the row. 
   // dum is a temporary variable.

   double amax, dum = 0;

   // scaling factor for each row is stored here
   double *scaling;
   scaling = new double [n];

   // a small number != zero
   double tiny = 1.0E-20;
   
   // Is the number of pivots even?
   parity = true;

   // Loop over rows to get the scaling information
   // The largest element in the row is the inverse of the scaling factor.
   for (i = 0; i < n; i++) 
   {
      amax = 0;
      for (j = 0; j < n; j++) 
      {
         if (fabs(matrix[i][j]) > amax) 
         {
            amax = matrix[i][j];
         }
      }
      if ( amax == 0 )
      { 
         cout << ("Singular Matrix");
      }
      // Save the scaling
      scaling[i] = 1.0/amax;
   }

   // Loop over columns using Crout's Method.
   for (j = 0; j < n; j++) {

      // lower left corner
      for (i = 0; i < j; i++) 
      {
         dum = matrix[i][j];
         for (k = 0; k < i; k++)
         {
            dum -= matrix[i][k] * matrix[k][j];
         }
         matrix[i][j] = dum;
      }

      // Initialize search for largest element
      amax = 0.0;
      
      // upper right corner
      for (i = j; i < n; i++) 
      {
         dum = matrix[i][j];
         for (k = 0; k < j; k++) 
         {
            dum -= matrix[i][k] * matrix[k][j];
         }
         matrix[i][j] = dum;
         if (scaling[i] * fabs(dum) > amax) 
         {
            amax = scaling[i]* fabs(dum);
            imax = i;
         }
      }

      // Change rows if it is necessary
      if ( j != imax)
      {
         for (k = 0; k < n; k++) 
         {
            dum = matrix[imax][k];
            matrix[imax][k] = matrix[j][k];
            matrix[j][k] = dum;
         }
         // Change parity
         parity = !parity;
         scaling[imax] = scaling[j];
      }
      // Mark the column with the pivot row.
      index[j] = imax;

      // replace zeroes on the diagonal with a small number.
      if (matrix[j][j] == 0.0) 
      {
         matrix[j][j] = tiny;
      }
      // Divide by the pivot element
      if (j != n) 
      {
         dum = 1.0/matrix[j][j];
         for (i=j+1; i < n; i++) 
         {
            matrix[i][j] *= dum;
         }
      }
   }
   delete [] scaling;
}
   
/**
 * Do the backsubstitution on matrix a which is the LU decomposition
 * of the original matrix. b is the right hand side vector which is NX1. b 
 * is replaced by the solution. index is the array that marks the row
 * permutations.
 */

void LU_BackSubstitute(double **a, double **b, int *index, int n) 
{
   // counters
   int i, ip, j, ii = -1;
   double sum = 0;

   for (i = 0; i < n; i++) 
   {
      ip = index[i];
      sum = b[ip][0];
      b[ip][0] = b[i][0];
      if (ii != -1) 
      {
         for (j = ii; j < i; j++)
         {
            sum -= a[i][j] * b[j][0];
         }
      }
      else 
      {
         if ( sum != 0) 
         {
            ii = i;
         }
      }
      b[i][0] = sum;
   }
   for (i=n-1; i >= 0; i--)
   {
      sum = b[i][0];
      for (j = i+1; j < n; j++) 
      {
         sum -= a[i][j] * b[j][0];
      }
      b[i][0] = sum / a[i][i];
   }
}

/**
 * Solve a set of linear equations. a is a square matrix of coefficients.
 * b is the right hand side. b is replaced by solution. 
 * Target is replaced by its LU decomposition.
 */

void LU_SolveSystem(double **a, double **b, int n)
{
   bool parity = true;
   int *index;
   index  = new int [n];
   LU_Decomposition(a, index, parity, n);
   LU_BackSubstitute(a, b, index, n);
   delete [] index;
}

/*
 * Invert a matrix. decomp==true: lu_decomp, decomp==false: cholesky decomp.
 */

void LU_Invert(double **matrix, double **result, int n)
{
   int i, *index;
   
   bool parity = true;
   // temporary storage
   double **col;
   // Place for LU decomposition
   double **lud;
   col = new double *[n];
   lud = new double *[n];
   index = new int [n];
   for (i=0; i<n; i++)
   {
      col[i] = new double [n];
      lud[i] = new double [n];
   }
   // An array holding the permutations used by LU decomposition
   
   // Swap matrix for its LU decomposition
   LU_Decomposition(matrix, index, parity, n);

   // Do backsubstitution with the b matrix being all zeros except for
   // a 1 in the row that matches the column we're in.
   for (int j = 0; j < n; j++) 
   {
      for (i = 0; i < n; i++) 
      {
         col[i][0] = 0;
      }
      col[j][0] = 1;
      LU_BackSubstitute(matrix, col, index, n);
      
      // plug values into result
      for (i = 0; i < n; i++) 
      {
         result[i][j] = col[i][0];
      }
   }
   
   for (i=0; i<n; i++)
   {
      delete [] col[i];
      delete [] lud[i];
   }
   delete [] col;
   delete [] lud;
   delete [] index;
}

/* This routine is supplied with M, a matrix containing the Jacobian (the value of each linear
   functional term at point x_i: 


   (F_0)x1   (F_1)x1   (F_2)x1 ... (F_order)x1
   (F_0)x2   (F_1)x2   (F_2)x2 ... (F_order)x2
   (F_0)x3   (F_1)x3   (F_2)x3 ... (F_order)x3
   M =   .      .      .
   .      .      .  ...
   .      .      .
   (F_0)xn   (F_1)xn   (F_2)xn ... (F_order)xn

   The variable "coeff" is a vector containing the linear coefficients for each functional term.
   y_raw contains the experimental y-values on entry
*/
void generalLeastSquares(float **M, unsigned int points, unsigned int order, float *y_raw, float **coeff)
{
   double **A, *B;
   unsigned int i, j, k;
   A = new double *[order];
   B = new double [order];
   for (i=0; i<order; i++)
   {
      A[i] = new double [order];
   }
   // generate matrix for decomposition (only lower triangular is needed for Cholesky)
   for (i=0; i<order; i++)
   {
      for (j=0; j<=i; j++)
      {
         A[i][j] = 0;
         for (k=0; k<points; k++)
         {
            A[i][j] += (double) M[k][i] * M[k][j];
         }
      }
   }
   for (j=0; j<order; j++)
   {
      B[j] = 0.0;
      for (k=0; k<points; k++)
      {
         B[j] += (double) y_raw[k] * M[k][j]; // y_raw contains the experimental y-values on entry
      }
   }
   Cholesky_Decomposition(A, order);
   Cholesky_SolveSystem(A, B, order);
   for (i=0; i<order; i++)
   {
      (*coeff)[i] = B[i];
   }
   for (i=0; i<order; i++)
   {
      delete [] A[i];
   }
   delete [] A;
   delete [] B;
}

US_lsfit::US_lsfit(double *coeff, double *x, double *y, int ord, int n, bool mesg)
{
   order = ord;
   numpoints = n;
   c = coeff;
   xval = x;
   yval = y;
   A = new double *[order];
   for (i=0; i<order; i++)
   {
      A[i] = new double [order];
   }
   b = new double [order];
   calc_coeff_polynomial();
   if (mesg)
   {
      int i;
      QString str1, str2;
      str1.sprintf("Coefficients for %d-order Polynomial fit:\n\n", order-1);
      for (i=0; i<order; i++)
      {
         str2.sprintf("%d order: %e\n", i, c[i]);
         str1.append(str2);
      }
      US_Static::us_message("Coefficients: ", str1);
   }
   for (i=0; i<order; i++)
   {
      delete [] A[i];
   }
   delete [] A;
   delete [] b;
}

US_lsfit::~US_lsfit()
{
}

void US_lsfit::calc_coeff_polynomial()
{

   /* 

   to calculate the matrix, calculate the product M'M, where M is the matrix constructed as follows:


   x1^0   x1^1   x1^2 ... x1^order
   x2^0   x2^1   x2^2 ... x2^order
   x3^0   x3^1   x3^2 ... x3^order
   M =   .      .      .
   .      .      .  ...
   .      .      .
   xn^0   xn^1   xn^2 ... xn^order

   A = M'M, and A is positive - definite, and can be decomposed by the Cholesky method.
   evaluate A by filling in the lower triangular of the A matrix. NOTE: instead of polynomials
   evaluated at each point xi, other linearly independent basis function sets can be used for a fit.

   */

   A[0][0] = (double) numpoints;
   
   for (i=1; i<order; i++)
   {
      for (j=0; j<=i; j++)
      {
         A[i][j] = 0;
         for (k=0; k<numpoints; k++)
         {
            A[i][j] += pow(xval[k], i) * pow(xval[k], j);
         }
      }
   } // only the lower triangular matrix is filled now, which is sufficient for solution by 
     // Cholesky decomposition.
   
   // print_matrix(A, order, order);
      
   // evaluate b:

   for (i=0; i<order; i++)
   {
      b[i] = 0;
      for (k=0; k<numpoints; k++)
      {
         b[i] += yval[k]*pow(xval[k], i);
      }
   }
   
   //
   // solve the system Ax=b using Cholesky decomposition:
   // 
   // Ax=b, A=LL', L(L'x)=b, L'x=y, Ly=b (solve for y), now L'x=y (solve for x)
   // 

   Cholesky_Decomposition(A, order);
   // print_matrix(A, order, order);
   Cholesky_SolveSystem(A, b, order);
   
   for (i=0; i<order; i++)
   {
      c[i] = b[i];
   }
}

void print_matrix( double **a, int rows, int cols)
{
   cout.precision(3);
   cout.setf(ios::scientific|ios::showpos);
   int i, j;
   for (i=0; i<rows; i++)
   {
      for (j=0; j<cols; j++)
      {
         cout << a[i][j] << " ";
      }
      cout << "\n";
   }
   cout << "\n\n";
}

void print_vector(double **v, unsigned int rows, const char *c)
{
   cout.setf(ios::scientific);
   unsigned int i;
   for (i=0; i<rows; i++)
   {
      cout << c << "[" << i << "]=" << (*v)[i] << "\n";
   }
   cout << "\n\n";
}

void conc_dep_s(float sigma, double **right, double **c_current, 
                double ***a2, float temp1, unsigned int points)
{
   double *ca;
   unsigned int i;
   ca = new double [points];
   if (ca == NULL)
   {
      us_qdebug("Not enough memory in conc_dep_s");
      return;
   }
   for (i=0; i<points; i++)
   {
#if defined SECOND_ORDER
      ca[i] = temp1 * sigma * square(c_current[i]) +
         square(sigma) * square((*c_current)[i]) * (*c_current)[i];
#else
      ca[i] = (double) temp1 * sigma * square((*c_current)[i]);
#endif
   }
   m3vm(a2, &ca, points);
   for (i=0; i<points; i++)
   {
      (*right)[i] = (*right)[i] - ca[i];
   }
   delete [] ca;
}

void conc_dep_s(float sigma, float **right, float **c_current, 
                float ***a2, float temp1, unsigned int points)
{
   float *ca;
   unsigned int i;
   ca = new float [points];
   if (ca == NULL)
   {
      us_qdebug("Not enough memory in conc_dep_s");
      return;
   }
   for (i=0; i<points; i++)
   {
#if defined SECOND_ORDER
      ca[i] = temp1 * sigma * square(c_current[i]) +
         square(sigma) * square((*c_current)[i]) * (*c_current)[i];
#else
      ca[i] = temp1 * sigma * square((*c_current)[i]);
#endif
   }
   m3vm(a2, &ca, points);
   for (i=0; i<points; i++)
   {
      (*right)[i] = (*right)[i] - ca[i];
   }
   delete [] ca;
}

#ifdef ADOLC
void conc_dep_s(adouble sigma, adouble **right, adouble **c_current, 
                float ***a2, adouble temp1, unsigned int points)
{
   adouble *ca;
   unsigned int i;
   ca = new adouble [points];
   if (ca == NULL)
   {
      us_qdebug("Not enough memory in conc_dep_s");
      return;
   }
   for (i=0; i<points; i++)
   {
#if defined SECOND_ORDER
      ca[i] = temp1 * sigma * pow(c_current[i], 2.0) +
         pow(sigma, 2.0) * pow((*c_current)[i], 2.0) * (*c_current)[i];
#else
      ca[i] = temp1 * sigma * pow((*c_current)[i], 2.0);
#endif
   }
   m3vm(a2, &ca, points);
   for (i=0; i<points; i++)
   {
      (*right)[i] = (*right)[i] - ca[i];
   }
   delete [] ca;
}

void conc_dep_s(adouble sigma, adouble **right, adouble **c_current, 
                double ***a2, adouble temp1, unsigned int points)
{
   adouble *ca;
   unsigned int i;
   ca = new adouble [points];
   if (ca == NULL)
   {
      us_qdebug("Not enough memory in conc_dep_s");
      return;
   }
   for (i=0; i<points; i++)
   {
#if defined SECOND_ORDER
      ca[i] = temp1 * sigma * pow(c_current[i], 2.0) +
         pow(sigma, 2.0) * pow((*c_current)[i], 2.0) * (*c_current)[i];
#else
      ca[i] = temp1 * sigma * pow((*c_current)[i], 2.0);
#endif
   }
   m3vm(a2, &ca, points);
   for (i=0; i<points; i++)
   {
      (*right)[i] = (*right)[i] - ca[i];
   }
   delete [] ca;
}
#endif

/*
  subroutine to calculate concentration dependency of D, add result to right():
*/

void conc_dep_d(float delta, double **right, double **c_current, double ***u,
                double ***v, double ***w, float temp2, unsigned int points, double **cu,
                double **cv, double **cw)
{
   unsigned int i;
   for (i=0; i<points-1; i++)
   {
      (*cu)[i] = delta * (*c_current)[i] * (*c_current)[i+1];
      (*cv)[i] = delta * square((*c_current)[i]);
      (*cw)[i] = (*cu)[i];
   }
   (*cv)[points-1] = delta * square((*c_current)[i]);
   (*cu)[points-1] = (*cv)[points-1];

   /* calculate UCU, VCV and WCW: */

   m2vm(u, cu, points);
   m3vm(v, cv, points);
   m2vm(w, cw, points);

   /* update right hand side: */

   for (i=0; i<points; i++)
   {
      (*right)[i] = (*right)[i] + temp2 * ((*cu)[i] + (*cv)[i] + (*cw)[i]);
   }
}

void conc_dep_d(float delta, float **right, float **c_current, float ***u,
                float ***v, float ***w, float temp2, unsigned int points, float **cu,
                float **cv, float **cw)
{
   unsigned int i;
   for (i=0; i<points-1; i++)
   {
      (*cu)[i] = delta * (*c_current)[i] * (*c_current)[i+1];
      (*cv)[i] = delta * square((*c_current)[i]);
      (*cw)[i] = (*cu)[i];
   }
   (*cv)[points-1] = delta * square((*c_current)[i]);
   (*cu)[points-1] = (*cv)[points-1];

   /* calculate UCU, VCV and WCW: */

   m2vm(u, cu, points);
   m3vm(v, cv, points);
   m2vm(w, cw, points);

   /* update right hand side: */

   for (i=0; i<points; i++)
   {
      (*right)[i] = (*right)[i] + temp2 * ((*cu)[i] + (*cv)[i] + (*cw)[i]);
   }
}

#ifdef ADOLC
void conc_dep_d(adouble delta, adouble **right, adouble **c_current, float ***u,
                float ***v, float ***w, adouble temp2, unsigned int points, adouble **cu,
                adouble **cv, adouble **cw)
{
   unsigned int i;
   for (i=0; i<points-1; i++)
   {
      (*cu)[i] = delta * (*c_current)[i] * (*c_current)[i+1];
      (*cv)[i] = delta * pow((*c_current)[i], 2.0);
      (*cw)[i] = (*cu)[i];
   }
   (*cv)[points-1] = delta * pow((*c_current)[i], 2.0);
   (*cu)[points-1] = (*cv)[points-1];

   /* calculate UCU, VCV and WCW: */

   m2vm(u, cu, points);
   m3vm(v, cv, points);
   m2vm(w, cw, points);

   /* update right hand side: */

   for (i=0; i<points; i++)
   {
      (*right)[i] = (*right)[i] + temp2 * ((*cu)[i] + (*cv)[i] + (*cw)[i]);
   }
}

void conc_dep_d(adouble delta, adouble **right, adouble **c_current, double ***u,
                double ***v, double ***w, adouble temp2, unsigned int points, adouble **cu,
                adouble **cv, adouble **cw)
{
   unsigned int i;
   for (i=0; i<points-1; i++)
   {
      (*cu)[i] = delta * (*c_current)[i] * (*c_current)[i+1];
      (*cv)[i] = delta * pow((*c_current)[i], 2.0);
      (*cw)[i] = (*cu)[i];
   }
   (*cv)[points-1] = delta * pow((*c_current)[i], 2.0);
   (*cu)[points-1] = (*cv)[points-1];

   /* calculate UCU, VCV and WCW: */

   m2vm(u, cu, points);
   m3vm(v, cv, points);
   m2vm(w, cw, points);

   /* update right hand side: */

   for (i=0; i<points; i++)
   {
      (*right)[i] = (*right)[i] + temp2 * ((*cu)[i] + (*cv)[i] + (*cw)[i]);
   }
}
#endif
/*
  #ifdef THREAD

  void calc_A_transpose_A(double ***A, double ***product, unsigned int rows, unsigned int columns, bool flag)
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


  void calc_A_transpose_A(double ***A, double ***product, unsigned int rows, unsigned int columns, bool flag)
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
                                  vector <vector <dpairs> > *a_dataarray
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

      vector <dpairs> m, n;
      unsigned int i, j, k, l;

      for (i = c_start; i < c_end; i++) {
         for (j = 0; j < columns; j++) {
            if (i <= j) {
               double sum=0;
               k=0;
               l=0;
               m.clear( );
               n.clear( );
               m=(*dataarray)[i];
               n=(*dataarray)[j];
               while((k<m.size()) && (l<n.size())) {
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

void calc_A_transpose_A(double ***A, double ***product, unsigned int rows, unsigned int columns, unsigned int threads)
{
   vector <dpairs> data_pairs, m, n;
   vector <vector <dpairs> > dataarray;
   dpairs temp_pair;
   dataarray.clear( );
   data_pairs.clear( );
   m.clear( );
   n.clear( );
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
      data_pairs.clear( );
   }
   //   unsigned int threads = 4; // USglobal->config_list.numThreads
   //   printf("calc ata threads %d\n", threads);
   if(threads > 1) {
     
      // create threads
      unsigned int j;
     
      //ata_d_thr_t *ata_d_thr_threads[threads];
      vector < ata_d_thr_t* > ata_d_thr_threads( threads );
     
      for(j = 0; j < threads; j++) {
         ata_d_thr_threads[j] = new ata_d_thr_t(j);
         ata_d_thr_threads[j]->start();
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
         ata_d_thr_threads[j]->ata_d_thr_setup(columns, c_start, c_end, product, &dataarray);
         c_start = c_end;
      }
     
      for(j = 0; j < threads; j++) {
         ata_d_thr_threads[j]->ata_d_thr_wait();
      }
     
      // destroy
      for(j = 0; j < threads; j++) {
         ata_d_thr_threads[j]->ata_d_thr_shutdown();
      }
     
      for(j = 0; j < threads; j++) {
         ata_d_thr_threads[j]->wait();
      }
     
      for(j = 0; j < threads; j++) {
         delete ata_d_thr_threads[j];
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
               m.clear( );
               n.clear( );
               m=dataarray[i];
               n=dataarray[j];
               while((k<m.size()) && (l<n.size()))
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

  void calc_A_transpose_A(float ***A, float ***product, unsigned int rows, unsigned int columns, bool flag)
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


  void calc_A_transpose_A(float ***A, float ***product, unsigned int rows, unsigned int columns, bool flag)
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
void calc_A_transpose_A(float ***A, float ***product, unsigned int rows, unsigned int columns)
{
   vector <dpairs> data_pairs, m, n;
   vector <vector <dpairs> > dataarray;
   dpairs temp_pair;
   dataarray.clear( );
   data_pairs.clear( );
   m.clear( );
   n.clear( );
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
      data_pairs.clear( );
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
            m.clear( );
            n.clear( );
            m=dataarray[i];
            n=dataarray[j];
            while((k<m.size()) && (l<n.size()))
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

double calc_vector_transpose_times_vector(unsigned int rows, double **vector1, double **vector2)
{
   double result = 0.0;
   for (unsigned int i=0; i<rows; i++)
   {
      result += (*vector1)[i] * (*vector2)[i];
   }
   return result;
}

float calc_vector_transpose_times_vector(unsigned int rows, float **vector1, float **vector2)
{
   float result = 0.0;
   for (unsigned int i=0; i<rows; i++)
   {
      result += (*vector1)[i] * (*vector2)[i];
   }
   return result;
}

void calc_matrix_times_matrix(double ***A, double ***B, double ***product, unsigned int rows, unsigned int columns)
{
   for (unsigned int i=0; i<rows; i++)
   {
      for (unsigned int j=0; j<columns; j++)
      {
         (*product)[i][j] = 0.0;
         for (unsigned int k=0; k<columns; k++)
         {
            (*product)[i][j] += (*A)[i][k] * (*B)[k][j];
         }
      }
   }
}

void calc_matrix_times_matrix(float ***A, float ***B, float ***product, unsigned int rows, unsigned int columns)
{
   for (unsigned int i=0; i<rows; i++)
   {
      for (unsigned int j=0; j<columns; j++)
      {
         (*product)[i][j] = 0.0;
         for (unsigned int k=0; k<columns; k++)
         {
            (*product)[i][j] += (*A)[i][k] * (*B)[k][j];
         }
      }
   }
}

void calc_matrix_times_vector(double ***A, double **B, double **product, unsigned int rows, unsigned int columns)
{
   for (unsigned int i=0; i<rows; i++)
   {
      (*product)[i] = 0.0;
      for (unsigned int j=0; j<columns; j++)
      {
         (*product)[i] += (*A)[i][j] * (*B)[j];
      }
   }
}

void calc_matrix_times_vector(float ***A, float **B, float **product, unsigned int rows, unsigned int columns)
{
   for (unsigned int i=0; i<rows; i++)
   {
      (*product)[i] = 0.0;
      for (unsigned int j=0; j<columns; j++)
      {
         (*product)[i] += (*A)[i][j] * (*B)[j];
      }
   }
}
