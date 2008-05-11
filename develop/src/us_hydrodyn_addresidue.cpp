#include "../include/us_hydrodyn_addresidue.h"
#include "../include/color_black.xpm"
#include "../include/color_blue.xpm"
#include "../include/color_brightwhite.xpm"
#include "../include/color_brown.xpm"
#include "../include/color_cyan.xpm"
#include "../include/color_green.xpm"
#include "../include/color_grey.xpm"
#include "../include/color_lightblue.xpm"
#include "../include/color_lightcyan.xpm"
#include "../include/color_lightgreen.xpm"
#include "../include/color_lightmagenta.xpm"
#include "../include/color_lightred.xpm"
#include "../include/color_magenta.xpm"
#include "../include/color_red.xpm"
#include "../include/color_white.xpm"
#include "../include/color_yellow.xpm"



US_AddResidue::US_AddResidue(bool *widget_flag, QWidget *p, const char *name) : QWidget( p, name)
{
	this->widget_flag = widget_flag;
	*widget_flag = true;
	online_help = NULL;
	USglobal = new US_Config();
	position_flag = false;
	existing_residue = false;
	current_atom = 0;
	current_bead = 0;
	atom_filename = USglobal->config_list.system_dir + "/etc/somo.atom";
	residue_filename = USglobal->config_list.system_dir + "/etc/somo.residue";
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("SoMo: Modify Residue Lookup Tables"));
	setupGUI();
	global_Xpos += 30;
	global_Ypos += 30;
	new_residue.type = 0;
	new_bead.color = 0;
	new_bead.placing_method = 0;
	new_bead.hydration = 0;
	new_bead.visibility = 0;
	new_bead.volume = 0;
	setGeometry(global_Xpos, global_Ypos, 0, 0);
	hybrids.clear();
	atoms.clear();
}

US_AddResidue::~US_AddResidue()
{
}

