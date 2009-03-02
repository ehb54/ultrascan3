//! \brief us_math.h
#ifndef US_MATH_H
#define US_MATH_H

#include "us_extern.h"

#define sq(x) ((x) * (x))
#ifndef max
   #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
   #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

//! \brief A collecion of methematical routines.  All functions are static.
class US_EXTERN US_Math
{
   public:
      //! \brief Normal random variate generator
      //! \param m - mean
      //! \param s - standard deviation
      static float  box_muller( float, float );

      //! \brief Return a random floating point number
      static float  ranf      ( void );      

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
  
      //! \brief An overloaded version of the above function with floats 
      //!        instead of doubles.
      //!       
      //! \param x Pointer to an array of x values
      //! \param y Pointer to an array of y values
      //! \param slope       Pointer to location to retrun the line's slope
      //! \param intercept   Pointer to location to retrun the line's y intercept
      //! \param sigma       Pointer to location to retrun the standard deviation 
      //! \param correlation Pointer to location to retrun the correlation
      //! \param arraysize   Input array size
      static float  linefit   ( float** , float** , float* , float* , float* , 
                                float*  , int );
};
#endif
