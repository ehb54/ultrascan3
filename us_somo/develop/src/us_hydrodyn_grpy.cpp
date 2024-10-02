// (this) us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o
// us_hydrodyn_info.cpp contains code to report structures for debugging
// us_hydrodyn_util.cpp contains other various code, such as disulfide code
// us_hydrodyn_load.cpp contains code to load files 
// us_hydrodyn_grpy.cpp contains code for grpy interface

// includes and defines need cleanup
 
#include "../include/us_hydrodyn.h"

#define SLASH QDir::separator()

double US_Hydrodyn::model_mw( const vector < PDB_atom *> use_model ) {
   double mw = 0e0;
   for (unsigned int i = 0; i < use_model.size(); i++) {
      if ( use_model[i]->active ) {
         mw += use_model[i]->bead_ref_mw + use_model[i]->bead_ref_ionized_mw_delta;
      }
   }
   return mw;
}

vector < PDB_atom > US_Hydrodyn::exposed_model( const vector < PDB_atom > & model ) {
   vector < PDB_atom > rmodel;

   for (int i = 0; i < (int) model.size(); ++i) {
      if ( model[i].active ) {
         int color = get_color( &model[ i ] );
         if ( /* color != 6 && */ color != 0 ) {
            rmodel.push_back( model[ i ] );
         }
      }
   }
   return rmodel;
}

double US_Hydrodyn::model_mw( const vector < PDB_atom > & use_model ) {
   double mw = 0e0;
   for (unsigned int i = 0; i < use_model.size(); i++) {
      if ( use_model[i].active ) {
         mw += use_model[i].bead_ref_mw + use_model[i].bead_ref_ionized_mw_delta;
      }
   }
   return mw;
}

int US_Hydrodyn::grpy_used_beads_count( const vector < PDB_atom *> use_model ) {
   int used_beads = 0;
   for (unsigned int i = 0; i < use_model.size(); i++) {
      if ( use_model[i]->active ) {
         int color = get_color( use_model[ i ] );
         if ( hydro.grpy_bead_inclusion || color != 6 ) {
            used_beads++;
         }
      }
   }
   return used_beads;
}

int US_Hydrodyn::grpy_used_beads_count( const vector < PDB_atom > & use_model ) {
   int used_beads = 0;
   for (unsigned int i = 0; i < use_model.size(); i++) {
      if ( use_model[i].active ) {
         int color = get_color( & use_model[ i ] );
         if ( hydro.grpy_bead_inclusion || color != 6 ) {
            used_beads++;
         }
      }
   }
   return used_beads;
}

int US_Hydrodyn::total_beads_count( const vector < PDB_atom > & use_model ) {
   int total_beads = 0;
   for (unsigned int i = 0; i < use_model.size(); i++) {
      if ( use_model[i].active ) {
         total_beads++;
      }
   }
   return total_beads;
}

// grpy process flow
// start: calc_grpy_hydro()
// for each model grpy_process_next()
// when finished  grpy_finished()
// when all models complete grpy_finalize() - which stores save_data etc

