// us_hydrodyn.cpp contains class creation & gui connected functions
#include <QRegularExpression>
// us_hydrodyn_core.cpp contains the main computational routines
// (this) us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o

// includes and defines need cleanup

#include "../include/us_hydrodyn.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
//Added by qt3to4:
#include <QTextStream>


#ifndef WIN32
#   include <unistd.h>
#   define SLASH "/"
#else
#   include <direct.h>
#   include <io.h>
#   define SLASH "\\"
#endif

void US_Hydrodyn::bd_prepare()
{
   bd_ready_to_run = false;
   bd_anaflex_enables(false);

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_somo->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
   pb_grid->setEnabled(false);
   display_default_differences();
   model_vector = model_vector_as_loaded;

   bool any_errors = false;
   bool any_models = false;

   bead_model_suffix = 
      QString("bd-%1-%2-%3")
      .arg(bd_options.threshold_pb_pb)
      .arg(bd_options.threshold_pb_sc)
      .arg(bd_options.threshold_sc_sc);

   le_bead_model_suffix->setText(bead_model_suffix);

   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");

      bd_anaflex_enables(true);

      pb_somo->setEnabled(true);
      pb_grid_pdb->setEnabled(true);
      progress->reset();
      return;
   }

   if (!residue_list.size() ||
       !model_vector.size())
   {
      fprintf(stderr, "calculations can not be run until residue & pdb files are read!\n");
      pb_stop_calc->setEnabled(false);
      return;
   }
   pb_visualize->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);

   somo_processed.resize(lb_model->count());
   bead_models.resize(lb_model->count());
   QString msg = QString("\n%1 models selected:").arg(project);
   for(int i = 0; i < lb_model->count(); i++) {
      somo_processed[i] = 0;
      if ( lb_model->item(i)->isSelected() ) {
         current_model = i;
         msg += QString(" %1").arg(i+1);
      }
   }
   msg += "\n";
   editor->append(msg);


   overlap_reduction save_sidechain_overlap = sidechain_overlap;;
   overlap_reduction save_mainchain_overlap = mainchain_overlap;;
   overlap_reduction save_buried_overlap = buried_overlap;;

   for (current_model = 0; 
        current_model < (unsigned int)lb_model->count(); 
        current_model++)
   {
      if (!any_errors && lb_model->item(current_model)->isSelected())
      {
         any_models = true;
         if(!compute_asa(true))
         {
            somo_processed[current_model] = 1;
            bead_models[current_model] = bead_model;
            QString corr_name = 
               somo_dir + SLASH + "bd" + SLASH + 
               project + QString("_%1").arg(current_model + 1) +
               QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
               + "-bf-main";
            write_corr(corr_name, &bead_models[current_model]);
            // write_bead_model( corr_name + "-A", &bead_models[current_model] );
         }
         else
         {
            any_errors = true;
         }
         write_contact_plot( 
                            somo_dir + SLASH + "bd" + SLASH + project + QString("_%1").arg(current_model + 1) +
                            QString("-pdb_contact-%1.txt").arg(bd_options.threshold_pb_pb)
                            , &model_vector_as_loaded[current_model]
                            , bd_options.threshold_pb_pb );
      }
      if (stopFlag)
      {
         sidechain_overlap = save_sidechain_overlap;
         mainchain_overlap = save_mainchain_overlap;
         buried_overlap = save_buried_overlap;
         editor->append("Stopped by user\n\n");
         bd_anaflex_enables(true);
         pb_somo->setEnabled(true);
         pb_grid_pdb->setEnabled(true);
         progress->reset();
         return;
      }

      // calculate bead model and generate hydrodynamics calculation output
      // if successful, enable follow-on buttons:
   }
   sidechain_overlap = save_sidechain_overlap;
   mainchain_overlap = save_mainchain_overlap;
   buried_overlap = save_buried_overlap;
   if (any_models && !any_errors)
   {
      editor->append("Build bead model for BD completed\n");
      qApp->processEvents();
      compute_bd_connections();
      for (current_model = 0; 
           current_model < (unsigned int)lb_model->count(); 
           current_model++)
      {
         if ( lb_model->item(current_model)->isSelected() )
         {
            QString fname = 
               somo_dir + SLASH + "bd" + SLASH + project + QString("_%1").arg(current_model + 1) +
               QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "");
            write_pdb( fname, &bead_models[current_model] );
            create_browflex_files();
            bd_ready_to_run = true;
            break;
         }
      }
      editor->append("\nBrowflex files created.\n");
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_stop_calc->setEnabled(false);
   pb_somo->setEnabled(true);
   pb_grid_pdb->setEnabled(true);
   pb_grid->setEnabled(true);
   bd_anaflex_enables( ( ( browflex && browflex->state() == QProcess::Running ) ||
                         ( anaflex && anaflex->state() == QProcess::Running ) ) ? false : true );
}

void US_Hydrodyn::bd_run()
{
   run_browflex();
}

