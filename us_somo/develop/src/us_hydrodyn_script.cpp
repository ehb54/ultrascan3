// us_hydrodyn.cpp contains class creation & gui connected functions
#include <QRegularExpression>
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o
// us_hydrodyn_info.cpp contains code to report structures for debugging
// us_hydrodyn_util.cpp contains other various code, such as disulfide code
// (this) us_hydrodyn_load.cpp contains code to load files 

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_perceive_somo.h"
#include "../include/us_write_config.h"

#define TSE QTextStream( stderr )
#define TSO QTextStream( stdout )
#define LDB "------------------------------------------------------------\n"
#define LDE "============================================================\n"

void US_Hydrodyn::gui_script_msg( int line, QString cmd, QString msg ) {
   TSO << QString( "US_Hydrodyn::gui_script: %1 : %2 : %3\n" ).arg( line + 1).arg( cmd ).arg( msg );
}

void US_Hydrodyn::gui_script_error( int line, QString cmd, QString msg, bool doexit ) {
   gui_script_msg( line, cmd, "ERROR: " + msg );
   if ( doexit ) {
      gui_script_exit( -1 );
   }
}

// every gui_script exit comes through here.  a script leaves by exit(), never through
// closeEvent(), so the RasMol viewers it opened - auto_view_pdb is on unless the script says
// "norasmol" - would just be orphaned.  close them, unless the script asked otherwise.

void US_Hydrodyn::gui_script_exit( int rc ) {
   QList < qint64 > open_rasmols = rasmol_running_pids();

   if ( open_rasmols.size() ) {
      if ( gui_script_rasmol_leave_open ) {
         TSO << QString( "US_Hydrodyn::gui_script: leaving %1 RasMol window(s) open\n" ).arg( open_rasmols.size() );
      } else {
         TSO << QString( "US_Hydrodyn::gui_script: closing %1 RasMol window(s)\n" ).arg( open_rasmols.size() );
         rasmol_close_all();
      }
   }

   exit( rc );
}

