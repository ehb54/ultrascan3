#include "../include/us_pseudo3d_combine.h"

US_Pseudo3D_Combine::US_Pseudo3D_Combine(QWidget *p, const char *name) : QFrame( p, name)
{
	USglobal=new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("Combine Pseudo-3D Distributions"));

	distro_type = -1;
	minmax = false;
	zoom = false;
	monte_carlo = false;
	autolimit = true;
	plot_s = true;
	current_distro = 0;
	system.clear();
	plot_fmin = 1.0;
	plot_fmax = 4.0;
	plot_smin = 1.0;
	plot_smax = 10.0;
	resolution = 90.0;
	global_Xpos += 30;
	global_Ypos += 30;
	x_pixel = 2;
	y_pixel = 2;
	x_resolution = 300;
	y_resolution = 300;
	setup_GUI();
	move(global_Xpos, global_Ypos);
	current_gradient.resize(510);
	unsigned int k, g, b;
	for (k=0; k<510; k++) // assign default gradient from black to bright cyan over 510 color points
	{
		if (k > 255)
		{
			g = (unsigned int) (k - 255);
			b = 255;
		}
		else
		{
			g = 0;
			b = k;
		}
		current_gradient[k].setRgb(0, g, b);
	}
}

US_Pseudo3D_Combine::~US_Pseudo3D_Combine()
{
}

