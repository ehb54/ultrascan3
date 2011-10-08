#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include <qregexp.h>

#ifndef WIN32
#   define SLASH "/"
#else
#   define SLASH "\\"
#endif

void US_Hydrodyn_Saxs::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
}

// -------------------- FoXS ------------------------------

int US_Hydrodyn_Saxs::run_saxs_iq_foxs( QString pdb )
{
   QString prog = 
      USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "/bin/"
#endif
      + SLASH
      + "foxs" 
#if defined(WIN32)
      + ".exe"
#endif      
      ;

   {
      QFileInfo qfi(prog);
      if ( !qfi.exists() )
      {
         editor_msg("red", QString("FoXS program '%1' does not exist\n").arg(prog));
         return -1;
      }
      if ( !qfi.isExecutable() )
      {
         editor_msg("red", QString("FoXS program '%1' is not executable\n").arg(prog));
         return -1;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      editor_msg("red", QString("FoXS called but PDB file '%1' does not exist\n").arg(pdb));
      return -1;
   }

   pb_plot_saxs_sans->setEnabled(false);

   foxs_last_pdb = pdb;

   foxs = new QProcess( this );
   //   foxs->setWorkingDirectory( dir );
   foxs->addArgument( prog );

   foxs->addArgument( "-q" );
   foxs->addArgument( QString("%1").arg( our_saxs_options->end_q ) );

   foxs->addArgument( "-s" );
   foxs->addArgument( QString("%1").arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q)) );

   foxs->addArgument( pdb );

   cout << 
      QString("foxs -q %1 -s %2 %3\n")
      .arg( our_saxs_options->end_q )
      .arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q) )
      .arg( pdb );

   connect( foxs, SIGNAL(readyReadStdout()), this, SLOT(foxs_readFromStdout()) );
   connect( foxs, SIGNAL(readyReadStderr()), this, SLOT(foxs_readFromStderr()) );
   connect( foxs, SIGNAL(processExited()), this, SLOT(foxs_processExited()) );
   connect( foxs, SIGNAL(launchFinished()), this, SLOT(foxs_launchFinished()) );

   editor->append("\n\nStarting FoXS\n");
   foxs->start();

   return 0;
}

