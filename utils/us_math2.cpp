//! \file us_math2.cpp

#include <stdlib.h>
#include <math.h>
#ifdef _BF_NNLS_
#include <dlfcn.h>
#endif

#include "us_math2.h"
#include "us_constants.h"
#include "us_dataIO.h"
#include "us_matrix.h"
#include "us_settings.h"

#ifdef _BF_NNLS_
class libnnls
{
public:
   typedef int (*nnls_fn) ( double *, int, int, int, double *, double *,
                            double *, double *, double *, int * );

   nnls_fn nnls = NULL;

   libnnls ()
   {
      qDebug() << "BF-NNLS: Trying to load libnnls";

      lib = dlopen( "libnnls.so", RTLD_NOW );
      if ( lib == NULL ) {
         qDebug() << "BF-NNLS: Unable to load libnnls";
         return;
      }

      nnls = (nnls_fn) dlsym( lib, "nnls" );
      if ( nnls == NULL ) {
         qDebug() << "BF-NNLS: Unable to find nnls symbol inside libnnls";
         return;
      }

      qDebug() << "BF-NNLS: libnnls successfully loaded";
   }

   ~libnnls ()
   {
      if ( lib != NULL )
         dlclose( lib );
   }

private:
   void *lib = NULL;

};

static libnnls libnnls0;
#endif
/*  The function implements the Box-Muller algorithm for generating
 *  pairs of independent standard normally distributed (zero expectation, 
 *  unit variance) random numbers, given a source of uniformly distributed 
 *  random numbers.

 *  The function returns the input mean value modified by the standard
 *  deviation weighted by a random normally distributed increment.

*/

double US_Math2::box_muller( double m, double s )   
{
   static bool  use_last = false;

   double        x1;
   double        x2;
   double        w;
   double        y1;
   static double y2;

   if ( use_last )  // Use value from previous call 
   {
      y1       = y2;
      use_last = false;
   }
   else
   {
      do
      {
         x1 = 2.0 * ranf() - 1.0;
         x2 = 2.0 * ranf() - 1.0;
         w = sq( x1 ) + sq( x2 );
      } while ( w >= 1.0 );

      w        = sqrt( ( -2.0 * log( w ) ) / w );
      y1       = x1 * w;
      y2       = x2 * w;
      use_last = true;
   }

   return m + y1 * s;
}

// This function returns a randomly distributed double value R
//  such that 0.0 <= R < 1.0
double US_Math2::ranf( void )
{
   return  (double)qrand() / ( (double)RAND_MAX + 1.0 );
}


double US_Math2::linefit( double** x       , double** y    , double* slope, 
                          double* intercept, double*  sigma, double* correlation, 
                          int     arraysize )
{
   double sumx      = 0.0;
   double sumy      = 0.0;
   double sumxy     = 0.0;
   double sumx_sq   = 0.0;
   double sumy_sq   = 0.0;
   double sumchi_sq = 0.0;
   double average;
   
   for ( int i = 0; i < arraysize; i++ ) sumy += (*y)[ i ]; 
   
   average = sumy / arraysize;
   
   for ( int i = 0; i < arraysize; i++ )
   {
      sumx      += (*x)[ i ];
      sumxy     += (*x)[ i ] * (*y)[ i ];
      sumx_sq   += (*x)[ i ] * (*x)[ i ];
      sumy_sq   += (*y)[ i ] * (*y)[ i ];

      sumchi_sq += ( (*y)[ i ] - average ) * ( (*y)[ i ] - average );
   }

   *slope       = ( arraysize * sumxy   - sumx * sumy ) / 
                  ( arraysize * sumx_sq - sumx * sumx );

   *intercept   = ( sumy - *slope * sumx ) / arraysize;
   
   *correlation = ( arraysize * sumxy - sumx * sumy ) /
                  ( sqrt( arraysize * sumx_sq - sumx * sumx ) * 
                    sqrt( arraysize * sumy_sq - sumy * sumy )
                  );
   
   *sigma       = sqrt( sumchi_sq ) / arraysize;
   
   return average;
}
/*
float US_Math2::linefit( float** x        , float** y    , float* slope, 
                        float*  intercept, float*  sigma, float* correlation, 
                        int     arraysize )
{
   float sumx      = 0.0;
   float sumy      = 0.0;
   float sumxy     = 0.0;
   float sumx_sq   = 0.0;
   float sumy_sq   = 0.0;
   float sumchi_sq = 0.0;
   float average;
   
   for ( int i = 0; i < arraysize; i++ )  sumy += (*y)[ i ];

   average = sumy / arraysize;

   for ( int i = 0; i < arraysize; i++ )
   {
      sumx    += (*x)[ i ];
      sumxy   += (*x)[ i ] * (*y)[ i ];
      sumx_sq += (*x)[ i ] * (*x)[ i ];
      sumy_sq += (*y)[ i ] * (*y)[ i ];

      sumchi_sq += ( (*y)[ i ] - average ) * ( (*y)[ i ] - average );
   }

   *slope       = ( arraysize * sumxy   - sumx * sumy ) /
                  ( arraysize * sumx_sq - sumx * sumx );
   
   *intercept   = ( sumy - *slope * sumx ) / arraysize;

   *correlation = ( arraysize * sumxy - sumx * sumy ) / 
                  ( sqrt( arraysize * sumx_sq - sumx * sumx ) * 
                    sqrt( arraysize * sumy_sq - sumy * sumy ) );

   *sigma       = sqrt( sumchi_sq ) / arraysize;
   
   return average;
}
*/

// This function determines the point on a curve that is nearest to
// a given point. The returned point may be an exact curve point or one
// interpolated between the two nearest curve points. An optional value
// may be returned which is the exact or interpolated value in a third
// dimension or associated array.

