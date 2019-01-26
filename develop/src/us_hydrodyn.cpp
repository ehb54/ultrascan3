// (this) us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
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
#include "../include/us_revision.h"
#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_best.h"
#include <qregexp.h>
#include <qfont.h>
//Added by qt3to4:
#include <QBoxLayout>
#include <QFrame>
#include <QLabel>
 //#include <Q3PopupMenu>
#include <QHBoxLayout>
#include <QTextStream>
#include <QGridLayout>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <qtimer.h>

// #define NO_BD

#undef DEBUG

#ifndef WIN32
#   include <unistd.h>
#   define SLASH "/"
#   define __open open
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

// #define PINV_TEST
#if defined( PINV_TEST )
#include "../include/us_svd.h"
#endif
// #define SVD_TEST
#if defined( SVD_TEST )
#include "../include/us_svd.h"
#endif
// #define USE_H

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

// #define GRID_HYDRATE_DEBUG

// static bool no_rr;

US_Hydrodyn::US_Hydrodyn(vector < QString > batch_file,
                         QWidget *p, 
                         const char *name) : QFrame( p )
{
#if defined( PINV_TEST )
   qDebug() << "PINV_TEST";

   vector < vector < double > > A =
      { { 1, 2, 3 },
        { 4, 5, 6 } }
      // { { 1, 4 },
      //   { 2, 5 },
      //   { 3, 6 } }
   ;
   vector < vector < double > > Ainv;

   SVD::pinv( A, Ainv );
   qDebug() << "PINV_TEST done";
   exit(0);
#endif

#if defined( SVD_TEST )
   qDebug() << "SVD_TEST";

   vector < vector < double > > A2x3 =
        { { 1, 2, 3 },
          { 4, 5, 6 } };
   
   vector < vector < double > > A3x2 =
      { { 1, 4 },
        { 2, 5 },
        { 3, 6 } }
   ;

   vector < vector < double > > A = A2x3;

   int m = (int) A.size();
   int n = (int) A[0].size();
   
   qDebug() << "SVD_TEST 1";
   
   double a[ m ][ n ];
   for ( int i = 0; i < m; ++i ) {
      for ( int j = 0; j < n; ++j) {
         a[ i ][ j ] = A[ i ][ j ];
      }
   }

   qDebug() << "SVD_TEST 2";
   double *ause[ m ];
   for ( int j = 0; j < m; ++j ) {
      ause[ j ] = a[ j ];
   }
   
   qDebug() << "SVD_TEST 3";

   SVD::cout_dpp( "a before svd", ause, m, n );

   qDebug() << "SVD_TEST 4";
   // w can stay the same?

   double w[ n ];
         
   // allocate vt
   double v[ n ][ n ];

   double *vuse[ n ];

   for ( int i = 0; i < n; ++i ) {
      vuse[ i ] = v[ i ];
   }
   qDebug() << "SVD_TEST 5";

   if ( !SVD::dsvd( ause, m, n, w, vuse ) ) {
      qDebug() << "SVD failed";
      exit(0);
   }

   // convert to vvd, vd

   vector < vector < double > > u_vvd;
   SVD::dpp_to_vvd( ause, m, m, u_vvd );
   vector < vector < double > > v_vvd;
   SVD::dpp_to_vvd( vuse, n, n, v_vvd );
   vector < double > w_vd;
   SVD::dp_to_vd( w, n, w_vd );
   
   SVD::cout_vvd( "u", u_vvd );
   SVD::cout_vd( "w", w_vd );
   SVD::cout_vvd( "v", v_vvd );

   vector < vector < double > > uw = SVD::vvd_usmult( u_vvd, w_vd );
   SVD::cout_vvd( "uw", uw );

   vector < vector < double > > vt = SVD::vvd_transpose( v_vvd );

   vector < vector < double > > uwvt = SVD::vvd_mult( uw, v_vvd );
   SVD::cout_vvd( "uwvt", uwvt );

   qDebug() << "max norm org - uwvt " << SVD::vvd2_maxnorm( uwvt, A );

#if defined( OLD_SVD_TEST )
   SVD::cout_dpp( "u", ause, m, m );
   SVD::cout_dp( "w", w, n );
   SVD::cout_dpp( "v", vuse, n, n );

   vector < vector < double > > us( m );
   for ( int i = 0; i < m; ++i ) {
      us[ i ].resize( n );
   }

   vector < vector < double > > afinal = us;

   // mult u * w
   for ( int i = 0; i < m; ++i ) {
      for ( int j = 0; j < n; ++j ) {
         us[ i ][ j ] = ause[ i ][ j ] * w[ j ];
      }
   }

   SVD::cout_vvd( "us(u*s) ", us );

   for ( int i = 0; i < m; ++i ) {
      for ( int j = 0; j < n; ++j ) {
         afinal[ i ][ j ] = 0;
         for ( int k = 0; k < n; ++k ) {
            afinal[ i ][ j ] += us[ i ][ k ] * vuse[ j ][ k ] ;
         }
      }
   }

   SVD::cout_vvd( "afinal ", afinal );

#endif
   
   exit(0);
#endif

   USglobal = new US_Config();
   this->batch_file = batch_file;
   numThreads = USglobal->config_list.numThreads;
   extra_saxs_coefficients.clear( );
   hullrad_running = false;

   last_pdb_filename = "";
   last_pdb_title.clear( );
   last_pdb_header.clear( );
   last_no_model_selected = false;

   // no_rr = false;

   // int r_stdout = __open(QString(somo_tmp_dir +
   //           SLASH + "last_stdout.txt").toLatin1().data(),
   //        O_WRONLY | O_CREAT | O_TRUNC, 0666);
   // dup2(r_stdout, STDOUT_FILENO);

   // int r_stderr = __open(QString(somo_tmp_dir +
   //           SLASH + "last_stderr.txt").toLatin1().data(),
   //        O_WRONLY | O_CREAT | O_TRUNC, 0666);
   // dup2(r_stderr, STDERR_FILENO);

   {
      QString tmp_dir = USglobal->config_list.root_dir + SLASH + "etc";
      QDir dir1( tmp_dir );
      if (!dir1.exists())
      {
         dir1.mkdir(tmp_dir);
      }
   }

   somo_dir = USglobal->config_list.root_dir + SLASH +  "somo";
   QDir dir1(somo_dir);
   if (!dir1.exists())
   {
      dir1.mkdir(somo_dir);
   }
   somo_pdb_dir = somo_dir + SLASH + "structures";
   QDir dir2(somo_pdb_dir);
   if (!dir2.exists())
   {
      dir2.mkdir(somo_pdb_dir);
   }
   somo_tmp_dir = somo_dir + SLASH + "tmp";
   QDir dir3(somo_tmp_dir);
   if (!dir3.exists())
   {
      dir3.mkdir(somo_tmp_dir);
   }
   QString somo_saxs_dir = somo_dir + SLASH + "saxs";
   QDir dir4(somo_saxs_dir);
   if (!dir4.exists())
   {
      dir4.mkdir(somo_saxs_dir);
   }

   QString somo_saxs_tmp_dir = somo_dir + SLASH + "saxs" + SLASH + "tmp";
   QDir dir5(somo_saxs_tmp_dir);
   if (!dir5.exists())
   {
      dir5.mkdir(somo_saxs_tmp_dir);
   }

   QString somo_bd_dir = somo_dir + SLASH + "bd";
   QDir dir6(somo_bd_dir);
   if (!dir6.exists())
   {
      dir6.mkdir(somo_bd_dir);
   }

   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("SOMO Solution Bead Modeler"));
   advanced_config.auto_view_pdb = true;
   advanced_config.scroll_editor = false;
   advanced_config.auto_calc_somo = false;
   advanced_config.auto_show_hydro = false;
   advanced_config.pbr_broken_logic = true;
   advanced_config.use_sounds = false;
   advanced_config.expert_mode = false;
   advanced_config.experimental_threads = false;
   advanced_config.experimental_renum = false;
   advanced_config.debug_1 = false;
   advanced_config.debug_2 = false;
   advanced_config.debug_3 = false;
   advanced_config.debug_4 = false;
   advanced_config.debug_5 = false;
   advanced_config.debug_6 = false;
   advanced_config.debug_7 = false;
   atom_widget                     = false;
   best_widget                     = false;
   residue_widget                  = false;
   hybrid_widget                   = false;
   saxs_widget                     = false;
   saxs_options_widget             = false;
   sas_options_saxs_widget         = false;
   sas_options_sans_widget         = false;
   sas_options_curve_widget        = false;
   sas_options_bead_model_widget   = false;
   sas_options_hydration_widget    = false;
   sas_options_guinier_widget      = false;
   sas_options_xsr_widget          = false;
   sas_options_misc_widget         = false;
   sas_options_experimental_widget = false;
   saxs_plot_widget                = false;
   asa_widget = false;
   misc_widget = false;
   grid_widget = false;
   hydro_widget = false;
   hydro_zeno_widget = false;
   overlap_widget = false;
   grid_overlap_widget = false;
   bead_output_widget = false;
   results_widget = false;
   pdb_visualization_widget = false;
   pdb_parsing_widget = false;
   advanced_config_widget = false;
   saxs_search_widget = false;
   saxs_screen_widget = false;
   saxs_buffer_widget = false;
   saxs_hplc_widget = false;
   saxs_1d_widget = false;
   saxs_2d_widget = false;
   bd_widget = false;
   bd_options_widget = false;
   dmd_options_widget = false;
   anaflex_options_widget = false;
   batch_widget = false;
   save_widget = false;
   comparative_widget = false;
   if ( !install_new_version() )
   {
      exit(0);
   }
   set_default();   // setup configuration defaults before reading initial config
   read_config(""); // specify default configuration by leaving argument empty
   calcAutoHydro = false;
   overwrite = false;
   saveParams = false;
   setSuffix = true;
   guiFlag = true;
   bead_model_selected_filter = "";
   residue_filename = US_Config::get_home_dir() + "etc/somo.residue";
   editor = (QTextEdit *)0;

#if QT_VERSION >= 0x040000
   gparams[ "zeno_cxx" ] = "true";
