#include "../include/us_dud.h"

US_Dud::US_Dud(const unsigned int temp_model, bool *temp_fitting_widget, QWidget *p, const char *name) : QFrame(p, name)
{
	configuration = new US_Config(0);
	suspend_flag = false;
	converged = false;
	aborted = false;
	completed = false;
	plotGroup = false;
	plotResiduals = true;
	firstScan = 1;
	allpoints = 0;

	model = temp_model;
	fitting_widget = temp_fitting_widget;
	*fitting_widget = true;		// widget is on screen now, if we push Fit control in calling program, it is only raised, not re-instantiated
	int xpos = border;
	int ypos = 4;
	int buttonh = 26;
	column1 = 140;
	column2 = 93;
	column3 = 93;
	span = column1 + column2 + column3 + 2 * spacing;

	setPalette(QPalette(global_colors.cg_frame, global_colors.cg_frame, global_colors.cg_frame));
	setCaption(tr("Fitting Control"));
	
	data_plot = new QwtPlot(this);
	Q_CHECK_PTR(data_plot);
	data_plot->setPalette(QPalette(global_colors.cg_plot, global_colors.cg_plot, global_colors.cg_plot));
	data_plot->enableGridXMin();
	data_plot->enableGridYMin();
	data_plot->setGridMajPen(QPen(global_colors.major_ticks, 0, DotLine));
	data_plot->setGridMinPen(QPen(global_colors.minor_ticks, 0, DotLine));
	data_plot->setPlotBackground(global_colors.plot);
	data_plot->enableOutline(true);
	data_plot->setAxisTitle(QwtPlot::xBottom, "");
	data_plot->setAxisTitle(QwtPlot::yLeft, "");
	data_plot->setTitle(" ");
	data_plot->setOutlinePen(white);
	data_plot->setOutlineStyle(Qwt::Cross);

	lbl_header = new QLabel("", this);
	lbl_header->setAlignment(AlignCenter|AlignVCenter);
	lbl_header->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_header->setPalette(QPalette(global_colors.cg_frame, global_colors.cg_frame, global_colors.cg_frame));
	lbl_header->setGeometry(xpos, ypos, span, 2 * buttonh + spacing);
	lbl_header->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize, QFont::Bold));

	xpos = border;
	ypos += 2 * buttonh + 2 * spacing;

	lbl_iteration1 = new QLabel(tr("Iteration Number:"),this);
	lbl_iteration1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_iteration1->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_iteration1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_iteration1->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	lbl_iteration2 = new QLabel("0",this);
	lbl_iteration2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_iteration2->setPalette(QPalette(global_colors.cg_edit, global_colors.cg_edit, global_colors.cg_edit));
	lbl_iteration2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_iteration2->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1));
	
	xpos += column2 + spacing;
	
	pb_fit = new QPushButton(tr("Fit"), this);
	Q_CHECK_PTR(pb_fit);
	pb_fit->setAutoDefault(false);
	pb_fit->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize));
	pb_fit->setPalette( QPalette(global_colors.cg_pushb, global_colors.cg_pushb_disabled, global_colors.cg_pushb_active));
	pb_fit->setGeometry(xpos, ypos, column3, buttonh);
	connect(pb_fit, SIGNAL(clicked()), SLOT(dud()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_variance1 = new QLabel(tr("Variance:"),this);
	lbl_variance1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_variance1->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_variance1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_variance1->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	lbl_variance2 = new QLabel("0",this);
	lbl_variance2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_variance2->setPalette(QPalette(global_colors.cg_edit, global_colors.cg_edit, global_colors.cg_edit));
	lbl_variance2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_variance2->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_pause = new QPushButton(tr("Pause"), this);
	Q_CHECK_PTR(pb_pause);
	pb_pause->setAutoDefault(false);
	pb_pause->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize));
	pb_pause->setPalette( QPalette(global_colors.cg_pushb, global_colors.cg_pushb_disabled, global_colors.cg_pushb_active));
	pb_pause->setGeometry(xpos, ypos, column3, buttonh);
	pb_pause->setEnabled(false);
	connect(pb_pause, SIGNAL(clicked()), SLOT(suspendFit()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_stddev1 = new QLabel(tr("Std. Deviation:"),this);
	lbl_stddev1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_stddev1->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_stddev1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_stddev1->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	lbl_stddev2 = new QLabel("0",this);
	lbl_stddev2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_stddev2->setPalette(QPalette(global_colors.cg_edit, global_colors.cg_edit, global_colors.cg_edit));
	lbl_stddev2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_stddev2->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_resume = new QPushButton(tr("Resume"), this);
	Q_CHECK_PTR(pb_resume);
	pb_resume->setAutoDefault(false);
	pb_resume->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize));
	pb_resume->setPalette( QPalette(global_colors.cg_pushb, global_colors.cg_pushb_disabled, global_colors.cg_pushb_active));
	pb_resume->setGeometry(xpos, ypos, column3+2, buttonh);
	pb_resume->setEnabled(false);
	connect(pb_resume, SIGNAL(clicked()), SLOT(resumeFit()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_improvement1 = new QLabel(tr("Improvement:"),this);
	lbl_improvement1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_improvement1->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_improvement1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_improvement1->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	lbl_improvement2 = new QLabel("0",this);
	lbl_improvement2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_improvement2->setPalette(QPalette(global_colors.cg_edit, global_colors.cg_edit, global_colors.cg_edit));
	lbl_improvement2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_improvement2->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_saveFit = new QPushButton(tr("Save Fit"), this);
	Q_CHECK_PTR(pb_saveFit);
	pb_saveFit->setAutoDefault(false);
	pb_saveFit->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize));
	pb_saveFit->setPalette( QPalette(global_colors.cg_pushb, global_colors.cg_pushb_disabled, global_colors.cg_pushb_active));
	pb_saveFit->setGeometry(xpos, ypos, column3, buttonh);
	pb_saveFit->setEnabled(false);
	connect(pb_saveFit, SIGNAL(clicked()), SLOT(saveFit()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_best1 = new QLabel(tr("Best sofar:"),this);
	lbl_best1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_best1->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_best1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_best1->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_best2 = new QLabel("0",this);
	lbl_best2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_best2->setPalette(QPalette(global_colors.cg_edit, global_colors.cg_edit, global_colors.cg_edit));
	lbl_best2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_best2->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_report = new QPushButton(tr("View Report"), this);
	Q_CHECK_PTR(pb_report);
	pb_report->setAutoDefault(false);
	pb_report->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize));
	pb_report->setPalette( QPalette(global_colors.cg_pushb, global_colors.cg_pushb_disabled, global_colors.cg_pushb_active));
	pb_report->setGeometry(xpos, ypos, column3, buttonh);
	pb_report->setEnabled(false);
	connect(pb_report, SIGNAL(clicked()), SLOT(view_report()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_evaluations1 = new QLabel(tr("Function Evaluations:"),this);
	lbl_evaluations1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_evaluations1->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_evaluations1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_evaluations1->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	lbl_evaluations2 = new QLabel("0",this);
	lbl_evaluations2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_evaluations2->setPalette(QPalette(global_colors.cg_edit, global_colors.cg_edit, global_colors.cg_edit));
	lbl_evaluations2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_evaluations2->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_parameters1 = new QLabel(tr("# of Parameters:"),this);
	lbl_parameters1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_parameters1->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_parameters1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_parameters1->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	lbl_parameters2 = new QLabel("0",this);
	lbl_parameters2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_parameters2->setPalette(QPalette(global_colors.cg_edit, global_colors.cg_edit, global_colors.cg_edit));
	lbl_parameters2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_parameters2->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_datasets1 = new QLabel(tr("# of Datasets in Fit:"),this);
	lbl_datasets1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_datasets1->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_datasets1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_datasets1->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	lbl_datasets2 = new QLabel("0",this);
	lbl_datasets2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_datasets2->setPalette(QPalette(global_colors.cg_edit, global_colors.cg_edit, global_colors.cg_edit));
	lbl_datasets2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_datasets2->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_residuals = new QPushButton(tr("Residuals"), this);
	Q_CHECK_PTR(pb_residuals);
	pb_residuals->setAutoDefault(false);
	pb_residuals->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize));
	pb_residuals->setPalette( QPalette(global_colors.cg_pushb, global_colors.cg_pushb_disabled, global_colors.cg_pushb_active));
	pb_residuals->setGeometry(xpos, ypos, column3, buttonh);
	pb_residuals->setEnabled(false);
	connect(pb_residuals, SIGNAL(clicked()), SLOT(residuals()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_points1 = new QLabel(tr("# of Datapoints in Fit:"),this);
	lbl_points1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_points1->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_points1->setGeometry(xpos, ypos, column1, buttonh);
	lbl_points1->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	lbl_points2 = new QLabel("0",this);
	lbl_points2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_points2->setPalette(QPalette(global_colors.cg_edit, global_colors.cg_edit, global_colors.cg_edit));
	lbl_points2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_points2->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1));

	xpos += column2 + spacing;

	pb_overlays = new QPushButton(tr("Overlays"), this);
	Q_CHECK_PTR(pb_overlays);
	pb_overlays->setAutoDefault(false);
	pb_overlays->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize));
	pb_overlays->setPalette( QPalette(global_colors.cg_pushb, global_colors.cg_pushb_disabled, global_colors.cg_pushb_active));
	pb_overlays->setGeometry(xpos, ypos, column3, buttonh);
	pb_overlays->setEnabled(false);
	connect(pb_overlays, SIGNAL(clicked()), SLOT(overlays()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_progress = new QLabel(tr("Current Iteration:"),this);
	lbl_progress->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_progress->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_progress->setGeometry(xpos, ypos, column1, buttonh);
	lbl_progress->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	pgb_progress = new QProgressBar(this, "iteration progress");
	pgb_progress->setPalette( QPalette(global_colors.cg_normal, global_colors.cg_normal, global_colors.cg_normal));
	pgb_progress->setGeometry(xpos, ypos, column2 + column3 + spacing, buttonh);
	
	xpos = border;
	ypos += buttonh + spacing;

	lbl_status1 = new QLabel(tr("Status:"),this);
	lbl_status1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_status1->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_status1->setGeometry(xpos, ypos, (int) (column1/2), buttonh);
	lbl_status1->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));
	
	xpos += (int) (column1/2) + spacing;

	lbl_status2 = new QLabel(tr("Waiting for Input..."),this);
	lbl_status2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_status2->setPalette(QPalette(global_colors.cg_edit, global_colors.cg_edit, global_colors.cg_edit));
	lbl_status2->setGeometry(xpos, ypos, (int) (column1/2) + column2 + column3 + spacing, buttonh);
	lbl_status2->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_status3 = new QLabel("",this);
	lbl_status3->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_status3->setPalette(QPalette(global_colors.cg_edit, global_colors.cg_edit, global_colors.cg_edit));
	lbl_status3->setGeometry(xpos, ypos, column1 + column2 + column3 + 2 * spacing, buttonh);
	lbl_status3->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_blank = new QLabel("",this);
	lbl_blank->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_blank->setPalette( QPalette(global_colors.cg_normal, global_colors.cg_normal, global_colors.cg_normal));
	lbl_blank->setGeometry(xpos, ypos, column1, buttonh);

	bt_plotAll = new QRadioButton(this);
	bt_plotAll->setText(tr("Plot all Data"));
	bt_plotAll->setGeometry(xpos+10, ypos+4, column1-10, 18);
	bt_plotAll->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1)); //, QFont::Bold));
	bt_plotAll->setPalette( QPalette(global_colors.cg_normal, global_colors.cg_normal, global_colors.cg_normal));
	if (plotGroup)
	{
		bt_plotAll->setChecked(false);
	}
	else
	{
		bt_plotAll->setChecked(true);
	}
	connect(bt_plotAll, SIGNAL(clicked()), SLOT(update_plotAll()));

	xpos += column1 + spacing;

	lbl_status4 = new QLabel("",this);
	lbl_status4->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_status4->setPalette(QPalette(global_colors.cg_edit, global_colors.cg_edit, global_colors.cg_edit));
	lbl_status4->setGeometry(xpos, ypos, column2 + column3 + spacing, buttonh);
	lbl_status4->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_blank = new QLabel("",this);
	lbl_blank->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_blank->setPalette( QPalette(global_colors.cg_normal, global_colors.cg_normal, global_colors.cg_normal));
	lbl_blank->setGeometry(xpos, ypos, column1, buttonh);

	bt_plotGroup = new QRadioButton(this);
	bt_plotGroup->setText(tr("Plot Groups of 5"));
	bt_plotGroup->setGeometry(xpos+10, ypos+4, column1-10, 18);
	bt_plotGroup->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1)); //, QFont::Bold));
	bt_plotGroup->setPalette( QPalette(global_colors.cg_normal, global_colors.cg_normal, global_colors.cg_normal));
	if (plotGroup)
	{
		bt_plotGroup->setChecked(true);
	}
	else
	{
		bt_plotGroup->setChecked(false);
	}
	connect(bt_plotGroup, SIGNAL(clicked()), SLOT(update_plotGroup()));

	xpos += column1 + spacing;

	cnt_scan = new QwtCounter(this);
	Q_CHECK_PTR(cnt_scan);
	cnt_scan->setNumButtons(3);
	cnt_scan->setRange(1, 6, 5);
	cnt_scan->setValue(1);
	cnt_scan->setPalette( QPalette(global_colors.cg_normal, global_colors.cg_normal, global_colors.cg_normal));
	cnt_scan->setGeometry(xpos, ypos, column2 + column3, buttonh);
	connect(cnt_scan, SIGNAL(buttonReleased(double)), SLOT(updateRange(double)));

	xpos = border;
	ypos += buttonh + 2 * spacing;

	lbl_controls = new QLabel(tr("NLSQ Fit Tuning Controls"), this);
	lbl_controls->setAlignment(AlignCenter|AlignVCenter);
	lbl_controls->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_controls->setPalette(QPalette(global_colors.cg_frame, global_colors.cg_frame, global_colors.cg_frame));
	lbl_controls->setGeometry(xpos, ypos, span, buttonh);
	lbl_controls->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize, QFont::Bold));

	ypos += buttonh + 2 * spacing;

	lbl_alphaCutoff = new QLabel(tr("Alpha Cutoff:"),this);
	lbl_alphaCutoff->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_alphaCutoff->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_alphaCutoff->setGeometry(xpos, ypos, column1, buttonh);
	lbl_alphaCutoff->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	le_alphaCutoff = new QLineEdit(this, "alphaCutoff");
	le_alphaCutoff->setGeometry(xpos, ypos, column2, buttonh);
	le_alphaCutoff->setPalette(QPalette(global_colors.cg_normal, global_colors.cg_normal, global_colors.cg_normal));
	le_alphaCutoff->setText(" 1.0e-5");
	alphaCutoff = 1.0e-5;
	connect(le_alphaCutoff, SIGNAL(textChanged(const QString &)), 
				SLOT(update_alphaCutoff(const QString &)));	

	xpos += column2 + spacing;

	pb_print = new QPushButton(tr("Print"), this);
	Q_CHECK_PTR(pb_print);
	pb_print->setAutoDefault(false);
	pb_print->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize));
	pb_print->setPalette( QPalette(global_colors.cg_pushb, global_colors.cg_pushb_disabled, global_colors.cg_pushb_active));
	pb_print->setGeometry(xpos, ypos, column3, buttonh);
	pb_print->setEnabled(false);
	connect(pb_print, SIGNAL(clicked()), SLOT(print()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_maxLoops = new QLabel(tr("Maximum Loop #:"),this);
	lbl_maxLoops->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_maxLoops->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_maxLoops->setGeometry(xpos, ypos, column1, buttonh);
	lbl_maxLoops->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	le_maxLoops = new QLineEdit(this, "maxLoops");
	le_maxLoops->setGeometry(xpos, ypos, column2, buttonh);
	le_maxLoops->setPalette(QPalette(global_colors.cg_normal, global_colors.cg_normal, global_colors.cg_normal));
	le_maxLoops->setText(" 6");
	maxLoops = 6;
	connect(le_maxLoops, SIGNAL(textChanged(const QString &)), 
				SLOT(update_maxLoops(const QString &)));	

	xpos += column2 + spacing;

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize));
	pb_help->setPalette( QPalette(global_colors.cg_pushb, global_colors.cg_pushb_disabled, global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, column3, buttonh);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_dInit = new QLabel(tr("Loop Initializer:"),this);
	lbl_dInit->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dInit->setPalette(QPalette(global_colors.cg_label, global_colors.cg_label, global_colors.cg_label));
	lbl_dInit->setGeometry(xpos, ypos, column1, buttonh);
	lbl_dInit->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	le_dInit = new QLineEdit(this, "dInit");
	le_dInit->setGeometry(xpos, ypos, column2, buttonh);
	le_dInit->setPalette(QPalette(global_colors.cg_normal, global_colors.cg_normal, global_colors.cg_normal));
	le_dInit->setText(" 0.10");
	dInit = 0.1;
	connect(le_dInit, SIGNAL(textChanged(const QString &)), 
				SLOT(update_dInit(const QString &)));	

	xpos += column2 + spacing;

	pb_cancel = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_cancel);
	pb_cancel->setAutoDefault(false);
	pb_cancel->setFont(QFont( configuration->config_list.fontFamily, configuration->config_list.fontSize));
	pb_cancel->setPalette( QPalette(global_colors.cg_pushb, global_colors.cg_pushb_disabled, global_colors.cg_pushb_active));
	pb_cancel->setGeometry(xpos, ypos, column3+1, buttonh);
	connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel_fit()));

	ypos += buttonh;
	global_Xpos += 30;
	global_Ypos += 30;

	setGeometry(global_Xpos, global_Ypos, span + 10, ypos + 4); 
	setMinimumSize((int) (2.7 * span), ypos + 4);
}


