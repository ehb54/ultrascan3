#include "../include/us_velocmodelctrl.h"

US_VelocModelControl::US_VelocModelControl(bool *temp_model_widget, int temp_components, 
int temp_model, vector <struct fefit_component> *temp_fef_comp,
struct fefit_run **temp_fef_run, float *temp_viscosity_tb, float *temp_density_tb, 
struct runinfo temp_run_inf, unsigned int temp_selected_cell, unsigned int temp_selected_lambda, 
unsigned int temp_selected_channel, vector <float> temp_Vbar20,
QWidget *parent, const char *name) : QFrame(parent, name)
{
	USglobal 	= new US_Config();
	run_inf			= temp_run_inf;
	selected_cell	= temp_selected_cell;
	selected_lambda	= temp_selected_lambda;
	selected_channel	= temp_selected_channel;
	components 		= temp_components;
	baseline 		= run_inf.baseline[selected_cell][selected_lambda];
	meniscus 		= run_inf.meniscus[selected_cell];
	total_conc 		= run_inf.plateau[selected_cell][selected_lambda][0] - baseline;
	temperature		= temp_run_inf.avg_temperature;
	viscosity_tb 	= temp_viscosity_tb;
	density_tb	 	= temp_density_tb;
	fef_comp		 	= temp_fef_comp;
	fef_run 			= temp_fef_run;
	run_inf			= temp_run_inf;
	const_Vbar20	= temp_Vbar20; // this value is saved for reset operation
	Vbar20			= temp_Vbar20; // this value can be modified by the user
	model 			= temp_model;
	model_widget	= temp_model_widget;
	*model_widget   = true;
	
	current_component = 0;
	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	int xpos = 4;
	int ypos = 5;
	int buttonh = 26;
	int column1 = 160;
	int column2 = 90;
	int column3 = 90;
	int column4 = 30;
	int column5 = 40;
	int column6 = 40;
	int column7 = 50;
	int span = column1 + column2 + column3 + column4 + column5 + column6 + column7;
	
	setCaption(tr("Model Control"));

	lbl_header = new QLabel("", this);
	lbl_header->setAlignment(AlignCenter|AlignVCenter);
	lbl_header->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_header->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_header->setGeometry(xpos, ypos, span, buttonh);
	lbl_header->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos = 4;
	ypos += buttonh + spacing * 2;
	
	lbl_sample = new QLabel(tr("Sample:"),this);
	lbl_sample->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_sample->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sample->setGeometry(xpos, ypos, column1, buttonh);
	lbl_sample->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_sample_descr = new QLabel(run_inf.cell_id[selected_cell], this);
	lbl_sample_descr->setAlignment(AlignLeft|AlignVCenter);
	lbl_sample_descr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_sample_descr->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_sample_descr->setPalette(QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	lbl_sample_descr->setGeometry(xpos, ypos, span - column1 - spacing, buttonh-1);

	xpos = 4;
	ypos += buttonh + 15;

// Row 1:
	
	lbl_legend1 = new QLabel(tr("Run Parameter:"),this);
	lbl_legend1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_legend1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_legend1->setGeometry(xpos, ypos, span, buttonh);
	lbl_legend1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_legend2 = new QLabel(tr("Initial Guess:"),this);
	lbl_legend2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_legend2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_legend2->setGeometry(xpos, ypos, column2, buttonh);
	lbl_legend2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;

	lbl_legend3 = new QLabel(tr("+/- Bounds:"),this);
	lbl_legend3->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_legend3->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_legend3->setGeometry(xpos, ypos, column3 + column4 + column5, buttonh);
	lbl_legend3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column3 + column4;

	lbl_legend4 = new QLabel(tr("Float:"),this);
	lbl_legend4->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_legend4->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_legend4->setGeometry(xpos, ypos, column5, buttonh);
	lbl_legend4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column5 + spacing;

	lbl_legend5 = new QLabel(tr("Fix:"),this);
	lbl_legend5->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_legend5->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_legend5->setGeometry(xpos, ypos, column6, buttonh);
	lbl_legend5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column6 + spacing;

	lbl_legend6 = new QLabel(tr("Bound:"),this);
	lbl_legend6->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_legend6->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_legend6->setGeometry(xpos-13, ypos, column7, buttonh);
	lbl_legend6->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

// Row 2:
	
	xpos = 4;
	ypos += buttonh + spacing;

	lbl_meniscus = new QLabel(tr("Meniscus:"),this);
	lbl_meniscus->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_meniscus->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_meniscus->setGeometry(xpos, ypos, column1, buttonh);
	lbl_meniscus->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_meniscus = new QLineEdit(this);
	le_meniscus->setGeometry(xpos, ypos, column2, buttonh-1);
	le_meniscus->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_meniscus->setText("");
	le_meniscus->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_meniscus, SIGNAL(textChanged(const QString &)), SLOT(update_meniscus(const QString &)));

	xpos += column2 + spacing;

	le_meniscus_range = new QLineEdit(this);
	le_meniscus_range->setGeometry(xpos, ypos, column3, buttonh-1);
	le_meniscus_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_meniscus_range->setText("");
	le_meniscus_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_meniscus_range, SIGNAL(textChanged(const QString &)), SLOT(update_meniscus_range(const QString &)));

	xpos += column3 + spacing;

	lbl_percent = new QLabel("  cm",this);
	lbl_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_percent->setGeometry(xpos, ypos, column4, buttonh);
	lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column4 + spacing;

	cb_meniscus_float = new QCheckBox(this);
	cb_meniscus_float->setGeometry(xpos + (unsigned int) (column5/2) - 10, ypos+5, 14, 14);
	cb_meniscus_float->setChecked(false);
	cb_meniscus_float->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_meniscus_float->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_meniscus_float, SIGNAL(clicked()), SLOT(update_meniscus_float()));

	xpos += column5;

	cb_meniscus_fix = new QCheckBox(this);
	cb_meniscus_fix->setGeometry(xpos + (unsigned int) (column6/2) - 15, ypos+5, 14, 14);
	cb_meniscus_fix->setChecked(true);
	cb_meniscus_fix->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_meniscus_fix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	connect(cb_meniscus_fix, SIGNAL(clicked()), SLOT(update_meniscus_fix()));

	xpos += column6;

	lcd_meniscus = new QLabel("", this);
	lcd_meniscus->setGeometry(xpos, ypos+5, 14, 14);
	lcd_meniscus->setFrameStyle(QFrame::WinPanel|Sunken);
	lcd_meniscus->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );

// Row 3:
	
	xpos = 4;
	ypos += buttonh + spacing;

	lbl_baseline = new QLabel(tr("Baseline:"),this);
	lbl_baseline->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_baseline->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_baseline->setGeometry(xpos, ypos, column1, buttonh);
	lbl_baseline->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_baseline = new QLineEdit(this);
	le_baseline->setGeometry(xpos, ypos, column2, buttonh-1);
	le_baseline->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_baseline->setText("");
	le_baseline->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_baseline, SIGNAL(textChanged(const QString &)), SLOT(update_baseline(const QString &)));

	xpos += column2 + spacing;

	le_baseline_range = new QLineEdit(this);
	le_baseline_range->setGeometry(xpos, ypos, column3, buttonh-1);
	le_baseline_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_baseline_range->setText("");
	le_baseline_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_baseline_range, SIGNAL(textChanged(const QString &)), SLOT(update_baseline_range(const QString &)));

	xpos += column3 + spacing;

	lbl_percent = new QLabel("  OD",this);
	lbl_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_percent->setGeometry(xpos, ypos, column4, buttonh);
	lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column4 + spacing;

	cb_baseline_float = new QCheckBox(this);
	cb_baseline_float->setGeometry(xpos + (unsigned int) (column5/2) - 10, ypos+5, 14, 14);
	cb_baseline_float->setChecked(true);
	cb_baseline_float->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_baseline_float->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_baseline_float, SIGNAL(clicked()), SLOT(update_baseline_float()));

	xpos += column5;

	cb_baseline_fix = new QCheckBox(this);
	cb_baseline_fix->setGeometry(xpos + (unsigned int) (column6/2) - 15, ypos+5, 14, 14);
	cb_baseline_fix->setChecked(false);
	cb_baseline_fix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_baseline_fix->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_baseline_fix, SIGNAL(clicked()), SLOT(update_baseline_fix()));

	xpos += column6;

	lcd_baseline = new QLabel("", this);
	lcd_baseline->setGeometry(xpos, ypos+5, 14, 14);
	lcd_baseline->setFrameStyle(QFrame::WinPanel|Sunken);
	lcd_baseline->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );

// Row 4:
	
	xpos = 4;
	ypos += buttonh + spacing;

	lbl_slope = new QLabel(tr("Slope Correction(r):"),this);
	lbl_slope->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_slope->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_slope->setGeometry(xpos, ypos, column1, buttonh);
	lbl_slope->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_slope = new QLineEdit(this);
	le_slope->setGeometry(xpos, ypos, column2, buttonh-1);
	le_slope->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_slope->setText("");
	le_slope->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_slope, SIGNAL(textChanged(const QString &)), SLOT(update_slope(const QString &)));

	xpos += column2 + spacing;

	(*fef_run)->slope_range = 0.001;
	le_slope_range = new QLineEdit(this);
	le_slope_range->setGeometry(xpos, ypos, column3, buttonh-1);
	le_slope_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_slope_range->setText("");
	le_slope_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_slope_range, SIGNAL(textChanged(const QString &)), SLOT(update_slope_range(const QString &)));

	xpos += column3 + spacing;

	lbl_percent = new QLabel("  OD",this);
	lbl_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_percent->setGeometry(xpos, ypos, column4, buttonh);
	lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column4 + spacing;

	cb_slope_float = new QCheckBox(this);
	cb_slope_float->setGeometry(xpos + (unsigned int) (column5/2) - 10, ypos+5, 14, 14);
	cb_slope_float->setChecked(true);
	cb_slope_float->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_slope_float->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_slope_float, SIGNAL(clicked()), SLOT(update_slope_float()));

	xpos += column5;

	cb_slope_fix = new QCheckBox(this);
	cb_slope_fix->setGeometry(xpos + (unsigned int) (column6/2) - 15, ypos+5, 14, 14);
	cb_slope_fix->setChecked(false);
	cb_slope_fix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_slope_fix->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_slope_fix, SIGNAL(clicked()), SLOT(update_slope_fix()));

	xpos += column6;

	lcd_slope = new QLabel("", this);
	lcd_slope->setGeometry(xpos, ypos+5, 14, 14);
	lcd_slope->setFrameStyle(QFrame::WinPanel|Sunken);
	lcd_slope->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );

// Row 5:
	
	xpos = 4;
	ypos += buttonh + spacing;

	lbl_stray = new QLabel(tr("Slope Correction(c,r):"),this);
	lbl_stray->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_stray->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_stray->setGeometry(xpos, ypos, column1, buttonh);
	lbl_stray->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_stray = new QLineEdit(this);
	le_stray->setGeometry(xpos, ypos, column2, buttonh-1);
	le_stray->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_stray->setText("");
	le_stray->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_stray, SIGNAL(textChanged(const QString &)), SLOT(update_stray(const QString &)));

	xpos += column2 + spacing;

	(*fef_run)->stray_range = 0.001;
	le_stray_range = new QLineEdit(this);
	le_stray_range->setGeometry(xpos, ypos, column3, buttonh-1);
	le_stray_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_stray_range->setText("");
	le_stray_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_stray_range, SIGNAL(textChanged(const QString &)), SLOT(update_stray_range(const QString &)));

	xpos += column3 + spacing;

	lbl_percent = new QLabel("  OD",this);
	lbl_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_percent->setGeometry(xpos, ypos, column4, buttonh);
	lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column4 + spacing;

	cb_stray_float = new QCheckBox(this);
	cb_stray_float->setGeometry(xpos + (unsigned int) (column5/2) - 10, ypos+5, 14, 14);
	cb_stray_float->setChecked(true);
	cb_stray_float->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_stray_float->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	connect(cb_stray_float, SIGNAL(clicked()), SLOT(update_stray_float()));

	xpos += column5;

	cb_stray_fix = new QCheckBox(this);
	cb_stray_fix->setGeometry(xpos + (unsigned int) (column6/2) - 15, ypos+5, 14, 14);
	cb_stray_fix->setChecked(false);
	cb_stray_fix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_stray_fix->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_stray_fix, SIGNAL(clicked()), SLOT(update_stray_fix()));

	xpos += column6;

	lcd_stray = new QLabel("", this);
	lcd_stray->setGeometry(xpos, ypos+5, 14, 14);
	lcd_stray->setFrameStyle(QFrame::WinPanel|Sunken);
	lcd_stray->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );

// Row 6:

	xpos = 4;
	ypos += buttonh + spacing;

	lbl_simpoints = new QLabel(tr("Radial Points:"),this);
	lbl_simpoints->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_simpoints->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_simpoints->setGeometry(xpos, ypos, column1 + column2 + column3 + 2*spacing, buttonh+2*spacing);
	lbl_simpoints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	ct_simpoints= new QwtCounter(this);
	ct_simpoints->setNumButtons(3);
	ct_simpoints->setRange(50, 1e6, 10);
	ct_simpoints->setValue(100);
	ct_simpoints->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	ct_simpoints->setGeometry(xpos, ypos+3, column2 + column3 + spacing, 23);
	connect(ct_simpoints, SIGNAL(valueChanged(double)), SLOT(assign_simpoints(double)));

// Row 7:

	xpos = 4;
	ypos += buttonh + 15;

	lbl_component = new QLabel(tr("Parameters for Component:"),this);
	lbl_component->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_component->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_component->setGeometry(xpos, ypos, span, buttonh+4);
	lbl_component->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + column2 + 2 * spacing;

	ct_component= new QwtCounter(this);
	ct_component->setNumButtons(2);
	ct_component->setRange(1, components, 1);
	ct_component->setValue(1);
	ct_component->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	ct_component->setGeometry(xpos - (unsigned int) column2/2, ypos+3, (unsigned int) column2/2 + column3, 23);
//	connect(ct_component, SIGNAL(buttonReleased(double)), SLOT(assign_component(double)));
	connect(ct_component, SIGNAL(valueChanged(double)), SLOT(assign_component(double)));


	xpos = 4 + column1 + column2 + column3 + column4 + 2 * spacing;

	lbl_legend4 = new QLabel(tr("Float:"),this);
	lbl_legend4->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_legend4->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_legend4->setGeometry(xpos, ypos, column5, buttonh);
	lbl_legend4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column5 + spacing;

	lbl_legend5 = new QLabel(tr("Fix:"),this);
	lbl_legend5->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_legend5->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_legend5->setGeometry(xpos, ypos, column6, buttonh);
	lbl_legend5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column6 + spacing;

	lbl_legend6 = new QLabel(tr("Bound:"),this);
	lbl_legend6->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_legend6->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_legend6->setGeometry(xpos-13, ypos, column7, buttonh);
	lbl_legend6->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

// Row 8:
	
	xpos = 4;
	ypos += buttonh + 4 + spacing;

	lbl_sed = new QLabel(tr("Sedimentation Coeff.:"),this);
	lbl_sed->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_sed->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sed->setGeometry(xpos, ypos, column1 + spacing, buttonh);
	lbl_sed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_sed = new QLineEdit(this);
	le_sed->setGeometry(xpos, ypos, column2, buttonh-1);
	le_sed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sed->setText("");
	le_sed->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_sed, SIGNAL(textChanged(const QString &)), SLOT(update_sed(const QString &)));

	xpos += column2 + spacing;

	le_sed_range = new QLineEdit(this);
	le_sed_range->setGeometry(xpos, ypos, column3, buttonh-1);
	le_sed_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sed_range->setText("");
	le_sed_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_sed_range, SIGNAL(textChanged(const QString &)), SLOT(update_sed_range(const QString &)));

	xpos += column3 + spacing;

	lbl_percent = new QLabel("  s",this);
	lbl_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_percent->setGeometry(xpos, ypos, column4, buttonh);
	lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column4 + spacing;

	cb_sed_float = new QCheckBox(this);
	cb_sed_float->setGeometry(xpos + (unsigned int) (column5/2) - 10, ypos+5, 14, 14);
	cb_sed_float->setChecked(true);
	cb_sed_float->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_sed_float->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_sed_float, SIGNAL(clicked()), SLOT(update_sed_float()));

	xpos += column5;

	cb_sed_fix = new QCheckBox(this);
	cb_sed_fix->setGeometry(xpos + (unsigned int) (column6/2) - 15, ypos+5, 14, 14);
	cb_sed_fix->setChecked(false);
	cb_sed_fix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_sed_fix->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_sed_fix, SIGNAL(clicked()), SLOT(update_sed_fix()));

	xpos += column6;

	lcd_sed = new QLabel("", this);
	lcd_sed->setGeometry(xpos, ypos+5, 14, 14);
	lcd_sed->setFrameStyle(QFrame::WinPanel|Sunken);
	lcd_sed->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );

// Row 9:
	
	xpos = 4;
	ypos += buttonh + spacing;

	lbl_diff = new QLabel(tr("Diffusion Coeff.:"),this);
	lbl_diff->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_diff->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_diff->setGeometry(xpos, ypos, column1 + spacing, buttonh);
	lbl_diff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_diff = new QLineEdit(this);
	le_diff->setGeometry(xpos, ypos, column2, buttonh-1);
	le_diff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_diff->setText("");
	le_diff->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_diff, SIGNAL(textChanged(const QString &)), SLOT(update_diff(const QString &)));

	xpos += column2 + spacing;

	le_diff_range = new QLineEdit(this);
	le_diff_range->setGeometry(xpos, ypos, column3, buttonh-1);
	le_diff_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_diff_range->setText("");
	le_diff_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_diff_range, SIGNAL(textChanged(const QString &)), SLOT(update_diff_range(const QString &)));

	xpos += column3 + spacing;

	lbl_percent = new QLabel("  D",this);
	lbl_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_percent->setGeometry(xpos, ypos, column4, buttonh);
	lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column4 + spacing;

	cb_diff_float = new QCheckBox(this);
	cb_diff_float->setGeometry(xpos + (unsigned int) (column5/2) - 10, ypos+5, 14, 14);
	cb_diff_float->setChecked(true);
	cb_diff_float->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_diff_float->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_diff_float, SIGNAL(clicked()), SLOT(update_diff_float()));

	xpos += column5;

	cb_diff_fix = new QCheckBox(this);
	cb_diff_fix->setGeometry(xpos + (unsigned int) (column6/2) - 15, ypos+5, 14, 14);
	cb_diff_fix->setChecked(false);
	cb_diff_fix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_diff_fix->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_diff_fix, SIGNAL(clicked()), SLOT(update_diff_fix()));

	xpos += column6;

	lcd_diff = new QLabel("", this);
	lcd_diff->setGeometry(xpos, ypos+5, 14, 14);
	lcd_diff->setFrameStyle(QFrame::WinPanel|Sunken);
	lcd_diff->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );

// Row 10:
	
	xpos = 4;
	ypos += buttonh + spacing;

	lbl_conc = new QLabel("",this);	
	lbl_conc->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_conc->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_conc->setGeometry(xpos, ypos, column1 + spacing, buttonh);
	lbl_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_conc = new QLineEdit(this);
	le_conc->setGeometry(xpos, ypos, column2, buttonh-1);
	le_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_conc->setText("");
	le_conc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_conc, SIGNAL(textChanged(const QString &)), SLOT(update_conc(const QString &)));

	xpos += column2 + spacing;

	le_conc_range = new QLineEdit(this);
	le_conc_range->setGeometry(xpos, ypos, column3, buttonh-1);
	le_conc_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_conc_range->setText("");
	le_conc_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_conc_range, SIGNAL(textChanged(const QString &)), SLOT(update_conc_range(const QString &)));

	xpos += column3 + spacing;

	lbl_percent = new QLabel("  OD",this);
	lbl_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_percent->setGeometry(xpos, ypos, column4, buttonh);
	lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column4 + spacing;

	cb_conc_float = new QCheckBox(this);
	cb_conc_float->setGeometry(xpos + (unsigned int) (column5/2) - 10, ypos+5, 14, 14);
	cb_conc_float->setChecked(true);
	cb_conc_float->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_conc_float->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_conc_float, SIGNAL(clicked()), SLOT(update_conc_float()));

	xpos += column5;

	cb_conc_fix = new QCheckBox(this);
	cb_conc_fix->setGeometry(xpos + (unsigned int) (column6/2) - 15, ypos+5, 14, 14);
	cb_conc_fix->setChecked(false);
	cb_conc_fix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_conc_fix->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_conc_fix, SIGNAL(clicked()), SLOT(update_conc_fix()));

	xpos += column6;

	lcd_conc = new QLabel("", this);
	lcd_conc->setGeometry(xpos, ypos+5, 14, 14);
	lcd_conc->setFrameStyle(QFrame::WinPanel|Sunken);
	lcd_conc->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );

// Row 11:
	
	xpos = 4;
	ypos += buttonh + spacing;

	lbl_sigma = new QLabel(tr("Sigma:"),this);
	lbl_sigma->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_sigma->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sigma->setGeometry(xpos, ypos, column1 + spacing, buttonh);
	lbl_sigma->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_sigma = new QLineEdit(this);
	le_sigma->setGeometry(xpos, ypos, column2, buttonh-1);
	le_sigma->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sigma->setText("");
	le_sigma->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_sigma, SIGNAL(textChanged(const QString &)), SLOT(update_sigma(const QString &)));

	xpos += column2 + spacing;

	le_sigma_range = new QLineEdit(this);
	le_sigma_range->setGeometry(xpos, ypos, column3, buttonh-1);
	le_sigma_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sigma_range->setText("");
	le_sigma_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_sigma_range, SIGNAL(textChanged(const QString &)), SLOT(update_sigma_range(const QString &)));

	xpos += column3 + spacing;

	lbl_percent = new QLabel("  %",this);
	lbl_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_percent->setGeometry(xpos, ypos, column4, buttonh);
	lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column4 + spacing;

	cb_sigma_float = new QCheckBox(this);
	cb_sigma_float->setGeometry(xpos + (unsigned int) (column5/2) - 10, ypos+5, 14, 14);
	cb_sigma_float->setChecked(false);
	cb_sigma_float->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_sigma_float->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_sigma_float, SIGNAL(clicked()), SLOT(update_sigma_float()));

	xpos += column5;

	cb_sigma_fix = new QCheckBox(this);
	cb_sigma_fix->setGeometry(xpos + (unsigned int) (column6/2) - 15, ypos+5, 14, 14);
	cb_sigma_fix->setChecked(true);
	cb_sigma_fix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_sigma_fix->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_sigma_fix, SIGNAL(clicked()), SLOT(update_sigma_fix()));

	xpos += column6;

	lcd_sigma = new QLabel("", this);
	lcd_sigma->setGeometry(xpos, ypos+5, 14, 14);
	lcd_sigma->setFrameStyle(QFrame::WinPanel|Sunken);
	lcd_sigma->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );

// Row 12:
	
	xpos = 4;
	ypos += buttonh + spacing;

	lbl_delta = new QLabel(tr("Delta:"),this);
	lbl_delta->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_delta->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_delta->setGeometry(xpos, ypos, column1 + spacing, buttonh);
	lbl_delta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_delta = new QLineEdit(this);
	le_delta->setGeometry(xpos, ypos, column2, buttonh-1);
	le_delta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_delta->setText("");
	le_delta->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_delta, SIGNAL(textChanged(const QString &)), SLOT(update_delta(const QString &)));

	xpos += column2 + spacing;

	le_delta_range = new QLineEdit(this);
	le_delta_range->setGeometry(xpos, ypos, column3, buttonh-1);
	le_delta_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_delta_range->setText("");
	le_delta_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_delta_range, SIGNAL(textChanged(const QString &)), SLOT(update_delta_range(const QString &)));

	xpos += column3 + spacing;

	lbl_percent = new QLabel("  %",this);
	lbl_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_percent->setGeometry(xpos, ypos, column4, buttonh);
	lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column4 + spacing;

	cb_delta_float = new QCheckBox(this);
	cb_delta_float->setGeometry(xpos + (unsigned int) (column5/2) - 10, ypos+5, 14, 14);
	cb_delta_float->setChecked(false);
	cb_delta_float->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_delta_float->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_delta_float, SIGNAL(clicked()), SLOT(update_delta_float()));

	xpos += column5;

	cb_delta_fix = new QCheckBox(this);
	cb_delta_fix->setGeometry(xpos + (unsigned int) (column6/2) - 15, ypos+5, 14, 14);
	cb_delta_fix->setChecked(true);
	cb_delta_fix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_delta_fix->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_delta_fix, SIGNAL(clicked()), SLOT(update_delta_fix()));

	xpos += column6;

	lcd_delta = new QLabel("", this);
	lcd_delta->setGeometry(xpos, ypos+5, 14, 14);
	lcd_delta->setFrameStyle(QFrame::WinPanel|Sunken);
	lcd_delta->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );

// Row 13:
	
	xpos = 4;
	ypos += buttonh + spacing;

	lbl_mw = new QLabel(tr("Molecular Weight:"),this);
	lbl_mw->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_mw->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw->setGeometry(xpos, ypos, column1, buttonh);
	lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_mw = new QLineEdit(this);
	le_mw->setGeometry(xpos, ypos, column2, buttonh-1);
	le_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_mw->setText("");
	le_mw->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_mw, SIGNAL(textChanged(const QString &)), SLOT(update_mw(const QString &)));

	xpos += column2 + spacing;

	lbl_percent = new QLabel(tr("  Dalton"),this);
	lbl_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_percent->setGeometry(xpos, ypos, column3, buttonh);
	lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column3 + column4 + 2*spacing;

	cb_mw_float = new QCheckBox(this);
	cb_mw_float->setGeometry(xpos + (unsigned int) (column5/2) - 10, ypos+5, 14, 14);
	cb_mw_float->setChecked(true);
	cb_mw_float->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_mw_float->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_mw_float, SIGNAL(clicked()), SLOT(update_mw_float()));

	xpos += column5;

	cb_mw_fix = new QCheckBox(this);
	cb_mw_fix->setGeometry(xpos + (unsigned int) (column6/2) - 15, ypos+5, 14, 14);
	cb_mw_fix->setChecked(false);
	cb_mw_fix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_mw_fix->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_mw_fix, SIGNAL(clicked()), SLOT(update_mw_fix()));

