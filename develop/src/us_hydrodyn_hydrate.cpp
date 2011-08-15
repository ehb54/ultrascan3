#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_asab1.h"
#include "../include/us_surfracer.h"
#include "qmessagebox.h"

#undef DEBUG

#define DOTSOMO      ""
#ifndef WIN32
#   include <unistd.h>
#   define SLASH "/"
#   define __open open
#else
#   define chdir _chdir
#   define dup2 _dup2
#   define __open _open
#   include <direct.h>
#   include <io.h>
#   define SLASH "\\"
#   define STDOUT_FILENO 1
#   define STDERR_FILENO 2
#endif

// #define DEBUG_DIHEDRAL
// #define DEBUG_TO_HYDRATE_DIHEDRAL
#define MAX_WATER_POSITIONING_ATOMS 4

int US_Hydrodyn::pdb_hydrate_for_saxs( bool quiet )
{
   QString error_msg;
   if ( !load_rotamer( error_msg ) )
   {
      QMessageBox::warning( this,
                            tr( "Error reading Hydrated Rotamer file" ),
                            error_msg );
      return -1;
   } 
   //   cout << list_rotamers( false );

   vector < unsigned int > selected_models;
   for ( unsigned int i = 0; i < (unsigned int)lb_model->numRows(); i++ ) 
   {
      if ( lb_model->isSelected(i) ) 
      {
         selected_models.push_back(i);
      }
   }
   if ( selected_models.size() != 1 )
   {
      QMessageBox::message(tr("Please note:"),
                           tr("You must select exactly one model to hydrate."));
      return -1;
   } 

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_somo->setEnabled(false);

   bd_anaflex_enables(false);

   pb_grid_pdb->setEnabled(false);
   pb_grid->setEnabled(false);
   options_log = "";
   display_default_differences();
   model_vector = model_vector_as_loaded;

   if (!residue_list.size() ||
       !model_vector.size())
   {
      fprintf(stderr, "calculations can not be run until residue & pdb files are read!\n");
      pb_stop_calc->setEnabled(false);
      return -1;
   }

   pb_visualize->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);

   bool any_errors = false;
   bool any_models = false;

   QString msg = QString("\n%1 models selected:").arg(project);

   for(int i = 0; i < lb_model->numRows(); i++) {
      if (lb_model->isSelected(i)) {
         current_model = i;
         msg += QString(" %1").arg(i+1);
      }
   }
   msg += "\n";
   editor->append(msg);

   for (current_model = 0; current_model < (unsigned int)lb_model->numRows(); current_model++)
   {
      if (!any_errors && lb_model->isSelected(current_model))
      {
         any_models = true;
         if(!pdb_asa_for_saxs_hydrate())
         {
            // list_exposed();
            // view_exposed();
            build_to_hydrate();
            // cout << list_to_hydrate();
            progress->setProgress(9);
            qApp->processEvents();
            if ( !compute_to_hydrate_dihedrals( error_msg ) )
            {
               QMessageBox::warning( this,
                                     tr( "Error computing dihedrals of exposed side chain" ),
                                     error_msg );
               any_errors = true;
            }
            // cout << list_to_hydrate_dihedrals();
            progress->setProgress(10);
            qApp->processEvents();
            if ( !compute_best_fit_rotamer( error_msg ) )
            {
               QMessageBox::warning( this,
                                     tr( "Error finding best fit rotamer" ),
                                     error_msg );
               any_errors = true;
            }
            // cout << list_best_fit_rotamer();
            progress->setProgress(11);
            qApp->processEvents();
            if ( !compute_waters_to_add( error_msg ) )
            {
               QMessageBox::warning( this,
                                     tr( "Error trying to add waters" ),
                                     error_msg );
               any_errors = true;
            }
            // cout << list_waters_to_add();
            progress->setProgress(1, 1);
            qApp->processEvents();
            if ( !write_pdb_with_waters( error_msg ) )
            {
               QMessageBox::warning( this,
                                     tr( "Error trying to write pdb with waters" ),
                                     error_msg );
               any_errors = true;
            }
            if ( !any_errors &&
                 !screen_pdb( last_hydrated_pdb_name, !quiet ) )
            {
               QMessageBox::warning( this,
                                     tr( "Error trying to reload hydrated pdb" ),
                                     error_msg );
               any_errors = true;
            }
            if ( !any_errors )
            {
               pdb_saxs();
            }
         }
         else
         {
            any_errors = true;
         }
      }
      if (stopFlag)
      {
         editor->append("Stopped by user\n\n");
         pb_somo->setEnabled(true);
         bd_anaflex_enables( ( ( browflex && browflex->isRunning() ) ||
                               ( anaflex && anaflex->isRunning() ) ) ? false : true );
         pb_grid_pdb->setEnabled(true);
         progress->reset();
         return -1;
      }
   }
   if (any_models && !any_errors)
   {
      qApp->processEvents();
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_somo->setEnabled(true);
   pb_grid_pdb->setEnabled(true);
   pb_grid->setEnabled(true);
   pb_stop_calc->setEnabled(false);
   return 0;
}

