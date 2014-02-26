#include "../include/us_simwin.h"

//-----------------------------------------------------------------
// simwin.cpp:
// GUI program that simulates a variety of sedimentation velocity models
//-----------------------------------------------------------------


SpeedLegend_F::SpeedLegend_F(unsigned long int *temp_speed, QWidget *p, const char* name)
: QFrame(p, name, false)
{
	USglobal = new US_Config();
	int spacing=2, xpos=2, ypos=2, buttonw=200, buttonh=26;
	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

	speed = temp_speed;
	speed_lbl = new QLabel(tr(" Rotor Speed (rpm):"), this);
	speed_lbl->setAlignment(AlignLeft|AlignVCenter);
	speed_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	speed_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	speed_lbl->setGeometry(xpos, ypos, buttonw, buttonh);
	
	ypos += buttonh + spacing;

	speed_counter= new QwtCounter(this);
	speed_counter->setNumButtons(3);
	speed_counter->setRange(1000, 100000, 100);
	speed_counter->setValue((double) (*speed));
	speed_counter->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	speed_counter->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(speed_counter, SIGNAL(valueChanged(double)), SLOT(update_speed(double)));
	
	ypos += buttonh + spacing;

	pb_continue = new QPushButton(tr("Continue..."), this);
	Q_CHECK_PTR(pb_continue);
	pb_continue->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_continue->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_continue->setAutoDefault(false);
	pb_continue->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos = 2 * spacing + buttonw;
	ypos = 4 * spacing + 3 * buttonh;
	setMinimumSize(xpos, ypos);
	global_Xpos += 30;
	global_Ypos += 30;
	setMaximumSize(xpos, ypos);
	setGeometry(global_Xpos, global_Ypos, xpos, ypos);
}

SpeedLegend_F::~SpeedLegend_F()
{
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void SpeedLegend_F::update_speed(double val)
{
	*speed = (unsigned long) val;
}

dcdt_Window::dcdt_Window(bool *temp_widget, QWidget *p, const char* name)
: QWidget(p, name)
{
	USglobal = new US_Config();
	widget = temp_widget;
	(*widget) = true;
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("dc/dt Window"));
	plt_dcdt = new QwtPlot(this);
	plt_dcdt->setTitle(tr("dcdt Window"));
	plt_dcdt->enableGridXMin();
	plt_dcdt->enableGridYMin();
	plt_dcdt->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	plt_dcdt->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	plt_dcdt->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	//plt_dcdt->setPlotBackground(USglobal->global_colors.plot);		//old version
	plt_dcdt->setCanvasBackground(USglobal->global_colors.plot);		//new version
	plt_dcdt->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
	plt_dcdt->setAxisTitle(QwtPlot::yLeft, tr("Delta-t Concentration"));
	plt_dcdt->enableOutline(true);
	plt_dcdt->setOutlinePen(white);
	plt_dcdt->setOutlineStyle(Qwt::Cross);
	plt_dcdt->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	plt_dcdt->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	plt_dcdt->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	plt_dcdt->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	plt_dcdt->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	plt_dcdt->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	plt_dcdt->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));			
	plt_dcdt->setMargin(USglobal->config_list.margin);
	
	pb_close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_close);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setAutoDefault(false);
	connect(pb_close, SIGNAL(clicked()), this, SLOT(quit()));
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 640, 480);
}

dcdt_Window::~dcdt_Window()
{
}

void dcdt_Window::resizeEvent(QResizeEvent *e)
{
	pb_close->setGeometry(4, 4, 100, 26);
	plt_dcdt->setGeometry(2, 2, e->size().width() - 4, e->size().height() - 4);
}

void dcdt_Window::dcdtPlot(double **x, double **y, unsigned int points)
{
	unsigned int dcdtCurve;
	plt_dcdt->clear();
	dcdtCurve = plt_dcdt->insertCurve("dcdt Optical Density");
	plt_dcdt->setCurvePen(dcdtCurve, yellow);
	plt_dcdt->setCurveData(dcdtCurve, *x, *y, points);
	plt_dcdt->replot();
}

void dcdt_Window::closeEvent(QCloseEvent *e)
{
	e->accept();
	*widget = false;
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void dcdt_Window::quit()
{
	*widget = false;
	close();
}

MovieLegend_F::MovieLegend_F(QWidget *p, const char* name)
: QFrame(p, name)
{
	int xpos = 7;
	USglobal = new US_Config();
	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	time_lbl = new QLabel(tr("  Elapsed Time (sec):"), this);
	time_lbl->setAlignment(AlignLeft|AlignVCenter);
	time_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	time_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	time_lbl->setGeometry(xpos, 7, 593, 26);

	xpos += 135;
	
	time_lcd  = new QLCDNumber( 7, this, "lcd" );
	time_lcd->setPalette( QPalette(USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd));
	time_lcd->setGeometry(xpos, 7, 105, 26);
	time_lcd->setSegmentStyle ( QLCDNumber::Filled );
	time_lcd->setMode( QLCDNumber::DEC );
	
	xpos += 110;

	comp_lbl = new QLabel(tr("    Current Component:"), this);
	comp_lbl->setAlignment(AlignLeft|AlignVCenter);
	comp_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	comp_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	comp_lbl->setGeometry(xpos, 7, 135, 26);
	
	xpos += 140;

	comp_lcd  = new QLCDNumber( 3, this, "lcd" );
	comp_lcd->setPalette( QPalette(USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd));
	comp_lcd->setGeometry(xpos, 7, 60, 26);
	comp_lcd->setSegmentStyle ( QLCDNumber::Filled );
	comp_lcd->setMode( QLCDNumber::DEC );

	pb_speed = new QPushButton(tr("Change Speed"), this);
	Q_CHECK_PTR(pb_speed);
	pb_speed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_speed->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_speed->setAutoDefault(false);
	pb_speed->setGeometry(600-130, 7, 130, 26);
	
	QHBoxLayout *legend_bar= new QHBoxLayout(this,4);
	legend_bar->addWidget(time_lbl);
	legend_bar->addWidget(time_lcd);
	legend_bar->addWidget(comp_lbl);
	legend_bar->addWidget(comp_lcd);
	legend_bar->addWidget(pb_speed);
}

MovieLegend_F::~MovieLegend_F()
{
}

ScanLegend_F::ScanLegend_F(QWidget *p, const char* name)
: QFrame(p, name)
{
	int xpos = 7;
	USglobal = new US_Config();
		
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	scan_lbl = new QLabel(tr("   Scan #:"), this);
	scan_lbl->setAlignment(AlignLeft|AlignVCenter);
	scan_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	scan_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	scan_lbl->setGeometry(xpos, 7, 593, 26);

	xpos += 64;
	
	scan_lcd  = new QLCDNumber( 4, this, "lcd" );
	scan_lcd->setPalette( QPalette(USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd));
	scan_lcd->setGeometry(xpos, 7, 80, 26);
	scan_lcd->setSegmentStyle ( QLCDNumber::Filled );
	scan_lcd->setMode( QLCDNumber::DEC );

	xpos += 95;
	
	progress_lbl = new QLabel(tr("Completed:"), this);
	progress_lbl->setAlignment(AlignLeft|AlignVCenter);
	progress_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	progress_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	progress_lbl->setGeometry(xpos, 7, 75, 26);

	xpos += 80;

	progress = new QProgressBar(100, this, "Simulation Progress");
	progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	progress->setGeometry(xpos, 10, 204, 20);

	pb_dcdt = new QPushButton(tr("dC/dt Window"), this);
	Q_CHECK_PTR(pb_dcdt);
	pb_dcdt->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_dcdt->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_dcdt->setAutoDefault(false);
	pb_dcdt->setGeometry(600-130, 7, 130, 26);
	
	QHBoxLayout *legend_bar= new QHBoxLayout(this,4);
	legend_bar->addWidget(scan_lbl);
	legend_bar->addWidget(scan_lcd);
	legend_bar->addWidget(progress_lbl);
	legend_bar->addWidget(progress);
	legend_bar->addWidget(pb_dcdt);
}

ScanLegend_F::~ScanLegend_F()
{
}

SimWin::SimWin(QWidget *p , const char *name) : QWidget(p, name)
{
	USglobal = new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
/* 
	initialize all variables for a generic single component, non-interacting, ideal
   model and set up the text for the status window: 
*/

	QString str;
	simflag = false;		// determines if we need to call the simulation parameter window
	calc_flag = false;	// determines if a calculation has been performed and we can save 
								// or analyze the data
	plateau_flag = true; // true for writing plateau data to first row of data
	stop_calculation = false;
	dcdtWindow = false;
	saveTimes = false; 	// if we change speeds during the equilibrium simulation we need to save the times
	speedChanged        = false;
	comp                = 1;
	model               = 0;
	lastscans           = 0;

	components.clear();
	components.resize(1);
	components[0].conc  = 1.0;

	components[0].sed   = (float) 5.0e-13;
	components[0].diff  = (float) 7.0e-7;
	components[0].sigma = 0.0;
	components[0].delta = 0.0;
	simparams.time      = 240;
	simparams.delay     = 3;
	simparams.speed     = 45000;
	simparams.delta_t   = 10;
	simparams.delta_r   = 100;
	simparams.meniscus  = (float) 5.8;
	simparams.scans     = 30;
	simparams.bottom    = (float) 7.2;
	simparams.rnoise    = 0;
	simparams.inoise    = 0;
	simparams.rinoise   = 0;

	scantimes   = new float [1];
	inoise      = new float [1];
	rinoise     = new float [1];
	scan_index  = 1;
	point_index = 1;
	c           = new double* [1];
	c[0]        = new double  [1];
	radius      = new double  [1];
	dcdtData    = new double  [1];
	oldScan     = new double  [1];
	curve       = new unsigned int [1];
	
	frmInp = new SimControl_F(&simflag, &comp, &model, &components, &simparams, this);
	frmInp->setFrameStyle(QFrame::Panel|QFrame::Raised);
	frmInp->setLineWidth(2);

	frmInp->setmodel_T("Non-Interacting");
	frmInp->setcomponents_T(str.sprintf("%d",1));
	frmInp->setspeed_T(str.sprintf("%ld", simparams.speed));
	frmInp->setduration_T(str.sprintf("%ld",simparams.time));
	frmInp->setscans_T(str.sprintf("%d",simparams.scans));
	frmInp->setdelta_t_T(str.sprintf("%1.2e",simparams.delta_t));
	frmInp->setdelta_r_T(str.sprintf("%d",simparams.delta_r));

	m_legend = new MovieLegend_F(this);
	m_legend->setFrameStyle(QFrame::Panel|QFrame::Raised);
	m_legend->setLineWidth(2);
	connect(m_legend->pb_speed, SIGNAL(clicked()), this, SLOT(change_speed()));

	s_legend = new ScanLegend_F(this);
	s_legend->setFrameStyle(QFrame::Panel|QFrame::Raised);
	s_legend->setLineWidth(2);
	connect(s_legend->pb_dcdt, SIGNAL(clicked()), this, SLOT(dcdtOpen()));
	

	connect(frmInp->write_file, SIGNAL(clicked()), SLOT(savescans()));
	connect(frmInp->start_sim, SIGNAL(clicked()), this, SLOT(simulate()));
	connect(frmInp->display, SIGNAL(clicked()), this, SLOT(display_params()));
	connect(frmInp->vhw, SIGNAL(clicked()), this, SLOT(vhw_Analysis()));
	connect(frmInp->secm, SIGNAL(clicked()), this, SLOT(SecondMoment_Analysis()));
	connect(frmInp->dcdr, SIGNAL(clicked()), this, SLOT(dcdr_Analysis()));
	connect(frmInp->dcdt, SIGNAL(clicked()), this, SLOT(dcdt_Analysis()));
	connect(frmInp->quit, SIGNAL(clicked()), this, SLOT(quitthis()));
	connect(frmInp->help, SIGNAL(clicked()), this, SLOT(sim_help()));
	connect(frmInp->stop_now, SIGNAL(clicked()), this, SLOT(stop_now()));

	movie_plot = new QwtPlot(this);
	movie_plot->setTitle(tr("Simulation Window"));
	movie_plot->enableGridXMin();
	movie_plot->enableGridYMin();
	movie_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	movie_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	movie_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	//movie_plot->setPlotBackground(USglobal->global_colors.plot);		//old version
	movie_plot->setCanvasBackground(USglobal->global_colors.plot);		//new version
	movie_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
	movie_plot->setAxisTitle(QwtPlot::yLeft, tr("Relative Concentration"));
	movie_plot->enableOutline(true);
	movie_plot->setOutlinePen(white);
	movie_plot->setOutlineStyle(Qwt::Cross);
	movie_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	movie_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	movie_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	movie_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	movie_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	movie_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	movie_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));		
	movie_plot->setMargin(USglobal->config_list.margin);

