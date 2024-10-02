#include "../include/us_saxs_util.h"

#define TSO QTextStream( stdout )
#define TSE QTextStream( stderr )

// #define DEBUG_SS

bool US_Saxs_Util::run_ssbond(
                              map < QString, QString >           & parameters,
                              map < QString, QString >           & results
                              ) {
   if ( !parameters.count( "pdbfile" ) ) {
      results[ "errors" ] = "no pdbfile defined";
      return false;
   }
   if ( !parameters.count( "thresh_SS" ) ) {
      parameters[ "thresh_SS" ] = "2.5";
   }
   gparams[ "thresh_SS" ] = parameters[ "thresh_SS" ];
   
   if ( !parameters.count( "_base_directory" ) ) {
      parameters[ "_base_directory" ] = ".";
   }

   // for files with spaces in names
   QString file = parameters[ "pdbfile" ].replace('"', "").replace("\\/","/") ;

   QFileInfo fi( file );
   if ( !fi.exists() ) {
      results[ "errors" ] += QString( " file %1 does not exist." ).arg( file );
      return false;
   } else {
      if ( !fi.isReadable() ) {
         results[ "errors" ] += QString( " file %1 exists but is not readable." ).arg( file );
         return false;
      } else {
         file = fi.filePath();
      }
   }

   bool parameters_set_first_model;
   if ( !parameters.count( "first_model" ) ) {
      parameters_set_first_model = false;
   } else {
      parameters_set_first_model = true;
   } 

   pdb_parse.find_sh = true;
   bead_model_from_file = false;
   new_residues.clear( );

   if ( misc.pb_rule_on ) {
      residue_list = save_residue_list;
   } else {
      residue_list = save_residue_list_no_pbr;
   }

   multi_residue_map = save_multi_residue_map;

   SS_setup();
   // SS_init();

   read_residue_file();

   if ( read_pdb_hydro( file, parameters_set_first_model) ) {
      results[ "errors" ] += "read_pdb_hydro failed\n";
      return false;
   }

   QString ssbond_data;

   SS_apply( model_vector[0], ssbond_data );
   if ( !noticemsg.isEmpty() ) {
      results[ "notices" ] += noticemsg;
   }
   if ( !errormsg.isEmpty() ) {
      results[ "errors" ]  += errormsg;
   }
   results[ "ssbonds" ] = ssbond_data;
   if ( parameters.count( "output" ) ) {
      QString error;
      if ( !US_File_Util::putcontents( parameters[ "output" ], ssbond_data, error ) ) {
         results[ "errors" ]  += error;
      }
   }         

   return true;
}

void US_Saxs_Util::SS_setup() {
#if defined( DEBUG_SS )
   TSE << "SS_setup()\n";
#endif
   cystine_residues.clear();
   cystine_residues.insert( "CYS" );
   cystine_residues.insert( "CYH" );
   
   sulfur_atoms.clear();
   sulfur_atoms.insert( "SG" );
   sulfur_atoms.insert( "S" );
}

void US_Saxs_Util::SS_init() {
#if defined( DEBUG_SS )
   TSE << "SS_init()\n";
#endif
   sulfur_pdb_line             .clear();
   sulfur_coordinates          .clear();
   sulfur_paired               .clear();
   sulfur_pdb_chain_atom_idx   .clear();
   sulfur_pdb_chain_idx        .clear();
}

