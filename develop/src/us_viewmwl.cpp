#include "../include/us_viewmwl.h"

US_ViewMWL::US_ViewMWL(QWidget *p, const char *name) : QFrame(p, name)
{
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Multiwavelength Data Viewer (Generation 2)"));

   progress = new QProgressBar(this, "File Processing Progress");
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   max_lambda       = 800.0;
   max_time         = 1;
   max_radius       = (float) 7.3;
   min_lambda       = 200.0;
   min_time         = 1;
   min_radius       = (float) 5.8;
   average          = 1;
   view_mode        = true;    // plot 2D by default
   export_mode      = false;   // plot radius by default
   file_format      = true;    // load ASCII data by default
   current_cell     = 0;       // select cell 1 by default
   current_channel  = 0;       // select channel 1 by default
   measurement_mode = 0;       // select absorbance by default
   widget3d_flag    = false;
   pngs             = false;
   show_model       = false;
   loading          = true;
   pm               = new US_Pixmap();

   lbl_info = new QLabel(tr("Multiwavelength Data Viewer"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   //   lbl_info->setMinimumSize(250, 35);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_update = new QLabel("",this);
   lbl_update->setAlignment(AlignCenter|AlignVCenter);
   lbl_update->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_update->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lbl_update->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_max_lambda = new QLabel(tr(" Max. Wavelength:"), this);
   Q_CHECK_PTR(lbl_max_lambda);
   lbl_max_lambda->setAlignment(AlignLeft|AlignVCenter);
   lbl_max_lambda->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_max_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_min_lambda = new QLabel(tr(" Min. Wavelength:"), this);
   Q_CHECK_PTR(lbl_min_lambda);
   lbl_min_lambda->setAlignment(AlignLeft|AlignVCenter);
   lbl_min_lambda->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_min_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_min_radius = new QLabel(tr(" Min. Radius:"), this);
   Q_CHECK_PTR(lbl_min_radius);
   lbl_min_radius->setAlignment(AlignLeft|AlignVCenter);
   lbl_min_radius->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_min_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_max_radius = new QLabel(tr(" Max. Radius:"), this);
   Q_CHECK_PTR(lbl_max_radius);
   lbl_max_radius->setAlignment(AlignLeft|AlignVCenter);
   lbl_max_radius->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_max_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_max_time = new QLabel(tr(" Last Scan:"), this);
   Q_CHECK_PTR(lbl_max_time);
   lbl_max_time->setAlignment(AlignLeft|AlignVCenter);
   lbl_max_time->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_max_time->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_min_time = new QLabel(tr(" First Scan:"), this);
   Q_CHECK_PTR(lbl_min_time);
   lbl_min_time->setAlignment(AlignLeft|AlignVCenter);
   lbl_min_time->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_min_time->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_average = new QLabel(tr(" Lambda Average:"), this);
   Q_CHECK_PTR(lbl_average);
   lbl_average->setAlignment(AlignLeft|AlignVCenter);
   lbl_average->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_average->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_max_lambda= new QwtCounter(this);
   Q_CHECK_PTR(cnt_max_lambda);
   cnt_max_lambda->setRange(200.0, 800.0, 0.1);
   cnt_max_lambda->setValue((double)max_lambda);
   cnt_max_lambda->setNumButtons(3);
   cnt_max_lambda->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_max_lambda, SIGNAL(valueChanged(double)), SLOT(update_max_lambda(double)));

   cnt_min_lambda= new QwtCounter(this);
   Q_CHECK_PTR(cnt_min_lambda);
   cnt_min_lambda->setRange(200.0, 800.0, 0.1);
   cnt_min_lambda->setValue((double)min_lambda);
   cnt_min_lambda->setNumButtons(3);
   cnt_min_lambda->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_min_lambda, SIGNAL(valueChanged(double)), SLOT(update_min_lambda(double)));

   cnt_max_radius= new QwtCounter(this);
   Q_CHECK_PTR(cnt_max_radius);
   cnt_max_radius->setRange(5.8, 7.3, 0.001);
   cnt_max_radius->setValue(max_radius);
   cnt_max_radius->setNumButtons(3);
   cnt_max_radius->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_max_radius, SIGNAL(valueChanged(double)), SLOT(update_max_radius(double)));

   cnt_min_radius= new QwtCounter(this);
   Q_CHECK_PTR(cnt_min_radius);
   cnt_min_radius->setRange(5.8, 7.3, 0.001);
   cnt_min_radius->setValue(min_radius);
   cnt_min_radius->setNumButtons(3);
   cnt_min_radius->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_min_radius, SIGNAL(valueChanged(double)), SLOT(update_min_radius(double)));

   cnt_max_time= new QwtCounter(this);
   Q_CHECK_PTR(cnt_max_time);
   cnt_max_time->setRange(1, 1000, 1);
   cnt_max_time->setValue(max_time);
   cnt_max_time->setNumButtons(3);
   cnt_max_time->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_max_time, SIGNAL(valueChanged(double)), SLOT(update_max_time(double)));

   cnt_min_time= new QwtCounter(this);
   Q_CHECK_PTR(cnt_min_time);
   cnt_min_time->setRange(1, 1000, 1);
   cnt_min_time->setValue(min_time);
   cnt_min_time->setNumButtons(3);
   cnt_min_time->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_min_time, SIGNAL(valueChanged(double)), SLOT(update_min_time(double)));

   cnt_average= new QwtCounter(this);
   Q_CHECK_PTR(cnt_average);
   cnt_average->setRange(1, 50, 1);
   cnt_average->setValue(average);
   cnt_average->setNumButtons(3);
   cnt_average->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_average, SIGNAL(valueChanged(double)), SLOT(update_average(double)));

   cb_ascii = new QCheckBox(this);
   cb_ascii->setText(tr(" Load ASCII Data"));
   cb_ascii->setChecked(true);
   cb_ascii->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_ascii->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_ascii, SIGNAL(clicked()), SLOT(set_ascii()));

   cb_binary = new QCheckBox(this);
   cb_binary->setText(tr(" Load Binary Data"));
   cb_binary->setChecked(false);
   cb_binary->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_binary->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_binary, SIGNAL(clicked()), SLOT(set_binary()));

   cb_set_radius = new QCheckBox(this);
   cb_set_radius->setText(tr(" Plot Radius"));
   cb_set_radius->setChecked(true);
   cb_set_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_set_radius->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_set_radius, SIGNAL(clicked()), SLOT(set_radius()));

   cb_set_wavelength = new QCheckBox(this);
   cb_set_wavelength->setText(tr(" Plot Wavelength"));
   cb_set_wavelength->setChecked(false);
   cb_set_wavelength->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_set_wavelength->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_set_wavelength, SIGNAL(clicked()), SLOT(set_wavelength()));

   cb_set_2d = new QCheckBox(this);
   cb_set_2d->setText(tr(" 2-Dimensions"));
   cb_set_2d->setChecked(false);
   cb_set_2d->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_set_2d->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_set_2d, SIGNAL(clicked()), SLOT(set_2d()));

   cb_set_3d = new QCheckBox(this);
   cb_set_3d->setText(tr(" 3-Dimensions"));
   cb_set_3d->setChecked(true);
   cb_set_3d->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_set_3d->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_set_3d, SIGNAL(clicked()), SLOT(set_3d()));

   cb_absorbance = new QCheckBox(this);
   cb_absorbance->setText(tr(" Absorbance Mode"));
   cb_absorbance->setChecked(true);
   cb_absorbance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_absorbance->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_absorbance, SIGNAL(clicked()), SLOT(set_absorbance()));

   cb_intensity = new QCheckBox(this);
   cb_intensity->setText(tr(" Intensity Mode"));
   cb_intensity->setChecked(false);
   cb_intensity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_intensity->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_intensity, SIGNAL(clicked()), SLOT(set_intensity()));

   cb_pngs = new QCheckBox(this);
   cb_pngs->setText(tr(" Save Movie Frames to PNG Images"));
   cb_pngs->setChecked(pngs);
   cb_pngs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pngs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_pngs, SIGNAL(clicked()), SLOT(set_pngs()));

   cb_model = new QCheckBox(this);
   cb_model->setText(tr(" Show Model"));
   cb_model->setChecked(show_model);
   cb_model->setEnabled(false);
   cb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_model->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_model, SIGNAL(clicked()), SLOT(set_model()));

   pb_save = new QPushButton(tr("Save Binary Data"), this);
   Q_CHECK_PTR(pb_save);
   pb_save->setEnabled(false);
   pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));

   lbl_status = new QLabel(tr(" Status: "),this);
   lbl_status->setAlignment(AlignLeft|AlignVCenter);
   lbl_status->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_status->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_export_data = new QPushButton(tr("Export ASCII"), this);
   Q_CHECK_PTR(pb_export_data);
   pb_export_data->setEnabled(false);
   pb_export_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_export_data->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_export_data, SIGNAL(clicked()), SLOT(export_data()));

   pb_print = new QPushButton(tr("Print"), this);
   Q_CHECK_PTR(pb_print);
   pb_print->setEnabled(false);
   pb_print->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_print->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_print, SIGNAL(clicked()), SLOT(print()));

   pb_movie = new QPushButton(tr("Movie"), this);
   Q_CHECK_PTR(pb_movie);
   pb_movie->setEnabled(false);
   pb_movie->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_movie->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_movie, SIGNAL(clicked()), SLOT(movie()));

   pb_update = new QPushButton(tr("Update Plot"), this);
   Q_CHECK_PTR(pb_update);
   pb_update->setEnabled(false);
   pb_update->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_update->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_update, SIGNAL(clicked()), SLOT(update()));

   pb_load = new QPushButton(tr("Load MWL Data"), this);
   Q_CHECK_PTR(pb_load);
   pb_load->setAutoDefault(true);
   pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load, SIGNAL(clicked()), SLOT(load()));

   pb_model = new QPushButton(tr("Load MWL Model"), this);
   Q_CHECK_PTR(pb_model);
   pb_model->setAutoDefault(true);
   pb_model->setEnabled(false);
   pb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_model->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_model, SIGNAL(clicked()), SLOT(load_model()));

   lb_cell = new QListBox(this, "Cell");
   lb_cell->insertItem(tr("Cell 1"));
   lb_cell->insertItem(tr("Cell 2"));
   lb_cell->insertItem(tr("Cell 3"));
   lb_cell->insertItem(tr("Cell 4"));
   lb_cell->insertItem(tr("Cell 5"));
   lb_cell->insertItem(tr("Cell 6"));
   lb_cell->insertItem(tr("Cell 7"));
   lb_cell->insertItem(tr("Cell 8"));
   lb_cell->setSelected(current_cell, true);
   lb_cell->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_cell->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(lb_cell, SIGNAL(highlighted(int)), SLOT(select_cell(int)));

   lb_channel = new QListBox(this, "Channel");
   lb_channel->insertItem(tr("Channel 1"));
   lb_channel->insertItem(tr("Channel 2"));
   lb_channel->insertItem(tr("Channel 3"));
   lb_channel->insertItem(tr("Channel 4"));
   lb_channel->insertItem(tr("Channel 5"));
   lb_channel->insertItem(tr("Channel 6"));
   lb_channel->insertItem(tr("Channel 7"));
   lb_channel->insertItem(tr("Channel 8"));
   lb_channel->setSelected(current_channel, true);
   lb_channel->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_channel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(lb_channel, SIGNAL(highlighted(int)), SLOT(select_channel(int)));

   plot_2d = new QwtPlot(this);
   plot_2d->enableOutline(true);
   plot_2d->setOutlinePen(white);
   plot_2d->setOutlineStyle(Qwt::VLine);
   plot_2d->enableGridXMin();
   plot_2d->enableGridYMin();
   plot_2d->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   plot_2d->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_2d->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   plot_2d->setAxisTitle(QwtPlot::xBottom, tr("Radius (in cm)"));
   plot_2d->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_2d->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_2d->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_2d->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_2d->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_2d->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_2d->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_2d->setMargin(USglobal->config_list.margin);
   plot_2d->setTitle(tr("Experimental Data"));
   plot_2d->setCanvasBackground(USglobal->global_colors.plot);      //new version
   /*
     connect(plot_2d, SIGNAL(plotMouseReleased(const QMouseEvent &)),
     SLOT(get_x(const QMouseEvent &)));
     connect(plot_2d, SIGNAL(plotMousePressed(const QMouseEvent &)),
     SLOT(getMousePressed(const QMouseEvent &)));
   */
   plot_residual = new QwtPlot(this);
   plot_residual->enableOutline(true);
   plot_residual->setOutlinePen(white);
   plot_residual->setOutlineStyle(Qwt::VLine);
   plot_residual->enableGridXMin();
   plot_residual->enableGridYMin();
   plot_residual->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   plot_residual->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_residual->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   plot_residual->setAxisTitle(QwtPlot::xBottom, tr("Radius (in cm)"));
   plot_residual->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_residual->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_residual->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_residual->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_residual->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_residual->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_residual->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_residual->setMargin(USglobal->config_list.margin);
   plot_residual->setTitle(tr("Residuals"));
   plot_residual->setCanvasBackground(USglobal->global_colors.plot);      //new version

   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);

   setup_GUI();
}

