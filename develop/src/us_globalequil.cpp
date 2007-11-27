#include "../include/us_globalequil.h"
#include "../include/redarrow.xpm"
#include "../include/bluearrow.xpm"
#include "../include/greenarrow.xpm"
#include <stdio.h>
#include <float.h>
#include <cerrno>

US_GlobalEquil::US_GlobalEquil(QWidget *p, const char *name) : QFrame( p, name)
{
	USglobal = new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	buttonw = 156;	//make divisible by 3 and by 2
	buttonh = 26;
	border = 4;
	xpos = border;
	ypos = border;
	model_widget = false;
	fitting_widget = false;
	monte_carlo_widget = false;
	buffer_from_db = false;
	vbar_from_db = false;
	model = -1; //initialize with "No Model selected"
	dataset = 0;
	scans_in_list = 0;
	od_limit = 0.0;
	mouse_was_moved = false;
	float_parameters = false;
	scans_have_problems = false;
	show_messages = true;
	scans_initialized = false;
	projectName = "SampleFit";
	pm = new US_Pixmap();
	mc.mw_upperLimit = 0.0;
	mc.mw_lowerLimit = 0.0;

	data_plot = new QwtPlot(this);
	Q_CHECK_PTR(data_plot);
	data_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	data_plot->enableGridXMin();
	data_plot->enableGridYMin();
	data_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	data_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	//data_plot->setPlotBackground(USglobal->global_colors.plot);		//old version
	data_plot->setCanvasBackground(USglobal->global_colors.plot);		//new version
	data_plot->setMargin(USglobal->config_list.margin);
	data_plot->enableOutline(true);
	data_plot->setAxisTitle(QwtPlot::xBottom, " ");
	data_plot->setAxisTitle(QwtPlot::yLeft, " ");
	data_plot->setTitle(tr("Experimental Data"));
	data_plot->setOutlinePen(white);
	data_plot->setOutlineStyle(Qwt::Cross);
	data_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	data_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	connect(data_plot, SIGNAL(plotMouseReleased(const QMouseEvent &)),
			  SLOT(getPlotMouseReleased(const QMouseEvent &)));
	connect(data_plot, SIGNAL(plotMouseMoved(const QMouseEvent &)),
			  SLOT(getPlotMouseMoved(const QMouseEvent &)));

	InfoPanel = new US_InfoPanel(model, &od_limit, this);
	InfoPanel->le_max_od->setText("0.90");
	InfoPanel->lbl_status2->setText(tr("Please select an edited Equilibrium Dataset with \"Load Data\""));
	connect(InfoPanel, SIGNAL(limitChanged(float)), SLOT(update_limit(float)));
	connect(InfoPanel->le_max_od, SIGNAL(returnPressed()), SLOT(call_show_scan()));

	pb_load = new QPushButton(tr("Load Data"), this);
	Q_CHECK_PTR(pb_load);
	pb_load->setAutoDefault(false);
	pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_load->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_load, SIGNAL(clicked()), SLOT(load()));

	xpos += buttonw + spacing;

	pb_details = new QPushButton(tr("Run Details"), this);
	Q_CHECK_PTR(pb_details);
	pb_details->setAutoDefault(false);
	pb_details->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_details->setEnabled(false);
	pb_details->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_details->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_details, SIGNAL(clicked()), SLOT(details()));

	xpos = border;
	ypos += buttonh + spacing;

	pb_unload = new QPushButton(tr("Unload all Data"), this);
	Q_CHECK_PTR(pb_unload);
	pb_unload->setAutoDefault(false);
	pb_unload->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_unload->setEnabled(false);
	pb_unload->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_unload->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_unload, SIGNAL(clicked()), SLOT(unload()));

	xpos += buttonw + spacing;

	pb_print = new QPushButton(tr("Print"), this);
	Q_CHECK_PTR(pb_print);
	pb_print->setAutoDefault(false);
	pb_print->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_print->setEnabled(false);
	pb_print->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_print->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_print, SIGNAL(clicked()), SLOT(print()));

	xpos = border;
	ypos += buttonh + spacing;

	pb_diagnostics = new QPushButton(tr("Scan Diagnostics"), this);
	Q_CHECK_PTR(pb_diagnostics);
	pb_diagnostics->setAutoDefault(false);
	pb_diagnostics->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_diagnostics->setEnabled(false);
	pb_diagnostics->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_diagnostics->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_diagnostics, SIGNAL(clicked()), SLOT(diagnostics()));

	xpos += buttonw + spacing;

	pb_scan_check = new QPushButton(tr("Check Scans for Fit"), this);
	Q_CHECK_PTR(pb_scan_check);
	pb_scan_check->setAutoDefault(false);
	pb_scan_check->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_scan_check->setEnabled(false);
	pb_scan_check->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_scan_check->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_scan_check, SIGNAL(clicked()), SLOT(scan_check()));

	xpos = border;
	ypos += buttonh + spacing;

	pb_histogram = new QPushButton(tr("Conc. Histogram"), this);
	Q_CHECK_PTR(pb_histogram);
	pb_histogram->setAutoDefault(false);
	pb_histogram->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_histogram->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_histogram->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_histogram->setEnabled(false);
	connect(pb_histogram, SIGNAL(clicked()), SLOT(histogram()));

	xpos += buttonw + spacing;

	pb_reset_limits = new QPushButton(tr("Reset Scan Limits"), this);
	Q_CHECK_PTR(pb_reset_limits);
	pb_reset_limits->setAutoDefault(false);
	pb_reset_limits->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset_limits->setEnabled(false);
	pb_reset_limits->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_reset_limits->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_reset_limits, SIGNAL(clicked()), SLOT(reset_limits()));

	xpos = border;
	ypos += buttonh + 2 * spacing;

	lbl_projectName = new QLabel(tr(" Project Name:"), this);
	Q_CHECK_PTR(lbl_projectName);
	lbl_projectName->setAlignment(AlignLeft|AlignVCenter);
	lbl_projectName->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_projectName->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_projectName->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos += buttonw + spacing;

	le_projectName = new QLineEdit(this, "projectName");
	le_projectName->setGeometry(xpos, ypos, buttonw, buttonh);
	le_projectName->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_projectName->setText(tr(" SampleFit"));
	connect(le_projectName, SIGNAL(textChanged(const QString &)),
				SLOT(update_projectName(const QString &)));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_banner1 = new QLabel(tr("List of available Equilibrium Scans:"),this);
	Q_CHECK_PTR(lbl_banner1);
	lbl_banner1->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_banner1->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner1->setGeometry(xpos, ypos, buttonw*2+spacing, buttonh);
	lbl_banner1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += buttonh;

	lb_scans = new QListBox(this, "Scans");
	lb_scans->setGeometry(xpos, ypos, 2 * buttonw + spacing, 9 * buttonh);
	lb_scans->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_scans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(lb_scans, SIGNAL(highlighted(int)), SLOT(select_scan(int)));
	connect(lb_scans, SIGNAL(selected(int)), SLOT(change_FitFlag(int)));

	ypos += 9 * buttonh + 2 * spacing;

	lbl_banner2 = new QLabel(tr("Model and Fitting Information:"), this);
	Q_CHECK_PTR(lbl_banner2);
	lbl_banner2->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_banner2->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner2->setGeometry(xpos, ypos, buttonw*2+spacing, buttonh);
	lbl_banner2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos = border;
	ypos += buttonh + 2 * spacing;

	pb_model = new QPushButton(tr("Select Model"), this);
	Q_CHECK_PTR(pb_model);
	pb_model->setAutoDefault(false);
	pb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_model->setEnabled(false);
	pb_model->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_model->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_model, SIGNAL(clicked()), SLOT(select_model()));

	xpos += buttonw + spacing;

	pb_fitcontrol = new QPushButton(tr("Fitting Control"), this);
	Q_CHECK_PTR(pb_fitcontrol);
	pb_fitcontrol->setAutoDefault(false);
	pb_fitcontrol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_fitcontrol->setEnabled(false);
	pb_fitcontrol->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_fitcontrol->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_fitcontrol, SIGNAL(clicked()), SLOT(fitcontrol()));

	xpos = border;
	ypos += buttonh + spacing;

	pb_report = new QPushButton(tr("View Report"), this);
	Q_CHECK_PTR(pb_report);
	pb_report->setAutoDefault(false);
	pb_report->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_report->setEnabled(false);
	pb_report->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_report->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_report, SIGNAL(clicked()), SLOT(report()));

	xpos += buttonw + spacing;

	pb_monte_carlo = new QPushButton(tr("Monte Carlo"), this);
	Q_CHECK_PTR(pb_monte_carlo);
	pb_monte_carlo->setAutoDefault(false);
	pb_monte_carlo->setEnabled(false);
	pb_monte_carlo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_monte_carlo->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_monte_carlo->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_monte_carlo, SIGNAL(clicked()), SLOT(monte_carlo()));

	xpos = border;
	ypos += buttonh + spacing;

	pb_loadfit = new QPushButton(tr("Load Fit"), this);
	Q_CHECK_PTR(pb_loadfit);
	pb_loadfit->setAutoDefault(false);
	pb_loadfit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_loadfit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_loadfit->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_loadfit, SIGNAL(clicked()), SLOT(loadfit()));

	xpos += buttonw + spacing;

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	xpos = border;
	ypos += buttonh + 2 * spacing;

	lbl_banner3 = new QLabel(tr("Parameter Information:"),this);
	Q_CHECK_PTR(lbl_banner3);
	lbl_banner3->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_banner3->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner3->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner3->setGeometry(xpos, ypos, buttonw*2+spacing, buttonh);
	lbl_banner3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos = border;
	ypos += buttonh + 2 * spacing;

	pb_float_all = new QPushButton(tr("Float Parameters"), this);
	Q_CHECK_PTR(pb_float_all);
	pb_float_all->setAutoDefault(false);
	pb_float_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_float_all->setEnabled(false);
	pb_float_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_float_all->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_float_all, SIGNAL(clicked()), SLOT(float_all()));

	xpos += buttonw + spacing;

	pb_initialize = new QPushButton(tr("Initialize Parameters"), this);
	Q_CHECK_PTR(pb_initialize);
	pb_initialize->setAutoDefault(false);
	pb_initialize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_initialize->setEnabled(false);
	pb_initialize->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_initialize->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_initialize, SIGNAL(clicked()), SLOT(initialize()));

	xpos = border;
	ypos += buttonh + spacing;

	pb_model_control = new QPushButton(tr("Model Control"), this);
	Q_CHECK_PTR(pb_model_control);
	pb_model_control->setAutoDefault(false);
	pb_model_control->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_model_control->setEnabled(false);
	pb_model_control->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_model_control->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_model_control, SIGNAL(clicked()), SLOT(select_model_control()));

	xpos += buttonw + spacing;

	pb_close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_close);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_scans = new QLabel(tr(" Scan Selector:"), this);
	Q_CHECK_PTR(lbl_scans);
	lbl_scans->setAlignment(AlignLeft|AlignVCenter);
	lbl_scans->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_scans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_scans->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos += buttonw + spacing;

	cnt_scans= new QwtCounter(this);
	Q_CHECK_PTR(cnt_scans);
	cnt_scans->setRange(0, 0, 1);
	cnt_scans->setNumButtons(2);
	cnt_scans->setValue(0);
	cnt_scans->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_scans->setGeometry(xpos, ypos, buttonw, buttonh);
//	connect(cnt_scans, SIGNAL(buttonReleased(double)), SLOT(select_scan(double)));
	connect(cnt_scans, SIGNAL(valueChanged(double)), SLOT(select_scan(double)));

	ypos += buttonh + border;

	global_Xpos += 30;
	global_Ypos += 30;

	setMinimumSize(11 * buttonw / 2, ypos);
	setGeometry(global_Xpos, global_Ypos, 11 * buttonw / 2, ypos);

	setup_GUI();

	cp_list.clear();
	rotor_list.clear();
	if (!readCenterpieceInfo(&cp_list))
	{
		QMessageBox::critical(0, tr("UltraScan Fatal Error:"), tr("There was a problem opening the\n"
									"centerpiece database file:\n\n")
									+ USglobal->config_list.system_dir + "/etc/centerpiece.dat\n\n"
									+ tr("Please install the centerpiece database file\n"
									"before proceeding."), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		exit(-1);
	}
	if (!readRotorInfo(&rotor_list))
	{
		QMessageBox::critical(0, tr("UltraScan Fatal Error:"), tr("There was a problem opening the\n"
									"rotor database file:\n\n")
									+ USglobal->config_list.system_dir + "/etc/rotor.dat\n\n"
									+ tr("Please install the rotor database file\n"
									"before proceeding."), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		exit(-1);
	}
}

US_GlobalEquil::~US_GlobalEquil()
{
}

void US_GlobalEquil::setup_GUI()
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
	subGrid1->addWidget(pb_load,j,0);
	subGrid1->addWidget(pb_details,j,1);
	j++;
	subGrid1->addWidget(pb_unload,j,0);
	subGrid1->addWidget(pb_print,j,1);
	j++;
	subGrid1->addWidget(pb_diagnostics,j,0);
	subGrid1->addWidget(pb_scan_check,j,1);
	j++;
	subGrid1->addWidget(pb_histogram,j,0);
	subGrid1->addWidget(pb_reset_limits,j,1);
	j++;
	subGrid1->addWidget(lbl_projectName,j,0);
	subGrid1->addWidget(le_projectName,j,1);
	j++;
	subGrid1->addMultiCellWidget(lbl_banner1,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(lb_scans,j,j+10,0,1);
	j=j+11;
	subGrid1->addMultiCellWidget(lbl_banner2,j,j,0,1);
	j++;
	subGrid1->addWidget(pb_model,j,0);
	subGrid1->addWidget(pb_fitcontrol,j,1);
	j++;
	subGrid1->addWidget(pb_report,j,0);
	subGrid1->addWidget(pb_monte_carlo,j,1);
	j++;
	subGrid1->addWidget(pb_loadfit,j,0);
	subGrid1->addWidget(pb_help,j,1);
	j++;
	subGrid1->addMultiCellWidget(lbl_banner3,j,j,0,1);
	j++;
	subGrid1->addWidget(pb_float_all,j,0);
	subGrid1->addWidget(pb_initialize,j,1);
	j++;
	subGrid1->addWidget(pb_model_control,j,0);
	subGrid1->addWidget(pb_close,j,1);
	j++;
	subGrid1->addWidget(lbl_scans,j,0);
	subGrid1->addWidget(cnt_scans,j,1);

	background->addMultiCell(subGrid1,0,1,0,0);
	background->addWidget(data_plot,0,1);
	background->addWidget(InfoPanel,1,1);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(1,600);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;

	this->setGeometry(global_Xpos, global_Ypos, r.width()+600+spacing*3, r.height());


}

void US_GlobalEquil::clean_runInfo()
{
/*
	unsigned int i, j;
	for (i=0; i<8; i++)
	{
		for (j=0; j<run_inf.wavelength_count[i]; j++)
		{
			delete [] run_inf.temperature[i][j];
			delete [] run_inf.rpm[i][j];
			delete [] run_inf.time[i][j];
			delete [] run_inf.omega_s_t[i][j];
		}
		delete [] run_inf.temperature[i];
		delete [] run_inf.rpm[i];
		delete [] run_inf.time[i];
		delete [] run_inf.omega_s_t[i];
	}
	delete [] run_inf.temperature;
	delete [] run_inf.rpm;
	delete [] run_inf.time;
	delete [] run_inf.omega_s_t;
	*/
}

void US_GlobalEquil::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
	if (fitting_widget)
	{
		eqFitter->close();
	}
	if (model_widget)
	{
		model_control->close();
	}
}
/*
void US_GlobalEquil::resizeEvent(QResizeEvent *e)
{
	int dialogw = 2 * buttonw + spacing + 2 * border;
	int plot_width = e->size().width() - dialogw - border;
	int plot_height = e->size().height() - 3 * border - 3 * buttonh - 2 * spacing;
	data_plot->setGeometry(	dialogw, border, plot_width, plot_height);
	InfoPanel->setGeometry(	dialogw, plot_height + 2 * border, plot_width, 3 * buttonh + 2 * spacing);
}
*/

/* This function will accept the current mouse position and plot the portion of the scan
 * to be excluded in red while the mouse is depressed. Which portion of the scan is plotted
 * in red depends on wether the point is to the left or right of the X-middle of the scan
 */
void US_GlobalEquil::getPlotMouseMoved(const QMouseEvent &e)
{
//	cout << "Selected scan in getPlotMouseMoved: " << selected_scan << endl;
	if (scans_in_list == 0)
	{
		return;
	}
	float current_x = (data_plot->invTransform(QwtPlot::xBottom, e.x()));
	float start_x, stop_x, half_x;
	int yellow_points=0, red_points=0;
	long curve_yellow = 0, curve_red = 0;
	double *x_temp1, *y_temp1, *x_temp2, *y_temp2;
	vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin() + selected_scan;
	vector<float>::iterator yval_it = (*sfv_it).y.begin() + (*sfv_it).start_index;
	vector<float>::iterator xval_it1 = (*sfv_it).x.begin() + (*sfv_it).start_index;
	vector<float>::iterator xval_it2 = (*sfv_it).x.begin() + (*sfv_it).stop_index;

	start_x = *xval_it1;
	stop_x  = *xval_it2;
	half_x  = *xval_it1 + (*xval_it2 - *xval_it1)/2;
	if (current_x <= start_x || current_x >= stop_x) // user clicked outside the scan range
	{
		return;
	}

	QwtSymbol symbol;
	symbol.setSize(10);
	symbol.setStyle(QwtSymbol::Ellipse);
	data_plot->clear();
	curve_yellow = data_plot->insertCurve("curve_yellow");
	curve_red = data_plot->insertCurve("curve_red");
	x_temp1 = new double [(*sfv_it).points];
	y_temp1 = new double [(*sfv_it).points];
	x_temp2 = new double [(*sfv_it).points];
	y_temp2 = new double [(*sfv_it).points];

	if (current_x >= half_x)	// we are on the upper portion
	{
		yellow_points = 0;
		while (*xval_it1 < current_x)
		{
			x_temp1[yellow_points] = (double) *xval_it1;	//need to promote to double for plotting
			y_temp1[yellow_points] = (double) *yval_it;
			xval_it1++;
			yval_it++;
			yellow_points++;
		}
		red_points = 0;
		while (*xval_it1 < stop_x)
		{
			x_temp2[red_points] = (double) *xval_it1;	//need to promote to double for plotting
			y_temp2[red_points] = (double) *yval_it;
			xval_it1++;
			yval_it++;
			red_points++;
		}
	}
	else
	{
		while (*xval_it1 < current_x)
		{
			x_temp2[red_points] = (double) *xval_it1;	//need to promote to double for plotting
			y_temp2[red_points] = (double) *yval_it;
			xval_it1++;
			yval_it++;
			red_points++;
		}
		while (*xval_it1 < stop_x)
		{
			x_temp1[yellow_points] = (double) *xval_it1;	//need to promote to double for plotting
			y_temp1[yellow_points] = (double) *yval_it;
			xval_it1++;
			yval_it++;
			yellow_points++;
		}
	}
	symbol.setPen(Qt::blue);
	symbol.setBrush(Qt::yellow);
	data_plot->setCurveData(curve_yellow, x_temp1, y_temp1, yellow_points);
	data_plot->setCurveSymbol(curve_yellow, symbol);
	symbol.setPen(Qt::white);
	symbol.setBrush(Qt::red);
	data_plot->setCurveData(curve_red, x_temp2, y_temp2, red_points);
	data_plot->setCurveSymbol(curve_red, symbol);
	data_plot->replot();
	delete [] x_temp1;
	delete [] y_temp1;
	delete [] x_temp2;
	delete [] y_temp2;
	mouse_was_moved = true;
}

