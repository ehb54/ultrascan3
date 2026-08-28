// (this) us_hydrodyn.cpp contains class creation & gui connected functions
#include <QRegularExpression>
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o
// us_hydrodyn_info.cpp contains code to report structures for debugging
// us_hydrodyn_util.cpp contains other various code, such as disulfide code
// us_hydrodyn_load.cpp contains code to load files 
// us_hydrodyn_grpy.cpp contains code for grpy interface

// includes and defines need cleanup
 
#include "../include/us_hydrodyn.h"

// GRPY module (issue 1012): the shell reduction and the process boundary to the GRPY
// program, which is run rather than linked
#include "grpy_process.hpp"
#include "grpy_shell.hpp"

#define SLASH QDir::separator()

// Total physical RAM in bytes (0 = unknown). Used by the GRPY pre-flight memory
// guard so an oversized model is refused rather than swapping the machine to death.
#if defined( Q_OS_WIN ) || defined( _WIN32 )
#  include <windows.h>
#elif defined( Q_OS_MACOS ) || defined( __APPLE__ )
#  include <sys/sysctl.h>
#elif defined( Q_OS_LINUX ) || defined( __linux__ )
#  include <unistd.h>
#endif
static qint64 grpy_physical_ram_bytes() {
#if defined( Q_OS_WIN ) || defined( _WIN32 )
   MEMORYSTATUSEX st; st.dwLength = sizeof( st );
   if ( GlobalMemoryStatusEx( &st ) ) {
      return (qint64) st.ullTotalPhys;
   }
   return 0;
#elif defined( Q_OS_MACOS ) || defined( __APPLE__ )
   int64_t mem = 0; size_t len = sizeof( mem );
   if ( sysctlbyname( "hw.memsize", &mem, &len, nullptr, 0 ) == 0 ) {
      return (qint64) mem;
   }
   return 0;
#elif defined( Q_OS_LINUX ) || defined( __linux__ )
   long pages = sysconf( _SC_PHYS_PAGES ), ps = sysconf( _SC_PAGE_SIZE );
   if ( pages > 0 && ps > 0 ) {
      return (qint64) pages * (qint64) ps;
   }
   return 0;
#else
   return 0;   // unknown platform: no guard (fail open)
#endif
}

// Scripting-override parser. File-static rather than a lambda so the pre-flight guard in
// calc_grpy_hydro() and the solver setup in grpy_process_next() read gparams identically.
static bool truthy( QString v ) {
   v = v.trimmed().toLower();
   return v == "1" || v == "true" || v == "yes" || v == "on";
}

// Pause a US_Timer across a blocking modal dialog, resuming on scope exit.
//
// The GRPY run is timed end to end, but the results-writing path prompts for a filename
// whenever an output file already exists and overwrite_hydro is false -- and that prompt is
// modal, so the operator's response time landed in "Time to process". Measured at 71 s on
// one run, and it varies per run, which makes interactive GRPY timings incomparable.
//
// US_Timer::stop_timer banks the interval so far WITHOUT counting a completion, and
// start_timer restarts from zero; so stop/start is exactly pause/resume, and the paused
// span is simply never accumulated. No change to US_Timer is needed.
//
// The prompt itself is unchanged -- this only stops it being counted as compute.
class GrpyTimerPause {
public:
   GrpyTimerPause( US_Timer & t, const QString & name ) : t_( t ), name_( name ) {
      t_.stop_timer( name_ );
   }
   ~GrpyTimerPause() { t_.start_timer( name_ ); }
private:
   US_Timer & t_;
   QString    name_;
};
#define GRPY_PAUSE_TIMER GrpyTimerPause grpy_timer_pause__( timers, "compute grpy all models" )

// Write, and display, a bead model of each rung's reduced shell.
//
// The shell report records its selection as indices into the bead list GRPY was given,
// which is the .grpy file -- written from `use_model` in bead_model_output_order(), keeping
// only beads that are active and (unless buried beads are included) not buried. That same
// filter and order is rebuilt here, so index k from the report is the k-th bead here. Note
// bead_output.sequence == 1 reorders the model, so taking bead_model order instead would
// quietly select the wrong beads.
//
// Diagnostic only, off by default: this exists so the reduction can be seen and checked.
// Files land in <somo>/tmp with fixed per-rung names and overwrite freely -- they are
// regenerated every run, and prompting per rung would add a modal dialog to each.
void US_Hydrodyn::grpy_write_shell_model( const vector < int > & idx,
                                          int rung,
                                          const QString & base_name ) {
   if ( idx.empty() ) {
      return;
   }

   // The model being solved, NOT `bead_model`. `bead_model` is left holding whichever model
   // the setup loop in calc_grpy_hydro() assigned last, so with several models selected every
   // one but the last would have its shells rebuilt from the wrong structure -- and the
   // index-bounds check below only catches that when the bead COUNTS happen to differ.
   // grpy_process_next() sets grpy_last_model_number per model, so it names the current one.
   if ( grpy_last_model_number < 0 ||
        grpy_last_model_number >= (int) bead_models.size() ) {
      return;
   }
   // Rebuild the .grpy bead order/filter exactly.
   vector < PDB_atom * > use_model;
   bead_model_output_order( & bead_models[ grpy_last_model_number ], use_model );
   vector < PDB_atom * > used;
   used.reserve( use_model.size() );
   for ( unsigned int i = 0; i < use_model.size(); i++ ) {
      if ( use_model[ i ]->active ) {
         int color = get_color( use_model[ i ] );
         if ( hydro.grpy_bead_inclusion || color != 6 ) {
            used.push_back( use_model[ i ] );
         }
      }
   }

   const QString dir = get_somo_dir() + SLASH + "tmp";
   QDir().mkpath( dir );

   {
      const int r = rung;
      vector < PDB_atom > shell;
      shell.reserve( idx.size() );
      bool bad = false;
      for ( int k : idx ) {
         if ( k < 0 || k >= (int) used.size() ) { bad = true; break; }
         shell.push_back( * used[ (unsigned int) k ] );
      }
      if ( bad ) {
         // Only reachable if the .grpy bead list and this one disagree, which would mean
         // the models are of the wrong beads -- say so rather than write something wrong.
         editor_msg( "red",
                     QString( us_tr( "GRPY shell reduction: internal bead-index mismatch"
                                     " (%1 selected of %2 available); shell models not"
                                     " written.\n" ) )
                     .arg( idx.size() ).arg( used.size() ) );
         return;
      }

      const QString name = QString( "%1-shell-rung-%2" ).arg( base_name ).arg( r + 1 );
      write_bead_model( dir + SLASH + name, & shell );
      write_bead_spt  ( dir + SLASH + name, & shell );
      editor_msg( "dark blue",
                  QString( us_tr( "GRPY shell reduction: wrote shell model rung %1,"
                                  " %2 beads: %3\n" ) )
                  .arg( r + 1 ).arg( shell.size() ).arg( dir + SLASH + name ) );
      model_viewer( dir + SLASH + name + ".spt", "-script" );
   }
}

// Encode the GRPY settings that change the answer into a file-name suffix, in the style
// SOMO already uses for bead models (short mnemonic + value, '.' -> '_', appended only when
// the option is set -- cf. PR1_4, TH10, pH7, A20, hy, G4 in getExtendedSuffix()).
//
// Only non-default settings appear, so a default run produces exactly the names it always
// did. The point is that shells produced under different settings must not collide: at one
// target accuracy, or with intrinsic viscosity required rather than not, the retained beads
// differ, and without this the files would silently overwrite each other.
//
//   sp        single precision
//   SR0_5     shell reduction, 0.5% target accuracy
//   eta/noeta whether intrinsic viscosity had to converge
static QString grpy_settings_suffix( bool single, bool shell, double tol, bool require_eta ) {
   QString s;
   if ( single ) {
      s += "-sp";
   }
   if ( shell ) {
      s += QString( "-SR%1" ).arg( 100.0 * tol );
      s += require_eta ? "eta" : "noeta";
   }
   return s.replace( ".", "_" );
}

// Format a fraction as a percentage string, e.g. 0.00489 -> "0.489%".
//
// The percent sign has to be attached HERE rather than written as a literal in a format
// string, because QString::arg() -- unlike printf -- has no "%%" escape: it substitutes
// %1..%99 and passes every other "%" through untouched, so "%1%%" renders a stray double
// percent. Producing the whole token here also keeps a literal "%" from ever sitting next
// to a placeholder, where "%1%%2" would be genuinely ambiguous to read.
static QString grpy_pct( double frac, int prec = 3 ) {
   return QString::number( 100.0 * frac, 'g', prec ) + "%";
}

// Peak bytes for the tiled upper triangle of the 11N x 11N mobility matrix, plus ~10%.
// Single definition so the pre-flight guard and the shell-reduction budget below can never
// drift apart and disagree about what fits.
static double grpy_matrix_bytes( int beads, bool single ) {
   const double dim = 11.0 * (double) beads;
   return dim * dim * 0.5 * ( single ? 4.0 : 8.0 ) * 1.10;
}

// Inverse of the above: the largest bead count that still fits the guard's budget
// (70% of physical RAM). 0 = no limit known, so no cap is applied.
static int grpy_max_beads_for_ram( bool single ) {
   const qint64 ram = grpy_physical_ram_bytes();
   if ( ram <= 0 ) {
      return 0;
   }
   const double per_bead2 = grpy_matrix_bytes( 1, single );      // bytes at N = 1
   const double n = sqrt( ( 0.70 * (double) ram ) / per_bead2 );
   return n > 1.0 ? (int) n : 1;
}

// Explicit override of the shell-reduction bead cap: GRPY_SHELL_MAX_BEADS in the
// environment, or the grpy_shell_max_beads script parameter (script wins). Returns 0 when
// unset or unparseable, meaning "use the memory-derived cap".
//
// Env/script only, with no GUI control, matching grpy_ooc_dir: this is a diagnostic and
// shared-machine knob, not a user setting. Two uses:
//   - bounding GRPY's footprint below 70% of RAM on a machine shared with other work;
//   - exercising the memory-capped path at all. Without it the ladder can only reach the
//     cap by FAILING on the rung below it, which on a model large enough to have a cap
//     means paying for that rung first -- measured at 5h44m on a 11328-bead model.
// It is honoured in both directions; a value above what memory supports is warned about
// rather than silently clamped, since overriding is a deliberate act.
static int grpy_shell_cap_override( const map < QString, QString > & gparams ) {
   int v = 0;
   bool ok = false;
   if ( !qEnvironmentVariableIsEmpty( "GRPY_SHELL_MAX_BEADS" ) ) {
      const int e = QString( qgetenv( "GRPY_SHELL_MAX_BEADS" ) ).trimmed().toInt( &ok );
      if ( ok && e > 0 ) {
         v = e;
      }
   }
   if ( gparams.count( "grpy_shell_max_beads" ) ) {
      const int s = gparams.at( "grpy_shell_max_beads" ).trimmed().toInt( &ok );
      if ( ok && s > 0 ) {
         v = s;
      }
   }
   return v;
}