// ---------- create browflex files
int US_Hydrodyn::create_browflex_files()
{
   double conv = 1e-8;

   bd_project = project;
   bd_current_model = current_model;

   QString filename = 
      project + QString("_%1").arg(current_model + 1) +
      QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
      + "-bf";
   QString basename = 
      somo_dir + SLASH + "bd" + SLASH + filename;

   // browflex-main.txt
   QFile f;
   // main file
   {
      f.setFileName(basename + "-main.txt");
      if ( !f.open(QIODevice::WriteOnly) )
      {
         editor->append(QString("File write error: can't create %1\n").arg(f.fileName()));
         return -1;
      }
      bd_last_file = f.fileName();
      bd_last_traj_file = somo_dir + SLASH + "bd" + SLASH + filename + "-tra.txt";
      bd_last_molec_file = somo_dir + SLASH + "bd" + SLASH + filename + "-molec.txt";
#if defined(DEBUG_CONN)
      cout << "last tra file " << bd_last_traj_file << endl;
      cout << "last molec file " << bd_last_molec_file << endl;
#endif
      QTextStream ts(&f);
      ts <<
         QString(
                 "%1-log.txt            !logfile\n"
                 "%2-tra.txt            !trajectory file\n"
                 "%3-molec.txt          !molecular file\n"
                 "%4-initc.txt          !initial coords. file\n"
                 "-                                       !Flow file\n"
                 "-                                       !Elec file\n"
                 "-                                       !Wall file\n"
                 "-                                       !Spec file\n"
                 "%5-brown.txt          !Brownian data file\n" 
                 "*\n\n\n"
                 )
         .arg(filename)
         .arg(filename)
         .arg(filename)
         .arg(filename)
         .arg(filename)
         ;
      f.close();
   }
   // molecular file
   {
      f.setFileName(basename + "-molec.txt");
      if ( !f.open(QIODevice::WriteOnly) )
      {
         editor->append(QString("File write error: can't create %1\n").arg(f.fileName()));
         return -1;
      }
      QTextStream ts(&f);
      ts << QString(" %1,   Temperature (C)\n").arg(hydro.temperature);
      ts << QString(" %1,   Solvent viscosity (poise)\n").arg(hydro.solvent_viscosity / 100.0);
      // mass
      double usemass = 0e0;
      if ( hydro.mass_correction )
      {
         usemass = hydro.mass;
      } else {
         for ( unsigned int i = 0; i < bead_models[current_model].size(); i++ )
         {
            if ( bead_models[current_model][i].active )
            {
               usemass += bead_models[current_model][i].bead_ref_mw;
            }
         }
      }
      ts << QString(" %1,   Molecular weight (g/mol)\n").arg(usemass);
      ts << " " + filename + "\n";

      ts << QString(" %1,   Number of beads\n").arg(bead_models[current_model].size());
      for (  unsigned int i = 0; i < bead_models[current_model].size(); i++ )
      {
         ts << QString("  %1\n").arg(bead_models[current_model][i].bead_computed_radius * conv);
      }

      // connectors
      unsigned int connectors = 0;

      QRegularExpression rx(QStringLiteral("^(\\d+)~(\\d+)$"));

      for ( map<QString, bool>::iterator it = connection_active.begin();
            it != connection_active.end();
            it++ )
      {
         if ( connection_active[it->first] )
         {
            connectors++;
         }
      }

      ts << QString(" %1,   Number of connectors\n").arg(connectors);

      for ( map<QString, bool>::iterator it = connection_active.begin();
            it != connection_active.end();
            it++ )
      {
         if ( connection_active[it->first] )
         {
            QRegularExpressionMatch m = rx.match(it->first);
            if ( !m.hasMatch() )
            {
               editor->append("unexpected regexp extract failure (write_browflex_files)!\n");
               return -1;
            }
            int i = m.captured(1).toInt();
            int j = m.captured(2).toInt();
            
            // determine connector type for correct bd_options
            float force_constant;
            float equilibrium_dist;
            float max_elong;

            switch ( (connection_forced[it->first] == 0 ? 0 : 1) * 10 + 
                     connection_pair_type[it->first] )
            {
            case 0: // pb-pb not chemical
               {
                  if ( bd_options.compute_pb_pb_force_constant )
                  {
                     editor->append("compute hookean force constant for pb_pb not implemented!\n");
                     return -1;
                  } else {
                     force_constant = bd_options.pb_pb_force_constant;
                  }
                  if ( bd_options.compute_pb_pb_equilibrium_dist )
                  {
                     equilibrium_dist = connection_dist_stats[it->first][2] * conv;
                  } else {
                     equilibrium_dist = bd_options.pb_pb_equilibrium_dist;
                  }
                  if ( bd_options.compute_pb_pb_max_elong )
                  {
                     max_elong = connection_dist_stats[it->first][1] * conv;
                  } else {
                     max_elong = bd_options.pb_pb_max_elong;
                  }
                  switch ( bd_options.pb_pb_bond_type )
                  {
                  case 0 : // fraenkel (hard hookean)
                     {
                        ts << 
                           QString("%1 %2 %3 %4 %5\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(1)
                           .arg(force_constant)
                           .arg(equilibrium_dist)
                           ;
                     }
                     break;
                  case 1 : // hookean, gaussian (soft)
                     {
                        ts << 
                           QString("%1 %2 %3 %4\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(2)
                           .arg(force_constant)
                           ;
                     }
                     break;
                  case 2 : // fene
                     {
                        ts << 
                           QString("%1 %2 %3 %4 %5\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(3)
                           .arg(force_constant)
                           .arg(max_elong)
                           ;
                     }
                     break;
                  case 3 : // hard-fene
                     {
                        ts <<
                           QString("%1 %2 %3 %4 %5 %6\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(6)
                           .arg(force_constant)
                           .arg(equilibrium_dist)
                           .arg(max_elong)
                           ;
                     }
                     break;
                  default :
                     editor->append("unexpected case type pb-pb (write_browflex_files)!\n");
                     return -1;
                     break;
                  }
               }
               break;
            case 1: // pb-sc not chemical
               {
                  if ( bd_options.compute_pb_sc_force_constant )
                  {
                     editor->append("compute hookean force constant for pb_sc not implemented!\n");
                     return -1;
                  } else {
                     force_constant = bd_options.pb_sc_force_constant;
                  }
                  if ( bd_options.compute_pb_sc_equilibrium_dist )
                  {
                     equilibrium_dist = connection_dist_stats[it->first][2] * conv;
                  } else {
                     equilibrium_dist = bd_options.pb_sc_equilibrium_dist;
                  }
                  if ( bd_options.compute_pb_sc_max_elong )
                  {
                     max_elong = connection_dist_stats[it->first][1] * conv;
                  } else {
                     max_elong = bd_options.pb_sc_max_elong;
                  }
                  switch ( bd_options.pb_sc_bond_type )
                  {
                  case 0 : // fraenkel (hard hookean)
                     {
                        ts << 
                           QString("%1 %2 %3 %4 %5\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(1)
                           .arg(force_constant)
                           .arg(equilibrium_dist)
                           ;
                     }
                     break;
                  case 1 : // hookean, gaussian (soft)
                     {
                        ts << 
                           QString("%1 %2 %3 %4\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(2)
                           .arg(force_constant)
                           ;
                     }
                     break;
                  case 2 : // fene
                     {
                        ts << 
                           QString("%1 %2 %3 %4 %5\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(3)
                           .arg(force_constant)
                           .arg(max_elong)
                           ;
                     }
                     break;
                  case 3 : // hard-fene
                     {
                        ts <<
                           QString("%1 %2 %3 %4 %5 %6\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(6)
                           .arg(force_constant)
                           .arg(equilibrium_dist)
                           .arg(max_elong)
                           ;
                     }
                     break;
                  default :
                     editor->append("unexpected case type pb_sc (write_browflex_files)!\n");
                     return -1;
                     break;
                  }
               }
               break;
            case 2: // sc-sc not chemical
               {
                  if ( bd_options.compute_sc_sc_force_constant )
                  {
                     editor->append("compute hookean force constant for sc_sc not implemented!\n");
                     return -1;
                  } else {
                     force_constant = bd_options.sc_sc_force_constant;
                  }
                  if ( bd_options.compute_sc_sc_equilibrium_dist )
                  {
                     equilibrium_dist = connection_dist_stats[it->first][2] * conv;
                  } else {
                     equilibrium_dist = bd_options.sc_sc_equilibrium_dist;
                  }
                  if ( bd_options.compute_sc_sc_max_elong )
                  {
                     max_elong = connection_dist_stats[it->first][1] * conv;
                  } else {
                     max_elong = bd_options.sc_sc_max_elong;
                  }
                  switch ( bd_options.sc_sc_bond_type )
                  {
                  case 0 : // fraenkel (hard hookean)
                     {
                        ts << 
                           QString("%1 %2 %3 %4 %5\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(1)
                           .arg(force_constant)
                           .arg(equilibrium_dist)
                           ;
                     }
                     break;
                  case 1 : // hookean, gaussian (soft)
                     {
                        ts << 
                           QString("%1 %2 %3 %4\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(2)
                           .arg(force_constant)
                           ;
                     }
                     break;
                  case 2 : // fene
                     {
                        ts << 
                           QString("%1 %2 %3 %4 %5\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(3)
                           .arg(force_constant)
                           .arg(max_elong)
                           ;
                     }
                     break;
                  case 3 : // hard-fene
                     {
                        ts <<
                           QString("%1 %2 %3 %4 %5 %6\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(6)
                           .arg(force_constant)
                           .arg(equilibrium_dist)
                           .arg(max_elong)
                           ;
                     }
                     break;
                  default :
                     editor->append("unexpected case type sc_sc (write_browflex_files)!\n");
                     return -1;
                     break;
                  }
               }
               break;
            case 10: // pb_pb chemical
               {
                  if ( bd_options.compute_chem_pb_pb_force_constant )
                  {
                     editor->append("compute hookean force constant for chem_pb_pb not implemented!\n");
                     return -1;
                  } else {
                     force_constant = bd_options.chem_pb_pb_force_constant;
                  }
                  if ( bd_options.compute_chem_pb_pb_equilibrium_dist )
                  {
                     equilibrium_dist = connection_dist_stats[it->first][2] * conv;
                  } else {
                     equilibrium_dist = bd_options.chem_pb_pb_equilibrium_dist;
                  }
                  if ( bd_options.compute_chem_pb_pb_max_elong )
                  {
                     max_elong = connection_dist_stats[it->first][1] * conv;
                  } else {
                     max_elong = bd_options.chem_pb_pb_max_elong;
                  }
                  switch ( bd_options.chem_pb_pb_bond_type )
                  {
                  case 0 : // fraenkel (hard hookean)
                     {
                        ts << 
                           QString("%1 %2 %3 %4 %5\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(1)
                           .arg(force_constant)
                           .arg(equilibrium_dist)
                           ;
                     }
                     break;
                  case 1 : // hookean, gaussian (soft)
                     {
                        ts << 
                           QString("%1 %2 %3 %4\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(2)
                           .arg(force_constant)
                           ;
                     }
                     break;
                  case 2 : // fene
                     {
                        ts << 
                           QString("%1 %2 %3 %4 %5\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(3)
                           .arg(force_constant)
                           .arg(max_elong)
                           ;
                     }
                     break;
                  case 3 : // hard-fene
                     {
                        ts <<
                           QString("%1 %2 %3 %4 %5 %6\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(6)
                           .arg(force_constant)
                           .arg(equilibrium_dist)
                           .arg(max_elong)
                           ;
                     }
                     break;
                  default :
                     editor->append("unexpected case type chem_pb_pb (write_browflex_files)!\n");
                     return -1;
                     break;
                  }
               }
               break;
            case 11: // pb-sc chemical
               {
                  if ( bd_options.compute_chem_pb_sc_force_constant )
                  {
                     editor->append("compute hookean force constant for chem_pb_sc not implemented!\n");
                     return -1;
                  } else {
                     force_constant = bd_options.chem_pb_sc_force_constant;
                  }
                  if ( bd_options.compute_chem_pb_sc_equilibrium_dist )
                  {
                     equilibrium_dist = connection_dist_stats[it->first][2] * conv;
                  } else {
                     equilibrium_dist = bd_options.chem_pb_sc_equilibrium_dist;
                  }
                  if ( bd_options.compute_chem_pb_sc_max_elong )
                  {
                     max_elong = connection_dist_stats[it->first][1] * conv;
                  } else {
                     max_elong = bd_options.chem_pb_sc_max_elong;
                  }
                  switch ( bd_options.chem_pb_sc_bond_type )
                  {
                  case 0 : // fraenkel (hard hookean)
                     {
                        ts << 
                           QString("%1 %2 %3 %4 %5\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(1)
                           .arg(force_constant)
                           .arg(equilibrium_dist)
                           ;
                     }
                     break;
                  case 1 : // hookean, gaussian (soft)
                     {
                        ts << 
                           QString("%1 %2 %3 %4\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(2)
                           .arg(force_constant)
                           ;
                     }
                     break;
                  case 2 : // fene
                     {
                        ts << 
                           QString("%1 %2 %3 %4 %5\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(3)
                           .arg(force_constant)
                           .arg(max_elong)
                           ;
                     }
                     break;
                  case 3 : // hard-fene
                     {
                        ts <<
                           QString("%1 %2 %3 %4 %5 %6\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(6)
                           .arg(force_constant)
                           .arg(equilibrium_dist)
                           .arg(max_elong)
                           ;
                     }
                     break;
                  default :
                     editor->append("unexpected case type chem_pb_sc (write_browflex_files)!\n");
                     return -1;
                     break;
                  }
               }
               break;
            case 12: // sc-sc chemical
               {
                  if ( bd_options.compute_chem_sc_sc_force_constant )
                  {
                     editor->append("compute hookean force constant for chem_sc_sc not implemented!\n");
                     return -1;
                  } else {
                     force_constant = bd_options.chem_sc_sc_force_constant;
                  }
                  if ( bd_options.compute_chem_sc_sc_equilibrium_dist )
                  {
                     equilibrium_dist = connection_dist_stats[it->first][2] * conv;
                  } else {
                     equilibrium_dist = bd_options.chem_sc_sc_equilibrium_dist;
                  }
                  if ( bd_options.compute_chem_sc_sc_max_elong )
                  {
                     max_elong = connection_dist_stats[it->first][1] * conv;
                  } else {
                     max_elong = bd_options.chem_sc_sc_max_elong;
                  }
                  switch ( bd_options.chem_sc_sc_bond_type )
                  {
                  case 0 : // fraenkel (hard hookean)
                     {
                        ts << 
                           QString("%1 %2 %3 %4 %5\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(1)
                           .arg(force_constant)
                           .arg(equilibrium_dist)
                           ;
                     }
                     break;
                  case 1 : // hookean, gaussian (soft)
                     {
                        ts << 
                           QString("%1 %2 %3 %4\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(2)
                           .arg(force_constant)
                           ;
                     }
                     break;
                  case 2 : // fene
                     {
                        ts << 
                           QString("%1 %2 %3 %4 %5\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(3)
                           .arg(force_constant)
                           .arg(max_elong)
                           ;
                     }
                     break;
                  case 3 : // hard-fene
                     {
                        ts <<
                           QString("%1 %2 %3 %4 %5 %6\n")
                           .arg(i + 1)
                           .arg(j + 1)
                           .arg(6)
                           .arg(force_constant)
                           .arg(equilibrium_dist)
                           .arg(max_elong)
                           ;
                     }
                     break;
                  default :
                     editor->append("unexpected case type chem sc-sc (write_browflex_files)!\n");
                     return -1;
                     break;
                  }
               }
               break;
            default: 
               editor->append("unexpected case type (write_browflex_files)!\n");
               return -1;
               break;
            }
         }
      }
      ts << 
         "0   Number of angles\n"
         "0   Number of torsions\n"
         "0   Number of pairs\n"
         ;
      f.close();
   }
   // initial coordinates file
   {
      f.setFileName(basename + "-initc.txt");
      if ( !f.open(QIODevice::WriteOnly) )
      {
         editor->append(QString("File write error: can't create %1\n").arg(f.fileName()));
         return -1;
      }
      QTextStream ts(&f);
      for ( unsigned int i = 0; i < bead_models[current_model].size(); i++ )
      {
         ts << 
            QString("%1 %2 %3\n")
            .arg(bead_models[current_model][i].bead_coordinate.axis[0] * conv)
            .arg(bead_models[current_model][i].bead_coordinate.axis[1] * conv)
            .arg(bead_models[current_model][i].bead_coordinate.axis[2] * conv)
            ;
      }
      f.close();
   }
   // brownian dynamics file
   {
      f.setFileName(basename + "-brown.txt");
      if ( !f.open(QIODevice::WriteOnly) )
      {
         editor->append(QString("File write error: can't create %1\n").arg(f.fileName()));
         return -1;
      }
      QTextStream ts(&f);
      ts << QString("%1,         mol\n").arg(bd_options.nmol);
      ts << QString("%1,         tprev\n").arg(bd_options.tprev);
      ts << QString("%1,         ttraj\n").arg(bd_options.ttraj);
      ts << QString("%1,         nconf \n").arg(bd_options.nconf);
      ts << QString("%1,         nscreen\n").arg(1);
      ts << QString("%1,         deltat \n").arg(bd_options.deltat);
      ts << QString("%1,         npadif\n").arg(bd_options.npadif);
      ts << QString("%1,         inter\n").arg(bd_options.inter);
      ts << QString("%1,         iorder\n").arg(bd_options.iorder + 1);
      ts << QString("%1,        iseed\n").arg(bd_options.iseed);
      ts << QString("%1,        icdm\n").arg(bd_options.icdm);
      f.close();
   }
   return 0;
}

// ---------- compute connections

// write a pdb file with connections
class sortable_uint {
public:
   unsigned int x;
   bool operator < (const sortable_uint& objIn) const
   {
      return x < objIn.x;
   }
};

int US_Hydrodyn::write_pdb( QString fname, vector < PDB_atom > *model )
{
   QFile fpdb;
   fpdb.setFileName(QString("%1.pdb").arg(fname));
   if ( !fpdb.open(QIODevice::WriteOnly) )
   {
      return -1;
   }
   QTextStream ts(&fpdb);
   ts << 
      QString("HEADER  US-SOMO BD Connection file %1\n").arg(fname);

   // write all the beads

   for ( unsigned int i = 0; i < model->size(); i++ ) 
   {
      if ( 
          (*model)[i].active
          // &&
          // ( bd_options.include_sc ||
          //  (*model)[i].chain == 0 )
          )
      {
         ts << 
             QString::asprintf(      
                     "ATOM   %4d  PB  UNK A%4d    %8.3f%8.3f%8.3f  1.00 10.00           PB  \n",
                     i + 1, i + 1,
                     (*model)[i].bead_coordinate.axis[0],
                     (*model)[i].bead_coordinate.axis[1],
                     (*model)[i].bead_coordinate.axis[2]
                      ) ;
      }
   }

   // consolidate & symmetrize connections

   QRegularExpression rx("^(\\d+)~(\\d+)$");

   sortable_uint tmp_suint_i;
   sortable_uint tmp_suint_j;

   list < sortable_uint > connect_parents;
   map < sortable_uint, bool > parent_pushed;
   map < unsigned int, list < sortable_uint > > connect_list;

   for ( map < QString, bool >::iterator it = connection_active.begin();
         it != connection_active.end();
         it++ )
   {
      if ( connection_active[it->first] ) 
      {
         QRegularExpressionMatch m = rx.match( it->first );
         if ( !m.hasMatch() ) 
         {
            editor->append("unexpected regexp extract failure (write_pdb)!\n");
            return -1;
         }
         tmp_suint_i.x = m.captured(1).toInt();
         tmp_suint_j.x = m.captured(2).toInt();

         connect_list[tmp_suint_i.x].push_back(tmp_suint_j);
         connect_list[tmp_suint_j.x].push_back(tmp_suint_i);

         if ( !parent_pushed.count(tmp_suint_i) ) 
         {
            connect_parents.push_back(tmp_suint_i);
            parent_pushed[tmp_suint_i] = true;
         }
         if ( !parent_pushed.count(tmp_suint_j) ) 
         {
            connect_parents.push_back(tmp_suint_j);
            parent_pushed[tmp_suint_j] = true;
         }

      }
   }
   
   connect_parents.sort();

   for (
        list < sortable_uint > ::const_iterator it = connect_parents.begin(); 
        it != connect_parents.end(); 
        ++it 
        )
   {
      connect_list[it->x].sort();
   }

   for (
        list < sortable_uint > ::const_iterator it = connect_parents.begin(); 
        it != connect_parents.end(); 
        ++it 
        )
   {
      QString cbase =  QString::asprintf( "CONECT%5d",it->x + 1 ) ;
      QString out;
      QString tmp = "";
      unsigned int j = 0;
      for (
           list < sortable_uint > ::const_iterator itj = connect_list[it->x].begin(); 
           itj != connect_list[it->x].end(); 
           ++itj
           )
      {
         tmp +=  QString::asprintf( "%5d",itj->x + 1 ) ;
         if ( !((j + 1) % 4) )
         {
            out += cbase + tmp + "\n";
            tmp = "";
         }
         j++;
      }
      if ( tmp != "" )
      {
         out += cbase + tmp + "\n";
      }
      ts << out;
   }
   ts << "END\n";
   fpdb.close();

   if ( bd_options.show_pdb )
   { // display pdb
      model_viewer( fpdb.fileName() );
   }
   
   return 0;
}

int US_Hydrodyn::compute_bd_connections()
{
   connection_active.clear( );
   connection_dists.clear( );
   connection_dist_stats.clear( );
   connection_pair_type.clear( );
   connection_forced.clear( );

   vector < unsigned int > models_to_proc;

   for ( current_model = 0; 
         current_model < (unsigned int)lb_model->count(); 
         current_model++)
   {
      if ( lb_model->item(current_model)->isSelected() )
      {
         models_to_proc.push_back(current_model);
      }
   }

   if ( !models_to_proc.size() )
   {
      editor->append("No models!\n");
      return -1;
   }

   float d; // distance
      
   // build connection_forced for 1st model

   if ( bd_options.force_chem )
   {
      current_model = models_to_proc[0];

      // force all mc to subsequent mc
      // mc to subsequent sc's

      // pass 1 -  force mc to mc
      for ( unsigned int i = 0; i < bead_models[current_model].size() - 1; i++ ) 
      {
         if ( 
             bead_models[current_model][i].active &&
             bead_models[current_model][i].chain == 0
             )
         {
            for ( unsigned int j = i + 1; j < bead_models[current_model].size(); j++ ) 
            {
               if ( 
                   bead_models[current_model][j].active &&
                   bead_models[current_model][j].chain == 0 
                   )
               {
                  d = dist( bead_models[current_model][i].bead_coordinate,
                            bead_models[current_model][j].bead_coordinate );
                  editor->append(QString("adding forced connection %1 %2\n").arg(i).arg(j));
                  connection_forced[QString("%1~%2").arg(i).arg(j)] = true;
                  connection_active[QString("%1~%2").arg(i).arg(j)] = true;
                  connection_dists[QString("%1~%2").arg(i).arg(j)].push_back(d);
                  break;
               }
            }
         }
      }

      // pass 2 -  force mc to sc
      unsigned int last_i;
      for ( unsigned int i = 0; i < bead_models[current_model].size() - 1; i++ ) 
      {
         if ( 
             bead_models[current_model][i].active &&
             bead_models[current_model][i].chain == 0
             )
         {
            last_i = i;
            // add all sc bead in a subchain until first main chain
            for ( unsigned int j = i + 1; j < bead_models[current_model].size(); j++ ) 
            {
               if ( bead_models[current_model][j].active )
               {
                  if ( bead_models[current_model][j].chain != 0 )
                  {
                     d = dist( bead_models[current_model][last_i].bead_coordinate,
                               bead_models[current_model][j].bead_coordinate );
                     editor->append(QString("adding forced connection %1 %2\n").arg(last_i).arg(j));
                     connection_forced[QString("%1~%2").arg(last_i).arg(j)] = true;
                     connection_active[QString("%1~%2").arg(last_i).arg(j)] = true;
                     connection_dists[QString("%1~%2").arg(last_i).arg(j)].push_back(d);
                     last_i = j;
                  } else {
                     // break at next main chain
                     break;
                  }
               }
            }
         }
      }
   }

   // build connection_active for 1st model
               
   current_model = models_to_proc[0];

   write_contact_plot(
                      somo_dir + SLASH + "bd" + SLASH + project + QString("_%1").arg(current_model + 1) +
                      QString("-bead_contact-%1.txt").arg(bd_options.threshold_pb_pb)
                      , &bead_models[current_model]
                      , bd_options.threshold_pb_pb
                      );

   for ( unsigned int i = 0; i < bead_models[current_model].size() - 1; i++ ) 
   {
      if ( bead_models[current_model][i].active )
      {
         for ( unsigned int j = i + 1; j < bead_models[current_model].size(); j++ ) 
         {
            if ( bead_models[current_model][j].active )
            {
               d = dist( bead_models[current_model][i].bead_coordinate,
                         bead_models[current_model][j].bead_coordinate );

               if ( bead_models[current_model][i].chain == 0 &&
                    bead_models[current_model][j].chain == 0 )
               {
                  connection_pair_type[QString("%1~%2").arg(i).arg(j)] = 0;
               }
               if ( (  bead_models[current_model][i].chain == 0 &&
                       bead_models[current_model][j].chain != 0 ) ||
                    (  bead_models[current_model][i].chain != 0 &&
                       bead_models[current_model][j].chain == 0 ) )
               {
                  connection_pair_type[QString("%1~%2").arg(i).arg(j)] = 1;
               }
               if ( bead_models[current_model][i].chain == 1 &&
                    bead_models[current_model][j].chain == 1 )
               {
                  connection_pair_type[QString("%1~%2").arg(i).arg(j)] = 2;
               }
               if ( 
                   ( // mc - mc
                    connection_pair_type[QString("%1~%2").arg(i).arg(j)] == 0 &&
                    d <= bd_options.threshold_pb_pb 
                    ) 
                   ||
                   ( // mc - sc
                    connection_pair_type[QString("%1~%2").arg(i).arg(j)] == 1 &&
                    d <= bd_options.threshold_pb_sc 
                    )
                   ||
                   ( // sc - sc
                    connection_pair_type[QString("%1~%2").arg(i).arg(j)] == 2 &&
                    d <= bd_options.threshold_sc_sc 
                    )
                   )
               {
                  editor->append(QString("adding connection %1 %2\n").arg(i).arg(j));
                  connection_active[QString("%1~%2").arg(i).arg(j)] = true;
                  connection_dists[QString("%1~%2").arg(i).arg(j)].push_back(d);
               }
            }
         }
      }
   }

   // remove connection_active non-existant in subsequent models

   QRegularExpression rx("^(\\d+)~(\\d+)$");

   for ( unsigned int k = 1; k < models_to_proc.size(); k++ )
   {
      current_model = models_to_proc[k];
      

      for ( map < QString, bool >::iterator it = connection_active.begin();
            it != connection_active.end();
            it++ )
      {
         QRegularExpressionMatch m = rx.match( it->first );
         if ( !m.hasMatch() )
         {
            editor->append("unexpected regexp extract failure (compute_connections)!\n");
            return -1;
         }
         int i = m.captured(1).toInt();
         int j = m.captured(2).toInt();

         if ( 
             !connection_forced.count(QString("%1~%2").arg(i).arg(j)) &&
             (
              !bead_models[current_model][i].active ||
              !bead_models[current_model][j].active 
              )
             )
         {
            d = dist( bead_models[current_model][i].bead_coordinate,
                      bead_models[current_model][j].bead_coordinate );
            if ( 
                ( // mc - mc
                 connection_pair_type[QString("%1~%2").arg(i).arg(j)] == 0 &&
                 d > bd_options.threshold_pb_pb
                 ) 
                ||
                ( // mc - sc
                 connection_pair_type[QString("%1~%2").arg(i).arg(j)] == 1 &&
                 d > bd_options.threshold_pb_sc 
                 )
                ||
                ( // sc - sc
                 connection_pair_type[QString("%1~%2").arg(i).arg(j)] == 2 &&
                 d > bd_options.threshold_sc_sc 
                 )
                )
            {
               editor->append(
                              QString("removing type %1 connection %2 %3\n")
                              .arg(connection_pair_type[QString("%1~%2").arg(i).arg(j)])
                              .arg(i)
                              .arg(j)
                              );
               connection_active[it->first] = false;
            } else {
               connection_dists[QString("%1~%2").arg(i).arg(j)].push_back(d);
            }
         } else {
            connection_dists[QString("%1~%2").arg(i).arg(j)].push_back(d);
         }
      }
   }

   // compute stats

   for ( map < QString, bool >::iterator it = connection_active.begin();
         it != connection_active.end();
         it++ )
   {
      if ( connection_active[it->first] ) 
      {
         connection_dist_stats[it->first].resize(3);
         connection_dist_stats[it->first][0] = FLT_MAX;
         connection_dist_stats[it->first][1] = 0.0;
         connection_dist_stats[it->first][2] = 0.0;
         if ( connection_dists[it->first].size() ) 
         {
            for ( unsigned int i = 0; i < connection_dists[it->first].size(); i++ )
            {
               if ( connection_dists[it->first][i] < connection_dist_stats[it->first][0] )
               {
                  connection_dist_stats[it->first][0] = connection_dists[it->first][i];
               }
               if ( connection_dists[it->first][i] > connection_dist_stats[it->first][1] )
               {
                  connection_dist_stats[it->first][1] = connection_dists[it->first][i];
               }
               connection_dist_stats[it->first][2] += connection_dists[it->first][i];
            }
            connection_dist_stats[it->first][2] /= connection_dists[it->first].size();
         } else {
            editor->append(QString("Error: BD connection %1 has no values!\n")
                           .arg(it->first));
            return -1;
         }
      }
   }

#if defined(DEBUG_CONN)
   for ( map < QString, bool >::iterator it = connection_active.begin();
         it != connection_active.end();
         it++ )
   {
      if ( connection_active[it->first] ) 
      {
         cout << 
            QString("Connection %1 distance min %2 max %3 avg %4 dist:\n")
            .arg(it->first)
            .arg(connection_dist_stats[it->first][0])
            .arg(connection_dist_stats[it->first][1])
            .arg(connection_dist_stats[it->first][2]);

         for ( unsigned int i = 0; i <  connection_dists[it->first].size(); i++ )
         {
            cout << connection_dists[it->first][i] << " ";
         }
         cout << endl;
      }
   }
#endif
         
   return 0;
}
               

// -------------- contact plot stuff --------------------


int US_Hydrodyn::write_contact_plot( QString fname, PDB_model *model, float thresh )
{
   // create a contact plot of each residues c-alpha's
   QFile f(fname);
   if ( !f.open(QIODevice::WriteOnly) )
   {
      return -1;
   }
   QTextStream ts(&f);

   for (unsigned int j = 0; j < model->molecule.size(); j++)
   {
      for (unsigned int k = 0; k < model->molecule[j].atom.size(); k++)
      {
         PDB_atom *this_atom = &(model->molecule[j].atom[k]);
         if ( this_atom->name == "CA" )
         {
            for (unsigned int l = j; l < model->molecule.size(); l++)
            {
               for (unsigned int m = (l == j ? k : 0); m < model->molecule[l].atom.size(); m++)
               {
                  PDB_atom *alt_atom = &(model->molecule[l].atom[m]);
                  if ( alt_atom->name == "CA" )
                  {
                     if (
                         dist ( this_atom->coordinate, alt_atom->coordinate ) <= thresh 
                         )
                     {
                        ts << 
                           QString("%1 %2\n")
                           .arg(this_atom->resSeq)
                           .arg(alt_atom->resSeq);
                     }
                  }
               }
            }
         }
      }
   }
   f.close();
   return 0;
}

int US_Hydrodyn::write_contact_plot( QString fname, vector < PDB_atom > *model, float thresh )
{
   // create a contact plot of the mc beads
   QFile f(fname);
   if ( !f.open(QIODevice::WriteOnly) )
   {
      return -1;
   }
   QTextStream ts(&f);

   // find all the mc beads which are within thresh dist and write out the pairs

   for ( unsigned int i = 0; i < model->size(); i++ ) 
   {
      if ( (*model)[i].chain == 0 ) 
      {
         for ( unsigned int j = i; j < model->size(); j++ ) 
         {
            if ( (*model)[j].chain == 0 ) 
            {
               if (
                   dist ( (*model)[i].bead_coordinate, (*model)[j].bead_coordinate ) <= thresh 
                   )
               {
                  ts << 
                     QString("%1 %2\n")
                     .arg((*model)[i].resSeq)
                     .arg((*model)[j].resSeq);
               }
            }
         }
      }
   }
   f.close();
   return 0;
}

// ----------  compute normals for peptide bond routines

static vector < vector < unsigned int > > pb_list;  // filters relevant atoms to molecule#, atom#
static vector < vector < unsigned int > > pb_base_list;  // gives offsets into pb_list for pbs

// cross products:
// (ca - c) x (o - c)  : normal(o, c, ca)
// (o - c) x (n - c)   : normal(n, c, o)
// (n - c) x (ca - c)  : normal(ca, c, n)
// (ca - n) x (o - n)  : normal(o, n, ca)
// etc... should all be the same handedness (in direction of sc)

point US_Hydrodyn::minus( point p1, point p2 ) // p1 - p2
{
   point result;
   result.axis[0] = p1.axis[0] - p2.axis[0];
   result.axis[1] = p1.axis[1] - p2.axis[1];
   result.axis[2] = p1.axis[2] - p2.axis[2];
   return result;
}

point US_Hydrodyn::cross( point p1, point p2) // p1 cross p2
{
   point result;
   result.axis[0] = p1.axis[1] * p2.axis[2] -  p1.axis[2] * p2.axis[1];
   result.axis[1] = p1.axis[2] * p2.axis[0] -  p1.axis[0] * p2.axis[2];
   result.axis[2] = p1.axis[0] * p2.axis[1] -  p1.axis[1] * p2.axis[0];
   return result;
}

float US_Hydrodyn::dot( point p1, point p2) // p1 dot p2
{
   return 
      p1.axis[0] * p2.axis[0] +
      p1.axis[1] * p2.axis[1] +
      p1.axis[2] * p2.axis[2];
}

point US_Hydrodyn::normal( point p1 )
{
   point result = p1;
   float divisor = sqrt( result.axis[0] * result.axis[0] +
                         result.axis[1] * result.axis[1] +
                         result.axis[2] * result.axis[2] );
   result.axis[0] /= divisor;
   result.axis[1] /= divisor;
   result.axis[2] /= divisor;
   return result;
}   

float US_Hydrodyn::dist( point p1, point p2) // sqrt( (p1 - p2) dot (p1 - p2) )
{
   point p = minus( p1, p2 );
   return sqrt( dot( p, p ) );
}

point US_Hydrodyn::plane( PDB_atom *a1, PDB_atom *a2, PDB_atom *a3 )
{
   point result = normal ( cross(
                                 minus(a3->coordinate, a2->coordinate),
                                 minus(a1->coordinate, a2->coordinate) ) );
   return result;
}

point US_Hydrodyn::plane( point p1, point p2, point p3 )
{
   point result = normal ( cross(
                                 minus( p3, p2 ),
                                 minus( p1, p2 )
                                 ) );
   return result;
}

point US_Hydrodyn::average( vector < point > *v )
{
   point result = (*v)[0];
   for ( unsigned int i = 1; i < v->size(); i++ )
   {
      result.axis[0] += (*v)[i].axis[0];
      result.axis[1] += (*v)[i].axis[1];
      result.axis[2] += (*v)[i].axis[2];
   }
   result.axis[0] /= (float) v->size();
   result.axis[1] /= (float) v->size();
   result.axis[2] /= (float) v->size();
   return result;
}

int US_Hydrodyn::build_pb_structures( PDB_model *model )
{
   // identify the pb atoms
   // skip 1st N

   // create vector of possibilities

   pb_list.clear( );
   pb_list.resize(model->molecule.size());
   pb_base_list.clear( );
   pb_base_list.resize(model->molecule.size());

   for (unsigned int j = 0; j < model->molecule.size(); j++)
   {
      bool first_N_found = false;
      map < QString, int > pb_atoms;
      QString lastResSeq = "";
      for ( unsigned int k = 0; k < model->molecule[j].atom.size(); k++ )
      {
         PDB_atom *this_atom = &(model->molecule[j].atom[k]);
         if ( !first_N_found &&
              this_atom->name == "N" )
         {
            first_N_found = true;
            lastResSeq = this_atom->resSeq;
            continue;
         }
         if ( this_atom->name.contains(QRegularExpression( QStringLiteral( "^(N|CA|C|O)$" ) )) )
         {
            pb_atoms[this_atom->name] = k;
         }

         if ( lastResSeq != this_atom->resSeq )
         {
            // new residue
            if ( !pb_atoms.count("CA") ||
                 !pb_atoms.count("C") ||
                 !pb_atoms.count("O") ||
                 !pb_atoms.count("N") )
            {
               editor->append("Couldn't find CA,C,O,N\n");
               return -1;
            }
            pb_base_list[j].push_back(pb_list[j].size());
            pb_list[j].push_back(pb_atoms["CA"]);
            pb_list[j].push_back(pb_atoms["C"]);
            pb_list[j].push_back(pb_atoms["O"]);
            pb_list[j].push_back(pb_atoms["N"]);
            pb_atoms.clear( );
            lastResSeq = this_atom->resSeq;
         }
      }
      if ( pb_atoms.count("CA") &&
           pb_atoms.count("C") &&
           pb_atoms.count("O") )
      {
         pb_base_list[j].push_back(pb_list[j].size());
         pb_list[j].push_back(pb_atoms["CA"]);
         pb_list[j].push_back(pb_atoms["C"]);
         pb_list[j].push_back(pb_atoms["O"]);
      }
      
   }

#if defined(DEBUG_CONN)
   for (unsigned int j = 0; j < pb_list.size(); j++ )
   {
      for ( unsigned int k = 0; k < pb_list[j].size(); k++ ) {
         PDB_atom *this_atom = &(model->molecule[j].atom[pb_list[j][k]]);
         cout << 
            QString("pb_atom %1 %2 %3 %4 %5\n")
            .arg(this_atom->serial)
            .arg(this_atom->name)
            .arg(this_atom->resName)
            .arg(this_atom->chainID)
            .arg(this_atom->resSeq);
      }
   }
#endif
   for (unsigned int j = 0; j < pb_base_list.size(); j++ )
   {
      for ( unsigned int k = 0; k < pb_base_list[j].size(); k++ ) {
         unsigned int p = pb_base_list[j][k];
#if defined(DEBUG_CONN)
         PDB_atom *this_atom = &(model->molecule[j].atom[pb_list[j][p]]);
         cout << 
            QString("base pb_atom %1 %2 %3 %4 %5\n")
            .arg(this_atom->serial)
            .arg(this_atom->name)
            .arg(this_atom->resName)
            .arg(this_atom->chainID)
            .arg(this_atom->resSeq);
#endif
         vector < point > v;
         point cross1 = plane(&(model->molecule[j].atom[pb_list[j][p+2]]),  // O
                              &(model->molecule[j].atom[pb_list[j][p+1]]),  // C
                              &(model->molecule[j].atom[pb_list[j][p+0]])); // CA
         v.push_back(cross1);
#if defined(DEBUG_CONN)
         cout << "(ca - c) x (o - c): " << cross1 << endl;
#endif
         if ( p + 3 < pb_list[j].size() )
         {
            // we have the n, can compute multiple crosses
            point cross2 = plane(&(model->molecule[j].atom[pb_list[j][p+3]]),  // N
                                 &(model->molecule[j].atom[pb_list[j][p+1]]),  // C
                                 &(model->molecule[j].atom[pb_list[j][p+2]])); // O

            point cross3 = plane(&(model->molecule[j].atom[pb_list[j][p+0]]),  // CA
                                 &(model->molecule[j].atom[pb_list[j][p+1]]),  // C
                                 &(model->molecule[j].atom[pb_list[j][p+3]])); // N
            

            point cross4 = plane(&(model->molecule[j].atom[pb_list[j][p+2]]),  // O
                                 &(model->molecule[j].atom[pb_list[j][p+3]]),  // N
                                 &(model->molecule[j].atom[pb_list[j][p+0]])); // CA
#if defined(DEBUG_CONN)
            cout << "(o - c) x (n - c): " << cross2 << endl;
            cout << "(n - c) x (ca - c): " << cross3 << endl;
            cout << "(ca - n) x (o - n): " << cross4 << endl;
#endif
            v.push_back(cross2);
            v.push_back(cross3);
            v.push_back(cross4);
#if defined(DEBUG_CONN)
            cout << "average: " << average(&v) << endl;
#endif
         }

         // compute angles between C-N-CA

#if defined(DEBUG_CONN)
         if ( k ) 
         {
            float angle = 
               (180.0 / 3.14159265) *
               acos(
                    dot( 
                        normal( 
                               minus ( 
                                      model->molecule[j].atom[pb_list[j][p-3]].coordinate, // C (prior)
                                      model->molecule[j].atom[pb_list[j][p-1]].coordinate  // N (prior)
                                      ) 
                               ) ,
                        normal( 
                               minus ( 
                                      model->molecule[j].atom[pb_list[j][p+0]].coordinate, // CA 
                                      model->molecule[j].atom[pb_list[j][p-1]].coordinate  // N (prior)
                                      ) 
                               ) 
                        )
                   );

            cout << "(c - n) dot (ca - n): "
                 << angle
                 << endl;
         }
#endif
      }
   }
   
   return 0;
}

int US_Hydrodyn::compute_pb_normals()
{
   // compute for selected models
   // 
   QString error_string = "";
   progress->reset();
   editor->append(QString("\nChecking PDB to compute normals: %1 model %2\n").arg(project).arg(current_model+1));
   editor->append("Checking the pdb structure\n");
   if (check_for_missing_atoms(&error_string, &model_vector[current_model])) {
      editor->append("Encountered the following errors with your PDB structure:\n" +
                     error_string);
      printError("Encountered errors with your PDB structure:\n"
                 "please check the text window");
      return -1;
   }
   editor->append("Model ok.\n");

   build_pb_structures( &model_vector[current_model] );
   return 0;
}

// ------------- run browflex --------------

int US_Hydrodyn::run_browflex()
{
   // possible setup a new text window for the browflex runs?
   QString prog = 
      USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "/bin/"
#endif
      + SLASH
      + "browflex2a2-"
#if defined(WIN32)
      + "msd"
#else
      + "lnx"
#endif
      + ".exe";

   {
      QFileInfo qfi(prog);
      if ( !qfi.exists() )
      {
         editor_msg("red", QString("Browflex program '%1' does not exist\n").arg(prog));
         return -1;
      }
      if ( !qfi.isExecutable() )
      {
         editor_msg("red", QString("Browflex program '%1' is not executable\n").arg(prog));
         return -1;
      }
   }

   QFileInfo fi(bd_last_file);
   QString browfile = fi.fileName();
      //      project + QString("_%1").arg(current_model + 1) +
      //      QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
      //      + "-bf-main.txt\n" ;
   QString dir = fi.path();

#if defined(DEBUG_CONN)
   cout << QString("run browflex dir <%1> prog <%2> stdin <%3>\n")
      .arg(dir)
      .arg(prog)
      .arg(browfile);
#endif
   browflex = new QProcess( this );
   browflex->setWorkingDirectory( dir );
#if QT_VERSION < 0x04000
   browflex->addArgument( prog );
#else
   QStringList args;
   args
      << "<"
      << browfile
      ;
#endif
   
   connect( browflex, SIGNAL(readyReadStandardOutput()), this, SLOT(browflex_readFromStdout()) );
   connect( browflex, SIGNAL(readyReadStandardError()), this, SLOT(browflex_readFromStderr()) );
   connect( browflex, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(browflex_finished( int, QProcess::ExitStatus )) );
   connect( browflex, SIGNAL(started()), this, SLOT(browflex_started()) );

   editor->append("\n\nStarting Browflex\n");
#if QT_VERSION < 0x040000
   browflex->launch( browfile );
#else
   browflex->start( prog, args, QIODevice::ReadOnly );
#endif
   return 0;
}

void US_Hydrodyn::browflex_readFromStdout()
{
#if QT_VERSION < 0x040000
   while ( browflex->canReadLineStdout() )
   {
      editor_msg("brown", browflex->readLineStdout() + "\n");
   }
#else
   editor_msg( "brown", QString( browflex->readAllStandardOutput() ) );
#endif   
   //  qApp->processEvents();
}
   
void US_Hydrodyn::browflex_readFromStderr()
{
#if QT_VERSION < 0x040000
   while ( browflex->canReadLineStderr() )
   {
      editor_msg("red", browflex->readLineStderr() + "\n");
   }
#else
   editor_msg( "red", QString( browflex->readAllStandardError() ) );
#endif   
   //  qApp->processEvents();
}
   
void US_Hydrodyn::browflex_finished( int, QProcess::ExitStatus )
{
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   browflex_readFromStderr();
   browflex_readFromStdout();
      //   }
   disconnect( browflex, SIGNAL(readyReadStandardOutput()), 0, 0);
   disconnect( browflex, SIGNAL(readyReadStandardError()), 0, 0);
   disconnect( browflex, SIGNAL(finished( int, QProcess::ExitStatus )), 0, 0);
   editor->append("Browflex finished.\n");
   for ( current_model = 0; 
         current_model < (unsigned int)lb_model->count(); 
         current_model++)
   {
      if ( lb_model->item(current_model)->isSelected() )
      {
         if ( anaflex_options.run_anaflex )
         {
            create_anaflex_files();
            anaflex_ready_to_run = true;
         }
         break;
      }
   }
   bd_anaflex_enables(true);
   pb_stop_calc->setEnabled(false);
}
   
void US_Hydrodyn::browflex_started()
{
   editor_msg("brown", "Browflex launch exited\n");
   disconnect( browflex, SIGNAL(started()), 0, 0);
   bd_anaflex_enables(false);
   pb_stop_calc->setEnabled(true);
}
   
void US_Hydrodyn::bd_anaflex_enables( bool flag )
{
   // this needs better logic
   if ( !bd_widget )
   {
      return;
   }

   if ( ( browflex && browflex->state() == QProcess::Running ) ||
        ( anaflex && anaflex->state() == QProcess::Running ) )
   {
      bd_window->pb_bd_prepare->setEnabled(false);
      bd_window->pb_bd_load->setEnabled(false);
      bd_window->pb_bd_edit->setEnabled(false);
      bd_window->pb_bd_run->setEnabled(false);
      bd_window->pb_bd_load_results->setEnabled(false);
      
      bd_window->pb_anaflex_prepare->setEnabled(false);
      bd_window->pb_anaflex_load->setEnabled(false);
      bd_window->pb_anaflex_edit->setEnabled(false);
      bd_window->pb_anaflex_run->setEnabled(false);
      bd_window->pb_anaflex_load_results->setEnabled(false);
      return;
   }

   // any models selected
   int models = 0;
   for(int i = 0; i < lb_model->count(); i++) {
      if ( lb_model->item(i)->isSelected() ) {
         models++;
      }
   }

   if (!residue_list.size() ||
       !model_vector.size())
   {
      models = 0;
   }

   bd_window->pb_bd_prepare->setEnabled((bool) models);
   bd_window->pb_bd_load->setEnabled(true);
   bd_window->pb_bd_edit->setEnabled(true);
   bd_window->pb_bd_run->setEnabled(flag && bd_ready_to_run);
   bd_window->pb_bd_load_results->setEnabled(bd_last_file != "");

   bd_window->pb_anaflex_prepare->setEnabled(flag);
   bd_window->pb_anaflex_load->setEnabled(true);
   bd_window->pb_anaflex_edit->setEnabled(true);
   bd_window->pb_anaflex_run->setEnabled(flag && anaflex_ready_to_run);
   bd_window->pb_anaflex_load_results->setEnabled(anaflex_last_file != "");
}

void US_Hydrodyn::bd_load_error( QString filename )
{
   editor_msg("red", QString(us_tr("\nFile %1 does not look like a Browflex file.\n")).arg(filename));
}

bool US_Hydrodyn::bd_valid_browflex_main( QString filename )
{
   // make sure this is a valid browflex main file
   /* 
--- example browflex file ----:
1BPI_1-bd-4-4-4-bf-log.txt            !logfile
1BPI_1-bd-4-4-4-bf-tra.txt            !trajectory file
1BPI_1-bd-4-4-4-bf-molec.txt          !molecular file
1BPI_1-bd-4-4-4-bf-initc.txt          !initial coords. file
-                                       !Flow file
-                                       !Elec file
-                                       !Wall file
-                                       !Spec file
1BPI_1-bd-4-4-4-bf-brown.txt          !Brownian data file
*
--- end example --- */
   if ( !filename.isEmpty() )
   {
      QFileInfo fi(filename);
      QString dir = fi.path();
      // check for file format
      QFile f( filename );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         return false;
      }
      QTextStream ts( &f );
      QString tmp_filename;
      // logfile
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         if ( !tmp_filename.contains(QRegularExpression( QStringLiteral( "(txt|TXT)$" ) )) )
         {
            f.close();
            return false;
         }
      } else {
         f.close();
         return false;
      }
      // traj file (don't view)
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         if ( !tmp_filename.contains(QRegularExpression( QStringLiteral( "(txt|TXT)$" ) )) )
         {
            f.close();
            return false;
         }
      } else {
         f.close();
         return false;
      }
      // molecular file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         if ( !tmp_filename.contains(QRegularExpression( QStringLiteral( "(txt|TXT)$" ) )) )
         {
            f.close();
            return false;
         }
      } else {
         f.close();
         return false;
      }
      // initial coords. file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         if ( !tmp_filename.contains(QRegularExpression( QStringLiteral( "(txt|TXT)$" ) )) )
         {
            f.close();
            return false;
         }
      } else {
         f.close();
         return false;
      }
      // flow file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         if ( tmp_filename != "-" &&
              !tmp_filename.contains(QRegularExpression( QStringLiteral( "(txt|TXT)$" ) )) )
         {
            f.close();
            return false;
         }
      } else {
         f.close();
         return false;
      }
      // elec file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         if ( tmp_filename != "-" &&
              !tmp_filename.contains(QRegularExpression( QStringLiteral( "(txt|TXT)$" ) )) )
         {
            f.close();
            return false;
         }
      } else {
         f.close();
         return false;
      }
      // wall file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         if ( tmp_filename != "-" &&
              !tmp_filename.contains(QRegularExpression( QStringLiteral( "(txt|TXT)$" ) )) )
         {
            f.close();
            return false;
         }
      } else {
         f.close();
         return false;
      }
      // spec file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         if ( tmp_filename != "-" &&
              !tmp_filename.contains(QRegularExpression( QStringLiteral( "(txt|TXT)$" ) )) )
         {
            f.close();
            return false;
         }
      } else {
         f.close();
         return false;
      }
      // brownian data file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         if (!tmp_filename.contains(QRegularExpression( QStringLiteral( "(txt|TXT)$" ) )) )
         {
            f.close();
            return false;
         }
      } else {
         f.close();
         return false;
      }
      f.close();
   } else {
      return false;
   }
   return true;
}

void US_Hydrodyn::bd_load()
{
   QString filename;
   if ( bd_last_file != "" )
   {
      // ask if use existing or new
      QFileInfo fi(bd_last_file);
      switch (
              QMessageBox::question(
                                    this,
                                    us_tr("Load Browflex Files"),
                                    QString(us_tr("Replace current Browflex file ") + fi.fileName() + "?"),
                                    QMessageBox::Yes, 
                                    QMessageBox::No,
                                    QMessageBox::NoButton
                                    ) )
      {
      case QMessageBox::Yes : 
         filename = QFileDialog::getOpenFileName( this , windowTitle() , somo_dir + SLASH + "bd" , "*.txt *.TXT" );
         break;
      case QMessageBox::No : 
         return;
         break;
      case QMessageBox::Cancel :
      default :
         return;
         break;
      }
   } else {
      filename = QFileDialog::getOpenFileName( this , windowTitle() , somo_dir + SLASH + "bd" , "*.txt *.TXT" );
   }
   // check to make sure it is a good browflex file
      
   if ( !filename.isEmpty() )
   {
      QFileInfo fi(filename);
      QString dir = fi.path();
      QString name = fi.fileName();
      QString traj_file = "";
      QString molec_file = "";
      // check for file format
      QFile f( filename );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         editor_msg("red",QString(us_tr("\nCould not open file %1 for reading. Check permissions.\n")).arg(filename));
         return;
      }
      if ( !bd_valid_browflex_main( filename ) )
      {
         bd_load_error(filename);
         f.close();
         return;
      }
      QTextStream ts( &f );
      QString tmp_filename;
      // logfile
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         if ( !tmp_filename.contains(QRegularExpression( QStringLiteral( "(txt|TXT)$" ) )) )
         {
            bd_load_error(filename);
            f.close();
            return;
         }
      } else {
         bd_load_error(filename);
         f.close();
         return;
      }
      // traj file (don't view)
      if ( !ts.atEnd() )
      {
         ts >> traj_file;
         ts.readLine();
      } else {
         bd_load_error(filename);
         f.close();
         return;
      }
      // molecular file
      if ( !ts.atEnd() )
      {
         ts >> molec_file;
         ts.readLine();
      } else {
         bd_load_error(filename);
         f.close();
         return;
      }
      // initial coords. file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
      } else {
         bd_load_error(filename);
         f.close();
         return;
      }
      // flow file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
      } else {
         bd_load_error(filename);
         f.close();
         return;
      }
      // elec file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
      } else {
         bd_load_error(filename);
         f.close();
         return;
      }
      // wall file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
      } else {
         bd_load_error(filename);
         f.close();
         return;
      }
      // spec file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
      } else {
         bd_load_error(filename);
         f.close();
         return;
      }
      // brownian data file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
      } else {
         bd_load_error(filename);
         f.close();
         return;
      }
      f.close();
      editor->append(QString("Browflex file %1 loaded\n").arg(filename));

      project = name.left(5);
      bd_project = project;
      bd_last_file = filename;
      bd_last_traj_file = dir + SLASH + traj_file;
      bd_last_molec_file = dir + SLASH + molec_file;
#if defined(DEBUG_CONN)
      cout << "last tra file " << bd_last_traj_file << endl;
      cout << "last molec file " << bd_last_molec_file << endl;
#endif
      bd_ready_to_run = true;
      bd_anaflex_enables(true);
   }
}

void US_Hydrodyn::bd_edit_util( QString dir, QString filename )
{
   if ( filename != "-" && filename != "*" )
   {
      filename = dir + SLASH + filename;
      QFileInfo fi(filename);
      if( fi.exists() && fi.isReadable() )
      {
         if ( fi.size() )
         {
            view_file( filename );
         } else {
            editor_msg("dark red", QString(us_tr("\nFile %1 is empty, not openend.\n")).arg(filename));
         }
      } else {
         if( !fi.exists() )
         {
            editor_msg("red", QString(us_tr("\nCould not open file %1, does not exist.\n")).arg(filename));
         } else {
            editor_msg("red", QString(us_tr("\nCould not open file %1 for reading. Check permissions.\n")).arg(filename));
         }
      }
   }
}

void US_Hydrodyn::bd_edit()
{
   QString filename;
   if ( bd_last_file != "" )
   {
      // ask if use existing or new
      QFileInfo fi(bd_last_file);
      switch (
              QMessageBox::question(
                                    this,
                                    us_tr("View/Edit Browflex Files"),
                                    QString(us_tr("View/Edit current file ") + fi.fileName() + "?"),
                                    QMessageBox::Yes, 
                                    QMessageBox::No,
                                    QMessageBox::Cancel
                                    ) )
      {
      case QMessageBox::Yes : 
         filename = bd_last_file;
         break;
      case QMessageBox::No : 
         filename = QFileDialog::getOpenFileName( this , windowTitle() , somo_dir + SLASH + "bd" , "*.txt *.TXT" );
         break;
      case QMessageBox::Cancel :
      default :
         return;
         break;
      }
   } else {
      filename = QFileDialog::getOpenFileName( this , windowTitle() , somo_dir + SLASH + "bd" , "*.txt *.TXT" );
   }
   if ( !filename.isEmpty() )
   {
      QFileInfo fi(filename);
      QString dir = fi.path();
      // open file and view this and all associated files if first line ends with .txt or .TXT
      QFile f( filename );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         editor_msg("red", QString(us_tr("\nCould not open file %1 for reading. Check permissions.\n")).arg(filename));
         return;
      }
      view_file( filename );
      if ( !bd_valid_browflex_main( filename ) )
      {
         f.close();
         return;
      }
      switch (
              QMessageBox::question(
                                    this,
                                    us_tr("Load Browflex Files"),
                                    QString(us_tr(fi.fileName() + " appears to be a 'main' Browflex file, open all ancillary files? ")),
                                    QMessageBox::Yes, 
                                    QMessageBox::No,
                                    QMessageBox::NoButton
                                    ) )
      {
      case QMessageBox::No : 
         f.close();
         return;
         break;
      case QMessageBox::Yes : 
      default :
         break;
      }
      
      QTextStream ts( &f );
      QString tmp_filename;
      // logfile
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         if ( !tmp_filename.contains(QRegularExpression( QStringLiteral( "(txt|TXT)$" ) )) )
         {
            f.close();
            return;
         }
         bd_edit_util( dir, tmp_filename );
      } else {
         f.close();
         return;
      }
      // traj file (don't view)
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
      } else {
         f.close();
         return;
      }
      // molecular file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         bd_edit_util( dir, tmp_filename );
      } else {
         f.close();
         return;
      }
      // initial coords. file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         bd_edit_util( dir, tmp_filename );
      } else {
         f.close();
         return;
      }
      // flow file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         bd_edit_util( dir, tmp_filename );
      } else {
         f.close();
         return;
      }
      // elec file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         bd_edit_util( dir, tmp_filename );
      } else {
         f.close();
         return;
      }
      // wall file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         bd_edit_util( dir, tmp_filename );
      } else {
         f.close();
         return;
      }
      // spec file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         bd_edit_util( dir, tmp_filename );
      } else {
         f.close();
         return;
      }
      // brownian data file
      if ( !ts.atEnd() )
      {
         ts >> tmp_filename;
         ts.readLine();
         bd_edit_util( dir, tmp_filename );
      } else {
         f.close();
         return;
      }
      f.close();
   }
}

