// Robust automatic Guinier range search ("guinier_search").
//
// Algorithm outline (all parameters exposed through US_Guinier_Search_Params):
//   1. clean: keep finite points, q >= 0 (q > 0 for rc/rt), I > 0 (or fail when dropneg is off),
//      inside the optional hard q limits; decide whether SDs are usable.
//   2. preliminary Rg from windows starting at the first point, to bound the search region to
//      q <= regionfactor * qrgmax / Rg.
//   3. enumerate every window [i,j] in the region with minpts <= n <= maxpts, negative slope,
//      q_j * Rg <= qrgmax, optional q_i * Rg <= qrgmin, optional Rg limits.
//   4. score each window with six terms in [0,1]: reduced chi2, quadratic curvature t-statistic,
//      q*Rg span, coverage of the available Guinier region, low-q start, and consensus with the
//      quality weighted median Rg of all windows. quality = weighted geometric mean of the terms.
//   5. best window = maximum quality; optional outlier rejection; Rg, I(0) with SDs; the z-score of
//      the lowest-q residuals gives the aggregation / repulsion flags; the top fraction of windows
//      gives the Rg spread.
//
// Existing guinier_fit* routines are left untouched; this file is additive.

#include "../include/us_saxs_util.h"
#include <QRegularExpression>
#include <cmath>
#include <algorithm>

namespace {

   struct linfit
   {
      double a;        // intercept
      double b;        // slope
      double siga;
      double sigb;
      double chi2;     // weighted sum of squared residuals
      double chi2_red; // chi2 / ( n - 2 ), 1 when unweighted
      double rms;      // unweighted rms residual
      int    n;
   };

   // weighted least squares y = a + b x, w = 1 / sd^2 or 1 when unweighted.
   // unweighted: parameter SDs come from the residual variance.
   // weighted:   parameter SDs come from the covariance, inflated by sqrt( chi2_red ) when chi2_red > 1.
   void fit_line( const vector < double > & x,
                  const vector < double > & y,
                  const vector < double > & w,
                  bool                      weighted,
                  linfit                  & f )
   {
      int n = (int) x.size();
      f.n = n;
      double sw  = 0e0;
      double swx = 0e0;
      double swy = 0e0;
      for ( int k = 0; k < n; ++k )
      {
         sw  += w[ k ];
         swx += w[ k ] * x[ k ];
         swy += w[ k ] * y[ k ];
      }
      double xbar = swx / sw;
      double st2  = 0e0;
      double b    = 0e0;
      for ( int k = 0; k < n; ++k )
      {
         double t = x[ k ] - xbar;
         st2 += w[ k ] * t * t;
         b   += w[ k ] * t * y[ k ];
      }
      b /= st2;
      double a = ( swy - swx * b ) / sw;
      double chi2 = 0e0;
      double ssr  = 0e0;
      for ( int k = 0; k < n; ++k )
      {
         double r = y[ k ] - a - b * x[ k ];
         chi2 += w[ k ] * r * r;
         ssr  += r * r;
      }
      double vara = ( 1e0 + swx * swx / ( sw * st2 ) ) / sw;
      double varb = 1e0 / st2;
      double scale;
      if ( weighted )
      {
         f.chi2_red = n > 2 ? chi2 / ( n - 2 ) : 1e0;
         scale      = f.chi2_red > 1e0 ? f.chi2_red : 1e0;
      } else {
         f.chi2_red = 1e0;
         scale      = n > 2 ? ssr / ( n - 2 ) : 0e0;
      }
      f.a    = a;
      f.b    = b;
      f.siga = sqrt( vara * scale );
      f.sigb = sqrt( varb * scale );
      f.chi2 = chi2;
      f.rms  = n > 2 ? sqrt( ssr / ( n - 2 ) ) : 0e0;
   }

   // t-statistic |c| / sd(c) of the quadratic term of y = a + b xc + c xc^2, xc centered.
   // returns 0 when it cannot be computed ( n < 4 or singular )
   double curvature_t( const vector < double > & x,
                       const vector < double > & y,
                       const vector < double > & w,
                       bool                      weighted,
                       double                    sigfloor )
   {
      int n = (int) x.size();
      if ( n < 4 )
      {
         return 0e0;
      }
      double sw = 0e0;
      double sx = 0e0;
      for ( int k = 0; k < n; ++k )
      {
         sw += w[ k ];
         sx += w[ k ] * x[ k ];
      }
      double xbar = sx / sw;

      // normal equations  M p = v  with basis ( 1, xc, xc^2 )
      double m[ 3 ][ 3 ] = { { 0e0, 0e0, 0e0 }, { 0e0, 0e0, 0e0 }, { 0e0, 0e0, 0e0 } };
      double v[ 3 ]      = { 0e0, 0e0, 0e0 };
      for ( int k = 0; k < n; ++k )
      {
         double xc = x[ k ] - xbar;
         double p[ 3 ] = { 1e0, xc, xc * xc };
         for ( int r = 0; r < 3; ++r )
         {
            v[ r ] += w[ k ] * p[ r ] * y[ k ];
            for ( int c = 0; c < 3; ++c )
            {
               m[ r ][ c ] += w[ k ] * p[ r ] * p[ c ];
            }
         }
      }

      // invert the symmetric 3x3 by cofactors
      double det =
         m[ 0 ][ 0 ] * ( m[ 1 ][ 1 ] * m[ 2 ][ 2 ] - m[ 1 ][ 2 ] * m[ 2 ][ 1 ] )
         - m[ 0 ][ 1 ] * ( m[ 1 ][ 0 ] * m[ 2 ][ 2 ] - m[ 1 ][ 2 ] * m[ 2 ][ 0 ] )
         + m[ 0 ][ 2 ] * ( m[ 1 ][ 0 ] * m[ 2 ][ 1 ] - m[ 1 ][ 1 ] * m[ 2 ][ 0 ] );
      if ( !std::isfinite( det ) || fabs( det ) <= 0e0 )
      {
         return 0e0;
      }
      double inv[ 3 ][ 3 ];
      inv[ 0 ][ 0 ] =  ( m[ 1 ][ 1 ] * m[ 2 ][ 2 ] - m[ 1 ][ 2 ] * m[ 2 ][ 1 ] ) / det;
      inv[ 0 ][ 1 ] = -( m[ 0 ][ 1 ] * m[ 2 ][ 2 ] - m[ 0 ][ 2 ] * m[ 2 ][ 1 ] ) / det;
      inv[ 0 ][ 2 ] =  ( m[ 0 ][ 1 ] * m[ 1 ][ 2 ] - m[ 0 ][ 2 ] * m[ 1 ][ 1 ] ) / det;
      inv[ 1 ][ 0 ] = -( m[ 1 ][ 0 ] * m[ 2 ][ 2 ] - m[ 1 ][ 2 ] * m[ 2 ][ 0 ] ) / det;
      inv[ 1 ][ 1 ] =  ( m[ 0 ][ 0 ] * m[ 2 ][ 2 ] - m[ 0 ][ 2 ] * m[ 2 ][ 0 ] ) / det;
      inv[ 1 ][ 2 ] = -( m[ 0 ][ 0 ] * m[ 1 ][ 2 ] - m[ 0 ][ 2 ] * m[ 1 ][ 0 ] ) / det;
      inv[ 2 ][ 0 ] =  ( m[ 1 ][ 0 ] * m[ 2 ][ 1 ] - m[ 1 ][ 1 ] * m[ 2 ][ 0 ] ) / det;
      inv[ 2 ][ 1 ] = -( m[ 0 ][ 0 ] * m[ 2 ][ 1 ] - m[ 0 ][ 1 ] * m[ 2 ][ 0 ] ) / det;
      inv[ 2 ][ 2 ] =  ( m[ 0 ][ 0 ] * m[ 1 ][ 1 ] - m[ 0 ][ 1 ] * m[ 1 ][ 0 ] ) / det;

      double p[ 3 ];
      for ( int r = 0; r < 3; ++r )
      {
         p[ r ] = 0e0;
         for ( int c = 0; c < 3; ++c )
         {
            p[ r ] += inv[ r ][ c ] * v[ c ];
         }
      }

      double chi2 = 0e0;
      double ssr  = 0e0;
      for ( int k = 0; k < n; ++k )
      {
         double xc = x[ k ] - xbar;
         double r  = y[ k ] - p[ 0 ] - p[ 1 ] * xc - p[ 2 ] * xc * xc;
         chi2 += w[ k ] * r * r;
         ssr  += r * r;
      }
      double scale;
      if ( weighted )
      {
         double chi2_red = chi2 / ( n - 3 );
         scale = chi2_red > 1e0 ? chi2_red : 1e0;
      } else {
         scale = std::max( ssr / ( n - 3 ), sigfloor * sigfloor );
      }
      double varc = inv[ 2 ][ 2 ] * scale;
      if ( !( varc > 0e0 ) || !std::isfinite( varc ) )
      {
         return 0e0;
      }
      return fabs( p[ 2 ] ) / sqrt( varc );
   }