double US_Hydrodyn::model_mw( const vector < PDB_atom *> use_model ) {
   double mw = 0e0;
   for (unsigned int i = 0; i < use_model.size(); i++) {
      if ( use_model[i]->active ) {
         mw += use_model[i]->bead_ref_mw + use_model[i]->bead_ref_ionized_mw_delta;
      }
   }
   return mw;
}

vector < PDB_atom > US_Hydrodyn::exposed_model( const vector < PDB_atom > & model ) {
   vector < PDB_atom > rmodel;

   for (int i = 0; i < (int) model.size(); ++i) {
      if ( model[i].active ) {
         int color = get_color( &model[ i ] );
         if ( /* color != 6 && */ color != 0 ) {
            rmodel.push_back( model[ i ] );
         }
      }
   }
   return rmodel;
}

double US_Hydrodyn::model_mw( const vector < PDB_atom > & use_model ) {
   double mw = 0e0;
   for (unsigned int i = 0; i < use_model.size(); i++) {
      if ( use_model[i].active ) {
         mw += use_model[i].bead_ref_mw + use_model[i].bead_ref_ionized_mw_delta;
      }
   }
   return mw;
}

int US_Hydrodyn::grpy_used_beads_count( const vector < PDB_atom *> use_model ) {
   int used_beads = 0;
   for (unsigned int i = 0; i < use_model.size(); i++) {
      if ( use_model[i]->active ) {
         int color = get_color( use_model[ i ] );
         if ( hydro.grpy_bead_inclusion || color != 6 ) {
            used_beads++;
         }
      }
   }
   return used_beads;
}

int US_Hydrodyn::grpy_used_beads_count( const vector < PDB_atom > & use_model ) {
   int used_beads = 0;
   for (unsigned int i = 0; i < use_model.size(); i++) {
      if ( use_model[i].active ) {
         int color = get_color( & use_model[ i ] );
         if ( hydro.grpy_bead_inclusion || color != 6 ) {
            used_beads++;
         }
      }
   }
   return used_beads;
}

int US_Hydrodyn::total_beads_count( const vector < PDB_atom > & use_model ) {
   int total_beads = 0;
   for (unsigned int i = 0; i < use_model.size(); i++) {
      if ( use_model[i].active ) {
         total_beads++;
      }
   }
   return total_beads;
}

// grpy process flow
// start: calc_grpy_hydro()
// for each model grpy_process_next()
// when finished  grpy_finished()
// when all models complete grpy_finalize() - which stores save_data etc


// Full path to the GRPY program. This is the lookup the external path used before the
// in-process port (issue 1012 restores it), without the Docker/container branch: the
// program ships in the UltraScan bin directory as one of GRPY_osx10.11, GRPY_linux64 or
// GRPY_win64.exe, and add_to_bin still carries those names.
static QString grpy_program_path() {
   US_Config* cfg = new US_Config();
   const QString dir =
      cfg->config_list.system_dir + QDir::separator() +
#if defined( BIN64 )
      "bin64"
#else
      "bin"
#endif
      ;
   delete cfg;
   return QDir( dir ).filePath(
#if defined( Q_OS_WIN )
      "GRPY_win64.exe"
#elif defined( Q_OS_MAC )
      "GRPY_osx10.11"
#else
      "GRPY_linux64"
#endif
      );
}

