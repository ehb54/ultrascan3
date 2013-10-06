//! \file us_model_record.cpp

#include "us_model_record.h"

// Construct model record
ModelRecord::ModelRecord( void )
{
   taskx      = -1;
   str_k      = 0.0;
   end_k      = 0.0;
   par1       = 0.0;
   par2       = 0.0;
   variance   = 9999.9;
   rmsd       = 9999.9;
}

// Model record destructor
ModelRecord::~ModelRecord()
{
   isolutes.clear();
   csolutes.clear();

   clear_data();
}

// Public slot to clear data vectors
void ModelRecord::clear_data( void )
{
   sim_data .scanData.clear();
   sim_data .xvalues .clear();
   residuals.scanData.clear();
   residuals.xvalues .clear();
   ti_noise          .clear();
   ri_noise          .clear();
}

// Static public function to compute straight lines
int ModelRecord::compute_slines( double& smin, double& smax,
      double& fmin, double& fmax, int& nkpts, int& nlpts,
      double* parlims, QVector< ModelRecord >& mrecs )
{
   mrecs.clear();
   ModelRecord mrec;

   double  prng  = (double)( nlpts - 1 );
   double  krng  = (double)( nkpts - 1 );
   double  xrng  = smax - smin;
   double  xinc  = xrng / prng;
   double  ystr  = fmin;
   if ( parlims[ 0 ] < 0.0 )
   {
      parlims[ 0 ] = fmin;
      parlims[ 1 ] = fmax;
      parlims[ 2 ] = fmin;
      parlims[ 3 ] = fmax;
   }
   double  yslo  = parlims[ 0 ];
   double  yshi  = parlims[ 1 ];
   double  yelo  = parlims[ 2 ];
   double  yehi  = parlims[ 3 ];
   double  ysinc = ( yshi - yslo ) / krng;
   double  yeinc = ( yehi - yelo ) / krng;
           ystr  = yslo;

   int     mndx  = 0;
   int     nmodl = nkpts * nkpts;
   mrecs.reserve( nmodl );

   // Generate straight lines
   for ( int ii = 0; ii < nkpts; ii++ )
   { // Loop over k start values
      double yend = yelo;

      for ( int jj = 0; jj < nkpts; jj++ )
      { // Loop over k end values
         double xval = smin;
         double yval = ystr;
         double yinc = ( yend - ystr ) / prng; 

         mrec.isolutes.clear();
         US_Solute isol;

         for ( int kk = 0; kk < nlpts; kk++ )
         { // Loop over points on a line
            isol.s      = xval * 1.e-13;
            isol.k      = yval;
            mrec.isolutes << isol;
            xval       += xinc;
            yval       += yinc;
         } // END: points-per-line loop

         mrec.taskx     = mndx;
         mrec.str_k     = ystr;
         mrec.end_k     = yend;
         mrec.par1      = ystr;
         mrec.par2      = ( yend - ystr ) / xrng;
//qDebug() << "MR: ii jj" << ii << jj << "ys ye p1 p2" << ystr << yend
// << mrec.par1 << mrec.par2;
         mrecs << mrec;

         yend   += yeinc;
         mndx++;
      } // END: k-end loop

      ystr   += ysinc;
   } // END: k-start loop

   return nmodl;
}