int US_Math2::nearest_curve_point( double* xs, double* ys, const int npoints,
      bool interp,   double& xgiven, double& ygiven,
      double* xnear, double* ynear,  double* zs, double* znear )
{
   int    jmin   = npoints / 2;
   int    jend   = npoints - 1;
   double xnorm  = qAbs( xs[ 0 ] - xs[ jend ] );  // X normalizing factor
   double ynorm  = qAbs( ys[ 0 ] - ys[ jend ] );  // Y normalizing factor
          xnorm  = ( xnorm == 0.0 ) ? 1.0 : ( 1.0 / xnorm );
          ynorm  = ( ynorm == 0.0 ) ? 1.0 : ( 1.0 / ynorm );
   double xdif   = ( xs[ 0 ] - xgiven ) * xnorm;
   double ydif   = ( ys[ 0 ] - ygiven ) * ynorm;
   double dmin   = sqrt( sq( xdif ) + sq( ydif ) );

   // Find the nearest point to the given point
   for ( int jj = 0; jj < npoints; jj++ )
   {
      xdif          = ( xs[ jj ] - xgiven ) * xnorm;
      ydif          = ( ys[ jj ] - ygiven ) * ynorm;
      double dval   = sqrt( sq( xdif ) + sq( ydif ) );

      if ( dval < dmin )
      {
         dmin          = dval;
         jmin          = jj;
      }
   }

   if ( interp )
   {  // Interpolate a curve point between nearest point and an adjacent one
      int    jmin1  = jmin - 1;   // Index to point preceding the nearest
      int    jmin2  = jmin + 1;   // Index to point following the nearest
      double dmin1  = dmin;
      double dmin2  = dmin;

      if ( jmin == 0 )
      {  // If nearest is first on curve, use the following point
         xdif          = ( xs[ jmin2 ] - xgiven ) * xnorm;
         ydif          = ( ys[ jmin2 ] - ygiven ) * ynorm;
         dmin2         = sqrt( sq( xdif ) + sq( ydif ) );
      }

      else if ( jmin2 == npoints )
      {  // If nearest is last on curve, use the preceding point
         jmin2         = jmin1;
         xdif          = ( xs[ jmin2 ] - xgiven ) * xnorm;
         ydif          = ( ys[ jmin2 ] - ygiven ) * ynorm;
         dmin2         = sqrt( sq( xdif ) + sq( ydif ) );
      }

      else
      {  // If the nearest point is somewhere in the middle of the curve,
         // determine whether to pick the adjacent point from the one
         // following or the one preceding. Do this by finding which of the
         // two gives the closest path to the curve.
         xdif          = ( xs[ jmin1 ] - xgiven ) * xnorm;
         ydif          = ( ys[ jmin1 ] - ygiven ) * ynorm;
         dmin1         = sqrt( sq( xdif ) + sq( ydif ) );  // Dist of preceding
         xdif          = ( xs[ jmin2 ] - xgiven ) * xnorm;
         ydif          = ( ys[ jmin2 ] - ygiven ) * ynorm;
         dmin2         = sqrt( sq( xdif ) + sq( ydif ) );  // Dist of following
         double ratio  = dmin1 / ( dmin + dmin1 );
         double ratio2 = 1.0 - ratio;
         double x1     = xs[ jmin ] * ratio + xs[ jmin1 ] * ratio2;
         double y1     = ys[ jmin ] * ratio + ys[ jmin1 ] * ratio2;
         ratio         = dmin2 / ( dmin + dmin2 );
         ratio2        = 1.0 - ratio;
         double x2     = xs[ jmin ] * ratio + xs[ jmin2 ] * ratio2;
         double y2     = ys[ jmin ] * ratio + ys[ jmin2 ] * ratio2;
         xdif          = ( x1 - xgiven ) * xnorm;
         ydif          = ( y1 - ygiven ) * ynorm;
         double dmin1g = sqrt( sq( xdif ) + sq( ydif ) );
         xdif          = ( x2 - xgiven ) * xnorm;
         ydif          = ( y2 - ygiven ) * ynorm;
         double dmin2g = sqrt( sq( xdif ) + sq( ydif ) );

         if ( dmin2g > dmin1g )
         {  // Where the test perpendicular towards the preceding point is
            // closer to the given point, choose that preceding point to
            // use as adjacent. Otherwise, default to the point following.
            dmin2         = dmin1;
            jmin2         = jmin1;
         }
      }

      // Determine ratio values based on relative distances of the nearest
      // curve point and a curve point adjacent to it.
      double ratio  = dmin2 / ( dmin + dmin2 );
      double ratio2 = 1.0 - ratio;

      if ( xnear != NULL  &&  ynear != NULL )
      {  // Interpolate the curve point nearest to the given point
         *xnear        = xs[ jmin ] * ratio + xs[ jmin2 ] * ratio2;
         *ynear        = ys[ jmin ] * ratio + ys[ jmin2 ] * ratio2;
      }

      if ( zs != NULL  &&  znear != NULL )
      {  // Interpolate the additional value associated with the nearest point
         *znear        = zs[ jmin ] * ratio + zs[ jmin2 ] * ratio2;
      }
   }

   else
   {  // Return the exact point found to be the nearest
      if ( xnear != NULL  &&  ynear != NULL )
      {  // Return the curve point nearest to the given point
         *xnear        = xs[ jmin ];
         *ynear        = ys[ jmin ];
      }

      if ( zs != NULL  &&  znear != NULL )
      {  // If desired, return the additional value associated with the point
         *znear        = zs[ jmin ];
      }
   }

   return jmin;
}

// This function determines the intersection point of two lines.
//   This version takes as input the slopes and intercepts of the two lines.
bool US_Math2::intersect( double& slope1, double& intcp1,
      double& slope2, double& intcp2, double* xisec, double* yisec )
{
   bool isect    = ( slope1 != slope2 );  // Parallel lines do not intersect!

   if ( isect )
   {  // Return intersection point of non-parallel lines
      *xisec        = ( intcp2 - intcp1 ) / ( slope1 - slope2 );
      *yisec        = *xisec * slope1 + intcp1;
   }

   return isect;
}

// This function determines the intersection point of two fitted lines.
//   This version takes as input the x and y arrays of two curves
//   and returns the intersection of straight lines fitted to them.
bool US_Math2::intersect( double* x1s, double* y1s, int npoint1,
      double* x2s, double* y2s, int npoint2, double* xisec, double* yisec )
{
   double slope1;
   double intcp1;
   double slope2;
   double intcp2;
   double sigma;
   double corre;

   // Compute the slope and intercept of a line fitted to the first curve
   US_Math2::linefit( &x1s, &y1s, &slope1, &intcp1, &sigma, &corre, npoint1 );

   // Compute the slope and intercept of a line fitted to the second curve
   US_Math2::linefit( &x2s, &y2s, &slope2, &intcp2, &sigma, &corre, npoint2 );

   // Use slopes and intercepts to compute and return the intersection point
   bool isect = intersect( slope1, intcp1, slope2, intcp2, xisec, yisec );

   return isect;
}

