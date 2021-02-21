// us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o
// us_hydrodyn_info.cpp contains code to report structures for debugging
// (this) us_hydrodyn_util.cpp contains other various code, such as disulfide code

#include "us_hydrodyn.h"

// #define DEBUG_SS

void US_Hydrodyn::SS_setup() {
#if defined( DEBUG_SS )
   qDebug() << "SS_setup()";
#endif
   cystine_residues.clear();
   cystine_residues.insert( "CYS" );
   cystine_residues.insert( "CYH" );
   
   sulfur_atoms.clear();
   sulfur_atoms.insert( "SG" );
   sulfur_atoms.insert( "S" );
}

void US_Hydrodyn::SS_init() {
#if defined( DEBUG_SS )
   qDebug() << "SS_init()";
#endif
   sulfur_pdb_line             .clear();
   sulfur_coordinates          .clear();
   sulfur_paired               .clear();
   sulfur_pdb_chain_atom_idx   .clear();
   sulfur_pdb_chain_idx        .clear();
}

void US_Hydrodyn::SS_apply( struct PDB_model & model ) {
   int sulfurs = (int) sulfur_pdb_line.size();

   model.num_SS_bonds = 0;
   model.num_SH_free  = 0;

   if ( !sulfurs ) {
      return;
   }
   if ( !pdb_parse.find_sh ) {
      SS_init();
      return;
   }

#if defined( DEBUG_SS )
   QTextStream( stdout ) << "SS_apply()" << endl;
#endif

   if ( !gparams.count( "thresh_SS" ) ) {
      editor_msg( "red", us_tr( "internal error: no disulfide distance threshold defined" ) );
      return;
   }

   map < int, double > pair_distance;

   double thresh_SS = gparams[ "thresh_SS" ].toDouble();
   editor_msg( "dark blue", QString( "Checking disulfide distance, current distance threshold %1 [A]" ).arg( thresh_SS ) );

   for ( int i = 0; i < sulfurs; ++i ) {
      if ( !sulfur_paired.count( i ) ) {
#if defined( DEBUG_SS )
         QTextStream( stdout )
            << "sulfur pdb line   " <<  sulfur_pdb_line[ i ] << endl
            << "sulfur coordinates"
            <<  sulfur_coordinates[ i ].axis[ 0 ] << " , "
            <<  sulfur_coordinates[ i ].axis[ 1 ] << " , "
            <<  sulfur_coordinates[ i ].axis[ 2 ]
            << endl;
#endif
         for ( int j = i + 1; j < sulfurs; ++j ) {
            if ( !sulfur_paired.count( j ) ) {
               double this_distance =  dist( sulfur_coordinates[ i ], sulfur_coordinates[ j ] );
               if ( this_distance <= thresh_SS ) {
                  ++model.num_SS_bonds;
                  pair_distance[ j ] = pair_distance[ i ] = this_distance;
                  sulfur_paired[ i ] = j;
                  sulfur_paired[ j ] = i;
                  editor_msg( "dark blue", QString( "SS bond found %1 %2 distance %3 [A]" )
                              .arg( sulfur_pdb_line[ i ].left( 26 ) )
                              .arg( sulfur_pdb_line[ j ].left( 26 ) )
                              .arg( this_distance ) );
               }
            }
         }
      }
   }

   for ( auto it = sulfur_paired.begin();
         it != sulfur_paired.end();
         ++it ) {
      if ( it->first < it->second ) {
#if defined( DEBUG_SS )
         QTextStream( stdout )
            << "sulfurs paired: distance [A]:" << pair_distance[ it->first ] << endl
            << sulfur_pdb_line[ it->first ] << endl
            << sulfur_pdb_line[ it->second ] << endl
            ;
#endif
         SS_change_residue( model, sulfur_pdb_line[ it->first ], "CYS" );
         SS_change_residue( model, sulfur_pdb_line[ it->second ], "CYS" );
      }
   }

   for ( int i = 0; i < sulfurs; ++i ) {
      if ( !sulfur_paired.count( i ) ) {
#if defined( DEBUG_SS )
         QTextStream( stdout )
            << "sulfurs unpaired:" << endl
            << sulfur_pdb_line[ i ] << endl
            ;
#endif
         SS_change_residue( model, sulfur_pdb_line[ i ], "CYH" );
         model.num_SH_free++;
         editor_msg( "dark blue", QString( "SH found, CYS converted to CYH %1" )
                     .arg( sulfur_pdb_line[ i ].left( 26 ) )
                     );
      }
   }

   SS_init(); // clear after application
}

