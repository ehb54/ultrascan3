//! \file test_us_solve_sim.cpp
// test_us_solve_sim.cpp - Unit tests for US_SolveSim noise removal and NNLS
#include "qt_test_base.h"
#include "us_astfem_math.h"
#include "us_constants.h"
#include "us_math2.h"
#include "us_solve_sim.h"
#include <QTemporaryDir>
#include <QVector>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <random>

namespace
{
// Moving, diffusing, radially diluting boundary (Faxen-like approximation)
double boundary( double sval, double dval, double radius, double time )
{
   const double omega2 = pow( 2.0 * M_PI * 50000.0 / 60.0, 2 );
   const double rbound = 5.9 * exp( sval * omega2 * time );

   return exp( -2.0 * sval * omega2 * time ) * 0.5 * erfc( ( rbound - radius ) / ( 2.0 * sqrt( dval * time ) ) );
}

// Reference projection:  remove per-scan (RI) and/or per-radius (TI) means
//  of scan-major values
void project( int noisflag, int nscans, int npoints, double* vals )
{
   QVector<double> smean( nscans, 0.0 );
   QVector<double> rmean( npoints, 0.0 );
   double          gmean = 0.0;

   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         double val = vals[ss * npoints + rr];
         smean[ss] += val / npoints;
         rmean[rr] += val / nscans;
         gmean += val / ( nscans * npoints );
      }
   }

   bool fit_ti = ( noisflag & 1 ) != 0;
   bool fit_ri = ( noisflag & 2 ) != 0;

   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         vals[ss * npoints + rr] -=
            ( fit_ti ? rmean[rr] : 0.0 ) + ( fit_ri ? smean[ss] : 0.0 ) - ( fit_ti && fit_ri ? gmean : 0.0 );
      }
   }
}

// Remove the per-scan (RI) or per-radius (TI) means of the unmasked values;
//  return the largest mean removed
double removeMaskedMeans( bool per_scan, int nscans, int npoints, const QVector<bool>& masked, double* vals )
{
   int    ngroups = per_scan ? nscans : npoints;
   int    nmember = per_scan ? npoints : nscans;
   double maxmean = 0.0;

   for ( int gg = 0; gg < ngroups; gg++ )
   {
      double sum = 0.0;
      int    cnt = 0;
      for ( int mm = 0; mm < nmember; mm++ )
      {
         int kk = per_scan ? ( gg * npoints + mm ) : ( mm * npoints + gg );
         if ( !masked[kk] )
         {
            sum += vals[kk];
            cnt++;
         }
      }
      double mean = ( cnt > 0 ) ? sum / cnt : 0.0;
      for ( int mm = 0; mm < nmember; mm++ )
      {
         int kk = per_scan ? ( gg * npoints + mm ) : ( mm * npoints + gg );
         if ( !masked[kk] )
         {
            vals[kk] -= mean;
         }
      }
      maxmean = qMax( maxmean, fabs( mean ) );
   }

   return maxmean;
}

// Reference projection with masked values left out (and set to zero):
//  for TI and RI noise, alternately remove the per-scan and per-radius
//  means of the unmasked values until converged (alternating projections)
void projectMasked( int noisflag, int nscans, int npoints, const QVector<bool>& masked, double* vals )
{
   bool fit_ti = ( noisflag & 1 ) != 0;
   bool fit_ri = ( noisflag & 2 ) != 0;

   for ( int kk = 0; kk < nscans * npoints; kk++ )
   {
      vals[kk] = masked[kk] ? 0.0 : vals[kk];
   }

   for ( int iter = 0; iter < 1000000; iter++ )
   {
      double change = 0.0;
      if ( fit_ri )
      {
         change = qMax( change, removeMaskedMeans( true, nscans, npoints, masked, vals ) );
      }
      if ( fit_ti )
      {
         change = qMax( change, removeMaskedMeans( false, nscans, npoints, masked, vals ) );
      }
      if ( !( fit_ti && fit_ri ) || change < 1e-17 )
      {
         break;
      }
   }
}
}

// ============================================================================
// NNLS WITH ALGEBRAIC NOISE REMOVAL (closed-form boundary columns)
// ============================================================================

class TestUSSolveSimNnlsNoise : public QtTestBase
{
   protected:
      enum
      {
         NSCANS  = 12,
         NPOINTS = 40,
         NTOTAL  = NSCANS * NPOINTS
      };

      void SetUp() override
      {
         QtTestBase::SetUp();

         // A columns:  6 s values x 3 D values, scan-major rows
         for ( int is = 0; is < 6; is++ )
         {
            for ( int id = 0; id < 3; id++ )
            {
               for ( int ss = 0; ss < NSCANS; ss++ )
               {
                  for ( int rr = 0; rr < NPOINTS; rr++ )
                  {
                     amat << boundary(
                        ( 2.0 + 1.5 * is ) * 1.0e-13, ( 3.0 + 2.0 * id ) * 1.0e-7, 6.0 + rr * 0.025, 1200.0 + ss * 500.0 );
                  }
               }
            }
         }

         nsolutes = amat.size() / NTOTAL;
         ctrue.fill( 0.0, nsolutes );
         ctrue[4]  = 0.3;
         ctrue[9]  = 0.5;
         ctrue[14] = 0.2;
      }

      // Model data plus TI noise, RI noise and random noise
      QVector<double> makeData( double tiamp, double riamp, double rndamp, unsigned seed )
      {
         std::mt19937                     gen( seed );
         std::normal_distribution<double> gauss( 0.0, 1.0 );
         QVector<double>                  tinoi( NPOINTS );
         QVector<double>                  rinoi( NSCANS );
         for ( double& val : tinoi )
         {
            val = tiamp * gauss( gen );
         }
         for ( double& val : rinoi )
         {
            val = riamp * gauss( gen );
         }

         QVector<double> data( NTOTAL, 0.0 );
         for ( int ss = 0; ss < NSCANS; ss++ )
         {
            for ( int rr = 0; rr < NPOINTS; rr++ )
            {
               int kk = ss * NPOINTS + rr;
               for ( int cc = 0; cc < nsolutes; cc++ )
               {
                  data[kk] += ctrue[cc] * amat[cc * NTOTAL + kk];
               }
               data[kk] += tinoi[rr] + rinoi[ss] + rndamp * gauss( gen );
            }
         }

         return data;
      }

      // Solve with nnls_noise on copies (A and b are overwritten); with
      //  alpha != 0, Tikhonov rows follow the data rows in each A column
      void solve( int                    noisflag,
                  const QVector<double>& data,
                  QVector<double>&       xvec,
                  QVector<double>&       tivec,
                  QVector<double>&       rivec,
                  double                 alpha = 0.0 )
      {
         int             narows = ( alpha != 0.0 ) ? ( NTOTAL + nsolutes ) : NTOTAL;
         QVector<double> awork( narows * nsolutes, 0.0 );
         QVector<double> bwork( narows, 0.0 );

         for ( int cc = 0; cc < nsolutes; cc++ )
         {
            for ( int kk = 0; kk < NTOTAL; kk++ )
            {
               awork[cc * narows + kk] = amat[cc * NTOTAL + kk];
            }
            if ( alpha != 0.0 )
            {
               awork[cc * narows + NTOTAL + cc] = alpha;
            }
         }

         for ( int kk = 0; kk < NTOTAL; kk++ )
         {
            bwork[kk] = data[kk];
         }

         xvec.fill( 0.0, nsolutes );
         tivec.fill( 0.0, NPOINTS );
         rivec.fill( 0.0, NSCANS );

         ASSERT_EQ( US_SolveSim::nnls_noise( noisflag,
                                             QVector<int>( 1, NSCANS ),
                                             QVector<int>( 1, NPOINTS ),
                                             nsolutes,
                                             narows,
                                             awork,
                                             bwork,
                                             xvec,
                                             tivec,
                                             rivec ),
                    0 );
      }