#endif

   last_saxs_search_csv.name = "__empty__";
   last_saxs_screen_csv.name = "__empty__";
   last_saxs_buffer_csv.name = "__empty__";
   last_saxs_hplc_csv.name = "__empty__";

   misc.restore_pb_rule = false;

   read_residue_file();

   bd_ready_to_run = false;
   anaflex_ready_to_run = false;
   bd_last_file = "";
   anaflex_last_file = "";
   state = NOTHING_LOADED;
   setupGUI();
   //   global_Xpos += 30;
   //   global_Ypos += 30;
   //   setGeometry(global_Xpos, global_Ypos, 0, 0);
   create_beads_normally = true;
   alt_method = false;
   rasmol = NULL;
   browflex = NULL;
   anaflex = NULL;
   anaflex_return_to_bd_load_results = false;
   bd_anaflex_enables(false);

   last_read_bead_model = "";
   last_hydro_res = "";
   QDir::setCurrent(somo_tmp_dir);
   if ( advanced_config.debug_5 )
   {
      printf("%s\n", QString(somo_tmp_dir).toLatin1().data());
   }
   results.total_beads = 0;
   results.used_beads = 0;
   results.mass = 0.0;
   results.s20w = 0.0;
   results.D20w = 0.0;
   results.viscosity = 0.0;
   results.ff0 = 0.0;
   results.rs = 0.0;
   results.rg = 0.0;
   results.vbar = 0.72;
   results.tau = 0.0;
   results.s20w_sd = 0.0;
   results.D20w_sd = 0.0;
   results.viscosity_sd = 0.0;
   results.ff0_sd = 0.0;
   results.rs_sd = 0.0;
   results.rg_sd = 0.0;
   results.tau_sd = 0.0;
   results.asa_rg_pos = 0.0;
   results.asa_rg_neg = 0.0;

   // move to save/restore
   // put in set_default!!!!!

   // this should be stored in the residue file, hardcoded for now

   residue_short_names["GLY"] = 'G';
   residue_short_names["ALA"] = 'A';
   residue_short_names["VAL"] = 'V';
   residue_short_names["LEU"] = 'L';
   residue_short_names["ILE"] = 'I';
   residue_short_names["MET"] = 'M';
   residue_short_names["PHE"] = 'F';
   residue_short_names["TRP"] = 'W';
   residue_short_names["PRO"] = 'P';
   residue_short_names["SER"] = 'S';
   residue_short_names["THR"] = 'T';
   residue_short_names["CYS"] = 'C';
   residue_short_names["CYH"] = 'B';
   residue_short_names["TYR"] = 'Y';
   residue_short_names["ASN"] = 'N';
   residue_short_names["GLN"] = 'Q';
   residue_short_names["ASP"] = 'D';
   residue_short_names["GLU"] = 'E';
   residue_short_names["LYS"] = 'K';
   residue_short_names["ARG"] = 'R';
   residue_short_names["HIS"] = 'H';
   residue_short_names["WAT"] = '~';

   residue_short_names["G"] = 'g';
   residue_short_names["A"] = 'a';
   residue_short_names["C"] = 'c';
   residue_short_names["T"] = 't';
   residue_short_names["U"] = 'u';
   residue_short_names["DG"] = 'g';
   residue_short_names["DA"] = 'a';
   residue_short_names["DC"] = 'c';
   residue_short_names["DT"] = 't';

   create_fasta_vbar_mw();

   options_log = "";
   last_abb_msgs = "";

   comparative = US_Hydrodyn_Comparative::empty_comparative_info();

   rasmol = new QProcess(this);
   rasmol->setWorkingDirectory(
#if QT_VERSION < 0x040000
                               QDir(
#endif
                                    USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
                                     "bin64"
#else
                                    "/bin/"
#endif
				    + SLASH
#if QT_VERSION < 0x040000
                                    )
#endif
                               );

   bead_model_from_file = false;
   QString RMP = "RASMOLPATH=" + USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
     + SLASH
     ;
   bead_model_prefix = "";
   if (!getenv("RASMOLPATH"))
   {
      int n = RMP.length();
      // note this is not free'd because it becomes part of the environment....
      char* rmp = (char*) malloc (n + 1);
      if ( rmp )
      {
         strncpy( rmp, RMP.toLatin1().data(), n );
         *( rmp + n ) = 0;
         putenv( rmp );
      }
   }
   play_sounds(1);
   editor->append(QString(us_tr("\n\nWelcome to SOMO UltraScan %1 %2\nProduced on: %3\n"))
		  .arg(US_Version)
		  .arg(REVISION)
		  .arg(REVISION_DATE)
		  );
   if ( numThreads > 1 )
   {
      editor->append(QString(us_tr("Multi-threading enabled using %1 threads\n")).arg(numThreads));
   }
   QColor save_color = editor->textColor();
   editor->setTextColor("red");
   if (!dir1.exists())
   {
      editor->append(us_tr("Warning: Directory ") + somo_dir + us_tr(" does not exist.\n"));
   }
   if (!dir2.exists())
   {
      editor->append(us_tr("Warning: Directory ") + somo_pdb_dir + us_tr(" does not exist.\n"));
   }
   if (!dir3.exists())
   {
      editor->append(us_tr("Warning: Directory ") + somo_tmp_dir + us_tr(" does not exist.\n"));
   }
   if (!dir4.exists())
   {
      editor->append(us_tr("Warning: Directory ") + somo_saxs_dir + us_tr(" does not exist.\n"));
   }
   if (!dir5.exists())
   {
      editor->append(us_tr("Warning: Directory ") + somo_saxs_tmp_dir + us_tr(" does not exist.\n"));
   }
   if (!dir6.exists())
   {
      editor->append(us_tr("Warning: Directory ") + somo_bd_dir + us_tr(" does not exist.\n"));
   }
   editor->setTextColor(save_color);

   if ( batch_file.size() )
   {
      batch.file = batch_file;
      batch_window = new US_Hydrodyn_Batch(&batch, &batch_widget, this);
      fixWinButtons( batch_window );
      batch_window->show();
      batch_window->raise();
   }
   
   save_util = new US_Hydrodyn_Save(&save_params, this);
   fixWinButtons( save_util );
 
   if ( !gparams.count( "zeno_repeats" ) )
   {
      gparams[ "zeno_repeats" ] = "1";
   }
   if ( !gparams.count( "zeno_max_cap" ) )
   {
      gparams[ "zeno_max_cap" ] = "false";
   }
   if ( !gparams.count( "zeno_max_cap_pct" ) )
   {
      gparams[ "zeno_max_cap_pct" ] = "0.5";
   }

   if ( saxs_options.default_atom_filename.isEmpty() )
   {
      saxs_options.default_atom_filename = US_Config::get_home_dir() + "etc" + SLASH + "somo.atom";
   }
   if ( saxs_options.default_hybrid_filename.isEmpty() )
   {
      saxs_options.default_hybrid_filename = US_Config::get_home_dir() + "etc" + SLASH + "somo.hybrid";
   }
   if ( saxs_options.default_saxs_filename.isEmpty() )
   {
      saxs_options.default_saxs_filename = US_Config::get_home_dir() + "etc" + SLASH + "somo.saxs_atoms";
   }
   if ( saxs_options.default_rotamer_filename.isEmpty() )
   {
      saxs_options.default_rotamer_filename = US_Config::get_home_dir() + "etc" + SLASH + "somo.hydrated_rotamer";
   }
   if ( saxs_options.default_ff_filename.isEmpty() )
   {
      saxs_options.default_ff_filename = USglobal->config_list.system_dir + SLASH + "etc" + SLASH + "somo.ff";
   }

   saxs_util = new US_Saxs_Util();
   if ( 
       !saxs_util->setup_saxs_maps( 
                                   saxs_options.default_atom_filename ,
                                   saxs_options.default_hybrid_filename ,
                                   saxs_options.default_saxs_filename 
                                   )
       &&
       saxs_options.compute_saxs_coeff_for_bead_models
       )
   {
      QColor save_color = editor->textColor();
      editor->setTextColor("red");
      editor->append(us_tr(
                        "Warning: Error setting up SAXS structure factor files.\n"
                        "Bead model SAXS disabled.\n"
                        "Check to make sure the files in SOMO->SAXS/SANS Options->Miscellaneous are correct.\n"
                        ));
      editor->setTextColor(save_color);
      saxs_options.compute_saxs_coeff_for_bead_models = false;
   } else {
      saxs_util->setup_saxs_options();
      saxs_util->our_saxs_options = saxs_options;
   }

   if ( 
       !saxs_util->load_mw_json( USglobal->config_list.system_dir + 
                                 QDir::separator() + "etc" +
                                 QDir::separator() + "mw.json" ) )
   {
      editor_msg( "red", us_tr( "Warning: mw.json not read" ) );
   }

   if ( 
       !saxs_util->load_vdw_json( USglobal->config_list.system_dir + 
                                  QDir::separator() + "etc" +
                                  QDir::separator() + "vdw.json" ) )
   {
      editor_msg( "red", us_tr( "Warning: vdw.json not read" ) );
   }

   if ( 
       !saxs_util->load_vcm_json( USglobal->config_list.system_dir + 
                                  QDir::separator() + "etc" +
                                  QDir::separator() + "vcm.json" ) )
   {
      editor_msg( "red", us_tr( "Warning: vcm.json not read" ) );
   }

   // now loaded with residue file
   // if ( 
   //     load_vdwf_json( USglobal->config_list.system_dir + 
   //                     QDir::separator() + "etc" +
   //                     QDir::separator() + "vdwf.json" ) )
   // {
   //    editor_msg( "red", us_tr( "Warning: vdwf.json not read" ) );
   // }

   if ( saxs_options.wavelength == 0 )
   {
      saxs_options.start_q = 
         saxs_options.end_q = 
         saxs_options.delta_q = 0;
   }
   else
   {
      saxs_options.start_q = 4.0 * M_PI * 
         sin(saxs_options.start_angle * M_PI / 360.0) / 
         saxs_options.wavelength;
      saxs_options.start_q =  floor(saxs_options.start_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
      saxs_options.end_q = 4.0 * M_PI * 
         sin(saxs_options.end_angle * M_PI / 360.0) / 
         saxs_options.wavelength;
      saxs_options.end_q =  floor(saxs_options.end_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
      saxs_options.delta_q = 4.0 * M_PI * 
         sin(saxs_options.delta_angle * M_PI / 360.0) / 
         saxs_options.wavelength;
      saxs_options.delta_q =  floor(saxs_options.delta_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
   }         
   add_to_directory_history( somo_pdb_dir, false );
   add_to_directory_history( somo_saxs_dir, false );

#if defined( HB_TEST )
   // hb test

   {
      vector < double >  P;
      P.push_back( .0007 );
      P.push_back( .0003 );
      P.push_back( .00001 );
      P.push_back( .006 );
      P.push_back( .006 );
      P.push_back( .00002 );

      us_qdebug( QString( "holm bonferroni returns %1" ).arg( US_Saxs_Util::holm_bonferroni( P, 0.01 ) ) );
   }
#endif
}

US_Hydrodyn::~US_Hydrodyn()
{
}


void US_Hydrodyn::setupGUI()
{
   int minHeight1 = 24;
   bead_model_file = "";

#if QT_VERSION < 0x040000
 //   lookup_tables = new Q3PopupMenu;
   lookup_tables->insertItem(us_tr("Add/Edit &Hybridization"), this, SLOT(hybrid()));
   lookup_tables->insertItem(us_tr("Add/Edit &Atom"), this, SLOT(edit_atom()));
   lookup_tables->insertItem(us_tr("Add/Edit &Residue"), this, SLOT(residue()));
   lookup_tables->insertItem(us_tr("Add/Edit &SAXS coefficients"), this, SLOT(do_saxs()));

 //   somo_options = new Q3PopupMenu;
   somo_options->insertItem(us_tr("&ASA Calculation"), this, SLOT(show_asa()));
   somo_options->insertItem(us_tr("&SoMo Overlap Reduction"), this, SLOT(show_overlap()));
   somo_options->insertItem(us_tr("AtoB (Grid) &Overlap Reduction"), this, SLOT(show_grid_overlap()));
   somo_options->insertItem(us_tr("&Hydrodynamic Calculations"), this, SLOT(show_hydro()));
   somo_options->insertItem(us_tr("Hydrodynamic Calculations &Zeno"), this, SLOT(show_zeno_options()));
   somo_options->insertItem(us_tr("&Miscellaneous Options"), this, SLOT(show_misc()));
   somo_options->insertItem(us_tr("&Bead Model Output"), this, SLOT(show_bead_output()));
   somo_options->insertItem(us_tr("&Grid Functions (AtoB)"), this, SLOT(show_grid()));
   somo_options->insertItem(us_tr("SA&XS/SANS Options"), this, SLOT(show_saxs_options()));

 //   md_options = new Q3PopupMenu;
   // md_options->insertItem(us_tr("&DMD Options"), this, SLOT(show_dmd_options()));
   md_options->insertItem(us_tr("&Browflex Options"), this, SLOT(show_bd_options()));
   md_options->insertItem(us_tr("&Anaflex Options"), this, SLOT(show_anaflex_options()));

 //   pdb_options = new Q3PopupMenu;
   pdb_options->insertItem(us_tr("&Parsing"), this, SLOT(pdb_parsing()));
   pdb_options->insertItem(us_tr("&Visualization"), this, SLOT(pdb_visualization()));

 //   configuration = new Q3PopupMenu;
   configuration->insertItem(us_tr("&Load Configuration"), this, SLOT(load_config()));
   configuration->insertItem(us_tr("&Save Current Configuration"), this, SLOT(write_config()));
   configuration->insertItem(us_tr("&Reset to Default Configuration"), this, SLOT(reset()));
   configuration->insertItem(us_tr("&Advanced Configuration"), this, SLOT(show_advanced_config()));
   configuration->insertItem(us_tr("S&ystem Configuration"), this, SLOT(run_us_config()));
   // configuration->insertItem(us_tr("A&dministrator"), this, SLOT(run_us_admin()));

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);

# if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   menu = new QMenuBar(frame);
# else
   menu = new QMenuBar( this );
# endif
   menu->setPalette( PALET_NORMAL );
   AUTFBACK( menu );
   menu->insertItem(us_tr("&Lookup Tables"), lookup_tables);
   menu->insertItem(us_tr("&SOMO"), somo_options);
   menu->insertItem(us_tr("&MD"), md_options);
   menu->insertItem(us_tr("&PDB"), pdb_options);
   menu->insertItem(us_tr("&Configuration"), configuration);

# if QT_VERSION >= 0x040000 && defined(Q_OS_MAC)
   {
 //      Q3PopupMenu * file = new Q3PopupMenu;
      file->insertItem( us_tr("&Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
      file->insertItem( us_tr("&Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
#  ifndef NO_EDITOR_PRINT
      file->insertItem( us_tr("&Print"), this, SLOT(print()),   Qt::ALT+Qt::Key_P );
#  endif
      file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
      menu->insertItem(us_tr("&Messages"), file );
   }
# endif
#else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);
   frame->setPalette( PALET_NORMAL );
   AUTFBACK( frame );

# if !defined(Q_OS_MAC) || QT_VERSION > 0x050000
   menu = new QMenuBar(frame);
# else
   menu = new QMenuBar( this );
# endif
   {
      QMenu *submenu = new QMenu( us_tr("&Lookup Tables") );
      {
         QAction *qa = submenu->addAction( us_tr("Add/Edit &Hybridization") );
         connect( qa, SIGNAL(triggered()), this, SLOT( hybrid() ) );
      }
      {
         QAction *qa = submenu->addAction( us_tr("Add/Edit &Atom") );
         connect( qa, SIGNAL(triggered()),  this, SLOT(edit_atom()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("Add/Edit &Residue") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(residue()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("Add/Edit &SAXS coefficients") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(do_saxs()));
      }
      menu->addMenu( submenu );
   }
   {
      QMenu *submenu = new QMenu( us_tr("&SOMO") );
      {
         QAction *qa = submenu->addAction( us_tr("&ASA Calculation") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_asa()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&SoMo Overlap Reduction") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_overlap()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("AtoB (Grid) &Overlap Reduction") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_grid_overlap()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&Hydrodynamic Calculations") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_hydro()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("Hydrodynamic Calculations &Zeno") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_zeno_options()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&Miscellaneous Options") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_misc()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&Bead Model Output") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_bead_output()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&Grid Functions (AtoB)") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_grid()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("SA&XS/SANS Options") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_saxs_options()));
      }
      menu->addMenu( submenu );
   }
   {
      QMenu *submenu = new QMenu( us_tr("&MD") );
      // {
      //    QAction *qa = submenu->addAction( us_tr("&DMD Options") );
      //    connect( qa, SIGNAL( triggered() ), this, SLOT(show_dmd_options()));
      // }
      {
         QAction *qa = submenu->addAction( us_tr("&Browflex Options") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_bd_options()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&Anaflex Options") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_anaflex_options()));
      }
      menu->addMenu( submenu );
   }
   {
      QMenu *submenu = new QMenu( us_tr("&PDB") );
      {
         QAction *qa = submenu->addAction( us_tr("&Parsing") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(pdb_parsing()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&Visualization") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(pdb_visualization()));
      }
      menu->addMenu( submenu );
   }
   {
      QMenu *submenu = new QMenu( us_tr("&Configuration") );
      {
         QAction *qa = submenu->addAction( us_tr( "&Load Configuration" ) );
         connect( qa, SIGNAL( triggered() ), this, SLOT(load_config()));
      }
      {
         QAction *qa = submenu->addAction( us_tr( "&Save Current Configuration") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(write_config()));
      }
      {
         QAction *qa = submenu->addAction( us_tr( "&Reset to Default Configuration") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(reset()));
      }
      {
         QAction *qa = submenu->addAction( us_tr( "&Advanced Configuration") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_advanced_config()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("S&ystem Configuration") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(run_us_config()));
      }
      // {
      //    QAction *qa = submenu->addAction( us_tr("A&dministrator") );
      //    connect( qa, SIGNAL( triggered() ), this, SLOT(run_us_admin()));
      // }
      menu->addMenu( submenu );
   }
# if defined(Q_OS_MAC)
   {
#  if QT_VERSION >= 0x050000
      QMenu * submenu = new QMenu( us_tr( "&File" ) );
#  else
      QMenu * submenu = m->addMenu( us_tr( "&File" ) );
#  endif

      QAction *qa1 = submenu->addAction( us_tr( "Font" ) );
      qa1->setShortcut( Qt::ALT+Qt::Key_F );
      connect( qa1, SIGNAL(triggered()), this, SLOT( update_font() ) );

      QAction *qa2 = submenu->addAction( us_tr( "Save" ) );
      qa2->setShortcut( Qt::ALT+Qt::Key_S );
      connect( qa2, SIGNAL(triggered()), this, SLOT( save() ) );

      QAction *qa3 = submenu->addAction( us_tr( "Clear Display" ) );
      qa3->setShortcut( Qt::ALT+Qt::Key_X );
      connect( qa3, SIGNAL(triggered()), this, SLOT( clear_display() ) );

      menu->addMenu( submenu );
   }
# endif
#endif

   lbl_info1 = new QLabel(us_tr("PDB Functions:"), this);
   Q_CHECK_PTR(lbl_info1);
   lbl_info1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info1->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info1->setMinimumHeight(minHeight1);
   lbl_info1->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info1 );
   lbl_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_info2 = new QLabel(us_tr("Bead Model Functions:"), this);
   Q_CHECK_PTR(lbl_info2);
   lbl_info2->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info2->setMinimumHeight(minHeight1);
   lbl_info2->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info2 );
   lbl_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_info3 = new QLabel(us_tr("Hydrodynamic Calculations:"), this);
   Q_CHECK_PTR(lbl_info3);
   lbl_info3->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info3->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info3->setMinimumHeight(minHeight1);
   lbl_info3->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info3 );
   lbl_info3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   pb_select_residue_file = new QPushButton(us_tr("Select Lookup Table"), this);
   Q_CHECK_PTR(pb_select_residue_file);
   pb_select_residue_file->setMinimumHeight(minHeight1);
   pb_select_residue_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_select_residue_file->setPalette( PALET_PUSHB );
   connect(pb_select_residue_file, SIGNAL(clicked()), SLOT(select_residue_file()));

   lbl_table = new QLabel( QDir::toNativeSeparators( residue_filename ), this );
   lbl_table->setMinimumHeight(minHeight1);
   lbl_table->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_table->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_table->setPalette( PALET_EDIT );
   AUTFBACK( lbl_table );
   lbl_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   pb_batch = new QPushButton(us_tr("Batch Mode/Cluster Operation"), this);
   Q_CHECK_PTR(pb_batch);
   pb_batch->setMinimumHeight(minHeight1);
   pb_batch->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_batch->setPalette( PALET_PUSHB );
   pb_batch->setAutoDefault( false );
   connect(pb_batch, SIGNAL(clicked()), SLOT(show_batch()));

   pb_view_bead_model = new QPushButton(us_tr("View Bead Model File"), this);
   Q_CHECK_PTR(pb_view_bead_model);
   pb_view_bead_model->setMinimumHeight(minHeight1);
   pb_view_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_view_bead_model->setPalette( PALET_PUSHB );
   connect(pb_view_bead_model, SIGNAL(clicked()), SLOT(view_bead_model()));

   pb_load_pdb = new QPushButton(us_tr("Load Single PDB File"), this);
   Q_CHECK_PTR(pb_load_pdb);
   pb_load_pdb->setMinimumHeight(minHeight1);
   pb_load_pdb->setEnabled(true);
   pb_load_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_pdb->setPalette( PALET_PUSHB );
   connect(pb_load_pdb, SIGNAL(clicked()), SLOT(load_pdb()));

   le_pdb_file = new mQLineEdit( this );
   le_pdb_file->setText( us_tr( "not selected" ) );
#if QT_VERSION < 0x040000
   le_pdb_file->setFrameStyle(QFrame::WinPanel|Sunken);
#endif
   le_pdb_file->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pdb_file->setMinimumHeight(minHeight1);
   le_pdb_file->setPalette( PALET_EDIT );
   AUTFBACK( le_pdb_file );
   le_pdb_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect( le_pdb_file, SIGNAL( focussed( bool ) ), SLOT( le_pdb_file_focus( bool ) ) );
   connect( le_pdb_file, SIGNAL( textChanged( const QString & ) ), SLOT( le_pdb_file_changed( const QString & ) ) );

   lbl_model = new QLabel(us_tr(" Please select a PDB Structure:"), this);
   Q_CHECK_PTR(lbl_model);
   lbl_model->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_model->setMinimumHeight(minHeight1);
   lbl_model->setMargin( 2 );
   lbl_model->setPalette( PALET_LABEL );
   AUTFBACK( lbl_model );
   lbl_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   pb_view_pdb = new QPushButton(us_tr("View/Edit PDB File"), this);
   Q_CHECK_PTR(pb_view_pdb);
   pb_view_pdb->setMinimumHeight(minHeight1);
   pb_view_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_view_pdb->setPalette( PALET_PUSHB );
   connect(pb_view_pdb, SIGNAL(clicked()), SLOT(view_pdb()));

   pb_pdb_tool = new QPushButton(us_tr("PDB Editor"), this);
   pb_pdb_tool->setMinimumHeight(minHeight1);
   pb_pdb_tool->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pdb_tool->setPalette( PALET_PUSHB );
   connect(pb_pdb_tool, SIGNAL(clicked()), SLOT(pdb_tool()));

   lb_model = new QListWidget( this );
   lb_model->setPalette( PALET_LISTB );
   lb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //lb_model->setEnabled(false);
   lb_model->setEnabled( true );
   lb_model->setSelectionMode(QAbstractItemView::ExtendedSelection);
 //   lb_model->setHScrollBarMode(QScrollView::Auto);
 //   lb_model->setVScrollBarMode(QScrollView::Auto);
   // connect(lb_model, SIGNAL(currentRowChanged(int)), this, SLOT(select_model(int)));
   connect(lb_model, SIGNAL(itemSelectionChanged()), this, SLOT(model_selection_changed()));

   pb_load_bead_model = new QPushButton(us_tr("Load Single Bead Model File"), this);
   Q_CHECK_PTR(pb_load_bead_model);
   pb_load_bead_model->setMinimumHeight(minHeight1);
   pb_load_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_bead_model->setPalette( PALET_PUSHB );
   connect(pb_load_bead_model, SIGNAL(clicked()), SLOT(load_bead_model()));

   le_bead_model_file = new QLineEdit( this );    le_bead_model_file->setObjectName( "bead_model_file Line Edit" );
   le_bead_model_file->setText(us_tr(" not selected "));
   le_bead_model_file->setMinimumHeight(minHeight1);
   le_bead_model_file->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_bead_model_file->setPalette( PALET_EDIT );
   AUTFBACK( le_bead_model_file );
   le_bead_model_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_bead_model_file, SIGNAL(textChanged(const QString &)), SLOT(update_bead_model_file(const QString &)));

   lbl_bead_model_prefix = new QLabel(us_tr(" Bead Model Suffix:"), this);
   Q_CHECK_PTR(lbl_bead_model_prefix);
   lbl_bead_model_prefix->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bead_model_prefix->setMinimumHeight(minHeight1);
   lbl_bead_model_prefix->setPalette( PALET_LABEL );
   AUTFBACK( lbl_bead_model_prefix );
   lbl_bead_model_prefix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cb_setSuffix = new QCheckBox(this);
   cb_setSuffix->setText(us_tr(" Add auto-generated suffix "));
   cb_setSuffix->setChecked(setSuffix);
   cb_setSuffix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_setSuffix->setPalette( PALET_NORMAL );
   AUTFBACK( cb_setSuffix );
   connect(cb_setSuffix, SIGNAL(clicked()), this, SLOT(set_setSuffix()));

   cb_overwrite = new QCheckBox(this);
   cb_overwrite->setText(us_tr(" Overwrite existing filenames "));
   cb_overwrite->setChecked(overwrite);
   cb_overwrite->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_overwrite->setPalette( PALET_NORMAL );
   AUTFBACK( cb_overwrite );
   connect(cb_overwrite, SIGNAL(clicked()), this, SLOT(set_overwrite()));

   le_bead_model_prefix = new QLineEdit( this );    le_bead_model_prefix->setObjectName( "bead_model_prefix Line Edit" );
   le_bead_model_prefix->setText(us_tr(""));
   le_bead_model_prefix->setMinimumHeight(minHeight1);
   le_bead_model_prefix->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_bead_model_prefix->setPalette( PALET_EDIT );
   AUTFBACK( le_bead_model_prefix );
   le_bead_model_prefix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_bead_model_prefix, SIGNAL(textChanged(const QString &)), SLOT(update_bead_model_prefix(const QString &)));

   le_bead_model_suffix = new QLineEdit( this );    le_bead_model_suffix->setObjectName( "bead_model_suffix Line Edit" );
   le_bead_model_suffix->setText(us_tr(""));
   le_bead_model_suffix->setMinimumHeight(minHeight1);
   le_bead_model_suffix->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_bead_model_suffix->setPalette( PALET_EDIT );
   AUTFBACK( le_bead_model_suffix );
   le_bead_model_suffix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_bead_model_suffix->setReadOnly(true);

   pb_somo = new QPushButton(us_tr("Build SoMo Bead Model"), this);
   Q_CHECK_PTR(pb_somo);
   pb_somo->setMinimumHeight(minHeight1);
   pb_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_somo->setEnabled(false);
   pb_somo->setPalette( PALET_PUSHB );
   connect(pb_somo, SIGNAL(clicked()), SLOT(calc_somo()));

   pb_somo_o = new QPushButton(us_tr("Build SoMo Overlap Bead Model"), this);
   Q_CHECK_PTR(pb_somo_o);
   pb_somo_o->setMinimumHeight(minHeight1);
   pb_somo_o->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_somo_o->setEnabled(false);
   pb_somo_o->setPalette( PALET_PUSHB );
   connect(pb_somo_o, SIGNAL(clicked()), SLOT(calc_somo_o()));

#if defined(USE_H)
   pb_pdb_hydrate_for_saxs = new QPushButton(us_tr("Hydrate"), this);
   pb_pdb_hydrate_for_saxs->setMinimumHeight(minHeight1);
   pb_pdb_hydrate_for_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pdb_hydrate_for_saxs->setEnabled(true);
   pb_pdb_hydrate_for_saxs->setPalette( PALET_PUSHB );
   connect(pb_pdb_hydrate_for_saxs, SIGNAL(clicked()), SLOT(pdb_hydrate_for_saxs()));
#endif

   pb_pdb_saxs = new QPushButton(us_tr("SAXS/SANS Functions"), this);
   Q_CHECK_PTR(pb_pdb_saxs);
   pb_pdb_saxs->setMinimumHeight(minHeight1);
   pb_pdb_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pdb_saxs->setEnabled(true);
   pb_pdb_saxs->setPalette( PALET_PUSHB );
   connect(pb_pdb_saxs, SIGNAL(clicked()), SLOT(pdb_saxs()));

   pb_bead_saxs = new QPushButton(us_tr("SAXS/SANS Functions"), this);
   Q_CHECK_PTR(pb_bead_saxs);
   pb_bead_saxs->setMinimumHeight(minHeight1);
   pb_bead_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_bead_saxs->setEnabled(false);
   pb_bead_saxs->setPalette( PALET_PUSHB );
   connect(pb_bead_saxs, SIGNAL(clicked()), SLOT(bead_saxs()));

   pb_rescale_bead_model = new QPushButton(us_tr("Rescale/Equalize Bead Model"), this);
   pb_rescale_bead_model->setMinimumHeight(minHeight1);
   pb_rescale_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_rescale_bead_model->setEnabled(false);
   pb_rescale_bead_model->setPalette( PALET_PUSHB );
   connect(pb_rescale_bead_model, SIGNAL(clicked()), SLOT(rescale_bead_model()));

   pb_equi_grid_bead_model = new QPushButton(us_tr("Simple grid"), this);
   pb_equi_grid_bead_model->setMinimumHeight(minHeight1);
   pb_equi_grid_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_equi_grid_bead_model->setEnabled(false);
   pb_equi_grid_bead_model->setPalette( PALET_PUSHB );
   connect(pb_equi_grid_bead_model, SIGNAL(clicked()), SLOT(equi_grid_bead_model()));

   pb_grid_pdb = new QPushButton(us_tr("Build AtoB (Grid) Bead Model"), this);
   Q_CHECK_PTR(pb_grid_pdb);
   pb_grid_pdb->setMinimumHeight(minHeight1);
   pb_grid_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_grid_pdb->setEnabled(false);
   pb_grid_pdb->setPalette( PALET_PUSHB );
   connect(pb_grid_pdb, SIGNAL(clicked()), SLOT(calc_grid_pdb()));

   // pb_grid_pdb_o = new QPushButton(us_tr("Build AtoB (Grid) Overlap Bead Model"), this);
   // pb_grid_pdb_o->setMinimumHeight(minHeight1);
   // pb_grid_pdb_o->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   // pb_grid_pdb_o->setEnabled(false);
   // pb_grid_pdb_o->setPalette( PALET_PUSHB );
   // connect(pb_grid_pdb_o, SIGNAL(clicked()), SLOT(calc_grid_pdb_o()));

   pb_vdw_beads = new QPushButton(us_tr("Build vdW Overlap Bead Model"), this);
   pb_vdw_beads->setMinimumHeight(minHeight1);
   pb_vdw_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_vdw_beads->setEnabled(false);
   pb_vdw_beads->setPalette( PALET_PUSHB );
   connect(pb_vdw_beads, SIGNAL(clicked()), SLOT(calc_vdw_beads()));

   pb_grid = new QPushButton(us_tr("Grid Existing Bead Model"), this);
   Q_CHECK_PTR(pb_grid);
   pb_grid->setMinimumHeight(minHeight1);
   pb_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_grid->setEnabled(false);
   pb_grid->setPalette( PALET_PUSHB );
   connect(pb_grid, SIGNAL(clicked()), SLOT(calc_grid()));

   cb_calcAutoHydro = new QCheckBox(this);
   cb_calcAutoHydro->setText(us_tr(" Automatically Calculate Hydrodynamics "));
   cb_calcAutoHydro->setChecked(calcAutoHydro);
   cb_calcAutoHydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_calcAutoHydro->setPalette( PALET_NORMAL );
   AUTFBACK( cb_calcAutoHydro );
   connect(cb_calcAutoHydro, SIGNAL(clicked()), this, SLOT(set_calcAutoHydro()));

   pb_view_asa = new QPushButton(us_tr("View ASA Results"), this);
   Q_CHECK_PTR(pb_view_asa);
   pb_view_asa->setMinimumHeight(minHeight1);
   pb_view_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_view_asa->setPalette( PALET_PUSHB );
   connect(pb_view_asa, SIGNAL(clicked()), SLOT(view_asa()));

   pb_visualize = new QPushButton(us_tr("Visualize Bead Model"), this);
   Q_CHECK_PTR(pb_visualize);
   pb_visualize->setMinimumHeight(minHeight1);
   pb_visualize->setEnabled(false);
   pb_visualize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_visualize->setPalette( PALET_PUSHB );
   connect(pb_visualize, SIGNAL(clicked()), SLOT(visualize()));

   pb_batch2 = new QPushButton(us_tr("Batch Mode/Cluster Operation"), this);
   Q_CHECK_PTR(pb_batch2);
   pb_batch2->setMinimumHeight(minHeight1);
   pb_batch2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_batch2->setPalette( PALET_PUSHB );
   connect(pb_batch2, SIGNAL(clicked()), SLOT(show_batch()));

   pb_calc_hydro = new QPushButton(us_tr("Calculate RB Hydrodynamics SMI"), this);
   Q_CHECK_PTR(pb_calc_hydro);
   pb_calc_hydro->setEnabled(false);
   pb_calc_hydro->setMinimumHeight(minHeight1);
   pb_calc_hydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_calc_hydro->setPalette( PALET_PUSHB );
   connect(pb_calc_hydro, SIGNAL(clicked()), SLOT(calc_hydro()));

   pb_calc_zeno = new QPushButton(us_tr("Calculate RB Hydrodynamics ZENO"), this);
   Q_CHECK_PTR(pb_calc_zeno);
   pb_calc_zeno->setEnabled(false);
   pb_calc_zeno->setMinimumHeight(minHeight1);
   pb_calc_zeno->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_calc_zeno->setPalette( PALET_PUSHB );
   connect(pb_calc_zeno, SIGNAL(clicked()), SLOT(calc_zeno_hydro()));

   pb_calc_hullrad = new QPushButton(us_tr("Hullrad"), this);
   Q_CHECK_PTR(pb_calc_hullrad);
   pb_calc_hullrad->setEnabled(true);
   pb_calc_hullrad->setMinimumHeight(minHeight1);
   pb_calc_hullrad->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_calc_hullrad->setPalette( PALET_PUSHB );
   connect(pb_calc_hullrad, SIGNAL(clicked()), SLOT(calc_hullrad_hydro()));
   pb_calc_hullrad->hide();

   pb_show_hydro_results = new QPushButton(us_tr("Show Hydrodynamic Calculations"), this);
   Q_CHECK_PTR(pb_show_hydro_results);
   pb_show_hydro_results->setMinimumHeight(minHeight1);
   pb_show_hydro_results->setEnabled(false);
   pb_show_hydro_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_show_hydro_results->setPalette( PALET_PUSHB );
   connect(pb_show_hydro_results, SIGNAL(clicked()), SLOT(show_hydro_results()));

   pb_comparative = new QPushButton(us_tr("Model classifier"), this);
   pb_comparative->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_comparative->setEnabled(true);
   pb_comparative->setPalette( PALET_PUSHB );
   connect(pb_comparative, SIGNAL(clicked()), SLOT(select_comparative()));

   pb_best = new QPushButton(us_tr("BEST"), this);
   pb_best->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_best->setEnabled(true);
   pb_best->setPalette( PALET_PUSHB );
   connect(pb_best, SIGNAL(clicked()), SLOT( best_analysis()));
   // pb_best->hide();

   pb_open_hydro_results = new QPushButton(us_tr("Open Hydrodynamic Calculations File"), this);
   Q_CHECK_PTR(pb_open_hydro_results);
   pb_open_hydro_results->setMinimumHeight(minHeight1);
   pb_open_hydro_results->setEnabled(true);
   pb_open_hydro_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_open_hydro_results->setPalette( PALET_PUSHB );
   connect(pb_open_hydro_results, SIGNAL(clicked()), SLOT(open_hydro_results()));

   pb_select_save_params = new QPushButton(us_tr("Select Parameters to be Saved"), this);
   Q_CHECK_PTR(pb_select_save_params);
   pb_select_save_params->setMinimumHeight(minHeight1);
   pb_select_save_params->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_select_save_params->setEnabled(true);
   pb_select_save_params->setPalette( PALET_PUSHB );
   connect(pb_select_save_params, SIGNAL(clicked()), SLOT(select_save_params()));

   cb_saveParams = new QCheckBox(this);
   cb_saveParams->setText(us_tr(" Save parameters to file "));
   cb_saveParams->setChecked(saveParams);
   cb_saveParams->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saveParams->setPalette( PALET_NORMAL );
   AUTFBACK( cb_saveParams );
   connect(cb_saveParams, SIGNAL(clicked()), this, SLOT(set_saveParams()));

   // ***** dmd *******
   pb_dmd_run = new QPushButton(us_tr("Run DMD"), this);
   pb_dmd_run->setMinimumHeight(minHeight1);
   pb_dmd_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_dmd_run->setEnabled( true );
   pb_dmd_run->setPalette( PALET_PUSHB );
   connect(pb_dmd_run, SIGNAL(clicked()), SLOT(dmd_run()));

   pb_bd = new QPushButton(us_tr("BD"), this);
   Q_CHECK_PTR(pb_bd);
   pb_bd->setMinimumHeight(minHeight1);
   pb_bd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
#ifdef NO_BD
   pb_bd->setEnabled(false);
#else
   pb_bd->setEnabled(true);
#endif
   pb_bd->setPalette( PALET_PUSHB );
   connect(pb_bd, SIGNAL(clicked()), SLOT(show_bd()));

   // ***** bd *******
   //   pb_bd_prepare = new QPushButton(us_tr("Create Browflex files"), this);
   //   pb_bd_prepare->setMinimumHeight(minHeight1);
   //   pb_bd_prepare->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_prepare->setEnabled(false);
   //   pb_bd_prepare->setPalette( PALET_PUSHB );
   //   connect(pb_bd_prepare, SIGNAL(clicked()), SLOT(bd_prepare()));

   //   pb_bd_load = new QPushButton(us_tr("Load Browflex files"), this);
   //   pb_bd_load->setMinimumHeight(minHeight1);
   //   pb_bd_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_load->setEnabled(false);
   //   pb_bd_load->setPalette( PALET_PUSHB );
   //   connect(pb_bd_load, SIGNAL(clicked()), SLOT(bd_load()));

   //   pb_bd_edit = new QPushButton(us_tr("View/Edit Browflex files"), this);
   //   pb_bd_edit->setMinimumHeight(minHeight1);
   //   pb_bd_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_edit->setEnabled(false);
   //   pb_bd_edit->setPalette( PALET_PUSHB );
   //   connect(pb_bd_edit, SIGNAL(clicked()), SLOT(bd_edit()));

   //   pb_bd_run = new QPushButton(us_tr("Run Browflex"), this);
   //   pb_bd_run->setMinimumHeight(minHeight1);
   //   pb_bd_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_run->setEnabled(false);
   //   pb_bd_run->setPalette( PALET_PUSHB );
   //   connect(pb_bd_run, SIGNAL(clicked()), SLOT(bd_run()));

   //   pb_bd_load_results = new QPushButton(us_tr("Load/Process Browflex results"), this);
   //   pb_bd_load_results->setMinimumHeight(minHeight1);
   //   pb_bd_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_load_results->setEnabled(false);
   //   pb_bd_load_results->setPalette( PALET_PUSHB );
   //   connect(pb_bd_load_results, SIGNAL(clicked()), SLOT(bd_load_results()));

   // ***** anaflex *******
   //   pb_anaflex_prepare = new QPushButton(us_tr("Create Anaflex files"), this);
   //   pb_anaflex_prepare->setMinimumHeight(minHeight1);
   //   pb_anaflex_prepare->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_prepare->setEnabled(false);
   //   pb_anaflex_prepare->setPalette( PALET_PUSHB );
   //   connect(pb_anaflex_prepare, SIGNAL(clicked()), SLOT(anaflex_prepare()));

   //   pb_anaflex_load = new QPushButton(us_tr("Load Anaflex files"), this);
   //   pb_anaflex_load->setMinimumHeight(minHeight1);
   //   pb_anaflex_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_load->setEnabled(false);
   //   pb_anaflex_load->setPalette( PALET_PUSHB );
   //   connect(pb_anaflex_load, SIGNAL(clicked()), SLOT(anaflex_load()));

   //   pb_anaflex_edit = new QPushButton(us_tr("View/Edit Anaflex files"), this);
   //   pb_anaflex_edit->setMinimumHeight(minHeight1);
   //   pb_anaflex_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_edit->setEnabled(false);
   //   pb_anaflex_edit->setPalette( PALET_PUSHB );
   //   connect(pb_anaflex_edit, SIGNAL(clicked()), SLOT(anaflex_edit()));

   //   pb_anaflex_run = new QPushButton(us_tr("Run Anaflex"), this);
   //   pb_anaflex_run->setMinimumHeight(minHeight1);
   //   pb_anaflex_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_run->setEnabled(false);
   //   pb_anaflex_run->setPalette( PALET_PUSHB );
   //   connect(pb_anaflex_run, SIGNAL(clicked()), SLOT(anaflex_run()));

   //   pb_anaflex_load_results = new QPushButton(us_tr("Load Anaflex results"), this);
   //   pb_anaflex_load_results->setMinimumHeight(minHeight1);
   //   pb_anaflex_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_load_results->setEnabled(false);
   //   pb_anaflex_load_results->setPalette( PALET_PUSHB );
   //   connect(pb_anaflex_load_results, SIGNAL(clicked()), SLOT(anaflex_load_results()));

   pb_help = new QPushButton(us_tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));


   pb_config = new QPushButton(us_tr("Config"), this);
   pb_config->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_config->setMinimumHeight(minHeight1);
   pb_config->setPalette( PALET_PUSHB );
   connect(pb_config, SIGNAL(clicked()), SLOT(config()));

   pb_stop_calc = new QPushButton(us_tr("Stop"), this);
   Q_CHECK_PTR(pb_stop_calc);
   pb_stop_calc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_stop_calc->setMinimumHeight(minHeight1);
   pb_stop_calc->setPalette( PALET_PUSHB );
   connect(pb_stop_calc, SIGNAL(clicked()), SLOT(stop_calc()));
   pb_stop_calc->setEnabled(false);

   pb_cancel = new QPushButton(us_tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   progress = new QProgressBar( this );
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->reset();

   editor = new QTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   editor->setReadOnly(true);
   editor->setMinimumWidth(550);

#if QT_VERSION < 0x040000
# if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   m = new QMenuBar( editor );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
 //   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( us_tr("&File"), file );
   file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( us_tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );

#  ifndef NO_EDITOR_PRINT
   file->insertItem( us_tr("Print"), this, SLOT(print()),   Qt::ALT+Qt::Key_P );
#  endif
   file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
# endif
#else
# if !defined(Q_OS_MAC)
   m = new QMenuBar( editor );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1);
   {
      QMenu * new_menu = m->addMenu( us_tr( "&File" ) );

      QAction *qa1 = new_menu->addAction( us_tr( "Font" ) );
      qa1->setShortcut( Qt::ALT+Qt::Key_F );
      connect( qa1, SIGNAL(triggered()), this, SLOT( update_font() ) );

      QAction *qa2 = new_menu->addAction( us_tr( "Save" ) );
      qa2->setShortcut( Qt::ALT+Qt::Key_S );
      connect( qa2, SIGNAL(triggered()), this, SLOT( save() ) );

      QAction *qa3 = new_menu->addAction( us_tr( "Clear Display" ) );
      qa3->setShortcut( Qt::ALT+Qt::Key_X );
      connect( qa3, SIGNAL(triggered()), this, SLOT( clear_display() ) );
   }
# endif
#endif
   
   editor->setWordWrapMode (advanced_config.scroll_editor ? QTextOption::NoWrap : QTextOption::WordWrap);

   lbl_core_progress = new QLabel("", this);
   Q_CHECK_PTR(lbl_core_progress);
   lbl_core_progress->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_core_progress->setMinimumHeight(minHeight1);
   lbl_core_progress->setPalette( PALET_LABEL );
   AUTFBACK( lbl_core_progress );
   lbl_core_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   clear_display();

   int rows=20, columns = 3, spacing = 2, j=0, margin=4;
   QGridLayout * background = new QGridLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 ); background->setSpacing( spacing ); background->setContentsMargins( margin, margin, margin, margin );

   background->addWidget( frame , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   background->addWidget( editor , j , 2 , 1 + ( j+24 ) - ( j ) , 1 + ( 2 ) - ( 2 ) );
   j++;
   background->addWidget( lbl_info1 , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   background->addWidget(pb_select_residue_file, j, 0);
   background->addWidget(lbl_table, j, 1);
   j++;
   background->addWidget(pb_batch, j, 0);
   j++;
   background->addWidget(pb_load_pdb, j, 0);
   background->addWidget(le_pdb_file, j, 1);
   j++;
   background->addWidget(lbl_model, j, 0);
   background->addWidget( lb_model , j , 1 , 1 + ( j+4 ) - ( j ) , 1 + ( 1 ) - ( 1 ) );
   j++;
   QHBoxLayout * hbl_pdb = new QHBoxLayout(); hbl_pdb->setContentsMargins( 0, 0, 0, 0 ); hbl_pdb->setSpacing( 0 );
   hbl_pdb->addWidget(pb_view_pdb);
   hbl_pdb->addWidget(pb_pdb_tool);
   background->addLayout(hbl_pdb, j, 0);
   j++;
   QHBoxLayout * hbl_pdb_saxs = new QHBoxLayout; hbl_pdb_saxs->setContentsMargins( 0, 0, 0, 0 ); hbl_pdb_saxs->setSpacing( 0 );
   hbl_pdb_saxs->addWidget(pb_pdb_saxs);
#if defined(USE_H)
   hbl_pdb_saxs->addWidget(pb_pdb_hydrate_for_saxs);
#endif
   background->addLayout(hbl_pdb_saxs, j, 0);
   j++;
   background->addWidget(pb_dmd_run, j, 0);
   j++;
   background->addWidget(pb_bd, j, 0);
   j++;
   background->addWidget( lbl_info2 , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   //   background->addWidget(lbl_bead_model_prefix, j, 0);
   QHBoxLayout * hbl_prefix_suffix = new QHBoxLayout; hbl_prefix_suffix->setContentsMargins( 0, 0, 0, 0 ); hbl_prefix_suffix->setSpacing( 0 );
   hbl_prefix_suffix->addWidget(lbl_bead_model_prefix);
   hbl_prefix_suffix->addWidget(le_bead_model_prefix);
   // hbl_prefix_suffix->addWidget(le_bead_model_suffix);
   background->addLayout(hbl_prefix_suffix, j, 0);
   background->addWidget(le_bead_model_suffix, j, 1);
   // QGridLayout * gl_prefix_suffix = new QGridLayout; gl_prefix_suffix->setContentsMargins( 0, 0, 0, 0 ); gl_prefix_suffix->setSpacing( 0 );
   // gl_prefix_suffix->addWidget( le_bead_model_prefix , 0 , 0 , 1 + ( 0 ) - ( 0 ) , 1 + ( 0 ) - ( 0 ) );
   // gl_prefix_suffix->addWidget( le_bead_model_suffix , 0 , 1 , 1 + ( 0 ) - ( 0 ) , 1 + ( 2 ) - ( 1 ) );
   // background->addLayout(gl_prefix_suffix, j, 1);   
   j++;
   background->addWidget(cb_overwrite, j, 0);
   background->addWidget(cb_setSuffix, j, 1);
   j++;
   background->addWidget(pb_somo, j, 0);
   background->addWidget(pb_grid_pdb, j, 1);
   j++;

   background->addWidget(pb_somo_o, j, 0);
   // background->addWidget(pb_grid_pdb_o, j, 1);
   background->addWidget(pb_vdw_beads, j, 1);
   j++;


   //   background->addWidget(pb_bd_prepare, j, 0);
   //   QHBoxLayout * qhl_bd_1 = new QHBoxLayout; qhl_bd_1->setContentsMargins( 0, 0, 0, 0 ); qhl_bd_1->setSpacing( 0 );
   //   qhl_bd_1->addWidget(pb_bd_load);
   //   qhl_bd_1->addWidget(pb_bd_edit);
   //   background->addLayout(qhl_bd_1, j, 1);
   //   j++;
   //   background->addWidget(pb_bd_run, j, 0);
   //   background->addWidget(pb_bd_load_results, j, 1);
   //   j++;
   background->addWidget(pb_view_asa, j, 0);
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( pb_grid );
      hbl->addWidget( pb_visualize );
      background->addLayout( hbl, j, 1 );
   }
   j++;

   background->addWidget(pb_batch2, j, 0);
   background->addWidget(pb_view_bead_model, j, 1);
   j++;

   background->addWidget(pb_load_bead_model, j, 0);
   background->addWidget(le_bead_model_file, j, 1);
   j++;

   background->addWidget(pb_bead_saxs, j, 0);
   background->addWidget(cb_calcAutoHydro, j, 1);
   j++;

   background->addWidget(pb_rescale_bead_model, j, 0);
   background->addWidget(pb_equi_grid_bead_model, j, 1);
   j++;

   background->addWidget( lbl_info3 , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;

   background->addWidget(pb_calc_hydro, j, 0);
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( pb_calc_zeno);
      hbl->addWidget( pb_calc_hullrad );
      background->addLayout(hbl, j, 1);
   }
   j++;

   background->addWidget(pb_show_hydro_results, j, 0);
   background->addWidget(pb_open_hydro_results, j, 1);
   j++;

   background->addWidget(pb_select_save_params, j, 0);
   background->addWidget(cb_saveParams, j, 1);
   j++;

   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( pb_best );
      hbl->addWidget( pb_comparative );
      background->addLayout( hbl, j, 0 );
   }
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( pb_stop_calc );
      hbl->addWidget( pb_cancel );
      background->addLayout( hbl, j, 1 );
   }
   j++;
   QBoxLayout * bl_help_config = new QHBoxLayout; bl_help_config->setContentsMargins( 0, 0, 0, 0 ); bl_help_config->setSpacing( 0 );
   bl_help_config->addWidget( pb_help );
   bl_help_config->addWidget( pb_config );

   background->addLayout( bl_help_config, j, 0);
   background->addWidget(progress, j, 1);
   background->addWidget(lbl_core_progress, j, 2);

   fixWinButtons( this );

   connect( this, SIGNAL( progress_updated( int, int ) ), SLOT( update_progress( int, int ) ) );
}

void US_Hydrodyn::set_expert( bool expert )
{
   if ( expert )
   {
      //      pb_best->show();
#if QT_VERSION < 0x040000
      lookup_tables->insertItem(us_tr("Make test set"), this, SLOT( make_test_set() ) );
#endif
   }
   expert ? pb_equi_grid_bead_model->show() : pb_equi_grid_bead_model->hide();
   expert ? pb_rescale_bead_model->show() : pb_rescale_bead_model->hide();
}

void US_Hydrodyn::set_disabled()
{
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_grid->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_visualize->setEnabled(false);
   //   pb_pdb_saxs->setEnabled(false);

   bd_anaflex_enables(false);

   pb_bead_saxs->setEnabled(false);
   le_bead_model_file->setText(" not selected ");
   pb_rescale_bead_model->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
}

void US_Hydrodyn::hybrid()
{
   if (hybrid_widget)
   {
      addHybrid->raise();
   }
   else
   {
      addHybrid = new US_AddHybridization(&hybrid_widget, 0);
      fixWinButtons( addHybrid );
      addHybrid->show();
   }
}

void US_Hydrodyn::edit_atom()
{
   if (atom_widget)
   {
      addAtom->raise();
   }
   else
   {
      addAtom = new US_AddAtom(&atom_widget, 0);
      fixWinButtons( addAtom );
      addAtom->show();
   }
}

void US_Hydrodyn::residue()
{
   if (residue_widget)
   {
      addResidue->raise();
   }
   else
   {
      addResidue = new US_AddResidue(&residue_widget, misc.hydrovol, 0);
      fixWinButtons( addResidue );
      addResidue->show();
   }
}

void US_Hydrodyn::do_saxs()
{
   if (saxs_widget)
   {
      addSaxs->raise();
   }
   else
   {
      addSaxs = new US_AddSaxs(&saxs_widget, 0);
      fixWinButtons( addSaxs );
      addSaxs->show();
   }
}

void US_Hydrodyn::show_asa()
{
   if (asa_widget)
   {
      if (asa_window->isVisible())
      {
         asa_window->raise();
      }
      else
      {
         asa_window->show();
      }
      return;
   }
   else
   {
      asa_window = new US_Hydrodyn_ASA(&asa, &asa_widget, this);
      fixWinButtons( asa_window );
      asa_window->show();
   }
}

void US_Hydrodyn::show_overlap()
{
   if (overlap_widget)
   {
      if (overlap_window->isVisible())
      {
         overlap_window->raise();
      }
      else
      {
         overlap_window->show();
      }
      return;
   }
   else
   {
      overlap_window = new US_Hydrodyn_Overlap(&sidechain_overlap,
                                               &mainchain_overlap,
                                               &buried_overlap,
                                               &replicate_o_r_method_somo,
                                               &grid_exposed_overlap,
                                               &grid_buried_overlap,
                                               &grid_overlap,
                                               &replicate_o_r_method_grid,
                                               &overlap_tolerance,
                                               &overlap_widget,
                                               this);
      fixWinButtons( overlap_window );
      overlap_window->show();
   }
}

void US_Hydrodyn::show_grid_overlap()
{
   if (grid_overlap_widget)
   {
      if (grid_overlap_window->isVisible())
      {
         grid_overlap_window->raise();
      }
      else
      {
         grid_overlap_window->show();
      }
      return;
   }
   else
   {
      grid_overlap_window = new US_Hydrodyn_Overlap(&grid_exposed_overlap,
                                                    &grid_buried_overlap,
                                                    &grid_overlap,
                                                    &replicate_o_r_method_grid,
                                                    &overlap_tolerance,
                                                    &grid_overlap_widget,
                                                    this);
      fixWinButtons( grid_overlap_window );
      grid_overlap_window->show();
   }
}

void US_Hydrodyn::show_hydro()
{
   if (hydro_widget)
   {
      if (hydro_window->isVisible())
      {
         hydro_window->raise();
      }
      else
      {
         hydro_window->show();
      }
      return;
   }
   else
   {
      hydro_window = new US_Hydrodyn_Hydro(&hydro, &hydro_widget, this);
      fixWinButtons( hydro_window );
      hydro_window->show();
   }
}

void US_Hydrodyn::show_misc()
{
   if (misc_widget)
   {
      if (misc_window->isVisible())
      {
         misc_window->raise();
      }
      else
      {
         misc_window->show();
      }
      return;
   }
   else
   {
      if ( misc.restore_pb_rule ) {
         // us_qdebug( "show_misc() restoring pb rule" );
         misc.pb_rule_on      = true;
         misc.restore_pb_rule = false;
      }

      misc_window = new US_Hydrodyn_Misc(&misc, &misc_widget, this);
      fixWinButtons( misc_window );
      connect(misc_window, SIGNAL(vbar_changed()), this, SLOT(update_vbar()));
      misc_window->show();
   }
   update_vbar();
}

void US_Hydrodyn::show_saxs_options()
{
   if (saxs_options_widget)
   {
      if (saxs_options_window->isVisible())
      {
         saxs_options_window->raise();
      }
      else
      {
         saxs_options_window->show();
      }
      return;
   }
   else
   {
      saxs_options_window = new US_Hydrodyn_SaxsOptions( &saxs_options, 
                                                         &saxs_options_widget, 
                                                         &sas_options_saxs_widget,
                                                         &sas_options_sans_widget,
                                                         &sas_options_curve_widget,
                                                         &sas_options_bead_model_widget,
                                                         &sas_options_hydration_widget,
                                                         &sas_options_guinier_widget,
                                                         &sas_options_xsr_widget,
                                                         &sas_options_misc_widget,
                                                         &sas_options_experimental_widget,
                                                         this );
      fixWinButtons( saxs_options_window );
      saxs_options_window->show();
   }
}

void US_Hydrodyn::show_dmd_options()
{
   if (dmd_options_widget)
   {
      if (dmd_options_window->isVisible())
      {
         dmd_options_window->raise();
      }
      else
      {
         dmd_options_window->show();
      }
      return;
   }
   else
   {
      dmd_options_window = new US_Hydrodyn_DMD_Options(&dmd_options, &dmd_options_widget, this);
      fixWinButtons( dmd_options_window );
      dmd_options_window->show();
   }
}

void US_Hydrodyn::show_bd()
{
#ifdef NO_BD
   US_Static::us_message(us_tr("Please note:"),
                        us_tr("Function not available in this version." ) );
   return;
#endif

   if (bd_widget)
   {
      if (bd_window->isVisible())
      {
         bd_window->raise();
      }
      else
      {
         bd_window->show();
      }
      return;
   }
   else
   {
      bd_window = new US_Hydrodyn_BD(&bd_widget, &bd_options, &anaflex_options, this);
      fixWinButtons( bd_window );
      bd_anaflex_enables(false);
      bd_window->show();
   }
}

void US_Hydrodyn::show_bd_options()
{
#ifdef NO_BD
   US_Static::us_message(us_tr("Please note:"),
                        us_tr("Function not available in this version." ) );
   return;
#endif
   if (bd_options_widget)
   {
      if (bd_options_window->isVisible())
      {
         bd_options_window->raise();
      }
      else
      {
         bd_options_window->show();
      }
      return;
   }
   else
   {
      bd_options_window = new US_Hydrodyn_BD_Options(&bd_options, &bd_options_widget, this);
      fixWinButtons( bd_options_window );
      bd_options_window->show();
   }
}

void US_Hydrodyn::show_anaflex_options()
{
#ifdef NO_BD
   US_Static::us_message(us_tr("Please note:"),
                        us_tr("Function not available in this version." ) );
   return;
#endif
   if (anaflex_options_widget)
   {
      if (anaflex_options_window->isVisible())
      {
         anaflex_options_window->raise();
      }
      else
      {
         anaflex_options_window->show();
      }
      return;
   }
   else
   {
      anaflex_options_window = new US_Hydrodyn_Anaflex_Options(&anaflex_options, &anaflex_options_widget, this);
      fixWinButtons( anaflex_options_window );
      anaflex_options_window->show();
   }
}

void US_Hydrodyn::show_bead_output()
{
   if (bead_output_widget)
   {
      if (bead_output_window->isVisible())
      {
         bead_output_window->raise();
      }
      else
      {
         bead_output_window->show();
      }
      return;
   }
   else
   {
      bead_output_window = new US_Hydrodyn_Bead_Output(&bead_output, &bead_output_widget, this);
      fixWinButtons( bead_output_window );
      bead_output_window->show();
   }
}

void US_Hydrodyn::show_grid()
{
   if (grid_widget)
   {
      if (grid_window->isVisible())
      {
         grid_window->raise();
      }
      else
      {
         grid_window->show();
      }
      return;
   }
   else
   {
      grid_window = new US_Hydrodyn_Grid(&grid_exposed_overlap,
                                         &grid_buried_overlap,
                                         &grid_overlap,
                                         &replicate_o_r_method_grid,
                                         &grid,
                                         &overlap_tolerance,
                                         &grid_widget,
                                         this);
      fixWinButtons( grid_window );
      grid_window->show();
   }
}

void US_Hydrodyn::pdb_parsing()
{
   if (pdb_parsing_widget)
   {
      if (pdb_parsing_window->isVisible())
      {
         pdb_parsing_window->raise();
      }
      else
      {
         pdb_parsing_window->show();
      }
      return;
   }
   else
   {
      pdb_parsing_window = new US_Hydrodyn_PDB_Parsing(&pdb_parse, &pdb_parsing_widget, this);
      fixWinButtons( pdb_parsing_window );
      pdb_parsing_window->show();
   }
}

void US_Hydrodyn::pdb_visualization()
{
   if (pdb_visualization_widget)
   {
      if (pdb_visualization_window->isVisible())
      {
         pdb_visualization_window->raise();
      }
      else
      {
         pdb_visualization_window->show();
      }
      return;
   }
   else
   {
      pdb_visualization_window = new US_Hydrodyn_PDB_Visualization(&pdb_vis, &pdb_visualization_widget);
      fixWinButtons( pdb_visualization_window );
      pdb_visualization_window->show();
   }
}

void US_Hydrodyn::load_config()
{
   QString fname = QFileDialog::getOpenFileName( 0 , "Please select a SOMO configuration file..." , somo_dir , "*.config" , 0 );
   if ( fname == QString::null )
   {
      QColor save_color = editor->textColor();
      editor->setTextColor("red");
      editor->append(us_tr("\nLoad configuration canceled."));
      editor->setTextColor(save_color);
      display_default_differences();
      return;
   }

   if ( read_config(fname) )
   {
      US_Static::us_message(us_tr("Please note:"),
                           us_tr("The configuration file was found to be corrupt.\n"
                              "Resorting to default values."));
      set_default();
   }
   clear_display();
}

void US_Hydrodyn::write_config()
{
   QString fname = US_Config::get_home_dir() + "etc/somo.config";
   switch (
           QMessageBox::question(
                                 this,
                                 windowTitle() + us_tr(": Save configuration "),
                                 us_tr( "Save the current configuration as 'startup' configuration?" ),
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
      fname = QFileDialog::getSaveFileName( 0 , "Please name your SOMO configuration file..." , somo_dir , "*.config" , 0 );
      break;
   default :
      return;
      break;
   }

   if ( fname.isEmpty() )
   {
      return;
   }

   if (fname.right(7) != ".config")
   {
      fname += ".config";
   }

   write_config( fname );
}

void US_Hydrodyn::do_reset()
{
   sidechain_overlap = default_sidechain_overlap;
   mainchain_overlap = default_mainchain_overlap;
   buried_overlap = default_buried_overlap;
   grid_exposed_overlap = default_grid_exposed_overlap;
   grid_buried_overlap = default_grid_buried_overlap;
   grid_overlap = default_grid_overlap;
   bead_output = default_bead_output;
   asa = default_asa;
   misc = default_misc;
   overlap_tolerance = default_overlap_tolerance;
   hydro = default_hydro;
   pdb_vis = default_pdb_vis;
   pdb_parse = default_pdb_parse;
   grid = default_grid;
   saxs_options = default_saxs_options;
   batch = default_batch;
   bd_options = default_bd_options;
   dmd_options = default_dmd_options;
   anaflex_options = default_anaflex_options;
   gparams = default_gparams;
   //  save = default_save;
}

void US_Hydrodyn::reset()
{
   QMessageBox mb(us_tr("UltraScan"), us_tr("Attention:\nAre you sure you want to reset to the default options?\nAll currently defined options will be reset."),
                  QMessageBox::Information,
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::Cancel | QMessageBox::Escape,
                  QMessageBox::No);
   mb.setButtonText(QMessageBox::Yes, us_tr("Yes"));
   mb.setButtonText(QMessageBox::Cancel, us_tr("Cancel"));
   mb.setButtonText(QMessageBox::No, us_tr("Save Current Options"));
   switch(mb.exec())
   {
   case QMessageBox::Cancel:
      {
         return;
      }
   case QMessageBox::No:
      {
         write_config();
      }
   }
   do_reset();
   clear_display();
}

void US_Hydrodyn::select_residue_file()
{
   QString old_filename = residue_filename;
   residue_filename = QFileDialog::getOpenFileName( this , windowTitle() , US_Config::get_home_dir() + "/etc" , "*.residue *.RESIDUE" );
   if (residue_filename.isEmpty())
   {
      residue_filename = old_filename;
      return;
   }
   read_residue_file();
   create_fasta_vbar_mw();
   set_disabled();
   lbl_table->setText( QDir::toNativeSeparators( residue_filename ) );
}

void US_Hydrodyn::reload_pdb()
{
   if ( advanced_config.debug_1 )
   {
      printf("Reload PDB file called %d %d.\n", pdb_file.isEmpty(), !pb_somo->isEnabled());
   }
   if ( pdb_file.isEmpty() ) // why did we ever do this ? || !pb_somo->isEnabled() )
   {
      return;
   }
   editor->append("\nReloading PDB file.\n");
   if ( advanced_config.debug_1 )
   {
      printf("Reloading PDB file.\n");
   }
   int errors_found = 0;
   if ( misc.pb_rule_on )
   {
      residue_list = save_residue_list;
   }
   else
   {
      residue_list = save_residue_list_no_pbr;
   }
   multi_residue_map = save_multi_residue_map;
   read_pdb(pdb_file);
   QString error_string = "";
   for(unsigned int i = 0; i < model_vector.size(); i++)
   {
      multi_residue_map = save_multi_residue_map;
      editor->append(QString("Checking the pdb structure for model %1\n").arg( model_name( i ) ) );
      if (check_for_missing_atoms(&error_string, &model_vector[i]))
      {
         errors_found++;
         editor->append(QString("Encountered errors with your PDB structure for model %1:\n").
                        arg( model_name( i ) ) + error_string);
         printError(QString("Encountered errors with your PDB structure for model %1:\n").
                    arg( model_name( i ) ) + "please check the text window");
      }
      reset_chain_residues( &model_vector[ i ] );
   }
   model_vector_as_loaded = model_vector;
   set_pdb_info( "reload pdb" );
   editor->append(QString("Loaded pdb file : %1\n").arg(errors_found ? "ERRORS PRESENT" : "ok"));
   if ( errors_found )
   {
      calc_vol_for_saxs();
   } else {
      calc_mw();
   }
   bead_models.resize( model_vector.size() );
   somo_processed.resize( model_vector.size() );
   update_vbar();
   pb_somo->setEnabled(true);
   pb_somo_o->setEnabled(true);
   pb_grid_pdb->setEnabled(true);
   pb_vdw_beads->setEnabled(true);
   pb_grid->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_bead_saxs->setEnabled(false);
   pb_rescale_bead_model->setEnabled(false);
   pb_pdb_saxs->setEnabled(true);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   le_bead_model_file->setText(" not selected ");
   bead_models_as_loaded = bead_models;
}

void US_Hydrodyn::clear_pdb_info( QString /* msg */ ) {
   // us_qdebug( QString( "clear_pdb_info() %1" ). arg( msg ) );
   pdb_info.clear( );
}

void US_Hydrodyn::set_pdb_info( QString /* msg */ ) {
   if ( misc.pb_rule_on ) {
      // us_qdebug( QString( "set_pdb_info() %1 setting pb_rule ON" ). arg( msg ) );
      pdb_info.insert( "pb_rule" );
   } else {
      // us_qdebug( QString( "set_pdb_info() %1 setting pb_rule OFF" ). arg( msg ) );
      pdb_info.erase( "pb_rule" );
   }
}

void US_Hydrodyn::sync_pdb_info( QString /* msg */ ) {
   // us_qdebug( QString( "sync_pdb_info() %1 called misc.pb_rule_on %2 pdb_info.count( 'pb_rule' ) %3" )
   //         .arg( msg )
   //         .arg( misc.pb_rule_on ? "true" : "false" )
   //         .arg( pdb_info.count( "pb_rule" ) ? "true" : "false" )
   //         );
   
   if ( (  misc.pb_rule_on && !pdb_info.count( "pb_rule" ) ) ||
        ( !misc.pb_rule_on && pdb_info.count( "pb_rule" ) ) ) {
      if ( misc_widget ) {
         misc_window->close();
         delete misc_window;
         misc_widget = false;
      }
           
      if ( misc.pb_rule_on && !pdb_info.count( "pb_rule" ) ) {
         // us_qdebug( QString( "sync_pdb_info() %1 misc.pb_rule was ON, now OFF" ). arg( msg ) );
         misc.pb_rule_on      = false;
         misc.restore_pb_rule = true;
      } else {
         // us_qdebug( QString( "sync_pdb_info() %1 misc.pb_rule was OFF, now ON" ). arg( msg ) );
         misc.pb_rule_on      = true;
         misc.restore_pb_rule = false;
      }
   }
}

// add store & reset code for these in state_info ?

int US_Hydrodyn::issue_non_coded( bool quiet ) {
   us_qdebug( "issue_non_coded()" );
   if ( quiet || advanced_config.expert_mode ) {
      us_qdebug( "issue_non_coded() returning quiet or expert" );
      switch ( pdb_parse.missing_residues ) {
      case 0 : // list & stop op
         return ISSUE_RESPONSE_STOP;
         break;
      case 1 : // list and skip
         return ISSUE_RESPONSE_NC_SKIP;
         break;
      case 2 : // abb
      default :
         return ISSUE_RESPONSE_NC_REPLACE;
         break;
      }
   }

   if ( issue_info.count( "stop" ) ) {
      us_qdebug( "issue_non_coded() auto return stop" );
      return ISSUE_RESPONSE_STOP;
   }
   if ( issue_info.count( "nc_skip" ) ) {
      us_qdebug( "issue_non_coded() auto return skip" );
      return ISSUE_RESPONSE_NC_SKIP;
   }
   if ( issue_info.count( "nc_replace" ) ) {
      us_qdebug( "issue_non_coded() auto return replace" );
      return ISSUE_RESPONSE_NC_REPLACE;
   }

   switch ( pdb_parse.missing_residues ) {
   case 0 :
      us_qdebug( "issue_non_coded() return stop" );
      return ISSUE_RESPONSE_STOP;
      break;
   case 1 :
      {
         us_qdebug( "issue_non_coded() setting skip q" );
         switch( QMessageBox::question(
                                       this
                                       ,us_tr( "US-SOMO: Non coded residue" )
                                       ,us_tr("A non-coded residue was encountered\n"
                                           "It can be skipped or you can stop processing.\n"
                                           "This choice will be maintained for all remaining non coded residues encountered in this PDB file.\n"
                                           "For specific residue details, please review the main panel text area after making your selection.\n"
                                           "\n"
                                           "If your model contains non-coded residues, the calculated molecular weight\n"
                                           "and vbar may be incorrect. Therefore, you could manually enter a global\n"
                                           "value for the molecular weight in the SOMO hydrodynamic options, and a\n"
                                           "global value for the vbar in the SOMO Miscellaneous options.\n"
                                           "\n"
                                           "Please select your option below"
                                           )
                                       ,us_tr( "Skip non-coded residues" )
                                       ,us_tr( "Stop processing" )
                                       ,QString::null
                                       ,1
                                       ,1
                                       ) ) {
         case 0 : 
            issue_info.insert( "nc_skip" );
            return ISSUE_RESPONSE_NC_SKIP;
            break;

         case 1 : 
         default :
            issue_info.insert( "stop" );
            return ISSUE_RESPONSE_STOP;
            break;
         }
      }
      break;
   case 2 :
      {
         us_qdebug( "issue_non_coded() setting abb q" );
         switch( QMessageBox::question(
                                       this
                                       ,us_tr( "US-SOMO: Non coded residue" )
                                       ,us_tr("A non-coded residue was encountered\n"
                                           "It can be replaced with an average residue, skipped or you can stop processing.\n"
                                           "This choice will be maintained for all remaining non coded residues encountered in this PDB file.\n"
                                           "For specific residue details, please review the main panel text area after making your selection.\n"
                                           "\n"
                                           "If your model contains non-coded residues, the calculated molecular weight\n"
                                           "and vbar may be incorrect. Therefore, you could manually enter a global\n"
                                           "value for the molecular weight in the SOMO hydrodynamic options, and a\n"
                                           "global value for the vbar in the SOMO Miscellaneous options. You can also\n"
                                           "review the average residue settings in the SOMO Miscellaneous options.\n"
                                           "\n"
                                           "Please select your option below"
                                           )
                                       ,us_tr( "Replace non-coded with average residues" )
                                       ,us_tr( "Skip non-coded residues" )
                                       ,us_tr( "Stop processing" )
                                       ,2
                                       ,2
                                       ) ) {
         case 0 : 
            issue_info.insert( "nc_replace" );
            return ISSUE_RESPONSE_NC_REPLACE;
            break;

         case 1 : 
            issue_info.insert( "nc_skip" );
            return ISSUE_RESPONSE_NC_SKIP;
            break;

         case 2 : 
         default :
            issue_info.insert( "stop" );
            return ISSUE_RESPONSE_STOP;
            break;
         }
      }
      break;
   default :
      break;

   }

   us_qdebug( "issue_non_coded() fall thru" );

   return ISSUE_RESPONSE_STOP;
}

int US_Hydrodyn::issue_missing_atom( bool quiet ) {

   us_qdebug( "issue_missing_atom()" );
   if ( quiet || advanced_config.expert_mode ) {
      us_qdebug( "issue_missing_atom() returning quiet or expert" );
      switch ( pdb_parse.missing_atoms ) {
      case 0 : // list & stop op
         return ISSUE_RESPONSE_STOP;
         break;
      case 1 : // list and skip
         return ISSUE_RESPONSE_MA_SKIP;
         break;
      case 2 : // approx
      default :
         return ISSUE_RESPONSE_MA_MODEL;
         break;
      }
   }

   if ( issue_info.count( "stop" ) ) {
      us_qdebug( "issue_missing_atom() auto return stop" );
      return ISSUE_RESPONSE_STOP;
   }
   if ( issue_info.count( "ma_skip" ) ) {
      us_qdebug( "issue_missing_atom() auto return skip" );
      return ISSUE_RESPONSE_MA_SKIP;
   }
   if ( issue_info.count( "ma_model" ) ) {
      us_qdebug( "issue_missing_atom() auto return model" );
      return ISSUE_RESPONSE_MA_MODEL;
   }

   switch ( pdb_parse.missing_atoms ) {
   case 0 :
      us_qdebug( "issue_missing_atom() return stop" );
      return ISSUE_RESPONSE_STOP;
      break;
   case 1 :
      {
         us_qdebug( "issue_missing_atom() setting skip q" );
         switch( QMessageBox::question(
                                       this
                                       ,us_tr( "US-SOMO: Missing or extra atoms" )
                                       ,us_tr("A missing or extra atom was encountered when comparing with the residue table\n"
                                           "It can be skipped or you can stop processing.\n"
                                           "This choice will be maintained for all remaining missing atoms encountered in this PDB file.\n"
                                           "For specific residue details, please review the main panel text area after making your selection.\n"
                                           "\n"
                                           "If your model contains missing atoms, the calculated molecular\n"
                                           "weight and vbar may be incorrect, and you should manually enter\n"
                                           "a global value for the molecular weight in the SOMO hydrodynamic\n"
                                           "options, and a global value for the vbar in the SOMO Miscellaneous\n"
                                           "options.\n"
                                           "\n"
                                           "Please select your option below"
                                           )
                                       ,us_tr( "Skip residues with missing atoms" )
                                       ,us_tr( "Stop processing" )
                                       ,QString::null
                                       ,1
                                       ,1
                                       ) ) {
         case 0 : 
            issue_info.insert( "ma_skip" );
            return ISSUE_RESPONSE_MA_SKIP;
            break;

         case 1 : 
         default :
            issue_info.insert( "stop" );
            return ISSUE_RESPONSE_STOP;
            break;
         }
      }
      break;
   case 2 :
      {
         us_qdebug( "issue_missing_atom() setting approx q" );
         switch( QMessageBox::question(
                                       this
                                       ,us_tr( "US-SOMO: Missing or extra atoms" )
                                       ,us_tr("A missing or extra atom was encountered when comparing with the residue table\n"
                                           "It can be modeled with an approximate method, skipped or you can stop processing.\n"
                                           "This choice will be maintained for all remaining missing atoms encountered in this PDB file.\n"
                                           "For specific residue details, please review the main panel text area after making your selection.\n"
                                           "\n"
                                           "If you model with an approximate method, the shape of this residue will\n"
                                           "be different from an exact residue which can effect the hydrodynamic computations\n"
                                           "\n"
                                           "If you skip missing atoms, the calculated molecular\n"
                                           "weight and vbar may be incorrect, and you should manually enter\n"
                                           "a global value for the molecular weight in the SOMO hydrodynamic\n"
                                           "options, and a global value for the vbar in the SOMO Miscellaneous\n"
                                           "options.\n"
                                           "\n"
                                           "Please select your option below"
                                           )
                                       ,us_tr( "Model with the approximate method" )
                                       ,us_tr( "Skip residues with missing atoms" )
                                       ,us_tr( "Stop processing" )
                                       ,2
                                       ,2
                                       ) ) {
         case 0 : 
            issue_info.insert( "ma_model" );
            return ISSUE_RESPONSE_MA_MODEL;
            break;

         case 1 : 
            issue_info.insert( "ma_skip" );
            return ISSUE_RESPONSE_MA_SKIP;
            break;

         case 2 : 
         default :
            issue_info.insert( "stop" );
            return ISSUE_RESPONSE_STOP;
            break;
         }
      }
      break;
   default :
      break;

   }

   us_qdebug( "issue_missing_atom() fall thru" );

   return ISSUE_RESPONSE_STOP;
}

void US_Hydrodyn::load_pdb()
{
   issue_info.clear( );
   clear_pdb_info( "load_pdb" );
   if ( misc.restore_pb_rule ) {
      us_qdebug( "load_pdb() restoring pb rule" );
      if ( misc_widget ) {
         misc_window->close();
         delete misc_window;
         misc_widget = false;
      }
         
      misc.pb_rule_on      = true;
      misc.restore_pb_rule = false;
   }

   QString message = "";
   if ( 0 && pdb_parse.missing_residues == 1 &&
        !advanced_config.expert_mode )
   {
      message += us_tr("You have selected to skip missing residues. If your model contains missing\n"
                    "residues, the calculated molecular weight and vbar may be incorrect, and\n"
                    "you should manually enter a global value for the molecular weight in the\n"
                    "SOMO hydrodynamic options, and a global value for the vbar in the SOMO\n"
                    "Miscellaneous options.\n\nAre you sure you want to proceed?");
   }
   if ( 0 && pdb_parse.missing_residues == 2 &&
        !advanced_config.expert_mode )
   {
      message += us_tr("You have selected to replace non-coded residues with an average residue.\n"
                    "If your model contains non-coded residues, the calculated molecular weight\n"
                    "and vbar may be incorrect. Therefore, you could manually enter a global\n"
                    "value for the molecular weight in the SOMO hydrodynamic options, and a\n"
                    "global value for the vbar in the SOMO Miscellaneous options. You can also\n"
                    "review the average residue settings in the SOMO Miscellaneous options.\n\n"
                    "Are you sure you want to proceed?");
   }
   if (message != "")
   {
      QMessageBox mb(us_tr("UltraScan"), us_tr("Attention:\n" + message),
                     QMessageBox::Information,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::Cancel | QMessageBox::Escape,
                     Qt::NoButton);
      mb.setButtonText(QMessageBox::Yes, us_tr("Yes"));
      mb.setButtonText(QMessageBox::Cancel, us_tr("Cancel"));
      switch(mb.exec())
      {
         case QMessageBox::Cancel:
         {
            return;
         }
      }
   }
   message = "";
   if ( 0 && pdb_parse.missing_atoms == 1 &&
        !advanced_config.expert_mode )
   {
      message += us_tr("You have selected to skip coded residues containing missing atoms.\n"
                    "If your model contains missing atoms, the calculated molecular\n"
                    "weight and vbar may be incorrect, and you should manually enter\n"
                    "a global value for the molecular weight in the SOMO hydrodynamic\n"
                    "options, and a global value for the vbar in the SOMO Miscellaneous\n"
                    "options.\n\nAre you sure you want to proceed?");
   }
   if ( 0 && pdb_parse.missing_atoms == 2 &&
        !advanced_config.expert_mode )
   {
      message += us_tr("You have selected to model coded residues with missing atoms\n"
                    "with an approximate method.  For best results, you should complete\n"
                    "the structure.\n\n"
                    "Do you want to proceed anyway?");
   }
   if (message != "")
   {
      QMessageBox mb(us_tr("UltraScan"), us_tr("Attention:\n" + message),
                     QMessageBox::Information,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::Cancel | QMessageBox::Escape,
                     Qt::NoButton);
      mb.setButtonText(QMessageBox::Yes, us_tr("Yes"));
      mb.setButtonText(QMessageBox::Cancel, us_tr("Cancel"));
      switch(mb.exec())
      {
      case QMessageBox::Cancel:
         {
            return;
         }
      }
   }
   cout << somo_pdb_dir << endl;
   
   QString use_dir = 
      path_load_pdb.isEmpty() ?
      somo_pdb_dir :
      path_load_pdb;

   select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , "Please select a PDB file..." , use_dir , "Structures (*.pdb *.PDB)" );

   if ( !filename.isEmpty() )
   {
      path_load_pdb = QFileInfo(filename).absolutePath();
   }

   if ( QFileInfo(filename).fileName().contains(" ") )
   {
      printError(us_tr("Filenames containing spaces are not currently supported.\n"
                    "Please rename the file to remove spaces."));
      return;
   }


   int errors_found = 0;
   if (!filename.isEmpty())
   {
      add_to_directory_history( filename );
      if ( is_dammin_dammif(filename) ) 
      {
         screen_bead_model(filename);
#if defined(START_RASMOL)
         if ( advanced_config.auto_view_pdb ) {
            model_viewer( filename );
         }
#endif
         return;
      }
      pdb_file = filename;
      options_log = "";
      last_abb_msgs = "";
      bead_model_from_file = false;
      le_pdb_file_save_text = QDir::toNativeSeparators( filename );
      le_pdb_file->setText( QDir::toNativeSeparators( filename ) );
      clear_display();

#if defined(START_RASMOL)
      if ( advanced_config.auto_view_pdb ) {
         model_viewer( filename );
      }
#endif
      QFileInfo fi(filename);
      project = fi.baseName();
      new_residues.clear( );
      if ( misc.pb_rule_on )
      {
         residue_list = save_residue_list;
      }
      else
      {
         residue_list = save_residue_list_no_pbr;
      }
      multi_residue_map = save_multi_residue_map;
      read_pdb(filename);
      QString error_string = "";
      for(unsigned int i = 0; i < model_vector.size(); i++)
      {
         multi_residue_map = save_multi_residue_map;
         editor->append(QString("Checking the pdb structure for model %1\n").arg( model_name( i ) ) );
         if (check_for_missing_atoms(&error_string, &model_vector[i]))
         {
            errors_found++;
            editor->append(QString("Encountered errors with your PDB structure for model %1:\n").
                           arg( model_name( i ) ) + error_string);
            printError(QString("Encountered errors with your PDB structure for model %1:\n").
                       arg( model_name( i ) ) + "please check the text window");
         }
         reset_chain_residues( &model_vector[ i ] );
      }
      model_vector_as_loaded = model_vector;
      set_pdb_info( "load_pdb" );
         

      editor->append(QString("Loaded pdb file : %1\n").arg(errors_found ? "ERRORS PRESENT" : "ok"));
      bead_models.clear( );
      somo_processed.clear( );
      state = PDB_LOADED;
   }
   else
   {
      return; // user canceled loading PDB file
   }
   if ( errors_found )
   {
      calc_vol_for_saxs();
   } else {
      calc_mw();
   }
   update_vbar();
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }
   // bead_model_prefix = "";
   pb_somo->setEnabled(true);
   pb_somo_o->setEnabled(true);
   pb_grid_pdb->setEnabled(true);
   pb_vdw_beads->setEnabled(true);
   pb_grid->setEnabled(false);
   bd_anaflex_enables(true);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_pdb_saxs->setEnabled(true);
   pb_bead_saxs->setEnabled(false);
   pb_rescale_bead_model->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   le_bead_model_file->setText(" not selected ");
   bead_models_as_loaded = bead_models;

   // if ( lb_model->count() )
   // {
   //    select_model(0);
   // }

   if ( dmd_options.pdb_static_pairs )
   {
      dmd_static_pairs();
   }

   if ( advanced_config.auto_calc_somo  &&
        !errors_found )
   {
      calc_somo();
   }
}

bool US_Hydrodyn::screen_pdb(QString filename, bool display_pdb, bool skipclearissue )
{
   if ( !skipclearissue ) {
      issue_info.clear( );
   }
   clear_pdb_info( "screen_pdb" );

   if ( misc.restore_pb_rule ) {
      // us_qdebug( "screen_pdb() restoring pb rule" );
      if ( misc_widget ) {
         misc_window->close();
         delete misc_window;
         misc_widget = false;
      }
      misc.pb_rule_on      = true;
      misc.restore_pb_rule = false;
   }

   cout << QString( "screen pdb display is %1\n" ).arg( display_pdb ? "true" : "false" );
   pdb_file = filename;

   if ( QFileInfo(filename).fileName().contains(" ") )
   {
      printError(us_tr("Filenames containing spaces are not currently supported.\n"
                    "Please rename the file to remove spaces."));
      return false;
   }

   if ( is_dammin_dammif(filename) )
   {
      return screen_bead_model(filename);
   }

   options_log = "";
   last_abb_msgs = "";
   bead_model_from_file = false;
   int errors_found = 0;
   le_pdb_file_save_text = QDir::toNativeSeparators( filename );
   le_pdb_file->setText( QDir::toNativeSeparators( filename ) );

   bead_model_suffix = "";
   le_bead_model_suffix->setText( bead_model_suffix );

#if defined(START_RASMOL)
   if ( display_pdb ) {
      model_viewer( filename );
   }
#endif

   QFileInfo fi(filename);
   project = fi.baseName();
   new_residues.clear( );
   if ( misc.pb_rule_on )
   {
      residue_list = save_residue_list;
   }
   else
   {
      residue_list = save_residue_list_no_pbr;
   }
   multi_residue_map = save_multi_residue_map;
   if ( read_pdb(filename) )
   {
      return false;
   }
#if defined( DEBUG_TESTING_JML )
   us_qdebug( "extra reset0" );
   reset_chain_residues( &model_vector[0]);
   us_qdebug( "after extra reset0" );
#endif

   QString error_string = "";
   for(unsigned int i = 0; i < model_vector.size(); i++)
   {
      multi_residue_map = save_multi_residue_map;
      editor->append(QString("Checking the pdb structure for model %1\n").arg( model_name( i ) ) );
#if defined( DEBUG_TESTING_JML )
      us_qdebug( "extra reset" );
      reset_chain_residues( &model_vector[i]);
      us_qdebug( "after extra reset" );
#endif
      if (check_for_missing_atoms(&error_string, &model_vector[i]))
      {
         errors_found++;
         editor->append(QString("Encountered errors with your PDB structure for model %1:\n").
                        arg( model_name( i ) ) + error_string);
         printError(QString("Encountered errors with your PDB structure for model %1:\n").
                    arg( model_name( i ) ) + "please check the text window");
      }
      reset_chain_residues( &model_vector[ i ] );
   }
   if ( errors_found )
   {
      calc_vol_for_saxs();
   } else {
      calc_mw();
   }
   model_vector_as_loaded = model_vector;
   set_pdb_info( "screen_pdb" );
   if ( !model_vector.size() ||
        !model_vector[0].molecule.size() )
   {
      QColor save_color = editor->textColor();
      editor->setTextColor("red");
      editor->append(us_tr("ERROR : PDB file contains no atoms!"));
      editor->setTextColor(save_color);
      errors_found++;
   }

   editor->append(QString("Loaded pdb file : %1\n").arg(errors_found ? "ERRORS PRESENT" : "ok"));
   bead_models.clear( );
   somo_processed.clear( );
   update_vbar();
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }
   // bead_model_prefix = "";
   pb_somo->setEnabled(errors_found ? false : true);
   pb_somo_o->setEnabled(errors_found ? false : true);
   pb_grid_pdb->setEnabled(errors_found ? false : true);
   pb_vdw_beads->setEnabled(errors_found ? false : true);
   pb_grid->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_bead_saxs->setEnabled(false);
   pb_rescale_bead_model->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   le_bead_model_file->setText(" not selected ");
   bead_models_as_loaded = bead_models;
   // if ( lb_model->count() == 1 )
   // {
   //    select_model(0);
   // }
   return errors_found ? false : true;
}   