bool US_Hydrodyn::calc_grpy_hydro() {
   progress->set_cli_prefix( "ch" );

   {
      QFont courier = QFont( "Courier", USglobal->config_list.fontSize );
      editor_msg( "dark blue", courier, visc_dens_msg() );
   }

   if ( !us_container_grpy ) {
      if ( !grpy_parallel_pulled & misc.parallel_grpy ) {
         editor_msg( "dark red", us_tr(
                                       "Attempting to get the docker image for parallel GRPY\n"
                                       "The first time you run this, it may take awhile\n"
                                       "Please be patient\n"
                                       ) );
         qApp->processEvents();
      }                                        
      us_container_grpy = new US_Container_Grpy( !grpy_parallel_pulled, !misc.parallel_grpy );
   } else if ( ( misc.parallel_grpy && !us_container_grpy->arguments().size() )
               || ( !misc.parallel_grpy && us_container_grpy->arguments().size() ) ) {
      // in case they switched from non-parallel to parallel or vice-versa
      qDebug() << "grpy recreate!!";
      delete us_container_grpy;
      if ( !grpy_parallel_pulled & misc.parallel_grpy ) {
         editor_msg( "dark red", us_tr(
                                       "Attempting to get the docker image for parallel GRPY\n"
                                       "The first time you run this, it may take awhile\n"
                                       "Please be patient\n"
                                       ) );
         qApp->processEvents();
      }                                        
      us_container_grpy = new US_Container_Grpy( !grpy_parallel_pulled, !misc.parallel_grpy );
   } else {
      qDebug() << "grpy NOT recreated";
   }      
   

   if ( misc.parallel_grpy && !us_container_grpy->arguments().size() ) {
      editor_msg( "red", us_tr( "parallel GRPY requested but unable to start\n"
                                "Check to make sure you have 'docker' installed and a working network connection to the internet\n"
                                "Resorting to non-parallel GRPY\n" ) );
   }

   if ( misc.parallel_grpy && !us_container_grpy->arguments().size() ) {
      grpy_parallel_pulled = true;
   }

   grpy_prog = us_container_grpy->executable();
   
   if ( us_container_grpy->arguments().size() ) {
      editor_msg( "darkblue", QString( us_tr( "\nParallel GRPY enabled with %1 threads\n" ) ).arg( USglobal->config_list.numThreads ) );
   } else {
      QFileInfo qfi( grpy_prog );
      if ( !qfi.exists() ) {
         editor_msg( (QString) "red", QString("GRPY program '%1' does not exist\n").arg(grpy_prog));
         return false;
      }
   }

   // if ( !overwrite_hydro )
   // {
   //    setHydroFile();
   // }

   stopFlag = false;
   grpy_was_hydro_enabled = pb_calc_hydro->isEnabled();
   pb_stop_calc->setEnabled(true);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled( false );
   pb_calc_hullrad->setEnabled( false );
   //   puts("calc hydro (supc)");
   set_disabled( false );
   display_default_differences();
   editor->append("\nBegin hydrodynamic calculations\n\n");
   results.s20w_sd = 0.0;
   results.D20w_sd = 0.0;
   results.viscosity_sd = 0.0;
   results.rs_sd = 0.0;
   results.rg_sd = 0.0;
   results.tau_sd = 0.0;

   int models_to_proc = 0;
   int first_model_no = 0;
   QStringList model_names;
   grpy_to_process   .clear();
   grpy_model_numbers.clear();
   grpy_processed    .clear();
   grpy_addl_params  .clear();

   grpy_results.method                = "GRPY";
   grpy_results.mass                  = 0e0;
   grpy_results.s20w                  = 0e0;
   grpy_results.s20w_sd               = 0e0;
   grpy_results.D20w                  = 0e0;
   grpy_results.D20w_sd               = 0e0;
   grpy_results.viscosity             = 0e0;
   grpy_results.viscosity_sd          = 0e0;
   grpy_results.rs                    = 0e0;
   grpy_results.rs_sd                 = 0e0;
   grpy_results.rg                    = 0e0;
   grpy_results.rg_sd                 = 0e0;
   grpy_results.tau                   = 0e0;
   grpy_results.tau_sd                = 0e0;
   grpy_results.asa_rg_pos            = 0e0;
   grpy_results.asa_rg_neg            = 0e0;
   grpy_results.ff0                   = 0e0;
   grpy_results.ff0_sd                = 0e0;

   grpy_results.solvent_name          = hydro.solvent_name;
   grpy_results.solvent_acronym       = hydro.solvent_acronym;
   grpy_results.solvent_viscosity     = use_solvent_visc();
   grpy_results.solvent_density       = use_solvent_dens();
   grpy_results.temperature           = hydro.temperature;
   grpy_results.pH                    = hydro.pH;
   grpy_results.name                  = "";
   grpy_results.used_beads            = 0;
   grpy_results.used_beads_sd         = 0e0;
   grpy_results.total_beads           = 0;
   grpy_results.total_beads_sd        = 0e0;
   grpy_results.vbar                  = 0;
   
   grpy_results.num_models            = 0;

   grpy_results2                      = grpy_results;

   grpy_vdw                           = bead_model_suffix.contains( "-vdw" );
   
   QDir::setCurrent(get_somo_dir());
   
   QString extension;

#if defined( TEST_VDW_GRPY_ASA )
   bool use_threshold = false;
   if ( !hydro.bead_inclusion && bead_model_suffix.contains( "-vdw" ) ) {

      switch ( QMessageBox::question(this, 
                                     this->windowTitle() + us_tr(": GRPY ASA" ),
                                     us_tr( "Choose the ASA method for GRPY bead inclusion"),
                                     us_tr( "&Use SOMO ASA Threshold [A^2]" ), 
                                     us_tr( "Use SOMO Bead ASA Threshold %" ), 
                                     QString(),
                                     1, // Stop == button 0
                                     1 // Escape == button 0
                                     ) )
      {
      case 0 : //
         use_threshold = true;
         break;
      case 1 : // keep
         use_threshold = false;
         break;
      }

      qDebug() << "use_threshold is " << ( use_threshold ? "true" : "false" );

   }
#endif

   if ( !hydro.grpy_bead_inclusion && grpy_vdw ) {

      extension =
         QString( "_R%1PR%2" )
         .arg( asa.vdw_grpy_threshold_percent )
         .arg( asa.vdw_grpy_probe_radius )
         ;
   }

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         if (somo_processed[current_model]) {
            if (!first_model_no) {
               first_model_no = current_model + 1;
            }
            models_to_proc++;
            results.vbar = use_vbar( model_vector[ current_model ].vbar );

            editor_msg( "black", QString( "Model %1 will be included\n").arg( model_name( current_model ) ) );
            model_names.push_back( model_name( current_model ) );
            set_bead_colors( bead_models[ current_model ] );

            bead_model = bead_models[current_model];

            QString fname = 
               get_somo_dir() + SLASH +
               project +
               ( bead_model_from_file ? "" : QString( "_%1" ).arg( model_name( current_model ) ) ) +
               QString( bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
               extension
               ;

            if ( !overwrite_hydro ) {
               fname = fileNameCheck( fname, 0, this );
            }

            QTextStream( stdout ) << "grpy file is '" << fname << "'" << Qt::endl
                                  << "grpy file.fileName() is '" << QFileInfo( fname ).baseName() << "'" << Qt::endl
                                  << "bead model size is " << bead_model.size() << Qt::endl
               ;
            
            // always run asa

            if ( !hydro.grpy_bead_inclusion && grpy_vdw ) {
               // expose all
               for ( int i = 0; i < (int) bead_model.size(); ++i ) {
                  bead_model[ i ].exposed_code = 1;
               }
               editor_msg( "black",
                           QString( us_tr( "Running ASA check on vdW bead model: Probe radius %1 [A], Threshold %2%." ) )
                           .arg( asa.vdw_grpy_probe_radius )
                           .arg( asa.vdw_grpy_threshold_percent )
                           );
               bead_check( false, true, true );
            }

            editor_msg( "black", us_tr( "Running PAT" ) );
            pat_model( bead_model );
            
            // qDebug() << "calc_grpy_hydro() asa rg +/- " << results.asa_rg_pos << " " << results.asa_rg_neg;

            grpy_addl_param.clear();
            // grpy_addl_param[ "asa_rg_pos" ] = model_vector[ current_model ].asa_rg_pos;
            // grpy_addl_param[ "asa_rg_neg" ] = model_vector[ current_model ].asa_rg_neg;
            grpy_addl_params.push_back( grpy_addl_param );
               
            write_bead_model( QFileInfo( fname ).fileName(),
                              & bead_model,
                              US_HYDRODYN_OUTPUT_GRPY );

            write_bead_model( QFileInfo( fname ).fileName() + "-grpy", 
                              & bead_model );

            grpy_to_process    << QFileInfo( fname ).fileName() + ".grpy";
            grpy_model_numbers.push_back( current_model );
            grpy_used_beads   .push_back( grpy_used_beads_count( bead_model ) );
         } else {
            editor->append(QString("Model %1 - selected but bead model not built\n").arg( model_name( current_model ) ) );
         }
      }
   }

   qApp->processEvents();
   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      set_enabled();
      pb_calc_hydro->setEnabled(grpy_was_hydro_enabled);
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
      grpy_success = false;
      return false;
   }

   editor_msg( "dark blue", grpy_to_process.join( "\n" ) );

   grpy_mm_save_params.data_vector.clear();
   grpy_mm         = grpy_to_process.size() > 1;
   grpy_mm_results = "";
   grpy_mm_name    =
      get_somo_dir() + SLASH +
      project +
      QString( bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
      extension
      ;

   grpy_running    = true;
   grpy_success    = true;
   progress->setMaximum( 101 * ( grpy_to_process.size() ) + 1 );
   // qDebug() << "progress max " <<  101 * ( grpy_to_process.size() ) + 1;
   progress->setValue( 0 );
   mprogress->setValue( 0 );
   mprogress->setMaximum( 100 );
   mprogress->setFormat( "Model %p%" );
   if ( grpy_mm ) {
      mprogress->show();
   }
   
   timers.clear_timers();
   timers.init_timer( "compute grpy all models" );
   timers.start_timer( "compute grpy all models" );

   grpy_process_next();

   return true;
}

