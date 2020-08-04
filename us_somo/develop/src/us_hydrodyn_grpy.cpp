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
   grpy_to_process.clear();

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         if (somo_processed[current_model]) {
            if (!first_model_no) {
               first_model_no = current_model + 1;
            }
            models_to_proc++;
            editor->append( QString( "Model %1 will be included\n").arg( model_name( current_model ) ) );
            model_names.push_back( model_name( current_model ) );
            bead_model = bead_models[current_model];

            grpy_to_process <<
               somo_dir + SLASH +
               project +
               ( bead_model_from_file ? "" : QString( "_%1" ).arg( model_name( current_model ) ) ) +
               QString( bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
               + ".bead_model"
               ;
            
         } else {
            editor->append(QString("Model %1 - selected but bead model not built\n").arg( model_name( current_model ) ) );
         }
      }
   }

   QDir::setCurrent(somo_dir);

   qApp->processEvents();
   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
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
   grpy_process_next();

   return false;
}

void US_Hydrodyn::grpy_process_next() {
   
   // us_qdebug( QString( "grpy_process_next %1" ).arg( grpy_filename ) );

   if ( !grpy_to_process.size() ) {
      grpy_finalize();
      return;
   }

   grpy_last_processed = grpy_to_process[ 0 ];
   grpy_to_process.pop_front();
   grpy_processed.push_back( grpy_last_processed ); 

   grpy_stdout = "";

   grpy = new QProcess( this );
   //   grpy->setWorkingDirectory( dir );
   // us_qdebug( "prog is " + grpy_prog );
   // us_qdebug( "grpy_last_processed " + grpy_last_processed );
   {
      QStringList args;
      args
         << "-u"
         << grpy_last_processed;

      connect( grpy, SIGNAL(readyReadStandardOutput()), this, SLOT(grpy_readFromStdout()) );
      connect( grpy, SIGNAL(readyReadStandardError()), this, SLOT(grpy_readFromStderr()) );
      connect( grpy, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(grpy_finished( int, QProcess::ExitStatus )) );
      connect( grpy, SIGNAL(started()), this, SLOT(grpy_started()) );

      editor_msg( "black", QString( "\nStarting Grpy on %1\n" ).arg( QFileInfo( grpy_last_processed ).baseName() ) );
      grpy->start( grpy_prog, args, QIODevice::ReadOnly );
   }
   
   return;
}

void US_Hydrodyn::grpy_readFromStdout()
{
   // us_qdebug( QString( "grpy_readFromStdout %1" ).arg( grpy_filename ) );
   QString qs = QString( grpy->readAllStandardOutput() );
   grpy_stdout += qs;
   editor_msg( "brown", qs );
   //  qApp->processEvents();
}
   
void US_Hydrodyn::grpy_readFromStderr()
{
   // us_qdebug( QString( "grpy_readFromStderr %1" ).arg( grpy_filename ) );

   editor_msg( "red", QString( grpy->readAllStandardError() ) );
   //  qApp->processEvents();
}

void US_Hydrodyn::grpy_finished( int, QProcess::ExitStatus )
{
   // us_qdebug( QString( "grpy_processExited %1" ).arg( grpy_filename ) );
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   grpy_readFromStderr();
   grpy_readFromStdout();
      //   }
   disconnect( grpy, SIGNAL(readyReadStandardOutput()), 0, 0);
   disconnect( grpy, SIGNAL(readyReadStandardError()), 0, 0);
   disconnect( grpy, SIGNAL(finished( int, QProcess::ExitStatus )), 0, 0);
   editor->append("GRPY finished.\n");

   // post process the files
#if defined( GRPY_POST_PROCESS )

   QStringList caps;
   caps
      // M               :        14315     g/mol
      // v_bar           :        0.718     mL/g
      // R(Anhydrous)    :        15.98     Angstroms
      // Axial Ratio     :         1.49
      // f/fo            :         1.16
      // Dt              :       1.16e-06   cm^2/s
      // R(Translation)  :        18.48     Angstroms
      // s               :       1.93e-13   sec
      // [eta]           :         2.78     cm^3/g
      // Dr              :       1.99e+07   s^-1
      // R(Rotation)     :        20.06     Angstroms

      << "M"
      << "v_bar"
      << "R\\(Anhydrous\\)"
      << "Axial Ratio"
      << "f/fo"
      << "Dt"
      << "R\\(Translation\\)"
      << "s"
      << "\\[eta\\]"
      << "Dr"
      << "R\\(Rotation\\)"
      ;

   map < QString, double > captures;

   for ( int i = 0; i < (int) caps.size(); ++i ) {
      QRegExp rx( caps[ i ] + "\\s+:\\s*(\\S+)" );

      if ( rx.indexIn( grpy_stdout ) == -1 ) {
         editor_msg( "red", QString( us_tr( "Could not find %1 file in GRPY output" ) ).arg( caps[ i ].replace( "\\", "" ) ) );
         grpy_captures[ caps[ i ] ].push_back( -9e99 );
      } else {
         grpy_captures[ caps[ i ] ].push_back( rx.cap( 1 ).toDouble() );
         us_qdebug( QString( "%1 : '%2'\n" ).arg( caps[ i ] ).arg( grpy_captures[ caps[ i ] ].back() ) );
      }
   }

   // accumulate data as in zeno (e.g. push values to data structures )
#endif

   // save stdout
   {
      QString grpy_out_name = grpy_last_processed.replace( QRegExp( ".bead_model$" ), ".grpy_res" );
      QFile f( grpy_out_name );
      if ( !f.open( QIODevice::WriteOnly ) ) {
         editor_msg( "red", QString( us_tr( "Error: could not open output file %1 for writing" ) ).arg( grpy_out_name ) );
      } else {
         QTextStream t( &f );
         t << grpy_stdout;
         editor_msg( "dark blue", QString( us_tr( "Wrote %1" ) ).arg( grpy_out_name ) );
         f.close();
      }
   }
   
   grpy_process_next();
}
   