void US_Hydrodyn::bd_load_results()
{
   // check to make sure browflex file is ok

   QString filename = bd_last_file;
   if ( !bd_valid_browflex_main( filename ) )
   {
      bd_load_error(filename);
      return;
   }
   QFileInfo fi_traj( bd_last_traj_file );
   if( !fi_traj.exists() )
   {
      editor_msg("red", QString(us_tr("\nCould not open trajectory file %1, does not exist.\n")).arg(bd_last_traj_file));
      return;
   }
   if( !fi_traj.isReadable() )
   {
      editor_msg("red", QString(us_tr("\nCould not open trajectory file %1 for reading. Check permissions.\n")).arg(bd_last_traj_file));
      return;
   }
   if( !fi_traj.size() )
   {
      editor_msg("red", QString(us_tr("\nTrajectory file %1 is empty!\n")).arg(bd_last_traj_file));
      return;
   }
   QFileInfo fi_molec( bd_last_molec_file );
   if( !fi_molec.exists() )
   {
      editor_msg("red", QString(us_tr("\nCould not open molecular file %1, does not exist.\n")).arg(bd_last_molec_file));
      return;
   }
   if( !fi_molec.isReadable() )
   {
      editor_msg("red", QString(us_tr("\nCould not open molecular file %1 for reading. Check permissions.\n")).arg(bd_last_molec_file));
      return;
   }
   if( !fi_molec.size() )
   {
      editor_msg("red", QString(us_tr("\nMolecular file %1 is empty!\n")).arg(bd_last_molec_file));
      return;
   }
   // read bead sizes
   {
      QString errmsg = QString(us_tr("\nMolecular file ") + "%1" + us_tr("error at line")).arg(bd_last_molec_file) + "%1\n";
      QFile f( bd_last_molec_file );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         editor_msg("red", QString(us_tr("\nCould not open molecular file %1 for reading. Check permissions.\n")).arg(bd_last_molec_file));
         return;
      }
      QTextStream ts( &f );

      // temperature
      int line = 1;
      ts >> bd_load_results_temp;  
#if defined(DEBUG_CONN)
      cout << "temperature " << bd_load_results_temp << endl;
#endif
      if ( ts.atEnd() )
      {
         editor_msg("red", QString(errmsg).arg(line));
         f.close();
         return;
      }
      ts.readLine(); 

      // psv
      line++;
      ts >> bd_load_results_solvent_visc;
      bd_load_results_solvent_visc *= 100;  // cP to Poise
      
#if defined(DEBUG_CONN)
      cout << "solvent visc " << bd_load_results_solvent_visc << endl;
#endif
      if ( ts.atEnd() )
      {
         editor_msg("red", QString(errmsg).arg(line));
         f.close();
         return;
      }
      ts.readLine();

      // mw
      line++;
      ts >> bd_load_results_mw;
      if ( ts.atEnd() )
      {
         editor_msg("red", QString(errmsg).arg(line));
         f.close();
         return;
      }
      ts.readLine();

      // name of case
      line++;
      if ( ts.atEnd() )
      {
         editor_msg("red", QString(errmsg).arg(line));
         f.close();
         return;
      }
      ts.readLine();

      // beads
      line++;
      ts >> bd_load_results_beads;
#if defined(DEBUG_CONN)
      cout << "beads " << bd_load_results_beads << endl;
#endif
      if ( ts.atEnd() )
      {
         editor_msg("red", QString(errmsg).arg(line));
         f.close();
         return;
      }
      ts.readLine();

      bd_load_results_mw /= bd_load_results_beads;
#if defined(DEBUG_CONN)
      cout << "mw " << bd_load_results_mw << endl;
#endif

      // beads radii
      float tmp_float;
      bd_load_results_bead_radius.clear( );
      for ( unsigned int i = 0; i < bd_load_results_beads; i++ )
      {
         line++;
         ts >> tmp_float;
         // these are in cm in the file
         bd_load_results_bead_radius.push_back(tmp_float);
         if ( ts.atEnd() )
         {
            editor_msg("red", QString(errmsg).arg(line));
            f.close();
            return;
         }
         ts.readLine();
      }

      // bonds etc... not needed right now
      f.close();
   }

