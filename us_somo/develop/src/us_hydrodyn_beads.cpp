// us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o
// us_hydrodyn_info.cpp contains code to report structures for debugging
// us_hydrodyn_util.cpp contains other various code, such as disulfide code
// us_hydrodyn_load.cpp contains code to load files 
// us_hydrodyn_grpy.cpp contains code for grpy interface
// (this) us_hydrodyn_beads.cpp contains code for generating bead models

#include "../include/us_hydrodyn.h"
#include "../include/us_surfracer.h"
#include "../include/us_hydrodyn_grid_atob.h"
#include "../include/us_hydrodyn_pat.h"
#include "../include/us_hydrodyn_asab1.h"

#define SLASH        "/"
#define DOTSOMO      ""


int US_Hydrodyn::calc_somo_o()
{
   return calc_somo( true );
}

int US_Hydrodyn::calc_somo( bool no_ovlp_removal )
{
   progress->set_cli_prefix( "bm" );
   citation_build_bead_model( no_ovlp_removal ? "somo-overlaps" : "somo-no-overlaps" );
   {
      int models_selected = 0;
      for(int i = 0; i < lb_model->count(); i++) {
         if (lb_model->item(i)->isSelected()) {
            models_selected++;
         }
      }
      if ( !models_selected ) {
         editor_msg( "black", "No models selected!\n" );
         return -1;
      }
   }

   if ( selected_models_contain( "WAT" ) )
   {
      QMessageBox::warning( this,
                            us_tr( "Selected model contains WAT residue" ),
                            us_tr( 
                               "Can not process models that contain the WAT residue.\n"
                               "These are currently generated only for SAXS/SANS computations\n"
                               )
                            );
      return -1;
   }

   if ( selected_models_contain( "XHY" ) )
   {
      QMessageBox::warning( this,
                            us_tr( "Selected model contains XHY residue" ),
                            us_tr( 
                               "Can not process models that contain the XHY residue.\n"
                               "These are currently generated only for SAXS/SANS computations\n"
                               )
                            );
      return -1;
   }

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);

   bd_anaflex_enables(false);

   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_grid->setEnabled(false);
   model_vector = model_vector_as_loaded;
   sync_pdb_info( "calc_somo" );
   editor_msg( "dark blue", QString( us_tr( "Peptide Bond Rule is %1 for this PDB" ) ).arg( misc.pb_rule_on ? "on" : "off" ) );
   options_log = "";
   no_ovlp_removal ? append_options_log_somo_ovlp() : append_options_log_somo();
   display_default_differences();
   bead_model_suffix = getExtendedSuffix(false, true, no_ovlp_removal );
   le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );
   if ( !overwrite )
   {
      setSomoGridFile(true);
   }

   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      bd_anaflex_enables( ( ( browflex && browflex->state() == QProcess::Running ) ||
                            ( anaflex && anaflex->state() == QProcess::Running ) ) ? false : true );
      pb_somo->setEnabled(true);
      pb_somo_o->setEnabled(true);
      pb_grid_pdb->setEnabled(true);
      pb_vdw_beads->setEnabled(true);
      progress->reset();
      return -1;
   }

   if (!residue_list.size() ||
       !model_vector.size())
   {
      fprintf(stderr, "calculations can not be run until residue & pdb files are read!\n");
      pb_stop_calc->setEnabled(false);
      return -1;
   }
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled( false );
   pb_calc_hullrad->setEnabled( false );
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }
   bool any_errors = false;
   bool any_models = false;
   somo_processed.resize(lb_model->count());
   bead_models.resize(lb_model->count());
   QString msg = QString("\n%1 models selected:").arg(project);
   for(int i = 0; i < lb_model->count(); i++) {
      somo_processed[i] = 0;
      if (lb_model->item(i)->isSelected()) {
         current_model = i;
         // msg += QString( " %1" ).arg( i + 1 );
         msg += " " + model_name( i );
      }
   }
   msg += "\n";
   editor->append(msg);

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++)
   {
      if (!any_errors && lb_model->item(current_model)->isSelected())
      {
         any_models = true;
         if(!compute_asa( false, no_ovlp_removal ))
         {
            model_vector[ current_model ].asa_rg_pos = results.asa_rg_pos;
            model_vector[ current_model ].asa_rg_neg = results.asa_rg_neg;
            // qDebug() << "us_hydrodyn::calc_somo() current_model " << current_model << " asa_rg +/- " << results.asa_rg_pos << " " << results.asa_rg_neg;

            somo_processed[current_model] = 1;
            if ( asa.recheck_beads ) // && !no_ovlp_removal )
            {
               // puts("recheck beads disabled");
               editor->append("Rechecking beads\n");
               qApp->processEvents();

               bead_check(false, false);
               editor->append("Finished rechecking beads\n");
               progress->setValue(19);
            }
            else
            {
               editor->append("No rechecking of beads\n");
               qApp->processEvents();
            }
            bead_models[current_model] = bead_model;
         }
         else
         {
            model_vector[ current_model ].asa_rg_pos = 0;
            model_vector[ current_model ].asa_rg_neg = 0;
            any_errors = true;
         }
      }
      if (stopFlag)
      {
         editor->append("Stopped by user\n\n");
         pb_somo->setEnabled(true);
         pb_somo_o->setEnabled(true);
         bd_anaflex_enables( ( ( browflex && browflex->state() == QProcess::Running ) ||
                               ( anaflex && anaflex->state() == QProcess::Running ) ) ? false : true );
         pb_grid_pdb->setEnabled(true);
         pb_vdw_beads->setEnabled(true);
         progress->reset();
         return -1;
      }

      // calculate bead model and generate hydrodynamics calculation output
      // if successful, enable follow-on buttons:
   }
   if (any_models && !any_errors)
   {
      editor->append("Build bead model completed\n");
      qApp->processEvents();
      pb_visualize->setEnabled(true);
      pb_equi_grid_bead_model->setEnabled(true);
      pb_calc_hydro->setEnabled( !no_ovlp_removal );
      pb_calc_zeno->setEnabled(true);
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_somo->setEnabled(true);
   pb_somo_o->setEnabled(true);
   pb_grid_pdb->setEnabled(true);
   pb_vdw_beads->setEnabled(true);
   pb_grid->setEnabled(true);
   pb_stop_calc->setEnabled(false);
   if (calcAutoHydro) {
      switch ( misc.auto_calc_hydro_method ) {
      case AUTO_CALC_HYDRO_SMI :
         no_ovlp_removal ? 
            calc_zeno_hydro() : calc_hydro();
         break;
      case AUTO_CALC_HYDRO_ZENO :
         calc_zeno_hydro();
         break;
      case AUTO_CALC_HYDRO_GRPY :
         calc_grpy_hydro();
         break;
      default :
         editor_msg( "red", us_tr( "No known hydrodynamic method set for automatic hydrodynamic calculations\n"
                                   "Check SOMO->Miscellaneous Options->Automatically calculate hydrodynamics method" ) );
         break;
      }
   } 
   else
   {
      play_sounds(1);
   }
   // info_model_vector_mw( QString( "after calc_somo() : model_vector" ), model_vector, true );
   // info_bead_models_mw( QString( "after calc_somo() : model_vector" ), bead_models );
   progress->reset();
   return 0;
}