US_ViewMWL::~US_ViewMWL()
{
}

void US_ViewMWL::set_model()
{
   show_model = cb_model->isChecked();
}

void US_ViewMWL::load_model()
{
   unsigned int i, j, count;
   QString s = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*-model*.dat",
                                            this, "open file dialog", "Please select a Model" );
   if (s.isEmpty())
   {
      return;
   }
   int pos = s.find("-model-", 0, false);
   US_FemGlobal *fg;
   fg = new US_FemGlobal(this);
   model_vector.clear();
   count = fg->read_mwl_model_data(&model_vector, s.left(pos));
   delete fg;
   if (count > 0)
   {
      cb_model->setEnabled(true);
      lambda_flag.resize(cell_data.cell_channel[0].data[0].scan.size());
      for (i=0; i<lambda_flag.size(); i++)
      {
         lambda_flag[i] = -1;
      }
      for (i=0; i<model_vector.size(); i++)
      {
         for (j=0; j<cell_data.cell_channel[0].data[0].scan.size(); j++)
         {
            if(model_vector[i].wavelength == j)
            { // identify the wavelength index for which corresponding model data exists
               lambda_flag[j] = i;
               cout << cell_data.cell_channel[0].data[0].scan[j].wavelength << endl;
            }
         }
      }
   }
}

void US_ViewMWL::setLoading()
{
   if (loading)
   {
      loading = false;
      pb_load->setText(tr("Cancelling..."));
   }
}

