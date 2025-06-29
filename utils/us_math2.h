//! \brief us_math2.h
#ifndef US_MATH_H
#define US_MATH_H

#include <QtCore>
#include "us_extern.h"
#include "us_dataIO.h"
#include "us_solution.h"

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

#ifndef DbgLv
#define DbgLv(a) if(0 >= a)qDebug()
#endif

//! \brief A collecion of methematical routines.  All functions are static.
class US_UTIL_EXTERN US_Math2
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
         bool   manual;           //!< manual/unadjusted dens.,visc.
      };


      //! \brief Normal random variate generator
      //! \param m - mean
      //! \param s - standard deviation
      static double  box_muller( double, double );

      //! \brief Return a random floating point number
      static double  ranf      ( void );      

      //! \brief Given a line as a set of x and y coordinates, calculate
      //!        the line's characteristics
      //!       
      //! \param x Pointer to an array of x values
      //! \param y Pointer to an array of y values
      //! \param slope       Pointer to location to return the line's slope
      //! \param intercept   Pointer to location to return the line's y intercept
      //! \param sigma       Pointer to location to return the square root of the population variance (not related to the fit result)
      //! \param correlation Pointer to location to return the correlation
      //! \param arraysize   Input array size
      static double linefit   ( double**, double**, double*, double*, double*, 
                                double* , int );

      //! \brief Given a set of curve points and a given outside point, return
      //!        the curve point nearest the given point. Return either a point
      //!        in the curve array or one interpolated based on distance.
      //!        Optionally also return a value in a third dimension that
      //!        corresponds to the nearest point.
      //!       
      //! \param xs       An array of x values of the curve
      //! \param ys       An array of y values of the curve
      //! \param npoints  The number of curve points
      //! \param interp   Flag to interpolate curve point (false->exact point)
      //! \param xgiven   The x value of the given point
      //! \param ygiven   The y value of the given point
      //! \param xnear    Pointer for return of x of the nearest curve point
      //! \param ynear    Pointer for return of y of the nearest curve point
      //! \param zs       Optional third dimension values array (if non-NULL)
      //! \param znear    Optional pointer for return of z for nearest point
      //! \return         Index in arrays of nearest curve point
      static int nearest_curve_point( double*, double*, const int, bool,
                                      double&, double&, double*, double*,
                                      double*, double* );

      //! \brief Given the slopes and intercepts of two lines, return the
      //!        intersection point of the two lines, if it exists.
      //! \param slope1   The slope of the first line
      //! \param intcp1   The intercept of the first line
      //! \param slope2   The slope of the second line
      //! \param intcp2   The intercept of the second line
      //! \param xisec    Pointer for return of x of the intersection point
      //! \param yisec    Pointer for return of y of the intersection point
      //! \return         Boolean flag if intersection exists (false->parallel)
      static bool intersect( double&, double&, double&, double&,
                             double*, double* );

      //! \brief Given arrays representing two curves, return the intersection
      //!        point of two lines fitted to them, if it exists.
      //! \param x1s      An array of x values of the first curve
      //! \param y1s      An array of y values of the first curve
      //! \param npoint1  The number of points for the first curve
      //! \param x2s      An array of x values of the second curve
      //! \param y2s      An array of y values of the second curve
      //! \param npoint2  The number of points for the second curve
      //! \param xisec    Pointer for return of x of the intersection point
      //! \param yisec    Pointer for return of y of the intersection point
      //! \return         Boolean flag if intersection exists (false=>parallel)

      static bool intersect( double*, double*, int, double*, double*, int,
                             double*, double* );
      //! \brief A routine to calculate the vbar and MW of a peptide sequence
      //! \param pep      The structure to be populated
      //! \param sequence The secuence used for the calculations
      //! \param temperature The temperature to use for calculations

      static void calc_vbar( Peptide&, const QString&, double );

      //! Adjust the vbar for temperature.  The values used in the 
      //! unadjusted calculations are based on 20 degrees C.  We 
      //! use an observed linear adjustment of 0.000425 per degreee.
      
      //! \param vbar The unadjusted vbar value
      //! \param degC The temperature used for the adjustment
      static double adjust_vbar20( double vbar, double degC )
      // { return vbar + 4.25e-4 * ( degC - 20.0 ); }
      { return vbar; }

      //! The inverse of adjust_vbar20.  Returns vbar20.
      
      //! \param vbar The vbar value at the specified temperature
      //! \param degC The temperature of the sample associated with vbar
      static double adjust_vbar( double vbar, double degC )
      // { return vbar + 4.25e-4 * ( 20.0 - degC ); }
      { return vbar; }

      //! \brief Correct buffer data for temperature
      //! \param t  Temperature of solution
      //! \param d  Data to be corrected

      static void data_correction( double, SolutionData& );

      //! \brief A routine to calculate the value of a normally distributed
      //!        value.
      //! \param sigma Standard deviation of the distribution
      //! \param mean  Mean value of the distribution
      //! \param x     Point where to compute desired value
      static double normal_distribution( double, double, double );

      //! \brief Calculate the time correction in a run due to acceleration
      //!        of the rotor.
      //! \param dataList The list of editedData for the run
      static double time_correction( const QVector< US_DataIO::EditedData >& );

      //! \brief Calculate the time correction in a run due to acceleration
      //!        of the rotor.
      //! \param dataList The list of rawData for the run
      static double time_correction( const QVector< US_DataIO::RawData >& );

      //! \brief Set the sysem random sequence.
      //! \return The seed used to set the system time
      static uint randomize( void );

      //! \brief Set the sysem random sequence.
      //! \param seed A predetermined seed 
      //! \return The seed used to set the system time
      static uint randomize( uint );

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
                    returned value is not needed.

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

      static void gaussian_smoothing( QVector< double >&, int );

      //! \brief Calculate common vbar of a solution
      //! \param solution    Solution for which to calculate common vbar
      //! \param temperature Average temperature of associated experiment
      //! \return The common temperature-corrected vbar
      static double calcCommonVbar( US_Solution&, double );