void US_Math2::calc_vbar( Peptide& pep, const QString& sequence, 
      double temperature )
{
   pep.vbar_sum = 0.0;
   pep.mw       = 0.0;
   pep.weight   = 0.0;
   pep.e280     = 0.0;
   pep.vbar20   = 0.0;
   pep.vbar     = 0.0;

   pep.a   = sequence.count( "a", Qt::CaseInsensitive );
   pep.b   = sequence.count( "b", Qt::CaseInsensitive );
   pep.c   = sequence.count( "c", Qt::CaseInsensitive );
   pep.d   = sequence.count( "d", Qt::CaseInsensitive );
   pep.e   = sequence.count( "e", Qt::CaseInsensitive );
   pep.f   = sequence.count( "f", Qt::CaseInsensitive );
   pep.g   = sequence.count( "g", Qt::CaseInsensitive );
   pep.h   = sequence.count( "h", Qt::CaseInsensitive );
   pep.i   = sequence.count( "i", Qt::CaseInsensitive );
   pep.j   = sequence.count( "j", Qt::CaseInsensitive );
   pep.k   = sequence.count( "k", Qt::CaseInsensitive );
   pep.l   = sequence.count( "l", Qt::CaseInsensitive );
   pep.m   = sequence.count( "m", Qt::CaseInsensitive );
   pep.n   = sequence.count( "n", Qt::CaseInsensitive );
   pep.o   = sequence.count( "o", Qt::CaseInsensitive );
   pep.p   = sequence.count( "p", Qt::CaseInsensitive );
   pep.q   = sequence.count( "q", Qt::CaseInsensitive );
   pep.r   = sequence.count( "r", Qt::CaseInsensitive );
   pep.s   = sequence.count( "s", Qt::CaseInsensitive );
   pep.t   = sequence.count( "t", Qt::CaseInsensitive );
   
   pep.v   = sequence.count( "v", Qt::CaseInsensitive );
   pep.w   = sequence.count( "w", Qt::CaseInsensitive );
   pep.x   = sequence.count( "x", Qt::CaseInsensitive );
   pep.x  += sequence.count( "?" );
   pep.y   = sequence.count( "y", Qt::CaseInsensitive );
   pep.z   = sequence.count( "z", Qt::CaseInsensitive );
   pep.dab = sequence.count( "+" );
   pep.dpr = sequence.count( "@" );

   pep.residues = pep.a + pep.b + pep.c + pep.d + pep.e + pep.f +
                  pep.g + pep.h + pep.i + pep.j + pep.k + pep.l +
                  pep.m + pep.n + pep.o + pep.p + pep.q + pep.r +
                  pep.s + pep.t +         pep.v + pep.w + pep.x +
                  pep.y + pep.z + pep.dab + pep.dpr; 
   
   pep.mw       += pep.a * ALANINE_MW;
   pep.weight   += pep.a * ALANINE_MW * ALANINE_VBAR;
   pep.vbar_sum += pep.a * ALANINE_VBAR;

   pep.mw       += pep.b * M_OR_D_MW;
   pep.weight   += pep.b * M_OR_D_MW * M_OR_D_VBAR;
   pep.vbar_sum += pep.b * M_OR_D_VBAR;

   pep.mw       += pep.c * CYSTEINE_MW;
   pep.weight   += pep.c * CYSTEINE_MW * CYSTEINE_VBAR;
   pep.vbar_sum += pep.c * CYSTEINE_VBAR;
   pep.e280     += pep.c * CYSTEINE_E280;

   pep.mw       += pep.d * ASPARTIC_ACID_MW;
   pep.weight   += pep.d * ASPARTIC_ACID_MW * ASPARTIC_ACID_VBAR;
   pep.vbar_sum += pep.d * ASPARTIC_ACID_VBAR;

   pep.mw       += pep.e * GLUTAMIC_ACID_MW;
   pep.weight   += pep.e * GLUTAMIC_ACID_MW * GLUTAMIC_ACID_VBAr;
   pep.vbar_sum += pep.e * GLUTAMIC_ACID_VBAr;

   pep.mw       += pep.f * PHENYLALANINE_MW;
   pep.weight   += pep.f * PHENYLALANINE_MW * PHENYLALANINE_VBAR;
   pep.vbar_sum += pep.f * PHENYLALANINE_VBAR;

   pep.mw       += pep.g * GLYCINE_MW;
   pep.weight   += pep.g * GLYCINE_MW * GLYCINE_VBAR;
   pep.vbar_sum += pep.g * GLYCINE_VBAR;

   pep.mw       += pep.h * HISTIDINE_MW;
   pep.weight   += pep.h * HISTIDINE_MW * HISTIDINE_VBAR;
   pep.vbar_sum += pep.h * HISTIDINE_VBAR;

   pep.mw       += pep.i * ISOLEUCINE_MW;
   pep.weight   += pep.i * ISOLEUCINE_MW * ISOLEUCINE_VBAR;
   pep.vbar_sum += pep.i * ISOLEUCINE_VBAR;

   pep.mw       += pep.j * HAO_MW; // James Nowick: Hao
   pep.weight   += pep.j * HAO_MW * HAO_VBAR;
   pep.vbar_sum += pep.j * HAO_VBAR;
   pep.e280     += pep.j * HAO_E280;

   // John Kulp: diaminobutyric acid
   pep.mw       += pep.dab * DIAMINOBUTYRIC_ACID_MW;
   pep.weight   += pep.dab * DIAMINOBUTYRIC_ACID_MW * DIAMINOBUTYRIC_ACID_VBAR;
   pep.vbar_sum += pep.dab * DIAMINOBUTYRIC_ACID_VBAR;

   // John Kulp: diaminopropanoic acid
   pep.mw       += pep.dpr * DIAMINOPROPANOIC_ACID_MW;
   pep.weight   += pep.dpr * DIAMINOPROPANOIC_ACID_MW * DIAMINOPROPANOIC_ACID_VBAR;
   pep.vbar_sum += pep.dpr * DIAMINOPROPANOIC_ACID_VBAR;

   pep.mw       += pep.k * LYSINE_MW;
   pep.weight   += pep.k * LYSINE_MW * LYSINE_VBAR;
   pep.vbar_sum += pep.k * LYSINE_VBAR;

   pep.mw       += pep.l * LEUCINE_MW;
   pep.weight   += pep.l * LEUCINE_MW * LEUCINE_VBAR;
   pep.vbar_sum += pep.l * LEUCINE_VBAR;

   pep.mw       += pep.m * METHIONINE_MW;
   pep.weight   += pep.m * METHIONINE_MW * METHIONINE_VBAR;
   pep.vbar_sum += pep.m * METHIONINE_VBAR;

   pep.mw       += pep.n * ASPARAGINE_MW;
   pep.weight   += pep.n * ASPARAGINE_MW * ASPARAGINE_VBAR;
   pep.vbar_sum += pep.n * ASPARAGINE_VBAR;

   // James Nowick: delta-linked Ornithine
   pep.mw       += pep.o * ORNITHIN_MW;
   pep.weight   += pep.o * ORNITHIN_MW * ORNITHIN_VBAR;
   pep.vbar_sum += pep.o * ORNITHIN_VBAR;
   pep.e280     += pep.o * ORNITHIN_E280;

   pep.mw       += pep.p * PROLINE_MW;
   pep.weight   += pep.p * PROLINE_MW * PROLINE_VBAR;
   pep.vbar_sum += pep.p * PROLINE_VBAR;

   pep.mw       += pep.q * GLUTAMINE_MW;
   pep.weight   += pep.q * GLUTAMINE_MW * GLUTAMINE_VBAR;
   pep.vbar_sum += pep.q * GLUTAMINE_VBAR;

   pep.mw       += pep.r * ARGININE_MW;
   pep.weight   += pep.r * ARGININE_MW * ARGININE_VBAR;
   pep.vbar_sum += pep.r * ARGININE_VBAR;

   pep.mw       += pep.s * SERINE_MW;
   pep.weight   += pep.s * SERINE_MW * SERINE_VBAR;
   pep.vbar_sum += pep.s * SERINE_VBAR;

   pep.mw       += pep.t * THREONINE_MW;
   pep.weight   += pep.t * THREONINE_MW * THREONINE_VBAR;
   pep.vbar_sum += pep.t * THREONINE_VBAR;

   pep.mw       += pep.v * VALINE_MW;
   pep.weight   += pep.v * VALINE_MW * VALINE_VBAR;
   pep.vbar_sum += pep.v * VALINE_VBAR;

   pep.mw       += pep.w * TRYPTOPHAN_MW;
   pep.weight   += pep.w * TRYPTOPHAN_MW * TRYPTOPHAN_VBAR;
   pep.vbar_sum += pep.w * TRYPTOPHAN_VBAR;
   pep.e280     += pep.w * TRYPTOPHAN_E280;

   pep.mw       += pep.x * UNK_MW; // Using an average
   pep.weight   += pep.x * UNK_MW * UNK_VBAR;
   pep.vbar_sum += pep.x * UNK_VBAR;

   pep.mw       += pep.y * TYROSINE_MW;
   pep.weight   += pep.y * TYROSINE_MW * TYROSINE_VBAR;
   pep.vbar_sum += pep.y * TYROSINE_VBAR;
   pep.e280     += pep.y * TYROSINE_E280;

   pep.mw       += pep.z * E_OR_Q_MW;
   pep.weight   += pep.z * E_OR_Q_MW * E_OR_Q_VBAR;
   pep.vbar_sum += pep.z * E_OR_Q_VBAR;

   // These equations are empirically derived.  The values above are for
   // a temperature of 25C.  Adjust the values to 20C from the current 
   // temperature.
   
   if ( pep.mw > 0.0 )
   {
      pep.vbar20 = ( pep.weight / pep.mw ) - 0.002125;
      pep.vbar   = ( ( pep.weight / pep.mw ) + 4.25e-4 * ( temperature - 25 ) );

      // Add one water molecule for the end of the chain
      pep.mw +=  WATER_MW;
   }
}

