#include "../include/us_extinction.h"

US_Extinction::US_Extinction(QString temp_projectName, int temp_investigatorID, QWidget *p , const char *name) : QFrame(p, name)
{
	USglobal = new US_Config();

	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("UltraScan: Extinction Coefficient Calculator"));

	QString str;
	pm = new US_Pixmap();
	lambdaCutoff = 220.0;
	lambda_min = 1000;
	lambda_max = -1000;
	pathlength = (float) 1.2;
	odCutoff = 1.0;
	projectName = temp_projectName;
	investigatorID=temp_investigatorID;
	extinction_coefficient = 1.0;
	selected_wavelength = 280;
	factor = 1.0;
	fitted = false;
	print_plot = false;
	order = 5;
	
	data_plot = new QwtPlot(this);
	Q_CHECK_PTR(data_plot);
	data_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	data_plot->enableGridXMin();
	data_plot->enableGridYMin();
	data_plot->enableAxis(QwtPlot::yRight);
	data_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	data_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	//data_plot->setPlotBackground(USglobal->global_colors.plot);			//old version
	data_plot->setCanvasBackground(USglobal->global_colors.plot);			//new version
	data_plot->setMargin(USglobal->config_list.margin);
	data_plot->enableOutline(true);
	data_plot->setAxisTitle(QwtPlot::xBottom, tr("Wavelength (nm)"));
	data_plot->setAxisTitle(QwtPlot::yLeft, tr("Optical Density"));
	data_plot->setAxisTitle(QwtPlot::yRight, tr("Extinction OD/(mol*cm)"));
	data_plot->setTitle(tr("Absorbance and Extinction Profile"));
	data_plot->setOutlinePen(white);
	data_plot->setOutlineStyle(Qwt::Cross);
	data_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	data_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));			
	data_plot->enableOutline(false);
	connect(data_plot, SIGNAL(plotMouseMoved(const QMouseEvent &)), SLOT(mouseMoved(const QMouseEvent&)));
	connect(data_plot, SIGNAL(plotMousePressed(const QMouseEvent &)), SLOT(mousePressed(const QMouseEvent&)));
	connect(data_plot, SIGNAL(plotMouseReleased(const QMouseEvent &)), SLOT(mouseReleased(const QMouseEvent&)));

	ext_l = new US_ExtinctionLegend(this);
	ext_l->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	connect (ext_l, SIGNAL(scaleChanged()), SLOT(update_scale()));

	lbl_banner1 = new QLabel(tr("Wavelength Information:"), this);
	Q_CHECK_PTR(lbl_banner1);
	lbl_banner1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_banner1->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	pb_selectScans = new QPushButton(tr("Add Wavelength Scanfile"), this);
	Q_CHECK_PTR(pb_selectScans);
	pb_selectScans->setAutoDefault(false);
	pb_selectScans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_selectScans->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_selectScans, SIGNAL(clicked()), SLOT(selectScans()));

	lb_scans = new QListBox(this, "Scans");
	lb_scans->setSelected(0, true);
	lb_scans->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_scans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	
	lbl_project = new QLabel(tr(" Associate with Run:"), this);
	Q_CHECK_PTR(lbl_project);
	lbl_project->setAlignment(AlignLeft|AlignVCenter);
	lbl_project->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_project->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_project = new QLineEdit(this, "project");
	le_project->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_project->setText(str.sprintf(" " + projectName));
	connect(le_project, SIGNAL(textChanged(const QString &)), 
				SLOT(update_project(const QString &)));	

	pb_reset = new QPushButton(tr("Reset Scanlist"), this);
	Q_CHECK_PTR(pb_reset);
	pb_reset->setAutoDefault(false);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

	pb_plot = new QPushButton(tr("Update Data Plot"), this);
	Q_CHECK_PTR(pb_plot);
	pb_plot->setAutoDefault(false);
	pb_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_plot->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_plot, SIGNAL(clicked()), SLOT(plot()));

	pb_fit = new QPushButton(tr("Perform Global Fit"), this);
	Q_CHECK_PTR(pb_fit);
	pb_fit->setAutoDefault(false);
	pb_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_fit, SIGNAL(clicked()), SLOT(fit()));

	lbl_order = new QLabel(tr(" # of Gaussians:"), this);
	Q_CHECK_PTR(lbl_order);
	lbl_order->setAlignment(AlignLeft|AlignVCenter);
	lbl_order->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_order->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_order= new QwtCounter(this);
	Q_CHECK_PTR(cnt_order);
	cnt_order->setRange(1, 30, 1);
	cnt_order->setNumButtons(1);
	cnt_order->setValue(order);
	cnt_order->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_order, SIGNAL(valueChanged(double)), SLOT(update_order(double)));

	cnt_wavelength= new QwtCounter(this);
	Q_CHECK_PTR(cnt_wavelength);
	cnt_wavelength->setRange(200, 750, 1);
	cnt_wavelength->setNumButtons(2);
	cnt_wavelength->setValue(selected_wavelength);
	cnt_wavelength->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_wavelength, SIGNAL(valueChanged(double)), SLOT(update_wavelength(double)));

	lbl_odCutoff = new QLabel(tr(" OD Cutoff:"), this);
	Q_CHECK_PTR(lbl_odCutoff);
	lbl_odCutoff->setAlignment(AlignLeft|AlignVCenter);
	lbl_odCutoff->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_odCutoff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	le_odCutoff = new QLineEdit(this, "odCutoff");
	le_odCutoff->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_odCutoff->setText(str.sprintf(" %2.3f", odCutoff));
	connect(le_odCutoff, SIGNAL(textChanged(const QString &)), 
				SLOT(update_odCutoff(const QString &)));	

	lbl_lambdaCutoff = new QLabel(tr(" Lambda Cutoff:"), this);
	Q_CHECK_PTR(lbl_lambdaCutoff);
	lbl_lambdaCutoff->setAlignment(AlignLeft|AlignVCenter);
	lbl_lambdaCutoff->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_lambdaCutoff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	le_lambdaCutoff = new QLineEdit(this, "lambdaCutoff");
	le_lambdaCutoff->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_lambdaCutoff->setText(str.sprintf(" %2.3f", lambdaCutoff));
	connect(le_lambdaCutoff, SIGNAL(textChanged(const QString &)), 
				SLOT(update_lambdaCutoff(const QString &)));	

	lbl_pathlength = new QLabel(tr(" Pathlength:"), this);
	Q_CHECK_PTR(lbl_pathlength);
	lbl_pathlength->setAlignment(AlignLeft|AlignVCenter);
	lbl_pathlength->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_pathlength->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	le_pathlength = new QLineEdit(this, "pathlength");
	le_pathlength->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_pathlength->setText(str.sprintf(" %2.4f", pathlength));
	connect(le_pathlength, SIGNAL(textChanged(const QString &)), 
				SLOT(update_pathlength(const QString &)));	

	lbl_banner2 = new QLabel(tr("Peptide Information:"), this);
	Q_CHECK_PTR(lbl_banner2);
	lbl_banner2->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_banner2->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	pb_selectPeptide = new QPushButton(tr("Calculate E280 from Peptide File"), this);
	Q_CHECK_PTR(pb_selectPeptide);
	pb_selectPeptide->setAutoDefault(false);
	pb_selectPeptide->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_selectPeptide->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_selectPeptide, SIGNAL(clicked()), SLOT(selectPeptide()));

	lbl_extinction = new QLabel(tr(" Extinction Coeff.:"), this);
	Q_CHECK_PTR(lbl_extinction);
	lbl_extinction->setAlignment(AlignLeft|AlignVCenter);
	lbl_extinction->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_extinction->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	le_extinction = new QLineEdit(this, "extinction");
	le_extinction->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_extinction->setText(str.sprintf(" %2.4f", 1.0));
	connect(le_extinction, SIGNAL(textChanged(const QString &)), 
				SLOT(update_extinction(const QString &)));	

	pb_save = new QPushButton(tr("Save"), this);
	Q_CHECK_PTR(pb_save);
	pb_save->setAutoDefault(false);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_save, SIGNAL(clicked()), SLOT(save()));

	pb_view = new QPushButton(tr("View Result File"), this);
	Q_CHECK_PTR(pb_view);
	pb_view->setAutoDefault(false);
	pb_view->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_view->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_view, SIGNAL(clicked()), SLOT(view()));

	pb_print = new QPushButton(tr("Print Plot Window"), this);
	Q_CHECK_PTR(pb_print);
	pb_print->setAutoDefault(false);
	pb_print->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_print->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_print, SIGNAL(clicked()), SLOT(print()));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_close);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_close, SIGNAL(clicked()), SLOT(cancel()));

	global_Xpos += 30;
	global_Ypos += 30;

	move(global_Xpos, global_Ypos);
	
	setup_GUI();
}