void US_GlobalEquil::getPlotMouseReleased(const QMouseEvent &e)
{
	bool check_widgets=false;
	if (scans_in_list == 0)
	{
		return;
	}
	float current_x = (data_plot->invTransform(QwtPlot::xBottom, e.x()));
	float start_x, stop_x, half_x;
	vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin() + selected_scan;
//cout << "before modifying scan " << selected_scan << ": " << (*sfv_it).start_index << " - " << (*sfv_it).stop_index << endl;
	if (mouse_was_moved)
	{
		vector<float>::iterator xval_it1 = (*sfv_it).x.begin() + (*sfv_it).start_index;
		vector<float>::iterator xval_it2 = (*sfv_it).x.begin() + (*sfv_it).stop_index;
		start_x = *xval_it1;
		stop_x  = *xval_it2;
		half_x  = *xval_it1 + (*xval_it2 - *xval_it1)/2;
//cout << "current_x: " << current_x << ", start_x: " << start_x << ", stop_x: " << stop_x << ", half_x: " << half_x << endl;
		if (current_x > start_x && current_x < stop_x) // && current_x < half_x) // user clicked within
		{																							// the scan range (left side)
			unsigned int i = 0;
			while (*xval_it1 < current_x)
			{
				xval_it1++;
				i++;
//cout << "Current x: " << current_x << ", xval: " << *xval_it1 << ", i: " << i << endl;
			}
			int new_start_index_addition = i;
//cout << "New start_index addition: " << i << endl;
			unsigned int j = 0;
			while (i+(*sfv_it).start_index != (*sfv_it).stop_index)
			{
				xval_it1++;
				j++;
				i++;
			}
			if (current_x >= half_x)	// stop_index is modified
			{
				(*sfv_it).stop_index -= j;
			}
			else	// start_index is modified
			{
				(*sfv_it).start_index += new_start_index_addition;
			}
//cout << "j: " << j << ", i: " << i << ", stop_index: " << (*sfv_it).stop_index << ", start_index: " << (*sfv_it).start_index << endl;
			(*sfv_it).limits_modified = true;
		}
//cout << "Modified scan " << selected_scan << ": " << (*sfv_it).start_index << " - " << (*sfv_it).stop_index << endl;
		check_widgets = true;
		mouse_was_moved = false;
	}
	show_scan(selected_scan);		// display the scan showing only the (possibly new) limits
/*
	if (model_widget && check_widgets)	//close the model widget and the fitting widget
	{												//to force re-initialization with new array limits if limits were changed
		model_control->close();
	}
*/
	if (fitting_widget && check_widgets)
	{
		eqFitter->cancel();
	}
	check_widgets = false;
}

void US_GlobalEquil::load()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.us.e", 0);
	if (!fn.isEmpty())
	{
		load(fn);
	}
}

int US_GlobalEquil::load(const QString &filename)
{
	unsigned int i, j, k, l;
	int et;
	QString version, str;
	QFile f;
	if ( !filename.isEmpty() )
	{
		QFile f(filename);
		if (f.open(IO_ReadOnly))
		{
			QDataStream ts (&f);
			ts >> version;
			if (version.toFloat() < 6.0)
			{
				QMessageBox::message(tr("Attention:"), tr("These data were edited with a previous release\n"
															  "of UltraScan (version < " + US_Version + "), which is not\n"
															  "binary compatible with the current version\n\n"
															  "Please re-edit the experimental data before\n"
															  "using the data for data analysis."));
				f.close();
				return(-1);
			}
			ts >> run_inf.data_dir;
//cout << "Directory: " << run_inf.data_dir << endl;
			ts >> run_inf.run_id;
//cout << "Run ID: " << run_inf.run_id << endl;
			ts >> run_inf.duration;
//cout << "Duration: " << run_inf.duration << endl;
			ts >> run_inf.total_scans;
//cout << "Total scans: " << run_inf.total_scans << endl;
			ts >> run_inf.delta_r;
//cout << "Delta_r: " << run_inf.delta_r << endl;
			ts >> run_inf.expdata_id;
			ts >> run_inf.investigator;
			ts >> run_inf.date;
			ts >> run_inf.description;
			ts >> run_inf.dbname;
			ts >> run_inf.dbhost;
			ts >> run_inf.dbdriver;
			ts >> et;
			run_inf.exp_type.velocity = (bool) et;
			ts >> et;
			run_inf.exp_type.equilibrium = (bool) et;
			ts >> et;
			run_inf.exp_type.diffusion = (bool) et;
			ts >> et;
			run_inf.exp_type.simulation = (bool) et;
			ts >> et;
			run_inf.exp_type.interference = (bool) et;
			ts >> et;
			run_inf.exp_type.absorbance = (bool) et;
			ts >> et;
			run_inf.exp_type.fluorescence = (bool) et;
			ts >> et;
			run_inf.exp_type.intensity = (bool) et;
			ts >> et;
			run_inf.exp_type.wavelength = (bool) et;

			for (i=0; i<8; i++)
			{
				ts >> run_inf.centerpiece[i];
				ts >> run_inf.cell_id[i];
				ts >> run_inf.wavelength_count[i];
//cout << "Centerpiece: " << run_inf.centerpiece[i] <<  ", cell id: " << run_inf.cell_id[i] <<  ", wavelength count: " << run_inf.wavelength_count[i] << endl;
			}
			for (i=0; i<8; i++)
			{
				if (version.toFloat() < 7.0)
				{
					for (j=0; j<4; j++)	// one for each channel
					{
						ts >> run_inf.DNA_serialnumber[i][j][0];
						ts >> run_inf.buffer_serialnumber[i][j];
						ts >> run_inf.peptide_serialnumber[i][j][0];
					}
				}
				else
				{
					for (j=0; j<4; j++)
					{
						ts >> run_inf.buffer_serialnumber[i][j];
						for(int k=0; k<3; k++)
						{
							ts >> run_inf.peptide_serialnumber[i][j][k];
							ts >> run_inf.DNA_serialnumber[i][j][k];
						}
					}
				}

				for (j=0; j<3; j++)
				{
					ts >> run_inf.wavelength[i][j];
					ts >> run_inf.scans[i][j];
//cout << "Scans for cell " << i+1 << ", wavelength " << j+1 << ": " << run_inf.scans[i][j] << " (wavelength: " << run_inf.wavelength[i][j] << ")" << endl;
					if (run_inf.centerpiece[i] >= 0 && run_inf.scans[i][j] > 0)
					{
						for (k=0; k<cp_list[run_inf.centerpiece[i]].channels; k++)
						{
							ts >> run_inf.range_left[i][j][k];	// we don't need individual scan ranges
							ts >> run_inf.range_right[i][j][k];	// because scan limits where picked together
							ts >> run_inf.points[i][j][k];
							ts >> run_inf.point_density[i][j][k];
/*
cout << "Range_left: " << run_inf.range_left[i][j][k] << ", Range_right: " << run_inf.range_right[i][j][k]
	  << ", point_dens: " << run_inf.point_density[i][j][k] << ", cell " << i+1 << ", wavelength " << j+1
	  << ", channel: " << k+1 << endl;
*/
						}
					}
				}
			}
			run_inf.temperature = new float** [8];
			run_inf.rpm = new unsigned int** [8];
			run_inf.time		= new unsigned int** [8];
			run_inf.omega_s_t   = new float** [8];
			for (i=0; i<8; i++)
			{
				run_inf.temperature[i] = new float* [run_inf.wavelength_count[i]];
				run_inf.rpm[i] = new unsigned int* [run_inf.wavelength_count[i]];
				run_inf.time[i]		= new unsigned int* [run_inf.wavelength_count[i]];
				run_inf.omega_s_t[i]   = new float* [run_inf.wavelength_count[i]];
				for (j=0; j<run_inf.wavelength_count[i]; j++)
				{
					run_inf.temperature[i][j] = new float [run_inf.scans[i][j]];
					run_inf.rpm[i][j] = new unsigned int [run_inf.scans[i][j]];
					run_inf.time[i][j]		= new unsigned int [run_inf.scans[i][j]];
					run_inf.omega_s_t[i][j]   = new float [run_inf.scans[i][j]];
				}
			}
			for (i=0; i<8; i++)
			{
//cout << "wavelength_count[" << i << "]: " << run_inf.wavelength_count[i] << endl;
				for (j=0; j<run_inf.wavelength_count[i]; j++)
				{
//cout << "scans[" << i << "][" << j << "]: " << run_inf.scans[i][j] << endl;
					for (k=0; k<run_inf.scans[i][j]; k++)
					{
						ts >> run_inf.rpm[i][j][k];
//cout << "rpm[" << i << "][" << j << "][" << k << "]: " << run_inf.rpm[i][j][k] << endl;
						ts >> run_inf.temperature[i][j][k];
//cout << "temperature[" << i << "][" << j << "][" << k << "]: " << run_inf.temperature[i][j][k] << endl;
						ts >> run_inf.time[i][j][k];
//cout << "time[" << i << "][" << j << "][" << k << "]: " << run_inf.time[i][j][k] << endl;
						ts >> run_inf.omega_s_t[i][j][k];
//cout << "omega[" << i << "][" << j << "][" << k << "]: " << run_inf.omega_s_t[i][j][k] << endl;
					}
				}
			}
			bool db_used = false, db_works = false;
			for(int i=0; i<8; i++)
			{
				for(int j=0; j<4; j++)
				{
					if(run_inf.buffer_serialnumber[i][j] != -1)
					{
						db_used = true;
					}
				}
			}
			if (db_used)
			{
				QString default_db;
				if(!check_dbname(run_inf.dbname, &default_db))
				{
					QMessageBox::message(tr("Attention:"),
					tr("The current dataset is obtained from the " + run_inf.dbname + " database.\n"
					"Your default database is set to the " + default_db + " database.\n"
					"Please change your default database to the " + run_inf.dbname + " database\n"
					"before analyzing this dataset!\n\n(File:Configuration:Database Preferences:Change)\n\n"));
					exit(0);
				}
				buffer_db_dlg = new US_Buffer(run_inf.investigator);
				float vbar = .72, vbar20 = .72;
				vbar_db_dlg = new US_Vbar_DB(run_inf.avg_temperature, &vbar, &vbar20, false, false, run_inf.investigator);
				int result_flag = buffer_db_dlg->db_connect();
				if(result_flag < 0) // Error!
				{
					QString error_mesg;
					if (result_flag == -1)
					{
						error_mesg = tr("Your database login information could not be found.\n"
										"Please configure your database preferences in:\n\n"
										"\"File:Configuration:Database Preferences\"");
					}
					if (result_flag == -2)
					{
						error_mesg = tr("The database driver you have selected is not available on your system.\n"
									"Please select the correct database driver in:\n\n"
									"\"File:Configuration:Database Preferences\"");
					}
					if (result_flag == -3)
					{
						error_mesg = tr("The database could not be opened.\n"
										"Possible causes for this error are:\n\n"
										"1. database name is incorrect\n"
										"2. the database password is incorrect\n"
										"3. your network connection has failed\n\n"
										"You can check your USglobal in:\n\n"
										"\"File:Configuration:Database Preferences\"");
					}
					QMessageBox::message(tr("Attention:\n"), tr("An error occured while trying to access the database:\n\n"
																		+ error_mesg +
																		"\n\nYou can continue to analyze your data by manually\n"
																		"entering the buffer and vbar information, or correct the USglobal\n"
																		"error to retrieve the buffer and vbar information from the database."));
				}
				else
				{
					db_works = true;
				}
			}
			for(int i=0; i<8; i++)
			{
				for(int j=0; j<4; j++)
				{
					if(run_inf.buffer_serialnumber[i][j] > 0 && db_works)
					{
							Buffer = buffer_db_dlg->export_buffer(run_inf.buffer_serialnumber[i][j]);
							Density[i][j] = Buffer.density;
							buffer_from_db = true;
					}
					for(int k=0; k<3; k++)
					{
						if(run_inf.peptide_serialnumber[i][j][k] > 0 && db_works)
						{
							Vbar_info = vbar_db_dlg->export_vbar(run_inf.peptide_serialnumber[i][j][k]);
							Vbar20[i][j][k] = Vbar_info.vbar20;
							vbar_from_db = true;
						}
						else
						{
							Vbar20[i][j][k]= 0.72;
						}
					//	cout<<"run_inf.peptide_serialnumber["<<i<<"]["<<j<<"]["<<k<<"]= "<<run_inf.peptide_serialnumber[i][j][k]<<endl;
					//	cout<<"Vbar20["<<i<<"]["<<j<<"]["<<k<<"]= "<<Vbar20[i][j][k]<<endl;
					}
				}
			}
			if(db_works)
			{
				buffer_db_dlg->close_db();
			}
/*****************************************************************************************************/
			ts >> run_inf.rotor;
			f.close();
		}
		else
		{
			QMessageBox::message(tr("Attention:\n"),
										tr("UltraScan can not find an associated datafile...\n\n"
											"Please make sure that the result directory\n"
											"is set to the directory where the result\n"
											"data for:\n\n" + filename + "\n\nis stored.\n"
											"This information can be set under:\n\n"
											"File:Configuration:Result Directory"));
			return(-2);
		}
		for (int i=0; i<8; i++)
		{
			bottom[i] = 0;
			has_data[i]=false;
			if (run_inf.scans[i][0] != 0)		// check which cell has data in it
			{
				has_data[i] = true;
				for (int j=0; j<3; j++)
				{
					if (run_inf.scans[i][j] != 0 && run_inf.centerpiece[i] >= 0)		// for each cell and each wavelength, check if there are
					{											// multiple channels
						for (l=0; l<cp_list[run_inf.centerpiece[i]].channels; l++)
						{
							int result = assign_scanfit(i, j, l, filename);
							if (result < 0)
							{
								return(-3);
							}
						}
					}
				}
			}
		}
		if(run_inf.exp_type.absorbance)
		{
			od_limit = 0.9;
			InfoPanel->le_max_od->setText("0.90");
		}
		if(run_inf.exp_type.interference)
		{
			od_limit = 0.0;
			InfoPanel->le_max_od->setText("0.0");
		}
		update_limit(od_limit);
		lb_scans->setCurrentItem(0);
		lb_scans->setSelected(0, true);
		if (show_messages)
		{
			run_details = new RunDetails_F(2, 1, &run_inf);
			connect(run_details, SIGNAL(isClosed()), SLOT(clean_runInfo()));
			run_details->show();
		}
		dataset++;										// we got another dataset that can be unloaded
		filenames.push_back(filename);			// add the file name of the dataset
		run_information.push_back(run_inf);		// save the dataset's run information

		pb_scan_check->setEnabled(false);
		pb_histogram->setEnabled(false);
		pb_reset_limits->setEnabled(false);
		pb_model->setEnabled(false);
		pb_fitcontrol->setEnabled(false);
		pb_report->setEnabled(false);
		pb_monte_carlo->setEnabled(false);
		pb_float_all->setEnabled(false);
		pb_initialize->setEnabled(false);
		pb_model_control->setEnabled(false);
		pb_diagnostics->setEnabled(false);
		pb_details->setEnabled(true);
		pb_unload->setEnabled(true);
		pb_print->setEnabled(true);
		pb_model->setEnabled(true);
	}
	else
	{
		str = tr("The requested datafile:\n\n\"") + filename + "\"\n\n"
				+ tr("is not available.\n\nPlease make sure all associated datafiles\n"
				"are located in the result directory\nbefore proceeding.");
		QMessageBox::message(tr("Attention:"), str);
		return(-4);
	}
	InfoPanel->lbl_status2->setText(tr("To edit scan limits, click, move and release mouse in the plot area."));
	return(0);
}