void US_Math2::data_correction( double t, SolutionData& d )
{
#ifdef OLD_WATER_MODEL
   double xi_max =    1.000028e-3  ;
   double c0     =  999.83952      ; 
   double c1     =   16.945176     ;
   double c2     =   -7.9870401e-3 ;
   double c3     =  -46.170461e-6  ;
   double c4     =  105.56302e-9   ;
   double c5     = -280.54253e-12  ;
   double b      =   16.879850e-3  ;

   double t2 = t * t;
   double t3 = t * t2;
   double t4 = t * t3;
   double t5 = t * t4;
//if ( qAbs(d.vbar-0.72) > 0.001 )
//qDebug() << "M2:dacor: t" << t << "dens visc" << d.density << d.viscosity
// << "vbar20 vbartb" << d.vbar20 << d.vbar;
  
   /*!
   The density of water.  An empirical equation derived from
   density vs temperature data. See Kell, J. Chem. & Eng. Data
   1975, 20.

   \f[  \rho_t = \xi_{max}{{c_0 + c_1 * t + c_2 * t^2 + c_3 * t^3 + c_4 * t^4 + c_5 * t^5}
                 \over{ 1 + bt }}
   \f]

   where:<br>
     \f$ \xi_{max} =   1.000028   \times 10^{-3} \f$ <br>
     \f$ c_0 =  999.83952                  \f$ <br>
     \f$ c_1 =   16.945176                 \f$ <br>
     \f$ c_2 =   -7.9870401 \times 10^{-3} \f$ <br>
     \f$ c_3 =  -46.170461  \times 10^{-6} \f$ <br>
     \f$ c_4 =  105.56302   \times 10^{-9} \f$ <br>
     \f$ c_5 = -280.54253   \times 10^{-12}\f$ <br>
     \f$ b   = 16.879850    \times 10^{-3}\f$ <br>
   */

   d.density_wt = 
      xi_max * ( c0  + c1 * t + c2 * t2 + c3 * t3 + c4 * t4 + c5 * t5 ) /
         ( 1.0 + b * t );

   /*!
      Viscosity of water. <br>
      Implements an empirical equation taken from CRC Handbook of
      Chemistry and Physics, 55th Edition.<br><br>
      For temperature 0 to 20 degress C:

      \f[  \log_{10} \eta_t ={ c_0 \over{ c_1 + c_2 ( t-20 )
               + c_3 ( t - 20 )^2 }} - c_4
      \f]

      where:<br>
      \f$ \eta \f$ = viscosity in centipoise ( gm / sec-cm / 100 )<br>
      \f$ t \f$ = temperature ( Celcius )<br>
      \f$ c_0 = 1301\f$<br>
      \f$ c_1 = 998.333\f$<br>
      \f$ c_2 = 8.1855\f$<br>
      \f$ c_3 = 0.00585\f$<br>
      \f$ c_4 = 3.30233\f$

      For temperature 20 to 100 degrees C:

      \f[
           \log_{10}{\eta_t\over\eta_{20}} = { {c_1(20 - t) -c_2(t-20)^2}\over {t+c_3}}
      \f]

      where:<br>
      \f$ \eta \f$ = viscosity in centipoise ( gm / sec-cm / 100 )<br>
      \f$ t \f$ = temperature ( Celcius )<br>
      \f$ c_1 = 1.3272\f$<br>
      \f$ c_2 = 1.053 \times 10^{-3}\f$<br>
      \f$ c_3 = 105\f$
   */

   double exponent;
   double t20;

   if ( t < 20.0 )
   {
      c0 = 1301.0;
      c1 =  998.333;
      c2 =    8.1855; 
      c3 =    0.00585;
      c4 =    3.30233;

      t20 = t - 20.0;

      exponent = ( c0 / ( c1 + c2 * t20 + c3 * sq( t20 ) ) ) - c4;

      d.viscosity_wt = 100.0 * pow( 10.0, exponent );
   }
   else
   {
      c1 =   1.3272;
      c2 =   1.053e-3;
      c3 = 105.0;

      t20 = 20.0 - t;

      exponent = ( c1 * t20 - c2 * sq( t20 ) ) / ( t + c3 );
      
      d.viscosity_wt = VISC_20W * pow( 10.0, exponent );
   }
#else
   /*
   Reference:
   Philo, J. S. (2023). SEDNTERP: a calculation and database utility to aid interpretation of
   analytical ultracentrifugation and light scattering data. European Biophysics Journal, 1-34.
   Equations 21 and 30
   */

   const double param[13][4] = {{ 1.93763157e-2, 0,  5.78545292e-3, 1},
                                { 6.74458446e3 , 4, -1.53195665e-2, 2},
                                {-2.22521604e5,  5,  3.11337859e-2, 3},
                                { 1.00231247e8,  7, -4.23546241e-2, 4},
                                {-1.63552118e9,  8,  3.38713507e-2, 5},
                                { 8.32299658e9,  9, -1.19946761e-2, 6},
                                {-7.5245878e-6,  1, -3.1091470e-6,  1},
                                {-1.3767418e-2,  3,  2.8964919e-5,  3},
                                { 1.0627293e1,   5, -1.3112763e-4,  4},
                                {-2.0457795e2,   6,  3.0410453e-4,  5},
                                { 1.2037414e3,   7, -3.9034594e-4,  6},
                                { 0,             0,  2.3403117e-4,  7},
                                { 0,             0, -4.8510101e-5,  9}};
   double R = 461.51805;
   double Ta = 593.0;
   double Tb = 232.0;
   double alpha = 10.0 / (Ta - t - K0);
   double beta = 10.0 / (t + K0 - Tb);
   double v0_c = 0;
   for (int ii = 0; ii < 6; ii++){
      v0_c += param[ii][0] * qPow(alpha, param[ii][1]);
      v0_c += param[ii][2] * qPow(beta, param[ii][3]);
   }

   double V0 = v0_c * 100.0 * R;
   v0_c = 0;
   for (int ii = 6; ii < 13; ii++){
      v0_c += param[ii][0] * qPow(alpha, param[ii][1]);
      v0_c += param[ii][2] * qPow(beta, param[ii][3]);
   }
   V0 += v0_c * R * 1000.0 * 0.01325;
   d.density_wt = 1000.0 / V0;

   double a1 = 280.68;
   double b1 = -1.9;
   double a2 = 511.45;
   double b2 = -7.7;
   double a3 = 61.131;
   double b3 = -19.6;
   double a4 = 0.45903;
   double b4 = -40.0;
   double T = (t + K0) / 300.0;
   d.viscosity_wt = 1e-3 * (a1 * qPow(T, b1) + a2 * qPow(T, b2) + a3 * qPow(T, b3) + a4 * qPow(T, b4));
#endif

   if ( d.manual )
   {
      d.density_tb      = d.density;
      d.viscosity_tb    = d.viscosity;
   }
   else
   {
      d.density_tb      = d.density * d.density_wt / DENS_20W;
      d.viscosity_tb    = d.viscosity * d.viscosity_wt / VISC_20W;
   }
   d.buoyancyb       = 1.0 - d.vbar * d.density_tb;
   d.buoyancyw       = 1.0 - d.vbar20 * DENS_20W;
   d.s20w_correction = ( d.buoyancyw / d.buoyancyb ) * ( d.viscosity_tb / VISC_20W );

   double K          = t + K0;

   d.D20w_correction = ( K20 / K ) * ( d.viscosity_tb / VISC_20W );

DbgLv(1) << QObject::tr("dataCorr: manual: %1").arg(d.manual) << d.manual;
DbgLv(1) << QObject::tr("dataCorr:  water: visc(20)=%1, visc(%2)=%3").arg(VISC_20W).arg(t).arg(d.viscosity_wt);
DbgLv(1) << QObject::tr("dataCorr:  water: dens(20)=%1, dens(%2)=%3").arg(DENS_20W).arg(t).arg(d.density_wt);
DbgLv(1) << QObject::tr("dataCorr: buffer: visc(20)=%1, visc(%2)=%3").arg(d.viscosity).arg(t).arg(d.viscosity_tb);
DbgLv(1) << QObject::tr("dataCorr: buffer: dens(20)=%1, dens(%2)=%3").arg(d.density).arg(t).arg(d.density_tb);
DbgLv(1) << QObject::tr("dataCorr: solute: vbar(20)=%1, vbar(%2)=%3").arg(d.vbar20).arg(t).arg(d.vbar);
DbgLv(1) << QObject::tr("dataCorr: boyncy: water(20)=%1, buffer(%2)=%3").arg(d.buoyancyw).arg(t).arg(d.buoyancyb);
DbgLv(1) << QObject::tr("dataCorr: factor: sed=%1, diff=%2").arg(d.s20w_correction).arg(d.D20w_correction) << "\n";

//if ( qAbs(d.vbar-0.72) > 0.001 ) {
//qDebug() << "M2:dacor:  denstb denswt" << d.density_tb << d.density_wt << "manual" << d.manual;
//qDebug() << "M2:dacor:  visctb viscwt" << d.viscosity_tb << d.viscosity_wt;
//qDebug() << "M2:dacor:  buoyb buoyw" << d.buoyancyb << d.buoyancyw;
//qDebug() << "M2:dacor:    vbtb vb20" << d.vbar << d.vbar20;
//qDebug() << "M2:dacor:    dentb den20 den" << d.density_tb << DENS_20W << d.density;
//qDebug() << "M2:dacor:    vistb vis20 vis" << d.viscosity_tb << VISC_20W << d.viscosity;
//qDebug() << "M2:dacor:   scorr dcorr" << d.s20w_correction << d.D20w_correction; }

}

