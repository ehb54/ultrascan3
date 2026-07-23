#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include <QRegularExpression>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTextStream>

#define TSO QTextStream(stdout)

// Non-interactive driver for "extrapolate to zero concentration", exposed to the
// us3_somo -g gui_script interpreter as:  saxs_extrap_c0 <controlfile>
//
// The control file is a small key/value text file (one directive per line, '#' comments).
// It reproduces every choice a user makes across the two interactive dialogs so that a
// headless run computes exactly what the GUI would, and writes the extrapolated curve to a
// file for regression testing / validating all option combinations. Directives:
//
//   input   <path to input .dat>      (repeatable; >= 3 required; conc read from Conc: header)
//   output  <path to result .dat>     (required)
//   common_crop     0 | 1             (crop inputs to their shared q-overlap first; default 1.
//                                      prevents the loader from spline-resampling mismatched grids)
//   fill_sd         0 | 1             (repair non-positive input SDs before loading; default 1.
//                                      A single zero SD (often a grid edge) otherwise disables that
//                                      curve's errors entirely -- SOMO error handling is all-or-
//                                      nothing -- forcing an unweighted fit. Each non-positive SD is
//                                      replaced by its nearest positive neighbour, but only when the
//                                      positive SDs are the majority (a mostly-zero curve has no real
//                                      errors to draw from). Warnings are logged either way. Set 0 to
//                                      disable repair, e.g. to reproduce the raw unweighted result.)
//   model   additive | reciprocal | virial2         (default additive)
//   ref_scale       0 | 1             (output on reference absolute scale; default 0)
//   merge_ref       0 | 1             (splice reference curve above merge q; default 0)
//   merge_q         <double>          (optional; pin the splice switchover to this q instead of
//                                      auto-locating it. Requires merge_ref 1. 0 = automatic.
//                                      Lets you test a specific cut, or override an automatic
//                                      placement distorted by the regularization near that point.)
//   gcv             0 | 1             (automatic GCV slope regularization; default 1)
//   sd_weights      0 | 1             (1/sigma^2 weighting of the regressions; default 1)
//   fit_broaden     <int>             (Zimm fit-broadening q-window; default 0 = off)
//   recompute_inputs off | constant | nonconstant | intensity   (default off)
//   sd_reassess      off | constant | nonconstant | intensity   (post-fit output SDs; default off)
//   discard_outlier 0 | 1             (auto-discard outlier concentration curve(s); default 0)
//   outlier_max     <int>             (max curves the QC may discard; default 1 = original single
//                                      discard. >1 re-runs the detector on the cleaned set, which
//                                      matters when more than one curve is contaminated: a single
//                                      pass judges each curve against a trend still containing the
//                                      other offender, so the nominee itself can be wrong.)
//   outlier_leverage 0 | 1            (studentize the nomination residual by sqrt(1-h); default 1.
//                                      Without it the highest-concentration curve is masked -- it is
//                                      the extreme x, so it has the most leverage, pulls the fit onto
//                                      itself and hides its own residual.)
//   outlier_sigma   <double>          (outlier magnitude threshold; default 3.0)
//   outlier_chi2    <double>          (required pooled chi^2 gain to confirm a drop; default 1.5)
//   reference <curve-name | conc>     (optional; choose the reference curve used for the absolute
//                                      scale and/or the high-q splice, instead of the default
//                                      highest-concentration pick. Accepts a curve name (or a
//                                      distinctive part of one) or a concentration value. Useful
//                                      when the top-concentration curve is known to be defective:
//                                      that cannot be detected from the concentration regression,
//                                      because the extreme-concentration curve has leverage ~1 and
//                                      the fit passes essentially through it. A non-matching value
//                                      is reported, never silently ignored.)
//   conc    <curve-name> <double>     (optional; override a curve's concentration. name may be
//                                      quoted; matched against the plotted/dequoted curve name)
//   dump_conc_csv   <path>            (optional; after the run, write the conc_csv table as
//                                      name<TAB>conc<TAB>psv<TAB>i0se for every plotted curve --
//                                      used to validate that psv/i0se are carried to the output)
//
// Input/output paths take the entire remainder of the line (they may contain spaces).