#if defined(DEBUG_CONN)
   for ( unsigned int i = 0; i < bd_load_results_beads; i++ )
   {
      cout << QString("bead %1 radius %2\n").arg(i).arg(bd_load_results_bead_radius[i]);
   }
#endif

   // ok, now setup a type 9 anaflex run and run it.
   editor->append("\ncreate anaflex files\n");
   create_anaflex_files( 9, 0 );
   editor->append("\nrun anaflex\n");
   bd_anaflex_enables( false );
   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   anaflex_return_to_bd_load_results = true;
   run_anaflex( 9, 0 );
}

void US_Hydrodyn::bd_load_results_after_anaflex()
{
   editor->append("\nback from run anaflex\n");
   // GET unit of measure from log file
   pb_stop_calc->setEnabled(false);
   if ( stopFlag )
   {
      editor_msg("red", "\nload Browflex results was stopped\n");
      return;
   }

   // need output1 for length unit

   QFileInfo fi_out1( anaflex_last_out1_file );
   if( !fi_out1.exists() )
   {
      editor_msg("red", QString(us_tr("\nCould not Anaflex log file %1, does not exist.\n")).arg(anaflex_last_out1_file));
      return;
   }
   if( !fi_out1.isReadable() )
   {
      editor_msg("red", QString(us_tr("\nCould not open Anaflex log file %1 for reading. Check permissions.\n")).arg(anaflex_last_out1_file));
      return;
   }
   if( !fi_out1.size() )
   {
      editor_msg("red", QString(us_tr("\nAnaflex log file %1 is empty!\n")).arg(anaflex_last_out1_file));
      return;
   }
   QFile f_out1( anaflex_last_out1_file );
   if ( !f_out1.open( QIODevice::ReadOnly ) )
   {
      editor_msg("red", QString(us_tr("\nCould not open output file %1 for reading. Check permissions.\n")).arg(anaflex_last_out1_file));
      return;
   }

   QTextStream ts_out1( &f_out1 );
   
   float unit_of_length = 0.0;
   {
      QString tmp;
      bool done = false;
      QRegularExpression rx("^Unit of length = (.*)$");
      while( !done &&  !ts_out1.atEnd() )
      {
         QRegularExpressionMatch m = rx.match( ts_out1.readLine() );
         if ( !m.hasMatch() )
         {
            done = true;
            unit_of_length = m.captured(1).toFloat();
         }
      }
      f_out1.close();
      if ( !done ) 
      {
         editor_msg("red", QString(us_tr("\nCould not find \"Unit of length\" in file %1.\n")).arg(anaflex_last_out1_file));
         return;
      }
   }
#if defined(DEBUG_CONN)
   cout << "unit_of_length " << unit_of_length << endl;
   cout << "log10 unit_of_length " << log10(unit_of_length) << endl;
#endif
   int exp = (int)(log10(unit_of_length) - .01);
   exp -= 2;  // convert from browflex cm to our m.
   hydro.unit = exp;
   display_default_differences();
   
#if defined(DEBUG_CONN)
   cout << "exp = " << exp << endl;
#endif

   for ( unsigned int i = 0; i < bd_load_results_beads; i++ )
   {
      bd_load_results_bead_radius[i] /= unit_of_length;
#if defined(DEBUG_CONN)
      cout << QString("after redo, bead %1 radius %2\n").arg(i).arg(bd_load_results_bead_radius[i]);
#endif
   }

   // log file into bead models
   // read traj file, molec file, build bead models & load them up! (into batch?)
   // molec file has molecular weight, have to input psv (like dammin/dammif files)
   // create bead model file

   QFileInfo fi_log( anaflex_last_log_file );
   if( !fi_log.exists() )
   {
      editor_msg("red", QString(us_tr("\nCould not Anaflex log file %1, does not exist.\n")).arg(anaflex_last_log_file));
      return;
   }
   if( !fi_log.isReadable() )
   {
      editor_msg("red", QString(us_tr("\nCould not open Anaflex log file %1 for reading. Check permissions.\n")).arg(anaflex_last_log_file));
      return;
   }
   if( !fi_log.size() )
   {
      editor_msg("red", QString(us_tr("\nAnaflex log file %1 is empty!\n")).arg(anaflex_last_log_file));
      return;
   }
   QString dir = fi_log.path();

   QString errmsg = QString(us_tr("\nAnaflex log file ") + "%1" + us_tr(" error at line ")).arg(anaflex_last_log_file) + "%1\n";
   QFile f( anaflex_last_log_file );
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      editor_msg("red", QString(us_tr("\nCould not open molecular file %1 for reading. Check permissions.\n")).arg(anaflex_last_log_file));
      return;
   }

   QTextStream ts( &f );

   // name
   QString name;
   int line = 1;
   ts >> name;
   if ( ts.atEnd() )
   {
      editor_msg("red", QString(errmsg).arg(line));
      f.close();
      return;
   }
   ts.readLine();

   name.replace(QRegularExpression( QStringLiteral( "\\.(txt|TXT)" ) ),"");