bool US_Hydrodyn::calc_grpy_hydro() {
   progress->set_cli_prefix( "ch" );

   {
      QFont courier = QFont( "Courier", USglobal->config_list.fontSize );
      editor_msg( "dark blue", courier, visc_dens_msg() );
   }

   // The GRPY program is run per model (issue 1012); the thread count is passed to it.
   editor_msg( "darkblue", QString( us_tr( "\nGRPY enabled with %1 threads\n" ) )
               .arg( USglobal->config_list.numThreads ) );

   // if ( !overwrite_hydro )
   // {
   //    setHydroFile();
   // }

   // Scripting override for buried-bead inclusion (issue 984 follow-up). The solver-level
   // overrides below are resolved into locals at the call site, but this one cannot be:
   // bead selection happens in several places across a run, including in grpy_finished()
   // after this function has returned, so the setting itself must carry the override.
   // Applied, not restored -- a script's `global` is a settings change and the run must be
   // self-consistent from selection through to the results it writes. Announced, because a
   // silent change to which beads are used would invalidate every number the run produces.
   if ( gparams.count( "grpy_bead_inclusion" ) ) {
      const bool want = truthy( gparams[ "grpy_bead_inclusion" ] );
      if ( want != hydro.grpy_bead_inclusion ) {
         hydro.grpy_bead_inclusion = want;
         display_default_differences();
      }
      editor_msg( "dark red",
                  QString( us_tr( "GRPY: buried-bead inclusion set to %1 by grpy_bead_inclusion\n" ) )
                  .arg( want ? us_tr( "INCLUDE all beads" ) : us_tr( "exclude buried beads" ) ) );
   }

   stopFlag = false;
   grpy_was_hydro_enabled = pb_calc_hydro->isEnabled();
   pb_stop_calc->setEnabled(true);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled( false );
   pb_calc_hullrad->setEnabled( false );
   //   puts("calc hydro (supc)");
   set_disabled( false );
   display_default_differences( false );
   editor->append("\nBegin hydrodynamic calculations\n\n");
   results.s20w_sd = 0.0;
   results.D20w_sd = 0.0;
   results.viscosity_sd = 0.0;
   results.rs_sd = 0.0;
   results.rg_sd = 0.0;
   results.tau_sd = 0.0;

   int models_to_proc = 0;
   int first_model_no = 0;
   QStringList model_names;
   grpy_to_process   .clear();
   grpy_model_numbers.clear();
   grpy_processed    .clear();
   grpy_addl_params  .clear();
   grpy_used_beads   .clear();   // was drained only via pop_front, so an early abort
                                 // (e.g. the memory guard) left a stale count behind
   grpy_settings_sfx .clear();   // likewise: a run that aborts before the settings are
                                 // resolved must not leave the previous run's suffix
   grpy_eff_hydro        = hydro; // refined in grpy_process_next once overrides resolve
   grpy_shell_err_pct    = 0e0;
   grpy_shell_worst_name = "";

   grpy_results.method                = "GRPY";
   grpy_results.mass                  = 0e0;
   grpy_results.s20w                  = 0e0;
   grpy_results.s20w_sd               = 0e0;
   grpy_results.D20w                  = 0e0;
   grpy_results.D20w_sd               = 0e0;
   grpy_results.viscosity             = 0e0;
   grpy_results.viscosity_sd          = 0e0;
   grpy_results.rs                    = 0e0;
   grpy_results.rs_sd                 = 0e0;
   grpy_results.rg                    = 0e0;
   grpy_results.rg_sd                 = 0e0;
   grpy_results.tau                   = 0e0;
   grpy_results.tau_sd                = 0e0;
   grpy_results.asa_rg_pos            = 0e0;
   grpy_results.asa_rg_neg            = 0e0;
   grpy_results.ff0                   = 0e0;
   grpy_results.ff0_sd                = 0e0;

   grpy_results.solvent_name          = hydro.solvent_name;
   grpy_results.solvent_acronym       = hydro.solvent_acronym;
   grpy_results.solvent_viscosity     = use_solvent_visc();
   grpy_results.solvent_density       = use_solvent_dens();
   grpy_results.temperature           = hydro.temperature;
   grpy_results.pH                    = hydro.pH;
   grpy_results.name                  = "";
   grpy_results.used_beads            = 0;
   grpy_results.used_beads_sd         = 0e0;
   grpy_results.total_beads           = 0;
   grpy_results.total_beads_sd        = 0e0;
   grpy_results.vbar                  = 0;
   
   grpy_results.num_models            = 0;

   grpy_results2                      = grpy_results;

   grpy_vdw                           = bead_model_suffix.contains( "-vdw" );
   
   QDir::setCurrent(get_somo_dir());
   
   QString extension;

#if defined( TEST_VDW_GRPY_ASA )
   bool use_threshold = false;
   if ( !hydro.bead_inclusion && grpy_vdw ) {

      switch ( QMessageBox::question(this, 
                                     this->windowTitle() + us_tr(": GRPY ASA" ),
                                     us_tr( "Choose the ASA method for GRPY bead inclusion"),
                                     us_tr( "&Use SOMO ASA Threshold [A^2]" ), 
                                     us_tr( "Use SOMO Bead ASA Threshold %" ), 
                                     QString(),
                                     1, // Stop == button 0
                                     1 // Escape == button 0
                                     ) )
      {
      case 0 : //
         use_threshold = true;
         break;
      case 1 : // keep
         use_threshold = false;
         break;
      }

      qDebug() << "use_threshold is " << ( use_threshold ? "true" : "false" );

   }
#endif

   if ( !hydro.grpy_bead_inclusion && grpy_vdw ) {

      extension =
         QString( "_R%1PR%2" )
         .arg( asa.vdw_grpy_threshold_percent )
         .arg( asa.vdw_grpy_probe_radius )
         ;
   }

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         if (somo_processed[current_model]) {
            if (!first_model_no) {
               first_model_no = current_model + 1;
            }
            models_to_proc++;
            results.vbar = use_vbar( model_vector[ current_model ].vbar );

            editor_msg( "black", QString( "Model %1 will be included\n").arg( model_name( current_model ) ) );
            model_names.push_back( model_name( current_model ) );
            set_bead_colors( bead_models[ current_model ] );

            bead_model = bead_models[current_model];

            QString fname = 
               get_somo_dir() + SLASH +
               project +
               ( bead_model_from_file ? "" : QString( "_%1" ).arg( model_name( current_model ) ) ) +
               QString( bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
               extension
               ;

            if ( !overwrite_hydro ) {
               fname = fileNameCheck( fname, 0, this );
            }

            QTextStream( stdout ) << "grpy file is '" << fname << "'" << Qt::endl
                                  << "grpy file.fileName() is '" << QFileInfo( fname ).baseName() << "'" << Qt::endl
                                  << "bead model size is " << bead_model.size() << Qt::endl
               ;
            
            // always run asa

            if ( !hydro.grpy_bead_inclusion && grpy_vdw ) {
               // expose all
               for ( int i = 0; i < (int) bead_model.size(); ++i ) {
                  bead_model[ i ].exposed_code = 1;
               }
               editor_msg( "black",
                           QString( us_tr( "Running ASA check on vdW bead model: Probe radius %1 [A], Threshold %2%." ) )
                           .arg( asa.vdw_grpy_probe_radius )
                           .arg( asa.vdw_grpy_threshold_percent )
                           );
               bead_check( false, true, true );
            }

            editor_msg( "black", us_tr( "Running PAT" ) );
            pat_model( bead_model );
            
            // qDebug() << "calc_grpy_hydro() asa rg +/- " << results.asa_rg_pos << " " << results.asa_rg_neg;

            grpy_addl_param.clear();
            // grpy_addl_param[ "asa_rg_pos" ] = model_vector[ current_model ].asa_rg_pos;
            // grpy_addl_param[ "asa_rg_neg" ] = model_vector[ current_model ].asa_rg_neg;
            grpy_addl_params.push_back( grpy_addl_param );
               
            write_bead_model( QFileInfo( fname ).fileName(),
                              & bead_model,
                              US_HYDRODYN_OUTPUT_GRPY );

            #define GRPY_MAX_FILENAME_LENGTH 70
            #define GRPY_MAX_FILENAME_LENGTH_PAD 10
            if ( last_bead_model.length() > GRPY_MAX_FILENAME_LENGTH ) {
               QString msg =
                  QString( us_tr( "Resulting file name length is %1 characters too long for GRPY calculations\n" ) )
                  .arg( last_bead_model.length() + GRPY_MAX_FILENAME_LENGTH_PAD - GRPY_MAX_FILENAME_LENGTH )
                  ;
               editor_msg( "red", msg );
               if ( batch_active() ) {
                  batch_window->editor_msg( "red", msg );
               }
               set_enabled();
               pb_calc_hydro->setEnabled(grpy_was_hydro_enabled);
               pb_calc_zeno->setEnabled(true);
               pb_bead_saxs->setEnabled(true);
               pb_calc_grpy->setEnabled( true );
               pb_calc_hullrad->setEnabled( true );
               pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
               pb_show_hydro_results->setEnabled(false);
               progress->reset();
               grpy_success = false;
               return false;
            }               

            write_bead_model( QFileInfo( fname ).fileName() + "-grpy", 
                              & bead_model );

            grpy_to_process    << QFileInfo( fname ).fileName() + ".grpy";
            grpy_model_numbers.push_back( current_model );
            grpy_used_beads   .push_back( grpy_used_beads_count( bead_model ) );
         } else {
            editor->append(QString("Model %1 - selected but bead model not built\n").arg( model_name( current_model ) ) );
         }
      }
   }

   qApp->processEvents();
   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      set_enabled();
      pb_calc_hydro->setEnabled(grpy_was_hydro_enabled);
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
      grpy_success = false;
      return false;
   }

   editor_msg( "dark blue", grpy_to_process.join( "\n" ) );

   // Pre-flight memory guard (issue 987): GRPY holds the tiled upper triangle of the
   // 11N x 11N mobility matrix in RAM, with peak ~ (11N)^2/2 * scalar bytes. A model
   // larger than physical RAM will thrash the machine to a standstill (e.g. 20k beads
   // is ~180 GB in double, ~90 GB in single). Estimate the largest model in the batch
   // and refuse rather than beachball. In script mode never block on a dialog -- fail
   // with a clear non-zero exit.
   {
      int max_beads = 0;
      for ( int nb : grpy_used_beads ) {
         if ( nb > max_beads ) {
            max_beads = nb;
         }
      }
      const qint64 ram = grpy_physical_ram_bytes();             // 0 = unknown -> skip
      // Resolve the same scripting overrides the solver applies further down, so the guard
      // judges the run that will actually happen rather than the dialog's settings.
      const bool single_eff = gparams.count( "grpy_single" )
         ? truthy( gparams[ "grpy_single" ] ) : hydro.grpy_single;
      const bool shell_eff  = gparams.count( "grpy_shell" )
         ? truthy( gparams[ "grpy_shell" ] )  : hydro.grpy_shell;
      const double est = grpy_matrix_bytes( max_beads, single_eff );

      // Shell reduction (issue 984) makes this survivable: its ladder is capped to what
      // fits (see ShellOptions::max_beads at the call site), so an oversized model yields
      // the largest rung that fits WITH its error bar instead of being refused outright.
      // Defer to it whenever the budget admits even the smallest rung -- below that there
      // is nothing to compute and the refusal still stands.
      const int    ram_cap  = grpy_max_beads_for_ram( single_eff );
      const int    cap_ovr  = grpy_shell_cap_override( gparams );
      const int    cap      = cap_ovr > 0 ? cap_ovr : ram_cap;
      const double smallest = grpy::ShellOptions{}.ladder.empty()
         ? 1.0 : grpy::ShellOptions{}.ladder.front();
      const bool   shell_can_help =
         shell_eff && cap > 0 && (double) cap >= smallest * (double) max_beads;

      // Say what the cap is, every run. An override especially must never be silent: it
      // changes which results are obtainable, and a stale environment variable would
      // otherwise be invisible while quietly bounding every calculation.
      if ( cap_ovr > 0 ) {
         const bool over_ram = ram_cap > 0 && cap_ovr > ram_cap;
         editor_msg( over_ram ? "red" : "dark red",
                     QString( us_tr( "GRPY: shell reduction bead cap OVERRIDDEN to %1 by %2"
                                     " (memory-derived cap %3)%4\n" ) )
                     .arg( cap_ovr )
                     .arg( gparams.count( "grpy_shell_max_beads" )
                           ? "grpy_shell_max_beads" : "GRPY_SHELL_MAX_BEADS" )
                     .arg( ram_cap > 0 ? QString::number( ram_cap ) : us_tr( "unknown" ) )
                     .arg( !shell_eff
                           // The cap only bounds the shell-reduction ladder. Announcing it
                           // as though it were in force while shell reduction is off reads
                           // as if it were limiting this run, which it is not.
                           ? us_tr( " -- no effect: shell reduction is off" )
                           : over_ram
                           ? us_tr( " -- ABOVE what this machine's memory supports; it may"
                                    " swap heavily" )
                           : QString() ) );
      } else if ( shell_eff && ram_cap > 0 ) {
         editor_msg( "dark blue",
                     QString( us_tr( "GRPY shell reduction: bead cap %1, from %2 GB of"
                                     " memory (set GRPY_SHELL_MAX_BEADS to override)\n" ) )
                     .arg( ram_cap )
                     .arg( (double) ram / ( 1024.0 * 1024.0 * 1024.0 ), 0, 'f', 1 ) );
      }

      if ( ram > 0 && max_beads > 0 && est > 0.70 * (double) ram && shell_can_help ) {
         editor_msg( "dark red",
                     QString( us_tr( "GRPY: the largest model (%1 beads) exceeds the memory "
                                     "budget for a full calculation; shell reduction will cap "
                                     "its ladder at %2 beads and report the resulting error.\n" ) )
                     .arg( max_beads ).arg( cap ) );
      }

      if ( ram > 0 && max_beads > 0 && est > 0.70 * (double) ram && !shell_can_help ) {
         const double GB = 1024.0 * 1024.0 * 1024.0;
         QString msg =
            QString( us_tr( "GRPY memory estimate exceeds available RAM:\n"
                            "the largest selected model has %1 beads and would need about "
                            "%2 GB in %3 precision, but this machine has %4 GB.\n"
                            "Enable single precision (about half the memory) or reduce the "
                            "model; for much larger structures use the ZENO method instead, "
                            "which is nearly size-independent (but does not compute "
                            "rotational diffusion). Then retry.\n" ) )
            .arg( max_beads )
            .arg( est / GB, 0, 'f', 1 )
            .arg( single_eff ? us_tr( "single" ) : us_tr( "double" ) )
            .arg( (double) ram / GB, 0, 'f', 1 );
         if ( !shell_eff ) {
            msg += us_tr( "\nGRPY shell reduction is another option: it computes on the "
                          "most solvent-exposed beads only, stopping when the result stops "
                          "changing, and reports the remaining error. Unlike ZENO it still "
                          "computes rotational diffusion.\n" );
         }
         if ( gui_script ) {
            // headless script mode: proper failure, no blocking dialog.
            editor_msg( "red", msg );
            fprintf( stderr, "%s", (const char *) ( msg + "\n" ).toUtf8() );
            exit( -1 );
         }
         // interactive: let the user override, but default to Cancel.
         if ( QMessageBox::warning(
                 this, windowTitle() + us_tr( ": GRPY memory" ),
                 msg + us_tr( "\nRun anyway (the interface may become unresponsive and the "
                              "machine may swap heavily)?" ),
                 us_tr( "&Cancel" ), us_tr( "Run &anyway" ), QString(), 0, 0 ) == 0 ) {
            editor_msg( "red", us_tr( "GRPY cancelled: model too large for available memory.\n" ) );
            set_enabled();
            pb_calc_hydro->setEnabled( grpy_was_hydro_enabled );
            pb_calc_zeno->setEnabled( true );
            pb_bead_saxs->setEnabled( true );
            pb_calc_grpy->setEnabled( true );
            pb_calc_hullrad->setEnabled( true );
            pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
            pb_show_hydro_results->setEnabled( false );
            progress->reset();
            grpy_success = false;
            return false;
         }
      }
   }

   grpy_mm_save_params.data_vector.clear();
   grpy_mm         = grpy_to_process.size() > 1;
   grpy_mm_results = "";
   grpy_mm_name    =
      get_somo_dir() + SLASH +
      project +
      QString( bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
      extension
      ;

   grpy_running    = true;
   grpy_success    = true;
   // Cleared at run start so the non-shell path (and any earlier run) can never
   // leave viscosity suppressed. Set per model by the shell-reduction path.
   grpy_viscosity_unreliable = false;
   progress->setMaximum( 101 * ( grpy_to_process.size() ) + 1 );
   // qDebug() << "progress max " <<  101 * ( grpy_to_process.size() ) + 1;
   progress->setValue( 0 );
   mprogress->setValue( 0 );
   mprogress->setMaximum( 100 );
   mprogress->setFormat( "Model %p%" );
   if ( grpy_mm ) {
      mprogress->show();
   }
   
   timers.clear_timers();
   timers.init_timer( "compute grpy all models" );
   timers.start_timer( "compute grpy all models" );

   grpy_process_next();

   return true;
}