//	connect(movie_plot, SIGNAL(plotMouseMoved(const QMouseEvent &)),SLOT(movie_mouseMoved(const QMouseEvent&)));
//	connect(movie_plot, SIGNAL(plotMousePressed(const QMouseEvent &)),SLOT(movie_mousePressed(const QMouseEvent&)));
//	connect(movie_plot, SIGNAL(plotMouseReleased(const QMouseEvent &)),SLOT(movie_mouseReleased(const QMouseEvent&)));

	scan_plot= new QwtPlot(this);
	scan_plot->setTitle(tr("Saved Scans:"));
	scan_plot->enableGridXMin();
	scan_plot->enableGridYMin();
	scan_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	scan_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	scan_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	//scan_plot->setPlotBackground(USglobal->global_colors.plot);		//old version
	scan_plot->setCanvasBackground(USglobal->global_colors.plot);		//new version
	scan_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
	scan_plot->setAxisTitle(QwtPlot::yLeft, tr("Relative Concentration"));
	scan_plot->enableOutline(true);
	scan_plot->setOutlinePen(white);
	scan_plot->setOutlineStyle(Qwt::Cross);
	scan_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	scan_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	scan_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	scan_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	scan_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	scan_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	scan_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));		
	scan_plot->setMargin(USglobal->config_list.margin);
	global_Xpos += 30;
	global_Ypos += 30;
	
	setMinimumSize(780,632);
	setGeometry(global_Xpos, global_Ypos, 780, 632);
	
	QGridLayout * background = new QGridLayout(this,2,2,spacing);
	QVBoxLayout *plot_grid = new QVBoxLayout(2);
	plot_grid->addWidget(movie_plot);
	plot_grid->addWidget(m_legend);
	plot_grid->addWidget(scan_plot);
	plot_grid->addWidget(s_legend);
	
	background->addWidget(frmInp,0,0);
	background->addLayout(plot_grid,0,1);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(1,660);
	
	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);
	
	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width()+660, r.height());



}

SimWin::~SimWin()
{
}

void SimWin::closeEvent(QCloseEvent *e)
{
	if (dcdtWindow)
	{
		dcdt_W->close();
	}
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}
/*
void SimWin::resizeEvent(QResizeEvent *e)
{
	int frmInpWidth=170;
	QRect r(frmInpWidth+4, 2, e->size().width()-(frmInpWidth+2), (int)((e->size().height()-4)/2)-41);
	movie_plot->setGeometry(r);
	m_legend->setGeometry(frmInpWidth+4, e->size().height()/2-41, r.width(), 40);
	scan_plot->setGeometry(frmInpWidth+4, e->size().height()/2, r.width(), (e->size().height()/2)-41);
	s_legend->setGeometry(frmInpWidth+4, e->size().height()-41, r.width(), 40 );
	frmInp->setGeometry(2, 2, frmInpWidth, e->size().height()-2);

}
*/
void SimWin::simulate()
{
	
	movie_plot->clear();
	scan_plot->clear();
	movie_plot->replot();
	scan_plot->replot();
	switch (model)
	{
		case 0:
		{
			non_interacting();
			calc_flag = true;
			break;
		}
		case 1:
		{
			monomer_dimer();
			calc_flag = true;
			break;
		}
		case 2:
		{
			isomerization();
			calc_flag = true;
			break;
		}
	}
}

void SimWin::change_speed()
{
	if (comp != 1 || (simparams.time - (long unsigned int) (current_time/60.0)) < 10)
	{
		return;
	}
	
	mfem->stop();
	qApp->processEvents();
	sp_f = new SpeedLegend_F(&simparams.speed);
	connect(sp_f->pb_continue, SIGNAL(clicked()), SLOT(nextSpeed()));
	sp_f->show();
}

void SimWin::nextSpeed()
{
	sp_f->close();
	speedChanged = true;
	omega_sqr = pow((double) ((M_PI/30.0) * (float) simparams.speed), (double) 2);
//	simparams.time = simparams.time - (long unsigned int) (current_time/60.0);
	speedV.push_back(simparams.speed);
	speedTimeV.push_back((float) (current_time));
	saveTimes = true;
	initial_concentration = 0.0;
	non_interacting();
}

void SimWin::dcdtOpen()
{
	dcdtWindow = true;
	dcdt_W = new dcdt_Window(&dcdtWindow);
	dcdt_W->show();
}

void SimWin::display_params()
{
	
	display_p = new DisplayPanel(&comp, &model, components, &simparams);
	display_p->show();
}

void SimWin::savescans()
{
	if (calc_flag)
	{
		QMessageBox mb(tr("UltraScan"), tr("Please choose an export format:"),
		QMessageBox::Information,
		QMessageBox::Yes | QMessageBox::Default,
		QMessageBox::No,
		QMessageBox::Cancel | QMessageBox::Escape);
		mb.setButtonText(QMessageBox::Yes, "UltraScan");
		mb.setButtonText(QMessageBox::No, "XLA");
		mb.setButtonText(QMessageBox::Cancel, tr("Cancel"));
		switch(mb.exec())
		{
			case QMessageBox::Yes: // save in UltraScan format
			{
				QString fn = QFileDialog::getSaveFileName(USglobal->config_list.result_dir, "*.us.v", 0);
				int k=0;
				if ( !fn.isEmpty() ) 
				{
					while (k != -1)
					{
						k = fn.findRev(".", -1, false);
						if (k != -1) //if an extension was given, strip it
						{
							fn.truncate(k);
						}
					}
					save_ultrascan(fn);	// the user gave a file name, save in UltraScan format
				}
				break;
			}
			case QMessageBox::No: // save in XL-A format
			{
				QString fn = QFileDialog::getExistingDirectory(USglobal->config_list.data_dir, 0, 0, 
								 tr("Please select or create a directory for the simulated data files:"), true, true);
				save_xla(fn);	// the user gave a directory name, save in Beckman/XLA format
				break;
			}
			case QMessageBox::Cancel:
			{
				return;
			}
		}
	}
	else
	{
		QMessageBox::message(tr("Attention:"), tr("Please simulate some data first\n"
													  "before attempting to save it!"));
	}
}

