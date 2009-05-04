#include "../include/us_hydrodyn.h"
#include "../include/us_surfracer.h"
#include "../include/us_hydrodyn_supc.h"
#include "../include/us_hydrodyn_pat.h"
#include "../include/us_hydrodyn_asab1.h"
#include "../include/us_hydrodyn_grid_atob.h"
#include <qregexp.h>
#include <qfont.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// #define OLD_ASAB1_SC_COMPUTE

// #define DEBUG
// #define DEBUG1
// #define AUTO_BB_DEBUG
#define BUILD_MAPS_DEBUG

#ifndef WIN32
#   include <unistd.h>
#   define TIMING
#   if defined(TIMING)
#      include <sys/time.h>
static struct timeval start_tv, end_tv;
#   endif

#   define SLASH "/"
#    define __open open
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

static bool no_rr;

US_Hydrodyn::US_Hydrodyn(QWidget *p, const char *name) : QFrame(p, name)
{
   USglobal = new US_Config();

   no_rr = false;

   // int r_stdout = __open(QString(somo_tmp_dir +
   //           SLASH + "last_stdout.txt").ascii(),
   //        O_WRONLY | O_CREAT | O_TRUNC, 0666);
   // dup2(r_stdout, STDOUT_FILENO);

   // int r_stderr = __open(QString(somo_tmp_dir +
   //           SLASH + "last_stderr.txt").ascii(),
   //        O_WRONLY | O_CREAT | O_TRUNC, 0666);
   // dup2(r_stderr, STDERR_FILENO);

   somo_dir = USglobal->config_list.root_dir + "/somo";
   QDir dir1(somo_dir);
   if (!dir1.exists())
   {
      dir1.mkdir(somo_dir);
   }
   somo_pdb_dir = somo_dir + "/structures";
   QDir dir2(somo_pdb_dir);
   if (!dir2.exists())
   {
      dir2.mkdir(somo_pdb_dir);
   }
   somo_tmp_dir = somo_dir + "/tmp";
   QDir dir3(somo_tmp_dir);
   if (!dir3.exists())
   {
      dir3.mkdir(somo_tmp_dir);
   }

   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SOMO Solution Bead Modeler"));
   read_config(""); // specify default configuration by leaving argument empty
   atom_widget = false;
   residue_widget = false;
   hybrid_widget = false;
   asa_widget = false;
   misc_widget = false;
   grid_widget = false;
   hydro_widget = false;
   overlap_widget = false;
   bead_output_widget = false;
   results_widget = false;
   pdb_visualization_widget = false;
   pdb_parsing_widget = false;
   calcAutoHydro = false;
   residue_filename = USglobal->config_list.system_dir + "/etc/somo.residue";
   editor = (QTextEdit *)0;
   read_residue_file();
   setupGUI();
   //   global_Xpos += 30;
   //   global_Ypos += 30;
   //   setGeometry(global_Xpos, global_Ypos, 0, 0);
   create_beads_normally = true;
   regular_N_handling = true;
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
   rasmol = new QProcess(this);
   rasmol->setWorkingDirectory(
                               QDir(USglobal->config_list.system_dir +
#if defined(BIN64)
                                    "/bin64/"
#else
                                    "/bin/"
#endif
                                    ));

   bead_model_from_file = false;
   QString RMP = "RASMOLPATH=" + USglobal->config_list.system_dir +
#if defined(BIN64)
      "/bin64/"
#else
      "/bin/"
#endif
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
   lookup_tables->insertItem(tr("Add/Edit &Atom"), this, SLOT(atom()));
   lookup_tables->insertItem(tr("Add/Edit &Residue"), this, SLOT(residue()));

   somo_options = new QPopupMenu;
   somo_options->insertItem(tr("&ASA Calculation"), this, SLOT(show_asa()));
   somo_options->insertItem(tr("&Overlap Reduction"), this, SLOT(show_overlap()));
   somo_options->insertItem(tr("&Hydrodynamic Calculations"), this, SLOT(show_hydro()));
   somo_options->insertItem(tr("&Miscellaneous Options"), this, SLOT(show_misc()));
   somo_options->insertItem(tr("&Bead Model Output"), this, SLOT(show_bead_output()));
   somo_options->insertItem(tr("&Grid Functions (AtoB)"), this, SLOT(show_grid()));
   
   pdb_options = new QPopupMenu;
   pdb_options->insertItem(tr("&Parsing"), this, SLOT(pdb_parsing()));
   pdb_options->insertItem(tr("&Visualization"), this, SLOT(pdb_visualization()));

   configuration = new QPopupMenu;
   configuration->insertItem(tr("&Load Configuration"), this, SLOT(load_config()));
   configuration->insertItem(tr("&Save Current Configuration"), this, SLOT(write_config()));
   configuration->insertItem(tr("&Reset to Default Configuration"), this, SLOT(reset()));

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);
   
   menu = new QMenuBar(frame);
   menu->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   menu->insertItem(tr("&Lookup Tables"), lookup_tables);
   menu->insertItem(tr("&SOMO Options"), somo_options);
   menu->insertItem(tr("&PDB Options"), pdb_options);
   menu->insertItem(tr("&Configurations"), configuration);

   lbl_somo = new QLabel(tr("SOMO Program:"), this);
   Q_CHECK_PTR(lbl_somo);
   lbl_somo->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_somo->setAlignment(AlignCenter|AlignVCenter);
   lbl_somo->setMinimumHeight(minHeight1);
   lbl_somo->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

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

   pb_somo = new QPushButton(tr("Build SOMO Bead Model"), this);
   Q_CHECK_PTR(pb_somo);
   pb_somo->setMinimumHeight(minHeight1);
   pb_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_somo->setEnabled(false);
   pb_somo->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_somo, SIGNAL(clicked()), SLOT(calc_somo()));

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
   
   pb_view_asa = new QPushButton(tr("Show ASA Results"), this);
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
   editor->setMinimumWidth(500);
   m = new QMenuBar(editor, "menu" );
   m->setMinimumHeight(minHeight1);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   file->insertItem( tr("Print"), this, SLOT(print()),   ALT+Key_P );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (QTextEdit::WidgetWidth);

   int rows=13, columns = 3, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(frame, j, j, 0, 1);
   background->addMultiCellWidget(editor, j, j+12, 2, 2);
   j++;
   background->addMultiCellWidget(lbl_somo, j, j, 0, 1);
   j++;
   background->addWidget(pb_select_residue_file, j, 0);
   background->addWidget(lbl_table, j, 1);
   j++;
   background->addWidget(pb_load_pdb, j, 0);
   background->addWidget(lbl_pdb_file, j, 1);
   j++;
   background->addWidget(lbl_model, j, 0);
   background->addMultiCellWidget(lb_model, j, j+1, 1, 1);
   j++;
   background->addWidget(pb_view_pdb, j, 0);
   j++;
   background->addWidget(lbl_bead_model_prefix, j, 0);
   background->addWidget(le_bead_model_prefix, j, 1);
   j++;
   background->addWidget(pb_somo, j, 0);
   background->addWidget(pb_grid, j, 1);
   j++;
   background->addWidget(cb_calcAutoHydro, j, 1);
   j++;
   background->addWidget(pb_view_asa, j, 0);
   background->addWidget(pb_visualize, j, 1);
   j++;
   background->addWidget(pb_load_bead_model, j, 0);
   background->addWidget(le_bead_model_file, j, 1);
   j++;
   background->addWidget(pb_calc_hydro, j, 0);
   background->addWidget(pb_show_hydro_results, j, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
   j++;
   background->addWidget(pb_stop_calc, j, 0);
   background->addWidget(progress, j, 1);
   //   background->addMultiCellWidget(progress, j, j, 0, 1);
}

void US_Hydrodyn::load_config()
{
   QString fname = QFileDialog::getOpenFileName ( somo_dir, "*.config", 0, 0, "Please select a SOMO configuration file...", 0, TRUE );
   read_config(fname);
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


struct BPair {
   int i;
   int j;
   float separation;
   bool active;
};


#if defined(TO_DO)
static void outward_translate_1_sphere_1_fixed(
                                               float *r1,  // radius of sphere 1
                                               float *r2,  // radius of sphere 2
                                               float *x1, // center of sphere 1
                                               float *x2, // center of sphere 2
                                               float *v1 // normalized vector to sphere 1 from COG
                                               ) {
#if defined(DEBUG)
   printf("outward_translate_1_sphere_1_fixed\n"
          "sphere 1 radius %f center [%f,%f,%f] cog vector [%f,%f,%f]\n"
          "sphere 2 radius %f center [%f,%f,%f]\n",
          *r1, x1[0], x1[1], x1[2], v1[0], v1[1], v1[2],
          *r2, x2[0], x2[1], x2[2]);
#endif

   float k;
   float denom =
      -2 * *r1 * *r2 - *r2 * *r2 +
      *r1 * *r1 * (-1 + v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]);

   float sfact =
      sqrt(
           (*r1 + *r2) * (*r1 + *r2) *
           ((-2 * *r1 * *r2 - *r2 * *r2 +
             *r1 * *r1 * (-1 + v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]))
            * (
               (*r1 + *r2) * (*r1 + *r2) -
               (x1[0] - x2[0]) * (x1[0] - x2[0]) -
               (x1[1] - x2[1]) * (x1[1] - x2[1]) -
               (x1[2] - x2[2]) * (x1[2] - x2[2])) +
            pow(*r1 * *r1 + *r2 * *r2 +
                *r1 * (
                       2 * *r2 + v1[0] * x1[0] + v1[1] * x1[1] + v1[2] * x1[2] -
                       v1[0] * x2[0] - v1[1] * x2[1] - v1[2] * x2[2]),2)));

   float b =
      - (*r1 + *r2) *
      (*r1 * *r1 + *r2 * *r2 +
       *r1 * (2 * *r2 + v1[0] * x1[0] + v1[1] * x1[1] + v1[2] * x1[2] -
              v1[0] * x2[0] - v1[1] * x2[1] - v1[2] * x2[2]));

   k = (b + sfact) / denom;

   float k1 = k * *r1 / (*r1 + *r2);
   float k2 = k * *r2 / (*r1 + *r2);

   x1[0] += k1 * v1[0];
   x1[1] += k1 * v1[1];
   x1[2] += k1 * v1[2];

   *r1 -= k1;
   *r2 -= k2;

#if defined(DEBUG)

   printf("k %.3f\n", k);

   printf("new coordinates, radius\n"
          "sphere 1 radius %.3f center [%.3f,%.3f,%.3f]\n"
          "sphere 2 radius %.3f center [%.3f,%.3f,%.3f]\n"
          "r1 + r2 - distance between spheres = %f\n",
          *r1,
          x1[0],
          x1[1],
          x1[2],
          *r2,
          x2[0],
          x2[1],
          x2[2],
          *r1 + *r2 -
          sqrt(
               (x1[0] - x2[0]) * (x1[0] - x2[0]) +
               (x1[1] - x2[1]) * (x1[1] - x2[1]) +
               (x1[2] - x2[2]) * (x1[2] - x2[2]))
          );
#endif

}
#endif

static void outward_translate_2_spheres(float *r1, // radius of sphere 1
                                        float *r2, // radius of sphere 2
                                        float *x1, // center of sphere 1
                                        float *x2, // center of sphere 2
                                        float *v1, // normalized vector to sphere 1 from COG
                                        float *v2  // normalized vector to sphere 1 from COG
                                        ) {
#if defined(DEBUG) || defined(DEBUG1)
   printf("outward_translate_2_spheres\n"
          "sphere 1 radius %.3f center [%.3f,%.3f,%.3f] cog vector [%.3f,%.3f,%.3f]\n"
          "sphere 2 radius %.3f center [%.3f,%.3f,%.3f] cog vector [%.3f,%.3f,%.3f]\n",
          *r1, x1[0], x1[1], x1[2], v1[0], v1[1], v1[2],
          *r2, x2[0], x2[1], x2[2], v2[0], v2[1], v2[2]);
#endif

   float k;
   float denom =
      2 * *r1 * *r2 * (1 + v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]) -
      *r1 * *r1 * (-1 + v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]) -
      *r2 * *r2 * (-1 + v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2]);

   float sfact =
      sqrt(
           (*r1 + *r2) * (*r1 + *r2) *
           ((*r1 * *r1 * (-1 + v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]) -

             2 * *r1 * *r2 * (1 + v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]) +

             *r2 * *r2 * (-1 + v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2])) *

            ((*r1 + *r2) * (*r1 + *r2) -
             (x1[0] - x2[0]) * (x1[0] - x2[0]) -
             (x1[1] - x2[1]) * (x1[1] - x2[1]) -
             (x1[2] - x2[2]) * (x1[2] - x2[2])) +

            pow(*r1 * *r1 +
                *r1 * (
                       2 * *r2 + v1[0] * x1[0] + v1[1] * x1[1] + v1[2] * x1[2] -
                       v1[0] * x2[0] - v1[1] * x2[1] - v1[2] * x2[2]) +
                *r2 * (
                       *r2 - v2[0] * x1[0] - v2[1] * x1[1] - v2[2] * x1[2] +
                       v2[0] * x2[0] + v2[1] * x2[1] + v2[2] * x2[2]),2)));

   float b =
      (*r1 + *r2) *
      (*r1 * *r1 +
       *r1 * (2 * *r2 + v1[0] * x1[0] + v1[1] * x1[1] + v1[2] * x1[2] -
              v1[0] * x2[0] - v1[1] * x2[1] - v1[2] * x2[2]) +
       *r2 * (*r2 - v2[0] * x1[0] - v2[1] * x1[1] - v2[2] * x1[2] + v2[0] * x2[0] +
              v2[1] * x2[1] + v2[2] * x2[2]));

   k = (b - sfact) / denom;

   float k1 = k * *r1 / (*r1 + *r2);
   float k2 = k * *r2 / (*r1 + *r2);

   x1[0] += k1 * v1[0];
   x1[1] += k1 * v1[1];
   x1[2] += k1 * v1[2];

   x2[0] += k2 * v2[0];
   x2[1] += k2 * v2[1];
   x2[2] += k2 * v2[2];

   *r1 -= k1;
   *r2 -= k2;

#if defined(DEBUG) || defined(DEBUG1)

   printf("k %.3f\n", k);

   printf("new coordinates, radius\n"
          "sphere 1 radius %.3f center [%.3f,%.3f,%.3f]\n"
          "sphere 2 radius %.3f center [%.3f,%.3f,%.3f]\n"
          "r1 + r2 - distance between spheres = %f\n",
          *r1,
          x1[0],
          x1[1],
          x1[2],
          *r2,
          x2[0],
          x2[1],
          x2[2],
          *r1 + *r2 -
          sqrt(
               (x1[0] - x2[0]) * (x1[0] - x2[0]) +
               (x1[1] - x2[1]) * (x1[1] - x2[1]) +
               (x1[2] - x2[2]) * (x1[2] - x2[2]))
          );
#endif
}

void US_Hydrodyn::get_atom_map(PDB_model *model)
{
   atom_counts.clear();
   has_OXT.clear();
   for (unsigned int j = 0; j < model->molecule.size(); j++)
   {
      QString lastResSeq = "";
      QString lastResName = "";
      int atom_count = 0;
      for (unsigned int k = 0; k < model->molecule[j].atom.size(); k++)
      {
         PDB_atom *this_atom = &(model->molecule[j].atom[k]);
         if (lastResSeq != this_atom->resSeq ||
             lastResName != this_atom->resName)
         {
            // new residue
            if (lastResSeq)
            {
               atom_counts[QString("%1|%2|%3")
                           .arg(j)
                           .arg(lastResName)
                           .arg(lastResSeq)] = atom_count;
            }
            lastResSeq = this_atom->resSeq;
            lastResName = this_atom->resName;
            atom_count = 0;
         }
         if(this_atom->name == "OXT") {
            has_OXT[QString("%1|%2|%3")
                    .arg(j)
                    .arg(this_atom->resName)
                    .arg(this_atom->resSeq)]++;
         }
         atom_count++;
      }
      if (lastResSeq)
      {
         atom_counts[QString("%1|%2|%3")
                     .arg(j)
                     .arg(lastResName)
                     .arg(lastResSeq)] = atom_count;
      }
   }

#if defined(DEBUG_MULTI_RESIDUE)
   for (map < QString, int >::iterator it = atom_counts.begin();
        it != atom_counts.end();
        it++)
   {
      printf("atom count map %s map pos %d\n",
             it->first.ascii(), it->second);
   }
   for (map < QString, int >::iterator it = has_OXT.begin();
        it != has_OXT.end();
        it++)
   {
      printf("has_OXT map %s map value %d\n",
             it->first.ascii(), it->second);
   }
#endif
   // end of atom count build
}

void US_Hydrodyn::build_molecule_maps(PDB_model *model)
{
   // creates molecules_residues_atoms map
   // molecules_residues_atoms maps molecule #|resSeq to vector of atom names
   molecules_residues_atoms.clear();
   // molecules_residue_name maps molecule #|resSeq to residue name
   molecules_residue_name.clear();
   // molecules_idx_seq is a vector of the idx's
   molecules_idx_seq.clear();
   molecules_residue_errors.clear();
   molecules_residue_missing_counts.clear();
   molecules_residue_min_missing.clear();
   molecules_residue_missing_atoms.clear();
   molecules_residue_missing_atoms_beads.clear();
   molecules_residue_missing_atoms_skip.clear();

   // pass 1 setup molecule basic maps
   for (unsigned int j = 0; j < model->molecule.size(); j++)
   {
      for (unsigned int k = 0; k < model->molecule[j].atom.size(); k++)
      {
         QString idx = QString("%1|%2").arg(j).arg(model->molecule[j].atom[k].resSeq);
         molecules_residues_atoms[idx].push_back(model->molecule[j].atom[k].name);
         if (!molecules_residue_name[idx])
         {
            molecules_residue_name[idx] = model->molecule[j].atom[k].resName;
            molecules_idx_seq.push_back(idx);
         }
      }
   }

   // pass 2 setup error maps
   for (unsigned int i = 0; i < molecules_idx_seq.size(); i++)
   {
      QString idx = molecules_idx_seq[i];
      QString resName = molecules_residue_name[idx];
      if (multi_residue_map[resName].size())
      {
         int minimum_missing = INT_MAX; // find the entry with the minimum # of missing atoms
         int minimum_missing_pos = 0;   // and its position
         for (unsigned int j = 0; j < multi_residue_map[resName].size(); j++)
         {
            QString error_msg = "";
            int missing_count = 0;
            // for this residue, clear flags
            for (unsigned int r = 0; 
                 r < residue_list[multi_residue_map[resName][j]].r_atom.size(); 
                 r++)
            {
               residue_list[multi_residue_map[resName][j]].r_atom[r].tmp_used = false;
            }

            // now set flags
            // first check for non-coded atoms
            for (unsigned int k = 0; k < molecules_residues_atoms[idx].size(); k++)
            {
               bool found = false;
               bool any = false;
               for (unsigned int r = 0; 
                    !found && r < residue_list[multi_residue_map[resName][j]].r_atom.size(); 
                    r++)
               {
                  if (residue_list[multi_residue_map[resName][j]].r_atom[r].name == 
                      molecules_residues_atoms[idx][k]) 
                  {
                     any = true;
                     if (!residue_list[multi_residue_map[resName][j]].r_atom[r].tmp_used)
                     {
                        residue_list[multi_residue_map[resName][j]].r_atom[r].tmp_used = true;
                        found = true;
                     }
                  }
               }
               if (!found)
               {
                  missing_count = -1;
                  error_msg += QString("%1coded atom %2. ")
                     .arg(any ? "Duplicate " : "Non-")
                     .arg(molecules_residues_atoms[idx][k]);
               }
            }
            // now check for missing atoms
            vector < QString > missing_atoms;
            vector < unsigned int > missing_atoms_beads;
            for (unsigned int r = 0; 
                 r < residue_list[multi_residue_map[resName][j]].r_atom.size(); 
                 r++)
            {
               if (!residue_list[multi_residue_map[resName][j]].r_atom[r].tmp_used)
               {
                  error_msg += QString("Missing atom %1. ")
                     .arg(residue_list[multi_residue_map[resName][j]].r_atom[r].name);
                  if ( missing_count != -1 )
                  {
                     missing_atoms.push_back(residue_list[multi_residue_map[resName][j]].r_atom[r].name);
                     missing_atoms_beads.push_back(residue_list[multi_residue_map[resName][j]].r_atom[r].bead_assignment);
                     molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                          .arg(idx)
                                                          .arg(j)
                                                          .arg(r)] = true;
                     missing_count++;
                  }
               }
            }
            molecules_residue_errors[idx].push_back(error_msg);
            molecules_residue_missing_counts[idx].push_back(missing_count);
            molecules_residue_missing_atoms[idx].push_back(missing_atoms);
            molecules_residue_missing_atoms_beads[idx].push_back(missing_atoms_beads);
            if ( missing_count >= 0 && missing_count < minimum_missing )
            {
               minimum_missing = missing_count;
               minimum_missing_pos = j;
            }
         }
         if ( minimum_missing < INT_MAX )
         {
            molecules_residue_min_missing[idx] = minimum_missing_pos;
         } 
         else
         {
            molecules_residue_min_missing[idx] = -1;
         }
      }
      else 
      {
         molecules_residue_errors[idx].push_back("Non-coded residue. ");
         molecules_residue_missing_counts[idx].push_back(-1);
         molecules_residue_min_missing[idx] = -1;
      }
   }

#if defined(BUILD_MAPS_DEBUG)
   cout << "--------molecules_residue_errors---------\n";
   for (unsigned int i = 0; i < molecules_idx_seq.size(); i++)
   {
      QString idx = molecules_idx_seq[i];
      QString resName = molecules_residue_name[idx];
      if (molecules_residue_errors[idx].size())
      {
         for (unsigned int j = 0; j < molecules_residue_errors[idx].size(); j++) 
         {
            if (molecules_residue_errors[idx][j].length()) 
            {
               cout << QString("Molecule idx <%1> resName <%2> match <%3> errors:\n")
                  .arg(idx).arg(resName).arg(j);
               cout << molecules_residue_errors[idx][j] << endl;
            }
         }
         if ( molecules_residue_min_missing[idx] > -1 )
         {
            int k = molecules_residue_min_missing[idx];
            cout << QString("^^^ Minimum missing position <%1> ma.sz %2 mab.sz %3\n")
               .arg(k)
               .arg(molecules_residue_missing_atoms[idx][k].size())
               .arg(molecules_residue_missing_atoms_beads[idx][k].size());
            for ( unsigned int j = 0; j < molecules_residue_missing_atoms[idx][k].size(); j++ )
            {
               cout << QString(".... missing atom %1 bead %2\n")
                  .arg(molecules_residue_missing_atoms[idx][k][j])
                  .arg(molecules_residue_missing_atoms_beads[idx][k][j]);
            }
         }
      }
   }
#endif    
}

