#include "../include/us_hydrodyn_hydro.h"

US_Hydrodyn_Hydro::US_Hydrodyn_Hydro(struct hydro_options *hydro,
bool *hydro_widget, QWidget *p, const char *name) : QFrame(p, name)
{
	this->hydro = hydro;
	this->hydro_widget = hydro_widget;
	*hydro_widget = true;
	USglobal=new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("SOMO Hydrodynamic Calculation Options"));
	this->setMinimumWidth(680);
	setupGUI();
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Hydro::~US_Hydrodyn_Hydro()
{
	*hydro_widget = false;
}

void US_Hydrodyn_Hydro::setupGUI()
{
	int minHeight1 = 30;
	QString str;	
	lbl_info = new QLabel(tr("SOMO Hydrodynamic Calculation Options:"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setMinimumHeight(minHeight1);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_unit = new QLabel(tr(" Model units (-10 = Angstrom, -9 = nanometer): "), this);
	Q_CHECK_PTR(lbl_unit);
	lbl_unit->setAlignment(AlignLeft|AlignVCenter);
	lbl_unit->setMinimumHeight(minHeight1);
	lbl_unit->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_unit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cnt_unit= new QwtCounter(this);
	Q_CHECK_PTR(cnt_unit);
	cnt_unit->setRange(-10, -5, 1);
	cnt_unit->setValue((*hydro).unit);
	cnt_unit->setMinimumHeight(minHeight1);
	cnt_unit->setEnabled(true);
	cnt_unit->setNumButtons(2);
	cnt_unit->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_unit->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_unit, SIGNAL(valueChanged(double)), SLOT(update_unit(double)));

	bg_reference_system = new QButtonGroup(2, Qt::Horizontal, "Computations Relative to:", this);
	bg_reference_system->setExclusive(true);
	connect(bg_reference_system, SIGNAL(clicked(int)), this, SLOT(select_reference_system(int)));

	cb_diffusion_center = new QCheckBox(bg_reference_system);
	cb_diffusion_center->setText(tr(" Diffusion Center "));
	cb_diffusion_center->setEnabled(true);
	cb_diffusion_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_diffusion_center->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_cartesian_origin = new QCheckBox(bg_reference_system);
	cb_cartesian_origin->setText(tr(" Cartesian Origin "));
	cb_cartesian_origin->setEnabled(true);
	cb_cartesian_origin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_cartesian_origin->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	bg_reference_system->setButton((*hydro).reference_system);

	bg_boundary_cond = new QButtonGroup(2, Qt::Horizontal, "Boundary Conditions:", this);
	bg_boundary_cond->setExclusive(true);
	connect(bg_boundary_cond, SIGNAL(clicked(int)), this, SLOT(select_boundary_cond(int)));

	cb_stick = new QCheckBox(bg_boundary_cond);
	cb_stick->setText(tr(" Stick "));
	cb_stick->setEnabled(true);
	cb_stick->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_stick->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_slip = new QCheckBox(bg_boundary_cond);
	cb_slip->setText(tr(" Slip "));
	cb_slip->setEnabled(true);
	cb_slip->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_slip->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	bg_boundary_cond->setButton((*hydro).boundary_cond);
	
	bg_mass_correction = new QButtonGroup(4, Qt::Horizontal, "Total Mass of Model:", this);
	bg_mass_correction->setExclusive(true);
	connect(bg_mass_correction, SIGNAL(clicked(int)), this, SLOT(select_mass_correction(int)));

	cb_auto_mass = new QCheckBox(bg_mass_correction);
	cb_auto_mass->setText(tr(" Automatic (Sum of Bead Masses)"));
	cb_auto_mass->setEnabled(true);
	cb_auto_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_auto_mass->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_manual_mass = new QCheckBox(bg_mass_correction);
	cb_manual_mass->setText(tr(" Manual "));
	cb_manual_mass->setEnabled(true);
	cb_manual_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_manual_mass->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	bg_mass_correction->setButton((*hydro).mass_correction);

	lbl_mass = new QLabel(tr(" Enter mass:    "), bg_mass_correction);
	Q_CHECK_PTR(lbl_mass);
	lbl_mass->setAlignment(AlignLeft|AlignVCenter);
	lbl_mass->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_mass = new QLineEdit(bg_mass_correction, "Mass Line Edit");
	le_mass->setText(str.sprintf("%5.3e", (*hydro).mass));
	le_mass->setAlignment(AlignVCenter);
	le_mass->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	le_mass->setEnabled((*hydro).mass_correction);
	connect(le_mass, SIGNAL(textChanged(const QString &)), SLOT(update_mass(const QString &)));

	bg_volume_correction = new QButtonGroup(4, Qt::Horizontal, "Total Volume of Model: (for Rotational Diff. and Intrinsic Visc. Volume Corr.)", this);
	bg_volume_correction->setExclusive(true);
	connect(bg_volume_correction, SIGNAL(clicked(int)), this, SLOT(select_volume_correction(int)));

	cb_auto_volume = new QCheckBox(bg_volume_correction);
	cb_auto_volume->setText(tr(" Automatic (Sum of Bead Volumes)"));
	cb_auto_volume->setEnabled(true);
	cb_auto_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_auto_volume->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_manual_volume = new QCheckBox(bg_volume_correction);
	cb_manual_volume->setText(tr(" Manual "));
	cb_manual_volume->setEnabled(true);
	cb_manual_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_manual_volume->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	bg_volume_correction->setButton((*hydro).volume_correction);

	lbl_volume = new QLabel(tr(" Enter volume: "), bg_volume_correction);
	Q_CHECK_PTR(lbl_volume);
	lbl_volume->setAlignment(AlignLeft|AlignVCenter);
	lbl_volume->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_volume = new QLineEdit(bg_volume_correction, "Volume Line Edit");
	le_volume->setText(str.sprintf("%5.3e", (*hydro).volume));
	le_volume->setAlignment(AlignVCenter);
	le_volume->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	le_volume->setEnabled((*hydro).volume_correction);
	connect(le_volume, SIGNAL(textChanged(const QString &)), SLOT(update_volume(const QString &)));

	bg_bead_inclusion = new QButtonGroup(2, Qt::Horizontal, "Inclusion of Buried Beads in Hydrodynamic Calculations:", this);
	bg_bead_inclusion->setExclusive(true);
	connect(bg_bead_inclusion, SIGNAL(clicked(int)), this, SLOT(select_bead_inclusion(int)));

	cb_exclusion = new QCheckBox(bg_bead_inclusion);
	cb_exclusion->setText(tr(" Exclude "));
	cb_exclusion->setEnabled(true);
	cb_exclusion->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_exclusion->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_inclusion = new QCheckBox(bg_bead_inclusion);
	cb_inclusion->setText(tr(" Include "));
	cb_inclusion->setEnabled(true);
	cb_inclusion->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_inclusion->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	bg_bead_inclusion->setButton((*hydro).bead_inclusion);
	
	gb_buried = new QButtonGroup(2, Qt::Horizontal, "Include Buried Beads in Volume Correction for Calculation of:", this);

	cb_rotational = new QCheckBox(gb_buried);
	cb_rotational->setText(tr(" Rotational Diffusion "));
	if ((*hydro).volume_correction)
	{
		cb_rotational->setEnabled(false);
	}
	else
	{
		cb_rotational->setEnabled(!(*hydro).bead_inclusion);
	}
	cb_rotational->setChecked((*hydro).rotational);
	cb_rotational->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_rotational->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_rotational, SIGNAL(clicked()), this, SLOT(set_rotational()));

	cb_viscosity = new QCheckBox(gb_buried);
	cb_viscosity->setText(tr(" Intrinsic Viscosity "));
	if ((*hydro).volume_correction)
	{
		cb_viscosity->setEnabled(false);
	}
	else
	{
		cb_viscosity->setEnabled(!(*hydro).bead_inclusion);
	}
	cb_viscosity->setChecked((*hydro).viscosity);
	cb_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_viscosity->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_viscosity, SIGNAL(clicked()), this, SLOT(set_viscosity()));

	bg_overlap = new QButtonGroup(4, Qt::Horizontal, "Overlap cut-off:", this);
	bg_overlap->setExclusive(true);
	connect(bg_overlap, SIGNAL(clicked(int)), this, SLOT(select_overlap(int)));

	cb_auto_overlap = new QCheckBox(bg_overlap);
	cb_auto_overlap->setText(tr(" From Bead Model "));
	cb_auto_overlap->setEnabled(true);
	cb_auto_overlap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_auto_overlap->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_manual_overlap = new QCheckBox(bg_overlap);
	cb_manual_overlap->setText(tr(" Manual "));
	cb_manual_overlap->setEnabled(true);
	cb_manual_overlap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_manual_overlap->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	bg_overlap->setButton((*hydro).overlap_cutoff);

	lbl_overlap = new QLabel(tr(" Enter cut-off: "), bg_overlap);
	Q_CHECK_PTR(lbl_overlap);
	lbl_overlap->setAlignment(AlignLeft|AlignVCenter);
	lbl_overlap->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_overlap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_overlap = new QLineEdit(bg_overlap, "Mass Line Edit");
	le_overlap->setText(str.sprintf("%5.3e", (*hydro).overlap));
	le_overlap->setAlignment(AlignVCenter);
	le_overlap->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_overlap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	le_overlap->setEnabled((*hydro).overlap_cutoff);
	connect(le_overlap, SIGNAL(textChanged(const QString &)), SLOT(update_overlap(const QString &)));

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

	int rows=6, columns = 2, spacing = 2, j=0, margin=4;
	QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

	background->addMultiCellWidget(lbl_info, j, j, 0, 1);
	j++;
	background->addWidget(lbl_unit, j, 0);
	background->addWidget(cnt_unit, j, 1);
	j++;
	background->addMultiCellWidget(bg_reference_system, j, j+2, 0, 1);
	j+=3;
	background->addMultiCellWidget(bg_boundary_cond, j, j+2, 0, 1);
	j+=3;
	background->addMultiCellWidget(bg_mass_correction, j, j+2, 0, 1);
	j+=3;
	background->addMultiCellWidget(bg_volume_correction, j, j+2, 0, 1);
	j+=3;
	background->addMultiCellWidget(bg_bead_inclusion, j, j+2, 0, 1);
	j+=3;
	background->addMultiCellWidget(gb_buried, j, j+2, 0, 1);
	j+=3;
	background->addMultiCellWidget(bg_overlap, j, j+2, 0, 1);
	j+=3;
	background->addWidget(pb_help, j, 0);
	background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_Hydro::update_unit(double val)
{
	(*hydro).unit = (int) val;
}

