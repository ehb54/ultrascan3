// us_hydrodyn.cpp contains class creation & gui connected functions
// (this) us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o

// includes and defines need cleanup

#include "../include/us_hydrodyn.h"
#include "../include/us_surfracer.h"
#include "../include/us_hydrodyn_supc.h"
#include "../include/us_hydrodyn_pat.h"
#include "../include/us_hydrodyn_asab1.h"
#include "../include/us_hydrodyn_grid_atob.h"
#include "../include/us_vvv.h"
#include <qregexp.h>
#include <qfont.h>
//Added by qt3to4:
#include <QTextStream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// #define OLD_ASAB1_SC_COMPUTE

#undef DEBUG
// #define DEBUG
// #define DEBUG1
// #define AUTO_BB_DEBUG
// #define BUILD_MAPS_DEBUG
// #define DEBUG_BEAD_CHECK

#define USE_THREADS

#ifndef WIN32
#   include <unistd.h>
#   define TIMING
#   if defined(TIMING)
#      include <sys/time.h>
static struct timeval start_tv, end_tv;
#   endif

#   define SLASH "/"
#    define __open open
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
#define DOTSOMO      ""
#define DOTSOMOCAP   ""

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#if defined(TO_DO)
static void outward_translate_1_sphere_1_fixed(
                                               float *r1,  // radius of sphere 1
                                               float *r2,  // radius of sphere 2
                                               float *x1, // center of sphere 1
                                               float *x2, // center of sphere 2
                                               float *v1 // normalized vector to sphere 1 from COG
                                               ) {
#if defined(DEBUG)
   printf("outward_translate_1_sphere_1_fixed\n"
          "sphere 1 radius %f center [%f,%f,%f] cog vector [%f,%f,%f]\n"
          "sphere 2 radius %f center [%f,%f,%f]\n",
          *r1, x1[0], x1[1], x1[2], v1[0], v1[1], v1[2],
          *r2, x2[0], x2[1], x2[2]);
#endif

   float k;
   float denom =
      -2 * *r1 * *r2 - *r2 * *r2 +
      *r1 * *r1 * (-1 + v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]);

   float sfact =
      sqrt(
           (*r1 + *r2) * (*r1 + *r2) *
           ((-2 * *r1 * *r2 - *r2 * *r2 +
             *r1 * *r1 * (-1 + v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]))
            * (
               (*r1 + *r2) * (*r1 + *r2) -
               (x1[0] - x2[0]) * (x1[0] - x2[0]) -
               (x1[1] - x2[1]) * (x1[1] - x2[1]) -
               (x1[2] - x2[2]) * (x1[2] - x2[2])) +
            pow(*r1 * *r1 + *r2 * *r2 +
                *r1 * (
                       2 * *r2 + v1[0] * x1[0] + v1[1] * x1[1] + v1[2] * x1[2] -
                       v1[0] * x2[0] - v1[1] * x2[1] - v1[2] * x2[2]),2)));

   float b =
      - (*r1 + *r2) *
      (*r1 * *r1 + *r2 * *r2 +
       *r1 * (2 * *r2 + v1[0] * x1[0] + v1[1] * x1[1] + v1[2] * x1[2] -
              v1[0] * x2[0] - v1[1] * x2[1] - v1[2] * x2[2]));

   k = (b + sfact) / denom;

   float k1 = k * *r1 / (*r1 + *r2);
   float k2 = k * *r2 / (*r1 + *r2);

   x1[0] += k1 * v1[0];
   x1[1] += k1 * v1[1];
   x1[2] += k1 * v1[2];

   *r1 -= k1;
   *r2 -= k2;

#if defined(DEBUG)

   printf("k %.3f\n", k);

   printf("new coordinates, radius\n"
          "sphere 1 radius %.3f center [%.3f,%.3f,%.3f]\n"
          "sphere 2 radius %.3f center [%.3f,%.3f,%.3f]\n"
          "r1 + r2 - distance between spheres = %f\n",
          *r1,
          x1[0],
          x1[1],
          x1[2],
          *r2,
          x2[0],
          x2[1],
          x2[2],
          *r1 + *r2 -
          sqrt(
               (x1[0] - x2[0]) * (x1[0] - x2[0]) +
               (x1[1] - x2[1]) * (x1[1] - x2[1]) +
               (x1[2] - x2[2]) * (x1[2] - x2[2]))
          );
#endif

}
#endif

static void outward_translate_2_spheres(float *r1, // radius of sphere 1
                                        float *r2, // radius of sphere 2
                                        float *x1, // center of sphere 1
                                        float *x2, // center of sphere 2
                                        float *v1, // normalized vector to sphere 1 from COG
                                        float *v2  // normalized vector to sphere 1 from COG
                                        ) {
#if defined(DEBUG) || defined(DEBUG1)
   printf("outward_translate_2_spheres\n"
          "sphere 1 radius %.3f center [%.3f,%.3f,%.3f] cog vector [%.3f,%.3f,%.3f]\n"
          "sphere 2 radius %.3f center [%.3f,%.3f,%.3f] cog vector [%.3f,%.3f,%.3f]\n",
          *r1, x1[0], x1[1], x1[2], v1[0], v1[1], v1[2],
          *r2, x2[0], x2[1], x2[2], v2[0], v2[1], v2[2]);
#endif

   float k;
   float denom =
      2 * *r1 * *r2 * (1 + v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]) -
      *r1 * *r1 * (-1 + v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]) -
      *r2 * *r2 * (-1 + v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2]);

   float sfact =
      sqrt(
           (double)(*r1 + *r2) * (*r1 + *r2) *
           ((double)(*r1 * *r1 * (-1 + v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]) -

             2 * *r1 * *r2 * (1 + v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]) +

             *r2 * *r2 * (-1 + v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2])) *

            ((*r1 + *r2) * (*r1 + *r2) -
             (x1[0] - x2[0]) * (x1[0] - x2[0]) -
             (x1[1] - x2[1]) * (x1[1] - x2[1]) -
             (x1[2] - x2[2]) * (x1[2] - x2[2])) +

            pow(*r1 * *r1 +
                *r1 * (
                       2 * *r2 + v1[0] * x1[0] + v1[1] * x1[1] + v1[2] * x1[2] -
                       v1[0] * x2[0] - v1[1] * x2[1] - v1[2] * x2[2]) +
                *r2 * (
                       *r2 - v2[0] * x1[0] - v2[1] * x1[1] - v2[2] * x1[2] +
                       v2[0] * x2[0] + v2[1] * x2[1] + v2[2] * x2[2]),2)));

   float b =
      (*r1 + *r2) *
      (*r1 * *r1 +
       *r1 * (2 * *r2 + v1[0] * x1[0] + v1[1] * x1[1] + v1[2] * x1[2] -
              v1[0] * x2[0] - v1[1] * x2[1] - v1[2] * x2[2]) +
       *r2 * (*r2 - v2[0] * x1[0] - v2[1] * x1[1] - v2[2] * x1[2] + v2[0] * x2[0] +
              v2[1] * x2[1] + v2[2] * x2[2]));

   k = (b - sfact) / denom;

   float k1 = k * *r1 / (*r1 + *r2);
   float k2 = k * *r2 / (*r1 + *r2);

   x1[0] += k1 * v1[0];
   x1[1] += k1 * v1[1];
   x1[2] += k1 * v1[2];

   x2[0] += k2 * v2[0];
   x2[1] += k2 * v2[1];
   x2[2] += k2 * v2[2];

   *r1 -= k1;
   *r2 -= k2;

#if defined(DEBUG) || defined(DEBUG1)

   printf("k %.3f\n", k);

   printf("new coordinates, radius\n"
          "sphere 1 radius %.3f center [%.3f,%.3f,%.3f]\n"
          "sphere 2 radius %.3f center [%.3f,%.3f,%.3f]\n"
          "r1 + r2 - distance between spheres = %f\n",
          *r1,
          x1[0],
          x1[1],
          x1[2],
          *r2,
          x2[0],
          x2[1],
          x2[2],
          *r1 + *r2 -
          sqrt(
               (x1[0] - x2[0]) * (x1[0] - x2[0]) +
               (x1[1] - x2[1]) * (x1[1] - x2[1]) +
               (x1[2] - x2[2]) * (x1[2] - x2[2]))
          );
#endif
}

void US_Hydrodyn::get_atom_map(PDB_model *model)
{
   atom_counts.clear( );
   has_OXT.clear( );

   for (unsigned int j = 0; j < model->molecule.size(); j++)
   {
      QString lastResSeq = "";
      QString lastResName = "";
      int atom_count = 0;
      for (unsigned int k = 0; k < model->molecule[j].atom.size(); k++)
      {
         PDB_atom *this_atom = &(model->molecule[j].atom[k]);
         if (lastResSeq != this_atom->resSeq ||
             lastResName != this_atom->resName)
         {
            // new residue
            if ( lastResSeq != "" )
            {
               atom_counts[QString("%1|%2|%3")
                           .arg(j)
                           .arg(lastResName)
                           .arg(lastResSeq)] = atom_count;
            }
            lastResSeq = this_atom->resSeq;
            lastResName = this_atom->resName;
            atom_count = 0;
         }
         if(this_atom->name == "OXT") {
            has_OXT[QString("%1|%2|%3")
                    .arg(j)
                    .arg(this_atom->resName)
                    .arg(this_atom->resSeq)]++;
         }
         atom_count++;
      }
      if ( lastResSeq != "" )
      {
         atom_counts[QString("%1|%2|%3")
                     .arg(j)
                     .arg(lastResName)
                     .arg(lastResSeq)] = atom_count;
      }
   }

   // pass for broken chains
   // later add distance check for CA-N ?
   broken_chain_end.clear( );
   broken_chain_head.clear( );
   int breaks = 0;
   int total_aa = 0;
   for (unsigned int j = 0; j < model->molecule.size(); j++)
   {
      QString lastResSeq = "";
      QString lastResName = "";
      QString lastChainID = "";
      // count non AA types
      int non_aa = 0;
      int aa = 0;
      for (unsigned int k = 0; k < model->molecule[j].atom.size(); k++)
      {
         PDB_atom *this_atom = &(model->molecule[j].atom[k]);
         if ( lastResSeq != this_atom->resSeq ||
              lastResName != this_atom->resName )
         {
            if ( multi_residue_map.count(this_atom->resName) &&
                 multi_residue_map[this_atom->resName].size() && 
                 multi_residue_map[this_atom->resName][0] >= 0 &&
                 multi_residue_map[this_atom->resName][0] < (int)residue_list.size() &&
                 residue_list[multi_residue_map[this_atom->resName][0]].type == 0 )
            {
               aa++;
            }
            else
            {
               non_aa++;
            }
         }
      }
      if ( aa ) 
      {
         lastResSeq = "";
         lastResName = "";
         non_aa = 0;
         aa = 0;
         for (unsigned int k = 0; k < model->molecule[j].atom.size(); k++)
         {
            PDB_atom *this_atom = &(model->molecule[j].atom[k]);
            
            if ( lastResSeq != this_atom->resSeq ||
                 lastResName != this_atom->resName )
            {
               if ( multi_residue_map.count(this_atom->resName) &&
                    multi_residue_map[this_atom->resName].size() &&
                    (int) residue_list.size() > multi_residue_map[this_atom->resName][0] &&
                    residue_list[multi_residue_map[this_atom->resName][0]].type == 0 )
               {
                  aa++;
                  total_aa++;
               }
               else
               {
                  non_aa++;
               }
               if ( advanced_config.debug_2 )
               {
                  printf("unknown_residues.count(%s) %u\n",
                         this_atom->resName.toLatin1().data(),
                         (unsigned int)unknown_residues.count(this_atom->resName));
               }
               if ( 
                   ( lastResSeq != "" &&
                     ( lastResSeq.toInt() + 1 !=  this_atom->resSeq.toInt() ||
                       !multi_residue_map.count(this_atom->resName) ||
                       unknown_residues.count(this_atom->resName) ||
                       unknown_residues.count(lastResName) ) ) ||
                   ( lastResSeq == "" &&
                     unknown_residues.count(this_atom->resName) 
                     )
                   )
               {
                  breaks++;
                  if ( lastResSeq != "" )
                  {
                     broken_chain_end[QString("%1|%2")
                                      .arg(lastResSeq)
                                      .arg(lastResName)] = true;
                  } 
                  else
                  {
                     broken_chain_end[QString("%1|%2")
                                      .arg(this_atom->resSeq)
                                      .arg(this_atom->resName)] = true;
                  }
                  
                  broken_chain_head[QString("%1|%2")
                                    .arg(this_atom->resSeq)
                                    .arg(this_atom->resName)] = true;
                  
                  //               if ( multi_residue_map.count(this_atom->resName) &&
                  //   residue_list[multi_residue_map[this_atom->resName][0]].type == 0 &&
                  //   multi_residue_map.count(lastResName) &&
                  //   residue_list[multi_residue_map[lastResName][0]].type == 0 )
                  {
                     QColor save_color = editor->textColor();
                     editor->setTextColor("dark red");
                     editor->append(
                                    lastResSeq != "" ? 
                                    QString(us_tr("Warning: break in residue sequence or unknown residue: %1Molecule %2 Residue %3 %4 & %5 %6."))
                                    .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                    .arg(j + 1)
                                    .arg(lastResName)
                                    .arg(lastResSeq)
                                    .arg(this_atom->resName)
                                    .arg(this_atom->resSeq)
                                    :
                                    QString(us_tr("Warning: break in residue sequence or unknown residue: %1Molecule %2 Residue %3 %4."))
                                    .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                    .arg(j + 1)
                                    .arg(this_atom->resName)
                                    .arg(this_atom->resSeq)
                                    );
                     editor->setTextColor(save_color);
                  }
               }
               lastChainID = this_atom->chainID;
               lastResSeq = this_atom->resSeq;
               lastResName = this_atom->resName;
            }
         }
      }
      if ( aa && non_aa )
      {
         QColor save_color = editor->textColor();
         editor->setTextColor("dark magenta");
         editor->append(
                        QString(us_tr("Notice: %1found %2 non or unknown Amino Acids in a chain containing %3 AA Residues."))
                        .arg(lastChainID == " " ? "" : ("Chain " + lastChainID + " "))
                        .arg(non_aa)
                        .arg(aa)
                        );
         editor->setTextColor(save_color);
      }
   }
   // should be chain based
   if ( !total_aa || !advanced_config.pbr_broken_logic )
   {
      broken_chain_end.clear( );
      broken_chain_head.clear( );
   }

#if defined(DEBUG_MULTI_RESIDUE)
   for (map < QString, int >::iterator it = atom_counts.begin();
        it != atom_counts.end();
        it++)
   {
      printf("atom count map %s map pos %d\n",
             it->first.toLatin1().data(), it->second);
   }
   for (map < QString, int >::iterator it = has_OXT.begin();
        it != has_OXT.end();
        it++)
   {
      printf("has_OXT map %s map value %d\n",
             it->first.toLatin1().data(), it->second);
   }
#endif
   // end of atom_counts & has_OXT map create
}

void US_Hydrodyn::build_molecule_maps(PDB_model *model)
{
   // creates molecules_residues_atoms map
   // molecules_residues_atoms maps molecule #|resSeq to vector of atom names
   molecules_residues_atoms.clear( );
   // molecules_residue_name maps molecule #|resSeq to residue name
   molecules_residue_name.clear( );
   // molecules_idx_seq is a vector of the idx's
   molecules_idx_seq.clear( );
   molecules_residue_errors.clear( );
   molecules_residue_missing_counts.clear( );
   molecules_residue_min_missing.clear( );
   molecules_residue_missing_atoms.clear( );
   molecules_residue_missing_atoms_beads.clear( );
   molecules_residue_missing_atoms_skip.clear( );
   use_residue.clear( );
   skip_residue.clear( );

   // pass 1 setup molecule basic maps
   for (unsigned int j = 0; j < model->molecule.size(); j++)
   {
      for (unsigned int k = 0; k < model->molecule[j].atom.size(); k++)
      {
         QString idx = QString("%1|%2").arg(j).arg(model->molecule[j].atom[k].resSeq);
         molecules_residues_atoms[idx].push_back(model->molecule[j].atom[k].name);
         if ( !molecules_residue_name.count(idx) )
         {
            molecules_residue_name[idx] = model->molecule[j].atom[k].resName;
            molecules_idx_seq.push_back(idx);
         }
      }
   }

   // pass 2 setup error maps
   for (unsigned int i = 0; i < molecules_idx_seq.size(); i++)
   {
      QString idx = molecules_idx_seq[i];
      QString resName = molecules_residue_name[idx];
      if (multi_residue_map.count(resName))
      {
         int minimum_missing = INT_MAX; // find the entry with the minimum # of missing atoms
         int minimum_missing_pos = 0;   // and its position
         for (unsigned int j = 0; j < multi_residue_map[resName].size(); j++)
         {
            QString error_msg = "";
            int missing_count = 0;
            // for this residue, clear flags
            for (unsigned int r = 0; 
                 r < residue_list[multi_residue_map[resName][j]].r_atom.size(); 
                 r++)
            {
               residue_list[multi_residue_map[resName][j]].r_atom[r].tmp_used = false;
            }
            skip_residue[QString("%1|%2").arg(idx).arg(multi_residue_map[resName][j])] = false;

            // now set flags
            // first check for non-coded atoms
            for (unsigned int k = 0; k < molecules_residues_atoms[idx].size(); k++)
            {
               bool found = false;
               bool any = false;
               for (unsigned int r = 0; 
                    !found && r < residue_list[multi_residue_map[resName][j]].r_atom.size(); 
                    r++)
               {
                  if (residue_list[multi_residue_map[resName][j]].r_atom[r].name == 
                      molecules_residues_atoms[idx][k]) 
                  {
                     any = true;
                     if (!residue_list[multi_residue_map[resName][j]].r_atom[r].tmp_used)
                     {
                        residue_list[multi_residue_map[resName][j]].r_atom[r].tmp_used = true;
                        found = true;
                     }
                  }
               }
               if (!found)
               {
                  missing_count = -1;
                  error_msg += QString("%1coded atom %2. ")
                     .arg(any ? "Duplicate " : "Non-")
                     .arg(molecules_residues_atoms[idx][k]);
               }
            }
            // now check for missing atoms
            vector < QString > missing_atoms;
            vector < unsigned int > missing_atoms_beads;
            for (unsigned int r = 0; 
                 r < residue_list[multi_residue_map[resName][j]].r_atom.size(); 
                 r++)
            {
               if (!residue_list[multi_residue_map[resName][j]].r_atom[r].tmp_used)
               {
                  error_msg += QString("Missing atom %1. ")
                     .arg(residue_list[multi_residue_map[resName][j]].r_atom[r].name);
                  if ( missing_count != -1 )
                  {
                     missing_atoms.push_back(residue_list[multi_residue_map[resName][j]].r_atom[r].name);
                     missing_atoms_beads.push_back(residue_list[multi_residue_map[resName][j]].r_atom[r].bead_assignment);
                     molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                          .arg(idx)
                                                          .arg(j)
                                                          .arg(r)] = true;
                     missing_count++;
                  }
               }
            }
            molecules_residue_errors[idx].push_back(error_msg);
            molecules_residue_missing_counts[idx].push_back(missing_count);
            molecules_residue_missing_atoms[idx].push_back(missing_atoms);
            molecules_residue_missing_atoms_beads[idx].push_back(missing_atoms_beads);
            if ( missing_count >= 0 && missing_count < minimum_missing )
            {
               minimum_missing = missing_count;
               minimum_missing_pos = j;
            }
         }
         if ( minimum_missing < INT_MAX )
         {
            molecules_residue_min_missing[idx] = minimum_missing_pos;
         } 
         else
         {
            molecules_residue_min_missing[idx] = -1;
         }
      }
      else 
      {
         molecules_residue_errors[idx].push_back("Non-coded residue. ");
         molecules_residue_missing_counts[idx].push_back(-1);
         molecules_residue_min_missing[idx] = -1;
         unknown_residues[resName] = true;
         
         if ( advanced_config.debug_2 )
         {
            printf("unknown residue %s\n", resName.toLatin1().data());
         }
      }
      {
         int k = molecules_residue_min_missing[idx];
         if ( k > -1  &&
              molecules_residue_missing_atoms[idx][k].size() == 0 &&
              molecules_residue_missing_atoms_beads[idx][k].size() == 0
              )
         {
            if ( advanced_config.debug_1 )
            {
               cout << 
                  QString("Molecule idx <%1> resName <%2> pos <%3> no errors, should be ok\n")
                  .arg(idx).arg(resName).arg(k);
            }
            use_residue[idx] = multi_residue_map[resName][k];
            // skip the rest
            for (unsigned int j = 0; j < multi_residue_map[resName].size(); j++)
            {
               if ( j != (unsigned int) k )
               {
                  skip_residue[QString("%1|%2").arg(idx).arg(multi_residue_map[resName][j])] = true;
                  if ( advanced_config.debug_1 )
                  {
                     cout << 
                        QString("Setup skip idx %1|%2\n")
                        .arg(idx).arg(multi_residue_map[resName][j]);
                  }
               }
            }
            molecules_residue_errors[idx].clear( );
            molecules_residue_missing_counts[idx].clear( );
            molecules_residue_missing_atoms[idx].clear( );
            molecules_residue_missing_atoms_beads[idx].clear( );
         } else {
            use_residue[idx] = -1;
         }
      }
   }

   if ( advanced_config.debug_1 )
   {
      cout << "--------molecules_residue_errors---------\n";
      for (unsigned int i = 0; i < molecules_idx_seq.size(); i++)
      {
         QString idx = molecules_idx_seq[i];
         QString resName = molecules_residue_name[idx];
         if (molecules_residue_errors[idx].size())
         {
            for (unsigned int j = 0; j < molecules_residue_errors[idx].size(); j++) 
            {
               if (molecules_residue_errors[idx][j].length()) 
               {
                  cout << QString("Molecule idx <%1> resName <%2> match <%3> errors:\n")
                     .arg(idx).arg(resName).arg(j);
                  cout << molecules_residue_errors[idx][j] << endl;
               } else {
                  cout << QString("Molecule idx <%1> resName <%2> match <%3> good match\n")
                     .arg(idx).arg(resName).arg(j);
               }
            }
            if ( molecules_residue_min_missing[idx] > -1 )
            {
               int k = molecules_residue_min_missing[idx];
               cout << QString("^^^ Minimum missing position <%1> ma.sz %2 mab.sz %3\n")
                  .arg(k)
                  .arg(molecules_residue_missing_atoms[idx][k].size())
                  .arg(molecules_residue_missing_atoms_beads[idx][k].size());
               for ( unsigned int j = 0; j < molecules_residue_missing_atoms[idx][k].size(); j++ )
               {
                  cout << QString(".... missing atom %1 bead %2\n")
                     .arg(molecules_residue_missing_atoms[idx][k][j])
                     .arg(molecules_residue_missing_atoms_beads[idx][k][j]);
               }
            }
         }
      }
   }
}