int US_Hydrodyn::pdb_asa_for_saxs_hydrate()
{
   QString error_string = "";
   progress->reset();
   editor->append(QString("\nHydrating the pdb for %1 model %2\n").arg(project).arg(current_model+1));
   editor->append("Checking the pdb structure\n");
   if (check_for_missing_atoms(&error_string, &model_vector[current_model])) {
      editor->append("Encountered the following errors with your PDB structure:\n" +
                     error_string);
      printError("Encountered errors with your PDB structure:\n"
                 "please check the text window");
      return -1;
   }
   results.asa_rg_pos = 0.0;
   results.asa_rg_neg = 0.0;
   editor->append("PDB structure ok\n");
   int mppos = 12;
   progress->setTotalSteps(mppos);
   int ppos = 1;
   progress->setProgress(ppos++); // 1
   qApp->processEvents();

   {
      int no_of_atoms = 0;
      int no_of_molecules = model_vector[current_model].molecule.size();
      int i;
      for (i = 0; i < no_of_molecules; i++) {
         no_of_atoms +=  model_vector[current_model].molecule[i].atom.size();
      }

      editor->append(QString("There are %1 atoms in %2 chain(s) in this model\n").arg(no_of_atoms).arg(no_of_molecules));
   }
   if (stopFlag)
   {
      return -1;
   }
   int retval = create_beads(&error_string, true);
   if ( retval )
   {
      editor->append("Errors found\n");
      progress->setProgress(mppos);
      qApp->processEvents();
      if (stopFlag)
      {
         return -1;
      }
      switch ( retval )
      {
      case US_SURFRACER_ERR_MISSING_RESIDUE:
         {
            printError("Encountered an unknown residue:\n" +
                       error_string);
            return US_SURFRACER_ERR_MISSING_RESIDUE;
            break;
         }
      case US_SURFRACER_ERR_MISSING_ATOM:
         {
            printError("Encountered a unknown atom:\n" +
                       error_string);
            return US_SURFRACER_ERR_MISSING_ATOM;
            break;
         }
      case US_SURFRACER_ERR_MEMORY_ALLOC:
         {
            printError("Encountered a memory allocation error");
            return US_SURFRACER_ERR_MEMORY_ALLOC;
            break;
         }
      default:
         {
            printError("Encountered an unknown error");
            // unknown error
            return -1;
            break;
         }
      }
   }

   if(error_string.length()) {
      progress->setProgress(mppos);
      qApp->processEvents();
      printError("Encountered unknown atom(s) error:\n" +
                 error_string);
      return US_SURFRACER_ERR_MISSING_ATOM;
   }

   if(asa.method == 0) {
      // surfracer
      editor->append("Computing ASA via SurfRacer\n");
      qApp->processEvents();
      if (stopFlag)
      {
         return -1;
      }
      int retval = surfracer_main(asa.hydrate_probe_radius,
                                  active_atoms,
                                  false,
                                  progress,
                                  editor
                                  );

      progress->setProgress(ppos++); // 2
      qApp->processEvents();
      editor->append("Return from Computing ASA\n");
      if (stopFlag)
      {
         return -1;
      }
      if ( retval )
      {
         editor->append("Errors found during ASA calculation\n");
         progress->setProgress(mppos);
         qApp->processEvents();
         if (stopFlag)
         {
            return -1;
         }
         switch ( retval )
         {
         case US_SURFRACER_ERR_MISSING_RESIDUE:
            {
               printError("US_SURFRACER encountered an unknown residue:\n" +
                          error_string);
               return US_SURFRACER_ERR_MISSING_RESIDUE;
               break;
            }
         case US_SURFRACER_ERR_MISSING_ATOM:
            {
               printError("US_SURFRACER encountered a unknown atom:\n" +
                          error_string);
               return US_SURFRACER_ERR_MISSING_ATOM;
               break;
            }
         case US_SURFRACER_ERR_MEMORY_ALLOC:
            {
               printError("US_SURFRACER encountered a memory allocation error");
               return US_SURFRACER_ERR_MEMORY_ALLOC;
               break;
            }
         default:
            {
               printError("US_SURFRACER encountered an unknown error");
               // unknown error
               return -1;
               break;
            }
         }
      }
      if(error_string.length()) {
         progress->setProgress(mppos);
         qApp->processEvents();
         printError("US_SURFRACER encountered unknown atom(s) error:\n" +
                    error_string);
         return US_SURFRACER_ERR_MISSING_ATOM;
      }
   }

   if(asa.method == 1) {
      // surfracer
      editor->append("Computing ASA via ASAB1\n");
      qApp->processEvents();
      if (stopFlag)
      {
         return -1;
      }
      int retval = us_hydrodyn_asab1_main(active_atoms,
                                          &asa,
                                          &results,
                                          false,
                                          progress,
                                          editor,
                                          this
                                          );

      progress->setProgress(ppos++); // 2
      qApp->processEvents();
      editor->append("Return from Computing ASA\n");
      if (stopFlag)
      {
         return -1;
      }
      if ( retval )
      {
         editor->append("Errors found during ASA calculation\n");
         progress->setProgress(mppos);
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

   // pass 1 assign bead #'s, chain #'s, initialize data

   FILE *asaf = fopen(QString(somo_tmp_dir + SLASH + "atom.asa"), "w");

   // for (unsigned int i = 0; i < model_vector.size (); i++)
   {
      unsigned int i = current_model;

      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p1 i j k %d %d %d %lx %s\n", i, j, k, (long unsigned int)this_atom->p_atom, this_atom->active ? "active" : "not active"); fflush(stdout);

            //   this_atom->bead_assignment =
            //     (this_atom->p_atom ? (int) this_atom->p_atom->bead_assignment : -1);
            //   printf("this_atom->bead_assignment %d\n", this_atom->bead_assignment); fflush(stdout);
            //   this_atom->chain =
            //     ((this_atom->p_residue && this_atom->p_atom) ?
            //      (int) this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].chain : -1);

            // initialize data
            // this_atom->bead_positioner = false;
            this_atom->normalized_ot_is_valid = false;
            fprintf(asaf, "%s\t%s\t%s\t%.2f\n",
                    this_atom->name.ascii(),
                    this_atom->resName.ascii(),
                    this_atom->resSeq.ascii(),
                    this_atom->asa);

            for (unsigned int m = 0; m < 3; m++) {
               this_atom->bead_cog_coordinate.axis[m] = 0;
               this_atom->bead_position_coordinate.axis[m] = 0;
               this_atom->bead_coordinate.axis[m] = 0;
            }
         }
      }
   }
   fclose(asaf);

   progress->setProgress(ppos++); // 3
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }

   // #define DEBUG
   // pass 2 determine beads, cog_position, fixed_position, molecular cog phase 1.

   int count_actives;
   float molecular_cog[3] = { 0, 0, 0 };
   float molecular_mw = 0;
   QString cog_msg = "COG calc summary information\n";

   // for (unsigned int i = 0; i < model_vector.size (); i++)
   {
      unsigned int i = current_model;

      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         int last_bead_assignment = -1;
         int last_chain = -1;
         QString last_resName = "not a residue";
         QString last_resSeq = "";
         PDB_atom *last_main_chain_bead = (PDB_atom *) 0;
         PDB_atom *last_main_bead = (PDB_atom *) 0;
         PDB_atom *sidechain_N = (PDB_atom *) 0;

         count_actives = 0;
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p2 i j k %d %d %d %lx\n", i, j, k, this_atom->p_atom); fflush(stdout);
            // this_atom->bead_positioner = false;
            if (this_atom->active) {
               if ( advanced_config.debug_1 )
               {
                  printf("pass 2 active %s %s %d pm %d %d\n",
                         this_atom->name.ascii(),
                         this_atom->resName.ascii(),
                         this_atom->serial,
                         this_atom->placing_method,
                         this_atom->bead_assignment); fflush(stdout);
               }
               molecular_mw += this_atom->mw;
               for (unsigned int m = 0; m < 3; m++) {
                  molecular_cog[m] += this_atom->coordinate.axis[m] * this_atom->mw;
               }

               this_atom->bead_mw = 0;
               this_atom->bead_asa = 0;
               this_atom->bead_recheck_asa = 0;

               // do we have a new bead?
               // we want to put the N on a previous bead unless it is the first one of the molecule
               // ONLY FOR residue type = 0! (amino acid)
               // AND ONLY for residues not part of the 'auto bead builder'

               if (!create_beads_normally ||
                   (
                    (this_atom->bead_assignment != last_bead_assignment ||
                     this_atom->chain != last_chain ||
                     this_atom->resName != last_resName ||
                     this_atom->resSeq != last_resSeq) &&
                    !(misc.pb_rule_on &&
                      this_atom->chain == 0 &&
                      this_atom->name == "N" &&
                      count_actives &&
                      !broken_chain_head.count(QString("%1|%2")
                                               .arg(this_atom->resSeq)
                                               .arg(this_atom->resName))
                      ) ) ) {
                  if ( advanced_config.debug_1 ||
                       advanced_config.debug_2 )
                  {
                     printf("pass 2 active %s %s %d new bead chain %d\n",
                            this_atom->name.ascii(),
                            this_atom->resName.ascii(),
                            this_atom->serial,
                            this_atom->chain); fflush(stdout);
                  }

                  // this_atom->bead_positioner = true;
                  this_atom->is_bead = true;
                  last_main_bead = this_atom;
                  last_bead_assignment = this_atom->bead_assignment;
                  last_chain = this_atom->chain;
                  last_resName = this_atom->resName;
                  last_resSeq = this_atom->resSeq;
                  if (create_beads_normally && 
                      misc.pb_rule_on &&
                      !this_atom->type) {
                     if(sidechain_N &&
                        this_atom->chain == 1) {
                        if(this_atom->name == "N") {
                           printf("ERROR double N on sequential sidechains! PRO PRO?\n");
                        }
                        if ( advanced_config.debug_1 ||
                             advanced_config.debug_2 )
                        {
                           printf("adding sidechain N %f %f to this_atom\n",
                                  sidechain_N->asa,
                                  sidechain_N->mw
                                  );
                        }
                        this_atom->bead_asa += sidechain_N->bead_asa;
                        this_atom->bead_mw += sidechain_N->bead_mw;
                        sidechain_N->bead_mw = 0;
                        sidechain_N->bead_asa = 0;
                        sidechain_N = (PDB_atom *) 0;
                     }
                     if(this_atom->name == "N" &&
                        this_atom->chain == 1) {
                        sidechain_N = this_atom;
                        this_atom->is_bead = false;
                     }
                  }
               }
               else 
               {
                  if ( advanced_config.debug_1 )
                  {
                     printf("pass 2 active %s %s %d not a new bead\n",
                            this_atom->name.ascii(),
                            this_atom->resName.ascii(),
                            this_atom->serial); fflush(stdout);
                  }
                  if (this_atom->bead_positioner) {

                     if (last_main_bead->bead_positioner &&
                         this_atom->placing_method == 1) {
                        fprintf(stderr, "warning: 2 positioners in bead %s %s %d\n",
                                last_main_bead->name.ascii(),
                                last_main_bead->resName.ascii(),
                                last_main_bead->serial);
                     }
                     last_main_bead->bead_positioner = true;
                     last_main_bead->bead_position_coordinate = this_atom->coordinate;
                  }

                  this_atom->is_bead = false;
                  // this_atom->bead_cog_mw = 0;
               }

               this_atom->bead_cog_mw = 0;

               // special nitrogen asa handling
               PDB_atom *use_atom;
               if (misc.pb_rule_on &&
                   create_beads_normally &&
                   this_atom->chain == 0 &&
                   this_atom->name == "N" &&
                   !broken_chain_head.count(QString("%1|%2")
                                           .arg(this_atom->resSeq)
                                           .arg(this_atom->resName)) &&
                   last_main_chain_bead) {
                  use_atom = last_main_chain_bead;
               }
               else 
               {
                  use_atom = last_main_bead;
               }

               use_atom->bead_asa += this_atom->asa;
               use_atom->bead_mw += this_atom->mw;
               if ( advanced_config.debug_1 )
               {
                  printf("atom %s %s p_atom.hybrid.mw %f atom.mw %f\n",
                         this_atom->name.ascii(),
                         this_atom->resName.ascii(),
                         this_atom->p_atom->hybrid.mw,
                         this_atom->mw
                         );
               }
               // accum
               if (!create_beads_normally ||
                   this_atom->bead_positioner) {
                  if ( advanced_config.debug_3 )
                  {
                     printf("adding cog from %d to %d mw %f totmw %f (this pos [%f,%f,%f], org pos [%f,%f,%f])\n", 
                            this_atom->serial, 
                            use_atom->serial, 
                            this_atom->mw, 
                            use_atom->bead_cog_mw,
                            this_atom->coordinate.axis[0],
                            this_atom->coordinate.axis[1],
                            this_atom->coordinate.axis[2],
                            use_atom->bead_cog_coordinate.axis[0],
                            use_atom->bead_cog_coordinate.axis[1],
                            use_atom->bead_cog_coordinate.axis[2]
                            );
                  }
                  cog_msg += QString("adding %1 to %2\n").arg(this_atom->serial).arg(use_atom->serial);
                  use_atom->bead_cog_mw += this_atom->mw;
                  for (unsigned int m = 0; m < 3; m++) {
                     use_atom->bead_cog_coordinate.axis[m] +=
                        this_atom->coordinate.axis[m] * this_atom->mw;
                  }
                  if ( advanced_config.debug_3 )
                  {
                     printf("afterwards: target mw %f pos [%f,%f,%f]\n",
                            use_atom->bead_cog_mw,
                            use_atom->bead_cog_coordinate.axis[0],
                            use_atom->bead_cog_coordinate.axis[1],
                            use_atom->bead_cog_coordinate.axis[2]);
                  }
               }
               else 
               {
                  if ( advanced_config.debug_3 )
                  {
                     printf("notice: atom %s %s %d excluded from cog calculation in bead %s %s %d\n",
                            this_atom->name.ascii(),
                            this_atom->resName.ascii(),
                            this_atom->serial,
                            use_atom->name.ascii(),
                            use_atom->resName.ascii(),
                            use_atom->serial);
                  }
               }

               if (!create_beads_normally ||
                   this_atom->bead_positioner) {
                  if (use_atom->bead_positioner &&
                      this_atom->placing_method == 1) {
                     fprintf(stderr, "warning: 2 or more positioners in bead %s %s %d\n",
                             use_atom->name.ascii(),
                             use_atom->resName.ascii(),
                             use_atom->serial);
                  }
                  use_atom->bead_positioner = true;
                  use_atom->bead_position_coordinate = this_atom->coordinate;
               }

               if (this_atom->chain == 0 &&
                   misc.pb_rule_on &&
                   this_atom->name == "N" &&
                   !broken_chain_head.count(QString("%1|%2")
                                            .arg(this_atom->resSeq)
                                            .arg(this_atom->resName)) && 
                   !count_actives)
               {
                  last_resName = "first N";
               }

               if (this_atom->chain == 0 &&
                   this_atom->name == "CA") {
                  last_main_chain_bead = this_atom;
               }
               count_actives++;
            }
            else 
            {
               this_atom->is_bead = false;
            }
         }
      }
   }
   if ( advanced_config.debug_3 )
   {
      cout << cog_msg;
   }

   if (molecular_mw) {
      for (unsigned int m = 0; m < 3; m++) {
         molecular_cog[m] /= molecular_mw;
      }
      if ( advanced_config.debug_3 )
      {
         printf("molecular cog [%f,%f,%f] mw %f\n",
                molecular_cog[0],
                molecular_cog[1],
                molecular_cog[2],
                molecular_mw);
      }
   }
   else 
   {
      printf("ERROR: this molecule has zero mw!\n");
   }

   for (unsigned int m = 0; m < 3; m++) {
      last_molecular_cog.axis[m] = molecular_cog[m];
   }

   progress->setProgress(ppos++); // 4
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }

   // pass 2b move bead_ref_volume, ref_mw, computed_radius from
   // next main chain back one including adjustments for GLY, PRO, OXT

   // for (unsigned int i = 0; i < model_vector.size (); i++)   {
   {
      unsigned int i = current_model;
      bool placed_N1 = false;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         bool first_is_pro = false;
         unsigned int main_chain_beads = 0;
         PDB_atom *last_main_chain_bead = (PDB_atom *) 0;
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            if ( advanced_config.debug_1 )
            {
               printf("pass 2b active %d is_bead %d chain %d atom %s %s %d pm %d\n",
                      this_atom->active,
                      this_atom->is_bead,
                      this_atom->chain,
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->serial,
                      this_atom->placing_method); fflush(stdout);
            }
            if ( this_atom->active &&
                 this_atom->is_bead &&
                 this_atom->chain == 0 )
            {
               main_chain_beads++;
            }
            if ( 
                misc.pb_rule_on &&
                !k &&
                this_atom->resName == "PRO" 
                )
            {
               if ( advanced_config.debug_1 )
               {
                  puts("pass 2b PRO 1st is pro recognized");
               }
               first_is_pro = true;
            }
            if ( placed_N1 &&
                 broken_chain_end.count(QString("%1|%2")
                                        .arg(this_atom->resSeq)
                                        .arg(this_atom->resName))
                 )
            {
               placed_N1 = false;
               if ( advanced_config.debug_1 )
               {
                  printf("placed N1, found break, turned off placed N1 %s %s\n", 
                         this_atom->resName.ascii(),
                         this_atom->resSeq.ascii()
                         );
               }
            }

            if ( 
                misc.pb_rule_on &&
                !k &&
                this_atom->resName != "PRO" &&
                this_atom->name == "N" &&
                !broken_chain_head.count(QString("%1|%2")
                                         .arg(this_atom->resSeq)
                                         .arg(this_atom->resName))
                )
            {
               placed_N1 = true;
               if ( advanced_config.debug_1 )
               {
                  puts("placed N1");
               }
            }
            if ( 
                first_is_pro &&
                this_atom->active &&
                this_atom->is_bead &&
                this_atom->chain == 1 &&
                this_atom->resName == "PRO"
                )
            {
               if ( advanced_config.debug_1 )
               {
                  puts("pass 2b PRO 1st N +1 mw adjustment");
               }
               this_atom->bead_ref_mw += 1.0;
               // what about a volume adjustment?
               first_is_pro = false;
            }
                 
            if (this_atom->name == "OXT" &&
                last_main_chain_bead) {
               if ( advanced_config.debug_1 )
               {
                  printf("pass 2b active OXT %s %s %d last %s %s %d mw org %f mw new %f\n",
                         this_atom->name.ascii(),
                         this_atom->resName.ascii(),
                         this_atom->serial,
                         last_main_chain_bead->name.ascii(),
                         last_main_chain_bead->resName.ascii(),
                         last_main_chain_bead->serial,
                         last_main_chain_bead->bead_ref_mw,
                         this_atom->bead_ref_mw
                         );
                  fflush(stdout);
               }
               this_atom->is_bead = false;
               // override broken head OXT residue
               if ( misc.pb_rule_on &&
                    this_atom->resName != "PRO" &&
                    broken_chain_head.count(QString("%1|%2")
                                            .arg(this_atom->resSeq)
                                            .arg(this_atom->resName)) &&
                    multi_residue_map.count("NPBR-OXT") )
               {
                  if ( advanced_config.debug_1 )
                  {
                     puts("pass 2b broken head OXT NPBR replacement");
                  }
                  int posNPBR_OXT = multi_residue_map["NPBR-OXT"][0];
                  this_atom->bead_ref_volume = residue_list[posNPBR_OXT].r_bead[0].volume;
                  this_atom->bead_ref_mw = residue_list[posNPBR_OXT].r_bead[0].mw;
               }
                  
               last_main_chain_bead->bead_ref_volume = this_atom->bead_ref_volume;
               last_main_chain_bead->bead_ref_mw = this_atom->bead_ref_mw;
               if (last_main_chain_bead->resName == "GLY") 
               {
                  if ( advanced_config.debug_1 )
                  {
                     puts("pass 2b GLY adjustment +1");
                  }
                  last_main_chain_bead->bead_ref_mw += 1.01f;
               }
               if ( !misc.pb_rule_on &&
                    main_chain_beads == 1 &&
                    this_atom->resName != "PRO" )
               {
                  if ( advanced_config.debug_1 )
                  {
                     puts("pass 2b main_chain bead adjustment +1");
                  }
                  last_main_chain_bead->bead_ref_mw += 1.0;
               }
               last_main_chain_bead->bead_computed_radius = this_atom->bead_computed_radius;
            } // OXT

            if (this_atom->active &&
                this_atom->is_bead &&
                this_atom->chain == 0) {
               if ( advanced_config.debug_1 )
               {
                  printf("pass 2b active, bead, chain == 0 %s %s %d pm %d\n",
                         this_atom->name.ascii(),
                         this_atom->resName.ascii(),
                         this_atom->serial,
                         this_atom->placing_method); fflush(stdout);
               }

               if (misc.pb_rule_on &&
                   last_main_chain_bead &&
                   !broken_chain_head.count(QString("%1|%2")
                                            .arg(this_atom->resSeq)
                                            .arg(this_atom->resName)) &&
                   (this_atom->resName == "PRO" ||
                    last_main_chain_bead->resName == "PRO")
                   ) {

                  if ( advanced_config.debug_1 )
                  {
                     printf("pass 2b active PRO %s %s %d pm %d\n",
                            this_atom->name.ascii(),
                            this_atom->resName.ascii(),
                            this_atom->serial,
                            this_atom->placing_method); fflush(stdout);
                  }

                  last_main_chain_bead->bead_ref_volume = this_atom->bead_ref_volume;
                  last_main_chain_bead->bead_ref_mw = this_atom->bead_ref_mw;
                  last_main_chain_bead->bead_computed_radius = this_atom->bead_computed_radius;
                  if (this_atom->resName == "GLY") {
                     last_main_chain_bead->bead_ref_mw -= 1.01f;
                     if ( advanced_config.debug_1 )
                     {
                        puts("pass 2b GLY adjustment -1");
                     }
                  }
                  if (last_main_chain_bead->resName == "GLY") {
                     last_main_chain_bead->bead_ref_mw += 1.01f;
                     if ( advanced_config.debug_1 )
                     {
                        puts("pass 2b GLY adjustment +1");
                     }
                  }
               } // PRO
               last_main_chain_bead = this_atom;
            }

            // fix up mw, vol at end for broken end when PBR rule is on
            if ( misc.pb_rule_on &&
                 this_atom->p_residue->type == 0 &&
                 this_atom->is_bead &&
                 this_atom->chain == 0 &&
                 broken_chain_end.count(QString("%1|%2")
                                        .arg(this_atom->resSeq)
                                        .arg(this_atom->resName)) &&
                 !broken_chain_head.count(QString("%1|%2")
                                        .arg(this_atom->resSeq)
                                        .arg(this_atom->resName)) &&
                 this_atom->resName != "PRO" &&
                 (k || this_atom->name != "N")
                 )
            {
               if ( advanced_config.debug_1 )
               {
                  printf("pass 2b broken end adjustment %s %s %s org mw %.4f vol %.4f\n",
                         this_atom->name.ascii(),
                         this_atom->resName.ascii(),
                         this_atom->resSeq.ascii(),
                         this_atom->bead_ref_mw,
                         this_atom->bead_ref_volume);
               }
               if ( multi_residue_map["PBR-NO-OXT"].size() == 1 )
               {
                  int pos = multi_residue_map["PBR-NO-OXT"][0];
                  this_atom->bead_ref_volume = residue_list[pos].r_bead[0].volume;
                  this_atom->bead_ref_mw = residue_list[pos].r_bead[0].mw;
                  this_atom->bead_computed_radius = pow(3 * last_main_chain_bead->bead_ref_volume / (4.0*M_PI), 1.0/3);
                  if (this_atom->resName == "GLY") {
                     this_atom->bead_ref_mw += 1.01f;
                     if ( advanced_config.debug_1 )
                     {
                        puts("pass 2b GLY adjustment +1 on broken end");
                     }
                  }
                  if ( advanced_config.debug_1 )
                  {
                     printf("pass 2b broken end adjustment %s %s %s new mw %.4f vol %.4f\n",
                            this_atom->name.ascii(),
                            this_atom->resName.ascii(),
                            this_atom->resSeq.ascii(),
                            this_atom->bead_ref_mw,
                            this_atom->bead_ref_volume);
                  }
               }
               else
               {
                  QColor save_color = editor->color();
                  editor->setColor("red");
                  editor->append("Chain has broken end and PBR-NO-OXT isn't uniquely defined in the residue file.");
                  editor->setColor(save_color);
               }
            }
         } // for k < atom.size()
         // fix up mw, vol at end for no OXT when PBR rule is on
         if ( misc.pb_rule_on &&
              last_main_chain_bead &&
              last_main_chain_bead->p_residue->type == 0 &&
              last_main_chain_bead->name != "PRO" &&
              !has_OXT[QString("%1|%2|%3")
               .arg(j)
               .arg(last_main_chain_bead->resName)
               .arg(last_main_chain_bead->resSeq)]  &&
              !broken_chain_head.count(QString("%1|%2")
                                       .arg(last_main_chain_bead->resSeq)
                                       .arg(last_main_chain_bead->resName)) )
         {
            if ( advanced_config.debug_1 )
            {
               puts("pass 2b missing OXT adjustment");
            }
            if ( multi_residue_map["PBR-NO-OXT"].size() == 1 )
            {
               int pos = multi_residue_map["PBR-NO-OXT"][0];
               last_main_chain_bead->bead_ref_volume = residue_list[pos].r_bead[0].volume;
               last_main_chain_bead->bead_ref_mw = residue_list[pos].r_bead[0].mw;
               last_main_chain_bead->bead_computed_radius = pow(3 * last_main_chain_bead->bead_ref_volume / (4.0*M_PI), 1.0/3);
               if (last_main_chain_bead->resName == "GLY") {
                  last_main_chain_bead->bead_ref_mw += 1.01f;
                  if ( advanced_config.debug_1 )
                  {
                     puts("pass 2b GLY adjustment +1 on last MC of atom");
                  }
               }
            }
            else
            {
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append("Chain has no terminating OXT and PBR-NO-OXT isn't uniquely defined in the residue file.");
               editor->setColor(save_color);
            }
         }
      } // for j < molecule.size()
   }

   progress->setProgress(ppos++); // 5
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }

   // pass 2c hydration

   // for (unsigned int i = 0; i < model_vector.size (); i++)   {
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

            if (this_atom->active &&
                this_atom->is_bead) {
#if defined(DEBUG)
               printf("pass 2c hydration %s %s %d pm %d\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->serial,
                      this_atom->placing_method); fflush(stdout);
#endif
               this_atom->bead_ref_volume_unhydrated = this_atom->bead_ref_volume;
               this_atom->bead_ref_volume += misc.hydrovol * this_atom->bead_hydration;
               this_atom->bead_computed_radius = pow(3 * this_atom->bead_ref_volume / (4.0*M_PI), 1.0/3);
            }
         }
      }
   }

   // pass 3 determine visibility, exposed code, normalize cog position, final position determination
   // compute com of entire molecule
   progress->setProgress(ppos++); // 6
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }

