#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_saxs_iqq_extrap_c0_conc.h"
#include "../include/us_hydrodyn_saxs_iqq_extrap_c0_regplot.h"
#include <QRegularExpression>
#include <QFileInfo>
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

static QString us_extrap_c0_curve_name( const QStringList &names, bool absolute_mode )
{
   // method token so Zimm vs absolute-scale outputs are distinguishable on a plot and on
   // disk
   QString method = absolute_mode ? "_absolute" : "_zimm";

   QString prefix = us_extrap_c0_common_prefix( names ).trimmed();
   prefix.remove( QRegularExpression( "[\\s_-]+$" ) );

   if ( prefix.isEmpty() )
   {
      return "extrap_c0" + method;
   }
   return prefix + "_extrap_c0" + method;
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

   int    k         = ( n < 25 ) ? n : 25;      // initial low-q window
   double slope     = 0e0;
   double intercept = 0e0;

   for ( int pass = 0; pass < 2; pass++ )
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

      // refine the window once so q_max*Rg <= ~1.3 (the valid Guinier range)
      if ( pass == 0 && Rg > 0e0 )
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
         k = knew;
      }
      else
      {
         break;
      }
   }
   return ( Rg > 0e0 );
}

void US_Hydrodyn_Saxs::do_extrap_c0(
                                    QStringList qsl_sel_names,
                                    QStringList qsl_data,
                                    map < QString, QString > name_to_errors_map,
                                    vector < double > q,
                                    QString filename
                                    )
{
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
      QMessageBox::warning( this, "UltraScan",
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
   }

   map < QString, double > name_to_conc;
   QStringList selected_names;
   bool dlg_ok        = false;
   bool absolute_mode   = false;
   bool show_regplots = false;
   int  fit_broaden   = 0;
   bool use_gcv       = true;   // automatic GCV slope regularization (recommended default)
   {
      US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc dlg( ordered_names, prepop_conc, &name_to_conc, &selected_names, &dlg_ok, &absolute_mode, &show_regplots, &fit_broaden, &use_gcv, us_hydrodyn, this );
      US_Hydrodyn::fixWinButtons( &dlg );
      dlg.exec();
   }

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
      QMessageBox::warning( this, "UltraScan",
                            us_tr( "Fewer than 3 curves were selected; "
                                  "cannot extrapolate to zero concentration." ) );
      return;
   }

   // 4. persist entered concentrations back into the existing conc_csv facility,
   //    keyed by the dequoted (canonical, unquoted) curve name -- matches the
   //    convention used by other writers such as US_Hydrodyn_Saxs_Hplc::to_saxs()

   for ( QStringList::iterator it = ordered_names.begin(); it != ordered_names.end(); it++ )
   {
      QString dequoted_name = *it;
      dequoted_name.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );
      update_conc_csv( dequoted_name, name_to_conc[ *it ] );
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
      QMessageBox::warning( this, "UltraScan",
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
      QMessageBox::warning( this, "UltraScan",
                            QString( us_tr( "The selection mixes %1 I*(q) (already concentration-normalized, "
                                            "I(0)=MW) curve(s) with %2 raw-intensity curve(s). These are on "
                                            "different scales; the extrapolation may not be meaningful. "
                                            "Proceeding anyway." ) )
                            .arg( istar_count ).arg( ordered_names.size() - (int) istar_count ) );
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
      QMessageBox::warning( this, "UltraScan",
                            QString( us_tr( "%1 curve(s) have a concentration of 0 and will be excluded "
                                            "from the extrapolation (intensity can't be normalized by a zero concentration)." ) )
                            .arg( zero_conc_excluded ) );
   }

   // Absolute-scale setup: pick the reference (highest-concentration) curve, compute a
   // least-squares scale factor for every curve onto that reference over the common
   // q-grid, and grab the reference curve's error column to carry into the output.

   int    ref_ci   = -1;
   double ref_conc = 0e0;
   vector < double > scale( ordered_names.size(), 1e0 );
   vector < double > ref_sd;
   vector < double > Iref_full;                 // reference (highest-conc) raw I(q), full grid
   vector < double > Iex( npts, 0e0 );          // per-q extrapolated intercept (reference scale)
   vector < double > Islope( npts, 0e0 );       // per-q fit slope (for the regression viewer)
   vector < double > Iex_se( npts, 0e0 );       // per-q intercept standard error (extrapolated region)
   int    merge_idx = 0;                        // grid index: output = reference from here up
   double merge_q   = 0e0;                      // q at the merge point (0 => no merge / all reference)
   double pvalue_alpha = 1e-2;                  // CORMAP merge-point significance (cf. cormap_alpha)

   // GCV effective slope dof actually used (0 => GCV not applied); surfaced to the
   // regression viewer so it can annotate that automatic regularization was in effect
   double gcv_edof_used = 0e0;

   if ( absolute_mode )
   {
      {
         US_Hydrodyn *uh = (US_Hydrodyn *) us_hydrodyn;
         if ( uh->gparams.count( "saxs_extrap_c0_pvalue_alpha" ) )
         {
            pvalue_alpha = uh->gparams[ "saxs_extrap_c0_pvalue_alpha" ].toDouble();
         }
      }

      for ( int ci = 0; ci < ordered_names.size(); ci++ )
      {
         if ( concs[ ci ] > ref_conc )
         {
            ref_conc = concs[ ci ];
            ref_ci   = ci;
         }
      }

      if ( ref_ci < 0 )
      {
         QMessageBox::critical( this, "UltraScan",
                                us_tr( "Absolute-scale mode: no curve has a positive concentration to use as the "
                                       "scaling reference; aborting." ) );
         return;
      }

      const vector < double > &Iref = name_to_I[ ordered_names[ ref_ci ] ];

      // Scale each curve to the reference concentration by c_ref/c_i (equivalent to
      // normalizing by concentration, I/c, then re-expressing on the reference's
      // absolute scale). A data-driven least-squares scale was tried but is corrupted
      // at low q by the structure factor for strongly-interacting samples -- it made
      // the extrapolation recover only ~85% of the true dilute limit on a simulated
      // strong-dependence series, vs ~99% with the concentration scale used here.
      QString scale_report;
      for ( int ci = 0; ci < ordered_names.size(); ci++ )
      {
         if ( concs[ ci ] <= 0e0 )
         {
            continue;
         }
         scale[ ci ] = ref_conc / concs[ ci ];
         scale_report += QString( "    %1  conc %2  scale %3\n" )
            .arg( ordered_names[ ci ] ).arg( concs[ ci ] ).arg( scale[ ci ] );
      }
      editor_msg( "black",
                 QString( us_tr( "Absolute-scale extrapolation: reference (highest conc) is \"%1\" (conc %2); "
                                 "curves scaled to it by c_ref/c_i\n%3" ) )
                 .arg( ordered_names[ ref_ci ] ).arg( ref_conc ).arg( scale_report ) );

      // the extrapolated curve inherits the reference curve's error bars (matches almerge)
      if ( name_to_errors_map.count( ordered_names[ ref_ci ] ) )
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

      // pointwise weighted linear fit of the scaled intensity vs concentration
      // (Petoukhov et al. 2012, Eq 1): intercept = infinite-dilution intensity I_ex.
      for ( unsigned int qi = 0; qi < npts; qi++ )
      {
         double S = 0e0, Sx = 0e0, Sy = 0e0, Sxx = 0e0, Sxy = 0e0;
         int    cnt = 0;
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
            double sig = scale[ ci ] * sd;
            double w   = ( sig > 0e0 && !us_isnan( sig ) ) ? 1e0 / ( sig * sig ) : 1e0;
            double xv  = concs[ ci ];
            double yv  = scale[ ci ] * Iv;
            S += w; Sx += w * xv; Sy += w * yv; Sxx += w * xv * xv; Sxy += w * xv * yv; cnt++;
         }
         double d = S * Sxx - Sx * Sx;
         if ( cnt >= 2 && d != 0e0 )
         {
            Iex[ qi ]    = ( Sxx * Sy - Sx * Sxy ) / d;
            Islope[ qi ] = ( S * Sxy - Sx * Sy ) / d;
            // weighted-fit intercept standard error, Var(a) = Sxx/d (used below the merge
            // point where the output is the extrapolated intercept). With GCV the slope is
            // regularized so the true intercept uncertainty is smaller -- this is a
            // conservative (upper-bound) error bar for the extrapolated low-q region.
            Iex_se[ qi ] = ( Sxx / d > 0e0 ) ? std::sqrt( Sxx / d ) : 0e0;
         }
         else
         {
            Iex[ qi ]    = Iref_full[ qi ];
            Islope[ qi ] = 0e0;
            Iex_se[ qi ] = 0e0;
         }
      }

      // GCV-penalized slope regularization: replace the noisy per-q intercepts with a
      // globally slope-smoothed fit (auto-tuned by GCV) so the merge test below and the
      // low-q output use the cleaner curve. Overwrites Iex/Islope; on any degeneracy the
      // helper returns false and the per-q OLS values above are kept.
      if ( use_gcv )
      {
         vector < double > vM( npts ), vR( npts ), vB( npts ), vC( npts ), vQ( npts );
         for ( unsigned int qi = 0; qi < npts; qi++ )
         {
            double S = 0e0, Sx = 0e0, Sy = 0e0, Sxx = 0e0, Sxy = 0e0;
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
               double sig = scale[ ci ] * sd;
               double w   = ( sig > 0e0 && !us_isnan( sig ) ) ? 1e0 / ( sig * sig ) : 1e0;
               double xv  = concs[ ci ];
               double yv  = scale[ ci ] * Iv;
               S += w; Sx += w * xv; Sy += w * yv; Sxx += w * xv * xv; Sxy += w * xv * yv;
            }
            vC[ qi ] = S; vB[ qi ] = Sx; vQ[ qi ] = Sy;
            vM[ qi ] = ( S > 0e0 ) ? ( Sxx - Sx * Sx / S ) : 0e0;
            vR[ qi ] = ( S > 0e0 ) ? ( Sxy - Sx * Sy / S ) : 0e0;
         }
         vector < double > Iex_pen, alpha_pen;
         double gcv_lambda = 0e0, gcv_edof = 0e0;
         if ( us_extrap_c0_gcv_penalized( vM, vR, vB, vC, vQ, Iex_pen, alpha_pen, gcv_lambda, gcv_edof ) )
         {
            for ( unsigned int qi = 0; qi < npts; qi++ )
            {
               Iex[ qi ]    = Iex_pen[ qi ];
               Islope[ qi ] = alpha_pen[ qi ];
            }
            gcv_edof_used = gcv_edof;
            editor_msg( "black",
                       QString( us_tr( "Absolute-scale GCV slope regularization applied: lambda = %1, effective slope dof = %2 of %3\n" ) )
                       .arg( gcv_lambda ).arg( gcv_edof, 0, 'f', 1 ).arg( (int) npts ) );
         }
      }

      // Merging point (ATSAS almerge idea): below it the extrapolated (structure-factor-
      // removed) intercept is used; at/above it the reference curve is copied verbatim.
      // It is located by scanning from low q for the first trailing window where the
      // extrapolated curve agrees with the reference to within the reference's OWN errors
      // -- a reduced chi-square of (Iex - Iref)/sd_ref <= threshold (default 4, i.e. ~2
      // sigma agreement; gparam saxs_extrap_c0_merge_chi2). This error-band test is robust
      // to GCV smoothing: a sign-run (CORMAP) test compares the smoothed Iex against the
      // noisy reference and is biased toward large merge points, because a small consistent
      // offset (e.g. the residual bias of a nearly-linear regularized slope at high q)
      // produces long same-sign runs even when the curves agree to within noise. CORMAP is
      // kept as a fallback when the reference curve has no error column.
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

         // trailing-window size; shrink it for short grids so absolute-scale still extrapolates
         // (a fixed 50-pt window that only ran when npts>50 left merge_idx=0 for shorter
         // curves, silently returning the input with no extrapolation and no warning)
         int win = 50;
         if ( (int) npts <= win )
         {
            win = (int) npts / 2;
         }

         if ( win < 5 )
         {
            // too few q-points to locate a merging point: extrapolate the whole curve
            // (merge_idx == npts => no high-q reference copy), merge_q = 0 signals "no
            // merge point" to the summary and the regression viewer
            merge_idx = (int) npts;
            merge_q   = 0e0;
            editor_msg( "dark red",
                       QString( us_tr( "Absolute-scale mode: only %1 q-point(s) -- too few to locate a merging point; "
                                       "extrapolating the whole curve (no high-q reference copy).\n" ) )
                       .arg( (int) npts ) );
         }
         else if ( have_ref_err )
         {
            merge_idx = (int) npts - win;      // fallback: extrapolate all but the last window
            for ( int j = 0; j + win <= (int) npts; j++ )
            {
               double num = 0e0;
               int    m   = 0;
               for ( int i = j; i < j + win; i++ )
               {
                  if ( i < (int) ref_sd.size() && ref_sd[ i ] > 0e0 )
                  {
                     double r = ( Iex[ i ] - Iref_full[ i ] ) / ref_sd[ i ];
                     num += r * r;
                     m++;
                  }
               }
               if ( m > 0 && num / (double) m <= merge_chi2 )
               {
                  merge_idx = j;
                  break;
               }
            }
            merge_q = q[ merge_idx ];
            editor_msg( "black",
                       QString( us_tr( "Absolute-scale merging point: q = %1 (index %2 of %3); extrapolating below, "
                                       "taking the highest-concentration curve above (error band: reduced chi^2 of "
                                       "(Iex-Iref)/sd_ref <= %4)\n" ) )
                       .arg( merge_q ).arg( merge_idx ).arg( (int) npts ).arg( merge_chi2 ) );
         }
         else
         {
            // no reference error column: fall back to the CORMAP longest-run sign test
            merge_idx = (int) npts - win;
            for ( int j = 0; j + win <= (int) npts; j++ )
            {
               vector < double > wex( Iex.begin() + j, Iex.begin() + j + win );
               vector < double > wrf( Iref_full.begin() + j, Iref_full.begin() + j + win );
               int lr;
               if ( US_Saxs_Util::compute_p_value( wex, wrf, lr ) >= pvalue_alpha )
               {
                  merge_idx = j;
                  break;
               }
            }
            merge_q = q[ merge_idx ];
            editor_msg( "black",
                       QString( us_tr( "Absolute-scale merging point: q = %1 (index %2 of %3); extrapolating below, "
                                       "taking the highest-concentration curve above (no reference errors; CORMAP "
                                       "alpha = %4)\n" ) )
                       .arg( merge_q ).arg( merge_idx ).arg( (int) npts ).arg( pvalue_alpha ) );
         }
      }
   }

   // 6. per-q linear regression -> intercept (I0) + error
   //    Zimm mode:   fit I(q)/c            vs concentration; error = regression SE
   //    absolute-scale mode: fit scale*I(q) (abs)  vs concentration; error = reference curve sd

   US_Saxs_Util usu;

   vector < double > out_q;
   vector < double > out_I0;
   vector < double > out_I0_err;
   unsigned int       skipped_points = 0;

   // per-q regression data captured for the optional scrollable regression-plot pop-up:
   // the (concentration, y, y-error) points, plus the fit intercept, slope and its SE
   vector < double >               reg_q;
   vector < vector < double > >    reg_x;
   vector < vector < double > >    reg_y;
   vector < vector < double > >    reg_e;
   vector < double >               reg_a;
   vector < double >               reg_b;
   vector < double >               reg_siga;

   // per-output-point concentration centroids, used only for Zimm fit-broadening:
   // for an OLS line a = ybar - b*xbar, so after smoothing the slope the intercept
   // is recomputed as ybar - b_smoothed*xbar (parallel to out_q)
   vector < double >               out_xbar;
   vector < double >               out_ybar;

   // Zimm GCV-penalized fit: per-q weighted stats over the whole grid -> a globally
   // slope-regularized intercept Iex(q) (and slope) at every q, lambda auto-tuned by GCV.
   // Used in the main loop below when GCV is enabled (absolute-scale does its own GCV above).
   vector < double > zimm_Iex, zimm_alpha;
   bool   zimm_gcv_ok = false;
   if ( use_gcv && !absolute_mode )
   {
      vector < double > vM( npts ), vR( npts ), vB( npts ), vC( npts ), vQ( npts );
      for ( unsigned int qi = 0; qi < npts; qi++ )
      {
         double S = 0e0, Sx = 0e0, Sy = 0e0, Sxx = 0e0, Sxy = 0e0;
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
            // Zimm axis: y = I/c (raw) or I as-is (already-normalized I*(q)); weight 1/sig^2
            double yv, sig;
            if ( is_istar[ ci ] ) { yv = Iv;              sig = sd; }
            else                  { yv = Iv / concs[ ci ]; sig = sd / concs[ ci ]; }
            double w  = ( sig > 0e0 && !us_isnan( sig ) ) ? 1e0 / ( sig * sig ) : 1e0;
            double xv = concs[ ci ];
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
         if ( absolute_mode )
         {
            yv  = scale[ ci ] * Iv;
            yev = scale[ ci ] * sd;
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

      double a = 0e0, b = 0e0, siga = 0e0;

      if ( absolute_mode )
      {
         // per-q extrapolation (Eq 1 intercept) + slope were precomputed above for
         // every q (falling back to the reference where a fit was impossible), and
         // above the merge point the output is simply the reference curve, so absolute-scale
         // does not skip a q for having < 2 valid points. reg_a/reg_b show the fit.
         a    = Iex[ qi ];
         b    = Islope[ qi ];
         siga = 0e0;
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
            usu.linear_fit( x, y, aa, bb, siga, sigb, chi2 );
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
         skipped_points++;
         continue;
      }
      else if ( x.size() == 2 )
      {
         if ( x[ 1 ] == x[ 0 ] )
         {
            TSO << QString( "WARNING: do_extrap_c0: duplicate concentrations with only 2 points at q[%1]=%2, skipping\n" )
               .arg( qi ).arg( q[ qi ] );
            skipped_points++;
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
         usu.linear_fit( x, y, a, b, siga, sigb, chi2 );
      }

      // Error bars: Zimm reports the regression SE of the intercept. absolute-scale reports the
      // reference curve's error at/above the merge point (where the output IS the
      // reference), but the propagated intercept SE below the merge point (the
      // extrapolated region), so the low-q extrapolation carries its own uncertainty
      // rather than borrowing the reference's.
      double err_val = siga;
      if ( absolute_mode )
      {
         if ( (int) qi >= merge_idx && qi < ref_sd.size() )
         {
            err_val = ref_sd[ qi ];
         }
         else if ( qi < Iex_se.size() )
         {
            err_val = Iex_se[ qi ];
         }
      }

      // absolute-scale hybrid output: extrapolation below the merge point, the reference
      // (highest-concentration) curve verbatim at/above it. Zimm: the intercept.
      double out_val = a;
      if ( absolute_mode && (int) qi >= merge_idx && qi < Iref_full.size() )
      {
         out_val = Iref_full[ qi ];
      }

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

      reg_q   .push_back( q[ qi ] );
      reg_x   .push_back( x );
      reg_y   .push_back( y );
      reg_e   .push_back( ye );
      reg_a   .push_back( a );                 // the per-q fit intercept (diagnostic)
      reg_b   .push_back( b );
      reg_siga.push_back( err_val );
   }

   if ( skipped_points )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString( us_tr( "%1 of %2 q-points could not be extrapolated "
                                            "(fewer than 2 valid data points) and were skipped." ) )
                            .arg( skipped_points ).arg( npts ) );
   }

   if ( out_q.empty() )
   {
      QMessageBox::critical( this, "UltraScan", us_tr( "No q-points could be extrapolated; aborting." ) );
      return;
   }

   // 6b. Zimm fit broadening: the interparticle/interaction slope varies smoothly with
   //     q (and dies off at high q), so smoothing the per-q concentration slope across
   //     a q-window and recomputing the intercept (a = ybar - b_smooth*xbar) removes
   //     extrapolation noise without smearing the form-factor detail carried by the
   //     intercept. Off by default (window <= 1); absolute-scale mode is unaffected. Superseded
   //     by GCV regularization when that is enabled (the two are mutually exclusive).
   if ( !absolute_mode && !zimm_gcv_ok && fit_broaden > 1 )
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

   // 6c. Guinier quality check of the extrapolated curve (ground-truth-free): a clean
   //     ln I vs q^2 line with a physically sensible Rg indicates a well-behaved low-q
   //     extrapolation. Reported as a QC signal (flagged if R^2 is low).
   {
      double g_Rg = 0e0, g_I0 = 0e0, g_r2 = 0e0;
      int    g_n  = 0;
      if ( us_extrap_c0_guinier( out_q, out_I0, g_Rg, g_I0, g_r2, g_n ) )
      {
         editor_msg( ( g_r2 >= 0.9 ) ? "black" : "dark red",
                    QString( us_tr( "Guinier QC of the extrapolated curve: Rg = %1 A, I(0) = %2, "
                                    "R^2 = %3 over the first %4 points (q*Rg <= 1.3)%5\n" ) )
                    .arg( g_Rg, 0, 'f', 1 ).arg( g_I0, 0, 'g', 4 ).arg( g_r2, 0, 'f', 4 ).arg( g_n )
                    .arg( ( g_r2 >= 0.9 )
                          ? QString( us_tr( " -- a clean, linear Guinier region." ) )
                          : QString( us_tr( " -- low R^2: check the low-q data/concentrations." ) ) ) );
      }
   }

   // 7. name the new curve, avoiding collisions with already-plotted curves

   QString base_name  = us_extrap_c0_curve_name( ordered_names, absolute_mode );
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

   if ( absolute_mode )
   {
      // absolute intensity on the reference curve's scale -- tag it with the reference
      // concentration so dividing by it recovers the normalized form. Exception: if
      // every input was already I*(q) (I(0)=MW), the output is likewise already
      // normalized, so tag it Conc:1 rather than the reference concentration.
      double out_conc_tag = all_istar ? 1e0 : ref_conc;
      update_conc_csv( final_name, out_conc_tag );

      editor_msg( "black",
                 QString( "Added zero-concentration curve \"%1\" (%2 q-points, %3 skipped)\n"
                          "Absolute-scale mode: low-q extrapolated to c=0 on the scale of "
                          "the highest-concentration curve (conc %4); at/above the merging point q=%5 the "
                          "reference curve is taken verbatim, carrying its error bars.%6\n" )
                 .arg( final_name ).arg( out_q.size() ).arg( skipped_points )
                 .arg( ref_conc ).arg( merge_q )
                 .arg( all_istar ? QString( " Inputs were I*(q) (I(0)=MW), so the result is tagged Conc:1." ) : QString( "" ) ) );
   }
   else
   {
      // this curve is I(q)/c extrapolated to c=0, i.e. already concentration-normalized
      // -- mark it with SOMO's "Conc:1" convention for already-normalized curves so
      // anything downstream that respects conc_csv treats it correctly
      update_conc_csv( final_name, 1e0 );

      editor_msg( "black",
                 QString( "Added zero-concentration extrapolation curve \"%1\" (%2 q-points, %3 skipped)\n"
                          "Zimm mode: concentration-normalized intensity extrapolated to c=0, tagged Conc:1.%4\n" )
                 .arg( final_name ).arg( out_q.size() ).arg( skipped_points )
                 .arg( all_istar
                       ? QString( " Inputs were already I*(q) (I(0)=MW); used as-is (not re-divided by concentration)." )
                       : QString( " Its scale differs from raw input curves by a factor of concentration." ) ) );
   }

   // 9. optional per-q regression viewer (scrollable pop-up requested from the dialog)

   if ( show_regplots && reg_q.size() )
   {
      QString y_axis_title =
         absolute_mode ? us_tr( "I(q) scaled to reference concentration" )
                     : ( all_istar ? us_tr( "I*(q)  (normalized)" )
                                   : us_tr( "I(q)/concentration" ) );

      US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot *regplot =
         new US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot(
                                                    us_hydrodyn,
                                                    y_axis_title,
                                                    absolute_mode ? merge_q : 0e0,   // merge point q (0 => none)
                                                    absolute_mode ? 0 : fit_broaden, // Zimm fit-broadening q-window
                                                    gcv_edof_used,                 // GCV effective slope dof (0 => off)
                                                    reg_q, reg_x, reg_y, reg_e,
                                                    reg_a, reg_b, reg_siga,
                                                    this
                                                    );
      regplot->setAttribute( Qt::WA_DeleteOnClose );
      regplot->show();
   }
}