   QString json_num( double v )
   {
      if ( !std::isfinite( v ) )
      {
         return "null";
      }
      return QString::number( v, 'g', 8 );
   }

   QString json_str( const QString & s )
   {
      QString r = s;
      r.replace( "\\", "\\\\" ).replace( "\"", "\\\"" ).replace( "\n", " " );
      return "\"" + r + "\"";
   }

   struct window
   {
      int    i;
      int    j;
      int    n;
      linfit f;
      double rg;
      double qrg0;
      double qrg1;
      double curv_t;
      double terms[ 7 ];
      double q0;       // quality before the consensus term
      double quality;
   };
}

// ---------------------------------------------------------------- parameters

US_Guinier_Search_Params::US_Guinier_Search_Params()
{
   minpts       = 10;
   maxpts       = 0;
   qrgmax       = 0e0;
   qrgmin       = 0e0;
   qmin         = 0e0;
   qmax         = 0e0;
   rgmin        = 0e0;
   rgmax        = 0e0;
   regionfactor = 3e0;
   usesd        = -1;
   dropneg      = true;
   qscale       = 1e0;
   outlier      = 0e0;
   slopet       = 2e0;
   minspan      = 0.2e0;
   sigfloor     = 0.005e0;
   curvt        = 2e0;
   skipz        = 2e0;
   fwdz         = 3e0;
   fwddev       = 0.02e0;
   constol      = 0.1e0;
   topfrac      = 0.1e0;
   aggthresh    = 3e0;
   aggmindev    = 0.01e0;
   weights      = vector < double >( 7, 1e0 );
   weights[ 3 ] = 0e0;   // coverage term off by default: it favours long windows on dense data
   type         = "rg";
   debug        = 0;
}

double US_Guinier_Search_Params::effective_qrgmax() const
{
   if ( qrgmax > 0e0 )
   {
      return qrgmax;
   }
   return type == "rg" ? 1.3e0 : 1e0;
}

QStringList US_Guinier_Search_Params::keys()
{
   QStringList known;
   known << "type" << "minpts" << "maxpts" << "qrgmax" << "qrgmin" << "qmin" << "qmax"
         << "rgmin" << "rgmax" << "regionfactor" << "usesd" << "dropneg" << "qscale"
         << "outlier" << "slopet" << "minspan" << "sigfloor" << "curvt" << "skipz" << "fwdz" << "fwddev"
         << "constol" << "topfrac" << "aggthresh" << "aggmindev" << "weights" << "debug";
   return known;
}

QString US_Guinier_Search_Params::help()
{
   return
      "guinier_search parameters (key value), defaults in parentheses:\n"
      "  type         rg | rc | rt : Guinier, cross-section or transverse (rg)\n"
      "  minpts       minimum points in a window (10)\n"
      "  maxpts       maximum points in a window, 0 = unlimited (0)\n"
      "  qrgmax       maximum q*Rg at the window end, 0 = 1.3 for rg, 1.0 for rc/rt (0)\n"
      "  qrgmin       maximum q*Rg at the window start, 0 = unlimited (0)\n"
      "  qmin         hard lower q limit, 0 = none (0)\n"
      "  qmax         hard upper q limit, 0 = none (0)\n"
      "  rgmin        reject windows with Rg below this, 0 = none (0)\n"
      "  rgmax        reject windows with Rg above this, 0 = none (0)\n"
      "  regionfactor search only q <= regionfactor * qrgmax / preliminary Rg (3)\n"
      "  usesd        auto | 0 | 1 : weight by SDs when usable, never, or require (auto)\n"
      "  dropneg      1 drops I <= 0 points, 0 fails on them (1)\n"
      "  qscale       multiply q on input, e.g. 0.1 for nm^-1 data (1)\n"
      "  outlier      outlier rejection distance in SDs on the chosen window, 0 = off (0)\n"
      "  slopet       minimum |slope|/sd(slope) for a window to be valid (2)\n"
      "  minspan      minimum q*Rg span of a valid window (0.2)\n"
      "  sigfloor     noise floor in ln I used for unweighted fits (0.005)\n"
      "  curvt        curvature t-statistic giving a penalty of 1/2 (2)\n"
      "  skipz        skipped low-q points within skipz SDs of the line count against the window (2)\n"
      "  fwdz         |z| above which the points beyond the window inside q*Rg<=qrgmax are tested (3)\n"
      "  fwddev       mean deviation in ln I of those points giving a penalty of 1/2 (0.02)\n"
      "  constol      relative Rg tolerance of the consensus term (0.1)\n"
      "  topfrac      fraction of windows, by quality, used for the Rg spread (0.1)\n"
      "  aggthresh    |z| of the low-q residuals flagging aggregation / repulsion (3)\n"
      "  aggmindev    minimum mean low-q deviation in ln I for those flags (0.01)\n"
      "  weights      seven exponents: chi2,curvature,span,coverage,start,forward,consensus (1,1,1,0,1,1,1)\n"
      "  debug        1 dumps every candidate window to stderr (0)\n"
      ;
}