void US_Pseudo3D_Combine::setup_GUI()
{
	unsigned int minHeight1=26;

	lbl_info1 = new QLabel(tr("Pseudo-3D Plotting Controls"), this);
	Q_CHECK_PTR(lbl_info1);
	lbl_info1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info1->setAlignment(AlignCenter|AlignVCenter);
	lbl_info1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_resolution = new QLabel(tr(" Pseudo-3D Resolution: "), this);
	Q_CHECK_PTR(lbl_resolution);
	lbl_resolution->setAlignment(AlignLeft|AlignVCenter);
	lbl_resolution->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_resolution->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_resolution= new QwtCounter(this);
	Q_CHECK_PTR(cnt_resolution);
	cnt_resolution->setRange(1, 100, 1);
	cnt_resolution->setValue(resolution);
	cnt_resolution->setEnabled(true);
	cnt_resolution->setNumButtons(3);
	cnt_resolution->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_resolution, SIGNAL(valueChanged(double)), SLOT(update_resolution(double)));

	lbl_x_resolution = new QLabel(tr(" X Resolution: "), this);
	Q_CHECK_PTR(lbl_x_resolution);
	lbl_x_resolution->setAlignment(AlignLeft|AlignVCenter);
	lbl_x_resolution->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_x_resolution->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_x_resolution= new QwtCounter(this);
	Q_CHECK_PTR(cnt_x_resolution);
	cnt_x_resolution->setRange(10, 1000, 1);
	cnt_x_resolution->setValue(x_resolution);
	cnt_x_resolution->setNumButtons(3);
	cnt_x_resolution->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_x_resolution, SIGNAL(valueChanged(double)), SLOT(update_x_resolution(double)));

	lbl_y_resolution = new QLabel(tr(" Y Resolution: "), this);
	Q_CHECK_PTR(lbl_y_resolution);
	lbl_y_resolution->setAlignment(AlignLeft|AlignVCenter);
	lbl_y_resolution->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_y_resolution->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_y_resolution= new QwtCounter(this);
	Q_CHECK_PTR(cnt_y_resolution);
	cnt_y_resolution->setRange(10, 1000, 1);
	cnt_y_resolution->setValue(y_resolution);
	cnt_y_resolution->setNumButtons(3);
	cnt_y_resolution->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_y_resolution, SIGNAL(valueChanged(double)), SLOT(update_y_resolution(double)));

	lbl_x_pixel = new QLabel(tr(" X-pixel width: "), this);
	Q_CHECK_PTR(lbl_x_pixel);
	lbl_x_pixel->setAlignment(AlignLeft|AlignVCenter);
	lbl_x_pixel->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_x_pixel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_x_pixel= new QwtCounter(this);
	Q_CHECK_PTR(cnt_x_pixel);
	cnt_x_pixel->setRange(2, 50, 1);
	cnt_x_pixel->setValue(x_pixel);
	cnt_x_pixel->setNumButtons(3);
	cnt_x_pixel->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_x_pixel, SIGNAL(valueChanged(double)), SLOT(update_x_pixel(double)));

	lbl_y_pixel = new QLabel(tr(" Y-pixel width: "), this);
	Q_CHECK_PTR(lbl_y_pixel);
	lbl_y_pixel->setAlignment(AlignLeft|AlignVCenter);
	lbl_y_pixel->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_y_pixel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_y_pixel= new QwtCounter(this);
	Q_CHECK_PTR(cnt_y_pixel);
	cnt_y_pixel->setRange(2, 50, 1);
	cnt_y_pixel->setValue(y_pixel);
	cnt_y_pixel->setNumButtons(3);
	cnt_y_pixel->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_y_pixel, SIGNAL(valueChanged(double)), SLOT(update_y_pixel(double)));

	lbl_autolimit = new QLabel(tr(" Automatic Plot Limits"), this);
	Q_CHECK_PTR(lbl_autolimit);
	lbl_autolimit->setAlignment(AlignLeft|AlignVCenter);
	lbl_autolimit->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_autolimit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_autolimit = new QCheckBox(tr("(unselect to override)"),this);
	cb_autolimit->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_autolimit->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_autolimit->setChecked(autolimit);
	cb_autolimit->setEnabled(true);
	cb_autolimit->setMinimumHeight(minHeight1);
	connect(cb_autolimit, SIGNAL(clicked()), SLOT(select_autolimit()));

	lbl_plot_fmin = new QLabel(tr(" Plot Limit f/f0 min.: "), this);
	Q_CHECK_PTR(lbl_plot_fmin);
	lbl_plot_fmin->setAlignment(AlignLeft|AlignVCenter);
	lbl_plot_fmin->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_plot_fmin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_plot_fmin= new QwtCounter(this);
	Q_CHECK_PTR(cnt_plot_fmin);
	cnt_plot_fmin->setRange(0, 50, 0.01);
	cnt_plot_fmin->setValue(plot_fmin);
	cnt_plot_fmin->setEnabled(false);
	cnt_plot_fmin->setNumButtons(3);
	cnt_plot_fmin->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_plot_fmin, SIGNAL(valueChanged(double)), SLOT(update_plot_fmin(double)));

	cnt_current_distro= new QwtCounter(this);
	Q_CHECK_PTR(cnt_current_distro);
	cnt_current_distro->setRange(1.0, 50.0, 1.0);
	cnt_current_distro->setEnabled(false);
	cnt_current_distro->setNumButtons(3);
	cnt_current_distro->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_current_distro, SIGNAL(valueChanged(double)), SLOT(update_current_distro(double)));

	lbl_plot_fmax = new QLabel(tr("  Plot Limit f/f0 max.: "), this);
	Q_CHECK_PTR(lbl_plot_fmax);
	lbl_plot_fmax->setAlignment(AlignLeft|AlignVCenter);
	lbl_plot_fmax->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_plot_fmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_plot_fmax= new QwtCounter(this);
	Q_CHECK_PTR(cnt_plot_fmax);
	cnt_plot_fmax->setRange(1, 50, 0.01);
	cnt_plot_fmax->setEnabled(false);
	cnt_plot_fmax->setValue(plot_fmax);
	cnt_plot_fmax->setNumButtons(3);
	cnt_plot_fmax->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_plot_fmax, SIGNAL(valueChanged(double)), SLOT(update_plot_fmax(double)));

	lbl_plot_smin = new QLabel(tr("  Plot Limit s min.: "), this);
	Q_CHECK_PTR(lbl_plot_smin);
	lbl_plot_smin->setAlignment(AlignLeft|AlignVCenter);
	lbl_plot_smin->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_plot_smin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_plot_smin= new QwtCounter(this);
	Q_CHECK_PTR(cnt_plot_smin);
	cnt_plot_smin->setRange(0.0, 10000.0, 0.1);
	cnt_plot_smin->setValue(plot_smin);
	cnt_plot_smin->setEnabled(false);
	cnt_plot_smin->setNumButtons(3);
	cnt_plot_smin->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_plot_smin, SIGNAL(valueChanged(double)), SLOT(update_plot_smin(double)));

	lbl_plot_smax = new QLabel(tr("  Plot Limit s max.: "), this);
	Q_CHECK_PTR(lbl_plot_smax);
	lbl_plot_smax->setAlignment(AlignLeft|AlignVCenter);
	lbl_plot_smax->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_plot_smax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_plot_smax= new QwtCounter(this);
	Q_CHECK_PTR(cnt_plot_smax);
	cnt_plot_smax->setRange(0.0, 10000.0, 0.1);
	cnt_plot_smax->setValue(plot_smax);
	cnt_plot_smax->setEnabled(false);
	cnt_plot_smax->setNumButtons(3);
	cnt_plot_smax->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_plot_smax, SIGNAL(valueChanged(double)), SLOT(update_plot_smax(double)));

	cb_plot_s = new QCheckBox(tr("Plot f/f0 vs s"),this);
	cb_plot_s->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_plot_s->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_plot_s->setChecked(plot_s);
	cb_plot_s->setEnabled(true);
	cb_plot_s->setMinimumHeight(minHeight1);
	connect(cb_plot_s, SIGNAL(clicked()), SLOT(select_plot_s()));

	cb_plot_mw = new QCheckBox(tr("Plot f/f0 vs MW"),this);
	cb_plot_mw->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_plot_mw->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_plot_mw->setChecked(!plot_s);
	cb_plot_mw->setEnabled(true);
	cb_plot_mw->setMinimumHeight(minHeight1);
	connect(cb_plot_mw, SIGNAL(clicked()), SLOT(select_plot_mw()));

	pb_load_distro = new QPushButton(tr(" Load Distribution "), this);
	Q_CHECK_PTR(pb_load_distro);
	pb_load_distro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load_distro->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_load_distro->setAutoDefault(false);
	connect(pb_load_distro, SIGNAL(clicked()), SLOT(load_distro()));

	pb_help = new QPushButton(tr(" Help "), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setAutoDefault(false);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_print = new QPushButton(tr("Print"), this);
	Q_CHECK_PTR(pb_print);
	pb_print->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_print->setEnabled(true);
	pb_print->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_print->setAutoDefault(false);
	pb_print->setEnabled(false);
	connect(pb_print, SIGNAL(clicked()), SLOT(print()));

	pb_color = new QPushButton(tr("Load Color File"), this);
	Q_CHECK_PTR(pb_color);
	pb_color->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_color->setEnabled(true);
	pb_color->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_color->setAutoDefault(false);
	pb_color->setEnabled(true);
	connect(pb_color, SIGNAL(clicked()), SLOT(load_color()));

	pb_common_limits = new QPushButton(tr("Find Common Limits"), this);
	Q_CHECK_PTR(pb_common_limits);
	pb_common_limits->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_common_limits->setEnabled(true);
	pb_common_limits->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_common_limits->setAutoDefault(false);
	pb_common_limits->setEnabled(true);
	connect(pb_common_limits, SIGNAL(clicked()), SLOT(common_limits()));

	pb_replot3d = new QPushButton(tr("Refresh Pseudo-3D Plot"), this);
	Q_CHECK_PTR(pb_replot3d);
	pb_replot3d->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_replot3d->setEnabled(true);
	pb_replot3d->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_replot3d->setAutoDefault(false);
	pb_replot3d->setEnabled(false);
	connect(pb_replot3d, SIGNAL(clicked()), SLOT(plot_3dim()));

	pb_save = new QPushButton(tr("Save"), this);
	Q_CHECK_PTR(pb_save);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_save->setEnabled(false);
	pb_save->setAutoDefault(false);
	connect(pb_save, SIGNAL(clicked()), SLOT(save()));

	pb_reset = new QPushButton(tr("Reset"), this);
	Q_CHECK_PTR(pb_reset);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_reset->setEnabled(false);
	pb_reset->setAutoDefault(false);
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

	pb_close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_close);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setAutoDefault(false);
	connect(pb_close, SIGNAL(clicked()), SLOT(close()));

	plot = new QwtPlot(this);
	Q_CHECK_PTR(plot);
	plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	plot->enableGridXMin(false);
	plot->enableGridYMin(false);
	plot->enableGridX(false);
	plot->enableGridY(false);
	plot->enableOutline(false);
	plot->setCanvasBackground(USglobal->global_colors.plot);		//new version
	plot->setTitle(tr("Pseudo-3D Distribution Data"));
	plot->setMinimumSize(550,300);
	plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	plot->setMargin(USglobal->config_list.margin);

	progress = new QProgressBar(this, "Progress Bar");
	progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	int rows=17, columns = 3, spacing = 2, j=0;
	QGridLayout *controlGrid = new QGridLayout(this, rows, columns, spacing, spacing);

	controlGrid->setColSpacing(0, 120);
	controlGrid->setColSpacing(1, 120);
	for (int i=0; i<1; i++)
	{
		controlGrid->setRowSpacing(i, 30);
	}
	for (int i=2; i<rows; i++)
	{
		controlGrid->setRowSpacing(i, 26);
	}
	controlGrid->addMultiCellWidget(lbl_info1, j, j, 0, 1);
	controlGrid->addMultiCellWidget(plot, j, j+rows-1, 2, 2);
	controlGrid->setRowStretch(j, 1);
	j++;
	controlGrid->addWidget(lbl_resolution, j, 0);
	controlGrid->addWidget(cnt_resolution, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(lbl_x_resolution, j, 0);
	controlGrid->addWidget(cnt_x_resolution, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(lbl_y_resolution, j, 0);
	controlGrid->addWidget(cnt_y_resolution, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(lbl_x_pixel, j, 0);
	controlGrid->addWidget(cnt_x_pixel, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(lbl_y_pixel, j, 0);
	controlGrid->addWidget(cnt_y_pixel, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(lbl_autolimit, j, 0);
	controlGrid->addWidget(cb_autolimit, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(lbl_plot_fmin, j, 0);
	controlGrid->addWidget(cnt_plot_fmin, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(lbl_plot_fmax, j, 0);
	controlGrid->addWidget(cnt_plot_fmax, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(lbl_plot_smin, j, 0);
	controlGrid->addWidget(cnt_plot_smin, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(lbl_plot_smax, j, 0);
	controlGrid->addWidget(cnt_plot_smax, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(cb_plot_s, j, 0);
	controlGrid->addWidget(cb_plot_mw, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(pb_replot3d, j, 0);
	controlGrid->addWidget(pb_common_limits, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(pb_load_distro, j, 0);
	controlGrid->addWidget(pb_color, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(pb_reset, j, 0);
	controlGrid->addWidget(cnt_current_distro, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(pb_print, j, 0);
	controlGrid->addWidget(pb_save, j, 1);
	controlGrid->setRowStretch(j, 0);
	j++;
	controlGrid->addWidget(pb_help, j, 0);
	controlGrid->addWidget(pb_close, j, 1);
	j++;
	controlGrid->addMultiCellWidget(progress, j, j, 0, 1);
	controlGrid->setRowStretch(j, 0);
	controlGrid->setColStretch(0, 0);
	controlGrid->setColStretch(1, 1);
	controlGrid->setColStretch(2, 3);
}

void US_Pseudo3D_Combine::load_distro()
{
	QString filename = QFileDialog::getOpenFileName(USglobal->config_list.result_dir,
														 "*.fe_dis.* *.cofs_dis.* *.sa2d_dis.* *.sa2d_mw_dis.* *.ga_dis.* *.ga_mw_dis.* *.ga_mw_mc_dis.* *.ga_mc_dis.* *.sa2d_mc_dis.* *.sa2d_mw_mc_dis.* *.global_dis.* ", 0);
	if (filename.isEmpty())
	{
		return;
	}
	else
	{
		load_distro(filename);
	}
}

void US_Pseudo3D_Combine::load_distro(const QString &filename)
{
	QFile f;
	unsigned int index, i;
	float temp1, temp2, temp3, temp4;
	QString str;
	struct distro_system temp_system;
	temp_system.gradient.clear();
	unsigned int k;
	for (k=0; k<current_gradient.size(); k++) // assign default gradient from black to bright cyan over 510 color points
	{
		temp_system.gradient.push_back(current_gradient[k]);
	}

	temp_system.s_distro.clear();
	temp_system.mw_distro.clear();
	class Solute temp_s_distro, temp_mw_distro;
	index = filename.findRev(".", -1, true);
	cell_info = filename.right(filename.length() - index);
	f.setName(filename);
	monte_carlo = false;
	if (filename.contains("vhw_his", false))
	{
		distro_type = 0;
		index = filename.findRev(".vhw_his.", -1, true);
		id = filename.left(index);
	}
	else if (filename.contains("cofs_dis", false))
	{
		distro_type = 1;
		index = filename.findRev(".cofs_dis.", -1, true);
		id = filename.left(index);
	}
	else if (filename.contains("fe_dis", false))
	{
		distro_type = 2;
		index = filename.findRev(".fe_dis.", -1, true);
		id = filename.left(index);
	}
	else if (filename.contains("sa2d_dis", false))
	{
		distro_type = 3;
		index = filename.findRev(".sa2d_dis.", -1, true);
		id = filename.left(index);
	}
	else if (filename.contains("ga_mc_dis", false))
	{
		distro_type = 4;
		monte_carlo = true;
		index = filename.findRev(".ga_mc_dis.", -1, true);
		id = filename.left(index);
	}
	else if (filename.contains("sa2d_mc_dis", false))
	{
		distro_type = 5;
		monte_carlo = true;
		index = filename.findRev(".sa2d_mc_dis.", -1, true);
		id = filename.left(index);
	}
	else if (filename.contains("ga_dis", false))
	{
		distro_type = 6;
		monte_carlo = false;
		index = filename.findRev(".ga_dis.", -1, true);
		id = filename.left(index);
	}
	else if (filename.contains("global_dis", false))
	{
		distro_type = 7;
		monte_carlo = false;
		index = filename.findRev(".global_dis.", -1, true);
		id = filename.left(index);
	}
	else if (filename.contains("sa2d_mw_dis", false))
	{
		distro_type = 8;
		monte_carlo = false;
		index = filename.findRev(".sa2d_mw_dis.", -1, true);
		id = filename.left(index);
	}
	else if (filename.contains("ga_mw_dis", false))
	{
		distro_type = 9;
		monte_carlo = false;
		index = filename.findRev(".ga_mw_dis.", -1, true);
		id = filename.left(index);
	}
	else if (filename.contains("sa2d_mw_mc_dis", false))
	{
		distro_type = 10;
		monte_carlo = true;
		index = filename.findRev(".sa2d_mw_mc_dis.", -1, true);
		id = filename.left(index);
	}
	else if (filename.contains("ga_mw_mc_dis", false))
	{
		distro_type = 11;
		monte_carlo = true;
		index = filename.findRev(".ga_mw_mc_dis.", -1, true);
		id = filename.left(index);
	}
	else
	{
		distro_type = -1; // undefined
	}
	if (distro_type > 0)
	{
		if(f.open(IO_ReadOnly))
		{
			QTextStream ts(&f);
			if (!ts.atEnd())
			{
				ts.readLine(); // discard header line
			}
			if (monte_carlo) // GA Monte Carlo, we need the number of MC iterations
			{
				QString s1;
				ts >> s1;
				ts.readLine(); // read the rest of the line
				monte_carlo_iterations = s1.toUInt();
			}
			while (!ts.atEnd())
			{
				ts >> temp1; // s_apparent
				ts >> temp1; // s_20,W
				temp1 *= 1.0e13; // change to proper scale
				ts >> temp2; // D_apparent
				ts >> temp2; // D_20,W
				ts >> temp2; // MW
				ts >> temp3; // Frequency
				ts >> temp4; // f/f0
				temp_s_distro.s = temp1;
				temp_s_distro.c = temp3;
				temp_s_distro.k = temp4;
				temp_mw_distro.s = temp2;
				temp_mw_distro.c = temp3;
				temp_mw_distro.k = temp4;
				temp_system.s_distro.push_back(temp_s_distro);
				temp_system.mw_distro.push_back(temp_mw_distro);
			}
			f.close();
		}
		temp_system.s_distro.sort();
		temp_system.mw_distro.sort();

// combine identical solutes:

		list <class Solute> reduced;
		list <class Solute>::iterator j, j1, j2;
		reduced.clear();
		i = 0;
		reduced = temp_system.s_distro;
		temp_system.s_distro.clear();
		j1 = reduced.begin();
		j2 = reduced.begin();
		while (j2 != reduced.end())
		{
			j2++;
			if ((*j1).s == (*j2).s && (*j1).k == (*j2).k)
			{
				(*j2).c += (*j1).c;
			}
			else
			{
				temp_system.s_distro.push_back(*j1);
			}
			j1++;
		}
		reduced.clear();
		reduced = temp_system.mw_distro;
		temp_system.mw_distro.clear();
		j1 = reduced.begin();
		j2 = reduced.begin();
		while (j2 != reduced.end())
		{
			j2++;
			if ((*j1).s == (*j2).s && (*j1).k == (*j2).k)
			{
				(*j2).c += (*j1).c;
			}
			else
			{
				temp_system.mw_distro.push_back(*j1);
			}
			j1++;
		}
		reduced.clear();
	}
	system.push_back(temp_system);
	current_distro = system.size() - 1;
	cnt_current_distro->setRange(1.0, current_distro+1, 1.0);
	current_distro = system.size() - 1;
	cnt_current_distro->setValue(current_distro+1);
	cnt_current_distro->setEnabled(true);
	if (autolimit)
	{
		set_limits();
		cnt_plot_fmin->setEnabled(false);
		cnt_plot_smin->setEnabled(false);
		cnt_plot_fmax->setEnabled(false);
		cnt_plot_smax->setEnabled(false);
	}
	pb_print->setEnabled(true);
	pb_replot3d->setEnabled(true);
	pb_reset->setEnabled(true);
	cb_plot_s->setEnabled(true);
	cb_plot_mw->setEnabled(true);
}

void US_Pseudo3D_Combine::set_limits()
{
	double smin=1.0e30, smax=-1.0e30, fmin=1.0e30, fmax=-1.0e30;
	unsigned int i;
	list <struct Solute>::iterator iter;
	if (plot_s)
	{
		plot->setAxisTitle(QwtPlot::xBottom, tr("Sedimentation Coefficient corrected for water at 20ºC"));
		for (i=0; i<system.size(); i++)
		{
			for (iter = system[i].s_distro.begin(); iter != system[i].s_distro.end(); iter++)
			{
				smin = min(smin, (double) (*iter).s);
				smax = max(smax, (double) (*iter).s);
				fmin = min(fmin, (double) (*iter).k);
				fmax = max(fmax, (double) (*iter).k);
			}
		}
	}
	else
	{
		plot->setAxisTitle(QwtPlot::xBottom, tr("Molecular Weight (Dalton)"));
		for (i=0; i<system.size(); i++)
		{
			for (iter = system[i].mw_distro.begin(); iter != system[i].mw_distro.end(); iter++)
			{
				smin = min(smin, (double) (*iter).s);
				smax = max(smax, (double) (*iter).s);
				fmin = min(fmin, (double) (*iter).k);
				fmax = max(fmax, (double) (*iter).k);
			}
		}
	}
	float diff;
	diff = smax - smin;
	smin -= diff/10;
	smax += diff/10;
	diff = fmax - fmin;
	fmin -= diff/10;
	fmax += diff/10;
	if (smin < 0.0)
	{
		smin = 0.0;
	}
	if (autolimit)
	{
		cnt_plot_smax->setRange(0.0, smax * 10, pow(10.0, (double)((int) (log10(smax*10)-3.0))));
		cnt_plot_smin->setRange(0.0, smax * 10, pow(10.0, (double)((int) (log10(smax*10)-3.0))));
		smax += (smax-smin)/20;
		smin -= (smax-smin)/20;
		fmax += (fmax-fmin)/20;
		fmin -= (fmax-fmin)/20;
		if (fmin < 0.0)
		{
			fmin = 0.0;
		}
		if ((fmax - fmin) < 1.0e-3)
		{
			fmax += fmax/10.0;
		}
		cnt_plot_fmin->setValue(fmin);
		plot_fmin = fmin;
		cnt_plot_smin->setValue(smin);
		plot_smin = smin;
		cnt_plot_fmax->setValue(fmax);
		plot_fmax = fmax;
		cnt_plot_smax->setValue(smax);
		plot_smax = smax;
	}
}

void US_Pseudo3D_Combine::plot_3dim()
{
	QString str;
	plot->clear();
	progress->reset();
	plot->setAxisTitle(QwtPlot::yLeft, tr("Frictional Ratio f/f0"));
	QwtSymbol symbol;
	QSize size(x_pixel, y_pixel);
	symbol.setStyle(QwtSymbol::Rect);
	symbol.setSize(size);
	unsigned int i, j, k, count;
	unsigned int curve[system[current_distro].gradient.size()];
	double x[x_resolution], y[y_resolution], z[x_resolution][y_resolution];
	double frange, srange, sstep, fstep, ssigma, fsigma;
	double *xval, *yval;
	xval = new double [x_resolution*y_resolution];
	yval = new double [x_resolution*y_resolution];
	list <struct Solute>::iterator iter;
	progress->setTotalSteps(system[current_distro].s_distro.size());
	srange = plot_smax - plot_smin;
	sstep = srange/(double) x_resolution;
	frange = plot_fmax - plot_fmin;
	fstep = frange/(double) y_resolution;
	ssigma = srange/resolution;
	fsigma = frange/resolution;
	plot->setAxisScale(QwtPlot::xBottom, plot_smin, plot_smax);
	plot->setAxisScale(QwtPlot::yLeft, plot_fmin, plot_fmax);
	for (i=0; i<x_resolution; i++)
	{
		x[i] = plot_smin + i * sstep;
		for (j=0; j<y_resolution; j++)
		{
			z[i][j] = 0.0;
		}
	}
	for (j=0; j<y_resolution; j++)
	{
		y[j] = plot_fmin + j * fstep;
	}
	double maxval = 0;
	count = 0;
	if (plot_s)
	{
		if (resolution == 100)
		{
			for (iter = system[current_distro].s_distro.begin(); iter != system[current_distro].s_distro.end(); iter++)
			{
				for (i=0; i<x_resolution; i++)
				{
					for (j=0; j<y_resolution; j++)
					{
//				cout << "y: " << y[i] << ", it: " <<  (*iter).k  << ", x: " << x[i] << ", it: " <<  (*iter).k << endl;
						if (y[j] > (*iter).k - fsigma && y[j] < (*iter).k + fsigma
											 && x[i] > (*iter).s - ssigma && x[i] < (*iter).s + ssigma)
						{
							z[i][j] += (*iter).c;
						}
						maxval = max(maxval, z[i][j]);
//					cout << "z[" << i << "][" << j << "]: " << z[i][j] << ", maxval: " << maxval <<endl;
					}
				}
				count++;
				progress->setProgress(count);
			}
		}
		else
		{
			if(USglobal->config_list.numThreads > 1)
			{
				double maxvals[USglobal->config_list.numThreads];
				double *zz[x_resolution];
				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					maxvals[j] = 0;
				}
				for(j = 0; j < x_resolution; j++)
				{
					zz[j] = z[j];
				}

			// create threads

				US_Plot3d_thr_t *plot3d_thr_threads[USglobal->config_list.numThreads];
				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					plot3d_thr_threads[j] = new US_Plot3d_thr_t(j);
					plot3d_thr_threads[j]->start();
				}
				unsigned int x_inc = x_resolution / USglobal->config_list.numThreads;
				unsigned int x_end;
				unsigned int x_start;
				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					x_start = x_inc * j;
					x_end = (x_inc * (j + 1)) - 1;
					if(j + 1 == USglobal->config_list.numThreads)
					{
						x_end = x_resolution - 1;
					}
					plot3d_thr_threads[j]->plot3d_thr_setup(j, zz, system[current_distro].s_distro, x, y, x_start, x_end, y_resolution, &maxvals[j], ssigma, fsigma, progress);
				}
				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					plot3d_thr_threads[j]->plot3d_thr_wait();
				}

			// destroy

				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					plot3d_thr_threads[j]->plot3d_thr_shutdown();
				}

				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					plot3d_thr_threads[j]->wait();
				}

				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					maxval = max(maxval,maxvals[j]);
					delete plot3d_thr_threads[j];
				}
			}
			else
			{
				for (iter = system[current_distro].s_distro.begin(); iter != system[current_distro].s_distro.end(); iter++)
				{
					for (i=0; i<x_resolution; i++)
					{
						for (j=0; j<y_resolution; j++)
						{
							z[i][j] += (*iter).c * exp(-pow((x[i] - (*iter).s), 2.0)/(pow(2.0 * ssigma, 2.0)))
									* exp(-pow((y[j] - (*iter).k), 2.0)/(pow(2.0 * fsigma, 2.0)));
							maxval = max(maxval, z[i][j]);
						}
					}
					count++;
					progress->setProgress(count);
				}
			}
		}
	}
	else
	{
		if (resolution == 100)
		{
			for (iter = system[current_distro].mw_distro.begin(); iter != system[current_distro].mw_distro.end(); iter++)
			{
				for (i=0; i<x_resolution; i++)
				{
					for (j=0; j<y_resolution; j++)
					{
//				cout << "y: " << y[i] << ", it: " <<  (*iter).k  << ", x: " << x[i] << ", it: " <<  (*iter).k << endl;
						if (y[j] > (*iter).k - fsigma && y[j] < (*iter).k + fsigma
											 && x[i] > (*iter).s - ssigma && x[i] < (*iter).s + ssigma)
						{
							z[i][j] += (*iter).c;
						}
						maxval = max(maxval, z[i][j]);
//					cout << "z[" << i << "][" << j << "]: " << z[i][j] << ", maxval: " << maxval <<endl;
					}
				}
				count++;
				progress->setProgress(count);
			}
		}
		else
		{
			if(USglobal->config_list.numThreads > 1)
			{
				double maxvals[USglobal->config_list.numThreads];
				double *zz[x_resolution];
				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					maxvals[j] = 0;
				}
				for(j = 0; j < x_resolution; j++)
				{
					zz[j] = z[j];
				}

			// create threads

				US_Plot3d_thr_t *plot3d_thr_threads[USglobal->config_list.numThreads];
				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					plot3d_thr_threads[j] = new US_Plot3d_thr_t(j);
					plot3d_thr_threads[j]->start();
				}
				unsigned int x_inc = x_resolution / USglobal->config_list.numThreads;
				unsigned int x_end;
				unsigned int x_start;
				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					x_start = x_inc * j;
					x_end = (x_inc * (j + 1)) - 1;
					if(j + 1 == USglobal->config_list.numThreads)
					{
						x_end = x_resolution - 1;
					}
					plot3d_thr_threads[j]->plot3d_thr_setup(j, zz, system[current_distro].mw_distro, x, y, x_start, x_end, y_resolution, &maxvals[j], ssigma, fsigma, progress);
				}
				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					plot3d_thr_threads[j]->plot3d_thr_wait();
				}

			// destroy

				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					plot3d_thr_threads[j]->plot3d_thr_shutdown();
				}

				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					plot3d_thr_threads[j]->wait();
				}

				for(j = 0; j < USglobal->config_list.numThreads; j++)
				{
					maxval = max(maxval,maxvals[j]);
					delete plot3d_thr_threads[j];
				}
			}
			else
			{
				for (iter = system[current_distro].mw_distro.begin(); iter != system[current_distro].mw_distro.end(); iter++)
				{
					for (i=0; i<x_resolution; i++)
					{
						for (j=0; j<y_resolution; j++)
						{
							z[i][j] += (*iter).c * exp(-pow((x[i] - (*iter).s), 2.0)/(pow(2.0 * ssigma, 2.0)))
									* exp(-pow((y[j] - (*iter).k), 2.0)/(pow(2.0 * fsigma, 2.0)));
							maxval = max(maxval, z[i][j]);
						}
					}
					count++;
					progress->setProgress(count);
				}
			}
		}
	}
	for (i=0; i<x_resolution; i++)
	{
		x[i] = plot_smin + i * sstep;
		for (j=0; j<y_resolution; j++)
		{
			z[i][j] = (system[current_distro].gradient.size()-1) * z[i][j]/maxval;
		}
	}
	QFileInfo fi(id);
	QString htmlDir = USglobal->config_list.html_dir + "/" + fi.fileName();
	QString str1;
	switch (distro_type)
	{
		case 1:
		{
			str1 = "\n(C(s))";
			break;
		}
		case 2:
		{
			str1 = "\n(FE)";
			break;
		}
		case 3:
		{
			str1 = "\n(2DSA)";
			break;
		}
		case 4:
		{
			str1 = "\n(GA-MC)";
			break;
		}
		case 5:
		{
			str1 = "\n(2DSA-MC)";
			break;
		}
		case 6:
		{
			str1 = "\n(GA)";
			break;
		}
		case 7:
		{
			str1 = "\n(Global)";
			break;
		}
	}
	for (k=0; k<system[current_distro].gradient.size(); k++)
	{
		count = 0;
		for (i=0; i<x_resolution; i++)
		{
			for (j=0; j<y_resolution; j++)
			{
				if ((unsigned int) (z[i][j]) == k) // filter out the points with color k
				{
					xval[count] = x[i];
					yval[count] = y[j];
					count++;
				}
			}
		}
		curve[k] = plot->insertCurve(str.sprintf("color level %d", k + 1));
		symbol.setPen(system[current_distro].gradient[k]);
		symbol.setBrush(system[current_distro].gradient[k]);
		plot->setCurveSymbol(curve[k], symbol);
		plot->setCurveStyle(curve[k], QwtCurve::NoCurve);
		plot->setCurveData(curve[k], xval, yval, count);
		plot->setTitle(fi.fileName() + cell_info + str1);
		system[current_distro].name = fi.fileName() + cell_info + str1;
	}
	plot->replot();
	delete [] xval;
	delete [] yval;
//	cout << distro_type << endl;
	switch (distro_type)
	{
		case 1:
		{
			if (plot_s)
			{
				str = htmlDir + "/cofs_pseudo3d_fs" + cell_info + ".";
			}
			else
			{
				str = htmlDir + "/cofs_pseudo3d_fmw" + cell_info + ".";
			}
			break;
		}
		case 2:
		{
			if (plot_s)
			{
				str = htmlDir + "/fe_pseudo3d_fs" + cell_info + ".";
			}
			else
			{
				str = htmlDir + "/fe_pseudo3d_fmw" + cell_info + ".";
			}
			break;
		}
		case 3:
		{
			if (plot_s)
			{
				str = htmlDir + "/sa2d_pseudo3d_fs" + cell_info + ".";
			}
			else
			{
				str = htmlDir + "/sa2d_pseudo3d_fmw" + cell_info + ".";
			}
			break;
		}
		case 4:
		{
			if (plot_s)
			{
				str = htmlDir + "/ga_mc_pseudo3d_fs" + cell_info + ".";
			}
			else
			{
				str = htmlDir + "/ga_mc_pseudo3d_fmw" + cell_info + ".";
			}
			break;
		}
		case 5:
		{
			if (plot_s)
			{
				str = htmlDir + "/sa2d_mc_pseudo3d_fs" + cell_info + ".";
			}
			else
			{
				str = htmlDir + "/sa2d_mc_pseudo3d_fmw" + cell_info + ".";
			}
			break;
		}
		case 6:
		{
			if (plot_s)
			{
				str = htmlDir + "/ga_pseudo3d_fs" + cell_info + ".";
			}
			else
			{
				str = htmlDir + "/ga_pseudo3d_fmw" + cell_info + ".";
			}
			break;
		}
		case 7:
		{
			if (plot_s)
			{
				str = htmlDir + "/global_pseudo3d_fs" + cell_info + ".";
			}
			else
			{
				str = htmlDir + "/global_pseudo3d_fmw" + cell_info + ".";
			}
			break;
		}
		case 8:
		{
			if (plot_s)
			{
				str = htmlDir + "/sa2d_mw_pseudo3d_fs" + cell_info + ".";
			}
			else
			{
				str = htmlDir + "/sa2d_mw_pseudo3d_fmw" + cell_info + ".";
			}
			break;
		}
		case 9:
		{
			if (plot_s)
			{
				str = htmlDir + "/ga_mw_pseudo3d_fs" + cell_info + ".";
			}
			else
			{
				str = htmlDir + "/ga_mw_pseudo3d_fmw" + cell_info + ".";
			}
			break;
		}
		case 10:
		{
			if (plot_s)
			{
				str = htmlDir + "/sa2d_mw_mc_pseudo3d_fs" + cell_info + ".";
			}
			else
			{
				str = htmlDir + "/sa2d_mw_mc_pseudo3d_fmw" + cell_info + ".";
			}
			break;
		}
		case 11:
		{
			if (plot_s)
			{
				str = htmlDir + "/ga_mw_mc_pseudo3d_fs" + cell_info + ".";
			}
			else
			{
				str = htmlDir + "/ga_mw_mc_pseudo3d_fmw" + cell_info + ".";
			}
			break;
		}
	}
//	cout << str << endl;
	QPixmap p;
	US_Pixmap *pm;
	pm = new US_Pixmap();
	p = QPixmap::grabWidget(plot, 2, 2, plot->width() - 4, plot->height() - 4);
	pm->save_file(str, p);
}