void US_AddResidue::setupGUI()
{
	unsigned int minHeight1 = 30;

	lbl_info1 = new QLabel(tr(" 1: Define Residue Properties: "), this);
	Q_CHECK_PTR(lbl_info1);
	lbl_info1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info1->setMinimumHeight(minHeight1);
	lbl_info1->setAlignment(AlignCenter|AlignVCenter);
	lbl_info1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	pb_select_atom_file = new QPushButton(tr("Load Atom Definition File"), this);
	Q_CHECK_PTR(pb_select_atom_file);
	pb_select_atom_file->setMinimumHeight(minHeight1);
	pb_select_atom_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_select_atom_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_select_atom_file, SIGNAL(clicked()), SLOT(select_atom_file()));

	lbl_atom_file = new QLabel(tr(" not selected"),this);
	lbl_atom_file->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_atom_file->setMinimumHeight(minHeight1);
	lbl_atom_file->setAlignment(AlignLeft|AlignVCenter);
	lbl_atom_file->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_atom_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	pb_select_residue_file = new QPushButton(tr("Load Residue Definition File"), this);
	Q_CHECK_PTR(pb_select_residue_file);
	pb_select_residue_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_select_residue_file->setEnabled(false);
	pb_select_residue_file->setMinimumHeight(minHeight1);
	pb_select_residue_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_select_residue_file, SIGNAL(clicked()), SLOT(select_residue_file()));

	lbl_residue_file = new QLabel(tr(" not selected"),this);
	lbl_residue_file->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_residue_file->setAlignment(AlignLeft|AlignVCenter);
	lbl_residue_file->setMinimumHeight(minHeight1);
	lbl_residue_file->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_residue_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_numresidues = new QLabel(tr(" Number of Residues in File: 0"), this);
	Q_CHECK_PTR(lbl_numresidues);
	lbl_numresidues->setAlignment(AlignLeft|AlignVCenter);
	lbl_numresidues->setMinimumHeight(minHeight1);
	lbl_numresidues->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_numresidues->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	lb_residues = new QListBox(this, "Residue Listing" );
	lb_residues->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_residues->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	lb_residues->setMinimumHeight(minHeight1);
	connect(lb_residues, SIGNAL(selected(int)), this, SLOT(select_residue(int)));

	lbl_residue_name = new QLabel(tr(" Residue Name:"), this);
	Q_CHECK_PTR(lbl_residue_name);
	lbl_residue_name->setAlignment(AlignLeft|AlignVCenter);
	lbl_residue_name->setMinimumHeight(minHeight1);
	lbl_residue_name->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_residue_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_residue_name = new QLineEdit(this, "Residue name Line Edit");
	le_residue_name->setMinimumHeight(minHeight1);
	le_residue_name->setEnabled(false);
	le_residue_name->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_residue_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(le_residue_name, SIGNAL(textChanged(const QString &)), SLOT(update_name(const QString &)));

	lbl_numatoms = new QLabel(tr(" Number of Atoms in Residue:"), this);
	Q_CHECK_PTR(lbl_numatoms);
	lbl_numatoms->setAlignment(AlignLeft|AlignVCenter);
	lbl_numatoms->setMinimumHeight(minHeight1);
	lbl_numatoms->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_numatoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cnt_numatoms= new QwtCounter(this);
	Q_CHECK_PTR(cnt_numatoms);
	cnt_numatoms->setRange(0, 1000, 1);
	cnt_numatoms->setValue(0);
	cnt_numatoms->setMinimumHeight(minHeight1);
	cnt_numatoms->setEnabled(false);
	cnt_numatoms->setNumButtons(3);
	cnt_numatoms->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_numatoms, SIGNAL(valueChanged(double)), SLOT(update_numatoms(double)));

	lbl_numbeads = new QLabel(tr(" Number of Beads for Residue:"), this);
	Q_CHECK_PTR(lbl_numbeads);
	lbl_numbeads->setAlignment(AlignLeft|AlignVCenter);
	lbl_numbeads->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_numbeads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));
	lbl_numbeads->setMinimumHeight(minHeight1);

	cnt_numbeads= new QwtCounter(this);
	Q_CHECK_PTR(cnt_numbeads);
	cnt_numbeads->setRange(0, 1000, 1);
	cnt_numbeads->setValue(0);
	cnt_numbeads->setEnabled(false);
	cnt_numbeads->setNumButtons(3);
	cnt_numbeads->setMinimumHeight(minHeight1);
	cnt_numbeads->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_numbeads, SIGNAL(valueChanged(double)), SLOT(update_numbeads(double)));

	lbl_type = new QLabel(tr(" Residue Type:"), this);
	Q_CHECK_PTR(lbl_type);
	lbl_type->setMinimumHeight(minHeight1);
	lbl_type->setAlignment(AlignLeft|AlignVCenter);
	lbl_type->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_type->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cmb_type = new QComboBox(false, this, "Bead Color" );
	cmb_type->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_type->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_type->setSizeLimit(6);
	cmb_type->setEnabled(false);
	cmb_type->insertItem("Amino Acid");
	cmb_type->insertItem("Sugar Moiety");
	cmb_type->insertItem("Nucleotide");
	cmb_type->insertItem("Heme");
	cmb_type->insertItem("Phosphate");
	cmb_type->insertItem("Co-factor");
	cmb_type->setMinimumHeight(minHeight1);
	connect(cmb_type, SIGNAL(activated(int)), this, SLOT(select_type(int)));

	lbl_molvol = new QLabel(tr(" Residue Molec. anhydrous Vol. (A^3):"), this);
	Q_CHECK_PTR(lbl_molvol);
	lbl_molvol->setAlignment(AlignLeft|AlignVCenter);
	lbl_molvol->setMinimumHeight(minHeight1);
	lbl_molvol->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_molvol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_molvol = new QLineEdit(this, "Residue molvol Line Edit");
	le_molvol->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_molvol->setMinimumHeight(minHeight1);
	le_molvol->setEnabled(false);
	le_molvol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(le_molvol, SIGNAL(textChanged(const QString &)), SLOT(update_molvol(const QString &)));

	lbl_asa = new QLabel(tr(" Max. Accessible Surface Area (A^2):"), this);
	Q_CHECK_PTR(lbl_asa);
	lbl_asa->setMinimumHeight(minHeight1);
	lbl_asa->setAlignment(AlignLeft|AlignVCenter);
	lbl_asa->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_asa = new QLineEdit(this, "Residue asa Line Edit");
	le_asa->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_asa->setMinimumHeight(minHeight1);
	le_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	le_asa->setEnabled(false);
	connect(le_asa, SIGNAL(textChanged(const QString &)), SLOT(update_asa(const QString &)));

	pb_accept_residue = new QPushButton(tr(" Accept Residue and Continue "), this);
	Q_CHECK_PTR(pb_accept_residue);
	pb_accept_residue->setEnabled(false);
	pb_accept_residue->setMinimumHeight(minHeight1);
	pb_accept_residue->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_accept_residue->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_accept_residue, SIGNAL(clicked()), SLOT(accept_residue()));

	lbl_info2 = new QLabel(tr(" 2. Define Residue Atoms: "), this);
	Q_CHECK_PTR(lbl_info2);
	lbl_info2->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info2->setAlignment(AlignCenter|AlignVCenter);
	lbl_info2->setMinimumHeight(minHeight1);
	lbl_info2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_r_atoms = new QLabel(tr(" Select Residue Atom to be defined:  "), this);
	Q_CHECK_PTR(lbl_r_atoms);
	lbl_r_atoms->setAlignment(AlignLeft|AlignVCenter);
	lbl_r_atoms->setMinimumHeight(minHeight1);
	lbl_r_atoms->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_r_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cmb_r_atoms = new QComboBox(false, this, "Residue's Atom Listing" );
	cmb_r_atoms->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_r_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_r_atoms->setSizeLimit(5);
	cmb_r_atoms->setMinimumHeight(minHeight1);
	connect(cmb_r_atoms, SIGNAL(activated(int)), this, SLOT(select_r_atom(int)));

	lbl_define_atom = new QLabel(tr(" Select Atom from Lookup Table:"), this);
	Q_CHECK_PTR(lbl_define_atom);
	lbl_define_atom->setAlignment(AlignLeft|AlignVCenter);
	lbl_define_atom->setMinimumHeight(minHeight1);
	lbl_define_atom->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_define_atom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cmb_atoms = new QComboBox(false, this, "Atom Listing" );
	cmb_atoms->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_atoms->setSizeLimit(5);
	cmb_atoms->setMinimumHeight(minHeight1);
	connect(cmb_atoms, SIGNAL(activated(int)), this, SLOT(update_hybrid(int)));

	lbl_define_hybrid = new QLabel(tr(" Select Hybridization for Atom:"), this);
	Q_CHECK_PTR(lbl_define_hybrid);
	lbl_define_hybrid->setAlignment(AlignLeft|AlignVCenter);
	lbl_define_hybrid->setMinimumHeight(minHeight1);
	lbl_define_hybrid->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_define_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cmb_hybrids = new QComboBox(false, this, "Hybridization Listing" );
	cmb_hybrids->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_hybrids->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_hybrids->setSizeLimit(5);
	cmb_hybrids->setMinimumHeight(minHeight1);

	lbl_positioning = new QLabel(tr(" Atom determines Position: "), this);
	Q_CHECK_PTR(lbl_positioning);
	lbl_positioning->setMinimumHeight(minHeight1);
	lbl_positioning->setAlignment(AlignLeft|AlignVCenter);
	lbl_positioning->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_positioning->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cb_positioning = new QCheckBox(this);
	cb_positioning->setText(tr(" (Check if true)"));
	cb_positioning->setChecked(position_flag);
	cb_positioning->setEnabled(false);
	cb_positioning->setMinimumHeight(minHeight1);
	cb_positioning->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_positioning->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_positioning, SIGNAL(clicked()), SLOT(set_positioning()));

	pb_accept_atom = new QPushButton(tr("Assign Current Atom"), this);
	Q_CHECK_PTR(pb_accept_atom);
	pb_accept_atom->setEnabled(false);
	pb_accept_atom->setMinimumHeight(minHeight1);
	pb_accept_atom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_accept_atom->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_accept_atom, SIGNAL(clicked()), SLOT(accept_atom()));

	pb_atom_continue = new QPushButton(tr("Continue"), this);
	Q_CHECK_PTR(pb_atom_continue);
	pb_atom_continue->setEnabled(false);
	pb_atom_continue->setMinimumHeight(minHeight1);
	pb_atom_continue->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_atom_continue->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_atom_continue, SIGNAL(clicked()), SLOT(atom_continue()));

	lbl_info3 = new QLabel(tr(" 3. Define Residue Bead Properties: "), this);
	Q_CHECK_PTR(lbl_info3);
	lbl_info3->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info3->setAlignment(AlignCenter|AlignVCenter);
	lbl_info3->setMinimumHeight(minHeight1);
	lbl_info3->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_r_beads = new QLabel(tr(" Select Residue Bead to be defined: "), this);
	Q_CHECK_PTR(lbl_r_beads);
	lbl_r_beads->setMinimumHeight(minHeight1);
	lbl_r_beads->setAlignment(AlignLeft|AlignVCenter);
	lbl_r_beads->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_r_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cmb_r_beads = new QComboBox(false, this, "bead Listing" );
	cmb_r_beads->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_r_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_r_beads->setSizeLimit(5);
	cmb_r_beads->setEnabled(false);
	cmb_r_beads->setMinimumHeight(minHeight1);
	connect(cmb_r_beads, SIGNAL(activated(int)), this, SLOT(select_r_bead(int)));

	lbl_bead_color = new QLabel(tr(" Select Bead Color: "), this);
	Q_CHECK_PTR(lbl_bead_color);
	lbl_bead_color->setMinimumHeight(minHeight1);
	lbl_bead_color->setAlignment(AlignLeft|AlignVCenter);
	lbl_bead_color->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_bead_color->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cmb_bead_color = new QComboBox(false, this, "Bead Color" );
	cmb_bead_color->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_bead_color->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_bead_color->setSizeLimit(5);
	cmb_bead_color->setMinimumHeight(minHeight1);
	cmb_bead_color->insertItem(QPixmap(color_black), " (1)");
	cmb_bead_color->insertItem(QPixmap(color_blue), " (2)");
	cmb_bead_color->insertItem(QPixmap(color_green), " (3)");
	cmb_bead_color->insertItem(QPixmap(color_cyan), " (4)");
	cmb_bead_color->insertItem(QPixmap(color_red), " (5)");
	cmb_bead_color->insertItem(QPixmap(color_magenta), " (6)");
	cmb_bead_color->insertItem(QPixmap(color_brown), " (7)");
	cmb_bead_color->insertItem(QPixmap(color_white), " (8)");
	cmb_bead_color->insertItem(QPixmap(color_grey), " (9)");
	cmb_bead_color->insertItem(QPixmap(color_lightblue), " (10)");
	cmb_bead_color->insertItem(QPixmap(color_lightgreen), " (11)");
	cmb_bead_color->insertItem(QPixmap(color_lightcyan), " (12)");
	cmb_bead_color->insertItem(QPixmap(color_lightred), " (13)");
	cmb_bead_color->insertItem(QPixmap(color_lightmagenta), " (14)");
	cmb_bead_color->insertItem(QPixmap(color_yellow), " (15)");
	cmb_bead_color->insertItem(QPixmap(color_brightwhite), " (16)");
	cmb_bead_color->setEnabled(false);
	connect(cmb_bead_color, SIGNAL(activated(int)), this, SLOT(select_bead_color(int)));

	lbl_hydration = new QLabel(tr(" Hydration Number for Bead: "), this);
	Q_CHECK_PTR(lbl_hydration);
	lbl_hydration->setMinimumHeight(minHeight1);
	lbl_hydration->setAlignment(AlignLeft|AlignVCenter);
	lbl_hydration->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_hydration->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cnt_hydration= new QwtCounter(this);
	Q_CHECK_PTR(cnt_hydration);
	cnt_hydration->setRange(0, 100, 1);
	cnt_hydration->setValue(0);
	cnt_hydration->setMinimumHeight(minHeight1);
	cnt_hydration->setEnabled(true);
	cnt_hydration->setNumButtons(3);
	cnt_hydration->setEnabled(false);
	cnt_hydration->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_hydration, SIGNAL(valueChanged(double)), SLOT(update_hydration(double)));

	lbl_placing = new QLabel(tr(" Select Positioning Method: "), this);
	Q_CHECK_PTR(lbl_placing);
	lbl_placing->setAlignment(AlignLeft|AlignVCenter);
	lbl_placing->setMinimumHeight(minHeight1);
	lbl_placing->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_placing->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cmb_placing = new QComboBox(false, this, "Placing Combo" );
	cmb_placing->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_placing->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_placing->setSizeLimit(5);
	cmb_placing->insertItem("Center of Gravity");
	cmb_placing->insertItem("Farthest Atom");
	cmb_placing->insertItem("No Positioning");
	cmb_placing->setEnabled(false);
	cmb_placing->setMinimumHeight(minHeight1);
	connect(cmb_placing, SIGNAL(activated(int)), this, SLOT(select_placing_method(int)));

	lbl_select_beadatom = new QLabel(tr(" Select Atom for Bead (multi-selection OK): "), this);
	Q_CHECK_PTR(lbl_select_beadatom);
	lbl_select_beadatom->setAlignment(AlignLeft|AlignVCenter);
	lbl_select_beadatom->setMinimumHeight(minHeight1);
	lbl_select_beadatom->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_select_beadatom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	lbl_list_beadatom = new QLabel(tr(" Currently defined Atoms for Bead: "), this);
	Q_CHECK_PTR(lbl_list_beadatom);
	lbl_list_beadatom->setAlignment(AlignLeft|AlignVCenter);
	lbl_list_beadatom->setMinimumHeight(minHeight1);
	lbl_list_beadatom->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_list_beadatom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	lb_select_beadatom = new QListBox(this, "beadatom selection listbox" );
	lb_select_beadatom->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_select_beadatom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	lb_select_beadatom->setEnabled(false);
	lb_select_beadatom->setSelectionMode(QListBox::Extended);
	lb_select_beadatom->setMinimumHeight(minHeight1);
	connect(lb_select_beadatom, SIGNAL(selectionChanged()), this, SLOT(select_beadatom()));

	lb_list_beadatom = new QListBox(this, "beadatom list listbox" );
	lb_list_beadatom->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_list_beadatom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	lb_list_beadatom->setEnabled(false);
	lb_list_beadatom->setSelectionMode(QListBox::Extended);
	lb_list_beadatom->setMinimumHeight(minHeight1);

	lbl_beadchain = new QLabel(tr(" This Bead is part of the: "), this);
	Q_CHECK_PTR(lbl_beadchain);
	lbl_beadchain->setAlignment(AlignLeft|AlignVCenter);
	lbl_beadchain->setMinimumHeight(minHeight1);
	lbl_beadchain->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_beadchain->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	rb_backbone = new QRadioButton(tr("Backbone"), this);
	rb_backbone->setMinimumHeight(minHeight1);
	rb_backbone->setEnabled(false);
	rb_backbone->setChecked(true);
	rb_backbone->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	rb_backbone->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	rb_sidechain = new QRadioButton(tr("Sidechain"), this);
	rb_sidechain->setEnabled(false);
	rb_sidechain->setChecked(false);
	rb_sidechain->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	rb_sidechain->setMinimumHeight(minHeight1);
	rb_sidechain->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	bg_chain = new QButtonGroup(1, Qt::Horizontal, 0);
	bg_chain->setRadioButtonExclusive(true);
	bg_chain->insert(rb_backbone);
	bg_chain->insert(rb_sidechain);
	bg_chain->setMinimumHeight(minHeight1);
	connect(bg_chain, SIGNAL(clicked(int)), SLOT(set_chain(int)));

	lbl_bead_volume = new QLabel(tr(" Bead Volume: "), this);
	Q_CHECK_PTR(lbl_bead_volume);
	lbl_bead_volume->setAlignment(AlignLeft|AlignVCenter);
	lbl_bead_volume->setMinimumHeight(minHeight1);
	lbl_bead_volume->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_bead_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_bead_volume = new QLineEdit(this, "Residue asa Line Edit");
	le_bead_volume->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_bead_volume->setMinimumHeight(minHeight1);
	le_bead_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	le_bead_volume->setEnabled(false);
	connect(le_bead_volume, SIGNAL(textChanged(const QString &)), SLOT(update_bead_volume(const QString &)));

	pb_accept_bead = new QPushButton(tr("Accept Bead Definition"), this);
	Q_CHECK_PTR(pb_accept_bead);
	pb_accept_bead->setMinimumHeight(minHeight1);
	pb_accept_bead->setEnabled(false);
	pb_accept_bead->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_accept_bead->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_accept_bead, SIGNAL(clicked()), SLOT(accept_bead()));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setMinimumHeight(minHeight1);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_reset = new QPushButton(tr("Reset"), this);
	Q_CHECK_PTR(pb_reset);
	pb_reset->setMinimumHeight(minHeight1);
	pb_reset->setEnabled(false);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

	pb_add = new QPushButton(tr("Add Residue to File"), this);
	Q_CHECK_PTR(pb_add);
	pb_add->setEnabled(false);
	pb_add->setMinimumHeight(minHeight1);
	pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_add->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_add, SIGNAL(clicked()), SLOT(add()));

	pb_close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_close);
	pb_close->setMinimumHeight(minHeight1);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_close, SIGNAL(clicked()), SLOT(close()));

	int rows=3, columns = 5, spacing = 2, j=0, margin=4, colspace=10;
	QGridLayout *background = new QGridLayout(this, rows, columns, margin, spacing);

	background->setColSpacing(2, colspace);