void US_Hydrodyn::grpy_started()
{
   // us_qdebug( QString( "grpy_started %1" ).arg( grpy_filename ) );
   // editor_msg("brown", "GRPY launch exited\n");
   disconnect( grpy, SIGNAL(started()), 0, 0);
}

void US_Hydrodyn::grpy_finalize() {
   // us_qdebug( QString( "grpy_finalize %1" ).arg( grpy_filename ) );
   editor_msg( "black", "Finalizing GRPY results" );
   for ( map < QString, vector < double > >::iterator it = grpy_captures.begin();
         it != grpy_captures.end();
         ++it ) {
      editor_msg( "dark red",  US_Vector::qs_vector( it->first, it->second ) );
   }

   hydro_results grpy_results;
   hydro_results grpy_results2;

   grpy_results.method                = "Grpy";
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
   grpy_results.vbar                  = 0e0;
   grpy_results.asa_rg_pos            = 0e0;
   grpy_results.asa_rg_neg            = 0e0;
   grpy_results.ff0                   = 0e0;
   grpy_results.ff0_sd                = 0e0;

   grpy_results.solvent_name          = hydro.solvent_name;
   grpy_results.solvent_acronym       = hydro.solvent_acronym;
   grpy_results.solvent_viscosity     = hydro.solvent_viscosity;
   grpy_results.solvent_density       = hydro.solvent_density;
   grpy_results.temperature           = hydro.temperature;
   grpy_results.name                  = project;
   grpy_results.used_beads            = 0;
   grpy_results.used_beads_sd         = 0e0;
   grpy_results.total_beads           = 0;
   grpy_results.total_beads_sd        = 0e0;
   grpy_results.vbar                  = 0;

   grpy_results.num_models            = grpy_processed.size();

   grpy_results2 = grpy_results;

   map < int, map < QString, double > > data_to_save;
   
   for ( map < QString, vector < double > >::iterator it = grpy_captures.begin();
         it != grpy_captures.end();
         ++it ) {

      for ( int i = 0; i < (int) it->second.size(); ++i ) {
         data_to_save[ i ][ it->first ] = it->second[ i ];

         if ( it->first  == "M" ) {
            {
               grpy_results.mass += it->second[ i ];
               // grpy_results2.mass += it->second[ i ] * it->second[ i ];
            }
            break;
         }               
         if ( it->first == "v_bar" ) {
            grpy_results.vbar += it->second[ i ];
            // grpy_results2.vbar += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "R\\(Anhydrous\\)" ) {
            grpy_results.rs += it->second[ i ];
            grpy_results2.rs += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "Axial Ratio" ) {
            //    grpy_results.mass += it->second[ i ];
            //    grpy_results2.mass += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "f/fo" ) {
            grpy_results.ff0 += it->second[ i ];
            grpy_results2.ff0 += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "Dt" ) {
            grpy_results.D20w += it->second[ i ];
            grpy_results2.D20w += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "R\\(Translation\\)" ) {
            //    grpy_results.mass += it->second[ i ];
            //    grpy_results2.mass += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "s" ) {
            grpy_results.s20w += it->second[ i ] * 1e13;
            grpy_results2.s20w += it->second[ i ] * it->second[ i ] * 1e26;
         }

         if ( it->first == "\\[eta\\]" ) {
            grpy_results.viscosity += it->second[ i ];
            grpy_results2.viscosity += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "Dr" ) {
            //    grpy_results.mass += it->second[ i ];
            //    grpy_results2.mass += it->second[ i ] * it->second[ i ];
         }
         if ( it->first == "R\\(Rotation\\)" ) {
            //    grpy_results.mass += it->second[ i ];
            //    grpy_results2.mass += it->second[ i ] * it->second[ i ];
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
      this_data.results.method                = "Grpy";
      this_data.results.mass                  = 0e0;
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
      this_data.results.name                  = QString( "%1-%1" ).arg( QFileInfo( grpy_filename ).completeBaseName() ).arg( it->first + 1 );
      this_data.results.used_beads            = 0;
      this_data.results.used_beads_sd         = 0e0;
      this_data.results.total_beads           = 0;
      this_data.results.total_beads_sd        = 0e0;
      this_data.results.vbar                  = 0;

      if ( it->second.count( "M" ) ) {
         this_data.results.mass = it->second[ "M" ];
      }

      if ( it->second.count( "v_bar" ) ) {
         this_data.results.vbar = it->second[ "v_bar" ];
      }

      if ( it->second.count( "R\\(Anhydrous\\)" ) ) {
         this_data.results.rs = it->second[ "R\\(Anhydrous\\)" ];
      }

      if ( it->second.count( "Axial Ratio" ) ) {
         // this_data.results.mass = it->second[ "Axial Ratio" ];
      }

      if ( it->second.count( "f/fo" ) ) {
         this_data.results.ff0 = it->second[ "f/fo" ];
      }

      if ( it->second.count( "Dt" ) ) {
         this_data.results.D20w = it->second[ "Dt" ];
      }

      if ( it->second.count( "R\\(Translation\\)" ) ) {
         // this_data.results.mass = it->second[ "R\\(Translation\\)" ];
      }

      if ( it->second.count( "s" ) ) {
         this_data.results.s20w = it->second[ "s" ] * 1e13;
      }

      if ( it->second.count( "\\[eta\\]" ) ) {
         this_data.results.viscosity = it->second[ "\\[eta\\]" ];
      }

      if ( it->second.count( "Dr" ) ) {
         // this_data.results.mass = it->second[ "Dr" ];
      }

      if ( it->second.count( "R\\(Rotation\\)" ) ) {
         // this_data.results.mass = it->second[ "R\\(Rotation\\)" ];
      }
      
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

   {
      double num = (double) grpy_results.num_models;
      if ( num <= 1 ) {
         results = grpy_results;
      } else {
         grpy_results.name = QFileInfo( grpy_filename ).baseName();
         double numinv = 1e0 / num;
         grpy_results.mass          *= numinv;
         grpy_results.s20w          *= numinv;
         grpy_results.D20w          *= numinv;
         grpy_results.viscosity     *= numinv;
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
            grpy_results.rs_sd             = sqrt( fabs( ( grpy_results2.rs          - grpy_results.rs          * grpy_results.rs          * num ) * numdecinv ) );
            grpy_results.rg_sd             = sqrt( fabs( ( grpy_results2.rg          - grpy_results.rg          * grpy_results.rg          * num ) * numdecinv ) );
            grpy_results.ff0_sd            = sqrt( fabs( ( grpy_results2.ff0         - grpy_results.ff0         * grpy_results.ff0         * num ) * numdecinv ) );
            grpy_results.used_beads_sd     = sqrt( fabs( ( grpy_results2.used_beads  - grpy_results.used_beads  * grpy_results.used_beads  * num ) * numdecinv ) );
            grpy_results.total_beads_sd    = sqrt( fabs( ( grpy_results2.total_beads - grpy_results.total_beads * grpy_results.total_beads * num ) * numdecinv ) );
            
            results = grpy_results;
         }
      }
   }

   pb_show_hydro_results->setEnabled( true );
   grpy_running = false;
   pb_calc_hydro->setEnabled(true);
   pb_calc_zeno->setEnabled(true);
   pb_bead_saxs->setEnabled(true);
   pb_calc_grpy->setEnabled( true );
   pb_calc_hullrad->setEnabled( true );
   pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   pb_show_hydro_results->setEnabled(false);
   progress->reset();
   editor_msg( "black", "GRPY finished" );

   // us_qdebug( QString( "grpy_finalize %1 end" ).arg( grpy_filename ) );
}