double US_Math2::normal_distribution( double sigma, double mean, double x )
{
   double exponent = -sq( ( x - mean ) / sigma ) / 2.0;
   return exp( exponent ) / sqrt( 2.0 * M_PI * sq( sigma ) );
}

double US_Math2::time_correction(
      const QVector< US_DataIO::EditedData >& dataList )
{
   int size  = dataList[ 0 ].scanData.size();

   for ( int ii = 1; ii < dataList.size(); ii++ )
      size += dataList[ ii ].scanData.size();

   int count = 0;

   QVector< double > vecx( size );
   QVector< double > vecy( size );
   double* xx = vecx.data();
   double* yy = vecy.data();
   
   double cc[ 2 ];  // Looking for a linear fit

   for ( int ii = 0; ii < dataList.size(); ii++ )
   {
      const US_DataIO::EditedData* ee = &dataList[ ii ];

      for ( int jj = 0; jj < ee->scanData.size(); jj++ )
      {
        xx[ count ] = ee->scanData[ jj ].omega2t;
        yy[ count ] = ee->scanData[ jj ].seconds;
        count++;
      }
   }

   US_Matrix::lsfit( cc, xx, yy, count, 2 );

   return cc[ 0 ]; // Return the time value corresponding to zero omega2t
}

double US_Math2::time_correction(
      const QVector< US_DataIO::RawData >& dataList )
{
   int size  = dataList[ 0 ].scanData.size();

   for ( int ii = 1; ii < dataList.size(); ii++ )
      size += dataList[ ii ].scanData.size();

   int count = 0;

   QVector< double > vecx( size );
   QVector< double > vecy( size );
   double* xx = vecx.data();
   double* yy = vecy.data();
   
   double cc[ 2 ];  // Looking for a linear fit

   for ( int ii = 0; ii < dataList.size(); ii++ )
   {
      const US_DataIO::RawData* ee = &dataList[ ii ];

      for ( int jj = 0; jj < ee->scanData.size(); jj++ )
      {
        xx[ count ] = ee->scanData[ jj ].omega2t;
        yy[ count ] = ee->scanData[ jj ].seconds;
        count++;
      }
   }

   US_Matrix::lsfit( cc, xx, yy, count, 2 );

   return cc[ 0 ]; // Return the time value corresponding to zero omega2t
}

uint US_Math2::randomize( void )
{
   QTime now = QTime::currentTime();

   uint seed = now.msec() 
             + now.second() *    1000 
             + now.minute() *   60000 
             + now.hour()   * 3600000;

#ifdef UNIX
   seed -= getpid();
#endif

   qsrand( seed );
   return seed;
}

uint US_Math2::randomize( uint seed )
{
   if ( seed == 0 ) 
      seed = randomize();
   else
      qsrand( seed );

   return seed;
}

#ifdef WIN32
// Complementary Error Function erfc(x) with fractional error everywhere less
//  than 1.2 e 10 7.
double US_Math2::erfc( double x )
{

   double z = fabs( x );
   double t = 1.0 / ( 1.0 + 0.5 * z );

   double ans = t * exp( -z *
                     z -1.26551223 + t *
                         ( 1.00002368  + t *
                           ( 0.37409196  + t *
                             ( 0.09678418  + t *
                               ( -0.18628806 + t *
                                 ( 0.27886807  + t *
                                   ( -1.13520398 + t *
                                     ( 1.48851587  + t *
                                       ( -0.82215223 + t * 0.17087277
                                       )
                                     )
                                   )
                                 )
                               )
                             )
                           )
                         )
                       );

   return x >= 0.0 ? ans : 2.0 - ans;
}
#endif