bool US_Guinier_Search_Params::set( const map < QString, QString > & kv, QString & errormsg )
{
   errormsg = "";
   QStringList known = keys();

   for ( map < QString, QString >::const_iterator it = kv.begin(); it != kv.end(); ++it )
   {
      QString key = it->first.toLower();
      QString val = it->second.trimmed();
      val.replace( QRegularExpression( "^\"|\"$" ), "" );
      if ( !known.contains( key ) )
      {
         continue;
      }
      bool ok = true;
      if ( key == "type" )
      {
         type = val.toLower();
         if ( type != "rg" && type != "rc" && type != "rt" )
         {
            errormsg = "type must be rg, rc or rt";
            return false;
         }
      } else if ( key == "usesd" ) {
         if ( val.toLower() == "auto" || val == "-1" )
         {
            usesd = -1;
         } else {
            usesd = val.toInt( &ok ) ? 1 : 0;
         }
      } else if ( key == "dropneg" ) {
         dropneg = val.toInt( &ok ) != 0;
      } else if ( key == "minpts" ) {
         minpts = val.toInt( &ok );
      } else if ( key == "maxpts" ) {
         maxpts = val.toInt( &ok );
      } else if ( key == "debug" ) {
         debug = val.toInt( &ok );
      } else if ( key == "weights" ) {
         QString w = val;
         w.replace( QRegularExpression( "[\\[\\]\\s]" ), "" );
         QStringList parts = w.split( ",", Qt::SkipEmptyParts );
         if ( parts.size() != 7 )
         {
            errormsg = "weights needs exactly 7 comma separated values";
            return false;
         }
         weights.clear();
         for ( int k = 0; k < 7; ++k )
         {
            bool okw;
            double d = parts[ k ].toDouble( &okw );
            if ( !okw || d < 0e0 )
            {
               errormsg = "weights must be non-negative numbers";
               return false;
            }
            weights.push_back( d );
         }
      } else {
         double d = val.toDouble( &ok );
         if ( ok )
         {
            if ( key == "qrgmax" )       { qrgmax       = d; }
            if ( key == "qrgmin" )       { qrgmin       = d; }
            if ( key == "qmin" )         { qmin         = d; }
            if ( key == "qmax" )         { qmax         = d; }
            if ( key == "rgmin" )        { rgmin        = d; }
            if ( key == "rgmax" )        { rgmax        = d; }
            if ( key == "regionfactor" ) { regionfactor = d; }
            if ( key == "qscale" )       { qscale       = d; }
            if ( key == "outlier" )      { outlier      = d; }
            if ( key == "slopet" )       { slopet       = d; }
            if ( key == "minspan" )      { minspan      = d; }
            if ( key == "sigfloor" )     { sigfloor     = d; }
            if ( key == "curvt" )        { curvt        = d; }
            if ( key == "skipz" )        { skipz        = d; }
            if ( key == "fwdz" )         { fwdz         = d; }
            if ( key == "fwddev" )       { fwddev       = d; }
            if ( key == "constol" )      { constol      = d; }
            if ( key == "topfrac" )      { topfrac      = d; }
            if ( key == "aggthresh" )    { aggthresh    = d; }
            if ( key == "aggmindev" )    { aggmindev    = d; }
         }
      }
      if ( !ok )
      {
         errormsg = QString( "invalid value '%1' for %2" ).arg( val ).arg( key );
         return false;
      }
   }

   // range checks
   if ( minpts < 4 )
   {
      errormsg = "minpts must be at least 4";
      return false;
   }
   if ( maxpts && maxpts < minpts )
   {
      errormsg = "maxpts must be 0 or at least minpts";
      return false;
   }
   if ( qrgmax < 0e0 || qrgmin < 0e0 || qmin < 0e0 || qmax < 0e0 || rgmin < 0e0 || rgmax < 0e0 || outlier < 0e0 )
   {
      errormsg = "qrgmax, qrgmin, qmin, qmax, rgmin, rgmax and outlier must be non-negative";
      return false;
   }
   if ( qmax > 0e0 && qmin >= qmax )
   {
      errormsg = "qmin must be below qmax";
      return false;
   }
   if ( rgmax > 0e0 && rgmin >= rgmax )
   {
      errormsg = "rgmin must be below rgmax";
      return false;
   }
   if ( regionfactor < 1e0 )
   {
      errormsg = "regionfactor must be at least 1";
      return false;
   }
   if ( qscale <= 0e0 || curvt <= 0e0 || constol <= 0e0 || aggthresh <= 0e0 || skipz <= 0e0 || fwdz <= 0e0 || fwddev <= 0e0 )
   {
      errormsg = "qscale, curvt, skipz, fwdz, fwddev, constol and aggthresh must be positive";
      return false;
   }
   if ( slopet < 0e0 || minspan < 0e0 || sigfloor < 0e0 || aggmindev < 0e0 )
   {
      errormsg = "slopet, minspan, sigfloor and aggmindev must be non-negative";
      return false;
   }
   if ( topfrac <= 0e0 || topfrac > 1e0 )
   {
      errormsg = "topfrac must be in (0,1]";
      return false;
   }
   double wsum = 0e0;
   for ( int k = 0; k < 7; ++k )
   {
      wsum += weights[ k ];
   }
   if ( wsum <= 0e0 )
   {
      errormsg = "at least one weight must be positive";
      return false;
   }
   return true;
}

// ---------------------------------------------------------------- result

US_Guinier_Search_Result::US_Guinier_Search_Result()
{
   ok             = false;
   rg             = 0e0;
   rg_sd          = 0e0;
   i0             = 0e0;
   i0_sd          = 0e0;
   slope          = 0e0;
   intercept      = 0e0;
   first          = 0;
   last           = 0;
   npts           = 0;
   nskipped       = 0;
   ndropped       = 0;
   nremoved       = 0;
   ntotal         = 0;
   sd_used        = false;
   qmin           = 0e0;
   qmax           = 0e0;
   qrgmin         = 0e0;
   qrgmax         = 0e0;
   chi2_red       = 0e0;
   curv_t         = 0e0;
   quality        = 0e0;
   lowq_z         = 0e0;
   lowq_dev       = 0e0;
   aggregation    = false;
   repulsion      = false;
   nwindows       = 0;
   nspread        = 0;
   rg_spread_mean = 0e0;
   rg_spread_sd   = 0e0;
   rg_spread_min  = 0e0;
   rg_spread_max  = 0e0;
}

QString US_Guinier_Search_Result::json() const
{
   QStringList w;
   for ( int k = 0; k < (int) warnings.size(); ++k )
   {
      w << json_str( warnings[ k ] );
   }
   QString r = "{";
   r += "\"name\":" + json_str( name );
   r += ",\"ok\":" + QString( ok ? "true" : "false" );
   if ( !ok )
   {
      r += ",\"errormsg\":" + json_str( errormsg );
   }
   r += ",\"type\":" + json_str( type );
   r += ",\"rg\":" + json_num( rg );
   r += ",\"rg_sd\":" + json_num( rg_sd );
   r += ",\"i0\":" + json_num( i0 );
   r += ",\"i0_sd\":" + json_num( i0_sd );
   r += ",\"slope\":" + json_num( slope );
   r += ",\"intercept\":" + json_num( intercept );
   r += ",\"first\":" + QString::number( first );
   r += ",\"last\":" + QString::number( last );
   r += ",\"npts\":" + QString::number( npts );
   r += ",\"nskipped\":" + QString::number( nskipped );
   r += ",\"ndropped\":" + QString::number( ndropped );
   r += ",\"nremoved\":" + QString::number( nremoved );
   r += ",\"ntotal\":" + QString::number( ntotal );
   r += ",\"sd_used\":" + QString( sd_used ? "true" : "false" );
   r += ",\"qmin\":" + json_num( qmin );
   r += ",\"qmax\":" + json_num( qmax );
   r += ",\"qrgmin\":" + json_num( qrgmin );
   r += ",\"qrgmax\":" + json_num( qrgmax );
   r += ",\"chi2_red\":" + json_num( chi2_red );
   r += ",\"curv_t\":" + json_num( curv_t );
   r += ",\"quality\":" + json_num( quality );
   r += ",\"lowq_z\":" + json_num( lowq_z );
   r += ",\"lowq_dev\":" + json_num( lowq_dev );
   r += ",\"aggregation\":" + QString( aggregation ? "true" : "false" );
   r += ",\"repulsion\":" + QString( repulsion ? "true" : "false" );
   r += ",\"nwindows\":" + QString::number( nwindows );
   r += ",\"nspread\":" + QString::number( nspread );
   r += ",\"rg_spread_mean\":" + json_num( rg_spread_mean );
   r += ",\"rg_spread_sd\":" + json_num( rg_spread_sd );
   r += ",\"rg_spread_min\":" + json_num( rg_spread_min );
   r += ",\"rg_spread_max\":" + json_num( rg_spread_max );
   r += ",\"warnings\":[" + w.join( "," ) + "]";
   r += "}";
   return r;
}