int US_Hydrodyn::check_for_missing_atoms(QString *error_string, PDB_model *model)
{
   // go through molecules, build vector of residues
   // expand vector of residues to atom list from residue file
   // compare expanded list of residues to model ... list missing atoms missing
   int errors_found = 0;
   get_atom_map(model);
   bool failure_errors = 0;
   bead_exceptions.clear();

   // residue types are for automatic build builder to determine
   // if we have a protein, so that a special 'temporary' residue can
   // be created

   vector < map < unsigned int, unsigned int > > residue_types;
   vector < unsigned int > last_residue_type;

   residue_types.resize(model->molecule.size());
   last_residue_type.resize(model->molecule.size());

   build_molecule_maps(model);

   // keep track of errors shown
   map < QString, bool > error_shown;

   for (unsigned int j = 0; j < model->molecule.size(); j++)
   {
      QString lastResSeq = "";
      unsigned int lastResPos = 0;
      QString lastChainID = " ";
      bool spec_N1 = false;
      for (unsigned int k = 0; k < model->molecule[j].atom.size(); k++)
      {
         PDB_atom *this_atom = &(model->molecule[j].atom[k]);
         this_atom->active = false;
         QString count_idx =
            QString("%1|%2|%3")
            .arg(j)
            .arg(this_atom->resName)
            .arg(this_atom->resSeq);

         if (!bead_exceptions[count_idx])
         {
            bead_exceptions[count_idx] = 1;
         }

         // find residue in residues
#if defined(DEBUG)
         printf("check_for_missing_atoms search name %s resName %s\n",
                this_atom->name.ascii(),
                this_atom->resName.ascii());
#endif
         int respos = -1;
    
         for (unsigned int m = 0; m < residue_list.size(); m++)
         {
            if ((residue_list[m].name == this_atom->resName &&
                 (int)residue_list[m].r_atom.size() ==
                 atom_counts[count_idx] - has_OXT[count_idx] &&
                 this_atom->name != "OXT" &&
                 (k || this_atom->name != "N" || !regular_N_handling)) ||
                (residue_list[m].name == "OXT"
                 && this_atom->name == "OXT") ||
                (!k &&
                 this_atom->name == "N" &&
                 regular_N_handling &&
                 residue_list[m].name == "N1"))
            {
               respos = (int) m;
               residue_types[j][residue_list[m].type]++;
               last_residue_type[j] = residue_list[m].type;

#if defined(DEBUG_MULTI_RESIDUE)
               printf("atom name %s residue name %s pos %u atom.size() %u map %s size %u has oxt %d\n"
                      ,this_atom->name.ascii()
                      ,this_atom->resName.ascii()
                      ,m
                      ,residue_list[m].r_atom.size()
                      ,QString("%1|%2|%3")
                      .arg(j)
                      .arg(this_atom->resName)
                      .arg(this_atom->resSeq)
                      .ascii()
                      ,atom_counts[count_idx]
                      ,has_OXT[count_idx]
                      );
#endif

               this_atom->p_residue = &(residue_list[m]);
#if defined(DEBUG)
               printf("residue match %d resName %s \n", m, residue_list[m].name.ascii());
               printf("resname %s respos %d mappos %d mapsize %u\n"
                      ,this_atom->resName.ascii()
                      ,m
                      ,multi_residue_map[this_atom->resName][0]
                      ,multi_residue_map[this_atom->resName].size()
                      );
#endif
               if (lastResSeq != this_atom->resSeq)
               {
                  // new residue
                  if (lastResPos)
                  {
                     // check for false entries in last residue and warn about them
                     for (unsigned int l = 0; l < residue_list[lastResPos].r_atom.size(); l++)
                     {
                        if (spec_N1 &&
                            regular_N_handling &&
                            residue_list[lastResPos].r_atom[l].name == "N") {
                           residue_list[lastResPos].r_atom[l].tmp_flag = true;
                           spec_N1 = false;
                        }

                        if (!residue_list[lastResPos].r_atom[l].tmp_flag)
                        {
                           // missing atoms
                           errors_found++;
                           if (pdb_parse.missing_atoms == 0)
                           {
                              failure_errors++;
                           }
                           if (pdb_parse.missing_atoms == 1)
                           {
                              bead_exceptions[count_idx] = 2;
                           }
                           if (pdb_parse.missing_atoms == 2)
                           {
                              bead_exceptions[count_idx] = 4;
                           }
                           if (!error_shown[count_idx]) 
                           {
                              QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                                 .arg(lastChainID == " " ? "" : ("Chain " + lastChainID))
                                 .arg(j + 1)
                                 .arg(residue_list[lastResPos].name)
                                 .arg(lastResSeq);
                              QString idx = QString("%1|%2").arg(j).arg(lastResSeq);
                              switch (molecules_residue_errors[idx].size())
                              {
                              case 0: 
                                 this_error += QString("Missing atom %1.\n").arg(residue_list[lastResPos].r_atom[l].name);
                                 break;
                              case 1:
                                 this_error += molecules_residue_errors[idx][0] + "\n";
                                 break;
                              default :
                                 {
                                    this_error += "\n";
                                    for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                                    {
                                       this_error += QString("    Residue file entry %1: %2\n").
                                          arg(t+1).arg(molecules_residue_errors[idx][t]);
                                    }
                                 }
                                 break;
                              }
                              error_string->append(this_error);
                              error_shown[count_idx] = true;
                           }
                           // error_string->
                           // append(QString("").
                           //     sprintf("Missing atom: chain %s molecule %d atom %s residue %s %s\n",
                           //        lastChainID.ascii(),
                           //        j + 1,
                           //        residue_list[lastResPos].r_atom[l].name.ascii(),
                           //        lastResSeq.ascii(),
                           //        residue_list[lastResPos].name.ascii()));
                        }
                     }
                  }

                  // reset residue list
                  for (unsigned int l = 0; l < residue_list[m].r_atom.size(); l++)
                  {
                     residue_list[m].r_atom[l].tmp_flag = false;
                  }
                  lastResSeq = this_atom->resSeq;
                  lastChainID = this_atom->chainID;
                  lastResPos = m;
               }

               if (residue_list[m].name == "N1")
               {
                  lastResSeq = "";
                  lastResPos = 0;
                  spec_N1 = true;
               }
               break;
            }
         }
         if (respos == -1)
         {
            if ((this_atom->name != "H" && this_atom->name != "D"
                 && this_atom->resName != "DOD"
                 && this_atom->resName != "HOH" && (this_atom->altLoc == "A" || this_atom->altLoc == " ")))
            {
               QString msg_tag;
               bool do_error_msg = true;
               if (pdb_parse.missing_residues == 0 &&
                   pdb_parse.missing_atoms == 0)
               {
                  failure_errors++;
                  msg_tag = "Missing residue or atom";
               } 
               else 
               {
                  // ok, we have three cases here:
                  // 1. residue does exist & residue/atom doesn't
                  //    1.1 skip missing atoms controls
                  // 2. residue does exist & residue/atom does
                  //    2.1 there must be a missing atom since count doesn't match so atom controls
                  // 3. residue does not exist
                  //    3.1 skip missing residue control
                  // ---------------------
                  //
                  // note: we're just checking the 1st of multiple possibilities
                  puts("cases---");
                  if (multi_residue_map[this_atom->resName].size()) 
                  {
                     // residue exists, does residue/atom?
                     QString idx = QString("%1|%2|%3")
                        .arg(this_atom->resName)
                        .arg(this_atom->name)
                        .arg(0); 
                     printf("cases residue found: idx %s\n", idx.ascii());
                     if (valid_atom_map[idx].size()) 
                     {
                        puts("case 2.1");
                        msg_tag = "Missing or extra atom in residue";
                     } 
                     else
                     {
                        // atom does not exist, skip missing atoms controls
                        puts("case 1.1");
                        msg_tag = "Missing atom";
                     }
                     if (pdb_parse.missing_atoms == 0)
                     {
                        failure_errors++;
                     }
                     if (pdb_parse.missing_atoms == 1)
                     {
                        bead_exceptions[count_idx] = 2;
                     }
                     if (pdb_parse.missing_atoms == 2)
                     {
                        if ( molecules_residue_min_missing[QString("%1|%2").arg(j).arg(this_atom->resSeq)] == -1 )
                        {
                           failure_errors++;
                           bead_exceptions[count_idx] = 5;
                        } 
                        else
                        {
                           bead_exceptions[count_idx] = 4;
                        }
                     }
                  } 
                  else 
                  {
                     // residue does not exist, skip missing residue controls
                     puts("case 3.1");
                     msg_tag = "Non-coded residue";
                     if (pdb_parse.missing_residues == 0)
                     {
                        failure_errors++;
                     }
                     if (pdb_parse.missing_residues == 1)
                     {
                        bead_exceptions[count_idx] = 2;
                     }
                     if (pdb_parse.missing_residues == 2)
                     {
                        bead_exceptions[count_idx] = 3;
                     }
                  }
               }
               if (do_error_msg) {
                  errors_found++;
                  if (!error_shown[count_idx]) 
                  {
                     QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                        .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID))
                        .arg(j + 1)
                        .arg(this_atom->resName)
                        .arg(this_atom->resSeq);
                     QString idx = QString("%1|%2").arg(j).arg(this_atom->resSeq);
                     switch (molecules_residue_errors[idx].size())
                     {
                     case 0: 
                        this_error += msg_tag;
                        break;
                     case 1:
                        this_error += molecules_residue_errors[idx][0] + "\n";
                        break;
                     default :
                        {
                           this_error += "\n";
                           for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                           {
                              this_error += QString("    Residue file entry %1: %2\n").
                                 arg(t+1).arg(molecules_residue_errors[idx][t]);
                           }
                        }
                        break;
                     }
                     error_string->append(this_error);
                     error_shown[count_idx] = true;
                  }
        
                  // error_string->append(QString("").sprintf("%s: chain %s molecule %d atom %s residue %s %s\n",
                  //               msg_tag.ascii(),
                  //               this_atom->chainID.ascii(),
                  //               j + 1,
                  //               this_atom->name.ascii(),
                  //               this_atom->resSeq.ascii(),
                  //               this_atom->resName.ascii()
                  //               ));
               }
            }
         }
         else 
         {
            int atompos = -1;
            for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
            {

               if (residue_list[respos].r_atom[m].name == this_atom->name ||
                   (
                    this_atom->name == "N" &&
                    regular_N_handling &&
                    !k &&
                    residue_list[respos].r_atom[m].name == "N1"
                    )
                   )
               {
                  this_atom->active = true;
                  residue_list[respos].r_atom[m].tmp_flag = true;
                  atompos = (int) m;
                  break;
               }
            }
            if (atompos == -1)
            {
               errors_found++;
               // currently, we still fail for unknown or 'extra' atoms
               failure_errors++;
               if (!error_shown[count_idx]) 
               {
                  QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                     .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID))
                     .arg(j + 1)
                     .arg(this_atom->resName)
                     .arg(this_atom->resSeq);
                  QString idx = QString("%1|%2").arg(j).arg(this_atom->resSeq);
                  switch (molecules_residue_errors[idx].size())
                  {
                  case 0: 
                     this_error += QString("Unknown atom %1.\n").arg(this_atom->name);
                     break;
                  case 1:
                     this_error += molecules_residue_errors[idx][0] + "\n";
                     break;
                  default :
                     {
                        this_error += "\n";
                        for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                        {
                           this_error += QString("    Residue file entry %1: %2\n").
                              arg(t+1).arg(molecules_residue_errors[idx][t]);
                        }
                     }
                     break;
                  }
                  error_string->append(this_error);
                  error_shown[count_idx] = true;
               }
        

               // error_string->append(QString("").sprintf("unknown atom chain %s molecule %d atom %s residue %s %s\n",
               //            this_atom->chainID.ascii(),
               //            j + 1,
               //            this_atom->name.ascii(),
               //            this_atom->resSeq.ascii(),
               //            this_atom->resName.ascii()
               //            ));
            }
         }
      }
      if (lastResPos)
      {
         // check for false entries in last residue and warn about them
         for (unsigned int l = 0; l < residue_list[lastResPos].r_atom.size(); l++)
         {
            if (!residue_list[lastResPos].r_atom[l].tmp_flag)
            {
               errors_found++;
               QString count_idx =
                  QString("%1|%2|%3")
                  .arg(j)
                  .arg(residue_list[lastResPos].r_atom[l].name)
                  .arg(lastResSeq);
               if (pdb_parse.missing_atoms == 0)
               {
                  failure_errors++;
               } 
               else 
               {
                  if (pdb_parse.missing_atoms == 1)
                  {
                     bead_exceptions[count_idx] = 2;
                  }
                  if (pdb_parse.missing_atoms == 2)
                  {
                     bead_exceptions[count_idx] = 4;
                  }
               }
               if (!error_shown[count_idx]) 
               {
                  QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                     .arg(lastChainID == " " ? "" : ("Chain " + lastChainID))
                     .arg(j + 1)
                     .arg(residue_list[lastResPos].name)
                     .arg(lastResSeq);
                  QString idx = QString("%1|%2").arg(j).arg(lastResSeq);
                  switch (molecules_residue_errors[idx].size())
                  {
                  case 0: 
                     this_error += QString("Missing atom %1.\n").arg(residue_list[lastResPos].r_atom[l].name);
                     break;
                  case 1:
                     this_error += molecules_residue_errors[idx][0] + "\n";
                     break;
                  default :
                     {
                        this_error += "\n";
                        for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                        {
                           this_error += QString("    Residue file entry %1: %2\n").
                              arg(t+1).arg(molecules_residue_errors[idx][t]);
                        }
                     }
                     break;
                  }
                  error_string->append(this_error);
                  error_shown[count_idx] = true;
               }
               // error_string->
               //  append(QString("").
               // sprintf("missing atom chain %s molecule %d atom %s residue %s %s\n",
               //    lastChainID.ascii(),
               //    j + 1,
               //    residue_list[lastResPos].r_atom[l].name.ascii(),
               //    lastResSeq.ascii(),
               //    residue_list[lastResPos].name.ascii()));
            }
         }
      }
   }

   if (error_string->length())
   {
      if (failure_errors > 0) 
      {
         return errors_found;
      }
      else 
      {
         editor->append("Encountered the following warnings with your PDB structure:\n" + *error_string);
         *error_string = "";
         // repair model...
         PDB_model org_model = *model;
         model->molecule.clear();
         // we may need to redo the residues also
         model->residue.clear();
         printf("vbar before: %g\n", model->vbar);
         QString new_residue_name = "";
         for (unsigned int j = 0; j < org_model.molecule.size(); j++)
         {
            PDB_chain tmp_chain;
            QString lastResSeq = "";
            QString lastResName = "";

            bool auto_bb_aa = false;             // are we doing special amino acid handling?
            map < QString, int > aa_main_chain;  // to make sure we have a good main chain
            int current_bead_assignment = 0;
            bool one_bead = false;               // fall back to one bead for missing atoms
            if (last_residue_type[j] == 0 &&      
                residue_types[j].size() == 1) 
            {
               // only amino acids, so we can create two beads
#if defined(AUTO_BB_DEBUG)
               puts("auto_bb_amino_acids");
#endif
               auto_bb_aa = true;
            }

            for (unsigned int k = 0; k < org_model.molecule[j].atom.size(); k++)
            {
               QString count_idx =
                  QString("%1|%2|%3")
                  .arg(j)
                  .arg(org_model.molecule[j].atom[k].resName)
                  .arg(org_model.molecule[j].atom[k].resSeq);
               switch (bead_exceptions[count_idx])
               {
               case 1:
                  tmp_chain.atom.push_back(org_model.molecule[j].atom[k]);
                  if (org_model.molecule[j].atom[k].resSeq != lastResSeq) 
                  {
                     lastResSeq = org_model.molecule[j].atom[k].resSeq;
                     lastResName = org_model.molecule[j].atom[k].resName;
                     model->residue.push_back(residue_list[multi_residue_map[lastResName][0]]);
                  }
                  break;
               case 2:
               case 5: // extra or non-coded atom
                  printf("removing molecule %u atom %u from model\n", 
                         j, k);
                  break;
               case 3:
                  {
                     // create the temporary residue
                     // is this a new residue
                     PDB_atom *this_atom = &org_model.molecule[j].atom[k];
                     QString new_residue_idx = this_atom->resName;  // we could add atom_count to the idx for counting by unique atom counts...
#if defined(AUTO_BB_DEBUG)
                     printf("1.0 <%s> residue types %u, last_type %u\n", 
                            new_residue_idx.ascii(),
                            residue_types[j].size(),
                            last_residue_type[j]
                            );
#endif
           
                     if (this_atom->resSeq != lastResSeq) 
                     {
                        current_bead_assignment = 0;
                        if (auto_bb_aa) 
                        {
                           // reset the main chain counts
                           aa_main_chain.clear();
                           aa_main_chain["N"] = 0;
                           aa_main_chain["CA"] = 0;
                           aa_main_chain["C"] = 0;
                           aa_main_chain["O"] = 0;
                        }
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;
                        // this is a new unknown residue
                        // each instance of the residue gets a unique name, so we don't have
                        // to worry about duplicates and alternate duplicates
                        new_residues[new_residue_idx]++;
                        new_residue_name = QString("%1_%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
#if defined(AUTO_BB_DEBUG)
                        printf("1.1 <%s>\n", new_residue_name.ascii());
#endif
                        atom_counts[QString("%1|%2|%3")
                                    .arg(j)
                                    .arg(new_residue_name)
                                    .arg(this_atom->resSeq)] = atom_counts[count_idx];
                        // create new_residue
                        new_residue.comment = QString("Temporary residue %1").arg(new_residue_name);
                        new_residue.name = new_residue_name;
                        new_residue.type = 5;  // other
                        new_residue.molvol = misc.avg_volume * atom_counts[count_idx];
                        // new_residue.asa = misc.avg_asa * atom_counts[count_idx];
                        new_residue.asa = 0;
                        new_residue.vbar = misc.avg_vbar;
                        new_residue.r_atom.clear();
                        new_residue.r_bead.clear();
                        new_bead.hydration = (unsigned int)(misc.avg_hydration * atom_counts[count_idx] + .5);
                        new_bead.color = 10;         // light green
                        new_bead.placing_method = 0; // cog
                        new_bead.chain = 1;          // side chain
                        new_bead.volume = misc.avg_volume * atom_counts[count_idx];
                        new_bead.mw = misc.avg_mass * atom_counts[count_idx];
                        new_residue.r_bead.push_back(new_bead);
                        multi_residue_map[new_residue.name].push_back(residue_list.size());
                        residue_list.push_back(new_residue);
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;
#if defined(AUTO_BB_DEBUG)
                        printf("1.1b <%s>\n", new_residue.name.ascii());
#endif
                        model->residue.push_back(residue_list[multi_residue_map[new_residue.name][0]]);
                     } 
                     else
                     {
                        new_residue_name = QString("%1_%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
#if defined(AUTO_BB_DEBUG)
                        printf("1.2 <%s>\n", new_residue_name.ascii());
#endif
                        if (new_residue_name != new_residue.name)
                        {
                           QMessageBox::message(tr("Internal error:"), 
                                                tr("Unexpected new residue name inconsistancy.\n") +
                                                QString("\"%1\" != \"%2\"").arg(new_residue_name).arg(new_residue.name));
                           exit(-1);
                        }
                     }
                     // the new residue should always be the last one, since we just added it
                     int respos = residue_list.size() - 1;
                     if (new_residue.name != residue_list[respos].name)
                     {
                        QMessageBox::message(tr("Internal error:"), 
                                             tr("Unexpected new residue name residue_list inconsistancy."));
                        exit(-1);
                     }
                     // ok, now we can push back the modified atom
                     if (auto_bb_aa)
                     {
                        if (residue_list[respos].r_atom.size() < 4) 
                        {
                           aa_main_chain[this_atom->name]++;
                           if (residue_list[respos].r_atom.size() == 3) 
                           {
                              // early handling in case of no side chain
                              if (aa_main_chain["N"] == 1 &&
                                  aa_main_chain["CA"] == 1 &&
                                  aa_main_chain["C"] == 1 &&
                                  aa_main_chain["O"] == 1)
                              {
                                 residue_list[respos].type = 0;
                                 residue_list[respos].r_bead[0].chain = 0;  // main chain
                              }
                           }
                        } 
                        else 
                        {
                           if (residue_list[respos].r_atom.size() == 4) 
                           {
                              if (aa_main_chain["N"] == 1 &&
                                  aa_main_chain["CA"] == 1 &&
                                  aa_main_chain["C"] == 1 &&
                                  aa_main_chain["O"] == 1)
                              {
                                 // ok, we have a proper backbone
                                 // so we have to redo the beads etc.
                                 current_bead_assignment = 1;
                                 // redo 1st bead
                                 residue_list[respos].type = 0;

                                 residue_list[respos].r_bead[0].hydration = 
                                    (unsigned int)(misc.avg_hydration * 4 + .5);
                                 residue_list[respos].r_bead[0].chain = 0;  // main chain
                                 residue_list[respos].r_bead[0].volume = misc.avg_volume * 4;
                                 residue_list[respos].r_bead[0].mw = misc.avg_mass * 4;

                                 // create a 2nd bead
                                 residue_list[respos].r_bead.push_back(residue_list[respos].r_bead[0]);
                                 residue_list[respos].r_bead[1].hydration = 
                                    (unsigned int)(misc.avg_hydration * (atom_counts[count_idx] - 4) + .5);
                                 residue_list[respos].r_bead[1].chain = 1;  // side chain
                                 residue_list[respos].r_bead[1].volume =
                                    misc.avg_volume * (atom_counts[count_idx] - 4);
                                 residue_list[respos].r_bead[1].mw = 
                                    misc.avg_mass * (atom_counts[count_idx] - 4);
                              }
                           }
                        }
                     }
            
                     new_atom.name = this_atom->name;
                     new_atom.hybrid.name = this_atom->name;
                     new_atom.hybrid.mw = misc.avg_mass;
                     new_atom.hybrid.radius = misc.avg_radius;
                     new_atom.bead_assignment = current_bead_assignment; 
                     new_atom.positioner = true;
                     new_atom.serial_number = residue_list[respos].r_atom.size();
                     residue_list[respos].r_atom.push_back(new_atom);
                     PDB_atom atom_to_add = org_model.molecule[j].atom[k];
                     atom_to_add.resName = new_residue.name;
#if defined(AUTO_BB_DEBUG)
                     printf("1.3 <%s>\n", new_residue.name.ascii());
#endif
                     tmp_chain.atom.push_back(atom_to_add);
                  }
                  break;
               case 4: // missing atoms
                  {
                     PDB_atom *this_atom = &org_model.molecule[j].atom[k];
                     QString idx = QString("%1|%2").arg(j).arg(org_model.molecule[j].atom[k].resSeq);
                     QString new_residue_idx = this_atom->resName;  
                     int pos = molecules_residue_min_missing[idx];
                     if ( pos < 0 )
                     {
                        cout << QString("Internal error: idx %1 Missing atom error %2 ! < 0!\n")
                           .arg(idx)
                           .arg(pos);
                        exit(-1);
                     }
                     cout << QString("ta->resseq %1, pos %2\n").arg(this_atom->resName).arg(pos);
                     int orgrespos = multi_residue_map[this_atom->resName][pos];
                     if (this_atom->resSeq != lastResSeq) 
                     {
                        // new residue to create with existing atoms, but increased values...
                        // possibly for a multi-bead residue
                        one_bead = false;
                        current_bead_assignment = 0;
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;
                        // setup how many atoms are missing from each bead
                        vector < int > beads_missing_atom_count;
                        printf("orgrespos %d\n", orgrespos);
                        beads_missing_atom_count.resize(residue_list[orgrespos].r_bead.size());

                        for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                        {
                           beads_missing_atom_count[i] = 0;
                        }
                        // accumulate missing atom count for each bead
                        for ( unsigned int i = 0; i < molecules_residue_missing_atoms[idx][pos].size(); i++ )
                        {
                           beads_missing_atom_count[molecules_residue_missing_atoms_beads[idx][pos][i]]++;
                        }
                        // do any of these leave an empty bead?
                        // first, we need to know how many atoms are in each bead...
                        vector < int > atoms_per_bead;
                        atoms_per_bead.resize(residue_list[orgrespos].r_bead.size());
                        for ( unsigned int i = 0; i < atoms_per_bead.size(); i++ )
                        {
                           atoms_per_bead[i] = 0;
                        }
                        for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                        {
                           atoms_per_bead[residue_list[orgrespos].r_atom[i].bead_assignment]++;
                        }
                        
                        for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                        {
                           if ( atoms_per_bead[i] - beads_missing_atom_count[i] < 1 )
                           {
                              // ugh, fall back to one bead!
                              one_bead = true;
                              printf("notice: fallback to one bead for missing atom\n");
                           }
                        }

                        vector < double > atoms_scale_weight; // multiplier for atoms weight
                        vector < double > atoms_scale_radius; // multiplier for atoms radii
                        atoms_scale_weight.resize(residue_list[orgrespos].r_bead.size());
                        atoms_scale_radius.resize(residue_list[orgrespos].r_bead.size());
                        if ( one_bead )
                        {
                           // sum molecular weight & radii cubed for total bead & bead sans missing
                           double tot_mw = 0;
                           double tot_mw_missing = 0;
                           double tot_radii3 = 0;
                           double tot_radii3_missing = 0;
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              tot_mw += residue_list[orgrespos].r_atom[i].hybrid.mw;
                              tot_radii3 += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3.0);
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 tot_mw_missing += residue_list[orgrespos].r_atom[i].hybrid.mw;
                                 tot_radii3_missing += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3.0);
                              }
                           }
                           atoms_scale_weight[0] = 
                              tot_mw / tot_mw_missing;
                           atoms_scale_radius[0] = 
                              pow(tot_radii3 / tot_radii3_missing, 1.0/3.0);
                        }
                        else
                        {
                           // sum molecular weight & radii cubed for total bead & bead sans missing
                           vector < double > tot_mw;
                           vector < double > tot_mw_missing;
                           vector < double > tot_radii3;
                           vector < double > tot_radii3_missing;
                           for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                           {
                              tot_mw.push_back(0.0);
                              tot_mw_missing.push_back(0.0);
                              tot_radii3.push_back(0.0);
                              tot_radii3_missing.push_back(0.0);
                           }
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              tot_mw[residue_list[orgrespos].r_atom[i].bead_assignment]
                                 += residue_list[orgrespos].r_atom[i].hybrid.mw;
                              tot_radii3[residue_list[orgrespos].r_atom[i].bead_assignment]
                                 += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3.0);
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 tot_mw_missing[residue_list[orgrespos].r_atom[i].bead_assignment]
                                    += residue_list[orgrespos].r_atom[i].hybrid.mw;
                                 tot_radii3_missing[residue_list[orgrespos].r_atom[i].bead_assignment]
                                    += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3.0);
                              }
                           }
                           for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                           {
                              atoms_scale_weight[i] = 
                                 tot_mw[i] / tot_mw_missing[i];
                              atoms_scale_radius[i] = 
                                 pow(tot_radii3[i] / tot_radii3_missing[i], 1.0/3.0);
                           }
                        }

                        // create the new residue
                        new_residues[new_residue_idx]++;
                        new_residue_name = QString("%1_%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
#if defined(AUTO_BB_DEBUG)
                        printf("a1.1 <%s>\n", new_residue_name.ascii());
#endif
                        atom_counts[QString("%1|%2|%3")
                                    .arg(j)
                                    .arg(new_residue_name)
                                    .arg(this_atom->resSeq)] = atom_counts[count_idx];
                        new_residue.comment = QString("Temporary residue %1").arg(new_residue_name);
                        new_residue.name = new_residue_name;
                        new_residue.type = residue_list[orgrespos].type;
                        new_residue.molvol = residue_list[orgrespos].molvol;
                        new_residue.asa = residue_list[orgrespos].asa;
                        new_residue.vbar = residue_list[orgrespos].vbar;
                        new_residue.r_atom.clear();
                        new_residue.r_bead.clear();

                        // create the beads
                        if ( one_bead )
                        {
                           new_bead.volume = 0;
                           new_bead.mw = 0;
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_bead.size(); i++ )
                           {
                              new_bead.volume += residue_list[orgrespos].r_bead[i].volume;
                              new_bead.mw += residue_list[orgrespos].r_bead[i].mw;
                           }
                           new_bead.hydration = (unsigned int)(misc.avg_hydration * atom_counts[count_idx] + .5);
                           new_bead.color = 10;         // light green
                           new_bead.placing_method = 0; // cog
                           new_bead.chain = 1;          // side chain
                           new_residue.r_bead.push_back(new_bead);
                           // create the atoms
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 new_atom = residue_list[orgrespos].r_atom[i];
                                 new_atom.hybrid.mw *= atoms_scale_weight[0]; // misc.avg_mass;
                                 new_atom.hybrid.mw = (int)(new_atom.hybrid.mw * 100 + .5) / 100.0;
                                 new_atom.hybrid.radius *= atoms_scale_radius[0]; // misc.avg_radius;
                                 new_atom.hybrid.radius = (int)(new_atom.hybrid.radius * 100 + .5) / 100.0;
                                 new_atom.bead_assignment = 0; 
                                 new_atom.positioner = true;
                                 new_residue.r_atom.push_back(new_atom);
                              }
                           }
                        } 
                        else
                        {
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_bead.size(); i++ )
                           {
                              new_bead = residue_list[orgrespos].r_bead[i];
                              if ( beads_missing_atom_count[i] )
                              {
                                 // only set the beads with missing atoms
                                 new_bead.color = 10;
                                 new_bead.placing_method = 0;
                              }
                              new_residue.r_bead.push_back(new_bead);
                           }
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 new_atom = residue_list[orgrespos].r_atom[i];
                                 if ( beads_missing_atom_count[new_atom.bead_assignment] )
                                 {
                                    new_atom.hybrid.mw *= atoms_scale_weight[new_atom.bead_assignment]; // misc.avg_mass;
                                    new_atom.hybrid.mw = (int)(new_atom.hybrid.mw * 100 + .5) / 100.0;
                                    new_atom.hybrid.radius *= atoms_scale_radius[new_atom.bead_assignment]; // misc.avg_radius;
                                    new_atom.hybrid.radius = (int)(new_atom.hybrid.radius * 100 + .5) / 100.0;
                                    new_atom.positioner = true;
                                 }
                                 new_residue.r_atom.push_back(new_atom);
                              }
                           }
                        }
                        multi_residue_map[new_residue.name].push_back(residue_list.size());
                        residue_list.push_back(new_residue);
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;
#if defined(AUTO_BB_DEBUG)
                        printf("a1.1b <%s>\n", new_residue.name.ascii());
#endif
                        model->residue.push_back(residue_list[multi_residue_map[new_residue.name][0]]);
                     }
                     else
                     {
                        new_residue_name = QString("%1_%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
#if defined(AUTO_BB_DEBUG)
                        printf("a1.2 <%s>\n", new_residue_name.ascii());
#endif
                        if (new_residue_name != new_residue.name)
                        {
                           QMessageBox::message(tr("Internal error:"), 
                                                tr("Unexpected new residue name inconsistancy (a).\n") +
                                                QString("\"%1\" != \"%2\"").arg(new_residue_name).arg(new_residue.name));
                           exit(-1);
                        }
                     }
                     // the new residue should always be the last one, since we just added it
                     int respos = residue_list.size() - 1;
                     if (new_residue.name != residue_list[respos].name)
                     {
                        QMessageBox::message(tr("Internal error:"), 
                                             tr("Unexpected new residue name residue_list inconsistancy (a)."));
                        exit(-1);
                     }
                     // ok, now we can push back the modified atom
                     PDB_atom atom_to_add = org_model.molecule[j].atom[k];
                     atom_to_add.resName = new_residue.name;
#if defined(AUTO_BB_DEBUG)
                     printf("a1.3 <%s>\n", new_residue.name.ascii());
#endif
                     tmp_chain.atom.push_back(atom_to_add);
                  }
                  break;
                     
               default:
                  QMessageBox::message(tr("Internal error:"), 
                                       tr("Unhandled bead_exception code."));
                  exit(-1);
                  break;
               }
            }
            model->molecule.push_back(tmp_chain);
         }
         calc_vbar(model);
         update_vbar();
         printf("vbar after: %g\n", model->vbar);
      }
   }
#if defined(AUTO_BB_DEBUG) || 1
   QString str1;
   QFile f(somo_tmp_dir + SLASH + "tmp.somo.residue");
   if (f.open(IO_WriteOnly|IO_Translate))
   {
      QTextStream ts(&f);
      for (unsigned int i=0; i<residue_list.size(); i++)
      {
         ts << residue_list[i].comment << endl;
         ts << residue_list[i].name.upper()
            << "\t" << residue_list[i].type
            << "\t" << str1.sprintf("%7.2f", residue_list[i].molvol)
            << "\t" << residue_list[i].asa
            << "\t" << residue_list[i].r_atom.size()
            << "\t" << residue_list[i].r_bead.size()
            << "\t" << residue_list[i].vbar << endl;
         for (unsigned int j=0; j<residue_list[i].r_atom.size(); j++)
         {
            ts << residue_list[i].r_atom[j].name.upper()
               << "\t" << residue_list[i].r_atom[j].hybrid.name
               << "\t" << residue_list[i].r_atom[j].hybrid.mw
               << "\t" << residue_list[i].r_atom[j].hybrid.radius
               << "\t" << residue_list[i].r_atom[j].bead_assignment
               << "\t" << (unsigned int) residue_list[i].r_atom[j].positioner
               << "\t" << residue_list[i].r_atom[j].serial_number << endl;
         }
         for (unsigned int j=0; j<residue_list[i].r_bead.size(); j++)
         {
            ts << residue_list[i].r_bead[j].hydration
               << "\t" << residue_list[i].r_bead[j].color
               << "\t" << residue_list[i].r_bead[j].placing_method
               << "\t" << residue_list[i].r_bead[j].chain
               << "\t" << residue_list[i].r_bead[j].volume << endl;
         }
         str1.sprintf("%d: ", i+1);
         str1 += residue_list[i].name.upper();
      }
      f.close();
   }
#endif
   if (!misc.compute_vbar) 
   {
      editor->append(QString("vbar: %1 (User Entered)\n").arg(misc.vbar));
   }
   return 0;
}

// #define DEBUG_OVERLAP

// # define TOLERANCE 0.001       // this is used to place a limit on the allowed radial overlap
#define TOLERANCE overlap_tolerance

int US_Hydrodyn::overlap_check(bool sc, bool mc, bool buried)
{
   int retval = 0;
#if defined(DEBUG_OVERLAP)
   printf("overlap checking--overlap tolerance %f--%s-%s-%s--------------------\n",
          TOLERANCE,
          sc ? "side chain" : "",
          mc ? "main & side chain" : "",
          buried ? "buried" : "");
#endif
   for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
      for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG_OVERLAP_2)
         if(i == 2 && (j == 107 || j == 108))
            printf("x1 overlap check  beads %d %d on chains %d %d exposed code %d %d active %s %s : radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] sep of %f - %s\n",
                   i, j,
                   bead_model[i].chain,
                   bead_model[j].chain,
                   bead_model[i].exposed_code,
                   bead_model[j].exposed_code,
                   bead_model[i].active ? "Y" : "N",
                   bead_model[j].active ? "Y" : "N",
                   bead_model[i].bead_computed_radius,
                   bead_model[j].bead_computed_radius,
                   bead_model[i].bead_coordinate.axis[0],
                   bead_model[i].bead_coordinate.axis[1],
                   bead_model[i].bead_coordinate.axis[2],
                   bead_model[j].bead_coordinate.axis[0],
                   bead_model[j].bead_coordinate.axis[1],
                   bead_model[j].bead_coordinate.axis[2],
                   bead_model[i].bead_computed_radius +
                   bead_model[j].bead_computed_radius -
                   sqrt(
                        pow(bead_model[i].bead_coordinate.axis[0] -
                            bead_model[j].bead_coordinate.axis[0], 2) +
                        pow(bead_model[i].bead_coordinate.axis[1] -
                            bead_model[j].bead_coordinate.axis[1], 2) +
                        pow(bead_model[i].bead_coordinate.axis[2] -
                            bead_model[j].bead_coordinate.axis[2], 2)),
                   sqrt(
                        pow(bead_model[i].bead_coordinate.axis[0] -
                            bead_model[j].bead_coordinate.axis[0], 2) +
                        pow(bead_model[i].bead_coordinate.axis[1] -
                            bead_model[j].bead_coordinate.axis[1], 2) +
                        pow(bead_model[i].bead_coordinate.axis[2] -
                            bead_model[j].bead_coordinate.axis[2], 2))
                   <= TOLERANCE ? "ok" : "needs reduction"
                   );
#endif
         if (bead_model[i].active &&
             bead_model[j].active
             &&
             (mc ||
              (sc &&
               bead_model[i].chain == 1 &&
               bead_model[j].chain == 1))
             &&
             (buried ||
              (bead_model[i].exposed_code == 1 &&
               bead_model[j].exposed_code == 1)) &&
             bead_model[i].bead_computed_radius > TOLERANCE &&
             bead_model[j].bead_computed_radius > TOLERANCE
             ) {
            float separation =
               bead_model[i].bead_computed_radius +
               bead_model[j].bead_computed_radius -
               sqrt(
                    pow(bead_model[i].bead_coordinate.axis[0] -
                        bead_model[j].bead_coordinate.axis[0], 2) +
                    pow(bead_model[i].bead_coordinate.axis[1] -
                        bead_model[j].bead_coordinate.axis[1], 2) +
                    pow(bead_model[i].bead_coordinate.axis[2] -
                        bead_model[j].bead_coordinate.axis[2], 2));
#if defined(DEBUG_OVERLAP_2)
            if(i == 2 && (j == 107 || j == 108))
               printf("x2 overlap check  beads %d %d on chains %d %d exposed code %d %d active %s %s : radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] sep of %f - %s\n",
                      i, j,
                      bead_model[i].chain,
                      bead_model[j].chain,
                      bead_model[i].exposed_code,
                      bead_model[j].exposed_code,
                      bead_model[i].active ? "Y" : "N",
                      bead_model[j].active ? "Y" : "N",
                      bead_model[i].bead_computed_radius,
                      bead_model[j].bead_computed_radius,
                      bead_model[i].bead_coordinate.axis[0],
                      bead_model[i].bead_coordinate.axis[1],
                      bead_model[i].bead_coordinate.axis[2],
                      bead_model[j].bead_coordinate.axis[0],
                      bead_model[j].bead_coordinate.axis[1],
                      bead_model[j].bead_coordinate.axis[2],
                      separation,
                      separation <= TOLERANCE ? "ok" : "needs reduction"
                      );
#endif
            if (separation <= TOLERANCE) {
               continue;
            }
            retval++;
#if defined(DEBUG_OVERLAP)
            printf("overlap check  beads %d %d on chains %d %d exposed code %d %d active %s %s : radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] sep of %f - needs reduction\n",
                   i, j,
                   bead_model[i].chain,
                   bead_model[j].chain,
                   bead_model[i].exposed_code,
                   bead_model[j].exposed_code,
                   bead_model[i].active ? "Y" : "N",
                   bead_model[j].active ? "Y" : "N",
                   bead_model[i].bead_computed_radius,
                   bead_model[j].bead_computed_radius,
                   bead_model[i].bead_coordinate.axis[0],
                   bead_model[i].bead_coordinate.axis[1],
                   bead_model[i].bead_coordinate.axis[2],
                   bead_model[j].bead_coordinate.axis[0],
                   bead_model[j].bead_coordinate.axis[1],
                   bead_model[j].bead_coordinate.axis[2],
                   separation
                   );
#endif
         }
      }
   }
#if defined(DEBUG_OVERLAP)
   printf("end of overlap checking--overlap tolerance %f--%s-%s-%s--violations %d------------------\n",
          TOLERANCE,
          sc ? "side chain" : "",
          mc ? "main & side chain" : "",
          buried ? "buried" : "",
          retval);
#endif
   return retval;
}

class sortable_PDB_atom {
public:
   PDB_atom pdb_atom;
   bool PRO_N_override;
   bool operator < (const sortable_PDB_atom& objIn) const
   {
      if (
          //   (PRO_N_override ? 0 : pdb_atom.bead_assignment)
          pdb_atom.atom_assignment
          <
          //   (objIn.PRO_N_override ? 0 : objIn.pdb_atom.bead_assignment)
          objIn.pdb_atom.atom_assignment
          )
      {
         return (true);
      }
      else
      {
         return (false);
      }
   }
};

// #define DEBUG
int US_Hydrodyn::create_beads(QString *error_string)
{

   editor->append("Creating beads from model\n");
   qApp->processEvents();
   active_atoms.clear();

   // #define DEBUG_MM
#if defined(DEBUG_MM)
   {
      int no_of_atoms = 0;
      int no_of_molecules = model_vector[current_model].molecule.size();
      int i;
      for (i = 0; i < no_of_molecules; i++) {
         no_of_atoms +=  model_vector[current_model].molecule[i].atom.size();
      }

      editor->append(QString("There are %1 atoms in %2 molecule(s) in this model\n").arg(no_of_atoms).arg(no_of_molecules));
   }
#endif
   get_atom_map(&model_vector[current_model]);

   for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
   {
      QString last_resSeq = "";
      QString last_resName = "";
      for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
      {
         PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

         QString count_idx =
            QString("%1|%2|%3")
            .arg(j)
            .arg(this_atom->resName)
            .arg(this_atom->resSeq);

         // initialize data
         this_atom->active = false;
         this_atom->asa = 0;
         this_atom->p_residue = 0;
         this_atom->p_atom = 0;
         this_atom->radius = 0;
         this_atom->bead_hydration = 0;
         this_atom->bead_color = 0;
         this_atom->bead_ref_volume = 0;
         this_atom->bead_ref_mw = 0;
         this_atom->bead_assignment = -1;
         this_atom->atom_assignment = -1;
         this_atom->chain = -1;

         // find residue in residues
         int respos = -1;
#if defined(DEBUG) || defined(AUTO_BB_DEBUG)
         printf("residue search name %s resName %s\n",
                this_atom->name.ascii(),
                this_atom->resName.ascii());
#endif
         for (unsigned int m = 0; m < residue_list.size(); m++)
         {
            if ((residue_list[m].name == this_atom->resName &&
                 (int)residue_list[m].r_atom.size() ==
                 atom_counts[count_idx] - has_OXT[count_idx] &&
                 this_atom->name != "OXT" &&
                 (k || this_atom->name != "N" || !regular_N_handling)) ||
                (residue_list[m].name == "OXT"
                 && this_atom->name == "OXT") ||
                (!k &&
                 this_atom->name == "N" &&
                 regular_N_handling &&
                 residue_list[m].name == "N1"))
            {
               respos = (int) m;
               this_atom->p_residue = &(residue_list[m]);
#if defined(DEBUG) || defined(AUTO_BB_DEBUG)
               printf("residue match %d resName %s \n", m, residue_list[m].name.ascii());
#endif
               break;
            }
         }
         if (respos == -1)
         {
            if ((this_atom->name != "H" && this_atom->name != "D"
                 && this_atom->resName != "DOD"
                 && this_atom->resName != "HOH" && (this_atom->altLoc == "A" || this_atom->altLoc == " ")))
            {
               error_string->append(QString("").sprintf("unknown residue molecule %d atom %d name %s resname %s coord [%f,%f,%f]\n",
                                                        j + 1, k, this_atom->name.ascii(),
                                                        this_atom->resName.ascii(),
                                                        this_atom->coordinate.axis[0], this_atom->coordinate.axis[1], this_atom->coordinate.axis[2]));
               return (US_SURFRACER_ERR_MISSING_RESIDUE);
            }
         } 
         else 
         {
#if defined(DEBUG)
            printf("found residue pos %d\n", respos);
#endif
         }
         int atompos = -1;

         if (respos != -1)
         {
            // clear tmp_used if new resSeq
#if defined(DEBUG)
            printf("respos %d != -1 last used %s %s\n", respos, this_atom->resSeq.ascii(), last_resSeq.ascii());
#endif
            if (this_atom->resSeq != last_resSeq ||
                this_atom->resName != last_resName ||
                residue_list[respos].name == "OXT" ||
                residue_list[respos].name == "N1")
            {
#if defined(DEBUG)
               printf("clear last used %s %s\n", this_atom->resSeq.ascii(), last_resSeq.ascii());
#endif
               for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
               {
                  residue_list[respos].r_atom[m].tmp_used = false;
               }
               last_resSeq = this_atom->resSeq;
               last_resName = this_atom->resName;
               if(residue_list[respos].name == "OXT" ||
                  residue_list[respos].name == "N1") {
                  last_resSeq = "";
               }
            }
#if defined(DEBUG)
            for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
            {
               if(residue_list[respos].r_atom[m].tmp_used) {
                  printf("used %u %u\n", respos, m);
               }
            }
#endif

            for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
            {
#if defined(DEBUG)
               if(this_atom->name == "N" && !k && regular_N_handling) {
                  printf("this_atom->name == N/N1 this residue_list[%d].r_atom[%d].name == %s\n",
                         respos, m, residue_list[respos].r_atom[m].name.ascii());
               }
#endif
               if (!residue_list[respos].r_atom[m].tmp_used &&
                   (residue_list[respos].r_atom[m].name == this_atom->name ||
                    (
                     this_atom->name == "N" &&
                     regular_N_handling &&
                     !k &&
                     residue_list[respos].r_atom[m].name == "N1"
                     )
                    )
                   )
               {
                  residue_list[respos].r_atom[m].tmp_used = true;
                  this_atom->p_atom = &(residue_list[respos].r_atom[m]);
                  atompos = (int) m;
                  break;
               }
            }

            if (atompos == -1)
            {
               error_string->append(QString("").sprintf("unknown atom molecule %d atom %d name %s resname %s coord [%f,%f,%f]\n",
                                                        j + 1, k, this_atom->name.ascii(),
                                                        this_atom->resName.ascii(),
                                                        this_atom->coordinate.axis[0], this_atom->coordinate.axis[1], this_atom->coordinate.axis[2]));
            } 
            else 
            {
               this_atom->active = true;
               this_atom->radius = residue_list[respos].r_atom[atompos].hybrid.radius;
               this_atom->mw = residue_list[respos].r_atom[atompos].hybrid.mw;
               this_atom->placing_method =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].placing_method;
               this_atom->bead_hydration =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].hydration;
               this_atom->bead_color =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].color;
               this_atom->bead_ref_volume =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].volume;
               this_atom->bead_ref_mw =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].mw;
               this_atom->ref_asa =  this_atom->p_residue->asa;
               this_atom->bead_computed_radius =  pow(3 * this_atom->bead_ref_volume / (4.0*M_PI), 1.0/3);
               this_atom->bead_assignment = this_atom->p_atom->bead_assignment;
               this_atom->atom_assignment = atompos;
               this_atom->chain = (int) this_atom->p_residue->r_bead[this_atom->bead_assignment].chain;