// sets up Scan fit vector with all scans that are loaded
int US_GlobalEquil::assign_scanfit(int i, int j, int l, const QString)
{
	float tempy;
	QString filename, str;
	QFile datafile;
	QDataStream s;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	str.sprintf(".equil.%d%d%d",i+1, j+1, l+1);
	filename.append(str);
	datafile.setName(filename);
	if (!datafile.open(IO_ReadOnly))
	{
		QString str1;
		str1.sprintf(tr("Sorry, a requested file is missing:\n\n"));
		str1.append(filename);
		str1.append(tr("\n\nPlease restore the file, then try again."));
		QMessageBox::message(tr("Attention:\n"), str1);
		return(-1);
	}
	s.setDevice(&datafile);
	for (unsigned int k=0; k<run_inf.scans[i][j]; k++)
	{
		struct EquilScan_FitProfile scanfit;
		scans_in_list++;
		QString temp1, temp2;
		temp1 = temp2.sprintf("%d. ", scans_in_list) + run_inf.run_id;
		temp1 += temp2.sprintf(tr(": Cell %d, Channel %d, %d nm, %4.2f ºC, %u rpm, "), i+1, l+1,
		run_inf.wavelength[i][j], run_inf.temperature[i][j][k], run_inf.rpm[i][j][k]);
		temp1 += run_inf.cell_id[i];
		scan_info.push_back(temp1);
		lb_scans->insertItem(QPixmap(bluearrow), temp1, -1);
		scanfit.points = (unsigned int) run_inf.points[i][j][l];
		scanfit.start_index = 0;
		scanfit.stop_index = scanfit.points - 1;
		scanfit.cell = i;
		scanfit.lambda = j;
		scanfit.channel = l;
//		scanfit.points =(unsigned int) ((run_inf.range_right[i][j][l] - run_inf.range_left[i][j][l])/run_inf.delta_r + 1);
//cout << "points: " << scanfit.points << ", k: " << k << ", i: " << i << ", j: " << j << ", stop_index: " << scanfit.stop_index << endl;
//cout << "points: " << scanfit.points << ", left: " << run_inf.range_left[i][j][l] << ", right: " << run_inf.range_right[i][j][l]
//	  << ", delta_r: " << run_inf.delta_r << " (cell: " << i << ", lambda: " << j << ", channel: " << l << ")" << endl;
		s >> scanfit.meniscus;
//cout << "Scan " << k << " starts here, meniscus: " << scanfit.meniscus << endl;
		for (unsigned int m=0; m<scanfit.points; m++)
		{
			s >> tempy;
//cout << "point: " << m << ", y: " << tempy << endl;
			scanfit.y.push_back(tempy);
		}
		for (unsigned int m=0; m<scanfit.points; m++)
		{
			scanfit.x.push_back(run_inf.range_left[i][j][l] + m * run_inf.delta_r);
		}
		scanfit.wavelength = run_inf.wavelength[i][j];
		scanfit.rpm = run_inf.rpm[i][j][k];
		scanfit.temperature = run_inf.temperature[i][j][k];
		scanfit.run_id = run_inf.run_id;
		scanfit.description = run_inf.cell_id[i];
		scanfit.rotor = run_inf.rotor;
		scanfit.centerpiece = run_inf.centerpiece[i];
		scanfit.density = Density[i][l];
		scanfit.FitScanFlag = false;
		scanfit.limits_modified = false;
		scanfit.amplitude.clear();
		scanfit.amplitude_index.clear();
		scanfit.amplitude_range.clear();
		scanfit.amplitude_fit.clear();
		scanfit.amplitude_bound.clear();
		scanfit.extinction.clear();
		scanfit_vector.push_back(scanfit);
	}
	lb_scans->setSelected(0, true);
	datafile.close();
	cnt_scans->setRange(1, scans_in_list, 1);
	return(scans_in_list);
}

void US_GlobalEquil::details()
{
	int dataset_selected;

	ShowDetails = new US_ShowDetails(&filenames, &dataset_selected);
	QString h;

	if (ShowDetails->exec())
	{
		unsigned int i, j, k;
		if (dataset_selected >= 0)
		{
			run_inf.data_dir = run_information[dataset_selected].data_dir;
			run_inf.run_id = run_information[dataset_selected].run_id;
			run_inf.duration = run_information[dataset_selected].duration;
			run_inf.total_scans = run_information[dataset_selected].total_scans;
			run_inf.delta_r = run_information[dataset_selected].delta_r;
			for (i=0; i<8; i++)
			{
				run_inf.centerpiece[i] = run_information[dataset_selected].centerpiece[i];
				run_inf.cell_id[i] = run_information[dataset_selected].cell_id[i];
				run_inf.wavelength_count[i] = run_information[dataset_selected].wavelength_count[i];
			}
			for (i=0; i<8; i++)
			{
				for (j=0; j<3; j++)
				{
					run_inf.wavelength[i][j] = run_information[dataset_selected].wavelength[i][j];
					run_inf.scans[i][j] = run_information[dataset_selected].scans[i][j];
					if (run_inf.centerpiece[i] >= 0 && run_inf.scans[i][j] > 0)
					{
						for (k=0; k<cp_list[run_inf.centerpiece[i]].channels; k++)
						{
							run_inf.range_left[i][j][k] = run_information[dataset_selected].range_left[i][j][k];
							run_inf.range_right[i][j][k] = run_information[dataset_selected].range_right[i][j][k];
							run_inf.points[i][j][k] = run_information[dataset_selected].points[i][j][k];
							run_inf.point_density[i][j][k] = run_information[dataset_selected].point_density[i][j][k];
						}
					}
				}
			}
			run_inf.temperature = new float** [8];
			run_inf.rpm = new unsigned int** [8];
			run_inf.time		= new unsigned int** [8];
			run_inf.omega_s_t   = new float** [8];
			for (i=0; i<8; i++)
			{
				run_inf.temperature[i] = new float* [run_inf.wavelength_count[i]];
				run_inf.rpm[i] = new unsigned int* [run_inf.wavelength_count[i]];
				run_inf.time[i]		= new unsigned int* [run_inf.wavelength_count[i]];
				run_inf.omega_s_t[i]   = new float* [run_inf.wavelength_count[i]];
				for (j=0; j<run_inf.wavelength_count[i]; j++)
				{
					run_inf.temperature[i][j] = new float [run_inf.scans[i][j]];
					run_inf.rpm[i][j] = new unsigned int [run_inf.scans[i][j]];
					run_inf.time[i][j]		= new unsigned int [run_inf.scans[i][j]];
					run_inf.omega_s_t[i][j]   = new float [run_inf.scans[i][j]];
				}
			}
			for (i=0; i<8; i++)
			{
				for (j=0; j<run_inf.wavelength_count[i]; j++)
				{
					for (k=0; k<run_inf.scans[i][j]; k++)
					{
						run_inf.rpm[i][j][k] = run_information[dataset_selected].rpm[i][j][k];
						run_inf.temperature[i][j][k] = run_information[dataset_selected].temperature[i][j][k];
						run_inf.time[i][j][k] = run_information[dataset_selected].time[i][j][k];
						run_inf.omega_s_t[i][j][k] = run_information[dataset_selected].omega_s_t[i][j][k];
					}
				}
			}
			run_inf.rotor = run_information[dataset_selected].rotor;

			run_inf = run_information[dataset_selected];
			run_details = new RunDetails_F(2, 1, &run_inf);
			run_details->show();
		}
	}
}

void US_GlobalEquil::update_projectName(const QString &str)
{
	projectName = str.stripWhiteSpace();
}

void US_GlobalEquil::unload()
{
	if (fitting_widget)
	{
		eqFitter->close();
	}
	if (model_widget)
	{
		model_control->close();
	}
	run_information.clear();
	filenames.clear();
	scan_info.clear();
	scanfit_vector.clear();
	dataset = 0;
	scans_in_list = 0;
	model = -1;
	scans_initialized = false;
	InfoPanel->set_model(model);
	lb_scans->clear();
	pb_details->setEnabled(false);
	pb_fitcontrol->setEnabled(false);
	pb_model->setEnabled(false);
	pb_unload->setEnabled(false);
	pb_diagnostics->setEnabled(false);
	pb_monte_carlo->setEnabled(false);
	pb_scan_check->setEnabled(false);
	pb_float_all->setEnabled(false);
	pb_initialize->setEnabled(false);
	pb_histogram->setEnabled(false);
	pb_model_control->setEnabled(false);
	cnt_scans->setRange(0, 0, 1);
	cnt_scans->setValue(0);
	pb_print->setEnabled(false);
	data_plot->clear();
	data_plot->replot();
	pb_reset_limits->setEnabled(false);
	float_parameters = false;
	buffer_from_db = false;
	vbar_from_db = false;
	InfoPanel->lbl_status2->setText(tr("Please select an edited Equilibrium Dataset with \"Load Data\""));
}

void US_GlobalEquil::print()
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

bool US_GlobalEquil::createHtmlDir()
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

// Function will display a histogram of concentration for all included files
void US_GlobalEquil::histogram()
{
	QPixmap pix;
	QString filename, str;
	float temp_od_limit, min_extinction = 1.0e28, max_extinction = - 1.0, lowest_bin, highest_bin;
	double *xplot, *yplot;
	unsigned int i, j;
	unsigned int ARRAY_SIZE = 50;
	bool first_scan = true, multiple_lambdas = false, extinction_defined = false, plot_concentration = false;
	unsigned int first_lambda = 0;

// first check if there are multiple wavelengths, and if the extinction coefficients have been set:
	for (i=0; i<scanfit_vector.size(); i++)
	{
		if (scanfit_vector[i].FitScanFlag)
		{
			if (first_scan)
			{
				first_lambda = scanfit_vector[i].wavelength;
				first_scan = false;
			}
			else
			{
				if (first_lambda != scanfit_vector[i].wavelength)
				{
					multiple_lambdas = true;
				}
			}
			if (1 != (int) scanfit_vector[i].extinction[0])	// check if the extinction coeff. has been set to
			{ //anything else but 1, otherwise it hasn't been set
				extinction_defined = true;
// we need to find the smallest extinction coefficient listed since it will be used to set the upper
// concentration bin:
				min_extinction = min(min_extinction, scanfit_vector[i].extinction[0]);
				max_extinction = max(max_extinction, scanfit_vector[i].extinction[0]);
			}
		}
	}
	if(multiple_lambdas && !extinction_defined)
	{
		QMessageBox mb(tr("Attention:"),
							tr("You appear to be fitting scans taken at\n"
							"multiple wavelengths, but you did not define\n"
							"multiple extinction coefficients!\n\n"
							"You will not be able to display a\n"
							"concentration corrected histogram."),
							QMessageBox::Warning,
							QMessageBox::Ok,
							QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default,
							QMessageBox::NoButton);
		switch(mb.exec())
		{
			case QMessageBox::Ok:
			{
				break;
			}
			default:
			{
				return;
			}
		}
	}
	if(multiple_lambdas && extinction_defined)
	{
		str = tr("You appear to be fitting multiple wavelengths\n"
					"for which different extinction coefficients are\n"
					"available, would you like to correct the optical\n"
					"density profile for extinction and display real\n"
					"concentrations?\n\n"
					"NOTE: This program only corrects for the 1st component");
	}
	if(!multiple_lambdas && extinction_defined)
	{
		str = tr("You have extinction coefficients defined for\n"
					"your scans, would you like to correct the optical\n"
					"density profile for extinction and display real\n"
					"concentrations?\n\n"
					"NOTE: This program only corrects for the 1st component");
	}
	if (extinction_defined)
	{
		QMessageBox mb(tr("UltraScan"),
						str,
						QMessageBox::Information,
						QMessageBox::Yes,
						QMessageBox::No,
						QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default);
		switch(mb.exec())
		{
			case QMessageBox::Yes:
			{
				plot_concentration = true;
				break;
			}
			case QMessageBox::No:
			{
				plot_concentration = false;
				break;
			}
			default:
			{
				return;
			}
		}
	}
	createHtmlDir();
	xplot = new double [ARRAY_SIZE];
	yplot = new double [ARRAY_SIZE];
	temp_od_limit = od_limit;
	if (temp_od_limit == 0)  // we need to set an arbitrary maximum for bin creation.
	{
		temp_od_limit = 1.0;
	}
	if (plot_concentration)
	{
		lowest_bin = 1e-2 / (1.2 * max_extinction);
		highest_bin = temp_od_limit / (1.2 * min_extinction);
		float increment = (highest_bin - lowest_bin) / ARRAY_SIZE;
		for (i=0; i<ARRAY_SIZE; i++)
		{
			xplot[i] = (double) (lowest_bin + (i * increment));
			yplot[i] = 0.0;
		}
	}
	else
	{
		for (i=0; i<ARRAY_SIZE; i++)
		{
			xplot[i] = (double) (temp_od_limit * (i+1))/ARRAY_SIZE;
			yplot[i] = 0.0;
		}
	}
	if (plot_concentration)
	{
		filename = htmlDir + "/" + projectName + str.sprintf("-%d", model) + ".conc_histogram.dat";
	}
	else
	{
		filename = htmlDir + "/" + projectName + str.sprintf("-%d", model) + ".od_histogram.dat";
	}
	QFile f(filename);
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		for (i=0; i<scanfit_vector.size(); i++)
		{
			if (scanfit_vector[i].FitScanFlag)
			{
				vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin() + i;
				vector<float>::iterator yval_it1 = (*sfv_it).y.begin() + (*sfv_it).start_index;
				vector<float>::iterator yval_it2 = (*sfv_it).y.begin() + (*sfv_it).stop_index;
				if (plot_concentration)
				{
					for (j=0; j<ARRAY_SIZE; j++)
					{
						while ((*yval_it1/(scanfit_vector[i].pathlength * scanfit_vector[i].extinction[0]) < xplot[j]) && (yval_it1 != yval_it2))
						{
							yplot[j] += 1.0;
							yval_it1++;
						}
					}
				}
				else
				{
					for (j=0; j<ARRAY_SIZE; j++)
					{
						while ((*yval_it1 < xplot[j]) && (yval_it1 != yval_it2))
						{
							yplot[j] += 1.0;
							yval_it1++;
						}
					}
				}
			}
		}
		if (plot_concentration)
		{
			ts << tr("Molar Concentration\tFrequency\n");
		}
		else
		{
			ts << tr("Optical Density\tFrequency\n");
		}
		for (i=0; i<ARRAY_SIZE; i++)
		{
			ts << xplot[i] << "\t" << yplot[i] << "\n";
		}
	}
	f.close();
	QWidget *p=0;
	histogram_plot = new QwtPlot(p);
	histogram_plot->setGeometry(global_Xpos + 60, global_Ypos + 60, 600, 500);
	if (plot_concentration)
	{
		histogram_plot->setCaption(tr("Molar Concentration Histogram"));
	}
	else
	{
		histogram_plot->setCaption(tr("Optical Density Histogram"));
	}
	Q_CHECK_PTR(histogram_plot);
	histogram_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	histogram_plot->enableGridXMin();
	histogram_plot->enableGridYMin();
	histogram_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	histogram_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	histogram_plot->setCanvasBackground(USglobal->global_colors.plot);		//new version
	histogram_plot->setMargin(USglobal->config_list.margin);
	histogram_plot->enableOutline(true);
	if (plot_concentration)
	{
		histogram_plot->setAxisTitle(QwtPlot::xBottom, tr("Molar Concentration Bin"));
	}
	else
	{
		histogram_plot->setAxisTitle(QwtPlot::xBottom, tr("Optical Density Bin"));
	}
	histogram_plot->setAxisTitle(QwtPlot::yLeft, tr("Frequency"));
	if (plot_concentration)
	{
		histogram_plot->setTitle(tr("Molar Concentration Histogram"));
	}
	else
	{
		histogram_plot->setTitle(tr("Optical Density Histogram"));
	}
	histogram_plot->setOutlinePen(white);
	histogram_plot->setOutlineStyle(Qwt::Cross);
	histogram_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	histogram_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	histogram_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	histogram_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	histogram_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	histogram_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	histogram_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	histogram_plot->clear();
	long curve = histogram_plot->insertCurve("Histogram");
	histogram_plot->setCurveStyle(curve, QwtCurve::Sticks);
	histogram_plot->setCurvePen(curve, QPen(Qt::red, 7));
	histogram_plot->setCurveData(curve, xplot, yplot, ARRAY_SIZE);
	QwtSymbol symbol;
	symbol.setSize(12);
	symbol.setStyle(QwtSymbol::Ellipse);
	symbol.setPen(Qt::blue);
	symbol.setBrush(Qt::yellow);
	histogram_plot->setCurveSymbol(curve, symbol);
	histogram_plot->show();
	histogram_plot->replot();
	if (plot_concentration)
	{
		filename = htmlDir + "/" + projectName + str.sprintf("-%d", model) + ".conc_histogram.";
	}
	else
	{
		filename = htmlDir + "/" + projectName + str.sprintf("-%d", model) + ".od_histogram.";
	}
	pix = QPixmap::grabWidget(histogram_plot, 2, 2, histogram_plot->width() - 4, histogram_plot->height() - 4);
	pm->save_file(filename, pix);
	delete [] xplot;
	delete [] yplot;
}

