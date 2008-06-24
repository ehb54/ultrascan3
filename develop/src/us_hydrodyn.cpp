#include "../include/us_hydrodyn.h"
#include "../include/us_surfracer.h"

US_Hydrodyn::US_Hydrodyn(QWidget *p, const char *name) : QFrame(p, name)
{
	USglobal=new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("SOMO Solution Bead Modeler"));
	read_config();
	atom_widget = false;
	residue_widget = false;
	hybrid_widget = false;
	setupGUI();
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn::~US_Hydrodyn()
{
}

void US_Hydrodyn::setupGUI()
{
	int minHeight1 = 30;
	residue_filename = "";
	sidechain_OR = new US_Hydrodyn_OR(&sidechain_overlap, this);
	mainchain_OR = new US_Hydrodyn_OR(&mainchain_overlap, this);
	buried_OR = new US_Hydrodyn_OR(&buried_overlap, this);
	buried_OR->cnt_fuse->setEnabled(false);
	buried_OR->cb_fuse->setEnabled(false);
	
	lbl_info = new QLabel(tr("Bead Overlap Reduction Options:"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setMinimumHeight(minHeight1);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	tw_overlap = new QTabWidget(this);
	tw_overlap->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	tw_overlap->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	tw_overlap->addTab(sidechain_OR, "Side chain beads");
	tw_overlap->addTab(mainchain_OR, "Main and side chain beads");
	tw_overlap->addTab(buried_OR, "Buried beads");

	pb_select_residue_file = new QPushButton(tr("Select Lookup Table"), this);
	Q_CHECK_PTR(pb_select_residue_file);
	pb_select_residue_file->setMinimumHeight(minHeight1);
	pb_select_residue_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_select_residue_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_select_residue_file, SIGNAL(clicked()), SLOT(select_residue_file()));

	pb_somo = new QPushButton(tr("Build Bead Model"), this);
	Q_CHECK_PTR(pb_somo);
	pb_somo->setMinimumHeight(minHeight1);
	pb_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_somo->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_somo, SIGNAL(clicked()), SLOT(calc_somo()));

	pb_visualize = new QPushButton(tr("Visualize Bead Model"), this);
	Q_CHECK_PTR(pb_visualize);
	pb_visualize->setMinimumHeight(minHeight1);
	pb_visualize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_visualize->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_visualize, SIGNAL(clicked()), SLOT(visualize()));

	lbl_table = new QLabel(tr(" not selected"),this);
	lbl_table->setMinimumHeight(minHeight1);
	lbl_table->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_table->setAlignment(AlignCenter|AlignVCenter);
	lbl_table->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	pb_load_pdb = new QPushButton(tr("Load PDB File"), this);
	Q_CHECK_PTR(pb_load_pdb);
	pb_load_pdb->setMinimumHeight(minHeight1);
	pb_load_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_load_pdb->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load_pdb, SIGNAL(clicked()), SLOT(load_pdb()));

	lbl_pdb_file = new QLabel(tr(" not selected"),this);
	lbl_pdb_file->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_pdb_file->setAlignment(AlignCenter|AlignVCenter);
	lbl_pdb_file->setMinimumHeight(minHeight1);
	lbl_pdb_file->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_pdb_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_model = new QLabel(tr(" Please select a Model:"), this);
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
	
	lbl_probe_radius = new QLabel(tr(" ASA Probe Radius (A): "), this);
	Q_CHECK_PTR(lbl_probe_radius);
	lbl_probe_radius->setAlignment(AlignLeft|AlignVCenter);
	lbl_probe_radius->setMinimumHeight(minHeight1);
	lbl_probe_radius->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_probe_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cnt_probe_radius= new QwtCounter(this);
	Q_CHECK_PTR(cnt_probe_radius);
	cnt_probe_radius->setRange(0, 10, 0.01);
	cnt_probe_radius->setValue(probe_radius);
	cnt_probe_radius->setMinimumHeight(minHeight1);
	cnt_probe_radius->setEnabled(true);
	cnt_probe_radius->setNumButtons(3);
	cnt_probe_radius->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_probe_radius, SIGNAL(valueChanged(double)), SLOT(update_probe_radius(double)));

	lbl_asa_threshold = new QLabel(tr(" ASA Threshold (A^2)): "), this);
	Q_CHECK_PTR(lbl_asa_threshold);
	lbl_asa_threshold->setAlignment(AlignLeft|AlignVCenter);
	lbl_asa_threshold->setMinimumHeight(minHeight1);
	lbl_asa_threshold->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_asa_threshold->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cnt_asa_threshold= new QwtCounter(this);
	Q_CHECK_PTR(cnt_asa_threshold);
	cnt_asa_threshold->setRange(0, 100, 0.1);
	cnt_asa_threshold->setValue(asa_threshold);
	cnt_asa_threshold->setMinimumHeight(minHeight1);
	cnt_asa_threshold->setEnabled(true);
	cnt_asa_threshold->setNumButtons(3);
	cnt_asa_threshold->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_asa_threshold, SIGNAL(valueChanged(double)), SLOT(update_asa_threshold(double)));

	lbl_asa_threshold_percent = new QLabel(tr(" Bead ASA Threshold %: "), this);
	Q_CHECK_PTR(lbl_asa_threshold_percent);
	lbl_asa_threshold_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_asa_threshold_percent->setMinimumHeight(minHeight1);
	lbl_asa_threshold_percent->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_asa_threshold_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cnt_asa_threshold_percent= new QwtCounter(this);
	Q_CHECK_PTR(cnt_asa_threshold_percent);
	cnt_asa_threshold_percent->setRange(0, 100, 0.1);
	cnt_asa_threshold_percent->setValue(asa_threshold_percent);
	cnt_asa_threshold_percent->setMinimumHeight(minHeight1);
	cnt_asa_threshold_percent->setEnabled(true);
	cnt_asa_threshold_percent->setNumButtons(3);
	cnt_asa_threshold_percent->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_asa_threshold_percent, SIGNAL(valueChanged(double)), SLOT(update_asa_threshold_percent(double)));

	cb_asa_calculation = new QCheckBox(this);
	cb_asa_calculation->setText(tr(" Perform ASA Calculation "));
	cb_asa_calculation->setChecked(asa_calculation);
	cb_asa_calculation->setEnabled(true);
	cb_asa_calculation->setMinimumHeight(minHeight1);
	cb_asa_calculation->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_asa_calculation->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_asa_calculation, SIGNAL(clicked()), SLOT(set_asa_calculation()));

	cb_bead_check = new QCheckBox(this);
	cb_bead_check->setText(tr(" Re-check bead ASA "));
	cb_bead_check->setChecked(recheck_beads);
	cb_bead_check->setEnabled(true);
	cb_bead_check->setMinimumHeight(minHeight1);
	cb_bead_check->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_bead_check->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_bead_check, SIGNAL(clicked()), SLOT(set_bead_check()));

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

	lbl_tabletabs = new QLabel(tr("Modify Lookup Tables:"), this);
	Q_CHECK_PTR(lbl_tabletabs);
	lbl_tabletabs->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_tabletabs->setAlignment(AlignCenter|AlignVCenter);
	lbl_tabletabs->setMinimumHeight(minHeight1);
	lbl_tabletabs->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_tabletabs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_output = new QLabel(tr("Output Options:"), this);
	Q_CHECK_PTR(lbl_output);
	lbl_output->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_output->setAlignment(AlignCenter|AlignVCenter);
	lbl_output->setMinimumHeight(minHeight1);
	lbl_output->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_output->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	bg_output = new QButtonGroup(3, Qt::Horizontal, "Output Format:", this);
	bg_output->setExclusive(true);
	connect(bg_output, SIGNAL(clicked(int)), this, SLOT(select_output(int)));
	
	cb_somo_output = new QCheckBox(bg_output);
	cb_somo_output->setText(tr(" SOMO "));
	cb_somo_output->setEnabled(true);
	cb_somo_output->setMinimumHeight(minHeight1);
	cb_somo_output->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_somo_output->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_beams_output = new QCheckBox(bg_output);
	cb_beams_output->setText(tr(" BEAMS "));
	cb_beams_output->setEnabled(true);
	cb_beams_output->setMinimumHeight(minHeight1);
	cb_beams_output->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_beams_output->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_hydro_output = new QCheckBox(bg_output);
	cb_hydro_output->setText(tr(" HYDRO "));
	cb_hydro_output->setEnabled(true);
	cb_hydro_output->setMinimumHeight(minHeight1);
	cb_hydro_output->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_hydro_output->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	bg_output->setButton(output);
	
	bg_sequence = new QButtonGroup(3, Qt::Vertical, "Bead Sequence Format:", this);
	bg_sequence->setExclusive(true);
	connect(bg_sequence, SIGNAL(clicked(int)), this, SLOT(select_sequence(int)));
	
	cb_pdb_sequence = new QCheckBox(bg_sequence);
	cb_pdb_sequence->setText(tr(" as in original PDB  "));
	cb_pdb_sequence->setEnabled(true);
	cb_pdb_sequence->setMinimumHeight(minHeight1);
	cb_pdb_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_pdb_sequence->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_chain_sequence = new QCheckBox(bg_sequence);
	cb_chain_sequence->setText(tr(" exposed side chain -> exposed main chain -> buried "));
	cb_chain_sequence->setEnabled(true);
	cb_chain_sequence->setMinimumHeight(minHeight1);
	cb_chain_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_chain_sequence->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_correspondence_sequence = new QCheckBox(bg_sequence);
	cb_correspondence_sequence->setText(tr(" include bead - original residue correspondence "));
	cb_correspondence_sequence->setEnabled(true);
	cb_correspondence_sequence->setMinimumHeight(minHeight1);
	cb_correspondence_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_correspondence_sequence->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	bg_sequence->setButton(sequence);
	
	cb_vbar = new QCheckBox(this);
	cb_vbar->setText(tr(" Calculate vbar "));
	cb_vbar->setChecked(compute_vbar);
	cb_vbar->setMinimumHeight(minHeight1);
	cb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_vbar->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_vbar, SIGNAL(clicked()), SLOT(set_vbar()));

	pb_vbar = new QPushButton(tr("Select vbar"), this);
	Q_CHECK_PTR(pb_vbar);
	pb_vbar->setEnabled(!compute_vbar);
	pb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_vbar->setMinimumHeight(minHeight1);
	pb_vbar->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_vbar, SIGNAL(clicked()), SLOT(select_vbar()));
	
	le_vbar = new QLineEdit(this, "vbar Line Edit");
	le_vbar->setMinimumHeight(minHeight1);
	le_vbar->setEnabled(!compute_vbar);
	le_vbar->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_vbar(const QString &)));

	le_output_file = new QLineEdit(this, "output_file Line Edit");
	le_output_file->setMinimumHeight(minHeight1);
	le_output_file->setEnabled(true);
	le_output_file->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_output_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(le_output_file, SIGNAL(textChanged(const QString &)), SLOT(update_output_file(const QString &)));

	pb_select_output_file = new QPushButton(tr("Select Output File"), this);
	Q_CHECK_PTR(pb_select_output_file);
	pb_select_output_file->setMinimumHeight(minHeight1);
	pb_select_output_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_select_output_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_select_output_file, SIGNAL(clicked()), SLOT(select_output_file()));

	pb_reset = new QPushButton(tr("Reset to Default Options"), this);
	Q_CHECK_PTR(pb_reset);
	pb_reset->setMinimumHeight(minHeight1);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

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

	pb_hybrid = new QPushButton(tr("Add/Edit Hybridization"), this);
	Q_CHECK_PTR(pb_hybrid);
	pb_hybrid->setMinimumHeight(minHeight1);
	pb_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_hybrid->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_hybrid, SIGNAL(clicked()), SLOT(hybrid()));

	int rows=7, columns = 4, spacing = 2, j=0, margin=4;
	QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);
	
	background->addWidget(pb_select_residue_file, j, 0);
	background->addWidget(lbl_table, j, 1);
	background->addMultiCellWidget(lbl_info, j, j, 2, 4);
	j++;
	background->addWidget(pb_load_pdb, j, 0);
	background->addWidget(lbl_pdb_file, j, 1);
	background->addMultiCellWidget(tw_overlap, j, j+5, 2, 4);
	j++;
	background->addWidget(lbl_model, j, 0);
	background->addMultiCellWidget(lb_model, j, j+3, 1, 1);
	j+=4;
	background->addWidget(lbl_probe_radius, j, 0);
	background->addWidget(cnt_probe_radius, j, 1);
	j++;
	background->addWidget(lbl_asa_threshold, j, 0);
	background->addWidget(cnt_asa_threshold, j, 1);
	background->addMultiCellWidget(lbl_output, j, j, 2, 4);
	j++;
	background->addWidget(lbl_asa_threshold_percent, j, 0);
	background->addWidget(cnt_asa_threshold_percent, j, 1);
	background->addMultiCellWidget(bg_output, j, j+1, 2, 4);
	j++;
	background->addWidget(cb_asa_calculation, j, 0);
	background->addWidget(cb_bead_check, j, 1);
	j++;
	background->addWidget(cb_vbar, j, 0);
	background->addMultiCellWidget(bg_sequence, j, j+3, 2, 4);
	j++;
	background->addWidget(pb_vbar, j, 0);
	background->addWidget(le_vbar, j, 1);
	j++;
	background->addWidget(pb_somo, j, 0);
	background->addWidget(pb_visualize, j, 1);
	j++;
	j++;
	background->addWidget(pb_select_output_file, j, 2);
	background->addWidget(le_output_file, j, 3);
	background->addWidget(pb_reset, j, 4);
	j++;
	background->addMultiCellWidget(lbl_tabletabs, j, j, 2, 4);
	j++;
	background->addWidget(pb_help, j, 0);
	background->addWidget(pb_cancel, j, 1);
	background->addWidget(pb_atom, j, 2);
	background->addWidget(pb_residue, j, 3);
	background->addWidget(pb_hybrid, j, 4);
}