//section 1
	background->addMultiCellWidget(lbl_info1, j, j, 0, 1);
	j++;
	background->addWidget(pb_select_atom_file, j, 0);
	background->addWidget(lbl_atom_file, j, 1);
	j++;
	background->addWidget(pb_select_residue_file, j, 0);
	background->addWidget(lbl_residue_file, j, 1);
	j++;
	background->addWidget(lbl_residue_name, j, 0);
	background->addWidget(le_residue_name, j, 1);
	j++;
	background->addWidget(lbl_numatoms, j, 0);
	background->addWidget(cnt_numatoms, j, 1);
	j++;
	background->addWidget(lbl_numbeads, j, 0);
	background->addWidget(cnt_numbeads, j, 1);
	j++;
	background->addWidget(lbl_type, j, 0);
	background->addWidget(cmb_type, j, 1);
	j++;
	background->addWidget(lbl_molvol, j, 0);
	background->addWidget(le_molvol, j, 1);
	j++;
	background->addWidget(lbl_asa, j, 0);
	background->addWidget(le_asa, j, 1);
	j++;
	background->addWidget(lbl_numresidues, j, 0);
	background->addMultiCellWidget(lb_residues, j, j+1, 1, 1);
	j++;
	background->addWidget(pb_accept_residue, j, 0);
	j++;