void SimWin::save_xla(const QString &fileName)
{
	float *temp_radius;
	unsigned int i, j, pts;
	pts = 30 + (unsigned int) ((simparams.bottom - simparams.meniscus)/simparams.resolution); // add 30 points in front of meniscus
	QString str1, str2;
	QFile f;
	US_ClipData *cd;
	float maxrad = simparams.bottom;
	float maxc = c[simparams.scans - 1][points-1];
	cd = new US_ClipData(&maxc, &maxrad, simparams.meniscus, maxconc);
	cd->exec();
	s_legend->progress->setTotalSteps(simparams.scans);
	s_legend->progress->reset();
	temp_radius = new float [pts];
	temp_radius[0] = simparams.meniscus - 30 * simparams.resolution;
	for (i=1; i<30; i++)
	{
		temp_radius[i] = temp_radius[i-1] + simparams.resolution;
	}
	for (i=30; i<pts; i++)
	{
		temp_radius[i] = simparams.meniscus + (simparams.resolution * (i-30));
	}
	s_legend->progress_lbl->setText("Writing...");
	for (i=0; i<simparams.scans; i++)
	{
		if (i < 9)
		{
			str2 = fileName + "/" + "0000" + str1.sprintf("%d", i+1) + ".ra1";
		}
		else if (i >= 9 && i < 99)
		{
			str2 = fileName + "/" + "000" + str1.sprintf("%d", i+1) + ".ra1";
		}
		else if (i >= 99 && i < 999)
		{
			str2 = fileName + "/" + "00" + str1.sprintf("%d", i+1) + ".ra1";
		}
		else if (i >= 999 && i < 9999)
		{
			str2 = fileName + "/" + "0" + str1.sprintf("%d", i+1) + ".ra1";
		}
		else if (i >= 9999 && i < 10000)
		{
			str2 = fileName + "/" + str1.sprintf("%d", i+1) + ".ra1";
		}
		else
		{
			return;
		}
		f.setName(str2);
		if (f.open(IO_WriteOnly | IO_Translate))
		{
			QTextStream ts(&f);
			ts << "Simulated Velocity Data" << endl;
			ts << "R 1 20.0 " 
			<< str1.sprintf("%5ld %7ld %1.5e %d %d\n", simparams.speed, 
			(long int) scantimes[i], scantimes[i] * omega_sqr, 999, 1);
			for (j=0; j<30; j++) // the region in front of the meniscus
			{
				ts << str1.sprintf("%9.4f %12.5e %12.5e", temp_radius[j], 0.0, 0.0) << endl;
			}
			// one high point for the meniscus:
			ts << str1.sprintf("%9.4f %12.5e %12.5e", temp_radius[30], maxconc*1.3, 0.0) << endl;
			j=31;
			if (maxc == 0) // if 0 then use the entire range
			{
				while (j<pts && temp_radius[j] <= maxrad)
				{
					ts << str1.sprintf("%9.4f %12.5e %12.5e", temp_radius[j], c[i][j-30], 0.0) << endl;
					j++;
				}
			}
			else
			{
				while (j<pts && c[i][j-30] <= maxc && temp_radius[j] <= maxrad)
				{
					ts << str1.sprintf("%9.4f %12.5e %12.5e", temp_radius[j], c[i][j-30], 0.0) << endl;
					j++;
				}
			}
		}
		s_legend->progress->setProgress(i+1);
		qApp->processEvents();
		f.close();
	}
	s_legend->progress->setProgress(simparams.scans);
	s_legend->progress_lbl->setText("Completed:");
}

void SimWin::save_ultrascan(const QString &fileName)
{
	float *plateau, new_bottom, check;
	unsigned int i, j, new_points;
	US_ClipData *cd;
	float maxrad = simparams.bottom;
	float maxc = c[simparams.scans - 1][points-1];
	cd = new US_ClipData(&maxc, &maxrad, simparams.meniscus, maxconc);
	cd->exec();
	check = 0.0;
	new_points = points;
	i=0;
	while (i<points && c[simparams.scans-1][i] < maxc)
	{				// find the radius from the last scan where the
		i++;		// concentration is higher than the threshold (if at all)
	}
	new_points = i-1;
	while (maxrad < radius[new_points])	// then check to see if this radius is larger than the maxrad
	{												// if it is, then decrease the point count until the max radius
		new_points --;							// matches.
	}
	new_points++;
	new_bottom = radius[new_points];
	plateau = new float[simparams.scans];
	QString temp_str = QString(fileName);
	QString run_file = temp_str.copy();
	QString data_file = temp_str.copy();
	QString scan_file = temp_str.copy();
	QString temp = temp_str.copy();
	int position = temp.findRev("/", -1, false);
	i = temp.length();
	j = i - position - 1;		// -1 because we dont want to count the null terminating character
	QString run_name = temp.right(j);	// run name without leading path
	run_file.append(".us.v");
	data_file.append(".veloc.11");
	scan_file.append(".scn");
	QFile f1(run_file);
	if (f1.exists())
	{
		if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
		{
			f1.close();
			return;
		}
	}
	if (f1.open(IO_WriteOnly))	// write the binary run file *.us.v
	{
		QDataStream ds(&f1);
		ds << US_Version;
		ds << USglobal->config_list.data_dir + "/simulation";	// data directory
		int k = temp_str.findRev("/", -1, false);
		if (k != -1) //strip the path
		{
			temp_str = temp_str.mid(k+1, temp_str.length());
		}
		ds << temp_str;
		ds << (float) 20.0;							// average temperature
		ds << (int) 1; 								// run_inf.temperature_check;
		ds << (float) 0.0;							// run_inf.time_correction;	
		ds << (float) (simparams.time * 60);	// run_inf.duration;
		ds << (unsigned int) simparams.scans;	// for simulated data, total scans = scans for simulation, since only one "cell" is simuulated
		ds << (float) simparams.resolution;		// run_inf.delta_r
		ds << (int) -1; 								// experimental data ID
		ds << (int) -1;								// Investigator ID
		QDate today = QDate::currentDate();
		QString current_date;
		current_date.sprintf( "%d/%d/%d", today.month(), today.day(), today.year() );
		ds << current_date;
		ds << (QString) "Simulated Velocity Data";
		ds << (QString) "ultrascan";
		ds << (QString) "192.168.0.1";
		ds << (QString) "QMYSQL3";
		ds << (int) 1; // run_inf.exp_type.velocity;
		ds << (int) 0; // run_inf.exp_type.equilibrium;
		ds << (int) 0; // run_inf.exp_type.diffusion;
		ds << (int) 1; // run_inf.exp_type.simulation;
		ds << (int) 0; // run_inf.exp_type.interference;
		ds << (int) 1; // run_inf.exp_type.absorbance;
		ds << (int) 0; // run_inf.exp_type.fluorescence;
		ds << (int) 0; // run_inf.exp_type.intensity;
		ds << (int) 0; // run_inf.exp_type.wavelength;
		for (i=0; i<8; i++)
		{
			if (i == 0) 
			{
				ds << (int) 0;	//centerpiece = simulation cell is zero
				ds << (float) simparams.meniscus;
				QString temp_string = QString(tr("Simulated Data - see corresponding model"));	// data discription
				ds << temp_string;
				ds << (unsigned int) 1;	//how many wavelengths?
			}
			else
			{
				ds << (unsigned int) 0;	//centerpiece = conventional 2 channel epon
				ds << (float) 0.0;
				QString temp_string = QString("");	// data description
				ds << temp_string;
				ds << (unsigned int) 0;
			}
		}
		int serial_number = -1;
		for (i=0; i<8; i++)
		{
			for (j=0; j<4; j++)
			{
				ds << serial_number;	//buffer serial number
				for(int k=0; k<3; k++)
				{
					ds << serial_number;	//peptide serial number
					ds << serial_number;	//DNA serial number
				}
			}
			for (j=0; j<3; j++)
			{
				if (i == 0 && j == 0)
				{
					ds << (unsigned int) 999;	// run_inf.wavelength[i][j]
					ds << (unsigned int) simparams.scans;
					ds << (float) 0.0;	// baseline
					ds << (float) simparams.meniscus;
					ds << (float) new_bottom;
					ds << (unsigned int) new_points;
					ds << (float) (new_bottom - simparams.meniscus)/new_points;
				}
				else 
				{
					ds << (unsigned int) 0;
					ds << (unsigned int) 0;
					ds << (float) 0.0;	// baseline
					ds << (float) 0.0;
					ds << (float) 0.0;
					ds << (unsigned int) 0;
					ds << (float) 0.0;
				}
			}
		}
		for (i=0; i<simparams.scans; i++)
		{
			plateau[i] = 0.0;
			for (j=0; j<components.size(); j++)
			{
//
// This is the equation for radial dilution:
//
				plateau[i] += components[j].conc * exp(-2.0 * 
				components[j].sed * omega_sqr * scantimes[i]);
			}
		}
		for (i=0; i<8; i++)
		{
			for (j=0; j<simparams.scans; j++)
			{
				if (i == 0)
				{
					ds << (uint) simparams.speed;
					ds << (float) 20.0; 		// temperature
					ds << (unsigned int) scantimes[j];
					ds << (float) scantimes[j] * omega_sqr;
					ds << (float) plateau[j];
				}
				else
				{
					ds << (uint) 0;
					ds << (float) 0.0;
					ds << (unsigned int) 0.0;
					ds << (float) 0.0;
					ds << (float) 0.0;
				}
			}
		}
		ds << (int) -1; 	// run_inf.rotor;
		f1.flush();
		f1.close();
	}
	QFile f2(data_file);
	if (f2.open(IO_WriteOnly))	// write the binary scan data file *.veloc.11
	{
		QDataStream ds2(&f2);
		for (j=0; j<simparams.scans; j++)
		{
			for (i=0; i<new_points; i++)
			{
				ds2 << (float) c[j][i];
			}
		}
		f2.flush();
		f2.close();
	}
	QFile f3(scan_file);
	if (f3.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f3);
//
// Calculate expected plateaus for each scan by summing the plateau contributions 
// of each component:
//
		for (i=0; i<simparams.scans; i++)
		{
			ts << plateau[i] << "\t";
		}
		ts << "\n";
		for (i=0; i<new_points; i++)
		{
			ts << radius[i] << "\t";
			for (j=0; j<(simparams.scans-1); j++)
			{
				ts << c[j][i] << "\t";
			}
			ts << c[simparams.scans-1][i] << "\n";
		}
		f3.close();
	}
	component_reset();
}


