#include "../include/us_combine.h"
#include <algorithm>
#ifndef WIN32
#include <unistd.h>
#endif

US_Combine::US_Combine(int tmp_prog_flag, QWidget *p, const char *name) : QFrame( p, name)
{
	USglobal = new US_Config();
	prog_flag = tmp_prog_flag;  // prog_flag:   0 = vHW s-val distributions
	                            //              1 = vHW MW distributions
	                            //              2 = C(s) distribution
	                            //              3 = C(MW) distribution
	                            //              4 = C(D) distribution
	int minHeight1=26, minHeight2=30;
	ff0 = (float) 1.2;
	vbar = (float) 0.72;
	min_s = 1.0e10;
	max_s = -1.0e-10;
	curve = new uint [20];
	symbol = new QwtSymbol [20];
	distribName = "distribution";
	analysis_type = "sa2d";
	pm = new US_Pixmap();
	distro.clear();
	if (prog_flag == 0 || prog_flag == 1)
	{
		distro_type = INTEGRAL;
	}
	else
	{
		distro_type = ENVELOPE;
	}
	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

	set_symbol();
	
	pb_load = new QPushButton(tr("Load Data"), this);
	Q_CHECK_PTR(pb_load);
	pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load->setAutoDefault(false);
	pb_load->setMinimumHeight(minHeight1);
	pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load, SIGNAL(clicked()), SLOT(load_data()));

	pb_details = new QPushButton(tr("Run Details"), this);
	pb_details->setAutoDefault(false);
	Q_CHECK_PTR(pb_details);
	pb_details->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_details->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_details->setMinimumHeight(minHeight1);
	pb_details->setEnabled(false);
	connect(pb_details, SIGNAL(clicked()), SLOT(details()));

	pb_print = new QPushButton(tr("Print Data"), this);
	Q_CHECK_PTR(pb_print);
	pb_print->setAutoDefault(false);
	pb_print->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_print->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_print->setMinimumHeight(minHeight1);
	pb_print->setEnabled(false);
	connect(pb_print, SIGNAL(clicked()), SLOT(print()));
	
	pb_save = new QPushButton(tr("Save Data"), this);
	Q_CHECK_PTR(pb_save);
	pb_save->setAutoDefault(false);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_save->setMinimumHeight(minHeight1);
	pb_save->setEnabled(false);
	connect(pb_save, SIGNAL(clicked()), SLOT(save()));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setMinimumHeight(minHeight1);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_close);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setMinimumHeight(minHeight1);
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));
	
	pb_reset = new QPushButton(tr("Reset"), this);
	Q_CHECK_PTR(pb_reset);
	pb_reset->setAutoDefault(false);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_reset->setMinimumHeight(minHeight1);
	pb_reset->setEnabled(false);
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));
	
	if (prog_flag == 0 || prog_flag == 1) // vHW distributions
	{
		banner0 = new QLabel(tr("Select Distribution Type:"), this);
		banner0->setFrameStyle(QFrame::WinPanel|Raised);
		banner0->setAlignment(AlignCenter|AlignVCenter);
		banner0->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
		banner0->setMinimumHeight(minHeight2);
		banner0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

		cb_integral = new QCheckBox(tr("Integral"), this);
		cb_integral->setMinimumHeight(minHeight1);
		cb_integral->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		cb_integral->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
		cb_integral->setChecked(true);
		connect(cb_integral, SIGNAL(clicked()), SLOT(select_integral()));

		cb_envelope = new QCheckBox(tr("Envelope"), this);
		cb_envelope->setMinimumHeight(minHeight1);
		cb_envelope->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		cb_envelope->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
		cb_envelope->setChecked(false);
		connect(cb_envelope, SIGNAL(clicked()), SLOT(select_envelope()));
	}
	else
	{
		bg_method_selection = new QButtonGroup(2, Qt::Horizontal, "Please select a Method:", this);
		bg_method_selection->setExclusive(true);
		connect(bg_method_selection, SIGNAL(clicked(int)), this, SLOT(select_method(int)));
		
		b_sa2d = new QCheckBox(tr(" 2-D SA"), bg_method_selection);
		b_sa2d->setMinimumHeight(minHeight1);
		b_sa2d->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		b_sa2d->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

		b_sa2d_mc = new QCheckBox(tr(" 2-D SA, MC"), bg_method_selection);
		b_sa2d_mc->setMinimumHeight(minHeight1);
		b_sa2d_mc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		b_sa2d_mc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

		b_sa2d_mw = new QCheckBox(tr(" 2-D SA, MW Constraints"), bg_method_selection);
		b_sa2d_mw->setMinimumHeight(minHeight1);
		b_sa2d_mw->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		b_sa2d_mw->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

		b_sa2d_mw_mc = new QCheckBox(tr(" 2-D SA, MC, MW Constraints"), bg_method_selection);
		b_sa2d_mw_mc->setMinimumHeight(minHeight1);
		b_sa2d_mw_mc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		b_sa2d_mw_mc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

		b_ga = new QCheckBox(tr(" GA"), bg_method_selection);
		b_ga->setMinimumHeight(minHeight1);
		b_ga->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		b_ga->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

		b_ga_mc = new QCheckBox(tr(" GA, MC"), bg_method_selection);
		b_ga_mc->setMinimumHeight(minHeight1);
		b_ga_mc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		b_ga_mc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

		b_ga_mw = new QCheckBox(tr(" GA, MW Constraints"), bg_method_selection);
		b_ga_mw->setMinimumHeight(minHeight1);
		b_ga_mw->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		b_ga_mw->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

		b_ga_mw_mc = new QCheckBox(tr(" GA, MC, MW Constraints"), bg_method_selection);
		b_ga_mw_mc->setMinimumHeight(minHeight1);
		b_ga_mw_mc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		b_ga_mw_mc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

		b_fe = new QCheckBox(tr(" Finite Element"), bg_method_selection);
		b_fe->setMinimumHeight(minHeight1);
		b_fe->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		b_fe->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

		b_cofs = new QCheckBox(tr(" C(s) Analysis"), bg_method_selection);
		b_cofs->setMinimumHeight(minHeight1);
		b_cofs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		b_cofs->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

		bg_method_selection->setButton(0);
	}

	lbl_run_id1 = new QLabel(tr(" Run Id:"),this);
	lbl_run_id1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_run_id1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_run_id1->setMinimumHeight(minHeight1);
	lbl_run_id1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_run_id = new QLineEdit(this, "run_id");
	le_run_id->setMinimumHeight(minHeight1);
	le_run_id->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_run_id->setText(tr(" Run ID "));
	le_run_id->setReadOnly(true);
	le_run_id->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		
	lbl_temperature1 = new QLabel(tr(" Temperature:"),this);
	lbl_temperature1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_temperature1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_temperature1->setMinimumHeight(minHeight1);
	lbl_temperature1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_temperature2 = new QLabel(tr("not selected"),this);
	lbl_temperature2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_temperature2->setAlignment(AlignCenter|AlignVCenter);
	lbl_temperature2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_temperature2->setMinimumHeight(minHeight1);
	lbl_temperature2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_cell_info1 = new QLabel(tr(" Available Cells:"),this);
	lbl_cell_info1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_cell_info1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_cell_info1->setMinimumHeight(minHeight1);
	lbl_cell_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_cell_info2 = new QLabel(tr("not selected"),this);
	lbl_cell_info2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_cell_info2->setAlignment(AlignCenter|AlignVCenter);
	lbl_cell_info2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_cell_info2->setMinimumHeight(minHeight1);
	lbl_cell_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	
	lbl_distribName = new QLabel(tr(" Distribution Name:"),this);
	lbl_distribName->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_distribName->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_distribName->setMinimumHeight(minHeight1);
	lbl_distribName->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_distribName = new QLineEdit(this, "distribName");
	le_distribName->setMinimumHeight(minHeight1);
	le_distribName->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_distribName->setText(tr(" distribution"));
	le_distribName->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(le_distribName, SIGNAL(textChanged(const QString &)), 
				SLOT(update_distribName(const QString &)));	

	if (prog_flag == 1)
	{
		banner2 = new QLabel(tr("Please enter f/f0 and vbar for the S->MW\nconversion of the next distribution:"), this);
		banner2->setFrameStyle(QFrame::WinPanel|Raised);
		banner2->setAlignment(AlignCenter|AlignVCenter);
		banner2->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
		banner2->setMinimumHeight(minHeight2);
		banner2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

		lbl_ff0 = new QLabel(tr(" f/f0:"),this);
		lbl_ff0->setFrameStyle(QFrame::WinPanel|Sunken);
		lbl_ff0->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
		lbl_ff0->setMinimumHeight(minHeight1);
		lbl_ff0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

		ct_ff0 = new QwtCounter(this);
		ct_ff0->setRange(1.0, 10, 0.01);
		ct_ff0->setValue(1.2);
		ct_ff0->setNumButtons(3);
		ct_ff0->setMinimumHeight(minHeight1);
		ct_ff0->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		ct_ff0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		connect(ct_ff0, SIGNAL(valueChanged(double)), SLOT(update_ff0(double)));

		pb_vbar = new QPushButton(tr(" vbar(20º):"), this);
		Q_CHECK_PTR(pb_vbar);
		pb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
		pb_vbar->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
		pb_vbar->setMinimumHeight(minHeight1);
		pb_vbar->setAutoDefault(false);
		connect(pb_vbar, SIGNAL(clicked()), SLOT(read_vbar()));

		le_vbar = new QLineEdit(this, "vbar");
		le_vbar->setMinimumHeight(minHeight1);
		le_vbar->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		le_vbar->setText(tr(" 0.72"));
		le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		connect(le_vbar, SIGNAL(textChanged(const QString &)), 
					SLOT(update_vbar(const QString &)));	
	}

	banner3 = new QLabel(tr("Double-click on Cell to select Distribution:"),this);
	banner3->setFrameStyle(QFrame::WinPanel|Raised);
	banner3->setAlignment(AlignCenter|AlignVCenter);
	banner3->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	banner3->setMinimumHeight(minHeight2);
	banner3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_cell_descr = new QTextEdit(this);
	lbl_cell_descr->setMaximumHeight(minHeight1 * 2);
	lbl_cell_descr->setWordWrap(QTextEdit::WidgetWidth);
	lbl_cell_descr->setText("no data available");
	lbl_cell_descr->setReadOnly(true);	
	lbl_cell_descr->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_cell_descr->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_cell_descr->setAlignment(AlignCenter|AlignVCenter);
	lbl_cell_descr->setMinimumHeight(minHeight1);
	lbl_cell_descr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	
	selected_cell = 0;
	cell_select = new QListBox(this, "Cell");
	cell_select->setMinimumHeight(minHeight1);
	//cell_select->setMaximumHeight(minHeight1 * 3);
	cell_select->insertItem(tr("Cell 1"));
	cell_select->insertItem(tr("Cell 2"));
	cell_select->insertItem(tr("Cell 3"));
	cell_select->insertItem(tr("Cell 4"));
	cell_select->insertItem(tr("Cell 5"));
	cell_select->insertItem(tr("Cell 6"));
	cell_select->insertItem(tr("Cell 7"));
	cell_select->insertItem(tr("Cell 8"));
	cell_select->setSelected(0, TRUE);
	cell_select->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	cell_select->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cell_select, SIGNAL(highlighted(int)), SLOT(show_cell(int)));
	connect(cell_select, SIGNAL(selected(int)), SLOT(add_distribution(int)));

	selected_lambda = 0;
	lambda_select = new QListBox(this, "Wavelength");
	lambda_select->setMinimumHeight(minHeight1);
	//lambda_select->setMaximumHeight(minHeight1 * 3);
	lambda_select->insertItem(tr("Wavelength 1"));
	lambda_select->insertItem(tr("Wavelength 2"));
	lambda_select->insertItem(tr("Wavelength 3"));
	lambda_select->setSelected(0, TRUE);
	lambda_select->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lambda_select->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(lambda_select, SIGNAL(highlighted(int)), SLOT(show_lambda(int)));

	pb_zoom_in = new QPushButton(tr("Zoom in"), this);
	Q_CHECK_PTR(pb_zoom_in);
	pb_zoom_in->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_zoom_in->setAutoDefault(false);
	pb_zoom_in->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_zoom_in->setMinimumHeight(minHeight1);
	pb_zoom_in->setEnabled(false);
	connect(pb_zoom_in, SIGNAL(clicked()), SLOT(zoom_in()));

	pb_zoom_out = new QPushButton(tr("Zoom out"), this);
	Q_CHECK_PTR(pb_zoom_out);
	pb_zoom_out->setAutoDefault(false);
	pb_zoom_out->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_zoom_out->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_zoom_out->setMinimumHeight(minHeight1);
	pb_zoom_out->setEnabled(false);
	connect(pb_zoom_out, SIGNAL(clicked()), SLOT(zoom_out()));

	analysis_plot = new QwtPlot(this);
	Q_CHECK_PTR(analysis_plot);
	analysis_plot->enableGridXMin();
	analysis_plot->enableGridYMin();
	analysis_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	analysis_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	analysis_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	analysis_plot->setCanvasBackground(USglobal->global_colors.plot);
	analysis_plot->setMargin(USglobal->config_list.margin);
	analysis_plot->enableAxis(QwtPlot::yRight);
	analysis_plot->enableAxis(QwtPlot::yLeft);
	analysis_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	analysis_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	analysis_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	analysis_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	analysis_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	analysis_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	analysis_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	analysis_plot->enableOutline(false);
	
	connect(analysis_plot, SIGNAL(plotMousePressed(const QMouseEvent &)), SLOT(mousePressed(const QMouseEvent&)));

	lbl_Xmin = new QLabel(tr(" X-min:"),this);
	lbl_Xmin->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_Xmin->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_Xmin->setMinimumHeight(minHeight1);
	lbl_Xmin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	ct_Xmin = new QwtCounter(this);
	ct_Xmin->setValue(0.0);
	ct_Xmin->setEnabled(false);
	ct_Xmin->setNumButtons(3);
	ct_Xmin->setMinimumHeight(minHeight1);
	ct_Xmin->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	ct_Xmin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(ct_Xmin, SIGNAL(valueChanged(double)), SLOT(update_Xmin(double)));

	lbl_Xmax = new QLabel(tr(" X-max:"),this);
	lbl_Xmax->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_Xmax->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_Xmax->setMinimumHeight(minHeight1);
	lbl_Xmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	ct_Xmax = new QwtCounter(this);
	ct_Xmax->setValue(0.0);
	ct_Xmax->setEnabled(false);
	ct_Xmax->setNumButtons(3);
	ct_Xmax->setMinimumHeight(minHeight1);
	ct_Xmax->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	ct_Xmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//	connect(ct_Xmax, SIGNAL(buttonReleased(double)), SLOT(update_Xmax(double)));
	connect(ct_Xmax, SIGNAL(valueChanged(double)), SLOT(update_Xmax(double)));

	global_Xpos += 30;
	global_Ypos += 30;
	
	setup_GUI(prog_flag);
	
	cp_list.clear();
	rotor_list.clear();
	if (!readCenterpieceInfo(&cp_list))
	{
		QMessageBox::critical(0, "UltraScan Fatal Error:", "There was a problem opening the\n"
									"centerpiece database file:\n\n"
									+ USglobal->config_list.system_dir + "/etc/centerpiece.dat\n\n"
									"Please install the centerpiece database file\n"
									"before proceeding.", QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		exit(-1);
	}
	if (!readRotorInfo(&rotor_list))
	{
		QMessageBox::critical(0, "UltraScan Fatal Error:", "There was a problem opening the\n"
									"rotor database file:\n\n"
									+ USglobal->config_list.system_dir + "/etc/rotor.dat\n\n"
									"Please install the rotor database file\n"
									"before proceeding.", QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		exit(-1);
	}
	set_colors();
}

US_Combine::~US_Combine()
{
	delete analysis_plot;
	delete curve;
}

void US_Combine::setup_GUI(int prog_flag)
{
	int j=0;
	int rows = 13, columns = 3, margin = 2, spacing = 2;
	
	QGridLayout * grid = new QGridLayout(this, 1, columns, margin, spacing);
	for (int i=0; i<rows; i++)
	{
		grid->setRowSpacing(i, 26);
	}
	grid->setColSpacing(2, 650);
	grid->addWidget(pb_load, j, 0);
	grid->addWidget(pb_details, j, 1);
	j++;
	grid->addWidget(pb_print, j, 0);
	grid->addWidget(pb_save, j, 1);
	j++;
	grid->addWidget(pb_help, j, 0);
	grid->addWidget(pb_close, j, 1);
	j++;
	grid->addMultiCellWidget(pb_reset, j, j, 0, 1);
	j++;
	if (prog_flag == 0 || prog_flag == 1) // vHW distributions
	{
		grid->addMultiCellWidget(banner0, j, j, 0, 1);
		j++;
		grid->addWidget(cb_integral, j, 0);
		grid->addWidget(cb_envelope, j, 1);
		j++;
	}
	else // finite element distros
	{
		grid->addMultiCellWidget(bg_method_selection, j, j, 0, 1);
		j++;
	}
	grid->addWidget(lbl_run_id1, j, 0);
	grid->addWidget(le_run_id, j, 1);
	j++;
	grid->addWidget(lbl_temperature1, j, 0);
	grid->addWidget(lbl_temperature2, j, 1);
	j++;
	grid->addWidget(lbl_cell_info1, j, 0);
	grid->addWidget(lbl_cell_info2, j, 1);
	j++;
	grid->addWidget(lbl_distribName, j, 0);
	grid->addWidget(le_distribName, j, 1);
	j++;
	if (prog_flag == 1)
	{ 
		grid->addMultiCellWidget(banner2, j, j+1, 0, 1);
		j += 2;
		grid->addWidget(lbl_ff0, j, 0);
		grid->addWidget(ct_ff0, j, 1);
		j++;
		grid->addWidget(pb_vbar, j, 0);
		grid->addWidget(le_vbar, j, 1);
		j++;
	}
	grid->addMultiCellWidget(banner3, j, j, 0, 1);
	j++;
	grid->addMultiCellWidget(lbl_cell_descr, j, j, 0, 1);
	j++;
	grid->addWidget(cell_select, j, 0);
	grid->addWidget(lambda_select, j, 1);
	j++;
	grid->addWidget(pb_zoom_in, j, 0);
	grid->addWidget(pb_zoom_out, j, 1);
	j++;
	grid->addWidget(lbl_Xmin, j, 0);
	grid->addWidget(ct_Xmin, j, 1);
	j++;
	grid->addWidget(lbl_Xmax, j, 0);
	grid->addWidget(ct_Xmax, j, 1);
	grid->addMultiCellWidget(analysis_plot, 0, j, 2, 2);
	grid->setColStretch(0, 0);
	grid->setColStretch(1, 1);
	grid->setColStretch(2, 4);
	for (int i=0; i<=j; i++)
	{
		grid->setRowSpacing(i, 26);
	}
}

void US_Combine::set_colors()
{
	col[0].setRgb(255,0,0);
	col[1].setRgb(0,255,0);
	col[2].setRgb(0,0,255);
	col[3].setRgb(255,255,0);
	col[4].setRgb(255,0,255);
	col[5].setRgb(0,255,255);
	col[6].setRgb(122,0,255);
	col[7].setRgb(255,0,122);
	col[8].setRgb(0,255,122);
	col[9].setRgb(0,122,255);
	col[10].setRgb(255,122,0);
	col[11].setRgb(122,255,0);
	col[12].setRgb(80,0,255);
	col[13].setRgb(255,0,80);
	col[14].setRgb(0,255,80);
	col[15].setRgb(0,80,255);
	col[16].setRgb(80,255,0);
	col[17].setRgb(255,80,40);
	col[18].setRgb(40,255,40);
	col[19].setRgb(40,40,255);
}

void US_Combine::select_method(int id)
{
	switch(id)
	{
		case 0:
		{
			analysis_type = "sa2d";
			break;
		}
		case 1:
		{
			analysis_type = "sa2d_mc";
			break;
		}
		case 2:
		{
			analysis_type = "sa2d_mw";
			break;
		}
		case 3:
		{
			analysis_type = "sa2d_mw_mc";
			break;
		}
		case 4:
		{
			analysis_type = "ga";
			break;
		}
		case 5:
		{
			analysis_type = "ga_mc";
			break;
		}
		case 6:
		{
			analysis_type = "ga_mw";
			break;
		}
		case 7:
		{
			analysis_type = "ga_mw_mc";
			break;
		}
		case 8:
		{
			analysis_type = "fe";
			break;
		}
		case 9:
		{
			analysis_type = "cofs";
			break;
		}
	}
}

void US_Combine::select_integral()
{
	distro_type = INTEGRAL;
	cb_envelope->setChecked(false);
	cb_integral->setChecked(true);
}

void US_Combine::select_envelope()
{
	distro_type = ENVELOPE;
	cb_envelope->setChecked(true);
	cb_integral->setChecked(false);
}

void US_Combine::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}
/*
void US_Combine::resizeEvent(QResizeEvent *e)
{
	int dialogw = 2 * buttonw + spacing + 2 * border;
	analysis_plot->setGeometry(dialogw, spacing, e->size().width()-dialogw-2, e->size().height() - 4);
}
*/
void US_Combine::show_cell(int val)
{
	selected_cell = val;
	if(run_inf.cell_id[val] != "" 
   && run_inf.wavelength[selected_cell][selected_lambda] != 0)
	{
		lbl_cell_descr->setText(run_inf.cell_id[val]);
	}
	else
	{
		lbl_cell_descr->setText(tr(" no data available"));
	}
}