#if defined(DEBUG) || defined(AUTO_BB_DEBUG)
               printf("found atom %s %s in residue %d pos %d bead asgn %d %d placing info %d mw %f bead_ref_mw %f hybrid name %s %s ba %d\n",
                      this_atom->name.ascii(), this_atom->resName.ascii(), respos, atompos,
                      this_atom->p_atom->bead_assignment,
                      this_atom->bead_assignment,
                      this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].placing_method,
                      this_atom->mw,
                      this_atom->bead_ref_mw,
                      residue_list[respos].r_atom[atompos].hybrid.name.ascii(),
                      this_atom->p_atom->hybrid.name.ascii(),
                      this_atom->p_atom->bead_assignment
                      );
#endif
            }
         }

         if (this_atom->active)
         {
#if defined(DEBUG) || defined(AUTO_BB_DEBUG)
            puts("active atom"); fflush(stdout);
#endif
            this_atom->active = false;
            if (this_atom->name != "H" &&
                this_atom->name != "D" &&
                this_atom->p_residue->name != "DOD" &&
                this_atom->p_residue->name != "HOH" && (this_atom->altLoc == "A" || this_atom->altLoc == " "))
            {
               this_atom->active = true;
               active_atoms.push_back(this_atom);
            }
            else 
            {
#if defined(DEBUG)
               printf
                  ("skipped bound waters & H %s %s rad %f resseq %s\n",
                   this_atom->name.ascii(), this_atom->resName.ascii(), this_atom->radius, this_atom->resSeq.ascii());
               fflush(stdout);
#endif
            }
         }
      }
   }

#if defined(DEBUG)
   puts("before sort::");
#endif
   {
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];

#if defined(DEBUG) || defined(AUTO_BB_DEBUG)
            printf("p1 j k %d %d %lx %s %d\n", j, k,
                   (long unsigned int)this_atom->p_atom,
                   this_atom->active ? "active" : "not active",
                   this_atom->bead_assignment
                   ); fflush(stdout);
#endif

            // this_atom->bead_assignment =
            // (this_atom->p_atom ? (int) this_atom->p_atom->bead_assignment : -1);
            // this_atom->chain =
            // ((this_atom->p_residue && this_atom->p_atom) ?
            //  (int) this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].chain : -1);
            this_atom->org_chain = this_atom->chain;
            this_atom->bead_positioner = this_atom->p_atom ? this_atom->p_atom->positioner : false;
         }
      }
   }
#define DO_SORT
#if defined(DO_SORT)
   // reorder residue
   {
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
         QString last_resSeq = "";
         list <sortable_PDB_atom> last_residue_atoms;
         unsigned int k;
         for (k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];

            if (this_atom->resSeq != last_resSeq) {
               if (last_resSeq != "") {
                  // reorder previous residue
                  last_residue_atoms.sort();
                  int base_ofs = (int) last_residue_atoms.size();
                  // printf("resort last residue... size/base %d k %u\n", base_ofs, k);
                  for (unsigned int m = k - base_ofs; m < k; m++) {
                     // printf("resort m = %u size lra %u\n", m, last_residue_atoms.size());
                     model_vector[current_model].molecule[j].atom[m] = (last_residue_atoms.front()).pdb_atom;
                     last_residue_atoms.pop_front();
                  }
               }
               last_resSeq = this_atom->resSeq;
            }
            sortable_PDB_atom tmp_sortable_pdb_atom;
            tmp_sortable_pdb_atom.pdb_atom = *this_atom;
            tmp_sortable_pdb_atom.PRO_N_override = false;
            if(this_atom->resName == "PRO" &&
               this_atom->name == "N") {
               tmp_sortable_pdb_atom.PRO_N_override = true;
            }
            last_residue_atoms.push_back(tmp_sortable_pdb_atom);
         }
         if (last_resSeq != "") {
            // reorder 'last' residue
            last_residue_atoms.sort();
            int base_ofs = (int) last_residue_atoms.size();
            // printf("final resort last residue... size/base %d k %u\n", base_ofs, k);
            for (unsigned int m = k - base_ofs; m < k; m++) {
               //       printf("finalresort m = %u size lra %u\n", m, last_residue_atoms.size());
               model_vector[current_model].molecule[j].atom[m] = (last_residue_atoms.front()).pdb_atom;
               last_residue_atoms.pop_front();
            }
         }
      }
   }
#endif
#if !defined(DO_SORT)
   puts("sorting disabled");
#endif

#if defined(DEBUG)
   puts("after sort::");
   // list them again...
   {
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];

            printf("p1 j k %d %d %lx %s %d %d\n", j, k,
                   (long unsigned int)this_atom->p_atom,
                   this_atom->active ? "active" : "not active",
                   this_atom->bead_assignment,
                   this_atom->atom_assignment
                   ); fflush(stdout);
         }
      }
   }
#endif

   // #define DEBUG_MW
#if defined(DEBUG_MW)
   puts("mw totals:::");
   double tot_atom_mw = 0e0;
   double tot_bead_mw = 0e0;
   double prev_bead_mw = 0e0;
   double prev_atom_mw = 0e0;
   int bead_count = 0;
   int atom_count = 0;
   int last_asgn = -1;
   int last_res = "";
   QString last_resName = "";
   {
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];
            if(this_atom->active) {
               if(last_asgn != (int)this_atom->bead_assignment ||
                  last_res != this_atom->resSeq) {
                  if(bead_count) {
                     printf("res %s %s bead %d bead_mw %f sum atom mw %f diff %f\n",
                            last_res.ascii(),
                            last_resName.ascii(),
                            last_asgn,
                            prev_bead_mw,
                            prev_atom_mw,
                            prev_bead_mw -
                            prev_atom_mw
                            );
                  }
                  prev_bead_mw = 0;
                  prev_atom_mw = 0;

                  bead_count++;
                  tot_bead_mw += this_atom->bead_ref_mw;
                  prev_bead_mw += this_atom->bead_ref_mw;
                  last_asgn = (int)this_atom->bead_assignment;
                  last_res = this_atom->resSeq;
                  last_resName = this_atom->resName;
               }
               atom_count++;
               tot_atom_mw += this_atom->mw;
               prev_atom_mw += this_atom->mw;
            }
         }
      }
      printf("res %s %s bead %d bead_mw %f sum atom mw %f diff %f\n",
             last_res.ascii(),
             last_resName.ascii(),
             last_asgn,
             prev_bead_mw,
             prev_atom_mw,
             prev_bead_mw -
             prev_atom_mw
             );
   }
   printf("~~tot atom %d mw %f tot bead %d mw %f\n",
          atom_count,
          tot_atom_mw,
          bead_count,
          tot_bead_mw);
#endif

   return 0;
}

# define POP_MC              (1 << 0)
# define POP_SC              (1 << 1)
# define POP_MCSC            (1 << 2)
# define POP_EXPOSED         (1 << 3)
# define POP_BURIED          (1 << 4)
# define POP_ALL             (1 << 5)
# define RADIAL_REDUCTION    (1 << 6)
# define RR_MC               (1 << 7)
# define RR_SC               (1 << 8)
# define RR_MCSC             (1 << 9)
# define RR_EXPOSED          (1 << 10)
# define RR_BURIED           (1 << 11)
# define RR_ALL              (1 << 12)
# define OUTWARD_TRANSLATION (1 << 13)
# define RR_HIERC            (1 << 14)
# define MIN_OVERLAP 0.0

void US_Hydrodyn::radial_reduction()
{
   // popping radial reduction

   // or sc fb Y rh Y rs Y to Y st Y ro Y 70.000000 1.000000 0.000000
   // or scmc fb Y rh N rs N to Y st N ro Y 1.000000 0.000000 70.000000
   // or bb fb Y rh Y rs N to N st Y ro N 0.000000 0.000000 0.000000
   float molecular_cog[3] = { 0, 0, 0 };
   float molecular_mw = 0;

   for (unsigned int i = 0; i < bead_model.size(); i++) {
      PDB_atom *this_atom = &bead_model[i];
      if (this_atom->active) {
         molecular_mw += this_atom->bead_mw;
         for (unsigned int m = 0; m < 3; m++) {
            molecular_cog[m] += this_atom->bead_coordinate.axis[m] * this_atom->bead_mw;
         }
      }
   }
   if (molecular_mw) {
      for (unsigned int m = 0; m < 3; m++) {
         molecular_cog[m] /= molecular_mw;
      }
   } 
   else 
   {
      printf("ERROR: this molecule has zero mw!\n");
   }

#if defined(DEBUG)
   printf("or sc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or scmc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or bb fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or grid fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n",
          sidechain_overlap.fuse_beads ? "Y" : "N",
          sidechain_overlap.remove_hierarch ? "Y" : "N",
          sidechain_overlap.remove_sync ? "Y" : "N",
          sidechain_overlap.translate_out ? "Y" : "N",
          sidechain_overlap.show_translate ? "Y" : "N",
          sidechain_overlap.remove_overlap ? "Y" : "N",
          sidechain_overlap.fuse_beads_percent,
          sidechain_overlap.remove_sync_percent,
          sidechain_overlap.remove_hierarch_percent,

          mainchain_overlap.fuse_beads ? "Y" : "N",
          mainchain_overlap.remove_hierarch ? "Y" : "N",
          mainchain_overlap.remove_sync ? "Y" : "N",
          mainchain_overlap.translate_out ? "Y" : "N",
          mainchain_overlap.show_translate ? "Y" : "N",
          mainchain_overlap.remove_overlap ? "Y" : "N",
          mainchain_overlap.fuse_beads_percent,
          mainchain_overlap.remove_sync_percent,
          mainchain_overlap.remove_hierarch_percent,

          buried_overlap.fuse_beads ? "Y" : "N",
          buried_overlap.remove_hierarch ? "Y" : "N",
          buried_overlap.remove_sync ? "Y" : "N",
          buried_overlap.translate_out ? "Y" : "N",
          buried_overlap.show_translate ? "Y" : "N",
          buried_overlap.remove_overlap ? "Y" : "N",
          buried_overlap.fuse_beads_percent,
          buried_overlap.remove_sync_percent,
          buried_overlap.remove_hierarch_percent,

          grid_overlap.fuse_beads ? "Y" : "N",
          grid_overlap.remove_hierarch ? "Y" : "N",
          grid_overlap.remove_sync ? "Y" : "N",
          grid_overlap.translate_out ? "Y" : "N",
          grid_overlap.show_translate ? "Y" : "N",
          grid_overlap.remove_overlap ? "Y" : "N",
          grid_overlap.fuse_beads_percent,
          grid_overlap.remove_sync_percent,
          grid_overlap.remove_hierarch_percent);
#endif

   int methods[] =
      {
         RADIAL_REDUCTION | RR_SC | RR_EXPOSED,
         RADIAL_REDUCTION | RR_MCSC | RR_EXPOSED,
         RADIAL_REDUCTION | RR_MCSC | RR_BURIED,
         RADIAL_REDUCTION | RR_MCSC | RR_BURIED,
      };

   if (no_rr) {
      methods[0] = 0;
      methods[1] = 0;
      methods[2] = 0;
      methods[3] = 0;
   }

   if (sidechain_overlap.fuse_beads) {
      methods[0] |= POP_SC | POP_EXPOSED;
   }

   if (mainchain_overlap.fuse_beads) {
      methods[1] |= POP_MCSC | POP_EXPOSED;
   }

   if (buried_overlap.fuse_beads) {
      methods[2] |= POP_ALL | POP_BURIED;
   }

   if (grid_overlap.fuse_beads) {
      methods[3] |= POP_ALL | POP_BURIED;
   }

   if (sidechain_overlap.remove_hierarch) {
      methods[0] |= RR_HIERC;
   }

   if (mainchain_overlap.remove_hierarch) {
      methods[1] |= RR_HIERC;
   }

   if (buried_overlap.remove_hierarch) {
      methods[2] |= RR_HIERC;
   }

   if (grid_overlap.remove_hierarch) {
      methods[3] |= RR_HIERC;
   }

   if (sidechain_overlap.translate_out) {
      methods[0] |= OUTWARD_TRANSLATION;
   }

   if (mainchain_overlap.translate_out) {
      methods[1] |= OUTWARD_TRANSLATION;
   }

   if (buried_overlap.translate_out) {
      methods[2] |= OUTWARD_TRANSLATION;
   }

   if (grid_overlap.translate_out) {
      methods[3] |= OUTWARD_TRANSLATION;
   }

   if (!sidechain_overlap.remove_overlap) {
      methods[0] = 0;
   }

   if (!mainchain_overlap.remove_overlap) {
      methods[1] = 0;
   }

   if (!buried_overlap.remove_overlap) {
      methods[2] = 0;
   }

   if (!grid_overlap.remove_overlap) {
      methods[3] = 0;
   }

   float overlap[] =
      {
         sidechain_overlap.fuse_beads_percent / 100.0,
         mainchain_overlap.fuse_beads_percent / 100.0,
         buried_overlap.fuse_beads_percent / 100.0,
         grid_overlap.fuse_beads_percent / 100.0
      };

   float rr_overlap[] =
      {
         (sidechain_overlap.remove_hierarch ?
          sidechain_overlap.remove_hierarch_percent : sidechain_overlap.remove_sync_percent) / 100.0,
         (mainchain_overlap.remove_hierarch ?
          mainchain_overlap.remove_hierarch_percent : mainchain_overlap.remove_sync_percent) / 100.0,
         (buried_overlap.remove_hierarch ?
          buried_overlap.remove_hierarch_percent : buried_overlap.remove_sync_percent) / 100.0,
         (grid_overlap.remove_hierarch ?
          grid_overlap.remove_hierarch_percent : grid_overlap.remove_sync_percent) / 100.0
      };


#if defined(WRITE_EXTRA_FILES)
   write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO + ".tsv", &bead_model);
   write_bead_spt(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO, &bead_model);
#endif
   for(unsigned int k = 3; k < sizeof(methods) / sizeof(int); k++) {
      // only grid method

   stage_loop:

      qApp->processEvents();
      if (stopFlag)
      {
         return;
      }
      int beads_popped = 0;

#if defined(DEBUG1) || defined(DEBUG)
      printf("popping stage %d %s%s%s%s%s%s%s%s%s%s%s%s%s%s%soverlap_reduction %f rroverlap %f\n",
             k,
             (methods[k] & POP_MC) ? "main chain " : "",
             (methods[k] & POP_SC) ? "side chain " : "",
             (methods[k] & POP_MCSC) ? "main & side chain " : "",
             (methods[k] & POP_EXPOSED) ? "exposed " : "",
             (methods[k] & POP_BURIED) ? "buried " : "",
             (methods[k] & POP_ALL) ? "all " : "",
             (methods[k] & RADIAL_REDUCTION) ? "radial reduction " : "",
             (methods[k] & RR_HIERC) ? "hierarchically " : "synchronously ",
             (methods[k] & RR_MC) ? "main chain " : "",
             (methods[k] & RR_SC) ? "side chain " : "",
             (methods[k] & RR_MCSC) ? "main & side chain " : "",
             (methods[k] & RR_EXPOSED) ? "exposed " : "",
             (methods[k] & RR_BURIED) ? "buried " : "",
             (methods[k] & RR_ALL) ? "all " : "",
             (methods[k] & OUTWARD_TRANSLATION) ? "outward translation " : "",
             overlap[k],
             rr_overlap[k]);
#endif

      if (overlap[k] < MIN_OVERLAP) {
         printf("using %f as minimum overlap\n", MIN_OVERLAP);
         overlap[k] = MIN_OVERLAP;
      }

      float max_intersection_volume;
      float intersection_volume = 0;
      int max_bead1 = 0;
      int max_bead2 = 0;
#if defined(DEBUG1) || defined(DEBUG)
      unsigned iter = 0;
#endif
      bool overlaps_exist;
#if defined(TIMING)
      gettimeofday(&start_tv, NULL);
#endif
      editor->append(QString("Begin popping stage %1\n").arg(k + 1));
      qApp->processEvents();

      do {
         qApp->processEvents();
         if (stopFlag)
         {
            return;
         }
         // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_bp-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
         // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-bp-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
#if defined(DEBUG1) || defined(DEBUG)
         printf("popping iteration %d\n", iter++);
#endif
         max_intersection_volume = -1;
         overlaps_exist = false;
         if (methods[k] & (POP_MC | POP_SC | POP_MCSC | POP_EXPOSED | POP_BURIED | POP_ALL)) {
            for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
               for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG)
                  printf("checking popping stage %d beads %d %d on chains %d %d exposed %d %d active %s %s max iv %f\n",
                         k, i, j,
                         bead_model[i].chain,
                         bead_model[j].chain,
                         bead_model[i].exposed_code,
                         bead_model[j].exposed_code,
                         bead_model[i].active ? "Y" : "N",
                         bead_model[j].active ? "Y" : "N",
                         max_intersection_volume
                         );
#endif
                  if (bead_model[i].active &&
                      bead_model[j].active &&
                      ( ((methods[k] & POP_SC) &&
                         bead_model[i].chain == 1 &&
                         bead_model[j].chain == 1) ||
                        ((methods[k] & POP_MC) &&
                         bead_model[i].chain == 0 &&
                         bead_model[j].chain == 0) ||
                        ((methods[k] & POP_MCSC)
                         // &&
                         // (bead_model[i].chain != 1 ||
                         // bead_model[j].chain != 1))
                         ) ) &&
                      ( ((methods[k] & POP_EXPOSED) &&
                         bead_model[i].exposed_code == 1 &&
                         bead_model[j].exposed_code == 1) ||
                        ((methods[k] & POP_BURIED) &&
                         (bead_model[i].exposed_code != 1 ||
                          bead_model[j].exposed_code != 1)) ||
                        (methods[k] & POP_ALL) )) {
                     intersection_volume =
                        int_vol_2sphere(
                                        bead_model[i].bead_computed_radius,
                                        bead_model[j].bead_computed_radius,
                                        sqrt(
                                             pow(bead_model[i].bead_coordinate.axis[0] -
                                                 bead_model[j].bead_coordinate.axis[0], 2) +
                                             pow(bead_model[i].bead_coordinate.axis[1] -
                                                 bead_model[j].bead_coordinate.axis[1], 2) +
                                             pow(bead_model[i].bead_coordinate.axis[2] -
                                                 bead_model[j].bead_coordinate.axis[2], 2)) );
#if defined(DEBUG)
                     printf("this overlap bead %u %u vol %f rv1 %f rv2 %f r1 %f r2 %f p1 [%f,%f,%f] p2 [%f,%f,%f]\n",
                            bead_model[i].serial,
                            bead_model[j].serial,
                            intersection_volume,
                            bead_model[i].bead_ref_volume,
                            bead_model[j].bead_ref_volume,
                            bead_model[i].bead_computed_radius,
                            bead_model[j].bead_computed_radius,
                            bead_model[i].bead_coordinate.axis[0],
                            bead_model[i].bead_coordinate.axis[1],
                            bead_model[i].bead_coordinate.axis[2],
                            bead_model[j].bead_coordinate.axis[0],
                            bead_model[j].bead_coordinate.axis[1],
                            bead_model[j].bead_coordinate.axis[2]
                            );
#endif
                     if (intersection_volume > bead_model[i].bead_ref_volume * overlap[k] ||
                         intersection_volume > bead_model[j].bead_ref_volume * overlap[k]) {
                        overlaps_exist = true;
                        if (intersection_volume > max_intersection_volume) {
#if defined(DEBUG)
                           printf("best overlap so far bead %u %u vol %f\n",
                                  bead_model[i].serial,
                                  bead_model[j].serial,
                                  intersection_volume);
#endif
                           max_intersection_volume = intersection_volume;
                           max_bead1 = i;
                           max_bead2 = j;
                        }
                     }
                  }
               }
            }

            bool back_to_zero = false;
            if (overlaps_exist) {
               beads_popped++;
               //#define DEBUG_FUSED
#if defined(DEBUG1) || defined(DEBUG) || defined(DEBUG_FUSED)
               printf("popping beads %u %u int vol %f mw1 %f mw2 %f v1 %f v2 %f c1 [%f,%f,%f] c2 [%f,%f,%f]\n",
                      max_bead1,
                      max_bead2,
                      max_intersection_volume,
                      bead_model[max_bead1].bead_ref_mw,
                      bead_model[max_bead2].bead_ref_mw,
                      bead_model[max_bead1].bead_ref_volume,
                      bead_model[max_bead2].bead_ref_volume,
                      bead_model[max_bead1].bead_coordinate.axis[0],
                      bead_model[max_bead1].bead_coordinate.axis[1],
                      bead_model[max_bead1].bead_coordinate.axis[2],
                      bead_model[max_bead2].bead_coordinate.axis[0],
                      bead_model[max_bead2].bead_coordinate.axis[1],
                      bead_model[max_bead2].bead_coordinate.axis[2]
                      );
#endif
               if (bead_model[max_bead1].chain == 0 &&
                   bead_model[max_bead2].chain == 1) {
                  // always select the sc!
#if defined(DEBUG1) || defined(DEBUG) || defined(DEBUG_FUSED)
                  puts("swap beads");
#endif
                  int tmp = max_bead2;
                  max_bead2 = max_bead1;
                  max_bead1 = tmp;
               }
               if (bead_model[max_bead1].chain != bead_model[max_bead2].chain &&
                   k == 1) {
                  back_to_zero = true;
               }
               // bead_model[max_bead1].all_beads.push_back(&bead_model[max_bead1]); ??
               bead_model[max_bead1].all_beads.push_back(&bead_model[max_bead2]);
               for (unsigned int n = 0; n < bead_model[max_bead2].all_beads.size(); n++) {
                  bead_model[max_bead1].all_beads.push_back(bead_model[max_bead2].all_beads[n]);
               }

               bead_model[max_bead2].active = false;
               for (unsigned int m = 0; m < 3; m++) {
                  bead_model[max_bead1].bead_coordinate.axis[m] *= bead_model[max_bead1].bead_ref_mw;
                  bead_model[max_bead1].bead_coordinate.axis[m] +=
                     bead_model[max_bead2].bead_coordinate.axis[m] * bead_model[max_bead2].bead_ref_mw;
                  bead_model[max_bead1].bead_coordinate.axis[m] /= bead_model[max_bead1].bead_ref_mw + bead_model[max_bead2].bead_ref_mw;
               }
               bead_model[max_bead1].bead_ref_mw = bead_model[max_bead1].bead_ref_mw + bead_model[max_bead2].bead_ref_mw;
               bead_model[max_bead1].bead_ref_volume = bead_model[max_bead1].bead_ref_volume + bead_model[max_bead2].bead_ref_volume;
               // - max_intersection_volume;
               bead_model[max_bead1].bead_actual_radius =
                  bead_model[max_bead1].bead_computed_radius =
                  pow(3 * bead_model[max_bead1].bead_ref_volume / (4.0*M_PI), 1.0/3);
               // if fusing with a side chain bead, make sure the fused is side-chain
               // if (bead_model[max_bead2].chain) {
               //   bead_model[max_bead1].chain = 1;
               // }
               bead_model[max_bead1].normalized_ot_is_valid = false;
               bead_model[max_bead2].normalized_ot_is_valid = false;
#if defined(DEBUG)
               printf("after popping beads %d %d int volume %f radius %f mw %f vol %f coordinate [%f,%f,%f]\n",
                      bead_model[max_bead1].serial,
                      bead_model[max_bead2].serial,
                      intersection_volume,
                      bead_model[max_bead1].bead_computed_radius,
                      bead_model[max_bead1].bead_ref_mw,
                      bead_model[max_bead2].bead_ref_volume,
                      bead_model[max_bead1].bead_coordinate.axis[0],
                      bead_model[max_bead1].bead_coordinate.axis[1],
                      bead_model[max_bead1].bead_coordinate.axis[2]
                      );
#endif
               if (back_to_zero) {
                  editor->append(QString("Beads popped %1, Go back to stage %2\n").arg(beads_popped).arg(k));
                  printf("fused sc/mc bead in stage SC/MC, back to stage SC\n");
                  k = 0;
                  goto stage_loop;
               }
            }
         } // if pop method
         // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_ap-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
         // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-ap-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
         qApp->processEvents();
         if (stopFlag)
         {
            return;
         }
      } while(overlaps_exist);
#if defined(TIMING)
      gettimeofday(&end_tv, NULL);
      printf("popping %d time %lu\n",
             k,
             1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
             start_tv.tv_usec);
      fflush(stdout);
#endif
#if defined(WRITE_EXTRA_FILES)
      write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
      write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO, &bead_model);
#endif
      printf("stage %d beads popped %d\n", k, beads_popped);
      editor->append(QString("Beads popped %1.\nBegin radial reduction stage %2\n").arg(beads_popped).arg(k + 1));
      qApp->processEvents();


      // radial reduction phase
#if defined(TIMING)
      gettimeofday(&start_tv, NULL);
#endif

      if (methods[k] & RADIAL_REDUCTION) {
         BPair pair;
         vector <BPair> pairs;

         vector <bool> reduced;
         vector <bool> reduced_any; // this is for a final recomputation of the volumes
         vector <bool> last_reduced; // to prevent rescreening
         reduced.resize(bead_model.size());
         reduced_any.resize(bead_model.size());
         last_reduced.resize(bead_model.size());

         for (unsigned int i = 0; i < bead_model.size(); i++) {
            reduced_any[i] = false;
            last_reduced[i] = true;
         }

         int iter = 0;
         int count;
         float max_intersection_length;
         // bool tb[bead_model.size() * bead_model.size()];
         // printf("sizeof tb %d, bm.size^2 %d\n",
         //     sizeof(tb), bead_model.size() * bead_model.size());
#if defined(DEBUG_OVERLAP)
         overlap_check(methods[k] & RR_SC ? true : false,
                       methods[k] & RR_MCSC ? true : false,
                       methods[k] & RR_BURIED ? true : false);
#endif
         if (methods[k] & RR_HIERC) {
#if defined(DEBUG1) || defined(DEBUG)
            printf("preprocessing processing hierarchical radial reduction\n");
#endif
            max_intersection_length = 0;
            pairs.clear();
            count = 0;
            // build list of intersecting pairs
            for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
               for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG)
                  printf("checking radial stage %d beads %d %d on chains %d %d exposed code %d %d active %s %s max il %f\n",
                         k, i, j,
                         bead_model[i].chain,
                         bead_model[j].chain,
                         bead_model[i].exposed_code,
                         bead_model[j].exposed_code,
                         bead_model[i].active ? "Y" : "N",
                         bead_model[j].active ? "Y" : "N",
                         max_intersection_length
                         );
#endif
                  if (
                      bead_model[i].active &&
                      bead_model[j].active &&
                      (methods[k] & RR_MCSC ||
                       ((methods[k] & RR_SC) &&
                        bead_model[i].chain == 1 &&
                        bead_model[j].chain == 1)) &&
                      ((methods[k] & RR_BURIED) ||
                       (bead_model[i].exposed_code == 1 ||
                        bead_model[j].exposed_code == 1)) &&
                      bead_model[i].bead_computed_radius > TOLERANCE &&
                      bead_model[j].bead_computed_radius > TOLERANCE
                      ) {

                     float separation =
                        bead_model[i].bead_computed_radius +
                        bead_model[j].bead_computed_radius -
                        sqrt(
                             pow(bead_model[i].bead_coordinate.axis[0] -
                                 bead_model[j].bead_coordinate.axis[0], 2) +
                             pow(bead_model[i].bead_coordinate.axis[1] -
                                 bead_model[j].bead_coordinate.axis[1], 2) +
                             pow(bead_model[i].bead_coordinate.axis[2] -
                                 bead_model[j].bead_coordinate.axis[2], 2));

#if defined(DEBUG)
                     printf("beads %d %d with radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] have a sep of %f\n",
                            i, j,
                            bead_model[i].bead_computed_radius,
                            bead_model[j].bead_computed_radius,
                            bead_model[i].bead_coordinate.axis[0],
                            bead_model[i].bead_coordinate.axis[1],
                            bead_model[i].bead_coordinate.axis[2],
                            bead_model[j].bead_coordinate.axis[0],
                            bead_model[j].bead_coordinate.axis[1],
                            bead_model[j].bead_coordinate.axis[2],
                            separation);
#endif
                     if (separation <= TOLERANCE) {
                        continue;
                     }

                     pair.i = i;
                     pair.j = j;
                     pair.separation = separation;
                     pair.active = true;
                     pairs.push_back(pair);
                     count++;
                  }
               }
            }
            // ok, now we have the list of pairs
            max_bead1 =
               max_bead2 = -1;
            float radius_delta;
            do {
#if defined(DEBUG1) || defined(DEBUG)
               printf("processing hierarchical radial reduction iteration %d\n", iter++);
#endif
               max_intersection_length = 0;
               int max_pair = -1;
               count = 0;
               for (unsigned int i = 0; i < pairs.size(); i++) {
#if defined(DEBUG_OVERLAP)
                  printf("pair %d %d sep %f %s %s\n",
                         pairs[i].i, pairs[i].j, pairs[i].separation, pairs[i].active ? "active" : "not active",
                         pairs[i].i == max_bead1 ||
                         pairs[i].j == max_bead1 ||
                         pairs[i].i == max_bead2 ||
                         pairs[i].j == max_bead2 ? "needs recompute of separation" : "separation valid");
#endif
                  if (pairs[i].active) {
                     if (
                         pairs[i].i == max_bead1 ||
                         pairs[i].j == max_bead1 ||
                         pairs[i].i == max_bead2 ||
                         pairs[i].j == max_bead2
                         ) {
                        pairs[i].separation =
                           bead_model[pairs[i].i].bead_computed_radius +
                           bead_model[pairs[i].j].bead_computed_radius -
                           sqrt(
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[0] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[0], 2) +
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[1] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[1], 2) +
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[2] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[2], 2));
                        pairs[i].active = true;
                     }
                     if (pairs[i].separation > max_intersection_length) {
                        max_intersection_length = pairs[i].separation;
                        max_pair = i;
                     }
                  }
               }

               if (max_intersection_length > TOLERANCE) {
                  count++;
                  pairs[max_pair].active = false;
                  max_bead1 = pairs[max_pair].i;
                  max_bead2 = pairs[max_pair].j;
#if defined(DEBUG1) || defined(DEBUG)
                  printf("processing radial reduction hierc iteration %d pair %d processed %d\n", iter, max_pair, count);
                  printf("reducing beads %d %d\n", max_bead1, max_bead2);
#endif
                  do {
                     if (methods[k] & OUTWARD_TRANSLATION ||
                         ((bead_model[max_bead1].chain == 1 ||
                           bead_model[max_bead2].chain == 1) &&
                          methods[0] & OUTWARD_TRANSLATION)) {
                        // new 1 step ot
                        if((methods[k] & RR_MCSC &&
                            (bead_model[max_bead1].chain == 1 ||
                             bead_model[max_bead2].chain == 1))
                           ) {
                           // new 1 bead 1 OT / treat as no ot...
                           int use_bead;
                           if (bead_model[max_bead1].chain == 1) {
                              use_bead = max_bead2;
                           } 
                           else 
                           {
                              use_bead = max_bead1;
                           }
                           // one bead to shrink
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta = max_intersection_length;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required, one bead to shrink, radius delta %f  cr %f %f\n",
                                  use_bead,
                                  radius_delta,
                                  bead_model[use_bead].bead_computed_radius,
                                  bead_model[use_bead].bead_computed_radius - radius_delta
                                  );
#endif
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                        else 
                        {
                           int use_bead = max_bead1;
                           if (!bead_model[use_bead].normalized_ot_is_valid) {
                              float norm = 0.0;
                              for (unsigned int l = 0; l < 3; l++) {
                                 bead_model[use_bead].normalized_ot.axis[l] =
                                    bead_model[use_bead].bead_coordinate.axis[l] -
                                    molecular_cog[l];
                                 norm +=
                                    bead_model[use_bead].normalized_ot.axis[l] *
                                    bead_model[use_bead].normalized_ot.axis[l];
                              }
                              norm = sqrt(norm);
                              if (norm) {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                 }
                                 bead_model[use_bead].normalized_ot_is_valid = true;
                              } 
                              else 
                              {
                                 printf("wow! bead %d is at the molecular cog!\n", use_bead);
                              }
                           }
                           use_bead = max_bead2;
                           if (!bead_model[use_bead].normalized_ot_is_valid) {
                              float norm = 0.0;
                              for (unsigned int l = 0; l < 3; l++) {
                                 bead_model[use_bead].normalized_ot.axis[l] =
                                    bead_model[use_bead].bead_coordinate.axis[l] -
                                    molecular_cog[l];
                                 norm +=
                                    bead_model[use_bead].normalized_ot.axis[l] *
                                    bead_model[use_bead].normalized_ot.axis[l];
                              }
                              norm = sqrt(norm);
                              if (norm) {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                 }
                                 bead_model[use_bead].normalized_ot_is_valid = true;
                              }
                              else 
                              {
                                 printf("wow! bead %d is at the molecular cog!\n", use_bead);
                              }
                           }
                           // we need to handle 1 fixed case and
                           // the slight potential of one being at the molecular cog
                           reduced_any[max_bead1] = true;
                           reduced_any[max_bead2] = true;
                           outward_translate_2_spheres(
                                                       &bead_model[max_bead1].bead_computed_radius,
                                                       &bead_model[max_bead2].bead_computed_radius,
                                                       bead_model[max_bead1].bead_coordinate.axis,
                                                       bead_model[max_bead2].bead_coordinate.axis,
                                                       bead_model[max_bead1].normalized_ot.axis,
                                                       bead_model[max_bead2].normalized_ot.axis
                                                       );
                        }
                     }
                     else 
                     {
                        // no outward translation is required for either bead
                        // are we shrinking just 1 bead ... if we are dealing with buried beads, then
                        // only buried beads should be shrunk, not exposed beads
#if defined(DEBUG2)
                        printf("no outward translation is required\n");
#endif
                        if(methods[k] & RR_BURIED &&
                           bead_model[max_bead1].exposed_code == 1 &&
                           bead_model[max_bead2].exposed_code == 1) {
                           printf("what are we doing here?  buried and two exposed??\n");
                           exit(-1);
                        }
                        if(methods[k] & RR_MCSC &&
                           !(methods[k] & RR_BURIED) &&
                           bead_model[max_bead1].chain == 1 &&
                           bead_model[max_bead2].chain == 1) {
                           printf("what are we doing here?  dealing with 2 SC's on the MCSC run??\n");
                           exit(-1);
                        }
                        if((methods[k] & RR_BURIED &&
                            (bead_model[max_bead1].exposed_code == 1 ||
                             bead_model[max_bead2].exposed_code == 1)) ||
                           (methods[k] & RR_MCSC &&
                            !(methods[k] & RR_BURIED) &&
                            (bead_model[max_bead1].chain == 1 ||
                             bead_model[max_bead2].chain == 1))) {
                           // only one bead to shrink, since
                           // we are either buried with one of the beads exposed or
                           // on the MCSC and one of the beads is SC
                           int use_bead;
                           if (methods[k] & RR_BURIED) {
                              if (bead_model[max_bead1].exposed_code == 1) {
                                 use_bead = max_bead2;
                              }
                              else 
                              {
                                 use_bead = max_bead1;
                              }
                           }
                           else 
                           {
                              if (bead_model[max_bead1].chain == 1) {
                                 use_bead = max_bead2;
                              }
                              else 
                              {
                                 use_bead = max_bead1;
                              }
                           }
                           // one bead to shrink
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta = max_intersection_length;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required, one bead to shrink, radius delta %f  cr %f %f\n",
                                  use_bead,
                                  radius_delta,
                                  bead_model[use_bead].bead_computed_radius,
                                  bead_model[use_bead].bead_computed_radius - radius_delta
                                  );
#endif
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                        else 
                        {
                           // two beads to shrink
                           int use_bead = max_bead1;
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta =
                              // bead_model[use_bead].bead_actual_radius * rr_overlap[k];
                              max_intersection_length * bead_model[max_bead1].bead_computed_radius /
                              (bead_model[max_bead1].bead_computed_radius + bead_model[max_bead2].bead_computed_radius);
                           float radius_delta2 =
                              // bead_model[use_bead].bead_actual_radius * rr_overlap[k];
                              max_intersection_length * bead_model[max_bead2].bead_computed_radius /
                              (bead_model[max_bead1].bead_computed_radius + bead_model[max_bead2].bead_computed_radius);
#if defined(DEBUG2)
                           printf("intersection len %f recomputed %f radius delta %f r1 %f r2 %f\n",
                                  max_intersection_length,
                                  bead_model[max_bead1].bead_computed_radius +
                                  bead_model[max_bead2].bead_computed_radius -
                                  sqrt(
                                       pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                           bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                           bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                           bead_model[max_bead2].bead_coordinate.axis[2], 2)),
                                  radius_delta,
                                  bead_model[max_bead1].bead_computed_radius,
                                  bead_model[max_bead2].bead_computed_radius
                                  );
#endif
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE / 2);
                              reduced[use_bead] = false;
                           }

