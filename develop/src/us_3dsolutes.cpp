#include "../include/us_3dsolutes.h"

US_3d_Solutes::US_3d_Solutes(SA2d_control_variables *sa2d_ctrl_vars, bool *window_3d_flag,
QString run_id, unsigned int *scans, unsigned int *points, unsigned int current_cell, 
unsigned int current_lambda, double **absorbance, double *radius, 
struct mfem_data *fem_model, vector <double> *ti_noise, vector <double> *ri_noise,
QWidget *p, const char *name) : QFrame(p, name)
{
	USglobal=new US_Config();
	this->sa2d_ctrl_vars = sa2d_ctrl_vars;
	this->window_3d_flag = window_3d_flag;
	this->run_id = run_id;
	this->scans = scans;
	this->points = points;
	this->current_cell = current_cell;
	this->current_lambda = current_lambda;
	this->absorbance = absorbance;
	this->radius = radius;
	this->fem_model = fem_model;
	this->ti_noise = ti_noise;
	this->ri_noise = ri_noise;
	zAdjust = 2;
	resolutionAdjust = 150;
	alphaAdjust = 80.0;
	betaAdjust = 0.3;
	*window_3d_flag = true;
	residual_plots_flag = false;
	setup();
}

void US_3d_Solutes::setup()
{
	QString str;
	mw1_flag = true;
	mw2_flag = false;
	s1_flag = false;
	s2_flag = false;
	D1_flag = false;
	D2_flag = false;
	f1_flag = false;
	f2_flag = false;
	ff01_flag = false;
	ff02_flag = true;
	widget3d_flag = false;
	
	solutes_3D.clear();
	solutes.clear();

	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("Enhanced Plotting Controls"));

	lbl_info = new QLabel(tr("3D Plotting Controls"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setMinimumSize(250, 35);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_parameter = new QLabel(tr("Dimension:"), this);
	Q_CHECK_PTR(lbl_parameter);
	lbl_parameter->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_parameter->setAlignment(AlignLeft|AlignVCenter);
	lbl_parameter->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_parameter->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_dimension1 = new QLabel(tr("1"), this);
	Q_CHECK_PTR(lbl_dimension1);
	lbl_dimension1->setAlignment(AlignCenter|AlignVCenter);
	lbl_dimension1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_dimension1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_dimension2 = new QLabel(tr("2"), this);
	Q_CHECK_PTR(lbl_dimension1);
	lbl_dimension2->setAlignment(AlignCenter|AlignVCenter);
	lbl_dimension2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_dimension2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_mw = new QLabel(tr(" Molecular Weight:"), this);
	Q_CHECK_PTR(lbl_mw);
	lbl_mw->setAlignment(AlignLeft|AlignVCenter);
	lbl_mw->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_zAdjust = new QLabel(tr(" Z-Scaling Factor:"), this);
	Q_CHECK_PTR(lbl_zAdjust);
	lbl_zAdjust->setAlignment(AlignLeft|AlignVCenter);
	lbl_zAdjust->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_zAdjust->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_resolutionAdjust = new QLabel(tr(" Grid Resolution:"), this);
	Q_CHECK_PTR(lbl_resolutionAdjust);
	lbl_resolutionAdjust->setAlignment(AlignLeft|AlignVCenter);
	lbl_resolutionAdjust->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_resolutionAdjust->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_alphaAdjust = new QLabel(tr(" Peak Smoothing:"), this);
	Q_CHECK_PTR(lbl_alphaAdjust);
	lbl_alphaAdjust->setAlignment(AlignLeft|AlignVCenter);
	lbl_alphaAdjust->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_alphaAdjust->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_betaAdjust = new QLabel(tr(" Peak Width:"), this);
	Q_CHECK_PTR(lbl_betaAdjust);
	lbl_betaAdjust->setAlignment(AlignLeft|AlignVCenter);
	lbl_betaAdjust->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_betaAdjust->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_mw1 = new QCheckBox(this);
   cb_mw1->setChecked(true);
   cb_mw1->setText(tr(""));
	cb_mw1->setMaximumWidth(24);
   cb_mw1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_mw1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_mw1, SIGNAL(clicked()), SLOT(set_mw1()));

   cb_mw2 = new QCheckBox(this);
   cb_mw2->setChecked(false);
   cb_mw2->setEnabled(false);
   cb_mw2->setText(tr(""));
   cb_mw2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_mw2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_mw2, SIGNAL(clicked()), SLOT(set_mw2()));

	lbl_s = new QLabel(tr(" Sedimentation Coeff.:  "), this);
	Q_CHECK_PTR(lbl_s);
	lbl_s->setAlignment(AlignLeft|AlignVCenter);
	lbl_s->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cb_s1 = new QCheckBox(this);
   cb_s1->setChecked(false);
   cb_s1->setText(tr(""));
   cb_s1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_s1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_s1, SIGNAL(clicked()), SLOT(set_s1()));

   cb_s2 = new QCheckBox(this);
   cb_s2->setChecked(false);
   cb_s2->setText(tr(""));
   cb_s2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_s2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_s2, SIGNAL(clicked()), SLOT(set_s2()));

	lbl_D = new QLabel(tr(" Diffusion Coeff.:"), this);
	Q_CHECK_PTR(lbl_D);
	lbl_D->setAlignment(AlignLeft|AlignVCenter);
	lbl_D->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cb_D1 = new QCheckBox(this);
   cb_D1->setChecked(false);
   cb_D1->setText(tr(""));
   cb_D1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_D1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_D1, SIGNAL(clicked()), SLOT(set_D1()));

   cb_D2 = new QCheckBox(this);
   cb_D2->setChecked(false);
   cb_D2->setText(tr(""));
   cb_D2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_D2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_D2, SIGNAL(clicked()), SLOT(set_D2()));

	lbl_f = new QLabel(tr(" Frictional Coeff.:"), this);
	Q_CHECK_PTR(lbl_f);
	lbl_f->setAlignment(AlignLeft|AlignVCenter);
	lbl_f->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_f->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cb_f1 = new QCheckBox(this);
   cb_f1->setChecked(false);
   cb_f1->setText(tr(""));
   cb_f1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_f1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_f1, SIGNAL(clicked()), SLOT(set_f1()));

   cb_f2 = new QCheckBox(this);
   cb_f2->setChecked(false);
   cb_f2->setText(tr(""));
   cb_f2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_f2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_f2, SIGNAL(clicked()), SLOT(set_f2()));

	lbl_ff0 = new QLabel(tr(" Frictional Ratio:"), this);
	Q_CHECK_PTR(lbl_ff0);
	lbl_ff0->setAlignment(AlignLeft|AlignVCenter);
	lbl_ff0->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_ff0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cb_ff01 = new QCheckBox(this);
   cb_ff01->setChecked(false);
   cb_ff01->setEnabled(false);
   cb_ff01->setText(tr(""));
   cb_ff01->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_ff01->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_ff01, SIGNAL(clicked()), SLOT(set_ff01()));

   cb_ff02 = new QCheckBox(this);
   cb_ff02->setChecked(true);
   cb_ff02->setText(tr(""));
   cb_ff02->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_ff02->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_ff02, SIGNAL(clicked()), SLOT(set_ff02()));

	cnt_zAdjust= new QwtCounter(this);
	Q_CHECK_PTR(cnt_zAdjust);
	cnt_zAdjust->setRange(-10, 10, 0.01);
	cnt_zAdjust->setValue(zAdjust);
	cnt_zAdjust->setEnabled(true);
	cnt_zAdjust->setNumButtons(3);
	cnt_zAdjust->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_zAdjust, SIGNAL(valueChanged(double)), SLOT(set_zAdjust(double)));

	cnt_alphaAdjust= new QwtCounter(this);
	Q_CHECK_PTR(cnt_alphaAdjust);
	cnt_alphaAdjust->setRange(1, 200, 1);
	cnt_alphaAdjust->setValue(alphaAdjust);
	cnt_alphaAdjust->setEnabled(true);
	cnt_alphaAdjust->setNumButtons(3);
	cnt_alphaAdjust->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_alphaAdjust, SIGNAL(valueChanged(double)), SLOT(set_alphaAdjust(double)));

	cnt_betaAdjust= new QwtCounter(this);
	Q_CHECK_PTR(cnt_betaAdjust);
	cnt_betaAdjust->setRange(0.01, 10, 0.01);
	cnt_betaAdjust->setValue(betaAdjust);
	cnt_betaAdjust->setEnabled(true);
	cnt_betaAdjust->setNumButtons(3);
	cnt_betaAdjust->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_betaAdjust, SIGNAL(valueChanged(double)), SLOT(set_betaAdjust(double)));

	cnt_resolutionAdjust= new QwtCounter(this);
	Q_CHECK_PTR(cnt_resolutionAdjust);
	cnt_resolutionAdjust->setRange(50, 1000, 10);
	cnt_resolutionAdjust->setValue(resolutionAdjust);
	cnt_resolutionAdjust->setEnabled(true);
	cnt_resolutionAdjust->setNumButtons(3);
	cnt_resolutionAdjust->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_resolutionAdjust, SIGNAL(valueChanged(double)), SLOT(set_resolutionAdjust(double)));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_cancel = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_cancel);
	pb_cancel->setAutoDefault(false);
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

	pb_plot = new QPushButton(tr("3D Plot"), this);
	Q_CHECK_PTR(pb_plot);
	pb_plot->setAutoDefault(false);
	pb_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_plot->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_plot, SIGNAL(clicked()), SLOT(plot()));

	pb_residuals = new QPushButton(tr("Residuals Plot"), this);
	Q_CHECK_PTR(pb_residuals);
	pb_residuals->setAutoDefault(false);
	pb_residuals->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_residuals->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_residuals, SIGNAL(clicked()), SLOT(plot_residuals()));

	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);

	setup_GUI();
}

