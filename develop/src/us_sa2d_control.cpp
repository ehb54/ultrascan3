#include "../include/us_sa2d_control.h"

US_Sa2d_Control::US_Sa2d_Control(SA2d_control_variables *tmp_sa2d_ctrl_vars, bool *tmp_control_window_flag,
QWidget *p, const char *name) : QFrame(p, name)
{
	USglobal=new US_Config();
	sa2d_ctrl_vars = tmp_sa2d_ctrl_vars;
	control_window_flag = tmp_control_window_flag;
	*control_window_flag = true;
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
	uniform = true;
	local_uniform = false;
	coalesce = false;
	random = false;
	regularize = false;
	widget3d_flag = false;
	autoupdate = false;
	fit_ti = false;
	fit_ri = false;
	clip = false;
	
	uniform_steps = 1.0;
	local_uniform_steps = 1.0;
	random_steps = 1.0;
	scaling_factor = 0.3;
	scaling_factor2 = 0.9;
	random_distance = 0.1;
	regfactor = 0.9;
	metric = 0.1;
//	threshold = 0.001;
	thread_count = 1;
	solutes_3D.clear();
	solutes.clear();

	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("2-D Spectrum Analysis Control Window"));


	progress = new QProgressBar(this, "Fitting Progress");
	progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	lbl_info = new QLabel(tr("2-D Spectrum Analysis Control Window"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setMinimumSize(250, 35);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_fitcontrol = new QLabel(tr("Fitting Controls:"), this);
	Q_CHECK_PTR(lbl_fitcontrol);
	lbl_fitcontrol->setAlignment(AlignCenter|AlignVCenter);
	lbl_fitcontrol->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_fitcontrol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_plotcontrol = new QLabel(tr("Plotting Controls:"), this);
	Q_CHECK_PTR(lbl_plotcontrol);
	lbl_plotcontrol->setAlignment(AlignCenter|AlignVCenter);
	lbl_plotcontrol->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_plotcontrol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_parameter = new QLabel(tr("Dimension:"), this);
	Q_CHECK_PTR(lbl_parameter);
	lbl_parameter->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_parameter->setAlignment(AlignLeft|AlignVCenter);
	lbl_parameter->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_parameter->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_optimization = new QLabel(tr("Optimization Methods:"), this);
	Q_CHECK_PTR(lbl_optimization);
	lbl_optimization->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_optimization->setAlignment(AlignCenter|AlignVCenter);
	lbl_optimization->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_optimization->setMinimumSize(150, 35);
	lbl_optimization->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

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

	lbl_min_s = new QLabel(tr(" Lower Limit (s):"), this);
	Q_CHECK_PTR(lbl_min_s);
	lbl_min_s->setAlignment(AlignLeft|AlignVCenter);
	lbl_min_s->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_min_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_uniform = new QLabel(tr("   Uniform Grid:"), this);
	Q_CHECK_PTR(lbl_uniform);
	lbl_uniform->setAlignment(AlignLeft|AlignVCenter);
	lbl_uniform->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_uniform->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_local_uniform = new QLabel(tr("   Local Uniform Grid:"), this);
	Q_CHECK_PTR(lbl_local_uniform);
	lbl_local_uniform->setAlignment(AlignLeft|AlignVCenter);
	lbl_local_uniform->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_local_uniform->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_random = new QLabel(tr("   Random Local Grid:   "), this);
	Q_CHECK_PTR(lbl_random);
	lbl_random->setAlignment(AlignLeft|AlignVCenter);
	lbl_random->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_random->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_regularize = new QLabel(tr("   Regularization:"), this);
	Q_CHECK_PTR(lbl_regularize);
	lbl_regularize->setAlignment(AlignLeft|AlignVCenter);
	lbl_regularize->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_regularize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_coalesce = new QLabel(tr("   Solute Coalescing:"), this);
	Q_CHECK_PTR(lbl_coalesce);
	lbl_coalesce->setAlignment(AlignLeft|AlignVCenter);
	lbl_coalesce->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_coalesce->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_clip = new QLabel(tr("   Clip lowest Conc. Solute:"), this);
	Q_CHECK_PTR(lbl_clip);
	lbl_clip->setAlignment(AlignLeft|AlignVCenter);
	lbl_clip->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_clip->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_uniform_steps = new QLabel(tr(" Grid Refinements:"), this);
	Q_CHECK_PTR(lbl_uniform_steps);
	lbl_uniform_steps->setAlignment(AlignLeft|AlignVCenter);
	lbl_uniform_steps->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_uniform_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_local_uniform_steps = new QLabel(tr(" Repetitions:"), this);
	Q_CHECK_PTR(lbl_local_uniform_steps);
	lbl_local_uniform_steps->setAlignment(AlignLeft|AlignVCenter);
	lbl_local_uniform_steps->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_local_uniform_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_random_steps = new QLabel(tr(" Repetitions:"), this);
	Q_CHECK_PTR(lbl_random_steps);
	lbl_random_steps->setAlignment(AlignLeft|AlignVCenter);
	lbl_random_steps->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_random_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_scaling_factor = new QLabel(tr(" Scaling Factor:"), this);
	Q_CHECK_PTR(lbl_scaling_factor);
	lbl_scaling_factor->setAlignment(AlignLeft|AlignVCenter);
	lbl_scaling_factor->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_scaling_factor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_scaling_factor2 = new QLabel(tr(" Scaling Factor 2:"), this);
	Q_CHECK_PTR(lbl_scaling_factor2);
	lbl_scaling_factor2->setAlignment(AlignLeft|AlignVCenter);
	lbl_scaling_factor2->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_scaling_factor2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_distance = new QLabel(tr(" Standard Deviation:"), this);
	Q_CHECK_PTR(lbl_distance);
	lbl_distance->setAlignment(AlignLeft|AlignVCenter);
	lbl_distance->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_distance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_metric = new QLabel(tr(" Coalescing Distance:"), this);
	Q_CHECK_PTR(lbl_metric);
	lbl_metric->setAlignment(AlignLeft|AlignVCenter);
	lbl_metric->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_metric->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
/*
	lbl_threshold = new QLabel(tr(" Concentration Threshold:"), this);
	Q_CHECK_PTR(lbl_threshold);
	lbl_threshold->setAlignment(AlignLeft|AlignVCenter);
	lbl_threshold->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_threshold->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
*/
	lbl_clip_steps = new QLabel(tr(" # of Clipped Solutes:"), this);
	Q_CHECK_PTR(lbl_clip_steps);
	lbl_clip_steps->setAlignment(AlignLeft|AlignVCenter);
	lbl_clip_steps->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_clip_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_regfactor = new QLabel(tr(" Regularization Factor:"), this);
	Q_CHECK_PTR(lbl_regfactor);
	lbl_regfactor->setAlignment(AlignLeft|AlignVCenter);
	lbl_regfactor->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_regfactor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_thread_count = new QLabel(tr(" Thread Count:"), this);
	Q_CHECK_PTR(lbl_thread_count);
	lbl_thread_count->setAlignment(AlignLeft|AlignVCenter);
	lbl_thread_count->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_thread_count->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_min_s= new QwtCounter(this);
	Q_CHECK_PTR(cnt_min_s);
	cnt_min_s->setRange(0.1, 9999.0, 0.1);
	cnt_min_s->setValue((*sa2d_ctrl_vars).min_s);
	cnt_min_s->setNumButtons(3);
	cnt_min_s->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_min_s, SIGNAL(valueChanged(double)), SLOT(update_min_s(double)));

	lbl_max_s = new QLabel(tr(" Upper Limit (s):"), this);
	Q_CHECK_PTR(lbl_max_s);
	lbl_max_s->setAlignment(AlignLeft|AlignVCenter);
	lbl_max_s->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_max_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_max_s= new QwtCounter(this);
	Q_CHECK_PTR(cnt_max_s);
	cnt_max_s->setRange(1.0, 10000, 1);
	cnt_max_s->setValue((*sa2d_ctrl_vars).max_s);
	cnt_max_s->setNumButtons(3);
	cnt_max_s->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_max_s, SIGNAL(valueChanged(double)), SLOT(update_max_s(double)));

	lbl_resolution_s = new QLabel(tr(" Increments (s):"), this);
	Q_CHECK_PTR(lbl_resolution_s);
	lbl_resolution_s->setAlignment(AlignLeft|AlignVCenter);
	lbl_resolution_s->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_resolution_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_resolution_s= new QwtCounter(this);
	Q_CHECK_PTR(cnt_resolution_s);
	cnt_resolution_s->setRange(1, 500, 1);
	cnt_resolution_s->setValue((*sa2d_ctrl_vars).resolution_s);
	cnt_resolution_s->setNumButtons(3);
	cnt_resolution_s->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_resolution_s, SIGNAL(valueChanged(double)), SLOT(update_resolution_s(double)));

	lbl_min_ff0 = new QLabel(tr(" Lower Limit (f/f0):"), this);
	Q_CHECK_PTR(lbl_min_ff0);
	lbl_min_ff0->setAlignment(AlignLeft|AlignVCenter);
	lbl_min_ff0->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_min_ff0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_min_ff0= new QwtCounter(this);
	Q_CHECK_PTR(cnt_min_ff0);
	cnt_min_ff0->setRange(1.0, 20.0, 0.01);
	cnt_min_ff0->setValue((*sa2d_ctrl_vars).min_ff0);
	cnt_min_ff0->setNumButtons(3);
	cnt_min_ff0->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_min_ff0, SIGNAL(valueChanged(double)), SLOT(update_min_ff0(double)));

	lbl_max_ff0 = new QLabel(tr(" Upper Limit (f/f0):"), this);
	Q_CHECK_PTR(lbl_max_ff0);
	lbl_max_ff0->setAlignment(AlignLeft|AlignVCenter);
	lbl_max_ff0->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_max_ff0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_max_ff0= new QwtCounter(this);
	Q_CHECK_PTR(cnt_max_ff0);
	cnt_max_ff0->setRange(1.01, 20, 0.01);
	cnt_max_ff0->setValue((*sa2d_ctrl_vars).max_ff0);
	cnt_max_ff0->setNumButtons(3);
	cnt_max_ff0->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_max_ff0, SIGNAL(valueChanged(double)), SLOT(update_max_ff0(double)));

	lbl_resolution_ff0 = new QLabel(tr(" Increments (f/f0):"), this);
	Q_CHECK_PTR(lbl_resolution_ff0);
	lbl_resolution_ff0->setAlignment(AlignLeft|AlignVCenter);
	lbl_resolution_ff0->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_resolution_ff0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_resolution_ff0= new QwtCounter(this);
	Q_CHECK_PTR(cnt_resolution_ff0);
	cnt_resolution_ff0->setRange(1, 500, 1);
	cnt_resolution_ff0->setValue((*sa2d_ctrl_vars).resolution_ff0);
	cnt_resolution_ff0->setNumButtons(3);
	cnt_resolution_ff0->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_resolution_ff0, SIGNAL(valueChanged(double)), SLOT(update_resolution_ff0(double)));