#if defined(DEBUG_CONN)
   cout << "name " << name << endl;
#endif

   // nmol
   unsigned int nmol;
   line++;
   ts >> nmol;
#if defined(DEBUG_CONN)
   cout << "nmol " << nmol << endl;
#endif
   if ( ts.atEnd() )
   {
      editor_msg("red", QString(errmsg).arg(line));
      f.close();
      return;
   }
   ts.readLine();

   // ttraj
   float ttraj;
   line++;
   ts >> ttraj;
#if defined(DEBUG_CONN)
   cout << "ttraj " << ttraj << endl;
#endif
   if ( ts.atEnd() )
   {
      editor_msg("red", QString(errmsg).arg(line));
      f.close();
      return;
   }
   ts.readLine();

   // nconf
   unsigned int nconf;
   line++;
   ts >> nconf;
#if defined(DEBUG_CONN)
   cout << "nconf " << nconf << endl;
#endif
   if ( ts.atEnd() )
   {
      editor_msg("red", QString(errmsg).arg(line));
      f.close();
      return;
   }
   ts.readLine();

   if ( anaflex_options.nfrec > 1 ) 
   {
      nconf = 
         ( nconf / anaflex_options.nfrec ) +
         ( nconf % anaflex_options.nfrec ? 1 : 0 );
   }

   // beads
   unsigned int beads;
   line++;
   ts >> beads;
