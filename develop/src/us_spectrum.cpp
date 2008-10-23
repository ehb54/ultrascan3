#include "../include/us_spectrum.h"

US_Spectrum::US_Spectrum(QWidget *p, const char* name) : QFrame(p, name)
{
	basis.clear();
	curve.clear();
	curve.resize(1);
	lambda_min.clear();
	lambda_max.clear();
	target.curve_number = -1;
	solution_curve = -1;
	unsigned int minHeight1=26, minHeight2=30;
	USglobal = new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("Spectrum Fitting Module"));

	lb_basis = new QListBox(this, "Listing of Basis Vectors" );
	lb_basis->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_basis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	lb_basis->setMinimumHeight(100);
	connect(lb_basis, SIGNAL(selected(int)), this, SLOT(edit_basis(int)));

	lb_target = new QListBox(this, "Listing of Target Vector" );
	lb_target->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	lb_target->setMinimumHeight(minHeight2);
//	lb_target->setMaximumHeight(minHeight2);
	connect(lb_target, SIGNAL(selected(int)), this, SLOT(edit_target(int)));
	
	lbl_info = new QLabel(tr("(Double-click to Edit Scaling)"), this);
	lbl_info->setAlignment(AlignHCenter|AlignVCenter);
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setMinimumHeight(minHeight2);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

	lbl_extinction = new QLabel(tr(" Extinction Coeff.: "), this);
	lbl_extinction->setAlignment(AlignLeft|AlignVCenter);
	lbl_extinction->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_extinction->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_extinction->setMinimumHeight(minHeight1);

	lbl_wavelength = new QLabel(tr(" Wavelength: "), this);
	lbl_wavelength->setAlignment(AlignLeft|AlignVCenter);
	lbl_wavelength->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_wavelength->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_wavelength->setMinimumHeight(minHeight1);

	le_rmsd = new QLineEdit(this, "RMSD Line Edit");
	le_rmsd->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_rmsd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_rmsd->setMinimumHeight(minHeight1);
	le_rmsd->setAlignment(AlignCenter|AlignVCenter);
	le_rmsd->setReadOnly(true);

	le_extinction = new QLineEdit(this, "Extinction Line Edit");
	le_extinction->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_extinction->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_extinction->setMinimumHeight(minHeight1);
	le_extinction->setEnabled(false);

	le_wavelength = new QLineEdit(this, "wavelength Line Edit");
	le_wavelength->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_wavelength->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_wavelength->setMinimumHeight(minHeight1);
	le_wavelength->setEnabled(false);
	
	pb_update = new QPushButton( tr("Update Extinction Scaling"), this );
	pb_update->setAutoDefault(false);
	pb_update->setEnabled(false);
	pb_update->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_update->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_update->setMinimumHeight(minHeight1);
	connect(pb_update, SIGNAL(clicked()), SLOT(update_scale()) );

	pb_difference = new QPushButton( tr("Difference Spectrum"), this );
	pb_difference->setAutoDefault(false);
	pb_difference->setEnabled(false);
	pb_difference->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_difference->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_difference->setMinimumHeight(minHeight1);
	connect(pb_difference, SIGNAL(clicked()), SLOT(difference()) );

	pb_delete = new QPushButton( tr("Delete Current Basis Scan"), this );
	pb_delete->setAutoDefault(false);
	pb_delete->setEnabled(false);
	pb_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_delete->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_delete->setMinimumHeight(minHeight1);
	connect(pb_delete, SIGNAL(clicked()), SLOT(delete_scan()) );

	pb_load_target = new QPushButton( tr("Load Target Spectrum"), this );
	pb_load_target->setAutoDefault(false);
	pb_load_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load_target->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_load_target->setMinimumHeight(minHeight1);
	connect(pb_load_target, SIGNAL(clicked()), SLOT(load_target()) );

	pb_load_basis = new QPushButton( tr("Load Basis Spectrum"), this );
	pb_load_basis->setAutoDefault(false);
	pb_load_basis->setEnabled(false);
	pb_load_basis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load_basis->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_load_basis->setMinimumHeight(minHeight1);
	connect(pb_load_basis, SIGNAL(clicked()), SLOT(load_basis()) );

	pb_load_fit = new QPushButton( tr("Load Fit"), this );
	pb_load_fit->setAutoDefault(false);
	pb_load_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_load_fit->setMinimumHeight(minHeight1);
	connect(pb_load_fit, SIGNAL(clicked()), SLOT(load_fit()) );

	pb_extrapolate = new QPushButton( tr("Extrapolate Extinction Profile"), this );
	pb_extrapolate->setEnabled(false);
	pb_extrapolate->setAutoDefault(false);
	pb_extrapolate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_extrapolate->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_extrapolate->setMinimumHeight(minHeight1);
	connect(pb_extrapolate, SIGNAL(clicked()), SLOT(extrapolate()) );

	pb_overlap = new QPushButton( tr("Find Extinction Profile Overlap"), this );
	pb_overlap->setAutoDefault(false);
	pb_overlap->setEnabled(false);
	pb_overlap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_overlap->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_overlap->setMinimumHeight(minHeight1);
	connect(pb_overlap, SIGNAL(clicked()), SLOT(overlap()) );

	pb_fit = new QPushButton( tr("Fit Data"), this );
	pb_fit->setAutoDefault(false);
	pb_fit->setEnabled(false);
	pb_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_fit->setMinimumHeight(minHeight1);
	connect(pb_fit, SIGNAL(clicked()), SLOT(fit()) );

	pb_help = new QPushButton( tr("Help"), this );
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setMinimumHeight(minHeight1);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()) );

	pb_reset_basis = new QPushButton( tr("Reset Basis Spectra"), this );
	pb_reset_basis->setAutoDefault(false);
	pb_reset_basis->setEnabled(false);
	pb_reset_basis->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset_basis->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_reset_basis->setMinimumHeight(minHeight1);
	connect(pb_reset_basis, SIGNAL(clicked()), SLOT(reset_basis()) );

	pb_save = new QPushButton( tr("Save Fit"), this );
	pb_save->setAutoDefault(false);
	pb_save->setEnabled(false);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_save->setMinimumHeight(minHeight1);
	connect(pb_save, SIGNAL(clicked()), SLOT(save()) );

	pb_print_fit = new QPushButton( tr("Print Fit"), this );
	pb_print_fit->setAutoDefault(false);
	pb_print_fit->setEnabled(false);
	pb_print_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_print_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_print_fit->setMinimumHeight(minHeight1);
	connect(pb_print_fit, SIGNAL(clicked()), SLOT(print_fit()) );

	pb_print_residuals = new QPushButton( tr("Print Residuals"), this );
	pb_print_residuals->setAutoDefault(false);
	pb_print_residuals->setEnabled(false);
	pb_print_residuals->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_print_residuals->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_print_residuals->setMinimumHeight(minHeight1);
	connect(pb_print_residuals, SIGNAL(clicked()), SLOT(print_residuals()) );

	pb_close = new QPushButton( tr("Close"), this );
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setMinimumHeight(minHeight1);
	connect(pb_close, SIGNAL(clicked()), SLOT(close()) );

	data_plot = new QwtPlot(this);
	data_plot->setTitle(tr("Wavelength Spectrum Fit"));
	data_plot->enableGridXMin();
	data_plot->enableGridYMin();
	data_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	data_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	data_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	data_plot->setCanvasBackground(USglobal->global_colors.plot);		//new version
	data_plot->setAxisTitle(QwtPlot::xBottom, tr("Wavelength (nm)"));
	data_plot->setAxisTitle(QwtPlot::yLeft, tr("Extinction"));