// Non-negative least-squares matrix solution
int US_Math2::nnls( double* a, int a_dim1, int m, int n,
                    double* b,
                    double* x,
                    double* rnorm,
                    double* wp,  
                    double* zzp,
                    int*    indexp 
                  ) 
{
#ifdef _BF_NNLS_
   /* If there is an external NNLS algorithm implementation, execute it */
   qDebug() << "BF-NNLS: NNLS size (m, n) = (" << m << ", " << n << ")";
   if ( libnnls0.nnls != NULL ) {
      qDebug() << "BF-NNLS: Using libnnls";
      return libnnls0.nnls( a, a_dim1, m, n, b, x, rnorm, wp, zzp, indexp );
   }
   qDebug() << "BF-NNLS: Using regular nnls";
#endif

   int pfeas, ret = 0, iz, jz;
   double d1, d2, sm, up, ss;
   int k, j=0, l, izmax=0, ii, jj=0, ip;
   double temp, wmax, t, alpha, asave, dummy, unorm, ztest, cc;

   /* Check the parameters and data */
   if ( m <= 0 || n <= 0 || a == NULL || b == NULL || x == NULL ) return 2;

   /* Allocate memory for working space, if required */
   QVector< double > wVec;
   QVector< double > zVec;
   QVector< int >    iVec;

   double* w;
   double* zz;
   int*    index;

   if ( wp != NULL )
      w     = wp;
   else
   {
      wVec.fill( 0.0, n );
      w     = wVec.data();
   }

   if ( zzp != NULL )
      zz    = zzp;
   else
   {
      zVec.fill( 0.0, m );
      zz    = zVec.data();
   }

   if ( indexp != NULL )
      index = indexp;
   else
   {
      iVec.fill( 0, n );
      index = iVec.data();
   }

   if ( w == NULL || zz == NULL || index == NULL ) return 2;

   /* Initialize the arrays index[] and x[] */
   for( int k = 0; k < n; k++ ) 
   {
      x    [ k ] = 0.0; 
      index[ k ] = k;
   }

   int iz2   = n - 1; 
   int iz1   = 0; 
   int nsetp = 0; 
   int npp1  = 0;

   /* Main loop; quit if all coeffs are already in the solution or */
   /* if M cols of A have been triangularized */
   
   int iter  = 0; 
   int itmax = n * 3;
   int ja_dim1;

   while ( iz1 <= iz2 && nsetp < m ) 
   {
      /* Compute components of the dual (negative gradient) vector W[] */
      for ( iz = iz1; iz <= iz2; iz++ ) 
      {
         j  = index[ iz ]; 
         ja_dim1 = j * a_dim1;
         sm = 0.0; 
         
         for( l = npp1; l < m; l++ ) 
            sm += a[ l + ja_dim1 ] * b[ l ];

         w[ j ] = sm;
      }

      while ( true ) 
      {
         /* Find largest positive W[j] */
         for ( wmax = 0.0, iz = iz1; iz <= iz2; iz++ ) 
         {
            j = index[ iz ]; 
            if ( w[ j ] > wmax ) 
            {
               wmax  = w[ j ]; 
               izmax = iz;
            }
         }

         /* Terminate if wmax <= 0.0; */
         /* it indicates satisfaction of the Kuhn-Tucker conditions */
         if ( wmax <= 0.0 ) break;
         
         iz = izmax; 
         j  = index[ iz ];

         /* The sign of W[ j ] is ok for j to be moved to set P. */
         /* Begin the transformation and check new diagonal element to avoid */
         /* near linear dependence. */

         ja_dim1 = j * a_dim1;
         asave = a[ npp1 + ja_dim1 ];
         _nnls_h12( 1, npp1, npp1 + 1, m, &a[ ja_dim1 ], 
                    1, &up, &dummy, 1, 1, 0 );
         
         unorm = 0.0;

         if ( nsetp != 0 ) 
            for ( l = 0; l < nsetp; l++ ) 
            { 
               d1     = a[ l + ja_dim1 ]; 
               unorm += d1 * d1;
            }

         unorm = sqrt( unorm );

         d2 = unorm + ( d1 = a[ npp1 + ja_dim1 ], fabs( d1 ) ) * 0.01;
         
         if ( ( d2 - unorm ) > 0.0 ) 
         {
            /* Col j is sufficiently independent. Copy B into ZZ, update ZZ 
               and solve for ztest ( = proposed new value for X[ j ] ) */

            for ( l = 0; l < m; l++ ) zz[ l ] =b[ l ];

            _nnls_h12( 2, npp1, npp1 + 1, m, &a[ ja_dim1 ], 
                       1, &up, zz, 1, 1, 1 );

            ztest = zz[ npp1 ] / a[ npp1 + ja_dim1 ];

            /* See if ztest is positive */
            if ( ztest > 0.0 ) break;
         }

         /* Reject j as a candidate to be moved from set Z to set P. Restore */
         /* A[npp1,j], set W[j]=0., and loop back to test dual coeffs again */
         a[ npp1 + ja_dim1 ] = asave; 
         w[ j ]              = 0.0;

      } // while( true )
      
      if ( wmax <= 0.0 ) break;

      /* Index j = index[ iz ] has been selected to be moved from set Z to set P.
         Update B and indices, apply householder transformations to cols in 
         new set Z, zero subdiagonal elts in col j, set W[j]=0. */
      
      for ( l = 0; l < m; ++l ) b[ l ] =zz[ l ];

      index[ iz  ] = index[ iz1 ]; 
      index[ iz1 ] = j; 
      iz1++; 
      nsetp        = npp1 + 1; 
      npp1++;

      if ( iz1 <= iz2 ) 
         for ( jz = iz1; jz <= iz2; jz++ ) 
         {
            jj = index[ jz ];
            _nnls_h12( 2, nsetp - 1, npp1, m, &a[ ja_dim1 ], 
                       1, &up, &a[ jj * a_dim1 ], 1, a_dim1, 1 );
         }

      if ( nsetp !=m ) 
         for ( l = npp1; l < m; l++ ) 
            a[ l + ja_dim1 ] = 0.0;

      w[ j ]= 0.0;

      /* Solve the triangular system; store the solution temporarily in Z[] */
      for ( l = 0; l < nsetp; l++ ) 
      {
         ip = nsetp - ( l + 1 );
         
         if ( l != 0 ) 
            for ( ii = 0; ii <= ip; ii++ ) 
               zz[ ii ] -= a[ ii + jj * a_dim1 ] * zz[ ip + 1 ];

         jj        = index[ ip ]; 
         zz[ ip ] /= a[ ip + jj *a_dim1 ];
      }

      /* Secondary loop begins here */
      while ( ++iter < itmax ) 
      {
         /* See if all new constrained coeffs are feasible; 
            if not, compute alpha */
         
         for ( alpha = 2.0, ip = 0; ip < nsetp; ip++ ) 
         {
            l = index[ ip ];
            if ( zz[ ip ] <= 0.0 ) 
            {
               t = -x[ l ] / ( zz[ ip ] - x[ l ]); 
               if ( alpha > t ) 
               {
                  alpha = t; 
                  jj = ip - 1;
               }
            }
         }

         /* If all new constrained coeffs are feasible then still alpha==2. */
         /* If so, then exit from the secondary loop to main loop */
         
         if ( alpha == 2.0 ) break;

         /* Use alpha ( 0.0 < alpha < 1.0 ) to interpolate between old 
          * X and new ZZ */
         
         for ( ip = 0; ip < nsetp; ip++ ) 
         {
            l       = index[ ip ]; 
            x[ l ] += alpha * ( zz[ ip ] - x[ l ] );
         }

         /* Modify A and B and the INDEX arrays to move coefficient i */
         /* from set P to set Z. */
         
         k     = index[ jj + 1 ]; 
         pfeas = 1;

         do 
         {
            x[ k ] = 0.0;
            if ( jj != ( nsetp - 1 ) ) 
            {
               jj++;
               for ( j = jj + 1; j < nsetp; j++ ) 
               {
                  ii             = index[ j ];
                  int  iia_dim1  = ii * a_dim1;
                  index[ j - 1 ] = ii;

                  _nnls_g1( a [ j - 1 + iia_dim1 ], a[ j + iia_dim1 ], 
                            &cc, &ss, &a[ j - 1 + iia_dim1 ] );

                  for ( a[ j + iia_dim1 ] = 0.0, l = 0; l < n; l++ ) 
                     if ( l != ii ) 
                     {
                        int  jla_dim1  = j + l * a_dim1;
                        /* Apply procedure G2 (CC,SS,A(J-1,L),A(J,L)) */
                        temp = a[ jla_dim1 - 1 ];
                        a[ jla_dim1 - 1 ] =  cc * temp + ss * a[ jla_dim1 ];
                        a[ jla_dim1     ] = -ss * temp + cc * a[ jla_dim1 ];
                     }

                  /* Apply procedure G2 (CC,SS,B(J-1),B(J)) */
                  temp       =  b[ j - 1 ]; 
                  b[ j - 1 ] =  cc * temp + ss * b[ j ]; 
                  b[ j     ] = -ss * temp + cc * b[ j ];
               }
            }

            npp1 = nsetp - 1; 
            nsetp--; 
            iz1--; 
            index[ iz1 ] = k;

            /* See if the remaining coeffs in set P are feasible; they should */
            /* be because of the way alpha was determined. If any are */
            /* infeasible it is due to round-off error. Any that are */
            /* nonpositive will be set to zero and moved from set P to set Z */
            for( jj = 0; jj < nsetp; jj++ ) 
            {
               k = index[ jj ]; 
               if ( x[ k ] <= 0.0 ) 
               {
                  pfeas = 0; 
                  break;
               }
            }
         } while ( pfeas == 0 );

         /* Copy b[] into zz[], then solve again and loop back */
         for ( k = 0; k < m; k++ ) 
            zz[ k ] = b[ k ];

         for ( l = 0; l < nsetp; l++ ) 
         {
            ip = nsetp - ( l + 1 );
            if ( l != 0 ) 
               for ( ii = 0; ii <= ip; ii++ ) 
                  zz[ ii ] -= a[ ii + jj * a_dim1 ] * zz[ ip + 1 ];

            jj        = index[ ip ]; 
            zz[ ip ] /= a[ ip + jj * a_dim1 ];
         }
      } /* end of secondary loop */

      if ( iter > itmax ) 
      {
         ret = 1; 
         break;
      }

      for ( ip = 0; ip < nsetp; ip++ ) 
      {
         k      = index[ ip ]; 
         x[ k ] = zz   [ ip ];
      }
   } /* end of main loop */

   /* Compute the norm of the final residual vector */
   
   sm = 0.0;

   if ( npp1 < m ) 
      for ( k = npp1; k < m; k++ ) 
         sm += ( b[ k ] * b[ k ] );
   else 
      for( j = 0; j < n; j++ ) 
         w[ j ] = 0.0;

   if ( rnorm != NULL ) *rnorm = sqrt( sm );

   return ret;
}

/*****************************************************************************
 *
 *  Compute orthogonal rotation matrix:
 *    (C, S) so that (C, S)(A) = ( sqrt( A**2 + B**2 ) )
 *    (-S,C)         (-S,C)(B)   ( 0                 )
 *  Compute sig = sqrt(A**2+B**2):
 *    sig is computed last to allow for the possibility that sig may be in
 *    the same location as A or B.
 */
void US_Math2::_nnls_g1( double  a,     double  b, double* cterm, 
                         double* sterm, double* sig )
{
   double d1;
   double xr;
   double yr;

   if ( fabs( a ) > fabs( b ) ) 
   {
      xr = b / a; 
      d1 = xr; 
      yr = sqrt( d1 * d1 + 1.0 ); 
      d1 = 1.0 / yr;
      *cterm = ( a >= 0.0 ? fabs( d1 ) : -fabs( d1 ) );
      *sterm = ( *cterm ) * xr; 
      *sig   = fabs( a ) * yr;
   } 
   else if ( b != 0.0 ) 
   {
      xr = a / b; 
      d1 = xr; 
      yr = sqrt( d1*d1 + 1.0 ); 
      d1 = 1.0 / yr;
      *sterm = ( b >= 0.0 ? fabs( d1 ) : -fabs( d1 ) );
      *cterm = ( *sterm ) * xr; 
      *sig   = fabs( b ) * yr;
   } 
   else 
   {
      *sig   = 0.0; 
      *cterm = 0.0; 
      *sterm = 1.0;
   }
} /* _nnls_g1 */


/*****************************************************************************
 *
 *  Construction and/or application of a single Householder transformation:
 *           Q = I + U*(U**T)/B
 *
 *  Function returns 0 if succesful, or >0 in case of erroneous parameters.
 *
 */
