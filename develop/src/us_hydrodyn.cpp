// (this) us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_other.cpp contains other routines such as file i/o

// includes and defines need cleanup

#include "../include/us_hydrodyn.h"
#include "../include/us_surfracer.h"
#include "../include/us_hydrodyn_supc.h"
#include "../include/us_hydrodyn_pat.h"
#include "../include/us_hydrodyn_asab1.h"
#include "../include/us_hydrodyn_grid_atob.h"
#include "../include/us_revision.h"
#include <qregexp.h>
#include <qfont.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

#define GRID_HYDRATE_DEBUG

// static bool no_rr;

US_Hydrodyn::US_Hydrodyn(QWidget *p, const char *name) : QFrame(p, name)
{
   USglobal = new US_Config();

   // no_rr = false;

   // int r_stdout = __open(QString(somo_tmp_dir +
   //           SLASH + "last_stdout.txt").ascii(),
   //        O_WRONLY | O_CREAT | O_TRUNC, 0666);
   // dup2(r_stdout, STDOUT_FILENO);

   // int r_stderr = __open(QString(somo_tmp_dir +
   //           SLASH + "last_stderr.txt").ascii(),
   //        O_WRONLY | O_CREAT | O_TRUNC, 0666);
   // dup2(r_stderr, STDERR_FILENO);

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

   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
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
   set_default();   // setup configuration defaults before reading initial config
   read_config(""); // specify default configuration by leaving argument empty
   atom_widget = false;
   residue_widget = false;
   hybrid_widget = false;
   saxs_widget = false;
   saxs_options_widget = false;
   saxs_plot_widget = false;
   asa_widget = false;
   misc_widget = false;
   grid_widget = false;
   hydro_widget = false;
   overlap_widget = false;
   grid_overlap_widget = false;
   bead_output_widget = false;
   results_widget = false;
   pdb_visualization_widget = false;
   pdb_parsing_widget = false;
   advanced_config_widget = false;
   calcAutoHydro = false;
   residue_filename = USglobal->config_list.system_dir + "/etc/somo.residue";
   editor = (QTextEdit *)0;
   read_residue_file();
   setupGUI();
   //   global_Xpos += 30;
   //   global_Ypos += 30;
   //   setGeometry(global_Xpos, global_Ypos, 0, 0);
   create_beads_normally = true;
   alt_method = false;
   rasmol = NULL;
   chdir(somo_tmp_dir);
   printf("%s\n", QString(somo_tmp_dir).ascii());
   results.total_beads = 0;
   results.used_beads = 0;
   results.mass = 0.0;
   results.s20w = 0.0;
   results.D20w = 0.0;
   results.viscosity = 0.0;
   results.rs = 0.0;
   results.rg = 0.0;
   results.vbar = 0.72;
   results.tau = 0.0;
   results.s20w_sd = 0.0;
   results.D20w_sd = 0.0;
   results.viscosity_sd = 0.0;
   results.rs_sd = 0.0;
   results.rg_sd = 0.0;
   results.tau_sd = 0.0;
   results.asa_rg_pos = 0.0;
   results.asa_rg_neg = 0.0;
   rasmol = new QProcess(this);
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
   editor->append(QString(tr("\n\nWelcome to UltraScan %1 revision %2 SOMO\n")
                          // "somo dir is <%3>\n"
                          // "somo pdb dir is <%4>\n"
			  // "somo tmp is <%5>\n"
			  // "somo saxs is <%6>\n"
			  )
		  .arg(US_Version)
		  .arg(REVISION)
		  // .arg(somo_dir)
		  // .arg(somo_pdb_dir)
		  // .arg(somo_tmp_dir)
		  // .arg(somo_saxs_dir)
		  );
}

US_Hydrodyn::~US_Hydrodyn()
{
}

