#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_cmdline_app.h"
#include <qregexp.h>
//Added by qt3to4:
#include <Q3TextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

void US_Hydrodyn_Saxs::editor_msg( QColor color, QColor bgcolor, QString msg )
{
   msg.replace( QRegExp( "\n*$" ), "" );
   msg += "\n";
   // QColor save_color_bg = editor->paragraphBackgroundColor( editor->paragraphs() - 1 );
   editor->setParagraphBackgroundColor( editor->paragraphs() - 1,  bgcolor );
   editor_msg( color, msg );
   editor->setParagraphBackgroundColor( editor->paragraphs() - 1,  "white" );
}

void US_Hydrodyn_Saxs::editor_msg( QColor color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
}

void US_Hydrodyn_Saxs::editor_msg( QString color, QString msg )
{
   editor_msg( QColor( color ), msg );
}

void US_Hydrodyn_Saxs::editor_msg( const char * color, QString msg )
{
   editor_msg( QString( color ), msg );
}

void US_Hydrodyn_Saxs::editor_msg( QString color, QColor bgcolor, QString msg )
{
   editor_msg( QColor( color ), bgcolor, msg );
}

void US_Hydrodyn_Saxs::editor_msg( const char *color, QColor bgcolor, QString msg )
{
   editor_msg( QColor( color ), bgcolor, msg );
}

void US_Hydrodyn_Saxs::editor_msg( QColor color, QString bgcolor, QString msg )
{
   editor_msg( color, QColor( bgcolor ), msg );
}

void US_Hydrodyn_Saxs::editor_msg( QColor color, const char * bgcolor, QString msg )
{
   editor_msg( color, QColor( bgcolor ), msg );
}

void US_Hydrodyn_Saxs::editor_msg( QString color, QString bgcolor, QString msg )
{
   editor_msg( QColor( color ), QColor( bgcolor ), msg );
}

void US_Hydrodyn_Saxs::editor_msg( const char * color, const char * bgcolor, QString msg )
{
   editor_msg( QColor( color ), QColor( bgcolor ), msg );
}


// -------------------- FoXS ------------------------------

