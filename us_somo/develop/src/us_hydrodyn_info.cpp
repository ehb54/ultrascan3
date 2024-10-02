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

void US_Hydrodyn::info_bead_models_mw( const QString & msg, const vector < vector < PDB_atom > > & b_models ) {
   int models = (int) bead_models.size();
   TSO
      << LBE
      << "US_Hydrodyn::info_bead_models_mw()" << Qt::endl
      << msg << Qt::endl
      << "bead_models.size()             : " << models << Qt::endl
      ;
   for ( int i = 0; i < models; ++i ) {
      info_bead_models_mw( msg + QString( "model %1" ).arg( i ), b_models[ i ] );
   }
}
   
void US_Hydrodyn::info_bead_models_mw( const QString & msg, const vector < PDB_atom > & b_model ) {
   int beads = (int) b_model.size();
   TSO
      << LBE
      << "US_Hydrodyn::info_bead_models_mw()" << Qt::endl
      << msg << Qt::endl
      << "bead_model.size() == beads          : " << beads << Qt::endl
      ;
   double sum_mw          = 0e0;
   double sum_bead_mw     = 0e0;
   double sum_bead_ref_mw = 0e0;
   for ( int j = 0; j < beads; ++j ) {
      if ( b_model[ j ].active ) {
         sum_mw          += b_model[ j ].mw          + b_model[ j ].ionized_mw_delta;
         if ( b_model[ j ].is_bead ) {
            sum_bead_mw     += b_model[ j ].bead_mw     + b_model[ j ].bead_ionized_mw_delta;
            sum_bead_ref_mw += b_model[ j ].bead_ref_mw + b_model[ j ].bead_ref_ionized_mw_delta;
         }
      }
      TSO
         << LBD
         << "  bead model bead                           : " << j << Qt::endl
         << "  bead model bead name                      : " << b_model[ j ].name << Qt::endl
         << "  bead model bead resName                   : " << b_model[ j ].resName << Qt::endl
         << "  bead model bead resSeq                    : " << b_model[ j ].resSeq << Qt::endl
         << "  bead model bead mw                        : " << b_model[ j ].mw << Qt::endl
         << "  bead model bead ionized mw delta          : " << b_model[ j ].ionized_mw_delta << Qt::endl
         << "  bead model bead bead_mw                   : " << b_model[ j ].bead_mw << Qt::endl
         << "  bead model bead bead_ionized_mw_delta     : " << b_model[ j ].bead_ionized_mw_delta << Qt::endl
         << "  bead model bead bead_ref_mw               : " << b_model[ j ].bead_ref_mw << Qt::endl
         << "  bead model bead bead_ref_ionized_mw_delta : " << b_model[ j ].bead_ref_ionized_mw_delta << Qt::endl
         << "  bead model bead p_residue                 : " << ( b_model[ j ].p_residue ?
                                                                  b_model[ j ].p_residue->name : QString( "not set" ) ) << Qt::endl
         << "  bead model bead p_atom                    : " << ( b_model[ j ].p_atom ?
                                                                  b_model[ j ].p_atom->name : QString( "not set" ) ) << Qt::endl
         << "  bead model bead model_residue_pos         : " << b_model[ j ].model_residue_pos << Qt::endl
         << "  bead model bead bead_assignment           : " << b_model[ j ].bead_assignment << Qt::endl
         ;
   }

   for ( int j = 0; j < beads; ++j ) {
      QTextStream( stdout ) << "bead " << j << " mw " << b_model[j].bead_mw + b_model[j].bead_ionized_mw_delta << Qt::endl;
   }

   TSO
      << LBD
      << " active sum_mw                  : " << sum_mw << Qt::endl
      << " active bead sum_bead_mw        : " << sum_bead_mw << Qt::endl
      << " active bead sum_bead_ref_mw    : " << sum_bead_ref_mw << Qt::endl
      ;
}