void US_Hydrodyn::setupGUI()
{
   int minHeight1 = 30;
   bead_model_file = "";

   lookup_tables = new QPopupMenu;
   lookup_tables->insertItem(tr("Add/Edit &Hybridization"), this, SLOT(hybrid()));
   lookup_tables->insertItem(tr("Add/Edit &Atom"), this, SLOT(edit_atom()));
   lookup_tables->insertItem(tr("Add/Edit &Residue"), this, SLOT(residue()));
   lookup_tables->insertItem(tr("Add/Edit &SAXS coefficients"), this, SLOT(saxs()));

   somo_options = new QPopupMenu;
   somo_options->insertItem(tr("&ASA Calculation"), this, SLOT(show_asa()));
   somo_options->insertItem(tr("&SoMo Overlap Reduction"), this, SLOT(show_overlap()));
   somo_options->insertItem(tr("AtoB (Grid) &Overlap Reduction"), this, SLOT(show_grid_overlap()));
   somo_options->insertItem(tr("&Hydrodynamic Calculations"), this, SLOT(show_hydro()));
   somo_options->insertItem(tr("&Miscellaneous Options"), this, SLOT(show_misc()));
   somo_options->insertItem(tr("&Bead Model Output"), this, SLOT(show_bead_output()));
   somo_options->insertItem(tr("&Grid Functions (AtoB)"), this, SLOT(show_grid()));
   somo_options->insertItem(tr("SA&XS Options"), this, SLOT(show_saxs_options()));

   pdb_options = new QPopupMenu;
   pdb_options->insertItem(tr("&Parsing"), this, SLOT(pdb_parsing()));
   pdb_options->insertItem(tr("&Visualization"), this, SLOT(pdb_visualization()));

   configuration = new QPopupMenu;
   configuration->insertItem(tr("&Load Configuration"), this, SLOT(load_config()));
   configuration->insertItem(tr("&Save Current Configuration"), this, SLOT(write_config()));
   configuration->insertItem(tr("&Reset to Default Configuration"), this, SLOT(reset()));
   configuration->insertItem(tr("&Advanced Configuration"), this, SLOT(show_advanced_config()));

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);

   menu = new QMenuBar(frame);
   menu->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   menu->insertItem(tr("&Lookup Tables"), lookup_tables);
   menu->insertItem(tr("&SOMO Options"), somo_options);
   menu->insertItem(tr("&PDB Options"), pdb_options);
   menu->insertItem(tr("&Configurations"), configuration);

   lbl_info1 = new QLabel(tr("PDB Functions:"), this);
   Q_CHECK_PTR(lbl_info1);
   lbl_info1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info1->setAlignment(AlignCenter|AlignVCenter);
   lbl_info1->setMinimumHeight(minHeight1);
   lbl_info1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_info2 = new QLabel(tr("Bead Model Functions:"), this);
   Q_CHECK_PTR(lbl_info2);
   lbl_info2->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info2->setAlignment(AlignCenter|AlignVCenter);
   lbl_info2->setMinimumHeight(minHeight1);
   lbl_info2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_info3 = new QLabel(tr("Hydrodynamic Calculations:"), this);
   Q_CHECK_PTR(lbl_info3);
   lbl_info3->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info3->setAlignment(AlignCenter|AlignVCenter);
   lbl_info3->setMinimumHeight(minHeight1);
   lbl_info3->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   pb_select_residue_file = new QPushButton(tr("Select Lookup Table"), this);
   Q_CHECK_PTR(pb_select_residue_file);
   pb_select_residue_file->setMinimumHeight(minHeight1);
   pb_select_residue_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_residue_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_residue_file, SIGNAL(clicked()), SLOT(select_residue_file()));

   lbl_table = new QLabel( QDir::convertSeparators( residue_filename ), this );
   lbl_table->setMinimumHeight(minHeight1);
   lbl_table->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_table->setAlignment(AlignCenter|AlignVCenter);
   lbl_table->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   pb_load_pdb = new QPushButton(tr("Load PDB File"), this);
   Q_CHECK_PTR(pb_load_pdb);
   pb_load_pdb->setMinimumHeight(minHeight1);
   pb_load_pdb->setEnabled(true);
   pb_load_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_pdb->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_pdb, SIGNAL(clicked()), SLOT(load_pdb()));

   lbl_pdb_file = new QLabel(tr(" not selected"),this);
   lbl_pdb_file->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_pdb_file->setAlignment(AlignCenter|AlignVCenter);
   lbl_pdb_file->setMinimumHeight(minHeight1);
   lbl_pdb_file->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_pdb_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_model = new QLabel(tr(" Please select a PDB Structure:"), this);
   Q_CHECK_PTR(lbl_model);
   lbl_model->setAlignment(AlignLeft|AlignVCenter);
   lbl_model->setMinimumHeight(minHeight1);
   lbl_model->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   pb_view_pdb = new QPushButton(tr("View/Edit PDB File"), this);
   Q_CHECK_PTR(pb_view_pdb);
   pb_view_pdb->setMinimumHeight(minHeight1);
   pb_view_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_view_pdb->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_view_pdb, SIGNAL(clicked()), SLOT(view_pdb()));

   lb_model = new QListBox(this, "model selection listbox" );
   lb_model->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   lb_model->setEnabled(false);
   lb_model->setSelectionMode(QListBox::Extended);
   lb_model->setHScrollBarMode(QScrollView::Auto);
   lb_model->setVScrollBarMode(QScrollView::Auto);
   connect(lb_model, SIGNAL(selected(int)), this, SLOT(select_model(int)));

   pb_load_bead_model = new QPushButton(tr("Load Bead Model File"), this);
   Q_CHECK_PTR(pb_load_bead_model);
   pb_load_bead_model->setMinimumHeight(minHeight1);
   pb_load_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_bead_model->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_bead_model, SIGNAL(clicked()), SLOT(load_bead_model()));

   le_bead_model_file = new QLineEdit(this, "bead_model_file Line Edit");
   le_bead_model_file->setText(tr(" not selected "));
   le_bead_model_file->setMinimumHeight(minHeight1);
   le_bead_model_file->setAlignment(AlignCenter|AlignVCenter);
   le_bead_model_file->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_bead_model_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_bead_model_file, SIGNAL(textChanged(const QString &)), SLOT(update_bead_model_file(const QString &)));

   lbl_bead_model_prefix = new QLabel(tr(" Bead Model Prefix:"), this);
   Q_CHECK_PTR(lbl_bead_model_prefix);
   lbl_bead_model_prefix->setAlignment(AlignLeft|AlignVCenter);
   lbl_bead_model_prefix->setMinimumHeight(minHeight1);
   lbl_bead_model_prefix->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bead_model_prefix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_bead_model_prefix = new QLineEdit(this, "bead_model_prefix Line Edit");
   le_bead_model_prefix->setText(tr(""));
   le_bead_model_prefix->setMinimumHeight(minHeight1);
   le_bead_model_prefix->setAlignment(AlignCenter|AlignVCenter);
   le_bead_model_prefix->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_bead_model_prefix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_bead_model_prefix, SIGNAL(textChanged(const QString &)), SLOT(update_bead_model_prefix(const QString &)));

   pb_somo = new QPushButton(tr("Build SoMo Bead Model"), this);
   Q_CHECK_PTR(pb_somo);
   pb_somo->setMinimumHeight(minHeight1);
   pb_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_somo->setEnabled(false);
   pb_somo->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_somo, SIGNAL(clicked()), SLOT(calc_somo()));

   pb_pdb_saxs = new QPushButton(tr("SAXS Functions"), this);
   Q_CHECK_PTR(pb_pdb_saxs);
   pb_pdb_saxs->setMinimumHeight(minHeight1);
   pb_pdb_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_pdb_saxs->setEnabled(false);
   pb_pdb_saxs->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_pdb_saxs, SIGNAL(clicked()), SLOT(pdb_saxs()));

   pb_bead_saxs = new QPushButton(tr("SAXS Functions"), this);
   Q_CHECK_PTR(pb_bead_saxs);
   pb_bead_saxs->setMinimumHeight(minHeight1);
   pb_bead_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_bead_saxs->setEnabled(false);
   pb_bead_saxs->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_bead_saxs, SIGNAL(clicked()), SLOT(bead_saxs()));

   pb_grid_pdb = new QPushButton(tr("Build AtoB (Grid) Bead Model"), this);
   Q_CHECK_PTR(pb_grid_pdb);
   pb_grid_pdb->setMinimumHeight(minHeight1);
   pb_grid_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_grid_pdb->setEnabled(false);
   pb_grid_pdb->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_grid_pdb, SIGNAL(clicked()), SLOT(calc_grid_pdb()));

   pb_grid = new QPushButton(tr("Build Grid Bead Model"), this);
   Q_CHECK_PTR(pb_grid);
   pb_grid->setMinimumHeight(minHeight1);
   pb_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_grid->setEnabled(false);
   pb_grid->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_grid, SIGNAL(clicked()), SLOT(calc_grid()));

   cb_calcAutoHydro = new QCheckBox(this);
   cb_calcAutoHydro->setText(tr(" Automatically Calculate Hydrodynamics "));
   cb_calcAutoHydro->setChecked(calcAutoHydro);
   cb_calcAutoHydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_calcAutoHydro->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_calcAutoHydro, SIGNAL(clicked()), this, SLOT(set_calcAutoHydro()));

   pb_view_asa = new QPushButton(tr("View ASA Results"), this);
   Q_CHECK_PTR(pb_view_asa);
   pb_view_asa->setMinimumHeight(minHeight1);
   pb_view_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_view_asa->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_view_asa, SIGNAL(clicked()), SLOT(view_asa()));

   pb_visualize = new QPushButton(tr("Visualize Bead Model"), this);
   Q_CHECK_PTR(pb_visualize);
   pb_visualize->setMinimumHeight(minHeight1);
   pb_visualize->setEnabled(false);
   pb_visualize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_visualize->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_visualize, SIGNAL(clicked()), SLOT(visualize()));

   pb_calc_hydro = new QPushButton(tr("Calculate Hydrodynamics"), this);
   Q_CHECK_PTR(pb_calc_hydro);
   pb_calc_hydro->setEnabled(false);
   pb_calc_hydro->setMinimumHeight(minHeight1);
   pb_calc_hydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_calc_hydro->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_calc_hydro, SIGNAL(clicked()), SLOT(calc_hydro()));

   pb_show_hydro_results = new QPushButton(tr("Show Hydrodynamic Calculations"), this);
   Q_CHECK_PTR(pb_show_hydro_results);
   pb_show_hydro_results->setMinimumHeight(minHeight1);
   //pb_show_hydro_results->setEnabled(true);
   pb_show_hydro_results->setEnabled(false);
   pb_show_hydro_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_show_hydro_results->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_show_hydro_results, SIGNAL(clicked()), SLOT(show_hydro_results()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_stop_calc = new QPushButton(tr("Stop"), this);
   Q_CHECK_PTR(pb_stop_calc);
   pb_stop_calc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop_calc->setMinimumHeight(minHeight1);
   pb_stop_calc->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stop_calc, SIGNAL(clicked()), SLOT(stop_calc()));
   pb_stop_calc->setEnabled(false);

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   progress = new QProgressBar(this, "Loading Progress");
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress->reset();

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);
   editor->setMinimumWidth(550);
   m = new QMenuBar(editor, "menu" );
   m->setMinimumHeight(minHeight1);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   file->insertItem( tr("Print"), this, SLOT(print()),   ALT+Key_P );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (advanced_config.scroll_editor ? QTextEdit::NoWrap : QTextEdit::WidgetWidth);

   lbl_core_progress = new QLabel("", this);
   Q_CHECK_PTR(lbl_core_progress);
   lbl_core_progress->setAlignment(AlignHCenter|AlignVCenter);
   lbl_core_progress->setMinimumHeight(minHeight1);
   lbl_core_progress->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_core_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   clear_display();

   int rows=13, columns = 3, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(frame, j, j, 0, 1);
   background->addMultiCellWidget(editor, j, j+16, 2, 2);
   j++;
   background->addMultiCellWidget(lbl_info1, j, j, 0, 1);
   j++;
   background->addWidget(pb_select_residue_file, j, 0);
   background->addWidget(lbl_table, j, 1);
   j++;
   background->addWidget(pb_load_pdb, j, 0);
   background->addWidget(lbl_pdb_file, j, 1);
   j++;
   background->addWidget(lbl_model, j, 0);
   background->addMultiCellWidget(lb_model, j, j+2, 1, 1);
   j++;
   background->addWidget(pb_view_pdb, j, 0);
   j++;
   background->addWidget(pb_pdb_saxs, j, 0);
   j++;
   background->addMultiCellWidget(lbl_info2, j, j, 0, 1);
   j++;
   background->addWidget(lbl_bead_model_prefix, j, 0);
   background->addWidget(le_bead_model_prefix, j, 1);
   j++;
   background->addWidget(pb_somo, j, 0);
   background->addWidget(pb_grid_pdb, j, 1);
   j++;
   background->addWidget(pb_grid, j, 0);
   background->addWidget(cb_calcAutoHydro, j, 1);
   j++;
   background->addWidget(pb_view_asa, j, 0);
   background->addWidget(pb_visualize, j, 1);
   j++;
   background->addWidget(pb_load_bead_model, j, 0);
   background->addWidget(le_bead_model_file, j, 1);
   j++;
   background->addWidget(pb_bead_saxs, j, 0);
   j++;
   background->addMultiCellWidget(lbl_info3, j, j, 0, 1);
   j++;
   background->addWidget(pb_calc_hydro, j, 0);
   background->addWidget(pb_show_hydro_results, j, 1);
   j++;
   background->addWidget(pb_stop_calc, j, 0);
   background->addWidget(pb_cancel, j, 1);
