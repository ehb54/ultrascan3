#include "../include/us_hydro.h"
const int ARRAYSIZE = 999;

US_Hydro1::US_Hydro1(QWidget *p, const char* name) : QDialog(p, name)
{
	allparams = new struct hydrosim;
	USglobal = new US_Config();
	data_control = new Data_Control_W(101);
	data_control->step=1;
	data_control->run_inf.investigator=0;
	data_control->density   = (float) DENS_20W;
	data_control->viscosity = (float) (100.0 * VISC_20W);
	data_control->vbar20    =  (float) 0.72;
	data_control->run_inf.avg_temperature = 20;
	data_control->vbar = data_control->vbar20 + (4.25e-4 * (data_control->run_inf.avg_temperature - 20));
	data_control->calc_correction(data_control->run_inf.avg_temperature);

	GUI();
}

US_Hydro1::US_Hydro1(struct hydrosim *temp_allparams, QWidget *p, const char* name) : QDialog(p, name)
{
	allparams = temp_allparams;
	USglobal = new US_Config();
	data_control = new Data_Control_W(101);
	data_control->step=1;
	data_control->run_inf.investigator=0;
	data_control->density   =  (float) DENS_20W;
	data_control->viscosity =  (float) (100.0 * VISC_20W);
	data_control->vbar20    =  (float) 0.72;
	data_control->run_inf.avg_temperature = 20;
	data_control->vbar = data_control->vbar20 + (4.25e-4 * (data_control->run_inf.avg_temperature - 20));
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	GUI();
}

US_Hydro1::~US_Hydro1()
{
}

void US_Hydro1::GUI()
{
	int i;	
	xpos = 5;
	ypos = 5;
	width = 140;
	height = 26;
	QString str;

	oblate = new double [ARRAYSIZE];
	rod = new double [ARRAYSIZE];
	prolate = new double [ARRAYSIZE];
	ratio_x = new double [ARRAYSIZE];
	calc_models();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
//	setMinimumSize(8*(unsigned int) (2*width/3)+10+7*spacing, 13*(height+spacing)+109);	
//	setMaximumSize(8*(unsigned int) (2*width/3)+10+7*spacing, 13*(height+spacing)+109);	

	QPen pen;
	plt = new QwtPlot(this);
	plt->enableOutline(true);
	plt->setOutlinePen(white);
	plt->setOutlineStyle(Qwt::VLine);
	plt->setTitle(tr("f/f0 Dependence on Axial Ratios"));
	plt->legend()->setReadOnly(TRUE);
	cv_prolate = plt->insertCurve("Prolate Ellipsoid");
	cv_oblate = plt->insertCurve("Oblate Ellipsoid");
	cv_rod = plt->insertCurve("Long Rod");
	cv_vline = plt->insertCurve("Axial Ratio");
	plt->enableLegend(TRUE);
	plt->setLegendPos(Qwt::Bottom);
	plt->setLegendFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	QIntDictIterator<QWidget> it = plt->legend()->itemIterator();
	for ( QWidget *w = it.toFirst(); w != 0; w = ++it)
	{
		w->setPaletteBackgroundColor(Qt::black);
		w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1));
	}
	pen.setWidth(2);
	pen.setColor(Qt::magenta);
	plt->setCurvePen(cv_prolate, pen);
	pen.setColor(Qt::yellow);
	plt->setCurvePen(cv_oblate, pen);
	pen.setColor(Qt::cyan);
	plt->setCurvePen(cv_rod, pen);
	pen.setColor(Qt::white);
	plt->setCurvePen(cv_vline, pen);
	plt->enableGridXMin();
	plt->enableGridYMin();
	plt->setMinimumHeight(350);
	plt->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	plt->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	plt->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	plt->setCanvasBackground(USglobal->global_colors.plot);		//new version
	plt->setMargin(USglobal->config_list.margin);
	plt->setAxisTitle(QwtPlot::xBottom, "Axial Ratio");
	plt->setAxisTitle(QwtPlot::yLeft, "f/f0");
	plt->setAxisScale(QwtPlot::xBottom, 0, 100);
	plt->setAxisScale(QwtPlot::yLeft, 1, 4.5);
	plt->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	plt->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	plt->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	plt->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	plt->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	plt->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	plt->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));			
	plt->setCurveData(cv_prolate, ratio_x, prolate, ARRAYSIZE);
	plt->setCurveData(cv_oblate, ratio_x, oblate, ARRAYSIZE);
	plt->setCurveData(cv_rod, ratio_x, rod, ARRAYSIZE);

	vline_x[0]=10.0;
	vline_x[1]=10.0;
	vline_y[0]=1.1;
	vline_y[1]=4.3;
	plt->setCurveData(cv_vline, vline_x, vline_y, 2);

	pb_density = new QPushButton(tr("Density:"), this);
	Q_CHECK_PTR(pb_density);
	pb_density->setAutoDefault(false);
	pb_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_density->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_density->setGeometry(xpos, ypos, width, height);
	connect(pb_density, SIGNAL(clicked()), data_control, SLOT(get_buffer()));
	connect(data_control, SIGNAL(bufferChanged(float, float)), SLOT(update_buffer_lbl(float, float)));

	xpos += width + spacing;

	le_density = new QLineEdit(this);
	le_density->setGeometry(xpos, ypos, (unsigned int) width/2, height);
	le_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_density->setText(str.sprintf("%8.6f", DENS_20W));
	le_density->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_density, SIGNAL(textChanged(const QString &)), SLOT(update_density_lbl(const QString &)));

	ypos += height + spacing;
	xpos = 5;	

	pb_viscosity = new QPushButton(tr("Viscosity:"), this);
	Q_CHECK_PTR(pb_viscosity);
	pb_viscosity->setAutoDefault(false);
	pb_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_viscosity->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_viscosity->setGeometry(xpos, ypos, width, height);
	connect(pb_viscosity, SIGNAL(clicked()), data_control, SLOT(get_buffer()));
	
	xpos += width + spacing;

	le_viscosity = new QLineEdit(this);
	le_viscosity->setGeometry(xpos, ypos, (unsigned int) width/2, height);
	le_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_viscosity->setText(str.sprintf("%8.6f", (VISC_20W * 100)));
	le_viscosity->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_viscosity, SIGNAL(textChanged(const QString &)), SLOT(update_viscosity_lbl(const QString &)));

	ypos += height + spacing;
	xpos = 5;	

	pb_vbar = new QPushButton(tr(" vbar(20º):"), this);
	Q_CHECK_PTR(pb_vbar);
	pb_vbar->setAutoDefault(false);
	pb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_vbar->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_vbar->setGeometry(xpos, ypos, width, height);
	connect(pb_vbar, SIGNAL(clicked()), data_control, SLOT(read_vbar()));
	connect(data_control, SIGNAL(vbarChanged(float, float)), SLOT(update_vbar_lbl(float, float)));
	xpos += width + spacing;

	le_vbar = new QLineEdit(this);
	le_vbar->setGeometry(xpos, ypos, (unsigned int) width/2, height);
	le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_vbar->setText(" .7200");
	le_vbar->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_vbar_lbl(const QString &)));

	ypos += height + spacing;
	xpos = 5;	

	lbl_mw = new QLabel(tr("Molecular Weight:"), this);
	lbl_mw->setAlignment(AlignCenter|AlignVCenter);
	lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_mw->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw->setGeometry(xpos, ypos, width, height);
	
	xpos += width + spacing;
	
	mw = 50000;
	str.sprintf("%4.3e", mw);
	le_mw = new QLineEdit(this, "Molecular Weight");
	le_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_mw->setGeometry(xpos, ypos, (unsigned int) width/2, height);
	le_mw->setText(str);
	le_mw->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_mw, SIGNAL(textChanged(const QString &)), SLOT(update_mw_lbl(const QString &)));
	connect(data_control, SIGNAL(mwChanged(float)), SLOT(update_mw_lbl(float)));
	
	ypos += height + spacing;
	xpos = 5;	

	lbl_temperature = new QLabel(tr("Temperature (ºC):"),this);
	lbl_temperature->setAlignment(AlignCenter|AlignVCenter);
	lbl_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_temperature->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_temperature->setGeometry(xpos, ypos, width, height);

	xpos += width + spacing;

//
// Normally, when using class Data_Control_W, the temperature is derived from the average 
// temperature of the run that is being analyzed. This variable is stored in the structure member
// run_inf.avg_temperature. Rather than inventing a new variable, this one is used here, since it
// is also used in calculation inside the class Data_Control_W, so we better stick with it:
//
	str.sprintf("%6.2f", data_control->run_inf.avg_temperature);
	le_temperature = new QLineEdit(this, "Temperature");
	le_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_temperature->setGeometry(xpos, ypos, (unsigned int) width/2, height);
	le_temperature->setText(str);
	le_temperature->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_temperature, SIGNAL(textChanged(const QString &)), SLOT(update_temperature_lbl(const QString &)));

	ypos += height + spacing;
	xpos = 5;	

	lbl_ratio = new QLabel(tr("Axial Ratio:"), this);
	lbl_ratio->setAlignment(AlignCenter|AlignVCenter);
	lbl_ratio->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_ratio->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_ratio->setGeometry(xpos, ypos, width, height);

	xpos += width + spacing;

	ratio = 10.0;
	str.sprintf("%4.1f", ratio);
	le_ratio = new QLineEdit(this, "Temperature");
	le_ratio->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_ratio->setGeometry(xpos, ypos, (unsigned int) width/2, height);
	le_ratio->setText(str);
	le_ratio->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_ratio, SIGNAL(textChanged(const QString &)), SLOT(update_ratio_lbl(const QString &)));

	ypos += height + spacing;
	xpos = 5;	

	cnt_ratio = new QwtCounter(this);
	cnt_ratio->setRange(1.1, (ARRAYSIZE+1)/10, 0.1);
	cnt_ratio->setValue(10);
	cnt_ratio->setNumButtons(3);
	cnt_ratio->setGeometry(xpos, ypos, (unsigned int) (1.5 * width) + spacing, height);
	cnt_ratio->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
