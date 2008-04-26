#include "../include/us_dcdtsim.h"

dCdT_Legend_F::dCdT_Legend_F(double radius, QWidget *p, const char* name)
: QFrame(p, name)
{
	USglobal = new US_Config();

	radius_lbl = new QLabel(tr(" Clip at (cm):"), this);
	Q_CHECK_PTR(radius_lbl);

	setFrameStyle(QFrame::Panel|QFrame::Raised);
	setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	setLineWidth(2);

	radius_lbl->setAlignment(AlignLeft|AlignVCenter);
	radius_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	radius_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	radius_lbl->setGeometry(7, 7, 210, 26);

	radius_counter= new QwtCounter(this);
	Q_CHECK_PTR(radius_counter);
	radius_counter->setRange(6.0, 7.2, 0.01);
	radius_counter->setValue(radius);
	radius_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	radius_counter->setGeometry(90,9,120,22);

	progress_lbl = new QLabel(tr("    Analysis Progress:"), this);
	Q_CHECK_PTR(progress_lbl);
	progress_lbl->setAlignment(AlignLeft|AlignVCenter);
	progress_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	progress_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	progress_lbl->setGeometry(210,7,342,26);

	progress = new QProgressBar(100, this, "Analysis Progress");
	Q_CHECK_PTR(progress);
	progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	progress->setGeometry(350, 10, 195, 20);

	calc = new QPushButton(tr("Calculate"), this);
	Q_CHECK_PTR(calc);
	calc->setAutoDefault(false);
	calc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	calc->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	calc->setGeometry(7, 40, 90, 26);

	average = new QPushButton(tr("Average"), this);
	Q_CHECK_PTR(average);
	average->setAutoDefault(false);
	average->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	average->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	average->setGeometry(107, 40, 90, 26);

	close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(close);
	close->setAutoDefault(false);
	close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	close->setGeometry(207, 40, 80, 26);
	close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
}

dCdT_Legend_F::~dCdT_Legend_F()
{
}

dCdT::dCdT(int temp_model, unsigned int *comps, vector <struct component> *components,
				double ***concentration, struct simulation_parameters *simparams, 
				float **scan_t, float temp_conc, float temp_k, QWidget *p , const char *name) : QWidget(p, name)
{
	USglobal = new US_Config();
	setup_already = false;
	test = false;
	radius=7.2;
	model = temp_model;
	total_conc = temp_conc;
	equil_k = temp_k;
	temp_sp = simparams;
	comp = *comps;
	temp_components = components;
	c = concentration;
	scantimes = scan_t;
	omega = (double) (M_PI * (*temp_sp).speed/30);
	omega = square(omega);

	dcdt_plot = new QwtPlot(this);
	Q_CHECK_PTR(dcdt_plot);
	dcdt_plot->setAxisTitle(QwtPlot::xBottom, tr("Apparent Sedimentation Coefficient x 10^(13) sec"));
	dcdt_plot->setAxisTitle(QwtPlot::yLeft, tr("Relative Amount"));
	dcdt_plot->enableGridXMin();
	dcdt_plot->enableGridYMin();
	dcdt_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	dcdt_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	dcdt_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	//dcdt_plot->setPlotBackground(USglobal->global_colors.plot);		//old version
	dcdt_plot->setCanvasBackground(USglobal->global_colors.plot);		//new version	
	dcdt_plot->enableOutline(true);
	dcdt_plot->setOutlinePen(white);
	dcdt_plot->setOutlineStyle(Qwt::Cross);
	dcdt_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	dcdt_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	dcdt_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	dcdt_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	dcdt_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	dcdt_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	dcdt_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));		
	dcdt_plot->setMargin(USglobal->config_list.margin);

	dcdt_legend = new dCdT_Legend_F(radius, this);
	Q_CHECK_PTR(dcdt_legend);

//	connect(dcdt_legend->radius_counter, SIGNAL(buttonReleased(double)), this, SLOT(change_radius(double)));
	connect(dcdt_legend->radius_counter, SIGNAL(valueChanged(double)), this, SLOT(change_radius(double)));
	connect(dcdt_legend->calc, SIGNAL(clicked()), this, SLOT(recalc()));
	connect(dcdt_legend->average, SIGNAL(clicked()), this, SLOT(average()));
	connect(dcdt_legend->close, SIGNAL(clicked()), this, SLOT(close_dcdt()));

	global_Xpos += 30;
	global_Ypos += 30;
	
	move(global_Xpos, global_Ypos);
}

dCdT::~dCdT()
{
}