int US_Hydrodyn_Saxs::run_saxs_iq_foxs( QString pdb )
{
   QString prog = 
      USglobal->config_list.system_dir + SLASH +
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
      QFileInfo qfi(prog);
      if ( !qfi.exists() )
      {
         editor_msg( (QString) "red", QString("FoXS program '%1' does not exist\n").arg(prog));
         return -1;
      }
      if ( !qfi.isExecutable() )
      {
         editor_msg( "red", QString("FoXS program '%1' is not executable\n").arg(prog));
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

   foxs = new Q3Process( this );
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

   editor_msg( "black", "\nStarting FoXS\n");
   foxs->start();
   external_running = true;

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
      external_running = false;
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
         new_created_dat = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( new_created_dat, 0, this );
      }
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( !qd.rename( created_dat, new_created_dat ) )
   {
      editor_msg("red", 
                 QString(tr("Notice: could not rename %1 to %2"))
                 .arg( created_dat )
                 .arg( new_created_dat )
                 );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   load_saxs( new_created_dat );
   pb_plot_saxs_sans->setEnabled(true);
   external_running = false;
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
      "bin"
#endif
      + SLASH
      + "crysol" 
#if defined(WIN32)
      + ".exe"
#endif      
      ;

   crysol_stdout.clear();
   crysol_stderr.clear();

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
      editor_msg("red", QString("Crysol called but the directory '%1' does not exist\n").arg(qd_dir.path()));
      return -1;
   }

   if ( !qd_dir.isReadable() )
   {
      editor_msg("red", QString("Crysol called but the directory '%1' is not readable. Check permissions\n").arg(qd_dir.path()));
      return -1;
   }

   QString uniqstr = QDateTime::currentDateTime().toString( "yyyyMMddhhmmsszzz" );
   QDir qd_dir2( dir + QDir::separator() + uniqstr );
   if ( qd_dir2.exists() )
   {
      unsigned int ext = 1;
      do {
         qd_dir2.setPath( dir + QDir::separator() + uniqstr + QString( "-%1" ).arg( ext++ ) );
      } while ( qd_dir2.exists() );
   }

   if ( !qd_dir2.mkdir( qd_dir2.path() ) )
   {
      editor_msg("red", QString("Crysol called but could not create the temporary directory '%1'. Check permissions\n").arg(qd_dir2.path()));
      return -1;
   }
   if ( !qd_dir2.exists() )
   {
      editor_msg("red", QString("Crysol called but the directory '%1' does not exist\n").arg(qd_dir2.path()));
      return -1;
   }

   if ( !qd_dir2.isReadable() )
   {
      editor_msg("red", QString("Crysol called but the directory '%1' is not readable. Check permissions\n").arg(qd_dir2.path()));
      return -1;
   }

   dir = qd_dir2.path();

   crysol_last_pdb = pdb;
   if ( our_saxs_options->crysol_version_26 )
   {
      crysol_last_pdb_base = dir + SLASH + QFileInfo(crysol_last_pdb).fileName().replace(QRegExp("\\.(pdb|PDB)$"),"").left(6) + ".pdb";
   } else {
      crysol_last_pdb_base = dir + SLASH + QFileInfo(crysol_last_pdb).fileName().replace(QRegExp("\\.(pdb|PDB)$"),"") + ".pdb";
   }
      
   QString use_pdb = pdb;
   
   // always copy pdb
   //   if (  our_saxs_options->crysol_version_26 || 
   //        QFileInfo(crysol_last_pdb).fileName() != QFileInfo(crysol_last_pdb_base).fileName() )
   {
      QFile f( pdb );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         editor_msg("red", QString("Could not open file %1. Check permissions\n").arg( pdb ));
         return -1;
      }

      QFile f2( crysol_last_pdb_base );
      if ( !f2.open( QIODevice::WriteOnly ) )
      {
         editor_msg("red", QString("Could not open file %1. Check permissions\n").arg( crysol_last_pdb_base ));
         return -1;
      }

      QString qs;
      Q3TextStream ts( &f );
      Q3TextStream ts2( &f2 );

      if ( selected_models[ 0 ] != 0 )
      {
         QRegExp rx_model( "^MODEL\\s+(\\S+)(\\s+|$)" );
         
         bool found_model = false;
         while ( !ts.atEnd() )
         {
            qs = ts.readLine();
            if ( rx_model.search( qs ) != -1 )
            {
               if ( rx_model.cap( 1 ).toUInt() == selected_models[ 0 ] + 1 )
               {
                  found_model = true;
               }
            }
            if ( found_model )
            {
               ts2 << qs << endl;
               if ( qs.left( 6 ) == "ENDMDL" )
               {
                  break;
               }
            }
         }                  
      } else {
         while ( !ts.atEnd() )
         {
            qs = ts.readLine();
            ts2 << qs << endl;
            if ( qs.left( 6 ) == "ENDMDL" )
            {
               break;
            }
         }
      }
      f.close();
      f2.close();
      use_pdb = crysol_last_pdb_base;
   }
   //else {
   //      use_pdb = pdb;
   //   }

   cout << "use_pdb: <" << use_pdb << ">\n";
   cout << "crysol_last_pdb_base: <" << crysol_last_pdb_base << ">\n";

   editor_msg( "dark blue", QString( "Crysol: use pdb [%1]" ).arg( use_pdb ) );
   editor_msg( "dark blue", QString( "Crysol: last pdb base [%1]" ).arg( crysol_last_pdb_base ) );
   editor_msg( "dark blue", QString( "Crysol: working dir [%1]" ).arg( dir ) );

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

      to_remove = base + "00.fit";
      QFile::remove( to_remove );
   }      

   pb_plot_saxs_sans->setEnabled(false);


   crysol_manual_mode = false;
   crysol_manual_input.clear();

   if ( U_EXPT &&
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_crysol_target" ) &&
        !( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ].isEmpty() )
   {
      if ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_ra" ) &&
           (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ].toDouble() > 0e0 )
      {
         editor_msg( "dark red", QString( tr( "Note: Manual average atomic radius %1 (A)" ) )
                     .arg( (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ] ) );
         crysol_manual_mode = true;
      }
      if ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_vol" ) &&
           (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ].toDouble() >  0e0 )
      {
         editor_msg( "dark red", QString( tr( "Note: Manual excluded volume %1 (A^3)" ) )
                     .arg( (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ] ) );
         crysol_manual_mode = true;
      }
   }

   if ( crysol_manual_mode )
   {
      editor_msg( "dark red", QString( tr( "Note: crysol running in interactive mode" ) ) );
#if defined( UHSE_APP_RESPONSE_WAY )
      crysol_app_text .clear();
      crysol_response .clear();

      crysol_app_text << "Enter your option ...................... <";
      crysol_response << "0";
      crysol_app_text << "Brookhaven file name ................... <";
      crysol_response << ( our_saxs_options->crysol_version_26 ? QFileInfo(use_pdb).fileName() : use_pdb );
      crysol_app_text << "Maximum order of  harmonics ........... <";
      crysol_response << QString("%1").arg( our_saxs_options->sh_max_harmonics );
      crysol_app_text << "Order of Fibonacci grid ............... <";
      crysol_response << QString("%1").arg( our_saxs_options->sh_fibonacci_grid_order );
      crysol_app_text << "Maximum s value ........................ <";
      crysol_response << QString("%1").arg( our_saxs_options->end_q );
      crysol_app_text << "Number of points ....................... <";
      crysol_response << QString("%1").arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q) );
      crysol_app_text << "Account for explicit hydrogens? [ Y / N ] <";
      crysol_response << ( our_saxs_options->crysol_explicit_hydrogens ? "Y" : "N" );
      crysol_app_text << "Fit the experimental curve [ Y / N ] .. <";
      crysol_response << "Y";
      crysol_app_text << "Enter data file ........................ <";
      crysol_response << ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ];
      crysol_app_text << "Subtract constant ...................... <";
      crysol_response << "N";
      crysol_app_text << "sin(theta)/lambda [1/nm]  (4) ..... <";
      crysol_response << "1";
      crysol_app_text << "Electron density of the solvent, e/A**3  <";
      crysol_response << QString("%1").arg( our_saxs_options->water_e_density, 0, 'f', 4 );
      crysol_app_text << "Plot the fit [ Y / N ] ................. <";
      crysol_response << "N";
      crysol_app_text << "Another set of parameters [ Y / N ] .... <";
      crysol_response << "Y";
      crysol_app_text << "Minimize again with new limits [ Y / N ] <";
      crysol_response << "N";
      crysol_app_text << "Contrast of the solvation shell ........ <";
      crysol_response << QString("%1").arg( our_saxs_options->crysol_hydration_shell_contrast, 0, 'f', 4 ); // hydration shell contrast
      crysol_app_text << "Average atomic radius .................. <";
      crysol_response << 
         QString( ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_ra" ) &&
                    (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ].toDouble() > 0e0 ) ?
                  (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ] : ""  )
         ;
      crysol_app_text << "Excluded volume ........................ <";
      crysol_response << 
         QString( ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_vol" ) &&
                    (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ].toDouble() > 0e0 ) ?
                  (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ] : "" )
         ;
      crysol_app_text << "Plot the fit [ Y / N ] ................. <";
      crysol_response << "N";
      crysol_app_text << "Another set of parameters [ Y / N ] .... <";
      crysol_response << "N";
      crysol_app_text << "Intensities    saved to file";
      crysol_response << "___run___";