QString US_Guinier_Search_Result::text_header()
{
   return QString::asprintf( "%-40s %10s %8s %12s %10s %6s %6s %5s %9s %9s %6s %6s %7s %7s %s\n",
                             "name", "Rg", "Rg_sd", "I0", "I0_sd", "first", "last", "npts",
                             "qmin", "qmax", "qRgmin", "qRgmax", "quality", "lowq_z", "flags" );
}

QString US_Guinier_Search_Result::text() const
{
   if ( !ok )
   {
      return QString::asprintf( "%-40s FAILED: %s\n", name.toLatin1().data(), errormsg.toLatin1().data() );
   }
   QStringList flags;
   if ( aggregation )
   {
      flags << "aggregation";
   }
   if ( repulsion )
   {
      flags << "repulsion";
   }
   if ( !sd_used )
   {
      flags << "unweighted";
   }
   if ( nremoved )
   {
      flags << QString( "%1_outliers" ).arg( nremoved );
   }
   QString line =
      QString::asprintf( "%-40s %10.4g %8.3g %12.6g %10.4g %6d %6d %5d %9.5g %9.5g %6.3f %6.3f %7.3f %7.2f %s\n",
                         name.toLatin1().data(), rg, rg_sd, i0, i0_sd, first, last, npts,
                         qmin, qmax, qrgmin, qrgmax, quality, lowq_z,
                         flags.isEmpty() ? "-" : flags.join( "," ).toLatin1().data() );
   for ( int k = 0; k < (int) warnings.size(); ++k )
   {
      line += "    warning: " + warnings[ k ] + "\n";
   }
   return line;
}

QString US_Guinier_Search_Result::csv_header()
{
   return
      "\"name\",\"ok\",\"type\",\"Rg\",\"Rg sd\",\"I(0)\",\"I(0) sd\",\"first point\",\"last point\",\"points used\","
      "\"points skipped\",\"points dropped\",\"outliers removed\",\"points total\",\"SD weighting\","
      "\"q min\",\"q max\",\"q*Rg min\",\"q*Rg max\",\"reduced chi2\",\"curvature t\",\"quality\","
      "\"low-q z\",\"low-q deviation\",\"aggregation\",\"repulsion\",\"windows\","
      "\"Rg spread mean\",\"Rg spread sd\",\"Rg spread min\",\"Rg spread max\",\"warnings\"\n";
}

QString US_Guinier_Search_Result::csv() const
{
   QString w = warnings.join( "; " );
   w.replace( "\"", "'" );
   QStringList f;
   f << "\"" + name + "\""
     << ( ok ? "1" : "0" )
     << type
     << json_num( rg ) << json_num( rg_sd ) << json_num( i0 ) << json_num( i0_sd )
     << QString::number( first ) << QString::number( last ) << QString::number( npts )
     << QString::number( nskipped ) << QString::number( ndropped ) << QString::number( nremoved ) << QString::number( ntotal )
     << ( sd_used ? "1" : "0" )
     << json_num( qmin ) << json_num( qmax ) << json_num( qrgmin ) << json_num( qrgmax )
     << json_num( chi2_red ) << json_num( curv_t ) << json_num( quality )
     << json_num( lowq_z ) << json_num( lowq_dev )
     << ( aggregation ? "1" : "0" ) << ( repulsion ? "1" : "0" )
     << QString::number( nwindows )
     << json_num( rg_spread_mean ) << json_num( rg_spread_sd ) << json_num( rg_spread_min ) << json_num( rg_spread_max )
     << "\"" + ( ok ? w : errormsg ) + "\"";
   return f.join( "," ) + "\n";
}

// ---------------------------------------------------------------- flexible reader

// reads any whitespace or comma separated text file with at least two numeric columns
// ( q, I(q) [, sd] ), skipping every line whose first two fields are not numbers.
// the sd column is kept only if every data line has a numeric third field.
bool US_Saxs_Util::read_iq_flexible( const QString & filename, const QString & tag, double qscale )
{
   errormsg = "";
   wave[ tag ].clear();
   wave[ tag ].filename = filename;

   QFile f( filename );
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = "could not open file " + filename;
      return false;
   }

   QTextStream ts( &f );
   QRegularExpression rx_sep( "[\\s,;]+" );
   vector < double > sds;
   bool all_have_sd = true;
   QString header;

   while ( !ts.atEnd() )
   {
      QString line = ts.readLine().trimmed();
      if ( line.isEmpty() )
      {
         continue;
      }
      QStringList toks = line.split( rx_sep, Qt::SkipEmptyParts );
      if ( toks.size() < 2 )
      {
         if ( wave[ tag ].q.empty() && header.isEmpty() )
         {
            header = line;
         }
         continue;
      }
      bool okq;
      bool oki;
      double q = toks[ 0 ].toDouble( &okq );
      double I = toks[ 1 ].toDouble( &oki );
      if ( !okq || !oki )
      {
         if ( wave[ tag ].q.empty() && header.isEmpty() )
         {
            header = line;
         }
         continue;
      }
      wave[ tag ].q.push_back( q * qscale );
      wave[ tag ].r.push_back( I );
      bool oks = false;
      double sd = toks.size() >= 3 ? toks[ 2 ].toDouble( &oks ) : 0e0;
      if ( !oks )
      {
         all_have_sd = false;
         sd = 0e0;
      }
      sds.push_back( sd );
   }
   f.close();

   if ( wave[ tag ].q.empty() )
   {
      errormsg = "no numeric q, I(q) lines found in " + filename;
      return false;
   }
   wave[ tag ].header = header.isEmpty() ? QString( "q I(q) error" ) : header;
   if ( all_have_sd )
   {
      wave[ tag ].s = sds;
   }
   return true;
}

// ---------------------------------------------------------------- guinier_search

