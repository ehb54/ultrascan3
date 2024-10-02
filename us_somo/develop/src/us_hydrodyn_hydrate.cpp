#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_asab1.h"
#include "../include/us_surfracer.h"
#include "qmessagebox.h"
//Added by qt3to4:
#include <QTextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

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
// #define DEBUG_POINTMAP
// #define DEBUG_POINTMAP_ROTATE
#define MAX_WATER_POSITIONING_ATOMS 4

ostream& operator<<(ostream& out, const rotamer& c)
{
   out << "rotamer name: " << c.name 
       << " residue:"      << c.residue 
       << " extension:" << c.extension << endl;
   
   out << QString( " dihedral angles: %1\n" ).arg( c.dihedral_angles.size() );

   out << QString( " side chain size: %1 elements:" ).arg( c.dihedral_angles.size() );
   for ( unsigned int i = 0; i < c.side_chain.size(); i++ )
   {
      out << 
         QString( " side chain element %1 name %2 %3 %4 %5\n" )
         .arg( i )
         .arg( c.side_chain[ i ].name )
         .arg( c.side_chain[ i ].coordinate.axis[ 0 ] )
         .arg( c.side_chain[ i ].coordinate.axis[ 1 ] )
         .arg( c.side_chain[ i ].coordinate.axis[ 2 ] );
   }

   out << QString( " waters size: %1 elements:" ).arg( c.waters.size() );
   for ( unsigned int i = 0; i < c.waters.size(); i++ )
   {
      out << 
         QString( " waters element %1 name %2 %3 %4 %5\n" )
         .arg( i )
         .arg( c.waters[ i ].name )
         .arg( c.waters[ i ].coordinate.axis[ 0 ] )
         .arg( c.waters[ i ].coordinate.axis[ 1 ] )
         .arg( c.waters[ i ].coordinate.axis[ 2 ] );
   }

   out << QString( " water positioning atoms vector: %1\n" ).arg( c.water_positioning_atoms.size() );
   for ( unsigned int i = 0; i < c.water_positioning_atoms.size(); i++ )
   {
      out << QString( " water positioning atoms pos %1 elements:" ).arg( i );
      for ( unsigned int j = 0; j < c.water_positioning_atoms[ i ].size(); j++ )
      {
         out << " " << c.water_positioning_atoms[ i ][ j ];
      }
      out << endl;
   }

   out << " atom map:\n";
   rotamer tc = c;
   for ( map < QString, rotamer_atom >::iterator it = tc.atom_map.begin();
         it != tc.atom_map.end();
         it++ )
   {
      out << QString( " %1 maps to %2 %3 %4 %5\n" ).arg( it->first )
         .arg( it->second.name )
         .arg( it->second.coordinate.axis[ 0 ] )
         .arg( it->second.coordinate.axis[ 1 ] )
         .arg( it->second.coordinate.axis[ 2 ] )
         ;
   }
   out << " Rotamer end\n";
   return out;
}

int US_Hydrodyn::pdb_hydrate_for_saxs( bool quiet )
{
   QString error_msg;
   if ( !load_rotamer( error_msg ) )
   {
      QMessageBox::warning( this,
                            us_tr( "Error reading Hydrated Rotamer file" ),
                            error_msg );
      return -1;
   } 
#if defined(DEBUG_POINTMAP)
   cout << list_rotamers( false );
   cout << list_pointmaps();
   cout << list_rotated_rotamers();
#endif

   vector < unsigned int > selected_models;
   for ( unsigned int i = 0; i < (unsigned int)lb_model->count(); i++ ) 
   {
      if ( lb_model->item(i)->isSelected() ) 
      {
         selected_models.push_back(i);
      }
   }
   if ( selected_models.size() != 1 )
   {
      US_Static::us_message(us_tr("Please note:"),
                           us_tr("You must select exactly one model to hydrate."));
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

   for(int i = 0; i < lb_model->count(); i++) {
      if (lb_model->item(i)->isSelected()) {
         current_model = i;
         msg += QString(" %1").arg(i+1);
      }
   }
   msg += "\n";
   editor->append(msg);

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++)
   {
      if (!any_errors && lb_model->item(current_model)->isSelected())
      {
         any_models = true;
         if(!pdb_asa_for_saxs_hydrate())
         {
            // cout << list_exposed();
            // view_exposed();
            build_to_hydrate();
#if defined(DEBUG_TO_HYDRATE_DIHEDRAL)
            cout << list_to_hydrate();
#endif
            progress->setValue(9);
            qApp->processEvents();
            if ( !compute_to_hydrate_dihedrals( error_msg ) )
            {
               QMessageBox::warning( this,
                                     us_tr( "Error computing to-hydrate dihedrals" ),
                                     error_msg );
               any_errors = true;
            }
#if defined(DEBUG_TO_HYDRATE_DIHEDRAL)
            cout << list_to_hydrate_dihedrals();
#endif
            progress->setValue(10);
            qApp->processEvents();
            if ( !compute_best_fit_rotamer( error_msg ) )
            {
               QMessageBox::warning( this,
                                     us_tr( "Error finding best fit rotamer" ),
                                     error_msg );
               any_errors = true;
            }
            // cout << list_best_fit_rotamer();

            progress->setValue(11);
            qApp->processEvents();
            if ( !setup_pointmap_rotamers( error_msg ) )
            {
               QMessageBox::warning( this,
                                     us_tr( "Error setting up pointmaps" ),
                                     error_msg );
               any_errors = true;
            }
            // cout << list_pointmap_rotamers();

            progress->setValue(12);
            qApp->processEvents();
            if ( saxs_options.alt_hydration )
            {
               editor_msg( "blue", us_tr( "NOTICE: using alternate hydration (experimental) method" ) );
               if ( !compute_waters_to_add_alt( error_msg, quiet ) )
               {
                  QMessageBox::warning( this,
                                        us_tr( "Error trying to add waters" ),
                                        error_msg );
                  any_errors = true;
               }
            } else {
               if ( !compute_waters_to_add( error_msg, quiet ) )
               {
                  QMessageBox::warning( this,
                                        us_tr( "Error trying to add waters" ),
                                        error_msg );
                  any_errors = true;
               }
            }
            // cout << list_waters_to_add();
            progress->setValue( 1 ); progress->setMaximum( 1 );
            qApp->processEvents();
            if ( !write_pdb_with_waters( error_msg, quiet ) )
            {
               QMessageBox::warning( this,
                                     us_tr( "Error trying to write pdb with waters" ),
                                     error_msg );
               any_errors = true;
            }
            if ( !any_errors &&
                 !screen_pdb( last_hydrated_pdb_name, !quiet ) )
            {
               QMessageBox::warning( this,
                                     us_tr( "Error trying to reload hydrated pdb" ),
                                     error_msg );
               any_errors = true;
            }
            if ( !any_errors )
            {
               pdb_saxs( true, !quiet );
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
         bd_anaflex_enables( ( ( browflex && browflex->state() == QProcess::Running ) ||
                               ( anaflex && anaflex->state() == QProcess::Running ) ) ? false : true );
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
   int mppos = 13;
   progress->setMaximum(mppos);
   int ppos = 1;
   progress->setValue(ppos++); // 1
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
      progress->setValue(mppos);
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
      progress->setValue(mppos);
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

      progress->setValue(ppos++); // 2
      qApp->processEvents();
      editor->append("Return from Computing ASA\n");
      if (stopFlag)
      {
         return -1;
      }
      if ( retval )
      {
         editor->append("Errors found during ASA calculation\n");
         progress->setValue(mppos);
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
         progress->setValue(mppos);
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
      float save_radius = asa.probe_radius;
      asa.probe_radius  = asa.hydrate_probe_radius; 
      cout << QString(
                      "asa.hydrate_probe_radius %1\n"
                      "asa.probe_radius         %2\n"
                      "asa.asab1_step           %3\n"
                      "asa.threshold_percent    %4\n"
                      "asa.pb_rule_on           %5\n"
                      )
         .arg( asa.hydrate_probe_radius )
         .arg( asa.probe_radius )
         .arg( asa.asab1_step )
         .arg( asa.threshold_percent )
         .arg( misc.pb_rule_on ? "yes" : "no" )
         ;
      int retval = us_hydrodyn_asab1_main(active_atoms,
                                          &asa,
                                          &results,
                                          false,
                                          progress,
                                          editor,
                                          this
                                          );
      asa.probe_radius  = save_radius;
      progress->setValue(ppos++); // 2
      qApp->processEvents();
      editor->append("Return from Computing ASA\n");
      if (stopFlag)
      {
         return -1;
      }
      if ( retval )
      {
         editor->append("Errors found during ASA calculation\n");
         progress->setValue(mppos);
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

   FILE *asaf = us_fopen(QString(somo_tmp_dir + SLASH + "atom.asa"), "w");

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
                    this_atom->name.toLatin1().data(),
                    this_atom->resName.toLatin1().data(),
                    this_atom->resSeq.toLatin1().data(),
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

   progress->setValue(ppos++); // 3
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
#if defined( UHHH_DEBUG_ASA )
               cout << QString( "active atom asa %1 %2 %3 %4\n" )
                  .arg( this_atom->name )
                  .arg( this_atom->resName )
                  .arg( this_atom->serial )
                  .arg( this_atom->asa )
                  ;
#endif
               if ( advanced_config.debug_1 )
               {
                  printf("pass 2 active %s %s %d pm %d %d\n",
                         this_atom->name.toLatin1().data(),
                         this_atom->resName.toLatin1().data(),
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
                            this_atom->name.toLatin1().data(),
                            this_atom->resName.toLatin1().data(),
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
                            this_atom->name.toLatin1().data(),
                            this_atom->resName.toLatin1().data(),
                            this_atom->serial); fflush(stdout);
                  }
                  if (this_atom->bead_positioner) {

                     if (last_main_bead->bead_positioner &&
                         this_atom->placing_method == 1) {
                        fprintf(stderr, "warning: 2 positioners in bead %s %s %d\n",
                                last_main_bead->name.toLatin1().data(),
                                last_main_bead->resName.toLatin1().data(),
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
                         this_atom->name.toLatin1().data(),
                         this_atom->resName.toLatin1().data(),
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
                            this_atom->name.toLatin1().data(),
                            this_atom->resName.toLatin1().data(),
                            this_atom->serial,
                            use_atom->name.toLatin1().data(),
                            use_atom->resName.toLatin1().data(),
                            use_atom->serial);
                  }
               }

               if (!create_beads_normally ||
                   this_atom->bead_positioner) {
                  if (use_atom->bead_positioner &&
                      this_atom->placing_method == 1) {
                     fprintf(stderr, "warning: 2 or more positioners in bead %s %s %d\n",
                             use_atom->name.toLatin1().data(),
                             use_atom->resName.toLatin1().data(),
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

   progress->setValue(ppos++); // 4
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
                      this_atom->name.toLatin1().data(),
                      this_atom->resName.toLatin1().data(),
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
                         this_atom->resName.toLatin1().data(),
                         this_atom->resSeq.toLatin1().data()
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
                         this_atom->name.toLatin1().data(),
                         this_atom->resName.toLatin1().data(),
                         this_atom->serial,
                         last_main_chain_bead->name.toLatin1().data(),
                         last_main_chain_bead->resName.toLatin1().data(),
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
                         this_atom->name.toLatin1().data(),
                         this_atom->resName.toLatin1().data(),
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
                            this_atom->name.toLatin1().data(),
                            this_atom->resName.toLatin1().data(),
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
                         this_atom->name.toLatin1().data(),
                         this_atom->resName.toLatin1().data(),
                         this_atom->resSeq.toLatin1().data(),
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
                            this_atom->name.toLatin1().data(),
                            this_atom->resName.toLatin1().data(),
                            this_atom->resSeq.toLatin1().data(),
                            this_atom->bead_ref_mw,
                            this_atom->bead_ref_volume);
                  }
               }
               else
               {
                  QColor save_color = editor->textColor();
                  editor->setTextColor("red");
                  editor->append("Chain has broken end and PBR-NO-OXT isn't uniquely defined in the residue file.");
                  editor->setTextColor(save_color);
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
               QColor save_color = editor->textColor();
               editor->setTextColor("red");
               editor->append("Chain has no terminating OXT and PBR-NO-OXT isn't uniquely defined in the residue file.");
               editor->setTextColor(save_color);
            }
         }
      } // for j < molecule.size()
   }

   progress->setValue(ppos++); // 5
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
                      this_atom->name.toLatin1().data(),
                      this_atom->resName.toLatin1().data(),
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
   progress->setValue(ppos++); // 6
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
                      this_atom->name.toLatin1().data(),
                      this_atom->resName.toLatin1().data(),
                      this_atom->serial,
                      this_atom->placing_method); fflush(stdout);
# endif
               if(bead_mc_asa.size() < this_atom->resSeq + 1) {
                  bead_mc_asa.resize(this_atom->resSeq + 32);
               }
#warning broken by resSeq->QString
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
                      this_atom->name.toLatin1().data(),
                      this_atom->resName.toLatin1().data(),
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
                         this_atom->name.toLatin1().data(),
                         this_atom->resName.toLatin1().data(),
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
                     //   this_atom->name.toLatin1().data(),
                     //   this_atom->resName.toLatin1().data(),
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
                             this_atom->name.toLatin1().data(),
                             this_atom->resName.toLatin1().data(),
                             this_atom->serial);
                     break;
                  }
               }
               else 
               {
                  fprintf(stderr, "serious internal error 1 on %s %s %d, quitting\n",
                          this_atom->name.toLatin1().data(),
                          this_atom->resName.toLatin1().data(),
                          this_atom->serial);
                  exit(-1);
                  break;
               }
#if defined(DEBUG) || defined(OLD_ASAB1_SC_COMPUTE)
               printf("pass 3 active is bead %s %s %s checkpoint 1\n",
                      this_atom->name.toLatin1().data(),
                      this_atom->resName.toLatin1().data(),
                      this_atom->resSeq.toLatin1().data()); fflush(stdout);
#endif
               this_atom->visibility = (this_atom->bead_asa >= asa.hydrate_threshold);

#if defined( UHHH_DEBUG_ASA )
               cout << QString( "bead_asa %1 %2 %3 %4 %5\n" ).arg( this_atom->name ).arg( this_atom->resName ).arg( this_atom->serial ).arg( this_atom->bead_asa ).arg( asa.hydrate_threshold );
#endif

#if defined(OLD_ASAB1_SC_COMPUTE)
               if (this_atom->chain == 1) {
                  printf("visibility was %d is ", this_atom->visibility);
                  this_atom->visibility = (this_atom->bead_asa + bead_mc_asa[this_atom->resSeq] >= asa.hydrate_threshold);
                  printf("%d\n", this_atom->visibility);
               }
#endif

#if defined(DEBUG)
               printf("pass 3 active is bead %s %s %d checkpoint 2\n",
                      this_atom->name.toLatin1().data(),
                      this_atom->resName.toLatin1().data(),
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
                      this_atom->name.toLatin1().data(),
                      this_atom->resName.toLatin1().data(),
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
   progress->setValue(ppos++); // 7
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
                   this_atom->name.toLatin1().data(),
                   this_atom->resName.toLatin1().data(),
                   this_atom->chainID.toLatin1().data(),

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

   progress->setValue(ppos++); // 8
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }
   return 0;
}

void US_Hydrodyn::build_to_hydrate()
{
   to_hydrate.clear( );
   to_hydrate_pointmaps.clear( );

   residue_asa.clear( );
   residue_asa_sc.clear( );
   residue_asa_mc.clear( );

   unsigned int i = current_model;

   // pass 1 identify exposed sc's

   map < QString, bool > exposed_sc;
   map < QString, bool > exposed_for_pointmap;

   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         if ( this_atom->exposed_code == 1 )
         {
            if ( this_atom->chain == 1 )
            {
               exposed_sc[ QString( "%1~%2~%3" )
                           .arg( this_atom->resName )
                           .arg( this_atom->resSeq )
                           .arg( this_atom->chainID ) ] = true;
            }
            if ( pointmap_atoms.count( this_atom->resName ) )
            {
               exposed_for_pointmap[ QString( "%1~%2~%3" )
                                     .arg( this_atom->resName )
                                     .arg( this_atom->resSeq )
                                     .arg( this_atom->chainID ) ] = true;
            }
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

         if ( rx_main_chain.indexIn( this_atom->resName ) == -1 
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
         if ( exposed_for_pointmap.count( mapkey ) )
         {
            to_hydrate_pointmaps[ mapkey ][ this_atom->name ] = this_atom->coordinate;
         }
      }
   }

   // pass 3 add up ASA's
   for ( unsigned int j = 0; j < model_vector[i].molecule.size (); j++ )
   {
      for ( unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++ )
      {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

         QString mapkey =
            QString( "%1~%2~%3" )
            .arg( this_atom->resName )
            .arg( this_atom->resSeq )
            .arg( this_atom->chainID );

         residue_asa[ mapkey ] +=  this_atom->asa;

         if ( this_atom->chain == 1 )
         {
            residue_asa_sc[ mapkey ] +=  this_atom->asa;
         } else {
            residue_asa_mc[ mapkey ] +=  this_atom->asa;
         }
      }
   }  

   // pass 4 compute possible hydration
   hydrate_max_waters_no_asa = 0;

   map < QString, bool > accumulated_residue;

   for ( unsigned int j = 0; j < model_vector[i].molecule.size (); j++ )
   {
      for ( unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++ )
      {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

         QString mapkey =
            QString( "%1~%2~%3" )
            .arg( this_atom->resName )
            .arg( this_atom->resSeq )
            .arg( this_atom->chainID );

         if ( !accumulated_residue.count( mapkey ) )
         {
            accumulated_residue[ mapkey ] = true;
            if ( hydrate_count.count( this_atom->resName ) )
            {
               hydrate_max_waters_no_asa += hydrate_count[ this_atom->resName ];
            }
         }
      }
   }
   cout << QString( "max waters %1\n" ).arg( hydrate_max_waters_no_asa );
}