void US_Hydrodyn_Saxs::foxs_readFromStdout()
{
   while ( foxs->canReadLineStdout() )
   {
      editor_msg("brown", foxs->readLineStdout() + "\n");
   }
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::foxs_readFromStderr()
{
   while ( foxs->canReadLineStderr() )
   {
      editor_msg("red", foxs->readLineStderr() + "\n");
   }
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::foxs_processExited()
{
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   foxs_readFromStderr();
   foxs_readFromStdout();
      //   }
   disconnect( foxs, SIGNAL(readyReadStdout()), 0, 0);
   disconnect( foxs, SIGNAL(readyReadStderr()), 0, 0);
   disconnect( foxs, SIGNAL(processExited()), 0, 0);
   editor->append("FoXS finished.\n");

   // foxs creates 2 files:
   // pdb_pdb.dat
   // pdb.dat

   QString created_dat = foxs_last_pdb + ".dat";
   QString created_plt = foxs_last_pdb;
   created_plt.replace(QRegExp("\\.(pdb|PDB)$"),".plt");

   if ( !QFile::exists( created_dat ) )
   {
      editor_msg("red", QString(tr("Error: FoXS did not create file %1")).arg( created_dat ));
      pb_plot_saxs_sans->setEnabled(true);
      return;
   }

   if ( !QFile::exists( created_plt ) )
   {
      editor_msg("dark red", QString(tr("Notice: FoXS did not create file %1")).arg( created_dat ));
   } else {
      if ( !QFile::remove( created_plt ) )
      {
         editor_msg("red", QString(tr("Notice: remove of FoXS created file %1 failed")).arg( created_dat ));
      }
   }

   // now move the file to the saxs directory
   QString new_created_dat = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + QFileInfo(foxs_last_pdb).fileName() + iqq_suffix() + ".dat";

   if ( QFile::exists(new_created_dat) )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->overwrite )
      {
         editor_msg("red", QString(tr("Notice: overwriting %1")).arg( new_created_dat ));
      } else {
         new_created_dat = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( new_created_dat );
      }
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( !qd.rename( created_dat, new_created_dat ) )
   {
      editor_msg("red", 
                 QString(tr("Notice: could not rename %1 to %1"))
                 .arg( created_dat )
                 .arg( new_created_dat )
                 );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   load_saxs( new_created_dat );
   pb_plot_saxs_sans->setEnabled(true);
}
   
void US_Hydrodyn_Saxs::foxs_launchFinished()
{
   editor_msg("brown", "FoXS launch exited\n");
   disconnect( foxs, SIGNAL(launchFinished()), 0, 0);
}

// -------------------- crysol ------------------------------

int US_Hydrodyn_Saxs::run_saxs_iq_crysol( QString pdb )
{
   QString prog = 
      USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "/bin/"
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
         editor_msg("red", QString("Crysol program '%1' does not exist\n").arg(prog));
         return -1;
      }
      if ( !qfi.isExecutable() )
      {
         editor_msg("red", QString("Crysol program '%1' is not executable\n").arg(prog));
         return -1;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      editor_msg("red", QString("Crysol called but PDB file '%1' does not exist\n").arg(pdb));
      return -1;
   }

   QString dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "tmp";

   QDir qd_dir( dir );

   if ( !qd_dir.exists() )
   {
      editor_msg("red", QString("Crysol called but the director '%1' does not exist\n").arg(qd_dir.path()));
      return -1;
   }

   if ( !qd_dir.isReadable() )
   {
      editor_msg("red", QString("Crysol called but the director '%1' is not readable. Check permissions\n").arg(qd_dir.path()));
      return -1;
   }

   crysol_last_pdb = pdb;
   crysol_last_pdb_base = dir + SLASH + QFileInfo(crysol_last_pdb).fileName().replace(QRegExp("\\.(pdb|PDB)$"),"").left(6) + ".pdb";
   QString use_pdb = pdb;
   
   // copy pdb if the name is too long
   if ( our_saxs_options->crysol_version_26 ||
        QFileInfo(crysol_last_pdb).fileName() != QFileInfo(crysol_last_pdb_base).fileName() )
   {
      QFile f( pdb );
      if ( !f.open( IO_ReadOnly ) )
      {
         editor_msg("red", QString("Could not open file %1. Check permissions\n").arg( pdb ));
         return -1;
      }

      QFile f2( crysol_last_pdb_base );
      if ( !f2.open( IO_WriteOnly ) )
      {
         editor_msg("red", QString("Could not open file %1. Check permissions\n").arg( crysol_last_pdb_base ));
         return -1;
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
      QString base = crysol_last_pdb_base.replace(QRegExp("\\.(pdb|PDB)$"),"");
      cout << "base: <" << base << ">\n";

      QString to_remove = base + "00.alm";
      cout << "to_remove: <" << to_remove << ">\n";
      QFile::remove( to_remove );

      to_remove = base + "00.log";
      QFile::remove( to_remove );

      to_remove = base + "00.int";
      QFile::remove( to_remove );
   }      

   pb_plot_saxs_sans->setEnabled(false);

   crysol = new QProcess( this );
   crysol->setWorkingDirectory( dir );
   crysol->addArgument( prog );
   crysol->addArgument( our_saxs_options->crysol_version_26 ? QFileInfo(use_pdb).fileName() : use_pdb );

   crysol->addArgument( "/sm" );
   crysol->addArgument( QString("%1").arg( our_saxs_options->end_q ) );

   crysol->addArgument( "/ns" );
   crysol->addArgument( QString("%1").arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q)) );

   crysol->addArgument( "/dns" );
   crysol->addArgument( QString("%1").arg( our_saxs_options->water_e_density ) );

   crysol->addArgument( "/dro" );
   crysol->addArgument( QString("%1").arg( our_saxs_options->crysol_hydration_shell_contrast ) );

   crysol->addArgument( "/lm" );
   crysol->addArgument( QString("%1").arg( our_saxs_options->crysol_max_harmonics ) );

   crysol->addArgument( "/fb" );
   crysol->addArgument( QString("%1").arg( our_saxs_options->crysol_fibonacci_grid_order ) );

   connect( crysol, SIGNAL(readyReadStdout()), this, SLOT(crysol_readFromStdout()) );
   connect( crysol, SIGNAL(readyReadStderr()), this, SLOT(crysol_readFromStderr()) );
   connect( crysol, SIGNAL(processExited()), this, SLOT(crysol_processExited()) );
   connect( crysol, SIGNAL(launchFinished()), this, SLOT(crysol_launchFinished()) );

   editor->append("\n\nStarting Crysol\n");
   crysol->start();

   return 0;
}