bool US_Saxs_Util::guinier_search( const QString & tag, const US_Guinier_Search_Params & params, US_Guinier_Search_Result & result )
{
   errormsg = "";
   result   = US_Guinier_Search_Result();
   result.name = wave.count( tag ) && !wave[ tag ].filename.isEmpty() ? wave[ tag ].filename : tag;
   result.type = params.type;

   if ( !wave.count( tag ) )
   {
      result.errormsg = errormsg = "no data named " + tag;
      return false;
   }

   {
      QString perr;
      US_Guinier_Search_Params check = params;
      map < QString, QString > empty;
      if ( !check.set( empty, perr ) )
      {
         result.errormsg = errormsg = perr;
         return false;
      }
   }

   const vector < double > & q  = wave[ tag ].q;
   const vector < double > & I  = wave[ tag ].r;
   const vector < double > & sd = wave[ tag ].s;
   int ntotal = (int) q.size();
   result.ntotal = ntotal;

   if ( I.size() != q.size() )
   {
      result.errormsg = errormsg = "q and I(q) have different lengths";
      return false;
   }

   double mult   = params.type == "rg" ? 3e0 : ( params.type == "rc" ? 2e0 : 1e0 );
   int    lnpow  = params.type == "rg" ? 0 : ( params.type == "rc" ? 1 : 2 );
   double qrgmax = params.effective_qrgmax();
   bool   have_sd = sd.size() == q.size() && !sd.empty();

   // ---- 1. clean
   vector < int >    idx;
   vector < double > qv;
   vector < double > x;
   vector < double > y;
   vector < double > ey;
   int ndropped = 0;
   int nbadsd   = 0;

   for ( int k = 0; k < ntotal; ++k )
   {
      double qq = q[ k ] * params.qscale;
      double ii = I[ k ];
      if ( !std::isfinite( qq ) || !std::isfinite( ii ) )
      {
         ++ndropped;
         continue;
      }
      if ( qq < 0e0 || ( lnpow && qq <= 0e0 ) )
      {
         ++ndropped;
         continue;
      }
      if ( params.qmin > 0e0 && qq < params.qmin )
      {
         continue;
      }
      if ( params.qmax > 0e0 && qq > params.qmax )
      {
         continue;
      }
      if ( ii <= 0e0 )
      {
         if ( !params.dropneg )
         {
            result.errormsg = errormsg = QString( "non-positive intensity at q = %1 (point %2); set dropneg 1 to skip such points" )
               .arg( qq ).arg( k + 1 );
            return false;
         }
         ++ndropped;
         continue;
      }
      double yy = log( ii ) + ( lnpow ? lnpow * log( qq ) : 0e0 );
      if ( !std::isfinite( yy ) )
      {
         ++ndropped;
         continue;
      }
      double e = 0e0;
      if ( have_sd )
      {
         e = sd[ k ];
         if ( !std::isfinite( e ) || e <= 0e0 )
         {
            ++nbadsd;
            e = 0e0;
         } else {
            e /= ii;   // d ln I
         }
      }
      idx.push_back( k );
      qv .push_back( qq );
      x  .push_back( qq * qq );
      y  .push_back( yy );
      ey .push_back( e );
   }
   result.ndropped = ndropped;
   if ( ndropped )
   {
      result.warnings << QString( "%1 non-positive or non-finite points dropped" ).arg( ndropped );
   }

   int m = (int) qv.size();
   if ( m < params.minpts )
   {
      result.errormsg = errormsg = QString( "too few usable points (%1) for minpts %2" ).arg( m ).arg( params.minpts );
      return false;
   }

   // sort by q if needed
   {
      bool sorted = true;
      for ( int k = 1; k < m; ++k )
      {
         if ( qv[ k ] < qv[ k - 1 ] )
         {
            sorted = false;
            break;
         }
      }
      if ( !sorted )
      {
         vector < int > order( m );
         for ( int k = 0; k < m; ++k )
         {
            order[ k ] = k;
         }
         stable_sort( order.begin(), order.end(), [ & ]( int a, int b ) { return qv[ a ] < qv[ b ]; } );
         vector < int >    idx2;
         vector < double > qv2;
         vector < double > x2;
         vector < double > y2;
         vector < double > ey2;
         for ( int k = 0; k < m; ++k )
         {
            idx2.push_back( idx[ order[ k ] ] );
            qv2 .push_back( qv [ order[ k ] ] );
            x2  .push_back( x  [ order[ k ] ] );
            y2  .push_back( y  [ order[ k ] ] );
            ey2 .push_back( ey [ order[ k ] ] );
         }
         idx = idx2;
         qv  = qv2;
         x   = x2;
         y   = y2;
         ey  = ey2;
         result.warnings << "q values were not in increasing order: points sorted by q";
      }
   }
   if ( qv.back() > 2e0 )
   {
      result.warnings << QString( "maximum q is %1: data may be in nm^-1, consider qscale 0.1" ).arg( qv.back() );
   }

   // ---- SD decision
   bool sd_ok = have_sd && !nbadsd;
   bool use_sd;
   if ( params.usesd == 0 )
   {
      use_sd = false;
   } else if ( params.usesd == 1 ) {
      if ( !sd_ok )
      {
         result.errormsg = errormsg =
            have_sd
            ? QString( "usesd 1 requested but %1 SDs are zero, negative or non-finite" ).arg( nbadsd )
            : QString( "usesd 1 requested but the data has no SD column" );
         return false;
      }
      use_sd = true;
   } else {
      use_sd = sd_ok;
      if ( have_sd && !sd_ok )
      {
         result.warnings << QString( "SDs present but %1 are zero, negative or non-finite: unweighted fit" ).arg( nbadsd );
      }
   }
   result.sd_used = use_sd;

   vector < double > w( m, 1e0 );
   if ( use_sd )
   {
      for ( int k = 0; k < m; ++k )
      {
         w[ k ] = 1e0 / ( ey[ k ] * ey[ k ] );
      }
   }

   auto sub = []( const vector < double > & v, int i, int j ) {
      return vector < double >( v.begin() + i, v.begin() + j + 1 );
   };
   // per point sigma in ln I for residual tests
   auto sigma_of = [ & ]( int p, const linfit & f ) {
      return use_sd ? ey[ p ] : std::max( f.rms, params.sigfloor );
   };

   // ---- 2. preliminary Rg and search region
   //      for each start i ( up to a cap ) grow the window while the slope is either not yet significant or
   //      still within q*Rg <= qrgmax; the longest valid window over all starts gives the preliminary Rg.
   //      this steps over a low-q upturn ( a short valid window of large Rg ) in favour of the particle
   //      ( a long valid window ), and handles noisy dense data where short windows have no significant slope.
   //      incremental sums keep each fit O(1).
   double rg0       = 0e0;
   double rg0_first = 0e0;
   int    pre_i     = -1;
   int    pre_j     = -1;
   {
      int best_len = 0;
      int scanned  = 0;
      for ( int i = 0; i + params.minpts - 1 < m; ++i )
      {
         if ( pre_j >= 0 && i > pre_j )
         {
            break;
         }
         if ( ++scanned > 1000 )
         {
            break;
         }
         double sw = 0e0;
         double sx = 0e0;
         double sy = 0e0;
         double sxx = 0e0;
         double sxy = 0e0;
         double syy = 0e0;
         int    last_j  = -1;
         double last_rg = 0e0;
         for ( int j = i; j < m; ++j )
         {
            double wk = w[ j ];
            sw  += wk;
            sx  += wk * x[ j ];
            sy  += wk * y[ j ];
            sxx += wk * x[ j ] * x[ j ];
            sxy += wk * x[ j ] * y[ j ];
            syy += wk * y[ j ] * y[ j ];
            int n = j - i + 1;
            if ( n < params.minpts )
            {
               continue;
            }
            double st2 = sxx - sx * sx / sw;
            if ( !( st2 > 0e0 ) )
            {
               continue;
            }
            double b    = ( sxy - sx * sy / sw ) / st2;
            double a    = ( sy - sx * b ) / sw;
            double chi2 = syy - a * sy - b * sxy;
            if ( chi2 < 0e0 )
            {
               chi2 = 0e0;
            }
            double scale;
            if ( use_sd )
            {
               double chi2_red = n > 2 ? chi2 / ( n - 2 ) : 1e0;
               scale = chi2_red > 1e0 ? chi2_red : 1e0;
            } else {
               scale = n > 2 ? chi2 / ( n - 2 ) : 0e0;
            }
            double sigb = sqrt( scale / st2 );
            if ( !( b < 0e0 ) || !std::isfinite( b ) )
            {
               continue;
            }
            double rg = sqrt( -mult * b );
            if ( rg0_first == 0e0 )
            {
               rg0_first = rg;
            }
            bool sig = params.slopet <= 0e0 || ( sigb > 0e0 && -b / sigb >= params.slopet );
            if ( !sig )
            {
               continue;
            }
            if ( qv[ j ] * rg > qrgmax )
            {
               break;
            }
            last_j  = j;
            last_rg = rg;
         }
         if ( last_j >= 0 && ( qv[ last_j ] - qv[ i ] ) * last_rg >= params.minspan )
         {
            int len = last_j - i + 1;
            if ( len > best_len )
            {
               best_len = len;
               pre_i    = i;
               pre_j    = last_j;
               rg0      = last_rg;
            }
         }
      }
   }
   int nreg      = m;
   int nguinier0 = m;
   double rgb    = rg0 > 0e0 ? rg0 : rg0_first;
   if ( rgb > 0e0 )
   {
      double qlim = params.regionfactor * qrgmax / rgb;
      nreg      = (int) ( std::upper_bound( qv.begin(), qv.end(), qlim ) - qv.begin() );
      nguinier0 = (int) ( std::upper_bound( qv.begin(), qv.end(), qrgmax / rgb ) - qv.begin() );
      int nmin = std::min( m, 3 * params.minpts );
      if ( nreg < nmin )
      {
         nreg = nmin;
      }
   } else {
      nreg = std::min( m, 30 * params.minpts );
   }
   if ( params.debug )
   {
      QTextStream( stderr )
         << QString( "guinier_search %1: %2 usable points, preliminary Rg %3 from window %4-%5 "
                     "(first negative slope Rg %6), search region %7 points\n" )
         .arg( result.name ).arg( m ).arg( rg0 ).arg( pre_i + 1 ).arg( pre_j + 1 ).arg( rg0_first ).arg( nreg );
   }

   // ---- 3. enumerate windows
   vector < window > cands;
   for ( int i = 0; i + params.minpts - 1 < nreg; ++i )
   {
      int violations = 0;
      for ( int j = i + params.minpts - 1; j < nreg; ++j )
      {
         int n = j - i + 1;
         if ( params.maxpts && n > params.maxpts )
         {
            break;
         }
         window c;
         c.i = i;
         c.j = j;
         c.n = n;
         vector < double > xs = sub( x, i, j );
         vector < double > ys = sub( y, i, j );
         vector < double > ws = sub( w, i, j );
         fit_line( xs, ys, ws, use_sd, c.f );
         if ( !( c.f.b < 0e0 ) || !std::isfinite( c.f.b ) )
         {
            continue;
         }
         c.rg   = sqrt( -mult * c.f.b );
         c.qrg0 = qv[ i ] * c.rg;
         c.qrg1 = qv[ j ] * c.rg;
         if ( c.qrg1 > qrgmax )
         {
            if ( ++violations >= 3 )
            {
               break;
            }
            continue;
         }
         violations = 0;
         if ( params.qrgmin > 0e0 && c.qrg0 > params.qrgmin )
         {
            continue;
         }
         if ( params.rgmin > 0e0 && c.rg < params.rgmin )
         {
            continue;
         }
         if ( params.rgmax > 0e0 && c.rg > params.rgmax )
         {
            continue;
         }
         if ( params.slopet > 0e0 && ( c.f.sigb <= 0e0 || -c.f.b / c.f.sigb < params.slopet ) )
         {
            continue;
         }
         if ( c.qrg1 - c.qrg0 < params.minspan )
         {
            continue;
         }
         c.curv_t = curvature_t( xs, ys, ws, use_sd, params.sigfloor );

         // skipped low-q points that are consistent with the line count against the window
         int nskip_pen = 0;
         for ( int p = 0; p < i; ++p )
         {
            double r = y[ p ] - c.f.a - c.f.b * x[ p ];
            double s = sigma_of( p, c.f );
            if ( s > 0e0 && fabs( r ) <= params.skipz * s )
            {
               ++nskip_pen;
            }
         }
         // forward consistency: points beyond the window but inside q*Rg <= qrgmax must follow the line;
         // tested by z-score, penalized by the mean deviation in ln I so precise data are not over-penalized
         double zfwd = 0e0;
         double dfwd = 0e0;
         {
            double sum  = 0e0;
            double dsum = 0e0;
            int    cnt  = 0;
            for ( int p = j + 1; p < m && qv[ p ] * c.rg <= qrgmax; ++p )
            {
               double r = y[ p ] - c.f.a - c.f.b * x[ p ];
               double s = sigma_of( p, c.f );
               if ( s > 0e0 )
               {
                  sum  += r / s;
                  dsum += r;
                  ++cnt;
               }
            }
            zfwd = cnt ? sum / sqrt( (double) cnt ) : 0e0;
            dfwd = cnt ? dsum / cnt : 0e0;
         }

         // terms
         c.terms[ 0 ] = use_sd && c.f.chi2_red > 1e0 ? 1e0 / c.f.chi2_red : 1e0;
         c.terms[ 1 ] = 1e0 / ( 1e0 + ( c.curv_t / params.curvt ) * ( c.curv_t / params.curvt ) );
         c.terms[ 2 ] = std::min( 1e0, ( c.qrg1 - c.qrg0 ) / qrgmax );
         int nguinier = (int) ( std::upper_bound( qv.begin(), qv.end(), qrgmax / c.rg ) - qv.begin() );
         c.terms[ 3 ] = nguinier > 0 ? std::min( 1e0, (double) n / nguinier ) : 0e0;
         c.terms[ 4 ] = 1e0 / ( 1e0 + (double) nskip_pen / n );
         c.terms[ 5 ] = fabs( zfwd ) > params.fwdz ? 1e0 / ( 1e0 + ( dfwd / params.fwddev ) * ( dfwd / params.fwddev ) ) : 1e0;
         c.terms[ 6 ] = 1e0;
         c.q0 = 1e0;
         for ( int t = 0; t < 6; ++t )
         {
            c.q0 *= pow( c.terms[ t ], params.weights[ t ] );
         }
         c.quality = 0e0;
         cands.push_back( c );
      }
   }
   result.nwindows = (int) cands.size();
   if ( cands.empty() )
   {
      if ( rgb > 0e0 && nguinier0 < params.minpts )
      {
         result.errormsg = errormsg =
            QString( "no valid Guinier window: only %1 points have q*Rg <= %2 for the preliminary Rg %3, fewer than minpts %4" )
            .arg( nguinier0 ).arg( qrgmax ).arg( rgb, 0, 'g', 4 ).arg( params.minpts );
      } else {
         result.errormsg = errormsg =
            "no valid Guinier window found (non-negative or insignificant slopes, or constraints exclude every window)";
      }
      return false;
   }

   // ---- 4. consensus and quality
   double wsum = 0e0;
   for ( int t = 0; t < 7; ++t )
   {
      wsum += params.weights[ t ];
   }
   double rg_ref;
   {
      vector < pair < double, double > > rq;   // rg, weight
      double tot = 0e0;
      for ( int k = 0; k < (int) cands.size(); ++k )
      {
         rq.push_back( make_pair( cands[ k ].rg, cands[ k ].q0 ) );
         tot += cands[ k ].q0;
      }
      sort( rq.begin(), rq.end() );
      rg_ref = rq.back().first;
      if ( tot > 0e0 )
      {
         double cum = 0e0;
         for ( int k = 0; k < (int) rq.size(); ++k )
         {
            cum += rq[ k ].second;
            if ( cum >= 0.5e0 * tot )
            {
               rg_ref = rq[ k ].first;
               break;
            }
         }
      }
   }
   int best = 0;
   for ( int k = 0; k < (int) cands.size(); ++k )
   {
      window & c = cands[ k ];
      double dev = ( c.rg - rg_ref ) / ( params.constol * rg_ref );
      c.terms[ 6 ] = exp( -dev * dev );
      c.quality = pow( c.q0 * pow( c.terms[ 6 ], params.weights[ 6 ] ), 1e0 / wsum );
      if ( c.quality > cands[ best ].quality ||
           ( c.quality == cands[ best ].quality && c.n > cands[ best ].n ) )
      {
         best = k;
      }
   }
   if ( params.debug )
   {
      QTextStream ts( stderr );
      ts << QString( "guinier_search %1: consensus Rg %2, %3 candidate windows\n" )
         .arg( result.name ).arg( rg_ref ).arg( cands.size() );
      ts << "    i    j    n       Rg   qRg0   qRg1  chi2r  curvt   chi2  curv   span   cov  start   fwd   cons  quality\n";
      for ( int k = 0; k < (int) cands.size(); ++k )
      {
         const window & c = cands[ k ];
         ts << QString::asprintf( "%5d %5d %4d %8.3f %6.3f %6.3f %6.2f %6.2f %6.3f %5.3f %6.3f %5.3f %6.3f %5.3f %6.3f %8.4f%s\n",
                                  c.i, c.j, c.n, c.rg, c.qrg0, c.qrg1, c.f.chi2_red, c.curv_t,
                                  c.terms[ 0 ], c.terms[ 1 ], c.terms[ 2 ], c.terms[ 3 ], c.terms[ 4 ], c.terms[ 5 ], c.terms[ 6 ],
                                  c.quality, k == best ? "  <== best" : "" );
      }
   }

   // ---- 5. best window, optional outlier rejection
   window bw = cands[ best ];
   vector < int >    keep;
   for ( int k = bw.i; k <= bw.j; ++k )
   {
      keep.push_back( k );
   }
   linfit bf = bw.f;
   int nremoved = 0;
   if ( params.outlier > 0e0 )
   {
      for ( ;; )
      {
         vector < int > kept;
         for ( int k = 0; k < (int) keep.size(); ++k )
         {
            int    p = keep[ k ];
            double r = y[ p ] - bf.a - bf.b * x[ p ];
            double s = sigma_of( p, bf );
            if ( s > 0e0 && fabs( r ) > params.outlier * s )
            {
               continue;
            }
            kept.push_back( p );
         }
         if ( kept.size() == keep.size() )
         {
            break;
         }
         if ( (int) kept.size() < params.minpts )
         {
            result.warnings << "outlier rejection stopped: fewer than minpts points would remain";
            break;
         }
         nremoved += (int) ( keep.size() - kept.size() );
         keep = kept;
         vector < double > xs;
         vector < double > ys;
         vector < double > ws;
         for ( int k = 0; k < (int) keep.size(); ++k )
         {
            xs.push_back( x[ keep[ k ] ] );
            ys.push_back( y[ keep[ k ] ] );
            ws.push_back( w[ keep[ k ] ] );
         }
         fit_line( xs, ys, ws, use_sd, bf );
         if ( !( bf.b < 0e0 ) )
         {
            result.errormsg = errormsg = "slope became non-negative during outlier rejection";
            return false;
         }
      }
      if ( nremoved )
      {
         result.warnings << QString( "%1 outlier points removed from the window" ).arg( nremoved );
      }
   }
   result.nremoved = nremoved;

   double rg = sqrt( -mult * bf.b );
   result.rg        = rg;
   result.rg_sd     = mult * bf.sigb / ( 2e0 * rg );
   result.i0        = exp( bf.a );
   result.i0_sd     = result.i0 * bf.siga;
   result.slope     = bf.b;
   result.intercept = bf.a;
   result.first     = idx[ keep.front() ] + 1;
   result.last      = idx[ keep.back() ] + 1;
   result.npts      = (int) keep.size();
   result.nskipped  = bw.i;
   result.qmin      = qv[ keep.front() ];
   result.qmax      = qv[ keep.back() ];
   result.qrgmin    = result.qmin * rg;
   result.qrgmax    = result.qmax * rg;
   result.chi2_red  = bf.chi2_red;
   result.curv_t    = bw.curv_t;
   result.quality   = bw.quality;

   // ---- low-q test: skipped points plus the first third of the window, against the line
   //      fitted to the remaining two thirds ( falls back to the full window fit when that is too short )
   {
      int    nhead = std::max( 3, (int) keep.size() / 3 );
      linfit tf    = bf;
      if ( (int) keep.size() - nhead >= std::max( 4, params.minpts / 2 ) )
      {
         vector < double > xs;
         vector < double > ys;
         vector < double > ws;
         for ( int k = nhead; k < (int) keep.size(); ++k )
         {
            xs.push_back( x[ keep[ k ] ] );
            ys.push_back( y[ keep[ k ] ] );
            ws.push_back( w[ keep[ k ] ] );
         }
         fit_line( xs, ys, ws, use_sd, tf );
      }
      double sum  = 0e0;
      double dsum = 0e0;
      int    cnt  = 0;
      for ( int p = 0; p < bw.i; ++p )
      {
         double r = y[ p ] - tf.a - tf.b * x[ p ];
         double s = sigma_of( p, tf );
         if ( s > 0e0 )
         {
            sum  += r / s;
            dsum += r;
            ++cnt;
         }
      }
      for ( int k = 0; k < nhead && k < (int) keep.size(); ++k )
      {
         int    p = keep[ k ];
         double r = y[ p ] - tf.a - tf.b * x[ p ];
         double s = sigma_of( p, tf );
         if ( s > 0e0 )
         {
            sum  += r / s;
            dsum += r;
            ++cnt;
         }
      }
      result.lowq_z      = cnt ? sum / sqrt( (double) cnt ) : 0e0;
      result.lowq_dev    = cnt ? dsum / cnt : 0e0;
      result.aggregation = result.lowq_z >  params.aggthresh && result.lowq_dev >  params.aggmindev;
      result.repulsion   = result.lowq_z < -params.aggthresh && result.lowq_dev < -params.aggmindev;
      if ( result.aggregation )
      {
         result.warnings << QString( "low-q points lie above the Guinier line (z = %1, mean %2%): possible aggregation" )
            .arg( result.lowq_z, 0, 'f', 1 ).arg( 100e0 * result.lowq_dev, 0, 'f', 1 );
      }
      if ( result.repulsion )
      {
         result.warnings << QString( "low-q points lie below the Guinier line (z = %1, mean %2%): possible repulsive interactions" )
            .arg( result.lowq_z, 0, 'f', 1 ).arg( 100e0 * result.lowq_dev, 0, 'f', 1 );
      }
   }

   // ---- spread over the top fraction of windows
   {
      vector < pair < double, double > > qr;   // quality, rg
      for ( int k = 0; k < (int) cands.size(); ++k )
      {
         qr.push_back( make_pair( cands[ k ].quality, cands[ k ].rg ) );
      }
      sort( qr.begin(), qr.end() );
      int ktop = std::max( 3, (int) ceil( params.topfrac * qr.size() ) );
      ktop = std::min( ktop, (int) qr.size() );
      double s1 = 0e0;
      double s2 = 0e0;
      double mn = 1e99;
      double mx = -1e99;
      for ( int k = (int) qr.size() - ktop; k < (int) qr.size(); ++k )
      {
         double v = qr[ k ].second;
         s1 += v;
         s2 += v * v;
         mn = std::min( mn, v );
         mx = std::max( mx, v );
      }
      result.nspread        = ktop;
      result.rg_spread_mean = s1 / ktop;
      result.rg_spread_sd   = ktop > 1 ? sqrt( std::max( 0e0, ( s2 - s1 * s1 / ktop ) / ( ktop - 1 ) ) ) : 0e0;
      result.rg_spread_min  = mn;
      result.rg_spread_max  = mx;
   }

   result.ok = true;
   return true;
}