// Function will check the ratio of the slopes of a linear fit to 10 starting- and endpoints
// of scan to determine curvature of gradient and display in a window.
void US_GlobalEquil::diagnostics()
{
	float *x_temp, *y_temp, slope1, slope2, intercept, sigma, correlation;
	float xstart, xend, ystart, yend, ratio;
	unsigned int i, j, numpoints, slope_points, count;
	int warnings;

// This is a good place to initialize pathlength and density, this way, they can be changed
// later on in the program and kept changed and don't have to be reset to some specific
// value each time the fitting parameters are re-initialized.

	for (unsigned int i=0; i<scanfit_vector.size(); i++)
	{
		scanfit_vector[i].pathlength = 1.2;
		if(!buffer_from_db)
		{
			scanfit_vector[i].density = DENS_20W;
		}
	}
	QString filename = USglobal->config_list.result_dir.copy() + "/diagnostics.res";
	QString name1, name2;
	QFile f(filename);
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		ts << tr("\n                              PLEASE READ THIS!\n\n");
		ts << tr("Below is a listing of the ratios of slopes in the endpoints of each indicated scan.\n");
		ts << tr("If the ratios are less than 30, then there is little information content in the scan\n");
		ts << tr("and chances are that the experiment was improperly set up and should be repeated.\n\n");
		ts << tr("Additional warnings will be generated if the scan doesn't contain enough datapoints\n");
		ts << tr("or if the experimenter didn't use the majority of the linear absorbance range (at\n");
		ts << tr("least 0.6 OD between 0.0 OD and 0.9 OD) available. \n\n");
		ts << tr("These warnings are for your information only, they have no effect on the rest of the\n");
		ts << tr("program since there are valid exceptions to these warnings when including such scans\n");
		ts << tr("is appropriate. Please refer to the global equilibrium analysis tutorial for more\n");
		ts << tr("information\n\n");
		for (i=0; i<scans_in_list; i++)
		{
			int split = lb_scans->text(i).find("rpm") + 3;
			name1 = lb_scans->text(i);
			name1.truncate(split);
			name2 = lb_scans->text(i);
			name2.remove(0, split+2);
			ts << "                                                                                                                                                                             \n";
			ts << "******************************************************************************************\n";
			ts << tr("Information for Scan ") << name1 << "\n";
			ts << "(" << name2 << ")\n";
			ts << "******************************************************************************************\n\n";
			warnings = 0;
			numpoints = scanfit_vector[i].stop_index - scanfit_vector[i].start_index;
			if (numpoints > 50)
			{
				xstart = scanfit_vector[i].x[scanfit_vector[i].start_index];
				slope_points = (unsigned int) (numpoints/5.0);
//			cout << "Scan: " << i << ", numpoints: " << numpoints << ", slope_points " << slope_points << endl;
				x_temp = new float [slope_points];
				y_temp = new float [slope_points];
				count = 0;
				for (j=scanfit_vector[i].start_index; j<scanfit_vector[i].start_index + slope_points; j++)
				{
					x_temp[count] = scanfit_vector[i].x[j];
					y_temp[count] = scanfit_vector[i].y[j];
					count ++;
				}
//cout << "Count1: " << count << endl;
				linefit (&x_temp, &y_temp, &slope1, &intercept, &sigma, &correlation, (int) slope_points);
				ystart = slope1 * xstart + intercept;
				delete [] x_temp;
				delete [] y_temp;
				count = 0;
				slope_points = (unsigned int) (numpoints/10.0);
				x_temp = new float [slope_points];
				y_temp = new float [slope_points];
				for (j=scanfit_vector[i].stop_index - slope_points; j<scanfit_vector[i].stop_index; j++)
				{
					x_temp[count] = scanfit_vector[i].x[j];
					y_temp[count] = scanfit_vector[i].y[j];
					count ++;
				}
//cout << "Count2: " << count << endl;
				linefit (&x_temp, &y_temp, &slope2, &intercept, &sigma, &correlation, (int) slope_points);
//cout << "Slope: " << slope2 << endl;
//			xval_it2--;
				xend = scanfit_vector[i].x[scanfit_vector[i].stop_index];
				yend = slope2 * xend + intercept;
				delete [] x_temp;
				delete [] y_temp;
				if (slope1 == 0)
				{
					slope1 = 9.999999e-21;
				}
				ratio = slope2/slope1;
				ts << tr("Slope at beginning: ") << slope1 << tr(", Slope at end: ") << slope2
				<< tr(", Ratio: ") << ratio << "\n\n";
				if (ratio > 0 && ratio < 1.5)
				{
					ts << tr("Warning: The ratio is very small - there is probably not enough\n");
					ts << tr("information in this scan.\n");
					ts << tr("Suggestion: use a higher speed. Also, check for aggregation!\n\n");
					warnings++;
				}
				if (slope1 < 0)
				{
					ts << tr("Warning: The start point slope for this scan is negative!\n");
					ts << tr("Possible reasons: excessive noise in the data, or time invariant\n");
					ts << tr("noise from interference data has not been subtracted .\n\n");
					warnings++;
				}
				if (slope2 < 0)
				{
					ts << tr("Warning: The end point slope for this scan is negative!\n");
					ts << tr("Possible reasons: excessive noise in the data, or time invariant\n");
					ts << tr("noise from interference data has not been subtracted.\n\n");
					warnings++;
				}
				if ((yend - ystart) < 0.4)
				{
					ts << tr("Warning: This scan only spans ") << (yend - ystart) << tr(" OD of the possible\n");
					ts << tr("0.9 - 1.0 OD range the instrument allows.\n\n");
					warnings++;
				}
				if (yend < 0.6)
				{
					ts << tr("Warning: This scan's maximum absorbance is only ") << yend << " OD.\n";
					ts << tr("This is lower than the linear range of the XL-A which generally extends\n");
					ts << tr("up to ~0.9 OD. You may be discarding information. Check for Aggregation!\n\n");
					warnings++;
				}
			}
			ts << tr("Number of points in this scan: ") << numpoints ;
			if (numpoints > 100)
			{
				ts << "\n\n";
			}
			if (numpoints > 50 && numpoints < 100)
			{
				ts << tr(" (low!)\n\n");
				warnings++;
			}
			if (numpoints < 50)
			{
				ts << tr(" (too low! Are the data below the OD cutoff?)\n\n");
				warnings++;
			}
			if (warnings == 1)
			{
				ts << tr("There was ") << warnings << tr(" warning generated for this scan.\n");
			}
			else
			{
				ts << tr("There were ") << warnings << tr(" warnings generated for this scan.\n");
			}
			if (warnings > 2)
			{
				ts << tr("Please check the scan to make sure it is appropriate for inclusion in a global fit!\n\n");
			}
			else
			{
				ts << "\n";
			}
			int temp_position = lb_scans->currentItem();
			lb_scans->setSelectionMode(QListBox::NoSelection);
			lb_scans->disconnect();
			if (warnings == 0)
			{
				scanfit_vector[i].FitScanFlag = true;
				scanfit_vector[i].autoExcluded = false;
				lb_scans->changeItem(QPixmap(greenarrow), lb_scans->text(i), i);
			}
			else
			{
				scanfit_vector[i].autoExcluded = true;
				scanfit_vector[i].FitScanFlag = false;
				lb_scans->changeItem(QPixmap(bluearrow), lb_scans->text(i), i);
				scans_have_problems = true;
			}
			connect(lb_scans, SIGNAL(highlighted(int)), SLOT(select_scan(int)));
			connect(lb_scans, SIGNAL(selected(int)), SLOT(change_FitFlag(int)));
			lb_scans->setSelectionMode(QListBox::Single);
			lb_scans->setCurrentItem(temp_position);
		}
	}
	f.close();
	if (scans_have_problems)
	{
		if (show_messages)
		{
			QMessageBox::message(tr("Attention:"), tr("One or more scans have been excluded from the\n"
														"fit. The Diagnostics report will help you to\n"
														"determine which problems occured. You can manually\n"
														"override scan exclusions and include them once\n"
														"you identify the reasons for the exclusion."));
		}
	}
	//view_file(filename);
	TextEdit *e;
	e = new TextEdit();
	e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
	e->load(filename);
	e->show();

	scans_initialized = true;
	pb_histogram->setEnabled(true);
	pb_model_control->setEnabled(true);
	pb_initialize->setEnabled(true);
}

// Function will update the stop index to reflect the od_max setting, it is called
// each time the od_limit is adjusted.
void US_GlobalEquil::update_limit(float new_limit) // each time a new od_limit is entered, a signal is connected to this slot to truncate data to desired OD
{
	int i;
	if (scans_in_list > 0 && new_limit != 0)
	{
		for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
		{
			vector<float>::iterator yval_it = (*sfv_it).y.begin();
			i = 0;
			while ((yval_it != (*sfv_it).y.end()-1) && (*yval_it < new_limit))
			{
				yval_it++;
				i++;
			}
			(*sfv_it).stop_index = i;
		}
	}
	else	// limit is = 0, i.e., no limits on the data, and stop_index needs to be set to the
	{		// last element which is equal to the number of points in the scan minus one.
		for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
		{
			(*sfv_it).stop_index = (*sfv_it).points - 1;
		}
	}
	if (fitting_widget)
	{
		eqFitter->cancel();
	}
}

// function will check if all parameters have been assigned before fitting can commence
void US_GlobalEquil::scan_check()
{
	unsigned int i = 0;
	while (!scanfit_vector[i].FitScanFlag)
	{
		i++;
	}
	first_fitted_scan = i;

// first write out a project file:
	QString ProjectFile = USglobal->config_list.result_dir + "/" + projectName + ".eq-project";
	QFile p(ProjectFile);
	if (p.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&p);
		ts << projectName << "\n";
		p.close();
	}
	else
	{
		QMessageBox::message(tr("UltraScan Error:"), tr("Trouble writing the report file:\n\n")
									+ "   " + ProjectFile + "\n\n"
									+ tr("Please make sure the disk is not full or write protected"));
									return;
	}
	bool critical = false;
	QString filename = USglobal->config_list.result_dir.copy() + "/scan_check.res";
	QString name1, name2;
	QFile f(filename);
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		ts << tr("In order to assure that the proper parameters are used for the fitting process\n");
		ts << tr("the following information has been compiled about the components in your model\n");
		ts << tr("and the scans included in the fit:\n\n");
		ts << tr("Component Information:\n\n");
		for (unsigned int i=0; i<runfit.components; i++)
		{
			if (!runfit.mw_fit[i])
			{
				ts << tr("The molecular weight parameter for component ") << (i+1) << tr(" has been fixed.\n");
			}
			else
			{
				if(runfit.mw[i] == 0.0)
				{
					ts << tr("Although the molecular weight parameter for component ") << (i+1) << tr(" has been floated,\n");
					ts << tr("the value for this parameter is equal to zero - fitting aborted!\n");
					critical = true;
				}
				if(runfit.mw_range[i] == 0.0)
				{
					ts << tr("Although the molecular weight parameter for component ") << (i+1) << tr(" has been floated,\n");
					ts << tr("the range for this parameter is equal to zero - fitting aborted!\n");
					critical = true;
				}
			}
			if (!runfit.vbar20_fit[i])
			{
				ts << tr("The vbar parameter for component ") << (i+1) << tr(" has been fixed.\n");
			}
			else
			{
				if(runfit.vbar20[i] == 0.0)
				{
					ts << tr("Although the vbar parameter for component ") << (i+1) << tr(" has been floated,\n");
					ts << tr("the value for this parameter is equal to zero - fitting aborted!\n");
					critical = true;
				}
				if(runfit.vbar20_range[i] == 0.0)
				{
					ts << tr("Although the vbar parameter for component ") << (i+1) << tr(" has been floated,\n");
					ts << tr("the range for this parameter is equal to zero - fitting aborted!\n");
					critical = true;
				}
			}
			if((unsigned long) (1000000 * runfit.vbar20[i] + 0.5) == 720000)
			{
				ts << tr("The value for vbar for component ") << (i+1) << tr(" has been left at 0.72,\n");
				ts << tr("which is the default value - are you sure you want to use this value?\n");
			}
		}
		for (unsigned int i=0; i<runfit.association_constants; i++)
		{
			if (!runfit.eq_fit[i])
			{
				ts << tr("The equilibrium constant ") << (i+1) << tr(" has been fixed - are you sure you want to do that?.\n");
			}
			else
			{
				if(runfit.eq[i] == 0.0)
				{
					ts << tr("Although the equilibrium constant ") << (i+1) << tr(" has been floated,\n");
					ts << tr("the value for this parameter is equal to zero - fitting aborted!\n");
					critical = true;
				}
				if(runfit.eq_range[i] == 0.0)
				{
					ts << tr("Although the equilibrium constant ") << (i+1) << tr(" has been floated,\n");
					ts << tr("the range for this parameter is equal to zero - fitting aborted!\n");
					critical = true;
				}
			}
		}
		bool same_lambda = true;
		bool same_extinction = true;
		unsigned int test_lambda = scanfit_vector[first_fitted_scan].lambda;
		float test_extinction = scanfit_vector[first_fitted_scan].extinction[0];
		for (unsigned int i=0; i<scanfit_vector.size(); i++)
		{
			if(scanfit_vector[i].FitScanFlag)
			{
				if(scanfit_vector[i].lambda != test_lambda)
				{
					same_lambda = false;
				}
				if(scanfit_vector[i].extinction[0] != test_extinction)
				{
					same_extinction = false;
				}
			}
		}
		if (model >= 4)
		if (!same_lambda && same_extinction)
		{
			ts << tr("\nWarning:\n");
			ts << tr("Your project contains scans with different wavelengths but identical extinction\n");
			ts << tr("coefficients!\n");
		}
		if (!same_lambda && !same_extinction)
		{
			ts << tr("\nWarning:\n");
			ts << tr("Your project contains scans with different wavelengths, make sure the extinction\n");
			ts << tr("coefficients match!\n");
		}
		for (unsigned int i=0; i<scans_in_list; i++)
		{
			int split = lb_scans->text(i).find("rpm") + 3;
			name1 = lb_scans->text(i);
			name1.truncate(split);
			name2 = lb_scans->text(i);
			name2.remove(0, split+2);
			ts << "                                                                                                                                                                             \n";
			ts << "******************************************************************************************\n";
			ts << tr("Information for Scan ") << name1 << "\n";
			ts << "(" << name2 << ")\n";
			ts << "******************************************************************************************\n\n";
			if (!scanfit_vector[i].FitScanFlag)
			{
				ts << tr("This scan has been excluded from the fit.\n");
			}
			else
			{
				for (unsigned int j=0; j<runfit.components; j++)
				{
					if (!scanfit_vector[i].amplitude_fit[j])
					{
						ts << tr("The equilibrium constant ") << (j+1) << tr(" has been fixed - are you sure you want to do that?.\n");
					}
					else
					{
						if(scanfit_vector[i].amplitude[j] == 0.0)
						{
							ts << tr("Although the amplitude for component ") << (j+1) << tr(" has been floated,\n");
							ts << tr("the value for this parameter is equal to zero - fitting aborted!\n");
							critical = true;
						}
						if(scanfit_vector[i].amplitude_range[j] == 0.0)
						{
							ts << tr("Although the amplitude for component ") << (j+1) << tr(" has been floated,\n");
							ts << tr("the range for this parameter is equal to zero - fitting aborted!\n");
							critical = true;
						}
					}
					if (runfit.association_constants > 0)
					{
						if(scanfit_vector[i].extinction[j] == 0.0)
						{
							ts << tr("The extinction coefficient for component ") << (j+1) << tr(" is equal to zero - are you sure?\n");
							ts << tr("(This could be valid if this component doesn't absorb at ") << scanfit_vector[i].wavelength << " nm)\n";
						}
					}
				}
				if (!scanfit_vector[i].baseline_fit)
				{
					ts << tr("The baseline for this scan has been fixed - are you sure you want to do that?.\n");
				}
				else
				{
					if(scanfit_vector[i].baseline_range == 0.0)
					{
						ts << tr("Although the baseline for this scan has been floated,\n");
						ts << tr("the range for this parameter is equal to zero - fitting aborted!\n");
						critical = true;
					}
				}
				if ((unsigned long) (1000000 * scanfit_vector[i].density + 0.5) == 998234 )
				{
					ts << tr("The density setting corresponds to pure water - are you sure you want to use that?.\n");
				}
				ts << "\n";
			}
		}
		if (!critical)
		{
			pb_fitcontrol->setEnabled(true);
		}
		else
		{
			pb_fitcontrol->setEnabled(false);
		}
	}
	f.close();
	//view_file(filename);
	TextEdit *e;
	e = new TextEdit();
	e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
	e->load(filename);
	e->show();
}

// resets start and stop limits of data range to original values, truncates data at od_limit
// is called each time textChanged is activated from InfoPanel's max od line edit.
void US_GlobalEquil::reset_limits()
{
	vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin() + selected_scan;
	vector<float>::iterator yval_it = (*sfv_it).y.begin();

	if (od_limit != 0)
	{
		int i = 0;
		while (yval_it != ((*sfv_it).y.end() - 1) && *yval_it < od_limit)
		{
			yval_it++;
			i++;
		}
		(*sfv_it).stop_index = i;
	}
	else
	{
		(*sfv_it).stop_index = (*sfv_it).points - 1;
	}
	(*sfv_it).start_index = 0;
	pb_reset_limits->setEnabled(false);
	(*sfv_it).limits_modified = false;
	show_scan(selected_scan);
}

void US_GlobalEquil::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/equil0.html");
}

// relay slot which is called when return is pressed on the line edit for max od.
// to make sure the scan image is updated when the max od is changed.
void US_GlobalEquil::call_show_scan()
{
	show_scan(selected_scan);
}

void US_GlobalEquil::quit()
{
	close();
}

// Function is called when the user selects a different scan and during constructor init
// it coordinates the scan counter value with the listbox value and sets the reset limits button
// it calls a scan display function
void US_GlobalEquil::select_scan(int scan)
{
//cout << "scan in select_scan(int): " << scan << endl;
	if (scan < 0)
	{
		scan = 0;
	}
	uint scan1 = (uint) scan;
	select_scan(scan1);
}

void US_GlobalEquil::select_scan(uint scan)
{
//cout << "scan in select_scan(unsigned int): " << scan << endl;
	vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin() + scan;
	if ((*sfv_it).limits_modified)
	{
		pb_reset_limits->setEnabled(true);
	}
	else
	{
		pb_reset_limits->setEnabled(false);
	}
	cnt_scans->setValue(scan+1);
	show_scan(scan);
	if (model_widget)	// also update the model control window if active
	{
		model_control->update_scan_number(scan);
	}
}

// displays scan between start and stop index.

void US_GlobalEquil::show_scan(int scan)
{
//cout << "scan in show_scan(int): " << scan << endl;

	int points=0;
	double *x_temp, *y_temp;
	selected_scan = scan;
	long curve = 0;
	vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin() + scan;
	vector<float>::iterator xval_it = (*sfv_it).x.begin() + (*sfv_it).start_index;
	vector<float>::iterator yval_it = (*sfv_it).y.begin() + (*sfv_it).start_index;
	if ((*sfv_it).limits_modified)
	{
		pb_reset_limits->setEnabled(true);
	}
	else
	{
		pb_reset_limits->setEnabled(false);
	}
	x_temp = new double [(*sfv_it).points];
	y_temp = new double [(*sfv_it).points];
//cout << "selected scan: " << selected_scan << endl;
//cout << "sfv_it.points: " << (*sfv_it).points << ", points:" << points << ", scan: " << scan << endl;
//cout << "stop_index: " << (*sfv_it).stop_index <<  ", start_index: " << (*sfv_it).start_index << endl;
	while (yval_it != (*sfv_it).y.begin() + (*sfv_it).stop_index)
	{
		x_temp[points] = (double) *xval_it;	//need to promote to double for plotting
		y_temp[points] = (double) *yval_it;
		xval_it++;
		yval_it++;
		points++;
	}

	int split = lb_scans->text(scan).find(".") + 1;
	QString xaxis=tr("Radius (cm)"), yaxis="", title, temp;
	title = lb_scans->text(scan);
	title.remove(0, split);
	split = title.find("nm,") - 5;
	title.remove(split, 8);
	split = title.find("rpm") + 3;
	title.truncate(split);
	yaxis.sprintf("%d", scanfit_vector[scan].wavelength);
	if (run_inf.exp_type.absorbance)
	{
		yaxis.prepend(tr("Absorbance ("));
	}
	if (run_inf.exp_type.interference)
	{
		yaxis.prepend(tr("Fringes ("));
	}
	yaxis.append(" nm)");

/*	// for some reason making a two-line title casues the postscript engine to fail and the
	// screen update to fail, only the last line is shown until resized.
	name2 = lb_scans->text(scan);
	name2.remove(0, split+2);
	name1.append("\n" + name2);
*/
	QwtSymbol symbol;
	symbol.setSize(10);
	symbol.setStyle(QwtSymbol::Ellipse);
	symbol.setPen(Qt::blue);
	symbol.setBrush(Qt::yellow);
	data_plot->clear();
	curve = data_plot->insertCurve(lb_scans->text(scan));
	data_plot->setCurveData(curve, x_temp, y_temp, points);
	data_plot->setCurveSymbol(curve, symbol);
	data_plot->setTitle(title);
	data_plot->setAxisTitle(QwtPlot::yLeft, yaxis);
	data_plot->setAxisTitle(QwtPlot::xBottom, xaxis);
	data_plot->replot();
	delete x_temp;
	delete y_temp;
}

