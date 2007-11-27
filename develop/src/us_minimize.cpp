#include "../include/us_minimize.h"
#include <cerrno> //for floating point errors

US_Minimize::US_Minimize(bool *temp_fitting_widget, bool temp_GUI, QWidget *p, const char *name)
: QFrame(p, name)
{
	USglobal = new US_Config();
	suspend_flag = false;
	converged = false;
	aborted = false;
	completed = false;
	plotGroup = 0;
	plotResiduals = false;
	autoconverge = false;
	lambdaStart = 1.0e5;
	lambdaStep = 10.0;
	maxIterations = 1000;
	tolerance = 1.0e-6;
	nlsMethod = 0;
	GUI = temp_GUI;
	constrained = false;
	firstScan = 1;
	points = 0;
	showGuiFit = false; // show fitting process graphically

	if (GUI)
	{
		showGuiFit = true;
		fitting_widget = temp_fitting_widget;
		*fitting_widget = true;	// widget is on screen now, if we push Fit control in calling program, it is only raised, not re-instantiated
		setup_GUI();
	}
}

void US_Minimize::setup_GUI()
{
	int xpos = border;
	int ypos = 4;
	column1 = 140;
	column2 = 93;
	column3 = 93;
	buttonh = 26;
	span = column1 + column2 + column3 + 2 * spacing;

	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("Fitting Control"));

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
	data_plot->setAxisTitle(QwtPlot::xBottom, "");
	data_plot->setAxisTitle(QwtPlot::yLeft, "");
	data_plot->setTitle(" ");
	data_plot->setOutlinePen(white);
	data_plot->setOutlineStyle(Qwt::Cross);
	data_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	data_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setMinimumSize(550,650);

	lbl_header = new QLabel("", this);
	lbl_header->setAlignment(AlignCenter|AlignVCenter);
	lbl_header->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_header->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_header->setGeometry(xpos, ypos, span, 2 * buttonh + spacing);
	lbl_header->setMinimumHeight(buttonh*2);
	lbl_header->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos = border;
	ypos += 2 * buttonh + 2 * spacing;

	lbl_iteration1 = new QLabel(tr("Iteration Number:"),this);
	lbl_iteration1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_iteration1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_iteration1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_iteration1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_iteration2 = new QLabel("0",this);
	lbl_iteration2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_iteration2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_iteration2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_iteration2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_fit = new QPushButton(tr("Fit"), this);
	Q_CHECK_PTR(pb_fit);
	pb_fit->setAutoDefault(false);
	pb_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_fit->setGeometry(xpos, ypos, column3, buttonh);
	connect(pb_fit, SIGNAL(clicked()), SLOT(startFit()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_variance1 = new QLabel(tr("Variance:"),this);
	lbl_variance1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_variance1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_variance1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_variance1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_variance2 = new QLabel("0",this);
	lbl_variance2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_variance2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_variance2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_variance2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_pause = new QPushButton(tr("Pause"), this);
	Q_CHECK_PTR(pb_pause);
	pb_pause->setAutoDefault(false);
	pb_pause->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_pause->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_pause->setGeometry(xpos, ypos, column3, buttonh);
	pb_pause->setEnabled(false);
	connect(pb_pause, SIGNAL(clicked()), SLOT(suspendFit()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_stddev1 = new QLabel(tr("Std. Deviation:"),this);
	lbl_stddev1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_stddev1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_stddev1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_stddev1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_stddev2 = new QLabel("0",this);
	lbl_stddev2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_stddev2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_stddev2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_stddev2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_resume = new QPushButton(tr("Resume"), this);
	Q_CHECK_PTR(pb_resume);
	pb_resume->setAutoDefault(false);
	pb_resume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_resume->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_resume->setGeometry(xpos, ypos, column3, buttonh);
	pb_resume->setEnabled(false);
	connect(pb_resume, SIGNAL(clicked()), SLOT(resumeFit()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_improvement1 = new QLabel(tr("Improvement:"),this);
	lbl_improvement1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_improvement1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_improvement1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_improvement1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_improvement2 = new QLabel("0",this);
	lbl_improvement2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_improvement2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_improvement2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_improvement2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_saveFit = new QPushButton(tr("Save Fit"), this);
	Q_CHECK_PTR(pb_saveFit);
	pb_saveFit->setAutoDefault(false);
	pb_saveFit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_saveFit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_saveFit->setGeometry(xpos, ypos, column3, buttonh);
	pb_saveFit->setEnabled(false);
	connect(pb_saveFit, SIGNAL(clicked()), SLOT(saveFit()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_evaluations1 = new QLabel(tr("Function Evaluations:"),this);
	lbl_evaluations1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_evaluations1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_evaluations1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_evaluations1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_evaluations2 = new QLabel("0",this);
	lbl_evaluations2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_evaluations2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_evaluations2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_evaluations2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_report = new QPushButton(tr("View Report"), this);
	Q_CHECK_PTR(pb_report);
	pb_report->setAutoDefault(false);
	pb_report->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_report->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_report->setGeometry(xpos, ypos, column3, buttonh);
	pb_report->setEnabled(false);
	connect(pb_report, SIGNAL(clicked()), SLOT(view_report()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_decompositions1 = new QLabel(tr("Decompostions:"),this);
	lbl_decompositions1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_decompositions1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_decompositions1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_decompositions1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_decompositions2 = new QLabel("0",this);
	lbl_decompositions2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_decompositions2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_decompositions2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_decompositions2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_residuals = new QPushButton(tr("Residuals"), this);
	Q_CHECK_PTR(pb_residuals);
	pb_residuals->setAutoDefault(false);
	pb_residuals->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_residuals->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_residuals->setGeometry(xpos, ypos, column3, buttonh);
	pb_residuals->setEnabled(false);
	connect(pb_residuals, SIGNAL(clicked()), SLOT(plot_residuals()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_currentLambda1 = new QLabel(tr("Current Lambda:"),this);
	lbl_currentLambda1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_currentLambda1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_currentLambda1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_currentLambda1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_currentLambda2 = new QLabel("1.0e+05",this);
	lbl_currentLambda2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_currentLambda2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_currentLambda2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_currentLambda2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_overlays = new QPushButton(tr("Overlays"), this);
	Q_CHECK_PTR(pb_overlays);
	pb_overlays->setAutoDefault(false);
	pb_overlays->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_overlays->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_overlays->setGeometry(xpos, ypos, column3, buttonh);
	pb_overlays->setEnabled(false);
	connect(pb_overlays, SIGNAL(clicked()), SLOT(plot_overlays()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_parameters1 = new QLabel(tr("# of Parameters:"),this);
	lbl_parameters1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_parameters1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_parameters1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_parameters1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_parameters2 = new QLabel("0",this);
	lbl_parameters2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_parameters2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_parameters2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_parameters2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_print = new QPushButton(tr("Print"), this);
	Q_CHECK_PTR(pb_print);
	pb_print->setAutoDefault(false);
	pb_print->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_print->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_print->setGeometry(xpos, ypos, column3, buttonh);
	pb_print->setEnabled(false);
	connect(pb_print, SIGNAL(clicked()), SLOT(print()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_datasets1 = new QLabel(tr("# of Datasets in Fit:"),this);
	lbl_datasets1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_datasets1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_datasets1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_datasets1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_datasets2 = new QLabel("0",this);
	lbl_datasets2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_datasets2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_datasets2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_datasets2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, column3, buttonh);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_points1 = new QLabel(tr("# of Datapoints in Fit:"),this);
	lbl_points1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_points1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_points1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_points1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_points2 = new QLabel("0",this);
	lbl_points2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_points2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_points2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_points2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_cancel = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_cancel);
	pb_cancel->setAutoDefault(false);
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_cancel->setGeometry(xpos, ypos, column3+1, buttonh);
	connect(pb_cancel, SIGNAL(clicked()), SLOT(cancelFit()));

	xpos = border;
	ypos += buttonh + 2 * spacing;

	lbl_controls1 = new QLabel(tr("Graph Plotting Controls:"), this);
	lbl_controls1->setAlignment(AlignCenter|AlignVCenter);
	lbl_controls1->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_controls1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_controls1->setGeometry(xpos, ypos, span, buttonh);
	lbl_controls1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += buttonh + 2 * spacing;

	bt_plotAll = new QRadioButton(this);
	bt_plotAll->setText(tr("Plot all Data"));
	bt_plotAll->setGeometry(xpos+10, ypos+4, column1-10, 18);
	bt_plotAll->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_plotAll->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(bt_plotAll, SIGNAL(clicked()), SLOT(update_plotAll()));

	xpos += column1 + spacing;

	xpos += column2 + column3 - 23;

	cb_showGuiFit = new QCheckBox(tr("Monitor Fit Graphically"),this);
	Q_CHECK_PTR(cb_showGuiFit);
	cb_showGuiFit->setGeometry(xpos , ypos+5, 14, 14);
	cb_showGuiFit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1));
	cb_showGuiFit->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_showGuiFit->setChecked(showGuiFit);
	connect(cb_showGuiFit, SIGNAL(clicked()), SLOT(change_showGuiFit()));

	xpos = border;
	ypos += buttonh + spacing;

	bt_plotGroup = new QRadioButton(this);
	bt_plotGroup->setText(tr("Plot Groups of 5"));
	bt_plotGroup->setGeometry(xpos+10, ypos+4, column1-10, 18);
	bt_plotGroup->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_plotGroup->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(bt_plotGroup, SIGNAL(clicked()), SLOT(update_plotGroup()));

	xpos += column1 + spacing;

	cnt_scan = new QwtCounter(this);
	Q_CHECK_PTR(cnt_scan);
	cnt_scan->setNumButtons(3);
	cnt_scan->setRange(1, 6, 5);
	cnt_scan->setValue(1);
	cnt_scan->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_scan->setGeometry(xpos, ypos, column2 + column3 + spacing, buttonh);
	cnt_scan->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//	connect(cnt_scan, SIGNAL(buttonReleased(double)), SLOT(updateRange(double)));
	connect(cnt_scan, SIGNAL(valueChanged(double)), SLOT(updateRange(double)));

	xpos = border;
	ypos += buttonh + spacing;

	bt_plotSingle = new QRadioButton(this);
	bt_plotSingle->setText(tr("Plot single Scans"));
	bt_plotSingle->setGeometry(xpos+10, ypos+4, column1-10, 18);
	bt_plotSingle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_plotSingle->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(bt_plotSingle, SIGNAL(clicked()), SLOT(update_plotSingle()));

	xpos += column1 + spacing;

	bt_plotAll->setEnabled(false);
	bt_plotGroup->setEnabled(false);
	bt_plotSingle->setEnabled(false);
	if (plotGroup == 0)
	{
		bt_plotAll->setChecked(true);
		bt_plotGroup->setChecked(false);
		bt_plotSingle->setChecked(false);
	}
	else if (plotGroup == 1)
	{
		bt_plotAll->setChecked(false);
		bt_plotGroup->setChecked(false);
		bt_plotSingle->setChecked(true);
	}
	else if (plotGroup == 5)
	{
		bt_plotAll->setChecked(false);
		bt_plotGroup->setChecked(true);
		bt_plotSingle->setChecked(false);
	}

	xpos = border;
	ypos += buttonh + 2 * spacing;

	lbl_controls2 = new QLabel(tr("NLSQ Fit Tuning Controls"), this);
	lbl_controls2->setAlignment(AlignCenter|AlignVCenter);
	lbl_controls2->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_controls2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_controls2->setGeometry(xpos, ypos, span, buttonh);
	lbl_controls2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += buttonh + 2 * spacing;

	lbl_constrained = new QLabel(tr("Linear Constraints:"),this);
	lbl_constrained->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_constrained->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_constrained->setGeometry(xpos, ypos, column1, buttonh);
	lbl_constrained->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	bt_unconstrained = new QRadioButton(this);
	bt_unconstrained->setText(tr("No"));
	bt_unconstrained->setGeometry(xpos+10, ypos+4, column2-10, 18);
	bt_unconstrained->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_unconstrained->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	if (constrained)
	{
		bt_unconstrained->setChecked(false);
	}
	else
	{
		bt_unconstrained->setChecked(true);
	}
	bt_unconstrained->setEnabled(true);
	connect(bt_unconstrained, SIGNAL(clicked()), SLOT(update_unconstrained()));

	xpos += column2 + spacing;

	bt_constrained = new QRadioButton(this);
	bt_constrained->setText(tr("Yes"));
	bt_constrained->setGeometry(xpos+10, ypos+4, column3-10, 18);
	bt_constrained->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_constrained->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	if (constrained)
	{
		bt_constrained->setChecked(true);
	}
	else
	{
		bt_constrained->setChecked(false);
	}
	bt_constrained->setEnabled(true);
	connect(bt_constrained, SIGNAL(clicked()), SLOT(update_constrained()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_autoconverge = new QLabel(tr("Autoconverge:"),this);
	lbl_autoconverge->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_autoconverge->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_autoconverge->setGeometry(xpos, ypos, column1, buttonh);
	lbl_autoconverge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	bt_manualconverge = new QRadioButton(this);
	bt_manualconverge->setText(tr("No"));
	bt_manualconverge->setGeometry(xpos+10, ypos+4, column2-10, 18);
	bt_manualconverge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_manualconverge->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	if (autoconverge)
	{
		bt_manualconverge->setChecked(false);
	}
	else
	{
		bt_manualconverge->setChecked(true);
	}
	bt_manualconverge->setEnabled(true);
	connect(bt_manualconverge, SIGNAL(clicked()), SLOT(update_manualconverge()));

	xpos += column2 + spacing;

	bt_autoconverge = new QRadioButton(this);
	bt_autoconverge->setText(tr("Yes"));
	bt_autoconverge->setGeometry(xpos+10, ypos+4, column3-10, 18);
	bt_autoconverge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_autoconverge->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	if (autoconverge)
	{
		bt_autoconverge->setChecked(true);
	}
	else
	{
		bt_autoconverge->setChecked(false);
	}
	bt_autoconverge->setEnabled(true);
	connect(bt_autoconverge, SIGNAL(clicked()), SLOT(update_autoconverge()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_nlsMethod = new QLabel(tr("NLS Algorithm:"),this);
	lbl_nlsMethod->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_nlsMethod->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_nlsMethod->setGeometry(xpos, ypos, column1, buttonh);
	lbl_nlsMethod->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	cbb_nlsMethod = new QComboBox(this, "NLS Combobox");
	cbb_nlsMethod->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cbb_nlsMethod->insertItem(tr("Levenberg-Marquardt"), 0);
	cbb_nlsMethod->insertItem(tr("Modified Gauss-Newton"), 1);
	cbb_nlsMethod->insertItem(tr("Hybrid Method"), 2);
	cbb_nlsMethod->insertItem(tr("Quasi-Newton Method"), 3);
	cbb_nlsMethod->insertItem(tr("Generalized Linear LS"), 4);
	cbb_nlsMethod->insertItem(tr("Nonnegative constrained LS"), 5);
	cbb_nlsMethod->setGeometry(xpos, ypos, column2 + column3 + spacing, buttonh);
	cbb_nlsMethod->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(cbb_nlsMethod, SIGNAL(activated(int)), this, SLOT(update_nlsMethod(int)));
	cbb_nlsMethod->setCurrentItem(0);

	xpos = border;
	ypos += buttonh + spacing;

	lbl_lambdaStart = new QLabel(tr("Lambda Start:"),this);
	lbl_lambdaStart->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_lambdaStart->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_lambdaStart->setGeometry(xpos, ypos, column1, buttonh);
	lbl_lambdaStart->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_lambdaStart = new QLineEdit(this, "lambdaStart");
	le_lambdaStart->setGeometry(xpos, ypos, column2, buttonh);
	le_lambdaStart->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_lambdaStart->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_lambdaStart->setText(" 1.0e+05");
	connect(le_lambdaStart, SIGNAL(textChanged(const QString &)),
	SLOT(update_lambdaStart(const QString &)));

	xpos += column2 + spacing;

	pb_plottwo = new QPushButton("", this);
	Q_CHECK_PTR(pb_plottwo);
	pb_plottwo->setAutoDefault(false);
	pb_plottwo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_plottwo->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_plottwo->setGeometry(xpos, ypos, column3+1, buttonh);
	pb_plottwo->setEnabled(false);
	connect(pb_plottwo, SIGNAL(clicked()), SLOT(plot_two()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_lambdaStep = new QLabel(tr("Lambda Step Size:"),this);
	lbl_lambdaStep->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_lambdaStep->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_lambdaStep->setGeometry(xpos, ypos, column1, buttonh);
	lbl_lambdaStep->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_lambdaStep = new QLineEdit(this, "lambdaStep");
	le_lambdaStep->setGeometry(xpos, ypos, column2, buttonh);
	le_lambdaStep->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_lambdaStep->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_lambdaStep->setText(" 10.0");
	connect(le_lambdaStep, SIGNAL(textChanged(const QString &)),
	SLOT(update_lambdaStep(const QString &)));

	xpos += column2 + spacing;

	pb_plotthree = new QPushButton("", this);
	Q_CHECK_PTR(pb_plotthree);
	pb_plotthree->setAutoDefault(false);
	pb_plotthree->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_plotthree->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_plotthree->setGeometry(xpos, ypos, column3+1, buttonh);
	pb_plotthree->setEnabled(false);
	connect(pb_plotthree, SIGNAL(clicked()), SLOT(plot_three()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_maxIterations = new QLabel(tr("Maximum Iterations:"),this);
	lbl_maxIterations->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_maxIterations->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_maxIterations->setGeometry(xpos, ypos, column1, buttonh);
	lbl_maxIterations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_maxIterations = new QLineEdit(this, "maxIterations");
	le_maxIterations->setGeometry(xpos, ypos, column2, buttonh);
	le_maxIterations->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_maxIterations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_maxIterations->setText(" 1000");
	connect(le_maxIterations, SIGNAL(textChanged(const QString &)),
	SLOT(update_maxIterations(const QString &)));

	xpos += column2 + spacing;

	pb_plotfour = new QPushButton("", this);
	Q_CHECK_PTR(pb_plotfour);
	pb_plotfour->setAutoDefault(false);
	pb_plotfour->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_plotfour->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_plotfour->setGeometry(xpos, ypos, column3+1, buttonh);
	pb_plotfour->setEnabled(false);
	connect(pb_plotfour, SIGNAL(clicked()), SLOT(plot_four()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_tolerance = new QLabel(tr("Fit Tolerance:"),this);
	lbl_tolerance->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_tolerance->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_tolerance->setGeometry(xpos, ypos, column1, buttonh);
	lbl_tolerance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_tolerance = new QLineEdit(this, "Fit Tolerance");
	le_tolerance->setGeometry(xpos, ypos, column2, buttonh);
	le_tolerance->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_tolerance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_tolerance->setText(" 1.0e-06");
	connect(le_tolerance, SIGNAL(textChanged(const QString &)),
	SLOT(update_tolerance(const QString &)));

	xpos += column2 + spacing;

	pb_plotfive = new QPushButton("", this);
	Q_CHECK_PTR(pb_plotfive);
	pb_plotfive->setAutoDefault(false);
	pb_plotfive->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_plotfive->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_plotfive->setGeometry(xpos, ypos, column3+1, buttonh);
	pb_plotfive->setEnabled(false);
	connect(pb_plotfive, SIGNAL(clicked()), SLOT(plot_five()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_progress = new QLabel(tr("Fitting Progress:"),this);
	lbl_progress->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_progress->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_progress->setGeometry(xpos, ypos, column1, buttonh);
	lbl_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	pgb_progress = new QProgressBar(this, "iteration progress");
	pgb_progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	pgb_progress->setGeometry(xpos, ypos, column2 + column3 + spacing, buttonh);

	ypos += buttonh;
	global_Xpos += 30;
	global_Ypos += 30;

	lbl_status1 = new QLabel(tr("Status:"),this);
	lbl_status1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_status1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_status1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_status2 = new QLabel(tr("Waiting for Input - Please click on \"Fit\" to start the fitting process..."),this);
	lbl_status2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_status2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_status2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_status3 = new QLabel(tr("Information:"),this);
	lbl_status3->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_status3->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_status3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_status4 = new QLabel("",this);
	lbl_status4->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_status4->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_status4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

//	setGeometry(global_Xpos, global_Ypos, span + 10, ypos + 4);
//	setMinimumSize((int) (2.7 * span), ypos + 4);
	setup_GUI2();
}

US_Minimize::~US_Minimize()
{
	*fitting_widget = false;
}

void US_Minimize::setup_GUI2()
{
	int j=0;
	int rows = 20, columns = 3, spacing = 2;

	QGridLayout * background = new QGridLayout(this,2,2,spacing);
	background->setMargin(spacing);
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->addMultiCellWidget(lbl_header,j,j,0,2);
	j++;
	subGrid1->addWidget(lbl_iteration1,j,0);
	subGrid1->addWidget(lbl_iteration2,j,1);
	subGrid1->addWidget(pb_fit,j,2);
	j++;
	subGrid1->addWidget(lbl_variance1,j,0);
	subGrid1->addWidget(lbl_variance2,j,1);
	subGrid1->addWidget(pb_pause,j,2);
	j++;
	subGrid1->addWidget(lbl_stddev1,j,0);
	subGrid1->addWidget(lbl_stddev2,j,1);
	subGrid1->addWidget(pb_resume,j,2);
	j++;
	subGrid1->addWidget(lbl_improvement1,j,0);
	subGrid1->addWidget(lbl_improvement2,j,1);
	subGrid1->addWidget(pb_saveFit,j,2);
	j++;
	subGrid1->addWidget(lbl_evaluations1,j,0);
	subGrid1->addWidget(lbl_evaluations2,j,1);
	subGrid1->addWidget(pb_report,j,2);
	j++;
	subGrid1->addWidget(lbl_decompositions1,j,0);
	subGrid1->addWidget(lbl_decompositions2,j,1);
	subGrid1->addWidget(pb_residuals,j,2);
	j++;
	subGrid1->addWidget(lbl_currentLambda1,j,0);
	subGrid1->addWidget(lbl_currentLambda2,j,1);
	subGrid1->addWidget(pb_overlays,j,2);
	j++;
	subGrid1->addWidget(lbl_parameters1,j,0);
	subGrid1->addWidget(lbl_parameters2,j,1);
	subGrid1->addWidget(pb_print,j,2);
	j++;
	subGrid1->addWidget(lbl_datasets1,j,0);
	subGrid1->addWidget(lbl_datasets2,j,1);
	subGrid1->addWidget(pb_help,j,2);
	j++;
	subGrid1->addWidget(lbl_points1,j,0);
	subGrid1->addWidget(lbl_points2,j,1);
	subGrid1->addWidget(pb_cancel,j,2);
	j++;
	subGrid1->addMultiCellWidget(lbl_controls1,j,j,0,2);
	j++;
	subGrid1->addWidget(bt_plotAll,j,0);
	subGrid1->addMultiCellWidget(cb_showGuiFit,j,j,1,2);
	j++;
	subGrid1->addWidget(bt_plotGroup,j,0);
	subGrid1->addMultiCellWidget(cnt_scan,j,j,1,2);
	j++;
	subGrid1->addWidget(bt_plotSingle,j,0);
	j++;
	subGrid1->addMultiCellWidget(lbl_controls2,j,j,0,2);
	j++;
	subGrid1->addWidget(lbl_constrained,j,0);
	subGrid1->addWidget(bt_unconstrained,j,1);
	subGrid1->addWidget(bt_constrained,j,2);
	j++;
	subGrid1->addWidget(lbl_autoconverge,j,0);
	subGrid1->addWidget(bt_manualconverge,j,1);
	subGrid1->addWidget(bt_autoconverge,j,2);
	j++;
	subGrid1->addWidget(lbl_nlsMethod,j,0);
	subGrid1->addMultiCellWidget(cbb_nlsMethod,j,j,1,2);
	j++;
	subGrid1->addWidget(lbl_lambdaStart,j,0);
	subGrid1->addWidget(le_lambdaStart,j,1);
	subGrid1->addWidget(pb_plottwo,j,2);
	j++;
	subGrid1->addWidget(lbl_lambdaStep,j,0);
	subGrid1->addWidget(le_lambdaStep,j,1);
	subGrid1->addWidget(pb_plotthree,j,2);
	j++;
	subGrid1->addWidget(lbl_maxIterations,j,0);
	subGrid1->addWidget(le_maxIterations,j,1);
	subGrid1->addWidget(pb_plotfour,j,2);
	j++;
	subGrid1->addWidget(lbl_tolerance,j,0);
	subGrid1->addWidget(le_tolerance,j,1);
	subGrid1->addWidget(pb_plotfive,j,2);
	j++;
	subGrid1->addWidget(lbl_progress,j,0);
	subGrid1->addMultiCellWidget(pgb_progress,j,j,1,2);

	QGridLayout * subGrid2 = new QGridLayout(2, 4, 2);
	for (int i=0; i<2; i++)
	{
		subGrid2->setRowSpacing(i, 26);
	}
	subGrid2->addWidget(lbl_status1,0,0);
	subGrid2->addMultiCellWidget(lbl_status2,0,0,1,3);
	subGrid2->addWidget(lbl_status3,1,0);
	subGrid2->addMultiCellWidget(lbl_status4,1,1,1,3);


	background->addMultiCellLayout(subGrid1,0,1,0,0);
	background->addWidget(data_plot,0,1);
	background->addLayout(subGrid2,1,1);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(1,550);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;

	this->setGeometry(global_Xpos, global_Ypos, r.width()+550, r.height());

}

void US_Minimize::closeEvent(QCloseEvent *e)
{
	emit fittingWidgetClosed();
	*fitting_widget = false;
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}
/*
void US_Minimize::resizeEvent(QResizeEvent *e)
{
	if (!GUI)
	{
		return;
	}
	int column1 = 120;
	int dialogw = span + 2 * border;
	int plot_width = e->size().width() - dialogw - border;
	data_plot->setGeometry(	dialogw, border, plot_width, e->size().height() - (2 * (buttonh + spacing) + border));
	int ypos = e->size().height() - (2 * buttonh + spacing + border);
	int width = plot_width - column1 - spacing;
	lbl_status1->setGeometry(dialogw, ypos, column1, buttonh);
	lbl_status2->setGeometry(dialogw + column1 + spacing, ypos, width, buttonh);
	ypos += buttonh + spacing;
	lbl_status3->setGeometry(dialogw, ypos, column1, buttonh);
	lbl_status4->setGeometry(dialogw + column1 + spacing, ypos, width, buttonh);

}
*/
void US_Minimize::cancelFit()
{
	if (aborted || converged)
	{
		cancel();
	}
	else
	{
		suspend_flag = true;
//cout << "Aborting at 0...\n";
		aborted = true;
		if (GUI)
		{
			pgb_progress->reset();
			pb_cancel->setText(tr("Close"));
			pb_fit->setEnabled(true);
			lbl_iteration2->setText("0");
			lbl_variance2->setText("0");
			lbl_stddev2->setText("0");
			lbl_improvement2->setText("0");
			lbl_evaluations2->setText("0");
			lbl_status2->setText(tr("Aborted"));
			lbl_status3->setText("");
		}
	}
}

int US_Minimize::Fit()
{
//cout << "converged: " << converged << ", completed: " << completed << ", aborted: " << aborted << endl;
	float lambda = lambdaStart; // re-initialize Lambda
	float *search, alpha, *gamma, *delta;
	int step_counter = 0; // count how many times we try to shorten the step size in the hybrid method
	int lambda_loop = 0; //counts how many times we enlarge lambda
	// if we try more than 3 times, chances are we found the local minimum.

	QString str;
	if (converged || completed || aborted)
	{
//cout << "calling cleanup from Fit()\n";
		cleanup();
	}
//cout << "starting fit\n";
	first_plot = true;		// reset first plot each time we do a new fit
	completed = false;
	aborted = false;
	converged = false;
	if (GUI)
	{
		suspend_flag = false;
		pb_cancel->setText(tr("Abort"));
		pb_fit->setEnabled(false);
		pb_pause->setEnabled(true);
		qApp->processEvents();
	}
	function_evaluations = 0;
	decompositions = 0;
	iteration = 0;
	variance = 0;
	if (GUI)
	{
//		update_fitDialog();
		lbl_status2->setText(tr("Results from loaded fit are shown"));
	}
	if (!fit_init())	// initialize the fitting process
	{
		if (GUI)
		{
			QMessageBox::message(tr("Please Note:"), tr("No scans have been selected\n"
															 "for fitting or all scans have\n"
															 "been excluded.\n\n"
															 "Please review the Scan Diagnostics\n"
															 "and check the scans for fit before\n"
															 "proceeding."));
			lbl_status2->setText(tr("Fit aborted..."));
			lbl_status3->setText("");
			pb_cancel->setText(tr("Close"));
			pb_fit->setEnabled(true);
			pb_pause->setEnabled(false);
		}
//cout << "Aborting at 1...\n";
		aborted = true;
		return(-2);
	}
	if (nlsMethod == 3)
	{
	// set up Hessian matrix, initialized to identity matrix:
		for (unsigned int i=0; i<parameters; i++)
		{
			for(unsigned int j=0; j<parameters; j++)
			{
				if (i == j)
				{
					information_matrix[i][j] = 1.0;
				}
				else
				{
					information_matrix[i][j] = 0.0;
				}
			}
		}
		search = new float [parameters];
		gamma = new float [parameters];
		delta = new float [parameters];
	}
	if (GUI)
	{
		str.sprintf(" %d", parameters);
		lbl_parameters2->setText(str);
		str.sprintf(" %ld", datasets);
		lbl_datasets2->setText(str);
		str.sprintf(" %ld", points);
		lbl_points2->setText(str);
	}
	bool dostep = false;	// only needed for hybrid method
	float step = 1.0;		// only needed for hybrid method
	unsigned int count;
	float target; // keeps track of progress
	float old_residuals = 0, new_residuals = 0;
	if(calc_model(guess) < 0)
	{
		if (GUI)
		{
			QMessageBox::message(tr("UltraScan Error:"),
			tr("The residuals from the initial guess\n"
  				"are too large.\n\n"
  				"Please manually adjust the fitting\n"
  				"parameters and retry the fit.\n\n"
				"Return code: -4"));
			pgb_progress->setProgress(totalSteps);
			pb_cancel->setText(tr("Close"));
			pb_fit->setEnabled(true);
			lbl_status2->setText(tr("The fitting process didn't converge..."));
			pb_saveFit->setEnabled(true);
			pb_report->setEnabled(true);
			pb_residuals->setEnabled(true);
			pb_overlays->setEnabled(true);
		}
		cout << "The residuals from the initial guess are too large.\n"
			  << "Please manually adjust the fitting parameters and retry the fit.\n"
			  << "Return code: -4 (using nls method" << nlsMethod << ").\n\nFailing Parameters:\n";
		for (unsigned int i=0; i<parameters; i++)
		{
			cout << guess[i] << endl;
		}
		cleanup();
		return (-4);
	}
	if (GUI)
	{
		bt_plotAll->setEnabled(true);
		bt_plotGroup->setEnabled(true);
		bt_plotSingle->setEnabled(true);
	}
	new_residuals = calc_residuals();
//cout << "returning from calc_residuals in CP1 with " << new_residuals << endl;
	if (new_residuals < 0)
	{
		cout << "exited with -4, residuals: " << new_residuals << endl;
		return(-4);
	}
//cout << "Residuals: " << new_residuals << endl;
	target = new_residuals/points;
	totalSteps = 0;

	if(target <= 0)
	{
//		if(!try_adjustment())	// call virtual function to try to adjust
//		{								// unreasonable parameter values in calling routine
			if (GUI)
			{
				QMessageBox::message(tr("UltraScan Error:"),
				tr("The residuals from the initial guess\n"
	  				"are too large.\n\n"
	  				"Please manually adjust the fitting\n"
	  				"parameters and retry the fit."));
				pgb_progress->setProgress(totalSteps);
				pb_cancel->setText(tr("Close"));
				pb_fit->setEnabled(true);
				lbl_status2->setText(tr("The fitting process converged..."));
				pb_saveFit->setEnabled(true);
				pb_report->setEnabled(true);
				pb_residuals->setEnabled(true);
				pb_overlays->setEnabled(true);
			}
			cleanup();
			return(-5);
//		}
	}
	while(target > tolerance)
	{
		target /= 2.0;
		totalSteps++;
	}
	totalSteps -= 3;
	target = new_residuals/(points * 2);
	if (GUI)
	{
		pgb_progress->reset();
		pgb_progress->setTotalSteps(totalSteps);
	}
	count = 0;

//cout << "iter: " << iteration << ", maxiter: " << maxIterations << ", new_resid: " << new_residuals/points << ", tolerance: " << tolerance << endl;
	while((iteration < maxIterations) && (new_residuals/points > tolerance))
	{
		if(GUI && showGuiFit)
		{
			if(plotResiduals)
			{
				plot_residuals();
			}
			else
			{
				plot_overlays();
			}
			qApp->processEvents();
		}
		variance = new_residuals/points;
		iteration++;
		if (GUI)
		{
			str.sprintf("%3.5e", variance);
			lbl_variance2->setText(str);
			str.sprintf("%3.5e", pow((double) variance, (double) 0.5));
			lbl_stddev2->setText(str);
			str.sprintf("%3.5e", (old_residuals - new_residuals)/points);
			lbl_improvement2->setText(str);
			str.sprintf("%d", iteration);
			lbl_iteration2->setText(str);
		}
		old_residuals = new_residuals;
		if (old_residuals/points < target)
		{
			count++;
			if (GUI)
			{
				pgb_progress->setProgress(count);
			}
			target /= 2.0;
		}
		if (GUI)
		{
			lbl_status2->setText(tr("Calculating the Information Matrix..."));
			qApp->processEvents();
		}
//cout << "current iteration: " << iteration << endl;
		if ((nlsMethod < 3) || (nlsMethod == 3 && iteration == 1))
		{
			calc_jacobian();
//print_matrix(jacobian, points, parameters);
			calc_A_transpose_A(&jacobian, &information_matrix, points, parameters, USglobal->config_list.numThreads);
		}
//print_matrix(information_matrix, parameters, parameters);
		if (nlsMethod == 3)	// Quasi-Newton
		{
			if (iteration == 1)
			{
				double **inf;
				inf =  new  double * [parameters];
				for (unsigned int i=0; i<parameters; i++)
				{
					inf[i] = new double [parameters];
				}
				calc_B();
				for (unsigned int i=0; i<parameters; i++)
				{
					for (unsigned int j=0; j<parameters; j++)
					{
						inf[i][j] = information_matrix[i][j];
					}
				}
				Cholesky_Invert(inf, LL_transpose, parameters);
				for (unsigned int i=0; i<parameters; i++)
				{
					delete [] inf[i];
				}
				delete [] inf;

				for (unsigned int i=0; i<parameters; i++)
				{
					for (unsigned int j=0; j<parameters; j++)
					{
						information_matrix[i][j] = LL_transpose[i][j];
					}
				}
			}
			float test=0;
			for (unsigned int i=0; i<parameters; i++)
			{
				test += pow((double) B[i], (double) 2);
			}
			test = pow((double) test, (double) 0.5);
			if (test < tolerance)
			{
				variance = new_residuals/points;
//cout << "Calling endfit 1\n";
				endFit();
				delete [] delta;
				delete [] gamma;
				delete [] search;
				return(0);
			}
/*
print_matrix(information_matrix, parameters, parameters);
for (unsigned int i=0; i<parameters; i++)
{
	cout << "Search before: " << search[i] << endl;
}
*/
			mmv(&search, &B, &information_matrix, parameters, parameters);
/*
for (unsigned int i=0; i<parameters; i++)
{
	cout << "Search after: " << search[i] << endl;
}
*/
			for (unsigned int i=0; i<parameters; i++)
			{
				gamma[i] = (float) B[i];	// B = J'y_delta = -gradient; in updateQN we need
			}										// gamma=gradient(k+1) - gradient(k)
													// in updateQN we will calculate gamma = gamma - B
													// where gamma at this point is the old gradient, and -B is the new
													// gradient.
//cout << "calling linesearch from lsp1\n";
			alpha = linesearch(&search, new_residuals);
			if (alpha == 0.0)
			{
				variance = new_residuals/points;
//cout << "alpha is zero, and variance = " << variance << "\n";
				delete [] delta;
				delete [] gamma;
				delete [] search;
//cout << "Calling endfit 2, points: " << points << "\n";
				endFit();
				return(0);
			}
			if (alpha < 0)
			{
				if (autoconverge)
				{
					delete [] delta;
					delete [] gamma;
					delete [] search;
					cleanup();
					return(-10);
				}
				else
				{
					QMessageBox::message(tr("UltraScan Error:"), tr("The fit failed to converge!\n\n"
					"Please try different initial guesses."));
					delete [] delta;
					delete [] gamma;
					delete [] search;
					cleanup();
					return(-10);
				}
			}
			for (unsigned int i=0; i<parameters; i++)
			{
				guess[i] = guess[i] + search[i] * alpha;
			}
			calc_model(guess);// updates solution, needed to calculate y_delta
			calc_jacobian();  // gives us jacobian matrix
			old_residuals = new_residuals;
			new_residuals = calc_residuals(); // gives us new y_delta
//cout << "returning from calc_residuals in CP2 with " << new_residuals << endl;
			calc_B();
			for (unsigned int i=0; i<parameters; i++)
			{
				gamma[i] = gamma[i] - (float) B[i];
				delta[i] = alpha * search[i];
//cout << "Paramter " << i << ": gamma:" << gamma[i] << ", delta: " << delta[i] << ", alpha: " << alpha << ", search: " << search[i] << ", B: " << B[i] << endl;
			}
			updateQN(&gamma, &delta);
		}

		if (nlsMethod == 0 || nlsMethod == 2)
		{
			// Add lambda to the information matrix to make diagonal large to make columns
			// linearly independent:

			for (unsigned int i=0; i<parameters; i++)
			{
				information_matrix[i][i] += lambda;
//				for (unsigned int j=0; j<parameters; j++)
//				{
//					cout << information_matrix[i][j] << "\t";
//				}
//				cout << endl;
			}
		}

		while (new_residuals >= old_residuals && nlsMethod != 3)
		{

//	Problem: J * R = y_delta, where R = (a[i] - guess[i]) and we want to find a[i], so solve for R:
// (1)	J'J * R = J' * y_delta

// Solution - try to find R:
// (2)	(J'J)-1 * (J'J) * R = (J'J)-1 * (J' * y_delta)
// (3)	R = (J'J)-1 * (J`*y_delta)

// Alternative Solution: This can be solved using first forward and then backward substitution,
// and Cholesky Decomposition:
// (1)	J'J * R = J'*y_delta

// (2) 	Substitute: B = J'*y_delta

			if (GUI)
			{
				lbl_status2->setText(tr("Calculating the Gradient..."));
				qApp->processEvents();
			}
			calc_B();

// save information matrix in LL_transpose in case chi-2 is larger, in which case we would
// need to reset Lambda

			cout.precision(3);
			cout.setf(ios::scientific|ios::showpos);

//cout << "\nLL_transpose (for " << parameters << " parameters): ";
			for (unsigned int i=0; i<parameters; i++)
			{
//cout << "\n";
				for (unsigned int j=0; j<parameters; j++)
				{
//					cout << i << ", " << j << ": " << information_matrix[i][j] << " ";
					LL_transpose[i][j] = information_matrix[i][j];
				}
			}
//cout << "\n";

// (3)	J'J -> Cholesky Decomposition -> LL'

			decompositions++;
			if (GUI)
			{
				lbl_status2->setText(tr("Processing Cholesky Decomposition..."));
				str.sprintf("%d", decompositions);
				lbl_decompositions2->setText(str);
				qApp->processEvents();
			}

			if(!Cholesky_Decomposition(LL_transpose, parameters))
			{
				if (GUI)
				{
					if (nlsMethod == 0)
					{
						if (showGuiFit)
						{
							QMessageBox::message(tr("Attention:"), tr("The Cholesky Decomposition of the\n"
																		  "Information matrix failed due to a\n"
																		  "singularity in the matrix.\n\n"
																		  "You may achieve convergence by\n"
																		  "re-fitting the current data with\n"
																		  "new initial parameter estimates."));
						}
					}
					else
					{
						if (showGuiFit)
						{
							QMessageBox::message(tr("Attention:"), tr("The Cholesky Decomposition of the\n"
																		  "Information matrix failed due to a\n"
																		  "singularity in the matrix.\n\n"
																		  "You may achieve convergence by\n"
																		  "re-fitting the current data with\n"
																		  "the Levenberg-Marquardt method or\n"
																		  "by using different initial parameter\n"
																		  "estimates."));
						}
					}
					lbl_status2->setText(tr("Fit aborted..."));
					lbl_status3->setText("");
					pb_cancel->setText(tr("Close"));
					pb_resume->setEnabled(false);
					pb_pause->setEnabled(false);
					pb_fit->setEnabled(true);
				}
//cout << "Aborting at 2...\n";
				aborted = true;
				cleanup();
				return (-6);
			}

// Now the information matrix actually contains the decomposed information matrix LL'
// (4)	L(L'*R) = B
// (5) 	Substitute:  Z = L'*R
// (6) 	Solve for Z using forward substitution: L*Z = B
// (7) 	Solve for R using backward substitution: L'*R = Z

			if (GUI)
			{
				lbl_status2->setText(tr("Solving Cholesky System..."));
				qApp->processEvents();
			}
			Cholesky_SolveSystem(LL_transpose, B, parameters);

// Now B is changed to R (the difference between the current parameter estimate with the "true" parameter)

			switch (nlsMethod)
			{
				case 0:
				{
					for (unsigned int i=0; i<parameters; i++)
					{
						test_guess[i] = guess[i] + (float) B[i];
					}
					break;
				}
				case 1:
				{
					// Add the new R (parameter correction) to the old guess

					float *t, st;
					t = new float [parameters];
					for (unsigned int i=0; i<parameters; i++)
					{
						t[i] = (float) B[i];
					}
//cout << "calling linesearch from lsp2\n";
					st = linesearch(&t, new_residuals);
					delete [] t;
					if (st == 0)
					{
						variance = new_residuals/points;
//cout << "Calling endfit 3\n";
						endFit();
						return(0);
					}
					if (st < 0)
					{
						QMessageBox::message(tr("UltraScan Error:"), tr("The fit failed to converge!\n\n"
						"Please try different initial guesses."));
						return(-11);
					}
//					cout << "Alpha: " << st << endl;
					for (unsigned int i=0; i<parameters; i++)
					{
						test_guess[i] = guess[i] + st * (float) B[i];
//						test_guess[i] = guess[i] + lambda * (float) B[i];
					}
					break;
				}
				case 2:
				{
					float *t, st;
					t = new float [parameters];
					for (unsigned int i=0; i<parameters; i++)
					{
						t[i] = (float) B[i];
					}
//cout << "calling linesearch from lsp3\n";
					st = linesearch(&t, new_residuals);
					delete [] t;
					if (st == 0)
					{
						variance = new_residuals/points;
//cout << "Calling endfit 4\n";
						endFit();
						return(0);
					}
					if (st < 0)
					{
						QMessageBox::message(tr("UltraScan Error:"),tr( "The fit failed to converge!\n\n"
						"Please try different initial guesses."));
						return(-11);
					}
					for (unsigned int i=0; i<parameters; i++)
					{
						test_guess[i] = guess[i] + st * (float) B[i];
					}
					break;
				}
			}
			if(calc_model(test_guess) < 0)
			{
				cleanup();
				return(-7);
			}

			new_residuals = calc_residuals();
//cout << "returning from calc_residuals in CP3 with " << new_residuals << endl;
			if (new_residuals < old_residuals)
			{
				if (GUI)
				{
					qApp->processEvents();
				}
				switch (nlsMethod)
				{
					case 0:
					{
						lambda = lambda/lambdaStep;
						break;
					}
					case 1:
					{
						lambda_loop ++;
						lambda *= pow((double) lambdaStep, (double) lambda_loop);
						if (lambda > 1.0e10)
						{
							lambda = 1.0e6;
							variance = new_residuals/points;
//cout << "Calling endfit 5\n";
							endFit();
							return(0);
						}
						break;
					}
					case 2:
					{
						lambda = lambda/lambdaStep;
						if (lambda < 1.0)
						{
							if (!dostep)
							{
								step = 0.01;
							}
							lambda = 0.0;
							step *= 2.0;
							step_counter = 0; //reset step counter to zero, because we lengthened the step.
							dostep = true;
						}
						break;
					}
				}
				for (unsigned int i=0; i<parameters; i++)
				{
					guess[i] = test_guess[i];
				}
			}
			else if (new_residuals == old_residuals)
			{
				variance = new_residuals/points;
//cout << "Calling endfit 6\n";
				endFit();
				return(0);
			}
			else
			{
				if(GUI)
				{
					str.sprintf("%3.5e", (old_residuals - new_residuals)/points);
					lbl_improvement2->setText(str);
					qApp->processEvents();
				}
				switch (nlsMethod)
				{
					case 0:
					{
						for (unsigned int i=0; i<parameters; i++)
						{
							information_matrix[i][i] -= lambda;
						}
						lambda_loop ++;
						lambda *= pow((double) lambdaStep, (double) lambda_loop);
						if (lambda > 1.0e10)
						{
							lambda = 1.0e6;
							variance = new_residuals/points;
//cout << "Calling endfit 7\n";
							endFit();
							return(0);
						}
						for (unsigned int i=0; i<parameters; i++)
						{
							information_matrix[i][i] += lambda;
						}
						break;
					}
					case 1:
					{
						lambda /= lambdaStep;
						if (lambda < tolerance)
						{
//cout << "Calling endfit 8\n";
							endFit();
							return(0);
						}
						break;
					}
					case 2:
					{
						for (unsigned int i=0; i<parameters; i++)
						{
							information_matrix[i][i] -= lambda;
						}
						lambda *= lambdaStep;
						if (lambda > 1.0e10)
						{
							lambda = 1.0e6;
							if (GUI)
							{
								qApp->processEvents();
							}
							variance = new_residuals/points;
//cout << "Calling endfit 9\n";
							endFit();
							return(0);
						}
						if (dostep)
						{
							step /= 2.0;
							step_counter++;
							if (step_counter > 3 && step < tolerance)
							{
//cout << "Calling endfit 10\n";
								endFit();
								return(0);
							}
						}
						for (unsigned int i=0; i<parameters; i++)
						{
							information_matrix[i][i] += lambda;
						}
						break;
					}
				}
				if(calc_model(guess) < 0)
				{
					cleanup();
					return (-8);
				}
				new_residuals = calc_residuals();
//cout << "returning from calc_residuals in CP4 with " << new_residuals << endl;
			}
			if (GUI)
			{
				str.sprintf("%3.5e", lambda);
				lbl_currentLambda2->setText(str);
			}
		}
	}
	return(0);
}

// if a fit was loaded, update all the dialogs and plot windows:
void US_Minimize::update_fitDialog()
{
	if (!GUI)
	{
		return;
	}
	QString str;
	str.sprintf("%1.4e", variance);
	lbl_variance2->setText(str);
	str.sprintf("%d", function_evaluations);
	lbl_evaluations2->setText(str);
	str.sprintf("%1.4e", pow((double) variance, (double) 0.5));
	lbl_stddev2->setText(str);
	str.sprintf("%1.4e", 0.0);
	lbl_improvement2->setText(str);
	lbl_status2->setText(tr("Results from loaded fit are shown"));
	str.sprintf(" %d", parameters);
	lbl_parameters2->setText(str);
	str.sprintf(" %ld", datasets);
	lbl_datasets2->setText(str);
	str.sprintf(" %ld", points);
	lbl_points2->setText(str);
	str.sprintf(tr("%ld Runs (%2.2f"), runs, runs_percent);
	str += " %)";
	lbl_status4->setText(str);
}


void US_Minimize::update_nlsMethod(int item)
{
	nlsMethod = item;
	switch (item)
	{
		case 0:
		{
			lambdaStart = 1.0e6;
			lambdaStep = 10.0;
			if (GUI)
			{
				lbl_status4->setText(tr("Levenberg-Marquardt Method selected..."));
				le_lambdaStart->setText(" 1.0e+06");
				le_lambdaStep->setText(" 10.0");
			}
			break;
		}
		case 1:
		{
			lambdaStart = 1.0e-6;
			lambdaStep = 2.0;
			if (GUI)
			{
				lbl_status4->setText(tr("Modified Gauss Newton Method selected..."));
				le_lambdaStart->setText(" 1.0e-6");
				le_lambdaStep->setText(" 2.0");
			}
			break;
		}
		case 2:
		{
			lambdaStart = 1.0e6;
			lambdaStep = 10.0;
			if (GUI)
			{
				lbl_status4->setText(tr("Hybrid Method selected..."));
				le_lambdaStart->setText(" 1.0e6");
				le_lambdaStep->setText(" 10.0");
			}
			break;
		}
		case 3:
		{
			if (GUI)
			{
				lbl_status4->setText(tr("Quasi-Newton Method selected..."));
			}
			break;
		}
		case 4:
		{
			lambdaStart = 0.0;
			lambdaStep = 0.0;
			if (GUI)
			{
				lbl_status4->setText(tr("Generalized Linear Least Squares Method selected..."));
				le_lambdaStart->setText(" 0.0");
				le_lambdaStep->setText(" 0.0");
			}
			break;
		}
	}
}

// calculate the B matrix:
int US_Minimize::calc_B()
{
	for (unsigned int i=0; i<parameters; i++)
	{
		B[i] = 0.0;
		for (unsigned int j=0; j<points; j++)
		{
			B[i] += jacobian[j][i] * y_delta[j];
//			cout << "jac: " << jacobian[j][i] << ", y_delta: " << y_delta[j] << endl;
		}
//		cout << "B["<<i<<"]: " << B[i]<<endl;
	}
	return (-1);
}

// calculate the residuals:
float US_Minimize::calc_residuals()
{
	errno = 0;
	float residual=0.0;
	for (unsigned int i=0; i<points; i++)
	{
		y_delta[i] = y_raw[i] - y_guess[i];
//cout << "y_raw: " << y_raw[i] << ", y_guess: " << y_guess[i] << endl;
		residual += pow((double) y_delta[i], (double) 2.0);
		if (errno != 0)
		{
			cout << "Floating point exception in the residuals calculation!\n";
			if (GUI)
			{
				QMessageBox::message(tr("UltraScan Error:"),
				tr("The residuals from the initial guess\n"
				"are too large.\n\n"
				"Please manually adjust the fitting\n"
				"parameters and retry the fit.\n\n"
				"Return code: -4"));
				pgb_progress->setProgress(totalSteps);
				pb_cancel->setText(tr("Close"));
				pb_fit->setEnabled(true);
				lbl_status2->setText(tr("The fitting process converged..."));
				pb_saveFit->setEnabled(true);
				pb_report->setEnabled(true);
				pb_residuals->setEnabled(true);
				pb_overlays->setEnabled(true);
			}
      	cleanup();
//cout << "about to return with -1\n";
			return (-1);
		}
	}
	if(residual > FLT_MAX)
	{
		residual = -1;
	}
	return residual;
}

// set lambda:
void US_Minimize::update_lambdaStart(const QString &str)
{
	lambdaStart = str.toFloat();
}

// set the lambda step size:
void US_Minimize::update_lambdaStep(const QString &str)
{
	lambdaStep = str.toFloat();
}

// set the maximum number of iterations per fit:
void US_Minimize::update_maxIterations(const QString &str)
{
	maxIterations = str.toInt();
}

// set the tolerance of the fit:
void US_Minimize::update_tolerance(const QString &str)
{
	tolerance = str.toFloat();
}

// update the graphical fitting progress monitor setting:
void US_Minimize::change_showGuiFit()
{
	if (!GUI)
	{
		return;
	}
	if (showGuiFit)
	{
		showGuiFit = false;
	}
	else
	{
		showGuiFit = true;
	}
	cb_showGuiFit->setChecked(showGuiFit);
}

void US_Minimize::update_unconstrained()
{
	if (!GUI)
	{
		return;
	}
	constrained = false;
	bt_constrained->setChecked(false);
	bt_unconstrained->setChecked(true);
}

void US_Minimize::update_constrained()
{
	if (!GUI)
	{
		return;
	}
	constrained = true;
	bt_constrained->setChecked(true);
	bt_unconstrained->setChecked(false);
}

void US_Minimize::update_manualconverge()
{
	if (!GUI)
	{
		return;
	}
	autoconverge = false;
	bt_autoconverge->setChecked(false);
	bt_manualconverge->setChecked(true);
}

void US_Minimize::update_autoconverge()
{
	if (!GUI)
	{
		return;
	}
	autoconverge = true;
	bt_autoconverge->setChecked(true);
	bt_manualconverge->setChecked(false);
}

// set plotting to plotting all scans at once:
void US_Minimize::update_plotAll()
{
	if (!GUI)
	{
		return;
	}
	plotGroup = 0;
	bt_plotAll->setChecked(true);
	bt_plotGroup->setChecked(false);
	bt_plotSingle->setChecked(false);
	if (plotResiduals)
	{
		plot_residuals();
	}
	else
	{
		plot_overlays();
	}
}

// set plotting to plotting in groups of five:
void US_Minimize::update_plotGroup()
{
	if (!GUI)
	{
		return;
	}
	plotGroup = 5;
	cnt_scan->setRange(1, 6, 5);
	bt_plotAll->setChecked(false);
	bt_plotGroup->setChecked(true);
	bt_plotSingle->setChecked(false);
	if (plotResiduals)
	{
		plot_residuals();
	}
	else
	{
		plot_overlays();
	}
}

// set plotting to plotting in single scans:
void US_Minimize::update_plotSingle()
{
	if (!GUI)
	{
		return;
	}
	plotGroup = 1;
	cnt_scan->setRange(1, 1, 1);
	bt_plotAll->setChecked(false);
	bt_plotGroup->setChecked(false);
	bt_plotSingle->setChecked(true);
	if (plotResiduals)
	{
		plot_residuals();
	}
	else
	{
		plot_overlays();
	}
}

// sets which scan is the first one to be plotted if group plotting is selected:
void US_Minimize::updateRange(double scan)
{
	firstScan = (int) scan;
}

// close the fitting control widget:
void US_Minimize::cancel()
{
	*fitting_widget = false;
	close();
}

// close the fitting control widget:
void US_Minimize::setGUI(bool val)
{
	GUI = val;
}

// suspend the fit:
void US_Minimize::suspendFit()
{
	if (!GUI)
	{
		return;
	}
	suspend_flag = true;
	pb_saveFit->setEnabled(true);
	pb_resume->setEnabled(true);
	pb_residuals->setEnabled(true);
	pb_overlays->setEnabled(true);
	pb_pause->setEnabled(false);
	emit newParameters();
	emit fitSuspended();
}

// resume the fit:
void US_Minimize::resumeFit()
{
	if (!GUI)
	{
		return;
	}
	suspend_flag = false;
	pb_resume->setEnabled(false);
	pb_pause->setEnabled(true);
	emit fitResumed();
}

// save a copy of the plot window to a printer or a postscript file:
void US_Minimize::print()
{
	if (!GUI)
	{
		return;
	}
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

void US_Minimize::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/nls.html");
}

//----------------------
// Virtual Functions:
//----------------------

// called when fit has converged, can be overridden by derived class
void US_Minimize::endFit()
{
	emit currentStatus(tr("Converged"));
	emit hasConverged();
	converged = true;
	if (GUI)
	{
		pgb_progress->setProgress(totalSteps);
		pb_cancel->setText(tr("Close"));
		pb_fit->setEnabled(true);
		lbl_status2->setText(tr("The fitting process converged..."));
		pb_saveFit->setEnabled(true);
		pb_report->setEnabled(true);
		pb_residuals->setEnabled(true);
		pb_overlays->setEnabled(true);
	}
//cout << "executing endFit()...\n";
}

// this function should be used to invoke the fit, it can be reimplemented to handle special fitting cases
void US_Minimize::startFit()
{
	return_value = Fit();
}

// this function is needed to initialize all variables for the fit:
bool US_Minimize::fit_init()
{
	emit fitStarted();
	return(false);
}

// plot overlays
void US_Minimize::plot_overlays()
{
	plotResiduals = false;
}

// plot residuals
void US_Minimize::plot_residuals()
{
	plotResiduals = true;
}

// plot the second plot
void US_Minimize::plot_two()
{
}

// plot the third plot
void US_Minimize::plot_three()
{
}

// plot the fourth plot
void US_Minimize::plot_four()
{
}

// plot the 5th plot
void US_Minimize::plot_five()
{
}

// present the data report (different file for each derived class):
void US_Minimize::view_report()
{
}

// calculate the simulation functions:
int US_Minimize::calc_model(double *)
{
	return(-1);
}

// calculated the Jacobian matrix:
int US_Minimize::calc_jacobian()
{
	return (-1);
}

// save the converged fit to disk
void US_Minimize::saveFit()
{
}

// Garbage collection, reimplement in derived class if necessary:
void US_Minimize::cleanup()
{
}

bool US_Minimize::try_adjustment()
{
	return (false);
}

// write out the data report:
void US_Minimize::write_data()
{
}

float US_Minimize::linesearch(float **search, float f0)
{
	bool check_flag=true;
	double test_val, old_f0=0.0, old_f1=0.0, old_f2=0.0;
//cout << "before: " << errno << endl;
	errno = 0; //clear old error state
	for (unsigned int i=0; i<parameters; i++)
	{
		test_val = (*search)[i];
//cout << "Error info: " << errno << ", parameter value: " << test_val << endl;
	}
// look for the minimum residual. Residual values are f0, f1, f2, evaluated at x0, x1, x2.
// x0, x1, x2 are multipliers for incremental change of the parameter vector 'search'
// `search` contains the vector of parameters we need to optimize
// calculate bracket: Assume the minimum is between x0=0 and some stepsize x2 away from x0.
// then find an x1 in the middle between x0 and x2 and calculate f1(x1) and f2(x2), where
// shift to the right
// f is the residual of the function.
	float x0 = 0.0, x1 = 0.5, x2 = 1.0, h = 0.01,xmin, fmin;
	unsigned int iter = 0;
//cout << "calling calc_testParameter from ctp1...\n";
	float f1 = calc_testParameter(search, x1);
if (f1 < 0) return(0.0);
//cout << "-1: " << errno << endl;

//cout << "calling calc_testParameter from ctp2...\n";
	float f2 = calc_testParameter(search, x2);
if (f2 < 0) return(0.0);
//cout << "-2: " << errno << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;

	while (errno != 0 || (f0 >= 10000 || f0 < 0 || f1 >= 10000 || f1 < 0 || f2 >= 10000 || f2 < 0)) //make the initial step size smaller if we have infinite residuals
	{
		x1 /= 10;
		x2 /= 10;
//cout << "Calling from 1. loop\n";
//cout << "calling calc_testParameter from Ctp3...\n";
		f1 = calc_testParameter(search, x1);
if (f1 < 0) return(0.0);
//cout << "calling calc_testParameter from ctp4...\n";
		f2 = calc_testParameter(search, x2);
if (f2 < 0) return(0.0);
		if (x1 < FLT_MIN)
		{
//cout << "abandoned with: " << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
//cout << FLT_MIN << ", " << FLT_MAX << ", errno: " << errno << endl;
			errno = 0;
			return (-1); //couldn't do anything for this search direction - fit didn't converge
		}
	}
	check_flag=true;
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
//cout << "calling calc_testParameter from ctp5...\n";
			f1 = calc_testParameter(search, x1);
if (f1 < 0) return(0.0);
		}
		else if (f0 > f1 && f1 > f2) // shift to the right
		{
//cout << "rightshift " << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
			x0 = x1;
			f0 = f1;
			x1 = x2;
			f1 = f2;
			x2 = x2 + (pow((double) 2, (double) (iter+2))) * h;
//cout << "calling calc_testParameter from ctp6...\n";
			f2 = calc_testParameter(search, x2);
if (f2 < 0) return(0.0);
//cout << "rightshift #2" << "x0: " << x0 << ", x1: " << x1 << ", x2: " << x2 << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
		}
 		iter++;
//cout << iter << endl;
	} // get a bracket
// search inside the bracket for the minimum and do a 2nd order polynomial fit
	x1 = (x0 + x2)/2.0;
	h = x1 - x0;
//cout << "Calling from 2. loop\n";
//cout << "calling calc_testParameter from ctp7...\n";
	f1 = calc_testParameter(search, x1);
if (f1 < 0) return(0.0);
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
//cout << "calling calc_testParameter from ctp8...\n";
			f0 = calc_testParameter(search, x0);
if (f0 < 0) return(0.0);
		}
		if (f2 < f1) // shift right
		{
			x0 = x1;
			f0 = f1;
			x1 = x2;
			f1 = f2;
			x2 = x1 + h;
//cout << "Calling from 4. loop\n";
//cout << "calling calc_testParameter from ctp9...\n";
			f2 = calc_testParameter(search, x2);
			if (f2 < 0) return(0.0);
		}
		errno = 0;
		if (fabs(f0 - 2 * f1 + f2) <  FLT_MIN)
		{
			return(0); //division by zero above
		}
		xmin = x1 + (h * (f0 - f2))/(2 * (f0 - 2 * f1 + f2));
//cout << "Calling from 5. loop\n";
//cout << "xmin: " << xmin << ", diff: " << (2 * (f0 - 2 * f1 + f2)) << ", h: " << h << ", f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << endl;
//cout << "calling calc_testParameter from ctp10...\n";
		fmin = calc_testParameter(search, xmin);
		if (fmin < 0) return(0.0);
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
//cout << "calling calc_testParameter from ctp11...\n";
		f0 = calc_testParameter(search, x0);
		if (f0 < 0) return(0.0);
//cout << "calling calc_testParameter from ctp12...\n";
		f2 = calc_testParameter(search, x2);
		if (f2 < 0) return(0.0);
	}
}

float US_Minimize::calc_testParameter(float **search, float step)
{
	float res;
	for (unsigned int i=0; i<parameters; i++)
	{
		test_guess[i] = guess[i] + step * (*search)[i];
//cout << "Step: " << step << ", Test-guess(" << i << "): " << test_guess[i] << ", guess: " << guess[i] << ", search: " << (*search)[i] << endl;
	}
/*
	QString s="", format;
	for (unsigned int i=0; i<parameters; i++)
   {
		s += format.sprintf("Parameter[%d]: %e, Search[%d]: %e\n", i, test_guess[i], i, (*search)[i]);
	}
	QMessageBox::message("Debug:", "The following values are passed to calc_model:\n" + s + "");
*/
	if(calc_model(test_guess) < 0)
	{
		cout << "Attention: there was an error in the model calculation, resetting to original parameters...\n";
		for (unsigned int i=0; i<parameters; i++)
		{
			test_guess[i] = guess[i];
//cout << "Step: " << step << ", Test-guess(" << i << "): " << test_guess[i] << ", guess: " << guess[i] << ", search: " << (*search)[i] << endl;
			calc_model(guess); // reset model to
		}
	}
if (errno > 0) return(-1.0);
//cout << "calling calc_residuals from calc_testParameter...\n";
	res = calc_residuals();
//cout << "res: " << res << endl;
	return (res);
}

void US_Minimize::updateQN(float **gamma, float **delta)
{
	unsigned int i, j;
/*
//cout << "Parameters in updateQN: " << parameters << endl;
	for (i=0; i<parameters; i++)
	{
		//cout << "Gamma[" << i << "]: " << (*gamma)[i] << ", Delta[" << i << "]: " << (*delta)[i] << endl;
	}
*/
	float *hgamma, lambda, deltagamma, *v, *temp, **vvtranspose, **ddtranspose, **hgammatranspose;
	hgamma = new float [parameters];
	v = new float [parameters];
	temp = new float [parameters];
	vvtranspose = new float *[parameters];
	hgammatranspose = new float *[parameters];
	ddtranspose = new float *[parameters];
	for (i=0; i<parameters; i++)
	{
		vvtranspose[i] = new float [parameters];
		hgammatranspose[i] = new float [parameters];
		ddtranspose[i] = new float [parameters];
	}

	mmv(&hgamma, gamma, &information_matrix, parameters, parameters);
	lambda = dotproduct(gamma, &hgamma, parameters);
	deltagamma = dotproduct(delta, gamma, parameters);
//cout << "Deltagamma: " << deltagamma << endl;
	for (i=0; i<parameters; i++)
	{
//cout << "Gamma: " << (*gamma)[i] << endl;
		v[i] = (*delta)[i]/deltagamma - hgamma[i]/lambda;
	}
	vvt(&vvtranspose, &v, &v, parameters);
	vvt(&ddtranspose, delta, delta, parameters);
	vvt(&hgammatranspose, &hgamma, &hgamma, parameters);
	for (i=0; i<parameters; i++)
	{
		for (j=0; j<parameters; j++)
		{
			information_matrix[i][j] = information_matrix[i][j] - hgammatranspose[i][j]/lambda
			+ ddtranspose[i][j]/deltagamma + lambda * vvtranspose[i][j];
		}
	}
	mmv(&temp, gamma, &information_matrix, parameters, parameters);
/*
	for (i=0; i<parameters; i++)
	{
		cout << "difference: " << temp[i] - (*delta)[i] << endl;
	}
*/
	for (i=0; i<parameters; i++)
	{
		delete [] vvtranspose[i];
		delete [] hgammatranspose[i];
		delete [] ddtranspose[i];
	}
	delete [] hgamma;
	delete [] v;
	delete [] temp;
	delete [] vvtranspose;
	delete [] hgammatranspose;
	delete [] ddtranspose;

}