#if defined(DEBUG2)
                           printf("intersection len %f recomputed %f radius delta %f r1 %f r2 %f\n",
                                  max_intersection_length,
                                  bead_model[max_bead1].bead_computed_radius +
                                  bead_model[max_bead2].bead_computed_radius -
                                  sqrt(
                                       pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                           bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                           bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                           bead_model[max_bead2].bead_coordinate.axis[2], 2)),
                                  radius_delta,
                                  bead_model[max_bead1].bead_computed_radius,
                                  bead_model[max_bead2].bead_computed_radius
                                  );
#endif
                           use_bead = max_bead2;
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = radius_delta2;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                     }
#if defined(DEBUG2)
                     printf("b1r %f b2r %f current separation %f\n",
                            bead_model[max_bead1].bead_computed_radius,
                            bead_model[max_bead2].bead_computed_radius,
                            bead_model[max_bead1].bead_computed_radius +
                            bead_model[max_bead2].bead_computed_radius -
                            sqrt(
                                 pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                     bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                     bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                     bead_model[max_bead2].bead_coordinate.axis[2], 2)));
                     printf("flags %s %s %s %s\n",
                            bead_model[max_bead1].bead_computed_radius > TOLERANCE ? "Y" : "N",
                            bead_model[max_bead2].bead_computed_radius > TOLERANCE ? "Y" : "N",
                            (bead_model[max_bead1].bead_computed_radius +
                             bead_model[max_bead2].bead_computed_radius -
                             sqrt(
                                  pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                      bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                      bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                      bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE) ? "Y" : "N",
                            bead_model[max_bead1].bead_computed_radius > TOLERANCE &&
                            bead_model[max_bead2].bead_computed_radius > TOLERANCE &&
                            (bead_model[max_bead1].bead_computed_radius +
                             bead_model[max_bead2].bead_computed_radius -
                             sqrt(
                                  pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                      bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                      bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                      bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE) ? "Y" : "N");
#endif
                  } while (
                           bead_model[max_bead1].bead_computed_radius > TOLERANCE &&
                           bead_model[max_bead2].bead_computed_radius > TOLERANCE &&
                           (bead_model[max_bead1].bead_computed_radius +
                            bead_model[max_bead2].bead_computed_radius -
                            sqrt(
                                 pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                     bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                     bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                     bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE));
#if defined(DEBUG2)
                  printf("out of while 1\n");
#endif
               } // if max intersection length > TOLERANCE
               qApp->processEvents();
               if (stopFlag)
               {
                  return;
               }
            } while(count);
#if defined(DEBUG2)
            printf("out of while 2\n");
#endif
         }
         else 
         {
            // simultaneous reduction

            do {
#if defined(DEBUG_OVERLAP)
               overlap_check(methods[k] & RR_SC ? true : false,
                             methods[k] & RR_MCSC ? true : false,
                             methods[k] & RR_BURIED ? true : false);
#endif
               // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_br-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
               // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-br-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
               iter++;
#if defined(DEBUG1) || defined(DEBUG)
               printf("processing simultaneous radial reduction iteration %d\n", iter);
               editor->append(QString(" %1").arg(iter));
#endif
               if(iter > 10000) {
                  printf("too many iterations\n");
                  exit(-1);
               }
               max_intersection_length = 0;
               pairs.clear();
               count = 0;
               reduced[bead_model.size() - 1] = false;
               for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
                  reduced[i] = false;
                  if (1 || last_reduced[i]) {
                     for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUGX)
                        printf("checking radial stage %d beads %d %d on chains %d %d exposed code %d %d active %s %s max il %f\n",
                               k, i, j,
                               bead_model[i].chain,
                               bead_model[j].chain,
                               bead_model[i].exposed_code,
                               bead_model[j].exposed_code,
                               bead_model[i].active ? "Y" : "N",
                               bead_model[j].active ? "Y" : "N",
                               max_intersection_length
                               );
#endif
                        if ((1 || last_reduced[j]) &&
                            bead_model[i].active &&
                            bead_model[j].active &&
                            (methods[k] & RR_MCSC ||
                             ((methods[k] & RR_SC) &&
                              bead_model[i].chain == 1 &&
                              bead_model[j].chain == 1)) &&
                            ((methods[k] & RR_BURIED) ||
                             (bead_model[i].exposed_code == 1 ||
                              bead_model[j].exposed_code == 1)) &&
                            bead_model[i].bead_computed_radius > TOLERANCE &&
                            bead_model[j].bead_computed_radius > TOLERANCE
                            ) {
                           float separation =
                              bead_model[i].bead_computed_radius +
                              bead_model[j].bead_computed_radius -
                              sqrt(
                                   pow(bead_model[i].bead_coordinate.axis[0] -
                                       bead_model[j].bead_coordinate.axis[0], 2) +
                                   pow(bead_model[i].bead_coordinate.axis[1] -
                                       bead_model[j].bead_coordinate.axis[1], 2) +
                                   pow(bead_model[i].bead_coordinate.axis[2] -
                                       bead_model[j].bead_coordinate.axis[2], 2));


                           if (separation <= TOLERANCE) {
                              continue;
                           }

#if defined(DEBUG)
                           printf("beads %d %d with radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] have a sep of %f\n",
                                  i, j,
                                  bead_model[i].bead_computed_radius,
                                  bead_model[j].bead_computed_radius,
                                  bead_model[i].bead_coordinate.axis[0],
                                  bead_model[i].bead_coordinate.axis[1],
                                  bead_model[i].bead_coordinate.axis[2],
                                  bead_model[j].bead_coordinate.axis[0],
                                  bead_model[j].bead_coordinate.axis[1],
                                  bead_model[j].bead_coordinate.axis[2],
                                  separation);
#endif

                           if (separation > max_intersection_length) {
                              max_intersection_length = separation;
                              max_bead1 = i;
                              max_bead2 = j;
                           }
                           pair.i = i;
                           pair.j = j;
                           pair.separation = separation;
                           pairs.push_back(pair);
                           count++;
                        }
                     }
                  } // if last_reduced[i]
               }
#if defined(DEBUG1) || defined(DEBUG)
               printf("processing radial reduction sync iteration %d pairs to process %d max int len %f\n", iter, count, max_intersection_length);
#endif
               if (max_intersection_length > TOLERANCE) {
#if defined(DEBUG1) || defined(DEBUG)
                  printf("processing radial reduction sync iteration %d pairs to process %d\n", iter, count);
#endif
                  for (unsigned int i = 0; i < pairs.size(); i++) {
                     if (
                         !reduced[pairs[i].i] &&
                         (k == 3 ||
                          (bead_model[pairs[i].i].exposed_code != 1 ||
                           methods[k] & RR_EXPOSED ||
                           methods[k] & RR_ALL) &&
                          (!(methods[k] & RR_MCSC) ||
                           bead_model[pairs[i].i].chain == 0 ||
                           (methods[k] & RR_BURIED &&
                            bead_model[pairs[i].i].exposed_code != 1)))
                         ) {
                        int use_bead = pairs[i].i;
                        /*      if ( !(methods[k] & RR_MCSC) ||
                                bead_model[use_bead].exposed_code != 1 ||
                                bead_model[use_bead].chain == 0 ||
                                (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1) ) */
                        if(1) {
#if defined(DEBUG)
                           printf("reducing beads %d\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = bead_model[use_bead].bead_computed_radius * rr_overlap[k];
                           if (radius_delta < TOLERANCE) {
                              radius_delta = (float)TOLERANCE;
                           }
                           if (methods[k] & OUTWARD_TRANSLATION ||
                               (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1 &&
                                methods[0] & OUTWARD_TRANSLATION) ) {
#if defined(DEBUG)
                              printf("outward translation from [%f,%f,%f] to ",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                              if (!bead_model[use_bead].normalized_ot_is_valid) {
                                 float norm = 0.0;
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] =
                                       bead_model[use_bead].bead_coordinate.axis[l] -
                                       molecular_cog[l];
                                    norm +=
                                       bead_model[use_bead].normalized_ot.axis[l] *
                                       bead_model[use_bead].normalized_ot.axis[l];
                                 }
                                 norm = sqrt(norm);
                                 if (norm) {
                                    for (unsigned int l = 0; l < 3; l++) {
                                       bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                       bead_model[use_bead].bead_coordinate.axis[l] +=
                                          radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                    }
                                    bead_model[use_bead].normalized_ot_is_valid = true;
                                 }
                                 else 
                                 {
                                    printf("wow! bead %d is at the molecular cog!\n", use_bead);
                                 }
                              }
                              else 
                              {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].bead_coordinate.axis[l] +=
                                       radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                 }
                              }
#if defined(DEBUG)
                              printf(" [%f,%f,%f]\n",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)TOLERANCE * 0.9999;
                              reduced[use_bead] = false;
                           }
                        }
                     }
                     if (
                         !reduced[pairs[i].j] &&
                         (bead_model[pairs[i].j].exposed_code != 1 ||
                          methods[k] & RR_EXPOSED ||
                          methods[k] & RR_ALL) &&
                         (!(methods[k] & RR_MCSC) ||
                          bead_model[pairs[i].j].chain == 0 ||
                          (methods[k] & RR_BURIED &&
                           bead_model[pairs[i].j].exposed_code != 1))
                         ) {
                        int use_bead = pairs[i].j;
                        /* if ( !(methods[k] & RR_MCSC) ||
                           bead_model[use_bead].chain == 0 ||
                           bead_model[use_bead].exposed_code != 1 ||
                           (bead_model[pairs[i].i].chain == 1 &&
                           bead_model[pairs[i].j].chain == 1) ) */
                        {
#if defined(DEBUG)
                           printf("reducing beads %d\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = bead_model[use_bead].bead_computed_radius * rr_overlap[k];
                           if (radius_delta < TOLERANCE) {
                              radius_delta = (float)TOLERANCE;
                           }
                           if (methods[k] & OUTWARD_TRANSLATION ||
                               (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1 &&
                                methods[0] & OUTWARD_TRANSLATION) ) {
#if defined(DEBUG)
                              printf("outward translation from [%f,%f,%f] to ",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                              if (!bead_model[use_bead].normalized_ot_is_valid) {
                                 float norm = 0.0;
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] =
                                       bead_model[use_bead].bead_coordinate.axis[l] -
                                       molecular_cog[l];
                                    norm +=
                                       bead_model[use_bead].normalized_ot.axis[l] *
                                       bead_model[use_bead].normalized_ot.axis[l];
                                 }
                                 norm = sqrt(norm);
                                 if (norm) {
                                    for (unsigned int l = 0; l < 3; l++) {
                                       bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                       bead_model[use_bead].bead_coordinate.axis[l] +=
                                          radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                    }
                                    bead_model[use_bead].normalized_ot_is_valid = true;
                                 }
                                 else 
                                 {
                                    printf("wow! bead %d is at the molecular cog!\n", use_bead);
                                 }
                              }
                              else 
                              {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].bead_coordinate.axis[l] +=
                                       radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                 }
                              }
#if defined(DEBUG)
                              printf(" [%f,%f,%f]\n",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)TOLERANCE * 0.9999;
                              reduced[use_bead] = false;
                           }
                        }
                     }
                  }
               }
               last_reduced = reduced;
               // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_ar-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
               // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-ar-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
               qApp->processEvents();
               if (stopFlag)
               {
                  return;
               }
            } while(count);
         }
#if defined(TIMING)
         gettimeofday(&end_tv, NULL);
         printf("radial reduction %d time %lu\n",
                k,
                1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
                start_tv.tv_usec);
         fflush(stdout);
#endif

         // recompute volumes
         for (unsigned int i = 0; i < bead_model.size(); i++) {
            if (reduced_any[i]) {
#if defined(DEBUG1) || defined(DEBUG)
               printf("recomputing volume bead %d\n", i);
#endif
               bead_model[i].bead_ref_volume =
                  (4.0*M_PI/3.0) * pow(bead_model[i].bead_computed_radius, 3);
            }
         }
      }
#if defined(DEBUG_OVERLAP)
      if(overlap_check(methods[k] & RR_SC ? true : false,
                       methods[k] & RR_MCSC ? true : false,
                       methods[k] & RR_BURIED ? true : false)) {
         printf("internal over lap error... exiting!");
         exit(-1);
      }
#endif
#if defined(WRITE_EXTRA_FILES)
      write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
      write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO, &bead_model);
#endif
   } // methods
#if defined(WRITE_EXTRA_FILES)
   write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO + ".tsv", &bead_model);
   write_bead_spt(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO, &bead_model);
#endif
   //  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
   //       QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
   //       DOTSOMO, &bead_model);
   write_bead_model(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
                    QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") + DOTSOMO
                    , &bead_model);
   editor->append("Finished with popping and radial reduction\n");
}

int US_Hydrodyn::compute_asa()
{
   QString error_string = "";
   progress->reset();
   editor->append(QString("\nBuilding the bead model for %1 model %2\n").arg(project).arg(current_model+1));
   editor->append("Checking the pdb structure\n");
   if (check_for_missing_atoms(&error_string, &model_vector[current_model])) {
      editor->append("Encountered the following errors with your PDB structure:\n" +
                     error_string);
      printError("Encountered errors with your PDB structure:\n"
                 "please check the text window");
      return -1;
   }
   editor->append("PDB structure ok\n");
   int mppos = 18 + (asa.recheck_beads ? 1 : 0);
   progress->setTotalSteps(mppos);
   int ppos = 1;
   progress->setProgress(ppos++); // 1
   qApp->processEvents();

   {
      int no_of_atoms = 0;
      int no_of_molecules = model_vector[current_model].molecule.size();
      int i;
      for (i = 0; i < no_of_molecules; i++) {
         no_of_atoms +=  model_vector[current_model].molecule[i].atom.size();
      }

      editor->append(QString("There are %1 atoms in %2 chain(s) in this model\n").arg(no_of_atoms).arg(no_of_molecules));
   }
   if (stopFlag)
   {
      return -1;
   }
   int retval = create_beads(&error_string);
   if ( retval )
   {
      editor->append("Errors found during the initial creation of beads\n");
      progress->setProgress(mppos);
      qApp->processEvents();
      if (stopFlag)
      {
         return -1;
      }
      switch ( retval )
      {
      case US_SURFRACER_ERR_MISSING_RESIDUE:
         {
            printError("Encountered an unknown residue:\n" +
                       error_string);
            return US_SURFRACER_ERR_MISSING_RESIDUE;
            break;
         }
      case US_SURFRACER_ERR_MISSING_ATOM:
         {
            printError("Encountered a unknown atom:\n" +
                       error_string);
            return US_SURFRACER_ERR_MISSING_ATOM;
            break;
         }
      case US_SURFRACER_ERR_MEMORY_ALLOC:
         {
            printError("Encountered a memory allocation error");
            return US_SURFRACER_ERR_MEMORY_ALLOC;
            break;
         }
      default:
         {
            printError("Encountered an unknown error");
            // unknown error
            return -1;
            break;
         }
      }
   }

   if(error_string.length()) {
      progress->setProgress(mppos);
      qApp->processEvents();
      printError("Encountered unknown atom(s) error:\n" +
                 error_string);
      return US_SURFRACER_ERR_MISSING_ATOM;
   }


   if(asa.method == 0) {
      // surfracer
      editor->append("Computing ASA via SurfRacer\n");
      qApp->processEvents();
      if (stopFlag)
      {
         return -1;
      }
      int retval = surfracer_main(asa.probe_radius,
                                  active_atoms,
                                  false,
                                  progress,
                                  editor
                                  );

      progress->setProgress(ppos++); // 2
      qApp->processEvents();
      editor->append("Return from Computing ASA\n");
      if (stopFlag)
      {
         return -1;
      }
      if ( retval )
      {
         editor->append("Errors found during ASA calculation\n");
         progress->setProgress(mppos);
         qApp->processEvents();
         if (stopFlag)
         {
            return -1;
         }
         switch ( retval )
         {
         case US_SURFRACER_ERR_MISSING_RESIDUE:
            {
               printError("US_SURFRACER encountered an unknown residue:\n" +
                          error_string);
               return US_SURFRACER_ERR_MISSING_RESIDUE;
               break;
            }
         case US_SURFRACER_ERR_MISSING_ATOM:
            {
               printError("US_SURFRACER encountered a unknown atom:\n" +
                          error_string);
               return US_SURFRACER_ERR_MISSING_ATOM;
               break;
            }
         case US_SURFRACER_ERR_MEMORY_ALLOC:
            {
               printError("US_SURFRACER encountered a memory allocation error");
               return US_SURFRACER_ERR_MEMORY_ALLOC;
               break;
            }
         default:
            {
               printError("US_SURFRACER encountered an unknown error");
               // unknown error
               return -1;
               break;
            }
         }
      }
      if(error_string.length()) {
         progress->setProgress(mppos);
         qApp->processEvents();
         printError("US_SURFRACER encountered unknown atom(s) error:\n" +
                    error_string);
         return US_SURFRACER_ERR_MISSING_ATOM;
      }
   }

   if(asa.method == 1) {
      // surfracer
      editor->append("Computing ASA via ASAB1\n");
      qApp->processEvents();
      if (stopFlag)
      {
         return -1;
      }
      int retval = us_hydrodyn_asab1_main(active_atoms,
                                          &asa,
                                          &results,
                                          false,
                                          progress,
                                          editor,
                                          this
                                          );

      progress->setProgress(ppos++); // 2
      qApp->processEvents();
      editor->append("Return from Computing ASA\n");
      if (stopFlag)
      {
         return -1;
      }
      if ( retval )
      {
         editor->append("Errors found during ASA calculation\n");
         progress->setProgress(mppos);
         qApp->processEvents();
         if (stopFlag)
         {
            return -1;
         }
         switch ( retval )
         {
         case US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC:
            {
               printError("US_HYDRODYN_ASAB1 encountered a memory allocation error");
               return US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC;
               break;
            }
         default:
            {
               printError("US_HYDRODYN_ASAB1 encountered an unknown error");
               // unknown error
               return -1;
               break;
            }
         }
      }
   }

   // pass 1 assign bead #'s, chain #'s, initialize data
   printf("made it to here\n"); fflush(stdout);

   FILE *asaf = fopen(QString(somo_tmp_dir + "/atom.asa"), "w");


   // for (unsigned int i = 0; i < model_vector.size (); i++)
   {
      unsigned int i = current_model;

      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p1 i j k %d %d %d %lx %s\n", i, j, k, (long unsigned int)this_atom->p_atom, this_atom->active ? "active" : "not active"); fflush(stdout);

            //   this_atom->bead_assignment =
            //     (this_atom->p_atom ? (int) this_atom->p_atom->bead_assignment : -1);
            //   printf("this_atom->bead_assignment %d\n", this_atom->bead_assignment); fflush(stdout);
            //   this_atom->chain =
            //     ((this_atom->p_residue && this_atom->p_atom) ?
            //      (int) this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].chain : -1);

            // initialize data
            // this_atom->bead_positioner = false;
            this_atom->normalized_ot_is_valid = false;
            fprintf(asaf, "%s\t%s\t%s\t%.2f\n",
                    this_atom->name.ascii(),
                    this_atom->resName.ascii(),
                    this_atom->resSeq.ascii(),
                    this_atom->asa);


            for (unsigned int m = 0; m < 3; m++) {
               this_atom->bead_cog_coordinate.axis[m] = 0;
               this_atom->bead_position_coordinate.axis[m] = 0;
               this_atom->bead_coordinate.axis[m] = 0;
            }
         }
      }
   }
   fclose(asaf);

   progress->setProgress(ppos++); // 3
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }

   // #define DEBUG
   // pass 2 determine beads, cog_position, fixed_position, molecular cog phase 1.

   int count_actives;
   float molecular_cog[3] = { 0, 0, 0 };
   float molecular_mw = 0;

   // for (unsigned int i = 0; i < model_vector.size (); i++)
   {
      unsigned int i = current_model;

      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         int last_bead_assignment = -1;
         int last_chain = -1;
         QString last_resName = "not a residue";
         QString last_resSeq = "";
         PDB_atom *last_main_chain_bead = (PDB_atom *) 0;
         PDB_atom *last_main_bead = (PDB_atom *) 0;
         PDB_atom *sidechain_N = (PDB_atom *) 0;

         count_actives = 0;
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p2 i j k %d %d %d %lx\n", i, j, k, this_atom->p_atom); fflush(stdout);
            // this_atom->bead_positioner = false;
            if (this_atom->active) {
#if defined(DEBUG)
               printf("pass 2 active %s %s %d pm %d %d\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->serial,
                      this_atom->placing_method,
                      this_atom->bead_assignment); fflush(stdout);
#endif

               molecular_mw += this_atom->mw;
               for (unsigned int m = 0; m < 3; m++) {
                  molecular_cog[m] += this_atom->coordinate.axis[m] * this_atom->mw;
               }

               this_atom->bead_mw = 0;
               this_atom->bead_asa = 0;
               this_atom->bead_recheck_asa = 0;

               // do we have a new bead?
               // we want to put the N on a previous bead unless it is the first one of the molecule
               // ONLY FOR residue type = 0! (amino acid)
               // AND ONLY for residues not part of the 'auto bead builder'

               if (!create_beads_normally ||
                   (
                    (this_atom->bead_assignment != last_bead_assignment ||
                     this_atom->chain != last_chain ||
                     this_atom->resName != last_resName ||
                     this_atom->resSeq != last_resSeq) &&
                    !(regular_N_handling &&
                      this_atom->chain == 0 &&
                      this_atom->name == "N" &&
                      count_actives))) {
#if defined(DEBUG) || defined(AUTO_BB_DEBUG)
                  printf("pass 2 active %s %s %d new bead chain %d\n",
                         this_atom->name.ascii(),
                         this_atom->resName.ascii(),
                         this_atom->serial,
                         this_atom->chain); fflush(stdout);
#endif

                  // this_atom->bead_positioner = true;
                  this_atom->is_bead = true;
                  last_main_bead = this_atom;
                  last_bead_assignment = this_atom->bead_assignment;
                  last_chain = this_atom->chain;
                  last_resName = this_atom->resName;
                  last_resSeq = this_atom->resSeq;
                  if (create_beads_normally) {
                     if(sidechain_N &&
                        this_atom->chain == 1) {
                        if(this_atom->name == "N") {
                           printf("ERROR double N on sequential sidechains! PRO PRO?\n");
                        }
#if defined(DEBUG) || defined(AUTO_BB_DEBUG)
                        printf("adding sidechain N %f %f to this_atom\n",
                               sidechain_N->asa,
                               sidechain_N->mw
                               );
#endif
                        this_atom->bead_asa += sidechain_N->bead_asa;
                        this_atom->bead_mw += sidechain_N->bead_mw;
                        sidechain_N->bead_mw = 0;
                        sidechain_N->bead_asa = 0;
                        sidechain_N = (PDB_atom *) 0;
                     }
                     if(regular_N_handling &&
                        this_atom->name == "N" &&
                        this_atom->chain == 1) {
                        sidechain_N = this_atom;
                        this_atom->is_bead = false;
                     }
                  }
               }
               else 
               {
#if defined(DEBUG)
                  printf("pass 2 active %s %s %d not a new bead\n",
                         this_atom->name.ascii(),
                         this_atom->resName.ascii(),
                         this_atom->serial); fflush(stdout);
#endif
                  if (this_atom->bead_positioner) {

                     if (last_main_bead->bead_positioner &&
                         this_atom->placing_method == 1) {
                        fprintf(stderr, "warning: 2 positioners in bead %s %s %d\n",
                                last_main_bead->name.ascii(),
                                last_main_bead->resName.ascii(),
                                last_main_bead->serial);
                     }
                     last_main_bead->bead_positioner = true;
                     last_main_bead->bead_position_coordinate = this_atom->coordinate;
                  }

                  this_atom->is_bead = false;
                  // this_atom->bead_cog_mw = 0;
               }

               this_atom->bead_cog_mw = 0;

               // special nitrogen asa handling
               PDB_atom *use_atom;
               if (create_beads_normally &&
                   this_atom->chain == 0 &&
                   regular_N_handling &&
                   this_atom->name == "N" &&
                   last_main_chain_bead) {
                  use_atom = last_main_chain_bead;
               }
               else 
               {
                  use_atom = last_main_bead;
               }


               use_atom->bead_asa += this_atom->asa;
               use_atom->bead_mw += this_atom->mw;
#if defined(DEBUG)
               printf("atom %s %s p_atom.hybrid.mw %f atom.mw %f\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->p_atom->hybrid.mw,
                      this_atom->mw
                      );
#endif
               // accum

               if (!create_beads_normally ||
                   this_atom->bead_positioner) {
#if defined(DEBUG_COG)
                  printf("adding cog from %d to %d mw %f totmw %f (this pos [%f,%f,%f], org pos [%f,%f,%f])\n", this_atom->serial, use_atom->serial, this_atom->mw, use_atom->bead_cog_mw,
                         this_atom->coordinate.axis[0],
                         this_atom->coordinate.axis[1],
                         this_atom->coordinate.axis[2],
                         use_atom->bead_cog_coordinate.axis[0],
                         use_atom->bead_cog_coordinate.axis[1],
                         use_atom->bead_cog_coordinate.axis[2]
                         );
#endif
                  use_atom->bead_cog_mw += this_atom->mw;
                  for (unsigned int m = 0; m < 3; m++) {
                     use_atom->bead_cog_coordinate.axis[m] +=
                        this_atom->coordinate.axis[m] * this_atom->mw;
                  }
#if defined(DEBUG_COG)
                  printf("afterwards: target mw %f pos [%f,%f,%f]\n",
                         use_atom->bead_cog_mw,
                         use_atom->bead_cog_coordinate.axis[0],
                         use_atom->bead_cog_coordinate.axis[1],
                         use_atom->bead_cog_coordinate.axis[2]);
#endif
               }
               else 
               {
#if defined(DEBUG_COG)
                  fprintf(stderr, "notice: atom %s %s %d excluded from cog calculation in bead %s %s %d\n",
                          this_atom->name.ascii(),
                          this_atom->resName.ascii(),
                          this_atom->serial,
                          use_atom->name.ascii(),
                          use_atom->resName.ascii(),
                          use_atom->serial);
#endif
               }

               if (!create_beads_normally ||
                   this_atom->bead_positioner) {
                  if (use_atom->bead_positioner &&
                      this_atom->placing_method == 1) {
                     fprintf(stderr, "warning: 2 or more positioners in bead %s %s %d\n",
                             use_atom->name.ascii(),
                             use_atom->resName.ascii(),
                             use_atom->serial);
                  }
                  use_atom->bead_positioner = true;
                  use_atom->bead_position_coordinate = this_atom->coordinate;
               }

               if (this_atom->chain == 0 &&
                   regular_N_handling &&
                   this_atom->name == "N" &&
                   !count_actives)
               {
                  last_resName = "first N";
               }

               if (this_atom->chain == 0 &&
                   this_atom->name == "CA") {

                  last_main_chain_bead = this_atom;
               }

               count_actives++;

            }
            else 
            {
               this_atom->is_bead = false;
            }
         }
      }
   }

   if (molecular_mw) {
      for (unsigned int m = 0; m < 3; m++) {
         molecular_cog[m] /= molecular_mw;
      }
#if defined(DEBUG1) || defined(DEBUG)
      printf("molecular cog [%f,%f,%f] mw %f\n",
             molecular_cog[0],
             molecular_cog[1],
             molecular_cog[2],
             molecular_mw);
#endif
   }
   else 
   {
      printf("ERROR: this molecule has zero mw!\n");
   }

   for (unsigned int m = 0; m < 3; m++) {
      last_molecular_cog.axis[m] = molecular_cog[m];
   }

   progress->setProgress(ppos++); // 4
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }

   // pass 2b move bead_ref_volume, ref_mw, computed_radius from
   // next main chain back one

   // for (unsigned int i = 0; i < model_vector.size (); i++)   {
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         PDB_atom *last_main_chain_bead = (PDB_atom *) 0;

         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

            if (this_atom->active &&
                this_atom->is_bead &&
                this_atom->chain == 0) {
               if (last_main_chain_bead &&
                   (this_atom->resName == "PRO" ||
                    last_main_chain_bead->resName == "PRO")
                   ) {
#if defined(DEBUG)
                  printf("pass 2b active PRO %s %s %d pm %d\n",
                         this_atom->name.ascii(),
                         this_atom->resName.ascii(),
                         this_atom->serial,
                         this_atom->placing_method); fflush(stdout);
#endif
                  last_main_chain_bead->bead_ref_volume = this_atom->bead_ref_volume;
                  last_main_chain_bead->bead_ref_mw = this_atom->bead_ref_mw;
                  last_main_chain_bead->bead_computed_radius = this_atom->bead_computed_radius;
                  if (this_atom->resName == "GLY") {
                     last_main_chain_bead->bead_ref_mw -= 1.01f;
                  }
               }
               if (this_atom->name == "OXT" &&
                   last_main_chain_bead) {
#if defined(DEBUG)
                  printf("pass 2b active OXT %s %s %d last %s %s %d\n",
                         this_atom->name.ascii(),
                         this_atom->resName.ascii(),
                         this_atom->serial,
                         last_main_chain_bead->name.ascii(),
                         last_main_chain_bead->resName.ascii(),
                         last_main_chain_bead->serial);
                  fflush(stdout);
#endif

                  this_atom->is_bead = false;
                  last_main_chain_bead->bead_ref_volume = this_atom->bead_ref_volume;
                  last_main_chain_bead->bead_ref_mw = this_atom->bead_ref_mw;
                  if (last_main_chain_bead->resName == "GLY") {
                     last_main_chain_bead->bead_ref_mw += 1.01f;
                  }
                  last_main_chain_bead->bead_computed_radius = this_atom->bead_computed_radius;
               }
               else 
               {
                  last_main_chain_bead = this_atom;
               }
            }
         }
      }
   }

   progress->setProgress(ppos++); // 5
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }

   // pass 2c hydration

   // for (unsigned int i = 0; i < model_vector.size (); i++)   {
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

            if (this_atom->active &&
                this_atom->is_bead) {
#if defined(DEBUG)
               printf("pass 2c hydration %s %s %d pm %d\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->serial,
                      this_atom->placing_method); fflush(stdout);
#endif
               this_atom->bead_ref_volume_unhydrated = this_atom->bead_ref_volume;
               this_atom->bead_ref_volume += misc.hydrovol * this_atom->bead_hydration;
               this_atom->bead_computed_radius = pow(3 * this_atom->bead_ref_volume / (4.0*M_PI), 1.0/3);
            }
         }
      }
   }

   // pass 3 determine visibility, exposed code, normalize cog position, final position determination
   // compute com of entire molecule
   progress->setProgress(ppos++); // 6
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }


#if defined(OLD_ASAB1_SC_COMPUTE)
   // pass 2d compute mc asa
   vector <float> bead_mc_asa;
# if defined(DEBUG1) || defined(DEBUG)
   printf("pass 2d\n"); fflush(stdout);
# endif
   // for (unsigned int i = 0; i < model_vector.size (); i++)   {
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

            if (this_atom->active &&
                this_atom->chain == 0) {
# if defined(DEBUG)
               printf("pass 2d mc_asa %s %s %d pm %d\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->serial,
                      this_atom->placing_method); fflush(stdout);
# endif
               if(bead_mc_asa.size() < this_atom->resSeq + 1) {
                  bead_mc_asa.resize(this_atom->resSeq + 32);
               }
#warn broken by resSeq->QString
               bead_mc_asa[this_atom->resSeq] += this_atom->asa;
            }
         }
      }
   }
#endif // OLD_ASABA_SC_COMPUTE

#if defined(DEBUG1) || defined(DEBUG)
   printf("pass 3\n"); fflush(stdout);
#endif

   // for (unsigned int i = 0; i < model_vector.size (); i++) {
   {
      unsigned int i = current_model;

      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {

#if defined(OLD_ASAB1_SC_COMPUTE)
         float mc_asab1 = 0;
         QString mc_resname = "";
         unsigned int mc_resSeq = 0;
#endif

         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {


            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p3 i j k %d %d %d %lx\n", i, j, k, this_atom->p_atom); fflush(stdout);
            this_atom->exposed_code = -1;
            if (this_atom->active &&
                this_atom->is_bead) {

#if defined(DEBUG)
               printf("pass 3 active is bead %s %s %d\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->serial); fflush(stdout);
#endif
               for (unsigned int m = 0; m < 3; m++) {
                  if (this_atom->bead_cog_mw) {
                     this_atom->bead_cog_coordinate.axis[m] /= this_atom->bead_cog_mw;
                  }
                  else 
                  {
                     this_atom->bead_cog_coordinate.axis[m] = 0;
                  }
               }

               if (this_atom->p_residue && this_atom->p_atom) {

#if defined(DEBUG)
                  printf("pass 3 active is bead %s %s %d bead assignment %d placing method %d\n",
                         this_atom->name.ascii(),
                         this_atom->resName.ascii(),
                         this_atom->serial,
                         this_atom->bead_assignment,
                         this_atom->placing_method
                         ); fflush(stdout);
#endif
                  switch (this_atom->placing_method) {

                  case 0 : // cog
                     this_atom->bead_coordinate = this_atom->bead_cog_coordinate;
                     // if (this_atom->bead_positioner) {
                     // fprintf(stderr, "warning: this bead had a atom claiming position & a bead placing method of cog! %s %s %d\n",
                     //   this_atom->name.ascii(),
                     //   this_atom->resName.ascii(),
                     //   this_atom->serial);
                     // }
                     break;
                  case 1 : // positioner
                     this_atom->bead_coordinate = this_atom->bead_position_coordinate;
                     break;
                  case 2 : // no positioning necessary
                     this_atom->bead_coordinate = this_atom->coordinate;
                     break;
                  default :
                     this_atom->bead_coordinate = this_atom->bead_cog_coordinate;
                     fprintf(stderr, "warning: unknown bead placing method %d %s %s %d <using cog!>\n",
                             this_atom->placing_method,
                             this_atom->name.ascii(),
                             this_atom->resName.ascii(),
                             this_atom->serial);
                     break;
                  }
               }
               else 
               {
                  fprintf(stderr, "serious internal error 1 on %s %s %d, quitting\n",
                          this_atom->name.ascii(),
                          this_atom->resName.ascii(),
                          this_atom->serial);
                  exit(-1);
                  break;
               }
#if defined(DEBUG) || defined(OLD_ASAB1_SC_COMPUTE)
               printf("pass 3 active is bead %s %s %s checkpoint 1\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->resSeq.ascii()); fflush(stdout);
#endif
               this_atom->visibility = (this_atom->bead_asa >= asa.threshold);
#if defined(OLD_ASAB1_SC_COMPUTE)
               if (this_atom->chain == 1) {
                  printf("visibility was %d is ", this_atom->visibility);
                  this_atom->visibility = (this_atom->bead_asa + bead_mc_asa[this_atom->resSeq] >= asa.threshold);
                  printf("%d\n", this_atom->visibility);
               }
#endif

#if defined(DEBUG)
               printf("pass 3 active is bead %s %s %d checkpoint 2\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->serial); fflush(stdout);
#endif
               if (!create_beads_normally ||
                   this_atom->visibility ||
                   !asa.calculation) {
                  this_atom->exposed_code = 1;  // exposed
               }
               else 
               {
                  if (this_atom->chain == 0) {
                     this_atom->exposed_code = 10;  // main chain, buried
                  }
                  if (this_atom->chain == 1) {
                     this_atom->exposed_code = 6;   // side chain, buried
                  }
               }
#if defined(DEBUG)
               printf("pass 3 active is bead %s %s %d checkpoint 3\n",
                      this_atom->name.ascii(),
                      this_atom->resName.ascii(),
                      this_atom->serial); fflush(stdout);
#endif

#if defined(OLD_ASAB1_SC_COMPUTE)
               if(this_atom->chain == 0) {
                  mc_asab1 = this_atom->bead_asa;
                  mc_resname = this_atom->resName;
                  mc_resSeq = this_atom->resSeq;
                  printf("saving last mc asa %.2f\n", mc_asab1);
               }
#endif
            }
            else 
            {
               this_atom->placing_method = -1;
            }
         }
      }
   }

   // pass 4 print results
   progress->setProgress(ppos++); // 7
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }

#if defined(DEBUG)
   printf("model~molecule~atom~name~residue~chainID~"
          "position~active~radius~asa~mw~"
          "bead #~chain~serial~is_bead~bead_asa~visible~code/color~"
          "bead mw~position controlled?~bead positioner~baric method~bead hydration~bead color~"
          "bead ref volume~bead ref mw~bead computed radius~mw match?~"
          "position_coordinate~cog position~use position\n");
   // for (unsigned int i = 0; i < model_vector.size (); i++)
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p4 i j k %d %d %d %lx\n", i, j, k, this_atom->p_atom); fflush(stdout);

            // printf("model %d mol %d atm %d nam %s res %s xyz [%f,%f,%f] act %s rads %f asa %f bead # %d chain %d serl %d is_bead %s bead_asa %f vis %s code %d pos? %s pos_co [%f,%f,%f] cog [%f,%f,%f] use [%f, %f, %f]\n",
            printf("%d~%d~%d~%s~%s~%s~"
                   "[%f,%f,%f]~%s~%f~%f~%f~"
                   "%d~%d~%d~%s~%f~%s~"
                   "%d~%f~%s~%s~%d~%u~%u~"
                   "%f~%f~%f~%s~"
                   "[%f,%f,%f]~[%f,%f,%f]~[%f, %f, %f]\n",

                   i, j, k,
                   this_atom->name.ascii(),
                   this_atom->resName.ascii(),
                   this_atom->chainID.ascii(),

                   this_atom->coordinate.axis[0],
                   this_atom->coordinate.axis[1],
                   this_atom->coordinate.axis[2],
                   this_atom->active ? "Y" : "N",
                   this_atom->radius,
                   this_atom->asa,
                   this_atom->mw,

                   this_atom->bead_assignment,
                   this_atom->chain,
                   this_atom->serial,
                   this_atom->is_bead ? "Y" : "N",
                   this_atom->bead_asa,
                   this_atom->visibility ? "Y" : "N",

                   this_atom->exposed_code,
                   this_atom->bead_mw,
                   this_atom->bead_positioner ? "Y" : "N",
                   this_atom->active ? (this_atom->bead_positioner ? "Y" : "N") : "Inactive",
                   this_atom->placing_method,
                   this_atom->bead_hydration,
                   this_atom->bead_color,

                   this_atom->bead_ref_volume,
                   this_atom->bead_ref_mw,
                   this_atom->bead_computed_radius,
                   fabs(this_atom->bead_ref_mw - this_atom->bead_mw) < .03 ? "Y" : "N",

                   this_atom->bead_position_coordinate.axis[0],
                   this_atom->bead_position_coordinate.axis[1],
                   this_atom->bead_position_coordinate.axis[2],
                   this_atom->bead_cog_coordinate.axis[0],
                   this_atom->bead_cog_coordinate.axis[1],
                   this_atom->bead_cog_coordinate.axis[2],
                   this_atom->bead_coordinate.axis[0],
                   this_atom->bead_coordinate.axis[1],
                   this_atom->bead_coordinate.axis[2]
                   ); fflush(stdout);
         }
      }
   }
#endif
   // build vector of beads
   bead_model.clear();
   // #define DEBUG_MOD
#if defined(DEBUG_MOD)
   vector<PDB_atom> dbg_model;
#endif
   //  for (unsigned int i = 0; i < model_vector.size (); i++)
#if defined(DEBUG_MW)
   double tot_atom_mw = 0e0;
   double tot_bead_mw = 0e0;
   double prev_bead_mw = 0e0;
   double prev_atom_mw = 0e0;
   int bead_count = 0;
   int atom_count = 0;
   int last_asgn = -1;
   QString last_res = "";
   QString last_resName = "";
#endif
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
#if defined(DEBUG_MOD)
            this_atom->bead_number = 0;
            dbg_model.push_back(*this_atom);
#endif
            if(this_atom->active &&
               this_atom->is_bead) {
               this_atom->bead_number = bead_model.size();
               this_atom->bead_actual_radius = this_atom->bead_computed_radius;
#if defined(DEBUG_MW)
               if(bead_count) {
                  printf("res %s %s bead %d bead_mw %f sum atom mw %f diff %f\n",
                         last_res.ascii(),
                         last_resName.ascii(),
                         last_asgn,
                         prev_bead_mw,
                         prev_atom_mw,
                         prev_bead_mw -
                         prev_atom_mw
                         );
               }
               prev_bead_mw = 0;
               prev_atom_mw = 0;
               bead_count++;
               tot_bead_mw += this_atom->bead_ref_mw;
               prev_bead_mw += this_atom->bead_ref_mw;
               last_asgn = (int)this_atom->bead_assignment;
               last_res = this_atom->resSeq;
               last_resName = this_atom->resName;
#endif
               bead_model.push_back(*this_atom);
            }
#if defined(DEBUG_MW)
            if(this_atom->active) {
               atom_count++;
               tot_atom_mw += this_atom->mw;
               prev_atom_mw += this_atom->mw;
            }
#endif
         }
      }
   }
#if defined(DEBUG_MW)
   printf("res %d %s bead %d bead_mw %f sum atom mw %f diff %f\n",
          last_res,
          last_resName.ascii(),
          last_asgn,
          prev_bead_mw,
          prev_atom_mw,
          prev_bead_mw -
          prev_atom_mw
          );
   printf("~~tot atom %d mw %f tot bead %d mw %f beads_size %u\n",
          atom_count,
          tot_atom_mw,
          bead_count,
          tot_bead_mw,
          (unsigned int)bead_model.size());
#endif

   write_bead_asa(somo_dir + SLASH +
                  project + QString("_%1").arg(current_model + 1) +
                  QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "")
                  + DOTSOMO + ".asa_res", &bead_model);

#if defined(DEBUG_MOD)
   write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_debug" + DOTSOMO + ".tsv", &dbg_model);
#endif
   editor->append(QString("There are %1 beads in this model before popping\n").arg(bead_model.size()));

   progress->setProgress(ppos++); // 8
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }

   // #define DEBUG

   // popping radial reduction
#if !defined(POP_MC)
# define POP_MC              (1 << 0)
# define POP_SC              (1 << 1)
# define POP_MCSC            (1 << 2)
# define POP_EXPOSED         (1 << 3)
# define POP_BURIED          (1 << 4)
# define POP_ALL             (1 << 5)
# define RADIAL_REDUCTION    (1 << 6)
# define RR_MC               (1 << 7)
# define RR_SC               (1 << 8)
# define RR_MCSC             (1 << 9)
# define RR_EXPOSED          (1 << 10)
# define RR_BURIED           (1 << 11)
# define RR_ALL              (1 << 12)
# define OUTWARD_TRANSLATION (1 << 13)
# define RR_HIERC            (1 << 14)
# define MIN_OVERLAP 0.0
#endif

   // or sc fb Y rh Y rs Y to Y st Y ro Y 70.000000 1.000000 0.000000
   // or scmc fb Y rh N rs N to Y st N ro Y 1.000000 0.000000 70.000000
   // or bb fb Y rh Y rs N to N st Y ro N 0.000000 0.000000 0.000000

#if defined(DEBUG)
   printf("or sc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or scmc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or bb fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or gb fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n",
          sidechain_overlap.fuse_beads ? "Y" : "N",
          sidechain_overlap.remove_hierarch ? "Y" : "N",
          sidechain_overlap.remove_sync ? "Y" : "N",
          sidechain_overlap.translate_out ? "Y" : "N",
          sidechain_overlap.show_translate ? "Y" : "N",
          sidechain_overlap.remove_overlap ? "Y" : "N",
          sidechain_overlap.fuse_beads_percent,
          sidechain_overlap.remove_sync_percent,
          sidechain_overlap.remove_hierarch_percent,

          mainchain_overlap.fuse_beads ? "Y" : "N",
          mainchain_overlap.remove_hierarch ? "Y" : "N",
          mainchain_overlap.remove_sync ? "Y" : "N",
          mainchain_overlap.translate_out ? "Y" : "N",
          mainchain_overlap.show_translate ? "Y" : "N",
          mainchain_overlap.remove_overlap ? "Y" : "N",
          mainchain_overlap.fuse_beads_percent,
          mainchain_overlap.remove_sync_percent,
          mainchain_overlap.remove_hierarch_percent,

          buried_overlap.fuse_beads ? "Y" : "N",
          buried_overlap.remove_hierarch ? "Y" : "N",
          buried_overlap.remove_sync ? "Y" : "N",
          buried_overlap.translate_out ? "Y" : "N",
          buried_overlap.show_translate ? "Y" : "N",
          buried_overlap.remove_overlap ? "Y" : "N",
          buried_overlap.fuse_beads_percent,
          buried_overlap.remove_sync_percent,
          buried_overlap.remove_hierarch_percent,

          grid_overlap.fuse_beads ? "Y" : "N",
          grid_overlap.remove_hierarch ? "Y" : "N",
          grid_overlap.remove_sync ? "Y" : "N",
          grid_overlap.translate_out ? "Y" : "N",
          grid_overlap.show_translate ? "Y" : "N",
          grid_overlap.remove_overlap ? "Y" : "N",
          grid_overlap.fuse_beads_percent,
          grid_overlap.remove_sync_percent,
          grid_overlap.remove_hierarch_percent);

#endif

   int methods[] =
      {
         RADIAL_REDUCTION | RR_SC | RR_EXPOSED,
         RADIAL_REDUCTION | RR_MCSC | RR_EXPOSED,
         RADIAL_REDUCTION | RR_MCSC | RR_BURIED,
      };

   if (no_rr) {
      methods[0] = 0;
      methods[1] = 0;
      methods[2] = 0;
   }

   if (sidechain_overlap.fuse_beads) {
      methods[0] |= POP_SC | POP_EXPOSED;
   }

   if (mainchain_overlap.fuse_beads) {
      methods[1] |= POP_MCSC | POP_EXPOSED;
   }

   if (buried_overlap.fuse_beads) {
      methods[2] |= POP_ALL | POP_BURIED;
   }

   if (sidechain_overlap.remove_hierarch) {
      methods[0] |= RR_HIERC;
   }

   if (mainchain_overlap.remove_hierarch) {
      methods[1] |= RR_HIERC;
   }

   if (buried_overlap.remove_hierarch) {
      methods[2] |= RR_HIERC;
   }

   if (sidechain_overlap.translate_out) {
      methods[0] |= OUTWARD_TRANSLATION;
   }

   if (mainchain_overlap.translate_out) {
      methods[1] |= OUTWARD_TRANSLATION;
   }

   if (buried_overlap.translate_out) {
      methods[2] |= OUTWARD_TRANSLATION;
   }

   if (!sidechain_overlap.remove_overlap) {
      methods[0] = 0;
   }

   if (!mainchain_overlap.remove_overlap) {
      methods[1] = 0;
   }

   if (!buried_overlap.remove_overlap) {
      methods[2] = 0;
   }

   float overlap[] =
      {
         sidechain_overlap.fuse_beads_percent / 100.0,
         mainchain_overlap.fuse_beads_percent / 100.0,
         buried_overlap.fuse_beads_percent / 100.0
      };

   float rr_overlap[] =
      {
         (sidechain_overlap.remove_hierarch ?
          sidechain_overlap.remove_hierarch_percent : sidechain_overlap.remove_sync_percent) / 100.0,
         (mainchain_overlap.remove_hierarch ?
          mainchain_overlap.remove_hierarch_percent : mainchain_overlap.remove_sync_percent) / 100.0,
         (buried_overlap.remove_hierarch ?
          buried_overlap.remove_hierarch_percent : buried_overlap.remove_sync_percent) / 100.0
      };


#if defined(WRITE_EXTRA_FILES)
   write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO + ".tsv", &bead_model);
   write_bead_spt(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO, &bead_model);
#endif
   for(unsigned int k = 0; k < sizeof(methods) / sizeof(int); k++) {

   stage_loop:

      int beads_popped = 0;

#if defined(DEBUG1) || defined(DEBUG)
      printf("popping stage %d %s%s%s%s%s%s%s%s%s%s%s%s%s%s%soverlap_reduction %f rroverlap %f\n",
             k,
             (methods[k] & POP_MC) ? "main chain " : "",
             (methods[k] & POP_SC) ? "side chain " : "",
             (methods[k] & POP_MCSC) ? "main & side chain " : "",
             (methods[k] & POP_EXPOSED) ? "exposed " : "",
             (methods[k] & POP_BURIED) ? "buried " : "",
             (methods[k] & POP_ALL) ? "all " : "",
             (methods[k] & RADIAL_REDUCTION) ? "radial reduction " : "",
             (methods[k] & RR_HIERC) ? "hierarchically " : "synchronously ",
             (methods[k] & RR_MC) ? "main chain " : "",
             (methods[k] & RR_SC) ? "side chain " : "",
             (methods[k] & RR_MCSC) ? "main & side chain " : "",
             (methods[k] & RR_EXPOSED) ? "exposed " : "",
             (methods[k] & RR_BURIED) ? "buried " : "",
             (methods[k] & RR_ALL) ? "all " : "",
             (methods[k] & OUTWARD_TRANSLATION) ? "outward translation " : "",
             overlap[k],
             rr_overlap[k]);
#endif

      if (overlap[k] < MIN_OVERLAP) {
         printf("using %f as minimum overlap\n", MIN_OVERLAP);
         overlap[k] = MIN_OVERLAP;
      }

      float max_intersection_volume;
      float intersection_volume = 0;
      int max_bead1 = 0;
      int max_bead2 = 0;
#if defined(DEBUG1) || defined(DEBUG)
      unsigned iter = 0;
#endif
      bool overlaps_exist;
#if defined(TIMING)
      gettimeofday(&start_tv, NULL);
#endif
      editor->append(QString("Begin popping stage %1\n").arg(k + 1));
      progress->setProgress(ppos++); // 9, 10, 11
      qApp->processEvents();
      if (stopFlag)
      {
         return -1;
      }

      do {
         // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_bp-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
         // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-bp-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
#if defined(DEBUG1) || defined(DEBUG)
         printf("popping iteration %d\n", iter++);
#endif
         max_intersection_volume = -1;
         overlaps_exist = false;
         if (methods[k] & (POP_MC | POP_SC | POP_MCSC | POP_EXPOSED | POP_BURIED | POP_ALL)) {
            for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
               for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG)
                  printf("checking popping stage %d beads %d %d on chains %d %d exposed %d %d active %s %s max iv %f\n",
                         k, i, j,
                         bead_model[i].chain,
                         bead_model[j].chain,
                         bead_model[i].exposed_code,
                         bead_model[j].exposed_code,
                         bead_model[i].active ? "Y" : "N",
                         bead_model[j].active ? "Y" : "N",
                         max_intersection_volume
                         );
#endif
                  if (bead_model[i].active &&
                      bead_model[j].active &&
                      ( ((methods[k] & POP_SC) &&
                         bead_model[i].chain == 1 &&
                         bead_model[j].chain == 1) ||
                        ((methods[k] & POP_MC) &&
                         bead_model[i].chain == 0 &&
                         bead_model[j].chain == 0) ||
                        ((methods[k] & POP_MCSC)
                         // &&
                         // (bead_model[i].chain != 1 ||
                         // bead_model[j].chain != 1))
                         ) ) &&
                      ( ((methods[k] & POP_EXPOSED) &&
                         bead_model[i].exposed_code == 1 &&
                         bead_model[j].exposed_code == 1) ||
                        ((methods[k] & POP_BURIED) &&
                         (bead_model[i].exposed_code != 1 ||
                          bead_model[j].exposed_code != 1)) ||
                        (methods[k] & POP_ALL) )) {
                     intersection_volume =
                        int_vol_2sphere(
                                        bead_model[i].bead_computed_radius,
                                        bead_model[j].bead_computed_radius,
                                        sqrt(
                                             pow(bead_model[i].bead_coordinate.axis[0] -
                                                 bead_model[j].bead_coordinate.axis[0], 2) +
                                             pow(bead_model[i].bead_coordinate.axis[1] -
                                                 bead_model[j].bead_coordinate.axis[1], 2) +
                                             pow(bead_model[i].bead_coordinate.axis[2] -
                                                 bead_model[j].bead_coordinate.axis[2], 2)) );
#if defined(DEBUG)
                     printf("this overlap bead %u %u vol %f rv1 %f rv2 %f r1 %f r2 %f p1 [%f,%f,%f] p2 [%f,%f,%f]\n",
                            bead_model[i].serial,
                            bead_model[j].serial,
                            intersection_volume,
                            bead_model[i].bead_ref_volume,
                            bead_model[j].bead_ref_volume,
                            bead_model[i].bead_computed_radius,
                            bead_model[j].bead_computed_radius,
                            bead_model[i].bead_coordinate.axis[0],
                            bead_model[i].bead_coordinate.axis[1],
                            bead_model[i].bead_coordinate.axis[2],
                            bead_model[j].bead_coordinate.axis[0],
                            bead_model[j].bead_coordinate.axis[1],
                            bead_model[j].bead_coordinate.axis[2]
                            );
#endif
                     if (intersection_volume > bead_model[i].bead_ref_volume * overlap[k] ||
                         intersection_volume > bead_model[j].bead_ref_volume * overlap[k]) {
                        overlaps_exist = true;
                        if (intersection_volume > max_intersection_volume) {
#if defined(DEBUG)
                           printf("best overlap so far bead %u %u vol %f\n",
                                  bead_model[i].serial,
                                  bead_model[j].serial,
                                  intersection_volume);
#endif
                           max_intersection_volume = intersection_volume;
                           max_bead1 = i;
                           max_bead2 = j;
                        }
                     }
                  }
               }
            }

            bool back_to_zero = false;
            if (overlaps_exist) {
               beads_popped++;
               //#define DEBUG_FUSED
#if defined(DEBUG1) || defined(DEBUG) || defined(DEBUG_FUSED)
               printf("popping beads %u %u int vol %f mw1 %f mw2 %f v1 %f v2 %f c1 [%f,%f,%f] c2 [%f,%f,%f]\n",
                      max_bead1,
                      max_bead2,
                      max_intersection_volume,
                      bead_model[max_bead1].bead_ref_mw,
                      bead_model[max_bead2].bead_ref_mw,
                      bead_model[max_bead1].bead_ref_volume,
                      bead_model[max_bead2].bead_ref_volume,
                      bead_model[max_bead1].bead_coordinate.axis[0],
                      bead_model[max_bead1].bead_coordinate.axis[1],
                      bead_model[max_bead1].bead_coordinate.axis[2],
                      bead_model[max_bead2].bead_coordinate.axis[0],
                      bead_model[max_bead2].bead_coordinate.axis[1],
                      bead_model[max_bead2].bead_coordinate.axis[2]
                      );
#endif
               if (bead_model[max_bead1].chain == 0 &&
                   bead_model[max_bead2].chain == 1) {
                  // always select the sc!
#if defined(DEBUG1) || defined(DEBUG) || defined(DEBUG_FUSED)
                  puts("swap beads");
#endif
                  int tmp = max_bead2;
                  max_bead2 = max_bead1;
                  max_bead1 = tmp;
               }
               if (bead_model[max_bead1].chain != bead_model[max_bead2].chain &&
                   k == 1) {
                  back_to_zero = true;
               }
               // bead_model[max_bead1].all_beads.push_back(&bead_model[max_bead1]); ??
               bead_model[max_bead1].all_beads.push_back(&bead_model[max_bead2]);
               for (unsigned int n = 0; n < bead_model[max_bead2].all_beads.size(); n++) {
                  bead_model[max_bead1].all_beads.push_back(bead_model[max_bead2].all_beads[n]);
               }

               bead_model[max_bead2].active = false;
               for (unsigned int m = 0; m < 3; m++) {
                  bead_model[max_bead1].bead_coordinate.axis[m] *= bead_model[max_bead1].bead_ref_mw;
                  bead_model[max_bead1].bead_coordinate.axis[m] +=
                     bead_model[max_bead2].bead_coordinate.axis[m] * bead_model[max_bead2].bead_ref_mw;
                  bead_model[max_bead1].bead_coordinate.axis[m] /= bead_model[max_bead1].bead_ref_mw + bead_model[max_bead2].bead_ref_mw;
               }
               bead_model[max_bead1].bead_ref_mw = bead_model[max_bead1].bead_ref_mw + bead_model[max_bead2].bead_ref_mw;
               bead_model[max_bead1].bead_ref_volume = bead_model[max_bead1].bead_ref_volume + bead_model[max_bead2].bead_ref_volume;
               // - max_intersection_volume;
               bead_model[max_bead1].bead_actual_radius =
                  bead_model[max_bead1].bead_computed_radius =
                  pow(3 * bead_model[max_bead1].bead_ref_volume / (4.0*M_PI), 1.0/3);
               // if fusing with a side chain bead, make sure the fused is side-chain
               // if (bead_model[max_bead2].chain) {
               //   bead_model[max_bead1].chain = 1;
               // }
               bead_model[max_bead1].normalized_ot_is_valid = false;
               bead_model[max_bead2].normalized_ot_is_valid = false;
#if defined(DEBUG)
               printf("after popping beads %d %d int volume %f radius %f mw %f vol %f coordinate [%f,%f,%f]\n",
                      bead_model[max_bead1].serial,
                      bead_model[max_bead2].serial,
                      intersection_volume,
                      bead_model[max_bead1].bead_computed_radius,
                      bead_model[max_bead1].bead_ref_mw,
                      bead_model[max_bead2].bead_ref_volume,
                      bead_model[max_bead1].bead_coordinate.axis[0],
                      bead_model[max_bead1].bead_coordinate.axis[1],
                      bead_model[max_bead1].bead_coordinate.axis[2]
                      );
#endif
               if (back_to_zero) {
                  editor->append(QString("Beads popped %1, Go back to stage %2\n").arg(beads_popped).arg(k));
                  printf("fused sc/mc bead in stage SC/MC, back to stage SC\n");
                  k = 0;
                  ppos -= 4;
                  goto stage_loop;
               }
            }
         } // if pop method
         // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_ap-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
         // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-ap-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
      } while(overlaps_exist);
#if defined(TIMING)
      gettimeofday(&end_tv, NULL);
      printf("popping %d time %lu\n",
             k,
             1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
             start_tv.tv_usec);
      fflush(stdout);
#endif
#if defined(WRITE_EXTRA_FILES)
      write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
      write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO, &bead_model);
#endif
      printf("stage %d beads popped %d\n", k, beads_popped);
      progress->setProgress(ppos++); // 12,13,14
      editor->append(QString("Beads popped %1.\nBegin radial reduction stage %2\n").arg(beads_popped).arg(k + 1));
      qApp->processEvents();
      if (stopFlag)
      {
         return -1;
      }


      // radial reduction phase
#if defined(TIMING)
      gettimeofday(&start_tv, NULL);