int US_Hydrodyn::check_for_missing_atoms(QString *error_string, PDB_model *model)
{
   // go through molecules, build vector of residues
   // expand vector of residues to atom list from residue file
   // compare expanded list of residues to model ... list missing atoms missing
   int errors_found = 0;
   get_atom_map(model);
   if ( misc.pb_rule_on &&
        (broken_chain_head.size() || broken_chain_end.size()) )
   {
      misc.pb_rule_on = false;
      misc.restore_pb_rule = true;
      // us_qdebug( "check_for_missing_atoms() setting restore_pb_rule" );
      editor->append("Broken chain turns off Peptide Bond Rule.\n");
      if ( misc_widget ) {
         misc_window->close();
         delete misc_window;
         misc_widget = false;
      }
      if ( advanced_config.debug_1 )
      {
         printf("Broken chain turns off Peptide Bond Rule.\n");
      }
      display_default_differences();
      reload_pdb();
      get_atom_map(model);
   }

   int failure_errors = 0;
   bead_exceptions.clear( );

   // residue types are for automatic build builder to determine
   // if we have a protein, so that a special 'temporary' residue can
   // be created

   vector < map < unsigned int, unsigned int > > residue_types;
   vector < unsigned int > last_residue_type;

   residue_types.resize(model->molecule.size());
   last_residue_type.resize(model->molecule.size());

   build_molecule_maps(model);
   QString abb_msgs = "";

   // keep track of errors shown
   map < QString, bool > error_shown;

   for (unsigned int j = 0; j < model->molecule.size(); j++)
   {
      QString lastResSeq = "";
      int lastResPos = -1;
      QString lastChainID = " ";
      bool spec_N1 = false;
      QString last_count_idx;
      unsigned int residues_found = 0;
      unsigned int N1s_placed = 0;
      for ( unsigned int k = 0; k < model->molecule[j].atom.size(); k++ )
      {
         PDB_atom *this_atom = &(model->molecule[j].atom[k]);
         this_atom->active = false;
         QString count_idx =
            QString("%1|%2|%3")
            .arg(j)
            .arg(this_atom->resName)
            .arg(this_atom->resSeq);

         QString res_idx =
            QString("%1|%2")
            .arg(j)
            .arg(this_atom->resSeq);

         if (!bead_exceptions[count_idx])
         {
            bead_exceptions[count_idx] = 1;
         }

         // find residue in residues
         if ( advanced_config.debug_1 )
         {
            printf("check_for_missing_atoms search name %s resName %s\n",
                   this_atom->name.toLatin1().data(),
                   this_atom->resName.toLatin1().data());
         }
         int respos = -1;
         int restype = 99;
         if ( multi_residue_map.count(this_atom->resName) &&
              multi_residue_map[this_atom->resName].size() )
         {
            restype = residue_list[multi_residue_map[this_atom->resName][0]].type;
         }

         for ( unsigned int m = 0; m < residue_list.size(); m++ )
         {
            if ( residue_list[m].name == this_atom->resName &&
                 skip_residue.count(QString("%1|%2").arg(res_idx).arg(m)) &&
                 skip_residue[QString("%1|%2").arg(res_idx).arg(m)] == true )
            {
               if ( advanced_config.debug_1 )
               {
                  cout << 
                     QString("(check for missing atoms) skipping %1 %2\n").arg(res_idx).arg(m) << endl;
               }
               continue;
            }
            if ((residue_list[m].name == this_atom->resName &&
                 (int)residue_list[m].r_atom.size() ==
                 atom_counts[count_idx] - has_OXT[count_idx] &&
                 this_atom->name != "OXT" &&
                 (k ||
                  this_atom->name != "N" || 
                  restype != 0 ||
                  broken_chain_head.count(QString("%1|%2")
                                          .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) ||
                  this_atom->resName == "PRO" ||
                  !misc.pb_rule_on)) ||
                
                // if pb_rule is off, final OXT before P needs to use OXT-P

                (residue_list[m].name == "OXT"
                 && this_atom->name == "OXT" && (misc.pb_rule_on || this_atom->resName != "PRO")) ||
                (residue_list[m].name == "OXT-P"
                 && (this_atom->name == "OXT" && !misc.pb_rule_on && this_atom->resName == "PRO")) ||

                (!k &&
                 this_atom->name == "N" &&
                 restype == 0 &&
                 misc.pb_rule_on &&
                 residue_list[m].name == "N1" &&
                 !broken_chain_head.count(QString("%1|%2")
                                          .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) &&
                 this_atom->resName != "PRO"))
            {
               respos = (int) m;
               residue_types[j][residue_list[m].type]++;
               last_residue_type[j] = residue_list[m].type;
#if defined(DEBUG_MULTI_RESIDUE)
               printf("atom name %s residue name %s pos %u atom.size() %u map %s size %u has oxt %d\n"
                      ,this_atom->name.toLatin1().data()
                      ,this_atom->resName.toLatin1().data()
                      ,m
                      ,residue_list[m].r_atom.size()
                      ,QString("%1|%2|%3")
                      .arg(j)
                      .arg(this_atom->resName)
                      .arg(this_atom->resSeq)
                      .toLatin1().data()
                      ,atom_counts[count_idx]
                      ,has_OXT[count_idx]
                      );
#endif

               this_atom->p_residue = &(residue_list[m]);
               if ( advanced_config.debug_1 )
               {
                  
                  printf("residue match %d resName %s \n", m, residue_list[m].name.toLatin1().data());
                  printf("resname %s respos %d mappos %d mapsize %u\n"
                         ,this_atom->resName.toLatin1().data()
                         ,m
                         ,multi_residue_map[this_atom->resName][0]
                         ,(unsigned int)multi_residue_map[this_atom->resName].size()
                         );
               }
               if (lastResSeq != this_atom->resSeq)
               {
                  // new residue
                  // printf("new residue %s\n", this_atom->resSeq.toLatin1().data());
                  residues_found++;
                  if (lastResPos != -1)
                  {
                     // check for false entries in last residue and warn about them
                     for (unsigned int l = 0; l < residue_list[lastResPos].r_atom.size(); l++)
                     {
                        if (spec_N1 &&
                            misc.pb_rule_on &&
                            residue_list[lastResPos].r_atom[l].name == "N") {
                           residue_list[lastResPos].r_atom[l].tmp_flag = true;
                           spec_N1 = false;
                        }

                        if (!residue_list[lastResPos].r_atom[l].tmp_flag)
                        {
                           // missing atoms
                           errors_found++;
                           if (pdb_parse.missing_atoms == 0)
                           {
                              failure_errors++;
                           }
                           if (pdb_parse.missing_atoms == 1)
                           {
                              bead_exceptions[last_count_idx] = 2;
                           }
                           if (pdb_parse.missing_atoms == 2)
                           {
                              if ( bead_exceptions[last_count_idx] == 1 ) 
                              {
                                 bead_exceptions[last_count_idx] = 4;
                              }
                           }
                           if (!error_shown[last_count_idx]) 
                           {
                              QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                                 .arg(lastChainID == " " ? "" : ("Chain " + lastChainID + " "))
                                 .arg(j + 1)
                                 .arg(residue_list[lastResPos].name)
                                 .arg(lastResSeq);
                              QString idx = QString("%1|%2").arg(j).arg(lastResSeq);
                              switch (molecules_residue_errors[idx].size())
                              {
                              case 0: 
                                 this_error += QString("Missing atom %1.\n").arg(residue_list[lastResPos].r_atom[l].name);
                                 break;
                              case 1:
                                 this_error += molecules_residue_errors[idx][0] + "\n";
                                 break;
                              default :
                                 {
                                    this_error += "\n";
                                    for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                                    {
                                       if ( advanced_config.debug_1 )
                                       {
                                          this_error += " d1";
                                       }
                                       this_error += QString("    Residue file entry %1: %2\n").
                                          arg(t+1).arg(molecules_residue_errors[idx][t]);
                                    }
                                 }
                                 break;
                              }
                              error_string->append(this_error);
                              cout << QString("dbg 1: idx <%1> msg <%2>\n").arg(last_count_idx).arg(this_error);
                              error_shown[last_count_idx] = true;
                           }
                           // error_string->
                           // append(QString("").
                           //     sprintf("Missing atom: chain %s molecule %d atom %s residue %s %s\n",
                           //        lastChainID.toLatin1().data(),
                           //        j + 1,
                           //        residue_list[lastResPos].r_atom[l].name.toLatin1().data(),
                           //        lastResSeq.toLatin1().data(),
                           //        residue_list[lastResPos].name.toLatin1().data()));
                        }
                     }
                  }

                  // reset residue list
                  // printf("reset residue list for residue_list[%d]\n", m);
                  for (unsigned int l = 0; l < residue_list[m].r_atom.size(); l++)
                  {
                     residue_list[m].r_atom[l].tmp_flag = false;
                  }
                  lastResSeq = this_atom->resSeq;
                  lastChainID = this_atom->chainID;
                  lastResPos = (int) m;
                  last_count_idx = count_idx;
               }

               if (residue_list[m].name == "N1")
               {
                  lastResSeq = "";
                  lastResPos = -1;
                  spec_N1 = true;
                  N1s_placed++;
                  residues_found--;
               }
               break;
            }
         }
         if (respos == -1)
         {
            if ((this_atom->name != "H" && this_atom->name != "D"
                 && this_atom->resName != "DOD"
                 && this_atom->resName != "HOH" && (this_atom->altLoc == "A" || this_atom->altLoc == " ")))
            {
               QString msg_tag;
               bool do_error_msg = true;
               if (pdb_parse.missing_residues == 0 &&
                   pdb_parse.missing_atoms == 0)
               {
                  failure_errors++;
                  msg_tag = "Missing residue or atom";
               } 
               else 
               {
                  // ok, we have three cases here:
                  // 1. residue does exist & residue/atom doesn't
                  //    1.1 skip missing atoms controls
                  // 2. residue does exist & residue/atom does
                  //    2.1 there must be a missing atom since count doesn't match so atom controls
                  // 3. residue does not exist
                  //    3.1 skip missing residue control
                  // ---------------------
                  //
                  // note: we're just checking the 1st of multiple possibilities
                  puts("cases---");
                  if (multi_residue_map[this_atom->resName].size()) 
                  {
                     // residue exists, does residue/atom?
                     QString idx = QString("%1|%2|%3")
                        .arg(this_atom->resName)
                        .arg(this_atom->name)
                        .arg(0); 
                     printf("cases residue found: idx %s\n", idx.toLatin1().data());
                     if (valid_atom_map[idx].size()) 
                     {
                        puts("case 2.1");
                        msg_tag = "Missing or extra atom in residue";
                     } 
                     else
                     {
                        // atom does not exist, skip missing atoms controls
                        puts("case 1.1");
                        msg_tag = "Missing atom";
                     }
                     
                     switch( issue_missing_atom() ) {
                     case ISSUE_RESPONSE_STOP :
                        failure_errors++;
                        break;
                        
                     case ISSUE_RESPONSE_MA_SKIP :
                        bead_exceptions[count_idx] = 2;
                        break;

                     case ISSUE_RESPONSE_MA_MODEL :
                        if ( molecules_residue_min_missing[QString("%1|%2").arg(j).arg(this_atom->resSeq)] == -1 ) {
                           bead_exceptions[count_idx] = 3;
                        } else {
                           if ( bead_exceptions[count_idx] == 1 ) 
                           {
                              bead_exceptions[count_idx] = 4;
                           }
                        }
                        break;
                     }

                     // if (pdb_parse.missing_atoms == 0)
                     // {
                     //    failure_errors++;
                     // }
                     // if (pdb_parse.missing_atoms == 1)
                     // {
                     //    bead_exceptions[count_idx] = 2;
                     // }
                     // if (pdb_parse.missing_atoms == 2)
                     // {
                     //    if ( molecules_residue_min_missing[QString("%1|%2").arg(j).arg(this_atom->resSeq)] == -1 )
                     //    {
                     //       // fall back to non-coded residue 
                     //       switch ( pdb_parse.missing_residues )
                     //       {
                     //       case 0 :
                     //          failure_errors++;
                     //          break;
                     //       case 1 :
                     //          bead_exceptions[count_idx] = 2;
                     //          break;
                     //       case 2 :
                     //          bead_exceptions[count_idx] = 3;
                     //          break;
                     //       default :
                     //          failure_errors++;
                     //          US_Static::us_message(us_tr("Internal error:"), 
                     //                               us_tr("Unexpected pdb_parse.missing residues type ") +
                     //                               QString("%1\n").arg(pdb_parse.missing_residues));
                     //          exit(-1);
                     //          break;
                     //       }
                     //    } 
                     //    else
                     //    {
                     //       if ( bead_exceptions[count_idx] == 1 ) 
                     //       {
                     //          bead_exceptions[count_idx] = 4;
                     //       }
                     //    }
                     // }
                  } 
                  else 
                  {
                     // residue does not exist, skip missing residue controls
                     puts("case 3.1");
                     msg_tag = "Non-coded residue";
                     switch( issue_non_coded() ) {
                     case ISSUE_RESPONSE_STOP :
                        failure_errors++;
                        break;
                        
                     case ISSUE_RESPONSE_NC_SKIP :
                        bead_exceptions[count_idx] = 2;
                        break;

                     case ISSUE_RESPONSE_NC_REPLACE :
                        bead_exceptions[count_idx] = 3;
                        break;
                        
                     }

                     // if (pdb_parse.missing_residues == 0)
                     // {
                     //    failure_errors++;
                     // }
                     // if (pdb_parse.missing_residues == 1)
                     // {
                     //    bead_exceptions[count_idx] = 2;
                     // }
                     // if (pdb_parse.missing_residues == 2)
                     // {
                     //    bead_exceptions[count_idx] = 3;
                     // }
                  }
               }
               if (do_error_msg) {
                  errors_found++;
                  if (!error_shown[count_idx]) 
                  {
                     QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                        .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                        .arg(j + 1)
                        .arg(this_atom->resName)
                        .arg(this_atom->resSeq);
                     QString idx = QString("%1|%2").arg(j).arg(this_atom->resSeq);
                     switch (molecules_residue_errors[idx].size())
                     {
                     case 0: 
                        this_error += msg_tag;
                        break;
                     case 1:
                        this_error += molecules_residue_errors[idx][0] + "\n";
                        break;
                     default :
                        {
                           this_error += "\n";
                           for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                           {
                              if ( advanced_config.debug_1 )
                              {
                                 this_error += " d2";
                              }
                              this_error += QString("    Residue file entry %1: %2\n").
                                 arg(t+1).arg(molecules_residue_errors[idx][t]);
                           }
                        }
                        break;
                     }
                     error_string->append(this_error);
                     cout << QString("dbg 2: idx <%1> msg <%2>\n").arg(count_idx).arg(this_error);
                     error_shown[count_idx] = true;
                  }
        
                  // error_string->append(QString("").sprintf("%s: chain %s molecule %d atom %s residue %s %s\n",
                  //               msg_tag.toLatin1().data(),
                  //               this_atom->chainID.toLatin1().data(),
                  //               j + 1,
                  //               this_atom->name.toLatin1().data(),
                  //               this_atom->resSeq.toLatin1().data(),
                  //               this_atom->resName.toLatin1().data()
                  //               ));
               }
            }
         }
         else 
         {
            // find atom in residues atoms
            int atompos = -1;
            if ( advanced_config.debug_1 )
            {
               printf("now find atom in residues atoms\n");
            }
            for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
            {
               if (residue_list[respos].r_atom[m].name == this_atom->name ||
                   (
                    this_atom->name == "N" &&
                    misc.pb_rule_on &&
                    !k &&
                    residue_list[respos].r_atom[m].name == "N1" &&
                    !broken_chain_head.count(QString("%1|%2")
                                             .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) &&
                    this_atom->resName != "PRO"
                    )
                   )
               {
                  this_atom->active = true;
                  residue_list[respos].r_atom[m].tmp_flag = true;
                  atompos = (int) m;
                  break;
               }
            }
            if (atompos == -1)
            {
               errors_found++;
               // valid residue, but non-coded atom
               switch ( pdb_parse.missing_atoms )
               {
               case 0 :
                  failure_errors++;
                  break;
               case 1 :
                  bead_exceptions[count_idx] = 2;
                  break;
               case 2 :
                  // resort to missing_residue controls
                  if ( bead_exceptions[count_idx] == 1 ) 
                  {
                     switch ( pdb_parse.missing_residues )
                     {
                     case 0 :
                        failure_errors++;
                        break;
                     case 1 :
                        bead_exceptions[count_idx] = 2;
                        break;
                     case 2 :
                        bead_exceptions[count_idx] = 3;
                        break;
                     default :
                        failure_errors++;
                        US_Static::us_message(us_tr("Internal error:"), 
                                             us_tr("Unexpected pdb_parse.missing residues type ") +
                                             QString("%1\n").arg(pdb_parse.missing_residues));
                        exit(-1);
                        break;
                     }
                  } 
                  break;
               default :
                  failure_errors++;
                  US_Static::us_message(us_tr("Internal error:"), 
                                       us_tr("Unexpected pdb_parse.missing residues type ") +
                                       QString("%1\n").arg(pdb_parse.missing_residues));
                  exit(-1);
                  break;
               }

               if (!error_shown[count_idx]) 
               {
                  QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                     .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                     .arg(j + 1)
                     .arg(this_atom->resName)
                     .arg(this_atom->resSeq);
                  QString idx = QString("%1|%2").arg(j).arg(this_atom->resSeq);
                  switch (molecules_residue_errors[idx].size())
                  {
                  case 0: 
                     this_error += QString("Unknown atom %1.\n").arg(this_atom->name);
                     break;
                  case 1:
                     this_error += molecules_residue_errors[idx][0] + "\n";
                     break;
                  default :
                     {
                        this_error += "\n";
                        for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                        {
                           if ( advanced_config.debug_1 )
                           {
                              this_error += " d3";
                           }
                           this_error += QString("    Residue file entry %1: %2\n").
                              arg(t+1).arg(molecules_residue_errors[idx][t]);
                        }
                     }
                     break;
                  }
                  error_string->append(this_error);
                  cout << QString("dbg 3: idx <%1> msg <%2>\n").arg(count_idx).arg(this_error);
                  error_shown[count_idx] = true;
               }
        

               // error_string->append(QString("").sprintf("unknown atom chain %s molecule %d atom %s residue %s %s\n",
               //            this_atom->chainID.toLatin1().data(),
               //            j + 1,
               //            this_atom->name.toLatin1().data(),
               //            this_atom->resSeq.toLatin1().data(),
               //            this_atom->resName.toLatin1().data()
               //            ));
            }
         }
      } // end for ( unsigned int k = 0; k < model->molecule[j].atom.size(); k++ )
      if (lastResPos != -1)
      {
         // check for false entries in last residue and warn about them
         // printf("check for false entries in last residue %d, residues_found %d, N1s_placed %d\n", lastResPos, residues_found, N1s_placed);
         for (unsigned int l = 0; l < residue_list[lastResPos].r_atom.size(); l++)
         {
            if ( !residue_list[lastResPos].r_atom[l].tmp_flag &&
                 !(misc.pb_rule_on && residues_found == 1 && N1s_placed == 1) )
            {
               errors_found++;
               QString count_idx =
                  QString("%1|%2|%3")
                  .arg(j)
                  .arg(residue_list[lastResPos].r_atom[l].name)
                  .arg(lastResSeq);
               if ( advanced_config.debug_1 )
               {
                  printf("dbg x pdb_parse.missing_atoms %d bead_exceptions[%s] %d residue_list[%d].r_atom[%d].tmp_flag %d\n"
                         , pdb_parse.missing_atoms
                         , count_idx.toLatin1().data()
                         , bead_exceptions[count_idx]
                         , lastResPos
                         , l
                         , residue_list[lastResPos].r_atom[l].tmp_flag
                         );
               }
                      
               if (pdb_parse.missing_atoms == 0)
               {
                  failure_errors++;
               } 
               else 
               {
                  if (pdb_parse.missing_atoms == 1)
                  {
                     bead_exceptions[count_idx] = 2;
                  }
                  if (pdb_parse.missing_atoms == 2)
                  {
                     if ( bead_exceptions[count_idx] == 1 ) 
                     {
                        bead_exceptions[count_idx] = 4;
                     }
                  }
               }
               if (!error_shown[count_idx]) 
               {
                  QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                     .arg(lastChainID == " " ? "" : ("Chain " + lastChainID + " "))
                     .arg(j + 1)
                     .arg(residue_list[lastResPos].name)
                     .arg(lastResSeq);
                  QString idx = QString("%1|%2").arg(j).arg(lastResSeq);
                  switch (molecules_residue_errors[idx].size())
                  {
                  case 0: 
                     this_error += QString("Missing atom %1.\n").arg(residue_list[lastResPos].r_atom[l].name);
                     break;
                  case 1:
                     this_error += molecules_residue_errors[idx][0] + "\n";
                     break;
                  default :
                     {
                        this_error += "\n";
                        for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                        {
                           if ( advanced_config.debug_1 )
                           {
                              this_error += " d4";
                           }
                           this_error += QString("    Residue file entry %1: %2\n").
                              arg(t+1).arg(molecules_residue_errors[idx][t]);
                        }
                     }
                     break;
                  }
                  error_string->append(this_error);
                  cout << QString("dbg 4: idx <%1> msg <%2>\n").arg(count_idx).arg(this_error);
                  error_shown[count_idx] = true;
               }
               // error_string->
               //  append(QString("").
               // sprintf("missing atom chain %s molecule %d atom %s residue %s %s\n",
               //    lastChainID.toLatin1().data(),
               //    j + 1,
               //    residue_list[lastResPos].r_atom[l].name.toLatin1().data(),
               //    lastResSeq.toLatin1().data(),
               //    residue_list[lastResPos].name.toLatin1().data()));
            }
         }
      }
   } // j

   if (error_string->length())
   {
      // to remove from unknown those that have 2 beads and a correct main chain
      vector < QString > unknown_aa_residues_to_check; 
      if (failure_errors > 0) 
      {
         return errors_found;
      }
      else 
      {
         editor->append("Encountered the following warnings with your PDB structure:\n" + *error_string);
         *error_string = "";
         // repair model...
         PDB_model org_model = *model;
         model->molecule.clear( );
         // we may need to redo the residues also
         model->residue.clear( );
         printf("vbar before: %g\n", model->vbar);
         QString new_residue_name = "";
         map < QString, bool > abb_msg_done;  // keep 1 message for the residue
         for (unsigned int j = 0; j < org_model.molecule.size(); j++)
         {
            PDB_chain tmp_chain;
            QString lastResSeq = "";
            QString lastResName = "";

            bool auto_bb_aa = false;             // are we doing special amino acid handling?
            map < QString, int > aa_main_chain;  // to make sure we have a good main chain
            int current_bead_assignment = 0;
            bool one_bead = false;               // fall back to one bead for missing atoms
            if (last_residue_type[j] == 0 &&      
                residue_types[j].size() == 1) 
            {
               // only amino acids, so we can create two beads
               if ( advanced_config.debug_2  )
               {
                  puts("auto_bb_amino_acids");
               }
               auto_bb_aa = true;
            }

            for (unsigned int k = 0; k < org_model.molecule[j].atom.size(); k++)
            {
               QString count_idx =
                  QString("%1|%2|%3")
                  .arg(j)
                  .arg(org_model.molecule[j].atom[k].resName)
                  .arg(org_model.molecule[j].atom[k].resSeq);
               switch (bead_exceptions[count_idx])
               {
               case 1:
                  tmp_chain.atom.push_back(org_model.molecule[j].atom[k]);
                  if (org_model.molecule[j].atom[k].resSeq != lastResSeq) 
                  {
                     lastResSeq = org_model.molecule[j].atom[k].resSeq;
                     lastResName = org_model.molecule[j].atom[k].resName;
                     model->residue.push_back(residue_list[multi_residue_map[lastResName][0]]);
                  }
                  break;
               case 2:
               case 5: // extra or non-coded atom
                  {
                     PDB_atom *this_atom = &org_model.molecule[j].atom[k];
                     if ( !abb_msg_done[count_idx] ) 
                     {
                        abb_msgs += QString("ABB: %1Molecule %2 Residue %3 %4 Skipped\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq);
                        abb_msg_done[count_idx] = true;
                     }
                     printf("removing molecule %u atom %u from model\n", 
                            j, k);
                  }
                  break;
               case 3:
                  {
                     // create the temporary residue
                     // is this a new residue

                     PDB_atom *this_atom = &org_model.molecule[j].atom[k];
                     QString new_residue_idx = this_atom->resName;  // we could add atom_count to the idx for counting by unique atom counts...
                     if ( advanced_config.debug_2  )
                     {
                        printf("1.0 <%s> residue types %u, last_type %u\n", 
                               new_residue_idx.toLatin1().data(),
                               (unsigned int)residue_types[j].size(),
                               last_residue_type[j]
                               );
                     }           
                     if (this_atom->resSeq != lastResSeq) 
                     {

                        current_bead_assignment = 0;
                        if (auto_bb_aa) 
                        {
                           // reset the main chain counts
                           aa_main_chain.clear( );
                           aa_main_chain["N"] = 0;
                           aa_main_chain["CA"] = 0;
                           aa_main_chain["C"] = 0;
                           aa_main_chain["O"] = 0;
                        }
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;
                        // this is a new unknown residue
                        // each instance of the residue gets a unique name, so we don't have
                        // to worry about duplicates and alternate duplicates
                        new_residues[new_residue_idx]++;
                        new_residue_name = QString("%1_NC%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
                        unknown_residues[new_residue_name] = true;
                        abb_msgs += QString("ABB: %1Molecule %2 Residue %3 %4 Non-coded residue %5 created.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(new_residue_name);
                        if ( advanced_config.debug_2  )
                        {
                           printf("1.1 <%s>\n", new_residue_name.toLatin1().data());
                        }
                        atom_counts[QString("%1|%2|%3")
                                    .arg(j)
                                    .arg(new_residue_name)
                                    .arg(this_atom->resSeq)] = atom_counts[count_idx];
                        // create new_residue
                        new_residue.comment = QString("Temporary residue %1").arg(new_residue_name);
                        new_residue.name = new_residue_name;
                        new_residue.unique_name = new_residue_name;
                        new_residue.type = 8;  // other
                        new_residue.molvol = misc.avg_volume * atom_counts[count_idx];
                        // new_residue.asa = misc.avg_asa * atom_counts[count_idx];
                        new_residue.asa = 0;
                        new_residue.vbar       = misc.avg_vbar;
                        new_residue.vbar_at_pH = misc.avg_vbar;
                        new_residue.r_atom.clear( );
                        new_residue.r_bead.clear( );

                        new_bead.hydration = (unsigned int)(misc.avg_hydration * atom_counts[count_idx] + .5);
                        new_bead.color = 10;         // light green
                        new_bead.placing_method = 0; // cog
                        new_bead.chain = 1;          // side chain
                        new_bead.volume = misc.avg_volume * atom_counts[count_idx];
                        new_bead.mw = misc.avg_mass * atom_counts[count_idx];
// #define DEBUG_ABB_VBAR
#if defined( DEBUG_ABB_VBAR )
                        QTextStream( stdout )
                           << "--------------------------------------------------------------------------------\n"
                           << "residue     : " << j << "\n"
                           << "avg_volume  : " << misc.avg_volume << "\n"
                           << "count_idx   : " << count_idx << "\n"
                           << "atom_counts : " << atom_counts[count_idx] << "\n"
                           << "molvol      : " << new_residue.molvol << "\n"
                           << "bead.vol    : " << new_bead.volume << "\n"
                           << "mw          : " << new_bead.mw << "\n"
                           ;
#endif

                        new_bead.ionized_mw_delta = 0;
                        new_residue.r_bead.push_back(new_bead);
                        multi_residue_map[new_residue.name].push_back(residue_list.size());
#if defined( DEBUG_TESTING_JML )
                        us_qdebug( QString( "residue list push back, residue_list.size() %1 residue_name %2" ).arg( residue_list.size() ).arg( new_residue.name ) );
#endif
                        residue_list.push_back(new_residue);
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;
                        if ( advanced_config.debug_2  )
                        {
                           printf("1.1b <%s>\n", new_residue.name.toLatin1().data());
                        }
                        model->residue.push_back(residue_list[multi_residue_map[new_residue.name][0]]);
                     }
                     else
                     {
                        new_residue_name = QString("%1_NC%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
                        unknown_residues[new_residue_name] = true;
                        if ( advanced_config.debug_2  )
                        {
                           printf("1.2 <%s>\n", new_residue_name.toLatin1().data());
                        }
                        if (new_residue_name != new_residue.name)
                        {
                           US_Static::us_message(us_tr("Internal error:"), 
                                                us_tr("Unexpected new residue name inconsistancy.\n") +
                                                QString("\"%1\" != \"%2\"").arg(new_residue_name).arg(new_residue.name));
                           exit(-1);
                        }
                     }
                     // the new residue should always be the last one, since we just added it
                     int respos = residue_list.size() - 1;
                     if (new_residue.name != residue_list[respos].name)
                     {
                        US_Static::us_message(us_tr("Internal error:"), 
                                             us_tr("Unexpected new residue name residue_list inconsistancy."));
                        exit(-1);
                     }
                     // ok, now we can push back the modified atom
                     if (auto_bb_aa)
                     {
                        if (residue_list[respos].r_atom.size() < 4) 
                        {
                           aa_main_chain[this_atom->name]++;
                           if (residue_list[respos].r_atom.size() == 3) 
                           {
                              // early handling in case of no side chain
                              if (aa_main_chain["N"] == 1 &&
                                  aa_main_chain["CA"] == 1 &&
                                  aa_main_chain["C"] == 1 &&
                                  aa_main_chain["O"] == 1)
                              {
                                 residue_list[respos].type = 0;
                                 residue_list[respos].r_bead[0].chain = 0;  // main chain
                              }
                           }
                        } 
                        else 
                        {
                           if (residue_list[respos].r_atom.size() == 4) 
                           {
                              if (aa_main_chain["N"] == 1 &&
                                  aa_main_chain["CA"] == 1 &&
                                  aa_main_chain["C"] == 1 &&
                                  aa_main_chain["O"] == 1)
                              {
                                 // ok, we have a proper backbone
                                 // so we have to redo the beads etc.
                                 abb_msgs += QString("ABB: %1Molecule %2 Residue %3 %4 Non-coded residue %5 PB found, 2 beads created.\n")
                                    .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                    .arg(j+1)
                                    .arg(this_atom->resName)
                                    .arg(this_atom->resSeq)
                                    .arg(new_residue_name);
                                 current_bead_assignment = 1;
                                 // redo 1st bead
                                 residue_list[respos].type = 0;

                                 residue_list[respos].r_bead[0].hydration = 
                                    (unsigned int)(misc.avg_hydration * 4 + .5);
                                 residue_list[respos].r_bead[0].chain = 0;  // main chain
                                 residue_list[respos].r_bead[0].volume = misc.avg_volume * 4;
                                 residue_list[respos].r_bead[0].mw = misc.avg_mass * 4;
                                 residue_list[respos].r_bead[0].ionized_mw_delta = 0;

                                 // create a 2nd bead
                                 residue_list[respos].r_bead.push_back(residue_list[respos].r_bead[0]);
                                 residue_list[respos].r_bead[1].hydration = 
                                    (unsigned int)(misc.avg_hydration * (atom_counts[count_idx] - 4) + .5);
                                 residue_list[respos].r_bead[1].chain = 1;  // side chain
                                 residue_list[respos].r_bead[1].volume =
                                    misc.avg_volume * (atom_counts[count_idx] - 4);
                                 residue_list[respos].r_bead[1].mw = 
                                    misc.avg_mass * (atom_counts[count_idx] - 4);
                                 residue_list[respos].r_bead[1].ionized_mw_delta = 0;
                              }
                           }
                        }
                     }
            
                     new_atom.name                    = (this_atom->name == "OXT" ? "OXT'" : this_atom->name);
                     new_atom.hybrid.name             = this_atom->name;
                     new_atom.hybrid.mw               = misc.avg_mass;
                     new_atom.hybrid.ionized_mw_delta = 0;
                     new_atom.hybrid.radius           = misc.avg_radius;
                     new_atom.hybrid.scat_len         = 0;
                     new_atom.hybrid.saxs_name        = "ABB";
                     new_atom.hybrid.num_elect        = misc.avg_num_elect;
                     new_atom.hybrid.protons          = misc.avg_protons;
                     
                     new_atom.bead_assignment = current_bead_assignment; 
                     new_atom.positioner = true;
                     new_atom.serial_number = residue_list[respos].r_atom.size();
                     new_atom.hydration = misc.avg_hydration;
                     residue_list[respos].r_atom.push_back(new_atom);
                     PDB_atom atom_to_add = org_model.molecule[j].atom[k];
                     atom_to_add.resName = new_residue.name;
                     if ( atom_to_add.name == "OXT" )
                     {
                        atom_to_add.name = "OXT'";
                     }
                     if ( advanced_config.debug_2  )
                     {
                        printf("1.3 <%s> adding <%s> respos %u\n", new_residue.name.toLatin1().data(), atom_to_add.name.toLatin1().data(), respos);
                     }
                     tmp_chain.atom.push_back(atom_to_add);
                     model->residue[model->residue.size() - 1] = residue_list[residue_list.size() - 1];
                     {
                        QString mapkey = QString("%1|%2").arg(atom_to_add.resName).arg(atom_to_add.name);
                        if ( atom_to_add.name == "OXT" )
                        {
                           mapkey = "OXT|OXT";
                        }
                        if ( !residue_atom_hybrid_map.count( mapkey ) &&
                             residue_atom_abb_hybrid_map.count( atom_to_add.name ) ) {
                           // should use ABB atoms hybridization
                           // residue_atom_hybrid_map[ mapkey ] = residue_atom_abb_hybrid_map[ atom_to_add.name ];
                           // us_qdebug( QString( "hybrid map created for %1 created as %2" ).arg( mapkey ).arg( residue_atom_abb_hybrid_map[atom_to_add.name ] ) );
                           residue_atom_hybrid_map[ mapkey ] = "ABB";
                           {
                              QString mapkey_no_nc = QString("%1|%2").arg(this_atom->resName).arg(atom_to_add.name);
                              residue_atom_hybrid_map[ mapkey_no_nc ] = "ABB";
                           }
                           residue_atom_hybrid_map[ mapkey ] = "ABB";
                           if ( !saxs_util->hybrid_map.count( "ABB" ) ) {
                              saxs_util->hybrid_map[ "ABB" ] = new_atom.hybrid;
                           }
                           {
                              QString atom_map_key = new_atom.name + "~ABB";
                              if ( !saxs_util->atom_map.count( atom_map_key ) ) {
                                 saxs_util->atom_map[ atom_map_key ] = new_atom;
                              }
                           }
                           us_qdebug( QString( "hybrid map created for %1 created as %2" ).arg( mapkey ).arg( "ABB" ) );
                        }
                     }
                  }
                  break;
               case 4: // missing atoms
                  {
                     PDB_atom *this_atom = &org_model.molecule[j].atom[k];
                     QString idx = QString("%1|%2").arg(j).arg(org_model.molecule[j].atom[k].resSeq);
                     QString new_residue_idx = this_atom->resName;  
                     int pos = molecules_residue_min_missing[idx];
                     if ( pos < 0 )
                     {
                        cout << QString("Internal error: idx %1 Missing atom error %2 ! < 0!\n")
                           .arg(idx)
                           .arg(pos);
                        exit(-1);
                     }
                     cout << QString("ta->resseq %1, pos %2\n").arg(this_atom->resName).arg(pos);
                     int orgrespos = multi_residue_map[this_atom->resName][pos];
                     if (this_atom->resSeq != lastResSeq) 
                     {
                        // new residue to create with existing atoms, but increased values...
                        // possibly for a multi-bead residue
                        one_bead = false;
                        current_bead_assignment = 0;
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;
                        // setup how many atoms are missing from each bead
                        vector < int > beads_missing_atom_count;
                        printf("orgrespos %d\n", orgrespos);
                        beads_missing_atom_count.resize(residue_list[orgrespos].r_bead.size());

                        for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                        {
                           beads_missing_atom_count[i] = 0;
                        }
                        // accumulate missing atom count for each bead
                        for ( unsigned int i = 0; i < molecules_residue_missing_atoms[idx][pos].size(); i++ )
                        {
                           beads_missing_atom_count[molecules_residue_missing_atoms_beads[idx][pos][i]]++;
                        }
                        // do any of these leave an empty bead?
                        // first, we need to know how many atoms are in each bead...
                        vector < int > atoms_per_bead;
                        atoms_per_bead.resize(residue_list[orgrespos].r_bead.size());
                        for ( unsigned int i = 0; i < atoms_per_bead.size(); i++ )
                        {
                           atoms_per_bead[i] = 0;
                        }
                        for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                        {
                           atoms_per_bead[residue_list[orgrespos].r_atom[i].bead_assignment]++;
                        }
                        
                        for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                        {
                           if ( atoms_per_bead[i] - beads_missing_atom_count[i] < 1 )
                           {
                              // ugh, fall back to one bead!
                              one_bead = true;
                              printf("notice: fallback to one bead for missing atom\n");
                           }
                        }

                        vector < double > atoms_scale_weight; // multiplier for atoms weight
                        vector < double > atoms_scale_radius; // multiplier for atoms radii
                        atoms_scale_weight.resize(residue_list[orgrespos].r_bead.size());
                        atoms_scale_radius.resize(residue_list[orgrespos].r_bead.size());
                        if ( one_bead )
                        {
                           // sum molecular weight & radii cubed for total bead & bead sans missing
                           double tot_mw = 0;
                           double tot_mw_missing = 0;
                           double tot_radii3 = 0;
                           double tot_radii3_missing = 0;
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              tot_mw += residue_list[orgrespos].r_atom[i].hybrid.mw + residue_list[orgrespos].r_atom[i].hybrid.ionized_mw_delta;
                              tot_radii3 += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3);
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 tot_mw_missing += residue_list[orgrespos].r_atom[i].hybrid.mw + residue_list[orgrespos].r_atom[i].hybrid.ionized_mw_delta;
                                 tot_radii3_missing += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3);
                              }
                           }
                           atoms_scale_weight[0] = 
                              tot_mw / tot_mw_missing;
                           atoms_scale_radius[0] = 
                              pow(tot_radii3 / tot_radii3_missing, 1.0/3.0);
                        }
                        else
                        {
                           // sum molecular weight & radii cubed for total bead & bead sans missing
                           vector < double > tot_mw;
                           vector < double > tot_mw_missing;
                           vector < double > tot_radii3;
                           vector < double > tot_radii3_missing;
                           for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                           {
                              tot_mw.push_back(0.0);
                              tot_mw_missing.push_back(0.0);
                              tot_radii3.push_back(0.0);
                              tot_radii3_missing.push_back(0.0);
                           }
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              tot_mw[residue_list[orgrespos].r_atom[i].bead_assignment]
                                 += residue_list[orgrespos].r_atom[i].hybrid.mw + residue_list[orgrespos].r_atom[i].hybrid.ionized_mw_delta;
                              tot_radii3[residue_list[orgrespos].r_atom[i].bead_assignment]
                                 += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3);
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 tot_mw_missing[residue_list[orgrespos].r_atom[i].bead_assignment]
                                    += residue_list[orgrespos].r_atom[i].hybrid.mw + residue_list[orgrespos].r_atom[i].hybrid.ionized_mw_delta;
                                 tot_radii3_missing[residue_list[orgrespos].r_atom[i].bead_assignment]
                                    += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3);
                              }
                           }
                           for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                           {
                              atoms_scale_weight[i] = 
                                 tot_mw[i] / tot_mw_missing[i];
                              atoms_scale_radius[i] = 
                                 pow(tot_radii3[i] / tot_radii3_missing[i], 1.0/3.0);
                           }
                        }

                        // create the new residue
                        new_residues[new_residue_idx]++;
                        new_residue_name = QString("%1_MA%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
                        unknown_residues[new_residue_name] = true;
                        if ( residue_list[orgrespos].type == 0 ) // aa
                        {
                           unknown_aa_residues_to_check.push_back(new_residue_name);
                        }
                        abb_msgs += QString("ABB: %1Molecule %2 Residue %3 %4 Missing atom residue copy %5 created%6.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(new_residue_name)
                           .arg(one_bead ? " as one bead" : "");

                        if ( advanced_config.debug_2  )
                        {
                           printf("a1.1 <%s>\n", new_residue_name.toLatin1().data());
                        }
                        atom_counts[QString("%1|%2|%3")
                                    .arg(j)
                                    .arg(new_residue_name)
                                    .arg(this_atom->resSeq)] = atom_counts[count_idx];
                        new_residue.comment = QString("Temporary residue %1").arg(new_residue_name);
                        new_residue.name = new_residue_name;
                        new_residue.unique_name = new_residue_name;
                        new_residue.type = residue_list[orgrespos].type;
                        new_residue.molvol = residue_list[orgrespos].molvol;
                        new_residue.asa = residue_list[orgrespos].asa;
                        new_residue.vbar = residue_list[orgrespos].vbar;
                        new_residue.r_atom.clear( );
                        new_residue.r_bead.clear( );

                        // create the beads
                        if ( one_bead )
                        {
                           new_bead.volume           = 0;
                           new_bead.mw               = 0;
                           new_bead.ionized_mw_delta = 0;
                           new_bead.hydration = 0;
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_bead.size(); i++ )
                           {
                              new_bead.volume           += residue_list[orgrespos].r_bead[i].volume;
                              new_bead.mw               += residue_list[orgrespos].r_bead[i].mw;
                              new_bead.ionized_mw_delta += residue_list[orgrespos].r_bead[i].ionized_mw_delta;
                              new_bead.hydration        += residue_list[orgrespos].r_bead[i].hydration;
                           }
                           // new_bead.hydration = (unsigned int)(misc.avg_hydration * atom_counts[count_idx] + .5);
                           new_bead.color = 10;         // light green
                           new_bead.placing_method = 0; // cog
                           new_bead.chain = 1;          // side chain
                           // do we have an AA with a complete PB?
                           if ( auto_bb_aa &&
                                !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                      .arg(idx)
                                                                      .arg(pos)
                                                                      .arg(0)] &&
                                !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                      .arg(idx)
                                                                      .arg(pos)
                                                                      .arg(1)] &&
                                !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                      .arg(idx)
                                                                      .arg(pos)
                                                                      .arg(2)] &&
                                !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                      .arg(idx)
                                                                      .arg(pos)
                                                                      .arg(3)] )
                           {
                              new_bead.chain = 0;      // main chain
                           }

                           new_residue.r_bead.push_back(new_bead);
                           // check for positioner
                           bool any_positioner = false;
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 if ( residue_list[orgrespos].r_atom[i].positioner )
                                 {
                                    any_positioner = true;
                                    break;
                                 }
                              }
                           }
                           // create the atoms
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 new_atom = residue_list[orgrespos].r_atom[i];
                                 if ( new_atom.name == "OXT" )
                                 {
                                    new_atom.name = "OXT'";
                                 }
                                 new_atom.hybrid.mw               *= atoms_scale_weight[0]; // misc.avg_mass;
                                 new_atom.hybrid.mw               = (int)(new_atom.hybrid.mw * 100 + .5) / 100.0;
                                 new_atom.hybrid.ionized_mw_delta *= atoms_scale_weight[0]; // misc.avg_mass;
                                 new_atom.hybrid.ionized_mw_delta = (int)(new_atom.hybrid.ionized_mw_delta * 100 + .5) / 100.0;
                                 new_atom.hybrid.radius           *= atoms_scale_radius[0]; // misc.avg_radius;
                                 new_atom.hybrid.radius           = (int)(new_atom.hybrid.radius * 100 + .5) / 100.0;
                                 new_atom.bead_assignment         = 0; 
                                 if ( !any_positioner )
                                 {
                                    new_atom.positioner = true;
                                 }
                                 new_residue.r_atom.push_back(new_atom);
                              }
                           }
                        } 
                        else
                        {
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_bead.size(); i++ )
                           {
                              new_bead = residue_list[orgrespos].r_bead[i];
                              if ( beads_missing_atom_count[i] )
                              {
                                 // only set the beads with missing atoms
                                 new_bead.color = 10;
                                 new_bead.placing_method = 0;
                              }
                              new_residue.r_bead.push_back(new_bead);
                           }
                           // check for positioners
                           vector < bool > any_positioner;
                           any_positioner.resize(residue_list[orgrespos].r_bead.size());
                           for ( unsigned int i = 0; i < any_positioner.size(); i++ )
                           {
                              any_positioner[i] = false;
                           }
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 if ( residue_list[orgrespos].r_atom[i].positioner )
                                 {
                                    any_positioner[residue_list[orgrespos].r_atom[i].bead_assignment] = true;
                                 }
                              }
                           }

                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 new_atom = residue_list[orgrespos].r_atom[i];
                                 if ( beads_missing_atom_count[new_atom.bead_assignment] )
                                 {
                                    new_atom.hybrid.mw               *= atoms_scale_weight[new_atom.bead_assignment]; // misc.avg_mass;
                                    new_atom.hybrid.mw               = (int)(new_atom.hybrid.mw * 100 + .5) / 100.0;
                                    new_atom.hybrid.ionized_mw_delta *= atoms_scale_weight[0]; // misc.avg_mass;
                                    new_atom.hybrid.ionized_mw_delta = (int)(new_atom.hybrid.ionized_mw_delta * 100 + .5) / 100.0;
                                    new_atom.hybrid.radius           *= atoms_scale_radius[new_atom.bead_assignment]; // misc.avg_radius;
                                    new_atom.hybrid.radius           = (int)(new_atom.hybrid.radius * 100 + .5) / 100.0;
                                    if ( !any_positioner[new_atom.bead_assignment] )
                                    {
                                       new_atom.positioner = true;
                                    }
                                 }
                                 new_residue.r_atom.push_back(new_atom);
                              }
                           }
                        }
                        multi_residue_map[new_residue.name].push_back(residue_list.size());
                        residue_list.push_back(new_residue);
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;
                        if ( advanced_config.debug_2  )
                        {
                           printf("a1.1b <%s>\n", new_residue.name.toLatin1().data());
                        }
                        model->residue.push_back(residue_list[multi_residue_map[new_residue.name][0]]);
                     }
                     else
                     {
                        new_residue_name = QString("%1_MA%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
                        unknown_residues[new_residue_name] = true;
                        if ( advanced_config.debug_2  )
                        {
                           printf("a1.2 <%s>\n", new_residue_name.toLatin1().data());
                        }
                        if (new_residue_name != new_residue.name)
                        {
                           US_Static::us_message(us_tr("Internal error:"), 
                                                us_tr("Unexpected new residue name inconsistancy (a).\n") +
                                                QString("\"%1\" != \"%2\"").arg(new_residue_name).arg(new_residue.name));
                           exit(-1);
                        }
                     }
                     // the new residue should always be the last one, since we just added it
                     int respos = residue_list.size() - 1;
                     if (new_residue.name != residue_list[respos].name)
                     {
                        US_Static::us_message(us_tr("Internal error:"), 
                                             us_tr("Unexpected new residue name residue_list inconsistancy (a)."));
                        exit(-1);
                     }
                     // ok, now we can push back the modified atom
                     PDB_atom atom_to_add = org_model.molecule[j].atom[k];
                     atom_to_add.resName = new_residue.name;
                     if ( atom_to_add.name == "OXT" )
                     {
                        atom_to_add.name = "OXT'";
                     }
                     if ( advanced_config.debug_2  )
                     {
                        printf("a1.3 <%s>\n", new_residue.name.toLatin1().data());
                     }
                     tmp_chain.atom.push_back(atom_to_add);
                     model->residue[model->residue.size() - 1] = residue_list[residue_list.size() - 1];
                     {
                        QString mapkey = QString("%1|%2").arg(atom_to_add.resName).arg(atom_to_add.name);
                        if ( atom_to_add.name == "OXT" )
                        {
                           mapkey = "OXT|OXT";
                        }
                        if ( !residue_atom_hybrid_map.count( mapkey ) &&
                             residue_atom_abb_hybrid_map.count( atom_to_add.name ) ) {
                           residue_atom_hybrid_map[ mapkey ] = residue_atom_abb_hybrid_map[ atom_to_add.name ];
                           // us_qdebug( QString( "hybrid map created for %1 created as %2" ).arg( mapkey ).arg( residue_atom_abb_hybrid_map[atom_to_add.name ] ) );
                        }
                     }
                  }
                  break;
                     
               default:
                  US_Static::us_message(us_tr("Internal error:"), 
                                       us_tr("Unhandled bead_exception code."));
                  exit(-1);
                  break;
               } // switch
            } // atoms
            if ( tmp_chain.atom.size() )
            {
               tmp_chain.chainID = tmp_chain.atom[0].chainID;
            }
            model->molecule.push_back(tmp_chain);
            abb_msgs += "\n";
         } // molecules

         {
            QColor save_color = editor->textColor();
            QFont save_font = editor->currentFont();
            QFont new_font = save_font;
            new_font.setStretch(70);
            editor->setCurrentFont(new_font);
            editor->setTextColor("blue");
            editor->append("\n" + abb_msgs);
            editor->setTextColor(save_color);
            editor->setCurrentFont(save_font);
            last_abb_msgs = "\n\nAutomatic Bead Builder messages:\n" + abb_msgs.replace("ABB: ","  ");
         }
         
         calc_vbar(model);
         update_vbar();
         printf("vbar after: %g\n", model->vbar);

         // remove from unknown any aa residues that have 2 beads and a side chain
         // with a N,C,CA,O MC
         for ( unsigned int i = 0; i < unknown_aa_residues_to_check.size(); i++ )
         {
            if ( advanced_config.debug_2  )
            {
               printf("unknown residue %s checking for removal from broken\n",
                      unknown_aa_residues_to_check[i].toLatin1().data());
            }
            unsigned int respos = multi_residue_map[unknown_aa_residues_to_check[i]][0];
            if ( residue_list[respos].r_bead.size() == 2 &&
                 residue_list[respos].r_atom.size() > 4 ) 
            {
               map < QString, int > aa_main_chain;  // to make sure we have a good main chain
               aa_main_chain.clear( );
               aa_main_chain["N"] = 0;
               aa_main_chain["CA"] = 0;
               aa_main_chain["C"] = 0;
               aa_main_chain["O"] = 0;
               for ( unsigned int j = 0; j < residue_list[respos].r_atom.size(); j++ )
               {
                  aa_main_chain[residue_list[respos].r_atom[j].name]++;
               }
               if (aa_main_chain["N"] == 1 &&
                   aa_main_chain["CA"] == 1 &&
                   aa_main_chain["C"] == 1 &&
                   aa_main_chain["O"] == 1)
               {
                  if ( advanced_config.debug_2  )
                  {
                     printf("unknown residue %s removed from broken override\n",
                            unknown_aa_residues_to_check[i].toLatin1().data());
                  }
                  unknown_residues.erase(unknown_aa_residues_to_check[i]);
               }
            }
         }
      }
   }