void US_Hydrodyn_Hydro::update_volume(const QString &str)
{
	(*hydro).volume = str.toDouble();
}

void US_Hydrodyn_Hydro::update_mass(const QString &str)
{
	(*hydro).mass = str.toDouble();
}

void US_Hydrodyn_Hydro::update_overlap(const QString &str)
{
	(*hydro).overlap = str.toDouble();
}

void US_Hydrodyn_Hydro::select_reference_system(int val)
{
	(*hydro).reference_system = val;
}

void US_Hydrodyn_Hydro::select_boundary_cond(int val)
{
	(*hydro).boundary_cond = val;
}

void US_Hydrodyn_Hydro::select_bead_inclusion(int val)
{
	(*hydro).bead_inclusion = val;
	if ((*hydro).volume_correction)
	{
		cb_viscosity->setEnabled(false);
		cb_rotational->setEnabled(false);
	}
	else
	{
		cb_viscosity->setEnabled(!(*hydro).bead_inclusion);
		cb_rotational->setEnabled(!(*hydro).bead_inclusion);
	}
}

void US_Hydrodyn_Hydro::select_volume_correction(int val)
{
	(*hydro).volume_correction = val;
	le_volume->setEnabled((*hydro).volume_correction);
	if ((*hydro).volume_correction)
	{
		cb_viscosity->setEnabled(false);
		cb_rotational->setEnabled(false);
	}
	else
	{
		cb_viscosity->setEnabled(!(*hydro).bead_inclusion);
		cb_rotational->setEnabled(!(*hydro).bead_inclusion);
	}
}

void US_Hydrodyn_Hydro::select_mass_correction(int val)
{
	(*hydro).mass_correction = val;
	le_mass->setEnabled((*hydro).mass_correction);
}

void US_Hydrodyn_Hydro::select_overlap(int val)
{
	(*hydro).overlap_cutoff = val;
	le_overlap->setEnabled((*hydro).overlap_cutoff);
}

void US_Hydrodyn_Hydro::set_rotational()
{
	(*hydro).rotational = cb_rotational->isChecked();
}

void US_Hydrodyn_Hydro::set_viscosity()
{
	(*hydro).viscosity = cb_viscosity->isChecked();
}

void US_Hydrodyn_Hydro::cancel()
{
	close();
}

void US_Hydrodyn_Hydro::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/somo_hydro.html");
}

void US_Hydrodyn_Hydro::closeEvent(QCloseEvent *e)
{
	*hydro_widget = false;
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

