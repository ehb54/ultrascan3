#include "../include/us_saxs_util.h"
#include "../include/us_saxs_gp.h"
#include "../include/us_hydrodyn_pat.h"

QString US_Saxs_Util::sgp_physical_stats( sgp_node *node )
{
   vector < PDB_atom > bm = node->bead_model();

   if ( !bm.size() )
   {
      return "empty model\n";
   }

   // run pat? i.e. do a principal axis transformation

   {
      vector < dati1_supc > in_dt;

      for ( unsigned int i = 0; i < bm.size(); i++ )
      {
         dati1_supc dt;

         dt.x  = bm[ i ].coordinate.axis[ 0 ];
         dt.y  = bm[ i ].coordinate.axis[ 1 ];
         dt.z  = bm[ i ].coordinate.axis[ 2 ];
         dt.r  = bm[ i ].radius;
         dt.ru = bm[ i ].radius;
         dt.m  = ( 4.0 / 3.0 ) * M_PI * bm[ i ].radius * bm[ i ].radius * bm[ i ].radius;

         in_dt.push_back( dt );
      }

      int out_nat;
      vector < dati1_pat > out_dt( in_dt.size() + 1 );

      if ( !us_hydrodyn_pat_main( ( int ) in_dt.size(),
                                 ( int ) in_dt.size(),
                                  &( in_dt[ 0 ] ),
                                  &out_nat,
                                  &( out_dt[ 0 ] ) ) )
      {
         // cout << QString( "pat ok, out_nat %1\n" ).arg( out_nat );
         for ( unsigned int i = 0; i < bm.size(); i++ )
         {
            bm[ i ].coordinate.axis[ 0 ] = out_dt[ i ].x;
            bm[ i ].coordinate.axis[ 1 ] = out_dt[ i ].y;
            bm[ i ].coordinate.axis[ 2 ] = out_dt[ i ].z;
            bm[ i ].bead_coordinate      = bm[ i ].coordinate;
            // bm[ i ].radius               = out_dt[ i ].r;
            // bm[ i ].bead_actual_radius   = out_dt[ i ].r;
            // bm[ i ].bead_computed_radius = out_dt[ i ].r;
         }
      }
   }

   // also compute intersection volumes & subtract?
   // every pair, remove intersection volume ?

   // compute average, center of electron density?

   double volume = 0e0;
   double volume_intersection = 0e0;
   point pmin;
   point pmax;

   // subtract each radius from min add to max?
   // possible alternate bounding box ...

   for ( unsigned int i = 0; i < bm.size(); i++ )
   {
      if ( i )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( pmin.axis[ j ] > bm[ i ].coordinate.axis[ j ] )
            {
               pmin.axis[ j ] = bm[ i ].coordinate.axis[ j ];
            }
            if ( pmax.axis[ j ] < bm[ i ].coordinate.axis[ j ] )
            {
               pmax.axis[ j ] = bm[ i ].coordinate.axis[ j ];
            }
         }
      } else {
         pmin = bm[ i ].coordinate;
         pmax = bm[ i ].coordinate;
      }

      volume += ( 4e0 / 3e0 ) * M_PI * bm[ i ].radius * bm[ i ].radius * bm[ i ].radius;
   }

   for ( unsigned int i = 0; i < bm.size(); i++ )
   {
      for ( unsigned int j = i + 1; j < bm.size(); j++ )
      {
         float d = dist( bm[ i ].coordinate , bm[ j ].coordinate );
         if ( d < bm[ i ].radius + bm[ j ].radius )
         {
            float r1 = bm[ i ].radius;
            float r  = bm[ j ].radius;
            if ( d > 0.0 )
            {
               volume_intersection +=
                  M_PI * ( r1 + r - d ) * ( r1 + r - d ) *
                  ( d * d + 
                    2.0 * d * r + 2.0 * d * r1 
                    - 3.0 * r * r  - 3.0 * r1 * r1 
                    + 6.0 * r * r1 ) 
                  /
                  ( 12.0 * d );
            } else {
               // one is within the other:
               if ( r > r1 )
               {
                  r = r1;
               }
               volume_intersection +=
                  ( 4.0 / 3.0 ) * M_PI * r * r * r;
            }
         }
      }
   }

   QString qs;

   qs += QString( 
                 "total        volume (A^3) %1\n" 
                 "intersection volume (A^3) %2\n" 
                 "excluded     volume (A^3) %3\n" 
                 )
      .arg( volume )
      .arg( volume_intersection )
      .arg( volume - volume_intersection );

   qs += QString( 
                  "bounding box size (A) %1 %2 %2\n" 
                  )
      .arg( pmax.axis[ 0 ] - pmin.axis[ 0 ] )
      .arg( pmax.axis[ 1 ] - pmin.axis[ 1 ] )
      .arg( pmax.axis[ 2 ] - pmin.axis[ 2 ] );

   qs += QString( 
                  "axial ratios: [x:z] = %1  [x:y] = %2  [y:z] = %3\n"
                  )
      .arg( ( pmax.axis[ 0 ] - pmin.axis[ 0 ] ) / ( pmax.axis[ 2 ] - pmin.axis[ 2 ] ) )
      .arg( ( pmax.axis[ 0 ] - pmin.axis[ 0 ] ) / ( pmax.axis[ 1 ] - pmin.axis[ 1 ] ) )
      .arg( ( pmax.axis[ 1 ] - pmin.axis[ 1 ] ) / ( pmax.axis[ 2 ] - pmin.axis[ 2 ] ) );

   return qs;
}

         
