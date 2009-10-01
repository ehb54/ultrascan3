//! \file us_math.cpp

#include <stdlib.h>
#include <math.h>

#include "us_math.h"
#include "us_constants.h"

double US_Math::box_muller( double m, double s )   
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
         w = x1 * x1 + x2 * x2;
      } while ( w >= 1.0 );

      w        = sqrt( ( -2.0 * log( w ) ) / w );
      y1       = x1 * w;
      y2       = x2 * w;
      use_last = true;
   }

   return m + y1 * s;
}


double US_Math::ranf( void )
{
   return  (double)rand() / ( RAND_MAX + 1 );
}


double US_Math::linefit( double** x       , double** y    , double* slope, 
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
float US_Math::linefit( float** x        , float** y    , float* slope, 
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

void US_Math::calc_vbar( struct peptide& pep, const QString& sequence, 
      double temperature )
{
   pep.vbar_sum = 0.0;
   pep.mw       = 0.0;
   pep.weight   = 0.0;
   pep.e280     = 0.0;

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
   
   pep.vbar20 = ( pep.weight / pep.mw ) - 0.002125;
   pep.vbar   = ( ( pep.weight / pep.mw ) + 4.25e-4 * ( temperature - 25 ) );

   // Add one water molecule for the end of the chain
   pep.mw +=  WATER_MW;
}

void US_Math::data_correction( double t, struct solution_data& d )
{
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
  
   /*!
   The density of water.  An empirical equation derived from
   density vs temperature data. See Krell, J. Chem. & Eng. Data
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
      \f$ c_2 = 8.1855\f$
      \f$ c_3 = 0.00585\f$
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

      exponent = ( c0 / ( c1 + c2 * t20 + c2 * sq( t20 ) ) ) - c4;

      d.viscosity_wt = 100.0 * pow( 10.0, exponent );
   }
   else
   {
      c1 =   1.3272;
      c2 =   1.053e-3;
      c3 = 105.0;

      t20 = 20.0 - t;

      exponent = ( c1 * t20 - c2 * sq( t20 ) ) / ( t + c3 );
      
      d.viscosity_wt = 100.0 * VISC_20W * pow( 10.0, exponent );
   }

   d.density_tb   = d.density * d.density_wt / DENS_20W;
   d.viscosity_tb = d.viscosity * d.viscosity_wt / ( 100.0 * VISC_20W );
   d.buoyancyb    = 1.0 - d.vbar * d.density_tb;
   d.buoyancyw    = 1.0 - d.vbar20 * DENS_20W;
   d.correction   = d.buoyancyw / d.buoyancyb * 
                    d.viscosity_tb / ( 100.0 * VISC_20W );
}

double US_Math::normal_distribution( double sigma, double mean, double x )
{
   double exponent = -sq( ( x - mean ) / sigma ) / 2.0;
   return exp( exponent ) / sqrt( 2.0 * M_PI * sq( sigma ) );
}
