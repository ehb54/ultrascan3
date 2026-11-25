//! \file us_sim_residuals.cpp
#include "us_sim_residuals.h"
#include "us_math2.h"

double US_SimResiduals::calculate( US_DataIO::EditedData&    edited,
                                   US_DataIO::RawData&       simulated,
                                   const US_Noise&           ti_noise,
                                   const US_Noise&           ri_noise,
                                   const QList<int>&         excluded,
                                   int                       scanCount,
                                   QVector<QVector<double>>& resids )
{
   int             dsize = edited.pointCount();
   int             ssize = simulated.pointCount();
   QVector<double> vecxx( dsize );
   QVector<double> vecsx( ssize );
   QVector<double> vecsy( ssize );
   double*         xx = vecxx.data();
   double*         sx = vecsx.data();
   double*         sy = vecsy.data();
   double          yval;
   double          sval;
   double          rmsd   = 0.0;
   double          tnoi   = 0.0;
   double          rnoi   = 0.0;
   bool            ftin   = ti_noise.count > 0;
   bool            frin   = ri_noise.count > 0;
   bool            matchd = ( dsize == ssize );
   int             kpts   = 0;

   QVector<double> resscan;

   resids.clear();
   resscan.resize( dsize );

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xx[jj] = edited.radius( jj );
   }

   for ( int jj = 0; jj < ssize; jj++ )
   {
      sx[jj] = simulated.radius( jj );
      // Radii past the data's cannot match; the sizes already differ then
      if ( jj >= dsize || sx[jj] != xx[jj] )
      {
         matchd = false;
      }
   }

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      bool usescan = !excluded.contains( ii );

      rnoi = frin ? ri_noise.values[ii] : 0.0;

      for ( int jj = 0; jj < ssize; jj++ )
      {
         sy[jj] = simulated.value( ii, jj );
      }

      for ( int jj = 0; jj < dsize; jj++ )
      {
         tnoi = ftin ? ti_noise.values[jj] : 0.0;

         if ( matchd )
         {
            sval = sy[jj];
         }
         else
         {
            sval = interpolate( xx[jj], sx, sy, ssize );
         }

         yval = edited.value( ii, jj ) - sval - rnoi - tnoi;

         if ( usescan )
         {
            rmsd += sq( yval );
            kpts++;
         }

         resscan[jj] = yval;
      }

      resids << resscan;
   }

   return rmsd / (double) ( kpts );
}

double US_SimResiduals::interpolate( double xv, const double* sx, const double* sy, int ssize )
{
   for ( int jj = 1; jj < ssize; jj++ )
   {
      if ( xv == sx[jj] )
      {
         return sy[jj];
      }

      if ( xv < sx[jj] )
      { // Below this point: interpolate between it and the previous one
         int    ii = jj - 1;
         double dx = sx[jj] - sx[ii];
         double dy = sy[jj] - sy[ii];
         return ( sy[ii] + ( xv - sx[ii] ) * dy / dx );
      }
   }

   // Not below any point: extend the last two points
   int    jj = ssize - 1;
   int    ii = jj - 1;
   double dx = sx[jj] - sx[ii];
   double dy = sy[jj] - sy[ii];
   return ( sy[ii] + ( xv - sx[ii] ) * dy / dx );
}