bool US_Hydrodyn::screen_bead_model( QString filename )
{
   if ( QFileInfo(filename).fileName().contains(" ") )
   {
      printError(us_tr("Filenames containing spaces are not currently supported.\n"
                    "Please rename the file to remove spaces."));
      return false;
   }
   options_log = "";
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_grid->setEnabled(false);
   bead_model_prefix = "";
   le_bead_model_prefix->setText( bead_model_prefix );
   bead_model_suffix = "";
   le_bead_model_suffix->setText( "" );

   if ( results_widget )
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }

   bead_model_file = filename;
   le_bead_model_file->setText( QDir::toNativeSeparators( filename ) );
   bool only_overlap = false;
   if ( !read_bead_model(filename, only_overlap ))
   {
      bool so_ovlp = QFileInfo( filename ).completeBaseName().contains( "so_ovlp" );
      us_qdebug( QString( "screen bead model so_ovlp %1" ).arg( so_ovlp ? "true" : "false" ) );
      state = BEAD_MODEL_LOADED;
      pb_visualize->setEnabled(true);
      pb_equi_grid_bead_model->setEnabled(true);
      pb_calc_hydro->setEnabled( !so_ovlp );
      pb_calc_zeno->setEnabled( true );
      pb_grid->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_pdb_saxs->setEnabled(false);
      bd_anaflex_enables(false);
      return true;
   }
   else
   {
      if ( only_overlap )
      {
         state = BEAD_MODEL_LOADED;
         pb_visualize->setEnabled(true);
         pb_equi_grid_bead_model->setEnabled(true);
         pb_calc_hydro->setEnabled( false );
         pb_calc_zeno->setEnabled( true );
         pb_grid->setEnabled(true);
         pb_bead_saxs->setEnabled(true);
         pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
         pb_pdb_saxs->setEnabled(false);
         bd_anaflex_enables(false);
         return true;
      } else {
         pb_visualize->setEnabled(true);
         pb_equi_grid_bead_model->setEnabled(true);
         pb_bead_saxs->setEnabled(false);
         pb_rescale_bead_model->setEnabled(false);
         pb_pdb_saxs->setEnabled(true);
         return false;
      }
   }
}
   