#endif

      if (methods[k] & RADIAL_REDUCTION) {
         BPair pair;
         vector <BPair> pairs;

         vector <bool> reduced;
         vector <bool> reduced_any; // this is for a final recomputation of the volumes
         vector <bool> last_reduced; // to prevent rescreening
         reduced.resize(bead_model.size());
         reduced_any.resize(bead_model.size());
         last_reduced.resize(bead_model.size());

         for (unsigned int i = 0; i < bead_model.size(); i++) {
            reduced_any[i] = false;
            last_reduced[i] = true;
         }

         int iter = 0;
         int count;
         float max_intersection_length;
         // bool tb[bead_model.size() * bead_model.size()];
         // printf("sizeof tb %d, bm.size^2 %d\n",
         //     sizeof(tb), bead_model.size() * bead_model.size());
#if defined(DEBUG_OVERLAP)
         overlap_check(methods[k] & RR_SC ? true : false,
                       methods[k] & RR_MCSC ? true : false,
                       methods[k] & RR_BURIED ? true : false);
#endif
         if (methods[k] & RR_HIERC) {
#if defined(DEBUG1) || defined(DEBUG)
            printf("preprocessing processing hierarchical radial reduction\n");
#endif
            max_intersection_length = 0;
            pairs.clear();
            count = 0;
            // build list of intersecting pairs
            for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
               for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG)
                  printf("checking radial stage %d beads %d %d on chains %d %d exposed code %d %d active %s %s max il %f\n",
                         k, i, j,
                         bead_model[i].chain,
                         bead_model[j].chain,
                         bead_model[i].exposed_code,
                         bead_model[j].exposed_code,
                         bead_model[i].active ? "Y" : "N",
                         bead_model[j].active ? "Y" : "N",
                         max_intersection_length
                         );
#endif
                  if (
                      bead_model[i].active &&
                      bead_model[j].active &&
                      (methods[k] & RR_MCSC ||
                       ((methods[k] & RR_SC) &&
                        bead_model[i].chain == 1 &&
                        bead_model[j].chain == 1)) &&
                      ((methods[k] & RR_BURIED) ||
                       (bead_model[i].exposed_code == 1 ||
                        bead_model[j].exposed_code == 1)) &&
                      bead_model[i].bead_computed_radius > TOLERANCE &&
                      bead_model[j].bead_computed_radius > TOLERANCE
                      ) {

                     float separation =
                        bead_model[i].bead_computed_radius +
                        bead_model[j].bead_computed_radius -
                        sqrt(
                             pow(bead_model[i].bead_coordinate.axis[0] -
                                 bead_model[j].bead_coordinate.axis[0], 2) +
                             pow(bead_model[i].bead_coordinate.axis[1] -
                                 bead_model[j].bead_coordinate.axis[1], 2) +
                             pow(bead_model[i].bead_coordinate.axis[2] -
                                 bead_model[j].bead_coordinate.axis[2], 2));

#if defined(DEBUG)
                     printf("beads %d %d with radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] have a sep of %f\n",
                            i, j,
                            bead_model[i].bead_computed_radius,
                            bead_model[j].bead_computed_radius,
                            bead_model[i].bead_coordinate.axis[0],
                            bead_model[i].bead_coordinate.axis[1],
                            bead_model[i].bead_coordinate.axis[2],
                            bead_model[j].bead_coordinate.axis[0],
                            bead_model[j].bead_coordinate.axis[1],
                            bead_model[j].bead_coordinate.axis[2],
                            separation);
#endif
                     if (separation <= TOLERANCE) {
                        continue;
                     }

                     pair.i = i;
                     pair.j = j;
                     pair.separation = separation;
                     pair.active = true;
                     pairs.push_back(pair);
                     count++;
                  }
               }
            }
            // ok, now we have the list of pairs
            max_bead1 =
               max_bead2 = -1;
            float radius_delta;
            do {
#if defined(DEBUG1) || defined(DEBUG)
               printf("processing hierarchical radial reduction iteration %d\n", iter++);
#endif
               max_intersection_length = 0;
               int max_pair = -1;
               count = 0;
               for (unsigned int i = 0; i < pairs.size(); i++) {
#if defined(DEBUG_OVERLAP)
                  printf("pair %d %d sep %f %s %s\n",
                         pairs[i].i, pairs[i].j, pairs[i].separation, pairs[i].active ? "active" : "not active",
                         pairs[i].i == max_bead1 ||
                         pairs[i].j == max_bead1 ||
                         pairs[i].i == max_bead2 ||
                         pairs[i].j == max_bead2 ? "needs recompute of separation" : "separation valid");
#endif
                  if (pairs[i].active) {
                     if (
                         pairs[i].i == max_bead1 ||
                         pairs[i].j == max_bead1 ||
                         pairs[i].i == max_bead2 ||
                         pairs[i].j == max_bead2
                         ) {
                        pairs[i].separation =
                           bead_model[pairs[i].i].bead_computed_radius +
                           bead_model[pairs[i].j].bead_computed_radius -
                           sqrt(
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[0] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[0], 2) +
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[1] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[1], 2) +
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[2] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[2], 2));
                        pairs[i].active = true;
                     }
                     if (pairs[i].separation > max_intersection_length) {
                        max_intersection_length = pairs[i].separation;
                        max_pair = i;
                     }
                  }
               }

               if (max_intersection_length > TOLERANCE) {
                  count++;
                  pairs[max_pair].active = false;
                  max_bead1 = pairs[max_pair].i;
                  max_bead2 = pairs[max_pair].j;
#if defined(DEBUG1) || defined(DEBUG)
                  printf("processing radial reduction hierc iteration %d pair %d processed %d\n", iter, max_pair, count);
                  printf("reducing beads %d %d\n", max_bead1, max_bead2);
#endif
                  do {
                     if (methods[k] & OUTWARD_TRANSLATION ||
                         ((bead_model[max_bead1].chain == 1 ||
                           bead_model[max_bead2].chain == 1) &&
                          methods[0] & OUTWARD_TRANSLATION)) {
                        // new 1 step ot
                        if((methods[k] & RR_MCSC &&
                            (bead_model[max_bead1].chain == 1 ||
                             bead_model[max_bead2].chain == 1))
                           ) {
                           // new 1 bead 1 OT / treat as no ot...
                           int use_bead;
                           if (bead_model[max_bead1].chain == 1) {
                              use_bead = max_bead2;
                           }
                           else 
                           {
                              use_bead = max_bead1;
                           }
                           // one bead to shrink
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta = max_intersection_length;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required, one bead to shrink, radius delta %f  cr %f %f\n",
                                  use_bead,
                                  radius_delta,
                                  bead_model[use_bead].bead_computed_radius,
                                  bead_model[use_bead].bead_computed_radius - radius_delta
                                  );
#endif
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                        else 
                        {
                           int use_bead = max_bead1;
                           if (!bead_model[use_bead].normalized_ot_is_valid) {
                              float norm = 0.0;
                              for (unsigned int l = 0; l < 3; l++) {
                                 bead_model[use_bead].normalized_ot.axis[l] =
                                    bead_model[use_bead].bead_coordinate.axis[l] -
                                    molecular_cog[l];
                                 norm +=
                                    bead_model[use_bead].normalized_ot.axis[l] *
                                    bead_model[use_bead].normalized_ot.axis[l];
                              }
                              norm = sqrt(norm);
                              if (norm) {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                 }
                                 bead_model[use_bead].normalized_ot_is_valid = true;
                              }
                              else 
                              {
                                 printf("wow! bead %d is at the molecular cog!\n", use_bead);
                              }
                           }
                           use_bead = max_bead2;
                           if (!bead_model[use_bead].normalized_ot_is_valid) {
                              float norm = 0.0;
                              for (unsigned int l = 0; l < 3; l++) {
                                 bead_model[use_bead].normalized_ot.axis[l] =
                                    bead_model[use_bead].bead_coordinate.axis[l] -
                                    molecular_cog[l];
                                 norm +=
                                    bead_model[use_bead].normalized_ot.axis[l] *
                                    bead_model[use_bead].normalized_ot.axis[l];
                              }
                              norm = sqrt(norm);
                              if (norm) {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                 }
                                 bead_model[use_bead].normalized_ot_is_valid = true;
                              }
                              else 
                              {
                                 printf("wow! bead %d is at the molecular cog!\n", use_bead);
                              }
                           }
                           // we need to handle 1 fixed case and
                           // the slight potential of one being at the molecular cog
                           reduced_any[max_bead1] = true;
                           reduced_any[max_bead2] = true;
                           outward_translate_2_spheres(
                                                       &bead_model[max_bead1].bead_computed_radius,
                                                       &bead_model[max_bead2].bead_computed_radius,
                                                       bead_model[max_bead1].bead_coordinate.axis,
                                                       bead_model[max_bead2].bead_coordinate.axis,
                                                       bead_model[max_bead1].normalized_ot.axis,
                                                       bead_model[max_bead2].normalized_ot.axis
                                                       );
                        }
                     }
                     else 
                     {
                        // no outward translation is required for either bead
                        // are we shrinking just 1 bead ... if we are dealing with buried beads, then
                        // only buried beads should be shrunk, not exposed beads
#if defined(DEBUG2)
                        printf("no outward translation is required\n");
#endif
                        if(methods[k] & RR_BURIED &&
                           bead_model[max_bead1].exposed_code == 1 &&
                           bead_model[max_bead2].exposed_code == 1) {
                           printf("what are we doing here?  buried and two exposed??\n");
                           exit(-1);
                        }
                        if(methods[k] & RR_MCSC &&
                           !(methods[k] & RR_BURIED) &&
                           bead_model[max_bead1].chain == 1 &&
                           bead_model[max_bead2].chain == 1) {
                           printf("what are we doing here?  dealing with 2 SC's on the MCSC run??\n");
                           exit(-1);
                        }
                        if((methods[k] & RR_BURIED &&
                            (bead_model[max_bead1].exposed_code == 1 ||
                             bead_model[max_bead2].exposed_code == 1)) ||
                           (methods[k] & RR_MCSC &&
                            !(methods[k] & RR_BURIED) &&
                            (bead_model[max_bead1].chain == 1 ||
                             bead_model[max_bead2].chain == 1))) {
                           // only one bead to shrink, since
                           // we are either buried with one of the beads exposed or
                           // on the MCSC and one of the beads is SC
                           int use_bead;
                           if (methods[k] & RR_BURIED) {
                              if (bead_model[max_bead1].exposed_code == 1) {
                                 use_bead = max_bead2;
                              }
                              else 
                              {
                                 use_bead = max_bead1;
                              }
                           }
                           else 
                           {
                              if (bead_model[max_bead1].chain == 1) {
                                 use_bead = max_bead2;
                              }
                              else 
                              {
                                 use_bead = max_bead1;
                              }
                           }
                           // one bead to shrink
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta = max_intersection_length;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required, one bead to shrink, radius delta %f  cr %f %f\n",
                                  use_bead,
                                  radius_delta,
                                  bead_model[use_bead].bead_computed_radius,
                                  bead_model[use_bead].bead_computed_radius - radius_delta
                                  );
#endif
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                        else 
                        {
                           // two beads to shrink
                           int use_bead = max_bead1;
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta =
                              // bead_model[use_bead].bead_actual_radius * rr_overlap[k];
                              max_intersection_length * bead_model[max_bead1].bead_computed_radius /
                              (bead_model[max_bead1].bead_computed_radius + bead_model[max_bead2].bead_computed_radius);
                           float radius_delta2 =
                              // bead_model[use_bead].bead_actual_radius * rr_overlap[k];
                              max_intersection_length * bead_model[max_bead2].bead_computed_radius /
                              (bead_model[max_bead1].bead_computed_radius + bead_model[max_bead2].bead_computed_radius);
#if defined(DEBUG2)
                           printf("intersection len %f recomputed %f radius delta %f r1 %f r2 %f\n",
                                  max_intersection_length,
                                  bead_model[max_bead1].bead_computed_radius +
                                  bead_model[max_bead2].bead_computed_radius -
                                  sqrt(
                                       pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                           bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                           bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                           bead_model[max_bead2].bead_coordinate.axis[2], 2)),
                                  radius_delta,
                                  bead_model[max_bead1].bead_computed_radius,
                                  bead_model[max_bead2].bead_computed_radius
                                  );
#endif
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE / 2);
                              reduced[use_bead] = false;
                           }

#if defined(DEBUG2)
                           printf("intersection len %f recomputed %f radius delta %f r1 %f r2 %f\n",
                                  max_intersection_length,
                                  bead_model[max_bead1].bead_computed_radius +
                                  bead_model[max_bead2].bead_computed_radius -
                                  sqrt(
                                       pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                           bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                           bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                           bead_model[max_bead2].bead_coordinate.axis[2], 2)),
                                  radius_delta,
                                  bead_model[max_bead1].bead_computed_radius,
                                  bead_model[max_bead2].bead_computed_radius
                                  );
#endif
                           use_bead = max_bead2;
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = radius_delta2;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                     }
#if defined(DEBUG2)
                     printf("b1r %f b2r %f current separation %f\n",
                            bead_model[max_bead1].bead_computed_radius,
                            bead_model[max_bead2].bead_computed_radius,
                            bead_model[max_bead1].bead_computed_radius +
                            bead_model[max_bead2].bead_computed_radius -
                            sqrt(
                                 pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                     bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                     bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                     bead_model[max_bead2].bead_coordinate.axis[2], 2)));
                     printf("flags %s %s %s %s\n",
                            bead_model[max_bead1].bead_computed_radius > TOLERANCE ? "Y" : "N",
                            bead_model[max_bead2].bead_computed_radius > TOLERANCE ? "Y" : "N",
                            (bead_model[max_bead1].bead_computed_radius +
                             bead_model[max_bead2].bead_computed_radius -
                             sqrt(
                                  pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                      bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                      bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                      bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE) ? "Y" : "N",
                            bead_model[max_bead1].bead_computed_radius > TOLERANCE &&
                            bead_model[max_bead2].bead_computed_radius > TOLERANCE &&
                            (bead_model[max_bead1].bead_computed_radius +
                             bead_model[max_bead2].bead_computed_radius -
                             sqrt(
                                  pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                      bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                      bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                      bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE) ? "Y" : "N");
#endif
                  } while (
                           bead_model[max_bead1].bead_computed_radius > TOLERANCE &&
                           bead_model[max_bead2].bead_computed_radius > TOLERANCE &&
                           (bead_model[max_bead1].bead_computed_radius +
                            bead_model[max_bead2].bead_computed_radius -
                            sqrt(
                                 pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                     bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                     bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                     bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE));
#if defined(DEBUG2)
                  printf("out of while 1\n");
#endif
               } // if max intersection length > TOLERANCE
            } while(count);
#if defined(DEBUG2)
            printf("out of while 2\n");
#endif
         }
         else 
         {
            // simultaneous reduction
            // #define DEBUG
            do {
#if defined(DEBUG_OVERLAP)
               overlap_check(methods[k] & RR_SC ? true : false,
                             methods[k] & RR_MCSC ? true : false,
                             methods[k] & RR_BURIED ? true : false);
#endif
               // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_br-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
               // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-br-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
#if defined(DEBUG1) || defined(DEBUG)
               printf("processing simultaneous radial reduction iteration %d\n", iter++);
#endif
               if(iter > 10000) {
                  printf("too many iterations\n");
                  exit(-1);
               }
               max_intersection_length = 0;
               pairs.clear();
               count = 0;
               reduced[bead_model.size() - 1] = false;
               for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
                  reduced[i] = false;
                  if (1 || last_reduced[i]) {
                     for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUGX)
                        printf("checking radial stage %d beads %d %d on chains %d %d exposed code %d %d active %s %s max il %f\n",
                               k, i, j,
                               bead_model[i].chain,
                               bead_model[j].chain,
                               bead_model[i].exposed_code,
                               bead_model[j].exposed_code,
                               bead_model[i].active ? "Y" : "N",
                               bead_model[j].active ? "Y" : "N",
                               max_intersection_length
                               );
#endif
                        if ((1 || last_reduced[j]) &&
                            bead_model[i].active &&
                            bead_model[j].active &&
                            (methods[k] & RR_MCSC ||
                             ((methods[k] & RR_SC) &&
                              bead_model[i].chain == 1 &&
                              bead_model[j].chain == 1)) &&
                            ((methods[k] & RR_BURIED) ||
                             (bead_model[i].exposed_code == 1 ||
                              bead_model[j].exposed_code == 1)) &&
                            bead_model[i].bead_computed_radius > TOLERANCE &&
                            bead_model[j].bead_computed_radius > TOLERANCE
                            ) {
                           float separation =
                              bead_model[i].bead_computed_radius +
                              bead_model[j].bead_computed_radius -
                              sqrt(
                                   pow(bead_model[i].bead_coordinate.axis[0] -
                                       bead_model[j].bead_coordinate.axis[0], 2) +
                                   pow(bead_model[i].bead_coordinate.axis[1] -
                                       bead_model[j].bead_coordinate.axis[1], 2) +
                                   pow(bead_model[i].bead_coordinate.axis[2] -
                                       bead_model[j].bead_coordinate.axis[2], 2));


                           if (separation <= TOLERANCE) {
                              continue;
                           }

#if defined(DEBUG)
                           printf("beads %d %d with radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] have a sep of %f\n",
                                  i, j,
                                  bead_model[i].bead_computed_radius,
                                  bead_model[j].bead_computed_radius,
                                  bead_model[i].bead_coordinate.axis[0],
                                  bead_model[i].bead_coordinate.axis[1],
                                  bead_model[i].bead_coordinate.axis[2],
                                  bead_model[j].bead_coordinate.axis[0],
                                  bead_model[j].bead_coordinate.axis[1],
                                  bead_model[j].bead_coordinate.axis[2],
                                  separation);
#endif

                           if (separation > max_intersection_length) {
                              max_intersection_length = separation;
                              max_bead1 = i;
                              max_bead2 = j;
                           }
                           pair.i = i;
                           pair.j = j;
                           pair.separation = separation;
                           pairs.push_back(pair);
                           count++;
                        }
                     }
                  } // if last_reduced[i]
               }
#if defined(DEBUG1) || defined(DEBUG)
               printf("processing radial reduction sync iteration %d pairs to process %d max int len %f\n", iter, count, max_intersection_length);
#endif
               if (max_intersection_length > TOLERANCE) {
#if defined(DEBUG1) || defined(DEBUG)
                  printf("processing radial reduction sync iteration %d pairs to process %d\n", iter, count);
#endif
                  for (unsigned int i = 0; i < pairs.size(); i++) {
                     if (
                         !reduced[pairs[i].i] &&
                         (bead_model[pairs[i].i].exposed_code != 1 ||
                          methods[k] & RR_EXPOSED ||
                          methods[k] & RR_ALL) &&
                         (!(methods[k] & RR_MCSC) ||
                          bead_model[pairs[i].i].chain == 0 ||
                          (methods[k] & RR_BURIED &&
                           bead_model[pairs[i].i].exposed_code != 1))
                         ) {
                        int use_bead = pairs[i].i;
                        /*      if ( !(methods[k] & RR_MCSC) ||
                                bead_model[use_bead].exposed_code != 1 ||
                                bead_model[use_bead].chain == 0 ||
                                (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1) ) */
                        if(1) {
#if defined(DEBUG)
                           printf("reducing beads %d\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = bead_model[use_bead].bead_computed_radius * rr_overlap[k];
                           if (radius_delta < TOLERANCE) {
                              radius_delta = (float)TOLERANCE;
                           }
                           if (methods[k] & OUTWARD_TRANSLATION ||
                               (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1 &&
                                methods[0] & OUTWARD_TRANSLATION) ) {
#if defined(DEBUG)
                              printf("outward translation from [%f,%f,%f] to ",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                              if (!bead_model[use_bead].normalized_ot_is_valid) {
                                 float norm = 0.0;
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] =
                                       bead_model[use_bead].bead_coordinate.axis[l] -
                                       molecular_cog[l];
                                    norm +=
                                       bead_model[use_bead].normalized_ot.axis[l] *
                                       bead_model[use_bead].normalized_ot.axis[l];
                                 }
                                 norm = sqrt(norm);
                                 if (norm) {
                                    for (unsigned int l = 0; l < 3; l++) {
                                       bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                       bead_model[use_bead].bead_coordinate.axis[l] +=
                                          radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                    }
                                    bead_model[use_bead].normalized_ot_is_valid = true;
                                 }
                                 else 
                                 {
                                    printf("wow! bead %d is at the molecular cog!\n", use_bead);
                                 }
                              }
                              else 
                              {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].bead_coordinate.axis[l] +=
                                       radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                 }
                              }
#if defined(DEBUG)
                              printf(" [%f,%f,%f]\n",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)TOLERANCE * 0.9999;
                              reduced[use_bead] = false;
                           }
                        }
                     }
                     if (
                         !reduced[pairs[i].j] &&
                         (bead_model[pairs[i].j].exposed_code != 1 ||
                          methods[k] & RR_EXPOSED ||
                          methods[k] & RR_ALL) &&
                         (!(methods[k] & RR_MCSC) ||
                          bead_model[pairs[i].j].chain == 0 ||
                          (methods[k] & RR_BURIED &&
                           bead_model[pairs[i].j].exposed_code != 1))
                         ) {
                        int use_bead = pairs[i].j;
                        /* if ( !(methods[k] & RR_MCSC) ||
                           bead_model[use_bead].chain == 0 ||
                           bead_model[use_bead].exposed_code != 1 ||
                           (bead_model[pairs[i].i].chain == 1 &&
                           bead_model[pairs[i].j].chain == 1) ) */
                        {
#if defined(DEBUG)
                           printf("reducing beads %d\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = bead_model[use_bead].bead_computed_radius * rr_overlap[k];
                           if (radius_delta < TOLERANCE) {
                              radius_delta = (float)TOLERANCE;
                           }
                           if (methods[k] & OUTWARD_TRANSLATION ||
                               (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1 &&
                                methods[0] & OUTWARD_TRANSLATION) ) {
#if defined(DEBUG)
                              printf("outward translation from [%f,%f,%f] to ",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                              if (!bead_model[use_bead].normalized_ot_is_valid) {
                                 float norm = 0.0;
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] =
                                       bead_model[use_bead].bead_coordinate.axis[l] -
                                       molecular_cog[l];
                                    norm +=
                                       bead_model[use_bead].normalized_ot.axis[l] *
                                       bead_model[use_bead].normalized_ot.axis[l];
                                 }
                                 norm = sqrt(norm);
                                 if (norm) {
                                    for (unsigned int l = 0; l < 3; l++) {
                                       bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                       bead_model[use_bead].bead_coordinate.axis[l] +=
                                          radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                    }
                                    bead_model[use_bead].normalized_ot_is_valid = true;
                                 }
                                 else 
                                 {
                                    printf("wow! bead %d is at the molecular cog!\n", use_bead);
                                 }
                              }
                              else 
                              {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].bead_coordinate.axis[l] +=
                                       radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                 }
                              }
#if defined(DEBUG)
                              printf(" [%f,%f,%f]\n",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)TOLERANCE * 0.9999;
                              reduced[use_bead] = false;
                           }
                        }
                     }
                  }
               }
               last_reduced = reduced;
               // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_ar-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
               // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-ar-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
            } while(count);
         }
#if defined(TIMING)
         gettimeofday(&end_tv, NULL);
         printf("radial reduction %d time %lu\n",
                k,
                1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
                start_tv.tv_usec);
         fflush(stdout);
#endif
         progress->setProgress(ppos++); // 15,16,17
         qApp->processEvents();
         if (stopFlag)
         {
            return -1;
         }

         // recompute volumes
         for (unsigned int i = 0; i < bead_model.size(); i++) {
            if (reduced_any[i]) {
#if defined(DEBUG1) || defined(DEBUG)
               printf("recomputing volume bead %d\n", i);
#endif
               bead_model[i].bead_ref_volume =
                  (4.0*M_PI/3.0) * pow(bead_model[i].bead_computed_radius, 3);
            }
         }
      }
#if defined(DEBUG_OVERLAP)
      if(overlap_check(methods[k] & RR_SC ? true : false,
                       methods[k] & RR_MCSC ? true : false,
                       methods[k] & RR_BURIED ? true : false)) {
         printf("internal over lap error... exiting!");
         exit(-1);
      }
#endif
#if defined(WRITE_EXTRA_FILES)
      write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
      write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO, &bead_model);
#endif
   } // methods
#if defined(WRITE_EXTRA_FILES)
   write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO + ".tsv", &bead_model);
   write_bead_spt(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO, &bead_model);
#endif
   //  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
   //       QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
   //       DOTSOMO, &bead_model);
   write_bead_model(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
                    QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") + DOTSOMO
                    , &bead_model);
   editor->append("Finished with popping and radial reduction\n");
   progress->setProgress(mppos - (asa.recheck_beads ? 1 : 0));
   qApp->processEvents();
   if (stopFlag)
   {
      return -1;
   }
   return 0;
}