// background->addMultiCellWidget(progress, j, j, 0, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(progress, j, 1);
   background->addWidget(lbl_core_progress, j, 2);
}


void US_Hydrodyn::set_disabled()
{
   pb_somo->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
   pb_grid->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_pdb_saxs->setEnabled(false);
   pb_bead_saxs->setEnabled(false);
   le_bead_model_file->setText(" not selected ");
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
      addResidue->show();
   }
}

void US_Hydrodyn::saxs()
{
   if (saxs_widget)
   {
      addSaxs->raise();
   }
   else
   {
      addSaxs = new US_AddSaxs(&saxs_widget, 0);
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
                                               &grid_exposed_overlap,
                                               &grid_buried_overlap,
                                               &grid_overlap,
                                               &overlap_tolerance,
                                               &overlap_widget,
                                               this);
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
                                                    &overlap_tolerance,
                                                    &grid_overlap_widget,
                                                    this);
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
      saxs_options_window = new US_Hydrodyn_SaxsOptions(&saxs_options, &saxs_options_widget, this);
      saxs_options_window->show();
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
                                         &grid,
                                         &overlap_tolerance,
                                         &grid_widget,
                                         this);
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
      pdb_visualization_window->show();
   }
}

void US_Hydrodyn::load_config()
{
   QString fname = QFileDialog::getOpenFileName ( somo_dir, "*.config", 0, 0, "Please select a SOMO configuration file...", 0, TRUE );
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
   QString fname = QFileDialog::getSaveFileName ( somo_dir, "*.config", 0, 0, "Please name your SOMO configuration file...", 0, TRUE );
   if (fname.right(7) != ".config")
   {
      fname += ".config";
   }
   write_config(fname);
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
   clear_display();
}