// Row 14:
	
	xpos = 4;
	ypos += buttonh + spacing;

	pb_vbar = new QPushButton(tr("Vbar (20º):"), this);
	Q_CHECK_PTR(pb_vbar);
	pb_vbar->setAutoDefault(false);
	pb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	pb_vbar->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_vbar->setGeometry(xpos, ypos, column1, buttonh);
	connect(pb_vbar, SIGNAL(clicked()), SLOT(read_vbar()));

	xpos += column1 + spacing;
	QString str;
	str.sprintf("%4.7f", Vbar20[current_component]);
	le_vbar = new QLineEdit(this);
	le_vbar->setGeometry(xpos, ypos, column2, buttonh-1);
	le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_vbar->setText(str);
	le_vbar->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_model_vbar(const QString &)));

	xpos += column2 + spacing;

	lbl_percent = new QLabel(tr("  ccm/g"),this);
	lbl_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_percent->setGeometry(xpos, ypos, column3, buttonh);
	lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column3 + column4 + 2*spacing;


	cb_vbar_float = new QCheckBox(this);
	cb_vbar_float->setGeometry(xpos + (unsigned int) (column5/2) - 10, ypos+5, 14, 14);
	cb_vbar_float->setChecked(false);
	cb_vbar_float->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_vbar_float->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_vbar_float, SIGNAL(clicked()), SLOT(update_vbar_float()));

	xpos += column5;

	cb_vbar_fix = new QCheckBox(this);
	cb_vbar_fix->setGeometry(xpos + (unsigned int) (column6/2) - 15, ypos+5, 14, 14);
	cb_vbar_fix->setChecked(true);
	cb_vbar_fix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_vbar_fix->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_vbar_fix, SIGNAL(clicked()), SLOT(update_vbar_fix()));

// Row 15:

	xpos = 4;
	ypos += buttonh + spacing;

	lbl1_s20w = new QLabel(tr("S20,W:"),this);
	lbl1_s20w->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl1_s20w->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl1_s20w->setGeometry(xpos, ypos, column1, buttonh);
	lbl1_s20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl2_s20w = new QLabel("",this);
	lbl2_s20w->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl2_s20w->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl2_s20w->setGeometry(xpos, ypos, column2, buttonh-1);
	lbl2_s20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;

	lbl_percent = new QLabel("  s",this);
	lbl_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_percent->setGeometry(xpos, ypos, column3, buttonh);
	lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

// Row 16:

	xpos = 4;
	ypos += buttonh + spacing;

	lbl1_d20w = new QLabel(tr("D20,W:"),this);
	lbl1_d20w->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl1_d20w->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl1_d20w->setGeometry(xpos, ypos, column1, buttonh-1);
	lbl1_d20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl2_d20w = new QLabel("",this);
	lbl2_d20w->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl2_d20w->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl2_d20w->setGeometry(xpos, ypos, column2, buttonh);
	lbl2_d20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));


	xpos += column2 + spacing;

	lbl_percent = new QLabel("  D",this);
	lbl_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_percent->setGeometry(xpos, ypos, column3, buttonh);
	lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos = 4;
	ypos += buttonh + 15;

	int buttonw = (unsigned int)((span - (3.0 * spacing)) / 4.0);

	pb_update = new QPushButton(tr("Update for 20º,W"), this);
	Q_CHECK_PTR(pb_update);
	pb_update->setAutoDefault(false);
	pb_update->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	pb_update->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_update->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_update, SIGNAL(clicked()), SLOT(update_labels()));

	xpos += buttonw + spacing;

	pb_model = new QPushButton(tr("Model Results"), this);
	Q_CHECK_PTR(pb_model);
	pb_model->setAutoDefault(false);
	pb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	pb_model->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_model->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_model, SIGNAL(clicked()), SLOT(show_model()));

	xpos += buttonw + spacing;

/*
	pb_noise = new QPushButton(tr("Invariant Noise"), this);
	Q_CHECK_PTR(pb_noise);
	pb_noise->setAutoDefault(false);
	pb_noise->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	pb_noise->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_noise->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_noise->setEnabled(false);
//	connect(pb_noise, SIGNAL(clicked()), SLOT(noise()));
*/

	pb_reset_range = new QPushButton(tr("Reset Range"), this);
	Q_CHECK_PTR(pb_reset_range);
	pb_reset_range->setAutoDefault(false);
	pb_reset_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	pb_reset_range->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_reset_range->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_reset_range->setEnabled(true);
	connect(pb_reset_range, SIGNAL(clicked()), SLOT(reset_range()));

	xpos += buttonw + spacing;

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	xpos = 4;
	ypos += buttonh + spacing;

	pb_load = new QPushButton(tr("Load Model"), this);
	Q_CHECK_PTR(pb_load);
	pb_load->setAutoDefault(false);
	pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_load->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_load, SIGNAL(clicked()), SLOT(load_model()));

	xpos += buttonw + spacing;

	pb_save = new QPushButton(tr("Save Model"), this);
	Q_CHECK_PTR(pb_save);
	pb_save->setAutoDefault(false);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_save->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_save, SIGNAL(clicked()), SLOT(save_model()));

	xpos += buttonw + spacing;

	pb_reset = new QPushButton(tr("Reset Parameters"), this);
	Q_CHECK_PTR(pb_reset);
	pb_reset->setAutoDefault(false);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_reset->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

	xpos += buttonw + spacing;

	pb_quit = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_quit);
	pb_quit->setAutoDefault(false);
	pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_quit->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

	ypos += buttonh;
	global_Xpos += 30;
	global_Ypos += 30;

	setGeometry(global_Xpos, global_Ypos, span + 8, ypos + 4); 
	setMinimumSize(span + 8, ypos + 4); 

	reset();
}

US_VelocModelControl::~US_VelocModelControl()
{
	*model_widget = false;
}

void US_VelocModelControl::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
	*model_widget = false;
}