void US_Hydrodyn::write_bead_ebf(QString fname, vector<PDB_atom> *model) {
   {
      FILE *f = fopen(fname.ascii(), "w");
      for (unsigned int i = 0; i < model->size(); i++) {
         if ((*model)[i].active) {
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
      for (unsigned int i = 0; i < model->size(); i++) {
         if ((*model)[i].active) {
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

// #define DEBUG_COLOR

int US_Hydrodyn::get_color(PDB_atom *a) {
   int color = a->bead_color;
   if (a->all_beads.size()) {
      color = 7;
   }
   if (a->exposed_code != 1) {
      color = 6;
   }
   if (a->bead_computed_radius <= TOLERANCE) {
      color = 0;
   }
   //  color = a->bead_number % 15;
#if defined DEBUG_COLOR
   color = 0;
   if (a->chain == 1) {
      color = 4;
      if (a->exposed_code != 1) {
         color = 6;
      }
   }
   else 
   {
      color = 1;
      if (a->exposed_code != 1) {
         color = 10;
      }
   }
#endif
   return color;
}

void US_Hydrodyn::write_bead_spt(QString fname, vector<PDB_atom> *model) {

   char *colormap[] =
      {
         "black",        // 0 black
         "blue",         // 1 blue
         "greenblue",    // 2 greenblue
         "cyan",         // 3 cyan
         "red",          // 4 red
         "magenta",      // 5 magenta
         "orange",       // 6 orange
         "white",        // 7 white
         "redorange",    // 8 redorange
         "purple",       // 9 purple
         "green",        // 10 green
         "cyan",         // 11 cyan
         "redorange",    // 12 redorange
         "violet",       // 13 violet
         "yellow",       // 14 yellow
      };

#if defined(DEBUG)
   printf("write bead spt %s\n", fname.ascii()); fflush(stdout);
#endif

   FILE *fspt = fopen(QString("%1.spt").arg(fname).ascii(), "w");
   FILE *fbms = fopen(QString("%1.bms").arg(fname).ascii(), "w");

   int beads = 0;
   if(!(fspt) ||
      !(fbms)) {
      printf("file write error!\n"); fflush(stdout);
   }

   float max_radius = 0;
   for (unsigned int i = 0; i < model->size(); i++) {
      if ((*model)[i].active) {
         beads++;
         if (max_radius < (*model)[i].bead_computed_radius) {
            max_radius = (*model)[i].bead_computed_radius;
         }
      }
   }

# define MAX_RADIUS 10.0
   float scaling = 1.0;
   if(max_radius > MAX_RADIUS) {
      scaling = max_radius / MAX_RADIUS;
   }
   scaling = 5.0;
   fprintf(fbms,
           "%d\n%s\n",
           beads,
           QFileInfo(fname).fileName().ascii()
           );
   fprintf(fspt,
           "load xyz %s\nselect all\nwireframe off\nset background white\n",
           QString("%1.bms").arg(QFileInfo(fname).fileName()).ascii()
           );

   int atomno = 0;
   for (unsigned int i = 0; i < model->size(); i++) {
      if ((*model)[i].active) {
         if ((*model)[i].bead_color >= (sizeof(colormap) / sizeof(char))) {
            printf("ERROR: bead color for bead %u is to large %u\n",
                   (*model)[i].serial,
                   get_color(&(*model)[i])); fflush(stdout);
         }
         fprintf(fbms,
                 "Pb %.2f %.2f %.2f\n",
                 (*model)[i].bead_coordinate.axis[0] / scaling,
                 (*model)[i].bead_coordinate.axis[1] / scaling,
                 (*model)[i].bead_coordinate.axis[2] / scaling
                 );
         fprintf(fspt,
                 "select atomno=%d\nspacefill %.2f\ncolour %s\n",
                 atomno++,
                 (*model)[i].bead_computed_radius / scaling,
                 colormap[get_color(&(*model)[i])]
                 );
      }
   }
   fclose(fspt);
   fclose(fbms);
}

void US_Hydrodyn::write_bead_tsv(QString fname, vector<PDB_atom> *model) {
   FILE *f = fopen(fname.ascii(), "w");
   fprintf(f, "name~residue~chainID~"
           "position~active~radius~asa~mw~"
           "bead #~chain~serial~is_bead~bead_asa~visible~code/color~"
           "bead mw~position controlled?~bead positioner~baric method~bead hydration~bead color~"
           "bead ref volume~bead ref mw~bead computed radius~"
           "position_coordinate~cog position~use position~sizeof(parents)~beads referenced\n");

   for (unsigned int i = 0; i < model->size(); i++) {
      if ((*model)[i].active) {
         QString beads_referenced = "";
         for (unsigned int j = 0; j < (*model)[i].all_beads.size(); j++) {

            beads_referenced +=
               QString("%1-%2-%3-%4;").
               arg(((*model)[i].all_beads)[j]->serial).
               arg(((*model)[i].all_beads)[j]->name).
               arg(((*model)[i].all_beads)[j]->resName).
               arg(((*model)[i].all_beads)[j]->chainID);

            //     ((*model)[i].all_beads)[j]->serial + "-" +
            //     ((*model)[i].all_beads)[j]->name + "-" +
            //     ((*model)[i].all_beads)[j]->resName + "-" +
            //     ((*model)[i].all_beads)[j]->chainID + ";";
         }
         fprintf(f,
                 "%s~%s~%s~"
                 "[%f,%f,%f]~%s~%f~%f~%f~"
                 "%d~%d~%d~%s~%f~%s~"
                 "%d~%f~%s~%s~%d~%u~%u~"
                 "%f~%f~%f~"
                 "[%f,%f,%f]~[%f,%f,%f]~[%f, %f, %f]~%u~%s\n",

                 (*model)[i].name.ascii(),
                 (*model)[i].resName.ascii(),
                 (*model)[i].chainID.ascii(),

                 (*model)[i].coordinate.axis[0],
                 (*model)[i].coordinate.axis[1],
                 (*model)[i].coordinate.axis[2],
                 (*model)[i].active ? "Y" : "N",
                 (*model)[i].radius,
                 (*model)[i].asa,
                 (*model)[i].mw,

                 (*model)[i].bead_number,
                 (*model)[i].chain,
                 (*model)[i].serial,
                 (*model)[i].is_bead ? "Y" : "N",
                 (*model)[i].bead_asa,
                 (*model)[i].visibility ? "Y" : "N",

                 (*model)[i].exposed_code,
                 (*model)[i].bead_mw,
                 (*model)[i].bead_positioner ? "Y" : "N",
                 (*model)[i].active ? ((*model)[i].bead_positioner ? "Y" : "N") : "Inactive",
                 (*model)[i].placing_method,
                 (*model)[i].bead_hydration,
                 (*model)[i].bead_color,

                 (*model)[i].bead_ref_volume,
                 (*model)[i].bead_ref_mw,
                 (*model)[i].bead_computed_radius,

                 (*model)[i].bead_position_coordinate.axis[0],
                 (*model)[i].bead_position_coordinate.axis[1],
                 (*model)[i].bead_position_coordinate.axis[2],
                 (*model)[i].bead_cog_coordinate.axis[0],
                 (*model)[i].bead_cog_coordinate.axis[1],
                 (*model)[i].bead_cog_coordinate.axis[2],
                 (*model)[i].bead_coordinate.axis[0],
                 (*model)[i].bead_coordinate.axis[1],
                 (*model)[i].bead_coordinate.axis[2],
                 (unsigned int)(*model)[i].all_beads.size(),
                 beads_referenced.ascii()
                 );
      }
   }
   fclose(f);
}

void US_Hydrodyn::write_bead_asa(QString fname, vector<PDB_atom> *model) {
   FILE *f = fopen(fname.ascii(), "w");
   fprintf(f, " N.      Res.       ASA        MAX ASA         %%\n");

   float total_asa = 0.0;
   float total_ref_asa = 0.0;
   float total_vol = 0.0;
   float total_mass = 0.0;

   QString last_residue = "";
   int seqno = 0;
   float residue_asa = 0;
   float residue_ref_asa = 0;

   for (unsigned int i = 0; i < model->size(); i++) {
      if ((*model)[i].active) {
         total_asa += (*model)[i].bead_asa;
         total_ref_asa += (*model)[i].ref_asa;
         total_mass += (*model)[i].bead_ref_mw;
         total_vol += (*model)[i].bead_ref_volume_unhydrated;

         QString residue =
            (*model)[i].resName + "_" +
            ((*model)[i].chainID == " " ? "_" : (*model)[i].chainID) +
            QString("_%1").arg((*model)[i].resSeq);
         if (residue != last_residue) {
            if (last_residue != "") {
               fprintf(f,
                       " [ %-6d %s ]\t%.0f\t%.0f\t%.2f\n",
                       seqno, last_residue.ascii(), residue_asa, residue_ref_asa, 100.0 * residue_asa / residue_ref_asa);
            }
            residue_asa = 0;
            residue_ref_asa = 0;
            last_residue = residue;
            seqno++;
         }
         residue_asa += (*model)[i].bead_asa;
         residue_ref_asa = (*model)[i].ref_asa;
      }
   }
   if (last_residue != "") {
      fprintf(f,
              " [ %-6d %s ]\t%.0f\t%.0f\t%.2f\n",
              seqno, last_residue.ascii(), residue_asa, residue_ref_asa, 100.0 * residue_asa / residue_ref_asa);
   }

   fprintf(f,
           "\n\n\n"
           "\tTOTAL ASA OF THE MOLECULE    = %.0f\t[A^2] (Threshold used: %.1f A^2]\n"
           "\tTOTAL VOLUME OF THE MOLECULE = %-.2f\t[A^3]\n"
           //     "\tRADIUS OF GYRATION (+r) =  %-.2f\t[A]\n"
           //     "\tRADIUS OF GYRATION (-r) =  %-.2f   [A]\n"
           "\tMASS OF THE MOLECULE    =  %.0f   [Da]\n"
           "\tCENTRE OF MASS          =  %.4f %.4f %.4f [A]\n"
           ,
           total_asa,
           asa.threshold,
           total_vol,
           // 0.0,
           // 0.0,
           total_mass,
           last_molecular_cog.axis[0], last_molecular_cog.axis[1], last_molecular_cog.axis[2]
           );

   fclose(f);
}



void US_Hydrodyn::write_bead_model(QString fname, vector<PDB_atom> *model) {

   int decpts = -(int)log10(overlap_tolerance/9.9999) + 1;
   if (decpts < 4) {
      decpts = 4;
   }

   QString fstring_somo =
      QString("%.%1f\t%.%2f\t%.%3f\t%.%4f\t%.2f\t%d\t%s\t%.4f\n").
      arg(decpts).
      arg(decpts).
      arg(decpts).
      arg(decpts);

   QString fstring_beams =
      QString("%.%1f\t%.%2f\t%.%3f\n").
      arg(decpts).
      arg(decpts).
      arg(decpts);

   QString fstring_rmc =
      QString("%.%1f\t%.0f\t%d\n"). // zero digit MW
      arg(decpts);

   QString fstring_rmc1 =
      QString("%.%1f\t%.0f\t%d\t%s\n"). // zero digit MW
      arg(decpts);

   QString fstring_hydro =
      QString("%.%1f\t%.%2f\t%.%3f\t%.%4f\n").
      arg(decpts).
      arg(decpts).
      arg(decpts).
      arg(decpts);

#if defined(DEBUG)
   printf("write bead model %s\n", fname.ascii()); fflush(stdout);
   printf("decimal points to use: %d\n", decpts);
#endif

   vector <PDB_atom *> use_model;
   switch (bead_output.sequence) {
   case 0: // as in original pdb file
   case 2: // include bead-original residue correspondence
      for (unsigned int i = 0; i < model->size(); i++) {
         use_model.push_back(&(*model)[i]);
      }
      break;
   case 1: // exposed sidechain -> exposed main chain -> buried
      for (unsigned int i = 0; i < model->size(); i++) {
         if ((*model)[i].visibility == 1 &&
             (*model)[i].chain == 1) {
            use_model.push_back(&(*model)[i]);
         }
      }
      for (unsigned int i = 0; i < model->size(); i++) {
         if ((*model)[i].visibility == 1 &&
             (*model)[i].chain == 0) {
            use_model.push_back(&(*model)[i]);
         }
      }
      for (unsigned int i = 0; i < model->size(); i++) {
         if ((*model)[i].visibility == 0 &&
             (*model)[i].chain == 1) {
            use_model.push_back(&(*model)[i]);
         }
      }
      for (unsigned int i = 0; i < model->size(); i++) {
         if ((*model)[i].visibility == 0 &&
             (*model)[i].chain == 0) {
            use_model.push_back(&(*model)[i]);
         }
      }

   default :
      break;
   }

   FILE *fsomo = (FILE *)0;
   FILE *fbeams = (FILE *)0;
   FILE *frmc = (FILE *)0;
   FILE *frmc1 = (FILE *)0;
   FILE *fhydro = (FILE *)0;

   if (bead_output.output & US_HYDRODYN_OUTPUT_SOMO) {
      fsomo = fopen(QString("%1.bead_model").arg(fname).ascii(), "w");
   }
   if (bead_output.output & US_HYDRODYN_OUTPUT_BEAMS) {
      fbeams = fopen(QString("%1.beams").arg(fname).ascii(), "w");
      frmc = fopen(QString("%1.rmc").arg(fname).ascii(), "w");
      frmc1 = fopen(QString("%1.rmc1").arg(fname).ascii(), "w");
   }
   if (bead_output.output & US_HYDRODYN_OUTPUT_HYDRO) {
      fhydro = fopen(QString("%1.dat").arg(fname).ascii(), "w");
   }

   int beads = 0;

   for (unsigned int i = 0; i < use_model.size(); i++) {
      if (use_model[i]->active) {
         beads++;
      }
   }

   if (fsomo) {
      fprintf(fsomo,
              "%d\t%.3f\n",
              beads,
              results.vbar
              );
   }

   if (fbeams) {
      fprintf(fbeams,
              "%d\t-2.000000\t%s.rmc\t%.3f\n",
              beads,
              QFileInfo(fname).fileName().ascii(),
              results.vbar
              );
   }

   if (fhydro) {
      fprintf(fhydro,
              "1.E%d,\t!Unit of length for coordinates and radii, cm\n"
              "%d,\t!Number of beads\n",
              hydro.unit + 2,
              beads
              );
   }

   for (unsigned int i = 0; i < use_model.size(); i++) {
      if (use_model[i]->active) {
         QString tmp_serial = use_model[i]->resSeq; // was serial
         QString residues;

         if (!bead_model_from_file) {
            residues =
               use_model[i]->resName +
               (use_model[i]->org_chain ? ".SC." : (regular_N_handling ? ".PB." : ".MC.")) +
               (use_model[i]->chainID == " " ? "" : (use_model[i]->chainID + "."));
            // a compiler error forced this kludge using tmp_serial
            //   + QString("%1").arg((*use_model)[i].serial);
            residues += QString("%1").arg(tmp_serial);

            for (unsigned int j = 0; j < use_model[i]->all_beads.size(); j++)
            {
               QString tmp_serial = use_model[i]->all_beads[j]->resSeq;

               residues += "," +
                  (use_model[i]->all_beads[j]->resName +
                   (use_model[i]->all_beads[j]->org_chain ? ".SC." : (regular_N_handling ? ".PB." : ".MC.")) +
                   (use_model[i]->all_beads[j]->chainID == " " ? "" : (use_model[i]->all_beads[j]->chainID + ".")));
               // a compiler error forced this kludge using tmp_serial
               //  + QString("%1").arg((*use_model)[i].all_beads[j].serial);
               residues += QString("%1").arg(tmp_serial);
            }
         }
         else 
         {
            residues = use_model[i]->residue_list;
         }

         if (fsomo) {
            fprintf(fsomo,
                    fstring_somo.ascii(),
                    use_model[i]->bead_coordinate.axis[0],
                    use_model[i]->bead_coordinate.axis[1],
                    use_model[i]->bead_coordinate.axis[2],
                    use_model[i]->bead_computed_radius,
                    use_model[i]->bead_ref_mw,
                    get_color(use_model[i]),
                    residues.ascii(),
                    use_model[i]->bead_recheck_asa
                    );
         }
         if (fbeams) {
            fprintf(fbeams,
                    fstring_beams.ascii(),
                    use_model[i]->bead_coordinate.axis[0],
                    use_model[i]->bead_coordinate.axis[1],
                    use_model[i]->bead_coordinate.axis[2]
                    );
            fprintf(frmc,
                    fstring_rmc.ascii(),
                    use_model[i]->bead_computed_radius,
                    use_model[i]->bead_ref_mw,
                    get_color(use_model[i]));
            fprintf(frmc1,
                    fstring_rmc1.ascii(),
                    use_model[i]->bead_computed_radius,
                    use_model[i]->bead_ref_mw,
                    get_color(use_model[i]),
                    residues.ascii()
                    );
         }
         if (fhydro) {
            fprintf(fhydro,
                    fstring_hydro.ascii(),
                    use_model[i]->bead_coordinate.axis[0],
                    use_model[i]->bead_coordinate.axis[1],
                    use_model[i]->bead_coordinate.axis[2],
                    use_model[i]->bead_computed_radius);
         }
      }
   }
   if (fsomo) {
      fprintf(fsomo,
              "\n"
              "Bead Model Output:\n"
              "  Sequence:                   %s\n"
              "\n"

              ,bead_output.sequence ?
              (bead_output.sequence == 1 ?
               "exposed sidechain -> exposed main chain -> buried" :
               "include bead-original residue correspondence") :
              "as in original PDB file"
              );

      fprintf(fsomo, options_log.ascii());
      fclose(fsomo);
   }
   if (fbeams) {
      fclose(fbeams);
      fclose(frmc);
      fclose(frmc1);

   }
   if (fhydro) {
      fclose(fhydro);
   }
}

void US_Hydrodyn::bead_check()
{
   // recheck beads here

   puts("bead recheck");
   active_atoms.clear();
   for(unsigned int i = 0; i < bead_model.size(); i++) {
      active_atoms.push_back(&bead_model[i]);
   }

   QString error_string = "";
   int retval = us_hydrodyn_asab1_main(active_atoms,
                                       &asa,
                                       &results,
                                       true,
                                       progress,
                                       editor,
                                       this
                                       );
   if (stopFlag)
   {
      return;
   }
   if ( retval ) {
      switch ( retval ) {
      case US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC:
         {
            printError("US_HYDRODYN_ASAB1 on bead recheck encountered a memory allocation error");
            fprintf(stderr, "bead recheck: memory alloc error\n");
            return;
            break;
         }
      default:
         {
            // unknown error
            printError("US_HYDRODYN_ASAB1 encountered an unknown error");
            fprintf(stderr, "bead recheck: unknown error %d\n", retval);
            return;
            break;
         }
      }
   }

   int b2e = 0;
#if defined(EXPOSED_TO_BURIED)
   int e2b = 0;
#endif

   //  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) + "_pre_recheck" + DOTSOMO, &bead_model);
   //  write_bead_model(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) + "_pre_recheck" + DOTSOMO, &bead_model);

   for (unsigned int i = 0; i < bead_model.size(); i++)
   {
      float surface_area =
         (asa.probe_radius + bead_model[i].bead_computed_radius) *
         (asa.probe_radius + bead_model[i].bead_computed_radius) * 4 * M_PI;
      QString msg = "";
      if(bead_model[i].bead_recheck_asa > (asa.threshold_percent / 100.0) * surface_area) {
         // now exposed
         if(bead_model[i].exposed_code != 1) {
            // was buried
            msg = "buried->exposed";
            b2e++;
            bead_model[i].exposed_code = 1;
            bead_model[i].bead_color = 8;
         }
      }
#if defined(EXPOSED_TO_BURIED)
      else {
         // now buried
         if(bead_model[i].exposed_code == 1) {
            // was exposed
            msg = "exposed->buried";
            e2b++;
            bead_model[i].exposed_code = 6;
            bead_model[i].bead_color = 6;
         }
      }
#endif

      printf("bead %d %.2f %.2f %.2f %s %s\n",
             i,
             bead_model[i].bead_computed_radius,
             surface_area,
             bead_model[i].bead_recheck_asa,
             (bead_model[i].bead_recheck_asa >
              (asa.threshold_percent / 100.0) * surface_area) ?
             "exposed" : "buried",
             msg.ascii());
   }
   //  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
   //         QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
   //       DOTSOMO, &bead_model);
   write_bead_model(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
                    QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
                    DOTSOMO, &bead_model);
#if defined(EXPOSED_TO_BURIED)
   editor->append(QString("%1 exposed beads became buried\n").arg(e2b));
#endif
   editor->append(QString("%1 buried beads became exposed\n").arg(b2e));
}

void US_Hydrodyn::select_model(int val)
{
   current_model = val;
   QString msg = QString("\n\n%1 models selected:").arg(project);
   for(int i = 0; i < lb_model->numRows(); i++) {
      if (lb_model->isSelected(i)) {
         current_model = i;
         msg += QString(" %1").arg(i+1);
      }
   }
   msg += "\n";
   editor->setText(msg);

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

int US_Hydrodyn::calc_grid()
{
   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   append_options_log_atob();
   int flag = 0;
   bool any_errors = false;
   bool any_models = false;
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

   if(!bead_model_prefix.contains("a2b"))
   {
      if(bead_model_prefix.length()) {
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
         if (somo_processed.size() > current_model && somo_processed[current_model]) {
            printf("in calc_grid: somo_processed %d\n", current_model); fflush(stdout);
            editor->append(QString("Gridding bead model %1\n").arg(current_model + 1));
            qApp->processEvents();
            bead_models[current_model] =
               us_hydrodyn_grid_atob(&bead_models[current_model],
                                     &grid,
                                     progress,
                                     editor,
                                     this);
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid->setEnabled(true);
               pb_somo->setEnabled(true);
               progress->reset();
               return -1;
            }
            bead_model = bead_models[current_model];
            any_models = true;
            somo_processed[current_model] = 1;
            if (grid_overlap.remove_overlap)
            {
               radial_reduction();
               if (stopFlag)
               {
                  editor->append("Stopped by user\n\n");
                  pb_grid->setEnabled(true);
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
                  pb_grid->setEnabled(true);
                  pb_somo->setEnabled(true);
                  progress->reset();
                  return -1;
               }
               for(unsigned int i = 0; i < bead_model.size(); i++) {
                  bead_model[i].exposed_code = 6;
                  bead_model[i].bead_color = 6;
               }
               bead_check();
               if (stopFlag)
               {
                  editor->append("Stopped by user\n\n");
                  pb_grid->setEnabled(true);
                  pb_somo->setEnabled(true);
                  progress->reset();
                  return -1;
               }
               bead_models[current_model] = bead_model;
            }
            // write_bead_spt(somo_dir + SLASH + project +
            //        (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
            //        QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
            //           DOTSOMO, &bead_model, bead_model_from_file);
            write_bead_model(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
                             QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
                             DOTSOMO, &bead_model);
         }
         else
         {
            QString error_string;
            printf("in calc_grid: !somo_processed %d\n", current_model); fflush(stdout);
            // create bead model from atoms
            editor->append(QString("Gridding atom model %1\n").arg(current_model + 1));
            qApp->processEvents();
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid->setEnabled(true);
               pb_somo->setEnabled(true);
               progress->reset();
               return -1;
            }
            int retval = create_beads(&error_string);
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid->setEnabled(true);
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
                           this_atom->bead_actual_radius = this_atom->bead_computed_radius;
                           this_atom->bead_mw = this_atom->mw;
                           bead_model.push_back(*this_atom);
                        }
                     }
                  }
                  if (bead_models.size() < current_model + 1) {
                     bead_models.resize(current_model + 1);
                  }

                  bead_models[current_model] =
                     us_hydrodyn_grid_atob(&bead_model,
                                           &grid,
                                           progress,
                                           editor,
                                           this);
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid->setEnabled(true);
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
                  printf("back from grid_atob 1\n"); fflush(stdout);
                  if (grid_overlap.remove_overlap)
                  {
                     radial_reduction();
                     bead_models[current_model] = bead_model;
                  }
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid->setEnabled(true);
                     pb_somo->setEnabled(true);
                     progress->reset();
                     return -1;
                  }
                  if (asa.recheck_beads)
                  {
                     editor->append("Rechecking beads\n");
                     qApp->processEvents();
                     for(unsigned int i = 0; i < bead_model.size(); i++) {
                        bead_model[i].exposed_code = 6;
                        bead_model[i].bead_color = 6;
                     }
                     bead_check();
                     bead_models[current_model] = bead_model;
                  }
                  if (stopFlag)
                  {
                     editor->append("Stopped by user\n\n");
                     pb_grid->setEnabled(true);
                     pb_somo->setEnabled(true);
                     progress->reset();
                     return -1;
                  }

                  // write_bead_spt(somo_dir + SLASH + project +
                  //       (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
                  //       QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
                  //       DOTSOMO, &bead_model, bead_model_from_file);
                  write_bead_model(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
                                   QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
                                   DOTSOMO, &bead_model);

               }
            }
         }
      }
   }

   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_grid->setEnabled(true);
      pb_somo->setEnabled(true);
      progress->reset();
      return -1;
   }

   if (any_models && !any_errors)
   {
      editor->append("Build bead model completed\n\n");
      qApp->processEvents();
      pb_visualize->setEnabled(true);
      pb_calc_hydro->setEnabled(true);
   }
   else
   {
      editor->append("Errors encountered\n\n");
   }

   pb_grid->setEnabled(true);
   pb_somo->setEnabled(true);
   pb_stop_calc->setEnabled(false);
   if (calcAutoHydro)
   {
      calc_hydro();
   }

   return (flag);
}

int US_Hydrodyn::calc_somo()
{
   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_somo->setEnabled(false);
   pb_grid->setEnabled(false);
   options_log = "";
   append_options_log_somo();

   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_somo->setEnabled(true);
      pb_grid->setEnabled(true);
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
   QString msg = QString("\n\n%1 models selected:").arg(project);
   for(int i = 0; i < lb_model->numRows(); i++) {
      somo_processed[i] = 0;
      if (lb_model->isSelected(i)) {
         current_model = i;
         msg += QString(" %1").arg(i+1);
      }
   }
   msg += "\n";
   editor->setText(msg);

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

               bead_check();
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
         pb_grid->setEnabled(true);
         progress->reset();
         return -1;
      }

      // calculate bead model and generate hydrodynamics calculation output
      // if successful, enable follow-on buttons:
   }
   if (any_models && !any_errors)
   {
      editor->append("Build bead model completed\n\n");
      qApp->processEvents();
      pb_visualize->setEnabled(true);
      pb_calc_hydro->setEnabled(true);
   }
   else
   {
      editor->append("Errors encountered\n\n");
   }

   pb_somo->setEnabled(true);
   pb_grid->setEnabled(true);
   pb_stop_calc->setEnabled(false);
   if (calcAutoHydro)
   {
      calc_hydro();
   }
   return 0;
}

void US_Hydrodyn::calc_hydro()
{
   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_calc_hydro->setEnabled(false);
   puts("calc hydro (supc)");
   editor->append("Begin hydrodynamic calculations\n\n");
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
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
      return;
   }

   printf("back from supc retval %d\n", retval);
   pb_show_hydro_results->setEnabled(retval ? false : true);
   pb_calc_hydro->setEnabled(true);
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
   editor->append("Calculate hydrodynamics completed\n\n");
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

void US_Hydrodyn::cancel()
{
   close();
}

void US_Hydrodyn::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo.html");
}

void US_Hydrodyn::atom()
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

void US_Hydrodyn::update_bead_model_file(const QString &str)
{
   bead_model_file = str;
}

void US_Hydrodyn::update_bead_model_prefix(const QString &str)
{
   bead_model_prefix = str;
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
   lbl_table->setText( QDir::convertSeparators( residue_filename ) );
}

void US_Hydrodyn::read_residue_file()
{
   QString str1, str2;
   unsigned int numatoms, numbeads, i, j, positioner;
   QFile f(residue_filename);
   int error_count = 0;
   int line_count = 1;
   QString error_msg = tr("Residue file errors:\n");
   QString error_text = tr("Residue file errors:\n");
   cout << "residue file name: " << residue_filename << endl;
   residue_list.clear();
   multi_residue_map.clear();
   new_residues.clear();
   map < QString, int > dup_residue_map;
   i=1;
   if (f.open(IO_ReadOnly|IO_Translate))
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         new_residue.comment = ts.readLine();
         line_count++;
         ts >> new_residue.name;
         ts >> new_residue.type;
         ts >> new_residue.molvol;
         ts >> new_residue.asa;
         ts >> numatoms;
         ts >> numbeads;
         ts >> new_residue.vbar;
         ts.readLine(); // read rest of line
         line_count++;
         new_residue.r_atom.clear();
         new_residue.r_bead.clear();
         for (j=0; j<numatoms; j++)
         {
            ts >> new_atom.name;
            ts >> new_atom.hybrid.name;
            ts >> new_atom.hybrid.mw;
            ts >> new_atom.hybrid.radius;
            ts >> new_atom.bead_assignment;
            if (new_atom.bead_assignment >= numbeads) 
            {
               error_count++;
               QString tmp_msg = 
                  tr(QString(
                             "\nThe atom's bead assignment has exceeded the number of beads.\n"
                             "For residue: %1 and Atom: %2 "
                             "on line %3 of the residue file.\n")
                     .arg(new_residue.comment)
                     .arg(new_atom.name)
                     .arg(line_count)
                     );
               error_text += tmp_msg;
               if (error_count < 5) 
               {
                  error_msg += tmp_msg;
               }
               else 
               {
                  if (error_count == 5)
                  {
                     error_msg += "Further errors not listed\n";
                  }
               }
            }
            ts >> positioner;
            if(positioner == 0)
            {
               new_atom.positioner = false;
            }
            else
            {
               new_atom.positioner = true;
            }
            ts >> new_atom.serial_number;
            str2 = ts.readLine(); // read rest of line
            line_count++;
            if (!new_atom.name.isEmpty() && new_atom.hybrid.radius > 0.0 && new_atom.hybrid.mw > 0.0)
            {
               new_residue.r_atom.push_back(new_atom);
            }
            else
            {
               QMessageBox::warning(this, tr("UltraScan Warning"),
                                    tr("Please note:\n\nThere was an error reading\nthe selected Residue File!\n\nAtom "
                                       + new_atom.name + " cannot be read and will be deleted from List."),
                                    QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            }
         }
         for (j=0; j<numbeads; j++)
         {
            ts >> new_bead.hydration;
            ts >> new_bead.color;
            ts >> new_bead.placing_method;
            ts >> new_bead.chain;
            ts >> new_bead.volume;
            str2 = ts.readLine(); // read rest of line
            line_count++;
            printf("residue name %s loading bead %d placing method %d\n",
                   new_residue.name.ascii(),
                   j, new_bead.placing_method); fflush(stdout);
            new_residue.r_bead.push_back(new_bead);
         }
         calc_bead_mw(&new_residue);
         if ( !new_residue.name.isEmpty()
              && new_residue.molvol > 0.0
              && new_residue.asa > 0.0)
         {
            new_residue.unique_name = QString("%1%2")
               .arg(new_residue.name)
               .arg(dup_residue_map[new_residue.name] ?
                    QString("_%1").arg(dup_residue_map[new_residue.name]) : "");
            dup_residue_map[new_residue.name]++;
            printf("residue name %s unique name %s\n"
                   ,new_residue.name.ascii()
                   ,new_residue.unique_name.ascii()
                   ); fflush(stdout);
            multi_residue_map[new_residue.name].push_back(residue_list.size());
            residue_list.push_back(new_residue);
            for (unsigned int k = 0; k < new_residue.r_atom.size(); k++) {
               QString idx = QString("%1|%2|%3")
                  .arg(new_residue.name)
                  .arg(new_residue.r_atom[k].name)
                  .arg(multi_residue_map[new_residue.name].size() - 1);
               valid_atom_map[idx].push_back(k);
            }
         }
      }
      f.close();
   }
#if defined(DEBUG_MULTI_RESIDUE)
   printf("map index:\n");
   for (map < QString, vector < int > >::iterator it = multi_residue_map.begin();
        it != multi_residue_map.end();
        it++)
   {
      for (unsigned int i = 0; i < it->second.size(); i++)
      {
         printf("residue %s map pos %u\n",
                it->first.ascii(), it->second[i]);
      }
   }
#endif
   if (error_count)
   {
      QMessageBox::message(tr("ERRORS:"), error_msg);
      if (editor) 
      {
         editor->append(error_text);
      }
   } 
   save_residue_list = residue_list;
   save_multi_residue_map = multi_residue_map;
}

void US_Hydrodyn::load_pdb()
{
   QString message = "";
   if (pdb_parse.missing_residues == 1)
   {
      message += tr("You have selected to skip missing residues. If your model contains missing\n"
                    "residues, the calculated molecular weight and vbar may be incorrect, and\n"
                    "you should manually enter a global value for the molecular weight in the\n"
                    "SOMO hydrodynamic options, and a global value for the vbar in the SOMO\n"
                    "Miscellaneous options.\n\nAre you sure you want to proceed?");
   }
   if (pdb_parse.missing_residues == 2)
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
   if (pdb_parse.missing_atoms == 1)
   {
      message += tr("You have selected to skip coded residues containing missing atoms.\n"
                    "If your model contains missing atoms, the calculated molecular\n"
                    "weight and vbar may be incorrect, and you should manually enter\n"
                    "a global value for the molecular weight in the SOMO hydrodynamic\n"
                    "options, and a global value for the vbar in the SOMO Miscellaneous\n"
                    "options.\n\nAre you sure you want to proceed?");
   }
   if (pdb_parse.missing_atoms == 2)
   {
      message += tr("You have selected to replace coded residues containing missing atoms\n"
                    "with an average residue. If your model contains coded residues with\n"
                    "missing atoms, the calculated molecular weight and vbar may be incorrect.\n"
                    "Therefore, you could manually enter a global value for the molecular weight\n"
                    "in the SOMO hydrodynamic options, and a global value for the vbar in the\n"
                    "SOMO Miscellaneous options. You can also review the average residue settings\n"
                    "in the SOMO Miscellaneous options.\n\n"
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
   cout << somo_pdb_dir << endl;
   QString filename = QFileDialog::getOpenFileName(somo_pdb_dir,
                                                   "Structures (*.pdb *.PDB)",
                                                   this,
                                                   "Open Structure Files",
                                                   "Please select a PDB file...");
   if (!filename.isEmpty())
   {
      options_log = "";
      bead_model_from_file = false;
      int errors_found = 0;
      lbl_pdb_file->setText( QDir::convertSeparators( filename ) );
      editor->setText("\n\n");

#if defined(START_RASMOL)
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
      argument.append(QFileInfo(filename).fileName());
      rasmol->setWorkingDirectory(QFileInfo(filename).dirPath());
      rasmol->setArguments(argument);
      if (!rasmol->start())
      {
         QMessageBox::message(tr("Please note:"), tr("There was a problem starting RASMOL\n"
                                                     "Please check to make sure RASMOL is properly installed..."));
      }
#endif
      QFileInfo fi(filename);
      project = fi.baseName();
      new_residues.clear();
      residue_list = save_residue_list;
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
   pb_grid->setEnabled(true);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_visualize->setEnabled(false);
   le_bead_model_file->setText(" not selected ");
}

void US_Hydrodyn::view_pdb()
{
   QString filename = QFileDialog::getOpenFileName(somo_pdb_dir, "*.pdb *.PDB", this);
   if (!filename.isEmpty())
   {
      view_file(filename);
   }
}

void US_Hydrodyn::view_file(const QString &filename)
{
   US_Editor *edit;
   edit = new US_Editor(1);
   edit->setFont(QFont("Courier"));
   edit->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   edit->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
   edit->load(filename);
   edit->show();
}

void US_Hydrodyn::view_asa()
{
   QString filename = QFileDialog::getOpenFileName(somo_dir, "*.asa_res *.ASA_RES", this);
   if (!filename.isEmpty())
   {
      view_file(filename);
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
      }
      // bead_model_prefix = "";
   }
}

void US_Hydrodyn::read_pdb(const QString &filename)
{
   lb_model->clear();
   QString str, str1, str2, temp;
   model_vector.clear();
   bead_model.clear();
   QString last_resSeq = ""; // keeps track of residue sequence number, initialize to zero, first real one will be "1"
   struct PDB_chain temp_chain;
   QFile f(filename);
   struct PDB_model temp_model;
   bool chain_flag = false;
   bool model_flag = false;
   temp_model.molecule.clear();
   temp_model.residue.clear();
   clear_temp_chain(&temp_chain);
   if (f.open(IO_ReadOnly))
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         str1 = ts.readLine();
         if (str1.left(6) == "HEADER")
         {
            QString tmp_str = str1.mid(10,62);
            tmp_str.replace(QRegExp("\\s+")," ");
            editor->append(QString("Reading pdb: %1\n").arg(tmp_str));
         }
         if (str1.left(5) == "MODEL") // we have a new model in a multi-model file
         {
            model_flag = true; // we are using model descriptions (possibly multiple models)
            str2 = str1.mid(6, 15);
            temp_model.model_id = str2.toUInt();
            chain_flag = false; // we are starting a new molecule
            temp_model.molecule.clear();
            temp_model.residue.clear();
            clear_temp_chain(&temp_chain);
         }
         if (str1.left(6) == "ENDMDL") // we need to save the previously recorded molecule
         {
            temp_model.molecule.push_back(temp_chain); // add the last chain of this model
            editor->append("\nResidue sequence from " + project +".pdb model " +
                           QString("%1").arg(model_vector.size() + 1) + ": \n");
            str = "";
            // the residue list is wrong if there are unknown residues
            for ( unsigned int i = 0; i < temp_model.molecule.size(); i++ )
            {
               QString lastResSeq = "";
               for ( unsigned int j = 0; j < temp_model.molecule[i].atom.size(); j++ )
               {
                  if ( temp_model.molecule[i].atom[j].resSeq != lastResSeq )
                  {
                     str += temp_model.molecule[i].atom[j].resName + " ";
                     lastResSeq = temp_model.molecule[i].atom[j].resSeq;
                  }
               }
            }
            // for (unsigned int m=0; m<temp_model.residue.size(); m++ )
            // {
            //   str += temp_model.residue[m].name + " ";
            // }
            editor->append(str);
            // calc_vbar is wrong if there unknown residues, fixed later in check_for_missing_atoms()
            calc_vbar(&temp_model); // update the calculated vbar for this model
            model_vector.push_back(temp_model); // save the model in the model vector.
            clear_temp_chain(&temp_chain); // we are done with this molecule and can delete it
         }
         if (str1.left(4) == "ATOM" || str1.left(6) == "HETATM") // need to add TER
         {
            if(str1.mid(12,1) != "H" && str1.mid(13,1) != "H" &&
               str1.mid(17,3) != "HOH")
            {
               if (str1.mid(16,1) == " " || str1.mid(16,1) == "A")
               {
                  if (str1.mid(16,1) == "A")
                  {
                     editor->append(QString("Atom %1 conformation A selected").arg(str1.mid(6,5)));
                  }
                  if (!chain_flag)    // at the first time we encounter the word ATOM
                  {             // we don't have a chain yet, so let's start a new one
                     temp_chain.chainID = str1.mid(21, 1);
                     str2 = str1.mid(72, 4);
                     temp_chain.segID = str2.stripWhiteSpace();
                     chain_flag = true;
                  }
                  else // we have a chain, let's make sure the chain is still the same
                  {
                     if(temp_chain.chainID != str1.mid(21, 1)) // then we just started a new chain
                     {
                        temp_model.molecule.push_back(temp_chain);
                        clear_temp_chain(&temp_chain);
                        temp_chain.chainID = str1.mid(21, 1); // we have to start a new chain
                        str2 = str1.mid(72, 4);
                        temp_chain.segID = str2.stripWhiteSpace();
                     }
                  }
                  if (assign_atom(str1, &temp_chain, &last_resSeq)) // parse the current line and add it to temp_chain
                  { // if true, we have new residue and need to add it to the residue vector
                     temp_model.residue.push_back(current_residue); // add the next residue of this model
                  }
               }
            }
         }
      }
      f.close();
   }
   if(!model_flag)   // there were no model definitions, just a single molecule,
   {                  // we still need to save the results
      temp_model.molecule.push_back(temp_chain);
      editor->append("\nResidue sequence from " + project +".pdb:\n");
      str = "";
      // the residue list is wrong if there are unknown residues
      for ( unsigned int i = 0; i < temp_model.molecule.size(); i++ )
      {
         QString lastResSeq = "";
         for ( unsigned int j = 0; j < temp_model.molecule[i].atom.size(); j++ )
         {
            if ( temp_model.molecule[i].atom[j].resSeq != lastResSeq )
            {
               str += temp_model.molecule[i].atom[j].resName + " ";
               lastResSeq = temp_model.molecule[i].atom[j].resSeq;
            }
         }
      }
      // for (unsigned int m=0; m<temp_model.residue.size(); m++ )
      // {
      //   str += temp_model.residue[m].name + " ";
      // }
      editor->append(str);
      temp_model.model_id = 1;
      // calc_vbar is wrong if there unknown residues, fixed later in check_for_missing_atoms()
      calc_vbar(&temp_model); // update the calculated vbar for this model
      model_vector.push_back(temp_model);
      clear_temp_chain(&temp_chain);
   }
   for (unsigned int i=0; i<model_vector.size(); i++)
   {
      str1.sprintf("Model: %d", model_vector[i].model_id);
      lb_model->insertItem(str1);
   }
   lb_model->setEnabled(true);
   lb_model->setSelected(0, true);
   current_model = 0;
}