/*
   cb_posBaseline = new QCheckBox(this);
   cb_posBaseline->setText(tr("Fit pos. Baseline?"));
   cb_posBaseline->setChecked(false);
   cb_posBaseline->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_posBaseline->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_posBaseline, SIGNAL(clicked()), SLOT(set_posBaseline()));

   cb_negBaseline = new QCheckBox(this);
   cb_negBaseline->setChecked(false);
   cb_negBaseline->setText(tr("Fit neg. Baseline?"));
   cb_negBaseline->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_negBaseline->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_negBaseline, SIGNAL(clicked()), SLOT(set_negBaseline()));
*/
   cb_autoupdate = new QCheckBox(this);
   cb_autoupdate->setText(tr(" Automatically Update Plot"));
   cb_autoupdate->setChecked(false);
   cb_autoupdate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_autoupdate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_autoupdate, SIGNAL(clicked()), SLOT(set_autoupdate()));

   cb_fit_ti = new QCheckBox(this);
   cb_fit_ti->setText(tr(" Fit time invariant noise"));
   cb_fit_ti->setChecked(false);
   cb_fit_ti->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_fit_ti->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_fit_ti, SIGNAL(clicked()), SLOT(set_fit_ti()));

   cb_fit_ri = new QCheckBox(this);
   cb_fit_ri->setText(tr(" Fit radial invariant noise"));
   cb_fit_ri->setChecked(false);
   cb_fit_ri->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_fit_ri->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_fit_ri, SIGNAL(clicked()), SLOT(set_fit_ri()));

	lbl_mw = new QLabel(tr(" Molecular Weight:"), this);
	Q_CHECK_PTR(lbl_mw);
	lbl_mw->setAlignment(AlignLeft|AlignVCenter);
	lbl_mw->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

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

	pb_fit = new QPushButton(tr("Start Fit"), this);
	Q_CHECK_PTR(pb_fit);
	pb_fit->setAutoDefault(false);
	pb_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	pb_save = new QPushButton(tr("Save Results"), this);
	Q_CHECK_PTR(pb_save);
	pb_save->setAutoDefault(false);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	lbl_status = new QLabel(tr(" Status: "),this);
	lbl_status->setAlignment(AlignLeft|AlignVCenter);
	lbl_status->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_status->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_iteration1 = new QLabel(tr(" Iteration: "),this);
	lbl_iteration1->setAlignment(AlignLeft|AlignVCenter);
	lbl_iteration1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_iteration1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_iteration2 = new QLabel(tr(" "),this);
	lbl_iteration2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_iteration2->setAlignment(AlignCenter|AlignVCenter);
	lbl_iteration2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_iteration2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_variance1 = new QLabel(tr(" Old Variance: "),this);
	lbl_variance1->setAlignment(AlignLeft|AlignVCenter);
	lbl_variance1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_variance1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_variance2 = new QLabel(tr(" "),this);
	lbl_variance2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_variance2->setAlignment(AlignCenter|AlignVCenter);
	lbl_variance2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_variance2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_new_variance1 = new QLabel(tr(" New Variance: "),this);
	lbl_new_variance1->setAlignment(AlignLeft|AlignVCenter);
	lbl_new_variance1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_new_variance1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_new_variance2 = new QLabel(tr(" "),this);
	lbl_new_variance2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_new_variance2->setAlignment(AlignCenter|AlignVCenter);
	lbl_new_variance2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_new_variance2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_difference1 = new QLabel(tr(" Improvement: "),this);
	lbl_difference1->setAlignment(AlignLeft|AlignVCenter);
	lbl_difference1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_difference1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_difference2 = new QLabel(tr(" "),this);
	lbl_difference2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_difference2->setAlignment(AlignCenter|AlignVCenter);
	lbl_difference2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_difference2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_memory1 = new QLabel(tr(" Est. Memory (MB): "),this);
	lbl_memory1->setAlignment(AlignLeft|AlignVCenter);
	lbl_memory1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_memory1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_memory2 = new QLabel(tr(" "),this);
	lbl_memory2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_memory2->setAlignment(AlignCenter|AlignVCenter);
	lbl_memory2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_memory2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_cancel = new QPushButton(tr("Cancel"), this);
	Q_CHECK_PTR(pb_cancel);
	pb_cancel->setAutoDefault(false);
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

	pb_plot = new QPushButton(tr("Plot Results"), this);
	Q_CHECK_PTR(pb_plot);
	pb_plot->setAutoDefault(false);
	pb_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_plot->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_plot, SIGNAL(clicked()), SLOT(plot()));

	cnt_uniform_steps= new QwtCounter(this);
	Q_CHECK_PTR(cnt_uniform_steps);
	cnt_uniform_steps->setRange(1, 10, 1);
	cnt_uniform_steps->setValue(uniform_steps);
	cnt_uniform_steps->setNumButtons(3);
	cnt_uniform_steps->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_uniform_steps, SIGNAL(valueChanged(double)), SLOT(update_uniform_steps(double)));

	cnt_local_uniform_steps= new QwtCounter(this);
	Q_CHECK_PTR(cnt_local_uniform_steps);
	cnt_local_uniform_steps->setRange(1, 500, 1);
	cnt_local_uniform_steps->setValue(local_uniform_steps);
	cnt_local_uniform_steps->setNumButtons(3);
	cnt_local_uniform_steps->setEnabled(false);
	cnt_local_uniform_steps->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_local_uniform_steps, SIGNAL(valueChanged(double)), SLOT(update_local_uniform_steps(double)));

	cnt_random_steps= new QwtCounter(this);
	Q_CHECK_PTR(cnt_random_steps);
	cnt_random_steps->setRange(1, 500, 1);
	cnt_random_steps->setValue(random_steps);
	cnt_random_steps->setNumButtons(3);
	cnt_random_steps->setEnabled(false);
	cnt_random_steps->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_random_steps, SIGNAL(valueChanged(double)), SLOT(update_random_steps(double)));

	cnt_scaling_factor= new QwtCounter(this);
	Q_CHECK_PTR(cnt_scaling_factor);
	cnt_scaling_factor->setRange(0.05, 0.5, 0.001);
	cnt_scaling_factor->setValue(scaling_factor);
	cnt_scaling_factor->setNumButtons(3);
	cnt_scaling_factor->setEnabled(false);
	cnt_scaling_factor->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_scaling_factor, SIGNAL(valueChanged(double)), SLOT(update_scaling_factor(double)));

	cnt_scaling_factor2= new QwtCounter(this);
	Q_CHECK_PTR(cnt_scaling_factor2);
	cnt_scaling_factor2->setRange(0.1, 1.0, 0.001);
	cnt_scaling_factor2->setValue(scaling_factor2);
	cnt_scaling_factor2->setNumButtons(3);
	cnt_scaling_factor2->setEnabled(false);
	cnt_scaling_factor2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_scaling_factor2, SIGNAL(valueChanged(double)), SLOT(update_scaling_factor2(double)));
	
	cnt_random_distance= new QwtCounter(this);
	Q_CHECK_PTR(cnt_random_distance);
	cnt_random_distance->setRange(0.01, 0.5, 0.001);
	cnt_random_distance->setValue(random_distance);
	cnt_random_distance->setNumButtons(3);
	cnt_random_distance->setEnabled(false);
	cnt_random_distance->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_random_distance, SIGNAL(valueChanged(double)), SLOT(update_random_distance(double)));

	cnt_regfactor= new QwtCounter(this);
	Q_CHECK_PTR(cnt_regfactor);
	cnt_regfactor->setRange(0.0, 1.0, 0.001);
	cnt_regfactor->setValue(regfactor);
	cnt_regfactor->setNumButtons(3);
	cnt_regfactor->setEnabled(false);
	cnt_regfactor->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_regfactor, SIGNAL(valueChanged(double)), SLOT(update_regfactor(double)));

	cnt_thread_count= new QwtCounter(this);
	Q_CHECK_PTR(cnt_thread_count);
	cnt_thread_count->setRange(1, 32, 1);
	cnt_thread_count->setValue(thread_count);
	cnt_thread_count->setNumButtons(2);
	cnt_thread_count->setEnabled(true);
	cnt_thread_count->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_thread_count, SIGNAL(valueChanged(double)), SLOT(update_thread_count(double)));

	cnt_metric= new QwtCounter(this);
	Q_CHECK_PTR(cnt_metric);
	cnt_metric->setRange(0.01, 10, 0.01);
	cnt_metric->setValue(metric);
	cnt_metric->setNumButtons(3);
	cnt_metric->setEnabled(false);
	cnt_metric->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_metric, SIGNAL(valueChanged(double)), SLOT(update_metric(double)));