bool US_Hydrodyn::compute_to_hydrate_dihedrals( QString &error_msg )
{
   puts("computing to hydrate dihedrals");
   to_hydrate_dihedrals.clear( );

   vector < point > p(4);
   float dihedral;

   QRegExp rx_expand_mapkey("^(.+)~(.+)~(.*)$");

   for (  map < QString, map < QString, point > >::iterator it = to_hydrate.begin();
          it != to_hydrate.end();
          it++ )
   {
      if ( rx_expand_mapkey.indexIn( it->first ) == -1 )
      {
         error_msg = QString( us_tr( "Internal error: could not expand mapkey %1" ) ).arg( it->first );
         return false;
      }
      QString resName = rx_expand_mapkey.cap( 1 );
      // check dihedrals for this residue
      if ( !dihedral_atoms.count( resName ) )
      {
         error_msg = QString( us_tr( "No dihedral group found for residue %1." ) ).arg( resName );
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
   QString out = "to hydrate via dihedrals:\n";
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

   out += "to hydrate via pointmaps:\n";
   for ( map < QString, map < QString, point > >::iterator it = to_hydrate_pointmaps.begin();
         it != to_hydrate_pointmaps.end();
         it++ )
   {
      out += QString( "pointmap chains for: %1\n" ).arg( it->first );
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

   // asa's

   out += "ASA:\n";
   for ( map < QString, float >::iterator it = residue_asa.begin();
         it != residue_asa.end();
         it++ )
   {
      out += QString( "%1 tot=%2 sc=%3 mc=%4\n" )
         .arg( it->first )
         .arg( it->second )
         .arg( residue_asa_sc.count( it->first ) ?
               residue_asa_sc[ it->first ] : 0.0 )
         .arg( residue_asa_mc.count( it->first ) ?
               residue_asa_mc[ it->first ] : 0.0 )
         ;
   }

   return out;
}         

QString US_Hydrodyn::list_exposed()
{
   unsigned int i = current_model;
   QString out = "exposed side chain atom list:\n";
   map < QString, bool > exposed_sc;
   map < QString, bool > exposed_for_pointmap;

   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) 
   {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) 
      {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         if ( this_atom->exposed_code == 1 )
         {
            if ( this_atom->chain == 1 )
            {
               out +=
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
            if ( pointmap_atoms.count( this_atom->resName ) )
            {
               out +=
                  QString("%1 %2 %3 %4 %5 pointmap\n")
                  .arg(this_atom->serial)
                  .arg(this_atom->name)
                  .arg(this_atom->resName)
                  .arg(this_atom->chainID)
                  .arg(this_atom->resSeq);
               exposed_for_pointmap[ QString( "%1~%2~%3" )
                                     .arg( this_atom->resName )
                                     .arg( this_atom->resSeq )
                                     .arg( this_atom->chainID ) ] = true;
            }
         }
      }
   }

   for ( map < QString,bool >::iterator it = exposed_sc.begin();
         it != exposed_sc.end();
         it++ )
   {
      out += it->first + "\n";
   }

   for ( map < QString,bool >::iterator it = exposed_for_pointmap.begin();
         it != exposed_for_pointmap.end();
         it++ )
   {
      out += it->first + "\n";
   }
   return out;
}

void US_Hydrodyn::view_exposed()
{
   unsigned int i = current_model;
   // cout << "exposed side chain atom list:\n";
   map < QString, bool > exposed_sc;
   map < QString, bool > exposed_for_pointmap;
   map < QString, QString > use_color;
   
   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         if ( this_atom->exposed_code == 1 )
         {
            QString mapkey = 
               QString( "%1%2%3" )
               .arg( this_atom->resSeq )
               .arg( this_atom->chainID != "" ? ":" : "" )
               .arg( this_atom->chainID );
            
            if ( this_atom->chain == 1 )
            {
               exposed_sc[ mapkey ] = true;
               use_color[ mapkey ] = rotamers.count( this_atom->resName ) ? "green" : "yellow";
            }
            if ( pointmap_atoms.count( this_atom->resName ) )
            {
               exposed_for_pointmap[ mapkey ] = true;
               use_color[ mapkey ] = pointmap_atoms.count( this_atom->resName ) ? "cyan" : "yellow";
            }
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
   for ( map < QString,bool >::iterator it = exposed_for_pointmap.begin();
         it != exposed_for_pointmap.end();
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
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      editor->append("Error creating file " + fname + "\n");
      return;
   }
   QTextStream t( &f );
   t << out;
   f.close();
   model_viewer( fname, "-script" );
}

bool US_Hydrodyn::load_rotamer( QString &error_msg )
{
   puts("load_rotamer");
   if ( !rotamer_changed )
   {
      return true;
   }

   editor->append( us_tr("Reading hydrated rotamer file\n") );
   qApp->processEvents();
   rotamers.clear( );
   rotated_rotamers.clear( );
   dihedral_atoms.clear( );

   pointmap_atoms.clear( );
   pointmap_atoms_dest.clear( );
   pointmap_atoms_ref_residue.clear( );

   QFile f( saxs_options.default_rotamer_filename );
   if ( !f.exists() )
   {
      error_msg = us_tr( "Hydrated Rotamer file not found:\n"
                      "The Hydrated Rotamer file can be set in SOMO->SAXS/SANS Options" );
      return false;
   }
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      error_msg = us_tr( "Hydrated Rotamer file is not readable:\n"
                      "Check file permissions" );
      return false;
   }      

   QStringList qsl;
   QTextStream ts( &f );
   
   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      if ( qs.toLower().contains( QRegExp( "^end-file" ) ) )
      {
         break;
      }
      qsl << qs;
   }

   f.close();

   QRegExp rx_whitespace("\\s+");
   QRegExp rx_skip("^(#|\\s*$)");
   QRegExp rx_main_chain("^(N|O)$");
   QRegExp rx_atom("^ATOM");

   bool in_rotamer = false;
   bool in_rotamer_waters = false;

   rotamer tmp_rotamer;

   for ( unsigned int i = 0; i < (unsigned int) qsl.size(); i++ )
   {
      if ( rx_skip.indexIn( qsl[ i ] ) != -1 )
      {
         continue;
      }
      if ( qsl[ i ].length() > 30 && qsl[ i ].contains( rx_atom ) )
      {
#if QT_VERSION >= 0x040000
         qsl[ i ].data()[ 22 ] = ' ';
#else
         qsl[ i ].at( 22 ) = ' ';
#endif
      }
      QStringList qsl_line = (qsl[ i ] ).split( rx_whitespace , Qt::SkipEmptyParts );
      if ( qsl_line[ 0 ] == "multiple-rotate:" )
      {
         if ( in_rotamer )
         {
            error_msg = 
               QString( us_tr( "Error in hydrated rotamer file line %1.  multiple-rotate token can not occur within a rotamer definition" ) )
               .arg( i + 1 );
            return false;
         }            

         qsl_line.pop_front();

         if ( qsl_line.size() < 5 )
         {
            error_msg = 
               QString( us_tr( "Error in hydrated rotamer file line %1.  multiple-rotate: requires at least 5 arguments" ) )
               .arg( i + 1 );
            return false;
         }

         if ( ( qsl_line.size() - 2 ) % 3 )
         {
            error_msg = 
               QString( us_tr( "Error in hydrated rotamer file line %1.  multiple-rotate: requires exactly 2 + 3n arguments, n >= 1" ) )
               .arg( i + 1 );
            return false;
         }

         QString name                = qsl_line[ 0 ];
         QString residue             = name.left( 3 );
         unsigned int rotate         = qsl_line[ 1 ].toUInt();

         if ( rotated_rotamers.count( name ) )
         {
            error_msg = 
               QString( us_tr( "Error in hydrated rotamer file line %1.  multiple-rotate: %2 previously multiple-rotate'd" ) )
               .arg( i + 1 )
               .arg( name );
            return false;
         }

         if ( !rotamers.count( residue ) )
         {
            error_msg = 
               QString( us_tr( "Error in hydrated rotamer file line %1.  multiple-rotate: residue: %2 must be previously defined" ) )
               .arg( i + 1 )
               .arg( residue );
            return false;
         }

         if ( rotate <= 1 )
         {
            error_msg = 
               QString( us_tr( "Error in hydrated rotamer file line %1.  multiple-rotate: rotate count %2, must be 2 or greater" ) )
               .arg( i + 1 )
               .arg( rotate );
            return false;
         }

         unsigned int water_count = ( qsl_line.size() - 2 ) / 3;
#if defined( UHH_DEBUG_RR )
         cout << QString( "multiple-rotate %1 water count %2\n" ).arg( name ).arg( water_count );
#endif
         // // we need the rotamer name matched one
         // this info is not available yet, needs compute water_positioning atoms to complete
         
         //          if ( water_count != rotamers[ residue ][ 0 ].water_positioning_atoms.size() )
         //          {
         //             error_msg = 
         //                QString( us_tr( "Error in hydrated rotamer file line %1.  multiple-rotate: difference in water count (%2) with reference rotamer water count (%3)" ) )
         //                .arg( i + 1 )
         //                .arg( water_count )
         //                .arg( rotamers[ residue ][ 0 ].water_positioning_atoms.size() );
         //             return false;
         //          }

         rotated_rotamers[ name ].resize( water_count );

         for ( unsigned int water_number = 0; water_number < water_count; water_number++ )
         {
            vector < QString >  atoms( 3 );
            atoms[ 0 ]                  = qsl_line[ 2 + 3 * water_number ];
            atoms[ 1 ]                  = qsl_line[ 3 + 3 * water_number ];
            atoms[ 2 ]                  = qsl_line[ 4 + 3 * water_number ];

            bool         found_match = false;
            unsigned int pos;
            for ( unsigned int j = 0; j < (unsigned int) rotamers[ residue ].size(); j++ )
            {
               // cout << QString( "rotamers '%1' %2 name '%3'\n" )
               // .arg( residue )
               // .arg( j )
               // .arg( rotamers[ residue ][ j ].name );
               if ( rotamers[ residue ][ j ].name == name )
               {
                  found_match = true;
                  pos = j;
                  break;
               }
            }
            if ( !found_match )
            {
               error_msg = 
                  QString( us_tr( "Error in hydrated rotamer file line %1.  multiple-rotate: residue: %2 found, but full name %3 missing" ) )
                  .arg( i + 1 )
                  .arg( residue )
                  .arg( name );
               return false;
            }
            
#if defined( UHH_DEBUG_RR )
            cout << QString( "multiple-rotate %1 x%2 found rotamer match %3 pos %4 for water %5\n" )
               .arg( name )
               .arg( rotate )
               .arg( residue )
               .arg( pos )
               .arg( water_number )
               ;
#endif
         
            rotamer ref_rotamer = rotamers[ residue ][ pos ];

            // check existance & set atom pos for first two
            vector < unsigned int > atom_pos( 3 );

            for ( unsigned int j = 0; j < 2; j++ )
            {
               bool found_sc_atom = false;
               for ( unsigned int k = 0; k < (unsigned int) ref_rotamer.side_chain.size(); k++ )
               {
                  if ( ref_rotamer.side_chain[ k ].name == atoms[ j ] )
                  {
                     found_sc_atom = true;
                     atom_pos[ j ] = k;
                     break;
                  }
               }
               if ( !found_sc_atom )
               {
                  error_msg = 
                     QString( us_tr( "Error in hydrated rotamer file line %1.  multiple-rotate: residue: %2 found, atom %3 missing" ) )
                     .arg( i + 1 )
                     .arg( residue )
                     .arg( atoms[ j ] );
                  return false;
               }
            }               
               
            if ( (unsigned int) ref_rotamer.waters.size() <= water_number )
            {
               error_msg = 
                  QString( us_tr( "Error in hydrated rotamer file line %1.  multiple-rotate: residue: %2 found reference rotamer, but it has %3 waters, less than %4 requested" ) )
                  .arg( i + 1 )
                  .arg( residue )
                  .arg( ref_rotamer.waters.size() )
                  .arg( water_number + 1 )
                  ;
               return false;
            }
            
            // check existance of atoms[ 2 ]
         
            if ( ref_rotamer.waters[ water_number ].name != atoms[ 2 ] )
            {
               error_msg = 
                  QString( us_tr( "Error in hydrated rotamer file line %1.  multiple-rotate: residue: %2 found, atom %3 missing from waters" ) )
                  .arg( i + 1 )
                  .arg( residue )
                  .arg( atoms[ 2 ] );
               return false;
            }
            
            // now build up rotation into

            // compute distance & angle

            vector < point > p( 3 );
            p[ 0 ] = ref_rotamer.side_chain[ atom_pos[ 0 ] ].coordinate;
            p[ 1 ] = ref_rotamer.side_chain[ atom_pos[ 1 ] ].coordinate;
            p[ 2 ] = ref_rotamer.waters    [ water_number  ].coordinate;

            float d01 = dist( p[ 0 ], p[ 1 ] );
            float d12 = dist( p[ 1 ], p[ 2 ] );

#if defined( UHH_DEBUG_RR )
            float d02 = dist( p[ 0 ], p[ 2 ] );

            cout << QString( "distance %1 to %2 = %3 A\n" )
               .arg( atoms[ 0 ] )
               .arg( atoms[ 1 ] )
               .arg( d01 );

            cout << QString( "distance %1 to %2 = %3 A\n" )
               .arg( atoms[ 1 ] )
               .arg( atoms[ 2 ] )
               .arg( d12 );

            cout << QString( "distance %1 to %2 = %3 A\n" )
               .arg( atoms[ 0 ] )
               .arg( atoms[ 2 ] )
               .arg( d02 );
#endif

            float a = acosf( dot( normal( minus( p[ 0 ], p[ 1 ] ) ),
                                  normal( minus( p[ 2 ], p[ 1 ] ) ) ) );
      
#if defined( UHH_DEBUG_RR )
            cout << QString( us_tr( "Angle %1,%2,%3 = %4 or %5 degrees\n" ) )
               .arg( atoms[ 0 ] )
               .arg( atoms[ 1 ] )
               .arg( atoms[ 2 ] )
               .arg( a )
               .arg( a * 180.0 / M_PI )
               ;
#endif

            // transform reference rotamer from p(3) to 
            // (0,0,-d01), (0,0,0), ( d12 * cos( a ), 0, d12 * sin( a ) )
            float xdist = d12 * cos( a - M_PI / 2.0 );

            vector < point > np( 3 );
            np[ 1 ].axis[ 0 ] = 0.0;
            np[ 1 ].axis[ 1 ] = 0.0;
            np[ 1 ].axis[ 2 ] = 0.0;

            np[ 0 ]           = np[ 1 ];
            np[ 0 ].axis[ 2 ] = - d01;

            np[ 2 ].axis[ 0 ] = xdist;
            np[ 2 ].axis[ 1 ] = 0.0;
            np[ 2 ].axis[ 2 ] = d12 * sin( a - M_PI / 2.0 );
         
#if defined( UHH_DEBUG_RR )
            cout << QString( "transform from: " ) <<  p[ 0 ] << " " <<  p[ 1 ] << " " <<  p[ 2 ] << endl;
            cout << QString( "transform to: " )   << np[ 0 ] << " " << np[ 1 ] << " " << np[ 2 ] << endl;

            float nd01 = dist( np[ 0 ], np[ 1 ] );
            float nd12 = dist( np[ 1 ], np[ 2 ] );
            float nd02 = dist( np[ 0 ], np[ 2 ] );

            cout << QString( "new distance %1 to %2 = %3 A\n" )
               .arg( atoms[ 0 ] )
               .arg( atoms[ 1 ] )
               .arg( nd01 );

            cout << QString( "new distance %1 to %2 = %3 A\n" )
               .arg( atoms[ 1 ] )
               .arg( atoms[ 2 ] )
               .arg( nd12 );

            cout << QString( "new distance %1 to %2 = %3 A\n" )
               .arg( atoms[ 0 ] )
               .arg( atoms[ 2 ] )
               .arg( nd02 );

            float na = acosf( dot( normal( minus( np[ 0 ], np[ 1 ] ) ),
                                   normal( minus( np[ 2 ], np[ 1 ] ) ) ) );
      
            cout << QString( us_tr( "new: Angle %1,%2,%3 = %4 or %5 degrees\n" ) )
               .arg( atoms[ 0 ] )
               .arg( atoms[ 1 ] )
               .arg( atoms[ 2 ] )
               .arg( na )
               .arg( na * 180.0 / M_PI )
               ;
#endif

            // now setup and do transformation
            vector < point > ref_rotamer_p;
            for ( unsigned int j = 0; j < (unsigned int) ref_rotamer.side_chain.size(); j++ )
            {
               ref_rotamer_p.push_back( ref_rotamer.side_chain[ j ].coordinate );
            }
            for ( unsigned int j = 0; j < (unsigned int) ref_rotamer.waters.size(); j++ )
            {
               ref_rotamer_p.push_back( ref_rotamer.waters[ j ].coordinate );
            }

            vector < point > ref_rotamer_np( ref_rotamer_p.size() );
            if ( !atom_align( p, np, ref_rotamer_p, ref_rotamer_np, error_msg ) )
            {
               error_msg = 
                  QString( us_tr( "Error in hydrated rotamer file line %1.  multiple-rotate: " ) ).
                  arg( i + 1 ) + error_msg;
               return false;
            }
            rotamer new_ref_rotamer = ref_rotamer;
            // not avail yet
            // new_ref_rotamer.water_positioning_atoms.push_back( ref_rotamer.water_positioning_atoms[ water_number ] );
            for ( unsigned int j = 0; j < (unsigned int) ref_rotamer.side_chain.size(); j++ )
            {
               new_ref_rotamer.side_chain[ j ].coordinate = ref_rotamer_np[ j ];
               new_ref_rotamer.atom_map[ new_ref_rotamer.side_chain[ j ].name ].coordinate =
                  new_ref_rotamer.side_chain[ j ].coordinate;
            }
            
            // now we are putting each water in its own ref rotamer since
            // each is being multiply rotated and
            // it is a water of memory to store rot**waters variants
            // for ( unsigned int j = 0; j < (unsigned int) ref_rotamer.waters.size(); j++ )
            // {
            new_ref_rotamer.waters.resize( 1 );
            new_ref_rotamer.waters[ 0 ].coordinate = ref_rotamer_np[ ref_rotamer.side_chain.size() + water_number ];
            // }

            float delta_rho = ( 2.0 * M_PI ) / (float) rotate;

            for ( unsigned int j = 0; j < rotate; j++ )
            {
               float rho =  (float) j * delta_rho;
            
               new_ref_rotamer.waters[ 0 ].coordinate.axis[ 0 ] = xdist * cos( rho );
               new_ref_rotamer.waters[ 0 ].coordinate.axis[ 1 ] = xdist * sin( rho );

               // FIX THIS!!!    check rotamer for internal steric clash!

               rotated_rotamers[ name ][ water_number ].push_back( new_ref_rotamer );
            }

#if defined( UHH_DEBUG_RR )
            cout << QString( "multiple-rotate: created %1 rotamers for %2\n" )
               .arg( rotated_rotamers[ name ].size() )
               .arg( name );
#endif
         } // water number

         // #define UHH_LIST_NMR_MODELS
#if defined( UHH_LIST_NMR_MODELS )
         // create nmr style pdb for rotamer

         QString out;

         // now we have a vector < vector < rotamer > > for this name 
         // where the waters are each top level vector
         // i.e. we need to linearize the choices of this 
         // but each chain ref is rotated into a different coordinate frame, so
         // each one needs to be added separately, so
         // we will just list each vector as a separate model


         for ( unsigned int jj = 0; jj < (unsigned int) rotated_rotamers[ name ].size(); jj++ )
         {
            out += QString( "REMARK  water %1\n" ).arg( jj + 1 );
            for ( unsigned int j = 0; j < (unsigned int) rotated_rotamers[ name ][ jj ].size(); j++ )
            {
               out += QString( "MODEL     %1\n" ).arg( j + 1 );

               unsigned int apos = 0;

               for ( unsigned int k = 0; k < (unsigned int) rotated_rotamers[ name ][ jj ][ j ].side_chain.size(); k++ )
               {
                  out += QString( "" )
                     .sprintf(     
                              "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                              ++apos,
                              rotated_rotamers[ name ][ jj ][ j ].side_chain[ k ].name.toLatin1().data(),
                              name.left( 3 ).toLatin1().data(),
                              "A",
                              1,
                              rotated_rotamers[ name ][ jj ][ j ].side_chain[ k ].coordinate.axis[ 0 ],
                              rotated_rotamers[ name ][ jj ][ j ].side_chain[ k ].coordinate.axis[ 1 ],
                              rotated_rotamers[ name ][ jj ][ j ].side_chain[ k ].coordinate.axis[ 2 ],
                              1.00,
                              0.00,
                              " "
                              );
               }
               out += "TER\n";
               for ( unsigned int k = 0; k < (unsigned int) rotated_rotamers[ name ][ jj ][ j ].waters.size(); k++ )
               {
                  out += QString( "" )
                     .sprintf(     
                              "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                              ++apos,
                              rotated_rotamers[ name ][ jj ][ j ].waters[ k ].name.toLatin1().data(),
                              "WAT",
                              "B",
                              2,
                              rotated_rotamers[ name ][ jj ][ j ].waters[ k ].coordinate.axis[ 0 ],
                              rotated_rotamers[ name ][ jj ][ j ].waters[ k ].coordinate.axis[ 1 ],
                              rotated_rotamers[ name ][ jj ][ j ].waters[ k ].coordinate.axis[ 2 ],
                              1.00,
                              0.00,
                              " "
                              );
               }
               out += "TER\nENDMDL\n";
            }
            out += "END\n";
            out += "REMARK  --- split models -----------\n";
         }

         cout << "--- start nmr style PDB-----------\n" 
              << out 
              << "--- end nmr style PDB-----------\n";
#endif //  defined( UHH_LIST_NMR_MODELS )
         continue;
      } // multiple-rotate

      if ( qsl_line[ 0 ] == "pointmap:" )
      {
         qsl_line.pop_front();
         QString res = qsl_line[ 0 ];
         qsl_line.pop_front();
         if ( pointmap_atoms.count( res ) )
         {
            error_msg = 
               QString( us_tr( "Error in hydrated rotamer file line %1.  %2 was previously defined as a pointmap" ) )
               .arg( i + 1 )
               .arg( res );
            return false;
         }
            
         while ( qsl_line.size() )
         {
            unsigned int points = qsl_line[ 0 ].toUInt();
            qsl_line.pop_front();
            if ( points < 3 )
            {
               error_msg = 
                  QString( us_tr( "Error in hydrated rotamer file line %1.  Invalid number of points for a map on pointmap line" ) )
                  .arg( i + 1 );
               return false;
            }
            vector < QString > atoms;
            for ( unsigned int j = 0; j < points; j++ )
            {
               if ( !qsl_line.size() )
               {
                  error_msg = 
                     QString( us_tr( "Error in hydrated rotamer file line %1.  Invalid number of tokens on pointmap line" ) )
                     .arg( i + 1 );
                  return false;
               }
               atoms.push_back( qsl_line[ 0 ] );
               qsl_line.pop_front();
            }
            pointmap_atoms[ res ].push_back( atoms );

            if ( !qsl_line.size() )
            {
               error_msg = 
                  QString( us_tr( "Error in hydrated rotamer file line %1.  Invalid number of tokens on pointmap line" ) )
                  .arg( i + 1 );
               return false;
            }
            pointmap_atoms_ref_residue[ res ].push_back( qsl_line[ 0 ] );
            qsl_line.pop_front();

            vector < QString > atoms_dest;
            for ( unsigned int j = 0; j < points; j++ )
            {
               if ( !qsl_line.size() )
               {
                  error_msg = 
                     QString( us_tr( "Error in hydrated rotamer file line %1.  Invalid number of tokens on pointmap line" ) )
                     .arg( i + 1 );
                  return false;
               }
               atoms_dest.push_back( qsl_line[ 0 ] );
               qsl_line.pop_front();
            }
            pointmap_atoms_dest[ res ].push_back( atoms_dest );
         }
         if ( res == "AA" )
         {
            cout << QString( "expanding AA pointmap\n" );
            QStringList AAs;
            AAs 
               << "ALA"
               << "ARG"
               << "ASN"
               << "ASP"
               << "CYS"
               << "GLN"
               << "GLU"
               << "GLY"
               << "HIS"
               << "ILE"
               << "LEU"
               << "LYS"
               << "MET"
               << "PHE"
               << "PRO"
               << "SER"
               << "THR"
               << "TRP"
               << "TYR"
               << "VAL";
            for ( unsigned int j = 0; j < (unsigned int) AAs.size(); j++ )
            {
               cout << QString( "expanding AA pointmap for AA %1\n" ).arg( AAs[ j ] );
               if ( pointmap_atoms.count( AAs[ j ] ) )
               {
                  error_msg = 
                     QString( us_tr( "Error in hydrated rotamer file line %1. "
                                  "Pointmap defined for AA, but preexisting pointmap defined for Amino Acid residue %2." ) )
                     .arg( i + 1 )
                     .arg( AAs[ j ] );
                  return false;
               }
               pointmap_atoms            [ AAs[ j ] ] = pointmap_atoms            [ res ];
               pointmap_atoms_dest       [ AAs[ j ] ] = pointmap_atoms_dest       [ res ];
               pointmap_atoms_ref_residue[ AAs[ j ] ] = pointmap_atoms_ref_residue[ res ];
            }
         }
         continue;
      }  

      if ( qsl_line[ 0 ] == "dihedral:" )
      {
         qsl_line.pop_front();
         QString res = qsl_line[ 0 ];
         qsl_line.pop_front();
         vector < QString > four_atoms;
         for ( unsigned int j = 0; j < (unsigned int) qsl_line.size(); j++ )
         {
            four_atoms.push_back( qsl_line[ j ] );
            if ( four_atoms.size() == 4 )
            {
               dihedral_atoms[ res ].push_back( four_atoms );
               four_atoms.clear( );
            }
         }
         if ( four_atoms.size() )
         {
            error_msg = 
               QString( us_tr( "Error in hydrated rotamer file line %1.  Invalid number of tokens %2 on dihedral line" ) )
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
               QString( us_tr( "Error in hydrated rotamer file line %1.  Invalid number of tokens %2" ) )
               .arg( i + 1 )
               .arg( qsl_line.size() );
            return false;
         }
         
         if ( qsl_line[ 0 ] != "start-rotamer:" )
         {
            error_msg = 
               QString( us_tr( "Error in hydrated rotamer file line %1.  Unrecognized token %2" ) )
               .arg( i + 1 )
               .arg( qsl_line[ 0 ] );
            return false;
         }
         tmp_rotamer.name = qsl_line[ 1 ];
         tmp_rotamer.residue = tmp_rotamer.name.left( 3 );
         tmp_rotamer.extension = tmp_rotamer.name.right( 7 );
         tmp_rotamer.side_chain.clear( );
         tmp_rotamer.waters.clear( );
         tmp_rotamer.atom_map.clear( );
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
                  QString( us_tr( "Error in hydrated rotamer file line %1.  Empty rotamer" ) )
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
               QString( us_tr( "Error in hydrated rotamer file line %1.  Unrecognized token %2 in this context" ) )
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
            QString( us_tr( "Error in hydrated rotamer file line %1:\n"
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
            QString( us_tr( "Error in hydrated rotamer file line %1.  Unrecognized token %2" ) )
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
           rx_main_chain.indexIn( qsl_line[ 2 ] ) != -1 )
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
      ra.coordinate.axis[ 0 ] = qsl_line[ 6  ].toFloat();
      ra.coordinate.axis[ 1 ] = qsl_line[ 7  ].toFloat();
      ra.coordinate.axis[ 2 ] = qsl_line[ 8  ].toFloat();
      // ra.occupancy            = qsl_line[ 9  ].toFloat();
      // ra.tempFactor           = qsl_line[ 10 ];
            
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
         us_tr( "Error in hydrated rotamer file at end, no final end-rotamer." );
      return false;
   }

   cout << validate_pointmap();

   editor->append( us_tr("Done reading hydrated rotamer file\n") );
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


   // compute totals
   hydrate_count.clear( );
   for ( map < QString, vector < vector < QString > > >::iterator it = pointmap_atoms.begin();
         it != pointmap_atoms.end();
         it++ )
   {
      if ( hydrate_count.count( it->first ) )
      {
         hydrate_count[ it->first ] += ( unsigned int ) it->second.size();
      } else {
         hydrate_count[ it->first ] =  ( unsigned int ) it->second.size();
      }
   }

   for ( map < QString, vector < rotamer > >::iterator it = rotamers.begin();
         it != rotamers.end();
         it++ )
   {
      for ( unsigned int i = 0; i < (unsigned int) it->second.size() && i < 1; i++ )
      {
         if ( hydrate_count.count( it->first ) )
         {
            hydrate_count[ it->first ] += ( unsigned int ) it->second[ i ].waters.size();
         } else {
            hydrate_count[ it->first ] =  ( unsigned int ) it->second[ i ].waters.size();
         }
      }
   }

   for ( map < QString, unsigned int >::iterator it = hydrate_count.begin();
         it != hydrate_count.end();
         it++ )
   {
      cout << QString( "res %1 hydrate count %2\n" ).arg( it->first ).arg( it->second );
   }

   return true;
}

bool US_Hydrodyn::compute_rotamer_dihedrals( QString &error_msg )
{
   editor->append( us_tr("Calculating dihedrals of rotamers\n") );
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
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
#if defined( DEBUG_DIHEDRAL )
         cout << 
            QString("computation for dihedral %1 %2:\n")
            .arg( it->first )
            .arg( it->second[ i ].extension );
#endif
         it->second[ i ].dihedral_angles.clear( );
         // process each dihedral chain, giving 4 points
         for ( unsigned int j = 0; j < (unsigned int) dihedral_atoms[ it->first ].size(); j++ )
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
            for ( unsigned int k = 0; k < (unsigned int) dihedral_atoms[ it->first ][ j ].size(); k++ )
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
   editor->append( us_tr("Done calculating dihedrals of rotamers\n") );
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
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         out += QString(" %1: sc").arg( it->second[ i ].extension );
         for ( unsigned int j = 0; j < (unsigned int) it->second[ i ].dihedral_angles.size(); j++ )
         {
            out += QString(" %1").arg( it->second[ i ].dihedral_angles[ j ] );
         }
         out += "\n";
      }
      out += "\n";
   }
   return out;
}

QString US_Hydrodyn::list_pointmaps()
{
   QString out;

   out = "pointmaps:\n";

   for ( map < QString, vector < vector < QString > > >::iterator it = pointmap_atoms.begin();
         it != pointmap_atoms.end();
         it++ )
   {
      out += 
         QString( "Pointmaps for residue: %1  number of maps %2\n" )
         .arg( it->first )
         .arg( it->second.size() );
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         out += 
            QString( "    mapping to %1\n" )
            .arg( pointmap_atoms_ref_residue.count( it->first ) 
                  ?
                  ( pointmap_atoms_ref_residue[ it->first ].size() > i 
                    ?
                    pointmap_atoms_ref_residue[ it->first ][ i ]
                    :
                    "ERROR missing reference residue map" )
                  :
                  "ERROR missing reference residue" );

         if ( pointmap_atoms_ref_residue.count( it->first ) &&
              pointmap_atoms_ref_residue[ it->first ].size() > i )
         {
            for ( unsigned int j = 0; j < (unsigned int) it->second[ i ].size(); j++ )
            {
               out += QString("    %1 -> %2\n")
                  .arg( it->second[ i ][ j ] )
                  .arg( pointmap_atoms_dest.count( it->first )
                        ?
                        ( (unsigned int) pointmap_atoms_dest[ it->first ].size() > i 
                          ?
                          ( (unsigned int) pointmap_atoms_dest[ it->first ][ i ].size() > j 
                            ?
                            pointmap_atoms_dest[ it->first ][ i ][ j ] 
                            :
                            "ERROR missing destination residue map elememt" )
                          :
                          "ERROR missing destination residue map" )
                        :
                        "ERROR missing destination residue" );
            }
         }
      }
   }
   return out;
}