void US_Hydrodyn::SS_change_residue( struct PDB_model & model, const QString & line, const QString target_residue ) {
#if defined( DEBUG_SS )
   QTextStream( stdout ) << "SS_change_residue()" << endl;
#endif
   QString source_residue   = line.mid( 17, 3 ).trimmed();
   QString residue_sequence = line.mid( 22, 5 ).trimmed();
   QString chain_id         = line.mid( 20, 2 ).trimmed();

   if ( !multi_residue_map.count( target_residue ) || !multi_residue_map[ target_residue ].size() ) {
      editor_msg( "red", QString( us_tr( "Residue file is missing residue name %1, can not covert residue %2 %3 chain %4" ) )
                  .arg( target_residue )
                  .arg( residue_sequence )
                  .arg( source_residue )
                  .arg( chain_id )
                  );
      return;
   }

#if defined( DEBUG_SS )
   QTextStream( stdout )
      << "SS_change_residue():"
      << "source_residue   '" << source_residue   << "'" << endl
      << "target_residue   '" << target_residue   << "'" << endl
      << "residue_sequence '" << residue_sequence << "'" << endl
      << "chain_id         '" << chain_id         << "'" << endl
      << endl;
#endif

   if ( source_residue == target_residue ) {
      // nothing to do
#if defined( DEBUG_SS )
      QTextStream( stdout ) << "SS_change_residue(): nothing to do" << endl;
#endif
      return;
   }

   struct residue new_residue = residue_list[ multi_residue_map[ target_residue ][ 0 ] ];
   
   // find matching model_vector molecule(chain) atoms

   if ( !sulfur_pdb_chain_idx.count( chain_id ) || !sulfur_pdb_chain_idx[ chain_id ].size() ) {
      QTextStream( stdout ) << "SS_change_residue(): Internal error chain index vector missing or empty chain_id " << chain_id << endl;
      return;
   }
   if (
       !sulfur_pdb_chain_atom_idx.count( chain_id ) ||
       !sulfur_pdb_chain_atom_idx[ chain_id ].count( residue_sequence ) ||
       !sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ].size() 
       ) {
      QTextStream( stdout ) << "SS_change_residue(): Internal error chain index vector missing or empty chain_id " << chain_id << " residue_sequence " << residue_sequence << endl;
      return;
   }

   unsigned int update_model_residue_pos = 0;
   
#if defined( DEBUG_SS )
   {
      unsigned int min_atom_idx = sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ][ 0 ];
      unsigned int max_atom_idx = sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ].back();

      QTextStream( stdout )
         << "SS_change_residue(): structure info" << endl
         << "  sulfur_pdb_chain_atom_idx[ " << chain_id << " ][ 0 ]   = " << min_atom_idx << endl
         << "  sulfur_pdb_chain_atom_idx[ " << chain_id << " ].back() = " << max_atom_idx << endl
         << "  sulfur_pdb_chain_idx     [ " << chain_id << " ].size() = " << sulfur_pdb_chain_idx[ chain_id ].size() << endl
         ;
      for ( int i = 0; i < (int) sulfur_pdb_chain_idx[ chain_id ].size(); ++i ) {
         QTextStream( stdout )
            << "    sulfur_pdb_chain_idx[ " << chain_id << " ][ " << i << " ] = " << sulfur_pdb_chain_idx[ chain_id ][ i ] << endl;
      }
      for ( int i = 0; i < (int) sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ].size(); ++i ) {
         QTextStream( stdout )
            << "    sulfur_pdb_chain_atom_idx[ " << chain_id << " ][ " << residue_sequence << " ][ " << i << " ] = " << sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ][ i ] << endl;
      }
   }