void US_Hydrodyn::view_pdb()
{
   
   QString use_dir = 
      path_view_pdb.isEmpty() ?
      somo_pdb_dir :
      path_view_pdb;

   select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.pdb *.PDB" );
   
   if (!filename.isEmpty())
   {
      path_view_pdb = QFileInfo(filename).absolutePath();
      view_file(filename);
   }
}

QString US_Hydrodyn::model_name( int val )
{
   return QString( "%1" ).arg( lb_model->item( val )->text() ).replace( "Model: ", "" );
}

void US_Hydrodyn::model_selection_changed()
{
   select_model();
   // QString msg = QString( "\n%1 models selected:" ).arg( project );
   // for( int i = 0; i < lb_model->count(); i++ )
   // {
   //    if ( lb_model->item( i )->isSelected() )
   //    {
   //       current_model = i;
   //       // msg += QString( " %1" ).arg( i + 1 );
   //       msg += " " + model_name( i );
   //    }
   // }
   // msg += "\n";
   // editor->append( msg );
}

void US_Hydrodyn::select_model( int )
{
   // current_model = val;
   QString msg = QString( "\n%1 models selected:" ).arg( project );
   int selected_count = 0;
   for( int i = 0; i < lb_model->count(); i++ )
   {
      if ( lb_model->item( i )->isSelected() )
      {
         selected_count++;
         current_model = i;
         // msg += QString( " %1" ).arg( i + 1 );
         msg += " " + model_name( i );
      }
   }
   if ( selected_count ) {
      last_no_model_selected = false;
      msg += "\n";
      editor->append( msg );
   } else {
      if ( !last_no_model_selected ) {
         editor_msg( "black", "\nNo models selected." );
      }
      last_no_model_selected = true;
   }

   // check integrity of PDB file and confirm that all residues are correctly defined in residue table
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }
   pb_somo->setEnabled( selected_count );
   pb_grid_pdb->setEnabled( selected_count );
   pb_vdw_beads->setEnabled( selected_count );
   pb_somo_o->setEnabled( selected_count );
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   //   pb_pdb_saxs->setEnabled(true);
   bd_anaflex_enables( ( ( browflex && browflex->state() == QProcess::Running ) ||
                         ( anaflex && anaflex->state() == QProcess::Running ) ) ? false : true );
}

void US_Hydrodyn::write_bead_ebf(QString fname, vector<PDB_atom> *model)
{
   {
      FILE *f = us_fopen(fname.toLatin1().data(), "w");
      for (unsigned int i = 0; i < model->size(); i++)
      {
         if ((*model)[i].active)
         {
            fwrite(&((*model)[i].bead_cog_coordinate.axis[0]), sizeof(float), 3, f);
            float color[3] = { .2f, .2f, ((int)(*model)[i].all_beads.size() / 5) > 1 ? 1 : ((float)(*model)[i].all_beads.size() / 5.0f) };
            fwrite(color, sizeof(float), 3, f);
            fwrite(&(*model)[i].bead_computed_radius, sizeof(float), 1, f);
         }
      }
      fclose(f);
   }
   {
      FILE *f = us_fopen(QString("%1-info").arg(fname).toLatin1().data(), "w");
      for (unsigned int i = 0; i < model->size(); i++)
      {
         if ((*model)[i].active)
         {
            float color[3] = { .2f, .2f, ((*model)[i].all_beads.size() / 5) > 1 ? 1 : ((float)(*model)[i].all_beads.size() / 5.0f) };
            fprintf(f,"%f %f %f %f %f %f %f\n",
                    (*model)[i].bead_coordinate.axis[0],
                    (*model)[i].bead_coordinate.axis[1],
                    (*model)[i].bead_coordinate.axis[2],
                    color[0],
                    color[1],
                    color[2],
                    (*model)[i].bead_computed_radius);
         }
      }
      fclose(f);
   }
}

void US_Hydrodyn::load_bead_model()
{
   QString use_dir = 
      path_load_bead_model.isEmpty() ?
      somo_dir :
      path_load_bead_model;

   select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , "Open" , use_dir , "Bead models (*.bead_model *.BEAD_MODEL);;"
                                                   "BEAMS (*.beams *.BEAMS);;"
                                                   "DAMMIN/DAMMIF/DAMAVER (*.pdb)" , &bead_model_selected_filter );


   if ( !filename.isEmpty() )
   {
      path_load_bead_model = QFileInfo(filename).absolutePath();
   }

   if ( QFileInfo(filename).fileName().contains(" ") )
   {
      printError(us_tr("Filenames containing spaces are not currently supported.\n"
                    "Please rename the file to remove spaces."));
      return;
   }

   if (!filename.isEmpty())
   {
      add_to_directory_history( filename );

      options_log = "";
      pb_somo->setEnabled(false);
      pb_somo_o->setEnabled(false);
      pb_visualize->setEnabled(false);
      pb_equi_grid_bead_model->setEnabled(false);
      pb_calc_hydro->setEnabled(false);
      pb_calc_zeno->setEnabled(false);
      pb_show_hydro_results->setEnabled(false);
      pb_grid_pdb->setEnabled(false);
      pb_vdw_beads->setEnabled(false);
      pb_grid->setEnabled(false);
      bead_model_prefix = "";
      le_bead_model_prefix->setText(bead_model_prefix);
      bead_model_suffix = "";

      if (results_widget)
      {
         results_window->close();
         delete results_window;
         results_widget = false;
      }

      bead_model_file = filename;
      le_bead_model_file->setText( QDir::toNativeSeparators( filename ) );

      if ( is_dammin_dammif(filename) &&
           advanced_config.auto_view_pdb ) 
      {
#if defined(START_RASMOL)
         model_viewer( filename );
#endif
      }

      bool only_overlap = false;
      if (!read_bead_model(filename, only_overlap ))
      {
         bool so_ovlp = QFileInfo( filename ).completeBaseName().contains( "so_ovlp" );
         us_qdebug( QString( "load bead model so_ovlp %1" ).arg( so_ovlp ? "true" : "false" ) );
         state = BEAD_MODEL_LOADED;
         pb_visualize->setEnabled(true);
         pb_equi_grid_bead_model->setEnabled(true);
         pb_calc_hydro->setEnabled( !so_ovlp );
         pb_calc_zeno->setEnabled(true);
         pb_grid->setEnabled(true);
         pb_bead_saxs->setEnabled(true);
         pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
         pb_pdb_saxs->setEnabled(false);
         bd_anaflex_enables(false);
      }
      else
      {
         if ( only_overlap )
         {
            state = BEAD_MODEL_LOADED;
            pb_visualize->setEnabled(true);
            pb_equi_grid_bead_model->setEnabled(true);
            pb_calc_hydro->setEnabled( false );
            pb_calc_zeno->setEnabled( true );
            pb_grid->setEnabled(true);
            pb_bead_saxs->setEnabled(true);
            pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
            pb_pdb_saxs->setEnabled(false);
            bd_anaflex_enables(false);
         } else {            
            pb_visualize->setEnabled(true);
            pb_equi_grid_bead_model->setEnabled(true);
            pb_bead_saxs->setEnabled(false);
            pb_rescale_bead_model->setEnabled(false);
            pb_pdb_saxs->setEnabled(true);
         }
      }
      // bead_model_prefix = "";
   }
}

void US_Hydrodyn::update_bead_model_file(const QString &str)
{
   bead_model_file = str;
}

void US_Hydrodyn::update_bead_model_prefix(const QString &str)
{
   bead_model_prefix = str;
}

int US_Hydrodyn::calc_somo( bool no_ovlp_removal )
{
   {
      int models_selected = 0;
      for(int i = 0; i < lb_model->count(); i++) {
         if (lb_model->item(i)->isSelected()) {
            models_selected++;
         }
      }
      if ( !models_selected ) {
         editor_msg( "black", "No models selected!\n" );
         return -1;
      }
   }

   if ( selected_models_contain( "WAT" ) )
   {
      QMessageBox::warning( this,
                            us_tr( "Selected model contains WAT residue" ),
                            us_tr( 
                               "Can not process models that contain the WAT residue.\n"
                               "These are currently generated only for SAXS/SANS computations\n"
                               )
                            );
      return -1;
   }

   if ( selected_models_contain( "XHY" ) )
   {
      QMessageBox::warning( this,
                            us_tr( "Selected model contains XHY residue" ),
                            us_tr( 
                               "Can not process models that contain the XHY residue.\n"
                               "These are currently generated only for SAXS/SANS computations\n"
                               )
                            );
      return -1;
   }

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);

   bd_anaflex_enables(false);

   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_grid->setEnabled(false);
   model_vector = model_vector_as_loaded;
   sync_pdb_info( "calc_somo" );
   editor_msg( "dark blue", QString( us_tr( "Peptide Bond Rule is %1 for this PDB" ) ).arg( misc.pb_rule_on ? "on" : "off" ) );
   options_log = "";
   no_ovlp_removal ? append_options_log_somo_ovlp() : append_options_log_somo();
   display_default_differences();
   bead_model_suffix = getExtendedSuffix(false, true, no_ovlp_removal );
   le_bead_model_suffix->setText(bead_model_suffix);
   if ( !overwrite )
   {
      setSomoGridFile(true);
   }

   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      bd_anaflex_enables( ( ( browflex && browflex->state() == QProcess::Running ) ||
                            ( anaflex && anaflex->state() == QProcess::Running ) ) ? false : true );
      pb_somo->setEnabled(true);
      pb_somo_o->setEnabled(true);
      pb_grid_pdb->setEnabled(true);
      pb_vdw_beads->setEnabled(true);
      progress->reset();
      return -1;
   }

   if (!residue_list.size() ||
       !model_vector.size())
   {
      fprintf(stderr, "calculations can not be run until residue & pdb files are read!\n");
      pb_stop_calc->setEnabled(false);
      return -1;
   }
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }
   bool any_errors = false;
   bool any_models = false;
   somo_processed.resize(lb_model->count());
   bead_models.resize(lb_model->count());
   QString msg = QString("\n%1 models selected:").arg(project);
   for(int i = 0; i < lb_model->count(); i++) {
      somo_processed[i] = 0;
      if (lb_model->item(i)->isSelected()) {
         current_model = i;
         // msg += QString( " %1" ).arg( i + 1 );
         msg += " " + model_name( i );
      }
   }
   msg += "\n";
   editor->append(msg);

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++)
   {
      if (!any_errors && lb_model->item(current_model)->isSelected())
      {
         any_models = true;
         if(!compute_asa( false, no_ovlp_removal ))
         {
            somo_processed[current_model] = 1;
            if ( asa.recheck_beads && !no_ovlp_removal )
            {
               // puts("recheck beads disabled");
               editor->append("Rechecking beads\n");
               qApp->processEvents();

               bead_check(false, false);
               editor->append("Finished rechecking beads\n");
               progress->setValue(19);
            }
            else
            {
               editor->append("No rechecking of beads\n");
               qApp->processEvents();
            }
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
         pb_somo->setEnabled(true);
         pb_somo_o->setEnabled(true);
         bd_anaflex_enables( ( ( browflex && browflex->state() == QProcess::Running ) ||
                               ( anaflex && anaflex->state() == QProcess::Running ) ) ? false : true );
         pb_grid_pdb->setEnabled(true);
         pb_vdw_beads->setEnabled(true);
         progress->reset();
         return -1;
      }

      // calculate bead model and generate hydrodynamics calculation output
      // if successful, enable follow-on buttons:
   }
   if (any_models && !any_errors)
   {
      editor->append("Build bead model completed\n");
      qApp->processEvents();
      pb_visualize->setEnabled(true);
      pb_equi_grid_bead_model->setEnabled(true);
      pb_calc_hydro->setEnabled( !no_ovlp_removal );
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_somo->setEnabled(true);
   pb_somo_o->setEnabled(true);
   pb_grid_pdb->setEnabled(true);
   pb_vdw_beads->setEnabled(true);
   pb_grid->setEnabled(true);
   pb_stop_calc->setEnabled(false);
   if (calcAutoHydro)
   {
      no_ovlp_removal ? calc_zeno_hydro() : calc_hydro();
   } 
   else
   {
      play_sounds(1);
   }
   return 0;
}

int US_Hydrodyn::calc_grid_pdb( bool no_ovlp_removal )
{
   {
      int models_selected = 0;
      for(int i = 0; i < lb_model->count(); i++) {
         if (lb_model->item(i)->isSelected()) {
            models_selected++;
         }
      }
      if ( !models_selected ) {
         editor_msg( "black", "No models selected!\n" );
         return -1;
      }
   }

   //    if ( selected_models_contain( "WAT" ) )
   //    {
   //       QMessageBox::warning( this,
   //                             us_tr( "Selected model contains WAT residue" ),
   //                             us_tr( 
   //                                "Can not process models that contain the WAT residue.\n"
   //                                "These are currently generated only for SAXS/SANS computations"
   //                                )
   //                             );
   //       return -1;
   //    }

   if ( selected_models_contain( "XHY" ) )
   {
      QMessageBox::warning( this,
                            us_tr( "Selected model contains XHY residue" ),
                            us_tr( 
                               "Can not process models that contain the XHY residue.\n"
                               "These are currently generated only for SAXS/SANS computations\n"
                               )
                            );
      return -1;
   }

   overlap_reduction org_grid_exposed_overlap = grid_exposed_overlap;
   overlap_reduction org_grid_overlap         = grid_overlap;
   overlap_reduction org_grid_buried_overlap  = grid_buried_overlap;
   grid_options      org_grid                 = grid;

   if ( grid.create_nmr_bead_pdb )
   {
      if ( 
          (
           grid_exposed_overlap.remove_overlap ||
           grid_overlap.        remove_overlap ||
           grid_buried_overlap. remove_overlap
           ) &&
          (
           grid_exposed_overlap.translate_out ||
           grid_overlap.translate_out         ||
           grid_buried_overlap.translate_out 
           )
          )
      {
         grid_exposed_overlap.translate_out = false;
         grid_overlap.translate_out         = false;
         grid_buried_overlap.translate_out  = false;
         QString msg = "Temporarily turning off outward translation";
         if ( !grid.enable_asa )
         {
            grid.enable_asa = true;
            msg += " and enabling ASA screening";
         }
            
         QMessageBox::information( this,
                                   "US-SOMO: Build AtoB models with structure factors",
                                   us_tr( msg ) );

         display_default_differences();
      }
   }

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   model_vector = model_vector_as_loaded;
   sync_pdb_info( "calc_grid_pdb" );
   editor_msg( "dark blue", QString( us_tr( "Peptide Bond Rule is %1 for this PDB" ) ).arg( misc.pb_rule_on ? "on" : "off" ) );
   options_log = "";
   no_ovlp_removal ? append_options_log_atob_ovlp() : append_options_log_atob();
   display_default_differences();
   int flag = 0;
   bool any_errors = false;
   bool any_models = false;
   pb_grid->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);

   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }

   bead_model_suffix = getExtendedSuffix(false, false, no_ovlp_removal);
   le_bead_model_suffix->setText(bead_model_suffix);
   if ( !overwrite )
   {
      setSomoGridFile(false);
   }

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         printf("in calc_grid: is selected current model %d\n", current_model); fflush(stdout);
         {
            QString error_string;
            printf("in calc_grid: !somo_processed %d\n", current_model); fflush(stdout);
            // create bead model from atoms
            editor->append(QString("Gridding atom model %1\n").arg(current_model + 1));
            qApp->processEvents();
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               grid_exposed_overlap = org_grid_exposed_overlap;
               grid_overlap         = org_grid_overlap;
               grid_buried_overlap  = org_grid_buried_overlap;
               grid                 = org_grid;
               return -1;
            }
            // compute maximum position for progress
            int mpos =
               2 // create beads
               + 4 // grid_atob
               + (grid.enable_asa ?
                  ( 1 
                    + 10 // radial reduction
                    + ( asa.recheck_beads ? 2 : 0 )
                    )
                  : 0 )
               + ( ( !grid.enable_asa && grid_overlap.remove_overlap ) ?
                   ( 10 // radial reduction
                     + ( asa.recheck_beads ? 2 : 0 )
                     )
                   : 0 )
               + 1 // finish off
               ;
               
            progress->reset();
            progress->setMaximum(mpos);
            progress->setValue(progress->value() + 1);
            int retval = create_beads(&error_string);
            progress->setValue(progress->value() + 1);
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               grid_exposed_overlap = org_grid_exposed_overlap;
               grid_overlap         = org_grid_overlap;
               grid_buried_overlap  = org_grid_buried_overlap;
               grid                 = org_grid;
               return -1;
            }
            if ( retval )
            {
               editor->append("Errors found during the initial creation of beads\n");
               qApp->processEvents();
               any_errors = true;
               switch ( retval )
               {
               case US_SURFRACER_ERR_MISSING_RESIDUE:
                  {
                     printError("Encountered an unknown residue:\n" +
                                error_string);
                     break;
                  }
               case US_SURFRACER_ERR_MISSING_ATOM:
                  {
                     printError("Encountered a unknown atom:\n" +
                                error_string);
                     break;
                  }
               case US_SURFRACER_ERR_MEMORY_ALLOC:
                  {
                     printError("Encountered a memory allocation error");
                     break;
                  }
               default:
                  {
                     printError("Encountered an unknown error");
                     // unknown error
                     break;
                  }
               }
            }
            else
            {
               if(error_string.length()) {
                  printError("Encountered unknown atom(s) error:\n" +
                             error_string);
                  any_errors = true;
               }
               else
               {
                  // ok, we have the basic "bead" info loaded...
                  unsigned int i = current_model;
                  bead_model.clear( );
                  bool any_zero_si = false;
                  for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
                     for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
                        PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
                        if(this_atom->active) {
                           for (unsigned int m = 0; m < 3; m++) {
                              this_atom->bead_coordinate.axis[m] = this_atom->coordinate.axis[m];
                           }
                           this_atom->bead_number = bead_model.size();
                           this_atom->bead_computed_radius = this_atom->radius;
                           if ( grid.hydrate && this_atom->atom_hydration ) 
                           {
                              double additional_radius = 
                                 pow(3 * this_atom->atom_hydration * misc.hydrovol / (M_PI * 4), 1.0/3.0);
                              this_atom->bead_computed_radius += additional_radius;
#if defined(GRID_HYDRATE_DEBUG)
                              printf("hydrating atom %s %s %s hydration %f radius %f + %f -> %f\n"
                                     , this_atom->name.toLatin1().data()
                                     , this_atom->resName.toLatin1().data()
                                     , this_atom->resSeq.toLatin1().data()
                                     , this_atom->atom_hydration
                                     , this_atom->radius
                                     , additional_radius
                                     , this_atom->bead_computed_radius 
                                     );
#endif
                           }

                           this_atom->bead_actual_radius = this_atom->bead_computed_radius;
                           this_atom->bead_mw = this_atom->mw;
                           if ( this_atom->si == 0e0 )
                           {
                              any_zero_si = true;
                           }
                           bead_model.push_back(*this_atom);
                        }
                     }
                  }
                  if (bead_models.size() < current_model + 1) {
                     bead_models.resize(current_model + 1);
                  }

                  progress->setValue( progress->value() + 1 );
                  int save_progress = progress->value();
                  int save_total_steps = progress->maximum();
                  if ( grid.center == 2  && any_zero_si ) // ssi
                  {
                     editor_msg( "red", "Center of scattering requested, but zero scattering intensities are present" );
                     pb_grid_pdb->setEnabled(true);
                     pb_vdw_beads->setEnabled(true);
                     pb_somo->setEnabled(true);
                     pb_somo_o->setEnabled(true);
                     progress->reset();
                     grid_exposed_overlap = org_grid_exposed_overlap;
                     grid_overlap         = org_grid_overlap;
                     grid_buried_overlap  = org_grid_buried_overlap;
                     grid                 = org_grid;
                     return -1;
                  }
                     
                  bead_models[current_model] =
                     us_hydrodyn_grid_atob(&bead_model,
                                           &grid,
                                           progress,
                                           editor,
                                           this);
                  progress->setMaximum( save_total_steps );
                  progress->setValue( save_progress + 1 );
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid_pdb->setEnabled(true);
                     pb_vdw_beads->setEnabled(true);
                     pb_somo->setEnabled(true);
                     pb_somo_o->setEnabled(true);
                     progress->reset();
                     grid_exposed_overlap = org_grid_exposed_overlap;
                     grid_overlap         = org_grid_overlap;
                     grid_buried_overlap  = org_grid_buried_overlap;
                     grid                 = org_grid;
                     return -1;
                  }
                  if (errorFlag)
                  {
                     editor->append("Error occured\n\n");
                     pb_grid_pdb->setEnabled(true);
                     pb_vdw_beads->setEnabled(true);
                     pb_somo->setEnabled(true);
                     pb_somo_o->setEnabled(true);
                     progress->reset();
                     grid_exposed_overlap = org_grid_exposed_overlap;
                     grid_overlap         = org_grid_overlap;
                     grid_buried_overlap  = org_grid_buried_overlap;
                     grid                 = org_grid;
                     return -1;
                  }
                  printf("back from grid_atob 0\n"); fflush(stdout);
                  if (somo_processed.size() < current_model + 1) {
                     somo_processed.resize(current_model + 1);
                  }
                  bead_model = bead_models[current_model];

                  any_models = true;
                  somo_processed[current_model] = 1;