QString US_Hydrodyn::list_rotated_rotamers()
{
   QString out = "no";

   cout << "list rotated rotamers\n";
   for ( map < QString, vector < vector < rotamer > > >::iterator it = rotated_rotamers.begin();
         it != rotated_rotamers.end();
         it++ )
   {
      cout << QString( "key %1 entries %2\n" ).arg( it->first ).arg( it->second.size() );
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         cout << QString( "key %1 entry %2 entries %3\n" ).arg( it->first ).arg( i ).arg( it->second[ i ].size() );
         for ( unsigned int j = 0; j < it->second[ i ].size(); j++ )
         {
            cout << it->second[ i ][ j ];
          }
      }
   }


   cout << "list rotamers for reference\n";
   for ( map < QString, vector < rotamer > >::iterator it = rotamers.begin();
         it != rotamers.end();
         it++ )
   {
      cout << QString( "key %1 entries %2\n" ).arg( it->first ).arg( it->second.size() );
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         cout << it->second[ i ];
      }
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
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         for ( unsigned int j = 0; j < (unsigned int) it->second[i].size(); j++ )
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
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         out += QString(" %1: sc").arg( it->second[ i ].extension );
         for ( unsigned int j = 0; j < (unsigned int) it->second[ i ].side_chain.size(); j++ )
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
         for ( unsigned int j = 0; j < (unsigned int) it->second[ i ].waters.size(); j++ )
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
   best_fit_rotamer.clear( );

   QRegExp rx_expand_mapkey("^(.+)~(.+)~(.*)$");

   // go through the computed dihedrals, compute sum of abs differences, choose best one
   for ( map < QString, vector < float > >::iterator it = to_hydrate_dihedrals.begin();
         it != to_hydrate_dihedrals.end();
         it++ )
   {
      if ( rx_expand_mapkey.indexIn( it->first ) == -1 )
      {
         error_msg = QString( us_tr( "Internal error: could not expand mapkey %1" ) ).arg( it->first );
         return false;
      }
      QString resName = rx_expand_mapkey.cap( 1 );
      // check dihedrals for this residue
      if ( !rotamers.count( resName ) )
      {
         error_msg = QString( us_tr( "No rotamer group found for residue %1." ) ).arg( resName );
         return false;
      }
      
      float        best_fitness     = 1e30f;
      unsigned int best_fitness_pos = 0;

      for ( unsigned int i = 0; i < (unsigned int) rotamers[ resName ].size(); i++ )
      {
         if ( it->second.size() != rotamers[ resName ][ i ].dihedral_angles.size() )
         {
            error_msg = 
               QString( us_tr( "Number of dihedrals do not match between side chain %1 (%2) and rotamer %3 (%4)" ) )
               .arg( it->first )
               .arg( it->second.size() )
               .arg( rotamers[ resName ][ i ].name )
               .arg( rotamers[ resName ][ i ].dihedral_angles.size() );
            return false;
         }
         float fitness = 0.0;
         for ( unsigned int j = 0; j < (unsigned int) it->second.size(); j++ )
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

bool US_Hydrodyn::setup_pointmap_rotamers( QString &error_msg )
{
   pointmap_rotamers.clear( );

   QRegExp rx_expand_mapkey("^(.+)~(.+)~(.*)$");

   for ( map < QString, map < QString, point > >::iterator it = to_hydrate_pointmaps.begin();
         it != to_hydrate_pointmaps.end();
         it++ )
   {
      if ( rx_expand_mapkey.indexIn( it->first ) == -1 )
      {
         error_msg = QString( us_tr( "Internal error: could not expand mapkey %1" ) ).arg( it->first );
         return false;
      }
      QString resName = rx_expand_mapkey.cap( 1 );
      // for each entry in pointmap_atoms_ref_residue
      if ( !pointmap_atoms_ref_residue.count( resName ) )
      {
         error_msg = QString( us_tr( "Internal error: could not find rotamer for pointmap key %1" ) ).arg( it->first );
         return false;
      }

      for ( unsigned int i = 0; i < (unsigned int) pointmap_atoms_ref_residue[ resName ].size(); i++ )
      {
         if ( !rotamers.count( pointmap_atoms_ref_residue[ resName ][ i ] ) )
         {
            error_msg = QString( us_tr( "No rotamer group found for pointmap key %1 reference residue %2." ) )
               .arg( it->first )
               .arg( pointmap_atoms_ref_residue[ resName ][ i ] );
            return false;
         }
         // use first one
         pointmap_rotamers[ it->first ].push_back( rotamers[ pointmap_atoms_ref_residue[ resName ][ i ] ][ 0 ] );
      }
   }
   return true;
}

QString US_Hydrodyn::list_pointmap_rotamers()
{
   QString out;

   out = "Pointmap rotamers:\n";

   for ( map < QString, vector < rotamer > >::iterator it = pointmap_rotamers.begin();
         it != pointmap_rotamers.end();
         it++ )
   {
      out += QString( "%1:" ).arg( it->first );
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         out += QString( " %1" ).arg( it->second[ i ].name );
      }
      out += "\n";
   }
   return out;
}

bool US_Hydrodyn::compute_water_positioning_atoms( QString & error_msg )
{
   editor->append( us_tr("Calculating water positioning atoms for each rotamer\n") );
   qApp->processEvents();
   puts("Calculating water positioning atoms for each rotamer");

   list < sortable_float > lsf;
   sortable_float          sf;
   vector < QString >      wpa;

   map < QString, rotamer * > back_ref;

   for ( map < QString, vector < rotamer > >::iterator it = rotamers.begin();
         it != rotamers.end();
         it++ )
   {
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         it->second[ i ].water_positioning_atoms.clear( );

         // for each water

         for ( unsigned int j = 0; j < (unsigned int) it->second[ i ].waters.size(); j++ )
         {
            lsf.clear( );
            wpa.clear( );
            
            // find the distance to each side chain atom
            for ( unsigned int k = 0; k < (unsigned int) it->second[ i ].side_chain.size(); k++ )
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
         back_ref[ it->second[ i ].name ] = &(it->second[ i ]);
      }
   }

   editor->append( us_tr("Done calculating water positioning atoms for each rotamer\n") );
   qApp->processEvents();
   puts("Done calculating water positioning atoms for each rotamer");

   editor->append( us_tr("Copy over water positioning info for rotated rotamers\n") );
   qApp->processEvents();

   for ( map < QString, vector < vector < rotamer > > >::iterator it = rotated_rotamers.begin();
         it != rotated_rotamers.end();
         it++ )
   {
      if ( !back_ref.count( it->first ) )
      {
         error_msg = QString( us_tr( "Error: rotated_rotamer wth name %1 not found it rotamers" ) )
            .arg( it->first );
         return false;
      }

      unsigned int water_count = (unsigned int) it->second.size();
      
      if ( water_count != back_ref[ it->first ]->water_positioning_atoms.size() )
      {
         error_msg = QString( us_tr( "Error: rotated_rotamer wth name %1 found but water count differs %2 vs %3" ) )
            .arg( it->first )
            .arg( water_count )
            .arg( back_ref[ it->first ]->water_positioning_atoms.size() );
         return false;
      }

      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         for ( unsigned int j = 0; j < (unsigned int) it->second[ i ].size(); j++ )
         {
            it->second[ i ][ j ].water_positioning_atoms.push_back( back_ref[ it->first ]->water_positioning_atoms[ i ] );
         }
      }
   }

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
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         out += QString("  %1:\n").arg( it->second[ i ].extension );
         for ( unsigned int j = 0; j < (unsigned int) it->second[ i ].water_positioning_atoms.size(); j++ )
         {
            out += QString( "    water %1 :" ).arg( j );
            for ( unsigned int k = 0; k < (unsigned int) it->second[ i ].water_positioning_atoms[ j ].size(); k++ )
            {
               out += QString(" %1").arg( it->second[ i ].water_positioning_atoms[ j ][ k ] );
            }
            out += "\n";
         }
      }
      out += "\n";
   }

   // alternate format

   for (  map < QString, vector < rotamer > >::iterator it = rotamers.begin();
          it != rotamers.end();
          it++ )
   {
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         out += QString("%1-%2 ").arg( it->first ).arg( it->second[ i ].extension );
         for ( unsigned int j = 0; j < (unsigned int) it->second[ i ].water_positioning_atoms.size(); j++ )
         {
            out += QString( " [%1]" ).arg( j );
            for ( unsigned int k = 0; k < (unsigned int) it->second[ i ].water_positioning_atoms[ j ].size(); k++ )
            {
               out += QString(" %1").arg( it->second[ i ].water_positioning_atoms[ j ][ k ] );
            }
         }
         out += "\n";
      }
      out += "\n";
   }
   return out;
}