//	connect(cnt_ratio, SIGNAL(buttonReleased(double)), SLOT(update_ratio_lbl(double)));
	connect(cnt_ratio, SIGNAL(valueChanged(double)), SLOT(update_ratio_lbl(double)));
	
	ypos += height + 5;
	xpos = 5;	

	lbl_text = new QLabel(tr("Please select an axial ratio..."), this);
	lbl_text->setAlignment(AlignCenter|AlignVCenter);
	lbl_text->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_text->setFrameStyle(QFrame::Panel|QFrame::Sunken);
	lbl_text->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	lbl_text->setGeometry(xpos, ypos, (unsigned int) (1.5 * width) + spacing, 90);

	ypos += 90 + 5;
	xpos = 5;

	pb_close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_close);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setGeometry(xpos, ypos, (unsigned int) (1.5*width/2), height);
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

	xpos +=  (unsigned int) (1.5*width/2) + spacing;

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, (unsigned int) (1.5*width/2), height);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	ypos += height + 5;
	xpos = 5;

	QString label[8];
	label[0] = tr("Model:");
	label[1] = "s (sec)";
	label[2] = "D (cm^2/sec)";
	label[3] = "f";
	label[4] = "f/f0";
	label[5] = "a (Å)";
	label[6] = "b (Å)";
	label[7] = tr("Volume (Å^3)");
	for (i=0; i<8; i++)
	{
		lbl_header[i] = new QLabel(label[i], this);
		lbl_header[i]->setAlignment(AlignCenter|AlignVCenter);
		lbl_header[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
		lbl_header[i]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
		lbl_header[i]->setGeometry(xpos, ypos, (unsigned int) (2 * width/3), height);
		xpos += spacing + (unsigned int) (2 * width/3);
	}

	ypos += height + spacing;
	xpos = 5;	

	lbl_sphere[0] = new QLabel(tr("Sphere:"), this);
	lbl_sphere[0]->setAlignment(AlignCenter|AlignVCenter);
	lbl_sphere[0]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_sphere[0]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sphere[0]->setGeometry(xpos, ypos, (unsigned int) (2 * width/3), height);

	for (i=1; i<8; i++)
	{
		xpos += spacing + (unsigned int) (2 * width/3);
		lbl_sphere[i] = new QLabel("", this);
		lbl_sphere[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		lbl_sphere[i]->setAlignment(AlignCenter|AlignVCenter);
		lbl_sphere[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
		lbl_sphere[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		lbl_sphere[i]->setGeometry(xpos, ypos, (unsigned int) (2 * width/3), height);
	}

	ypos += height + spacing;
	xpos = 5;	

	lbl_prolate[0] = new QLabel(tr("Prolate:"), this);
	lbl_prolate[0]->setAlignment(AlignCenter|AlignVCenter);
	lbl_prolate[0]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_prolate[0]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_prolate[0]->setGeometry(xpos, ypos, (unsigned int) (2 * width/3), height);

	for (i=1; i<8; i++)
	{
		xpos += spacing + (unsigned int) (2 * width/3);
		lbl_prolate[i] = new QLabel("", this);
		lbl_prolate[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		lbl_prolate[i]->setAlignment(AlignCenter|AlignVCenter);
		lbl_prolate[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
		lbl_prolate[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		lbl_prolate[i]->setGeometry(xpos, ypos, (unsigned int) (2 * width/3), height);
	}

	ypos += height + spacing;
	xpos = 5;	

	lbl_oblate[0] = new QLabel(tr("Oblate:"), this);
	lbl_oblate[0]->setAlignment(AlignCenter|AlignVCenter);
	lbl_oblate[0]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_oblate[0]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_oblate[0]->setGeometry(xpos, ypos, (unsigned int) (2 * width/3), height);

	for (i=1; i<8; i++)
	{
		xpos += spacing + (unsigned int) (2 * width/3);
		lbl_oblate[i] = new QLabel("", this);
		lbl_oblate[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		lbl_oblate[i]->setAlignment(AlignCenter|AlignVCenter);
		lbl_oblate[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
		lbl_oblate[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		lbl_oblate[i]->setGeometry(xpos, ypos, (unsigned int) (2 * width/3), height);
	}

	ypos += height + spacing;
	xpos = 5;	

	lbl_rod[0] = new QLabel(tr("Long Rod:"), this);
	lbl_rod[0]->setAlignment(AlignCenter|AlignVCenter);
	lbl_rod[0]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_rod[0]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rod[0]->setGeometry(xpos, ypos, (unsigned int) (2 * width/3), height);

	for (i=1; i<8; i++)
	{
		xpos += spacing + (unsigned int) (2 * width/3);
		lbl_rod[i] = new QLabel("", this);
		lbl_rod[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		lbl_rod[i]->setAlignment(AlignCenter|AlignVCenter);
		lbl_rod[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
		lbl_rod[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		lbl_rod[i]->setGeometry(xpos, ypos, (unsigned int) (2 * width/3), height);
	}


	plt->replot();

	global_Xpos += 30;
	global_Ypos += 30;
	
	move(global_Xpos, global_Ypos);
	update();
	setup_GUI();
}

void US_Hydro1::setup_GUI()
{
	int j=0;
	int rows = 8, columns = 2, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,2,2,spacing);
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}

	subGrid1->addWidget(pb_density,j,0);
	subGrid1->addWidget(le_density,j,1);
	j++;	
	subGrid1->addWidget(pb_viscosity,j,0);
	subGrid1->addWidget(le_viscosity,j,1);
	j++;
	subGrid1->addWidget(pb_vbar,j,0);
	subGrid1->addWidget(le_vbar,j,1);
	j++;
	subGrid1->addWidget(lbl_mw,j,0);
	subGrid1->addWidget(le_mw,j,1);
	j++;
	subGrid1->addWidget(lbl_temperature,j,0);
	subGrid1->addWidget(le_temperature,j,1);
	j++;
	subGrid1->addWidget(lbl_ratio,j,0);
	subGrid1->addWidget(le_ratio,j,1);
	j++;
	subGrid1->addMultiCellWidget(cnt_ratio,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(lbl_text,j,j,0,1);
	j++;
	subGrid1->addWidget(pb_close,j,0);
	subGrid1->addWidget(pb_help,j,1);
	
	j=0;
	rows = 5, columns = 8, spacing = 2;

	QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid2->setRowSpacing(i, 26);
	}
	for (int i=0; i<columns; i++)
	{
		subGrid2->addWidget(lbl_header[i],0,i);
		subGrid2->addWidget(lbl_sphere[i],1,i);
		subGrid2->addWidget(lbl_prolate[i],2,i);
		subGrid2->addWidget(lbl_oblate[i],3,i);
		subGrid2->addWidget(lbl_rod[i],4,i);
	}

	background->addLayout(subGrid1,0,0);
	background->addWidget(plt,0,1);
	background->addMultiCellLayout(subGrid2,1,1,0,1);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(1,640);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);
	
	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width()+600, r.height());

}

void US_Hydro1::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_Hydro1::update_buffer_lbl(float dens, float visc)
{
	QString str, str2;
	data_control->density = dens;
	data_control->viscosity = visc;
	str.sprintf("%6.4f", data_control->density);
	le_density->setText(str);
	str.sprintf("%6.4f", data_control->viscosity);
	le_viscosity->setText(str);
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	update();
}


void US_Hydro1::update_vbar_lbl(float vbar, float vbar20)
{
	QString str, str2;
	data_control->vbar = vbar;
	data_control->vbar20 = vbar20;
	str.sprintf("%6.4f", data_control->vbar20);
	le_vbar->setText(str);
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	update();
}

void US_Hydro1::update_temperature_lbl(const QString &str)
{
	data_control->run_inf.avg_temperature = str.toFloat();
	data_control->vbar = data_control->vbar20 + (4.25e-4 * (data_control->run_inf.avg_temperature - 20));
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	update();
}

void US_Hydro1::update_viscosity_lbl(const QString &str)
{
	data_control->viscosity = str.toFloat();
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	(*allparams).viscosity = data_control->viscosity;
	update();
}

void US_Hydro1::update_density_lbl(const QString &str)
{
	data_control->density = str.toFloat();
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	update();
}

void US_Hydro1::update_mw_lbl(const QString &str)
{
	mw = str.toFloat();
	update();
}
void US_Hydro1::update_mw_lbl(float val)
{
	QString str;
	mw = val;
	str.sprintf("%4.3e", mw);
	le_mw->setText(str);
	update();
}

void US_Hydro1::update_vbar_lbl(const QString &str)
{
	data_control->vbar20 = str.toFloat();
	data_control->vbar = data_control->vbar20 + (4.25e-4 * (data_control->run_inf.avg_temperature - 20));
	update();
}

void US_Hydro1::update_ratio_lbl(const QString &str)
{
	ratio = str.toFloat();
	if (ratio < 6.0 && ratio >= 1.1)
	{
		lbl_text->setText(tr("Attention:\n\nThe rod model is unreliable\nfor axial ratios less than 6.0"));
	}
	if (ratio < 1.1)
	{
		lbl_text->setText(tr("Attention:\n\nThe lower axial ratio limit is 1.1!"));
		ratio =  (float) 1.1;
	}
	if (ratio >= 6.0 && ratio <=100)
	{
		lbl_text->setText(tr("Please select an axial ratio..."));
	}
	if (ratio > 100)
	{
		lbl_text->setText(tr("Attention:\n\nThe upper axial ratio limit is 100!"));
		ratio = 100;
	}
	vline_x[0]=ratio;
	vline_x[1]=ratio;
	plt->removeCurve(cv_vline);
	cv_vline = plt->insertCurve("Axial Ratio");
	plt->setCurvePen(cv_vline, white);
	plt->setCurveData(cv_vline, vline_x, vline_y, 2);
	plt->replot();
	cnt_ratio->setValue(ratio);
	update();
}

void US_Hydro1::update_ratio_lbl(double val)
{
	QString str;
	ratio = (float) val;
	str.sprintf("%4.1f", ratio);
	le_ratio->setText(str);
	vline_x[0]=ratio;
	vline_x[1]=ratio;
	plt->removeCurve(cv_vline);
	cv_vline = plt->insertCurve("Axial Ratio");
	plt->setCurvePen(cv_vline, white);
	plt->setCurveData(cv_vline, vline_x, vline_y, 2);
	plt->replot();
	update();
}

void US_Hydro1::update()
{
	(*allparams).mw = mw;
	(*allparams).vbar = data_control->vbar;
	(*allparams).density = data_control->density;
	(*allparams).viscosity = data_control->viscosity;
	(*allparams).temperature = data_control->run_inf.avg_temperature;
	(*allparams).axial_ratio = ratio;
	int index = (int) ((ratio - 1.1) * 10.0 + 0.5);
	QString str;
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	float t = data_control->run_inf.avg_temperature + K0;
	float vol_per_molecule = data_control->vbar * mw / AVOGADRO;
	float rad_sphere = pow((double) (vol_per_molecule * (3.0/4.0))/M_PI, (double) (1.0/3.0));
	float f0 = rad_sphere * 6.0 * M_PI * data_control->viscosity_tb*0.01;
//
// recaluclate volume to put into cubic angstroms:
//
	vol_per_molecule = (4.0 / 3.0) * M_PI * pow((double) rad_sphere * 1.0e+08, (double) 3.0);
//
// prolate ellipsoid, ratio = ap/bp  (a = semi-major axis)
//
	float ap = 1.0e+08 * (rad_sphere * pow((double) ratio, (double) (2.0/3.0)));
	float bp = ap/ratio;
	float fp = prolate[index] * f0;
	float sp = (mw * data_control->buoyancyb)/(AVOGADRO * fp);
	float Dp = (sp * R * t) / (mw * data_control->buoyancyb);
	lbl_prolate[1]->setText(str.sprintf("%6.4e", sp));
	lbl_prolate[2]->setText(str.sprintf("%6.4e", Dp));
	lbl_prolate[3]->setText(str.sprintf("%6.4e", fp));
	lbl_prolate[4]->setText(str.sprintf("%6.4e", prolate[index]));
	lbl_prolate[5]->setText(str.sprintf("%6.4e", ap));
	lbl_prolate[6]->setText(str.sprintf("%6.4e", bp));
	lbl_prolate[7]->setText(str.sprintf("%6.4e", vol_per_molecule));
	(*allparams).prolate.sedcoeff = sp;
	(*allparams).prolate.diffcoeff = Dp;
	(*allparams).prolate.f = fp;
	(*allparams).prolate.f_f0 = prolate[index];
	(*allparams).prolate.a = ap;
	(*allparams).prolate.b = bp;
	(*allparams).prolate.volume = vol_per_molecule;
	
//
// oblate ellipsoid:
//
	float bo = 1.0e+08 * (rad_sphere/pow((double) ratio, (double) (2.0/3.0)));
	float ao = ratio * bo;
	float fo = oblate[index] * f0;
	float so = (mw * data_control->buoyancyb)/(AVOGADRO * fo);
	float Do = (so * R * t) / (mw * data_control->buoyancyb);
	lbl_oblate[1]->setText(str.sprintf("%6.4e", so));
	lbl_oblate[2]->setText(str.sprintf("%6.4e", Do));
	lbl_oblate[3]->setText(str.sprintf("%6.4e", fo));
	lbl_oblate[4]->setText(str.sprintf("%6.4e", oblate[index]));
	lbl_oblate[5]->setText(str.sprintf("%6.4e", ao));
	lbl_oblate[6]->setText(str.sprintf("%6.4e", bo));
	lbl_oblate[7]->setText(str.sprintf("%6.4e", vol_per_molecule));
	(*allparams).oblate.sedcoeff = so;
	(*allparams).oblate.diffcoeff = Do;
	(*allparams).oblate.f = fo;
	(*allparams).oblate.f_f0 = oblate[index];
	(*allparams).oblate.a = ao;
	(*allparams).oblate.b = bo;
	(*allparams).oblate.volume = vol_per_molecule;


//
// long rod:
//
	
	float br = 1.0e+08 * (pow((double) (2.0/(3.0*ratio)), (double) (1.0/3.0)) * rad_sphere);
	float ar = ratio * br;
	float fr = rod[index] * f0;
	float sr = (mw * data_control->buoyancyb)/(AVOGADRO * fr);
	float Dr = (sr * R * t) / (mw * data_control->buoyancyb);
	lbl_rod[1]->setText(str.sprintf("%6.4e", sr));
	lbl_rod[2]->setText(str.sprintf("%6.4e", Dr));
	lbl_rod[3]->setText(str.sprintf("%6.4e", fr));
	lbl_rod[4]->setText(str.sprintf("%6.4e", rod[index]));
	lbl_rod[5]->setText(str.sprintf("%6.4e", ar));
	lbl_rod[6]->setText(str.sprintf("%6.4e", br));
	lbl_rod[7]->setText(str.sprintf("%6.4e", vol_per_molecule));
	(*allparams).rod.sedcoeff = sr;
	(*allparams).rod.diffcoeff = Dr;
	(*allparams).rod.f = fr;
	(*allparams).rod.f_f0 = rod[index];
	(*allparams).rod.a = ar;
	(*allparams).rod.b = br;
	(*allparams).rod.volume = vol_per_molecule;


//
// Sphere:
//	

	float ss = (mw * data_control->buoyancyb) / (AVOGADRO * f0);
	float Ds = (ss * R * t) / (mw * data_control->buoyancyb);
	lbl_sphere[1]->setText(str.sprintf("%6.4e", ss));
	lbl_sphere[2]->setText(str.sprintf("%6.4e", Ds));
	lbl_sphere[3]->setText(str.sprintf("%6.4e", f0));
	lbl_sphere[4]->setText(str.sprintf("%6.4e", 1.0));
	lbl_sphere[5]->setText(str.sprintf("%6.4e", 1.0e+08 * rad_sphere));
	lbl_sphere[6]->setText(str.sprintf("%6.4e", 1.0e+08 * rad_sphere));
	lbl_sphere[7]->setText(str.sprintf("%6.4e", vol_per_molecule));
	(*allparams).sphere.sedcoeff = ss;
	(*allparams).sphere.diffcoeff = Ds;
	(*allparams).sphere.f_f0 = f0;
	(*allparams).sphere.f_f0 = 1.0;
	(*allparams).sphere.a = 1.0e+08 * rad_sphere;
	(*allparams).sphere.b = 1.0e+08 * rad_sphere;
	(*allparams).sphere.volume = vol_per_molecule;
	emit updated();
}

void US_Hydro1::update(struct hydrosim *temp_allparams)
{
	(*temp_allparams).mw = mw;
	(*temp_allparams).vbar = data_control->vbar;
	(*temp_allparams).density = data_control->density;
	(*temp_allparams).viscosity = data_control->viscosity;
	(*temp_allparams).temperature = data_control->run_inf.avg_temperature;
	(*temp_allparams).axial_ratio = ratio;
	int index = (int) ((ratio - 1.1) * 10.0 + 0.5);
	QString str;
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	float t = data_control->run_inf.avg_temperature + K0;
	float vol_per_molecule = data_control->vbar * mw / AVOGADRO;
	float rad_sphere = pow((double) (vol_per_molecule * (3.0/4.0))/M_PI, (double) (1.0/3.0));
	float f0 = rad_sphere * 6.0 * M_PI * data_control->viscosity_tb*0.01;
//
// recaluclate volume to put into cubic angstroms:
//
	vol_per_molecule = (4.0 / 3.0) * M_PI * pow((double) rad_sphere * 1.0e+08, (double) 3.0);
//
// prolate ellipsoid, ratio = ap/bp  (a = semi-major axis)
//

	float ap = 1.0e+08 * (rad_sphere * pow((double) ratio, (double) (2.0/3.0)));
	float bp = ap/ratio;
	float fp = prolate[index] * f0;
	float sp = (mw * data_control->buoyancyb)/(AVOGADRO * fp);
	float Dp = (sp * R * t) / (mw * data_control->buoyancyb);
	lbl_prolate[1]->setText(str.sprintf("%6.4e", sp));
	lbl_prolate[2]->setText(str.sprintf("%6.4e", Dp));
	lbl_prolate[3]->setText(str.sprintf("%6.4e", fp));
	lbl_prolate[4]->setText(str.sprintf("%6.4e", prolate[index]));
	lbl_prolate[5]->setText(str.sprintf("%6.4e", ap));
	lbl_prolate[6]->setText(str.sprintf("%6.4e", bp));
	lbl_prolate[7]->setText(str.sprintf("%6.4e", vol_per_molecule));
	(*temp_allparams).prolate.sedcoeff = sp;
	(*temp_allparams).prolate.diffcoeff = Dp;
	(*temp_allparams).prolate.f = fp;
	(*temp_allparams).prolate.f_f0 = prolate[index];
	(*temp_allparams).prolate.a = ap;
	(*temp_allparams).prolate.b = bp;
	(*temp_allparams).prolate.volume = vol_per_molecule;
	
//
// oblate ellipsoid:
//
	float bo = 1.0e+08 * (rad_sphere/pow((double) ratio, (double) (2.0/3.0)));
	float ao = ratio * bo;
	float fo = oblate[index] * f0;
	float so = (mw * data_control->buoyancyb)/(AVOGADRO * fo);
	float Do = (so * R * t) / (mw * data_control->buoyancyb);
	lbl_oblate[1]->setText(str.sprintf("%6.4e", so));
	lbl_oblate[2]->setText(str.sprintf("%6.4e", Do));
	lbl_oblate[3]->setText(str.sprintf("%6.4e", fo));
	lbl_oblate[4]->setText(str.sprintf("%6.4e", oblate[index]));
	lbl_oblate[5]->setText(str.sprintf("%6.4e", ao));
	lbl_oblate[6]->setText(str.sprintf("%6.4e", bo));
	lbl_oblate[7]->setText(str.sprintf("%6.4e", vol_per_molecule));
	(*temp_allparams).oblate.sedcoeff = so;
	(*temp_allparams).oblate.diffcoeff = Do;
	(*temp_allparams).oblate.f = fo;
	(*temp_allparams).oblate.f_f0 = oblate[index];
	(*temp_allparams).oblate.a = ao;
	(*temp_allparams).oblate.b = bo;
	(*temp_allparams).oblate.volume = vol_per_molecule;


//
// long rod:
//
	
	float br = 1.0e+08 * (pow((double) (2.0/(3.0*ratio)), (double) (1.0/3.0)) * rad_sphere);
	float ar = ratio * br;
	float fr = rod[index] * f0;
	float sr = (mw * data_control->buoyancyb)/(AVOGADRO * fr);
	float Dr = (sr * R * t) / (mw * data_control->buoyancyb);
	lbl_rod[1]->setText(str.sprintf("%6.4e", sr));
	lbl_rod[2]->setText(str.sprintf("%6.4e", Dr));
	lbl_rod[3]->setText(str.sprintf("%6.4e", fr));
	lbl_rod[4]->setText(str.sprintf("%6.4e", rod[index]));
	lbl_rod[5]->setText(str.sprintf("%6.4e", ar));
	lbl_rod[6]->setText(str.sprintf("%6.4e", br));
	lbl_rod[7]->setText(str.sprintf("%6.4e", vol_per_molecule));
	(*temp_allparams).rod.sedcoeff = sr;
	(*temp_allparams).rod.diffcoeff = Dr;
	(*temp_allparams).rod.f = fr;
	(*temp_allparams).rod.f_f0 = rod[index];
	(*temp_allparams).rod.a = ar;
	(*temp_allparams).rod.b = br;
	(*temp_allparams).rod.volume = vol_per_molecule;


//
// Sphere:
//	

	float ss = (mw * data_control->buoyancyb) / (AVOGADRO * f0);
	float Ds = (ss * R * t) / (mw * data_control->buoyancyb);
	lbl_sphere[1]->setText(str.sprintf("%6.4e", ss));
	lbl_sphere[2]->setText(str.sprintf("%6.4e", Ds));
	lbl_sphere[3]->setText(str.sprintf("%6.4e", f0));
	lbl_sphere[4]->setText(str.sprintf("%6.4e", 1.0));
	lbl_sphere[5]->setText(str.sprintf("%6.4e", 1.0e+08 * rad_sphere));
	lbl_sphere[6]->setText(str.sprintf("%6.4e", 1.0e+08 * rad_sphere));
	lbl_sphere[7]->setText(str.sprintf("%6.4e", vol_per_molecule));
	(*temp_allparams).sphere.sedcoeff = ss;
	(*temp_allparams).sphere.diffcoeff = Ds;
	(*temp_allparams).sphere.f_f0 = f0;
	(*temp_allparams).sphere.f_f0 = 1.0;
	(*temp_allparams).sphere.a = 1.0e+08 * rad_sphere;
	(*temp_allparams).sphere.b = 1.0e+08 * rad_sphere;
	(*temp_allparams).sphere.volume = vol_per_molecule;
	emit updated();
}

double US_Hydro1::calc_invtangent(double x)
{
	double tol = 1e-8;
	double i = 0.0;
	double lower = 0.0;
	double upper = M_PI/2;
	double func = 0;
	while (fabs(x - func) >= tol)
	{
		i += (upper - lower) / 10.0;
		func = tan(i);
		if (x < func)
		{
			lower = i - (upper - lower) / 10.0;
			upper = i;
			i = lower;
		}
	}
	return(i);
}

void US_Hydro1::calc_models()
{
	double x = 1.1;
	int i;
	for(i=0; i< ARRAYSIZE; i++)
	{
		prolate[i] = (pow((double) x, (double) (-1.0/3.0)) * sqrt(pow((double) x, (double) 2.0) - 1.0)) / log(x + sqrt(pow((double) x, (double) 2.0) - 1.0));
		oblate[i]  = sqrt(pow((double) x, (double) 2.0) - 1.0) / (pow((double) x, (double) (2.0/3.0)) * calc_invtangent(sqrt(pow((double) x, (double) 2.0) - 1.0)));
		rod[i]     = (pow((double) (2.0/3.0), (double) (1.0/3.0)) * pow((double) x, (double) (2.0/3.0))) / (log(2.0 * x) - 0.3);
		ratio_x[i] = x;
		x += 0.10000;
	}
}

void US_Hydro1::quit()
{
	update();
	close();
}

void US_Hydro1::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/predict1.html");
}
/*
void US_Hydro1::resizeEvent(QResizeEvent *e)
{
	int dialogw = (unsigned int) (1.5*width) + 10 + spacing;
	plt->setGeometry(dialogw, 5, e->size().width()-dialogw-5, e->size().height() - 15 - 4 * spacing - 5 * height);
}
*/
US_Hydro2::US_Hydro2(QWidget *p, const char* name) : QFrame(p, name)
{
	USglobal = new US_Config();
	int i;	
	xpos = 5;
	ypos = 5;
	width = 140;
	height = 26;
	QString str;
	flag = 0;
	sed_coeff = 0.0;  
	diff_coeff = 0.0;
	mw = 0.0;
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
//	setMinimumSize((unsigned int) (480 + 4 * spacing + 15 + 1.5 * width), (unsigned int) (10 * height + 7 * spacing + 20));	
//	setMaximumSize((unsigned int) (480 + 4 * spacing + 15 + 1.5 * width), (unsigned int) (10 * height + 7 * spacing + 20));	

	data_control = new Data_Control_W(0);
	data_control->step=1;

	data_control->density =  (float) DENS_20W;
	pb_density = new QPushButton(tr("Density:"), this);
	Q_CHECK_PTR(pb_density);
	pb_density->setAutoDefault(false);
	pb_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_density->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_density->setGeometry(xpos, ypos, width, height);
	connect(pb_density, SIGNAL(clicked()), data_control, SLOT(get_buffer()));
	
	connect(data_control, SIGNAL(bufferChanged(float, float)), SLOT(update_buffer_lbl(float, float)));

	xpos += width + spacing;

	le_density = new QLineEdit(this);
	le_density->setGeometry(xpos, ypos, (unsigned int) width/2, height);
	le_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_density->setText( str.sprintf( "%8.6f", DENS_20W ) );
	le_density->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_density, SIGNAL(textChanged(const QString &)), SLOT(update_density_lbl(const QString &)));

	ypos += height + spacing;
	xpos = 5;	

	data_control->viscosity =  (float) (100.0 * VISC_20W);
	pb_viscosity = new QPushButton(tr("Viscosity:"), this);
	Q_CHECK_PTR(pb_viscosity);
	pb_viscosity->setAutoDefault(false);
	pb_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_viscosity->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_viscosity->setGeometry(xpos, ypos, width, height);
	connect(pb_viscosity, SIGNAL(clicked()), data_control, SLOT(get_buffer()));
		
	xpos += width + spacing;

	le_viscosity = new QLineEdit(this);
	le_viscosity->setGeometry(xpos, ypos, (unsigned int) width/2, height);
	le_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_viscosity->setText(" (100.0 * VISC_20W)");
	le_viscosity->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_viscosity, SIGNAL(textChanged(const QString &)), SLOT(update_viscosity_lbl(const QString &)));

	ypos += height + spacing;
	xpos = 5;	

	pb_vbar = new QPushButton(tr(" vbar(20º):"), this);
	Q_CHECK_PTR(pb_vbar);
	pb_vbar->setAutoDefault(false);
	pb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_vbar->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_vbar->setGeometry(xpos, ypos, width, height);
	connect(pb_vbar, SIGNAL(clicked()), data_control, SLOT(read_vbar()));
	xpos += width + spacing;

	data_control->vbar20 =  (float) 0.72;
	le_vbar = new QLineEdit(this);
	le_vbar->setGeometry(xpos, ypos, (unsigned int) width/2, height);
	le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_vbar->setText(" .7200");
	le_vbar->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_vbar_lbl(const QString &)));
	connect(data_control, SIGNAL(vbarChanged(float, float)), SLOT(update_vbar_lbl(float, float)));

	ypos += height + spacing;
	xpos = 5;	

	lbl_temperature = new QLabel(tr("Temperature (ºC):"),this);
	lbl_temperature->setAlignment(AlignCenter|AlignVCenter);
	lbl_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_temperature->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_temperature->setGeometry(xpos, ypos, width, height);

	xpos += width + spacing;

//
// Normally, when using class Data_Control_W, the temperature is derived from the average 
// temperature of the run that is being analyzed. This variable is stored in the structure member
// run_inf.avg_temperature. Rather than inventing a new variable, this one is used here, since it
// is also used in calculation inside the class Data_Control_W, so we better stick with it:
//
	data_control->run_inf.avg_temperature = 20;
	str.sprintf("%6.2f", data_control->run_inf.avg_temperature);
	le_temperature = new QLineEdit(this, "Temperature");
	le_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_temperature->setGeometry(xpos, ypos, (unsigned int) width/2, height);
	le_temperature->setText(str);
	le_temperature->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_temperature, SIGNAL(textChanged(const QString &)), SLOT(update_temperature_lbl(const QString &)));

	ypos += height + 5;
	xpos = 5;	

	lbl_text = new QLabel(tr("Select a Parameter Combination:"), this);
	lbl_text->setAlignment(AlignCenter|AlignVCenter);
	lbl_text->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_text->setFrameStyle(QFrame::Panel|QFrame::Sunken);
	lbl_text->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	lbl_text->setGeometry(xpos, ypos, (unsigned int) (1.5 * width) + spacing, height);

	ypos += height + spacing;
	xpos = 5;	

	lbl_param1 = new QLabel(tr("Parameter 1:"),this);
	lbl_param1->setAlignment(AlignCenter|AlignVCenter);
	lbl_param1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_param1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_param1->setGeometry(xpos, ypos, width, height);

	xpos += width + spacing;

	le_param1 = new QLineEdit(this, "Temperature");
	le_param1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_param1->setGeometry(xpos, ypos, (unsigned int) width/2, height);
	le_param1->setText("0.000");
	le_param1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_param1, SIGNAL(textChanged(const QString &)), SLOT(update_param1_lbl(const QString &)));

	ypos += height + spacing;
	xpos = 5;	

	lbl_param2 = new QLabel(tr("Parameter 2:"),this);
	lbl_param2->setAlignment(AlignCenter|AlignVCenter);
	lbl_param2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_param2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_param2->setGeometry(xpos, ypos, width, height);

	xpos += width + spacing;

	le_param2 = new QLineEdit(this, "Temperature");
	le_param2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_param2->setText("0.000");
	le_param2->setGeometry(xpos, ypos, (unsigned int) width/2, height);
	le_param2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_param2, SIGNAL(textChanged(const QString &)), SLOT(update_param2_lbl(const QString &)));

	xpos = 5;	
	ypos += height + 5;

	pb_mw_s = new QPushButton(tr("Mol. Weight + Sed. Coeff."), this);
	Q_CHECK_PTR(pb_mw_s);
	pb_mw_s->setAutoDefault(false);
	pb_mw_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_mw_s->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_mw_s->setGeometry(xpos, ypos,  (unsigned int) (1.5 * width) + spacing, height);
	connect(pb_mw_s, SIGNAL(clicked()), SLOT(do_mw_s()));

	ypos += height + spacing;

	pb_mw_d = new QPushButton(tr("Mol. Weight + Diff. Coeff."), this);
	Q_CHECK_PTR(pb_mw_d);
	pb_mw_d->setAutoDefault(false);
	pb_mw_d->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_mw_d->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_mw_d->setGeometry(xpos, ypos, (unsigned int) (1.5 * width) + spacing, height);
	connect(pb_mw_d, SIGNAL(clicked()), SLOT(do_mw_d()));

	ypos += height + spacing;

	pb_s_d = new QPushButton(tr("Sed. Coeff. + Diff. Coeff."), this);
	Q_CHECK_PTR(pb_s_d);
	pb_s_d->setAutoDefault(false);
	pb_s_d->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_s_d->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_s_d->setGeometry(xpos, ypos, (unsigned int) (1.5 * width) + spacing, height);
	connect(pb_s_d, SIGNAL(clicked()), SLOT(do_s_d()));

	ypos = 5;
	int newx = (int) (3.0 * width / 2.0) + spacing + 10;
	xpos = newx;
	width = 120;

	QString label[4];
	label[0] = tr("Model:");
	label[1] = "a (Å)";
	label[2] = "b (Å)";
	label[3] = "a/b";

	for (i=0; i<4; i++)
	{
		lbl_header[i] = new QLabel(label[i], this);
		lbl_header[i]->setAlignment(AlignCenter|AlignVCenter);
		lbl_header[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
		lbl_header[i]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
		lbl_header[i]->setGeometry(xpos, ypos, width, height);
		xpos += spacing + width;
	}
	
	xpos = newx;
	ypos += height + spacing;

	lbl_prolate[0] = new QLabel(tr("Prolate:"), this);
	lbl_prolate[0]->setAlignment(AlignCenter|AlignVCenter);
	lbl_prolate[0]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_prolate[0]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_prolate[0]->setGeometry(xpos, ypos, width, height);

	for (i=1; i<4; i++)
	{
		xpos += spacing + width;
		lbl_prolate[i] = new QLabel("", this);
		lbl_prolate[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		lbl_prolate[i]->setAlignment(AlignCenter|AlignVCenter);
		lbl_prolate[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
		lbl_prolate[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		lbl_prolate[i]->setGeometry(xpos, ypos, width, height);
	}

	ypos += height + spacing;
	xpos = newx;	

	lbl_oblate[0] = new QLabel(tr("Oblate:"), this);
	lbl_oblate[0]->setAlignment(AlignCenter|AlignVCenter);
	lbl_oblate[0]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_oblate[0]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_oblate[0]->setGeometry(xpos, ypos, width, height);

	for (i=1; i<4; i++)
	{
		xpos += spacing + width;
		lbl_oblate[i] = new QLabel("", this);
		lbl_oblate[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		lbl_oblate[i]->setAlignment(AlignCenter|AlignVCenter);
		lbl_oblate[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
		lbl_oblate[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		lbl_oblate[i]->setGeometry(xpos, ypos, width, height);
	}

	ypos += height + spacing;
	xpos = newx;	

	lbl_rod[0] = new QLabel(tr("Long Rod:"), this);
	lbl_rod[0]->setAlignment(AlignCenter|AlignVCenter);
	lbl_rod[0]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_rod[0]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rod[0]->setGeometry(xpos, ypos, width, height);

	for (i=1; i<4; i++)
	{
		xpos += spacing + width;
		lbl_rod[i] = new QLabel("", this);
		lbl_rod[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		lbl_rod[i]->setAlignment(AlignCenter|AlignVCenter);
		lbl_rod[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
		lbl_rod[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		lbl_rod[i]->setGeometry(xpos, ypos, width, height);
	}

	ypos += height + 5;
	xpos = newx;	

	for (i=0; i<4; i++)
	{
		lbl_line1[i] = new QLabel("", this);
		lbl_line1[i]->setAlignment(AlignCenter|AlignVCenter);
		if (i == 0 || i == 2)
		{
			lbl_line1[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
			lbl_line1[i]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
		}
		else
		{
			lbl_line1[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
			lbl_line1[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
			lbl_line1[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		}
		lbl_line1[i]->setGeometry(xpos, ypos, width, height);
		xpos += spacing + width;
	}
	lbl_line1[0]->setText(tr("Frict. Coefficient:"));
	lbl_line1[2]->setText(tr("R0 (Sphere):"));

	ypos += height + spacing;
	xpos = newx;	

	for (i=0; i<4; i++)
	{
		lbl_line2[i] = new QLabel("", this);
		lbl_line2[i]->setAlignment(AlignCenter|AlignVCenter);
		if (i == 0 || i == 2)
		{
			lbl_line2[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
			lbl_line2[i]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
		}
		else
		{
			lbl_line2[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
			lbl_line2[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
			lbl_line2[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		}
		lbl_line2[i]->setGeometry(xpos, ypos, width, height);
		xpos += spacing + width;
	}
	lbl_line2[0]->setText(tr("Parameter 3:"));
	lbl_line2[2]->setText(tr("f0 (Sphere):"));

	ypos += height + spacing;
	xpos = newx;	

	for (i=0; i<4; i++)
	{
		lbl_line3[i] = new QLabel("", this);
		lbl_line3[i]->setAlignment(AlignCenter|AlignVCenter);
		if (i == 0 || i == 2)
		{
			lbl_line3[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
			lbl_line3[i]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
		}
		else
		{
			lbl_line3[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
			lbl_line3[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
			lbl_line3[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		}
		lbl_line3[i]->setGeometry(xpos, ypos, width, height);
		xpos += spacing + width;
	}
	lbl_line3[0]->setText(tr("Volume (Å^3):"));
	lbl_line3[2]->setText("f/f0:");

	ypos += height + 5;
	xpos = newx;	

	pb_update = new QPushButton(tr("Update"), this);
	Q_CHECK_PTR(pb_update);
	pb_update->setAutoDefault(false);
	pb_update->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_update->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_update->setGeometry(xpos, ypos, width, height);
	connect(pb_update, SIGNAL(clicked()), SLOT(update()));

	xpos = newx;
	ypos += height + spacing;

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, width, height);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	xpos = newx;
	ypos += height + spacing;

	pb_close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_close);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setGeometry(xpos, ypos, width, height);
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

	data_control->vbar = data_control->vbar20 + (4.25e-4 * (data_control->run_inf.avg_temperature - 20));
	data_control->calc_correction(data_control->run_inf.avg_temperature);

	global_Xpos += 30;
	global_Ypos += 30;
	
	move(global_Xpos, global_Ypos);
	
	setup_GUI();
}

US_Hydro2::~US_Hydro2()
{
}

void US_Hydro2::setup_GUI()
{
	int j=0;
	int rows = 10, columns = 2, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,2,2,spacing*2);
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}

	subGrid1->addWidget(pb_density,j,0);
	subGrid1->addWidget(le_density,j,1);
	j++;	
	subGrid1->addWidget(pb_viscosity,j,0);
	subGrid1->addWidget(le_viscosity,j,1);
	j++;
	subGrid1->addWidget(pb_vbar,j,0);
	subGrid1->addWidget(le_vbar,j,1);
	j++;
	subGrid1->addWidget(lbl_temperature,j,0);
	subGrid1->addWidget(le_temperature,j,1);
	j++;
	subGrid1->addMultiCellWidget(lbl_text,j,j,0,1);
	j++;
	subGrid1->addWidget(lbl_param1,j,0);
	subGrid1->addWidget(le_param1,j,1);
	j++;
	subGrid1->addWidget(lbl_param2,j,0);
	subGrid1->addWidget(le_param2,j,1);
	j++;
	subGrid1->addMultiCellWidget(pb_mw_s,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_mw_d,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_s_d,j,j,0,1);

	
	j=0;
	rows = 10, columns = 4, spacing = 2;

	QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid2->setRowSpacing(i, 26);
	}
	for (int i=0; i<columns; i++)
	{
		subGrid2->addWidget(lbl_header[i],0,i);
		subGrid2->addWidget(lbl_prolate[i],1,i);
		subGrid2->addWidget(lbl_oblate[i],2,i);
		subGrid2->addWidget(lbl_rod[i],3,i);
		subGrid2->addWidget(lbl_line1[i],4,i);
		subGrid2->addWidget(lbl_line2[i],5,i);
		subGrid2->addWidget(lbl_line3[i],6,i);
	}
	j=j+7;
	subGrid2->addWidget(pb_update,j,0);
	j++;
	subGrid2->addWidget(pb_help,j,0);
	j++;
	subGrid2->addWidget(pb_close,j,0);
	
	background->addLayout(subGrid1,0,0);
	background->addLayout(subGrid2,0,1);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(1,600);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height());

}



void US_Hydro2::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_Hydro2::do_mw_s()
{
	flag = 1;
	le_param1->setText("0.000");
	le_param2->setText("0.000");
	sed_coeff = 0.0;
	mw = 0.0;
	lbl_param1->setText(tr("Molecular Weight:"));
	lbl_param2->setText(tr("Sed. Coefficient:"));
	lbl_line2[0]->setText(tr("Diff. Coefficient:"));
}

void US_Hydro2::do_mw_d()
{
	flag = 2;
	le_param1->setText("0.000");
	le_param2->setText("0.000");
	diff_coeff = 0.0;
	mw = 0.0;
	lbl_param1->setText(tr("Molecular Weight:"));
	lbl_param2->setText(tr("Diff. Coefficient:"));
	lbl_line2[0]->setText(tr("Sed. Coefficient:"));
}

void US_Hydro2::do_s_d()
{
	flag = 3;
	le_param1->setText("0.000");
	le_param2->setText("0.000");
	sed_coeff = 0.0;
	diff_coeff = 0.0;
	lbl_param1->setText(tr("Sed. Coefficient:"));
	lbl_param2->setText(tr("Diff. Coefficient:"));
	lbl_line2[0]->setText(tr("Molecular Weight:"));
}

void US_Hydro2::update_param1_lbl(const QString &str)
{
	if (flag == 3)
	{
		sed_coeff = str.toFloat();
	}
	else
	{
		mw = str.toFloat();
	}
}

void US_Hydro2::update_param2_lbl(const QString &str)
{
	if (flag == 1)
	{
		sed_coeff = str.toFloat();
	}
	else
	{
		diff_coeff = str.toFloat();
	}
}

void US_Hydro2::update_buffer_lbl(float dens, float visc)
{
	QString str, str2;
	data_control->density = dens;
	data_control->viscosity = visc;
	str.sprintf("%6.4f", data_control->density);
	le_density->setText(str);
	str.sprintf("%6.4f", data_control->viscosity);
	le_viscosity->setText(str);
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	update();
}

void US_Hydro2::update_vbar_lbl(float vbar, float vbar20)
{
	QString str, str2;
	data_control->vbar = vbar;
	data_control->vbar20 = vbar20;
	str.sprintf("%6.4f", data_control->vbar20);
	le_vbar->setText(str);
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	update();
}

void US_Hydro2::update_temperature_lbl(const QString &str)
{
	data_control->run_inf.avg_temperature = str.toFloat();
	data_control->vbar = data_control->vbar20 + (4.25e-4 * (data_control->run_inf.avg_temperature - 20));
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	update();
}

void US_Hydro2::update_viscosity_lbl(const QString &str)
{
	data_control->viscosity = str.toFloat();
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	update();
}

void US_Hydro2::update_density_lbl(const QString &str)
{
	data_control->density = str.toFloat();
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	update();
}

void US_Hydro2::update_vbar_lbl(const QString &str)
{
	data_control->vbar20 = str.toFloat();
	data_control->vbar = data_control->vbar20 + (4.25e-4 * (data_control->run_inf.avg_temperature - 20));
	data_control->calc_correction(data_control->run_inf.avg_temperature);
	update();
}

void US_Hydro2::update()
{
	QString str;
	float t, vol_per_molecule = 0.0, rad_sphere = 0.0, f0 = 0.0, frict_coeff = 0.0, temp=0;
	t = data_control->run_inf.avg_temperature + K0;

	if (flag == 0)
	{
		QMessageBox::message(tr("Please Note:"), tr("You have to define a\nparameter combination first!"));
		return;
	}
	if (flag == 1)
	{
		if (mw == 0)
		{
			QMessageBox::message(tr("Please Note:"), tr("Please define the Molecular Weight first!"));
			return;
		}
		if (sed_coeff == 0)
		{
			QMessageBox::message(tr("Please Note:"), tr("Please define a Sedimentation Coefficient first!"));
			return;
		}
		vol_per_molecule = data_control->vbar * mw / AVOGADRO;
		rad_sphere = pow((double) (vol_per_molecule * (3.0/4.0))/M_PI, (double) (1.0/3.0));
		f0 = rad_sphere * 6.0 * M_PI * data_control->viscosity_tb*0.01;
//
// recaluclate volume to put into cubic angstroms:
//
		vol_per_molecule = (4.0 / 3.0) * M_PI * pow((double) (rad_sphere * 1.0e+08), (double) 3.0);
		diff_coeff = (sed_coeff * R * t) / (data_control->buoyancyb * mw);
		frict_coeff = (mw * data_control->buoyancyb) / (sed_coeff * AVOGADRO);
		if (!check_valid(frict_coeff, f0))
		{
			return;
		}
		lbl_line2[1]->setText(str.sprintf("%6.4e", diff_coeff));
	}
	if (flag == 2)
	{
		if (mw == 0)
		{
			QMessageBox::message(tr("Please Note:"), tr("Please define the Molecular Weight first!"));
			return;
		}
		if (diff_coeff == 0)
		{
			QMessageBox::message(tr("Please Note:"), tr("Please define a Diffusion Coefficient first!"));
			return;
		}
		vol_per_molecule = data_control->vbar * mw / AVOGADRO;
		rad_sphere = pow((double) (vol_per_molecule * (3.0/4.0))/M_PI, (double) (1.0/3.0));
		f0 = rad_sphere * 6.0 * M_PI * data_control->viscosity_tb*0.01;
//
// recalculate volume to put into cubic angstroms:
//
		vol_per_molecule = (4.0 / 3.0) * M_PI * pow((double) (rad_sphere * 1.0e+08), (double) 3.0);
		sed_coeff = (diff_coeff * data_control->buoyancyb * mw) / (R * t);
		frict_coeff = (mw * data_control->buoyancyb) / (sed_coeff * AVOGADRO);
		if (!check_valid(frict_coeff, f0))
		{
			return;
		}
		lbl_line2[1]->setText(str.sprintf("%6.4e", sed_coeff));
	}

	if (flag == 3)
	{
		if (sed_coeff == 0)
		{
			QMessageBox::message(tr("Please Note:"), tr("Please define the Sedimentation Coefficient first!"));
			return;
		}
		if (diff_coeff == 0)
		{
			QMessageBox::message(tr("Please Note:"), tr("Please define a Diffusion Coefficient first!"));
			return;
		}
		mw = (sed_coeff * R * t) / (diff_coeff * data_control->buoyancyb);
		vol_per_molecule = data_control->vbar * mw / AVOGADRO;
		rad_sphere = pow((double) (vol_per_molecule * (3.0/4.0))/M_PI, (double) (1.0/3.0));
		f0 = rad_sphere * 6.0 * M_PI * data_control->viscosity_tb*0.01;
//
// recaluclate volume to put into cubic angstroms:
//
		vol_per_molecule = (4.0 / 3.0) * M_PI * pow((double) (rad_sphere * 1.0e+08), (double) 3.0);
		frict_coeff = (mw * data_control->buoyancyb) / (sed_coeff * AVOGADRO);
		if (!check_valid(frict_coeff, f0))
		{
			return;
		}
		lbl_line2[1]->setText(str.sprintf("%6.4e", mw));
	}
	lbl_line1[1]->setText(str.sprintf("%6.4e", frict_coeff));
	lbl_line1[3]->setText(str.sprintf("%6.4e", rad_sphere * 1.0e+08));
	lbl_line2[3]->setText(str.sprintf("%6.4e", f0));
	lbl_line3[1]->setText(str.sprintf("%6.4e", vol_per_molecule));
	lbl_line3[3]->setText(str.sprintf("%6.4e", frict_coeff/f0));
//
// prolate ellipsoid: model = 1
//
	temp = frict_coeff/f0;
	ratio = root(1, temp);
	float ap = 1.0e+08 * (rad_sphere * pow((double) ratio, (double) (2.0/3.0)));
	float bp = ap/ratio;

	lbl_prolate[1]->setText(str.sprintf("%6.4e", ap));
	lbl_prolate[2]->setText(str.sprintf("%6.4e", bp));
	lbl_prolate[3]->setText(str.sprintf("%6.4e", ratio));

//
// oblate ellipsoid: model = 2
//
	ratio = root(2, temp);
	float bo = 1.0e+08 * (rad_sphere / pow((double) ratio, (double) (2.0/3.0)));
	float ao = ratio * bo;
	lbl_oblate[1]->setText(str.sprintf("%6.4e", ao));
	lbl_oblate[2]->setText(str.sprintf("%6.4e", bo));
	lbl_oblate[3]->setText(str.sprintf("%6.4e", ratio));

//
// long rod: model = 3
//
	if(temp > 1.32)
	{
		ratio = root(3, temp);
		float br = 1.0e+08 * (pow((double) (2.0/(3.0*ratio)), (double) (1.0/3.0)) * rad_sphere);
		float ar = ratio * br;
		lbl_rod[1]->setText(str.sprintf("%6.4e", ar));
		lbl_rod[2]->setText(str.sprintf("%6.4e", br));
		lbl_rod[3]->setText(str.sprintf("%6.4e", ratio));
	}
	else
	{
		lbl_rod[1]->setText("f/f0 < 1.32, n/a");
		lbl_rod[2]->setText("f/f0 < 1.32, n/a");
		lbl_rod[3]->setText("f/f0 < 1.32, n/a");
	}
}

double US_Hydro2::calc_invtangent(double x)
{
	double tol = 1e-8;
	double i = 0.0;
	double lower = 0.0;
	double upper = M_PI/2;
	double func = 0;
	while (fabs(x - func) >= tol)
	{
		i += (upper - lower) / 10.0;
		func = tan(i);
		if (x < func)
		{
			lower = i - (upper - lower) / 10.0;
			upper = i;
			i = lower;
		}
	}
	return(i);
}

double US_Hydro2::root(int model, float goal)
{

	double test=0, x1=0, x2=0, x=0, xdiff=0;
	if (model == 1)
	{
		x1 = 1.1;	// lower range limit
		x2 = 5000;	// upper range limit
		x = 2500;	// half-way point
		xdiff = 1.0;
//
// Does the y-value fall above or below our goal?
//
		test = goal - (pow((double) x, (double) (-1.0/3.0)) * sqrt(pow((double) x, (double) 2.0) - 1.0)) / log(x + sqrt(pow((double) x, (double) 2.0) - 1.0));
//
// iterate until the difference between subsequent x value evaluations is too small to be relevant
//
		while (fabs(xdiff/x) > 1.0e-4)
		{
//
// Note: this test is only valid for monotone increasing functional values!
// If the difference between the goal and the testvalue is negative, then
// the x value was too high. We then adjust the top (x2) limit:
//
			if (test < 0.0)
			{
				x2 = x;
			}
//
// Otherwise, we adjust the lower limit:
//
			else
			{
				x1 = x;
			}
//
// We adjust the new test x-value by adding half the stepsize from the last step to the lower limit:
//
			xdiff = (x2 - x1)/2.0;
			x = x1 + xdiff;
//
// then we update the test y-value:
//
			test = goal - (pow((double) x, (double) (-1.0/3.0)) * sqrt(pow(x, 2.0) - 1.0)) / log(x + sqrt(pow((double) x, (double) 2.0) - 1.0));
		}
	}
	else if (model == 2)
	{
		x1 = 1.1;	// lower range limit
		x2 = 5000;	// upper range limit
		x = 2500;	// half-way point
		xdiff = 1.0;
//
// Does the y-value fall above or below our goal?
//
		test = goal - sqrt(pow((double) x, (double) 2.0) - 1.0) / (pow((double) x, (double) (2.0/3.0)) * calc_invtangent(sqrt(pow((double) x, (double) 2.0) - 1.0)));
		while (fabs(xdiff/x) > 1.0e-4)
		{
//
// Note: this test is only valid for monotone increasing functional values!
// If the difference between the goal and the testvalue is negative, then
// the x value was too high. We then adjust the top (x2) limit:
//
			if (test < 0.0)
			{
				x2 = x;
			}
//
// Otherwise, we adjust the lower limit:
//
			else
			{
				x1 = x;
			}
//
// We adjust the new test x-value by adding half the stepsize from the last step to the lower limit:
//
			xdiff = (x2 - x1) / 2.0;
			x = x1 + xdiff;
//
// then we update the test y-value:
//
			test = goal - sqrt(pow((double) x, (double) 2.0) - 1.0) / (pow((double) x, (double) (2.0/3.0)) * calc_invtangent(sqrt(pow((double) x, (double) 2.0) - 1.0)));
		}
	}
	else if (model == 3)
	{
		x1 = 6;	// lower range limit
		x2 = 5000;	// upper range limit
		x = 2497;	// half-way point
		xdiff = 1.0;
//
// Does the y-value fall above or below our goal?
//
		test = goal - (pow((double) (2.0/3.0), (double) (1.0/3.0)) * pow((double) x, (double) (2.0/3.0))) / (log(2.0 * x) - 0.3);
		while (fabs(xdiff/x) > 1.0e-4)
		{
//
// Note: this test is only valid for monotone increasing functional values!
// If the difference between the goal and the testvalue is negative, then
// the x value was too high. We then adjust the top (x2) limit:
//
			if (test < 0.0)
			{
				x2 = x;
			}
//
// Otherwise, we adjust the lower limit:
//
			else
			{
				x1 = x;
			}
//
// We adjust the new test x-value by adding half the stepsize from the last step to the lower limit:
//
			xdiff = (x2 - x1) / 2.0;
			x = x1 + xdiff;
//
// then we update the test y-value:
//
			test = goal - (pow((double) (2.0/3.0), (1.0/3.0)) * pow((double) x, (double) (2.0/3.0))) / (log(2.0 * x) - 0.3);
		}
	}
	return (x);
}

bool US_Hydro2::check_valid(float f, float f0)
{
	if ((f/f0) < 1.0)
	{
		QMessageBox::message(tr("Attention:"), tr("This model is physically impossible!\nThe f/f0 ratio is less than 1."));
		sed_coeff = 0.0;
		diff_coeff = 0.0;
		mw = 0.0;
		le_param1->setText("0.000");
		le_param2->setText("0.000");
		return (false);
	}
	else
	{
		return (true);
	}
}

void US_Hydro2::quit()
{
	close();
}

void US_Hydro2::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/predict2.html");
}

US_Hydro3::US_Hydro3(int run_type, float *temp_s20w, float *temp_D20w, float *temp_mw, float *temp_vbar20, 
const QString header_str, const int temp_component, QWidget *p, const char* name) : QFrame(p, name)
{
	
	int i;	
	int xpos = 4;
	int ypos = 4;
	int width = 140;
	int height = 26;
	mw = temp_mw;
	s20w = temp_s20w;
	D20w = temp_D20w;
	vbar = temp_vbar20;
	USglobal = new US_Config();
	component = temp_component;
	setGeometry (30+30*component, 30+30*component, 8 + 4*width + 3*spacing, 8*height + 5*spacing + 38);
	setMinimumSize(8 + 4*width + 3*spacing, 8*height + 5*spacing + 38);	
	setCaption(header_str);
	setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	
	QString label[4];
	label[0] = tr("Model:");
	label[1] = "a (Å)";
	label[2] = "b (Å)";
	label[3] = "a/b";

	for (i=0; i<4; i++)
	{
		lbl_header[i] = new QLabel(label[i], this);
		lbl_header[i]->setAlignment(AlignCenter|AlignVCenter);
		lbl_header[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
		lbl_header[i]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
		lbl_header[i]->setGeometry(xpos, ypos, width, height);
		xpos += spacing + width;
	}
	
	xpos = 4;
	ypos += height + spacing;

	lbl_prolate[0] = new QLabel(tr("Prolate:"), this);
	lbl_prolate[0]->setAlignment(AlignCenter|AlignVCenter);
	lbl_prolate[0]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_prolate[0]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_prolate[0]->setGeometry(xpos, ypos, width, height);

	for (i=1; i<4; i++)
	{
		xpos += spacing + width;
		lbl_prolate[i] = new QLabel("", this);
		lbl_prolate[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		lbl_prolate[i]->setAlignment(AlignCenter|AlignVCenter);
		lbl_prolate[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
		lbl_prolate[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		lbl_prolate[i]->setGeometry(xpos, ypos, width, height);
	}

	ypos += height + spacing;
	xpos = 4;	

	lbl_oblate[0] = new QLabel(tr("Oblate:"), this);
	lbl_oblate[0]->setAlignment(AlignCenter|AlignVCenter);
	lbl_oblate[0]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_oblate[0]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_oblate[0]->setGeometry(xpos, ypos, width, height);

	for (i=1; i<4; i++)
	{
		xpos += spacing + width;
		lbl_oblate[i] = new QLabel("", this);
		lbl_oblate[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		lbl_oblate[i]->setAlignment(AlignCenter|AlignVCenter);
		lbl_oblate[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
		lbl_oblate[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		lbl_oblate[i]->setGeometry(xpos, ypos, width, height);
	}

	ypos += height + spacing;
	xpos = 4;	

	lbl_rod[0] = new QLabel(tr("Long Rod:"), this);
	lbl_rod[0]->setAlignment(AlignCenter|AlignVCenter);
	lbl_rod[0]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_rod[0]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rod[0]->setGeometry(xpos, ypos, width, height);

	for (i=1; i<4; i++)
	{
		xpos += spacing + width;
		lbl_rod[i] = new QLabel("", this);
		lbl_rod[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		lbl_rod[i]->setAlignment(AlignCenter|AlignVCenter);
		lbl_rod[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
		lbl_rod[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		lbl_rod[i]->setGeometry(xpos, ypos, width, height);
	}

	ypos += height + 15;
	xpos = 4;	

	for (i=0; i<4; i++)
	{
		lbl_line1[i] = new QLabel("", this);
		lbl_line1[i]->setAlignment(AlignCenter|AlignVCenter);
		if (i == 0 || i == 2)
		{
			lbl_line1[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
			lbl_line1[i]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
		}
		else
		{
			lbl_line1[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
			lbl_line1[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
			lbl_line1[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		}
		lbl_line1[i]->setGeometry(xpos, ypos, width, height);
		xpos += spacing + width;
	}
	lbl_line1[0]->setText(tr("Frict. Coefficient:"));
	lbl_line1[2]->setText(tr("R0 (Sphere):"));

	ypos += height + spacing;
	xpos = 4;	

	for (i=0; i<4; i++)
	{
		lbl_line2[i] = new QLabel("", this);
		lbl_line2[i]->setAlignment(AlignCenter|AlignVCenter);
		if (i == 0 || i == 2)
		{
			lbl_line2[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
			lbl_line2[i]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
		}
		else
		{
			lbl_line2[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
			lbl_line2[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
			lbl_line2[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		}
		lbl_line2[i]->setGeometry(xpos, ypos, width, height);
		xpos += spacing + width;
	}
	lbl_line2[0]->setText(tr("Molecular Weight:"));
	lbl_line2[2]->setText(tr("f0 (Sphere):"));

	ypos += height + spacing;
	xpos = 4;	

	for (i=0; i<4; i++)
	{
		lbl_line3[i] = new QLabel("", this);
		lbl_line3[i]->setAlignment(AlignCenter|AlignVCenter);
		if (i == 0 || i == 2)
		{
			lbl_line3[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
			lbl_line3[i]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
		}
		else
		{
			lbl_line3[i]->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
			lbl_line3[i]->setFrameStyle(QFrame::Panel|QFrame::Sunken);
			lbl_line3[i]->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
		}
		lbl_line3[i]->setGeometry(xpos, ypos, width, height);
		xpos += spacing + width;
	}
	lbl_line3[0]->setText(tr("Volume (Å^3):"));
	lbl_line3[2]->setText("f/f0:");

	ypos += height + 15;
	xpos = 4;	

	pb_update = new QPushButton(tr("Update"), this);
	Q_CHECK_PTR(pb_update);
	pb_update->setAutoDefault(false);
	pb_update->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_update->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_update->setGeometry(xpos, ypos, width, height);
	connect(pb_update, SIGNAL(clicked()), SLOT(update()));

	xpos += width + spacing;

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, width, height);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	xpos += width + spacing;

	pb_close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_close);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setGeometry(xpos, ypos, width, height);
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));
	update();
	if (run_type != 0)
	{
		global_Xpos += 30;
		global_Ypos += 30;
		move(global_Xpos, global_Ypos);
	}
}

US_Hydro3::~US_Hydro3()
{
}

void US_Hydro3::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_Hydro3::update()
{
	QString str, str2;
	

	calc();
	lbl_line1[1]->setText(str.sprintf("%6.4e", frict_coeff));
	lbl_line1[3]->setText(str.sprintf("%6.4e", rad_sphere * 1.0e+08));
	lbl_line2[1]->setText(str.sprintf("%6.4e", *mw));
	lbl_line2[3]->setText(str.sprintf("%6.4e", f0));
	lbl_line3[1]->setText(str.sprintf("%6.4e", vol_per_molecule));
	if (f_ratio < 1.0)
	{
		lbl_line3[2]->setPalette(QPalette(USglobal->global_colors.cg_label_warn, USglobal->global_colors.cg_label_warn, USglobal->global_colors.cg_label_warn));
		lbl_line3[3]->setPalette(QPalette(USglobal->global_colors.cg_edit_warn, USglobal->global_colors.cg_edit_warn, USglobal->global_colors.cg_edit_warn));
		str2.sprintf(tr("\nThe F/F0 value for component %d is below 1.0!\n\n"
						  "Such a model is physically impossible. Therefore,\n"
						  "the fitted Model is meaningless and should be repeated\n"
						  "- possibly with different starting guesses."), component+1);
		QMessageBox::message(tr("Please note:"), str2);
	}
	else
	{
		lbl_line3[2]->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
		lbl_line3[3]->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	}
	lbl_line3[3]->setText(str.sprintf("%6.4e", f_ratio));

//
// prolate ellipsoid: model = 1
//
	lbl_prolate[1]->setText(str.sprintf("%6.4e", ap));
	lbl_prolate[2]->setText(str.sprintf("%6.4e", bp));
	lbl_prolate[3]->setText(str.sprintf("%6.4e", ratio_p));

//
// oblate ellipsoid: model = 2
//
	lbl_oblate[1]->setText(str.sprintf("%6.4e", ao));
	lbl_oblate[2]->setText(str.sprintf("%6.4e", bo));
	lbl_oblate[3]->setText(str.sprintf("%6.4e", ratio_o));

//
// long rod: model = 3
//
	
	if(f_ratio > 1.32)
	{
		lbl_rod[1]->setText(str.sprintf("%6.4e", ar));
		lbl_rod[2]->setText(str.sprintf("%6.4e", br));
		lbl_rod[3]->setText(str.sprintf("%6.4e", ratio_r));
	}
	else
	{
		lbl_rod[1]->setText("f/f0 < 1.32, n/a");
		lbl_rod[2]->setText("f/f0 < 1.32, n/a");
		lbl_rod[3]->setText("f/f0 < 1.32, n/a");
	}
}

void US_Hydro3::calc()
{
	US_Hydro2 *shape_model;
	shape_model = new US_Hydro2;

	vol_per_molecule = (*vbar) * (*mw) / AVOGADRO;
	rad_sphere = pow((double) (vol_per_molecule * (3.0/4.0))/M_PI, (double) (1.0/3.0));
	f0 = rad_sphere * 6.0 * M_PI * VISC_20W;
//
// recaluclate volume to put into cubic angstroms:
//
	vol_per_molecule = (4.0 / 3.0) * M_PI * pow((double) (rad_sphere * 1.0e+08), (double) 3.0);
	frict_coeff = ((*mw) * (1.0 - (*vbar) * DENS_20W)) / ((*s20w) * AVOGADRO);

	f_ratio = frict_coeff/f0;

//
// prolate ellipsoid: model = 1
//
	ratio_p = shape_model->root(1, f_ratio);
	ap = 1.0e+08 * (rad_sphere * pow((double) ratio_p, (double) (2.0/3.0)));
	bp = ap/ratio_p;
//
// oblate ellipsoid: model = 2
//
	ratio_o = shape_model->root(2, f_ratio);
	bo = 1.0e+08 * (rad_sphere / pow((double) ratio_o, (double) (2.0/3.0)));
	ao = ratio_o * bo;
//
// long rod: model = 3
//
	if(f_ratio > 1.32)
	{
		ratio_r = shape_model->root(3, f_ratio);
		br = 1.0e+08 * (pow((double) (2.0/(3.0*ratio_r)), (double) (1.0/3.0)) * rad_sphere);
		ar = ratio_r * br;
	}
}

void US_Hydro3::quit()
{
	close();
}

void US_Hydro3::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/finite_interpretation.html");
}



