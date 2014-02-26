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
#include <Q3BoxLayout>
#include <QLabel>
#include <Q3GridLayout>
#include <Q3TextStream>
#include <Q3HBoxLayout>
#include <Q3Frame>
#include <Q3PopupMenu>
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

// #define USE_H

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

// #define GRID_HYDRATE_DEBUG

// static bool no_rr;

US_Hydrodyn::US_Hydrodyn(vector < QString > batch_file,
                         QWidget *p, 
                         const char *name) : Q3Frame(p, name)
{
   USglobal = new US_Config();
   this->batch_file = batch_file;
   numThreads = USglobal->config_list.numThreads;
   extra_saxs_coefficients.clear();

   last_pdb_filename = "";
   last_pdb_title.clear();
   last_pdb_header.clear();

   // no_rr = false;

   // int r_stdout = __open(QString(somo_tmp_dir +
   //           SLASH + "last_stdout.txt").ascii(),
   //        O_WRONLY | O_CREAT | O_TRUNC, 0666);
   // dup2(r_stdout, STDOUT_FILENO);

   // int r_stderr = __open(QString(somo_tmp_dir +
   //           SLASH + "last_stderr.txt").ascii(),
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
   setCaption(tr("SOMO Solution Bead Modeler"));
   advanced_config.auto_view_pdb = true;
   advanced_config.scroll_editor = false;
   advanced_config.auto_calc_somo = false;
   advanced_config.auto_show_hydro = false;
   advanced_config.pbr_broken_logic = true;
   advanced_config.use_sounds = false;
   advanced_config.expert_mode = false;
   advanced_config.experimental_threads = false;
   advanced_config.debug_1 = false;
   advanced_config.debug_2 = false;
   advanced_config.debug_3 = false;
   advanced_config.debug_4 = false;
   advanced_config.debug_5 = false;
   advanced_config.debug_6 = false;
   advanced_config.debug_7 = false;
   if ( !install_new_version() )
   {
      exit(0);
   }
   set_default();   // setup configuration defaults before reading initial config
   read_config(""); // specify default configuration by leaving argument empty
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
   calcAutoHydro = false;
   setSuffix = false;
   overwrite = false;
   saveParams = false;
   guiFlag = true;
   bead_model_selected_filter = "";
   residue_filename = USglobal->config_list.system_dir + "/etc/somo.residue";
   editor = (Q3TextEdit *)0;

   last_saxs_search_csv.name = "__empty__";
   last_saxs_screen_csv.name = "__empty__";
   last_saxs_buffer_csv.name = "__empty__";
   last_saxs_hplc_csv.name = "__empty__";

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
   chdir(somo_tmp_dir);
   if ( advanced_config.debug_5 )
   {
      printf("%s\n", QString(somo_tmp_dir).ascii());
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

   options_log = "";
   last_abb_msgs = "";

   comparative = US_Hydrodyn_Comparative::empty_comparative_info();

   rasmol = new Q3Process(this);
   rasmol->setWorkingDirectory(
                               QDir(USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
                                     "bin64"
#else
                                    "/bin/"
#endif
				    + SLASH
                                    ));

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
         strncpy( rmp, RMP.ascii(), n );
         *( rmp + n ) = 0;
         putenv( rmp );
      }
   }
   play_sounds(1);
   editor->append(QString(tr("\n\nWelcome to SOMO UltraScan %1 %2\n"))
		  .arg(US_Version)
		  .arg(REVISION)
		  );
   if ( numThreads > 1 )
   {
      editor->append(QString(tr("Multi-threading enabled using %1 threads\n")).arg(numThreads));
   }
   QColor save_color = editor->color();
   editor->setColor("red");
   if (!dir1.exists())
   {
      editor->append(tr("Warning: Directory ") + somo_dir + tr(" does not exist.\n"));
   }
   if (!dir2.exists())
   {
      editor->append(tr("Warning: Directory ") + somo_pdb_dir + tr(" does not exist.\n"));
   }
   if (!dir3.exists())
   {
      editor->append(tr("Warning: Directory ") + somo_tmp_dir + tr(" does not exist.\n"));
   }
   if (!dir4.exists())
   {
      editor->append(tr("Warning: Directory ") + somo_saxs_dir + tr(" does not exist.\n"));
   }
   if (!dir5.exists())
   {
      editor->append(tr("Warning: Directory ") + somo_saxs_tmp_dir + tr(" does not exist.\n"));
   }
   if (!dir6.exists())
   {
      editor->append(tr("Warning: Directory ") + somo_bd_dir + tr(" does not exist.\n"));
   }
   editor->setColor(save_color);

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
      QColor save_color = editor->color();
      editor->setColor("red");
      editor->append(tr(
                        "Warning: Error setting up SAXS structure factor files.\n"
                        "Bead model SAXS disabled.\n"
                        "Check to make sure the files in SOMO->SAXS/SANS Options are correct.\n"
                        ));
      editor->setColor(save_color);
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
      editor_msg( "red", tr( "Warning: mw.json not read" ) );
   }

   if ( 
       !saxs_util->load_vdw_json( USglobal->config_list.system_dir + 
                                  QDir::separator() + "etc" +
                                  QDir::separator() + "vdw.json" ) )
   {
      editor_msg( "red", tr( "Warning: vdw.json not read" ) );
   }

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
}

US_Hydrodyn::~US_Hydrodyn()
{
}