#if defined(OLD_ASAB1_SC_COMPUTE)
   // pass 2d compute mc asa
   vector <float> bead_mc_asa;
# if defined(DEBUG1) || defined(DEBUG)
   printf("pass 2d\n"); fflush(stdout);
# endif
   // for (unsigned int i = 0; i < model_vector.size (); i++)   {
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

            if (this_atom->active &&
                this_atom->chain == 0) {
# if defined(DEBUG)
               printf("pass 2d mc_asa %s %s %d pm %d\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->serial,
                      this_atom->placing_method); fflush(stdout);
# endif
               if(bead_mc_asa.size() < this_atom->resSeq + 1) {
                  bead_mc_asa.resize(this_atom->resSeq + 32);
               }
#warn broken by resSeq->QString
               bead_mc_asa[this_atom->resSeq] += this_atom->asa;
            }
         }
      }
   }
#endif // OLD_ASABA_SC_COMPUTE

#if defined(DEBUG1) || defined(DEBUG)
   printf("pass 3\n"); fflush(stdout);
#endif

   // for (unsigned int i = 0; i < model_vector.size (); i++) {
   {
      unsigned int i = current_model;

      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {

#if defined(OLD_ASAB1_SC_COMPUTE)
         float mc_asab1 = 0;
         QString mc_resname = "";
         unsigned int mc_resSeq = 0;
#endif

         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {


            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p3 i j k %d %d %d %lx\n", i, j, k, this_atom->p_atom); fflush(stdout);
            this_atom->exposed_code = -1;
            if (this_atom->active &&
                this_atom->is_bead) {

#if defined(DEBUG)
               printf("pass 3 active is bead %s %s %d\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->serial); fflush(stdout);
#endif
               for (unsigned int m = 0; m < 3; m++) {
                  if (this_atom->bead_cog_mw) {
                     this_atom->bead_cog_coordinate.axis[m] /= this_atom->bead_cog_mw;
                  }
                  else 
                  {
                     this_atom->bead_cog_coordinate.axis[m] = 0;
                  }
               }

               if (this_atom->p_residue && this_atom->p_atom) {

#if defined(DEBUG)
                  printf("pass 3 active is bead %s %s %d bead assignment %d placing method %d\n",
                         this_atom->name.ascii(),
                         this_atom->resName.ascii(),
                         this_atom->serial,
                         this_atom->bead_assignment,
                         this_atom->placing_method
                         ); fflush(stdout);
#endif
                  switch (this_atom->placing_method) {

                  case 0 : // cog
                     this_atom->bead_coordinate = this_atom->bead_cog_coordinate;
                     // if (this_atom->bead_positioner) {
                     // fprintf(stderr, "warning: this bead had a atom claiming position & a bead placing method of cog! %s %s %d\n",
                     //   this_atom->name.ascii(),
                     //   this_atom->resName.ascii(),
                     //   this_atom->serial);
                     // }
                     break;
                  case 1 : // positioner
                     this_atom->bead_coordinate = this_atom->bead_position_coordinate;
                     break;
                  case 2 : // no positioning necessary
                     this_atom->bead_coordinate = this_atom->coordinate;
                     break;
                  default :
                     this_atom->bead_coordinate = this_atom->bead_cog_coordinate;
                     fprintf(stderr, "warning: unknown bead placing method %d %s %s %d <using cog!>\n",
                             this_atom->placing_method,
                             this_atom->name.ascii(),
                             this_atom->resName.ascii(),
                             this_atom->serial);
                     break;
                  }
               }
               else 
               {
                  fprintf(stderr, "serious internal error 1 on %s %s %d, quitting\n",
                          this_atom->name.ascii(),
                          this_atom->resName.ascii(),
                          this_atom->serial);
                  exit(-1);
                  break;
               }
#if defined(DEBUG) || defined(OLD_ASAB1_SC_COMPUTE)
               printf("pass 3 active is bead %s %s %s checkpoint 1\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->resSeq.ascii()); fflush(stdout);
#endif
               this_atom->visibility = (this_atom->bead_asa >= asa.hydrate_threshold);
#if defined(OLD_ASAB1_SC_COMPUTE)
               if (this_atom->chain == 1) {
                  printf("visibility was %d is ", this_atom->visibility);
                  this_atom->visibility = (this_atom->bead_asa + bead_mc_asa[this_atom->resSeq] >= asa.hydrate_threshold);
                  printf("%d\n", this_atom->visibility);
               }
#endif

#if defined(DEBUG)
               printf("pass 3 active is bead %s %s %d checkpoint 2\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->serial); fflush(stdout);
#endif
               if (!create_beads_normally ||
                   this_atom->visibility ||
                   !asa.calculation) {
                  this_atom->exposed_code = 1;  // exposed
               }
               else 
               {
                  if (this_atom->chain == 0) {
                     this_atom->exposed_code = 10;  // main chain, buried
                  }
                  if (this_atom->chain == 1) {
                     this_atom->exposed_code = 6;   // side chain, buried
                  }
               }
#if defined(DEBUG)
               printf("pass 3 active is bead %s %s %d checkpoint 3\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->serial); fflush(stdout);
#endif

#if defined(OLD_ASAB1_SC_COMPUTE)
               if(this_atom->chain == 0) {
                  mc_asab1 = this_atom->bead_asa;
                  mc_resname = this_atom->resName;
                  mc_resSeq = this_atom->resSeq;
                  printf("saving last mc asa %.2f\n", mc_asab1);
               }
#endif
            }
            else 
            {
               this_atom->placing_method = -1;
            }
         }
      }
   }

   // pass 4 print results
   progress->setProgress(ppos++); // 7
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }

#if defined(DEBUG)
   printf("model~molecule~atom~name~residue~chainID~"
          "position~active~radius~asa~mw~"
          "bead #~chain~serial~is_bead~bead_asa~visible~code/color~"
          "bead mw~position controlled?~bead positioner~baric method~bead hydration~bead color~"
          "bead ref volume~bead ref mw~bead computed radius~mw match?~"
          "position_coordinate~cog position~use position\n");
   // for (unsigned int i = 0; i < model_vector.size (); i++)
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p4 i j k %d %d %d %lx\n", i, j, k, this_atom->p_atom); fflush(stdout);

            // printf("model %d mol %d atm %d nam %s res %s xyz [%f,%f,%f] act %s rads %f asa %f bead # %d chain %d serl %d is_bead %s bead_asa %f vis %s code %d pos? %s pos_co [%f,%f,%f] cog [%f,%f,%f] use [%f, %f, %f]\n",
            printf("%d~%d~%d~%s~%s~%s~"
                   "[%f,%f,%f]~%s~%f~%f~%f~"
                   "%d~%d~%d~%s~%f~%s~"
                   "%d~%f~%s~%s~%d~%u~%u~"
                   "%f~%f~%f~%s~"
                   "[%f,%f,%f]~[%f,%f,%f]~[%f, %f, %f]\n",

                   i, j, k,
                   this_atom->name.ascii(),
                   this_atom->resName.ascii(),
                   this_atom->chainID.ascii(),

                   this_atom->coordinate.axis[0],
                   this_atom->coordinate.axis[1],
                   this_atom->coordinate.axis[2],
                   this_atom->active ? "Y" : "N",
                   this_atom->radius,
                   this_atom->asa,
                   this_atom->mw,

                   this_atom->bead_assignment,
                   this_atom->chain,
                   this_atom->serial,
                   this_atom->is_bead ? "Y" : "N",
                   this_atom->bead_asa,
                   this_atom->visibility ? "Y" : "N",

                   this_atom->exposed_code,
                   this_atom->bead_mw,
                   this_atom->bead_positioner ? "Y" : "N",
                   this_atom->active ? (this_atom->bead_positioner ? "Y" : "N") : "Inactive",
                   this_atom->placing_method,
                   this_atom->bead_hydration,
                   this_atom->bead_color,

                   this_atom->bead_ref_volume,
                   this_atom->bead_ref_mw,
                   this_atom->bead_computed_radius,
                   fabs(this_atom->bead_ref_mw - this_atom->bead_mw) < .03 ? "Y" : "N",

                   this_atom->bead_position_coordinate.axis[0],
                   this_atom->bead_position_coordinate.axis[1],
                   this_atom->bead_position_coordinate.axis[2],
                   this_atom->bead_cog_coordinate.axis[0],
                   this_atom->bead_cog_coordinate.axis[1],
                   this_atom->bead_cog_coordinate.axis[2],
                   this_atom->bead_coordinate.axis[0],
                   this_atom->bead_coordinate.axis[1],
                   this_atom->bead_coordinate.axis[2]
                   ); fflush(stdout);
         }
      }
   }