US_Extinction::~US_Extinction()
{
}

void US_Extinction::setup_GUI()
{
	int j=0;
	int rows = 21, columns = 2, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,2,2,spacing);
	background->setMargin(spacing);	
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->addMultiCellWidget(lbl_banner1,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_selectScans,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(lb_scans,j,j+4,0,1);
	j=j+5;
	subGrid1->addWidget(lbl_project,j,0);
	subGrid1->addWidget(le_project,j,1);
	j++;
	subGrid1->addMultiCellWidget(pb_reset,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_plot,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_fit,j,j,0,1);
	j++;
	subGrid1->addWidget(lbl_order,j,0);
	subGrid1->addWidget(cnt_order,j,1);
	j++;
	subGrid1->addWidget(lbl_odCutoff,j,0);
	subGrid1->addWidget(le_odCutoff,j,1);
	j++;
	subGrid1->addWidget(lbl_lambdaCutoff,j,0);
	subGrid1->addWidget(le_lambdaCutoff,j,1);
	j++;
	subGrid1->addWidget(lbl_pathlength,j,0);
	subGrid1->addWidget(le_pathlength,j,1);
	j++;
	subGrid1->addMultiCellWidget(lbl_banner2,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_selectPeptide,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(lbl_extinction, j, j+1, 0, 0);
	subGrid1->addWidget(cnt_wavelength, j, 1);
	subGrid1->addWidget(le_extinction, j+1, 1);
	j+=2;
	subGrid1->addMultiCellWidget(pb_save,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_view,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_print,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_help,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_close,j,j,0,1);
	
	background->addMultiCell(subGrid1,0,1,0,0);
	background->addWidget(data_plot,0,1);
	background->addWidget(ext_l,1,1);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(1,750);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);
	
	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width()+650+spacing*3, r.height());


}