#if defined(DEBUG)
                  printf("back from grid_atob 1\n"); fflush(stdout);
#endif
                  if ( grid.enable_asa )
                  {
                     editor->append("ASA check\n");
                     qApp->processEvents();
                     // set all beads buried
                     for(unsigned int i = 0; i < bead_model.size(); i++) {
                        bead_model[i].exposed_code = 6;
                        bead_model[i].bead_color = 6;
                        bead_model[i].chain = 1; // all 'side' chain
                     }
                     double save_threshold = asa.threshold;
                     double save_threshold_percent = asa.threshold_percent;
                     asa.threshold = asa.grid_threshold;
                     asa.threshold_percent = asa.grid_threshold_percent;
                     progress->setValue(progress->value() + 1);
                     bead_check(true, true);
                     progress->setValue(progress->value() + 1);
                     asa.threshold = save_threshold;
                     asa.threshold_percent = save_threshold_percent;
                     bead_models[current_model] = bead_model;
#if defined(DEBUG)
                     for(unsigned int i = 0; i < bead_model.size(); i++) {
                        printf("after asa bead %d exposed %d color %d chain %d active %d mw %f vol %f cr %f [%f,%f,%f]\n",
                               i,
                               bead_model[i].exposed_code,
                               bead_model[i].bead_color,
                               bead_model[i].chain,
                               bead_model[i].active,
                               bead_model[i].bead_ref_mw,
                               bead_model[i].bead_ref_volume,
                               bead_model[i].bead_computed_radius,
                               bead_model[i].bead_coordinate.axis[0],
                               bead_model[i].bead_coordinate.axis[1],
                               bead_model[i].bead_coordinate.axis[2]

                               );
                     }
#endif
                     // now apply radial reduction with outward translation using

                     if ( !no_ovlp_removal ) {
                        // grid_exposed/buried_overlap
                        overlap_reduction save_sidechain_overlap = sidechain_overlap;
                        overlap_reduction save_mainchain_overlap = mainchain_overlap;
                        overlap_reduction save_buried_overlap = buried_overlap;
                        sidechain_overlap = grid_exposed_overlap;
                        mainchain_overlap = grid_exposed_overlap;
                        buried_overlap = grid_buried_overlap;
                        progress->setValue(progress->value() + 1);

                        radial_reduction( true );
                        sidechain_overlap = save_sidechain_overlap;
                        mainchain_overlap = save_mainchain_overlap;
                        buried_overlap = save_buried_overlap;

                        bead_models[current_model] = bead_model;
                        // grid_buried_overlap

                        if (asa.recheck_beads)
                        {
                           editor->append("Rechecking beads\n");
                           qApp->processEvents();
                           double save_threshold = asa.threshold;
                           double save_threshold_percent = asa.threshold_percent;
                           asa.threshold = asa.grid_threshold;
                           asa.threshold_percent = asa.grid_threshold_percent;
                           progress->setValue(progress->value() + 1);
                           bead_check(false, false);
                           progress->setValue(progress->value() + 1);
                           asa.threshold = save_threshold;
                           asa.threshold_percent = save_threshold_percent;
                           bead_models[current_model] = bead_model;
                        }
                     }
                  }
                  else
                  {

                     if ( !no_ovlp_removal ) {
                        if (grid_overlap.remove_overlap)
                        {
                           progress->setValue(progress->value() + 1);
                           radial_reduction( true );
                           progress->setValue(progress->value() + 1);
                           bead_models[current_model] = bead_model;
                        }
                        if (stopFlag)
                        {
                           editor->append("Stopped by user\n\n");
                           pb_grid_pdb->setEnabled(true);
                           pb_vdw_beads->setEnabled(true);
                           pb_somo->setEnabled(true);
                           pb_somo_o->setEnabled(true);
                           progress->reset();
                           grid_exposed_overlap = org_grid_exposed_overlap;
                           grid_overlap         = org_grid_overlap;
                           grid_buried_overlap  = org_grid_buried_overlap;
                           grid                 = org_grid;
                           return -1;
                        }
                     }
                     if (asa.recheck_beads)
                     {
                        editor->append("Rechecking beads\n");
                        qApp->processEvents();
                        // all buried
                        for(unsigned int i = 0; i < bead_model.size(); i++) {
                           bead_model[i].exposed_code = 6;
                           bead_model[i].bead_color = 6;
                        }
                        double save_threshold = asa.threshold;
                        double save_threshold_percent = asa.threshold_percent;
                        asa.threshold = asa.grid_threshold;
                        asa.threshold_percent = asa.grid_threshold_percent;
                        progress->setValue(progress->value() + 1);
                        bead_check(false, false);
                        progress->setValue(progress->value() + 1);
                        asa.threshold = save_threshold;
                        asa.threshold_percent = save_threshold_percent;
                        bead_models[current_model] = bead_model;
                     }
                     else
                     {
                        // all exposed
                        for(unsigned int i = 0; i < bead_model.size(); i++) {
                           bead_model[i].exposed_code = 1;
                           bead_model[i].bead_color = 8;
                        }
                        bead_models[current_model] = bead_model;
                     }
                     if (stopFlag)
                     {
                        editor->append("Stopped by user\n\n");
                        pb_grid_pdb->setEnabled(true);
                        pb_vdw_beads->setEnabled(true);
                        pb_somo->setEnabled(true);
                        pb_somo_o->setEnabled(true);
                        progress->reset();
                        grid_exposed_overlap = org_grid_exposed_overlap;
                        grid_overlap         = org_grid_overlap;
                        grid_buried_overlap  = org_grid_buried_overlap;
                        grid                 = org_grid;
                        return -1;
                     }
                  }

                  QString extra_text = "";
                  if ( grid.create_nmr_bead_pdb &&
                       sf_factors.saxs_name != "undefined" &&
                       !sf_factors.saxs_name.isEmpty() )
                  {
                     extra_text = 
                        QString( "\nSAXS exponential fitting information\n"
                                 "    Global average 4 term fit: %1\n"
                                 "    Global average 5 term fit: %2\n\n" )
                        .arg( sf_4term_notes )
                        .arg( sf_5term_notes )
                        ;

                     float tot_excl_vol = 0.0f;
                     for ( unsigned int k = 0; k < sf_bead_factors.size(); k++ )
                     {
                        extra_text += "BSAXS:: " + sf_bead_factors[ k ].saxs_name.toUpper();
                        for ( unsigned int i = 0; i < 4; i++ )
                        {
                           extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].a[ i ] );
                           extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].b[ i ] );
                        }
                        extra_text += QString( " %1 %2\n" ).arg( sf_bead_factors[ k ].c ).arg( bead_model[ k ].saxs_excl_vol );
                       
                        extra_text += "BSAXS:: " + sf_bead_factors[ k ].saxs_name;
                        for ( unsigned int i = 0; i < 5; i++ )
                        {
                           extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].a5[ i ] );
                           extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].b5[ i ] );
                        }
                        extra_text += QString( " %1 %2\n" ).arg( sf_bead_factors[ k ].c5 ).arg( bead_model[ k ].saxs_excl_vol );
                        tot_excl_vol += bead_model[ k ].saxs_excl_vol;

                        extra_text += "BSAXSV:: " + sf_bead_factors[ k ].saxs_name;
                        for ( unsigned int i = 0; i < sf_bead_factors[ k ].vcoeff.size(); i++ )
                        {
                           extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].vcoeff[ i ] );
                        }
                        extra_text += QString( " %1\n" ).arg( bead_model[ k ].saxs_excl_vol );
                        tot_excl_vol += bead_model[ k ].saxs_excl_vol;
                     }

                     extra_text += "\nSAXS:: " + sf_factors.saxs_name.toUpper();
                     for ( unsigned int i = 0; i < 4; i++ )
                     {
                        extra_text += QString( " %1" ).arg( sf_factors.a[ i ] );
                        extra_text += QString( " %1" ).arg( sf_factors.b[ i ] );
                     }
                     extra_text += QString( " %1 %2\n" ).arg( sf_factors.c ).arg( tot_excl_vol );
                       
                     extra_text += "SAXS:: " + sf_factors.saxs_name;
                     for ( unsigned int i = 0; i < 5; i++ )
                     {
                        extra_text += QString( " %1" ).arg( sf_factors.a5[ i ] );
                        extra_text += QString( " %1" ).arg( sf_factors.b5[ i ] );
                     }
                     extra_text += QString( " %1 %2\n" ).arg( sf_factors.c5 ).arg( tot_excl_vol );

                     if ( extra_saxs_coefficients.count( sf_factors.saxs_name ) )
                     {
                        editor_msg( "dark red", QString( "Notice: extra saxs coefficients %1 replaced\n" ).arg( sf_factors.saxs_name ) );
                     } else {
                        saxs_options.dummy_saxs_names.push_back( sf_factors.saxs_name );
                     }
                     extra_saxs_coefficients[ sf_factors.saxs_name ] = sf_factors;
                     saxs_options.dummy_saxs_name = sf_factors.saxs_name;
                     editor_msg( "blue", QString( "Saxs name for dummy atom models set to %1" ).arg( saxs_options.dummy_saxs_name ) );

                     if ( saxs_util->saxs_map.count( sf_factors.saxs_name ) )
                     {
                        editor_msg( "dark red", 
                                    QString( us_tr( "Notice: saxs coefficients for %1 replaced by newly loaded values\n" ) )
                                    .arg( sf_factors.saxs_name ) );
                     } else {
                        saxs_util->saxs_list.push_back( sf_factors );
                        editor_msg( "dark blue", 
                                    QString( us_tr( "Notice: added coefficients for %1 from newly loaded values\n" ) )
                                    .arg( sf_factors.saxs_name ) );
                     }
                     saxs_util->saxs_map[ sf_factors.saxs_name ] = sf_factors;

                     if ( saxs_plot_widget )
                     {
                        if ( saxs_plot_window->saxs_map.count( sf_factors.saxs_name ) )
                        {
                           editor_msg( "dark red", 
                                       QString( us_tr( "Notice: saxs window: saxs coefficients for %1 replaced by newly loaded values\n" ) )
                                       .arg( sf_factors.saxs_name ) );
                        } else {
                           saxs_plot_window->saxs_list.push_back( sf_factors );
                           editor_msg( "dark blue", 
                                       QString( us_tr( "Notice: saxs window: added coefficients for %1 from newly loaded values\n" ) )
                                       .arg( sf_factors.saxs_name ) );
                        }
                        saxs_plot_window->saxs_map[ sf_factors.saxs_name ] = sf_factors;
                     }
                  }

                  if ( saxs_options.compute_saxs_coeff_for_bead_models && grid.create_nmr_bead_pdb )
                  {
                     if ( !saxs_options.iq_global_avg_for_bead_models && sf_bead_factors.size() != bead_model.size() )
                     {
                        editor_msg( "red", 
                                    QString( us_tr( "Overriding setting to use global structure factors since bead model doesn't contain the correct number of structure factors (%1) for the beads (%2)" ) )
                                    .arg( sf_bead_factors.size() )
                                    .arg( bead_model.size() )
                                    );
                     }
                     if ( saxs_options.iq_global_avg_for_bead_models || sf_bead_factors.size() != bead_model.size() )
                     {
                        if ( !saxs_util->saxs_map.count( saxs_options.dummy_saxs_name ) )
                        {
                           editor_msg( "red", QString( us_tr("Warning: No '%1' SAXS atom found.\n" ) )
                                       .arg( saxs_options.dummy_saxs_name ) );
                           for(unsigned int i = 0; i < bead_model.size(); i++) {
                              bead_model[i].saxs_data.saxs_name = "";
                           }
                        } else {
                           editor_msg( "blue", QString( us_tr("Notice: Loading beads with saxs coefficients '%1'" ) )
                                       .arg( saxs_options.dummy_saxs_name ) );
                           for( unsigned int i = 0; i < bead_model.size(); i++ ) 
                           {
                              bead_model[i].saxs_name = saxs_options.dummy_saxs_name;
                              bead_model[i].saxs_data = saxs_util->saxs_map[ saxs_options.dummy_saxs_name ];
                              bead_model[i].hydrogens = 0;
                           }
                        }
                     } else {
                        if ( !saxs_options.iq_global_avg_for_bead_models && sf_bead_factors.size() == bead_model.size() )
                        {
                           editor_msg( "blue", us_tr("Notice: Loading beads with bead computed structure factors" ) );
                           for( unsigned int i = 0; i < bead_model.size(); i++ ) 
                           {
                              bead_model[i].saxs_name = sf_bead_factors[ i ].saxs_name;
                              bead_model[i].saxs_data = sf_bead_factors[ i ];
                              bead_model[i].hydrogens = 0;
                           }
                        }
                     }
                     bead_models[current_model] = bead_model;
                  }                    

                  editor->append( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );
                  progress->setValue(progress->value() + 1);

                  // write_bead_spt(somo_dir + SLASH + project +
                  //       (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
                  //       QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
                  //       DOTSOMO, &bead_model, bead_model_from_file);

                  QString filename = 
                     project + 
                     QString( "_%1" ).arg( current_model + 1 );

                  le_bead_model_file->setText( filename );

                  write_bead_model( 
                                   somo_dir + SLASH + filename +
                                   QString( bead_model_suffix.length() ? ( "-" + bead_model_suffix ) : "" ) +
                                   DOTSOMO, 
                                   &bead_model,
                                   extra_text
                                   );

               }
            }
            progress->setValue(progress->maximum());
         }
      }
   }

   grid_exposed_overlap = org_grid_exposed_overlap;
   grid_overlap         = org_grid_overlap;
   grid_buried_overlap  = org_grid_buried_overlap;
   grid                 = org_grid;

   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_grid_pdb->setEnabled(true);
      pb_vdw_beads->setEnabled(true);
      pb_somo->setEnabled(true);
      pb_somo_o->setEnabled(true);
      progress->reset();
      return -1;
   }

   if (any_models && !any_errors)
   {
      editor->append("Build bead model completed\n");
      qApp->processEvents();
      pb_visualize->setEnabled(true);
      pb_equi_grid_bead_model->setEnabled(true);
      pb_calc_hydro->setEnabled( !no_ovlp_removal );
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_grid_pdb->setEnabled(true);
   pb_vdw_beads->setEnabled(true);
   pb_grid->setEnabled(true);
   pb_somo->setEnabled(true);
   pb_somo_o->setEnabled(true);
   pb_stop_calc->setEnabled(false);
   if (calcAutoHydro)
   {
      calc_hydro();
   }
   else
   {
      play_sounds(1);
   }

   return (flag);
}

int US_Hydrodyn::calc_grid()
{
   {
      int models_selected = 0;
      for(int i = 0; i < lb_model->count(); i++) {
         if (lb_model->item(i)->isSelected()) {
            models_selected++;
         }
      }
      if ( !models_selected ) {
         editor_msg( "black", "No models selected!\n" );
         return -1;
      }
   }

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   append_options_log_atob();
   display_default_differences();
   int flag = 0;
   bool any_errors = false;
   bool any_models = false;
   bool grid_pdb_state = pb_grid_pdb->isEnabled();
   bool somo_state = pb_grid_pdb->isEnabled();
   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_grid->setEnabled(false);
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }

   bead_model_suffix = getExtendedSuffix(false, false) + "g";
   le_bead_model_suffix->setText(bead_model_suffix);
   if ( !overwrite )
   {
      setSomoGridFile(false);
   }

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         printf("in calc_grid: is selected current model %d\n", current_model); fflush(stdout);
         if (somo_processed.size() > current_model && somo_processed[current_model]) {
            printf("in calc_grid: somo_processed %d\n", current_model); fflush(stdout);
            editor->append(QString("Gridding bead model %1\n").arg(current_model + 1));
            // compute maximum position for progress
            int mpos =
               3 // grid_atob
               + (grid.enable_asa ?
                  ( 1 
                    + 10 // radial reduction
                    + ( asa.recheck_beads ? 2 : 0 )
                    )
                  : 0 )
               + ( ( !grid.enable_asa && grid_overlap.remove_overlap ) ?
                   ( 10 // radial reduction
                     + ( asa.recheck_beads ? 2 : 0 )
                     )
                   : 0 )
               + 1 // finish off
               ;
               
            progress->reset();
            progress->setMaximum(mpos);
            progress->setValue(progress->value() + 1);
            qApp->processEvents();

            if ( grid.center == 2 ) // ssi
            {
               bool any_zero_si = false;
               for ( unsigned int i = 0; i <  bead_models[ current_model ].size(); i++ )
               {
                  if ( bead_models[ current_model ][ i ].si == 0e0 )
                  {
                     any_zero_si = true;
                     break;
                  }
               }
               if ( any_zero_si )
               {
                  editor_msg( "red", "Center of scattering requested, but zero scattering intensities are present" );
                  pb_grid_pdb->setEnabled(true);
                  pb_vdw_beads->setEnabled(true);
                  pb_somo->setEnabled(true);
                  pb_somo_o->setEnabled(true);
                  progress->reset();
                  return -1;
               }
            }

            bead_models[current_model] =
               us_hydrodyn_grid_atob(&bead_models[current_model],
                                     &grid,
                                     progress,
                                     editor,
                                     this);
            progress->setValue(progress->value() + 1);

            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               return -1;
            }
            if (errorFlag)
            {
               editor->append("Error occured\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               return -1;
            }
            bead_model = bead_models[current_model];
            any_models = true;
            if (somo_processed.size() < current_model + 1) {
               somo_processed.resize(current_model + 1);
            }
            somo_processed[current_model] = 1;
            if ( grid.enable_asa )
            {
               editor->append("ASA check\n");
               qApp->processEvents();
               // set all beads buried
               for(unsigned int i = 0; i < bead_model.size(); i++) {
                  bead_model[i].exposed_code = 6;
                  bead_model[i].bead_color = 6;
                  bead_model[i].chain = 1; // all 'side' chain
               }
               double save_threshold = asa.threshold;
               double save_threshold_percent = asa.threshold_percent;
               asa.threshold = asa.grid_threshold;
               asa.threshold_percent = asa.grid_threshold_percent;
               progress->setValue(progress->value() + 1);
               bead_check(true, true);
               progress->setValue(progress->value() + 1);
               asa.threshold = save_threshold;
               asa.threshold_percent = save_threshold_percent;
               bead_models[current_model] = bead_model;
#if defined(DEBUG)
               for(unsigned int i = 0; i < bead_model.size(); i++) {
                  printf("after asa bead %d exposed %d color %d chain %d active %d mw %f vol %f cr %f [%f,%f,%f]\n",
                         i,
                         bead_model[i].exposed_code,
                         bead_model[i].bead_color,
                         bead_model[i].chain,
                         bead_model[i].active,
                         bead_model[i].bead_ref_mw,
                         bead_model[i].bead_ref_volume,
                         bead_model[i].bead_computed_radius,
                         bead_model[i].bead_coordinate.axis[0],
                         bead_model[i].bead_coordinate.axis[1],
                         bead_model[i].bead_coordinate.axis[2]

                         );
               }
#endif
               // now apply radial reduction with outward translation using

               // grid_exposed/buried_overlap
               overlap_reduction save_sidechain_overlap = sidechain_overlap;
               overlap_reduction save_mainchain_overlap = mainchain_overlap;
               overlap_reduction save_buried_overlap = buried_overlap;
               sidechain_overlap = grid_exposed_overlap;
               mainchain_overlap = grid_exposed_overlap;
               buried_overlap = grid_buried_overlap;
               progress->setValue(progress->value() + 1);
               radial_reduction( true );
               progress->setValue(progress->value() + 1);
               sidechain_overlap = save_sidechain_overlap;
               mainchain_overlap = save_mainchain_overlap;
               buried_overlap = save_buried_overlap;

               bead_models[current_model] = bead_model;
               // grid_buried_overlap

               if (asa.recheck_beads)
               {
                  editor->append("Rechecking beads\n");
                  qApp->processEvents();
                  double save_threshold = asa.threshold;
                  double save_threshold_percent = asa.threshold_percent;
                  asa.threshold = asa.grid_threshold;
                  asa.threshold_percent = asa.grid_threshold_percent;
                  progress->setValue(progress->value() + 1);
                  bead_check(false, false);
                  progress->setValue(progress->value() + 1);
                  asa.threshold = save_threshold;
                  asa.threshold_percent = save_threshold_percent;
                  bead_models[current_model] = bead_model;
               }
            }
            else
            {
               if (grid_overlap.remove_overlap)
               {
                  progress->setValue(progress->value() + 1);
                  radial_reduction( true );
                  progress->setValue(progress->value() + 1);
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid_pdb->setEnabled(true);
                     pb_vdw_beads->setEnabled(true);
                     pb_somo->setEnabled(true);
                     pb_somo_o->setEnabled(true);
                     progress->reset();
                     return -1;
                  }
                  bead_models[current_model] = bead_model;
               }
               if (asa.recheck_beads)
               {
                  editor->append("Rechecking beads\n");
                  qApp->processEvents();
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid_pdb->setEnabled(true);
                     pb_vdw_beads->setEnabled(true);
                     pb_somo->setEnabled(true);
                     pb_somo_o->setEnabled(true);
                     progress->reset();
                     return -1;
                  }
                  // Set all beads buried
                  for(unsigned int i = 0; i < bead_model.size(); i++) {
                     bead_model[i].exposed_code = 6;
                     bead_model[i].bead_color = 6;
                  }
                  double save_threshold = asa.threshold;
                  double save_threshold_percent = asa.threshold_percent;
                  asa.threshold = asa.grid_threshold;
                  asa.threshold_percent = asa.grid_threshold_percent;
                  progress->setValue(progress->value() + 1);
                  bead_check(false, false);
                  progress->setValue(progress->value() + 1);
                  asa.threshold = save_threshold;
                  asa.threshold_percent = save_threshold_percent;
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid_pdb->setEnabled(true);
                     pb_vdw_beads->setEnabled(true);
                     pb_somo->setEnabled(true);
                     pb_somo_o->setEnabled(true);
                     progress->reset();
                     return -1;
                  }
                  else
                  {
                     // all exposed
                     for(unsigned int i = 0; i < bead_model.size(); i++) {
                        bead_model[i].exposed_code = 1;
                        bead_model[i].bead_color = 8;
                     }
                  }
                  bead_models[current_model] = bead_model;
               }
            }
            editor->append( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );
            progress->setValue(progress->value() + 1);
            // write_bead_spt(somo_dir + SLASH + project +
            //        (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
            //        QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
            //           DOTSOMO, &bead_model, bead_model_from_file);

            QString filename = 
               project + 
               QString( "_%1" ).arg( current_model + 1 );

            le_bead_model_file->setText( filename );

            write_bead_model(somo_dir + SLASH +
                             filename +
                             QString( bead_model_suffix.length() ? ( "-" + bead_model_suffix ) : "" ) +
                             DOTSOMO, &bead_model);

            progress->setValue(progress->maximum());
         }
      }
   }

   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_grid_pdb->setEnabled(true);
      pb_vdw_beads->setEnabled(true);
      pb_somo->setEnabled(true);
      pb_somo_o->setEnabled(true);
      progress->reset();
      return -1;
   }

   if (any_models && !any_errors)
   {
      editor->append("Build bead model completed\n");
      qApp->processEvents();
      pb_visualize->setEnabled(true);
      pb_equi_grid_bead_model->setEnabled(true);
      pb_calc_hydro->setEnabled(true);
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_grid_pdb->setEnabled(grid_pdb_state);
   pb_vdw_beads->setEnabled(grid_pdb_state);
   pb_grid->setEnabled(true);
   pb_somo->setEnabled(somo_state);
   pb_somo_o->setEnabled(somo_state);
   pb_stop_calc->setEnabled(false);
   if (calcAutoHydro)
   {
      calc_hydro();
   }
   else
   {
      play_sounds(1);
   }

   return (flag);
}

void US_Hydrodyn::set_calcAutoHydro()
{
   calcAutoHydro = cb_calcAutoHydro->isChecked();
}

void US_Hydrodyn::set_setSuffix()
{
   setSuffix = cb_setSuffix->isChecked();
   display_default_differences();
}

void US_Hydrodyn::set_overwrite()
{
   overwrite = cb_overwrite->isChecked();
}

void US_Hydrodyn::set_saveParams()
{
   saveParams = cb_saveParams->isChecked();
   if ( batch_widget )
   {
      batch_window->cb_saveParams->setChecked(saveParams);
   }
}

void US_Hydrodyn::view_asa()
{
   QString use_dir = 
      path_view_asa_res.isEmpty() ?
      somo_dir :
      path_view_asa_res;

   select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.asa_res *.ASA_RES" );
   if (!filename.isEmpty())
   {
      path_view_asa_res = QFileInfo(filename).absolutePath();
      view_file(filename);
   }
}

void US_Hydrodyn::view_bead_model()
{
   QString filename = "";
   if ( last_read_bead_model != "" )
   {
      QFileInfo fi(last_read_bead_model);
      switch (
              QMessageBox::question(
                                    this,
                                    us_tr("View Bead Model File"),
                                    QString(us_tr("View last read bead model ") + fi.fileName() + " ?"),
                                    QMessageBox::Yes, 
                                    QMessageBox::No,
                                    QMessageBox::Cancel
                                    ) )
      {
      case QMessageBox::Yes : 
         filename = last_read_bead_model;
         break;
      case QMessageBox::No : 
         {
            QString use_dir = 
               path_view_bead_model.isEmpty() ?
               somo_dir :
               path_view_bead_model;
            
            select_from_directory_history( use_dir, this );

            filename = QFileDialog::getOpenFileName( this , "Open" , use_dir , "Bead models (*.bead_model *.BEAD_MODEL);;"
                                                    "BEAMS (*.beams *.BEAMS);;"
                                                    "DAMMIN/DAMMIF (*.pdb)" , &bead_model_selected_filter );

            if ( !filename.isEmpty() )
            {
               path_view_bead_model = QFileInfo(filename).absolutePath();
            }
         }
         break;
      case QMessageBox::Cancel :
      default :
         return;
         break;
      }
   } else {
      QString use_dir = 
         path_view_bead_model.isEmpty() ?
         somo_dir :
         path_view_bead_model;

      select_from_directory_history( use_dir, this );

      filename = QFileDialog::getOpenFileName( this , "Open" , use_dir , "Bead models (*.bead_model *.BEAD_MODEL);;"
                                              "BEAMS (*.beams *.BEAMS);;"
                                              "DAMMIN/DAMMIF (*.pdb)" , &bead_model_selected_filter );

      if ( !filename.isEmpty() )
      {
         path_view_bead_model = QFileInfo(filename).absolutePath();
      }
   }
   if (!filename.isEmpty())
   {
      view_file(filename);
   }
}