void US_Hydrodyn::setupGUI()
{
   int minHeight1 = 27;
   bead_model_file = "";

   lookup_tables = new Q3PopupMenu;
   lookup_tables->insertItem(tr("Add/Edit &Hybridization"), this, SLOT(hybrid()));
   lookup_tables->insertItem(tr("Add/Edit &Atom"), this, SLOT(edit_atom()));
   lookup_tables->insertItem(tr("Add/Edit &Residue"), this, SLOT(residue()));
   lookup_tables->insertItem(tr("Add/Edit &SAXS coefficients"), this, SLOT(do_saxs()));

   somo_options = new Q3PopupMenu;
   somo_options->insertItem(tr("&ASA Calculation"), this, SLOT(show_asa()));
   somo_options->insertItem(tr("&SoMo Overlap Reduction"), this, SLOT(show_overlap()));
   somo_options->insertItem(tr("AtoB (Grid) &Overlap Reduction"), this, SLOT(show_grid_overlap()));
   somo_options->insertItem(tr("&Hydrodynamic Calculations"), this, SLOT(show_hydro()));
   somo_options->insertItem(tr("Hydrodynamic Calculations &Zeno"), this, SLOT(show_zeno_options()));
   somo_options->insertItem(tr("&Miscellaneous Options"), this, SLOT(show_misc()));
   somo_options->insertItem(tr("&Bead Model Output"), this, SLOT(show_bead_output()));
   somo_options->insertItem(tr("&Grid Functions (AtoB)"), this, SLOT(show_grid()));
   somo_options->insertItem(tr("SA&XS/SANS Options"), this, SLOT(show_saxs_options()));

   md_options = new Q3PopupMenu;
   // md_options->insertItem(tr("&DMD Options"), this, SLOT(show_dmd_options()));
   md_options->insertItem(tr("&Browflex Options"), this, SLOT(show_bd_options()));
   md_options->insertItem(tr("&Anaflex Options"), this, SLOT(show_anaflex_options()));

   pdb_options = new Q3PopupMenu;
   pdb_options->insertItem(tr("&Parsing"), this, SLOT(pdb_parsing()));
   pdb_options->insertItem(tr("&Visualization"), this, SLOT(pdb_visualization()));

   configuration = new Q3PopupMenu;
   configuration->insertItem(tr("&Load Configuration"), this, SLOT(load_config()));
   configuration->insertItem(tr("&Save Current Configuration"), this, SLOT(write_config()));
   configuration->insertItem(tr("&Reset to Default Configuration"), this, SLOT(reset()));
   configuration->insertItem(tr("&Advanced Configuration"), this, SLOT(show_advanced_config()));
   configuration->insertItem(tr("S&ystem Configuration"), this, SLOT(run_us_config()));
   configuration->insertItem(tr("A&dministrator"), this, SLOT(run_us_admin()));

   Q3Frame *frame;
   frame = new Q3Frame(this);
   frame->setMinimumHeight(minHeight1);

#if !defined(QT4) || !defined(Q_WS_MAC)
   menu = new QMenuBar(frame);
#else
   menu = new QMenuBar( this );
#endif
   menu->setPalette( PALET_NORMAL );
   AUTFBACK( menu );
   menu->insertItem(tr("&Lookup Tables"), lookup_tables);
   menu->insertItem(tr("&SOMO"), somo_options);
   menu->insertItem(tr("&MD"), md_options);
   menu->insertItem(tr("&PDB"), pdb_options);
   menu->insertItem(tr("&Configuration"), configuration);

   lbl_info1 = new QLabel(tr("PDB Functions:"), this);
   Q_CHECK_PTR(lbl_info1);
   lbl_info1->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_info1->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info1->setMinimumHeight(minHeight1);
   lbl_info1->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info1 );
   lbl_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_info2 = new QLabel(tr("Bead Model Functions:"), this);
   Q_CHECK_PTR(lbl_info2);
   lbl_info2->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_info2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info2->setMinimumHeight(minHeight1);
   lbl_info2->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info2 );
   lbl_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_info3 = new QLabel(tr("Hydrodynamic Calculations:"), this);
   Q_CHECK_PTR(lbl_info3);
   lbl_info3->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_info3->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info3->setMinimumHeight(minHeight1);
   lbl_info3->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info3 );
   lbl_info3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   pb_select_residue_file = new QPushButton(tr("Select Lookup Table"), this);
   Q_CHECK_PTR(pb_select_residue_file);
   pb_select_residue_file->setMinimumHeight(minHeight1);
   pb_select_residue_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_select_residue_file->setPalette( PALET_PUSHB );
   connect(pb_select_residue_file, SIGNAL(clicked()), SLOT(select_residue_file()));

   lbl_table = new QLabel( QDir::convertSeparators( residue_filename ), this );
   lbl_table->setMinimumHeight(minHeight1);
   lbl_table->setFrameStyle(Q3Frame::WinPanel|Sunken);
   lbl_table->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_table->setPalette( PALET_EDIT );
   AUTFBACK( lbl_table );
   lbl_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   pb_batch = new QPushButton(tr("Batch Mode Operation"), this);
   Q_CHECK_PTR(pb_batch);
   pb_batch->setMinimumHeight(minHeight1);
   pb_batch->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_batch->setPalette( PALET_PUSHB );
   pb_batch->setAutoDefault( false );
   connect(pb_batch, SIGNAL(clicked()), SLOT(show_batch()));

   pb_view_bead_model = new QPushButton(tr("View Bead Model File"), this);
   Q_CHECK_PTR(pb_view_bead_model);
   pb_view_bead_model->setMinimumHeight(minHeight1);
   pb_view_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_view_bead_model->setPalette( PALET_PUSHB );
   connect(pb_view_bead_model, SIGNAL(clicked()), SLOT(view_bead_model()));

   pb_load_pdb = new QPushButton(tr("Load Single PDB File"), this);
   Q_CHECK_PTR(pb_load_pdb);
   pb_load_pdb->setMinimumHeight(minHeight1);
   pb_load_pdb->setEnabled(true);
   pb_load_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_pdb->setPalette( PALET_PUSHB );
   connect(pb_load_pdb, SIGNAL(clicked()), SLOT(load_pdb()));

   lbl_pdb_file = new QLabel(tr(" not selected"),this);
   lbl_pdb_file->setFrameStyle(Q3Frame::WinPanel|Sunken);
   lbl_pdb_file->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_pdb_file->setMinimumHeight(minHeight1);
   lbl_pdb_file->setPalette( PALET_EDIT );
   AUTFBACK( lbl_pdb_file );
   lbl_pdb_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_model = new QLabel(tr(" Please select a PDB Structure:"), this);
   Q_CHECK_PTR(lbl_model);
   lbl_model->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_model->setMinimumHeight(minHeight1);
   lbl_model->setMargin( 2 );
   lbl_model->setPalette( PALET_LABEL );
   AUTFBACK( lbl_model );
   lbl_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   pb_view_pdb = new QPushButton(tr("View/Edit PDB File"), this);
   Q_CHECK_PTR(pb_view_pdb);
   pb_view_pdb->setMinimumHeight(minHeight1);
   pb_view_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_view_pdb->setPalette( PALET_PUSHB );
   connect(pb_view_pdb, SIGNAL(clicked()), SLOT(view_pdb()));

   pb_pdb_tool = new QPushButton(tr("PDB Editor"), this);
   pb_pdb_tool->setMinimumHeight(minHeight1);
   pb_pdb_tool->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pdb_tool->setPalette( PALET_PUSHB );
   connect(pb_pdb_tool, SIGNAL(clicked()), SLOT(pdb_tool()));

   lb_model = new Q3ListBox(this, "model selection listbox" );
   lb_model->setPalette( PALET_LISTB );
   lb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //lb_model->setEnabled(false);
   lb_model->setEnabled( true );
   lb_model->setSelectionMode(Q3ListBox::Extended);
   lb_model->setHScrollBarMode(Q3ScrollView::Auto);
   lb_model->setVScrollBarMode(Q3ScrollView::Auto);
   connect(lb_model, SIGNAL(selected(int)), this, SLOT(select_model(int)));

   pb_load_bead_model = new QPushButton(tr("Load Single Bead Model File"), this);
   Q_CHECK_PTR(pb_load_bead_model);
   pb_load_bead_model->setMinimumHeight(minHeight1);
   pb_load_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_bead_model->setPalette( PALET_PUSHB );
   connect(pb_load_bead_model, SIGNAL(clicked()), SLOT(load_bead_model()));

   le_bead_model_file = new QLineEdit(this, "bead_model_file Line Edit");
   le_bead_model_file->setText(tr(" not selected "));
   le_bead_model_file->setMinimumHeight(minHeight1);
   le_bead_model_file->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_bead_model_file->setPalette( PALET_EDIT );
   AUTFBACK( le_bead_model_file );
   le_bead_model_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_bead_model_file, SIGNAL(textChanged(const QString &)), SLOT(update_bead_model_file(const QString &)));

   lbl_bead_model_prefix = new QLabel(tr(" Bead Model Suffix:"), this);
   Q_CHECK_PTR(lbl_bead_model_prefix);
   lbl_bead_model_prefix->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bead_model_prefix->setMinimumHeight(minHeight1);
   lbl_bead_model_prefix->setPalette( PALET_LABEL );
   AUTFBACK( lbl_bead_model_prefix );
   lbl_bead_model_prefix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cb_setSuffix = new QCheckBox(this);
   cb_setSuffix->setText(tr(" Add auto-generated suffix "));
   cb_setSuffix->setChecked(setSuffix);
   cb_setSuffix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_setSuffix->setPalette( PALET_NORMAL );
   AUTFBACK( cb_setSuffix );
   connect(cb_setSuffix, SIGNAL(clicked()), this, SLOT(set_setSuffix()));

   cb_overwrite = new QCheckBox(this);
   cb_overwrite->setText(tr(" Overwrite existing filenames "));
   cb_overwrite->setChecked(overwrite);
   cb_overwrite->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_overwrite->setPalette( PALET_NORMAL );
   AUTFBACK( cb_overwrite );
   connect(cb_overwrite, SIGNAL(clicked()), this, SLOT(set_overwrite()));

   le_bead_model_prefix = new QLineEdit(this, "bead_model_prefix Line Edit");
   le_bead_model_prefix->setText(tr(""));
   le_bead_model_prefix->setMinimumHeight(minHeight1);
   le_bead_model_prefix->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_bead_model_prefix->setPalette( PALET_EDIT );
   AUTFBACK( le_bead_model_prefix );
   le_bead_model_prefix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_bead_model_prefix, SIGNAL(textChanged(const QString &)), SLOT(update_bead_model_prefix(const QString &)));

   le_bead_model_suffix = new QLineEdit(this, "bead_model_suffix Line Edit");
   le_bead_model_suffix->setText(tr(""));
   le_bead_model_suffix->setMinimumHeight(minHeight1);
   le_bead_model_suffix->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_bead_model_suffix->setPalette( PALET_EDIT );
   AUTFBACK( le_bead_model_suffix );
   le_bead_model_suffix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_bead_model_suffix->setReadOnly(true);

   pb_somo = new QPushButton(tr("Build SoMo Bead Model"), this);
   Q_CHECK_PTR(pb_somo);
   pb_somo->setMinimumHeight(minHeight1);
   pb_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_somo->setEnabled(false);
   pb_somo->setPalette( PALET_PUSHB );
   connect(pb_somo, SIGNAL(clicked()), SLOT(calc_somo()));

#if defined(USE_H)
   pb_pdb_hydrate_for_saxs = new QPushButton(tr("Hydrate"), this);
   pb_pdb_hydrate_for_saxs->setMinimumHeight(minHeight1);
   pb_pdb_hydrate_for_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pdb_hydrate_for_saxs->setEnabled(true);
   pb_pdb_hydrate_for_saxs->setPalette( PALET_PUSHB );
   connect(pb_pdb_hydrate_for_saxs, SIGNAL(clicked()), SLOT(pdb_hydrate_for_saxs()));