void US_VelocModelControl::update_labels()
{
	QString str;
	int i;

	for (i=0; i<components; i++)
	{
		if (((*fef_comp)[i].mw_fit) && (!(*fef_comp)[i].vbar_fit) && ((*fef_comp)[i].diff_fit))
		{
			calc_mw(i); // vbar is fixed, but D and MW can float
		}
		else if ((!(*fef_comp)[i].mw_fit) && ((*fef_comp)[i].vbar_fit) && ((*fef_comp)[i].diff_fit))
		{
			calc_vbar(i); // MW is fixed, but D and vbar can float
		}
		else if ((!(*fef_comp)[i].mw_fit) && (!(*fef_comp)[i].vbar_fit) && (!(*fef_comp)[i].diff_fit))
		{
			calc_D(i); // MW, D and vbar are fixed, so D needs to be adjusted according to S/D ratio (S/D = C * MW, where C is a constant)
		}
	}
	switch (model)
	{
		case 0:
		{
			str.sprintf(tr("Non-Interacting %d-Component Model"), components);
			lbl_conc->setText(tr("Part. Concentration:"));
			break;
		}
		case 1:
		{
			str.sprintf(tr("Non-Interacting %d-Component Model"), components);
			lbl_conc->setText(tr("Part. Concentration:"));
			break;
		}
		case 2:
		{
			str.sprintf(tr("Non-Interacting %d-Component Model"), components);
			lbl_conc->setText(tr("Part. Concentration:"));
			break;
		}
		case 3:
		{
			str.sprintf(tr("Fixed Molecular Weight %d-Component Model"), components);
			lbl_conc->setText(tr("Part. Concentration:"));
			break;
		}
		case 4:
		{
			str.sprintf(tr("Monomer-Dimer Self-associating Model"));
			(*fef_comp)[1].mw = 2.0 * (*fef_comp)[0].mw;
			(*fef_comp)[1].vbar = (*fef_comp)[0].vbar;
			(*fef_comp)[1].vbar_fit = false;
			(*fef_comp)[1].mw_fit = false;
			(*fef_comp)[1].diff_fit = false;
			(*fef_comp)[0].sigma_fit = false;
			(*fef_comp)[0].delta_fit = false;
			(*fef_comp)[1].sigma_fit = false;
			(*fef_comp)[1].delta_fit = false;
			cb_sigma_float->setEnabled(false);
			cb_sigma_fix->setEnabled(false);
			cb_delta_float->setEnabled(false);
			cb_delta_fix->setEnabled(false);
			le_sigma->setEnabled(false);
			le_delta->setEnabled(false);
			le_sigma_range->setEnabled(false);
			le_delta_range->setEnabled(false);
			lbl_sigma->setPalette(QPalette(USglobal->global_colors.cg_label_disabled, USglobal->global_colors.cg_label_disabled, USglobal->global_colors.cg_label_disabled));
			lbl_delta->setPalette(QPalette(USglobal->global_colors.cg_label_disabled, USglobal->global_colors.cg_label_disabled, USglobal->global_colors.cg_label_disabled));

			if (current_component == 0)
			{
				lbl_conc->setText(tr("Total Concentration:"));
				lbl_component->setText(tr("Parameters for Monomer"));
			}
			else
			{
				lbl_component->setText(tr("Parameters for Dimer"));
				lbl_conc->setText(tr("Equilibrium Constant:"));
			}
			break;
		}
		case 20:
		{
			str.sprintf(tr("2-State Isomerization Model"));
			(*fef_comp)[1].mw = (*fef_comp)[0].mw;
			(*fef_comp)[1].vbar = (*fef_comp)[0].vbar;
			(*fef_comp)[1].vbar_fit = false;
			(*fef_comp)[1].mw_fit = false;
			(*fef_comp)[1].diff_fit = false;
			(*fef_comp)[0].sigma_fit = false;
			(*fef_comp)[0].delta_fit = false;
			(*fef_comp)[1].sigma_fit = false;
			(*fef_comp)[1].delta_fit = false;
			cb_sigma_float->setEnabled(false);
			cb_sigma_fix->setEnabled(false);
			cb_delta_float->setEnabled(false);
			cb_delta_fix->setEnabled(false);
			le_sigma->setEnabled(false);
			le_delta->setEnabled(false);
			le_sigma_range->setEnabled(false);
			le_delta_range->setEnabled(false);
			lbl_sigma->setPalette(QPalette(USglobal->global_colors.cg_label_disabled, USglobal->global_colors.cg_label_disabled, USglobal->global_colors.cg_label_disabled));
			lbl_delta->setPalette(QPalette(USglobal->global_colors.cg_label_disabled, USglobal->global_colors.cg_label_disabled, USglobal->global_colors.cg_label_disabled));
			if (current_component == 0)
			{
				lbl_component->setText(tr("Parameters for Isomer:"));
				lbl_conc->setText(tr("Total Concentration:"));
			}
			else
			{
				lbl_component->setText(tr("Parameters for Isomer:"));
				lbl_conc->setText(tr("Equilibrium Constant:"));
			}
			break;
		}
	}
	lbl_header->setText(str);
	str.sprintf("%1.6f", (*fef_run)->meniscus);
	le_meniscus->setText(str);
	str.sprintf("%1.6f", (*fef_run)->meniscus_range);
	le_meniscus_range->setText(str);

	str.sprintf("%1.5e", (*fef_run)->baseline);
	le_baseline->setText(str);
	str.sprintf("%1.5e", (*fef_run)->baseline_range);
	le_baseline_range->setText(str);

	str.sprintf("%1.5e", (*fef_run)->slope);
	le_slope->setText(str);
	str.sprintf("%1.5e", (*fef_run)->slope_range);
	le_slope_range->setText(str);

	str.sprintf("%1.5e", (*fef_run)->stray);
	le_stray->setText(str);
	str.sprintf("%1.5e", (*fef_run)->stray_range);
	le_stray_range->setText(str);

	str.sprintf("%1.5e", (*fef_comp)[current_component].sed);
	le_sed->setText(str);
	str.sprintf("%1.2e", (*fef_comp)[current_component].sed_range);
	le_sed_range->setText(str);

	str.sprintf("%1.5e", (*fef_comp)[current_component].diff);
	le_diff->setText(str);
	str.sprintf("%1.2e", (*fef_comp)[current_component].diff_range);
	le_diff_range->setText(str);

	str.sprintf("%1.6f", (*fef_comp)[current_component].conc);
	le_conc->setText(str);
	str.sprintf("%1.6f", (*fef_comp)[current_component].conc_range);
	le_conc_range->setText(str);

	str.sprintf("%1.5e", (*fef_comp)[current_component].sigma);
	le_sigma->setText(str);
	str.sprintf("%1.5e", (*fef_comp)[current_component].sigma_range);
	le_sigma_range->setText(str);

	str.sprintf("%1.5e", (*fef_comp)[current_component].delta);
	le_delta->setText(str);
	str.sprintf("%1.5e", (*fef_comp)[current_component].delta_range);
	le_delta_range->setText(str);
	
	str.sprintf("%1.5e", (*fef_comp)[current_component].mw);
	le_mw->setText(str);

	str.sprintf("%1.5e", (*fef_comp)[current_component].vbar - (4.25e-4 * (temperature - 20)));
	le_vbar->setText(str);

	str.sprintf("%1.5e", (*fef_comp)[current_component].s20w);
	lbl2_s20w->setText(str);

	str.sprintf("%1.5e", (*fef_comp)[current_component].d20w);
	lbl2_d20w->setText(str);

	if ((*fef_run)->meniscus_fit)
	{
		cb_meniscus_float->setChecked(true);
		cb_meniscus_fix->setChecked(false);
	}
	else
	{
		cb_meniscus_float->setChecked(false);
		cb_meniscus_fix->setChecked(true);
	}
	if ((*fef_run)->baseline_fit)
	{
		cb_baseline_float->setChecked(true);
		cb_baseline_fix->setChecked(false);
	}
	else
	{
		cb_baseline_float->setChecked(false);
		cb_baseline_fix->setChecked(true);
	}
	if ((*fef_run)->slope_fit)
	{
		cb_slope_float->setChecked(true);
		cb_slope_fix->setChecked(false);
	}
	else
	{
		cb_slope_float->setChecked(false);
		cb_slope_fix->setChecked(true);
	}
	if ((*fef_run)->stray_fit)
	{
		cb_stray_float->setChecked(true);
		cb_stray_fix->setChecked(false);
	}
	else
	{
		cb_stray_float->setChecked(false);
		cb_stray_fix->setChecked(true);
	}
	ct_simpoints->setValue((*fef_run)->sim_points);
	if ((*fef_comp)[current_component].sed_fit)
	{
		cb_sed_float->setChecked(true);
		cb_sed_fix->setChecked(false);
	}
	else
	{
		cb_sed_float->setChecked(false);
		cb_sed_fix->setChecked(true);
	}
	if ((*fef_comp)[current_component].diff_fit)
	{
		cb_diff_float->setChecked(true);
		cb_diff_fix->setChecked(false);
	}
	else
	{
		cb_diff_float->setChecked(false);
		cb_diff_fix->setChecked(true);
	}
	if ((*fef_comp)[current_component].conc_fit)
	{
		cb_conc_float->setChecked(true);
		cb_conc_fix->setChecked(false);
	}
	else
	{
		cb_conc_float->setChecked(false);
		cb_conc_fix->setChecked(true);
	}
	if ((*fef_comp)[current_component].sigma_fit)
	{
		cb_sigma_float->setChecked(true);
		cb_sigma_fix->setChecked(false);
	}
	else
	{
		cb_sigma_float->setChecked(false);
		cb_sigma_fix->setChecked(true);
	}
	if ((*fef_comp)[current_component].delta_fit)
	{
		cb_delta_float->setChecked(true);
		cb_delta_fix->setChecked(false);
	}
	else
	{
		cb_delta_float->setChecked(false);
		cb_delta_fix->setChecked(true);
	}
	if ((*fef_comp)[current_component].mw_fit)
	{
		cb_mw_float->setChecked(true);
		cb_mw_fix->setChecked(false);
	}
	else
	{
		cb_mw_float->setChecked(false);
		cb_mw_fix->setChecked(true);
	}
	if ((*fef_comp)[current_component].vbar_fit)
	{
		cb_vbar_float->setChecked(true);
		cb_vbar_fix->setChecked(false);
	}
	else
	{
		cb_vbar_float->setChecked(false);
		cb_vbar_fix->setChecked(true);
	}
/*
	if ((model == 1 || model == 2) && current_component == 1)
	{
		cb_diff_float->setEnabled(false);
		cb_diff_fix->setEnabled(false);
		cb_mw_float->setEnabled(false);
		cb_mw_fix->setEnabled(false);
		cb_vbar_float->setEnabled(false);
		cb_vbar_fix->setEnabled(false);
	}
	else
	{
		cb_diff_float->setEnabled(true);
		cb_diff_fix->setEnabled(true);
		cb_mw_float->setEnabled(true);
		cb_mw_fix->setEnabled(true);
		cb_vbar_float->setEnabled(true);
		cb_vbar_fix->setEnabled(true);
	}
*/
	cb_diff_float->setEnabled(true);
	cb_diff_fix->setEnabled(true);
	cb_mw_float->setEnabled(true);
	cb_mw_fix->setEnabled(true);
	cb_vbar_float->setEnabled(true);
	cb_vbar_fix->setEnabled(true);
}

void US_VelocModelControl::update_meniscus(const QString &val)
{
	(*fef_run)->meniscus = val.toFloat();
}

void US_VelocModelControl::update_meniscus_range(const QString &val)
{
	(*fef_run)->meniscus_range = val.toFloat();
}

void US_VelocModelControl::update_meniscus_float()
{
	(*fef_run)->meniscus_fit = true;
	cb_meniscus_float->setChecked(true);
	cb_meniscus_fix->setChecked(false);
}

void US_VelocModelControl::update_meniscus_fix()
{
	(*fef_run)->meniscus_fit = false;
	cb_meniscus_float->setChecked(false);
	cb_meniscus_fix->setChecked(true);
}

void US_VelocModelControl::assign_component(double val)
{
	current_component = (unsigned int) val - 1;
	update_labels();
}

void US_VelocModelControl::assign_simpoints(double val)
{
	(*fef_run)->sim_points = (unsigned int) val;
}

void US_VelocModelControl::update_baseline(const QString &val)
{
	(*fef_run)->baseline = val.toFloat();
}

void US_VelocModelControl::update_baseline_range(const QString &val)
{
	(*fef_run)->baseline_range = val.toFloat();
}

void US_VelocModelControl::update_baseline_float()
{
	(*fef_run)->baseline_fit = true;
	cb_baseline_float->setChecked(true);
	cb_baseline_fix->setChecked(false);
}

void US_VelocModelControl::update_baseline_fix()
{
	(*fef_run)->baseline_fit = false;
	cb_baseline_float->setChecked(false);
	cb_baseline_fix->setChecked(true);
}

void US_VelocModelControl::update_slope(const QString &val)
{
	(*fef_run)->slope = val.toFloat();
}

void US_VelocModelControl::update_slope_range(const QString &val)
{
	(*fef_run)->slope_range = val.toFloat();
}

void US_VelocModelControl::update_slope_float()
{
	(*fef_run)->slope_fit = true;
	cb_slope_float->setChecked(true);
	cb_slope_fix->setChecked(false);
}