void US_Hydrodyn::visualize( bool movie_frame, 
                             QString dir, 
                             float scale, 
                             bool black_background )
{
   QString use_dir = ( dir == "" ? somo_dir : dir );

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         if (somo_processed[current_model]) {
            bead_model = bead_models[current_model];
            QString spt_name = 
               project +
               ( bead_model_from_file ? 
                 "" 
                 : 
                 ( QString("_%1").arg( current_model + 1 ) +
                   QString( bead_model_suffix.length() ? ("-" + bead_model_suffix) : "" ) ) );

            spt_name = spt_name.left( 30 );

            write_bead_spt( use_dir + SLASH + spt_name,
                            &bead_model, 
                            movie_frame, 
                            scale, 
                            black_background );

            editor->append(QString("Visualizing model %1\n").arg(current_model + 1));
            model_viewer( use_dir + SLASH + spt_name + ".spt", "-script" );

#if defined( TODO_FIX_MOVIE_FRAME )
            if ( movie_frame )
            {
               if (!rasmol->launch(last_spt_text))
               {
                  US_Static::us_message(us_tr("Please note:"), us_tr("There was a problem starting RASMOL\n"
                                                              "Please check to make sure RASMOL is properly installed..."));
                  return;
               }
               cout << "movie text [" << last_spt_text << "]\n";
               //               rasmol->writeToStdin(last_spt_text);
               //               rasmol->closeStdin();
               //               qApp->processEvents();
               while ( rasmol && rasmol->state() == QProcess::Running )
               {
# if defined(WIN32) && !defined( MINGW )
                  _sleep(1);
# else
                  US_Saxs_Util::us_usleep(1000);
# endif
                  qApp->processEvents();
               }
            }
#endif
         }
         else
         {
            editor->append(QString("Model %1 - selected but bead model not built\n").arg(current_model + 1));
         }
      }
   }
}

int US_Hydrodyn::calc_hydro()
{
   return do_calc_hydro();
}

int US_Hydrodyn::do_calc_hydro()
{
   if ( !overwrite )
   {
      setHydroFile();
   }

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   //   puts("calc hydro (supc)");
   display_default_differences();
   editor->append("\nBegin hydrodynamic calculations\n\n");
   results.s20w_sd = 0.0;
   results.D20w_sd = 0.0;
   results.viscosity_sd = 0.0;
   results.rs_sd = 0.0;
   results.rg_sd = 0.0;
   results.tau_sd = 0.0;

   int models_to_proc = 0;
   int first_model_no = 0;
   vector < QString > model_names;
   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         if (somo_processed[current_model]) {
            if (!first_model_no) {
               first_model_no = current_model + 1;
            }
            models_to_proc++;
            editor->append( QString( "Model %1 will be included\n").arg( model_name( current_model ) ) );
            model_names.push_back( model_name( current_model ) );
            bead_model = bead_models[current_model];

            // write_bead_spt(somo_dir + SLASH + project +
            //          (bead_model_from_file ? "" : QString("_%1").arg( model_name( current_model ) ) ) +
            //          QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
            //          DOTSOMO, &bead_model, bead_model_from_file);
         }
         else
         {
            editor->append(QString("Model %1 - selected but bead model not built\n").arg( model_name( current_model ) ) );
         }
      }
   }

   QDir::setCurrent(somo_dir);

   editor->append(QString("%1")
                  //       .arg(hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance)
                  .arg((fabs((hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance) - overlap_tolerance) > 1e-5)
                       ? QString("\nNotice: Overlap reduction bead overlap tolerance %1 does not equal the manually selected hydrodynamic calculations bead overlap cut-off %2\n")
                       .arg(overlap_tolerance).arg(hydro.overlap) : ""));

   qApp->processEvents();
   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_calc_hydro->setEnabled(true);
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
      return -1;
   }
   le_bead_model_suffix->setText(bead_model_suffix);
   int retval = us_hydrodyn_supc_main(&results,
                                      &hydro,
                                      hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance,
                                      &bead_models,
                                      &somo_processed,
                                      &model_vector,
                                      lb_model,
                                      QString(project +
                                              // (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
                                              // (bead_model_from_file ? "" : (models_to_proc == 1 ? "_1" : "_%1")) +
                                              (bead_model_from_file ? "" : "_%1") +
                                              QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
                                              DOTSOMO + ".beams").toLatin1().data(),
                                      QString(project +
                                              (bead_model_from_file ? "" : QString("_%1").arg( model_name( first_model_no - 1 ) ) ) +
                                              QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
                                              DOTSOMO + ".beams").toLatin1().data(),
                                      model_names,
                                      progress,
                                      editor,
                                      this);
   QDir::setCurrent(somo_tmp_dir);
   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_calc_hydro->setEnabled(true);
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
      return -1;
   }

   printf("back from supc retval %d\n", retval);
   pb_show_hydro_results->setEnabled(retval ? false : true);
   pb_calc_hydro->setEnabled(true);
   pb_calc_zeno->setEnabled(true);
   pb_bead_saxs->setEnabled(true);
   pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   if ( retval )
   {
      editor->append("Calculate hydrodynamics failed\n\n");
      qApp->processEvents();
      switch ( retval )
      {
      case US_HYDRODYN_SUPC_FILE_NOT_FOUND:
         {
            printError("Calculate RB Hydrodynamics SMI encountered a file not found error");
            return retval;
            break;
         }
      case US_HYDRODYN_SUPC_OVERLAPS_EXIST:
         {
            printError("Calculate RB Hydrodynamics SMI encountered overlaps in the bead model\nPerhaps you should Calculate RB Hydrodynamics ZENO which supports bead models with overlaps");
            return retval;
            break;
         }
      case US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC:
         {
            printError("Calculate RB Hydrodynamics SMI encountered a memory allocation error");
            return retval;
            break;
         }
      case US_HYDRODYN_SUPC_NO_SEL_MODELS:
         {
            printError("Calculate RB Hydrodynamics SMI was called with no processed models selected");
            return retval;
            break;
         }
      case US_HYDRODYN_PAT_ERR_MEMORY_ALLOC:
         {
            printError("US_HYDRODYN_PAT encountered a memory allocation error");
            return retval;
            break;
         }
      default:
         {
            printError("Calculate RB Hydrodynamics SMI encountered an unknown error");
            // unknown error
            return retval;
            break;
         }
      }
   }

   pb_stop_calc->setEnabled(false);
   editor->append("Calculate hydrodynamics completed\n");
   if ( advanced_config.auto_show_hydro ) 
   {
      show_hydro_results();
   }
   play_sounds(1);
   qApp->processEvents();
   return 0;
}

void US_Hydrodyn::show_hydro_results()
{
   if ( misc.hydro_supc )
   {
      if (results_widget)
      {
         if (results_window->isVisible())
         {
            results_window->raise();
         }
         else
         {
            results_window->show();
         }
         return;
      }
      else
      {
         results_window = new US_Hydrodyn_Results(&results, &results_widget);
         fixWinButtons( results_window );
         results_window->show();
      }
   }
   if ( misc.hydro_zeno )
   {
      show_zeno();
   }
}

void US_Hydrodyn::select_comparative()
{
   if (comparative_widget)
   {
      if (comparative_window->isVisible())
      {
         comparative_window->raise();
      }
      else
      {
         comparative_window->show();
      }
      return;
   }
   else
   {
      comparative_window = 
         new US_Hydrodyn_Comparative(&comparative, this, &comparative_widget);
      fixWinButtons( comparative_window );
      comparative_window->show();
   }
}

void US_Hydrodyn::best_analysis()
{
   if ( best_widget )
   {
      if ( best_window->isVisible() )
      {
         best_window->raise();
      }
      else
      {
         best_window->show();
      }
      return;
   }
   else
   {
      best_window = 
         (QWidget *) new US_Hydrodyn_Best( this, & best_widget );
      fixWinButtons( best_window );
      best_window->show();
   }
}

void US_Hydrodyn::open_hydro_results()
{
   QString filename = "";
   if ( last_hydro_res != "" )
   {
      QFileInfo fi(last_hydro_res);
      switch (
              QMessageBox::question(
                                    this,
                                    us_tr("Open Hydrodynamic Calculations File"),
                                    QString(us_tr("View last results file ") + fi.fileName() + " ?"),
                                    QMessageBox::Yes, 
                                    QMessageBox::No,
                                    QMessageBox::Cancel
                                    ) )
      {
      case QMessageBox::Yes : 
         filename = somo_dir + SLASH + last_hydro_res;
         break;
      case QMessageBox::No : 
         {
            QString use_dir = 
               path_open_hydro_res.isEmpty() ?
               somo_dir :
               path_open_hydro_res;
            
            select_from_directory_history( use_dir, this );

            filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.hydro_res *.HYDRO_RES *.zno *.ZNO" );

            if ( !filename.isEmpty() )
            {
               path_open_hydro_res = QFileInfo(filename).absolutePath();
            }
         }
         break;
      case QMessageBox::Cancel :
      default :
         return;
         break;
      }
   } else {
      QString use_dir = 
         path_open_hydro_res.isEmpty() ?
         somo_dir :
         path_open_hydro_res;

      select_from_directory_history( use_dir, this );

      filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.hydro_res *.HYDRO_RES *.zno *.ZNO" );

      if ( !filename.isEmpty() )
      {
         path_open_hydro_res = QFileInfo(filename).absolutePath();
      }
   }
   if (!filename.isEmpty())
   {
      view_file(filename);
   }
}

void US_Hydrodyn::select_save_params()
{
   if (save_widget)
   {
      if (save_window->isVisible())
      {
         save_window->raise();
      }
      else
      {
         save_window->show();
      }
      return;
   }
   else
   {
      save_window = new US_Hydrodyn_Save(&save_params, this, &save_widget);
      fixWinButtons( save_window );
      save_window->show();
   }
}

void US_Hydrodyn::show_advanced_config()
{
   if (advanced_config_widget)
   {
      if (advanced_config_window->isVisible())
      {
         advanced_config_window->raise();
      }
      else
      {
         advanced_config_window->show();
      }
      return;
   }
   else
   {
      advanced_config_window = new US_Hydrodyn_AdvancedConfig(&advanced_config, &advanced_config_widget, this);
      fixWinButtons( advanced_config_window );
      advanced_config_window->show();
   }
}

void US_Hydrodyn::show_batch()
{
   if (batch_widget)
   {
      if (batch_window->isVisible())
      {
         batch_window->raise();
      }
      else
      {
         batch_window->show();
      }
      return;
   }
   else
   {
      batch_window = new US_Hydrodyn_Batch(&batch, &batch_widget, this);
      fixWinButtons( batch_window );
      batch_window->show();
   }
}

void US_Hydrodyn::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo.html");
}

void US_Hydrodyn::stop_calc()
{
   stopFlag = true;
   anaflex_return_to_bd_load_results = false;
   if ( browflex && browflex->state() == QProcess::Running )
   {
      browflex->terminate();
      QTimer::singleShot( 1000, browflex, SLOT( kill() ) );
   }
   if ( anaflex && anaflex->state() == QProcess::Running )
   {
      anaflex->terminate();
      QTimer::singleShot( 1000, anaflex, SLOT( kill() ) );
   }
   pb_stop_calc->setEnabled(false);
}

void US_Hydrodyn::cancel()
{
   close();
}

void US_Hydrodyn::update_font()
{
   bool ok;
   QFont newFont;
   newFont = QFontDialog::getFont( &ok, ft, this );
   if ( ok )
   {
      ft = newFont;
   }
   editor->setFont(ft);
}

void US_Hydrodyn::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName( this , windowTitle() , QString::null , QString::null );
   if(!fn.isEmpty() )
   {
      QString text = editor->toPlainText();
      QFile f( fn );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
 //      editor->setModified( false );
      setWindowTitle( fn );
   }
}

void US_Hydrodyn::print()
{
#ifndef NO_EDITOR_PRINT
   const int MARGIN = 10;
   printer.setPageSize(QPrinter::Letter);

   if ( printer.setup(this) ) {      // opens printer dialog
      QPainter p;
      p.begin( &printer );         // paint on printer
      p.setFont(editor->font() );
      int yPos      = 0;         // y position for each line
      QFontMetrics fm = p.fontMetrics();
      //  QPaintDeviceMetrics metrics( &printer ); // need width/height
      // of printer surface
      for( int i = 0 ; i < editor->lines() ; i++ ) {
         if ( MARGIN + yPos > printer.height() - MARGIN ) {
            printer.newPage();      // no more room on this page
            yPos = 0;         // back to top of page
         }
         p.drawText( MARGIN, MARGIN + yPos,
                     printer.width(), fm.lineSpacing(),
                     ExpandTabs | DontClip,
                     editor->toPlainText( i ) );
         yPos = yPos + fm.lineSpacing();
      }
      p.end();            // send job to printer
   }
#endif
}

void US_Hydrodyn::clear_display()
{
   editor->clear( );
   editor->setText("\n");
   display_default_differences();
}

int US_Hydrodyn::calc_iqq( bool bead_model, bool create_native_saxs, bool do_raise )
{
   bead_model ? bead_saxs( create_native_saxs, do_raise ) : pdb_saxs( create_native_saxs, do_raise );
   saxs_plot_window->show_plot_saxs_sans();
   return 0;
}

int US_Hydrodyn::calc_prr( bool bead_model, bool create_native_saxs, bool do_raise )
{
   bead_model ? bead_saxs( create_native_saxs, do_raise ) : pdb_saxs( create_native_saxs, do_raise );
   saxs_plot_window->show_plot_pr();
   return 0;
}

void US_Hydrodyn::pdb_saxs( bool create_native_saxs, bool do_raise )
{
   //   cout << QString("ext %1 sans_sans %2 curve %3\n")
   //      .arg(saxs_sans_ext())
   //      .arg(saxs_options.saxs_sans)
   //      .arg(saxs_options.curve);

   vector < unsigned int > selected_models;
   for ( unsigned int i = 0; i < (unsigned int)lb_model->count(); i++ ) 
   {
      if ( lb_model->item(i)->isSelected() ) 
      {
         selected_models.push_back(i);
      }
   }
   if ( selected_models.size() > 1 )
   {
      US_Static::us_message(us_tr("Please note:"),
                           us_tr("You must select exactly one model to perform SAXS functions."));
   } 
   else
   {
      QString filename = project;
      // probably want to set specially for batch mode?
      if ( filename.length() )
      {
         if ( dammix_remember_mw.count(QFileInfo(filename).fileName()) &&
              !saxs_options.multiply_iq_by_atomic_volume )
         {
            if ( dammix_remember_mw[QFileInfo(filename).fileName()] != 
                 model_vector[selected_models[0]].mw )
            {
               switch ( QMessageBox::question(this, 
                                              us_tr("UltraScan Notice"),
                                              QString(us_tr("Please note:\n\n"
                                                         "You have remembered a molecular weight of %1 Daltons\n"
                                                         "but the loaded pdb has a computed molecular weight of %2 Daltons\n"
                                                         "What would you like to do?\n"))
                                              .arg(dammix_remember_mw[QFileInfo(filename).fileName()])
                                              .arg(model_vector[selected_models[0]].mw)
                                              ,
                                              us_tr("&Set to the newly computed value"),
                                              us_tr("&Keep the remembered value"), 
                                              us_tr("&Enter it manually later"),
                                              0, // Stop == button 0
                                              0 // Escape == button 0
                                              )
                        )
               {
               case 0 : 
                  dammix_remember_mw[QFileInfo(filename).fileName()] =
                     model_vector[selected_models[0]].mw;
                  dammix_remember_mw_source[QFileInfo(filename).fileName()] =
                     "computed from pdb";
                  break;
               case 1 : 
                  break;
               case 2 : 
                  dammix_remember_mw.erase(QFileInfo(filename).fileName());
                  dammix_remember_mw_source.erase(QFileInfo(filename).fileName());
                  break;
               default :
                  break;
               }
            }
         } else {
            dammix_remember_mw[QFileInfo(filename).fileName()] =
               model_vector[selected_models[0]].mw;
            dammix_remember_mw_source[QFileInfo(filename).fileName()] =
               "computed from pdb";
         }
      }
            
      if (saxs_plot_widget)
      {
         saxs_plot_window->refresh(
                                   filename,
                                   pdb_file,
                                   residue_list,
                                   model_vector,
                                   bead_models,
                                   selected_models,
                                   multi_residue_map,
                                   residue_atom_hybrid_map,
                                   0,
                                   create_native_saxs
                                   );
         if ( do_raise )
         {
            saxs_plot_window->raise();
         }
      }
      else
      {
         saxs_plot_window = new US_Hydrodyn_Saxs(
                                                 &saxs_plot_widget,
                                                 &saxs_options,
                                                 filename,
                                                 pdb_file,
                                                 residue_list,
                                                 model_vector,
                                                 bead_models,
                                                 selected_models,
                                                 multi_residue_map,
                                                 residue_atom_hybrid_map,
                                                 0, 
                                                 create_native_saxs,
                                                 this,
                                                 0
                                                 );
         fixWinButtons( saxs_plot_window );
         // always show
         saxs_plot_window->show();
      }
   }
}

void US_Hydrodyn::bead_saxs( bool create_native_saxs, bool do_raise )
{
   vector < unsigned int > selected_models;
   for ( unsigned int i = 0; i < (unsigned int)lb_model->count(); i++ ) 
   {
      if ( lb_model->item(i)->isSelected() ) 
      {
         selected_models.push_back(i);
      }
   }
   if ( selected_models.size() != 1 )
   {
      US_Static::us_message(us_tr("Please note:"),
                           us_tr("You must select exactly one model to perform SAXS functions.."));
   } 
   else
   {
      QString filename = QFileInfo(bead_model_file).completeBaseName() +
         QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "");
      if ( !QFileInfo(bead_model_file).completeBaseName().length() )
      {
         filename = project + QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "");
      }
      if ( !filename.length() )
      {
         filename = "unknown";
      }
      
      printf("selected models size %u bead_models.size %u\n",
             (unsigned int)bead_models.size(),
             (unsigned int)selected_models.size()
             );

      // compute mw
      float tmp_mw = 0.0;
      for ( unsigned int i = 0; i < bead_models[selected_models[0]].size(); i++ )
      {
         tmp_mw += bead_models[selected_models[0]][i].bead_mw;
      }

      if ( tmp_mw != 0.0 &&
           dammix_remember_mw.count(QFileInfo(filename).fileName()) )
      {
         if ( dammix_remember_mw[QFileInfo(filename).fileName()] != 
              tmp_mw )
         {
            switch ( QMessageBox::question(this, 
                                           us_tr("UltraScan Notice"),
                                           QString(us_tr("Please note:\n\n"
                                                      "You have remembered a molecular weight of %1 Daltons\n"
                                                      "but the loaded bead model a computed molecular weight of %2 Daltons\n"
                                                      "What would you like to do?\n"))
                                           .arg(dammix_remember_mw[QFileInfo(filename).fileName()])
                                           .arg(tmp_mw)
                                           ,
                                           us_tr("&Set to the newly computed value"),
                                           us_tr("&Keep the remembered value"), 
                                           us_tr("&Enter it manually later"),
                                           0, // Stop == button 0
                                           0 // Escape == button 0
                                           )
                     )
            {
            case 0 : 
               dammix_remember_mw[QFileInfo(filename).fileName()] =
                  tmp_mw;
               dammix_remember_mw_source[QFileInfo(filename).fileName()] =
                  "computed from bead model";
               break;
            case 1 : 
               break;
            case 2 : 
               dammix_remember_mw.erase(QFileInfo(filename).fileName());
               dammix_remember_mw_source.erase(QFileInfo(filename).fileName());
               break;
            default :
               break;
            }
         }
      } else {
         dammix_remember_mw[QFileInfo(filename).fileName()] = tmp_mw;
         dammix_remember_mw_source[QFileInfo(filename).fileName()] =
            "computed from bead model";
      }

      if (saxs_plot_widget)
      {
         saxs_plot_window->refresh(
                                   filename,
                                   pdb_file,
                                   residue_list,
                                   model_vector,
                                   bead_models,
                                   selected_models,
                                   multi_residue_map,
                                   residue_atom_hybrid_map,
                                   1,
                                   create_native_saxs
                                   );
         if ( do_raise )
         {
            saxs_plot_window->raise();
         }
      }
      else
      {
         saxs_plot_window = new US_Hydrodyn_Saxs(
                                                 &saxs_plot_widget,
                                                 &saxs_options,
                                                 filename,
                                                 pdb_file,
                                                 residue_list,
                                                 model_vector,
                                                 bead_models,
                                                 selected_models,
                                                 multi_residue_map,
                                                 residue_atom_hybrid_map,
                                                 1, 
                                                 create_native_saxs,
                                                 this,
                                                 0
                                                 );
         fixWinButtons( saxs_plot_window );
         // always show
         saxs_plot_window->show();
      }
   }
}

QString US_Hydrodyn::getExtendedSuffix(bool prerun, bool somo, bool no_ovlp_removal )
{
   // produce a suffix based upon settings
   // e.g.
   // A20 for ASA screen 20 
   // R50 for ASA recheck 50% 
   // hi or sy (if ALL 3 panels set to the same thing) 
   // OT outward translation
   // hy = hydration on in grid models,
   // G4 grid setting in grid models & 
   // -so for somo models

   QString result = le_bead_model_prefix->text();

   if ( setSuffix )
   {
      result += result.length() ? "-" : "";

      if ( ( asa.calculation && somo ) ||
           ( grid.enable_asa && !somo ) )
      {
         result += QString("A%1").arg(somo ? asa.threshold : asa.grid_threshold);
      }
      
      if ( asa.recheck_beads )
      {
         result += QString("R%1").arg(somo ? asa.threshold_percent : asa.grid_threshold_percent);
      }
      
      if ( somo ) 
      {
         if ( !no_ovlp_removal ) 
         {
            if ( sidechain_overlap.remove_overlap &&
                 mainchain_overlap.remove_overlap &&
                 buried_overlap.remove_overlap &&
                 (sidechain_overlap.remove_sync == 
                  mainchain_overlap.remove_sync) && 
                 (mainchain_overlap.remove_sync == 
                  buried_overlap.remove_sync ) &&
                 (sidechain_overlap.remove_hierarch == 
                  mainchain_overlap.remove_hierarch) &&
                 (mainchain_overlap.remove_hierarch == 
                  buried_overlap.remove_hierarch ) )
            {
               result += QString("%1").arg(sidechain_overlap.remove_sync ? "sy" : "hi");
            }
         
            if ( sidechain_overlap.remove_overlap &&
                 sidechain_overlap.translate_out )
            {
               result += "OT";
            }
         }
      }
      if ( !somo ) 
      {
         if ( !no_ovlp_removal ) 
         {
            if ( grid_exposed_overlap.remove_overlap &&
                 grid_buried_overlap.remove_overlap &&
                 grid_overlap.remove_overlap &&
                 (grid_exposed_overlap.remove_sync == 
                  grid_buried_overlap.remove_sync) &&
                 (grid_buried_overlap.remove_sync ==
                  grid_overlap.remove_sync ) &&
                 (grid_exposed_overlap.remove_hierarch == 
                  grid_buried_overlap.remove_hierarch) &&
                 (grid_buried_overlap.remove_hierarch == 
                  grid_overlap.remove_hierarch ) )
            {
               result += QString("%1").arg(grid_exposed_overlap.remove_sync ? "sy" : "hi");
            }
         
            if ( grid_exposed_overlap.remove_overlap &&
                 grid_exposed_overlap.translate_out )
            {
               result += "OT";
            }
         }

         if ( grid.hydrate )
         {
            result += "hy";
         }
         
         if ( grid.cubic )
         {
            result += QString("G%1").arg(grid.cube_side);
         }
      }
   }
   if ( !prerun )
   { 
      result += QString(result.length() ? "-" : "") + QString(somo ? "so" : "a2b");
      if ( somo && no_ovlp_removal )
      {
         result += "_ovlp";
      }
   }
   //   editor->append(result);
   result.replace( ".", "_" );
   return result;
}

void US_Hydrodyn::setHydroFile()
{
   bool any_changes;
   QString path;
   QString file;
   QString ext;
   QString new_file;

   do {
      any_changes = false;
      for( int i = 0; i < lb_model->count(); i++ )
      {
         if ( lb_model->item(i)->isSelected() )
         {
            path = 
               somo_dir + QDir::separator() +
               project + QString("_%1").arg(i + 1) + 
               (bead_model_suffix.length() ? "-" : "");
            
            ext = ".hydro_res";
            file = path + bead_model_suffix + ext;
            new_file = fileNameCheck( &path, &bead_model_suffix, &ext, 0 );
            if ( file != new_file )
            {
               le_bead_model_suffix->setText(bead_model_suffix);
               any_changes = true;
            }

            if ( saveParams )
            {
               ext = ".csv";
               file = path + bead_model_suffix + ext;
               new_file = fileNameCheck( &path, &bead_model_suffix, &ext, 0 );
               if ( file != new_file )
               {
                  le_bead_model_suffix->setText(bead_model_suffix);
                  any_changes = true;
               }
            }
         }
      }
   } while ( any_changes );
}

void US_Hydrodyn::setSomoGridFile(bool somo)
{
   // check all selected models, all output types

   bool any_changes;
   QString path;
   QString file;
   QString ext;
   QString new_file;

   do {
      any_changes = false;
      for( int i = 0; i < lb_model->count(); i++ )
      {
         if ( lb_model->item(i)->isSelected() )
         {
            path = 
               somo_dir + QDir::separator() +
               project + QString("_%1").arg(i + 1) + 
               (bead_model_suffix.length() ? "-" : "");
            
            if (bead_output.output & US_HYDRODYN_OUTPUT_SOMO) {
               ext = ".bead_model";
               file = path + bead_model_suffix + ext;
               new_file = fileNameCheck( &path, &bead_model_suffix, &ext, 0 );
               if ( file != new_file )
               {
                  le_bead_model_suffix->setText(bead_model_suffix);
                  any_changes = true;
               }
            }
            
            if (bead_output.output & US_HYDRODYN_OUTPUT_BEAMS) {
               ext = ".beams";
               file = path + bead_model_suffix + ext;
               new_file = fileNameCheck( &path, &bead_model_suffix, &ext, 0 );
               if ( file != new_file )
               {
                  le_bead_model_suffix->setText(bead_model_suffix);
                  any_changes = true;
               }
               // frmc = us_fopen(QString("%1.rmc").arg(fname).toLatin1().data(), "w");
               // frmc1 = us_fopen(QString("%1.rmc1").arg(fname).toLatin1().data(), "w");
            }
            
            if (bead_output.output & US_HYDRODYN_OUTPUT_HYDRO) {
               ext = ".dat";
               file = path + bead_model_suffix + ext;
               new_file = fileNameCheck( &path, &bead_model_suffix, &ext, 0 );
               if ( file != new_file )
               {
                  le_bead_model_suffix->setText(bead_model_suffix);
                  any_changes = true;
               }
            }

            if ( somo )
            {
               ext = ".asa_res";
               file = path + bead_model_suffix + ext;
               new_file = fileNameCheck( &path, &bead_model_suffix, &ext, 0 );
               if ( file != new_file )
               {
                  le_bead_model_suffix->setText(bead_model_suffix);
                  any_changes = true;
               }
            }
         }
      }
   } while ( any_changes );
}


QString US_Hydrodyn::fileNameCheck( QString filename, int mode, QWidget *p )
{
   // checks to see if file name exists, and if it does, according to 'mode'
   // mode == 0, stop and ask with the option for an new filename, mode == 1 auto increment, 

   if ( !QFile::exists(filename) )
   {
      return filename;
   }

   // file does exist
   // based upon mode, do next stuff

   QFileInfo fi(filename);
   QString path = fi.path() + QDir::separator();
   QString base = fi.completeBaseName();
   QString ext = fi.suffix();
   ext = ext.length() ? "." + ext : ext;
   fileNameCheck( &path, &base, &ext, mode, p);
   return path + base + ext;
}

QString US_Hydrodyn::fileNameCheck( QString *path, QString *base, QString *ext, int mode, QWidget *p )
{
   if ( !QFile::exists(*path + *base + *ext) )
   {
      return *path + *base + *ext;
   }

   if ( mode == 1 )
   {
      // split filename into pieces, do increment until !exists
      QRegExp rx("-(\\d+)$");
      do 
      {
         if ( rx.indexIn(*base) != -1 ) 
         {
            base->replace(rx, QString("-%1").arg(rx.cap(1).toInt() + 1));
         } else {
            *base += "-1";
         }
      } while ( QFile::exists(*path + *base + *ext) );
         
      return *path + *base + *ext;
   }

   // do the fancy window stuff with filename split up into pieces
   // make up a window with notice,
   // filename dirpath (fixed) base (editable) ext (fixed)
   // choices: auto-increment / try again 

   int result;

   US_Hydrodyn_File *hf = new US_Hydrodyn_File( path, base, ext, &result, p );
   fixWinButtons( hf );

   do 
   {
      hf->exec();
   } while ( result == -1 );

   switch ( result )
   {
   case 0 : // overwrite
   case 2 : // try again
      break;
   case 1 : // auto inc
      return fileNameCheck(path, base, ext, 1);
      break;
   }
   delete hf;
   return *path + *base + *ext;
}