/*	cnt_threshold= new QwtCounter(this);
	Q_CHECK_PTR(cnt_threshold);
	cnt_threshold->setRange(1.0e-4, 10.0, 1.0e-4);
	cnt_threshold->setValue(threshold);
	cnt_threshold->setNumButtons(3);
	cnt_threshold->setEnabled(false);
	cnt_threshold->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_threshold, SIGNAL(valueChanged(double)), SLOT(update_threshold(double)));
*/
	cnt_clip_steps= new QwtCounter(this);
	Q_CHECK_PTR(cnt_clip_steps);
	cnt_clip_steps->setRange(1.0, 100.0, 1.0);
	cnt_clip_steps->setValue(clip_steps);
	cnt_clip_steps->setNumButtons(3);
	cnt_clip_steps->setEnabled(false);
	cnt_clip_steps->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_clip_steps, SIGNAL(valueChanged(double)), SLOT(update_clip_steps(double)));

   cb_uniform = new QCheckBox(this);
   cb_uniform->setChecked(true);
   cb_uniform->setText(tr(""));
   cb_uniform->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_uniform->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_uniform, SIGNAL(clicked()), SLOT(set_uniform()));

   cb_local_uniform = new QCheckBox(this);
   cb_local_uniform->setChecked(false);
   cb_local_uniform->setEnabled(false);
   cb_local_uniform->setText(tr(""));
   cb_local_uniform->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_local_uniform->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_local_uniform, SIGNAL(clicked()), SLOT(set_local_uniform()));

   cb_random = new QCheckBox(this);
   cb_random->setChecked(false);
   cb_random->setEnabled(false);
   cb_random->setText(tr(""));
   cb_random->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_random->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_random, SIGNAL(clicked()), SLOT(set_random()));

   cb_regularize = new QCheckBox(this);
   cb_regularize->setChecked(false);
   cb_regularize->setEnabled(false);
   cb_regularize->setText(tr(""));
   cb_regularize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_regularize->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_regularize, SIGNAL(clicked()), SLOT(set_regularize()));

   cb_coalesce = new QCheckBox(this);
   cb_coalesce->setChecked(false);
   cb_coalesce->setEnabled(false);
   cb_coalesce->setText(tr(""));
   cb_coalesce->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_coalesce->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_coalesce, SIGNAL(clicked()), SLOT(set_coalesce()));

   cb_clip = new QCheckBox(this);
   cb_clip->setChecked(false);
   cb_clip->setEnabled(false);
   cb_clip->setText(tr(""));
   cb_clip->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_clip->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_clip, SIGNAL(clicked()), SLOT(set_clip()));

	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);

	setup_GUI();
	calc_memory();
}