      QVector<double> amat;  // Column-major A, NTOTAL rows
      QVector<double> ctrue; // True concentrations
      int             nsolutes = 0;
};

TEST_F( TestUSSolveSimNnlsNoise, FittedNoiseDoesNotChangeConcentrations )
{
   // Noise of the fitted kind is removed exactly:  adding it to the data
   // changes only the returned noise vectors
   QVector<double>                  base = makeData( 0.0, 0.0, 0.002, 7 );
   std::mt19937                     gen( 11 );
   std::normal_distribution<double> gauss( 0.0, 1.0 );

   for ( int noisflag = 1; noisflag <= 3; noisflag++ )
   {
      SCOPED_TRACE( noisflag );
      QVector<double> tadd( NPOINTS, 0.0 );
      QVector<double> radd( NSCANS, 0.0 );
      if ( noisflag & 1 )
      {
         for ( double& val : tadd )
         {
            val = 0.05 * gauss( gen );
         }
      }
      if ( noisflag & 2 )
      {
         for ( double& val : radd )
         {
            val = 0.20 * gauss( gen );
         }
      }

      QVector<double> noisy = base;
      for ( int ss = 0; ss < NSCANS; ss++ )
      {
         for ( int rr = 0; rr < NPOINTS; rr++ )
         {
            noisy[ss * NPOINTS + rr] += tadd[rr] + radd[ss];
         }
      }

      QVector<double> x0, t0, r0, x1, t1, r1;
      solve( noisflag, base, x0, t0, r0 );
      solve( noisflag, noisy, x1, t1, r1 );

      double xdiff = 0.0;
      for ( int cc = 0; cc < nsolutes; cc++ )
      {
         xdiff = qMax( xdiff, fabs( x1[cc] - x0[cc] ) );
      }
      EXPECT_LT( xdiff, 1e-9 );

      // TI and RI noise share a constant; compare their sum at each point
      double ndiff = 0.0;
      for ( int ss = 0; ss < NSCANS; ss++ )
      {
         for ( int rr = 0; rr < NPOINTS; rr++ )
         {
            ndiff = qMax( ndiff, fabs( ( t1[rr] - t0[rr] ) + ( r1[ss] - r0[ss] ) - ( tadd[rr] + radd[ss] ) ) );
         }
      }
      EXPECT_LT( ndiff, 1e-9 );
   }
}

TEST_F( TestUSSolveSimNnlsNoise, SolutionIsOptimalForProjectedProblem )
{
   // x minimizes ||P (b - A x)|| subject to x >= 0, where P removes the
   // fitted noise; the residual b - A x - noise equals P (b - A x)
   QVector<double> data = makeData( 0.05, 0.20, 0.003, 5 );

   for ( int noisflag = 1; noisflag <= 3; noisflag++ )
   {
      SCOPED_TRACE( noisflag );
      QVector<double> xvec, tivec, rivec;
      solve( noisflag, data, xvec, tivec, rivec );

      QVector<double> resid = data;
      for ( int cc = 0; cc < nsolutes; cc++ )
      {
         for ( int kk = 0; kk < NTOTAL; kk++ )
         {
            resid[kk] -= xvec[cc] * amat[cc * NTOTAL + kk];
         }
      }

      QVector<double> presid = resid;
      project( noisflag, NSCANS, NPOINTS, presid.data() );

      double rdiff = 0.0;
      for ( int ss = 0; ss < NSCANS; ss++ )
      {
         for ( int rr = 0; rr < NPOINTS; rr++ )
         {
            int kk = ss * NPOINTS + rr;
            rdiff  = qMax( rdiff, fabs( resid[kk] - tivec[rr] - rivec[ss] - presid[kk] ) );
         }
      }
      EXPECT_LT( rdiff, 1e-12 );

      // Kuhn-Tucker conditions:  the dual A^T P r is zero on the positive
      // set and non-positive elsewhere (P is a symmetric projection)
      double dual_pos  = 0.0;
      double dual_zero = -1.0;
      for ( int cc = 0; cc < nsolutes; cc++ )
      {
         double dual = 0.0;
         for ( int kk = 0; kk < NTOTAL; kk++ )
         {
            dual += amat[cc * NTOTAL + kk] * presid[kk];
         }

         EXPECT_GE( xvec[cc], 0.0 );
         if ( xvec[cc] > 0.0 )
         {
            dual_pos = qMax( dual_pos, fabs( dual ) );
         }
         else
         {
            dual_zero = qMax( dual_zero, dual );
         }
      }
      EXPECT_LT( dual_pos, 1e-9 );
      EXPECT_LT( dual_zero, 1e-9 );
   }
}

TEST_F( TestUSSolveSimNnlsNoise, TikhonovRowsAreNotProjected )
{
   // Only the data rows are projected:  the result is the NNLS solution of
   // [ P A ; alpha I ] x = [ P b ; 0 ]
   const double    alpha  = 0.5;
   const int       narows = NTOTAL + nsolutes;
   QVector<double> data   = makeData( 0.05, 0.20, 0.003, 9 );
   QVector<double> xvec, tivec, rivec;
   solve( 3, data, xvec, tivec, rivec, alpha );

   QVector<double> aref( narows * nsolutes, 0.0 );
   QVector<double> bref( narows, 0.0 );
   QVector<double> xref( nsolutes, 0.0 );
   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      for ( int kk = 0; kk < NTOTAL; kk++ )
      {
         aref[cc * narows + kk] = amat[cc * NTOTAL + kk];
      }
      project( 3, NSCANS, NPOINTS, aref.data() + cc * narows );
      aref[cc * narows + NTOTAL + cc] = alpha;
   }
   for ( int kk = 0; kk < NTOTAL; kk++ )
   {
      bref[kk] = data[kk];
   }
   project( 3, NSCANS, NPOINTS, bref.data() );

   ASSERT_EQ( US_Math2::nnls( aref.data(), narows, narows, nsolutes, bref.data(), xref.data() ), 0 );

   double xdiff = 0.0;
   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      xdiff = qMax( xdiff, fabs( xvec[cc] - xref[cc] ) );
   }
   EXPECT_LT( xdiff, 1e-10 );
}