//section 2
	background->addMultiCellWidget(lbl_info2, j, j, 0, 1);
	j++;
	background->addWidget(lbl_r_atoms, j, 0);
	background->addWidget(cmb_r_atoms, j, 1);
	j++;
	background->addWidget(lbl_define_atom, j, 0);
	background->addWidget(cmb_atoms, j, 1);
	j++;
	background->addWidget(lbl_define_hybrid, j, 0);
	background->addWidget(cmb_hybrids, j, 1);
	j++;
	background->addWidget(lbl_positioning, j, 0);
	background->addWidget(cb_positioning, j, 1);
	j++;
	background->addWidget(pb_accept_atom, j, 0);
	background->addWidget(pb_atom_continue, j, 1);
	j = 0;
//section 3
	background->addMultiCellWidget(lbl_info3, j, j, 3, 4);
	j++;
	background->addWidget(lbl_r_beads, j, 3);
	background->addWidget(cmb_r_beads, j, 4);
	j++;
	background->addWidget(lbl_bead_color, j, 3);
	background->addWidget(cmb_bead_color, j, 4);
	j++;
	background->addWidget(lbl_hydration, j, 3);
	background->addWidget(cnt_hydration, j, 4);
	j++;
	background->addWidget(lbl_placing, j, 3);
	background->addWidget(cmb_placing, j, 4);
	j++;
	background->addWidget(lbl_beadchain, j, 3);
	QBoxLayout *bl = new QHBoxLayout(0);
	bl->addWidget(rb_backbone);
	bl->addWidget(rb_sidechain);
	background->addLayout(bl, j, 4);
	j++;
	background->addWidget(lbl_bead_volume, j, 3);
	background->addWidget(le_bead_volume, j, 4);
	j++;
	background->addWidget(lbl_list_beadatom, j, 3);
	background->addWidget(lbl_select_beadatom, j, 4);
	j++;
	background->addMultiCellWidget(lb_list_beadatom, j, j+5, 3, 3);
	background->addMultiCellWidget(lb_select_beadatom, j, j+6, 4, 4);
	j+=6;
	background->addWidget(pb_accept_bead, j, 3);
	j++;
	background->addWidget(pb_add, j, 3);
	background->addWidget(pb_reset, j, 4);
	j++;
	background->addWidget(pb_help, j, 3);
	background->addWidget(pb_close, j, 4);
}