void US_Hydrodyn::info_model_residues( const QString & msg, struct PDB_model & model ) {
   TSO
      << "US_Hydrodyn::info_model_residues()" << Qt::endl
      << msg << Qt::endl
      //      << "models.size()             : " << model_count << Qt::endl
      ;
   int chains   = (int) model.molecule.size();
   int residues = (int) model.residue .size();

   TSO
      << LBE
      << "model chains            : " << chains << Qt::endl
      << "model residues          : " << residues << Qt::endl
      ;

   if ( 0 ) {
      for ( int j = 0; j < chains; ++j ) {
         // struct PDB_chain
         int atoms = (int) model.molecule[ j ].atom.size();

         TSO
            << LBD
            << " model chain             : " << j << Qt::endl
            << " model chain atoms       : " << atoms << Qt::endl
            ;

         for ( int k = 0; k < atoms; ++k ) {
            // struct PDB_atom
            TSO
               << LBD
               << "  model chain atom                    : " << k << Qt::endl
               << "  model chain atom name               : " << model.molecule[ j ].atom[ k ].name << Qt::endl
               << "  model chain atom resName            : " << model.molecule[ j ].atom[ k ].resName << Qt::endl
               << "  model chain atom resSeq             : " << model.molecule[ j ].atom[ k ].resSeq << Qt::endl
               << "  model chain atom resSeq             : " << model.molecule[ j ].atom[ k ].resSeq << Qt::endl
               << "  model chain atom p_residue          : " << ( model.molecule[ j ].atom[ k ].p_residue ?
                                                                  model.molecule[ j ].atom[ k ].p_residue->name : QString( "not set" ) ) << Qt::endl
               << "  model chain atom p_atom             : " << ( model.molecule[ j ].atom[ k ].p_atom ?
                                                                  model.molecule[ j ].atom[ k ].p_atom->name : QString( "not set" ) ) << Qt::endl
               << "  model chain atom model_residue_pos  : " << model.molecule[ j ].atom[ k ].model_residue_pos << Qt::endl
               << "  model chain atom bead_assignment    : " << model.molecule[ j ].atom[ k ].bead_assignment << Qt::endl
               ;
         }
      }
   }
   // spec residue info
   map < QString, QString > spec_residue = { { "N1", "N1" },
                                             { "N1-", "N1-" },
                                             { "OXT", "OXT" } };
   map < QString, double > spec_mw;
   map < QString, double > spec_psv;

   for ( auto it = spec_residue.begin();
         it != spec_residue.end();
         ++it ) {
      TSO
         << LBD
         << " spec residue : " << it->first << Qt::endl;
      if ( multi_residue_map.count( it->first ) ) {
         for ( int j = 0; j < (int) multi_residue_map[ it->first ].size(); ++j ) {
            struct residue residue_entry = residue_list[ multi_residue_map[ it->first ][ j ] ];
            map < QString, struct atom * > res_atom_map = residue_atom_map( residue_entry );
            int atoms = (int) residue_entry.r_atom.size();
            int beads = (int) residue_entry.r_bead.size();
            TSO
               << LBD
               << " residue list position            : " << j << Qt::endl
               << " residue list name                : " << residue_entry.name << Qt::endl
               << " residue list unique_name         : " << residue_entry.unique_name << Qt::endl
               << " residue list r_atom.size()       : " << atoms << Qt::endl
               << " residue list r_bead.size()       : " << beads << Qt::endl
               << " residue list vbar                : " << residue_entry.vbar << Qt::endl
               << " residue list vbars               : " << US_Vector::qs_vector( residue_entry.vbars ) << Qt::endl
               << " residue list pKas                : " << US_Vector::qs_vector( residue_entry.pKas ) << Qt::endl
               << " residue list vbar_at_pH          : " << residue_entry.vbar_at_pH << Qt::endl
               << " residue list mw                  : " << residue_entry.mw << Qt::endl
               << " residue list ionized_mw_delta    : " << residue_entry.ionized_mw_delta << Qt::endl
               ;
            
            if ( res_atom_map.count( it->second ) ) {
               TSO
                  << "  residue list atom lookup name             : " << it->second << Qt::endl
                  << "  residue list atom name                    : " << res_atom_map[ it->second ]->name << Qt::endl
                  << "  residue list atom hybrid mw               : " << res_atom_map[ it->second ]->hybrid.mw << Qt::endl
                  << "  residue list atom hybrid ionized mw delta : " << res_atom_map[ it->second ]->hybrid.ionized_mw_delta << Qt::endl
                  ;
               spec_mw[ it->first ] =
                     res_atom_map[ it->second ]->hybrid.mw +
                     res_atom_map[ it->second ]->hybrid.ionized_mw_delta
                     ;
               spec_psv[ it->first ] =
                     residue_entry.vbar_at_pH
                     ;
            } else {
               TSO
                  << "  residue list atom lookup name missing: " << it->second << Qt::endl
                  ;
            }
         }
      } else {
         TSO
            <<  " multi residue map is missing this residue" << Qt::endl
            ;
      }
   }

   map < QString, QString > hybrid_name_to_N = { { "N3H0", "N1-" },
                                                 { "N3H1", "N1" } };

   map < QString, double > delta_mw;
   map < QString, double > delta_mv;

   {
      TSO
         << LBD
         << "Aggregate special residues in chains"
         << Qt::endl
         ;

      map < QString, int    > delta_counts;
      
      for ( int j = 0; j < chains; ++j ) {
         int atoms = (int) model.molecule[ j ].atom.size();
         if ( atoms ) {
            {
               map < QString, struct atom * > first_atom_map = first_residue_atom_map( model.molecule[ j ] );
               if ( first_atom_map.count( "N" ) ) {
                  if ( hybrid_name_to_N.count( first_atom_map[ "N" ]->hybrid.name ) ) {
                     QString use_name = hybrid_name_to_N[ first_atom_map[ "N" ]->hybrid.name ];
                     ++delta_counts[ use_name ];

                     double this_mw =
                        spec_mw[ use_name ] -
                        first_atom_map[ "N" ]->hybrid.mw - first_atom_map[ "N" ]->hybrid.ionized_mw_delta;
                     double this_mv =
                        this_mw * spec_psv[ use_name ];
                     delta_mw[ hybrid_name_to_N[ first_atom_map[ "N" ]->hybrid.name ] ] += this_mw;
                     delta_mv[ hybrid_name_to_N[ first_atom_map[ "N" ]->hybrid.name ] ] += this_mv;

                     TSO
                        << " model chain first residue N hybrid name       : " << first_atom_map[ "N" ]->hybrid.name << Qt::endl
                        << " model chain first residue N mw                : " << first_atom_map[ "N" ]->hybrid.mw << Qt::endl
                        << " model chain first residue N ionized mw delta  : " << first_atom_map[ "N" ]->hybrid.ionized_mw_delta << Qt::endl
                        << " model chain first residue N computed delta mw : " << this_mw  << Qt::endl
                        << " model chain first residue N computed delta mv : " << this_mv  << Qt::endl
                        ;
                  } else {
                     TSO << " WARNING: first N in chain has unexpected hybridization " << first_atom_map[ "N" ]->hybrid.name  << Qt::endl;
                  }
               }
            }
            {
               map < QString, struct atom * > last_atom_map  = last_residue_atom_map( model.molecule[ j ] );
               QString use_name = "OXT";
               if ( last_atom_map.count( use_name ) ) {
                  ++delta_counts[ use_name ];

                  double this_mw = spec_mw[ use_name ];
                  double this_mv = this_mw * spec_psv[ use_name ];
                  delta_mw[ use_name ] += this_mw;
                  delta_mv[ use_name ] += this_mv;
               }
            }
         }
      }

      TSO << LBD;
      
      for ( auto it = delta_mw.begin();
            it != delta_mw.end();
            ++it ) {
         TSO
            << " model molecule " << it->first << "'s count     : " << delta_counts[ it->first ] << Qt::endl
            << " model molecule " << it->first << "'s delta mw  : " << delta_mw[ it->first ] << Qt::endl
            << " model molecule " << it->first << "'s delta mv  : " << delta_mv[ it->first ] << Qt::endl
         ;
      }
   }
      
   double mw = 0;
   double mv = 0;

   for ( int j = 0; j < residues; ++j ) {
      // struct residue
      int atoms = (int) model.residue[ j ].r_atom.size();
      int beads = (int) model.residue[ j ].r_bead.size();
      double this_mw = model.residue[ j ].mw + model.residue[ j ].ionized_mw_delta;
      double this_mv = model.residue[ j ].vbar_at_pH * this_mw;
      TSO
         << LBD
         << " model residue                     : " << j << Qt::endl
         << " model residue name                : " << model.residue[ j ].name << Qt::endl
         << " model residue unique_name         : " << model.residue[ j ].unique_name << Qt::endl
         << " model residue r_atom.size()       : " << atoms << Qt::endl
         << " model residue r_bead.size()       : " << beads << Qt::endl
         << " model residue vbar                : " << model.residue[ j ].vbar << Qt::endl
         << " model residue vbars               : " << US_Vector::qs_vector( model.residue[ j ].vbars ) << Qt::endl
         << " model residue pKas                : " << US_Vector::qs_vector( model.residue[ j ].pKas ) << Qt::endl
         << " model residue vbar_at_pH          : " << model.residue[ j ].vbar_at_pH << Qt::endl
         << " model residue mw                  : " << model.residue[ j ].mw << Qt::endl
         << " model residue ionized_mw_delta    : " << model.residue[ j ].ionized_mw_delta << Qt::endl
         << " model residue total mw            : " << this_mw << Qt::endl
         << " model residue mv                  : " << this_mv << Qt::endl
         ;
      mw += this_mw;
      mv += this_mv;
   }
   {
      double covolume = gparams.count( "covolume" ) ? gparams[ "covolume" ].toDouble() : 0e0;
      double total_mw = mw;
      double total_mv = mv;
      
      TSO
         << LBD
         << "vbar calculation" << Qt::endl
         << "mw residues        : " << mw << Qt::endl
         << "mv residues        : " << mv << Qt::endl
         ;

      for ( auto it = delta_mw.begin();
            it != delta_mw.end();
            ++it ) {
         TSO
            <<  "mw " << it->first << "'s            : " << it->second << Qt::endl
            <<  "mv " << it->first << "'s            : " << delta_mv[ it->first ] << Qt::endl
            ;
         total_mw += it->second;
         total_mv += delta_mv[ it->first ];
      }
      
      TSO
         << "total mw            : " << total_mw << Qt::endl
         << "total mv            : " << total_mv << Qt::endl
         << "covolume            : " << covolume << Qt::endl
         << "total mv + covolume : " << ( total_mv + covolume ) << Qt::endl
         << "vbar                : " << (( total_mv + covolume ) / total_mw ) << Qt::endl
         ;
   }

}

