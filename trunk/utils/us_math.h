//! \brief us_math.h
#ifndef US_MATH_H
#define US_MATH_H

#include <QtCore>
#include "us_extern.h"
#include "us_dataIO.h"

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

//! A structure used for calculating a peptide sequence
struct peptide
{
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
struct solution_data
{
   double density;       //!< density
   double density_wt;    //!< density wt
   double density_tb;    //!< density tb
   double viscosity;     //!< viscosity
   double viscosity_wt;  //!< viscosity wt
   double viscosity_tb;  //!< viscosity tb
   double buoyancyb;     //!< buoyancy b
   double buoyancyw;     //!< buoyancy w
   double vbar;          //!< partial specific volume
   double vbar20;        //!< partial specific volume at 20 deg.
   double correction;    //!< correction
};


//! \brief A collecion of methematical routines.  All functions are static.
class US_EXTERN US_Math
{
   public:
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

      static void calc_vbar( struct peptide&, const QString&, double );

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

/*  
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
*/
      //! \brief Correct buffer data for temperature
      //! \param t  Temperture of solution
      //! \param d  Data to be corrected

      static void data_correction( double, struct solution_data& );

      //! \brief A routine to calulate the value of a normally distrubted
      //!        value.
      //! \param sigma Standard deviation of the distribution
      //! \param mean  Mean value of the distribution
      //! \param x     Point where to compute desired value
      static double normal_distribution( double, double, double );

      //! \brief Calculate the time correction in a run due to acceleration
      //!        of the rotor.
      //! \param dataList The list of editedData for the run

      static double time_correction( const QList< US_DataIO::editedData >& );
};
#endif

