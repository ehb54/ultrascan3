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

bool US_Hydrodyn::calc_grpy_hydro() {
   {
      QFont courier = QFont( "Courier", USglobal->config_list.fontSize );
      editor_msg( "dark blue", courier, visc_dens_msg() );
   }

   grpy_prog = 
      USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + "GRPY"
#if defined(Q_OS_WIN)
      + "_win64.exe"
#else
# if defined(Q_OS_MAC)
      + "_osx10.13"
# else
      + "_linux64"
# endif
#endif      
      ;

   QFileInfo qfi( grpy_prog );
   if ( !qfi.exists() ) {
      editor_msg( (QString) "red", QString("GRPY program '%1' does not exist\n").arg(grpy_prog));
      return false;
   }

   if ( !overwrite )
   {
      setHydroFile();
   }

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled( false );
   pb_calc_hullrad->setEnabled( false );
   //   puts("calc hydro (supc)");
   set_disabled();
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

   QDir::setCurrent(somo_dir);

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         if (somo_processed[current_model]) {
            if (!first_model_no) {
               first_model_no = current_model + 1;
            }
            models_to_proc++;
            results.vbar = use_vbar( model_vector[ current_model ].vbar );

            editor->append( QString( "Model %1 will be included\n").arg( model_name( current_model ) ) );
            model_names.push_back( model_name( current_model ) );
            bead_model = bead_models[current_model];

            QString fname = 
               somo_dir + SLASH +
               project +
               ( bead_model_from_file ? "" : QString( "_%1" ).arg( model_name( current_model ) ) ) +
               QString( bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
               ;

            QTextStream( stdout ) << "grpy file is '" << fname << "'" << endl
                                  << "grpy file.fileName() is '" << QFileInfo( fname ).baseName() << "'" << endl
                                  << "bead model size is " << bead_model.size() << endl
               ;
            
            write_bead_model( QFileInfo( fname ).fileName(),
                              & bead_model,
                              US_HYDRODYN_OUTPUT_GRPY );

            grpy_to_process    << QFileInfo( fname ).fileName() + ".grpy";
            grpy_model_numbers.push_back( current_model );
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
      pb_calc_hydro->setEnabled(true);
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
      return false;
   }

   editor_msg( "dark blue", grpy_to_process.join( "\n" ) );

   grpy_running = true;
   progress->setMaximum( grpy_to_process.size() + 1 );
   progress->setValue( 0 );
   
   timers.clear_timers();
   timers.init_timer( "compute grpy" );
   timers.start_timer( "compute grpy" );

   grpy_process_next();

   return true;
}

void US_Hydrodyn::grpy_process_next() {
   // qDebug() << "grpy_process_next()";
   if ( !grpy_to_process.size() ) {
      grpy_finalize();
      return;
   }

   grpy_last_processed    = grpy_to_process   [ 0 ];
   grpy_last_model_number = grpy_model_numbers[ 0 ];
   grpy_to_process        .pop_front();
   grpy_model_numbers     .pop_front();
   grpy_processed         .push_back( grpy_last_processed ); 

   grpy_stdout = "";
   progress->setValue( progress->value() + 1 );

   grpy = new QProcess( this );
   grpy->setWorkingDirectory( somo_dir );
   // us_qdebug( "prog is " + grpy_prog );
   // us_qdebug( "grpy_last_processed " + grpy_last_processed );
   {
      QStringList args;
      args
         << grpy_last_processed;

      connect( grpy, SIGNAL(readyReadStandardOutput()), this, SLOT(grpy_readFromStdout()) );
      connect( grpy, SIGNAL(readyReadStandardError()), this, SLOT(grpy_readFromStderr()) );
      connect( grpy, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(grpy_finished( int, QProcess::ExitStatus )) );
      connect( grpy, SIGNAL(started()), this, SLOT(grpy_started()) );

      editor_msg( "black", QString( "\nStarting GRPY on %1 with %2 beads\n" )
                  .arg( QFileInfo( grpy_last_processed ).baseName() )
                  .arg( bead_models[ grpy_last_model_number ].size() )
                  );
      grpy->start( grpy_prog, args, QIODevice::ReadOnly );
   }
   
   return;
}

void US_Hydrodyn::grpy_readFromStdout()
{
   // us_qdebug( QString( "grpy_readFromStdout %1" ).arg( grpy_last_processed ) );
   QString qs = QString( grpy->readAllStandardOutput() );
   grpy_stdout += qs;
   // editor_msg( "brown", qs );
   //  qApp->processEvents();
}
   
void US_Hydrodyn::grpy_readFromStderr()
{
   // us_qdebug( QString( "grpy_readFromStderr %1" ).arg( grpy_last_processed ) );

   editor_msg( "red", QString( grpy->readAllStandardError() ) );
   //  qApp->processEvents();
}

void US_Hydrodyn::grpy_finished( int, QProcess::ExitStatus )
{
   // qDebug() << "grpy_finished():" << grpy_last_processed;
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
      pb_calc_hydro->setEnabled(true);
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
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

      ;

   map < QString, double > captures;
   grpy_captures     .clear();

   for ( int i = 0; i < (int) caps.size(); i += 3 ) {
      QRegExp rx = QRegExp( caps[ i ] + "\\s*:\\s*(\\S+)" );
      int pos = 0;
      bool found = false;
      int cappos = caps[ i + 1 ].toInt();
      int count  = 1;
      while ( ( pos = rx.indexIn( grpy_stdout,pos ) ) != -1 ) {
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
         grpy_captures[ caps[ i + 2 ] ].push_back( -9e99 );
      }


      // failed attempt with the new QRegularExpression class :(

      // QString thisre = caps[ i ] + "\\s*:\\s*(\\S+)";
      // QRegularExpression re( caps[ i ] + "\\s*:\\s*(\\S+)", QRegularExpression::MultilineOption );
      // QRegularExpressionMatchIterator remi = re.globalMatch( grpy_stdout );
      
      // bool found = true;
      // QRegularExpressionMatch rem;
      // for ( int j = 0; j < caps[ i + 2 ].toInt(); ++j ) {
      //    if ( remi.hasNext() ) {
      //       rem = remi.next();
      //       qDebug() << "captured match " << i << " full stringlist " << rem.capturedTexts().join( " : " );
      //    } else {
      //       found = false;
      //       break;
      //    }
      // }
      // if ( found ) {
      //    qDebug() << "grpy caps FOUND " << i << "'" << thisre << "'";
      //    qDebug() << "rem captured " << rem.captured( 0 );
         
      //    grpy_captures[ caps[ i + 2 ] ].push_back( rem.captured( 0 ).toDouble() );
      //    us_qdebug( QString( "%1 : '%2'\n" ).arg( caps[ i + 2 ] ).arg( grpy_captures[ caps[ i + 2 ] ].back() ) );
      // } else {
      //    qDebug() << "grpy caps not found " << i << "'" << caps[ i ] << "'";
      //    editor_msg( "red", QString( us_tr( "Could not find '%1' in GRPY output" ) ).arg( caps[ i ].replace( "\\", "" ) ) );
      //    grpy_captures[ caps[ i + 2 ] ].push_back( -9e99 );
      // }

   }

   // save stdout
   {
      QString grpy_out_name = grpy_last_processed.replace( QRegExp( ".grpy$" ), ".grpy_res" );
      QFile f( grpy_out_name );
      if ( !f.open( QIODevice::WriteOnly ) ) {
         editor_msg( "red", QString( us_tr( "Error: could not open output file %1 for writing" ) ).arg( grpy_out_name ) );
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
      }
   }
   
   for ( map < int, map < QString, double > >::iterator it = data_to_save.begin();
         it != data_to_save.end();
         ++it ) {
      save_data this_data;

      this_data.tot_surf_area                 = 0e0;
      this_data.tot_volume_of                 = 0e0;
      this_data.num_of_unused                 = 0e0;
      this_data.use_beads_vol                 = 0e0;
      this_data.use_beads_surf                = 0e0;
      this_data.use_bead_mass                 = 0e0;
      this_data.con_factor                    = 0e0;
      this_data.tra_fric_coef                 = 0e0;
      this_data.rot_fric_coef                 = 0e0;
      this_data.rot_diff_coef                 = 0e0;
      this_data.rot_fric_coef_x               = 0e0;
      this_data.rot_fric_coef_y               = 0e0;
      this_data.rot_fric_coef_z               = 0e0;
      this_data.rot_diff_coef_x               = 0e0;
      this_data.rot_diff_coef_y               = 0e0;
      this_data.rot_diff_coef_z               = 0e0;
      this_data.rot_stokes_rad_x              = 0e0;
      this_data.rot_stokes_rad_y              = 0e0;
      this_data.rot_stokes_rad_z              = 0e0;
      this_data.cen_of_res_x                  = 0e0;
      this_data.cen_of_res_y                  = 0e0;
      this_data.cen_of_res_z                  = 0e0;
      this_data.cen_of_mass_x                 = 0e0;
      this_data.cen_of_mass_y                 = 0e0;
      this_data.cen_of_mass_z                 = 0e0;
      this_data.cen_of_diff_x                 = 0e0;
      this_data.cen_of_diff_y                 = 0e0;
      this_data.cen_of_diff_z                 = 0e0;
      this_data.cen_of_visc_x                 = 0e0;
      this_data.cen_of_visc_y                 = 0e0;
      this_data.cen_of_visc_z                 = 0e0;
      this_data.unc_int_visc                  = 0e0;
      this_data.unc_einst_rad                 = 0e0;
      this_data.cor_int_visc                  = 0e0;
      this_data.cor_einst_rad                 = 0e0;
      this_data.rel_times_tau_1               = 0e0;
      this_data.rel_times_tau_2               = 0e0;
      this_data.rel_times_tau_3               = 0e0;
      this_data.rel_times_tau_4               = 0e0;
      this_data.rel_times_tau_5               = 0e0;
      this_data.rel_times_tau_m               = 0e0;
      this_data.rel_times_tau_h               = 0e0;
      this_data.max_ext_x                     = 0e0;
      this_data.max_ext_y                     = 0e0;
      this_data.max_ext_z                     = 0e0;
      this_data.axi_ratios_xz                 = 0e0;
      this_data.axi_ratios_xy                 = 0e0;
      this_data.axi_ratios_yz                 = 0e0;
      this_data.results.method                = "GRPY";
      this_data.results.mass                  = model_vector[ grpy_last_model_number ].mw + model_vector[ grpy_last_model_number ].ionized_mw_delta;
      this_data.results.s20w                  = 0e0;
      this_data.results.s20w_sd               = 0e0;
      this_data.results.D20w                  = 0e0;
      this_data.results.D20w_sd               = 0e0;
      this_data.results.viscosity             = 0e0;
      this_data.results.viscosity_sd          = 0e0;
      this_data.results.rs                    = 0e0;
      this_data.results.rs_sd                 = 0e0;
      this_data.results.rg                    = 0e0;
      this_data.results.rg_sd                 = 0e0;
      this_data.results.tau                   = 0e0;
      this_data.results.tau_sd                = 0e0;
      this_data.results.vbar                  = 0e0;
      this_data.results.asa_rg_pos            = 0e0;
      this_data.results.asa_rg_neg            = 0e0;
      this_data.results.ff0                   = 0e0;
      this_data.results.ff0_sd                = 0e0;
      this_data.results.solvent_name          = "";
      this_data.results.solvent_acronym       = "";
      this_data.results.solvent_viscosity     = 0e0;
      this_data.results.solvent_density       = 0e0;

      this_data.hydro                         = hydro;
      this_data.results.num_models            = 1;
      this_data.results.name                  =
         // QString( "%1-%2" ).arg( QFileInfo( grpy_last_processed ).completeBaseName().replace( QRegExp( ".grpy$" ), "" ) ).arg( it->first + 1 )
         QFileInfo( grpy_last_processed ).completeBaseName().replace( QRegExp( ".grpy$" ), "" )
         ;
      this_data.results.used_beads            = bead_models[ grpy_last_model_number ].size();
      this_data.results.used_beads_sd         = 0e0;
      this_data.results.total_beads           = bead_models[ grpy_last_model_number ].size();
      this_data.results.total_beads_sd        = 0e0;
      this_data.results.vbar                  = use_vbar( model_vector[ grpy_last_model_number ].vbar );

      if ( it->second.count( "Dr" ) ) {
         this_data.rot_diff_coef = it->second[ "Dr" ];
      }
      if ( it->second.count( "s" ) ) {
         this_data.results.s20w = it->second[ "s" ] * 1e13;
      }
      if ( it->second.count( "Dt" ) ) {
         this_data.results.D20w = it->second[ "Dt" ];
      }
      if ( it->second.count( "\\[eta\\]" ) ) {
         this_data.results.viscosity = it->second[ "\\[eta\\]" ];
      }
      if ( it->second.count( "tau1" ) ) {
         this_data.rel_times_tau_1 = it->second[ "tau1" ];
      }
      if ( it->second.count( "tau2" ) ) {
         this_data.rel_times_tau_2 = it->second[ "tau2" ];
      }
      if ( it->second.count( "tau3" ) ) {
         this_data.rel_times_tau_3 = it->second[ "tau3" ];
      }
      if ( it->second.count( "tau4" ) ) {
         this_data.rel_times_tau_4 = it->second[ "tau4" ];
      }
      if ( it->second.count( "tau5" ) ) {
         this_data.rel_times_tau_5 = it->second[ "tau5" ];
      }
      if ( it->second.count( "tauh" ) ) {
         this_data.rel_times_tau_h = it->second[ "tauh" ];
      }

      if ( it->second.count( "R\\(Rotation\\)" ) ) {
         // this_data.results.mass = it->second[ "R\\(Rotation\\)" ];
      }
      
      // calculated params

      {
         double fconv = pow(10.0, this_data.hydro.unit + 9);
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
               double bead_tot_mw = this_atom->bead_mw + this_atom->bead_ionized_mw_delta;
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
               Rg2 += ( this_atom->bead_mw + this_atom->bead_ionized_mw_delta ) * 
                  ( 
                   ( this_atom->bead_coordinate.axis[ 0 ] - cm.axis[ 0 ] ) *
                   ( this_atom->bead_coordinate.axis[ 0 ] - cm.axis[ 0 ] ) +
                   ( this_atom->bead_coordinate.axis[ 1 ] - cm.axis[ 1 ] ) *
                   ( this_atom->bead_coordinate.axis[ 1 ] - cm.axis[ 1 ] ) +
                   ( this_atom->bead_coordinate.axis[ 2 ] - cm.axis[ 2 ] ) *
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
                  R * ( K0 + hydro.temperature ) / ( AVOGADRO * this_data.results.D20w );

               QTextStream( stdout )
                  << "tfc " << this_data.tra_fric_coef << " = " << endl
                  << "R * ( K0 + hydro.temperature )" << ( R * ( K0 + hydro.temperature ) ) << " / " << endl
                  << "AVOGADRO * this_data.results.D20w" << ( AVOGADRO * this_data.results.D20w ) << endl
                  ;
               
            }
         }

         // stokes radius
         {
            if ( this_data.results.D20w ) {
               this_data.results.rs = 1e1 * ( 1e7 / fconv ) * 
                  R * ( K0 + hydro.temperature ) / ( AVOGADRO * 6.0 * M_PI * use_solvent_visc() * this_data.results.D20w );
               grpy_results.rs  += this_data.results.rs;
               grpy_results2.rs += this_data.results.rs * this_data.results.rs;
            }
         }

         // need ff0 & stokes radius

         {
            QTextStream(stdout)
               << "f    = " << this_data.tra_fric_coef << endl
               << "Rs   = " << this_data.results.rs << endl
               << "f0   = " << ( this_data.results.rs * 6.0 * M_PI * use_solvent_visc() ) << endl
               << "f/f0 = " << ( this_data.tra_fric_coef / ( this_data.results.rs * 6.0 * M_PI * use_solvent_visc() ) ) << endl
               ;
            
            this_data.results.ff0 = 
               this_data.tra_fric_coef * 10 / 
               ( fconv * 6e0 * M_PI *  use_solvent_visc() * 
                 pow( 3.0 * this_data.results.mass * this_data.results.vbar / (4.0 * M_PI * AVOGADRO), 1.0/3.0 ) );

            QTextStream( stdout )
               << "ff0 " << this_data.results.ff0 << " = " << endl
               ;

            // this_data.results.ff0_sd = this_data.results.ff0 * this_data.tra_fric_coef_sd / this_data.tra_fric_coef;
            grpy_results.ff0  += this_data.results.ff0;
            grpy_results2.ff0 += this_data.results.ff0 * this_data.results.ff0;
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

      if ( batch_widget &&
           batch_window->save_batch_active )
      {
         save_params.data_vector.push_back( this_data );
      }

      bool create_hydro_res = !(batch_widget &&
                                batch_window->save_batch_active);

      if ( saveParams && create_hydro_res )
      {
         QString fname = somo_dir + "/" + this_data.results.name + ".grpy.csv";
         FILE *of = us_fopen(fname, "wb");
         if ( of )
         {
            fprintf(of, "%s", save_util->header().toLatin1().data());

            fprintf(of, "%s", save_util->dataString(&this_data).toLatin1().data());
            fclose(of);
            editor_msg( "dark blue", QString( "created %1\n" ).arg( fname ) );
         }
      }
      // print out results:
      save_util->header();
      save_util->dataString(&this_data);
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
   // qDebug() << "grpy_finalize():" << grpy_last_processed;
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
         grpy_results.mass          *= numinv;
         grpy_results.s20w          *= numinv;
         grpy_results.D20w          *= numinv;
         grpy_results.viscosity     *= numinv;
         grpy_results.tau           *= numinv;
         grpy_results.rs            *= numinv;
         grpy_results.rg            *= numinv;
         grpy_results.vbar          *= numinv;
         grpy_results.ff0           *= numinv;
         grpy_results.used_beads    *= numinv;
         grpy_results.total_beads   *= numinv;
         if ( num <= 1 ) {
            results = grpy_results;
         } else {
            double numdecinv = 1e0 / ( num - 1e0 );
         
            grpy_results.s20w_sd           = sqrt( fabs( ( grpy_results2.s20w        - grpy_results.s20w        * grpy_results.s20w        * num ) * numdecinv ) );
            grpy_results.D20w_sd           = sqrt( fabs( ( grpy_results2.D20w        - grpy_results.D20w        * grpy_results.D20w        * num ) * numdecinv ) );
            grpy_results.viscosity_sd      = sqrt( fabs( ( grpy_results2.viscosity   - grpy_results.viscosity   * grpy_results.viscosity   * num ) * numdecinv ) );
            grpy_results.tau_sd            = sqrt( fabs( ( grpy_results2.tau         - grpy_results.tau         * grpy_results.tau         * num ) * numdecinv ) );
            grpy_results.rs_sd             = sqrt( fabs( ( grpy_results2.rs          - grpy_results.rs          * grpy_results.rs          * num ) * numdecinv ) );
            grpy_results.rg_sd             = sqrt( fabs( ( grpy_results2.rg          - grpy_results.rg          * grpy_results.rg          * num ) * numdecinv ) );
            grpy_results.ff0_sd            = sqrt( fabs( ( grpy_results2.ff0         - grpy_results.ff0         * grpy_results.ff0         * num ) * numdecinv ) );
            grpy_results.used_beads_sd     = sqrt( fabs( ( grpy_results2.used_beads  - grpy_results.used_beads  * grpy_results.used_beads  * num ) * numdecinv ) );
            grpy_results.total_beads_sd    = sqrt( fabs( ( grpy_results2.total_beads - grpy_results.total_beads * grpy_results.total_beads * num ) * numdecinv ) );
            
            results = grpy_results;
         }
      }
   }

   qDebug() << "hydro_results enabled";
   
   pb_show_hydro_results->setEnabled( true );
   grpy_running = false;
   set_enabled();
   pb_calc_hydro->setEnabled(true);
   pb_calc_zeno->setEnabled(true);
   pb_bead_saxs->setEnabled(true);
   pb_calc_grpy->setEnabled( true );
   pb_calc_hullrad->setEnabled( true );
   pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   progress->reset();
   editor_msg( "black", "GRPY finished" );
   timers.end_timer( "compute grpy" );
   editor_msg( "black", QString( "Time to process %1").arg( timers.time_min_sec( "compute grpy" ) ) );
   editor_msg( "dark blue", info_cite( "grpy" ) );

   // us_qdebug( QString( "grpy_finalize %1 end" ).arg( grpy_last_processed ) );
}