TEST_F( TestUSSolveSimNnlsNoise, EachDataSetHasItsOwnNoise )
{
   // Global fit of two data sets with shared concentrations:  the rows of
   // the second (fewer scans and points, other radii and times) follow
   // those of the first, and each data set has its own TI and RI noise
   const int       ns2    = 9;
   const int       np2    = 30;
   const int       narows = NTOTAL + ns2 * np2;
   QVector<double> atwo; // Column-major, narows rows
   for ( int is = 0; is < 6; is++ )
   {
      for ( int id = 0; id < 3; id++ )
      {
         int cc = is * 3 + id;
         for ( int kk = 0; kk < NTOTAL; kk++ )
         {
            atwo << amat[cc * NTOTAL + kk];
         }
         for ( int ss = 0; ss < ns2; ss++ )
         {
            for ( int rr = 0; rr < np2; rr++ )
            {
               atwo << boundary( ( 2.0 + 1.5 * is ) * 1.0e-13, ( 3.0 + 2.0 * id ) * 1.0e-7, 6.1 + rr * 0.03, 900.0 + ss * 650.0 );
            }
         }
      }
   }
   ASSERT_EQ( atwo.size(), narows * nsolutes );

   QVector<double> ti1( NPOINTS ), ri1( NSCANS ), ti2( np2 ), ri2( ns2 );
   for ( int rr = 0; rr < NPOINTS; rr++ )
   {
      ti1[rr] = 0.03 * sin( 0.4 * rr );
   }
   for ( int ss = 0; ss < NSCANS; ss++ )
   {
      ri1[ss] = 0.02 * ( ss % 4 ) - 0.01;
   }
   for ( int rr = 0; rr < np2; rr++ )
   {
      ti2[rr] = 0.05 - 0.002 * rr;
   }
   for ( int ss = 0; ss < ns2; ss++ )
   {
      ri2[ss] = 0.04 * cos( 1.3 * ss );
   }

   for ( int noisflag = 1; noisflag <= 3; noisflag++ )
   {
      SCOPED_TRACE( noisflag );
      bool            fit_ti = ( noisflag & 1 ) != 0;
      bool            fit_ri = ( noisflag & 2 ) != 0;

      // Model plus the fitted kind of noise of each data set
      QVector<double> data( narows, 0.0 );
      for ( int kk = 0; kk < narows; kk++ )
      {
         for ( int cc = 0; cc < nsolutes; cc++ )
         {
            data[kk] += ctrue[cc] * atwo[cc * narows + kk];
         }
         if ( kk < NTOTAL )
         {
            data[kk] += ( fit_ti ? ti1[kk % NPOINTS] : 0.0 ) + ( fit_ri ? ri1[kk / NPOINTS] : 0.0 );
         }
         else
         {
            data[kk] += ( fit_ti ? ti2[( kk - NTOTAL ) % np2] : 0.0 ) + ( fit_ri ? ri2[( kk - NTOTAL ) / np2] : 0.0 );
         }
      }

      QVector<double> awork = atwo;
      QVector<double> bwork = data;
      QVector<double> xvec( nsolutes, 0.0 );
      QVector<double> tivec( NPOINTS + np2, 0.0 );
      QVector<double> rivec( NSCANS + ns2, 0.0 );
      QVector<int>    dscans  = { NSCANS, ns2 };
      QVector<int>    dpoints = { NPOINTS, np2 };
      ASSERT_EQ( US_SolveSim::nnls_noise( noisflag, dscans, dpoints, nsolutes, narows, awork, bwork, xvec, tivec, rivec ), 0 );

      double xdiff = 0.0;
      for ( int cc = 0; cc < nsolutes; cc++ )
      {
         xdiff = qMax( xdiff, fabs( xvec[cc] - ctrue[cc] ) );
      }
      EXPECT_LT( xdiff, 1e-8 );

      // Fitted noise at each point of each data set (TI and RI noise of a
      //  data set share a constant)
      double ndiff = 0.0;
      for ( int kk = 0; kk < narows; kk++ )
      {
         int    ks   = ( kk < NTOTAL ) ? kk : kk - NTOTAL;
         int    npts = ( kk < NTOTAL ) ? NPOINTS : np2;
         int    toff = ( kk < NTOTAL ) ? 0 : NPOINTS;
         int    roff = ( kk < NTOTAL ) ? 0 : NSCANS;
         int    rr   = ks % npts;
         int    ss   = ks / npts;
         double fit  = ( fit_ti ? tivec[toff + rr] : 0.0 ) + ( fit_ri ? rivec[roff + ss] : 0.0 );
         double tru  = ( kk < NTOTAL ) ? ( fit_ti ? ti1[rr] : 0.0 ) + ( fit_ri ? ri1[ss] : 0.0 )
                                       : ( fit_ti ? ti2[rr] : 0.0 ) + ( fit_ri ? ri2[ss] : 0.0 );
         ndiff       = qMax( ndiff, fabs( fit - tru ) );
      }
      EXPECT_LT( ndiff, 1e-9 );
   }
}

TEST_F( TestUSSolveSimNnlsNoise, MaskedRowsAreLeftOutOfTheNoiseFit )
{
   // Masked rows (zero in A and b) are not data:  the noise is the
   // least-squares fit over the unmasked rows, x minimizes the projected
   // residual there, and the masked rows stay zero.  Shapes with fewer and
   // with more scans than points, a fully masked scan and radius, and
   // Tikhonov rows below the data rows
   struct Shape
   {
         int    nscans;
         int    npoints;
         double alpha;
   };

   for ( const Shape& shape : { Shape{ 12, 40, 0.0 }, Shape{ 30, 8, 0.0 }, Shape{ 12, 40, 0.5 } } )
   {
      const int ns     = shape.nscans;
      const int np     = shape.npoints;
      const int nt     = ns * np;
      const int narows = ( shape.alpha != 0.0 ) ? ( nt + nsolutes ) : nt;
      SCOPED_TRACE( QString( "%1x%2 alpha %3" ).arg( ns ).arg( np ).arg( shape.alpha ).toStdString() );

      QVector<double> acols; // Column-major, nt rows
      for ( int is = 0; is < 6; is++ )
      {
         for ( int id = 0; id < 3; id++ )
         {
            for ( int ss = 0; ss < ns; ss++ )
            {
               for ( int rr = 0; rr < np; rr++ )
               {
                  acols << boundary( ( 2.0 + 1.5 * is ) * 1.0e-13,
                                     ( 3.0 + 2.0 * id ) * 1.0e-7,
                                     6.0 + rr * ( 1.0 / np ),
                                     1200.0 + ss * ( 6000.0 / ns ) );
               }
            }
         }
      }

      // Masked:  a growing band at the bottom of later scans, the last
      // radius, the second scan and one isolated point
      QVector<bool> masked( nt, false );
      for ( int ss = 0; ss < ns; ss++ )
      {
         for ( int rr = 0; rr < np; rr++ )
         {
            bool band            = ( ss >= ns / 3 ) && ( rr >= np - 1 - ( ss - ns / 3 ) * np / ( 2 * ns ) );
            masked[ss * np + rr] = band || rr == np - 1 || ss == 1;
         }
      }
      masked[( ns / 2 ) * np + np / 2] = true;

      std::mt19937                     gen( 23 );
      std::normal_distribution<double> gauss( 0.0, 1.0 );
      QVector<double>                  tinoi( np ), rinoi( ns );
      for ( double& val : tinoi )
      {
         val = 0.05 * gauss( gen );
      }
      for ( double& val : rinoi )
      {
         val = 0.20 * gauss( gen );
      }
      QVector<double> data( nt, 0.0 );
      for ( int kk = 0; kk < nt; kk++ )
      {
         for ( int cc = 0; cc < nsolutes; cc++ )
         {
            data[kk] += ctrue[cc] * acols[cc * nt + kk];
         }
         data[kk] += tinoi[kk % np] + rinoi[kk / np] + 0.003 * gauss( gen );
      }

      for ( int noisflag = 1; noisflag <= 3; noisflag++ )
      {
         SCOPED_TRACE( noisflag );
         QVector<double> awork( narows * nsolutes, 0.0 );
         QVector<double> bwork( narows, 0.0 );
         for ( int kk = 0; kk < nt; kk++ )
         {
            bwork[kk] = masked[kk] ? 0.0 : data[kk];
            for ( int cc = 0; cc < nsolutes; cc++ )
            {
               awork[cc * narows + kk] = masked[kk] ? 0.0 : acols[cc * nt + kk];
            }
         }
         for ( int cc = 0; cc < nsolutes && shape.alpha != 0.0; cc++ )
         {
            awork[cc * narows + nt + cc] = shape.alpha;
         }

         QVector<double> xvec( nsolutes, 0.0 ), tivec( np, 0.0 ), rivec( ns, 0.0 );
         ASSERT_EQ( US_SolveSim::nnls_noise( noisflag,
                                             QVector<int>( 1, ns ),
                                             QVector<int>( 1, np ),
                                             nsolutes,
                                             narows,
                                             awork,
                                             bwork,
                                             xvec,
                                             tivec,
                                             rivec,
                                             nullptr,
                                             &masked ),
                    0 );

         // awork is not checked here:  the NNLS solve overwrites it, and its
         // Householder steps fill a masked row once it becomes a pivot row.
         // Masked rows left non-zero by the projection would change the
         // problem solved, which the residual and Kuhn-Tucker checks catch.

         // At the unmasked rows, b - A x - noise = P (b - A x)
         QVector<double> resid = data;
         for ( int cc = 0; cc < nsolutes; cc++ )
         {
            for ( int kk = 0; kk < nt; kk++ )
            {
               resid[kk] -= xvec[cc] * acols[cc * nt + kk];
            }
         }
         QVector<double> presid = resid;
         projectMasked( noisflag, ns, np, masked, presid.data() );

         double rdiff = 0.0;
         for ( int kk = 0; kk < nt; kk++ )
         {
            if ( !masked[kk] )
            {
               double nfit = ( ( noisflag & 1 ) ? tivec[kk % np] : 0.0 ) + ( ( noisflag & 2 ) ? rivec[kk / np] : 0.0 );
               rdiff       = qMax( rdiff, fabs( resid[kk] - nfit - presid[kk] ) );
            }
         }
         EXPECT_LT( rdiff, 1e-10 );

         // Kuhn-Tucker conditions of min ||P (b - A x)||^2 + alpha^2 ||x||^2
         double dual_pos  = 0.0;
         double dual_zero = -1.0;
         for ( int cc = 0; cc < nsolutes; cc++ )
         {
            QVector<double> pcol( acols.mid( cc * nt, nt ) );
            projectMasked( noisflag, ns, np, masked, pcol.data() );
            double dual = -shape.alpha * shape.alpha * xvec[cc];
            for ( int kk = 0; kk < nt; kk++ )
            {
               dual += pcol[kk] * presid[kk];
            }

            EXPECT_GE( xvec[cc], 0.0 );
            if ( xvec[cc] > 0.0 )
            {
               dual_pos = qMax( dual_pos, fabs( dual ) );
            }
            else
            {
               dual_zero = qMax( dual_zero, dual );
            }
         }
         EXPECT_LT( dual_pos, 1e-9 );
         EXPECT_LT( dual_zero, 1e-9 );
      }
   }
}