#if defined(DEBUG_CONN)
   cout << "beads " << beads << endl;
#endif
   if ( ts.atEnd() )
   {
      editor_msg("red", QString(errmsg).arg(line));
      f.close();
      return;
   }
   ts.readLine();

   if ( beads != bd_load_results_bead_radius.size() )
   {
      editor_msg("red", 
                 QString(us_tr("Number of beads in Anaflex log file (%1) does not match"
                            " number of beads in Browflex molecular file (%2)"))
                 .arg(beads)
                 .arg(bd_load_results_bead_radius.size()));
      f.close();
      return;
   }                 

   PDB_atom tmp_atom;

   //   results.vbar = bd_load_results_solvent_visc;

   // set the parameters

   QString msg = QString(us_tr(" Enter conditions for the bead models: "));
   
   double psv = 0.0; // misc.vbar;
   bool check_fix_overlaps = true;
   bool load_results_win_done = false;

   // create the bead models
   QString basename = dir + SLASH + name;
   int save_bead_output = bead_output.output;
   bead_output.output |= US_HYDRODYN_OUTPUT_SOMO;
   vector < QString > model_names;
   for ( unsigned int i = 0; i < nmol; i++ )
   {
      for ( unsigned int j = 0; j < nconf; j++ )
      {
         bead_model.clear( );
         for ( unsigned int k = 0; k < beads; k++ )
         {
            line++;
            for ( unsigned int l = 0; l < 3; l++ )
            {
               tmp_atom.serial = k + 1;
               if ( ts.atEnd() )
               {
                  editor_msg("red", QString(errmsg).arg(line));
                  f.close();
                  bead_output.output = save_bead_output;
                  return;
               }
               ts >> tmp_atom.bead_coordinate.axis[l];
               //  tmp_atom.bead_coordinate.axis[l] *= unit_of_length * 1e6; // to micron
            }

            ts.readLine();

            tmp_atom.bead_computed_radius = bd_load_results_bead_radius[k];
            tmp_atom.bead_actual_radius = bd_load_results_bead_radius[k];
            tmp_atom.radius = bd_load_results_bead_radius[k];
            tmp_atom.bead_mw = bd_load_results_mw;
            tmp_atom.bead_ref_mw = tmp_atom.bead_mw;
            tmp_atom.bead_ref_volume = 0;
            tmp_atom.bead_color = 1;

            tmp_atom.exposed_code = 1;
            tmp_atom.all_beads.clear( );
            tmp_atom.active = true;
            tmp_atom.chain = 1;
            tmp_atom.normalized_ot_is_valid = false;
            tmp_atom.name = "ATOM";
            tmp_atom.resName = "RESIDUE";
            tmp_atom.iCode = "ICODE";
            tmp_atom.chainID = "CHAIN";
            bead_model.push_back(tmp_atom);
         }
         QString model_name = basename + QString("-m%1-c%2").arg(i).arg(j);
         editor->append(QString("base name is %1\n").arg(basename));
         // write_bead_model( model_name + "-pp0", &bead_model );
         bool has_corr = read_corr(basename + ".corr", &bead_model);
#if defined(DEBUG_CONN)
         puts(has_corr ? "has corr" : "does not have corr");
         cout << QString("base name %1\n").arg(basename);
#endif
         // write_bead_model( model_name + "-pp1", &bead_model );
         if ( !load_results_win_done )
         {
            if ( has_corr )
            {
               psv = results.vbar;
            }
            
            US_Hydrodyn_BD_Load_Results_Opts *hblro = 
               new US_Hydrodyn_BD_Load_Results_Opts (
                                                     msg,
                                                     &hydro.temperature,
                                                     &hydro.solvent_viscosity,
                                                     &hydro.solvent_density,
                                                     &hydro.solvent_name,
                                                     &hydro.solvent_acronym,
                                                     &psv,
                                                     bd_load_results_temp,
                                                     bd_load_results_solvent_visc,
                                                     &check_fix_overlaps
                                                     );
            US_Hydrodyn::fixWinButtons( hblro );
            do {
               hblro->exec();
            } while ( psv <= 0.0 );
         
            results.vbar = psv;
            
            delete hblro;

            load_results_win_done = true;
         }
#if defined(DEBUG_CONN)
         cout << "model name " << model_name << endl;
#endif
         if ( check_fix_overlaps )
         {
#if defined(DEBUG_CONN)
            puts("check fix overlaps\n"); fflush(stdout);
#endif
            if ( overlap_check(true, true, true,
                               hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance) )
            {
               pb_stop_calc->setEnabled(true);
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
               // write_bead_model( model_name + "-X", &bead_model );
               if ( has_corr )
               {
                  //                  compute_asa();
                  radial_reduction();
                  if (asa.recheck_beads)
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
                  // write_bead_model( model_name + "-Y", &bead_model );
               } else { 
                  if ( grid.enable_asa )
                  {
                     editor->append("ASA check\n");
                     qApp->processEvents();
                     // set all beads buried
                     if ( !has_corr )
                     {
                        for(unsigned int i = 0; i < bead_model.size(); i++) {
                           bead_model[i].exposed_code = 6;
                           bead_model[i].bead_color = 6;
                           bead_model[i].chain = 1; // all 'side' chain
                        }
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
                     // now apply radial reduction with outward translation using
                     
                     // grid_exposed/buried_overlap
                     overlap_reduction save_sidechain_overlap = sidechain_overlap;
                     overlap_reduction save_mainchain_overlap = mainchain_overlap;
                     overlap_reduction save_buried_overlap = buried_overlap;
                     sidechain_overlap = grid_exposed_overlap;
                     mainchain_overlap = grid_exposed_overlap;
                     buried_overlap = grid_buried_overlap;
                     progress->setValue(progress->value() + 1);
                     
                     double save_overlap = overlap_tolerance;
                     overlap_tolerance *= .8;
                     radial_reduction();
                     overlap_tolerance = save_overlap;
                     
                     sidechain_overlap = save_sidechain_overlap;
                     mainchain_overlap = save_mainchain_overlap;
                     buried_overlap = save_buried_overlap;
                     
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
                     }
                  }
                  else
                  {
                     //                  // set all beads exposed
                     //                  for(unsigned int i = 0; i < bead_model.size(); i++) {
                     //		     bead_model[i].exposed_code = 1;
                     //		     bead_model[i].bead_color = 8;
                     //		     bead_model[i].chain = 1; // all 'side' chain
                     //                  }
                     
                     if (grid_overlap.remove_overlap)
                     {
                        progress->setValue(progress->value() + 1);
                        double save_overlap = overlap_tolerance;
                        overlap_tolerance *= .8;
                        radial_reduction();
                        overlap_tolerance = save_overlap;
                        progress->setValue(progress->value() + 1);
                     }
                     if (stopFlag)
                     {
                        editor->append("Stopped by user\n\n");
                        progress->reset();
                        f.close();
                        return;
                     }
                     if (asa.recheck_beads)
                     {
                        editor->append("Rechecking beads\n");
                        qApp->processEvents();
                        // all buried
                        if ( !has_corr )
                        {
                           for(unsigned int i = 0; i < bead_model.size(); i++) {
                              bead_model[i].exposed_code = 6;
                              bead_model[i].bead_color = 6;
                           }
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
                     }
                     else
                     {
                        // all exposed
                        if ( !has_corr )
                        {
                           for(unsigned int i = 0; i < bead_model.size(); i++) {
                              bead_model[i].exposed_code = 1;
                              bead_model[i].bead_color = 8;
                           }
                        }
                     }
                  }
               } //
               if (stopFlag)
               {
                  editor->append("Stopped by user\n\n");
                  progress->reset();
                  f.close();
                  return;
               }
            } else {
#if defined(DEBUG_CONN)
               puts("no overlaps found\n"); fflush(stdout);
#endif
            }               
         } else {
#if defined(DEBUG_CONN)
            puts("no ! check fix overlaps\n"); fflush(stdout);
#endif
         }            
         // set all exposed for now
         // write_bead_model( model_name + "-pp8", &bead_model );
         if ( !has_corr ) {
            for( unsigned int i = 0; i < bead_model.size(); i++) {
               bead_model[i].exposed_code = 1;
               bead_model[i].bead_color = 8;
            }
         }
         // write_bead_model( model_name + "-pp9", &bead_model );
         check_bead_model_for_nan();
         write_bead_model( model_name, &bead_model );
         model_names.push_back(  model_name + ".bead_model" );
         editor->append(QString(us_tr("Created bead model %1\n")).arg(name + QString("-m%1-c%2").arg(i).arg(j) + ".bead_model"));
      }
   }
   progress->setValue( 1 ); progress->setMaximum( 1 );
   f.close();
   bead_output.output = save_bead_output;

   editor->append(us_tr("Loading into batch window. (this may take some time)\n"));
   qApp->processEvents();
   
   // add output to batch window

   // check if want to clear existing batch files ....

   if ( !batch_widget )
   {
      batch_window = new US_Hydrodyn_Batch(&batch, &batch_widget, this);
   }
   batch_window->show();
   
   if ( batch_window->count_files() ) 
   {
      batch_window->raise();
      switch (
              QMessageBox::question(
                                    this,
                                    us_tr("Load Browflex Files"),
                                    QString(us_tr("The batch operation window currently has files loaded.\n"
                                               "      Should they be removed before loading ?\n\n"
                                               "    CANCEL to skip loading of batch operations."
                                               )),
                                    QMessageBox::Yes, 
                                    QMessageBox::No,
                                    QMessageBox::Cancel
                                    ) )
      {
      case QMessageBox::Yes : 
         batch_window->clear_files();
         break;
      case QMessageBox::Cancel : 
         editor_msg("red", "Loading of models into batch operation window canceled\n");
         return;
         break;
      case QMessageBox::No : 
      default :
         break;
      }
   }
   this->raise();
      
   batch_window->add_files( model_names );

   editor->append(us_tr("Load Browflex results completed.\n"));
   batch_window->raise();
}