void US_Combine::set_symbol()
{
	for (i=0; i<20; i++)
	{
		symbol[i].setPen(white);
		symbol[i].setSize(8);
		switch(i)
		{
			case 0:
			{
				symbol[i].setStyle(QwtSymbol::Ellipse);
				symbol[i].setBrush(col[i]);
				break;
			} 
			case 1:
			{
				symbol[i].setStyle(QwtSymbol::Rect);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 2:
			{
				symbol[i].setStyle(QwtSymbol::Triangle);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 3:
			{
				symbol[i].setStyle(QwtSymbol::Diamond);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 4:
			{
				symbol[i].setStyle(QwtSymbol::Ellipse);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 5:
			{
				symbol[i].setStyle(QwtSymbol::UTriangle);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 6:
			{
				symbol[i].setStyle(QwtSymbol::LTriangle);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 7:
			{
				symbol[i].setStyle(QwtSymbol::RTriangle);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 8:
			{
				symbol[i].setStyle(QwtSymbol::Cross);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 9:
			{
				symbol[i].setStyle(QwtSymbol::DTriangle);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 10:
			{
				symbol[i].setStyle(QwtSymbol::Ellipse);
				symbol[i].setBrush(col[i]);
				break;
			} 
			case 11:
			{
				symbol[i].setStyle(QwtSymbol::Rect);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 12:
			{
				symbol[i].setStyle(QwtSymbol::Triangle);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 13:
			{
				symbol[i].setStyle(QwtSymbol::Diamond);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 14:
			{
				symbol[i].setStyle(QwtSymbol::Ellipse);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 15:
			{
				symbol[i].setStyle(QwtSymbol::UTriangle);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 16:
			{
				symbol[i].setStyle(QwtSymbol::LTriangle);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 17:
			{
				symbol[i].setStyle(QwtSymbol::RTriangle);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 18:
			{
				symbol[i].setStyle(QwtSymbol::Cross);
				symbol[i].setBrush(col[i]);
				break;
			}
			case 19:
			{
				symbol[i].setStyle(QwtSymbol::DTriangle);
				symbol[i].setBrush(col[i]);
				break;
			}
		}
	}
}

void US_Combine::show_lambda(int val)
{
	selected_lambda = val;
	if(run_inf.cell_id[val] != "" && run_inf.wavelength[selected_cell][selected_lambda] != 0)
	{
		lbl_cell_descr->setText(run_inf.cell_id[selected_cell]);
	}
	else
	{
		lbl_cell_descr->setText(tr(" no data available"));
	}
}

void US_Combine::load_data()
{
	QString version;
	fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.us.v", 0);
	int et;
	if ( !fn.isEmpty() ) 
	{
		QFile f(fn);
		f.open(IO_ReadOnly);
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
			return;
		}
		ts >> run_inf.data_dir;
//cout << "data_dir: " << run_inf.data_dir.latin1() << endl;
		ts >> run_inf.run_id;
		distribName = run_inf.run_id;
		le_distribName->setText(distribName);
//cout << "Run ID: " << run_inf.run_id.latin1() << endl;
		ts >> run_inf.avg_temperature;
//cout << "Avg. temp: " << run_inf.avg_temperature << endl;
		ts >> run_inf.temperature_check;
//cout << "temp check: " << run_inf.temperature_check << endl;
		ts >> run_inf.time_correction;
//cout << "Time correction: " << run_inf.time_correction << endl;
		ts >> run_inf.duration;
//cout << "duration: " << run_inf.duration << endl;
		ts >> run_inf.total_scans;
//cout << "Total Scans: " << run_inf.total_scans << endl;
		ts >> run_inf.delta_r;
//cout << "Delta-r: " << run_inf.delta_r << endl;
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
			ts >> run_inf.meniscus[i];
			ts >> run_inf.cell_id[i];
			ts >> run_inf.wavelength_count[i];
		}

		for (i=0; i<8; i++)
		{
			if(version.toFloat() < 7.0)
			{
				for(int j=0; j<4; j++)
				{
					ts >> run_inf.DNA_serialnumber[i][j][0];	
					ts >> run_inf.buffer_serialnumber[i][j];		
					ts >> run_inf.peptide_serialnumber[i][j][0];
				}
			}
			else
			{
				for(int j=0; j<4; j++)
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
				ts >> run_inf.baseline[i][j];
				if (run_inf.centerpiece[i] >= 0)
				{
					for (k=0; k<cp_list[run_inf.centerpiece[i]].channels; k++)
					{
						ts >> run_inf.range_left[i][j][k];
						ts >> run_inf.range_right[i][j][k];
						ts >> run_inf.points[i][j][k];
						ts >> run_inf.point_density[i][j][k];
					}
				}
			}
		}
		run_inf.temperature = new float** [8];
		run_inf.rpm = new unsigned int** [8];
		run_inf.time		= new unsigned int** [8];
		run_inf.omega_s_t   = new float** [8];
		run_inf.plateau	 = new float** [8];
		for (i=0; i<8; i++)
		{
			run_inf.temperature[i] = new float* [run_inf.wavelength_count[i]];
			run_inf.rpm[i] = new unsigned int* [run_inf.wavelength_count[i]];
			run_inf.time[i]		= new unsigned int* [run_inf.wavelength_count[i]];
			run_inf.omega_s_t[i]   = new float* [run_inf.wavelength_count[i]];
			run_inf.plateau[i]	 = new float* [run_inf.wavelength_count[i]];
			for (j=0; j<run_inf.wavelength_count[i]; j++)
			{
				run_inf.temperature[i][j] = new float [run_inf.scans[i][j]];
				run_inf.rpm[i][j] = new unsigned int [run_inf.scans[i][j]];
				run_inf.time[i][j]		= new unsigned int [run_inf.scans[i][j]];
				run_inf.omega_s_t[i][j]   = new float [run_inf.scans[i][j]];
				run_inf.plateau[i][j]	 = new float [run_inf.scans[i][j]];
			}
		}
		for (i=0; i<8; i++)
		{
			for (j=0; j<run_inf.wavelength_count[i]; j++)
			{
				for (k=0; k<run_inf.scans[i][j]; k++)
				{
					ts >> run_inf.rpm[i][j][k];					
					ts >> run_inf.temperature[i][j][k];
					ts >> run_inf.time[i][j][k];
					run_inf.time[i][j][k] -= (long) (run_inf.time_correction + 0.5);
					ts >> run_inf.omega_s_t[i][j][k];
					ts >> run_inf.plateau[i][j][k];
					run_inf.plateau[i][j][k] -= run_inf.baseline[i][j];
				}
			}
		}
		ts >> run_inf.rotor;
		f.close();
		update_screen();
	}
}

void US_Combine::add_distribution(int val)
{
	struct distribution temp_distro;
	double temp, temp2;
	temp_distro.sval.clear();
	temp_distro.fraction.clear();
	unsigned int i;
	selected_cell = val;
	if(!(run_inf.cell_id[val] != "" 
	&& run_inf.wavelength[selected_cell][selected_lambda] != 0))
	{
		QString str1;
		str1.sprintf(tr("There is no data available for Cell %d, Wavelength %d\n\n"
						 "Please select another cell or wavelength."), 
					 	  selected_cell+1, selected_lambda+1);
		QMessageBox::message(tr("Attention:"), str1);
		return;
	}
	if (distro.size() == 20)
	{
		QMessageBox::message(tr("Attention:"), tr("Sorry, 20 distributions max!"));
		return;
	}
	float trashcan;
	QString filename, str;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	if (prog_flag == 0 || prog_flag == 1)
	{
		switch (distro_type)
		{
			case ENVELOPE:
			{
				str.sprintf(".vhw_his.%d%d",selected_cell+1, selected_lambda+1);
				break;
			}
			case INTEGRAL:
			{
				str.sprintf(".vhw_dis.%d%d",selected_cell+1, selected_lambda+1);
				break;
			}
		}
	}
	else
	{
		str.sprintf("." + analysis_type + "_dis.%d%d",selected_cell+1, selected_lambda+1);
	}
	filename.append(str);
	QFile f(filename);
	if(f.open(IO_ReadOnly))
	{
		QTextStream s( &f );
		if (!s.eof())
		{
			s.readLine();
		}
		while (!s.eof())
		{
			switch (prog_flag)
			{
				case 0:
				{
					switch (distro_type)
					{
						case ENVELOPE:
						{
							s >> temp;
							s >> temp2;
							break;
						}
						case INTEGRAL:
						{
							s >> temp2;
							s >> trashcan;
							s >> trashcan;
							s >> temp;
							break;
						}
					}
					break;
				}
				case 1:
				{
					switch (distro_type)
					{
						case ENVELOPE:
						{
							s >> temp;
							s >> temp2;
							break;
						}
						case INTEGRAL:
						{
							s >> temp2;
							s >> trashcan;
							s >> trashcan;
							s >> temp;
							break;
						}
					}
					break;
				}
				case 2:
				{
					s >> trashcan;
					s >> temp;
					s >> trashcan;
					s >> trashcan;
					s >> trashcan;
					s >> temp2;
					break;
				}
				case 3:
				{
					s >> trashcan;
					s >> trashcan;
					s >> trashcan;
					s >> trashcan;
					s >> temp;
					s >> temp2;
					break;
				}
				case 4:
				{
					s >> trashcan;
					s >> trashcan;
					s >> trashcan;
					s >> temp;
					s >> trashcan;
					s >> temp2;
					break;
				}
			}
			if (prog_flag == 1)
			{
				temp = convert(temp); //convert s value to MW
			}
// if we are plotting a monte carlo analysis we need to combine duplicate signals:

			if (analysis_type == "ga_mc" || analysis_type == "sa2d_mc")
			{
				bool flag = true;
				for (unsigned int j=0; j<temp_distro.sval.size(); j++)
				{
					if (temp == temp_distro.sval[j])
					{
						temp_distro.fraction[j] += temp2;
						flag = false;
					}
				}
				if (flag) // if this is a new solute we need to add it
				{
				cout << temp << ", " <<temp2 << endl;
					temp_distro.sval.push_back(temp);
					temp_distro.fraction.push_back(temp2);
				}
			}
			else
			{
				temp_distro.sval.push_back(temp);
				temp_distro.fraction.push_back(temp2);
			}
			s.readLine();
		}
		if (prog_flag == 1)
		{
			temp_distro.id = run_inf.cell_id[selected_cell] + str.sprintf(", vbar: %5.3f, f/f0: %5.3f", vbar, ff0);
		}
		else
		{
			temp_distro.id = run_inf.cell_id[selected_cell];
		}
		f.close();
		distro.push_back(temp_distro);
		if (distro_type == INTEGRAL)
		{
			is_integral.push_back(true);
		}
		else
		{
			is_integral.push_back(false);
		}
		max_s = -100.0;
		min_s = 100.0;
		for (i=0; i<temp_distro.sval.size(); i++)
		{
			max_s = max (max_s, temp_distro.sval[i]);
			min_s = min (min_s, temp_distro.sval[i]);
		}
//cout << "Max: " << max_s << ", min: " << min_s << endl;
		calc_limits();
//cout << "Max: " << max_s << ", min: " << min_s << endl;
		plotDistros();
		enable_all();
	}
	else
	{
		str = tr("The distribution for this cell has not yet been\n"
				"calculated or has not been saved.\n\n"
				"Please perform the analysis before including this\n"
				"distribution in the combined distribution plot.");
		QMessageBox::message(tr("Attention:"), str);
	}
}

void US_Combine::plotDistros()
{
	set_symbol(); // reset symbol to appropriate colors
	analysis_plot->clear();
	double *fraction, *sval;
	unsigned int count, i, j;
	for (i=0; i<distro.size(); i++)
	{
		count = distro[i].sval.size();
		fraction = new double [count];
		sval = new double [count];
		for (j=0; j<count; j++)
		{
			sval[j] = distro[i].sval[j];
			fraction[j] = distro[i].fraction[j];
		}
		curve[i] = analysis_plot->insertCurve(distro[i].id);
		analysis_plot->setCurvePen(curve[i], QPen(col[i], 3, SolidLine));
		if(prog_flag >=2 && prog_flag <=4)
		{
			analysis_plot->setCurveStyle(curve[i], QwtCurve::Sticks);
			analysis_plot->setCurvePen(curve[i], QPen(col[i], 6, SolidLine));
		}
		else
		{
			analysis_plot->setCurveStyle(curve[i], QwtCurve::Lines);
		}
		if (is_integral[i])
		{
			analysis_plot->setCurvePen(curve[i], yellow);
			analysis_plot->setCurveSymbol(curve[i], symbol[i]);
		}
		analysis_plot->setCurveData(curve[i], sval, fraction, count);
		if (is_integral[i])
		{
			analysis_plot->setCurveYAxis(curve[i], QwtPlot::yLeft);
		}
		else
		{
			analysis_plot->setCurveYAxis(curve[i], QwtPlot::yRight);
		}
		delete [] fraction;
		delete [] sval;
	}
	analysis_plot->enableLegend(TRUE);
	analysis_plot->setLegendPos(Qwt::Bottom);
	analysis_plot->setLegendFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	switch (prog_flag)
	{
		case 0:
		{
			analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Sedimentation Coefficient corrected for water at 20ºC"));
			if (distro_type == ENVELOPE)
			{
				analysis_plot->setTitle(tr("s20,W distributions:"));
				analysis_plot->setAxisTitle(QwtPlot::yRight, tr("Relative Frequency"));
			}
			else if (distro_type == INTEGRAL)
			{
				analysis_plot->setTitle(tr("G(s) distributions:"));
				analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Boundary Fraction (in %)"));
			}
			break;
		}
		case 1:
		{
			analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Molecular Weight Distribution"));
			if (distro_type == ENVELOPE)
			{
				analysis_plot->setTitle(tr("MW distributions:"));
				analysis_plot->setAxisTitle(QwtPlot::yRight, tr("Relative Frequency"));
			}
			else if (distro_type == INTEGRAL)
			{
				analysis_plot->setTitle(tr("G(MW) distributions:"));
				analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Boundary Fraction (in %)"));
			}
			break;
		}
		case 2:
		{
			analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Sedimentation Coefficient corrected for water at 20ºC"));
			analysis_plot->setTitle(tr("Discrete s20,W distributions:"));
			analysis_plot->setAxisTitle(QwtPlot::yRight, tr("Relative Frequency"));
			break;
		}
		case 3:
		{
			analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Molecular Weight Distribution"));
			analysis_plot->setTitle(tr("Discrete MW distributions:"));
			analysis_plot->setAxisTitle(QwtPlot::yRight, tr("Relative Frequency"));
			break;
		}
		case 4:
		{
			analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Diffusion Coefficient corrected for water at 20ºC"));
			analysis_plot->setAxisTitle(QwtPlot::yRight, tr("Relative Frequency"));
			analysis_plot->setTitle(tr("Discrete D20,W distributions:"));
			break;
		}
	}
	analysis_plot->setAxisAutoScale(QwtPlot::xBottom);
	analysis_plot->replot();
}

void US_Combine::calc_limits()
{

//cout << "Max: " << max_s << ", min: " << min_s << ", case: " << prog_flag << endl;
	ct_Xmin->setEnabled(true);
	ct_Xmax->setEnabled(true);
	disconnect(ct_Xmin, SIGNAL(valueChanged(double)), 0, 0);
	disconnect(ct_Xmax, SIGNAL(valueChanged(double)), 0, 0);
	switch (prog_flag)
	{
		case 0:
		{
			ct_Xmin->setRange(-1.0e5, max_s, 0.1);
			break;
		}
		case 1:
		{
			ct_Xmin->setRange(0, max_s, 100);
			break;
		}
		case 2:
		{
			ct_Xmin->setRange(-1e-5, max_s, 1.0e-15);
			break;
		}
		case 3:
		{
			ct_Xmin->setRange(0, max_s, 100);
			break;
		}
		case 4:
		{
			ct_Xmin->setRange(0, max_s, 1.0e-9);
			break;
		}
	}
	ct_Xmin->setValue(min_s);

	switch (prog_flag)
	{
		case 0:
		{
			ct_Xmax->setRange(min_s, 1.0e5, 0.1);
			break;
		}
		case 1:
		{
			ct_Xmax->setRange(min_s, 1.0e10, 100);
			break;
		}
		case 2:
		{
			ct_Xmax->setRange(min_s, 1.0e-7, 1.0e-15);
			break;
		}
		case 3:
		{
			ct_Xmax->setRange(min_s, 1.0e10, 100);
			break;
		}
		case 4:
		{
			ct_Xmax->setRange(min_s, 1.0e-5, 1.0e-9);
			break;
		}
	}
	ct_Xmax->setValue(max_s);
	connect(ct_Xmin, SIGNAL(valueChanged(double)), SLOT(update_Xmin(double)));
	connect(ct_Xmax, SIGNAL(valueChanged(double)), SLOT(update_Xmax(double)));
}

void US_Combine::disable_all()
{
	pb_details->setEnabled(false);
	pb_print->setEnabled(false);
	pb_save->setEnabled(false);
	pb_zoom_in->setEnabled(false);
	pb_zoom_out->setEnabled(false);
	pb_reset->setEnabled(false);
}

void US_Combine::enable_all()
{
	pb_details->setEnabled(true);
	pb_print->setEnabled(true);
	pb_save->setEnabled(true);
	pb_zoom_in->setEnabled(true);
	pb_zoom_out->setEnabled(true);
	pb_reset->setEnabled(true);
}

void US_Combine::mousePressed(const QMouseEvent &e)
{
	QFile f;
	QwtSymbol sym;
	QString str1;
	long int index;
	int dist;
	index = analysis_plot->closestCurve(e.x(), e.y(), dist);
	sym.setPen(white);
	sym.setBrush(white);
	sym.setSize(8);
	sym.setStyle(symbol[index-1].style());
	if (index == 0)
	{
		return;
	}
	analysis_plot->setCurvePen(index, QPen(Qt::white, 3, SolidLine));
	if (distro_type == INTEGRAL)
	{
		analysis_plot->setCurveSymbol(index, sym);
	}
	analysis_plot->replot();
	//analysis_plot->updatePlot(); 	 //no updatePlot() function in new version
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
			vector <struct distribution>::iterator distro_iter = distro.begin() + index - 1;
			vector <bool>::iterator integral_iter = is_integral.begin() + index - 1;
			analysis_plot->removeCurve(index);
			distro.erase(distro_iter);
			is_integral.erase(integral_iter);
			max_s = -1.0e10;
			min_s = 1.0e10;
			for (unsigned int j=0; j<distro.size(); j++)
			{
				for (i=0; i<distro[j].sval.size(); i++)
				{
					max_s = max (max_s, distro[j].sval[i]);
					min_s = min (min_s, distro[j].sval[i]);
				}
			}
			if (distro.size() == 0)
			{
				disable_all();
			}
			/*
			temp = max_s - min_s;
			max_s = max_s + temp/20;
			min_s = min_s - temp/20;
			orig_max_s = max_s;
			orig_min_s = min_s;
			*/
			calc_limits();
			break;
		}
	}
	plotDistros();
}

void US_Combine::details()
{
	run_details = new RunDetails_F(1, 1, &run_inf);
	run_details->setFixedSize(666,500);
	run_details->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	run_details->show();
}

void US_Combine::update_screen()
{
	QString str, str1, str2;
	le_run_id->setText(run_inf.run_id);
	lbl_temperature2->setText(str1.sprintf(" %5.3f ºC", run_inf.avg_temperature));
	int i;
	str = "";
	for (i=0; i<8; i++)
	{
		has_data[i]=FALSE;
		if (run_inf.scans[i][0] != 0)
		{
			has_data[i]=TRUE;
			if (str == "")
			{ 
				str.sprintf("%d", i+1);
			}
			else
			{
				str2.sprintf(", %d", i+1);
				str.append(str2);
			}
		}
		lbl_cell_info2->setText(str);
	}
	i=0;
	while (!has_data[i])	//let's find the first cell with data in it
	{
		i++;
	}
	first_cell = i;
	lbl_cell_descr->setText(run_inf.cell_id[first_cell]);
	cell_select->setCurrentItem(first_cell);
	pb_details->setEnabled(true);
}

void US_Combine::quit()
{
	close();
}

void US_Combine::reset()
{
	analysis_plot->clear();
	analysis_plot->replot();
	distro.clear();
	is_integral.clear();
	le_run_id->setText(tr("not selected"));
	lbl_cell_info2->setText(tr("not selected"));
	lbl_cell_descr->setText(tr("no data available"));
	lbl_temperature2->setText(tr("not selected"));
	disable_all();
}

void US_Combine::print()
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
			analysis_plot->print(printer, pf);
		}
		else if (print_inv)
		{
			PrintFilterDistro pf;
			pf.setOptions(QwtPlotPrintFilter::PrintTitle
							 |QwtPlotPrintFilter::PrintMargin
							 |QwtPlotPrintFilter::PrintLegend
							 |QwtPlotPrintFilter::PrintGrid);
			analysis_plot->print(printer, pf);
		}
		else
		{
			analysis_plot->print(printer);
		}
	}
}

void US_Combine::zoom_in()
{
	if (distro.size() == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load a distribution first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	double diff;
	diff = (max_s - min_s)/10;
	if (diff > 0)
	{
		max_s -= diff;
		min_s += diff;
	}
	calc_limits();
	analysis_plot->setAxisScale(QwtPlot::xBottom, min_s, max_s);
	analysis_plot->replot();	
}

void US_Combine::zoom_out()
{
	if (distro.size() == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load a distribution first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	double diff;
	diff = (max_s - min_s)/10;
	max_s += diff;
	min_s -= diff;
	calc_limits();
	analysis_plot->setAxisScale(QwtPlot::xBottom, min_s, max_s);
	analysis_plot->replot();
}

void US_Combine::update_Xmin(double val)
{
	min_s = val;
	calc_limits();
	analysis_plot->setAxisScale(QwtPlot::xBottom, min_s, max_s);
	analysis_plot->replot();
}

void US_Combine::update_Xmax(double val)
{
	max_s = val;
	calc_limits();
	analysis_plot->setAxisScale(QwtPlot::xBottom, min_s, max_s);
	analysis_plot->replot();
}

void US_Combine::update_ff0(double val)
{
	ff0 = val;
}

void US_Combine::update_distribName(const QString &str)
{
	distribName = str;
}

void US_Combine::update_vbar(const QString &str)
{
	vbar = str.toFloat();
}

void US_Combine::view()
{
}

void US_Combine::read_vbar()
{
	float v;
	US_Vbar_DB *vbar_dlg;
	vbar_dlg = new US_Vbar_DB(20.0, &v, &vbar, true, false, run_inf.investigator);
	vbar_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	vbar_dlg->setCaption(tr("V-bar Calculation"));
	vbar_dlg->show();
	connect(vbar_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_vbar_lbl(float, float)));
}

void US_Combine::update_vbar_lbl(float val, float val20)
{
	vbar = val;
	vbar = val20;
	QString str;
	str.sprintf("%6.4f", vbar);
	le_vbar->setText(str);
}

double US_Combine::convert(double val)
{
	double tmp1, tmp2, tmp;
	tmp1 = val * 1.0e-13 * AVOGADRO * ff0 * 6.0 * M_PI * VISC_20W;
	tmp2 = pow(((3.0 * vbar)/(4.0 * M_PI * AVOGADRO)), (1.0/3.0));
	tmp = pow(((tmp1 * tmp2)/(1.0 - vbar * DENS_20W)), 1.5);
	return(tmp);
}

void US_Combine::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	switch (prog_flag)
	{
		case 0:
		{
			online_help->show_help("manual/combine.html");
			break;
		}
		case 1:
		{
			online_help->show_help("manual/combine_mw.html");
			break;
		}
		case 2:
		{
			online_help->show_help("manual/combine_cofs.html");
			break;
		}
		case 3:
		{
			online_help->show_help("manual/combine_cofmw.html");
			break;
		}
		case 4:
		{
			online_help->show_help("manual/combine_cofd.html");
			break;
		}
	}
}

void US_Combine::save()
{
	uint i, j, maxcount=0;
	QString htmlDir, file1, file2, fileName;
	htmlDir = USglobal->config_list.html_dir + "/" + distribName;
	switch (prog_flag)
	{
		case 0:
		{
			file1 = (htmlDir + "/distrib.dat");
			file2 = (htmlDir + "/distrib.txt");
			fileName = (htmlDir + "/distribution.");
			break;
		}
		case 1:
		{
			file1 = (htmlDir + "/mw_distrib.dat");
			file2 = (htmlDir + "/mw_distrib.txt");
			fileName = (htmlDir + "/mw_distribution.");
			break;
		}
		case 2:
		{
			file1 = (htmlDir + "/" +analysis_type + "_s_distrib.dat");
			file2 = (htmlDir + "/" +analysis_type + "_s_distrib.txt");
			fileName = (htmlDir + "/" +analysis_type + "_s_distribution.");
			break;
		}
		case 3:
		{
			file1 = (htmlDir + "/" +analysis_type + "_mw_distrib.dat");
			file2 = (htmlDir + "/" +analysis_type + "_mw_distrib.txt");
			fileName = (htmlDir + "/" +analysis_type + "_mw_distribution.");
			break;
		}
		case 4:
		{
			file1 = (htmlDir + "/" +analysis_type + "_d_distrib.dat");
			file2 = (htmlDir + "/" +analysis_type + "_d_distrib.txt");
			fileName = (htmlDir + "/" +analysis_type + "_d_distribution.");
			break;
		}
	}
	QDir d(htmlDir);
	QPixmap p;
	if (!d.exists())
	{
		d.mkdir(htmlDir, true);
	}
	p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
	pm->save_file(fileName, p);

	for (i=0; i<distro.size(); i++)
	{
		maxcount = (unsigned int) max((double) maxcount, (double) distro[i].sval.size());
	}
	QFile distrib_f(file1);
	if (distrib_f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&distrib_f);
		ts.setf(QTextStream::scientific);
		for (i=0; i<maxcount; i++)
		{
			for (j=0; j<distro.size(); j++)
			{
				if(distro[j].sval.size() > i)
				{
					ts << distro[j].fraction[i] << "\t";
					ts << distro[j].sval[i] << "\t";
				}
				else //if there aren't that many divisions fill with blanks
				{
					ts << "            " << "\t";
					ts << "            " << "\t";
				}
			}
			ts << "\n";	//finish each division with a new line
		}
		distrib_f.close();
	}
	else
	{
		QMessageBox::message("Warning", "Could not open " + file1 + ".\nPlease chechk to make sure you have write permission");
	}
	QFile distrib2_f(file2);
	if (distrib2_f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&distrib2_f);
		for (i=0; i<distro.size(); i++)
		{
			ts << distro[i].id << "\n";
		}
		distrib2_f.close();
	}
	else
	{
		QMessageBox::message("Warning", "Could not open " + file2 + ".\nPlease chechk to make sure you have write permission");
	}
}