void SimWin::vhw_Analysis()
{
	component_init();
	int run_flag=0;
	if (calc_flag)
	{
		vhw = new vHW_Sim(model, &comp, &points, &run_flag, &components, &c, &simparams, 
		&scantimes, temp_conc, temp_k);
		vhw->setCaption(tr("van Holde - Weischet Analysis:"));
		vhw->setMinimumSize(560,480);
		vhw->setGeometry(0, 0, 560, 480);
		if (run_flag == 0)
		{
			vhw->show();
		}
		else
		{
			component_reset();
			return;
		}
	}
	else
	{
		QMessageBox::message(tr("Attention:"), tr("Please simulate some data first\n"
													  "before attempting to analyze it!"));
	}
}

void SimWin::SecondMoment_Analysis()
{
	component_init();
	if (calc_flag)
	{
		secm = new SecondMoment(model, &comp, &points, &c, &components, &simparams, &scantimes, temp_conc, temp_k);
		secm->setCaption(tr("Second Moment Analysis:"));
		secm->setMinimumSize(560,480);
		secm->setGeometry(0, 0, 560, 480);
		secm->show();
	}
	else
	{
		QMessageBox::message(tr("Attention:"), tr("Please simulate some data first\n"
													  "before attempting to analyze it!"));
	}
}

void SimWin::dcdt_Analysis()
{
	component_init();
	if (calc_flag)
	{
		dcdt = new dCdT(model, &comp, &components, &c, &simparams, &scantimes, temp_conc, temp_k);
		dcdt->setCaption(tr("dC/dT Analysis:"));
		dcdt->setMinimumSize(560,480);
		dcdt->setGeometry(0, 0, 560, 480);
		dcdt->show();
	}
	else
	{
		QMessageBox::message(tr("Attention:"), tr("Please simulate some data first\n"
													  "before attempting to analyze it!"));
	}
}

void SimWin::dcdr_Analysis()
{
	component_init();
	if (calc_flag)
	{
		dcdr = new dCdr(&c, &simparams, &scantimes);
		dcdr->setCaption(tr("dC/dr Analysis:"));
		dcdr->setMinimumSize(560,480);
		dcdr->setGeometry(0, 0, 560, 480);
		dcdr->show();
	}
	else
	{
		QMessageBox::message(tr("Attention:"), tr("Please simulate some data first\n"
													  "before attempting to analyze it!"));
	}
}

void SimWin::update_GUI(struct mfem_scan newscan, std::vector <double> rad)
{
	unsigned int i;
	double *x, *y;
	x = new double [rad.size()];
	y = new double [rad.size()];
	initCvector.radius.clear();
	initCvector.concentration.clear();
	current_time = newscan.time;
	for (i=0; i<rad.size(); i++)
	{
		x[i] = rad[i];
		initCvector.radius.push_back(x[i]);
		y[i] = newscan.conc[i];
		initCvector.concentration.push_back(y[i]);
	}
	movie_plot->clear();
	movie_plot->setAxisScale(QwtPlot::xBottom, 5.8, (double) simparams.bottom, 0);
	movie_plot->setAxisScale(QwtPlot::yLeft, 0, (double) components[current_component].conc*2.0, 0);
	long int movie_scan = movie_plot->insertCurve("Optical Density");
	movie_plot->setCurvePen(movie_scan, yellow);
	m_legend->comp_lcd->display((int) (current_component+1));
	m_legend->time_lcd->display(newscan.time);
	s_legend->progress->setProgress((int)((current_component * simparams.time * 60) + newscan.time));
	movie_plot->setCurveData(movie_scan, x, y, rad.size());
	movie_plot->replot();
	float box;
	if (dcdtWindow)
	{
		// check to see if we need to allocate memory for oldScan:
		if (dcdt_flag == false)
		{
			delete [] oldScan;
			delete [] dcdtData;
			oldScan = new double [rad.size()];
			dcdtData = new double [rad.size()];
			for (i=0; i<rad.size(); i++)
			{
				if (simparams.rnoise != 0.0)
				{
					oldScan[i] = y[i] + box_muller(0, maxconc * simparams.rnoise/100);
				}
				else
				{
					oldScan[i] = y[i];
				}
			}
			dcdt_flag = true;
		}
		else
		{
			for (i=0; i<rad.size(); i++)
			{
				if (simparams.rnoise != 0.0)
				{
					box = box_muller(0, maxconc * simparams.rnoise/100);
					dcdtData[i] = y[i] - oldScan[i] + box;
					oldScan[i] = y[i] + box;
				}
				else
				{
					dcdtData[i] = y[i] - oldScan[i];
					oldScan[i] = y[i];
				}
			}
			dcdt_W->dcdtPlot(&x, &dcdtData, rad.size());
		}
	}
	delete [] x;
	delete [] y;
	// update the GUI:
	qApp->processEvents();
}

void SimWin::non_interacting()
{
	unsigned int i;
	init();
	mfem = new US_MovingFEM(&fem_data, true, 0, 0);
	connect(mfem, SIGNAL(scan_updated(struct mfem_scan, std::vector <double>)), 
	        this, SLOT (update_GUI(struct mfem_scan, std::vector <double>)) );
	s_legend->progress->setTotalSteps(comp*simparams.time*60);
	s_legend->progress->reset();
	double total_t = (double) simparams.time * 60;
	int result;
	if (comp > 1)
	{
		m_legend->pb_speed->setEnabled(false);
	}
	for (i=0; i<comp; i++)
	{
		dcdt_flag = false; 
		current_component = i;
		if (!speedChanged)
		{ // if initial_concentration != 0 then we initialize from initial_concentration
			initial_concentration = components[i].conc; // otherwise we use initCvector (when speed was changed)
		}
		mfem->set_params(simparams.delta_r, components[i].sed, components[i].diff, 
		(double) simparams.speed, total_t, simparams.meniscus, simparams.bottom,
		initial_concentration, &initCvector);
		
		result = mfem->run();
		if(result < 0)
		{
			QMessageBox::message(tr("Attention:"), 
			tr("The finite element simulation failed.\n"
			"Please check the initialization\n"
			"parameters, and try again after\n"
			"adjusting the parameters to different\n"
			"values... Fit was aborted."));
			qApp->processEvents();
			return;
		}
		else if(result == 1) // simulation was stopped
		{
			qApp->processEvents();
			s_legend->progress->setProgress(comp*simparams.time*60);
			simparams.time = simparams.time - (unsigned long int) (fem_data.scan[fem_data.scan.size()-1].time/60.0);
			float scan_diff = 60.0 * (float) (simparams.time - simparams.delay)/simparams.scans;
			float temp = simparams.delay * 60;
			for (i=0; i<simparams.scans; i++)
			{
				scantimes[i] = temp; // put scan times into seconds...
				temp +=  scan_diff;
			}
			mfem->interpolate(&fem_data, simparams.scans, points, scantimes, radius, c);
			plot_scans();
			qApp->processEvents();
			delete mfem;
			stop_calculation = false;
			speedChanged = false;
			return;
		}
		else
		{
			mfem->interpolate(&fem_data, simparams.scans, points, scantimes, radius, c);
		}
	} // end component loop
	s_legend->progress->setProgress(comp*simparams.time*60);
	plot_scans();
	delete mfem;
	m_legend->pb_speed->setEnabled(true);
	speedChanged = false;
	if (saveTimes) // more than one speed was used, show the times and speeds.
	{
		show_speeds();
	}
}

