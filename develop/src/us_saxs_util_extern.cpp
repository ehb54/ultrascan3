#include "../include/us_saxs_util.h"

#define SLASH QDir::separator()

bool US_Saxs_Util::run_saxs_iq_foxs()
{
   errormsg = "";
   noticemsg = "";
      
   QString pdb = control_parameters[ "inputfile" ];

   QString prog = 
      env_ultrascan + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + "foxs" 
#if defined(WIN32)
      + ".exe"
#endif      
      ;

   {
      QFileInfo qfi( prog );
      if ( !qfi.exists() )
      {
         errormsg = QString( "FoXS program '%1' does not exist\n" ).arg( prog );
         return false;
      }
      if ( !qfi.isExecutable() )
      {
         errormsg = QString( "FoXS program '%1' is not executable\n" ).arg( prog );
         return false;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      errormsg = QString( "FoXS called but PDB file '%1' does not exist\n" ).arg( pdb );
      return false;
   }

   QString cmd = 
      QString( "%1 -q %2 -s %3 %4\n" )
      .arg( prog )
      .arg( our_saxs_options.end_q )
      .arg( (unsigned int)( our_saxs_options.end_q / our_saxs_options.delta_q ) )
      .arg( pdb );

   cout << "Starting FoXS\n";
   cout << cmd;
   system( "cmd" );
   cout << "FoXS finished.\n";

   // foxs creates 2 files:
   // pdb_pdb.dat
   // pdb.dat

   QString created_dat = pdb + ".dat";
   QString created_plt = pdb;
   created_plt.replace( QRegExp( "\\.(pdb|PDB)$" ), ".plt" );
   
   if ( !QFile::exists( created_dat ) )
   {
      errormsg =  QString( "Error: FoXS did not create file %1" ).arg( created_dat );
      return false;
   }

   if ( !QFile::exists( created_plt ) )
   {
      errormsg = QString( "Notice: FoXS did not create file %1" ).arg( created_dat );
      return false;
   } else {
      if ( !QFile::remove( created_plt ) )
      {
         noticemsg += QString( "Notice: remove of FoXS created file %1 failed" ).arg( created_dat );
      }
   }

   // now move the file to the saxs directory
   QString new_created_dat = 
      QFileInfo( pdb ).fileName() + iqq_suffix() + ".dat";

   if ( QFile::exists( new_created_dat ) )
   {
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( created_dat != new_created_dat &&
        !qd.rename( created_dat, new_created_dat ) )
   {
      noticemsg +=
         QString( "Notice: could not rename %1 to %1" )
         .arg( created_dat )
         .arg( new_created_dat );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   if ( !load_saxs( new_created_dat ) )
   {
      return false;
   }
   return true;
}

// -------------------- crysol ------------------------------

bool US_Saxs_Util::run_saxs_iq_crysol()
{
   QString pdb = control_parameters[ "inputfile" ];

   QString prog = 
      env_ultrascan + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + "crysol" 
#if defined(WIN32)
      + ".exe"
#endif      
      ;

   {
      QFileInfo qfi(prog);
      if ( !qfi.exists() )
      {
         errormsg = QString("Crysol program '%1' does not exist\n" ).arg( prog );
         return false;
      }
      if ( !qfi.isExecutable() )
      {
         errormsg = QString("Crysol program '%1' is not executable\n" ).arg( prog );
         return false;
      }
   }
   
   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      errormsg = QString( "Crysol called but PDB file '%1' does not exist\n" ).arg( pdb );
      return false;
   }

   QString crysol_last_pdb = pdb;
   QString crysol_last_pdb_base = QFileInfo( crysol_last_pdb ).fileName().replace( QRegExp( "\\.(pdb|PDB)$" ), "").left( 6 ) + ".pdb";
   QString use_pdb = pdb;
   
   // copy pdb if the name is too long
   if ( our_saxs_options.crysol_version_26 ||
        QFileInfo( crysol_last_pdb ).fileName() != QFileInfo( crysol_last_pdb_base ).fileName() )
   {
      QFile f( pdb );
      if ( !f.open( IO_ReadOnly ) )
      {
         errormsg = QString( "Could not open file %1. Check permissions\n" ).arg( pdb );
         return false;
      }

      QFile f2( crysol_last_pdb_base );
      if ( !f2.open( IO_WriteOnly ) )
      {
         errormsg = QString( "Could not open file %1. Check permissions\n" ).arg( crysol_last_pdb_base );
         return false;
      }

      QString qs;
      QTextStream ts( &f );
      QTextStream ts2( &f2 );

      while ( !ts.atEnd() )
      {
         qs = ts.readLine();
         ts2 << qs << endl;
      }
      f.close();
      f2.close();
      use_pdb = crysol_last_pdb_base;
   } else {
      use_pdb = pdb;
   }
   
   cout << "use_pdb: <" << use_pdb << ">\n";
   cout << "crysol_last_pdb_base: <" << crysol_last_pdb_base << ">\n";

   // clean up so we have new files

   {
      QString base = crysol_last_pdb_base.replace( QRegExp("\\.(pdb|PDB)$" ), "" );
      cout << "base: <" << base << ">\n";

      QString to_remove = base + "00.alm";
      cout << "to_remove: <" << to_remove << ">\n";
      QFile::remove( to_remove );

      to_remove = base + "00.log";
      QFile::remove( to_remove );

      to_remove = base + "00.int";
      QFile::remove( to_remove );
   }      

   QString cmd = 
      QString( "%1" ).arg( prog );

   cmd += 
      QString( " %1" )
      .arg( our_saxs_options.crysol_version_26 ? QFileInfo(use_pdb).fileName() : use_pdb );

   cmd += 
      QString( " /sm %1" )
      .arg( our_saxs_options.end_q );

   cmd += 
      QString( " /ns %1" )
      .arg( (unsigned int)( our_saxs_options.end_q / our_saxs_options.delta_q ) );

   cmd += 
      QString( " /dns %1" )
      .arg( our_saxs_options.water_e_density );

   cmd += 
      QString( " /dro %1" )
      .arg( our_saxs_options.crysol_hydration_shell_contrast );

   cmd += 
      QString( " /lm %1" )
      .arg( our_saxs_options.crysol_max_harmonics );

   cmd += 
      QString( " /fb %1" )
      .arg( our_saxs_options.crysol_fibonacci_grid_order );

   cout << "Starting Crysol\n";
   cout << cmd;
   system( "cmd" );
   cout << "Crysol Finished\n";
   
   // crysol creates 4 files:
   // crysol_summary.txt
   // pdb##.alm
   // pdb##.log
   // pdb##.int

   // we just want the .int, the rest will be removed if needed

   QString created_dat = crysol_last_pdb_base.replace( QRegExp( "\\.(pdb|PDB)$" ), "" ) +  "00.int";

   if ( !QFile::exists( created_dat ) )
   {
      errormsg = QString( "Error: Crysol did not create file %1" ).arg( created_dat );
      return false;
   }

   // now move the file to the saxs directory
   QString new_created_dat = 
      QFileInfo( crysol_last_pdb.replace( QRegExp( "\\.(pdb|PDB)$" ), "" ) ).fileName() + iqq_suffix() + ".int";

   if ( QFile::exists(new_created_dat) )
   {
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( created_dat != new_created_dat &&
        !qd.rename( created_dat, new_created_dat ) )
   {
      noticemsg += 
         QString( "Notice: could not rename %1 to %1" )
         .arg( created_dat )
         .arg( new_created_dat );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   if ( !load_saxs( new_created_dat ) )
   {
      return false;
   }
   return true;
}
   

// -------------------- cryson ------------------------------

bool US_Saxs_Util::run_sans_iq_cryson()
{
   errormsg = "cryson currently unimplemented";
   return false;
}

bool US_Saxs_Util::load_saxs( QString /* filename */ )
{
   errormsg = "load_saxs currently unimplemented";
   return false;
}