TEST_F( TestUSSolveSimNnlsNoise, AllFalseMaskGivesUnmaskedResult )
{
   QVector<double> data = makeData( 0.05, 0.20, 0.003, 5 );
   QVector<bool>   masked( NTOTAL, false );

   for ( int noisflag = 1; noisflag <= 3; noisflag++ )
   {
      SCOPED_TRACE( noisflag );
      QVector<double> x0, t0, r0;
      solve( noisflag, data, x0, t0, r0 );

      QVector<double> awork = amat;
      QVector<double> bwork = data;
      QVector<double> xvec( nsolutes, 0.0 ), tivec( NPOINTS, 0.0 ), rivec( NSCANS, 0.0 );
      ASSERT_EQ( US_SolveSim::nnls_noise( noisflag,
                                          QVector<int>( 1, NSCANS ),
                                          QVector<int>( 1, NPOINTS ),
                                          nsolutes,
                                          NTOTAL,
                                          awork,
                                          bwork,
                                          xvec,
                                          tivec,
                                          rivec,
                                          nullptr,
                                          &masked ),
                 0 );
      EXPECT_EQ( xvec, x0 );
      EXPECT_EQ( tivec, t0 );
      EXPECT_EQ( rivec, r0 );
   }
}

// ============================================================================
// CALC_RESIDUALS ON SIMULATED DATA (ASTFEM)
// ============================================================================

class TestUSSolveSimCalcResiduals : public QtTestBase
{
   protected:
      // Radii 6.0 to 7.0 cm by 0.02 cm
      enum
      {
         NSCANS  = 12,
         NPOINTS = 51,
         NTOTAL  = NSCANS * NPOINTS
      };

      void SetUp() override
      {
         QtTestBase::SetUp();
         ASSERT_TRUE( tmpdir.isValid() );
         buildDataSet();
         dsets << &dset;
      }

      // One speed step at 50000 rpm with scan times and omega2t as generated
      //  by us_astfem_sim, a time state file in a temporary directory, and
      //  zero-valued data
      void buildDataSet()
      {
         US_SimulationParameters&              simparams = dset.simparams;
         US_SimulationParameters::SpeedProfile step;
         step.rotorspeed           = 50000;
         step.avg_speed            = 50000.0;
         step.set_speed            = 50000;
         step.scans                = NSCANS;
         step.acceleration         = 400;
         step.acceleration_flag    = true;
         step.delay_hours          = 0;
         step.delay_minutes        = 20.0;
         step.duration_hours       = 2;
         step.duration_minutes     = 0.0;
         simparams.meniscus        = 5.9;
         simparams.bottom          = 7.2;
         simparams.bottom_position = 7.2;
         simparams.sim             = true;

         US_DataIO::RawData rdata;
         rdata.type[0] = 'R';
         rdata.type[1] = 'A';
         memset( rdata.rawGUID, 0, sizeof( rdata.rawGUID ) );
         rdata.cell        = 1;
         rdata.channel     = 'S';
         rdata.description = "Synthetic";
         for ( int rr = 0; rr < NPOINTS; rr++ )
         {
            rdata.xvalues << 6.0 + rr * 0.02;
         }

         double delay = step.delay_minutes * 60.0;
         double durat = step.duration_hours * 3600.0 + step.duration_minutes * 60.0;
         double tinc  = ( durat - delay ) / (double) ( NSCANS - 1 );

         for ( int ss = 0; ss < NSCANS; ss++ )
         {
            US_DataIO::Scan scan;
            scan.temperature = NORMAL_TEMP;
            scan.rpm         = step.rotorspeed;
            scan.seconds     = (double) qRound( delay + tinc * ss );
            scan.omega2t     = US_AstfemMath::calc_omega2t( 0.0, 0.0, 0.0, step.rotorspeed, step.acceleration, scan.seconds );
            scan.wavelength  = 280.0;
            scan.plateau     = 0.0;
            scan.delta_r     = 0.02;
            scan.nz_stddev   = false;
            scan.rvalues.fill( 0.0, NPOINTS );
            rdata.scanData << scan;
         }

         step.time_first = qRound( rdata.scanData.first().seconds );
         step.w2t_first  = rdata.scanData.first().omega2t;
         step.time_last  = qRound( rdata.scanData.last().seconds );
         step.w2t_last   = rdata.scanData.last().omega2t;
         simparams.speed_step.clear();
         simparams.speed_step << step;

         QString tmst = tmpdir.filePath( "synthetic.time_state.tmst" );
         ASSERT_GT( US_AstfemMath::writetimestate( tmst, simparams, rdata ), 0 );
         ASSERT_GT( simparams.simSpeedsFromTimeState( tmst ), 0 );

         US_DataIO::EditedData& edata = dset.run_data;
         edata.expType                = "velocity";
         edata.runID                  = "synthetic";
         edata.editID                 = "0";
         edata.dataType               = "RA";
         edata.cell                   = "1";
         edata.channel                = "S";
         edata.wavelength             = "280";
         edata.description            = "Synthetic";
         edata.meniscus               = 5.9;
         edata.bottom                 = 7.2;
         edata.plateau                = 7.0;
         edata.baseline               = 0.0;
         edata.ODlimit                = 1.0e+20;
         edata.floatingData           = false;
         edata.xvalues                = rdata.xvalues;
         edata.scanData               = rdata.scanData;
         edata.bl_corr_slope          = 0.0;
         edata.bl_corr_yintercept     = 0.0;

         dset.viscosity          = VISC_20W;
         dset.density            = DENS_20W;
         dset.compress           = 0.0;
         dset.temperature        = NORMAL_TEMP;
         dset.vbar20             = 0.72;
         dset.vbartb             = 0.72;
         dset.s20w_correction    = 1.0;
         dset.D20w_correction    = 1.0;
         dset.rotor_stretch[0]   = 0.0;
         dset.rotor_stretch[1]   = 0.0;
         dset.centerpiece_bottom = 7.2;
         for ( double& zcoeff : dset.zcoeffs )
         {
            zcoeff = 0.0;
         }
         dset.solute_type = 0;
         dset.manual      = false;
      }

