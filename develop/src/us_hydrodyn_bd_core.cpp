// us_hydrodyn.cpp contains class creation & gui connected functions
// (this) us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic computations
// us_hydrodyn_other.cpp contains other routines such as file i/o

// includes and defines need cleanup

#include "../include/us_hydrodyn.h"
#include <qregexp.h>
#include <values.h>

#ifndef WIN32
#   include <unistd.h>
#   define SLASH "/"
#else
#   include <direct.h>
#   include <io.h>
#   define SLASH "\\"
#endif

ostream& operator<<(ostream& out, const point& c)
{
   out << "(" << c.axis[0] << "," << c.axis[1] << "," << c.axis[2] << ")";
   return out;
}

void US_Hydrodyn::calc_bd()
{
   pb_bd->setEnabled( false );
   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_somo->setEnabled(false);
   pb_bd->setEnabled(false);
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
      pb_bd->setEnabled(true);
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

   somo_processed.resize(lb_model->numRows());
   bead_models.resize(lb_model->numRows());
   QString msg = QString("\n%1 models selected:").arg(project);
   for(int i = 0; i < lb_model->numRows(); i++) {
      somo_processed[i] = 0;
      if ( lb_model->isSelected(i) ) {
         current_model = i;
         msg += QString(" %1").arg(i+1);
      }
   }
   msg += "\n";
   editor->append(msg);

   for (current_model = 0; 
        current_model < (unsigned int)lb_model->numRows(); 
        current_model++)
   {
      if (!any_errors && lb_model->isSelected(current_model))
      {
         any_models = true;
         if(!compute_asa(true))
         {
            somo_processed[current_model] = 1;
            bead_models[current_model] = bead_model;
         }
         else
         {
            any_errors = true;
         }
         write_contact_plot( 
                            somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
                            QString("-pdb_contact-%1.txt").arg(bd_options.threshold_pb_pb)
                            , &model_vector_as_loaded[current_model]
                            , bd_options.threshold_pb_pb );
      }
      if (stopFlag)
      {
         editor->append("Stopped by user\n\n");
         pb_bd->setEnabled(true);
         pb_somo->setEnabled(true);
         pb_grid_pdb->setEnabled(true);
         progress->reset();
         return;
      }

      // calculate bead model and generate hydrodynamics calculation output
      // if successful, enable follow-on buttons:
   }
   if (any_models && !any_errors)
   {
      editor->append("Build bead model for BD completed\n");
      qApp->processEvents();
      compute_bd_connections();
      for (current_model = 0; 
           current_model < (unsigned int)lb_model->numRows(); 
           current_model++)
      {
         if ( lb_model->isSelected(current_model) )
         {
            QString fname = 
               somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
               QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "");
            write_pdb( fname, &bead_models[current_model] );
            create_browflex_files();
            if ( bd_options.run_browflex )
            {
               run_browflex();
            }
            break;
         }
      }
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_bd->setEnabled( ( browflex && browflex->isRunning() ) ? false : true );
   pb_somo->setEnabled(true);
   pb_grid_pdb->setEnabled(true);
   pb_grid->setEnabled(true);
   pb_stop_calc->setEnabled( ( browflex && browflex->isRunning() ) ? true : false );
}