#if defined(AUTO_BB_DEBUG) || 1
   QString str1;
   QFile f(somo_tmp_dir + SLASH + "tmp.somo.residue");
   if (f.open(QIODevice::WriteOnly|QIODevice::Text))
   {
      QTextStream ts(&f);
      for (unsigned int i=0; i<residue_list.size(); i++)
      {
         ts << residue_list[i].comment << Qt::endl;
         ts << residue_list[i].name.toUpper()
            << "\t" << residue_list[i].type
            << "\t" << str1.sprintf("%7.2f", residue_list[i].molvol)
            << "\t" << residue_list[i].asa
            << "\t" << residue_list[i].r_atom.size()
            << "\t" << residue_list[i].r_bead.size()
            << "\t" << residue_list[i].vbar << Qt::endl;
         for (unsigned int j=0; j<residue_list[i].r_atom.size(); j++)
         {
            ts << residue_list[i].r_atom[j].name.toUpper()
               << "\t" << residue_list[i].r_atom[j].hybrid.name
               << "\t" << residue_list[i].r_atom[j].hybrid.mw
               << "\t" << residue_list[i].r_atom[j].hybrid.radius
               << "\t" << residue_list[i].r_atom[j].bead_assignment
               << "\t" << (unsigned int) residue_list[i].r_atom[j].positioner
               << "\t" << residue_list[i].r_atom[j].serial_number 
               << "\t" << residue_list[i].r_atom[j].hydration
               << Qt::endl;
         }
         for (unsigned int j=0; j<residue_list[i].r_bead.size(); j++)
         {
            ts << residue_list[i].r_bead[j].hydration
               << "\t" << residue_list[i].r_bead[j].color
               << "\t" << residue_list[i].r_bead[j].placing_method
               << "\t" << residue_list[i].r_bead[j].chain
               << "\t" << residue_list[i].r_bead[j].volume << Qt::endl;
         }
         str1.sprintf("%d: ", i+1);
         str1 += residue_list[i].name.toUpper();
      }
      f.close();
   }
#endif
   if ( !misc.compute_vbar || hydro.mass_correction )
   {
      QString str = us_tr("ATTENTION: User entered");
      if ( !misc.compute_vbar )
      {
         str += QString(" vbar = %1").arg(misc.vbar);
      }
      if ( hydro.mass_correction )
      {
         str += QString(" MW = %1").arg(hydro.mass);
      }
      QColor save_color = editor->textColor();
      editor->setTextColor("red");
      editor->append(str + "\n");
      editor->setTextColor(save_color);
   }
   return 0;
}

// #define DEBUG_OVERLAP
// #define DEBUG_OVERLAP_2
// #define DEBUG
// #define DEBUG2

// # define TOLERANCE 0.001       // this is used to place a limit on the allowed radial overlap
#define TOLERANCE overlap_tolerance

int US_Hydrodyn::overlap_check(bool sc, bool mc, bool buried, double tolerance, int limit, bool from_overlap_hydro )
{
   int retval = 0;
#if defined(DEBUG_OVERLAP)
   printf("overlap checking--overlap tolerance %f--%s-%s-%s--------------------\n",
          tolerance,
          sc ? "side chain" : "",
          mc ? "main & side chain" : "",
          buried ? "buried" : "");
#endif
   for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
      for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG_OVERLAP_2)
         if(i == 2 && (j == 34 || j == 60))
            printf("x1 overlap check  beads %d %d on chains %d %d exposed code %d %d active %s %s : radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] sep of %f - %s\n",
                   i, j,
                   bead_model[i].chain,
                   bead_model[j].chain,
                   bead_model[i].exposed_code,
                   bead_model[j].exposed_code,
                   bead_model[i].active ? "Y" : "N",
                   bead_model[j].active ? "Y" : "N",
                   bead_model[i].bead_computed_radius,
                   bead_model[j].bead_computed_radius,
                   bead_model[i].bead_coordinate.axis[0],
                   bead_model[i].bead_coordinate.axis[1],
                   bead_model[i].bead_coordinate.axis[2],
                   bead_model[j].bead_coordinate.axis[0],
                   bead_model[j].bead_coordinate.axis[1],
                   bead_model[j].bead_coordinate.axis[2],
                   bead_model[i].bead_computed_radius +
                   bead_model[j].bead_computed_radius -
                   sqrt(
                        pow(bead_model[i].bead_coordinate.axis[0] -
                            bead_model[j].bead_coordinate.axis[0], 2) +
                        pow(bead_model[i].bead_coordinate.axis[1] -
                            bead_model[j].bead_coordinate.axis[1], 2) +
                        pow(bead_model[i].bead_coordinate.axis[2] -
                            bead_model[j].bead_coordinate.axis[2], 2)),
                   sqrt(
                        pow(bead_model[i].bead_coordinate.axis[0] -
                            bead_model[j].bead_coordinate.axis[0], 2) +
                        pow(bead_model[i].bead_coordinate.axis[1] -
                            bead_model[j].bead_coordinate.axis[1], 2) +
                        pow(bead_model[i].bead_coordinate.axis[2] -
                            bead_model[j].bead_coordinate.axis[2], 2))
                   <= tolerance ? "ok" : "needs reduction"
                   );
#endif
         if (bead_model[i].active &&
             bead_model[j].active
             &&
             (mc ||
              (sc &&
               bead_model[i].chain == 1 &&
               bead_model[j].chain == 1))
             &&
             (buried ||
              (bead_model[i].exposed_code == 1 &&
               bead_model[j].exposed_code == 1)) &&
             bead_model[i].bead_computed_radius > tolerance &&
             bead_model[j].bead_computed_radius > tolerance
             ) {
            float separation =
               bead_model[i].bead_computed_radius +
               bead_model[j].bead_computed_radius -
               sqrt(
                    pow(bead_model[i].bead_coordinate.axis[0] -
                        bead_model[j].bead_coordinate.axis[0], 2) +
                    pow(bead_model[i].bead_coordinate.axis[1] -
                        bead_model[j].bead_coordinate.axis[1], 2) +
                    pow(bead_model[i].bead_coordinate.axis[2] -
                        bead_model[j].bead_coordinate.axis[2], 2));
#if defined(DEBUG_OVERLAP_2)
            if(i == 2 && (j == 34 || j == 60))
               printf("x2 overlap check  beads %d %d on chains %d %d exposed code %d %d active %s %s : radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] sep of %f - %s\n",
                      i, j,
                      bead_model[i].chain,
                      bead_model[j].chain,
                      bead_model[i].exposed_code,
                      bead_model[j].exposed_code,
                      bead_model[i].active ? "Y" : "N",
                      bead_model[j].active ? "Y" : "N",
                      bead_model[i].bead_computed_radius,
                      bead_model[j].bead_computed_radius,
                      bead_model[i].bead_coordinate.axis[0],
                      bead_model[i].bead_coordinate.axis[1],
                      bead_model[i].bead_coordinate.axis[2],
                      bead_model[j].bead_coordinate.axis[0],
                      bead_model[j].bead_coordinate.axis[1],
                      bead_model[j].bead_coordinate.axis[2],
                      separation,
                      separation <= tolerance ? "ok" : "needs reduction"
                      );
#endif
            if (separation <= tolerance) {
               continue;
            }
            if ( bead_model[i].bead_computed_radius > tolerance * 1.001 &&
                 bead_model[j].bead_computed_radius > tolerance * 1.001 ) {
               retval++;
               if ( limit && retval > limit ) {
                  if ( !from_overlap_hydro ) {
                     editor_msg( "red", us_tr( "There are more than %1 overlaps greater than tolerance of %2\nWe suggest you use ZENO or GRPY to calculate hydrodynamics for this model" ).arg( limit ).arg( tolerance ) );
                  }
                  return retval;
               }
               if ( !from_overlap_hydro ) {
                  editor_msg( "red", QString(us_tr("WARNING: Bead model has an overlap violation on beads %1 %2 overlap %3 A\n"))
                                 .arg(i + 1)
                                 .arg(j + 1)
                                 .arg(separation));
               }
#if defined(DEBUG_OVERLAP)
            printf("overlap check  beads %d %d on chains %d %d exposed code %d %d active %s %s : radii %f(%s) %f(%s) with coordinates [%f,%f,%f] [%f,%f,%f] sep of %f - needs reduction\n",
                   i + 1, j + 1,
                   bead_model[i].chain,
                   bead_model[j].chain,
                   bead_model[i].exposed_code,
                   bead_model[j].exposed_code,
                   bead_model[i].active ? "Y" : "N",
                   bead_model[j].active ? "Y" : "N",
                   bead_model[i].bead_computed_radius,
                   bead_model[i].bead_computed_radius > tolerance * 1.001 ? "gt tol" : "not gt tol",
                   bead_model[j].bead_computed_radius,
                   bead_model[j].bead_computed_radius > tolerance * 1.001 ? "gt tol" : "not gt tol",
                   bead_model[i].bead_coordinate.axis[0],
                   bead_model[i].bead_coordinate.axis[1],
                   bead_model[i].bead_coordinate.axis[2],
                   bead_model[j].bead_coordinate.axis[0],
                   bead_model[j].bead_coordinate.axis[1],
                   bead_model[j].bead_coordinate.axis[2],
                   separation
                   );
#endif
            }
         }
      }
   }
#if defined(DEBUG_OVERLAP)
   printf("end of overlap checking--overlap tolerance %f--%s-%s-%s--violations %d------------------\n",
          tolerance,
          sc ? "side chain" : "",
          mc ? "main & side chain" : "",
          buried ? "buried" : "",
          retval);
#endif
   return retval;
}

class sortable_PDB_atom {
public:
   PDB_atom pdb_atom;
   bool PRO_N_override;
   bool pb_rule_on;
   bool operator < (const sortable_PDB_atom& objIn) const
   {
      if (
          //   (PRO_N_override ? 0 : pdb_atom.bead_assignment)
          ( !pb_rule_on &&
            ( pdb_atom.bead_assignment < 
              objIn.pdb_atom.bead_assignment ) ) ? 1 : (
                                                pdb_atom.atom_assignment
                                                <
          //   (objIn.PRO_N_override ? 0 : objIn.pdb_atom.bead_assignment)
                                                objIn.pdb_atom.atom_assignment
                                                )
          )
      {
         return (true);
      }
      else
      {
         return (false);
      }
   }
};

