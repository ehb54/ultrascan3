#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_saxs_iqq_extrap_c0_conc.h"
#include "../include/us_hydrodyn_saxs_iqq_extrap_c0_regplot.h"
#include <QRegularExpression>
#include <QFileInfo>
#include <QMessageBox>
#include <set>

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

static QString us_extrap_c0_curve_name( const QStringList &names, bool primus_mode )
{
   // method token so Zimm vs Primus outputs are distinguishable on a plot and on
   // disk (the "zimm"/"primus" wording may change later)
   QString method = primus_mode ? "_primus" : "_zimm";

   QString prefix = us_extrap_c0_common_prefix( names ).trimmed();
   prefix.remove( QRegularExpression( "[\\s_-]+$" ) );

   if ( prefix.isEmpty() )
   {
      return "extrap_c0" + method;
   }
   return prefix + "_extrap_c0" + method;
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
   bool primus_mode   = false;
   bool show_regplots = false;
   int  fit_broaden   = 0;
   {
      US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc dlg( ordered_names, prepop_conc, &name_to_conc, &selected_names, &dlg_ok, &primus_mode, &show_regplots, &fit_broaden, us_hydrodyn, this );
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
   // therefore also yields an already-normalized (I(0)=MW) output. Primus mode never
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
   // Primus mode: reproduces ATSAS almerge (Petoukhov et al. 2012 Eq 1; Franke et al.
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

   // Primus-mode setup: pick the reference (highest-concentration) curve, compute a
   // least-squares scale factor for every curve onto that reference over the common
   // q-grid, and grab the reference curve's error column to carry into the output.

   int    ref_ci   = -1;
   double ref_conc = 0e0;
   vector < double > scale( ordered_names.size(), 1e0 );
   vector < double > ref_sd;
   vector < double > Iref_full;                 // reference (highest-conc) raw I(q), full grid
   vector < double > Iex( npts, 0e0 );          // per-q extrapolated intercept (reference scale)
   vector < double > Islope( npts, 0e0 );       // per-q fit slope (for the regression viewer)
   int    merge_idx = 0;                        // grid index: output = reference from here up
   double merge_q   = 0e0;                      // q at the merge point (0 => no merge / all reference)
   double pvalue_alpha = 1e-2;                  // CORMAP merge-point significance (cf. cormap_alpha)

   if ( primus_mode )
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
                                us_tr( "Primus mode: no curve has a positive concentration to use as the "
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
                 QString( us_tr( "Primus-mode extrapolation: reference (highest conc) is \"%1\" (conc %2); "
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
         }
         else
         {
            Iex[ qi ]    = Iref_full[ qi ];
            Islope[ qi ] = 0e0;
         }
      }

      // merging point (ATSAS almerge): extrapolate the low-q region where the
      // concentration effect is real, and take the reference curve verbatim at high
      // q where the profiles differ only by noise. Detect the crossover with a local
      // CORMAP longest-run p-value between the extrapolated curve and the reference:
      // scan from low q, and the merge point is the first q where a trailing window
      // agrees (p >= alpha). Below it: extrapolation; at/above it: reference.
      {
         const int win = 50;
         merge_idx = 0;
         if ( (int) npts > win )
         {
            merge_idx = (int) npts - win;      // fallback: extrapolate all but the last window
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
         }
         merge_q = ( merge_idx < (int) npts ) ? q[ merge_idx ] : q[ npts - 1 ];
         editor_msg( "black",
                    QString( us_tr( "Primus-mode merging point: q = %1 (index %2 of %3); extrapolating below, "
                                    "taking the highest-concentration curve above (CORMAP alpha = %4)\n" ) )
                    .arg( merge_q ).arg( merge_idx ).arg( (int) npts ).arg( pvalue_alpha ) );
      }
   }

   // 6. per-q linear regression -> intercept (I0) + error
   //    Zimm mode:   fit I(q)/c            vs concentration; error = regression SE
   //    Primus mode: fit scale*I(q) (abs)  vs concentration; error = reference curve sd

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
         if ( primus_mode )
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

      if ( primus_mode )
      {
         // per-q extrapolation (Eq 1 intercept) + slope were precomputed above for
         // every q (falling back to the reference where a fit was impossible), and
         // above the merge point the output is simply the reference curve, so Primus
         // does not skip a q for having < 2 valid points. reg_a/reg_b show the fit.
         a    = Iex[ qi ];
         b    = Islope[ qi ];
         siga = 0e0;
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

      // Primus mode carries the reference curve's error at this q; Zimm mode reports
      // the regression standard error of the intercept
      double err_val = siga;
      if ( primus_mode && qi < ref_sd.size() )
      {
         err_val = ref_sd[ qi ];
      }

      // Primus hybrid output: extrapolation below the merge point, the reference
      // (highest-concentration) curve verbatim at/above it. Zimm: the intercept.
      double out_val = a;
      if ( primus_mode && (int) qi >= merge_idx && qi < Iref_full.size() )
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
   //     intercept. Off by default (window <= 1); Primus mode is unaffected.
   if ( !primus_mode && fit_broaden > 1 )
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

   // 7. name the new curve, avoiding collisions with already-plotted curves

   QString base_name  = us_extrap_c0_curve_name( ordered_names, primus_mode );
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

   if ( primus_mode )
   {
      // absolute intensity on the reference curve's scale -- tag it with the reference
      // concentration so dividing by it recovers the normalized form. Exception: if
      // every input was already I*(q) (I(0)=MW), the output is likewise already
      // normalized, so tag it Conc:1 rather than the reference concentration.
      double out_conc_tag = all_istar ? 1e0 : ref_conc;
      update_conc_csv( final_name, out_conc_tag );

      editor_msg( "black",
                 QString( "Added zero-concentration curve \"%1\" (%2 q-points, %3 skipped)\n"
                          "Primus mode (ATSAS almerge-style): low-q extrapolated to c=0 on the scale of "
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
         primus_mode ? us_tr( "I(q) scaled to reference concentration" )
                     : ( all_istar ? us_tr( "I*(q)  (normalized)" )
                                   : us_tr( "I(q)/concentration" ) );

      US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot *regplot =
         new US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot(
                                                    us_hydrodyn,
                                                    y_axis_title,
                                                    primus_mode ? merge_q : 0e0,   // merge point q (0 => none)
                                                    primus_mode ? 0 : fit_broaden, // Zimm fit-broadening q-window
                                                    reg_q, reg_x, reg_y, reg_e,
                                                    reg_a, reg_b, reg_siga,
                                                    this
                                                    );
      regplot->setAttribute( Qt::WA_DeleteOnClose );
      regplot->show();
   }
}