#endif

      crysol_manual_input << ""; // option
      crysol_manual_input << ( our_saxs_options->crysol_version_26 ? QFileInfo(use_pdb).fileName() : use_pdb );
      crysol_manual_input << QString("%1").arg( our_saxs_options->sh_max_harmonics );
      crysol_manual_input << QString("%1").arg( our_saxs_options->sh_fibonacci_grid_order );
      crysol_manual_input << QString("%1").arg( our_saxs_options->end_q );
      crysol_manual_input << QString("%1").arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q) );
      crysol_manual_input << ( our_saxs_options->crysol_explicit_hydrogens ? "Y" : "N" );
      crysol_manual_input << "Y"; // fit expt curve
      crysol_manual_input << QFileInfo( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ] ).fileName();
      crysol_manual_input << "N"; // subtract constant
      crysol_manual_input << "1"; // angular units
      crysol_manual_input << QString("%1").arg( our_saxs_options->water_e_density, 0, 'f', 4 ) ;
      crysol_manual_input << "N"; // plot fit
      crysol_manual_input << "Y"; // another set
      crysol_manual_input << "N"; // minimize again
      crysol_manual_input << QString("%1").arg( our_saxs_options->crysol_hydration_shell_contrast, 0, 'f', 4 ); // hydration shell contrast
      crysol_manual_input << 
         QString( ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_ra" ) &&
                    (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ].toDouble() > 0e0 ) ?
                  (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ] : ""  )
         ;
      crysol_manual_input << 
         QString( ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_vol" ) &&
                    (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ].toDouble() > 0e0 ) ?
                  (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ] : "" )
         ;
      crysol_manual_input << "N";
      crysol_manual_input << "N";
      // qDebug( "crysol input\n------\n" + crysol_manual_input.join( "\n" ) + "\n----" );

      // create input log file, run system command into output 
      {
         {
            QFile f( dir + QDir::separator() + "input" );
            if ( !f.open( QIODevice::WriteOnly ) )
            {
               editor_msg( "red", QString( tr( "Error: trying to create input file %1" ) ).arg( f.name() ) );
               pb_plot_saxs_sans->setEnabled(true);
               return -1;
            }
            Q3TextStream ts( &f );
            ts << crysol_manual_input.join( "\n" ) << endl;
            f.close();
         }

         {
            US_File_Util ufu;
            if ( !ufu.copy( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ], dir ) )
            {
               editor_msg( "red", ufu.errormsg );
               pb_plot_saxs_sans->setEnabled(true);
               return -1;
            }
         }

         QString cmd = QString( "\"%1\" < input > output" ).arg( prog );
#if defined( WIN32 )
         {
            QFile f( dir + QDir::separator() + "run.bat" );
            if ( !f.open( QIODevice::WriteOnly ) )
            {
               editor_msg( "red", QString( tr( "Error: trying to create batch file %1" ) ).arg( f.name() ) );
               pb_plot_saxs_sans->setEnabled(true);
               return -1;
            }
            Q3TextStream ts( &f );
            ts << cmd << endl;
            f.close();

            crysol = new Q3Process( this );
            crysol->setWorkingDirectory( dir );
            crysol->addArgument( f.name() );
         }