#endif

   pb_pdb_saxs = new QPushButton(tr("SAXS/SANS Functions"), this);
   Q_CHECK_PTR(pb_pdb_saxs);
   pb_pdb_saxs->setMinimumHeight(minHeight1);
   pb_pdb_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pdb_saxs->setEnabled(true);
   pb_pdb_saxs->setPalette( PALET_PUSHB );
   connect(pb_pdb_saxs, SIGNAL(clicked()), SLOT(pdb_saxs()));

   pb_bead_saxs = new QPushButton(tr("SAXS/SANS Functions"), this);
   Q_CHECK_PTR(pb_bead_saxs);
   pb_bead_saxs->setMinimumHeight(minHeight1);
   pb_bead_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_bead_saxs->setEnabled(false);
   pb_bead_saxs->setPalette( PALET_PUSHB );
   connect(pb_bead_saxs, SIGNAL(clicked()), SLOT(bead_saxs()));

   pb_rescale_bead_model = new QPushButton(tr("Rescale/Equalize Bead Model"), this);
   pb_rescale_bead_model->setMinimumHeight(minHeight1);
   pb_rescale_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_rescale_bead_model->setEnabled(false);
   pb_rescale_bead_model->setPalette( PALET_PUSHB );
   connect(pb_rescale_bead_model, SIGNAL(clicked()), SLOT(rescale_bead_model()));

   pb_equi_grid_bead_model = new QPushButton(tr("Simple grid"), this);
   pb_equi_grid_bead_model->setMinimumHeight(minHeight1);
   pb_equi_grid_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_equi_grid_bead_model->setEnabled(false);
   pb_equi_grid_bead_model->setPalette( PALET_PUSHB );
   connect(pb_equi_grid_bead_model, SIGNAL(clicked()), SLOT(equi_grid_bead_model()));

   pb_grid_pdb = new QPushButton(tr("Build AtoB (Grid) Bead Model"), this);
   Q_CHECK_PTR(pb_grid_pdb);
   pb_grid_pdb->setMinimumHeight(minHeight1);
   pb_grid_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_grid_pdb->setEnabled(false);
   pb_grid_pdb->setPalette( PALET_PUSHB );
   connect(pb_grid_pdb, SIGNAL(clicked()), SLOT(calc_grid_pdb()));

   pb_grid = new QPushButton(tr("Grid Existing Bead Model"), this);
   Q_CHECK_PTR(pb_grid);
   pb_grid->setMinimumHeight(minHeight1);
   pb_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_grid->setEnabled(false);
   pb_grid->setPalette( PALET_PUSHB );
   connect(pb_grid, SIGNAL(clicked()), SLOT(calc_grid()));

   cb_calcAutoHydro = new QCheckBox(this);
   cb_calcAutoHydro->setText(tr(" Automatically Calculate Hydrodynamics "));
   cb_calcAutoHydro->setChecked(calcAutoHydro);
   cb_calcAutoHydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_calcAutoHydro->setPalette( PALET_NORMAL );
   AUTFBACK( cb_calcAutoHydro );
   connect(cb_calcAutoHydro, SIGNAL(clicked()), this, SLOT(set_calcAutoHydro()));

   pb_view_asa = new QPushButton(tr("View ASA Results"), this);
   Q_CHECK_PTR(pb_view_asa);
   pb_view_asa->setMinimumHeight(minHeight1);
   pb_view_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_view_asa->setPalette( PALET_PUSHB );
   connect(pb_view_asa, SIGNAL(clicked()), SLOT(view_asa()));

   pb_visualize = new QPushButton(tr("Visualize Bead Model"), this);
   Q_CHECK_PTR(pb_visualize);
   pb_visualize->setMinimumHeight(minHeight1);
   pb_visualize->setEnabled(false);
   pb_visualize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_visualize->setPalette( PALET_PUSHB );
   connect(pb_visualize, SIGNAL(clicked()), SLOT(visualize()));

   pb_batch2 = new QPushButton(tr("Batch Mode Operation"), this);
   Q_CHECK_PTR(pb_batch2);
   pb_batch2->setMinimumHeight(minHeight1);
   pb_batch2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_batch2->setPalette( PALET_PUSHB );
   connect(pb_batch2, SIGNAL(clicked()), SLOT(show_batch()));

   pb_calc_hydro = new QPushButton(tr("Calculate RB Hydrodynamics"), this);
   Q_CHECK_PTR(pb_calc_hydro);
   pb_calc_hydro->setEnabled(false);
   pb_calc_hydro->setMinimumHeight(minHeight1);
   pb_calc_hydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_calc_hydro->setPalette( PALET_PUSHB );
   connect(pb_calc_hydro, SIGNAL(clicked()), SLOT(calc_hydro()));

   pb_show_hydro_results = new QPushButton(tr("Show Hydrodynamic Calculations"), this);
   Q_CHECK_PTR(pb_show_hydro_results);
   pb_show_hydro_results->setMinimumHeight(minHeight1);
   pb_show_hydro_results->setEnabled(false);
   pb_show_hydro_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_show_hydro_results->setPalette( PALET_PUSHB );
   connect(pb_show_hydro_results, SIGNAL(clicked()), SLOT(show_hydro_results()));

   pb_comparative = new QPushButton(tr("Model classifier"), this);
   pb_comparative->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_comparative->setEnabled(true);
   pb_comparative->setPalette( PALET_PUSHB );
   connect(pb_comparative, SIGNAL(clicked()), SLOT(select_comparative()));

   pb_best = new QPushButton(tr("BEST"), this);
   pb_best->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_best->setEnabled(true);
   pb_best->setPalette( PALET_PUSHB );
   connect(pb_best, SIGNAL(clicked()), SLOT( best_analysis()));
   pb_best->hide();

   pb_open_hydro_results = new QPushButton(tr("Open Hydrodynamic Calculations File"), this);
   Q_CHECK_PTR(pb_open_hydro_results);
   pb_open_hydro_results->setMinimumHeight(minHeight1);
   pb_open_hydro_results->setEnabled(true);
   pb_open_hydro_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_open_hydro_results->setPalette( PALET_PUSHB );
   connect(pb_open_hydro_results, SIGNAL(clicked()), SLOT(open_hydro_results()));

   pb_select_save_params = new QPushButton(tr("Select Parameters to be Saved"), this);
   Q_CHECK_PTR(pb_select_save_params);
   pb_select_save_params->setMinimumHeight(minHeight1);
   pb_select_save_params->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_select_save_params->setEnabled(true);
   pb_select_save_params->setPalette( PALET_PUSHB );
   connect(pb_select_save_params, SIGNAL(clicked()), SLOT(select_save_params()));

   cb_saveParams = new QCheckBox(this);
   cb_saveParams->setText(tr(" Save parameters to file "));
   cb_saveParams->setChecked(saveParams);
   cb_saveParams->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saveParams->setPalette( PALET_NORMAL );
   AUTFBACK( cb_saveParams );
   connect(cb_saveParams, SIGNAL(clicked()), this, SLOT(set_saveParams()));

   // ***** dmd *******
   pb_dmd_run = new QPushButton(tr("Run DMD"), this);
   pb_dmd_run->setMinimumHeight(minHeight1);
   pb_dmd_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_dmd_run->setEnabled( true );
   pb_dmd_run->setPalette( PALET_PUSHB );
   connect(pb_dmd_run, SIGNAL(clicked()), SLOT(dmd_run()));

   pb_bd = new QPushButton(tr("BD"), this);
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
   //   pb_bd_prepare = new QPushButton(tr("Create Browflex files"), this);
   //   pb_bd_prepare->setMinimumHeight(minHeight1);
   //   pb_bd_prepare->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_prepare->setEnabled(false);
   //   pb_bd_prepare->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   connect(pb_bd_prepare, SIGNAL(clicked()), SLOT(bd_prepare()));

   //   pb_bd_load = new QPushButton(tr("Load Browflex files"), this);
   //   pb_bd_load->setMinimumHeight(minHeight1);
   //   pb_bd_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_load->setEnabled(false);
   //   pb_bd_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   connect(pb_bd_load, SIGNAL(clicked()), SLOT(bd_load()));

   //   pb_bd_edit = new QPushButton(tr("View/Edit Browflex files"), this);
   //   pb_bd_edit->setMinimumHeight(minHeight1);
   //   pb_bd_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_edit->setEnabled(false);
   //   pb_bd_edit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   connect(pb_bd_edit, SIGNAL(clicked()), SLOT(bd_edit()));

   //   pb_bd_run = new QPushButton(tr("Run Browflex"), this);
   //   pb_bd_run->setMinimumHeight(minHeight1);
   //   pb_bd_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_run->setEnabled(false);
   //   pb_bd_run->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   connect(pb_bd_run, SIGNAL(clicked()), SLOT(bd_run()));

   //   pb_bd_load_results = new QPushButton(tr("Load/Process Browflex results"), this);
   //   pb_bd_load_results->setMinimumHeight(minHeight1);
   //   pb_bd_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_load_results->setEnabled(false);
   //   pb_bd_load_results->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   connect(pb_bd_load_results, SIGNAL(clicked()), SLOT(bd_load_results()));

   // ***** anaflex *******
   //   pb_anaflex_prepare = new QPushButton(tr("Create Anaflex files"), this);
   //   pb_anaflex_prepare->setMinimumHeight(minHeight1);
   //   pb_anaflex_prepare->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_prepare->setEnabled(false);
   //   pb_anaflex_prepare->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   connect(pb_anaflex_prepare, SIGNAL(clicked()), SLOT(anaflex_prepare()));

   //   pb_anaflex_load = new QPushButton(tr("Load Anaflex files"), this);
   //   pb_anaflex_load->setMinimumHeight(minHeight1);
   //   pb_anaflex_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_load->setEnabled(false);
   //   pb_anaflex_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   connect(pb_anaflex_load, SIGNAL(clicked()), SLOT(anaflex_load()));

   //   pb_anaflex_edit = new QPushButton(tr("View/Edit Anaflex files"), this);
   //   pb_anaflex_edit->setMinimumHeight(minHeight1);
   //   pb_anaflex_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_edit->setEnabled(false);
   //   pb_anaflex_edit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   connect(pb_anaflex_edit, SIGNAL(clicked()), SLOT(anaflex_edit()));

   //   pb_anaflex_run = new QPushButton(tr("Run Anaflex"), this);
   //   pb_anaflex_run->setMinimumHeight(minHeight1);
   //   pb_anaflex_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_run->setEnabled(false);
   //   pb_anaflex_run->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   connect(pb_anaflex_run, SIGNAL(clicked()), SLOT(anaflex_run()));

   //   pb_anaflex_load_results = new QPushButton(tr("Load Anaflex results"), this);
   //   pb_anaflex_load_results->setMinimumHeight(minHeight1);
   //   pb_anaflex_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_load_results->setEnabled(false);
   //   pb_anaflex_load_results->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   //   connect(pb_anaflex_load_results, SIGNAL(clicked()), SLOT(anaflex_load_results()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));


   pb_config = new QPushButton(tr("Config"), this);
   pb_config->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_config->setMinimumHeight(minHeight1);
   pb_config->setPalette( PALET_PUSHB );
   connect(pb_config, SIGNAL(clicked()), SLOT(config()));

   pb_stop_calc = new QPushButton(tr("Stop"), this);
   Q_CHECK_PTR(pb_stop_calc);
   pb_stop_calc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_stop_calc->setMinimumHeight(minHeight1);
   pb_stop_calc->setPalette( PALET_PUSHB );
   connect(pb_stop_calc, SIGNAL(clicked()), SLOT(stop_calc()));
   pb_stop_calc->setEnabled(false);

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   progress = new Q3ProgressBar(this, "Loading Progress");
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->reset();

   editor = new Q3TextEdit(this);
   editor->setPalette( PALET_NORMAL );
   editor->setReadOnly(true);
   editor->setMinimumWidth(550);
   m = new QMenuBar(editor, "menu" );
   m->setMinimumHeight(minHeight1);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );

#ifndef NO_EDITOR_PRINT
   file->insertItem( tr("Print"), this, SLOT(print()),   Qt::ALT+Qt::Key_P );