void US_Extinction::closeEvent(QCloseEvent *e)
{
	if (fitted)
	{
		emit extinctionClosed();
	}
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}
/*
void US_Extinction::resizeEvent(QResizeEvent *e)
{
	int legendheight = 2 * buttonh + 2 * border + spacing;
	int panelwidth = 2 * border + buttonw;
	QRect r(panelwidth, border, e->size().width() - (panelwidth + border),
	e->size().height() - legendheight);
	data_plot->setGeometry(r);
	ext_l->setGeometry(panelwidth, e->size().height()-legendheight, e->size().width() - (panelwidth + border),
	legendheight);
}
*/
void US_Extinction::selectScans()
{
	QString filter, fileName, str, str1, extension;
	QStringList sl;
	int position;
	QFile f;
	filter = "*.W?? *.w??";

// allow multiple files to be selected:

	sl = QFileDialog::getOpenFileNames(filter, USglobal->config_list.data_dir, 0, 0);
	for (QStringList::Iterator it=sl.begin(); it!=sl.end(); it++)
	{
		fileName = *it;
		position = fileName.findRev(".");
		str = fileName.mid(position+2, 1);
		if(fileName.mid(position+2, 1) == "i" || fileName.mid(position+2, 1) == "I") // intensity scan
		{
			lambdaCutoff = 200.0;
			odCutoff = 1.0e5;
			le_lambdaCutoff->setText(str.sprintf(" %2.3f", lambdaCutoff));
			le_odCutoff->setText(str.sprintf(" %2.3e", odCutoff));
			data_plot->setTitle(tr("Intensity Profile"));
		}
		if (!fileName.isEmpty())
		{
			filenames.push_back(fileName);
			QFile f;
			f.setName(fileName);
			if (f.open(IO_ReadOnly))
			{
				QTextStream ts(&f);
				str1.sprintf(tr("Scan %d: "), filenames.size());
				str1 += ts.readLine();
				f.close();
			}
			else
			{
				QMessageBox::message(tr("UltraScan Error:"),	tr("The wavelength file:\n\n")
																		+ fileName
																		+ tr("\n\ncannot be read.\n\n"
																		"Please check to make sure that you have\n"
																		"read access to this file."));
			}
		}
		else
		{
			return;
		}
		lb_scans->insertItem(str1);
		plot();
	}
}

bool US_Extinction::loadScan(const QString &fileName)
{
	QString str1, str2;
	float temp_x, temp_y;
//	bool started = false;
	WavelengthScan wls;
	QFile f;
	f.setName(fileName);
	wls.lambda.clear();
	wls.od.clear();
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		wls.description = ts.readLine();
		str1 = ts.readLine(); // ignore second line from header - no useful information
		while (!ts.eof())
		{
			str1 = ts.readLine();
			str2 = getToken(&str1, " ");
			temp_x = str2.toFloat();
			str2 = getToken(&str1, " ");
			temp_y = str2.toFloat();
			if (temp_x >= lambdaCutoff && temp_y <= odCutoff)
			{
				wls.lambda.push_back(temp_x);
				lambda_max = max (temp_x, lambda_max);
				lambda_min = min (temp_x, lambda_min);
				wls.od.push_back(temp_y);
			}
		}
		f.close();
		if (wls.od.size() < 10)
		{
			QMessageBox::message(tr("UltraScan Error:"),	tr("This wavelength scan doesn't have\n"
																	"enough usable points - scan not loaded"));
			return (false);
		}
		wavelengthScan_vector.push_back(wls);
	}
	else
	{
		QMessageBox::message(tr("UltraScan Error:"),	tr("The wavelength file:\n\n")
																+ fileName
																+ tr("\n\ncannot be read.\n\n"
																"Please check to make sure that you have\n"
																"read access to this file."));
	}
	return(true);
}

void US_Extinction::update_order(double val)
{
	order = (int) val;
}

void US_Extinction::update_wavelength(double val)
{
	selected_wavelength = (float) val;
}

void US_Extinction::fit()
{
	if (wavelengthScan_vector.size() < 2)
	{
		QMessageBox::message(tr("UltraScan Error:"),	tr("You will need at least 2 scans\n"
																"to perform a global fit.\n\n"
																"Please add more scans before attempting\n"
																"a global fit."));
		return;
	}
	fitting_widget = false;
	parameters = order * 3 + wavelengthScan_vector.size();
	fitparameters = new double [parameters];
	for (unsigned int i=0; i<wavelengthScan_vector.size(); i++)
	{
		fitparameters[i] = 0.3;
	}
	float lambda_step = (lambda_max - lambda_min)/(order+1); // create "order" peaks evenly distributed over the range
	for (unsigned int i=0; i<order; i++)
	{
		fitparameters[wavelengthScan_vector.size() + (i * 3) ] = 1;
		// spread out the peaks 
		fitparameters[wavelengthScan_vector.size() + (i * 3) + 1] = lambda_min + lambda_step * i;
		fitparameters[wavelengthScan_vector.size() + (i * 3) + 2] = 10;
	}
	fitter = new US_ExtinctionFitter(&wavelengthScan_vector, fitparameters, order, parameters,
												projectName, &fitting_widget, 0, "fitter");
	fitter->show();
	connect(fitter, SIGNAL(fittingWidgetClosed()), SLOT(plot()));
	fitted = true;
	data_plot->enableOutline(true);
}