int US_Hydrodyn::create_beads(QString *error_string, bool quiet)
{
   // us_qdebug( "create_beads"
   //         "\nresidue_atom_hybrid_map" );
   
   // for ( map<QString,QString>::iterator it = residue_atom_hybrid_map.begin();
   //       it != residue_atom_hybrid_map.end();
   //       ++it ) {
   //    us_qdebug( QString( "%1 -> %2" ).arg( it->first ).arg( it->second ) );
   // }

   // us_qdebug( "end of residue_atom_hybrid_map" );

   if ( !quiet ) 
   {
      editor->append("Creating beads from atomic model\n");
      qApp->processEvents();
   }
   active_atoms.clear( );

   // #define DEBUG_MM
#if defined(DEBUG_MM)
   {
      int no_of_atoms = 0;
      int no_of_molecules = model_vector[current_model].molecule.size();
      int i;
      for (i = 0; i < no_of_molecules; i++) {
         no_of_atoms +=  model_vector[current_model].molecule[i].atom.size();
      }

      editor->append(QString("There are %1 atoms in %2 molecule(s) in this model\n").arg(no_of_atoms).arg(no_of_molecules));
   }
#endif
   get_atom_map(&model_vector[current_model]);

   QRegExp count_hydrogens("H(\\d)");

   map < QString, QString > hybrid_name_to_N = { { "N3H0", "N1-" },
                                                 { "N3H1", "N1" } };

   for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
   {
      QString last_resSeq = "";
      QString last_resName = "";
      for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
      {
         PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

         QString count_idx =
            QString("%1|%2|%3")
            .arg(j)
            .arg(this_atom->resName)
            .arg(this_atom->resSeq);

         // initialize data
         this_atom->active = false;
         this_atom->asa = 0;
         this_atom->p_residue = 0;
         this_atom->p_atom = 0;
         this_atom->radius = 0;
         this_atom->bead_hydration = 0;
         this_atom->bead_color = 0;
         this_atom->bead_ref_volume = 0;
         this_atom->bead_ref_mw = 0;
         this_atom->bead_ref_ionized_mw_delta = 0;
         this_atom->bead_assignment = -1;
         this_atom->atom_assignment = -1;
         this_atom->chain = -1;
         this_atom->type = 99;

         // find residue in residues
         int respos = -1;
         if ( advanced_config.debug_1 ||
              advanced_config.debug_2 )
         {
            printf("residue search name %s resName %s\n",
                   this_atom->name.toLatin1().data(),
                   this_atom->resName.toLatin1().data());
         }

         int restype = 99;
         if ( multi_residue_map.count(this_atom->resName) &&
              multi_residue_map[this_atom->resName].size() )
         {
            restype = residue_list[multi_residue_map[this_atom->resName][0]].type;
         }

         QString res_idx =
            QString("%1|%2")
            .arg(j)
            .arg(this_atom->resSeq);

         for (unsigned int m = 0; m < residue_list.size(); m++)
         {
            if ( residue_list[m].name == this_atom->resName &&
                 skip_residue.count(QString("%1|%2").arg(res_idx).arg(m)) &&
                 skip_residue[QString("%1|%2").arg(res_idx).arg(m)] == true )
            {
               if ( advanced_config.debug_1 )
               {
                  cout << 
                     QString("(create beads) skipping %1 %2\n").arg(res_idx).arg(m) << endl;
               }
               continue;
            }

            if ((residue_list[m].name == this_atom->resName &&
                 (int)residue_list[m].r_atom.size() ==
                 atom_counts[count_idx] - has_OXT[count_idx] &&
                 this_atom->name != "OXT" &&
                 (k ||
                  this_atom->name != "N" ||
                  restype != 0 ||
                  broken_chain_head.count(QString("%1|%2")
                                          .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) ||
                  this_atom->resName == "PRO" ||
                  !misc.pb_rule_on)) ||

                // if pb_rule is off, final OXT before P needs to use OXT-P
                (residue_list[m].name == "OXT"
                 && this_atom->name == "OXT" && (misc.pb_rule_on || this_atom->resName != "PRO")) ||
                (residue_list[m].name == "OXT-P"
                 && (this_atom->name == "OXT" && !misc.pb_rule_on && this_atom->resName == "PRO")) ||

                (!k &&
                 this_atom->name == "N" &&
                 restype == 0 &&                  
                 misc.pb_rule_on &&
                 residue_list[m].name == "N1" &&
                 !broken_chain_head.count(QString("%1|%2")
                                          .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) &&
                 this_atom->resName != "PRO"))
            {
               respos = (int) m;
               this_atom->p_residue = &(residue_list[m]);
               if ( advanced_config.debug_1 ||
                    advanced_config.debug_2 )
               {
                  printf("residue match %d resName %s \n", m, residue_list[m].name.toLatin1().data());
               }
               break;
            }
         }
         if (respos == -1)
         {
            if ((this_atom->name != "H" && this_atom->name != "D"
                 && this_atom->resName != "DOD"
                 && this_atom->resName != "HOH" && (this_atom->altLoc == "A" || this_atom->altLoc == " ")))
            {
               error_string->append(QString("").sprintf("unknown residue molecule %d atom %d name %s resname %s coord [%f,%f,%f]\n",
                                                        j + 1, k, this_atom->name.toLatin1().data(),
                                                        this_atom->resName.toLatin1().data(),
                                                        this_atom->coordinate.axis[0], this_atom->coordinate.axis[1], this_atom->coordinate.axis[2]));
               return (US_SURFRACER_ERR_MISSING_RESIDUE);
            }
         } 
         else 
         {
               if ( advanced_config.debug_1 ||
                    advanced_config.debug_2 )
               {
#if defined(DEBUG)
            printf("found residue pos %d\n", respos);
#endif
               }
         }
         int atompos = -1;

         if (respos != -1)
         {
            // clear tmp_used if new resSeq
#if defined(DEBUG)
            printf("respos %d != -1 last used %s %s\n", respos, this_atom->resSeq.toLatin1().data(), last_resSeq.toLatin1().data());
#endif
            if (this_atom->resSeq != last_resSeq ||
                this_atom->resName != last_resName ||
                residue_list[respos].name == "OXT" ||
                residue_list[respos].name == "OXT-P" ||
                residue_list[respos].name == "N1")
            {
#if defined(DEBUG)
               printf("clear last used %s %s\n", this_atom->resSeq.toLatin1().data(), last_resSeq.toLatin1().data());
#endif
               for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
               {
                  residue_list[respos].r_atom[m].tmp_used = false;
               }
               last_resSeq = this_atom->resSeq;
               last_resName = this_atom->resName;
               if(residue_list[respos].name == "OXT" ||
                  residue_list[respos].name == "OXT-P" ||
                  residue_list[respos].name == "N1") {
                  last_resSeq = "";
               }
            }
#if defined(DEBUG)
            for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
            {
               if(residue_list[respos].r_atom[m].tmp_used) {
                  printf("used %u %u\n", respos, m);
               }
            }
#endif

            for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
            {
#if defined(DEBUG)
               if(this_atom->name == "N" && !k && misc.pb_rule_on) {
                  printf("this_atom->name == N/N1 this residue_list[%d].r_atom[%d].name == %s\n",
                         respos, m, residue_list[respos].r_atom[m].name.toLatin1().data());
               }
#endif
               if (!residue_list[respos].r_atom[m].tmp_used &&
                   (residue_list[respos].r_atom[m].name == this_atom->name ||
                    (
                     this_atom->name == "N" &&
                     misc.pb_rule_on &&
                     !k &&
                     residue_list[respos].r_atom[m].name == "N1" &&
                     !broken_chain_head.count(QString("%1|%2")
                                              .arg(this_atom->resSeq)
                                              .arg(this_atom->resName)) &&
                     this_atom->resName != "PRO"
                     )
                    )
                   )
               {
                  residue_list[respos].r_atom[m].tmp_used = true;
                  this_atom->p_atom = &(residue_list[respos].r_atom[m]);
                  atompos = (int) m;
                  break;
               }
            }

            if (atompos == -1)
            {
               error_string->append(QString("").sprintf("unknown atom molecule %d atom %d name %s resname %s coord [%f,%f,%f]\n",
                                                        j + 1, k, this_atom->name.toLatin1().data(),
                                                        this_atom->resName.toLatin1().data(),
                                                        this_atom->coordinate.axis[0], this_atom->coordinate.axis[1], this_atom->coordinate.axis[2]));
            } 
            else 
            {
               this_atom->active = true;
               this_atom->radius = residue_list[respos].r_atom[atompos].hybrid.radius;
               this_atom->mw               = residue_list[respos].r_atom[atompos].hybrid.mw;
               this_atom->ionized_mw_delta = residue_list[respos].r_atom[atompos].hybrid.ionized_mw_delta;

               // if ( this_atom->p_residue ) {
               //    // QTextStream( stdout ) << "**************************************** p_residue set " << Qt::endl;
               //    this_atom->ionized_mw_delta = this_atom->p_residue->r_atom[atompos].hybrid.ionized_mw_delta;
               // } else {
               //    // QTextStream( stdout ) << "**************************************** p_residue NOT set " << Qt::endl;
               // }
                  
               // if ( this_atom->ionized_mw_delta != 0 ) {
               //    QTextStream( stdout ) << "**************************************** ionized_mw_delta found "
               //                          << this_atom->ionized_mw_delta
               //                          << " respos " << respos
               //                          << " atompos " << atompos
               //                          << Qt::endl
               //       ;
               // }
                  
               this_atom->placing_method =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].placing_method;
               if ( this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].hydration_flag ) {
                  this_atom->bead_hydration = this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].hydration;
               } else {
                  this_atom->bead_hydration = this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].atom_hydration;
               }
               this_atom->bead_color =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].color;
               this_atom->bead_ref_volume =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].volume;
               this_atom->bead_ref_mw =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].mw;
               this_atom->bead_ref_ionized_mw_delta = this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].ionized_mw_delta;
               this_atom->ref_asa =  this_atom->p_residue->asa;
               this_atom->bead_computed_radius =  pow(3 * this_atom->bead_ref_volume / (4.0*M_PI), 1.0/3);
               this_atom->bead_assignment = this_atom->p_atom->bead_assignment;
               this_atom->atom_assignment = atompos;
               this_atom->chain = (int) this_atom->p_residue->r_bead[this_atom->bead_assignment].chain;
               this_atom->atom_hydration =  residue_list[respos].r_atom[atompos].hydration;
               this_atom->type = this_atom->p_residue->type;

               if ( saxs_options.compute_saxs_coeff_for_bead_models )
               {
                  QString mapkey = QString("%1|%2").arg(this_atom->resName).arg(this_atom->name);
                  if ( this_atom->name == "OXT" )
                  {
                     mapkey = "OXT|OXT";
                  }
                  
                  if ( !residue_atom_hybrid_map.count(mapkey) )
                  {
                     cout << "error: hybrid name missing for " << this_atom->resName << "|" << this_atom->name << endl; 
                     QColor save_color = editor->textColor();
                     editor->setTextColor("red");
                     editor->append(QString("%1Molecule %2 Residue %3|%4 %5 Hybrid name missing. Atom skipped for SAS.\n")
                                    .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                    .arg(j+1)
                                    .arg(this_atom->resName)
                                    .arg(this_atom->name)
                                    .arg(this_atom->resSeq));
                     editor->setTextColor(save_color);
                  } else {
                     QString hybrid_name = residue_atom_hybrid_map[mapkey];

                     if ( hybrid_name.isEmpty() || !hybrid_name.length() )
                     {
                        cout << "error: hybrid name empty for " << this_atom->resName << "|" << this_atom->name << endl; 
                        QColor save_color = editor->textColor();
                        editor->setTextColor("red");
                        editor->append(QString("%1Molecule %2 Residue %3|%4 %5 Hybrid name missing. Atom skipped for SAS.\n")
                                       .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                       .arg(j+1)
                                       .arg(this_atom->resName)
                                       .arg(this_atom->name)
                                       .arg(this_atom->resSeq));
                        editor->setTextColor(save_color);
                     } else {
                        if ( !saxs_util->hybrid_map.count(hybrid_name) )
                        {
                           cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
                           QColor save_color = editor->textColor();
                           editor->setTextColor("red");
                           editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped for SAS.\n")
                                          .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                          .arg(j+1)
                                          .arg(this_atom->resName)
                                          .arg(this_atom->resSeq)
                                          .arg(hybrid_name)
                                          );
                           editor->setTextColor(save_color);
                        } else {
                           if ( !saxs_util->atom_map.count(this_atom->name + "~" + hybrid_name) )
                           {
                              cout << "error: atom_map missing for hybrid_name "
                                   << hybrid_name 
                                   << " atom name "
                                   << this_atom->name
                                   << endl;
                              QColor save_color = editor->textColor();
                              editor->setTextColor("red");
                              editor->append(QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped for SAS.\n")
                                             .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                             .arg(j+1)
                                             .arg(this_atom->name)
                                             .arg(this_atom->resName)
                                             .arg(this_atom->resSeq)
                                             .arg(hybrid_name)
                                             );
                              editor->setTextColor(save_color);
                           } else {
                              this_atom->saxs_name = saxs_util->hybrid_map[hybrid_name].saxs_name; 
                              this_atom->hybrid_name = hybrid_name;
                              this_atom->hydrogens = 0;
                              if ( count_hydrogens.indexIn(hybrid_name) != -1 )
                              {
                                 this_atom->hydrogens = count_hydrogens.cap(1).toInt();
                              }
                              this_atom->saxs_excl_vol = saxs_util->atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;
                              if ( !saxs_util->saxs_map.count(saxs_util->hybrid_map[hybrid_name].saxs_name) )
                              {
                                 cout << "error: saxs_map missing for hybrid_name "
                                      << hybrid_name 
                                      << " saxs name "
                                      << saxs_util->hybrid_map[hybrid_name].saxs_name
                                      << endl;
                                 QColor save_color = editor->textColor();
                                 editor->setTextColor("red");
                                 editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                                                .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                                .arg(j+1)
                                                .arg(this_atom->resName)
                                                .arg(this_atom->resSeq)
                                                .arg(hybrid_name)
                                                .arg(saxs_util->hybrid_map[hybrid_name].saxs_name)
                                                );
                                 editor->setTextColor(save_color);
                              } else {
                                 this_atom->saxs_data = saxs_util->saxs_map[saxs_util->hybrid_map[hybrid_name].saxs_name];
                              }
                           }
                        }
                     }
                  }
               } // saxs setup

               // This bit is going, we need to instead lookup ionized values

               // if ( misc.pb_rule_on &&
               //      this_atom->resName == "PRO" &&
               //      this_atom->name == "N" &&
               //      !k )
               // {
               //    if ( advanced_config.debug_1 )
               //    {
               //       puts("handled +1 mw for first N on PRO");
               //    }
               //    this_atom->mw += 1.01f;
               //    // bead ref mw handled in pass 2b
               // }

               if ( !k &&
                    this_atom->name == "N" &&
                    ( !misc.pb_rule_on ||
                      this_atom->resName == "PRO" ) )
               {
                  double delta_mw = 1.01f;
                  if ( hybrid_name_to_N.count( this_atom->p_atom->hybrid.name ) &&
                       multi_residue_map.count( hybrid_name_to_N[this_atom->p_atom->hybrid.name ] ) &&
                       multi_residue_map[ hybrid_name_to_N[this_atom->p_atom->hybrid.name ] ].size()
                       ) {
                     int index = multi_residue_map[ hybrid_name_to_N[this_atom->p_atom->hybrid.name ] ][0];
                     delta_mw =
                        residue_list[ index ].r_atom[ 0 ].hybrid.mw
                        + residue_list[ index ].r_atom[ 0 ].hybrid.ionized_mw_delta
                        - this_atom->mw
                        ;
                  }
                  qDebug() << "handled first N mw delta " << delta_mw;
                  this_atom->mw          += delta_mw;
                  this_atom->bead_ref_mw += delta_mw;
               }
               
               if ( advanced_config.debug_1 ||
                    advanced_config.debug_2 )
               {
                  printf("found atom %s %s in residue %d pos %d bead asgn %d %d placing info %d mw %f bead_ref_mw %f hybrid name %s %s ba %d chain %d\n",
                         this_atom->name.toLatin1().data(), this_atom->resName.toLatin1().data(), respos, atompos,
                         this_atom->p_atom->bead_assignment,
                         this_atom->bead_assignment,
                         this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].placing_method,
                         this_atom->mw,
                         this_atom->bead_ref_mw,
                         residue_list[respos].r_atom[atompos].hybrid.name.toLatin1().data(),
                         this_atom->p_atom->hybrid.name.toLatin1().data(),
                         this_atom->p_atom->bead_assignment,
                         this_atom->chain
                         );
               }
            }
         }

         if (this_atom->active)
         {
            if ( advanced_config.debug_1 ||
                 advanced_config.debug_2 )
            {
               puts("active atom"); fflush(stdout);
            }
            this_atom->active = false;
            if (this_atom->name != "H" &&
                this_atom->name != "D" &&
                this_atom->p_residue->name != "DOD" &&
                this_atom->p_residue->name != "HOH" && (this_atom->altLoc == "A" || this_atom->altLoc == " "))
            {
               this_atom->active = true;
               active_atoms.push_back(this_atom);
            }
            else 
            {
#if defined(DEBUG)
               printf
                  ("skipped bound waters & H %s %s rad %f resseq %s\n",
                   this_atom->name.toLatin1().data(), this_atom->resName.toLatin1().data(), this_atom->radius, this_atom->resSeq.toLatin1().data());
               fflush(stdout);
#endif
            }
         }
      }
   }
   {
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];
            
            if ( advanced_config.debug_1 ||
                 advanced_config.debug_2 )
            {
               printf("p1 %s j k %d %d %p %s %d chain %d\n", 
                      this_atom->name.toLatin1().data(),
                      j, k,
                      this_atom->p_atom,
                      this_atom->active ? "active" : "not active",
                      this_atom->bead_assignment,
                      this_atom->chain
                      ); fflush(stdout);
            }
            
            // this_atom->bead_assignment =
            // (this_atom->p_atom ? (int) this_atom->p_atom->bead_assignment : -1);
            // this_atom->chain =
            // ((this_atom->p_residue && this_atom->p_atom) ?
            //  (int) this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].chain : -1);
            this_atom->org_chain = this_atom->chain;
            this_atom->bead_positioner = this_atom->p_atom ? this_atom->p_atom->positioner : false;
         }
      }
   }
#define DO_SORT
#if defined(DO_SORT)
   // reorder residue
   {
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
         QString last_resSeq = "";
         list <sortable_PDB_atom> last_residue_atoms;
         unsigned int k;
         for (k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];

            if (this_atom->resSeq != last_resSeq) {
               if (last_resSeq != "") {
                  // reorder previous residue
                  last_residue_atoms.sort();
                  int base_ofs = (int) last_residue_atoms.size();
                  // printf("resort last residue... size/base %d k %u\n", base_ofs, k);
                  for (unsigned int m = k - base_ofs; m < k; m++) {
                     // printf("resort m = %u size lra %u\n", m, last_residue_atoms.size());
                     model_vector[current_model].molecule[j].atom[m] = (last_residue_atoms.front()).pdb_atom;
                     last_residue_atoms.pop_front();
                  }
               }
               last_resSeq = this_atom->resSeq;
            }
            sortable_PDB_atom tmp_sortable_pdb_atom;
            tmp_sortable_pdb_atom.pdb_atom = *this_atom;
            tmp_sortable_pdb_atom.PRO_N_override = false;
            tmp_sortable_pdb_atom.pb_rule_on = misc.pb_rule_on;
            if(misc.pb_rule_on &&
               this_atom->resName == "PRO" &&
               this_atom->name == "N") {
               tmp_sortable_pdb_atom.PRO_N_override = true;
            }
            last_residue_atoms.push_back(tmp_sortable_pdb_atom);
         }
         if (last_resSeq != "") {
            // reorder 'last' residue
            last_residue_atoms.sort();
            int base_ofs = (int) last_residue_atoms.size();
            // printf("final resort last residue... size/base %d k %u\n", base_ofs, k);
            for (unsigned int m = k - base_ofs; m < k; m++) {
               //       printf("finalresort m = %u size lra %u\n", m, last_residue_atoms.size());
               model_vector[current_model].molecule[j].atom[m] = (last_residue_atoms.front()).pdb_atom;
               last_residue_atoms.pop_front();
            }
         }
      }
   }
#endif
#if !defined(DO_SORT)
   puts("sorting disabled");
#endif

   if ( advanced_config.debug_1 )
   {
      puts("after sort::");
      // list them again...
      {
         for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
            for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
               PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];
               
               printf("p1 %s j k %d %d %p %s %d %d\n", 
                      this_atom->name.toLatin1().data(),
                      j, k,
                      this_atom->p_atom,
                      this_atom->active ? "active" : "not active",
                      this_atom->bead_assignment,
                      this_atom->atom_assignment
                   ); fflush(stdout);
            }
         }
      }
   }

   if ( advanced_config.debug_1 )
   {
      puts("mw totals:::");
      double tot_atom_mw = 0e0;
      double tot_bead_mw = 0e0;
      double prev_bead_mw = 0e0;
      double prev_atom_mw = 0e0;
      int bead_count = 0;
      int atom_count = 0;
      int last_asgn = -1;
      QString last_res = "";
      QString last_resName = "";
      {
         for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
            for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
               PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];
               if(this_atom->active) {
                  if(last_asgn != (int)this_atom->bead_assignment ||
                     last_res != this_atom->resSeq) {
                     if(bead_count) {
                        printf("res %s %s bead %d bead_mw %f sum atom mw %f diff %f\n",
                               last_res.toLatin1().data(),
                               last_resName.toLatin1().data(),
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
                  }
                  atom_count++;
                  tot_atom_mw += this_atom->mw;
                  prev_atom_mw += this_atom->mw;
               }
            }
         }
         printf("res %s %s bead %d bead_mw %f sum atom mw %f diff %f\n",
                last_res.toLatin1().data(),
                last_resName.toLatin1().data(),
                last_asgn,
                prev_bead_mw,
                prev_atom_mw,
                prev_bead_mw -
                prev_atom_mw
                );
      }
      printf("~~tot atom %d mw %f tot bead %d mw %f\n",
             atom_count,
             tot_atom_mw,
             bead_count,
             tot_bead_mw);
   }

   // fix N1
   fix_N1_non_pbr( model_vector[ current_model ] );

   return 0;
}

void US_Hydrodyn::fix_N1_non_pbr( struct PDB_model & model ) {
   int chains   = (int) model.molecule.size();
   map < QString, QString > hybrid_name_to_N = { { "N3H0", "N1-" },
                                                 { "N3H1", "N1" } };
   for ( int j = 0; j < chains; ++j ) {
      int atoms = (int) model.molecule[ j ].atom.size();
      if ( atoms ) {
         map < QString, struct atom * > first_atom_map = first_residue_atom_map( model.molecule[ j ] );
         if ( first_atom_map.count( "N" )
              && hybrid_name_to_N.count( first_atom_map[ "N" ]->hybrid.name )
              && multi_residue_map.count( hybrid_name_to_N[ first_atom_map[ "N" ]->hybrid.name ] )
              && multi_residue_map[ hybrid_name_to_N[ first_atom_map[ "N" ]->hybrid.name ] ].size()
              ) {
            map < QString, int > atompos_map = first_residue_PDB_atom_map( model.molecule[ j ] );
            if ( atompos_map.count( "N" ) ) {
               int atompos = atompos_map[ "N" ];
               int respos = multi_residue_map[ hybrid_name_to_N[ first_atom_map[ "N" ]->hybrid.name ] ][0];
               model.molecule[ j ].atom[ atompos ].p_residue        = & ( residue_list[ respos ] );
               model.molecule[ j ].atom[ atompos ].p_atom           = & ( residue_list[ respos ].r_atom[0] );
               model.molecule[ j ].atom[ atompos ].mw               = model.molecule[ j ].atom[ atompos ].p_atom->hybrid.mw;
               model.molecule[ j ].atom[ atompos ].ionized_mw_delta = model.molecule[ j ].atom[ atompos ].p_atom->hybrid.ionized_mw_delta;
            } else {
               QTextStream( stderr ) << "US_Hydrodyn::fix_N1_non_pbr no 'N' atom found in PDB_chain";
            }
         }
      }
   }
}        

# define POP_MC              (1 << 0)
# define POP_SC              (1 << 1)
# define POP_MCSC            (1 << 2)
# define POP_EXPOSED         (1 << 3)
# define POP_BURIED          (1 << 4)
# define POP_ALL             (1 << 5)
# define RADIAL_REDUCTION    (1 << 6)
# define RR_MC               (1 << 7)
# define RR_SC               (1 << 8)
# define RR_MCSC             (1 << 9)
# define RR_EXPOSED          (1 << 10)
# define RR_BURIED           (1 << 11)
# define RR_ALL              (1 << 12)
# define OUTWARD_TRANSLATION (1 << 13)
# define RR_HIERC            (1 << 14)
# define MIN_OVERLAP 0.0

//--------- thread for radial reduction ------------

// #define DEBUG_THREAD

radial_reduction_thr_t::radial_reduction_thr_t(int a_thread) : QThread()
{
   thread = a_thread;
   work_to_do = 0;
   work_done = 1;
   work_to_do_waiters = 0;
   work_done_waiters = 0;
}

void radial_reduction_thr_t::radial_reduction_thr_setup(
                                                        unsigned int methodk,
                                                        vector <PDB_atom> *p_bead_model,
                                                        vector <bool> *p_last_reduced,
                                                        vector <bool> *p_reduced,
                                                        vector <BPair> *p_my_pairs,
                                                        unsigned int threads,
                                                        double overlap_tolerance
                                                        )
{
   /* this starts up a new work load for the thread */
   this->methodk = methodk;
   this->p_bead_model = p_bead_model;
   this->p_my_pairs = p_my_pairs;
   this->p_last_reduced = p_last_reduced;
   this->p_reduced = p_reduced;
   this->overlap_tolerance = overlap_tolerance;
   
   this->threads = threads;

   work_mutex.lock();
   work_to_do = 1;
   work_done = 0;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();
#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " has new work to do\n";
#endif
}

void radial_reduction_thr_t::radial_reduction_thr_shutdown()
{
   /* this signals the thread to exit the run method */
   work_mutex.lock();
   work_to_do = -1;
   work_mutex.unlock();
   cond_work_to_do.wakeOne();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " shutdown requested\n";
#endif
}

void radial_reduction_thr_t::radial_reduction_thr_wait()
{
   /* this is for the master thread to wait until the work is done */
   work_mutex.lock();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " has a waiter\n";
#endif

   while(!work_done) {
      cond_work_done.wait(&work_mutex);
   }
   work_done = 0;
   work_mutex.unlock();

#if defined(DEBUG_THREAD)
   cerr << "thread " << thread << " waiter released\n";
#endif
}

void radial_reduction_thr_t::run()
{
   while(1)
   {
      work_mutex.lock();
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " waiting for work\n";
#endif
      work_to_do_waiters++;
      while(!work_to_do)
      {
         cond_work_to_do.wait(&work_mutex);
      }
      if(work_to_do == -1)
      {
#if defined(DEBUG_THREAD)
         cerr << "thread " << thread << " shutting down\n";
#endif
         work_mutex.unlock();
         return;
      }

      work_to_do_waiters = 0;
      work_mutex.unlock();
#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " starting work\n";
#endif
      BPair pair;

      for ( unsigned int i = thread; i < (*p_bead_model).size() - 1; i += threads )
      {
         (*p_reduced)[i] = false;
         if (1 || (*p_last_reduced)[i]) {
            for (unsigned int j = i + 1; j < (*p_bead_model).size(); j++) {
               if ((1 || (*p_last_reduced)[j]) &&
                   (*p_bead_model)[i].active &&
                   (*p_bead_model)[j].active &&
                   (methodk & RR_MCSC ||
                    ((methodk & RR_SC) &&
                     (*p_bead_model)[i].chain == 1 &&
                     (*p_bead_model)[j].chain == 1)) &&
                   ((methodk & RR_BURIED) ||
                    ((*p_bead_model)[i].exposed_code == 1 ||
                     (*p_bead_model)[j].exposed_code == 1)) &&
                   (*p_bead_model)[i].bead_computed_radius > TOLERANCE &&
                   (*p_bead_model)[j].bead_computed_radius > TOLERANCE
                   ) {
                  float separation =
                     (*p_bead_model)[i].bead_computed_radius +
                     (*p_bead_model)[j].bead_computed_radius -
                     sqrt(
                          pow((*p_bead_model)[i].bead_coordinate.axis[0] -
                              (*p_bead_model)[j].bead_coordinate.axis[0], 2) +
                          pow((*p_bead_model)[i].bead_coordinate.axis[1] -
                              (*p_bead_model)[j].bead_coordinate.axis[1], 2) +
                          pow((*p_bead_model)[i].bead_coordinate.axis[2] -
                              (*p_bead_model)[j].bead_coordinate.axis[2], 2));
                  
                  if (separation <= TOLERANCE) {
                     continue;
                  }
                  
                  pair.i = i;
                  pair.j = j;
                  pair.separation = separation;
                  (*p_my_pairs).push_back(pair);
               }
            }
         } // if last_reduced[i]
      }

#if defined(DEBUG_THREAD)
      cerr << "thread " << thread << " finished work\n";
#endif
      work_mutex.lock();
      work_done = 1;
      work_to_do = 0;
      work_mutex.unlock();
      cond_work_done.wakeOne();
   }
}