void SimWin::plot_scans()
{
	unsigned int i, j;
	// update the concentration with random and time invariant noise
	if (simparams.inoise > 0.0 || simparams.rnoise > 0.0)
	{ 
		for (i=0; i<simparams.scans; i++)
		{
			for (j=0; j<(unsigned int) points; j++)
			{
				c[i][j] += inoise[j] + box_muller(0, maxconc * simparams.rnoise/100 );
			}
		}
	}
	scan_plot->setAxisScale(QwtPlot::xBottom, 5.8, (double) simparams.bottom, 0);
	scan_plot->setAxisScale(QwtPlot::yLeft, 0, maxconc*2, 0);
	QString str;
	for (i=0; i<simparams.scans; i++)
	{
		if (simparams.rinoise > 0.0)
		{
			for (j=0; j<(unsigned int) points; j++)
			{
				c[i][j] += rinoise[i];
			}
		}
		curve[i] = scan_plot->insertCurve("Combined Concentrations " + str.sprintf("%d", i));
		scan_plot->setCurvePen(curve[i], yellow);
		scan_plot->setCurveData(curve[i], radius, c[i], points);
		scan_plot->replot();
	}
}

void SimWin::show_speeds()
{
	int hrs, mins, secs;
	unsigned int i;
	QString str, fileName;
	QFile f;
	fileName = USglobal->config_list.result_dir + "/equiltimes.dat";
	f.setName(fileName);
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		ts << tr("Times for changing rotor speeds:\n\n");
		for (i=0; i<speedTimeV.size(); i++)
		{
			hrs = (int) (speedTimeV[i]/3600);
			speedTimeV[i] -= hrs * 3600;
			mins = (int) (speedTimeV[i]/60);
			speedTimeV[i] -= mins * 60;
			secs = (int) speedTimeV[i];
			str.sprintf(tr("Change to speed %d (%d rpm) after %d hrs, %d min, %d sec\n"), i+2, speedV[i], hrs, mins, secs);
			ts << str;
		}
		f.close();
	}
	TextEdit *e;
	e = new TextEdit();
	e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
	e->load(fileName);
	e->show();
	saveTimes = false;
}

void SimWin::init()
{
	simparams.resolution = (float) 0.001;
	QFile f;
	unsigned int i, j;
	points = (unsigned int) (1.5+(simparams.bottom - simparams.meniscus)/simparams.resolution);
	delete [] scantimes;
	delete [] radius;
	delete [] inoise;
	delete [] rinoise;
	delete [] curve;
	for (i=0; i<fem_data.scan.size(); i++)
	{
		fem_data.scan[i].conc.clear();
	}
	fem_data.radius.clear();
	fem_data.scan.clear();
	for (i=0; i<scan_index; i++)
	{
		delete [] c[i];
	}
	delete [] c;
	scantimes = new float [simparams.scans];
	curve = new unsigned int [simparams.scans];
	scan_index = simparams.scans;
	float scan_diff = 60.0 * (float) (simparams.time - simparams.delay)/simparams.scans;
	float temp = simparams.delay * 60;
	c = new double * [simparams.scans];
	for (i=0; i<simparams.scans; i++)
	{
		scantimes[i] = temp; // put scan times into seconds...
		temp +=  scan_diff;
		c[i] = new double [points];
	}
	for (i=0; i<simparams.scans; i++)
	{
		for (j=0; j<points; j++)
		{
			c[i][j] = 0.0;
		}
	}
	radius = new double [points];
	inoise = new float [points];
	rinoise = new float [simparams.scans];
	point_index = points;
	maxconc = 0;
	for (i=0; i<comp; i++)
	{
		maxconc += components[i].conc;
	}
	temp_conc = maxconc;
	for (i=0; i<points; i++)
	{
		inoise[i] = 0.0;
		radius[i] = simparams.meniscus + i*(simparams.resolution);
	}
	if (simparams.inoise > 0.0)
	{ 
		inoise[0] = box_muller(0, maxconc * simparams.inoise/100 );
		for (i=1; i<points; i++)
		{
			inoise[i] += inoise[i-1] + box_muller(0, maxconc * simparams.inoise/100 );
		}
		f.setName(USglobal->config_list.result_dir + "/simulation.ti_noise.11");
		if (f.open(IO_WriteOnly))
		{
			QTextStream ts(&f);
			for (i=0; i<points; i++)
			{
				ts << inoise[i] << endl;
			}
			f.close();
		}
	}
	if (simparams.rinoise > 0.0)
	{ 
		for (i=0; i<simparams.scans; i++)
		{
			rinoise[i] = box_muller(0, maxconc * simparams.rinoise/100 );
		}
		f.setName(USglobal->config_list.result_dir + "/simulation.ri_noise.11");
		if (f.open(IO_WriteOnly))
		{
			QTextStream ts(&f);
			for (i=0; i<simparams.scans; i++)
			{
				ts << rinoise[i] << endl;
			}
			f.close();
		}
	}
	omega_sqr = pow(simparams.speed * M_PI/30.0, 2.0);
}