      // The A matrix (unit-concentration simulations of the uncut, sorted
      //  solutes) that calc_residuals builds without noise or regularization
      QVector<double> basis( US_SolveSim::Simulation sim )
      {
         sim.noisflag = 0;
         sim.alpha    = 0.0;
         sim.maxrss   = 0;
         QVector<double> amatrix;
         QVector<double> bvector;
         US_SolveSim     solvesim( dsets, 1, false );
         solvesim.calc_residuals( 0, 1, sim, false, &amatrix, &bvector );
         return amatrix;
      }

      // Set data to sum( conc * A column ) + TI noise + RI noise
      void setData( const QVector<double>& amatrix,
                    const QVector<double>& conc,
                    const QVector<double>& tinoi,
                    const QVector<double>& rinoi )
      {
         for ( int ss = 0; ss < NSCANS; ss++ )
         {
            for ( int rr = 0; rr < NPOINTS; rr++ )
            {
               int    kk  = ss * NPOINTS + rr;
               double val = tinoi[rr] + rinoi[ss];
               for ( int cc = 0; cc < conc.size(); cc++ )
               {
                  val += conc[cc] * amatrix[cc * NTOTAL + kk];
               }
               dset.run_data.scanData[ss].rvalues[rr] = val;
            }
         }
      }

      // Fit with calc_residuals
      US_SolveSim::Simulation fit( const US_SolveSim::Simulation& input, int noisflag, double alpha )
      {
         US_SolveSim::Simulation sim = input;
         sim.noisflag                = noisflag;
         sim.alpha                   = alpha;
         sim.maxrss                  = 0;
         US_SolveSim solvesim( dsets, 1, false );
         solvesim.calc_residuals( 0, 1, sim );
         return sim;
      }

      // Maximum deviation of fitted TI + RI noise from the true noise at any
      //  point (TI and RI noise share a constant)
      double noiseDeviation( const US_SolveSim::Simulation& sim, const QVector<double>& tinoi, const QVector<double>& rinoi )
      {
         double ndiff = 0.0;
         for ( int ss = 0; ss < NSCANS; ss++ )
         {
            for ( int rr = 0; rr < NPOINTS; rr++ )
            {
               ndiff = qMax( ndiff, fabs( sim.ti_noise[rr] + sim.ri_noise[ss] - tinoi[rr] - rinoi[ss] ) );
            }
         }
         return ndiff;
      }

      // Fitted concentration of each grid solute (zero if not returned)
      static QVector<double> fittedConc( const QVector<US_Solute>& grid, const QVector<US_Solute>& fitted )
      {
         QVector<double> cvals( grid.size(), 0.0 );
         for ( const US_Solute& solute : fitted )
         {
            for ( int cc = 0; cc < grid.size(); cc++ )
            {
               if ( grid[cc].s == solute.s && grid[cc].k == solute.k )
               {
                  cvals[cc] = solute.c;
               }
            }
         }
         return cvals;
      }

      static QVector<double> fittedConc( const QVector<US_ZSolute>& grid, const QVector<US_ZSolute>& fitted )
      {
         QVector<double> cvals( grid.size(), 0.0 );
         for ( const US_ZSolute& zsol : fitted )
         {
            for ( int cc = 0; cc < grid.size(); cc++ )
            {
               if ( grid[cc].x == zsol.x && grid[cc].y == zsol.y && grid[cc].z == zsol.z )
               {
                  cvals[cc] = zsol.c;
               }
            }
         }
         return cvals;
      }

      static double maxDiff( const QVector<double>& aval, const QVector<double>& bval )
      {
         double diff = 0.0;
         for ( int ii = 0; ii < aval.size(); ii++ )
         {
            diff = qMax( diff, fabs( aval[ii] - bval[ii] ) );
         }
         return diff;
      }

      // Test TI and RI noise:  smooth radial profile and scan offsets
      void makeNoise( QVector<double>& tinoi, QVector<double>& rinoi )
      {
         tinoi.clear();
         rinoi.clear();
         for ( int rr = 0; rr < NPOINTS; rr++ )
         {
            tinoi << 0.02 * sin( 0.3 * rr ) + 0.01;
         }
         for ( int ss = 0; ss < NSCANS; ss++ )
         {
            rinoi << 0.05 * ( ( ss % 3 ) - 1 ) + 0.01 * ss;
         }
      }

      QTemporaryDir                tmpdir;
      US_SolveSim::DataSet         dset;
      QList<US_SolveSim::DataSet*> dsets;
};

TEST_F( TestUSSolveSimCalcResiduals, RemovesTiAndRiNoise )
{
   // Data are exact sums of the solute simulations plus TI and RI noise
   US_SolveSim::Simulation input;
   for ( double sval : { 2.0, 4.0, 6.0 } )
   {
      for ( double kval : { 1.2, 2.0 } )
      {
         input.solutes << US_Solute( sval * 1.0e-13, kval );
      }
   }
   std::sort( input.solutes.begin(), input.solutes.end() );

   QVector<double> amatrix = basis( input );
   ASSERT_EQ( amatrix.size(), input.solutes.size() * NTOTAL );

   QVector<double> conc = { 0.3, 0.0, 0.5, 0.0, 0.0, 0.2 };
   QVector<double> tinoi, rinoi;
   makeNoise( tinoi, rinoi );
   setData( amatrix, conc, tinoi, rinoi );

   US_SolveSim::Simulation sim = fit( input, 3, 0.0 );

   EXPECT_LT( sim.variance, 1e-20 );
   ASSERT_EQ( sim.ti_noise.size(), NPOINTS );
   ASSERT_EQ( sim.ri_noise.size(), NSCANS );
   EXPECT_LT( noiseDeviation( sim, tinoi, rinoi ), 1e-9 );

   EXPECT_LT( maxDiff( fittedConc( input.solutes, sim.solutes ), conc ), 1e-8 );
}