void US_ViewMWL::load()
{
   min_od = 4.0;
   max_od = -4.0;
   struct channel channel_data;
   if (file_format) //Load ASCII
   {
      QString filter, trash, str1, str2, filename;
      unsigned int i, j, columns, rows;
      QStringList sl;
      float fval;
      QFile f;
      vector <unsigned int> error;
      channel_data.data.clear();
      cell_data.cell_channel.clear();
      struct time_point timepoint_data;
      QTime t;
      t.start();
      filter = str1.sprintf("F*.??%d f*.??%d", current_cell+1, current_cell+1);

      // allow multiple files to be selected:

      channel_data.channel_number = current_channel + 1;
      channel_data.measurement_mode = measurement_mode;
      cell_data.cell_number = current_cell + 1;
      cell_data.centerpiece = 1; //default setting for now, later it will be user-selectable.

      sl = QFileDialog::getOpenFileNames(filter, USglobal->config_list.data_dir, 0, 0);
      if (sl.empty())
      {
         return;
      }
      progress->reset();
      progress->setTotalSteps(sl.size());
      i = 1;
      pb_load->setText(tr("Stop Loading"));
      pb_load->disconnect();
      connect(pb_load, SIGNAL(clicked()), SLOT(setLoading()));
      for (QStringList::Iterator it=sl.begin(); it!=sl.end(); it++)
      {
         if (!loading)
         {
            loading = true;
            progress->reset();
            pb_load->disconnect();
            pb_load->setText(tr("Load MWL Data"));
            connect(pb_load, SIGNAL(clicked()), SLOT(load()));
            break;
         }
         timepoint_data.scan.clear();
         filename = *it;
         j = filename.findRev("/");
         filename = filename.right(filename.length() - j - 1);
         f.setName(*it);
         if (f.open(IO_ReadOnly))
         {
            QTextStream ts(&f);
            struct radial_scan rscan;
            channel_data.contents = ts.readLine();
            ts >> columns; // number of columns
            ts >> trash;   // arbitrary number
            ts >> rows;    // number of rows
            ts >> trash;   // absorbance mode
            ts >> trash;   // number of averages
            ts >> timepoint_data.rotor_speed;
            ts >> timepoint_data.time;
            cout << "Time: " << timepoint_data.time << endl;
            ts >> trash;   // arbitrary number
            ts >> timepoint_data.omega_2_t;
            ts >> timepoint_data.temperature;
            ts >> trash;   // arbitrary number in the first position of radius vector line
            timepoint_data.radius.clear();
            for (j=0; j<columns; j++)
            {
               ts >> fval;
               timepoint_data.radius.push_back(fval); // offset + 0.06);
            }
            while (!ts.eof())
            {
               ts >> fval;
               if (ts.eof())
               {
                  break;
               }
               lbl_update->setText(tr("Loading File: ") + filename + str1.sprintf(", Lambda: %4.1f nm", fval));
               qApp->processEvents();
               rscan.wavelength = fval;
               error.clear();
               rscan.absorbance.clear();
               for (j=0; j<columns; j++)
               {
                  ts >> str1;
                  if (str1 == "NaN")
                  {
                     error.push_back(j);
                     rscan.absorbance.push_back(0); //just push something at this location, fix later.
                  }
                  else
                  {
                     fval = str1.toFloat();
                     if (fval > 3.27) //make sure it fits into a short
                     {
                        fval = (float) 3.27;
                     }
                     if (fval < -3.27)
                     {
                        fval = (float) -3.27;
                     }
                     max_od = max(max_od, fval);
                     min_od = min(min_od, fval);
                     rscan.absorbance.push_back((short int) (fval * 10000));
                  }
               }
               for (j=0; j<error.size(); j++) // now fix up the NaN values:
               {
                  if(error[j] == 0) // first value is a NaN
                  {
                     rscan.absorbance[0] = rscan.absorbance[1]; // use the next value in the array.
                  }
                  else if(error[j] == rscan.absorbance.size()-1) // last value is a NaN
                  {
                     rscan.absorbance[error[j]] = rscan.absorbance[rscan.absorbance.size()-2]; //use the previous value in the array.
                  }
                  if(error[j] > 0 && error[j] < rscan.absorbance.size()-1) // value is somewhere in the middle and we can interpolate
                  {
                     rscan.absorbance[error[j]] = (short int) ((rscan.absorbance[error[j]-1] + rscan.absorbance[error[j]+1])/2) ; //use the next value in the array.
                  }
               }
               timepoint_data.scan.push_back(rscan);
            }
            f.close();
            channel_data.data.push_back(timepoint_data);
         }
         progress->setProgress(i);
         i++;
      }
      cell_data.cell_channel.push_back(channel_data);
      unsigned int elapsed_time = t.elapsed();
      unsigned int minutes = (unsigned int) (elapsed_time/60000);
      elapsed_time -= minutes*60000;
      unsigned int seconds = (unsigned int) (elapsed_time/1000);
      QString elapsed;
      elapsed.sprintf("%d min and %d sec, with %d radius points and %d wavelengths",
                      minutes, seconds, (int) channel_data.data[0].radius.size(),
                      (int) channel_data.data[0].scan.size());
      if (channel_data.data.size() == 1)
      {
         lbl_update->setText(tr(str1.sprintf("Loaded %d file in ",
                                             (int) channel_data.data.size()) + elapsed));
      }
      else
      {
         lbl_update->setText(tr(str1.sprintf("Loaded %d files in ",
                                             (int) channel_data.data.size()) + elapsed));
      }
      pb_load->disconnect();
      pb_load->setText(tr("Load MWL Data"));
      connect(pb_load, SIGNAL(clicked()), SLOT(load()));
   }
   else // load binary
   {
      QString filter, str1, str2, filename;
      unsigned int i, j, k, scans, radius_points, wavelengths;
      filename = QFileDialog::getOpenFileName(USglobal->config_list.result_dir,
                                              "Multiwavelength Files (*.mwl)",
                                              this,
                                              "open file dialog",
                                              "Choose a file to open" );
      if (filename == "")
      {
         return;
      }
      QFile f;
      Q_UINT16 int16;
      Q_UINT32 int32;
      Q_INT16 signed_int16;
      float fval;
      channel_data.data.clear();
      cell_data.cell_channel.clear();
      struct time_point timepoint_data;
      struct radial_scan rscan;
      progress->reset();
      f.setName(filename);
      if (f.open(IO_ReadOnly))
      {
         QDataStream ds(&f);
         ds >> int16;
         cell_data.cell_number = int16;
         lb_cell->setSelected(cell_data.cell_number-1, true);
         ds >> int16;
         cell_data.centerpiece = int16;
         ds >> int16;
         channel_data.channel_number = int16;
         lb_channel->setSelected(channel_data.channel_number-1, true);
         ds >> channel_data.contents;
         ds >> int16;
         channel_data.measurement_mode = int16;
         ds >> fval;
         min_od = fval;
         ds >> fval;
         max_od = fval;
         ds >> int32;
         scans = int32;
         progress->setTotalSteps(scans);
         for (i=0; i<scans; i++)
         {
            timepoint_data.radius.clear();
            timepoint_data.scan.clear();
            ds >> timepoint_data.rotor_speed;
            ds >> timepoint_data.time;
            lbl_update->setText(tr(str1.sprintf("Loading scan at %ld seconds ", (long unsigned int) timepoint_data.time)));
            ds >> timepoint_data.omega_2_t;
            ds >> timepoint_data.temperature;
            ds >> int32;
            radius_points = int32;
            for (j=0; j<radius_points; j++)
            {
               ds >> fval;
               //cout << fval << endl;
               timepoint_data.radius.push_back(fval);
            }
            ds >> int32;
            wavelengths = int32;
            for (j=0; j<wavelengths; j++)
            {
               ds >> fval;
               rscan.wavelength = fval;
               rscan.absorbance.clear();
               for (k=0; k<radius_points; k++)
               {
                  ds >> signed_int16;
                  rscan.absorbance.push_back(signed_int16);
               }
               timepoint_data.scan.push_back(rscan);
            }
            channel_data.data.push_back(timepoint_data);
            progress->setProgress(i+1);
            qApp->processEvents();
         }
         cell_data.cell_channel.push_back(channel_data);
         f.close();
         lbl_update->setText(tr(str1.sprintf("Read %d scans from ",
                                             (int) cell_data.cell_channel[0].data.size())) + filename);
      }
   }
   cnt_min_time->setRange(1, (double) channel_data.data.size(), 1);
   cnt_min_time->setValue(1);
   min_time = 1;
   cnt_max_time->setRange(1, (double) channel_data.data.size(), 1);
   cnt_max_time->setValue((double) channel_data.data.size());
   max_time = channel_data.data.size();
   cnt_min_lambda->setRange((double) channel_data.data[0].scan[0].wavelength, (double) channel_data.data[0].scan[channel_data.data[0].scan.size()-1].wavelength, 0.1);
   cnt_min_lambda->setValue((double) channel_data.data[0].scan[0].wavelength);
   min_lambda = channel_data.data[0].scan[0].wavelength;
   cnt_max_lambda->setRange((double) channel_data.data[0].scan[0].wavelength, (double) channel_data.data[0].scan[channel_data.data[0].scan.size()-1].wavelength, 0.1);
   cnt_max_lambda->setValue((double) channel_data.data[0].scan[channel_data.data[0].scan.size()-1].wavelength);
   max_lambda = channel_data.data[0].scan[channel_data.data[0].scan.size()-1].wavelength;
   cnt_max_radius->setRange((double) channel_data.data[0].radius[0], (double) channel_data.data[0].radius[channel_data.data[0].radius.size()-1], 0.001);
   cnt_max_radius->setValue((double) channel_data.data[0].scan[channel_data.data[0].scan.size()-1].wavelength);
   max_radius = channel_data.data[0].radius[channel_data.data[0].radius.size()-1];
   cnt_min_radius->setRange((double) channel_data.data[0].radius[0], (double) channel_data.data[0].radius[channel_data.data[0].radius.size()-1], 0.001);
   cnt_min_radius->setValue((double) channel_data.data[0].radius[0]);
   min_radius = channel_data.data[0].radius[0];
   min_radius_element = 0;
   min_lambda_element = 0;
   max_radius_element = channel_data.data[0].radius.size()-1;
   max_lambda_element = channel_data.data[0].scan.size()-1;
   pb_update->setEnabled(true);
   pb_movie->setEnabled(true);
   pb_print->setEnabled(true);
   pb_export_data->setEnabled(true);
   pb_save->setEnabled(true);
   pb_model->setEnabled(true);
}