#endif

   progress->setProgress(ppos++); // 8
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }
   return 0;
}

void US_Hydrodyn::build_to_hydrate()
{
   to_hydrate.clear();
   unsigned int i = current_model;

   // pass 1 identify exposed sc's

   map < QString, bool > exposed_sc;

   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         if ( this_atom->exposed_code == 1 &&
              this_atom->chain == 1 )
         {
            exposed_sc[ QString( "%1~%2~%3" )
                        .arg( this_atom->resName )
                        .arg( this_atom->resSeq )
                        .arg( this_atom->chainID ) ] = true;
         }
      }
   }

   // pass 2 add side chain to to_hydrate map
   QRegExp rx_main_chain("^(N|O)$");

   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {

         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

         QString mapkey =
            QString( "%1~%2~%3" )
            .arg( this_atom->resName )
            .arg( this_atom->resSeq )
            .arg( this_atom->chainID );

         if ( rx_main_chain.search( this_atom->resName ) == -1 
              && exposed_sc.count( mapkey )
              && rotamers.count( this_atom->resName )
              )
         {
            to_hydrate[ mapkey ][ this_atom->name ] = this_atom->coordinate;

            // cout << QString("adding %1 %2 %3 %4\n")
            // .arg(this_atom->resName)
            // .arg(this_atom->resSeq)
            // .arg(this_atom->chainID)
            // .arg(this_atom->name);
         }

      }
   }
}

