#include "../include/us_editwavelength.h"

US_EditWavelengthScan::US_EditWavelengthScan(QWidget *p , const char *name) : QFrame(p, name)
{
	USglobal = new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("Wavelength Scan Editor"));
	WavelengthFile.clear();
	currentScan = 0;
	target = "";
	copyFlag = true;
	int minHeight1 = 26;

	wavelength_plot = new QwtPlot(this);
	wavelength_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	wavelength_plot->enableGridXMin();
	wavelength_plot->enableGridYMin();
	wavelength_plot->enableAxis(QwtPlot::yRight);
	wavelength_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	wavelength_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	wavelength_plot->setCanvasBackground(USglobal->global_colors.plot);			//new version
	wavelength_plot->setMargin(USglobal->config_list.margin);
	wavelength_plot->enableOutline(true);
	wavelength_plot->setAxisTitle(QwtPlot::xBottom, tr("Wavelength (nm)"));
	wavelength_plot->setAxisTitle(QwtPlot::yLeft, tr("Optical Density"));
	wavelength_plot->setTitle(tr("Absorbance Wavelenngth Scan"));
	wavelength_plot->setOutlinePen(white);
	wavelength_plot->setOutlineStyle(Qwt::Cross);
	wavelength_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	wavelength_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	wavelength_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	wavelength_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	wavelength_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	wavelength_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	wavelength_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));			
	wavelength_plot->enableOutline(false);
	wavelength_plot->setMinimumSize(600, 450);
	connect(wavelength_plot, SIGNAL(plotMouseMoved(const QMouseEvent &)), SLOT(mouseMoved(const QMouseEvent&)));
	connect(wavelength_plot, SIGNAL(plotMouseReleased(const QMouseEvent &)), SLOT(mouseReleased(const QMouseEvent&)));

	pb_selectScans = new QPushButton(tr("Add Wavelength Scan(s)"), this);
	Q_CHECK_PTR(pb_selectScans);
	pb_selectScans->setAutoDefault(false);
	pb_selectScans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_selectScans->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_selectScans->setMinimumHeight(minHeight1);
	connect(pb_selectScans, SIGNAL(clicked()), SLOT(selectScans()));

	pb_selectTarget = new QPushButton(tr("Select Target Location"), this);
	Q_CHECK_PTR(pb_selectTarget);
	pb_selectTarget->setAutoDefault(false);
	pb_selectTarget->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_selectTarget->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_selectTarget->setEnabled(true);
	pb_selectTarget->setMinimumHeight(minHeight1);
	connect(pb_selectTarget, SIGNAL(clicked()), SLOT(selectTarget()));

	cmb_scans = new QComboBox( FALSE, this, "Scan List" );
	cmb_scans->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_scans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_scans->setSizeLimit(5);
	cmb_scans->setMinimumHeight(minHeight1);
	connect(cmb_scans, SIGNAL(activated(int)), this, SLOT(showScan(int)));
	connect(cmb_scans, SIGNAL(highlighted(int)), this, SLOT(showScan(int)));

	pb_resetList = new QPushButton(tr("Reset Scan List"), this);
	pb_resetList->setAutoDefault(false);
	pb_resetList->setEnabled(false);
	pb_resetList->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_resetList->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_resetList->setMinimumHeight(minHeight1);
	connect(pb_resetList, SIGNAL(clicked()), SLOT(resetList()));

	pb_resetScan = new QPushButton(tr("Reset Scan Limits"), this);
	pb_resetScan->setAutoDefault(false);
	pb_resetScan->setEnabled(false);
	pb_resetScan->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_resetScan->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_resetScan->setMinimumHeight(minHeight1);
	connect(pb_resetScan, SIGNAL(clicked()), SLOT(resetScan()));

	pb_save = new QPushButton(tr("Save current Scan"), this);
	Q_CHECK_PTR(pb_save);
	pb_save->setAutoDefault(false);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_save->setEnabled(false);
	pb_save->setMinimumHeight(minHeight1);
	connect(pb_save, SIGNAL(clicked()), SLOT(saveScan()));

	pb_next = new QPushButton(tr("Next Scan"), this);
	pb_next->setAutoDefault(false);
	pb_next->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_next->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_next->setEnabled(false);
	pb_next->setMinimumHeight(minHeight1);
	connect(pb_next, SIGNAL(clicked()), SLOT(nextScan()));

	pb_help = new QPushButton(tr("Help"), this);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setMinimumHeight(minHeight1);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_close = new QPushButton(tr("Close"), this);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setMinimumHeight(minHeight1);
	connect(pb_close, SIGNAL(clicked()), SLOT(close()));

	cb_copy = new QCheckBox("Copy Scanfile\nto new Location", this);
	cb_copy->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_copy->setChecked(true);
	cb_copy->setEnabled(true);
	cb_copy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_copy->setMinimumHeight(minHeight1);
	connect(cb_copy, SIGNAL(clicked()), SLOT(selectCopy()));

	cb_overwrite = new QCheckBox("Overwrite Original\nScanfile", this);
	cb_overwrite->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_overwrite->setChecked(false);
	cb_overwrite->setEnabled(true);
	cb_overwrite->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_overwrite->setMinimumHeight(minHeight1);
	connect(cb_overwrite, SIGNAL(clicked()), SLOT(selectOverwrite()));

	progress = new QProgressBar(100, this, "Editing Complete");
	progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	progress->setPercentageVisible(true);
	progress->setMinimumHeight(minHeight1);

	global_Xpos += 30;
	global_Ypos += 30;
	setupGUI();
	move(global_Xpos, global_Ypos);
}

