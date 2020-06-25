// us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o
// (this) us_hydrodyn_info.cpp contains code to report structures for debugging
// us_hydrodyn_util.cpp contains other various code, such as disulfide code

#include "../include/us_hydrodyn.h"

/********************************************************************************************
---------------
structure notes
---------------

::read_pdb() loads a pdb and sets up
model_vector
model_vector_as_loaded
(not guaranteed exhaustive ;)

*********************************************************************************************/

#define TSO QTextStream(stdout)
#define LBD  "--------------------------------------------------------------------------------\n"
#define LBE  "================================================================================\n"

void US_Hydrodyn::info_model_vector( const QString & msg, const vector <struct PDB_model> & models, const set < QString > only_residues ) {
   // print out model vector info

   int model_count = (int) models.size();

   TSO
      << "US_Hydrodyn::info_model_vector()" << endl
      << msg << endl
      << "models.size()             : " << model_count << endl
      ;

   for ( int i = 0; i < model_count; ++i ) {
      // struct PDB_model
      int chains   = (int) models[ i ].molecule.size();
      int residues = (int) models[ i ].residue .size();

      TSO
         << LBE
         << "models                   : " << i << endl
         << "models chains            : " << chains << endl
         << "models residues          : " << residues << endl
         ;

      for ( int j = 0; j < chains; ++j ) {
         // struct PDB_chain
         int atoms = (int) models[ i ].molecule[ j ].atom.size();
         TSO
            << LBD
            << " models chain             : " << j << endl
            << " models chain atoms       : " << atoms << endl
            ;
         for ( int k = 0; k < atoms; ++k ) {
            // struct PDB_atom
            if ( !only_residues.size() || only_residues.count( models[ i ].molecule[ j ].atom[ k ].resName ) ) {
               TSO
                  << LBD
                  << "  models chain atom                    : " << k << endl
                  << "  models chain atom name               : " << models[ i ].molecule[ j ].atom[ k ].name << endl
                  << "  models chain atom resName            : " << models[ i ].molecule[ j ].atom[ k ].resName << endl
                  << "  models chain atom resSeq             : " << models[ i ].molecule[ j ].atom[ k ].resSeq << endl
                  << "  models chain atom p_residue          : " << ( models[ i ].molecule[ j ].atom[ k ].p_residue ? "set" : "not set" ) << endl
                  << "  models chain atom model_residue_pos  : " << models[ i ].molecule[ j ].atom[ k ].model_residue_pos << endl
                  ;
            }
         }
      }
      for ( int j = 0; j < residues; ++j ) {
         // struct residue
         if ( !only_residues.size() || only_residues.count( models[ i ].residue[ j ].name ) ) {
            int atoms = (int) models[ i ].residue[ j ].r_atom.size();
            int beads = (int) models[ i ].residue[ j ].r_bead.size();
            TSO
               << LBD
               << " models residue               : " << j << endl
               << " models residue name          : " << models[ i ].residue[ j ].name << endl
               << " models residue unique_name   : " << models[ i ].residue[ j ].unique_name << endl
               << " models residue r_atom.size() : " << atoms << endl
               << " models residue r_bead.size() : " << beads << endl
               ;
            for ( int k = 0; k < atoms; ++k ) {
               TSO
                  << LBD
                  << "  models residue r_atom                    : " << k << endl
                  << "  models residue r_atom name               : " << models[ i ].residue[ j ].r_atom[ k ].name << endl
                  << "  models residue r_atom bead assignment    : " << models[ i ].residue[ j ].r_atom[ k ].bead_assignment << endl
                  << "  models residue r_atom serial number      : " << models[ i ].residue[ j ].r_atom[ k ].serial_number << endl
                  << "  models residue r_atom hybrid name        : " << models[ i ].residue[ j ].r_atom[ k ].hybrid.name << endl
                  ;
            }
            for ( int k = 0; k < beads; ++k ) {
               TSO
                  << LBD
                  << "  models residue r_bead                    : " << k << endl
                  << "  models residue r_bead atom_hydration     : " << models[ i ].residue[ j ].r_bead[ k ].atom_hydration << endl
                  ;
            }
         }
      }
   }
   TSO
      << LBE
      ;
}               

#undef TSO
#undef LBE
#undef LDB