US_3d_Solutes::~US_3d_Solutes()
{
}

void US_3d_Solutes::setup_GUI()
{
	int rows = 11, columns = 6, spacing = 2, j=0;
	QGridLayout *controlGrid = new QGridLayout(this, rows, columns, spacing);

	controlGrid->addColSpacing(1, 15);
	controlGrid->addColSpacing(3, 15);
	controlGrid->addColSpacing(5, 5);
	controlGrid->addMultiCellWidget(lbl_info, j, j, 0, 5);
	j++;
	controlGrid->addWidget(lbl_parameter, j, 0);
	controlGrid->addWidget(lbl_dimension1, j, 2);
	controlGrid->addWidget(lbl_dimension2, j, 4);
	j++;
	controlGrid->addWidget(lbl_mw, j, 0);
	controlGrid->addWidget(cb_mw1, j, 2);
	controlGrid->addWidget(cb_mw2, j, 4);
	j++;
	controlGrid->addWidget(lbl_s, j, 0);
	controlGrid->addWidget(cb_s1, j, 2);
	controlGrid->addWidget(cb_s2, j, 4);
	j++;
	controlGrid->addWidget(lbl_D, j, 0);
	controlGrid->addWidget(cb_D1, j, 2);
	controlGrid->addWidget(cb_D2, j, 4);
	j++;
	controlGrid->addWidget(lbl_f, j, 0);
	controlGrid->addWidget(cb_f1, j, 2);
	controlGrid->addWidget(cb_f2, j, 4);
	j++;
	controlGrid->addWidget(lbl_ff0, j, 0);
	controlGrid->addWidget(cb_ff01, j, 2);
	controlGrid->addWidget(cb_ff02, j, 4);
	j++;
	controlGrid->addWidget(lbl_zAdjust, j, 0);
	controlGrid->addMultiCellWidget(cnt_zAdjust, j, j, 1, 4);
	j++;
	controlGrid->addWidget(lbl_resolutionAdjust, j, 0);
	controlGrid->addMultiCellWidget(cnt_resolutionAdjust, j, j, 1, 4);
	j++;
	controlGrid->addWidget(lbl_alphaAdjust, j, 0);
	controlGrid->addMultiCellWidget(cnt_alphaAdjust, j, j, 1, 4);
	j++;
	controlGrid->addWidget(lbl_betaAdjust, j, 0);
	controlGrid->addMultiCellWidget(cnt_betaAdjust, j, j, 1, 4);
	j++;
	controlGrid->addWidget(pb_help, j, 0);
	controlGrid->addMultiCellWidget(pb_residuals, j, j, 1, 4);
	j++;
	controlGrid->addWidget(pb_plot, j, 0);
	controlGrid->addMultiCellWidget(pb_cancel, j, j, 1, 4);
	
	for (int i=1; i<rows; i++)
	{
		controlGrid->setRowSpacing(i, 26);
	}
}

