#include "../include/us_hydrodyn_pdb_parsing.h"

US_Hydrodyn_PDB_Parsing::US_Hydrodyn_PDB_Parsing(struct pdb_parsing *pdb,
bool *pdb_parsing_widget, QWidget *p, const char *name) : QFrame(p, name)
{
	this->pdb = pdb;
	this->pdb_parsing_widget = pdb_parsing_widget;
	*pdb_parsing_widget = true;
	USglobal=new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("SOMO PDB Parsing Options"));
	setupGUI();
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_PDB_Parsing::~US_Hydrodyn_PDB_Parsing()
{
	*pdb_parsing_widget = false;
}

void US_Hydrodyn_PDB_Parsing::setupGUI()
{
	int minHeight1 = 30;

	lbl_info = new QLabel(tr("SOMO PDB Parsing Options:"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setMinimumHeight(minHeight1);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	bg_misc = new QButtonGroup(4, Qt::Vertical, "Miscellaneous parsing options:", this);
	bg_misc->setExclusive(false);

	cb_skip_hydrogen = new QCheckBox(bg_misc);
	cb_skip_hydrogen->setText(tr(" Skip hydrogen atoms"));
	cb_skip_hydrogen->setEnabled(false);
	cb_skip_hydrogen->setChecked((*pdb).skip_hydrogen);
	cb_skip_hydrogen->setMinimumHeight(minHeight1);
	cb_skip_hydrogen->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_skip_hydrogen->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_skip_hydrogen, SIGNAL(clicked()), this, SLOT(skip_hydrogen()));

	cb_skip_water = new QCheckBox(bg_misc);
	cb_skip_water->setText(tr(" Skip solvent water molecules"));
	cb_skip_water->setEnabled(false);
	cb_skip_water->setChecked((*pdb).skip_water);
	cb_skip_water->setMinimumHeight(minHeight1);
	cb_skip_water->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_skip_water->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_skip_water, SIGNAL(clicked()), this, SLOT(skip_water()));

	cb_alternate = new QCheckBox(bg_misc);
	cb_alternate->setText(tr(" Skip alternate conformations"));
	cb_alternate->setEnabled(false);
	cb_alternate->setChecked((*pdb).alternate);
	cb_alternate->setMinimumHeight(minHeight1);
	cb_alternate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_alternate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_alternate, SIGNAL(clicked()), this, SLOT(alternate()));

	cb_find_sh = new QCheckBox(bg_misc);
	cb_find_sh->setText(tr(" Find free SH, change residue coding"));
	cb_find_sh->setEnabled(false);
	cb_find_sh->setChecked((*pdb).find_sh);
	cb_find_sh->setMinimumHeight(minHeight1);
	cb_find_sh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_find_sh->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_find_sh, SIGNAL(clicked()), this, SLOT(find_sh()));

	bg_residues = new QButtonGroup(3, Qt::Vertical, "If non-coded residues are found:", this);
	bg_residues->setExclusive(true);
	connect(bg_residues, SIGNAL(clicked(int)), this, SLOT(residue(int)));

	cb_residue_stop = new QCheckBox(bg_residues);
	cb_residue_stop->setText(tr(" List them and stop operation"));
	cb_residue_stop->setEnabled(true);
	cb_residue_stop->setMinimumHeight(minHeight1);
	cb_residue_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_residue_stop->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_residue_skip = new QCheckBox(bg_residues);
	cb_residue_skip->setText(tr(" List them, skip residue and proceed"));
	cb_residue_skip->setEnabled(true);
	cb_residue_skip->setMinimumHeight(minHeight1);
	cb_residue_skip->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_residue_skip->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_residue_auto = new QCheckBox(bg_residues);
	cb_residue_auto->setText(tr(" Use automatic bead builder (approximate method)"));
	cb_residue_auto->setEnabled(true);
	cb_residue_auto->setMinimumHeight(minHeight1);
	cb_residue_auto->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_residue_auto->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	bg_residues->setButton((*pdb).missing_residues);

	bg_atoms = new QButtonGroup(3, Qt::Vertical, "If missing atoms within a residue are found:", this);
	bg_atoms->setExclusive(true);
	connect(bg_atoms, SIGNAL(clicked(int)), this, SLOT(atom(int)));

	cb_atom_stop = new QCheckBox(bg_atoms);
	cb_atom_stop->setText(tr(" List them and stop operation (fix with WHATIF)"));
	cb_atom_stop->setEnabled(true);
	cb_atom_stop->setMinimumHeight(minHeight1);
	cb_atom_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_atom_stop->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_atom_skip = new QCheckBox(bg_atoms);
	cb_atom_skip->setText(tr(" List them, skip entire residue and proceed"));
	cb_atom_skip->setEnabled(true);
	cb_atom_skip->setMinimumHeight(minHeight1);
	cb_atom_skip->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_atom_skip->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_atom_auto = new QCheckBox(bg_atoms);
	cb_atom_auto->setText(tr(" Use approximate method to generate bead"));
	cb_atom_auto->setEnabled(false);
	cb_atom_auto->setMinimumHeight(minHeight1);
	cb_atom_auto->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_atom_auto->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	bg_atoms->setButton((*pdb).missing_atoms);

	pb_cancel = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_cancel);
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_cancel->setMinimumHeight(minHeight1);
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_help->setMinimumHeight(minHeight1);
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	int rows=11, columns = 2, spacing = 2, j=0, margin=4;
	QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

	background->addMultiCellWidget(lbl_info, j, j, 0, 1);
	j++;
	background->addMultiCellWidget(bg_misc, j, j, 0, 1);
	j++;
	background->addMultiCellWidget(bg_residues, j, j, 0, 1);
	j++;
	background->addMultiCellWidget(bg_atoms, j, j, 0, 1);
	j++;
	background->addWidget(pb_help, j, 0);
	background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_PDB_Parsing::skip_hydrogen()
{
	(*pdb).skip_hydrogen = cb_skip_hydrogen->isChecked();
}

void US_Hydrodyn_PDB_Parsing::skip_water()
{
	(*pdb).skip_water = cb_skip_water->isChecked();
}

void US_Hydrodyn_PDB_Parsing::alternate()
{
	(*pdb).alternate = cb_alternate->isChecked();
}

void US_Hydrodyn_PDB_Parsing::find_sh()
{
	(*pdb).find_sh = cb_find_sh->isChecked();
}

void US_Hydrodyn_PDB_Parsing::residue(int val)
{
	(*pdb).missing_residues = val;
}

void US_Hydrodyn_PDB_Parsing::atom(int val)
{
	(*pdb).missing_atoms = val;
}

void US_Hydrodyn_PDB_Parsing::cancel()
{
	close();
}

void US_Hydrodyn_PDB_Parsing::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/somo_pdb_parsing.html");
}

void US_Hydrodyn_PDB_Parsing::closeEvent(QCloseEvent *e)
{
	*pdb_parsing_widget = false;
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