int US_Hydrodyn::calc_grid_pdb( bool no_ovlp_removal )
{
   citation_build_bead_model( no_ovlp_removal ? "atob-overlaps" : "atob-no-overlaps" );
   {
      int models_selected = 0;
      for(int i = 0; i < lb_model->count(); i++) {
         if (lb_model->item(i)->isSelected()) {
            models_selected++;
         }
      }
      if ( !models_selected ) {
         editor_msg( "black", "No models selected!\n" );
         return -1;
      }
   }

   //    if ( selected_models_contain( "WAT" ) )
   //    {
   //       QMessageBox::warning( this,
   //                             us_tr( "Selected model contains WAT residue" ),
   //                             us_tr( 
   //                                "Can not process models that contain the WAT residue.\n"
   //                                "These are currently generated only for SAXS/SANS computations"
   //                                )
   //                             );
   //       return -1;
   //    }

   if ( selected_models_contain( "XHY" ) )
   {
      QMessageBox::warning( this,
                            us_tr( "Selected model contains XHY residue" ),
                            us_tr( 
                               "Can not process models that contain the XHY residue.\n"
                               "These are currently generated only for SAXS/SANS computations\n"
                               )
                            );
      return -1;
   }

   overlap_reduction org_grid_exposed_overlap = grid_exposed_overlap;
   overlap_reduction org_grid_overlap         = grid_overlap;
   overlap_reduction org_grid_buried_overlap  = grid_buried_overlap;
   grid_options      org_grid                 = grid;

   if ( grid.create_nmr_bead_pdb )
   {
      if ( 
          (
           grid_exposed_overlap.remove_overlap ||
           grid_overlap.        remove_overlap ||
           grid_buried_overlap. remove_overlap
           ) &&
          (
           grid_exposed_overlap.translate_out ||
           grid_overlap.translate_out         ||
           grid_buried_overlap.translate_out 
           )
          )
      {
         grid_exposed_overlap.translate_out = false;
         grid_overlap.translate_out         = false;
         grid_buried_overlap.translate_out  = false;
         QString msg = "Temporarily turning off outward translation";
         if ( !grid.enable_asa )
         {
            grid.enable_asa = true;
            msg += " and enabling ASA screening";
         }
            
         QMessageBox::information( this,
                                   "US-SOMO: Build AtoB models with structure factors",
                                   us_tr( msg ) );

         display_default_differences();
      }
   }

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   model_vector = model_vector_as_loaded;
   sync_pdb_info( "calc_grid_pdb" );
   editor_msg( "dark blue", QString( us_tr( "Peptide Bond Rule is %1 for this PDB" ) ).arg( misc.pb_rule_on ? "on" : "off" ) );
   options_log = "";
   no_ovlp_removal ? append_options_log_atob_ovlp() : append_options_log_atob();
   display_default_differences();
   int flag = 0;
   bool any_errors = false;
   bool any_models = false;
   pb_grid->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled( false );
   pb_calc_hullrad->setEnabled( false );

   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }

   bead_model_suffix = getExtendedSuffix(false, false, no_ovlp_removal);
   le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );
   if ( !overwrite )
   {
      setSomoGridFile(false);
   }

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         printf("in calc_grid: is selected current model %d\n", current_model); fflush(stdout);
         {
            QString error_string;
            printf("in calc_grid: !somo_processed %d\n", current_model); fflush(stdout);
            // create bead model from atoms
            editor->append(QString("Gridding atom model %1\n").arg(current_model + 1));
            qApp->processEvents();
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               grid_exposed_overlap = org_grid_exposed_overlap;
               grid_overlap         = org_grid_overlap;
               grid_buried_overlap  = org_grid_buried_overlap;
               grid                 = org_grid;
               return -1;
            }
            // compute maximum position for progress
            int mpos =
               2 // create beads
               + 4 // grid_atob
               + (grid.enable_asa ?
                  ( 1 
                    + 10 // radial reduction
                    + ( asa.recheck_beads ? 2 : 0 )
                    )
                  : 0 )
               + ( ( !grid.enable_asa && grid_overlap.remove_overlap ) ?
                   ( 10 // radial reduction
                     + ( asa.recheck_beads ? 2 : 0 )
                     )
                   : 0 )
               + 1 // finish off
               ;
               
            progress->reset();
            progress->setMaximum(mpos);
            progress->setValue(progress->value() + 1);

            int retval = create_beads(&error_string);

            {
               vector < PDB_atom > active_atoms;
               for (unsigned int j = 0; j < model_vector[ current_model ].molecule.size (); j++) {
                  for (unsigned int k = 0; k < model_vector[ current_model ].molecule[j].atom.size (); k++) {
                     PDB_atom *this_atom = &(model_vector[ current_model ].molecule[j].atom[k]);
                     if ( this_atom->active ) {
                        active_atoms.push_back( *this_atom );
                     }
                  }
               }
               compute_asa_rgs( active_atoms );
               model_vector[ current_model ].asa_rg_pos = results.asa_rg_pos;
               model_vector[ current_model ].asa_rg_neg = results.asa_rg_neg;
            }
               
            progress->setValue(progress->value() + 1);
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               grid_exposed_overlap = org_grid_exposed_overlap;
               grid_overlap         = org_grid_overlap;
               grid_buried_overlap  = org_grid_buried_overlap;
               grid                 = org_grid;
               return -1;
            }
            if ( retval )
            {
               editor->append("Errors found during the initial creation of beads\n");
               qApp->processEvents();
               any_errors = true;
               switch ( retval )
               {
               case US_SURFRACER_ERR_MISSING_RESIDUE:
                  {
                     printError("Encountered an unknown residue:\n" +
                                error_string);
                     break;
                  }
               case US_SURFRACER_ERR_MISSING_ATOM:
                  {
                     printError("Encountered a unknown atom:\n" +
                                error_string);
                     break;
                  }
               case US_SURFRACER_ERR_MEMORY_ALLOC:
                  {
                     printError("Encountered a memory allocation error");
                     break;
                  }
               default:
                  {
                     printError("Encountered an unknown error");
                     // unknown error
                     break;
                  }
               }
            }
            else
            {
               if(error_string.length()) {
                  printError("Encountered unknown atom(s) error:\n" +
                             error_string);
                  any_errors = true;
               }
               else
               {
                  // ok, we have the basic "bead" info loaded...
                  unsigned int i = current_model;
                  bead_model.clear( );
                  bool any_zero_si = false;
                  for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
                     for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
                        PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
                        if(this_atom->active) {
                           for (unsigned int m = 0; m < 3; m++) {
                              this_atom->bead_coordinate.axis[m] = this_atom->coordinate.axis[m];
                           }
                           this_atom->bead_number = bead_model.size();
                           this_atom->bead_computed_radius = this_atom->radius;
                           if ( grid.hydrate && this_atom->atom_hydration ) 
                           {
                              double additional_radius = 
                                 pow(3 * this_atom->atom_hydration * misc.hydrovol / (M_PI * 4), 1.0/3.0);
                              this_atom->bead_computed_radius += additional_radius;
#if defined(GRID_HYDRATE_DEBUG)
                              printf("hydrating atom %s %s %s hydration %f radius %f + %f -> %f\n"
                                     , this_atom->name.toLatin1().data()
                                     , this_atom->resName.toLatin1().data()
                                     , this_atom->resSeq.toLatin1().data()
                                     , this_atom->atom_hydration
                                     , this_atom->radius
                                     , additional_radius
                                     , this_atom->bead_computed_radius 
                                     );
#endif
                           }

                           this_atom->bead_actual_radius    = this_atom->bead_computed_radius;
                           this_atom->bead_mw               = this_atom->mw;
                           this_atom->bead_ionized_mw_delta = this_atom->ionized_mw_delta;
                           if ( this_atom->si == 0e0 )
                           {
                              any_zero_si = true;
                           }
                           bead_model.push_back(*this_atom);
                        }
                     }
                  }
                  if (bead_models.size() < current_model + 1) {
                     bead_models.resize(current_model + 1);
                  }

                  progress->setValue( progress->value() + 1 );
                  int save_progress = progress->value();
                  int save_total_steps = progress->maximum();
                  if ( grid.center == 2  && any_zero_si ) // ssi
                  {
                     editor_msg( "red", "Center of scattering requested, but zero scattering intensities are present" );
                     pb_grid_pdb->setEnabled(true);
                     pb_vdw_beads->setEnabled(true);
                     pb_somo->setEnabled(true);
                     pb_somo_o->setEnabled(true);
                     progress->reset();
                     grid_exposed_overlap = org_grid_exposed_overlap;
                     grid_overlap         = org_grid_overlap;
                     grid_buried_overlap  = org_grid_buried_overlap;
                     grid                 = org_grid;
                     return -1;
                  }
                     
                  bead_models[current_model] =
                     us_hydrodyn_grid_atob(&bead_model,
                                           &grid,
                                           progress,
                                           editor,
                                           this);
                  progress->setMaximum( save_total_steps );
                  progress->setValue( save_progress + 1 );
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid_pdb->setEnabled(true);
                     pb_vdw_beads->setEnabled(true);
                     pb_somo->setEnabled(true);
                     pb_somo_o->setEnabled(true);
                     progress->reset();
                     grid_exposed_overlap = org_grid_exposed_overlap;
                     grid_overlap         = org_grid_overlap;
                     grid_buried_overlap  = org_grid_buried_overlap;
                     grid                 = org_grid;
                     return -1;
                  }
                  if (errorFlag)
                  {
                     editor->append("Error occured\n\n");
                     pb_grid_pdb->setEnabled(true);
                     pb_vdw_beads->setEnabled(true);
                     pb_somo->setEnabled(true);
                     pb_somo_o->setEnabled(true);
                     progress->reset();
                     grid_exposed_overlap = org_grid_exposed_overlap;
                     grid_overlap         = org_grid_overlap;
                     grid_buried_overlap  = org_grid_buried_overlap;
                     grid                 = org_grid;
                     return -1;
                  }
#if defined(DEBUG)
                  printf("back from grid_atob 0\n"); fflush(stdout);
#endif
                  if (somo_processed.size() < current_model + 1) {
                     somo_processed.resize(current_model + 1);
                  }
                  bead_model = bead_models[current_model];

                  any_models = true;
                  somo_processed[current_model] = 1;
#if defined(DEBUG)
                  printf("back from grid_atob 1\n"); fflush(stdout);