US_Sa2d_Control::~US_Sa2d_Control()
{
}

void US_Sa2d_Control::calc_memory()
{
	QString str;
//	cout << "Points: " << (*sa2d_ctrl_vars).points * (*sa2d_ctrl_vars).resolution_ff0 * (*sa2d_ctrl_vars).resolution_s << endl;
//	cout << "res-ff0: " <<  (*sa2d_ctrl_vars).resolution_ff0 << ", res-s: " <<   (*sa2d_ctrl_vars).resolution_s << endl;
	unsigned int terms = (*sa2d_ctrl_vars).resolution_ff0 * (*sa2d_ctrl_vars).resolution_s;
	if ((*sa2d_ctrl_vars).fit_negBaseline)
	{
		terms ++;
	}
	if ((*sa2d_ctrl_vars).fit_posBaseline)
	{
		terms ++;
	}
	str.sprintf("%6.2f MB", (*sa2d_ctrl_vars).points * terms * 7.70078e-6 + 16.3576);
	lbl_memory2->setText(str);
}

void US_Sa2d_Control::setup_GUI()
{

	QBoxLayout *background=new QHBoxLayout(this, 2);
	QBoxLayout *topbox=new QVBoxLayout(background, 2);
	topbox->addWidget(lbl_info);

	int rows = 14, columns = 9, spacing = 2, j=0;
	QGridLayout *controlGrid = new QGridLayout(topbox, rows, columns, spacing);

	controlGrid->addMultiCellWidget(lbl_fitcontrol, j, j, 1, 2, Qt::AlignJustify);
	controlGrid->addMultiCellWidget(lbl_plotcontrol, j, j, 3, 8, Qt::AlignJustify);
	controlGrid->setRowSpacing(j, 40);

	controlGrid->addColSpacing(0, 5);
	controlGrid->addColSpacing(3, 5);
	controlGrid->addColSpacing(5, 15);
	controlGrid->addColSpacing(7, 15);
	controlGrid->addColSpacing(9, 15);
	j++;
	controlGrid->addWidget(lbl_min_s, j, 1);
	controlGrid->addWidget(cnt_min_s, j, 2);
	controlGrid->addWidget(lbl_parameter, j, 4);
	controlGrid->addWidget(lbl_dimension1, j, 6);
	controlGrid->addWidget(lbl_dimension2, j, 8);
	j++;
	controlGrid->addWidget(lbl_max_s, j, 1);
	controlGrid->addWidget(cnt_max_s, j, 2);
	controlGrid->addWidget(lbl_mw, j, 4);
	controlGrid->addWidget(cb_mw1, j, 6);
	controlGrid->addWidget(cb_mw2, j, 8);
	j++;
	controlGrid->addWidget(lbl_resolution_s, j, 1);
	controlGrid->addWidget(cnt_resolution_s, j, 2);
	controlGrid->addWidget(lbl_s, j, 4);
	controlGrid->addWidget(cb_s1, j, 6);
	controlGrid->addWidget(cb_s2, j, 8);
	j++;
	controlGrid->addWidget(lbl_min_ff0, j, 1);
	controlGrid->addWidget(cnt_min_ff0, j, 2);
	controlGrid->addWidget(lbl_D, j, 4);
	controlGrid->addWidget(cb_D1, j, 6);
	controlGrid->addWidget(cb_D2, j, 8);
	j++;
	controlGrid->addWidget(lbl_max_ff0, j, 1);
	controlGrid->addWidget(cnt_max_ff0, j, 2);
	controlGrid->addWidget(lbl_f, j, 4);
	controlGrid->addWidget(cb_f1, j, 6);
	controlGrid->addWidget(cb_f2, j, 8);
	j++;
	controlGrid->addWidget(lbl_resolution_ff0, j, 1);
	controlGrid->addWidget(cnt_resolution_ff0, j, 2);
	controlGrid->addWidget(lbl_ff0, j, 4);
	controlGrid->addWidget(cb_ff01, j, 6);
	controlGrid->addWidget(cb_ff02, j, 8);
	j++;
	controlGrid->addWidget(lbl_thread_count, j, 1);
	controlGrid->addWidget(cnt_thread_count, j, 2);
	j++;
//	controlGrid->addWidget(cb_negBaseline, j, 1);
//	controlGrid->addWidget(cb_posBaseline, j, 2);
	controlGrid->addWidget(cb_fit_ti, j, 1);
	controlGrid->addWidget(cb_fit_ri, j, 2);
	controlGrid->addMultiCellWidget(cb_autoupdate, j, j, 4, 8, Qt::AlignJustify);
	j++;
	controlGrid->addWidget(lbl_memory1, j, 1);
	controlGrid->addWidget(lbl_memory2, j, 2);
	controlGrid->addWidget(pb_help, j, 4);
	j++;
	controlGrid->addWidget(lbl_iteration1, j, 1);
	controlGrid->addWidget(lbl_iteration2, j, 2);
	controlGrid->addWidget(pb_cancel, j, 4);
	j++;
	controlGrid->addWidget(lbl_variance1, j, 1);
	controlGrid->addWidget(lbl_variance2, j, 2);
	controlGrid->addWidget(pb_fit, j, 4);
	j++;
	controlGrid->addWidget(lbl_new_variance1, j, 1);
	controlGrid->addWidget(lbl_new_variance2, j, 2);
	controlGrid->addWidget(pb_plot, j, 4);
	j++;
	controlGrid->addWidget(lbl_difference1, j, 1);
	controlGrid->addWidget(lbl_difference2, j, 2);
	controlGrid->addWidget(pb_save, j, 4);
	j++;
	controlGrid->addWidget(lbl_status, j, 1);
	controlGrid->addWidget(progress, j, 2);
	for (int i=1; i<rows; i++)
	{
		controlGrid->setRowSpacing(i, 26);
	}
	j++;
	controlGrid->addRowSpacing(j, 5);
	QBoxLayout *topbox2=new QVBoxLayout(background, 2);
	topbox2->addWidget(lbl_optimization);
	rows = 28, columns = 7, spacing = 2, j=0;
	QGridLayout *optGrid = new QGridLayout(topbox2, rows, columns, spacing);
	optGrid->addColSpacing(0, 5);
	optGrid->addColSpacing(3, 5);
	optGrid->addColSpacing(6, 5);
	optGrid->addRowSpacing(j, 5);
	j++;
	optGrid->addWidget(lbl_uniform_steps, j, 4);
	j++;
	optGrid->addWidget(cb_uniform, j, 1);
	optGrid->addWidget(lbl_uniform, j, 2);
	optGrid->addWidget(cnt_uniform_steps, j, 4);
	j++;
	optGrid->addRowSpacing(j, 5);
	j++;
	optGrid->addWidget(lbl_local_uniform_steps, j, 4);
	j++;
	optGrid->addWidget(cb_local_uniform, j, 1);
	optGrid->addWidget(lbl_local_uniform, j, 2);
	optGrid->addWidget(cnt_local_uniform_steps, j, 4);
	j++;
	optGrid->addRowSpacing(j, 5);
	j++;
	optGrid->addWidget(lbl_scaling_factor, j, 4);
	j++;
	optGrid->addWidget(cnt_scaling_factor, j, 4);
	j++;
	optGrid->addRowSpacing(j, 5);
	j++;
	optGrid->addWidget(lbl_scaling_factor2, j, 4);
	j++;
	optGrid->addWidget(cnt_scaling_factor2, j, 4);
	j++;
	optGrid->addRowSpacing(j, 5);
	j++;
	optGrid->addWidget(lbl_random_steps, j, 4);
	j++;
	optGrid->addWidget(cb_random, j, 1);
	optGrid->addWidget(lbl_random, j, 2);
	optGrid->addWidget(cnt_random_steps, j, 4);
	j++;
	optGrid->addRowSpacing(j, 5);
	j++;
	optGrid->addWidget(lbl_distance, j, 4);
	j++;
	optGrid->addWidget(cnt_random_distance, j, 4);
	j++;
	optGrid->addRowSpacing(j, 5);
	j++;
	optGrid->addWidget(lbl_metric, j, 4);
	j++;
	optGrid->addWidget(cb_coalesce, j, 1);
	optGrid->addWidget(lbl_coalesce, j, 2);
	optGrid->addWidget(cnt_metric, j, 4);
	j++;
	optGrid->addRowSpacing(j, 5);
/*
	j++;
	optGrid->addWidget(lbl_threshold, j, 4);
	j++;
	optGrid->addWidget(cnt_threshold, j, 4);
	j++;
	optGrid->addRowSpacing(j, 5);
*/
	j++;
	optGrid->addWidget(lbl_clip_steps, j, 4);
	j++;
	optGrid->addWidget(cb_clip, j, 1);
	optGrid->addWidget(lbl_clip, j, 2);
	optGrid->addWidget(cnt_clip_steps, j, 4);
	j++;
	optGrid->addRowSpacing(j, 5);
	j++;
	optGrid->addWidget(lbl_regfactor, j, 4);
	j++;
	optGrid->addWidget(cb_regularize, j, 1);
	optGrid->addWidget(lbl_regularize, j, 2);
	optGrid->addWidget(cnt_regfactor, j, 4);
	j++;
	optGrid->addRowSpacing(j, 5);
}

