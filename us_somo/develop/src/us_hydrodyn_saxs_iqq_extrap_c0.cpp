#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_saxs_iqq_extrap_c0_conc.h"
#include "../include/us_hydrodyn_saxs_iqq_extrap_c0_regplot.h"
#include <QRegularExpression>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <set>
#include <algorithm>
#include <limits>
#include <cmath>
#include "../include/Eigen/Dense"

#define TSO QTextStream(stdout)

static QString us_extrap_c0_common_prefix( const QStringList &names )
{
   if ( names.isEmpty() )
   {
      return "";
   }

   QString prefix = names[ 0 ];
   prefix.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );

   for ( int i = 1; i < names.size() && !prefix.isEmpty(); i++ )
   {
      QString cur = names[ i ];
      cur.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );

      int len    = 0;
      int maxlen = qMin( prefix.length(), cur.length() );
      while ( len < maxlen && prefix[ len ] == cur[ len ] )
      {
         len++;
      }
      prefix = prefix.left( len );
   }
   return prefix;
}

static QString us_extrap_c0_curve_name( const QStringList &names, int model, bool ref_scale,
                                        bool merge_ref, double merge_q, int sd_mode,
                                        int n_outlier_dropped, bool unweighted, int ri_mode,
                                        bool no_gcv, int fit_broaden, double ref_override_conc,
                                        bool raw_residual )
{
   // Method token encoding the choices that produced the curve, so distinct selections give
   // distinct (self-describing) names rather than colliding on a bare -1/-2 suffix.
   //
   // EVERY option that can change the result must appear here, otherwise two runs land on the same
   // name and are told apart only by an arbitrary -1 suffix. A token is emitted only for a
   // NON-DEFAULT choice, so an ordinary run keeps a short name.
   QString method = "_extrap_c0";
   method += ( model == 2 ) ? "_virial2" : ( model == 1 ) ? "_recip" : "_add";
   if ( ref_scale ) { method += "_abs"; }
   if ( merge_ref )
   {
      method += "_merge";
      if ( merge_q > 0e0 )
      {
         method += "_q" + QString::number( merge_q, 'f', 4 ).replace( '.', '_' );
      }
      // merge_q == 0 => splice requested but no cutover located (whole curve extrapolated)
   }
   // regularization: GCV is the default, so only its absence is marked. The manual fit-broadening
   // window only does anything when GCV is off, and then it changes the result, so record it too.
   if ( no_gcv ) { method += "_nogcv"; }
   if ( fit_broaden > 1 ) { method += "_fb" + QString::number( fit_broaden ); }
   // a user-chosen reference curve changes the absolute scale and/or where the splice cuts over
   if ( ref_override_conc > 0e0 )
   {
      method += "_refc" + QString::number( ref_override_conc, 'f', 4 ).replace( '.', '_' );
   }
   // input-SD recompute (before the fit) precedes the output SD reassessment token
   if ( ri_mode == 1 ) { method += "_riC"; }
   else if ( ri_mode == 2 ) { method += "_riN"; }
   else if ( ri_mode == 3 ) { method += "_riI"; }
   if ( sd_mode == 1 ) { method += "_sdC"; }
   else if ( sd_mode == 2 ) { method += "_sdN"; }
   else if ( sd_mode == 3 ) { method += "_sdI"; }
   if ( n_outlier_dropped > 0 )
   {
      method += "_qc" + QString::number( n_outlier_dropped );
      // the leverage correction can change WHICH curve is dropped, so two runs could otherwise
      // both read _qc1 while resting on different curves
      if ( raw_residual ) { method += "_rawres"; }
   }
   if ( unweighted ) { method += "_uw"; }

   QString prefix = us_extrap_c0_common_prefix( names ).trimmed();
   prefix.remove( QRegularExpression( "[\\s_-]+$" ) );

   if ( prefix.isEmpty() )
   {
      return method.mid( 1 );                    // drop the leading underscore when no prefix
   }
   return prefix + method;
}

// Penalized-slope extrapolation core with automatic GCV smoothing (shared by Zimm and
// absolute-scale). At each q the model is  y_i = c_i*alpha(q) + Iex(q)  (inverse-variance
// weighted); a global smoothness penalty is applied to ONLY the concentration slope
// alpha(q) -- the interparticle/second-virial term, which is smooth in q and dies off at
// high q -- while the intercept Iex(q) (which carries the form-factor detail) is left
// unpenalized. Given the per-q weighted sufficient statistics
//    M = sum w c^2 - (sum w c)^2/(sum w)   (slope Fisher info; per-q OLS slope = R/M)
//    R = sum w c y - (sum w c)(sum w y)/(sum w)
//    B = sum w c,  C = sum w,  Q = sum w y
// the fit reduces to  (diag(M) + lambda * D2'D2) alpha = R ,  Iex = (Q - B*alpha)/C ,
// where D2 is the second-difference operator. lambda is chosen automatically by
// Generalized Cross-Validation: one symmetric eigendecomposition of the whitened penalty
// K = M^-1/2 (D2'D2) M^-1/2 makes the whole GCV scan closed-form
// (GCV(lambda) = (RSS/n)/(1-edof/n)^2, edof = sum 1/(1+lambda*g)). lambda=0 recovers the
// independent per-q weighted fit. Validated to track the ground-truth-optimal lambda
// across simulated dilution series (weak/strong, repulsive/attractive interactions) and
// to give a cleaner low-q Guinier than either per-q OLS or ATSAS almerge on real data.
// Returns false on a degenerate problem, in which case the caller keeps its per-q result.
static bool us_extrap_c0_gcv_penalized(
                                       const vector < double > & M,
                                       const vector < double > & R,
                                       const vector < double > & B,
                                       const vector < double > & C,
                                       const vector < double > & Q,
                                       vector < double > & Iex_out,
                                       vector < double > & alpha_out,
                                       double & lambda_out,
                                       double & edof_out )
{
   int n = (int) M.size();
   if ( n < 5 )
   {
      return false;
   }
   // the dense symmetric eigendecomposition below is O(n^3); guard against pathologically
   // large grids so we fall back to the per-q fit rather than hang (a banded eigensolver
   // would lift this limit; typical SAXS grids are well under it)
   if ( n > 5000 )
   {
      return false;
   }

   // floor for degenerate q (M<=0, e.g. all-NaN column) so the whitening stays bounded
   vector < double > pos;
   for ( int i = 0; i < n; i++ )
   {
      if ( M[ i ] > 0e0 )
      {
         pos.push_back( M[ i ] );
      }
   }
   if ( (int) pos.size() < n / 2 )
   {
      return false;                            // too many degenerate q to regularize
   }
   std::sort( pos.begin(), pos.end() );
   double medM   = pos[ pos.size() / 2 ];
   double floorM = 1e-6 * medM;

   Eigen::VectorXd Mh( n ), Mih( n ), a0( n );
   for ( int i = 0; i < n; i++ )
   {
      double m = ( M[ i ] > floorM ) ? M[ i ] : floorM;
      Mh[ i ]  = std::sqrt( m );
      Mih[ i ] = 1e0 / Mh[ i ];
      a0[ i ]  = ( M[ i ] > 0e0 ) ? R[ i ] / M[ i ] : 0e0;   // per-q OLS slope
   }

   // second-difference operator D2 ((n-2) x n) and its Gram L = D2'D2 (pentadiagonal),
   // then the whitened, symmetric penalty K = M^-1/2 L M^-1/2
   Eigen::MatrixXd D2 = Eigen::MatrixXd::Zero( n - 2, n );
   for ( int i = 0; i < n - 2; i++ )
   {
      D2( i, i ) = 1e0; D2( i, i + 1 ) = -2e0; D2( i, i + 2 ) = 1e0;
   }
   Eigen::MatrixXd L = D2.transpose() * D2;
   Eigen::MatrixXd K = Mih.asDiagonal() * L * Mih.asDiagonal();

   Eigen::SelfAdjointEigenSolver < Eigen::MatrixXd > es( K );
   if ( es.info() != Eigen::Success )
   {
      return false;
   }
   Eigen::VectorXd g = es.eigenvalues().cwiseMax( 0e0 );     // penalty eigenvalues
   Eigen::MatrixXd U = es.eigenvectors();
   Eigen::VectorXd z = U.transpose() * ( Mh.asDiagonal() * a0 );   // whitened OLS slope

   double gmax = g.maxCoeff();
   double gmin = 0e0;
   for ( int i = 0; i < n; i++ )
   {
      if ( g[ i ] > 0e0 && ( gmin == 0e0 || g[ i ] < gmin ) )
      {
         gmin = g[ i ];
      }
   }
   if ( gmax <= 0e0 || gmin <= 0e0 )
   {
      return false;
   }

   // GCV over a log-lambda grid spanning from ~no smoothing (lambda*gmax<<1) to alpha
   // forced linear (lambda*gmin>>1); closed-form via the eigenbasis
   double lam_lo    = 1e-3 / gmax;
   double lam_hi    = 1e3  / gmin;
   const int NL     = 120;
   double best_gcv  = std::numeric_limits < double >::infinity();
   double best_lam  = lam_lo;
   double best_edof = (double) n;
   for ( int k = 0; k < NL; k++ )
   {
      double lam  = lam_lo * std::pow( lam_hi / lam_lo, (double) k / (double) ( NL - 1 ) );
      double edof = 0e0, rss = 0e0;
      for ( int i = 0; i < n; i++ )
      {
         double lg = lam * g[ i ];
         double s  = lg / ( 1e0 + lg );        // shrinkage of whitened coordinate i
         edof     += 1e0 - s;                  // 1/(1+lambda g)
         double r  = z[ i ] * s;
         rss      += r * r;
      }
      double denom = 1e0 - edof / (double) n;
      if ( denom <= 0e0 )
      {
         continue;
      }
      double gcv = ( rss / (double) n ) / ( denom * denom );
      if ( gcv < best_gcv )
      {
         best_gcv  = gcv;
         best_lam  = lam;
         best_edof = edof;
      }
   }

   Eigen::VectorXd shrunk( n );
   for ( int i = 0; i < n; i++ )
   {
      shrunk[ i ] = z[ i ] / ( 1e0 + best_lam * g[ i ] );
   }
   Eigen::VectorXd alpha = Mih.asDiagonal() * ( U * shrunk );

   Iex_out.resize( n );
   alpha_out.resize( n );
   for ( int i = 0; i < n; i++ )
   {
      alpha_out[ i ] = alpha[ i ];
      Iex_out[ i ]   = ( C[ i ] != 0e0 ) ? ( Q[ i ] - B[ i ] * alpha[ i ] ) / C[ i ] : 0e0;
   }
   lambda_out = best_lam;
   edof_out   = best_edof;
   return true;
}

// Guinier quality check of a produced curve: fit ln(I) vs q^2 over the low-q region
// (refined once so q_max*Rg <= ~1.3, the valid Guinier range), returning Rg, I(0), the
// fit R^2 and the number of points used. A high R^2 with a physically sensible Rg is a
// ground-truth-free indicator that the low-q extrapolation is clean (it was the decisive
// real-data test that showed the GCV fit outperforms per-q OLS and ATSAS almerge at low
// q). Returns false if no Guinier region can be formed.
static bool us_extrap_c0_guinier(
                                 const vector < double > & q,
                                 const vector < double > & I,
                                 double & Rg, double & I0, double & r2, int & npts_used )
{
   int n = (int) q.size();
   if ( n < 5 )
   {
      return false;
   }

   // initial low-q Guinier window chosen by q-VALUE (q <= ~0.08 A^-1), not point count: deposited
   // data can be on a very fine grid where a fixed point count spans only a tiny, noisy sliver of
   // low q, wrecking the fit (e.g. 25 points is q < 0.008 on a 2452-point grid). Fall back to a
   // point count for coarse/short grids.
   int    k = 0;
   for ( int i = 0; i < n; i++ ) { if ( q[ i ] <= 8e-2 ) { k = i + 1; } else { break; } }
   if ( k < 5 ) { k = ( n < 25 ) ? n : 25; }
   double slope     = 0e0;
   double intercept = 0e0;

   for ( int pass = 0; pass < 6; pass++ )
   {
      double sx = 0e0, sy = 0e0, sxx = 0e0, sxy = 0e0;
      int    m  = 0;
      for ( int i = 0; i < k && i < n; i++ )
      {
         if ( I[ i ] <= 0e0 || us_isnan( I[ i ] ) )
         {
            continue;
         }
         double xx = q[ i ] * q[ i ];
         double yy = std::log( I[ i ] );
         sx += xx; sy += yy; sxx += xx * xx; sxy += xx * yy; m++;
      }
      if ( m < 3 )
      {
         return false;
      }
      double den = (double) m * sxx - sx * sx;
      if ( den == 0e0 )
      {
         return false;
      }
      slope     = ( (double) m * sxy - sx * sy ) / den;
      intercept = ( sy - slope * sx ) / (double) m;

      double mean = sy / (double) m;
      double ssr = 0e0, sst = 0e0;
      for ( int i = 0; i < k && i < n; i++ )
      {
         if ( I[ i ] <= 0e0 || us_isnan( I[ i ] ) )
         {
            continue;
         }
         double xx   = q[ i ] * q[ i ];
         double yy   = std::log( I[ i ] );
         double pred = intercept + slope * xx;
         ssr += ( yy - pred ) * ( yy - pred );
         sst += ( yy - mean ) * ( yy - mean );
      }
      r2        = ( sst > 0e0 ) ? ( 1e0 - ssr / sst ) : 0e0;
      npts_used = m;
      Rg        = ( slope < 0e0 ) ? std::sqrt( -3e0 * slope ) : 0e0;
      I0        = std::exp( intercept );

      // refine the window so q_max*Rg <= ~1.3 (the valid Guinier range), iterating to convergence
      if ( Rg > 0e0 )
      {
         int knew = 0;
         for ( int i = 0; i < n; i++ )
         {
            if ( q[ i ] * Rg <= 1.3e0 )
            {
               knew = i + 1;
            }
            else
            {
               break;
            }
         }
         if ( knew < 5 )
         {
            knew = ( n < 10 ) ? n : 10;
         }
         if ( knew == k )
         {
            break;                              // window stable -> converged
         }
         k = knew;
      }
      else
      {
         break;
      }
   }
   return ( Rg > 0e0 );
}

