#include "../include/us_hydrodyn.h"
#include "../include/us_surfracer.h"
#include "../include/us_hydrodyn_supc.h"
#include "../include/us_hydrodyn_pat.h"
#include "../include/us_hydrodyn_asab1.h"
#include <qregexp.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifndef WIN32
	// #define DEBUG
        #include <unistd.h>
        // #define DEBUG1
	#define TIMING

	#if defined(TIMING)
		# include <sys/time.h>
		static struct timeval start_tv, end_tv;
	#endif
        #define SLASH "/"
	#define __open open
#else
	#define chdir _chdir
        #define dup2 _dup2
	#define __open _open
	#include <direct.h>
	#include <io.h>
        #define SLASH "\\"
        #define STDOUT_FILENO 1
        #define STDERR_FILENO 2
#endif
#define DOTSOMO		""
#define DOTSOMOCAP	""


US_Hydrodyn::US_Hydrodyn(QWidget *p, const char *name) : QFrame(p, name)
{

	USglobal = new US_Config();

	// int r_stdout = __open(QString(somo_tmp_dir +
	//			  SLASH + "last_stdout.txt").ascii(),
	//		  O_WRONLY | O_CREAT | O_TRUNC, 0666);
	// dup2(r_stdout, STDOUT_FILENO);

	// int r_stderr = __open(QString(somo_tmp_dir +
	//			  SLASH + "last_stderr.txt").ascii(),
	//		  O_WRONLY | O_CREAT | O_TRUNC, 0666);
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
	read_config();
	atom_widget = false;
	residue_widget = false;
	hybrid_widget = false;
	asa_widget = false;
	misc_widget = false;
	hydro_widget = false;
	overlap_widget = false;
	bead_output_widget = false;
	results_widget = false;
	residue_filename = USglobal->config_list.system_dir + "/etc/somo.residue";
	read_residue_file();
	setupGUI();
//	global_Xpos += 30;
//	global_Ypos += 30;
//	setGeometry(global_Xpos, global_Ypos, 0, 0);
	create_beads_normally = true;
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

	lbl_tabletabs = new QLabel(tr("Modify Lookup Tables:"), this);
	Q_CHECK_PTR(lbl_tabletabs);
	lbl_tabletabs->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_tabletabs->setAlignment(AlignCenter|AlignVCenter);
	lbl_tabletabs->setMinimumHeight(minHeight1);
	lbl_tabletabs->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_tabletabs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	pb_hybrid = new QPushButton(tr("Add/Edit Hybridization"), this);
	Q_CHECK_PTR(pb_hybrid);
	pb_hybrid->setMinimumHeight(minHeight1);
	pb_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_hybrid->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_hybrid, SIGNAL(clicked()), SLOT(hybrid()));

	pb_atom = new QPushButton(tr("Add/Edit Atom"), this);
	Q_CHECK_PTR(pb_atom);
	pb_atom->setMinimumHeight(minHeight1);
	pb_atom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_atom->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_atom, SIGNAL(clicked()), SLOT(atom()));

	pb_residue = new QPushButton(tr("Add/Edit Residue"), this);
	Q_CHECK_PTR(pb_residue);
	pb_residue->setMinimumHeight(minHeight1);
	pb_residue->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_residue->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_residue, SIGNAL(clicked()), SLOT(residue()));

	lbl_options = new QLabel(tr("Modify SOMO Options for:"), this);
	Q_CHECK_PTR(lbl_options);
	lbl_options->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_options->setAlignment(AlignCenter|AlignVCenter);
	lbl_options->setMinimumHeight(minHeight1);
	lbl_options->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_options->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	pb_show_asa = new QPushButton(tr("ASA Calculation"), this);
	Q_CHECK_PTR(pb_show_asa);
	pb_show_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_show_asa->setMinimumHeight(minHeight1);
	pb_show_asa->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_show_asa, SIGNAL(clicked()), SLOT(show_asa()));

	pb_show_overlap = new QPushButton(tr("Overlap Reduction"), this);
	Q_CHECK_PTR(pb_show_overlap);
	pb_show_overlap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_show_overlap->setMinimumHeight(minHeight1);
	pb_show_overlap->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_show_overlap, SIGNAL(clicked()), SLOT(show_overlap()));

	pb_show_hydro = new QPushButton(tr("Hydrodynamic Calculations"), this);
	Q_CHECK_PTR(pb_show_hydro);
	pb_show_hydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_show_hydro->setMinimumHeight(minHeight1);
	pb_show_hydro->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_show_hydro, SIGNAL(clicked()), SLOT(show_hydro()));

	pb_show_misc = new QPushButton(tr("Miscellaneous Options"), this);
	Q_CHECK_PTR(pb_show_misc);
	pb_show_misc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_show_misc->setMinimumHeight(minHeight1);
	pb_show_misc->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_show_misc, SIGNAL(clicked()), SLOT(show_misc()));

	pb_show_bead_output = new QPushButton(tr("Bead Model Output"), this);
	Q_CHECK_PTR(pb_show_bead_output);
	pb_show_bead_output->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_show_bead_output->setMinimumHeight(minHeight1);
	pb_show_bead_output->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_show_bead_output, SIGNAL(clicked()), SLOT(show_bead_output()));

	pb_reset = new QPushButton(tr("Reset to Default Options"), this);
	Q_CHECK_PTR(pb_reset);
	pb_reset->setMinimumHeight(minHeight1);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

	lbl_somo = new QLabel(tr("Run SOMO Program:"), this);
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

	pb_somo = new QPushButton(tr("Build Bead Model"), this);
	Q_CHECK_PTR(pb_somo);
	pb_somo->setMinimumHeight(minHeight1);
	pb_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_somo->setEnabled(false);
	pb_somo->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_somo, SIGNAL(clicked()), SLOT(calc_somo()));

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
	m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	QPopupMenu * file = new QPopupMenu(editor);
	m->insertItem( tr("&File"), file );
	file->insertItem( tr("Font"),  this, SLOT(update_font()),	 ALT+Key_F );
	file->insertItem( tr("Save"),  this, SLOT(save()),	 ALT+Key_S );
	file->insertItem( tr("Print"), this, SLOT(print()),	ALT+Key_P );
	file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),	ALT+Key_X );
	editor->setWordWrap (QTextEdit::WidgetWidth);

	int rows=7, columns = 3, spacing = 2, j=0, margin=4;
	QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

	background->addMultiCellWidget(lbl_tabletabs, j, j, 0, 1);
	background->addMultiCellWidget(editor, j, j+18, 2, 2);
	//background->addWidget(m, j, 2);
	j++;
	background->addWidget(pb_hybrid, j, 0);
	background->addWidget(pb_atom, j, 1);