int US_Math2::_nnls_h12 (
              int mode,
              /* mode=1 to construct and apply a Householder transformation, or
                 mode=2 to apply a previously constructed transformation */
              int lpivot,     /* Index of the pivot element */
              int l1, 
              int m,
              /* Transformation is constructed to zero elements indexed from l1 to M */
              double* u, 
              int     u_dim1, 
              double* up,
              /* With mode=1: On entry, u[] must contain the pivot vector.
                 On exit, u[] and up contain quantities defining the vector u[] of
                 the Householder transformation. */
              /* With mode=2: On entry, u[] and up should contain quantities previously
                 computed with mode=1. These will not be modified. */
              /* u_dim1 is the storage increment between elements. */
              double* cm,
              /* On entry, cm[] must contain the matrix (set of vectors) to which the
                 Householder transformation is to be applied. On exit, cm[] will contain
                 the set of transformed vectors */
              int ice,        /* Storage increment between elements of vectors in cm[] */
              int icv,        /* Storage increment between vectors in cm[] */
              int ncv         /* Nr of vectors in cm[] to be transformed;
                                 if ncv<=0, then no operations will be done on cm[] */
              ) 
{
   double d1, d2, b, clinv, cl, sm;
   int    incr, k, j, i2, i3, i4;

   /* Check parameters */
   if ( mode != 1 && mode != 2 ) return 1;

   if ( m < 1 || u == NULL || u_dim1 < 1 || cm == NULL ) return 2;

   if ( lpivot < 0 || lpivot >= l1 || l1 >= m ) return 0;

   /* Function Body */
   cl = ( d1 = u[ lpivot * u_dim1 ], fabs( d1 ) );

   if ( mode == 2 ) // Apply transformation I+U*(U**T)/B to cm[]
   { 
      if ( cl <= 0.0 ) return 0;
   } 
   else // Construct the transformation
   { 
      for ( j = l1; j < m; j++ ) 
      { /* Computing MAX */
         d2 = ( d1 = u[ j * u_dim1 ], fabs( d1 ) ); 
         if ( d2 > cl ) cl=d2;
      }

      if ( cl <= 0.0 ) return 0;

      clinv = 1.0 / cl;

      /* Computing 2nd power */
      d1 = u[ lpivot * u_dim1 ] * clinv; 
      sm = d1 * d1;
      
      for( j = l1; j < m; j++ ) 
      {
         d1  = u[ j * u_dim1 ] * clinv; 
         sm += d1 * d1;
      }
      
      cl *= sqrt( sm ); 
      if ( u[ lpivot * u_dim1 ] > 0.0 ) cl = -cl;

      *up                  = u[ lpivot * u_dim1 ] - cl; 
      u[ lpivot * u_dim1 ] = cl;
   }

   if ( ncv <= 0 ) return 0;

   b = (*up) * u[ lpivot * u_dim1 ];
   
   /* b must be nonpositive here; if b>=0., then return */
   if ( b >= 0.0 ) return 0;
   
   b    = 1.0 / b; 
   i2   = 1 -icv + ice * lpivot; 
   incr = ice * ( l1 - lpivot );
   
   for ( j = 0; j < ncv; j++ ) 
   {
      i2 += icv; 
      i3  = i2 + incr; 
      i4  = i3; 
      sm  = cm[ i2 - 1 ] * (*up);

      for ( k = l1; k < m; k++ ) 
      {
         sm += cm[ i3 - 1 ] * u[ k * u_dim1 ]; 
         i3 += ice;
      }

      if ( sm != 0.0 ) 
      {
         sm           *= b; 
         cm[ i2 - 1 ] += sm * (*up);

         for ( k = l1; k < m; k++ ) 
         {
            cm[ i4 - 1 ] += sm * u[ k * u_dim1 ]; 
            i4           += ice;
         }
      }
   }

   return 0;
} /* _nnls_h12 */

void US_Math2::gaussian_smoothing( QVector< double >& array, int smooth )
{
   if ( smooth <= 1 ) return;

   // Apply a normalized Gaussian smoothing kernel that goes out 
   // to 2 standard deviations

   int points = array.size();
   
   QVector< double > temp_array( points );
   QVector< double > y_weights( smooth );
   double            x_weight;
   
   temp_array = array;
   
   // Standard deviation = 1.0, Mean = 0;
   x_weight         = 0.0;
   y_weights[ 0 ]   = normal_distribution( 1.0, 0.0, x_weight ); 
   double increment = 2.0 / (double)smooth;
   
   // Only calculate half a Gaussian, since the other side is symmetric
   for ( int i = 1; i < smooth; i++ )
   {
      x_weight      += increment;
      y_weights[ i ] = normal_distribution( 1.0, 0.0, x_weight );
   }

   // First, take care of the left border, using an "appearing frame" algorithm,
   // starting with half a frame visible:

   for ( int j = 0; j < smooth; j++ )   // Loop over all border point centers
   {
      double sum      = 0.0;
      double sum_y    = 0.0;
      int    position = 0;

      // Sum all applicable points on the left of center
      for ( int k = j - 1; k >= 0; k-- )
      {
         position++;
         sum   += y_weights[ position ] * temp_array[ k ];  
         sum_y += y_weights[ position ];
      }

      position = 0;

      // Sum the weighted points right of center, including center
      for ( int k = j; k < j + smooth; k++ )
      {
         sum   += y_weights[ position ] * temp_array[ k ];  
         sum_y += y_weights[ position ];
         position++;
      }

      // Normalize by the sum of all weights that were used
      array[ j ] = sum / sum_y;
   }

   // Now deal with all non-border points:
   for ( int j = smooth; j < points - smooth; j++ ) 
   {
      double sum      = 0.0;
      double sum_y    = 0.0;
      int    position = 0;
      
      // Sum all applicable points on the left of center
      for ( int k = j - 1; k >= j - smooth + 1; k-- )
      {
         position++;
         sum   += y_weights[ position ] * temp_array[ k ];   
         sum_y += y_weights[ position ];
      }

      position = 0;
      
      // Sum the weighted points right of center, including center
      for ( int k = j; k < j + smooth; k++ )
      {
         sum   += y_weights[ position ] * temp_array[ k ];  
         sum_y += y_weights[ position ];
         position++;
      }

      // Normalize by the sum of all weights that were used
      array[ j ] = sum / sum_y;
   }

   // Now deal with all points from the right border, using a "disappearing
   // frame" algorithm, starting with a full frame minus 1 point visible:

   // Loop over all right-border points
   for ( int j = points - smooth; j < points; j++ )
   {
      double sum      = 0.0;
      double sum_y    = 0.0;
      int    position = 0;
      
      // Sum all points on the left of center
      for ( int k = j - 1; k >= j - smooth + 1; k-- )
      {
         position++;
         sum   += y_weights[ position ] * temp_array[ k ]; 
         sum_y += y_weights[ position ];
      }

      position = 0;

      // Right of center, including center
      for ( int k = j; k < points; k++ )
      {
         sum   += y_weights[ position ] * temp_array[ k ]; 
         sum_y += y_weights[ position ];
         position++;
      }

      // normalize by the sum of all weights that were used
      array[ j ] = sum / sum_y;
   }
}

// Calculate the weighted average of temperature-corrected vbars
double US_Math2::calcCommonVbar( US_Solution& solution, double temperature )
{
   double cvbar = TYPICAL_VBAR;
   double vbsum = 0.0;
   double wtsum = 0.0;

   for ( int ii = 0; ii < solution.analyteInfo.size(); ii++ )
   {
      double vb20 = solution.analyteInfo[ ii ].analyte.vbar20;
      // Use adjusted vbar if PROTEIN
      double vbar = solution.analyteInfo[ ii ].analyte.type == US_Analyte::PROTEIN ?
                    US_Math2::adjust_vbar20( vb20, temperature ) :
                    vb20;
      double wt   = solution.analyteInfo[ ii ].analyte.mw * solution.analyteInfo[ ii ].amount;
      vbsum      += ( vbar * wt );
      wtsum      += wt;
   }

   if ( wtsum != 0.0 )
      cvbar     = vbsum / wtsum;    // Common vbar is the weighted average

   return cvbar;
}

