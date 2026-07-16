#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include <QRegularExpression>
#include <QFileInfo>
#include <QFile>
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
//   model   additive | reciprocal | virial2         (default reciprocal)
//   ref_scale       0 | 1             (output on reference absolute scale; default 0)
//   merge_ref       0 | 1             (splice reference curve above merge q; default 0)
//   gcv             0 | 1             (automatic GCV slope regularization; default 1)
//   sd_weights      0 | 1             (1/sigma^2 weighting of the regressions; default 1)
//   fit_broaden     <int>             (Zimm fit-broadening q-window; default 0 = off)
//   recompute_inputs off | constant | nonconstant | intensity   (default off)
//   sd_reassess      off | constant | nonconstant | intensity   (post-fit output SDs; default off)
//   discard_outlier 0 | 1             (auto-discard one outlier concentration; default 0)
//   outlier_sigma   <double>          (outlier magnitude threshold; default 3.0)
//   outlier_chi2    <double>          (required pooled chi^2 gain to confirm a drop; default 1.5)
//   conc    <curve-name> <double>     (optional; override a curve's concentration. name may be
//                                      quoted; matched against the plotted/dequoted curve name)
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
   // start every run from the documented defaults, then apply directives
   extrap_c0_script_model          = 1;
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
      else if ( key == "ref_scale"  ) { extrap_c0_script_ref_scale  = ( val.toInt() != 0 ); }
      else if ( key == "merge_ref"  ) { extrap_c0_script_merge_ref  = ( val.toInt() != 0 ); }
      else if ( key == "gcv"        ) { extrap_c0_script_gcv        = ( val.toInt() != 0 ); }
      else if ( key == "sd_weights" ) { extrap_c0_script_sd_weights = ( val.toInt() != 0 ); }
      else if ( key == "fit_broaden") { extrap_c0_script_fit_broaden = val.toInt(); }
      else if ( key == "discard_outlier" ) { extrap_c0_script_discard = ( val.toInt() != 0 ); }
      else if ( key == "outlier_sigma" )   { extrap_c0_script_outlier_sigma = val.toDouble(); }
      else if ( key == "outlier_chi2" )    { extrap_c0_script_outlier_chi2  = val.toDouble(); }
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
   TSO << "saxs_extrap_c0: done\n";
}
