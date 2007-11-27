#include "../include/us_displp.h"

DisplayPanel::DisplayPanel(unsigned int *gcomp, int *gmodel, 
									vector <struct component> components,
									struct simulation_parameters *simparams,
									QWidget *p, const char *name): QFrame(p, name, 0)
{
	USglobal = new US_Config();

	this->components = components;
	sp = simparams;
	comp = gcomp;
	model = gmodel;
	current = 1;
	
	setCaption(tr("Comprehensive Parameter Listing:"));
	setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setLineWidth(2);

	int ButtonWidth=150, xpos=5, ypos=5, ButtonHeight=26;

	simsettings = new QLabel(tr("Simulation Settings:"), this);
	simsettings->setAlignment(AlignHCenter|AlignVCenter);
	simsettings->setFrameStyle(QFrame::WinPanel|Raised);
	simsettings->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	simsettings->setPalette( QPalette(USglobal->global_colors.cg_frame,USglobal->global_colors.cg_frame,USglobal->global_colors.cg_frame));
	simsettings->setGeometry(xpos,ypos,459, ButtonHeight+4);

	ypos+=ButtonHeight+9;

	speed_lb1 = new QLabel(tr(" Speed (rpm):"), this);
	speed_lb1->setAlignment(AlignLeft|AlignVCenter);
	speed_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	speed_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	speed_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 2;
	
	speed_lb2 = new QLabel(this);
	speed_lb2->setAlignment(AlignHCenter|AlignVCenter);
	speed_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	speed_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	speed_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	speed_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	xpos += ButtonWidth/2 + 5;

	speed_lb3 = new QLabel(tr(" Omega:"), this);
	speed_lb3->setAlignment(AlignLeft|AlignVCenter);
	speed_lb3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	speed_lb3->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	speed_lb3->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);
	
	xpos += ButtonWidth + 2;

	speed_lb4 = new QLabel(this);
	speed_lb4->setAlignment(AlignHCenter|AlignVCenter);
	speed_lb4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	speed_lb4->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	speed_lb4->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	speed_lb4->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	ypos += ButtonHeight+2;
	xpos = 5;

	meniscus_lb1 = new QLabel(tr(" Meniscus (cm):"), this);
	meniscus_lb1->setAlignment(AlignLeft|AlignVCenter);
	meniscus_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	meniscus_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	meniscus_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 2;
	
	meniscus_lb2 = new QLabel(this);
	meniscus_lb2->setAlignment(AlignHCenter|AlignVCenter);
	meniscus_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	meniscus_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	meniscus_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	meniscus_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	xpos += ButtonWidth/2 + 5;

	bottom_lb1 = new QLabel(tr(" Bottom of Cell (cm):"), this);
	bottom_lb1->setAlignment(AlignLeft|AlignVCenter);
	bottom_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	bottom_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	bottom_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);
	
	xpos += ButtonWidth + 2;

	bottom_lb2 = new QLabel(this);
	bottom_lb2->setAlignment(AlignHCenter|AlignVCenter);
	bottom_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	bottom_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	bottom_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	bottom_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	ypos += ButtonHeight + 2;
	xpos = 5;

	time_lb1 = new QLabel(tr(" Total Time (hrs:min):"), this);
	time_lb1->setAlignment(AlignLeft|AlignVCenter);
	time_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	time_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	time_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 2;
	
	time_lb2 = new QLabel(this);
	time_lb2->setAlignment(AlignHCenter|AlignVCenter);
	time_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	time_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	time_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	time_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	xpos += ButtonWidth/2 + 5;

	time_lb3 = new QLabel(tr(" Delay (hrs:min):"), this);
	time_lb3->setAlignment(AlignLeft|AlignVCenter);
	time_lb3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	time_lb3->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	time_lb3->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);
	
	xpos += ButtonWidth + 2;

	time_lb4 = new QLabel(this);
	time_lb4->setAlignment(AlignHCenter|AlignVCenter);
	time_lb4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	time_lb4->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	time_lb4->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	time_lb4->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	ypos += ButtonHeight + 2;
	xpos = 5;

	scans_lb1 = new QLabel(tr(" Scans:"), this);
	scans_lb1->setAlignment(AlignLeft|AlignVCenter);
	scans_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	scans_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	scans_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 2;
	
	scans_lb2 = new QLabel(this);
	scans_lb2->setAlignment(AlignHCenter|AlignVCenter);
	scans_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	scans_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	scans_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	scans_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	xpos += ButtonWidth/2 + 5;

	scans_lb3 = new QLabel(tr(" Scan Spacing (min):"), this);
	scans_lb3->setAlignment(AlignLeft|AlignVCenter);
	scans_lb3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	scans_lb3->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	scans_lb3->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);
	
	xpos += ButtonWidth + 2;

	scans_lb4 = new QLabel(this);
	scans_lb4->setAlignment(AlignHCenter|AlignVCenter);
	scans_lb4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	scans_lb4->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	scans_lb4->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	scans_lb4->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	ypos += ButtonHeight + 2;
	xpos = 5;

	rnoise_lb1 = new QLabel(tr(" Random Noise:"), this);
	rnoise_lb1->setAlignment(AlignLeft|AlignVCenter);
	rnoise_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	rnoise_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	rnoise_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 2;
	
	rnoise_lb2 = new QLabel(this);
	rnoise_lb2->setAlignment(AlignHCenter|AlignVCenter);
	rnoise_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	rnoise_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	rnoise_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	rnoise_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	xpos += ButtonWidth/2 + 5;

	inoise_lb1 = new QLabel(tr(" Time Invariant Noise:"), this);
	inoise_lb1->setAlignment(AlignLeft|AlignVCenter);
	inoise_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	inoise_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	inoise_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);
	
	xpos += ButtonWidth + 2;

	inoise_lb2 = new QLabel(this);
	inoise_lb2->setAlignment(AlignHCenter|AlignVCenter);
	inoise_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	inoise_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	inoise_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	inoise_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	ypos += ButtonHeight + 2;
	xpos = 5;

	delta_r_lb1 = new QLabel(tr(" Delta-r (cm):"), this);
	delta_r_lb1->setAlignment(AlignLeft|AlignVCenter);
	delta_r_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	delta_r_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	delta_r_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 2;
	
	delta_r_lb2 = new QLabel(this);
	delta_r_lb2->setAlignment(AlignHCenter|AlignVCenter);
	delta_r_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	delta_r_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	delta_r_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	delta_r_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	xpos += ButtonWidth/2 + 5;

	delta_t_lb1 = new QLabel(tr(" Delta-t (sec):"), this);
	delta_t_lb1->setAlignment(AlignLeft|AlignVCenter);
	delta_t_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	delta_t_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	delta_t_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);
	
	xpos += ButtonWidth + 2;

	delta_t_lb2 = new QLabel(this);
	delta_t_lb2->setAlignment(AlignHCenter|AlignVCenter);
	delta_t_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	delta_t_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	delta_t_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	delta_t_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	ypos += ButtonHeight + 2;
	xpos = 5;

	points_lb1 = new QLabel(tr(" Points:"), this);
	points_lb1->setAlignment(AlignLeft|AlignVCenter);
	points_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	points_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	points_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 4;
	
	points_lb2 = new QLabel(this);
	points_lb2->setAlignment(AlignHCenter|AlignVCenter);
	points_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	points_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	points_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	points_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	ypos += ButtonHeight + 5;
	xpos = 5;

	modelsettings = new QLabel(tr("Model Settings:"), this);
	modelsettings->setAlignment(AlignHCenter|AlignVCenter);
	modelsettings->setFrameStyle(QFrame::WinPanel|Raised);
	modelsettings->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	modelsettings->setPalette( QPalette(USglobal->global_colors.cg_frame,USglobal->global_colors.cg_frame,USglobal->global_colors.cg_frame));
	modelsettings->setGeometry(xpos,ypos, 459, ButtonHeight+4);

	ypos += ButtonHeight + 9;

	comp_lb1 = new QLabel(tr(" Components:"), this);
	comp_lb1->setAlignment(AlignLeft|AlignVCenter);
	comp_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	comp_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	comp_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 2;
	
	comp_lb2 = new QLabel(this);
	comp_lb2->setAlignment(AlignHCenter|AlignVCenter);
	comp_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	comp_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	comp_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	comp_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	xpos += ButtonWidth/2 + 5;

	component_id = new QLabel(tr(" Component: "), this);
	component_id->setAlignment(AlignLeft|AlignVCenter);
	component_id->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	component_id->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	component_id->setGeometry(xpos, ypos, ButtonWidth+2+ButtonWidth/2, ButtonHeight);

	xpos += ButtonWidth/2 + 17;

	comp_ct = new QwtCounter(this);
	comp_ct->setRange(1, *comp, 1);
	comp_ct->setValue(current);
	comp_ct->setNumButtons(2);
	comp_ct->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	comp_ct->setGeometry(xpos+13, ypos+2, ButtonWidth-30, ButtonHeight-4); 
	comp_ct->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//	connect (comp_ct, SIGNAL(buttonReleased(double)), this, SLOT(set_current(double)));
	connect (comp_ct, SIGNAL(valueChanged(double)), this, SLOT(set_current(double)));

	ypos += ButtonHeight + 2;
	xpos = 5;

	model_lb1 = new QLabel(tr(" Model:"), this);
	model_lb1->setAlignment(AlignLeft|AlignVCenter);
	model_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	model_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	model_lb1->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	xpos += ButtonWidth/2 + 2;
	
	model_lb2 = new QLabel(this);
	model_lb2->setAlignment(AlignHCenter|AlignVCenter);
	model_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	model_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	model_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	model_lb2->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 5;

	conc_lb1 = new QLabel(tr(" Partial Concentration"), this);
	conc_lb1->setAlignment(AlignLeft|AlignVCenter);
	conc_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	conc_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	conc_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);
	
	xpos += ButtonWidth + 2;

	conc_lb2 = new QLabel(this);
	conc_lb2->setAlignment(AlignHCenter|AlignVCenter);
	conc_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	conc_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	conc_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	conc_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);
	
	ypos += ButtonHeight + 2;
	xpos = 5;

	eq_lb1 = new QLabel(tr(" K-Equilibrium:"), this);
	eq_lb1->setAlignment(AlignLeft|AlignVCenter);
	eq_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	eq_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	eq_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 2;
	
	eq_lb2 = new QLabel(this);
	eq_lb2->setAlignment(AlignHCenter|AlignVCenter);
	eq_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	eq_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	eq_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	eq_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	xpos += ButtonWidth/2 + 5;

	sed_lb1 = new QLabel(tr(" Sedimentation Coeff.:"), this);
	sed_lb1->setAlignment(AlignLeft|AlignVCenter);
	sed_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	sed_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	sed_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);
	
	xpos += ButtonWidth + 2;

	sed_lb2 = new QLabel(this);
	sed_lb2->setAlignment(AlignHCenter|AlignVCenter);
	sed_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	sed_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	sed_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	sed_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);
	
	ypos += ButtonHeight + 2;
	xpos = 5;

	totalc_lb1 = new QLabel(tr(" Total Concentration:"), this);
	totalc_lb1->setAlignment(AlignLeft|AlignVCenter);
	totalc_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	totalc_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	totalc_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 2;
	
	totalc_lb2 = new QLabel(this);
	totalc_lb2->setAlignment(AlignHCenter|AlignVCenter);
	totalc_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	totalc_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	totalc_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	totalc_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	xpos += ButtonWidth/2 + 5;


	diff_lb1 = new QLabel(tr(" Diffusion Coeff.:"), this);
	diff_lb1->setAlignment(AlignLeft|AlignVCenter);
	diff_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	diff_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	diff_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 2;
	
	diff_lb2 = new QLabel(this);
	diff_lb2->setAlignment(AlignHCenter|AlignVCenter);
	diff_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	diff_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	diff_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	diff_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	ypos += ButtonHeight + 2;
	xpos = 3 * ButtonWidth/2 + 12;

	sigma_lb1 = new QLabel(tr(" Sigma:"), this);
	sigma_lb1->setAlignment(AlignLeft|AlignVCenter);
	sigma_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	sigma_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	sigma_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 2;
	
	sigma_lb2 = new QLabel(this);
	sigma_lb2->setAlignment(AlignHCenter|AlignVCenter);
	sigma_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	sigma_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	sigma_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	sigma_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	ypos += ButtonHeight + 2;
	xpos = 3 * ButtonWidth/2 + 12;

	delta_lb1 = new QLabel(tr(" Delta:"), this);
	delta_lb1->setAlignment(AlignLeft|AlignVCenter);
	delta_lb1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	delta_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	delta_lb1->setGeometry(xpos, ypos, ButtonWidth, ButtonHeight);

	xpos += ButtonWidth + 2;
	
	delta_lb2 = new QLabel(this);
	delta_lb2->setAlignment(AlignHCenter|AlignVCenter);
	delta_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	delta_lb2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	delta_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	delta_lb2->setGeometry(xpos, ypos, ButtonWidth/2, ButtonHeight);

	xpos = 5;

	update_b = new QPushButton(tr("Update"), this);
	update_b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	update_b->setGeometry(xpos, ypos-3, ButtonWidth/2, ButtonHeight);
	update_b->setAutoDefault(false);
	update_b->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(update_b, SIGNAL(clicked()), SLOT(update_panel()));

	xpos += ButtonWidth/2 + 5;

	close_b = new QPushButton(tr("Close"), this);
	close_b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	close_b->setGeometry(xpos, ypos-3, ButtonWidth/2, ButtonHeight);
	close_b->setAutoDefault(false);
	close_b->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(close_b, SIGNAL(clicked()), SLOT(close_panel()));
	update_panel();
	
	global_Xpos += 30;
	global_Ypos += 30;

