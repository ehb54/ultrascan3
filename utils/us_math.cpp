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
   // a temperature of 25C.  Adjust th evaues to 20C and the current 
   // temperature.
   
   pep.vbar20 = ( pep.weight / pep.mw ) - 0.002125;
   pep.vbar   = ( ( pep.weight / pep.mw ) + 4.25e-4 * ( temperature - 25 ) );

   // Add one water molecule for the end of the chain
   pep.mw +=  WATER_MW;
}