TEST_F( TestUSSolveSimCalcResiduals, GlobalFitRemovesNoiseOfEachDataSet )
{
   // Two data sets fitted together:  the second has scans 2-10 and every
   // other radius of the first.  Each has its own TI and RI noise, which
   // must be removed exactly and returned in data set order
   const int            ns2 = 9;
   const int            np2 = ( NPOINTS + 1 ) / 2;
   US_SolveSim::DataSet dset2;
   dset2                         = dset;
   US_DataIO::EditedData& edata2 = dset2.run_data;
   edata2.xvalues.clear();
   edata2.scanData.clear();
   for ( int rr = 0; rr < NPOINTS; rr += 2 )
   {
      edata2.xvalues << dset.run_data.xvalues[rr];
   }
   for ( int ss = 2; ss < 2 + ns2; ss++ )
   {
      US_DataIO::Scan scan = dset.run_data.scanData[ss];
      scan.delta_r         = 0.04;
      scan.rvalues.fill( 0.0, np2 );
      edata2.scanData << scan;
   }
   dsets << &dset2;

   US_SolveSim::Simulation input;
   for ( double sval : { 2.0, 4.0, 6.0 } )
   {
      for ( double kval : { 1.2, 2.0 } )
      {
         input.solutes << US_Solute( sval * 1.0e-13, kval );
      }
   }
   std::sort( input.solutes.begin(), input.solutes.end() );

   // A matrix of the global fit:  rows of data set 1, then of data set 2
   const int       narows = NTOTAL + ns2 * np2;
   QVector<double> amatrix;
   QVector<double> bvector;
   {
      US_SolveSim::Simulation sim = input;
      sim.noisflag                = 0;
      US_SolveSim solvesim( dsets, 1, false );
      solvesim.calc_residuals( 0, 2, sim, false, &amatrix, &bvector );
   }
   ASSERT_EQ( amatrix.size(), input.solutes.size() * narows );

   QVector<double> conc = { 0.3, 0.0, 0.5, 0.0, 0.0, 0.2 };
   QVector<double> tinoi, rinoi;
   makeNoise( tinoi, rinoi );
   QVector<double> ti2( np2 ), ri2( ns2 );
   for ( int rr = 0; rr < np2; rr++ )
   {
      ti2[rr] = 0.03 - 0.001 * rr;
   }
   for ( int ss = 0; ss < ns2; ss++ )
   {
      ri2[ss] = 0.04 * cos( 1.3 * ss );
   }

   for ( int kk = 0; kk < narows; kk++ )
   {
      double val = 0.0;
      for ( int cc = 0; cc < conc.size(); cc++ )
      {
         val += conc[cc] * amatrix[cc * narows + kk];
      }
      if ( kk < NTOTAL )
      {
         dset.run_data.scanData[kk / NPOINTS].rvalues[kk % NPOINTS] = val + tinoi[kk % NPOINTS] + rinoi[kk / NPOINTS];
      }
      else
      {
         int ks                                      = kk - NTOTAL;
         edata2.scanData[ks / np2].rvalues[ks % np2] = val + ti2[ks % np2] + ri2[ks / np2];
      }
   }

   US_SolveSim::Simulation sim = input;
   sim.noisflag                = 3;
   US_SolveSim solvesim( dsets, 1, false );
   solvesim.calc_residuals( 0, 2, sim );

   EXPECT_LT( sim.variance, 1e-20 );
   EXPECT_LT( maxDiff( fittedConc( input.solutes, sim.solutes ), conc ), 1e-8 );
   ASSERT_EQ( sim.ti_noise.size(), NPOINTS + np2 );
   ASSERT_EQ( sim.ri_noise.size(), NSCANS + ns2 );

   // TI and RI noise of a data set share a constant
   double ndiff = 0.0;
   for ( int ss = 0; ss < NSCANS; ss++ )
   {
      for ( int rr = 0; rr < NPOINTS; rr++ )
      {
         ndiff = qMax( ndiff, fabs( sim.ti_noise[rr] + sim.ri_noise[ss] - tinoi[rr] - rinoi[ss] ) );
      }
   }
   for ( int ss = 0; ss < ns2; ss++ )
   {
      for ( int rr = 0; rr < np2; rr++ )
      {
         ndiff = qMax( ndiff, fabs( sim.ti_noise[NPOINTS + rr] + sim.ri_noise[NSCANS + ss] - ti2[rr] - ri2[ss] ) );
      }
   }
   EXPECT_LT( ndiff, 1e-9 );
}

TEST_F( TestUSSolveSimCalcResiduals, ConstantOffsetIsAbsorbedByFittedNoise )
{
   // A constant baseline offset is TI noise and RI noise at once:  with TI,
   // RI or TI+RI noise fitted, it must not change the concentrations or the
   // residuals, only shift the fitted noise (for TI+RI, their sum) by itself
   US_SolveSim::Simulation input;
   for ( double sval : { 2.0, 4.0, 6.0 } )
   {
      for ( double kval : { 1.2, 2.0 } )
      {
         input.solutes << US_Solute( sval * 1.0e-13, kval );
      }
   }
   std::sort( input.solutes.begin(), input.solutes.end() );

   QVector<double> amatrix = basis( input );
   ASSERT_EQ( amatrix.size(), input.solutes.size() * NTOTAL );

   // Random noise on top of TI and RI noise, so that the fit is not exact
   QVector<double> conc = { 0.3, 0.0, 0.5, 0.0, 0.0, 0.2 };
   QVector<double> tinoi, rinoi;
   makeNoise( tinoi, rinoi );
   setData( amatrix, conc, tinoi, rinoi );
   std::mt19937                     gen( 17 );
   std::normal_distribution<double> gauss( 0.0, 0.003 );
   for ( int ss = 0; ss < NSCANS; ss++ )
   {
      for ( int rr = 0; rr < NPOINTS; rr++ )
      {
         dset.run_data.scanData[ss].rvalues[rr] += gauss( gen );
      }
   }
   const US_DataIO::EditedData base = dset.run_data;

   for ( int noisflag = 1; noisflag <= 3; noisflag++ )
   {
      SCOPED_TRACE( noisflag );
      dset.run_data                 = base;
      US_SolveSim::Simulation sim0  = fit( input, noisflag, 0.0 );
      QVector<double>         conc0 = fittedConc( input.solutes, sim0.solutes );

      for ( double offset : { 0.05, -0.05, 0.5 } )
      {
         SCOPED_TRACE( offset );
         dset.run_data = base;
         for ( int ss = 0; ss < NSCANS; ss++ )
         {
            for ( int rr = 0; rr < NPOINTS; rr++ )
            {
               dset.run_data.scanData[ss].rvalues[rr] += offset;
            }
         }

         US_SolveSim::Simulation sim = fit( input, noisflag, 0.0 );

         EXPECT_NEAR( sim.variance, sim0.variance, 1e-10 * sim0.variance );
         EXPECT_LT( maxDiff( fittedConc( input.solutes, sim.solutes ), conc0 ), 1e-10 );

         if ( noisflag & 1 )
         {
            ASSERT_EQ( sim.ti_noise.size(), NPOINTS );
         }
         if ( noisflag & 2 )
         {
            ASSERT_EQ( sim.ri_noise.size(), NSCANS );
         }
         double nshift = 0.0;
         for ( int ss = 0; ss < NSCANS; ss++ )
         {
            for ( int rr = 0; rr < NPOINTS; rr++ )
            {
               double dnoise = 0.0;
               if ( noisflag & 1 )
               {
                  dnoise += sim.ti_noise[rr] - sim0.ti_noise[rr];
               }
               if ( noisflag & 2 )
               {
                  dnoise += sim.ri_noise[ss] - sim0.ri_noise[ss];
               }
               nshift = qMax( nshift, fabs( dnoise - offset ) );
            }
         }
         EXPECT_LT( nshift, 1e-10 );
      }
   }
}

TEST_F( TestUSSolveSimCalcResiduals, TikhonovWithNoiseKeepsColumnsAligned )
{
   // Regularization rows lengthen each A column; the noise removal must
   // step over them (a small alpha barely changes the exact fit)
   US_SolveSim::Simulation input;
   for ( double sval : { 2.0, 4.0, 6.0 } )
   {
      for ( double kval : { 1.2, 2.0 } )
      {
         input.solutes << US_Solute( sval * 1.0e-13, kval );
      }
   }
   std::sort( input.solutes.begin(), input.solutes.end() );

   QVector<double> amatrix = basis( input );
   ASSERT_EQ( amatrix.size(), input.solutes.size() * NTOTAL );

   QVector<double> conc = { 0.3, 0.0, 0.5, 0.0, 0.0, 0.2 };
   QVector<double> tinoi, rinoi;
   makeNoise( tinoi, rinoi );

   for ( int noisflag = 1; noisflag <= 3; noisflag++ )
   {
      SCOPED_TRACE( noisflag );
      QVector<double> tfit = tinoi;
      QVector<double> rfit = rinoi;
      if ( ( noisflag & 1 ) == 0 )
      {
         tfit.fill( 0.0 );
      }
      if ( ( noisflag & 2 ) == 0 )
      {
         rfit.fill( 0.0 );
      }
      setData( amatrix, conc, tfit, rfit );

      US_SolveSim::Simulation sim = fit( input, noisflag, 1.0e-3 );

      EXPECT_LT( sim.variance, 1e-12 );
      EXPECT_LT( maxDiff( fittedConc( input.solutes, sim.solutes ), conc ), 1e-4 );
   }
}