//	data_plot->enableOutline(true);
//	data_plot->setOutlinePen(white);
//	data_plot->setOutlineStyle(Qwt::Cross);
	data_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	data_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));		
	data_plot->setMargin(USglobal->config_list.margin);
	data_plot->setMinimumSize(600, 275);
	connect(data_plot, SIGNAL(plotMousePressed(const QMouseEvent &)), SLOT(select_basis(const QMouseEvent&)));

	residuals_plot = new QwtPlot(this);
	residuals_plot->setTitle(tr("Fitting Residuals"));
	residuals_plot->enableGridXMin();
	residuals_plot->enableGridYMin();
	residuals_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	residuals_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	residuals_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	residuals_plot->setCanvasBackground(USglobal->global_colors.plot);		//new version
	residuals_plot->setAxisTitle(QwtPlot::xBottom, tr("Wavelength (nm)"));
	residuals_plot->setAxisTitle(QwtPlot::yLeft, tr("Extinction"));
	residuals_plot->enableOutline(true);
	residuals_plot->setOutlinePen(white);
	residuals_plot->setOutlineStyle(Qwt::Cross);
	residuals_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	residuals_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	residuals_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	residuals_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	residuals_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	residuals_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	residuals_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));		
	residuals_plot->setMargin(USglobal->config_list.margin);
	residuals_plot->setMinimumSize(600, 275);

	global_Xpos += 30;
	global_Ypos += 30;
	setup_GUI();
}

US_Spectrum::~US_Spectrum()
{
}

void US_Spectrum::setup_GUI()
{
	unsigned int j=0;
	QRect r;
	QGridLayout *topBox = new QGridLayout(this, 2, 2, 4, 2);
	topBox->setColStretch(0, 0);
	topBox->setColStretch(1, 1);
	topBox->setRowStretch(0, 3);
	topBox->setRowStretch(1, 2);
	QGridLayout *buttonBox = new QGridLayout(10, 2, 2);
	buttonBox->setRowStretch(1, 0);
	buttonBox->setRowStretch(3, 1);
	buttonBox->setRowSpacing(1, 26);
	buttonBox->addMultiCellWidget(pb_load_target, j, j, 0, 1, 0);
	j++;
	buttonBox->addMultiCellWidget(lb_target, j, j, 0, 1, 0);
	j++;
	buttonBox->addMultiCellWidget(pb_load_basis, j, j, 0, 1, 0);
	j++;
	buttonBox->addMultiCellWidget(lb_basis, j, j, 0, 1, 0);
	j++;
	buttonBox->addMultiCellWidget(lbl_info, j, j, 0, 1, 0);
	j++;
	buttonBox->addWidget(lbl_wavelength, j, 0, 0);
	buttonBox->addWidget(le_wavelength, j, 1, 0);
	j++;
	buttonBox->addWidget(lbl_extinction, j, 0, 0);
	buttonBox->addWidget(le_extinction, j, 1, 0);
	j++;
	buttonBox->addMultiCellWidget(pb_update, j, j, 0, 1, 0);
	j++;
	buttonBox->addMultiCellWidget(pb_delete, j, j, 0, 1, 0);
	j++;
	buttonBox->addMultiCellWidget(pb_reset_basis, j, j, 0, 1, 0);
	j++;
	buttonBox->addMultiCellWidget(pb_overlap, j, j, 0, 1, 0);
	j++;
	buttonBox->addMultiCellWidget(pb_extrapolate, j, j, 0, 1, 0);
	j++;
	buttonBox->addMultiCellWidget(pb_difference, j, j, 0, 1, 0);
	j++;
	buttonBox->addMultiCellWidget(pb_fit, j, j, 0, 1, 0);
	j++;
	buttonBox->addMultiCellWidget(le_rmsd, j, j, 0, 1, 0);
	j++;
	buttonBox->addWidget(pb_print_residuals, j, 0, 0);
	buttonBox->addWidget(pb_print_fit, j, 1, 0);
	j++;
	buttonBox->addWidget(pb_load_fit, j, 0, 0);
	buttonBox->addWidget(pb_save, j, 1, 0);
	j++;
	buttonBox->addWidget(pb_help, j, 0, 0);
	buttonBox->addWidget(pb_close, j, 1, 0);
	r = buttonBox->cellGeometry(1, 0);

	setGeometry(global_Xpos, global_Ypos, r.width()+10+600, 550+6+8);
	
	topBox->addMultiCellLayout(buttonBox, 0, 1, 0, 0, Qt::AlignTop|Qt::AlignLeft);
	topBox->addWidget(data_plot, 0, 1, 0);
	topBox->addWidget(residuals_plot, 1, 1, 0);

	qApp->processEvents();
	r = topBox->cellGeometry(0, 0);
	this->setGeometry(global_Xpos, global_Ypos, r.width()+10+600, 550+6+8);
}

void US_Spectrum::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

void US_Spectrum::printError(const int &ival)
{
	switch (ival)
	{
		case 0:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr("Please note:\n\nThere was an error reading\nthe selected Model File!\n\nThis file appears to be corrupted.\n\nPlease recreate the wavelength fit before loading."),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
	}
}

void US_Spectrum::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/us_spectrum.html");
}