#endif
                  if ( grid.enable_asa )
                  {
                     editor->append("ASA check\n");
                     qApp->processEvents();
                     // set all beads buried
                     for(unsigned int i = 0; i < bead_model.size(); i++) {
                        bead_model[i].exposed_code = 6;
                        bead_model[i].bead_color = 6;
                        bead_model[i].chain = 1; // all 'side' chain
                     }
                     double save_threshold = asa.threshold;
                     double save_threshold_percent = asa.threshold_percent;
                     asa.threshold = asa.grid_threshold;
                     asa.threshold_percent = asa.grid_threshold_percent;
                     progress->setValue(progress->value() + 1);
                     bead_check(true, true);
                     progress->setValue(progress->value() + 1);
                     asa.threshold = save_threshold;
                     asa.threshold_percent = save_threshold_percent;
                     bead_models[current_model] = bead_model;
#if defined(DEBUG)
                     for(unsigned int i = 0; i < bead_model.size(); i++) {
                        printf("after asa bead %d exposed %d color %d chain %d active %d mw %f vol %f cr %f [%f,%f,%f]\n",
                               i,
                               bead_model[i].exposed_code,
                               bead_model[i].bead_color,
                               bead_model[i].chain,
                               bead_model[i].active,
                               bead_model[i].bead_ref_mw,
                               bead_model[i].bead_ref_volume,
                               bead_model[i].bead_computed_radius,
                               bead_model[i].bead_coordinate.axis[0],
                               bead_model[i].bead_coordinate.axis[1],
                               bead_model[i].bead_coordinate.axis[2]

                               );
                     }
#endif
                     // now apply radial reduction with outward translation using

                     if ( !no_ovlp_removal ) {
                        // grid_exposed/buried_overlap
                        overlap_reduction save_sidechain_overlap = sidechain_overlap;
                        overlap_reduction save_mainchain_overlap = mainchain_overlap;
                        overlap_reduction save_buried_overlap = buried_overlap;
                        sidechain_overlap = grid_exposed_overlap;
                        mainchain_overlap = grid_exposed_overlap;
                        buried_overlap = grid_buried_overlap;
                        progress->setValue(progress->value() + 1);

                        radial_reduction( true );
                        sidechain_overlap = save_sidechain_overlap;
                        mainchain_overlap = save_mainchain_overlap;
                        buried_overlap = save_buried_overlap;

                        bead_models[current_model] = bead_model;
                        // grid_buried_overlap

                        if (asa.recheck_beads)
                        {
                           editor->append("Rechecking beads\n");
                           qApp->processEvents();
                           double save_threshold = asa.threshold;
                           double save_threshold_percent = asa.threshold_percent;
                           asa.threshold = asa.grid_threshold;
                           asa.threshold_percent = asa.grid_threshold_percent;
                           progress->setValue(progress->value() + 1);
                           bead_check(false, false);
                           progress->setValue(progress->value() + 1);
                           asa.threshold = save_threshold;
                           asa.threshold_percent = save_threshold_percent;
                           bead_models[current_model] = bead_model;
                        }
                     }
                  }
                  else
                  {

                     if ( !no_ovlp_removal ) {
                        if (grid_overlap.remove_overlap)
                        {
                           progress->setValue(progress->value() + 1);
                           radial_reduction( true );
                           progress->setValue(progress->value() + 1);
                           bead_models[current_model] = bead_model;
                        }
                        if (stopFlag)
                        {
                           editor->append("Stopped by user\n\n");
                           pb_grid_pdb->setEnabled(true);
                           pb_vdw_beads->setEnabled(true);
                           pb_somo->setEnabled(true);
                           pb_somo_o->setEnabled(true);
                           progress->reset();
                           grid_exposed_overlap = org_grid_exposed_overlap;
                           grid_overlap         = org_grid_overlap;
                           grid_buried_overlap  = org_grid_buried_overlap;
                           grid                 = org_grid;
                           return -1;
                        }
                     }
                     if (asa.recheck_beads)
                     {
                        editor->append("Rechecking beads\n");
                        qApp->processEvents();
                        // all buried
                        for(unsigned int i = 0; i < bead_model.size(); i++) {
                           bead_model[i].exposed_code = 6;
                           bead_model[i].bead_color = 6;
                        }
                        double save_threshold = asa.threshold;
                        double save_threshold_percent = asa.threshold_percent;
                        asa.threshold = asa.grid_threshold;
                        asa.threshold_percent = asa.grid_threshold_percent;
                        progress->setValue(progress->value() + 1);
                        bead_check(false, false);
                        progress->setValue(progress->value() + 1);
                        asa.threshold = save_threshold;
                        asa.threshold_percent = save_threshold_percent;
                        bead_models[current_model] = bead_model;
                     }
                     else
                     {
                        // all exposed
                        for(unsigned int i = 0; i < bead_model.size(); i++) {
                           bead_model[i].exposed_code = 1;
                           bead_model[i].bead_color = 8;
                        }
                        bead_models[current_model] = bead_model;
                     }
                     if (stopFlag)
                     {
                        editor->append("Stopped by user\n\n");
                        pb_grid_pdb->setEnabled(true);
                        pb_vdw_beads->setEnabled(true);
                        pb_somo->setEnabled(true);
                        pb_somo_o->setEnabled(true);
                        progress->reset();
                        grid_exposed_overlap = org_grid_exposed_overlap;
                        grid_overlap         = org_grid_overlap;
                        grid_buried_overlap  = org_grid_buried_overlap;
                        grid                 = org_grid;
                        return -1;
                     }
                  }

                  QString extra_text = "";
                  if ( grid.create_nmr_bead_pdb &&
                       sf_factors.saxs_name != "undefined" &&
                       !sf_factors.saxs_name.isEmpty() )
                  {
                     extra_text = 
                        QString( "\nSAXS exponential fitting information\n"
                                 "    Global average 4 term fit: %1\n"
                                 "    Global average 5 term fit: %2\n\n" )
                        .arg( sf_4term_notes )
                        .arg( sf_5term_notes )
                        ;

                     float tot_excl_vol = 0.0f;
                     for ( unsigned int k = 0; k < sf_bead_factors.size(); k++ )
                     {
                        extra_text += "BSAXS:: " + sf_bead_factors[ k ].saxs_name.toUpper();
                        for ( unsigned int i = 0; i < 4; i++ )
                        {
                           extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].a[ i ] );
                           extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].b[ i ] );
                        }
                        extra_text += QString( " %1 %2\n" ).arg( sf_bead_factors[ k ].c ).arg( bead_model[ k ].saxs_excl_vol );
                       
                        extra_text += "BSAXS:: " + sf_bead_factors[ k ].saxs_name;
                        for ( unsigned int i = 0; i < 5; i++ )
                        {
                           extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].a5[ i ] );
                           extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].b5[ i ] );
                        }
                        extra_text += QString( " %1 %2\n" ).arg( sf_bead_factors[ k ].c5 ).arg( bead_model[ k ].saxs_excl_vol );
                        tot_excl_vol += bead_model[ k ].saxs_excl_vol;

                        extra_text += "BSAXSV:: " + sf_bead_factors[ k ].saxs_name;
                        for ( unsigned int i = 0; i < sf_bead_factors[ k ].vcoeff.size(); i++ )
                        {
                           extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].vcoeff[ i ] );
                        }
                        extra_text += QString( " %1\n" ).arg( bead_model[ k ].saxs_excl_vol );
                        tot_excl_vol += bead_model[ k ].saxs_excl_vol;
                     }

                     extra_text += "\nSAXS:: " + sf_factors.saxs_name.toUpper();
                     for ( unsigned int i = 0; i < 4; i++ )
                     {
                        extra_text += QString( " %1" ).arg( sf_factors.a[ i ] );
                        extra_text += QString( " %1" ).arg( sf_factors.b[ i ] );
                     }
                     extra_text += QString( " %1 %2\n" ).arg( sf_factors.c ).arg( tot_excl_vol );
                       
                     extra_text += "SAXS:: " + sf_factors.saxs_name;
                     for ( unsigned int i = 0; i < 5; i++ )
                     {
                        extra_text += QString( " %1" ).arg( sf_factors.a5[ i ] );
                        extra_text += QString( " %1" ).arg( sf_factors.b5[ i ] );
                     }
                     extra_text += QString( " %1 %2\n" ).arg( sf_factors.c5 ).arg( tot_excl_vol );

                     if ( extra_saxs_coefficients.count( sf_factors.saxs_name ) )
                     {
                        editor_msg( "dark red", QString( "Notice: extra saxs coefficients %1 replaced\n" ).arg( sf_factors.saxs_name ) );
                     } else {
                        saxs_options.dummy_saxs_names.push_back( sf_factors.saxs_name );
                     }
                     extra_saxs_coefficients[ sf_factors.saxs_name ] = sf_factors;
                     saxs_options.dummy_saxs_name = sf_factors.saxs_name;
                     editor_msg( "blue", QString( "Saxs name for dummy atom models set to %1" ).arg( saxs_options.dummy_saxs_name ) );

                     if ( saxs_util->saxs_map.count( sf_factors.saxs_name ) )
                     {
                        editor_msg( "dark red", 
                                    QString( us_tr( "Notice: saxs coefficients for %1 replaced by newly loaded values\n" ) )
                                    .arg( sf_factors.saxs_name ) );
                     } else {
                        saxs_util->saxs_list.push_back( sf_factors );
                        editor_msg( "dark blue", 
                                    QString( us_tr( "Notice: added coefficients for %1 from newly loaded values\n" ) )
                                    .arg( sf_factors.saxs_name ) );
                     }
                     saxs_util->saxs_map[ sf_factors.saxs_name ] = sf_factors;

                     if ( saxs_plot_widget )
                     {
                        if ( saxs_plot_window->saxs_map.count( sf_factors.saxs_name ) )
                        {
                           editor_msg( "dark red", 
                                       QString( us_tr( "Notice: saxs window: saxs coefficients for %1 replaced by newly loaded values\n" ) )
                                       .arg( sf_factors.saxs_name ) );
                        } else {
                           saxs_plot_window->saxs_list.push_back( sf_factors );
                           editor_msg( "dark blue", 
                                       QString( us_tr( "Notice: saxs window: added coefficients for %1 from newly loaded values\n" ) )
                                       .arg( sf_factors.saxs_name ) );
                        }
                        saxs_plot_window->saxs_map[ sf_factors.saxs_name ] = sf_factors;
                     }
                  }

                  if ( saxs_options.compute_saxs_coeff_for_bead_models && grid.create_nmr_bead_pdb )
                  {
                     if ( !saxs_options.iq_global_avg_for_bead_models && sf_bead_factors.size() != bead_model.size() )
                     {
                        editor_msg( "red", 
                                    QString( us_tr( "Overriding setting to use global structure factors since bead model doesn't contain the correct number of structure factors (%1) for the beads (%2)" ) )
                                    .arg( sf_bead_factors.size() )
                                    .arg( bead_model.size() )
                                    );
                     }
                     if ( saxs_options.iq_global_avg_for_bead_models || sf_bead_factors.size() != bead_model.size() )
                     {
                        if ( !saxs_util->saxs_map.count( saxs_options.dummy_saxs_name ) )
                        {
                           editor_msg( "red", QString( us_tr("Warning: No '%1' SAXS atom found.\n" ) )
                                       .arg( saxs_options.dummy_saxs_name ) );
                           for(unsigned int i = 0; i < bead_model.size(); i++) {
                              bead_model[i].saxs_data.saxs_name = "";
                           }
                        } else {
                           editor_msg( "blue", QString( us_tr("Notice: Loading beads with saxs coefficients '%1'" ) )
                                       .arg( saxs_options.dummy_saxs_name ) );
                           for( unsigned int i = 0; i < bead_model.size(); i++ ) 
                           {
                              bead_model[i].saxs_name = saxs_options.dummy_saxs_name;
                              bead_model[i].saxs_data = saxs_util->saxs_map[ saxs_options.dummy_saxs_name ];
                              bead_model[i].hydrogens = 0;
                           }
                        }
                     } else {
                        if ( !saxs_options.iq_global_avg_for_bead_models && sf_bead_factors.size() == bead_model.size() )
                        {
                           editor_msg( "blue", us_tr("Notice: Loading beads with bead computed structure factors" ) );
                           for( unsigned int i = 0; i < bead_model.size(); i++ ) 
                           {
                              bead_model[i].saxs_name = sf_bead_factors[ i ].saxs_name;
                              bead_model[i].saxs_data = sf_bead_factors[ i ];
                              bead_model[i].hydrogens = 0;
                           }
                        }
                     }
                     bead_models[current_model] = bead_model;
                  }                    

                  editor->append( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );
                  progress->setValue(progress->value() + 1);

                  // write_bead_spt(somo_dir + SLASH + project +
                  //       (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
                  //       QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
                  //       DOTSOMO, &bead_model, bead_model_from_file);

                  QString filename = 
                     project + 
                     QString( "_%1" ).arg( current_model + 1 );

                  le_bead_model_file->setText( filename );

                  write_bead_model( 
                                   get_somo_dir() + SLASH + filename +
                                   QString( bead_model_suffix.length() ? ( "-" + bead_model_suffix ) : "" ) +
                                   DOTSOMO, 
                                   &bead_model,
                                   extra_text
                                   );

               }
            }
            progress->setValue(progress->maximum());
         }
      }
   }

   grid_exposed_overlap = org_grid_exposed_overlap;
   grid_overlap         = org_grid_overlap;
   grid_buried_overlap  = org_grid_buried_overlap;
   grid                 = org_grid;

   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_grid_pdb->setEnabled(true);
      pb_vdw_beads->setEnabled(true);
      pb_somo->setEnabled(true);
      pb_somo_o->setEnabled(true);
      progress->reset();
      return -1;
   }

   if (any_models && !any_errors)
   {
      editor->append("Build bead model completed\n");
      qApp->processEvents();
      pb_visualize->setEnabled(true);
      pb_equi_grid_bead_model->setEnabled(true);
      pb_calc_hydro->setEnabled( !no_ovlp_removal );
      pb_calc_zeno->setEnabled(true);
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_grid_pdb->setEnabled(true);
   pb_vdw_beads->setEnabled(true);
   pb_grid->setEnabled(true);
   pb_somo->setEnabled(true);
   pb_somo_o->setEnabled(true);
   pb_stop_calc->setEnabled(false);
   if (calcAutoHydro) {
      switch ( misc.auto_calc_hydro_method ) {
      case AUTO_CALC_HYDRO_SMI :
         calc_hydro();
         break;
      case AUTO_CALC_HYDRO_ZENO :
         calc_zeno_hydro();
         break;
      case AUTO_CALC_HYDRO_GRPY :
         calc_grpy_hydro();
         break;
      default :
         editor_msg( "red", us_tr( "No known hydrodynamic method set for automatic hydrodynamic calculations\n"
                                   "Check SOMO->Miscellaneous Options->Automatically calculate hydrodynamics method" ) );
         break;
      }
   } 
   else
   {
      play_sounds(1);
   }

   return (flag);
}

