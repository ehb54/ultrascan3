//! \file us_calc_smoothing.cpp
#include "us_calc_smoothing.h"

#include <cmath>
#include <string>

namespace US_Calc
{
namespace
{
// M_PI, which <cmath> does not define on every platform
constexpr double kPi = 3.14159265358979323846;
} // namespace

double normal_distribution( double sigma, double mean, double x )
{
   const double z        = ( x - mean ) / sigma;
   const double exponent = -( z * z ) / 2.0;
   return exp( exponent ) / sqrt( 2.0 * kPi * ( sigma * sigma ) );
}

std::vector<Diagnostic> gaussian_smoothing( std::vector<double>& array, int smooth )
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
                 "a smoothing width of " + std::to_string( smooth ) + " needs at least " + std::to_string( 2 * smooth - 1 ) +
                    " points; the series has " + std::to_string( points ) } };
   }

   const std::vector<double> temp_array = array;
   std::vector<double>       y_weights( smooth );

   // Standard deviation 1, mean 0:  half a Gaussian out to 2 standard
   // deviations, the other side being symmetric.
   double                    x_weight = 0.0;
   y_weights[0]                       = normal_distribution( 1.0, 0.0, x_weight );
   double increment                   = 2.0 / (double) smooth;
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