QString US_Hydrodyn::fileNameCheck2( QString filename, 
                                     bool &cancel, 
                                     bool &overwrite_all, 
                                     int mode, QWidget *p )
{
   // checks to see if file name exists, and if it does, according to 'mode'
   // mode == 0, stop and ask with the option for an new filename, mode == 1 auto increment, 

   if ( !QFile::exists(filename) )
   {
      return filename;
   }

   // file does exist
   // based upon mode, do next stuff

   QFileInfo fi(filename);
   QString path = fi.path() + QDir::separator();
   QString base = fi.completeBaseName();
   QString ext = fi.suffix();
   ext = ext.length() ? "." + ext : ext;
   fileNameCheck2( &path, &base, &ext, cancel, overwrite_all, mode, p);
   return path + base + ext;
}

QString US_Hydrodyn::fileNameCheck2( QString *path, 
                                     QString *base, 
                                     QString *ext, 
                                     bool &cancel,
                                     bool &overwrite_all,
                                     int mode, 
                                     QWidget *p )
{
   if ( !QFile::exists(*path + *base + *ext) )
   {
      return *path + *base + *ext;
   }

   if ( mode == 1 )
   {
      // split filename into pieces, do increment until !exists
      QRegExp rx("-(\\d+)$");
      do 
      {
         if ( rx.indexIn(*base) != -1 ) 
         {
            base->replace(rx, QString("-%1").arg(rx.cap(1).toInt() + 1));
         } else {
            *base += "-1";
         }
      } while ( QFile::exists(*path + *base + *ext) );
         
      return *path + *base + *ext;
   }

   // do the fancy window stuff with filename split up into pieces
   // make up a window with notice,
   // filename dirpath (fixed) base (editable) ext (fixed)
   // choices: auto-increment / try again 

   int result;

   US_Hydrodyn_File2 *hf2 = new US_Hydrodyn_File2( path, 
                                                   base, 
                                                   ext, 
                                                   cancel,
                                                   overwrite_all,
                                                   &result, 
                                                   p );
   fixWinButtons( hf2 );

   do 
   {
      hf2->exec();
   } while ( result == -1 );

   switch ( result )
   {
   case 0 : // overwrite
   case 2 : // try again
      break;
   case 1 : // auto inc
      return fileNameCheck(path, base, ext, 1);
      break;
   }
   delete hf2;
   return *path + *base + *ext;
}

void US_Hydrodyn::dmd_run()
{
   if ( pdb_file.isEmpty() )
   {
      QMessageBox::information( this,
                                "US-SOMO: Run DMD",
                                us_tr( "You must load a PDB file before Run DMD" ) );
      return;
   }
      
   // save current batch_files
   batch_info save_batch_info = batch;
   batch.file.clear( );
   batch.file.push_back( pdb_file );
   batch.mm_all = true;
   batch.dmd = true;
   batch.somo = false;
   batch.grid = false;
   batch.iqq = false;
   batch.prr = false;
   batch.hydro = false;
   bool created_batch = false;
   if ( !batch_widget )
   {
      created_batch = true;
      batch_window = new US_Hydrodyn_Batch(&batch, &batch_widget, this);
      fixWinButtons( batch_window );
      batch_window->lb_files->item( 0)->setSelected( true );
   } else {
      batch_window->lb_files->clear( );
      batch_window->lb_files->addItem(batch.file[0]);
      batch_window->lb_files->item( 0)->setSelected( true );
   }
   batch_window->cb_mm_first->setChecked( !batch.mm_all );
   batch_window->cb_mm_all  ->setChecked( batch.mm_all );
   batch_window->cb_dmd     ->setChecked( true ); // batch.dmd );
   batch_window->cb_somo    ->setChecked( batch.somo );
   batch_window->cb_grid    ->setChecked( batch.grid );
   batch_window->cb_iqq     ->setChecked( batch.iqq );
   batch_window->cb_prr     ->setChecked( batch.prr );
   batch_window->cb_hydro   ->setChecked( batch.hydro );

   US_Hydrodyn_Cluster *hc = 
      new US_Hydrodyn_Cluster(
                              this,
                              batch_window );
   fixWinButtons( hc );
   hc->exec();
   delete hc;
   batch = save_batch_info;
   if ( created_batch )
   {
      batch_window->close();
   } else {
      batch_window->lb_files->clear( );
      QString load_errors;
      for ( unsigned int i = 0; i < batch.file.size(); i++ ) 
      {
         if ( batch.file[i].contains(QRegExp("(pdb|PDB|bead_model|BEAD_MODEL|beams|BEAMS)$")) )
         {
            bool dup = false;
            if ( i ) 
            {
               for ( unsigned int j = 0; j < i; j++ )
               {
                  if ( batch.file[i] == batch.file[j] )
                  {
                     dup = true;
                     break;
                  }
               }
            }
            if ( !dup )
            {
               batch_window->lb_files->addItem(batch.file[i]);
            } else {
               load_errors += QString(us_tr("File skipped: %1 (already in list)\n")).arg(batch.file[i]);
            }
         } else {
            load_errors += QString(us_tr("File skipped: %1 (not a valid file name)\n")).arg(batch.file[i]);
         }
      }
      if ( load_errors != "" ) 
      {
         QColor save_color = batch_window->editor->textColor();
         batch_window->editor->setTextColor("dark red");
         batch_window->editor->append(load_errors);
         batch_window->editor->setTextColor(save_color);
      }
      batch_window->cb_mm_first->setChecked( !batch.mm_all );
      batch_window->cb_mm_all  ->setChecked( batch.mm_all );
      batch_window->cb_dmd     ->setChecked( batch.dmd );
      batch_window->cb_somo    ->setChecked( batch.grid );
      batch_window->cb_grid    ->setChecked( batch.somo );
      batch_window->cb_iqq     ->setChecked( batch.iqq );
      batch_window->cb_prr     ->setChecked( batch.prr );
      batch_window->cb_hydro   ->setChecked( batch.hydro );
   }
}

void US_Hydrodyn::run_us_config()
{
   QProcess* process = new QProcess( this );
#if QT_VERSION < 0x040000
   QString config_prog = "us_config";
   
   process->setCommunication( 0 );
# ifndef Q_OS_MAC
   process->addArgument( config_prog );
# else
   QString procbin = USglobal->config_list.system_dir + "/bin/" + config_prog;
   QString procapp = procbin + ".app";

   if ( !QFile( procapp ).exists()  &&  QFile( procbin ).exists() )
      procapp         = procbin;

   process->addArgument( "open" );
   process->addArgument( "-a" );
   process->addArgument( procapp );
# endif

   if ( ! process->start() )
   {
      QMessageBox::information( this,
                                us_tr( "Error" ),
                                us_tr( "There was a problem creating a subprocess\n"
                                    "for " ) + QString("us_config").toUpper() );
   }
#else
   QString prog = "us3_config";
   QStringList args;
# if defined( Q_OS_MAC )
   QString config_prog = "us3_config";
   prog = "open";

   QString procbin = USglobal->config_list.system_dir + "/bin/" + config_prog;
   QString procapp = procbin + ".app";

   if ( !QFile( procapp ).exists()  &&  QFile( procbin ).exists() )
      procapp         = procbin;

   args
      <<  "-a"
      << procapp
      ;
# endif
   if ( ! process->startDetached( prog, args ) )
   {
      QMessageBox::information( this,
                                us_tr( "Error" ),
                                us_tr( "There was a problem creating a subprocess\n"
                                    "for " ) + QString("us_config").toUpper() );
   }
#endif

}

void US_Hydrodyn::run_us_admin()
{
   QProcess* process = new QProcess( this );
#if QT_VERSION < 0x040000
   process->setCommunication( 0 );
# ifndef Q_OS_MAC
   process->addArgument( "us_admin" );
# else
   QString procbin = USglobal->config_list.system_dir + "/bin/" + "us_admin";
   QString procapp = procbin + ".app";

   if ( !QFile( procapp ).exists()  &&  QFile( procbin ).exists() )
      procapp         = procbin;

   process->addArgument( "open" );
   process->addArgument( "-a" );
   process->addArgument( procapp );
# endif

   if ( ! process->start() )
   {
      QMessageBox::information( this,
                                us_tr( "Error" ),
                                us_tr( "There was a problem creating a subprocess\n"
                                    "for " ) + QString("us_admin").toUpper() );
   }
#else
   QString prog = "us_admin";
   QStringList args;
# if defined( Q_OS_MAC )
   QString admin_prog = "us_admin";
   prog = "open";

   QString procbin = USglobal->config_list.system_dir + "/bin/" + admin_prog;
   QString procapp = procbin + ".app";

   if ( !QFile( procapp ).exists()  &&  QFile( procbin ).exists() )
      procapp         = procbin;

   args
      <<  "-a"
      << procapp
      ;
# endif
   if ( ! process->startDetached( prog, args ) )
   {
      QMessageBox::information( this,
                                us_tr( "Error" ),
                                us_tr( "There was a problem creating a subprocess\n"
                                    "for " ) + QString("us_admin").toUpper() );
   }
#endif
}


void US_Hydrodyn::update_enables()
{
}

void US_Hydrodyn::sizeArrows( QwtCounter* 
#if QT_VERSION >= 0x040000 && ( defined(Q_OS_MAC) || defined(Q_OS_WIN) )
                              counter 
#endif
)
{
#if QT_VERSION >= 0x040000 && ( defined(Q_OS_MAC) || defined(Q_OS_WIN) )
   QList< QObject* > children = counter->children();
   QStyle* btnstyle = QStyleFactory::create("fusion");
   for ( int jj = 0; jj < children.size(); jj++ )
   {
      QWidget* cwidg = (QWidget*)children.at( jj );
      QString clname = cwidg->metaObject()->className();
      if ( !clname.isEmpty()  &&  clname.contains( "Button" ) )
         cwidg->setStyle( btnstyle );
   }
#endif
}

void US_Hydrodyn::fixWinButtons( QWidget* 
#if QT_VERSION >= 0x040000
                                 widg 
#endif
                                 )
{
#if QT_VERSION >= 0x040000
   QList< QObject* > children = widg->children();
   QStyle* btnstyle = QStyleFactory::create("fusion");
   for ( int jj = 0; jj < children.size(); jj++ )
   {
      QObject* cobj  = children.at( jj );
      if ( !cobj->isWidgetType() )  continue;
      QWidget* cwidg = (QWidget*)cobj;
      QString clname = cwidg->metaObject()->className();
      if ( !clname.isEmpty()  &&  clname.contains( "Button" ) )
         cwidg->setStyle( btnstyle );
   }
#endif
}

bool US_Hydrodyn::equi_grid_bead_model( double dR )
{
   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   append_options_log_atob();
   display_default_differences();
   bool any_errors = false;
   bool any_models = false;
   bool grid_pdb_state = pb_grid_pdb->isEnabled();
   bool somo_state = pb_grid_pdb->isEnabled();
   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_grid->setEnabled(false);
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }

   // get dR

   printf( "equi_grid called with dR %g\n", dR );
   if ( !dR )
   {
      bool ok;

      do {
         dR = US_Static::getDouble(
                                      "dR for equi grid:",
                                      us_tr( "Enter a cube side value in Angstroms:" ),
                                      1e0, 
                                      -1e0,
                                      1e4, 
                                      3, 
                                      &ok, 
                                      this );
         if ( !ok )
         {
            editor->append("Stopped by user\n\n");
            pb_grid_pdb->setEnabled(true);
            pb_vdw_beads->setEnabled(true);
            pb_equi_grid_bead_model->setEnabled(true);
            pb_somo->setEnabled(true);
            pb_somo_o->setEnabled(true);
            progress->reset();
            return false;
         }
      } while ( !dR );
   }

   bead_model_suffix = getExtendedSuffix(false, false).replace( "a2b", QString( "eqm%1" ).arg( dR ).replace( ".","_" ) ) + "g";
   le_bead_model_suffix->setText(bead_model_suffix);
   if ( !overwrite )
   {
      setSomoGridFile(false);
   }

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         printf("in calc_grid: is selected current model %d\n", current_model); fflush(stdout);
         if (somo_processed.size() > current_model && somo_processed[current_model]) {
            printf("in calc_grid: somo_processed %d\n", current_model); fflush(stdout);
            editor->append(QString("Gridding bead model %1\n").arg(current_model + 1));
            progress->reset();
            qApp->processEvents();

            if ( !saxs_util->grid ( bead_models[current_model],
                                    bead_model,
                                    dR,
                                    false,
                                    false ) )
            {
               editor_msg( "red", saxs_util->errormsg );
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_equi_grid_bead_model->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               return false;
            }
               
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_equi_grid_bead_model->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               return false;
            }

            for ( unsigned int i = 0; i < ( unsigned int ) bead_model.size(); i++ )
            {
               bead_model[i].exposed_code = 1;
               bead_model[i].bead_color = 8;
               bead_model[i].chain      = 0;
               bead_model[i].visibility = 1;
               bead_model[i].active     = true;
            }

            bead_models[ current_model ] = bead_model;
            
            any_models = true;
            if (somo_processed.size() < current_model + 1) {
               somo_processed.resize(current_model + 1);
            }
            somo_processed[current_model] = 1;
            editor->append( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );

            QString filename =
               project + QString("_%1").arg(current_model + 1);

            le_bead_model_file->setText( filename );

            write_bead_model(somo_dir + SLASH + filename +
                             QString( bead_model_suffix.length() ? ("-" + bead_model_suffix) : "" ) +
                             DOTSOMO, &bead_model);
         }
      }
   }

   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_grid_pdb->setEnabled(true);
      pb_vdw_beads->setEnabled(true);
      pb_somo->setEnabled(true);
      pb_somo_o->setEnabled(true);
      progress->reset();
      return false;
   }

   if (any_models && !any_errors)
   {
      editor->append("Build bead model completed\n");
      qApp->processEvents();
      pb_visualize->setEnabled(true);
      pb_equi_grid_bead_model->setEnabled(true);
      pb_calc_hydro->setEnabled(true);
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_equi_grid_bead_model->setEnabled(true);
   pb_grid_pdb->setEnabled(grid_pdb_state);
   pb_vdw_beads->setEnabled(grid_pdb_state);
   pb_grid->setEnabled(true);
   pb_somo->setEnabled(somo_state);
   pb_somo_o->setEnabled(somo_state);
   pb_stop_calc->setEnabled(false);
   if (calcAutoHydro)
   {
      calc_hydro();
   }
   else
   {
      play_sounds(1);
   }

   return true;
}

int US_Hydrodyn::calc_somo_o()
{
   return calc_somo( true );
}

bool US_Hydrodyn::calc_zeno_hydro()
{
    return (int)(!calc_zeno());
}

void US_Hydrodyn::le_pdb_file_focus( bool )
{
   // us_qdebug( "focused" );
   le_pdb_file_save_text = le_pdb_file->text();
}

void US_Hydrodyn::le_pdb_file_changed( const QString & )
{
   // us_qdebug( "changed" );
   le_pdb_file->setText( le_pdb_file_save_text );
}

void US_Hydrodyn::update_progress( int pos, int total )
{
   // us_qdebug( QString( "update progress %1 %2" ).arg( pos ).arg( total ) );
   progress->setValue( pos ); progress->setMaximum( total );
}

void US_Hydrodyn::do_update_progress( int pos, int total ) {
   // us_qdebug( QString( "do update progress %1 %2" ).arg( pos ).arg( total ) );
   emit progress_updated( pos, total );
}

void US_Hydrodyn::process_events() {
   us_qdebug( QString( "process_events()?" ) );
   //   qApp->processEvents();
}

int US_Hydrodyn::calc_vdw_beads()
{
   {
      int models_selected = 0;
      for(int i = 0; i < lb_model->count(); i++) {
         if (lb_model->item(i)->isSelected()) {
            models_selected++;
         }
      }
      if ( !models_selected ) {
         editor_msg( "black", "No models selected!\n" );
         return -1;
      }
   }

   if ( selected_models_contain( "XHY" ) )
   {
      QMessageBox::warning( this,
                            us_tr( "Selected model contains XHY residue" ),
                            us_tr( 
                               "Can not process models that contain the XHY residue.\n"
                               "These are currently generated only for SAXS/SANS computations\n"
                               )
                            );
      return -1;
   }

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   model_vector = model_vector_as_loaded;
   options_log = "";
   bool any_errors = false;
   bool any_models = false;
   pb_grid->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }


   {
      double vdw_ot_mult = gparams.count( "vdw_ot_mult" ) ? gparams[ "vdw_ot_mult" ].toDouble() : 0;
      double vdw_ot_dpct = gparams.count( "vdw_ot_dpct" ) ? gparams[ "vdw_ot_dpct" ].toDouble() : 0;
      if ( vdw_ot_mult ) {
         if ( vdw_ot_dpct ) {
            bead_model_suffix = QString( "OT%1DP%2-vdw").arg( vdw_ot_mult ).arg( vdw_ot_dpct );
         } else {
            bead_model_suffix = QString( "OT%1-vdw").arg( vdw_ot_mult );
         }
      } else {
         bead_model_suffix = "vdw";
      }
   }
   le_bead_model_suffix->setText( bead_model_suffix );

   if ( !overwrite )
   {
      setSomoGridFile(false);
   }

   somo_processed.resize(lb_model->count());
   bead_models.resize(lb_model->count());

   for ( current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++ ) {
      if ( lb_model->item( current_model )->isSelected() ) {
         us_qdebug( QString( "in calc_vdw_beads: is selected current model %1" ).arg( current_model ) );
         {
            QString error_string;
            // create bead model from atoms
            editor->append( QString( "Building VdW beads for model %1\n" ).arg( current_model + 1 ) );
            qApp->processEvents();
            // somo_processed[current_model] = 0;
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               return -1;
            }
            // compute maximum position for progress
            somo_processed[ current_model ] = 0;
            bead_models[ current_model ].clear( );
            progress->reset();
            progress->setMaximum( model_vector[current_model].molecule.size() );
            progress->setValue( 0 );

            int retval = create_vdw_beads( error_string );

            if ( retval ) {
               editor_msg( "red", "Errors found during the initial creation of beads\n" );
               qApp->processEvents();
               any_errors = true;
            }
         }
      }
   }
   if ( !any_errors )
   {
      editor_msg( "blue", "Build VdW bead model completed\n" );
      qApp->processEvents();
      pb_visualize->setEnabled(true);
      pb_equi_grid_bead_model->setEnabled(true);
      pb_calc_hydro->setEnabled( false );
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   } else {
      editor_msg( "red", "Errors encountered\n" );
   }


   pb_grid_pdb->setEnabled(true);
   pb_vdw_beads->setEnabled(true);
   pb_grid->setEnabled(true);
   pb_somo->setEnabled(true);
   pb_somo_o->setEnabled(true);
   pb_stop_calc->setEnabled(false);
   if ( calcAutoHydro )
   {
      calc_hydro();
   } else {
      play_sounds(1);
   }


   return ( any_errors ? -1 : 0 );
}

// 2 pass, 1st to compute com
// 2nd upon hydration, find vector from bead to com and OT by multiplier
// add global multiplier for OT 

int US_Hydrodyn::create_vdw_beads( QString & error_string, bool quiet ) {
   error_string = "";
   double vdw_ot_mult = gparams.count( "vdw_ot_mult" ) ? gparams[ "vdw_ot_mult" ].toDouble() : 0;
   double vdw_ot_dpct = gparams.count( "vdw_ot_dpct" ) ? gparams[ "vdw_ot_dpct" ].toDouble() : 0;
   double vdw_ot_d = vdw_ot_dpct * 0.01;
   // us_qdebug( QString( "vdw ot mult %1, additional water decrease percent %2" ).arg( vdw_ot_mult ).arg( vdw_ot_dpct ) );
   point com;
   com.axis[ 0 ] = 0;
   com.axis[ 1 ] = 0;
   com.axis[ 2 ] = 0;
   double total_mw = 0e0;
   double hydro_radius = 0e0;

   bool any_wats = false;
   for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++) {
      for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++) {
         PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
         if ( this_atom->resName == "WAT" ) {
            any_wats = true;
            break;
         }
         if ( any_wats ) {
            break;
         }
      }
   }
   if ( any_wats ) {
      vdw_ot_mult = 0;
      editor_msg( "black", us_tr( "Explicit WATs found, hydration disabled\n" ) );
   }
   bool hydrate = !any_wats;

   if ( !quiet ) 
   {
      editor->append("Creating VdW beads from atomic model\n");
      qApp->processEvents();
   }

   bead_model.clear( );

   if ( vdw_ot_mult ) {
      editor_msg( "black", QString( us_tr( "Using OT multiplier %1\nStart CoM calculation.\n" ) ).arg( vdw_ot_mult ) );
      qApp->processEvents();

      hydro_radius = pow( ( 3e0 / ( 4e0 * M_PI ) ) * misc.hydrovol, 1e0 / 3e0 );
      
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++) {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++) {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
            QString res_idx =
               QString("%1|%2")
               .arg(this_atom->name != "OXT" ? this_atom->resName : "OXT" )
               .arg(this_atom->name);
            if ( vdwf.count( res_idx ) ) {
               _vdwf this_vdwf = vdwf[ res_idx ];
               // ? does mw include hydrated weight ?
               total_mw += this_vdwf.mw;
               com.axis[ 0 ] += this_atom->coordinate.axis[ 0 ] * this_vdwf.mw;
               com.axis[ 1 ] += this_atom->coordinate.axis[ 1 ] * this_vdwf.mw;
               com.axis[ 2 ] += this_atom->coordinate.axis[ 2 ] * this_vdwf.mw;
            } else {
               us_qdebug( QString( "not found %1 in vdwf" ).arg( res_idx ) );
            }
         }
      }
      if ( total_mw ) {
         com.axis[ 0 ] /= total_mw;
         com.axis[ 1 ] /= total_mw;
         com.axis[ 2 ] /= total_mw;
         editor_msg( "black", QString( us_tr( "CoM computed as [%1,%2,%3]\n" ).arg( com.axis[ 0 ] ).arg( com.axis[ 1 ] ).arg( com.axis[ 2 ] ) ) );
      } else {
         editor_msg( "red", QString( us_tr( "Error computing CoM, OT turned off.\n" ) ) );
         return -1;
      }         
   }

   int WAT_Tf_used = 0;
   bool use_WAT_Tf =
      gparams.count( "use_WAT_Tf_pdb" ) &&
      gparams[ "use_WAT_Tf_pdb" ] == "true";

   for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++) {
      for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++) {
         PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

         QString res_idx =
            QString("%1|%2")
            .arg(this_atom->name != "OXT" ? this_atom->resName : "OXT" )
            .arg(this_atom->name);
         if ( vdwf.count( res_idx ) ) {
            PDB_atom tmp_atom;
            _vdwf this_vdwf = vdwf[ res_idx ];
            if ( this_atom->resName == "WAT" ) {
               this_vdwf.mw = 0.0;
            }
            tmp_atom.bead_coordinate = this_atom->coordinate;
            if ( hydrate && this_vdwf.w ) {
               double tmp_vol = M_PI * ( 4e0 / 3e0 ) * this_vdwf.r * this_vdwf.r * this_vdwf.r + ( this_vdwf.w * misc.hydrovol );
               tmp_atom.bead_computed_radius = pow( tmp_vol * 3e0 / ( 4e0 * M_PI ), 1e0 / 3e0 );
               double use_vdw_ot_mult = vdw_ot_mult;
               if ( this_vdwf.w > 1 ) {
                  use_vdw_ot_mult -= vdw_ot_mult * vdw_ot_d * ( this_vdwf.w - 1 );
                  if ( use_vdw_ot_mult < 0e0 ) {
                     use_vdw_ot_mult = 0e0;
                  }
               }
               // us_qdebug( QString( "original ot mult %1, waters %2, decreased multiplier %3" ).arg( vdw_ot_mult ).arg( this_vdwf.w ).arg( use_vdw_ot_mult ) );
               if ( use_vdw_ot_mult ) {
                  tmp_atom.bead_coordinate = saxs_util->plus( tmp_atom.bead_coordinate, saxs_util->scale( saxs_util->normal( saxs_util->minus( this_atom->coordinate, com ) ), use_vdw_ot_mult * hydro_radius ) );
               }
            } else {
               tmp_atom.bead_computed_radius = this_vdwf.r;
               if (
                   use_WAT_Tf &&
                   this_atom->resName == "WAT" &&
                   this_atom->tempFactor ) {
                  // QTextStream( stderr ) <<
                  //    QString( "create_vdw_beads WAT Tf %1\n" ).arg( this_atom->tempFactor )
                  //    ;
                  tmp_atom.bead_computed_radius = this_atom->tempFactor;
                  WAT_Tf_used++;
               }
            }
            // us_qdebug( QString( "bead model radius for %1 = %2" ).arg( res_idx ).arg(  tmp_atom.bead_computed_radius ) );
            tmp_atom.bead_ref_mw = this_vdwf.mw;
            tmp_atom.mw = this_vdwf.mw;
            tmp_atom.bead_mw = this_vdwf.mw;
            tmp_atom.bead_color = this_vdwf.color;
            tmp_atom.exposed_code = 1;
            tmp_atom.all_beads.clear( );
            tmp_atom.active = true;
            tmp_atom.name = this_atom->name;
            tmp_atom.resName = this_atom->resName;
            tmp_atom.iCode = this_atom->iCode;
            tmp_atom.chainID = this_atom->chainID;
            tmp_atom.saxs_data.saxs_name = "";
            tmp_atom.bead_model_code = QString( "%1.%2.%3.%4.%5" )
               .arg( this_atom->serial )
               .arg( this_atom->name )
               .arg( this_atom->resName )
               .arg( this_atom->chainID )
               .arg( this_atom->resSeq )
               ;
            
            bead_model.push_back(tmp_atom);
         } else {
            editor_msg( "red", QString( "Residue atom pair %1 unknown in vdwf.json" ).arg( res_idx ) );
            return -1;
         }
      }
   }      

   if ( WAT_Tf_used ) {
      editor_msg( "dark blue", QString( us_tr( "Notice: %1 WATs using PDB's Tf radius recognized\n" ) ).arg( WAT_Tf_used ) );
   }

   bead_models[ current_model ] = bead_model;
   somo_processed[ current_model ] = 1;
   
   write_bead_model( get_somo_dir() + SLASH + project + fix_file_name( QString("_%1").arg( model_name( current_model ) ) ) +
                     QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
                     , &bead_model);
   
   editor->append( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );
   return 0;
}

bool US_Hydrodyn::load_vdwf_json( QString filename ) {
   QFile f( filename );
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      return false;
   }
   QString qs;
   QTextStream ts( &f );
   while ( !ts.atEnd() )
   {
      qs += ts.readLine();
   }
   f.close();
   vdwf.clear( );
   map < QString, QString > parameters = US_Json::split( qs );
   _vdwf this_vdwf;
   for ( map < QString, QString >::iterator it = parameters.begin();
         it != parameters.end();
         it++ )
   {
      // us_qdebug( QString( "vdwf read %1 %2" ).arg( it->first ).arg( it->second ) );
      map < QString, QString > tmp_param = US_Json::split( "{" + it->second + "}" );
      if ( !tmp_param.count( "mw" ) ||
           !tmp_param.count( "r" ) ||
           !tmp_param.count( "w" ) ) {
         us_qdebug( QString( "vdwf missing mw r or w %1 %2" ).arg( it->first ).arg( it->second ) );
      } else {
         this_vdwf.mw = tmp_param[ "mw" ].toDouble();
         this_vdwf.r  = tmp_param[ "r" ].toDouble();
         this_vdwf.w  = tmp_param[ "w" ].toDouble();
         vdwf[ it->first ] = this_vdwf;
      }
   }
   return false;
}