//	background->addMultiCellWidget(editor, j, j+17, 2, 2);
	j++;
	background->addWidget(pb_residue, j, 0);
	j++;
	background->addMultiCellWidget(lbl_options, j, j, 0, 1);
	j++;
	background->addWidget(pb_show_asa, j, 0);
	background->addWidget(pb_show_overlap, j, 1);
	j++;
	background->addWidget(pb_show_hydro, j, 0);
	background->addWidget(pb_show_misc, j, 1);
	j++;
	background->addWidget(pb_show_bead_output, j, 0);
	background->addWidget(pb_reset, j, 1);
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
	j+=2;
	background->addWidget(pb_somo, j, 0);
	background->addWidget(pb_visualize, j, 1);
	j++;
	background->addWidget(pb_load_bead_model, j, 0);
	background->addWidget(le_bead_model_file, j, 1);
	j++;
	background->addWidget(lbl_bead_model_prefix, j, 0);
	background->addWidget(le_bead_model_prefix, j, 1);
	j++;
	background->addWidget(pb_calc_hydro, j, 0);
	background->addWidget(pb_show_hydro_results, j, 1);
	j++;
	background->addWidget(pb_help, j, 0);
	background->addWidget(pb_cancel, j, 1);
	j++;
	background->addMultiCellWidget(progress, j, j, 0, 1);
}

struct BPair {
  int i;
  int j;
  float separation;
  bool active;
};


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


int US_Hydrodyn::check_for_missing_atoms(QString *error_string, PDB_model *model)
{
  // go through molecules, build vector of residues
  // expand vector of residues to atom list from residue file
  // compare expanded list of residues to model ... list missing atoms missing
  int errors_found = 0;

  for (unsigned int j = 0; j < model->molecule.size(); j++)
  {
    unsigned int lastResSeq = 0;
    unsigned int lastResPos = 0;
    QString lastChainID = "";
    bool spec_N1 = false;
    for (unsigned int k = 0; k < model->molecule[j].atom.size(); k++)
    {
      PDB_atom *this_atom = &(model->molecule[j].atom[k]);
      this_atom->active = false;

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
	     this_atom->name != "OXT" &&
	     (k || this_atom->name != "N")) ||
	    (residue_list[m].name == "OXT"
	     && this_atom->name == "OXT") ||
	    (!k &&
	     this_atom->name == "N" &&
	     residue_list[m].name == "N1"))
	{
	  respos = (int) m;
	  this_atom->p_residue = &(residue_list[m]);
#if defined(DEBUG)
	  printf("residue match %d resName %s \n", m, residue_list[m].name.ascii());
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
		    residue_list[lastResPos].r_atom[l].name == "N") {
		  residue_list[lastResPos].r_atom[l].tmp_flag = true;
		  spec_N1 = false;
		}

		if (!residue_list[lastResPos].r_atom[l].tmp_flag)
		{
		  errors_found++;
		  error_string->
		    append(QString("").
			   sprintf("missing atom chain %s molecule %d atom %s residue %d %s\n",
				   lastChainID.ascii(),
				   j + 1,
				   residue_list[lastResPos].r_atom[l].name.ascii(),
				   lastResSeq,
				   residue_list[lastResPos].name.ascii()));
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
	    lastResSeq = 0;
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
	  errors_found++;
	  error_string->append(QString("").sprintf("unknown residue chain %s molecule %d atom %s residue %d %s\n",
						   this_atom->chainID.ascii(),
						   j + 1,
						   this_atom->name.ascii(),
						   (int)this_atom->resSeq,
						   this_atom->resName.ascii()
						   ));
	}
      } else {
	int atompos = -1;
	for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
	{

	  if (residue_list[respos].r_atom[m].name == this_atom->name ||
	      (
	       this_atom->name == "N" &&
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
	  error_string->append(QString("").sprintf("unknown atom chain %s molecule %d atom %s residue %d %s\n",
						   this_atom->chainID.ascii(),
						   j + 1,
						   this_atom->name.ascii(),
						   (int)this_atom->resSeq,
						   this_atom->resName.ascii()
						   ));
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
	  error_string->
	    append(QString("").
		   sprintf("missing atom chain %s molecule %d atom %s residue %d %s\n",
			   lastChainID.ascii(),
			   j + 1,
			   residue_list[lastResPos].r_atom[l].name.ascii(),
			   lastResSeq,
			   residue_list[lastResPos].name.ascii()));
	}
      }
    }
  }

  if(error_string->length()) {
    return errors_found;
  } else {
    return 0;
  }
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
    for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
    {
      for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
      {
	PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

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
	
	// find residue in residues
	int respos = -1;
#if defined(DEBUG)
	printf("residue search name %s resName %s\n", 
	       this_atom->name.ascii(), 
	       this_atom->resName.ascii());
#endif
	for (unsigned int m = 0; m < residue_list.size(); m++)
	{
	  if ((residue_list[m].name == this_atom->resName &&
	       this_atom->name != "OXT" &&
	       (k || this_atom->name != "N")) || 
	      (residue_list[m].name == "OXT" 
	       && this_atom->name == "OXT") ||
	      (!k &&
	       this_atom->name == "N" &&
	       residue_list[m].name == "N1")) 
	  {
	    respos = (int) m;
	    this_atom->p_residue = &(residue_list[m]);
#if defined(DEBUG)
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
	} else {
#if defined(DEBUG)
	  printf("found residue pos %d\n", respos);
#endif
	}
	int atompos = -1;
	
	if (respos != -1)
	{
	  for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
	  {
#if defined(DEBUG)
	    if(this_atom->name == "N" && !k) {
	      printf("this_atom->name == N/N1 this residue_list[%d].r_atom[%d].name == %s\n",
		     respos, m, residue_list[respos].r_atom[m].name.ascii());
	    }
#endif
	    
	    if (residue_list[respos].r_atom[m].name == this_atom->name ||
		(
		 this_atom->name == "N" && 
		 !k &&
		 residue_list[respos].r_atom[m].name == "N1"
		 )
		)
	    {
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
	  } else {
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
#if defined(DEBUG)
	    printf("found atom %s %s in residue %d pos %d bead asgn %d placing info %d mw %f bead_ref_mw %f hybrid name %s %s\n", 
		   this_atom->name.ascii(), this_atom->resName.ascii(), respos, atompos,
		   this_atom->p_atom->bead_assignment,
		   this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].placing_method,
		   this_atom->mw,
		   this_atom->bead_ref_mw,
		   residue_list[respos].r_atom[atompos].hybrid.name.ascii(),
		   this_atom->p_atom->hybrid.name.ascii()
		   );
#endif
	  }
	}
	
	if (this_atom->active)
	{
#if defined(DEBUG)
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
	  } else {
#if defined(DEBUG)
	    printf
	      ("skipped bound waters & H %s %s rad %f resseq %d\n",
	       this_atom->name.ascii(), this_atom->resName.ascii(), this_atom->radius, this_atom->resSeq);
	    fflush(stdout);
#endif
	  }
	}
      }
    }

    {
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
	for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
	  PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];
	  //  printf("p1 i j k %d %d %d %lx %s %d\n", i, j, k, 
	  //	 (long unsigned int)this_atom->p_atom, 
	  //	 this_atom->active ? "active" : "not active",
	  //	 (this_atom->p_atom ? (int)this_atom->p_atom->bead_assignment : -1)
	  //	 ); fflush(stdout);
	  this_atom->bead_assignment =
	    (this_atom->p_atom ? (int) this_atom->p_atom->bead_assignment : -1);
	  this_atom->chain =
	    ((this_atom->p_residue && this_atom->p_atom) ?
	     (int) this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].chain : -1);
	  this_atom->org_chain = this_atom->chain;
	  this_atom->bead_positioner = this_atom->p_atom ? this_atom->p_atom->positioner : false;
	}
      }
    }
    return 0;
}  