/*
// Method for finite element using Crank Nicholson scheme for time discretization

void SimWin::non_interacting()
{
	unsigned int i, j, k, iterations, scan=0, count=0;
	int hrs, mins, secs;
	speedTimeV.clear();
	speedChanged = false;
	speedV.clear();
	double maxconc=0.0, *c_current;
	bool conc_dep= false, ldu_flag = true;
	if (comp > 1 && model == 0)
	{
		m_legend->pb_speed->setEnabled(false);
	}
	iterations = (unsigned int) (0.5+(simparams.time*60/simparams.delta_t));
	s_legend->progress->setTotalSteps(iterations*comp);
	s_legend->progress->reset();
	init();
	for (k=0; k<comp; k++)
	{
		maxconc += (double) components[k].conc;
		if (components[k].sigma != 0 || components[k].delta != 0)
		{
			conc_dep = TRUE;
		}
	}
	if (simparams.inoise != 0.0)
	{
		QString temp_fn = USglobal->config_list.root_dir;
		temp_fn.append("/ti-error.dat");
		QFile fn(temp_fn);
		if (fn.open(IO_WriteOnly | IO_Translate))
		{
			QTextStream ts(&fn);
			for (i=0; i<points; i++)
			{
				inoise[i] = box_muller(0, maxconc * simparams.inoise/100 );
				ts << radius[i] << ", "  << inoise[i] << "\n";
			}
			fn.close();
		}
	}
	if (conc_dep)
	{
		c_current = new double [points];
	}
	maxconc = 2.0 * maxconc;
	scan_plot->setAxisScale(QwtPlot::yLeft, 0, maxconc, 0);
	movie_plot->setAxisScale(QwtPlot::xBottom, 5.8, (double) simparams.bottom, 0);
	scan_plot->setAxisScale(QwtPlot::xBottom, 5.8, (double) simparams.bottom, 0);
	left_temp = new double* [points];
	for (i=0; i< points; i++)
	{
		left_temp[i] = new double [3];
	}
	right_old = new double [points];
	for (k=0; k<comp; k++)
	{
		ldu_flag = true;
		m_legend->comp_lcd->display((int) (k+1));
		movie_plot->setAxisScale(QwtPlot::yLeft, 0, (double) components[k].conc*2.0, 0);
		scan_timer=0.0;
		temp1=simparams.delta_t * components[k].sed * omega_sqr;
		temp2=simparams.delta_t * components[k].diff;
		for (i=0; i<points; i++)
		{
			right[i]=components[k].conc;
			for (j=0; j<3; j++)
			{
				left[i][j] = temp2 * a1[i][j] - temp1 * a2[i][j];
			}
		}
		for (i=0; i< points; i++)
		{
			for (j=0; j<3; j++)
			{
				left_temp[i][j] = b_orig[i][j] + 0.5 * left[i][j];
			}
		}

		for (j=0; j<iterations; j++)
		{
			s_legend->progress->setProgress(count);
			count++;
			scan_timer += simparams.delta_t;
			m_legend->time_lcd->display( (double) scan_timer);
			if (components[k].sigma != 0 || components[k].delta != 0)
			{
				for (i=0; i<points; i++)
				{
					c_current[i]=right[i];
				}
			}
			for (i=0; i<points; i++)
			{
				right_old[i] = right[i];
			}
			movie_plot->setCurveData(movie_scan, radius, right, points);
			movie_plot->replot();

			if (speedChanged)
			{
				speedTimeV.push_back(scan_timer);
				speedV.push_back((unsigned int) simparams.speed);
				temp1=simparams.delta_t * components[0].sed * omega_sqr;
			}
			if (components[k].sigma != 0)
			{
				cout << "calling conc_dep_s\n";
				conc_dep_s(components[k].sigma, &right, &c_current, &a2, temp1, points);
			}
			if (components[k].delta != 0)
			{
				conc_dep_d(components[k].delta, &right, &c_current, &u, &v, &w, temp2, points,
				&cu, &cv, &cw);
			}
			
			// Crank-Nicholson:

			m3vm_b(&b_orig, &left, &right, points, -0.5); // adds B to left and divides each element of A by 2 before multiplying with right[i]
			if (speedChanged)
			{
				ldu_flag = true;
				for (unsigned int l=0; l<points; l++)
				{
					for (unsigned int m=0; m<3; m++)
					{
						left[l][m] = temp2 * a1[l][m] - temp1 * a2[l][m];
					}
				}
				for (i=0; i< points; i++)
				{
					for (unsigned int m=0; m<3; m++)
					{
						left_temp[i][m] = b_orig[i][m] + 0.5 * left[i][m];
					}
				}
				ldu(&left_temp, &right, points, 3, ldu_flag); // end Corrector
				ldu_flag = false;
				speedChanged = false;
				saveTimes = true;
			}
			else
			{
				ldu(&left_temp, &right, points, 3, ldu_flag); // end Corrector
				ldu_flag = false;
			}
			if (fabs(scantimes[scan] - scan_timer) <= (simparams.delta_t/2.0))
			{
				if ((simparams.rnoise != 0) || (simparams.inoise != 0))
				{
					if ((simparams.rnoise != 0) && (simparams.inoise != 0))
					{
						for (i=0; i<points; i++)
						{
							c[i][scan] += right[i] + box_muller(0, maxconc * simparams.rnoise/100 ) + inoise[i];
							concentration[i] = c[i][scan];
						}
					}
					else if (simparams.rnoise != 0)
					{
						for (i=0; i<points; i++)
						{
							c[i][scan] += right[i] + box_muller(0, maxconc * simparams.rnoise/100 );
							concentration[i] = c[i][scan];
						}

					}
					else if (simparams.inoise != 0)
					{
						for (i=0; i<points; i++)
						{
							c[i][scan] += right[i] + inoise[i];
							concentration[i] = c[i][scan];
						}
					}
				}
				else
				{
					for (i=0; i<points; i++)
					{
						c[i][scan] += right[i];
						concentration[i] = c[i][scan];
					}
				}
				scan++;
				if (k == comp-1)
				{
					s_legend->scan_lcd->display((int) scan);
					curve[scan-1] = scan_plot->insertCurve("Optical Density");
					scan_plot->setCurvePen(curve[scan-1], yellow);
					scan_plot->setCurveData(curve[scan-1], radius, concentration, points);
					scan_plot->replot();
					if (scan > 1)
					{
						for (i=0; i<points; i++)
						{
							dcdtData[i] = c[i][scan-2] - c[i][scan-1];
						}
						if (dcdtWindow)
						{
							dcdt_W->dcdtPlot(&radius, &dcdtData, points);
						}
					}
				}
				if (scan == simparams.scans)
				{
					scan=0;
				}
			}
			qApp->processEvents();
			if (stop_calculation)
			{
				stop_calculation = false;
				return;
			}
		}
	}
	if (saveTimes) // more than one speed was used, show the times and speeds.
	{
		QString str, fileName;
		QFile f;
		fileName = USglobal->config_list.result_dir + "/equiltimes.dat";
		f.setName(fileName);
		if (f.open(IO_WriteOnly | IO_Translate))
		{
			QTextStream ts(&f);
			
			ts << tr("Times for changing rotor speeds:\n\n");
			for (i=0; i<speedTimeV.size(); i++)
			{
				hrs = (int) (speedTimeV[i]/3600);
				speedTimeV[i] -= hrs * 3600;
				mins = (int) (speedTimeV[i]/60);
				speedTimeV[i] -= mins * 60;
				secs = (int) speedTimeV[i];
				str.sprintf(tr("Change to speed %d (%d rpm) after %d hrs, %d min, %d sec\n"), i+2, speedV[i], hrs, mins, secs);
				ts << str;
			}
			f.close();
		}
		//view_file(fileName);
		TextEdit *e;
		e = new TextEdit();
		e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
		e->load(fileName);
		e->show();
		saveTimes = false;
	}
	lastscans=simparams.scans;
	s_legend->progress->setProgress(comp*iterations);
	m_legend->pb_speed->setEnabled(true);
	for (i=0; i<points; i++)
	{
		delete [] left_temp[i];
	}
	delete [] left_temp;
	delete [] right_old;
}
*/
/*

// Method for finite element using finite difference
void SimWin::non_interacting()
{
	unsigned int i, j, k, iterations, scan=0, count=0;
	int hrs, mins, secs;
	speedTimeV.clear();
	speedChanged = false;
	speedV.clear();
	double maxconc=0.0, *c_current;
	bool conc_dep= false, ldu_flag = true;
	if (comp > 1 && model == 0)
	{
		m_legend->pb_speed->setEnabled(false);
	}
	iterations = (unsigned int) (0.5+(simparams.time*60/simparams.delta_t));
	s_legend->progress->setTotalSteps(iterations*comp);
	s_legend->progress->reset();
	init();
	for (k=0; k<comp; k++)
	{
		maxconc += (double) components[k].conc;
		if (components[k].sigma != 0 || components[k].delta != 0)
		{
			conc_dep = TRUE;
		}
	}
	if (simparams.inoise != 0.0)
	{
		QString temp_fn = USglobal->config_list.root_dir;
		temp_fn.append("/ti-error.dat");
		QFile fn(temp_fn);
		if (fn.open(IO_WriteOnly | IO_Translate))
		{
			QTextStream ts(&fn);
			for (i=0; i<points; i++)
			{
				inoise[i] = box_muller(0, maxconc * simparams.inoise/100 );
				ts << radius[i] << ", "  << inoise[i] << "\n";
			}
			fn.close();
		}
	}
	if (conc_dep)
	{
		c_current = new double [points];
	}
	maxconc = 2.0 * maxconc;
	scan_plot->setAxisScale(QwtPlot::yLeft, 0, maxconc, 0);
	movie_plot->setAxisScale(QwtPlot::xBottom, 5.8, (double) simparams.bottom, 0);
	scan_plot->setAxisScale(QwtPlot::xBottom, 5.8, (double) simparams.bottom, 0);
	for (k=0; k<comp; k++)
	{
		ldu_flag = true;
		m_legend->comp_lcd->display((int) (k+1));
		movie_plot->setAxisScale(QwtPlot::yLeft, 0, (double) components[k].conc*2.0, 0);
		scan_timer=0.0;
		temp1=simparams.delta_t * components[k].sed * omega_sqr;
		temp2=simparams.delta_t * components[k].diff;
		for (i=0; i<points; i++)
		{
			right[i]=components[k].conc;
			for (j=0; j<3; j++)
			{
				left[i][j] = b[i][j] + temp2 * a1[i][j] - temp1 * a2[i][j];
			}
		}
		for (j=0; j<iterations; j++)
		{
			s_legend->progress->setProgress(count);
			count++;
			scan_timer += simparams.delta_t;
			m_legend->time_lcd->display( (double) scan_timer);
			if (components[k].sigma != 0 || components[k].delta != 0)
			{
				for (i=0; i<points; i++)
				{
					c_current[i]=right[i];
				}
			}
			movie_plot->setCurveData(movie_scan, radius, right, points);
			movie_plot->replot();
			m3vm(&b, &right, points);	

			if (speedChanged)
			{
				speedTimeV.push_back(scan_timer);
				speedV.push_back((unsigned int) simparams.speed);
				temp1=simparams.delta_t * components[0].sed * omega_sqr;
			}
			if (components[k].sigma != 0)
			{
				conc_dep_s(components[k].sigma, &right, &c_current, &a2, temp1, points);
			}
			if (components[k].delta != 0)
			{
				conc_dep_d(components[k].delta, &right, &c_current, &u, &v, &w, temp2, points,
				&cu, &cv, &cw);
			}
			if (speedChanged)
			{
				ldu_flag = true;
				for (unsigned int l=0; l<points; l++)
				{
					for (unsigned int m=0; m<3; m++)
					{
						left[l][m] = b[l][m] + temp2 * a1[l][m] - temp1 * a2[l][m];
					}
				}
				ldu(&left, &right, points, 3, ldu_flag);
				ldu_flag = false;
				speedChanged = false;
				saveTimes = true;
			}
			else
			{
				ldu(&left, &right, points, 3, ldu_flag);
				ldu_flag = false;
			}
			if (fabs(scantimes[scan] - scan_timer) <= (simparams.delta_t/2.0))
			{
				if ((simparams.rnoise != 0) || (simparams.inoise != 0))
				{
					if ((simparams.rnoise != 0) && (simparams.inoise != 0))
					{
						for (i=0; i<points; i++)
						{
							c[i][scan] += right[i] + box_muller(0, maxconc * simparams.rnoise/100 ) + inoise[i];
							concentration[i] = c[i][scan];
						}
					}
					else if (simparams.rnoise != 0)
					{
						for (i=0; i<points; i++)
						{
							c[i][scan] += right[i] + box_muller(0, maxconc * simparams.rnoise/100 );
							concentration[i] = c[i][scan];
						}

					}
					else if (simparams.inoise != 0)
					{
						for (i=0; i<points; i++)
						{
							c[i][scan] += right[i] + inoise[i];
							concentration[i] = c[i][scan];
						}
					}
				}
				else
				{
					for (i=0; i<points; i++)
					{
						c[i][scan] += right[i];
						concentration[i] = c[i][scan];
					}
				}
				scan++;
				if (k == comp-1)
				{
					s_legend->scan_lcd->display((int) scan);
					curve[scan-1] = scan_plot->insertCurve("Optical Density");
					scan_plot->setCurvePen(curve[scan-1], yellow);
					scan_plot->setCurveData(curve[scan-1], radius, concentration, points);
					scan_plot->replot();
					if (scan > 1)
					{
						for (i=0; i<points; i++)
						{
							dcdtData[i] = c[i][scan-2] - c[i][scan-1];
						}
						if (dcdtWindow)
						{
							dcdt_W->dcdtPlot(&radius, &dcdtData, points);
						}
					}
				}
				if (scan == simparams.scans)
				{
					scan=0;
				}
			}
			qApp->processEvents();
			if (stop_calculation)
			{
				stop_calculation = false;
				return;
			}
		}
	}
	if (saveTimes) // more than one speed was used, show the times and speeds.
	{
		QString str, fileName;
		QFile f;
		fileName = USglobal->config_list.result_dir + "/equiltimes.dat";
		f.setName(fileName);
		if (f.open(IO_WriteOnly | IO_Translate))
		{
			QTextStream ts(&f);
			
			ts << tr("Times for changing rotor speeds:\n\n");
			for (i=0; i<speedTimeV.size(); i++)
			{
				hrs = (int) (speedTimeV[i]/3600);
				speedTimeV[i] -= hrs * 3600;
				mins = (int) (speedTimeV[i]/60);
				speedTimeV[i] -= mins * 60;
				secs = (int) speedTimeV[i];
				str.sprintf(tr("Change to speed %d (%d rpm) after %d hrs, %d min, %d sec\n"), i+2, speedV[i], hrs, mins, secs);
				ts << str;
			}
			f.close();
		}
		view_file(fileName);
		saveTimes = false;
	}
	lastscans=simparams.scans;
	s_legend->progress->setProgress(comp*iterations);
	m_legend->pb_speed->setEnabled(true);
}
*/