//--------- end thread for radial reduction ------------

//--------- radial reduction for beads ---------------------------------------

void US_Hydrodyn::radial_reduction( bool from_grid )
{
   // popping radial reduction

   // or sc fb Y rh Y rs Y to Y st Y ro Y 70.000000 1.000000 0.000000
   // or scmc fb Y rh N rs N to Y st N ro Y 1.000000 0.000000 70.000000
   // or bb fb Y rh Y rs N to N st Y ro N 0.000000 0.000000 0.000000
   float molecular_cog[3] = { 0, 0, 0 };
   float molecular_mw = 0;
   int end_progress = progress->value() + 10;
   cout << QString("radial reduction, tolerance is %1\n").arg(TOLERANCE);

   for (unsigned int i = 0; i < bead_model.size(); i++) {
      PDB_atom *this_atom = &bead_model[i];
      if (this_atom->active) {
         molecular_mw += this_atom->bead_mw + this_atom->bead_ionized_mw_delta;
         for (unsigned int m = 0; m < 3; m++) {
            molecular_cog[m] += this_atom->bead_coordinate.axis[m] * ( this_atom->bead_mw + this_atom->bead_ionized_mw_delta );
         }
      }
   }
   if (molecular_mw) {
      for (unsigned int m = 0; m < 3; m++) {
         molecular_cog[m] /= molecular_mw;
      }
   } 
   else 
   {
      printf("ERROR: this molecule has zero mw!\n");
   }

#if defined(DEBUG)
   printf("or sc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or scmc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or bb fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or grid fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n",
          sidechain_overlap.fuse_beads ? "Y" : "N",
          sidechain_overlap.remove_hierarch ? "Y" : "N",
          sidechain_overlap.remove_sync ? "Y" : "N",
          sidechain_overlap.translate_out ? "Y" : "N",
          sidechain_overlap.show_translate ? "Y" : "N",
          sidechain_overlap.remove_overlap ? "Y" : "N",
          sidechain_overlap.fuse_beads_percent,
          sidechain_overlap.remove_sync_percent,
          sidechain_overlap.remove_hierarch_percent,

          mainchain_overlap.fuse_beads ? "Y" : "N",
          mainchain_overlap.remove_hierarch ? "Y" : "N",
          mainchain_overlap.remove_sync ? "Y" : "N",
          mainchain_overlap.translate_out ? "Y" : "N",
          mainchain_overlap.show_translate ? "Y" : "N",
          mainchain_overlap.remove_overlap ? "Y" : "N",
          mainchain_overlap.fuse_beads_percent,
          mainchain_overlap.remove_sync_percent,
          mainchain_overlap.remove_hierarch_percent,

          buried_overlap.fuse_beads ? "Y" : "N",
          buried_overlap.remove_hierarch ? "Y" : "N",
          buried_overlap.remove_sync ? "Y" : "N",
          buried_overlap.translate_out ? "Y" : "N",
          buried_overlap.show_translate ? "Y" : "N",
          buried_overlap.remove_overlap ? "Y" : "N",
          buried_overlap.fuse_beads_percent,
          buried_overlap.remove_sync_percent,
          buried_overlap.remove_hierarch_percent,

          grid_overlap.fuse_beads ? "Y" : "N",
          grid_overlap.remove_hierarch ? "Y" : "N",
          grid_overlap.remove_sync ? "Y" : "N",
          grid_overlap.translate_out ? "Y" : "N",
          grid_overlap.show_translate ? "Y" : "N",
          grid_overlap.remove_overlap ? "Y" : "N",
          grid_overlap.fuse_beads_percent,
          grid_overlap.remove_sync_percent,
          grid_overlap.remove_hierarch_percent);
#endif

   int methods[] =
      {
         RADIAL_REDUCTION | RR_SC | RR_EXPOSED,
         RADIAL_REDUCTION | RR_MCSC | RR_EXPOSED,
         RADIAL_REDUCTION | RR_MCSC | RR_BURIED,
         RADIAL_REDUCTION | RR_MCSC | RR_BURIED,
      };

   //   if (no_rr) {
   //      methods[0] = 0;
   //      methods[1] = 0;
   //      methods[2] = 0;
   //      methods[3] = 0;
   //   }

   if (sidechain_overlap.fuse_beads) {
      methods[0] |= POP_SC | POP_EXPOSED;
   }

   if (mainchain_overlap.fuse_beads) {
      methods[1] |= POP_MCSC | POP_EXPOSED;
   }

   if (buried_overlap.fuse_beads) {
      methods[2] |= POP_ALL | POP_BURIED;
   }

   if (grid_overlap.fuse_beads) {
      methods[3] |= POP_ALL | POP_BURIED;
   }

   if (sidechain_overlap.remove_hierarch) {
      methods[0] |= RR_HIERC;
   }

   if (mainchain_overlap.remove_hierarch) {
      methods[1] |= RR_HIERC;
   }

   if (buried_overlap.remove_hierarch) {
      methods[2] |= RR_HIERC;
   }

   if (grid_overlap.remove_hierarch) {
      methods[3] |= RR_HIERC;
   }

   if (sidechain_overlap.translate_out) {
      methods[0] |= OUTWARD_TRANSLATION;
   }

   if (mainchain_overlap.translate_out) {
      methods[1] |= OUTWARD_TRANSLATION;
   }

   if (buried_overlap.translate_out) {
      methods[2] |= OUTWARD_TRANSLATION;
   }

   if (grid_overlap.translate_out) {
      methods[3] |= OUTWARD_TRANSLATION;
   }

   if (!sidechain_overlap.remove_overlap) {
      methods[0] = 0;
   }

   if (!mainchain_overlap.remove_overlap) {
      methods[1] = 0;
   }

   if (!buried_overlap.remove_overlap) {
      methods[2] = 0;
   }

   if (!grid_overlap.remove_overlap) {
      methods[3] = 0;
   }

   float overlap[] =
      {
         (float) ( sidechain_overlap.fuse_beads_percent / 100.0 ),
         (float) ( mainchain_overlap.fuse_beads_percent / 100.0 ),
         (float) ( buried_overlap.fuse_beads_percent / 100.0 ),
         (float) ( grid_overlap.fuse_beads_percent / 100.0 )
      };

   float rr_overlap[] =
      {
         (float)( ( sidechain_overlap.remove_hierarch ?
                    sidechain_overlap.remove_hierarch_percent : sidechain_overlap.remove_sync_percent) / 100.0 ),
         (float)( ( mainchain_overlap.remove_hierarch ?
                    mainchain_overlap.remove_hierarch_percent : mainchain_overlap.remove_sync_percent) / 100.0 ),
         (float)( ( buried_overlap.remove_hierarch ?
                    buried_overlap.remove_hierarch_percent : buried_overlap.remove_sync_percent) / 100.0 ),
         (float)( ( grid_overlap.remove_hierarch ?
                    grid_overlap.remove_hierarch_percent : grid_overlap.remove_sync_percent) / 100.0 )
      };


#if defined(WRITE_EXTRA_FILES)
   write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO + ".tsv", &bead_model);
   write_bead_spt(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO, &bead_model);
#endif
   for ( unsigned int k = ( grid.enable_asa ? 0 : 3 ); 
         k < (unsigned int)( grid.enable_asa ? 3 : 4 );
         k++ )
   {
      // only grid method
      if ( !methods[k] ||
           ( ( from_grid || grid.enable_asa ) && k == 1 ) )
      {
         printf("skipping stage %d\n", k);
         continue;
      }

   stage_loop:

      qApp->processEvents();

      if (stopFlag)
      {
         return;
      }
      int beads_popped = 0;

#if defined(DEBUG1) || defined(DEBUG)
      printf("popping stage %d %s%s%s%s%s%s%s%s%s%s%s%s%s%s%soverlap_reduction %f rroverlap %f\n",
             k,
             (methods[k] & POP_MC) ? "main chain " : "",
             (methods[k] & POP_SC) ? "side chain " : "",
             (methods[k] & POP_MCSC) ? "main & side chain " : "",
             (methods[k] & POP_EXPOSED) ? "exposed " : "",
             (methods[k] & POP_BURIED) ? "buried " : "",
             (methods[k] & POP_ALL) ? "all " : "",
             (methods[k] & RADIAL_REDUCTION) ? "radial reduction " : "",
             (methods[k] & RR_HIERC) ? "hierarchically " : "synchronously ",
             (methods[k] & RR_MC) ? "main chain " : "",
             (methods[k] & RR_SC) ? "side chain " : "",
             (methods[k] & RR_MCSC) ? "main & side chain " : "",
             (methods[k] & RR_EXPOSED) ? "exposed " : "",
             (methods[k] & RR_BURIED) ? "buried " : "",
             (methods[k] & RR_ALL) ? "all " : "",
             (methods[k] & OUTWARD_TRANSLATION) ? "outward translation " : "",
             overlap[k],
             rr_overlap[k]);
#endif

      if (overlap[k] < MIN_OVERLAP) {
         printf("using %f as minimum overlap\n", MIN_OVERLAP);
         overlap[k] = MIN_OVERLAP;
      }

      float max_intersection_volume;
      float intersection_volume = 0;
      int max_bead1 = 0;
      int max_bead2 = 0;
      unsigned iter = 0;
      bool overlaps_exist;
#if defined(TIMING)
      gettimeofday(&start_tv, NULL);
#endif
      editor->append(QString("Begin popping stage %1\n").arg(k + 1));
      progress->setValue(progress->value() + 1); 
      qApp->processEvents();

      do {
         qApp->processEvents();
         if (stopFlag)
         {
            return;
         }
         // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_bp-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
         // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-bp-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
#if defined(DEBUG1) || defined(DEBUG)
         printf("popping iteration %d\n", iter++);
#endif
         lbl_core_progress->setText(QString("Stage %1 popping iteration %2").arg(k+1).arg(iter));
         qApp->processEvents();

         max_intersection_volume = -1;
         overlaps_exist = false;
         if (methods[k] & (POP_MC | POP_SC | POP_MCSC | POP_EXPOSED | POP_BURIED | POP_ALL)) {
            for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
               for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG)
                  printf("checking popping stage %d beads %d %d on chains %d %d exposed %d %d active %s %s max iv %f\n",
                         k, i, j,
                         bead_model[i].chain,
                         bead_model[j].chain,
                         bead_model[i].exposed_code,
                         bead_model[j].exposed_code,
                         bead_model[i].active ? "Y" : "N",
                         bead_model[j].active ? "Y" : "N",
                         max_intersection_volume
                         );
#endif
                  if (bead_model[i].active &&
                      bead_model[j].active &&
                      ( ((methods[k] & POP_SC) &&
                         bead_model[i].chain == 1 &&
                         bead_model[j].chain == 1) ||
                        ((methods[k] & POP_MC) &&
                         bead_model[i].chain == 0 &&
                         bead_model[j].chain == 0) ||
                        ((methods[k] & POP_MCSC)
                         // &&
                         // (bead_model[i].chain != 1 ||
                         // bead_model[j].chain != 1))
                         ) ) &&
                      ( ((methods[k] & POP_EXPOSED) &&
                         bead_model[i].exposed_code == 1 &&
                         bead_model[j].exposed_code == 1) ||
                        ((methods[k] & POP_BURIED) &&
                         (bead_model[i].exposed_code != 1 ||
                          bead_model[j].exposed_code != 1)) ||
                        (methods[k] & POP_ALL) )) {
                     intersection_volume =
                        int_vol_2sphere(
                                        bead_model[i].bead_computed_radius,
                                        bead_model[j].bead_computed_radius,
                                        sqrt(
                                             pow(bead_model[i].bead_coordinate.axis[0] -
                                                 bead_model[j].bead_coordinate.axis[0], 2) +
                                             pow(bead_model[i].bead_coordinate.axis[1] -
                                                 bead_model[j].bead_coordinate.axis[1], 2) +
                                             pow(bead_model[i].bead_coordinate.axis[2] -
                                                 bead_model[j].bead_coordinate.axis[2], 2)) );
#if defined(DEBUG)
                     printf("this overlap bead %u %u vol %f rv1 %f rv2 %f r1 %f r2 %f p1 [%f,%f,%f] p2 [%f,%f,%f]\n",
                            bead_model[i].serial,
                            bead_model[j].serial,
                            intersection_volume,
                            bead_model[i].bead_ref_volume,
                            bead_model[j].bead_ref_volume,
                            bead_model[i].bead_computed_radius,
                            bead_model[j].bead_computed_radius,
                            bead_model[i].bead_coordinate.axis[0],
                            bead_model[i].bead_coordinate.axis[1],
                            bead_model[i].bead_coordinate.axis[2],
                            bead_model[j].bead_coordinate.axis[0],
                            bead_model[j].bead_coordinate.axis[1],
                            bead_model[j].bead_coordinate.axis[2]
                            );
#endif
                     if (intersection_volume > bead_model[i].bead_ref_volume * overlap[k] ||
                         intersection_volume > bead_model[j].bead_ref_volume * overlap[k]) {
                        overlaps_exist = true;
                        if (intersection_volume > max_intersection_volume) {
#if defined(DEBUG)
                           printf("best overlap so far bead %u %u vol %f\n",
                                  bead_model[i].serial,
                                  bead_model[j].serial,
                                  intersection_volume);
#endif
                           max_intersection_volume = intersection_volume;
                           max_bead1 = i;
                           max_bead2 = j;
                        }
                     }
                  }
               }
            }

            bool back_to_zero = false;
            if (overlaps_exist) {
               beads_popped++;
               lbl_core_progress->setText(QString("Stage %1 popping iteration %2 beads popped %3").arg(k+1).arg(iter).arg(beads_popped));
               qApp->processEvents();
               //#define DEBUG_FUSED
#if defined(DEBUG1) || defined(DEBUG) || defined(DEBUG_FUSED)
               printf("popping beads %u %u int vol %f mw1 %f mw2 %f v1 %f v2 %f c1 [%f,%f,%f] c2 [%f,%f,%f]\n",
                      max_bead1,
                      max_bead2,
                      max_intersection_volume,
                      bead_model[max_bead1].bead_ref_mw,
                      bead_model[max_bead2].bead_ref_mw,
                      bead_model[max_bead1].bead_ref_volume,
                      bead_model[max_bead2].bead_ref_volume,
                      bead_model[max_bead1].bead_coordinate.axis[0],
                      bead_model[max_bead1].bead_coordinate.axis[1],
                      bead_model[max_bead1].bead_coordinate.axis[2],
                      bead_model[max_bead2].bead_coordinate.axis[0],
                      bead_model[max_bead2].bead_coordinate.axis[1],
                      bead_model[max_bead2].bead_coordinate.axis[2]
                      );
#endif
               if (bead_model[max_bead1].chain == 0 &&
                   bead_model[max_bead2].chain == 1) {
                  // always select the sc!
#if defined(DEBUG1) || defined(DEBUG) || defined(DEBUG_FUSED)
                  puts("swap beads");
#endif
                  int tmp = max_bead2;
                  max_bead2 = max_bead1;
                  max_bead1 = tmp;
               }
               if (bead_model[max_bead1].chain != bead_model[max_bead2].chain &&
                   k == 1) {
                  back_to_zero = true;
               }
               // bead_model[max_bead1].all_beads.push_back(&bead_model[max_bead1]); ??
               bead_model[max_bead1].all_beads.push_back(&bead_model[max_bead2]);
               for (unsigned int n = 0; n < bead_model[max_bead2].all_beads.size(); n++) {
                  bead_model[max_bead1].all_beads.push_back(bead_model[max_bead2].all_beads[n]);
               }

               bead_model[max_bead2].active = false;
               for (unsigned int m = 0; m < 3; m++) {
                  bead_model[max_bead1].bead_coordinate.axis[m] *= bead_model[max_bead1].bead_ref_mw + bead_model[max_bead1].bead_ref_ionized_mw_delta;
                  bead_model[max_bead1].bead_coordinate.axis[m] +=
                     bead_model[max_bead2].bead_coordinate.axis[m] * ( bead_model[max_bead2].bead_ref_mw + bead_model[max_bead2].bead_ref_ionized_mw_delta );
                  bead_model[max_bead1].bead_coordinate.axis[m] /=
                     bead_model[max_bead1].bead_ref_mw + bead_model[max_bead1].bead_ref_ionized_mw_delta +
                     bead_model[max_bead2].bead_ref_mw + bead_model[max_bead2].bead_ref_ionized_mw_delta;
               }
               bead_model[max_bead1].bead_ref_mw               = bead_model[max_bead1].bead_ref_mw + bead_model[max_bead2].bead_ref_mw;
               bead_model[max_bead1].bead_ref_ionized_mw_delta = bead_model[max_bead1].bead_ref_ionized_mw_delta + bead_model[max_bead2].bead_ref_ionized_mw_delta;
               bead_model[max_bead1].bead_ref_volume           = bead_model[max_bead1].bead_ref_volume + bead_model[max_bead2].bead_ref_volume;
               // - max_intersection_volume;
               bead_model[max_bead1].bead_actual_radius =
                  bead_model[max_bead1].bead_computed_radius =
                  pow(3 * bead_model[max_bead1].bead_ref_volume / (4.0*M_PI), 1.0/3);
               // if fusing with a side chain bead, make sure the fused is side-chain
               // if (bead_model[max_bead2].chain) {
               //   bead_model[max_bead1].chain = 1;
               // }
               bead_model[max_bead1].normalized_ot_is_valid = false;
               bead_model[max_bead2].normalized_ot_is_valid = false;
#if defined(DEBUG)
               printf("after popping beads %d %d int volume %f radius %f mw %f vol %f coordinate [%f,%f,%f]\n",
                      bead_model[max_bead1].serial,
                      bead_model[max_bead2].serial,
                      intersection_volume,
                      bead_model[max_bead1].bead_computed_radius,
                      bead_model[max_bead1].bead_ref_mw,
                      bead_model[max_bead2].bead_ref_volume,
                      bead_model[max_bead1].bead_coordinate.axis[0],
                      bead_model[max_bead1].bead_coordinate.axis[1],
                      bead_model[max_bead1].bead_coordinate.axis[2]
                      );
#endif
               if (back_to_zero) {
                  editor->append(QString("Beads popped %1, Go back to stage %2\n").arg(beads_popped).arg(k));
                  printf("fused sc/mc bead in stage SC/MC, back to stage SC\n");
                  k = 0;
                  progress->setValue(progress->value() - 4);
                  goto stage_loop;
               }
            }
         } // if pop method
         // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_ap-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
         // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-ap-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
         qApp->processEvents();
         if (stopFlag)
         {
            return;
         }
      } while(overlaps_exist);
#if defined(TIMING)
      gettimeofday(&end_tv, NULL);
      printf("popping %d time %lu\n",
             k,
             1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
             start_tv.tv_usec);
      fflush(stdout);
#endif
#if defined(WRITE_EXTRA_FILES)
      write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
      write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO, &bead_model);
#endif
      printf("stage %d beads popped %d\n", k, beads_popped);
      progress->setValue(progress->value() + 1);
      editor->append(QString("Beads popped %1.\nBegin radial reduction stage %2\n").arg(beads_popped).arg(k + 1));
      qApp->processEvents();

      // radial reduction phase

#if defined(TIMING)
      gettimeofday(&start_tv, NULL);
#endif

      if (methods[k] & RADIAL_REDUCTION) {
         BPair pair;
         vector <BPair> pairs;

         vector <bool> reduced;
         vector <bool> reduced_any; // this is for a final recomputation of the volumes
         vector <bool> last_reduced; // to prevent rescreening
         reduced.resize(bead_model.size());
         reduced_any.resize(bead_model.size());
         last_reduced.resize(bead_model.size());

         for (unsigned int i = 0; i < bead_model.size(); i++) {
            reduced_any[i] = false;
            last_reduced[i] = true;
         }

         int iter = 0;
         int count;
         float max_intersection_length;
         // bool tb[bead_model.size() * bead_model.size()];
         // printf("sizeof tb %d, bm.size^2 %d\n",
         //     sizeof(tb), bead_model.size() * bead_model.size());
#if defined(DEBUG_OVERLAP)
         overlap_check(methods[k] & RR_SC ? true : false,
                       methods[k] & RR_MCSC ? true : false,
                       methods[k] & RR_BURIED ? true : false,
                       overlap_tolerance);
#endif
         if (methods[k] & RR_HIERC) {
#if defined(DEBUG1) || defined(DEBUG)
            printf("preprocessing processing hierarchical radial reduction\n");
#endif
            lbl_core_progress->setText(QString("Stage %1 hierarchical radial reduction").arg(k+1));
            qApp->processEvents();
            max_intersection_length = 0;
            pairs.clear( );
            count = 0;
            // build list of intersecting pairs
            for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
               for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG)
                  printf("checking radial stage %d beads %d %d on chains %d %d exposed code %d %d active %s %s max il %f %s %s %s %s\n",
                         k, i, j,
                         bead_model[i].chain,
                         bead_model[j].chain,
                         bead_model[i].exposed_code,
                         bead_model[j].exposed_code,
                         bead_model[i].active ? "Y" : "N",
                         bead_model[j].active ? "Y" : "N",
                         max_intersection_length,
                         (methods[k] & RR_BURIED) ? "RR_BURIED" : "!RR_BURIED",
                         (methods[k] & RR_SC) ? "RR_SC" : "!RR_SC",
                         (methods[k] & RR_MCSC) ? "RR_MCSC" : "!RR_MCSC",
                         (bead_model[i].active &&
                          bead_model[j].active &&
                          (methods[k] & RR_MCSC ||
                           ((methods[k] & RR_SC) &&
                            bead_model[i].chain == 1 &&
                            bead_model[j].chain == 1)) &&
                          ((methods[k] & RR_BURIED) ||
                           (bead_model[i].exposed_code == 1 &&
                            bead_model[j].exposed_code == 1)) &&
                          bead_model[i].bead_computed_radius > TOLERANCE &&
                          bead_model[j].bead_computed_radius > TOLERANCE ) ? "logic true" : "logic false"
                         );
#endif
                  //                  bool active = bead_model[i].active && bead_model[j].active;
                     
                  if (
                      bead_model[i].active &&
                      bead_model[j].active &&
                      (
                       methods[k] & RR_MCSC ||
                       ((methods[k] & RR_SC) &&
                        bead_model[i].chain == 1 &&
                        bead_model[j].chain == 1)) &&
                      ((methods[k] & RR_BURIED) ||
                       (bead_model[i].exposed_code == 1 ||
                        bead_model[j].exposed_code == 1)
                       ) &&
                      bead_model[i].bead_computed_radius > TOLERANCE &&
                      bead_model[j].bead_computed_radius > TOLERANCE
                      ) {

                     float separation =
                        bead_model[i].bead_computed_radius +
                        bead_model[j].bead_computed_radius -
                        sqrt(
                             pow(bead_model[i].bead_coordinate.axis[0] -
                                 bead_model[j].bead_coordinate.axis[0], 2) +
                             pow(bead_model[i].bead_coordinate.axis[1] -
                                 bead_model[j].bead_coordinate.axis[1], 2) +
                             pow(bead_model[i].bead_coordinate.axis[2] -
                                 bead_model[j].bead_coordinate.axis[2], 2));

#if defined(DEBUG)
                     printf("beads %d %d with radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] have a sep of %f\n",
                            i, j,
                            bead_model[i].bead_computed_radius,
                            bead_model[j].bead_computed_radius,
                            bead_model[i].bead_coordinate.axis[0],
                            bead_model[i].bead_coordinate.axis[1],
                            bead_model[i].bead_coordinate.axis[2],
                            bead_model[j].bead_coordinate.axis[0],
                            bead_model[j].bead_coordinate.axis[1],
                            bead_model[j].bead_coordinate.axis[2],
                            separation);
#endif
                     if (separation <= TOLERANCE) {
                        continue;
                     }

                     pair.i = i;
                     pair.j = j;
                     pair.separation = separation;
                     pair.active = true;
                     pairs.push_back(pair);
                     count++;
                  }
               }
            }
            // ok, now we have the list of pairs
            lbl_core_progress->setText(QString("Stage %1 hierarchical radial reduction").arg(k+1));
            qApp->processEvents();
            max_bead1 =
               max_bead2 = -1;
            float radius_delta;
            do {
               iter++;
#if defined(DEBUG1) || defined(DEBUG)
               printf("processing hierarchical radial reduction iteration %d\n", iter);
#endif
               lbl_core_progress->setText(QString("Stage %1 hierarch. red. it. %2 with %3 couples").arg(k+1).arg(iter).arg(pairs.size()));
               qApp->processEvents();
               max_intersection_length = 0;
               int max_pair = -1;
               count = 0;
               for (unsigned int i = 0; i < pairs.size(); i++) {
#if defined(DEBUG_OVERLAP)
                  printf("pair %d %d sep %f %s %s\n",
                         pairs[i].i, pairs[i].j, pairs[i].separation, pairs[i].active ? "active" : "not active",
                         pairs[i].i == max_bead1 ||
                         pairs[i].j == max_bead1 ||
                         pairs[i].i == max_bead2 ||
                         pairs[i].j == max_bead2 ? "needs recompute of separation" : "separation valid");
#endif
                  if (pairs[i].active) {
                     if (
                         pairs[i].i == max_bead1 ||
                         pairs[i].j == max_bead1 ||
                         pairs[i].i == max_bead2 ||
                         pairs[i].j == max_bead2
                         ) {
                        pairs[i].separation =
                           bead_model[pairs[i].i].bead_computed_radius +
                           bead_model[pairs[i].j].bead_computed_radius -
                           sqrt(
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[0] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[0], 2) +
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[1] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[1], 2) +
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[2] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[2], 2));
                        pairs[i].active = true;
                     }
                     if (pairs[i].separation > max_intersection_length) {
                        max_intersection_length = pairs[i].separation;
                        max_pair = i;
                     }
                  }
               }
#if defined(DEBUG) 
               printf("1: max intersection length %f\n", max_intersection_length);
