//! \brief us_math.h
#ifndef US_MATH_H
#define US_MATH_H

#include <QtCore>
#include "us_extern.h"
#include "us_dataIO2.h"

#define sq(x) ((x) * (x))
#ifndef max
   #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
   #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef swap_double
   // No need to protect a and b for swap
   #define swap_double(a,b) {double t = a; a=b; b=t;}
#endif

//! \brief A collecion of methematical routines.  All functions are static.
class US_EXTERN US_Math2
{
   public:
      //! A structure used for calculating a peptide sequence
      class Peptide
      {
         public:
         uint   a;        //!< alanine
         uint   b;        //!< M or D
         uint   c;        //!< cysteine
         uint   d;        //!< aspartic acid
         uint   e;        //!< glutamic acid
         uint   f;        //!< phenylalanine
         uint   g;        //!< glycine
         uint   h;        //!< histidine
         uint   i;        //!< isoleucine
         uint   j;        //!< Hao (James Nowick)
         uint   k;        //!< lysine
         uint   l;        //!< leucine
         uint   m;        //!< methionine
         uint   n;        //!< asparagine
         uint   o;        //!< delta-linked ornithin (James Nowick)
         uint   p;        //!< proline
         uint   q;        //!< glutamine
         uint   r;        //!< arginine
         uint   s;        //!< serine
         uint   t;        //!< threonine
         uint   u;        //!< currently unused
         uint   v;        //!< valine
         uint   w;        //!< tryptophan
         uint   x;        //!< unknown
         uint   y;        //!< tyrosine
         uint   z;        //!< E or  Q
         uint   dab;      //!< diaminobutyric acid (John Kulp), symbol: "+"
         uint   dpr;      //!< diaminopropanoic acid (John Kulp), symbol: "@"
         double vbar;     //!< partial specific volume
         double vbar20;   //!< partial specific volume at 20 deg.
         double mw;       //!< molecular weight
         double vbar_sum; //!< vbar sum
         double weight;   //!< weight
         double e280;     //!< extinction coefficient at 280 nm
         uint   residues; //!< residues     
      };

      //! A structure used for holding solution data
      class SolutionData
      {
         public:
         double density;          //!< density
         double viscosity;        //!< viscosity
         double vbar;             //!< partial specific volume
         double vbar20;           //!< partial specific volume at 20 deg.
         double density_wt;       //!< density wt - output
         double density_tb;       //!< density tb - output
         double viscosity_wt;     //!< viscosity wt - output
         double viscosity_tb;     //!< viscosity tb - output
         double buoyancyb;        //!< buoyancy b - output
         double buoyancyw;        //!< buoyancy w - output
         double s20w_correction;  //!< s correction - output
         double D20w_correction;  //!< D correction - output
      };


      //! \brief Normal random variate generator
      //! \param m - mean
      //! \param s - standard deviation
      static double  box_muller( double, double );

      //! \brief Return a random floating point number
      static double  ranf      ( void );      

      //! \brief Given a line as a set of x and y coordinates, calulate
      //!        the line's characteristics
      //!       
      //! \param x Pointer to an array of x values
      //! \param y Pointer to an array of y values
      //! \param slope       Pointer to location to retrun the line's slope
      //! \param intercept   Pointer to location to retrun the line's y intercept
      //! \param sigma       Pointer to location to retrun the standard deviation 
      //! \param correlation Pointer to location to retrun the correlation
      //! \param arraysize   Input array size
      static double linefit   ( double**, double**, double*, double*, double*, 
                                double* , int );

      //! \brief A routine to calulate the vbar and MW of a peptide sequence
      //! \param pep      The structure to be populated
      //! \param sequence The secuence used for the calculations
      //! \param temperature The temperature to use for calulations

      static void calc_vbar( Peptide&, const QString&, double );

      //! Adjust the vbar for temperature.  The values used in the 
      //! unadjusted calculations are based on 25 degrees C.  We 
      //! use an observed linear adjustment of 0.000425 per degreee.
      
      //! \param vbar The unadjusted vbar value
      //! \param degC The temperature used for the adjustment
      static double adjust_vbar20( double vbar, double degC )
      { return vbar + 0.002125 + 4.25e-4 * ( degC - 25.0 ); }

      //! The inverse of adjust_vbar20.  Returns vbar20.
      
      //! \param vbar The vbar value at the specified temperature
      //! \param degC The temperature of the sample associated with vbar
      static double adjust_vbar( double vbar, double degC )
      { return vbar - 0.002125 - 4.25e-4 * ( degC - 25.0 ); }

      //! \brief Correct buffer data for temperature
      //! \param t  Temperture of solution
      //! \param d  Data to be corrected

      static void data_correction( double, SolutionData& );

      //! \brief A routine to calulate the value of a normally distrubted
      //!        value.
      //! \param sigma Standard deviation of the distribution
      //! \param mean  Mean value of the distribution
      //! \param x     Point where to compute desired value
      static double normal_distribution( double, double, double );

      //! \brief Calculate the time correction in a run due to acceleration
      //!        of the rotor.
      //! \param dataList The list of editedData for the run

      static double time_correction( const QVector< US_DataIO2::EditedData >& );

      //! \brief Set the sysem random sequence.
      //! \return The seed used to set the system time
      static int randomize( void );

      /*! Algorithm NNLS (Non-negative least-squares)
 
      Given an m by n matrix A, and an m-vector B, computes an n-vector X,
      that solves the least squares problem A * X = B   , subject to X>=0
      The A matrix is formatted such that the columns are placed into a
      vector end-to-end, and the parameter a_dim1 contains the length of
      each column

      Function returns 0 if succesful, 1, if iteration count exceeded 3*N,
      or 2 in case of invalid problem dimensions or memory allocation error.

      Instead of pointers for working space, NULL can be given to let this
      function to allocate and free the required memory.
      
      \param a      The m by n matrix A. On exit, a[] contains the
                    product matrix Q*A, where Q is an m by n orthogonal 
                    matrix generated implicitly by this function.  The
                    matrix is column major.
      
      \param a_dim1 Since matrix A is processed as a set of vectors, 
                    a_dim1 is needed to specify the storage increment between
                    vectors in a[]

      \param m      Columns
      \param n      Rows

      \param b      On entry, b[] must contain the m-vector B.
                    On exit, b[] contains Q*B 

      \param x      On exit, x[] will contain the solution vector.
      \param rnorm  On exit, rnorm contains the Euclidean norm of the
                    residual vector.  It may be set to NULL if the 
                    retruned value is not needed.

      \param wp     An n-array of working space, w[].  On exit, w[] will 
                    contain the dual solution vector.
                    w[i]=0.0 for all i in set p and w[ i ] <= 0.0 for 
                    all i in set z.
      
      \param zzp    An m-array of working space, zz[].
      \param indexp An n-array of working space, index[].
      */

      static int nnls(
         double* a, int a_dim1, int m, int n,
         double* b,
         double* x,
         double* rnorm  = NULL,
         double* wp     = NULL,  
         double* zzp    = NULL, 
         int*    indexp = NULL
         );

      /*! \brief Remove high frequency noise from a signal
          \param array   Data to be smoothed.  This array will be modified.
          \param smooth  Number of values to smooth to be considered when 
                         smoothing
      */

      void gaussian_smoothing( QVector< double >&, int );

      private:

      static void _nnls_g1 ( double a, double b, double*, double*, double* );
      static int  _nnls_h12( int, int, int, int m, double*, int,
                             double*, double *, int, int, int );
};
#endif