// attempt to create job with no "cmd" box
//          {
//             PROCESS_INFORMATION processInformation = {0};
//             STARTUPINFO startupInfo                = {0};
//             startupInfo.cb                         = sizeof(startupInfo);
 
//             wchar_t wtext[ 2048 ];
//             mbstowcs( wtext, cmd.ascii(), strlen( cmd.ascii() ) + 1 ); //Plus null

//             // Create the process
//             BOOL result = CreateProcess(NULL, wtext,
//                                         NULL, NULL, FALSE, 
//                                         NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, 
//                                         NULL, NULL, &startupInfo, &processInformation);


//             qDebug( "result: " + result );

//             // STARTUPINFOW si;
//             // PROCESS_INFORMATION pi;

//             // ZeroMemory(&si, sizeof(si));
//             // si.cb = sizeof(si);
//             // ZeroMemory(&pi, sizeof(pi));

//             // wchar_t wtext[ 1024 ];
//             // mbstowcs(wtext, cmd.ascii(), strlen(cmd.ascii())+1);//Plus null

//             // if (CreateProcessW( NULL, wtext, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
//             // {
//             //    WaitForSingleObject(pi.hProcess, INFINITE);
//             //    CloseHandle(pi.hProcess);
//             //    CloseHandle(pi.hThread);
//             // }
//          }

#else
         editor_msg( "blue", "\nStarting Crysol\n" + cmd );
         qApp->processEvents();
         QString savedir = QDir::currentDirPath();
         QDir::setCurrent( dir );
         system( cmd );
         QDir::setCurrent( savedir );
         {
            QFile f( dir + QDir::separator() + "output" );
            if ( !f.open( QIODevice::ReadOnly ) )
            {
               editor_msg( "red", QString( tr( "Error: trying to read output file %1" ) ).arg( f.name() ) );
               pb_plot_saxs_sans->setEnabled(true);
               return -1;
            }
            Q3TextStream ts( &f );
            while ( !ts.atEnd() )
            {
               crysol_stdout << ts.readLine();
            }
            f.close();
            editor_msg( "brown", crysol_stdout.join( "\n" ) );
         }
         crysol_finishup();
         return 0;
#endif
      }
   } else {
      crysol = new Q3Process( this );
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
      crysol->addArgument( QString("%1").arg( our_saxs_options->sh_max_harmonics ) );
      
      crysol->addArgument( "/fb" );
      crysol->addArgument( QString("%1").arg( our_saxs_options->sh_fibonacci_grid_order ) );

      if ( our_saxs_options->crysol_explicit_hydrogens )
      {
         crysol->addArgument( "/eh" );
      }

      if ( U_EXPT &&
           ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_crysol_target" ) &&
           !( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ].isEmpty() )
      {
         US_File_Util ufu;
         if ( !ufu.copy( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ], dir ) )
         {
            editor_msg( "red", ufu.errormsg );
            if ( ufu.errormsg.contains( QRegExp( "exists$" ) ) )
            {
               crysol->addArgument( QFileInfo( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ] ).fileName() );
            }
         } else {
            crysol->addArgument( QFileInfo( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ] ).fileName() );
         }
      }

   }

   connect( crysol, SIGNAL(readyReadStdout()), this, SLOT(crysol_readFromStdout()) );
   connect( crysol, SIGNAL(readyReadStderr()), this, SLOT(crysol_readFromStderr()) );
   connect( crysol, SIGNAL(processExited()), this, SLOT(crysol_processExited()) );

#if defined( UHSE_APP_RESPONSE_WAY )
   if ( crysol_manual_mode )
   {
      connect( &crysol_timer,  SIGNAL( timeout()         ), this, SLOT( crysol_timeout()        ) );
      crysol_query_response_pos = 0;
      crysol_run_to_end         = false;
      crysol_timer_delay_ms     = 240000;
   }
#endif
   
   editor->append("\n\nStarting Crysol\n");
   
   editor_msg( "dark blue", crysol->arguments().join( " " ) );
   crysol->start();
   external_running = true;
   return 0;
}

void US_Hydrodyn_Saxs::crysol_timeout()
{
   editor_msg( "red", tr( "Error: out of responses to queries (timeout)\n" ) );
   // qDebug( "timeout" );
   crysol->kill();
}