void US_Hydrodyn::grpy_process_next() {
   // qDebug() << "US_Hydrodyn::grpy_process_next()";
   if ( !grpy_to_process.size() ) {
      grpy_finalize();
      return;
   }

   progress->setValue( 101 * grpy_processed.size() + 1 );
   mprogress->setValue( 1 );
   // qDebug() << "progress value " << 101 * grpy_processed.size() + 1;

   grpy_last_processed    = grpy_to_process   [ 0 ];
   grpy_last_model_number = grpy_model_numbers[ 0 ];
   grpy_last_used_beads   = grpy_used_beads   [ 0 ];
   grpy_addl_param        = grpy_addl_params  [ 0 ];
   grpy_to_process        .pop_front();
   grpy_model_numbers     .pop_front();
   grpy_used_beads        .pop_front();
   grpy_addl_params       .pop_front();
   grpy_processed         .push_back( grpy_last_processed );

   grpy_stdout = "";

   timers.init_timer( "compute grpy this model" );
   timers.start_timer( "compute grpy this model" );

   {
      // Run the GRPY program on the .grpy file SOMO wrote (issue 1012), and populate
      // grpy_stdout with its report so grpy_finished()'s existing parsing and .grpy_res
      // preservation work unchanged. The program threads internally; processEvents() in
      // the progress callback keeps the GUI painting while it runs.
      editor_msg( "black", QString( "\nStarting GRPY on %1 with %2 beads\n" )
                  .arg( QFileInfo( grpy_last_processed ).completeBaseName() )
                  .arg( grpy_last_used_beads ) );

      // resolve against get_somo_dir() -- the old QProcess set its working
      // directory there and passed grpy_last_processed (a bare filename) as -e arg,
      // so the .grpy file lives in the somo dir regardless of the process CWD.
      QString grpy_path = QDir( get_somo_dir() ).filePath( grpy_last_processed );
      if ( !QFileInfo( grpy_path ).exists() ) {
         editor_msg( "red", QString( us_tr( "GRPY input file '%1' does not exist\n" ) ).arg( grpy_path ) );
         grpy_success = false;
         // defer to the event loop (avoids deep recursion through the model batch);
         // a lambda is used rather than invokeMethod-by-name so we don't need to
         // register QProcess::ExitStatus as a queued-connection metatype.
         QTimer::singleShot( 0, this, [ this ]() {
            grpy_finished( 0, QProcess::NormalExit );
         } );
         return;
      }
      // Failure boundary. With GRPY back in its own process a failed solve kills a child
      // rather than the session -- an allocation failure inside the solver is the child's
      // problem now -- but this side can still fail: an unreadable input file, a missing
      // program, a non-zero exit. Everything from the parse to the last result extraction
      // runs inside this try, and a user stop arrives here too, as grpy::Stopped.
      try {
      grpy::Input in = grpy::read_grpy_input( grpy_path );

      // large-N options (issue 972): single-precision storage/factor halves memory,
      // out-of-core spills the tiled matrix to disk so RAM stays bounded. Both matter
      // only for very large bead models. Single-precision is the GRPY Numerical
      // Precision control in the SOMO Hydrodynamic Calculation Options window
      // (hydro.grpy_single; default Double). The gui_script `global grpy_single`/
      // `global grpy_ooc_dir` params and GRPY_SINGLE/GRPY_OOC_DIR env vars still
      // override, for headless/batch automation. Out-of-core has no GUI control
      // (script/env only) as it is a cluster-scale knob. Default = in-core double.
      grpy::Options opt;
      opt.single = hydro.grpy_single;                       // GUI checkbox (default off)
      if ( gparams.count( "grpy_single" ) ) {               // scripting override
         opt.single = truthy( gparams[ "grpy_single" ] );
      } else if ( !qEnvironmentVariableIsEmpty( "GRPY_SINGLE" ) ) {
         opt.single = true;
      }
      QString ooc_dir = gparams.count( "grpy_ooc_dir" )
         ? gparams[ "grpy_ooc_dir" ]
         : QString::fromLocal8Bit( qgetenv( "GRPY_OOC_DIR" ) );
      opt.ooc_dir = ooc_dir.trimmed().toStdString();
      if ( opt.single || !opt.ooc_dir.empty() ) {
         editor_msg( "dark blue",
                     QString( us_tr( "GRPY options: %1%2\n" ) )
                     .arg( opt.single ? us_tr( "single-precision " ) : QString() )
                     .arg( opt.ooc_dir.empty() ? QString()
                           : QString( us_tr( "out-of-core (%1) " ) ).arg( ooc_dir.trimmed() ) ) );
      }

      // Shell reduction (issue 984): run exact GRPY on a solvent-exposed subset, with a
      // convergence ladder that reports its own error. Off by default -- with it off the
      // result is byte-identical to the plain Solver, so existing results never move.
      // The observable set matters: intrinsic viscosity needs roughly 3.3x the accuracy
      // budget of D_t at equal reduction and drove the stopping decision in every test
      // case, so requiring it costs a large part of the speedup. When it is NOT required,
      // viscosity is withheld from the reported results as unreliable (see
      // grpy_viscosity_unreliable and its use in grpy_finished).
      grpy::ShellOptions sopt;
      sopt.enabled = hydro.grpy_shell;
      sopt.tol     = hydro.grpy_shell_tol > 0 ? hydro.grpy_shell_tol : 5e-3;
      bool require_eta = hydro.grpy_shell_require_eta;
      if ( gparams.count( "grpy_shell" ) ) {                // scripting override
         sopt.enabled = truthy( gparams[ "grpy_shell" ] );
      } else if ( !qEnvironmentVariableIsEmpty( "GRPY_SHELL" ) ) {
         sopt.enabled = true;
      }
      if ( gparams.count( "grpy_shell_tol" ) ) {
         double v = gparams[ "grpy_shell_tol" ].toDouble();
         if ( v > 0 ) {
            sopt.tol = v;
         }
      }
      if ( gparams.count( "grpy_shell_require_eta" ) ) {
         require_eta = truthy( gparams[ "grpy_shell_require_eta" ] );
      }
      sopt.require = { grpy::Obs::Dt, grpy::Obs::Dr, grpy::Obs::Sedimentation };
      if ( require_eta ) {
         sopt.require.push_back( grpy::Obs::EtaInf );
         sopt.require.push_back( grpy::Obs::EtaZero );
      }
      // Cap the ladder at what physical RAM allows, using the same budget the pre-flight
      // guard enforces. On a model that fits this never binds (every rung is under the cap
      // and the ladder ends at the full model as before); on one that does not, the ladder
      // stops at the largest rung that fits and reports its error rather than thrashing.
      // Resolved exactly as the guard resolves it -- if these two disagreed, the guard
      // could admit a run the ladder then refuses to compute, or vice versa. Reported
      // there, once per run, rather than here, which runs per model.
      {
         const int cap_ovr = grpy_shell_cap_override( gparams );
         sopt.max_beads = cap_ovr > 0 ? cap_ovr : grpy_max_beads_for_ram( opt.single );
      }

      // Diagnostic: keep each rung's selection so the reduced shells can be written out.
      sopt.record_subsets = hydro.grpy_shell_save_models;

      // Resolved once here, from the settings this run will actually use, and consumed by
      // every file the run writes -- the shell models below and the results in
      // grpy_finished() -- so they cannot disagree. Empty for default settings, leaving
      // those file names exactly as they have always been.
      grpy_settings_sfx = grpy_settings_suffix( opt.single, sopt.enabled, sopt.tol, require_eta );

      // What this run will actually use, overrides included.
      grpy_eff_hydro                          = hydro;
      grpy_eff_hydro.grpy_single              = opt.single;
      grpy_eff_hydro.grpy_shell               = sopt.enabled;
      grpy_eff_hydro.grpy_shell_tol           = sopt.tol;
      grpy_eff_hydro.grpy_shell_require_eta   = require_eta;

      // Declared before the on_rung lambda below, which captures it by reference to write
      // each rung's shell model as that rung lands.
      grpy::ShellReport srep;

      if ( sopt.enabled ) {
         editor_msg( "dark blue",
                     QString( us_tr( "GRPY shell reduction: tolerance %1%2\n" ) )
                     .arg( grpy_pct( sopt.tol ) )
                     .arg( require_eta ? us_tr( ", intrinsic viscosity required" )
                                       : us_tr( ", intrinsic viscosity NOT required (will be withheld)" ) ) );
         // Log each rung as it lands. Without this the ladder is silent for its whole
         // duration and the only visible sign of several solves is the progress bar.
         const double rung_tol   = sopt.tol;
         const bool   save_models = hydro.grpy_shell_save_models;
         // Name the shell models for the settings that produced them: at a different
         // target accuracy, or with viscosity required rather than not, the retained beads
         // differ, and identically-named files would overwrite each other between runs.
         const QString shell_base =
            QFileInfo( grpy_last_processed ).completeBaseName() + grpy_settings_sfx;
         // srep outlives the solve below, and the module fills in each rung's selection
         // BEFORE calling on_rung, so the current rung is srep.kept.back() here.
         sopt.on_rung = [ this, rung_tol, save_models, shell_base, &srep ]
            ( int i, int planned, int nb, double err ) {
            editor_msg( "dark blue",
                        QString( us_tr( "GRPY shell reduction: rung %1 of %2, %3 beads%4\n" ) )
                        .arg( i + 1 ).arg( planned ).arg( nb )
                        .arg( i == 0
                              ? us_tr( " (first rung: no error estimate yet)" )
                              : QString( us_tr( ", estimated error %1 (target %2)" ) )
                                .arg( grpy_pct( err ) )
                                .arg( grpy_pct( rung_tol ) ) ) );
            // Written and shown as each rung lands rather than in a batch at the end, so an
            // obviously wrong shell can be seen while there is still something to stop.
            if ( save_models && (int) srep.kept.size() == i + 1 ) {
               grpy_write_shell_model( srep.kept.back(), i, shell_base );
            }
            qApp->processEvents();
         };

      }

      // Installed for EVERY run, not just a shell-reduced one. It used to sit inside the
      // `if ( sopt.enabled )` above, so with shell reduction off -- which is the default,
      // and is now the only configuration -- nothing was ever installed and Stop could not
      // reach the solve at all. The progress callback's `if ( stopFlag ) return;` only
      // suppresses repaints; it does not end the computation, so the model ran to
      // completion with a frozen-looking interface.
      //
      // With a ladder this ends it between rungs: each rung costs roughly eight times the
      // one before, so stopping before the next begins saves nearly all that remained. And
      // since the solver became a separate program (issue 1012), a rung already running is
      // interrupted too -- ProcessSolver polls this flag every 100 ms and kills the child --
      // so Stop takes effect promptly rather than at the end of the model. The rungs already
      // finished keep their error bars and the result is reported as not converged.
      sopt.should_stop = [ this ]() { return stopFlag; };

      // How one bead list gets solved: by running the GRPY program on it. The shell
      // reduction calls this once per rung (issue 1012), which is what lets the ladder and
      // the exposure ranking stay in UltraScan while the GRPY-derived solver lives in its
      // own GPLv3 program and is invoked rather than linked.
      grpy::ProcessSolver::Config pcfg;
      pcfg.program     = grpy_program_path();
      pcfg.working_dir = get_somo_dir();
      // A rung that uses every bead runs on the .grpy file SOMO already wrote, so the
      // ordinary unreduced calculation reads exactly the file the user sees.
      pcfg.full_input  = grpy_path;
      pcfg.full_beads  = (int) in.beads.size();
      pcfg.rung_dir    = get_somo_dir();
      pcfg.single      = opt.single;
      pcfg.ooc_dir     = QString::fromStdString( opt.ooc_dir );
      pcfg.threads     = USglobal->config_list.numThreads;

      grpy::ProcessSolver psolver( pcfg, [ this ]() { return stopFlag; } );
      grpy::ShellSolver   solver( psolver.fn(), sopt );
      const int model = grpy_last_model_number;
      grpy::Results r = solver.run(
         in.beads, in.params, srep,
         [ this, model ]( int pct, const char * stage ) {
            if ( stopFlag ) {
               return;
            }
            progress->setValue( 101 * ( grpy_processed.size() - 1 ) + pct );
            mprogress->setValue( pct );
            lbl_core_progress->setText( QString( "Model %1 : %2" )
                                        .arg( model + 1 ).arg( stage ) );
            qApp->processEvents();
         } );
      grpy_stdout = QString::fromStdString( r.report );
      grpy_viscosity_unreliable = srep.viscosity_unreliable;

      // Report the beads the calculation ACTUALLY used. grpy_last_used_beads was set at
      // setup from the ASA-excluded count, before any reduction, so a shell-reduced run
      // would otherwise report the unreduced figure -- 11328 for a run that used 2832 --
      // in the results table, the .grpy_res report line and the saved CSV alike. Only
      // narrowed, never widened: with shell reduction off, srep.n_used is the full count.
      if ( srep.attempted && srep.n_used > 0 && srep.n_used < grpy_last_used_beads ) {
         grpy_last_used_beads = srep.n_used;
      }

      // Carried to grpy_finished() for the saved results: the bar the run achieved, and
      // which quantity it belongs to. Zero/empty when reduction was not attempted.
      grpy_shell_err_pct    = srep.attempted ? 100.0 * srep.err_max : 0e0;
      grpy_shell_worst_name = ( srep.attempted && srep.err_max > 0.0 )
         ? QString( grpy::obs_name( srep.worst ) ) : QString();
      if ( sopt.enabled ) {
         // err_max is the MAX over the required observables, and intrinsic viscosity runs
         // ~3.3x the error of D_t at equal reduction -- so on an unconverged run this one
         // number is usually the viscosity's, and quoting it alone makes D_t look far worse
         // than it is. Name the observable it belongs to; the per-observable bars are in
         // the results file. Meaningless on an exact (unreduced) result, where all bars are 0.
         const QString worst = srep.err_max > 0.0
            ? QString( us_tr( ", worst: %1" ) ).arg( grpy::obs_name( srep.worst ) )
            : QString();
         editor_msg( srep.converged ? "dark blue" : "red",
                     QString( us_tr( "GRPY shell reduction: %1 of %2 beads used, %3\n" ) )
                     .arg( srep.n_used ).arg( srep.n_full )
                     .arg( srep.converged
                           ? QString( us_tr( "converged (estimated error %1%2)" ) )
                             .arg( grpy_pct( srep.err_max ) ).arg( worst )
                           : srep.stopped
                           ? QString( us_tr( "STOPPED BEFORE CONVERGING (estimated error %1%2)" ) )
                             .arg( grpy_pct( srep.err_max ) ).arg( worst )
                           : srep.mem_capped
                           ? QString( us_tr( "STOPPED BY AVAILABLE MEMORY at %1 beads "
                                             "(estimated error %2%3)" ) )
                             .arg( sopt.max_beads ).arg( grpy_pct( srep.err_max ) ).arg( worst )
                           : QString( us_tr( "DID NOT CONVERGE (estimated error %1%2)" ) )
                             .arg( grpy_pct( srep.err_max ) ).arg( worst ) ) );
         if ( srep.err_max > 0.0 ) {
            editor_msg( "dark blue",
                        us_tr( "GRPY shell reduction: that is the LARGEST of the requested"
                               " quantities; the individual estimates, which are typically"
                               " smaller, are listed in the results file.\n" ) );
         }
         if ( srep.viscosity_unreliable ) {
            editor_msg( "red", us_tr( "GRPY shell reduction: intrinsic viscosity and Einstein"
                                      " radius are unconverged and are being withheld from the"
                                      " results (retained in the results file, annotated).\n" ) );
         }
      }

      } catch ( const grpy::Stopped & ) {
         // The user pressed Stop while a solve was running. Now that GRPY is a subprocess
         // it can be killed mid-solve rather than having to run to completion, so this
         // arrives as an exception rather than as a flag noticed afterwards. It is not a
         // failure: restore the interface the way the ordinary stop path does, say so
         // plainly, and do not fail a script over it.
         editor_msg( "dark red", us_tr( "GRPY stopped\n" ) );
         set_enabled();
         pb_calc_hydro->setEnabled( grpy_was_hydro_enabled );
         pb_calc_zeno->setEnabled( true );
         pb_bead_saxs->setEnabled( true );
         pb_calc_grpy->setEnabled( true );
         pb_calc_hullrad->setEnabled( true );
         pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
         pb_show_hydro_results->setEnabled( false );
         progress->reset();
         grpy_success  = false;
         grpy_running  = false;
         return;
      } catch ( const std::exception & e ) {
         // Restores the interface exactly as the pre-flight memory guard does when it
         // refuses a model -- the batch stops here rather than continuing on a model whose
         // results do not exist.
         const QString emsg = QString( us_tr( "GRPY failed: %1\n" ) ).arg( e.what() );
         editor_msg( "red", emsg );
         if ( gui_script ) {
            // headless script mode: a proper failure, as the memory guard does above --
            // there is no one to read the editor and the batch must not report success.
            fprintf( stderr, "%s", (const char *) emsg.toUtf8() );
            exit( -1 );
         }
         set_enabled();
         pb_calc_hydro->setEnabled( grpy_was_hydro_enabled );
         pb_calc_zeno->setEnabled( true );
         pb_bead_saxs->setEnabled( true );
         pb_calc_grpy->setEnabled( true );
         pb_calc_hullrad->setEnabled( true );
         pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
         pb_show_hydro_results->setEnabled( false );
         progress->reset();
         grpy_success  = false;
         grpy_running  = false;
         return;
      }

      // hand off to the existing finish/parse path, deferred to the event loop so we
      // don't recurse through the model batch (mirrors the old async QProcess finished
      // signal). A lambda avoids invokeMethod-by-name, which would need
      // QProcess::ExitStatus registered as a queued-connection metatype.
      QTimer::singleShot( 0, this, [ this ]() {
         grpy_finished( 0, QProcess::NormalExit );
      } );
      return;
   }
}