bool US_Hydrodyn::calc_hullrad_hydro( QString filename ) {
   hullrad_filename = filename.isEmpty() ? last_pdb_filename : filename;
   
   editor_msg( "black", QString( "Hullrad %1" ).arg( hullrad_filename ) );
   // us_qdebug( QString( "calc_hullrad_hydro %1" ).arg( hullrad_filename ) );

   hullrad_prog = 
      USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + "HullRadV2.py" 
         ;

   QFileInfo qfi( hullrad_prog );
   if ( !qfi.exists() ) {
         editor_msg( (QString) "red", QString("Hullrad program '%1' does not exist\n").arg(hullrad_prog));
         return false;
   }

   // make tmp pdb, run hullrad

   QFile f( hullrad_filename );

   if ( !f.exists() ) {
      editor_msg( "red", QString( "PDB file %1 not found" ).arg( hullrad_filename ) );
      return false;
   }
   
   if ( !f.open( QIODevice::ReadOnly ) ) {
      QMessageBox::warning( this,
                            us_tr("Could not open file"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( f.fileName() )
                            );
      return false;
   }

   // make unique tmp dir
   QString hullrad_tmp_path = somo_dir + "/tmp/" + QDateTime::currentDateTime().toString( "yyMMddhhmmsszzz" );
   while ( QDir( hullrad_tmp_path ).exists() ) {
      US_Saxs_Util::us_usleep( 1000 );
      hullrad_tmp_path = somo_dir + "/tmp/" + QDateTime::currentDateTime().toString( "yyMMddhhmmsszzz" );
   }

   {
      QDir qd;
      if ( !qd.mkdir( hullrad_tmp_path ) ) {
         editor_msg( "red", QString( us_tr("Could not create directory %1, check permissions" ) ).arg( hullrad_tmp_path ) );
         return false;
      }
   }

   QDir::setCurrent( hullrad_tmp_path );

   // open pdb, split, save filenames

   QRegExp rx_model("^MODEL");
   QRegExp rx_end("^END");
   QRegExp rx_save_header("^("
                          "HEADER|"
                          "TITLE|"
                          "COMPND|"
                          "SOURCE|"
                          "KEYWDS|"
                          "AUTHOR|"
                          "REVDAT|"
                          "JRNL|"
                          "REMARK|"
                          "SEQRES|"
                          "SHEET|"
                          "HELIX|"
                          "SSBOND|"
                          "DBREF|"
                          "ORIGX|"
                          "SCALE"
                          ")\\.*" );
   
   unsigned int model_count = 0;

   editor_msg( "dark blue", QString( us_tr( "Checking file %1" ).arg( f.fileName() ) ) );

   map    < QString, bool > model_names;
   vector < QString >       model_name_vector;
   unsigned int             max_model_name_len      = 0;
   QString                  model_header;
   bool                     dup_model_name_msg_done = false;
   unsigned int             end_count               = 0;
   bool                     found_model             = false;
   
   {
      QTextStream ts( &f );
      unsigned int line_count = 0;
   
      while ( !ts.atEnd() ) {
         QString qs = ts.readLine();
         line_count++;
         if ( line_count && !(line_count % 100000 ) )
         {
            editor_msg( "dark blue", QString( us_tr( "Lines read %1" ).arg( line_count ) ) );
            qApp->processEvents();
         }
         if ( !found_model && qs.contains( rx_save_header ) )
         {
            model_header += qs + "\n";
         }
         
         if ( qs.contains( rx_end ) )
         {
            end_count++;
         }

         if ( qs.contains( rx_model ) )
         {
            found_model = true;
            model_count++;
            // QStringList qsl = (qs.left(20).split( QRegExp("\\s+") , QString::SkipEmptyParts ) );
            QStringList qsl;
            {
               QString qs2 = qs.left( 20 );
               qsl = (qs2 ).split( QRegExp("\\s+") , QString::SkipEmptyParts );
            }
            QString model_name;
            if ( qsl.size() == 1 )
            {
               model_name = QString("%1").arg( model_count );
            } else {
               model_name = qsl[1];
            }
            if ( model_names.count( model_name ) )
            {
               unsigned int mext = 1;
               QString use_model_name;
               do {
                  use_model_name = model_name + QString("-%1").arg( mext );
               } while ( model_names.count( use_model_name ) );
               model_name = use_model_name;
               if ( !dup_model_name_msg_done )
               {
                  dup_model_name_msg_done = true;
                  editor_msg( "red", us_tr( "Duplicate or missing model names found, -# extensions added" ) );
               }
            }
            model_names[ model_name ] = true;
            model_name_vector.push_back ( model_name );
            if ( (unsigned int) model_name.length() > max_model_name_len )
            {
               max_model_name_len = model_name.length();
            }
         }
      }
   }

   f.close();

   bool no_model_directives = false;

   if ( model_count == 0 )
   {
      if ( end_count > 1 )
      {
         no_model_directives = true;
         model_count = end_count;
         for ( unsigned int i = 0; i < end_count; i++ )
         {
            QString model_name = QString("%1").arg( i + 1 );
            model_names[ model_name ] = true;
            model_name_vector.push_back ( model_name );
            if ( (unsigned int) model_name.length() > max_model_name_len )
            {
               max_model_name_len = model_name.length();
            }
         }
      } else {
         model_count = 1;
      }
   }

   hullrad_to_process.clear( );
   hullrad_processed.clear( );
   hullrad_captures.clear( );

   if ( model_count == 1 ) {
      hullrad_to_process << hullrad_filename;
      hullrad_running = true;
      hullrad_process_next();
      return true;
   }

   // push stack of pdbs 

   int res = 1; // every model
   
   QString ext = "X";
   while ( (unsigned int) ext.length() < max_model_name_len )
   {
      ext = "X" + ext;
   }
   ext = "-" + ext + ".pdb";

   QString fn = hullrad_tmp_path + "/" + QFileInfo( f ).fileName().replace(QRegExp("\\.(pdb|PDB)$"),"") + ext;

   fn.replace(QRegExp(QString("%1$").arg(ext)), "" );

   if ( !f.open( QIODevice::ReadOnly ) ) {
      QMessageBox::warning( this,
                            us_tr("Could not open file"),
                            QString("An error occured when trying to open file\n"
                                    "%1\n"
                                    "Please check the permissions and try again\n")
                            .arg( f.fileName() )
                            );
      return false;
   }

   QTextStream ts( &f );

   QString       model_lines;
   bool          in_model = no_model_directives;
   unsigned int  pos = 0;

   if ( !ts.atEnd() ) {
      do {
         QString qs = ts.readLine();
         if ( qs.contains( rx_model ) || qs.contains( rx_end ) || ts.atEnd() ) {
            if ( model_lines.length() ) {
               if ( !( pos % res ) ) {
                  QString use_ext = model_name_vector[ pos ];
                  while ( (unsigned int) use_ext.length() < max_model_name_len ) {
                     use_ext = "0" + use_ext;
                  }
                  
                  QString use_fn = fn + "-" + use_ext + ".pdb";
                  
                  QFile fn_out( use_fn );
                  
                  if ( !fn_out.open( QIODevice::WriteOnly ) ) {
                     QMessageBox::warning( this, "US-SOMO: PDB Editor : Split",
                                           QString(us_tr("Could not open %1 for writing!")).arg( use_fn ) );
                     return false;
                  }
                  
                  QTextStream tso( &fn_out );
               
                  tso << QString("HEADER    split from %1: Model %2 of %3\n").arg( f.fileName() ).arg( pos + 1 ).arg( model_count );
                  tso << model_header;
                  tso << QString("").sprintf("MODEL  %7s\n", model_name_vector[ pos ].toLatin1().data() );
                  tso << model_lines;
                  tso << "ENDMDL\nEND\n";
                  
                  fn_out.close();
                  hullrad_to_process << fn_out.fileName();
                  editor_msg( "dark blue", QString( us_tr( "File %1 written" ) ).arg( fn_out.fileName() ) );
                  qApp->processEvents();
               } else {
                  // editor_msg( "dark red", QString("model %1 skipped").arg( model_name_vector[ pos ] ) );
               }
               in_model = false;
               model_lines = "";
               pos++;
            }
            if ( qs.contains( rx_model ) ||
                 ( no_model_directives && qs.contains( rx_end ) ) )
            {
               in_model = true;
               model_lines = "";
            }
         } else {
            if ( in_model )
            {
               model_lines += qs + "\n";
            }
         }
      } while ( !ts.atEnd() );
   }
   f.close();

   hullrad_running = true;
   hullrad_process_next();

   // summarize results into csv, hydrodyn results
   return true;
}

void US_Hydrodyn::hullrad_process_next() {
   
   // us_qdebug( QString( "hullrad_process_next %1" ).arg( hullrad_filename ) );

   if ( !hullrad_to_process.size() ) {
      hullrad_finalize();
      return;
   }

   hullrad_last_processed = hullrad_to_process[ 0 ];
   hullrad_to_process.pop_front();
   hullrad_processed.push_back( hullrad_last_processed ); 

   hullrad_stdout = "";

   hullrad = new QProcess( this );
   //   hullrad->setWorkingDirectory( dir );
   // us_qdebug( "prog is " + hullrad_prog );
#if QT_VERSION < 0x040000
   hullrad->addArgument( hullrad_prog );
   hullrad->addArgument( hullrad_last_processed );

   connect( hullrad, SIGNAL(readyReadStandardOutput()), this, SLOT(hullrad_readFromStdout()) );
   connect( hullrad, SIGNAL(readyReadStandardError()), this, SLOT(hullrad_readFromStderr()) );
   connect( hullrad, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(hullrad_finished( int, QProcess::ExitStatus )) );
   connect( hullrad, SIGNAL(started()), this, SLOT(hullrad_started()) );

   editor_msg( "black", "\nStarting Hullrad\n");
   hullrad->start();
#else
   {
      QStringList args;
      args << hullrad_last_processed;

      connect( hullrad, SIGNAL(readyReadStandardOutput()), this, SLOT(hullrad_readFromStdout()) );
      connect( hullrad, SIGNAL(readyReadStandardError()), this, SLOT(hullrad_readFromStderr()) );
      connect( hullrad, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(hullrad_finished( int, QProcess::ExitStatus )) );
      connect( hullrad, SIGNAL(started()), this, SLOT(hullrad_started()) );

      editor_msg( "black", "\nStarting Hullrad\n");
      hullrad->start( hullrad_prog, args, QIODevice::ReadOnly );
   }
#endif
   
   return;
}

void US_Hydrodyn::hullrad_readFromStdout()
{
   // us_qdebug( QString( "hullrad_readFromStdout %1" ).arg( hullrad_filename ) );
#if QT_VERSION < 0x040000
   while ( hullrad->canReadLineStdout() )
   {
      QString qs = hullrad->readLineStdout() + "\n";
      hullrad_stdout += qs;
      editor_msg("brown", qs );
   }
#else
   QString qs = QString( hullrad->readAllStandardOutput() );
   hullrad_stdout += qs;
   editor_msg( "brown", qs );
#endif   
   //  qApp->processEvents();
}
   
void US_Hydrodyn::hullrad_readFromStderr()
{
   // us_qdebug( QString( "hullrad_readFromStderr %1" ).arg( hullrad_filename ) );

#if QT_VERSION < 0x040000
   while ( hullrad->canReadLineStderr() )
   {
      editor_msg("red", hullrad->readLineStderr() + "\n");
   }
#else
   editor_msg( "red", QString( hullrad->readAllStandardError() ) );
#endif   
   //  qApp->processEvents();
}
   
void US_Hydrodyn::hullrad_finished( int, QProcess::ExitStatus )
{
   // us_qdebug( QString( "hullrad_processExited %1" ).arg( hullrad_filename ) );
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   hullrad_readFromStderr();
   hullrad_readFromStdout();
      //   }
   disconnect( hullrad, SIGNAL(readyReadStandardOutput()), 0, 0);
   disconnect( hullrad, SIGNAL(readyReadStandardError()), 0, 0);
   disconnect( hullrad, SIGNAL(finished( int, QProcess::ExitStatus )), 0, 0);
   editor->append("HULLRAD finished.\n");

   // post process the files

   QStringList caps;
   caps
      // M               :        14315     g/mol
      // v_bar           :        0.718     mL/g
      // R(Anhydrous)    :        15.98     Angstroms
      // Axial Ratio     :         1.49
      // f/fo            :         1.16
      // Dt              :       1.16e-06   cm^2/s
      // R(Translation)  :        18.48     Angstroms
      // s               :       1.93e-13   sec
      // [eta]           :         2.78     cm^3/g
      // Dr              :       1.99e+07   s^-1
      // R(Rotation)     :        20.06     Angstroms

      << "M"
      << "v_bar"
      << "R\\(Anhydrous\\)"
      << "Axial Ratio"
      << "f/fo"
      << "Dt"
      << "R\\(Translation\\)"
      << "s"
      << "\\[eta\\]"
      << "Dr"
      << "R\\(Rotation\\)"
      ;

   map < QString, double > captures;

   for ( int i = 0; i < (int) caps.size(); ++i ) {
      QRegExp rx( caps[ i ] + "\\s+:\\s*(\\S+)" );

      if ( rx.indexIn( hullrad_stdout ) == -1 ) {
         editor_msg( "red", QString( us_tr( "Could not find %1 file in HULLRAD output" ) ).arg( caps[ i ].replace( "\\", "" ) ) );
         hullrad_captures[ caps[ i ] ].push_back( -9e99 );
      } else {
         hullrad_captures[ caps[ i ] ].push_back( rx.cap( 1 ).toDouble() );
         us_qdebug( QString( "%1 : '%2'\n" ).arg( caps[ i ] ).arg( hullrad_captures[ caps[ i ] ].back() ) );
      }
   }

   // accumulate data as in zeno (e.g. push values to data structures
   
   hullrad_process_next();
}
   
void US_Hydrodyn::hullrad_started()
{
   // us_qdebug( QString( "hullrad_started %1" ).arg( hullrad_filename ) );
   editor_msg("brown", "HULLRAD launch exited\n");
   disconnect( hullrad, SIGNAL(started()), 0, 0);
}

void US_Hydrodyn::hullrad_finalize() {
   // us_qdebug( QString( "hullrad_finalize %1" ).arg( hullrad_filename ) );
   editor_msg( "dark red", "Finalizing results" );
   for ( map < QString, vector < double > >::iterator it = hullrad_captures.begin();
         it != hullrad_captures.end();
         ++it ) {
      editor_msg( "dark red",  US_Vector::qs_vector( it->first, it->second ) );
   }

   hydro_results hullrad_results;
   hydro_results hullrad_results2;

   hullrad_results.method                = "Hullrad";
   hullrad_results.mass                  = 0e0;
   hullrad_results.s20w                  = 0e0;
   hullrad_results.s20w_sd               = 0e0;
   hullrad_results.D20w                  = 0e0;
   hullrad_results.D20w_sd               = 0e0;
   hullrad_results.viscosity             = 0e0;
   hullrad_results.viscosity_sd          = 0e0;
   hullrad_results.rs                    = 0e0;
   hullrad_results.rs_sd                 = 0e0;
   hullrad_results.rg                    = 0e0;
   hullrad_results.rg_sd                 = 0e0;
   hullrad_results.tau                   = 0e0;
   hullrad_results.tau_sd                = 0e0;
   hullrad_results.vbar                  = 0e0;
   hullrad_results.asa_rg_pos            = 0e0;
   hullrad_results.asa_rg_neg            = 0e0;
   hullrad_results.ff0                   = 0e0;
   hullrad_results.ff0_sd                = 0e0;

   hullrad_results.solvent_name          = hydro.solvent_name;
   hullrad_results.solvent_acronym       = hydro.solvent_acronym;
   hullrad_results.solvent_viscosity     = hydro.solvent_viscosity;
   hullrad_results.solvent_density       = hydro.solvent_density;
   hullrad_results.temperature           = hydro.temperature;
   hullrad_results.name                  = project;
   hullrad_results.used_beads            = 0;
   hullrad_results.used_beads_sd         = 0e0;
   hullrad_results.total_beads           = 0;
   hullrad_results.total_beads_sd        = 0e0;
   hullrad_results.vbar                  = 0;

   hullrad_results.num_models            = hullrad_processed.size();

   hullrad_results2 = hullrad_results;

   map < int, map < QString, double > > data_to_save;
   
   for ( map < QString, vector < double > >::iterator it = hullrad_captures.begin();
         it != hullrad_captures.end();
         ++it ) {

      for ( int i = 0; i < (int) it->second.size(); ++i ) {
         data_to_save[ i ][ it->first ] = it->second[ i ];

         if ( it->first  == "M" ) {
            {
               hullrad_results.mass += it->second[ i ];
               // hullrad_results2.mass += it->second[ i ] * it->second[ i ];
            }
            break;
         }               
         if ( it->first == "v_bar" ) {
            hullrad_results.vbar += it->second[ i ];
            // hullrad_results2.vbar += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "R\\(Anhydrous\\)" ) {
            hullrad_results.rs += it->second[ i ];
            hullrad_results2.rs += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "Axial Ratio" ) {
            //    hullrad_results.mass += it->second[ i ];
            //    hullrad_results2.mass += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "f/fo" ) {
            hullrad_results.ff0 += it->second[ i ];
            hullrad_results2.ff0 += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "Dt" ) {
            hullrad_results.D20w += it->second[ i ];
            hullrad_results2.D20w += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "R\\(Translation\\)" ) {
            //    hullrad_results.mass += it->second[ i ];
            //    hullrad_results2.mass += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "s" ) {
            hullrad_results.s20w += it->second[ i ] * 1e13;
            hullrad_results2.s20w += it->second[ i ] * it->second[ i ] * 1e26;
         }

         if ( it->first == "\\[eta\\]" ) {
            hullrad_results.viscosity += it->second[ i ];
            hullrad_results2.viscosity += it->second[ i ] * it->second[ i ];
         }

         if ( it->first == "Dr" ) {
            //    hullrad_results.mass += it->second[ i ];
            //    hullrad_results2.mass += it->second[ i ] * it->second[ i ];
         }
         if ( it->first == "R\\(Rotation\\)" ) {
            //    hullrad_results.mass += it->second[ i ];
            //    hullrad_results2.mass += it->second[ i ] * it->second[ i ];
         }
      }
   }
   
   for ( map < int, map < QString, double > >::iterator it = data_to_save.begin();
         it != data_to_save.end();
         ++it ) {
      save_data this_data;

      this_data.tot_surf_area                 = 0e0;
      this_data.tot_volume_of                 = 0e0;
      this_data.num_of_unused                 = 0e0;
      this_data.use_beads_vol                 = 0e0;
      this_data.use_beads_surf                = 0e0;
      this_data.use_bead_mass                 = 0e0;
      this_data.con_factor                    = 0e0;
      this_data.tra_fric_coef                 = 0e0;
      this_data.rot_fric_coef                 = 0e0;
      this_data.rot_diff_coef                 = 0e0;
      this_data.rot_fric_coef_x               = 0e0;
      this_data.rot_fric_coef_y               = 0e0;
      this_data.rot_fric_coef_z               = 0e0;
      this_data.rot_diff_coef_x               = 0e0;
      this_data.rot_diff_coef_y               = 0e0;
      this_data.rot_diff_coef_z               = 0e0;
      this_data.rot_stokes_rad_x              = 0e0;
      this_data.rot_stokes_rad_y              = 0e0;
      this_data.rot_stokes_rad_z              = 0e0;
      this_data.cen_of_res_x                  = 0e0;
      this_data.cen_of_res_y                  = 0e0;
      this_data.cen_of_res_z                  = 0e0;
      this_data.cen_of_mass_x                 = 0e0;
      this_data.cen_of_mass_y                 = 0e0;
      this_data.cen_of_mass_z                 = 0e0;
      this_data.cen_of_diff_x                 = 0e0;
      this_data.cen_of_diff_y                 = 0e0;
      this_data.cen_of_diff_z                 = 0e0;
      this_data.cen_of_visc_x                 = 0e0;
      this_data.cen_of_visc_y                 = 0e0;
      this_data.cen_of_visc_z                 = 0e0;
      this_data.unc_int_visc                  = 0e0;
      this_data.unc_einst_rad                 = 0e0;
      this_data.cor_int_visc                  = 0e0;
      this_data.cor_einst_rad                 = 0e0;
      this_data.rel_times_tau_1               = 0e0;
      this_data.rel_times_tau_2               = 0e0;
      this_data.rel_times_tau_3               = 0e0;
      this_data.rel_times_tau_4               = 0e0;
      this_data.rel_times_tau_5               = 0e0;
      this_data.rel_times_tau_m               = 0e0;
      this_data.rel_times_tau_h               = 0e0;
      this_data.max_ext_x                     = 0e0;
      this_data.max_ext_y                     = 0e0;
      this_data.max_ext_z                     = 0e0;
      this_data.axi_ratios_xz                 = 0e0;
      this_data.axi_ratios_xy                 = 0e0;
      this_data.axi_ratios_yz                 = 0e0;
      this_data.results.method                = "Hullrad";
      this_data.results.mass                  = 0e0;
      this_data.results.s20w                  = 0e0;
      this_data.results.s20w_sd               = 0e0;
      this_data.results.D20w                  = 0e0;
      this_data.results.D20w_sd               = 0e0;
      this_data.results.viscosity             = 0e0;
      this_data.results.viscosity_sd          = 0e0;
      this_data.results.rs                    = 0e0;
      this_data.results.rs_sd                 = 0e0;
      this_data.results.rg                    = 0e0;
      this_data.results.rg_sd                 = 0e0;
      this_data.results.tau                   = 0e0;
      this_data.results.tau_sd                = 0e0;
      this_data.results.vbar                  = 0e0;
      this_data.results.asa_rg_pos            = 0e0;
      this_data.results.asa_rg_neg            = 0e0;
      this_data.results.ff0                   = 0e0;
      this_data.results.ff0_sd                = 0e0;
      this_data.results.solvent_name          = "";
      this_data.results.solvent_acronym       = "";
      this_data.results.solvent_viscosity     = 0e0;
      this_data.results.solvent_density       = 0e0;

      this_data.hydro                         = hydro;
      this_data.results.num_models            = 1;
      this_data.results.name                  = QString( "%1-%1" ).arg( QFileInfo( hullrad_filename ).completeBaseName() ).arg( it->first + 1 );
      this_data.results.used_beads            = 0;
      this_data.results.used_beads_sd         = 0e0;
      this_data.results.total_beads           = 0;
      this_data.results.total_beads_sd        = 0e0;
      this_data.results.vbar                  = 0;

      if ( it->second.count( "M" ) ) {
         this_data.results.mass = it->second[ "M" ];
      }

      if ( it->second.count( "v_bar" ) ) {
         this_data.results.vbar = it->second[ "v_bar" ];
      }

      if ( it->second.count( "R\\(Anhydrous\\)" ) ) {
         this_data.results.rs = it->second[ "R\\(Anhydrous\\)" ];
      }

      if ( it->second.count( "Axial Ratio" ) ) {
         // this_data.results.mass = it->second[ "Axial Ratio" ];
      }

      if ( it->second.count( "f/fo" ) ) {
         this_data.results.ff0 = it->second[ "f/fo" ];
      }

      if ( it->second.count( "Dt" ) ) {
         this_data.results.D20w = it->second[ "Dt" ];
      }

      if ( it->second.count( "R\\(Translation\\)" ) ) {
         // this_data.results.mass = it->second[ "R\\(Translation\\)" ];
      }

      if ( it->second.count( "s" ) ) {
         this_data.results.s20w = it->second[ "s" ] * 1e13;
      }

      if ( it->second.count( "\\[eta\\]" ) ) {
         this_data.results.viscosity = it->second[ "\\[eta\\]" ];
      }

      if ( it->second.count( "Dr" ) ) {
         // this_data.results.mass = it->second[ "Dr" ];
      }

      if ( it->second.count( "R\\(Rotation\\)" ) ) {
         // this_data.results.mass = it->second[ "R\\(Rotation\\)" ];
      }
      
      if ( batch_widget &&
           batch_window->save_batch_active )
      {
         save_params.data_vector.push_back( this_data );
      }

      bool create_hydro_res = !(batch_widget &&
                                batch_window->save_batch_active);

      if ( saveParams && create_hydro_res )
      {
         QString fname = somo_dir + "/" + this_data.results.name + ".hullrad.csv";
         FILE *of = us_fopen(fname, "wb");
         if ( of )
         {
            fprintf(of, "%s", save_util->header().toLatin1().data());

            fprintf(of, "%s", save_util->dataString(&this_data).toLatin1().data());
            fclose(of);
            editor_msg( "dark blue", QString( "created %1\n" ).arg( fname ) );
         }
      }
      // print out results:
      save_util->header();
      save_util->dataString(&this_data);
   }

   {
      double num = (double) hullrad_results.num_models;
      if ( num <= 1 ) {
         results = hullrad_results;
      } else {
         hullrad_results.name = QFileInfo( hullrad_filename ).baseName();
         double numinv = 1e0 / num;
         hullrad_results.mass          *= numinv;
         hullrad_results.s20w          *= numinv;
         hullrad_results.D20w          *= numinv;
         hullrad_results.viscosity     *= numinv;
         hullrad_results.rs            *= numinv;
         hullrad_results.rg            *= numinv;
         hullrad_results.vbar          *= numinv;
         hullrad_results.ff0           *= numinv;
         hullrad_results.used_beads    *= numinv;
         hullrad_results.total_beads   *= numinv;
         if ( num <= 1 ) {
            results = hullrad_results;
         } else {
            double numdecinv = 1e0 / ( num - 1e0 );
         
            hullrad_results.s20w_sd           = sqrt( fabs( ( hullrad_results2.s20w        - hullrad_results.s20w        * hullrad_results.s20w        * num ) * numdecinv ) );
            hullrad_results.D20w_sd           = sqrt( fabs( ( hullrad_results2.D20w        - hullrad_results.D20w        * hullrad_results.D20w        * num ) * numdecinv ) );
            hullrad_results.viscosity_sd      = sqrt( fabs( ( hullrad_results2.viscosity   - hullrad_results.viscosity   * hullrad_results.viscosity   * num ) * numdecinv ) );
            hullrad_results.rs_sd             = sqrt( fabs( ( hullrad_results2.rs          - hullrad_results.rs          * hullrad_results.rs          * num ) * numdecinv ) );
            hullrad_results.rg_sd             = sqrt( fabs( ( hullrad_results2.rg          - hullrad_results.rg          * hullrad_results.rg          * num ) * numdecinv ) );
            hullrad_results.ff0_sd            = sqrt( fabs( ( hullrad_results2.ff0         - hullrad_results.ff0         * hullrad_results.ff0         * num ) * numdecinv ) );
            hullrad_results.used_beads_sd     = sqrt( fabs( ( hullrad_results2.used_beads  - hullrad_results.used_beads  * hullrad_results.used_beads  * num ) * numdecinv ) );
            hullrad_results.total_beads_sd    = sqrt( fabs( ( hullrad_results2.total_beads - hullrad_results.total_beads * hullrad_results.total_beads * num ) * numdecinv ) );
            
            results = hullrad_results;
         }
      }
   }

   pb_show_hydro_results->setEnabled( true );
   hullrad_running = false;
   // us_qdebug( QString( "hullrad_finalize %1 end" ).arg( hullrad_filename ) );
}

void US_Hydrodyn::create_fasta_vbar_mw() {

#if defined( DEBUG_FASTA_SEQ )
   qDebug() << "create_fasta_vbar_mw";
   for ( map < QString, double >::iterator it = res_vbar.begin();
         it != res_vbar.end();
         ++it ) {
      qDebug() << "res_vbar[" << it->first << "] = " << it->second;
      qDebug() << "res_mw[" << it->first << "] = " << res_mw[ it->first ];
   }

#endif

   fasta_vbar.clear();
   fasta_mw.clear();
   
   for ( map < QString, QChar >::iterator it = residue_short_names.begin();
         it != residue_short_names.end();
         ++it ) {

      if ( it->first != "WAT" &&
           res_vbar.count( it->first ) &&
           res_mw.count( it->first ) 
           ) {
         fasta_vbar[ it->second ] = res_vbar[ it->first ];
         fasta_mw  [ it->second ] = res_mw  [ it->first ];
      }
   }

#if defined( DEBUG_FASTA_SEQ )
   qDebug() << "create_fasta_vbar_mw";
   for ( map < QString, double >::iterator it = fasta_vbar.begin();
         it != fasta_vbar.end();
         ++it ) {
      qDebug() << "fasta_vbar[" << it->first << "] = " << it->second;
      qDebug() << "fasta_mw[" << it->first << "] = " << fasta_mw[ it->first ];
   }

#endif
   
}

bool US_Hydrodyn::calc_fasta_vbar( QStringList & seq_chars, double & result, QString & msgs ) {
   result = 0e0;
   msgs = "";
   if ( ! seq_chars.size() ) {
      msgs = "Error: The sequence is empty!";
      return false;
   }
      
   double mw_vbar_sum = 0.0;
   double mw_sum = 0.0;
   double mw;

   map < QString, int > replaced;

   for ( int i = 0; i < (int) seq_chars.size(); ++i ) {
      QString se = seq_chars[ i ];
      if ( !fasta_mw.count( se ) ) {
         replaced[ se ]++;
         se = "A";
      }
      if ( !fasta_mw.count( se ) ) {
         msgs = "Error: No A defined in sequence database";
         return false;
      }
         
      mw_sum      += fasta_mw[ se ];
      mw_vbar_sum += fasta_mw[ se ] * fasta_vbar[ se ];
   }

   if ( replaced.size() ) {
      msgs = "Notice:\n";
      for ( map < QString, int >::iterator it = replaced.begin();
            it != replaced.end();
            ++it ) {
         msgs += QString( us_tr( "Unrecognized code '%1' replaced by 'A' %2 time(s)\n" ) )
            .arg( it->first )
            .arg( it->second )
            ;
      }
   }
   if ( ! seq_chars.size() ) {
      msgs = "Error: The sequence sum results is zero molecular weight.";
      return false;
   }

   result = (double)floor(0.5 + ( ( mw_vbar_sum / mw_sum ) * 1000e0 ) ) / 1000e0;
   // qDebug() << "calc_fasta_vbar result for seq of " << seq_chars.size() << " elements computed as " << result;
   return true;
}

QString US_Hydrodyn::get_somo_dir() {
   if ( !batch_widget ||
        !batch.results_dir ||
        batch.results_dir_name.isEmpty() ) {
      return somo_dir;
   }

   //  check && create if needed
   QString dir_path = somo_dir + QDir::separator() + batch.results_dir_name;
   // QTextStream( stderr ) << "somo results dir is '" + dir_path + "'\n";

   QDir dir( dir_path );
   if ( !dir.exists() ) {
      dir.mkdir( dir_path );
   }
   if ( dir.exists() ) {
      return dir_path;
   }
   batch_window->editor_msg( "red"
                             ,QString( us_tr( "WARNING: results directory %1 could not be created, using %2" ) )
                             .arg( dir.path() )
                             .arg( somo_dir )
                             );
   return somo_dir;
}