#endif


               if (max_intersection_length > TOLERANCE) {
                  count++;
#if defined(DEBUG) 
                  printf("1: count %d\n", count);
#endif
                  pairs[max_pair].active = false;
                  max_bead1 = pairs[max_pair].i;
                  max_bead2 = pairs[max_pair].j;
#if defined(DEBUG1) || defined(DEBUG)
                  printf("processing radial reduction hierc iteration %d pair %d processed %d\n", iter, max_pair, count);
                  printf("reducing beads %d %d\n", max_bead1, max_bead2);
#endif
                  do {
#if defined(DEBUG3) 
                     puts("rr0");
#endif
                     if (methods[k] & OUTWARD_TRANSLATION ||
                         ((bead_model[max_bead1].chain == 1 ||
                           bead_model[max_bead2].chain == 1) &&
                          methods[0] & OUTWARD_TRANSLATION)) {
                        // new 1 step ot
#if defined(DEBUG3) 
                        puts("rr1");
#endif
                        if((methods[k] & RR_MCSC &&
                            (bead_model[max_bead1].chain == 1 ||
                             bead_model[max_bead2].chain == 1))
                           ) {
#if defined(DEBUG3) 
                           puts("rr2");
#endif
                           // new 1 bead 1 OT / treat as no ot...
                           int use_bead;
                           if (bead_model[max_bead1].chain == 1) {
                              use_bead = max_bead2;
                           } 
                           else 
                           {
                              use_bead = max_bead1;
                           }
                           // one bead to shrink
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta = max_intersection_length;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required, one bead to shrink, radius delta %f  cr %f %f\n",
                                  use_bead,
                                  radius_delta,
                                  bead_model[use_bead].bead_computed_radius,
                                  bead_model[use_bead].bead_computed_radius - radius_delta
                                  );
#endif
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                        else 
                        {
                           int use_bead = max_bead1;
#if defined(DEBUG3) 
                           printf("rr4 notv %d\n", bead_model[use_bead].normalized_ot_is_valid);
#endif
                           if (!bead_model[use_bead].normalized_ot_is_valid) {
                              float norm = 0.0;
                              for (unsigned int l = 0; l < 3; l++) {
                                 bead_model[use_bead].normalized_ot.axis[l] =
                                    bead_model[use_bead].bead_coordinate.axis[l] -
                                    molecular_cog[l];
                                 norm +=
                                    bead_model[use_bead].normalized_ot.axis[l] *
                                    bead_model[use_bead].normalized_ot.axis[l];
                              }
                              norm = sqrt(norm);
                              if (norm) {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                 }
                                 bead_model[use_bead].normalized_ot_is_valid = true;
                              } 
                              else 
                              {
                                 printf("wow! bead %d is at the molecular cog!\n", use_bead);
                              }
                           }
                           use_bead = max_bead2;
                           if (!bead_model[use_bead].normalized_ot_is_valid) {
                              float norm = 0.0;
                              for (unsigned int l = 0; l < 3; l++) {
                                 bead_model[use_bead].normalized_ot.axis[l] =
                                    bead_model[use_bead].bead_coordinate.axis[l] -
                                    molecular_cog[l];
                                 norm +=
                                    bead_model[use_bead].normalized_ot.axis[l] *
                                    bead_model[use_bead].normalized_ot.axis[l];
                              }
                              norm = sqrt(norm);
                              if (norm) {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                 }
                                 bead_model[use_bead].normalized_ot_is_valid = true;
                              }
                              else 
                              {
                                 printf("wow! bead %d is at the molecular cog!\n", use_bead);
                              }
                           }
                           // we need to handle 1 fixed case and
                           // the slight potential of one being at the molecular cog
                           reduced_any[max_bead1] = true;
                           reduced_any[max_bead2] = true;
                           outward_translate_2_spheres(
                                                       &bead_model[max_bead1].bead_computed_radius,
                                                       &bead_model[max_bead2].bead_computed_radius,
                                                       bead_model[max_bead1].bead_coordinate.axis,
                                                       bead_model[max_bead2].bead_coordinate.axis,
                                                       bead_model[max_bead1].normalized_ot.axis,
                                                       bead_model[max_bead2].normalized_ot.axis
                                                       );
                        }
                     }
                     else 
                     {
#if defined(DEBUG3) 
                        puts("rr5");
#endif
                        // no outward translation is required for either bead
                        // are we shrinking just 1 bead ... if we are dealing with buried beads, then
                        // only buried beads should be shrunk, not exposed beads
#if defined(DEBUG2)
                        printf("no outward translation is required\n");
#endif
                        if(methods[k] & RR_BURIED &&
                           bead_model[max_bead1].exposed_code == 1 &&
                           bead_model[max_bead2].exposed_code == 1) {
                           printf("what are we doing here?  buried and two exposed??\n");
                           // exit(-1);
                        }
                        if(methods[k] & RR_MCSC &&
                           !(methods[k] & RR_BURIED) &&
                           bead_model[max_bead1].chain == 1 &&
                           bead_model[max_bead2].chain == 1) {
                           printf("what are we doing here?  dealing with 2 SC's on the MCSC run??\n");
                           // exit(-1);
                        }
                        if((methods[k] & RR_BURIED &&
                            (bead_model[max_bead1].exposed_code == 1 ||
                             bead_model[max_bead2].exposed_code == 1)) ||
                           (methods[k] & RR_MCSC &&
                            !(methods[k] & RR_BURIED) &&
                            (bead_model[max_bead1].chain == 1 ||
                             bead_model[max_bead2].chain == 1))) {
                           // only one bead to shrink, since
                           // we are either buried with one of the beads exposed or
                           // on the MCSC and one of the beads is SC
                           int use_bead;
                           if (methods[k] & RR_BURIED) {
                              if (bead_model[max_bead1].exposed_code == 1) {
                                 use_bead = max_bead2;
                              }
                              else 
                              {
                                 use_bead = max_bead1;
                              }
                           }
                           else 
                           {
                              if (bead_model[max_bead1].chain == 1) {
                                 use_bead = max_bead2;
                              }
                              else 
                              {
                                 use_bead = max_bead1;
                              }
                           }
                           // one bead to shrink
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta = max_intersection_length;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required, one bead to shrink, radius delta %f  cr %f %f\n",
                                  use_bead,
                                  radius_delta,
                                  bead_model[use_bead].bead_computed_radius,
                                  bead_model[use_bead].bead_computed_radius - radius_delta
                                  );
#endif
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                        else 
                        {
#if defined(DEBUG3) 
                           puts("rr6");
#endif
                           // two beads to shrink
                           int use_bead = max_bead1;
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta =
                              // bead_model[use_bead].bead_actual_radius * rr_overlap[k];
                              max_intersection_length * bead_model[max_bead1].bead_computed_radius /
                              (bead_model[max_bead1].bead_computed_radius + bead_model[max_bead2].bead_computed_radius);
                           float radius_delta2 =
                              // bead_model[use_bead].bead_actual_radius * rr_overlap[k];
                              max_intersection_length * bead_model[max_bead2].bead_computed_radius /
                              (bead_model[max_bead1].bead_computed_radius + bead_model[max_bead2].bead_computed_radius);
#if defined(DEBUG2)
                           printf("intersection len %f recomputed %f radius delta %f r1 %f r2 %f\n",
                                  max_intersection_length,
                                  bead_model[max_bead1].bead_computed_radius +
                                  bead_model[max_bead2].bead_computed_radius -
                                  sqrt(
                                       pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                           bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                           bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                           bead_model[max_bead2].bead_coordinate.axis[2], 2)),
                                  radius_delta,
                                  bead_model[max_bead1].bead_computed_radius,
                                  bead_model[max_bead2].bead_computed_radius
                                  );
#endif
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE / 2);
                              reduced[use_bead] = false;
                           }

#if defined(DEBUG2)
                           printf("intersection len %f recomputed %f radius delta %f r1 %f r2 %f\n",
                                  max_intersection_length,
                                  bead_model[max_bead1].bead_computed_radius +
                                  bead_model[max_bead2].bead_computed_radius -
                                  sqrt(
                                       pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                           bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                           bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                           bead_model[max_bead2].bead_coordinate.axis[2], 2)),
                                  radius_delta,
                                  bead_model[max_bead1].bead_computed_radius,
                                  bead_model[max_bead2].bead_computed_radius
                                  );
#endif
                           use_bead = max_bead2;
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = radius_delta2;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                     }
#if defined(DEBUG2)
                     printf("b1r %f b2r %f current separation %f\n",
                            bead_model[max_bead1].bead_computed_radius,
                            bead_model[max_bead2].bead_computed_radius,
                            bead_model[max_bead1].bead_computed_radius +
                            bead_model[max_bead2].bead_computed_radius -
                            sqrt(
                                 pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                     bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                     bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                     bead_model[max_bead2].bead_coordinate.axis[2], 2)));
                     printf("flags %s %s %s %s\n",
                            bead_model[max_bead1].bead_computed_radius > TOLERANCE ? "Y" : "N",
                            bead_model[max_bead2].bead_computed_radius > TOLERANCE ? "Y" : "N",
                            (bead_model[max_bead1].bead_computed_radius +
                             bead_model[max_bead2].bead_computed_radius -
                             sqrt(
                                  pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                      bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                      bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                      bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE) ? "Y" : "N",
                            bead_model[max_bead1].bead_computed_radius > TOLERANCE &&
                            bead_model[max_bead2].bead_computed_radius > TOLERANCE &&
                            (bead_model[max_bead1].bead_computed_radius +
                             bead_model[max_bead2].bead_computed_radius -
                             sqrt(
                                  pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                      bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                      bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                      bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE) ? "Y" : "N");
#endif
                  } while (
                           bead_model[max_bead1].bead_computed_radius > TOLERANCE &&
                           bead_model[max_bead2].bead_computed_radius > TOLERANCE &&
                           (bead_model[max_bead1].bead_computed_radius +
                            bead_model[max_bead2].bead_computed_radius -
                            sqrt(
                                 pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                     bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                     bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                     bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE));
#if defined(DEBUG2)
                  printf("out of while 1\n");
#endif
               } // if max intersection length > TOLERANCE
               qApp->processEvents();
               if (stopFlag)
               {
                  return;
               }
            } while(count);
#if defined(DEBUG2)
            printf("out of while 2 count = %d\n", count);
#endif
         }
         else 
         {
            // simultaneous reduction
            lbl_core_progress->setText(QString("Stage %1 sychronous radial reduction").arg(k+1));
            qApp->processEvents();
#if defined(USE_THREADS)
            unsigned int threads = numThreads;
            vector < radial_reduction_thr_t* > radial_reduction_thr_threads(threads);
            for ( unsigned int j = 0; j < threads; j++ )
            {
               radial_reduction_thr_threads[j] = new radial_reduction_thr_t(j);
               radial_reduction_thr_threads[j]->start();
            }
#endif

            do {
#if defined(DEBUG_OVERLAP)
               overlap_check(methods[k] & RR_SC ? true : false,
                             methods[k] & RR_MCSC ? true : false,
                             methods[k] & RR_BURIED ? true : false,
                             overlap_tolerance);
#endif
               // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_br-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
               // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-br-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
               iter++;
#if defined(DEBUG1) || defined(DEBUG)
               printf("processing synchronous radial reduction iteration %d\n", iter);
               editor->append(QString(" %1").arg(iter));
#endif
               //   lbl_core_progress->setText(QString("Stage %1 synchronous radial reduction iteration %2").arg(k+1).arg(iter));
               //               qApp->processEvents();
               if(iter > 10000) {
                  printf("too many iterations\n");
                  exit(-1);
               }
               max_intersection_length = 0;
               pairs.clear( );
               count = 0;
               reduced[bead_model.size() - 1] = false;
#if defined(USE_THREADS)
               {
                  unsigned int i;
                  unsigned int j;

                  vector < BPair > my_pairs[ MAX_THREADS ];
                  for ( j = 0; j < threads; j++ )
                  {
# if defined(DEBUG_THREAD)
                     cout << "thread " << j << endl;
# endif
                     radial_reduction_thr_threads[j]->radial_reduction_thr_setup(
                                                        methods[k],
                                                        &bead_model,
                                                        &last_reduced,
                                                        &reduced,
                                                        &my_pairs[j],
                                                        threads,
                                                        overlap_tolerance
                                                        );

                  }

                  for ( j = 0; j < threads; j++ )
                  {
                     radial_reduction_thr_threads[j]->radial_reduction_thr_wait();
                  }

                  // merge results
                  for ( j = 0; j < threads; j++ )
                  {
                     for ( i = 0; i < my_pairs[j].size(); i++ ) 
                     {
                        pairs.push_back(my_pairs[j][i]);
                        
                        if (my_pairs[j][i].separation > max_intersection_length) {
                           max_intersection_length = my_pairs[j][i].separation;
                           max_bead1 = my_pairs[j][i].i;
                           max_bead2 = my_pairs[j][i].j;
                        }
                        count++;
                     }
                  }
	       }
#else // !defined(USE_THREADS)
               for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
                  reduced[i] = false;
                  if (1 || last_reduced[i]) {
                     for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUGX)
                        printf("checking radial stage %d beads %d %d on chains %d %d exposed code %d %d active %s %s max il %f\n",
                               k, i, j,
                               bead_model[i].chain,
                               bead_model[j].chain,
                               bead_model[i].exposed_code,
                               bead_model[j].exposed_code,
                               bead_model[i].active ? "Y" : "N",
                               bead_model[j].active ? "Y" : "N",
                               max_intersection_length
                               );
#endif
                        if ((1 || last_reduced[j]) &&
                            bead_model[i].active &&
                            bead_model[j].active &&
                            (methods[k] & RR_MCSC ||
                             ((methods[k] & RR_SC) &&
                              bead_model[i].chain == 1 &&
                              bead_model[j].chain == 1)) &&
                            ((methods[k] & RR_BURIED) ||
                             (bead_model[i].exposed_code == 1 ||
                              bead_model[j].exposed_code == 1)) &&
                            bead_model[i].bead_computed_radius > TOLERANCE &&
                            bead_model[j].bead_computed_radius > TOLERANCE
                            ) {
                           float separation =
                              bead_model[i].bead_computed_radius +
                              bead_model[j].bead_computed_radius -
                              sqrt(
                                   pow(bead_model[i].bead_coordinate.axis[0] -
                                       bead_model[j].bead_coordinate.axis[0], 2) +
                                   pow(bead_model[i].bead_coordinate.axis[1] -
                                       bead_model[j].bead_coordinate.axis[1], 2) +
                                   pow(bead_model[i].bead_coordinate.axis[2] -
                                       bead_model[j].bead_coordinate.axis[2], 2));


                           if (separation <= TOLERANCE) {
                              continue;
                           }

#if defined(DEBUG)
                           printf("beads %d %d with radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] have a sep of %f\n",
                                  i, j,
                                  bead_model[i].bead_computed_radius,
                                  bead_model[j].bead_computed_radius,
                                  bead_model[i].bead_coordinate.axis[0],
                                  bead_model[i].bead_coordinate.axis[1],
                                  bead_model[i].bead_coordinate.axis[2],
                                  bead_model[j].bead_coordinate.axis[0],
                                  bead_model[j].bead_coordinate.axis[1],
                                  bead_model[j].bead_coordinate.axis[2],
                                  separation);
#endif

                           if (separation > max_intersection_length) {
                              max_intersection_length = separation;
                              max_bead1 = i;
                              max_bead2 = j;
                           }
                           pair.i = i;
                           pair.j = j;
                           pair.separation = separation;
                           pairs.push_back(pair);
                           count++;
                        }
                     }
                  }// if last_reduced[i]
               }
#endif // !defined(USE_THERADS)

#if defined(DEBUG1) || defined(DEBUG)
               printf("processing radial reduction sync iteration %d pairs to process %d max int len %f\n", iter, count, max_intersection_length);
#endif
               lbl_core_progress->setText(QString("Stage %1 synchron. red. it. %2 with %3 couples").arg(k+1).arg(iter).arg(pairs.size()));
               qApp->processEvents();
               if (max_intersection_length > TOLERANCE) {
#if defined(DEBUG1) || defined(DEBUG)
                  printf("processing radial reduction sync iteration %d pairs to process %d\n", iter, count);
#endif
                  for (unsigned int i = 0; i < pairs.size(); i++) {
                     if (
                         !reduced[pairs[i].i] &&
                         (k == 3 ||
                          (
                           (bead_model[pairs[i].i].exposed_code != 1 ||
                            methods[k] & RR_EXPOSED ||
                            methods[k] & RR_ALL) &&
                           (!(methods[k] & RR_MCSC) ||
                            bead_model[pairs[i].i].chain == 0 ||
                            (methods[k] & RR_BURIED &&
                             bead_model[pairs[i].i].exposed_code != 1))
                           ) )
                         ) {
                        int use_bead = pairs[i].i;
                        /*      if ( !(methods[k] & RR_MCSC) ||
                                bead_model[use_bead].exposed_code != 1 ||
                                bead_model[use_bead].chain == 0 ||
                                (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1) ) */
                        if(1) {
#if defined(DEBUG)
                           printf("reducing beads %d\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = bead_model[use_bead].bead_computed_radius * rr_overlap[k];
                           if (radius_delta < TOLERANCE) {
                              radius_delta = (float)TOLERANCE;
                           }
                           if (methods[k] & OUTWARD_TRANSLATION ||
                               (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1 &&
                                methods[0] & OUTWARD_TRANSLATION) ) {
#if defined(DEBUG)
                              printf("outward translation from [%f,%f,%f] to ",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                              if (!bead_model[use_bead].normalized_ot_is_valid) {
                                 float norm = 0.0;
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] =
                                       bead_model[use_bead].bead_coordinate.axis[l] -
                                       molecular_cog[l];
                                    norm +=
                                       bead_model[use_bead].normalized_ot.axis[l] *
                                       bead_model[use_bead].normalized_ot.axis[l];
                                 }
                                 norm = sqrt(norm);
                                 if (norm) {
                                    for (unsigned int l = 0; l < 3; l++) {
                                       bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                       bead_model[use_bead].bead_coordinate.axis[l] +=
                                          radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                    }
                                    bead_model[use_bead].normalized_ot_is_valid = true;
                                 }
                                 else 
                                 {
                                    printf("wow! bead %d is at the molecular cog!\n", use_bead);
                                 }
                              }
                              else 
                              {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].bead_coordinate.axis[l] +=
                                       radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                 }
                              }
#if defined(DEBUG)
                              printf(" [%f,%f,%f]\n",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)TOLERANCE * 0.9999;
                              reduced[use_bead] = false;
                           }
                        }
                     }
                     if (
                         !reduced[pairs[i].j] &&
                         (bead_model[pairs[i].j].exposed_code != 1 ||
                          methods[k] & RR_EXPOSED ||
                          methods[k] & RR_ALL) &&
                         (!(methods[k] & RR_MCSC) ||
                          bead_model[pairs[i].j].chain == 0 ||
                          (methods[k] & RR_BURIED &&
                           bead_model[pairs[i].j].exposed_code != 1))
                         ) {
                        int use_bead = pairs[i].j;
                        /* if ( !(methods[k] & RR_MCSC) ||
                           bead_model[use_bead].chain == 0 ||
                           bead_model[use_bead].exposed_code != 1 ||
                           (bead_model[pairs[i].i].chain == 1 &&
                           bead_model[pairs[i].j].chain == 1) ) */
                        {
#if defined(DEBUG)
                           printf("reducing beads %d\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = bead_model[use_bead].bead_computed_radius * rr_overlap[k];
                           if (radius_delta < TOLERANCE) {
                              radius_delta = (float)TOLERANCE;
                           }
                           if (methods[k] & OUTWARD_TRANSLATION ||
                               (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1 &&
                                methods[0] & OUTWARD_TRANSLATION) ) {
#if defined(DEBUG)
                              printf("outward translation from [%f,%f,%f] to ",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                              if (!bead_model[use_bead].normalized_ot_is_valid) {
                                 float norm = 0.0;
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] =
                                       bead_model[use_bead].bead_coordinate.axis[l] -
                                       molecular_cog[l];
                                    norm +=
                                       bead_model[use_bead].normalized_ot.axis[l] *
                                       bead_model[use_bead].normalized_ot.axis[l];
                                 }
                                 norm = sqrt(norm);
                                 if (norm) {
                                    for (unsigned int l = 0; l < 3; l++) {
                                       bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                       bead_model[use_bead].bead_coordinate.axis[l] +=
                                          radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                    }
                                    bead_model[use_bead].normalized_ot_is_valid = true;
                                 }
                                 else 
                                 {
                                    printf("wow! bead %d is at the molecular cog!\n", use_bead);
                                 }
                              }
                              else 
                              {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].bead_coordinate.axis[l] +=
                                       radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                 }
                              }
#if defined(DEBUG)
                              printf(" [%f,%f,%f]\n",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)TOLERANCE * 0.9999;
                              reduced[use_bead] = false;
                           }
                        }
                     }
		  }
               }
               last_reduced = reduced;
               // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_ar-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
               // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-ar-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
               qApp->processEvents();
               if (stopFlag)
               {
#if defined(USE_THREADS)
                  {
                     unsigned int j;
                     // destroy
                     
                     for ( j = 0; j < threads; j++ )
                     {
                        radial_reduction_thr_threads[j]->radial_reduction_thr_shutdown();
                     }
                     
                     for ( j = 0; j < threads; j++ )
                     {
                        radial_reduction_thr_threads[j]->wait();
                     }
                     
                     for ( j = 0; j < threads; j++ )
                     {
                        delete radial_reduction_thr_threads[j];
                     }
                  }
#endif
                  return;
               }
            } while(count);
#if defined(USE_THREADS)
            {
               unsigned int j;
               // destroy
               
               for ( j = 0; j < threads; j++ )
               {
                  radial_reduction_thr_threads[j]->radial_reduction_thr_shutdown();
               }
               
               for ( j = 0; j < threads; j++ )
               {
                  radial_reduction_thr_threads[j]->wait();
               }
               
               for ( j = 0; j < threads; j++ )
               {
                  delete radial_reduction_thr_threads[j];
               }
            }
#endif
         }

#if defined(TIMING)
         gettimeofday(&end_tv, NULL);
         printf("radial reduction %d time %lu\n",
                k,
                1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
                start_tv.tv_usec);
         fflush(stdout);
#endif

         progress->setValue(progress->value() + 1);
         qApp->processEvents();

         // recompute volumes
         for (unsigned int i = 0; i < bead_model.size(); i++) {
            if (reduced_any[i]) {
#if defined(DEBUG1) || defined(DEBUG)
               printf("recomputing volume bead %d\n", i);
#endif
               bead_model[i].bead_ref_volume =
                  (4.0*M_PI/3.0) * pow(bead_model[i].bead_computed_radius, 3);
            }
         }
      }
#if defined(DEBUG_OVERLAP)
      if(overlap_check(methods[k] & RR_SC ? true : false,
                       methods[k] & RR_MCSC ? true : false,
                       methods[k] & RR_BURIED ? true : false,
                             overlap_tolerance)) {
         printf("internal over lap error... exiting!");
         exit(-1);
      }
#endif
#if defined(WRITE_EXTRA_FILES)
      write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
      write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO, &bead_model);
#endif
   } // methods
#if defined(WRITE_EXTRA_FILES)
   write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO + ".tsv", &bead_model);
   write_bead_spt(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO, &bead_model);
#endif
   //  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
   //       QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
   //       DOTSOMO, &bead_model);

   QString filename = 
      project + 
      QString( "_%1" ).arg( model_name( current_model ) );

   le_bead_model_file->setText( filename );

   write_bead_model(somo_dir + SLASH + filename +
                    QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") + DOTSOMO
                    , &bead_model);
   editor->append("Finished with popping and radial reduction\n");
   progress->setValue(end_progress); 
}

//------------------------------ end of radial reduction ------------------------------------------------------