int US_Hydrodyn::compute_asa()
{

        QString error_string = "";
	progress->reset();
	editor->setText(QString("\n\nBuilding the bead model for %1 model %2\n").arg(project).arg(current_model+1));
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
	int retval = create_beads(&error_string);
	if ( retval )
	{
	  editor->append("Errors found during the initial creation of beads\n");
	  progress->setProgress(mppos);
	  qApp->processEvents();
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


	if(asa.method == 0) {
	  // surfracer
	  editor->append("Computing ASA via SurfRacer\n");
	  qApp->processEvents();
	  int retval = surfracer_main(&error_string,
				      asa.probe_radius,
				      residue_list,
				      active_atoms,
				      false,
				      progress,
				      editor
				      );

	  progress->setProgress(ppos++); // 2
	  qApp->processEvents();
	  editor->append("Return from Computing ASA\n");
	  if ( retval )
	  {
	    editor->append("Errors found during ASA calculation\n");
	    progress->setProgress(mppos);
	    qApp->processEvents();
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
	  int retval = us_hydrodyn_asab1_main(active_atoms,
					      &asa,
					      &results,
					      false,
					      progress,
					      editor
				      );

	  progress->setProgress(ppos++); // 2
	  qApp->processEvents();
	  editor->append("Return from Computing ASA\n");
	  if ( retval )
	  {
	    editor->append("Errors found during ASA calculation\n");
	    progress->setProgress(mppos);
	    qApp->processEvents();
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

  // for (unsigned int i = 0; i < model_vector.size (); i++)
  {
    unsigned int i = current_model;

    for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
	PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
        // printf("p1 i j k %d %d %d %lx %s\n", i, j, k, (long unsigned int)this_atom->p_atom, this_atom->active ? "active" : "not active"); fflush(stdout);

	//	this_atom->bead_assignment =
	//	  (this_atom->p_atom ? (int) this_atom->p_atom->bead_assignment : -1);
	//	printf("this_atom->bead_assignment %d\n", this_atom->bead_assignment); fflush(stdout);
	//	this_atom->chain =
	//	  ((this_atom->p_residue && this_atom->p_atom) ?
	//	   (int) this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].chain : -1);

	// initialize data
	// this_atom->bead_positioner = false;
	this_atom->normalized_ot_is_valid = false;

	for (unsigned int m = 0; m < 3; m++) {
	  this_atom->bead_cog_coordinate.axis[m] = 0;
	  this_atom->bead_position_coordinate.axis[m] = 0;
	  this_atom->bead_coordinate.axis[m] = 0;
	}
      }
    }
  }

  progress->setProgress(ppos++); // 3
  qApp->processEvents();

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
	  printf("pass 2 active %s %s %d pm %d\n",
		 this_atom->name.ascii(),
		 this_atom->resName.ascii(),
		 this_atom->serial,
		 this_atom->placing_method); fflush(stdout);
#endif

	  molecular_mw += this_atom->mw;
	  for (unsigned int m = 0; m < 3; m++) {
	    molecular_cog[m] += this_atom->coordinate.axis[m] * this_atom->mw;
	  }

	  this_atom->bead_mw = 0;
	  this_atom->bead_asa = 0;

	  // do we have a new bead?
	  // we want to put the N on a previous bead unless it is the first one of the molecule

	  if (!create_beads_normally ||
	      (
	       (this_atom->bead_assignment != last_bead_assignment ||
		this_atom->chain != last_chain ||
		this_atom->resName != last_resName) &&
	       !(this_atom->chain == 0 &&
		 this_atom->name == "N" &&
		 count_actives))) {
#if defined(DEBUG)
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
	    if (create_beads_normally) {
	      if(sidechain_N &&
		 this_atom->chain == 1) {
		if(this_atom->name == "N") {
		  printf("ERROR double N on sequential sidechains! PRO PRO?\n");
		}
#if defined(DEBUG)
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
	      if(this_atom->name == "N" &&
		 this_atom->chain == 1) {
		sidechain_N = this_atom;
		this_atom->is_bead = false;
	      }
	    }
	  } else {
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
	      this_atom->name == "N" &&
	      last_main_chain_bead) {
	    use_atom = last_main_chain_bead;
	  } else {
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
#if defined(DEBUG)
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
#if defined(DEBUG)
	    printf("afterwards: target mw %f pos [%f,%f,%f]\n",
		   use_atom->bead_cog_mw,
		   use_atom->bead_cog_coordinate.axis[0],
		   use_atom->bead_cog_coordinate.axis[1],
		   use_atom->bead_cog_coordinate.axis[2]);
#endif
	  } else {
#if defined(DEBUG)
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

	} else {
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
  } else {
    printf("ERROR: this molecule has zero mw!\n");
  }

  for (unsigned int m = 0; m < 3; m++) {
    last_molecular_cog.axis[m] = molecular_cog[m];
  }

  progress->setProgress(ppos++); // 4
  qApp->processEvents();

  // pass 2b move bead_ref_volume, ref_mw, computed_radius from
  // next main chain back one

  // for (unsigned int i = 0; i < model_vector.size (); i++)	{
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
	  } else {
	    last_main_chain_bead = this_atom;
	  }
	}
      }
    }
  }

  progress->setProgress(ppos++); // 5
  qApp->processEvents();

  // pass 2c hydration

  // for (unsigned int i = 0; i < model_vector.size (); i++)	{
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


#if defined(DEBUG1) || defined(DEBUG)
  printf("pass 3\n"); fflush(stdout);