void US_Spectrum::load_fit()
{
	unsigned int i, j, k, points;
	QFile f;
	QPen pen;
	double *x, *y;
	QString str, fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.spectrum_fit", 0);
	if (fn.isEmpty())
	{
		return;
	}
	target.gaussian.clear();
	for (i=0; i<basis.size(); i++)
	{
		basis[i].gaussian.clear();
	}
	curve.clear();
	curve.resize(1);
	lambda_min.clear();
	lambda_max.clear();
	target.curve_number = -1;
	solution_curve = -1;
	basis.clear();
	lb_basis->clear();
	lb_target->clear();
	le_rmsd->setText("");
	pb_extrapolate->setEnabled(true);
	pb_overlap->setEnabled(true);
	pb_fit->setEnabled(true);
	pb_difference->setEnabled(true);
	pb_save->setEnabled(true);
	pb_load_basis->setEnabled(true);
	pb_reset_basis->setEnabled(true);
	pb_print_fit->setEnabled(true);
	pb_print_residuals->setEnabled(true);
	reset_edit_gui();

	f.setName(fn);
	if (f.open(IO_ReadOnly))
	{
		QDataStream ds(&f);
		ds >> target.amplitude;
		ds >> target.filename;
		ds >> target.lambda_min;
		ds >> target.lambda_max;
		ds >> target.lambda_scale;
		ds >> target.scale;
		ds >> i;
		target.gaussian.resize(i);
		for (i=0; i<target.gaussian.size(); i++)
		{
			ds >> target.gaussian[i].amplitude;
			ds >> target.gaussian[i].sigma;
			ds >> target.gaussian[i].mean;
		}
		ds >> j;
		basis.resize(j);
		for (j=0; j<basis.size(); j++)
		{
			ds >> basis[j].amplitude;
			ds >> basis[j].filename;
			ds >> basis[j].lambda_min;
			ds >> basis[j].lambda_max;
			ds >> basis[j].lambda_scale;
			ds >> basis[j].scale;
			ds >> basis[j].nnls_factor;
			ds >> basis[j].nnls_percentage;
			ds >> i;
			basis[j].gaussian.resize(i);
			for (i=0; i<basis[j].gaussian.size(); i++)
			{
				ds >> basis[j].gaussian[i].amplitude;
				ds >> basis[j].gaussian[i].sigma;
				ds >> basis[j].gaussian[i].mean;
			}
		}
		f.close();
	}
	QFileInfo fi;
	fi.setFile(target.filename);
	lb_target->insertItem(fi.baseName());
	points = target.lambda_max - target.lambda_min + 1;
	x = new double [points];
	y = new double [points];
	find_amplitude(target);
	for (i=0; i<points; i++)
	{
		x[i] = target.lambda_min + i;
		y[i] = 0.0;
		for (j=0; j<target.gaussian.size(); j++)
		{
			y[i] += target.gaussian[j].amplitude *
			exp(-(pow(x[i] - target.gaussian[j].mean, 2.0)
			/ (2.0 * pow(target.gaussian[j].sigma, 2.0))));
		}
		y[i] *= target.amplitude;
	}
	if (target.curve_number != -1)
	{
		data_plot->removeCurve(target.curve_number);
	}
	pen.setColor(Qt::yellow);
	pen.setWidth(3);
	curve.resize(1);
	curve[0] = data_plot->insertCurve(fi.baseName());
	target.curve_number = (int) curve[0];
	data_plot->setCurveStyle(curve[0], QwtCurve::Lines);
	data_plot->setCurveData(curve[0], x, y, points);
	data_plot->setCurvePen(curve[0], pen);
	pen.setColor(Qt::green);
	pen.setWidth(2);
	
	for (k=0; k<basis.size(); k++)
	{
		fi.setFile(basis[k].filename);
		lb_basis->insertItem(fi.baseName(), -1);
		find_amplitude(basis[k]);
		for (i=0; i<points; i++)
		{
			y[i] = 0.0;
			for (j=0; j<basis[k].gaussian.size(); j++)
			{
				y[i] += basis[k].gaussian[j].amplitude *
				exp(-(pow(x[i] - basis[k].gaussian[j].mean, 2.0)
				/ (2.0 * pow(basis[k].gaussian[j].sigma, 2.0))));
			}
			y[i] *= basis[k].amplitude;
		}
		curve.push_back(data_plot->insertCurve(fi.baseName()));
		basis[k].curve_number = curve[k+1];
		data_plot->setCurveStyle(curve[k+1], QwtCurve::Lines);
		data_plot->setCurveData(curve[k+1], x, y, points);
		data_plot->setCurvePen(curve[k+1], pen);
		data_plot->replot();
	}
	pb_extrapolate->setEnabled(true);
	pb_overlap->setEnabled(true);
	pb_fit->setEnabled(true);
	pb_save->setEnabled(true);
	pb_difference->setEnabled(true);
	pb_save->setEnabled(false);
	pb_reset_basis->setEnabled(true);
	le_rmsd->setText("");
	data_plot->replot();
	fit();
	delete [] x;
	delete [] y;
}

void US_Spectrum::save()
{
	unsigned int i, j;
	int k;
	QFile f;
	QString fn = QFileDialog::getSaveFileName(USglobal->config_list.result_dir, "*.spectrum_fit", 0);
	QFileInfo fi;
	if (fn.isEmpty())
	{
		return;
	}
	k = fn.findRev(".", -1, false);
	if (k != -1) //if an extension was given, strip it
	{
		fn.truncate(k);
	}
	fn = fn + ".spectrum_fit";
	f.setName(fn);
	if (f.open(IO_WriteOnly))
	{
		QDataStream ds(&f);
		ds << target.amplitude;
		ds << target.filename;
		ds << target.lambda_min;
		ds << target.lambda_max;
		ds << target.lambda_scale;
		ds << target.scale;
		ds << target.gaussian.size();
		for (i=0; i<target.gaussian.size(); i++)
		{
			ds << target.gaussian[i].amplitude;
			ds << target.gaussian[i].sigma;
			ds << target.gaussian[i].mean;
		}
		ds << basis.size();
		for (j=0; j<basis.size(); j++)
		{
			ds << basis[j].amplitude;
			ds << basis[j].filename;
			ds << basis[j].lambda_min;
			ds << basis[j].lambda_max;
			ds << basis[j].lambda_scale;
			ds << basis[j].scale;
			ds << basis[j].nnls_factor;
			ds << basis[j].nnls_percentage;
			ds << basis[j].gaussian.size();
			for (i=0; i<basis[j].gaussian.size(); i++)
			{
				ds << basis[j].gaussian[i].amplitude;
				ds << basis[j].gaussian[i].sigma;
				ds << basis[j].gaussian[i].mean;
			}
		}
		f.close();
	}
	fi.setFile(target.filename);
	lb_target->insertItem(fi.baseName());
}