void US_Hydrodyn::info_model_vector( const QString & msg, const vector <struct PDB_model> & models, const set < QString > only_residues ) {
   // print out model vector info

   int model_count = (int) models.size();

   TSO
      << "US_Hydrodyn::info_model_vector()" << Qt::endl
      << msg << Qt::endl
      << "models.size()             : " << model_count << Qt::endl
      ;

   for ( int i = 0; i < model_count; ++i ) {
      // struct PDB_model
      int chains   = (int) models[ i ].molecule.size();
      int residues = (int) models[ i ].residue .size();

      TSO
         << LBE
         << "models                   : " << i << Qt::endl
         << "models chains            : " << chains << Qt::endl
         << "models residues          : " << residues << Qt::endl
         ;

      for ( int j = 0; j < chains; ++j ) {
         // struct PDB_chain
         int atoms = (int) models[ i ].molecule[ j ].atom.size();
         TSO
            << LBD
            << " models chain             : " << j << Qt::endl
            << " models chain atoms       : " << atoms << Qt::endl
            ;
         for ( int k = 0; k < atoms; ++k ) {
            // struct PDB_atom
            if ( !only_residues.size() || only_residues.count( models[ i ].molecule[ j ].atom[ k ].resName ) ) {
               TSO
                  << LBD
                  << "  models chain atom                    : " << k << Qt::endl
                  << "  models chain atom name               : " << models[ i ].molecule[ j ].atom[ k ].name << Qt::endl
                  << "  models chain atom resName            : " << models[ i ].molecule[ j ].atom[ k ].resName << Qt::endl
                  << "  models chain atom resSeq             : " << models[ i ].molecule[ j ].atom[ k ].resSeq << Qt::endl
                  << "  models chain atom p_residue          : " << ( models[ i ].molecule[ j ].atom[ k ].p_residue ?
                                                                      models[ i ].molecule[ j ].atom[ k ].p_residue->name : QString( "not set" ) ) << Qt::endl
                  << "  models chain atom p_atom             : " << ( models[ i ].molecule[ j ].atom[ k ].p_atom ?
                                                                      models[ i ].molecule[ j ].atom[ k ].p_atom->name : QString( "not set" ) ) << Qt::endl
                  << "  models chain atom model_residue_pos  : " << models[ i ].molecule[ j ].atom[ k ].model_residue_pos << Qt::endl
                  << "  models chain atom bead_assignment    : " << models[ i ].molecule[ j ].atom[ k ].bead_assignment << Qt::endl
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
               << " models residue               : " << j << Qt::endl
               << " models residue name          : " << models[ i ].residue[ j ].name << Qt::endl
               << " models residue unique_name   : " << models[ i ].residue[ j ].unique_name << Qt::endl
               << " models residue r_atom.size() : " << atoms << Qt::endl
               << " models residue r_bead.size() : " << beads << Qt::endl
               ;
            for ( int k = 0; k < atoms; ++k ) {
               TSO
                  << LBD
                  << "  models residue r_atom                    : " << k << Qt::endl
                  << "  models residue r_atom name               : " << models[ i ].residue[ j ].r_atom[ k ].name << Qt::endl
                  << "  models residue r_atom bead assignment    : " << models[ i ].residue[ j ].r_atom[ k ].bead_assignment << Qt::endl
                  << "  models residue r_atom serial number      : " << models[ i ].residue[ j ].r_atom[ k ].serial_number << Qt::endl
                  << "  models residue r_atom hybrid name        : " << models[ i ].residue[ j ].r_atom[ k ].hybrid.name << Qt::endl
                  << "  models residue r_atom hybrid radius      : " << models[ i ].residue[ j ].r_atom[ k ].hybrid.radius << Qt::endl
                  << "  models residue r_atom hybrid protons     : " << models[ i ].residue[ j ].r_atom[ k ].hybrid.protons << Qt::endl
                  << "  models residue r_atom hybrid num_elect   : " << models[ i ].residue[ j ].r_atom[ k ].hybrid.num_elect << Qt::endl
                  << "  models residue r_atom hydration          : " << models[ i ].residue[ j ].r_atom[ k ].hydration << Qt::endl
                  ;
            }
            for ( int k = 0; k < beads; ++k ) {
               TSO
                  << LBD
                  << "  models residue r_bead                    : " << k << Qt::endl
                  << "  models residue r_bead hydration_flag     : " << ( models[ i ].residue[ j ].r_bead[ k ].hydration_flag ? "true - use bead hydration override" : "false - use atom hydration " ) << Qt::endl
                  << "  models residue r_bead hydration          : " << models[ i ].residue[ j ].r_bead[ k ].hydration << Qt::endl
                  << "  models residue r_bead atom_hydration     : " << models[ i ].residue[ j ].r_bead[ k ].atom_hydration<< Qt::endl
                  ;
            }
         }
      }
   }
   TSO
      << LBE
      ;
}               

void US_Hydrodyn::info_model_vector_mw( const QString & msg, const vector < struct PDB_model > & models, bool detail ) {
   // print out model vector info

   int model_count = (int) models.size();

   TSO
      << "US_Hydrodyn::info_model_vector_mw():" 
      << msg << Qt::endl
      << "models.size()             : " << model_count << Qt::endl
      ;

   for ( int i = 0; i < model_count; ++i ) {
      // struct PDB_model
      int chains   = (int) models[ i ].molecule.size();
      int residues = (int) models[ i ].residue .size();

      TSO
         << LBE
         << "models                   : " << i << Qt::endl
         << "models chains            : " << chains << Qt::endl
         << "models residues          : " << residues << Qt::endl
         ;

      for ( int j = 0; j < chains; ++j ) {
         // struct PDB_chain
         int atoms = (int) models[ i ].molecule[ j ].atom.size();
         double chain_mw               = models[ i ].molecule[ j ].mw;
         double chain_mw_w_delta       = models[ i ].molecule[ j ].mw + models[ i ].molecule[ j ].ionized_mw_delta;
         double chain_ionized_mw_delta = models[ i ].molecule[ j ].ionized_mw_delta;
         double sum_atom_mw            = 0e0;
         double sum_atom_mw_w_delta    = 0e0;
         for ( int k = 0; k < atoms; ++k ) {
            sum_atom_mw += models[ i ].molecule[ j ].atom[ k ].mw ;
            sum_atom_mw_w_delta +=
               models[ i ].molecule[ j ].atom[ k ].mw
               + models[ i ].molecule[ j ].atom[ k ].ionized_mw_delta
               ;
         }
         double mw_diff         = chain_mw - sum_atom_mw;
         double mw_diff_w_delta = chain_mw_w_delta - sum_atom_mw_w_delta;
         
         TSO
            << LBD
            << " models chain idx                     : " << j << Qt::endl
            << " models chain atoms                   : " << atoms << Qt::endl
            << " models chain mw                      : " << chain_mw << Qt::endl
            << " models chain ionized mw delta        : " << chain_ionized_mw_delta << Qt::endl
            << " models chain mw w/delta              : " << chain_mw_w_delta << Qt::endl
            << " models chain sum atom mw             : " << sum_atom_mw << Qt::endl
            << " models chain sum atom mw w/delta     : " << sum_atom_mw_w_delta << Qt::endl
            << " models chain difference              : " << mw_diff << Qt::endl
            << " models chain difference w/delta      : " << mw_diff_w_delta << Qt::endl
            ;

         if ( detail ) {
            for ( int k = 0; k < atoms; ++k ) {
               // struct PDB_atom
               TSO
                  << LBD
                  << "  models chain atom,name,resName,resSeq,mw,idelta   : "
                  << k << " "
                  << models[ i ].molecule[ j ].atom[ k ].name << " "
                  << models[ i ].molecule[ j ].atom[ k ].resName << " "
                  << models[ i ].molecule[ j ].atom[ k ].resSeq << " "
                  << models[ i ].molecule[ j ].atom[ k ].mw << " "
                  << models[ i ].molecule[ j ].atom[ k ].ionized_mw_delta << " "
                  << Qt::endl
                  ;
            }
         }
      }
      {

         double all_residue_hybrid_mw         = 0e0;
         double all_residue_hybrid_mw_w_delta = 0e0;
         double all_bead_mw                   = 0e0;
         double all_bead_mw_w_delta           = 0e0;

         for ( int j = 0; j < residues; ++j ) {
            // struct residue
            {
               int atoms = (int) models[ i ].residue[ j ].r_atom.size();

               double sum_hybrid_mw         = 0e0;
               double sum_hybrid_mw_w_delta = 0e0;
               for ( int k = 0; k < atoms; ++k ) {
                  sum_hybrid_mw         += models[ i ].residue[ j ].r_atom[ k ].hybrid.mw;

                  sum_hybrid_mw_w_delta +=
                     models[ i ].residue[ j ].r_atom[ k ].hybrid.mw
                     + models[ i ].residue[ j ].r_atom[ k ].hybrid.ionized_mw_delta
                     ;
               }
               all_residue_hybrid_mw         += sum_hybrid_mw;
               all_residue_hybrid_mw_w_delta += sum_hybrid_mw_w_delta;

               TSO
                  << LBD
                  << " models residue idx,atoms, sum atom.hybrid mw, sum w/delta: "
                  << " models residue idx                   : " << j << Qt::endl
                  << " models residue atoms                 : " << atoms << Qt::endl
                  << " models residue sum hybrid mw         : " << sum_hybrid_mw << Qt::endl
                  << " models residue sum hybrid mw w/delta : " << sum_hybrid_mw_w_delta << Qt::endl
                  ;

               if ( detail ) {
                  for ( int k = 0; k < atoms; ++k ) {
                     TSO
                        << LBD
                        << "  models residue r_atom name, mw, idelta        : "
                        << k << " "
                        << models[ i ].residue[ j ].r_atom[ k ].name << " "
                        << models[ i ].residue[ j ].r_atom[ k ].hybrid.mw << " "
                        << models[ i ].residue[ j ].r_atom[ k ].hybrid.ionized_mw_delta << " "
                        << Qt::endl
                        ;
                  }
               }
            }
            {
               int beads = (int) models[ i ].residue[ j ].r_bead.size();
               double sum_bead_mw               = 0e0;
               double sum_bead_ionized_mw_delta = 0e0;
               double sum_bead_mw_w_delta       = 0e0;
               for ( int k = 0; k < beads; ++k ) {
                  sum_bead_mw               += models[ i ].residue[ j ].r_bead[ k ].mw;
                  sum_bead_ionized_mw_delta += models[ i ].residue[ j ].r_bead[ k ].mw + models[ i ].residue[ j ].r_bead[ k ].ionized_mw_delta ;
                  sum_bead_mw_w_delta       += models[ i ].residue[ j ].r_bead[ k ].mw + models[ i ].residue[ j ].r_bead[ k ].ionized_mw_delta ;
               }
               all_bead_mw         += sum_bead_mw;
               all_bead_mw_w_delta += sum_bead_mw_w_delta;

               TSO
                  << LBD
                  << " models residue idx                          : " << j << Qt::endl
                  << " models residue r_beads                      : " << beads                     << Qt::endl
                  << " models residue sum r_bead mw                : " << sum_bead_mw               << Qt::endl
                  << " models residue sum r_bead ionized mw delta  : " << sum_bead_ionized_mw_delta << Qt::endl
                  << " models residue sum r_bead mw w/delta        : " << sum_bead_mw_w_delta       << Qt::endl
                  ;

               if ( detail ) {
                  for ( int k = 0; k < beads; ++k ) {
                     TSO
                        << LBD
                        << "  models residue r_bead, mw, ionized_mw_delta            : "
                        << k << " "
                        << models[ i ].residue[ j ].r_bead[ k ].mw << " "
                        << models[ i ].residue[ j ].r_bead[ k ].ionized_mw_delta << " "
                        << Qt::endl
                        ;
                  }
               }
            }
         }
         TSO
            << LBD
            << " models sum residue atom hybrid mw, sum residue atom hybrid mw w/delta, sum bead mw, sum bead mw w/delta  : "
            << all_residue_hybrid_mw << " "
            << all_residue_hybrid_mw_w_delta << " "
            << all_bead_mw << " "
            << all_bead_mw_w_delta << " "
            << Qt::endl
            ;
      }
   }
   TSO
      << LBE
      ;
}

void US_Hydrodyn::info_residue_p_residue( struct PDB_model & model ) {
   TSO
      << LBE
      << "US_Hydrodyn::info_residue_p_residue():" 
      << LBE
      ;
   
   int chains   = (int) model.molecule.size();
   int residues = (int) model.residue .size();

   set < struct residue * > residue_checked;
   set < int >              residue_pos_checked;

   // check PDB_model's PDB_chains's PDB_atom's p_residues vs ref residues

   for ( int j = 0; j < chains; ++j ) {
      // struct PDB_chain
      int atoms = (int) model.molecule[ j ].atom.size();
      TSO
         << LBD
         << " models chain             : " << j << Qt::endl
         << " models chain atoms       : " << atoms << Qt::endl
         ;
      for ( int k = 0; k < atoms; ++k ) {
         // struct PDB_atom
         if ( !residue_checked.count( model.molecule[ j ].atom[ k ].p_residue ) ) {
               
            residue_checked    .insert( model.molecule[ j ].atom[ k ].p_residue );

            TSO
               << LBD
               << "  Unchecked p_residue"                    << Qt::endl
               << "  model chain atom                    : " << k << Qt::endl
               << "  model chain atom name               : " << model.molecule[ j ].atom[ k ].name << Qt::endl
               << "  model chain atom resName            : " << model.molecule[ j ].atom[ k ].resName << Qt::endl
               << "  model chain atom resSeq             : " << model.molecule[ j ].atom[ k ].resSeq << Qt::endl
               << "  model chain atom p_residue          : " << ( model.molecule[ j ].atom[ k ].p_residue ? "set" : "not set" ) << Qt::endl
               << "  model chain atom model_residue_pos  : " << model.molecule[ j ].atom[ k ].model_residue_pos << Qt::endl
               ;

            // compare residues
            if ( !model.molecule[ j ].atom[ k ].p_residue ) {
               TSO << "******** p_residue not set" << Qt::endl;
               continue;
            }

            if ( residues <= model.molecule[ j ].atom[ k ].model_residue_pos ) {
               TSO << "******** residues too small for model_residue_pos" << Qt::endl;
               continue;
            }
            residue_pos_checked.insert( model.molecule[ j ].atom[ k ].model_residue_pos );
                  
            info_compare_residues( model.molecule[ j ].atom[ k ].p_residue, & model.residue[ model.molecule[ j ].atom[ k ].model_residue_pos ] );
         }
      }
   }

   // check PDB_model's residue's for unchecked

   for ( int j = 0; j < residues; ++j ) {
      if ( !residue_pos_checked.count( j ) ) {
         TSO << "*** Warning: found orphan residue " << j << Qt::endl;
      }
   }
}

void US_Hydrodyn::info_compare_residues( struct residue * res1, struct residue * res2 ) {
   TSO
      << "US_Hydrodyn::info_compare_residues():"
      << Qt::endl
      ;

   int res1atoms = (int) res1->r_atom.size();
   int res1beads = (int) res1->r_bead.size();
   int res2atoms = (int) res2->r_atom.size();
   int res2beads = (int) res2->r_bead.size();

   bool errors = false;

   if ( res1atoms != res2atoms ) {
      TSO
         << "US_Hydrodyn::info_compare_residues(): atom sizes differ " << res1atoms << " vs " << res2atoms << Qt::endl;
      errors = true;
   }

   if ( res1beads != res2beads ) {
      TSO
         << "US_Hydrodyn::info_compare_residues(): bead sizes differ " << res1beads << " vs " << res2beads << Qt::endl;
      errors = true;
   }

   if ( errors ) {
      TSO
         << "US_Hydrodyn::info_compare_residues(): errors prevent further checks" << Qt::endl;
      return;
   }
      
   TSO
      << "US_Hydrodyn::info_compare_residues(): checking beads" << Qt::endl;
   for ( int i = 0; i < res1beads; ++i ) {
      if ( res1->r_bead[ i ].mw != res2->r_bead[ i ].mw ) {
         TSO
            << "US_Hydrodyn::info_compare_residues(): bead " << i << " mw difference " <<  res1->r_bead[ i ].mw << " vs " << res2->r_bead[ i ].mw << Qt::endl;
         ;
         errors = true;
      }
      if ( res1->r_bead[ i ].ionized_mw_delta != res2->r_bead[ i ].ionized_mw_delta ) {
         TSO
            << "US_Hydrodyn::info_compare_residues(): bead " << i << " ionized mw delta difference " <<  res1->r_bead[ i ].ionized_mw_delta << " vs " << res2->r_bead[ i ].ionized_mw_delta << Qt::endl;
         ;
         errors = true;
      }
   }

   if ( !errors ) {
      TSO
         << "US_Hydrodyn::info_compare_residues(): beads identical" << Qt::endl;
   }
}

void US_Hydrodyn::info_residue_vector( const QString & msg, vector < struct residue > & residue_v, bool only_pKa_dependent ) {
   TSO
      << LBE
      << "US_Hydrodyn::info_residue_vector()" << Qt::endl
      << msg << Qt::endl
      << LBE
      ;

   int residues = (int) residue_v.size();
   TSO
      << LBD
      << " residues                     : " << residues << Qt::endl
      ;

   for ( int j = 0; j < residues; ++j ) {
      // struct residue
      if ( !only_pKa_dependent || residue_v[ j ].pKas.size() ) {

         int atoms = (int) residue_v[ j ].r_atom.size();
         int beads = (int) residue_v[ j ].r_bead.size();
         TSO
            << LBD
            << " residue                     : " << j << Qt::endl
            << " residue name                : " << residue_v[ j ].name << Qt::endl
            << " residue unique_name         : " << residue_v[ j ].unique_name << Qt::endl
            << " residue r_atom.size()       : " << atoms << Qt::endl
            << " residue r_bead.size()       : " << beads << Qt::endl
            << " residue vbar                : " << residue_v[ j ].vbar << Qt::endl
            << " residue vbars               : " << US_Vector::qs_vector( residue_v[ j ].vbars ) << Qt::endl
            << " residue pKas                : " << US_Vector::qs_vector( residue_v[ j ].pKas ) << Qt::endl
            << " residue vbar_at_pH          : " << residue_v[ j ].vbar_at_pH << Qt::endl
            << " residue ionized_mw_delta    : " << residue_v[ j ].ionized_mw_delta << Qt::endl
            ;
         for ( int k = 0; k < atoms; ++k ) {
            if ( !only_pKa_dependent || residue_v[ j ].r_atom[ k ].hybrid.ionized_mw_delta ) {
               TSO
                  << LBD
                  << "  residue r_atom                         : " << k << Qt::endl
                  << "  residue r_atom name                    : " << residue_v[ j ].r_atom[ k ].name << Qt::endl
                  << "  residue r_atom bead assignment         : " << residue_v[ j ].r_atom[ k ].bead_assignment << Qt::endl
                  << "  residue r_atom serial number           : " << residue_v[ j ].r_atom[ k ].serial_number << Qt::endl
                  << "  residue r_atom hybrid name             : " << residue_v[ j ].r_atom[ k ].hybrid.name << Qt::endl
                  << "  residue r_atom hybrid mw               : " << residue_v[ j ].r_atom[ k ].hybrid.mw << Qt::endl
                  << "  residue r_atom hybrid ionized_mw_delta : " << residue_v[ j ].r_atom[ k ].hybrid.ionized_mw_delta << Qt::endl
                  << "  residue r_atom hybrid radius           : " << residue_v[ j ].r_atom[ k ].hybrid.radius << Qt::endl
                  << "  residue r_atom hybrid protons          : " << residue_v[ j ].r_atom[ k ].hybrid.protons << Qt::endl
                  << "  residue r_atom hydration               : " << residue_v[ j ].r_atom[ k ].hydration << Qt::endl
                  ;
            }
         }
         for ( int k = 0; k < beads; ++k ) {
            if ( !only_pKa_dependent || residue_v[ j ].r_bead[ k ].ionized_mw_delta ) {
               TSO
                  << LBD
                  << "  residue r_bead                    : " << k << Qt::endl
                  << "  residue r_bead atom_hydration     : " << residue_v[ j ].r_bead[ k ].atom_hydration << Qt::endl
                  << "  residue r_bead mw                 : " << residue_v[ j ].r_bead[ k ].mw << Qt::endl
                  << "  residue r_bead ionized_mw_delta   : " << residue_v[ j ].r_bead[ k ].ionized_mw_delta << Qt::endl
                  ;
            }
         }
      }
   }
   TSO
      << LBE
      ;
}

void US_Hydrodyn::info_mw( const QString & msg, const vector < struct PDB_model > & models, bool detail ) {
   TSO
      << LBE
      << "US_Hydrodyn::info_mw()" << Qt::endl
      << msg << Qt::endl
      << LBE
      ;

   for ( int i = 0; i < (int) models.size(); ++i ) {
      info_mw( QString( "model: %1" ).arg( i ), models[ i ], detail );
   }
}

void US_Hydrodyn::info_mw( const QString & msg, const struct PDB_model & model, bool detail ) {
   TSO
      << LBE
      << "US_Hydrodyn::info_mw()" << Qt::endl
      << msg << Qt::endl
      << LBE
      ;
   int chains   = (int) model.molecule.size();
   double model_mw_w_delta = model.mw + model.ionized_mw_delta;

   TSO
      << "model chains                    : " << chains << Qt::endl
      << "model mw                        : " << model.mw << Qt::endl
      << "model ionized_mw_delta          : " << model.ionized_mw_delta << Qt::endl
      << "model mw w/delta                : " << model_mw_w_delta << Qt::endl
      << "model vbar                      : " << model.vbar << Qt::endl
      << "model protons                   : " << model.protons << Qt::endl
      << "model electrons                 : " << model.num_elect << Qt::endl
      << "use_vbar( model vbar )          : " << use_vbar( model.vbar ) << Qt::endl
      ;

   for ( int j = 0; j < chains; ++j ) {
      // struct PDB_chain
      int atoms = (int) model.molecule[ j ].atom.size();
      double chain_mw               = model.molecule[ j ].mw;
      double chain_mw_w_delta       = model.molecule[ j ].mw + model.molecule[ j ].ionized_mw_delta;
      double chain_ionized_mw_delta = model.molecule[ j ].ionized_mw_delta;
      double sum_atom_mw            = 0e0;
      double sum_atom_mw_w_delta    = 0e0;
      for ( int k = 0; k < atoms; ++k ) {
         if ( !model.molecule[ j ].atom[ k ].p_atom ||
              !model.molecule[ j ].atom[ k ].p_residue ) {
            TSO << "***  US_Hydrodyn::info_mw() p_atom and/or p_residue not set!" << Qt::endl;
            continue;
         }
         sum_atom_mw += model.molecule[ j ].atom[ k ].p_atom->hybrid.mw;
         sum_atom_mw_w_delta +=
            model.molecule[ j ].atom[ k ].p_atom->hybrid.mw
            + model.molecule[ j ].atom[ k ].p_atom->hybrid.ionized_mw_delta
            ;
      }
      double mw_diff         = chain_mw - sum_atom_mw;
      double mw_diff_w_delta = chain_mw_w_delta - sum_atom_mw_w_delta;
         
      TSO
         << LBD
         << " models chain idx                     : " << j << Qt::endl
         << " models chain atoms                   : " << atoms << Qt::endl
         << " models chain mw                      : " << chain_mw << Qt::endl
         << " models chain ionized mw delta        : " << chain_ionized_mw_delta << Qt::endl
         << " models chain mw w/delta              : " << chain_mw_w_delta << Qt::endl
         << " models chain sum atom mw             : " << sum_atom_mw << Qt::endl
         << " models chain sum atom mw w/delta     : " << sum_atom_mw_w_delta << Qt::endl
         << " models chain difference              : " << mw_diff << Qt::endl
         << " models chain difference w/delta      : " << mw_diff_w_delta << Qt::endl
         ;


      if ( detail ) {
         double sum_mw          = 0e0;
         double sum_bead_mw     = 0e0;
         double sum_bead_ref_mw = 0e0;

         for ( int k = 0; k < atoms; ++k ) {
            // struct PDB_atom
            if ( !model.molecule[ j ].atom[ k ].p_atom ||
                 !model.molecule[ j ].atom[ k ].p_residue ) {
               continue;
            }

            if ( model.molecule[ j ].atom[ k ].active) {
               sum_mw          += model.molecule[ j ].atom[ k ].mw          + model.molecule[ j ].atom[ k ].ionized_mw_delta;
               if ( model.molecule[ j ].atom[ k ].is_bead ) {
                  sum_bead_mw     += model.molecule[ j ].atom[ k ].bead_mw     + model.molecule[ j ].atom[ k ].bead_ionized_mw_delta;
                  sum_bead_ref_mw += model.molecule[ j ].atom[ k ].bead_ref_mw + model.molecule[ j ].atom[ k ].bead_ref_ionized_mw_delta;
               }
            }
            
            TSO
               << LBD
               << "  model chain atom                                : " << k << Qt::endl
               << "  model chain atom name                           : " << model.molecule[ j ].atom[ k ].name << Qt::endl
               << "  model chain atom resName                        : " << model.molecule[ j ].atom[ k ].resName << Qt::endl
               << "  model chain atom resSeq                         : " << model.molecule[ j ].atom[ k ].resSeq << Qt::endl
               << "  model chain atom mw                             : " << model.molecule[ j ].atom[ k ].mw << Qt::endl
               << "  model chain atom ionized_mw_delta               : " << model.molecule[ j ].atom[ k ].ionized_mw_delta << Qt::endl
               << "  model chain atom p_residue name                 : " << model.molecule[ j ].atom[ k ].p_residue->name << Qt::endl
               << "  model chain atom p_atom name                    : " << model.molecule[ j ].atom[ k ].p_atom->name << Qt::endl
               << "  model chain atom p_atom mw                      : " << model.molecule[ j ].atom[ k ].p_atom->hybrid.mw << Qt::endl
               << "  model chain atom p_atom hybrid ionized_mw_delta : " << model.molecule[ j ].atom[ k ].p_atom->hybrid.ionized_mw_delta << Qt::endl
               << "  model chain atom p_atom hybrid radius           : " << model.molecule[ j ].atom[ k ].p_atom->hybrid.radius << Qt::endl
               << "  model chain atom p_atom hybrid protons          : " << model.molecule[ j ].atom[ k ].p_atom->hybrid.protons << Qt::endl
               << "  model chain atom p_atom hydration               : " << model.molecule[ j ].atom[ k ].p_atom->hydration << Qt::endl
               << "  model chain atom active                         : " << ( model.molecule[ j ].atom[ k ].active ? "yes" : "no" ) << Qt::endl
               << "  model chain atom is_bead                        : " << ( model.molecule[ j ].atom[ k ].is_bead ? "yes" : "no" ) << Qt::endl
               ;
            if ( model.molecule[ j ].atom[ k ].is_bead ) {
               TSO
                  << "  model chain atom bead_mw                        : " << model.molecule[ j ].atom[ k ].bead_mw << Qt::endl
                  << "  model chain atom bead_ionized_mw_delta          : " << model.molecule[ j ].atom[ k ].bead_ionized_mw_delta << Qt::endl
                  << "  model chain atom bead_ref_mw                    : " << model.molecule[ j ].atom[ k ].bead_ref_mw << Qt::endl
                  << "  model chain atom bead_ref_ionized_mw_delta      : " << model.molecule[ j ].atom[ k ].bead_ref_ionized_mw_delta << Qt::endl
                  ;
            }
         }
         TSO
            << LBD
            << " active sum_mw             : " << sum_mw << Qt::endl
            << " active bead sum_bead_mw   : " << sum_bead_mw << Qt::endl
            << " active sum_bead_ref_mw    : " << sum_bead_ref_mw << Qt::endl
            ;
      }
   }
}

void US_Hydrodyn::info_model_vector_vbar( const QString & msg, const vector <struct PDB_model> & models ) {
   // print out model vector info

   int model_count = (int) models.size();

   TSO
      << "US_Hydrodyn::info_model_vector_vbars()" << Qt::endl
      << msg << Qt::endl
      << "models.size()             : " << model_count << Qt::endl
      ;

   for ( int i = 0; i < model_count; ++i ) {
      // struct PDB_model
      TSO
         << " model " << ( i + 1 ) << " vbar " << models[ i ].vbar << " use_vbar " << use_vbar( models[ i ].vbar ) << Qt::endl;
   }
}

void US_Hydrodyn::info_residue_protons_electrons_at_pH( double pH, const struct PDB_model & model ) {
   // qDebug() << "US_Hydrodyn::protons_at_pH() start";
   int chains   = (int) model.molecule.size();
   double protons   = 0;
   double electrons = 0;

   TSO
      << "pH" << "," << pH << Qt::endl
      << "resname" << ","
      << "atomname" << ","
      << "r_0 protons" << ","
      << "r_0 hybrid name" << ","
      << "r_1 protons" << ","
      << "r_1 hybrid name" << ","
      << "fraction prior" << ","
      << "fraction this" << ","
      << "protons" << ","
      << "electrons" << ","
      << "net charge" << ","
      << Qt::endl
      ;

   for ( int j = 0; j < chains; ++j ) {
      // struct PDB_chain
      int atoms = (int) model.molecule[ j ].atom.size();
      for ( int k = 0; k < atoms; ++k ) {
         if ( !model.molecule[ j ].atom[ k ].p_residue ) {
            qDebug() << "**** US_Hydrodyn::protons_at_pH(): p_residue not set!";
            continue;
         }
         if ( !model.molecule[ j ].atom[ k ].p_atom ) {
            qDebug() << "**** US_Hydrodyn::protons_at_pH(): p_atom not set!";
            continue;
         }
         struct residue * res  = model.molecule[ j ].atom[ k ].p_residue;
         struct atom    * atom = model.molecule[ j ].atom[ k ].p_atom;
         vector < double > fractions = basic_fractions( pH, res );
         double this_protons   = ionized_residue_atom_protons( fractions, res, atom );
         double this_electrons = atom->hybrid.num_elect;

         if ( fractions.size() > 1 && atom->ionization_index ) {
            TSO 
               << res->name << ","
               << atom->name << ","
               << res->r_atom_0[ atom->ionization_index ].hybrid.protons << ","
               << res->r_atom_0[ atom->ionization_index ].hybrid.name << ","
               << res->r_atom_1[ atom->ionization_index ].hybrid.protons << ","
               << res->r_atom_1[ atom->ionization_index ].hybrid.name << ","
               << fractions[ atom->ionization_index - 1 ] << ","
               << fractions[ atom->ionization_index ] << ","
               << this_protons << ","
               << this_electrons << ","
               << ( this_protons - this_electrons ) << ","
               << Qt::endl;
         } else {
            TSO
               << res->name << ","
               << atom->name << ","
               << atom->hybrid.protons << ","
               << atom->hybrid.name << ","
               << "" << ","
               << "" << ","
               << "1" << ","
               << "" << ","
               << this_protons << ","
               << this_electrons << ","
               << ( this_protons - this_electrons ) << ","
               << Qt::endl;
         }            
                     
         protons   += this_protons;
         electrons += this_electrons;
      }
   }
   TSO
      << "Total" << ","
      << "" << ","
      << "" << ","
      << "" << ","
      << "" << ","
      << "" << ","
      << "" << ","
      << "" << ","
      << protons << ","
      << electrons << ","
      << ( protons - electrons ) << ","
      << Qt::endl;

}

QString US_Hydrodyn::info_cite( const QString & package ) {
   static map < QString, QString > citations =
      {
         {
            "somo-no-overlaps",
            "Rai et al., Structure. 2005;13:723–34.\n"
            "Brookes et al., Eur Biophys J Biophys Lett. 2010;39(3):423–35.\n"
            // "Brookes et al., Macromol Biosci. 2010;10(7):746–53\n"
         }
         ,{
            "atob-no-overlaps",
            "Byron, Biophys J. 1997;72:408–15.\n"
            "Brookes et al., Eur Biophys J Biophys Lett. 2010;39(3):423–35.\n"
            // "Brookes et al., Macromol Biosci. 2010;10(7):746–53\n"
         }
         ,{
            "somo-overlaps",
            "Rai et al., Structure. 2005;13:723–34.\n"
            "Brookes et al., Eur Biophys J Biophys Lett. 2010;39(3):423–35.\n"
            "Rocco and Byron, Eur Biophys J. 2015;44:417–431.\n"
         }
         ,{
            "vdw",
            "Brookes and Rocco, Eur Biophys J. 2018;47:855–864.\n"
         }
         ,{
            "smi",
            // "Garcia de la Torre and Bloomfield, Q Rev Biophys. 1981;14(1):81–139.\n"
            "Spotorno et al., Eur Biophys J. 1997;25(5/6):373–84.\n"
            // "Brookes et al., Eur Biophys J Biophys Lett. 2010b;39(3):423–35.\n"
         }
         ,{
            "zeno",
            "Kang et al., Phys Rev E Stat Nonlinear Soft Mater Phys. 2004;69:031918.\n"
            // "Mansfield and Douglas, Phys Rev E Stat Nonlinear Soft Matter Phys. 2008;78:046712.\n"
            "Juba et al., J Res Natl Inst Stand Technol. 2017;122:1–2.\n"
            "Rocco and Byron, Eur Biophys J. 2015;44:417–431.\n"
            // "Brookes and Rocco, Eur Biophys J. 2018;47:855–864.\n"
         }
         ,{
            "grpy",
            "Zuk, P.J., Cichocki, B. and Szymczak, P., 2018. Biophys J. 115(5), pp.782-800.\n"
            "Rocco M., Brookes E., Byron O. 2021 in Encyclopedia of Biophysics. https://doi.org/10.1007/978-3-642-35943-9_292-1\n"
         }
         ,{
            "best",
            "Aragon, J Comput Chem. 2004;25:1191–1205.\n"
            "Aragon, Methods. 2011;54:101–114.\n"
            "Brookes E., Rocco M. 2016 in Analytical Ultracentrifugation. Springer, Tokyo.\n"
         }
      }
   ;

   QString citations_used = "";
   
   // info_citation_stack();
   
   if ( citation_stack.size() ) {
      QString bead_model_citations = "";
      for ( int i = 0; i < (int) citation_stack.size(); ++i ) {
         if ( citations.count( citation_stack[i] ) ) {
            bead_model_citations += split_and_prepend( citations[ citation_stack[i] ], "  " );
         } else {
            TSO << "No citations found for bead model method " << citation_stack[i] << Qt::endl;
         }
      }
      if ( !bead_model_citations.isEmpty() ) {
         citations_used += " For the construction of the bead model:\n" + citation_cleanup( bead_model_citations ) + "\n";
      }
   }

   if ( citations.count( package ) ) {
      citations_used += " For the calculation of hydrodynamic parameters:\n" + split_and_prepend( citations[ package ], "  " );
   } else {
      TSO << "No citation informaton stored for package " << package << Qt::endl;
   }

   if ( citations_used.isEmpty() ) {
      return QString( "No citation information currently available ('%1')" ).arg( package );
   }

   return "\nFor usage of these results in publications, please cite:\n" + citations_used + "\n";
}

QString US_Hydrodyn::split_and_prepend( const QString & qs, const QString & prepend ) {
   QStringList qsl = qs.split( "\n" );
   qsl.replaceInStrings( QRegExp( "^" ), prepend );
   return qsl.join( "\n" ) + "\n";
}

QString US_Hydrodyn::citation_cleanup( const QString & qs ) {
   QStringList qsl = qs.split( "\n" );
   set < QString > used;
   QStringList qslout;
   for ( int i = 0; i < (int) qsl.size(); ++i ) {
      if ( !qsl[ i ].contains( QRegExp( "^\\s*$" ) ) && !used.count( qsl[ i ] ) ) {
         used.insert( qsl[ i ] );
         qslout << qsl[ i ];
      }
   }
   return qslout.join( "\n" ) + "\n";
}

void US_Hydrodyn::citation_load_pdb() {
   // TSO << "citation_load_pdb()" << Qt::endl;
   citation_clear();
}

bool US_Hydrodyn::citation_stack_contains_type( const QString & type ) {
   for ( int i = 0; i < (int) citation_stack.size(); ++i ) {
      if ( citation_stack[i] == type ) {
         return true;
      }
   }
   return false;
}

void US_Hydrodyn::citation_build_bead_model( const QString & type ) {
   // TSO << "citation_build_bead_model( " << type << " )" << Qt::endl;
   if ( !batch_active() ) {
      citation_clear();
   }
   if ( !citation_stack_contains_type( type ) ) {
      citation_stack << type;
      // TSO << "citation_load_bead_model() adding type " << type << Qt::endl;
   }
}

void US_Hydrodyn::citation_clear() {
   citation_stack.clear();
}   

void US_Hydrodyn::citation_load_bead_model( const QString & filename ) {
   // TSO << "citation_load_bead_model( " << filename << ")" << Qt::endl;
   if ( !batch_active() ) {
      citation_clear();
   }
   QString basename = QFileInfo( filename ).baseName();
   // TSO << "citation_load_bead_model( " << filename << ") basename:" << basename << Qt::endl;
   QString type;

   if ( basename.contains( "-so_ovlp" ) ) {
      type = "somo-overlaps";
   } else if ( basename.contains( "-so" ) ) {
      type = "somo-no-overlaps";
   } else if ( basename.contains( "-a2b_ovlp" ) ) {
      type = "atob-overlaps";
   } else if ( basename.contains( "-a2b" ) ) {
      type = "atob-no-overlaps";
   } else if ( basename.contains( "-vdw" ) ) {
      type = "vdw";
   }
   
   if ( !type.isEmpty() &&
        !citation_stack_contains_type( type ) ) {
      citation_stack << type;
      // TSO << "citation_load_bead_model() adding type " << type << Qt::endl;
   }
}

void US_Hydrodyn::info_citation_stack() {
   TSO << "citation_stack() : " << citation_stack.join( " , " ) << Qt::endl;
}

void US_Hydrodyn::info_residue( struct residue & r, const QString & msg ) {
   TSO << LBD << "info_residue() : " << msg << Qt::endl << LBD;
   
   TSO << r.comment << Qt::endl;
   TSO << r.name.toUpper()
       << "\t" << r.type
       << "\t" << QString().sprintf("%7.2f", r.molvol)
       << "\t" << r.asa
       << "\t" << r.r_atom.size()
       << "\t" << r.r_bead.size()
       << "\t" << r.vbar;

   for ( int j = 0; j < (int) r.pKas.size(); ++j ) {
      TSO << "\t" << r.vbar
          << "\t" << r.pKas[ j ]
         ;
   }
            
   TSO << "\n";
            
   for (unsigned int j=0; j<r.r_atom.size(); j++)
   {
      TSO << r.r_atom[j].name.toUpper()
          << "\t" << r.r_atom[j].hybrid.name
          << "\t" << r.r_atom[j].hybrid.mw
          << "\t" << r.r_atom[j].hybrid.radius
          << "\t" << r.r_atom[j].bead_assignment
          << "\t" << (unsigned int) r.r_atom[j].positioner
          << "\t" << r.r_atom[j].serial_number 
          << "\t" << r.r_atom[j].hydration
         ;
      if ( r.r_atom[j].ionization_index ) {
         int ionization_index = r.r_atom[j].ionization_index;
         if ( r.r_atom_1.count( ionization_index ) ) {
            TSO << "\t" << ionization_index
                << "\t" << r.r_atom_1[ ionization_index ].hybrid.name
                << "\t" << r.r_atom_1[ ionization_index ].hybrid.mw
                << "\t" << r.r_atom_1[ ionization_index ].hybrid.radius
                << "\t" << r.r_atom_1[ ionization_index ].bead_assignment
                << "\t" << (unsigned int) r.r_atom_1[ ionization_index ].positioner
                << "\t" << r.r_atom_1[ ionization_index ].serial_number 
                << "\t" << r.r_atom_1[ ionization_index ].hydration
               ;
         }
      }
      TSO << "\n";               
   }
   for (unsigned int j=0; j<r.r_bead.size(); j++)
   {
      TSO << r.r_bead[j].hydration
          << "\t" << r.r_bead[j].color
          << "\t" << r.r_bead[j].placing_method
          << "\t" << r.r_bead[j].chain
          << "\t" << r.r_bead[j].volume << "\n";
   }

   TSO << LBD;

   for ( int i = 0; i < (int) r.r_atom.size(); ++i ) {
      TSO <<
         QString( "r_atom %1 name %2 hybrid.name %3 ionization_index %4\n" )
         .arg( i, 2 )
         .arg( r.r_atom[i].name, 4 )
         .arg( r.r_atom[i].hybrid.name, 5 )
         .arg( r.r_atom[i].ionization_index )
         ;
   }

   TSO << LBD;
   
   for ( auto it = r.r_atom_1.begin();
         it != r.r_atom_1.end();
         ++it ) {
      TSO <<
         QString( "r_atom_1 %1 name %2 hybrid.name %3 ionization_index %4\n" )
         .arg( it->first, 2 )
         .arg( it->second.name, 4 )
         .arg( it->second.hybrid.name, 5 )
         .arg( it->second.ionization_index )
         ;
   }
   TSO << LBD;

   for ( int i = 0; i < (int) r.pKas.size(); ++i ) {
      TSO <<
         QString( " r.pKas[%1] = %2\n" )
         .arg( i )
         .arg( r.pKas[i] )
         ;
   }      
   TSO << LBD;
}



#undef TSO
#undef LBE
#undef LDB