US_Dud::~US_Dud()
{
}

void US_Dud::closeEvent(QCloseEvent *e)
{
	qApp->processEvents();		// take care of unfinished business before shutting down
/*
	if (scan_loaded)
	{
		cleanup_loaded_scan();
	}
	if (scan_copied)
	{
		cleanup_copied_scan();
	}
	if (run_loaded)
	{
		cleanup_loaded_run();
	}
*/
	emit fittingWidgetClosed();
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

void US_Dud::resizeEvent(QResizeEvent *e)
{
	int dialogw = span + 2 * border;
	int plot_width = e->size().width() - dialogw - border;
	data_plot->setGeometry(	dialogw, border, plot_width, e->size().height());
}

void US_Dud::cancel_fit()
{
	if (aborted || converged)
	{
		cancel();
	}
	else
	{	
		suspend_flag = true;
		pgb_progress->reset();
		pb_cancel->setText("Close");
		lbl_iteration2->setText("0");
		lbl_variance2->setText("0");
		lbl_best2->setText("0");
		lbl_stddev2->setText("0");
		lbl_improvement2->setText("0");
		lbl_evaluations2->setText("0");
		lbl_status2->setText(tr("Aborted"));
		lbl_status3->setText("");
		lbl_status4->setText("");
		aborted = true;
	}
}

void US_Dud::dud()
{
	QString str;
	if (converged || completed || aborted)
	{
		cleanup_dud();
	}
	first_plot = true;		// reset first plot each time we do a new fit
	completed = false;
	aborted = false;
	converged = false;
	pb_cancel->setText(tr("Abort"));
	pb_fit->setEnabled(false);
	pb_pause->setEnabled(true);
	qApp->processEvents();
//	QMessageBox::message("test", "testing....");
	func_eval = 0;
	float improvement=0, d;
	unsigned int i, j, loopcount = 0;
	init_simulation = false;
	if (!dud_init())	// initialize the basis vectors
	{
		QMessageBox::message(tr("Please Note:"), tr("No scans have been selected\n"
														 "for fitting or all scans have\n"
														 "been excluded.\n\n"
														 "Please review the Scan Diagnostics\n"
														 "and check the scans for fit before\n"
														 "proceeding."));
		aborted = true;
		lbl_status2->setText(tr("Fit aborted..."));
		lbl_status3->setText("");
		lbl_status4->setText("");
		pb_cancel->setText(tr("Close"));
		pb_fit->setEnabled(true);
		pb_pause->setEnabled(false);
		return;
	}
	str.sprintf(" %d", parameters);
	lbl_parameters2->setText(str);
	str.sprintf(" %d", datasets);
	lbl_datasets2->setText(str);
	str.sprintf(" %ld", allpoints);
	lbl_points2->setText(str);
	F_init();		// initialize the corresponding function vectors
	if (aborted)
	{
		suspend_flag = false;
		return;
	}
	order_variance();			// sort the variances - theta causing largest goes into first element, theta causing best variance goes into parameters+1
//QMessageBox::message("Stop 1", "Click on OK to continue...");
	while (!aborted)
	{
		for (i=0; i<parameters; i++)
		{
			calc_deltas(i);
		}

		if (!calc_alpha())
		{
			QMessageBox::message(tr("Attention:"), tr("The Cholesky Decomposition of the\n"
														  "Hessian matrix failed due to a\n"
														  "singularity in the matrix.\n\n"
														  "You can probably achieve convergence\n"
														  "by re-fitting with the current\n"
														  "parameter estimate as initial guess\n"
														  "or try with new initial parameter estimates."));
			aborted = true;
			lbl_status2->setText(tr("Fit aborted..."));
			lbl_status3->setText("");
			lbl_status4->setText("");
			pb_cancel->setText(tr("Close"));
			pb_resume->setEnabled(false);
			pb_pause->setEnabled(false);
			pb_fit->setEnabled(true);
			return;
		}

// update theta. We don't know how good the new guess is, so we temporarily place this
// guess for theta into the extra theta vector (p+2 = [parameters+1]).
// Then we calculate the residuals and reorder afterwards:

		for (i=0; i<parameters; i++)
		{
			theta[parameters+1][i] = 0.0;
			for (j=0; j<parameters; j++)
			{
				theta[parameters+1][i] += delta_theta[j][i] * alpha[j];
			}
			theta[parameters+1][i] += theta[parameters][i];
		}
		for (i=0; i<parameters; i++)
		{
			theta[parameters+1][i] = dInit * theta[parameters+1][i] + (1.0 - dInit) * theta[parameters][i];
		}
		iteration ++;
		str.sprintf("%d",iteration);
		lbl_iteration2->setText(str);
		str.sprintf(tr("Working on Iteration %d"), iteration);
		lbl_status3->setText(str);
		update_simulation_parameters(parameters+1);
		completed = false;
		while (!completed)
		{
			calc_model(model);
			while (suspend_flag)
			{
				qApp->processEvents();
				if (aborted)
				{
					suspend_flag = false;
					return;
				}
			}
		}
		assign_F(parameters+1);
		calc_residuals(parameters+1);
		improvement =  variance[parameters] - variance[parameters+1];
		str.sprintf("%1.4e", variance[parameters+1]);
		lbl_variance2->setText(str);
		if (improvement > 0.0 )
		{
			lbl_best2->setText(str);
		}
		str.sprintf("%1.4e", pow(variance[parameters+1], 0.5));
		lbl_stddev2->setText(str);
		str.sprintf("%1.4e", improvement);
		lbl_improvement2->setText(str);
		loopcount = 0;
		while (improvement <= 0.0)		// we need a step shortening procedure here
		{	//implement step shortening procedure
			str.sprintf(tr("Implementing Step-shortening..."));
			lbl_status2->setText(str);
			loopcount++;
			
//cout << "Loop: " << loopcount << endl;
			str.sprintf(tr("Working on loop %d"), loopcount);
			lbl_status3->setText(str);
			d = (-1.0) * pow(-dInit, loopcount);
			for (i=0; i<parameters; i++)
			{
				theta[parameters+1][i] = d * theta[parameters+1][i] + (1.0 - d) * theta[parameters][i];
			}
			update_simulation_parameters(parameters+1);
			completed = false;
			while (!completed)
			{
				calc_model(model);	// calculate
				while (suspend_flag)
				{
					qApp->processEvents();	//take care of piled up events
					if (aborted)
					{
						suspend_flag = false;
						return;
					}
				}
			}
			assign_F(parameters+1);
			calc_residuals(parameters+1);
			improvement =  variance[parameters] - variance[parameters+1];
			str.sprintf("%1.4e", variance[parameters+1]);
			lbl_variance2->setText(str);
			if (improvement > 0 )
			{
				lbl_best2->setText(str);
			}
			str.sprintf("%1.4e", pow(variance[parameters+1], 0.5));
			lbl_stddev2->setText(str);
			str.sprintf("%1.4e", improvement);
			lbl_improvement2->setText(str);
			if (improvement == 0 || loopcount >= maxLoops)
			{
				converged = true;
				pb_overlays->setEnabled(true);
				pb_residuals->setEnabled(true);
				pb_print->setEnabled(true);
				pb_report->setEnabled(true);
				pb_saveFit->setEnabled(true);
				pb_fit->setEnabled(true);
				pb_pause->setEnabled(false);
				lbl_status2->setText(tr("Solution converged."));
				pb_cancel->setText(tr("Close"));
				write_data();
				cleanup_model(); // cleanup after objective function calculation if required...
				residuals();
				str.sprintf("%ld Runs (%2.2f", runs, runs_percent);
				str += " %)";
				lbl_status3->setText(str);
				lbl_status4->setText("");
				emit hasConverged();
				return; // we can't do any better, quit (this is the exit of this function)
			}
		}
//		if (alpha[0] >= 1.0e-5)
		if (fabs(alpha[0]) >= alphaCutoff)
		{
//cout << "alpha[0] is larger than 1e-5: " << alpha[0] << endl;
			shift_all();
		}
		else
		{
			j = 0;
//			while ((alpha[j] < 1.0e-5) && (j < parameters-1))
			while ((fabs(alpha[j]) < alphaCutoff) && (j < parameters))
			{
				j ++; // find first alpha[j] that is larger/equal than 1e-5 and use that 
			}			// index's alpha to replace with theta new
			if (j == parameters) // we didn't find one large enough, now lets just pick the largest available
			{
				j = 0;
				float max_alpha = -1.0;
				for (unsigned int k=0; k<parameters; k++)
				{
					if (fabs(alpha[k]) > max_alpha)
					{
						j = k;
						max_alpha = fabs(alpha[k]);
					}
				}
			}
			str.sprintf(tr("Updating vectors 1 and %d"), j+1);
//QMessageBox::message("Stop 3", str);

			lbl_status2->setText(str);
			loopcount++;
			for (i=0; i<parameters; i++) 
			{ // make sure that old values of theta1 aren't retained forever
				theta[0][i] = (theta[0][i] + theta[parameters+1][i]) / 2.0;
				theta[j][i] = theta[parameters+1][i]; // update theta[j] with the new theta.
				variance[j] = variance[parameters+1];
			} // and replace with the average of theta1 and theta_new
			for (i=0; i<allpoints; i++)
			{
				F[j][i] = F[parameters+1][i];	// we already have the F[j] from above, no need to re-calculate
			}
			str.sprintf(tr("Working on vector 1")); // we still need to calculate theta[0], since that one is new
			lbl_status3->setText(str);
			update_simulation_parameters(0);
			completed = false;
			while (!completed)
			{
				calc_model(model);
				while (suspend_flag)
				{
					qApp->processEvents();
					if (aborted)
					{
						suspend_flag = false;
						return;
					}
				}
			}
			assign_F(0);
			calc_residuals(0);
			improvement =  variance[parameters+1] - variance[0]; // the best one we have so far is theta[parameters+1]
			str.sprintf("%1.4e", variance[0]);
			lbl_variance2->setText(str);
			if (improvement > 0 )
			{
				lbl_best2->setText(str);
			}
			str.sprintf("%1.4e", pow(variance[0], 0.5));
			lbl_stddev2->setText(str);
			str.sprintf("%1.4e", improvement);
			lbl_improvement2->setText(str);
			str.sprintf(tr("Working on vector %d"), j+1);
			lbl_status3->setText(str);
			order_variance();
//QMessageBox::message("Stop 2", "Click on OK to continue...");

		}
	}
}


void US_Dud::F_init()
{
	unsigned int i;
	float improvement=0;
	QString str1;
	for (i=0; i<parameters+1; i++)
	{ 
		str1 = tr("Initializing Parameter Vectors...");
		lbl_status2->setText(str1);
		str1.sprintf(tr("Working on Vector %d of %d"), i+1, parameters+1);
		lbl_status3->setText(str1);
		update_simulation_parameters(i);
		completed = false;
		while (!completed)
		{
			calc_model(model);
			while (suspend_flag)
			{
				qApp->processEvents();
				if (aborted)
				{
					suspend_flag = false;
					return;
				}
			}
		}
		assign_F(i);
		calc_residuals(i);
		if (i > 0)
		{
			improvement =  variance[i-1] - variance[i];
		}
		str1.sprintf("%1.4e", variance[i]);
		lbl_variance2->setText(str1);
		str1.sprintf("%1.4e", pow(variance[i], 0.5));
		lbl_stddev2->setText(str1);
		str1.sprintf("%1.4e", improvement);
		lbl_improvement2->setText(str1);
	}
}

void US_Dud::swap()
{// reorders theta, F and variance, so they are in the proper order as determined by order_variance()
	float **matrix;
	unsigned int i, j;
	
	matrix = new float* [parameters+1];
	for (i=0; i<parameters+1; i++)
	{
		matrix[i] = new float [parameters];
	}
	for (i=0; i<parameters+1; i++)
	{
		for (j=0; j<parameters; j++)
		{
			matrix[i][j] = theta[order[i]][j];
		}
	}
	for (i=0; i<parameters+1; i++)
	{
		for (j=0; j<parameters; j++)
		{
			theta[i][j] = matrix[i][j];
		}
	}
	for (i=0; i<parameters+1; i++)
	{
		delete [] matrix[i];
	}
	delete [] matrix;
	matrix = new float* [parameters+1];
	for (i=0; i<parameters+1; i++)
	{
		matrix[i] = new float [allpoints];
	}
	for (i=0; i<parameters+1; i++)
	{
		for (j=0; j<allpoints; j++)
		{
			matrix[i][j] = F[order[i]][j];
		}
	}
	for (i=0; i<parameters+1; i++)
	{
		for (j=0; j<allpoints; j++)
		{
			F[i][j] = matrix[i][j];
		}
	}
	for (i=0; i<parameters+1; i++)
	{
		delete [] matrix[i];
	}
	delete [] matrix;
	for (i=0; i<parameters+1; i++)
	{
		variance[i] = old_variance[order[i]];
	}
	for (i=0; i<parameters+1; i++)
	{	//update old_variance so we have the correct values in the proper indices in the next iteration when we compare
		old_variance[i] = variance[i];
	}
}

void US_Dud::calc_deltas(const unsigned int i)
{
	unsigned int j;
	for (j=0; j<parameters; j++)
	{
		delta_theta[i][j] = theta[i][j] - theta[parameters][j];
//cout << "j: " << j << ", Delta-theta: " << delta_theta[i][j] << "\n";
	}

	for (j=0; j<allpoints; j++)
	{
		delta_F[i][j] = F[i][j] - F[parameters][j];
//cout  << "j: " << j << ", delta-F: " << delta_F[i][j] << "\n";
	}
}

void US_Dud::shift_all()
{
	unsigned int i,j;
	for (i=1; i<parameters+2; i++)
	{
		for (j=0; j<parameters; j++)
		{
			theta[i-1][j] = theta[i][j];
		}
	}
	for (i=1; i<parameters+2; i++)
	{
		for (j=0; j<allpoints; j++)
		{
			F[i-1][j] = F[i][j];
		}
	}
	for (i=1; i<parameters+2; i++)
	{
		variance[i-1] = variance[i];
	}
/*	
	for (i=0; i<parameters+1; i++)
	{
		cout << "Shift Variance[" << i << "]: "<<  variance[i] << endl;
	}
	cout << endl;
*/
}

bool US_Dud::calc_alpha()
{
	double **temp1_matrix, **temp2_matrix, *temp1_vector, *temp2_vector;
	unsigned int i, j, k, count;
	temp1_matrix = new double* [parameters];
	temp2_matrix = new double* [parameters];
	temp1_vector = new double  [allpoints];
	temp2_vector = new double  [parameters];
	for (i=0; i<parameters; i++)
	{
		temp1_matrix[i] = new double [parameters];
		temp2_matrix[i] = new double [parameters];
	}
	count = 0;
	calc_diff(&temp1_vector);
	for (i=0; i<parameters; i++)
	{
		temp2_vector[i] = 0.0;
		count = 0;
		for (j=0; j<datasets; j++)
		{
			for (k=0; k<points_per_dataset[j]; k++)
			{
				temp2_vector[i] += delta_F[i][count] * temp1_vector[count];
				count++;
			}
		}
		
// calculate delta_F(transpose) * delta_F, hold temporary result in temp1_matrix:

		for (j=0; j<parameters; j++)
		{
			temp1_matrix[i][j] = 0.0;
			for (k=0; k<allpoints; k++)
			{
				temp1_matrix[i][j] += delta_F[i][k] * delta_F[j][k];
			}
//			cout.precision(5);
//			cout << temp1_matrix[i][j] << "\t";
//			cout << j << ": " << temp1_matrix[i][j] << "\t";
		}
//		cout << endl;
		qApp->processEvents();
	}

// invert delta_F(transpose) * delta_F using Cholesky Decomposition, hold inverse in temp2_matrix:
// if the cholesky decomposition fails, we need to start over with the last best estimate for theta

	if (!Cholesky_Invert(temp1_matrix, temp2_matrix, parameters))
	{
		return(false);
	}

// multiply the inverse matrix by the temp_vector to obtain alpha:

	for (i=0; i<parameters; i++)
	{
		alpha[i] = 0.0;
		for (j=0; j<parameters; j++)
		{
			alpha[i]	+= temp2_matrix[i][j] * temp2_vector[j];
		}
	}
	for (i=0; i<parameters; i++)
	{
		delete [] temp1_matrix[i];
		delete [] temp2_matrix[i];
	}
	delete [] temp1_matrix;
	delete [] temp2_matrix;
	delete [] temp1_vector;
	delete [] temp2_vector;
	return(true);
}

void US_Dud::order_variance()
{
	unsigned int i, j;
	for (i=0; i<parameters+1; i++)
	{
		old_variance[i] = variance[i];
	}
	for (j=0; j<parameters+1; j++)
	{
		float maxval=0.0;
		for (i=0; i<parameters+1; i++)
		{
			if (variance[i] > maxval)
			{
				maxval = variance[i];
				order[j] = i;
			}
		}
		if (j != parameters)
		{
			variance[order[j]] = 0.0;
// do not lose the best variance, since we need it for comparison for the first iteration!
		}
	}
	swap();
/*	
	for (i=0; i<parameters+1; i++)
	{
		cout << "Variance[" << i << "]: "<<  variance[i] << endl;
	}
	cout << endl;
*/
}

void US_Dud::update_alphaCutoff(const QString &str)
{
	alphaCutoff = str.toFloat();
}

void US_Dud::update_dInit(const QString &str)
{
	dInit = str.toFloat();
}

void US_Dud::update_plotAll()
{
	plotGroup = false;
	bt_plotAll->setChecked(true);
	bt_plotGroup->setChecked(false);
	if (plotResiduals)
	{
		residuals();
	}
	else
	{
		overlays();
	}
}

void US_Dud::update_plotGroup()
{
	plotGroup = true;
	bt_plotAll->setChecked(false);
	bt_plotGroup->setChecked(true);
	if (plotResiduals)
	{
		residuals();
	}
	else
	{
		overlays();
	}
}

void US_Dud::update_maxLoops(const QString &str)
{
	maxLoops = str.toInt();
}

void US_Dud::cancel()
{
	*fitting_widget = false;
	close();
}

// Virtual Function Placeholders:

void US_Dud::suspendFit()
{
	suspend_flag = true;
	pb_resume->setEnabled(true);
	pb_residuals->setEnabled(true);
	pb_overlays->setEnabled(true);
	pb_pause->setEnabled(false);
	emit newParameters();
	emit fitSuspended();
}

void US_Dud::resumeFit()
{
	suspend_flag = false;
	pb_resume->setEnabled(false);
	pb_pause->setEnabled(true);
	emit fitResumed();
}

void US_Dud::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/dud_fit_ctrl.html");
}