#ifdef WIN32
      //! \brief An approximation of the Complimentary Gauss
      //! Error Function, erfc().  Use for WIN32 only as Linux already 
      //! has it implemented in <math.h>
      //! \param x  The input value
      //! \return Approximation of erfc(x)
      static double erfc( double );
#endif

      //! \brief Compute best uniform grid repetitions for 2DSA
      //!        and possibly modify grid point counts in each dimension
      //! \param ngrid_s Reference to grid points in the s dimension,
      //!                both input and possibly modified value
      //! \param ngrid_k Reference to grid points in the k dimension,
      //!                both input and possibly modified value
      //! \return Computed best number of grid repetitions
      static int best_grid_reps( int&, int& );

      //! \brief Compute the norm value of a simulation data set
      //! \param simdat  Pointer to a simulation data set
      //! \return        The norm value of the dataset values
      static double norm_value( US_DataIO::RawData* );

      //! \brief Compute the norm value of an experiment data set
      //! \param expdat  Pointer to an experiement data set
      //! \return        The norm value of the dataset values
      static double norm_value( US_DataIO::EditedData* );

      //! \brief Compute the norm value of a data array
      //! \param datary  Pointer to an array of data values
      //! \param nval    Count of values in the array
      //! \return        The norm value of the array
      static double norm_value( double*, const int );

      //! \brief Compute the norm value of a data vector
      //! \param datvec  Pointer to a vector of data values
      //! \return        The norm value of the vector
      static double norm_value( QVector< double >* );

      //! \brief Find root X where evaluated Y is virtually equal to a goal.
      //! \param goal    Goal value to approach
      //! \return        The root X for Y virtually equal to goal
      static double find_root ( const double goal );
      private:

      static void _nnls_g1 ( double a, double b, double*, double*, double* );
      static int  _nnls_h12( int, int, int, int m, double*, int,
                             double*, double *, int, int, int );
};
#endif