#endif

   {
      unsigned int min_atom_idx = sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ][ 0 ];
      unsigned int max_atom_idx = sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ].back();
      bool found = false;
      for ( unsigned int i = 0; i < (unsigned int) sulfur_pdb_chain_idx[ chain_id ].size(); ++i ) {
         // find correct chain
         if ( (unsigned int) model.molecule.size() > sulfur_pdb_chain_idx[ chain_id ][ i ] &&
              model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom.size() ) {
            if ( (unsigned int)model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom.size() > max_atom_idx &&
                 model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ min_atom_idx ].resSeq == residue_sequence &&
                 model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ max_atom_idx ].resSeq == residue_sequence
                 ) {
               found = true;
               update_model_residue_pos = model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ max_atom_idx ].model_residue_pos;
               for ( unsigned int j = 0; j < (unsigned int) sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ].size(); ++j ) {
                  if ( model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ][ j ] ].resName != source_residue ) {
                     QTextStream( stdout ) << "SS_change_residue(): Internal error source residue resname mismatch "
                                           << " chain_id '" << chain_id << "'"
                                           << " ref resname '" << model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ][ j ] ].resName << "'"
                                           << " source_residue '" << source_residue << "'"
                                           << " residue_sequence '" << residue_sequence << "'"
                                           << endl
                        ;
                     return;
                  }
                  model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ][ j ] ].resName =
                     model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ][ j ] ].orgResName = target_residue;
               }
               break;
            } else {
#if defined( DEBUG_SS )
               QTextStream( stdout )
                  << "not found ?" << endl
                  << "model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom.size() = " << model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom.size() << endl
                  << "model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ min_atom_idx ].resSeq = " << model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ min_atom_idx ].resSeq << endl
                  << "model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ max_atom_idx ].resSeq = " << model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ max_atom_idx ].resSeq << endl
                  ;
#endif
            }
         }
         if ( found ) {
            break;
         }
      }

      if ( found ) {
         model.residue[ update_model_residue_pos ] = new_residue;
      } else {
         QTextStream( stdout ) << "SS_change_residue(): Internal error could not find match in model chain_id " << chain_id << " residue_sequence " << residue_sequence << endl;
         return;
      }      
   }

   // search through the model and replace those that match the chain, source_residue & residue sequence
   // once for the residue replace model_vector residue[ model_vector molecule(chain) model_residue_pos ] with new_residue 
}

double US_Hydrodyn::protons_at_pH( double pH, const struct PDB_model & model ) {
   // qDebug() << "US_Hydrodyn::protons_at_pH() start";
   int chains   = (int) model.molecule.size();
   double protons = 0;

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
         vector < double > fractions = basic_fractions( pH, model.molecule[ j ].atom[ k ].p_residue );
         protons += ionized_residue_atom_protons( fractions,
                                                  model.molecule[ j ].atom[ k ].p_residue,
                                                  model.molecule[ j ].atom[ k ].p_atom );
      }
   }
   // qDebug() << "US_Hydrodyn::protons_at_pH() returns " << protons;
   return protons;
}
         
double US_Hydrodyn::compute_isoelectric_point( const struct PDB_model & model ) {
   // QTextStream( stdout ) << "US_Hydrodyn::compute_isoelectric_point() start" << endl;
   // later for better performance  would could skip residues/atom with 1 fraction and count up residue/atom pairs to multiply

   // bisection scan

#define PH_START 1
#define PH_END   14
   
   {
      double start  = PH_START;
      double end    = PH_END;
      double middle = 0.5 * ( start + end );
      int iter      = 0;
      int max_iter  = 10000;
   
      double tolerance = 1e-3;
      bool root_found = false;
      do {
         middle          = 0.5 * ( start + end );
         double start_p  = protons_at_pH( start , model ) - model.num_elect;
         double middle_p = protons_at_pH( middle, model ) - model.num_elect;
         // double end_p    = protons_at_pH( end   , model ) - model.num_elect;
         // if ( !(iter % 50) ) {
         //    QTextStream( stdout )
         //       << QString("").sprintf(
         //                              "iter %d: net charge (start,middle,end) %g %g %g pos (start,middle,end) %g %g %g\n"
         //                              ,iter
         //                              ,start_p
         //                              ,middle_p
         //                              ,end_p
         //                              ,start
         //                              ,middle
         //                              ,end );
         // }

         if ( middle_p == 0 ||
              fabs( middle_p ) < tolerance ) {
            start = middle;
            end   = middle;
            root_found = true;
            break;
         }
         if ( start_p * middle_p < 0 ) {
            end   = middle;
         } else {
            start = middle;
         }
      } while ( ++iter < max_iter );
      if ( !root_found ) {
         editor_msg( "red",
                     QString( us_tr( "Isoelectric point could not be found in the pH range of %1 to %2" ) )
                     .arg( PH_START )
                     .arg( PH_END ) );

      }
      return middle;
   }

   // // grid scan
   // {
   // double best_pH  = 0;
   // double min_diff = 1e99;
   // for ( double pH = 1; pH < 14; pH += 0.01 ) {
   //    double this_diff = fabs( protons_at_pH( pH, model ) - model.num_elect );
   //    if ( min_diff > this_diff ) {
   //       min_diff = this_diff;
   //       best_pH  = pH;
   //    }
   //    // QTextStream( stdout ) << protons_at_pH( pH, model ) << " protons at pH " << pH << endl;
   // }
   // return best_pH;
   // }
}