US_EditWavelengthScan::~US_EditWavelengthScan()
{
}

void US_EditWavelengthScan::setupGUI()
{
	QGridLayout *topBox = new QGridLayout(this, 2, 2, 4, 2);
	topBox->setColStretch(1, 1);
	topBox->setColStretch(0, 0);
	QVBoxLayout *buttonBox = new QVBoxLayout();
	buttonBox->addWidget(pb_resetList);
	buttonBox->addWidget(pb_resetScan);
	buttonBox->addWidget(pb_next);
	buttonBox->addWidget(pb_save);
	buttonBox->addWidget(cb_copy);
	buttonBox->addWidget(pb_selectTarget);
	buttonBox->addWidget(cb_overwrite);
	buttonBox->addWidget(progress);
	buttonBox->addWidget(pb_help);
	buttonBox->addWidget(pb_close);
	
	topBox->addMultiCellLayout(buttonBox, 1, 1, 0, 0, Qt::AlignTop|Qt::AlignLeft);
	topBox->addWidget(pb_selectScans, 0, 0, 0);
	topBox->addWidget(cmb_scans, 0, 1, Qt::AlignTop);
	topBox->addWidget(wavelength_plot, 1, 1, 0);

	qApp->processEvents();
	QRect r = topBox->cellGeometry(0, 0);
	this->setGeometry(global_Xpos, global_Ypos, r.width()+10+600, 550+6+8);
}

void US_EditWavelengthScan::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

void US_EditWavelengthScan::selectTarget()
{
	target = QFileDialog::getExistingDirectory(USglobal->config_list.data_dir, 0, 0,
	tr("Please create/select a folder for the edited scans:"), true, true);
}

