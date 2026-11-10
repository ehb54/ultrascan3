//! \file us_calc_smoothing.cpp
#include "us_calc_smoothing.h"
#include "us_calc_fp.h" // first:  floating-point policy

#include <QtMath>

namespace US_Calc
{
double normal_distribution( double sigma, double mean, double x )
{
   const double z        = ( x - mean ) / sigma;
   const double exponent = -( z * z ) / 2.0;
   return exp( exponent ) / sqrt( 2.0 * M_PI * ( sigma * sigma ) );
}

QVector<Diagnostic> gaussian_smoothing( QVector<double>& array, int smooth )
{
   if ( smooth <= 1 )
   {
      return {};
   }

   const int points = static_cast<int>( array.size() );
   if ( points < 2 * smooth - 1 )
   {
      return { { Issue::SeriesTooShort,
                 Severity::Error,
                 QString( "a smoothing width of %1 needs at least %2 points; the series has %3" )
                    .arg( smooth )
                    .arg( 2 * smooth - 1 )
                    .arg( points ) } };
   }

   const QVector<double> temp_array = array;
   QVector<double>       y_weights( smooth );

   // Standard deviation 1, mean 0:  half a Gaussian out to 2 standard
   // deviations, the other side being symmetric.
   double                x_weight = 0.0;
   y_weights[0]                   = normal_distribution( 1.0, 0.0, x_weight );
   double increment               = 2.0 / (double) smooth;
   for ( int i = 1; i < smooth; i++ )
   {
      x_weight += increment;
      y_weights[i] = normal_distribution( 1.0, 0.0, x_weight );
   }

   // Left border:  an "appearing frame", starting with half a frame visible.
   for ( int j = 0; j < smooth; j++ )
   {
      double sum      = 0.0;
      double sum_y    = 0.0;
      int    position = 0;

      for ( int k = j - 1; k >= 0; k-- ) // left of center
      {
         position++;
         sum += y_weights[position] * temp_array[k];
         sum_y += y_weights[position];
      }

      position = 0;
      for ( int k = j; k < j + smooth; k++ ) // center and right of it
      {
         sum += y_weights[position] * temp_array[k];
         sum_y += y_weights[position];
         position++;
      }

      array[j] = sum / sum_y;
   }

   // Points away from both borders.
   for ( int j = smooth; j < points - smooth; j++ )
   {
      double sum      = 0.0;
      double sum_y    = 0.0;
      int    position = 0;

      for ( int k = j - 1; k >= j - smooth + 1; k-- )
      {
         position++;
         sum += y_weights[position] * temp_array[k];
         sum_y += y_weights[position];
      }

      position = 0;
      for ( int k = j; k < j + smooth; k++ )
      {
         sum += y_weights[position] * temp_array[k];
         sum_y += y_weights[position];
         position++;
      }

      array[j] = sum / sum_y;
   }

   // Right border:  a "disappearing frame", starting with a full frame minus
   // one point visible.  A series of exactly 2*width-1 points has its last
   // left-border point recomputed here.
   for ( int j = points - smooth; j < points; j++ )
   {
      double sum      = 0.0;
      double sum_y    = 0.0;
      int    position = 0;

      for ( int k = j - 1; k >= j - smooth + 1; k-- )
      {
         position++;
         sum += y_weights[position] * temp_array[k];
         sum_y += y_weights[position];
      }

      position = 0;
      for ( int k = j; k < points; k++ )
      {
         sum += y_weights[position] * temp_array[k];
         sum_y += y_weights[position];
         position++;
      }

      array[j] = sum / sum_y;
   }
   return {};
}
} // namespace US_Calc