//	setMinimumSize(469, 442);
//	setMaximumSize(469, 442);
//	setGeometry(global_Xpos, global_Ypos, 469, 442);
	
	setup_GUI();
}


DisplayPanel::~DisplayPanel()
{
}

void DisplayPanel::setup_GUI()
{
	int j=0;
	int rows = 15, columns = 4, spacing = 2;
	QGridLayout * background = new QGridLayout(this, rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		background->setRowSpacing(i, 26);
	}
	background->addMultiCellWidget(simsettings,j,j,0,3);
	j++;	
	background->addWidget(speed_lb1,j,0);
	background->addWidget(speed_lb2,j,1);
	background->addWidget(speed_lb3,j,2);
	background->addWidget(speed_lb4,j,3);
	j++;	
	background->addWidget(meniscus_lb1,j,0);
	background->addWidget(meniscus_lb2,j,1);
	background->addWidget(bottom_lb1,j,2);
	background->addWidget(bottom_lb2,j,3);
	j++;	
	background->addWidget(time_lb1,j,0);
	background->addWidget(time_lb2,j,1);
	background->addWidget(time_lb3,j,2);
	background->addWidget(time_lb4,j,3);
	j++;	
	background->addWidget(scans_lb1,j,0);
	background->addWidget(scans_lb2,j,1);
	background->addWidget(scans_lb3,j,2);
	background->addWidget(scans_lb4,j,3);
	j++;	
	background->addWidget(rnoise_lb1,j,0);
	background->addWidget(rnoise_lb2,j,1);
	background->addWidget(inoise_lb1,j,2);
	background->addWidget(inoise_lb2,j,3);
	j++;	
	background->addWidget(delta_r_lb1,j,0);
	background->addWidget(delta_r_lb2,j,1);
	background->addWidget(delta_t_lb1,j,2);
	background->addWidget(delta_t_lb2,j,3);
	j++;	
	background->addWidget(points_lb1,j,0);
	background->addWidget(points_lb2,j,1);
	j++;
	background->addMultiCellWidget(modelsettings,j,j,0,3);
	j++;
	background->addWidget(comp_lb1,j,0);
	background->addWidget(comp_lb2,j,1);
	background->addWidget(component_id,j,2);
	background->addWidget(comp_ct,j,3);
	j++;
	background->addWidget(model_lb1,j,0);
	background->addWidget(model_lb2,j,1);
	background->addWidget(conc_lb1,j,2);
	background->addWidget(conc_lb2,j,3);
	j++;
	background->addWidget(eq_lb1,j,0);
	background->addWidget(eq_lb2,j,1);
	background->addWidget(sed_lb1,j,2);
	background->addWidget(sed_lb2,j,3);
	j++;
	background->addWidget(totalc_lb1,j,0);
	background->addWidget(totalc_lb2,j,1);
	background->addWidget(diff_lb1,j,2);
	background->addWidget(diff_lb2,j,3);
	j++;
	background->addWidget(sigma_lb1,j,2);
	background->addWidget(sigma_lb2,j,3);
	j++;
	background->addWidget(update_b,j,0);
	background->addWidget(close_b,j,1);
	background->addWidget(delta_lb1,j,2);
	background->addWidget(delta_lb2,j,3);

	background->setColStretch(0,0);
	background->setColStretch(1,2);
	background->setColStretch(2,0);
	background->setColStretch(3,2);

}