void US_EditWavelengthScan::selectScans()
{
	QString filter, str, str1, extension, fileName;
	QStringList sl;
	double dval;
	int position;
	struct EditScan es;
	QFile f;
	filter = "*.W?? *.w??";

// allow multiple files to be selected:
	QFileDialog fd;
	QStringList list = fd.getOpenFileNames(filter, USglobal->config_list.data_dir, 0, 0);
	if (list.empty())
	{
		return;
	}
	sl = list;
	for (QStringList::Iterator it=sl.begin(); it!=sl.end(); it++)
	{
		fileName = *it;
		QFileInfo fi(fileName);
		es.filename = fi.fileName();
		es.path = fi.dirPath();
		position = es.filename.findRev(".");
		str = es.filename.mid(position+2, 1);
		if(es.filename.mid(position+2, 1) == "i" || es.filename.mid(position+2, 1) == "I") // intensity scan
		{
			wavelength_plot->setTitle(tr("Intensity Wavelength Scan"));
		}
		if (!es.filename.isEmpty())
		{
			QFile f;
			f.setName(fileName);
			if (f.open(IO_ReadOnly))
			{
				QTextStream ts(&f);
				es.header1 = ts.readLine();
				es.header2 = ts.readLine();
				es.line.clear();
				es.concentration.clear();
				es.wavelength.clear();
				while (!ts.atEnd())
				{
					str1 = ts.readLine();
					es.line.push_back(str1);
					str = getToken(&str1, " ");
					dval = str.toDouble();
					es.wavelength.push_back(dval);
					str = getToken(&str1, " ");
					dval = str.toDouble();
					es.concentration.push_back(dval);
				}
				es.startLineIndex = 0;
				es.stopLineIndex = es.line.size() - 1;
				f.close();
				str1.sprintf(tr("Scan %d: "), WavelengthFile.size()+1);
				str1 += fileName;
				cmb_scans->insertItem(str1);
			}
			else
			{
				QMessageBox::message(tr("UltraScan Error:"),	tr("The wavelength file:\n\n")
																		+ es.filename
																		+ tr("\n\ncannot be read.\n\n"
																		"Please check to make sure that you have\n"
																		"read access to this file."));
			}
			WavelengthFile.push_back(es);
		}
		else
		{
			return;
		}
		pb_resetList->setEnabled(true);
		pb_save->setEnabled(true);
	}
	plot();
}

void US_EditWavelengthScan::plot()
{
	double *x, *y;
	unsigned int count = 0, points;
	points = WavelengthFile[currentScan].stopLineIndex + 1 - WavelengthFile[currentScan].startLineIndex;
	x = new double [points];
	y = new double [points];
	for (unsigned int i=WavelengthFile[currentScan].startLineIndex; i<=WavelengthFile[currentScan].stopLineIndex; i++)
	{
		x[count] = (double) WavelengthFile[currentScan].wavelength[i];
		y[count] = (double) WavelengthFile[currentScan].concentration[i];
		count ++;
	}
	unsigned int curve;
	wavelength_plot->clear();
	QwtSymbol symbol;
	symbol.setSize(8);
	symbol.setStyle(QwtSymbol::Ellipse);
	symbol.setPen(Qt::blue);
	symbol.setBrush(Qt::yellow);
	curve = wavelength_plot->insertCurve("Extinction");
	wavelength_plot->setCurveData(curve, x, y, points);
	wavelength_plot->setCurveYAxis(curve, QwtPlot::yLeft);
	wavelength_plot->setCurveSymbol(curve, symbol);
	wavelength_plot->setCurvePen(curve, QPen(Qt::cyan, 2, SolidLine));
	wavelength_plot->replot();
}

void US_EditWavelengthScan::resetList()
{
	cmb_scans->clear();
	WavelengthFile.clear();
	wavelength_plot->clear();
	wavelength_plot->replot();
	wavelength_plot->enableOutline(false);
	pb_resetList->setEnabled(false);
	pb_next->setEnabled(false);
	pb_save->setEnabled(false);
	pb_resetScan->setEnabled(false);
}

void US_EditWavelengthScan::resetScan()
{
	WavelengthFile[currentScan].startLineIndex = 0;
	WavelengthFile[currentScan].stopLineIndex = WavelengthFile[currentScan].concentration.size()-1;
	pb_resetScan->setEnabled(false);
	plot();
}