void US_AddResidue::add()
{
	int item = -1;
	unsigned int i, j;
	QString str1;
	float sum=0.0;
	for (i=0; i<new_residue.r_bead.size(); i++)
	{
		sum += new_residue.r_bead[i].volume;
	}
	str1.sprintf("Residue volume: %f A^3, Sum of beads: %f A^3\n\nPlease correct the bead volume and try again...", new_residue.molvol, sum);
	if (sum != new_residue.molvol)
	{
		QMessageBox::message("Attention:", "The residue volume does not match the volume of the beads:\n\n" + str1);
		return;
	}
	for (i=0; i<residue_list.size(); i++)
	{
		if (residue_list[i].name.upper() == new_residue.name.upper())
		{
			item = (int) i;
			residue_list[i].type = new_residue.type;
			residue_list[i].molvol = new_residue.molvol;
			residue_list[i].asa = new_residue.asa;
			residue_list[i].r_atom.clear();
			residue_list[i].r_bead.clear();
			for (j=0; j<new_residue.r_atom.size(); j++)
			{
				residue_list[i].r_atom.push_back(new_residue.r_atom[j]);
			}
			for (j=0; j<new_residue.r_bead.size(); j++)
			{
				residue_list[i].r_bead.push_back(new_residue.r_bead[j]);
			}
		}
	}
	if (item < 0)
	{
		residue_list.push_back(new_residue);
	}
	QFile f(residue_filename);
	if (f.open(IO_WriteOnly|IO_Translate))
	{
		lb_residues->clear();
		QTextStream ts(&f);
		for (unsigned int i=0; i<residue_list.size(); i++)
		{
			ts << residue_list[i].name.upper()
				<< "\t" << residue_list[i].type
				<< "\t" << residue_list[i].molvol
				<< "\t" << residue_list[i].asa
				<< "\t" << residue_list[i].r_atom.size()
				<< "\t" << residue_list[i].r_bead.size() << endl;
			for (unsigned int j=0; j<residue_list[i].r_atom.size(); j++)
			{
				ts << residue_list[i].r_atom[j].name.upper()
					<< "\t" << residue_list[i].r_atom[j].hybrid.mw
					<< "\t" << residue_list[i].r_atom[j].hybrid.radius
					<< "\t" << residue_list[i].r_atom[j].bead_assignment
					<< "\t" << (unsigned int) residue_list[i].r_atom[j].positioner
					<< "\t" << residue_list[i].r_atom[j].serial_number
					<< "\t" << residue_list[i].r_atom[j].chain << endl;
			}
			for (unsigned int j=0; j<residue_list[i].r_bead.size(); j++)
			{
				ts << residue_list[i].r_bead[j].hydration
					<< "\t" << residue_list[i].r_bead[j].color
					<< "\t" << residue_list[i].r_bead[j].placing_method
					<< "\t" << residue_list[i].r_bead[j].chain
					<< "\t" << residue_list[i].r_bead[j].volume
					<< "\t" << residue_list[i].r_bead[j].visibility << endl;
			}
			str1.sprintf("%d: ", i+1);
			str1 += residue_list[i].name.upper();
			lb_residues->insertItem(str1);
		}
		f.close();
	}
	str1.sprintf(tr(" Number of Residues in File: %d"), residue_list.size());
	lbl_numresidues->setText(str1);
	pb_accept_bead->setEnabled(false);
	pb_add->setEnabled(false);
	existing_residue = false;
	reset();
}

void US_AddResidue::select_residue_file()
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
		lbl_residue_file->setText(residue_filename);
		QFile f(residue_filename);
		residue_list.clear();
		lb_residues->clear();
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
					ts >> new_atom.hybrid.mw;
					ts >> new_atom.hybrid.radius;
					ts >> new_atom.bead_assignment;
					ts >> positioner;
//					cout << "Atom: " << new_atom.name << ": " << new_atom.mw << ", " << new_atom.radius << ", "
//					<< new_atom.bead_assignment << endl;
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
/*						str1.sprintf("%d: ", j+1);
						str1 += new_atom.name;
						cmb_r_atoms->insertItem(str1);
*/
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
					ts >> new_bead.visibility;
					str2 = ts.readLine(); // read rest of line
					new_residue.r_bead.push_back(new_bead);
//					str1.sprintf("Bead %d: defined", j+1);
//					cmb_r_beads->insertItem(str1);
				}
				if ( !new_residue.name.isEmpty()
					&& new_residue.molvol > 0.0
					&& new_residue.asa > 0.0)
				{
					residue_list.push_back(new_residue);
					str1.sprintf("%d: ", i);
					str1 += new_residue.name;
					lb_residues->insertItem(str1);
					i++;
				}
			}
			f.close();
			pb_select_atom_file->setEnabled(false);
			pb_select_residue_file->setEnabled(false);
		}
	}
	str1.sprintf(tr(" Number of Residues in File: %d"), residue_list.size());
	lbl_numresidues->setText(str1);
}