void US_Extinction::plot()
{
	unsigned int *curve, *polynomial, fit_curve, i, j, k;
	double **x_plot, **y_plot, **poly_plot, *fit_plotx, *fit_ploty, **coeffs;
	if (fitted)
	{
		calc_extinction();
		ext_l->lbl_instructions->setText(tr("Drag mouse through plot area to see extinction coefficient calculation"));		
	}
	QString str;
	xmax = 0.0;
	xmin = 10000.0;
	wavelengthScan_vector.clear();
	parameter_vector.clear();
	
	for (i=0; i<filenames.size(); i++)
	{
		if(!loadScan(filenames[i]))
		{
			filenames.pop_back();
			lb_scans->removeItem(i);
		}
		
	}
	if (wavelengthScan_vector.size() == 0)
	{
		return; // if we loaded a single scan with insufficient # of points we give up
	}
	for (i=0; i<wavelengthScan_vector.size(); i++)
	{
		for (j=0; j<wavelengthScan_vector[i].od.size(); j++)
		{
			wavelengthScan_vector[i].od[j] /= factor;
		}
	}
	data_plot->clear();
	QwtSymbol symbol;
	symbol.setSize(8);
	symbol.setStyle(QwtSymbol::Ellipse);
	if (print_plot)
	{
		symbol.setPen(Qt::black);
		symbol.setBrush(Qt::white);
	}
	else
	{
		symbol.setPen(Qt::blue);
		symbol.setBrush(Qt::yellow);
	}
	for (i=0; i<wavelengthScan_vector.size(); i++)
	{
		xmax = max(xmax, wavelengthScan_vector[i].lambda[wavelengthScan_vector[i].lambda.size()-1]);
		xmin = min(xmin, wavelengthScan_vector[i].lambda[0]);
	}
	curve = new unsigned int [wavelengthScan_vector.size()];
	polynomial = new unsigned int [wavelengthScan_vector.size()];
	x_plot = new double *[wavelengthScan_vector.size()];
	y_plot = new double *[wavelengthScan_vector.size()];
	poly_plot = new double *[wavelengthScan_vector.size()];
	coeffs = new double *[wavelengthScan_vector.size()];
	for (i=0; i<wavelengthScan_vector.size(); i++)
	{
		coeffs[i] = new double [order];
	}
	for (i=0; i<wavelengthScan_vector.size(); i++)
	{
		x_plot[i] = new double [wavelengthScan_vector[i].lambda.size()];
		y_plot[i] = new double [wavelengthScan_vector[i].lambda.size()];
		poly_plot[i] = new double [wavelengthScan_vector[i].lambda.size()];
	}
	maxrange = (unsigned int) (xmax - xmin + 0.5);
	maxrange += 1;
	fit_plotx = new double [maxrange];
	fit_ploty = new double [maxrange];
	
	if (fitted)
	{
		for (i=0; i<maxrange; i++)
		{
			fit_plotx[i] = lambda[i];
			fit_ploty[i] = extinction[i];
		}
		fit_curve = data_plot->insertCurve("Extinction");
		if (print_plot)
		{
			data_plot->setCurvePen(fit_curve, QPen(Qt::white, 2, DotLine));
		}
		else
		{
			data_plot->setCurvePen(fit_curve, QPen(Qt::red, 2, SolidLine));
		}
		data_plot->setCurveData(fit_curve, fit_plotx, fit_ploty, maxrange);
		data_plot->setCurveYAxis(fit_curve, QwtPlot::yRight);
	}
	for (i=0; i<wavelengthScan_vector.size(); i++)
	{
		curve[i] = data_plot->insertCurve(wavelengthScan_vector[i].description);
		if (fitted)
		{
			polynomial[i] = data_plot->insertCurve(wavelengthScan_vector[i].description + "-fit");
			if (print_plot)
			{
				data_plot->setCurvePen(polynomial[i], QPen(Qt::white, 1, SolidLine));
				data_plot->setCurvePen(curve[i], QPen(Qt::white, 1, NoPen));
			}
			else
			{
				data_plot->setCurvePen(polynomial[i], QPen(Qt::cyan, 1, SolidLine));
				data_plot->setCurvePen(curve[i], QPen(Qt::green, 1, NoPen));
			}
		}
		else
		{
			if (print_plot)
			{
				data_plot->setCurvePen(curve[i], QPen(Qt::white, 1, SolidLine));
			}
			else
			{
				data_plot->setCurvePen(curve[i], QPen(Qt::green, 1, SolidLine));
			}
		}

		data_plot->setCurveSymbol(curve[i], symbol);
		for (j=0; j<wavelengthScan_vector[i].lambda.size(); j++)
		{
			x_plot[i][j] = wavelengthScan_vector[i].lambda[j];
			y_plot[i][j] = wavelengthScan_vector[i].od[j];
		}
		US_lsfit *polyfit;
		polyfit = new US_lsfit(coeffs[i], x_plot[i], y_plot[i], (int) order, (int) wavelengthScan_vector[i].lambda.size(), false);
		delete polyfit;
		if (fitted)
		{
			for (j=0; j<wavelengthScan_vector[i].lambda.size(); j++)
			{
				poly_plot[i][j] = 0.0;
				for (k=0; k<order; k++)
				{
					poly_plot[i][j] += exp(fitparameters[wavelengthScan_vector.size() + (3 * k)]
					- (pow((wavelengthScan_vector[i].lambda[j] - fitparameters[wavelengthScan_vector.size() + (3 * k) + 1]), 2)
					/ ( 2 * pow(fitparameters[wavelengthScan_vector.size() + (3 * k) + 2], 2))));
				}
				poly_plot[i][j] *= fitparameters[i]; 
			}
		}
		data_plot->setCurveData(curve[i], x_plot[i], y_plot[i], wavelengthScan_vector[i].lambda.size());
		if (fitted)
		{
			data_plot->setCurveData(polynomial[i], x_plot[i], poly_plot[i], wavelengthScan_vector[i].lambda.size());
		}
	}
	data_plot->replot();
	initialize(coeffs);
	if (fitted)	//send the results to the caller
	{
		emit fitUpdated(fit_plotx, fit_ploty, maxrange);
	}
	for (i=0; i<wavelengthScan_vector.size(); i++)
	{
		delete [] x_plot[i];
		delete [] y_plot[i];
		delete [] poly_plot[i];
	}
	for (i=0; i<wavelengthScan_vector.size(); i++)
	{
		delete [] coeffs[i];
	}
	delete [] fit_plotx;
	delete [] fit_ploty;
	delete [] coeffs;
	delete [] curve;
	delete [] polynomial;
	delete [] x_plot;
	delete [] y_plot;
	delete [] poly_plot;
}

