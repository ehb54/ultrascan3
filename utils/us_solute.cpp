//! \file us_solute.cpp

#include "us_solute.h"
#include "us_math2.h"

US_Solute::US_Solute( double s0, double k0, double c0 )
{
   s = s0;
   k = k0;
   c = c0;
}

void US_Solute::init_solutes( int workers,
                              double s_min,   double s_max,   int s_res,
                              double ff0_min, double ff0_max, int ff0_res,
                              QList< QVector< US_Solute > >& solute_list )
{
   double s_step   = fabs( s_max   - s_min   ) / ( s_res   - 1 );
   double ff0_step = fabs( ff0_max - ff0_min ) / ( ff0_res - 1 );

   // Allow a 1% overscan
   s_max   += 0.01 * s_step;
   ff0_max += 0.01 * ff0_step;

   int size = min( s_res * ff0_res / workers, 100 );
   int sets = (int)ceil( ( s_res * ff0_res * 1.0 ) / size );

   QVector< US_Solute > solute_vector( s_res * ff0_res );
   int                  index = 0;

   for ( double ff0 = ff0_min; ff0 <= ff0_max; ff0 += ff0_step )
      for ( double s = s_min; s <= s_max; s += s_step )
      {
         // Omit s values close to zero.
         if ( s >= -1.0e-14  &&  s <= 1.0e-14 ) continue;

         solute_vector[ index++ ] = US_Solute( s, ff0 );
      }

   solute_list.clear();

   QVector< US_Solute > vector;

   for ( int i = 0; i < sets; i++ )
      solute_list << vector;

   index = 0;
   
   for ( int i = 0; i < solute_vector.size(); i++ )
   {
      solute_list[ index++ ] << solute_vector[ i ];
      if ( index >= sets ) index = 0;
   }
}