void US_Hydrodyn_Saxs::crysol_readFromStdout()
{
   while ( crysol->canReadLineStdout() )
   {
      QString qs = crysol->readLineStdout();
      crysol_stdout << qs;
      editor_msg("brown", qs );
   }

#if defined( UHSE_APP_RESPONSE_WAY )
   if ( !crysol_manual_mode )
   {
      while ( crysol->canReadLineStdout() )
      {
         QString qs = crysol->readLineStdout();
         crysol_stdout << qs;
         editor_msg("brown", qs );
      }
      return;
   }
   crysol_timer.stop();

   // qDebug( "readFromStdout()" );
   QString qs;
   QString text;
   //   do {
   // while ( crysol->canReadLineStdout() )
   // {
   //    qs = crysol->readLineStdout();
   //    crysol_stdout << qs;
   //    editor_msg("brown", qs );
   //    text += qs;
   // };
      //   } while ( qs != QString::null );

   //do {
      QString read = crysol->readStdout();
      qs = QString( "%1" ).arg( read );
      crysol_stdout << qs;
      editor_msg("brown", qs );
      text += qs;
      // } while ( qs.length() );
   
      // qDebug( QString( "received <%1>" ).arg( text ) );

   if ( !crysol_run_to_end && crysol_app_text.size() )
   {
      // if not at first entry, read data to find match
      int previous_pos = crysol_query_response_pos;
      while ( ( int ) crysol_app_text.size() > crysol_query_response_pos &&
              !text.contains( crysol_app_text[ crysol_query_response_pos ] ) && 
              crysol_query_response_pos )
      {
         crysol_query_response_pos++;
      }
      if ( crysol_query_response_pos >= ( int ) crysol_app_text.size() )
      {
         crysol_query_response_pos = previous_pos;
         if ( crysol_timer_delay_ms )
         {
            // qDebug( QString( "starting timer for %1 seconds" ).arg( ( double )crysol_timer_delay_ms / 1000e0 ) );
            crysol_timer.start( crysol_timer_delay_ms );
         } else {
            // qDebug( tr( "Error: out of responses to queries" ) );
            crysol->kill();
         }
         return;
      }         

      // do we have a match?
      if ( ( int ) crysol_app_text.size() > crysol_query_response_pos &&
           text.contains( crysol_app_text[ crysol_query_response_pos ] ) )
      {
         // qDebug( QString( "received <%1> from application" ).arg( crysol_app_text[ crysol_query_response_pos ] ) );
         if ( crysol_response[ crysol_query_response_pos ] != "___run___" )
         {
            if ( crysol_response[ crysol_query_response_pos ].left( 2 ).contains( "__" ) )
            {
               // qDebug(  
               //        QString( tr( "Error: undefined response <%1> to query <%2>" ) )
               //        .arg( crysol_response[ crysol_query_response_pos ] )
               //        .arg( crysol_app_text[ crysol_query_response_pos ] ) )
               //    ;
               crysol->kill();
               return;
            }
            // qDebug( QString( "sent     <%1> to application"   ).arg( crysol_response[ crysol_query_response_pos ] ) );
            crysol->writeToStdin( crysol_response[ crysol_query_response_pos ] + "\n" );
            crysol_query_response_pos++;
         } else {
            // qDebug( "now run to end of application" );
            crysol_run_to_end = true;
         }
      }
   }
#endif
}
   
void US_Hydrodyn_Saxs::crysol_readFromStderr()
{
   while ( crysol->canReadLineStderr() )
   {
      QString qs = crysol->readLineStderr();
      crysol_stderr << qs;
      editor_msg("red", qs + "\n");
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

   if ( crysol_manual_mode )
   {
      QFile f( crysol->workingDirectory().dirName() + QDir::separator() + "output" );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         editor_msg( "red", QString( tr( "Error: trying to read output file %1" ) ).arg( f.name() ) );
         pb_plot_saxs_sans->setEnabled(true);
         return;
      }
      Q3TextStream ts( &f );
      while ( !ts.atEnd() )
      {
         crysol_stdout << ts.readLine();
      }
      f.close();
      editor_msg( "brown", crysol_stdout.join( "\n" ) );
   }

   // qDebug( "crysolstdout: " + crysol_stdout.join("\n") );
   // qDebug( "crysolstderr: " + crysol_stderr.join("\n") );

   // crysol creates 4 files:
   // crysol_summary.txt
   // pdb##.alm
   // pdb##.log
   // pdb##.int

   // we just want the .int, the rest will be removed if needed

   // crysol
   delete crysol;
   crysol_finishup();
}