bool US_Hydrodyn::compute_waters_to_add( QString &error_msg, bool quiet )
{
   editor->append( us_tr("Transforming waters to add to pdb coordinates\n") );
   qApp->processEvents();
   puts("Transforming waters to add to pdb coordinates");

   steric_clash_summary.clear( );

   count_waters           = 0;
   count_waters_added     = 0;
   count_waters_not_added = 0;

   vector < point > p1;
   vector < point > p2;
   waters_to_add.clear( );
   waters_source.clear( );
   alt_waters_to_add.clear( );
   alt_waters_source.clear( );

   QRegExp rx_expand_mapkey("^(.+)~(.+)~(.*)$");

   for ( map < QString, rotamer >::iterator it = best_fit_rotamer.begin();
          it != best_fit_rotamer.end();
         it++ )
   {
      if ( !to_hydrate.count( it->first ) )
      {
         error_msg = QString( us_tr( "Internal error: best_fit_rotamer key %1 not found in to_hydrate" ) )
            .arg( it->first );
         return false;
      }

      // for each water to add:
      // cout << QString( "need to compute best transform matrix for %1:\n" ).arg( it->first );
      if ( it->second.water_positioning_atoms.size() !=
           it->second.waters.size() )
      {
         error_msg = QString( us_tr( "Internal error: water positioning atom size %1 does not match waters size %2" ) )
            .arg( it->second.water_positioning_atoms.size() )
            .arg( it->second.waters.size() );
         return false;
      }

      for ( unsigned int i = 0; i < (unsigned int) it->second.water_positioning_atoms.size(); i++ )
      {
         // get coordinates of 
         p1.resize( it->second.water_positioning_atoms[ i ].size() );
         p2.resize( it->second.water_positioning_atoms[ i ].size() );
            
         for ( unsigned int j = 0; j < (unsigned int) it->second.water_positioning_atoms[ i ].size(); j++ )
         {
            if ( !it->second.atom_map.count( it->second.water_positioning_atoms[ i ][ j ] ) )
            {
               error_msg = QString( us_tr( "Internal error: water positioning atom %1 not found in atom_map" ) )
                  .arg( it->second.water_positioning_atoms[ i ][ j ] );
               return false;
            }
            p1[ j ] = it->second.atom_map[ it->second.water_positioning_atoms[ i ][ j ] ].coordinate;

            if ( !to_hydrate[ it->first ].count( it->second.water_positioning_atoms[ i ][ j ] ) )
            {
               error_msg = QString( us_tr( "Internal error: water positioning atom %1 not found in to_hydrate atoms" ) )
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
            waters_source[ it->first ].push_back( QString( "Rtmr:%1" ).arg( it->second.name ) );
#if defined( UHH_DEBUG_ALIGN )
            cout << QString( "p0 adding to waters source %1 %2\n" ).arg( it->first ).arg( it->second.name );
#endif
         } else {
            count_waters_not_added++;
#if defined( UHH_DEBUG_ALIGN )
            cout << QString( "p0 skiping due to clash %1 %2\n" ).arg( it->first ).arg( it->second.name );
#endif
         }
      }
   }

   for ( map < QString, vector < rotamer > >::iterator it = pointmap_rotamers.begin();
         it != pointmap_rotamers.end();
         it++ )
   {
      if ( !to_hydrate_pointmaps.count( it->first ) )
      {
         error_msg = QString( us_tr( "Internal error: pointmap_rotamers key %1 not found in to_hydrate_pointmaps" ) )
            .arg( it->first );
         return false;
      }

      // add a waters for each pointmap for this residue
      if ( rx_expand_mapkey.indexIn( it->first ) == -1 )
      {
         error_msg = QString( us_tr( "Internal error: could not expand mapkey %1" ) ).arg( it->first );
         return false;
      }
      QString resName = rx_expand_mapkey.cap( 1 );
      if ( !pointmap_atoms.count( resName ) ||
           !pointmap_atoms_dest.count( resName ) ||
           !pointmap_atoms_ref_residue.count( resName ) )
      {
         error_msg = QString( us_tr( "Internal error: could not find pointmap entries for residue of key %1" ) ).arg( it->first );
         return false;
      }
         
      if ( it->second.size() != pointmap_atoms[ resName ].size() ||
           it->second.size() != pointmap_atoms_dest[ resName ].size() ||
           it->second.size() != pointmap_atoms_ref_residue[ resName ].size() )
      {
         error_msg = QString( us_tr( "Internal error: could not find pointmap size inconsistancy for key %1" ) ).arg( it->first );
         return false;
      }
         
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         // for each water to add:
         // compute transformation matrix from the pointmap_atoms_dest in the pointmap_atom_ref_residue
         // to the pointmap_atoms in the residue and apply to each water in the rotamer
         // the rotamer's atom_map(equivalently side chain) & waters have the "dest" info

         if ( it->second[ i ].residue != pointmap_atoms_ref_residue[ resName ][ i ] )
         {
            error_msg = QString( us_tr( "Internal error: inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" ) )
               .arg( it->first )
               .arg( i )
               .arg( it->second[ i ].residue )
               .arg( pointmap_atoms_ref_residue[ resName ][ i ] );
            return false;
         }

         // build up the transformation

         p1.resize( pointmap_atoms     [ resName ][ i ].size() );
         p2.resize( pointmap_atoms_dest[ resName ][ i ].size() );
         
         if ( p1.size() != p2.size() )
         {
            error_msg = QString( us_tr( "Internal error: size inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" ) )
               .arg( it->first )
               .arg( i )
               .arg( pointmap_atoms[ resName ][ i ].size() )
               .arg( pointmap_atoms_dest[ resName ][ i ].size() );
            return false;
         }

         for ( unsigned int j = 0; j < (unsigned int) p1.size(); j++ )
         {
            // cout << QString("mapping: %1 %2 dest %3\n")
            // .arg(it->first)
            // .arg(i)
            // .arg(pointmap_atoms_dest[ resName ][ i ][ j ] );
            if ( !it->second[ i ].atom_map.count( pointmap_atoms_dest[ resName ][ i ][ j ] ) )
            {
               error_msg = QString( us_tr( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" ) )
                  .arg(  pointmap_atoms_dest[ resName ][ i ][ j ]  );
               return false;
            }
               
            p1[ j ] = it->second[ i ].atom_map[ pointmap_atoms_dest[ resName ][ i ][ j ] ].coordinate;

            if ( !to_hydrate_pointmaps[ it->first ].count( pointmap_atoms[ resName ][ i ][ j ] ) )
            {
               error_msg = QString( us_tr( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" ) )
                  .arg(  pointmap_atoms[ resName ][ i ][ j ] );
               return false;
            }
#if defined( UHHH_DEBUG_ALIGH )
            cout << QString("z1 mapping: %1 %2 source %3\n")
               .arg(it->first)
               .arg(i)
               .arg(pointmap_atoms[ resName ][ i ][ j ] );
#endif

            p2[ j ] = to_hydrate_pointmaps[ it->first ][ pointmap_atoms[ resName ][ i ][ j ] ];

//             cout << QString( "z1  %1 [%2,%3,%4] to [%5,%6,%7]\n" )
//                .arg( it->second.water_positioning_atoms[ i ][ j ] )
//                .arg( p1[ j ].axis[ 0 ] )
//                .arg( p1[ j ].axis[ 1 ] )
//                .arg( p1[ j ].axis[ 2 ] )
//                .arg( p2[ j ].axis[ 0 ] )
//                .arg( p2[ j ].axis[ 1 ] )
//                .arg( p2[ j ].axis[ 2 ] );
         }

         // now we have p1 & p2, apply to all of the waters
         // cout << QString("adding %1 waters for %2 %3\n")
         // .arg( it->second[ i ].waters.size() )
         // .arg( it->first )
         // .arg( resName );

         for ( unsigned int j = 0; j < (unsigned int) it->second[ i ].waters.size(); j++ )
         {
            vector < point > rotamer_waters;
            rotamer_waters.push_back( it->second[ i ].waters[ j ].coordinate );
            vector < point > new_waters;
#if defined( UHH_DEBUG_ALIGN )
            cout << QString( "aligning: " );
            for ( unsigned int k = 0; k < (unsigned int) p1.size(); k++ )
            {
               cout << p1[k] << p2[k] << " ";
            }
            cout << endl;
            cout << QString( "aligning waters: " );
            for ( unsigned int k = 0; k < (unsigned int) rotamer_waters.size(); k++ )
            {
               cout << rotamer_waters[k];
            }
            cout << endl;
#endif
            if ( !atom_align( p1, p2, rotamer_waters, new_waters, error_msg ) )
            {
               return false;
            }
#if defined( UHH_DEBUG_ALIGN )
            cout << QString( "aligning after xfrom waters: " );
            for ( unsigned int k = 0; k < (unsigned int) new_waters.size(); k++ )
            {
               cout << new_waters[k];
            }
            cout << endl << flush;
#endif
            count_waters++;
            if ( !has_steric_clash( new_waters[ 0 ] ) )
            {
               count_waters_added++;
               waters_to_add[ it->first ].push_back( new_waters[ 0 ] );
               waters_source[ it->first ].push_back( QString( "PM:%1" ).arg( it->second[ i ].name ) );
#if defined( UHH_DEBUG_ALIGN )
               cout << QString( "p1 adding to waters source %1 %2\n" ).arg( it->first ).arg( it->second[ i ].name );
#endif
            } else {
               count_waters_not_added++;
#if defined( UHH_DEBUG_ALIGN )
               cout << QString( "p1 skipping due to clash %1 %2\n" ).arg( it->first ).arg( it->second[ i ].name );
               {
                  point p = new_waters[ 0 ];
                  unsigned int i = current_model;
                  double dist_threshold = 1e0 - ( saxs_options.steric_clash_distance / 100e0 );
                  double water_radius   = multi_residue_map.count( "WAT" ) ?
                     residue_list[ multi_residue_map[ "WAT" ][ 0 ] ].r_atom[ 0 ].hybrid.radius : 1.401;

                  // cout << QString( "using %1 for water radius %2\n" ).arg( water_radius ).arg(  multi_residue_map.count( "WAT" ) ? "has WAT" : "no WAT" );

                  // check structure:
                  for (unsigned int j = 0; j < (unsigned int) model_vector[i].molecule.size (); j++) {
                     for (unsigned int k = 0; k < (unsigned int) model_vector[i].molecule[j].atom.size (); k++) {
                        PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         
                        if ( dist( this_atom->coordinate, p ) < ( this_atom->radius + water_radius ) * dist_threshold )
                        {
                           cout << QString( "clash with structure %1 %2 %3 %4" ).arg( dist( this_atom->coordinate, p ) - ( this_atom->radius + water_radius ) * dist_threshold )
                              .arg( this_atom->name ).arg( this_atom->resName ).arg( this_atom->serial ) << this_atom->coordinate << p << endl;
                        }
                     }
                  }

                  // check already added waters:
                  for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
                        it != waters_to_add.end();
                        it++ )
                  {
                     for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
                     {
                        if ( dist( it->second[ i ], p ) <=  2e0 * water_radius * dist_threshold )
                        {
                           cout << QString( "clash with water %1\n" ).arg( dist( it->second[ i ], p ) -  2e0 * water_radius * dist_threshold );
                           cout << it->second[ i ] << p << endl;
                        }
                     }
                  }
               }
#endif
            }
         }
      }
   }
   editor->append( us_tr("Done transforming waters to add to pdb coordinates\n") );
   editor->append( QString( us_tr("%1 waters added. %2 not added due to steric clashes \n") )
                   .arg( count_waters_added ) 
                   .arg( count_waters_not_added ) 
                   );
   editor->append( list_steric_clash        () );
   editor->append( list_steric_clash_recheck( quiet ) );
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
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
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

QString US_Hydrodyn::list_steric_clash()
{
   QString qs = QString( us_tr( "Steric clash tolerance %1%\n" ) ).arg( saxs_options.steric_clash_distance );
   for ( map < QString, unsigned int >::iterator it = steric_clash_summary.begin();
         it != steric_clash_summary.end();
         it++ )
   {
      qs += QString( us_tr( "Steric clashes with %1 : %2\n" ) ).arg( it->first ).arg( it->second );
   }
   return qs;
}

float US_Hydrodyn::min_dist_to_struct_and_waters( point p )
{
   unsigned int i = current_model;
   double dist_threshold = 1e0 - ( saxs_options.steric_clash_distance / 100e0 );
   double water_radius   = multi_residue_map.count( "WAT" ) ?
      residue_list[ multi_residue_map[ "WAT" ][ 0 ] ].r_atom[ 0 ].hybrid.radius : 1.401;
   
   float min_dist = (float)1e30;
   float this_dist;

   // check structure:
   for (unsigned int j = 0; j < (unsigned int) model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < (unsigned int) model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         
         this_dist = dist( this_atom->coordinate, p ) - ( this_atom->radius + water_radius ) * dist_threshold;
         if ( this_dist < 0e0 )
         {
            return this_dist;
         }
         if ( min_dist > this_dist )
         {
            min_dist = this_dist;
         }
      }
   }

   for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
         it != waters_to_add.end();
         it++ )
   {
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         this_dist = dist( it->second[ i ], p ) -  2e0 * water_radius * dist_threshold;
         if ( this_dist < 0e0 )
         {
            return -1.0;
         }
         if ( min_dist > this_dist )
         {
            min_dist = this_dist;
         }
      }
   }
   return min_dist;
}

bool US_Hydrodyn::has_steric_clash( point p, bool summary )
{
   unsigned int i = current_model;
   double dist_threshold = 1e0 - ( saxs_options.steric_clash_distance / 100e0 );
   double water_radius   = multi_residue_map.count( "WAT" ) ?
      residue_list[ multi_residue_map[ "WAT" ][ 0 ] ].r_atom[ 0 ].hybrid.radius : 1.401;

   // cout << QString( "using %1 for water radius %2\n" ).arg( water_radius ).arg(  multi_residue_map.count( "WAT" ) ? "has WAT" : "no WAT" );

   // check structure:
   for (unsigned int j = 0; j < (unsigned int) model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < (unsigned int) model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         
         if ( dist( this_atom->coordinate, p ) < ( this_atom->radius + water_radius ) * dist_threshold )
         {
            if ( summary )
            {
               if ( this_atom->chain == 1 )
               {
                  steric_clash_summary[ "structure side chain" ]++;
               } else {
                  steric_clash_summary[ "structure main chain" ]++;
               }
               steric_clash_summary[ "structure total" ]++;
            }
            return true;
         }
      }
   }

   // check already added waters:
   for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
         it != waters_to_add.end();
         it++ )
   {
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         if ( dist( it->second[ i ], p ) <=  2e0 * water_radius * dist_threshold )
         {
            if ( summary )
            {
               steric_clash_summary[ "other water" ]++;
            }
            return true;
         }
      }
   }
   return false;
}

float US_Hydrodyn::min_dist_to_waters( point p )
{
   // check already added waters:
   float min_dist = (float)1.0e15;
   bool first = true;

   for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
         it != waters_to_add.end();
         it++ )
   {
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         float this_dist = dist( it->second[ i ], p );
         if ( first )
         {
            min_dist = this_dist;
            first = false;
         } else {
            if ( min_dist > this_dist )
            {
               min_dist = this_dist;
            }
         }
      }
   }
   return min_dist;
}