void US_ViewMWL::save()
{
   if(cell_data.cell_channel.size() == 0) // nothing is loaded yet
   {
      return;
   }
   QString run_name, str1, str2, message, filename;
   message = tr("Please enter a Run Identification\nfor the Multiwavelength Data:");
   OneLiner ol_descr(message);
   ol_descr.show();
   if (ol_descr.exec())
   {
      run_name = ol_descr.string;
   }
   QFile f;
   switch (cell_data.cell_channel[0].measurement_mode)
   {
   case 0:
      {
         str1 = "-a."; // absorbance mode
         break;
      }
   case 1:
      {
         str1 = "-i."; // intensity mode
         break;
      }
   }
   progress->reset();
   progress->setTotalSteps(cell_data.cell_channel[0].data[0].scan.size()*cell_data.cell_channel[0].data.size());
   unsigned int prog_count = 0;
   filename =USglobal->config_list.result_dir + "/" + run_name
      + str2.sprintf(".%d%d%d" + str1 + "mwl", cell_data.cell_number,
                     cell_data.centerpiece, cell_data.cell_channel[0].channel_number);
   f.setName(filename);
   if (f.open(IO_WriteOnly))
   {
      QDataStream ds(&f);
      unsigned int i, j, k;
      ds << (Q_UINT16) cell_data.cell_number;
      ds << (Q_UINT16) cell_data.centerpiece;
      ds << (Q_UINT16) cell_data.cell_channel[0].channel_number;
      ds << cell_data.cell_channel[0].contents;
      ds << (Q_INT16) cell_data.cell_channel[0].measurement_mode;
      ds << min_od;
      ds << max_od;
      ds << (Q_UINT32) cell_data.cell_channel[0].data.size();
      for (i=0; i<cell_data.cell_channel[0].data.size(); i++)
      {
         lbl_update->setText(tr(str1.sprintf("Writing scan %d to disk...", i+1)));
         qApp->processEvents();
         ds <<   cell_data.cell_channel[0].data[i].rotor_speed;                        // rotor speed at this time
         ds << cell_data.cell_channel[0].data[i].time;                               // seconds elapsed since the start of the expt.
         ds << cell_data.cell_channel[0].data[i].omega_2_t;                            // omega^2*t integral at this time
         ds << cell_data.cell_channel[0].data[i].temperature;                        // the temperature at this time
         ds << (Q_UINT32) cell_data.cell_channel[0].data[i].radius.size();
         for (j=0; j<cell_data.cell_channel[0].data[i].radius.size(); j++)
         {
            ds << cell_data.cell_channel[0].data[i].radius[j];
         }
         ds << (Q_UINT32) cell_data.cell_channel[0].data[i].scan.size();
         for (j=0; j<cell_data.cell_channel[0].data[i].scan.size(); j++)
         {
            qApp->processEvents();
            prog_count ++;
            progress->setProgress(prog_count);
            ds << cell_data.cell_channel[0].data[i].scan[j].wavelength;
            for (k=0; k<cell_data.cell_channel[0].data[i].radius.size(); k++)
            {
               ds << (Q_UINT16) cell_data.cell_channel[0].data[i].scan[j].absorbance[k];
            }
         }
      }
      f.close();
      lbl_update->setText(tr(str1.sprintf("Wrote %d scans to ",
                                          (int) cell_data.cell_channel[0].data.size())) + filename);
   }
}

void US_ViewMWL::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/viewmwl.html");
}

void US_ViewMWL::cancel()
{
   close();
}

