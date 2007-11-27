#include "../include/us_smsim.h"

SecondMoment_Legend_F::SecondMoment_Legend_F(QWidget *p, const char* name)
: QFrame(p, name)
{
	USglobal = new US_Config();
	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setFrameStyle(QFrame::Panel|QFrame::Raised);
	setLineWidth(2);

	progress_lbl = new QLabel(tr("    Analysis Progress:"), this);
	Q_CHECK_PTR(progress_lbl);
	progress_lbl->setAlignment(AlignLeft|AlignVCenter);
	progress_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	progress_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	progress_lbl->setGeometry(178,7,374,26);

	progress = new QProgressBar(100, this, "Analysis Progress");
	progress->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	Q_CHECK_PTR(progress);
	progress->setGeometry(324, 10, 221, 20);

	save = new QPushButton(tr("Save"), this);
	Q_CHECK_PTR(save);
	save->setAutoDefault(false);
	save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	save->setGeometry(7, 7, 80, 26);
	save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(save, SIGNAL(clicked()), SIGNAL(clicked()));

	close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(close);
	close->setAutoDefault(false);
	close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	close->setGeometry(94, 7, 77, 26);
	close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(close, SIGNAL(clicked()), SIGNAL(clicked()));
}

SecondMoment_Legend_F::~SecondMoment_Legend_F()
{
}

SecondMoment::SecondMoment(int mod, unsigned int *comps, unsigned int *pts, double ***concentration, 
						vector <struct component> *components,
						struct simulation_parameters *simparams, float **scan_t,
						float temp_conc, float temp_k, QWidget *p , const char *name) : QWidget(p, name)
{
	
	USglobal = new US_Config();
	model = mod;
	total_conc = temp_conc;
	equil_k = temp_k;
	points = *pts;
	comp = *comps;
	temp_sp = simparams;
	temp_components = components;
	c = concentration;
	scantimes = scan_t;
	omega = (double) (M_PI * (*temp_sp).speed/30);
	omega = square(omega);
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

	secm_plot = new QwtPlot(this);
	Q_CHECK_PTR(secm_plot);
	secm_plot->setAxisTitle(QwtPlot::yLeft, tr("Sedimentation Coefficient x 10^(13) sec"));
	secm_plot->enableGridXMin();
	secm_plot->enableGridYMin();
	secm_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	secm_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	secm_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	//secm_plot->setPlotBackground(USglobal->global_colors.plot);		//old version
	secm_plot->setCanvasBackground(USglobal->global_colors.plot);		//new version
	secm_plot->setAxisTitle(QwtPlot::xBottom, tr("Scan Number"));
	secm_plot->enableOutline(true);
	secm_plot->setOutlinePen(white);
	secm_plot->setOutlineStyle(Qwt::Cross);
	secm_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	secm_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	secm_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	secm_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	secm_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	secm_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	secm_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));		
	secm_plot->setMargin(USglobal->config_list.margin);

	secm_legend = new SecondMoment_Legend_F(this);
	Q_CHECK_PTR(secm_legend);
	connect(secm_legend->save, SIGNAL(clicked()), this, SLOT(save_secm()));
	connect(secm_legend->close, SIGNAL(clicked()), this, SLOT(close_secm()));
	recalc();

	global_Xpos += 30;
	global_Ypos += 30;
	
	move(global_Xpos, global_Ypos);
}

SecondMoment::~SecondMoment()
{
}

void SecondMoment::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void SecondMoment::resizeEvent(QResizeEvent *e)
{
	QRect r(0,0, e->size().width(), (int) (e->size().height()-44));
	secm_plot->setGeometry(r);
	secm_legend->setGeometry(0, e->size().height()-42, r.width(), 40);
}