void DisplayPanel::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void DisplayPanel::set_current(double temp)
{
	QString str;
	current = (unsigned int) temp;
	float val1, val2;
	switch (*model)
	{
		case 0:
		{
			conc_lb2->setText(str.sprintf("%2.3e", components[current-1].conc));
			break;
		}
		case 1:
		{
			val1 = (-1 + pow((1.0 + 4.0 * components[0].conc * components[1].conc), 0.5)) 
					  / (2.0 * components[1].conc);
			val2 = components[0].conc - val1;
			if (current == 1)
			{
				conc_lb2->setText(str.sprintf("%2.3e", val1));
			}
			if (current == 2)
			{
				conc_lb2->setText(str.sprintf("%2.3e", val2));
			}
			break;
		}
		case 2:
		{
			val1 = components[0].conc / (components[1].conc + 1);
			val2 = components[0].conc - val1;
			if (current == 1)
			{
				conc_lb2->setText(str.sprintf("%2.3e", val1));
			}
			if (current == 2)
			{
				conc_lb2->setText(str.sprintf("%2.3e", val2));
			}
			break;
		}
	}
	sed_lb2->setText(str.sprintf("%2.3e", components[current-1].sed));
	diff_lb2->setText(str.sprintf("%2.3e", components[current-1].diff));
	sigma_lb2->setText(str.sprintf(" %2.3e", components[current-1].sigma));
	delta_lb2->setText(str.sprintf(" %2.3e", components[current-1].delta));
}