void US_Hydrodyn::gui_script_run() {
   TSO << "US_Hydrodyn::gui_script_run()\n";

   guiFlag = false;

   QString script;
   {
      QString error;
      if ( !US_File_Util::getcontents( gui_script_file, script, error ) ) {
         TSE << "US_Hydrodyn::gui_script_run() : Error " << error << "\n";
         gui_script_exit( -1 );
      }
   }

   QStringList scriptlines = script.split( "\n" );
   int scriptlinesc        = (int) scriptlines.count();

   QRegularExpression rx_comment( "#.*$" );
   QRegularExpression rx_empty("^\\s*$");
   
   for ( int i = 0; i < scriptlinesc; ++i ) {
      QString l = scriptlines[ i ];
      TSO << LDE << QString( "somo> %1\n" ).arg( l );
      if ( l.contains( rx_comment )
           || l.contains( rx_empty ) ) {
         continue;
      }
           
      QStringList ls = l.split( QRegularExpression( QStringLiteral( "\\s+" ) ) );
      int lsc = (int)ls.count();
      if ( !lsc ) {
         // blank line?
         continue;
      }

      QString cmd = ls.front(); ls.pop_front();
      
      if ( cmd == "exit" ) {
         gui_script_msg( i, cmd, "exiting normally" );
         gui_script_exit( 0 );
      } else if ( cmd == "expert" ) {
         advanced_config.expert_mode = true;
      } else if ( cmd == "norasmol" ) {
         advanced_config.auto_view_pdb = false;
      } else if ( cmd == "rasmol" ) {
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         gui_script_msg( i, cmd, opt1 );
         if ( opt1 == "leaveopen" ) {
            // leave any viewers the script opened running after it exits
            gui_script_rasmol_leave_open = true;
         } else if ( opt1 == "close" ) {
            gui_script_rasmol_leave_open = false;   // the default
         } else {
            gui_script_error( i, cmd, "unknown option : " + opt1 );
         }
      } else if ( cmd == "progress" ) {
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         cli_progress = true;
         progress->set_cli_prefix( opt1 );
      } else if ( cmd == "config" ) {
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         if ( read_config( opt1 ) ) {
            gui_script_error( i, cmd, QString( "config file '%1' failed" ).arg( opt1 ) );
         }            
      } else if ( cmd == "saveparams" ) {
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         if ( opt1 == "init" ) {
            save_params.field.clear();
         } else {
            save_params.field.push_back( opt1 );
         }
      } else if ( cmd == "global" ) {
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt2 = ls.front(); ls.pop_front();
         gparams[ opt1 ] = opt2;
      } else if ( cmd == "threads" ) {
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         USglobal->config_list.numThreads = opt1.toUInt();
         US_Write_Config *WConfig;
         WConfig = new US_Write_Config();
         bool result = WConfig->write_config(USglobal->config_list);
         gui_script_msg( i, cmd, QString("Threads now %1 write config result %2\n").arg( opt1 ).arg(result ? "ok" : "not ok") );
      } else if ( cmd == "show" ) {
         show();
         if ( batch_widget ) {
            batch_window->show();
         }
         if ( saxs_plot_widget ) {
            saxs_plot_window->show();
         }
      } else if ( cmd == "hide" ) {
         hide();
         if ( batch_widget ) {
            batch_window->hide();
         }
         if ( saxs_plot_widget ) {
            saxs_plot_window->hide();
         }
      } else if ( cmd == "somo" ) {
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         gui_script_msg( i, cmd, opt1 );
         if ( opt1 == "overwrite" ) {
            cb_overwrite->setChecked( true );
            set_overwrite();
         } else if ( opt1 == "saveloadstats" ) {
            TSO << "yes, saveloadstats\n";
            if ( ls.isEmpty() ) {
               gui_script_error( i, cmd + " " + opt1, "missing argument" );
            }
            QString opt2 = ls.front(); ls.pop_front();
            TSO << "yes, opt2 " + opt2 + "\n";
            if ( !model_summary_csv( opt2 ) ) {
               gui_script_error( i, cmd + " " + opt1 + " " + opt2, "failed" );
            }               
            TSO << "model_summary_csv returned ok\n";
         } else {
            gui_script_error( i, cmd, "unknown option : " + opt1 );
         }
      } else if ( cmd == "perceive" ) {
         // Perceive hybridization / vdW radius / electron count for residues somo.residue does not
         // code, and print a tentative entry for each. See ehb54/ultrascan-tickets#978.
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument: <pdbfile>" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         // "perceive compare <pdb>" diffs perception against somo.residue for residues it DOES
         // code -- a hand-testing aid. Plain "perceive <pdb>" proposes entries for the ones it does not.
         // "perceive auto <pdb>" is the headless path: every default accepted, nothing prompts,
         // nothing is written back to somo.residue. Same output as plain "perceive", which is
         // already non-interactive -- the sub-command exists so a pipeline states the intent,
         // and so the defaults below can be pinned per run.
         bool perceive_auto = ( opt1 == "auto" );
         if ( perceive_auto ) {
            if ( ls.isEmpty() ) {
               gui_script_error( i, cmd, "missing argument: perceive auto <pdbfile>" );
            }
            opt1 = ls.front(); ls.pop_front();
         }
         // "perceive validate <pdb>" rebuilds the residues somo.residue DOES code, pretending
         // they are unknown, and reports computed vbar/molvol/hydration against the stored
         // values. End-to-end including perception, so it is the measure any accuracy claim
         // has to rest on.
         bool perceive_validate = ( opt1 == "validate" );
         if ( perceive_validate ) {
            if ( ls.isEmpty() ) {
               gui_script_error( i, cmd, "missing argument: perceive validate <pdbfile>" );
            }
            opt1 = ls.front(); ls.pop_front();
         }
         // "perceive apply <pdb>" is the headless equivalent of accepting every entry in the GUI
         // review dialog: perceive, then put the entries into the tables SOMO builds from and
         // re-read the structure, so the residues stop being non-coded. It reports the non-coded
         // count before and after, which is the assertion worth making -- the GUI path failed for
         // a long time precisely because nothing checked that accepting changed anything.
         // The user's somo.residue is not modified; the entries go to the session overlay.
         // "perceive build <pdb>" is "apply" plus the thing the GUI user does next: build the
         // bead model. It exists because apply alone does NOT reach the code that resolves atoms
         // against somo.atom -- a clean load takes calc_mw(), not the excluded-volume path -- so
         // the atom-table fallback, and anything else that only bites at bead-build time, was
         // exercised solely by hand in the GUI. This is the headless equivalent of load, perceive,
         // accept, build.
         // An optional method token selects which bead model to build, matching the GUI buttons:
         //   somo    (default)  pb_somo       calc_somo()          SOMO, overlaps removed
         //   somo_o             pb_somo_o     calc_somo_o()        SOMO, overlaps kept
         //   vdw                pb_vdw_beads  calc_vdw_beads()     vdW beads (needs ATOMIC hydration)
         //   grid | a2b         pb_grid_pdb   calc_grid_pdb()      AtoB grid
         // They exercise different amounts of the model-building code, so a perceived residue that
         // satisfies one can still break another -- vdW in particular reads per-atom hydration,
         // which the perceiver only proposes.
         bool perceive_build = ( opt1 == "build" );
         QString build_method = "somo";
         if ( perceive_build ) {
            if ( ls.isEmpty() ) {
               gui_script_error( i, cmd, "missing argument: perceive build [somo|somo_o|vdw|grid] <pdbfile>" );
            }
            opt1 = ls.front(); ls.pop_front();
            if ( opt1 == "somo" || opt1 == "somo_o" || opt1 == "vdw"
                 || opt1 == "grid" || opt1 == "a2b" ) {
               build_method = ( opt1 == "a2b" ) ? "grid" : opt1;
               if ( ls.isEmpty() ) {
                  gui_script_error( i, cmd, "missing argument: perceive build " + opt1 + " <pdbfile>" );
               }
               opt1 = ls.front(); ls.pop_front();
            }
         }
         bool perceive_apply = perceive_build || ( opt1 == "apply" );
         if ( opt1 == "apply" ) {
            if ( ls.isEmpty() ) {
               gui_script_error( i, cmd, "missing argument: perceive apply <pdbfile>" );
            }
            opt1 = ls.front(); ls.pop_front();
         }
         bool perceive_compare = ( opt1 == "compare" );
         if ( perceive_compare ) {
            if ( ls.isEmpty() ) {
               gui_script_error( i, cmd, "missing argument: perceive compare <pdbfile>" );
            }
            opt1 = ls.front(); ls.pop_front();
         }
         gui_script_msg( i, cmd, opt1 );
         if ( !QFile::exists( opt1 ) ) {
            gui_script_error( i, cmd, "file does not exist: " + opt1 );
         }
         if ( !screen_pdb( opt1, false, true ) ) {
            gui_script_error( i, cmd, "could not load pdb: " + opt1 );
         }
         if ( model_vector.empty() ) {
            gui_script_error( i, cmd, "no models loaded from: " + opt1 );
         }
         somo_perceive::HybridTable ptbl;
         if ( !ptbl.load( saxs_options.default_hybrid_filename.toStdString() ) ) {
            gui_script_error( i, cmd, "could not load hybrid table: "
                              + saxs_options.default_hybrid_filename );
         }
         if ( perceive_compare ) {
            // Build resName -> (atomName -> curated hybrid) from SOMO's loaded residue table.
            std::map< QString, std::map< QString, QString > > curated;
            for ( map < QString, vector < int > >::iterator it = multi_residue_map.begin();
                  it != multi_residue_map.end();
                  ++it ) {
               for ( unsigned int ri = 0; ri < it->second.size(); ++ri ) {
                  int idx = it->second[ ri ];
                  if ( idx < 0 || idx >= (int) residue_list.size() ) continue;
                  const struct residue & rr = residue_list[ idx ];
                  for ( unsigned int ai = 0; ai < rr.r_atom.size(); ++ai ) {
                     // first definition wins; multi-pKa variants share the base atom naming
                     if ( !curated[ it->first ].count( rr.r_atom[ ai ].name ) ) {
                        curated[ it->first ][ rr.r_atom[ ai ].name ] = rr.r_atom[ ai ].hybrid.name;
                     }
                  }
               }
            }
            somo_perceive::CompareResult cr =
               somo_perceive::compare_against_table( model_vector[ 0 ], ptbl, curated, opt1 );
            TSO << QString( "\nperceive compare: %1\n" ).arg( opt1 );
            TSO << QString( "  coded residues compared : %1\n" ).arg( cr.residues );
            TSO << QString( "  atoms scored            : %1\n" ).arg( cr.scored );
            TSO << QString( "  exact hybrid match      : %1 (%2%)\n" )
               .arg( cr.exact )
               .arg( cr.scored ? 100.0 * cr.exact / cr.scored : 0, 0, 'f', 2 );
            TSO << QString( "  physics match           : %1 (%2%)   [mw/radius/electrons identical]\n" )
               .arg( cr.phys )
               .arg( cr.scored ? 100.0 * cr.phys / cr.scored : 0, 0, 'f', 2 );
            if ( !cr.by_pair.empty() ) {
               TSO << "\n  differences (curated -> perceived):\n";
               for ( std::map<QString,long>::const_iterator pit = cr.by_pair.begin();
                     pit != cr.by_pair.end();
                     ++pit ) {
                  TSO << QString( "   %1  %2\n" ).arg( pit->second, 6 ).arg( pit->first );
               }
               TSO << "\n  first differing atoms:\n";
               for ( int m = 0; m < cr.mismatches.size() && m < 25; ++m ) {
                  TSO << QString( "     %1 %2 : curated %3, perceived %4\n" )
                     .arg( cr.mismatches[ m ].res, -6 )
                     .arg( cr.mismatches[ m ].atom, -5 )
                     .arg( cr.mismatches[ m ].expected, -7 )
                     .arg( cr.mismatches[ m ].got );
               }
            } else {
               TSO << "\n  perception agrees with somo.residue on every compared atom.\n";
            }
            continue;
         }

         // somo.residue is the master: only residues it could NOT code are perceived, and a coded
         // residue whose instance merely failed to match is reported rather than perceived. See
         // US_Hydrodyn::select_perceivable().
         std::set< QString > to_perceive;
         QStringList unmatched;
         select_perceivable( to_perceive, unmatched );
         if ( !unmatched.isEmpty() ) {
            TSO << QString( "perceive: %1 residue type(s) ARE coded but their instances do not "
                            "match the table (missing or unexpected atoms), NOT perceived: %2\n"
                            "perceive: complete or repair the structure for these\n" )
               .arg( unmatched.size() ).arg( unmatched.join( ", " ) );
         }
         // Defaults a pipeline can pin, same pattern as gparams["covolume"].
         somo_residue_builder::Options pb_opt;
         {
            auto gp = [ & ]( const QString & k, double d ) {
               return gparams.count( k ) ? gparams[ k ].toDouble() : d;
            };
            auto gb = [ & ]( const QString & k, bool d ) {
               return gparams.count( k ) ? ( gparams[ k ] == "true" ) : d;
            };
            pb_opt.compute_psv       = gb( "perceive_psv",       true );
            pb_opt.compute_volume    = gb( "perceive_volume",    true );
            pb_opt.compute_hydration = gb( "perceive_hydration", true );
            pb_opt.volume_probe      = gp( "perceive_volume_probe", pb_opt.volume_probe );
            pb_opt.volume_grid       = gp( "perceive_volume_grid",  pb_opt.volume_grid );
            pb_opt.bead_color        = (int) gp( "perceive_bead_color", pb_opt.bead_color );
            if ( !somo_perceive::bead_color_is_selectable( pb_opt.bead_color ) ) {
               // 0/6/7/8 are reserved, and 0 and 6 exclude a bead from the hydrodynamics
               // entirely -- refuse rather than silently produce a model missing beads.
               gui_script_error( i, cmd, QString( "perceive_bead_color %1 is reserved or out of "
                                                  "range; pick a selectable colour" )
                                 .arg( pb_opt.bead_color ) );
            }
         }
         somo_hydration::Table pb_hyd =
            somo_perceive::hydration_from_residue_list( residue_list );

         if ( perceive_validate ) {
            somo_perceive::ValidateResult vr =
               somo_perceive::validate_against_table( model_vector[ 0 ], ptbl, residue_list,
                                                      &pb_hyd, pb_opt, opt1 );
            TSO << QString( "\nperceive validate: %1 coded residue instance(s) rebuilt, "
                            "%2 skipped as chain termini, %3 skipped as incomplete\n" )
               .arg( vr.instances ).arg( vr.skipped_terminal ).arg( vr.skipped_incomplete );
            TSO << QString( "%1 %2 %3 %4 %5 %6 %7 %8 %9\n" )
               .arg( "res", -6 ).arg( "n", 4 )
               .arg( "vbar", 8 ).arg( "stored", 8 ).arg( "d%", 7 )
               .arg( "molvol", 9 ).arg( "stored", 9 ).arg( "d%", 7 ).arg( "  hydration c/s" );
            double sv = 0, sm = 0; int nv = 0, nm = 0, nh_ok = 0, nh = 0;
            for ( int r = 0; r < vr.rows.size(); ++r ) {
               const somo_perceive::ValidateRow & w = vr.rows[ r ];
               const double dv = w.vbar_stored   > 0 ? 100.0 * ( w.vbar_computed   - w.vbar_stored   ) / w.vbar_stored   : 0.0;
               const double dm = w.molvol_stored > 0 ? 100.0 * ( w.molvol_computed - w.molvol_stored ) / w.molvol_stored : 0.0;
               if ( w.vbar_stored   > 0 ) { sv += qAbs( dv ); ++nv; }
               if ( w.molvol_stored > 0 ) { sm += qAbs( dm ); ++nm; }
               ++nh;
               if ( qAbs( w.hydration_computed - w.hydration_stored ) < 0.51 ) ++nh_ok;
               TSO << QString( "%1 %2 %3 %4 %5 %6 %7 %8   %9 / %10\n" )
                  .arg( w.resName, -6 ).arg( w.instances, 4 )
                  .arg( w.vbar_computed, 8, 'f', 3 ).arg( w.vbar_stored, 8, 'f', 3 )
                  .arg( dv, 7, 'f', 1 )
                  .arg( w.molvol_computed, 9, 'f', 1 ).arg( w.molvol_stored, 9, 'f', 1 )
                  .arg( dm, 7, 'f', 1 )
                  .arg( w.hydration_computed, 0, 'f', 1 ).arg( w.hydration_stored, 0, 'f', 1 );
            }
            TSO << QString( "\nSUMMARY  vbar: mean |error| %1%% over %2 type(s)   "
                            "molvol: mean |error| %3%% over %4 type(s)   "
                            "hydration: %5/%6 within 0.5 water\n" )
               .arg( nv ? sv / nv : 0.0, 0, 'f', 2 ).arg( nv )
               .arg( nm ? sm / nm : 0.0, 0, 'f', 2 ).arg( nm )
               .arg( nh_ok ).arg( nh );
            continue;
         }

         QList< somo_perceive::Tentative > tents =
            somo_perceive::perceive_unknown( model_vector[ 0 ], ptbl, to_perceive, opt1,
                                             &pb_hyd, pb_opt );
         unsigned int perceive_atom_count = 0;
         for ( unsigned int pc = 0; pc < model_vector[ 0 ].molecule.size(); ++pc ) {
            perceive_atom_count += (unsigned int) model_vector[ 0 ].molecule[ pc ].atom.size();
         }
         TSO << QString( "perceive: %1 model atom(s) in %4 chain(s), %2 non-coded residue instance(s), "
                         "%3 non-coded residue type(s) perceived\n" )
            .arg( perceive_atom_count )
            .arg( (unsigned int) to_perceive.size() )
            .arg( tents.size() )
            .arg( (unsigned int) model_vector[ 0 ].molecule.size() );
         for ( int t = 0; t < tents.size(); ++t ) {
            const somo_perceive::Tentative & tv = tents[ t ];
            TSO << QString( "\n===== tentative somo.residue entry: %1 "
                            "(%2 atoms, %3 instance(s) in model, %4 atom(s) flagged for review) =====\n" )
               .arg( tv.resName ).arg( tv.atoms ).arg( tv.instances ).arg( tv.flagged );
            if ( perceive_auto && tv.flagged ) {
               TSO << QString( "note: %1 atom(s) flagged -- accepted anyway because this is "
                               "'perceive auto'; review the REVIEW block before use\n" )
                  .arg( tv.flagged );
            }
            TSO << QString( "computed: vbar %1 cm^3/g, molvol %2 A^3, proposed hydration %3 waters\n" )
               .arg( tv.vbar, 0, 'f', 3 ).arg( tv.molvol, 0, 'f', 2 ).arg( tv.hydration, 0, 'f', 1 );
            TSO << tv.block;
            if ( !tv.new_hybrids.isEmpty() ) {
               TSO << QString( "----- new somo.hybrid rows (types not already in the table) -----\n" );
               for ( int h = 0; h < tv.new_hybrids.size(); ++h ) {
                  TSO << tv.new_hybrids[ h ] << "\n";
               }
            }
         }
         if ( tents.isEmpty() ) {
            TSO << "perceive: every residue in this structure is already coded in somo.residue\n";
         }
         if ( perceive_apply && !tents.isEmpty() ) {
            QStringList blocks;
            QStringList new_hybrids;
            for ( int t = 0; t < tents.size(); ++t ) {
               blocks      << tents[ t ].block;
               new_hybrids += tents[ t ].new_hybrids;
            }
            if ( !apply_perceived_entries( blocks, new_hybrids, false ) ) {
               gui_script_error( i, cmd, "could not apply the perceived entries" );
            }
            // Did the entries actually land in the table SOMO matches against? This is a
            // separate question from whether the file was written, and the two came apart once
            // already: the file was fine and the records in it were not parseable.
            for ( int t = 0; t < tents.size(); ++t ) {
               TSO << QString( "perceive apply: %1 %2 in the residue table (%3 residue(s) loaded "
                               "from %4)\n" )
                  .arg( tents[ t ].resName )
                  .arg( multi_residue_map.count( tents[ t ].resName ) ? "IS" : "is NOT" )
                  .arg( (unsigned int) residue_list.size() )
                  .arg( residue_filename );
            }
            // apply_perceived_entries() re-read the structure, so this is the state the bead
            // builder would now see. Anything still listed here did NOT take effect.
            std::set< QString > still_present;
            for ( unsigned int pm = 0; pm < model_vector[ 0 ].molecule.size(); ++pm ) {
               for ( unsigned int pa = 0;
                     pa < model_vector[ 0 ].molecule[ pm ].atom.size(); ++pa ) {
                  still_present.insert( model_vector[ 0 ].molecule[ pm ].atom[ pa ].resName );
               }
            }
            QStringList still_unknown;
            for ( map < QString, bool >::iterator it = unknown_residues.begin();
                  it != unknown_residues.end();
                  ++it ) {
               if ( it->second && still_present.count( it->first ) ) {
                  still_unknown << it->first;
               }
            }
            TSO << QString( "\nperceive apply: %1 entr(y/ies) applied to the session residue "
                            "table, %2 residue type(s) still non-coded%3\n" )
               .arg( blocks.size() )
               .arg( still_unknown.size() )
               .arg( still_unknown.isEmpty()
                     ? QString( " -- the bead builder will use the perceived entries" )
                     : QString( ": %1" ).arg( still_unknown.join( ", " ) ) );
         }

         if ( perceive_build ) {
            // reload_pdb() repopulated the model list and cleared the selection; calc_somo()
            // refuses to run without one, exactly as the button does.
            if ( !lb_model->count() ) {
               gui_script_error( i, cmd, "no models to build after perceive" );
            }
            lb_model->item( 0 )->setSelected( true );
            select_model( 0 );
            TSO << QString( "\nperceive build: building the %1 bead model\n" ).arg( build_method );
            int build_rc = 0;
            if ( build_method == "somo" ) {
               build_rc = calc_somo();
            } else if ( build_method == "somo_o" ) {
               build_rc = calc_somo_o();
            } else if ( build_method == "vdw" ) {
               build_rc = calc_vdw_beads();
            } else {
               build_rc = calc_grid_pdb();
            }
            // Count bead_model, the model just built. NOT bead_models[current_model]: the build
            // walks current_model as its loop variable and leaves it ONE PAST the last model, so
            // right after a build that index is out of range (for a single-model file it reads
            // bead_models[1] of 1). Reading it there reported 0 beads for a build that had plainly
            // succeeded -- rc 0, three overlap-reduction stages, a written bead model.
            const int nbeads = (int) bead_model.size();
            const unsigned int filed_idx = bead_models.size()
               ? qMin( current_model, (unsigned int) bead_models.size() - 1 ) : 0;
            const int nfiled = bead_models.size() ? (int) bead_models[ filed_idx ].size() : 0;
            TSO << QString( "perceive build: %1 rc %2, %3 bead(s) (filed %4 in slot %5 of %6)%7\n" )
               .arg( build_method )
               .arg( build_rc )
               .arg( nbeads )
               .arg( nfiled )
               .arg( filed_idx )
               .arg( (unsigned int) bead_models.size() )
               .arg( build_rc || !nbeads ? "  <-- FAILED" : "" );
            if ( build_rc || !nbeads ) {
               gui_script_error( i, cmd, QString( "%1 bead model build failed after applying the "
                                                  "perceived entries" ).arg( build_method ) );
            }
         }
      } else if ( cmd == "saxs_options" ) {
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         gui_script_msg( i, cmd, opt1 );
         if ( opt1 == "overwrite" ) {
            cb_overwrite->setChecked( true );
            set_overwrite();
         } else if ( opt1 == "iqscaleangstrom" || opt1 == "iqscalenm" ) {
            // controls whether loaded I(q) q values are converted from 1/nm
            saxs_options.iq_scale_angstrom = ( opt1 == "iqscaleangstrom" );
            saxs_options.iq_scale_nm       = !saxs_options.iq_scale_angstrom;
         } else if ( opt1 == "fulldebye" ) {
            saxs_options.saxs_iq_native_debye   = true;
            saxs_options.saxs_iq_native_sh      = false;
            saxs_options.saxs_iq_native_fast    = false;
            saxs_options.saxs_iq_native_hybrid  = false;
            saxs_options.saxs_iq_native_hybrid2 = false;
            saxs_options.saxs_iq_native_hybrid3 = false;
            saxs_options.saxs_iq_foxs           = false;
            saxs_options.saxs_iq_crysol         = false;
            saxs_options.saxs_iq_sastbx         = false;
         } else {
            gui_script_error( i, cmd, "unknown option : " + opt1 );
         }
      } else if ( cmd == "batch" ) {
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         if ( !batch_widget ) {
            gui_script_error( i, cmd, "batch widget not open" );
         }
         gui_script_msg( i, cmd, opt1 );
         if ( opt1 == "selectall" ) {
            batch_window->select_all();
         } else if ( opt1 == "somo_o" ) {
            if ( !batch_window->cb_somo_o->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_somo_o->setChecked( true );
            batch_window->set_somo_o();
         } else if ( opt1 == "vdw" ) {
            if ( !batch_window->cb_vdw_beads->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_vdw_beads->setChecked( true );
            batch_window->set_vdw_beads();
         } else if ( opt1 == "prr" ) {
            if ( !batch_window->cb_prr->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_prr->setChecked( true );
            batch_window->set_prr();
         } else if ( opt1 == "iqq" ) {
            if ( !batch_window->cb_iqq->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_iqq->setChecked( true );
            batch_window->set_iqq();
         } else if ( opt1 == "sascsv" ) {
            if ( !batch_window->cb_csv_saxs->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_csv_saxs->setChecked( true );
            batch_window->set_csv_saxs();
         } else if ( opt1 == "sascsvname" ) {
            if ( ls.isEmpty() ) {
               gui_script_error( i, cmd, "missing argument" );
            }
            QString opt2 = ls.front(); ls.pop_front();
            if ( !batch_window->le_csv_saxs_name->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->le_csv_saxs_name->setText( opt2 );
         } else if ( opt1 == "zeno" ) {
            if ( !batch_window->cb_zeno->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_zeno->setChecked( true );
            batch_window->set_zeno();
         } else if ( opt1 == "grpy" ) {
            if ( !batch_window->cb_grpy->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_grpy->setChecked( true );
            batch_window->set_grpy();
         } else if ( opt1 == "combineh" ) {
            if ( !batch_window->cb_avg_hydro->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_avg_hydro->setChecked( true );
            batch_window->set_avg_hydro();
         } else if ( opt1 == "combinehname" ) {
            if ( ls.isEmpty() ) {
               gui_script_error( i, cmd, "missing argument" );
            }
            QString opt2 = ls.front(); ls.pop_front();
            if ( !batch_window->le_avg_hydro_name->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->le_avg_hydro_name->setText( opt2 );
         } else if ( opt1 == "saveparams" ) {
            if ( !batch_window->cb_saveParams->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->cb_saveParams->setChecked( true );
            batch_window->set_saveParams();
         } else if ( opt1 == "start" ) {
            if ( !batch_window->pb_start->isEnabled() ) {
               gui_script_error( i, cmd, opt1 + ": not enabled" );
            }
            batch_window->start();
         } else if ( opt1 == "overwrite" ) {
            batch_window->overwrite_all = true;
         } else if ( opt1 == "load" ) {
            batch_window->load_somo();
         } else if ( opt1 == "set" ) {
            if ( ls.isEmpty() ) {
               gui_script_error( i, cmd, "missing argument" );
            }
            QString opt2 = ls.front(); ls.pop_front();
            if ( ls.isEmpty() ) {
               gui_script_error( i, cmd, "missing argument" );
            }
            QString opt3 = ls.front(); ls.pop_front();
            if ( opt2 == "residue" ) {
               if ( opt3 == "stop" ) {
                  batch_window->rb_residue_stop->setChecked( true );
                  batch_window->residue();
                  pdb_parse.missing_residues = batch_window->batch->missing_residues;
               } else if ( opt3 == "skip" ) {
                  batch_window->rb_residue_skip->setChecked( true );
                  batch_window->residue();
                  pdb_parse.missing_residues = batch_window->batch->missing_residues;
               } else if ( opt3 == "auto" ) {
                  batch_window->rb_residue_auto->setChecked( true );
                  batch_window->residue();
                  pdb_parse.missing_residues = batch_window->batch->missing_residues;
               } else if ( opt3 == "info" ) {
                  gui_script_msg( i, cmd, QString( "%1" ).arg( batch_window->batch->missing_residues ).arg( pdb_parse.missing_residues ) );;
               } else {
                  gui_script_error( i, cmd, "unknown option : " + opt1 + " " + opt2 + " " + opt3 );
               }
            } else if ( opt2 == "atom" ) {
               if ( opt3 == "stop" ) {
                  batch_window->rb_atom_stop->setChecked( true );
                  batch_window->atom();
                  pdb_parse.missing_atoms = batch_window->batch->missing_atoms;
               } else if ( opt3 == "skip" ) {
                  batch_window->rb_atom_skip->setChecked( true );
                  batch_window->atom();
                  pdb_parse.missing_atoms = batch_window->batch->missing_atoms;
               } else if ( opt3 == "auto" ) {
                  batch_window->rb_atom_auto->setChecked( true );
                  batch_window->atom();
                  pdb_parse.missing_atoms = batch_window->batch->missing_atoms;
               } else if ( opt3 == "info" ) {
                  gui_script_msg( i, cmd, QString( "%1 %2" ).arg( batch_window->batch->missing_atoms ).arg( pdb_parse.missing_atoms ) );
               } else {
                  gui_script_error( i, cmd, "unknown option : " + opt1 + " " + opt2 + " " + opt3 );
               }
            } else {
               gui_script_error( i, cmd, "unknown option : " + opt1 + " " + opt2 );
            }
         } else {
            gui_script_error( i, cmd, "unknown option : " + opt1 );
         }
         qApp->processEvents();
      } else if ( cmd == "dad" ) {
         if ( ls.isEmpty() ) {
            gui_script_error( i, cmd, "missing argument" );
         }
         QString opt1 = ls.front(); ls.pop_front();
         gui_script_msg( i, cmd, opt1 );

         if ( opt1 == "open" ) {
            if ( !saxs_plot_widget ) {
               pdb_saxs( false, false );
            }
            if ( !saxs_plot_widget ) {
               gui_script_error( i, cmd + " " + opt1, "could not open the SAS window" );
            }
            // dad() is a private slot, reach it through the meta-object rather
            // than widening the US_Hydrodyn_Saxs interface for scripting alone
            QMetaObject::invokeMethod( saxs_plot_window, "dad", Qt::DirectConnection );
            if ( !dad_widget ) {
               gui_script_error( i, cmd + " " + opt1, "could not open the UV-Vis window" );
            }
            // every dad command below runs without dialogs
            dad_window->script_mode = true;
         } else {
            if ( !dad_widget ) {
               gui_script_error( i, cmd + " " + opt1, "UV-Vis window not open, use \"dad open\" first" );
            }

            QString errormsg;

            if ( opt1 == "settimes" ) {
               if ( ls.size() < 2 ) {
                  gui_script_error( i, cmd + " " + opt1, "needs <start seconds> <interval seconds>" );
               }
               dad_window->script_start_time_seconds          = ls.front().toDouble(); ls.pop_front();
               dad_window->script_collection_interval_seconds = ls.front().toDouble(); ls.pop_front();
               if ( dad_window->script_collection_interval_seconds <= 0e0 ) {
                  gui_script_error( i, cmd + " " + opt1, "interval seconds must be greater than zero" );
               }
            } else if ( opt1 == "lambdarange" ) {
               if ( ls.size() < 2 ) {
                  gui_script_error( i, cmd + " " + opt1, "needs <start> <end> in nm, or 0 0 for the full spectrum" );
               }
               dad_window->script_lambda_start = ls.front().toDouble(); ls.pop_front();
               dad_window->script_lambda_end   = ls.front().toDouble(); ls.pop_front();
            } else if ( opt1 == "lambdas" ) {
               if ( ls.isEmpty() ) {
                  gui_script_error( i, cmd + " " + opt1, "missing file name" );
               }
               if ( !dad_window->script_load_lambdas( ls.front(), errormsg ) ) {
                  gui_script_error( i, cmd + " " + opt1 + " " + ls.front(), errormsg );
               }
               ls.pop_front();
            } else if ( opt1 == "load" ) {
               if ( ls.isEmpty() ) {
                  gui_script_error( i, cmd + " " + opt1, "missing file name" );
               }
               while ( !ls.isEmpty() ) {
                  if ( !dad_window->script_load( ls.front(), errormsg ) ) {
                     gui_script_error( i, cmd + " " + opt1 + " " + ls.front(), errormsg );
                  }
                  ls.pop_front();
               }
            } else if ( opt1 == "list" ) {
               QStringList files = dad_window->script_files();
               TSO << QString( "dad: %1 curves loaded\n" ).arg( files.size() );
               for ( int j = 0; j < (int) files.size(); ++j ) {
                  TSO << QString( "dad: curve %1\n" ).arg( files[ j ] );
               }
            } else if ( opt1 == "select" ) {
               if ( ls.isEmpty() ) {
                  gui_script_error( i, cmd + " " + opt1, "needs \"all\" or a substring to match" );
               }
               if ( !dad_window->script_select( ls.front(), errormsg ) ) {
                  gui_script_error( i, cmd + " " + opt1 + " " + ls.front(), errormsg );
               }
               ls.pop_front();
            } else if ( opt1 == "makealambda" ) {
               if ( !dad_window->script_make_a_of_lambda( errormsg ) ) {
                  gui_script_error( i, cmd + " " + opt1, errormsg );
               }
            } else if ( opt1 == "save" ) {
               if ( !dad_window->script_save( ls.isEmpty() ? QString( "" ) : ls.front(), errormsg ) ) {
                  gui_script_error( i, cmd + " " + opt1, errormsg );
               }
               if ( !ls.isEmpty() ) {
                  ls.pop_front();
               }
            } else {
               gui_script_error( i, cmd, "unknown option : " + opt1 );
            }
         }
         qApp->processEvents();
      } else {
         gui_script_error( i, cmd, "unknown command" );
      }
   }
   gui_script_msg( scriptlinesc, "script", "finished" );
   guiFlag = true;
}