map < QString, struct atom * > US_Hydrodyn::residue_atom_map( struct residue & residue_entry ) {
   // map residue to allow lookup on atoms by atom name
   map < QString, struct atom * > result;
   int atoms = (int) residue_entry.r_atom.size();
   for ( int i = 0; i < atoms; ++i ) {
      result[ residue_entry.r_atom[ i ].name ] = &( residue_entry.r_atom[ i ] );
   }
   return result;
}

map < QString, struct atom * > US_Hydrodyn::first_residue_atom_map( struct PDB_chain & chain ) {
   map < QString, struct atom * > result;
   int atoms = (int) chain.atom.size();
   if ( !atoms ) {
      return result;
   }
   QString resName = chain.atom[ 0 ].resName;
   QString resSeq  = chain.atom[ 0 ].resSeq;
   // QTextStream( stdout )
   //    << "US_Hydrodyn::first_residue_atom_map() resName " << resName
   //    << " orgResName " <<  chain.atom[ 0 ].orgResName
   //    << endl
   //    ;
   
   if ( !multi_residue_map.count( resName ) ||
        !multi_residue_map[ resName ].size() ) {
      QTextStream( stderr ) 
         << "US_Hydrodyn::first_residue_atom_map() resName " << resName
         << " MISSING from multi_residue_map!"
         << endl
         ;
      return result;
   }

   if ( multi_residue_map[ resName ].size() > 1 ) {
      QTextStream( stderr ) 
         << "US_Hydrodyn::first_residue_atom_map() resName " << resName
         << " WARNING more than one entry in multi_residue_map using first one!"
         << endl
         ;
   }

   auto org_residue_entry = residue_atom_map( residue_list[ multi_residue_map[ resName ][ 0 ] ] );

   for ( int i = 0; i < atoms; ++i ) {
      if ( i && ( resName != chain.atom[ i ].resName ||
                  resSeq  != chain.atom[ i ].resSeq ) ) {
         return result;
      }
      if ( chain.atom[ i ].p_atom ) {
         if ( org_residue_entry.count( chain.atom[ i ].name ) ) {
            result[ chain.atom[ i ].name ] = org_residue_entry[ chain.atom[ i ].name ];
         } else {
            if ( chain.atom[ i ].name != "OXT" ) {
               QTextStream( stderr ) 
                  << "US_Hydrodyn::first_residue_atom_map() resName " << resName
                  << " WARNING atom " << chain.atom[ i ].name << " not found in residue list, using p_atom"
                  << endl
                  ;
            }
            result[ chain.atom[ i ].name ] = chain.atom[ i ].p_atom;
         }
      } else {
         QTextStream( stderr ) << "US_Hydrodyn::first_residue_atom_map() p_atom not set for atom " << chain.atom[ i ].name << endl;
      }
   }
   return result;
}

map < QString, int > US_Hydrodyn::first_residue_PDB_atom_map( struct PDB_chain & chain ) {
   map < QString, int > result;
   int atoms = (int) chain.atom.size();
   if ( !atoms ) {
      return result;
   }
   QString resName = chain.atom[ 0 ].resName;
   QString resSeq  = chain.atom[ 0 ].resSeq;
   
   for ( int i = 0; i < atoms; ++i ) {
      if ( i && ( resName != chain.atom[ i ].resName ||
                  resSeq  != chain.atom[ i ].resSeq ) ) {
         return result;
      }
      result[ chain.atom[ i ].name ] = i;
   }
   return result;
}

map < QString, struct atom * > US_Hydrodyn::last_residue_atom_map( struct PDB_chain & chain ) {
   map < QString, struct atom * > result;
   int atoms = (int) chain.atom.size();
   if ( !atoms ) {
      return result;
   }
   QString resName = chain.atom[ atoms - 1 ].resName;
   QString resSeq  = chain.atom[ atoms - 1 ].resSeq;
   for ( int i = atoms - 1; i >= 0; --i ) {
      if ( i && ( resName != chain.atom[ i ].resName ||
                  resSeq  != chain.atom[ i ].resSeq ) ) {
         return result;
      }
      if ( chain.atom[ i ].p_atom ) {
         result[ chain.atom[ i ].name ] = chain.atom[ i ].p_atom;
      } else {
         QTextStream( stderr ) << "US_Hydrodyn::last_residue_atom_map() p_atom not set for atom " << chain.atom[ i ].name << endl;
      }
   }
   return result;
}
                  
   
   
      