int US_Hydrodyn::compute_asa()
{
       // run the surfracer code

       int retval = surfracer_main(probe_radius, residue_list, &model_vector);

       if ( retval ) 
       {
	 switch ( retval ) 
	 {
	
	 case US_SURFRACER_ERR_MISSING_RESIDUE :
	   return US_SURFRACER_ERR_MISSING_RESIDUE;
	   break;

	 case US_SURFRACER_ERR_MISSING_ATOM :
	   return US_SURFRACER_ERR_MISSING_ATOM;
	   break;

	 case US_SURFRACER_ERR_MEMORY_ALLOC :
	   return US_SURFRACER_ERR_MEMORY_ALLOC;
	   break;

	 default :
	   // unknown error
	   return -1;
	   break;

	 }
       }

       // pass 1 assign bead #'s, chain #'s, initialize data

       for (unsigned int i = 0; i < model_vector.size (); i++)
	 {
	   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++)
	     {
	       for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++)
		 {
		   PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

		   this_atom->bead_assignment = 
		     (this_atom->p_atom ? (int) this_atom->p_atom->bead_assignment : -1);
		   this_atom->chain = 
		     ((this_atom->p_residue && this_atom->p_atom) ? 
		      (int) this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].chain : -1);

		   // initialize data
		   this_atom->bead_positioner = false;

		   for (unsigned int m = 0; m < 3; m++) 
		     {
		       this_atom->bead_cog_coordinate.axis[m] = 0;
		       this_atom->bead_position_coordinate.axis[m] = 0;
		       this_atom->bead_coordinate.axis[m] = 0;
		     }
		 }
	     }
	 }

       // pass 2 determine beads, cog_position, fixed_position

       int last_bead_assignment = -1;
       int last_chain = -1;
       QString last_resName = "not a residue";
       PDB_atom *last_main_chain_bead = (PDB_atom *) 0;
       PDB_atom *last_main_bead = (PDB_atom *) 0;
       int count_actives;

       for (unsigned int i = 0; i < model_vector.size (); i++)
	 {
	   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++)
	     {
	       count_actives = 0;
	       for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++)
		 {
		   PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
		   this_atom->bead_positioner = false;
		   if (this_atom->active)
		     {
		       printf("pass 2 active %s %s %d\n",
			      this_atom->name.ascii(), 
			      this_atom->resName.ascii(),
			      this_atom->serial); fflush(stdout);

		       this_atom->mw = this_atom->p_atom->hybrid.mw;

		       // do we have a new bead?
		       // we want to put the N on a previous bead unless it is the first one of the molecule

		       if ((this_atom->bead_assignment != last_bead_assignment ||
			    this_atom->chain != last_chain ||
			    this_atom->resName != last_resName) &&
			    !(this_atom->chain == 0 &&
			      this_atom->name == "N" &&
			      count_actives))
			 {
			   printf("pass 2 active %s %s %d new bead\n",
				  this_atom->name.ascii(), 
				  this_atom->resName.ascii(),
				  this_atom->serial); fflush(stdout);

			   this_atom->bead_positioner = this_atom->p_atom->positioner;

			   this_atom->is_bead = true;
			   last_main_bead = this_atom;
			   last_bead_assignment = this_atom->bead_assignment;
			   last_chain = this_atom->chain;
			   last_resName = this_atom->resName;
			 } else {
			   printf("pass 2 active %s %s %d not a new bead\n",
				  this_atom->name.ascii(), 
				  this_atom->resName.ascii(),
				  this_atom->serial); fflush(stdout);
			   if (this_atom->p_atom->positioner)
			     {
			       if (last_main_bead->bead_positioner)
				 {
				   fprintf(stderr, "warning: 2 positioners in bead %s %s %d\n", 
					   last_main_bead->name.ascii(), 
					   last_main_bead->resName.ascii(),
					   last_main_bead->serial);
				 }
			       last_main_bead->bead_positioner = true;
			       last_main_bead->bead_position_coordinate = this_atom->coordinate;
			     }

			   this_atom->is_bead = false;
			   this_atom->bead_mw = 0;
			 }

		       this_atom->bead_asa = 0;
		       this_atom->bead_mw = 0;

		       // special nitrogen asa handling
		       PDB_atom *use_atom;
		       if (this_atom->chain == 0 &&
			   this_atom->name == "N" &&
			   last_main_chain_bead) 
			 {
			   use_atom = last_main_chain_bead;
			 } else {
			   use_atom = last_main_bead;
			 }

		       use_atom->bead_asa += this_atom->asa;
		       use_atom->bead_mw += this_atom->p_atom->hybrid.mw;

		       // accum 
		       for (unsigned int m = 0; m < 3; m++) 
			 {
			   use_atom->bead_cog_coordinate.axis[m] += 
			     this_atom->coordinate.axis[m] * this_atom->p_atom->hybrid.mw;
			 }
		       if (this_atom->p_atom->positioner)
			 {
			   if (use_atom->bead_positioner)
			     {
			       fprintf(stderr, "warning: 2 positioners in bead %s %s %d\n", 
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
			   this_atom->name == "CA")
			 {
			   last_main_chain_bead = this_atom;
			 }

		       count_actives++;
		   
		     } else {
		       this_atom->is_bead = false;
		     }
		 }
	     }
	 }

       // pass 3 determine visibility, exposed code, normalize cog position, final position determination

       printf("pass 3\n"); fflush(stdout);

       for (unsigned int i = 0; i < model_vector.size (); i++)
	 {
	   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++)
	     {
	       for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++)
		 {
		   PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
		   this_atom->exposed_code = -1;
		   if (this_atom->active &&
		       this_atom->is_bead)
		     {
		       printf("pass 3 active is bead %s %s %d\n",
			      this_atom->name.ascii(), 
			      this_atom->resName.ascii(),
			      this_atom->serial); fflush(stdout);

		       for (unsigned int m = 0; m < 3; m++) 
			 {
			   if (this_atom->bead_mw)
			     {
			       this_atom->bead_cog_coordinate.axis[m] /= this_atom->bead_mw;
			     } else {
			       this_atom->bead_cog_coordinate.axis[m] = 0;
			     }
			 }

		       if (this_atom->p_residue && this_atom->p_atom)
			 {
			   printf("pass 3 active is bead %s %s %d bead assignment %d placing method %d\n",
				  this_atom->name.ascii(), 
				  this_atom->resName.ascii(),
				  this_atom->serial,
				  this_atom->bead_assignment,
				  this_atom->placing_method
				  ); fflush(stdout);
			   switch (this_atom->placing_method) 
			     {
			     case 0 : // cog
			       this_atom->bead_coordinate = this_atom->bead_cog_coordinate;
			       if (this_atom->bead_positioner)
				 {
				   fprintf(stderr, "warning: this bead had a atom claiming position & a bead placing method of cog! %s %s %d\n",
					   this_atom->name.ascii(), 
					   this_atom->resName.ascii(),
					   this_atom->serial);
				 }
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
		       printf("pass 3 active is bead %s %s %d checkpoint 1\n",
			      this_atom->name.ascii(), 
			      this_atom->resName.ascii(),
			      this_atom->serial); fflush(stdout);
		     
		       this_atom->visibility = (this_atom->bead_asa >= asa_threshold);

		       printf("pass 3 active is bead %s %s %d checkpoint 2\n",
			      this_atom->name.ascii(), 
			      this_atom->resName.ascii(),
			      this_atom->serial); fflush(stdout);
		       if (this_atom->visibility)
			 {
			   this_atom->exposed_code = 1;  // exposed
			 } else {
			   if (this_atom->chain == 0) 
			     {
			       this_atom->exposed_code = 10;  // main chain, buried
			     }
			   if (this_atom->chain == 1) 
			     {
			       this_atom->exposed_code = 6;   // side chain, buried
			     }
			 }
		       printf("pass 3 active is bead %s %s %d checkpoint 3\n",
			      this_atom->name.ascii(), 
			      this_atom->resName.ascii(),
			      this_atom->serial); fflush(stdout);
		     } else {
		       this_atom->placing_method = -1;
		     }
		 }
	     }
	 }

       // pass 4 print results

       printf("model~molecule~atom~name~residue~position~active~radius~asa~mw~bead #~chain~serial~is_bead~ bead_asa~visible~code/color~bead mw~position controlled?~baric method~position_coordinate~cog position~use position\n");
       for (unsigned int i = 0; i < model_vector.size (); i++)
	 {
	   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++)
	     {
	       for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++)
		 {
		   PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

		   //		   printf("model %d mol %d atm %d nam %s res %s xyz [%f,%f,%f] act %s rads %f asa %f bead # %d chain %d serl %d is_bead %s bead_asa %f vis %s code %d pos? %s pos_co [%f,%f,%f] cog [%f,%f,%f] use [%f, %f, %f]\n",
		   printf("%d~%d~%d~%s~%s~[%f,%f,%f]~%s~%f~%f~%f~%d~%d~%d~%s~%f~%s~%d~%f~%s~%d~[%f,%f,%f]~[%f,%f,%f]~[%f, %f, %f]\n",			  
			  i, j, k, 
			  this_atom->name.ascii(),
			  this_atom->resName.ascii(),
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
			  this_atom->placing_method,
			  this_atom->bead_position_coordinate.axis[0],
			  this_atom->bead_position_coordinate.axis[1],
			  this_atom->bead_position_coordinate.axis[2],
			  this_atom->bead_cog_coordinate.axis[0],
			  this_atom->bead_cog_coordinate.axis[1],
			  this_atom->bead_cog_coordinate.axis[2],
			  this_atom->bead_coordinate.axis[0],
			  this_atom->bead_coordinate.axis[1],
			  this_atom->bead_coordinate.axis[2]
			  );
		 }
	     }
	 }

       return 0;
}