void US_Hydrodyn::grpy_finished( int, QProcess::ExitStatus )
{
   // qDebug() << "US_Hydrodyn::grpy_finished():" << grpy_last_processed;
   // us_qdebug( QString( "grpy_processExited %1" ).arg( grpy_last_processed) );
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   // grpy_stdout already holds the report the program produced; nothing to flush.
   if (stopFlag) {
      editor_msg( "red", us_tr( "Stopped by user\n" ) );
      set_enabled();
      pb_calc_hydro->setEnabled(grpy_was_hydro_enabled);
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
      mprogress->hide();
      lbl_core_progress->setText( "" );
      grpy_success = false;
      grpy_running = false;
      return;
   }

   editor_msg( "black", "GRPY finished.\n");

   // post process the files
   
   QStringList caps;
   caps
      << "Rotational diffusion coefficient"
      << "1"
      << "Dr"

      << "Sedimentation coefficient \\(Mw Dlt \\(1\\. - \\(vbar\\*rho\\)\\)/\\(nA kB T\\)\\)"
      << "1"
      << "s"

      << "Translational diffusion coefficient"
      << "2"
      << "Dt"

      << "Zero frequency intrinsic viscosity eta 0"
      << "1"
      << "\\[eta\\]"

      << "Relaxation time \\(1\\)"
      << "2"
      << "tau1"

      << "Relaxation time \\(2\\)"
      << "2"
      << "tau2"

      << "Relaxation time \\(3\\)"
      << "2"
      << "tau3"

      << "Relaxation time \\(4\\)"
      << "1"
      << "tau4"

      << "Relaxation time \\(5\\)"
      << "1"
      << "tau5"

      << "Harmonic mean \\(correlation\\) time"
      << "1"
      << "tauh"

      << "Translational diffusion coefficient"
      << "3"
      << "rs"

      << "Zero frequency intrinsic viscosity eta 0"
      << "2"
      << "grpy_einst_rad"

      ;

   map < QString, double > captures;
   grpy_captures     .clear();

   for ( int i = 0; i < (int) caps.size(); i += 3 ) {
      QRegularExpression rx( caps[ i ] + "\\s*:\\s*(\\S+)" );
      int pos = 0;
      bool found = false;
      int cappos = caps[ i + 1 ].toInt();
      int count  = 1;

      while ( true ) {
         QRegularExpressionMatch m = rx.match( grpy_stdout, pos );
         if ( !m.hasMatch() ) {
            break;
         }

         if ( cappos == count ) {
            grpy_captures[ caps[ i + 2 ] ].push_back( m.captured( 1 ).toDouble() );
            us_qdebug( QString( "%1 : '%2'\n" )
                       .arg( caps[ i + 2 ] )
                       .arg( grpy_captures[ caps[ i + 2 ] ].back() ) );
            found = true;
            break;
         }

         pos = m.capturedEnd();   // advance past this match (Qt6 replacement for matchedLength())
         ++count;
      }

      if ( !found ) {
         qDebug() << "grpy caps not found " << i << "'" << caps[ i ] << "'";
         editor_msg( "red", QString( us_tr( "Could not find '%1' in GRPY output" ) ).arg( caps[ i ].replace( "\\", "" ) ) );
         grpy_success = false;
         grpy_captures[ caps[ i + 2 ] ].push_back( -9e99 );
      }
   }

   // extra matrix captures for D1, D2, D3

   {
      QStringList qsl = grpy_stdout.split( "\n" );
      vector < QString > toXYZ = { "z", "y", "x" };
      bool ok = false;
      int i = 0;
      for ( ; i < (int) qsl.size(); ++i ) {
         if ( qsl[i].contains( "calculated using the mobility center:" ) ) {
            ok = true;
            break;
         }
      }
      if (!ok ) {
         editor_msg( "red", us_tr( "Could not find rotational diffusion coefficients" ) );
         grpy_success = false;
      } else {
         i += 19;
         int j = 4;
         QString Dr[3];
         for ( int k = 0; k < 3; ++k, ++i, ++j ) {
            if ( qsl.size() <= i ) {
               grpy_success = false;
               break;
            }
            QStringList qsld = qsl[ i ].split( QRegularExpression( QStringLiteral( "\\s+" ) ) );
            if ( qsld.size() <= j ) {
               grpy_success = false;
               break;
            }
            Dr[k] = qsld[ j ];
            grpy_captures[ "rot_diff_coef_" + toXYZ[ k ] ].push_back( Dr[k].toDouble() );
            QTextStream( stdout ) << "Dr[" << k << "]" << Dr[k] << Qt::endl;
         }
      }
   }

   if ( !grpy_success ) {
      grpy_process_next();
      return;
   }

   // save stdout
   if ( !batch_avg_hydro_active() && !grpy_mm ) {
      QString grpy_out_name = QString( grpy_last_processed )
         .replace( QRegularExpression( QStringLiteral( ".grpy$" ) ),
                   grpy_settings_sfx + ".grpy_res" );
      if ( !overwrite_hydro ) {
         GRPY_PAUSE_TIMER;   // operator response time is not compute
         grpy_out_name = fileNameCheck( grpy_out_name, 0, this );
      }
      QFile f( grpy_out_name );
      if ( !f.open( QIODevice::WriteOnly ) ) {
         editor_msg( "red", QString( us_tr( "Error: could not open output file %1 for writing" ) ).arg( grpy_out_name ) );
         grpy_success = false;
         grpy_process_next();
         return;
      } else {
         QTextStream t( &f );
         t << grpy_stdout;
         editor_msg( "dark blue", QString( us_tr( "Wrote %1" ) ).arg( grpy_out_name ) );
         f.close();
         last_hydro_res = grpy_out_name;
      }
   }

   // accumulate data as in zeno (e.g. push values to data structures )
      
   map < int, map < QString, double > > data_to_save;
   
   for ( map < QString, vector < double > >::iterator it = grpy_captures.begin();
         it != grpy_captures.end();
         ++it ) {

      for ( int i = 0; i < (int) it->second.size(); ++i ) {
         data_to_save[ i ][ it->first ] = it->second[ i ];

         if ( it->first == "R\\(Anhydrous\\)" ) {
            grpy_results.rs += it->second[ i ];
            grpy_results2.rs += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "f/fo" ) {
            grpy_results.ff0 += it->second[ i ];
            grpy_results2.ff0 += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "Dt" ) {
            grpy_results.D20w += it->second[ i ];
            grpy_results2.D20w += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "s" ) {
            grpy_results.s20w += it->second[ i ];
            grpy_results2.s20w += it->second[ i ] * it->second[ i ];
         }

         // Skipped when unconverged (issue 984). NOTE the cross-model mean below divides
         // by the TOTAL model count, not a per-observable count, so a run mixing
         // contributing and non-contributing models would silently corrupt it. That is
         // safe here only because shell reduction is a RUN-LEVEL setting: every model in
         // a run either requires viscosity convergence or none does.
         if ( it->first == "\\[eta\\]" && !grpy_viscosity_unreliable ) {
            grpy_results.viscosity += it->second[ i ];
            grpy_results2.viscosity += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "tauh" ) {
            grpy_results.tau += it->second[ i ] * 1e9;
            grpy_results2.tau += it->second[ i ] * it->second[ i ] * 1e18;
         }

         if ( it->first == "rs" ) {
            grpy_results.rs += it->second[ i ] * 1e7;
            grpy_results2.rs += it->second[ i ] * it->second[ i ] * 1e14;
         }
      }
   }
   
   timers.end_timer( "compute grpy this model" );

   for ( map < int, map < QString, double > >::iterator it = data_to_save.begin();
         it != data_to_save.end();
         ++it ) {
      save_data this_data = US_Hydrodyn_Save::save_data_initialized_from_bead_model( bead_models[ grpy_last_model_number ], !hydro.grpy_bead_inclusion );

      this_data.results.method                = "GRPY";
      this_data.results.mass                  = hydro.mass_correction ? hydro.mass : model_mw( bead_models[ grpy_last_model_number ] ); // ( model_vector[ grpy_last_model_number ].mw + model_vector[ grpy_last_model_number ].ionized_mw_delta );
      this_data.hydro                         = grpy_eff_hydro;
      this_data.grpy_shell_tol_pct            = 100.0 * grpy_eff_hydro.grpy_shell_tol;
      this_data.grpy_shell_err                = grpy_shell_err_pct;
      this_data.grpy_shell_worst              = grpy_shell_worst_name;
      this_data.results.num_models            = 1;
      this_data.results.name                  =
         // QString( "%1-%2" ).arg( QFileInfo( grpy_last_processed ).completeBaseName().replace( QRegularExpression( QStringLiteral( ".grpy$" ) ), "" ) ).arg( it->first + 1 )
         QFileInfo( grpy_last_processed ).completeBaseName().replace( QRegularExpression( QStringLiteral( ".grpy$" ) ), "" )
         ;
      this_data.results.used_beads            = grpy_last_used_beads;
      this_data.results.total_beads           = total_beads_count( bead_models[ grpy_last_model_number ] );
      this_data.num_of_unused                 = this_data.results.total_beads - this_data.results.used_beads;
      this_data.results.vbar                  = use_vbar( model_vector[ grpy_last_model_number ].vbar );
      this_data.proc_time                     = (double)(timers.times[ "compute grpy this model" ]) / 1e3;
      this_data.results.asa_rg_pos            = model_vector[ grpy_last_model_number ].asa_rg_pos;
      this_data.results.asa_rg_neg            = model_vector[ grpy_last_model_number ].asa_rg_neg;
      this_data.rot_diff_coef_x               = grpy_captures[ "rot_diff_coef_x" ][0];
      this_data.rot_diff_coef_y               = grpy_captures[ "rot_diff_coef_y" ][0];
      this_data.rot_diff_coef_z               = grpy_captures[ "rot_diff_coef_z" ][0];

      this_data.fractal_dimension_parameters         = model_vector[ grpy_last_model_number ].fractal_dimension_parameters;
      this_data.fractal_dimension                    = model_vector[ grpy_last_model_number ].fractal_dimension;
      this_data.fractal_dimension_sd                 = model_vector[ grpy_last_model_number ].fractal_dimension_sd;
      this_data.fractal_dimension_wtd                = model_vector[ grpy_last_model_number ].fractal_dimension_wtd;
      this_data.fractal_dimension_wtd_sd             = model_vector[ grpy_last_model_number ].fractal_dimension_wtd_sd;
      this_data.fractal_dimension_wtd_wtd            = model_vector[ grpy_last_model_number ].fractal_dimension_wtd_wtd;
      this_data.fractal_dimension_wtd_wtd_sd         = model_vector[ grpy_last_model_number ].fractal_dimension_wtd_wtd_sd;
      this_data.rg_over_fractal_dimension            = model_vector[ grpy_last_model_number ].rg_over_fractal_dimension;
      this_data.rg_over_fractal_dimension_sd         = model_vector[ grpy_last_model_number ].rg_over_fractal_dimension_sd;
      this_data.rg_over_fractal_dimension_wtd        = model_vector[ grpy_last_model_number ].rg_over_fractal_dimension_wtd;
      this_data.rg_over_fractal_dimension_wtd_sd     = model_vector[ grpy_last_model_number ].rg_over_fractal_dimension_wtd_sd;
      this_data.rg_over_fractal_dimension_wtd_wtd    = model_vector[ grpy_last_model_number ].rg_over_fractal_dimension_wtd_wtd;
      this_data.rg_over_fractal_dimension_wtd_wtd_sd = model_vector[ grpy_last_model_number ].rg_over_fractal_dimension_wtd_wtd_sd;

      if ( !hydro.grpy_bead_inclusion && grpy_vdw ) {
         this_data.vdw_grpy_probe_radius      = asa.vdw_grpy_probe_radius;
         this_data.vdw_grpy_threshold         = asa.vdw_grpy_threshold_percent;
      }

      if ( bead_models[ grpy_last_model_number ].size() &&
           bead_models[ grpy_last_model_number ][0].is_vdw == "vdw" ) {
         this_data.hydrate_probe_radius          = bead_models[ grpy_last_model_number ][0].asa_hydrate_probe_radius;
         this_data.hydrate_threshold             = bead_models[ grpy_last_model_number ][0].asa_hydrate_threshold;
         this_data.vdw_theo_waters               = bead_models[ grpy_last_model_number ][0].vdw_theo_waters;
         this_data.vdw_exposed_residues          = bead_models[ grpy_last_model_number ][0].vdw_count_exposed;
         this_data.vdw_exposed_waters            = bead_models[ grpy_last_model_number ][0].vdw_theo_waters_exposed;
      }

      // qDebug() << "US_Hydrodyn::grpy_finished() asa rg pos " << this_data.results.asa_rg_pos << " neg " << this_data.results.asa_rg_neg;
      
      if ( it->second.count( "Dr" ) ) {
         this_data.rot_diff_coef = it->second[ "Dr" ];
         // qDebug() << "this_data.rot_diff_coef : " << this_data.rot_diff_coef;
      }
      if ( it->second.count( "s" ) ) {
         this_data.results.s20w = it->second[ "s" ];
      }
      if ( it->second.count( "Dt" ) ) {
         this_data.results.D20w = it->second[ "Dt" ];
      }
      // Shell reduction (issue 984): when intrinsic viscosity was not converged it is
      // withheld from the reported results entirely rather than propagated with a caveat
      // -- a value carrying a warning is still a value that gets used downstream. It
      // remains in the on-disk report, annotated, for the record.
      if ( it->second.count( "\\[eta\\]" ) && !grpy_viscosity_unreliable ) {
         this_data.results.viscosity = it->second[ "\\[eta\\]" ];
      }
      if ( it->second.count( "tau1" ) ) {
         this_data.rel_times_tau_1 = it->second[ "tau1" ] * 1e9;
      }
      if ( it->second.count( "tau2" ) ) {
         this_data.rel_times_tau_2 = it->second[ "tau2" ] * 1e9;
      }
      if ( it->second.count( "tau3" ) ) {
         this_data.rel_times_tau_3 = it->second[ "tau3" ] * 1e9;
      }
      if ( it->second.count( "tau4" ) ) {
         this_data.rel_times_tau_4 = it->second[ "tau4" ] * 1e9;
      }
      if ( it->second.count( "tau5" ) ) {
         this_data.rel_times_tau_5 = it->second[ "tau5" ] * 1e9;
      }
      if ( it->second.count( "tauh" ) ) {
         this_data.rel_times_tau_h = it->second[ "tauh" ] * 1e9;
      }

      if ( it->second.count( "R\\(Rotation\\)" ) ) {
         // this_data.results.mass = it->second[ "R\\(Rotation\\)" ];
      }
      if ( it->second.count( "rs" ) ) {
         this_data.results.rs = it->second[ "rs" ] * 1e7;
      }
      // The Einstein radius is parsed from the SAME report line as the viscosity
      // ("Zero frequency intrinsic viscosity eta 0", fields 1 and 2) and is
      // viscosity-derived, so it inherits the same unreliability and must be withheld
      // together with it -- otherwise a trustworthy-looking radius would carry the
      // identical error.
      if ( it->second.count( "grpy_einst_rad" ) && !grpy_viscosity_unreliable ) {
         this_data.grpy_einst_rad = it->second[ "grpy_einst_rad" ] * 1e7;
      }
      
      // calculated params

      {
         double fconv = pow(10.0, this_data.hydro.unit + 9);
         this_data.con_factor = fconv;
         // bead model rg

         if ( this_data.results.mass ) {
            // compute center of mass
            bead_model = bead_models[ grpy_last_model_number ];
            point cm;
            cm.axis[ 0 ] = 0.0;
            cm.axis[ 1 ] = 0.0;
            cm.axis[ 2 ] = 0.0;
            double total_cm_mw = 0e0;

            for ( unsigned int i = 0; i < bead_model.size(); ++i )
            {
               PDB_atom *this_atom = &(bead_model[i]);
               double bead_tot_mw = this_atom->bead_ref_mw + this_atom->bead_ref_ionized_mw_delta;
               cm.axis[ 0 ] += bead_tot_mw * this_atom->bead_coordinate.axis[ 0 ];
               cm.axis[ 1 ] += bead_tot_mw * this_atom->bead_coordinate.axis[ 1 ];
               cm.axis[ 2 ] += bead_tot_mw * this_atom->bead_coordinate.axis[ 2 ];
               total_cm_mw += bead_tot_mw;
            }

            cm.axis[ 0 ] /= total_cm_mw;
            cm.axis[ 1 ] /= total_cm_mw;
            cm.axis[ 2 ] /= total_cm_mw;

            // now compute Rg
            double Rg2 = 0e0;

            for ( unsigned int i = 0; i < bead_model.size(); ++i )
            {
               PDB_atom *this_atom = &(bead_model[i]);
               Rg2 += ( (double) this_atom->bead_ref_mw + this_atom->bead_ref_ionized_mw_delta ) * 
                  ( 
                   (double) ( this_atom->bead_coordinate.axis[ 0 ] - cm.axis[ 0 ] ) *
                   ( this_atom->bead_coordinate.axis[ 0 ] - cm.axis[ 0 ] ) +
                   (double) ( this_atom->bead_coordinate.axis[ 1 ] - cm.axis[ 1 ] ) *
                   ( this_atom->bead_coordinate.axis[ 1 ] - cm.axis[ 1 ] ) +
                   (double) ( this_atom->bead_coordinate.axis[ 2 ] - cm.axis[ 2 ] ) *
                   ( this_atom->bead_coordinate.axis[ 2 ] - cm.axis[ 2 ] ) 
                    );
            }

            double Rg = sqrt( Rg2 / total_cm_mw );

            this_data.results.rg = Rg * fconv;
            grpy_results.rg  += this_data.results.rg;
            grpy_results2.rg += this_data.results.rg * this_data.results.rg;
            us_qdebug( QString( "rg %1 fconv %2 rg2 %3 total_cm_mw %4" ).arg( Rg ).arg( fconv ).arg( Rg2 ).arg( total_cm_mw ) );
         }

         // tra_fric_coef

         {
            if ( this_data.results.D20w ) {
               this_data.tra_fric_coef =
                  Rbar * ( K0 + hydro.temperature ) / ( AVOGADRO * this_data.results.D20w );

               QTextStream( stdout )
                  << "tfc " << this_data.tra_fric_coef << " = " << Qt::endl
                  << "Rbar * ( K0 + hydro.temperature )" << ( Rbar * ( K0 + hydro.temperature ) ) << " / " << Qt::endl
                  << "AVOGADRO * this_data.results.D20w" << ( AVOGADRO * this_data.results.D20w ) << Qt::endl
                  ;
               
            }
         }

         {
            if ( this_data.rot_diff_coef ) {
               this_data.rot_fric_coef =
                  Rbar * ( K0 + hydro.temperature ) / ( AVOGADRO * this_data.rot_diff_coef );

               // QTextStream( stdout )
               //    << "rfc " << this_data.rot_fric_coef << " = " << Qt::endl
               //    << "Rbar * ( K0 + hydro.temperature )" << ( Rbar * ( K0 + hydro.temperature ) ) << " / " << Qt::endl
               //    << "AVOGADRO * this_data.rot_diff_coef" << ( AVOGADRO * this_data.rot_diff_coef ) << Qt::endl
               //    ;
               
            }
         }

         // already computed
         // // stokes radius
         // {
         //    if ( this_data.results.D20w ) {
         //       this_data.results.rs = 1e1 * ( 1e7 / fconv ) * 
         //          Rbar * ( K0 + hydro.temperature ) / ( AVOGADRO * 6.0 * M_PI * use_solvent_visc() * this_data.results.D20w );
         //       grpy_results.rs  += this_data.results.rs;
         //       grpy_results2.rs += this_data.results.rs * this_data.results.rs;
         //    }
         // }

         // need ff0 & stokes radius

         {
            QTextStream(stdout)
               << "f    = " << this_data.tra_fric_coef << Qt::endl
               << "Rs   = " << this_data.results.rs << Qt::endl
               << "f0   = " << ( this_data.results.rs * 6.0 * M_PI * use_solvent_visc() ) << Qt::endl
               << "f/f0 = " << ( this_data.tra_fric_coef / ( this_data.results.rs * 6.0 * M_PI * use_solvent_visc() ) ) << Qt::endl
               ;
            
            this_data.results.ff0 = 
               this_data.tra_fric_coef * 10 / 
               ( fconv * 6e0 * M_PI *  use_solvent_visc() * 
                 pow( 3.0 * this_data.results.mass * this_data.results.vbar / (4.0 * M_PI * AVOGADRO), 1.0/3.0 ) );

            QTextStream( stdout )
               << "ff0 " << this_data.results.ff0 << " = " << Qt::endl
               ;

            // this_data.results.ff0_sd = this_data.results.ff0 * this_data.tra_fric_coef_sd / this_data.tra_fric_coef;
            grpy_results.ff0  += this_data.results.ff0;
            grpy_results2.ff0 += this_data.results.ff0 * this_data.results.ff0;
         }

         // nsa physical stats

         {
            vector < vector < PDB_atom > >  save_bead_models = bead_models;
            saxs_util->bead_models.resize( 1 );
            saxs_util->bead_models[ 0 ] = bead_models[ grpy_last_model_number ];
            if ( "empty model" != saxs_util->nsa_physical_stats() )
            {
               this_data.tot_volume_of = saxs_util->nsa_physical_stats_map[ "result excluded volume" ].toDouble();
                  
               this_data.max_ext_x = saxs_util->nsa_physical_stats_map[ "result radial extent bounding box size x" ].toDouble() * fconv;
               this_data.max_ext_y = saxs_util->nsa_physical_stats_map[ "result radial extent bounding box size y" ].toDouble() * fconv;
               this_data.max_ext_z = saxs_util->nsa_physical_stats_map[ "result radial extent bounding box size z" ].toDouble() * fconv;

               this_data.axi_ratios_xz = saxs_util->nsa_physical_stats_map[ "result radial extent axial ratios x:z" ].toDouble();
               this_data.axi_ratios_xy = saxs_util->nsa_physical_stats_map[ "result radial extent axial ratios x:y" ].toDouble();
               this_data.axi_ratios_yz = saxs_util->nsa_physical_stats_map[ "result radial extent axial ratios y:z" ].toDouble();
            } else {
               editor_msg( "red", QString( "Internal error: Bead model is empty?" ) );
            }
         }

         // rot fric and stokes
         {
            double factor = 1e-2 * (Rbar/AVOGADRO) * ( K0 + hydro.temperature ) / pow( fconv, 2 );
            this_data.rot_fric_coef_x =
               factor / this_data.rot_diff_coef_x;
            this_data.rot_fric_coef_y =
               factor / this_data.rot_diff_coef_y;
            this_data.rot_fric_coef_z =
               factor / this_data.rot_diff_coef_z;

            this_data.rot_stokes_rad_x =
               1e8 * fconv * pow( this_data.rot_fric_coef_x / ( 8.0 * M_PI * use_solvent_visc() * 1e-2 ), 1.0/3.0 );
            this_data.rot_stokes_rad_y =
               1e8 * fconv * pow( this_data.rot_fric_coef_y / ( 8.0 * M_PI * use_solvent_visc() * 1e-2 ), 1.0/3.0 );
            this_data.rot_stokes_rad_z =
               1e8 * fconv * pow( this_data.rot_fric_coef_z / ( 8.0 * M_PI * use_solvent_visc() * 1e-2 ), 1.0/3.0 );
         }
      }

      this_data.rel_times_tau_m =
         (
          this_data.rel_times_tau_1 +
          this_data.rel_times_tau_2 +
          this_data.rel_times_tau_3 +
          this_data.rel_times_tau_4 +
          this_data.rel_times_tau_5
          ) / 5.0;

      this_data.results.tau = this_data.rel_times_tau_h;
                                                                        
      // qDebug() << "GRPY accumulating data for " << this_data.results.name;

      grpy_results .name                 +=
         // ( grpy_results.name.isEmpty() ? QString( this_data.results.name + " " ) : QString( ",-%1" ).arg( grpy_last_model_number + 1 ) )
         this_data.results.name + " "
         ;
      grpy_results .mass                 += this_data.results.mass;
      grpy_results2.mass                 += this_data.results.mass        * this_data.results.mass;
      grpy_results .used_beads           += this_data.results.used_beads;
      grpy_results2.used_beads           += this_data.results.used_beads  * this_data.results.used_beads;
      grpy_results .total_beads          += this_data.results.total_beads;
      grpy_results2.total_beads          += this_data.results.total_beads * this_data.results.total_beads;
      grpy_results .vbar                 += this_data.results.vbar;
      grpy_results2.vbar                 += this_data.results.vbar        * this_data.results.vbar;
      grpy_results .num_models           ++ ;


      
      {
         QString add_to_grpy;

         add_to_grpy +=
            "\n---------------------------------------------------------\n"
            + us_tr(
                    "Summary information\n"
                    "The Translational Frictional Coefficient and Frictional Ratio are\n"
                    "derived from the GRPY results.\n"
                    )
            + "---------------------------------------------------------\n"
            ;
       

         add_to_grpy += QString( "\nGRPY computed on %1 Model %2%3\n" ).arg( project ).arg( grpy_last_model_number + 1 ).arg( bead_model_suffix.length() ? (" Bead model suffix: " + bead_model_suffix) : "" );
         add_to_grpy += QString( "Number of beads used: %1\n" ).arg( this_data.results.used_beads );
         add_to_grpy += QString( "MW: %1 [Da]\n" ).arg( this_data.results.mass );
         add_to_grpy += pH_msg();
         add_to_grpy += vbar_msg( this_data.results.vbar, true );
         add_to_grpy += visc_dens_msg( true );
                     
         if ( hydro.mass_correction ) {
            add_to_grpy += QString( "Manually corrected MW: %1 [Da]\n" ).arg( hydro.mass );
         }

         add_to_grpy += 
            QString( 
                    us_tr( 
                          "\n"
                          "US-SOMO Derived Parameters:\n"
                          "\n"
                          " Frictional Ratio                   f/f0 : %1\n"
                          " Tr. Frictional coefficient            f : %2\n"
                          " Radius of Gyration                   Rg : %3\n"
                           ) )
            .arg( QString::asprintf( "%3.2f"     , this_data.results.ff0 ) )
            .arg( QString::asprintf( "%4.2e g/s" , this_data.tra_fric_coef ) )
            .arg( QString::asprintf( "%4.2e nm"  , this_data.results.rg ) )
            ;

         add_to_grpy += 
            QString( 
                    us_tr( 
                          " Rot. Diffusion coefficient            X : %1 \t1/s (w@20C)\n"
                          " Rot. Diffusion coefficient            Y : %2 \t1/s (w@20C)\n"
                          " Rot. Diffusion coefficient            Z : %3 \t1/s (w@20C)\n"
                          " Rot. Frictional coefficient           X : %4 \tg*cm^2/s (w@20C)\n"
                          " Rot. Frictional coefficient           Y : %5 \tg*cm^2/s (w@20C)\n"
                          " Rot. Frictional coefficient           Z : %6 \tg*cm^2/s (w@20C)\n"
                          " Rot. Stokes' radius                   X : %7 \tnm (w@20C)\n"
                          " Rot. Stokes' radius                   Y : %8 \tnm (w@20C)\n"
                          " Rot. Stokes' radius                   Z : %9 \tnm (w@20C)\n"
                           ) )
            .arg( QString::asprintf( "%5.4g"           , this_data.rot_diff_coef_x ) )
            .arg( QString::asprintf( "%5.4g"           , this_data.rot_diff_coef_y ) )
            .arg( QString::asprintf( "%5.4g"           , this_data.rot_diff_coef_z ) )
            .arg( QString::asprintf( "%5.4g"           , this_data.rot_fric_coef_x ) )
            .arg( QString::asprintf( "%5.4g"           , this_data.rot_fric_coef_y ) )
            .arg( QString::asprintf( "%5.4g"           , this_data.rot_fric_coef_z ) )
            .arg( QString::asprintf( "%5.4g"           , this_data.rot_stokes_rad_x ) )
            .arg( QString::asprintf( "%5.4g"           , this_data.rot_stokes_rad_y ) )
            .arg( QString::asprintf( "%5.4g"           , this_data.rot_stokes_rad_z ) )
            ;

         add_to_grpy +=
            QString(
                    us_tr( 
                          " Maximum extension                     X : %1\n"
                          " Maximum extension                     Y : %2\n"
                          " Maximum extension                     Z : %3\n"
                          " Axial ratio                         X:Z : %4\n"
                          " Axial ratio                         X:Y : %5\n"
                          " Axial ratio                         Y:Z : %6\n"
                           ) )
            .arg( QString( "%1 nm" ).arg( this_data.max_ext_x, 0, 'g', 4 ) )
            .arg( QString( "%1 nm" ).arg( this_data.max_ext_y, 0, 'g', 4 ) )
            .arg( QString( "%1 nm" ).arg( this_data.max_ext_z, 0, 'g', 4 ) )
            .arg( this_data.axi_ratios_xz, 0, 'g', 3 )
            .arg( this_data.axi_ratios_xy, 0, 'g', 3 )
            .arg( this_data.axi_ratios_yz, 0, 'g', 3 )
            ;

         if ( !batch_avg_hydro_active() && !grpy_mm ) {
            QFile f( last_hydro_res );
            if ( f.exists() && f.open( QIODevice::WriteOnly | QIODevice::Append ) )
            {
               QTextStream ts( &f );
               ts << add_to_grpy;
               f.close();
            }
         }

         this_data.hydro_res = grpy_stdout + add_to_grpy;
         if ( grpy_mm ) {
            grpy_mm_results += this_data.hydro_res;
         }
      }
      if ( grpy_mm ) {
         grpy_mm_save_params.data_vector.push_back( this_data );
      }

      if ( batch_widget &&
           batch_window->save_batch_active )
      {
         save_params.data_vector.push_back( this_data );
      }

      bool create_hydro_res = !(batch_widget &&
                                batch_window->save_batch_active);

      if ( saveParams && create_hydro_res && !grpy_mm )
      {
         QString fname = get_somo_dir() + "/" + this_data.results.name + grpy_settings_sfx + ".grpy.csv";
         if ( !overwrite_hydro ) {
            GRPY_PAUSE_TIMER;   // operator response time is not compute
            fname = fileNameCheck( fname, 0, this );
         }

         FILE *of = us_fopen(fname, "wb");
         if ( of )
         {
            fprintf(of, "%s", save_util->header().toLatin1().data());

            fprintf(of, "%s", save_util->dataString(&this_data).toLatin1().data());
            fclose(of);
            editor_msg( "dark blue", QString( "created %1\n" ).arg( fname ) );
         }
      }
      // // print out results ?
      // save_util->header();
      // save_util->dataString(&this_data);
   }

   
   grpy_process_next();
}