QString US_Hydrodyn::list_steric_clash_recheck( bool quiet )
{
   unsigned int i = current_model;
   double dist_threshold = 1e0 - ( saxs_options.steric_clash_recheck_distance / 100e0 );
   double water_radius   = multi_residue_map.count( "WAT" ) ?
      residue_list[ multi_residue_map[ "WAT" ][ 0 ] ].r_atom[ 0 ].hybrid.radius : 1.401;

   map < QString, unsigned int > steric_clash_recheck_summary;
   hydrate_clash_log.clear( );
   QString hydrate_clash_detail;
   // cout << "steric clash recheck:\n";

   hydrate_clash_detail +=
      "Water,clash,radius water,radius clash,sum radii,distance,overlap,percent overlap,water x,clash x, water y, clash y, water z, clash z, source, source2\n";

   hydrate_clash_map_structure .clear( );
   hydrate_clash_map_rtmr_water.clear( );
   hydrate_clash_map_pm_water  .clear( );

   map < QString, bool > counted;

   for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
         it != waters_to_add.end();
         it++ )
   {
      // cout << QString( "checking steric clash water to add %1\n" ).arg( it->first );
      for ( unsigned int pos = 0; pos < (unsigned int) it->second.size(); pos++ )
      {
         point p = it->second[ pos ];

         // check structure:
         for (unsigned int j = 0; j < (unsigned int) model_vector[i].molecule.size (); j++) {
            for (unsigned int k = 0; k < (unsigned int) model_vector[i].molecule[j].atom.size (); k++) {
               PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
               if ( dist( this_atom->coordinate, p ) < ( this_atom->radius + water_radius ) * dist_threshold )
               {
                  hydrate_clash_map_structure[ 
                                              QString( "%1%2:%3.%4" )
                                              .arg( this_atom->resName )
                                              .arg( this_atom->resSeq )
                                              .arg( this_atom->chainID )
                                              .arg( this_atom->name )
                  ] = true;
                  
                  if ( waters_source[ it->first ][ pos ].startsWith( "PM" ) )
                  {
                     hydrate_clash_map_pm_water[ 
                                                  QString( "%1~%2" )
                                                  .arg( it->first )
                                                  .arg( pos ) 
                     ] = true;
                  } else {
                     hydrate_clash_map_rtmr_water[ 
                                                  QString( "%1~%2" )
                                                  .arg( it->first )
                                                  .arg( pos ) 
                     ] = true;
                  }                     

                  hydrate_clash_log << 
                     QString( "Water %1 number %2 clashes with %3~%4~%5 atom %6 by %7 A or %8%\n" )
                     .arg( it->first )
                     .arg( pos + 1 )
                     .arg( this_atom->resName )
                     .arg( this_atom->resSeq )
                     .arg( this_atom->chainID )
                     .arg( this_atom->name )
                     .arg( ( this_atom->radius + water_radius ) - dist( this_atom->coordinate, p ) )
                     .arg( 100.0 * ( ( this_atom->radius + water_radius ) - dist( this_atom->coordinate, p ) ) 
                           / ( this_atom->radius + water_radius ) )
                     ;
                        
                  hydrate_clash_detail +=
                     QString( "Water:%1~%2,%3~%4~%5~%6," )
                     .arg( it->first )
                     .arg( pos + 1 )
                     .arg( this_atom->resName )
                     .arg( this_atom->resSeq )
                     .arg( this_atom->chainID )
                     .arg( this_atom->name )
                         ;
                  hydrate_clash_detail +=
                     QString( "%1,%2,%3,%4," )
                     .arg( water_radius )
                     .arg( this_atom->radius )
                     .arg( this_atom->radius + water_radius )
                     .arg( dist( this_atom->coordinate, p ) )
                     ;

                  hydrate_clash_detail +=
                     QString( "%1,%2," )
                     .arg( ( this_atom->radius + water_radius ) - dist( this_atom->coordinate, p ) )
                     .arg( 100.0 * ( ( this_atom->radius + water_radius ) - dist( this_atom->coordinate, p ) ) 
                           / ( this_atom->radius + water_radius ) )
                     ;
                     
                  hydrate_clash_detail +=
                     QString( "%1,%2,%3,%4,%5,%6," )
                     .arg( p.axis[ 0 ] )
                     .arg( this_atom->coordinate.axis[ 0 ] )
                     .arg( p.axis[ 1 ] )
                     .arg( this_atom->coordinate.axis[ 1 ] )
                     .arg( p.axis[ 2 ] )
                     .arg( this_atom->coordinate.axis[ 2 ] )
                     ;

                  hydrate_clash_detail += 
                     QString( "%1" )
                     .arg( waters_source[ it->first ][ pos ] );

                  hydrate_clash_detail += "\n";
                        
                  if ( this_atom->chain == 1 )
                  {
                     steric_clash_recheck_summary[ " structure side chain" ]++;
                     if ( !counted.count( QString( "%1~%2" ).arg( it->first ).arg( pos ) ) )
                     {
                        steric_clash_recheck_summary[ "waters with clashes total" ]++;
                        if ( waters_source[ it->first ][ pos ].startsWith( "PM" ) )
                        {
                           steric_clash_recheck_summary[ "waters with clashes (Pointmap)" ]++;
                        } else {
                           steric_clash_recheck_summary[ "waters with clashes (Rotamer)" ]++;
                        }
                        counted[ QString( "%1~%2" ).arg( it->first ).arg( pos ) ] = true;
                     }
                  } else {
                     steric_clash_recheck_summary[ " structure main chain" ]++;
                  }
                  steric_clash_recheck_summary[ " structure total" ]++;
                  steric_clash_recheck_summary[ "structure or other water" ]++;
               }
            }
         }

         // check other added waters:
         // map < QString, vector < point > >::iterator it2 = waters_to_add.begin();
         // while ( *it2 != *it &&
         // it2 != waters_to_add.end() )
         // {
         // it2++;
         // }
         for ( map < QString, vector < point > >::iterator it2 = it;
               it2 != waters_to_add.end();
               it2++ )
         {
            for ( unsigned int j = ( it2 == it ? pos + 1 : 0 ); j < (unsigned int) it2->second.size(); j++ )
            {
               if ( dist( it2->second[ j ], p ) < 2e0 * water_radius * dist_threshold )
               {
                  if ( waters_source[ it->first ][ pos ].startsWith( "PM" ) )
                  {
                     hydrate_clash_map_pm_water[ 
                                                  QString( "%1~%2" )
                                                  .arg( it->first )
                                                  .arg( pos ) 
                     ] = true;
                  } else {
                     hydrate_clash_map_rtmr_water[ 
                                                  QString( "%1~%2" )
                                                  .arg( it->first )
                                                  .arg( pos ) 
                     ] = true;
                  }                     


                  if ( waters_source[ it2->first ][ j ].startsWith( "PM" ) )
                  {
                     hydrate_clash_map_pm_water[ 
                                                QString( "%1~%2" )
                                                .arg( it2->first )
                                                .arg( j ) 
                     ] = true;
                  } else {
                     hydrate_clash_map_rtmr_water[ 
                                                  QString( "%1~%2" )
                                                  .arg( it2->first )
                                                  .arg( j ) 
                     ] = true;
                  }                     

                  hydrate_clash_log << 
                     QString( "Water %1 number %2 clashes with water from %3 number %4 by %5 A or %6%\n" )
                     .arg( it->first )
                     .arg( pos + 1 )
                     .arg( it2->first )
                     .arg( j + 1 )
                     .arg( 2e0 * water_radius -  dist( it2->second[ j ], p ) )
                     .arg( 100.0 * ( 2e0 * water_radius - dist( it2->second[ j ], p ) )
                           / ( 2e0 * water_radius ) )
                     ;

                  hydrate_clash_detail +=
                     QString( "Water:%1~%2,Water:%3~%4," )
                     .arg( it->first )
                     .arg( i + 1 )
                     .arg( it2->first )
                     .arg( j + 1 )
                         ;

                  hydrate_clash_detail +=
                     QString( "%1,%2,%3,%4," )
                     .arg( water_radius )
                     .arg( water_radius )
                     .arg( 2e0 * water_radius )
                     .arg( dist( it2->second[ j ], p ) )
                     ;

                  hydrate_clash_detail +=
                     QString( "%1,%2," )
                     .arg( 2e0 * water_radius -  dist( it2->second[ j ], p ) )
                     .arg( 100.0 * ( 2e0 * water_radius - dist( it2->second[ j ], p ) )
                           / ( 2e0 * water_radius ) )
                     ;
                     
                  hydrate_clash_detail +=
                     QString( "%1,%2,%3,%4,%5,%6," )
                     .arg( p.axis[ 0 ] )
                     .arg( it2->second[ j ].axis[ 0 ] )
                     .arg( p.axis[ 1 ] )
                     .arg( it2->second[ j ].axis[ 1 ] )
                     .arg( p.axis[ 2 ] )
                     .arg( it2->second[ j ].axis[ 2 ] )
                     ;

                  hydrate_clash_detail += 
                     QString( "%1,%2" )
                     .arg( waters_source[ it ->first ][ pos ] )
                     .arg( waters_source[ it2->first ][ j   ] );

                  hydrate_clash_detail += "\n";

                  cout << QString( "Water %1 clashes with water from %2\n" )
                     .arg( it->first )
                     .arg( it2->first );
                  steric_clash_recheck_summary[ " other water" ]++;
                  steric_clash_recheck_summary[ "structure or other water" ]++;
                  if ( !counted.count( QString( "%1~%2" ).arg( it->first ).arg( pos ) ) )
                  {
                     steric_clash_recheck_summary[ "waters with clashes total" ]++;
                     if ( waters_source[ it->first ][ pos ].startsWith( "PM" ) )
                     {
                        steric_clash_recheck_summary[ "waters with clashes (Pointmap)" ]++;
                     } else {
                        steric_clash_recheck_summary[ "waters with clashes (Rotamer)" ]++;
                     }
                     counted[ QString( "%1~%2" ).arg( it->first ).arg( pos ) ] = true;
                  }
                  if ( !counted.count( QString( "%1~%2" ).arg( it2->first ).arg( j ) ) )
                  {
                     steric_clash_recheck_summary[ "waters with clashes total" ]++;
                     if ( waters_source[ it2->first ][ j ].startsWith( "PM" ) )
                     {
                        steric_clash_recheck_summary[ "waters with clashes (Pointmap)" ]++;
                     } else {
                        steric_clash_recheck_summary[ "waters with clashes (Rotamer)" ]++;
                     }
                     counted[ QString( "%1~%2" ).arg( it2->first ).arg( j ) ] = true;
                  }
               }
            }
         }
      }
   }

   QString qs = QString( us_tr( "Steric clash recheck tolerance %1%\n" ) ).arg( saxs_options.steric_clash_recheck_distance );
   for ( map < QString, unsigned int >::iterator it = steric_clash_recheck_summary.begin();
         it != steric_clash_recheck_summary.end();
         it++ )
   {
      qs += QString( us_tr( "Steric recheck clash %1 : %2\n" ) ).arg( it->first ).arg( it->second );
   }

   editor_msg( "blue",
               QString( "Maximum possible water to place %1, degree of hydration %2\n" )
               .arg( hydrate_max_waters_no_asa )
               .arg( QString( "").sprintf( "%.2f", ( double ) hydrate_max_waters_no_asa * 18e0 / model_vector[ current_model ].mw ) )
               +
               QString( "Maximum exposed possible water to place %1, degree of hydration %2\n" )
               .arg( count_waters_added + count_waters_not_added )
               .arg( QString( "" ).sprintf( "%.2f", ( double ) ( count_waters_added + count_waters_not_added )  * 18e0 / model_vector[ current_model ].mw ) )
               + 
               QString( "Waters placed %1, degree of hydration %2\n" )
               .arg( count_waters_added )
               .arg( QString( "" ).sprintf( "%.2f", ( double ) count_waters_added * 18e0 / model_vector[ current_model ].mw ) )
               );

   last_steric_clash_log.clear( );
   last_steric_clash_log <<
      QString( "REMARK MW %1 Daltons\n" ).arg( model_vector[ current_model ].mw );

   last_steric_clash_log <<
      QString( "REMARK Maximum possible water to place         %1   degree of hydration %2\n" )
      .arg( hydrate_max_waters_no_asa )
      .arg( QString( "").sprintf( "%.2f", ( double ) hydrate_max_waters_no_asa * 18e0 / model_vector[ current_model ].mw ) );

   last_steric_clash_log <<
      QString( "REMARK Maximum exposed possible water to place %1   degree of hydration %2\n" )
      .arg( count_waters_added + count_waters_not_added )
      .arg( QString( "" ).sprintf( "%.2f", ( double ) ( count_waters_added + count_waters_not_added )  * 18e0 / model_vector[ current_model ].mw ) );

   last_steric_clash_log <<
      QString( "REMARK Waters placed                           %1   degree of hydration %2\n" )
      .arg( count_waters_added )
      .arg( QString( "" ).sprintf( "%.2f", ( double ) count_waters_added * 18e0 / model_vector[ current_model ].mw ) );

   last_steric_clash_log << 
         QString(
                 "REMARK Hydration of                  %1\n"
                 "REMARK Hydration file                %2\n"
                 "REMARK Steric clash tolerance        %3%\n"
                 )
         .arg( pdb_file )
         .arg( saxs_options.default_rotamer_filename )
         .arg( saxs_options.steric_clash_distance )
         ;
   {
      QString qs2 = "REMARK " + qs;
      qs2.replace( "\n", "\nREMARK " );
      qs2 += "\n";
      last_steric_clash_log << qs2;
   }

   cout << "Steric clash log detail:\n";
   //   cout << hydrate_clash_log.join( "" ) << endl;
   QString fname = 
      somo_dir + SLASH + "tmp" + SLASH + QFileInfo( pdb_file ).baseName() + "_clash.txt";
   fname.replace( "//", "/" );
   QFile f(fname);
   if ( f.open( QIODevice::WriteOnly ) )
   {
      QTextStream ts( &f );
      ts << 
         QString(
                 "Hydration of                  %1\n"
                 "Hydration file                %2\n"
                 "Steric clash tolerance        %3%\n"
                 )
         .arg( pdb_file )
         .arg( saxs_options.default_rotamer_filename )
         .arg( saxs_options.steric_clash_distance )
         ;              
      ts << qs;
      ts << "\nDetailed clash info:\n\n";
      for ( unsigned int i = 0; i < (unsigned int) hydrate_clash_log.size(); i++ )
      {
         ts << hydrate_clash_log[ i ];
      }
      ts << "\nEnd clash info\n";
      qs += QString( us_tr( "Steric clash report in: %1\n" ) ).arg( fname );
      f.close();
      //      if ( !saxs_options.alt_hydration )
      //      {
      if ( !quiet )
      {
         view_file( fname );
      }
      //      }
   } else {
      editor->append( QString( us_tr( "Error: could not create %1" ) ).arg( fname ) );
   }
   
   {
      QString fname = 
         somo_dir + SLASH + "tmp" + SLASH + QFileInfo( pdb_file ).baseName() + "_clash.csv";
      fname.replace( "//", "/" );
      QFile f(fname);
      if ( f.open( QIODevice::WriteOnly ) )
      {
         QTextStream ts( &f );
         ts << hydrate_clash_detail;
         f.close();
         qs += QString( us_tr( "Steric clash report csv in: %1\n" ) ).arg( fname );
      } else {
         editor->append( QString( us_tr( "Error: could not create %1" ) ).arg( fname ) );
      }
   }

   hydrate_clash_log.clear( );
   return qs;
}

bool US_Hydrodyn::write_pdb_with_waters( QString &error_msg, bool quiet )
{
#if defined( OLD_WAY )
   if ( saxs_options.alt_hydration )
   {
      return alt_write_pdb_with_waters( error_msg, quiet );
   }
#endif

   QString fname = pdb_file;
   fname = fname.replace( QRegExp( "(|-(h|H))\\.(pdb|PDB)$" ), "" ) 
      + QString( "_%1-c%2-h%3.pdb" )
      .arg( current_model + 1 )
      .arg( QString( "%1" ).arg( saxs_options.steric_clash_distance ).replace( ".", "_" ) )
      .arg( saxs_options.alt_hydration ? "a" : "" )
      ;

   if ( !overwrite && QFile::exists( fname ) )
   {
      fname = fileNameCheck( fname, 0, this );
   }

   QFile f( fname );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      error_msg = QString( us_tr("can not open file %1 for writing" ) ).arg( fname );
      return false;
   }

   last_hydrated_pdb_header = "";
   last_hydrated_pdb_text   = "";

   last_hydrated_pdb_header +=
      QString( "HEADER  US-SOMO Hydrated pdb file %1\n" ).arg( fname );
   last_hydrated_pdb_text +=
      QString( "MODEL     %1\n" ).arg( current_model + 1 );

   unsigned int i = current_model;

   unsigned int atom_number = 0;
   unsigned int residue_number = 0;
   map < QString, bool > chains_used;

   for (unsigned int j = 0; j < (unsigned int) model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < (unsigned int) model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

         last_hydrated_pdb_text +=
            QString("")
            .sprintf(     
                     "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                     this_atom->serial,
                     this_atom->orgName.toLatin1().data(),
                     this_atom->resName.toLatin1().data(),
                     this_atom->chainID.toLatin1().data(),
                     this_atom->resSeq.toUInt(),
                     this_atom->coordinate.axis[ 0 ],
                     this_atom->coordinate.axis[ 1 ],
                     this_atom->coordinate.axis[ 2 ],
                     this_atom->occupancy,
                     this_atom->tempFactor,
                     this_atom->element.toLatin1().data()
                     );
         chains_used[ this_atom->chainID ] = true;

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
   last_hydrated_pdb_text +=
      "TER\n";

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
      
   // add waters:
   QStringList hydrate_clash_waters_rtmr_list;
   QStringList hydrate_clash_waters_pm_list;

   residue_number = 0;
   atom_number    = 0;

   for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
         it != waters_to_add.end();
         it++ )
   {
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         last_hydrated_pdb_text +=
            QString("")
            .sprintf(     
                     "ATOM  %5d  OW  WAT %1s%4d    %8.3f%8.3f%8.3f  1.00  0.00           O  \n",
                     ++atom_number,
                     chainID.toLatin1().data(),
                     ++residue_number,
                     it->second[ i ].axis[ 0 ],
                     it->second[ i ].axis[ 1 ],
                     it->second[ i ].axis[ 2 ]
                     );
         if ( hydrate_clash_map_rtmr_water.count( QString( "%1~%2" ).arg( it->first ).arg( i ) ) )
         {
            hydrate_clash_waters_rtmr_list << QString( "swh%1:%2" ).arg( residue_number ).arg( chainID );
         }
         if ( hydrate_clash_map_pm_water.count( QString( "%1~%2" ).arg( it->first ).arg( i ) ) )
         {
            hydrate_clash_waters_pm_list << QString( "swh%1:%2" ).arg( residue_number ).arg( chainID );
         }
      }
   }

   last_hydrated_pdb_text +=
      "TER\nENDMDL\n";

   QTextStream ts( &f );
   ts << last_hydrated_pdb_header;
   ts << last_pdb_load_calc_mw_msg.replaceInStrings( "\n", "" ).join( "\nREMARK " ) + "\n";
   for ( int i = 0; i < ( int ) last_steric_clash_log.size(); i++ )
   {
      ts << last_steric_clash_log[ i ];
   }
   ts << last_hydrated_pdb_text;
   ts << "END\n";
   f.close();

   last_hydrated_pdb_name = fname;
   editor->append( QString( us_tr( "File %1 created\n" ) ).arg( fname ) );

   // now rasmol it!
   if ( !quiet ) 
   {
      QString out;
      out = QString( "load %1\ncpk\nselect all\ncolor white\nselect swh\ncolor red\n" ).arg( fname );
      for ( map < QString, bool >::iterator it = hydrate_clash_map_structure.begin();
            it != hydrate_clash_map_structure.end();
            it++ )
      {
         out += QString( "select %1\ncolor cyan\n" ).arg( it->first );
      }
      for ( unsigned int i = 0; i < (unsigned int) hydrate_clash_waters_rtmr_list.size(); i++ )
      {
         out += QString( "select %1\ncolor yellow\n" ).arg( hydrate_clash_waters_rtmr_list[ i ] );
      }
      for ( unsigned int i = 0; i < (unsigned int) hydrate_clash_waters_pm_list.size(); i++ )
      {
         out += QString( "select %1\ncolor green\n" ).arg( hydrate_clash_waters_pm_list[ i ] );
      }

      QString fname = 
         somo_dir + SLASH + "tmp" + SLASH + QFileInfo(last_hydrated_pdb_name).baseName() + ".spt";
      QFile f(fname);
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         editor->append("Error creating file " + fname + "\n");
      } else {
         QTextStream t( &f );
         t << out;
         f.close();
         
         model_viewer( fname, "-script" );
      }
      hydrate_clash_map_structure .clear( );
      hydrate_clash_map_rtmr_water.clear( );
      hydrate_clash_map_pm_water  .clear( );
   }

   return true;
}

bool US_Hydrodyn::selected_models_contain( QString residue )
{
   for ( unsigned int i = 0; i < (unsigned int)lb_model->count(); i++) {
      if ( lb_model->item( i )->isSelected() ) {
         for (unsigned int j = 0; j < (unsigned int) model_vector[i].molecule.size (); j++) {
            for (unsigned int k = 0; k < (unsigned int) model_vector[i].molecule[j].atom.size (); k++) {
               PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
               if ( this_atom->resName == residue )
               {
                  return true;
               }
            }
         }
      }
   }
   return false;
}

QString US_Hydrodyn::validate_pointmap()
{
   // make sure all ref_residue's have a rotamer

   QString out = "";

   map < QString, bool > to_erase;

   for ( map < QString, vector < QString > >::iterator it = pointmap_atoms_ref_residue.begin();
         it != pointmap_atoms_ref_residue.end();
         it++ )
   {
      if ( !to_erase.count( it->first ) )
      {
         if ( !pointmap_atoms     .count( it->first ) ||
              !pointmap_atoms_dest.count( it->first ) ||
              pointmap_atoms[ it->first ]     .size() != it->second.size() ||
              pointmap_atoms_dest[ it->first ].size() != it->second.size() )
         {
            out += QString( "Internal error: inconsistancy with Pointmap %1\n" )
               .arg( it->first );
            to_erase[ it->first ] = true;
         } else {
            for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
            {
               if ( !rotamers.count( it->second[ i ] ) )
               {
                  out += QString( "Pointmap %1 contains invalid target rotamer %2, removing from loaded pointmaps\n" )
                     .arg( it->first )
                     .arg( it->second[ i ] );
                  to_erase[ it->first ] = true;
               } else {
                  if ( !rotamers[ it->second[ i ] ].size() )
                  {
                     out += QString( "Pointmap %1 contains empty target rotamer %2, removing from loaded pointmaps\n" )
                        .arg( it->first )
                        .arg( it->second[ i ] );
                     to_erase[ it->first ] = true;
                  }
               }
            }
         }
      }
   }

   for ( map < QString, vector < vector < QString > > >::iterator it = pointmap_atoms_dest.begin();
         it != pointmap_atoms_dest.end();
         it++ )
   {
      if ( !to_erase.count( it->first ) )
      {
         for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
         {
            for ( unsigned int j = 0; j < (unsigned int) it->second[ i ].size(); j++ )
            {
               if ( !rotamers[ pointmap_atoms_ref_residue[ it->first ][ i ] ][ 0 ].atom_map.count( it->second[ i ][ j ] ) )
               {
                  out += QString( "Pointmap %1 target rotamer %2 has an invalid atom %3 (not found in rotamer), removing from loaded pointmaps\n" )
                     .arg( it->first )
                     .arg( pointmap_atoms_ref_residue[ it->first ][ i ] )
                     .arg( it->second[ i ][ j ] );
                  to_erase[ it->first ] = true;
               }
            }
         }
      }
   }
   
   for ( map < QString, bool >::iterator it = to_erase.begin();
         it != to_erase.end();
         it++ )
   {
      pointmap_atoms            .erase( it->first );
      pointmap_atoms_dest       .erase( it->first );
      pointmap_atoms_ref_residue.erase( it->first );
   }

   if ( !out.isEmpty() )
   {
      editor_msg("red", out);
   }
   return out;
}