int US_Hydrodyn::calc_grid()
{
   {
      int models_selected = 0;
      for(int i = 0; i < lb_model->count(); i++) {
         if (lb_model->item(i)->isSelected()) {
            models_selected++;
         }
      }
      if ( !models_selected ) {
         editor_msg( "black", "No models selected!\n" );
         return -1;
      }
   }

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   append_options_log_atob();
   display_default_differences();
   int flag = 0;
   bool any_errors = false;
   bool any_models = false;
   bool grid_pdb_state = pb_grid_pdb->isEnabled();
   bool somo_state = pb_grid_pdb->isEnabled();
   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_grid->setEnabled(false);
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled( false );
   pb_calc_hullrad->setEnabled( false );
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }

   bead_model_suffix = getExtendedSuffix(false, false) + "g";
   le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );
   if ( !overwrite )
   {
      setSomoGridFile(false);
   }

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         printf("in calc_grid: is selected current model %d\n", current_model); fflush(stdout);
         if (somo_processed.size() > current_model && somo_processed[current_model]) {
            printf("in calc_grid: somo_processed %d\n", current_model); fflush(stdout);
            editor->append(QString("Gridding bead model %1\n").arg(current_model + 1));
            // compute maximum position for progress
            int mpos =
               3 // grid_atob
               + (grid.enable_asa ?
                  ( 1 
                    + 10 // radial reduction
                    + ( asa.recheck_beads ? 2 : 0 )
                    )
                  : 0 )
               + ( ( !grid.enable_asa && grid_overlap.remove_overlap ) ?
                   ( 10 // radial reduction
                     + ( asa.recheck_beads ? 2 : 0 )
                     )
                   : 0 )
               + 1 // finish off
               ;
               
            progress->reset();
            progress->setMaximum(mpos);
            progress->setValue(progress->value() + 1);
            qApp->processEvents();

            if ( grid.center == 2 ) // ssi
            {
               bool any_zero_si = false;
               for ( unsigned int i = 0; i <  bead_models[ current_model ].size(); i++ )
               {
                  if ( bead_models[ current_model ][ i ].si == 0e0 )
                  {
                     any_zero_si = true;
                     break;
                  }
               }
               if ( any_zero_si )
               {
                  editor_msg( "red", "Center of scattering requested, but zero scattering intensities are present" );
                  pb_grid_pdb->setEnabled(true);
                  pb_vdw_beads->setEnabled(true);
                  pb_somo->setEnabled(true);
                  pb_somo_o->setEnabled(true);
                  progress->reset();
                  return -1;
               }
            }

            bead_models[current_model] =
               us_hydrodyn_grid_atob(&bead_models[current_model],
                                     &grid,
                                     progress,
                                     editor,
                                     this);
            progress->setValue(progress->value() + 1);

            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               return -1;
            }
            if (errorFlag)
            {
               editor->append("Error occured\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               return -1;
            }
            bead_model = bead_models[current_model];
            any_models = true;
            if (somo_processed.size() < current_model + 1) {
               somo_processed.resize(current_model + 1);
            }
            somo_processed[current_model] = 1;
            if ( grid.enable_asa )
            {
               editor->append("ASA check\n");
               qApp->processEvents();
               // set all beads buried
               for(unsigned int i = 0; i < bead_model.size(); i++) {
                  bead_model[i].exposed_code = 6;
                  bead_model[i].bead_color = 6;
                  bead_model[i].chain = 1; // all 'side' chain
               }
               double save_threshold = asa.threshold;
               double save_threshold_percent = asa.threshold_percent;
               asa.threshold = asa.grid_threshold;
               asa.threshold_percent = asa.grid_threshold_percent;
               progress->setValue(progress->value() + 1);
               bead_check(true, true);
               progress->setValue(progress->value() + 1);
               asa.threshold = save_threshold;
               asa.threshold_percent = save_threshold_percent;
               bead_models[current_model] = bead_model;
#if defined(DEBUG)
               for(unsigned int i = 0; i < bead_model.size(); i++) {
                  printf("after asa bead %d exposed %d color %d chain %d active %d mw %f vol %f cr %f [%f,%f,%f]\n",
                         i,
                         bead_model[i].exposed_code,
                         bead_model[i].bead_color,
                         bead_model[i].chain,
                         bead_model[i].active,
                         bead_model[i].bead_ref_mw,
                         bead_model[i].bead_ref_volume,
                         bead_model[i].bead_computed_radius,
                         bead_model[i].bead_coordinate.axis[0],
                         bead_model[i].bead_coordinate.axis[1],
                         bead_model[i].bead_coordinate.axis[2]

                         );
               }
#endif
               // now apply radial reduction with outward translation using

               // grid_exposed/buried_overlap
               overlap_reduction save_sidechain_overlap = sidechain_overlap;
               overlap_reduction save_mainchain_overlap = mainchain_overlap;
               overlap_reduction save_buried_overlap = buried_overlap;
               sidechain_overlap = grid_exposed_overlap;
               mainchain_overlap = grid_exposed_overlap;
               buried_overlap = grid_buried_overlap;
               progress->setValue(progress->value() + 1);
               radial_reduction( true );
               progress->setValue(progress->value() + 1);
               sidechain_overlap = save_sidechain_overlap;
               mainchain_overlap = save_mainchain_overlap;
               buried_overlap = save_buried_overlap;

               bead_models[current_model] = bead_model;
               // grid_buried_overlap

               if (asa.recheck_beads)
               {
                  editor->append("Rechecking beads\n");
                  qApp->processEvents();
                  double save_threshold = asa.threshold;
                  double save_threshold_percent = asa.threshold_percent;
                  asa.threshold = asa.grid_threshold;
                  asa.threshold_percent = asa.grid_threshold_percent;
                  progress->setValue(progress->value() + 1);
                  bead_check(false, false);
                  progress->setValue(progress->value() + 1);
                  asa.threshold = save_threshold;
                  asa.threshold_percent = save_threshold_percent;
                  bead_models[current_model] = bead_model;
               }
            }
            else
            {
               if (grid_overlap.remove_overlap)
               {
                  progress->setValue(progress->value() + 1);
                  radial_reduction( true );
                  progress->setValue(progress->value() + 1);
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid_pdb->setEnabled(true);
                     pb_vdw_beads->setEnabled(true);
                     pb_somo->setEnabled(true);
                     pb_somo_o->setEnabled(true);
                     progress->reset();
                     return -1;
                  }
                  bead_models[current_model] = bead_model;
               }
               if (asa.recheck_beads)
               {
                  editor->append("Rechecking beads\n");
                  qApp->processEvents();
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid_pdb->setEnabled(true);
                     pb_vdw_beads->setEnabled(true);
                     pb_somo->setEnabled(true);
                     pb_somo_o->setEnabled(true);
                     progress->reset();
                     return -1;
                  }
                  // Set all beads buried
                  for(unsigned int i = 0; i < bead_model.size(); i++) {
                     bead_model[i].exposed_code = 6;
                     bead_model[i].bead_color = 6;
                  }
                  double save_threshold = asa.threshold;
                  double save_threshold_percent = asa.threshold_percent;
                  asa.threshold = asa.grid_threshold;
                  asa.threshold_percent = asa.grid_threshold_percent;
                  progress->setValue(progress->value() + 1);
                  bead_check(false, false);
                  progress->setValue(progress->value() + 1);
                  asa.threshold = save_threshold;
                  asa.threshold_percent = save_threshold_percent;
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid_pdb->setEnabled(true);
                     pb_vdw_beads->setEnabled(true);
                     pb_somo->setEnabled(true);
                     pb_somo_o->setEnabled(true);
                     progress->reset();
                     return -1;
                  }
                  else
                  {
                     // all exposed
                     for(unsigned int i = 0; i < bead_model.size(); i++) {
                        bead_model[i].exposed_code = 1;
                        bead_model[i].bead_color = 8;
                     }
                  }
                  bead_models[current_model] = bead_model;
               }
            }
            editor->append( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );
            progress->setValue(progress->value() + 1);
            // write_bead_spt(somo_dir + SLASH + project +
            //        (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
            //        QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
            //           DOTSOMO, &bead_model, bead_model_from_file);

            QString filename = 
               project + 
               QString( "_%1" ).arg( current_model + 1 );

            le_bead_model_file->setText( filename );

            write_bead_model(get_somo_dir() + SLASH +
                             filename +
                             QString( bead_model_suffix.length() ? ( "-" + bead_model_suffix ) : "" ) +
                             DOTSOMO, &bead_model);

            progress->setValue(progress->maximum());
         }
      }
   }

   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_grid_pdb->setEnabled(true);
      pb_vdw_beads->setEnabled(true);
      pb_somo->setEnabled(true);
      pb_somo_o->setEnabled(true);
      progress->reset();
      return -1;
   }

   if (any_models && !any_errors)
   {
      editor->append("Build bead model completed\n");
      qApp->processEvents();
      pb_visualize->setEnabled(true);
      pb_equi_grid_bead_model->setEnabled(true);
      pb_calc_hydro->setEnabled(true);
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_grid_pdb->setEnabled(grid_pdb_state);
   pb_vdw_beads->setEnabled(grid_pdb_state);
   pb_grid->setEnabled(true);
   pb_somo->setEnabled(somo_state);
   pb_somo_o->setEnabled(somo_state);
   pb_stop_calc->setEnabled(false);
   if (calcAutoHydro) {
      switch ( misc.auto_calc_hydro_method ) {
      case AUTO_CALC_HYDRO_SMI :
         calc_hydro();
         break;
      case AUTO_CALC_HYDRO_ZENO :
         calc_zeno_hydro();
         break;
      case AUTO_CALC_HYDRO_GRPY :
         calc_grpy_hydro();
         break;
      default :
         editor_msg( "red", us_tr( "No known hydrodynamic method set for automatic hydrodynamic calculations\n"
                                   "Check SOMO->Miscellaneous Options->Automatically calculate hydrodynamics method" ) );
         break;
      }
   } 
   else
   {
      play_sounds(1);
   }

   return (flag);
}

int US_Hydrodyn::calc_vdw_beads()
{
   // info_mw( "calc_vdw_beads() start : model_vector", model_vector, true );

   citation_build_bead_model( "vdw" );
   {
      int models_selected = 0;
      for(int i = 0; i < lb_model->count(); i++) {
         if (lb_model->item(i)->isSelected()) {
            models_selected++;
         }
      }
      if ( !models_selected ) {
         editor_msg( "black", "No models selected!\n" );
         return -1;
      }
   }

   if ( selected_models_contain( "XHY" ) )
   {
      QMessageBox::warning( this,
                            us_tr( "Selected model contains XHY residue" ),
                            us_tr( 
                                  "Can not process models that contain the XHY residue.\n"
                                  "These are currently generated only for SAXS/SANS computations\n"
                                   )
                            );
      return -1;
   }

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   // wipes out p_atom, no longer used
   // model_vector = model_vector_as_loaded;
   options_log = "";
   append_options_log_misc();
   bool any_errors = false;
   //   bool any_models = false;
   pb_grid->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled( false);
   pb_calc_hullrad->setEnabled( false );
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }

   // {
   //    double vdw_ot_mult = gparams.count( "vdw_ot_mult" ) ? gparams[ "vdw_ot_mult" ].toDouble() : 0;
   //    double vdw_ot_dpct = gparams.count( "vdw_ot_dpct" ) ? gparams[ "vdw_ot_dpct" ].toDouble() : 0;
   //    if ( vdw_ot_mult ) {
   //       if ( vdw_ot_dpct ) {
   //          bead_model_suffix = QString( "OT%1DP%2-vdw").arg( vdw_ot_mult ).arg( vdw_ot_dpct );
   //       } else {
   //          bead_model_suffix = QString( "OT%1-vdw").arg( vdw_ot_mult );
   //       }
   //    } else {
   //       bead_model_suffix = "vdw";
   //    }
   // }
   bead_model_suffix = getExtendedSuffix( false, false, true, true );
   le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );

   if ( !overwrite )
   {
      setSomoGridFile(false);
   }

   somo_processed.resize(lb_model->count());
   bead_models.resize(lb_model->count());


   for ( current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++ ) {
      if ( lb_model->item( current_model )->isSelected() ) {
         // us_qdebug( QString( "in calc_vdw_beads: is selected current model %1" ).arg( current_model ) );
         {
            QString error_string;
            // create bead model from atoms
            editor_msg( "black", QString( "Building vdW beads for model %1\n" ).arg( current_model + 1 ) );
            qApp->processEvents();
            // somo_processed[current_model] = 0;
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               return -1;
            }
            // compute maximum position for progress
            somo_processed[ current_model ] = 0;
            bead_models[ current_model ].clear( );
            progress->reset();
            progress->setMaximum( model_vector[current_model].molecule.size() );
            progress->setValue( 0 );

            int retval = create_vdw_beads( error_string );

            if ( retval ) {
               editor_msg( "red", "Errors found during the initial creation of beads\n" );
               qApp->processEvents();
               any_errors = true;
            } else {
               editor_msg( "black", QString( "vdW model %1 contains %2 beads\n" )
                           .arg( current_model + 1 )
                           .arg( bead_models[ current_model ].size() )
                           );
            }
         }
      }
   }
   if ( !any_errors )
   {
      editor_msg( "blue", "Build vdW bead model completed\n" );
      qApp->processEvents();
      pb_visualize->setEnabled(true);
      pb_equi_grid_bead_model->setEnabled(true);
      pb_calc_hydro->setEnabled( false );
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   } else {
      editor_msg( "red", "Errors encountered\n" );
   }


   pb_grid_pdb->setEnabled(true);
   pb_vdw_beads->setEnabled(true);
   pb_grid->setEnabled(true);
   pb_somo->setEnabled(true);
   pb_somo_o->setEnabled(true);
   pb_stop_calc->setEnabled(false);
   if (calcAutoHydro) {
      switch ( misc.auto_calc_hydro_method ) {
      case AUTO_CALC_HYDRO_SMI :
      case AUTO_CALC_HYDRO_ZENO :
         calc_zeno_hydro();
         break;
      case AUTO_CALC_HYDRO_GRPY :
         calc_grpy_hydro();
         break;
      default :
         editor_msg( "red", us_tr( "No known hydrodynamic method set for automatic hydrodynamic calculations\n"
                                   "Check SOMO->Miscellaneous Options->Automatically calculate hydrodynamics method" ) );
         break;
      }
   } else {
      play_sounds(1);
   }

   progress->reset();
   
   return ( any_errors ? -1 : 0 );
}

// 2 pass, 1st to compute com
// 2nd upon hydration, find vector from bead to com and OT by multiplier
// add global multiplier for OT 