// Compute the best number of uniform grid repetitions for 2DSA,
// given specified initial total grid points in the 's' and 'k'
// (f/f0 or vbar) dimensions. Modify grid points slightly to insure
// they are multiples of grid repetitions and are within reasonable limits.
int US_Math2::best_grid_reps( int& ngrid_s, int& ngrid_k )
{
   const int min_grid = 10;   // Grid points at least 10
   const int max_grid = 2100; // Grid points at most 2100
   const int min_subg = 40;   // Sub-grid size at least 40
   const int max_subg = 200;  // Sub-grid size at most 200
   const int min_reps = 1;    // Repetitions at least 1
   const int max_reps = 160;  // Repetitions at most 160

qDebug() << "BGR: IN";
   // Insure grid points are within reasonable limits
   ngrid_s         = qMin( max_grid, qMax( min_grid, ngrid_s ) );
   ngrid_k         = qMin( max_grid, qMax( min_grid, ngrid_k ) );
   int ngrid_s_i   = ngrid_s;
   int ngrid_k_i   = ngrid_k;
qDebug() << "BGR: ngrid_s_i" << ngrid_s_i << "ngrid_k_i" << ngrid_k_i;

   // Build the list of repetition values that divide into s grid points
   QList< int > reps;

   for ( int jreps = min_reps; jreps <= max_reps; jreps++ )
   {
      int jgrid_s     = ( ngrid_s / jreps ) * jreps;

      if ( jgrid_s == ngrid_s )
      {  // Save a repetition that divides evenly into S grid points
         reps << jreps;
qDebug() << "BGR:   jgrid_s" << jgrid_s << "jreps" << jreps;
      }
   }

   // Find the repetitions and k grid points that work best
   int kdiff       = 99999;
   int kreps       = reps[ 0 ];
   int kgrid_k     = ngrid_k;

   for ( int ii = 0; ii < reps.count(); ii++ )
   {
      int jreps       = reps[ ii ];
      int dim_s       = ngrid_s / jreps;
      int dim_k       = ngrid_k / jreps;
      int jgrid_k     = dim_k * jreps;
      int nsubgs      = jreps * jreps;
      int subgsz      = dim_s * dim_k;
      int jdiff       = qAbs( nsubgs - subgsz );
qDebug() << "BGR:   ii" << ii << "jreps" << jreps << "jgrid_k" << jgrid_k;

      if ( jdiff < kdiff )
      {  // Count and size of subgrid closely matched
         kdiff           = jdiff;
         kreps           = jreps;
         kgrid_k         = jgrid_k;
qDebug() << "BGR:     ii" << ii << "kdiff" << kdiff;
      }
   }

   ngrid_k         = kgrid_k;
   int nreps_g     = kreps;
   int nsubg_s     = ngrid_s / nreps_g;
   int nsubg_k     = ngrid_k / nreps_g;
qDebug() << "BGR: ngrid_s" << ngrid_s << "ngrid_k ngrid_k_i" << ngrid_k << ngrid_k_i;
qDebug() << "BGR:  nsubgs" << (nreps_g*nreps_g) << "subgsz" << (nsubg_s*nsubg_k)
 << "totgsz" << (ngrid_s*ngrid_k) << "nreps_g" << nreps_g;
   
#if 0
   double grfact   = 1.0;
qDebug() << "BGR: nreps_g" << nreps_g;
   // If debug text modifies grid-rep factor, apply it
   QStringList tgrfact = US_Settings::debug_text();

   for ( int ii = 0; ii < tgrfact.count(); ii++ )
   {  // If debug text modifies grid-rep factor, apply it
      if ( tgrfact[ ii ].startsWith( "grfact=" ) )
         grfact    = QString( tgrfact[ ii ]) .section( "=", 1, 1 ).toDouble();
   }
   nreps_g         = qRound( grfact * nreps_g );
qDebug() << "BGR: grfact" << grfact << "nreps_g" << nreps_g;
#endif

   // Adjust values until the product yields no more than 200 sub-grid points
   while ( ( nsubg_s * nsubg_k ) > max_subg  ||  nreps_g < min_reps )
   {  // Increase grid-reps and recompute sub-grid points
      nreps_g++;
      nsubg_s         = ngrid_s / nreps_g;
      nsubg_k         = ngrid_k / nreps_g;
   }

   // Adjust values until the product yields no less than 40 sub-grid points
   while ( ( nsubg_s * nsubg_k ) < min_subg  ||  nreps_g > max_reps )
   {  // Decrease grid-reps and recompute sub-grid points
      nreps_g--;
      nsubg_s         = ngrid_s / nreps_g;
      nsubg_k         = ngrid_k / nreps_g;
   }

   // Recalculate the total grid points in each dimension
   //  to be multiples of the grid repetitions
   ngrid_s         = nsubg_s * nreps_g;
   ngrid_k         = nsubg_k * nreps_g;

   // Return the computed number of grid repetitions
   return nreps_g;
}

// Compute the norm value for a simulation data set
double US_Math2::norm_value( US_DataIO::RawData* simdat )
{
   double normval  = 0.0;
   int npoint      = simdat->pointCount();

   for ( int ii = 0; ii < simdat->scanCount(); ii++ )
   {
      for ( int jj = 0; jj < npoint; jj++ )
      {
         double dval     = simdat->value( ii, jj );
         normval        += ( dval * dval );
      }
   }

   normval         = ( normval > 0.0 ) ? sqrt( normval ) : normval;
   return normval;
}

// Compute the norm value for an experiment data set
double US_Math2::norm_value( US_DataIO::EditedData* expdat )
{
   double normval  = 0.0;
   int npoint      = expdat->pointCount();

   for ( int ii = 0; ii < expdat->scanCount(); ii++ )
   {
      for ( int jj = 0; jj < npoint; jj++ )
      {
         double dval     = expdat->value( ii, jj );
         normval        += ( dval * dval );
      }
   }

   normval         = ( normval > 0.0 ) ? sqrt( normval ) : normval;
   return normval;
}

// Compute the norm value for a data array of doubles
double US_Math2::norm_value( double* datary, const int nval )
{
   double normval  = 0.0;

   for ( int ii = 0; ii < nval; ii++ )
   {
      normval        += ( datary[ ii ] * datary[ ii ] );
   }

   normval         = ( normval > 0.0 ) ? sqrt( normval ) : normval;
   return normval;
}

// Compute the norm value for a data vector of doubles
double US_Math2::norm_value( QVector< double >* datvec )
{
   return norm_value( datvec->data(), datvec->count() );
}

// Find root X where evaluated Y is virtually equal to a goal, using a
//  calculation including the inverse complementary error function (erfc).
double US_Math2::find_root( const double goal )
{
#define _FR_MXKNT 100            // Max find-root iteration count
   double  tolerance = 1.0e-7;   // Min difference tolerance
   double  x1        = 0.0;
   double  x2        = 10.0;
   double  xv        = 5.0;
   double  xdiff     = 2.5;
   double  xsqr      = xv * xv;
   double  rsqr_pi   = 1.0 / sqrt( M_PI );
   double  test      = exp( -xsqr ) * rsqr_pi - ( xv * erfc( xv ) );
           test      = ( goal != 0.0 ) ? test : 0.0;
   int     count     = 0;

   // Iterate until the difference between subsequent x value evaluations
   //  is too small to be relevant (or max count reached);

   while ( qAbs( test - goal ) > tolerance )
   {
      xdiff  = ( x2 - x1 ) / 2.0;

      if ( test < goal )
      { // At less than goal, adjust top (x2) limit
         x2     = xv;
         xv    -= xdiff;
      }

      else
      { // At greater than goal, adjust bottom (x1) limit
         x1     = xv;
         xv    += xdiff;
      }

      // Then update the test y-value
      xsqr   = xv * xv;
      test   = ( 1.0 + 2.0 * xsqr ) * erfc( xv ) 
         - ( 2.0 * xv * exp( -xsqr ) ) * rsqr_pi;

      if ( (++count) > _FR_MXKNT )
         break;
   }

   return xv;
}