void US_Hydrodyn::grpy_process_next() {
   // qDebug() << "US_Hydrodyn::grpy_process_next()";
   if ( !grpy_to_process.size() ) {
      grpy_finalize();
      return;
   }

   progress->setValue( 101 * grpy_processed.size() + 1 );
   mprogress->setValue( 1 );
   // qDebug() << "progress value " << 101 * grpy_processed.size() + 1;

   grpy_last_processed    = grpy_to_process   [ 0 ];
   grpy_last_model_number = grpy_model_numbers[ 0 ];
   grpy_last_used_beads   = grpy_used_beads   [ 0 ];
   grpy_addl_param        = grpy_addl_params  [ 0 ];
   grpy_to_process        .pop_front();
   grpy_model_numbers     .pop_front();
   grpy_used_beads        .pop_front();
   grpy_addl_params       .pop_front();
   grpy_processed         .push_back( grpy_last_processed );

   grpy_stdout = "";

   timers.init_timer( "compute grpy this model" );
   timers.start_timer( "compute grpy this model" );

   grpy = new QProcess( this );
   grpy->setWorkingDirectory( get_somo_dir() );
   // us_qdebug( "prog is " + grpy_prog );
   // us_qdebug( "grpy_last_processed " + grpy_last_processed );
   {
      QStringList args;
      args << us_container_grpy->arguments( get_somo_dir() );
      args
         << "-e"
         << grpy_last_processed
         ;
      
      connect( grpy, SIGNAL(readyReadStandardOutput()), this, SLOT(grpy_readFromStdout()) );
      connect( grpy, SIGNAL(readyReadStandardError()), this, SLOT(grpy_readFromStderr()) );
      connect( grpy, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(grpy_finished( int, QProcess::ExitStatus )) );
      connect( grpy, SIGNAL(started()), this, SLOT(grpy_started()) );

      editor_msg( "black", QString( "\nStarting GRPY on %1 with %2 beads\n" )
                  .arg( QFileInfo( grpy_last_processed ).completeBaseName() )
                  .arg( grpy_last_used_beads )
                  );
      grpy->start( grpy_prog, args, QIODevice::ReadOnly );
   }
   
   return;
}

void US_Hydrodyn::grpy_readFromStdout()
{
   // us_qdebug( QString( "grpy_readFromStdout %1" ).arg( grpy_last_processed ) );
   static QRegularExpression re = QRegularExpression( "^\\s*(\\d+)%\\s*TASK:\\s*(.*)$" );
   QString qs = QString( grpy->readAllStandardOutput() );
   // only needed for windows
   qs = qs.replace( "\r\n", "\n" );
   QStringList qsl = qs.split( "\r" );
   int size = (int) qsl.size();
   for ( int i = 0; i < size; ++i ) {
      qs = qsl[ i ];
      if ( qs.contains( "% TASK:" ) ) {
         qs = qs.split( "\r" ).takeLast();
         QRegularExpressionMatch match = re.match( qs );
         if ( match.hasMatch() ) {
            // qDebug() << "capture 1:" << match.captured( 1 );
            // qDebug() << "capture 2:" << match.captured( 2 );
            progress->setValue( 101 * ( grpy_processed.size() - 1 ) + match.captured( 1 ).toDouble() );
            // qDebug() << "progress value " <<  101 * ( grpy_processed.size() - 1 ) + match.captured( 1 ).toDouble() + 1;
            mprogress->setValue( match.captured( 1 ).toDouble() );
            lbl_core_progress->setText( QString( "Model %1 : %2" )
                                        .arg( grpy_last_model_number + 1 )
                                        .arg( match.captured( 2 ) )
                                        );
         }
      } else {
         grpy_stdout += qs;
         // editor_msg( "brown", qs );
      }
   }
   //   qApp->processEvents();
}

void US_Hydrodyn::grpy_readFromStderr()
{
   // us_qdebug( QString( "grpy_readFromStderr %1" ).arg( grpy_last_processed ) );

   editor_msg( "red", QString( grpy->readAllStandardError() ) );
   //  qApp->processEvents();
}