// This slot functions as a relay slot for the signal buttonReleased from the model
// control widget when model_widget is true. It serves to update the plot, the scan
// counter and the listbox when a new scan is selected in model control window.
void US_GlobalEquil::activate_scan(double val)
{
	selected_scan = (int) (val - 1.0);
	lb_scans->setCurrentItem(selected_scan);
	select_scan(selected_scan);
}

void US_GlobalEquil::select_model()
{
	SelectModel = new US_SelectModel(&model, true);
	if (float_parameters)	// if we already did a fit with floatable parameters, we need to reset it
	{
		show_messages = false;
		model = 0;
		setup_vectors();
		fix_all();
		show_messages = true;
	}
	if (SelectModel->exec())
	{
		InfoPanel->set_model(model);
		if (model != -1) // -1 = no model was selected
		{
			if (model == 3)	// fixed molecular weight distribution, we need some addtl. info from the user
			{

				unsigned int current_cell=0;
				unsigned int current_channel=0;
				for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
				{
					if((*sfv_it).FitScanFlag)
					{
						current_cell = (*sfv_it).cell;
						current_channel = (*sfv_it).channel;
						break;
					}
				}

				model3_vbar=Vbar20[current_cell][current_channel][0];

				US_SelectModel3 *SelectModel3;
				SelectModel3 = new US_SelectModel3(&mw_upperLimit, &mw_lowerLimit, &runfit.components,
				&model3_vbar, &model3_vbar_flag, run_inf.investigator);
				if (!SelectModel3->exec())
				{
					model = -1;
				}
				else
				{
					mc.mw_upperLimit = mw_upperLimit;
					mc.mw_lowerLimit = mw_lowerLimit;
				}
			}
			if (model == 10 || model == 15 || model == 16 || model == 17 || model == 18 || model == 19)	// User-Defined Monomer - N-mer Equilibrium, we need some addtl. info from the user
			{
				US_SelectModel10 *SelectModel10;
				SelectModel10 = new US_SelectModel10(&runfit.stoich1);
				if (!SelectModel10->exec())
				{
					model = -1;
				}
			}
			if (model == 13)	// User-Defined Monomer - N-mer - M-mer Equilibrium, we need some addtl. info from the user
			{
				US_SelectModel13 *SelectModel13;
				SelectModel13 = new US_SelectModel13(&runfit.stoich1, &runfit.stoich2);
				if (!SelectModel13->exec())
				{
					model = -1;
				}
			}
		}
	}
	InfoPanel->set_model(model);
	pb_monte_carlo->setEnabled(false);
	pb_fitcontrol->setEnabled(false);
	pb_report->setEnabled(false);
	pb_float_all->setEnabled(false);	// whenever a new model is selected, it needs to be re-initialized
	pb_scan_check->setEnabled(false); // before we can check scans and float parameters
	pb_diagnostics->setEnabled(true);
	if (model < 0 && !scans_initialized)
	{
		pb_model_control->setEnabled(false);
		pb_histogram->setEnabled(false);
		pb_initialize->setEnabled(false);
	}
	else if(model < 0 && scans_initialized)
	{
		pb_model_control->setEnabled(false);
		pb_histogram->setEnabled(true);
		pb_initialize->setEnabled(false);
	}
	else if(model > 0 && !scans_initialized)
	{
		pb_model_control->setEnabled(false);
		pb_histogram->setEnabled(false);
		pb_initialize->setEnabled(false);
	}
	else if(model > 0 && scans_initialized)
	{
		pb_model_control->setEnabled(true);
		pb_histogram->setEnabled(true);
		pb_initialize->setEnabled(true);
	}
	setup_vectors();
}

void US_GlobalEquil::update_model_button()
{
	pb_model->setEnabled(true);
	pb_unload->setEnabled(true);
	InfoPanel->lbl_status2->setText(tr("To edit scan limits, click, move and release mouse in the plot area."));
	disconnect(model_control);
}

void US_GlobalEquil::fit_converged()
{
	pb_report->setEnabled(true);
}

void US_GlobalEquil::updateViewer()
{
	pb_report->setEnabled(false);
}

void US_GlobalEquil::fit_suspended()
{
}

void US_GlobalEquil::fit_resumed()
{
}

void US_GlobalEquil::writeScanList(const QString &ProjectFile, const int model)
{
	QString str1, str2;
	QFile p(ProjectFile);
	if (p.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&p);
		str1 = tr("Scanfile Listing for Model ") + modelString[model];
		getHeader(&str2, str1);
		ts << str2;
		ts << tr("<h2>Scanfile Listing for Model ") << modelString[model] << "</h2>\n<p>";
		ts << tr("The following ") << scanfit_vector.size() << tr(" Scans were available in this project. ")
		<< tr("Excluded scans are marked:\n<p>\n<ul>\n");
		for (unsigned int i=0; i<scanfit_vector.size(); i++)
		{
			if (scanfit_vector[i].FitScanFlag)
			{
				ts << "\t<li><font color=green>" << lb_scans->text(i) << " (fitted)</font>\n";
			}
			else
			{
				ts << "\t<li><font color=red>" << lb_scans->text(i) << " (excluded)</font>\n";
			}
		}
		ts << "</ul>\n<p>\n";
		getFooter(&str1);
		ts << str1;
		p.close();
	}
	else
	{
		QMessageBox::message(tr("UltraScan Error:"), tr("Trouble writing the scan listing:\n\n")
									+ "   " + ProjectFile + "\n\n"
									+ tr("Please make sure the disk is not full or write protected"));
									return;
	}
}

void US_GlobalEquil::fitcontrol()
{
	if (fitting_widget)
	{
		if (eqFitter->isVisible())
		{
			eqFitter->raise();
		}
		else
		{
			eqFitter->show();
		}
		return;
	}
	else
	{
		eqFitter = new US_EqFitter(&scanfit_vector,
											&run_information,
											&runfit,
											&model_control,
											model,
											true, //show GUI
											&scan_info,
											&fitting_widget, &projectName, od_limit);
		connect(eqFitter, SIGNAL(fitSuspended()), SLOT(fit_suspended()));
		connect(eqFitter, SIGNAL(fitResumed()), SLOT(fit_resumed()));
		connect(eqFitter, SIGNAL(hasConverged()), SLOT(fit_converged()));
		connect(eqFitter, SIGNAL(newParameters()), SLOT(updateModelWindow()));
		connect(eqFitter, SIGNAL(hasConverged()), SLOT(updateModelWindow()));
		connect(eqFitter, SIGNAL(fittingWidgetClosed()), SLOT(updateViewer()));
		connect(eqFitter, SIGNAL(dataSaved(const QString &, const int)), SLOT(writeScanList(const QString &, const int)));
		eqFitter->show();
		if(model == 3)
		{
			eqFitter->update_nlsMethod(5); //select General Least Squares
			eqFitter->cbb_nlsMethod->setCurrentItem(5);
			eqFitter->lbl_autoconverge->setText("Baseline: ");
			eqFitter->bt_autoconverge->setText(tr("Pos (+)"));
			eqFitter->bt_manualconverge->setText(tr("Neg (-)"));
		}
		else
		{
			eqFitter->lbl_autoconverge->setText("Autoconverge: ");
			eqFitter->bt_autoconverge->setText(tr("Yes"));
			eqFitter->bt_manualconverge->setText(tr("No"));
		}
	}
}

void US_GlobalEquil::report()
{
	eqFitter->view_report();
}

void US_GlobalEquil::monte_carlo()
{
	QString str;
	mc.fitName = fitName;
	mc.experiment = 1; //equilibrium experiment
	mc.parameter.clear();
	mc.run_id = projectName + str.sprintf("-%d", model);
	mc.random_seed = 0;
	mc.iterations = 10000;
	mc.filename = USglobal->config_list.result_dir + "/" + projectName + str.sprintf("-%d.mc", model);
	mc.append = true;
	mc.status = false;
	mc.rule = 0;	// use residual as standard deviation
	mc.percent_bootstrap = 100;
	mc.data_type = 0;
	mc.percent_gaussian = 50;
	mc.parameters = 1;
	mc.parameterNoise = 8.0;	// add noise to the parameter value with a STDD of 10 percent of absolute parameter value
	mc.varianceThreshold = 1.0e-4;
	mc.addNoise = 1; // add random noise to parameters from original fit
	str.sprintf(tr("%d: Variance"), mc.parameters);
	mc.parameter.push_back(str);
	unsigned int i, j, k=0;
	for (i=0; i<scanfit_vector.size(); i++)
	{
		if (scanfit_vector[i].FitScanFlag)
		{
			for(j=scanfit_vector[i].start_index; j<scanfit_vector[i].stop_index; j++)
			{
				original_data.push_back(scanfit_vector[i].y[j]);	// needed for bootstrap
				residuals.push_back(scanfit_vector[i].y[j] - yfit[k]); // needed for bootstrap
				k++;
			}
		}
	}
	for (k=0; k<runfit.components; k++)
	{
		if (runfit.mw_fit[k])
		{
			mc.parameters++;
			str.sprintf(tr("%d: Molecular Weight (Component %d)"), mc.parameters, k+1);
			mc.parameter.push_back(str);
		}
		if (runfit.vbar20_fit[k])
		{
			mc.parameters++;
			str.sprintf(tr("%d: Vbar (Component %d)"), mc.parameters, k+1);
			mc.parameter.push_back(str);
		}
		if (runfit.virial_fit[k])
		{
			mc.parameters++;
			str.sprintf(tr("%d: 2nd Virial Coeff. (Componen %d)"), mc.parameters, k+1);
			mc.parameter.push_back(str);
		}
	}
	for (j=0; j<scanfit_vector.size(); j++)
	{
		if (scanfit_vector[j].FitScanFlag)
		{
			for (k=0; k<runfit.components; k++)
			{
				if (scanfit_vector[j].amplitude_fit[k])
				{
					mc.parameters++;
					str.sprintf(tr("%d: Amplitude (Component. %d, Scan %d)"), mc.parameters, k+1, j+1);
					mc.parameter.push_back(str);
				}
			}
			if (scanfit_vector[j].baseline_fit)
			{
				mc.parameters++;
				str.sprintf(tr("%d: Baseline (Scan %d)"), mc.parameters, j+1);
				mc.parameter.push_back(str);
			}
		}
	}
	for (k=0; k<runfit.association_constants; k++)
	{
		if (runfit.eq_fit[k])
		{
			mc.parameters++;
			str.sprintf(tr("%d: Ln(Association Constant %d)"), mc.parameters, k+1);
			mc.parameter.push_back(str);
		}
	}
	monte_carlo_window = new US_MonteCarlo(&mc, &monte_carlo_widget);
	connect(monte_carlo_window, SIGNAL(iterate()), SLOT(monte_carlo_iterate()));
	monte_carlo_window->show();
}

void US_GlobalEquil::monte_carlo_iterate()
{
	QString str;
	eqFitter->setGUI(false);
	float sigma = 0, *std_dev;
	double *temp_parameters;
	temp_parameters = new double [parameters];
	int now = 0;
	unsigned int point_counter=0, current_point, i, j, k=0;

	QTime t;
	t = QTime::currentTime();
	now = t.msec() + t.second() * 1000 + t.minute() * 60000 + t.hour() * 3600000;
	if (mc.random_seed == 0)
	{
		mc.random_seed = 	now;
	}
#ifdef UNIX
	mc.random_seed -= (int) getpid();
#endif

	str.sprintf(" %d", mc.random_seed);
	monte_carlo_window->le_seed->setText(str);
	srand(mc.random_seed);	//randomize the box muller function
	if (!fitting_widget)
	{
		fitcontrol();
	}
	// keep a copy of the original fit parameters to be used as initial guesses for each new
	// Monte Carlo iteration:
	eqFitter->guess_mapForward(temp_parameters);
	unsigned int iteration = 1;
	QFile mc_f(mc.filename);
	if (!mc.append)
	{
		mc_f.remove();
	}
	QTextStream ts(&mc_f);
	ts.width(14);
	ts.flags(0x1000);

	if(mc.rule > 2) // then we need to initialize a temporary array with the smoothed SD's
	{
		j = residuals.size();
		std_dev = new float [j];
		switch (mc.rule)
		{
			case 3:
			{
				k = 5;
				break;
			}
			case 4:
			{
				k = 10;
				break;
			}
			case 5:
			{
				k = 15;
				break;
			}
			case 6:
			{
				k = 20;
				break;
			}
			case 7:
			{
				k = 25;
				break;
			}
			case 8:
			{
				k = 30;
				break;
			}
		}
		for (i=0; i<j; i++)
		{
			std_dev[i] = fabs(residuals[i]);	// only send in the absolute values of the std. deviations
		}
		gaussian_smoothing(&std_dev, k, j);
		for (i=0; i<j; i++)
		{
			if (residuals[i] < 0)
			{
				std_dev[i] *= -1.0;	// correct the sign of the residuals after averaging
			}
		}
	}
	while (mc.status && iteration < mc.iterations)
	{
		// re-initialize the parameters with the best-fit result and overwrite the parameters from the
		// last Monte Carlo iteration:
		switch (mc.addNoise)
		{
			case 0:
			{
				// do nothing, use parameters from last fit
				break;
			}
			case 1:
			{
				eqFitter->parameter_addRandomNoise(temp_parameters, mc.parameterNoise);
				break;
			}
			case 2:
			{
				eqFitter->parameter_mapBackward(temp_parameters);
				break;
			}
		}
		str.sprintf(" %d", iteration);
		monte_carlo_window->lbl_current_iteration2->setText(str);
		switch (mc.data_type)
		{

			case 0:	// use normal Gaussian random residuals
			{
				point_counter = 0;
				for (i=0; i<scanfit_vector.size(); i++)
				{
					if (scanfit_vector[i].FitScanFlag)
					{
						for(j=scanfit_vector[i].start_index; j<scanfit_vector[i].stop_index; j++)
						{
							switch(mc.rule)
							{
								case 0:
								{
									sigma = residuals[point_counter];
									break;
								}
								case 1:
								{
									sigma = max(std_deviation, residuals[point_counter]);
									break;
								}
								case 2:
								{
									sigma = std_deviation;
									break;
								}
							}
							if (mc.rule > 2 && mc.rule < 9)
							{
								sigma = std_dev[point_counter];
							}
							scanfit_vector[i].y[j] = yfit[point_counter] + box_muller(0, sigma);
//cout << "yfit[" << point_counter<< "]: " << yfit[point_counter] << ", new-y: " << scanfit_vector[i].y[j] << endl;
							point_counter++;
						}
					}
				}
				break;
			}
			case 1:	// use original residuals reordered in bootstrapped fashion
			{
// This algorithm randomizes the order of the residuals of the original fit and applies the absolute
// magnitude of the residuals to different points than where they were originally.
				point_counter = 0;
				for (i=0; i<scanfit_vector.size(); i++)
				{
					if (scanfit_vector[i].FitScanFlag)
					{
						for(j=scanfit_vector[i].start_index; j<scanfit_vector[i].stop_index; j++)
						{
// First, find a random point between the first point of the first scan and the last point of the last scan
							current_point = (unsigned int) (rand()/(RAND_MAX + 1.0) * residuals.size());

// use the magnitude of the residual of the original fit and add it to the fitted solution:
							scanfit_vector[i].y[j] = yfit[point_counter] + residuals[current_point];

// if the point doesn't fall within the percentage of the bootstrapped points, use the original residual instead:
							if (rand()/(RAND_MAX + 1.0) > (mc.percent_bootstrap/100.0))
							{
								scanfit_vector[i].y[j] = original_data[point_counter];
							}
							point_counter++;
						}
					}
				}
				break;
			}
			case 2:	// use a mixture of random Gaussian residuals and bootstrapped residuals:
			{
				point_counter = 0;
				for (i=0; i<scanfit_vector.size(); i++)
				{
					if (scanfit_vector[i].FitScanFlag)
					{
						for(j=scanfit_vector[i].start_index; j<scanfit_vector[i].stop_index; j++)
						{
							if (rand()/(RAND_MAX + 1.0) > (mc.percent_gaussian/100.0))
							{
								switch(mc.rule)
								{
									case 0:
									{
										sigma = residuals[point_counter];
										break;
									}
									case 1:
									{
										sigma = max(std_deviation, residuals[point_counter]);
										break;
									}
									case 2:
									{
										sigma = std_deviation;
										break;
									}
								}
								if (mc.rule > 2 && mc.rule < 9)
								{
									sigma = std_dev[point_counter];
								}
								scanfit_vector[i].y[j] = yfit[point_counter] + box_muller(0, sigma);
							}
							else
							{
// First, find a random point between the first point of the first scan and the last point of the last scan
								current_point = (unsigned int) (rand()/(RAND_MAX + 1.0) * residuals.size());

// use the magnitude of the residual of the original fit and add it to the fitted solution:
								scanfit_vector[i].y[j] = yfit[point_counter] + residuals[current_point];

// if the point doesn't fall within the percentage of the bootstrapped points, use the original residual instead:
								if (rand()/(RAND_MAX + 1.0) > (mc.percent_bootstrap/100.0))
								{
									scanfit_vector[i].y[j] = original_data[point_counter];
								}
							}
							point_counter++;
						}
					}
				}
				break;
			}
		}
		if (model == 3)
		{
			eqFitter->nlsMethod = 5;
		}
		else
		{
			eqFitter->nlsMethod = 0;
		}
		eqFitter->startFit();
		if(eqFitter->return_value == 0)
		{
			iteration ++;
			mc_f.open(IO_WriteOnly | IO_Append);
			ts << tr("Iteration ") << iteration << " (" << mc.random_seed << "): " << eqFitter->variance << " ";
			for (k=0; k<runfit.components; k++)
			{
				if (runfit.mw_fit[k])
				{
					ts << runfit.mw[k] << " ";
				}
				if (runfit.vbar20_fit[k])
				{
					ts << runfit.vbar20[k] << " ";
				}
				if (runfit.virial_fit[k])
				{
					ts << runfit.virial[k] << " ";
				}
			}
			for (j=0; j<scanfit_vector.size(); j++)
			{
				if (scanfit_vector[j].FitScanFlag)
				{
					for (k=0; k<runfit.components; k++)
					{
						if (scanfit_vector[j].amplitude_fit[k])
						{
							ts << scanfit_vector[j].amplitude[k] << " ";
						}
					}
					if (scanfit_vector[j].baseline_fit)
					{
						ts << scanfit_vector[j].baseline << " ";
					}
				}
			}
			for (k=0; k<runfit.association_constants; k++)
			{
				if (runfit.eq_fit[k])
				{
					ts << runfit.eq[k] << " ";
				}
			}

			ts << endl;
			mc_f.flush();
			mc_f.close();
			/*
			if(monte_carlo_widget)
			{
				monte_carlo_window->update_iteration();
			}
			*/
		}
	}
	monte_carlo_window->pb_start->setEnabled(true);
	monte_carlo_window->pb_stop->setEnabled(false);
	mc.random_seed = 0;
	str.sprintf(" %d", mc.random_seed);
	monte_carlo_window->le_seed->setText(str);
	delete [] temp_parameters;
	if(mc.rule > 2) // then we need to delete the temporary array with the smoothed SD's
	{
		delete [] std_dev;
	}
}