void US_3d_Solutes::closeEvent(QCloseEvent *e)
{
	*window_3d_flag = false;
	if(widget3d_flag)
	{
		mainwindow->close();
	}
	global_Xpos -= 30;
	global_Ypos -= 30;
	if(residual_plots_flag)
	{
		residual_plots->close();
	}
	e->accept();
}

void US_3d_Solutes::cancel()
{
	close();
}

void US_3d_Solutes::help()
{
	US_Help *online_help; 
	online_help = new US_Help(this);
	online_help->show_help("manual/3d_solutes.html");
}

void US_3d_Solutes::set_mw1()
{
	if (mw1_flag)
	{
		mw1_flag = false;
		cb_mw2->setEnabled(true);
	}
	else
	{
		mw1_flag = true;
		cb_mw2->setEnabled(false);
		cb_mw2->setChecked(false);

		cb_ff02->setEnabled(true);
		cb_s2->setEnabled(true);
		cb_D2->setEnabled(true);
		cb_f2->setEnabled(true);

		cb_s1->setChecked(false);
		cb_D1->setChecked(false);
		cb_f1->setChecked(false);
		cb_ff01->setChecked(false);

		s1_flag = false;
		D1_flag = false;
		f1_flag = false;
		ff01_flag = false;
	}
}
	