void US_Extinction::initialize(double **coeffs)
{
	unsigned int i, j, highest = 0;
	int range = 0;
	QString str;
	float xmaxa = 0.0,	xmina = 10000.0;
   float *integral;
   integral = new float [wavelengthScan_vector.size()];
	for (i=0; i<wavelengthScan_vector.size(); i++)
	{
		integral[i] = 0.0;
	}

// find the largest range common to all scans:

	for (i=1; i<wavelengthScan_vector.size(); i++)
	{
		xmaxa = min(wavelengthScan_vector[0].lambda[wavelengthScan_vector[0].lambda.size()-1],
					  wavelengthScan_vector[i].lambda[wavelengthScan_vector[i].lambda.size()-1]);
		xmina = max(wavelengthScan_vector[0].lambda[0], wavelengthScan_vector[i].lambda[0]);
	}
	for (i=1; i<wavelengthScan_vector.size(); i++)
	{
		float val1 = 0.0;
		float val2 = 0.0;
// calculate the integrals and compare the total concentration of each:
		for (j=0; j<order; j++)
		{
			val1 += (coeffs[0][j]/(j+1)) * (pow((double) xmaxa, (double) (j+1)) - pow((double) xmina, (double) (j+1)));
			val2 += (coeffs[i][j]/(j+1)) * (pow((double) xmaxa, (double) (j+1)) - pow((double) xmina, (double) (j+1)));
		}
		integral[i] = val2/val1;
	}
	integral[0] = 1.0; // that's the ratio of the first integral with itself
	
/*
for (i=0; i<wavelengthScan_vector.size(); i++)
{
	cout << "Scan " << i << ": ";
	for (j=0; j<order; j++)
	{
		cout << coeffs[i][j] << "\t";
	}
	cout << endl;
}
*/

	for (i=1; i<wavelengthScan_vector.size(); i++)
	{
		for (j=0; j<order; j++)
		{
			coeffs[0][j] += coeffs[i][j];
		}
	}

// find largest integral:

	xmaxa = integral[0];
	for (i=1; i<wavelengthScan_vector.size(); i++)
	{
		if (integral[i] > xmaxa)
		{
			xmaxa = integral[i];
			highest = i;
		}
	}

// find largest weight:

	xmaxa = 0.0;
	for (i=0; i<wavelengthScan_vector.size(); i++)
	{
		range = (int) (wavelengthScan_vector[i].lambda[wavelengthScan_vector[i].lambda.size() - 1]
		- wavelengthScan_vector[i].lambda[0]);
		if ((float) range > xmaxa)
		{
			xmaxa = (float) range;
			highest = i;
		}
	}

// normalize all concentration multipliers with the concentration of the scan with the most datapoints:
	for (i=0; i<wavelengthScan_vector.size(); i++)
	{
		integral[i] /= integral[highest];
		parameter_vector.push_back(integral[i]);
	}

// initialize the parameters with the polynomial of the scan with the most datapoints:
	for (i=0; i<order; i++)
	{
		parameter_vector.push_back(coeffs[0][i]);
	}

	delete [] integral;
}

void US_Extinction::reset()
{
	lb_scans->clear();
	filenames.clear();
	wavelengthScan_vector.clear();
	data_plot->clear();
	lambda.clear();
	extinction.clear();
	data_plot->replot();
	fitted = false;
	data_plot->enableOutline(false);
}