void US_GlobalEquil::loadfit()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.eq_fit", 0);
	if (!fn.isEmpty())
	{
		loadfit(fn);
	}
}

void US_GlobalEquil::loadfit(const QString &filename)
{
	unload();	// first unload everything else that may be loaded
	fitName = filename;
	parameters = 0;
	QFile f(filename);
	QString str;
	Q_UINT16 int16;
	Q_UINT32 int32;
	show_messages = false; // when we load the data we don't want to see the run details.
	float val;
	if (f.open(IO_ReadOnly))
	{
		QDataStream ds(&f);
		ds >> int16;	// number of runs
		for (unsigned int i=0; i<(unsigned int) int16; i++)
		{
			ds >> str;
			if (load(USglobal->config_list.result_dir + "/" + str + ".us.e") > 0 )
			{
				//return;
			}
			pb_model->setEnabled(false);
		}
		ds >> int16;	// model number
		model = (int) int16;
		switch (model)
		{
			case 10:
			{
				ds >> int16;
				runfit.stoich1 = int16;
				break;
			}
			case 13:
			{
				ds >> int16;
				runfit.stoich1 = int16;
				ds >> int16;
				runfit.stoich2 = int16;
				break;
			}
			case 16:
			{
				ds >> int16;
				runfit.stoich1 = int16;
				break;
			}
			case 17:
			{
				ds >> int16;
				runfit.stoich1 = int16;
				break;
			}
			case 18:
			{
				ds >> int16;
				runfit.stoich1 = int16;
				break;
			}
			case 19:
			{
				ds >> int16;
				runfit.stoich1 = int16;
				break;
			}
		}
		ds >> projectName;
		ds >> val;
		std_deviation = val;
		le_projectName->setText(projectName);
		ds >> int16;
		if ((unsigned int) int16 != scanfit_vector.size())
		{
			QMessageBox::message(tr("Attention:"), tr("The number of scans loaded does not match\n"
														  "the number of scans in the saved fit.\n\n"
														  "Please make sure the original datafiles that\n"
														  "were associated with this fitting project are\n"
														  "available in the results directory before\n"
														  "proceeding."));
			return;
		}
		ds >> int16;
		runfit.components = (unsigned int) int16;
		ds >> int16;
		runfit.association_constants = int16;
		setup_vectors();
		select_model_control();		// open model control window
		float_parameters = false;  // this boolean will be toggled to "float" in the next call to float_all()
		show_messages =  false; // we don't want to see the message
		float_all();	// this function also initializes the scanfit_vector[].amplitude vector by calling the right function
		show_messages = true;
		for (unsigned int i=0; i<scanfit_vector.size(); i++)
		{
			ds >> int16;
			scanfit_vector[i].FitScanFlag = (bool) int16;
			ds >> int16;
			scanfit_vector[i].autoExcluded = (bool) int16;
			ds >> int32;
			scanfit_vector[i].start_index = (unsigned int) int32;
			ds >> int32;
			scanfit_vector[i].stop_index = (unsigned int) int32;
//cout << "Loadfit:\n " ;
//cout << "Scan: " << i << ": " << scanfit_vector[i].start_index << " - " << scanfit_vector[i].stop_index  << endl;
			ds >> int16;
			scanfit_vector[i].limits_modified = (bool) int16;
			ds >> val;
			scanfit_vector[i].density = val;
			ds >> val;
			scanfit_vector[i].pathlength = val;
			ds >> int16;
			scanfit_vector[i].baseline_fit = (bool) int16;
			if (scanfit_vector[i].baseline_fit)
			{
				parameters++;
			}
			ds >> val;
			scanfit_vector[i].baseline = val;
			ds >> val;
			scanfit_vector[i].baseline_range = val;
			ds >> int16;
			scanfit_vector[i].baseline_bound = (bool) int16;
			for (unsigned int j=0; j<runfit.components; j++)
			{
				ds >> int16;
				scanfit_vector[i].amplitude_fit[j] = (bool) int16;
				if (scanfit_vector[i].amplitude_fit[j])
				{
					parameters++;
				}
				ds >> val;
				scanfit_vector[i].amplitude[j] = val;
				ds >> val;
				scanfit_vector[i].amplitude_range[j] = val;
				ds >> int16;
				scanfit_vector[i].amplitude_bound[j] = (bool) int16;
				ds >> val;
				scanfit_vector[i].extinction[j] = val;
			}
		}
		for (unsigned int i=0; i<runfit.components; i++)
		{
			ds >> int16;
			runfit.mw_fit[i] = (bool) int16;
//cout << "runfit.mw_fit[" << i << "]: " << runfit.mw_fit[i] << endl;
			if (runfit.mw_fit[i])
			{
				parameters++;
			}
			ds >> val;
			runfit.mw[i] = val;
			if (model == 3)
			{
				if (i == 0)
				{
					mc.mw_lowerLimit = runfit.mw[i];
				}
				if (i == runfit.components - 1)
				{
					mc.mw_upperLimit = runfit.mw[i];
				}
			}
//cout << "runfit.mw["<<i<<"]: " << runfit.mw[i] << endl;
			ds >> val;
			runfit.mw_range[i] = val;
//cout << "mw_range["<<i<<"]: " << runfit.mw_range[i] << endl;
			ds >> int16;
			runfit.mw_bound[i] = (bool) int16;
//cout << "runfit.mw_bound["<<i<<"]: " << runfit.mw_bound[i] << endl;
			ds >> int16;
			runfit.vbar20_fit[i] = (bool) int16;
//cout << "runfit.vbar20_fit["<<i<<"]: " << runfit.vbar20_fit[i] << endl;
			if (runfit.vbar20_fit[i])
			{
				parameters++;
			}
			ds >> val;
			runfit.vbar20[i] = val;
//cout << "runfit.vbar20["<<i<<"]: " << runfit.vbar20[i] << endl;
			ds >> val;
			runfit.vbar20_range[i] = val;
			ds >> int16;
			runfit.vbar20_bound[i] = (bool) int16;
//cout << "runfit.vbar20_bound[i]: " << runfit.vbar20_bound[i] << endl;
			ds >> int16;
			runfit.virial_fit[i] = (bool) int16;
//cout << "runfit.vbar20_bound[i]: " << runfit.vbar20_bound[i] << endl;
			if (runfit.virial_fit[i])
			{
				parameters++;
			}
			ds >> val;
			runfit.virial[i] = val;
//cout << "runfit.virial[i]: " << runfit.virial[i] << endl;
			ds >> val;
			runfit.virial_range[i] = val;
//cout << "runfit.virial_range[i]: " << runfit.virial_range[i] << endl;
			ds >> int16;
			runfit.virial_bound[i] = (bool) int16;
//cout << "runfit.virial_range[i]: " << runfit.virial_range[i] << endl;
		}
		for (unsigned int i=0; i<runfit.association_constants; i++)
		{
			ds >> int16;
			runfit.eq_fit[i] = (bool) int16;
//cout << "runfit.eq_fit["<<i<<"]: " << runfit.eq_fit[i] << endl;
			if (runfit.eq_fit[i])
			{
				parameters++;
			}
			ds >> val;
			runfit.eq[i] = val;
//cout << "runfit.eq["<<i<<"]: " << runfit.eq[i] << endl;
			ds >> val;
			runfit.eq_range[i] = val;
//cout << "runfit.eq_range[i]: " << runfit.eq_range[i] << endl;
			ds >> int16;
			runfit.eq_bound[i] = (bool) int16;
//cout << "runfit.eq_bound[i]: " << runfit.eq_bound[i] << endl;
		}
		ds >> int16;
		fitpoints = int16;
//cout << "fitpoints: " << fitpoints << endl;
		yfit.clear();
		for (unsigned int i=0; i<fitpoints; i++)
		{
			ds >> val;
			yfit.push_back(val);
		}
		if (!ds.atEnd())
		{
			ds >> od_limit;
		}
		QString od;

		InfoPanel->disconnect();
		InfoPanel->le_max_od->setText(od.sprintf("%4.3f", od_limit));
		connect(InfoPanel, SIGNAL(limitChanged(float)), SLOT(update_limit(float)));
		connect(InfoPanel->le_max_od, SIGNAL(returnPressed()), SLOT(call_show_scan()));

		f.close();
		InfoPanel->set_model(model);
		str = tr("Model is locked - to change model, close Model and Fitting Window first!");
		InfoPanel->lbl_status2->setText(str);
		pb_model_control->setEnabled(true);
		pb_initialize->setEnabled(true);
		pb_float_all->setEnabled(true);
		pb_scan_check->setEnabled(true);
		pb_histogram->setEnabled(true);
		pb_diagnostics->setEnabled(true);
		pb_unload->setEnabled(true);
		pb_fitcontrol->setEnabled(true);
		pb_monte_carlo->setEnabled(true);
		pb_report->setEnabled(true);
		fitcontrol();
		eqFitter->startFit();
		for (unsigned int i=0; i<scanfit_vector.size(); i++)
		{
			selected_scan = i;
			update_pixmap();
		}
		selected_scan = 0;
		select_scan(0);
		lb_scans->setSelected(0, true);
//		show_scan(0);
		updateModelWindow();
	}
	scans_initialized = true;
	show_messages = true; // now that we are done loading the fit we want to see all messages by default
}

void US_GlobalEquil::updateModelWindow()
{
	if (model_widget)
	{
		model_control->update_component_labels();
		model_control->update_scan_number(selected_scan);
	}
}

void US_GlobalEquil::fix_all()
{
	float_parameters = false;
	for (unsigned int i=0; i<runfit.components; i++)
	{
		runfit.mw_fit[i] = false;
	}
	model_control->mw_float(false);
	for (unsigned int i=0; i<runfit.association_constants; i++)
	{
		runfit.eq_fit[i] = false;
	}
	for (unsigned int i=0; i<scanfit_vector.size(); i++)
	{
		scanfit_vector[i].baseline_fit = false;
		for (unsigned int j=0; j<runfit.components; j++)
		{
			scanfit_vector[i].amplitude_fit[j] = false;
		}
	}
	model_control->eqconst1_float(false);
	model_control->eqconst2_float(false);
	model_control->eqconst3_float(false);
	model_control->eqconst4_float(false);
	model_control->update_scan_info(selected_scan);
	if (show_messages)
	{
		QMessageBox::message(tr("Attention:"),	tr("All parameters have been set to \"fixed\"."));
	}
}

void US_GlobalEquil::float_all()
{
	if (model == -1)
	{
			QMessageBox::message(tr("Attention:"),tr("Please select a Model first!\n\n"
										"Click on \"Select Model\""));
			return;
	}
	if (!model_widget)
	{
		select_model_control();
	}
	float_parameters = true;
	if (model != 3)
	{
		for (unsigned int i=0; i<runfit.components; i++)
		{
			runfit.mw_fit[i] = true;
		}
	}
	else
	{
		for (unsigned int i=0; i<runfit.components; i++)
		{
			runfit.mw_fit[i] = false;
		}
	}
	if (model != 3)	// set the checkbox "on" for floating MW
	{
		model_control->mw_float(true);
	}
	else
	{
		model_control->mw_float(false);
	}
	for (unsigned int i=0; i<runfit.association_constants; i++)
	{
		runfit.eq_fit[i] = true;
	}
	if (model == 3)
	{
		for (unsigned int i=0; i<scanfit_vector.size(); i++)
		{
			if (scanfit_vector[i].FitScanFlag)
			{
				for (unsigned int j=0; j<runfit.components; j++)
				{
					scanfit_vector[i].amplitude_fit[j] = true;
				}
				scanfit_vector[i].baseline_fit = true;
			}
			else
			{
				for (unsigned int j=0; j<runfit.components; j++)
				{
					scanfit_vector[i].amplitude_fit[j] = false;
				}
				scanfit_vector[i].baseline_fit = false;
			}
		}
	}
	else
	{
		for (unsigned int i=0; i<scanfit_vector.size(); i++)
		{
			if (scanfit_vector[i].FitScanFlag)
			{
				scanfit_vector[i].baseline_fit = true;
				for (unsigned int j=0; j<runfit.components; j++)
				{
					scanfit_vector[i].amplitude_fit[j] = true;
				}
			}
			else
			{
				scanfit_vector[i].baseline_fit = false;
				for (unsigned int j=0; j<runfit.components; j++)
				{
					scanfit_vector[i].amplitude_fit[j] = false;
				}
			}
		}
	}
	if ((model >= 4) && (model <= 13))
	{
		model_control->eqconst1_float(true);
		model_control->eqconst2_float(false);
		model_control->eqconst3_float(false);
		model_control->eqconst4_float(false);
	}
	if ((model == 14) || (model == 15))
	{
		model_control->eqconst1_float(false);
		model_control->eqconst2_float(false);
		model_control->eqconst3_float(false);
		model_control->eqconst4_float(false);
	}
	if (model == 16)
	{
		runfit.mw_fit[1] = false;
		model_control->eqconst1_float(true);
		model_control->eqconst1_float(false);
		model_control->eqconst2_float(false);
		model_control->eqconst3_float(false);
		model_control->eqconst4_float(false);
	}
	if (model == 17)
	{
		runfit.mw_fit[1] = false;
		model_control->eqconst1_float(true);
		model_control->eqconst1_float(false);
		model_control->eqconst2_float(false);
		model_control->eqconst3_float(false);
		model_control->eqconst4_float(false);
	}
	if (model == 18)
	{
		runfit.mw_fit[1] = false;
		model_control->eqconst1_float(false);
		model_control->eqconst1_float(false);
		model_control->eqconst2_float(false);
		model_control->eqconst3_float(false);
		model_control->eqconst4_float(false);
	}
	if (model == 19)
	{
		model_control->eqconst1_float(true);
		model_control->eqconst1_float(false);
		model_control->eqconst2_float(false);
		model_control->eqconst3_float(false);
		model_control->eqconst4_float(false);
	}
	model_control->update_scan_info(selected_scan);
	switch (model)
	{
		case 0: // 1-Component, Ideal
		{
			if (show_messages)
			{
				QMessageBox::message(tr("Attention:"),	tr("The molecular weight parameter as well\n"
																"as all baseline and amplitude parameters\n"
																"for each scan have been set to \"float\""));
			}
			break;
		}
		case 1: // 2-Component, Ideal, Noninteracting
		{
			if (show_messages)
			{
				QMessageBox::message(tr("Attention:"),	tr("The molecular weight parameters as well\n"
																"as all baseline and amplitude parameters\n"
																"for each scan have been set to \"float\""));
			}
			break;
		}
		case 2: // 3-Component, Ideal, Noninteracting
		{
			if (show_messages)
			{
				QMessageBox::message(tr("Attention:"),	tr("The molecular weight parameters as well\n"
																"as all baseline and amplitude parameters\n"
																"for each scan have been set to \"float\""));
			}
			break;
		}
		case 3: // Fixed Molecular Weight Distribution
		{
			if (show_messages)
			{
				QMessageBox::message(tr("Attention:"),	tr("The baseline and amplitude parameters\n"
																"for each scan have been set to \"float\"\n"
																"(the molecular weight parameters are\n"
																"always fixed for this model)"));
			}
			break;
		}
	}
	if (model == 4  ||	// Monomer-Dimer Equilibrium
		 model == 5  ||	// Monomer-Trimer Equilibrium
		 model == 6  ||	// Monomer-Tetramer Equilibrium
		 model == 7  ||	// Monomer-Pentamer Equilibrium
		 model == 8  ||	// Monomer-Hexamer Equilibrium
		 model == 9  ||	// Monomer-Heptamer Equilibrium
		 model == 10)		// User-Definable Monomer-Nmer Equilibrium
	{
		if (show_messages)
		{
			QMessageBox::message(tr("Attention:"),	tr("The molecular weight parameter and all\n"
															"baseline and amplitude parameters for each\n"
															"scan have been set to \"float\"."));
		}
	}
	if (model == 16)		// User-Definable Monomer-Nmer Equilibrium, some monomer is incompetent
	{
		runfit.mw_fit[1] = false; //molecular weight for the second component is constrained.
		for (unsigned int i=0; i<scanfit_vector.size(); i++)
		{
			scanfit_vector[i].amplitude_fit[1] = false;
		}
		if (show_messages)
		{
			QMessageBox::message(tr("Attention:"),	tr("The molecular weight parameter for the\n"
																	"monomer (but not the incompetent monomer,\n"
																	"and for the baseline and amplitude parameters\n"
																	"for each scan have been set to \"float\"."));
		}
	}
	if (model == 17)		// User-Definable Monomer-Nmer Equilibrium, some Nmer is incompetent
	{
		runfit.mw_fit[1] = false; //molecular weight for the second component is constrained.
		for (unsigned int i=0; i<scanfit_vector.size(); i++)
		{
			scanfit_vector[i].amplitude_fit[1] = false;
		}
		if (show_messages)
		{
			QMessageBox::message(tr("Attention:"),	tr("The molecular weight parameter and all\n"
															"baseline and amplitude parameters for each\n"
															"scan have been set to \"float\"."));
		}
	}
	if (model == 18)		// User-Definable Monomer-Nmer Equilibrium, a portion of both species incompetent
	{
		runfit.mw_fit[1] = false; //molecular weight for the second component is constrained.
		if (show_messages)
		{
			QMessageBox::message(tr("Attention:"),	tr("The molecular weight parameter and all\n"
															"baseline and amplitude parameters for each\n"
															"scan have been set to \"float\"."));
		}
	}
	if (model == 19)		// User-Definable Monomer-Nmer Equilibrium plus contaminant
	{
		runfit.mw_fit[1] = false; //molecular weight for the second component is constrained.
		for (unsigned int i=0; i<scanfit_vector.size(); i++)
		{
			scanfit_vector[i].amplitude_fit[1] = false;
		}
		if (show_messages)
		{
			QMessageBox::message(tr("Attention:"),	tr("The molecular weight parameter and all\n"
															"baseline and amplitude parameters for each\n"
															"scan have been set to \"float\"."));
		}
	}
	else if (model == 11	||	// Monomer-Dimer-Trimer Equilibrium
			   model == 12	||	// Monomer-Dimer-Tetramer Equilibrium
			   model == 13)	// User-Definable Monomer - N-mer - M-mer Equilibrium
	{
		if (show_messages)
		{
			QMessageBox::message(tr("Attention:"),	tr("The molecular weight parameter, the first\n"
															"equilibrium constant, as well as all\n"
															"baseline and amplitude parameters for each\n"
															"scan have been set to \"float\".\n"
															"The second equilibrium constant will be\n"
															"floated after the fit converged on the\n"
															"monomer molecular weight and the first\n"
															"equilibrium constant."));
		}
	}
	else if (model == 14)	// 2-component hetero associating model
	{
		if (show_messages)
		{
			QMessageBox::message(tr("Attention:"),	tr("The molecular weight parameter, as well\n"
															"as all baseline and amplitude parameters\n"
															"for each scan have been set to \"float\".\n"
															"The equilibrium constant will be floated\n"
															"after the fit converged on the molecular weights."));
		}
	}
	else if (model == 15)	// 2-component hetero associating model plus self-association
	{
		if (show_messages)
		{
			QMessageBox::message(tr("Attention:"),	tr("The molecular weight parameter, as\n"
															"well as all baseline and amplitude\n"
															"parameters for each scan have been set\n"
															"to \"float\".	The equilibrium constants\n"
															"will be floated in steps after the fit\n"
															"converged on the molecular weights."));
		}
	}
}