void US_Hydrodyn_Saxs::crysol_finishup()
{
   QStringList intensity_lines = crysol_stdout.grep( QRegExp( "Intensities\\s+saved to file" ) );
   QStringList fit_lines       = crysol_stdout.grep( QRegExp( "Data fit\\s+saved to file" ) );

   QString new_intensity_file;
   QString new_fit_file;

   if ( intensity_lines.size() == 1 )
   {
      QRegExp rx( "Intensities\\s+saved to file (\\S+)" );
      if ( rx.search( intensity_lines[ 0 ] ) != -1 )
      {
         new_intensity_file = rx.cap( 1 );
      }
   }
   if ( fit_lines.size() == 1 )
   {
      QRegExp rx( "Data fit\\s+saved to file (\\S+)" );
      if ( rx.search( fit_lines[ 0 ] ) != -1 )
      {
         new_fit_file = rx.cap( 1 );
      }
   }

   // qDebug( "intensity_file: " + new_intensity_file );
   // qDebug( "fit_file: " + new_fit_file );

   QString type = ".int";
   if ( U_EXPT &&
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_crysol_target" ) &&
        !( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ].isEmpty() )
   {
      type = ".fit";
   }

   QString created_dat = crysol_last_pdb_base.replace(QRegExp("\\.(pdb|PDB)$"),"") +  "00" + type;

   // qDebug( "created_dat: " + created_dat );

   if ( !new_intensity_file.isEmpty() )
   {
      created_dat = QFileInfo( created_dat ).dirPath() + QDir::separator() + new_intensity_file;
   }
   if ( !new_fit_file.isEmpty() )
   {
      created_dat = QFileInfo( created_dat ).dirPath() + QDir::separator() + new_fit_file;
   }

   // qDebug( "created_dat after rplc: " + created_dat );

   if ( !QFile::exists( created_dat ) )
   {
      editor_msg("red", QString(tr("Error: Crysol did not create file %1")).arg( created_dat ));
      pb_plot_saxs_sans->setEnabled(true);
      external_running = false;
      return;
   }

   // now move the file to the saxs directory
   QString new_created_dat = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
      QFileInfo( crysol_last_pdb.replace(QRegExp("\\.(pdb|PDB)$"),"") ).fileName() + 
      ( selected_models[ 0 ] == 0 ? QString( "" ) : QString( "_m%1" ).arg( selected_models[ 0 ] + 1 ) )
      + iqq_suffix() + type;

   if ( QFile::exists(new_created_dat) )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->overwrite )
      {
         editor_msg("red", QString(tr("Notice: overwriting %1")).arg( new_created_dat ));
         // windows requires removing previous file
      } else {
         new_created_dat = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( new_created_dat, 0, this );
      }
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( !qd.rename( created_dat, new_created_dat ) )
   {
      editor_msg("red", 
                 QString(tr("Notice: could not rename %1 to %2"))
                 .arg( created_dat )
                 .arg( new_created_dat )
                 );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   load_saxs( new_created_dat );

   pb_plot_saxs_sans->setEnabled( true );
   editor->append("Crysol finished.\n");
   external_running = false;
}
   
void US_Hydrodyn_Saxs::crysol_wroteToStdin()
{
   // qDebug( "Crysol wroteToStdin" );
}

void US_Hydrodyn_Saxs::crysol_launchFinished()
{
   editor_msg("brown", "Crysol launch exited\n");
   disconnect( crysol, SIGNAL(launchFinished()), 0, 0);
   // qDebug( "crysol launchFinished" );
}

// -------------------- cryson ------------------------------

