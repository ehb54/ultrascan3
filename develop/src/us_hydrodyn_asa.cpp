#include "../include/us_hydrodyn_asa.h"

US_Hydrodyn_ASA::US_Hydrodyn_ASA(struct asa_options *asa, bool *asa_widget, QWidget *p, const char *name) : QFrame(p, name)
{
	this->asa = asa;
	this->asa_widget = asa_widget;
	*asa_widget = true;
	USglobal=new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("SOMO Accessible Surface Area Options"));
	setupGUI();
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_ASA::~US_Hydrodyn_ASA()
{
	*asa_widget = false;
}

void US_Hydrodyn_ASA::setupGUI()
{
	int minHeight1 = 30;
	lbl_info = new QLabel(tr("Accessible Surface Area Options:"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setMinimumHeight(minHeight1);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	bg_asa_method = new QButtonGroup(2, Qt::Vertical, "ASA Method:", this);
	bg_asa_method->setExclusive(true);
	connect(bg_asa_method, SIGNAL(clicked(int)), this, SLOT(select_asa_method(int)));

	cb_surfracer = new QCheckBox(bg_asa_method);
	cb_surfracer->setText(tr(" Voronoi Tesselation (Surfracer, Tsodikov et al.)"));
	cb_surfracer->setEnabled(true);
	cb_surfracer->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_surfracer->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	cb_asab1 = new QCheckBox(bg_asa_method);
	cb_asab1->setText(tr(" Rolling Sphere (ASAB1, Lee && Richard's Method)"));
	cb_asab1->setEnabled(true);
	cb_asab1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_asab1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	bg_asa_method->setButton((*asa).method);

	lbl_probe_radius = new QLabel(tr(" ASA Probe Radius (A): "), this);
	Q_CHECK_PTR(lbl_probe_radius);
	lbl_probe_radius->setAlignment(AlignLeft|AlignVCenter);
	lbl_probe_radius->setMinimumHeight(minHeight1);
	lbl_probe_radius->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_probe_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cnt_probe_radius= new QwtCounter(this);
	Q_CHECK_PTR(cnt_probe_radius);
	cnt_probe_radius->setRange(0, 10, 0.01);
	cnt_probe_radius->setValue((*asa).probe_radius);
	cnt_probe_radius->setMinimumHeight(minHeight1);
	cnt_probe_radius->setEnabled(true);
	cnt_probe_radius->setNumButtons(3);
	cnt_probe_radius->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
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
	cnt_asa_threshold->setValue((*asa).threshold);
	cnt_asa_threshold->setMinimumHeight(minHeight1);
	cnt_asa_threshold->setEnabled(true);
	cnt_asa_threshold->setNumButtons(3);
	cnt_asa_threshold->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
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
	cnt_asa_threshold_percent->setValue((*asa).threshold_percent);
	cnt_asa_threshold_percent->setMinimumHeight(minHeight1);
	cnt_asa_threshold_percent->setEnabled(true);
	cnt_asa_threshold_percent->setNumButtons(3);
	cnt_asa_threshold_percent->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_asa_threshold_percent->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_asa_threshold_percent, SIGNAL(valueChanged(double)), SLOT(update_asa_threshold_percent(double)));

	lbl_asab1_step = new QLabel(tr(" ASAB1 Step Size (A): "), this);
	Q_CHECK_PTR(lbl_asab1_step);
	lbl_asab1_step->setAlignment(AlignLeft|AlignVCenter);
	lbl_asab1_step->setMinimumHeight(minHeight1);
	lbl_asab1_step->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_asab1_step->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cnt_asab1_step= new QwtCounter(this);
	Q_CHECK_PTR(cnt_asab1_step);
	cnt_asab1_step->setRange(0.1, 100, 0.1);
	cnt_asab1_step->setValue((*asa).asab1_step);
	cnt_asab1_step->setMinimumHeight(minHeight1);
	cnt_asab1_step->setEnabled(true);
	cnt_asab1_step->setNumButtons(3);
	cnt_asab1_step->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_asab1_step->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_asab1_step, SIGNAL(valueChanged(double)), SLOT(update_asab1_step(double)));

	cb_asa_calculation = new QCheckBox(this);
	cb_asa_calculation->setText(tr(" Perform ASA Calculation "));
	cb_asa_calculation->setChecked((*asa).calculation);
	cb_asa_calculation->setEnabled(true);
	cb_asa_calculation->setMinimumHeight(minHeight1);
	cb_asa_calculation->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_asa_calculation->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_asa_calculation, SIGNAL(clicked()), SLOT(set_asa_calculation()));

	cb_bead_check = new QCheckBox(this);
	cb_bead_check->setText(tr(" Re-check bead ASA "));
	cb_bead_check->setChecked((*asa).recheck_beads);
	cb_bead_check->setEnabled(true);
	cb_bead_check->setMinimumHeight(minHeight1);
	cb_bead_check->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_bead_check->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_bead_check, SIGNAL(clicked()), SLOT(set_bead_check()));

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
	background->addWidget(cb_asa_calculation, j, 0);
	background->addWidget(cb_bead_check, j, 1);
	j++;
	background->addMultiCellWidget(bg_asa_method, j, j+3, 0, 1);
	j+=4;
	background->addWidget(lbl_probe_radius, j, 0);
	background->addWidget(cnt_probe_radius, j, 1);
	j++;
	background->addWidget(lbl_asa_threshold, j, 0);
	background->addWidget(cnt_asa_threshold, j, 1);
	j++;
	background->addWidget(lbl_asa_threshold_percent, j, 0);
	background->addWidget(cnt_asa_threshold_percent, j, 1);
	j++;
	background->addWidget(lbl_asab1_step, j, 0);
	background->addWidget(cnt_asab1_step, j, 1);
	j++;
	background->addWidget(pb_help, j, 0);
	background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_ASA::cancel()
{
	close();
}

void US_Hydrodyn_ASA::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/somo_asa.html");
}

void US_Hydrodyn_ASA::closeEvent(QCloseEvent *e)
{
	*asa_widget = false;
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

void US_Hydrodyn_ASA::update_probe_radius(double val)
{
	(*asa).probe_radius = val;
}

void US_Hydrodyn_ASA::update_asa_threshold(double val)
{
	(*asa).threshold = val;
}

void US_Hydrodyn_ASA::update_asa_threshold_percent(double val)
{
	(*asa).threshold_percent = val;
}

void US_Hydrodyn_ASA::update_asab1_step(double val)
{
	(*asa).asab1_step = (float) val;
}

void US_Hydrodyn_ASA::set_asa_calculation()
{
	(*asa).calculation = cb_asa_calculation->isChecked();
}

void US_Hydrodyn_ASA::set_bead_check()
{
	(*asa).recheck_beads = cb_bead_check->isChecked();
}

void US_Hydrodyn_ASA::select_asa_method(int val)
{
	(*asa).method = val;
}