void US_VelocModelControl::update_slope_fix()
{
	(*fef_run)->slope_fit = false;
	cb_slope_float->setChecked(false);
	cb_slope_fix->setChecked(true);
}

void US_VelocModelControl::update_stray(const QString &val)
{
	(*fef_run)->stray = val.toFloat();
}

void US_VelocModelControl::update_stray_range(const QString &val)
{
	(*fef_run)->stray_range = val.toFloat();
}

void US_VelocModelControl::update_stray_float()
{
	(*fef_run)->stray_fit = true;
	cb_stray_float->setChecked(true);
	cb_stray_fix->setChecked(false);
}

void US_VelocModelControl::update_stray_fix()
{
	(*fef_run)->stray_fit = false;
	cb_stray_float->setChecked(false);
	cb_stray_fix->setChecked(true);
}

void US_VelocModelControl::update_sed(const QString &val)
{
	(*fef_comp)[current_component].sed = val.toFloat();
}

void US_VelocModelControl::update_sed_range(const QString &val)
{
	(*fef_comp)[current_component].sed_range = val.toFloat();
}

void US_VelocModelControl::update_sed_float()
{
	(*fef_comp)[current_component].sed_fit = true;
	cb_sed_float->setChecked(true);
	cb_sed_fix->setChecked(false);
}

void US_VelocModelControl::update_sed_fix()
{
	(*fef_comp)[current_component].sed_fit = false;
	cb_sed_float->setChecked(false);
	cb_sed_fix->setChecked(true);
}

void US_VelocModelControl::update_diff(const QString &val)
{
	(*fef_comp)[current_component].diff = val.toFloat();
}

void US_VelocModelControl::update_diff_range(const QString &val)
{
	(*fef_comp)[current_component].diff_range = val.toFloat();
}

void US_VelocModelControl::update_diff_float()
{
	(*fef_comp)[current_component].diff_fit = true;
	cb_diff_float->setChecked(true);
	cb_diff_fix->setChecked(false);
}

void US_VelocModelControl::update_diff_fix()
{
	(*fef_comp)[current_component].diff_fit = false;
	cb_diff_float->setChecked(false);
	cb_diff_fix->setChecked(true);
}

void US_VelocModelControl::update_conc(const QString &val)
{
	(*fef_comp)[current_component].conc = val.toFloat();
}

void US_VelocModelControl::update_conc_range(const QString &val)
{
	(*fef_comp)[current_component].conc_range = val.toFloat();
}

void US_VelocModelControl::update_conc_float()
{
	(*fef_comp)[current_component].conc_fit = true;
	cb_conc_float->setChecked(true);
	cb_conc_fix->setChecked(false);
}

void US_VelocModelControl::update_conc_fix()
{
	(*fef_comp)[current_component].conc_fit = false;
	cb_conc_float->setChecked(false);
	cb_conc_fix->setChecked(true);
}

void US_VelocModelControl::update_sigma(const QString &val)
{
	(*fef_comp)[current_component].sigma = val.toFloat();
}

void US_VelocModelControl::update_sigma_range(const QString &val)
{
	(*fef_comp)[current_component].sigma_range = val.toFloat();
}

void US_VelocModelControl::update_sigma_float()
{
	(*fef_comp)[current_component].sigma_fit = true;
	cb_sigma_float->setChecked(true);
	cb_sigma_fix->setChecked(false);
}

void US_VelocModelControl::update_sigma_fix()
{
	(*fef_comp)[current_component].sigma_fit = false;
	cb_sigma_float->setChecked(false);
	cb_sigma_fix->setChecked(true);
}

void US_VelocModelControl::update_delta(const QString &val)
{
	(*fef_comp)[current_component].delta = val.toFloat();
}

void US_VelocModelControl::update_delta_range(const QString &val)
{
	(*fef_comp)[current_component].delta_range = val.toFloat();
}

void US_VelocModelControl::update_delta_float()
{
	(*fef_comp)[current_component].delta_fit = true;
	cb_delta_float->setChecked(true);
	cb_delta_fix->setChecked(false);
}

void US_VelocModelControl::update_delta_fix()
{
	(*fef_comp)[current_component].delta_fit = false;
	cb_delta_float->setChecked(false);
	cb_delta_fix->setChecked(true);
}

void US_VelocModelControl::update_mw(const QString &val)
{
	(*fef_comp)[current_component].mw = val.toFloat();
}

void US_VelocModelControl::update_mw_float()
{
	(*fef_comp)[current_component].mw_fit = true;
	cb_mw_float->setChecked(true);
	cb_mw_fix->setChecked(false);
	(*fef_comp)[current_component].vbar_fit = false;
	cb_vbar_float->setChecked(false);
	cb_vbar_fix->setChecked(true);
// see comments in int US_Finite_W::dud()
}

void US_VelocModelControl::update_mw_fix()
{
	(*fef_comp)[current_component].mw_fit = false;
	cb_mw_float->setChecked(false);
	cb_mw_fix->setChecked(true);
	if (!(*fef_comp)[current_component].vbar_fit)
	{
		(*fef_comp)[current_component].diff_fit = false;
		cb_diff_float->setChecked(false);
		cb_diff_fix->setChecked(true);
	}
// see comments in int US_Finite_W::dud()
}

void US_VelocModelControl::update_model_vbar(const QString &val)
{
	Vbar20[current_component] = val.toFloat();
	(*fef_comp)[current_component].vbar = Vbar20[current_component] + (4.25e-4 * (temperature - 20));
}

void US_VelocModelControl::update_vbar_float()
{
	(*fef_comp)[current_component].vbar_fit = true;
	cb_vbar_float->setChecked(true);
	cb_vbar_fix->setChecked(false);
	(*fef_comp)[current_component].mw_fit = false;
	cb_mw_float->setChecked(false);
	cb_mw_fix->setChecked(true);
// see comments in int US_Finite_W::dud()
}

void US_VelocModelControl::update_vbar_fix()
{
	(*fef_comp)[current_component].vbar_fit = false;
	cb_vbar_float->setChecked(false);
	cb_vbar_fix->setChecked(true);
	if (!(*fef_comp)[current_component].mw_fit)
	{
		(*fef_comp)[current_component].diff_fit = false;
		cb_diff_float->setChecked(false);
		cb_diff_fix->setChecked(true);
	}
// see comments in "int US_Finite_W::dud()"
}

void US_VelocModelControl::update_vbar_lbl(float val, float val20)
{
	(*fef_comp)[current_component].vbar = val;
	Vbar20[current_component] = val20;
	QString str;
	str.sprintf("%6.4f", Vbar20[current_component]);
	le_vbar->setText(str);
}

void US_VelocModelControl::read_vbar()
{
	vbar_from_seq = true;
	vbar_dlg = new US_Vbar_DB(temperature, &(*fef_comp)[current_component].vbar, &Vbar20[current_component], true, false, run_inf.investigator);
	vbar_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	vbar_dlg->setCaption(tr("V-bar Calculation"));
	vbar_dlg->show();
	connect(vbar_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_vbar_lbl(float, float)));
}

void US_VelocModelControl::load_model()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.root_dir, "*.fef_model", 0);
	if ( !fn.isEmpty() ) 
	{
		load_model(fn);
	}
}

void US_VelocModelControl::load_model(const QString &fileName)
{
	QFile f(fileName);
	int i, j, temp, temp_model, temp_components;
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);

// read 4 headerlines and throw away:

		for (i=0; i<4; i++)
		{
			ts.readLine();
		}
		if (!ts.atEnd())
		{
			ts >> temp_model;
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (model != temp_model)
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe file you have selected contains a model\n"
																		"that does not match the model from the current\n"
																		"settings. Please select a different file\n"
																		"or change the current settings."));
			return;
		
		}
		if (!ts.atEnd())
		{
			ts >> temp_components;
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}