void US_AddResidue::select_atom_file()
{
	QString old_filename = atom_filename, str1, str2;
	atom_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.atom *.ATOM", this);
	if (atom_filename.isEmpty())
	{
		atom_filename = old_filename;
	}
	else
	{
		lbl_atom_file->setText(atom_filename);
		QFile f(atom_filename);
		atom_list.clear();
		cmb_atoms->clear();
		unsigned int i, j;
		if (f.open(IO_ReadOnly|IO_Translate))
		{
			QTextStream ts(&f);
			while (!ts.atEnd())
			{
				ts >> new_atom.name;
				ts >> new_atom.hybrid.name;
				ts >> new_atom.hybrid.mw;
				ts >> new_atom.hybrid.radius;
				ts >> new_atom.chain;
				str2 = ts.readLine(); // read rest of line
				if (!new_atom.name.isEmpty() && new_atom.hybrid.radius > 0.0 && new_atom.hybrid.mw > 0.0)
				{
					atom_list.push_back(new_atom);
				}
			}
			f.close();
			bool flag;
			atoms.clear();
			for (i=0; i<atom_list.size(); i++)
			{
				flag = false;
				for (j=0; j<atoms.size(); j++)
				{
					if (atoms[j] == atom_list[i].name)
					{
						flag = true; // this atom_list item is already in the atoms vector
					}
				}
				if (flag)
				{
					flag = false; // reset flag
				}
				else // add it to the list
				{
					atoms.push_back(atom_list[i].name);
					cmb_atoms->insertItem(atom_list[i].name);
				}
			}
			pb_select_residue_file->setEnabled(true);
			cnt_numatoms->setEnabled(true);
			cnt_numbeads->setEnabled(true);
			le_asa->setEnabled(true);
			le_molvol->setEnabled(true);
			cmb_type->setEnabled(true);
			le_residue_name->setEnabled(true);
			pb_reset->setEnabled(true);
			pb_accept_residue->setEnabled(true);
		}
	}
}

void US_AddResidue::update_name(const QString &str)
{
	new_residue.name = str;
}

void US_AddResidue::update_numatoms(double val)
{
	QString str;
	new_residue.r_atom.clear();
	new_residue.r_atom.resize((unsigned int) val);
	cmb_r_atoms->clear();
	for (unsigned int i=0; i<(unsigned int) val; i++)
	{
		str.sprintf("Atom %d: (undefined)", i+1);
		cmb_r_atoms->insertItem(str);
	}
}

void US_AddResidue::update_numbeads(double val)
{
	QString str;
	new_residue.r_bead.clear();
	new_residue.r_bead.resize((unsigned int) val);
	cmb_r_beads->clear();
	for (unsigned int i=0; i<(unsigned int) val; i++)
	{
		str.sprintf("Bead %d: (undefined)", i+1);
		cmb_r_beads->insertItem(str);
		new_residue.r_bead[i].color = 0;
		new_residue.r_bead[i].chain = 0;
		new_residue.r_bead[i].hydration = 0;
		new_residue.r_bead[i].placing_method = 0;
	}
}

void US_AddResidue::update_molvol(const QString &str)
{
	new_residue.molvol = str.toFloat();
}

void US_AddResidue::update_asa(const QString &str)
{
	new_residue.asa = str.toFloat();
}

void US_AddResidue::select_type(int val)
{
	new_residue.type = (unsigned int) val;
}

void US_AddResidue::update_hybrid(int val)
{
	cmb_hybrids->clear();
	for (unsigned int i=0; i<atom_list.size(); i++)
	{
		if(atom_list[i].name == cmb_atoms->text(val))
		{
			cmb_hybrids->insertItem(atom_list[i].hybrid.name);
		}
	}
}

void US_AddResidue::select_beadatom()
{
	unsigned int i;
	lb_list_beadatom->clear();
	for (i=0; i<lb_select_beadatom->count(); i++)
	{
		if(lb_select_beadatom->isSelected(i))
		{
			lb_list_beadatom->insertItem(lb_select_beadatom->text(i));
//			cout << "Current bead 1: " << current_bead << endl;
			new_residue.r_atom[i].bead_assignment = current_bead;
		}
	}
}

void US_AddResidue::update_hydration(double val)
{
	new_bead.hydration = (unsigned int) val;
}

void US_AddResidue::select_bead_color(int val)
{
	new_bead.color = (unsigned int) val;
}

void US_AddResidue::select_placing_method(int val)
{
	new_bead.placing_method = (unsigned int) val;
}

void US_AddResidue::select_r_bead(int val)
{
	QString str;
	current_bead = (unsigned int) val;
	if (existing_residue)
	{
		new_bead.color = new_residue.r_bead[current_bead].color;
		new_bead.chain = new_residue.r_bead[current_bead].chain;
		new_bead.placing_method = new_residue.r_bead[current_bead].placing_method;
		new_bead.hydration = new_residue.r_bead[current_bead].hydration;
	}
	cmb_bead_color->setCurrentItem(new_residue.r_bead[current_bead].color);
	cmb_placing->setCurrentItem(new_residue.r_bead[current_bead].placing_method);
	cnt_hydration->setValue(new_residue.r_bead[current_bead].hydration);
	if (new_residue.r_bead[current_bead].chain)
	{
		bg_chain->setButton(1);
	}
	else
	{
		bg_chain->setButton(0);
	}
	unsigned int i;
	lb_list_beadatom->clear();
//	cout << "Current bead 2: " << current_bead << endl;
	for (i=0; i<new_residue.r_atom.size(); i++)
	{
		if (new_residue.r_atom[i].bead_assignment == current_bead)
		{
			str.sprintf("Atom %d: ", new_residue.r_atom[i].serial_number + 1);
			str += new_residue.r_atom[i].name;
			if (new_residue.r_atom[i].positioner)
			{
				str += " (" + new_residue.r_atom[i].hybrid.name + ", Positioning: yes)";
			}
			else
			{
				str += " (" + new_residue.r_atom[i].hybrid.name + ", Positioning: no)";
			}
			lb_list_beadatom->insertItem(str);
		}
	}
}

void US_AddResidue::select_r_atom(int val)
{
	if (existing_residue)
	{
		if (new_residue.r_atom[val].positioner)
		{
			cb_positioning->setChecked(true);
		}
		else
		{
			cb_positioning->setChecked(false);
		}
		for (unsigned int i=0; i<atom_list.size(); i++)
		{
			if(atom_list[i].name == new_residue.r_atom[val].name)
			{
				cmb_atoms->setCurrentItem(i);
				break;
			}
		}
	}
}