void US_Hydrodyn_Saxs::crysol_readFromStdout()
{
   while ( crysol->canReadLineStdout() )
   {
      editor_msg("brown", crysol->readLineStdout() + "\n");
   }
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::crysol_readFromStderr()
{
   while ( crysol->canReadLineStderr() )
   {
      editor_msg("red", crysol->readLineStderr() + "\n");
   }
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::crysol_processExited()
{
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   crysol_readFromStderr();
   crysol_readFromStdout();
      //   }
   disconnect( crysol, SIGNAL(readyReadStdout()), 0, 0);
   disconnect( crysol, SIGNAL(readyReadStderr()), 0, 0);
   disconnect( crysol, SIGNAL(processExited()), 0, 0);

   // crysol creates 4 files:
   // crysol_summary.txt
   // pdb##.alm
   // pdb##.log
   // pdb##.int

   // we just want the .int, the rest will be removed if needed

   QString created_dat = crysol_last_pdb_base.replace(QRegExp("\\.(pdb|PDB)$"),"") +  "00.int";

   if ( !QFile::exists( created_dat ) )
   {
      editor_msg("red", QString(tr("Error: Crysol did not create file %1")).arg( created_dat ));
      pb_plot_saxs_sans->setEnabled(true);
      return;
   }

   // now move the file to the saxs directory
   QString new_created_dat = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
      QFileInfo( crysol_last_pdb.replace(QRegExp("\\.(pdb|PDB)$"),"") ).fileName() + iqq_suffix() + ".int";

   if ( QFile::exists(new_created_dat) )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->overwrite )
      {
         editor_msg("red", QString(tr("Notice: overwriting %1")).arg( new_created_dat ));
         // windows requires removing previous file
      } else {
         new_created_dat = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( new_created_dat );
      }
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( !qd.rename( created_dat, new_created_dat ) )
   {
      editor_msg("red", 
                 QString(tr("Notice: could not rename %1 to %1"))
                 .arg( created_dat )
                 .arg( new_created_dat )
                 );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   load_saxs( new_created_dat );

   pb_plot_saxs_sans->setEnabled(true);
   editor->append("Crysol finished.\n");
}
   
void US_Hydrodyn_Saxs::crysol_launchFinished()
{
   editor_msg("brown", "Crysol launch exited\n");
   disconnect( crysol, SIGNAL(launchFinished()), 0, 0);
}

// -------------------- cryson ------------------------------

int US_Hydrodyn_Saxs::run_sans_iq_cryson( QString /* pdb */ )
{
   return -1;
}

void US_Hydrodyn_Saxs::cryson_readFromStdout()
{
   while ( cryson->canReadLineStdout() )
   {
      editor_msg("brown", cryson->readLineStdout() + "\n");
   }
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::cryson_readFromStderr()
{
   while ( cryson->canReadLineStderr() )
   {
      editor_msg("red", cryson->readLineStderr() + "\n");
   }
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::cryson_processExited()
{
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   cryson_readFromStderr();
   cryson_readFromStdout();
      //   }
   disconnect( cryson, SIGNAL(readyReadStdout()), 0, 0);
   disconnect( cryson, SIGNAL(readyReadStderr()), 0, 0);
   disconnect( cryson, SIGNAL(processExited()), 0, 0);
   editor->append("Cryson finished.\n");
}
   
void US_Hydrodyn_Saxs::cryson_launchFinished()
{
   editor_msg("brown", "Cryson launch exited\n");
   disconnect( cryson, SIGNAL(launchFinished()), 0, 0);
}