//vhw = new vHW_Sim(&comp, &points, &run_flag, &components, &c, &simparams, &scantimes);

void SimWin::monomer_dimer()
{
/*
	unsigned int i, j, k, iterations, scan=0;
	double maxconc = 2.0 * components[0].conc;
	double **partial_c, **c_current=0, **temp, *s_bar, *D_bar, temp1, temp2=0.0, temp3, temp4;
	int hrs, mins, secs;
	speedTimeV.clear();
	speedChanged = false;
	speedV.clear();
	bool c_dep_mon = false;
	bool c_dep_dim = false;
	bool ldu_flag = true;
	init();	
	s_bar = new double [points];
	D_bar = new double [points];
	if (components[0].sigma != 0 || components[0].delta != 0)
	{
		c_dep_mon = TRUE;		
	}
	if (components[1].sigma != 0 || components[1].delta != 0)
	{
		c_dep_dim = TRUE;		
	}
	temp = new double* [2];
	temp[0] = new double [2];
	temp[1] = new double [2];
//
// if both monomer or dimer have any kind of concentration dependency, we need 
// a temporary holding matrix for the concentration of both monomer and dimer:
//
	if (c_dep_mon || c_dep_dim)
	{
		c_current = new double* [2];
		c_current[0] = new double [points];
		c_current[1] = new double [points];
	}
//
// allocate memory for the partial concentrations of monomer ([0]) and dimer ([1]):
//
	partial_c = new double* [2];
	partial_c[0] = new double [points];
	partial_c[1] = new double [points];

	temp[0][0] = (-1 + pow((double) (1 + 8 * components[0].conc * components[1].conc), (double) 0.5)) 
								/ (4 * components[1].conc);
	temp[0][1] = (components[0].conc - temp[0][0]);
	
//	cout << "C-total: " << components[0].conc << ", C-monomer: " << temp[0][0] << ", C-dimer: "<< temp[0][1] << endl;
	for (i=0; i<points; i++)
	{
		right[i] = components[0].conc;
		partial_c[0][i] = temp[0][0];
		partial_c[1][i] = temp[0][1];
	}

	iterations = (unsigned int) (0.5 + (simparams.time * 60 / simparams.delta_t));
	s_legend->progress->setTotalSteps(iterations);
	s_legend->progress->reset();
	scan_plot->setAxisScale(QwtPlot::yLeft, 0, maxconc, 0);
	movie_plot->setAxisScale(QwtPlot::xBottom, 5.8, (double) simparams.bottom, 0);
	scan_plot->setAxisScale(QwtPlot::xBottom, 5.8, (double) simparams.bottom, 0);
	m_legend->comp_lcd->display("E");
	movie_plot->setAxisScale(QwtPlot::yLeft, 0, (double) maxconc, 0);
	scan_timer=0.0;

	for (k=0; k<2; k++)
	{
		temp[k][0] = simparams.delta_t * components[k].sed * omega_sqr;
		temp[k][1] = simparams.delta_t * components[k].diff;
	}
	temp1 = simparams.delta_t * omega_sqr;
	for (j=0; j<iterations; j++)
	{
		scan_timer += simparams.delta_t;
		m_legend->time_lcd->display((double) scan_timer);
		if ((c_dep_mon || c_dep_dim))
		{
			for (k=0; k<2; k++)
			{
				for (i=0; i<points; i++)
				{
					c_current[k][i] = partial_c[k][i];
				}
			}
		}
		movie_plot->setCurveData(movie_scan, radius, right, points);
		movie_plot->replot();
		if (speedChanged)
		{
			for (k=0; k<2; k++)
			{
				temp[k][0] = simparams.delta_t * components[k].sed * omega_sqr;
			}
			temp1 = simparams.delta_t * omega_sqr;
			speedTimeV.push_back(scan_timer);
			speedV.push_back((unsigned int) simparams.speed);
			speedChanged = false;
			saveTimes = true;
		}
		if (c_dep_mon || c_dep_dim)  	// check if there is any concentration dependency
		{										// otherwise, skip.
			for (k=0; k<2; k++)
			{
				if (components[k].sigma != 0)
				{
					conc_dep_s(components[k].sigma, &partial_c[k], &c_current[k], &a2, temp[k][0],
								 points);
				}
				if (components[k].delta != 0)
				{
					conc_dep_d(components[k].delta, &partial_c[k], &c_current[k], &u, &v, &w, 
									temp[k][1], points, &cu, &cv, &cw);
				}
			}
		}
//cout << "monomer s: " << components[0].sed << ", dimer s: " << components[1].sed << endl;
//cout << "monomer D: " << components[0].diff << ", dimer D: " << components[1].diff << endl;
		for (i=0; i<points; i++)
		{
			s_bar[i] = ((components[0].sed * partial_c[0][i])
						+  (components[1].sed * partial_c[1][i]))/right[i];
//cout << "C-right: " << right[i] << ", C-monomer: " << partial_c[0][i] << ", C-dimer: "<< partial_c[1][i] << ", sbar: " << s_bar[i] << endl;
		}
		for (i=0; i<points-1; i++)
		{

// 
//			D_bar[i] = (components[0].diff + (2 * components[1].diff * components[1].conc * partial_c[0][i]))
//						/ (1 + (2 * components[1].conc * partial_c[0][i]));
//	cout << "D_bar[i] " << D_bar[i] << endl;


			if (((partial_c[0][i+1] - partial_c[0][i])/simparams.delta_r + 
				 (partial_c[1][i+1] - partial_c[1][i])/simparams.delta_r) > 1e-5)
			{

				temp3 = (partial_c[0][i+1] - partial_c[0][i])/simparams.delta_r;
				temp4 = (partial_c[1][i+1] - partial_c[1][i])/simparams.delta_r;
				D_bar[i] = ((components[0].diff * temp3) + (components[1].diff * temp4))
							/ (temp3 + temp4);
				
//cout << "D-bar: " << D_bar[i] << ", delta_C: " << ((partial_c[0][i+1] - partial_c[0][i])/simparams.delta_r + 
//		(partial_c[1][i+1] - partial_c[1][i])/simparams.delta_r)<<endl;


			}
			else
			{
				D_bar[i] = (components[0].diff + components[1].diff)/2;
			}

		}
		D_bar[points-1] = (components[0].diff + components[1].diff)/2;
		for (i=0; i<points; i++)
		{
			for (k=0; k<3; k++)
			{
				left[i][k] = b[i][k] + simparams.delta_t * D_bar[i] * a1[i][k] -
				temp1 * s_bar[i] * a2[i][k];
			}
		}
		m3vm(&b, &right, points);
		ldu(&left, &right, points, 3, ldu_flag);
		ldu_flag = true;
		if (fabs(scantimes[scan] - scan_timer) <= (simparams.delta_t/2.0))
		{
			for (i=0; i<points; i++)
			{
				c[i][scan] = right[i];
				concentration[i] = c[i][scan];
			}
			scan++;
			s_legend->scan_lcd->display((int)scan);
			curve[scan-1] = scan_plot->insertCurve("Optical Density");
			scan_plot->setCurvePen(curve[scan-1], yellow);
			scan_plot->setCurveData(curve[scan-1], radius, concentration, points);
			scan_plot->replot();
			if (scan > 1)
			{
				for (i=0; i<points; i++)
				{
					dcdtData[i] = c[i][scan-2] - c[i][scan-1];
				}
				if (dcdtWindow)
				{
					dcdt_W->dcdtPlot(&radius, &dcdtData, points);
				}
			}
			if (scan == simparams.scans)
			{
				scan=0;
			}
		}
		for (i=0; i<points; i++)
		{
			temp2 =  1 + 8 * right[i] * components[1].conc;
			if (temp2 > 0)
			{
				partial_c[0][i] = (-1 + pow((double) temp2, (double) 0.5)) 
								/ (4 * components[1].conc);
				partial_c[1][i] = right[i] - partial_c[0][i];
//cout << "C-right: " << right[i] << ", C-monomer: " << partial_c[0][i] << ", C-dimer: "<< partial_c[1][i] << endl;
			}
			else
			{
				QMessageBox::message( tr("Warning"), tr("Attention:\n\n"
											 "The solution encountered a discontinuity at the bottom of the cell. This\n"
											 "can be caused when cases with small diffusion coefficients are simulated with\n"
											 "too few datapoints. Try to simulate the data with more datapoints or to\n"
											 "simulate a shorter experiment.\n\n"
											 "Alternatively, the data can be simulated with a larger diffusion coefficient\n"
											 "or slower speed."));
			}
		}
		s_legend->progress->setProgress(j);
		qApp->processEvents();
		if (stop_calculation)
		{
			stop_calculation = false;
			return;
		}
	}
	if (saveTimes) // more than one speed was used, show the times and speeds.
	{
		QString str, fileName;
		QFile f;
		fileName = USglobal->config_list.result_dir + "/equiltimes.dat";
		f.setName(fileName);
		if (f.open(IO_WriteOnly | IO_Translate))
		{
			QTextStream ts(&f);
			
			ts << tr("Times for changing rotor speeds:\n\n");
			for (i=0; i<speedTimeV.size(); i++)
			{
				hrs = (int) (speedTimeV[i]/3600);
				speedTimeV[i] -= hrs * 3600;
				mins = (int) (speedTimeV[i]/60);
				speedTimeV[i] -= mins * 60;
				secs = (int) speedTimeV[i];
				str.sprintf(tr("Change to speed %d (%d rpm) after %d hrs, %d min, %d sec\n"), i+2, speedV[i], hrs, mins, secs);
				ts << str;
			}
			f.close();
		}
		//view_file(fileName);
		TextEdit *e;
		e = new TextEdit();
		e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
		e->load(fileName);
		e->show();
		saveTimes = false;
	}
	lastscans = simparams.scans;
	s_legend->progress->setProgress(iterations);
*/
}

