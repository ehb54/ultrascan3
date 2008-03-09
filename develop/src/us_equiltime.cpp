#include "../include/us_equiltime.h"

US_EquilTime::US_EquilTime(QWidget *parent, const char *name) 
: QFrame( parent, name)
{
	USglobal = new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	molecule       = 1; // 1=prolate, 2=oblate, 3=rod, 4=sphere
	rpm_start      = 18000;
	rpm_stop       = 36000;
	use_sigma      = true;
	sigma_start    = 1;
	sigma_stop     = 4;
	speed_steps    = 5;
	time_increment = 15;
	monitor_flag   = true;
	delta_t        = 15;
	delta_r        = (float) 0.001;
	tolerance      = (float) 5e-4;
	sigma          = new float [100];
	rpm            = new unsigned int [100];
	meniscus       = 5.9;
	bottom         = 6.2;
	conc           = (float) 0.15;

	GUI();	
	global_Xpos += 30;
	global_Ypos += 30;
	move(global_Xpos, global_Ypos);
}

US_EquilTime::~US_EquilTime()
{
}

void US_EquilTime::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_EquilTime::GUI()
{
	data_plot = new QwtPlot(this);
	data_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	data_plot->enableGridXMin();
	data_plot->enableGridYMin();
	data_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	data_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	data_plot->setCanvasBackground(USglobal->global_colors.plot);
	data_plot->setMargin(USglobal->config_list.margin);
	data_plot->enableOutline(true);
	data_plot->setOutlinePen(white);
	data_plot->setOutlineStyle(Qwt::Cross);
	data_plot->setAxisTitle(QwtPlot::xBottom, "Radius");
	data_plot->setAxisTitle(QwtPlot::yLeft, "Concentration");
	data_plot->setTitle(tr("Approach to Equilibrium Simulation"));
	data_plot->setAxisScale(QwtPlot::yLeft, 0, 1.5, 0);
	data_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	data_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));		
	data_plot->show();
	
	textwindow = new US_Editor(2, this);
	textwindow->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	textwindow->e->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	textwindow->show();
	
	banner1 = new QLabel(tr("Simulation Settings:"),this);
	banner1->setFrameStyle(QFrame::WinPanel|Raised);
	banner1->setAlignment(AlignCenter|AlignVCenter);
	banner1->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	banner1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
		
	pb_simcomp = new QPushButton(tr("Simulate Component"), this);
	pb_simcomp->setAutoDefault(false);
	pb_simcomp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_simcomp->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_simcomp, SIGNAL(clicked()), SLOT(simulate_component()));

	lbl_mw1 = new QLabel(tr("Molecular Weight:"),this);
	lbl_mw1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_mw1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_mw2 = new QLabel("<not selected>",this);
	lbl_mw2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_mw2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_mw2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		
	lbl_sed1 = new QLabel(tr("Sedimentation Coeff.:"),this);
	lbl_sed1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_sed1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sed1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_sed2 = new QLabel("<not selected>",this);
	lbl_sed2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_sed2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_sed2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	
	lbl_diff1 = new QLabel(tr("Diffusion Coeff.:"),this);
	lbl_diff1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_diff1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));	
	lbl_diff1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_diff2 = new QLabel("<not selected>",this);
	lbl_diff2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_diff2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_diff2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	
	cb_prolate = new QCheckBox(tr("Prolate Ellipsoid"),this);
	cb_prolate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_prolate->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_prolate->setChecked(true);
	cb_prolate->setEnabled(false);
	connect(cb_prolate, SIGNAL(clicked()), SLOT(select_prolate()));

	cb_oblate = new QCheckBox(tr("Oblate Ellipsoid"),this);
	cb_oblate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_oblate->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_oblate->setEnabled(false);
	cb_oblate->setChecked(false);
	connect(cb_oblate, SIGNAL(clicked()), SLOT(select_oblate()));
	
	cb_rod = new QCheckBox(tr("Long Rod"),this);
	cb_rod->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_rod->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_rod->setChecked(false);
	cb_rod->setEnabled(false);
	connect(cb_rod, SIGNAL(clicked()), SLOT(select_rod()));

	cb_sphere = new QCheckBox(tr("Sphere"),this);
	cb_sphere->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_sphere->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_sphere->setChecked(false);
	cb_sphere->setEnabled(false);
	connect(cb_sphere, SIGNAL(clicked()), SLOT(select_sphere()));

	banner2 = new QLabel(tr("Radius Information:"),this);
	banner2->setFrameStyle(QFrame::WinPanel|Raised);
	banner2->setAlignment(AlignCenter|AlignVCenter);
	banner2->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	banner2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	

	cb_inner = new QCheckBox(tr("Inner Channel"),this);
	cb_inner->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_inner->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_inner->setChecked(true);
	connect(cb_inner, SIGNAL(clicked()), SLOT(select_inner()));

	cb_center = new QCheckBox(tr("Center Channel"),this);
	cb_center->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_center->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_center->setChecked(false);
	connect(cb_center, SIGNAL(clicked()), SLOT(select_center()));

	cb_outer = new QCheckBox(tr("Outer Channel"),this);
	cb_outer->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_outer->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_outer->setChecked(false);
	connect(cb_outer, SIGNAL(clicked()), SLOT(select_outer()));

	cb_custom = new QCheckBox(tr("Custom"),this);
	cb_custom->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_custom->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_custom->setChecked(false);
	connect(cb_custom, SIGNAL(clicked()), SLOT(select_custom()));

	lbl_topradius = new QLabel(tr("Top Radius:"),this);
	lbl_topradius->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_topradius->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_topradius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	ct_topradius= new QwtCounter(this);
	ct_topradius->setNumButtons(3);
	ct_topradius->setRange(5.8, 7.3, 0.01);
	ct_topradius->setValue(meniscus);
	ct_topradius->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(ct_topradius, SIGNAL(valueChanged(double)), SLOT(update_topradius(double)));

	lbl_bottomradius = new QLabel(tr("Bottom Radius:"),this);
	lbl_bottomradius->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_bottomradius->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_bottomradius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
		
	ct_bottomradius= new QwtCounter(this);
	ct_bottomradius->setNumButtons(3);
	ct_bottomradius->setRange(5.8, 7.3, 0.01);
	ct_bottomradius->setValue(bottom);
	ct_bottomradius->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(ct_bottomradius, SIGNAL(valueChanged(double)), SLOT(update_bottomradius(double)));

	banner3 = new QLabel(tr("Rotorspeed Information:"),this);
	banner3->setFrameStyle(QFrame::WinPanel|Raised);
	banner3->setAlignment(AlignCenter|AlignVCenter);
	banner3->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	banner3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	
	cb_sigma = new QCheckBox(tr("Use Sigma"),this);
	cb_sigma->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_sigma->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_sigma->setChecked(true);
	connect(cb_sigma, SIGNAL(clicked()), SLOT(select_rpm()));

	cb_rpm = new QCheckBox(tr("Use RPM"),this);
	cb_rpm->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_rpm->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_rpm->setChecked(false);
	connect(cb_rpm, SIGNAL(clicked()), SLOT(select_rpm()));

	lbl_speedstart = new QLabel(tr("Low Speed (sigma):"),this);
	lbl_speedstart->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_speedstart->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_speedstart->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
		
	ct_speedstart= new QwtCounter(this);
	ct_speedstart->setNumButtons(3);
	ct_speedstart->setRange(0.01, 10, 0.01);
	ct_speedstart->setValue(1.0);
	ct_speedstart->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(ct_speedstart, SIGNAL(valueChanged(double)), SLOT(update_speedstart(double)));

	lbl_speedstop = new QLabel(tr("High Speed (sigma):"),this);
	lbl_speedstop->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_speedstop->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_speedstop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	ct_speedstop= new QwtCounter(this);
	ct_speedstop->setNumButtons(3);
	ct_speedstop->setRange(0.1, 10, 0.01);
	ct_speedstop->setValue(sigma_stop);
	ct_speedstop->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(ct_speedstop, SIGNAL(valueChanged(double)), SLOT(update_speedstop(double)));

	lbl_speedsteps = new QLabel(tr("Speed Steps:"),this);
	lbl_speedsteps->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_speedsteps->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_speedsteps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	ct_speedsteps= new QwtCounter(this);
	ct_speedsteps->setNumButtons(3);
	ct_speedsteps->setRange(1, 100, 1);
	ct_speedsteps->setValue(speed_steps);
	ct_speedsteps->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(ct_speedsteps, SIGNAL(valueChanged(double)), SLOT(update_speedsteps(double)));

	lbl_speedlist = new QLabel(tr("Current Speed List:"),this);
	lbl_speedlist->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_speedlist->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_speedlist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lb_speeds = new QListBox(this, "Speeds");
	lb_speeds->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_speeds->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	calc_speeds();

	banner4 = new QLabel(tr("Simulation Settings:"),this);
	banner4->setFrameStyle(QFrame::WinPanel|Raised);
	banner4->setAlignment(AlignCenter|AlignVCenter);
	banner4->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	banner4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	
	lbl_tolerance = new QLabel(tr("Tolerance:"),this);
	lbl_tolerance->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_tolerance->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_tolerance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	ct_tolerance= new QwtCounter(this);
	ct_tolerance->setNumButtons(3);
	ct_tolerance->setRange(1e-5, 0.01, 1e-5);
	ct_tolerance->setValue(tolerance);
	ct_tolerance->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(ct_tolerance, SIGNAL(valueChanged(double)), SLOT(update_tolerance(double)));

	
	lbl_timesteps = new QLabel(tr("Time Increment (min):"),this);
	lbl_timesteps->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_timesteps->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_timesteps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	ct_timesteps= new QwtCounter(this);
	ct_timesteps->setNumButtons(3);
	ct_timesteps->setRange(1, 1000, 1);
	ct_timesteps->setValue(time_increment);
	ct_timesteps->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(ct_timesteps, SIGNAL(valueChanged(double)), SLOT(update_timesteps(double)));

	
	lbl_delta_r = new QLabel(tr("Delta-r (cm):"),this);
	lbl_delta_r->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_delta_r->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_delta_r->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
		
	ct_delta_r= new QwtCounter(this);
	ct_delta_r->setNumButtons(3);
	ct_delta_r->setRange(0.0001, 0.01, 0.0001);
	ct_delta_r->setValue(delta_r);
	ct_delta_r->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(ct_delta_r, SIGNAL(valueChanged(double)), SLOT(update_delta_r(double)));

		
	lbl_delta_t = new QLabel(tr("Delta-t (seconds):"),this);
	lbl_delta_t->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_delta_t->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_delta_t->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	ct_delta_t= new QwtCounter(this);
	ct_delta_t->setNumButtons(3);
	ct_delta_t->setRange(1, 50, 1);
	ct_delta_t->setValue(delta_t);
	ct_delta_t->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(ct_delta_t, SIGNAL(valueChanged(double)), SLOT(update_delta_t(double)));

	cb_monitor = new QCheckBox(tr("Monitor Simulation Progress"),this);
	cb_monitor->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_monitor->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_monitor->setChecked(true);
	connect(cb_monitor, SIGNAL(clicked()), SLOT(select_monitor()));

	pb_help = new QPushButton(tr("Help"), this);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));
		
	pb_estimate = new QPushButton(tr("Estimate Times"), this);
	pb_estimate->setAutoDefault(false);
	pb_estimate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_estimate->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_estimate->setEnabled(false);
	connect(pb_estimate, SIGNAL(clicked()), SLOT(simulate_times()));

	pb_save = new QPushButton(tr("Save to File"), this);
	pb_save->setAutoDefault(false);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_save->setEnabled(false);
	connect(pb_save, SIGNAL(clicked()), SLOT(save()));
		
	pb_quit = new QPushButton(tr("Close"), this);
	pb_quit->setAutoDefault(false);
	pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

	setup_GUI();
}