void US_EditWavelengthScan::saveScan()
{
	QString filename;
	if (target == "" && copyFlag == true)
	{
		selectTarget();
	}
	if (copyFlag)
	{
		QString str1;
		if(currentScan < 9)
		{
			str1.sprintf("0000%d.wa", currentScan + 1);
		}
		else if(currentScan > 8 && currentScan < 99)
		{
			str1.sprintf("000%d.wa", currentScan + 1);
		}
		else if(currentScan > 98 && currentScan < 999)
		{
			str1.sprintf("00%d.wa", currentScan + 1);
		}
		else if(currentScan > 998 && currentScan < 9999)
		{
			str1.sprintf("0%d.wa", currentScan + 1);
		}
		else
		{
			str1.sprintf("%d.wa", currentScan + 1);
		}

    int last = WavelengthFile[currentScan].filename.length() - 1;

		filename = target + str1 + 
      WavelengthFile[currentScan].filename[last];
	}
	else
	{
		filename = WavelengthFile[currentScan].path + "/" + WavelengthFile[currentScan].filename;
	}
	QFile f;
	f.setName(filename);
	if (f.exists())
	{
		f.remove();
	}
	if (f.open(IO_WriteOnly))
	{
		QTextStream ts(&f);
		ts << WavelengthFile[currentScan].header1 << endl;
		ts << WavelengthFile[currentScan].header2 << endl;
		for (unsigned int i=WavelengthFile[currentScan].startLineIndex;
			  i<=WavelengthFile[currentScan].stopLineIndex; i++)
		{
			ts << WavelengthFile[currentScan].line[i] << endl;
		}
		f.close();
	}
	if (currentScan != (int) WavelengthFile.size()-1)
	{
		pb_next->setEnabled(true);
	}
}

void US_EditWavelengthScan::nextScan()
{
	if (currentScan != (int) WavelengthFile.size()-1)
	{
		currentScan ++;
		showScan(currentScan);
	}
	cmb_scans->setCurrentItem(currentScan);
	pb_next->setEnabled(false);
}

void US_EditWavelengthScan::showScan(int val)
{
	currentScan = (unsigned int) val;
	plot();
}

void US_EditWavelengthScan::deleteScan(int val)
{
	val = 0;
}

void US_EditWavelengthScan::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/editwavelength.html");
}

void US_EditWavelengthScan::selectCopy()
{
	pb_selectTarget->setEnabled(true);
	cb_overwrite->setChecked(false);
	cb_copy->setChecked(true);
	copyFlag = true;
}

void US_EditWavelengthScan::selectOverwrite()
{
	pb_selectTarget->setEnabled(false);
	cb_overwrite->setChecked(true);
	cb_copy->setChecked(false);
	copyFlag = false;
}

void US_EditWavelengthScan::mouseReleased(const QMouseEvent &e)
{
	float current_x = (wavelength_plot->invTransform(QwtPlot::xBottom, e.x()));
	float start_x, stop_x, half_x;
	unsigned int i, current_index;
	QwtSymbol symbol;
	if (mouse_was_moved)
	{
		pb_resetScan->setEnabled(true);
		mouse_was_moved = false;
		start_x = WavelengthFile[currentScan].wavelength[WavelengthFile[currentScan].startLineIndex];
		stop_x  = WavelengthFile[currentScan].wavelength[WavelengthFile[currentScan].stopLineIndex];
		half_x  = start_x + (stop_x - start_x)/2;
		if (current_x <= start_x || current_x >= stop_x) // user released mouse outside the scan range
		{
			return;
		}
		i = 0;
		while (WavelengthFile[currentScan].wavelength[i] < current_x) 
		{
			i ++;
		}
		current_index = i;
		if (current_x >= half_x)	// we are on the upper portion, clip data above
		{
			WavelengthFile[currentScan].stopLineIndex = i - 1;
		}
		else
		{
			WavelengthFile[currentScan].startLineIndex = i;
		}
	}
	plot();
}