// ---------- create browflex files
int US_Hydrodyn::create_browflex_files()
{
   double conv = 1e-8;

   QString filename = 
      project + QString("_%1").arg(current_model + 1) +
      QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
      + "-bf";
   QString basename = 
      somo_dir + SLASH + filename;

   // browflex-main.txt
   QFile f;
   // main file
   {
      f.setName(basename + "-main.txt");
      if ( !f.open(IO_WriteOnly) )
      {
         editor->append(QString("File write error: can't create %1\n").arg(f.name()));
         return -1;
      }
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
      f.setName(basename + "-molec.txt");
      if ( !f.open(IO_WriteOnly) )
      {
         editor->append(QString("File write error: can't create %1\n").arg(f.name()));
         return -1;
      }
      QTextStream ts(&f);
      ts << QString(" %1,   Temperature (C)\n").arg(hydro.temperature);
      ts << QString(" %1,   Solvent viscosity (poise)\n").arg(hydro.solvent_viscosity);
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

      QRegExp rx("^(\\d+)~(\\d+)$");

      for ( map < QString, bool >::iterator it = connection_active.begin();
            it != connection_active.end();
            it++ )
      {
         if ( connection_active[it->first] )
         {
            connectors++;
         }
      }

      ts << QString(" %1,   Number of connectors\n").arg(connectors);

      for ( map < QString, bool >::iterator it = connection_active.begin();
            it != connection_active.end();
            it++ )
      {
         if ( connection_active[it->first] )
         {
            if ( rx.search(it->first) == -1 ) 
            {
               editor->append("unexpected regexp extract failure (write_browflex_files)!\n");
               return -1;
            }
            int i = rx.cap(1).toInt();
            int j = rx.cap(2).toInt();
            
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
      f.setName(basename + "-initc.txt");
      if ( !f.open(IO_WriteOnly) )
      {
         editor->append(QString("File write error: can't create %1\n").arg(f.name()));
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
      f.setName(basename + "-brown.txt");
      if ( !f.open(IO_WriteOnly) )
      {
         editor->append(QString("File write error: can't create %1\n").arg(f.name()));
         return -1;
      }
      QTextStream ts(&f);
      ts << QString("%1,         mol\n").arg(1);
      ts << QString("%1,         tprev\n").arg(0);
      ts << QString("%1,         ttraj\n").arg(bd_options.ttraj);
      ts << QString("%1,         nconf \n").arg(bd_options.nconf);
      ts << QString("%1,         nscreen\n").arg(20);
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
   fpdb.setName(QString("%1.pdb").arg(fname));
   if ( !fpdb.open(IO_WriteOnly) )
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
            QString("")
            .sprintf(     
                     "ATOM   %4d  PB  UNK A%4d    %8.3f%8.3f%8.3f  1.00 10.00           PB  \n",
                     i + 1, i + 1,
                     (*model)[i].bead_coordinate.axis[0],
                     (*model)[i].bead_coordinate.axis[1],
                     (*model)[i].bead_coordinate.axis[2]
                     );
      }
   }

   // consolidate & symmetrize connections

   QRegExp rx("^(\\d+)~(\\d+)$");

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
         if ( rx.search(it->first) == -1 ) 
         {
            editor->append("unexpected regexp extract failure (write_pdb)!\n");
            return -1;
         }
         tmp_suint_i.x = rx.cap(1).toInt();
         tmp_suint_j.x = rx.cap(2).toInt();

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
      QString cbase = QString("").sprintf("CONECT%5d",it->x + 1);
      QString out;
      QString tmp = "";
      unsigned int j = 0;
      for (
           list < sortable_uint > ::const_iterator itj = connect_list[it->x].begin(); 
           itj != connect_list[it->x].end(); 
           ++itj
           )
      {
         tmp += QString("").sprintf("%5d",itj->x + 1);
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
      QStringList argument;
#if !defined(WIN32)
      argument.append("xterm");
      argument.append("-e");
#endif
#if defined(BIN64)
      argument.append(USglobal->config_list.system_dir + SLASH + "bin64" + SLASH + "rasmol");
#else
      argument.append(USglobal->config_list.system_dir + SLASH + "bin" + SLASH + "rasmol");
#endif
      argument.append(QFileInfo(fpdb.name()).fileName());
      rasmol->setWorkingDirectory(QFileInfo(fpdb.name()).dirPath());
      rasmol->setArguments(argument);
      if (!rasmol->start())
      {
         QMessageBox::message(tr("Please note:"), tr("There was a problem starting RASMOL\n"
                                                     "Please check to make sure RASMOL is properly installed..."));
      }
   }
   
   return 0;
}

int US_Hydrodyn::compute_bd_connections()
{
   connection_active.clear();
   connection_dists.clear();
   connection_dist_stats.clear();
   connection_pair_type.clear();
   connection_forced.clear();

   vector < unsigned int > models_to_proc;

   for ( current_model = 0; 
         current_model < (unsigned int)lb_model->numRows(); 
         current_model++)
   {
      if ( lb_model->isSelected(current_model) )
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
                      somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
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

   QRegExp rx("^(\\d+)~(\\d+)$");

   for ( unsigned int k = 1; k < models_to_proc.size(); k++ )
   {
      current_model = models_to_proc[k];
      

      for ( map < QString, bool >::iterator it = connection_active.begin();
            it != connection_active.end();
            it++ )
      {
         if ( rx.search(it->first) == -1 ) 
         {
            editor->append("unexpected regexp extract failure (compute_connections)!\n");
            return -1;
         }
         int i = rx.cap(1).toInt();
         int j = rx.cap(2).toInt();

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
            editor->append(QString("Error: BD connection %! has no values!\n")
                           .arg(it->first));
            return -1;
         }
      }
   }

   for ( map < QString, bool >::iterator it = connection_active.begin();
         it != connection_active.end();
         it++ )
   {
      if ( connection_active[it->first] ) 
      {
         cout << 
            QString("Connection %1 distance min %1 max %2 avg %3 dist:\n")
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
         
   return 0;
}
               

// -------------- contact plot stuff --------------------


int US_Hydrodyn::write_contact_plot( QString fname, PDB_model *model, float thresh )
{
   // create a contact plot of each residues c-alpha's
   QFile f(fname);
   if ( !f.open(IO_WriteOnly) )
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
   if ( !f.open(IO_WriteOnly) )
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

   pb_list.clear();
   pb_list.resize(model->molecule.size());
   pb_base_list.clear();
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
         if ( this_atom->name.contains(QRegExp("^(N|CA|C|O)$")) )
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
            pb_atoms.clear();
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
   for (unsigned int j = 0; j < pb_base_list.size(); j++ )
   {
      for ( unsigned int k = 0; k < pb_base_list[j].size(); k++ ) {
         unsigned int p = pb_base_list[j][k];
         PDB_atom *this_atom = &(model->molecule[j].atom[pb_list[j][p]]);
         cout << 
            QString("base pb_atom %1 %2 %3 %4 %5\n")
            .arg(this_atom->serial)
            .arg(this_atom->name)
            .arg(this_atom->resName)
            .arg(this_atom->chainID)
            .arg(this_atom->resSeq);
         vector < point > v;
         point cross1 = plane(&(model->molecule[j].atom[pb_list[j][p+2]]),  // O
                              &(model->molecule[j].atom[pb_list[j][p+1]]),  // C
                              &(model->molecule[j].atom[pb_list[j][p+0]])); // CA
         v.push_back(cross1);
         cout << "(ca - c) x (o - c): " << cross1 << endl;
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
            cout << "(o - c) x (n - c): " << cross2 << endl;
            cout << "(n - c) x (ca - c): " << cross3 << endl;
            cout << "(ca - n) x (o - n): " << cross4 << endl;
            v.push_back(cross2);
            v.push_back(cross3);
            v.push_back(cross4);
            cout << "average: " << average(&v) << endl;
         }

         // compute angles between C-N-CA

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
   QString dir = somo_dir;
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
         QColor save_color = editor->color();
         editor->setColor("red");
         editor->append(QString("Browflex program '%1' does not exist\n").arg(prog));
         editor->setColor(save_color);
         return -1;
      }
      if ( !qfi.isExecutable() )
      {
         QColor save_color = editor->color();
         editor->setColor("red");
         editor->append(QString("Browflex program '%1' is not executable\n").arg(prog));
         editor->setColor(save_color);
         return -1;
      }
   }

   QString browfile = 
      project + QString("_%1").arg(current_model + 1) +
      QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
      + "-bf-main.txt\n" ;

   cout << QString("run browflex dir <%1> prog <%2> stdin <%3>\n")
      .arg(dir)
      .arg(prog)
      .arg(browfile);
   browflex = new QProcess( this );
   browflex->setWorkingDirectory( dir );
   browflex->addArgument( prog );
   connect( browflex, SIGNAL(readyReadStdout()), this, SLOT(browflex_readFromStdout()) );
   connect( browflex, SIGNAL(readyReadStderr()), this, SLOT(browflex_readFromStderr()) );
   connect( browflex, SIGNAL(processExited()), this, SLOT(browflex_processExited()) );
   connect( browflex, SIGNAL(launchFinished()), this, SLOT(browflex_launchFinished()) );

   browflex->launch( browfile );

   return 0;
}

void US_Hydrodyn::browflex_readFromStdout()
{
   QColor save_color = editor->color();
   while ( browflex->canReadLineStdout() )
   {
      editor->setColor("brown");
      editor->append(browflex->readLineStdout() + "\n");
      editor->setColor(save_color);
   }
   qApp->processEvents();
}
   
void US_Hydrodyn::browflex_readFromStderr()
{
   QColor save_color = editor->color();
   while ( browflex->canReadLineStderr() )
   {
      editor->setColor("red");
      editor->append(browflex->readLineStderr() + "\n");
      editor->setColor(save_color);
   }
   qApp->processEvents();
}
   
void US_Hydrodyn::browflex_processExited()
{
   QColor save_color = editor->color();
   editor->setColor("brown");
   editor->append("Browflex process exited\n");
   editor->setColor(save_color);
   browflex_readFromStderr();
   browflex_readFromStdout();
   disconnect( browflex, SIGNAL(readyReadStdout()), 0, 0);
   disconnect( browflex, SIGNAL(readyReadStderr()), 0, 0);
   disconnect( browflex, SIGNAL(processExited()), 0, 0);
   for ( current_model = 0; 
         current_model < (unsigned int)lb_model->numRows(); 
         current_model++)
   {
      if ( lb_model->isSelected(current_model) )
      {
         pb_bd->setEnabled( true );
         break;
      }
   }
}
   
void US_Hydrodyn::browflex_launchFinished()
{
   QColor save_color = editor->color();
   editor->setColor("brown");
   editor->append("Browflex launch exited\n");
   editor->setColor(save_color);
   disconnect( browflex, SIGNAL(launchFinished()), 0, 0);
}
   