void US_ViewMWL::movie()
{
   if(cell_data.cell_channel.size() == 0) // nothing is loaaded yet
   {
      return;
   }
   unsigned int i, j, k, l, m;
   QString str, imageName, imageDir="";
   QPixmap p;
   if (view_mode)
   {
      if(pngs)
      {
         imageDir = QFileDialog::getExistingDirectory(USglobal->config_list.data_dir, 0, 0,
                                                      tr("Please select or create a directory for the 3D-surface image files:"), true, true);
         if(imageDir.isEmpty())
         {
            pngs = false;
         }
      }
      progress->reset();
      progress->setTotalSteps(max_time - min_time + 1);
      j = 1;
      for (i=min_time; i<=max_time; i++)
      {
         update(i, imageDir);
         progress->setProgress(j);
         j++;
      }
   }
   else // 2D mode
   {
      progress->reset();
      plot_2d->setAxisScale(QwtPlot::yLeft, min_od, max_od, 0);
      double **data;
      double *x;
      unsigned int *curves, count;
      if(export_mode) // show wavelength data
      {
         if(pngs)
         {
            imageDir = QFileDialog::getExistingDirectory(USglobal->config_list.data_dir, 0, 0,
                                                         tr("Please select or create a directory for the wavelength image files:"), true, true);
            if(imageDir.isEmpty())
            {
               pngs = false;
            }
         }
         progress->setTotalSteps(max_radius_element - min_radius_element + 1);
         plot_2d->setAxisTitle(QwtPlot::xBottom, tr("Wavelength (in nm)"));
         curves = new unsigned int [max_time - min_time + 1];
         data = new double *[max_time - min_time + 1];
         unsigned int wavelengths = max_lambda_element - min_lambda_element + 1;
         x = new double [wavelengths];
         j=0;
         for (i=min_time; i<=max_time; i++)
         {
            data[j] = new double [wavelengths];
            j++;
         }
         count = 1;
         for (m=min_radius_element; m<=max_radius_element; m++)
         {
            progress->setProgress(count);
            plot_2d->clear();
            count++;
            lbl_update->setText(tr(str.sprintf("Plotting scans at %f cm", cell_data.cell_channel[0].data[0].radius[m])));
            uint wavelength_label = plot_2d->insertMarker();
            tr(str.sprintf("Radius: %5.3f cm", cell_data.cell_channel[0].data[0].radius[m]));
            plot_2d->setMarkerLabel(wavelength_label, str);
            plot_2d->setMarkerPos(wavelength_label, min_lambda + (max_lambda-min_lambda)/2, min_od + (max_od-min_od)/20);
            plot_2d->setMarkerFont(wavelength_label, QFont("Helvetica", 14, QFont::Bold));
            plot_2d->setMarkerPen(wavelength_label, QPen(cyan, 3, DashDotLine));
            j=0;
            for (i=min_time; i<=max_time; i++)
            {
               l=0;
               for (k=min_lambda_element; k<=max_lambda_element; k++)
               {
                  data[j][l] = cell_data.cell_channel[0].data[i-1].scan[k].absorbance[m]/10000.0;
                  l++;
               }
               j++;
            }
            l=0;
            for (k=min_lambda_element; k<=max_lambda_element; k++)
            {
               x[l] = cell_data.cell_channel[0].data[0].scan[k].wavelength;
               l++;
            }
            j=0;
            for (i=min_time; i<=max_time; i++)
            {
               curves[j] = plot_2d->insertCurve(tr(str.sprintf("Wavelength Scan %d", j)));
               plot_2d->setCurveStyle(curves[j], QwtCurve::Lines);
               plot_2d->setCurveData(curves[j], x, data[j], wavelengths);
               plot_2d->setCurvePen(curves[j], Qt::yellow);
               j++;
            }
            plot_2d->replot();
            if (pngs)
            {
               imageName.sprintf(imageDir + "/wavelength%05d.", m);
               p = QPixmap::grabWidget(plot_2d, 2, 2, plot_2d->width() - 4, plot_2d->height() - 4);
               pm->save_file(imageName, p);
            }
            qApp->processEvents();
         }
         delete [] curves;
         delete [] x;
         j=0;
         for (i=min_time; i<=max_time; i++)
         {
            delete [] data[j];
            j++;
         }
         delete [] data;
      }
      else // show radial data
      {
         if(pngs)
         {
            imageDir = QFileDialog::getExistingDirectory(USglobal->config_list.data_dir, 0, 0,
                                                         tr("Please select or create a directory for the radial image files:"), true, true);
            if(imageDir.isEmpty())
            {
               pngs = false;
            }
         }
         progress->setTotalSteps(max_lambda_element - min_lambda_element + 1);
         plot_2d->setAxisTitle(QwtPlot::xBottom, tr("Radius (in cm)"));
         curves = new unsigned int [max_time - min_time + 1];
         data = new double *[max_time - min_time + 1];
         unsigned int radius_points = max_radius_element - min_radius_element + 1;
         x = new double [radius_points];
         j=0;
         for (i=min_time; i<=max_time; i++)
         {
            data[j] = new double [radius_points];
            j++;
         }
         count = 1;
         for (m=min_lambda_element; m<=max_lambda_element; m++)
         {
            plot_2d->clear();
            progress->setProgress(count);
            lbl_update->setText(tr(str.sprintf("Plotting scans at %6.2f nm", cell_data.cell_channel[0].data[0].scan[m].wavelength)));
            uint wavelength_label = plot_2d->insertMarker();
            tr(str.sprintf("Wavelength: %6.2f nm", cell_data.cell_channel[0].data[0].scan[m].wavelength));
            plot_2d->setMarkerLabel(wavelength_label, str);
            plot_2d->setMarkerPos(wavelength_label, min_radius + (max_radius-min_radius)/2, min_od + (max_od-min_od)/20);
            plot_2d->setMarkerFont(wavelength_label, QFont("Helvetica", 14, QFont::Bold));
            plot_2d->setMarkerPen(wavelength_label, QPen(cyan, 3, DashDotLine));
            j=0;
            for (i=min_time; i<=max_time; i++)
            {
               l=0;
               for (k=min_radius_element; k<=max_radius_element; k++)
               {
                  data[j][l] = cell_data.cell_channel[0].data[i-1].scan[m].absorbance[k]/10000.0;
                  l++;
               }
               j++;
            }
            l=0;
            for (k=min_radius_element; k<=max_radius_element; k++)
            {
               x[l] = cell_data.cell_channel[0].data[0].radius[k];
               l++;
            }
            j=0;
            for (i=min_time; i<=max_time; i++)
            {
               curves[j] = plot_2d->insertCurve(tr(str.sprintf("Radial Scan %d", j)));
               plot_2d->setCurveStyle(curves[j], QwtCurve::Lines);
               plot_2d->setCurveData(curves[j], x, data[j], radius_points);
               plot_2d->setCurvePen(curves[j], Qt::yellow);
               j++;
            }
            plot_2d->replot();
            if (pngs)
            {
               imageName.sprintf(imageDir + "/radius%05d.", m);
               p = QPixmap::grabWidget(plot_2d, 2, 2, plot_2d->width() - 4, plot_2d->height() - 4);
               pm->save_file(imageName, p);
            }
            qApp->processEvents();
         }
         delete [] curves;
         delete [] x;
         j=0;
         for (i=min_time; i<=max_time; i++)
         {
            delete [] data[j];
            j++;
         }
         delete [] data;
      }
   }
}

void US_ViewMWL::print()
{
   if(cell_data.cell_channel.size() == 0) // nothing is loaaded yet
   {
      return;
   }
   QPrinter printer;
   if   (printer.setup(0))
   {
      plot_2d->print(printer);
   }
}

void US_ViewMWL::export_data()
{
   QString fileName = QFileDialog::getExistingDirectory(USglobal->config_list.data_dir, 0, 0,
                                                        tr("Please select or create a directory for the simulated data files:"), true, true);
   if(fileName.isEmpty())
   {
      return;
   }
   if(export_mode) // export wavelength data
   {
      unsigned int i, j;
      QString str1, str2, str3;
      QFile f;
      progress->reset();
      progress->setTotalSteps(max_time);
      for (i=0; i<cell_data.cell_channel[0].data.size(); i++)
      {
         if (i < 9)
         {
            str2 = fileName + "/" + "0000" + str1.sprintf("%d", i+1) + str3.sprintf(".wa%d", cell_data.cell_number);
         }
         else if (i >= 9 && i < 99)
         {
            str2 = fileName + "/" + "000" + str1.sprintf("%d", i+1) + str3.sprintf(".wa%d", cell_data.cell_number);
         }
         else if (i >= 99 && i < 999)
         {
            str2 = fileName + "/" + "00" + str1.sprintf("%d", i+1) + str3.sprintf(".wa%d", cell_data.cell_number);
         }
         else if (i >= 999 && i < 9999)
         {
            str2 = fileName + "/" + "0" + str1.sprintf("%d", i+1) + str3.sprintf(".wa%d", cell_data.cell_number);
         }
         else if (i >= 9999 && i < 10000)
         {
            str2 = fileName + "/" + str1.sprintf("%d", i+1) + str3.sprintf(".wa%d", cell_data.cell_number);
         }
         else
         {
            return;
         }
         f.setName(str2);
         lbl_update->setText(tr(str1.sprintf("Saving wavelength scan %d to file " + str2, i+1 )));
         qApp->processEvents();
         if (f.open(IO_WriteOnly | IO_Translate))
         {
            QTextStream ts(&f);
            ts << cell_data.cell_channel[0].contents << endl;
            ts << str1.sprintf("W %d %4.1f %5.0f %7.0f %1.5e %5.3f %d\n",
                               cell_data.cell_number,
                               cell_data.cell_channel[0].data[i].temperature,
                               cell_data.cell_channel[0].data[i].rotor_speed,
                               cell_data.cell_channel[0].data[i].time,
                               cell_data.cell_channel[0].data[i].omega_2_t,
                               min_radius, 1);
            for (j=0; j<cell_data.cell_channel[0].data[i].scan.size(); j++)
            {
               ts << str1.sprintf("%9.4f %12.5e %12.5e",
                                  cell_data.cell_channel[0].data[i].scan[j].wavelength,
                                  cell_data.cell_channel[0].data[i].scan[j].absorbance[min_radius_element]/10000.0, 0.0)
                  << endl;
            }
         }
         progress->setProgress(i+1);
         qApp->processEvents();
         f.close();
      }
   }
   else
   {
      unsigned int i, j;
      QString str1, str2, str3;
      QFile f;
      progress->reset();
      progress->setTotalSteps(max_time);
      for (i=0; i<cell_data.cell_channel[0].data.size(); i++)
      {
         if (i < 9)
         {
            str2 = fileName + "/" + "0000" + str1.sprintf("%d", i+1) + str3.sprintf(".ra%d", cell_data.cell_number);
         }
         else if (i >= 9 && i < 99)
         {
            str2 = fileName + "/" + "000" + str1.sprintf("%d", i+1) + str3.sprintf(".ra%d", cell_data.cell_number);
         }
         else if (i >= 99 && i < 999)
         {
            str2 = fileName + "/" + "00" + str1.sprintf("%d", i+1) + str3.sprintf(".ra%d", cell_data.cell_number);
         }
         else if (i >= 999 && i < 9999)
         {
            str2 = fileName + "/" + "0" + str1.sprintf("%d", i+1) + str3.sprintf(".ra%d", cell_data.cell_number);
         }
         else if (i >= 9999 && i < 10000)
         {
            str2 = fileName + "/" + str1.sprintf("%d", i+1) + str3.sprintf(".ra%d", cell_data.cell_number);
         }
         else
         {
            return;
         }
         f.setName(str2);
         lbl_update->setText(tr(str1.sprintf("Saving radial scan %d to file " + str2, i+1 )));
         qApp->processEvents();

         if (f.open(IO_WriteOnly | IO_Translate))
         {
            QTextStream ts(&f);
            ts << cell_data.cell_channel[0].contents << endl;
            ts << str1.sprintf("R %d %4.1f %5.0f %7.0f %1.5e %d %d\n",
                               cell_data.cell_number,
                               cell_data.cell_channel[0].data[i].temperature,
                               cell_data.cell_channel[0].data[i].rotor_speed,
                               cell_data.cell_channel[0].data[i].time,
                               cell_data.cell_channel[0].data[i].omega_2_t,
                               (unsigned int) min_lambda, 1);
            for (j=0; j<cell_data.cell_channel[0].data[i].radius.size(); j++)
            {
               ts << str1.sprintf("%9.4f %12.5e %12.5e",
                                  cell_data.cell_channel[0].data[i].radius[j],
                                  cell_data.cell_channel[0].data[i].scan[min_lambda_element].absorbance[j]/10000.0, 0.0)
                  << endl;
            }
         }
         progress->setProgress(i+1);
         qApp->processEvents();
         f.close();
      }
   }
}

