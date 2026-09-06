// us_hydrodyn_dad_script.cpp contains the non-interactive entry points used by
// the gui_script "dad" commands ( us3_somo -g ).  everything here runs with
// script_mode set, so the dialogs the module would otherwise raise are answered
// from the script_* values instead of prompting

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_dad.h"
#include <QRegularExpression>
#include <QTextStream>

#define TSO QTextStream( stdout )

bool US_Hydrodyn_Dad::script_load_lambdas( const QString & filename, QString & errormsg ) {
   errormsg = "";

   if ( !QFile::exists( filename ) ) {
      errormsg = QString( "file %1 not found" ).arg( filename );
      return false;
   }

   if ( !dad_lambdas.load( filename, errormsg ) ) {
      return false;
   }

   editor_msg( "black", QString( "%1 UV-Vis lambdas loaded\n" ).arg( filename ) );
   editor_msg( "black", dad_lambdas.summary() );
   lbl_dad_lambdas_data->setText( dad_lambdas.summary_rich() + DAD_LAMBDA_EXTC_MSG );
   TSO << QString( "dad: %1 lambdas loaded from %2\n" ).arg( dad_lambdas.lambdas.size() ).arg( filename );
   return true;
}

bool US_Hydrodyn_Dad::script_load( const QString & filename, QString & errormsg ) {
   errormsg = "";

   if ( !QFile::exists( filename ) ) {
      errormsg = QString( "file %1 not found" ).arg( filename );
      return false;
   }

   int    before         = lb_files->count();
   size_t lambdas_before = dad_lambdas.lambdas.size();

   // add_files() dispatches on extension and content: a saved .dat curve, a
   // UV-Vis wavelength list and a raw absorption matrix all arrive here.  it
   // is the entry point rather than load_file() because load_file() fills the
   // data maps but leaves the file list to its caller.  the UV-Vis paths
   // return false even when they succeed, so judge by what actually appeared

   QStringList one;
   one << filename;
   add_files( one );

   int added = lb_files->count() - before;

   if ( added > 0 ) {
      TSO << QString( "dad: %1 loaded, %2 curves added\n" ).arg( filename ).arg( added );
      return true;
   }

   if ( dad_lambdas.lambdas.size() != lambdas_before ) {
      // a wavelength list adds no curves, that is still a successful load
      TSO << QString( "dad: %1 loaded, %2 lambdas\n" ).arg( filename ).arg( dad_lambdas.lambdas.size() );
      return true;
   }

   errormsg = QString( "%1 produced no curves" ).arg( filename );
   return false;
}

QStringList US_Hydrodyn_Dad::script_files( bool selected_only ) {
   QStringList files;
   for ( int i = 0; i < lb_files->count(); ++i ) {
      if ( !selected_only || lb_files->item( i )->isSelected() ) {
         files << lb_files->item( i )->text();
      }
   }
   return files;
}

bool US_Hydrodyn_Dad::script_select( const QString & match, QString & errormsg ) {
   errormsg = "";

   set < QString > to_select;

   for ( int i = 0; i < lb_files->count(); ++i ) {
      QString name = lb_files->item( i )->text();
      if ( match == "all" || name.contains( match ) ) {
         to_select.insert( name );
      }
   }

   if ( to_select.empty() ) {
      errormsg = QString( "no curves match %1" ).arg( match );
      return false;
   }

   set_selected( to_select );
   TSO << QString( "dad: %1 curves selected\n" ).arg( to_select.size() );
   return true;
}

bool US_Hydrodyn_Dad::script_make_a_of_lambda( QString & errormsg ) {
   errormsg = "";

   QStringList selected = script_files( true );

   if ( selected.size() < 2 ) {
      errormsg = "Make A(lambda) needs at least two selected A(t) curves";
      return false;
   }

   int before = lb_files->count();

   if ( !create_i_of_q_ng( selected ) ) {
      errormsg = "Make A(lambda) failed";
      return false;
   }

   TSO << QString( "dad: Make A(lambda) created %1 curves\n" ).arg( lb_files->count() - before );
   return true;
}

bool US_Hydrodyn_Dad::script_save( const QString & dir, QString & errormsg ) {
   errormsg = "";

   QStringList selected = script_files( true );

   if ( selected.isEmpty() ) {
      errormsg = "no curves selected to save";
      return false;
   }

   if ( !dir.isEmpty() ) {
      le_created_dir->setText( dir );
   }

   // overwrite_all starts true so an existing file never raises the
   // overwrite/rename dialog

   bool errors        = false;
   bool overwrite_all = true;
   bool cancel        = false;

   for ( int i = 0; i < (int) selected.size(); ++i ) {
      if ( !save_file( selected[ i ], cancel, overwrite_all ) ) {
         errors = true;
      }
      if ( cancel ) {
         break;
      }
   }

   update_enables();

   if ( errors ) {
      errormsg = "one or more curves could not be saved";
      return false;
   }

   TSO << QString( "dad: %1 curves saved to %2\n" ).arg( selected.size() ).arg( le_created_dir->text() );
   return true;
}