#if defined( OLD_WAY )
bool US_Hydrodyn::compute_waters_to_add_alt( QString &error_msg, bool quiet )
{
   editor->append( us_tr("Transforming waters to add to pdb coordinates\n") );
   qApp->processEvents();
   puts("Transforming waters to add to pdb coordinates");

   editor_msg( "blue", us_tr( "NOTICE: only main chain CA-C-N currenly implemented" ) );

   steric_clash_summary.clear( );

   count_waters           = 0;
   count_waters_added     = 0;
   count_waters_not_added = 0;

   vector < point > p1;
   vector < point > p2;
   waters_to_add.clear( );
   waters_source.clear( );
   alt_waters_to_add.clear( );
   alt_waters_source.clear( );

   QRegExp rx_expand_mapkey("^(.+)~(.+)~(.*)$");

   // best fit rotamer loop: skip those with rotated_rotamers

   for ( map < QString, rotamer >::iterator it = best_fit_rotamer.begin();
         it != best_fit_rotamer.end();
         it++ )
   {

      cout << QString( "toh: asa %1 tot=%2 sc=%3 mc=%4\n" )
         .arg( it->first )
         .arg( residue_asa.count( it->first ) ?
               residue_asa[ it->first ] : -9.99 )
         .arg( residue_asa_sc.count( it->first ) ?
               residue_asa_sc[ it->first ] : 0.0 )
         .arg( residue_asa_mc.count( it->first ) ?
               residue_asa_mc[ it->first ] : 0.0 )
         ;

      if ( !to_hydrate.count( it->first ) )
      {
         error_msg = QString( us_tr( "Internal error: best_fit_rotamer key %1 not found in to_hydrate" ) )
            .arg( it->first );
         return false;
      }
      
      if ( rotated_rotamers.count( it->second.name ) )
      {
         cout << QString( "this best fit rotamer %1 is in with rotated rotamers %2\n" ).arg( it->first ).arg( it->second.name );
         continue; // skip this in the first pass
      }

      // for each water to add:
      // cout << QString( "need to compute best transform matrix for %1:\n" ).arg( it->first );
      if ( it->second.water_positioning_atoms.size() !=
           it->second.waters.size() )
      {
         error_msg = QString( us_tr( "Internal error: water positioning atom size %1 does not match waters size %2" ) )
            .arg( it->second.water_positioning_atoms.size() )
            .arg( it->second.waters.size() );
         return false;
      }

      for ( unsigned int i = 0; i < it->second.water_positioning_atoms.size(); i++ )
      {
         // get coordinates of 
         p1.resize( it->second.water_positioning_atoms[ i ].size() );
         p2.resize( it->second.water_positioning_atoms[ i ].size() );
            
         for ( unsigned int j = 0; j < (unsigned int) it->second.water_positioning_atoms[ i ].size(); j++ )
         {
            if ( !it->second.atom_map.count( it->second.water_positioning_atoms[ i ][ j ] ) )
            {
               error_msg = QString( us_tr( "Internal error: water positioning atom %1 not found in atom_map" ) )
                  .arg( it->second.water_positioning_atoms[ i ][ j ] );
               return false;
            }
            p1[ j ] = it->second.atom_map[ it->second.water_positioning_atoms[ i ][ j ] ].coordinate;

            if ( !to_hydrate[ it->first ].count( it->second.water_positioning_atoms[ i ][ j ] ) )
            {
               error_msg = QString( us_tr( "Internal error: water positioning atom %1 not found in to_hydrate atoms" ) )
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
            waters_source[ it->first ].push_back( QString( "Rtmr:%1" ).arg( it->second.name ) );
         } else {
            count_waters_not_added++;
         }
      }
   }

   // pass 1, process all with no "multiple rotated rotamers"
   cout << "alt hydration: pass 1: non-multiple rotated pointmaps rotamers\n";

   for ( map < QString, vector < rotamer > >::iterator it = pointmap_rotamers.begin();
         it != pointmap_rotamers.end();
         it++ )
   {
      cout << QString( "tonmpr: asa %1 tot=%2 sc=%3 mc=%4\n" )
         .arg( it->first )
         .arg( residue_asa.count( it->first ) ?
               residue_asa[ it->first ] : -9.99 )
         .arg( residue_asa_sc.count( it->first ) ?
               residue_asa_sc[ it->first ] : 0.0 )
         .arg( residue_asa_mc.count( it->first ) ?
               residue_asa_mc[ it->first ] : 0.0 )
         ;
      if ( !to_hydrate_pointmaps.count( it->first ) )
      {
         error_msg = QString( us_tr( "Internal error: pointmap_rotamers key %1 not found in to_hydrate_pointmaps" ) )
            .arg( it->first );
         return false;
      }

      // add waters for each pointmap for this residue
      if ( rx_expand_mapkey.indexIn( it->first ) == -1 )
      {
         error_msg = QString( us_tr( "Internal error: could not expand mapkey %1" ) ).arg( it->first );
         return false;
      }
      QString resName = rx_expand_mapkey.cap( 1 );
      if ( !pointmap_atoms.count( resName ) ||
           !pointmap_atoms_dest.count( resName ) ||
           !pointmap_atoms_ref_residue.count( resName ) )
      {
         error_msg = QString( us_tr( "Internal error: could not find pointmap entries for residue of key %1" ) ).arg( it->first );
         return false;
      }
         
      if ( it->second.size() != pointmap_atoms[ resName ].size() ||
           it->second.size() != pointmap_atoms_dest[ resName ].size() ||
           it->second.size() != pointmap_atoms_ref_residue[ resName ].size() )
      {
         error_msg = QString( us_tr( "Internal error: could not find pointmap size inconsistancy for key %1" ) ).arg( it->first );
         return false;
      }
         
      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         // for each water to add:
         // compute transformation matrix from the pointmap_atoms_dest in the pointmap_atom_ref_residue
         // to the pointmap_atoms in the residue and apply to each water in the rotamer
         // the rotamer's atom_map(equivalently side chain) & waters have the "dest" info
         
         // here's the diff:

         rotamer ref_rotamer = it->second[ i ];
#if defined(DEBUG_POINTMAP)
         cout << QString( "Note: in compute_waters_to_add_alt(), current name '%1' has alternates %2\n" )
            .arg( ref_rotamer.name )
            .arg( rotated_rotamers.count( ref_rotamer.name ) ? "yes" : "no" );
#endif

         if ( rotated_rotamers.count( ref_rotamer.name ) )
         {
            continue; // skip this in the first pass
         } else {
            // no multiple rotated rotamers
            if ( ref_rotamer.residue != pointmap_atoms_ref_residue[ resName ][ i ] )
            {
               error_msg = QString( us_tr( "Internal error: inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" ) )
                  .arg( it->first )
                  .arg( i )
                  .arg( ref_rotamer.residue )
                  .arg( pointmap_atoms_ref_residue[ resName ][ i ] );
               return false;
            }
            
            // build up the transformation
            
            p1.resize( pointmap_atoms     [ resName ][ i ].size() );
            p2.resize( pointmap_atoms_dest[ resName ][ i ].size() );
            
            if ( p1.size() != p2.size() )
            {
               error_msg = QString( us_tr( "Internal error: size inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" ) )
                  .arg( it->first )
                  .arg( i )
                  .arg( pointmap_atoms[ resName ][ i ].size() )
                  .arg( pointmap_atoms_dest[ resName ][ i ].size() );
               return false;
            }
            
            for ( unsigned int j = 0; j < (unsigned int) p1.size(); j++ )
            {
               // cout << QString("mapping: %1 %2 dest %3\n")
               // .arg(it->first)
               // .arg(i)
               // .arg(pointmap_atoms_dest[ resName ][ i ][ j ] );
               if ( !ref_rotamer.atom_map.count( pointmap_atoms_dest[ resName ][ i ][ j ] ) )
               {
                  error_msg = QString( us_tr( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" ) )
                     .arg(  pointmap_atoms_dest[ resName ][ i ][ j ]  );
                  return false;
               }
               
               p1[ j ] = ref_rotamer.atom_map[ pointmap_atoms_dest[ resName ][ i ][ j ] ].coordinate;
               
               if ( !to_hydrate_pointmaps[ it->first ].count( pointmap_atoms[ resName ][ i ][ j ] ) )
               {
                  error_msg = QString( us_tr( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" ) )
                     .arg(  pointmap_atoms[ resName ][ i ][ j ] );
                  return false;
               }
#if defined(DEBUG_POINTMAP)
               cout << QString("mapping: %1 %2 source %3\n")
                  .arg(it->first)
                  .arg(i)
                  .arg(pointmap_atoms[ resName ][ i ][ j ] );
#endif
               p2[ j ] = to_hydrate_pointmaps[ it->first ][ pointmap_atoms[ resName ][ i ][ j ] ];
               
#if defined(DEBUG_POINTMAP)
               cout << QString( "  %1 [%2,%3,%4] to [%5,%6,%7]\n" )
                  .arg( it->first )
                  .arg( p1[ j ].axis[ 0 ] )
                  .arg( p1[ j ].axis[ 1 ] )
                  .arg( p1[ j ].axis[ 2 ] )
                  .arg( p2[ j ].axis[ 0 ] )
                  .arg( p2[ j ].axis[ 1 ] )
                  .arg( p2[ j ].axis[ 2 ] );
#endif
            }
            
            // now we have p1 & p2, apply to all of the waters
#if defined(DEBUG_POINTMAP)
            cout << QString("adding %1 waters for %2 %3\n")
               .arg( ref_rotamer.waters.size() )
               .arg( it->first )
               .arg( resName );
#endif
            
            for ( unsigned int j = 0; j < (unsigned int) ref_rotamer.waters.size(); j++ )
            {
               vector < point > rotamer_waters;
               rotamer_waters.push_back( ref_rotamer.waters[ j ].coordinate );
               vector < point > new_waters;
               if ( !atom_align( p1, p2, rotamer_waters, new_waters, error_msg ) )
               {
                  return false;
               }
               count_waters++;
               if ( !has_steric_clash( new_waters[ 0 ] ) )
               {
                  count_waters_added++;
                  waters_to_add[ it->first ].push_back( new_waters[ 0 ] );
                  waters_source[ it->first ].push_back( QString( "PM:%1" ).arg( ref_rotamer.name ) );
               } else {
                  count_waters_not_added++;
               }
            }
         } // end no multiple rotamers
      } // end of search over it->second
   }

   // pass 2, multiple water choices
   // we need to build up a list sorted by % asa ascending possibilities of both pointmap and best fit rotamers
   // then handle them in a uniform way one water at a time
   // for now we are going to compute % asa of complete mc and sc
   // another possiblity would be to compute asa % of mapping atoms for each individual water

   map < QString, vector < vector < vector < point > > > > water_choices;
   map < QString, vector < vector < QString > > >          water_choices_source;
   map < QString, unsigned int > rejected;

   for ( map < QString, vector < rotamer > >::iterator it = pointmap_rotamers.begin();
         it != pointmap_rotamers.end();
         it++ )
   {
      cout << QString( "tompr: asa %1 tot=%2 sc=%3 mc=%4\n" )
         .arg( it->first )
         .arg( residue_asa.count( it->first ) ?
               residue_asa[ it->first ] : -9.99 )
         .arg( residue_asa_sc.count( it->first ) ?
               residue_asa_sc[ it->first ] : 0.0 )
         .arg( residue_asa_mc.count( it->first ) ?
               residue_asa_mc[ it->first ] : 0.0 )
         ;

      if ( !to_hydrate_pointmaps.count( it->first ) )
      {
         error_msg = QString( us_tr( "Internal error: pointmap_rotamers key %1 not found in to_hydrate_pointmaps" ) )
            .arg( it->first );
         return false;
      }

      // add waters for each pointmap for this residue
      if ( rx_expand_mapkey.indexIn( it->first ) == -1 )
      {
         error_msg = QString( us_tr( "Internal error: could not expand mapkey %1" ) ).arg( it->first );
         return false;
      }
      QString resName = rx_expand_mapkey.cap( 1 );
      if ( !pointmap_atoms.count( resName ) ||
           !pointmap_atoms_dest.count( resName ) ||
           !pointmap_atoms_ref_residue.count( resName ) )
      {
         error_msg = QString( us_tr( "Internal error: could not find pointmap entries for residue of key %1" ) ).arg( it->first );
         return false;
      }
         
      if ( it->second.size() != pointmap_atoms[ resName ].size() ||
           it->second.size() != pointmap_atoms_dest[ resName ].size() ||
           it->second.size() != pointmap_atoms_ref_residue[ resName ].size() )
      {
         error_msg = QString( us_tr( "Internal error: could not find pointmap size inconsistancy for key %1" ) ).arg( it->first );
         return false;
      }
         
      water_choices[ it->first ].resize( it->second.size() );
      water_choices_source[ it->first ].resize( it->second.size() );

      for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
      {
         // for each water to add:
         // compute transformation matrix from the pointmap_atoms_dest in the pointmap_atom_ref_residue
         // to the pointmap_atoms in the residue and apply to each water in the rotamer
         // the rotamer's atom_map(equivalently side chain) & waters have the "dest" info
         
         // here's the diff:

         rotamer ref_rotamer = it->second[ i ];
#if defined(DEBUG_POINTMAP)
         cout << QString( "Note: in compute_waters_to_add_alt(), current name '%1' has alternates %2\n" )
            .arg( ref_rotamer.name )
            .arg( rotated_rotamers.count( ref_rotamer.name ) ? "yes" : "no" );
#endif

         if ( rotated_rotamers.count( ref_rotamer.name ) )
         {
            // multiple rotated rotamers
#if defined( NOT_YET )
            bool found_rotated_rotamer_without_clash = false;
            for ( unsigned int k = 0; k < (unsigned int) rotated_rotamers[ it->second[ i ].name ].size(); k++ )
            {
               ref_rotamer = rotated_rotamers[ it->second[ i ].name ][ k ];

               if ( ref_rotamer.residue != pointmap_atoms_ref_residue[ resName ][ i ] )
               {
                  error_msg = QString( us_tr( "Internal error: inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" ) )
                     .arg( it->first )
                     .arg( i )
                     .arg( ref_rotamer.residue )
                     .arg( pointmap_atoms_ref_residue[ resName ][ i ] );
                  return false;
               }
            
               // build up the transformation
               
               p1.resize( pointmap_atoms     [ resName ][ i ].size() );
               p2.resize( pointmap_atoms_dest[ resName ][ i ].size() );
            
               if ( p1.size() != p2.size() )
               {
                  error_msg = QString( us_tr( "Internal error: size inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" ) )
                     .arg( it->first )
                     .arg( i )
                     .arg( pointmap_atoms[ resName ][ i ].size() )
                     .arg( pointmap_atoms_dest[ resName ][ i ].size() );
                  return false;
               }
            
               for ( unsigned int j = 0; j < (unsigned int) p1.size(); j++ )
               {
                  // cout << QString("mapping: %1 %2 dest %3\n")
                  // .arg(it->first)
                  // .arg(i)
                  // .arg(pointmap_atoms_dest[ resName ][ i ][ j ] );
                  if ( !ref_rotamer.atom_map.count( pointmap_atoms_dest[ resName ][ i ][ j ] ) )
                  {
                     error_msg = QString( us_tr( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" ) )
                        .arg(  pointmap_atoms_dest[ resName ][ i ][ j ]  );
                     return false;
                  }
                  
                  p1[ j ] = ref_rotamer.atom_map[ pointmap_atoms_dest[ resName ][ i ][ j ] ].coordinate;
                  
                  if ( !to_hydrate_pointmaps[ it->first ].count( pointmap_atoms[ resName ][ i ][ j ] ) )
                  {
                     error_msg = QString( us_tr( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" ) )
                        .arg(  pointmap_atoms[ resName ][ i ][ j ] );
                     return false;
                  }
#if defined(DEBUG_POINTMAP)
                  cout << QString("mapping: %1 %2 source %3\n")
                     .arg(it->first)
                     .arg(i)
                     .arg(pointmap_atoms[ resName ][ i ][ j ] );
#endif
                  p2[ j ] = to_hydrate_pointmaps[ it->first ][ pointmap_atoms[ resName ][ i ][ j ] ];
               
#if defined(DEBUG_POINTMAP)
                  cout << QString( "  %1 [%2,%3,%4] to [%5,%6,%7]\n" )
                     .arg( it->first )
                     .arg( p1[ j ].axis[ 0 ] )
                     .arg( p1[ j ].axis[ 1 ] )
                     .arg( p1[ j ].axis[ 2 ] )
                     .arg( p2[ j ].axis[ 0 ] )
                     .arg( p2[ j ].axis[ 1 ] )
                     .arg( p2[ j ].axis[ 2 ] );
#endif
               }
               
               // now we have p1 & p2, apply to all of the waters
#if defined(DEBUG_POINTMAP)
               cout << QString("adding %1 waters for %2 %3\n")
                  .arg( ref_rotamer.waters.size() )
                  .arg( it->first )
                  .arg( resName );
#endif
               // this is where we have a difference between !alt & this

               if ( ref_rotamer.waters.size() != 1 )
               {
                  error_msg = QString( us_tr( "Error: currently only one water supported for multiple rotated rotamer waters, found %1 for %2\n" ) )
                     .arg( ref_rotamer.waters.size() )
                     .arg( ref_rotamer.name );
                  return false;
               }
               
               // now we are going to save them all and create a nmr style set of options

               for ( unsigned int j = 0; j < (unsigned int) ref_rotamer.waters.size(); j++ )
               {
                  vector < point > rotamer_waters;
                  rotamer_waters.push_back( ref_rotamer.waters[ j ].coordinate );
                  vector < point > new_waters;
                  if ( !atom_align( p1, p2, rotamer_waters, new_waters, error_msg ) )
                  {
                     return false;
                  }
                  // count_waters++;
                  if ( !has_steric_clash( new_waters[ 0 ] ) )
                  {
                     water_choices[ it->first ][ i ].push_back( new_waters );
                     water_choices_source[ it->first ][ i ].push_back( QString( "PM:%1" ).arg( ref_rotamer.name ) );
                     // count_waters_added++;
                     // waters_to_add[ it->first ].push_back( new_waters[ 0 ] );
                     // waters_source [ it->first ].push_back( QString( "PM:%1" ).arg( ref_rotamer.name ) );
                     found_rotated_rotamer_without_clash = true;
#if defined(DEBUG_POINTMAP_ROTATE)
                     if ( k )
                     {
                        cout << QString( "found clash, resolved with rotamer pos %1\n" ).arg( k );
                     }
#endif
                  } else {
                     rejected[ it->first ]++;
                     // count_waters_not_added++;
                  }
               }
               // if ( found_rotated_rotamer_without_clash )
               // {
               // break;
               // }
            } // each rotated rotamer
            if ( !found_rotated_rotamer_without_clash )
            {
#if defined(DEBUG_POINTMAP_ROTATE)
               cout << QString( "found clash, exhausted rotated rotamers\n" );
#endif
               count_waters_not_added++;
            }
#endif // NOT_YET
         } // if multiple rotated rotamers
      } // end of search over it->second
   }
   // ok, now we potentially have a bunch of choices in water_choices for each water, let's list them:

   cout << "water choices:\n";
   vector < QString > water_choices_v;
   for ( map < QString, vector < vector < vector < point > > > >::iterator it = water_choices.begin();
         it != water_choices.end();
         it++ )
   {
      QString out = QString( "%1 %2 pointmap rotamers rejected %3\n" )
         .arg( it->first )
         .arg( it->second.size() )
         .arg( rejected.count( it->first ) ? rejected[ it->first ] : 0 )
         ;

      for ( int i = 0; i < ( int ) (unsigned int) it->second.size(); i++ )
      {
         if ( it->second[ i ].size() )
         {
            water_choices_v.push_back( QString( "%1:%2" ).arg( it->first ).arg( i ) );
         }
         out += QString( " %1 pr %2 choices %3\n" )
            .arg( it->first )
            .arg( i )
            .arg( it->second[ i ].size() );
         for ( int j = 0; j < ( int ) (unsigned int) it->second[ i ].size(); j++ )
         {
            out += QString( "  %1 pr %2 ch %3 waters %4\n" )
               .arg( it->first )
               .arg( i )
               .arg( j )
               .arg( it->second[ i ][ j ].size() );
         }
      }
      cout << out;
   }

   // now process (make alternate version for each possibility 

   map < QString, vector < point > >  base_waters_to_add        = waters_to_add;
   map < QString, unsigned int >      base_steric_clash_summary = steric_clash_summary;

   alt_waters_to_add                  .clear( );
   alt_steric_clash_summary           .clear( );
   alt_hydrate_clash_map_structure    .clear( );
   alt_hydrate_clash_map_rtmr_water   .clear( );
   alt_hydrate_clash_map_pm_water     .clear( );

   unsigned int base_count_waters           = count_waters;
   unsigned int base_count_waters_added     = count_waters_added;
   unsigned int base_count_waters_not_added = count_waters_not_added;

   vector < unsigned int > alt_count_waters;
   vector < unsigned int > alt_count_waters_added;
   vector < unsigned int > alt_count_waters_not_added;

   
   // linearize for loop over pointmap rotamers
   // since each residue may have multiples

   QRegExp rx_split_key( "^(.+):(\\d+)$" );

   for ( int i = 0; i < ( int ) water_choices_v.size(); i++ )
   {
      if ( rx_split_key.indexIn( water_choices_v[ i ] ) == -1 )
      {
         error_msg = QString( "Internal error: unexpected key in water_choices_v <%1>" ).arg( water_choices_v[ i ] );
         return false;
      }

      QString resname = rx_split_key.cap( 1 );
      int     pm_rot  = rx_split_key.cap( 2 ).toInt();

      // cout << QString( "key <%1>, resname <%2>, count <%3>\n" ).arg( water_choices_v[ i ] ).arg( resname ).arg( pm_rot ) << flush;

      if ( ( int ) water_choices[ resname ].size() <= pm_rot )
      {
         error_msg = QString( "Internal error: unexpected key pos in water_choices_v <%1>" ).arg( water_choices_v[ i ] );
         return false;
      }

      waters_to_add          = base_waters_to_add;
      count_waters           = base_count_waters;
      count_waters_added     = base_count_waters_added;
      count_waters_not_added = base_count_waters_not_added;
      steric_clash_summary   = base_steric_clash_summary;

      // cout << "x2\n" << flush;

      for ( int jj = 0; jj < ( int ) water_choices_v.size(); jj++ )
      {
         int j = ( jj + i + 1 ) % ( int ) water_choices_v.size();

         if ( rx_split_key.indexIn( water_choices_v[ j ] ) == -1 )
         {
            error_msg = QString( "Internal error: unexpected key in water_choices_v 2 <%1>" ).arg( water_choices_v[ j ] );
            return false;
         }

         QString resname = rx_split_key.cap( 1 );
         int     pm_rot  = rx_split_key.cap( 2 ).toInt();

         // cout << QString( "2: key <%1>, resname <%2>, count <%3>\n" ).arg( water_choices_v[ j ] ).arg( resname ).arg( pm_rot ) << flush;

         if ( ( int ) water_choices[ resname ].size() <= pm_rot )
         {
            error_msg = QString( "Internal error: unexpected key pos in water_choices_v 2 <%1>" ).arg( water_choices_v[ j ] );
            return false;
         }

         // add next waters
         if( water_choices[ resname ][ pm_rot ].size() == 1 )
         {
            // only one, no real choice
            // cout << QString( "for key %1: only one, no alternatives available\n" )
            // .arg( water_choices_v[ j ] );
            
            bool any_steric_clash = false;
            for ( int l = 0; l < ( int ) water_choices[ resname ][ pm_rot ][ 0 ].size(); l++ )
            {
               if ( has_steric_clash( water_choices[ resname ][ pm_rot ][ 0 ][ l ] ) )
               {
                  any_steric_clash = true;
                  break;
               }
            }

            for ( int l = 0; l < ( int ) water_choices[ resname ][ pm_rot ][ 0 ].size(); l++ )
            {
               if ( !any_steric_clash )
               {
                  count_waters_added++;
                  waters_to_add[ resname ]
                     .push_back( water_choices[ resname ][ pm_rot ][ 0 ][ l ] );
                  waters_source[ resname ]
                     .push_back( QString( "PM:%1" ).arg( water_choices_source[ resname ][ pm_rot ][ 0 ] ) );
               } else {
                  count_waters_not_added++;
               }
            }
            continue;
         }

         // cout << "x3\n" << flush;

         // find max steric clash distance over choices
         int    best_k = 0;
         float  best_k_dist = 0.0f;
            
         for ( int k = 0; k < ( int ) water_choices[ resname ][ pm_rot ].size(); k++ )
         {
            float this_dist = 0.0f;
            for ( int l = 0; l < ( int ) water_choices[ resname ][ pm_rot ][ k ].size(); l++ )
            {
               // cout << QString( "min dist to waters called k %1 l %2\n" ).arg( k ).arg( l ) << flush;
               float tmp_dist = min_dist_to_waters( water_choices[ resname ][ pm_rot  ][ k ][ l ] );
               // cout << QString( "back min dist to waters called k %1 l %2\n" ).arg( k ).arg( l ) << flush;
               this_dist += tmp_dist * tmp_dist;
            }
            if ( !k )
            {
               best_k_dist = this_dist;
            } else {
               if ( best_k_dist > this_dist )
               {
                  best_k = k;
                  best_k_dist = this_dist;
               }
            }
         }
         
         // cout << "x4\n" << flush;

         bool any_steric_clash = false;
         for ( int l = 0; l < ( int ) water_choices[ resname ][ pm_rot ][ best_k ].size(); l++ )
         {
            if ( has_steric_clash( water_choices[ resname ][ pm_rot ][ best_k ][ l ] ) )
            {
               any_steric_clash = true;
               break;
            }
         }

         // cout << "x5\n" << flush;
         for ( int l = 0; l < ( int ) water_choices[ resname ][ pm_rot ][ best_k ].size(); l++ )
         {
            if ( !any_steric_clash )
            {
               count_waters_added++;
               waters_to_add[ resname ]
                  .push_back( water_choices[ resname ][ pm_rot ][ best_k ][ l ] );
               waters_source[ resname ]
                  .push_back( QString( "PM:%1" ).arg( water_choices_source[ resname ][ pm_rot ][ best_k ] ) );
            } else {
               count_waters_not_added++;
            }
         }
      }
      
      // cout << "x7\n" << flush;

      alt_waters_to_add          .push_back( waters_to_add );
      alt_waters_source          .push_back( waters_source );
      alt_count_waters           .push_back( count_waters );
      alt_count_waters_added     .push_back( count_waters_added );
      alt_count_waters_not_added .push_back( count_waters_not_added );
      alt_steric_clash_summary   .push_back( steric_clash_summary );
   }

   editor->append( us_tr("Done transforming waters to add to pdb coordinates\n") );
   for ( int i = 0; i < ( int ) alt_waters_to_add.size(); i++ )
   {
      editor->append( QString( us_tr("Model %1: %2 waters added. %3 not added due to steric clashes \n") )
                      .arg( i + 1 )
                      .arg( alt_count_waters_added    [ i ] ) 
                      .arg( alt_count_waters_not_added[ i ] ) 
                      );
      steric_clash_summary = alt_steric_clash_summary[ i ];
      waters_to_add        = alt_waters_to_add[ i ];
      editor->append( list_steric_clash        () );
      editor->append( list_steric_clash_recheck( quiet ) );
      alt_hydrate_clash_map_structure .push_back( hydrate_clash_map_structure );
      alt_hydrate_clash_map_rtmr_water.push_back( hydrate_clash_map_rtmr_water );
      alt_hydrate_clash_map_pm_water  .push_back( hydrate_clash_map_pm_water );
   }
   waters_to_add        = base_waters_to_add;
   steric_clash_summary = base_steric_clash_summary;

   qApp->processEvents();
   puts("Done transforming waters to add to pdb coordinates");
   return true;
}

bool US_Hydrodyn::alt_write_pdb_with_waters( QString &error_msg, bool /* quiet */ )
{
   if ( alt_waters_to_add.size() != alt_steric_clash_summary.size() ||
        alt_waters_to_add.size() != alt_waters_source.size() ||
        alt_waters_to_add.size() != alt_hydrate_clash_map_structure.size() ||
        alt_waters_to_add.size() != alt_hydrate_clash_map_rtmr_water.size() ||
        alt_waters_to_add.size() != alt_hydrate_clash_map_pm_water.size() )
   {
      error_msg = QString( "Internal error: vector size mismatch %1 %2 %3 %4 %5 %6" )
         .arg( alt_waters_to_add.size() )
         .arg( alt_steric_clash_summary.size() )
         .arg( alt_waters_source.size() )
         .arg( alt_hydrate_clash_map_structure.size() )
         .arg( alt_hydrate_clash_map_rtmr_water.size() )
         .arg( alt_hydrate_clash_map_pm_water.size() )
         ;
      return false;
   }

   QString fname = pdb_file;
   fname = fname.replace( QRegExp( "(|-(h|H))\\.(pdb|PDB)$" ), "" ) 
      + QString( "_%1-c%2-h%3.pdb" )
      .arg( current_model + 1 )
      .arg( QString( "%1" ).arg( saxs_options.steric_clash_distance ).replace( ".", "_" ) )
      .arg( saxs_options.alt_hydration ? "a" : "" )
      ;
   if ( !overwrite && QFile::exists( fname ) )
   {
      fname = fileNameCheck( fname, 0, this );
   }


   QFile f( fname );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      error_msg = QString( us_tr("can not open file %1 for writing" ) ).arg( fname );
      return false;
   }

   last_hydrated_pdb_header = "";
   last_hydrated_pdb_text   = "";

   last_hydrated_pdb_header +=
      QString( "HEADER  US-SOMO Hydrated pdb file %1 with %2 alternates\n" )
      .arg( fname )
      .arg( alt_waters_to_add.size() );

   vector < QStringList > alt_hydrate_clash_waters_rtmr_list( alt_waters_to_add.size() );
   vector < QStringList > alt_hydrate_clash_waters_pm_list  ( alt_waters_to_add.size() );

   for ( int m = 0; m < ( int ) alt_waters_to_add.size(); m++ )
   {
      last_hydrated_pdb_text +=
         QString( "MODEL     %1\n" ).arg( m + 1 );

      unsigned int i = current_model;
      
      unsigned int atom_number = 0;
      unsigned int residue_number = 0;
      map < QString, bool > chains_used;

      for (unsigned int j = 0; j < (unsigned int) model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < (unsigned int) model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

            last_hydrated_pdb_text +=
               QString("")
               .sprintf(     
                        "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                        this_atom->serial,
                        this_atom->orgName.toLatin1().data(),
                        this_atom->resName.toLatin1().data(),
                        this_atom->chainID.toLatin1().data(),
                        this_atom->resSeq.toUInt(),
                        this_atom->coordinate.axis[ 0 ],
                        this_atom->coordinate.axis[ 1 ],
                        this_atom->coordinate.axis[ 2 ],
                        this_atom->occupancy,
                        this_atom->tempFactor,
                        this_atom->element.toLatin1().data()
                        );
            chains_used[ this_atom->chainID ] = true;

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
      last_hydrated_pdb_text +=
         "TER\n";

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
      
      // add waters:
      
      waters_to_add                = alt_waters_to_add                 [ m ];
      hydrate_clash_map_structure  = alt_hydrate_clash_map_structure   [ m ]; 
      hydrate_clash_map_rtmr_water = alt_hydrate_clash_map_rtmr_water  [ m ]; 
      hydrate_clash_map_pm_water   = alt_hydrate_clash_map_pm_water    [ m ]; 

      for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
            it != waters_to_add.end();
            it++ )
      {
         for ( unsigned int i = 0; i < (unsigned int) it->second.size(); i++ )
         {
            last_hydrated_pdb_text +=
               QString("")
               .sprintf(     
                        "ATOM  %5d  OW  WAT %1s%4d    %8.3f%8.3f%8.3f  1.00  0.00           O  \n",
                        ++atom_number,
                        chainID.toLatin1().data(),
                        ++residue_number,
                        it->second[ i ].axis[ 0 ],
                        it->second[ i ].axis[ 1 ],
                        it->second[ i ].axis[ 2 ]
                        );
            if ( hydrate_clash_map_rtmr_water.count( QString( "%1~%2" ).arg( it->first ).arg( i ) ) )
            {
               alt_hydrate_clash_waters_rtmr_list[ m ] << QString( "swh%1:%2" ).arg( residue_number ).arg( chainID );
            }
            if ( hydrate_clash_map_pm_water.count( QString( "%1~%2" ).arg( it->first ).arg( i ) ) )
            {
               alt_hydrate_clash_waters_pm_list[ m ] << QString( "swh%1:%2" ).arg( residue_number ).arg( chainID );
            }
         }
      }

      last_hydrated_pdb_text +=
         "TER\nENDMDL\n";
   } // for each model

   QTextStream ts( &f );
   ts << last_hydrated_pdb_header;
   ts << last_hydrated_pdb_text;
   ts << "END\n";
   f.close();

   last_hydrated_pdb_name = fname;
   editor->append( QString( us_tr( "File %1 created\n" ) ).arg( fname ) );

   /* no rasmol yet

   // now rasmol it!
   if ( !quiet )
   {
      QString out;
      out = QString( "load %1\ncpk\nselect all\ncolor white\nselect swh\ncolor red\n" ).arg( fname );
      for ( map < QString, bool >::iterator it = hydrate_clash_map_structure.begin();
            it != hydrate_clash_map_structure.end();
            it++ )
      {
         out += QString( "select %1\ncolor cyan\n" ).arg( it->first );
      }
      for ( unsigned int i = 0; i < (unsigned int) hydrate_clash_waters_rtmr_list.size(); i++ )
      {
         out += QString( "select %1\ncolor yellow\n" ).arg( hydrate_clash_waters_rtmr_list[ i ] );
      }
      for ( unsigned int i = 0; i < (unsigned int) hydrate_clash_waters_pm_list.size(); i++ )
      {
         out += QString( "select %1\ncolor green\n" ).arg( hydrate_clash_waters_pm_list[ i ] );
      }

      QString fname = 
         somo_dir + SLASH + "tmp" + SLASH + QFileInfo(last_hydrated_pdb_name).baseName() + ".spt";
      QFile f(fname);
      if ( !f.open( IO_WriteOnly ) )
      {
         editor->append("Error creating file " + fname + "\n");
      } else {
         QTextStream t( &f );
         t << out;
         f.close();
         
         model_viewer( fname, "-script" );
      }
      hydrate_clash_map_structure .clear( );
      hydrate_clash_map_rtmr_water.clear( );
      hydrate_clash_map_pm_water  .clear( );
   }
   */

   return true;
}
#endif


// new way
class sortable_float_qs {
public:
   float             f;
   QString           index;
   bool operator < (const sortable_float_qs& objIn) const
   {
      return ( f < objIn.f );
   }
};

#if !defined( OLD_WAY )
bool US_Hydrodyn::compute_waters_to_add_alt( QString &error_msg, bool quiet )
{
   editor->append( us_tr("Transforming waters to add to pdb coordinates\n") );
   qApp->processEvents();
   puts("Transforming waters to add to pdb coordinates");

   // editor_msg( "blue", us_tr( "NOTICE: only main chain CA-C-N currenly implemented" ) );

   steric_clash_summary.clear( );

   count_waters           = 0;
   count_waters_added     = 0;
   count_waters_not_added = 0;

   vector < point > p1;
   vector < point > p2;
   waters_to_add.clear( );
   waters_source.clear( );
   alt_waters_to_add.clear( );
   alt_waters_source.clear( );

   QRegExp rx_expand_mapkey("^(.+)~(.+)~(.*)$");


   // phase zero: build up list in ASA order and process in that order potentially
   // doing any type of pointmap/best fit etc.

   list < sortable_float_qs > lsf;
   sortable_float_qs          sf;
   vector < QString >         residues_in_order;

   for ( map < QString, float >::iterator it = residue_asa.begin();
         it != residue_asa.end();
         it++ )
   {
      if ( best_fit_rotamer.count( it->first ) ||
           pointmap_rotamers.count( it->first ) )
      {
         sf.f = it->second;
         sf.index = it->first;
         lsf.push_back( sf );
      }
   }

   lsf.sort();

   if ( saxs_options.hydration_rev_asa )
   {
      lsf.reverse();
      editor_msg( "blue", us_tr( "NOTICE: Hydration ASA sort order reversed" ) );
   }

   for ( list < sortable_float_qs >::iterator it = lsf.begin();
         it != lsf.end();
         it++ )
   {
      residues_in_order.push_back( it->index );
   }
   
   // later, we could separate by bead, mc/sc or map reference set of atoms

   progress->setValue( residues_in_order.size() ); progress->setMaximum( residues_in_order.size() * 2 );

   for ( unsigned int i = 0; i < (unsigned int) residues_in_order.size(); i++ )
   {
      
      progress->setValue( residues_in_order.size() + i ); progress->setMaximum( residues_in_order.size() * 2 );
      QString this_residue = residues_in_order[ i ];
      cout << QString( "asa sorted residues %1 asa %2 bfr %3 pm %4\n" )
         .arg( this_residue )
         .arg( residue_asa[ this_residue ] )
         .arg( best_fit_rotamer.count( this_residue ) ? "yes" : "no" )
         .arg( pointmap_rotamers.count( this_residue ) ? "yes" : "no" )
         ;

      // should be do pointmap or best fit first ?
      // probably should add up asa's and compare

      if ( best_fit_rotamer.count( this_residue ) )
      {
         rotamer best_fit = best_fit_rotamer[ this_residue ];
         cout << QString( "best fit rotamer name %1\n" ).arg( best_fit.name );

         if ( !to_hydrate.count( this_residue ) )
         {
            error_msg = QString( us_tr( "Internal error: best_fit_rotamer key %1 not found in to_hydrate" ) )
               .arg( this_residue );
            return false;
         }
      
         if ( best_fit.water_positioning_atoms.size() !=
              best_fit.waters.size() )
         {
            error_msg = QString( us_tr( "Internal error: water positioning atom size %1 does not match waters size %2" ) )
               .arg( best_fit.water_positioning_atoms.size() )
               .arg( best_fit.waters.size() );
            return false;
         }

         if ( rotated_rotamers.count( best_fit.name ) )
         {
            // find best choice
            cout << QString( "this best fit rotamer %1 is in with rotated rotamers %2\n" ).arg( this_residue ).arg( best_fit.name );
            // for each water
            if ( rotated_rotamers[ best_fit.name ].size() != best_fit.water_positioning_atoms.size() )
            {
               error_msg = QString( us_tr( "Internal error: water positioning atom size %1 does not match best fit rotamer size %2" ) )
                  .arg( best_fit.water_positioning_atoms.size() )
                  .arg( best_fit.waters.size() );
               return false;
            }
               
            // for each water:
            for ( unsigned int ii = 0; ii < (unsigned int) best_fit.water_positioning_atoms.size(); ii++ )
            {

               // find clash distance of each one, then exit for now:

               // float        min_dist        = (float)1e30;
               unsigned int min_dist_pos    = 0;
               bool         found_non_clash = false;
               vector < point > min_water;

               // for each rotated rotamer

               for ( unsigned int m = 0; m < (unsigned int) rotated_rotamers[ best_fit.name ][ ii ].size(); m++ )
               {
                  rotamer this_ref_rotamer = rotated_rotamers[ best_fit.name ][ ii ][ m ];

                  // get coordinates of 
                  p1.resize( best_fit.water_positioning_atoms[ ii ].size() );
                  p2.resize( best_fit.water_positioning_atoms[ ii ].size() );

                  if ( best_fit.water_positioning_atoms[ ii ].size() !=
                       this_ref_rotamer.water_positioning_atoms[ 0 ].size() )
                  {
                     error_msg = QString( us_tr( "Internal error: best fit waters count %1 does not match reference %2" ) )
                        .arg( best_fit.water_positioning_atoms[ ii ].size() )
                        .arg( this_ref_rotamer.water_positioning_atoms[ 0 ].size() )
                        ;
                     return false;
                  }
            
                  for ( unsigned int jj = 0; jj < (unsigned int) this_ref_rotamer.water_positioning_atoms[ 0 ].size(); jj++ )
                  {
                     if ( !this_ref_rotamer.atom_map.count( this_ref_rotamer.water_positioning_atoms[ 0 ][ jj ] ) )
                     {
                        error_msg = QString( us_tr( "Internal error: water positioning atom %1 not found in atom_map" ) )
                           .arg( this_ref_rotamer.water_positioning_atoms[ 0 ][ jj ] );
                        return false;
                     }
                     // this must change to the atom map of the reference rotamer
                     p1[ jj ] = this_ref_rotamer.atom_map[ this_ref_rotamer.water_positioning_atoms[ 0 ][ jj ] ].coordinate;

                     if ( !to_hydrate[ this_residue ].count( best_fit.water_positioning_atoms[ ii ][ jj ] ) )
                     {
                        error_msg = QString( us_tr( "Internal error: water positioning atom %1 not found in to_hydrate atoms" ) )
                           .arg( best_fit.water_positioning_atoms[ ii ][ jj ] );
                        return false;
                     }
                     p2[ jj ] = to_hydrate[ this_residue ][ best_fit.water_positioning_atoms[ ii ][ jj ] ];
                  }

                  vector < point > rotamer_waters;
                  // this is probably not correct:
                  rotamer_waters.push_back( this_ref_rotamer.waters[ 0 ].coordinate );
                  vector < point > new_waters;
                  if ( !atom_align( p1, p2, rotamer_waters, new_waters, error_msg ) )
                  {
                     return false;
                  }
                  float this_dist = min_dist_to_struct_and_waters( new_waters[ 0 ] );
                  if ( this_dist > 0 )
                  {
                     // min_dist = this_dist;
                     min_dist_pos = m;
                     found_non_clash = true;
                     min_water = new_waters;
                  }
               }

               count_waters++;
               cout << QString( "Notice: best_fit rotamer %1 water %2 has %3 rotated rotamers and %4\n" )
                  .arg( best_fit.name )
                  .arg( ii )
                  .arg( rotated_rotamers[ best_fit.name ][ ii ].size() )
                  .arg( found_non_clash ? 
                        QString( "the best one is number %1 " ).arg( min_dist_pos ) :
                        QString( "there was a clash at all tries" ) )
                  ;

               if ( found_non_clash && !has_steric_clash( min_water[ 0 ] ) )
               {
                  count_waters_added++;
                  waters_to_add[ this_residue ].push_back( min_water[ 0 ] );
                  waters_source[ this_residue ].push_back( QString( "RtmrMC:%1" ).arg( best_fit.name ) );
               } else {
                  count_waters_not_added++;
               }
            }
         } else {
            // std add one
            cout << QString( "this best fit rotamer %1 is NOT a rotated rotamers %2\n" ).arg( this_residue ).arg( best_fit.name );

            // for each water to add:
            // cout << QString( "need to compute best transform matrix for %1:\n" ).arg( it->first );

            //             if ( best_fit.water_positioning_atoms.size() !=
            //                  best_fit.waters.size() )
            //             {
            //                error_msg = QString( us_tr( "Internal error: water positioning atom size %1 does not match waters size %2" ) )
            //                   .arg( best_fit.water_positioning_atoms.size() )
            //                   .arg( best_fit.waters.size() );
            //                return false;
            //             }

            for ( unsigned int i = 0; i < (unsigned int) best_fit.water_positioning_atoms.size(); i++ )
            {
               // get coordinates of 
               p1.resize( best_fit.water_positioning_atoms[ i ].size() );
               p2.resize( best_fit.water_positioning_atoms[ i ].size() );
            
               for ( unsigned int j = 0; j < (unsigned int) best_fit.water_positioning_atoms[ i ].size(); j++ )
               {
                  if ( !best_fit.atom_map.count( best_fit.water_positioning_atoms[ i ][ j ] ) )
                  {
                     error_msg = QString( us_tr( "Internal error: water positioning atom %1 not found in atom_map" ) )
                        .arg( best_fit.water_positioning_atoms[ i ][ j ] );
                     return false;
                  }
                  p1[ j ] = best_fit.atom_map[ best_fit.water_positioning_atoms[ i ][ j ] ].coordinate;

                  if ( !to_hydrate[ this_residue ].count( best_fit.water_positioning_atoms[ i ][ j ] ) )
                  {
                     error_msg = QString( us_tr( "Internal error: water positioning atom %1 not found in to_hydrate atoms" ) )
                        .arg( best_fit.water_positioning_atoms[ i ][ j ] );
                     return false;
                  }
                  p2[ j ] = to_hydrate[ this_residue ][ best_fit.water_positioning_atoms[ i ][ j ] ];

                  // cout << QString( "  %1 [%2,%3,%4] to [%5,%6,%7]\n" )
                  // .arg( best_fit.water_positioning_atoms[ i ][ j ] )
                  // .arg( p1[ j ].axis[ 0 ] )
                  // .arg( p1[ j ].axis[ 1 ] )
                  // .arg( p1[ j ].axis[ 2 ] )
                  // .arg( p2[ j ].axis[ 0 ] )
                  // .arg( p2[ j ].axis[ 1 ] )
                  // .arg( p2[ j ].axis[ 2 ] );
               }
               vector < point > rotamer_waters;
               rotamer_waters.push_back( best_fit.waters[ i ].coordinate );
               vector < point > new_waters;
               // cout << QString( " and apply it to the point [%1,%2,%3]\n")
               // .arg( best_fit.waters[ i ].coordinate.axis[ 0 ] )
               // .arg( best_fit.waters[ i ].coordinate.axis[ 1 ] )
               // .arg( best_fit.waters[ i ].coordinate.axis[ 2 ] );
               if ( !atom_align( p1, p2, rotamer_waters, new_waters, error_msg ) )
               {
                  return false;
               }
               count_waters++;
               if ( !has_steric_clash( new_waters[ 0 ] ) )
               {
                  cout << QString( "Notice: best fit for %1 has no rotated rotamers and no clash\n" )
                     .arg( best_fit.name );
                  count_waters_added++;
                  waters_to_add[ this_residue ].push_back( new_waters[ 0 ] );
                  waters_source[ this_residue ].push_back( QString( "Rtmr:%1" ).arg( best_fit.name ) );
               } else {
                  cout << QString( "Notice: best fit for %1 has no rotated rotamers and clashes\n" )
                     .arg( best_fit.name );
                  count_waters_not_added++;
               }
            }
         } // end of std non-multiple best_fit_rotamer
      } // end if best_fit_rotamer
      // #define DEBUG_POINTMAP
      if ( pointmap_rotamers.count( this_residue ) )
      {
         if ( !to_hydrate_pointmaps.count( this_residue ) )
         {
            error_msg = QString( us_tr( "Internal error: pointmap_rotamers key %1 not found in to_hydrate_pointmaps" ) )
               .arg( this_residue );
            return false;
         }

         // add waters for each pointmap for this residue
         if ( rx_expand_mapkey.indexIn( this_residue ) == -1 )
         {
            error_msg = QString( us_tr( "Internal error: could not expand mapkey %1" ) ).arg( this_residue );
            return false;
         }
         QString resName = rx_expand_mapkey.cap( 1 );
         if ( !pointmap_atoms.count( resName ) ||
              !pointmap_atoms_dest.count( resName ) ||
              !pointmap_atoms_ref_residue.count( resName ) )
         {
            error_msg = QString( us_tr( "Internal error: could not find pointmap entries for residue of key %1" ) ).arg( this_residue );
            return false;
         }
         
         if ( pointmap_rotamers[ this_residue ].size() != pointmap_atoms[ resName ].size() ||
              pointmap_rotamers[ this_residue ].size() != pointmap_atoms_dest[ resName ].size() ||
              pointmap_rotamers[ this_residue ].size() != pointmap_atoms_ref_residue[ resName ].size() )
         {
            error_msg = QString( us_tr( "Internal error: could not find pointmap size inconsistancy for key %1" ) ).arg( this_residue );
            return false;
         }
         
         // this is for each water
         for ( unsigned int i = 0; i < (unsigned int) pointmap_rotamers[ this_residue ].size(); i++ )
         {
            // for each water to add:
            // compute transformation matrix from the pointmap_atoms_dest in the pointmap_atom_ref_residue
            // to the pointmap_atoms in the residue and apply to each water in the rotamer
            // the rotamer's atom_map(equivalently side chain) & waters have the "dest" info
         
            rotamer ref_rotamer = pointmap_rotamers[ this_residue ][ i ];
#if defined(DEBUG_POINTMAP)
            cout << QString( "Note: in compute_waters_to_add_alt(), current name '%1' has alternates %2\n" )
               .arg( ref_rotamer.name )
               .arg( rotated_rotamers.count( ref_rotamer.name ) ? "yes" : "no" );
#endif

            if ( rotated_rotamers.count( ref_rotamer.name ) )
            {
               // multiple rotated rotamers for pointmap

               if ( rx_expand_mapkey.indexIn( this_residue ) == -1 )
               {
                  error_msg = QString( us_tr( "Internal error: could not expand mapkey %1" ) ).arg( this_residue );
                  return false;
               }

               QString resName = rx_expand_mapkey.cap( 1 );

               if ( !pointmap_atoms.count( resName ) ||
                    !pointmap_atoms_dest.count( resName ) ||
                    !pointmap_atoms_ref_residue.count( resName ) )
               {
                  error_msg = QString( us_tr( "Internal error: could not find pointmap entries for residue of key %1" ) ).arg( resName );
                  return false;
               }
         
               // float        min_dist        = (float)1e30;
               unsigned int min_dist_pos    = 0;
               bool         found_non_clash = false;
               vector < point > min_water;

               // for each rotated rotamer:

               for ( unsigned int k = 0; k < (unsigned int) rotated_rotamers[ ref_rotamer.name ][ i ].size(); k++ )
               {
                  rotamer this_ref_rotamer = rotated_rotamers[ ref_rotamer.name ][ i ][ k ];

                  if ( this_ref_rotamer.residue != pointmap_atoms_ref_residue[ resName ][ i ] )
                  {
                     error_msg = QString( us_tr( "Internal error: inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" ) )
                        .arg( this_residue )
                        .arg( i )
                        .arg( this_ref_rotamer.residue )
                        .arg( pointmap_atoms_ref_residue[ resName ][ i ] );
                     return false;
                  }
            
                  // build up the transformation
                  
                  p1.resize( pointmap_atoms     [ resName ][ i ].size() );
                  p2.resize( pointmap_atoms_dest[ resName ][ i ].size() );
                  
                  if ( p1.size() != p2.size() )
                  {
                     error_msg = QString( us_tr( "Internal error: size inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" ) )
                        .arg( this_residue )
                        .arg( i )
                        .arg( pointmap_atoms[ resName ][ i ].size() )
                        .arg( pointmap_atoms_dest[ resName ][ i ].size() );
                     return false;
                  }
            
                  for ( unsigned int j = 0; j < (unsigned int) p1.size(); j++ )
                  {
                     // cout << QString("mapping: %1 %2 dest %3\n")
                     // .arg(this_residue)
                     // .arg(i)
                     // .arg(pointmap_atoms_dest[ resName ][ i ][ j ] );
                     if ( !this_ref_rotamer.atom_map.count( pointmap_atoms_dest[ resName ][ i ][ j ] ) )
                     {
                        error_msg = QString( us_tr( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" ) )
                           .arg(  pointmap_atoms_dest[ resName ][ i ][ j ]  );
                        return false;
                     }
               
                     p1[ j ] = this_ref_rotamer.atom_map[ pointmap_atoms_dest[ resName ][ i ][ j ] ].coordinate;
               
                     if ( !to_hydrate_pointmaps[ this_residue ].count( pointmap_atoms[ resName ][ i ][ j ] ) )
                     {
                        error_msg = QString( us_tr( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" ) )
                           .arg(  pointmap_atoms[ resName ][ i ][ j ] );
                        return false;
                     }
#if defined(DEBUG_POINTMAP)
                     cout << QString("mapping: %1 %2 source %3\n")
                        .arg(this_residue)
                        .arg(i)
                        .arg(pointmap_atoms[ resName ][ i ][ j ] );
#endif
                     p2[ j ] = to_hydrate_pointmaps[ this_residue ][ pointmap_atoms[ resName ][ i ][ j ] ];
               
#if defined(DEBUG_POINTMAP)
                     cout << QString( "  %1 [%2,%3,%4] to [%5,%6,%7]\n" )
                        .arg( this_residue )
                        .arg( p1[ j ].axis[ 0 ] )
                        .arg( p1[ j ].axis[ 1 ] )
                        .arg( p1[ j ].axis[ 2 ] )
                        .arg( p2[ j ].axis[ 0 ] )
                        .arg( p2[ j ].axis[ 1 ] )
                        .arg( p2[ j ].axis[ 2 ] );
#endif
                  }

                  // expect only one at this time
                  if ( rotated_rotamers[ this_ref_rotamer.name ].size() != 1 )
                  {
                     error_msg = QString( "Error: multiple rotated pointmap rotamer %1 has %2 waters and only one water is currently supported for this scheme " )
                        .arg( this_ref_rotamer.name ).arg( rotated_rotamers[ this_ref_rotamer.name ].size() );
                     return false;
                  }
               

                  for ( unsigned int m = 0; m < (unsigned int) rotated_rotamers[ this_ref_rotamer.name ][ 0 ].size(); m++ )
                  {
                     vector < point > rotamer_waters;
                     // this is probably not correct:
                     rotamer_waters.push_back( rotated_rotamers[ this_ref_rotamer.name ][ 0 ][ m ].waters[ 0 ].coordinate );
                     vector < point > new_waters;
                     if ( !atom_align( p1, p2, rotamer_waters, new_waters, error_msg ) )
                     {
                        return false;
                     }
                     float this_dist = min_dist_to_struct_and_waters( new_waters[ 0 ] );
                     if ( this_dist > 0 )
                     {
                        // min_dist = this_dist;
                        min_dist_pos = m;
                        found_non_clash = true;
                        min_water = new_waters;
                     }
                  }
               }

               count_waters++;

               cout << QString( "Notice: pointmap for %1 has %2 rotated rotamers and %3\n" )
                  .arg( ref_rotamer.name )
                  .arg( rotated_rotamers[ ref_rotamer.name ][ 0 ].size() )
                  .arg( found_non_clash ? 
                        QString( "the best one is number %1 " ).arg( min_dist_pos ) :
                        QString( "there was a clash at all tries" ) )
                  ;

               if ( found_non_clash && !has_steric_clash( min_water[ 0 ] ) )
               {
                  count_waters_added++;
                  waters_to_add[ this_residue ].push_back( min_water[ 0 ] );
                  waters_source[ this_residue ].push_back( QString( "PMmc:%1" ).arg( ref_rotamer.name ) );
               } else {
                  count_waters_not_added++;
               }

            } else {
               // no multiple rotated rotamers

               if ( ref_rotamer.residue != pointmap_atoms_ref_residue[ resName ][ i ] )
               {
                  error_msg = QString( us_tr( "Internal error: inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" ) )
                     .arg( this_residue )
                     .arg( i )
                     .arg( ref_rotamer.residue )
                     .arg( pointmap_atoms_ref_residue[ resName ][ i ] );
                  return false;
               }
            
               // build up the transformation
            
               p1.resize( pointmap_atoms     [ resName ][ i ].size() );
               p2.resize( pointmap_atoms_dest[ resName ][ i ].size() );
            
               if ( p1.size() != p2.size() )
               {
                  error_msg = QString( us_tr( "Internal error: size inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" ) )
                     .arg( this_residue )
                     .arg( i )
                     .arg( pointmap_atoms[ resName ][ i ].size() )
                     .arg( pointmap_atoms_dest[ resName ][ i ].size() );
                  return false;
               }
            
               for ( unsigned int j = 0; j < (unsigned int) p1.size(); j++ )
               {
                  // cout << QString("mapping: %1 %2 dest %3\n")
                  // .arg(this_residue)
                  // .arg(i)
                  // .arg(pointmap_atoms_dest[ resName ][ i ][ j ] );
                  if ( !ref_rotamer.atom_map.count( pointmap_atoms_dest[ resName ][ i ][ j ] ) )
                  {
                     error_msg = QString( us_tr( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" ) )
                        .arg(  pointmap_atoms_dest[ resName ][ i ][ j ]  );
                     return false;
                  }
               
                  p1[ j ] = ref_rotamer.atom_map[ pointmap_atoms_dest[ resName ][ i ][ j ] ].coordinate;
               
                  if ( !to_hydrate_pointmaps[ this_residue ].count( pointmap_atoms[ resName ][ i ][ j ] ) )
                  {
                     error_msg = QString( us_tr( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" ) )
                        .arg(  pointmap_atoms[ resName ][ i ][ j ] );
                     return false;
                  }
#if defined(DEBUG_POINTMAP)
                  cout << QString("mapping: %1 %2 source %3\n")
                     .arg(this_residue)
                     .arg(i)
                     .arg(pointmap_atoms[ resName ][ i ][ j ] );
#endif
                  p2[ j ] = to_hydrate_pointmaps[ this_residue ][ pointmap_atoms[ resName ][ i ][ j ] ];
               
#if defined(DEBUG_POINTMAP)
                  cout << QString( "  %1 [%2,%3,%4] to [%5,%6,%7]\n" )
                     .arg( this_residue )
                     .arg( p1[ j ].axis[ 0 ] )
                     .arg( p1[ j ].axis[ 1 ] )
                     .arg( p1[ j ].axis[ 2 ] )
                     .arg( p2[ j ].axis[ 0 ] )
                     .arg( p2[ j ].axis[ 1 ] )
                     .arg( p2[ j ].axis[ 2 ] );
#endif
               }
            
               // now we have p1 & p2, apply to all of the waters
#if defined(DEBUG_POINTMAP)
               cout << QString("adding %1 waters for %2 %3\n")
                  .arg( ref_rotamer.waters.size() )
                  .arg( this_residue )
                  .arg( resName );
#endif
            
               for ( unsigned int j = 0; j < (unsigned int) ref_rotamer.waters.size(); j++ )
               {
                  vector < point > rotamer_waters;
                  rotamer_waters.push_back( ref_rotamer.waters[ j ].coordinate );
                  vector < point > new_waters;
                  if ( !atom_align( p1, p2, rotamer_waters, new_waters, error_msg ) )
                  {
                     return false;
                  }
                  count_waters++;

                  if ( !has_steric_clash( new_waters[ 0 ] ) )
                  {
                     cout << QString( "Notice: pointmap for %1 has no rotated rotamers and no clash\n" )
                        .arg( ref_rotamer.name );
                     count_waters_added++;
                     waters_to_add[ this_residue ].push_back( new_waters[ 0 ] );
                     waters_source[ this_residue ].push_back( QString( "PM:%1" ).arg( ref_rotamer.name ) );
                  } else {
                     cout << QString( "Notice: pointmap for %1 has no rotated rotamers and has a clash\n" )
                        .arg( ref_rotamer.name );
                     count_waters_not_added++;
                  }
               }
            } // end of else no rotated rotamers
         } // end of search over pointmap_rotamers[ this_residue ]
      } // end if pointmap_rotamers
   }

   editor->append( us_tr("Done transforming waters to add to pdb coordinates\n") );
   editor->append( QString( us_tr("%1 waters added. %2 not added due to steric clashes \n") )
                   .arg( count_waters_added ) 
                   .arg( count_waters_not_added ) 
                   );
   editor->append( list_steric_clash        () );
   editor->append( list_steric_clash_recheck( quiet ) );
   qApp->processEvents();
   puts("Done transforming waters to add to pdb coordinates");
   return true;
}
#endif