// make sure we read a model compatible with the current settings:

		if (components != temp_components)
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe file you have selected does not match\n"
																		"the number of components from the current\n"
																		"settings. Please select a different file\n"
																		"or change the current settings."));
			return;
		
		}
		if (!ts.atEnd())
		{
			ts >> (*fef_run)->meniscus;
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*fef_run)->meniscus_range;
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.atEnd())
		{
			ts >> temp;
			if (temp == 0)
			{
				(*fef_run)->meniscus_fit = false;
			}
			else
			{
				(*fef_run)->meniscus_fit = true;
			}
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*fef_run)->baseline;
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*fef_run)->baseline_range;
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.atEnd())
		{
			ts >> temp;
			if (temp == 0)
			{
				(*fef_run)->baseline_fit = false;
			}
			else
			{
				(*fef_run)->baseline_fit = true;
			}
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*fef_run)->slope;
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*fef_run)->slope_range;
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.atEnd())
		{
			ts >> temp;
			if (temp == 0)
			{
				(*fef_run)->slope_fit = false;
			}
			else
			{
				(*fef_run)->slope_fit = true;
			}
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*fef_run)->stray;
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*fef_run)->stray_range;
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.atEnd())
		{
			ts >> temp;
			if (temp == 0)
			{
				(*fef_run)->stray_fit = false;
			}
			else
			{
				(*fef_run)->stray_fit = true;
			}
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*fef_run)->delta_t;
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*fef_run)->delta_r;
			ts.readLine();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		for (i=0; i<components; i++)
		{
			for (j=0; j<3; j++) // read 3 headerlines and throw away:
			{
				ts.readLine();
			}
			if (!ts.atEnd())
			{
				ts >> (*fef_comp)[i].sed;
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*fef_comp)[i].sed_range;
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> temp;
				if (temp == 0)
				{
					(*fef_comp)[i].sed_fit = false;
				}
				else
				{
					(*fef_comp)[i].sed_fit = true;
				}
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*fef_comp)[i].diff;
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*fef_comp)[i].diff_range;
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> temp;
				if (temp == 0)
				{
					(*fef_comp)[i].diff_fit = false;
				}
				else
				{
					(*fef_comp)[i].diff_fit = true;
				}
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*fef_comp)[i].conc;
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*fef_comp)[i].conc_range;
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> temp;
				if (temp == 0)
				{
					(*fef_comp)[i].conc_fit = false;
				}
				else
				{
					(*fef_comp)[i].conc_fit = true;
				}
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*fef_comp)[i].sigma;
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*fef_comp)[i].sigma_range;
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> temp;
				if (temp == 0)
				{
					(*fef_comp)[i].sigma_fit = false;
				}
				else
				{
					(*fef_comp)[i].sigma_fit = true;
				}
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*fef_comp)[i].delta;
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*fef_comp)[i].delta_range;
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> temp;
				if (temp == 0)
				{
					(*fef_comp)[i].delta_fit = false;
				}
				else
				{
					(*fef_comp)[i].delta_fit = true;
				}
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> temp;
				calc_mw(i);
				if (temp == 0)
				{
					(*fef_comp)[i].mw_fit = false;
				}
				else
				{
					(*fef_comp)[i].mw_fit = true;
				}
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
			if (!ts.atEnd())
			{
				ts >> temp;
				if (temp == 0)
				{
					(*fef_comp)[i].vbar_fit = false;
				}
				else
				{
					(*fef_comp)[i].vbar_fit = true;
				}
				ts.readLine();
			}
			else
			{
				f.close();
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
				return;
			}
		}
		f.close();
	}
	update_labels();
}

void US_VelocModelControl::save_model()
{
	QString fn = QFileDialog::getSaveFileName( USglobal->config_list.root_dir, "*.fef_model", 0);
	int k;
	if ( !fn.isEmpty() ) 
	{
		k = fn.find(".", 0, FALSE);
		if (k != -1) //if an extension was given, strip it
		{
			fn.truncate(k);
		}
		save_model(fn);		// the user gave a file name
	}
}

void US_VelocModelControl::save_model(const QString &filename)
{
	QString filestr;
	filestr = filename;
	filestr.append(".fef_model");
	QFile fef_file(filestr);
	if (fef_file.exists())
	{
		if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
		{
			fef_file.close();
			return;
		}
	}
	if (fef_file.open(IO_WriteOnly | IO_Translate))
	{
		int i;
		QTextStream ts(&fef_file);
		ts.setf(0x1000);
		ts << "*************************************\n";
		ts << tr("*   Please do not edit this file!   *\n");
		ts << "*************************************\n\n";
		ts << model << "\t# " << modelString[model].latin1() << endl;
		ts << components << tr("\t\t# Number of Components\n");
		ts << (*fef_run)->meniscus       << tr("\t# Meniscus in cm\n");
		ts << (*fef_run)->meniscus_range << tr("\t# Meniscus range in cm\n");
		ts << (int) (*fef_run)->meniscus_fit << tr("\t\t# Meniscus fitting control\n");
		ts << (*fef_run)->baseline       << tr("\t# Baseline in OD\n");
		ts << (*fef_run)->baseline_range << tr("\t# Baseline range in OD\n");
		ts << (int) (*fef_run)->baseline_fit << tr("\t\t# Baseline fitting control\n");
		ts << (*fef_run)->slope       << tr("\t# Slope(r) Correction in OD\n");
		ts << (*fef_run)->slope_range << tr("\t# Slope(r) Correction range in OD\n");
		ts << (int) (*fef_run)->slope_fit << tr("\t\t# Slope(r) Correction fitting control\n");
		ts << (*fef_run)->stray       << tr("\t# Slope(c,r) Correction in OD\n");
		ts << (*fef_run)->stray_range << tr("\t# Slope(c,r) Correction range in OD\n");
		ts << (int) (*fef_run)->stray_fit << tr("\t\t# Slope(c,r) Correction fitting control\n");
		ts << (*fef_run)->delta_t << tr("\t# Delta_t in seconds\n"); 
		ts << (*fef_run)->delta_r << tr("\t# Delta_r in cm\n");
		for (i=0; i<components; i++)
		{
			if (model == 4)
			{
				if (i==0)
				{
					ts << tr("\nParameters for Monomer:\n\n");
				}
				else
				{
					ts << tr("\nParameters for Dimer:\n\n");
				}
			}
			else if(model == 20)
			{
				if (i==0)
				{
					ts << tr("\nParameters for Isomer 1:\n\n");
				}
				else
				{
					ts << tr("\nParameters for Isomer 2:\n\n");
				}
			}
			else
			{
				ts << tr("\nParameters for Component ") << (i+1) << ":\n\n";
			}
			ts << (*fef_comp)[i].sed << tr("\t# Sedimentation Coefficient in s\n");
			ts << (*fef_comp)[i].sed_range << tr("\t# Sedimentation Coefficient range in s\n");
			ts << (int) (*fef_comp)[i].sed_fit << tr("\t\t# Sedimentation Coefficient fitting control\n");
			ts << (*fef_comp)[i].diff << tr("\t# Diffusion Coefficient in D\n");
			ts << (*fef_comp)[i].diff_range << tr("\t# Diffusion Coefficient range in D\n");
			ts << (int) (*fef_comp)[i].diff_fit << tr("\t\t# Diffusion Coefficient fitting control\n");
			if (model == 4)
			{
				if (i==0)
				{
					ts << (*fef_comp)[i].conc << tr("\t# Total Concentration in OD\n");
					ts << (*fef_comp)[i].conc_range << tr("\t# Total Concentration range in OD\n");
					ts << (int) (*fef_comp)[i].conc_fit << tr("\t\t# Total Concentration fitting control\n");
				}
				else
				{
					ts << (*fef_comp)[i].conc << tr("\t# Monomer-Dimer association constant in OD\n");
					ts << (*fef_comp)[i].conc_range << tr("\t# Monomer-Dimer association constant range in OD\n");
					ts << (int) (*fef_comp)[i].conc_fit << tr("\t\t# Monomer-Dimer association constant fitting control\n");
				}
			}
			else if (model == 20)
			{
				if (i==0)
				{
					ts << (*fef_comp)[i].conc << tr("\t# Total Concentration in OD\n");
					ts << (*fef_comp)[i].conc_range << tr("\t# Total Concentration range in OD\n");
					ts << (int) (*fef_comp)[i].conc_fit << tr("\t\t# Total Concentration fitting control\n");
				}
				else
				{
					ts << (*fef_comp)[i].conc << tr("\t# Isomerization equilibrium constant in OD\n");
					ts << (*fef_comp)[i].conc_range << tr("\t# Isomerization equilibrium constant range in OD\n");
					ts << (int) (*fef_comp)[i].conc_fit << tr("\t\t# Isomerization equilibrium constant fitting control\n");
				}
			}
			else
			{

				ts << (*fef_comp)[i].conc << tr("\t# Partial Concentration in OD\n");
				ts << (*fef_comp)[i].conc_range << tr("\t# Partial Concentration range in OD\n");
				ts << (int) (*fef_comp)[i].conc_fit << tr("\t\t# Partial Concentration fitting control\n");
			}
			ts << (*fef_comp)[i].sigma << tr("\t# Sigma\n");
			ts << (*fef_comp)[i].sigma_range << tr("\t# Sigma range\n");
			ts << (int) (*fef_comp)[i].sigma_fit << tr("\t\t# Sigma fitting control\n");
			ts << (*fef_comp)[i].delta << tr("\t# Delta\n");
			ts << (*fef_comp)[i].delta_range << tr("\t# Delta range\n");
			ts << (int) (*fef_comp)[i].delta_fit << tr("\t\t# Delta fitting control\n");
			ts << (int) (*fef_comp)[i].mw_fit << tr("\t\t# Molecular Weight fitting control\n");
			ts << (int) (*fef_comp)[i].vbar_fit << tr("\t\t# Part. Spec. Volume fitting control\n");
		}
		fef_file.close();
	}
}