void US_Hydrodyn::bead_check()
{
	// recheck beads here
}

void US_Hydrodyn::select_model(int val)
{
	current_model = val;
	// check integrity of PDB file and confirm that all residues are correctly defined in residue table
}

void US_Hydrodyn::visualize()
{
	// visualize bead model with rasmol etc.
}

int US_Hydrodyn::calc_somo()
{
	if (recheck_beads)
	{
		bead_check();
	}
	if (asa_calculation)
	{
		compute_asa();
	}

	// calculate bead model and generate hydrodynamics calculation output
	return 0;
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
		addResidue = new US_AddResidue(&residue_widget, 0);
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

void US_Hydrodyn::update_vbar(const QString &str)
{
	vbar = str.toDouble();
}

void US_Hydrodyn::update_vbar_signal(float val1, float val2)
{
	QString str;
	float vbar20;
	vbar20 = val1;
	vbar = val2;
	le_vbar->setText(str.sprintf("%7.4f", vbar));
}

void US_Hydrodyn::update_output_file(const QString &str)
{
	output_filename = str;
}

void US_Hydrodyn::select_vbar()
{
	float val;
	vbar_dlg = new US_Vbar_DB(20.0, &val, &val, true, false, 0);
	vbar_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	vbar_dlg->setCaption(tr("V-bar Calculation"));
	connect(vbar_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_vbar_signal(float, float)));
	vbar_dlg->exec();
}