int US_Hydrodyn_Saxs::run_sans_iq_cryson( QString pdb )
{
   QString prog = 
      USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "/bin/"
#endif
      + SLASH
      + "cryson" 
#if defined(WIN32)
      + ".exe"
#endif      
      ;

   {
      QFileInfo qfi(prog);
      if ( !qfi.exists() )
      {
         editor_msg("red", QString("Cryson program '%1' does not exist\n").arg(prog));
         return -1;
      }
      if ( !qfi.isExecutable() )
      {
         editor_msg("red", QString("Cryson program '%1' is not executable\n").arg(prog));
         return -1;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      editor_msg("red", QString("Cryson called but PDB file '%1' does not exist\n").arg(pdb));
      return -1;
   }

   QString dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "tmp";

   QDir qd_dir( dir );

   if ( !qd_dir.exists() )
   {
      editor_msg("red", QString("Cryson called but the directory '%1' does not exist\n").arg(qd_dir.path()));
      return -1;
   }

   if ( !qd_dir.isReadable() )
   {
      editor_msg("red", QString("Cryson called but the directory '%1' is not readable. Check permissions\n").arg(qd_dir.path()));
      return -1;
   }

   cryson_last_pdb = pdb;
   cryson_last_pdb_base = dir + SLASH + QFileInfo(cryson_last_pdb).fileName().replace(QRegExp("\\.(pdb|PDB)$"),"").left(6) + ".pdb";
   QString use_pdb = pdb;
   
   // copy pdb if the name is too long
   if ( our_saxs_options->crysol_version_26 &&
        QFileInfo(cryson_last_pdb).fileName() != QFileInfo(cryson_last_pdb_base).fileName() )
   {
      QFile f( pdb );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         editor_msg("red", QString("Could not open file %1. Check permissions\n").arg( pdb ));
         return -1;
      }

      QFile f2( cryson_last_pdb_base );
      if ( !f2.open( QIODevice::WriteOnly ) )
      {
         editor_msg("red", QString("Could not open file %1. Check permissions\n").arg( cryson_last_pdb_base ));
         return -1;
      }

      QString qs;
      Q3TextStream ts( &f );
      Q3TextStream ts2( &f2 );

      while ( !ts.atEnd() )
      {
         qs = ts.readLine();
         ts2 << qs << endl;
      }
      f.close();
      f2.close();
      use_pdb = cryson_last_pdb_base;
   } else {
      use_pdb = pdb;
   }

   cout << "use_pdb: <" << use_pdb << ">\n";
   cout << "cryson_last_pdb_base: <" << cryson_last_pdb_base << ">\n";

   // clean up so we have new files

   // {
   //    QString base = cryson_last_pdb_base.replace(QRegExp("\\.(pdb|PDB)$"),"");
   //    cout << "base: <" << base << ">\n";

   //    QString to_remove = base + "00.alm";
   //    cout << "to_remove: <" << to_remove << ">\n";
   //    QFile::remove( to_remove );

   //    to_remove = base + "00.log";
   //    QFile::remove( to_remove );

   //    to_remove = base + "00.int";
   //    QFile::remove( to_remove );
   // }      

   pb_plot_saxs_sans->setEnabled(false);

   cryson = new Q3Process( this );
   cryson->setWorkingDirectory( dir );
   cryson->addArgument( prog );
   cryson->addArgument( our_saxs_options->crysol_version_26 ? QFileInfo(use_pdb).fileName() : use_pdb );

   cryson->addArgument( "/sm" );
   cryson->addArgument( QString("%1").arg( our_saxs_options->end_q ) );

   cryson->addArgument( "/ns" );
   cryson->addArgument( QString("%1").arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q)) );

   cryson->addArgument( "/D2O" );
   cryson->addArgument( QString("%1").arg( our_saxs_options->d2o_conc ) );

   if ( our_saxs_options->cryson_manual_hs )
   {
      cryson->addArgument( "/dro" );
      cryson->addArgument( QString("%1").arg( our_saxs_options->cryson_hydration_shell_contrast ) );
   }

   if ( ((US_Hydrodyn *)us_hydrodyn)->gparams.count( "perdeuteration" ) )
   {
      cryson->addArgument( "/per" );
      cryson->addArgument( ((US_Hydrodyn *)us_hydrodyn)->gparams[ "perdeuteration" ] );
   }

   cryson->addArgument( "/lm" );
   cryson->addArgument( QString("%1").arg( our_saxs_options->cryson_sh_max_harmonics ) );

   cryson->addArgument( "/fb" );
   cryson->addArgument( QString("%1").arg( our_saxs_options->cryson_sh_fibonacci_grid_order ) );

   //    if ( our_saxs_options->cryson_explicit_hydrogens )
   //    {
   //       cryson->addArgument( "/eh" );
   //    }

   connect( cryson, SIGNAL(readyReadStdout()), this, SLOT(cryson_readFromStdout()) );
   connect( cryson, SIGNAL(readyReadStderr()), this, SLOT(cryson_readFromStderr()) );
   connect( cryson, SIGNAL(processExited()), this, SLOT(cryson_processExited()) );
   connect( cryson, SIGNAL(launchFinished()), this, SLOT(cryson_launchFinished()) );

   editor->append("\n\nStarting Cryson\n");
   cryson->start();
   external_running = true;

   return 0;
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

   // cryson creates 4 files:
   // cryson_summary.txt
   // pdb##.alm
   // pdb##.log
   // pdb##.int

   // we just want the .int, the rest will be removed if needed

   QString created_dat = cryson_last_pdb_base.replace(QRegExp("\\.(pdb|PDB)$"),"") +  "00.int";

   if ( !QFile::exists( created_dat ) )
   {
      editor_msg("red", QString(tr("Error: Cryson did not create file %1")).arg( created_dat ));
      pb_plot_saxs_sans->setEnabled(true);
      external_running = false;
      return;
   }

   // now move the file to the saxs directory
   QString new_created_dat = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
      QFileInfo( cryson_last_pdb.replace(QRegExp("\\.(pdb|PDB)$"),"") ).fileName() + iqq_suffix() + ".int";

   if ( QFile::exists(new_created_dat) )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->overwrite )
      {
         editor_msg("red", QString(tr("Notice: overwriting %1")).arg( new_created_dat ));
         // windows requires removing previous file
      } else {
         new_created_dat = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( new_created_dat, 0, this );
      }
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( !qd.rename( created_dat, new_created_dat ) )
   {
      editor_msg("red", 
                 QString(tr("Notice: could not rename %1 to %2"))
                 .arg( created_dat )
                 .arg( new_created_dat )
                 );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   load_saxs( new_created_dat );

   pb_plot_saxs_sans->setEnabled( true );
   editor->append("Cryson finished.\n");
   external_running = false;
}
   
void US_Hydrodyn_Saxs::cryson_launchFinished()
{
   editor_msg("brown", "Cryson launch exited\n");
   disconnect( cryson, SIGNAL(launchFinished()), 0, 0);
}

// -------------------- Sastbx ------------------------------