void US_Hydrodyn::select_residue_file()
{
   QString old_filename = residue_filename;
   residue_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.residue *.RESIDUE", this);
   if (residue_filename.isEmpty())
   {
      residue_filename = old_filename;
      return;
   }
   read_residue_file();
   set_disabled();
   lbl_table->setText( QDir::convertSeparators( residue_filename ) );
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
                     QMessageBox::NoButton);
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
                     QMessageBox::NoButton);
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
   QString filename = QFileDialog::getOpenFileName(somo_pdb_dir,
                                                   "Structures (*.pdb *.PDB)",
                                                   this,
                                                   "Open Structure Files",
                                                   "Please select a PDB file...");
   int errors_found = 0;
   if (!filename.isEmpty())
   {
      pdb_file = filename;
      options_log = "";
      last_abb_msgs = "";
      bead_model_from_file = false;
      lbl_pdb_file->setText( QDir::convertSeparators( filename ) );
      clear_display();

#if defined(START_RASMOL)
      QStringList argument;
#if !defined(WIN32)
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
      // editor->append(QString("starting rasmol <%1>\n").arg(argument.join("><")));
      // printf("starting rasmol<%s><s>\n", argument.join("><").ascii());
      // fflush(stdout);
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
         editor->append(QString("Checking the pdb structure for model %1\n").arg(i+1));
         if (check_for_missing_atoms(&error_string, &model_vector[i]))
         {
            errors_found++;
            editor->append(QString("Encountered errors with your PDB structure for model %1:\n").
                           arg(i + 1) + error_string);
            printError(QString("Encountered errors with your PDB structure for model %1:\n").
                       arg(i + 1) + "please check the text window");
         }
      }
      model_vector_as_loaded = model_vector;
      editor->append(QString("Loaded pdb file : %1\n").arg(errors_found ? "ERRORS PRESENT" : "ok"));
      bead_models.clear();
      somo_processed.clear();
   }
   else
   {
      return; // user canceled loading PDB file
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
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_bead_saxs->setEnabled(false);
   pb_visualize->setEnabled(false);
   le_bead_model_file->setText(" not selected ");
   bead_models_as_loaded = bead_models;
   if ( lb_model->numRows() == 1 )
   {
      select_model(0);
   }
   if ( advanced_config.auto_calc_somo  &&
        !errors_found )
   {
      calc_somo();
   }
}