void US_Spectrum::reset_basis()
{
	unsigned int i, j, points;
	double *x, *y;
	for (i=0; i<basis.size(); i++)
	{
		basis[i].gaussian.clear();
		data_plot->removeCurve(basis[i].curve_number);
	}
	basis.clear();
	lb_basis->clear();
	data_plot->clear();
	residuals_plot->clear();
	residuals_plot->replot();
	QPen pen;
	points = target.lambda_max - target.lambda_min + 1;
	x = new double [points];
	y = new double [points];
	pen.setWidth(3);
	pen.setColor(Qt::yellow);
	//find_amplitude(target);
	for (i=0; i<points; i++)
	{
		x[i] = target.lambda_min + i;
		y[i] = 0.0;
		for (j=0; j<target.gaussian.size(); j++)
		{
			y[i] += target.gaussian[j].amplitude *
			exp(-(pow(x[i] - target.gaussian[j].mean, 2.0)
			/ (2.0 * pow(target.gaussian[j].sigma, 2.0))));
		}
		y[i] *= target.amplitude;
	}
	curve.clear();
	curve.push_back(data_plot->insertCurve(target.filename));
	data_plot->setCurveStyle(target.curve_number, QwtCurve::Lines);
	data_plot->setCurveData(target.curve_number, x, y, points);
	data_plot->setCurvePen(target.curve_number, pen);
	data_plot->replot();
	pb_extrapolate->setEnabled(false);
	pb_overlap->setEnabled(false);
	pb_fit->setEnabled(false);
	pb_save->setEnabled(false);
	pb_difference->setEnabled(false);
	pb_reset_basis->setEnabled(false);
	pb_print_residuals->setEnabled(false);
	le_rmsd->setText("");
	delete [] x;
	delete [] y;
}

void US_Spectrum::edit_target(int val)
{
	QString str;
	le_wavelength->setEnabled(true);
	le_extinction->setEnabled(true);
	pb_delete->setEnabled(false);
	pb_update->setEnabled(true);
	current_scan = target.curve_number;
	le_wavelength->setText(str.sprintf("%3d", target.lambda_scale));
	le_extinction->setText(str.sprintf("%6.4e", target.scale));
	val = val *2;
}

void US_Spectrum::edit_basis(int val)
{
	QString str;
	le_wavelength->setEnabled(true);
	le_extinction->setEnabled(true);
	pb_delete->setEnabled(true);
	pb_update->setEnabled(true);
	current_scan = basis[val].curve_number;
	le_wavelength->setText(str.sprintf("%3d", basis[val].lambda_scale));
	le_extinction->setText(str.sprintf("%6.4e", basis[val].scale));
}

void US_Spectrum::reset_edit_gui()
{
	le_wavelength->setText("");
	le_extinction->setText("");
	le_wavelength->setEnabled(false);
	le_extinction->setEnabled(false);
	pb_delete->setEnabled(false);
	pb_update->setEnabled(false);
}

void US_Spectrum::delete_basis(int val)
{
	basis[val].gaussian.clear();
	data_plot->removeCurve(basis[val].curve_number);
	vector <struct WavelengthProfile>::iterator it=basis.begin();
	basis.erase(it + val);
	lb_basis->removeItem(val);
	data_plot->replot();
	reset_edit_gui();
	if (basis.size() < 1)
	{
		pb_extrapolate->setEnabled(false);
		pb_overlap->setEnabled(false);
		pb_fit->setEnabled(false);
		pb_difference->setEnabled(false);
		pb_save->setEnabled(false);
		pb_reset_basis->setEnabled(false);
		pb_print_residuals->setEnabled(false);
	}
	le_rmsd->setText("");
}

void US_Spectrum::load_gaussian_profile(struct WavelengthProfile &profile, const QString &fileName)
{
	QString line, str1, str2;
	struct Gaussian temp_gauss;
	unsigned int i, order;
	QFileInfo fi;
	fi.setFile(fileName);
	profile.filenameBasis = fi.baseName();
	QFile f(fileName);
	profile.gaussian.clear();
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		for (i=0; i<3; i++)
		{
			line = ts.readLine();
		}
		str1 = getToken(&line, ":");
		order = line.toUInt();
		ts.readLine();
		line = ts.readLine();
		str1 = getToken(&line, " "); // Extinction
		str1 = getToken(&line, " "); // Coefficient
		str1 = getToken(&line, " "); // at
		str1 = getToken(&line, " "); // wavelength
		profile.lambda_scale = str1.toUInt(); // wavelength at which the data is normalized for an extinction coefficient
		str1 = getToken(&line, ":");
		profile.scale = line.toFloat(); // extinction coefficient used for normalization
		line = ts.readLine();
		line = ts.readLine();
		for (i=0; i<order; i++)
		{
			if (!ts.atEnd()) ts.readLine();
			if (!ts.atEnd()) ts.readLine();
			if (!ts.atEnd()) line = ts.readLine();
			str1 = getToken(&line, ":");
			line.stripWhiteSpace();
			str1 = getToken(&line, " ");
			temp_gauss.mean = str1.toFloat();
			if (!ts.atEnd()) line = ts.readLine();
			str1 = getToken(&line, ":");
			line.stripWhiteSpace();
			str1 = getToken(&line, " ");
			temp_gauss.amplitude = str1.toFloat();
			if (!ts.atEnd()) line = ts.readLine();
			str1 = getToken(&line, ":");
			line.stripWhiteSpace();
			str1 = getToken(&line, " ");
			temp_gauss.sigma = str1.toFloat();
			if (temp_gauss.mean == 0.0 || temp_gauss.amplitude == 0.0 || temp_gauss.sigma == 0.0)
			{	
				printError(0);
				return;
			}
			else
			{
				profile.gaussian.push_back(temp_gauss);
			}
		}
		f.close();
	}
	profile.filename = fileName;
	QString datfile = fi.dirPath() + "/" + fi.baseName() + ".extinction.dat";
	f.setName(datfile);
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		ts.readLine();
		ts >> str1;
		float tmp = str1.toFloat();
		profile.lambda_min = (int) tmp;
		while (!ts.atEnd())
		{
			str1 = ts.readLine(); // read last line in absorption spectrum
		}
		str2 = getToken(&str1, "\t");
		profile.lambda_max = (int) str2.toFloat();
	}
	else
	{
		QMessageBox::message("Attention:", "Could not read the wavelength data file:\n" + datfile );
	}
	find_amplitude(profile);
	print_profile(profile);
}