TEST_F( TestUSSolveSimCalcResiduals, TikhonovAfterNormCutKeepsColumnsAligned )
{
   // Solutes sorted by x = f/f0 (y = s, z = vbar); the first one (100 S)
   // pellets before the first scan, so its column is cut by the norm test
   // and the regularized A matrix is rebuilt for the remaining columns
   dset.solute_type = ( 1 << 6 ) | ( 0 << 3 ) | 3;
   US_SolveSim::Simulation input;
   input.zsolutes << US_ZSolute( 1.2, 100.0e-13, 0.72, 0.0 ) << US_ZSolute( 1.5, 3.0e-13, 0.72, 0.0 )
                  << US_ZSolute( 2.0, 5.0e-13, 0.72, 0.0 ) << US_ZSolute( 2.5, 7.0e-13, 0.72, 0.0 );

   QVector<double> amatrix = basis( input );
   ASSERT_EQ( amatrix.size(), 3 * NTOTAL ); // Pelleted solute was cut

   QVector<double> conc  = { 0.4, 0.0, 0.3 }; // For the uncut solutes
   QVector<double> cgrid = { 0.0, 0.4, 0.0, 0.3 };
   QVector<double> tinoi, rinoi;
   makeNoise( tinoi, rinoi );

   for ( int noisflag : { 0, 3 } )
   {
      SCOPED_TRACE( noisflag );
      QVector<double> tfit = tinoi;
      QVector<double> rfit = rinoi;
      if ( noisflag == 0 )
      {
         tfit.fill( 0.0 );
         rfit.fill( 0.0 );
      }
      setData( amatrix, conc, tfit, rfit );

      US_SolveSim::Simulation sim = fit( input, noisflag, 1.0e-3 );

      EXPECT_LT( sim.variance, 1e-12 );
      EXPECT_LT( maxDiff( fittedConc( input.zsolutes, sim.zsolutes ), cgrid ), 1e-4 );
   }
}

TEST_F( TestUSSolveSimCalcResiduals, OdLimitMaskedPointsDoNotBiasNoise )
{
   // Points at or above the OD limit are left out of the fit.  The data at
   // all other points are exact sums of the solute simulations plus TI
   // and/or RI noise, so the concentrations and the noise at the fitted
   // points must still be recovered exactly
   US_SolveSim::Simulation input;
   for ( double sval : { 2.0, 4.0, 6.0 } )
   {
      for ( double kval : { 1.2, 2.0 } )
      {
         input.solutes << US_Solute( sval * 1.0e-13, kval );
      }
   }
   std::sort( input.solutes.begin(), input.solutes.end() );

   QVector<double> amatrix = basis( input );
   ASSERT_EQ( amatrix.size(), input.solutes.size() * NTOTAL );

   QVector<double> conc = { 0.3, 0.0, 0.5, 0.0, 0.0, 0.2 };
   QVector<double> tinoi, rinoi;
   makeNoise( tinoi, rinoi );

   // Masked:  a growing band at the bottom of later scans, the last radius
   // of every scan and one isolated point
   QVector<bool> masked( NTOTAL, false );
   for ( int ss = 0; ss < NSCANS; ss++ )
   {
      for ( int rr = 0; rr < NPOINTS; rr++ )
      {
         masked[ss * NPOINTS + rr] = ( ss >= 4 && rr >= NPOINTS - 2 * ( ss - 3 ) ) || rr == NPOINTS - 1;
      }
   }
   masked[2 * NPOINTS + 20] = true;

   for ( int noisflag = 1; noisflag <= 3; noisflag++ )
   {
      SCOPED_TRACE( noisflag );
      const bool      fit_ti = ( noisflag & 1 ) != 0;
      const bool      fit_ri = ( noisflag & 2 ) != 0;
      QVector<double> tfit   = tinoi;
      QVector<double> rfit   = rinoi;
      if ( !fit_ti )
      {
         tfit.fill( 0.0 );
      }
      if ( !fit_ri )
      {
         rfit.fill( 0.0 );
      }
      setData( amatrix, conc, tfit, rfit );
      dset.run_data.ODlimit = 1.5;
      for ( int kk = 0; kk < NTOTAL; kk++ )
      {
         if ( masked[kk] )
         {
            dset.run_data.scanData[kk / NPOINTS].rvalues[kk % NPOINTS] = 2.0;
         }
      }

      US_SolveSim::Simulation sim = fit( input, noisflag, 0.0 );

      EXPECT_LT( maxDiff( fittedConc( input.solutes, sim.solutes ), conc ), 1e-8 );

      // Fitted noise and residuals at the unmasked points
      double ndiff = 0.0;
      double rmax  = 0.0;
      for ( int ss = 0; ss < NSCANS; ss++ )
      {
         for ( int rr = 0; rr < NPOINTS; rr++ )
         {
            if ( masked[ss * NPOINTS + rr] )
            {
               continue;
            }
            double nfit = ( fit_ti ? sim.ti_noise[rr] : 0.0 ) + ( fit_ri ? sim.ri_noise[ss] : 0.0 );
            ndiff       = qMax( ndiff, fabs( nfit - tfit[rr] - rfit[ss] ) );
            rmax        = qMax( rmax, fabs( sim.residuals.value( ss, rr ) ) );
         }
      }
      EXPECT_LT( ndiff, 1e-9 );
      EXPECT_LT( rmax, 1e-9 );
   }
}

TEST_F( TestUSSolveSimCalcResiduals, OdLimitMaskedPointsAreLeftOutOfVariance )
{
   // Points at or above the OD limit were not fitted:  the variances are
   // the mean squared residual of the other points only.  The residuals of
   // the masked points are still data - fit (Monte Carlo sigmas use them)
   US_SolveSim::Simulation input;
   for ( double sval : { 2.0, 4.0, 6.0 } )
   {
      for ( double kval : { 1.2, 2.0 } )
      {
         input.solutes << US_Solute( sval * 1.0e-13, kval );
      }
   }
   std::sort( input.solutes.begin(), input.solutes.end() );

   QVector<double> amatrix = basis( input );
   ASSERT_EQ( amatrix.size(), input.solutes.size() * NTOTAL );

   QVector<double> conc = { 0.3, 0.0, 0.5, 0.0, 0.0, 0.2 };
   QVector<double> tinoi, rinoi;
   makeNoise( tinoi, rinoi );

   QVector<bool> masked( NTOTAL, false );
   int           nunmask = 0;
   for ( int kk = 0; kk < NTOTAL; kk++ )
   {
      int ss     = kk / NPOINTS;
      int rr     = kk % NPOINTS;
      masked[kk] = ( ss >= 6 && rr >= NPOINTS - ( ss - 4 ) ) || kk == 3 * NPOINTS + 10;
      nunmask += masked[kk] ? 0 : 1;
   }

   for ( int noisflag : { 0, 3 } )
   {
      for ( double rndamp : { 0.0, 0.004 } )
      {
         SCOPED_TRACE( QString( "noisflag %1 random %2" ).arg( noisflag ).arg( rndamp ).toStdString() );
         QVector<double> tfit = tinoi;
         QVector<double> rfit = rinoi;
         if ( noisflag == 0 )
         {
            tfit.fill( 0.0 );
            rfit.fill( 0.0 );
         }
         setData( amatrix, conc, tfit, rfit );
         std::mt19937                     gen( 31 );
         std::normal_distribution<double> gauss( 0.0, 1.0 );
         dset.run_data.ODlimit = 1.5;
         for ( int kk = 0; kk < NTOTAL; kk++ )
         {
            double& val = dset.run_data.scanData[kk / NPOINTS].rvalues[kk % NPOINTS];
            val         = masked[kk] ? 2.5 : ( val + rndamp * gauss( gen ) );
         }

         US_SolveSim::Simulation sim = fit( input, noisflag, 0.0 );

         double                  rsum  = 0.0;
         double                  rmask = 0.0;
         for ( int kk = 0; kk < NTOTAL; kk++ )
         {
            double rval = sim.residuals.value( kk / NPOINTS, kk % NPOINTS );
            rsum += masked[kk] ? 0.0 : rval * rval;
            rmask = masked[kk] ? qMax( rmask, fabs( rval ) ) : rmask;
         }
         EXPECT_GT( rmask, 1.0 ); // Masked data 2.5, fit at most about 1.1
         ASSERT_EQ( sim.variances.size(), 1 );
         EXPECT_NEAR( sim.variance, rsum / nunmask, 1e-12 * rsum / nunmask + 1e-30 );
         EXPECT_NEAR( sim.variances[0], rsum / nunmask, 1e-12 * rsum / nunmask + 1e-30 );
         if ( rndamp == 0.0 )
         {
            EXPECT_LT( sim.variance, 1e-20 );
         }
      }
   }
}