int US_Hydrodyn::browflex_get_no_of_beads( QString filename )
{
#if defined(DEBUG_CONN)
   cout << "check no_of_beads\n";
#endif
   if ( !bd_valid_browflex_main( filename ) )
   {
#if defined(DEBUG_CONN)
      cout << "check no_of_beads invalid file\n";
#endif
      return 0;
   }
   QFile f( filename );
   if ( !f.open( QIODevice::ReadOnly ) )
   {
#if defined(DEBUG_CONN)
      cout << "check no_of_beads can't read\n";
#endif
      return 0;
   }
   QTextStream ts( &f );
   QString molec_file = "";
   ts.readLine();    // logfile
   ts.readLine();    // trajfile
   // molecular file
   if ( !ts.atEnd() )
   {
      ts >> molec_file;
   } else {
#if defined(DEBUG_CONN)
      cout << "check no_of_beads no molec file line\n";
#endif
      f.close();
      return 0;
   }
   f.close();
   molec_file = QFileInfo(filename).path() + SLASH + molec_file;
   f.setFileName(molec_file);
   if ( !f.open( QIODevice::ReadOnly ) )
   {
#if defined(DEBUG_CONN)
      cout << "check no_of_beads can't open molec file\n";
#endif
      return 0;
   }
   ts.readLine();
   ts.readLine();
   ts.readLine();
   ts.readLine();
   int no_of_beads;
   if ( !ts.atEnd() )
   {
      ts >> no_of_beads;
   } else {
#if defined(DEBUG_CONN)
      cout << "check no_of_beads molec file not enough lines\n";
#endif
      f.close();
      return 0;
   }
#if defined(DEBUG_CONN)
   cout << "no of beads from molec " << no_of_beads << endl;
#endif
   return no_of_beads;
}