bool US_Hydrodyn::compute_to_hydrate_dihedrals( QString &error_msg )
{
   puts("computing to hydrate dihedrals");
   to_hydrate_dihedrals.clear();

   vector < point > p(4);
   float dihedral;

   QRegExp rx_expand_mapkey("^(.+)~(.+)~(.*)$");

   for (  map < QString, map < QString, point > >::iterator it = to_hydrate.begin();
          it != to_hydrate.end();
          it++ )
   {
      if ( rx_expand_mapkey.search( it->first ) == -1 )
      {
         error_msg = QString( tr( "internal error: could not expand mapkey %1" ) ).arg( it->first );
         return false;
      }
      QString resName = rx_expand_mapkey.cap( 1 );
      // check dihedrals for this residue
      if ( !dihedral_atoms.count( resName ) )
      {
         error_msg = QString( tr( "No dihedral group found for residue %1." ) ).arg( resName );
         return false;
      }
#if defined( DEBUG_TO_HYDRATE_DIHEDRAL )
      cout << 
         QString("computation for to hydrate dihedral %1 %2\n")
         .arg( it->first )
         .arg( resName );
#endif
      for ( unsigned int i = 0; i < dihedral_atoms[ resName ].size(); i++ )
      {
         if ( dihedral_atoms[ resName ][ i ].size() != 4 )
         {
            error_msg = 
               QString( "Dihedral table size incorrect, should be 4 but is %1 for %2th dihedral group of %3" )
               .arg( dihedral_atoms[ resName ][ i ].size() )
               .arg( i + 1 )
               .arg( resName );
            return false;
         }

         for ( unsigned int j = 0; j < dihedral_atoms[ resName ][ i ].size(); j++ )
         {
            if ( !( it->second.count( dihedral_atoms[ resName ][ i ][ j ] ) ) )
            {
               error_msg = 
                  QString( "Side chain %1 is missing required dihedral atom %2" )
                  .arg( it->first )
                  .arg( dihedral_atoms[ it->first ][ i ][ j ] );
               return false;
            }
            p[ j ] = it->second[ dihedral_atoms[ resName ][ i ][ j ] ];
         }
         dihedral = acos( dot( plane( p[ 0 ], p[ 1 ], p[ 2 ] ),
                               plane( p[ 1 ], p[ 2 ], p[ 3 ] ) ) );
         to_hydrate_dihedrals[ it->first ].push_back( dihedral );
      }
   }
   puts("done computing to hydrate dihedrals");
   return true;
}

QString US_Hydrodyn::list_to_hydrate_dihedrals()
{
   QString out = "";
   for ( map < QString, vector < float > >::iterator it = to_hydrate_dihedrals.begin();
         it != to_hydrate_dihedrals.end();
         it++ )
   {
      out += QString( "Dihedrals for: %1 " ).arg( it->first );
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         out += QString( " %1" ).arg( it->second[ i ] );
      }
      out += "\n";
   }
   return out;
}

QString US_Hydrodyn::list_to_hydrate( bool coords )
{
   QString out = "";
   for ( map < QString, map < QString, point > >::iterator it = to_hydrate.begin();
         it != to_hydrate.end();
         it++ )
   {
      out += QString( "Side chains for: %1\n" ).arg( it->first );
      for ( map < QString, point >::iterator it2 = it->second.begin();
            it2 != it->second.end();
            it2++ )
      {
         out += QString(" %1").arg( it2->first );
         if ( coords )
         {
            out += QString( " [%1,%2,%3]" )
               .arg( it2->second.axis[ 0 ] )
               .arg( it2->second.axis[ 1 ] )
               .arg( it2->second.axis[ 2 ] );
         }
      }
      out += "\n";
   }
   return out;
}         

void US_Hydrodyn::list_exposed()
{
   unsigned int i = current_model;
   cout << "exposed side chain atom list:\n";
   map < QString, bool > exposed_sc;

   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         if ( this_atom->exposed_code == 1 &&
              this_atom->chain == 1 )
         {
            cout << 
               QString("%1 %2 %3 %4 %5\n")
               .arg(this_atom->serial)
               .arg(this_atom->name)
               .arg(this_atom->resName)
               .arg(this_atom->chainID)
               .arg(this_atom->resSeq);
            exposed_sc[ QString( "%1~%2~%3" )
                        .arg( this_atom->resName )
                        .arg( this_atom->resSeq )
                        .arg( this_atom->chainID ) ] = true;
         }
      }
   }

   for ( map < QString,bool >::iterator it = exposed_sc.begin();
         it != exposed_sc.end();
         it++ )
   {
      cout << it->first << endl;
   }
}

void US_Hydrodyn::view_exposed()
{
   unsigned int i = current_model;
   // cout << "exposed side chain atom list:\n";
   map < QString, bool > exposed_sc;
   map < QString, QString > use_color;
   
   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         if ( this_atom->exposed_code == 1 &&
              this_atom->chain == 1 )
         {

            QString mapkey = 
               QString( "%1%2%3" )
               .arg( this_atom->resSeq )
               .arg( this_atom->chainID != "" ? ":" : "" )
               .arg( this_atom->chainID );

            exposed_sc[ mapkey ] = true;
            use_color[ mapkey ] = rotamers.count( this_atom->resName ) ? "green" : "yellow";
         }
      }
   }

   // rasmol view it
   QString out = QString("load %1\nselect all\ncolor gray\n").arg(pdb_file);

   for ( map < QString,bool >::iterator it = exposed_sc.begin();
         it != exposed_sc.end();
         it++ )
   {
      // cout << it->first << endl;
      out += QString("select %1\ncolour %2\n")
         .arg( it->first )
         .arg( use_color[ it->first ] );
   }
   out += "select all\n";

   QString fname = 
      somo_dir + SLASH + "tmp" + SLASH + QFileInfo( pdb_file ).baseName() + ".spt";
   QFile f(fname);
   if ( !f.open( IO_WriteOnly ) )
   {
      editor->append("Error creating file " + fname + "\n");
      return;
   }
   QTextStream t( &f );
   t << out;
   f.close();
   QStringList argument;