void US_Saxs_Util::SS_apply( struct PDB_model & model, QString & ssbond_data ) {
   int sulfurs = (int) sulfur_pdb_line.size();
   //   TSE << "SS_apply()\n";

   model.num_SS_bonds = 0;
   model.num_SH_free  = 0;

   if ( !sulfurs ) {
      TSE << "SS_apply() no sulfurs\n";
      return;
   }
   if ( !pdb_parse.find_sh ) {
      TSE << "SS_apply() no pdb_parse.find_sh\n";
      SS_init();
      return;
   }

#if defined( DEBUG_SS )
   QTextStream( stdout ) << "SS_apply()" << Qt::endl;
#endif

   if ( !gparams.count( "thresh_SS" ) ) {
      errormsg += "internal error: no disulfide distance threshold defined\n";
      TSE << "no thresh_SS\n";
      return;
   }

   map < int, double > pair_distance;

   double thresh_SS = gparams[ "thresh_SS" ].toDouble();
#if defined( DEBUG_SS )
   noticemsg += QString( "Checking disulfide distance, current distance threshold %1 [A]" ).arg( thresh_SS );
#endif

/* SS pdb format format https://www.rbvi.ucsf.edu/chimera/docs/UsersGuide/tutorials/pdbintro.html
SSBOND   1 CYS A   28    CYS D   28                          1555   1555  2.05 
*/
   
   for ( int i = 0; i < sulfurs; ++i ) {
      if ( !sulfur_paired.count( i ) ) {
#if defined( DEBUG_SS )
         QTextStream( stdout )
            << "sulfur pdb line   " <<  sulfur_pdb_line[ i ] << Qt::endl
            << "sulfur coordinates"
            <<  sulfur_coordinates[ i ].axis[ 0 ] << " , "
            <<  sulfur_coordinates[ i ].axis[ 1 ] << " , "
            <<  sulfur_coordinates[ i ].axis[ 2 ]
            << Qt::endl;
#endif
         for ( int j = i + 1; j < sulfurs; ++j ) {
            if ( !sulfur_paired.count( j ) ) {
               double this_distance =  dist( sulfur_coordinates[ i ], sulfur_coordinates[ j ] );
               if ( this_distance <= thresh_SS ) {
                  ++model.num_SS_bonds;
                  pair_distance[ j ] = pair_distance[ i ] = this_distance;
                  sulfur_paired[ i ] = j;
                  sulfur_paired[ j ] = i;
#if defined( DEBUG_SS )
                  noticemsg += QString( "SS bond found %1 %2 distance %3 [A]" )
                              .arg( sulfur_pdb_line[ i ].left( 26 ) )
                              .arg( sulfur_pdb_line[ j ].left( 26 ) )
                              .arg( this_distance );
#endif
                  {
                     QString pdb_ssbond =
                        QString( "SSBOND %1 %2 %3    %4 %5                          1555   1555 %6\n" )
                        .arg( model.num_SS_bonds, 3 )
                        .arg( sulfur_pdb_line[ i ].mid( 17, 5 ) )
                        .arg( sulfur_pdb_line[ i ].mid( 22, 4 ) )
                        .arg( sulfur_pdb_line[ j ].mid( 17, 5 ) )
                        .arg( sulfur_pdb_line[ j ].mid( 22, 4 ) )
                        .arg( this_distance, 5, 'f', 2 )
                        ;
                     // QTextStream(stdout) << pdb_ssbond;
                     ssbond_data += pdb_ssbond;
                  }
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
            << "sulfurs paired: distance [A]:" << pair_distance[ it->first ] << Qt::endl
            << sulfur_pdb_line[ it->first ] << Qt::endl
            << sulfur_pdb_line[ it->second ] << Qt::endl
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
            << "sulfurs unpaired:" << Qt::endl
            << sulfur_pdb_line[ i ] << Qt::endl
            ;
#endif
         SS_change_residue( model, sulfur_pdb_line[ i ], "CYH" );
         model.num_SH_free++;
#if defined( DEBUG_SS )
         noticemsg += QString( "SH found, CYS converted to CYH %1" )
                     .arg( sulfur_pdb_line[ i ].left( 26 ) )
            ;
#endif
      }
   }

   SS_init(); // clear after application
}

void US_Saxs_Util::SS_change_residue( struct PDB_model & model, const QString & line, const QString target_residue ) {
#if defined( DEBUG_SS )
   QTextStream( stdout ) << "SS_change_residue()" << Qt::endl;
#endif
   QString source_residue   = line.mid( 17, 3 ).trimmed();
   QString residue_sequence = line.mid( 22, 5 ).trimmed();
   QString chain_id         = line.mid( 20, 2 ).trimmed();

   if ( !multi_residue_map.count( target_residue ) || !multi_residue_map[ target_residue ].size() ) {
      errormsg += QString( us_tr( "Residue file is missing residue name %1, can not covert residue %2 %3 chain %4" ) )
         .arg( target_residue )
         .arg( residue_sequence )
         .arg( source_residue )
         .arg( chain_id )
         ;
      return;
   }

#if defined( DEBUG_SS )
   QTextStream( stdout )
      << "SS_change_residue():"
      << "source_residue   '" << source_residue   << "'" << Qt::endl
      << "target_residue   '" << target_residue   << "'" << Qt::endl
      << "residue_sequence '" << residue_sequence << "'" << Qt::endl
      << "chain_id         '" << chain_id         << "'" << Qt::endl
      << Qt::endl;
#endif

   if ( source_residue == target_residue ) {
      // nothing to do
#if defined( DEBUG_SS )
      QTextStream( stdout ) << "SS_change_residue(): nothing to do" << Qt::endl;
#endif
      return;
   }

   struct residue new_residue = residue_list[ multi_residue_map[ target_residue ][ 0 ] ];
   
   // find matching model_vector molecule(chain) atoms

   if ( !sulfur_pdb_chain_idx.count( chain_id ) || !sulfur_pdb_chain_idx[ chain_id ].size() ) {
      QTextStream( stdout ) << "SS_change_residue(): Internal error chain index vector missing or empty chain_id " << chain_id << Qt::endl;
      return;
   }
   if (
       !sulfur_pdb_chain_atom_idx.count( chain_id ) ||
       !sulfur_pdb_chain_atom_idx[ chain_id ].count( residue_sequence ) ||
       !sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ].size() 
       ) {
      QTextStream( stdout ) << "SS_change_residue(): Internal error chain index vector missing or empty chain_id " << chain_id << " residue_sequence " << residue_sequence << Qt::endl;
      return;
   }

   unsigned int update_model_residue_pos = 0;
   
#if defined( DEBUG_SS )
   {
      unsigned int min_atom_idx = sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ][ 0 ];
      unsigned int max_atom_idx = sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ].back();

      QTextStream( stdout )
         << "SS_change_residue(): structure info" << Qt::endl
         << "  sulfur_pdb_chain_atom_idx[ " << chain_id << " ][ 0 ]   = " << min_atom_idx << Qt::endl
         << "  sulfur_pdb_chain_atom_idx[ " << chain_id << " ].back() = " << max_atom_idx << Qt::endl
         << "  sulfur_pdb_chain_idx     [ " << chain_id << " ].size() = " << sulfur_pdb_chain_idx[ chain_id ].size() << Qt::endl
         ;
      for ( int i = 0; i < (int) sulfur_pdb_chain_idx[ chain_id ].size(); ++i ) {
         QTextStream( stdout )
            << "    sulfur_pdb_chain_idx[ " << chain_id << " ][ " << i << " ] = " << sulfur_pdb_chain_idx[ chain_id ][ i ] << Qt::endl;
      }
      for ( int i = 0; i < (int) sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ].size(); ++i ) {
         QTextStream( stdout )
            << "    sulfur_pdb_chain_atom_idx[ " << chain_id << " ][ " << residue_sequence << " ][ " << i << " ] = " << sulfur_pdb_chain_atom_idx[ chain_id ][ residue_sequence ][ i ] << Qt::endl;
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
                                           << Qt::endl
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
                  << "not found ?" << Qt::endl
                  << "model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom.size() = " << model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom.size() << Qt::endl
                  << "model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ min_atom_idx ].resSeq = " << model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ min_atom_idx ].resSeq << Qt::endl
                  << "model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ max_atom_idx ].resSeq = " << model.molecule[ sulfur_pdb_chain_idx[ chain_id ][ i ] ].atom[ max_atom_idx ].resSeq << Qt::endl
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
         QTextStream( stdout ) << "SS_change_residue(): Internal error could not find match in model chain_id " << chain_id << " residue_sequence " << residue_sequence << Qt::endl;
         return;
      }      
   }

   // search through the model and replace those that match the chain, source_residue & residue sequence
   // once for the residue replace model_vector residue[ model_vector molecule(chain) model_residue_pos ] with new_residue 
}

