// gui_script entry points of the SAS window for the robust automatic Guinier range search:
//    sas load_iq <file> [<file> ...]
//    sas guinier_search [csv <file>] [key value ...]
// see us_saxs_util_guinier_search.cpp for the algorithm and US_Guinier_Search_Params::help() for the keys.

#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_saxs_util.h"

bool US_Hydrodyn_Saxs::script_load_iq( const QString & filename, QString & errormsg )
{
   errormsg = "";

   QFileInfo fi( filename );
   if ( !fi.exists() )
   {
      errormsg = us_tr( "file does not exist" );
      return false;
   }
   if ( !fi.isFile() || !fi.isReadable() )
   {
      errormsg = us_tr( "file is not readable" );
      return false;
   }

   int before = (int) plotted_Iq.size();
   load_saxs( filename, false, "", true );
   if ( (int) plotted_Iq.size() == before )
   {
      errormsg = us_tr( "the file did not load as an I(q) curve" );
      return false;
   }
   return true;
}

bool US_Hydrodyn_Saxs::script_guinier_search( const map < QString, QString > & kv, const QString & csvfile, QString & errormsg )
{
   errormsg = "";

   for ( map < QString, QString >::const_iterator it = kv.begin(); it != kv.end(); ++it )
   {
      if ( !US_Guinier_Search_Params::keys().contains( it->first.toLower() ) )
      {
         errormsg = us_tr( "unknown guinier_search parameter " ) + it->first + "\n" + US_Guinier_Search_Params::help();
         return false;
      }
   }

   US_Guinier_Search_Params params;
   if ( !params.set( kv, errormsg ) )
   {
      return false;
   }

   if ( plotted_Iq.empty() )
   {
      errormsg = us_tr( "no I(q) curves are loaded in the SAS window" );
      return false;
   }

   QFile       f( csvfile );
   QTextStream csv;
   if ( !csvfile.isEmpty() )
   {
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         errormsg = us_tr( "could not open csv file for writing: " ) + csvfile;
         return false;
      }
      csv.setDevice( &f );
      csv << US_Guinier_Search_Result::csv_header();
   }

   QTextStream  ts( stdout );
   US_Saxs_Util usu;
   int          nfail = 0;

   editor_msg( "blue", us_tr( "Automatic Guinier range search (guinier_search):" ) );
   ts << US_Guinier_Search_Result::text_header();

   for ( int i = 0; i < (int) plotted_Iq.size(); ++i )
   {
      QString name = qsl_plotted_iq_names[ i ];
      usu.wave[ name ].clear();
      usu.wave[ name ].filename = name;
      usu.wave[ name ].q        = plotted_q[ i ];
      usu.wave[ name ].r        = plotted_I[ i ];
      if ( plotted_I_error[ i ].size() == plotted_q[ i ].size() )
      {
         usu.wave[ name ].s = plotted_I_error[ i ];
      }

      US_Guinier_Search_Result r;
      usu.guinier_search( name, params, r );
      guinier_search_results[ name ] = r;
      if ( !r.ok )
      {
         ++nfail;
      }
      QString line = r.text();
      ts << line;
      editor_msg( r.ok ? "black" : "red", line );
      if ( !csvfile.isEmpty() )
      {
         csv << r.csv();
      }
   }
   ts.flush();

   if ( !csvfile.isEmpty() )
   {
      f.close();
      editor_msg( "blue", us_tr( "guinier_search results written to " ) + csvfile );
   }

   if ( nfail )
   {
      errormsg = QString( us_tr( "guinier_search failed on %1 of %2 curves" ) ).arg( nfail ).arg( plotted_Iq.size() );
      return false;
   }
   return true;
}