// Robust one-outlier-curve detection for the dilution series. A bad concentration / scale /
// aggregating curve is off the concentration trend at (nearly) every q, so it is detected at
// the CURVE level (never per-q). Cheap per-q weighted OLS on the model axis (I/c for additive,
// c/I for reciprocal/virial -- 1st order even when the output model is 2nd-virial, for stability
// with a curve removed) gives standardized residuals t = (y - yhat)/sigma; per curve we take the
// mean |t| across q and the fraction of q where the residual keeps one sign (a real
// scale/concentration error is systematically one-signed and biases low q most; noise flips). The
// worst curve is nominated; the caller applies the significance + one-sided + pooled-chi2 gates and
// the count guard, and decides flag-vs-discard. Returns the nominated curve index (argmax mean|t|)
// or -1 if the set is too small; fills the diagnostics the gates and the log need. This targets
// concentration/scale outliers only -- aggregation (extra low-q intensity) stays the Guinier QC's job.
static int us_extrap_c0_detect_outlier(
                                       const QStringList                        & names,
                                       const vector < double >                  & concs,
                                       const vector < bool >                    & is_istar,
                                       map < QString, vector < double > >       & name_to_I,
                                       map < QString, vector < double > >       & name_to_err,
                                       unsigned int                               npts,
                                       bool                                       reciprocal,
                                       bool                                       use_weights,  // 1/sigma^2 fit + standardize by sigma; else OLS + per-q RMS
                                       bool                                       use_leverage, // studentize by sqrt(1-h) (unmasks high-leverage top-conc curve)
                                       double                                   & T_out,       // mean |t| of the nominee
                                       double                                   & sgn_out,     // one-sided fraction of the nominee
                                       double                                   & sep_out,     // T_nominee / median T of the rest (reported, not gated)
                                       double                                   & chi2_full_out,
                                       double                                   & chi2_red_out )
{
   T_out = 0e0; sgn_out = 0e0; sep_out = 0e0; chi2_full_out = 0e0; chi2_red_out = 0e0;
   int nc = names.size();
   if ( nc < 4 )
   {
      return -1;                                 // need >= 4 so dropping one still leaves >= 3
   }

   // per-curve accumulators over q
   vector < vector < double > > absres( nc );    // |standardized residual| per q (valid only)
   vector < int >    npos( nc, 0 ), nval( nc, 0 );
   double pooled_num_full = 0e0; double pooled_den_full = 0e0;

   for ( unsigned int qi = 0; qi < npts; qi++ )
   {
      // weighted OLS of y vs c at this q (model axis), collecting sufficient stats
      double S = 0e0, Sx = 0e0, Sy = 0e0, Sxx = 0e0, Sxy = 0e0;
      int    n_here = 0;
      vector < double > yv( nc ), wv( nc ), xv( nc );
      vector < bool >   ok( nc, false );
      for ( int ci = 0; ci < nc; ci++ )
      {
         if ( concs[ ci ] <= 0e0 ) { continue; }
         const vector < double > & Iarr = name_to_I[ names[ ci ] ];
         if ( qi >= Iarr.size() ) { continue; }
         double Iv = Iarr[ qi ];
         if ( us_isnan( Iv ) ) { continue; }
         double sd = ( name_to_err.count( names[ ci ] ) && qi < name_to_err[ names[ ci ] ].size() )
            ? name_to_err[ names[ ci ] ][ qi ] : 0e0;
         if ( us_isnan( sd ) ) { sd = 0e0; }
         double y, sig;
         if ( reciprocal )
         {
            if ( Iv <= 0e0 ) { continue; }
            if ( is_istar[ ci ] ) { y = 1e0 / Iv;           sig = sd / ( Iv * Iv ); }
            else                  { y = concs[ ci ] / Iv;   sig = sd * concs[ ci ] / ( Iv * Iv ); }
         }
         else if ( is_istar[ ci ] ) { y = Iv;               sig = sd; }
         else                       { y = Iv / concs[ ci ];  sig = sd / concs[ ci ]; }
         double w = ( use_weights && sig > 0e0 && !us_isnan( sig ) ) ? 1e0 / ( sig * sig ) : 1e0;
         yv[ ci ] = y; wv[ ci ] = w; xv[ ci ] = concs[ ci ]; ok[ ci ] = true;
         S += w; Sx += w * xv[ ci ]; Sy += w * y; Sxx += w * xv[ ci ] * xv[ ci ]; Sxy += w * xv[ ci ] * y;
         n_here++;
      }
      double M = ( S > 0e0 ) ? ( Sxx - Sx * Sx / S ) : 0e0;
      if ( n_here < 3 || M <= 0e0 ) { continue; } // no usable trend at this q
      double slope = ( Sxy - Sx * Sy / S ) / M;
      double icept = ( Sy - Sx * slope ) / S;
      // standardize residuals by sigma (weighted mode) or by the per-q residual RMS (unweighted
      // mode) -- raw residuals span orders of magnitude across q, so a per-q scale is required
      double s_q = 0e0;
      if ( !use_weights )
      {
         double ss = 0e0;
         for ( int ci = 0; ci < nc; ci++ )
         {
            if ( !ok[ ci ] ) { continue; }
            double r = yv[ ci ] - ( icept + slope * xv[ ci ] );
            ss += r * r;
         }
         s_q = ( n_here > 2 ) ? sqrt( ss / ( n_here - 2 ) ) : 0e0;
      }
      double xbar_w = ( S > 0e0 ) ? Sx / S : 0e0;
      for ( int ci = 0; ci < nc; ci++ )
      {
         if ( !ok[ ci ] ) { continue; }
         double r  = yv[ ci ] - ( icept + slope * xv[ ci ] );
         double t;
         if ( use_weights )
         {
            double sg = ( wv[ ci ] > 0e0 ) ? 1e0 / sqrt( wv[ ci ] ) : 0e0;
            t = ( sg > 0e0 ) ? r / sg : 0e0;
         }
         else
         {
            t = ( s_q > 0e0 ) ? r / s_q : 0e0;
         }
         if ( use_leverage )
         {
            // Studentize: divide by sqrt(1-h). h is the point's leverage in the weighted line fit,
            //   h_i = w_i * ( 1/S + (x_i - xbar_w)^2 / M ) ,  sum_i h_i = 2 (a 2-parameter fit).
            // Without this the highest-CONCENTRATION curve is systematically masked: it is the
            // extreme x, so it has the largest leverage, drags the fitted line onto itself and
            // shrinks its own residual. Measured on the 7-curve alpha-syn set the top curve carries
            // mean leverage 0.80 (vs ~0.29 average), which pushed a genuinely suspect curve from
            // rank 2 down to rank 5 on the raw statistic. Var(r_i) = sigma_i^2 (1-h_i), so this is
            // the standard internally-studentized residual, not a heuristic.
            double h = ( S > 0e0 && M > 0e0 )
               ? wv[ ci ] * ( 1e0 / S + ( xv[ ci ] - xbar_w ) * ( xv[ ci ] - xbar_w ) / M )
               : 0e0;
            double denom = sqrt( ( h < 1e0 ) ? ( 1e0 - h ) : 1e-9 );   // h->1 = point barely constrained
            if ( denom < 1e-4 ) { denom = 1e-4; }                      // cap the inflation
            t /= denom;
         }
         absres[ ci ].push_back( fabs( t ) );
         nval[ ci ]++;
         if ( r >= 0e0 ) { npos[ ci ]++; }
         pooled_num_full += wv[ ci ] * r * r;
      }
      pooled_den_full += ( n_here - 2 );         // per-q dof of a 2-parameter line
   }

   // per-curve aggregate: mean |t| across q. (Mean, not median: a bad concentration/scale error
   // bites hardest at low q -- exactly where it corrupts I(0)/MW -- so its |t| is concentrated in
   // a minority of points; the median washes that out, the mean keeps it while the one-sided and
   // pooled-chi2 gates below still guard against a merely noisy curve.)
   vector < double > Tj( nc, 0e0 );
   for ( int ci = 0; ci < nc; ci++ )
   {
      if ( nval[ ci ] < 1 ) { Tj[ ci ] = 0e0; continue; }
      double s = 0e0;
      for ( int k = 0; k < (int) absres[ ci ].size(); k++ ) { s += absres[ ci ][ k ]; }
      Tj[ ci ] = s / (double) absres[ ci ].size();
   }
   int j_star = -1; double T_best = -1e0;
   for ( int ci = 0; ci < nc; ci++ )
   {
      if ( Tj[ ci ] > T_best ) { T_best = Tj[ ci ]; j_star = ci; }
   }
   if ( j_star < 0 ) { return -1; }

   // separation: nominee vs the median of the OTHER curves' Tj
   vector < double > rest;
   for ( int ci = 0; ci < nc; ci++ ) { if ( ci != j_star ) { rest.push_back( Tj[ ci ] ); } }
   std::sort( rest.begin(), rest.end() );
   double T_rest_med = rest.size() ? rest[ rest.size() / 2 ] : 0e0;

   // pooled reduced chi^2 with the nominee removed (cheap: subtract its residual contribution)
   double pooled_num_red = 0e0, pooled_den_red = 0e0;
   {
      // recompute quickly without j_star (its removal also changes each q's fit, but for the
      // gate a refit-free approximation that drops its residuals is adequate and conservative)
      pooled_num_red = pooled_num_full;
      // subtract j_star's weighted squared residuals
      double sub = 0e0; int nsub = 0;
      // (absres holds |t| = |r|/sg, and t^2 = w r^2, so w r^2 = t^2)
      for ( int k = 0; k < (int) absres[ j_star ].size(); k++ )
      {
         sub += absres[ j_star ][ k ] * absres[ j_star ][ k ]; nsub++;
      }
      pooled_num_red -= sub;
      pooled_den_red  = pooled_den_full - nsub;  // each dropped point removes ~1 dof
   }

   double chi2_full = ( pooled_den_full > 0e0 ) ? pooled_num_full / pooled_den_full : 0e0;
   double chi2_red  = ( pooled_den_red  > 0e0 ) ? pooled_num_red  / pooled_den_red  : 0e0;

   T_out         = Tj[ j_star ];
   sgn_out       = ( nval[ j_star ] > 0 )
      ? qMax( (double) npos[ j_star ], (double) ( nval[ j_star ] - npos[ j_star ] ) ) / (double) nval[ j_star ]
      : 0e0;
   sep_out       = ( T_rest_med > 0e0 ) ? T_out / T_rest_med : ( T_out > 0e0 ? 1e30 : 0e0 );
   chi2_full_out = chi2_full;
   chi2_red_out  = chi2_red;
   return j_star;
}