void US_Pseudo3D_Combine::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/pseudo3d_combine.html");
}

void US_Pseudo3D_Combine::load_color()
{
	QString line, filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc",
	"*.map", 0);
	QFile f(filename);
	QColor col;
	current_gradient.clear();
	int r, g, b;
	float val;
	if (system.size() > 0)
	{
		system[current_distro].gradient.clear();
	}
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		while (!ts.atEnd())
		{
			ts >> val;
			r = (int) val;
			ts >> val;
			g = (int) val;
			ts >> val;
			b = (int) val;
			col.setRgb(r, g, b);
			current_gradient.push_back(col);
			if (system.size() > 0)
			{
				system[current_distro].gradient.push_back(col);
			}
		}
		f.close();
	}
}

void US_Pseudo3D_Combine::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

void US_Pseudo3D_Combine::save()
{
}

void US_Pseudo3D_Combine::reset()
{
	for (unsigned int i=0; i<system.size(); i++)
	{
		system[i].gradient.clear();
	}
	system.clear();
	plot->clear();
	plot->replot();
	pb_reset->setEnabled(false);
	distro_type = -1;
	minmax = false;
	zoom = false;
	monte_carlo = false;
	plot_s = true;
	select_plot_s();
	cb_plot_s->setChecked(true);
	cb_plot_mw->setChecked(false);
	cb_plot_s->setEnabled(false);
	cb_plot_mw->setEnabled(false);
	current_distro = 0;
	cnt_plot_fmin->setRange(0, 50, 0.01);
	plot_fmin = 1.0;
	cnt_plot_fmin->setValue(plot_fmin);
	cnt_plot_fmin->setEnabled(false);

	cnt_plot_fmax->setRange(1, 50, 0.01);
	plot_fmax = 4.0;
	cnt_plot_fmax->setValue(plot_fmax);
	cnt_plot_fmax->setEnabled(false);

	cnt_plot_smin->setRange(0.0, 10000.0, 0.1);
	plot_smin = 1.0;
	cnt_plot_smin->setValue(plot_smin);
	cnt_plot_smin->setEnabled(false);

	cnt_plot_smax->setRange(0.0, 10000.0, 0.1);
	plot_smax = 10.0;
	cnt_plot_smax->setValue(plot_smax);
	cnt_plot_smax->setEnabled(false);

	resolution = 90.0;
	cnt_resolution->setRange(1, 100, 1);
	cnt_resolution->setValue(resolution);
	x_pixel = 2;
	y_pixel = 2;
	cnt_x_pixel->setRange(2, 50, 1);
	cnt_x_pixel->setValue(x_pixel);
	cnt_y_pixel->setRange(2, 50, 1);
	cnt_y_pixel->setValue(y_pixel);
	x_resolution = 300;
	y_resolution = 300;
	cnt_y_resolution->setRange(10, 1000, 1);
	cnt_y_resolution->setValue(y_resolution);
	cnt_x_resolution->setRange(10, 1000, 1);
	cnt_x_resolution->setValue(x_resolution);

	autolimit = true;
	cb_autolimit->setChecked(true);
	current_distro=0;
	cnt_current_distro->setRange(1.0, 1.0, 1.0);
	cnt_current_distro->setValue(1.0);
	cnt_current_distro->setEnabled(false);

	current_gradient.resize(510);
	unsigned int k, g, b;
	for (k=0; k<510; k++) // assign default gradient from black to bright cyan over 510 color points
	{
		if (k > 255)
		{
			g = (unsigned int) (k - 255);
			b = 255;
		}
		else
		{
			g = 0;
			b = k;
		}
		current_gradient[k].setRgb(0, g, b);
	}
}