void US_3d_Solutes::plot_residuals()
{
	if(!residual_plots_flag)
	{
		residual_plots = new US_PlotData(run_id, scans, points, current_cell, current_lambda,
		&residual_plots_flag, absorbance, radius, fem_model, ti_noise, ri_noise);
		residual_plots->show();
	}
	else
	{
		residual_plots->raise();
	}
}

void US_3d_Solutes::set_mw2()
{
	if (mw2_flag)
	{
		mw2_flag = false;
		cb_mw1->setEnabled(true);
	}
	else
	{
		mw2_flag = true;

		cb_mw1->setEnabled(false);
		cb_mw1->setChecked(false);

		cb_s1->setEnabled(true);
		cb_D1->setEnabled(true);
		cb_f1->setEnabled(true);
		cb_ff01->setEnabled(true);

		cb_s2->setChecked(false);
		cb_D2->setChecked(false);
		cb_f2->setChecked(false);
		cb_ff02->setChecked(false);

		s2_flag = false;
		D2_flag = false;
		f2_flag = false;
		ff02_flag = false;
	}
}

void US_3d_Solutes::set_s1()
{
	if (s1_flag)
	{
		s1_flag = false;
		cb_s2->setEnabled(true);
	}
	else
	{
		s1_flag = true;
		cb_s2->setEnabled(false);
		cb_s2->setChecked(false);

		cb_mw2->setEnabled(true);
		cb_D2->setEnabled(true);
		cb_f2->setEnabled(true);
		cb_ff02->setEnabled(true);

		cb_mw1->setChecked(false);
		cb_D1->setChecked(false);
		cb_f1->setChecked(false);
		cb_ff01->setChecked(false);

		mw1_flag = false;
		D1_flag = false;
		f1_flag = false;
		ff01_flag = false;
	}
}

void US_3d_Solutes::set_s2()
{
	if (s2_flag)
	{
		s2_flag = false;
		cb_s1->setEnabled(true);
	}
	else
	{
		s2_flag = true;

		cb_s1->setEnabled(false);
		cb_s1->setChecked(false);

		cb_mw1->setEnabled(true);
		cb_D1->setEnabled(true);
		cb_f1->setEnabled(true);
		cb_ff01->setEnabled(true);

		cb_mw2->setChecked(false);
		cb_D2->setChecked(false);
		cb_f2->setChecked(false);
		cb_ff02->setChecked(false);

		mw2_flag = false;
		D2_flag = false;
		f2_flag = false;
		ff02_flag = false;
	}
}