int US_Hydrodyn::create_vdw_beads( QString & error_string, bool quiet ) {
   // qDebug() << "create_vdw_beads() start current model = " << current_model;
   // info_mw( "create_vdw_beads() start : model_vector[current_model]", model_vector[current_model], true );
   error_string = "";
   double vdw_ot_mult = gparams.count( "vdw_ot_mult" ) ? gparams[ "vdw_ot_mult" ].toDouble() : 0;
   // double vdw_ot_dpct = gparams.count( "vdw_ot_dpct" ) ? gparams[ "vdw_ot_dpct" ].toDouble() : 0;
   // double vdw_ot_d = vdw_ot_dpct * 0.01;
   // us_qdebug( QString( "vdw ot mult %1, additional water decrease percent %2" ).arg( vdw_ot_mult ).arg( vdw_ot_dpct ) );
   bool vdw_ot_alt = gparams.count( "vdw_ot_alt" ) && gparams[ "vdw_ot_alt" ] == "true";

   if ( vdw_ot_alt ) {
      editor_msg( "darkred", us_tr( "OT alternate method selected but not yet implemented\n" ) );
      return -1;
   }
      
   if ( model_vector_has_hydration_differences( model_vector ) ) {
      editor_msg( "darkred", us_tr( "WARNING: PDB contains residues with bead hydration without atomic hydration,\nvdW models should not be used a they rely on atomic hydration\n\n" ) );
   }

   point com;
   com.axis[ 0 ] = 0;
   com.axis[ 1 ] = 0;
   com.axis[ 2 ] = 0;
   double total_mw = 0e0;
   double hydro_radius = 0e0;

   bool any_wats = false;
   for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++) {
      for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++) {
         PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
         if ( this_atom->resName == "WAT" ) {
            any_wats = true;
            break;
         }
         if ( any_wats ) {
            break;
         }
      }
   }
   if ( any_wats ) {
      vdw_ot_mult = 0;
      editor_msg( "black", us_tr( "Explicit WATs found, hydration disabled\n" ) );
   }
   bool hydrate = !any_wats;

// #warning hydration off
//    hydrate = false;
//    vdw_ot_mult = 0;
//    editor_msg( "red", "hydration off for testing\n" );

   if ( !quiet ) 
   {
      editor->append("Creating vdW beads from atomic model\n");
      qApp->processEvents();
   }

   bead_model.clear( );

   // compute ASA for hydration
   {
      vector < PDB_atom * > active_atoms;
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++) {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++) {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
            active_atoms.push_back( this_atom );
         }
      }
      progress->reset();
      qApp->processEvents();
      int retval;
      {
         float save_radius = asa.probe_radius;
         asa.probe_radius  = asa.hydrate_probe_radius; 

         retval = us_hydrodyn_asab1_main(active_atoms,
                                         &asa,
                                         &results,
                                         false,
                                         progress,
                                         editor,
                                         this
                                         );
         asa.probe_radius  = save_radius;
      }         

      if ( retval )
      {
         editor->append("Errors found during ASA calculation\n");
         progress->reset();
         qApp->processEvents();
         if (stopFlag)
         {
            return -1;
         }
         switch ( retval )
         {
         case US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC:
            {
               printError("US_HYDRODYN_ASAB1 encountered a memory allocation error");
               return US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC;
               break;
            }
         default:
            {
               printError("US_HYDRODYN_ASAB1 encountered an unknown error");
               // unknown error
               return -1;
               break;
            }
         }
      }
   }

   if ( vdw_ot_mult ) {
      hydro_radius = pow( ( 3e0 / ( 4e0 * M_PI ) ) * misc.hydrovol, 1e0 / 3e0 );
      if ( vdw_ot_alt ) {
         editor_msg( "black", us_tr( "OT alternate method active\n" ) );
      }
      editor_msg( "black"
                  ,QString(
                           us_tr(
                                 "Using OT multiplier %1 hydrodynamic radius %2 multiplied %3\n"
                                 "Start global CoM calculation.\n"
                                 )
                           )
                  .arg( vdw_ot_mult )
                  .arg( hydro_radius )
                  .arg( hydro_radius * vdw_ot_mult )
                  );
      qApp->processEvents();
      
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++) {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++) {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
            QString res_idx =
               QString("%1|%2")
               .arg(this_atom->name != "OXT" ? this_atom->resName : "OXT" )
               .arg(this_atom->name);
            if ( vdwf.count( res_idx ) ) {
               _vdwf this_vdwf = vdwf[ res_idx ];
               // ? does mw include hydrated weight ?
               double this_mw_w_delta = this_vdwf.mw + this_vdwf.ionized_mw_delta;
               total_mw += this_mw_w_delta;
               com.axis[ 0 ] += this_atom->coordinate.axis[ 0 ] * this_mw_w_delta;
               com.axis[ 1 ] += this_atom->coordinate.axis[ 1 ] * this_mw_w_delta;
               com.axis[ 2 ] += this_atom->coordinate.axis[ 2 ] * this_mw_w_delta;
            } else {
               us_qdebug( QString( "not found %1 in vdwf" ).arg( res_idx ) );
            }
         }
      }
      if ( total_mw ) {
         com.axis[ 0 ] /= total_mw;
         com.axis[ 1 ] /= total_mw;
         com.axis[ 2 ] /= total_mw;
         editor_msg( "black", QString( us_tr( "CoM computed as [%1,%2,%3]\n" ).arg( com.axis[ 0 ] ).arg( com.axis[ 1 ] ).arg( com.axis[ 2 ] ) ) );
      } else {
         editor_msg( "red", QString( us_tr( "Error computing CoM, OT turned off.\n" ) ) );
         vdw_ot_mult = 0;
      }         
   }

   int WAT_Tf_used = 0;
   bool use_WAT_Tf =
      gparams.count( "use_WAT_Tf_pdb" ) &&
      gparams[ "use_WAT_Tf_pdb" ] == "true";

   bool use_vdw_inflate = 
      gparams.count( "vdw_inflate" ) &&
      gparams.count( "vdw_inflate_multiplier" ) &&
      gparams[ "vdw_inflate_mult" ] != "1" &&
      gparams[ "vdw_inflate" ] == "true";

   double vdw_inflate_mult = 1;
   if ( use_vdw_inflate ) {
      vdw_inflate_mult = gparams[ "vdw_inflate_multiplier" ].toDouble();
      editor_msg( "dark red", us_tr( QString( "WARNING: beads are inflated by a factor of %1" ).arg( vdw_inflate_mult ) ) );
   }

   // redo summary info
   class summary_info {
   public: 
      int     count;
      double  theo_waters;
      int     count_exposed;
      double  theo_waters_exposed;
      summary_info() {
         count               = 0;
         count_exposed       = 0;
         theo_waters         = 0;
         theo_waters_exposed = 0;
      };
   };

   map < QString, summary_info > summary_infos;
   summary_info next_summary_info;
   bool         summary_info_exposed_counted = false;

   for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++) {
      QString last_resSeq;

      for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++) {
         PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

         if ( !this_atom->p_residue ) {
            editor_msg( "red", QString( us_tr( "Internal error: p_residue not set!, contact the developers" ) ) );
            return -1;
         }
         if ( !this_atom->p_atom) {
            editor_msg( "red", QString( us_tr( "Internal error: p_atom not set!, contact the developers" ) ) );
            return -1;
         }

         if ( last_resSeq != this_atom->resSeq ) {
            ++summary_infos[ this_atom->resName ].count;
            last_resSeq = this_atom->resSeq;
            summary_info_exposed_counted = false;
         }

         // QTextStream( stdout )
         //    << "vdwf: atom name " << this_atom->name
         //    << " p_residue->name " << this_atom->p_residue->name
         //    << " p_atom->hybrid.name " << this_atom->p_atom->hybrid.name
         //    << endl;

         QString use_res_name  = this_atom->name != "OXT" ? this_atom->p_residue->name : "OXT";
         QString use_atom_name = this_atom->p_residue->name == "N1" ? "N1" : this_atom->name;

         if ( !k &&
              this_atom->name == "N" ) {
            if ( use_res_name == "PRO" ) {
               use_res_name = "N1-";
            } else {
               use_res_name = "N1";
            }
            use_atom_name = use_res_name;
         }
              
         QString res_idx =
            QString("%1|%2")
            .arg( use_res_name )
            .arg( use_atom_name )
            ;

         double saxs_excl_vol;
         {
            QString mapkey = QString("%1|%2").arg(this_atom->resName).arg(this_atom->name);
            if ( this_atom->name == "OXT" ) {
               mapkey = "OXT|OXT";
            }
            if ( !residue_atom_hybrid_map.count( mapkey ) ) {
               editor_msg( "red", QString( us_tr( "Error: Missing hybrid name for key %1" ) ).arg( mapkey ) );
               return -1;
            }
            QString hybrid_name = residue_atom_hybrid_map[mapkey];
            QString this_atom_name = hybrid_name == "ABB" ? "ABB" : this_atom->name;
            if ( !atom_map.count( this_atom_name + "~" + hybrid_name ) ) {
               editor_msg( "red", QString( us_tr( "Error: Missing hybrid name for key %1" ) ).arg( mapkey ) );
               return -1;
            }
            saxs_excl_vol = atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol;
         }

         // QTextStream( stdout ) << "vdwf: res_idx is " << res_idx << " " << endl;
         
         if ( vdwf.count( res_idx ) ) {
            // QTextStream( stdout ) << "found ionized_mw_delta " << vdwf[ res_idx ].ionized_mw_delta << endl;
            PDB_atom tmp_atom;
            _vdwf this_vdwf            = vdwf[ res_idx ];
            // overwrite from p_atom
            this_vdwf.mw               = this_atom->p_atom->hybrid.mw;
            this_vdwf.ionized_mw_delta = this_atom->p_atom->hybrid.ionized_mw_delta;
            this_vdwf.r                = this_atom->p_atom->hybrid.radius;
            this_vdwf.w                = this_atom->p_atom->hydration;
            this_vdwf.e                = this_atom->p_atom->hybrid.num_elect;
            summary_infos[ this_atom->resName ].theo_waters += this_vdwf.w;
            if ( this_atom->asa >= asa.hydrate_threshold && this_vdwf.w ) {
               summary_infos[ this_atom->resName ].theo_waters_exposed += this_vdwf.w;
               if ( !summary_info_exposed_counted ) {
                  ++summary_infos[ this_atom->resName ].count_exposed;
                  summary_info_exposed_counted = true;
               }
            }

            if ( this_atom->resName == "WAT" ) {
               this_vdwf.mw = 0.0;
            }
            tmp_atom.bead_coordinate = this_atom->coordinate;
            if ( hydrate && this_vdwf.w && this_atom->asa >= asa.hydrate_threshold && this_vdwf.w ) {
               double tmp_vol = M_PI * ( 4e0 / 3e0 ) * this_vdwf.r * this_vdwf.r * this_vdwf.r + ( this_vdwf.w * misc.hydrovol );
               tmp_atom.bead_computed_radius = pow( tmp_vol * 3e0 / ( 4e0 * M_PI ), 1e0 / 3e0 );
               double use_vdw_ot_mult = vdw_ot_mult;
               // if ( this_vdwf.w > 1 ) {
               //    use_vdw_ot_mult -= vdw_ot_mult * vdw_ot_d * ( this_vdwf.w - 1 );
               //    if ( use_vdw_ot_mult < 0e0 ) {
               //       use_vdw_ot_mult = 0e0;
               //    }
               // }
               // us_qdebug( QString( "original ot mult %1, waters %2, decreased multiplier %3" ).arg( vdw_ot_mult ).arg( this_vdwf.w ).arg( use_vdw_ot_mult ) );
               if ( use_vdw_ot_mult ) {
                  tmp_atom.bead_coordinate = saxs_util->plus( tmp_atom.bead_coordinate, saxs_util->scale( saxs_util->normal( saxs_util->minus( this_atom->coordinate, com ) ), use_vdw_ot_mult * hydro_radius ) );
               }
            } else {
               tmp_atom.bead_computed_radius = this_vdwf.r * vdw_inflate_mult;
               if (
                   use_WAT_Tf &&
                   this_atom->resName == "WAT" &&
                   this_atom->tempFactor ) {
                  // QTextStream( stderr ) <<
                  //    QString( "create_vdw_beads WAT Tf %1\n" ).arg( this_atom->tempFactor )
                  //    ;
                  tmp_atom.bead_computed_radius = this_atom->tempFactor;
                  WAT_Tf_used++;
               }
            }
            // us_qdebug( QString( "bead model radius for %1 = %2" ).arg( res_idx ).arg(  tmp_atom.bead_computed_radius ) );
            // us_qdebug( QString( "bead model mw for %1 = %2" ).arg( res_idx ).arg( this_vdwf.mw ) );

            tmp_atom.radius                    = tmp_atom.bead_computed_radius;
            tmp_atom.bead_ref_mw               = this_vdwf.mw;
            tmp_atom.bead_ref_ionized_mw_delta = this_vdwf.ionized_mw_delta;
            tmp_atom.mw                        = this_vdwf.mw;
            tmp_atom.ionized_mw_delta          = this_vdwf.ionized_mw_delta;
            tmp_atom.bead_mw                   = this_vdwf.mw;
            tmp_atom.bead_ionized_mw_delta     = this_vdwf.ionized_mw_delta;
            tmp_atom.bead_color                = this_vdwf.color;
            // #warning recolored for hydration
            // tmp_atom.bead_color = hydrate && this_vdwf.w ? 1 : 6;
            // #warning recolored for ASA testing
            // tmp_atom.bead_color                = this_atom->asa >= asa.hydrate_threshold ? 6 : 8;
            tmp_atom.bead_recheck_asa          = this_atom->asa;
            tmp_atom.num_elect                 = this_vdwf.e;
            tmp_atom.exposed_code              = 1;
            tmp_atom.all_beads                 .clear( );
            tmp_atom.active                    = true;
            tmp_atom.name                      = this_atom->name;
            tmp_atom.resName                   = this_atom->resName;
            tmp_atom.iCode                     = this_atom->iCode;
            tmp_atom.chainID                   = this_atom->chainID;
            tmp_atom.saxs_excl_vol             = saxs_excl_vol;
            if ( this_atom->asa >= asa.hydrate_threshold && this_vdwf.w ) {
               tmp_atom.bead_hydration            = this_vdwf.w;
            } else {
               tmp_atom.bead_hydration            = 0;
            }
            tmp_atom.saxs_data.saxs_name       = "";
            tmp_atom.bead_model_code = QString( "%1.%2.%3.%4.%5" )
               .arg( this_atom->serial )
               .arg( this_atom->name )
               .arg( this_atom->resName )
               .arg( this_atom->chainID )
               .arg( this_atom->resSeq )
               ;

            // QTextStream(stdout) << QString( "atom name %1 resname %2 asa %3 bead_asa %4 ref_asa %5 bead_recheck_asa %6\n" )
            //    .arg( tmp_atom.name )
            //    .arg( tmp_atom.resName )
            //    .arg( this_atom->asa )
            //    .arg( this_atom->bead_asa )
            //    .arg( this_atom->ref_asa )
            //    .arg( this_atom->bead_recheck_asa )
            //    ;

#warning commented out the next three lines, likely side effects ... were they added for some purpose?
            // this_atom->mw               = tmp_atom.mw;
            // this_atom->ionized_mw_delta = tmp_atom.ionized_mw_delta;
            // this_atom->radius           = tmp_atom.radius;

            bead_model.push_back(tmp_atom);
         } else {
            editor_msg( "red", QString( "Residue atom pair %1 unknown in vdwf.json" ).arg( res_idx ) );
            return -1;
         }
      }
   }      

   {
      QFont courier = QFont( "Courier", USglobal->config_list.fontSize - 1 );
      editor_msg( "black", courier,
                  "Atomic level pH dependent hydration:\n"
                  "Residue  Count   Percent  Theo. wat  Exp. sites  Exp. theo. wat\n"
                  );

      summary_info summary_totals;
      for ( auto it = summary_infos.begin();
            it != summary_infos.end();
            ++it ) {
         summary_totals.count               += it->second.count;
         summary_totals.theo_waters         += it->second.theo_waters;
         summary_totals.count_exposed       += it->second.count_exposed;
         summary_totals.theo_waters_exposed += it->second.theo_waters_exposed;
      }

      for ( auto it = summary_infos.begin();
            it != summary_infos.end();
            ++it ) {
         editor_msg( "black", courier, QString( "%1  %2  %3   %4  %5     %6\n" )
                     .arg( it->first, -7 )
                     .arg( it->second.count, -6 )
                     .arg( QString( "%1%" )
                           .arg( floor( 100 * 100.0 * (double) it->second.count / (double) summary_totals.count ) / 100, 0, 'g', 3 ), -6 )
                     .arg( it->second.theo_waters, -9, 'f', 0 )
                     .arg( it->second.count_exposed, -7 )
                     .arg( it->second.theo_waters_exposed, 0, 'f', 0 )
                     );
      }
      
      editor_msg( "black", courier, QString( "%1  %2  %3   %4  %5     %6\n" )
                  .arg( "Total", -7 )
                  .arg( summary_totals.count, -6 )
                  .arg( QString( "%1%" )
                        .arg( floor( 100 * 100.0 * (double) summary_totals.count / (double) summary_totals.count ) / 100, 0, 'g', 3 ), -6 )
                  .arg( summary_totals.theo_waters, -9, 'f', 0 )
                  .arg( summary_totals.count_exposed, -7 )
                  .arg( summary_totals.theo_waters_exposed, 0, 'f', 0 )
                  );

      if ( bead_model.size() ) {
         bead_model[0].is_vdw                   = "vdw";
         bead_model[0].vdw_theo_waters          = QString( "%1" ).arg( summary_totals.theo_waters, 0, 'f', 0 ).toDouble();
         bead_model[0].vdw_count_exposed        = summary_totals.count_exposed;
         bead_model[0].vdw_theo_waters_exposed  = QString( "%1" ).arg( summary_totals.theo_waters_exposed, 0, 'f', 0 ).toDouble();
         bead_model[0].asa_hydrate_probe_radius = asa.hydrate_probe_radius;
         bead_model[0].asa_hydrate_threshold    = asa.hydrate_threshold;
      }
   }

   if ( WAT_Tf_used ) {
      editor_msg( "dark blue", QString( us_tr( "Notice: %1 WATs using PDB's Tf radius recognized\n" ) ).arg( WAT_Tf_used ) );
   }

   // if we wanted to compute asa at bead model generation
   // if ( !hydro.bead_inclusion ) {
   //    // bury_all
   //    qDebug() << "name is $fname";
   // for ( int i = 0; i < (int) bead_model.size(); ++i ) {
   //    bead_model[ i ].radius = bead_model[ i ].bead_computed_radius;
   // }
   // results.asa_rg_pos = 0.0;
   // results.asa_rg_neg = 0.0;
   // bead_check( false, true, true, false );
   {
      vector < PDB_atom > active_atoms;
      for (unsigned int j = 0; j < model_vector[ current_model ].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[ current_model ].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[ current_model ].molecule[j].atom[k]);
            if(this_atom->active) {
               active_atoms.push_back( *this_atom );
            }
         }
      }
      compute_asa_rgs( active_atoms );
      model_vector[ current_model ].asa_rg_pos = results.asa_rg_pos;
      model_vector[ current_model ].asa_rg_neg = results.asa_rg_neg;
   }
   
   // qDebug() << "US_Hydrodyn::create_vdw_beads() asa rg pos " << results.asa_rg_pos << " neg " << results.asa_rg_neg;


   bead_models[ current_model ] = bead_model;
   somo_processed[ current_model ] = 1;
   
   write_bead_model( get_somo_dir() + SLASH + project + fix_file_name( QString("_%1").arg( model_name( current_model ) ) ) +
                     QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
                     , &bead_model);
   
   editor->append( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );
   return 0;
}