void US_Extinction::selectPeptide()
{
	float vbar= (float) 0.72, temp=20, vbar20= (float) 0.72;
	cnt_wavelength->setValue(280);
	US_Vbar_DB *vbar_dlg;
	vbar_dlg = new US_Vbar_DB(temp, &vbar, &vbar20,true, false, investigatorID);
	vbar_dlg->setCaption(tr("V-bar Calculation"));
	vbar_dlg->pb_ok->setText(tr(" Close "));
	connect(vbar_dlg, SIGNAL(e280Changed(float)), SLOT(update_extinction_coefficient(float)));
	vbar_dlg->show();
}

void US_Extinction::update_extinction_coefficient(float val)
{
	QString str;
	extinction_coefficient = val;
	le_extinction->setText(str.sprintf("%7.1f", extinction_coefficient));
	if (fitted)
	{
		calc_extinction();
	}
}

void US_Extinction::calc_extinction()
{
	unsigned int i, j;
	if (wavelengthScan_vector.empty())
	{
		return;
	}
	if (!lambda.empty())
	{
		lambda.clear();
		extinction.clear();
	}
	float od_wavelength=0, od;
	xmax = -1.0;
	xmin = 1e6;
	for (i=0; i<wavelengthScan_vector.size(); i++)
	{
		xmax = max(xmax, wavelengthScan_vector[i].lambda[wavelengthScan_vector[i].lambda.size()-1]);
		xmin = min(xmin, wavelengthScan_vector[i].lambda[0]);
	}
	maxrange = (unsigned int) (xmax - xmin + 0.5);
	maxrange += 1;
	for (i=0; i<maxrange; i++)
	{
		lambda.push_back(xmin + i);
		od = 0.0;
		for (j=0; j<order; j++)
		{
			od += exp(fitparameters[wavelengthScan_vector.size() + (3 * j)]
					- (pow((lambda[i] - fitparameters[wavelengthScan_vector.size() + (3 * j) + 1]), 2)
					/ ( 2 * pow(fitparameters[wavelengthScan_vector.size() + (3 * j) + 2], 2))));
		}
		extinction.push_back(od);
		if((unsigned int) lambda[i] == selected_wavelength)
		{
			od_wavelength = od;
		}
	}
	for (i=0; i<maxrange; i++)
	{
		extinction[i] = extinction_coefficient * (extinction[i]/od_wavelength);
	}
}

void US_Extinction::save()
{
	if (!fitted)
	{
		QMessageBox::message(tr("UltraScan Error:"),	tr("Nothing to save. First you have\n"
																"to perform a global fit"));
		return;
	}
	unsigned int i;
	QString str;
	createHtmlDir();
	QString fileName = USglobal->config_list.result_dir + "/" + projectName + ".extinction.dat";
	QFile f(fileName);
	if(f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		ts << tr("\"Wavelength\"\t\"Extinction\"\n");
		for (i=0; i<maxrange; i++)
		{
			ts << lambda[i] << "\t";
			ts << extinction[i] << "\n";
		}
	}
	f.close();
	fileName = USglobal->config_list.result_dir + "/" + projectName + ".extinction.res";
	f.setName(fileName);
	if(f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		ts << tr("Results for global wavelength scan/extinction coefficient fit:\n\n");
		ts << tr("Number of Gaussian terms: ") << order << "\n\n";
		ts << tr("Extinction coeffcient at ") << selected_wavelength << tr(" nm used for normalization of Data: ") << extinction_coefficient << "\n\n";
		ts << tr("Parameters for each Gaussian term:\n");
		for (i=0; i<order; i++)
		{
			ts << "\n" << (i+1) << tr(". Gaussian:\n");
			ts << tr("Peak position: ") << fitparameters[wavelengthScan_vector.size() + (i * 3) + 1] << " nm\n";
			ts << tr("Amplitude of peak: ") << exp(fitparameters[wavelengthScan_vector.size() + (i * 3)]) << tr(" extinction\n");
			ts << tr("Peak width: ") << fitparameters[wavelengthScan_vector.size() + (i * 3) + 2] << " nm\n";
		}
		ts << "\n";
		for (i=0; i<wavelengthScan_vector.size(); i++)
		{
			ts << tr("Relative Concentration of Scan ") << (i+1) << ": " << fitparameters[i] << "\n";
		}
		ts << "\n";
	}
	f.close();
	QPixmap p;
	fileName = htmlDir + "/" + projectName + ".extinction.";
	p = QPixmap::grabWidget(data_plot, 2, 2, data_plot->width() - 4, data_plot->height() - 4);
	pm->save_file(fileName, p);
	QMessageBox::message(tr("UltraScan Information:"),
								tr("The data were successfully saved in the following files:\n\n"
								"Extinction X/Y data (ASCII format):\n") +
								USglobal->config_list.result_dir + "/" + projectName + ".extinction.dat\n\n"
								+ tr("Result file:\n") + 
								USglobal->config_list.result_dir + "/" + projectName + ".extinction.res\n\n"
								+ tr("Plot image:\n") +
								htmlDir + "/" + projectName + ".extinction.gif");
}