void dCdT::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void dCdT::resizeEvent(QResizeEvent *e)
{
	QRect r(0,0, e->size().width(), (int) (e->size().height()-77));
	dcdt_plot->setGeometry(r);
	dcdt_legend->setGeometry(0, e->size().height()-75, r.width(), 73);
}

void dCdT::change_radius(double val)
{
	radius = val;
}

void dCdT::average()
{
	QString htmlDir = USglobal->config_list.html_dir + "/simulated";
	QDir d(htmlDir);
	if (!d.exists())
	{
		d.mkdir(htmlDir, true);
	}

	if (test)
	{
		avg = new US_Average(points, ((*temp_sp).scans-1), &sval, &dcdt_points, "dcdt", htmlDir, "Simulated Data", 1, 1);
		avg->setCaption(tr("Time Derivative: Average S Distribution:"));
		avg->show();
	}
	else
	{
		QMessageBox::message(tr("Attention:"), tr("You have to calculate\nthe data before averaging!"));
	}
}

void dCdT::recalc()
{
	if (setup_already)
	{
		cleanup();
	}
	setups();
	dcdt_plot->clear();
	dcdt_plot->enableGridXMin();
	dcdt_plot->enableGridYMin();
	dcdt_plot->setGridMajPen(QPen(colorGroup().light(), 0, DotLine));
	dcdt_plot->setGridMinPen(QPen(colorGroup().mid(), 0 , DotLine));

	for (j=1; j<(*temp_sp).scans; j++)
	{

		for (i=0; i<points; i++)
		{
//
// Normalize the plateaus:
//
			dcdt_points[j-1][i] = (*c)[j-1][i]/plateau[j-1] - (*c)[j][i]/plateau[j];
			sval[j-1][i] = 1e13 * log (((*temp_sp).resolution * i + (*temp_sp).meniscus)
							/(*temp_sp).meniscus) / 
							(omega * ((*scantimes)[j-1]+(*scantimes)[j])/2);
		}
		dcdt_legend->progress->setProgress(j);
		plots[j-1] = dcdt_plot->insertCurve("dcdt");
		dcdt_plot->setCurveStyle(plots[j-1], QwtCurve::Lines);
		dcdt_plot->setCurvePen(plots[j-1], yellow);
		dcdt_plot->setCurveData(plots[j-1], sval[j-1], dcdt_points[j-1], points);
	}
	dcdt_plot->replot();
}

void dCdT::setups()
{
//
// Calculate expected plateaus for each scan by summing the plateau contributions 
// of each component:
//
	plateau = new double[(*temp_sp).scans];
	for (i=0; i<(*temp_sp).scans; i++)
	{
		plateau[i] = 0.0;
		for (j=0; j<comp; j++)
		{
//
// This is the equation for radial dilution:
//
			plateau[i] += (*temp_components)[j].conc * exp(-2.0 * 
			(*temp_components)[j].sed * omega * (*scantimes)[i]);
		}
	}
	dcdt_legend->progress->setTotalSteps((*temp_sp).scans-1);
	dcdt_legend->progress->reset();
	dcdt_legend->progress->setProgress(0);
//
// The number of points for which the derivative is calculated, the counter
// in the legend allows clipping the radius at the bottom of the cell to 
// avoid the large derivatives at the bottom of the cell:
//
	points = (unsigned int) ((radius - (*temp_sp).meniscus)/(*temp_sp).resolution);
	dcdt_points = new double* [(*temp_sp).scans-1];
	Q_CHECK_PTR(dcdt_points);
	sval = new double* [(*temp_sp).scans-1];
	Q_CHECK_PTR(sval);
	for (i=0; i<(*temp_sp).scans-1; i++)
	{
		dcdt_points[i] = new double [points];
		Q_CHECK_PTR(dcdt_points[i]);
		sval[i] = new double [points];
		Q_CHECK_PTR(sval[i]);
	}
	plots = new uint [(*temp_sp).scans-1];
	Q_CHECK_PTR(plots);
	test = true;
	setup_already = true;
}

void dCdT::cleanup()
{
	for (i=0; i<(*temp_sp).scans-1; i++)
	{
		delete [] dcdt_points[i];
		delete [] sval[i];
	}
	delete [] dcdt_points;
	delete [] sval;
	delete [] plots;
}

void dCdT::close_dcdt()
{//
// temp_components[0/1].conc are now set to partial concentrations, 
// reset to total concentration and equilibrium constant before returning
//
	if (model == 1 || model == 2)
	{
		(*temp_components)[0].conc = total_conc;
		(*temp_components)[1].conc = equil_k;
	}

	close();
}