void DisplayPanel::update_panel()
{
	QString str, str2;
	unsigned int hours, minutes, points, i;
	float temp;
	comp_ct->setRange(1, *comp, 1);
	speed_lb2->setText(str.sprintf("%ld", (*sp).speed));
	speed_lb4->setText(str.sprintf("%2.3e", ((*sp).speed)*M_PI/30));
	meniscus_lb2->setText(str.sprintf("%2.3e", (*sp).meniscus));
	bottom_lb2->setText(str.sprintf("%2.3e", (*sp).bottom));
	hours = (unsigned int) (*sp).time/60;
	minutes = (*sp).time - hours * 60;
	time_lb2->setText(str.sprintf("%d:%d", hours, minutes));
	hours = (unsigned int) (*sp).delay/60;
	minutes = (*sp).delay - hours * 60;
	time_lb4->setText(str.sprintf("%d:%d", hours, minutes));
	scans_lb2->setText(str.sprintf("%d", (*sp).scans));
	str.sprintf("%f", (*sp).rnoise);
	str.append(" %");
	rnoise_lb2->setText(str);
	str.sprintf("%f", (*sp).inoise);
	str.append(" %");
	inoise_lb2->setText(str);
	temp = (float)(((*sp).time - (*sp).delay))/((float)(*sp).scans);
	scans_lb4->setText(str.sprintf("%2.3e", temp));
	delta_r_lb2->setText(str.sprintf("%d", (*sp).delta_r));
	delta_t_lb2->setText(str.sprintf("%2.3e", (*sp).delta_t));
	points = 1 + (unsigned int) (0.5 +((*sp).bottom - (*sp).meniscus)/(*sp).delta_r);
	points_lb2->setText(str.sprintf("%d", points));
	comp_lb2->setText(str.sprintf("%d", *comp));
	switch (*model)
	{
		case 0:
		{
			model_lb2->setText(tr("Non-Interacting"));
			temp = 0;
			for (i=0; i<*comp; i++)
			{
				temp += components[i].conc;
			}
			str2.sprintf("%2.3e", temp);
			totalc_lb2->setText(str2);
			eq_lb2->setText("N/A");
			break;
		}
		case 1:
		{
			model_lb2->setText(tr("Monomer-Dimer"));
			totalc_lb2->setText(str.sprintf("%2.3e", components[0].conc));
			eq_lb2->setText(str.sprintf("%2.3e", components[1].conc));
			break;
		}
		case 2:
		{
			model_lb2->setText(tr("Isomerization"));
			str2.sprintf("%2.3e", components[0].conc);
			totalc_lb2->setText(str2);
			eq_lb2->setText(str.sprintf("%2.3e", components[1].conc));
			break;
		}
	}
	set_current((double) current);
}

void DisplayPanel::close_panel()
{
	close();
}