#endif

  // for (unsigned int i = 0; i < model_vector.size (); i++) {
  {
    unsigned int i = current_model;

    for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {

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
	    } else {
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
	      //	this_atom->name.ascii(),
	      //	this_atom->resName.ascii(),
	      //	this_atom->serial);
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
	  } else {
	    fprintf(stderr, "serious internal error 1 on %s %s %d, quitting\n",
		    this_atom->name.ascii(),
		    this_atom->resName.ascii(),
		    this_atom->serial);
	    exit(-1);
	    break;
	  }
#if defined(DEBUG)
	  printf("pass 3 active is bead %s %s %d checkpoint 1\n",
		 this_atom->name.ascii(),
		 this_atom->resName.ascii(),
		 this_atom->serial); fflush(stdout);
#endif
	  this_atom->visibility = (this_atom->bead_asa >= asa.threshold);

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
	  } else {
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
	} else {
	  this_atom->placing_method = -1;
	}
      }
    }
  }

  // pass 4 print results
  progress->setProgress(ppos++); // 7
  qApp->processEvents();

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
	  bead_model.push_back(*this_atom);
	}
      }
    }
  }
  write_bead_asa(somo_dir + SLASH +
		 project + QString("_%1").arg(current_model + 1) + 
		 QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "")
		 + DOTSOMO + ".asa", &bead_model);

#if defined(DEBUG_MOD)
  write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_debug" + DOTSOMO + ".tsv", &dbg_model);
#endif
  editor->append(QString("There are %1 beads in this model before popping\n").arg(bead_model.size()));

  progress->setProgress(ppos++); // 8
  qApp->processEvents();

  // #define DEBUG

  // popping radial reduction

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

  // or sc fb Y rh Y rs Y to Y st Y ro Y 70.000000 1.000000 0.000000
  // or scmc fb Y rh N rs N to Y st N ro Y 1.000000 0.000000 70.000000
  // or bb fb Y rh Y rs N to N st Y ro N 0.000000 0.000000 0.000000

#if defined(DEBUG)
  printf("or sc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
	 "or scmc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
	 "or bb fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n",
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
	 buried_overlap.remove_hierarch_percent);