void US_Dud::calc_residuals(const unsigned int current_parameter)
{
}

void US_Dud::calc_model(const unsigned int which_model)
{
}

void US_Dud::update_simulation_parameters(const unsigned int i)
{
}

bool US_Dud::dud_init()
{
	return(true);
}

void US_Dud::assign_F(const unsigned int i)
{
}

void US_Dud::calc_diff(double **diff_vector)
{
}

// Garbage collection for dud, reimplement in derived class:
void US_Dud::cleanup_dud()
{
}

// Garbage collection for the calculated model, reimplement in derived class if necessary:
void US_Dud::cleanup_model()
{
}

void US_Dud::write_data()
{
}

void US_Dud::update_final_fit()
{
	QString str;
	dud_init();	// we need to initialize the various parameters used in eqfitter before we can proceed
	calc_model(model);
	assign_F(parameters);
	calc_residuals(parameters);
	pb_saveFit->setEnabled(true);
	pb_report->setEnabled(true);
	pb_residuals->setEnabled(true);
	pb_overlays->setEnabled(true);
	pb_print->setEnabled(true);
	str.sprintf("%1.4e", variance[parameters]);
	lbl_variance2->setText(str);
	lbl_best2->setText(str);
	lbl_evaluations2->setText("0");
	str.sprintf("%1.4e", pow(variance[parameters], 0.5));
	lbl_stddev2->setText(str);
	str.sprintf("%1.4e", 0.0);
	lbl_improvement2->setText(str);
	lbl_status2->setText(tr("Results from loaded fit are shown"));
	str.sprintf(" %d", parameters);
	lbl_parameters2->setText(str);
	str.sprintf(" %d", datasets);
	lbl_datasets2->setText(str);
	str.sprintf(" %ld", allpoints);
	lbl_points2->setText(str);
	residuals();
	str.sprintf("%ld Runs (%2.2f", runs, runs_percent);
	str += " %)";
	lbl_status3->setText(str);
	lbl_status4->setText("");
}

void US_Dud::overlays()
{
	plotResiduals = false;
}

void US_Dud::residuals()
{
	plotResiduals = true;
}

void US_Dud::saveFit()
{
}

void US_Dud::print()
{
	QPrinter printer;
	if	(printer.setup(0))
	{
		data_plot->print(printer, QwtFltrInv());
	}
}

void US_Dud::view_report()
{
}

void US_Dud::updateRange(double scan)
{
	firstScan = (int) scan;
}