// ---------------------------------------------------------------- json run type

// input keys: "guinier_search" (run type), "files" (JSON array of file names) or "q","i"[,"e"] arrays,
//             plus any US_Guinier_Search_Params key
// output:     "results" JSON array with one object per curve, "nok" successes, "params" effective parameters
bool US_Saxs_Util::run_guinier_search(
                              map < QString, QString >           & parameters,
                              map < QString, QString >           & results
                              )
{
   US_Guinier_Search_Params params;
   QString perr;
   if ( !params.set( parameters, perr ) )
   {
      results[ "errors" ] = perr;
      return false;
   }

   QStringList tags;

   if ( parameters.count( "files" ) )
   {
      QString fl = parameters[ "files" ];
      fl.replace( "\\/", "/" ).replace( QRegularExpression( "^\\s*\\[|\\]\\s*$" ), "" );
      QStringList files = fl.split( QRegularExpression( "\"\\s*,\\s*\"" ), Qt::SkipEmptyParts );
      for ( int k = 0; k < (int) files.size(); ++k )
      {
         QString fn = files[ k ].trimmed();
         fn.replace( QRegularExpression( "^\"|\"$" ), "" );
         if ( fn.isEmpty() )
         {
            continue;
         }
         tags << fn;
      }
      if ( tags.isEmpty() )
      {
         results[ "errors" ] = "files is empty";
         return false;
      }
   } else if ( parameters.count( "q" ) && parameters.count( "i" ) ) {
      QString tag = parameters.count( "name" ) ? parameters[ "name" ] : QString( "inline" );
      tag.replace( QRegularExpression( "^\"|\"$" ), "" );
      wave[ tag ].clear();
      wave[ tag ].filename = tag;
      QString derr;
      if ( !US_Json::decode_array_to_vector_double( parameters[ "q" ], wave[ tag ].q, derr )
           || !US_Json::decode_array_to_vector_double( parameters[ "i" ], wave[ tag ].r, derr ) )
      {
         results[ "errors" ] = "could not decode q or i arrays";
         return false;
      }
      if ( parameters.count( "e" ) )
      {
         if ( !US_Json::decode_array_to_vector_double( parameters[ "e" ], wave[ tag ].s, derr ) )
         {
            results[ "errors" ] = "could not decode e array";
            return false;
         }
         if ( wave[ tag ].s.size() != wave[ tag ].q.size() )
         {
            results[ "errors" ] = "e array length differs from q";
            return false;
         }
      }
      if ( wave[ tag ].r.size() != wave[ tag ].q.size() )
      {
         results[ "errors" ] = "i array length differs from q";
         return false;
      }
      tags << tag;
   } else {
      results[ "errors" ] = "guinier_search needs files or q and i arrays";
      return false;
   }

   QStringList out;
   int nok = 0;
   for ( int k = 0; k < (int) tags.size(); ++k )
   {
      US_Guinier_Search_Result r;
      if ( parameters.count( "files" ) && !read_iq_flexible( tags[ k ], tags[ k ], 1e0 ) )
      {
         r.name     = tags[ k ];
         r.ok       = false;
         r.errormsg = errormsg;
      } else {
         guinier_search( tags[ k ], params, r );
      }
      if ( r.ok )
      {
         ++nok;
      }
      out << r.json();
   }

   results[ "results" ] = "[" + out.join( "," ) + "]";
   results[ "nok" ]     = QString::number( nok );
   {
      QString p = "{";
      p += "\"type\":" + json_str( params.type );
      p += ",\"minpts\":" + QString::number( params.minpts );
      p += ",\"maxpts\":" + QString::number( params.maxpts );
      p += ",\"qrgmax\":" + json_num( params.effective_qrgmax() );
      p += ",\"qrgmin\":" + json_num( params.qrgmin );
      p += ",\"qmin\":" + json_num( params.qmin );
      p += ",\"qmax\":" + json_num( params.qmax );
      p += ",\"rgmin\":" + json_num( params.rgmin );
      p += ",\"rgmax\":" + json_num( params.rgmax );
      p += ",\"regionfactor\":" + json_num( params.regionfactor );
      p += ",\"usesd\":" + QString::number( params.usesd );
      p += ",\"dropneg\":" + QString::number( params.dropneg ? 1 : 0 );
      p += ",\"qscale\":" + json_num( params.qscale );
      p += ",\"outlier\":" + json_num( params.outlier );
      p += ",\"slopet\":" + json_num( params.slopet );
      p += ",\"minspan\":" + json_num( params.minspan );
      p += ",\"sigfloor\":" + json_num( params.sigfloor );
      p += ",\"curvt\":" + json_num( params.curvt );
      p += ",\"skipz\":" + json_num( params.skipz );
      p += ",\"fwdz\":" + json_num( params.fwdz );
      p += ",\"fwddev\":" + json_num( params.fwddev );
      p += ",\"constol\":" + json_num( params.constol );
      p += ",\"topfrac\":" + json_num( params.topfrac );
      p += ",\"aggthresh\":" + json_num( params.aggthresh );
      p += ",\"aggmindev\":" + json_num( params.aggmindev );
      p += ",\"weights\":" + US_Json::encode_vector_double( params.weights );
      p += "}";
      results[ "params" ] = p;
   }
   if ( !nok )
   {
      results[ "errors" ] = "no curve could be analyzed";
      return false;
   }
   return true;
}
