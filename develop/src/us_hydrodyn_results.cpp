#include "../include/us_hydrodyn_results.h"
#define DOTSOMO		""
#define DOTSOMOCAP	""

US_Hydrodyn_Results::US_Hydrodyn_Results(struct hydro_results *results,
bool *result_widget, QWidget *p, const char *name) : QFrame(p, name)
{
	this->results = results;
	this->result_widget = result_widget;
	*result_widget = true;
	USglobal=new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("SOMO Hydrodynamic Results"));
	setupGUI();
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);
	somo_dir = USglobal->config_list.root_dir + "/somo";
}

US_Hydrodyn_Results::~US_Hydrodyn_Results()
{
	*result_widget = false;
}

void US_Hydrodyn_Results::setupGUI()
{
	int minHeight1 = 30;
	QString str;

	lbl_info = new QLabel(tr("   SOMO Hydrodynamic Results (Water at 20ºC):   "), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setMinimumHeight(minHeight1);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_total_beads = new QLabel(tr(" Total Beads in Model: "), this);
	Q_CHECK_PTR(lbl_total_beads);
	lbl_total_beads->setAlignment(AlignLeft|AlignVCenter);
	lbl_total_beads->setMinimumWidth(200);
	lbl_total_beads->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_total_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_total_beads = new QLineEdit(this, "total_beads Line Edit");
	le_total_beads->setText(str.sprintf("%d", (*results).total_beads));
	le_total_beads->setReadOnly(true);
	le_total_beads->setMinimumWidth(200);
	le_total_beads->setAlignment(AlignVCenter);
	le_total_beads->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_total_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_used_beads = new QLabel(tr(" Used Beads in Model: "), this);
	Q_CHECK_PTR(lbl_used_beads);
	lbl_used_beads->setAlignment(AlignLeft|AlignVCenter);
	lbl_used_beads->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_used_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_used_beads = new QLineEdit(this, "used_beads Line Edit");
	le_used_beads->setText(str.sprintf("%d", (*results).used_beads));
	le_used_beads->setReadOnly(true);
	le_used_beads->setAlignment(AlignVCenter);
	le_used_beads->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_used_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_mass = new QLabel(tr(" Molecular Mass: "), this);
	Q_CHECK_PTR(lbl_mass);
	lbl_mass->setAlignment(AlignLeft|AlignVCenter);
	lbl_mass->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_mass = new QLineEdit(this, "mass Line Edit");
	le_mass->setText(str.sprintf("%6.4e Da", (*results).mass));
	le_mass->setReadOnly(true);
	le_mass->setAlignment(AlignVCenter);
	le_mass->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_vbar = new QLabel(tr(" Part. Specif. Volume: "), this);
	Q_CHECK_PTR(lbl_vbar);
	lbl_vbar->setAlignment(AlignLeft|AlignVCenter);
	lbl_vbar->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_vbar = new QLineEdit(this, "vbar Line Edit");
	le_vbar->setText(str.sprintf("%5.3f cm^3/g", (*results).vbar));
	le_vbar->setReadOnly(true);
	le_vbar->setAlignment(AlignVCenter);
	le_vbar->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_s20w = new QLabel(tr(" s(20,w): "), this);
	Q_CHECK_PTR(lbl_s20w);
	lbl_s20w->setAlignment(AlignLeft|AlignVCenter);
	lbl_s20w->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_s20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_s20w = new QLineEdit(this, "s20w Line Edit");
	le_s20w->setText(str.sprintf("%4.2e S", (*results).s20w));
	le_s20w->setReadOnly(true);
	le_s20w->setAlignment(AlignVCenter);
	le_s20w->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_s20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_D20w = new QLabel(tr(" D(20,w), transl.: "), this);
	Q_CHECK_PTR(lbl_D20w);
	lbl_D20w->setAlignment(AlignLeft|AlignVCenter);
	lbl_D20w->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_D20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_D20w = new QLineEdit(this, "D20w Line Edit");
	le_D20w->setText(str.sprintf("%4.2e cm/sec^2", (*results).D20w));
	le_D20w->setReadOnly(true);
	le_D20w->setAlignment(AlignVCenter);
	le_D20w->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_D20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_tau = new QLabel(tr(" Relaxation Time, tau(h): "), this);
	Q_CHECK_PTR(lbl_tau);
	lbl_tau->setAlignment(AlignLeft|AlignVCenter);
	lbl_tau->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_tau = new QLineEdit(this, "tau Line Edit");
	le_tau->setText(str.sprintf("%4.2e ns", (*results).tau));
	le_tau->setReadOnly(true);
	le_tau->setAlignment(AlignVCenter);
	le_tau->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_viscosity = new QLabel(tr(" Intrinsic Viscosity: "), this);
	Q_CHECK_PTR(lbl_viscosity);
	lbl_viscosity->setAlignment(AlignLeft|AlignVCenter);
	lbl_viscosity->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_viscosity = new QLineEdit(this, "viscosity Line Edit");
	le_viscosity->setText(str.sprintf("%4.2e cm^3/g", (*results).viscosity));
	le_viscosity->setReadOnly(true);
	le_viscosity->setAlignment(AlignVCenter);
	le_viscosity->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_rs = new QLabel(tr(" Stokes Radius: "), this);
	Q_CHECK_PTR(lbl_rs);
	lbl_rs->setAlignment(AlignLeft|AlignVCenter);
	lbl_rs->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_rs = new QLineEdit(this, "rs Line Edit");
	le_rs->setText(str.sprintf("%4.2e nm", (*results).rs));
	le_rs->setReadOnly(true);
	le_rs->setAlignment(AlignVCenter);
	le_rs->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_rs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_rg = new QLabel(tr(" Radius of Gyration: "), this);
	Q_CHECK_PTR(lbl_rg);
	lbl_rg->setAlignment(AlignLeft|AlignVCenter);
	lbl_rg->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	le_rg = new QLineEdit(this, "rg Line Edit");
	le_rg->setText(str.sprintf("%4.2e nm", (*results).rg));
	le_rg->setReadOnly(true);
	le_rg->setAlignment(AlignVCenter);
	le_rg->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	pb_load_asa = new QPushButton(tr("Load ASA Results File"), this);
	Q_CHECK_PTR(pb_load_asa);
	pb_load_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_load_asa->setMinimumHeight(minHeight1);
	pb_load_asa->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load_asa, SIGNAL(clicked()), SLOT(load_asa()));

	pb_load_results = new QPushButton(tr("Load Full Hydrodynamics Results File"), this);
	Q_CHECK_PTR(pb_load_results);
	pb_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_load_results->setMinimumHeight(minHeight1);
	pb_load_results->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load_results, SIGNAL(clicked()), SLOT(load_results()));

	pb_load_beadmodel = new QPushButton(tr("Load Bead Model"), this);
	Q_CHECK_PTR(pb_load_beadmodel);
	pb_load_beadmodel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_load_beadmodel->setMinimumHeight(minHeight1);
	pb_load_beadmodel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load_beadmodel, SIGNAL(clicked()), SLOT(load_beadmodel()));

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
	background->addWidget(lbl_total_beads, j, 0);
	background->addWidget(le_total_beads, j, 1);
	j++;
	background->addWidget(lbl_used_beads, j, 0);
	background->addWidget(le_used_beads, j, 1);
	j++;
	background->addWidget(lbl_mass, j, 0);
	background->addWidget(le_mass, j, 1);
	j++;
	background->addWidget(lbl_vbar, j, 0);
	background->addWidget(le_vbar, j, 1);
	j++;
	background->addWidget(lbl_s20w, j, 0);
	background->addWidget(le_s20w, j, 1);
	j++;
	background->addWidget(lbl_D20w, j, 0);
	background->addWidget(le_D20w, j, 1);
	j++;
	background->addWidget(lbl_rs, j, 0);
	background->addWidget(le_rs, j, 1);
	j++;
	background->addWidget(lbl_rg, j, 0);
	background->addWidget(le_rg, j, 1);
	j++;
	background->addWidget(lbl_tau, j, 0);
	background->addWidget(le_tau, j, 1);
	j++;
	background->addWidget(lbl_viscosity, j, 0);
	background->addWidget(le_viscosity, j, 1);
	j++;
	background->addWidget(pb_load_asa, j, 0);
	background->addWidget(pb_load_beadmodel, j, 1);
	j++;
	background->addMultiCellWidget(pb_load_results, j, j, 0, 1);
	j++;
	background->addWidget(pb_help, j, 0);
	background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_Results::cancel()
{
	close();
}

void US_Hydrodyn_Results::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/somo_results.html");
}

void US_Hydrodyn_Results::load_results()
{
	QString filename = QFileDialog::getOpenFileName(somo_dir, "*.hydro_res *.hydro_res", this);
	if (!filename.isEmpty())
	{
		view_file(filename);
	}
}

void US_Hydrodyn_Results::load_beadmodel()
{
	QString filename = QFileDialog::getOpenFileName(somo_dir, "*.bead_model* *.BEAD_MODEL*", this);
	if (!filename.isEmpty())
	{
		view_file(filename);
	}
}

void US_Hydrodyn_Results::load_asa()
{
	QString filename = QFileDialog::getOpenFileName(somo_dir, "*.asa *.ASA", this);
	if (!filename.isEmpty())
	{
		view_file(filename);
	}
}

void US_Hydrodyn_Results::view_file(const QString &filename)
{
	e = new TextEdit();
	e->setFont(QFont("Courier"));
	e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
	e->load(filename);
	e->show();
}

void US_Hydrodyn_Results::closeEvent(QCloseEvent *e)
{
	*result_widget = false;
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