void US_3d_Solutes::set_D1()
{
	if (D1_flag)
	{
		D1_flag = false;
		cb_D2->setEnabled(true);
	}
	else
	{
		D1_flag = true;
		cb_D2->setEnabled(false);
		cb_D2->setChecked(false);

		cb_mw2->setEnabled(true);
		cb_s2->setEnabled(true);
		cb_f2->setEnabled(true);
		cb_ff02->setEnabled(true);

		cb_mw1->setChecked(false);
		cb_s1->setChecked(false);
		cb_f1->setChecked(false);
		cb_ff01->setChecked(false);

		mw1_flag = false;
		s1_flag = false;
		f1_flag = false;
		ff01_flag = false;
	}
}

void US_3d_Solutes::set_D2()
{
	if (D2_flag)
	{
		D2_flag = false;
		cb_D1->setEnabled(true);
	}
	else
	{
		D2_flag = true;

		cb_D1->setEnabled(false);
		cb_D1->setChecked(false);

		cb_mw1->setEnabled(true);
		cb_s1->setEnabled(true);
		cb_f1->setEnabled(true);
		cb_ff01->setEnabled(true);

		cb_mw2->setChecked(false);
		cb_s2->setChecked(false);
		cb_f2->setChecked(false);
		cb_ff02->setChecked(false);

		mw2_flag = false;
		s2_flag = false;
		f2_flag = false;
		ff02_flag = false;
	}
}


void US_3d_Solutes::set_f1()
{
	if (f1_flag)
	{
		f1_flag = false;
		cb_f2->setEnabled(true);
	}
	else
	{
		f1_flag = true;
		cb_f2->setEnabled(false);
		cb_f2->setChecked(false);

		cb_mw2->setEnabled(true);
		cb_s2->setEnabled(true);
		cb_D2->setEnabled(true);
		cb_ff02->setEnabled(true);

		cb_mw1->setChecked(false);
		cb_s1->setChecked(false);
		cb_D1->setChecked(false);
		cb_ff01->setChecked(false);

		mw1_flag = false;
		s1_flag = false;
		D1_flag = false;
		ff01_flag = false;
	}
}

void US_3d_Solutes::set_f2()
{
	if (f2_flag)
	{
		f2_flag = false;
		cb_f1->setEnabled(true);
	}
	else
	{
		f2_flag = true;

		cb_f1->setEnabled(false);
		cb_f1->setChecked(false);

		cb_mw1->setEnabled(true);
		cb_s1->setEnabled(true);
		cb_D1->setEnabled(true);
		cb_ff01->setEnabled(true);

		cb_mw2->setChecked(false);
		cb_s2->setChecked(false);
		cb_D2->setChecked(false);
		cb_ff02->setChecked(false);

		mw2_flag = false;
		s2_flag = false;
		D2_flag = false;
		ff02_flag = false;
	}
}

void US_3d_Solutes::set_ff01()
{
	if (ff01_flag)
	{
		ff01_flag = false;
		cb_ff02->setEnabled(true);
	}
	else
	{
		ff01_flag = true;
		cb_ff02->setEnabled(false);
		cb_ff02->setChecked(false);

		cb_mw2->setEnabled(true);
		cb_s2->setEnabled(true);
		cb_D2->setEnabled(true);
		cb_f2->setEnabled(true);

		cb_mw1->setChecked(false);
		cb_s1->setChecked(false);
		cb_D1->setChecked(false);
		cb_f1->setChecked(false);

		mw1_flag = false;
		s1_flag = false;
		D1_flag = false;
		f1_flag = false;
	}
}

void US_3d_Solutes::set_ff02()
{
	if (ff02_flag)
	{
		ff02_flag = false;
		cb_ff01->setEnabled(true);
	}
	else
	{
		ff02_flag = true;

		cb_ff01->setEnabled(false);
		cb_ff01->setChecked(false);

		cb_mw1->setEnabled(true);
		cb_s1->setEnabled(true);
		cb_D1->setEnabled(true);
		cb_f1->setEnabled(true);

		cb_mw2->setChecked(false);
		cb_s2->setChecked(false);
		cb_D2->setChecked(false);
		cb_f2->setChecked(false);

		mw2_flag = false;
		s2_flag = false;
		D2_flag = false;
		f2_flag = false;
	}
}