void US_VelocModelControl::reset()
{
	int i=0;

// Run parameters:

	(*fef_run)->meniscus = meniscus;
	(*fef_run)->meniscus_range = 0.003;
	(*fef_run)->meniscus_fit = false;
	(*fef_run)->baseline = baseline;
	(*fef_run)->baseline_range = 0.05;
	(*fef_run)->baseline_fit = true;
	(*fef_run)->slope = 0.0;
	(*fef_run)->slope_range = 0.001;
	(*fef_run)->slope_fit = true;
	(*fef_run)->stray = 0.0;
	(*fef_run)->stray_range = 0.001;
	(*fef_run)->stray_fit = true;
	(*fef_run)->delta_t = 20.0;
	(*fef_run)->delta_r = 0.001;
	(*fef_run)->sim_points = 100;

	
// concentration:
	switch (model)
	{
		case (0):
		{
			for (i=0; i<components; i++)
			{
				(*fef_comp)[i].conc = total_conc/components;
			}
			break;
		}
		case (1):
		{
			for (i=0; i<components; i++)
			{
				(*fef_comp)[i].conc = total_conc/components;
			}
			break;
		}
		case (2):
		{
			for (i=0; i<components; i++)
			{
				(*fef_comp)[i].conc = total_conc/components;
			}
			break;
		}
		case (3):
		{
			for (i=0; i<components; i++)
			{
				(*fef_comp)[i].conc = total_conc/components;
			}
			break;
		}
		case (4):
		{
			(*fef_comp)[0].conc = total_conc;
			(*fef_comp)[1].conc = 1.0;
			break;
		}
		case (20):
		{
			(*fef_comp)[0].conc = total_conc;
			(*fef_comp)[1].conc = 1.0;
			break;
		}
	}
	for (i=0; i<components; i++)
	{
		(*fef_comp)[i].sed = 2.5E-13 * (i+1);
		(*fef_comp)[i].s20w = 0.0;
		(*fef_comp)[i].diff = 9.0E-7 / (i+1);
		(*fef_comp)[i].d20w = 0.0;
		(*fef_comp)[i].sigma = 0.0;
		(*fef_comp)[i].delta = 0.0;
		(*fef_comp)[i].mw = 0.0;
		Vbar20[i] = const_Vbar20[i]; // reset vbar 20 to the originally passed value
		(*fef_comp)[i].vbar = Vbar20[i] + (4.25e-4 * (temperature - 20));
		(*fef_comp)[i].conc_range = (*fef_comp)[i].conc/5.0;
		(*fef_comp)[i].sed_range = (*fef_comp)[i].sed/5.0;
		(*fef_comp)[i].diff_range = (*fef_comp)[i].diff/5.0;
		(*fef_comp)[i].sigma_range = (*fef_comp)[i].sigma/5.0;
		(*fef_comp)[i].delta_range = (*fef_comp)[i].delta/5.0;
		(*fef_comp)[i].conc_fit = true;
		(*fef_comp)[i].sed_fit = true;
		(*fef_comp)[i].diff_fit = true;
		(*fef_comp)[i].sigma_fit = false;
		(*fef_comp)[i].delta_fit = false;
		(*fef_comp)[i].mw_fit = true;
		(*fef_comp)[i].vbar_fit = false;
	}
	update_labels();
}

void US_VelocModelControl::reset_range()
{
	(*fef_run)->meniscus_range = 0.003;
	(*fef_run)->baseline_range = 0.05;
	(*fef_run)->slope_range = 0.001;
	(*fef_run)->stray_range = 0.001;

	for (int i=0; i<components; i++)
	{
		(*fef_comp)[i].conc_range = (*fef_comp)[i].conc/5.0;
		(*fef_comp)[i].sed_range = (*fef_comp)[i].sed/5.0;
		(*fef_comp)[i].diff_range = (*fef_comp)[i].diff/5.0;
		(*fef_comp)[i].sigma_range = (*fef_comp)[i].sigma/5.0;
		(*fef_comp)[i].delta_range = (*fef_comp)[i].delta/5.0;
	}
	update_labels();
}

void US_VelocModelControl::calc_mw(const int comp)
{
	buoyancyb	 = 1 - (*fef_comp)[comp].vbar * (*density_tb);
	buoyancyw	 = 1 - ((*fef_comp)[comp].vbar - (4.25e-4 * (temperature - 20))) * DENS_20W;
	(*fef_comp)[comp].s20w = (buoyancyw / buoyancyb) * ((*viscosity_tb) / (100.0 * VISC_20W)) 
													  * (*fef_comp)[comp].sed;
	(*fef_comp)[comp].d20w = K20/(K0 + temperature) * ((*viscosity_tb) / (100.0 * VISC_20W))
													  * (*fef_comp)[comp].diff;
	(*fef_comp)[comp].mw = ((*fef_comp)[comp].s20w/(*fef_comp)[comp].d20w)
													* ((R * K20)/buoyancyw);
}

void US_VelocModelControl::calc_vbar(const int comp)
{
	(*fef_comp)[comp].vbar = (1.0 - ((*fef_comp)[comp].sed * R * (temperature + 
	K0))/((*fef_comp)[comp].diff * (*fef_comp)[comp].mw))/(*density_tb);
	buoyancyb	 = 1 - (*fef_comp)[comp].vbar * (*density_tb);
	buoyancyw	 = 1 - ((*fef_comp)[comp].vbar - (4.25e-4 * (temperature - 20))) * DENS_20W;
	(*fef_comp)[comp].s20w = (buoyancyw / buoyancyb) * ((*viscosity_tb) / (100.0 * VISC_20W)) 
													  * (*fef_comp)[comp].sed;
	(*fef_comp)[comp].d20w = K20/(K0 + temperature) * ((*viscosity_tb) / (100.0 * VISC_20W))
													  * (*fef_comp)[comp].diff;
}

void US_VelocModelControl::calc_D(const int comp)
{
	buoyancyb	 = 1 - (*fef_comp)[comp].vbar * (*density_tb);
	buoyancyw	 = 1 - ((*fef_comp)[comp].vbar - (4.25e-4 * (temperature - 20))) * DENS_20W;
	(*fef_comp)[comp].diff = ((*fef_comp)[comp].sed * R * (temperature + K0))/
	((*fef_comp)[comp].mw * buoyancyb);
	(*fef_comp)[comp].s20w = (buoyancyw / buoyancyb) * ((*viscosity_tb) / (100.0 * VISC_20W)) 
													  * (*fef_comp)[comp].sed;
	(*fef_comp)[comp].d20w = K20/(K0 + temperature) * ((*viscosity_tb) / (100.0 * VISC_20W))
													  * (*fef_comp)[comp].diff;
}

void US_VelocModelControl::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/finite_model_ctrl.html");
}

void US_VelocModelControl::show_model()
{
	QString str;
	switch (model)
	{
		case 0:
		{
			str.sprintf(tr("Molecular Shape Model for Component %d"), current_component+1);
			break;
		}
		case 1:
		{
			if (current_component == 0)
			{
				str.sprintf(tr("Molecular Shape Model for Monomer"));
			}
			else
			{
				str.sprintf(tr("Molecular Shape Model for Dimer"));
			}
			break;
		}
		case 2:
		{
			str.sprintf("Molecular Shape Model for Isomer %d", current_component+1);
			break;
		}
	}
	update_labels();
	shape = new US_Hydro3(1, &(*fef_comp)[current_component].s20w, &(*fef_comp)[current_component].d20w,
								 &(*fef_comp)[current_component].mw, &Vbar20[current_component], str, current_component);
	shape->show();
}

void US_VelocModelControl::meniscus_red()
{
	lcd_meniscus->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
}

void US_VelocModelControl::meniscus_green()
{
	lcd_meniscus->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
}

void US_VelocModelControl::meniscus_dk_green()
{
	lcd_meniscus->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
}

void US_VelocModelControl::baseline_red()
{
	lcd_baseline->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
}

void US_VelocModelControl::baseline_green()
{
	lcd_baseline->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
}

void US_VelocModelControl::baseline_dk_green()
{
	lcd_baseline->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
}


void US_VelocModelControl::slope_red()
{
	lcd_slope->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
}

void US_VelocModelControl::slope_green()
{
	lcd_slope->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
}

void US_VelocModelControl::slope_dk_green()
{
	lcd_slope->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
}

void US_VelocModelControl::stray_red()
{
	lcd_stray->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
}

void US_VelocModelControl::stray_green()
{
	lcd_stray->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
}

void US_VelocModelControl::stray_dk_green()
{
	lcd_stray->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
}

void US_VelocModelControl::sed_red()
{
	lcd_sed->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
}

void US_VelocModelControl::sed_green()
{
	lcd_sed->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
}

void US_VelocModelControl::sed_dk_green()
{
	lcd_sed->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
}

void US_VelocModelControl::diff_red()
{
	lcd_diff->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
}

void US_VelocModelControl::diff_green()
{
	lcd_diff->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
}

void US_VelocModelControl::diff_dk_green()
{
	lcd_diff->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
}

void US_VelocModelControl::conc_red()
{
	lcd_conc->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
}

void US_VelocModelControl::conc_green()
{
	lcd_conc->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
}

void US_VelocModelControl::conc_dk_green()
{
	lcd_conc->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
}

void US_VelocModelControl::sigma_red()
{
	lcd_sigma->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
}

void US_VelocModelControl::sigma_green()
{
	lcd_sigma->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
}

void US_VelocModelControl::sigma_dk_green()
{
	lcd_sigma->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
}

void US_VelocModelControl::delta_red()
{
	lcd_delta->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
}

void US_VelocModelControl::delta_green()
{
	lcd_delta->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
}

void US_VelocModelControl::delta_dk_green()
{
	lcd_delta->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
}

void US_VelocModelControl::quit()
{
	close();
}