void US_Hydrodyn::view_pdb()
{
   QString filename = QFileDialog::getOpenFileName(somo_pdb_dir, "*.pdb *.PDB", this);
   if (!filename.isEmpty())
   {
      view_file(filename);
   }
}

void US_Hydrodyn::select_model(int val)
{
   current_model = val;
   QString msg = QString("\n%1 models selected:").arg(project);
   for(int i = 0; i < lb_model->numRows(); i++)
   {
      if (lb_model->isSelected(i))
      {
         current_model = i;
         msg += QString(" %1").arg(i+1);
      }
   }
   msg += "\n";
   editor->append(msg);

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
   pb_pdb_saxs->setEnabled(true);
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
            float color[3] = { .2f, .2f, ((*model)[i].all_beads.size() / 5) > 1 ? 1 : ((*model)[i].all_beads.size() / 5) };
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
            float color[3] = { .2f, .2f, ((*model)[i].all_beads.size() / 5) > 1 ? 1 : ((*model)[i].all_beads.size() / 5) };
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
   QString filename = QFileDialog::getOpenFileName(somo_dir, QString("*%1.bead_model *%2.beams *%3.BEAD_MODEL *%4.BEAMS")
                                                   .arg(DOTSOMO).arg(DOTSOMO).arg(DOTSOMOCAP).arg(DOTSOMOCAP),
                                                   this);
   if (!filename.isEmpty())
   {
      options_log = "";
      pb_somo->setEnabled(false);
      pb_visualize->setEnabled(false);
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
      if (!read_bead_model(filename))
      {
         pb_visualize->setEnabled(true);
         pb_calc_hydro->setEnabled(true);
         pb_grid->setEnabled(true);
         pb_bead_saxs->setEnabled(true);
         pb_pdb_saxs->setEnabled(false);
      }
      else
      {
         pb_bead_saxs->setEnabled(false);
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
   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_somo->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
   pb_grid->setEnabled(false);
   options_log = "";
   append_options_log_somo();
   display_default_differences();
   model_vector = model_vector_as_loaded;
   if ( bead_model_prefix.contains("a2b") )
   {
      bead_model_prefix.replace("-a2bg","");
      bead_model_prefix.replace("-a2b","");
      bead_model_prefix.replace("a2bg","");
      bead_model_prefix.replace("a2b","");
      le_bead_model_prefix->setText(bead_model_prefix);
   }
   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
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
         msg += QString(" %1").arg(i+1);
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
      pb_calc_hydro->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
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
   stopFlag = false;
   pb_stop_calc->setEnabled(true);
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
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);

   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }

   if ( bead_model_prefix.contains("a2bg") )
   {
      bead_model_prefix.replace("-a2bg","");
      bead_model_prefix.replace("a2bg","");
      le_bead_model_prefix->setText(bead_model_prefix);
   }
   if ( !bead_model_prefix.contains("a2b") )
   {
      if ( bead_model_prefix.length() )
      {
         bead_model_prefix += "-a2b";
      }
      else
      {
         bead_model_prefix += "a2b";
      }
      le_bead_model_prefix->setText(bead_model_prefix);
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
                           bead_model.push_back(*this_atom);
                        }
                     }
                  }
                  if (bead_models.size() < current_model + 1) {
                     bead_models.resize(current_model + 1);
                  }

                  progress->setProgress(progress->progress() + 1);
                  bead_models[current_model] =
                     us_hydrodyn_grid_atob(&bead_model,
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
                     radial_reduction();
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
                        radial_reduction();
                        progress->setProgress(progress->progress() + 1);
                        bead_models[current_model] = bead_model;
                     }
                     if (stopFlag)
                     {
                        editor->append("Stopped by user\n\n");
                        pb_grid_pdb->setEnabled(true);
                        pb_somo->setEnabled(true);
                        progress->reset();
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
                        return -1;
                     }
                  }
                  progress->setProgress(progress->progress() + 1);

                  // write_bead_spt(somo_dir + SLASH + project +
                  //       (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
                  //       QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
                  //       DOTSOMO, &bead_model, bead_model_from_file);
                  write_bead_model(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
                                   QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
                                   DOTSOMO, &bead_model);

               }
            }
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
      pb_calc_hydro->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
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
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }

   if ( !bead_model_prefix.contains("a2bg") )
   {
      if( bead_model_prefix.length() )
      {
         if ( bead_model_prefix.contains(QRegExp("a2b$")) )
         {
            bead_model_prefix += "g";
         }
         else
         {
            bead_model_prefix += "-a2bg";
         }
      }
      else
      {
         bead_model_prefix += "a2bg";
      }
      le_bead_model_prefix->setText(bead_model_prefix);
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
               radial_reduction();
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
                  radial_reduction();
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
            progress->setProgress(progress->progress() + 1);
            // write_bead_spt(somo_dir + SLASH + project +
            //        (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
            //        QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
            //           DOTSOMO, &bead_model, bead_model_from_file);
            write_bead_model(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
                             QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
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
      pb_calc_hydro->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
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

void US_Hydrodyn::view_asa()
{
   QString filename = QFileDialog::getOpenFileName(somo_dir, "*.asa_res *.ASA_RES", this);
   if (!filename.isEmpty())
   {
      view_file(filename);
   }
}

void US_Hydrodyn::visualize()
{
   for (current_model = 0; current_model < (unsigned int)lb_model->numRows(); current_model++) {
      if (lb_model->isSelected(current_model)) {
         if (somo_processed[current_model]) {
            bead_model = bead_models[current_model];
            write_bead_spt(somo_dir + SLASH + project +
                           (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
                           QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
                           DOTSOMO, &bead_model);
            editor->append(QString("Visualizing model %1\n").arg(current_model + 1));
            QStringList argument;
#if !defined(WIN32)
            // maybe we should make this a user defined terminal window?
            argument.append("xterm");
            argument.append("-e");
#endif
#if defined(BIN64)
            argument.append(USglobal->config_list.system_dir + "/bin64/rasmol");
#else
            argument.append(USglobal->config_list.system_dir + "/bin/rasmol");
#endif
            argument.append("-script");
            argument.append(
                            project +
                            (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
                            QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
                            DOTSOMO + ".spt");

            rasmol->setWorkingDirectory(somo_dir);

            rasmol->setArguments(argument);
            if (!rasmol->start())
            {
               QMessageBox::message(tr("Please note:"), tr("There was a problem starting RASMOL\n"
                                                           "Please check to make sure RASMOL is properly installed..."));
               return;
            }
         }
         else
         {
            editor->append(QString("Model %1 - selected but bead model not built\n").arg(current_model + 1));
         }
      }
   }
}

void US_Hydrodyn::calc_hydro()
{
   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_calc_hydro->setEnabled(false);
   puts("calc hydro (supc)");
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
   for (current_model = 0; current_model < (unsigned int)lb_model->numRows(); current_model++) {
      if (lb_model->isSelected(current_model)) {
         if (somo_processed[current_model]) {
            if (!first_model_no) {
               first_model_no = current_model + 1;
            }
            models_to_proc++;
            editor->append(QString("Model %1 will be included\n").arg(current_model + 1));
            bead_model = bead_models[current_model];

            // write_bead_spt(somo_dir + SLASH + project +
            //          (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
            //          QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
            //          DOTSOMO, &bead_model, bead_model_from_file);
         }
         else
         {
            editor->append(QString("Model %1 - selected but bead model not built\n").arg(current_model + 1));
         }
      }
   }

   chdir(somo_dir);

   editor->append(QString("%1")
                  //       .arg(hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance)
                  .arg((fabs((hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance) - overlap_tolerance) > 1e-5)
                       ? QString("\nNOTICE: Overlap reduction bead overlap tolerance %1 does not equal the manually selected hydrodynamic calculations bead overlap cut-off %2\n")
                       .arg(overlap_tolerance).arg(hydro.overlap) : ""));

   qApp->processEvents();
   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_calc_hydro->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
      return;
   }
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
                                              QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
                                              DOTSOMO + ".beams").ascii(),
                                      QString(project +
                                              (bead_model_from_file ? "" : QString("_%1").arg(first_model_no)) +
                                              QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
                                              DOTSOMO + ".beams").ascii(),
                                      progress,
                                      editor,
                                      this);
   chdir(somo_tmp_dir);
   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_calc_hydro->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
      return;
   }

   printf("back from supc retval %d\n", retval);
   pb_show_hydro_results->setEnabled(retval ? false : true);
   pb_calc_hydro->setEnabled(true);
   pb_bead_saxs->setEnabled(true);
   if ( retval )
   {
      editor->append("Calculate hydrodynamics failed\n\n");
      qApp->processEvents();
      switch ( retval )
      {
      case US_HYDRODYN_SUPC_FILE_NOT_FOUND:
         {
            printError("US_HYDRODYN_SUPC encountered a file not found error");
            return;
            break;
         }
      case US_HYDRODYN_SUPC_OVERLAPS_EXIST:
         {
            printError("US_HYDRODYN_SUPC encountered an overlaps in the bead model error");
            return;
            break;
         }
      case US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC:
         {
            printError("US_HYDRODYN_SUPC encountered a memory allocation error");
            return;
            break;
         }
      case US_HYDRODYN_SUPC_NO_SEL_MODELS:
         {
            printError("US_HYDRODYN_SUPC was called with no processed models selected");
            return;
            break;
         }
      case US_HYDRODYN_PAT_ERR_MEMORY_ALLOC:
         {
            printError("US_HYDRODYN_PAT encountered a memory allocation error");
            return;
            break;
         }
      default:
         {
            printError("US_HYDRODYN_SUPC encountered an unknown error");
            // unknown error
            return;
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
}

void US_Hydrodyn::show_hydro_results()
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
      results_window->show();
   }
   puts("show hydro");
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
      advanced_config_window->show();
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
   fn = QFileDialog::getSaveFileName(QString::null, QString::null,this );
   if(!fn.isEmpty() )
   {
      QString text = editor->text();
      QFile f( fn );
      if ( !f.open( IO_WriteOnly | IO_Translate) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
      editor->setModified( false );
      setCaption( fn );
   }
}

void US_Hydrodyn::print()
{
   const int MARGIN = 10;
   printer.setPageSize(QPrinter::Letter);

   if ( printer.setup(this) ) {      // opens printer dialog
      QPainter p;
      p.begin( &printer );         // paint on printer
      p.setFont(editor->font() );
      int yPos      = 0;         // y position for each line
      QFontMetrics fm = p.fontMetrics();
      QPaintDeviceMetrics metrics( &printer ); // need width/height
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
}

void US_Hydrodyn::clear_display()
{
   editor->clear();
   editor->setText("\n");
   display_default_differences();
}

void US_Hydrodyn::pdb_saxs()
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
      QString filename = project + QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "");
      if (saxs_plot_widget)
      {
         saxs_plot_window->refresh(
                                   filename,
                                   residue_list,
                                   model_vector,
                                   bead_models,
                                   selected_models,
                                   multi_residue_map,
                                   residue_atom_hybrid_map,
                                   0
                                   );
         saxs_plot_window->raise();
      }
      else
      {
         saxs_plot_window = new US_Hydrodyn_Saxs(
                                                 &saxs_plot_widget,
                                                 &saxs_options,
                                                 filename,
                                                 residue_list,
                                                 model_vector,
                                                 bead_models,
                                                 selected_models,
                                                 multi_residue_map,
                                                 residue_atom_hybrid_map,
                                                 0, 
                                                 this,
                                                 0
                                                 );
         saxs_plot_window->show();
      }
   }
}

void US_Hydrodyn::bead_saxs()
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
         QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "");
      if ( !filename.length() )
      {
         filename = project + QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "");
      }
      if ( !filename.length() )
      {
         filename = "unknown";
      }
      
      printf("selected models size %u bead_models.size %u\n",
             (unsigned int)bead_models.size(),
             (unsigned int)selected_models.size()
             );
      if (saxs_plot_widget)
      {
         saxs_plot_window->refresh(
                                   filename,
                                   residue_list,
                                   model_vector,
                                   bead_models,
                                   selected_models,
                                   multi_residue_map,
                                   residue_atom_hybrid_map,
                                   1
                                   );
         saxs_plot_window->raise();
      }
      else
      {
         saxs_plot_window = new US_Hydrodyn_Saxs(
                                                 &saxs_plot_widget,
                                                 &saxs_options,
                                                 filename,
                                                 residue_list,
                                                 model_vector,
                                                 bead_models,
                                                 selected_models,
                                                 multi_residue_map,
                                                 residue_atom_hybrid_map,
                                                 1, 
                                                 this,
                                                 0
                                                 );
         saxs_plot_window->show();
      }
   }
}