// Static public function to compute sigmoid curves
int ModelRecord::compute_sigmoids( int& ctype, double& smin, double& smax,
      double& fmin, double& fmax, int& nkpts, int& nlpts,
      double* parlims, QVector< ModelRecord >& mrecs )
{
   if ( parlims[ 0 ] < 0.0 )
   {
      parlims[ 0 ] = 0.001;
      parlims[ 1 ] = 0.5;
      parlims[ 2 ] = 0.0;
      parlims[ 3 ] = 1.0;
   }
   double  p1lo = parlims[ 0 ];
   double  p1up = parlims[ 1 ];
   double  p2lo = parlims[ 2 ];
   double  p2up = parlims[ 3 ];

   mrecs.clear();
   ModelRecord mrec;

   double srng  = smax - smin;
   double p1llg = log( p1lo );
   double p1ulg = log( p1up );
   double prng  = (double)( nlpts - 1 );
   double krng  = (double)( nkpts - 1 );
   double p1inc = ( p1ulg - p1llg ) / krng;
   double p2inc = ( p2up  - p2lo  ) / krng;
   double xinc  = 1.0 / prng;
   double kstr  = fmin;
   double kdif  = fmax - fmin;
   if ( ctype == 2 )
   {
      kstr         = fmax;
      kdif         = -kdif;
   }
   double p1vlg = p1llg;
   int    mndx  = 0;
   int    nmodl = nkpts * nkpts;
   mrecs.reserve( nmodl );

   for ( int ii = 0; ii < nkpts; ii++ )
   { // Loop over par1 values (logarithmic progression)
      double p1val = exp( p1vlg );
      double p1fac = sqrt( 2.0 * p1val );
      double p2val = p2lo;

      for ( int jj = 0; jj < nkpts; jj++ )
      { // Loop over par2 value (linear progression)
         double xval  = 0.0;
         double kval  = kstr;
         double kval0 = kval;
         double sval  = smin;

         mrec.isolutes.clear();
         US_Solute isol;

         for ( int kk = 0; kk < nlpts; kk++ )
         { // Loop over points on a curve
            double efac  = 0.5 * erf( ( xval - p2val ) / p1fac ) + 0.5;
            kval         = kstr + kdif * efac;
            sval         = smin + xval * srng;
            xval        += xinc;
            if ( kk == 0 )
               kval0        = kval;

            isol.s       = sval * 1.e-13;
            isol.k       = kval;
            mrec.isolutes << isol;
         } // END: points-on-curve loop

         mrec.taskx   = mndx;
         mrec.str_k   = kval0;
         mrec.end_k   = kval;
         mrec.par1    = p1val;
         mrec.par2    = p2val;
         mrecs << mrec;

         mndx++;
         p2val    += p2inc;
      } // END: par2 values loop

      p1vlg    += p1inc;
   } // END: par1 values loop

   return nmodl;
}

// Static public function to compute horizontal lines [ C(s) ]
int ModelRecord::compute_hlines( double& smin, double& smax,
      double& fmin, double& fmax, int& nkpts, int& nlpts,
      double* parlims, QVector< ModelRecord >& mrecs )
{
   mrecs.clear();
   ModelRecord mrec;

   double  krng  = (double)( nkpts - 1 );
   double  prng  = (double)( nlpts - 1 );
   double  xinc  = ( smax - smin ) / prng;

   if ( parlims[ 0 ] < 0.0 )
   {
      parlims[ 0 ] = fmin;
      parlims[ 1 ] = fmax;
      parlims[ 2 ] = fmin;
      parlims[ 3 ] = fmax;
   }

   double  yval  = parlims[ 0 ];
   double  yinc  = ( parlims[ 1 ] - yval ) / krng;
   int     mndx  = 0;
   mrecs.reserve( nkpts );

   // Generate horizontal lines
   for ( int ii = 0; ii < nkpts; ii++ )
   { // Loop over k start and end values
      double xval = smin;

      mrec.isolutes.clear();
      US_Solute isol;

      for ( int kk = 0; kk < nlpts; kk++ )
      { // Loop over points on a line
         isol.s      = xval * 1.e-13;
         isol.k      = yval;
         mrec.isolutes << isol;
         xval       += xinc;
      } // END: points-per-line loop

      mrec.taskx  = mndx;
      mrec.str_k  = yval;
      mrec.end_k  = yval;
      mrec.par1   = yval;
      mrec.par2   = 0.0;
      yval       += yinc;
      mndx++;

      mrecs << mrec;
   } // END: k-value loop

   return nkpts;
}

