// us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o
// us_hydrodyn_info.cpp contains code to report structures for debugging
// us_hydrodyn_util.cpp contains other various code, such as disulfide code
// (this) us_hydrodyn_load.cpp contains code to load files 

#include "../include/us_hydrodyn.h"
#include "../include/us_write_config.h"

#define TSE QTextStream( stderr )
#define TSO QTextStream( stdout )
#define LDB "------------------------------------------------------------\n"
#define LDE "============================================================\n"

void US_Hydrodyn::gui_script_msg( int line, QString cmd, QString msg ) {
   TSO << QString( "US_Hydrodyn::gui_script: %1 : %2 : %3\n" ).arg( line + 1).arg( cmd ).arg( msg );
}

void US_Hydrodyn::gui_script_error( int line, QString cmd, QString msg, bool doexit ) {
   gui_script_msg( line, cmd, "ERROR: " + msg );
   if ( doexit ) {
      exit( -1 );
   }
}

void US_Hydrodyn::gui_script_run() {
   TSO << "US_Hydrodyn::gui_script_run()\n";

   QString script;
   {
      QString error;
      if ( !US_File_Util::getcontents( gui_script_file, script, error ) ) {
         TSE << "US_Hydrodyn::gui_script_run() : Error " << error << "\n";
         exit( -1);
      } 
   }

   QStringList scriptlines = script.split( "\n" );
   int scriptlinesc        = (int) scriptlines.count();

   QRegExp rx_comment( "#.*$" );
   QRegExp rx_empty("^\\s*$");
   
   for ( int i = 0; i < scriptlinesc; ++i ) {
      QString l = scriptlines[ i ];
      TSO << LDE << QString( "somo> %1\n" ).arg( l );
      if ( l.contains( rx_comment )
           || l.contains( rx_empty ) ) {
         continue;
      }
           
      QStringList ls = l.split( QRegExp( "\\s+" ) );
      int lsc = (int)ls.count();
      if ( !lsc ) {
         // blank line?
         continue;
      }

      QString cmd = ls.front(); ls.pop_front();

      if ( cmd == "exit" ) {
         gui_script_msg( i, cmd, "exiting normally" );
         exit( 0 );
      } else if ( cmd == "threads" ) {
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         USglobal->config_list.numThreads = opt1.toUInt();
         US_Write_Config *WConfig;
         WConfig = new US_Write_Config();
         bool result = WConfig->write_config(USglobal->config_list);
         gui_script_msg( i, cmd, QString("Threads now %1 write config result %2\n").arg( opt1 ).arg(result ? "ok" : "not ok") );
      } else if ( cmd == "show" ) {
         show();
         if ( batch_widget ) {
            batch_window->show();
         }
         if ( saxs_plot_widget ) {
            saxs_plot_window->show();
         }
      } else if ( cmd == "hide" ) {
         hide();
         if ( batch_widget ) {
            batch_window->hide();
         }
         if ( saxs_plot_widget ) {
            saxs_plot_window->hide();
         }
      } else if ( cmd == "somo" ) {
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         gui_script_msg( i, cmd, opt1 );
         cb_overwrite->setChecked( true );
         set_overwrite();
      } else if ( cmd == "batch" ) {
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         if ( !batch_widget ) {
            gui_script_error( i, cmd, "batch widget not open" );
         }
         gui_script_msg( i, cmd, opt1 );
         if ( opt1 == "selectall" ) {
            batch_window->select_all();
         } else if ( opt1 == "somo_o" ) {
            if ( !batch_window->cb_somo_o->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_somo_o->setChecked( true );
            batch_window->set_somo_o();
         } else if ( opt1 == "prr" ) {
            if ( !batch_window->cb_prr->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_prr->setChecked( true );
            batch_window->set_prr();
         } else if ( opt1 == "sascsv" ) {
            if ( !batch_window->cb_csv_saxs->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_csv_saxs->setChecked( true );
            batch_window->set_csv_saxs();
         } else if ( opt1 == "sascsvname" ) {
            if ( ls.isEmpty() ) {
               gui_script_error( i, cmd, "missing argument" );
            }
            QString opt2 = ls.front(); ls.pop_front();
            if ( !batch_window->le_csv_saxs_name->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->le_csv_saxs_name->setText( opt2 );
         } else if ( opt1 == "zeno" ) {
            if ( !batch_window->cb_zeno->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_zeno->setChecked( true );
            batch_window->set_zeno();
         } else if ( opt1 == "combineh" ) {
            if ( !batch_window->cb_avg_hydro->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_avg_hydro->setChecked( true );
            batch_window->set_avg_hydro();
         } else if ( opt1 == "combinehname" ) {
            if ( ls.isEmpty() ) {
               gui_script_error( i, cmd, "missing argument" );
            }
            QString opt2 = ls.front(); ls.pop_front();
            if ( !batch_window->le_avg_hydro_name->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->le_avg_hydro_name->setText( opt2 );
         } else if ( opt1 == "saveparams" ) {
            if ( !batch_window->cb_saveParams->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_saveParams->setChecked( true );
            batch_window->set_saveParams();
         } else if ( opt1 == "start" ) {
            if ( !batch_window->pb_start->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->start();
         } else if ( opt1 == "overwrite" ) {
            batch_window->overwrite_all = true;
         } else {
            gui_script_error( i, cmd, "unknown option : " + opt1 );
         }
         qApp->processEvents();
      } else {
         gui_script_error( i, cmd, "unknown command" );
      }
   }
   gui_script_msg( scriptlinesc, "script", "finished" );
}