void US_Hydrodyn::grpy_finalize() {
   // qDebug() << "US_Hydrodyn::grpy_finalize():" << grpy_last_processed;
   // us_qdebug( QString( "grpy_finalize %1" ).arg( grpy_last_processed ) );
   editor_msg( "black", "Finalizing GRPY results" );
   // for ( map < QString, vector < double > >::iterator it = grpy_captures.begin();
   //       it != grpy_captures.end();
   //       ++it ) {
   //    editor_msg( "dark red",  US_Vector::qs_vector( it->first, it->second ) );
   // }

   {
      double num = (double) grpy_results.num_models;
      if ( num <= 1 ) {
         results = grpy_results;
      } else {
         double numinv = 1e0 / num;
         grpy_results.mass              *= numinv;
         grpy_results.s20w              *= numinv;
         grpy_results.D20w              *= numinv;
         grpy_results.viscosity         *= numinv;
         grpy_results.tau               *= numinv;
         grpy_results.rs                *= numinv;
         grpy_results.rg                *= numinv;
         grpy_results.vbar              *= numinv;
         grpy_results.ff0               *= numinv;
         grpy_results.used_beads        *= numinv;
         grpy_results.total_beads       *= numinv;
         if ( num <= 1 ) {
            results = grpy_results;
         } else {
            double numdecinv = 1e0 / ( num - 1e0 );
         
            grpy_results.s20w_sd              = sqrt( fabs( ( grpy_results2.s20w              - grpy_results.s20w              * grpy_results.s20w              * num ) * numdecinv ) );
            grpy_results.D20w_sd              = sqrt( fabs( ( grpy_results2.D20w              - grpy_results.D20w              * grpy_results.D20w              * num ) * numdecinv ) );
            grpy_results.viscosity_sd         = sqrt( fabs( ( grpy_results2.viscosity         - grpy_results.viscosity         * grpy_results.viscosity         * num ) * numdecinv ) );
            grpy_results.tau_sd               = sqrt( fabs( ( grpy_results2.tau               - grpy_results.tau               * grpy_results.tau               * num ) * numdecinv ) );
            grpy_results.rs_sd                = sqrt( fabs( ( grpy_results2.rs                - grpy_results.rs                * grpy_results.rs                * num ) * numdecinv ) );
            grpy_results.rg_sd                = sqrt( fabs( ( grpy_results2.rg                - grpy_results.rg                * grpy_results.rg                * num ) * numdecinv ) );
            grpy_results.ff0_sd               = sqrt( fabs( ( grpy_results2.ff0               - grpy_results.ff0               * grpy_results.ff0               * num ) * numdecinv ) );
            grpy_results.used_beads_sd        = sqrt( fabs( ( grpy_results2.used_beads        - (double) grpy_results.used_beads  * grpy_results.used_beads     * num ) * numdecinv ) );
            grpy_results.total_beads_sd       = sqrt( fabs( ( grpy_results2.total_beads       - (double) grpy_results.total_beads * grpy_results.total_beads    * num ) * numdecinv ) );
            
            results = grpy_results;
         }
      }
   }

   if ( grpy_mm ) {
      vector < save_data > stats = save_util->stats( & grpy_mm_save_params.data_vector );

      {
         QString grpy_out_name = grpy_mm_name + grpy_settings_sfx + ".grpy_res";
         if ( !overwrite_hydro ) {
            GRPY_PAUSE_TIMER;   // operator response time is not compute
            grpy_out_name = fileNameCheck( grpy_out_name, 0, this );
         }
      
         QFile f( grpy_out_name );
         if ( !f.open( QIODevice::WriteOnly ) ) {
            editor_msg( "red", QString( us_tr( "Error: could not open output file %1 for writing" ) ).arg( grpy_out_name ) );
         } else {
            vector < save_data > stats = save_util->stats( & grpy_mm_save_params.data_vector );
            QTextStream t( &f );
            t << grpy_mm_results;
            t << save_util->hydroFormatStats( stats, US_Hydrodyn_Save::HYDRO_GRPY );
            editor_msg( "dark blue", QString( us_tr( "Wrote %1" ) ).arg( grpy_out_name ) );
            f.close();
            last_hydro_res = QFileInfo( grpy_out_name ).fileName();
         }
      }

      bool create_hydro_res = !(batch_widget &&
                                batch_window->save_batch_active);

      if ( saveParams && create_hydro_res ) {
         QString grpy_out_name = grpy_mm_name + grpy_settings_sfx + ".grpy.csv";
         if ( !overwrite_hydro ) {
            GRPY_PAUSE_TIMER;   // operator response time is not compute
            grpy_out_name = fileNameCheck( grpy_out_name, 0, this );
         }
         QFile f( grpy_out_name );
         if ( !f.open( QIODevice::WriteOnly ) ) {
            editor_msg( "red", QString( us_tr( "Error: could not open output file %1 for writing" ) ).arg( grpy_out_name ) );
         } else {
            QTextStream t( &f );
            t << save_util->header().toLatin1().data();

            for ( int i = 0; i < (int) grpy_mm_save_params.data_vector.size(); ++i ) {
               t << save_util->dataString( & grpy_mm_save_params.data_vector[ i ] ).toLatin1().data();
            }
            for ( int i = 0; i < (int) stats.size(); ++i ) {
               t << save_util->dataString( & stats[ i ] ).toLatin1().data();
            }
               
            editor_msg( "dark blue", QString( us_tr( "Wrote %1" ) ).arg( grpy_out_name ) );
            f.close();
         }
      }
   }

   // qDebug() << "hydro_results enabled";
   
   pb_show_hydro_results->setEnabled( true );
   grpy_running = false;
   set_enabled();
   pb_calc_hydro->setEnabled(grpy_was_hydro_enabled);
   pb_calc_zeno->setEnabled(true);
   pb_bead_saxs->setEnabled(true);
   pb_calc_grpy->setEnabled( true );
   pb_calc_hullrad->setEnabled( true );
   pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   progress->reset();
   mprogress->hide();
   lbl_core_progress->setText( "" );
   editor_msg( "black", "GRPY finished" );
   timers.end_timer( "compute grpy all models" );
   editor_msg( "black", QString( "Time to process %1").arg( timers.time_min_sec( "compute grpy all models" ) ) );
   editor_msg( "dark blue", info_cite( "grpy" ) );

   // us_qdebug( QString( "grpy_finalize %1 end" ).arg( grpy_last_processed ) );
}
