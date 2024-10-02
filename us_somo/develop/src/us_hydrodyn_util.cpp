// us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o
// us_hydrodyn_info.cpp contains code to report structures for debugging
// (this) us_hydrodyn_util.cpp contains other various code, such as disulfide code

#include "us_hydrodyn.h"
#define TSO QTextStream(stdout)

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

void US_Hydrodyn::SS_apply( struct PDB_model & model, const QString & ssbondfile ) {
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
   QTextStream( stdout ) << "SS_apply()" << Qt::endl;
#endif

   if ( !gparams.count( "thresh_SS" ) ) {
      editor_msg( "red", us_tr( "internal error: no disulfide distance threshold defined" ) );
      return;
   }

   map < int, double > pair_distance;

   double thresh_SS = gparams[ "thresh_SS" ].toDouble();
   editor_msg( "dark blue", QString( "Checking disulfide distance, current distance threshold %1 [A]" ).arg( thresh_SS ) );

/* SS pdb format format https://www.rbvi.ucsf.edu/chimera/docs/UsersGuide/tutorials/pdbintro.html
SSBOND   1 CYS A   28    CYS D   28                          1555   1555  2.05 
*/
   
   QString ssbond_data;

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
                  editor_msg( "dark blue", QString( "SS bond found %1 %2 distance %3 [A]" )
                              .arg( sulfur_pdb_line[ i ].left( 26 ) )
                              .arg( sulfur_pdb_line[ j ].left( 26 ) )
                              .arg( this_distance ) );
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

   // disabled file output for now
   if ( advanced_config.expert_mode && misc.export_ssbond && !ssbond_data.isEmpty() ) {
      QString error;
      QString fname = somo_tmp_dir + "/" + ssbondfile + ".ssbond.txt";
      editor_msg( "dark blue", QString( "SSBONDs in PDB format written to %1" ).arg( fname ) );
      US_File_Util::putcontents( fname, ssbond_data, error );
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
         editor_msg( "dark blue", QString( "SH found, CYS converted to CYH %1" )
                     .arg( sulfur_pdb_line[ i ].left( 26 ) )
                     );
      }
   }

   SS_init(); // clear after application
}

void US_Hydrodyn::SS_change_residue( struct PDB_model & model, const QString & line, const QString target_residue ) {
#if defined( DEBUG_SS )
   QTextStream( stdout ) << "SS_change_residue()" << Qt::endl;
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
         if ( model.molecule[ j ].atom[ k ].p_residue->name == "WAT" ) {
            continue;
         }
              
         // TSO <<
         //    QString( "protons at pH p_residue name %1 p_atom name %2\n" )
         //    .arg( model.molecule[ j ].atom[ k ].p_residue->name )
         //    .arg( model.molecule[ j ].atom[ k ].p_atom->name )
         //    ;
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
   // QTextStream( stdout ) << "US_Hydrodyn::compute_isoelectric_point() start" << Qt::endl;
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
   //    // QTextStream( stdout ) << protons_at_pH( pH, model ) << " protons at pH " << pH << Qt::endl;
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
   //    << Qt::endl
   //    ;
   
   if ( !multi_residue_map.count( resName ) ||
        !multi_residue_map[ resName ].size() ) {
      QTextStream( stderr ) 
         << "US_Hydrodyn::first_residue_atom_map() resName " << resName
         << " MISSING from multi_residue_map!"
         << Qt::endl
         ;
      return result;
   }

   if ( multi_residue_map[ resName ].size() > 1 ) {
      QTextStream( stderr ) 
         << "US_Hydrodyn::first_residue_atom_map() resName " << resName
         << " WARNING more than one entry in multi_residue_map using first one!"
         << Qt::endl
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
                  << Qt::endl
                  ;
            }
            result[ chain.atom[ i ].name ] = chain.atom[ i ].p_atom;
         }
      } else {
         QTextStream( stderr ) << "US_Hydrodyn::first_residue_atom_map() p_atom not set for atom " << chain.atom[ i ].name << Qt::endl;
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
         QTextStream( stderr ) << "US_Hydrodyn::last_residue_atom_map() p_atom not set for atom " << chain.atom[ i ].name << Qt::endl;
      }
   }
   return result;
}

static quint64 dir_size(const QString & str) {
   quint64 sizex = 0;
   QFileInfo str_info(str);
   if (str_info.isDir()) {
      QDir dir(str);
      QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs |  QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
      for (int i = 0; i < list.size(); ++i) {
         QFileInfo fileInfo = list.at(i);
         if(fileInfo.isDir()){
               sizex += dir_size(fileInfo.absoluteFilePath());
         } else {
               sizex += fileInfo.size();
         }
      }
   }
   return sizex;
}
   
void US_Hydrodyn::clear_temp_dirs() {
   // somo_tmp_dir globally defined
   if ( gui_script ) {
      return;
   }

   QString somo_saxs_tmp_dir = somo_dir + QDir::separator() + "saxs" + QDir::separator() + "tmp";

   quint64 somo_tmp_size = dir_size( somo_tmp_dir ) / (1024 * 1024);
   quint64 saxs_tmp_size = dir_size( somo_saxs_tmp_dir ) / (1024 * 1024);
   quint64 total_size    = somo_tmp_size + saxs_tmp_size;

   TSO <<
      QString(

              "clear_temp_dirs()\n"
              "%1 : %2 M\n"
              "%3 : %4 M\n"
              )
      .arg( somo_tmp_dir )
      .arg( somo_tmp_size )
      .arg( somo_saxs_tmp_dir )
      .arg( saxs_tmp_size )
      ;

   if ( total_size >= (quint64) advanced_config.temp_dir_threshold_mb ) {
      switch (
              QMessageBox::question(
                                    this,
                                    windowTitle() + us_tr(": Clear Temporary Directories"),
                                    QString(
                                            us_tr(
                                                  "Clear temporary directories using approximately %1 MB of disk space?\n"
                                                  "Do not do this if you have other SOMOs currently processing" 
                                                  ) ).arg( total_size ),
                                    QMessageBox::Yes, 
                                    QMessageBox::No,
                                    QMessageBox::Cancel
                                    ) )
      {
      case QMessageBox::Cancel :
         return;
         break;
      case QMessageBox::Yes : 
         break;
      case QMessageBox::No : 
         return;
         break;
      default :
         return;
         break;
      }
   } else {
      return;
   }

   QDir qd_somo_tmp_dir( somo_tmp_dir );
   QDir qd_saxs_tmp_dir( somo_saxs_tmp_dir );

   TSO << "will clear directories\n";
   TSO << QString( "somo tmp dir name %1\n" ).arg( qd_somo_tmp_dir.path() );
   TSO << QString( "saxs tmp dir name %1\n" ).arg( qd_saxs_tmp_dir.path() );

   qd_somo_tmp_dir.removeRecursively();
   qd_saxs_tmp_dir.removeRecursively();
   TSO << QString( "somo tmp dir name %1\n" ).arg( qd_somo_tmp_dir.path() );
   TSO << QString( "saxs tmp dir name %1\n" ).arg( qd_saxs_tmp_dir.path() );
   qd_somo_tmp_dir.mkdir( qd_somo_tmp_dir.path() );
   qd_saxs_tmp_dir.mkdir( qd_saxs_tmp_dir.path() );
}

QString US_Hydrodyn::gparam_value( const QString & param ) {
   return
      gparams.count( param )
      ? gparams[ param ]
      : ""
      ;
}