int US_Hydrodyn::read_bead_model(QString filename)
{
   lb_model->clear();
   lbl_pdb_file->setText(tr(" not selected "));
   project = filename;
   //   project.replace(QRegExp(".*(/|\\\\)"), "");
   //   project.replace(QRegExp("\\.(somo|SOMO)\\.(bead_model|BEAD_MODEL)$"), "");
   project = QFileInfo(QFileInfo(filename).fileName()).baseName();
   QString ftype = QFileInfo(filename).extension(false).lower();
   editor->setText("\n\nLoading bead model " + project + " of type " + ftype + "\n");
   bead_model.clear();
   PDB_atom tmp_atom;
   QFile f(filename);
   int bead_count;
   int linepos = 0;

   if (ftype == "bead_model")
   {
      if (f.open(IO_ReadOnly))
      {
         QTextStream ts(&f);
         if (!ts.atEnd()) {
            ts >> bead_count;
         }
         else 
         {
            editor->append("Error in line 1!\n");
            return 1;
         }
         if (!ts.atEnd()) {
            ts >> results.vbar;
         }
         else 
         {
            editor->append("Error in line 1!\n");
            return 1;
         }
         editor->append(QString("Beads %1\n").arg(bead_count));
         while (!ts.atEnd() && linepos < bead_count)
         {
            ++linepos;
            for (unsigned int i = 0; i < 3; i++)
            {
               if (!ts.atEnd()) {
                  ts >>  tmp_atom.bead_coordinate.axis[i];
               }
               else 
               {
                  editor->append(QString("\nError in line %1!\n").arg(linepos));
                  return linepos;
               }
            }
            if (!ts.atEnd()) {
               ts >>  tmp_atom.bead_computed_radius;
            }
            else 
            {
               editor->append(QString("\nError in line %1!\n").arg(linepos));
               return linepos;
            }
            if (!ts.atEnd()) {
               ts >>  tmp_atom.bead_mw;
               tmp_atom.bead_ref_mw = tmp_atom.bead_mw;
            }
            else 
            {
               editor->append(QString("\nError in line %1!\n").arg(linepos));
               return linepos;
            }
            if (!ts.atEnd()) {
               ts >>  tmp_atom.bead_color;
            }
            else 
            {
               editor->append(QString("\nError in line %1!\n").arg(linepos));
               return linepos;
            }
            if (!ts.atEnd()) {
               tmp_atom.serial = linepos;
            }
            else 
            {
               editor->append(QString("\nError in line %1!\n").arg(linepos));
               return linepos;
            }
            if (!ts.atEnd()) {
               ts >>  tmp_atom.residue_list;
            }
            else 
            {
               editor->append(QString("\nError in line %1!\n").arg(linepos));
               return linepos;
            }
            if (!ts.atEnd()) {
               ts >>  tmp_atom.bead_recheck_asa;
            }
            else 
            {
               editor->append(QString("\nError in line %1!\n").arg(linepos));
               return linepos;
            }
            tmp_atom.exposed_code = 1;
            tmp_atom.all_beads.clear();
            tmp_atom.active = true;
            tmp_atom.name = "ATOM";
            tmp_atom.resName = "RESIDUE";
            tmp_atom.iCode = "ICODE";
            tmp_atom.chainID = "CHAIN";
            bead_model.push_back(tmp_atom);
         }
         QFont save_font = editor->currentFont();
         QFont new_font = QFont("Courier");
         new_font.setStretch(75);
         editor->setCurrentFont(new_font);
         while (!ts.atEnd()) 
         {
            editor->append(ts.readLine());
         }
         editor->setCurrentFont(save_font);
         editor->append(QString("\nvbar: %1\n\n").arg(results.vbar));
         f.close();
         if (bead_count != (int)bead_model.size())
         {
            editor->append(QString("Error: bead count %1 does not match # of beads read from file (%2) \n").arg(bead_count).arg(bead_model.size()));
            return -1;
         }
         bead_model_from_file = true;
         editor->append("Bead model loaded\n\n");
         // write_bead_spt(somo_dir + SLASH + project +
         //          QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
         //          DOTSOMO, &bead_model, true);
         lb_model->clear();
         lb_model->insertItem("Model 1 from bead_model file");
         lb_model->setSelected(0, true);
         lb_model->setEnabled(false);
         model_vector.resize(1);
         model_vector[0].vbar = results.vbar;
         somo_processed.resize(lb_model->numRows());
         bead_models.resize(lb_model->numRows());
         current_model = 0;
         bead_models[0] = bead_model;
         somo_processed[0] = 1;
         return 0;
      }
   }

   if (ftype == "beams")
   {
      if (f.open(IO_ReadOnly))
      {
         QTextStream ts(&f);
         if (!ts.atEnd()) {
            ts >> bead_count;
         }
         else 
         {
            editor->append("Error in line 1!\n");
            return 1;
         }
         if (!ts.atEnd()) {
            ts >> results.vbar;
         }
         else 
         {
            editor->append("Error in line 1!\n");
            return 1;
         }
         QString rmcfile;
         if (!ts.atEnd()) {
            ts >> rmcfile;
         }
         else 
         {
            editor->append("Error in line 1!\n");
            return 1;
         }
         if (results.vbar == -2)
         {
            if (!ts.atEnd()) {
               ts >> results.vbar;
            }
            else 
            {
               editor->append("Error in line 1!\n");
               return 1;
            }
         }
         QFile frmc(QFileInfo(filename).dirPath() + SLASH + rmcfile);
         if (frmc.open(IO_ReadOnly))
         {
            QTextStream tsrmc(&frmc);

            editor->append(QString("Beads %1\n").arg(bead_count));
            while (!tsrmc.atEnd() && !ts.atEnd() && linepos < bead_count)
            {
               ++linepos;
               for (unsigned int i = 0; i < 3; i++)
               {
                  if (!ts.atEnd()) {
                     ts >>  tmp_atom.bead_coordinate.axis[i];
                  }
                  else 
                  {
                     editor->append(QString("\nError in line %1!\n").arg(linepos));
                     return linepos;
                  }
               }
               if (!tsrmc.atEnd()) {
                  tsrmc >>  tmp_atom.bead_computed_radius;
               }
               else 
               {
                  editor->append(QString("\nError in line %1!\n").arg(linepos));
                  return linepos;
               }
               if (!tsrmc.atEnd()) {
                  tsrmc >>  tmp_atom.bead_mw;
                  tmp_atom.bead_ref_mw = tmp_atom.bead_mw;
               }
               else 
               {
                  editor->append(QString("\nError in line %1!\n").arg(linepos));
                  return linepos;
               }
               if (!tsrmc.atEnd()) {
                  tsrmc >>  tmp_atom.bead_color;
               }
               else 
               {
                  editor->append(QString("\nError in line %1!\n").arg(linepos));
                  return linepos;
               }
               tmp_atom.serial = linepos;
               tmp_atom.exposed_code = 1;
               tmp_atom.all_beads.clear();
               tmp_atom.active = true;
               tmp_atom.name = "ATOM";
               tmp_atom.resName = "RESIDUE";
               tmp_atom.iCode = "ICODE";
               tmp_atom.chainID = "CHAIN";
               bead_model.push_back(tmp_atom);
            }
            frmc.close();
         }
         f.close();
         if (bead_count != (int)bead_model.size())
         {
            editor->append(QString("Error: bead count %1 does not match # of beads read from file (%2) \n").arg(bead_count).arg(bead_model.size()));
            return -1;
         }
         bead_model_from_file = true;
         f.close();
         editor->append(QString("\nvbar: %1\n\n").arg(results.vbar));
         editor->append("Bead model loaded\n\n");
         // write_bead_spt(somo_dir + SLASH + project +
         //          QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
         //          DOTSOMO, &bead_model, true);

         lb_model->clear();
         lb_model->insertItem("Model 1 from beams file");
         lb_model->setSelected(0, true);
         lb_model->setEnabled(false);
         model_vector.resize(1);
         model_vector[0].vbar = results.vbar;
         somo_processed.resize(lb_model->numRows());
         bead_models.resize(lb_model->numRows());
         current_model = 0;
         bead_models[0] = bead_model;
         somo_processed[0] = 1;

         return 0;
      }
   }

   editor->append("File read error\n");
   return -2;
}

void US_Hydrodyn::update_vbar()
{
   if (misc.compute_vbar && model_vector.size() > 0) // after reading the pdb file, the vbar is calculated.
   {// If we computed vbar, we assign this to result.vbar, which should be used in the calculation.
      results.vbar = model_vector[current_model].vbar;
   }
   else
   {
      results.vbar = misc.vbar;
   }
}

void US_Hydrodyn::calc_vbar(struct PDB_model *model)
{
   float mw_vbar_sum = 0.0;
   float mw_sum = 0.0;
   float mw;
   for (unsigned int i=0; i<(*model).residue.size(); i++)
   {
      mw = 0.0;
      for (unsigned int j=0; j<(*model).residue[i].r_atom.size(); j++)
      {
         mw += (*model).residue[i].r_atom[j].hybrid.mw;
      }
      mw_sum += mw;
      mw_vbar_sum += mw * (*model).residue[i].vbar;
   }
   (*model).vbar = (mw_vbar_sum/mw_sum); //  - 0.002125;
   //cout << "VBAR: " << (*model).vbar << endl;
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

void US_Hydrodyn::clear_display()
{
   editor->clear();
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

void US_Hydrodyn::clear_temp_chain(struct PDB_chain *temp_chain) // clear all the memory from the vectors in temp_chain
{
   (*temp_chain).atom.clear();
   (*temp_chain).chainID = "";
   (*temp_chain).segID = "";
}

bool US_Hydrodyn::assign_atom(const QString &str1, struct PDB_chain *temp_chain, QString *last_resSeq)
{
   /*
     http://www.rcsb.org/pdb/docs/format/pdbguide2.2/part_11.html

     ATOM record:
     COLUMNS        DATA TYPE       FIELD         DEFINITION
     ---------------------------------------------------------------------------------
     1 -  6        Record name     "ATOM  "
     7 - 11        Integer         serial        Atom serial number.
     13 - 16        Atom            name          Atom name. (sometimes starts at 12)
     17             Character       altLoc        Alternate location indicator.
     18 - 20        Residue name    resName       Residue name.
     22             Character       chainID       Chain identifier.
     23 - 26        Integer         resSeq        Residue sequence number.
     27             AChar           iCode         Code for insertion of residues.
     31 - 38        Real(8.3)       x             Orthogonal coordinates for X in Angstroms.
     39 - 46        Real(8.3)       y             Orthogonal coordinates for Y in Angstroms.
     47 - 54        Real(8.3)       z             Orthogonal coordinates for Z in Angstroms.
     55 - 60        Real(6.2)       occupancy     Occupancy.
     61 - 66        Real(6.2)       tempFactor    Temperature factor.
     73 - 76        LString(4)      segID         Segment identifier, left-justified.
     77 - 78        LString(2)      element       Element symbol, right-justified.
     79 - 80        LString(2)      charge        Charge on the atom.
   */
   QString str2;
   bool flag;
   struct PDB_atom temp_atom;
   str2 = str1.mid(6, 5);
   temp_atom.serial = str2.toUInt();

   str2 = str1.mid(11, 5);
   temp_atom.name = str2.stripWhiteSpace();

   temp_atom.altLoc = str1.mid(16, 1);

   str2 = str1.mid(17, 3);
   temp_atom.resName = str2.stripWhiteSpace();

   temp_atom.chainID = str1.mid(21, 1);

   temp_atom.resSeq = str1.mid(22, 5);
   temp_atom.resSeq.replace(QRegExp(" *"),"");
   if (temp_atom.resSeq == *last_resSeq)
   {
      flag = false;
   }
   else
   {
      flag = true;
   }
   if (temp_atom.resName == "HOH" || // we dont want to add waters to the sequence
       temp_atom.resName == "DOD")
   {
      flag = false;
   }
   *last_resSeq = temp_atom.resSeq; //update last residue sequence number
   temp_atom.iCode = str1.mid(26, 1);

   str2 = str1.mid(30, 8);
   temp_atom.coordinate.axis[0] = str2.toFloat();
   str2 = str1.mid(38, 8);
   temp_atom.coordinate.axis[1] = str2.toFloat();
   str2 = str1.mid(46, 8);
   temp_atom.coordinate.axis[2] = str2.toFloat();

   str2 = str1.mid(54, 6);
   temp_atom.occupancy = str2.toFloat();

   str2 = str1.mid(60, 6);
   temp_atom.tempFactor = str2.toFloat();

   temp_atom.element = str1.mid(76, 2);
   if (str1.length() >= 80)
   {
      temp_atom.charge = str1.mid(78, 2);
   }
   else
   {
      temp_atom.charge = "  ";
   }
   (*temp_chain).atom.push_back(temp_atom);
   bool found = false;
   for (unsigned int m = 0; m < residue_list.size(); m++)
   {
      if (temp_atom.resName == residue_list[m].name)
      {
         current_residue = residue_list[m];
         found = true;
         break;
      }
   }
   if (!found)
   {
      //   printError(tr("The residue " + temp_atom.resName + " listed in this PDB file is not found in the residue table!"));
   }

   return(flag);
}

void US_Hydrodyn::closeEvent(QCloseEvent *e)
{
   QMessageBox mb(tr("UltraScan"), tr("Attention:\nAre you sure you want to exit?"),
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
   global_Xpos -= 30;
   global_Ypos -= 30;
   write_config(USglobal->config_list.root_dir + "/etc/somo.config");
   // removing the temporary directory requires that the temporary directory is empty.
   // we don't know what else is in the directory if it was created previously.
   // tmp_dir.rmdir(USglobal->config_list.root_dir + "/tmp");
   if (rasmol != NULL)
   {
      if (rasmol->isRunning())
      {
         closeAttnt(rasmol, "RASMOL");
      }
   }
   e->accept();
}

void US_Hydrodyn::calc_bead_mw(struct residue *res)
{
   for (unsigned int i=0; i<(*res).r_bead.size(); i++)
   {
      (*res).r_bead[i].mw = 0.0;
      for (unsigned int j=0; j<(*res).r_atom.size(); j++)
      {
         if ((*res).r_atom[j].bead_assignment == i)
         {
            (*res).r_bead[i].mw += (*res).r_atom[j].hybrid.mw;
         }
      }
   }
}

void US_Hydrodyn::read_config(const QString& fname)
{
   QFile f;
   QString str;
   if (fname.isEmpty()) // this is true if we don't explicitely set a user-defined file name
   {
      f.setName(USglobal->config_list.root_dir + "/etc/somo.config");
   }
   else
   {
      f.setName(fname);
   }
   if (!f.open(IO_ReadOnly)) // first try user's directory for default settings
   {
      f.setName(USglobal->config_list.system_dir + "/etc/somo.config");
      if (!f.open(IO_ReadOnly)) // read system directory
      {
         reset(); // no file available, reset and return
         return;
      }
   }
   QTextStream ts(&f);
   ts.readLine(); // first line is comment
   ts >> str;
   ts.readLine();
   sidechain_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   sidechain_overlap.fuse_beads = (bool) str.toInt();
   ts >> sidechain_overlap.fuse_beads_percent;
   ts.readLine();
   ts >> str;
   ts.readLine();
   sidechain_overlap.remove_hierarch = (bool) str.toInt();
   ts >> sidechain_overlap.remove_hierarch_percent;
   ts.readLine();
   ts >> str;
   ts.readLine();
   sidechain_overlap.remove_sync = (bool) str.toInt();
   ts >> sidechain_overlap.remove_sync_percent;
   ts.readLine();
   ts >> str;
   ts.readLine();
   sidechain_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   sidechain_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   ts.readLine();
   mainchain_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   mainchain_overlap.fuse_beads = (bool) str.toInt();
   ts >> mainchain_overlap.fuse_beads_percent;
   ts.readLine();
   ts >> str;
   ts.readLine();
   mainchain_overlap.remove_hierarch = (bool) str.toInt();
   ts >> mainchain_overlap.remove_hierarch_percent;
   ts.readLine();
   ts >> str;
   ts.readLine();
   mainchain_overlap.remove_sync = (bool) str.toInt();
   ts >> mainchain_overlap.remove_sync_percent;
   ts.readLine();
   ts >> str;
   ts.readLine();
   mainchain_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   mainchain_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   ts.readLine();
   buried_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   buried_overlap.fuse_beads = (bool) str.toInt();
   ts >> buried_overlap.fuse_beads_percent;
   ts.readLine();
   ts >> str;
   ts.readLine();
   buried_overlap.remove_hierarch = (bool) str.toInt();
   ts >> buried_overlap.remove_hierarch_percent;
   ts.readLine();
   ts >> str;
   ts.readLine();
   buried_overlap.remove_sync = (bool) str.toInt();
   ts >> buried_overlap.remove_sync_percent;
   ts.readLine();
   ts >> str;
   ts.readLine();
   buried_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   buried_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   ts.readLine();
   grid_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   grid_overlap.fuse_beads = (bool) str.toInt();
   ts >> grid_overlap.fuse_beads_percent;
   ts.readLine();
   ts >> str;
   ts.readLine();
   grid_overlap.remove_hierarch = (bool) str.toInt();
   ts >> grid_overlap.remove_hierarch_percent;
   ts.readLine();
   ts >> str;
   ts.readLine();
   grid_overlap.remove_sync = (bool) str.toInt();
   ts >> grid_overlap.remove_sync_percent;
   ts.readLine();
   ts >> str;
   ts.readLine();
   grid_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   grid_overlap.show_translate = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   overlap_tolerance = str.toDouble();

   ts >> str;
   ts.readLine();
   bead_output.output = str.toInt();
   ts >> str;
   ts.readLine();
   bead_output.sequence = str.toInt();
   ts >> str;
   ts.readLine();
   bead_output.correspondence = (bool) str.toInt();

   ts >> str;
   ts.readLine();
   asa.probe_radius = str.toFloat();
   ts >> str;
   ts.readLine();
   asa.probe_recheck_radius = str.toFloat();
   ts >> str;
   ts.readLine();
   asa.threshold = str.toFloat();
   ts >> str;
   ts.readLine();
   asa.threshold_percent = str.toFloat();
   ts >> str;
   ts.readLine();
   asa.calculation = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   asa.recheck_beads = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   asa.method = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   asa.asab1_step = str.toFloat();

   ts >> str;
   ts.readLine();
   grid.cubic = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   grid.hydrate = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   grid.center = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   grid.tangency = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   grid.cube_side = str.toDouble();

   ts >> str;
   ts.readLine();
   misc.hydrovol = str.toDouble();
   ts >> str;
   ts.readLine();
   misc.compute_vbar = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   misc.vbar = str.toDouble();
   ts >> str;
   ts.readLine();
   misc.avg_radius = str.toDouble();
   ts >> str;
   ts.readLine();
   misc.avg_mass = str.toDouble();
   ts >> str;
   ts.readLine();
   misc.avg_hydration = str.toDouble();
   ts >> str;
   ts.readLine();
   misc.avg_volume = str.toDouble();
   ts >> str;
   ts.readLine();
   misc.avg_vbar = str.toDouble();


   ts >> str;
   ts.readLine();
   hydro.unit = str.toInt();
   ts >> str;
   ts.readLine();
   hydro.reference_system = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   hydro.boundary_cond = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   hydro.volume_correction = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   hydro.volume = str.toDouble();
   ts >> str;
   ts.readLine();
   hydro.mass_correction = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   hydro.mass = str.toDouble();
   ts >> str;
   ts.readLine();
   hydro.bead_inclusion = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   hydro.rotational = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   hydro.viscosity = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   hydro.overlap_cutoff = (bool) str.toInt();
   ts >> str;
   ts.readLine();
   hydro.overlap = str.toDouble();
   sidechain_overlap.title = "exposed side chain beads";
   mainchain_overlap.title = "exposed main/main and\nmain/side chain beads";
   buried_overlap.title = "buried beads";
   grid_overlap.title = "Grid beads";

   // pdb_visualization options:

   ts >> str; // visualization option
   ts.readLine();
   pdb_vis.visualization = str.toInt();
   ts >> str;
   ts.readLine();
   pdb_vis.filename = str; // custom Rasmol script file

   // pdb_parsing options:

   ts >> str; // skip hydrogens?
   ts.readLine();
   pdb_parse.skip_hydrogen = (bool) str.toInt();
   ts >> str; // skip water molecules?
   ts.readLine();
   pdb_parse.skip_water = (bool) str.toInt();
   ts >> str; // skip alternate conformations?
   ts.readLine();
   pdb_parse.alternate = (bool) str.toInt();
   ts >> str; // find free SH atoms?
   ts.readLine();
   pdb_parse.find_sh = (bool) str.toInt();
   ts >> str; // missing residue choice
   ts.readLine();
   pdb_parse.missing_residues = str.toInt();
   ts >> str; // missing atom choice
   ts.readLine();
   pdb_parse.missing_atoms = str.toInt();

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
   sidechain_overlap.remove_overlap = true;
   sidechain_overlap.fuse_beads = true;
   sidechain_overlap.fuse_beads_percent = 70.0;
   sidechain_overlap.remove_hierarch = true;
   sidechain_overlap.remove_hierarch_percent = 1.0;
   sidechain_overlap.remove_sync = false;
   sidechain_overlap.remove_sync_percent = 1.0;
   sidechain_overlap.translate_out = true;
   sidechain_overlap.show_translate = true;

   mainchain_overlap.remove_overlap = true;
   mainchain_overlap.fuse_beads = true;
   mainchain_overlap.fuse_beads_percent = 70.0;
   mainchain_overlap.remove_hierarch = true;
   mainchain_overlap.remove_hierarch_percent = 1.0;
   mainchain_overlap.remove_sync = false;
   mainchain_overlap.remove_sync_percent = 1.0;
   mainchain_overlap.translate_out = false;
   mainchain_overlap.show_translate = false;

   buried_overlap.remove_overlap = true;
   buried_overlap.fuse_beads = false;
   buried_overlap.fuse_beads_percent = 0.0;
   buried_overlap.remove_hierarch = true;
   buried_overlap.remove_hierarch_percent = 1.0;
   buried_overlap.remove_sync = false;
   buried_overlap.remove_sync_percent = 1.0;
   buried_overlap.translate_out = false;
   buried_overlap.show_translate = false;

   grid_overlap.remove_overlap = true;
   grid_overlap.fuse_beads = false;
   grid_overlap.fuse_beads_percent = 0.0;
   grid_overlap.remove_hierarch = true;
   grid_overlap.remove_hierarch_percent = 1.0;
   grid_overlap.remove_sync = false;
   grid_overlap.remove_sync_percent = 1.0;
   grid_overlap.translate_out = false;
   grid_overlap.show_translate = false;

   sidechain_overlap.title = "exposed side chain beads";
   mainchain_overlap.title = "exposed main/main and main/side chain beads";
   buried_overlap.title = "buried beads";
   grid_overlap.title = "Grid beads";

   bead_output.sequence = 0;
   bead_output.output = 1;
   bead_output.correspondence = true;
   
   asa.probe_radius = (float) 1.4;
   asa.probe_recheck_radius = (float) 1.4;
   asa.threshold = 20.0;
   asa.threshold_percent = 50.0;
   asa.calculation = true;
   asa.recheck_beads = true;
   asa.method = true; // by default use ASAB1
   asa.asab1_step = 1.0;

   grid.cubic = true;       // apply cubic grid
   grid.hydrate = false;    // true: hydrate model
   grid.center = false;    // true: center of cubelet, false: center of mass
   grid.tangency = false;   // true: Expand beads to tangency
   grid.cube_side = 10.0;

   misc.hydrovol = 24.041;
   misc.compute_vbar = true;
   misc.vbar = 0.72;
   misc.avg_radius = 1.68;
   misc.avg_mass = 16.0;
   misc.avg_hydration = 0.4;
   misc.avg_volume = 15.3;
   misc.avg_vbar = 0.72;
   overlap_tolerance = 0.001;

   hydro.unit = -10;                // exponent from units in meter (example: -10 = Angstrom, -9 = nanometers)
   hydro.reference_system = false;   // false: diffusion center, true: cartesian origin (default false)
   hydro.boundary_cond = false;      // false: stick, true: slip (default false)
   hydro.volume_correction = false;   // false: Automatic, true: manual (provide value)
   hydro.volume = 0.0;               // volume correction
   hydro.mass_correction = false;      // false: Automatic, true: manual (provide value)
   hydro.mass = 0.0;                  // mass correction
   hydro.bead_inclusion = false;      // false: exclude hidden beads; true: use all beads
   hydro.rotational = false;         // false: include beads in volume correction for rotational diffusion, true: exclude
   hydro.viscosity = false;            // false: include beads in volume correction for intrinsic viscosity, true: exclude
   hydro.overlap_cutoff = false;      // false: same as in model building, true: enter manually
   hydro.overlap = 0.0;               // overlap

   pdb_vis.visualization = 0;                // default RasMol colors
   pdb_vis.filename = USglobal->config_list.system_dir + "/etc/rasmol.spt"; //default color file
   
   pdb_parse.skip_hydrogen = true;
   pdb_parse.skip_water = true;
   pdb_parse.alternate = true;
   pdb_parse.find_sh = false;
   pdb_parse.missing_residues = 0;
   pdb_parse.missing_atoms = 0;
}

void US_Hydrodyn::write_config(const QString& fname)
{
   QFile f;
   QString str;
   f.setName(fname);
   cout << fname << endl;
   if (f.open(IO_WriteOnly | IO_Translate)) // first try user's directory for default settings
   {
      QTextStream ts(&f);
      ts << "SOMO Config file - computer generated, please do not edit...\n";
      ts << sidechain_overlap.remove_overlap << "\t\t# Remove side chain overlaps flag\n";
      ts << sidechain_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
      ts << sidechain_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
      ts << sidechain_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
      ts << sidechain_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
      ts << sidechain_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
      ts << sidechain_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
      ts << sidechain_overlap.translate_out << "\t\t# Outward translation flag\n";
      ts << sidechain_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

      ts << mainchain_overlap.remove_overlap << "\t\t# Remove mainchain overlaps flag\n";
      ts << mainchain_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
      ts << mainchain_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
      ts << mainchain_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
      ts << mainchain_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
      ts << mainchain_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
      ts << mainchain_overlap.remove_sync_percent << "\t\t# percent synchronously step\n";
      ts << mainchain_overlap.translate_out << "\t\t# Outward translation flag\n";
      ts << mainchain_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

      ts << buried_overlap.remove_overlap << "\t\t# Remove buried beads overlaps flag\n";
      ts << buried_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
      ts << buried_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
      ts << buried_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
      ts << buried_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
      ts << buried_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
      ts << buried_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
      ts << buried_overlap.translate_out << "\t\t# Outward translation flag\n";
      ts << buried_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

      ts << grid_overlap.remove_overlap << "\t\t# Remove grid bead overlaps flag\n";
      ts << grid_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
      ts << grid_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
      ts << grid_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
      ts << grid_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
      ts << grid_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
      ts << grid_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
      ts << grid_overlap.translate_out << "\t\t# Outward translation flag\n";
      ts << grid_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";
      ts << overlap_tolerance << "\t\t# bead overlap tolerance\n";

      ts << bead_output.output << "\t\t# flag for selecting output format\n";
      ts << bead_output.sequence << "\t\t# flag for selecting sequence format\n";
      ts << bead_output.correspondence << "\t\t# flag for residue correspondence (BEAMS only)\n";
      ts << asa.probe_radius << "\t\t# probe radius in angstrom\n";
      ts << asa.probe_recheck_radius << "\t\t# probe recheck radius in angstrom\n";
      ts << asa.threshold << "\t\t# ASA threshold\n";
      ts << asa.threshold_percent << "\t\t# ASA threshold percent\n";
      ts << asa.calculation << "\t\t# flag for calculation of ASA\n";
      ts << asa.recheck_beads << "\t\t# flag for rechecking beads\n";
      ts << asa.method << "\t\t# flag for ASAB1/Surfracer method\n";
      ts << asa.asab1_step << "\t\t# ASAB1 step size\n";

      ts << grid.cubic << "\t\t# flag to apply cubic grid\n";
      ts << grid.hydrate << "\t\t# flag to hydrate original model (grid)\n";
      ts << grid.center << "\t\t# flag for positioning bead in center of mass or cubelet (grid)\n";
      ts << grid.tangency << "\t\t# flag for expanding beads to tangency (grid)\n";
      ts << grid.cube_side << "\t\t# Length of cube side (grid)\n";

      ts << misc.hydrovol << "\t\t# hydration volume\n";
      ts << misc.compute_vbar << "\t\t# flag for selecting vbar calculation\n";
      ts << misc.vbar << "\t\t# vbar value\n";
      ts << misc.avg_radius << "\t\t# Average atomic radius value\n";
      ts << misc.avg_mass << "\t\t# Average atomic mass value\n";
      ts << misc.avg_hydration << "\t\t# Average atomic hydration value\n";
      ts << misc.avg_volume << "\t\t# Average bead/atom volume value\n";
      ts << misc.avg_vbar << "\t\t# Average vbar value\n";

      ts << hydro.unit << "\t\t# exponent from units in meter (example: -10 = Angstrom, -9 = nanometers)\n";
      ts << hydro.reference_system << "\t\t# flag for reference system\n";
      ts << hydro.boundary_cond << "\t\t# flag for boundary condition: false: stick, true: slip\n";
      ts << hydro.volume_correction << "\t\t# flag for volume correction - false: Automatic, true: manual\n";
      ts << hydro.volume << "\t\t# volume correction value for manual entry\n";
      ts << hydro.mass_correction << "\t\t# flag for mass correction: false: Automatic, true: manual\n";
      ts << hydro.mass << "\t\t# mass correction value for manual entry\n";
      ts << hydro.bead_inclusion << "\t\t# flag for bead inclusion in computation - false: exclude hidden beads; true: use all beads\n";
      ts << hydro.rotational << "\t\t# flag false: include beads in volume correction for rotational diffusion, true: exclude\n";
      ts << hydro.viscosity << "\t\t# flag false: include beads in volume correction for intrinsic viscosity, true: exclude\n";
      ts << hydro.overlap_cutoff << "\t\t# flag for overlap cutoff: false: same as in model building, true: enter manually\n";
      ts << hydro.overlap << "\t\t# overlap value\n";

      ts << pdb_vis.visualization << "\t\t# PDB visualization option\n";
      ts << pdb_vis.filename << "\t\t# RasMol color filename\n";

      ts << pdb_parse.skip_hydrogen << "\t\t# skip hydrogen atoms?\n";
      ts << pdb_parse.skip_water << "\t\t# skip water molecules?\n";
      ts << pdb_parse.alternate << "\t\t# skip alternate conformations?\n";
      ts << pdb_parse.find_sh << "\t\t# find SH groups?\n";
      ts << pdb_parse.missing_residues << "\t\t# how to handle missing residues\n";
      ts << pdb_parse.missing_atoms << "\t\t# how to handle missing atoms\n";

      f.close();
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
      asa_window = new US_Hydrodyn_ASA(&asa, &asa_widget);
      asa_window->show();
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
      pdb_parsing_window = new US_Hydrodyn_PDB_Parsing(&pdb_parse, &pdb_parsing_widget);
      pdb_parsing_window->show();
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
                                               &mainchain_overlap, &buried_overlap, &grid_overlap, &overlap_tolerance, &overlap_widget);
      overlap_window->show();
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
      bead_output_window = new US_Hydrodyn_Bead_Output(&bead_output, &bead_output_widget);
      bead_output_window->show();
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
      misc_window = new US_Hydrodyn_Misc(&misc, &misc_widget);
      connect(misc_window, SIGNAL(vbar_changed()), this, SLOT(update_vbar()));
      misc_window->show();
   }
   update_vbar();
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
      hydro_window = new US_Hydrodyn_Hydro(&hydro, &hydro_widget);
      hydro_window->show();
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
      grid_window = new US_Hydrodyn_Grid(&grid_overlap, &grid,
                                         &overlap_tolerance, &grid_widget);
      grid_window->show();
   }
}

void US_Hydrodyn::printError(const QString &str)
{
   QMessageBox::warning(this, tr("UltraScan Warning"), tr("Please note:\n\n") +
                        tr(str), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
}

void US_Hydrodyn::closeAttnt(QProcess *proc, QString message)
{
   switch( QMessageBox::information( this,
                                     tr("Attention!"),   message + tr(" is still running.\n"
                                                                      "Do you want to close it?"),
                                     tr("&Kill"), tr("&Close gracefully"), tr("Leave running"),
                                     0,      // Enter == button 0
                                     2 ) )   // Escape == button 2
   {
   case 0:
      {
         proc->kill();
         break;
      }
   case 1:
      {
         proc->tryTerminate();
         break;
      }
   case 2:
      {
         break;
      }
   }
}

void US_Hydrodyn::set_calcAutoHydro()
{
   calcAutoHydro = cb_calcAutoHydro->isChecked();
}

void US_Hydrodyn::append_options_log_somo()
{
   QString s;

   s.sprintf("Bead model built with the following options:\n");
   options_log += s;

   s.sprintf(
             "ASA Calculation:\n"
             "  Perform ASA Calculation:    %s\n"
             "  Recheck Bead ASA:           %s\n"
             "  ASA Method:                 %s\n"
             "  ASA Probe Radius (A):       %.2f\n"
             "  Probe Recheck Radius (A):   %.2f\n"
             "  ASA Threshold (A^2):        %.1f\n"
             "  Bead ASA Threshold %%:       %.1f\n"
             "  ASAB1 Step Size (A):        %.1f\n"
             "\n"

             ,asa.calculation ? "On" : "Off"
             ,asa.recheck_beads ? "On" : "Off"
             ,asa.method ? "Rolling Sphere" : "Voronoi Tesselation"
             ,asa.probe_radius
             ,asa.probe_recheck_radius
             ,asa.threshold
             ,asa.threshold_percent
             ,asa.asab1_step
             );
   options_log += s;

   s.sprintf(
             "Overlap Reduction:\n"
             "  Bead Overlap Tolerance:     %f\n"
             "    Exposed Side chain beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"
             "      Outward Translation:                        %s\n"

             "    Exposed Main and side chain beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"

             "    Buried beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"

             "\n"

             ,overlap_tolerance

             ,sidechain_overlap.fuse_beads ? "On" : "Off"
             ,sidechain_overlap.fuse_beads_percent
             ,sidechain_overlap.remove_overlap ? "On" : "Off"
             ,sidechain_overlap.remove_sync ? "On" : "Off"
             ,sidechain_overlap.remove_sync_percent
             ,sidechain_overlap.remove_hierarch ? "On" : "Off"
             ,sidechain_overlap.remove_hierarch_percent
             ,sidechain_overlap.translate_out ? "On" : "Off"

             ,mainchain_overlap.fuse_beads ? "On" : "Off"
             ,mainchain_overlap.fuse_beads_percent
             ,mainchain_overlap.remove_overlap ? "On" : "Off"
             ,mainchain_overlap.remove_sync ? "On" : "Off"
             ,mainchain_overlap.remove_sync_percent
             ,mainchain_overlap.remove_hierarch ? "On" : "Off"
             ,mainchain_overlap.remove_hierarch_percent

             ,buried_overlap.fuse_beads ? "On" : "Off"
             ,buried_overlap.fuse_beads_percent
             ,buried_overlap.remove_overlap ? "On" : "Off"
             ,buried_overlap.remove_sync ? "On" : "Off"
             ,buried_overlap.remove_sync_percent
             ,buried_overlap.remove_hierarch ? "On" : "Off"
             ,buried_overlap.remove_hierarch_percent

             );
   options_log += s;
}



void US_Hydrodyn::append_options_log_atob()
{
   QString s;

   s.sprintf("Grid model built with the following options:\n");
   options_log += s;

   s.sprintf(
             "Grid Functions (AtoB):\n"
             "  Computations Relative to:   %s\n"
             "  Cube Side (Angstrom):       %.1f\n"
             "  Apply Cubic Grid:           %s\n"
             "  Hydrate the Original Model: %s\n"
             "  Expand Beads to Tangency:   %s\n"
             "\n"

             "Overlap Reduction:\n"
             "  Bead Overlap Tolerance:     %f\n"

             "    Grid beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"
             "\n"

             ,grid.center ? "Center of Mass" : "Center of Cubelet"
             ,grid.cube_side
             ,grid.cubic ? "On" : "Off"
             ,grid.hydrate ? "On" : "Off"
             ,grid.tangency ? "On" : "Off"

             ,overlap_tolerance

             ,grid_overlap.fuse_beads ? "On" : "Off"
             ,grid_overlap.fuse_beads_percent
             ,grid_overlap.remove_overlap ? "On" : "Off"
             ,grid_overlap.remove_sync ? "On" : "Off"
             ,grid_overlap.remove_sync_percent
             ,grid_overlap.remove_hierarch ? "On" : "Off"
             ,grid_overlap.remove_hierarch_percent
             );

   options_log += s;
}

void US_Hydrodyn::stop_calc()
{
   stopFlag = true;
   pb_stop_calc->setEnabled(false);
}