bool US_Hydrodyn::compute_asa_rgs( const vector < PDB_atom > & model ) {
   results.asa_rg_neg = 0e0;
   results.asa_rg_pos = 0e0;

   int n = (int) model.size();
   if ( !n ) {
      editor_msg( "red", "No atoms found in model!" );
      return false;
   }

   double xm = 0e0;
   double ym = 0e0;
   double zm = 0e0;
   double mt = 0e0;

   for ( int i = 0; i < n; ++i ) {
      double m = model[ i ].mw + model[ i ].ionized_mw_delta;
      double x = model[ i ].coordinate.axis[ 0 ];
      double y = model[ i ].coordinate.axis[ 1 ];
      double z = model[ i ].coordinate.axis[ 2 ];

      mt += m;
      xm += x * m;
      ym += y * m;
      zm += z * m;
   }

   if ( !mt ) {
      editor_msg( "red", "Atoms in model have no molecular weight!" );
      return false;
   }

   xm /= mt;
   ym /= mt;
   zm /= mt;

   double rg_pos = 0e0;
   double rg_neg = 0e0;

   for ( int i = 0; i < n; ++i ) {
      double m  = model[ i ].mw + model[ i ].ionized_mw_delta;
      double dx = model[ i ].coordinate.axis[ 0 ] - xm;
      double dy = model[ i ].coordinate.axis[ 1 ] - ym;
      double dz = model[ i ].coordinate.axis[ 2 ] - zm;
      double dr = model[ i ].radius;

      double rg = 0.6 * dr * dr;
      double this_contrib = dx * dx + dy * dy + dz * dz;
      rg_neg += m * this_contrib;
      rg_pos += m * ( this_contrib + rg );
   }

   rg_neg /= mt;
   rg_pos /= mt;

   results.asa_rg_neg = sqrt( rg_neg );
   results.asa_rg_pos = sqrt( rg_pos );
   return true;
}

bool US_Hydrodyn::load_vdwf_json( QString filename ) {
   QFile f( filename );
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      return false;
   }
   QString qs;
   QTextStream ts( &f );
   while ( !ts.atEnd() )
   {
      qs += ts.readLine();
   }
   f.close();
   vdwf.clear( );
   map < QString, QString > parameters = US_Json::split( qs );
   _vdwf this_vdwf;
   for ( map < QString, QString >::iterator it = parameters.begin();
         it != parameters.end();
         it++ )
   {
      // us_qdebug( QString( "vdwf read %1 %2" ).arg( it->first ).arg( it->second ) );
      map < QString, QString > tmp_param = US_Json::split( "{" + it->second + "}" );
      if ( !tmp_param.count( "mw" ) ||
           !tmp_param.count( "r" ) ||
           !tmp_param.count( "w" ) ) {
         us_qdebug( QString( "vdwf missing mw r or w %1 %2" ).arg( it->first ).arg( it->second ) );
      } else {
         this_vdwf.mw = tmp_param[ "mw" ].toDouble();
         this_vdwf.r  = tmp_param[ "r" ].toDouble();
         this_vdwf.w  = tmp_param[ "w" ].toDouble();
         vdwf[ it->first ] = this_vdwf;
      }
   }
   return false;
}

   
void US_Hydrodyn::hullrad_finished( int, QProcess::ExitStatus )
{
   // us_qdebug( QString( "hullrad_processExited %1" ).arg( hullrad_filename ) );
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   hullrad_readFromStderr();
   hullrad_readFromStdout();
      //   }
   disconnect( hullrad, SIGNAL(readyReadStandardOutput()), 0, 0);
   disconnect( hullrad, SIGNAL(readyReadStandardError()), 0, 0);
   disconnect( hullrad, SIGNAL(finished( int, QProcess::ExitStatus )), 0, 0);
   editor->append("HULLRAD finished.\n");

   // post process the files

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

      if ( rx.indexIn( hullrad_stdout ) == -1 ) {
         editor_msg( "red", QString( us_tr( "Could not find %1 file in HULLRAD output" ) ).arg( caps[ i ].replace( "\\", "" ) ) );
         hullrad_captures[ caps[ i ] ].push_back( -9e99 );
      } else {
         hullrad_captures[ caps[ i ] ].push_back( rx.cap( 1 ).toDouble() );
         us_qdebug( QString( "%1 : '%2'\n" ).arg( caps[ i ] ).arg( hullrad_captures[ caps[ i ] ].back() ) );
      }
   }

   // accumulate data as in zeno (e.g. push values to data structures
   
   hullrad_process_next();
}
   
void US_Hydrodyn::hullrad_started()
{
   // us_qdebug( QString( "hullrad_started %1" ).arg( hullrad_filename ) );
   editor_msg("brown", "HULLRAD launch exited\n");
   disconnect( hullrad, SIGNAL(started()), 0, 0);
}