void US_Hydrodyn::grpy_finished( int, QProcess::ExitStatus )
{
   // qDebug() << "US_Hydrodyn::grpy_finished():" << grpy_last_processed;
   // us_qdebug( QString( "grpy_processExited %1" ).arg( grpy_last_processed) );
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   grpy_readFromStderr();
   grpy_readFromStdout();
      //   }
   disconnect( grpy, SIGNAL(readyReadStandardOutput()), 0, 0);
   disconnect( grpy, SIGNAL(readyReadStandardError()), 0, 0);
   disconnect( grpy, SIGNAL(finished( int, QProcess::ExitStatus )), 0, 0);
   if (stopFlag) {
      editor_msg( "red", us_tr( "Stopped by user\n" ) );
      set_enabled();
      pb_calc_hydro->setEnabled(grpy_was_hydro_enabled);
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
      mprogress->hide();
      lbl_core_progress->setText( "" );
      grpy_success = false;
      grpy_running = false;
      return;
   }

   editor_msg( "black", "GRPY finished.\n");

   // post process the files
   
   QStringList caps;
   caps
      << "Rotational diffusion coefficient"
      << "1"
      << "Dr"

      << "Sedimentation coefficient \\(Mw Dlt \\(1\\. - \\(vbar\\*rho\\)\\)/\\(nA kB T\\)\\)"
      << "1"
      << "s"

      << "Translational diffusion coefficient"
      << "2"
      << "Dt"

      << "Zero frequency intrinsic viscosity eta 0"
      << "1"
      << "\\[eta\\]"

      << "Relaxation time \\(1\\)"
      << "2"
      << "tau1"

      << "Relaxation time \\(2\\)"
      << "2"
      << "tau2"

      << "Relaxation time \\(3\\)"
      << "2"
      << "tau3"

      << "Relaxation time \\(4\\)"
      << "1"
      << "tau4"

      << "Relaxation time \\(5\\)"
      << "1"
      << "tau5"

      << "Harmonic mean \\(correlation\\) time"
      << "1"
      << "tauh"

      << "Translational diffusion coefficient"
      << "3"
      << "rs"

      << "Zero frequency intrinsic viscosity eta 0"
      << "2"
      << "grpy_einst_rad"

      ;

   map < QString, double > captures;
   grpy_captures     .clear();

   for ( int i = 0; i < (int) caps.size(); i += 3 ) {
      QRegExp rx = QRegExp( caps[ i ] + "\\s*:\\s*(\\S+)" );
      int pos = 0;
      bool found = false;
      int cappos = caps[ i + 1 ].toInt();
      int count  = 1;
      while ( ( pos = rx.indexIn( grpy_stdout, pos ) ) != -1 ) {
         if ( cappos == count ) {
            grpy_captures[ caps[ i + 2 ] ].push_back( rx.cap( 1 ).toDouble() );
            us_qdebug( QString( "%1 : '%2'\n" ).arg( caps[ i + 2 ] ).arg( grpy_captures[ caps[ i + 2 ] ].back() ) );
            found = true;
            break;
         }
         pos += rx.matchedLength();
         ++count;
      }         
      if ( !found ) {
         qDebug() << "grpy caps not found " << i << "'" << caps[ i ] << "'";
         editor_msg( "red", QString( us_tr( "Could not find '%1' in GRPY output" ) ).arg( caps[ i ].replace( "\\", "" ) ) );
         grpy_success = false;
         grpy_captures[ caps[ i + 2 ] ].push_back( -9e99 );
      }
   }

   // extra matrix captures for D1, D2, D3

   {
      QStringList qsl = grpy_stdout.split( "\n" );
      vector < QString > toXYZ = { "z", "y", "x" };
      bool ok = false;
      int i = 0;
      for ( ; i < (int) qsl.size(); ++i ) {
         if ( qsl[i].contains( "calculated using the mobility center:" ) ) {
            ok = true;
            break;
         }
      }
      if (!ok ) {
         editor_msg( "red", us_tr( "Could not find rotational diffusion coefficients" ) );
         grpy_success = false;
      } else {
         i += 19;
         int j = 4;
         QString Dr[3];
         for ( int k = 0; k < 3; ++k, ++i, ++j ) {
            if ( qsl.size() <= i ) {
               grpy_success = false;
               break;
            }
            QStringList qsld = qsl[ i ].split( QRegExp( "\\s+" ) );
            if ( qsld.size() <= j ) {
               grpy_success = false;
               break;
            }
            Dr[k] = qsld[ j ];
            grpy_captures[ "rot_diff_coef_" + toXYZ[ k ] ].push_back( Dr[k].toDouble() );
            QTextStream( stdout ) << "Dr[" << k << "]" << Dr[k] << Qt::endl;
         }
      }
   }

   if ( !grpy_success ) {
      grpy_process_next();
      return;
   }

   // save stdout
   if ( !batch_avg_hydro_active() && !grpy_mm ) {
      QString grpy_out_name = grpy_last_processed.replace( QRegExp( ".grpy$" ), ".grpy_res" );
      if ( !overwrite_hydro ) {
         grpy_out_name = fileNameCheck( grpy_out_name, 0, this );
      }
      QFile f( grpy_out_name );
      if ( !f.open( QIODevice::WriteOnly ) ) {
         editor_msg( "red", QString( us_tr( "Error: could not open output file %1 for writing" ) ).arg( grpy_out_name ) );
         grpy_success = false;
         grpy_process_next();
         return;
      } else {
         QTextStream t( &f );
         t << grpy_stdout;
         editor_msg( "dark blue", QString( us_tr( "Wrote %1" ) ).arg( grpy_out_name ) );
         f.close();
         last_hydro_res = grpy_out_name;
      }
   }

   // accumulate data as in zeno (e.g. push values to data structures )
      
   map < int, map < QString, double > > data_to_save;
   
   for ( map < QString, vector < double > >::iterator it = grpy_captures.begin();
         it != grpy_captures.end();
         ++it ) {

      for ( int i = 0; i < (int) it->second.size(); ++i ) {
         data_to_save[ i ][ it->first ] = it->second[ i ];

         if ( it->first == "R\\(Anhydrous\\)" ) {
            grpy_results.rs += it->second[ i ];
            grpy_results2.rs += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "f/fo" ) {
            grpy_results.ff0 += it->second[ i ];
            grpy_results2.ff0 += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "Dt" ) {
            grpy_results.D20w += it->second[ i ];
            grpy_results2.D20w += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "s" ) {
            grpy_results.s20w += it->second[ i ];
            grpy_results2.s20w += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "\\[eta\\]" ) {
            grpy_results.viscosity += it->second[ i ];
            grpy_results2.viscosity += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "tauh" ) {
            grpy_results.tau += it->second[ i ] * 1e9;
            grpy_results2.tau += it->second[ i ] * it->second[ i ] * 1e18;
         }

         if ( it->first == "rs" ) {
            grpy_results.rs += it->second[ i ] * 1e7;
            grpy_results2.rs += it->second[ i ] * it->second[ i ] * 1e14;
         }
      }
   }
   
   timers.end_timer( "compute grpy this model" );

   for ( map < int, map < QString, double > >::iterator it = data_to_save.begin();
         it != data_to_save.end();
         ++it ) {
      save_data this_data = US_Hydrodyn_Save::save_data_initialized_from_bead_model( bead_models[ grpy_last_model_number ], !hydro.grpy_bead_inclusion );

      this_data.results.method                = "GRPY";
      this_data.results.mass                  = hydro.mass_correction ? hydro.mass : model_mw( bead_models[ grpy_last_model_number ] ); // ( model_vector[ grpy_last_model_number ].mw + model_vector[ grpy_last_model_number ].ionized_mw_delta );
      this_data.hydro                         = hydro;
      this_data.results.num_models            = 1;
      this_data.results.name                  =
         // QString( "%1-%2" ).arg( QFileInfo( grpy_last_processed ).completeBaseName().replace( QRegExp( ".grpy$" ), "" ) ).arg( it->first + 1 )
         QFileInfo( grpy_last_processed ).completeBaseName().replace( QRegExp( ".grpy$" ), "" )
         ;
      this_data.results.used_beads            = grpy_last_used_beads;
      this_data.results.total_beads           = total_beads_count( bead_models[ grpy_last_model_number ] );
      this_data.num_of_unused                 = this_data.results.total_beads - this_data.results.used_beads;
      this_data.results.vbar                  = use_vbar( model_vector[ grpy_last_model_number ].vbar );
      this_data.proc_time                     = (double)(timers.times[ "compute grpy this model" ]) / 1e3;
      this_data.results.asa_rg_pos            = model_vector[ grpy_last_model_number ].asa_rg_pos;
      this_data.results.asa_rg_neg            = model_vector[ grpy_last_model_number ].asa_rg_neg;
      this_data.rot_diff_coef_x               = grpy_captures[ "rot_diff_coef_x" ][0];
      this_data.rot_diff_coef_y               = grpy_captures[ "rot_diff_coef_y" ][0];
      this_data.rot_diff_coef_z               = grpy_captures[ "rot_diff_coef_z" ][0];

      this_data.fractal_dimension_parameters         = model_vector[ grpy_last_model_number ].fractal_dimension_parameters;
      this_data.fractal_dimension                    = model_vector[ grpy_last_model_number ].fractal_dimension;
      this_data.fractal_dimension_sd                 = model_vector[ grpy_last_model_number ].fractal_dimension_sd;
      this_data.fractal_dimension_wtd                = model_vector[ grpy_last_model_number ].fractal_dimension_wtd;
      this_data.fractal_dimension_wtd_sd             = model_vector[ grpy_last_model_number ].fractal_dimension_wtd_sd;
      this_data.fractal_dimension_wtd_wtd            = model_vector[ grpy_last_model_number ].fractal_dimension_wtd_wtd;
      this_data.fractal_dimension_wtd_wtd_sd         = model_vector[ grpy_last_model_number ].fractal_dimension_wtd_wtd_sd;
      this_data.rg_over_fractal_dimension            = model_vector[ grpy_last_model_number ].rg_over_fractal_dimension;
      this_data.rg_over_fractal_dimension_sd         = model_vector[ grpy_last_model_number ].rg_over_fractal_dimension_sd;
      this_data.rg_over_fractal_dimension_wtd        = model_vector[ grpy_last_model_number ].rg_over_fractal_dimension_wtd;
      this_data.rg_over_fractal_dimension_wtd_sd     = model_vector[ grpy_last_model_number ].rg_over_fractal_dimension_wtd_sd;
      this_data.rg_over_fractal_dimension_wtd_wtd    = model_vector[ grpy_last_model_number ].rg_over_fractal_dimension_wtd_wtd;
      this_data.rg_over_fractal_dimension_wtd_wtd_sd = model_vector[ grpy_last_model_number ].rg_over_fractal_dimension_wtd_wtd_sd;

      if ( !hydro.grpy_bead_inclusion && grpy_vdw ) {
         this_data.vdw_grpy_probe_radius      = asa.vdw_grpy_probe_radius;
         this_data.vdw_grpy_threshold         = asa.vdw_grpy_threshold_percent;
      }

      if ( bead_models[ grpy_last_model_number ].size() &&
           bead_models[ grpy_last_model_number ][0].is_vdw == "vdw" ) {
         this_data.hydrate_probe_radius          = bead_models[ grpy_last_model_number ][0].asa_hydrate_probe_radius;
         this_data.hydrate_threshold             = bead_models[ grpy_last_model_number ][0].asa_hydrate_threshold;
         this_data.vdw_theo_waters               = bead_models[ grpy_last_model_number ][0].vdw_theo_waters;
         this_data.vdw_exposed_residues          = bead_models[ grpy_last_model_number ][0].vdw_count_exposed;
         this_data.vdw_exposed_waters            = bead_models[ grpy_last_model_number ][0].vdw_theo_waters_exposed;
      }

      // qDebug() << "US_Hydrodyn::grpy_finished() asa rg pos " << this_data.results.asa_rg_pos << " neg " << this_data.results.asa_rg_neg;
      
      if ( it->second.count( "Dr" ) ) {
         this_data.rot_diff_coef = it->second[ "Dr" ];
         // qDebug() << "this_data.rot_diff_coef : " << this_data.rot_diff_coef;
      }
      if ( it->second.count( "s" ) ) {
         this_data.results.s20w = it->second[ "s" ];
      }
      if ( it->second.count( "Dt" ) ) {
         this_data.results.D20w = it->second[ "Dt" ];
      }
      if ( it->second.count( "\\[eta\\]" ) ) {
         this_data.results.viscosity = it->second[ "\\[eta\\]" ];
      }
      if ( it->second.count( "tau1" ) ) {
         this_data.rel_times_tau_1 = it->second[ "tau1" ] * 1e9;
      }
      if ( it->second.count( "tau2" ) ) {
         this_data.rel_times_tau_2 = it->second[ "tau2" ] * 1e9;
      }
      if ( it->second.count( "tau3" ) ) {
         this_data.rel_times_tau_3 = it->second[ "tau3" ] * 1e9;
      }
      if ( it->second.count( "tau4" ) ) {
         this_data.rel_times_tau_4 = it->second[ "tau4" ] * 1e9;
      }
      if ( it->second.count( "tau5" ) ) {
         this_data.rel_times_tau_5 = it->second[ "tau5" ] * 1e9;
      }
      if ( it->second.count( "tauh" ) ) {
         this_data.rel_times_tau_h = it->second[ "tauh" ] * 1e9;
      }

      if ( it->second.count( "R\\(Rotation\\)" ) ) {
         // this_data.results.mass = it->second[ "R\\(Rotation\\)" ];
      }
      if ( it->second.count( "rs" ) ) {
         this_data.results.rs = it->second[ "rs" ] * 1e7;
      }
      if ( it->second.count( "grpy_einst_rad" ) ) {
         this_data.grpy_einst_rad = it->second[ "grpy_einst_rad" ] * 1e7;
      }
      
      // calculated params

      {
         double fconv = pow(10.0, this_data.hydro.unit + 9);
         this_data.con_factor = fconv;
         // bead model rg

         if ( this_data.results.mass ) {
            // compute center of mass
            bead_model = bead_models[ grpy_last_model_number ];
            point cm;
            cm.axis[ 0 ] = 0.0;
            cm.axis[ 1 ] = 0.0;
            cm.axis[ 2 ] = 0.0;
            double total_cm_mw = 0e0;

            for ( unsigned int i = 0; i < bead_model.size(); ++i )
            {
               PDB_atom *this_atom = &(bead_model[i]);
               double bead_tot_mw = this_atom->bead_ref_mw + this_atom->bead_ref_ionized_mw_delta;
               cm.axis[ 0 ] += bead_tot_mw * this_atom->bead_coordinate.axis[ 0 ];
               cm.axis[ 1 ] += bead_tot_mw * this_atom->bead_coordinate.axis[ 1 ];
               cm.axis[ 2 ] += bead_tot_mw * this_atom->bead_coordinate.axis[ 2 ];
               total_cm_mw += bead_tot_mw;
            }

            cm.axis[ 0 ] /= total_cm_mw;
            cm.axis[ 1 ] /= total_cm_mw;
            cm.axis[ 2 ] /= total_cm_mw;

            // now compute Rg
            double Rg2 = 0e0;

            for ( unsigned int i = 0; i < bead_model.size(); ++i )
            {
               PDB_atom *this_atom = &(bead_model[i]);
               Rg2 += ( (double) this_atom->bead_ref_mw + this_atom->bead_ref_ionized_mw_delta ) * 
                  ( 
                   (double) ( this_atom->bead_coordinate.axis[ 0 ] - cm.axis[ 0 ] ) *
                   ( this_atom->bead_coordinate.axis[ 0 ] - cm.axis[ 0 ] ) +
                   (double) ( this_atom->bead_coordinate.axis[ 1 ] - cm.axis[ 1 ] ) *
                   ( this_atom->bead_coordinate.axis[ 1 ] - cm.axis[ 1 ] ) +
                   (double) ( this_atom->bead_coordinate.axis[ 2 ] - cm.axis[ 2 ] ) *
                   ( this_atom->bead_coordinate.axis[ 2 ] - cm.axis[ 2 ] ) 
                    );
            }

            double Rg = sqrt( Rg2 / total_cm_mw );

            this_data.results.rg = Rg * fconv;
            grpy_results.rg  += this_data.results.rg;
            grpy_results2.rg += this_data.results.rg * this_data.results.rg;
            us_qdebug( QString( "rg %1 fconv %2 rg2 %3 total_cm_mw %4" ).arg( Rg ).arg( fconv ).arg( Rg2 ).arg( total_cm_mw ) );
         }

         // tra_fric_coef

         {
            if ( this_data.results.D20w ) {
               this_data.tra_fric_coef =
                  Rbar * ( K0 + hydro.temperature ) / ( AVOGADRO * this_data.results.D20w );

               QTextStream( stdout )
                  << "tfc " << this_data.tra_fric_coef << " = " << Qt::endl
                  << "Rbar * ( K0 + hydro.temperature )" << ( Rbar * ( K0 + hydro.temperature ) ) << " / " << Qt::endl
                  << "AVOGADRO * this_data.results.D20w" << ( AVOGADRO * this_data.results.D20w ) << Qt::endl
                  ;
               
            }
         }

         {
            if ( this_data.rot_diff_coef ) {
               this_data.rot_fric_coef =
                  Rbar * ( K0 + hydro.temperature ) / ( AVOGADRO * this_data.rot_diff_coef );

               // QTextStream( stdout )
               //    << "rfc " << this_data.rot_fric_coef << " = " << Qt::endl
               //    << "Rbar * ( K0 + hydro.temperature )" << ( Rbar * ( K0 + hydro.temperature ) ) << " / " << Qt::endl
               //    << "AVOGADRO * this_data.rot_diff_coef" << ( AVOGADRO * this_data.rot_diff_coef ) << Qt::endl
               //    ;
               
            }
         }

         // already computed
         // // stokes radius
         // {
         //    if ( this_data.results.D20w ) {
         //       this_data.results.rs = 1e1 * ( 1e7 / fconv ) * 
         //          Rbar * ( K0 + hydro.temperature ) / ( AVOGADRO * 6.0 * M_PI * use_solvent_visc() * this_data.results.D20w );
         //       grpy_results.rs  += this_data.results.rs;
         //       grpy_results2.rs += this_data.results.rs * this_data.results.rs;
         //    }
         // }

         // need ff0 & stokes radius

         {
            QTextStream(stdout)
               << "f    = " << this_data.tra_fric_coef << Qt::endl
               << "Rs   = " << this_data.results.rs << Qt::endl
               << "f0   = " << ( this_data.results.rs * 6.0 * M_PI * use_solvent_visc() ) << Qt::endl
               << "f/f0 = " << ( this_data.tra_fric_coef / ( this_data.results.rs * 6.0 * M_PI * use_solvent_visc() ) ) << Qt::endl
               ;
            
            this_data.results.ff0 = 
               this_data.tra_fric_coef * 10 / 
               ( fconv * 6e0 * M_PI *  use_solvent_visc() * 
                 pow( 3.0 * this_data.results.mass * this_data.results.vbar / (4.0 * M_PI * AVOGADRO), 1.0/3.0 ) );

            QTextStream( stdout )
               << "ff0 " << this_data.results.ff0 << " = " << Qt::endl
               ;

            // this_data.results.ff0_sd = this_data.results.ff0 * this_data.tra_fric_coef_sd / this_data.tra_fric_coef;
            grpy_results.ff0  += this_data.results.ff0;
            grpy_results2.ff0 += this_data.results.ff0 * this_data.results.ff0;
         }

         // nsa physical stats

         {
            vector < vector < PDB_atom > >  save_bead_models = bead_models;
            saxs_util->bead_models.resize( 1 );
            saxs_util->bead_models[ 0 ] = bead_models[ grpy_last_model_number ];
            if ( "empty model" != saxs_util->nsa_physical_stats() )
            {
               this_data.tot_volume_of = saxs_util->nsa_physical_stats_map[ "result excluded volume" ].toDouble();
                  
               this_data.max_ext_x = saxs_util->nsa_physical_stats_map[ "result radial extent bounding box size x" ].toDouble() * fconv;
               this_data.max_ext_y = saxs_util->nsa_physical_stats_map[ "result radial extent bounding box size y" ].toDouble() * fconv;
               this_data.max_ext_z = saxs_util->nsa_physical_stats_map[ "result radial extent bounding box size z" ].toDouble() * fconv;

               this_data.axi_ratios_xz = saxs_util->nsa_physical_stats_map[ "result radial extent axial ratios x:z" ].toDouble();
               this_data.axi_ratios_xy = saxs_util->nsa_physical_stats_map[ "result radial extent axial ratios x:y" ].toDouble();
               this_data.axi_ratios_yz = saxs_util->nsa_physical_stats_map[ "result radial extent axial ratios y:z" ].toDouble();
            } else {
               editor_msg( "red", QString( "Internal error: Bead model is empty?" ) );
            }
         }

         // rot fric and stokes
         {
            double factor = 1e-2 * (Rbar/AVOGADRO) * ( K0 + hydro.temperature ) / pow( fconv, 2 );
            this_data.rot_fric_coef_x =
               factor / this_data.rot_diff_coef_x;
            this_data.rot_fric_coef_y =
               factor / this_data.rot_diff_coef_y;
            this_data.rot_fric_coef_z =
               factor / this_data.rot_diff_coef_z;

            this_data.rot_stokes_rad_x =
               1e8 * fconv * pow( this_data.rot_fric_coef_x / ( 8.0 * M_PI * use_solvent_visc() * 1e-2 ), 1.0/3.0 );
            this_data.rot_stokes_rad_y =
               1e8 * fconv * pow( this_data.rot_fric_coef_y / ( 8.0 * M_PI * use_solvent_visc() * 1e-2 ), 1.0/3.0 );
            this_data.rot_stokes_rad_z =
               1e8 * fconv * pow( this_data.rot_fric_coef_z / ( 8.0 * M_PI * use_solvent_visc() * 1e-2 ), 1.0/3.0 );
         }
      }

      this_data.rel_times_tau_m =
         (
          this_data.rel_times_tau_1 +
          this_data.rel_times_tau_2 +
          this_data.rel_times_tau_3 +
          this_data.rel_times_tau_4 +
          this_data.rel_times_tau_5
          ) / 5.0;

      this_data.results.tau = this_data.rel_times_tau_h;
                                                                        
      // qDebug() << "GRPY accumulating data for " << this_data.results.name;

      grpy_results .name                 +=
         // ( grpy_results.name.isEmpty() ? QString( this_data.results.name + " " ) : QString( ",-%1" ).arg( grpy_last_model_number + 1 ) )
         this_data.results.name + " "
         ;
      grpy_results .mass                 += this_data.results.mass;
      grpy_results2.mass                 += this_data.results.mass        * this_data.results.mass;
      grpy_results .used_beads           += this_data.results.used_beads;
      grpy_results2.used_beads           += this_data.results.used_beads  * this_data.results.used_beads;
      grpy_results .total_beads          += this_data.results.total_beads;
      grpy_results2.total_beads          += this_data.results.total_beads * this_data.results.total_beads;
      grpy_results .vbar                 += this_data.results.vbar;
      grpy_results2.vbar                 += this_data.results.vbar        * this_data.results.vbar;
      grpy_results .num_models           ++ ;


      
      {
         QString add_to_grpy;

         add_to_grpy +=
            "\n---------------------------------------------------------\n"
            + us_tr(
                    "Summary information\n"
                    "The Translational Frictional Coefficient and Frictional Ratio are\n"
                    "derived from the GRPY results.\n"
                    )
            + "---------------------------------------------------------\n"
            ;
       

         add_to_grpy += QString( "\nGRPY computed on %1 Model %2%3\n" ).arg( project ).arg( grpy_last_model_number + 1 ).arg( bead_model_suffix.length() ? (" Bead model suffix: " + bead_model_suffix) : "" );
         add_to_grpy += QString( "Number of beads used: %1\n" ).arg( this_data.results.used_beads );
         add_to_grpy += QString( "MW: %1 [Da]\n" ).arg( this_data.results.mass );
         add_to_grpy += pH_msg();
         add_to_grpy += vbar_msg( this_data.results.vbar, true );
         add_to_grpy += visc_dens_msg( true );
                     
         if ( hydro.mass_correction ) {
            add_to_grpy += QString( "Manually corrected MW: %1 [Da]\n" ).arg( hydro.mass );
         }

         add_to_grpy += 
            QString( 
                    us_tr( 
                          "\n"
                          "US-SOMO Derived Parameters:\n"
                          "\n"
                          " Frictional Ratio                   f/f0 : %1\n"
                          " Tr. Frictional coefficient            f : %2\n"
                          " Radius of Gyration                   Rg : %3\n"
                           ) )
            .arg( QString( "" ).sprintf( "%3.2f"     , this_data.results.ff0   ) )
            .arg( QString( "" ).sprintf( "%4.2e g/s" , this_data.tra_fric_coef ) )
            .arg( QString( "" ).sprintf( "%4.2e nm"  , this_data.results.rg    ) )
            ;

         add_to_grpy += 
            QString( 
                    us_tr( 
                          " Rot. Diffusion coefficient            X : %1 \t1/s (w@20C)\n"
                          " Rot. Diffusion coefficient            Y : %2 \t1/s (w@20C)\n"
                          " Rot. Diffusion coefficient            Z : %3 \t1/s (w@20C)\n"
                          " Rot. Frictional coefficient           X : %4 \tg*cm^2/s (w@20C)\n"
                          " Rot. Frictional coefficient           Y : %5 \tg*cm^2/s (w@20C)\n"
                          " Rot. Frictional coefficient           Z : %6 \tg*cm^2/s (w@20C)\n"
                          " Rot. Stokes' radius                   X : %7 \tnm (w@20C)\n"
                          " Rot. Stokes' radius                   Y : %8 \tnm (w@20C)\n"
                          " Rot. Stokes' radius                   Z : %9 \tnm (w@20C)\n"
                           ) )
            .arg( QString().sprintf( "%5.4g"           , this_data.rot_diff_coef_x ) )
            .arg( QString().sprintf( "%5.4g"           , this_data.rot_diff_coef_y ) )
            .arg( QString().sprintf( "%5.4g"           , this_data.rot_diff_coef_z ) )
            .arg( QString().sprintf( "%5.4g"           , this_data.rot_fric_coef_x ) )
            .arg( QString().sprintf( "%5.4g"           , this_data.rot_fric_coef_y ) )
            .arg( QString().sprintf( "%5.4g"           , this_data.rot_fric_coef_z ) )
            .arg( QString().sprintf( "%5.4g"           , this_data.rot_stokes_rad_x ) )
            .arg( QString().sprintf( "%5.4g"           , this_data.rot_stokes_rad_y ) )
            .arg( QString().sprintf( "%5.4g"           , this_data.rot_stokes_rad_z ) )
            ;

         add_to_grpy +=
            QString(
                    us_tr( 
                          " Maximum extension                     X : %1\n"
                          " Maximum extension                     Y : %2\n"
                          " Maximum extension                     Z : %3\n"
                          " Axial ratio                         X:Z : %4\n"
                          " Axial ratio                         X:Y : %5\n"
                          " Axial ratio                         Y:Z : %6\n"
                           ) )
            .arg( QString( "%1 nm" ).arg( this_data.max_ext_x, 0, 'g', 4 ) )
            .arg( QString( "%1 nm" ).arg( this_data.max_ext_y, 0, 'g', 4 ) )
            .arg( QString( "%1 nm" ).arg( this_data.max_ext_z, 0, 'g', 4 ) )
            .arg( this_data.axi_ratios_xz, 0, 'g', 3 )
            .arg( this_data.axi_ratios_xy, 0, 'g', 3 )
            .arg( this_data.axi_ratios_yz, 0, 'g', 3 )
            ;

         if ( !batch_avg_hydro_active() && !grpy_mm ) {
            QFile f( last_hydro_res );
            if ( f.exists() && f.open( QIODevice::WriteOnly | QIODevice::Append ) )
            {
               QTextStream ts( &f );
               ts << add_to_grpy;
               f.close();
            }
         }

         this_data.hydro_res = grpy_stdout + add_to_grpy;
         if ( grpy_mm ) {
            grpy_mm_results += this_data.hydro_res;
         }
      }
      if ( grpy_mm ) {
         grpy_mm_save_params.data_vector.push_back( this_data );
      }

      if ( batch_widget &&
           batch_window->save_batch_active )
      {
         save_params.data_vector.push_back( this_data );
      }

      bool create_hydro_res = !(batch_widget &&
                                batch_window->save_batch_active);

      if ( saveParams && create_hydro_res && !grpy_mm )
      {
         QString fname = get_somo_dir() + "/" + this_data.results.name + ".grpy.csv";
         if ( !overwrite_hydro ) {
            fname = fileNameCheck( fname, 0, this );
         }

         FILE *of = us_fopen(fname, "wb");
         if ( of )
         {
            fprintf(of, "%s", save_util->header().toLatin1().data());

            fprintf(of, "%s", save_util->dataString(&this_data).toLatin1().data());
            fclose(of);
            editor_msg( "dark blue", QString( "created %1\n" ).arg( fname ) );
         }
      }
      // // print out results ?
      // save_util->header();
      // save_util->dataString(&this_data);
   }

   
   grpy_process_next();
}
   