void US_AddResidue::select_residue(int val)
{
	reset();
	existing_residue = true;
	QString str;
	new_residue.r_atom.clear();
	new_residue.r_bead.clear();
	le_residue_name->setText(residue_list[val].name);
	cnt_numatoms->setValue(residue_list[val].r_atom.size());
	cnt_numbeads->setValue(residue_list[val].r_bead.size());
	cmb_type->setCurrentItem(residue_list[val].type);
	le_molvol->setText(str.sprintf("%6.4e", residue_list[val].molvol));
	le_asa->setText(str.sprintf("%6.4e", residue_list[val].asa));
	new_residue = residue_list[val];
//	print_residue (new_residue);
//	print_residue (residue_list[val]);
}

void US_AddResidue::print_residue(struct residue res)
{
	unsigned int i;
	cout << "Residue name: " << res.name << endl;
	cout << "Residue type: " << res.type << endl;
	cout << "Residue molvol: " << res.molvol << endl;
	cout << "Residue asa: " << res.asa << endl;
	cout << "Number of atoms: " << res.r_atom.size() << endl;
	for (i=0; i<res.r_atom.size(); i++)
	{
		cout << "Atom " << i+1 << ":\n";
		cout << "\tName:" << res.r_atom[i].name << endl;
		cout << "\tMW:" << res.r_atom[i].hybrid.mw << endl;
		cout << "\tRadius:" << res.r_atom[i].hybrid.radius << endl;
		cout << "\tBead Assignment:" << res.r_atom[i].bead_assignment << endl;
		cout << "\tPositioner:" << res.r_atom[i].positioner << endl;
		cout << "\tSerial #:" << res.r_atom[i].serial_number << endl;
		cout << "\tChain:" << res.r_atom[i].chain << endl;
	}
	cout << endl;
	cout << "Number of beads: " << res.r_bead.size() << endl;
	for (i=0; i<res.r_bead.size(); i++)
	{
		cout << "Bead " << i+1 << ":\n";
		cout << "\tHydration:" << res.r_bead[i].hydration << endl;
		cout << "\tColor:" << res.r_bead[i].color << endl;
		cout << "\tPlacing Method:" << res.r_bead[i].placing_method << endl;
		cout << "\tVisibility:" << res.r_bead[i].visibility << endl;
		cout << "\tChain:" << res.r_bead[i].chain << endl;
	}
}

void US_AddResidue::help()
{
	if (online_help == NULL)
	{
		online_help = new US_Help(this);
	}
	online_help->show_help("manual/residue.html");
}

void US_AddResidue::reset()
{
	pb_select_atom_file->setEnabled(false);
	pb_select_residue_file->setEnabled(false);
	le_residue_name->setEnabled(true);
	le_residue_name->setText("");
	cnt_numbeads->setEnabled(true);
	cnt_numbeads->setValue(0);
	cnt_numatoms->setEnabled(true);
	cnt_numatoms->setValue(0);
	cmb_type->setEnabled(true);
	le_molvol->setEnabled(true);
	le_molvol->setText("");
	le_asa->setEnabled(true);
	le_asa->setText("");

	pb_accept_residue->setEnabled(true);
	pb_accept_atom->setEnabled(false);
	pb_atom_continue->setEnabled(false);
	pb_accept_atom->setEnabled(false);
	cmb_r_atoms->setEnabled(false);
	cmb_r_atoms->clear();
	cmb_atoms->setCurrentItem(0);

	cmb_r_beads->setEnabled(false);
	cmb_r_beads->clear();
	cmb_bead_color->setEnabled(false);
	cmb_bead_color->setCurrentItem(0);
	cnt_hydration->setEnabled(false);
	cnt_hydration->setValue(0);
	cmb_placing->setEnabled(false);
	rb_backbone->setEnabled(false);
	rb_backbone->setChecked(true);
	rb_sidechain->setEnabled(false);
	rb_sidechain->setChecked(false);
	bg_chain->setEnabled(false);
	bg_chain->setButton(1);
	lb_select_beadatom->setEnabled(false);
	lb_select_beadatom->clear();
	lb_list_beadatom->setEnabled(false);
	lb_list_beadatom->clear();
	cb_positioning->setEnabled(false);
	cb_positioning->setChecked(false);
	existing_residue = false;
}