void US_Hydrodyn_Saxs::do_extrap_c0(
                                    QStringList qsl_sel_names,
                                    QStringList qsl_data,
                                    map < QString, QString > name_to_errors_map,
                                    vector < double > q,
                                    QString filename
                                    )
{
   // In a scripted (headless) run the modal warning/critical popups below would block with no
   // one to dismiss them; route them to the editor log instead. Interactively, behaviour is
   // unchanged (a modal warning box). The abort messages are still followed by their return.
   auto extrap_msg = [this]( const QString & msg ) {
      if ( extrap_c0_script ) { editor_msg( "red", msg + "\n" ); }
      else { QMessageBox::warning( this, "UltraScan", msg ); }
   };

   // 1. parse the selected curves' I(q) rows (mirrors the parsing in load_iqq_csv())

   map < QString, vector < double > > name_to_I;
   QStringList ordered_names;

   for ( QStringList::iterator it = qsl_sel_names.begin();
         it != qsl_sel_names.end();
         it++ )
   {
      QString name = *it;
      bool    found = false;

      for ( QStringList::iterator it2 = qsl_data.begin();
            it2 != qsl_data.end();
            it2++ )
      {
         QStringList qsl_tmp = (*it2).split( "," );
         if ( qsl_tmp.size() && qsl_tmp[ 0 ] == name )
         {
            found = true;
            if ( qsl_tmp.size() < 3 )
            {
               editor_msg( "red",
                          us_tr( "Skipping curve with insufficient I(q) values: " ) + name + "\n" );
               break;
            }

            vector < double > I;
            QStringList::iterator it3 = qsl_tmp.begin();
            it3 += 2;
            for ( ; it3 != qsl_tmp.end(); it3++ )
            {
               I.push_back( (*it3).toDouble() );
            }
            I.pop_back();

            name_to_I[ name ] = I;
            ordered_names << name;
            break;
         }
      }

      if ( !found )
      {
         editor_msg( "red", us_tr( "Could not find data for selected curve: " ) + name + "\n" );
      }
   }

   if ( ordered_names.size() < 3 )
   {
      extrap_msg(
                            us_tr( "Fewer than 3 curves with usable data are available; "
                                  "cannot extrapolate to zero concentration." ) );
      return;
   }

   // 2. common q-grid length: all rows share q's header row, just truncate ragged rows

   unsigned int npts = (unsigned int) q.size();
   for ( QStringList::iterator it = ordered_names.begin(); it != ordered_names.end(); it++ )
   {
      if ( name_to_I[ *it ].size() < npts )
      {
         npts = (unsigned int) name_to_I[ *it ].size();
      }
   }
   for ( QStringList::iterator it = ordered_names.begin(); it != ordered_names.end(); it++ )
   {
      name_to_I[ *it ].resize( npts );
   }

   // 2b. parse each curve's I(q) sd row (same format/keying as name_to_errors_map),
   //     used for the per-point error bars in the optional regression-plot pop-up.
   //     Absent errors are left as an empty vector (no error bar drawn for that curve).
   map < QString, vector < double > > name_to_err;
   for ( QStringList::iterator it = ordered_names.begin(); it != ordered_names.end(); it++ )
   {
      if ( !name_to_errors_map.count( *it ) )
      {
         continue;
      }
      QStringList qsl_err = name_to_errors_map[ *it ].split( "," );
      if ( qsl_err.size() < 3 )
      {
         continue;
      }
      vector < double > err;
      for ( int k = 2; k < qsl_err.size(); k++ )
      {
         err.push_back( qsl_err[ k ].toDouble() );
      }
      err.pop_back();
      err.resize( npts );
      name_to_err[ *it ] = err;
   }

   // 3. launch the concentration-assignment dialog

   // conc_csv is keyed inconsistently depending on how a curve arrived:
   // curves loaded straight from this CSV file and previously plotted get keyed
   // by "<csv filename> <quoted row name>" (e.g. plot_one_iqq() callers elsewhere
   // in load_iqq_csv() use QFileInfo(filename).fileName() + " " + qsl_tmp[0]);
   // curves pushed in from elsewhere (e.g. SAXS Hplc's to_saxs()) are keyed by
   // the bare, unquoted plotted curve name. Try all three forms.
   map < QString, double > prepop_conc;
   // Also remember each curve's partial specific volume (psv) and standard I(0) (i0se). These are
   // read from the PSV:/I0se: header tokens into conc_csv on load; the extrapolated curve must
   // carry them (and the inputs must keep them) so a later Guinier sees the real values rather than
   // the global defaults. get_conc_csv_values() leaves the global default in psv/I0_std when a
   // curve has no entry, so these maps always hold a usable value for every name.
   map < QString, double > name_to_psv;
   map < QString, double > name_to_i0se;
   for ( QStringList::iterator it = ordered_names.begin(); it != ordered_names.end(); it++ )
   {
      QString name = *it;
      QString dequoted_name = name;
      dequoted_name.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );

      double  conc, psv, I0_std;

      QString composed_name = QFileInfo( filename ).fileName() + " " + name;
      if ( get_conc_csv_values( composed_name, conc, psv, I0_std ) ||
           get_conc_csv_values( name, conc, psv, I0_std ) ||
           get_conc_csv_values( dequoted_name, conc, psv, I0_std ) )
      {
         prepop_conc[ name ] = conc;
      }
      name_to_psv [ name ] = psv;
      name_to_i0se[ name ] = I0_std;
   }

   map < QString, double > name_to_conc;
   QStringList selected_names;
   bool dlg_ok        = false;
   bool ref_scale     = false;  // output on the reference (max-conc) absolute scale (else Conc:1)
   bool merge_ref     = false;  // splice the reference curve above the almerge switchover
   bool show_regplots = false;
   int  fit_broaden   = 0;
   bool use_gcv       = true;   // automatic GCV slope regularization (recommended default)
   bool use_sd_weights = true;  // weight the concentration regressions by 1/sigma^2 (recommended default)
   int  extrap_model  = 1;      // concentration model: 0 additive, 1 reciprocal (default), 2 2nd-virial
   bool recompute_inputs      = false; // transiently reassess each input curve's SDs before fitting
   int  recompute_inputs_mode = 0;     // 0 constant, 1 non-constant, 2 intensity-dependent
   int  sd_mode       = 0;      // post-fit SD reassessment: 0 off, 1 constant, 2 non-constant, 3 intensity
   bool   discard_outlier     = false; // auto-discard outlier concentration curve(s) (robust QC)
   // Maximum number of curves the QC may discard. 1 = the original single-discard behavior (and the
   // GUI default). Raising it re-runs the detector on the cleaned set: with TWO contaminated curves
   // present, a single pass judges every curve against a trend that still contains the other bad
   // one, so the nomination itself can be unreliable.
   int    outlier_max         = 1;
   // Studentize the nomination residual by sqrt(1-h). ON by default: the raw statistic
   // systematically masks the highest-concentration curve, which is the highest-leverage point.
   bool   outlier_leverage    = true;
   // Optional explicit reference-curve choice (name substring, or a concentration value).
   // Empty = the default highest-concentration pick.
   QString ref_override;
   // Manual splice switchover: if > 0 and merge_ref is on, force the merge point to the nearest
   // grid q instead of auto-locating it. 0 = automatic (default).
   double  merge_q_override = 0e0;
   double outlier_sigma       = 3e0;   // detection threshold: median standardized residual across q
   double outlier_chi2_ratio  = 1.5e0; // required pooled reduced-chi^2 improvement to confirm a drop
   if ( extrap_c0_script )
   {
      // scripted run: skip the modal concentration/options dialog and take every choice from
      // the members set by saxs_extrap_c0_script(). Concentrations come from each curve's
      // Conc: header (prepop_conc), optionally overridden by "conc" directives (dequoted key).
      selected_names = ordered_names;
      for ( QStringList::iterator it = ordered_names.begin(); it != ordered_names.end(); it++ )
      {
         QString name = *it;
         QString dequoted_name = name;
         dequoted_name.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );
         if ( extrap_c0_script_conc.count( dequoted_name ) )
         {
            name_to_conc[ name ] = extrap_c0_script_conc[ dequoted_name ];
         }
         else if ( prepop_conc.count( name ) )
         {
            name_to_conc[ name ] = prepop_conc[ name ];
         }
         else
         {
            editor_msg( "red",
                       QString( us_tr( "saxs_extrap_c0: no concentration for curve %1 "
                                       "(no Conc: header and no conc override); using 0\n" ) ).arg( name ) );
            name_to_conc[ name ] = 0e0;
         }
      }
      dlg_ok             = true;
      ref_scale          = extrap_c0_script_ref_scale;
      merge_ref          = extrap_c0_script_merge_ref;
      show_regplots      = false;
      fit_broaden        = extrap_c0_script_fit_broaden;
      use_gcv            = extrap_c0_script_gcv;
      use_sd_weights     = extrap_c0_script_sd_weights;
      extrap_model       = extrap_c0_script_model;
      recompute_inputs      = extrap_c0_script_recompute_in;
      recompute_inputs_mode = extrap_c0_script_recompute_mode;
      sd_mode            = extrap_c0_script_sd_mode;
      discard_outlier    = extrap_c0_script_discard;
      outlier_max        = extrap_c0_script_outlier_max;
      outlier_leverage   = extrap_c0_script_outlier_leverage;
      ref_override       = extrap_c0_script_reference;
      merge_q_override   = extrap_c0_script_merge_q;
      outlier_sigma      = extrap_c0_script_outlier_sigma;
      outlier_chi2_ratio = extrap_c0_script_outlier_chi2;
   }
   else
   {
      US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc dlg( ordered_names, prepop_conc, &name_to_conc, &selected_names, &dlg_ok, &ref_scale, &merge_ref, &show_regplots, &fit_broaden, &use_gcv, &use_sd_weights, &extrap_model, &recompute_inputs, &recompute_inputs_mode, &sd_mode, &discard_outlier, &outlier_sigma, &outlier_chi2_ratio, &ref_override, &merge_q_override, us_hydrodyn, this );
      US_Hydrodyn::fixWinButtons( &dlg );
      dlg.exec();
   }
   // The reference (max-concentration) curve is needed for either the absolute output scale
   // or the high-q reference splice; the fit itself (below) is always the normalized
   // concentration-model fit, applied identically to both output conventions.
   bool use_ref_path = ref_scale || merge_ref;

   if ( !dlg_ok )
   {
      this->isVisible() ? this->raise() : this->show();
      editor_msg( "black", us_tr( "Extrapolation to zero concentration cancelled\n" ) );
      return;
   }

   // the dialog's selected (highlighted) rows are the curves to extrapolate; narrow
   // the working set to those (the dialog already enforces >= 3 selected, but guard)
   ordered_names = selected_names;
   if ( ordered_names.size() < 3 )
   {
      extrap_msg(
                            us_tr( "Fewer than 3 curves were selected; "
                                  "cannot extrapolate to zero concentration." ) );
      return;
   }

   // 3b. psv / i0se consistency: partial specific volume (psv) and standard I(0) (i0se) are
   //     molecular properties, so every curve in a concentration series must carry the same values.
   //     A curve with no explicit PSV:/I0se: header uses the global default -- treat that as "unset"
   //     (it constrains nothing, so it is always compatible). Two curves with *different explicit*
   //     values are a fatal input error (extrapolating a mix of psv/i0se is meaningless): stop.
   //     The agreed value, if any, is carried onto the inputs (step 4) and the extrapolated curve
   //     (conc_csv + output header) so a later Guinier reproduces it rather than the defaults.
   double common_psv  = our_saxs_options->psv;
   double common_i0se = our_saxs_options->I0_exp;
   bool   have_psv    = false;   // an explicit (non-default) psv was found and agreed on
   bool   have_i0se   = false;
   {
      const double def_psv  = our_saxs_options->psv;
      const double def_i0se = our_saxs_options->I0_exp;
      auto differs = []( double a, double b )
      {
         return qAbs( a - b ) > 1e-6 + 1e-4 * qMax( qAbs( a ), qAbs( b ) );
      };
      QString conflicts;
      for ( QStringList::iterator it = ordered_names.begin(); it != ordered_names.end(); it++ )
      {
         double  p  = name_to_psv .count( *it ) ? name_to_psv [ *it ] : def_psv;
         double  e  = name_to_i0se.count( *it ) ? name_to_i0se[ *it ] : def_i0se;
         QString dq = *it;
         dq.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );
         if ( differs( p, def_psv ) )       // explicitly set (differs from the global default)
         {
            if ( !have_psv ) { common_psv = p; have_psv = true; }
            else if ( differs( p, common_psv ) )
            {
               conflicts += QString( us_tr( "\n  %1: psv %2 (expected %3)" ) ).arg( dq ).arg( p, 0, 'g', 6 ).arg( common_psv, 0, 'g', 6 );
            }
         }
         if ( differs( e, def_i0se ) )
         {
            if ( !have_i0se ) { common_i0se = e; have_i0se = true; }
            else if ( differs( e, common_i0se ) )
            {
               conflicts += QString( us_tr( "\n  %1: i0se %2 (expected %3)" ) ).arg( dq ).arg( e, 0, 'g', 6 ).arg( common_i0se, 0, 'g', 6 );
            }
         }
      }
      if ( !conflicts.isEmpty() )
      {
         extrap_msg(
            us_tr( "The selected curves carry conflicting partial specific volume (PSV) or "
                   "standard I(0) (i0se) values. These are molecular properties and must be "
                   "identical across a concentration series, so extrapolation is aborted. "
                   "Correct the PSV:/I0se: headers so they agree (a curve without the header "
                   "uses the global default and is treated as compatible):" ) + conflicts );
         return;
      }
   }

   // 4. persist entered concentrations back into the existing conc_csv facility, keyed by the
   //    dequoted (canonical, unquoted) curve name -- matches the convention used by other writers
   //    such as US_Hydrodyn_Saxs_Hplc::to_saxs(). Carry each curve's psv/i0se too, so re-plotting
   //    does not reset them to the global defaults.

   for ( QStringList::iterator it = ordered_names.begin(); it != ordered_names.end(); it++ )
   {
      QString dequoted_name = *it;
      dequoted_name.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );
      update_conc_csv( dequoted_name, name_to_conc[ *it ],
                       name_to_psv .count( *it ) ? name_to_psv [ *it ] : our_saxs_options->psv,
                       name_to_i0se.count( *it ) ? name_to_i0se[ *it ] : our_saxs_options->I0_exp );
   }

   // 5. distinct-concentration sanity check (warn, don't block)

   vector < double > concs;
   for ( QStringList::iterator it = ordered_names.begin(); it != ordered_names.end(); it++ )
   {
      concs.push_back( name_to_conc[ *it ] );
   }

   set < double > distinct_concs( concs.begin(), concs.end() );
   if ( distinct_concs.size() < 3 )
   {
      extrap_msg(
                            us_tr( "Fewer than 3 distinct concentration values were entered.\n"
                                  "The zero-concentration extrapolation may not be meaningful.\n"
                                  "Proceeding anyway." ) );
   }

   // Curves whose name contains "_Istarq_" are already in I*(q) form (header units
   // g/mol, I(0)=MW): their intensity has ALREADY been normalized by concentration
   // even though their Conc: header is non-unity. Such a curve must NOT be divided by
   // its concentration again in the Zimm fit -- its intensity is used as-is, while its
   // real concentration still serves as the regression x-axis. A wholly-I*(q) input
   // therefore also yields an already-normalized (I(0)=MW) output. absolute-scale mode never
   // divides by c, so its intensity handling is unaffected (only its output conc tag
   // differs, below).
   vector < bool > is_istar( ordered_names.size(), false );
   unsigned int    istar_count = 0;
   for ( int ci = 0; ci < ordered_names.size(); ci++ )
   {
      if ( ordered_names[ ci ].contains( "_Istarq_" ) )
      {
         is_istar[ ci ] = true;
         istar_count++;
      }
   }
   bool all_istar = ( istar_count == (unsigned int) ordered_names.size() );
   if ( istar_count && !all_istar )
   {
      extrap_msg(
                            QString( us_tr( "The selection mixes %1 I*(q) (already concentration-normalized, "
                                            "I(0)=MW) curve(s) with %2 raw-intensity curve(s). These are on "
                                            "different scales; the extrapolation may not be meaningful. "
                                            "Proceeding anyway." ) )
                            .arg( istar_count ).arg( ordered_names.size() - (int) istar_count ) );
   }

   if ( !use_sd_weights )
   {
      editor_msg( "black",
                 QString( us_tr( "Unweighted (OLS) regression: the curve errors are not used in the fit; the "
                                 "intercept error bar is the fit's residual scatter.\n" ) ) );
   }

   // Two extrapolation modes are offered (selected via the concentration dialog):
   //
   // Zimm mode (default): loaded I(q) curves are raw (not pre-normalized by
   // concentration); raw SAXS intensity is dominated by a term directly proportional
   // to c, so a linear fit of raw I(q) vs c trivially extrapolates to ~0 at c=0
   // (no protein, no excess scattering) -- not physically useful. The standard
   // "extrapolation to zero concentration" technique (the SAXS analogue of a Zimm
   // plot) instead fits the concentration-NORMALIZED intensity I(q,c)/c against c;
   // its c=0 intercept is the ideal, structure-factor-free dilute-limit curve. So
   // we divide by concentration on the intensity axis only -- the concentration (x)
   // axis stays the real, distinct entered values. The output is I(q)/c, tagged
   // Conc:1 (SOMO's "already normalized" convention).
   //
   // absolute-scale mode: reproduces ATSAS almerge (Petoukhov et al. 2012 Eq 1; Franke et al.
   // 2017). Each curve is least-squares scaled onto the highest-concentration curve
   // (the reference), then at each q the scaled intensity is fitted linearly against
   // concentration; the intercept is the infinite-dilution intensity I_ex. This
   // extrapolation is applied only in the low-q "overlap" region where the curves
   // show a real concentration dependence; above the merging point -- located where
   // the extrapolation and the reference differ only by noise (a CORMAP longest-run
   // p-value test, US_Saxs_Util::compute_p_value) -- the reference curve is taken
   // verbatim. This removes the interparticle/structure-factor effect at low q while
   // keeping stable, positive intensities at high q (a per-point fit there would just
   // track noise and go negative). The result is on the reference curve's absolute
   // intensity scale, carries the reference curve's error bars, and is tagged with
   // the reference concentration rather than Conc:1.
   unsigned int zero_conc_excluded = 0;
   for ( int ci = 0; ci < (int) concs.size(); ci++ )
   {
      if ( concs[ ci ] <= 0e0 )
      {
         zero_conc_excluded++;
      }
   }
   if ( zero_conc_excluded )
   {
      extrap_msg(
                            QString( us_tr( "%1 curve(s) have a concentration of 0 and will be excluded "
                                            "from the extrapolation (intensity can't be normalized by a zero concentration)." ) )
                            .arg( zero_conc_excluded ) );
   }

   // 5a. optional transient input-SD recompute: reassess each selected curve's error bars from
   //     its own point-to-point scatter (the BayesApp engine, same as the output SD reassessment)
   //     and use those errors for THIS extrapolation only -- name_to_err is a local copy, the
   //     loaded curves are untouched. Runs before the outlier QC and the fit, so the reassessed
   //     errors feed the outlier detector, the 1/sigma^2 weights and the reference merge test.
   if ( recompute_inputs )
   {
      char ri_mode = ( recompute_inputs_mode == 2 ) ? 'I' : ( recompute_inputs_mode == 1 ) ? 'N' : 'C';
      QString ri_label = ( recompute_inputs_mode == 2 ) ? us_tr( "intensity-dependent" )
                       : ( recompute_inputs_mode == 1 ) ? us_tr( "non-constant" )
                                                        : us_tr( "constant" );
      vector < double > qv( q.begin(), q.begin() + ( ( npts <= (unsigned int) q.size() ) ? npts : q.size() ) );
      editor_msg( "black",
                 QString( us_tr( "Recomputing input curve SDs (%1) for this extrapolation only:\n" ) ).arg( ri_label ) );
      int ri_done = 0, ri_skipped = 0;
      for ( int ci = 0; ci < ordered_names.size(); ci++ )
      {
         const QString & nm = ordered_names[ ci ];
         if ( !name_to_err.count( nm ) || name_to_err[ nm ].size() != name_to_I[ nm ].size()
              || !US_Saxs_Util::is_nonzero_vector( name_to_err[ nm ] ) )
         {
            editor_msg( "dark red", QString( us_tr( "  %1: skipped (no usable SDs)\n" ) ).arg( nm ) );
            ri_skipped++;
            continue;
         }
         vector < double > sd = name_to_err[ nm ];
         vector < double > factors;
         QString ri_errors, ri_verdict;
         double  ri_pval = 0e0, ri_chi2r = 0e0;
         if ( !US_Saxs_Util::recompute_errors( name_to_I[ nm ], qv, sd, ri_errors, ri_mode,
                                               10, 11, false, &factors, 0, &ri_verdict, &ri_pval, &ri_chi2r ) )
         {
            editor_msg( "dark red", QString( us_tr( "  %1: skipped (%2)\n" ) ).arg( nm ).arg( ri_errors ) );
            ri_skipped++;
            continue;
         }
         // factor summary matching the s.d.util "SD rescale" log
         double fmin = 1e0, fmed = 1e0, fmax = 1e0;
         {
            vector < double > fs = factors;
            std::sort( fs.begin(), fs.end() );
            if ( fs.size() ) { fmin = fs.front(); fmax = fs.back(); fmed = fs[ fs.size() / 2 ]; }
         }
         QString factor_str = ( fmax - fmin < 1e-9 )
            ? QString( us_tr( "factor %1" ) ).arg( fmed, 0, 'g', 4 )
            : QString( us_tr( "factor median %1 (%2..%3)" ) ).arg( fmed, 0, 'g', 4 ).arg( fmin, 0, 'g', 4 ).arg( fmax, 0, 'g', 4 );
         name_to_err[ nm ] = sd;
         ri_done++;
         editor_msg( "black",
                    QString( us_tr( "  %1: %2, %3 (reduced chi^2 %4, p %5)\n" ) )
                    .arg( nm ).arg( ri_verdict ).arg( factor_str )
                    .arg( ri_chi2r, 0, 'f', 3 ).arg( ri_pval, 0, 'f', 3 ) );
      }
      editor_msg( "black",
                 QString( us_tr( "Input SD recompute: %1 curve(s) reassessed, %2 skipped (loaded curves unchanged).\n" ) )
                 .arg( ri_done ).arg( ri_skipped ) );
   }

   // 5b. robust one-outlier-curve QC. Detect (always) a single concentration curve that is off
   //     the concentration trend at essentially every q -- a bad concentration/scale or an
   //     aggregating curve (the alpha-syn 1.818 case) -- and, if the user enabled auto-discard,
   //     remove it from the whole fit before the reference is picked (so a bad curve can't become
   //     the reference either). If detected but auto-discard is off (or too few curves remain to
   //     drop safely), just flag it. At most one curve is ever removed. Captured for the regplot
   //     as a red "x" on the excluded point. Targets concentration/scale outliers; aggregation is
   //     the Guinier QC's job.
   QString           excl_name;                 // name of the QC-excluded curve ("" => none)
   double            excl_conc = -1e0;           // its concentration (regplot x); < 0 => none
   vector < double > reg_excl_y;                 // its per-q y on the plot axis (NaN where none)
   // ---- Per-curve forward-scatter diagnostic (REPORTED, never acted on) ----------------------
   // I(0)/c is constant in the dilute limit, so its trend against concentration is the most direct
   // view of whether one curve is out of family: a mis-set concentration or aggregation shows up
   // here even when nothing in the concentration fit does. It is deliberately not turned into an
   // automatic decision. The highest-concentration curve cannot be judged from the fit itself --
   // it is the extreme x, carries nearly all the leverage, and so has a near-zero residual whatever
   // its quality -- and internal shape/noise statistics do not separate a defective top curve
   // either: they track signal-to-noise, which rises with concentration, so the curve most in need
   // of scrutiny looks best of all. Judging it needs knowledge from outside the regression (e.g.
   // the known molecular weight). This table puts the evidence in front of whoever holds that
   // knowledge; the reference-curve selector is how they act on it.
   {
      vector < QString > d_name;
      vector < double >  d_conc, d_rg, d_i0, d_i0c;
      for ( int ci = 0; ci < ordered_names.size(); ci++ )
      {
         if ( concs[ ci ] <= 0e0 ) { continue; }
         const vector < double > & Iarr = name_to_I[ ordered_names[ ci ] ];
         double g_rg = 0e0, g_i0 = 0e0, g_r2 = 0e0; int g_n = 0;
         if ( !us_extrap_c0_guinier( q, Iarr, g_rg, g_i0, g_r2, g_n ) ) { continue; }
         if ( !( g_i0 > 0e0 ) ) { continue; }
         QString nm = ordered_names[ ci ];
         nm.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );
         d_name.push_back( nm );
         d_conc.push_back( concs[ ci ] );
         d_rg  .push_back( g_rg );
         d_i0  .push_back( g_i0 );
         d_i0c .push_back( g_i0 / concs[ ci ] );
      }
      if ( d_i0c.size() >= 3 )
      {
         // compare against the MEDIAN of the series, not a fit: a median has no leverage, so the
         // extreme-concentration curve is judged on the same footing as every other one.
         vector < double > srt = d_i0c;
         std::sort( srt.begin(), srt.end() );
         double med = srt[ srt.size() / 2 ];
         editor_msg( "black",
                    us_tr( "Per-curve forward scatter (I(0)/c is constant in the dilute limit; a curve well "
                           "off the others is suspect). Reported for inspection only -- nothing is discarded "
                           "on this basis:\n" ) );
         for ( unsigned int k = 0; k < d_name.size(); k++ )
         {
            double  rel    = ( med > 0e0 ) ? 100e0 * ( d_i0c[ k ] / med - 1e0 ) : 0e0;
            QString relstr = ( rel >= 0e0 ? QString( "+" ) : QString() ) + QString::number( rel, 'f', 1 ) + "%";

            // Leave-one-out trend check. I(0)/c is not flat across a real series -- it falls as the
            // interparticle term grows -- so a curve can sit near the median and still be off the
            // trend. Fit I(0)/c against c on the OTHER curves only and predict this one. Because the
            // curve is excluded from the fit it cannot pull the line onto itself, so this is
            // leverage-free even for the extreme concentration, where an ordinary residual is
            // structurally uninformative. This is the automated form of plotting MW against
            // concentration and eyeing which point is out of line.
            QString devstr = us_tr( "n/a" );
            QString colour = ( qAbs( rel ) >= 15e0 ) ? "dark red" : "black";
            if ( d_i0c.size() >= 4 )
            {
               double S = 0e0, Sx = 0e0, Sy = 0e0, Sxx = 0e0, Sxy = 0e0;
               for ( unsigned int j = 0; j < d_i0c.size(); j++ )
               {
                  if ( j == k ) { continue; }
                  S += 1e0; Sx += d_conc[ j ]; Sy += d_i0c[ j ];
                  Sxx += d_conc[ j ] * d_conc[ j ]; Sxy += d_conc[ j ] * d_i0c[ j ];
               }
               double M = Sxx - Sx * Sx / S;
               if ( qAbs( M ) > 0e0 )
               {
                  double b    = ( Sxy - Sx * Sy / S ) / M;
                  double a    = ( Sy - Sx * b ) / S;
                  double pred = a + b * d_conc[ k ];
                  if ( pred > 0e0 )
                  {
                     double dev = 100e0 * ( d_i0c[ k ] / pred - 1e0 );
                     devstr = ( dev >= 0e0 ? QString( "+" ) : QString() ) + QString::number( dev, 'f', 1 ) + "%";
                     if ( qAbs( dev ) >= 15e0 ) { colour = "dark red"; }
                  }
               }
            }
            editor_msg( colour,
                       QString( us_tr( "   conc %1: Rg %2 A, I(0) %3, I(0)/c %4 (%5 vs median, %6 vs trend of the others)\n" ) )
                       .arg( d_conc[ k ], 0, 'g', 6 ).arg( d_rg[ k ], 0, 'f', 1 )
                       .arg( d_i0[ k ], 0, 'g', 4 ).arg( d_i0c[ k ], 0, 'g', 4 ).arg( relstr ).arg( devstr ) );
         }
      }
   }

   int               n_outlier_dropped = 0;
   // Detection is repeated on the CLEANED set while curves keep qualifying, up to outlier_max
   // (default 1 = the original behavior). Iterating matters when more than one curve is bad: a
   // single pass compares every curve against a trend that still contains the other offender, so
   // the nominee can be the wrong curve. Each pass re-runs the full detector and all gates.
   while ( true )
   {
      int n_participating = 0;
      for ( int ci = 0; ci < (int) concs.size(); ci++ ) { if ( concs[ ci ] > 0e0 ) { n_participating++; } }

      double oT = 0e0, osgn = 0e0, osep = 0e0, oc2f = 0e0, oc2r = 0e0;
      int    j_star = us_extrap_c0_detect_outlier( ordered_names, concs, is_istar,
                                                   name_to_I, name_to_err, npts,
                                                   ( extrap_model >= 1 ), use_sd_weights, outlier_leverage,
                                                   oT, osgn, osep, oc2f, oc2r );

      // gates: magnitude (mean |t| >= sigma), systematic (>=70% one-sided), and the curve
      // must matter globally (pooled reduced chi^2 improves by >= the chi^2-gain factor when it
      // is removed). Separation is reported but no longer a hard gate -- it needlessly blocked
      // genuine low-q outliers whose per-q spread overlaps the others (see the alpha-syn 1.818 set).
      double gain      = ( oc2r > 0e0 ) ? oc2f / oc2r : 0e0;
      bool   g_sigma   = ( j_star >= 0 ) && ( oT   >= outlier_sigma );
      bool   g_sign    = ( j_star >= 0 ) && ( osgn >= 0.7e0 );
      bool   g_gain    = ( j_star >= 0 ) && ( oc2r > 0e0 ) && ( gain >= outlier_chi2_ratio );
      bool   qualifies = g_sigma && g_sign && g_gain;

      int  min_remaining = ( extrap_model == 2 ) ? 4 : 3;
      bool count_ok      = false;
      if ( j_star >= 0 )
      {
         set < double > rem;
         for ( int ci = 0; ci < (int) concs.size(); ci++ )
         {
            if ( ci != j_star && concs[ ci ] > 0e0 ) { rem.insert( concs[ ci ] ); }
         }
         count_ok = ( n_participating - 1 >= min_remaining ) && ( (int) rem.size() >= 3 );
      }

      QString nominee = ( j_star >= 0 ) ? ordered_names[ j_star ] : QString();
      nominee.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );

      // full evaluation line, so a no-discard run is not silent and the thresholds can be tuned
      QString detail = ( j_star >= 0 )
         ? QString( us_tr( "most-outlying \"%1\" (conc %2): mean standardized residual %3 (threshold %4), "
                           "%5% one-sided (>=70%), pooled reduced chi^2 %6 -> %7 (x%8, threshold %9), separation %10x" ) )
             .arg( nominee ).arg( ( j_star >= 0 ) ? concs[ j_star ] : 0e0 )
             .arg( oT, 0, 'f', 2 ).arg( outlier_sigma, 0, 'f', 2 )
             .arg( 100e0 * osgn, 0, 'f', 0 )
             .arg( oc2f, 0, 'g', 3 ).arg( oc2r, 0, 'g', 3 ).arg( gain, 0, 'f', 2 ).arg( outlier_chi2_ratio, 0, 'f', 2 )
             .arg( osep, 0, 'f', 2 )
         : QString();

      if ( qualifies && count_ok && discard_outlier && n_outlier_dropped < outlier_max )
      {
         // regplot red "x" shows the FIRST (strongest) discard; with several drops the plot marks
         // only that one, while the log below records every discard.
         if ( excl_name.isEmpty() )
         {
            excl_name = ordered_names[ j_star ];
            excl_conc = concs[ j_star ];
            bool istar_j = is_istar[ j_star ];
            reg_excl_y.assign( npts, numeric_limits < double >::quiet_NaN() );
            const vector < double > & Iarr = name_to_I[ excl_name ];
            for ( unsigned int qi = 0; qi < npts && qi < Iarr.size(); qi++ )
            {
               double Iv = Iarr[ qi ];
               if ( us_isnan( Iv ) ) { continue; }
               if ( extrap_model >= 1 )              // reciprocal / virial axis c/I
               {
                  if ( Iv <= 0e0 ) { continue; }
                  reg_excl_y[ qi ] = istar_j ? 1e0 / Iv : excl_conc / Iv;
               }
               else                                  // additive axis I/c
               {
                  reg_excl_y[ qi ] = istar_j ? Iv : Iv / excl_conc;
               }
            }
         }

         editor_msg( "red",
                    QString( us_tr( "Outlier QC: DISCARDED (%1 of at most %2) -- %3. Refit on %4 curves.\n" ) )
                    .arg( n_outlier_dropped + 1 ).arg( outlier_max ).arg( detail ).arg( n_participating - 1 ) );

         ordered_names.removeAt( j_star );
         concs.erase( concs.begin() + j_star );
         is_istar.erase( is_istar.begin() + j_star );

         // refresh the I*(q) bookkeeping after removal
         istar_count = 0;
         for ( int ci = 0; ci < ordered_names.size(); ci++ ) { if ( is_istar[ ci ] ) { istar_count++; } }
         all_istar = ( istar_count == (unsigned int) ordered_names.size() );
         n_outlier_dropped++;
         continue;                                // re-detect on the cleaned set
      }

      if ( discard_outlier && n_outlier_dropped >= outlier_max && qualifies && count_ok )
      {
         // hit the cap with another curve still qualifying -- say so, don't silently stop
         editor_msg( "dark red",
                    QString( us_tr( "Outlier QC: discard limit (%1) reached; %2 still qualifies but was kept. "
                                    "Raise the limit if more than %1 curve(s) are suspect.\n" ) )
                    .arg( outlier_max ).arg( detail ) );
      }
      else if ( discard_outlier )
      {
         // auto-discard on but nothing dropped: always say why, so the thresholds can be tuned
         if ( j_star < 0 )
         {
            editor_msg( "black",
                       QString( us_tr( "Outlier QC: too few curves to evaluate (need >= 4 with a concentration).\n" ) ) );
         }
         else
         {
            QStringList fails;
            if ( !g_sigma )  { fails << us_tr( "residual below sigma threshold" ); }
            if ( !g_sign )   { fails << us_tr( "not systematic enough (<70% one-sided)" ); }
            if ( !g_gain )   { fails << us_tr( "chi^2 gain below threshold" ); }
            if ( qualifies && !count_ok ) { fails << us_tr( "too few curves would remain to drop safely" ); }
            editor_msg( "dark red",
                       QString( us_tr( "Outlier QC: kept (%1) -- %2.\n" ) )
                       .arg( fails.join( "; " ) ).arg( detail ) );
         }
      }
      else if ( qualifies && count_ok )
      {
         // auto-discard off: flag a would-be discard so the user can act
         editor_msg( "dark red",
                    QString( us_tr( "Outlier QC: %1 looks like an outlier. Not discarded (enable "
                                    "\"Automatically discard one outlier concentration\" to act, or verify this concentration).\n" ) )
                    .arg( detail ) );
      }
      break;                                      // nothing (further) discarded -> stop iterating
   }

   // Absolute-scale setup: pick the reference (highest-concentration) curve, compute a
   // least-squares scale factor for every curve onto that reference over the common
   // q-grid, and grab the reference curve's error column to carry into the output.

   int    ref_ci   = -1;
   double ref_override_conc_used = 0e0;         // >0 => the reference was user-selected (name token)
   double ref_conc = 0e0;
   vector < double > ref_sd;
   vector < double > Iref_full;                 // reference (highest-conc) raw I(q), full grid
   int    merge_idx = 0;                        // grid index: output = reference from here up
   double merge_q   = 0e0;                      // q at the merge point (0 => no merge / all reference)
   double pvalue_alpha = 1e-2;                  // CORMAP merge-point significance (cf. cormap_alpha)

   // GCV effective slope dof actually used (0 => GCV not applied); surfaced to the
   // regression viewer so it can annotate that automatic regularization was in effect
   double gcv_edof_used = 0e0;

   if ( use_ref_path )
   {
      {
         US_Hydrodyn *uh = (US_Hydrodyn *) us_hydrodyn;
         if ( uh->gparams.count( "saxs_extrap_c0_pvalue_alpha" ) )
         {
            pvalue_alpha = uh->gparams[ "saxs_extrap_c0_pvalue_alpha" ].toDouble();
         }
      }

      // Reference selection. Default = highest concentration, which is the right DEFAULT because
      // intensity scales with concentration, so the top curve normally has the best high-q
      // signal-to-noise -- exactly what the splice needs. It is only wrong when that particular
      // curve is defective (aggregation, a mis-set concentration, radiation damage, bad buffer
      // subtraction), and that cannot be detected from the concentration regression itself: the
      // top curve is the extreme x, so its leverage approaches 1, the fit passes essentially
      // through it and its residual carries almost no information about it. Judging it needs
      // knowledge from outside the regression (e.g. MW from I(0) against the known value), which
      // the user may well have and the program does not -- hence this explicit override.
      QString ref_override_note;
      if ( !ref_override.isEmpty() )
      {
         QString want = ref_override.trimmed();
         want.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );
         bool   numeric = false;
         double want_c  = want.toDouble( &numeric );
         int    hit     = -1;
         for ( int ci = 0; ci < ordered_names.size(); ci++ )
         {
            QString nm = ordered_names[ ci ];
            nm.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );
            bool match = numeric
               ? ( concs[ ci ] > 0e0 && qAbs( concs[ ci ] - want_c ) <= 1e-6 + 1e-6 * qAbs( want_c ) )
               : ( nm == want || nm.contains( want ) );
            if ( match && concs[ ci ] > 0e0 ) { hit = ci; break; }
         }
         if ( hit >= 0 )
         {
            ref_ci   = hit;
            ref_conc = concs[ hit ];
            ref_override_note = us_tr( " [user-selected reference]" );
            ref_override_conc_used = ref_conc;

            // The high-q splice is built around the reference being the HIGHEST concentration:
            // the merge point is where the extrapolation stops differing from it, i.e. where the
            // interparticle contribution has died away. A lower-concentration reference is itself
            // closer to dilute AND has larger error bars, so the agreement test passes almost
            // immediately -- the merge collapses towards q_min and the output becomes that single
            // raw curve instead of the de-noised extrapolation. Warn; do not silently comply.
            double max_conc = 0e0;
            for ( int ci = 0; ci < ordered_names.size(); ci++ )
            {
               if ( concs[ ci ] > max_conc ) { max_conc = concs[ ci ]; }
            }
            if ( merge_ref && ref_conc < max_conc )
            {
               extrap_msg(
                          QString( us_tr( "Reference override selects conc %1, which is NOT the highest concentration "
                                          "(%2), while the high-q splice is enabled. The splice takes the reference "
                                          "verbatim above the merge point, and a lower-concentration reference is both "
                                          "nearer the dilute limit and noisier, so the merge point will move down and "
                                          "may replace most of the extrapolation with this single raw curve. Check the "
                                          "reported merge point, or turn the splice off." ) )
                          .arg( ref_conc ).arg( max_conc ) );
            }
         }
         else
         {
            // never silently ignore an override -- the user asked for a specific curve
            extrap_msg(
                       QString( us_tr( "Reference override \"%1\" does not match any selected curve with a "
                                       "positive concentration; falling back to the highest concentration." ) )
                       .arg( want ) );
         }
      }

      if ( ref_ci < 0 )
      {
         for ( int ci = 0; ci < ordered_names.size(); ci++ )
         {
            if ( concs[ ci ] > ref_conc )
            {
               ref_conc = concs[ ci ];
               ref_ci   = ci;
            }
         }
      }

      if ( ref_ci < 0 )
      {
         extrap_msg(
                                us_tr( "Absolute-scale / reference-splice: no curve has a positive concentration "
                                       "to use as the reference; aborting." ) );
         return;
      }

      const vector < double > &Iref = name_to_I[ ordered_names[ ref_ci ] ];

      // The fit itself is the normalized concentration-model fit (below), identical to the
      // Conc:1 output; the reference curve is used only to place the output on its absolute
      // scale (ref_scale, multiply the normalized fit by c_ref) and/or to splice its clean
      // high-q data above the merge point (merge_ref).
      editor_msg( "black",
                 QString( us_tr( "Reference (%1) curve: \"%2\" (conc %3)%4%5%6\n" ) )
                 .arg( ref_override_note.isEmpty() ? us_tr( "highest-concentration" ) : us_tr( "user-selected" ) )
                 .arg( ordered_names[ ref_ci ] ).arg( ref_conc )
                 .arg( ref_override_note )
                 .arg( ref_scale ? us_tr( "; output on its absolute scale" ) : QString( "" ) )
                 .arg( merge_ref ? us_tr( "; high-q reference splice enabled" ) : QString( "" ) ) );

      // the extrapolated curve inherits the reference curve's error bars (matches almerge).
      // Source from name_to_err (already per-q aligned to npts), so a transient input-SD
      // recompute, if enabled, is reflected in the merge test too; fall back to the raw error
      // map only if the reference somehow has no parsed errors.
      if ( name_to_err.count( ordered_names[ ref_ci ] ) && !name_to_err[ ordered_names[ ref_ci ] ].empty() )
      {
         ref_sd = name_to_err[ ordered_names[ ref_ci ] ];
         ref_sd.resize( npts );
      }
      else if ( name_to_errors_map.count( ordered_names[ ref_ci ] ) )
      {
         QStringList qsl_err = name_to_errors_map[ ordered_names[ ref_ci ] ].split( "," );
         for ( int k = 2; k < qsl_err.size(); k++ )
         {
            ref_sd.push_back( qsl_err[ k ].toDouble() );
         }
         if ( !ref_sd.empty() )
         {
            ref_sd.pop_back();
         }
         ref_sd.resize( npts );
      }

      // reference raw I(q) on the full grid (sanitized), used as the high-q output
      // and as the comparison curve for the merging-point p-value test
      Iref_full.resize( npts );
      for ( unsigned int qi = 0; qi < npts; qi++ )
      {
         Iref_full[ qi ] = us_isnan( Iref[ qi ] ) ? 0e0 : Iref[ qi ];
      }

   }

   // 6. per-q linear regression -> intercept (I0) + error
   //    Zimm mode:   fit I(q)/c            vs concentration; error = regression SE
   //    absolute-scale mode: fit scale*I(q) (abs)  vs concentration; error = reference curve sd

   US_Saxs_Util usu;

   vector < double > out_q;
   vector < double > out_I0;
   vector < double > out_I0_err;
   vector < int >    out_qidx;                  // full-grid q index of each output point
   unsigned int       skipped_points = 0;
   unsigned int       n_missing_sd_pts = 0;     // points lacking a usable SD while weighting is on (logged)
   // Per-cause skip tallies. All four causes previously reported as "fewer than 2 valid data
   // points", which is wrong for three of them and actively misleading when the real cause is a
   // non-positive reciprocal intercept -- report each cause with its own count instead.
   unsigned int       n_skip_fewpts   = 0;      // < 2 usable points at this q
   unsigned int       n_skip_dupconc  = 0;      // exactly 2 points at the same concentration
   unsigned int       n_skip_virial2  = 0;      // 2nd-order virial needs >= 4 points
   unsigned int       n_skip_nonposu  = 0;      // reciprocal intercept <= 0 (cannot invert to I(0))
   unsigned int       n_gcv_fallback  = 0;      // GCV intercept was non-positive; per-q fit used instead

   // per-q regression data captured for the optional scrollable regression-plot pop-up:
   // the (concentration, y, y-error) points, plus the fit intercept, slope and its SE
   vector < double >               reg_q;
   vector < vector < double > >    reg_x;
   vector < vector < double > >    reg_y;
   vector < vector < double > >    reg_e;
   vector < double >               reg_a;
   vector < double >               reg_b;
   vector < double >               reg_c;      // quadratic coeff (2nd-virial); 0 for linear fits
   vector < double >               reg_siga;

   // per-output-point concentration centroids, used only for Zimm fit-broadening:
   // for an OLS line a = ybar - b*xbar, so after smoothing the slope the intercept
   // is recomputed as ybar - b_smoothed*xbar (parallel to out_q)
   vector < double >               out_xbar;
   vector < double >               out_ybar;

   // Intensity-vs-concentration model for the Zimm-family fit (absolute-scale mode is
   // unaffected). Selected via gparam saxs_extrap_c0_model:
   //   0 = additive    I(q)/c = Iex + alpha*c            (cleanest low-q profile)
   //   1 = reciprocal  c/I(q) = u + v*c,       I0 = 1/u  (default; 2nd-virial form, unbiased MW)
   //   2 = virial2     c/I(q) = u + v*c + w*c^2, I0 = 1/u (robust MW at strong interaction)
   // The additive fit is biased low for a saturating (second-virial) structure factor because
   // I(q,c)/c is convex in c; the reciprocal c/I is linear in c and unbiased, and the 2nd-order
   // form absorbs the residual curvature at strong interaction. Validated against a physical
   // Percus-Yevick hard-sphere ground truth. Model 2 needs >= 4-5 well-spread concentrations.
   // extrap_model is set by the concentration dialog (default 0); a power-user default can be
   // preselected via gparam saxs_extrap_c0_model.
   bool reciprocal = ( extrap_model >= 1 );
   if ( reciprocal )
   {
      editor_msg( "black",
                  QString( us_tr( "Extrapolation model: %1 (fits c/I vs concentration; I0 = 1/intercept)\n" ) )
                  .arg( extrap_model == 2 ? us_tr( "2nd-order virial" ) : us_tr( "reciprocal (2nd-virial)" ) ) );
   }

   // Zimm GCV-penalized fit: per-q weighted stats over the whole grid -> a globally
   // slope-regularized intercept Iex(q) (and slope) at every q, lambda auto-tuned by GCV.
   // Used in the main loop below when GCV is enabled (absolute-scale does its own GCV above).
   vector < double > zimm_Iex, zimm_alpha;
   bool   zimm_gcv_ok = false;
   if ( use_gcv )
   {
      vector < double > vM( npts ), vR( npts ), vB( npts ), vC( npts ), vQ( npts );
      for ( unsigned int qi = 0; qi < npts; qi++ )
      {
         // Collect this q's points first, so an unweighted run can derive a per-q scale below.
         vector < double > qx, qy, qsig;
         for ( int ci = 0; ci < ordered_names.size(); ci++ )
         {
            if ( concs[ ci ] <= 0e0 )
            {
               continue;
            }
            double Iv = name_to_I[ ordered_names[ ci ] ][ qi ];
            if ( us_isnan( Iv ) )
            {
               continue;
            }
            double sd = ( name_to_err.count( ordered_names[ ci ] ) && qi < name_to_err[ ordered_names[ ci ] ].size() )
               ? name_to_err[ ordered_names[ ci ] ][ qi ] : 0e0;
            if ( us_isnan( sd ) ) { sd = 0e0; }
            // Zimm axis: additive y = I/c (raw) or I as-is (I*(q)); reciprocal z = c/I
            // (for I*(q), I = Iv*c so z = 1/Iv). weight 1/sig^2; c/I needs I>0.
            double yv, sig;
            if ( reciprocal )
            {
               if ( Iv <= 0e0 ) { continue; }
               if ( is_istar[ ci ] ) { yv = 1e0 / Iv;            sig = sd / ( Iv * Iv ); }
               else                  { yv = concs[ ci ] / Iv;    sig = sd * concs[ ci ] / ( Iv * Iv ); }
            }
            else if ( is_istar[ ci ] ) { yv = Iv;              sig = sd; }
            else                       { yv = Iv / concs[ ci ]; sig = sd / concs[ ci ]; }
            qx.push_back( concs[ ci ] ); qy.push_back( yv ); qsig.push_back( sig );
         }

         // Per-q scale for the UNWEIGHTED case (weighted runs are untouched).
         // With unit weights every q contributes to the joint GCV fit in proportion to its own
         // magnitude. On the reciprocal (c/I) axis |y| spans orders of magnitude across q, so the
         // huge, noisy high-q points dominate lambda selection: GCV then over-smooths alpha(q)
         // into a near-constant, and Iex = (Q - B*alpha)/C goes NEGATIVE from the lowest q upward
         // -- every such q is dropped and the Guinier region is destroyed. Re-weighting each q by
         // 1/scale^2 restores the balanced conditioning the 1/sigma^2 path gets for free. The
         // factor is CONSTANT WITHIN a q, so it leaves every per-q fit (slope R/M and intercept
         // Iex) algebraically unchanged -- it only rebalances q against each other in the joint
         // penalized solve. It amounts to assuming constant RELATIVE error, the standard default
         // when error bars are absent or deliberately not trusted.
         double inv_scale2 = 1e0;
         if ( !use_sd_weights && qy.size() )
         {
            double acc = 0e0;
            for ( unsigned int k = 0; k < qy.size(); k++ ) { acc += qAbs( qy[ k ] ); }
            double scale = acc / (double) qy.size();
            if ( scale > 0e0 && !us_isnan( scale ) ) { inv_scale2 = 1e0 / ( scale * scale ); }
         }

         double S = 0e0, Sx = 0e0, Sy = 0e0, Sxx = 0e0, Sxy = 0e0;
         for ( unsigned int k = 0; k < qy.size(); k++ )
         {
            double sig = qsig[ k ];
            double w   = ( use_sd_weights && sig > 0e0 && !us_isnan( sig ) )
               ? 1e0 / ( sig * sig )
               : inv_scale2;                       // 1.0 for weighted-but-missing-sd (unchanged)
            double xv = qx[ k ], yv = qy[ k ];
            S += w; Sx += w * xv; Sy += w * yv; Sxx += w * xv * xv; Sxy += w * xv * yv;
         }
         vC[ qi ] = S; vB[ qi ] = Sx; vQ[ qi ] = Sy;
         vM[ qi ] = ( S > 0e0 ) ? ( Sxx - Sx * Sx / S ) : 0e0;
         vR[ qi ] = ( S > 0e0 ) ? ( Sxy - Sx * Sy / S ) : 0e0;
      }
      double gcv_lambda = 0e0, gcv_edof = 0e0;
      zimm_gcv_ok = us_extrap_c0_gcv_penalized( vM, vR, vB, vC, vQ, zimm_Iex, zimm_alpha, gcv_lambda, gcv_edof );
      if ( zimm_gcv_ok )
      {
         gcv_edof_used = gcv_edof;
         editor_msg( "black",
                    QString( us_tr( "Zimm GCV slope regularization applied: lambda = %1, effective slope dof = %2 of %3\n" ) )
                    .arg( gcv_lambda ).arg( gcv_edof, 0, 'f', 1 ).arg( (int) npts ) );
      }
   }

   // 2nd-order virial precompute (model 2): per-q weighted quadratic  c/I = u + v*c + w*c^2,
   // I0 = 1/u. Solves the 3x3 weighted normal equations by Cramer's rule; SE(u) from the
   // (0,0) element of the inverse normal matrix (weights are inverse variances). Needs >= 4
   // concentrations for a stable 3-parameter fit; degenerate q fall back to being skipped.
   vector < double > virial2_u( npts, 0e0 ), virial2_use( npts, 0e0 );
   vector < double > virial2_v( npts, 0e0 ), virial2_w( npts, 0e0 );   // slope/curvature (diagnostic plot)
   vector < bool >   virial2_ok( npts, false );
   if ( reciprocal && extrap_model == 2 )
   {
      for ( unsigned int qi = 0; qi < npts; qi++ )
      {
         double S0=0e0,S1=0e0,S2=0e0,S3=0e0,S4=0e0,Z0=0e0,Z1=0e0,Z2=0e0; int nvalid=0;
         for ( int ci = 0; ci < ordered_names.size(); ci++ )
         {
            if ( concs[ ci ] <= 0e0 ) { continue; }
            double Iv = name_to_I[ ordered_names[ ci ] ][ qi ];
            if ( us_isnan( Iv ) || Iv <= 0e0 ) { continue; }
            double sd = ( name_to_err.count( ordered_names[ ci ] ) && qi < name_to_err[ ordered_names[ ci ] ].size() )
               ? name_to_err[ ordered_names[ ci ] ][ qi ] : 0e0;
            if ( us_isnan( sd ) ) { sd = 0e0; }
            double cc = concs[ ci ];
            double zz = is_istar[ ci ] ? 1e0 / Iv         : cc / Iv;
            double sz = is_istar[ ci ] ? sd / ( Iv * Iv ) : sd * cc / ( Iv * Iv );
            double w  = ( use_sd_weights && sz > 0e0 && !us_isnan( sz ) ) ? 1e0 / ( sz * sz ) : 1e0;
            S0+=w; S1+=w*cc; S2+=w*cc*cc; S3+=w*cc*cc*cc; S4+=w*cc*cc*cc*cc;
            Z0+=w*zz; Z1+=w*cc*zz; Z2+=w*cc*cc*zz; nvalid++;
         }
         if ( nvalid < 4 ) { continue; }
         double det = S0*(S2*S4-S3*S3) - S1*(S1*S4-S3*S2) + S2*(S1*S3-S2*S2);
         if ( det == 0e0 || us_isnan( det ) ) { continue; }
         double u = ( Z0*(S2*S4-S3*S3) - S1*(Z1*S4-Z2*S3) + S2*(Z1*S3-Z2*S2) ) / det;
         if ( u <= 0e0 || us_isnan( u ) ) { continue; }
         double v = ( S0*(Z1*S4-S3*Z2) - Z0*(S1*S4-S3*S2) + S2*(S1*Z2-Z1*S2) ) / det;
         double w = ( S0*(S2*Z2-Z1*S3) - S1*(S1*Z2-Z1*S2) + Z0*(S1*S3-S2*S2) ) / det;
         double cof00 = S2*S4 - S3*S3;                // cofactor -> inverse[0,0] = cof00/det
         virial2_u[ qi ]   = u;
         virial2_v[ qi ]   = v;
         virial2_w[ qi ]   = w;
         virial2_use[ qi ] = ( cof00 > 0e0 && det > 0e0 ) ? std::sqrt( cof00 / det ) : 0e0;
         virial2_ok[ qi ]  = true;
      }
   }

   for ( unsigned int qi = 0; qi < npts; qi++ )
   {
      vector < double > x;
      vector < double > y;
      vector < double > ye;

      for ( int ci = 0; ci < ordered_names.size(); ci++ )
      {
         if ( concs[ ci ] <= 0e0 )
         {
            continue;
         }
         double Iv = name_to_I[ ordered_names[ ci ] ][ qi ];
         if ( us_isnan( Iv ) )
         {
            continue;
         }
         double sd = 0e0;
         if ( name_to_err.count( ordered_names[ ci ] ) && qi < name_to_err[ ordered_names[ ci ] ].size() )
         {
            sd = name_to_err[ ordered_names[ ci ] ][ qi ];
            if ( us_isnan( sd ) )
            {
               sd = 0e0;
            }
         }
         double yv;
         double yev;
         if ( reciprocal )
         {
            // reciprocal axis z = c/I (for I*(q), I = Iv*c so z = 1/Iv); needs I>0
            if ( Iv <= 0e0 ) { continue; }
            if ( is_istar[ ci ] ) { yv = 1e0 / Iv;          yev = sd / ( Iv * Iv ); }
            else                  { yv = concs[ ci ] / Iv;  yev = sd * concs[ ci ] / ( Iv * Iv ); }
         }
         else if ( is_istar[ ci ] )
         {
            yv  = Iv;                // already I*(q)/concentration-normalized; do not re-divide
            yev = sd;
         }
         else
         {
            yv  = Iv / concs[ ci ];
            yev = sd / concs[ ci ];
         }
         x.push_back( concs[ ci ] );
         y.push_back( yv );
         ye.push_back( yev );
      }

      // inverse-variance weighting is used only when EVERY point at this q carries a usable
      // error (the weighted linear_fit divides by e^2, and its built-in fallback only triggers
      // when ALL errors are zero); otherwise this q falls back to unweighted. Points lacking a
      // usable SD are counted so the run can report that weighting was partial.
      bool all_err = ( x.size() > 0 );
      for ( int j = 0; j < (int) ye.size(); j++ )
      {
         if ( !( ye[ j ] > 0e0 ) || us_isnan( ye[ j ] ) )
         {
            all_err = false;
            if ( use_sd_weights ) { n_missing_sd_pts++; }
         }
      }
      bool weight_here = use_sd_weights && all_err;

      double a = 0e0, b = 0e0, siga = 0e0;
      // Unregularized per-q intercept, kept as a safety net: if the GCV-smoothed intercept comes
      // out non-positive on the reciprocal axis (so it cannot be inverted to I(0)), fall back to
      // this rather than dropping the q point entirely.
      double perq_a      = 0e0;
      bool   have_perq_a = false;

      if ( reciprocal && extrap_model == 2 )
      {
         // 2nd-order virial: intercept u of the per-q weighted quadratic c/I = u + v*c + w*c^2
         // (precomputed above); inverted to I0 = 1/u after the cascade.
         if ( !virial2_ok[ qi ] )
         {
            TSO << QString( "WARNING: do_extrap_c0: 2nd-order virial fit unavailable at q[%1]=%2 (need >=4 valid points), skipping\n" )
               .arg( qi ).arg( q[ qi ] );
            skipped_points++; n_skip_virial2++;
            continue;
         }
         a    = virial2_u[ qi ];
         b    = 0e0;
         siga = virial2_use[ qi ];
      }
      else if ( zimm_gcv_ok )
      {
         // GCV-penalized Zimm: intercept/slope come from the globally slope-smoothed fit
         // (defined at every q, so no q is skipped). The reported error is the per-q OLS
         // intercept SE where computable -- a conservative bound, since the regularized
         // estimate borrows information across q and is actually less uncertain.
         a = zimm_Iex[ qi ];
         b = zimm_alpha[ qi ];
         if ( x.size() >= 3 )
         {
            double aa, bb, sigb, chi2;
            if ( weight_here ) { usu.linear_fit( x, y, ye, aa, bb, siga, sigb, chi2 ); }
            else               { usu.linear_fit( x, y,     aa, bb, siga, sigb, chi2 ); }
            perq_a = aa; have_perq_a = true;   // safety-net intercept (see reciprocal inversion)
         }
         else
         {
            siga = 0e0;
         }
      }
      else if ( x.size() < 2 )
      {
         TSO << QString( "WARNING: do_extrap_c0: insufficient valid points (%1) to regress at q[%2]=%3, skipping\n" )
            .arg( x.size() ).arg( qi ).arg( q[ qi ] );
         skipped_points++; n_skip_fewpts++;
         continue;
      }
      else if ( x.size() == 2 )
      {
         if ( x[ 1 ] == x[ 0 ] )
         {
            TSO << QString( "WARNING: do_extrap_c0: duplicate concentrations with only 2 points at q[%1]=%2, skipping\n" )
               .arg( qi ).arg( q[ qi ] );
            skipped_points++; n_skip_dupconc++;
            continue;
         }
         b    = ( y[ 1 ] - y[ 0 ] ) / ( x[ 1 ] - x[ 0 ] );
         a    = y[ 0 ] - b * x[ 0 ];
         siga = 0e0;
         TSO << QString( "WARNING: do_extrap_c0: exactly 2 valid points at q[%1]=%2, intercept SE undefined, reporting 0\n" )
            .arg( qi ).arg( q[ qi ] );
      }
      else
      {
         double sigb, chi2;
         if ( weight_here ) { usu.linear_fit( x, y, ye, a, b, siga, sigb, chi2 ); }
         else               { usu.linear_fit( x, y,     a, b, siga, sigb, chi2 ); }
      }

      // Capture the fit in the plotted axis (additive: I/c; reciprocal/virial: c/I) for the
      // regression-plot viewer, BEFORE any reciprocal inversion of the output value. For the
      // 2nd-order virial the slope/curvature come from the quadratic solve.
      double fit_a = a, fit_b = b, fit_c = 0e0, fit_siga = siga;
      if ( reciprocal && extrap_model == 2 && virial2_ok[ qi ] )
      {
         fit_b = virial2_v[ qi ];
         fit_c = virial2_w[ qi ];
      }

      // Reciprocal / virial models fit c/I, so the fitted intercept is 1/I0(q); invert to
      // recover the intensity I0(q) = 1/u and propagate SE(1/u) = SE(u)/u^2.
      if ( reciprocal )
      {
         // Safety net: a non-positive intercept cannot be inverted to I(0). If the value came from
         // the GCV-smoothed fit, prefer the unregularized per-q intercept for this q instead of
         // dropping the point -- dropping is what previously wiped out the low-q (Guinier) region
         // whenever the regularization over-smoothed. Only skip if both are unusable.
         if ( ( a <= 0e0 || us_isnan( a ) ) && have_perq_a && perq_a > 0e0 && !us_isnan( perq_a ) )
         {
            a = perq_a;
            n_gcv_fallback++;
         }
         if ( a <= 0e0 || us_isnan( a ) )
         {
            TSO << QString( "WARNING: do_extrap_c0: non-positive reciprocal intercept at q[%1]=%2, skipping\n" )
               .arg( qi ).arg( q[ qi ] );
            skipped_points++; n_skip_nonposu++;
            continue;
         }
         siga = siga / ( a * a );
         a    = 1e0 / a;
         b    = 0e0;                // reciprocal slope is in 1/I space; not meaningful post-inversion
      }

      // Error bars: Zimm reports the regression SE of the intercept. absolute-scale reports the
      // reference curve's error at/above the merge point (where the output IS the
      // reference), but the propagated intercept SE below the merge point (the
      // extrapolated region), so the low-q extrapolation carries its own uncertainty
      // rather than borrowing the reference's.
      // Normalized fit: the reported value is the intercept (I0/c per unit concentration)
      // and its regression SE. The reference-scale (x c_ref) and high-q reference splice, if
      // requested, are applied as a post-pass over the full grid after this loop.
      double err_val = siga;
      double out_val = a;

      double xbar = 0e0, ybar = 0e0;
      for ( int j = 0; j < (int) x.size(); j++ )
      {
         xbar += x[ j ];
         ybar += y[ j ];
      }
      if ( x.size() )
      {
         xbar /= (double) x.size();
         ybar /= (double) y.size();
      }

      out_q     .push_back( q[ qi ] );
      out_I0    .push_back( out_val );
      out_I0_err.push_back( err_val );
      out_xbar  .push_back( xbar );
      out_ybar  .push_back( ybar );
      out_qidx  .push_back( (int) qi );        // full-grid index (for the reference post-pass)

      reg_q   .push_back( q[ qi ] );
      reg_x   .push_back( x );
      reg_y   .push_back( y );
      reg_e   .push_back( ye );
      reg_a   .push_back( fit_a );             // fit intercept in the plotted axis (diagnostic)
      reg_b   .push_back( fit_b );             // slope
      reg_c   .push_back( fit_c );             // quadratic coeff (2nd-virial), 0 otherwise
      reg_siga.push_back( reciprocal ? fit_siga : err_val );   // SE in the plotted axis
   }

   // The GCV-smoothed intercept was unusable at some q and the unregularized per-q fit was used
   // there instead. Report it: the points are in the output, but they are NOT regularized, and a
   // large count means the slope regularization is fighting the data (see the reciprocal/unweighted
   // note below) rather than helping.
   if ( n_gcv_fallback )
   {
      extrap_msg(
                            QString( us_tr( "Slope regularization (GCV) produced a non-positive intercept at %1 of %2 "
                                            "q-points; the unregularized per-q fit was used at those q instead of "
                                            "dropping them. Those points are therefore not regularized. If this count "
                                            "is large, prefer turning GCV off for this dataset." ) )
                            .arg( n_gcv_fallback ).arg( npts ) );
   }

   if ( skipped_points )
   {
      // Report the ACTUAL cause(s). Previously every cause was reported as "fewer than 2 valid
      // data points", which is wrong for three of the four and hid the common real case (a
      // non-positive reciprocal intercept, which cannot be inverted to I(0)).
      QStringList why;
      if ( n_skip_fewpts )  { why << QString( us_tr( "%1 with fewer than 2 valid data points" ) ).arg( n_skip_fewpts ); }
      if ( n_skip_dupconc ) { why << QString( us_tr( "%1 with only 2 points at the same concentration" ) ).arg( n_skip_dupconc ); }
      if ( n_skip_virial2 ) { why << QString( us_tr( "%1 where the 2nd-order virial fit needs >= 4 concentrations" ) ).arg( n_skip_virial2 ); }
      if ( n_skip_nonposu ) { why << QString( us_tr( "%1 with a non-positive fitted 1/I(0) intercept (cannot invert to I(0))" ) ).arg( n_skip_nonposu ); }
      extrap_msg(
                            QString( us_tr( "%1 of %2 q-points could not be extrapolated and were skipped: %3." ) )
                            .arg( skipped_points ).arg( npts ).arg( why.join( "; " ) ) );
   }

   // weighting requested but some points had no usable error: those q fell back to unweighted.
   // Surface it (non-blocking editor message -- never a pop-up, so it doesn't hang headless runs)
   // rather than silently mixing weighted and unweighted fits. The usual cause is a curve carrying
   // one or more non-positive SD points: SOMO's error handling is all-or-nothing, so that disables
   // the whole curve's errors. Repair the SDs (the scripted path can auto-fill edge zeros) or, if
   // you intend an unweighted extrapolation, turn off "Weight regression by curve errors".
   if ( use_sd_weights && n_missing_sd_pts )
   {
      editor_msg( "dark red",
                 QString( us_tr( "Note: inverse-variance weighting was requested, but %1 data point(s) had no "
                                 "usable error (a curve likely carries non-positive SD points, which disables its "
                                 "errors entirely); those q were fitted unweighted. Repair the input SDs, or turn "
                                 "off error weighting for an intentionally unweighted fit.\n" ) )
                 .arg( n_missing_sd_pts ) );
   }

   if ( out_q.empty() )
   {
      extrap_msg( us_tr( "No q-points could be extrapolated; aborting." ) );
      return;
   }

   // 6b. Zimm fit broadening: the interparticle/interaction slope varies smoothly with
   //     q (and dies off at high q), so smoothing the per-q concentration slope across
   //     a q-window and recomputing the intercept (a = ybar - b_smooth*xbar) removes
   //     extrapolation noise without smearing the form-factor detail carried by the
   //     intercept. Off by default (window <= 1); absolute-scale mode is unaffected. Superseded
   //     by GCV regularization when that is enabled (the two are mutually exclusive).
   if ( !reciprocal && !zimm_gcv_ok && fit_broaden > 1 )
   {
      int n = (int) out_q.size();
      int half = fit_broaden / 2;
      vector < double > b_smooth( n );
      for ( int k = 0; k < n; k++ )
      {
         int lo = k - half; if ( lo < 0 ) lo = 0;
         int hi = k + half; if ( hi > n - 1 ) hi = n - 1;
         double sum = 0e0;
         for ( int j = lo; j <= hi; j++ )
         {
            sum += reg_b[ j ];
         }
         b_smooth[ k ] = sum / (double) ( hi - lo + 1 );
      }
      for ( int k = 0; k < n; k++ )
      {
         double a_new = out_ybar[ k ] - b_smooth[ k ] * out_xbar[ k ];
         out_I0[ k ] = a_new;
         reg_a[ k ]  = a_new;
         reg_b[ k ]  = b_smooth[ k ];
      }
      editor_msg( "black",
                 QString( us_tr( "Zimm fit broadening applied: concentration slope smoothed over a "
                                 "%1-point q-window before taking the c=0 intercept.\n" ) )
                 .arg( fit_broaden ) );
   }

   // 6b2. Output conventions (post-pass over the full grid). The fit above is always the
   //      normalized (Conc:1) concentration-model curve; here we optionally (a) place it on the
   //      reference (max-concentration) absolute scale by multiplying by c_ref [ref_scale], and
   //      (b) splice the reference curve's clean high-q data above an almerge-style merge point
   //      [merge_ref], taking the reference's error bars there. The two are independent.
   if ( use_ref_path )
   {
      double c_ref = ref_conc;
      vector < int > grid_to_out( npts, -1 );
      for ( int k = 0; k < (int) out_qidx.size(); k++ )
      {
         if ( out_qidx[ k ] >= 0 && out_qidx[ k ] < (int) npts ) { grid_to_out[ out_qidx[ k ] ] = k; }
      }

      merge_idx = (int) npts;                    // default: no splice (whole curve extrapolated)
      merge_q   = 0e0;
      // Manual switchover: if the user pinned a q, splice there instead of auto-locating the point.
      // The automatic search finds where the extrapolation agrees with the reference within its
      // errors; pinning it lets the user test a specific cut, or override a placement they disagree
      // with (e.g. when the regularization has distorted the curve near the automatic point).
      if ( merge_ref && merge_q_override > 0e0 )
      {
         int    j_best = (int) npts; double d_best = 1e30;
         for ( int j = 0; j < (int) npts; j++ )
         {
            if ( grid_to_out[ j ] < 0 ) { continue; }        // only where the output is defined
            double d = qAbs( q[ j ] - merge_q_override );
            if ( d < d_best ) { d_best = d; j_best = j; }
         }
         merge_idx = j_best;
         merge_q   = ( merge_idx < (int) npts ) ? q[ merge_idx ] : 0e0;
         editor_msg( "black",
                    QString( us_tr( "Reference merge point: q = %1 (index %2 of %3), pinned to the requested "
                                    "q = %4 [manual switchover]\n" ) )
                    .arg( merge_q ).arg( merge_idx ).arg( (int) npts ).arg( merge_q_override ) );
      }
      else if ( merge_ref )
      {
         double merge_chi2 = 4e0;
         {
            US_Hydrodyn *uh = (US_Hydrodyn *) us_hydrodyn;
            if ( uh->gparams.count( "saxs_extrap_c0_merge_chi2" ) )
            {
               merge_chi2 = uh->gparams[ "saxs_extrap_c0_merge_chi2" ].toDouble();
            }
         }
         bool have_ref_err = false;
         for ( unsigned int qi = 0; qi < npts && qi < ref_sd.size(); qi++ )
         {
            if ( ref_sd[ qi ] > 0e0 ) { have_ref_err = true; break; }
         }
         int win = 50;
         if ( (int) npts <= win ) { win = (int) npts / 2; }
         if ( win < 5 )
         {
            merge_idx = (int) npts;
            editor_msg( "dark red",
                       QString( us_tr( "Reference splice: only %1 q-point(s) -- too few to locate a merge point; "
                                       "extrapolating the whole curve.\n" ) ).arg( (int) npts ) );
         }
         else if ( have_ref_err )
         {
            merge_idx = (int) npts - win;
            for ( int j = 0; j + win <= (int) npts; j++ )
            {
               double num = 0e0; int m = 0;
               for ( int i = j; i < j + win; i++ )
               {
                  if ( grid_to_out[ i ] >= 0 && i < (int) ref_sd.size() && ref_sd[ i ] > 0e0 )
                  {
                     double r = ( out_I0[ grid_to_out[ i ] ] * c_ref - Iref_full[ i ] ) / ref_sd[ i ];
                     num += r * r; m++;
                  }
               }
               if ( m > 0 && num / (double) m <= merge_chi2 ) { merge_idx = j; break; }
            }
            merge_q = q[ merge_idx ];
            editor_msg( "black",
                       QString( us_tr( "Reference merge point: q = %1 (index %2 of %3); extrapolating below, "
                                       "taking the reference curve above (reduced chi^2 <= %4)\n" ) )
                       .arg( merge_q ).arg( merge_idx ).arg( (int) npts ).arg( merge_chi2 ) );
            // A merge point at (or near) the start means the output is essentially the raw reference
            // curve rather than the extrapolation -- the de-noising and the c=0 correction are both
            // thrown away. That is almost never intended, so say so rather than returning it quietly.
            if ( merge_idx * 10 < (int) npts )
            {
               extrap_msg(
                          QString( us_tr( "Reference splice: the merge point is at index %1 of %2, so almost the whole "
                                          "output is the reference curve taken verbatim, not the extrapolation. This "
                                          "usually means the reference agrees with the extrapolation everywhere -- "
                                          "typically because it is not the highest concentration, or its error bars are "
                                          "large. Consider turning the splice off, or using the highest-concentration "
                                          "curve as the reference." ) )
                          .arg( merge_idx ).arg( (int) npts ) );
            }
         }
         else
         {
            merge_idx = (int) npts - win;
            for ( int j = 0; j + win <= (int) npts; j++ )
            {
               vector < double > wex, wrf;
               for ( int i = j; i < j + win; i++ )
               {
                  if ( grid_to_out[ i ] >= 0 )
                  {
                     wex.push_back( out_I0[ grid_to_out[ i ] ] * c_ref );
                     wrf.push_back( Iref_full[ i ] );
                  }
               }
               int lr;
               if ( (int) wex.size() >= 5 && US_Saxs_Util::compute_p_value( wex, wrf, lr ) >= pvalue_alpha )
               {
                  merge_idx = j; break;
               }
            }
            merge_q = q[ merge_idx ];
            editor_msg( "black",
                       QString( us_tr( "Reference merge point: q = %1 (index %2 of %3); no reference errors, "
                                       "CORMAP alpha = %4\n" ) )
                       .arg( merge_q ).arg( merge_idx ).arg( (int) npts ).arg( pvalue_alpha ) );
         }
      }

      double S_out  = ref_scale ? c_ref : 1e0;                               // extrapolation scale
      double refdiv = ref_scale ? 1e0 : ( c_ref > 0e0 ? 1e0 / c_ref : 1e0 ); // reference onto output scale

      // Continuity: the extrapolation (which carries the dilute-limit scale) and the reference
      // generally differ by a few percent at the merge point, so copying the reference verbatim
      // leaves a visible step. Scale the spliced reference by the median ratio of the normalized
      // extrapolation to the normalized reference over a window just below the merge, so the
      // reference contributes clean high-q *shape* stitched smoothly onto the extrapolation.
      double ref_cont = 1e0;
      if ( merge_ref && merge_idx > 0 && merge_idx < (int) npts )
      {
         vector < double > ratios;
         int w0 = merge_idx - 25; if ( w0 < 0 ) { w0 = 0; }
         for ( int i = w0; i < merge_idx; i++ )
         {
            if ( grid_to_out[ i ] >= 0 && i < (int) Iref_full.size() && Iref_full[ i ] != 0e0 )
            {
               double rn = Iref_full[ i ] / ( c_ref > 0e0 ? c_ref : 1e0 );   // reference normalized
               if ( rn != 0e0 ) { ratios.push_back( out_I0[ grid_to_out[ i ] ] / rn ); }
            }
         }
         if ( !ratios.empty() )
         {
            std::sort( ratios.begin(), ratios.end() );
            ref_cont = ratios[ ratios.size() / 2 ];                          // median
            editor_msg( "black",
                       QString( us_tr( "Reference splice continuity factor: %1 (reference scaled to join the "
                                       "extrapolation smoothly at the merge point).\n" ) ).arg( ref_cont, 0, 'f', 4 ) );
         }
      }

      vector < double > nq, nI, nE;
      for ( unsigned int qi = 0; qi < npts; qi++ )
      {
         if ( merge_ref && (int) qi >= merge_idx )
         {
            nq.push_back( q[ qi ] );
            nI.push_back( ( qi < Iref_full.size() ? Iref_full[ qi ] : 0e0 ) * refdiv * ref_cont );
            nE.push_back( ( qi < ref_sd.size() ? ref_sd[ qi ] : 0e0 ) * refdiv * ref_cont );
         }
         else if ( grid_to_out[ qi ] >= 0 )
         {
            int k = grid_to_out[ qi ];
            nq.push_back( out_q[ k ] );
            nI.push_back( out_I0[ k ] * S_out );
            nE.push_back( out_I0_err[ k ] * S_out );
         }
      }
      out_q = nq; out_I0 = nI; out_I0_err = nE;
   }

   // 6c. Guinier quality check of the extrapolated curve (ground-truth-free): a clean
   //     ln I vs q^2 line with a physically sensible Rg indicates a well-behaved low-q
   //     extrapolation. Reported as a QC signal (flagged if R^2 is low).
   {
      double g_Rg = 0e0, g_I0 = 0e0, g_r2 = 0e0;
      int    g_n  = 0;
      if ( us_extrap_c0_guinier( out_q, out_I0, g_Rg, g_I0, g_r2, g_n ) )
      {
         editor_msg( ( g_r2 >= 0.9 ) ? "black" : "dark red",
                    QString( us_tr( "Guinier QC of the extrapolated curve (rough automatic check -- use the "
                                    "Guinier panel for a definitive fit): Rg = %1 A, I(0) = %2, "
                                    "R^2 = %3 over the first %4 points (q*Rg <= 1.3)%5\n" ) )
                    .arg( g_Rg, 0, 'f', 1 ).arg( g_I0, 0, 'g', 4 ).arg( g_r2, 0, 'f', 4 ).arg( g_n )
                    .arg( ( g_r2 >= 0.9 )
                          ? QString( us_tr( " -- a clean, linear Guinier region." ) )
                          : QString( us_tr( " -- low R^2: check the low-q data/concentrations." ) ) ) );
      }
   }

   // 6d. optional post-fit reassessment of the output error bars (the s.d.util "SD rescale"
   //     engine, US_Saxs_Util::recompute_errors). Off by default: keep the propagated errors.
   //     When requested, reassess sigma from the extrapolated curve's own point-to-point
   //     scatter and rescale in place (Constant / Non-constant / Intensity-dependent). If the
   //     reference was spliced in above the merge q, the reassessment spans the whole spliced
   //     curve. sigma is only actually rescaled when the fit warrants it (see recompute_errors);
   //     the verdict and factor are reported here.
   if ( sd_mode >= 1 && sd_mode <= 3 )
   {
      char sd_char = ( sd_mode == 2 ) ? 'N' : ( sd_mode == 3 ) ? 'I' : 'C';
      QString sd_label = ( sd_mode == 2 ) ? us_tr( "non-constant (per q-bin)" )
                       : ( sd_mode == 3 ) ? us_tr( "intensity-dependent" )
                                          : us_tr( "constant" );
      QString           sd_errors;
      QString           sd_verdict;
      double            sd_pval  = 0e0;
      double            sd_chi2r = 0e0;
      vector < double > sd_factors;
      if ( US_Saxs_Util::recompute_errors( out_I0, out_q, out_I0_err, sd_errors, sd_char,
                                           10, 11, false, &sd_factors, 0, &sd_verdict, &sd_pval, &sd_chi2r ) )
      {
         // summarize the applied scale factor(s): a single value for constant mode, median +
         // range for the per-point (non-constant / intensity) modes -- matching the s.d.util
         // "SD rescale" log. (Constant's factor is sqrt(reduced chi^2), so it is not implied by
         // the chi^2 alone and is worth reporting explicitly.)
         double fmin = 1e0, fmed = 1e0, fmax = 1e0;
         {
            vector < double > fs = sd_factors;
            std::sort( fs.begin(), fs.end() );
            if ( fs.size() )
            {
               fmin = fs.front();
               fmax = fs.back();
               fmed = fs[ fs.size() / 2 ];
            }
         }
         QString factor_str =
            ( fmax - fmin < 1e-9 )
            ? QString( us_tr( "factor %1" ) ).arg( fmed, 0, 'g', 4 )
            : QString( us_tr( "factor median %1 (%2..%3)" ) ).arg( fmed, 0, 'g', 4 ).arg( fmin, 0, 'g', 4 ).arg( fmax, 0, 'g', 4 );
         editor_msg( "black",
                    QString( us_tr( "Output SD reassessment (%1): %2, %3 (reduced chi^2 = %4, p = %5).\n" ) )
                    .arg( sd_label ).arg( sd_verdict ).arg( factor_str )
                    .arg( sd_chi2r, 0, 'f', 3 ).arg( sd_pval, 0, 'f', 3 ) );
      }
      else
      {
         editor_msg( "dark red",
                    QString( us_tr( "Output SD reassessment (%1) skipped: %2\n" ) )
                    .arg( sd_label ).arg( sd_errors ) );
      }
   }

   // 7. name the new curve, avoiding collisions with already-plotted curves

   QString base_name  = us_extrap_c0_curve_name( ordered_names, extrap_model, ref_scale, merge_ref, merge_q, sd_mode, n_outlier_dropped, !use_sd_weights,
                                                  recompute_inputs ? ( recompute_inputs_mode + 1 ) : 0,
                                                  !use_gcv, fit_broaden, ref_override_conc_used,
                                                  !outlier_leverage );
   QString final_name = base_name;
   {
      int suffix = 1;
      while ( qsl_plotted_iq_names.contains( final_name ) )
      {
         final_name = QString( "%1-%2" ).arg( base_name ).arg( suffix++ );
      }
   }

   // 8. plot the extrapolated curve

   plot_one_iqq( out_q, out_I0, out_I0_err, final_name );

   // Tag the output: reference (absolute) scale -> tag with the reference concentration so
   // dividing by it recovers the normalized form; otherwise Conc:1. Already-I*(q) inputs are
   // always Conc:1.
   double out_conc_tag = ( ref_scale && !all_istar ) ? ref_conc : 1e0;
   update_conc_csv( final_name, out_conc_tag, common_psv, common_i0se );

   // scripted run: also write the extrapolated curve to the requested output path, in the same
   // 3-column q/I(q)/sd .dat format as the inputs (with a Conc: header) so it round-trips and
   // can be diffed against a regression baseline.
   if ( extrap_c0_script && !extrap_c0_script_out.isEmpty() )
   {
      QFile of( extrap_c0_script_out );
      if ( !of.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         editor_msg( "red", QString( us_tr( "saxs_extrap_c0: cannot write output file '%1'\n" ) ).arg( extrap_c0_script_out ) );
      }
      else
      {
         QTextStream os( &of );
         // carry psv/i0se forward in the header (same tokens/format as HPLC/MALS/DAD) so the
         // extrapolated curve round-trips; only when an explicit value was actually present.
         QString hdr_extra;
         if ( have_psv )  { hdr_extra += QString( " PSV:%1 [mL/g]" ).arg( common_psv,  0, 'g', 8 ); }
         if ( have_i0se ) { hdr_extra += QString( " I0se:%1 [a.u.]" ).arg( common_i0se, 0, 'g', 8 ); }
         os << QString( "US-SOMO: extrapolate-to-zero-concentration: %1 q units:1/A Conc:%2 [mg/mL]%3\n" )
            .arg( final_name ).arg( out_conc_tag, 0, 'g', 8 ).arg( hdr_extra );
         os << "q\tI(q)\tsd\n";
         for ( unsigned int oi = 0; oi < (unsigned int) out_q.size(); oi++ )
         {
            double oe = ( oi < out_I0_err.size() ) ? out_I0_err[ oi ] : 0e0;
            os << QString( "%1\t%2\t%3\n" )
               .arg( out_q[ oi ], 0, 'e', 6 )
               .arg( out_I0[ oi ], 0, 'e', 6 )
               .arg( oe, 0, 'e', 6 );
         }
         of.close();
         editor_msg( "black", QString( us_tr( "saxs_extrap_c0: wrote %1 points to %2\n" ) )
                     .arg( out_q.size() ).arg( extrap_c0_script_out ) );
      }
   }

   QString fit_name   = ( extrap_model == 2 ) ? us_tr( "2nd-order virial (c/I)" )
                      : ( extrap_model == 1 ) ? us_tr( "reciprocal (c/I)" )
                                              : us_tr( "additive (I/c)" );
   QString scale_note = ref_scale
      ? QString( us_tr( "output on the reference absolute scale (conc %1)" ) ).arg( ref_conc )
      : QString( us_tr( "normalized (tagged Conc:1)" ) );
   QString merge_note = merge_ref
      ? QString( us_tr( "; reference curve spliced above q = %1" ) ).arg( merge_q )
      : QString( us_tr( "; no high-q reference splice" ) );
   editor_msg( "black",
              QString( us_tr( "Added zero-concentration curve \"%1\" (%2 q-points, %3 skipped). "
                              "Fit: %4; %5%6.%7\n" ) )
              .arg( final_name ).arg( out_q.size() ).arg( skipped_points )
              .arg( fit_name ).arg( scale_note ).arg( merge_note )
              .arg( all_istar ? QString( us_tr( " Inputs were I*(q) (I(0)=MW)." ) ) : QString( "" ) ) );

   // 9. optional per-q regression viewer (scrollable pop-up requested from the dialog)

   if ( show_regplots && reg_q.size() )
   {
      // the fit shown is always the normalized concentration-model fit (reciprocal -> c/I);
      // the reference scale/splice are output conventions applied after the fit
      QString y_axis_title =
         reciprocal ? us_tr( "concentration / I(q)   (c/I)" )
                    : ( all_istar ? us_tr( "I*(q)  (normalized)" )
                                  : us_tr( "I(q)/concentration" ) );

      US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot *regplot =
         new US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot(
                                                    us_hydrodyn,
                                                    y_axis_title,
                                                    merge_ref ? merge_q : 0e0,       // merge point q (0 => none)
                                                    use_ref_path ? 0 : fit_broaden,  // Zimm fit-broadening q-window
                                                    gcv_edof_used,                 // GCV effective slope dof (0 => off)
                                                    extrap_model,                  // 0 additive, 1 reciprocal, 2 virial
                                                    reg_q, reg_x, reg_y, reg_e,
                                                    reg_a, reg_b, reg_c, reg_siga,
                                                    excl_conc,                     // QC-excluded curve conc (<0 => none)
                                                    reg_excl_y,                    // its per-q y on the plot axis
                                                    QString( excl_name ).remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) ),
                                                    this
                                                    );
      regplot->setAttribute( Qt::WA_DeleteOnClose );
      regplot->show();
   }
}