void US_Extinction::view()
{
	if (!fitted)
	{
		QMessageBox::message(tr("UltraScan Error:"),	tr("Nothing to view. First you have\n"
																"to perform a global fit"));
		return;
	}
	QString fileName;
	save();
	fileName = USglobal->config_list.result_dir + "/" + projectName + ".extinction.dat";
	//view_file(fileName);
	TextEdit *e1;
	e1 = new TextEdit();
	e1->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	e1->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
	e1->load(fileName);
	e1->show();

	fileName = USglobal->config_list.result_dir + "/" + projectName + ".extinction.res";
	//view_file(fileName);
	TextEdit *e2;
	e2 = new TextEdit();
	e2->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	e2->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
	e2->load(fileName);
	e2->show();

}

void US_Extinction::print()
{
	QPrinter printer;
	bool print_bw =  false;
	bool print_inv =  false;
	US_SelectPlot *sp;
	sp = new US_SelectPlot(&print_bw, &print_inv);
	sp->exec();
	if	(printer.setup(0))
	{
		if (print_bw)
		{
			PrintFilter pf;
			pf.setOptions(QwtPlotPrintFilter::PrintTitle
							 |QwtPlotPrintFilter::PrintMargin
							 |QwtPlotPrintFilter::PrintLegend
							 |QwtPlotPrintFilter::PrintGrid);
			data_plot->print(printer, pf);
		}
		else if (print_inv)
		{
			PrintFilterDark pf;
			pf.setOptions(QwtPlotPrintFilter::PrintTitle
							 |QwtPlotPrintFilter::PrintMargin
							 |QwtPlotPrintFilter::PrintLegend
							 |QwtPlotPrintFilter::PrintGrid);
			data_plot->print(printer, pf);
		}
		else
		{
			data_plot->print(printer);
		}
	}
}

void US_Extinction::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/extinction.html");
}

void US_Extinction::cancel()
{
	plot();	// update the plot before exiting to make sure all arrays are properly initialized
	close();
}

void US_Extinction::update_odCutoff(const QString &str)
{
	odCutoff = str.toFloat();
}

void US_Extinction::update_lambdaCutoff(const QString &str)
{
	lambdaCutoff = str.toFloat();
	if (lambda_min < lambdaCutoff)
	{
		lambda_min = lambdaCutoff; // adjust lambda_min to the cutoff
	}
}

void US_Extinction::update_pathlength(const QString &str)
{
	wavelengthScan_vector.clear();
	pathlength = str.toFloat();
	update_scale();	
}

void US_Extinction::update_extinction(const QString &str)
{
	extinction_coefficient = str.toFloat();
	if (fitted)
	{
		calc_extinction();
	}
}

void US_Extinction::update_project(const QString &str)
{
	projectName = str.stripWhiteSpace();
	emit projectChanged(projectName);
}

void US_Extinction::update_scale()
{
	if(ext_l->normalized)
	{
		factor = pathlength;
	}
	else
	{
		factor = 1.0;
	}
	if (!filenames.empty())
	{
		plot();
	}
}

void US_Extinction::mouseMoved(const QMouseEvent &e)
{
	if(!fitted || lambda.size() == 0)
	{
		return;
	}
	QString str;
	unsigned int index = 0;
	double test_lambda;
	test_lambda = (data_plot->invTransform(QwtPlot::xBottom, e.x()));
	if (test_lambda >= xmin && test_lambda <= xmax && fitted)
	{
		while ((index < maxrange) && (lambda[index] < test_lambda))
		{
			index++;
		}
		str.sprintf(" %3.3f", lambda[index]);
		ext_l->lbl_lambda2->setText(str);
		str.sprintf(" %7.1f OD/(mol*cm)", extinction[index]);
		ext_l->lbl_extinction2->setText(str);
	}
	else
	{
		ext_l->lbl_lambda2->setText(" 0 nm");
		ext_l->lbl_extinction2->setText(" 0.0 OD/(mol*cm)");
	}
}

void US_Extinction::mousePressed(const QMouseEvent &e)
{
	QFile f;
	QString str1;
	long int index;
	int dist;
    // store position
	point = e.pos();

    // update cursor pos display
	if (fitted)	// then we want to display the coordinates as we move the mouse through plot
	{
		mouseMoved(e);
	}
	else // then we want to delete a scan
	{
		index = data_plot->closestCurve(e.x(), e.y(), dist);
		if (index == 0)
		{
			return;
		}
		data_plot->setCurvePen(index, QPen(Qt::magenta, 1, SolidLine));
		data_plot->replot();
		QMessageBox mb(tr("UltraScan"), 
							tr("Do you want to delete the highlighted curve?"),
		QMessageBox::Information,
		QMessageBox::Yes | QMessageBox::Default,
		QMessageBox::No,
		QMessageBox::Cancel | QMessageBox::Escape);
		mb.setButtonText(QMessageBox::Yes, "Yes");
		mb.setButtonText(QMessageBox::No, "No");
		switch(mb.exec())
		{
			case QMessageBox::Yes:
			{
				vector <QString>::iterator name_iter = filenames.begin() + index - 1;
				vector <struct WavelengthScan>::iterator wls_iter = wavelengthScan_vector.begin() + index;
				data_plot->removeCurve(index);
				filenames.erase(name_iter);
				wavelengthScan_vector.erase(wls_iter);
				lb_scans->clear();
				for (unsigned int i=0; i<filenames.size(); i++)
				{
					f.setName(filenames[i]);
					if (f.open(IO_ReadOnly))
					{
						QTextStream ts(&f);
						str1.sprintf(tr("Scan %d: "), i+1);
						str1 += ts.readLine();
						f.close();
					}
					lb_scans->insertItem(str1);
				}
				break;
			}
			default:
			{
				data_plot->setCurvePen(index, QPen(Qt::green, 1, SolidLine));
				break;
			}
		}
		plot();
	}
}