static int us_extrap_c0_script_sd_word( const QString & w )
{
   // map an SD-mode word to the internal code used by do_extrap_c0:
   // recompute_inputs uses 0/1/2 (constant/non-constant/intensity); sd_reassess uses 1/2/3
   // with 0 = off. Return -1 for "off", else 1=constant, 2=non-constant, 3=intensity.
   QString l = w.toLower();
   if ( l == "off" || l == "none" || l == "0" ) return -1;
   if ( l == "constant"    || l == "c" || l == "1" ) return 1;
   if ( l == "nonconstant" || l == "non-constant" || l == "n" || l == "2" ) return 2;
   if ( l == "intensity"   || l == "i" || l == "3" ) return 3;
   return -2; // unrecognized
}

// A data row is a line whose first whitespace token parses as a number and has >= 2 tokens;
// anything else (US-SOMO header, "q I sd" column line, comments) is treated as a header line.
static bool us_extrap_c0_script_is_data_row( const QString & line, double & q )
{
   QStringList t = line.simplified().split( ' ' );
   if ( t.size() < 2 ) { return false; }
   bool ok = false;
   q = t[ 0 ].toDouble( &ok );
   return ok;
}

// Scan an input curve's q-range (first and last data-row q) and count non-positive SD points.
static bool us_extrap_c0_script_parse_qrange( const QString & path, double & q_front, double & q_back,
                                              int & n_zero_sd, QString & errmsg )
{
   n_zero_sd = 0;
   QFile f( path );
   if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) ) { errmsg = QString( "cannot open '%1'" ).arg( path ); return false; }
   QTextStream ts( &f );
   bool have = false;
   double q = 0e0, last = 0e0;
   while ( !ts.atEnd() )
   {
      QString line = ts.readLine();
      if ( !us_extrap_c0_script_is_data_row( line, q ) ) { continue; }
      if ( !have ) { q_front = q; have = true; }
      last = q;
      QStringList t = line.simplified().split( ' ' );
      if ( t.size() < 3 || t[ 2 ].toDouble() <= 0e0 ) { n_zero_sd++; }
   }
   f.close();
   q_back = last;
   if ( !have ) { errmsg = QString( "no data rows in '%1'" ).arg( path ); return false; }
   return true;
}

// Copy an input curve to out_path keeping only data rows with q in [lo,hi] (header lines verbatim,
// data lines verbatim so precision/columns/Conc: header are preserved untouched).
// Copy an input curve to out_path keeping only data rows with q in [lo,hi]. Header lines are copied
// verbatim. n_zero counts kept rows whose SD (3rd column) is <= 0; if fill_sd, each such SD is
// replaced by the nearest positive-SD neighbour (n_filled counts replacements) so the loaded curve
// keeps all-nonzero errors. This matters because SOMO uses all-or-nothing error handling: a single
// zero SD (typically a grid edge) disables the whole curve's errors and forces an unweighted fit.
static bool us_extrap_c0_script_write_crop( const QString & in_path, const QString & out_path,
                                            double lo, double hi, bool fill_sd,
                                            int & n_zero, int & n_filled, QString & errmsg )
{
   n_zero = 0; n_filled = 0;
   QFile fi( in_path );
   if ( !fi.open( QIODevice::ReadOnly | QIODevice::Text ) ) { errmsg = QString( "cannot open '%1'" ).arg( in_path ); return false; }

   const double tol = 1e-7;
   QStringList        headers;                 // leading non-data lines, verbatim
   QStringList        rows;                     // in-range data rows, verbatim
   vector < QString > qtok, itok;               // parsed q,I token strings (to re-emit filled rows)
   vector < double >  sds;                      // parsed SD of each kept row
   {
      QTextStream is( &fi );
      double q = 0e0;
      while ( !is.atEnd() )
      {
         QString line = is.readLine();
         if ( !us_extrap_c0_script_is_data_row( line, q ) ) { headers << line; continue; }
         if ( q < lo - tol || q > hi + tol ) { continue; }
         QStringList t = line.simplified().split( ' ' );
         rows  << line;
         qtok.push_back( t.size() > 0 ? t[ 0 ] : QString( "0" ) );
         itok.push_back( t.size() > 1 ? t[ 1 ] : QString( "0" ) );
         double sd = ( t.size() > 2 ) ? t[ 2 ].toDouble() : 0e0;
         sds.push_back( sd );
         if ( sd <= 0e0 ) { n_zero++; }
      }
   }
   fi.close();

   // fill each non-positive SD from the nearest positive-SD neighbour (index distance), but only
   // when the positive SDs are the majority -- a mostly/all-zero curve has no real error information
   // to interpolate from, so it is left as-is (its errors stay disabled) and only warned about.
   vector < double > filled = sds;
   if ( fill_sd && n_zero && n_zero * 2 < (int) sds.size() )
   {
      int n = (int) sds.size();
      for ( int i = 0; i < n; i++ )
      {
         if ( sds[ i ] > 0e0 ) { continue; }
         double v = 0e0;
         for ( int d = 1; d < n; d++ )
         {
            if ( i - d >= 0 && sds[ i - d ] > 0e0 ) { v = sds[ i - d ]; break; }
            if ( i + d <  n && sds[ i + d ] > 0e0 ) { v = sds[ i + d ]; break; }
         }
         if ( v > 0e0 ) { filled[ i ] = v; n_filled++; }
      }
   }

   QFile fo( out_path );
   if ( !fo.open( QIODevice::WriteOnly | QIODevice::Text ) ) { errmsg = QString( "cannot write '%1'" ).arg( out_path ); return false; }
   QTextStream os( &fo );
   for ( int i = 0; i < headers.size(); i++ ) { os << headers[ i ] << "\n"; }
   for ( int i = 0; i < rows.size(); i++ )
   {
      if ( fill_sd && sds[ i ] <= 0e0 && filled[ i ] > 0e0 )
      {
         os << qtok[ i ] << "\t" << itok[ i ] << "\t" << QString::number( filled[ i ], 'e', 6 ) << "\n";
      }
      else
      {
         os << rows[ i ] << "\n";
      }
   }
   fo.close();
   return true;
}