void SecondMoment::recalc()
{
	QwtSymbol sym;
	setups();
	double sum1, sum2, dcdr;

	for (i=0; i<(*temp_sp).scans; i++)
	{
		sum1 = 0.0;
		sum2 = 0.0;
		count=1;
		while ((*c)[i][count] < plateau[i])
		{
			count++;		// climb up the concentration gradient until the plateau is reached
			dcdr = ((*c)[i][count] - (*c)[i][count-1])/(*temp_sp).resolution;
			sum1 += pow((count * (*temp_sp).resolution + (*temp_sp).meniscus), 2) * dcdr * (*temp_sp).resolution;
			sum2 += dcdr * (*temp_sp).resolution;
		}
		smp[i] = sqrt(sum1/sum2); //second moment points in cm
		sms[i] = 1e13 * log (smp[i]/(*temp_sp).meniscus) / (omega * (*scantimes)[i]); //second moment s

//
//update the progress bar:
// 
		secm_legend->progress->setProgress(i+1);
	}
	secm_plot->clear();
	secm_plot->enableGridXMin();
	secm_plot->enableGridYMin();
	secm_plot->setGridMajPen(QPen(colorGroup().light(), 0, DotLine));
	secm_plot->setGridMinPen(QPen(colorGroup().mid(), 0 , DotLine));
//
// sedimentation coefficients from all scans that have not cleared the meniscus
// form a  separate plot that will be plotted in red, and will not be included 
// in the line fit:
//
	for (i=0; i<exclude; i++)
	{
		plot1_x[i] = (double) (i + 1);
		plot1_y[i] = sms[i];
	}
//
// All others go into plot 2:
//
	avg = 0;
	for (i=0; i<(*temp_sp).scans-exclude; i++)
	{
		plot2_x[i] = (double) (i + exclude + 1);
		plot2_y[i] = sms[i+exclude];
		avg += sms[i+exclude];
	}
	avg = avg/((*temp_sp).scans - exclude);
//
// Plot 3 is a constant line of the average of all scans included
//
	plot3_x[0] = 0.0;
	plot3_x[1] = (*temp_sp).scans;
	plot3_y[0]=avg;
	plot3_y[1]=avg;
	sym.setStyle(QwtSymbol::Ellipse);
	sym.setPen(white);
	sym.setBrush(red);
	sym.setSize(8);
	plot1 = secm_plot->insertCurve("non-cleared Sedimentation Coefficients");
	secm_plot->setCurveStyle(plot1, QwtCurve::NoCurve);
	secm_plot->setCurveSymbol(plot1, QwtSymbol(sym));
	secm_plot->setCurveData(plot1, plot1_x, plot1_y, exclude);

	sym.setStyle(QwtSymbol::Ellipse);
	sym.setPen(blue);
	sym.setBrush(white);
	sym.setSize(8);
	plot2 = secm_plot->insertCurve("cleared Sedimentation Coefficients");
	secm_plot->setCurveStyle(plot2, QwtCurve::Spline);
	secm_plot->setCurveSymbol(plot2, QwtSymbol(sym));
	secm_plot->setCurvePen(plot2, yellow);
	secm_plot->setCurveData(plot2, plot2_x, plot2_y, (*temp_sp).scans-exclude);

	plot3 = secm_plot->insertCurve("Average");
	secm_plot->setCurveStyle(plot3, QwtCurve::Lines);
	secm_plot->setCurvePen(plot3, yellow);
	secm_plot->setCurveData(plot3, plot3_x, plot3_y, 2);

	secm_plot->replot();
}

void SecondMoment::setups()
{
//
// Calculate expected plateaus for each scan by summing the plateau contributions 
// of each component:
//
	divisions = 2000;
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
// 
// make sure that the first datapoint of each scan has a lower concentration than 
// what would be needed for the first division:
//
	exclude=0;
	for (i=0; i<(*temp_sp).scans; i++)
	{
		if ((*c)[i][0] > plateau[i]/divisions)
		{
			exclude++;
		}
	}
	if (exclude > 0)
	{
		QMessageBox::message( tr("Attention:"), 
				tr("One or more scans have not cleared the meniscus\n"
					"(as defined by 1/2000 of the total concentration).\n"
					"Scans that do not clear the meniscus will provide\n"
					"unreliable sedimentation coeffficients (generally\n"
					"too high). These scans will be plotted in red color.\n\n"
					"You can try to increase the length of the experiment\n"
					"or use a higher speed for the same sedimentation coefficient.\n"
					"Alternatively, you can increase the sedimentation coefficient\n"
					"and decrease the diffusion coefficient for the slowest component(s)."));
	}
	secm_legend->progress->setTotalSteps((*temp_sp).scans);
	secm_legend->progress->reset();
	secm_legend->progress->setProgress(0);
	smp = new double [(*temp_sp).scans];
	sms = new double [(*temp_sp).scans];
	plot1_x = new double [exclude];
	plot1_y = new double [exclude];
	plot2_x = new double [(*temp_sp).scans-exclude];
	plot2_y = new double [(*temp_sp).scans-exclude];
	plot3_x = new double [2];
	plot3_y = new double [2];
}

void SecondMoment::save_secm()
{
	QString fn = QFileDialog::getSaveFileName( 0, "*.sm", 0);
	int k;
	if ( !fn.isEmpty() ) 
	{
		k = fn.find(".", 0, FALSE);
		if (k != -1) //if an extension was given, strip it
		{
			fn.truncate(k);
		}
		save_secm(fn);		// the user gave a file name
	}
}

void SecondMoment::save_secm(const QString &filename)
{
	QString filestr;
	filestr = QString(filename);
	filestr.append(".sm");
	QFile f(filestr);
	unsigned int i;
	if (f.exists())
	{
		if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
		{
			f.close();
			return;
		}
	}
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		ts << tr("Scan #\tSecond Moment Point (cm)\tSecond Moment S-value\n");
		for (i=0; i<(*temp_sp).scans; i++)
		{
			ts << (i+1) << "\t" << smp[i] << "\t" << sms[i] << "\n";
		}
		f.close();
	}
}

void SecondMoment::close_secm()
{
//
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