void US_Sa2d_Control::closeEvent(QCloseEvent *e)
{
	*control_window_flag = false;
	if(widget3d_flag)
	{
		mainwindow->close();
	}
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

void US_Sa2d_Control::cancel()
{
	close();
}

void US_Sa2d_Control::help()
{
	US_Help *online_help; 
	online_help = new US_Help(this);
	online_help->show_help("manual/sa2d_control.html");
}

void US_Sa2d_Control::update_min_s(double val)
{
	(*sa2d_ctrl_vars).min_s = val;
	if ((*sa2d_ctrl_vars).min_s >= (*sa2d_ctrl_vars).max_s)
	{
		(*sa2d_ctrl_vars).max_s = (*sa2d_ctrl_vars).min_s + 1.0;
		cnt_max_s->setValue((double) (*sa2d_ctrl_vars).max_s);
	}
	emit mem_changed();

	emit distribution_changed();
}

void US_Sa2d_Control::update_max_s(double val)
{
	(*sa2d_ctrl_vars).max_s = val;
	if ((*sa2d_ctrl_vars).max_s <= (*sa2d_ctrl_vars).min_s)
	{
		(*sa2d_ctrl_vars).min_s = (*sa2d_ctrl_vars).max_s - 1.0;
		cnt_min_s->setValue((double) (*sa2d_ctrl_vars).min_s);
	}
	emit mem_changed();

	emit distribution_changed();
}

void US_Sa2d_Control::update_resolution_s(double val)
{
	(*sa2d_ctrl_vars).resolution_s = (unsigned int) val;
//	cout << "Resolution: " << (*sa2d_ctrl_vars).resolution_s  << endl;
	emit mem_changed();

	emit distribution_changed();
}

void US_Sa2d_Control::update_min_ff0(double val)
{
	(*sa2d_ctrl_vars).min_ff0 = val;
	if ((*sa2d_ctrl_vars).min_ff0 >= (*sa2d_ctrl_vars).max_ff0)
	{
		(*sa2d_ctrl_vars).max_ff0 = (*sa2d_ctrl_vars).min_ff0 + 1.0;
		cnt_max_ff0->setValue((double) (*sa2d_ctrl_vars).max_ff0);
	}
	emit mem_changed();

	emit distribution_changed();
}

void US_Sa2d_Control::update_max_ff0(double val)
{
	(*sa2d_ctrl_vars).max_ff0 = val;
	if ((*sa2d_ctrl_vars).max_ff0 <= (*sa2d_ctrl_vars).min_ff0)
	{
		(*sa2d_ctrl_vars).min_ff0 = (*sa2d_ctrl_vars).max_ff0 - 1.0;
		if ((*sa2d_ctrl_vars).min_ff0 < 1.0)
		{
			(*sa2d_ctrl_vars).min_ff0 = 1.0;
		}
		cnt_min_ff0->setValue((double) (*sa2d_ctrl_vars).min_ff0);
	}
	emit mem_changed();

	emit distribution_changed();
}

void US_Sa2d_Control::set_posBaseline()
{
	if ((*sa2d_ctrl_vars).fit_posBaseline)
	{
		(*sa2d_ctrl_vars).fit_posBaseline = false;
	}
	else
	{
		(*sa2d_ctrl_vars).fit_posBaseline = true;
		(*sa2d_ctrl_vars).fit_negBaseline = false;
		cb_negBaseline->setChecked(false);
	}
	emit mem_changed();
}

void US_Sa2d_Control::set_negBaseline()
{
	if ((*sa2d_ctrl_vars).fit_negBaseline)
	{
		(*sa2d_ctrl_vars).fit_negBaseline = false;
	}
	else
	{
		(*sa2d_ctrl_vars).fit_negBaseline = true;
		(*sa2d_ctrl_vars).fit_posBaseline = false;
		cb_posBaseline->setChecked(false);
	}
	emit mem_changed();
}

void US_Sa2d_Control::update_resolution_ff0(double val)
{
	(*sa2d_ctrl_vars).resolution_ff0 = (unsigned int) val;
	emit mem_changed();

	emit distribution_changed();
}

void US_Sa2d_Control::set_mw1()
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

void US_Sa2d_Control::set_mw2()
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

void US_Sa2d_Control::set_s1()
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

void US_Sa2d_Control::set_s2()
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


void US_Sa2d_Control::set_D1()
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

void US_Sa2d_Control::set_D2()
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


void US_Sa2d_Control::set_f1()
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

void US_Sa2d_Control::set_f2()
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


void US_Sa2d_Control::set_ff01()
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

void US_Sa2d_Control::set_ff02()
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

void US_Sa2d_Control::plot()
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
	(*sa2d_ctrl_vars).alpha = 50.0; // increases the resolution/smoothness of peaks
	(*sa2d_ctrl_vars).beta = 0.10; // smaller number makes sharper peaks
	(*sa2d_ctrl_vars).zscaling = 10.0/(*sa2d_ctrl_vars).maxz;
	(*sa2d_ctrl_vars).meshx = 80;
	(*sa2d_ctrl_vars).meshy = 80;


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
		cout << (*sa2d_ctrl_vars).maxx << endl;
		cout << (*sa2d_ctrl_vars).maxy << endl;
		cout << (*sa2d_ctrl_vars).maxz << endl;
		cout << (*sa2d_ctrl_vars).zscaling << endl;
		cout << (*sa2d_ctrl_vars).alpha << endl;
		cout << (*sa2d_ctrl_vars).beta << endl;
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

void US_Sa2d_Control::set_random()
{
	if(random)
	{
		random = false;
		cnt_random_steps->setEnabled(false);
		cnt_random_distance->setEnabled(false);
	}
	else
	{
		cnt_uniform_steps->setEnabled(false);
		cnt_clip_steps->setEnabled(false);
		cnt_local_uniform_steps->setEnabled(false);
		cnt_scaling_factor->setEnabled(false);
		cnt_scaling_factor2->setEnabled(false);
		cnt_random_steps->setEnabled(true);
		cnt_random_distance->setEnabled(true);
		cnt_metric->setEnabled(false);
//		cnt_threshold->setEnabled(false);
		cnt_regfactor->setEnabled(false);

		uniform = false;
		local_uniform = false;
		clip = false;
		coalesce = false;
		random = true;
		regularize = false;

		cb_uniform->setChecked(false);
		cb_local_uniform->setChecked(false);
		cb_coalesce->setChecked(false);
		cb_clip->setChecked(false);
		cb_regularize->setChecked(false);
	}
}

void US_Sa2d_Control::set_uniform()
{
	if(uniform)
	{
		uniform = false;
		cnt_uniform_steps->setEnabled(false);
	}
	else
	{
		cnt_uniform_steps->setEnabled(true);
		cnt_local_uniform_steps->setEnabled(false);
		cnt_clip_steps->setEnabled(false);
		cnt_scaling_factor->setEnabled(false);
		cnt_scaling_factor2->setEnabled(false);
		cnt_random_steps->setEnabled(false);
		cnt_random_distance->setEnabled(false);
		cnt_metric->setEnabled(false);
//		cnt_threshold->setEnabled(false);
		cnt_regfactor->setEnabled(false);

		uniform = true;
		local_uniform = false;
		clip = false;
		coalesce = false;
		random = false;
		regularize = false;

		cb_random->setChecked(false);
		cb_local_uniform->setChecked(false);
		cb_coalesce->setChecked(false);
		cb_clip->setChecked(false);
		cb_regularize->setChecked(false);
	}
}

void US_Sa2d_Control::set_local_uniform()
{
	if(local_uniform)
	{
		local_uniform = false;
		cnt_local_uniform_steps->setEnabled(false);
		cnt_scaling_factor->setEnabled(false);
		cnt_scaling_factor2->setEnabled(false);
	}
	else
	{
		cnt_uniform_steps->setEnabled(false);
		cnt_local_uniform_steps->setEnabled(true);
		cnt_clip_steps->setEnabled(false);
		cnt_scaling_factor->setEnabled(true);
		cnt_scaling_factor2->setEnabled(true);
		cnt_random_steps->setEnabled(false);
		cnt_random_distance->setEnabled(false);
		cnt_metric->setEnabled(false);
//		cnt_threshold->setEnabled(false);
		cnt_regfactor->setEnabled(false);

		uniform = false;
		local_uniform = true;
		coalesce = false;
		clip = false;
		random = false;
		regularize = false;

		cb_uniform->setChecked(false);
		cb_random->setChecked(false);
		cb_coalesce->setChecked(false);
		cb_clip->setChecked(false);
		cb_regularize->setChecked(false);
	}
}

void US_Sa2d_Control::set_coalesce()
{
	if(coalesce)
	{
		coalesce = false;
		cnt_metric->setEnabled(false);
//		cnt_threshold->setEnabled(false);
	}
	else
	{
		cnt_uniform_steps->setEnabled(false);
		cnt_local_uniform_steps->setEnabled(false);
		cnt_clip_steps->setEnabled(false);
		cnt_scaling_factor->setEnabled(false);
		cnt_scaling_factor2->setEnabled(false);
		cnt_random_steps->setEnabled(false);
		cnt_random_distance->setEnabled(false);
		cnt_metric->setEnabled(true);
//		cnt_threshold->setEnabled(true);
		cnt_regfactor->setEnabled(false);

		uniform = false;
		local_uniform = false;
		coalesce = true;
		clip = false;
		random = false;
		regularize = false;

		cb_uniform->setChecked(false);
		cb_local_uniform->setChecked(false);
		cb_random->setChecked(false);
		cb_clip->setChecked(false);
		cb_regularize->setChecked(false);
	}
}

void US_Sa2d_Control::set_clip()
{
	if(clip)
	{
		clip = false;
		cnt_clip_steps->setEnabled(false);
	}
	else
	{
		cnt_uniform_steps->setEnabled(false);
		cnt_local_uniform_steps->setEnabled(false);
		cnt_scaling_factor->setEnabled(false);
		cnt_scaling_factor2->setEnabled(false);
		cnt_random_steps->setEnabled(false);
		cnt_random_distance->setEnabled(false);
		cnt_metric->setEnabled(false);
//		cnt_threshold->setEnabled(false);
		cnt_clip_steps->setEnabled(true);
		cnt_regfactor->setEnabled(false);

		uniform = false;
		local_uniform = false;
		coalesce = false;
		clip = true;
		random = false;
		regularize = false;

		cb_uniform->setChecked(false);
		cb_coalesce->setChecked(false);
		cb_local_uniform->setChecked(false);
		cb_random->setChecked(false);
		cb_regularize->setChecked(false);
	}
}

void US_Sa2d_Control::set_regularize()
{
	if(regularize)
	{
		regularize = false;
		cnt_regfactor->setEnabled(false);
	}
	else
	{
		cnt_uniform_steps->setEnabled(false);
		cnt_local_uniform_steps->setEnabled(false);
		cnt_clip_steps->setEnabled(false);
		cnt_scaling_factor->setEnabled(false);
		cnt_scaling_factor2->setEnabled(false);
		cnt_random_steps->setEnabled(false);
		cnt_random_distance->setEnabled(false);
		cnt_metric->setEnabled(false);
//		cnt_threshold->setEnabled(false);
		cnt_regfactor->setEnabled(true);

		uniform = false;
		local_uniform = false;
		coalesce = false;
		clip = false;
		random = false;
		regularize = true;

		cb_clip->setChecked(false);
		cb_uniform->setChecked(false);
		cb_local_uniform->setChecked(false);
		cb_random->setChecked(false);
		cb_coalesce->setChecked(false);
	}
}

void US_Sa2d_Control::set_autoupdate()
{
	if(autoupdate)
	{
		autoupdate = false;
	}
	else
	{
		autoupdate = true;
	}
}

void US_Sa2d_Control::set_fit_ti()
{
	if(fit_ti)
	{
		fit_ti = false;
		fit_ri = false;
		cb_fit_ri->setChecked(false);
	}
	else
	{
		fit_ti = true;
	}
}

void US_Sa2d_Control::set_fit_ri()
{
	if(fit_ri)
	{
		fit_ri = false;
	}
	else
	{
		fit_ti = true;
		fit_ri = true;
		cb_fit_ti->setChecked(true);
	}
}

void US_Sa2d_Control::update_uniform_steps(double val)
{
	uniform_steps = val;
}

void US_Sa2d_Control::update_local_uniform_steps(double val)
{
	local_uniform_steps = val;
}

void US_Sa2d_Control::update_random_steps(double val)
{
	random_steps = val;
}

void US_Sa2d_Control::update_clip_steps(double val)
{
	clip_steps = val;
}

void US_Sa2d_Control::update_scaling_factor(double val)
{
	scaling_factor = val;
}

void US_Sa2d_Control::update_scaling_factor2(double val)
{
	scaling_factor2 = val;
}

void US_Sa2d_Control::update_random_distance(double val)
{
	random_distance = val;
}

void US_Sa2d_Control::update_regfactor(double val)
{
	regfactor = val;
}

void US_Sa2d_Control::update_metric(double val)
{
	metric = val;
}
/*
void US_Sa2d_Control::update_threshold(double val)
{
	threshold = val;
}
*/

void US_Sa2d_Control::update_thread_count(double val)
{
	thread_count = val;
}