void SimWin::isomerization()
{
/*
	unsigned int i, j, k, iterations, scan=0;
	double maxconc = 2.0 * components[0].conc;
	double **partial_c, **c_current=0, **temp, *s_bar, *D_bar, temp1;
	bool c_dep_mon = false;
	bool c_dep_dim = false;
	bool ldu_flag = true;

	init();	
	s_bar = new double [points];
	D_bar = new double [points];
	if (components[0].sigma != 0 || components[0].delta != 0)
	{
		c_dep_mon = TRUE;		
	}
	if (components[1].sigma != 0 || components[1].delta != 0)
	{
		c_dep_dim = TRUE;		
	}
	temp = new double* [2];
	temp[0] = new double [2];
	temp[1] = new double [2];
//
// if both monomer or dimer have any kind of concentration dependency, we need 
// a temporary holding matrix for the concentration of both monomer and dimer:
//
	if (c_dep_mon || c_dep_dim)
	{
		c_current = new double* [2];
		c_current[0] = new double [points];
		c_current[1] = new double [points];
	}
//
// allocate memory for the partial concentrations of monomer ([0]) and dimer ([1]):
//
	partial_c = new double* [2];
	partial_c[0] = new double [points];
	partial_c[1] = new double [points];

	for (i=0; i<points; i++)
	{
		right[i] = components[0].conc;
		partial_c[0][i] = right[i] / (components[1].conc + 1);
		partial_c[1][i] = right[i] - partial_c[0][i];
	}

	iterations = (unsigned int) (0.5 + (simparams.time * 60 / simparams.delta_t));
	s_legend->progress->setTotalSteps(iterations);
	s_legend->progress->reset();
	scan_plot->setAxisScale(QwtPlot::yLeft, 0, maxconc, 0);
	movie_plot->setAxisScale(QwtPlot::xBottom, 5.8, (double) simparams.bottom, 0);
	scan_plot->setAxisScale(QwtPlot::xBottom, 5.8, (double) simparams.bottom, 0);
	m_legend->comp_lcd->display("E");
	movie_plot->setAxisScale(QwtPlot::yLeft, 0, (double) maxconc, 0);
	scan_timer=0.0;

	for (k=0; k<2; k++)
	{
		temp[k][0] = simparams.delta_t * components[k].sed * omega_sqr;
		temp[k][1] = simparams.delta_t * components[k].diff;
	}
	temp1 = simparams.delta_t * omega_sqr;
	for (j=0; j<iterations; j++)
	{
		scan_timer += simparams.delta_t;
		m_legend->time_lcd->display( (double) scan_timer);
		if ((c_dep_mon || c_dep_dim))
		{
			for (k=0; k<2; k++)
			{
				for (i=0; i<points; i++)
				{
					c_current[k][i] = partial_c[k][i];
				}
			}
		}
		movie_plot->setCurveData(movie_scan, radius, right, points);
		movie_plot->replot();
		if (c_dep_mon || c_dep_dim)  	// check if there is any concentration dependency
		{										// otherwise, skip.
			for (k=0; k<2; k++)
			{
				if (components[k].sigma != 0)
				{
					conc_dep_s(components[k].sigma, &partial_c[k], &c_current[k], &a2, temp[k][0],
								 points);
				}
				if (components[k].delta != 0)
				{
					conc_dep_d(components[k].delta, &partial_c[k], &c_current[k], &u, &v, &w, 
									temp[k][1], points, &cu, &cv, &cw);
				}
			}
		}
		for (i=0; i<points; i++)
		{
			if ((partial_c[0][i] + partial_c[1][i]) > 1e-37)
			{
				s_bar[i] = (components[0].sed * partial_c[0][i]
							+ components[1].sed * partial_c[1][i])
							/ (partial_c[0][i] + partial_c[1][i]);
			}
			else
			{
				s_bar[i] = (components[0].sed + components[1].sed)/2;
			}
		}
		for (i=0; i<points; i++)
		{
//
//			if (((partial_c[0][i+1] - partial_c[0][i]) + 
//				 (partial_c[1][i+1] - partial_c[1][i])) > 1e-37)
//			{
//
//				D_bar[i] = ((components[0].diff * (partial_c[0][i+1] - partial_c[0][i]))
//							+ (components[1].diff * (partial_c[1][i+1] - partial_c[1][i])))
//							/ ((partial_c[0][i+1] - partial_c[0][i]) + 
//				 				(partial_c[1][i+1] - partial_c[1][i]));

//			}
//			else
//			{
//				D_bar[i] = (components[0].diff + components[1].diff)/2;
//
//			}
//
		}
		D_bar[points-1] = (components[0].diff + components[1].diff)/2;
		for (i=0; i<points; i++)
		{
			for (k=0; k<3; k++)
			{
				left[i][k] = b[i][k] + simparams.delta_t * D_bar[i] * a1[i][k] -
				temp1 * s_bar[i] * a2[i][k];
			}
		}
		m3vm(&b, &right, points);
		ldu(&left, &right, points, 3, ldu_flag);
		ldu_flag = true;
		if (fabs(scantimes[scan] - scan_timer) <= (simparams.delta_t/2.0))
		{
			for (i=0; i<points; i++)
			{
				c[i][scan] = c[i][scan]+right[i];
				concentration[i] = c[i][scan];
			}
			scan++;
			s_legend->scan_lcd->display((int)scan);
			curve[scan-1] = scan_plot->insertCurve("Optical Density");
			scan_plot->setCurvePen(curve[scan-1], yellow);
			scan_plot->setCurveData(curve[scan-1], radius, concentration, points);
			scan_plot->replot();
			if (scan == simparams.scans)
			{
				scan=0;
			}
		}
		for (i=0; i<points; i++)
		{
			partial_c[0][i] = right[i] / (components[1].conc + 1);
			partial_c[1][i] = right[i] - partial_c[0][i];
		}
		s_legend->progress->setProgress(j);
		qApp->processEvents();
		if (stop_calculation)
		{
			stop_calculation = false;
			return;
		}
	}
	lastscans = simparams.scans;
	s_legend->progress->setProgress(iterations);
*/
}

void SimWin::quitthis()
{
	if(dcdtWindow)
	{
		dcdt_W->quit();
		qApp->processEvents();
	}
	close();
}

void SimWin::sim_help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/finsim.html");
}

void SimWin::stop_now()
{
	stop_calculation = true;
	if (mfem)
	{
		mfem->stop();
	}
}


void SimWin::component_reset()
{
	if (model == 1 || model == 2)
	{
		components[0].conc = temp_conc;
		components[1].conc = temp_k;
	}
}


//
// set concentrations for monomer and dimer to the correct initial concentrations:
//
void SimWin::component_init()
{
   double temp;
   switch(model)
   {
      case 1:
      {
         comp=2;
         temp_conc = components[0].conc;
         temp_k = components[1].conc;
         temp = (-1 + pow((double) (1 + 4 * components[0].conc * components[1].conc), 
			       (double) 0.5)) / (2.0 * components[1].conc);
         components[1].conc = components[0].conc - temp;
         components[0].conc = temp;
         break;
      }
      case 2:
      {
         comp=2;
         temp_conc = components[0].conc;
         temp_k = components[1].conc;
         temp = components[0].conc / (components[1].conc + 1);
         components[1].conc = components[0].conc - temp;
         components[0].conc = temp;
         break;
      }
   }
}