void US_Pseudo3D_Combine::print()
{
	QPrinter printer;
	printer.setColorMode(QPrinter::Color);
	printer.setPageSize(QPrinter::Letter);
	printer.setOrientation(QPrinter::Landscape);
	if (printer.setup(0))
	{
		plot->print(printer);
	}
}

void US_Pseudo3D_Combine::select_autolimit()
{
	if (cb_autolimit->isChecked())
	{
		autolimit = true;
		cnt_plot_fmin->setEnabled(false);
		cnt_plot_smin->setEnabled(false);
		cnt_plot_fmax->setEnabled(false);
		cnt_plot_smax->setEnabled(false);
	}
	else
	{
		autolimit = false;
		cnt_plot_fmin->setEnabled(true);
		cnt_plot_smin->setEnabled(true);
		cnt_plot_fmax->setEnabled(true);
		cnt_plot_smax->setEnabled(true);
	}
}

void US_Pseudo3D_Combine::select_plot_s()
{
	cb_plot_s->setChecked(true);
	cb_plot_mw->setChecked(false);
	plot_s = true;
	set_limits();
	lbl_plot_smin->setText(tr("  Plot Limit s min.: "));
	lbl_plot_smax->setText(tr("  Plot Limit s max.: "));
}

void US_Pseudo3D_Combine::select_plot_mw()
{
	cb_plot_mw->setChecked(true);
	cb_plot_s->setChecked(false);
	plot_s = false;
	set_limits();
	lbl_plot_smin->setText(tr("  Plot Limit MW min.: "));
	lbl_plot_smax->setText(tr("  Plot Limit MW max.: "));
}

void US_Pseudo3D_Combine::common_limits()
{
	cb_autolimit->setChecked(true);
	select_autolimit();

}

void US_Pseudo3D_Combine::update_current_distro(double val)
{
	current_distro = (unsigned int) val;
	current_distro --;
}

void US_Pseudo3D_Combine::update_resolution(double val)
{
	resolution = (float) val;
}

void US_Pseudo3D_Combine::update_x_resolution(double val)
{
	x_resolution = (unsigned int) val;
}

void US_Pseudo3D_Combine::update_y_resolution(double val)
{
	y_resolution = (unsigned int) val;
}

void US_Pseudo3D_Combine::update_x_pixel(double val)
{
	x_pixel = (unsigned int) val;
}

void US_Pseudo3D_Combine::update_y_pixel(double val)
{
	y_pixel = (unsigned int) val;
}

void US_Pseudo3D_Combine::update_plot_smin(double val)
{
	plot_smin = val;
}

void US_Pseudo3D_Combine::update_plot_smax(double val)
{
	plot_smax = val;
}

void US_Pseudo3D_Combine::update_plot_fmin(double val)
{
	plot_fmin = val;
}

void US_Pseudo3D_Combine::update_plot_fmax(double val)
{
	plot_fmax = val;
}