#if !defined(WIN32)
   argument.append("xterm");
   argument.append("-e");
#endif
#if defined(BIN64)
   argument.append(USglobal->config_list.system_dir + "/bin64/rasmol");
#else
   argument.append(USglobal->config_list.system_dir + "/bin/rasmol");
#endif
   argument.append("-script");
   argument.append(fname);
   rasmol = new QProcess(this);
   rasmol->setWorkingDirectory( somo_dir + SLASH + "tmp" );
   rasmol->setArguments(argument);
   if (!rasmol->start())
   {
      QMessageBox::message(tr("Please note:"), tr("There was a problem starting RASMOL\n"
                                                  "Please check to make sure RASMOL is properly installed..."));
      return;
   }
}

bool US_Hydrodyn::load_rotamer( QString &error_msg )
{
   puts("load_rotamer");
   if ( !rotamer_changed )
   {
      return true;
   }

   editor->append( tr("Reading hydrated rotamer file\n") );
   qApp->processEvents();
   rotamers.clear();
   dihedral_atoms.clear();

   QFile f( saxs_options.default_rotamer_filename );
   if ( !f.exists() )
   {
      error_msg = tr( "Hydrated Rotamer file not found:\n"
                      "The Hydrated Rotamer file can be set in SOMO->SAXS/SANS Options" );
      return false;
   }
   if ( !f.open( IO_ReadOnly ) )
   {
      error_msg = tr( "Hydrated Rotamer file is not readable:\n"
                      "Check file permissions" );
      return false;
   }      

   QStringList qsl;
   QTextStream ts( &f );
   
   while ( !ts.atEnd() )
   {
      qsl << ts.readLine();
   }

   f.close();

   QRegExp rx_whitespace("\\s+");
   QRegExp rx_skip("^(#|\\s+$)");
   QRegExp rx_main_chain("^(N|O)$");
   QRegExp rx_atom("^ATOM");

   bool in_rotamer = false;
   bool in_rotamer_waters = false;

   rotamer tmp_rotamer;

   for ( unsigned int i = 0; i < qsl.size(); i++ )
   {
      if ( rx_skip.search( qsl[ i ] ) != -1 )
      {
         continue;
      }
      if ( qsl[ i ].length() > 30 && qsl[ i ].contains( rx_atom ) )
      {
         qsl[ i ].at( 22 ) = ' ';
      }
      QStringList qsl_line = QStringList::split( rx_whitespace, qsl[ i ] );
      if ( qsl_line[ 0 ] == "dihedral:" )
      {
         qsl_line.pop_front();
         QString res = qsl_line[ 0 ];
         qsl_line.pop_front();
         vector < QString > four_atoms;
         for ( unsigned int j = 0; j < qsl_line.size(); j++ )
         {
            four_atoms.push_back( qsl_line[ j ] );
            if ( four_atoms.size() == 4 )
            {
               dihedral_atoms[ res ].push_back( four_atoms );
               four_atoms.clear();
            }
         }
         if ( four_atoms.size() )
         {
            error_msg = 
               QString( tr( "Error in hydrated rotamer file line %1.  Invalid number of tokens %2 on dihedral line" ) )
               .arg( i + 1 )
               .arg( qsl_line.size() + 2 );
            return false;
         }
         continue;
      }  
               
      if ( !in_rotamer )
      {
         if ( qsl_line.size() != 2 )
         {
            error_msg = 
               QString( tr( "Error in hydrated rotamer file line %1.  Invalid number of tokens %2" ) )
               .arg( i + 1 )
               .arg( qsl_line.size() );
            return false;
         }
         
         if ( qsl_line[ 0 ] != "start-rotamer:" )
         {
            error_msg = 
               QString( tr( "Error in hydrated rotamer file line %1.  Unrecognized token %2" ) )
               .arg( i + 1 )
               .arg( qsl_line[ 0 ] );
            return false;
         }
         tmp_rotamer.name = qsl_line[ 1 ];
         tmp_rotamer.residue = tmp_rotamer.name.left( 3 );
         tmp_rotamer.extension = tmp_rotamer.name.right( 7 );
         tmp_rotamer.side_chain.clear();
         tmp_rotamer.waters.clear();
         tmp_rotamer.atom_map.clear();
         in_rotamer = true;
         in_rotamer_waters = false;
         continue;
      }
      if ( qsl_line.size() == 1 )
      {
         if ( qsl_line[ 0 ] == "end-rotamer" )
         {
            // push back rotamer info here
            if ( !tmp_rotamer.side_chain.size() ||
                 !tmp_rotamer.waters.size() )
            {
               error_msg = 
                  QString( tr( "Error in hydrated rotamer file line %1.  Empty rotamer" ) )
                  .arg( i + 1 );
               return false;
            }
            rotamers[ tmp_rotamer.residue ].push_back( tmp_rotamer );
            in_rotamer = false;
            in_rotamer_waters = false;
            continue;
         }
         if ( qsl_line[ 0 ] != "TER" )
         {
            error_msg = 
               QString( tr( "Error in hydrated rotamer file line %1.  Unrecognized token %2 in this context" ) )
               .arg( i + 1 )
               .arg( qsl_line[ 0 ] );
            return false;
         }
         in_rotamer_waters = true;
         continue;
      }
      if ( qsl_line.size() != 12 )
      {
         error_msg = 
            QString( tr ( "Error in hydrated rotamer file line %1:\n"
                          "%2\n"
                          "Unexpected number of tokens %3\n" ) )
            .arg( i + 1 )
            .arg( qsl[ i ] )
            .arg( qsl_line.size() );
         return false;
      }
      if ( qsl_line[ 0 ] != "ATOM" )
      {
         error_msg = 
            QString( tr( "Error in hydrated rotamer file line %1.  Unrecognized token %2" ) )
            .arg( i + 1 )
            .arg( qsl_line[ 0 ] );
         return false;
      }

      if ( qsl[ i ].at( 13 ) == 'H' )
      {
         // skip hydrogens for now
         continue;
      }
      
      if ( !in_rotamer_waters &&
           rx_main_chain.search( qsl_line[ 2 ] ) != -1 )
      {
         // skip main chain 
         continue;
      }
         
      if ( qsl_line[ 3 ] == "GLY" )
      {
         // skip GLY to support old romater file format 
         continue;
      }
      rotamer_atom ra;
      ra.name = qsl_line[ 2 ];
      ra.coordinate.axis[ 0 ] = qsl_line[ 6 ].toFloat();
      ra.coordinate.axis[ 1 ] = qsl_line[ 7 ].toFloat();
      ra.coordinate.axis[ 2 ] = qsl_line[ 8 ].toFloat();
            
      if ( in_rotamer_waters )
      {
         tmp_rotamer.waters.push_back( ra );
      } else {
         tmp_rotamer.atom_map[ ra.name ] = ra;
         tmp_rotamer.side_chain.push_back( ra );
      }
   }

   if ( in_rotamer )
   {
      error_msg = 
         tr( "Error in hydrated rotamer file at end, no final end-rotamer." );
      return false;
   }

   editor->append( tr("Done reading hydrated rotamer file\n") );
   qApp->processEvents();
   rotamer_changed = false;
   puts("load_rotamer done");
   if ( !compute_rotamer_dihedrals( error_msg ) )
   {
      return false;
   }
   // cout << list_rotamer_dihedrals();
   if ( !compute_water_positioning_atoms( error_msg ) )
   {
      return false;
   }
   // cout << list_water_positioning_atoms();
   return true;
}