TEST_F( TestUSSolveSimCalcResiduals, MonteCarloDataKeepsOdLimitMask )
{
   // Monte Carlo data are the fit plus a gaussian variation with the
   // residual magnitude as sigma.  At points above the OD limit, the
   // residual is the large unfitted data - fit:  with only base +
   // variation, many of them fall below the limit and enter the Monte
   // Carlo fits.  mc_data_value keeps them at their original value, so
   // each Monte Carlo fit leaves out exactly the points the base fit did
   US_SolveSim::Simulation input;
   for ( double sval : { 2.0, 4.0, 6.0 } )
   {
      for ( double kval : { 1.2, 2.0 } )
      {
         input.solutes << US_Solute( sval * 1.0e-13, kval );
      }
   }
   std::sort( input.solutes.begin(), input.solutes.end() );

   QVector<double> amatrix = basis( input );
   ASSERT_EQ( amatrix.size(), input.solutes.size() * NTOTAL );

   QVector<double> conc = { 0.3, 0.0, 0.5, 0.0, 0.0, 0.2 };
   QVector<double> zero( qMax( (int) NPOINTS, (int) NSCANS ), 0.0 );
   setData( amatrix, conc, zero, zero );
   std::mt19937                     gen( 41 );
   std::normal_distribution<double> gauss( 0.0, 1.0 );
   const double                     odlim = 1.5;
   dset.run_data.ODlimit                  = odlim;
   QVector<bool> masked( NTOTAL, false );
   for ( int kk = 0; kk < NTOTAL; kk++ )
   {
      int     ss  = kk / NPOINTS;
      int     rr  = kk % NPOINTS;
      double& val = dset.run_data.scanData[ss].rvalues[rr];
      masked[kk]  = ( ss >= 5 && rr >= NPOINTS - ( ss - 3 ) );
      val         = masked[kk] ? 1.6 + 0.4 * ( rr % 3 ) : val + 0.004 * gauss( gen );
   }
   US_DataIO::EditedData   odata = dset.run_data;

   US_SolveSim::Simulation sim = fit( input, 0, 0.0 );

   int                     nunmask = 0; // Masked points that base + variation would unmask
   for ( int iter = 0; iter < 5; iter++ )
   {
      SCOPED_TRACE( iter );
      for ( int kk = 0; kk < NTOTAL; kk++ )
      {
         int    ss    = kk / NPOINTS;
         int    rr    = kk % NPOINTS;
         double base  = sim.sim_data.value( ss, rr );
         double sigma = qAbs( sim.residuals.value( ss, rr ) );
         double vari  = sigma * gauss( gen );
         nunmask += ( masked[kk] && base + vari < odlim ) ? 1 : 0;
         dset.run_data.scanData[ss].rvalues[rr] = US_SolveSim::mc_data_value( base, vari, odata.value( ss, rr ), odata.ODlimit );
      }

      // Exactly the originally masked points are at or above the limit
      int nwrong = 0;
      for ( int kk = 0; kk < NTOTAL; kk++ )
      {
         nwrong += ( ( dset.run_data.value( kk / NPOINTS, kk % NPOINTS ) >= odlim ) != masked[kk] ) ? 1 : 0;
      }
      EXPECT_EQ( nwrong, 0 );

      // The Monte Carlo fit stays close to the base fit
      US_SolveSim::Simulation simmc = fit( input, 0, 0.0 );
      EXPECT_LT( maxDiff( fittedConc( input.solutes, simmc.solutes ), conc ), 0.05 );
      EXPECT_LT( sqrt( simmc.variance ), 0.02 );
   }
   EXPECT_GT( nunmask, 0 ); // The scenario does exercise the mask
   dset.run_data = odata;
}

TEST_F( TestUSSolveSimCalcResiduals, ZeroDataValueIsNotMaskedByOdLimit )
{
   // With points above the OD limit present, a data value of exactly 0.0
   // below the limit is still a data point:  the result must equal the
   // NNLS solution with only the OD-limit rows removed
   US_SolveSim::Simulation input;
   for ( double sval : { 2.0, 4.0, 6.0 } )
   {
      for ( double kval : { 1.2, 2.0 } )
      {
         input.solutes << US_Solute( sval * 1.0e-13, kval );
      }
   }
   std::sort( input.solutes.begin(), input.solutes.end() );

   QVector<double> amatrix = basis( input );
   const int       nsols   = input.solutes.size();
   ASSERT_EQ( amatrix.size(), nsols * NTOTAL );

   QVector<double> conc = { 0.3, 0.0, 0.5, 0.0, 0.0, 0.2 };
   QVector<double> zero( qMax( (int) NPOINTS, (int) NSCANS ), 0.0 );
   setData( amatrix, conc, zero, zero );

   // Zero values on the plateau of two scans; OD limit exceeded at the
   // bottom of the last scans
   const int kzero1                                                   = 5 * NPOINTS + 40;
   const int kzero2                                                   = 9 * NPOINTS + 45;
   dset.run_data.scanData[kzero1 / NPOINTS].rvalues[kzero1 % NPOINTS] = 0.0;
   dset.run_data.scanData[kzero2 / NPOINTS].rvalues[kzero2 % NPOINTS] = 0.0;
   dset.run_data.ODlimit                                              = 1.5;
   QVector<bool> masked( NTOTAL, false );
   for ( int ss = 8; ss < NSCANS; ss++ )
   {
      for ( int rr = NPOINTS - 4; rr < NPOINTS; rr++ )
      {
         masked[ss * NPOINTS + rr]              = true;
         dset.run_data.scanData[ss].rvalues[rr] = 3.0;
      }
   }

   // Reference:  NNLS with the OD-limit rows of A and b set to zero
   QVector<double> aref = amatrix;
   QVector<double> bref( NTOTAL, 0.0 );
   QVector<double> xref( nsols, 0.0 );
   for ( int kk = 0; kk < NTOTAL; kk++ )
   {
      bref[kk] = masked[kk] ? 0.0 : dset.run_data.scanData[kk / NPOINTS].rvalues[kk % NPOINTS];
      for ( int cc = 0; cc < nsols; cc++ )
      {
         if ( masked[kk] )
         {
            aref[cc * NTOTAL + kk] = 0.0;
         }
      }
   }
   ASSERT_EQ( US_Math2::nnls( aref.data(), NTOTAL, NTOTAL, nsols, bref.data(), xref.data() ), 0 );

   US_SolveSim::Simulation sim = fit( input, 0, 0.0 );

   EXPECT_LT( maxDiff( fittedConc( input.solutes, sim.solutes ), xref ), 1e-10 );
}