void US_GlobalEquil::setup_vectors()
{
	switch (model)
	{
		case -1:
		{
			QMessageBox::message(tr("Attention:\n"),tr("Please create a Model first!\n\n"
										"Click on \"Create Model\""));
			return;
		}
		case 0:	// 1-Component, Ideal
		{
			runfit.components = 1;
			runfit.association_constants = 0;
			break;
		}
		case 1:	// 2-Component, Ideal, Noninteracting
		{
			runfit.components = 2;
			runfit.association_constants = 0;
			break;
		}
		case 2:	// 3-Component, Ideal, Noninteracting
		{
			runfit.components = 3;
			runfit.association_constants = 0;
			break;
		}
		case 3:	// Fixed Molecular Weight Distribution
		{			// the runfit.components are initialized in the select_model() routine
			runfit.association_constants = 0;
			break;
		}
		case 4:	// Monomer-Dimer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 5:	// Monomer-Trimer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 6:	// Monomer-Tetramer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 7:	// Monomer-Pentamer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 8:	// Monomer-Hexamer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 9:	// Monomer-Heptamer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 10:	// User-Defined Monomer - N-mer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 11:	// Monomer-Dimer-Trimer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 2;
			break;
		}
		case 12:	// Monomer-Dimer-Tetramer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 2;
			break;
		}
		case 13:	// User-Defined Monomer - N-mer - M-mer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 2;
			break;
		}
		case 14:	// 2-Component Hetero-Associating Equilibrium
		{
			runfit.components = 2;
			runfit.association_constants = 1;
			break;
		}
		case 15:	// 2-Component Hetero-Associating Equilibrium plus one species self associating
		{
			runfit.components = 2;
			runfit.association_constants = 2;
			break;
		}
		case 16:	// User-Defined Monomer-Nmer, some monomer is incompetent
		{
			runfit.components = 2;
			runfit.association_constants = 1;
			break;
		}
		case 17:	// User-Defined Monomer-Nmer, some Nmer is incompetent
		{
			runfit.components = 2;
			runfit.association_constants = 1;
			break;
		}
		case 18:	// User-Defined irreversible Monomer-Nmer
		{
			runfit.components = 2;
			runfit.association_constants = 0;
			break;
		}
		case 19:	// User-Defined Monomer-Nmer plus contaminant
		{
			runfit.components = 2;
			runfit.association_constants = 1;
			break;
		}
	}
	runfit.mw.clear();
	runfit.mw_index.clear();
	runfit.mw_range.clear();
	runfit.mw_fit.clear();
	runfit.mw_bound.clear();
	runfit.vbar20.clear();
	runfit.vbar20_index.clear();
	runfit.vbar20_range.clear();
	runfit.vbar20_fit.clear();
	runfit.vbar20_bound.clear();
	runfit.virial.clear();
	runfit.virial_index.clear();
	runfit.virial_range.clear();
	runfit.virial_fit.clear();
	runfit.virial_bound.clear();
	for (unsigned int i=0; i<runfit.components; i++)
	{
		runfit.mw.push_back(0.0);
		runfit.mw_index.push_back(0);
		runfit.mw_range.push_back(0.0);
		runfit.mw_fit.push_back(false);
		runfit.mw_bound.push_back(false);
		runfit.vbar20.push_back(.72);
		runfit.vbar20_index.push_back(0);
		runfit.vbar20_range.push_back(.144);
		runfit.vbar20_fit.push_back(false);
		runfit.vbar20_bound.push_back(false);
		runfit.virial.push_back(0.0);
		runfit.virial_index.push_back(0);
		runfit.virial_range.push_back(0.0);
		runfit.virial_fit.push_back(false);
		runfit.virial_bound.push_back(false);
	}
	for (unsigned int j=0; j<runfit.association_constants; j++)
	{
		runfit.eq_fit[j] = false;
		runfit.eq_bound[j] = false;
	}
	for (unsigned int i=0; i<scanfit_vector.size(); i++)
	{
		scanfit_vector[i].baseline = 0.0;
		scanfit_vector[i].baseline_index = 0;
		scanfit_vector[i].baseline_range = 0.0;
		scanfit_vector[i].baseline_fit = false;
		scanfit_vector[i].baseline_bound = false;
		scanfit_vector[i].amplitude.clear();
		scanfit_vector[i].amplitude_index.clear();
		scanfit_vector[i].amplitude_range.clear();
		scanfit_vector[i].amplitude_fit.clear();
		scanfit_vector[i].amplitude_bound.clear();
		scanfit_vector[i].extinction.clear();
		for (unsigned int j=0; j<runfit.components; j++)
		{
			scanfit_vector[i].amplitude.push_back(0.0);
			scanfit_vector[i].amplitude_index.push_back(0);
			scanfit_vector[i].amplitude_range.push_back(0.0);
			scanfit_vector[i].amplitude_fit.push_back(false);
			scanfit_vector[i].amplitude_bound.push_back(false);
			scanfit_vector[i].extinction.push_back(1.0);
		}
	}
}

void US_GlobalEquil::initialize()
{
//check the first useful scan
	QString str, str1,str2;
	unsigned int current_cell=0;
	unsigned int current_channel=0;
	if (fitting_widget)
	{
		eqFitter->initialized_1 = false;
		eqFitter->initialized_2 = false;
	}
	for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
	{
		if((*sfv_it).FitScanFlag)
		{
			current_cell = (*sfv_it).cell;
			current_channel = (*sfv_it).channel;
			break;
		}
	}
//cout << "before: mw1: " << runfit.mw[0] << ", mw2: " << runfit.mw[1] << endl;
	bool update_mw=true;
	float portion, total;
	if(model < 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("Please select a Model first!\n\n"
									"Click on \"Select Model\""));
		return;
	}
	unsigned int i = 0;
	for (vector <struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
	{
		if((*sfv_it).FitScanFlag)
		{
			i++;
		}
	}
	if (i < 1)
	{
		QMessageBox::message(tr("Attention:\n"),tr("There are no scans to fit!\n\n"
									"Please select one or more scans to be fitted."));
		return;
	}
	if (!model_widget)
	{
		select_model_control();
	}
	if (runfit.mw[0] <= 0)	// if the MW has been initialized already, ask user if he wants to overwrite or
	{								// use exisiting initialization.
		runfit.mw[0] = linesearch();
	}
	else
	{
		QMessageBox mb(tr("UltraScan"),
							tr("Do you want to use the currently defined molecular weight for the parameter\n"
							"initialization or calculate a newly initialized molecular weight?"),
		QMessageBox::Information,
		QMessageBox::Yes | QMessageBox::Default,
		QMessageBox::No,
		QMessageBox::Cancel | QMessageBox::Escape);
		mb.setButtonText(QMessageBox::Yes, tr("New Molecular Weight"));
		mb.setButtonText(QMessageBox::No, tr("Current Molecular Weight"));
		switch(mb.exec())
		{
			case QMessageBox::Yes:
			{
				runfit.mw[0] = linesearch();
				runfit.mw_range[0] = runfit.mw[0]/5;
				update_mw = true;
				break;
			}
			case QMessageBox::No:
			{
				calc_testParameter(runfit.mw[0]);
				runfit.mw_range[0] = runfit.mw[0]/5;
				update_mw = false;
				break;
			}
			case QMessageBox::Cancel:
			{
				update_mw = false;
				return;
			}
			case QMessageBox::Default:
			{
				runfit.mw[0] = linesearch();
				runfit.mw_range[0] = runfit.mw[0]/5;
				break;
			}
		}
	}
	switch (model)
	{
		case 0:	// 1-Component, Ideal
		{
			runfit.vbar20[0] = Vbar20[current_cell][current_channel][0];
			runfit.vbar20_range[0] = runfit.vbar20[0]/5.0;
//		cout<<runfit.vbar20[0]<<endl;
			break; // do nothing, all parameters are already assigned in the calc_testParameter routine.
		}
		case 1:	// 2-Component, Ideal
		{
			portion = runfit.mw[0];
			if (update_mw)
			{
				runfit.mw[0] = portion - portion/5;	// subtract 20% from the center
				runfit.mw[1] = portion + portion/5;	// add 20% to the center
			}
			for (i=0; i<runfit.components; i++)
			{
				runfit.mw_range[i] = runfit.mw[i]/5.0;
				runfit.vbar20[i] = Vbar20[current_cell][current_channel][i];
				runfit.vbar20_range[i] = runfit.vbar20[i]/5.0;
			}
		//set second vbar value as same as first one if default value in second one
			if((int) (100*runfit.vbar20[1]) == 72)
			{
				runfit.vbar20[1]=runfit.vbar20[0];
				runfit.vbar20_range[1] = runfit.vbar20[1]/5.0;
			}
			for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
			{
				if((*sfv_it).FitScanFlag)
				{
					portion = exp((*sfv_it).amplitude[0])/2;
//QMessageBox::message("warning", "case 1: portion = "+QString::number(portion));
					(*sfv_it).amplitude[0] = log(portion*.7);
					(*sfv_it).amplitude[1] = log(portion*.3);

					(*sfv_it).amplitude_range[0] = (*sfv_it).amplitude[0] / 5.0;
					(*sfv_it).amplitude_range[1] = (*sfv_it).amplitude_range[0];
/*
					(*sfv_it).amplitude[0] = log(exp((*sfv_it).amplitude[0]) / 2.0); // use half for each
					(*sfv_it).amplitude[1] = (*sfv_it).amplitude[0];
*/
				}
			}
			break;
		}
		case 2:	// 3-Component, Ideal
		{
			float molweight = runfit.mw[0];
			if (update_mw)
			{
				runfit.mw[0] = molweight - molweight/5;	// subtract 33% from the center
				runfit.mw[1] = molweight;
				runfit.mw[2] = molweight + molweight/5;	// add 33% to the center
			}
			for (i=0; i<runfit.components; i++)
			{
				runfit.mw_range[i] = runfit.mw[i]/5.0;
				runfit.vbar20[i] = Vbar20[current_cell][current_channel][i];
				runfit.vbar20_range[i] = Vbar20[current_cell][current_channel][i]/5.0;
			}
			//set second vbar value as same as first one if default value in second one
			if((int)(100*runfit.vbar20[1])==72)
			{
				runfit.vbar20[1]=runfit.vbar20[0];
				runfit.vbar20_range[1] = runfit.vbar20[1]/5.0;
			}
		//set third vbar value as same as first one if default value in third one
			if((int)(100*runfit.vbar20[2])==72)
			{
				runfit.vbar20[2]=runfit.vbar20[0];
				runfit.vbar20_range[2] = runfit.vbar20[2]/5.0;
			}

			total = runfit.mw[0] + runfit.mw[1] + runfit.mw[2];
			for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
			{
				if((*sfv_it).FitScanFlag)
				{
// to estimate a good value for all three amplitudes, use the product of the estimated amplitude
// with the first molecular weight as a measure for the total. Divide the total by three. Each
// third gets then divided into the remaining molecular weights to get a fairly good estimate for
// the amplitudes:
// a1 * mw1 = total
// portion = total/3
// desired: a1 * mw1 + a2 * mw2 + a3 * mw3 = total
// therefore: a1 * mw1 = portion, and a1 = portion/mw1, a2 = portion/mw2 and a3 = portion/mw3

					portion = exp((*sfv_it).amplitude[0])/3;
//QMessageBox::message("warning", "case 2: portion = "+QString::number(portion));
					(*sfv_it).amplitude[0] = log(portion*.6);
					(*sfv_it).amplitude[1] = log(portion*.3);
					(*sfv_it).amplitude[2] = log(portion*.1);
/*
					(*sfv_it).amplitude[0] = log(exp((*sfv_it).amplitude[0]) / 3.0); // use a third for each
					(*sfv_it).amplitude[1] = (*sfv_it).amplitude[0];
					(*sfv_it).amplitude[2] = (*sfv_it).amplitude[0];
*/
					(*sfv_it).amplitude_range[0] = (*sfv_it).amplitude[0] / 5.0;
					(*sfv_it).amplitude_range[1] = (*sfv_it).amplitude_range[0];
					(*sfv_it).amplitude_range[2] = (*sfv_it).amplitude_range[0];
				}
			}
			break;
		}
		case 3: // Fixed Molecular Weight Distribution
		{
			float temp;
			unsigned int i;
			temp = log(1e-7/runfit.components);
			for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
			{
				if((*sfv_it).FitScanFlag)
				{
					for (i=0; i<runfit.components; i++)
					{
						(*sfv_it).amplitude[i] 			= temp;
						(*sfv_it).amplitude_range[i]	= (*sfv_it).amplitude[i] / 5.0;
						if (runfit.components == 1)
						{
							runfit.mw[i] = mw_lowerLimit;
						}
						else
						{
							runfit.mw[i] = mw_lowerLimit + (i * (mw_upperLimit - mw_lowerLimit)/(runfit.components-1));
						}
						(*sfv_it).baseline = 0.0;
						(*sfv_it).baseline_range = 0.05;
						if (model3_vbar_flag)
						{
							runfit.vbar20[i] = model3_vbar;
							runfit.vbar20_range[i] = model3_vbar/5.0;
						}
						else
						{
							runfit.vbar20[i] = 0.72;
							runfit.vbar20_range[i] = 0.72/5.0;
						}
					}
				}
			}
			break;
		}
	}
	if ((model >= 4) && (model <= 13))
	{
		runfit.eq[0] = -1;
		runfit.eq[1] = -1.0e4;
		runfit.eq_range[0] = 5.0;
		runfit.eq_range[1] = 5.0;
		runfit.vbar20[0] = Vbar20[current_cell][current_channel][0];
		runfit.vbar20_range[0] = Vbar20[current_cell][current_channel][0]/5.0;
		model_control->fp_eqconst1->updateValue(runfit.eq[0]);
		model_control->fp_eqconst2->updateValue(runfit.eq[1]);
	}
	if (model == 14)     //A+B<=>AB
	{
		float molweight = runfit.mw[0];
		for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
		{
			if((*sfv_it).FitScanFlag)
			{
//			QMessageBox::message("warning", "case 14: (*sfv_it).amplitude[0] = "+QString::number((*sfv_it).amplitude[0]));
				(*sfv_it).amplitude[0] = log(exp((*sfv_it).amplitude[0]) / 2.0); // use half for each
				(*sfv_it).amplitude_range[0] = (*sfv_it).amplitude[0] / 5.0;
				(*sfv_it).amplitude[1] = (*sfv_it).amplitude[0];
				(*sfv_it).amplitude_range[1] = (*sfv_it).amplitude[0];
			}
		}
		if (update_mw)
		{
			runfit.mw[0] = molweight - molweight/5;	// subtract 20% from the center
			runfit.mw[1] = molweight + molweight/5;	// add 20% to the center
		}
		runfit.mw_range[0] = runfit.mw[0]/5.0;
		runfit.mw_range[1] = runfit.mw[1]/5.0;
		runfit.vbar20[0] = Vbar20[current_cell][current_channel][0];
		runfit.vbar20_range[0] = Vbar20[current_cell][current_channel][0]/5.0;
		runfit.vbar20[1] = Vbar20[current_cell][current_channel][1];
		if((int) (100*runfit.vbar20[1]) == 72)
		{
			runfit.vbar20[1]=runfit.vbar20[0];
		}
		runfit.vbar20_range[1] = runfit.vbar20[1]/5.0;
		runfit.eq[0] = -1.0e4;
		runfit.eq_range[0] = 5.0e2;
		model_control->fp_eqconst1->updateValue(runfit.eq[0]);
	}
	if (model == 15) // A+B<=>AB, nA<=>An
	{
		float molweight = runfit.mw[0];
		for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
		{
			if((*sfv_it).FitScanFlag)
			{
//			QMessageBox::message("warning", "case 14: (*sfv_it).amplitude[0] = "+QString::number((*sfv_it).amplitude[0]));
				(*sfv_it).amplitude[0] = log(exp((*sfv_it).amplitude[0]) / 2.0); // use half for each
				(*sfv_it).amplitude_range[0] = (*sfv_it).amplitude[0] / 5.0;
				(*sfv_it).amplitude[1] = (*sfv_it).amplitude[0];
				(*sfv_it).amplitude_range[1] = (*sfv_it).amplitude[0];
			}
		}
		if (update_mw)
		{
			runfit.mw[0] = molweight - molweight/5;	// subtract 20% from the center
			runfit.mw[1] = molweight + molweight/5;	// add 20% to the center
		}
		runfit.mw_range[0] = runfit.mw[0]/5.0;
		runfit.mw_range[1] = runfit.mw[1]/5.0;
		runfit.vbar20[0] = Vbar20[current_cell][current_channel][0];
		runfit.vbar20_range[0] = Vbar20[current_cell][current_channel][0]/5.0;
		runfit.vbar20[1] = Vbar20[current_cell][current_channel][1];
		if((int) (100*runfit.vbar20[1]) == 72)
		{
			runfit.vbar20[1]=runfit.vbar20[0];
		}
		runfit.vbar20_range[1] = runfit.vbar20[1]/5.0;
		runfit.eq[0] = -1.0e4;
		runfit.eq_range[0] = 5.0e-2;
		model_control->fp_eqconst1->updateValue(runfit.eq[0]);
		runfit.eq[1] = -1.0e4;
		runfit.eq_range[0] = 5.0e-2;
		model_control->fp_eqconst2->updateValue(runfit.eq[1]);
	}
	if (model == 16)	// monomer - N-mer plus incompetent monomer
	{
		for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
		{
			if((*sfv_it).FitScanFlag)
			{
				(*sfv_it).amplitude[1] = -1e4;
				(*sfv_it).amplitude_range[1] = 1e-3;
			}
		}
		runfit.mw[1] = runfit.mw[0];	// incompetent monomer
		runfit.mw_range[0] = runfit.mw[0]/5.0;
		runfit.mw_range[1] = runfit.mw[1]/5.0;
		runfit.eq[0] = -0.1;
		runfit.eq_range[0] = 5.0;
		runfit.vbar20[0] = Vbar20[current_cell][current_channel][0];
		runfit.vbar20_range[0] = Vbar20[current_cell][current_channel][0]/5.0;
		runfit.vbar20[1] = Vbar20[current_cell][current_channel][0];
		runfit.vbar20_range[1] = Vbar20[current_cell][current_channel][0]/5.0;
		model_control->fp_eqconst1->updateValue(runfit.eq[0]);
	}
	if (model == 17)	// monomer - N-mer plus incompetent N-mer
	{
		unsigned int k=0;
		for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
		{
			k++;
			if((*sfv_it).FitScanFlag)
			{
				(*sfv_it).amplitude[1] = -1e4;
				(*sfv_it).amplitude_range[1] = 1e-3;
/*
				portion = exp((*sfv_it).amplitude[0])/2;
				(*sfv_it).amplitude[0] = log(portion*.7);
				(*sfv_it).amplitude[1] = log(portion*.3);
//cout << "Scan " << k << ": " << (*sfv_it).amplitude[0] << ",  " << (*sfv_it).amplitude[1] << endl;
				(*sfv_it).amplitude_range[0] = (*sfv_it).amplitude[0] / 5.0;
				(*sfv_it).amplitude_range[1] = (*sfv_it).amplitude_range[0];
*/
			}
		}
		runfit.mw[1] = runfit.stoich1 * runfit.mw[0];	// incompetent N-mer
		runfit.mw_range[0] = runfit.mw[0]/5.0;
		runfit.mw_range[1] = runfit.mw[1]/5.0;
		runfit.eq[0] = -0.1;
//		runfit.eq[0] = -1.0e4;
		runfit.eq_range[0] = 5.0;
		runfit.vbar20[0] = Vbar20[current_cell][current_channel][0];
		runfit.vbar20_range[0] = Vbar20[current_cell][current_channel][0]/5.0;
		runfit.vbar20[1] = Vbar20[current_cell][current_channel][0];
		runfit.vbar20_range[1] = Vbar20[current_cell][current_channel][0]/5.0;
		model_control->fp_eqconst1->updateValue(runfit.eq[0]);
	}
	if (model == 18) //incompetent monomer and n-mer
	{
		for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
		{
			if((*sfv_it).FitScanFlag)
			{
					portion = exp((*sfv_it).amplitude[0])/2;
					(*sfv_it).amplitude[0] = log(portion*.7);
					(*sfv_it).amplitude[1] = log(portion*.3);
					(*sfv_it).amplitude_range[0] = (*sfv_it).amplitude[0] / 5.0;
					(*sfv_it).amplitude_range[1] = (*sfv_it).amplitude_range[0];
/*
				(*sfv_it).amplitude[0] = log(3.0 * exp((*sfv_it).amplitude[0])/4.0);
				(*sfv_it).amplitude[1] = log(exp((*sfv_it).amplitude[0])/20.0);;
				(*sfv_it).amplitude_range[1] = (*sfv_it).amplitude[0];
*/
			}
		}
		runfit.mw[1] = runfit.stoich1 * runfit.mw[0];	// incompetent N-mer
		runfit.mw_range[0] = runfit.mw[0]/5.0;
		runfit.mw_range[1] = runfit.mw[1]/5.0;
		runfit.vbar20[0] = Vbar20[current_cell][current_channel][0];
		runfit.vbar20_range[0] = Vbar20[current_cell][current_channel][0]/5.0;
		runfit.vbar20[1] = Vbar20[current_cell][current_channel][0];
		runfit.vbar20_range[1] = Vbar20[current_cell][current_channel][0]/5.0;
	}
	if (model == 19) //monomer-nmer + contaminant
	{
		for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
		{
			if((*sfv_it).FitScanFlag)
			{
				(*sfv_it).amplitude[1] = -1e4;
				(*sfv_it).amplitude_range[1] = 1e-3;

/*
				portion = exp((*sfv_it).amplitude[0])/2;
				(*sfv_it).amplitude[0] = log(portion*.7);
				(*sfv_it).amplitude[1] = log(portion*.3);
				(*sfv_it).amplitude_range[0] = (*sfv_it).amplitude[0] / 5.0;
				(*sfv_it).amplitude_range[1] = (*sfv_it).amplitude_range[0];
*/
/*
				(*sfv_it).amplitude[0] = log(3.0 * exp((*sfv_it).amplitude[0])/4.0);
				(*sfv_it).amplitude[1] = log(exp((*sfv_it).amplitude[0])/4.0);;
				(*sfv_it).amplitude_range[1] = (*sfv_it).amplitude[0];
*/
			}
		}
		runfit.mw[1] = runfit.mw[0];	// contaminant (should be overridden by user)
		runfit.mw_range[0] = runfit.mw[0]/5.0;
		runfit.mw_range[1] = runfit.mw[1]/5.0;
		runfit.eq[0] = -1.0e4;
		runfit.eq_range[0] = 5.0;
		runfit.vbar20[0] = Vbar20[current_cell][current_channel][0];
		runfit.vbar20_range[0] = Vbar20[current_cell][current_channel][0]/5.0;
		runfit.vbar20[1] = Vbar20[current_cell][current_channel][0];
		runfit.vbar20_range[1] = Vbar20[current_cell][current_channel][0]/5.0;
		model_control->fp_eqconst1->updateValue(runfit.eq[0]);
	}
	model_control->fp_mw->updateValue(runfit.mw[model_control->current_component]);
	model_control->update_scan_info(selected_scan);
	model_control->update_component_labels();
	pb_float_all->setEnabled(true);
	pb_scan_check->setEnabled(true);
}