bool US_Hydrodyn::compute_rotamer_dihedrals( QString &error_msg )
{
   editor->append( tr("Calculating dihedrals of rotamers\n") );
   qApp->processEvents();
   puts("Calculating dihedrals of rotamers");

   // a dihedral computation takes 4 points
   vector < point > p(4);
   float dihedral;

   // for each residue
   for (  map < QString, vector < rotamer > >::iterator it = rotamers.begin();
          it != rotamers.end();
          it++ )
   {
      // for each rotamer for the residue
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
#if defined( DEBUG_DIHEDRAL )
         cout << 
            QString("computation for dihedral %1 %2:\n")
            .arg( it->first )
            .arg( it->second[ i ].extension );
#endif
         it->second[ i ].dihedral_angles.clear();
         // process each dihedral chain, giving 4 points
         for ( unsigned int j = 0; j < dihedral_atoms[ it->first ].size(); j++ )
         {
#if defined( DEBUG_DIHEDRAL )
            cout << QString("  dihedral chain pos %1:\n").arg( j );
#endif
            if ( dihedral_atoms[ it->first ][ j ].size() != 4 )
            {
               error_msg = 
                  QString( "Dihedral table size incorrect, should be 4 but is %1 for %2th dihedral group of %3" )
                  .arg( dihedral_atoms[ it->first ][ j ].size() )
                  .arg( j + 1 )
                  .arg( it->first );
               return false;
            }
            for ( unsigned int k = 0; k < dihedral_atoms[ it->first ][ j ].size(); k++ )
            {
#if defined( DEBUG_DIHEDRAL )
               cout << QString("  %1: ").arg( dihedral_atoms[ it->first ][ j ][ k ] );
#endif
               if ( !( it->second[ i ].atom_map.count( dihedral_atoms[ it->first ][ j ][ k ] ) ) )
               {
                  error_msg = 
                     QString( "Rotamer for %1 is missing dihedral atom %2" )
                     .arg( it->first )
                     .arg( dihedral_atoms[ it->first ][ j ][ k ] );
                  return false;
               }
               p[ k ] = it->second[ i ].atom_map[ dihedral_atoms[ it->first ][ j ][ k ] ].coordinate;
#if defined( DEBUG_DIHEDRAL )
               cout << p[ k ];
#endif
            }
#if defined( DEBUG_DIHEDRAL )
            cout << "\n"
                    "   plane1:  " << plane( p[ 0 ], p[ 1 ], p[ 2 ] ) << endl;
            cout << "   minus32: " << minus( p[ 2 ], p [ 1 ] ) << endl;
            cout << "   minus12: " << minus( p[ 0 ], p [ 1 ] ) << endl;
            cout << "   cross:   " << cross( minus( p[ 2 ], p [ 1 ] ), minus( p[ 0 ], p [ 1 ] ) ) << endl;
            cout << "   plane2: " << plane( p[ 1 ], p[ 2 ], p[ 3 ] ) << endl;
            cout << "   dot:    " << dot( plane( p[ 0 ], p[ 1 ], p[ 2 ] ),
                                          plane( p[ 1 ], p[ 2 ], p[ 3 ] ) ) << endl;
            cout << "   acos:   " << acos( dot( plane( p[ 0 ], p[ 1 ], p[ 2 ] ),
                                                plane( p[ 1 ], p[ 2 ], p[ 3 ] ) ) ) << endl;
#endif

            dihedral = acos( dot( plane( p[ 0 ], p[ 1 ], p[ 2 ] ),
                                  plane( p[ 1 ], p[ 2 ], p[ 3 ] ) ) );
            it->second[ i ].dihedral_angles.push_back( dihedral );
         }
      }
   }
   editor->append( tr("Done calculating dihedrals of rotamers\n") );
   qApp->processEvents();
   puts("Done calculating dihedrals of rotamers");
   return true;
}

QString US_Hydrodyn::list_rotamer_dihedrals()
{
   QString out;

   out = "rotamer dihedrals:\n";

   for (  map < QString, vector < rotamer > >::iterator it = rotamers.begin();
          it != rotamers.end();
          it++ )
   {
      out += QString( "Rotamers for residue: %1\n" ).arg( it->first );
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         out += QString(" %1: sc").arg( it->second[ i ].extension );
         for ( unsigned int j = 0; j < it->second[ i ].dihedral_angles.size(); j++ )
         {
            out += QString(" %1").arg( it->second[ i ].dihedral_angles[ j ] );
         }
         out += "\n";
      }
      out += "\n";
   }
   return out;
}


QString US_Hydrodyn::list_rotamers( bool coords )
{
   QString out;

   out = "dihedral atoms:\n";
   for (  map < QString, vector < vector < QString > > >::iterator it = dihedral_atoms.begin();
          it != dihedral_atoms.end();
          it++ )
   {
      out += QString(" %1:\n").arg( it->first );
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         for ( unsigned int j = 0; j < it->second[i].size(); j++ )
         {
            out += QString(" %1").arg( it->second[i][j] );
         }
         out += " ; ";
      }
      out += "\n";
   }
   out += "\n";

   for (  map < QString, vector < rotamer > >::iterator it = rotamers.begin();
          it != rotamers.end();
          it++ )
   {
      out += QString( "Rotamers for residue: %1\n" ).arg( it->first );
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         out += QString(" %1: sc").arg( it->second[ i ].extension );
         for ( unsigned int j = 0; j < it->second[ i ].side_chain.size(); j++ )
         {
            out += QString(" %1").arg( it->second[ i ].side_chain[ j ].name );
            if ( coords )
            {
               out += QString( " [%1,%2,%3]" )
                  .arg( it->second[ i ].side_chain[ j ].coordinate.axis[ 0 ] )
                  .arg( it->second[ i ].side_chain[ j ].coordinate.axis[ 1 ] )
                  .arg( it->second[ i ].side_chain[ j ].coordinate.axis[ 2 ] );
            }
         }
         out += " w";
         for ( unsigned int j = 0; j < it->second[ i ].waters.size(); j++ )
         {
            out += QString(" %1").arg( it->second[ i ].waters[ j ].name );
            if ( coords )
            {
               out += QString( " [%1,%2,%3]" )
                  .arg( it->second[ i ].side_chain[ j ].coordinate.axis[ 0 ] )
                  .arg( it->second[ i ].side_chain[ j ].coordinate.axis[ 1 ] )
                  .arg( it->second[ i ].side_chain[ j ].coordinate.axis[ 2 ] );
            }
         }
         out += "\n";
      }
   }
   return out;
}

bool US_Hydrodyn::compute_best_fit_rotamer( QString &error_msg )
{
   best_fit_rotamer.clear();

   QRegExp rx_expand_mapkey("^(.+)~(.+)~(.*)$");

   // go through the computed dihedrals, compute sum of abs differences, choose best one
   for ( map < QString, vector < float > >::iterator it = to_hydrate_dihedrals.begin();
         it != to_hydrate_dihedrals.end();
         it++ )
   {
      if ( rx_expand_mapkey.search( it->first ) == -1 )
      {
         error_msg = QString( tr( "internal error: could not expand mapkey %1" ) ).arg( it->first );
         return false;
      }
      QString resName = rx_expand_mapkey.cap( 1 );
      // check dihedrals for this residue
      if ( !rotamers.count( resName ) )
      {
         error_msg = QString( tr( "No rotamer group found for residue %1." ) ).arg( resName );
         return false;
      }
      
      float        best_fitness;
      unsigned int best_fitness_pos;

      for ( unsigned int i = 0; i < rotamers[ resName ].size(); i++ )
      {
         if ( it->second.size() != rotamers[ resName ][ i ].dihedral_angles.size() )
         {
            error_msg = 
               QString( tr( "Number of dihedrals do not match between side chain %1 (%2) and rotamer %3 (%4)" ) )
               .arg( it->first )
               .arg( it->second.size() )
               .arg( rotamers[ resName ][ i ].name )
               .arg( rotamers[ resName ][ i ].dihedral_angles.size() );
            return false;
         }
         float fitness = 0.0;
         for ( unsigned int j = 0; j < it->second.size(); j++ )
         {
            // the dihedral angles are computed by acos, and range of acos is [0,pi], 
            // so I think we are ok
            fitness += fabs( it->second[ j ] - rotamers[ resName ][ i ].dihedral_angles[ j ] );
         }
         if ( !i || best_fitness > fitness )
         {
            best_fitness = fitness;
            best_fitness_pos = i;
         }
      }
      
      best_fit_rotamer[ it->first ] = rotamers[ resName ][ best_fitness_pos ];
   }
   return true;
}

QString US_Hydrodyn::list_best_fit_rotamer()
{
   QString out;

   out = "Best fit rotamers:\n";

   for ( map < QString, rotamer >::iterator it = best_fit_rotamer.begin();
         it != best_fit_rotamer.end();
         it++ )
   {
      out += 
         QString( "%1 %2\n" )
         .arg( it->first )
         .arg( it->second.name );
   }
   return out;
}

class sortable_float {
public:
   float             f;
   unsigned int      index;
   bool operator < (const sortable_float& objIn) const
   {
      return ( f < objIn.f );
   }
};

bool US_Hydrodyn::compute_water_positioning_atoms( QString & /* error_msg */ )
{
   editor->append( tr("Calculating water positioning atoms for each rotamer\n") );
   qApp->processEvents();
   puts("Calculating water positioning atoms for each rotamer");

   list < sortable_float > lsf;
   sortable_float          sf;
   vector < QString >      wpa;

   for ( map < QString, vector < rotamer > >::iterator it = rotamers.begin();
         it != rotamers.end();
         it++ )
   {
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         it->second[ i ].water_positioning_atoms.clear();

         // for each water

         for ( unsigned int j = 0; j < it->second[ i ].waters.size(); j++ )
         {
            lsf.clear();
            wpa.clear();
            
            // find the distance to each side chain atom
            for ( unsigned int k = 0; k < it->second[ i ].side_chain.size(); k++ )
            {
               sf.f = dist( it->second[ i ].waters[ j ].coordinate, it->second[ i ].side_chain[ k ].coordinate );
               sf.index = k;
               lsf.push_back( sf );
            }

            lsf.sort();
            
            unsigned int pushed = 0;
            for ( list < sortable_float >::iterator it2 = lsf.begin();
                  it2 != lsf.end();
                  it2++ )
            {
               wpa.push_back( it->second[ i ].side_chain[ it2->index ].name );
               pushed++;
               if ( pushed >= MAX_WATER_POSITIONING_ATOMS )
               {
                  break;
               }
            }
            it->second[ i ].water_positioning_atoms.push_back( wpa );
         }
      }
   }
   editor->append( tr("Done calculating water positioning atoms for each rotamer\n") );
   qApp->processEvents();
   puts("Done calculating water positioning atoms for each rotamer");
   return true;
}