#endif
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
   editor->setWordWrap (advanced_config.scroll_editor ? Q3TextEdit::NoWrap : Q3TextEdit::WidgetWidth);

   lbl_core_progress = new QLabel("", this);
   Q_CHECK_PTR(lbl_core_progress);
   lbl_core_progress->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_core_progress->setMinimumHeight(minHeight1);
   lbl_core_progress->setPalette( PALET_LABEL );
   AUTFBACK( lbl_core_progress );
   lbl_core_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   clear_display();

   int rows=20, columns = 3, spacing = 2, j=0, margin=4;
   Q3GridLayout *background = new Q3GridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(frame, j, j, 0, 1);
   background->addMultiCellWidget(editor, j, j+23, 2, 2);
   j++;
   background->addMultiCellWidget(lbl_info1, j, j, 0, 1);
   j++;
   background->addWidget(pb_select_residue_file, j, 0);
   background->addWidget(lbl_table, j, 1);
   j++;
   background->addWidget(pb_batch, j, 0);
   j++;
   background->addWidget(pb_load_pdb, j, 0);
   background->addWidget(lbl_pdb_file, j, 1);
   j++;
   background->addWidget(lbl_model, j, 0);
   background->addMultiCellWidget(lb_model, j, j+4, 1, 1);
   j++;
   Q3HBoxLayout *hbl_pdb = new Q3HBoxLayout(0);
   hbl_pdb->addWidget(pb_view_pdb);
   hbl_pdb->addWidget(pb_pdb_tool);
   background->addLayout(hbl_pdb, j, 0);
   j++;
   Q3HBoxLayout *hbl_pdb_saxs = new Q3HBoxLayout;
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
   background->addMultiCellWidget(lbl_info2, j, j, 0, 1);
   j++;
   //   background->addWidget(lbl_bead_model_prefix, j, 0);
   Q3HBoxLayout *hbl_prefix_suffix = new Q3HBoxLayout;
   hbl_prefix_suffix->addWidget(lbl_bead_model_prefix);
   hbl_prefix_suffix->addWidget(le_bead_model_prefix);
   // hbl_prefix_suffix->addWidget(le_bead_model_suffix);
   background->addLayout(hbl_prefix_suffix, j, 0);
   background->addWidget(le_bead_model_suffix, j, 1);
   // QGridLayout *gl_prefix_suffix = new QGridLayout(1,3);
   // gl_prefix_suffix->addMultiCellWidget(le_bead_model_prefix, 0, 0, 0, 0);
   // gl_prefix_suffix->addMultiCellWidget(le_bead_model_suffix, 0, 0, 1, 2);
   // background->addLayout(gl_prefix_suffix, j, 1);   
   j++;
   background->addWidget(cb_overwrite, j, 0);
   background->addWidget(cb_setSuffix, j, 1);
   j++;
   background->addWidget(pb_somo, j, 0);
   background->addWidget(pb_grid_pdb, j, 1);
   j++;
   background->addWidget(pb_grid, j, 0);
   background->addWidget(cb_calcAutoHydro, j, 1);
   j++;
   //   background->addWidget(pb_bd_prepare, j, 0);
   //   QHBoxLayout *qhl_bd_1 = new QHBoxLayout;
   //   qhl_bd_1->addWidget(pb_bd_load);
   //   qhl_bd_1->addWidget(pb_bd_edit);
   //   background->addLayout(qhl_bd_1, j, 1);
   //   j++;
   //   background->addWidget(pb_bd_run, j, 0);
   //   background->addWidget(pb_bd_load_results, j, 1);
   //   j++;
   background->addWidget(pb_view_asa, j, 0);
   background->addWidget(pb_visualize, j, 1);
   j++;
   background->addWidget(pb_batch2, j, 0);
   background->addWidget(pb_view_bead_model, j, 1);
   j++;
   background->addWidget(pb_load_bead_model, j, 0);
   background->addWidget(le_bead_model_file, j, 1);
   j++;
   background->addWidget(pb_bead_saxs, j, 0);
   // background->addWidget(pb_rescale_bead_model, j, 1);
   Q3BoxLayout *hbl_rescale_equi = new Q3HBoxLayout( 0 );
   hbl_rescale_equi->addWidget( pb_rescale_bead_model );
   hbl_rescale_equi->addWidget( pb_equi_grid_bead_model );
   background->addLayout(hbl_rescale_equi, j, 1);
   j++;
   background->addMultiCellWidget(lbl_info3, j, j, 0, 1);
   j++;

   Q3BoxLayout *hbl_calc_hydros = new Q3HBoxLayout( 0 );
   hbl_calc_hydros->addWidget( pb_calc_hydro );
   background->addLayout(hbl_calc_hydros, j, 0);

   Q3BoxLayout *hbl_show_hydros = new Q3HBoxLayout( 0 );
   hbl_show_hydros->addWidget( pb_show_hydro_results );
   background->addLayout( hbl_show_hydros, j, 1);
   j++;
   //   background->addWidget(pb_anaflex_prepare, j, 0);
   //   QHBoxLayout *qhl_anaflex_1 = new QHBoxLayout;
   //   qhl_anaflex_1->addWidget(pb_anaflex_load);
   //   qhl_anaflex_1->addWidget(pb_anaflex_edit);
   //   background->addLayout(qhl_anaflex_1, j, 1);
   //   j++;
   //   background->addWidget(pb_anaflex_run, j, 0);
   //   background->addWidget(pb_anaflex_load_results, j, 1);
   //   j++;
   {
      Q3BoxLayout * hbl = new Q3HBoxLayout( 0 );
      hbl->addWidget( pb_comparative );
      hbl->addWidget( pb_best );
      background->addLayout( hbl, j, 0 );
   }
   // background->addWidget(pb_comparative, j, 0);
   background->addWidget(pb_open_hydro_results, j, 1);
   j++;
   background->addWidget(pb_select_save_params, j, 0);
   background->addWidget(cb_saveParams, j, 1);
   j++;
   background->addWidget(pb_stop_calc, j, 0);
   background->addWidget(pb_cancel, j, 1);
// background->addMultiCellWidget(progress, j, j, 0, 1);
   j++;
   Q3BoxLayout *bl_help_config = new Q3HBoxLayout;
   bl_help_config->addWidget( pb_help );
   bl_help_config->addWidget( pb_config );

   background->addLayout( bl_help_config, j, 0);
   background->addWidget(progress, j, 1);
   background->addWidget(lbl_core_progress, j, 2);

   fixWinButtons( this );
}

void US_Hydrodyn::set_expert( bool expert )
{
   if ( expert )
   {
      pb_best->show();
      lookup_tables->insertItem(tr("Make test set"), this, SLOT( make_test_set() ) );
   }
}

void US_Hydrodyn::set_disabled()
{
   pb_somo->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
   pb_grid->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
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
   QMessageBox::message(tr("Please note:"),
                        tr("Function not available in this version." ) );
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
   QMessageBox::message(tr("Please note:"),
                        tr("Function not available in this version." ) );
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
   QMessageBox::message(tr("Please note:"),
                        tr("Function not available in this version." ) );
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
   QString fname = Q3FileDialog::getOpenFileName ( somo_dir, "*.config", 0, 0, "Please select a SOMO configuration file...", 0, TRUE );
   if ( fname == QString::null )
   {
      QColor save_color = editor->color();
      editor->setColor("red");
      editor->append(tr("\nLoad configuration canceled."));
      editor->setColor(save_color);
      display_default_differences();
      return;
   }

   if ( read_config(fname) )
   {
      QMessageBox::message(tr("Please note:"),
                           tr("The configuration file was found to be corrupt.\n"
                              "Resorting to default values."));
      set_default();
   }
   clear_display();
}

void US_Hydrodyn::write_config()
{
   QString fname = Q3FileDialog::getSaveFileName ( somo_dir, "*.config", 0, 0, "Please name your SOMO configuration file...", 0, TRUE );
   if (fname.right(7) != ".config")
   {
      fname += ".config";
   }
   write_config(fname);
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
   //  save = default_save;
}

void US_Hydrodyn::reset()
{
   QMessageBox mb(tr("UltraScan"), tr("Attention:\nAre you sure you want to reset to the default options?\nAll currently defined options will be reset."),
                  QMessageBox::Information,
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::Cancel | QMessageBox::Escape,
                  QMessageBox::No);
   mb.setButtonText(QMessageBox::Yes, tr("Yes"));
   mb.setButtonText(QMessageBox::Cancel, tr("Cancel"));
   mb.setButtonText(QMessageBox::No, tr("Save Current Options"));
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
   residue_filename = Q3FileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.residue *.RESIDUE", this);
   if (residue_filename.isEmpty())
   {
      residue_filename = old_filename;
      return;
   }
   read_residue_file();
   set_disabled();
   lbl_table->setText( QDir::convertSeparators( residue_filename ) );
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
   editor->append(QString("Loaded pdb file : %1\n").arg(errors_found ? "ERRORS PRESENT" : "ok"));
   if ( errors_found )
   {
      calc_vol_for_saxs();
   } else {
      calc_mw();
   }
   bead_models.clear();
   somo_processed.clear();
   update_vbar();
   pb_somo->setEnabled(true);
   pb_grid_pdb->setEnabled(true);
   pb_grid->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_bead_saxs->setEnabled(false);
   pb_rescale_bead_model->setEnabled(false);
   pb_pdb_saxs->setEnabled(true);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   le_bead_model_file->setText(" not selected ");
   bead_models_as_loaded = bead_models;
}

