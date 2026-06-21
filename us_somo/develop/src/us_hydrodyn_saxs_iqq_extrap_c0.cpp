#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_saxs_iqq_extrap_c0_conc.h"
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

static QString us_extrap_c0_curve_name( const QStringList &names )
{
   QString prefix = us_extrap_c0_common_prefix( names ).trimmed();
   prefix.remove( QRegularExpression( "[\\s_-]+$" ) );

   if ( prefix.isEmpty() )
   {
      return "extrap_c0";
   }
   return prefix + "_extrap_c0";
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
   bool dlg_ok = false;
   {
      US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc dlg( ordered_names, prepop_conc, &name_to_conc, &dlg_ok, us_hydrodyn, this );
      US_Hydrodyn::fixWinButtons( &dlg );
      dlg.exec();
   }

   if ( !dlg_ok )
   {
      this->isVisible() ? this->raise() : this->show();
      editor_msg( "black", us_tr( "Extrapolation to zero concentration cancelled\n" ) );
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

   // loaded I(q) curves are raw (not pre-normalized by concentration): raw SAXS
   // intensity is dominated by a term directly proportional to c, so a linear fit
   // of raw I(q) vs c trivially extrapolates to ~0 at c=0 (no protein, no excess
   // scattering) -- not physically useful. The standard "extrapolation to zero
   // concentration" technique (the SAXS analogue of a Zimm plot) instead fits the
   // concentration-NORMALIZED intensity I(q,c)/c against c; its c=0 intercept is
   // the ideal, structure-factor-free dilute-limit curve. So divide by concentration
   // here, on the intensity axis only -- the concentration (x) axis stays the real,
   // distinct entered values; collapsing it (e.g. to 1 for "already normalized"
   // data) would remove the spread the regression needs and isn't equivalent.
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

   // 6. per-q linear regression: I(q)/c vs concentration -> intercept (I0) + its standard error

   US_Saxs_Util usu;

   vector < double > out_q;
   vector < double > out_I0;
   vector < double > out_I0_err;
   unsigned int       skipped_points = 0;

   for ( unsigned int qi = 0; qi < npts; qi++ )
   {
      vector < double > x;
      vector < double > y;

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
         x.push_back( concs[ ci ] );
         y.push_back( Iv / concs[ ci ] );
      }

      if ( x.size() < 2 )
      {
         TSO << QString( "WARNING: do_extrap_c0: insufficient valid points (%1) to regress at q[%2]=%3, skipping\n" )
            .arg( x.size() ).arg( qi ).arg( q[ qi ] );
         skipped_points++;
         continue;
      }

      double a, b, siga, sigb, chi2;

      if ( x.size() == 2 )
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
         usu.linear_fit( x, y, a, b, siga, sigb, chi2 );
      }

      out_q     .push_back( q[ qi ] );
      out_I0    .push_back( a );
      out_I0_err.push_back( siga );
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

   // 7. name the new curve, avoiding collisions with already-plotted curves

   QString base_name  = us_extrap_c0_curve_name( ordered_names );
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

   editor_msg( "black",
              QString( "Added zero-concentration extrapolation curve \"%1\" (%2 q-points, %3 skipped)\n"
                       "Note: this curve is I(q)/concentration extrapolated to c=0, not raw intensity -- "
                       "its scale differs from the input curves by a factor of concentration.\n" )
              .arg( final_name ).arg( out_q.size() ).arg( skipped_points ) );
}