int US_Hydrodyn::compute_asa( bool bd_mode, bool no_ovlp_removal )
{
   // advanced_config.debug_1 = true;
   // advanced_config.debug_2 = true;
   
   QString error_string = "";
   progress->reset();
   editor->append(QString("\nBuilding the bead model for %1 model %2\n").arg(project).arg( model_name( current_model ) ) );
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
   int retval = create_beads(&error_string);
   if ( retval )
   {
      editor->append("Errors found during the initial creation of beads\n");
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

   // info_mw( "compute_asa() after create_beads()", model_vector, true );
   
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
      int retval = surfracer_main(asa.probe_radius,
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
      // ASAB1
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
            // printf("p1 i j k %d %d %d %p %s\n", i, j, k, this_atom->p_atom, this_atom->active ? "active" : "not active"); fflush(stdout);

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
   // info_mw( "compute_asa() after pass 1", model_vector, true );

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
            // printf("p2 i j k %d %d %d %p\n", i, j, k, this_atom->p_atom); fflush(stdout);
            // this_atom->bead_positioner = false;
            if (this_atom->active) {
               if ( advanced_config.debug_1 )
               {
                  printf("pass 2 active %s %s %d pm %d %d\n",
                         this_atom->name.toLatin1().data(),
                         this_atom->resName.toLatin1().data(),
                         this_atom->serial,
                         this_atom->placing_method,
                         this_atom->bead_assignment); fflush(stdout);
               }
               molecular_mw += this_atom->mw + this_atom->ionized_mw_delta;
               for (unsigned int m = 0; m < 3; m++) {
                  molecular_cog[m] += this_atom->coordinate.axis[m] * ( this_atom->mw + this_atom->ionized_mw_delta );
               }

               this_atom->bead_mw               = 0;
               this_atom->bead_ionized_mw_delta = 0;
               this_atom->bead_asa              = 0;
               this_atom->bead_recheck_asa      = 0;

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
                        this_atom->bead_asa                += sidechain_N->bead_asa;
                        this_atom->bead_mw                 += sidechain_N->bead_mw;
                        this_atom->bead_ionized_mw_delta   += sidechain_N->bead_ionized_mw_delta;
                        sidechain_N->bead_mw               = 0;
                        sidechain_N->bead_ionized_mw_delta = 0;
                        sidechain_N->bead_asa              = 0;
                        sidechain_N                        = (PDB_atom *) 0;
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

               use_atom->bead_asa                  += this_atom->asa;
               use_atom->bead_mw                   += this_atom->mw;
               use_atom->bead_ionized_mw_delta     += this_atom->ionized_mw_delta;

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
                  use_atom->bead_cog_mw += this_atom->mw + this_atom->ionized_mw_delta;
                  for (unsigned int m = 0; m < 3; m++) {
                     use_atom->bead_cog_coordinate.axis[m] +=
                        this_atom->coordinate.axis[m] * ( this_atom->mw + this_atom->ionized_mw_delta );
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

   // info_mw( "compute_asa() after pass 2", model_vector, true );
   // // pass 2a set ref_mw from bead_mw
   // for ( int j = 0; j < (int) model_vector[ current_model ].molecule.size (); ++j) {
   //    for ( int k = 0; k < (int) model_vector[ current_model ].molecule[ j ].atom.size (); ++k) {
   //       PDB_atom *this_atom = &(model_vector[ current_model ].molecule[ j ].atom[ k ]);
   //       if ( this_atom->active &&
   //            this_atom->is_bead ) {
   //          this_atom->bead_ref_mw               = this_atom->bead_mw;
   //          this_atom->bead_ref_ionized_mw_delta = this_atom->bead_ionized_mw_delta;
   //       }
   //    }
   // }
         
   // info_mw( "compute_asa() after pass 2a", model_vector, true );
   
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
               double delta_mw = 1.01f;
               if ( multi_residue_map.count( "N1-" ) &&
                    multi_residue_map[ "N1-" ].size() &&
                    multi_residue_map.count( "PRO" ) &&
                    multi_residue_map[ "PRO" ].size()
                    ) {
                  int index1 = multi_residue_map[ "N1-" ][0];
                  int index2 = multi_residue_map[ "PRO" ][0];
                  delta_mw =
                     residue_list[ index1 ].r_atom[ 0 ].hybrid.mw
                     + residue_list[ index1 ].r_atom[ 0 ].hybrid.ionized_mw_delta
                     - residue_list[ index2 ].r_atom[ 0 ].hybrid.mw
                     - residue_list[ index2 ].r_atom[ 0 ].hybrid.ionized_mw_delta
                     ;
               }
               qDebug() << "handled first N mw delta " << delta_mw;
               this_atom->bead_ref_mw          += delta_mw;
               // if ( advanced_config.debug_1 )
               // {
               //    puts("pass 2b PRO 1st N +1 mw adjustment");
               // }
               // this_atom->bead_ref_mw += 1.01f;
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
                  this_atom->bead_ref_volume            = residue_list[posNPBR_OXT].r_bead[0].volume;
                  this_atom->bead_ref_mw                = residue_list[posNPBR_OXT].r_bead[0].mw;
                  this_atom->bead_ref_ionized_mw_delta  = residue_list[posNPBR_OXT].r_bead[0].ionized_mw_delta;
               }
                  
               last_main_chain_bead->bead_ref_volume           = this_atom->bead_ref_volume;
               last_main_chain_bead->bead_ref_mw               = this_atom->bead_ref_mw;
               last_main_chain_bead->bead_ref_ionized_mw_delta = this_atom->bead_ref_ionized_mw_delta;
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
                  last_main_chain_bead->bead_ref_mw += 1.01f;
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

                  last_main_chain_bead->bead_ref_volume           = this_atom->bead_ref_volume;
                  last_main_chain_bead->bead_ref_mw               = this_atom->bead_ref_mw;
                  last_main_chain_bead->bead_ref_ionized_mw_delta = this_atom->bead_ref_ionized_mw_delta;
                  last_main_chain_bead->bead_computed_radius      = this_atom->bead_computed_radius;
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
                  this_atom->bead_ref_volume           = residue_list[pos].r_bead[0].volume;
                  this_atom->bead_ref_mw               = residue_list[pos].r_bead[0].mw;
                  this_atom->bead_ref_ionized_mw_delta = residue_list[pos].r_bead[0].ionized_mw_delta;
                  this_atom->bead_computed_radius      = pow(3 * last_main_chain_bead->bead_ref_volume / (4.0*M_PI), 1.0/3);
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
               last_main_chain_bead->bead_ref_volume           = residue_list[pos].r_bead[0].volume;
               last_main_chain_bead->bead_ref_mw               = residue_list[pos].r_bead[0].mw;
               last_main_chain_bead->bead_ref_ionized_mw_delta = residue_list[pos].r_bead[0].ionized_mw_delta;
               last_main_chain_bead->bead_computed_radius      = pow(3 * last_main_chain_bead->bead_ref_volume / (4.0*M_PI), 1.0/3);
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

   // info_mw( "compute_asa() after pass 2b", model_vector, true );
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

   // info_mw( "compute_asa() after pass 2c", model_vector, true );

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
   // info_mw( "compute_asa() after pass 2d", model_vector, true );
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
            // printf("p3 i j k %d %d %d %p\n", i, j, k, this_atom->p_atom); fflush(stdout);
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

   // info_mw( "compute_asa() after pass 3", model_vector, true );

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
            // printf("p4 i j k %d %d %d %p\n", i, j, k, this_atom->p_atom); fflush(stdout);

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
   // build vector of beads
   bead_model.clear( );
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
                         last_res.toLatin1().data(),
                         last_resName.toLatin1().data(),
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
          last_resName.toLatin1().data(),
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
                  project + QString("_%1").arg( model_name( current_model ) ) +
                  QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
                  + DOTSOMO + ".asa_res", &bead_model);

#if defined(DEBUG_MOD)
   write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_debug" + DOTSOMO + ".tsv", &dbg_model);
#endif
   editor->append(QString("There are %1 beads in this model%2\n")
                  .arg(bead_model.size())
                  .arg(bd_mode ? "" : " before popping"));

   progress->setValue(ppos++); // 8
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }

   // #define DEBUG

   // popping radial reduction
#if !defined(POP_MC)
# define POP_MC              (1 << 0)
# define POP_SC              (1 << 1)
# define POP_MCSC            (1 << 2)
# define POP_EXPOSED         (1 << 3)
# define POP_BURIED          (1 << 4)
# define POP_ALL             (1 << 5)
# define RADIAL_REDUCTION    (1 << 6)
# define RR_MC               (1 << 7)
# define RR_SC               (1 << 8)
# define RR_MCSC             (1 << 9)
# define RR_EXPOSED          (1 << 10)
# define RR_BURIED           (1 << 11)
# define RR_ALL              (1 << 12)
# define OUTWARD_TRANSLATION (1 << 13)
# define RR_HIERC            (1 << 14)
# define MIN_OVERLAP 0.0
#endif

   // or sc fb Y rh Y rs Y to Y st Y ro Y 70.000000 1.000000 0.000000
   // or scmc fb Y rh N rs N to Y st N ro Y 1.000000 0.000000 70.000000
   // or bb fb Y rh Y rs N to N st Y ro N 0.000000 0.000000 0.000000

#if defined(DEBUG)
   printf("or sc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or scmc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or bb fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or gb fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n",
          sidechain_overlap.fuse_beads ? "Y" : "N",
          sidechain_overlap.remove_hierarch ? "Y" : "N",
          sidechain_overlap.remove_sync ? "Y" : "N",
          sidechain_overlap.translate_out ? "Y" : "N",
          sidechain_overlap.show_translate ? "Y" : "N",
          sidechain_overlap.remove_overlap ? "Y" : "N",
          sidechain_overlap.fuse_beads_percent,
          sidechain_overlap.remove_sync_percent,
          sidechain_overlap.remove_hierarch_percent,

          mainchain_overlap.fuse_beads ? "Y" : "N",
          mainchain_overlap.remove_hierarch ? "Y" : "N",
          mainchain_overlap.remove_sync ? "Y" : "N",
          mainchain_overlap.translate_out ? "Y" : "N",
          mainchain_overlap.show_translate ? "Y" : "N",
          mainchain_overlap.remove_overlap ? "Y" : "N",
          mainchain_overlap.fuse_beads_percent,
          mainchain_overlap.remove_sync_percent,
          mainchain_overlap.remove_hierarch_percent,

          buried_overlap.fuse_beads ? "Y" : "N",
          buried_overlap.remove_hierarch ? "Y" : "N",
          buried_overlap.remove_sync ? "Y" : "N",
          buried_overlap.translate_out ? "Y" : "N",
          buried_overlap.show_translate ? "Y" : "N",
          buried_overlap.remove_overlap ? "Y" : "N",
          buried_overlap.fuse_beads_percent,
          buried_overlap.remove_sync_percent,
          buried_overlap.remove_hierarch_percent,

          grid_overlap.fuse_beads ? "Y" : "N",
          grid_overlap.remove_hierarch ? "Y" : "N",
          grid_overlap.remove_sync ? "Y" : "N",
          grid_overlap.translate_out ? "Y" : "N",
          grid_overlap.show_translate ? "Y" : "N",
          grid_overlap.remove_overlap ? "Y" : "N",
          grid_overlap.fuse_beads_percent,
          grid_overlap.remove_sync_percent,
          grid_overlap.remove_hierarch_percent);

#endif

   int methods[] =
      {
         RADIAL_REDUCTION | RR_SC | RR_EXPOSED,
         RADIAL_REDUCTION | RR_MCSC | RR_EXPOSED,
         RADIAL_REDUCTION | RR_MCSC | RR_BURIED,
      };

   if ( !bd_mode &&
        sidechain_overlap.fuse_beads ) {
      methods[0] |= POP_SC | POP_EXPOSED;
   }

   if ( !bd_mode &&
        mainchain_overlap.fuse_beads) {
      methods[1] |= POP_MCSC | POP_EXPOSED;
   }

   if ( !bd_mode &&
        buried_overlap.fuse_beads ) {
      methods[2] |= POP_ALL | POP_BURIED;
   }

   if ( sidechain_overlap.remove_hierarch ) {
      methods[0] |= RR_HIERC;
   }

   if ( mainchain_overlap.remove_hierarch ) {
      methods[1] |= RR_HIERC;
   }

   if ( buried_overlap.remove_hierarch ) {
      methods[2] |= RR_HIERC;
   }

   if ( sidechain_overlap.translate_out ) {
      methods[0] |= OUTWARD_TRANSLATION;
   }

   if ( mainchain_overlap.translate_out ) {
      methods[1] |= OUTWARD_TRANSLATION;
   }

   if ( buried_overlap.translate_out ) {
      methods[2] |= OUTWARD_TRANSLATION;
   }

   if ( !sidechain_overlap.remove_overlap ) {
      methods[0] = 0;
   }

   if ( !mainchain_overlap.remove_overlap ) {
      methods[1] = 0;
   }

   if ( !buried_overlap.remove_overlap ) {
      methods[2] = 0;
   }

   if ( no_ovlp_removal ||
        ( bd_mode && 
          !bd_options.do_rr ) ) {
      methods[0] = 0;
      methods[1] = 0;
      methods[2] = 0;
      cout << "BD MODE REMOVED RR!\n"; 
   }

   float overlap[] =
      {
         (float) ( sidechain_overlap.fuse_beads_percent / 100.0 ),
         (float) ( mainchain_overlap.fuse_beads_percent / 100.0 ),
         (float) ( buried_overlap.fuse_beads_percent / 100.0 )
      };

   float rr_overlap[] =
      {
         (float) ( ( sidechain_overlap.remove_hierarch ?
                     sidechain_overlap.remove_hierarch_percent : sidechain_overlap.remove_sync_percent) / 100.0 ),
         (float) ( ( mainchain_overlap.remove_hierarch ?
                     mainchain_overlap.remove_hierarch_percent : mainchain_overlap.remove_sync_percent) / 100.0 ),
         (float) ( ( buried_overlap.remove_hierarch ?
                     buried_overlap.remove_hierarch_percent : buried_overlap.remove_sync_percent) / 100.0 )
      };


#if defined(WRITE_EXTRA_FILES)
   write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO + ".tsv", &bead_model);
   write_bead_spt(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO, &bead_model);
#endif
   for(unsigned int k = 0; k < sizeof(methods) / sizeof(int); k++) {

   stage_loop:

      int beads_popped = 0;

#if defined(DEBUG1) || defined(DEBUG)
      printf("popping stage %d %s%s%s%s%s%s%s%s%s%s%s%s%s%s%soverlap_reduction %f rroverlap %f\n",
             k,
             (methods[k] & POP_MC) ? "main chain " : "",
             (methods[k] & POP_SC) ? "side chain " : "",
             (methods[k] & POP_MCSC) ? "main & side chain " : "",
             (methods[k] & POP_EXPOSED) ? "exposed " : "",
             (methods[k] & POP_BURIED) ? "buried " : "",
             (methods[k] & POP_ALL) ? "all " : "",
             (methods[k] & RADIAL_REDUCTION) ? "radial reduction " : "",
             (methods[k] & RR_HIERC) ? "hierarchically " : "synchronously ",
             (methods[k] & RR_MC) ? "main chain " : "",
             (methods[k] & RR_SC) ? "side chain " : "",
             (methods[k] & RR_MCSC) ? "main & side chain " : "",
             (methods[k] & RR_EXPOSED) ? "exposed " : "",
             (methods[k] & RR_BURIED) ? "buried " : "",
             (methods[k] & RR_ALL) ? "all " : "",
             (methods[k] & OUTWARD_TRANSLATION) ? "outward translation " : "",
             overlap[k],
             rr_overlap[k]);
#endif

      if (overlap[k] < MIN_OVERLAP) {
         printf("using %f as minimum overlap\n", MIN_OVERLAP);
         overlap[k] = MIN_OVERLAP;
      }

      float max_intersection_volume;
      float intersection_volume = 0;
      int max_bead1 = 0;
      int max_bead2 = 0;
      unsigned iter = 0;
      bool overlaps_exist;
#if defined(TIMING)
      gettimeofday(&start_tv, NULL);
#endif
      if ( !bd_mode && !no_ovlp_removal )
      {
         editor->append(QString("Begin popping stage %1\n").arg(k + 1));
      }
      progress->setValue(ppos++); // 9, 10, 11
      qApp->processEvents();
      if (stopFlag)
      {
         return -1;
      }

      do {
         // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_bp-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
         // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-bp-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
         iter++;
#if defined(DEBUG1) || defined(DEBUG)
         printf("popping iteration %d\n", iter++);
#endif
         if ( !no_ovlp_removal ) {
            lbl_core_progress->setText(QString("Stage %1 popping iteration %2").arg(k+1).arg(iter));
         }
         qApp->processEvents();
         max_intersection_volume = -1;
         overlaps_exist = false;
         if (methods[k] & (POP_MC | POP_SC | POP_MCSC | POP_EXPOSED | POP_BURIED | POP_ALL)) {
            for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
               for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG)
                  printf("checking popping stage %d beads %d %d on chains %d %d exposed %d %d active %s %s max iv %f\n",
                         k, i, j,
                         bead_model[i].chain,
                         bead_model[j].chain,
                         bead_model[i].exposed_code,
                         bead_model[j].exposed_code,
                         bead_model[i].active ? "Y" : "N",
                         bead_model[j].active ? "Y" : "N",
                         max_intersection_volume
                         );
#endif
                  if (bead_model[i].active &&
                      bead_model[j].active &&
                      ( ((methods[k] & POP_SC) &&
                         bead_model[i].chain == 1 &&
                         bead_model[j].chain == 1) ||
                        ((methods[k] & POP_MC) &&
                         bead_model[i].chain == 0 &&
                         bead_model[j].chain == 0) ||
                        ((methods[k] & POP_MCSC)
                         // &&
                         // (bead_model[i].chain != 1 ||
                         // bead_model[j].chain != 1))
                         ) ) &&
                      ( ((methods[k] & POP_EXPOSED) &&
                         bead_model[i].exposed_code == 1 &&
                         bead_model[j].exposed_code == 1) ||
                        ((methods[k] & POP_BURIED) &&
                         (bead_model[i].exposed_code != 1 ||
                          bead_model[j].exposed_code != 1)) ||
                        (methods[k] & POP_ALL) )) {
                     intersection_volume =
                        int_vol_2sphere(
                                        bead_model[i].bead_computed_radius,
                                        bead_model[j].bead_computed_radius,
                                        sqrt(
                                             pow(bead_model[i].bead_coordinate.axis[0] -
                                                 bead_model[j].bead_coordinate.axis[0], 2) +
                                             pow(bead_model[i].bead_coordinate.axis[1] -
                                                 bead_model[j].bead_coordinate.axis[1], 2) +
                                             pow(bead_model[i].bead_coordinate.axis[2] -
                                                 bead_model[j].bead_coordinate.axis[2], 2)) );
#if defined(DEBUG)
                     printf("this overlap bead %u %u vol %f rv1 %f rv2 %f r1 %f r2 %f p1 [%f,%f,%f] p2 [%f,%f,%f]\n",
                            bead_model[i].serial,
                            bead_model[j].serial,
                            intersection_volume,
                            bead_model[i].bead_ref_volume,
                            bead_model[j].bead_ref_volume,
                            bead_model[i].bead_computed_radius,
                            bead_model[j].bead_computed_radius,
                            bead_model[i].bead_coordinate.axis[0],
                            bead_model[i].bead_coordinate.axis[1],
                            bead_model[i].bead_coordinate.axis[2],
                            bead_model[j].bead_coordinate.axis[0],
                            bead_model[j].bead_coordinate.axis[1],
                            bead_model[j].bead_coordinate.axis[2]
                            );
#endif
                     if (intersection_volume > bead_model[i].bead_ref_volume * overlap[k] ||
                         intersection_volume > bead_model[j].bead_ref_volume * overlap[k]) {
                        overlaps_exist = true;
                        if (intersection_volume > max_intersection_volume) {
#if defined(DEBUG)
                           printf("best overlap so far bead %u %u vol %f\n",
                                  bead_model[i].serial,
                                  bead_model[j].serial,
                                  intersection_volume);
#endif
                           max_intersection_volume = intersection_volume;
                           max_bead1 = i;
                           max_bead2 = j;
                        }
                     }
                  }
               }
            }

            bool back_to_zero = false;
            if (overlaps_exist) {
               beads_popped++;
               if ( !no_ovlp_removal ) {
                  lbl_core_progress->setText(QString("Stage %1 popping iteration %2 beads popped %3").arg(k+1).arg(iter).arg(beads_popped));
               }
               qApp->processEvents();
               //#define DEBUG_FUSED
#if defined(DEBUG1) || defined(DEBUG) || defined(DEBUG_FUSED)
               printf("popping beads %u %u int vol %f mw1 %f mw2 %f v1 %f v2 %f c1 [%f,%f,%f] c2 [%f,%f,%f]\n",
                      max_bead1,
                      max_bead2,
                      max_intersection_volume,
                      bead_model[max_bead1].bead_ref_mw,
                      bead_model[max_bead2].bead_ref_mw,
                      bead_model[max_bead1].bead_ref_volume,
                      bead_model[max_bead2].bead_ref_volume,
                      bead_model[max_bead1].bead_coordinate.axis[0],
                      bead_model[max_bead1].bead_coordinate.axis[1],
                      bead_model[max_bead1].bead_coordinate.axis[2],
                      bead_model[max_bead2].bead_coordinate.axis[0],
                      bead_model[max_bead2].bead_coordinate.axis[1],
                      bead_model[max_bead2].bead_coordinate.axis[2]
                      );
#endif
               if (bead_model[max_bead1].chain == 0 &&
                   bead_model[max_bead2].chain == 1) {
                  // always select the sc!
#if defined(DEBUG1) || defined(DEBUG) || defined(DEBUG_FUSED)
                  puts("swap beads");
#endif
                  int tmp = max_bead2;
                  max_bead2 = max_bead1;
                  max_bead1 = tmp;
               }
               if (bead_model[max_bead1].chain != bead_model[max_bead2].chain &&
                   k == 1) {
                  back_to_zero = true;
               }
               // bead_model[max_bead1].all_beads.push_back(&bead_model[max_bead1]); ??
               bead_model[max_bead1].all_beads.push_back(&bead_model[max_bead2]);
               for (unsigned int n = 0; n < bead_model[max_bead2].all_beads.size(); n++) {
                  bead_model[max_bead1].all_beads.push_back(bead_model[max_bead2].all_beads[n]);
               }

               bead_model[max_bead2].active = false;
               for (unsigned int m = 0; m < 3; m++) {
                  bead_model[max_bead1].bead_coordinate.axis[m] *=
                     bead_model[max_bead1].bead_ref_mw + bead_model[max_bead1].bead_ref_ionized_mw_delta;
                  bead_model[max_bead1].bead_coordinate.axis[m] +=
                     bead_model[max_bead2].bead_coordinate.axis[m] * ( bead_model[max_bead2].bead_ref_mw + bead_model[max_bead2].bead_ref_ionized_mw_delta );
                  bead_model[max_bead1].bead_coordinate.axis[m] /=
                     bead_model[max_bead1].bead_ref_mw + bead_model[max_bead1].bead_ref_ionized_mw_delta +
                     bead_model[max_bead2].bead_ref_mw + bead_model[max_bead2].bead_ref_ionized_mw_delta ;
               }
               bead_model[max_bead1].bead_ref_mw               = bead_model[max_bead1].bead_ref_mw + bead_model[max_bead2].bead_ref_mw;
               bead_model[max_bead1].bead_ref_ionized_mw_delta = bead_model[max_bead1].bead_ref_ionized_mw_delta + bead_model[max_bead2].bead_ref_ionized_mw_delta;
               bead_model[max_bead1].bead_ref_volume           = bead_model[max_bead1].bead_ref_volume + bead_model[max_bead2].bead_ref_volume;
               // - max_intersection_volume;
               bead_model[max_bead1].bead_actual_radius =
                  bead_model[max_bead1].bead_computed_radius =
                  pow(3 * bead_model[max_bead1].bead_ref_volume / (4.0*M_PI), 1.0/3);
               // if fusing with a side chain bead, make sure the fused is side-chain
               // if (bead_model[max_bead2].chain) {
               //   bead_model[max_bead1].chain = 1;
               // }
               bead_model[max_bead1].normalized_ot_is_valid = false;
               bead_model[max_bead2].normalized_ot_is_valid = false;
#if defined(DEBUG)
               printf("after popping beads %d %d int volume %f radius %f mw %f vol %f coordinate [%f,%f,%f]\n",
                      bead_model[max_bead1].serial,
                      bead_model[max_bead2].serial,
                      intersection_volume,
                      bead_model[max_bead1].bead_computed_radius,
                      bead_model[max_bead1].bead_ref_mw,
                      bead_model[max_bead2].bead_ref_volume,
                      bead_model[max_bead1].bead_coordinate.axis[0],
                      bead_model[max_bead1].bead_coordinate.axis[1],
                      bead_model[max_bead1].bead_coordinate.axis[2]
                      );
#endif
               if (back_to_zero) {
                  editor->append(QString("Beads popped %1, Go back to stage %2\n").arg(beads_popped).arg(k));
                  printf("fused sc/mc bead in stage SC/MC, back to stage SC\n");
                  k = 0;
                  ppos -= 4;
                  goto stage_loop;
               }
            }
         } // if pop method
         // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_ap-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
         // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-ap-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
      } while(overlaps_exist);
#if defined(TIMING)
      gettimeofday(&end_tv, NULL);
      printf("popping %d time %lu\n",
             k,
             1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
             start_tv.tv_usec);
      fflush(stdout);
#endif
#if defined(WRITE_EXTRA_FILES)
      write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
      write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO, &bead_model);
#endif
      printf("stage %d beads popped %d\n", k, beads_popped);
      progress->setValue(ppos++); // 12,13,14
      if ( !no_ovlp_removal && ( !bd_mode || bd_options.do_rr  ) )
      {
         editor->append(QString("Beads popped %1.\nBegin radial reduction stage %2\n").arg(beads_popped).arg(k + 1));
      }
      qApp->processEvents();
      if (stopFlag)
      {
         return -1;
      }


      // radial reduction phase
#if defined(TIMING)
      gettimeofday(&start_tv, NULL);
#endif

      if (methods[k] & RADIAL_REDUCTION) {
         BPair pair;
         vector <BPair> pairs;

         vector <bool> reduced;
         vector <bool> reduced_any; // this is for a final recomputation of the volumes
         vector <bool> last_reduced; // to prevent rescreening
         reduced.resize(bead_model.size());
         reduced_any.resize(bead_model.size());
         last_reduced.resize(bead_model.size());

         for (unsigned int i = 0; i < bead_model.size(); i++) {
            reduced_any[i] = false;
            last_reduced[i] = true;
         }

         int iter = 0;
         int count;
         float max_intersection_length;
         // bool tb[bead_model.size() * bead_model.size()];
         // printf("sizeof tb %d, bm.size^2 %d\n",
         //     sizeof(tb), bead_model.size() * bead_model.size());
#if defined(DEBUG_OVERLAP)
         overlap_check(methods[k] & RR_SC ? true : false,
                       methods[k] & RR_MCSC ? true : false,
                       methods[k] & RR_BURIED ? true : false,
                       overlap_tolerance);
#endif
         if (methods[k] & RR_HIERC) {
#if defined(DEBUG1) || defined(DEBUG)
            printf("preprocessing processing hierarchical radial reduction\n");
#endif
            lbl_core_progress->setText(QString("Stage %1 hierarchical radial reduction").arg(k+1));
            qApp->processEvents();
            max_intersection_length = 0;
            pairs.clear( );
            count = 0;
            // build list of intersecting pairs
            for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
               for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG)
                  printf("checking radial stage %d beads %d %d on chains %d %d exposed code %d %d active %s %s max il %f\n",
                         k, i, j,
                         bead_model[i].chain,
                         bead_model[j].chain,
                         bead_model[i].exposed_code,
                         bead_model[j].exposed_code,
                         bead_model[i].active ? "Y" : "N",
                         bead_model[j].active ? "Y" : "N",
                         max_intersection_length
                         );
#endif
                  if (
                      bead_model[i].active &&
                      bead_model[j].active &&
                      (methods[k] & RR_MCSC ||
                       ((methods[k] & RR_SC) &&
                        bead_model[i].chain == 1 &&
                        bead_model[j].chain == 1)) &&
                      ((methods[k] & RR_BURIED) ||
                       (bead_model[i].exposed_code == 1 ||
                        bead_model[j].exposed_code == 1)) &&
                      bead_model[i].bead_computed_radius > TOLERANCE &&
                      bead_model[j].bead_computed_radius > TOLERANCE
                      ) {

                     float separation =
                        bead_model[i].bead_computed_radius +
                        bead_model[j].bead_computed_radius -
                        sqrt(
                             pow(bead_model[i].bead_coordinate.axis[0] -
                                 bead_model[j].bead_coordinate.axis[0], 2) +
                             pow(bead_model[i].bead_coordinate.axis[1] -
                                 bead_model[j].bead_coordinate.axis[1], 2) +
                             pow(bead_model[i].bead_coordinate.axis[2] -
                                 bead_model[j].bead_coordinate.axis[2], 2));

#if defined(DEBUG)
                     printf("beads %d %d with radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] have a sep of %f\n",
                            i, j,
                            bead_model[i].bead_computed_radius,
                            bead_model[j].bead_computed_radius,
                            bead_model[i].bead_coordinate.axis[0],
                            bead_model[i].bead_coordinate.axis[1],
                            bead_model[i].bead_coordinate.axis[2],
                            bead_model[j].bead_coordinate.axis[0],
                            bead_model[j].bead_coordinate.axis[1],
                            bead_model[j].bead_coordinate.axis[2],
                            separation);
#endif
                     if (separation <= TOLERANCE) {
                        continue;
                     }

                     pair.i = i;
                     pair.j = j;
                     pair.separation = separation;
                     pair.active = true;
                     pairs.push_back(pair);
                     count++;
                  }
               }
            }
            // ok, now we have the list of pairs
            lbl_core_progress->setText(QString("Stage %1 hierarchical radial reduction").arg(k+1));
            qApp->processEvents();
            max_bead1 =
               max_bead2 = -1;
            float radius_delta;
            do {
               iter++;
#if defined(DEBUG1) || defined(DEBUG)
               printf("processing hierarchical radial reduction iteration %d\n", iter);
#endif
               lbl_core_progress->setText(QString("Stage %1 hierarch. red. it. %2 with %3 couples").arg(k+1).arg(iter).arg(pairs.size()));
               qApp->processEvents();
               max_intersection_length = 0;
               int max_pair = -1;
               count = 0;
               for (unsigned int i = 0; i < pairs.size(); i++) {
#if defined(DEBUG_OVERLAP)
                  printf("pair %d %d sep %f %s %s\n",
                         pairs[i].i, pairs[i].j, pairs[i].separation, pairs[i].active ? "active" : "not active",
                         pairs[i].i == max_bead1 ||
                         pairs[i].j == max_bead1 ||
                         pairs[i].i == max_bead2 ||
                         pairs[i].j == max_bead2 ? "needs recompute of separation" : "separation valid");
#endif
                  if (pairs[i].active) {
                     if (
                         pairs[i].i == max_bead1 ||
                         pairs[i].j == max_bead1 ||
                         pairs[i].i == max_bead2 ||
                         pairs[i].j == max_bead2
                         ) {
                        pairs[i].separation =
                           bead_model[pairs[i].i].bead_computed_radius +
                           bead_model[pairs[i].j].bead_computed_radius -
                           sqrt(
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[0] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[0], 2) +
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[1] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[1], 2) +
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[2] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[2], 2));
                        pairs[i].active = true;
                     }
                     if (pairs[i].separation > max_intersection_length) {
                        max_intersection_length = pairs[i].separation;
                        max_pair = i;
                     }
                  }
               }

               if (max_intersection_length > TOLERANCE) {
                  count++;
                  pairs[max_pair].active = false;
                  max_bead1 = pairs[max_pair].i;
                  max_bead2 = pairs[max_pair].j;
#if defined(DEBUG1) || defined(DEBUG)
                  printf("processing radial reduction hierc iteration %d pair %d processed %d\n", iter, max_pair, count);
                  printf("reducing beads %d %d\n", max_bead1, max_bead2);
#endif
                  do {
                     if (methods[k] & OUTWARD_TRANSLATION ||
                         ((bead_model[max_bead1].chain == 1 ||
                           bead_model[max_bead2].chain == 1) &&
                          methods[0] & OUTWARD_TRANSLATION)) {
                        // new 1 step ot
                        if((methods[k] & RR_MCSC &&
                            (bead_model[max_bead1].chain == 1 ||
                             bead_model[max_bead2].chain == 1))
                           ) {
                           // new 1 bead 1 OT / treat as no ot...
                           int use_bead;
                           if (bead_model[max_bead1].chain == 1) {
                              use_bead = max_bead2;
                           }
                           else 
                           {
                              use_bead = max_bead1;
                           }
                           // one bead to shrink
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta = max_intersection_length;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required, one bead to shrink, radius delta %f  cr %f %f\n",
                                  use_bead,
                                  radius_delta,
                                  bead_model[use_bead].bead_computed_radius,
                                  bead_model[use_bead].bead_computed_radius - radius_delta
                                  );
#endif
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                        else 
                        {
                           int use_bead = max_bead1;
                           if (!bead_model[use_bead].normalized_ot_is_valid) {
                              float norm = 0.0;
                              for (unsigned int l = 0; l < 3; l++) {
                                 bead_model[use_bead].normalized_ot.axis[l] =
                                    bead_model[use_bead].bead_coordinate.axis[l] -
                                    molecular_cog[l];
                                 norm +=
                                    bead_model[use_bead].normalized_ot.axis[l] *
                                    bead_model[use_bead].normalized_ot.axis[l];
                              }
                              norm = sqrt(norm);
                              if (norm) {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                 }
                                 bead_model[use_bead].normalized_ot_is_valid = true;
                              }
                              else 
                              {
                                 printf("wow! bead %d is at the molecular cog!\n", use_bead);
                              }
                           }
                           use_bead = max_bead2;
                           if (!bead_model[use_bead].normalized_ot_is_valid) {
                              float norm = 0.0;
                              for (unsigned int l = 0; l < 3; l++) {
                                 bead_model[use_bead].normalized_ot.axis[l] =
                                    bead_model[use_bead].bead_coordinate.axis[l] -
                                    molecular_cog[l];
                                 norm +=
                                    bead_model[use_bead].normalized_ot.axis[l] *
                                    bead_model[use_bead].normalized_ot.axis[l];
                              }
                              norm = sqrt(norm);
                              if (norm) {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                 }
                                 bead_model[use_bead].normalized_ot_is_valid = true;
                              }
                              else 
                              {
                                 printf("wow! bead %d is at the molecular cog!\n", use_bead);
                              }
                           }
                           // we need to handle 1 fixed case and
                           // the slight potential of one being at the molecular cog
                           reduced_any[max_bead1] = true;
                           reduced_any[max_bead2] = true;
                           outward_translate_2_spheres(
                                                       &bead_model[max_bead1].bead_computed_radius,
                                                       &bead_model[max_bead2].bead_computed_radius,
                                                       bead_model[max_bead1].bead_coordinate.axis,
                                                       bead_model[max_bead2].bead_coordinate.axis,
                                                       bead_model[max_bead1].normalized_ot.axis,
                                                       bead_model[max_bead2].normalized_ot.axis
                                                       );
                        }
                     }
                     else 
                     {
                        // no outward translation is required for either bead
                        // are we shrinking just 1 bead ... if we are dealing with buried beads, then
                        // only buried beads should be shrunk, not exposed beads
#if defined(DEBUG2)
                        printf("no outward translation is required\n");
#endif
                        if(methods[k] & RR_BURIED &&
                           bead_model[max_bead1].exposed_code == 1 &&
                           bead_model[max_bead2].exposed_code == 1) {
                           printf("what are we doing here?  buried and two exposed??\n");
                           // exit(-1);
                        }
                        if(methods[k] & RR_MCSC &&
                           !(methods[k] & RR_BURIED) &&
                           bead_model[max_bead1].chain == 1 &&
                           bead_model[max_bead2].chain == 1) {
                           printf("what are we doing here?  dealing with 2 SC's on the MCSC run??\n");
                           // exit(-1);
                        }
                        if((methods[k] & RR_BURIED &&
                            (bead_model[max_bead1].exposed_code == 1 ||
                             bead_model[max_bead2].exposed_code == 1)) ||
                           (methods[k] & RR_MCSC &&
                            !(methods[k] & RR_BURIED) &&
                            (bead_model[max_bead1].chain == 1 ||
                             bead_model[max_bead2].chain == 1))) {
                           // only one bead to shrink, since
                           // we are either buried with one of the beads exposed or
                           // on the MCSC and one of the beads is SC
                           int use_bead;
                           if (methods[k] & RR_BURIED) {
                              if (bead_model[max_bead1].exposed_code == 1) {
                                 use_bead = max_bead2;
                              }
                              else 
                              {
                                 use_bead = max_bead1;
                              }
                           }
                           else 
                           {
                              if (bead_model[max_bead1].chain == 1) {
                                 use_bead = max_bead2;
                              }
                              else 
                              {
                                 use_bead = max_bead1;
                              }
                           }
                           // one bead to shrink
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta = max_intersection_length;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required, one bead to shrink, radius delta %f  cr %f %f\n",
                                  use_bead,
                                  radius_delta,
                                  bead_model[use_bead].bead_computed_radius,
                                  bead_model[use_bead].bead_computed_radius - radius_delta
                                  );
#endif
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                        else 
                        {
                           // two beads to shrink
                           int use_bead = max_bead1;
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta =
                              // bead_model[use_bead].bead_actual_radius * rr_overlap[k];
                              max_intersection_length * bead_model[max_bead1].bead_computed_radius /
                              (bead_model[max_bead1].bead_computed_radius + bead_model[max_bead2].bead_computed_radius);
                           float radius_delta2 =
                              // bead_model[use_bead].bead_actual_radius * rr_overlap[k];
                              max_intersection_length * bead_model[max_bead2].bead_computed_radius /
                              (bead_model[max_bead1].bead_computed_radius + bead_model[max_bead2].bead_computed_radius);
#if defined(DEBUG2)
                           printf("intersection len %f recomputed %f radius delta %f r1 %f r2 %f\n",
                                  max_intersection_length,
                                  bead_model[max_bead1].bead_computed_radius +
                                  bead_model[max_bead2].bead_computed_radius -
                                  sqrt(
                                       pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                           bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                           bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                           bead_model[max_bead2].bead_coordinate.axis[2], 2)),
                                  radius_delta,
                                  bead_model[max_bead1].bead_computed_radius,
                                  bead_model[max_bead2].bead_computed_radius
                                  );
#endif
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE / 2);
                              reduced[use_bead] = false;
                           }