void US_ViewMWL::setup_GUI()
{

   int rows = 18, columns = 2, spacing = 2, j=0;

   QGridLayout *background = new QGridLayout(this, 2, 2, spacing);
   background->addWidget(plot_2d, 0, 1);
   background->addWidget(plot_residual, 1, 1);
   background->addWidget(lbl_update, 2, 1);

   QGridLayout *controlGrid = new QGridLayout(rows, columns, spacing);
   for (int i=0; i<rows; i++)
   {
      controlGrid->setRowSpacing(i, 26);
   }
   for (int i=0; i<4; i++)
   {
      controlGrid->setColStretch(i, 0);
   }
   controlGrid->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   controlGrid->addWidget(pb_load, j, 0);
   controlGrid->addWidget(pb_update, j, 1);
   j++;
   controlGrid->addWidget(pb_save, j, 0);
   controlGrid->addWidget(pb_export_data, j, 1);
   j++;
   controlGrid->addWidget(pb_movie, j, 0);
   controlGrid->addWidget(pb_print, j, 1);
   j++;
   controlGrid->addWidget(pb_help, j, 0);
   controlGrid->addWidget(pb_cancel, j, 1);
   j++;
   controlGrid->addWidget(pb_model, j, 0);
   controlGrid->addWidget(cb_model, j, 1);
   j++;
   controlGrid->addWidget(cb_binary, j, 0);
   controlGrid->addWidget(cb_ascii, j, 1);
   j++;
   controlGrid->addWidget(cb_set_wavelength, j, 0);
   controlGrid->addWidget(cb_set_radius, j, 1);
   j++;
   controlGrid->addWidget(cb_set_2d, j, 0);
   controlGrid->addWidget(cb_set_3d, j, 1);
   j++;
   controlGrid->addWidget(cb_intensity, j, 0);
   controlGrid->addWidget(cb_absorbance, j, 1);
   j++;
   controlGrid->addMultiCellWidget(cb_pngs, j, j, 0, 1);
   j++;
   controlGrid->addMultiCellWidget(lb_cell, j, j+2, 0, 0);
   controlGrid->addMultiCellWidget(lb_channel, j, j+2, 1, 1);
   j+=3;
   controlGrid->addWidget(lbl_min_lambda, j, 0);
   controlGrid->addWidget(cnt_min_lambda, j, 1);
   j++;
   controlGrid->addWidget(lbl_max_lambda, j, 0);
   controlGrid->addWidget(cnt_max_lambda, j, 1);
   j++;
   controlGrid->addWidget(lbl_min_radius, j, 0);
   controlGrid->addWidget(cnt_min_radius, j, 1);
   j++;
   controlGrid->addWidget(lbl_max_radius, j, 0);
   controlGrid->addWidget(cnt_max_radius, j, 1);
   j++;
   controlGrid->addWidget(lbl_min_time, j, 0);
   controlGrid->addWidget(cnt_min_time, j, 1);
   j++;
   controlGrid->addWidget(lbl_max_time, j, 0);
   controlGrid->addWidget(cnt_max_time, j, 1);
   j++;
   controlGrid->addWidget(lbl_average, j, 0);
   controlGrid->addWidget(cnt_average, j, 1);
   j++;
   controlGrid->addWidget(lbl_status, j, 0);
   controlGrid->addWidget(progress, j, 1);
   //controlGrid->setColStretch(1, 1);
   background->addMultiCellLayout(controlGrid, 0, 2, 0, 0);
   background->setColStretch(0,1);
   background->setColStretch(1,2);
   background->setColSpacing(0,350);
   background->setColSpacing(1,550);
}

void US_ViewMWL::closeEvent(QCloseEvent *e)
{
   /*
    *control_window_flag = false;
    if(widget3d_flag)
    {
    mainwindow->close();
    }
    global_Xpos -= 30;
    global_Ypos -= 30;
   */
   e->accept();
}

void US_ViewMWL::update_max_lambda(double val)
{
   max_lambda = (float) val;
   if (max_lambda < min_lambda)
   {
      min_lambda = max_lambda;
      cnt_min_lambda->setValue((double) min_lambda);
   }
   find_elements();
}

void US_ViewMWL::update_min_lambda(double val)
{
   min_lambda = (float) val;
   if (min_lambda > max_lambda)
   {
      max_lambda = min_lambda;
      cnt_max_lambda->setValue((double) max_lambda);
   }
   find_elements();
}

void US_ViewMWL::update_max_radius(double val)
{
   max_radius = (float) val;
   if (max_radius < min_radius)
   {
      min_radius = max_radius;
      cnt_min_radius->setValue((double) min_radius);
   }
   find_elements();
}

void US_ViewMWL::update_min_radius(double val)
{
   min_radius = (float) val;
   if (min_radius > max_radius)
   {
      max_radius = min_radius;
      cnt_max_radius->setValue((double) max_radius);
   }
   find_elements();
}

void US_ViewMWL::update_max_time(double val)
{
   max_time = (int) val;
   if (max_time < min_time)
   {
      min_time = max_time;
      cnt_min_time->setValue((double) min_time);
   }
}

void US_ViewMWL::update_min_time(double val)
{
   min_time = (int) val;
   if (min_time > max_time)
   {
      max_time = min_time;
      cnt_max_time->setValue((double) max_time);
   }
}

void US_ViewMWL::update_average(double val)
{
   average = (unsigned int) val;
}