void US_Hydrodyn::select_output_file()
{
	output_filename = QFileDialog::getSaveFileName(USglobal->config_list.root_dir,
	"Somo files (*.somo)",
	this,
	"save file dialog",
	"Choose a filename to save SOMO file under" );
	if (output_filename.isEmpty()) return;
	if (output_filename.right(5) != ".somo")
	{
		output_filename += ".somo";
	}
	le_output_file->setText(output_filename);
}

void US_Hydrodyn::select_residue_file()
{
	QString old_filename = residue_filename, str1, str2;
	unsigned int numatoms, numbeads, i, j, positioner;
	residue_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.residue *.RESIDUE", this);
	if (residue_filename.isEmpty())
	{
		residue_filename = old_filename;
	}
	else
	{
		lbl_table->setText(residue_filename);
		QFile f(residue_filename);
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
}

void US_Hydrodyn::load_pdb()
{
	QString filename = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.pdb *.PDB", this);
	if (!filename.isEmpty())
	{
		lbl_pdb_file->setText(filename);
		read_pdb(filename);
	}
}

void US_Hydrodyn::read_pdb(const QString &filename)
{
	QString str1, str2, temp;
	model_vector.clear();
	struct PDB_chain temp_chain;
	QFile f(filename);
	struct PDB_model temp_model;
	bool chain_flag = false;
	bool model_flag = false;
	temp_model.molecule.clear();
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
			}
			if (str1.left(6) == "ENDMDL") // we need to save the previously recorded molecule
			{
				temp_model.molecule.push_back(temp_chain);
				model_vector.push_back(temp_model);
				clear_temp_chain(&temp_chain); // we are done with this molecule and can delete it
			}
			if (str1.left(4) == "ATOM" || str1.left(6) == "HETATM") // need to add TER
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
				assign_atom(str1, &temp_chain); // parse the current line and add it to temp_chain
			}
		}
		f.close();
	}
	if(!model_flag)	// there were no model definitions, just a single molecule,
	{						// we still need to save the results
		temp_model.molecule.push_back(temp_chain);
		temp_model.model_id = 1;
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

void US_Hydrodyn::clear_temp_chain(struct PDB_chain *temp_chain) // clear all the memory from the vectors in temp_chain
{
	(*temp_chain).atom.clear();
	(*temp_chain).chainID = "";
	(*temp_chain).segID = "";
}

void US_Hydrodyn::assign_atom(const QString &str1, struct PDB_chain *temp_chain)
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
	struct PDB_atom temp_atom;
	str2 = str1.mid(6, 5);
	temp_atom.serial = str2.toUInt();
	
	str2 = str1.mid(13, 4);
	temp_atom.name = str2.stripWhiteSpace();

	temp_atom.altLoc = str1.mid(16, 1);

	temp_atom.resName = str1.mid(17, 3);

	str2 = str1.mid(22, 4);
	temp_atom.resSeq = str2.toUInt();

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
}