void US_EditWavelengthScan::mouseMoved(const QMouseEvent &e)
{
	float current_x = (wavelength_plot->invTransform(QwtPlot::xBottom, e.x()));
	float start_x, stop_x, half_x;
	unsigned int i, curve_yellow = 0, curve_red = 0, current_index, yellow_points=0, red_points=0;
	double *x_temp1, *y_temp1, *x_temp2, *y_temp2;
	QwtSymbol symbol;

	start_x = WavelengthFile[currentScan].wavelength[WavelengthFile[currentScan].startLineIndex];
	stop_x  = WavelengthFile[currentScan].wavelength[WavelengthFile[currentScan].stopLineIndex];
	half_x  = start_x + (stop_x - start_x)/2;
	if (current_x <= start_x || current_x >= stop_x) // user clicked outside the scan range
	{
		mouse_was_moved = false;
		plot();
		return;
	}
	else
	{
		i = 0;
		while (WavelengthFile[currentScan].wavelength[i] < current_x) 
		{
			i ++;
		}
		current_index = i;
	}
	if (current_x >= half_x)	// we are on the upper portion
	{
		red_points = WavelengthFile[currentScan].stopLineIndex - current_index + 1;
		yellow_points = current_index - WavelengthFile[currentScan].startLineIndex;
	}
	else
	{
		red_points = current_index - WavelengthFile[currentScan].startLineIndex;
		yellow_points = WavelengthFile[currentScan].stopLineIndex - current_index + 1;
	}
	x_temp1 = new double [yellow_points];
	y_temp1 = new double [yellow_points];
	x_temp2 = new double [red_points];
	y_temp2 = new double [red_points];

	if (current_x >= half_x)	// we are on the upper portion
	{
		for (i=0; i<yellow_points; i++)
		{
			x_temp1[i] = (double) WavelengthFile[currentScan].wavelength[i + WavelengthFile[currentScan].startLineIndex];	//need to promote to double for plotting
			y_temp1[i] = (double) WavelengthFile[currentScan].concentration[i + WavelengthFile[currentScan].startLineIndex];	//need to promote to double for plotting
		}
		for (i=0; i<red_points; i++)
		{
			x_temp2[i] = (double) WavelengthFile[currentScan].wavelength[i+current_index];	//need to promote to double for plotting
			y_temp2[i] = (double) WavelengthFile[currentScan].concentration[i+current_index];	//need to promote to double for plotting
		}
	}
	else // we are on the lower half
	{
		for (i=0; i<yellow_points; i++)
		{
			x_temp1[i] = (double) WavelengthFile[currentScan].wavelength[i+current_index];	//need to promote to double for plotting
			y_temp1[i] = (double) WavelengthFile[currentScan].concentration[i+current_index];	//need to promote to double for plotting
		}
		for (i=0; i<red_points; i++)
		{
			x_temp2[i] = (double) WavelengthFile[currentScan].wavelength[i + WavelengthFile[currentScan].startLineIndex];	//need to promote to double for plotting];	//need to promote to double for plotting
			y_temp2[i] = (double) WavelengthFile[currentScan].concentration[i + WavelengthFile[currentScan].startLineIndex];	//need to promote to double for plotting];	//need to promote to double for plotting
		}
	}

	wavelength_plot->clear();
	curve_yellow = wavelength_plot->insertCurve("curve_yellow");
	curve_red = wavelength_plot->insertCurve("curve_red");
	symbol.setSize(8);
	symbol.setStyle(QwtSymbol::Ellipse);
	symbol.setPen(Qt::blue);
	symbol.setBrush(Qt::yellow);
	wavelength_plot->setCurveData(curve_yellow, x_temp1, y_temp1, yellow_points);
	wavelength_plot->setCurvePen(curve_yellow, QPen(Qt::cyan, 2, SolidLine));
	wavelength_plot->setCurveSymbol(curve_yellow, symbol);
	symbol.setPen(Qt::white);
	symbol.setBrush(Qt::red);
	wavelength_plot->setCurveData(curve_red, x_temp2, y_temp2, red_points);
	wavelength_plot->setCurvePen(curve_red, QPen(Qt::red, 2, SolidLine));
	wavelength_plot->setCurveSymbol(curve_red, symbol);
	wavelength_plot->replot();
	delete [] x_temp1;
	delete [] y_temp1;
	delete [] x_temp2;
	delete [] y_temp2;
	mouse_was_moved = true;
}
