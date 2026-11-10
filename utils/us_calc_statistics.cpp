//! \file us_calc_statistics.cpp
#include "us_calc_statistics.h"
#include "us_calc_fp.h" // first:  floating-point policy

#include "us_calc_version.h"

#include <QtGlobal>

#include <algorithm>
#include <cmath>

namespace US_Calc
{
LineFit linefit( const double* x, const double* y, int arraysize )
{
   double sumx      = 0.0;
   double sumy      = 0.0;
   double sumxy     = 0.0;
   double sumx_sq   = 0.0;
   double sumy_sq   = 0.0;
   double sumchi_sq = 0.0;
   double average;

   for ( int i = 0; i < arraysize; i++ )
   {
      sumy += y[i];
   }

   average = sumy / arraysize;

   for ( int i = 0; i < arraysize; i++ )
   {
      sumx += x[i];
      sumxy += x[i] * y[i];
      sumx_sq += x[i] * x[i];
      sumy_sq += y[i] * y[i];

      sumchi_sq += ( y[i] - average ) * ( y[i] - average );
   }

   LineFit fit;
   fit.slope       = ( arraysize * sumxy - sumx * sumy ) / ( arraysize * sumx_sq - sumx * sumx );
   fit.intercept   = ( sumy - fit.slope * sumx ) / arraysize;
   fit.correlation = ( arraysize * sumxy - sumx * sumy ) /
                     ( sqrt( arraysize * sumx_sq - sumx * sumx ) * sqrt( arraysize * sumy_sq - sumy * sumy ) );
   fit.sigma   = sqrt( sumchi_sq ) / arraysize;
   fit.average = average;
   return fit;
}

namespace
{
std::optional<double> finite( double v )
{
   return std::isfinite( v ) ? std::optional<double>( v ) : std::nullopt;
}

// 50 bins that partition [minimum, maximum]:  bin ii holds the values from
// its edge up to the next bin's edge, and the last bin also holds the
// maximum.  Every value is counted once, so the area is the total
// concentration times the bin width.  (The desktop code tested each bin
// against edge + width, which rounds differently from the next edge, and
// left the last bin open, so the maximum was counted or not depending on
// rounding.)  Fills the bin edges and concentrations and returns the area.
double histogram( const QVector<double>& vals,
                  const QVector<double>& concs,
                  int                    nbins,
                  double                 vlo,
                  double                 vhi,
                  double                 bininc,
                  QVector<double>&       xplot,
                  QVector<double>&       yplot )
{
   const int nvals = static_cast<int>( vals.size() );
   double    area  = 0.0;
   for ( int ii = 0; ii < nbins; ii++ )
   {
      xplot[ii] = vlo + bininc * (double) ii;
   }
   for ( int ii = 0; ii < nbins; ii++ )
   {
      yplot[ii]          = 0.0;
      const bool   last  = ( ii == nbins - 1 );
      const double upper = last ? vhi : xplot[ii + 1];

      for ( int jj = 0; jj < nvals; jj++ )
      {
         const double val = vals[jj];
         if ( val >= xplot[ii] && ( val < upper || ( last && val <= upper ) ) )
         {
            yplot[ii] += ( concs[jj] );
         }
      }

      area += yplot[ii] * bininc;
   }
   return area;
}

// The first bin with the highest concentration.
int mode_bin( const QVector<double>& yplot, int nbins )
{
   double val   = -1.0;
   int    thisb = 0;
   for ( int ii = 0; ii < nbins; ii++ )
   {
      if ( yplot[ii] > val )
      {
         val   = yplot[ii];
         thisb = ii;
      }
   }
   return thisb;
}

// US_DmgaMcStats::compute_statistics, in its order.  The common part (count,
// minimum, maximum, weighted mean, line-fit input) is already in st, xplot
// and yplot.
void monte_carlo_statistics( WeightedStatistics&    st,
                             const QVector<double>& vals,
                             const QVector<double>& concs,
                             double                 vmean,
                             QVector<double>&       xplot,
                             QVector<double>&       yplot )
{
   const int    nvals = static_cast<int>( vals.size() );
   const int    nbins = 50;
   const double vsiz  = (double) nvals;
   const double binsz = 50.0;
   const double vlo   = st.minimum;
   const double vhi   = st.maximum;
   st.constant        = ( vlo == vhi );

   if ( st.constant )
   {
      st.median             = finite( vmean );
      st.mode_center        = finite( vmean );
      st.skew               = 0.0;
      st.kurtosis           = 0.0;
      st.mode_low           = 0.0;
      st.mode_high          = 0.0;
      st.confidence95_low   = finite( vmean );
      st.confidence95_high  = finite( vmean );
      st.confidence99_low   = finite( vmean );
      st.confidence99_high  = finite( vmean );
      st.variance           = 0.0;
      st.correlation        = 0.0;
      st.standard_deviation = 0.0;
      st.standard_error     = 0.0;
      st.area               = 0.0;
      return;
   }

   double vm2 = 0.0;
   double vm3 = 0.0;
   double vm4 = 0.0;
   for ( int jj = 0; jj < nvals; jj++ )
   {
      const double dif = vals[jj] - vmean;
      const double dsq = dif * dif;
      vm2 += dsq;
      vm3 += ( dsq * dif );
      vm4 += ( dsq * dsq );
   }
   vm2 /= vsiz;
   vm3 /= vsiz;
   vm4 /= vsiz;
   const double    skew  = vm3 / pow( vm2, 1.5 );
   const double    kurto = vm4 / pow( vm2, 2.0 ) - 3.0;

   const LineFit   fit = linefit( xplot.data(), yplot.data(), nvals );

   QVector<double> wkvls = vals;
   std::sort( wkvls.begin(), wkvls.end() );
   const int hx    = nvals / 2;
   double    vmedi = wkvls[hx];
   if ( ( hx * 2 ) == nvals )
   {
      vmedi = ( vmedi + wkvls[hx - 1] ) * 0.5;
   }

   const double sdevi  = pow( vm2, 0.5 );
   const double sderr  = sdevi / pow( vsiz, 0.5 );
   const double bininc = ( vhi - vlo ) / binsz;
   const double area   = histogram( vals, concs, nbins, vlo, vhi, bininc, xplot, yplot );

   const double mode_lo  = xplot[mode_bin( yplot, nbins )];
   const double mode_hi  = mode_lo + bininc;
   const double mode_cen = ( mode_lo + mode_hi ) * 0.5;

   st.median             = finite( vmedi );
   st.skew               = finite( skew );
   st.kurtosis           = finite( kurto );
   st.variance           = finite( vm2 );
   st.standard_deviation = finite( sdevi );
   st.standard_error     = finite( sderr );
   st.correlation        = finite( fit.correlation );
   st.area               = finite( area );
   st.mode_low           = finite( mode_lo );
   st.mode_high          = finite( mode_hi );
   st.mode_center        = finite( mode_cen );
   st.confidence99_low   = finite( vmean - 2.576 * sdevi );
   st.confidence99_high  = finite( vmean + 2.576 * sdevi );
   st.confidence95_low   = finite( vmean - 1.960 * sdevi );
   st.confidence95_high  = finite( vmean + 1.960 * sdevi );
}

// US_SoluteData::outputStats, in its order.  The common part is already in
// st, xplot and yplot.
void distribution_statistics( WeightedStatistics&    st,
                              const QVector<double>& vals,
                              const QVector<double>& concs,
                              double                 vmean,
                              double                 vctot,
                              QVector<double>&       xplot,
                              QVector<double>&       yplot )
{
   const int    nvals        = static_cast<int>( vals.size() );
   const int    nbins        = 50;
   const double vsiz         = (double) nvals;
   const double binsz        = 50.0;
   const double vlo          = st.minimum;
   const double vhi          = st.maximum;
   double       vm2          = 0.0;
   double       vm3          = 0.0;
   double       vm4          = 0.0;
   double       vctot_square = 0.0;

   for ( int jj = 0; jj < nvals; jj++ )
   {
      const double val  = vals[jj];
      const double conc = concs[jj];
      const double dif  = val - vmean;
      const double dsq  = dif * dif;
      vm2 += dsq * conc;
      vm3 += ( dsq * dif ) * conc;
      vm4 += ( dsq * dsq ) * conc;
      vctot_square += conc * conc;
   }

   vm2 *= vctot_square / ( vctot * vctot - vctot_square );
   vm3 *= vctot_square / ( vctot * vctot - vctot_square );
   vm4 *= vctot_square / ( vctot * vctot - vctot_square );
   const double  skew  = vm3 / pow( vm2, 1.5 );
   const double  kurto = vm4 / pow( vm2, 2.0 ) - 3.0;

   const LineFit fit = linefit( xplot.data(), yplot.data(), nvals );

   const double  sdevi  = pow( vm2, 0.5 );
   const double  sderr  = sdevi / pow( vsiz, 0.5 );
   const double  bininc = ( vhi - vlo ) / binsz;
   const double  area   = histogram( vals, concs, nbins, vlo, vhi, bininc, xplot, yplot );

   const double  vdif  = ( vhi - vlo ) / vlo;
   const double  fvdif = qAbs( vdif );

   const double  mode_lo  = xplot[mode_bin( yplot, nbins )];
   const double  mode_hi  = mode_lo + bininc;
   const double  mode_cen = ( mode_lo + mode_hi ) / 2.0;

   QVector<int>  idxs( nvals, 0 );
   for ( int jj = 0; jj < nvals; jj++ )
   {
      idxs[jj] = jj;
   }
   // Values in ascending order, equal values by ascending concentration, so
   // the quantiles depend only on the data.  (The desktop code sorted on the
   // value alone with std::sort, which orders equal values differently on
   // each C++ library and for each component order.)
   std::stable_sort( idxs.begin(),
                     idxs.end(),
                     [&vals, &concs]( int a, int b )
                     { return vals[a] < vals[b] || ( vals[a] == vals[b] && concs[a] < concs[b] ); } );
   QVector<double> vals_sorted;
   QVector<double> concs_sorted;
   for ( int jj = 0; jj < nvals; jj++ )
   {
      vals_sorted.push_back( vals[idxs[jj]] );
      concs_sorted.push_back( concs[idxs[jj]] );
   }

   const double lo99     = vctot * 0.005;
   const double hi99     = vctot * 0.995;
   const double lo95     = vctot * 0.025;
   const double hi95     = vctot * 0.975;
   double       acc_conc = 0;
   int          lo99_id  = -1;
   int          hi99_id  = -1;
   int          lo95_id  = -1;
   int          hi95_id  = -1;

   for ( int jj = 0; jj < nvals; jj++ )
   {
      acc_conc += concs_sorted[jj];
      if ( acc_conc >= lo99 && lo99_id == -1 )
      {
         lo99_id = jj;
      }
      if ( acc_conc >= lo95 && lo95_id == -1 )
      {
         lo95_id = jj;
      }

      if ( acc_conc > hi99 && hi99_id == -1 )
      {
         hi99_id = jj - 1;
      }
      if ( acc_conc > hi95 && hi95_id == -1 )
      {
         hi95_id = jj - 1;
      }
   }

   st.constant           = fvdif < 0.0001;
   st.variance           = finite( vm2 );
   st.standard_deviation = finite( sdevi );
   st.standard_error     = finite( sderr );
   st.skew               = finite( skew );
   st.kurtosis           = finite( kurto );
   st.correlation        = finite( fit.correlation );
   st.mode_low           = finite( mode_lo );
   st.mode_high          = finite( mode_hi );
   st.mode_center        = finite( mode_cen );
   st.area               = finite( area );

   // An index is undefined when the cumulative concentration never reaches
   // a limit (negative concentrations), or for a single value, whose upper
   // index is set to -1 and never corrected.
   auto quantile        = [&]( int id ) { return id >= 0 && id < nvals ? finite( vals_sorted[id] ) : std::nullopt; };
   st.confidence95_low  = quantile( lo95_id );
   st.confidence95_high = quantile( hi95_id );
   st.confidence99_low  = quantile( lo99_id );
   st.confidence99_high = quantile( hi99_id );

   if ( !st.variance )
   {
      st.diagnostics.push_back(
         { Issue::ZeroTotal, Severity::Warning, "the weighted variance is undefined (a single nonzero concentration)" } );
   }
   if ( !st.confidence95_low || !st.confidence95_high || !st.confidence99_low || !st.confidence99_high )
   {
      st.diagnostics.push_back( { Issue::EmptyRange, Severity::Warning, "a confidence limit is undefined" } );
   }
}
} // namespace

WeightedStatistics weighted_statistics( const QVector<double>& vals, const QVector<double>& concs, StatisticsConvention convention )
{
   WeightedStatistics st;
   st.algorithm    = *algorithm( "weighted_statistics" );
   st.convention   = convention;
   const int nvals = static_cast<int>( vals.size() );
   st.count        = vals.size();
   if ( vals.isEmpty() || concs.size() != vals.size() )
   {
      st.diagnostics.push_back( { Issue::EmptyModel,
                                  Severity::Error,
                                  vals.isEmpty() ? "there are no values" : "values and concentrations differ in length" } );
      return st;
   }

   // Minimum, maximum and weighted mean, and the line-fit input (each value
   // against its index), as both desktop calculations start.
   const int       nbins = 50;
   double          vlo   = 9.9e30;
   double          vhi   = -9.9e30;
   QVector<double> xplot( qMax( nvals, nbins ) );
   QVector<double> yplot( qMax( nvals, nbins ) );
   double          vsum  = 0.0;
   double          vctot = 0.0;

   for ( int jj = 0; jj < nvals; jj++ )
   {
      const double val  = vals[jj];
      const double conc = concs[jj];
      vsum += ( val * conc );
      vctot += conc;
      vlo       = qMin( vlo, val );
      vhi       = qMax( vhi, val );
      xplot[jj] = (double) jj;
      yplot[jj] = val;
   }

   const double vmean = vsum / vctot;
   st.minimum         = vlo;
   st.maximum         = vhi;
   st.mean            = finite( vmean );
   st.midrange        = finite( ( vlo + vhi ) / 2.0 );

   if ( convention == StatisticsConvention::MonteCarloIterations )
   {
      monte_carlo_statistics( st, vals, concs, vmean, xplot, yplot );
   }
   else
   {
      distribution_statistics( st, vals, concs, vmean, vctot, xplot, yplot );
   }

   if ( vctot == 0.0 )
   {
      st.diagnostics.prepend( { Issue::ZeroTotal, Severity::Error, "the total concentration is zero" } );
   }
   return st;
}
} // namespace US_Calc