void US_Hydrodyn::grpy_started()
{
   // us_qdebug( QString( "grpy_started %1" ).arg( grpy_last_processed ) );
   // editor_msg("brown", "GRPY launch exited\n");
   disconnect( grpy, SIGNAL(started()), 0, 0);
}

void US_Hydrodyn::grpy_finalize() {
   // qDebug() << "US_Hydrodyn::grpy_finalize():" << grpy_last_processed;
   // us_qdebug( QString( "grpy_finalize %1" ).arg( grpy_last_processed ) );
   editor_msg( "black", "Finalizing GRPY results" );
   // for ( map < QString, vector < double > >::iterator it = grpy_captures.begin();
   //       it != grpy_captures.end();
   //       ++it ) {
   //    editor_msg( "dark red",  US_Vector::qs_vector( it->first, it->second ) );
   // }

   {
      double num = (double) grpy_results.num_models;
      if ( num <= 1 ) {
         results = grpy_results;
      } else {
         double numinv = 1e0 / num;
         grpy_results.mass              *= numinv;
         grpy_results.s20w              *= numinv;
         grpy_results.D20w              *= numinv;
         grpy_results.viscosity         *= numinv;
         grpy_results.tau               *= numinv;
         grpy_results.rs                *= numinv;
         grpy_results.rg                *= numinv;
         grpy_results.vbar              *= numinv;
         grpy_results.ff0               *= numinv;
         grpy_results.used_beads        *= numinv;
         grpy_results.total_beads       *= numinv;
         if ( num <= 1 ) {
            results = grpy_results;
         } else {
            double numdecinv = 1e0 / ( num - 1e0 );
         
            grpy_results.s20w_sd              = sqrt( fabs( ( grpy_results2.s20w              - grpy_results.s20w              * grpy_results.s20w              * num ) * numdecinv ) );
            grpy_results.D20w_sd              = sqrt( fabs( ( grpy_results2.D20w              - grpy_results.D20w              * grpy_results.D20w              * num ) * numdecinv ) );
            grpy_results.viscosity_sd         = sqrt( fabs( ( grpy_results2.viscosity         - grpy_results.viscosity         * grpy_results.viscosity         * num ) * numdecinv ) );
            grpy_results.tau_sd               = sqrt( fabs( ( grpy_results2.tau               - grpy_results.tau               * grpy_results.tau               * num ) * numdecinv ) );
            grpy_results.rs_sd                = sqrt( fabs( ( grpy_results2.rs                - grpy_results.rs                * grpy_results.rs                * num ) * numdecinv ) );
            grpy_results.rg_sd                = sqrt( fabs( ( grpy_results2.rg                - grpy_results.rg                * grpy_results.rg                * num ) * numdecinv ) );
            grpy_results.ff0_sd               = sqrt( fabs( ( grpy_results2.ff0               - grpy_results.ff0               * grpy_results.ff0               * num ) * numdecinv ) );
            grpy_results.used_beads_sd        = sqrt( fabs( ( grpy_results2.used_beads        - (double) grpy_results.used_beads  * grpy_results.used_beads     * num ) * numdecinv ) );
            grpy_results.total_beads_sd       = sqrt( fabs( ( grpy_results2.total_beads       - (double) grpy_results.total_beads * grpy_results.total_beads    * num ) * numdecinv ) );
            
            results = grpy_results;
         }
      }
   }

   if ( grpy_mm ) {
      vector < save_data > stats = save_util->stats( & grpy_mm_save_params.data_vector );

      {
         QString grpy_out_name = grpy_mm_name + ".grpy_res";
         if ( !overwrite_hydro ) {
            grpy_out_name = fileNameCheck( grpy_out_name, 0, this );
         }
      
         QFile f( grpy_out_name );
         if ( !f.open( QIODevice::WriteOnly ) ) {
            editor_msg( "red", QString( us_tr( "Error: could not open output file %1 for writing" ) ).arg( grpy_out_name ) );
         } else {
            vector < save_data > stats = save_util->stats( & grpy_mm_save_params.data_vector );
            QTextStream t( &f );
            t << grpy_mm_results;
            t << save_util->hydroFormatStats( stats, US_Hydrodyn_Save::HYDRO_GRPY );
            editor_msg( "dark blue", QString( us_tr( "Wrote %1" ) ).arg( grpy_out_name ) );
            f.close();
            last_hydro_res = QFileInfo( grpy_out_name ).fileName();
         }
      }

      bool create_hydro_res = !(batch_widget &&
                                batch_window->save_batch_active);

      if ( saveParams && create_hydro_res ) {
         QString grpy_out_name = grpy_mm_name + ".grpy.csv";
         if ( !overwrite_hydro ) {
            grpy_out_name = fileNameCheck( grpy_out_name, 0, this );
         }
         QFile f( grpy_out_name );
         if ( !f.open( QIODevice::WriteOnly ) ) {
            editor_msg( "red", QString( us_tr( "Error: could not open output file %1 for writing" ) ).arg( grpy_out_name ) );
         } else {
            QTextStream t( &f );
            t << save_util->header().toLatin1().data();

            for ( int i = 0; i < (int) grpy_mm_save_params.data_vector.size(); ++i ) {
               t << save_util->dataString( & grpy_mm_save_params.data_vector[ i ] ).toLatin1().data();
            }
            for ( int i = 0; i < (int) stats.size(); ++i ) {
               t << save_util->dataString( & stats[ i ] ).toLatin1().data();
            }
               
            editor_msg( "dark blue", QString( us_tr( "Wrote %1" ) ).arg( grpy_out_name ) );
            f.close();
         }
      }
   }

   // qDebug() << "hydro_results enabled";
   
   pb_show_hydro_results->setEnabled( true );
   grpy_running = false;
   set_enabled();
   pb_calc_hydro->setEnabled(grpy_was_hydro_enabled);
   pb_calc_zeno->setEnabled(true);
   pb_bead_saxs->setEnabled(true);
   pb_calc_grpy->setEnabled( true );
   pb_calc_hullrad->setEnabled( true );
   pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   progress->reset();
   mprogress->hide();
   lbl_core_progress->setText( "" );
   editor_msg( "black", "GRPY finished" );
   timers.end_timer( "compute grpy all models" );
   editor_msg( "black", QString( "Time to process %1").arg( timers.time_min_sec( "compute grpy all models" ) ) );
   editor_msg( "dark blue", info_cite( "grpy" ) );

   // us_qdebug( QString( "grpy_finalize %1 end" ).arg( grpy_last_processed ) );
}