void US_AddResidue::accept_residue()
{
	if(new_residue.name.isEmpty()
	|| new_residue.r_atom.size() == 0
	|| new_residue.r_bead.size() == 0
	|| new_residue.molvol == 0
	|| new_residue.asa == 0)
	{
		QMessageBox::warning(this, tr("UltraScan Warning"),
		tr("Please define all residue fields\nbefore accepting the residue!"),
		QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	}
	else
	{
		le_residue_name->setEnabled(false);
		le_molvol->setEnabled(false);
		le_asa->setEnabled(false);
		cmb_type->setEnabled(false);
		cmb_r_atoms->setEnabled(true);
		cnt_numbeads->setEnabled(false);
		cnt_numatoms->setEnabled(false);
		pb_accept_residue->setEnabled(false);
		pb_accept_atom->setEnabled(true);
		cb_positioning->setEnabled(true);
		pb_select_atom_file->setEnabled(false);
		pb_select_residue_file->setEnabled(false);
		current_atom = 0;
		cmb_r_atoms->setCurrentItem(current_atom);
	}
	if(existing_residue)
	{
		QString str;
		cmb_r_atoms->clear();
		for (unsigned int i=0; i<new_residue.r_atom.size(); i++)
		{
			str.sprintf("Atom %d: ", i + 1);
			str += new_residue.r_atom[i].name;
			if (new_residue.r_atom[i].positioner)
			{
				str += " (" + new_residue.r_atom[i].hybrid.name + ", Positioning: yes)";
			}
			else
			{
				str += " (" + new_residue.r_atom[i].hybrid.name + ", Positioning: no)";
			}
			cmb_r_atoms->insertItem(str);
		}
		cmb_r_atoms->setCurrentItem(0);
		if (new_residue.r_atom[0].positioner)
		{
			cb_positioning->setChecked(true);
		}
		else
		{
			cb_positioning->setChecked(false);
		}
	}
}

void US_AddResidue::atom_continue()
{
//cout << "Residue print atom_continue -1 : \n";
//print_residue (new_residue);
	pb_accept_atom->setEnabled(false);
	cnt_hydration->setEnabled(true);
	cmb_r_beads->setEnabled(true);
	cmb_r_beads->setCurrentItem(0);
	current_bead = 0;
	cmb_bead_color->setEnabled(true);
	cmb_placing->setEnabled(true);
	lb_select_beadatom->setEnabled(true);
	le_bead_volume->setEnabled(true);
	le_bead_volume->setText("0.0");
	cb_positioning->setEnabled(false);
	rb_backbone->setEnabled(true);
	rb_backbone->setChecked(true);
	rb_sidechain->setEnabled(true);
	rb_sidechain->setChecked(false);
	bg_chain->setEnabled(true);
	bg_chain->setChecked(1);
	lb_list_beadatom->setEnabled(true);
	lb_list_beadatom->clear();
	lb_select_beadatom->setEnabled(true);
	lb_select_beadatom->clear();
	for (int i=0; i<cmb_r_atoms->count(); i++)
	{
		lb_select_beadatom->insertItem(cmb_r_atoms->text(i));
	}
	current_atom = 0;
	lb_select_beadatom->setCurrentItem(0);
	pb_accept_bead->setEnabled(true);
	pb_atom_continue->setEnabled(false);
	if(existing_residue)
	{
		QString str;
		cmb_r_beads->clear();
		for (unsigned int i=0; i<new_residue.r_bead.size(); i++)
		{
			str.sprintf("Bead %d: defined", i + 1);
			cmb_r_beads->insertItem(str);
		}
		cmb_r_beads->setCurrentItem(0);
		cmb_bead_color->setCurrentItem(new_residue.r_bead[0].color);
		cnt_hydration->setValue(new_residue.r_bead[0].hydration);
		cmb_placing->setCurrentItem(new_residue.r_bead[0].placing_method);
		for (unsigned int i=0; i<new_residue.r_atom.size(); i++)
		{
//cout << "i=" << i << ": " << new_residue.r_atom[i].name << " (" <<
//new_residue.r_atom[i].bead_assignment << ")" << endl;
			if(new_residue.r_atom[i].bead_assignment == 0)
			{
				lb_list_beadatom->insertItem(lb_select_beadatom->text(i));
			}
		}
	}
//cout << "Residue print atom_continue - 2: \n";
//print_residue (new_residue);
}

void US_AddResidue::accept_bead()
{
	if (new_bead.volume == 0.0)
	{
		QMessageBox::message("Attention:", "No bead volume entered for this bead!\nPlease correct this and try again...");
		return;
	}
	QString str;
	str.sprintf("Bead %d: defined", current_bead + 1);
	cmb_r_beads->changeItem(str, current_bead);
	bool flag = true;
	for (int i=0; i<cmb_r_beads->count(); i++)
	{
		str = cmb_r_beads->text(i);
		if (str.contains("undefined"))
		{
			flag = false;
		}
	}
	if (flag)
	{
		pb_add->setEnabled(true);
	}
	new_residue.r_bead[current_bead] = new_bead;
	new_bead.volume = 0.0;
	le_bead_volume->setText("0.0");
}

void US_AddResidue::accept_atom()
{
	int current_item1, current_item2=0;
	if (existing_residue)
	{
		position_flag = new_residue.r_atom[cmb_r_atoms->currentItem()].positioner;
	}
//	cout << "Residue print accept_atom() - 1: \n";
//	print_residue(new_residue);
	QString str;
// find current hybridization in atom_list:
	current_item1 = cmb_r_atoms->currentItem();
	for (unsigned int i=0; i<atom_list.size(); i++)
	{
		if (cmb_atoms->currentText() == atom_list[i].name && cmb_hybrids->currentText() == atom_list[i].hybrid.name)
		{
			current_item2 = i;
			break;
		}
	}
	if (existing_residue)
	{
		unsigned int bead = new_residue.r_atom[current_item1].bead_assignment;
		unsigned int snum = new_residue.r_atom[current_item1].serial_number;
		bool pos = new_residue.r_atom[current_item1].positioner;
		new_residue.r_atom[current_item1] = atom_list[current_item2];
		new_residue.r_atom[current_item1].bead_assignment = bead;
		new_residue.r_atom[current_item1].positioner = pos;
		new_residue.r_atom[current_item1].serial_number = snum;
	}
	else
	{
		new_residue.r_atom[current_item1] = atom_list[current_item2];
	}

// assign a serial number specific to this residue so atoms can be distinguished
// and prevented from being used twice in different beads

	new_residue.r_atom[current_item1].serial_number = current_item1;
	new_residue.r_atom[current_item1].positioner = position_flag;
	str.sprintf("Atom %d: " + atom_list[current_item2].name, current_item1+1);
	str += " (" + new_residue.r_atom[current_item1].hybrid.name + ", Positioning: ";
	if (new_residue.r_atom[current_item1].positioner)
	{
		str += "yes)";
	}
	else
	{
		str += "no)";
	}
	cmb_r_atoms->changeItem(str, current_item1);
	bool flag = true;
	for (int i=0; i<cmb_r_atoms->count(); i++)
	{
		str = cmb_r_atoms->text(i);
		if (str.contains("undefined"))
		{
			flag = false;
		}
	}
	if (flag)
	{
		pb_atom_continue->setEnabled(true);
	}
//	cout << "Residue print accept_atom() - 2: \n";
//	print_residue(new_residue);
}

void US_AddResidue::set_chain(int val)
{
	new_bead.chain = val;
}

void US_AddResidue::set_positioning()
{
	if (cb_positioning->isChecked())
	{
		position_flag = true;
	}
	else
	{
		position_flag = false;
 	}
}

void US_AddResidue::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;

	*widget_flag = false;
	e->accept();
}


void US_AddResidue::update_bead_volume(const QString & val)
{
	new_bead.volume = val.toFloat();
}