#endif

  int methods[] =
    {
      RADIAL_REDUCTION | RR_SC | RR_EXPOSED,
      RADIAL_REDUCTION | RR_MCSC | RR_EXPOSED,
      RADIAL_REDUCTION | RR_MCSC | RR_BURIED,
    };

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


  write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO + ".tsv", &bead_model);
  write_bead_spt(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO, &bead_model);
  for(unsigned int k = 0; k < sizeof(methods) / sizeof(int); k++) {

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
	if (overlaps_exist) {
	  beads_popped++;
#if defined(DEBUG1) || defined(DEBUG)
	  printf("popping beads %u %u int vol %f mw1 %f mw2 %f v1 %f v2 %f c1 [%f,%f,%f] c2 [%f,%f,%f]\n",
		 bead_model[max_bead1].serial,
		 bead_model[max_bead2].serial,
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
	  if (bead_model[max_bead2].chain) {
	    bead_model[max_bead1].chain = 1;
	  }
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
    write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
    write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO, &bead_model);
    printf("stage %d beads popped %d\n", k, beads_popped);
    progress->setProgress(ppos++); // 12,13,14
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
	    if (bead_model[i].active &&
		bead_model[j].active &&
		(((methods[k] & RR_SC) &&
		  bead_model[i].chain == 1 &&
		  bead_model[j].chain == 1) ||
		 ((methods[k] & RR_MC) &&
		  bead_model[i].chain == 0 &&
		  bead_model[j].chain == 0) ||
		 ((methods[k] & RR_MCSC)
		  // &&
		  // (bead_model[i].chain != 1 ||
		  // bead_model[j].chain != 1)))
		  )) &&
		(((methods[k] & RR_EXPOSED) &&
		  bead_model[i].exposed_code == 1 &&
		  bead_model[j].exposed_code == 1) ||
		 ((methods[k] & RR_BURIED) &&
		  (bead_model[i].exposed_code != 1 ||
		   bead_model[j].exposed_code != 1)) ||
		 (methods[k] & RR_ALL))) {

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
#if defined(DEBUG_OVERLAP)
	  overlap_check(methods[k] & RR_SC ? true : false,
			methods[k] & RR_MCSC ? true : false,
			methods[k] & RR_BURIED ? true : false);
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
		  } else {
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
		} else {
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
		    } else {
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
		    } else {
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
	      } else {
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
		   bead_model[max_bead1].chain == 1 &&
		   bead_model[max_bead2].chain == 1) {
		  printf("what are we doing here?  dealing with 2 SC's on the MCSC run??\n");
		  exit(-1);
		}
		if((methods[k] & RR_BURIED &&
		    (bead_model[max_bead1].exposed_code == 1 ||
		     bead_model[max_bead2].exposed_code == 1)) ||
		   (methods[k] & RR_MCSC &&
		    (bead_model[max_bead1].chain == 1 ||
		     bead_model[max_bead2].chain == 1))) {
		  // only one bead to shrink, since
		  // we are either buried with one of the beads exposed or
		  // on the MCSC and one of the beads is SC
		  int use_bead;
		  if (methods[k] & RR_BURIED) {
		    if (bead_model[max_bead1].exposed_code == 1) {
		      use_bead = max_bead2;
		    } else {
		      use_bead = max_bead1;
		    }
		  } else {
		    if (bead_model[max_bead1].chain == 1) {
		      use_bead = max_bead2;
		    } else {
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
		} else {
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
      } else {
	// simultaneous reduction
	do {
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
	    if (last_reduced[i]) {
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
		if (last_reduced[j] &&
		    bead_model[i].active &&
		    bead_model[j].active &&
		    (((methods[k] & RR_SC) &&
		      bead_model[i].chain == 1 &&
		      bead_model[j].chain == 1) ||
		     ((methods[k] & RR_MC) &&
		      bead_model[i].chain == 0 &&
		      bead_model[j].chain == 0) ||
		     ((methods[k] & RR_MCSC)
		      // &&
		      // (bead_model[i].chain != 1 ||
		      // bead_model[j].chain != 1)))
		      )) &&
		    (((methods[k] & RR_EXPOSED) &&
		      bead_model[i].exposed_code == 1 &&
		      bead_model[j].exposed_code == 1) ||
		     ((methods[k] & RR_BURIED) &&
		      (bead_model[i].exposed_code != 1 ||
		       bead_model[j].exposed_code != 1)) ||
		     (methods[k] & RR_ALL))) {

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
	      if (!reduced[pairs[i].i]) {
		int use_bead = pairs[i].i;
		/*		if ( !(methods[k] & RR_MCSC) ||
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
		      } else {
			printf("wow! bead %d is at the molecular cog!\n", use_bead);
		      }
		    } else {
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
		    bead_model[use_bead].bead_computed_radius = (float)TOLERANCE;
		    reduced[use_bead] = false;
		  }
		}
	      }
	      if (!reduced[pairs[i].j]) {
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
		      } else {
			printf("wow! bead %d is at the molecular cog!\n", use_bead);
		      }
		    } else {
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
		    bead_model[use_bead].bead_computed_radius = (float)TOLERANCE;
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
    write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
    write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO, &bead_model);
  } // methods
  write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO + ".tsv", &bead_model);
  write_bead_spt(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO, &bead_model);
  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) + DOTSOMO, &bead_model);
  write_bead_model(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) + 
		   QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") + DOTSOMO
		   , &bead_model);
  editor->append("Finished with popping and radial reduction\n");
  progress->setProgress(mppos - (asa.recheck_beads ? 1 : 0));
  qApp->processEvents();
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
  } else {
    color = 1;
    if (a->exposed_code != 1) {
      color = 10;
    }
  }
#endif
  return color;
}

void US_Hydrodyn::write_bead_spt(QString fname, vector<PDB_atom> *model, bool loaded_bead_model) {

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
  // #define DEBUG
#if defined(DEBUG)
  printf("write bead spt %s\n", fname.ascii()); fflush(stdout);
#endif

  FILE *fspt = fopen(QString("%1.spt").arg(fname).ascii(), "w");
  FILE *fbms = fopen(QString("%1.bms").arg(fname).ascii(), "w");
  FILE *fbeams = fopen(QString("%1.beams").arg(fname).ascii(), "w");
  FILE *frmc = fopen(QString("%1.rmc").arg(fname).ascii(), "w");
  FILE *frmc1 = fopen(QString("%1.rmc1").arg(fname).ascii(), "w");
  int beads = 0;
  if(!(fspt) ||
     !(fbms) ||
     !(fbeams) ||
     !(frmc) ||
     !(frmc1)) {
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

  fprintf(fbeams,
	  "%d\t-2.000000\t%s.rmc\t%f\n",
	  beads,
	  QFileInfo(fname).fileName().ascii(),
	  results.vbar
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
      fprintf(fbeams,
	      "%f\t%f\t%f\n",
	      (*model)[i].bead_coordinate.axis[0],
	      (*model)[i].bead_coordinate.axis[1],
	      (*model)[i].bead_coordinate.axis[2]
	      );
      fprintf(fspt,
	      "select atomno=%d\nspacefill %.2f\ncolour %s\n",
	      atomno++,
	      (*model)[i].bead_computed_radius / scaling,
	      colormap[get_color(&(*model)[i])]
	      );
      fprintf(frmc,
	      "%.6f\t%u\t%d\n",
	      (*model)[i].bead_computed_radius,
	      (int)(*model)[i].bead_mw,
	      get_color(&(*model)[i]));
      unsigned int tmp_serial = (*model)[i].serial;
      QString residues;
      if (!loaded_bead_model)
      {
	residues =
	  (*model)[i].resName +
	  ((*model)[i].org_chain ? ".SC." : ".MC.") +
	  ((*model)[i].chainID == " " ? "" : ((*model)[i].chainID + "."));
	// a compiler error forced this kludge using tmp_serial
	//	+ QString("%1").arg((*model)[i].serial);
	residues += QString("%1").arg(tmp_serial);

	for (unsigned int j = 0; j < (*model)[i].all_beads.size(); j++)
        {
	  unsigned int tmp_serial = (*model)[i].all_beads[j]->serial;
	  residues += "," +
	    (*model)[i].all_beads[j]->resName +
	    ((*model)[i].all_beads[j]->org_chain ? ".SC." : ".MC.") +
	    ((*model)[i].all_beads[j]->chainID == " " ? "" : ((*model)[i].all_beads[j]->chainID + "."));
	  // a compiler error forced this kludge using tmp_serial
	  //  + QString("%1").arg((*model)[i].all_beads[j].serial);
	residues += QString("%1").arg(tmp_serial);
	}
      } else {
	residues = (*model)[i].residue_list;
      }
      fprintf(frmc1,
	      "%.6f\t%u\t%d\t%d\t%s\n",
	      (*model)[i].bead_computed_radius,
	      (int)(*model)[i].bead_mw,
	      get_color(&(*model)[i]),
	      (*model)[i].serial,
	      residues.ascii()
	      );
    }
  }
  fclose(fspt);
  fclose(fbms);
  fclose(fbeams);
  fclose(frmc);
  fclose(frmc1);
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

	  //	  ((*model)[i].all_beads)[j]->serial + "-" +
	  //	  ((*model)[i].all_beads)[j]->name + "-" +
	  //	  ((*model)[i].all_beads)[j]->resName + "-" +
	  //	  ((*model)[i].all_beads)[j]->chainID + ";";
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
  fprintf(f, " N.	   Res.       ASA        MAX ASA\n");

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
      total_mass += (*model)[i].bead_mw;
      total_vol += (*model)[i].bead_ref_volume_unhydrated;

      QString residue =
	(*model)[i].resName + "_" +
	((*model)[i].chainID == " " ? "_" : (*model)[i].chainID) +
	QString("_%1").arg((*model)[i].resSeq);
      if (residue != last_residue) {
	if (last_residue != "") {
	  fprintf(f,
		  " [ %-6d %s ]\t%.0f\t%.0f\n",
		  seqno, last_residue.ascii(), residue_asa, residue_ref_asa);
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
	    " [ %-6d %s ]\t%.0f\t%.0f\n",
	    seqno, last_residue.ascii(), residue_asa, residue_ref_asa);
  }

  fprintf(f,
	  "\n\n\n"
	  "\tTOTAL ASA OF THE MOLECULE    = %.0f\t[A^2] (Threshold used: %.1f A^2]\n"
	  "\tTOTAL VOLUME OF THE MOLECULE = %-.2f\t[A^3]\n"
	  //	  "\tRADIUS OF GYRATION (+r) =  %-.2f\t[A]\n"
	  //	  "\tRADIUS OF GYRATION (-r) =  %-.2f	[A]\n"
	  "\tMASS OF THE MOLECULE    =  %.0f	[Da]\n"
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
  // int bead_output.sequence;           // 0 = 
  // 1 = 


#if defined(DEBUG)
  printf("write bead model %s\n", fname.ascii()); fflush(stdout);
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

  FILE *fbeadmodel = fopen(QString("%1.bead_model").arg(fname).ascii(), "w");
  int beads = 0;

  for (unsigned int i = 0; i < use_model.size(); i++) {
    if (use_model[i]->active) {
      beads++;
    }
  }

  fprintf(fbeadmodel,
	  "%d\t%f\n",
	  beads,
	  results.vbar
	  );

  for (unsigned int i = 0; i < use_model.size(); i++) {
    if (use_model[i]->active) {
      unsigned int tmp_serial = use_model[i]->resSeq; // was serial
      QString residues =
	use_model[i]->resName +
	(use_model[i]->org_chain ? ".SC." : ".MC.") +
	(use_model[i]->chainID == " " ? "" : (use_model[i]->chainID + "."));
      // a compiler error forced this kludge using tmp_serial
      //	+ QString("%1").arg((*use_model)[i].serial);
      residues += QString("%1").arg(tmp_serial);

      for (unsigned int j = 0; j < use_model[i]->all_beads.size(); j++)
      {
	unsigned int tmp_serial = use_model[i]->all_beads[j]->resSeq;
	residues += "," +
	  (use_model[i]->all_beads[j]->resName +
	  (use_model[i]->all_beads[j]->org_chain ? ".SC." : ".MC.") +
	  (use_model[i]->all_beads[j]->chainID == " " ? "" : (use_model[i]->all_beads[j]->chainID + ".")));
	// a compiler error forced this kludge using tmp_serial
	//  + QString("%1").arg((*use_model)[i].all_beads[j].serial);
	residues += QString("%1").arg(tmp_serial);
      }
      fprintf(fbeadmodel,
	      "%f\t%f\t%f\t%.6f\t%u\t%d\t%d\t%s\n",
	      use_model[i]->bead_coordinate.axis[0],
	      use_model[i]->bead_coordinate.axis[1],
	      use_model[i]->bead_coordinate.axis[2],
	      use_model[i]->bead_computed_radius,
	      (int)use_model[i]->bead_mw,
	      get_color(use_model[i]),
	      use_model[i]->serial,
	      residues.ascii()
	      );
    }
  }
  fclose(fbeadmodel);
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
				      editor
				      );
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
  int e2b = 0;

  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) + "_pre_recheck" + DOTSOMO, &bead_model);
  write_bead_model(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) + "_pre_recheck" + DOTSOMO, &bead_model);

  for (unsigned int i = 0; i < bead_model.size(); i++) 
  {
    float surface_area = bead_model[i].bead_computed_radius * bead_model[i].bead_computed_radius * 4 * M_PI;
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
    } else {
      // now buried
      if(bead_model[i].exposed_code == 1) {
	// was exposed
	msg = "exposed->buried";
	e2b++;
	bead_model[i].exposed_code = 6;
	bead_model[i].bead_color = 6;
      }
    }
      
    printf("bead %d %.2f %.2f %s %s\n",
	   i,
	   surface_area,
	   bead_model[i].bead_recheck_asa,
	   (bead_model[i].bead_recheck_asa >
	    (asa.threshold_percent / 100.0) * surface_area) ? 
	   "exposed" : "buried",
	   msg.ascii());
  }
  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
		   QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
		 DOTSOMO, &bead_model);
  write_bead_model(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
		   QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
		   DOTSOMO, &bead_model);
  editor->append(QString("%1 exposed beads became buried\n").arg(e2b));
  editor->append(QString("%1 buried beads became exposed\n").arg(b2e));
}

void US_Hydrodyn::select_model(int val)
{
	current_model = val;
	editor->setText(QString("\n\n%1 model %2 selected.\n").arg(project).arg(current_model+1));
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

int US_Hydrodyn::calc_somo()
{
  pb_somo->setEnabled(false);
  if (!residue_list.size() ||
      !model_vector.size())
	{
		fprintf(stderr, "calculations can not be run until residue & pdb files are read!\n");
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

	if(!compute_asa()) {
	  if (asa.recheck_beads)
	    {
	      // puts("recheck beads disabled");
	      editor->append("Rechecking beads\n");
	      qApp->processEvents();

	      bead_check();
	      editor->append("Finished rechecking beads\n");
	      progress->setProgress(19);
	    } else {
	      editor->append("No rechecking of beads\n");
	      qApp->processEvents();
	    }

	  // calculate bead model and generate hydrodynamics calculation output
	  // if successful, enable follow-on buttons:
	  editor->append("Build bead model completed\n\n");
	  qApp->processEvents();
	  pb_visualize->setEnabled(true);
	  pb_calc_hydro->setEnabled(true);
	}

	pb_somo->setEnabled(true);
	return 0;
}

void US_Hydrodyn::calc_hydro()
{
  pb_calc_hydro->setEnabled(false);
  puts("calc hydro (supc)");
  editor->append("Begin hydrodynamic calculations\n");
  write_bead_spt(somo_dir + SLASH + project +
		 (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
		 QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
		 DOTSOMO, &bead_model, bead_model_from_file);

  chdir(somo_dir);
  int retval = us_hydrodyn_supc_main(&results,
				     &hydro,
				     &bead_model,
				     QString(project +
					     (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
					     QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
					     DOTSOMO + ".beams").ascii(),
				     progress,
				     editor);
  chdir(somo_tmp_dir);

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
	cout << "residue file name: " << residue_filename << endl;
	residue_list.clear();
	i=1;
	if (f.open(IO_ReadOnly|IO_Translate))
	{
		QTextStream ts(&f);
		while (!ts.atEnd())
		{
			ts >> new_residue.name;
			ts >> new_residue.type;
			ts >> new_residue.molvol;
			ts >> new_residue.asa;
			ts >> numatoms;
			ts >> numbeads;
			ts >> new_residue.vbar;
			ts.readLine(); // read rest of line
			new_residue.r_atom.clear();
			new_residue.r_bead.clear();
			for (j=0; j<numatoms; j++)
			{
				ts >> new_atom.name;
				ts >> new_atom.hybrid.name;
				ts >> new_atom.hybrid.mw;
				ts >> new_atom.hybrid.radius;
				ts >> new_atom.bead_assignment;
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
				ts >> new_atom.chain;
				str2 = ts.readLine(); // read rest of line
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
				residue_list.push_back(new_residue);
			}
		}
		f.close();
	}
}

void US_Hydrodyn::load_pdb()
{
	QString filename = QFileDialog::getOpenFileName(somo_pdb_dir, "*.pdb *.PDB", this);
	if (!filename.isEmpty())
	{
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
		read_pdb(filename);
		QString error_string = "";
		for(unsigned int i = 0; i < model_vector.size(); i++)
		{
			editor->append(QString("Checking the pdb structure for model %1\n").arg(i+1));
		  	if (check_for_missing_atoms(&error_string, &model_vector[i]))
			{
		   	errors_found++;
		   	editor->append(QString("US_SURFRACER encountered errors with your PDB structure for model %1:\n").
				arg(i + 1) + error_string);
		   	printError(QString("US_SURFRACER encountered errors with your PDB structure for model %1:\n").
			   arg(i + 1) + "please check the text window");
			}
		}
		editor->append(QString("Loaded pdb file : %1\n").arg(errors_found ? "ERRORS PRESENT" : "ok"));
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
	pb_show_hydro_results->setEnabled(false);
	pb_calc_hydro->setEnabled(false);
	pb_visualize->setEnabled(false);
	le_bead_model_file->setText(" not selected ");
}

void US_Hydrodyn::load_bead_model()
{
	QString filename = QFileDialog::getOpenFileName(somo_dir, QString("*%1.bead_model *%2.beams *%3.BEAD_MODEL *%4.BEAMS")
							.arg(DOTSOMO).arg(DOTSOMO).arg(DOTSOMOCAP).arg(DOTSOMOCAP), 
							this);
	if (!filename.isEmpty())
	{
		pb_somo->setEnabled(false);
		pb_visualize->setEnabled(false);
		pb_calc_hydro->setEnabled(false);
		pb_show_hydro_results->setEnabled(false);
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
	unsigned int last_resSeq = 0; // keeps track of residue sequence number, initialize to zero, first real one will be "1"
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
				for (unsigned int m=0; m<temp_model.residue.size(); m++ )
				{
				    str += temp_model.residue[m].name + " ";
				}
				editor->append(str);
				calc_vbar(&temp_model); // update the calculated vbar for this model
				model_vector.push_back(temp_model); // save the model in the model vector.
				clear_temp_chain(&temp_chain); // we are done with this molecule and can delete it
			}
			if (str1.left(4) == "ATOM" || str1.left(6) == "HETATM") // need to add TER
			{
			  if(str1.mid(12,1) != "H" && str1.mid(13,1) != "H")
			  {
				if (!chain_flag) 	// at the first time we encounter the word ATOM
				{ 				// we don't have a chain yet, so let's start a new one
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
		f.close();
	}
	if(!model_flag)	// there were no model definitions, just a single molecule,
	{						// we still need to save the results
		temp_model.molecule.push_back(temp_chain);
		editor->append("\nResidue sequence from " + project +".pdb:\n");
		str = "";
		for (unsigned int m=0; m<temp_model.residue.size(); m++ )
		{
			str += temp_model.residue[m].name + " ";
		}
		editor->append(str);
		temp_model.model_id = 1;
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
	//	project.replace(QRegExp(".*(/|\\\\)"), "");
	//	project.replace(QRegExp("\\.(somo|SOMO)\\.(bead_model|BEAD_MODEL)$"), "");
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
		} else {
		  editor->append("Error in line 1!\n");
		  return 1;
		}
		if (!ts.atEnd()) {
		  ts >> results.vbar;
		} else {
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
		    } else {
		      editor->append(QString("\nError in line %1!\n").arg(linepos));
		      return linepos;
		    }
		  }
		  if (!ts.atEnd()) {
		    ts >>  tmp_atom.bead_computed_radius;
		  } else {
		    editor->append(QString("\nError in line %1!\n").arg(linepos));
		    return linepos;
		  }
		  if (!ts.atEnd()) {
		    ts >>  tmp_atom.bead_mw;
		  } else {
		    editor->append(QString("\nError in line %1!\n").arg(linepos));
		    return linepos;
		  }
		  if (!ts.atEnd()) {
		    ts >>  tmp_atom.bead_color;
		  } else {
		    editor->append(QString("\nError in line %1!\n").arg(linepos));
		    return linepos;
		  }
		  if (!ts.atEnd()) {
		    ts >>  tmp_atom.serial;
		  } else {
		    editor->append(QString("\nError in line %1!\n").arg(linepos));
		    return linepos;
		  }
		  if (!ts.atEnd()) {
		    ts >>  tmp_atom.residue_list;
		  } else {
		    editor->append(QString("\nError in line %1!\n").arg(linepos));
		    return linepos;
		  }
		  tmp_atom.exposed_code = 1;
		  tmp_atom.all_beads.clear();
		  tmp_atom.active = true;
		  bead_model.push_back(tmp_atom);
		}
		f.close();
		if (bead_count != (int)bead_model.size())
		{
		  editor->append(QString("Error: bead count %1 does not match # of beads read from file (%2) \n").arg(bead_count).arg(bead_model.size()));
		  return -1;
		}
		bead_model_from_file = true;
		editor->append("Bead model loaded\n\n");
		write_bead_spt(somo_dir + SLASH + project +
			       QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
			       DOTSOMO, &bead_model, true);
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
		} else {
		  editor->append("Error in line 1!\n");
		  return 1;
		}
		if (!ts.atEnd()) {
		  ts >> results.vbar;
		} else {
		  editor->append("Error in line 1!\n");
		  return 1;
		}
		QString rmcfile;
		if (!ts.atEnd()) {
		  ts >> rmcfile;
		} else {
		  editor->append("Error in line 1!\n");
		  return 1;
		}
		if (results.vbar == -2)
		{
		  if (!ts.atEnd()) {
		    ts >> results.vbar;
		  } else {
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
		      } else {
			editor->append(QString("\nError in line %1!\n").arg(linepos));
			return linepos;
		      }
		    }
		    if (!tsrmc.atEnd()) {
		      tsrmc >>  tmp_atom.bead_computed_radius;
		    } else {
		      editor->append(QString("\nError in line %1!\n").arg(linepos));
		      return linepos;
		    }
		    if (!tsrmc.atEnd()) {
		      tsrmc >>  tmp_atom.bead_mw;
		    } else {
		      editor->append(QString("\nError in line %1!\n").arg(linepos));
		      return linepos;
		    }
		    if (!tsrmc.atEnd()) {
		      tsrmc >>  tmp_atom.bead_color;
		    } else {
		      editor->append(QString("\nError in line %1!\n").arg(linepos));
		      return linepos;
		    }
		    tmp_atom.serial = linepos;
		    tmp_atom.exposed_code = 1;
		    tmp_atom.all_beads.clear();
		    tmp_atom.active = true;
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
		editor->append("Bead model loaded\n\n");
		write_bead_spt(somo_dir + SLASH + project +
			       QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
			       DOTSOMO, &bead_model, true);
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
	(*model).vbar = (mw_vbar_sum/mw_sum) - 0.002125;
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

	if ( printer.setup(this) ) {		// opens printer dialog
		QPainter p;
		p.begin( &printer );			// paint on printer
		p.setFont(editor->font() );
		int yPos		= 0;			// y position for each line
		QFontMetrics fm = p.fontMetrics();
		QPaintDeviceMetrics metrics( &printer ); // need width/height
											 // of printer surface
		for( int i = 0 ; i < editor->lines() ; i++ ) {
			if ( MARGIN + yPos > metrics.height() - MARGIN ) {
				printer.newPage();		// no more room on this page
				yPos = 0;			// back to top of page
			}
			p.drawText( MARGIN, MARGIN + yPos,
							metrics.width(), fm.lineSpacing(),
							ExpandTabs | DontClip,
							editor->text( i ) );
			yPos = yPos + fm.lineSpacing();
		}
		p.end();				// send job to printer
	}
}

void US_Hydrodyn::clear_temp_chain(struct PDB_chain *temp_chain) // clear all the memory from the vectors in temp_chain
{
	(*temp_chain).atom.clear();
	(*temp_chain).chainID = "";
	(*temp_chain).segID = "";
}

bool US_Hydrodyn::assign_atom(const QString &str1, struct PDB_chain *temp_chain, unsigned int *last_resSeq)
{
/*
http://www.rcsb.org/pdb/docs/format/pdbguide2.2/part_11.html

ATOM record:
COLUMNS        DATA TYPE       FIELD         DEFINITION
---------------------------------------------------------------------------------
 1 -  6        Record name     "ATOM  "
 7 - 11        Integer         serial        Atom serial number.
13 - 16        Atom            name          Atom name.
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

	str2 = str1.mid(13, 4);
	temp_atom.name = str2.stripWhiteSpace();

	temp_atom.altLoc = str1.mid(16, 1);

	temp_atom.resName = str1.mid(17, 3);

	temp_atom.chainID = str1.mid(21, 1);

	str2 = str1.mid(22, 4);
	temp_atom.resSeq = str2.toUInt();
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
	  //	printError(tr("The residue " + temp_atom.resName + " listed in this PDB file is not found in the residue table!"));
	}

	return(flag);
}

void US_Hydrodyn::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;
	write_config();
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

void US_Hydrodyn::read_config()
{
	QFile f;
	QString str;
	f.setName(USglobal->config_list.root_dir + "/etc/somo.config");
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
	overlap_tolerance = str.toDouble();

	ts >> str;
	ts.readLine();
	bead_output.output = str.toInt();
	ts >> str;
	ts.readLine();
	bead_output.sequence = str.toInt();
	ts >> str;
	ts.readLine();
	asa.probe_radius = str.toDouble();
	ts >> str;
	ts.readLine();
	asa.threshold = str.toDouble();
	ts >> str;
	ts.readLine();
	asa.threshold_percent = str.toDouble();
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
	misc.hydrovol = str.toDouble();
	ts >> str;
	ts.readLine();
	misc.compute_vbar = (bool) str.toInt();
	ts >> str;
	ts.readLine();
	misc.vbar = str.toDouble();
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
}

void US_Hydrodyn::reset()
{
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

	sidechain_overlap.title = "exposed side chain beads";
	mainchain_overlap.title = "exposed main/main and main/side chain beads";
	buried_overlap.title = "buried beads";

	bead_output.sequence = 0;
	bead_output.output = 0;
	asa.probe_radius = 1.4f;
	asa.threshold = 10.0;
	asa.threshold_percent = 30.0;
	asa.calculation = true;
	asa.recheck_beads = true;
	asa.method = true; // by default use ASAB1
	asa.asab1_step = 1.0;
	misc.hydrovol = 24.041;
	misc.compute_vbar = true;
	misc.vbar = 0.72;
	overlap_tolerance = 0.001;

	hydro.unit = -10; 					// exponent from units in meter (example: -10 = Angstrom, -9 = nanometers)
	hydro.reference_system = false;	// false: diffusion center, true: cartesian origin (default false)
	hydro.boundary_cond = false;		// false: stick, true: slip (default false)
	hydro.volume_correction = false;	// false: Automatic, true: manual (provide value)
	hydro.volume = 0.0;					// volume correction
	hydro.mass_correction = false;		// false: Automatic, true: manual (provide value)
	hydro.mass = 0.0;						// mass correction
	hydro.bead_inclusion = false;		// false: exclude hidden beads; true: use all beads
	hydro.rotational = false;			// false: include beads in volume correction for rotational diffusion, true: exclude
	hydro.viscosity = false;				// false: include beads in volume correction for intrinsic viscosity, true: exclude
	hydro.overlap_cutoff = false;		// false: same as in model building, true: enter manually
	hydro.overlap = 0.0;					// overlap
}

void US_Hydrodyn::write_config()
{
	QFile f;
	QString str;
	f.setName(USglobal->config_list.root_dir + "/etc/somo.config");
	if (f.open(IO_WriteOnly | IO_Translate)) // first try user's directory for default settings
	{
		QTextStream ts(&f);
		ts << "SOMO Config file - computer generated, please do not edit...\n";
		ts << sidechain_overlap.remove_overlap << "\t\t# Remove overlaps flag\n";
		ts << sidechain_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
		ts << sidechain_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
		ts << sidechain_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
		ts << sidechain_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
		ts << sidechain_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
		ts << sidechain_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
		ts << sidechain_overlap.translate_out << "\t\t# Outward translation flag\n";
		ts << sidechain_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

		ts << mainchain_overlap.remove_overlap << "\t\t# Remove overlaps flag\n";
		ts << mainchain_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
		ts << mainchain_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
		ts << mainchain_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
		ts << mainchain_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
		ts << mainchain_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
		ts << mainchain_overlap.remove_sync_percent << "\t\t# percent synchronously step\n";
		ts << mainchain_overlap.translate_out << "\t\t# Outward translation flag\n";
		ts << mainchain_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

		ts << buried_overlap.remove_overlap << "\t\t# Remove overlaps flag\n";
		ts << buried_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
		ts << buried_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
		ts << buried_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
		ts << buried_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
		ts << buried_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
		ts << buried_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
		ts << buried_overlap.translate_out << "\t\t# Outward translation flag\n";
		ts << buried_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";
		ts << overlap_tolerance << "\t\t# bead overlap tolerance\n";

		ts << bead_output.output << "\t\t# flag for selecting output format\n";
		ts << bead_output.sequence << "\t\t# flag for selecting sequence format\n";
		ts << asa.probe_radius << "\t\t# probe radius in angstrom\n";
		ts << asa.threshold << "\t\t# ASA threshold\n";
		ts << asa.threshold_percent << "\t\t# ASA threshold percent\n";
		ts << asa.calculation << "\t\t# flag for calculation of ASA\n";
		ts << asa.recheck_beads << "\t\t# flag for rechecking beads\n";
		ts << asa.method << "\t\t# flag for ASAB1/Surfracer method\n";
		ts << asa.asab1_step << "\t\t# ASAB1 step size\n";
		ts << misc.hydrovol << "\t\t# hydration volume\n";
		ts << misc.compute_vbar << "\t\t# flag for selecting vbar calculation\n";
		ts << misc.vbar << "\t\t# vbar value\n";

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
		&mainchain_overlap, &buried_overlap, &overlap_tolerance, &overlap_widget);
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

void US_Hydrodyn::printError(const QString &str)
{
	QMessageBox::warning(this, tr("UltraScan Warning"), tr("Please note:\n\n") +
			tr(str), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
}

void US_Hydrodyn::closeAttnt(QProcess *proc, QString message)
{
	switch( QMessageBox::information( this,
		tr("Attention!"),	message + tr(" is still running.\n"
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