#if defined(DEBUG2)
                           printf("intersection len %f recomputed %f radius delta %f r1 %f r2 %f\n",
                                  max_intersection_length,
                                  bead_model[max_bead1].bead_computed_radius +
                                  bead_model[max_bead2].bead_computed_radius -
                                  sqrt(
                                       pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                           bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                           bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                           bead_model[max_bead2].bead_coordinate.axis[2], 2)),
                                  radius_delta,
                                  bead_model[max_bead1].bead_computed_radius,
                                  bead_model[max_bead2].bead_computed_radius
                                  );
#endif
                           use_bead = max_bead2;
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = radius_delta2;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                     }
#if defined(DEBUG2)
                     printf("b1r %f b2r %f current separation %f\n",
                            bead_model[max_bead1].bead_computed_radius,
                            bead_model[max_bead2].bead_computed_radius,
                            bead_model[max_bead1].bead_computed_radius +
                            bead_model[max_bead2].bead_computed_radius -
                            sqrt(
                                 pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                     bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                     bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                     bead_model[max_bead2].bead_coordinate.axis[2], 2)));
                     printf("flags %s %s %s %s\n",
                            bead_model[max_bead1].bead_computed_radius > TOLERANCE ? "Y" : "N",
                            bead_model[max_bead2].bead_computed_radius > TOLERANCE ? "Y" : "N",
                            (bead_model[max_bead1].bead_computed_radius +
                             bead_model[max_bead2].bead_computed_radius -
                             sqrt(
                                  pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                      bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                      bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                      bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE) ? "Y" : "N",
                            bead_model[max_bead1].bead_computed_radius > TOLERANCE &&
                            bead_model[max_bead2].bead_computed_radius > TOLERANCE &&
                            (bead_model[max_bead1].bead_computed_radius +
                             bead_model[max_bead2].bead_computed_radius -
                             sqrt(
                                  pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                      bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                      bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                      bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE) ? "Y" : "N");
#endif
                  } while (
                           bead_model[max_bead1].bead_computed_radius > TOLERANCE &&
                           bead_model[max_bead2].bead_computed_radius > TOLERANCE &&
                           (bead_model[max_bead1].bead_computed_radius +
                            bead_model[max_bead2].bead_computed_radius -
                            sqrt(
                                 pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                     bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                     bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                     bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE));
#if defined(DEBUG2)
                  printf("out of while 1\n");
#endif
               } // if max intersection length > TOLERANCE
            } while(count);
#if defined(DEBUG2)
            printf("out of while 2\n");
#endif
         }
         else 
         {
            // synchronous reduction
            lbl_core_progress->setText(QString("Stage %1 synchronous radial reduction").arg(k+1));
            qApp->processEvents();
#if defined(USE_THREADS)
            unsigned int threads = numThreads;
            vector < radial_reduction_thr_t* > radial_reduction_thr_threads(threads);
            for ( unsigned int j = 0; j < threads; j++ )
            {
               radial_reduction_thr_threads[j] = new radial_reduction_thr_t(j);
               radial_reduction_thr_threads[j]->start();
            }
#endif
            do {
#if defined(DEBUG_OVERLAP)
               overlap_check(methods[k] & RR_SC ? true : false,
                             methods[k] & RR_MCSC ? true : false,
                             methods[k] & RR_BURIED ? true : false,
                             overlap_tolerance);
#endif
               // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_br-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
               // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-br-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
               iter++;
#if defined(DEBUG1) || defined(DEBUG)
               printf("processing synchronous radial reduction iteration %d\n", iter);
#endif
               // lbl_core_progress->setText(QString("Stage %1 synchronous radial reduction iteration %2").arg(k+1).arg(iter));
               // qApp->processEvents();

               if(iter > 10000) {
                  printf("too many iterations\n");
                  exit(-1);
               }
               max_intersection_length = 0;
               pairs.clear( );
               count = 0;
               reduced[bead_model.size() - 1] = false;

#if defined(USE_THREADS)
               {
                  unsigned int i;
                  unsigned int j;

                  vector < BPair > my_pairs[ MAX_THREADS ];
                  for ( j = 0; j < threads; j++ )
                  {
# if defined(DEBUG_THREAD)
                     cout << "thread " << j << endl;
# endif
                     radial_reduction_thr_threads[j]->radial_reduction_thr_setup(
                                                        methods[k],
                                                        &bead_model,
                                                        &last_reduced,
                                                        &reduced,
                                                        &my_pairs[j],
                                                        threads,
                                                        overlap_tolerance
                                                        );

                  }

                  for ( j = 0; j < threads; j++ )
                  {
                     radial_reduction_thr_threads[j]->radial_reduction_thr_wait();
                  }

                  // merge results
                  for ( j = 0; j < threads; j++ )
                  {
                     for ( i = 0; i < my_pairs[j].size(); i++ ) 
                     {
                        pairs.push_back(my_pairs[j][i]);
                        
                        if (my_pairs[j][i].separation > max_intersection_length) {
                           max_intersection_length = my_pairs[j][i].separation;
                           max_bead1 = my_pairs[j][i].i;
                           max_bead2 = my_pairs[j][i].j;
                        }
                        count++;
                     }
                  }
               }
#else // !defined(USE_THREADS)
               for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
                  reduced[i] = false;
                  if (1 || last_reduced[i]) {
                     for (unsigned int j = i + 1; j < bead_model.size(); j++) {
# if defined(DEBUGX)
                        printf("checking radial stage %d beads %d %d on chains %d %d exposed code %d %d active %s %s max il %f\n",
                               k, i, j,
                               bead_model[i].chain,
                               bead_model[j].chain,
                               bead_model[i].exposed_code,
                               bead_model[j].exposed_code,
                               bead_model[i].active ? "Y" : "N",
                               bead_model[j].active ? "Y" : "N",
                               max_intersection_length
                               );
# endif
                        if ((1 || last_reduced[j]) &&
                            bead_model[i].active &&
                            bead_model[j].active &&
                            (methods[k] & RR_MCSC ||
                             ((methods[k] & RR_SC) &&
                              bead_model[i].chain == 1 &&
                              bead_model[j].chain == 1)) &&
                            ((methods[k] & RR_BURIED) ||
                             (bead_model[i].exposed_code == 1 ||
                              bead_model[j].exposed_code == 1)) &&
                            bead_model[i].bead_computed_radius > TOLERANCE &&
                            bead_model[j].bead_computed_radius > TOLERANCE
                            ) {
                           float separation =
                              bead_model[i].bead_computed_radius +
                              bead_model[j].bead_computed_radius -
                              sqrt(
                                   pow(bead_model[i].bead_coordinate.axis[0] -
                                       bead_model[j].bead_coordinate.axis[0], 2) +
                                   pow(bead_model[i].bead_coordinate.axis[1] -
                                       bead_model[j].bead_coordinate.axis[1], 2) +
                                   pow(bead_model[i].bead_coordinate.axis[2] -
                                       bead_model[j].bead_coordinate.axis[2], 2));


                           if (separation <= TOLERANCE) {
                              continue;
                           }

# if defined(DEBUG)
                           printf("beads %d %d with radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] have a sep of %f\n",
                                  i, j,
                                  bead_model[i].bead_computed_radius,
                                  bead_model[j].bead_computed_radius,
                                  bead_model[i].bead_coordinate.axis[0],
                                  bead_model[i].bead_coordinate.axis[1],
                                  bead_model[i].bead_coordinate.axis[2],
                                  bead_model[j].bead_coordinate.axis[0],
                                  bead_model[j].bead_coordinate.axis[1],
                                  bead_model[j].bead_coordinate.axis[2],
                                  separation);
# endif

                           if (separation > max_intersection_length) {
                              max_intersection_length = separation;
                              max_bead1 = i;
                              max_bead2 = j;
                           }
                           pair.i = i;
                           pair.j = j;
                           pair.separation = separation;
                           pairs.push_back(pair);
                           count++;
                        }
                     }
                  } // if last_reduced[i]
               }
#endif // !defined(USE_THERADS)

#if defined(DEBUG1) || defined(DEBUG)
               printf("processing radial reduction sync iteration %d pairs to process %d max int len %f\n", iter, count, max_intersection_length);
#endif
               if (max_intersection_length > TOLERANCE) {
#if defined(DEBUG1) || defined(DEBUG)
                  printf("processing radial reduction sync iteration %d pairs to process %d\n", iter, count);
#endif
                  lbl_core_progress->setText(QString("Stage %1 synchron. red. it. %2 with %3 couples").arg(k+1).arg(iter).arg(pairs.size()));
                  qApp->processEvents();
                  for (unsigned int i = 0; i < pairs.size(); i++) {
                     if (
                         !reduced[pairs[i].i] &&
                         (bead_model[pairs[i].i].exposed_code != 1 ||
                          methods[k] & RR_EXPOSED ||
                          methods[k] & RR_ALL) &&
                         (!(methods[k] & RR_MCSC) ||
                          bead_model[pairs[i].i].chain == 0 ||
                          (methods[k] & RR_BURIED &&
                           bead_model[pairs[i].i].exposed_code != 1))
                         ) {
                        int use_bead = pairs[i].i;
                        /*      if ( !(methods[k] & RR_MCSC) ||
                                bead_model[use_bead].exposed_code != 1 ||
                                bead_model[use_bead].chain == 0 ||
                                (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1) ) */
                        if(1) {
#if defined(DEBUG)
                           printf("reducing beads %d\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = bead_model[use_bead].bead_computed_radius * rr_overlap[k];
                           if (radius_delta < TOLERANCE) {
                              radius_delta = (float)TOLERANCE;
                           }
                           if (methods[k] & OUTWARD_TRANSLATION ||
                               (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1 &&
                                methods[0] & OUTWARD_TRANSLATION) ) {
#if defined(DEBUG)
                              printf("outward translation from [%f,%f,%f] to ",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                              if (!bead_model[use_bead].normalized_ot_is_valid) {
                                 float norm = 0.0;
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] =
                                       bead_model[use_bead].bead_coordinate.axis[l] -
                                       molecular_cog[l];
                                    norm +=
                                       bead_model[use_bead].normalized_ot.axis[l] *
                                       bead_model[use_bead].normalized_ot.axis[l];
                                 }
                                 norm = sqrt(norm);
                                 if (norm) {
                                    for (unsigned int l = 0; l < 3; l++) {
                                       bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                       bead_model[use_bead].bead_coordinate.axis[l] +=
                                          radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                    }
                                    bead_model[use_bead].normalized_ot_is_valid = true;
                                 }
                                 else 
                                 {
                                    printf("wow! bead %d is at the molecular cog!\n", use_bead);
                                 }
                              }
                              else 
                              {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].bead_coordinate.axis[l] +=
                                       radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                 }
                              }
#if defined(DEBUG)
                              printf(" [%f,%f,%f]\n",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)TOLERANCE * 0.9999;
                              reduced[use_bead] = false;
                           }
                        }
                     }
                     if (
                         !reduced[pairs[i].j] &&
                         (bead_model[pairs[i].j].exposed_code != 1 ||
                          methods[k] & RR_EXPOSED ||
                          methods[k] & RR_ALL) &&
                         (!(methods[k] & RR_MCSC) ||
                          bead_model[pairs[i].j].chain == 0 ||
                          (methods[k] & RR_BURIED &&
                           bead_model[pairs[i].j].exposed_code != 1))
                         ) {
                        int use_bead = pairs[i].j;
                        /* if ( !(methods[k] & RR_MCSC) ||
                           bead_model[use_bead].chain == 0 ||
                           bead_model[use_bead].exposed_code != 1 ||
                           (bead_model[pairs[i].i].chain == 1 &&
                           bead_model[pairs[i].j].chain == 1) ) */
                        {
#if defined(DEBUG)
                           printf("reducing beads %d\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = bead_model[use_bead].bead_computed_radius * rr_overlap[k];
                           if (radius_delta < TOLERANCE) {
                              radius_delta = (float)TOLERANCE;
                           }
                           if (methods[k] & OUTWARD_TRANSLATION ||
                               (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1 &&
                                methods[0] & OUTWARD_TRANSLATION) ) {
#if defined(DEBUG)
                              printf("outward translation from [%f,%f,%f] to ",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                              if (!bead_model[use_bead].normalized_ot_is_valid) {
                                 float norm = 0.0;
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] =
                                       bead_model[use_bead].bead_coordinate.axis[l] -
                                       molecular_cog[l];
                                    norm +=
                                       bead_model[use_bead].normalized_ot.axis[l] *
                                       bead_model[use_bead].normalized_ot.axis[l];
                                 }
                                 norm = sqrt(norm);
                                 if (norm) {
                                    for (unsigned int l = 0; l < 3; l++) {
                                       bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                       bead_model[use_bead].bead_coordinate.axis[l] +=
                                          radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                    }
                                    bead_model[use_bead].normalized_ot_is_valid = true;
                                 }
                                 else 
                                 {
                                    printf("wow! bead %d is at the molecular cog!\n", use_bead);
                                 }
                              }
                              else 
                              {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].bead_coordinate.axis[l] +=
                                       radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                 }
                              }
#if defined(DEBUG)
                              printf(" [%f,%f,%f]\n",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)TOLERANCE * 0.9999;
                              reduced[use_bead] = false;
                           }
                        }
                     }
                  }
               }
               last_reduced = reduced;
               qApp->processEvents();
               if ( stopFlag )
               {
#if defined(USE_THREADS)
                  {
                     unsigned int j;
                     // destroy
                     
                     for ( j = 0; j < threads; j++ )
                     {
                        radial_reduction_thr_threads[j]->radial_reduction_thr_shutdown();
                     }
                     
                     for ( j = 0; j < threads; j++ )
                     {
                        radial_reduction_thr_threads[j]->wait();
                     }
                     
                     for ( j = 0; j < threads; j++ )
                     {
                        delete radial_reduction_thr_threads[j];
                     }
                  }
#endif
                  return -1;
               }
               // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_ar-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
               // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-ar-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
            } while(count);
#if defined(USE_THREADS)
            {
               unsigned int j;
               // destroy
               
               for ( j = 0; j < threads; j++ )
               {
                  radial_reduction_thr_threads[j]->radial_reduction_thr_shutdown();
               }
               
               for ( j = 0; j < threads; j++ )
               {
                  radial_reduction_thr_threads[j]->wait();
               }
               
               for ( j = 0; j < threads; j++ )
               {
                  delete radial_reduction_thr_threads[j];
               }
            }
#endif
         }

#if defined(TIMING)
         gettimeofday(&end_tv, NULL);
         printf("radial reduction %d time %lu\n",
                k,
                1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
                start_tv.tv_usec);
         fflush(stdout);
#endif
         progress->setValue(ppos++); // 15,16,17
         qApp->processEvents();
         if (stopFlag)
         {
            return -1;
         }

         // recompute volumes
         for (unsigned int i = 0; i < bead_model.size(); i++) {
            if (reduced_any[i]) {
#if defined(DEBUG1) || defined(DEBUG)
               printf("recomputing volume bead %d\n", i);
#endif
               bead_model[i].bead_ref_volume =
                  (4.0*M_PI/3.0) * pow(bead_model[i].bead_computed_radius, 3);
            }
         }
      }
#if defined(DEBUG_OVERLAP)
      if(overlap_check(methods[k] & RR_SC ? true : false,
                       methods[k] & RR_MCSC ? true : false,
                       methods[k] & RR_BURIED ? true : false,
                       overlap_tolerance)) {
         printf("internal over lap error... exiting!");
         exit(-1);
      }
#endif
#if defined(WRITE_EXTRA_FILES)
      write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
      write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO, &bead_model);
#endif
   } // methods
#if defined(WRITE_EXTRA_FILES)
   write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO + ".tsv", &bead_model);
   write_bead_spt(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO, &bead_model);
#endif
   //  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg( model_name( current_model ) ) +
   //       QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
   //       DOTSOMO, &bead_model);
   QString filename = 
      project + 
      QString( "_%1" ).arg( current_model + 1 );

   le_bead_model_file->setText( filename );

   write_bead_model(somo_dir + SLASH + filename  +
                    QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") + DOTSOMO
                    , &bead_model);
   if ( !no_ovlp_removal && ( !bd_mode  ||  bd_options.do_rr  ) )
   {
      editor->append("Finished with popping and radial reduction\n");
   }
   progress->setValue(mppos - (asa.recheck_beads ? 1 : 0));
   if ( bd_mode || no_ovlp_removal )
   {
      progress->setValue( 1 ); progress->setMaximum( 1 );
      lbl_core_progress->setText("");
   }
      
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }
   return 0;
}

void US_Hydrodyn::bead_check( bool use_threshold, bool message_type, bool vdw, bool only_asa )
{
   // recheck beads here

   // printf("bead recheck use threshold%s\n", use_threshold ? "" : " percent");
   active_atoms.clear( );
   for(unsigned int i = 0; i < bead_model.size(); i++) {
      active_atoms.push_back(&bead_model[i]);
   }

   QString error_string = "";
   double save_prr = asa.probe_recheck_radius;
   if ( vdw ) {
      asa.probe_recheck_radius = asa.vdw_grpy_probe_radius;
   }

   int retval;
   if ( asa.method == 1 ){
      // rolling sphere
      retval = us_hydrodyn_asab1_main(active_atoms,
                                          &asa,
                                          &results,
                                          true,
                                          progress,
                                          editor,
                                          this
                                          );


   } else {
      // surfracer
      editor_msg( "black", "Computing ASA via SurfRacer\n" );
      retval = surfracer_main(asa.probe_radius,
                              active_atoms,
                              true,
                              progress,
                              editor
                              );
   }

   asa.probe_recheck_radius = save_prr;

   if (stopFlag)
   {
      return;
   }
   if ( retval ) {
      switch ( retval ) {
      case US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC:
         {
            printError("US_HYDRODYN_ASAB1 on bead recheck encountered a memory allocation error");
            fprintf(stderr, "bead recheck: memory alloc error\n");
            return;
            break;
         }
      default:
         {
            // unknown error
            printError("US_HYDRODYN_ASAB1 encountered an unknown error");
            fprintf(stderr, "bead recheck: unknown error %d\n", retval);
            return;
            break;
         }
      }
   }

   int b2e = 0;
   int e2b = 0;

   //  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg( model_name( current_model ) ) + "_pre_recheck" + DOTSOMO, &bead_model);
   //  write_bead_model(somo_dir + SLASH + project + QString("_%1").arg( model_name( current_model ) ) + "_pre_recheck" + DOTSOMO, &bead_model );

   if ( only_asa ) {
      double total_asa = 0e0;
      double used_asa  = 0e0;
      double lconv = pow(10.0,9 + hydro.unit);
      double lconv2 = lconv * lconv;

      for ( int i = 0; i < (int) bead_model.size(); ++i ) {
         total_asa += bead_model[i].bead_recheck_asa * lconv2;
         if ( get_color( &bead_model[i] ) != 6 ) {
            used_asa += bead_model[i].bead_recheck_asa * lconv2;
         }
      }
      editor_msg( "dark blue",
                  QString().sprintf(
                                    "Recomputed ASA of beads:\n"
                                    "Total ASA: %f\n"
                                    "Used ASA: %f\n"
                                    ,total_asa
                                    ,used_asa
                                    )
                  );
      return;
   }
                                    

   if ( vdw ) {
      for (unsigned int i = 0; i < bead_model.size(); i++) {
         QString msg = "";
         float surface_area =
            (asa.vdw_grpy_probe_radius + bead_model[i].bead_computed_radius) *
            (asa.vdw_grpy_probe_radius + bead_model[i].bead_computed_radius) * 4 * M_PI;
         if( bead_model[i].bead_recheck_asa < (asa.vdw_grpy_threshold_percent / 100.0) * surface_area ) {
            // now buried
            if(bead_model[i].exposed_code == 1) {
               // now buried
               // was exposed
               msg = "exposed->buried";
               e2b++;
               bead_model[i].exposed_code = 6;
               bead_model[i].bead_color = 6;
            }
         }
      }
      if ( e2b > 0 ) {
         editor_msg( "black", QString("%1 beads are buried\n").arg(e2b) );
      }
      if ( b2e > 0 ) {
         editor_msg( "black", QString("%1 buried beads became exposed\n").arg(b2e) );
      }
      
      return;
   } else {

      for (unsigned int i = 0; i < bead_model.size(); i++) {
         float surface_area =
            (asa.probe_radius + bead_model[i].bead_computed_radius) *
            (asa.probe_radius + bead_model[i].bead_computed_radius) * 4 * M_PI;
         QString msg = "";
         if( use_threshold ?
             ( bead_model[i].bead_recheck_asa > asa.threshold )        
             :
             ( bead_model[i].bead_recheck_asa > (asa.threshold_percent / 100.0) * surface_area )
             )
         {
            // now exposed
            if(bead_model[i].exposed_code != 1) {
               // was buried
               msg = "buried->exposed";
               b2e++;
               bead_model[i].exposed_code = 1;
               bead_model[i].bead_color = 8;
            }
         }
#if defined(EXPOSED_TO_BURIED)
         else {
            // now buried
            if(bead_model[i].exposed_code == 1) {
               // was exposed
               msg = "exposed->buried";
               e2b++;
               bead_model[i].exposed_code = 6;
               bead_model[i].bead_color = 6;
            }
         }
#endif

#if defined(DEBUG_BEAD_CHECK)
         printf("bead %d %.2f %.2f %.2f %s %s bead mw %.2f bead ref mw %.2f\n",
                i,
                bead_model[i].bead_computed_radius,
                surface_area,
                bead_model[i].bead_recheck_asa,
                (
                 use_threshold ?
                 ( bead_model[i].bead_recheck_asa > asa.threshold )        
                 :
                 ( bead_model[i].bead_recheck_asa > (asa.threshold_percent / 100.0) * surface_area )
                 ) ?
                "exposed" : "buried",
                msg.toLatin1().data(),
                bead_model[i].bead_mw,
                bead_model[i].bead_ref_mw
                );
#endif
      }
   }
   //  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg( model_name( current_model ) ) +
   //         QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
   //       DOTSOMO, &bead_model);

   QString filename = 
      project + 
      QString( "_%1" ).arg( model_name( current_model ) );

   le_bead_model_file->setText( filename );

   write_bead_model(somo_dir + SLASH + project + QString("_%1").arg( model_name( current_model ) ) +
                    QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
                    DOTSOMO, &bead_model);
#if defined(EXPOSED_TO_BURIED)
   editor->append(QString("%1 exposed beads became buried\n").arg(e2b));
#endif
   if ( message_type ) 
   {
      editor->append(QString(us_tr("%1 beads are exposed\n")).arg(b2e));
   } 
   else
   {
      editor->append(QString(us_tr("%1 previously buried beads are exposed by rechecking\n")).arg(b2e));
   }
}

double US_Hydrodyn::total_volume_of_bead_model( vector < PDB_atom > &bead_model )
{
   double tot_vol = 0e0;

   double pi43 = M_PI * 4e0 / 3e0;

   for ( unsigned int i = 0; i < bead_model.size(); i++ )
   {
      if ( bead_model[ i ].active ) 
      {
         tot_vol += (double) bead_model[ i ].bead_computed_radius * bead_model[ i ].bead_computed_radius * bead_model[ i ].bead_computed_radius;
      }
   }
   return tot_vol * pi43;
}

unsigned int US_Hydrodyn::number_of_active_beads( vector < PDB_atom > &bead_model )
{
   unsigned int number_of_active_beads = 0;

   for ( unsigned int i = 0; i < bead_model.size(); i++ )
   {
      if ( bead_model[ i ].active ) 
      {
         number_of_active_beads++;
      }
   }
   return number_of_active_beads;
}

bool US_Hydrodyn::radii_all_equal( vector < PDB_atom > &bead_model )
{
   bool  first_found = false;
   float first_radius = 0.0f;

   for ( unsigned int i = 0; i < bead_model.size(); i++ )
   {
      if ( bead_model[ i ].active ) 
      {
         if ( !first_found )
         {
            first_radius = bead_model[ i ].bead_computed_radius;
            first_found  = true;
         } else {
            if ( first_radius != bead_model[ i ].bead_computed_radius )
            {
               return false;
            }
         }
      }
   }
   return true;
}
