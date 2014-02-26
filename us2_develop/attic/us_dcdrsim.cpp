#include "../include/us_dcdrsim.h"

dCdr_Legend_F::dCdr_Legend_F(double radius, QWidget *p, const char* name)
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
	progress->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
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

dCdr_Legend_F::~dCdr_Legend_F()
{
}

dCdr::dCdr(double ***concentration, struct simulation_parameters *simparams, 
				float **scan_t, QWidget *p , const char *name) : QWidget(p, name)
{
	USglobal = new US_Config();
	setup_already = false;
	test = false;
	radius=7.2;
	temp_sp = simparams;
	c = concentration;
	scantimes = scan_t;
	omega = (double) (M_PI * (*temp_sp).speed/30);
	omega = square(omega);

	dcdr_plot = new QwtPlot(this);
	Q_CHECK_PTR(dcdr_plot);
	dcdr_plot->enableGridXMin();
	dcdr_plot->enableGridYMin();
	dcdr_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	dcdr_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	dcdr_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	//dcdr_plot->setPlotBackground(USglobal->global_colors.plot);		//old version
	dcdr_plot->setCanvasBackground(USglobal->global_colors.plot);		//new version
	dcdr_plot->setAxisTitle(QwtPlot::xBottom, tr("Apparent Sedimentation Coefficient x 10^(13) sec"));
	dcdr_plot->setAxisTitle(QwtPlot::yLeft, tr("Relative Amount"));
	dcdr_plot->enableOutline(true);
	dcdr_plot->setOutlinePen(white);
	dcdr_plot->setOutlineStyle(Qwt::Cross);
	dcdr_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	dcdr_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	dcdr_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	dcdr_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	dcdr_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	dcdr_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	dcdr_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));		
	dcdr_plot->setMargin(USglobal->config_list.margin);


	dcdr_legend = new dCdr_Legend_F(radius, this);
	Q_CHECK_PTR(dcdr_legend);

//	connect(dcdr_legend->radius_counter, SIGNAL(buttonReleased(double)), this, SLOT(change_radius(double)));
	connect(dcdr_legend->radius_counter, SIGNAL(valueChanged(double)), this, SLOT(change_radius(double)));
	connect(dcdr_legend->calc, SIGNAL(clicked()), this, SLOT(recalc()));
	connect(dcdr_legend->average, SIGNAL(clicked()), this, SLOT(average()));
	connect(dcdr_legend->close, SIGNAL(clicked()), this, SLOT(close_dcdr()));

	global_Xpos += 30;
	global_Ypos += 30;
	
	move(global_Xpos, global_Ypos);
}

dCdr::~dCdr()
{
}

void dCdr::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void dCdr::resizeEvent(QResizeEvent *e)
{
	QRect r(0,0, e->size().width(), (int) (e->size().height()-77));
	dcdr_plot->setGeometry(r);
	dcdr_legend->setGeometry(0, e->size().height()-75, r.width(), 73);
}

void dCdr::change_radius(double val)
{
	radius = val;
}

void dCdr::average()
{
	
	QString htmlDir = USglobal->config_list.html_dir + "/simulated";
	QDir d(htmlDir);
	if (!d.exists())
	{
		d.mkdir(htmlDir, true);
	}

	if (test)
	{
		avg = new US_Average((points-1), (*temp_sp).scans, &sval, &dcdr_points, "dcdr", htmlDir, "Simulated Data", 1, 1);
		avg->setCaption(tr("Radial Derivative: Average S Distribution:"));
		avg->show();
	}
	else
	{
		QMessageBox::message(tr("Attention:"), tr("You have to calculate\nthe data before averaging!"));
	}

}

void dCdr::update_progress(int val)
{
	dcdr_legend->progress->setProgress(val);
}

void dCdr::recalc()
{
	if (setup_already)
	{
		cleanup();
	}
	setups();
	dcdr_plot->clear();

	for (j=0; j<(*temp_sp).scans; j++)
	{

		for (i=1; i<points; i++)
		{
			dcdr_points[j][i-1] = (*c)[j][i] - (*c)[j][i-1];
			sval[j][i-1]= 1e13 * log (((*temp_sp).resolution * i + (*temp_sp).meniscus)
							/(*temp_sp).meniscus) / (omega * (*scantimes)[j]);
		}
		dcdr_legend->progress->setProgress(j+1);
		plots[j] = dcdr_plot->insertCurve("dcdr");
		dcdr_plot->setCurveStyle(plots[j], QwtCurve::Lines);
		dcdr_plot->setCurvePen(plots[j], yellow);
		dcdr_plot->setCurveData(plots[j], sval[j], dcdr_points[j], points-1);
	}
	dcdr_plot->replot();
}

void dCdr::setups()
{
	dcdr_legend->progress->setTotalSteps((*temp_sp).scans);
	dcdr_legend->progress->reset();
	dcdr_legend->progress->setProgress(0);
//
// The number of points for which the derivative is calculated, the counter
// in the legend allows clipping the radius at the bottom of the cell to 
// avoid the large derivatives at the bottom of the cell:
//
	points = (unsigned int) ((radius - (*temp_sp).meniscus)/(*temp_sp).resolution);
	dcdr_points = new double* [(*temp_sp).scans];
	Q_CHECK_PTR(dcdr_points);
	sval = new double* [(*temp_sp).scans];
	Q_CHECK_PTR(sval);
	for (i=0; i<(*temp_sp).scans; i++)
	{
		dcdr_points[i] = new double [points-1];
		Q_CHECK_PTR(dcdr_points[i]);
		sval[i] = new double [points-1];
		Q_CHECK_PTR(sval[i]);
	}
	plots = new uint [(*temp_sp).scans];
	Q_CHECK_PTR(plots);
	test = true;
	setup_already = true;
}

void dCdr::cleanup()
{
	for (i=0; i<(*temp_sp).scans; i++)
	{
		delete [] dcdr_points[i];
		delete [] sval[i];
	}
	delete [] dcdr_points;
	delete [] sval;
	delete [] plots;
}

void dCdr::close_dcdr()
{
	close();
}