void US_ViewMWL::select_cell(int cell)
{
   current_cell = cell;
}

void US_ViewMWL::select_channel(int channel)
{
   current_channel = channel;
}

void US_ViewMWL::set_pngs()
{
   if(pngs)
   {
      pngs = false;
   }
   else
   {
      pngs = true;
   }
}

void US_ViewMWL::set_absorbance()
{
   measurement_mode = 0; // export as radius files
   cb_intensity->setChecked(false);
   cb_absorbance->setChecked(true);
}

void US_ViewMWL::set_intensity()
{
   measurement_mode = 0; // export as radius files
   cb_absorbance->setChecked(false);
   cb_intensity->setChecked(true);
}

void US_ViewMWL::set_2d()
{
   view_mode = false; // export as radius files
   cb_set_3d->setChecked(false);
   cb_set_2d->setChecked(true);
}

void US_ViewMWL::set_3d()
{
   view_mode = true; // export as radius files
   cb_set_3d->setChecked(true);
   cb_set_2d->setChecked(false);
}

void US_ViewMWL::set_radius()
{
   export_mode = false; // export as radius files
   cb_set_wavelength->setChecked(false);
   cb_set_radius->setChecked(true);
}

void US_ViewMWL::set_wavelength()
{
   export_mode = true; // export as radius files
   cb_set_wavelength->setChecked(true);
   cb_set_radius->setChecked(false);
}

void US_ViewMWL::set_ascii()
{
   file_format = true; // load ascii format
   cb_ascii->setChecked(true);
   cb_binary->setChecked(false);
}

void US_ViewMWL::set_binary()
{
   file_format = false; // load binary data
   cb_ascii->setChecked(false);
   cb_binary->setChecked(true);
}

void US_ViewMWL::find_minmax_od()
{
   unsigned int i, j, k;
   long unsigned int l, count=1;
   min_od = 4.0;
   max_od = -4.0;
   progress->reset();
   l = cell_data.cell_channel[0].data.size() * cell_data.cell_channel[0].data[0].scan.size();
   lbl_update->setText(tr("Scanning Data for minimum and maximum OD"));
   progress->setTotalSteps(l);
   for (i=0; i<cell_data.cell_channel[0].data.size(); i++)
   {
      for (j=0; j<cell_data.cell_channel[0].data[i].scan.size(); j++)
      {
         for (k=0; k<cell_data.cell_channel[0].data[i].radius.size(); k++)
         {
            max_od = max(max_od, (float) (cell_data.cell_channel[0].data[i].scan[j].absorbance[k]/10000.0));
            min_od = min(min_od, (float) (cell_data.cell_channel[0].data[i].scan[j].absorbance[k]/10000.0));
         }
         count ++;
         progress->setProgress(count);
      }
      qApp->processEvents();
   }
   progress->setProgress(l);
}

void US_ViewMWL::find_elements()
{
   unsigned int i;
   if(cell_data.cell_channel.size() == 0) // nothing is loaaded yet
   {
      return;
   }
   i=0;
   while (i < cell_data.cell_channel[0].data[0].radius.size())
   {
      if (cell_data.cell_channel[0].data[0].radius[i] >= min_radius)
      {
         min_radius_element = i;
         break;
      }
      i++;
   }
   i=cell_data.cell_channel[0].data[0].radius.size()-1;
   while (i >= 1)
   {
      if (cell_data.cell_channel[0].data[0].radius[i] <= max_radius)
      {
         max_radius_element = i;
         break;
      }
      i--;
   }
   i=0;
   while (i < cell_data.cell_channel[0].data[0].scan.size())
   {
      if (cell_data.cell_channel[0].data[0].scan[i].wavelength >= min_lambda)
      {
         min_lambda_element = i;
         break;
      }
      i++;
   }
   i=cell_data.cell_channel[0].data[0].scan.size()-1;
   while (i >= 1)
   {
      if (cell_data.cell_channel[0].data[0].scan[i].wavelength <= max_lambda)
      {
         max_lambda_element = i;
         break;
      }
      i--;
   }
   //cout << "minr: " << min_radius_element << ", maxr: " << max_radius_element << ", minl: " << min_lambda_element << ", maxl: " << max_lambda_element << endl;
}

void US_ViewMWL::update()
{
   update((unsigned int) min_time);
}