void US_EquilTime::setup_GUI()
{
	int j=0;
	int rows = 13, columns = 4, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,2,2,spacing);	
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->addMultiCellWidget(banner1,j,j,0,3);
	j++;
	subGrid1->addMultiCellWidget(pb_simcomp,j,j,0,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_mw1,j,j,0,1);
	subGrid1->addMultiCellWidget(lbl_mw2,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_sed1,j,j,0,1);
	subGrid1->addMultiCellWidget(lbl_sed2,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_diff1,j,j,0,1);
	subGrid1->addMultiCellWidget(lbl_diff2,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(cb_prolate,j,j,0,1);
	subGrid1->addMultiCellWidget(cb_oblate,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(cb_rod,j,j,0,1);
	subGrid1->addMultiCellWidget(cb_sphere,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(banner2,j,j,0,3);
	j++;
	subGrid1->addMultiCellWidget(cb_inner,j,j,0,1);
	subGrid1->addMultiCellWidget(cb_center,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(cb_outer,j,j,0,1);
	subGrid1->addMultiCellWidget(cb_custom,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_topradius,j,j,0,1);
	subGrid1->addMultiCellWidget(ct_topradius,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_bottomradius,j,j,0,1);
	subGrid1->addMultiCellWidget(ct_bottomradius,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(banner3,j,j,0,3);
	j++;
	subGrid1->addMultiCellWidget(cb_sigma,j,j,0,1);
	subGrid1->addMultiCellWidget(cb_rpm,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_speedstart,j,j,0,1);
	subGrid1->addMultiCellWidget(ct_speedstart,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_speedstop,j,j,0,1);
	subGrid1->addMultiCellWidget(ct_speedstop,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_speedsteps,j,j,0,1);
	subGrid1->addMultiCellWidget(ct_speedsteps,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_speedlist,j,j,0,1);
	subGrid1->addMultiCellWidget(lb_speeds,j,j+2,2,3);
	j=j+2;
	subGrid1->addMultiCellWidget(banner4,j,j,0,3);
	
	rows = 7, columns = 4, spacing = 2, j=0;
	QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid2->setRowSpacing(i, 26);
	}
	subGrid2->addMultiCellWidget(banner4,j,j,0,3);
	j++;
	subGrid2->addMultiCellWidget(lbl_tolerance,j,j,0,1);
	subGrid2->addMultiCellWidget(ct_tolerance,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(lbl_timesteps,j,j,0,1);
	subGrid2->addMultiCellWidget(ct_timesteps,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(lbl_delta_r,j,j,0,1);
	subGrid2->addMultiCellWidget(ct_delta_r,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(lbl_delta_t,j,j,0,1);
	subGrid2->addMultiCellWidget(ct_delta_t,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(cb_monitor,j,j,0,3);
	j++;
	subGrid2->addMultiCellWidget(pb_help,j,j,0,1);
	subGrid2->addMultiCellWidget(pb_estimate,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(pb_save,j,j,0,1);
	subGrid2->addMultiCellWidget(pb_quit,j,j,2,3);

	background->addLayout(subGrid1,0,0);
	background->addWidget(data_plot,0,1);
	background->addLayout(subGrid2,1,0);
	background->addWidget(textwindow,1,1);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(0,350);
	background->setColSpacing(1,530);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width()+530+spacing*3, this->height());


}

void US_EquilTime::simulate_component()
{
	US_Hydro1 *hydro1;
	hydro1 = new US_Hydro1(&simcomp);
	connect(hydro1, SIGNAL(updated()), SLOT(update_component()));
	hydro1->show();
	hydro1->update();
	cb_prolate->setEnabled(true);
	cb_oblate->setEnabled(true);
	cb_rod->setEnabled(true);
	cb_sphere->setEnabled(true);
	pb_save->setEnabled(true);
	pb_estimate->setEnabled(true);
}
/*
void US_EquilTime::resizeEvent(QResizeEvent *e)
{	
	data_plot->setGeometry(	2 * buttonw + 3 * spacing,
									spacing, 
									e->size().width() - 4 * spacing - 2 * buttonw,
									(int) (2 * e->size().height()/3) - spacing);
	textwindow->setGeometry(2 * buttonw + 3 * spacing, 
									(int) (2 * e->size().height()/3) + spacing, 
									e->size().width() - 4 * spacing - 2 * buttonw, 
									(int) e->size().height()/3 - 2 * spacing);
}
*/
void US_EquilTime::select_inner()
{
	meniscus = 5.9;
	bottom = 6.2;
	ct_topradius->setValue(meniscus);
	ct_bottomradius->setValue(bottom);
	cb_inner->setChecked(true);
	cb_center->setChecked(false);
	cb_outer->setChecked(false);
	cb_custom->setChecked(false);
}

void US_EquilTime::select_center()
{
	meniscus = 6.4;
	bottom = 6.7;
	ct_topradius->setValue(meniscus);
	ct_bottomradius->setValue(bottom);
	cb_inner->setChecked(false);
	cb_center->setChecked(true);
	cb_outer->setChecked(false);
	cb_custom->setChecked(false);
}

void US_EquilTime::select_outer()
{
	meniscus = 6.9;
	bottom = 7.2;
	ct_topradius->setValue(meniscus);
	ct_bottomradius->setValue(bottom);
	cb_inner->setChecked(false);
	cb_center->setChecked(false);
	cb_outer->setChecked(true);
	cb_custom->setChecked(false);
}

void US_EquilTime::select_custom()
{
	cb_inner->setChecked(false);
	cb_center->setChecked(false);
	cb_outer->setChecked(false);
	cb_custom->setChecked(true);
}

void US_EquilTime::select_prolate()
{
	molecule = 1;
	cb_prolate->setChecked(true);
	cb_oblate->setChecked(false);
	cb_rod->setChecked(false);
	cb_sphere->setChecked(false);
	update_component();
}

void US_EquilTime::select_oblate()
{
	molecule = 2;
	cb_prolate->setChecked(false);
	cb_oblate->setChecked(true);
	cb_rod->setChecked(false);
	cb_sphere->setChecked(false);
	update_component();
}

void US_EquilTime::select_rod()
{
	molecule = 3;
	cb_prolate->setChecked(false);
	cb_oblate->setChecked(false);
	cb_rod->setChecked(true);
	cb_sphere->setChecked(false);
	update_component();
}

void US_EquilTime::select_sphere()
{
	molecule = 4;
	cb_prolate->setChecked(false);
	cb_oblate->setChecked(false);
	cb_rod->setChecked(false);
	cb_sphere->setChecked(true);
	update_component();
}

void US_EquilTime::select_rpm()
{
	if(use_sigma)
	{
		use_sigma = false;
		cb_sigma->setChecked(false);
		cb_rpm->setChecked(true);
		ct_speedstart->setRange(100, 60000, 100);
		ct_speedstart->setValue(rpm_start);
		ct_speedstop->setRange(100, 60000, 100);
		ct_speedstop->setValue(rpm_stop);
		lbl_speedstart->setText(tr("Low Speed (rpm):"));
		lbl_speedstop->setText(tr("High Speed (rpm):"));
		if(fabs((double)(rpm_stop - rpm_start)) < 1e-4)
		{
			speed_steps = 1;
		}
		calc_speeds();
	}
	else
	{
		use_sigma = true;
		cb_sigma->setChecked(true);
		cb_rpm->setChecked(false);
		ct_speedstart->setRange(0.1, 10, 0.01);
		ct_speedstart->setValue(sigma_start);
		ct_speedstop->setRange(0.1, 10, 0.01);
		ct_speedstop->setValue(sigma_stop);
		lbl_speedstart->setText(tr("Low Speed (sigma):"));
		lbl_speedstop->setText(tr("High Speed (sigma):"));
		if(fabs((double)(sigma_stop - sigma_start)) < 1e-4)
		{
			speed_steps = 1;
		}
		calc_speeds();
	}
}

void US_EquilTime::calc_speeds()
{
	QString str;
	lb_speeds->clear();
	if(use_sigma)
	{
		float increment;
		delete [] sigma;
		sigma = new float [speed_steps];
		if (speed_steps >= 2)
		{
			increment = (sigma_stop - sigma_start)/(speed_steps - 1);
			for (unsigned int i=0; i<speed_steps; i++)
			{
				sigma[i] = sigma_start + i * increment;
				lb_speeds->insertItem(str.sprintf("%d: sigma = %5.3f", i+1, sigma[i]));
			}
		}
		else
		{
			sigma[0] = sigma_start;
			lb_speeds->insertItem(str.sprintf("%d: sigma = %5.3f", 1, sigma[0]));
		}
	}
	else
	{
		unsigned int increment;
		delete [] rpm;
		rpm = new unsigned int [speed_steps];
		if (speed_steps >= 2)
		{
			increment = (unsigned int) ((rpm_stop - rpm_start)/(speed_steps - 1));
			for (unsigned int i=0; i<speed_steps; i++)
			{
				rpm[i] = rpm_start + i * increment;
				lb_speeds->insertItem(str.sprintf("%d: rpm = %d", i+1, rpm[i]));
			}
		}
		else
		{
			rpm[0] = rpm_start;
			lb_speeds->insertItem(str.sprintf("%d: rpm = %d", 1, rpm[0]));
		}
	}
}

void US_EquilTime::select_monitor()
{
	if(monitor_flag)
	{
		monitor_flag = false;
	}
	else
	{
		monitor_flag = true;
	}
}

void US_EquilTime::update_topradius(double val)
{
	meniscus = val;
	cb_inner->setChecked(false);
	cb_center->setChecked(false);
	cb_outer->setChecked(false);
	cb_custom->setChecked(true);	
}

void US_EquilTime::update_bottomradius(double val)
{
	bottom = val;
	cb_inner->setChecked(false);
	cb_center->setChecked(false);
	cb_outer->setChecked(false);
	cb_custom->setChecked(true);	
}

void US_EquilTime::update_speedstart(double val)
{
	if (use_sigma)
	{
		sigma_start = (float) val;
	}
	else
	{
		rpm_start = (unsigned int) val;
	}
	calc_speeds();
}

void US_EquilTime::update_speedstop(double val)
{
	if (use_sigma)
	{
		sigma_stop = (float) val;
	}
	else
	{
		rpm_stop = (unsigned int) val;
	}
	calc_speeds();
}

void US_EquilTime::update_speedsteps(double val)
{
	speed_steps = (int) val;
	calc_speeds();
}

void US_EquilTime::update_tolerance(double val)
{
	tolerance = (float) val;
}

void US_EquilTime::update_timesteps(double val)
{
	time_increment = (float) val;
}

void US_EquilTime::update_delta_t(double val)
{
	delta_t = (float) val;
}

void US_EquilTime::update_delta_r(double val)
{
	delta_r = (float) val;
}

void US_EquilTime::simulate_times()
{
	unsigned int i;
	data_plot->clear();
	sim_points = (unsigned int) (1.5 + (bottom - meniscus)/delta_r);
	sim_radius = new double [sim_points];
	sim_radius[0] = meniscus;
	for (i=1; i<sim_points; i++)
	{
		sim_radius[i] = sim_radius[i-1] + delta_r;
	}
	for (i=0; i<speed_steps; i++)
	{
		if(use_sigma)
		{
			rpm[i] = (unsigned int) (30.0/M_PI * pow((double)((sigma[i] * R * 2 * (simcomp.temperature + K0))
			/(simcomp.mw * (1 - simcomp.vbar * simcomp.density))), 0.5) + 0.5);
		}
	}
	for (i=0; i<speed_steps; i++)
	{
		rpm[i] = (unsigned int) (0.5 + (rpm[i]/100)) * 100; // round to the nearest 100
		sigma[i] = (simcomp.mw * (1 - simcomp.vbar * simcomp.density) * pow((double)((M_PI/30) * rpm[i]), (double) 2))
		/(2 * R * (simcomp.temperature + K0));
	}
	cout << "going through..." << endl;
	curve1 = data_plot->insertCurve("Simulated Data");
	data_plot->setCurveStyle(curve1, QwtCurve::Lines);
	data_plot->setCurvePen(curve1, Qt::green);
	target_curve = new long [speed_steps];
//	final_curve = new long [speed_steps];
	for (i=0; i<speed_steps; i++)
	{
		target_curve[i] = data_plot->insertCurve("Speed targets");
		data_plot->setCurveStyle(target_curve[i], QwtCurve::Lines);
		data_plot->setCurvePen(target_curve[i], Qt::red);
//		final_curve[i] = data_plot->insertCurve("Final Curves");
//		data_plot->setCurveStyle(final_curve[i], QwtCurve::Lines);
//		data_plot->setCurvePen(final_curve[i], Qt::yellow);
	}

// calculate theoretical targets and plot them in the plot:
/*
	targets = new double * [speed_steps];
	for (i=0; i<speed_steps; i++)
	{
		targets[i] = new double [sim_points];
	}
	float sum=0, amplitude, integral, temp1, temp2, xval, dr=(bottom - meniscus)/5000;
	for (i=0; i<speed_steps; i++)
	{
		integral = (bottom - meniscus) * conc;
		temp1 = dr;
		for (int m=1; m<5000; m++) // use trapezoidal rule to calculate integral to get amplitude
		{
			xval = meniscus + m * dr;
			temp2 = exp(sigma[i] * (pow((double) xval, (double) 2) - pow((double) meniscus, (double) 2))) * dr;
			sum += temp1 + (temp2 - temp1)/2.0;
			temp1 = temp2;
		}
		amplitude = integral/sum;
		for (unsigned int j=0; j<sim_points; j++)
		{
			targets[i][j] = amplitude * exp(sigma[i] * (pow(sim_radius[j], 2.0) - pow(meniscus, 2.0)));
		}
		data_plot->setCurveData(final_curve[i], sim_radius, targets[i], sim_points);
	}
	data_plot->replot();
*/
	calc_function();
}

void US_EquilTime::calc_function()
{
	unsigned int i, j, count=0;
	QString str1, str2;
//	textwindow->append(str1.sprintf("Meniscus: %f, Bottom: %f", meniscus, bottom));
	double temp1, temp2;
	float omega_s;
	float last_time = 0, sum = 0, total_time = 0;
	bool flag, ldu_flag = true;
	init_finite_element();
	temp2 = delta_t * diff; //diff coeff
	omega_s = pow((double)(M_PI * rpm[0]/30.0), 2.0);
	temp1 = delta_t * sed * omega_s; //sed coeff
	for (i=0; i<sim_points; i++)
	{
		right[i] = conc;
		temp_val[i] = conc;
		for (j=0; j<3; j++)
		{
			left[i][j] = b[i][j] + temp2 * a1[i][j] - temp1 * a2[i][j];
		}
	}
	textwindow->append("Sigma:\tRPM:\tTime Increment:\tTotal Time:");
	textwindow->append("\n");
	for (unsigned int speed_counter=0; speed_counter<speed_steps; speed_counter++)
	{
		flag = false;
		omega_s = pow((double)(M_PI * rpm[speed_counter]/30.0), 2.0);
		temp1 = delta_t * sed * omega_s; //sed coeff
		for (i=0; i<sim_points; i++)
		{
			for (j=0; j<3; j++)
			{
				left[i][j] = b[i][j] + temp2 * a1[i][j] - temp1 * a2[i][j];
			}
		}
		str1.sprintf("%6.4f\t%5d", sigma[speed_counter],  rpm[speed_counter]);
		count = 0;
		while (true)
		{
			count++;
			m3vm(&b, &right, sim_points);
			ldu(&left, &right, sim_points, 3, ldu_flag);
			ldu_flag = false;
			if (((count * delta_t) - last_time) >= (time_increment * 60))
			{
				if(monitor_flag)
				{
					data_plot->setCurveData(curve1, sim_radius, right, sim_points);
					data_plot->replot();
				}
				last_time = count * delta_t;
				sum = 0;
				for (unsigned int k=0; k<sim_points; k++)
				{
					sum += fabs((double)(temp_val[k] - right[k]));
					temp_val[k] = right[k];
				}
				if (sum <= tolerance)
				{
					total_time += last_time;
					str2.sprintf("\t%6.2f hours\t%6.2f hours", last_time/3600, total_time/3600);
					textwindow->append(str1 + str2);
					flag = true;
					ldu_flag = true;
					last_time = 0;
					data_plot->setCurveData(target_curve[speed_counter], sim_radius, right, sim_points);
					data_plot->replot();
					qApp->processEvents();
				}
			}
			if (flag)
			{
				break;
			}
		}
	}
	textwindow->append("\n");
	textwindow->append("(Note: All speeds have been adjusted to be rounded to the nearest 100 RPM.)");
	textwindow->append("__________________________________________________________________________");
	textwindow->append("\n");
	delete [] sim_radius;
	for (i=0; i<sim_points; i++)
	{
		delete [] a1[i];
		delete [] a2[i];
		delete [] b[i];
		delete [] left[i];
	}
	delete [] a1;
	delete [] a2;
	delete [] b;
	delete [] left;
	delete [] right;
	delete [] temp_val;
/*
	for (i=0; i<speed_steps; i++)
	{
		delete [] targets[i];
	}
	delete [] targets;
	delete [] final_curve;
*/
	delete [] target_curve;
}

void US_EquilTime::init_finite_element()
{
	unsigned int i;
	a1 = new float* [sim_points];
	a2 = new float* [sim_points];
	b = new float* [sim_points];
	temp_val = new float [sim_points];

// concentration has the current concentration for all scans

	left = new double* [sim_points];
	right = new double [sim_points];
	for (i=0; i<sim_points; i++)
	{
		a1[i] = new float [3];
		a2[i] = new float [3];
		b[i] = new float [3];
		left[i] = new double [3];
	}
	a1[0][1] = (sim_radius[0]/delta_r) + 0.5;
	a1[0][0] = 0.0;
	for (i=1; i<sim_points; i++)
	{
		a1[i][0] = (-(sim_radius)[i]/delta_r) + 0.5;
		a1[i][1] = 2.0 * sim_radius[i]/delta_r;
		a1[i-1][2] = a1[i][0];
	}
	a1[sim_points-1][1] = (sim_radius[sim_points-1]/delta_r) - 0.5;
	a1[sim_points-1][2] = 0.0;
	float delta2 = pow((double) delta_r, 2.0);
	a2[0][1] = -1 * (sim_radius[0] * sim_radius[0])/2.0 - sim_radius[0] * (delta_r/3.0) - delta2/12.0;
	a2[0][0] = 0.0;
	for (i=1; i<sim_points; i++)
	{
		a2[i][0] = (sim_radius[i] * sim_radius[i])/2.0 - 2.0 * sim_radius[i] * (delta_r/3.0) + delta2/4.0;
		a2[i][1] = -2 * sim_radius[i] * delta_r/3.0;
		a2[i-1][2]= -1 * (sim_radius[i-1] * sim_radius[i-1])/2.0 - 2.0 * sim_radius[i-1] * (delta_r/3.0) - delta2/4.0;
	}
	a2[sim_points-1][1] = (sim_radius[sim_points-1] * sim_radius[sim_points-1])/2.0 - sim_radius[sim_points-1] * (delta_r/3.0) + delta2/12.0;
	a2[sim_points-1][2] = 0.0;
	b[0][0] = 0.0;
	b[0][1] = sim_radius[0] * (delta_r/3.0) + delta2/12.0;
	for (i=1; i<sim_points; i++)
	{
		b[i][0] = sim_radius[i] * (delta_r/6.0) - delta2/12.0;
		b[i][1] = 2.0 * sim_radius[i]*(delta_r/3.0);
		b[i-1][2] = b[i][0];
	}
	b[sim_points-1][1] = sim_radius[sim_points-1] * (delta_r/3.0) - delta2/12.0;
	b[sim_points-1][2] = 0.0;
}

void US_EquilTime::save()
{
}

void US_EquilTime::update_component()
{
	QString str;
	lbl_mw2->setText(str.sprintf("%6.4e", simcomp.mw));
	switch (molecule)
	{
		case 1:
		{
			sed = simcomp.prolate.sedcoeff;
			diff = simcomp.prolate.diffcoeff;
			break;
		}
		case 2:
		{
			sed = simcomp.oblate.sedcoeff;
			diff = simcomp.oblate.diffcoeff;
			break;
		}
		case 3:
		{
			sed = simcomp.rod.sedcoeff;
			diff = simcomp.rod.diffcoeff;
			break;
		}
		case 4:
		{
			sed = simcomp.sphere.sedcoeff;
			diff = simcomp.sphere.diffcoeff;
			break;
		}
	}
	lbl_sed2->setText(str.sprintf("%6.4e", sed));
	lbl_diff2->setText(str.sprintf("%6.4e", diff));
}

void US_EquilTime::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/calc_equiltime.html");
}

void US_EquilTime::quit()
{
	close();
}