void US_Hydrodyn::load_pdb()
{
   QString message = "";
   if ( pdb_parse.missing_residues == 1 &&
        !advanced_config.expert_mode )
   {
      message += tr("You have selected to skip missing residues. If your model contains missing\n"
                    "residues, the calculated molecular weight and vbar may be incorrect, and\n"
                    "you should manually enter a global value for the molecular weight in the\n"
                    "SOMO hydrodynamic options, and a global value for the vbar in the SOMO\n"
                    "Miscellaneous options.\n\nAre you sure you want to proceed?");
   }
   if ( pdb_parse.missing_residues == 2 &&
        !advanced_config.expert_mode )
   {
      message += tr("You have selected to replace non-coded residues with an average residue.\n"
                    "If your model contains non-coded residues, the calculated molecular weight\n"
                    "and vbar may be incorrect. Therefore, you could manually enter a global\n"
                    "value for the molecular weight in the SOMO hydrodynamic options, and a\n"
                    "global value for the vbar in the SOMO Miscellaneous options. You can also\n"
                    "review the average residue settings in the SOMO Miscellaneous options.\n\n"
                    "Are you sure you want to proceed?");
   }
   if (message != "")
   {
      QMessageBox mb(tr("UltraScan"), tr("Attention:\n" + message),
                     QMessageBox::Information,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::Cancel | QMessageBox::Escape,
                     Qt::NoButton);
      mb.setButtonText(QMessageBox::Yes, tr("Yes"));
      mb.setButtonText(QMessageBox::Cancel, tr("Cancel"));
      switch(mb.exec())
      {
         case QMessageBox::Cancel:
         {
            return;
         }
      }
   }
   message = "";
   if ( pdb_parse.missing_atoms == 1 &&
        !advanced_config.expert_mode )
   {
      message += tr("You have selected to skip coded residues containing missing atoms.\n"
                    "If your model contains missing atoms, the calculated molecular\n"
                    "weight and vbar may be incorrect, and you should manually enter\n"
                    "a global value for the molecular weight in the SOMO hydrodynamic\n"
                    "options, and a global value for the vbar in the SOMO Miscellaneous\n"
                    "options.\n\nAre you sure you want to proceed?");
   }
   if ( pdb_parse.missing_atoms == 2 &&
        !advanced_config.expert_mode )
   {
      message += tr("You have selected to model coded residues with missing atoms\n"
                    "with an approximate method.  For best results, you should complete\n"
                    "the structure.\n\n"
                    "Do you want to proceed anyway?");
   }
   if (message != "")
   {
      QMessageBox mb(tr("UltraScan"), tr("Attention:\n" + message),
                     QMessageBox::Information,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::Cancel | QMessageBox::Escape,
                     Qt::NoButton);
      mb.setButtonText(QMessageBox::Yes, tr("Yes"));
      mb.setButtonText(QMessageBox::Cancel, tr("Cancel"));
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

   QString filename = Q3FileDialog::getOpenFileName(use_dir,
                                                   "Structures (*.pdb *.PDB)",
                                                   this,
                                                   "Open Structure Files",
                                                   "Please select a PDB file...");
   if ( !filename.isEmpty() )
   {
      path_load_pdb = QFileInfo(filename).dirPath(true);
   }

   if ( QFileInfo(filename).fileName().contains(" ") )
   {
      printError(tr("Filenames containing spaces are not currently supported.\n"
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
         QStringList argument;
#if !defined(WIN32) && !defined(MAC)
         // maybe we should make this a user defined terminal window?
         argument.append("xterm");
         argument.append("-e");
#endif
#if defined(BIN64)
         argument.append(USglobal->config_list.system_dir + SLASH + "bin64" + SLASH + "rasmol");
#else
         argument.append(USglobal->config_list.system_dir + SLASH + "bin" + SLASH + "rasmol");
#endif
         argument.append(QFileInfo(filename).fileName());
         rasmol->setWorkingDirectory(QFileInfo(filename).dirPath());
         rasmol->setArguments(argument);
         if ( advanced_config.debug_1 )
         {
            editor->append(QString("starting rasmol <%1>\n").arg(argument.join("><")));
            printf("starting rasmol<%s><s>\n", argument.join("><").ascii());
            fflush(stdout);
         }
         if (advanced_config.auto_view_pdb &&
             !rasmol->start())
         {
            QMessageBox::message(tr("Please note:"), tr("There was a problem starting RASMOL\n"
                                                        "Please check to make sure RASMOL is properly installed..."));
         }
#endif
         return;
      }
      pdb_file = filename;
      options_log = "";
      last_abb_msgs = "";
      bead_model_from_file = false;
      lbl_pdb_file->setText( QDir::convertSeparators( filename ) );
      clear_display();

#if defined(START_RASMOL)
      QStringList argument;
#if !defined(WIN32) && !defined(MAC)
      // maybe we should make this a user defined terminal window?
      argument.append("xterm");
      argument.append("-e");
#endif
#if defined(BIN64)
      argument.append(USglobal->config_list.system_dir + SLASH + "bin64" + SLASH + "rasmol");
#else
      argument.append(USglobal->config_list.system_dir + SLASH + "bin" + SLASH + "rasmol");
#endif
      argument.append(QFileInfo(filename).fileName());
      rasmol->setWorkingDirectory(QFileInfo(filename).dirPath());
      rasmol->setArguments(argument);
      if ( advanced_config.debug_1 )
      {
         editor->append(QString("starting rasmol <%1>\n").arg(argument.join("><")));
         printf("starting rasmol<%s><s>\n", argument.join("><").ascii());
         fflush(stdout);
      }
      if (advanced_config.auto_view_pdb &&
          !rasmol->start())
      {
         QMessageBox::message(tr("Please note:"), tr("There was a problem starting RASMOL\n"
                                                    "Please check to make sure RASMOL is properly installed..."));
      }
#endif
      QFileInfo fi(filename);
      project = fi.baseName();
      new_residues.clear();
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
      editor->append(QString("Loaded pdb file : %1\n").arg(errors_found ? "ERRORS PRESENT" : "ok"));
      bead_models.clear();
      somo_processed.clear();
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
   pb_grid_pdb->setEnabled(true);
   pb_grid->setEnabled(false);
   bd_anaflex_enables(true);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_pdb_saxs->setEnabled(true);
   pb_bead_saxs->setEnabled(false);
   pb_rescale_bead_model->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   le_bead_model_file->setText(" not selected ");
   bead_models_as_loaded = bead_models;
   if ( lb_model->numRows() == 1 )
   {
      select_model(0);
   }

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

bool US_Hydrodyn::screen_pdb(QString filename, bool display_pdb)
{
   cout << QString( "screen pdb display is %1\n" ).arg( display_pdb ? "true" : "false" );
   pdb_file = filename;

   if ( QFileInfo(filename).fileName().contains(" ") )
   {
      printError(tr("Filenames containing spaces are not currently supported.\n"
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
   lbl_pdb_file->setText( QDir::convertSeparators( filename ) );

   bead_model_suffix = "";
   le_bead_model_suffix->setText( bead_model_suffix );

#if defined(START_RASMOL)
   if ( display_pdb )
   {
      QStringList argument;
#if !defined(WIN32) && !defined(MAC)
      // maybe we should make this a user defined terminal window?
      argument.append("xterm");
      argument.append("-e");
#endif
#if defined(BIN64)
      argument.append(USglobal->config_list.system_dir + SLASH + "bin64" + SLASH + "rasmol");
#else
      argument.append(USglobal->config_list.system_dir + SLASH + "bin" + SLASH + "rasmol");
#endif
      argument.append(QFileInfo(filename).fileName());
      rasmol->setWorkingDirectory(QFileInfo(filename).dirPath());
      rasmol->setArguments(argument);
      if (advanced_config.auto_view_pdb &&
          !rasmol->start())
      {
         QMessageBox::message(tr("Please note:"), tr("There was a problem starting RASMOL\n"
                                                     "Please check to make sure RASMOL is properly installed..."));
      }
   }
#endif

   QFileInfo fi(filename);
   project = fi.baseName();
   new_residues.clear();
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
   QString error_string = "";
   for(unsigned int i = 0; i < model_vector.size(); i++)
   {
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
   if ( errors_found )
   {
      calc_vol_for_saxs();
   } else {
      calc_mw();
   }
   model_vector_as_loaded = model_vector;
   if ( !model_vector.size() ||
        !model_vector[0].molecule.size() )
   {
      QColor save_color = editor->color();
      editor->setColor("red");
      editor->append(tr("ERROR : PDB file contains no atoms!"));
      editor->setColor(save_color);
      errors_found++;
   }

   editor->append(QString("Loaded pdb file : %1\n").arg(errors_found ? "ERRORS PRESENT" : "ok"));
   bead_models.clear();
   somo_processed.clear();
   update_vbar();
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }
   // bead_model_prefix = "";
   pb_somo->setEnabled(errors_found ? false : true);
   pb_grid_pdb->setEnabled(errors_found ? false : true);
   pb_grid->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_bead_saxs->setEnabled(false);
   pb_rescale_bead_model->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   le_bead_model_file->setText(" not selected ");
   bead_models_as_loaded = bead_models;
   if ( lb_model->numRows() == 1 )
   {
      select_model(0);
   }
   return errors_found ? false : true;
}   

bool US_Hydrodyn::screen_bead_model( QString filename )
{
   if ( QFileInfo(filename).fileName().contains(" ") )
   {
      printError(tr("Filenames containing spaces are not currently supported.\n"
                    "Please rename the file to remove spaces."));
      return false;
   }
   options_log = "";
   pb_somo->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
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
   le_bead_model_file->setText( QDir::convertSeparators( filename ) );
   bool only_overlap = false;
   if ( !read_bead_model(filename, only_overlap ))
   {
      state = BEAD_MODEL_LOADED;
      pb_visualize->setEnabled(true);
      pb_equi_grid_bead_model->setEnabled(true);
      pb_calc_hydro->setEnabled(true);
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

   QString filename = Q3FileDialog::getOpenFileName(use_dir, "*.pdb *.PDB", this);
   
   if (!filename.isEmpty())
   {
      path_view_pdb = QFileInfo(filename).dirPath(true);
      view_file(filename);
   }
}

QString US_Hydrodyn::model_name( int val )
{
   return QString( "%1" ).arg( lb_model->item( val )->text() ).replace( "Model: ", "" );
}

void US_Hydrodyn::select_model( int val )
{
   current_model = val;
   QString msg = QString( "\n%1 models selected:" ).arg( project );
   for( int i = 0; i < lb_model->numRows(); i++ )
   {
      if ( lb_model->isSelected( i ) )
      {
         current_model = i;
         // msg += QString( " %1" ).arg( i + 1 );
         msg += " " + model_name( i );
      }
   }
   msg += "\n";
   editor->append( msg );

   // check integrity of PDB file and confirm that all residues are correctly defined in residue table
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }
   pb_somo->setEnabled(true);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   //   pb_pdb_saxs->setEnabled(true);
   bd_anaflex_enables( ( ( browflex && browflex->isRunning() ) ||
                         ( anaflex && anaflex->isRunning() ) ) ? false : true );
}

void US_Hydrodyn::write_bead_ebf(QString fname, vector<PDB_atom> *model)
{
   {
      FILE *f = fopen(fname.ascii(), "w");
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
      FILE *f = fopen(QString("%1-info").arg(fname).ascii(), "w");
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

   QString filename = Q3FileDialog::getOpenFileName(use_dir

                                                   ,"Bead models (*.bead_model *.BEAD_MODEL);;"
                                                   "BEAMS (*.beams *.BEAMS);;"
                                                   "DAMMIN/DAMMIF/DAMAVER (*.pdb)"

                                                   , this
                                                   , "open file dialog"
                                                   , "Open"
                                                   , &bead_model_selected_filter
                                                   );

   if ( !filename.isEmpty() )
   {
      path_load_bead_model = QFileInfo(filename).dirPath(true);
   }

   if ( QFileInfo(filename).fileName().contains(" ") )
   {
      printError(tr("Filenames containing spaces are not currently supported.\n"
                    "Please rename the file to remove spaces."));
      return;
   }

   if (!filename.isEmpty())
   {
      add_to_directory_history( filename );

      options_log = "";
      pb_somo->setEnabled(false);
      pb_visualize->setEnabled(false);
      pb_equi_grid_bead_model->setEnabled(false);
      pb_calc_hydro->setEnabled(false);
      pb_show_hydro_results->setEnabled(false);
      pb_grid_pdb->setEnabled(false);
      pb_grid->setEnabled(false);
      bead_model_prefix = "";
      le_bead_model_prefix->setText(bead_model_prefix);

      if (results_widget)
      {
         results_window->close();
         delete results_window;
         results_widget = false;
      }

      bead_model_file = filename;
      le_bead_model_file->setText( QDir::convertSeparators( filename ) );

      if ( is_dammin_dammif(filename) &&
           advanced_config.auto_view_pdb ) 
      {
#if defined(START_RASMOL)
         QStringList argument;
#if !defined(WIN32) && !defined(MAC)
         // maybe we should make this a user defined terminal window?
         argument.append("xterm");
         argument.append("-e");
#endif
#if defined(BIN64)
         argument.append(USglobal->config_list.system_dir + SLASH + "bin64" + SLASH + "rasmol");
#else
         argument.append(USglobal->config_list.system_dir + SLASH + "bin" + SLASH + "rasmol");
#endif
         argument.append(QFileInfo(filename).fileName());
         rasmol->setWorkingDirectory(QFileInfo(filename).dirPath());
         rasmol->setArguments(argument);
         if ( advanced_config.debug_1 )
         {
            editor->append(QString("starting rasmol <%1>\n").arg(argument.join("><")));
            printf("starting rasmol<%s><s>\n", argument.join("><").ascii());
            fflush(stdout);
         }
         if (advanced_config.auto_view_pdb &&
             !rasmol->start())
         {
            QMessageBox::message(tr("Please note:"), tr("There was a problem starting RASMOL\n"
                                                        "Please check to make sure RASMOL is properly installed..."));
         }
#endif
      }

      bool only_overlap = false;
      if (!read_bead_model(filename, only_overlap ))
      {
         state = BEAD_MODEL_LOADED;
         pb_visualize->setEnabled(true);
         pb_equi_grid_bead_model->setEnabled(true);
         pb_calc_hydro->setEnabled(true);
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

int US_Hydrodyn::calc_somo()
{
   if ( selected_models_contain( "WAT" ) )
   {
      QMessageBox::warning( this,
                            tr( "Selected model contains WAT residue" ),
                            tr( 
                               "Can not process models that contain the WAT residue.\n"
                               "These are currently generated only for SAXS/SANS computations\n"
                               )
                            );
      return -1;
   }

   if ( selected_models_contain( "XHY" ) )
   {
      QMessageBox::warning( this,
                            tr( "Selected model contains XHY residue" ),
                            tr( 
                               "Can not process models that contain the XHY residue.\n"
                               "These are currently generated only for SAXS/SANS computations\n"
                               )
                            );
      return -1;
   }

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_somo->setEnabled(false);

   bd_anaflex_enables(false);

   pb_grid_pdb->setEnabled(false);
   pb_grid->setEnabled(false);
   options_log = "";
   append_options_log_somo();
   display_default_differences();
   model_vector = model_vector_as_loaded;

   bead_model_suffix = getExtendedSuffix(false, true);
   le_bead_model_suffix->setText(bead_model_suffix);
   if ( !overwrite )
   {
      setSomoGridFile(true);
   }

   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      bd_anaflex_enables( ( ( browflex && browflex->isRunning() ) ||
                            ( anaflex && anaflex->isRunning() ) ) ? false : true );
      pb_somo->setEnabled(true);
      pb_grid_pdb->setEnabled(true);
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
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }
   bool any_errors = false;
   bool any_models = false;
   somo_processed.resize(lb_model->numRows());
   bead_models.resize(lb_model->numRows());
   QString msg = QString("\n%1 models selected:").arg(project);
   for(int i = 0; i < lb_model->numRows(); i++) {
      somo_processed[i] = 0;
      if (lb_model->isSelected(i)) {
         current_model = i;
         // msg += QString( " %1" ).arg( i + 1 );
         msg += " " + model_name( i );
      }
   }
   msg += "\n";
   editor->append(msg);

   for (current_model = 0; current_model < (unsigned int)lb_model->numRows(); current_model++)
   {
      if (!any_errors && lb_model->isSelected(current_model))
      {
         any_models = true;
         if(!compute_asa())
         {
            somo_processed[current_model] = 1;
            if (asa.recheck_beads)
            {
               // puts("recheck beads disabled");
               editor->append("Rechecking beads\n");
               qApp->processEvents();

               bead_check(false, false);
               editor->append("Finished rechecking beads\n");
               progress->setProgress(19);
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
         bd_anaflex_enables( ( ( browflex && browflex->isRunning() ) ||
                               ( anaflex && anaflex->isRunning() ) ) ? false : true );
         pb_grid_pdb->setEnabled(true);
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
      pb_calc_hydro->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_somo->setEnabled(true);
   pb_grid_pdb->setEnabled(true);
   pb_grid->setEnabled(true);
   pb_stop_calc->setEnabled(false);
   if (calcAutoHydro)
   {
      calc_hydro();
   } 
   else
   {
      play_sounds(1);
   }
   return 0;
}

int US_Hydrodyn::calc_grid_pdb()
{
   //    if ( selected_models_contain( "WAT" ) )
   //    {
   //       QMessageBox::warning( this,
   //                             tr( "Selected model contains WAT residue" ),
   //                             tr( 
   //                                "Can not process models that contain the WAT residue.\n"
   //                                "These are currently generated only for SAXS/SANS computations"
   //                                )
   //                             );
   //       return -1;
   //    }

   if ( selected_models_contain( "XHY" ) )
   {
      QMessageBox::warning( this,
                            tr( "Selected model contains XHY residue" ),
                            tr( 
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
                                   tr( msg ) );

         display_default_differences();
      }
   }

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   options_log = "";
   append_options_log_atob();
   display_default_differences();
   model_vector = model_vector_as_loaded;
   int flag = 0;
   bool any_errors = false;
   bool any_models = false;
   pb_grid->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
   pb_somo->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);

   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }

   bead_model_suffix = getExtendedSuffix(false, false);
   le_bead_model_suffix->setText(bead_model_suffix);
   if ( !overwrite )
   {
      setSomoGridFile(false);
   }

   for (current_model = 0; current_model < (unsigned int)lb_model->numRows(); current_model++) {
      if (lb_model->isSelected(current_model)) {
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
               pb_somo->setEnabled(true);
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
            progress->setTotalSteps(mpos);
            progress->setProgress(progress->progress() + 1);
            int retval = create_beads(&error_string);
            progress->setProgress(progress->progress() + 1);
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_somo->setEnabled(true);
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
                  bead_model.clear();
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
                                     , this_atom->name.ascii()
                                     , this_atom->resName.ascii()
                                     , this_atom->resSeq.ascii()
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

                  progress->setProgress( progress->progress() + 1 );
                  int save_progress = progress->progress();
                  int save_total_steps = progress->totalSteps();
                  if ( grid.center == 2  && any_zero_si ) // ssi
                  {
                     editor_msg( "red", "Center of scattering requested, but zero scattering intensities are present" );
                     pb_grid_pdb->setEnabled(true);
                     pb_somo->setEnabled(true);
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
                  progress->setTotalSteps( save_total_steps );
                  progress->setProgress( save_progress + 1 );
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid_pdb->setEnabled(true);
                     pb_somo->setEnabled(true);
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
                     pb_somo->setEnabled(true);
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
                     progress->setProgress(progress->progress() + 1);
                     bead_check(true, true);
                     progress->setProgress(progress->progress() + 1);
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
                     progress->setProgress(progress->progress() + 1);

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
                        progress->setProgress(progress->progress() + 1);
                        bead_check(false, false);
                        progress->setProgress(progress->progress() + 1);
                        asa.threshold = save_threshold;
                        asa.threshold_percent = save_threshold_percent;
                        bead_models[current_model] = bead_model;
                     }

                  }
                  else
                  {
                     if (grid_overlap.remove_overlap)
                     {
                        progress->setProgress(progress->progress() + 1);
                        radial_reduction( true );
                        progress->setProgress(progress->progress() + 1);
                        bead_models[current_model] = bead_model;
                     }
                     if (stopFlag)
                     {
                        editor->append("Stopped by user\n\n");
                        pb_grid_pdb->setEnabled(true);
                        pb_somo->setEnabled(true);
                        progress->reset();
                        grid_exposed_overlap = org_grid_exposed_overlap;
                        grid_overlap         = org_grid_overlap;
                        grid_buried_overlap  = org_grid_buried_overlap;
                        grid                 = org_grid;
                        return -1;
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
                        progress->setProgress(progress->progress() + 1);
                        bead_check(false, false);
                        progress->setProgress(progress->progress() + 1);
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
                        pb_somo->setEnabled(true);
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
                        extra_text += "BSAXS:: " + sf_bead_factors[ k ].saxs_name.upper();
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

                     extra_text += "\nSAXS:: " + sf_factors.saxs_name.upper();
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
                                    QString( tr( "Notice: saxs coefficients for %1 replaced by newly loaded values\n" ) )
                                    .arg( sf_factors.saxs_name ) );
                     } else {
                        saxs_util->saxs_list.push_back( sf_factors );
                        editor_msg( "dark blue", 
                                    QString( tr( "Notice: added coefficients for %1 from newly loaded values\n" ) )
                                    .arg( sf_factors.saxs_name ) );
                     }
                     saxs_util->saxs_map[ sf_factors.saxs_name ] = sf_factors;

                     if ( saxs_plot_widget )
                     {
                        if ( saxs_plot_window->saxs_map.count( sf_factors.saxs_name ) )
                        {
                           editor_msg( "dark red", 
                                       QString( tr( "Notice: saxs window: saxs coefficients for %1 replaced by newly loaded values\n" ) )
                                       .arg( sf_factors.saxs_name ) );
                        } else {
                           saxs_plot_window->saxs_list.push_back( sf_factors );
                           editor_msg( "dark blue", 
                                       QString( tr( "Notice: saxs window: added coefficients for %1 from newly loaded values\n" ) )
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
                                    QString( tr( "Overriding setting to use global structure factors since bead model doesn't contain the correct number of structure factors (%1) for the beads (%2)" ) )
                                    .arg( sf_bead_factors.size() )
                                    .arg( bead_model.size() )
                                    );
                     }
                     if ( saxs_options.iq_global_avg_for_bead_models || sf_bead_factors.size() != bead_model.size() )
                     {
                        if ( !saxs_util->saxs_map.count( saxs_options.dummy_saxs_name ) )
                        {
                           editor_msg( "red", QString( tr("Warning: No '%1' SAXS atom found.\n" ) )
                                       .arg( saxs_options.dummy_saxs_name ) );
                           for(unsigned int i = 0; i < bead_model.size(); i++) {
                              bead_model[i].saxs_data.saxs_name = "";
                           }
                        } else {
                           editor_msg( "blue", QString( tr("Notice: Loading beads with saxs coefficients '%1'" ) )
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
                           editor_msg( "blue", tr("Notice: Loading beads with bead computed structure factors" ) );
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
                  progress->setProgress(progress->progress() + 1);

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
            progress->setProgress(progress->totalSteps());
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
      pb_somo->setEnabled(true);
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
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_grid_pdb->setEnabled(true);
   pb_grid->setEnabled(true);
   pb_somo->setEnabled(true);
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
   pb_grid->setEnabled(false);
   pb_somo->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
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

   for (current_model = 0; current_model < (unsigned int)lb_model->numRows(); current_model++) {
      if (lb_model->isSelected(current_model)) {
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
            progress->setTotalSteps(mpos);
            progress->setProgress(progress->progress() + 1);
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
                  pb_somo->setEnabled(true);
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
            progress->setProgress(progress->progress() + 1);

            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_somo->setEnabled(true);
               progress->reset();
               return -1;
            }
            if (errorFlag)
            {
               editor->append("Error occured\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_somo->setEnabled(true);
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
               progress->setProgress(progress->progress() + 1);
               bead_check(true, true);
               progress->setProgress(progress->progress() + 1);
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
               progress->setProgress(progress->progress() + 1);
               radial_reduction( true );
               progress->setProgress(progress->progress() + 1);
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
                  progress->setProgress(progress->progress() + 1);
                  bead_check(false, false);
                  progress->setProgress(progress->progress() + 1);
                  asa.threshold = save_threshold;
                  asa.threshold_percent = save_threshold_percent;
                  bead_models[current_model] = bead_model;
               }
            }
            else
            {
               if (grid_overlap.remove_overlap)
               {
                  progress->setProgress(progress->progress() + 1);
                  radial_reduction( true );
                  progress->setProgress(progress->progress() + 1);
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid_pdb->setEnabled(true);
                     pb_somo->setEnabled(true);
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
                     pb_somo->setEnabled(true);
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
                  progress->setProgress(progress->progress() + 1);
                  bead_check(false, false);
                  progress->setProgress(progress->progress() + 1);
                  asa.threshold = save_threshold;
                  asa.threshold_percent = save_threshold_percent;
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid_pdb->setEnabled(true);
                     pb_somo->setEnabled(true);
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
            progress->setProgress(progress->progress() + 1);
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

            progress->setProgress(progress->totalSteps());
         }
      }
   }

   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_grid_pdb->setEnabled(true);
      pb_somo->setEnabled(true);
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
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_grid_pdb->setEnabled(grid_pdb_state);
   pb_grid->setEnabled(true);
   pb_somo->setEnabled(somo_state);
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

   QString filename = Q3FileDialog::getOpenFileName(use_dir, "*.asa_res *.ASA_RES", this);
   if (!filename.isEmpty())
   {
      path_view_asa_res = QFileInfo(filename).dirPath(true);
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
                                    tr("View Bead Model File"),
                                    QString(tr("View last read bead model ") + fi.fileName() + " ?"),
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

            filename = Q3FileDialog::getOpenFileName(use_dir
                                                    ,"Bead models (*.bead_model *.BEAD_MODEL);;"
                                                    "BEAMS (*.beams *.BEAMS);;"
                                                    "DAMMIN/DAMMIF (*.pdb)"
                                                    , this
                                                    , "open file dialog"
                                                    , "Open"
                                                    , &bead_model_selected_filter
                                                    );
            if ( !filename.isEmpty() )
            {
               path_view_bead_model = QFileInfo(filename).dirPath(true);
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

      filename = Q3FileDialog::getOpenFileName(use_dir
                                              ,"Bead models (*.bead_model *.BEAD_MODEL);;"
                                              "BEAMS (*.beams *.BEAMS);;"
                                              "DAMMIN/DAMMIF (*.pdb)"
                                              , this
                                              , "open file dialog"
                                              , "Open"
                                              , &bead_model_selected_filter
                                              );
      if ( !filename.isEmpty() )
      {
         path_view_bead_model = QFileInfo(filename).dirPath(true);
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

   for (current_model = 0; current_model < (unsigned int)lb_model->numRows(); current_model++) {
      if (lb_model->isSelected(current_model)) {
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
            QStringList argument;
#if !defined(WIN32) && !defined(MAC)
            // maybe we should make this a user defined terminal window?
            if ( !movie_frame )
            {
               argument.append("xterm");
               argument.append("-e");
            }
#endif
#if defined(BIN64)
            argument.append(USglobal->config_list.system_dir + "/bin64/rasmol");
#else
            argument.append(USglobal->config_list.system_dir + "/bin/rasmol");
#endif
            if ( !movie_frame )
            {
               argument.append("-script");
               argument.append( spt_name + ".spt" ); 
            }
         
            rasmol->setWorkingDirectory(use_dir);

            rasmol->setArguments(argument);

            if ( movie_frame )
            {
               if (!rasmol->launch(last_spt_text))
               {
                  QMessageBox::message(tr("Please note:"), tr("There was a problem starting RASMOL\n"
                                                              "Please check to make sure RASMOL is properly installed..."));
                  return;
               }
               cout << "movie text [" << last_spt_text << "]\n";
               //               rasmol->writeToStdin(last_spt_text);
               //               rasmol->closeStdin();
               //               qApp->processEvents();
               while ( rasmol && rasmol->isRunning() )
               {
#if defined(WIN32) && !defined( MINGW )
                  _sleep(1);
#else
                  usleep(1000);
#endif
                  qApp->processEvents();
               }
            } else {
               if (!rasmol->start())
               {
                  QMessageBox::message(tr("Please note:"), tr("There was a problem starting RASMOL\n"
                                                              "Please check to make sure RASMOL is properly installed..."));
                  return;
               }
            }
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
   if ( misc.hydro_supc )
   {
      return do_calc_hydro();
   }
   if ( misc.hydro_zeno )
   {
      return (int)(!calc_zeno());
   }
   return -1;
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
   for (current_model = 0; current_model < (unsigned int)lb_model->numRows(); current_model++) {
      if (lb_model->isSelected(current_model)) {
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

   chdir(somo_dir);

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
                                              DOTSOMO + ".beams").ascii(),
                                      QString(project +
                                              (bead_model_from_file ? "" : QString("_%1").arg( model_name( first_model_no - 1 ) ) ) +
                                              QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
                                              DOTSOMO + ".beams").ascii(),
                                      model_names,
                                      progress,
                                      editor,
                                      this);
   chdir(somo_tmp_dir);
   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_calc_hydro->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
      return -1;
   }

   printf("back from supc retval %d\n", retval);
   pb_show_hydro_results->setEnabled(retval ? false : true);
   pb_calc_hydro->setEnabled(true);
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
            printError("US_HYDRODYN_SUPC encountered a file not found error");
            return retval;
            break;
         }
      case US_HYDRODYN_SUPC_OVERLAPS_EXIST:
         {
            printError("US_HYDRODYN_SUPC encountered an overlaps in the bead model error");
            return retval;
            break;
         }
      case US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC:
         {
            printError("US_HYDRODYN_SUPC encountered a memory allocation error");
            return retval;
            break;
         }
      case US_HYDRODYN_SUPC_NO_SEL_MODELS:
         {
            printError("US_HYDRODYN_SUPC was called with no processed models selected");
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
            printError("US_HYDRODYN_SUPC encountered an unknown error");
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
                                    tr("Open Hydrodynamic Calculations File"),
                                    QString(tr("View last results file ") + fi.fileName() + " ?"),
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

            filename = Q3FileDialog::getOpenFileName(use_dir, "*.hydro_res *.HYDRO_RES *.zno *.ZNO", this);

            if ( !filename.isEmpty() )
            {
               path_open_hydro_res = QFileInfo(filename).dirPath(true);
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

      filename = Q3FileDialog::getOpenFileName(use_dir, "*.hydro_res *.HYDRO_RES", this);

      if ( !filename.isEmpty() )
      {
         path_open_hydro_res = QFileInfo(filename).dirPath(true);
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
   online_help->show_help("manual/somo.html");
}

void US_Hydrodyn::stop_calc()
{
   stopFlag = true;
   anaflex_return_to_bd_load_results = false;
   if ( browflex && browflex->isRunning() )
   {
      browflex->tryTerminate();
      QTimer::singleShot( 1000, browflex, SLOT( kill() ) );
   }
   if ( anaflex && anaflex->isRunning() )
   {
      anaflex->tryTerminate();
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
   fn = Q3FileDialog::getSaveFileName(QString::null, QString::null,this );
   if(!fn.isEmpty() )
   {
      QString text = editor->text();
      QFile f( fn );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         return;
      }
      Q3TextStream t( &f );
      t << text;
      f.close();
      editor->setModified( false );
      setCaption( fn );
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
      Q3PaintDeviceMetrics metrics( &printer ); // need width/height
      // of printer surface
      for( int i = 0 ; i < editor->lines() ; i++ ) {
         if ( MARGIN + yPos > metrics.height() - MARGIN ) {
            printer.newPage();      // no more room on this page
            yPos = 0;         // back to top of page
         }
         p.drawText( MARGIN, MARGIN + yPos,
                     metrics.width(), fm.lineSpacing(),
                     ExpandTabs | DontClip,
                     editor->text( i ) );
         yPos = yPos + fm.lineSpacing();
      }
      p.end();            // send job to printer
   }
#endif
}

void US_Hydrodyn::clear_display()
{
   editor->clear();
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
   for ( unsigned int i = 0; i < (unsigned int)lb_model->numRows(); i++ ) 
   {
      if ( lb_model->isSelected(i) ) 
      {
         selected_models.push_back(i);
      }
   }
   if ( selected_models.size() > 1 )
   {
      QMessageBox::message(tr("Please note:"),
                           tr("You must select exactly one model to perform SAXS functions."));
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
                                              tr("UltraScan Notice"),
                                              QString(tr("Please note:\n\n"
                                                         "You have remembered a molecular weight of %1 Daltons\n"
                                                         "but the loaded pdb has a computed molecular weight of %2 Daltons\n"
                                                         "What would you like to do?\n"))
                                              .arg(dammix_remember_mw[QFileInfo(filename).fileName()])
                                              .arg(model_vector[selected_models[0]].mw)
                                              ,
                                              tr("&Set to the newly computed value"),
                                              tr("&Keep the remembered value"), 
                                              tr("&Enter it manually later"),
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
   for ( unsigned int i = 0; i < (unsigned int)lb_model->numRows(); i++ ) 
   {
      if ( lb_model->isSelected(i) ) 
      {
         selected_models.push_back(i);
      }
   }
   if ( selected_models.size() != 1 )
   {
      QMessageBox::message(tr("Please note:"),
                           tr("You must select exactly one model to perform SAXS functions.."));
   } 
   else
   {
      QString filename = QFileInfo(bead_model_file).baseName( TRUE ) +
         QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "");
      if ( !QFileInfo(bead_model_file).baseName( TRUE ).length() )
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
                                           tr("UltraScan Notice"),
                                           QString(tr("Please note:\n\n"
                                                      "You have remembered a molecular weight of %1 Daltons\n"
                                                      "but the loaded bead model a computed molecular weight of %2 Daltons\n"
                                                      "What would you like to do?\n"))
                                           .arg(dammix_remember_mw[QFileInfo(filename).fileName()])
                                           .arg(tmp_mw)
                                           ,
                                           tr("&Set to the newly computed value"),
                                           tr("&Keep the remembered value"), 
                                           tr("&Enter it manually later"),
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

QString US_Hydrodyn::getExtendedSuffix(bool prerun, bool somo)
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

      if ( asa.calculation )
      {
         result += QString("A%1").arg(somo ? asa.threshold : asa.grid_threshold);
      }
      
      if ( asa.recheck_beads )
      {
         result += QString("R%1").arg(somo ? asa.threshold_percent : asa.grid_threshold_percent);
      }
      
      if ( somo ) 
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
      if ( !somo ) 
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
      for( int i = 0; i < lb_model->numRows(); i++ )
      {
         if ( lb_model->isSelected(i) )
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
      for( int i = 0; i < lb_model->numRows(); i++ )
      {
         if ( lb_model->isSelected(i) )
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
               // frmc = fopen(QString("%1.rmc").arg(fname).ascii(), "w");
               // frmc1 = fopen(QString("%1.rmc1").arg(fname).ascii(), "w");
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
   QString path = fi.dirPath() + QDir::separator();
   QString base = fi.baseName(true);
   QString ext = fi.extension(false);
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
         if ( rx.search(*base) != -1 ) 
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
   QString path = fi.dirPath() + QDir::separator();
   QString base = fi.baseName(true);
   QString ext = fi.extension(false);
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
         if ( rx.search(*base) != -1 ) 
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
                                tr( "You must load a PDB file before Run DMD" ) );
      return;
   }
      
   // save current batch_files
   batch_info save_batch_info = batch;
   batch.file.clear();
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
      batch_window->lb_files->setSelected( 0, true );
   } else {
      batch_window->lb_files->clear();
      batch_window->lb_files->insertItem(batch.file[0]);
      batch_window->lb_files->setSelected( 0, true );
   }
   batch_window->cb_mm_first->setChecked( !batch.mm_all );
   batch_window->cb_mm_all  ->setChecked( batch.mm_all );
   batch_window->cb_dmd     ->setChecked( batch.dmd );
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
      batch_window->lb_files->clear();
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
               batch_window->lb_files->insertItem(batch.file[i]);
            } else {
               load_errors += QString(tr("File skipped: %1 (already in list)\n")).arg(batch.file[i]);
            }
         } else {
            load_errors += QString(tr("File skipped: %1 (not a valid file name)\n")).arg(batch.file[i]);
         }
      }
      if ( load_errors != "" ) 
      {
         QColor save_color = batch_window->editor->color();
         batch_window->editor->setColor("dark red");
         batch_window->editor->append(load_errors);
         batch_window->editor->setColor(save_color);
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
   Q3Process* process = new Q3Process( this );
   process->setCommunication( 0 );
#ifndef Q_WS_MAC
   process->addArgument( "us_config" );
#else
   QString procbin = USglobal->config_list.system_dir + "/bin/" + "us_config";
   QString procapp = procbin + ".app";

   if ( !QFile( procapp ).exists()  &&  QFile( procbin ).exists() )
      procapp         = procbin;

   process->addArgument( "open" );
   process->addArgument( "-a" );
   process->addArgument( procapp );
#endif

   if ( ! process->start() )
   {
      QMessageBox::information( this,
                                tr( "Error" ),
                                tr( "There was a problem creating a subprocess\n"
                                    "for " ) + QString("us_config").upper() );
   }
}

void US_Hydrodyn::run_us_admin()
{
   Q3Process* process = new Q3Process( this );
   process->setCommunication( 0 );
#ifndef Q_WS_MAC
   process->addArgument( "us_admin" );
#else
   QString procbin = USglobal->config_list.system_dir + "/bin/" + "us_admin";
   QString procapp = procbin + ".app";

   if ( !QFile( procapp ).exists()  &&  QFile( procbin ).exists() )
      procapp         = procbin;

   process->addArgument( "open" );
   process->addArgument( "-a" );
   process->addArgument( procapp );
#endif

   if ( ! process->start() )
   {
      QMessageBox::information( this,
                                tr( "Error" ),
                                tr( "There was a problem creating a subprocess\n"
                                    "for " ) + QString("us_admin").upper() );
   }
}


void US_Hydrodyn::update_enables()
{
}

void US_Hydrodyn::sizeArrows( QwtCounter* 
#if defined(QT4) && ( defined(Q_WS_MAC) || defined(Q_WS_WIN) )
                              counter 
#endif
)
{
#if defined(QT4) && ( defined(Q_WS_MAC) || defined(Q_WS_WIN) )
   QList< QObject* > children = counter->children();
   QStyle* btnstyle = new QPlastiqueStyle();
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
#if defined(QT4)
                                 widg 
#endif
                                 )
{
#if defined(QT4)
   QList< QObject* > children = widg->children();
   QStyle* btnstyle = new QPlastiqueStyle();
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
   pb_grid->setEnabled(false);
   pb_somo->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
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
         dR = QInputDialog::getDouble(
                                      "dR for equi grid:",
                                      tr( "Enter a cube side value in Angstroms:" ),
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
            pb_equi_grid_bead_model->setEnabled(true);
            pb_somo->setEnabled(true);
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

   for (current_model = 0; current_model < (unsigned int)lb_model->numRows(); current_model++) {
      if (lb_model->isSelected(current_model)) {
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
               pb_equi_grid_bead_model->setEnabled(true);
               pb_somo->setEnabled(true);
               progress->reset();
               return false;
            }
               
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_equi_grid_bead_model->setEnabled(true);
               pb_somo->setEnabled(true);
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
      pb_somo->setEnabled(true);
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
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_equi_grid_bead_model->setEnabled(true);
   pb_grid_pdb->setEnabled(grid_pdb_state);
   pb_grid->setEnabled(true);
   pb_somo->setEnabled(somo_state);
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