void US_Hydrodyn::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;
	write_config();
	e->accept();
}

void US_Hydrodyn::select_output(int val)
{
	output = val;
}

void US_Hydrodyn::select_sequence(int val)
{
	sequence = val;
}

void US_Hydrodyn::set_vbar()
{
	compute_vbar = cb_vbar->isChecked();
	le_vbar->setEnabled(!compute_vbar);
	pb_vbar->setEnabled(!compute_vbar);
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

void US_Hydrodyn::update_probe_radius(double val)
{
	probe_radius = val;
}

void US_Hydrodyn::update_asa_threshold(double val)
{
	asa_threshold = val;
}

void US_Hydrodyn::update_asa_threshold_percent(double val)
{
	asa_threshold_percent = val;
}

void US_Hydrodyn::set_asa_calculation()
{
	asa_calculation = cb_asa_calculation->isChecked();
}

void US_Hydrodyn::set_bead_check()
{
	recheck_beads = cb_bead_check->isChecked();
}

void US_Hydrodyn::read_config()
{
	QFile f;
	QString str;
	f.setName(USglobal->config_list.root_dir + "/etc/somo.config");
	if (f.open(IO_ReadOnly)) // first try user's directory for default settings
	{
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
		output = str.toInt();
		ts >> str;
		ts.readLine();
		sequence = str.toInt();
		ts >> str;
		ts.readLine();
		compute_vbar = (bool) str.toInt();
		ts >> str;
		ts.readLine();
		probe_radius = str.toFloat();
		ts >> str;
		ts.readLine();
		asa_threshold = str.toFloat();
		ts >> str;
		ts.readLine();
		asa_threshold_percent = str.toFloat();
		ts >> str;
		ts.readLine();
		asa_calculation = (bool) str.toInt();
		ts >> str;
		ts.readLine();
		recheck_beads = (bool) str.toInt();

		f.close();
	}
	else
	{
		f.setName(USglobal->config_list.system_dir + "/etc/somo.config");
		if (f.open(IO_ReadOnly)) // read system directory
		{
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
			output = str.toInt();
			ts >> str;
			ts.readLine();
			sequence = str.toInt();
			ts >> str;
			ts.readLine();
			compute_vbar = (bool) str.toInt();
			ts >> str;
			ts.readLine();
			probe_radius = str.toFloat();
			ts >> str;
			ts.readLine();
			asa_threshold = str.toFloat();
			ts >> str;
			ts.readLine();
			asa_threshold_percent = str.toFloat();
			ts >> str;
			ts.readLine();
			asa_calculation = (bool) str.toInt();
			ts >> str;
			ts.readLine();
			recheck_beads = (bool) str.toInt();

			f.close();
		}
		else // assign default values
		{
			reset();
		}
	}
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
	sidechain_overlap.remove_sync = false;
	sidechain_overlap.remove_sync_percent = 70.0;
	sidechain_overlap.translate_out = true;
	sidechain_overlap.show_translate = true;

	mainchain_overlap.remove_overlap = true;
	mainchain_overlap.fuse_beads = true;
	mainchain_overlap.fuse_beads_percent = 70.0;
	mainchain_overlap.remove_hierarch = true;
	mainchain_overlap.remove_sync = false;
	mainchain_overlap.remove_sync_percent = 70.0;
	mainchain_overlap.translate_out = false;
	mainchain_overlap.show_translate = false;

	buried_overlap.remove_overlap = true;
	buried_overlap.fuse_beads = false;
	buried_overlap.fuse_beads_percent = 0.0;
	buried_overlap.remove_hierarch = true;
	buried_overlap.remove_sync = false;
	buried_overlap.remove_sync_percent = 70.0;
	buried_overlap.translate_out = false;
	buried_overlap.show_translate = false;
	
	sidechain_overlap.title = "exposed side chain beads";
	mainchain_overlap.title = "exposed main/main and main/side chain beads";
	buried_overlap.title = "buried beads";

	sequence = 0;
	output = 0;
	compute_vbar = true;
	probe_radius = 1.4;
	asa_threshold = 10.0;
	asa_threshold_percent = 30.0;
	asa_calculation = true;
	recheck_beads = true;
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
		ts << sidechain_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
		ts << sidechain_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
		ts << sidechain_overlap.translate_out << "\t\t# Outward translation flag\n";
		ts << sidechain_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

		ts << mainchain_overlap.remove_overlap << "\t\t# Remove overlaps flag\n";
		ts << mainchain_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
		ts << mainchain_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
		ts << mainchain_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
		ts << mainchain_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
		ts << mainchain_overlap.remove_sync_percent << "\t\t# percent synchronously step\n";
		ts << mainchain_overlap.translate_out << "\t\t# Outward translation flag\n";
		ts << mainchain_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

		ts << buried_overlap.remove_overlap << "\t\t# Remove overlaps flag\n";
		ts << buried_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
		ts << buried_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
		ts << buried_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
		ts << buried_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
		ts << buried_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
		ts << buried_overlap.translate_out << "\t\t# Outward translation flag\n";
		ts << buried_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

		ts << output << "\t\t# flag for selecting output format\n";
		ts << sequence << "\t\t# flag for selecting sequence format\n";
		ts << compute_vbar << "\t\t# flag for selecting vbar calculation\n";
		ts << probe_radius << "\t\t# probe radius in angstrom\n";
		ts << asa_threshold << "\t\t# ASA threshold\n";
		ts << asa_threshold_percent << "\t\t# ASA threshold percent\n";
		ts << asa_calculation << "\t\t# flag for calculation of ASA\n";
		ts << recheck_beads << "\t\t# flag for rechecking beads\n";

		f.close();
	}
}