void US_Hydrodyn::hullrad_finalize() {
   // us_qdebug( QString( "hullrad_finalize %1" ).arg( hullrad_filename ) );
   editor_msg( "dark red", "Finalizing results" );
   for ( map < QString, vector < double > >::iterator it = hullrad_captures.begin();
         it != hullrad_captures.end();
         ++it ) {
      editor_msg( "dark red",  US_Vector::qs_vector( it->first, it->second ) );
   }

   hydro_results hullrad_results;
   hydro_results hullrad_results2;

   hullrad_results.method                = "Hullrad";
   hullrad_results.mass                  = 0e0;
   hullrad_results.s20w                  = 0e0;
   hullrad_results.s20w_sd               = 0e0;
   hullrad_results.D20w                  = 0e0;
   hullrad_results.D20w_sd               = 0e0;
   hullrad_results.viscosity             = 0e0;
   hullrad_results.viscosity_sd          = 0e0;
   hullrad_results.rs                    = 0e0;
   hullrad_results.rs_sd                 = 0e0;
   hullrad_results.rg                    = 0e0;
   hullrad_results.rg_sd                 = 0e0;
   hullrad_results.tau                   = 0e0;
   hullrad_results.tau_sd                = 0e0;
   hullrad_results.vbar                  = 0e0;
   hullrad_results.asa_rg_pos            = 0e0;
   hullrad_results.asa_rg_neg            = 0e0;
   hullrad_results.ff0                   = 0e0;
   hullrad_results.ff0_sd                = 0e0;

   hullrad_results.solvent_name          = hydro.solvent_name;
   hullrad_results.solvent_acronym       = hydro.solvent_acronym;
   hullrad_results.solvent_viscosity     = hydro.solvent_viscosity;
   hullrad_results.solvent_density       = hydro.solvent_density;
   hullrad_results.temperature           = hydro.temperature;
   hullrad_results.name                  = project;
   hullrad_results.used_beads            = 0;
   hullrad_results.used_beads_sd         = 0e0;
   hullrad_results.total_beads           = 0;
   hullrad_results.total_beads_sd        = 0e0;
   hullrad_results.vbar                  = 0;

   hullrad_results.num_models            = hullrad_processed.size();

   hullrad_results2 = hullrad_results;

   map < int, map < QString, double > > data_to_save;
   
   for ( map < QString, vector < double > >::iterator it = hullrad_captures.begin();
         it != hullrad_captures.end();
         ++it ) {

      for ( int i = 0; i < (int) it->second.size(); ++i ) {
         data_to_save[ i ][ it->first ] = it->second[ i ];

         if ( it->first  == "M" ) {
            {
               hullrad_results.mass += it->second[ i ];
               // hullrad_results2.mass += it->second[ i ] * it->second[ i ];
            }
            break;
         }               
         if ( it->first == "v_bar" ) {
            hullrad_results.vbar += it->second[ i ];
            // hullrad_results2.vbar += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "R\\(Anhydrous\\)" ) {
            hullrad_results.rs += it->second[ i ];
            hullrad_results2.rs += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "Axial Ratio" ) {
            //    hullrad_results.mass += it->second[ i ];
            //    hullrad_results2.mass += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "f/fo" ) {
            hullrad_results.ff0 += it->second[ i ];
            hullrad_results2.ff0 += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "Dt" ) {
            hullrad_results.D20w += it->second[ i ];
            hullrad_results2.D20w += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "R\\(Translation\\)" ) {
            //    hullrad_results.mass += it->second[ i ];
            //    hullrad_results2.mass += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "s" ) {
            hullrad_results.s20w += it->second[ i ] * 1e13;
            hullrad_results2.s20w += it->second[ i ] * it->second[ i ] * 1e26;
         }

         if ( it->first == "\\[eta\\]" ) {
            hullrad_results.viscosity += it->second[ i ];
            hullrad_results2.viscosity += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "Dr" ) {
            //    hullrad_results.mass += it->second[ i ];
            //    hullrad_results2.mass += it->second[ i ] * it->second[ i ];
         }
         if ( it->first == "R\\(Rotation\\)" ) {
            //    hullrad_results.mass += it->second[ i ];
            //    hullrad_results2.mass += it->second[ i ] * it->second[ i ];
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
      this_data.results.method                = "Hullrad";
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
      this_data.results.name                  = QString( "%1-%2" ).arg( QFileInfo( hullrad_filename ).completeBaseName() ).arg( it->first + 1 );
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
         QString fname = somo_dir + "/" + this_data.results.name + ".hullrad.csv";
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
      double num = (double) hullrad_results.num_models;
      if ( num <= 1 ) {
         results = hullrad_results;
      } else {
         hullrad_results.name = QFileInfo( hullrad_filename ).baseName();
         double numinv = 1e0 / num;
         hullrad_results.mass          *= numinv;
         hullrad_results.s20w          *= numinv;
         hullrad_results.D20w          *= numinv;
         hullrad_results.viscosity     *= numinv;
         hullrad_results.rs            *= numinv;
         hullrad_results.rg            *= numinv;
         hullrad_results.vbar          *= numinv;
         hullrad_results.ff0           *= numinv;
         hullrad_results.used_beads    *= numinv;
         hullrad_results.total_beads   *= numinv;
         if ( num <= 1 ) {
            results = hullrad_results;
         } else {
            double numdecinv = 1e0 / ( num - 1e0 );
         
            hullrad_results.s20w_sd           = sqrt( fabs( ( hullrad_results2.s20w        - hullrad_results.s20w        * hullrad_results.s20w        * num ) * numdecinv ) );
            hullrad_results.D20w_sd           = sqrt( fabs( ( hullrad_results2.D20w        - hullrad_results.D20w        * hullrad_results.D20w        * num ) * numdecinv ) );
            hullrad_results.viscosity_sd      = sqrt( fabs( ( hullrad_results2.viscosity   - hullrad_results.viscosity   * hullrad_results.viscosity   * num ) * numdecinv ) );
            hullrad_results.rs_sd             = sqrt( fabs( ( hullrad_results2.rs          - hullrad_results.rs          * hullrad_results.rs          * num ) * numdecinv ) );
            hullrad_results.rg_sd             = sqrt( fabs( ( hullrad_results2.rg          - hullrad_results.rg          * hullrad_results.rg          * num ) * numdecinv ) );
            hullrad_results.ff0_sd            = sqrt( fabs( ( hullrad_results2.ff0         - hullrad_results.ff0         * hullrad_results.ff0         * num ) * numdecinv ) );
            hullrad_results.used_beads_sd     = sqrt( fabs( ( (double) hullrad_results2.used_beads  - (double) hullrad_results.used_beads  * (double) hullrad_results.used_beads  * num ) * numdecinv ) );
            hullrad_results.total_beads_sd    = sqrt( fabs( ( (double) hullrad_results2.total_beads - (double) hullrad_results.total_beads * (double) hullrad_results.total_beads * num ) * numdecinv ) );
            
            results = hullrad_results;
         }
      }
   }

   pb_show_hydro_results->setEnabled( true );
   hullrad_running = false;
   // us_qdebug( QString( "hullrad_finalize %1 end" ).arg( hullrad_filename ) );
}

void US_Hydrodyn::create_fasta_vbar_mw() {

#if defined( DEBUG_FASTA_SEQ )
   qDebug() << "create_fasta_vbar_mw";
   for ( map < QString, double >::iterator it = res_vbar.begin();
         it != res_vbar.end();
         ++it ) {
      qDebug() << "res_vbar[" << it->first << "] = " << it->second;
      qDebug() << "res_mw[" << it->first << "] = " << res_mw[ it->first ];
   }

#endif

   fasta_vbar.clear();
   fasta_mw  .clear();
   
   for ( map < QString, QChar >::iterator it = residue_short_names.begin();
         it != residue_short_names.end();
         ++it ) {

      if ( it->first != "WAT" &&
           res_vbar.count( it->first ) &&
           res_mw.count( it->first ) 
           ) {
         fasta_vbar[ it->second ] = res_vbar[ it->first ];
         fasta_mw  [ it->second ] = res_mw  [ it->first ];
      }
   }

#if defined( DEBUG_FASTA_SEQ )
   qDebug() << "create_fasta_vbar_mw";
   for ( map < QString, double >::iterator it = fasta_vbar.begin();
         it != fasta_vbar.end();
         ++it ) {
      qDebug() << "fasta_vbar[" << it->first << "] = " << it->second;
      qDebug() << "fasta_mw[" << it->first << "] = " << fasta_mw[ it->first ];
   }

#endif
   
}

bool US_Hydrodyn::calc_hullrad_hydro( QString filename ) {
   {
      QFont courier = QFont( "Courier", USglobal->config_list.fontSize );
      editor_msg( "dark blue", courier, visc_dens_msg() );
   }

   hullrad_filename = filename.isEmpty() ? last_pdb_filename : filename;
   
   editor_msg( "black", QString( "Hullrad %1" ).arg( hullrad_filename ) );
   // us_qdebug( QString( "calc_hullrad_hydro %1" ).arg( hullrad_filename ) );

   hullrad_prog = 
      USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + "HullRadV2.py" 
         ;

   QFileInfo qfi( hullrad_prog );
   if ( !qfi.exists() ) {
         editor_msg( (QString) "red", QString("Hullrad program '%1' does not exist\n").arg(hullrad_prog));
         return false;
   }

   // make tmp pdb, run hullrad

   QFile f( hullrad_filename );

   if ( !f.exists() ) {
      editor_msg( "red", QString( "PDB file %1 not found" ).arg( hullrad_filename ) );
      return false;
   }
   
   if ( !f.open( QIODevice::ReadOnly ) ) {
      QMessageBox::warning( this,
                            us_tr("Could not open file"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( f.fileName() )
                            );
      return false;
   }

   // make unique tmp dir
   QString hullrad_tmp_path = somo_dir + "/tmp/" + QDateTime::currentDateTime().toString( "yyMMddhhmmsszzz" );
   while ( QDir( hullrad_tmp_path ).exists() ) {
      US_Saxs_Util::us_usleep( 1000 );
      hullrad_tmp_path = somo_dir + "/tmp/" + QDateTime::currentDateTime().toString( "yyMMddhhmmsszzz" );
   }

   {
      QDir qd;
      if ( !qd.mkdir( hullrad_tmp_path ) ) {
         editor_msg( "red", QString( us_tr("Could not create directory %1, check permissions" ) ).arg( hullrad_tmp_path ) );
         return false;
      }
   }

   QDir::setCurrent( hullrad_tmp_path );

   // open pdb, split, save filenames

   QRegExp rx_model("^MODEL");
   QRegExp rx_end("^END");
   QRegExp rx_save_header("^("
                          "HEADER|"
                          "TITLE|"
                          "COMPND|"
                          "SOURCE|"
                          "KEYWDS|"
                          "AUTHOR|"
                          "REVDAT|"
                          "JRNL|"
                          "REMARK|"
                          "SEQRES|"
                          "SHEET|"
                          "HELIX|"
                          "SSBOND|"
                          "DBREF|"
                          "ORIGX|"
                          "SCALE"
                          ")\\.*" );
   
   unsigned int model_count = 0;

   editor_msg( "dark blue", QString( us_tr( "Checking file %1" ).arg( f.fileName() ) ) );

   map    < QString, bool > model_names;
   vector < QString >       model_name_vector;
   unsigned int             max_model_name_len      = 0;
   QString                  model_header;
   bool                     dup_model_name_msg_done = false;
   unsigned int             end_count               = 0;
   bool                     found_model             = false;
   
   {
      QTextStream ts( &f );
      unsigned int line_count = 0;
   
      while ( !ts.atEnd() ) {
         QString qs = ts.readLine();
         line_count++;
         if ( line_count && !(line_count % 100000 ) )
         {
            editor_msg( "dark blue", QString( us_tr( "Lines read %1" ).arg( line_count ) ) );
            qApp->processEvents();
         }
         if ( !found_model && qs.contains( rx_save_header ) )
         {
            model_header += qs + "\n";
         }
         
         if ( qs.contains( rx_end ) )
         {
            end_count++;
         }

         if ( qs.contains( rx_model ) )
         {
            found_model = true;
            model_count++;
            // QStringList qsl = (qs.left(20).split( QRegExp("\\s+") , Qt::SkipEmptyParts ) );
            QStringList qsl;
            {
               QString qs2 = qs.left( 20 );
               qsl = (qs2 ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
            }
            QString model_name;
            if ( qsl.size() == 1 )
            {
               model_name = QString("%1").arg( model_count );
            } else {
               model_name = qsl[1];
            }
            if ( model_names.count( model_name ) )
            {
               unsigned int mext = 1;
               QString use_model_name;
               do {
                  use_model_name = model_name + QString("-%1").arg( mext );
               } while ( model_names.count( use_model_name ) );
               model_name = use_model_name;
               if ( !dup_model_name_msg_done )
               {
                  dup_model_name_msg_done = true;
                  editor_msg( "red", us_tr( "Duplicate or missing model names found, -# extensions added" ) );
               }
            }
            model_names[ model_name ] = true;
            model_name_vector.push_back ( model_name );
            if ( (unsigned int) model_name.length() > max_model_name_len )
            {
               max_model_name_len = model_name.length();
            }
         }
      }
   }

   f.close();

   bool no_model_directives = false;

   if ( model_count == 0 )
   {
      if ( end_count > 1 )
      {
         no_model_directives = true;
         model_count = end_count;
         for ( unsigned int i = 0; i < end_count; i++ )
         {
            QString model_name = QString("%1").arg( i + 1 );
            model_names[ model_name ] = true;
            model_name_vector.push_back ( model_name );
            if ( (unsigned int) model_name.length() > max_model_name_len )
            {
               max_model_name_len = model_name.length();
            }
         }
      } else {
         model_count = 1;
      }
   }

   hullrad_to_process.clear( );
   hullrad_processed.clear( );
   hullrad_captures.clear( );

   if ( model_count == 1 ) {
      hullrad_to_process << hullrad_filename;
      hullrad_running = true;
      hullrad_process_next();
      return true;
   }

   // push stack of pdbs 

   int res = 1; // every model
   
   QString ext = "X";
   while ( (unsigned int) ext.length() < max_model_name_len )
   {
      ext = "X" + ext;
   }
   ext = "-" + ext + ".pdb";

   QString fn = hullrad_tmp_path + "/" + QFileInfo( f ).fileName().replace(QRegExp("\\.(pdb|PDB)$"),"") + ext;

   fn.replace(QRegExp(QString("%1$").arg(ext)), "" );

   if ( !f.open( QIODevice::ReadOnly ) ) {
      QMessageBox::warning( this,
                            us_tr("Could not open file"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( f.fileName() )
                            );
      return false;
   }

   QTextStream ts( &f );

   QString       model_lines;
   bool          in_model = no_model_directives;
   unsigned int  pos = 0;

   if ( !ts.atEnd() ) {
      do {
         QString qs = ts.readLine();
         if ( qs.contains( rx_model ) || qs.contains( rx_end ) || ts.atEnd() ) {
            if ( model_lines.length() ) {
               if ( !( pos % res ) ) {
                  QString use_ext = model_name_vector[ pos ];
                  while ( (unsigned int) use_ext.length() < max_model_name_len ) {
                     use_ext = "0" + use_ext;
                  }
                  
                  QString use_fn = fn + "-" + use_ext + ".pdb";
                  
                  QFile fn_out( use_fn );
                  
                  if ( !fn_out.open( QIODevice::WriteOnly ) ) {
                     QMessageBox::warning( this, "US-SOMO: PDB Editor : Split",
                                           QString(us_tr("Could not open %1 for writing!")).arg( use_fn ) );
                     return false;
                  }
                  
                  QTextStream tso( &fn_out );
               
                  tso << QString("HEADER    split from %1: Model %2 of %3\n").arg( f.fileName() ).arg( pos + 1 ).arg( model_count );
                  tso << model_header;
                  tso << QString("").sprintf("MODEL  %7s\n", model_name_vector[ pos ].toLatin1().data() );
                  tso << model_lines;
                  tso << "ENDMDL\nEND\n";
                  
                  fn_out.close();
                  hullrad_to_process << fn_out.fileName();
                  editor_msg( "dark blue", QString( us_tr( "File %1 written" ) ).arg( fn_out.fileName() ) );
                  qApp->processEvents();
               } else {
                  // editor_msg( "dark red", QString("model %1 skipped").arg( model_name_vector[ pos ] ) );
               }
               in_model = false;
               model_lines = "";
               pos++;
            }
            if ( qs.contains( rx_model ) ||
                 ( no_model_directives && qs.contains( rx_end ) ) )
            {
               in_model = true;
               model_lines = "";
            }
         } else {
            if ( in_model )
            {
               model_lines += qs + "\n";
            }
         }
      } while ( !ts.atEnd() );
   }
   f.close();

   hullrad_running = true;
   hullrad_process_next();

   // summarize results into csv, hydrodyn results
   return true;
}

void US_Hydrodyn::hullrad_process_next() {
   
   // us_qdebug( QString( "hullrad_process_next %1" ).arg( hullrad_filename ) );

   if ( !hullrad_to_process.size() ) {
      hullrad_finalize();
      return;
   }

   hullrad_last_processed = hullrad_to_process[ 0 ];
   hullrad_to_process.pop_front();
   hullrad_processed.push_back( hullrad_last_processed ); 

   hullrad_stdout = "";

   hullrad = new QProcess( this );
   //   hullrad->setWorkingDirectory( dir );
   // us_qdebug( "prog is " + hullrad_prog );
#if QT_VERSION < 0x040000
   hullrad->addArgument( hullrad_prog );
   hullrad->addArgument( hullrad_last_processed );

   connect( hullrad, SIGNAL(readyReadStandardOutput()), this, SLOT(hullrad_readFromStdout()) );
   connect( hullrad, SIGNAL(readyReadStandardError()), this, SLOT(hullrad_readFromStderr()) );
   connect( hullrad, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(hullrad_finished( int, QProcess::ExitStatus )) );
   connect( hullrad, SIGNAL(started()), this, SLOT(hullrad_started()) );

   editor_msg( "black", "\nStarting Hullrad\n");
   hullrad->start();
#else
   {
      QStringList args;
      args << hullrad_last_processed;

      connect( hullrad, SIGNAL(readyReadStandardOutput()), this, SLOT(hullrad_readFromStdout()) );
      connect( hullrad, SIGNAL(readyReadStandardError()), this, SLOT(hullrad_readFromStderr()) );
      connect( hullrad, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(hullrad_finished( int, QProcess::ExitStatus )) );
      connect( hullrad, SIGNAL(started()), this, SLOT(hullrad_started()) );

      editor_msg( "black", "\nStarting Hullrad\n");
      hullrad->start( hullrad_prog, args, QIODevice::ReadOnly );
   }
#endif
   
   return;
}

void US_Hydrodyn::hullrad_readFromStdout()
{
   // us_qdebug( QString( "hullrad_readFromStdout %1" ).arg( hullrad_filename ) );
#if QT_VERSION < 0x040000
   while ( hullrad->canReadLineStdout() )
   {
      QString qs = hullrad->readLineStdout() + "\n";
      hullrad_stdout += qs;
      editor_msg("brown", qs );
   }
#else
   QString qs = QString( hullrad->readAllStandardOutput() );
   hullrad_stdout += qs;
   editor_msg( "brown", qs );
#endif   
   //  qApp->processEvents();
}
   
void US_Hydrodyn::hullrad_readFromStderr()
{
   // us_qdebug( QString( "hullrad_readFromStderr %1" ).arg( hullrad_filename ) );

#if QT_VERSION < 0x040000
   while ( hullrad->canReadLineStderr() )
   {
      editor_msg("red", hullrad->readLineStderr() + "\n");
   }
#else
   editor_msg( "red", QString( hullrad->readAllStandardError() ) );
#endif   
   //  qApp->processEvents();
}

bool US_Hydrodyn::calc_fasta_vbar( QStringList & seq_chars, double & result, QString & msgs ) {
   result = 0e0;
   msgs = "";
   if ( ! seq_chars.size() ) {
      msgs = "Error: The sequence is empty!";
      return false;
   }
      
   double mw_vbar_sum = 0.0;
   double mw_sum = 0.0;
   // double mw;

   map < QString, int > replaced;

   for ( int i = 0; i < (int) seq_chars.size(); ++i ) {
      QString se = seq_chars[ i ];
      if ( !fasta_mw.count( se ) ) {
         replaced[ se ]++;
         se = "A";
      }
      if ( !fasta_mw.count( se ) ) {
         msgs = "Error: No A defined in sequence database";
         return false;
      }
         
      mw_sum      += fasta_mw[ se ];
      mw_vbar_sum += fasta_mw[ se ] * fasta_vbar[ se ];
   }

   if ( replaced.size() ) {
      msgs = "Notice:\n";
      for ( map < QString, int >::iterator it = replaced.begin();
            it != replaced.end();
            ++it ) {
         msgs += QString( us_tr( "Unrecognized code '%1' replaced by 'A' %2 time(s)\n" ) )
            .arg( it->first )
            .arg( it->second )
            ;
      }
   }
   if ( ! seq_chars.size() ) {
      msgs = "Error: The sequence sum results is zero molecular weight.";
      return false;
   }

   result = (double)floor(0.5 + ( ( mw_vbar_sum / mw_sum ) * 1000e0 ) ) / 1000e0;
   // qDebug() << "calc_fasta_vbar result for seq of " << seq_chars.size() << " elements computed as " << result;
   return true;
}

bool US_Hydrodyn::pat_model( vector < PDB_atom > & model ) {
   vector < dati1_supc > in_dt;
   int nat = (int) model.size();
   {
      int dt_pos = 0;
      for ( int j = 0; j < nat; ++j )  {
         PDB_atom *this_atom = &( model[ j ] );
            
         dati1_supc dt;
               
         dt.x  = this_atom->bead_coordinate.axis[ 0 ];
         dt.y  = this_atom->bead_coordinate.axis[ 1 ];
         dt.z  = this_atom->bead_coordinate.axis[ 2 ];
         dt.r  = this_atom->bead_computed_radius;
         dt.ru = this_atom->bead_computed_radius;
         dt.m  = this_atom->bead_mw + this_atom->bead_ionized_mw_delta;
            
         in_dt.push_back( dt );

         // us_qdebug( QString( "atom %1 coords %2 %3 %4 radius %5 mw %6" )
         //         .arg( dt_pos )
         //         .arg( dt.x )
         //         .arg( dt.y )
         //         .arg( dt.z )
         //         .arg( dt.r )
         //         .arg( dt.m )
         //         );
         ++dt_pos;
      }
   }

   int out_nat;
   vector < dati1_pat > out_dt( in_dt.size() + 1 );
   
   // write_dati1_supc_bead_model( "us_hydrodyn_pat_model_pre_pat", ( int ) in_dt.size(),  &( in_dt[ 0 ] ) );
   if ( !us_hydrodyn_pat_main( ( int ) in_dt.size(),
                               ( int ) in_dt.size(),
                               &( in_dt[ 0 ] ),
                               &out_nat,
                               &( out_dt[ 0 ] ) )
        ) {
      // cout << QString( "pat ok, out_nat %1\n" ).arg( out_nat );
      int dt_pos = 0;

      // write_dati1_pat_bead_model( "us_hydrodyn_pat_model_post_pat", out_nat,  &( out_dt[ 0 ] ), &( in_dt[ 0 ] ) );

      for ( int j = 0; j < nat; ++j )  {
         PDB_atom *this_atom = &( model[ j ] );
            
         this_atom->bead_coordinate.axis[ 0 ] = out_dt[ dt_pos ].x;
         this_atom->bead_coordinate.axis[ 1 ] = out_dt[ dt_pos ].y;
         this_atom->bead_coordinate.axis[ 2 ] = out_dt[ dt_pos ].z;
         // us_qdebug( QString( "results atom %1 coords %2 %3 %4" )
         //         .arg( dt_pos )
         //         .arg( out_dt[ dt_pos ].x )
         //         .arg( out_dt[ dt_pos ].y )
         //         .arg( out_dt[ dt_pos ].z )
         //         );

         ++dt_pos;
      }
   } else {
      return false;
   }
   return true;
}   