QString US_Hydrodyn::list_water_positioning_atoms()
{
   QString out = "Water positioning atoms:\n";

   for (  map < QString, vector < rotamer > >::iterator it = rotamers.begin();
          it != rotamers.end();
          it++ )
   {
      out += QString( "  Rotamer for residue: %1\n" ).arg( it->first );
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         out += QString("  %1:\n").arg( it->second[ i ].extension );
         for ( unsigned int j = 0; j < it->second[ i ].water_positioning_atoms.size(); j++ )
         {
            out += QString( "    water %1 :" ).arg( j );
            for ( unsigned int k = 0; k < it->second[ i ].water_positioning_atoms[ j ].size(); k++ )
            {
               out += QString(" %1").arg( it->second[ i ].water_positioning_atoms[ j ][ k ] );
            }
            out += "\n";
         }
      }
      out += "\n";
   }
   return out;
}

bool US_Hydrodyn::compute_waters_to_add( QString &error_msg )
{
   editor->append( tr("Transforming waters to add to pdb coordinates\n") );
   qApp->processEvents();
   puts("Transforming waters to add to pdb coordinates");

   unsigned int count_waters           = 0;
   unsigned int count_waters_added     = 0;
   unsigned int count_waters_not_added = 0;

   vector < point > p1;
   vector < point > p2;
   waters_to_add.clear();

   for ( map < QString, rotamer >::iterator it = best_fit_rotamer.begin();
         it != best_fit_rotamer.end();
         it++ )
   {
      if ( !to_hydrate.count( it->first ) )
      {
         error_msg = QString( tr( "Internal error: best_fit_rotamer key %1 not found in to_hydrate" ) )
            .arg( it->first );
         return false;
      }

      // for each water to add:
      // cout << QString( "need to compute best transform matrix for %1:\n" ).arg( it->first );
      if ( it->second.water_positioning_atoms.size() !=
           it->second.waters.size() )
      {
         error_msg = QString( tr( "Internal error: water positioning atom size %1 does not match waters size %2" ) )
            .arg( it->second.water_positioning_atoms.size() )
            .arg( it->second.waters.size() );
         return false;
      }

      for ( unsigned int i = 0; i < it->second.water_positioning_atoms.size(); i++ )
      {
         // get coordinates of 
         p1.resize( it->second.water_positioning_atoms[ i ].size() );
         p2.resize( it->second.water_positioning_atoms[ i ].size() );

            
         for ( unsigned int j = 0; j < it->second.water_positioning_atoms[ i ].size(); j++ )
         {
            if ( !it->second.atom_map.count( it->second.water_positioning_atoms[ i ][ j ] ) )
            {
               error_msg = QString( tr( "Internal error: water positioning atom %1 not found in atom_map" ) )
                  .arg( it->second.water_positioning_atoms[ i ][ j ] );
               return false;
            }
            p1[ j ] = it->second.atom_map[ it->second.water_positioning_atoms[ i ][ j ] ].coordinate;

            if ( !to_hydrate[ it->first ].count( it->second.water_positioning_atoms[ i ][ j ] ) )
            {
               error_msg = QString( tr( "Internal error: water positioning atom %1 not found in to_hydrate atoms" ) )
                  .arg( it->second.water_positioning_atoms[ i ][ j ] );
               return false;
            }
            p2[ j ] = to_hydrate[ it->first ][ it->second.water_positioning_atoms[ i ][ j ] ];

            // cout << QString( "  %1 [%2,%3,%4] to [%5,%6,%7]\n" )
            // .arg( it->second.water_positioning_atoms[ i ][ j ] )
            // .arg( p1[ j ].axis[ 0 ] )
            // .arg( p1[ j ].axis[ 1 ] )
            // .arg( p1[ j ].axis[ 2 ] )
            // .arg( p2[ j ].axis[ 0 ] )
            // .arg( p2[ j ].axis[ 1 ] )
            // .arg( p2[ j ].axis[ 2 ] );
         }
         vector < point > rotamer_waters;
         rotamer_waters.push_back( it->second.waters[ i ].coordinate );
         vector < point > new_waters;
         // cout << QString( " and apply it to the point [%1,%2,%3]\n")
         // .arg( it->second.waters[ i ].coordinate.axis[ 0 ] )
         // .arg( it->second.waters[ i ].coordinate.axis[ 1 ] )
         // .arg( it->second.waters[ i ].coordinate.axis[ 2 ] );
         if ( !atom_align( p1, p2, rotamer_waters, new_waters, error_msg ) )
         {
            return false;
         }
         count_waters++;
         if ( !has_steric_clash( new_waters[ 0 ] ) )
         {
            count_waters_added++;
            waters_to_add[ it->first ].push_back( new_waters[ 0 ] );
         } else {
            count_waters_not_added++;
         }
      }
   }
   editor->append( tr("Done transforming waters to add to pdb coordinates\n") );
   editor->append( QString( tr("%1 waters added. %2 not added due to steric clashes \n") )
                   .arg( count_waters_added ) 
                   .arg( count_waters_not_added ) 
                   );
   qApp->processEvents();
   puts("Done transforming waters to add to pdb coordinates");
   return true;
}

QString US_Hydrodyn::list_waters_to_add()
{
   QString out;

   out = "Waters to add:\n";
   for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
         it != waters_to_add.end();
         it++ )
   {
      out += QString( "%1: " ).arg( it->first );
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         out += QString( " [%1,%2,%3]" )
            .arg( it->second[ i ].axis[ 0 ] )
            .arg( it->second[ i ].axis[ 1 ] )
            .arg( it->second[ i ].axis[ 2 ] )
            ;
      }
      out += "\n";
   }

   return out;
}

bool US_Hydrodyn::has_steric_clash( point p )
{
   unsigned int i = current_model;

   // check structure:
   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         if ( dist( this_atom->coordinate, p ) <= saxs_options.steric_clash_distance )
         {
            return true;
         }
      }
   }

   // check already added waters:
   for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
         it != waters_to_add.end();
         it++ )
   {
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         if ( dist( it->second[ i ], p ) <= saxs_options.steric_clash_distance )
         {
            return true;
         }
      }
   }
   return false;
}

bool US_Hydrodyn::write_pdb_with_waters( QString &error_msg )
{
   QString fname = pdb_file;
   fname = fname.replace( QRegExp( "(|-(h|H))\\.(pdb|PDB)$" ), "" ) 
      + QString( "_%1-h.pdb" ).arg( current_model + 1 );
   if ( !overwrite && QFile::exists( fname ) )
   {
      fname = fileNameCheck( fname );
   }

   QFile f( fname );
   if ( !f.open( IO_WriteOnly ) )
   {
      error_msg = QString( tr("can not open file %1 for writing" ) ).arg( fname );
      return false;
   }

   QTextStream ts( &f );

   ts << 
      QString( "HEADER  US-SOMO Hydrated pdb file %1\n" ).arg( fname );
   ts <<
      QString( "MODEL %1\n" ).arg( current_model + 1 );

   unsigned int i = current_model;

   unsigned int atom_number = 0;
   unsigned int residue_number = 0;
   map < QString, bool > chains_used;

   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

         ts << 
            QString("")
            .sprintf(     
                     "ATOM   %4d%5s%4s %1s%4d    %8.3f%8.3f%8.3f  1.00 10.00               \n",
                     this_atom->serial,
                     this_atom->orgName.ascii(),
                     this_atom->resName.ascii(),
                     this_atom->chainID.ascii(),
                     this_atom->resSeq.toUInt(),
                     this_atom->coordinate.axis[ 0 ],
                     this_atom->coordinate.axis[ 1 ],
                     this_atom->coordinate.axis[ 2 ]
                     );
         chains_used[ this_atom->chainID ]++;

         if ( atom_number < this_atom->serial )
         {
            atom_number = this_atom->serial;
         }
         if ( residue_number < this_atom->resSeq.toUInt() )
         {
            residue_number = this_atom->resSeq.toUInt();
         }
      }
   }
   ts << "TER\n";

   QString chainID = "W";
   if ( chains_used.count( chainID ) )
   {
      for ( int i = 9; i >= 0; i-- )
      {
         chainID = QString( "%1" ).arg( i );
         if ( !chains_used.count( chainID ) )
         {
            break;
         }
      }
      if ( chains_used.count( chainID ) )
      {
         chainID = "Z";
      }
      if ( chains_used.count( chainID ) )
      {
         chainID = "Y";
      }
      if ( chains_used.count( chainID ) )
      {
         chainID = "X";
      }
      if ( chains_used.count( chainID ) )
      {
         chainID = "w";
      }
      if ( chains_used.count( chainID ) )
      {
         chainID = "W";
      }
   }
      
   // check already added waters:
   for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
         it != waters_to_add.end();
         it++ )
   {
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         ts << 
            QString("")
            .sprintf(     
                     "ATOM  %5d  OW  SWH %1s%4d    %8.3f%8.3f%8.3f  1.00 10.00           O  \n",
                     ++atom_number,
                     chainID.ascii(),
                     ++residue_number,
                     it->second[ i ].axis[ 0 ],
                     it->second[ i ].axis[ 1 ],
                     it->second[ i ].axis[ 2 ]
                     );

      }
   }
   ts << "TER\nENDMDL\nEND\n";
   f.close();
   last_hydrated_pdb_name = fname;
   editor->append( QString( tr( "File %1 created\n" ) ).arg( fname ) );
   return true;
}

bool US_Hydrodyn::selected_models_contain_SWH()
{
   for ( unsigned int i = 0; i < (unsigned int)lb_model->numRows(); i++) {
      if ( lb_model->isSelected( i ) ) {
         for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
            for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
               PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
               if ( this_atom->resName == "SWH" )
               {
                  return true;
               }
            }
         }
      }
   }
   return false;
}
