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

   bead_model_suffix = QString("bd-%1").arg(bd_options.threshold);
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
      pb_visualize->setEnabled(true);
      pb_calc_hydro->setEnabled(false);
      pb_bead_saxs->setEnabled(true);
   }
   else
   {
      editor->append("Errors encountered\n");
   }

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
        break;
      }
   }

   pb_bd->setEnabled(true);
   pb_somo->setEnabled(true);
   pb_grid_pdb->setEnabled(true);
   pb_grid->setEnabled(true);
   pb_stop_calc->setEnabled(false);
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
          (*model)[i].active &&
          (*model)[i].chain == 0 // main chain
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
   return 0;
}

int US_Hydrodyn::compute_bd_connections()
{
   connection_active.clear();
   connection_dists.clear();
   connection_dist_stats.clear();

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
      
   // build connection_active for 1st model
   float d; // distance
   
   current_model = models_to_proc[0];

   for ( unsigned int i = 0; i < bead_models[current_model].size() - 1; i++ ) 
   {
      if ( 
          bead_models[current_model][i].active &&
          bead_models[current_model][i].chain == 0 // main chain
          )
      {
         for ( unsigned int j = i + 1; j < bead_models[current_model].size(); j++ ) 
         {
            if ( 
                bead_models[current_model][j].active &&
                bead_models[current_model][j].chain == 0 // main chain
                )
            {
               d = dist( bead_models[current_model][i].bead_coordinate,
                               bead_models[current_model][j].bead_coordinate );
               if ( d <= bd_options.threshold )
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
             !bead_models[current_model][i].active ||
             bead_models[current_model][i].chain != 0 ||
             !bead_models[current_model][j].active ||
             bead_models[current_model][j].chain != 0 ||
             
             ( d = dist( bead_models[current_model][i].bead_coordinate,
                         bead_models[current_model][j].bead_coordinate ) ) > bd_options.threshold 
             )
         {
            editor->append(QString("removing connection %1 %2\n").arg(i).arg(j));
            connection_active[it->first] = false;
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