void US_Extinction::mouseReleased(const QMouseEvent &)
{
	ext_l->lbl_lambda2->setText(" 0 nm");
	ext_l->lbl_extinction2->setText(" 0.0 OD/(mol*cm)");
}

bool US_Extinction::createHtmlDir()
{
	htmlDir = USglobal->config_list.html_dir + "/" + projectName;
	QDir d(htmlDir);
	if (d.exists())
	{
		return true;
	}
	else
	{
		if (d.mkdir(htmlDir, true))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

US_ExtinctionLegend::US_ExtinctionLegend(QWidget *p , const char *name) : QFrame(p, name)
{
	spacing = 2;
	buttonh = 26;
	column1 = 117;
	column2 = 120;
	border = 4;
	xpos = 0;
	ypos = 4;
	normalized = false;
	USglobal = new US_Config();
/*
	QLabel *lbl_blank1;
	lbl_blank1 = new QLabel("",this);
	lbl_blank1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_blank1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lbl_blank1->setGeometry(xpos, ypos, column2, buttonh);
*/
	rb_pathlength = new QRadioButton(this);
	rb_pathlength->setText(tr("Pathlength"));
	rb_pathlength->setGeometry(xpos+10, ypos+4, column2-10, 18);
	rb_pathlength->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	rb_pathlength->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	rb_pathlength->setChecked(true);
	connect(rb_pathlength, SIGNAL(clicked()), SLOT(set_pathlength()));

	xpos += column2 + spacing;

	lbl_lambda1 = new QLabel(tr("Wavelength:"),this);
	lbl_lambda1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_lambda1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_lambda1->setGeometry(xpos, ypos, column1 - 20, buttonh);
	lbl_lambda1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing - 20;

	lbl_lambda2 = new QLabel(" 0 nm",this);
	lbl_lambda2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_lambda2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_lambda2->setGeometry(xpos, ypos, column2 - 20, buttonh);
	lbl_lambda2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing - 20;

	lbl_extinction1 = new QLabel(tr("Extinction:"),this);
	lbl_extinction1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_extinction1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_extinction1->setGeometry(xpos, ypos, column1 - 20, buttonh);
	lbl_extinction1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing - 20;

	lbl_extinction2 = new QLabel(" 0.0 OD/(mol*cm)",this);
	lbl_extinction2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_extinction2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_extinction2->setGeometry(xpos, ypos, column2 + 60, buttonh);
	lbl_extinction2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos = 0;
	ypos += buttonh + spacing;
/*	
	QLabel *lbl_blank2;
	lbl_blank2 = new QLabel("",this);
	lbl_blank2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_blank2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lbl_blank2->setGeometry(xpos, ypos, column2, buttonh);
*/
	rb_normalized = new QRadioButton(this);
	rb_normalized->setText(tr("Normalized"));
	rb_normalized->setGeometry(xpos+10, ypos+4, column2-10, 18);
	rb_normalized->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	rb_normalized->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	rb_normalized->setChecked(false);
	connect(rb_normalized, SIGNAL(clicked()), SLOT(set_normalized()));

	xpos += column2 + spacing;

	lbl_instructions = new QLabel("",this);
	lbl_instructions->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_instructions->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_instructions->setGeometry(xpos, ypos, 2 * column2 + 2* column1 + 3 * spacing, buttonh);
	lbl_instructions->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	setup_GUI();
};

US_ExtinctionLegend::~US_ExtinctionLegend()
{
}

void US_ExtinctionLegend::setup_GUI()
{
	int j=0;
	int rows = 2, columns = 5, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,rows, columns,spacing);	
	for (int i=0; i<rows; i++)
	{
		background->setRowSpacing(i, 26);
	}
	background->addWidget(rb_pathlength,j,0);
	background->addWidget(lbl_lambda1,j,1);
	background->addWidget(lbl_lambda2,j,2);
	background->addWidget(lbl_extinction1,j,3);
	background->addWidget(lbl_extinction2,j,4);
	j++;
	background->addWidget(rb_normalized,j,0);
	background->addMultiCellWidget(lbl_instructions,j,j,1,4);

}


void US_ExtinctionLegend::set_pathlength()
{
	normalized = false;
	rb_pathlength->setChecked(true);
	rb_normalized->setChecked(false);
	emit scaleChanged();
}

void US_ExtinctionLegend::set_normalized()
{
	normalized = true;
	rb_pathlength->setChecked(false);
	rb_normalized->setChecked(true);
	emit scaleChanged();
}
