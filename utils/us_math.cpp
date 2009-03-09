//! \file us_math.cpp

#include <stdlib.h>
#include <math.h>

#include "us_math.h"

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