void US_Hydrodyn_Saxs::saxs_extrap_c0_script( QString controlfile )
{
   QFile f( controlfile );
   if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      TSO << QString( "saxs_extrap_c0: cannot open control file '%1'\n" ).arg( controlfile );
      return;
   }

   QStringList  inputs;
   QString      output;
   QString      dump_conc_csv_path;   // optional: dump the conc_csv table after the run (validation)
   bool         common_crop = true;   // crop inputs to their shared q-overlap before loading
   bool         fill_sd     = true;   // repair non-positive input SDs (when most are present) so errors stay enabled
   // start every run from the documented defaults, then apply directives
   extrap_c0_script_model          = 0;
   extrap_c0_script_ref_scale      = false;
   extrap_c0_script_merge_ref      = false;
   extrap_c0_script_gcv            = true;
   extrap_c0_script_sd_weights     = true;
   extrap_c0_script_fit_broaden    = 0;
   extrap_c0_script_recompute_in   = false;
   extrap_c0_script_recompute_mode = 0;
   extrap_c0_script_sd_mode        = 0;
   extrap_c0_script_discard        = false;
   extrap_c0_script_outlier_sigma  = 3e0;
   extrap_c0_script_outlier_chi2   = 1.5e0;
   extrap_c0_script_outlier_max    = 1;
   extrap_c0_script_outlier_leverage = true;
   extrap_c0_script_reference.clear();
   extrap_c0_script_merge_q        = 0e0;
   extrap_c0_script_gcv_lambda     = 0e0;
   extrap_c0_script_conc.clear();

   QRegularExpression rx_key( "^\\s*(\\S+)\\s+(.*\\S)\\s*$" );
   QRegularExpression rx_comment( "^\\s*#" );
   QRegularExpression rx_blank( "^\\s*$" );

   QTextStream ts( &f );
   int lineno = 0;
   bool parse_ok = true;
   while ( !ts.atEnd() )
   {
      QString line = ts.readLine();
      ++lineno;
      if ( line.contains( rx_comment ) || line.contains( rx_blank ) )
      {
         continue;
      }
      QRegularExpressionMatch m = rx_key.match( line );
      if ( !m.hasMatch() )
      {
         TSO << QString( "saxs_extrap_c0: line %1: cannot parse '%2'\n" ).arg( lineno ).arg( line );
         parse_ok = false;
         continue;
      }
      QString key = m.captured( 1 ).toLower();
      QString val = m.captured( 2 ).trimmed();

      if ( key == "input" )
      {
         inputs << val;
      }
      else if ( key == "output" )
      {
         output = val;
      }
      else if ( key == "model" )
      {
         QString l = val.toLower();
         if      ( l == "additive"   || l == "0" ) extrap_c0_script_model = 0;
         else if ( l == "reciprocal" || l == "1" ) extrap_c0_script_model = 1;
         else if ( l == "virial2"    || l == "virial" || l == "2" ) extrap_c0_script_model = 2;
         else { TSO << QString( "saxs_extrap_c0: line %1: unknown model '%2'\n" ).arg( lineno ).arg( val ); parse_ok = false; }
      }
      else if ( key == "common_crop") { common_crop = ( val.toInt() != 0 ); }
      else if ( key == "fill_sd"    ) { fill_sd     = ( val.toInt() != 0 ); }
      else if ( key == "dump_conc_csv" ) { dump_conc_csv_path = val; }
      else if ( key == "ref_scale"  ) { extrap_c0_script_ref_scale  = ( val.toInt() != 0 ); }
      else if ( key == "merge_ref"  ) { extrap_c0_script_merge_ref  = ( val.toInt() != 0 ); }
      else if ( key == "gcv"        ) { extrap_c0_script_gcv        = ( val.toInt() != 0 ); }
      else if ( key == "sd_weights" ) { extrap_c0_script_sd_weights = ( val.toInt() != 0 ); }
      else if ( key == "fit_broaden") { extrap_c0_script_fit_broaden = val.toInt(); }
      else if ( key == "discard_outlier" ) { extrap_c0_script_discard = ( val.toInt() != 0 ); }
      else if ( key == "outlier_sigma" )   { extrap_c0_script_outlier_sigma = val.toDouble(); }
      else if ( key == "outlier_chi2" )    { extrap_c0_script_outlier_chi2  = val.toDouble(); }
      else if ( key == "outlier_max" )     { extrap_c0_script_outlier_max   = val.toInt() > 0 ? val.toInt() : 1; }
      else if ( key == "outlier_leverage" ){ extrap_c0_script_outlier_leverage = ( val.toInt() != 0 ); }
      else if ( key == "reference" )       { extrap_c0_script_reference      = val; }
      else if ( key == "merge_q" )         { extrap_c0_script_merge_q       = val.toDouble(); }
      else if ( key == "gcv_lambda" )      { extrap_c0_script_gcv_lambda    = val.toDouble(); }
      else if ( key == "recompute_inputs" )
      {
         int code = us_extrap_c0_script_sd_word( val );
         if ( code == -1 ) { extrap_c0_script_recompute_in = false; extrap_c0_script_recompute_mode = 0; }
         else if ( code >= 1 ) { extrap_c0_script_recompute_in = true; extrap_c0_script_recompute_mode = code - 1; }
         else { TSO << QString( "saxs_extrap_c0: line %1: unknown recompute_inputs '%2'\n" ).arg( lineno ).arg( val ); parse_ok = false; }
      }
      else if ( key == "sd_reassess" )
      {
         int code = us_extrap_c0_script_sd_word( val );
         if ( code == -1 ) { extrap_c0_script_sd_mode = 0; }
         else if ( code >= 1 ) { extrap_c0_script_sd_mode = code; }
         else { TSO << QString( "saxs_extrap_c0: line %1: unknown sd_reassess '%2'\n" ).arg( lineno ).arg( val ); parse_ok = false; }
      }
      else if ( key == "conc" )
      {
         // "conc <name...> <value>": value is the last whitespace token, name is the rest
         int sp = val.lastIndexOf( QRegularExpression( "\\s+" ) );
         if ( sp <= 0 )
         {
            TSO << QString( "saxs_extrap_c0: line %1: conc needs '<name> <value>'\n" ).arg( lineno );
            parse_ok = false;
         }
         else
         {
            QString name = val.left( sp ).trimmed();
            double  c    = val.mid( sp + 1 ).trimmed().toDouble();
            name.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );
            extrap_c0_script_conc[ name ] = c;
         }
      }
      else
      {
         TSO << QString( "saxs_extrap_c0: line %1: unknown directive '%2'\n" ).arg( lineno ).arg( key );
         parse_ok = false;
      }
   }
   f.close();

   if ( !parse_ok )
   {
      TSO << "saxs_extrap_c0: aborting due to control-file parse errors\n";
      return;
   }
   if ( inputs.size() < 3 )
   {
      TSO << QString( "saxs_extrap_c0: need at least 3 'input' curves, got %1\n" ).arg( inputs.size() );
      return;
   }
   if ( output.isEmpty() )
   {
      TSO << "saxs_extrap_c0: missing 'output' directive\n";
      return;
   }

   // start from an empty plot so repeated saxs_extrap_c0 runs in one session are independent
   // (curves would otherwise accumulate across runs).
   clear_plot_saxs_data();

   // Pre-load conditioning of the inputs (this scripted flow only):
   //  * common_crop (default on): crop to the shared q-overlap [max(first q), min(last q)] so
   //    grid-aligned curves -- the common HPLC case, one detector grid cropped at different low-q --
   //    load as identical grids with no interpolation; otherwise the shared loader resamples
   //    mismatched grids, spline-extrapolating higher-starting curves below their data.
   //  * zero-SD handling: SOMO disables a curve's errors entirely if ANY point has a non-positive SD
   //    (all-or-nothing), forcing an unweighted fit. Such curves are always warned about; with
   //    fill_sd on, each non-positive SD is replaced by its nearest positive neighbour so errors
   //    stay enabled. Headers (incl. Conc:) are preserved, so concentrations/names are unchanged.
   {
      double lo = -1e99, hi = 1e99, min_front = 1e99, max_back = -1e99;
      int    total_zero = 0;
      bool   ok = true;
      vector < int > zero_per( inputs.size(), 0 );
      for ( int i = 0; i < inputs.size() && ok; ++i )
      {
         double fq = 0e0, bq = 0e0; int nz = 0; QString e;
         if ( !us_extrap_c0_script_parse_qrange( inputs[ i ], fq, bq, nz, e ) )
         {
            TSO << QString( "saxs_extrap_c0: input scan: %1\n" ).arg( e );
            ok = false; break;
         }
         if ( fq > lo ) { lo = fq; }
         if ( bq < hi ) { hi = bq; }
         if ( fq < min_front ) { min_front = fq; }
         if ( bq > max_back  ) { max_back  = bq; }
         zero_per[ i ] = nz; total_zero += nz;
      }
      if ( ok && lo >= hi )
      {
         TSO << QString( "saxs_extrap_c0: inputs have no common q-overlap (max start %1 >= min end %2)\n" ).arg( lo ).arg( hi );
         ok = false;
      }
      if ( !ok ) { TSO << "saxs_extrap_c0: aborting (input scan failed)\n"; return; }

      // warn about non-positive SDs (each disables its whole curve's errors -> unweighted fit)
      for ( int i = 0; i < inputs.size(); ++i )
      {
         if ( zero_per[ i ] )
         {
            TSO << QString( "saxs_extrap_c0: WARNING: %1 has %2 non-positive SD point(s); SOMO disables that "
                            "curve's errors entirely (unweighted fit)%3\n" )
               .arg( QFileInfo( inputs[ i ] ).fileName() ).arg( zero_per[ i ] )
               .arg( fill_sd ? " -- attempting repair (fill_sd on)" : " -- fill_sd off, not repaired" );
         }
      }

      const double tol = 1e-7;
      bool need_crop = common_crop && ( lo > min_front + tol || hi < max_back - tol );
      bool need_fill = fill_sd && total_zero;
      if ( need_crop || need_fill )
      {
         double clo = common_crop ? lo : min_front;   // keep each curve's full range when not cropping
         double chi = common_crop ? hi : max_back;
         QString tmpdir = QDir::tempPath() + "/us_extrap_c0_prep_" + QFileInfo( output ).completeBaseName();
         QDir().mkpath( tmpdir );
         TSO << QString( "saxs_extrap_c0: preparing %1 inputs%2%3 in %4\n" )
            .arg( inputs.size() )
            .arg( need_crop ? QString( " (crop to q %1:%2)" ).arg( clo ).arg( chi ) : QString() )
            .arg( need_fill ? QString( " (fill non-positive SDs)" ) : QString() )
            .arg( tmpdir );
         QStringList prepared;
         int total_filled = 0;
         // Name each cropped/filled copy by its ORIGINAL basename when the inputs' basenames are
         // all distinct (the normal case: a labelled concentration series). Then the loaded curve
         // names keep the shared series prefix (e.g. X4_Lys...), so the extrapolated curve's derived
         // name and any per-curve warnings stay meaningful instead of being defeated by an index
         // prefix. Only fall back to an index-prefixed basename if two inputs share a basename (same
         // filename from different dirs), which would otherwise collide as identical plotted names.
         bool basenames_unique = true;
         {
            QStringList seen;
            for ( int i = 0; i < inputs.size(); ++i )
            {
               QString b = QFileInfo( inputs[ i ] ).fileName();
               if ( seen.contains( b ) ) { basenames_unique = false; break; }
               seen << b;
            }
         }
         for ( int i = 0; i < inputs.size() && ok; ++i )
         {
            QString base = QFileInfo( inputs[ i ] ).fileName();
            QString outp = basenames_unique
               ? QString( "%1/%2" ).arg( tmpdir ).arg( base )
               : QString( "%1/%2_%3" ).arg( tmpdir ).arg( i ).arg( base );
            int nz = 0, nf = 0; QString e;
            if ( !us_extrap_c0_script_write_crop( inputs[ i ], outp, clo, chi, fill_sd, nz, nf, e ) )
            {
               TSO << QString( "saxs_extrap_c0: prepare: %1\n" ).arg( e );
               ok = false; break;
            }
            total_filled += nf;
            prepared << outp;
         }
         if ( ok )
         {
            inputs = prepared;
            if ( total_filled ) { TSO << QString( "saxs_extrap_c0: filled %1 non-positive SD point(s) across inputs\n" ).arg( total_filled ); }
         }
      }
      else if ( common_crop )
      {
         TSO << "saxs_extrap_c0: inputs already share a common q-range; no cropping needed\n";
      }
      if ( !ok ) { TSO << "saxs_extrap_c0: aborting (prepare failed)\n"; return; }
   }

   // load every input curve into the SAXS plot exactly as the user would (Conc: headers are
   // parsed into the conc facility here, feeding do_extrap_c0's prepopulated concentrations).
   for ( int i = 0; i < inputs.size(); ++i )
   {
      QFileInfo fi( inputs[ i ] );
      if ( !fi.exists() )
      {
         TSO << QString( "saxs_extrap_c0: input file not found: '%1'\n" ).arg( inputs[ i ] );
         return;
      }
      TSO << QString( "saxs_extrap_c0: loading %1\n" ).arg( inputs[ i ] );
      // no_scaling = true: skip the interactive "scale to plotted target" dialog and keep each
      // curve's own intensity (the extrapolation handles concentration itself; auto-scaling the
      // inputs would corrupt it).
      load_saxs( inputs[ i ], false, "", true );
   }

   // arm the dialog bypass and run the plotted-curve extrapolation path
   extrap_c0_script_out = output;
   extrap_c0_script     = true;
   TSO << QString( "saxs_extrap_c0: extrapolating %1 curves -> %2\n" ).arg( inputs.size() ).arg( output );
   load_saxs( "", true );
   extrap_c0_script     = false;

   // optional: dump the conc_csv table (name, conc, psv, i0se) for regression validation -- lets a
   // test assert that the extrapolated curve (and the inputs) carry the inputs' psv/i0se rather
   // than the global defaults.
   if ( !dump_conc_csv_path.isEmpty() )
   {
      QFile df( dump_conc_csv_path );
      if ( !df.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         TSO << QString( "saxs_extrap_c0: cannot write conc_csv dump '%1'\n" ).arg( dump_conc_csv_path );
      }
      else
      {
         QTextStream ds( &df );
         ds << "name\tconc\tpsv\ti0se\n";
         for ( unsigned int i = 0; i < conc_csv.data.size(); ++i )
         {
            const vector < QString > & r = conc_csv.data[ i ];
            ds << ( r.size() > 0 ? r[ 0 ] : QString() ) << "\t"
               << ( r.size() > 1 ? r[ 1 ] : QString() ) << "\t"
               << ( r.size() > 2 ? r[ 2 ] : QString() ) << "\t"
               << ( r.size() > 3 ? r[ 3 ] : QString() ) << "\n";
         }
         df.close();
         TSO << QString( "saxs_extrap_c0: wrote conc_csv (%1 rows) to %2\n" ).arg( (int) conc_csv.data.size() ).arg( dump_conc_csv_path );
      }
   }

   TSO << "saxs_extrap_c0: done\n";
}
