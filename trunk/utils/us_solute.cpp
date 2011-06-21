//! \file us_solute.cpp

#include "us_solute.h"
#include "us_math2.h"

US_Solute::US_Solute( double s0, double k0, double c0 )
{
   s = s0;
   k = k0;
   c = c0;
}

void US_Solute::init_solutes( double s_min,   double s_max,   int s_res,
                              double ff0_min, double ff0_max, int ff0_res,
                              int    grid_reps,
                              QList< QVector< US_Solute > >& solute_list )
{
   if ( grid_reps < 1 ) grid_reps = 1;

   double s_step   = fabs( s_max   - s_min   ) / ( s_res   - 1 ) * grid_reps;
   double ff0_step = fabs( ff0_max - ff0_min ) / ( ff0_res - 1 ) * grid_reps;

   // Allow a 1% overscan
   s_max   += 0.01 * s_step;
   ff0_max += 0.01 * ff0_step;

   solute_list.reserve( sq( grid_reps ) );

   double s_grid   = s_step   / grid_reps;
   double ff0_grid = ff0_step / grid_reps;

   // Generate solutes for each grid repetition
   for ( int i = 0; i < grid_reps; i++ )
   {
      for ( int j = 0; j < grid_reps; j++ )
      {
         solute_list << create_solutes(
               s_min   + s_grid   * i,   s_max,   s_step,
               ff0_min + ff0_grid * j, ff0_max, ff0_step );
      }
   }
}

QVector< US_Solute > US_Solute::create_solutes(
        double s_min,   double s_max,   double s_step,
        double ff0_min, double ff0_max, double ff0_step )
{
   QVector< US_Solute > solute_vector;

   for ( double ff0 = ff0_min; ff0 <= ff0_max; ff0 += ff0_step )
      for ( double s = s_min; s <= s_max; s += s_step )
      {
         // Omit s values close to zero.
         if ( s >= -1.0e-14  &&  s <= 1.0e-14 ) continue;

         solute_vector << US_Solute( s, ff0, 0.0 );
      }

   return solute_vector;
}