void US_ViewMWL::update(unsigned int time)
{
   unsigned int i, j, k, l, m;
   QString xtitle, ytitle, ztitle, str;
   if (view_mode) // 3D version
   {
      if ( !QGLFormat::hasOpenGL() )
      {
         qWarning( "This system has no OpenGL support. Exiting." );
         return;
      }
      double **absorbance;
      unsigned int rows, columns;
      rows = max_radius_element - min_radius_element;
      columns = (unsigned int) ((max_lambda_element - min_lambda_element)/average);
      absorbance = new double * [rows];
      lbl_update->setText(tr(str.sprintf("Showing 3D surface scan %d", time)));
      qApp->processEvents();
      for (i=0; i<rows; i++)
      {
         absorbance[i] = new double [columns];
      }
      l = 0;
      for (i=min_radius_element; i<max_radius_element; i++)
      {
         m = 0;
         for (j=min_lambda_element; j<max_lambda_element; j += average)
         {
            absorbance[l][m] = 0.0;
            if (j == max_lambda_element - 1)
            {
               absorbance[l][m] += (double) cell_data.cell_channel[0].data[time-1].scan[j].absorbance[i] * 1.0e-4;
            }
            else
            {
               for (k=0; k<average; k++)
               {
                  absorbance[l][m] += (double) cell_data.cell_channel[0].data[time-1].scan[j + k].absorbance[i] * 1.0e-4;
                  //cout << "Abs[" << l << "][" << m << "]: " << absorbance[l][m] << endl;
               }
               absorbance[l][m] /= average;
            }
            m++;
            if (m == columns)
            {
               break;
            }
         }
         l++;
      }
      //      cout << "rows: " << rows << ", columns: " << columns << endl;
      controlvar_3d.maxx = max_lambda;
      controlvar_3d.minx = min_lambda;
      controlvar_3d.maxy = max_radius;
      controlvar_3d.miny = min_radius;
      controlvar_3d.meshx = rows;
      controlvar_3d.meshy = columns;

      xtitle = "Radius (cm)";
      ytitle = "Wavelength (nm)";
      ztitle = "Absorbance";


      if (widget3d_flag)
      {
         mainwindow->setParameters(xtitle, ytitle, ztitle, absorbance, &controlvar_3d);
         mainwindow->raise();
      }
      else
      {
         mainwindow = new Mesh2MainWindow(&widget3d_flag, xtitle, ytitle, ztitle, absorbance, &controlvar_3d);
         mainwindow->show();
      }
      QPixmap p;
      for (i=0; i<rows; i++)
      {
         delete [] absorbance[i];
      }
      delete [] absorbance;
   }
   else // 2D mode
   {
      double **data;
      double *x;
      unsigned int *curves;
      if(export_mode) // show wavelength data
      {
         plot_2d->setAxisTitle(QwtPlot::xBottom, tr("Wavelength (in nm)"));
         lbl_update->setText(tr(str.sprintf("Showing scans at %6.2f nm (%u)... ", min_lambda, min_lambda_element)));
         qApp->processEvents();
         curves = new unsigned int [max_time - min_time + 1];
         data = new double *[max_time - min_time + 1];
         unsigned int wavelengths = max_lambda_element - min_lambda_element + 1;
         x = new double [wavelengths];
         j=0;
         for (i=min_time; i<=max_time; i++)
         {
            data[j] = new double [wavelengths];
            j++;
         }
         j=0;
         for (i=min_time; i<=max_time; i++)
         {
            l=0;
            for (k=min_lambda_element; k<=max_lambda_element; k++)
            {
               data[j][l] = cell_data.cell_channel[0].data[i-1].scan[k].absorbance[min_radius_element]/10000.0;
               l++;
            }
            j++;
         }
         l=0;
         for (k=min_lambda_element; k<=max_lambda_element; k++)
         {
            x[l] = cell_data.cell_channel[0].data[0].scan[k].wavelength;
            l++;
         }
         plot_2d->clear();
         j=0;
         for (i=min_time; i<=max_time; i++)
         {
            curves[j] = plot_2d->insertCurve(tr(str.sprintf("Wavelength Scan %d", j)));
            plot_2d->setCurveStyle(curves[j], QwtCurve::Lines);
            plot_2d->setCurveData(curves[j], x, data[j], wavelengths);
            plot_2d->setCurvePen(curves[j], Qt::yellow);
            j++;
         }
         plot_2d->replot();
         delete [] curves;
         delete [] x;
         j=0;
         for (i=min_time; i<=max_time; i++)
         {
            delete [] data[j];
            j++;
         }
         delete [] data;
      }
      else // show radial data
      {
         plot_2d->setAxisTitle(QwtPlot::xBottom, tr("Radius (in cm)"));
         lbl_update->setText(tr(str.sprintf("Showing scans at %6.2f nm (%u)... ", min_lambda, min_lambda_element)));
         qApp->processEvents();
         curves = new unsigned int [max_time - min_time + 1];
         data = new double *[max_time - min_time + 1];
         unsigned int radius_points = max_radius_element - min_radius_element + 1;
         x = new double [radius_points];
         j=0;
         for (i=min_time; i<=max_time; i++)
         {
            data[j] = new double [radius_points];
            j++;
         }
         j=0;
         for (i=min_time; i<=max_time; i++)
         {
            l=0;
            for (k=min_radius_element; k<=max_radius_element; k++)
            {
               data[j][l] = cell_data.cell_channel[0].data[i-1].scan[min_lambda_element].absorbance[k]/10000.0;
               l++;
            }
            j++;
         }
         l=0;
         for (k=min_radius_element; k<=max_radius_element; k++)
         {
            x[l] = cell_data.cell_channel[0].data[0].radius[k];
            l++;
         }
         plot_2d->clear();
         j=0;
         for (i=min_time; i<=max_time; i++)
         {
            curves[j] = plot_2d->insertCurve(tr(str.sprintf("Radial Scan %d", j)));
            plot_2d->setCurveStyle(curves[j], QwtCurve::Lines);
            plot_2d->setCurveData(curves[j], x, data[j], radius_points);
            plot_2d->setCurvePen(curves[j], Qt::yellow);
            j++;
         }
         if(cb_model->isChecked() && lambda_flag[min_lambda_element] > -1)
         {
            double **model_data=NULL;
            double *model_x=NULL;
            unsigned int *model_curves=NULL;
            model_curves = new unsigned int [max_time - min_time + 1];
            model_data = new double *[max_time - min_time + 1];
            model_x = new double [model_vector[lambda_flag[min_lambda_element]].radius.size()];
            j=0;
            printf("min_time %u max_time %u\n", min_time, max_time);
            for (i=min_time; i<=max_time; i++)
            {
               model_data[j] = new double [model_vector[lambda_flag[min_lambda_element]].radius.size()];
               j++;
            }
            j=0;
            // cout << "minlambda: " << min_lambda_element << ", lambda flag: " << lambda_flag[min_lambda_element] << endl;
            for (i=min_time; i<=max_time; i++)
            {
               for (k=0; k<model_vector[lambda_flag[min_lambda_element]].radius.size(); k++)
               {
                  if(i - 1 < model_vector[lambda_flag[min_lambda_element]].scan.size()) 
                  {
                     model_data[j][k] = model_vector[lambda_flag[min_lambda_element]].scan[i-1].conc[k];
                     // cout << "Abs: " << model_vector[lambda_flag[min_lambda_element]].scan[i-1].conc[k] << endl;
                  } else {
                     model_data[j][k] = 0;
                  }
                  // cout << "Abs: " << model_data[j][k] << endl;
               }
               j++;
            }
            for (k=0; k<model_vector[lambda_flag[min_lambda_element]].radius.size(); k++)
            {
               model_x[k] = model_vector[lambda_flag[min_lambda_element]].radius[k];
            }
            j=0;
            for (i=min_time; i<=max_time; i++)
            {
               model_curves[j] = plot_2d->insertCurve(tr(str.sprintf("Model data Scan %d", j)));
               plot_2d->setCurveStyle(model_curves[j], QwtCurve::Lines);
               plot_2d->setCurveData(model_curves[j], model_x, model_data[j], model_vector[lambda_flag[min_lambda_element]].radius.size());
               plot_2d->setCurvePen(model_curves[j], Qt::red);
               j++;
            }
            plot_2d->replot();
            delete [] model_curves;
            delete [] model_x;
            j=0;
            for (i=min_time; i<=max_time; i++)
            {
               delete [] model_data[j];
               j++;
            }
            delete [] model_data;
         }
         plot_2d->replot();
         delete [] curves;
         delete [] x;
         j=0;
         for (i=min_time; i<=max_time; i++)
         {
            delete [] data[j];
            j++;
         }
         delete [] data;
      }
   }
}

void US_ViewMWL::update(unsigned int time, QString imageDir)
{
   unsigned int i, j, k, l, m;
   QString xtitle, ytitle, ztitle, str;
   QPixmap p;
   if ( !QGLFormat::hasOpenGL() )
   {
      qWarning( "This system has no OpenGL support. Exiting." );
      return;
   }
   double **absorbance;
   unsigned int rows, columns;
   rows = max_radius_element - min_radius_element;
   columns = (unsigned int) ((max_lambda_element - min_lambda_element)/average);
   absorbance = new double * [rows];
   lbl_update->setText(tr(str.sprintf("Showing scan %d... ", time)));
   qApp->processEvents();
   for (i=0; i<rows; i++)
   {
      absorbance[i] = new double [columns];
   }
   l = 0;
   for (i=min_radius_element; i<max_radius_element; i++)
   {
      m = 0;
      for (j=min_lambda_element; j<max_lambda_element; j += average)
      {
         absorbance[l][m] = 0.0;
         if (j == max_lambda_element - 1)
         {
            absorbance[l][m] += (double) cell_data.cell_channel[0].data[time-1].scan[j].absorbance[i] * 1.0e-4;
         }
         else
         {
            for (k=0; k<average; k++)
            {
               absorbance[l][m] += (double) cell_data.cell_channel[0].data[time-1].scan[j + k].absorbance[i] * 1.0e-4;
               //cout << "Abs[" << l << "][" << m << "]: " << absorbance[l][m] << endl;
            }
            absorbance[l][m] /= average;
         }
         m++;
         if (m == columns)
         {
            break;
         }
      }
      l++;
   }
   //      cout << "rows: " << rows << ", columns: " << columns << endl;
   controlvar_3d.maxx = max_lambda;
   controlvar_3d.minx = min_lambda;
   controlvar_3d.maxy = max_radius;
   controlvar_3d.miny = min_radius;
   controlvar_3d.zscaling = 1e-10;
   controlvar_3d.meshx = rows;
   controlvar_3d.meshy = columns;

   xtitle = "Radius (cm)";
   ytitle = "Wavelength (nm)";
   ztitle = "Absorbance";


   if (widget3d_flag)
   {
      mainwindow->setParameters(xtitle, ytitle, ztitle, absorbance, &controlvar_3d);
      mainwindow->raise();
   }
   else
   {
      mainwindow = new Mesh2MainWindow(&widget3d_flag, xtitle, ytitle, ztitle, absorbance, &controlvar_3d);
      mainwindow->show();
   }
   if (pngs)
   {
      str.sprintf(imageDir + "/3D-surface%05d.png", time);
      mainwindow->dumpImage(str, "PNG");
   }
   for (i=0; i<rows; i++)
   {
      delete [] absorbance[i];
   }
   delete [] absorbance;
}