void US_3d_Solutes::plot()
{
	QString xtitle, ytitle;
	struct element_3D tmp_solute_3D;
	solutes_3D.clear();
	for (unsigned int i=0; i<solutes.size(); i++)
	{
		if (solutes[i].c > 0.0)
		{
			tmp_solute_3D.z = solutes[i].c;
			if(mw1_flag)
			{
				tmp_solute_3D.x = solutes[i].mw;
			}
			if(s1_flag)
			{
				tmp_solute_3D.x = solutes[i].s;
			}
			if(D1_flag)
			{
				tmp_solute_3D.x = solutes[i].D;
			}
			if(f1_flag)
			{
				tmp_solute_3D.x = solutes[i].f;
			}
			if(ff01_flag)
			{
				tmp_solute_3D.x = solutes[i].ff0;
			}
			if(mw2_flag)
			{
				tmp_solute_3D.y = solutes[i].mw;
			}
			if(s2_flag)
			{
				tmp_solute_3D.y = solutes[i].s;
			}
			if(D2_flag)
			{
				tmp_solute_3D.y = solutes[i].D;
			}
			if(f2_flag)
			{
				tmp_solute_3D.y = solutes[i].f;
			}
			if(ff02_flag)
			{
				tmp_solute_3D.y = solutes[i].ff0;
			}
			solutes_3D.push_back(tmp_solute_3D);
		}
	}
	
	if ( !QGLFormat::hasOpenGL() ) 
	{
		qWarning( "This system has no OpenGL support. Exiting." );     
		return;
	}
	(*sa2d_ctrl_vars).maxx = 0.0;
	(*sa2d_ctrl_vars).minx = 1.0e20;
	(*sa2d_ctrl_vars).maxy = 0.0;
	(*sa2d_ctrl_vars).miny = 1.0e20;
	(*sa2d_ctrl_vars).maxz = 0.0;
	for (unsigned int i=0; i<solutes_3D.size(); i++)
	{

		(*sa2d_ctrl_vars).maxx = max((*sa2d_ctrl_vars).maxx, solutes_3D[i].x);
		(*sa2d_ctrl_vars).maxy = max((*sa2d_ctrl_vars).maxy, solutes_3D[i].y);
		(*sa2d_ctrl_vars).maxz = max((*sa2d_ctrl_vars).maxz, solutes_3D[i].z);
		(*sa2d_ctrl_vars).minx = min((*sa2d_ctrl_vars).minx, solutes_3D[i].x);
		(*sa2d_ctrl_vars).miny = min((*sa2d_ctrl_vars).miny, solutes_3D[i].y);
		
	}
	int xscale, yscale;
	
	xscale = (int) log10((*sa2d_ctrl_vars).maxx);
	yscale = (int) log10((*sa2d_ctrl_vars).maxy);
	if (xscale < 0) xscale --;
	if (yscale < 0) yscale --;
	(*sa2d_ctrl_vars).maxx /= pow(10.0, double(xscale));
	(*sa2d_ctrl_vars).maxy /= pow(10.0, double(yscale));
	(*sa2d_ctrl_vars).minx /= pow(10.0, double(xscale));
	(*sa2d_ctrl_vars).miny /= pow(10.0, double(yscale));
	for (unsigned int i=0; i<solutes_3D.size(); i++)
	{

		solutes_3D[i].x /= pow(10.0, double(xscale));
		solutes_3D[i].y /= pow(10.0, double(yscale));
	}
	
	(*sa2d_ctrl_vars).maxx = (*sa2d_ctrl_vars).maxx + (*sa2d_ctrl_vars).maxx/10.0;
	(*sa2d_ctrl_vars).maxy = (*sa2d_ctrl_vars).maxy + (*sa2d_ctrl_vars).maxy/10.0;
	(*sa2d_ctrl_vars).minx = (*sa2d_ctrl_vars).minx - (*sa2d_ctrl_vars).maxx/10.0;
	(*sa2d_ctrl_vars).miny = (*sa2d_ctrl_vars).miny - (*sa2d_ctrl_vars).maxy/10.0;
	if ((*sa2d_ctrl_vars).minx < 0) (*sa2d_ctrl_vars).minx = 0.0;
	if ((*sa2d_ctrl_vars).miny < 0) (*sa2d_ctrl_vars).miny = 0.0;
	QString str;
	if(mw1_flag)
	{
		xtitle = str.sprintf("MW * 1e%d", xscale);
	}
	if(s1_flag)
	{
		xtitle = str.sprintf("s * 1e%d", xscale);
	}
	if(D1_flag)
	{
		xtitle = str.sprintf("D * 1e%d", xscale);
	}
	if(f1_flag)
	{
		xtitle = str.sprintf("f * 1e%d", xscale);
	}
	if(ff01_flag)
	{
		xtitle = "f/f0";
	}
	if(mw2_flag)
	{
		ytitle = str.sprintf("MW * 1e%d", yscale);
	}
	if(s2_flag)
	{
		ytitle = str.sprintf("s * 1e%d", yscale);
	}
	if(D2_flag)
	{
		ytitle = str.sprintf("D * 1e%d", yscale);
	}
	if(f2_flag)
	{
		ytitle = str.sprintf("f * 1e%d", yscale);
	}
	if(ff02_flag)
	{
		ytitle = "f/f0";
	}
	(*sa2d_ctrl_vars).alpha = alphaAdjust; // increases the resolution/smoothness of peaks
	(*sa2d_ctrl_vars).beta = betaAdjust; // smaller number makes sharper peaks
	(*sa2d_ctrl_vars).zscaling = (*sa2d_ctrl_vars).maxz * pow(10.0, zAdjust);
	(*sa2d_ctrl_vars).meshx = (unsigned int) resolutionAdjust;
	(*sa2d_ctrl_vars).meshy = (unsigned int) resolutionAdjust;


	if (widget3d_flag)
	{
		mainwindow->setParameters(xtitle, ytitle, solutes_3D, sa2d_ctrl_vars);
		mainwindow->raise();		
	}
	else
	{
		/*
		cout << widget3d_flag << endl;
		cout << xtitle << endl;
		cout << ytitle << endl;
		for (unsigned int m=0; m< solutes_3D.size(); m++)
		{
			cout << m << ": " << solutes_3D[m].x << ",\t" << solutes_3D[m].y << ",\t" << solutes_3D[m].z << endl; 
		}
		cout << (*sa2d_ctrl_vars).min_s << endl;
		cout << (*sa2d_ctrl_vars).max_s << endl;
		cout << (*sa2d_ctrl_vars).min_ff0 << endl;
		cout << (*sa2d_ctrl_vars).max_ff0 << endl;
		cout << (*sa2d_ctrl_vars).ff0 << endl;
		cout << (*sa2d_ctrl_vars).resolution_s << endl;
		cout << (*sa2d_ctrl_vars).resolution_ff0 << endl;
		cout << (*sa2d_ctrl_vars).points << endl;
		cout << (*sa2d_ctrl_vars).meshx << endl;
		cout << (*sa2d_ctrl_vars).meshy << endl;
		cout << (*sa2d_ctrl_vars).fit_negBaseline << endl;
		cout << (*sa2d_ctrl_vars).fit_posBaseline << endl;
		cout << (*sa2d_ctrl_vars).minx << endl;
		cout << (*sa2d_ctrl_vars).miny << endl;
		
		cout << "maxx: " << (*sa2d_ctrl_vars).maxx << endl;
		cout << "maxy: " <<(*sa2d_ctrl_vars).maxy << endl;
		cout << "maxx: " <<(*sa2d_ctrl_vars).maxz << endl;
		
		cout << "zscaling: " <<(*sa2d_ctrl_vars).zscaling << endl;
		cout << "alpha: " <<(*sa2d_ctrl_vars).alpha << endl;
		cout << "beta: " <<(*sa2d_ctrl_vars).beta << endl;
		for (unsigned int m=0; m<5; m++)
		{
			for (unsigned int n=0; n<5; n++)
			{
				cout << m << ", " << n << ": " << (*sa2d_ctrl_vars).plot_id[m][n] << endl; 
			}
		}
		*/
		mainwindow = new Mesh2MainWindow(&widget3d_flag, xtitle, ytitle, solutes_3D, sa2d_ctrl_vars);
		mainwindow->show();
	}
}

void US_3d_Solutes::set_zAdjust(double val)
{
	zAdjust = val;
}

void US_3d_Solutes::set_resolutionAdjust(double val)
{
	resolutionAdjust = val;
}

void US_3d_Solutes::set_alphaAdjust(double val)
{
	alphaAdjust = val;
}

void US_3d_Solutes::set_betaAdjust(double val)
{
	betaAdjust = val;
}