void US_GlobalEquil::select_model_control()
{
	QString model_id;
	if (model_widget)
	{
		if (model_control->isVisible())
		{
			model_control->raise();
		}
		else
		{
			model_control->show();
		}
		return;
	}
	model_control = new US_EqModelControl(&scanfit_vector, &runfit, &run_information, model, &model_widget, &selected_scan);
	connect (model_control, SIGNAL(ControlWindowClosed()), SLOT(update_model_button()));
//	connect (model_control->cnt_active_scan, SIGNAL(buttonReleased(double)), SLOT(activate_scan(double)));
	connect (model_control->cnt_active_scan, SIGNAL(valueChanged(double)), SLOT(activate_scan(double)));
	connect (model_control->cb_include, SIGNAL(clicked()), SLOT(update_pixmap()));
	pb_model->setEnabled(false);
	pb_unload->setEnabled(false);
	pb_float_all->setEnabled(true);
	pb_initialize->setEnabled(true);
	InfoPanel->lbl_status2->setText(tr("Model is locked - to change model, close Model Control Window first!"));
	model_control->show();
}

void US_GlobalEquil::change_FitFlag(int sc)
{
//cout << "Scan number in change_FitFlag: " << sc << endl;
	select_scan(sc);
	if (model_widget)
	{
		model_control->change_include();
	}
	else
	{
		if (scanfit_vector[sc].FitScanFlag)
		{
			scanfit_vector[sc].FitScanFlag = false;
		}
		else
		{
			scanfit_vector[sc].FitScanFlag = true;
		}
	}
	update_pixmap();
	if (fitting_widget)	// if the fitter is open, close it to force re-initialization when new scans are
	{							// include or excluded
		eqFitter->cancel();
	}

	if (model != -1)
	{
		pb_initialize->setEnabled(true);
		pb_model->setEnabled(false);
		pb_float_all->setEnabled(false);
		pb_scan_check->setEnabled(false);
		pb_fitcontrol->setEnabled(false);
		pb_monte_carlo->setEnabled(false);
		pb_report->setEnabled(false);
	}
}

void US_GlobalEquil::update_pixmap() // everytime we change the FitScanFlag in the model control window we call this
{
	if (scanfit_vector[selected_scan].FitScanFlag)
	{
		if (scanfit_vector[selected_scan].autoExcluded)
		{
			lb_scans->changeItem(QPixmap(redarrow), lb_scans->text(selected_scan), selected_scan);
		}
		else
		{
			lb_scans->changeItem(QPixmap(greenarrow), lb_scans->text(selected_scan), selected_scan);
		}
	}
	else
	{
		lb_scans->changeItem(QPixmap(bluearrow), lb_scans->text(selected_scan), selected_scan);
	}
}

void US_GlobalEquil::select_scan(double i)
{
	lb_scans->setSelected((int) i-1, true);
	lb_scans->setCurrentItem((int) (i-1));
}

float US_GlobalEquil::linesearch()
{
// look for the minimum residual. Residual values are f0, f1, f2, evaluated at x0, x1, x2.
// x0, x1, x2 are multipliers for incremental change of the parameter
// calculate bracket: Assume the minimum is between x0=0 and some stepsize x2 away from x0.
// then find an x1 in the middle between x0 and x2 and calculate f1(x1) and f2(x2), where
// shift to the right
// f is the residual of the function.
	bool check_flag=true;
	double old_f0=0.0, old_f1=0.0, old_f2=0.0;
	float x0 = 100, x1 = 5000, x2 = 10000, h = 0.01, xmin, fmin, tolerance=100, maxerror;
	unsigned int iter = 1;
	errno = 0;
	float f0 = calc_testParameter(x0);
	float f1 = calc_testParameter(x1);
	float f2 = calc_testParameter(x2);
	if(run_inf.exp_type.absorbance)
	{
		maxerror = 1.0e4;
	}
	else
	{
		maxerror = 1.0e12; // allow a larger error for interference data
	}
// make the initial step size smaller if we have very large residuals
	while (errno != 0 || (f0 >= maxerror || f0 < 0 || f1 >= maxerror || f1 < 0 || f2 >= maxerror || f2 < 0))
	{
		x1 /= 2;
		x2 /= 2;
//cout << "Calling from 1. loop\n";
		f1 = calc_testParameter(x1);
		f2 = calc_testParameter(x2);
		if (x1 < FLT_MIN)
		{
//cout << "abandoned with: " << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
//cout << FLT_MIN << ", " << FLT_MAX << ", errno: " << errno << endl;
			errno = 0;
			return (-1); //couldn't do anything for this search direction - fit didn't converge
		}
	}	check_flag=true;
	while(check_flag)
	{
//cout << "mathlib: "  << errno << ", x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << ", d1: " << f1-f0 << ", d2: " << f2-f1 << ", d3: " << f2-f0 << endl;
		if((isnotanumber(f0) && isnotanumber(f1))
		|| (isnotanumber(f1) && isnotanumber(f2))
		|| (isnotanumber(f0) || isnotanumber(f2))) // at least two values are screwed up, exit.
		{
//cout << "error1 " << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
			errno = 0;
			return(-1);
		}
// Check to see if the solution is really converging:

		if ((fabs(f2 - old_f2) < FLT_MIN) && (fabs(f1 - old_f1) < FLT_MIN) && (fabs(f0 - old_f0) < FLT_MIN))	// is the solution horizontal?
		{
			return(0);	// we found the minimum, return alpha=0
		}
		old_f0 = f0;
		old_f1 = f1;
		old_f2 = f2;


		if ((fabs(f2 - f0) < FLT_MIN) && (fabs(f1 - f0) < FLT_MIN) || (f0 > f1 && fabs(f2 - f1) < FLT_MIN))	// is the solution horizontal?
		{
//cout << "error2 "  << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
			return(0);	// we found the minimum, return alpha=0
		}
		if ((fabs(x0) < FLT_MIN) && (fabs(x1) < FLT_MIN) && (fabs(x2) < FLT_MIN))	// is the solution horizontal?
		{
//cout << "error3 "  << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
			return(0);	// we found the minimum, return alpha=0
		}
//		if ((f0 < f1) && (f1 == f2) || (f0 < f1) && (f1 > f2) || (fabs(f1 - f0) < FLT_MIN) && f2 > f1)	// some weird cases can happen near the minimum
		if ((fabs(f0 - f1) < FLT_MIN) && fabs(f1 - f2) < FLT_MIN
		   || (fabs(f0 - f1) < FLT_MIN) && f2 > f1)	// some weird cases can happen near the minimum
		{
//cout << "error4 "  << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
			return(0);	// we found the minimum, return alpha=0
		}
		if (f0 > f1 && f2 > f1) // we have a bracket
		{
//cout << "bracket " << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
			check_flag = false;
			break;
		}
		else if ((f2 > f1 && f1 > f0) || (f1 > f0 && f1 > f2) || (f1 == f2 && f1 > f0)) // shift to the left
		{
//cout << "leftshift " << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
			x2 = x1;
			f2 = f1;
			x1 = (x2 + x0)/2.0;
			f1 = calc_testParameter(x1);
		}
		else if (f0 > f1 && f1 > f2) // shift to the right
		{
//cout << "rightshift " << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
			x0 = x1;
			f0 = f1;
			x1 = x2;
			f1 = f2;
			x2 = x2 + (pow((double) 2, (double) (iter+2))) * h;
			f2 = calc_testParameter(x2);
//cout << "rightshift #2" << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
		}
 		iter++;
//cout << iter << endl;
	} // get a bracket
// search inside the bracket for the minimum and do a 2nd order polynomial fit
	x1 = (x0 + x2)/2.0;
	h = x1 - x0;
//cout << "Calling from 2. loop\n";
	f1 = calc_testParameter(x1);
	while(true)
	{
		if (f0 < f1) // shift left
		{
			x2 = x1;
			f2 = f1;
			x1 = x0;
			f1 = f0;
			x0 = x1 - h;
//cout << "Calling from 3. loop\n";
			f0 = calc_testParameter(x0);
		}
		if (f2 < f1) // shift right
		{
			x0 = x1;
			f0 = f1;
			x1 = x2;
			f1 = f2;
			x2 = x1 + h;
//cout << "Calling from 4. loop\n";
			f2 = calc_testParameter(x2);
		}
		errno = 0;
		if (fabs(f0 - 2 * f1 + f2) <  FLT_MIN)
		{
			return(0); //division by zero above
		}
		xmin = x1 + (h * (f0 - f2))/(2 * (f0 - 2 * f1 + f2));
//cout << "Calling from 5. loop\n";
//cout << "xmin: " << xmin << ", diff: " << (2 * (f0 - 2 * f1 + f2)) << ", h: " << h << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
		fmin = calc_testParameter(xmin);
		if (fmin < f1)
		{
			x1 = xmin;
			f1 = fmin;
		}
		h = h / 2.0;
		if (h < tolerance)
		{
			return(x1);
		}
		x0 = x1 - h;
		x2 = x1 + h;
//cout << "Calling from 6. loop\n";
		f0 = calc_testParameter(x0);
		f2 = calc_testParameter(x2);
	}
}

// calculate the chi-square for the fixed molecular weight estimate for a single component model:
float US_GlobalEquil::calc_testParameter(float mw)
{
	struct XY_data xy;
	unsigned int points;
	float chi_sq = 0.0, x_temp=0, constant, **M, *yraw, *coeff, buoyancy_tb, omega_s;
	for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
	{
		if((*sfv_it).FitScanFlag)
		{
			xy.X.clear();
			xy.Y.clear();
			vector<float>::iterator xval_it = (*sfv_it).x.begin() + (*sfv_it).start_index;
			vector<float>::iterator yval_it = (*sfv_it).y.begin() + (*sfv_it).start_index;
			vector<float>::iterator yval_it_stop = (*sfv_it).y.begin() + (*sfv_it).stop_index;
			points = 0;
			buoyancy_tb = calc_buoyancy_tb(runfit.vbar20[0], (*sfv_it).temperature, (*sfv_it).density);
			omega_s = pow((double) ((*sfv_it).rpm * M_PI / 30.0), (double) 2);
			constant = (buoyancy_tb * omega_s)/(2 * R * (K0 + (*sfv_it).temperature));
			while (yval_it != yval_it_stop + 1)
			{
				if (points == 0)
				{
					x_temp = pow((double) *xval_it, (double) 2);
					xy.X.push_back(0);
				}
				else
				{
					xy.X.push_back(constant * (pow((double) *xval_it, (double) 2) - x_temp));
				}
				xy.Y.push_back((float) *yval_it);
				xval_it++;
				yval_it++;
				points++;
			}
			yraw = new float [xy.X.size()];
			M = new float *[xy.X.size()];

			coeff = new float [2];
			coeff[0] = 0;
			coeff[1] = 0;
			for (unsigned int i=0; i<xy.X.size(); i++)
			{
				M[i] = new float [2];
				M[i][0] = 1.0;
				M[i][1] = exp(mw * xy.X[i]);
				yraw[i] = xy.Y[i];
			}
			generalLeastSquares(M, points, 2, yraw, &coeff);
			for (unsigned int i=0; i<xy.X.size(); i++)
			{
				chi_sq += pow((double) (yraw[i] - (coeff[0] + (coeff[1] * exp(mw * xy.X[i])))), (double) 2);
				delete [] M[i];
			}
			delete [] M;
			delete [] yraw;
			(*sfv_it).baseline = coeff[0];
			(*sfv_it).baseline_range = 0.05;
//QMessageBox::message("warning", "calc_testParameter(), coeff[1] = "+QString::number(coeff[1]));
			if (coeff[1] < fabs(FLT_MIN) || isnotanumber(coeff[1]))
			{
				(*sfv_it).amplitude[0] = FLT_MIN;
			}
			else
			{
				(*sfv_it).amplitude[0] = log(coeff[1]);
			}
			(*sfv_it).amplitude_range[0] = (*sfv_it).amplitude[0] / 5.0;
		}
	}
	return (chi_sq);
}

// General self-association model:
// K(1->n), association = K(1->n), concentration * n / (extinction * pathlength)^(n-1)
// Abs = SUM[   c^i
//            * i
//            * K(1->i), concentration/(extinction * pathlength)
//            * exp [ omega^2 * MW * (1 - vbar*rho) * (r^2 - ra^2)/(2 * R *T)]]
//            + Baseline