int US_Hydrodyn_Saxs::run_saxs_iq_sastbx( QString pdb )
{
   QString prog = 
      USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + "sastbx.she" 
#if defined(WIN32)
      + ".exe"
#endif      
      ;

   {
      QFileInfo qfi(prog);
      if ( !qfi.exists() )
      {
         editor_msg("red", QString("Sastbx program '%1' does not exist\n").arg(prog));
         return -1;
      }
      if ( !qfi.isExecutable() )
      {
         editor_msg("red", QString("Sastbx program '%1' is not executable\n").arg(prog));
         return -1;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      editor_msg("red", QString("Sastbx called but PDB file '%1' does not exist\n").arg(pdb));
      return -1;
   }

   pb_plot_saxs_sans->setEnabled(false);

   sastbx_last_pdb = pdb;

   sastbx = new Q3Process( this );
   //   sastbx->setWorkingDirectory( dir );
   sastbx->addArgument( prog );

   sastbx->addArgument( QString( "structure=%1" ).arg( pdb ) );
   
   QString method;
   switch ( our_saxs_options->sastbx_method )
   {
   case 1:
      method = "debye";
      break;
   case 2:
      method = "zernike";
      break;
   case 0:
   default:
      method = "she";
      break;
   }

   sastbx->addArgument( QString( "method=%1"    ).arg( method ) );
   sastbx->addArgument( QString( "q_start=%1"   ).arg( our_saxs_options->start_q ) );
   sastbx->addArgument( QString( "q_stop=%1"    ).arg( our_saxs_options->end_q ) );
   sastbx->addArgument( QString( "n_step=%1"    ).arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q)) );
   sastbx->addArgument( QString( "rho=%1"       ).arg( our_saxs_options->water_e_density ) );
   sastbx->addArgument( QString( "drho=%1"      ).arg( our_saxs_options->crysol_hydration_shell_contrast ) );
   sastbx->addArgument( QString( "max_i =%1"    ).arg( our_saxs_options->sh_fibonacci_grid_order ) );
   sastbx->addArgument( QString( "max_L =%1"    ).arg( our_saxs_options->sh_max_harmonics ) );
   sastbx->addArgument( QString( "output=%1"    ).arg( sastbx_last_pdb + ".int" ) );

   connect( sastbx, SIGNAL(readyReadStdout()), this, SLOT(sastbx_readFromStdout()) );
   connect( sastbx, SIGNAL(readyReadStderr()), this, SLOT(sastbx_readFromStderr()) );
   connect( sastbx, SIGNAL(processExited()), this, SLOT(sastbx_processExited()) );
   connect( sastbx, SIGNAL(launchFinished()), this, SLOT(sastbx_launchFinished()) );

   editor->append("\n\nStarting Sastbx\n");
   sastbx->start();
   external_running = true;

   return 0;
}

void US_Hydrodyn_Saxs::sastbx_readFromStdout()
{
   while ( sastbx->canReadLineStdout() )
   {
      editor_msg("brown", sastbx->readLineStdout() + "\n");
   }
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::sastbx_readFromStderr()
{
   while ( sastbx->canReadLineStderr() )
   {
      editor_msg("red", sastbx->readLineStderr() + "\n");
   }
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::sastbx_processExited()
{
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   sastbx_readFromStderr();
   sastbx_readFromStdout();
      //   }
   disconnect( sastbx, SIGNAL(readyReadStdout()), 0, 0);
   disconnect( sastbx, SIGNAL(readyReadStderr()), 0, 0);
   disconnect( sastbx, SIGNAL(processExited()), 0, 0);
   editor->append("Sastbx finished.\n");

   // sastbx creates 2 files:
   // pdb_pdb.dat
   // pdb.dat

   QString created_dat = sastbx_last_pdb + ".int";

   if ( !QFile::exists( created_dat ) )
   {
      editor_msg("red", QString(tr("Error: Sastbx did not create file %1")).arg( created_dat ));
      pb_plot_saxs_sans->setEnabled(true);
      external_running = false;
      return;
   }

   // now move the file to the saxs directory
   QString new_created_dat = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + QFileInfo(sastbx_last_pdb).fileName() + iqq_suffix() + ".int";

   if ( QFile::exists(new_created_dat) )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->overwrite )
      {
         editor_msg("red", QString(tr("Notice: overwriting %1")).arg( new_created_dat ));
      } else {
         new_created_dat = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( new_created_dat, 0, this );
      }
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( !qd.rename( created_dat, new_created_dat ) )
   {
      editor_msg("red", 
                 QString(tr("Notice: could not rename %1 to %2"))
                 .arg( created_dat )
                 .arg( new_created_dat )
                 );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   load_saxs( new_created_dat );
   pb_plot_saxs_sans->setEnabled(true);
   external_running = false;
}
   
void US_Hydrodyn_Saxs::sastbx_launchFinished()
{
   editor_msg("brown", "Sastbx launch exited\n");
   disconnect( sastbx, SIGNAL(launchFinished()), 0, 0);
}