void US_Spectrum::select_basis(const QMouseEvent &e)
{
	QString str;
	int dist;
	unsigned int i, j, points, val;
	double *x, *y;
	bool flag=false;
	QPen pen;
	pen.setColor(Qt::red);
	val = data_plot->closestCurve(e.x(), e.y(), dist);
	for (i = 0; i<basis.size(); i++)
	{
		if (val == (unsigned int) basis[i].curve_number)
		{
			flag = true;
			break;
		}
	}
	if (flag)
	{
		pen.setWidth(2);
		val = i;
		lb_basis->setSelected(i, true);
		points = basis[val].lambda_max - basis[val].lambda_min + 1;
		x = new double [points];
		y = new double [points];
		//find_amplitude(basis[val]);
		for (i=0; i<points; i++)
		{
			x[i] = basis[val].lambda_min + i;
			y[i] = 0.0;
			for (j=0; j<basis[val].gaussian.size(); j++)
			{
				y[i] += basis[val].gaussian[j].amplitude *
				exp(-(pow(x[i] - basis[val].gaussian[j].mean, 2.0)
				/ (2.0 * pow(basis[val].gaussian[j].sigma, 2.0))));
			}
			y[i] *= basis[val].amplitude;
		}
		data_plot->setCurveStyle(basis[val].curve_number, QwtCurve::Lines);
		data_plot->setCurveData(basis[val].curve_number, x, y, points);
		data_plot->setCurvePen(basis[val].curve_number, pen);
		data_plot->replot();
		QFileInfo fi;
		fi.setFile(basis[val].filename);
		str.sprintf(fi.dirPath() + "/" + fi.baseName() + ":\n\nMinimum Wavelength: %3d nm\n"
		+ "Maximum Wavelength: %3d nm\n" 
		+ "Extinction Coefficient Scale at %3d nm: %6.4e", 
		basis[val].lambda_min, basis[val].lambda_max, basis[val].lambda_scale, basis[val].scale);
		QMessageBox::information(this, tr("UltraScan Information"),
		tr("Information for this plot:\n\n" 
		+ str),
		QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		pen.setColor(Qt::green);
		data_plot->setCurvePen(basis[val].curve_number, pen);
		data_plot->replot();
		delete [] x;
		delete [] y;
	}
	else if (val == (unsigned int) target.curve_number) // we clicked on the target plot
	{
		pen.setWidth(3);
		points = target.lambda_max - target.lambda_min + 1;
		x = new double [points];
		y = new double [points];
		find_amplitude(target);
		for (i=0; i<points; i++)
		{
			x[i] = target.lambda_min + i;
			y[i] = 0.0;
			for (j=0; j<target.gaussian.size(); j++)
			{
				y[i] += target.gaussian[j].amplitude *
				exp(-(pow(x[i] - target.gaussian[j].mean, 2.0)
				/ (2.0 * pow(target.gaussian[j].sigma, 2.0))));
			}
			y[i] *= target.amplitude;
		}
		data_plot->setCurveStyle(target.curve_number, QwtCurve::Lines);
		data_plot->setCurveData(target.curve_number, x, y, points);
		data_plot->setCurvePen(target.curve_number, pen);
		data_plot->replot();
		QFileInfo fi;
		fi.setFile(target.filename);
		str.sprintf(fi.dirPath() + "/" + fi.baseName() + ":\n\nMinimum Wavelength: %3d nm\n"
		+ "Maximum Wavelength: %3d nm\n" 
		+ "Extinction Coefficient Scale at %3d nm: %6.4e ", 
		target.lambda_min, target.lambda_max, target.lambda_scale, target.scale);
		QMessageBox::information(this, tr("UltraScan Information"),
		tr("Information for this plot:\n\n" 
		+ str),
		QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		pen.setColor(Qt::yellow);
		data_plot->setCurvePen(target.curve_number, pen);
		data_plot->replot();
		delete [] x;
		delete [] y;
	}
	else // we clicked on some other plot.
	{
		QMessageBox::information(this, tr("UltraScan Information"),
		tr("This is the fitted solution plot"),
		QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		
	}
}

void US_Spectrum::load_basis()
{
	struct WavelengthProfile temp_wp;
	QString str, fileName;
	unsigned int i, j, current, points;
	double *x, *y;
	QPen pen;
	pen.setColor(Qt::green);
	pen.setWidth(2);
	fileName = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.extinction.res", 0);
	if (fileName.isEmpty()) 
	{
		return;
	}
	else
	{
		load_gaussian_profile(temp_wp, fileName);
		basis.push_back(temp_wp);
	}
	i = basis.size() - 1;
//	str.sprintf(fi.baseName() + " [%3d:%3d], E_%3d: %6.4e",
//	basis[i].lambda_min, basis[i].lambda_max, basis[i].lambda_scale, basis[i].scale);
	lb_basis->insertItem(temp_wp.filenameBasis, -1);
	
	current = i;
	points = basis[current].lambda_max - basis[current].lambda_min + 1;
	x = new double [points];
	y = new double [points];
	find_amplitude(basis[current]);
	for (i=0; i<points; i++)
	{
		x[i] = basis[current].lambda_min + i;
		y[i] = 0.0;
		for (j=0; j<basis[current].gaussian.size(); j++)
		{
			y[i] += basis[current].gaussian[j].amplitude *
			exp(-(pow(x[i] - basis[current].gaussian[j].mean, 2.0)
			/ (2.0 * pow(basis[current].gaussian[j].sigma, 2.0))));
		}
		y[i] *= basis[current].amplitude;
	}
	curve.push_back(data_plot->insertCurve(basis[current].filenameBasis));
	i = curve.size() - 1;
	basis[current].curve_number = curve[i];
	data_plot->setCurveStyle(curve[i], QwtCurve::Lines);
	data_plot->setCurveData(curve[i], x, y, points);
	data_plot->setCurvePen(curve[i], pen);
	data_plot->replot();
	pb_extrapolate->setEnabled(true);
	pb_overlap->setEnabled(true);
	pb_fit->setEnabled(true);
	pb_difference->setEnabled(true);
	pb_save->setEnabled(false);
	pb_reset_basis->setEnabled(true);
	le_rmsd->setText("");
	delete [] x;
	delete [] y;
}

void US_Spectrum::overlap()
{
	unsigned int min_lambda = target.lambda_min;
	unsigned int max_lambda = target.lambda_max;
	unsigned int i, j, k, points;
	double *x, *y;
	QPen pen;
	if (basis.size() < 1)
	{
		return;
	}
	pb_fit->setEnabled(true);
	pb_difference->setEnabled(true);

	for (i=0; i<basis.size(); i++)
	{
		if (basis[i].lambda_min > min_lambda)
		{
			min_lambda = basis[i].lambda_min;
		}
		if (basis[i].lambda_max < max_lambda)
		{
			max_lambda = basis[i].lambda_max;
		}
	}
	points = max_lambda - min_lambda + 1;
	x = new double [points];
	y = new double [points];
	pen.setWidth(2);
	pen.setColor(Qt::green);
	for (k=0; k<basis.size(); k++)
	{
		basis[k].lambda_min = min_lambda;
		basis[k].lambda_max = max_lambda;
		find_amplitude(basis[k]);
		for (i=0; i<points; i++)
		{
			x[i] = basis[k].lambda_min + i;
			y[i] = 0.0;
			for (j=0; j<basis[k].gaussian.size(); j++)
			{
				y[i] += basis[k].gaussian[j].amplitude *
				exp(-(pow(x[i] - basis[k].gaussian[j].mean, 2.0)
				/ (2.0 * pow(basis[k].gaussian[j].sigma, 2.0))));
			}
			y[i] *= basis[k].amplitude;
		}
		data_plot->setCurveStyle(basis[k].curve_number, QwtCurve::Lines);
		data_plot->setCurveData(basis[k].curve_number, x, y, points);
		data_plot->setCurvePen(basis[k].curve_number, pen);
	}
	pen.setWidth(3);
	pen.setColor(Qt::yellow);
	target.lambda_min = min_lambda;
	target.lambda_max = max_lambda;
	find_amplitude(target);
	for (i=0; i<points; i++)
	{
		x[i] = target.lambda_min + i;
		y[i] = 0.0;
		for (j=0; j<target.gaussian.size(); j++)
		{
			y[i] += target.gaussian[j].amplitude *
			exp(-(pow(x[i] - target.gaussian[j].mean, 2.0)
			/ (2.0 * pow(target.gaussian[j].sigma, 2.0))));
		}
		y[i] *= target.amplitude;
	}
	data_plot->setCurveStyle(target.curve_number, QwtCurve::Lines);
	data_plot->setCurveData(target.curve_number, x, y, points);
	data_plot->setCurvePen(target.curve_number, pen);
	data_plot->replot();
	le_rmsd->setText("");
	delete [] x;
	delete [] y;
}

void US_Spectrum::extrapolate()
{
	unsigned int min_lambda = target.lambda_min;
	unsigned int max_lambda = target.lambda_max;
	unsigned int i, j, k, points;
	double *x, *y;
	if (basis.size() < 1)
	{
		return;
	}
	pb_fit->setEnabled(true);
	pb_difference->setEnabled(true);
	QPen pen;
	for (i=0; i<basis.size(); i++)
	{
		if (basis[i].lambda_min < min_lambda)
		{
			min_lambda = basis[i].lambda_min;
		}
		if (basis[i].lambda_max > max_lambda)
		{
			max_lambda = basis[i].lambda_max;
		}
	}
	points = max_lambda - min_lambda + 1;
	x = new double [points];
	y = new double [points];
	pen.setWidth(2);
	pen.setColor(Qt::green);
	for (k=0; k<basis.size(); k++)
	{
		basis[k].lambda_min = min_lambda;
		basis[k].lambda_max = max_lambda;
		find_amplitude(basis[k]);
		for (i=0; i<points; i++)
		{
			x[i] = basis[k].lambda_min + i;
			y[i] = 0.0;
			for (j=0; j<basis[k].gaussian.size(); j++)
			{
				y[i] += basis[k].gaussian[j].amplitude *
				exp(-(pow(x[i] - basis[k].gaussian[j].mean, 2.0)
				/ (2.0 * pow(basis[k].gaussian[j].sigma, 2.0))));
			}
			y[i] *= basis[k].amplitude;
		}
		data_plot->setCurveStyle(basis[k].curve_number, QwtCurve::Lines);
		data_plot->setCurveData(basis[k].curve_number, x, y, points);
		data_plot->setCurvePen(basis[k].curve_number, pen);
	}
	pen.setWidth(3);
	pen.setColor(Qt::yellow);
	target.lambda_min = min_lambda;
	target.lambda_max = max_lambda;
	for (i=0; i<points; i++)
	{
		x[i] = target.lambda_min + i;
		y[i] = 0.0;
		for (j=0; j<target.gaussian.size(); j++)
		{
			y[i] += target.gaussian[j].amplitude *
			exp(-(pow(x[i] - target.gaussian[j].mean, 2.0)
			/ (2.0 * pow(target.gaussian[j].sigma, 2.0))));
		}
		y[i] *= target.amplitude;
	}
	data_plot->setCurveStyle(target.curve_number, QwtCurve::Lines);
	data_plot->setCurveData(target.curve_number, x, y, points);
	data_plot->setCurvePen(target.curve_number, pen);
	data_plot->replot();
	le_rmsd->setText("");
	delete [] x;
	delete [] y;
}

void US_Spectrum::update_scale()
{
	QString str;
	unsigned int i, j, k, points;
	QPen pen;
	if (target.curve_number == current_scan)
	{
		double *x, *y;
		str = le_extinction->text();
		target.scale = str.toFloat();
		str = le_wavelength->text();
		target.lambda_scale = str.toUInt();
		find_amplitude(target);
		pen.setColor(Qt::yellow);
		pen.setWidth(3);
		points = target.lambda_max - target.lambda_min + 1;
		x = new double [points];
		y = new double [points];
		for (i=0; i<points; i++)
		{
			x[i] = target.lambda_min + i;
			y[i] = 0.0;
			for (j=0; j<target.gaussian.size(); j++)
			{
				y[i] += target.gaussian[j].amplitude *
				exp(-(pow(x[i] - target.gaussian[j].mean, 2.0)
				/ (2.0 * pow(target.gaussian[j].sigma, 2.0))));
			}
			y[i] *= target.amplitude;
		}
		k = target.curve_number;
		data_plot->setCurveStyle(k, QwtCurve::Lines);
		data_plot->setCurveData(k, x, y, points);
		data_plot->setCurvePen(k, pen);
		data_plot->replot();
		delete [] x;
		delete [] y;
	}
	else
	{
		for (i=0; i<basis.size(); i++)
		{
			if (basis[i].curve_number == current_scan)
			{
				double *x, *y;
				str = le_extinction->text();
				basis[i].scale = str.toFloat();
				str = le_wavelength->text();
				basis[i].lambda_scale = str.toUInt();
				find_amplitude(basis[i]);
				pen.setColor(Qt::green);
				pen.setWidth(2);
				points = basis[i].lambda_max - basis[i].lambda_min + 1;
				x = new double [points];
				y = new double [points];
				for (k=0; k<points; k++)
				{
					x[k] = basis[i].lambda_min + k;
					y[k] = 0.0;
					for (j=0; j<basis[i].gaussian.size(); j++)
					{
						y[k] += basis[i].gaussian[j].amplitude *
						exp(-(pow(x[k] - basis[i].gaussian[j].mean, 2.0)
						/ (2.0 * pow(basis[i].gaussian[j].sigma, 2.0))));
					}
					y[k] *= basis[i].amplitude;
				}
				k = basis[i].curve_number;
				data_plot->setCurveStyle(k, QwtCurve::Lines);
				data_plot->setCurveData(k, x, y, points);
				data_plot->setCurvePen(k, pen);
				data_plot->replot();
				delete [] x;
				delete [] y;
				break;
			}
		}
	}
	le_rmsd->setText("");
	reset_edit_gui();
}

void US_Spectrum::delete_scan()
{
	unsigned int i;
	for (i=0; i<basis.size(); i++)
	{
		if (basis[i].curve_number == current_scan)
		{
			delete_basis((int) i);
		}
	}
}

void US_Spectrum::find_amplitude(struct WavelengthProfile &profile)
{
	unsigned int j;
	profile.amplitude = 0;
	for (j=0; j<profile.gaussian.size(); j++)
	{
		profile.amplitude += profile.gaussian[j].amplitude *
		exp(-(pow(profile.lambda_scale - profile.gaussian[j].mean, 2.0)
		/ (2.0 * pow(profile.gaussian[j].sigma, 2.0))));
	}
	cout << "profile.amplitude in find_amplitude(): " << profile.amplitude << endl; 
	profile.amplitude = 1.0/profile.amplitude;
	profile.amplitude *= profile.scale;
}

void US_Spectrum::print_profile(struct WavelengthProfile &profile)
{
	cout << "Gaussian size: " << profile.gaussian.size() << endl;
	for (unsigned int i=0; i<profile.gaussian.size(); i++)
	{
		cout << "Gaussian " << i+1 << ": " << endl;
		cout << "Mean: " << profile.gaussian[i].mean << endl;
		cout << "amplitude: " << profile.gaussian[i].amplitude << endl;
		cout << "Sigma: " << profile.gaussian[i].sigma << endl;
	}

	cout << "scale: " << profile.scale << endl;
	cout << "lambda_scale: " << profile.lambda_scale << endl;
	cout << "lambda_min: " << profile.lambda_min << endl;
	cout << "lambda_max: " << profile.lambda_max << endl;
	cout << "filename: " << profile.filename << endl;
	cout << "filenameBasis: " << profile.filenameBasis << endl;
	cout << "curve_number: " << profile.curve_number << endl;
	cout << "amplitude: " << profile.amplitude << endl;
	cout << "nnls_factor: " << profile.nnls_factor << endl;
	cout << "nnls_percentage: " << profile.nnls_percentage << endl;
}

void US_Spectrum::load_target()
{
	unsigned int i, j, points;
	double *x, *y;
	QString str1, str2;
	QPen pen;
	QString fileName = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.extinction.res", 0);
	if (fileName.isEmpty()) 
	{
		return;
	}
	else
	{
		load_gaussian_profile(target, fileName);
	}
	QFileInfo fi;
	fi.setFile(fileName);
//	str1.sprintf(fi.baseName() + " [%3d:%3d], E_%3d: %6.4e", 
//	target.lambda_min, target.lambda_max, target.lambda_scale, target.scale);
	lb_target->clear();
	lb_target->insertItem(fi.baseName());
	points = target.lambda_max - target.lambda_min + 1;
	x = new double [points];
	y = new double [points];
	//find_amplitude(target);
	for (i=0; i<points; i++)
	{
		x[i] = target.lambda_min + i;
		y[i] = 0.0;
		for (j=0; j<target.gaussian.size(); j++)
		{
			y[i] += target.gaussian[j].amplitude *
			exp(-(pow(x[i] - target.gaussian[j].mean, 2.0)
			/ (2.0 * pow(target.gaussian[j].sigma, 2.0))));
		}
		y[i] *= target.amplitude;
		//cout << target.amplitude << ", y[" << target.lambda_min + i << "]: " << y[i] << endl;
	}
	if (target.curve_number != -1)
	{
		data_plot->removeCurve(target.curve_number);
	}
	pen.setColor(Qt::yellow);
	pen.setWidth(3);
	curve[0] = data_plot->insertCurve(fi.baseName());
	target.curve_number = (int) curve[0];
	data_plot->setCurveStyle(curve[0], QwtCurve::Lines);
	data_plot->setCurveData(curve[0], x, y, points);
	data_plot->setCurvePen(curve[0], pen);
	data_plot->replot();
	if(basis.size() > 0)
	{
		pb_extrapolate->setEnabled(true);
		pb_overlap->setEnabled(true);
	}
	else
	{
		pb_extrapolate->setEnabled(false);
		pb_overlap->setEnabled(false);
	}
	pb_save->setEnabled(false);
	pb_load_basis->setEnabled(true);
	pb_print_fit->setEnabled(true);
	le_rmsd->setText("");
	delete [] x;
	delete [] y;
}

void US_Spectrum::fit()
{
	unsigned int min_lambda = target.lambda_min;
	unsigned int max_lambda = target.lambda_max;
	unsigned int points, order, i, j, k, counter=0;
	double *nnls_a, *nnls_b, *nnls_x, nnls_rnorm, *nnls_wp, *nnls_zzp, *x, *y;
	float fval = 0.0;
	vector <float> residuals, solution, b;
	QPen pen;
	residuals.clear();
	solution.clear();
	b.clear();
	int *nnls_indexp;
	QString str = "Please note:\n\n"
					"The target and basic spectra have different limits.\n" 
					"These vectors need to be congruent before you can fit\n"
					"the data. You can correct the problem by first running\n"
					"\"Find Extinction Profile Overlap\" (preferred), or by\n"
					"running \"Extrapolate Extinction Profile\" (possibly imprecise).";
	for (i=0; i<basis.size(); i++)
	{
		if (basis[i].lambda_min != min_lambda || basis[i].lambda_max != max_lambda)
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr(str),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			return;
		}
	}
	
	points = target.lambda_max - target.lambda_min + 1;
	x = new double [points];
	y = new double [points];
	order = basis.size(); // no baseline necessary with gaussians
	nnls_a = new double [points * order]; // contains the model functions, end-to-end
	nnls_b = new double [points]; // contains the experimental data
	nnls_zzp = new double [points]; // pre-allocated working space for nnls
	nnls_x = new double [order]; // the solution vector, pre-allocated for nnls
	nnls_wp = new double [order]; // pre-allocated working space for nnls, On exit, wp[] will contain the dual solution vector, wp[i]=0.0 for all i in set p and wp[i]<=0.0 for all i in set z. 
	nnls_indexp = new int [order];
	//find_amplitude(target);
	for (i=0; i<points; i++)
	{
		x[i] = target.lambda_min + i;
		nnls_b[i] = 0.0;
		for (j=0; j<target.gaussian.size(); j++)
		{
			nnls_b[i] += target.gaussian[j].amplitude *
			exp(-(pow(x[i] - target.gaussian[j].mean, 2.0)
			/ (2.0 * pow(target.gaussian[j].sigma, 2.0))));
		}
		nnls_b[i] *= target.amplitude;
		b.push_back((float) nnls_b[i]);
	}
	counter = 0;
	for (k=0; k<order; k++)
	{
		//find_amplitude(basis[k]);
		for (i=0; i<points; i++)
		{
			x[i] = basis[k].lambda_min + i;
			nnls_a[counter] = 0.0;
			for (j=0; j<basis[k].gaussian.size(); j++)
			{
				nnls_a[counter] += basis[k].gaussian[j].amplitude *
				exp(-(pow(x[i] - basis[k].gaussian[j].mean, 2.0)
				/ (2.0 * pow(basis[k].gaussian[j].sigma, 2.0))));
			}
			nnls_a[counter] *= basis[k].amplitude;
			counter ++;
		}
	}
	nnls(nnls_a, points, points, order, nnls_b, nnls_x, &nnls_rnorm, nnls_wp, nnls_zzp, nnls_indexp);
	vector <float> results;
	results.clear();
	fval = 0.0;
	for (i=0; i<basis.size(); i++)
	{
		fval += nnls_x[i];
	}
	for (i=0; i<basis.size(); i++)
	{
		results.push_back(100.0 * nnls_x[i]/fval);
		str.sprintf(basis[i].filenameBasis + ": %3.2f%% (%6.4e)", results[i], nnls_x[i]);
		lb_basis->changeItem(str, i);
		basis[i].nnls_factor = nnls_x[i];
		basis[i].nnls_percentage = results[i];
	}
	
	for (i=0; i<points; i++)
	{
		solution.push_back(0.0);
	}
	for (k=0; k<order; k++)
	{
		//find_amplitude(basis[k]);
		for (i=0; i<points; i++)
		{
			x[i] = basis[k].lambda_min + i;
			
			for (j=0; j<basis[k].gaussian.size(); j++)
			{
				solution[i] += (basis[k].gaussian[j].amplitude *
				exp(-(pow(x[i] - basis[k].gaussian[j].mean, 2.0)
				/ (2.0 * pow(basis[k].gaussian[j].sigma, 2.0))))) * basis[k].amplitude * nnls_x[k];
			}
		}
	}
	for (i=0; i<points; i++)
	{
		residuals.push_back(solution[i] - b[i]);
		y[i] = solution[i];
	}
	residuals_plot->clear();
	unsigned int resid_curve = residuals_plot->insertCurve("Residuals");
	unsigned int target_curve = residuals_plot->insertCurve("Mean");
	if (solution_curve > -1)
	{
		data_plot->removeCurve(solution_curve);
	}
	solution_curve = data_plot->insertCurve("Solution");

	residuals_plot->setCurveStyle(resid_curve, QwtCurve::Lines);
	residuals_plot->setCurveStyle(target_curve, QwtCurve::Lines);
	data_plot->setCurveStyle(solution_curve, QwtCurve::Lines);
	
	data_plot->setCurveData(solution_curve, x, y, points);
	pen.setColor(Qt::magenta);
	pen.setWidth(3);
	data_plot->setCurvePen(solution_curve, pen);
	data_plot->replot();

	fval = 0.0;
	for (i=0; i<points; i++)
	{
		y[i] = residuals[i];
		fval += pow(residuals[i], (float) 2.0);
	}
	fval /= points;
	le_rmsd->setText(str.sprintf("RMSD: %3.2e", pow(fval, (float) 0.5)));
	residuals_plot->setCurveData(resid_curve, x, y, points);
	pen.setColor(Qt::yellow);
	pen.setWidth(2);
	residuals_plot->setCurvePen(resid_curve, pen);
	residuals_plot->replot();

	x[1] = x[points - 1];
	y[0] = 0.0;
	y[1] = 0.0;
	residuals_plot->setCurveData(target_curve, x, y, 2);
	pen.setColor(Qt::red);
	pen.setWidth(3);
	residuals_plot->setCurvePen(target_curve, pen);
	residuals_plot->replot();
	pb_save->setEnabled(true);
	pb_print_residuals->setEnabled(true);
	delete [] x;
	delete [] y;
}

void US_Spectrum::difference()
{
	unsigned int min_lambda = target.lambda_min;
	unsigned int max_lambda = target.lambda_max;
	unsigned int points, i, j;
	double *x, *y;
	float fval = 0.0;
	vector <float> residuals;
	QPen pen;
	residuals.clear();
	QString str = "Please note:\n\n"
					"The target and basic spectra have different limits.\n" 
					"These vectors need to be congruent before you can fit\n"
					"the data. You can correct the problem by first running\n"
					"\"Find Extinction Profile Overlap\" (preferred), or by\n"
					"running \"Extrapolate Extinction Profile\" (possibly imprecise).";
	for (i=0; i<basis.size(); i++)
	{
		if (basis[i].lambda_min != min_lambda || basis[i].lambda_max != max_lambda)
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr(str),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			return;
		}
	}
	
	points = target.lambda_max - target.lambda_min + 1;
	x = new double [points];
	y = new double [points];
	residuals.resize(points);
	//find_amplitude(target);
	for (i=0; i<points; i++)
	{
		x[i] = target.lambda_min + i;
		y[i] = 0.0;
		for (j=0; j<target.gaussian.size(); j++)
		{
			y[i] += target.gaussian[j].amplitude *
			exp(-(pow(x[i] - target.gaussian[j].mean, 2.0)
			/ (2.0 * pow(target.gaussian[j].sigma, 2.0))));
		}
		y[i] *= target.amplitude;
	}
	//find_amplitude(basis[0]);
	for (i=0; i<points; i++)
	{
		x[i] = basis[0].lambda_min + i;
		residuals[i] = 0.0;
		for (j=0; j<basis[0].gaussian.size(); j++)
		{
			residuals[i] += basis[0].gaussian[j].amplitude *
			exp(-(pow(x[i] - basis[0].gaussian[j].mean, 2.0)
			/ (2.0 * pow(basis[0].gaussian[j].sigma, 2.0))));
		}
		residuals[i] *= basis[0].amplitude;
	}
	for (i=0; i<points; i++)
	{
		residuals[i] = y[i] - residuals[i];
	}
	residuals_plot->clear();
	unsigned int resid_curve = residuals_plot->insertCurve("Residuals");
	unsigned int target_curve = residuals_plot->insertCurve("Mean");
	residuals_plot->setCurveStyle(resid_curve, QwtCurve::Lines);
	residuals_plot->setCurveStyle(target_curve, QwtCurve::Lines);
	data_plot->setCurveStyle(solution_curve, QwtCurve::Lines);
	pen.setColor(Qt::magenta);
	pen.setWidth(3);
	fval = 0.0;
	for (i=0; i<points; i++)
	{
		y[i] = residuals[i];
		fval += pow(residuals[i], (float) 2.0);
	}
	fval /= points;
	le_rmsd->setText(str.sprintf("RMSD: %3.2e", pow(fval, (float) 0.5)));
	residuals_plot->setCurveData(resid_curve, x, y, points);
	pen.setColor(Qt::yellow);
	pen.setWidth(2);
	residuals_plot->setCurvePen(resid_curve, pen);
	residuals_plot->replot();

	x[1] = x[points - 1];
	y[0] = 0.0;
	y[1] = 0.0;
	residuals_plot->setCurveData(target_curve, x, y, 2);
	pen.setColor(Qt::red);
	pen.setWidth(3);
	residuals_plot->setCurvePen(target_curve, pen);
	residuals_plot->replot();
	pb_print_residuals->setEnabled(true);
	delete [] x;
	delete [] y;
}

void US_Spectrum::print_fit()
{
	QPrinter printer;
	if	(printer.setup(0))
	{
		PrintFilter pf;
		data_plot->print(printer);
	}
}

void US_Spectrum::print_residuals()
{
	QPrinter printer;
	if	(printer.setup(0))
	{
		PrintFilter pf;
		residuals_plot->print(printer);
	}
}
