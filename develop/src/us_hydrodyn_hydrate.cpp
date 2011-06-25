#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_asab1.h"
#include "../include/us_surfracer.h"

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

int US_Hydrodyn::pdb_hydrate_for_saxs()
{
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
                           tr("You must select exactly one model to perform SAXS functions."));
      return -1;
   } 

   puts("ph1");
   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_somo->setEnabled(false);

   bd_anaflex_enables(false);

   pb_grid_pdb->setEnabled(false);
   pb_grid->setEnabled(false);
   options_log = "";
   display_default_differences();
   model_vector = model_vector_as_loaded;
   puts("ph1 a");

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

   puts("ph1 b");
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

   puts("ph2");
   for (current_model = 0; current_model < (unsigned int)lb_model->numRows(); current_model++)
   {
      if (!any_errors && lb_model->isSelected(current_model))
      {
         any_models = true;
         if(!pdb_asa_for_saxs_hydrate())
         {
            list_exposed();
            view_exposed();
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

      // calculate bead model and generate hydrodynamics calculation output
      // if successful, enable follow-on buttons:
   }
   puts("ph3");
   if (any_models && !any_errors)
   {
      qApp->processEvents();
   }
   else
   {
      editor->append("Errors encountered\n");
   }
   puts("ph4");

   pb_somo->setEnabled(true);
   pb_grid_pdb->setEnabled(true);
   pb_grid->setEnabled(true);
   pb_stop_calc->setEnabled(false);
   return 0;
}

int US_Hydrodyn::pdb_asa_for_saxs_hydrate()
{
   puts("pa1");
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
   int mppos = 18 + (asa.recheck_beads ? 1 : 0);
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
   puts("pa2");
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

   puts("pa3");
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
      int retval = surfracer_main(asa.probe_radius,
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
   puts("pa4");

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
               this_atom->visibility = (this_atom->bead_asa >= asa.threshold);
#if defined(OLD_ASAB1_SC_COMPUTE)
               if (this_atom->chain == 1) {
                  printf("visibility was %d is ", this_atom->visibility);
                  this_atom->visibility = (this_atom->bead_asa + bead_mc_asa[this_atom->resSeq] >= asa.threshold);
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
   // build vector of beads
   bead_model.clear();
   // #define DEBUG_MOD
#if defined(DEBUG_MOD)
   vector<PDB_atom> dbg_model;
#endif
   //  for (unsigned int i = 0; i < model_vector.size (); i++)
#if defined(DEBUG_MW)
   double tot_atom_mw = 0e0;
   double tot_bead_mw = 0e0;
   double prev_bead_mw = 0e0;
   double prev_atom_mw = 0e0;
   int bead_count = 0;
   int atom_count = 0;
   int last_asgn = -1;
   QString last_res = "";
   QString last_resName = "";
#endif
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
#if defined(DEBUG_MOD)
            this_atom->bead_number = 0;
            dbg_model.push_back(*this_atom);
#endif
            if(this_atom->active &&
               this_atom->is_bead) {
               this_atom->bead_number = bead_model.size();
               this_atom->bead_actual_radius = this_atom->bead_computed_radius;
#if defined(DEBUG_MW)
               if(bead_count) {
                  printf("res %s %s bead %d bead_mw %f sum atom mw %f diff %f\n",
                         last_res.ascii(),
                         last_resName.ascii(),
                         last_asgn,
                         prev_bead_mw,
                         prev_atom_mw,
                         prev_bead_mw -
                         prev_atom_mw
                         );
               }
               prev_bead_mw = 0;
               prev_atom_mw = 0;
               bead_count++;
               tot_bead_mw += this_atom->bead_ref_mw;
               prev_bead_mw += this_atom->bead_ref_mw;
               last_asgn = (int)this_atom->bead_assignment;
               last_res = this_atom->resSeq;
               last_resName = this_atom->resName;
#endif
               bead_model.push_back(*this_atom);
            }
#if defined(DEBUG_MW)
            if(this_atom->active) {
               atom_count++;
               tot_atom_mw += this_atom->mw;
               prev_atom_mw += this_atom->mw;
            }
#endif
         }
      }
   }
#if defined(DEBUG_MW)
   printf("res %d %s bead %d bead_mw %f sum atom mw %f diff %f\n",
          last_res,
          last_resName.ascii(),
          last_asgn,
          prev_bead_mw,
          prev_atom_mw,
          prev_bead_mw -
          prev_atom_mw
          );
   printf("~~tot atom %d mw %f tot bead %d mw %f beads_size %u\n",
          atom_count,
          tot_atom_mw,
          bead_count,
          tot_bead_mw,
          (unsigned int)bead_model.size());
#endif

   write_bead_asa(somo_dir + SLASH +
                  project + QString("_%1").arg(current_model + 1) +
                  QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
                  + DOTSOMO + ".asa_res", &bead_model);

   progress->setProgress(ppos++); // 8
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }
   return 0;
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
            // cout << 
            // QString("%1 %2 %3 %4 %5\n")
            // .arg(this_atom->serial)
            // .arg(this_atom->name)
            // .arg(this_atom->resName)
            // .arg(this_atom->chainID)
            // .arg(this_atom->resSeq);
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
   cout << "exposed side chain atom list:\n";
   map < QString, bool > exposed_sc;

   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         if ( this_atom->exposed_code == 1 &&
              this_atom->chain == 1 )
         {
            exposed_sc[ 
                       QString( "%1%2%3" )
                       .arg( this_atom->resSeq )
                       .arg( this_atom->chainID != "" ? ":" : "" )
                       .arg( this_atom->chainID ) ] = true;
         }
      }
   }

   // rasmol view it
   QString out = QString("load %1\nselect all\ncolor gray\n").arg(pdb_file);

   for ( map < QString,bool >::iterator it = exposed_sc.begin();
         it != exposed_sc.end();
         it++ )
   {
      cout << it->first << endl;
      out += QString("select %1\ncolour green\n").arg(it->first);
   }

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
