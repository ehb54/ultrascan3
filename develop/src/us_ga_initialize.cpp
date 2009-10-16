#include "../include/us_ga_initialize.h"

US_GA_Initialize::US_GA_Initialize(QWidget *p, const char *name) : QFrame( p, name)
{
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Genetic Algorithm Initialization Control Window"));
   QString str;
   distro_type = -1;
   dimension = 1;
   k_range = 0.0;
   s_range = 0.0;
   minmax = false;
   zoom = false;
   monte_carlo = false;
   autolimit = true;
   current_solute = 0;
   ff0_min = 1.0;
   ff0_max = 4.0;
   plot_fmin = 1.0;
   plot_fmax = 4.0;
   plot_smin = 1.0;
   plot_smax = 10.0;
   initial_solutes = 0;
   GA_Solute.clear();
   GA_Solute.resize(initial_solutes);
   MC_solute.clear();
   component.clear();
   resolution = 90.0;
   global_Xpos += 30;
   global_Ypos += 30;
   x_pixel = 2;
   y_pixel = 2;
   x_resolution = 300;
   y_resolution = 300;
   gradient.resize(510);
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
      gradient[k].setRgb(0, g, b);
   }

   setup_GUI();
   move(global_Xpos, global_Ypos);
}

US_GA_Initialize::~US_GA_Initialize()
{
}

void US_GA_Initialize::setup_GUI()
{
   unsigned int minHeight1=26;
   textwindow = new QTextEdit(this);
   textwindow->setReadOnly(true);
   textwindow->setText("Please load a sedimentation coefficient distribution to initialize the"
                       " genetic algorithm s-value range. The distribution should have a good"
                       " resolution over the sedimentation coefficients. This distribution will"
                       " be used to initialize all experiments used in the run, so the"
                       " distribution taken from the experiment with the highest speed is"
                       " probably the most appropriate distribution. You can use a distribution"
                       " from the van Holde - Weischet method, the C(s) method, or 2-D Spectrum"
                       " Analysis.");
   textwindow->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   lbl_info1 = new QLabel(tr("Genetic Algorithm Control Window"), this);
   Q_CHECK_PTR(lbl_info1);
   lbl_info1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info1->setAlignment(AlignCenter|AlignVCenter);
   lbl_info1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_1dim = new QCheckBox(tr("1-Dimensional Plot"),this);
   cb_1dim->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_1dim->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_1dim->setChecked(true);
   cb_1dim->setEnabled(false);
   cb_1dim->setMinimumHeight(minHeight1);
   connect(cb_1dim, SIGNAL(clicked()), SLOT(select_1dim()));

   cb_2dim = new QCheckBox(tr("2-Dimensional Plot"),this);
   cb_2dim->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_2dim->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_2dim->setChecked(false);
   cb_2dim->setEnabled(false);
   cb_2dim->setMinimumHeight(minHeight1);
   connect(cb_2dim, SIGNAL(clicked()), SLOT(select_2dim()));

   cb_3dim = new QCheckBox(tr("Pseudo 3-D Plot"),this);
   cb_3dim->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_3dim->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   cb_3dim->setChecked(false);
   cb_3dim->setEnabled(false);
   cb_3dim->setMinimumHeight(minHeight1);
   connect(cb_3dim, SIGNAL(clicked()), SLOT(select_3dim()));

   lbl_info2 = new QLabel(tr("Distribution Data"), this);
   Q_CHECK_PTR(lbl_info2);
   lbl_info2->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info2->setAlignment(AlignCenter|AlignVCenter);
   lbl_info2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_info3 = new QLabel(tr("Pseudo-3D Controls"), this);
   Q_CHECK_PTR(lbl_info3);
   lbl_info3->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info3->setAlignment(AlignCenter|AlignVCenter);
   lbl_info3->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_k_range = new QLabel(tr(" Width of f/f0 bucket: "), this);
   Q_CHECK_PTR(lbl_k_range);
   lbl_k_range->setAlignment(AlignLeft|AlignVCenter);
   lbl_k_range->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_k_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_k_range= new QwtCounter(this);
   Q_CHECK_PTR(cnt_k_range);
   cnt_k_range->setRange(0, 1, 1);
   cnt_k_range->setValue(k_range);
   cnt_k_range->setEnabled(false);
   cnt_k_range->setNumButtons(3);
   cnt_k_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_k_range, SIGNAL(valueChanged(double)), SLOT(update_k_range(double)));

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

   lbl_s_range = new QLabel(tr(" Width of s bucket: "), this);
   Q_CHECK_PTR(lbl_s_range);
   lbl_s_range->setAlignment(AlignLeft|AlignVCenter);
   lbl_s_range->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_s_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_s_range= new QwtCounter(this);
   Q_CHECK_PTR(cnt_s_range);
   cnt_s_range->setRange(0, 1, 1);
   cnt_s_range->setValue(s_range);
   cnt_s_range->setNumButtons(3);
   cnt_s_range->setEnabled(false);
   cnt_s_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_s_range, SIGNAL(valueChanged(double)), SLOT(update_s_range(double)));

   lbl_initial_solutes = new QLabel(tr(" Number of initial Solutes: "), this);
   Q_CHECK_PTR(lbl_initial_solutes);
   lbl_initial_solutes->setAlignment(AlignLeft|AlignVCenter);
   lbl_initial_solutes->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_initial_solutes->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_initial_solutes= new QwtCounter(this);
   Q_CHECK_PTR(cnt_initial_solutes);
   cnt_initial_solutes->setRange(0, 1000, 1);
   cnt_initial_solutes->setValue(initial_solutes);
   cnt_initial_solutes->setNumButtons(3);
   cnt_initial_solutes->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_initial_solutes, SIGNAL(valueChanged(double)), SLOT(update_initial_solutes(double)));

   lbl_ff0_min = new QLabel(tr(" f/f0 minimum: "), this);
   Q_CHECK_PTR(lbl_ff0_min);
   lbl_ff0_min->setAlignment(AlignLeft|AlignVCenter);
   lbl_ff0_min->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_ff0_min->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_ff0_min= new QwtCounter(this);
   Q_CHECK_PTR(cnt_ff0_min);
   cnt_ff0_min->setRange(0, 49, 0.01);
   cnt_ff0_min->setValue(ff0_min);
   cnt_ff0_min->setNumButtons(3);
   cnt_ff0_min->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_ff0_min, SIGNAL(valueChanged(double)), SLOT(update_ff0_min(double)));

   lbl_ff0_max = new QLabel(tr(" f/f0 maximum: "), this);
   Q_CHECK_PTR(lbl_ff0_max);
   lbl_ff0_max->setAlignment(AlignLeft|AlignVCenter);
   lbl_ff0_max->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_ff0_max->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_ff0_max= new QwtCounter(this);
   Q_CHECK_PTR(cnt_ff0_max);
   cnt_ff0_max->setRange(1, 50, 0.01);
   cnt_ff0_max->setValue(ff0_max);
   cnt_ff0_max->setNumButtons(3);
   cnt_ff0_max->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_ff0_max, SIGNAL(valueChanged(double)), SLOT(update_ff0_max(double)));

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

   pb_draw = new QPushButton(tr(" Manually Draw Bins "), this);
   Q_CHECK_PTR(pb_draw);
   pb_draw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_draw->setEnabled(false);
   pb_draw->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_draw->setAutoDefault(false);
   connect(pb_draw, SIGNAL(clicked()), SLOT(draw()));

   pb_shrink = new QPushButton(tr(" Shrink Bins "), this);
   Q_CHECK_PTR(pb_shrink);
   pb_shrink->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_shrink->setEnabled(false);
   pb_shrink->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_shrink->setAutoDefault(false);
   connect(pb_shrink, SIGNAL(clicked()), SLOT(shrink()));

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

   pb_replot3d = new QPushButton(tr("Refresh Pseudo-3D Plot"), this);
   Q_CHECK_PTR(pb_replot3d);
   pb_replot3d->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_replot3d->setEnabled(true);
   pb_replot3d->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_replot3d->setAutoDefault(false);
   pb_replot3d->setEnabled(false);
   connect(pb_replot3d, SIGNAL(clicked()), SLOT(plot_3dim()));

   pb_reset_peaks = new QPushButton(tr(" Reset Solute Bins "), this);
   Q_CHECK_PTR(pb_reset_peaks);
   pb_reset_peaks->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_reset_peaks->setEnabled(false);
   pb_reset_peaks->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_reset_peaks->setAutoDefault(false);
   connect(pb_reset_peaks, SIGNAL(clicked()), SLOT(reset_peaks_replot()));

   pb_assign_peaks = new QPushButton(tr(" Autoassign Solute Bins "), this);
   Q_CHECK_PTR(pb_assign_peaks);
   pb_assign_peaks->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_assign_peaks->setEnabled(false);
   pb_assign_peaks->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_assign_peaks->setAutoDefault(false);
   connect(pb_assign_peaks, SIGNAL(clicked()), SLOT(assign_peaks()));

   pb_save = new QPushButton(tr(" Save "), this);
   Q_CHECK_PTR(pb_save);
   pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_save->setEnabled(false);
   pb_save->setAutoDefault(false);
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));

   pb_close = new QPushButton(tr(" Close "), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_close->setAutoDefault(false);
   connect(pb_close, SIGNAL(clicked()), SLOT(close()));

   lb_solutes = new QListBox(this, "Solutes");
   //   lb_solutes->setMinimumHeight(350);
   lb_solutes->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_solutes->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(lb_solutes, SIGNAL(selected(int)), SLOT(edit_solute(int)));
   connect(lb_solutes, SIGNAL(highlighted(int)), SLOT(highlight_solute(int)));

   plot = new QwtPlot(this);
   Q_CHECK_PTR(plot);
   plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   plot->enableGridXMin();
   plot->enableGridYMin();
   plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   plot->setCanvasBackground(USglobal->global_colors.plot);      //new version
   plot->enableOutline(true);
   plot->setAxisTitle(QwtPlot::xBottom, "s value");
   plot->setAxisTitle(QwtPlot::yLeft, "Frequency");
   plot->setTitle(tr("Distribution Data"));
   plot->setOutlinePen(white);
   plot->setOutlineStyle(Qwt::Cross);
   plot->setMinimumSize(550,300);
   plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot->setMargin(USglobal->config_list.margin);
   connect(plot, SIGNAL(plotMouseReleased(const QMouseEvent &)),
           SLOT(getMouseReleased(const QMouseEvent &)));
   connect(plot, SIGNAL(plotMousePressed(const QMouseEvent &)),
           SLOT(getMousePressed(const QMouseEvent &)));

   progress = new QProgressBar(this, "Progress Bar");
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   int rows=9, columns = 2, spacing = 2, j=0;
   QGridLayout *background=new QGridLayout(this, 3, 2, 4, 2);
   background->setRowSpacing(0, 40);
   background->addWidget(lbl_info1, 0, 0);
   background->addWidget(lbl_info2, 0, 1);
   QGridLayout *controlGrid = new QGridLayout(rows, columns, spacing, 0);

   for (int i=0; i<3; i++)
   {
      controlGrid->setRowSpacing(i, 26);
   }
   for (int i=4; i<7; i++)
   {
      controlGrid->setRowSpacing(i, 26);
   }
   controlGrid->setColSpacing(0, 120);
   controlGrid->setColSpacing(1, 120);

   controlGrid->addWidget(lbl_initial_solutes, j, 0);
   controlGrid->addWidget(cnt_initial_solutes, j, 1);
   controlGrid->setRowStretch(j, 0);
   j++;
   controlGrid->addWidget(lbl_ff0_min, j, 0);
   controlGrid->addWidget(cnt_ff0_min, j, 1);
   controlGrid->setRowStretch(j, 0);
   j++;
   controlGrid->addWidget(lbl_ff0_max, j, 0);
   controlGrid->addWidget(cnt_ff0_max, j, 1);
   controlGrid->setRowStretch(j, 0);
   j++;
   controlGrid->addWidget(lbl_s_range, j, 0);
   controlGrid->addWidget(cnt_s_range, j, 1);
   controlGrid->setRowStretch(j, 0);
   j++;
   controlGrid->addWidget(lbl_k_range, j, 0);
   controlGrid->addWidget(cnt_k_range, j, 1);
   controlGrid->setRowStretch(j, 0);
   j++;
   controlGrid->addMultiCellWidget(lbl_info3, j, j, 0, 1);
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
   controlGrid->addWidget(pb_color, j, 0);
   controlGrid->addWidget(pb_replot3d, j, 1);
   controlGrid->setRowStretch(j, 0);
   j++;
   controlGrid->addMultiCellWidget(lb_solutes, j, j, 0, 1);
   controlGrid->setRowStretch(j, 1);
   j++;
   controlGrid->addWidget(pb_help, j, 0);
   controlGrid->addWidget(cb_1dim, j, 1);
   controlGrid->setRowStretch(j, 0);
   j++;
   controlGrid->addWidget(pb_print, j, 0);
   controlGrid->addWidget(cb_2dim, j, 1);
   controlGrid->setRowStretch(j, 0);
   j++;
   controlGrid->addWidget(pb_load_distro, j, 0);
   controlGrid->addWidget(cb_3dim, j, 1);
   controlGrid->setRowStretch(j, 0);
   j++;
   controlGrid->addWidget(pb_draw, j, 0);
   controlGrid->addWidget(pb_shrink, j, 1);
   controlGrid->setRowStretch(j, 0);
   j++;
   controlGrid->addWidget(pb_assign_peaks, j, 0);
   controlGrid->addWidget(pb_reset_peaks, j, 1);
   controlGrid->setRowStretch(j, 0);
   j++;
   controlGrid->addWidget(pb_close, j, 0);
   controlGrid->addWidget(pb_save, j, 1);
   j++;
   controlGrid->addMultiCellWidget(progress, j, j, 0, 1);
   controlGrid->setRowStretch(j, 0);

   background->addMultiCellLayout(controlGrid, 1, 2, 0, 0);
   background->addWidget(plot, 1, 1);
   background->setRowStretch(1, 3);
   background->addWidget(textwindow, 2, 1);
   background->setRowStretch(2, 1);
   background->setColStretch(0, 4);
   background->setColStretch(1, 8);
}

void US_GA_Initialize::draw()
{
   zoom = true;
   GA_Solute.clear();
   lb_solutes->clear();
   initial_solutes = 0;
   cnt_initial_solutes->disconnect();
   cnt_initial_solutes->setValue(initial_solutes);
   connect(cnt_initial_solutes, SIGNAL(valueChanged(double)), SLOT(update_initial_solutes(double)));
   plot->setOutlineStyle(Qwt::Rect);
}

// shrink the bucket to the minmax s/ff0-values of all contained solutes
void US_GA_Initialize::shrink()
{
   QString str;
   if (GA_Solute.size() < 1)
   {
      QMessageBox::warning(this, tr("UltraScan Warning"),
                           tr("Please note:\n\nPlease select at least one initial solute!"),
                           QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      return;
   }
   unsigned i, j;
   double minval_s, maxval_s;
   double minval_ff0, maxval_ff0;
   plot->enableGridXMin(true);
   plot->enableGridYMin(true);
   plot->enableGridX(true);
   plot->enableGridY(true);
   //list <struct Solute>::iterator sol_it;
   list <Solute>::iterator sol_it;
   vector <struct SimulationComponent>::iterator comp_it;
   initial_solutes = GA_Solute.size();
   cnt_initial_solutes->disconnect();
   cnt_initial_solutes->setValue(initial_solutes);
   connect(cnt_initial_solutes, SIGNAL(valueChanged(double)), SLOT(update_initial_solutes(double)));
   lb_solutes->clear();
   MC_solute.clear();
   MC_solute.resize(GA_Solute.size());
   /*
     cout << "GA_smin: " << GA_Solute[0].s_min;
     cout << ", GA_smax: " << GA_Solute[0].s_max;
     cout << ", GA_fmin: " << GA_Solute[0].ff0_min;
     cout << ", GA_fmax: " << GA_Solute[0].ff0_max << endl;
   */
   for (i=0; i<GA_Solute.size(); i++)
   {
      minval_s = 1.0e200;
      maxval_s = -1.0e200;
      minval_ff0 = 1.0e200;
      maxval_ff0 = -1.0e200;
      j=0;
      for (sol_it = distro_solute.begin(); sol_it != distro_solute.end(); sol_it++)
      {
         if ((*sol_it).s >= GA_Solute[i].s_min && (*sol_it).s <= GA_Solute[i].s_max
             &&  (*sol_it).k >= GA_Solute[i].ff0_min && (*sol_it).k <= GA_Solute[i].ff0_max)
         {
            minval_s = min((*sol_it).s, minval_s);
            maxval_s = max((*sol_it).s, maxval_s);
            minval_ff0 = min((*sol_it).k, minval_ff0);
            maxval_ff0 = max((*sol_it).k, maxval_ff0);
            j++; // count how many solutes are in this bin
         }
      }
      /*      cout << "GA_smin2: " << GA_Solute[0].s_min;
              cout << ", GA_smax2: " << GA_Solute[0].s_max;
              cout << ", GA_fmin2: " << GA_Solute[0].ff0_min;
              cout << ", GA_fmax2: " << GA_Solute[0].ff0_max << endl;
      */
      MC_solute[i].clear();
      bool flag=false;
      for (comp_it = component.begin(); comp_it != component.end(); comp_it++)
      {
         flag = false;
         if ((*comp_it).s * 1.0e13 >= GA_Solute[i].s_min && (*comp_it).s * 1.0e13 <= GA_Solute[i].s_max
             &&  (*comp_it).f_f0 >= GA_Solute[i].ff0_min && (*comp_it).f_f0 <= GA_Solute[i].ff0_max)
         {
            //            cout << "inside: " << (*comp_it).s << ", " << (*comp_it).concentration << ", " << (*comp_it).D << ", " << (*comp_it).f_f0 << endl;
            MC_solute[i].push_back(*comp_it);
         }
      }
      if (MC_solute[i].size() == 0)
      {
         QMessageBox::warning(this, tr("UltraScan Warning"),
                              tr("Please note:\n\nBin #") + str.sprintf("%d", i+1) +
                              tr(" appears to be empty.\n\nPlease start over and redo the bin selection..."),
                              QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
         MC_solute[i].clear();
         GA_Solute.clear();
         lb_solutes->clear();
         initial_solutes = 0;
         cnt_initial_solutes->disconnect();
         cnt_initial_solutes->setValue(initial_solutes);
         connect(cnt_initial_solutes, SIGNAL(valueChanged(double)), SLOT(update_initial_solutes(double)));
         plot->clear();
         plot->replot();
         pb_shrink->setEnabled(false);
         return;
      }
      // create a border around each bucket so that all solutes are within the range

      minval_s -= s_range/2.0;
      maxval_s += s_range/2.0;
      minval_ff0 -= k_range/2.0;
      maxval_ff0 += k_range/2.0;
      if (minval_s < 0.01)
      {
         minval_s = 0.01;
      }
      if (minval_ff0 < 1.0)
      {
         minval_ff0 = 1.0;
      }
      GA_Solute[i].s_min = minval_s;
      GA_Solute[i].s_max = maxval_s;
      GA_Solute[i].ff0_min = minval_ff0;
      GA_Solute[i].ff0_max = maxval_ff0;
      str.sprintf("Solute Bin %d: s_min=%4.2e, s_max=%4.2e, f/f0_min=%4.2f, f/f0_max=%4.2f", i+1,
                  GA_Solute[i].s_min,
                  GA_Solute[i].s_max,
                  GA_Solute[i].ff0_min,
                  GA_Solute[i].ff0_max);
      lb_solutes->insertItem(str);
   }
   QwtSymbol symbol;
   symbol.setStyle(QwtSymbol::Ellipse);
   symbol.setPen(Qt::red);
   symbol.setBrush(Qt::yellow);
   if (distro_solute.size() < 100 && distro_solute.size() > 50)
   {
      symbol.setSize(8);
   }
   else if (distro_solute.size() < 50  && distro_solute.size() > 20)
   {
      symbol.setSize(10);
   }
   else if (distro_solute.size() < 21)
   {
      symbol.setSize(12);
   }
   else if (distro_solute.size() > 100)
   {
      symbol.setSize(6);
   }
   double *x, *y;
   x = new double [distro_solute.size()];
   y = new double [distro_solute.size()];
   j=0;
   for (sol_it = distro_solute.begin(); sol_it != distro_solute.end(); sol_it++)
   {
      x[j] = (*sol_it).s;
      y[j] = (*sol_it).k;
      j++;
   }
   plot->clear();
   plot->setTitle(tr("f/f0 vs Sed. Coeff."));
   plot->setAxisTitle(QwtPlot::xBottom, tr("s20,W (cm^2/sec)"));
   plot->setAxisTitle(QwtPlot::yLeft, tr("Frictional Ratio f/f0"));
   plot->setAxisAutoScale(QwtPlot::xBottom);
   plot->setAxisAutoScale(QwtPlot::yLeft);
   //      plot->setAxisScale(QwtPlot::yLeft, 0, max_y);
   long distro = plot->insertCurve("Solute Distribution");
   plot->setCurveStyle(distro, QwtCurve::NoCurve);
   plot->setCurveSymbol(distro, symbol);
   //      plot->setCurvePen(distro, QPen(Qt::yellow, 2, SolidLine));
   plot->setCurveData(distro, x, y, distro_solute.size());
   plot->replot();
   delete [] x;
   delete [] y;
   highlight_solute(GA_Solute.size() - 1, false);
   pb_reset_peaks->setEnabled(true);
   pb_save->setEnabled(true);
}

void US_GA_Initialize::load_distro()
{
   QFile f;
   unsigned int index, i;
   float temp1, temp2, temp3;
   QString filename, str;
   cb_1dim->setEnabled(false);
   cb_2dim->setEnabled(false);
   cb_3dim->setEnabled(false);
   plot->enableGridXMin(true);
   plot->enableGridYMin(true);
   plot->enableGridX(true);
   plot->enableGridY(true);
   plot->setAxisAutoScale(QwtPlot::xBottom);
   plot->setAxisAutoScale(QwtPlot::yLeft);
   lb_solutes->clear();
   initial_solutes = 0;
   cnt_initial_solutes->disconnect();
   cnt_initial_solutes->setValue(initial_solutes);
   connect(cnt_initial_solutes, SIGNAL(valueChanged(double)), SLOT(update_initial_solutes(double)));
   reset_peaks();
   pb_shrink->setEnabled(false);
   pb_draw->setEnabled(false);
   pb_assign_peaks->setEnabled(false);
   pb_reset_peaks->setEnabled(false);
   distro_solute.clear();
   struct SimulationComponent temp_component;
   component.clear(); // save monte carlo data for statistics and sort them from this vector
   Solute temp_solute;
   filename = QFileDialog::getOpenFileName(USglobal->config_list.result_dir,
                                           "*.vhw_his.* *.fe_dis.* *.cofs_dis.* *.sa2d_dis.* *.sa2d_mw_dis.* *.ga_dis.* *.ga_mw_dis.* *.ga_mw_mc_dis.* *.ga_mc_dis.* *.sa2d_mc_dis.* *.sa2d_mw_mc_dis.* *.global_dis.* *.global_mc_dis.* ", 0);
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
   else if (filename.contains("global_mc_dis", false))
   {
      distro_type = 12;
      monte_carlo = true;
      index = filename.findRev(".global_mc_dis.", -1, true);
      id = filename.left(index);
   }
   else
   {
      distro_type = -1; // undefined
   }
   //   cout << ":distro: " << distro_type << endl;
   if (distro_type == 0)
   {
      dimension = 1;
      if(f.open(IO_ReadOnly))
      {
         QTextStream ts(&f);
         if (!ts.atEnd())
         {
            ts.readLine(); // discard header line
         }
         while (!ts.atEnd())
         {
            ts >> temp1; // s_20W
            ts >> temp2; // Frequency
            ts.readLine(); // rest of line
            temp_solute.s = temp1;
            temp_solute.c = temp2;
            temp_solute.k = (ff0_max - ff0_min)/2.0;
            distro_solute.push_back(temp_solute);
         }
         f.close();
         cnt_s_range->setEnabled(false);
         cnt_k_range->setEnabled(false);
         cb_1dim->setChecked(true);
         cb_2dim->setChecked(false);
         cb_3dim->setChecked(false);
         cb_1dim->setEnabled(true);
      }
   }
   if (distro_type > 0)
   {
      //      cout << distro_type << endl;
      if(f.open(IO_ReadOnly))
      {
         QTextStream ts(&f);
         if (!ts.atEnd())
         {
            ts.readLine(); // discard header line
         }
         if (distro_type == 7) // global distro, could be Monte Carlo, so check...
         {
            QString s1;
            ts >> s1;
            ts >> s1;
            if (s1 == "#")
            {
               ts.readLine();
               monte_carlo = true;
            }
            ts.setDevice(0);
            ts.setDevice(&f);
            ts.readLine();
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
            //            cout << temp1 << ", ";
            ts >> temp1; // s_20,W
            //            cout << temp1 << ", ";
            temp_component.s = (float) temp1;
            temp1 *= (float) 1.0e13; // change to proper scale
            ts >> temp2; // D_apparent
            //            cout << temp2 << ", ";
            ts >> temp2; // D_20,W
            //            cout << temp2 << ", ";
            temp_component.D = temp2;
            ts >> temp2; // MW
            //            cout << temp2 << ", ";
            temp_component.mw = temp2;
            ts >> temp2; // Frequency
            //            cout << temp2 << ", ";
            temp_component.concentration = temp2;
            ts >> temp3; // f/f0
            if (temp3 < 1.0) temp3 = 1.0;
            //            cout << temp3 << "\n";
            temp_component.f_f0 = temp3;
            component.push_back(temp_component);
            temp_solute.s = temp1;
            temp_solute.c = temp2;
            temp_solute.k = temp3;
            //            cout << temp_solute.s << ", " << temp_solute.c << ", " << temp_solute.k << endl;
            distro_solute.push_back(temp_solute);
            //      //cerr << temp1 << ", " << temp2 << ", " << temp3 << endl;
         }
         cb_1dim->setEnabled(true);
         cb_2dim->setEnabled(true);
         cb_3dim->setEnabled(true);
         f.close();
      }
      distro_solute.sort();

      // combine identical solutes:
      list <Solute> reduced;
      reduced.clear();
      list <Solute>::iterator j, j1, j2;
      i = 0;
      reduced = distro_solute;
      distro_solute.clear();
      j1 = reduced.begin();
      j2 = reduced.begin();
      //cout << "size before ga_init: " << reduced.size() << endl;
      if ( reduced.size() > 1 )
      {
         j2++;
         while ( j2 != reduced.end() )
         {
            if ( (*j1).s == (*j2).s && (*j1).k == (*j2).k )
            {
               (*j1).c += (*j2).c;
               j2++;
            }
            else
            {
               distro_solute.push_back(*j1);
               j1 = j2;
               j2++;
            }
         }
         distro_solute.push_back(*j1);
      }
      else if (reduced.size() == 1)
      {
         distro_solute = reduced;
      }
      //cout << "size after ga_init: " << distro_solute.size() << endl;
      reduced.clear();
      float low_s, high_s, val_k, low_k=1.0e6, high_k = -1.0e6;
      for (j = distro_solute.begin(); j != distro_solute.end(); j++)
      {
         if ((*j).k > high_k)
         {
            high_k = (*j).k;
         }
         if ((*j).k < low_k)
         {
            low_k = (*j).k;
         }
      }
      val_k = high_k - low_k;
      j = distro_solute.begin();
      low_s = (*j).s;
      low_k = (*j).k;
      j = distro_solute.end();
      j --;
      high_s = (*j).s;
      high_k = (*j).k;
      float val_s = high_s - low_s;
      if (val_s > 0)
      {
         s_range = 0.9 * val_s / distro_solute.size();
      }
      else
      {
         j = distro_solute.begin();
         s_range = (*j).s * 0.9;
      }
      if (val_k > 0)
      {
         k_range = 0.9 * val_k / distro_solute.size();
      }
      else
      {
         j = distro_solute.begin();
         k_range = (*j).k * 0.9;
      }
      cnt_s_range->disconnect();
      cnt_k_range->disconnect();
      float k_tmp, s_tmp;
      k_tmp = (int) (log(k_range));
      k_tmp = pow(10.0, (double) (k_tmp-1));
      s_tmp = (int) (log(s_range));
      s_tmp = pow(10.0, (double) (s_tmp-1));
      //cout << "k_range: " << k_range << ", k_tmp: " << k_tmp << ", s_range: " << s_range << ", s_tmp: " << s_tmp << endl;
      cnt_s_range->setRange(s_tmp, high_s * 2, s_tmp);
      cnt_s_range->setValue(s_range);
      cnt_s_range->setEnabled(true);
      cnt_k_range->setRange(k_tmp, high_k*2, k_tmp);
      cnt_k_range->setValue(k_range);
      cnt_k_range->setEnabled(true);
      cb_autolimit->setChecked(true);
      connect(cnt_s_range, SIGNAL(valueChanged(double)), SLOT(update_s_range(double)));
      connect(cnt_k_range, SIGNAL(valueChanged(double)), SLOT(update_k_range(double)));
   }
   double smin=1.0e30, smax=-1.0e30, fmin=1.0e30, fmax=-1.0e30;
   //list <struct Solute>::iterator iter;
   list <Solute>::iterator iter;
   for (iter = distro_solute.begin(); iter != distro_solute.end(); iter++)
   {
      smin = min(smin, (double) (*iter).s);
      smax = max(smax, (double) (*iter).s);
      fmin = min(fmin, (double) (*iter).k);
      fmax = max(fmax, (double) (*iter).k);
      //      cout <<"c: " << (*iter).c << "s: " << (*iter).s << ", ff0: " << (*iter).k << ", smax: " << smax << ", smin: " << smin << ", fmin: " << fmin << ", fmax: " << fmax << endl;
   }
   float diff;
   diff = smax - smin;
   smin -= diff/10;
   smax += diff/10;
   diff = fmax - fmin;
   fmin -= diff/10;
   fmax += diff/10;
   cnt_plot_fmin->setValue(fmin);
   cnt_plot_smin->setValue(smin);
   cnt_plot_fmax->setValue(fmax);
   cnt_plot_smax->setValue(smax);
   cnt_plot_fmin->setEnabled(false);
   cnt_plot_smin->setEnabled(false);
   cnt_plot_fmax->setEnabled(false);
   cnt_plot_smax->setEnabled(false);
   autolimit = false;
   plot->clear();
   double *x, *y;
   unsigned int curve;
   x = new double [distro_solute.size()];
   y = new double [distro_solute.size()];
   i = 0;
   //list <struct Solute>::iterator j;
   list <Solute>::iterator j;
   for (j = distro_solute.begin(); j != distro_solute.end(); j++)
   {
      x[i] = (*j).s;
      y[i] = (*j).c;
      i++;
   }
   curve = plot->insertCurve("distro");
   plot->setCurveData(curve, x, y, distro_solute.size());
   plot->setCurvePen(curve, QPen(yellow, 3, SolidLine));
   plot->setCurveStyle(curve, QwtCurve::Sticks);
   delete [] x;
   delete [] y;
   plot->setAxisTitle(QwtPlot::xBottom, tr("Sedimentation Coefficient corrected for water at 20ºC"));
   plot->setAxisTitle(QwtPlot::yLeft, tr("Relative Frequency"));
   plot->setAxisAutoScale(QwtPlot::xBottom);
   plot->replot();
   textwindow->setText(tr("Now either auto-assign the solute bins, or manually select bins"
                          " by clicking first on the lower and then on the upper limit of"
                          " the bin range. If you auto-assign the bins you should first"
                          " select the number of solute bins you want to use. UltraScan will"
                          " space the bins proportional to the integral value of each peak,"
                          " such that each bin contains the same integral value."
                          " You can select each solute bin from the listbox on the left"
                          " and modify the frictional ratio limits by selecting them first"
                          " with the respective counters. To change the frictional ratios for"
                          " the solutes, change to the desired f/f0 values in the counter, then"
                          " double-click on the listbox item to change the selected bin accordingly."));
   pb_print->setEnabled(true);
   if (monte_carlo) // too many solutes in MC to be useful
   {
      pb_assign_peaks->setEnabled(false);
   }
   else
   {
      pb_assign_peaks->setEnabled(true);
   }
   //   //cerr << "Solute size in setup_gui: " << GA_Solute.size() << endl;
}

void US_GA_Initialize::plot_1dim()
{
   pb_draw->setEnabled(false);
   QString str;
   plot->clear();
   double *x, *y;
   unsigned int curve, i;
   x = new double [distro_solute.size()];
   y = new double [distro_solute.size()];
   //list <struct Solute>::iterator j;
   list <Solute>::iterator j;
   i = 0;
   for (j = distro_solute.begin(); j != distro_solute.end(); j++)
   {
      x[i] = (*j).s;
      y[i] = (*j).c;
      i++;
   }
   plot->enableGridXMin(true);
   plot->enableGridYMin(true);
   plot->enableGridX(true);
   plot->enableGridY(true);
   curve = plot->insertCurve("distro");
   plot->setCurveData(curve, x, y, distro_solute.size());
   plot->setCurvePen(curve, QPen(yellow, 3, SolidLine));
   plot->setCurveStyle(curve, QwtCurve::Sticks);
   delete [] x;
   delete [] y;
   plot->setAxisTitle(QwtPlot::xBottom, tr("Sedimentation Coefficient corrected for water at 20ºC"));
   plot->setAxisTitle(QwtPlot::yLeft, tr("Relative Frequency"));
   plot->setAxisAutoScale(QwtPlot::xBottom);
   plot->setAxisAutoScale(QwtPlot::yLeft);
   plot->replot();
}

void US_GA_Initialize::plot_2dim()
{
   QwtSymbol symbol;
   symbol.setStyle(QwtSymbol::Ellipse);
   symbol.setPen(Qt::red);
   symbol.setBrush(Qt::yellow);
   if (distro_solute.size() < 100 && distro_solute.size() > 50)
   {
      symbol.setSize(8);
   }
   else if (distro_solute.size() < 50  && distro_solute.size() > 20)
   {
      symbol.setSize(10);
   }
   else if (distro_solute.size() < 21)
   {
      symbol.setSize(12);
   }
   else if (distro_solute.size() > 100)
   {
      symbol.setSize(6);
   }
   double *x, *y;
   x = new double [distro_solute.size()];
   y = new double [distro_solute.size()];
   //list <struct Solute>::iterator sol_it;
   list <Solute>::iterator sol_it;
   unsigned int j=0;
   for (sol_it = distro_solute.begin(); sol_it != distro_solute.end(); sol_it++)
   {
      x[j] = (*sol_it).s;
      y[j] = (*sol_it).k;
      //      cout << j << ": " << y[j] << endl;
      j++;
   }
   plot->clear();
   plot->setTitle(tr("f/f0 vs Sed. Coeff."));
   plot->setAxisTitle(QwtPlot::xBottom, tr("s20,W (cm^2/sec)"));
   plot->setAxisTitle(QwtPlot::yLeft, tr("Frictional Ratio f/f0"));
   plot->setAxisAutoScale(QwtPlot::xBottom);
   plot->setAxisAutoScale(QwtPlot::yLeft);
   plot->enableGridXMin(true);
   plot->enableGridYMin(true);
   plot->enableGridX(true);
   plot->enableGridY(true);
   //      plot->setAxisScale(QwtPlot::yLeft, 0, max_y);
   long distro = plot->insertCurve("Solute Distribution");
   plot->setCurveStyle(distro, QwtCurve::NoCurve);
   plot->setCurveSymbol(distro, symbol);
   //      plot->setCurvePen(distro, QPen(Qt::yellow, 2, SolidLine));
   plot->setCurveData(distro, x, y, distro_solute.size());
   plot->replot();
   pb_draw->setEnabled(true);
   delete [] x;
   delete [] y;
}

void US_GA_Initialize::plot_buckets()
{
   QString str;
   //   cout << "Dimension: " << dimension << endl;
   unsigned int i;
   lb_solutes->clear();
   limits.clear();
   limits.resize(GA_Solute.size());
   if (dimension == 1)
   {
      double temp_x[3], temp_y[3];
      for (i=0; i<GA_Solute.size(); i++)
      {
         temp_x[0] = GA_Solute[i].s_min;
         temp_x[1] = GA_Solute[i].s_min;
         temp_x[2] = GA_Solute[i].s;
         temp_y[0] = 0.0;
         temp_y[1] = plot->axisScale(QwtPlot::yLeft)->hBound()/2.0;
         temp_y[2] = 3.0 * plot->axisScale(QwtPlot::yLeft)->hBound()/4.0;
         limits[i].line1 = plot->insertCurve(str.sprintf("Lower Limit for solute %d", i + 1));
         plot->setCurvePen(limits[i].line1, QPen(red, 2));
         plot->setCurveStyle(limits[i].line1, QwtCurve::Lines);
         plot->setCurveData(limits[i].line1, temp_x, temp_y, 3);
         temp_x[0] = GA_Solute[i].s_max;
         temp_x[1] = GA_Solute[i].s_max;
         limits[i].line2 = plot->insertCurve(str.sprintf("Upper Limit for solute %d", i + 1));
         plot->setCurveData(limits[i].line2, temp_x, temp_y, 3);
         plot->setCurvePen(limits[i].line2, QPen(red, 2, SolidLine));
         plot->setCurveStyle(limits[current_solute].line2, QwtCurve::Lines);
         plot->replot();
         str.sprintf("Solute Bin %d: s_min=%4.2e, s_max=%4.2e, f/f0_min=%4.2f, f/f0_max=%4.2f", i+1,
                     GA_Solute[i].s_min,
                     GA_Solute[i].s_max,
                     GA_Solute[i].ff0_min,
                     GA_Solute[i].ff0_max);
         lb_solutes->insertItem(str);
      }
      return;
   }

   double temp_x[2], temp_y[2];
   unsigned int box_border[4];
   for (i=0; i<GA_Solute.size(); i++)
   {
      temp_x[0] = GA_Solute[i].s_min;
      temp_x[1] = GA_Solute[i].s_min;
      temp_y[0] = GA_Solute[i].ff0_min;
      temp_y[1] = GA_Solute[i].ff0_max;
      box_border[0] = plot->insertCurve(str.sprintf("s-min border for solute %d", i + 1));
      plot->setCurvePen(box_border[0], QPen(yellow, 1));
      plot->setCurveStyle(box_border[0], QwtCurve::Lines);
      plot->setCurveData(box_border[0], temp_x, temp_y, 2);

      temp_x[0] = GA_Solute[i].s_min;
      temp_x[1] = GA_Solute[i].s_max;
      temp_y[0] = GA_Solute[i].ff0_max;
      temp_y[1] = GA_Solute[i].ff0_max;
      box_border[1] = plot->insertCurve(str.sprintf("ff0-max border for solute %d", i + 1));
      plot->setCurvePen(box_border[1], QPen(yellow, 1));
      plot->setCurveStyle(box_border[1], QwtCurve::Lines);
      plot->setCurveData(box_border[1], temp_x, temp_y, 2);

      temp_x[0] = GA_Solute[i].s_max;
      temp_x[1] = GA_Solute[i].s_max;
      temp_y[0] = GA_Solute[i].ff0_max;
      temp_y[1] = GA_Solute[i].ff0_min;
      box_border[2] = plot->insertCurve(str.sprintf("s-max border solute %d", i + 1));
      plot->setCurvePen(box_border[2], QPen(yellow, 1));
      plot->setCurveStyle(box_border[2], QwtCurve::Lines);
      plot->setCurveData(box_border[2], temp_x, temp_y, 2);

      temp_x[0] = GA_Solute[i].s_min;
      temp_x[1] = GA_Solute[i].s_max;
      temp_y[0] = GA_Solute[i].ff0_min;
      temp_y[1] = GA_Solute[i].ff0_min;
      box_border[3] = plot->insertCurve(str.sprintf("ff0-min border for solute %d", i + 1));
      plot->setCurvePen(box_border[3], QPen(yellow, 1));
      plot->setCurveStyle(box_border[3], QwtCurve::Lines);
      plot->setCurveData(box_border[3], temp_x, temp_y, 2);
      str.sprintf("Solute Bin %d: s_min=%4.2e, s_max=%4.2e, f/f0_min=%4.2f, f/f0_max=%4.2f", i+1,
                  GA_Solute[i].s_min,
                  GA_Solute[i].s_max,
                  GA_Solute[i].ff0_min,
                  GA_Solute[i].ff0_max);
      lb_solutes->insertItem(str);
   }
   //   plot->setAxisAutoScale(QwtPlot::xBottom);
   //   plot->setAxisAutoScale(QwtPlot::yLeft);
   plot->replot();
   pb_save->setEnabled(true);
}

void US_GA_Initialize::plot_3dim()
{
   pb_draw->setEnabled(true);
   QString str;
   plot->clear();
   progress->reset();
   lb_solutes->clear();
   limits.clear();
   limits.resize(GA_Solute.size());
   plot->setAxisTitle(QwtPlot::xBottom, tr("Sedimentation Coefficient corrected for water at 20ºC"));
   plot->setAxisTitle(QwtPlot::yLeft, tr("Frictional Ratio f/f0"));
   plot->enableGridXMin(false);
   plot->enableGridYMin(false);
   plot->enableGridX(false);
   plot->enableGridY(false);
   QwtSymbol symbol;
   QSize size(x_pixel, y_pixel);
   symbol.setStyle(QwtSymbol::Rect);
   symbol.setSize(size);
   unsigned int i, j, k, count;
   //unsigned int curve[gradient.size()];
   vector<unsigned int> curve( gradient.size() );
   //double x[x_resolution], y[y_resolution], z[x_resolution][y_resolution];
   double* x = new double [ x_resolution ];
   double* y = new double [ y_resolution ];

   double** z;
   z = new double* [x_resolution];
   for ( unsigned int i = 0; i < x_resolution; i++ ) z[i] = new double [ y_resolution ];

   double smin=1.0e30, smax=-1.0e30, fmin=1.0e30, fmax=-1.0e30;
   double frange, srange, sstep, fstep, ssigma, fsigma;
   double *xval, *yval;
   xval = new double [x_resolution*y_resolution];
   yval = new double [x_resolution*y_resolution];
   list <Solute>::iterator iter;
   for (iter = distro_solute.begin(); iter != distro_solute.end(); iter++)
   {
      smin = min(smin, (double) (*iter).s);
      smax = max(smax, (double) (*iter).s);
      fmin = min(fmin, (double) (*iter).k);
      fmax = max(fmax, (double) (*iter).k);
      //cout  << "s: " << (*iter).s << ", ff0: " << (*iter).k << ", smax: " << smax << ", smin: " << smin << ", fmin: " << fmin << ", fmax: " << fmax << endl;

   }
   if (smax - smin < 1e-10)
   {
      s_range = smax/10.0;
   }
   if (fmax - fmin < 1e-10)
   {
      k_range = fmax/10.0;
   }
   //cout << s_range << ", " << k_range << endl;
   smax += s_range;
   smin -= s_range;
   fmax += k_range;
   fmin -= k_range;
   if(!autolimit)
   {
      smax = plot_smax;
      smin = plot_smin;
      fmax = plot_fmax;
      fmin = plot_fmin;
   }
   if (fmin < 0.0)
   {
      fmin = 0.0;
   }

   cnt_plot_fmin->setValue(fmin);
   srange = smax - smin;
   sstep = srange/(double) x_resolution;
   frange = fmax - fmin;
   if (frange < 1.0e-3)
   {
      fmax += fmax/10.0;
      fmin = 1.0;
      frange = fmax - fmin;
   }
   fstep = frange/(double) y_resolution;
   ssigma = srange/resolution;
   fsigma = frange/resolution;
   ssigma /= 2.0;
   fsigma /= 2.0;
   plot->setAxisScale(QwtPlot::xBottom, smin, smax);
   plot->setAxisScale(QwtPlot::yLeft, fmin, fmax);
   //cout << "ssigma: " << ssigma << ", fsigma: " << fsigma << ", srange: " << srange << ", frange: " << frange << ", resolution: " << resolution << endl;
   for (i=0; i<x_resolution; i++)
   {
      x[i] = smin + i * sstep;
      for (j=0; j<y_resolution; j++)
      {
         z[i][j] = 0.0;
      }
   }
   for (j=0; j<y_resolution; j++)
   {
      y[j] = fmin + j * fstep;
   }
   double maxval = 0;
   progress->setTotalSteps(distro_solute.size());
   count = 0;
   //   cout << "Resolution: " << resolution << ", size: " << distro_solute.size() << ", x: " << x_resolution
   //         << ", y: " << y_resolution << ", ssigma: " << ssigma << ", fsigma: " << fsigma << endl;
   if (resolution == 100)
   {
      for (iter = distro_solute.begin(); iter != distro_solute.end(); iter++)
      {
         for (i=0; i<x_resolution; i++)
         {
            for (j=0; j<y_resolution; j++)
            {
               //cout << "y: " << y[i] << ", it_k: " <<  (*iter).k  << ", x: " << x[i] << ", it_s: " <<  (*iter).s <<  ", sig_f: " << fsigma << ", sig_s: " << ssigma << endl;
               if (y[j] > (*iter).k - fsigma && y[j] < (*iter).k + fsigma
                   && x[i] > (*iter).s - ssigma && x[i] < (*iter).s + ssigma)
               {
                  z[i][j] += (*iter).c;
               }
               maxval = max(maxval, z[i][j]);
               //               cout << "z[" << i << "][" << j << "]: " << z[i][j] << ", maxval: " << maxval <<endl;
            }
         }
         count++;
         progress->setProgress(count);
      }
      /*
        cout << "maxval: " << maxval << endl;
        for (i=0; i<x_resolution; i++)
        {
        for (j=0; j<y_resolution; j++)
        {
        cout << z[i][j] << endl;
        }
        }
      */
   }
   else
   {
      if(USglobal->config_list.numThreads > 1)
      {
         //double maxvals[USglobal->config_list.numThreads];
         vector<double> maxvals( USglobal->config_list.numThreads );
         //double *zz[x_resolution];
         double** zz = new double* [ x_resolution ];

         for(j = 0; j < USglobal->config_list.numThreads; j++)
         {
            maxvals[j] = 0;
         }

         for(j = 0; j < x_resolution; j++)
         {
            zz[j] = z[j];
         }

         // create threads

         //US_Plot3d_thr_t *plot3d_thr_threads[USglobal->config_list.numThreads];
         vector<US_Plot3d_thr_t*> plot3d_thr_threads( USglobal->config_list.numThreads );
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
            x_end   = (x_inc * (j + 1)) - 1;

            if ( j + 1 == USglobal->config_list.numThreads )
            {
               x_end = x_resolution - 1;
            }

            //         cout << "thread " << j << " x range " << x_start << " - " << x_end << endl;
            plot3d_thr_threads[j]->plot3d_thr_setup(j, zz, distro_solute, x, y,
                                                    x_start, x_end, y_resolution, &maxvals[j], ssigma, fsigma, progress);
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
            //         cout << "maxval " << j << " " << maxvals[j] << endl;
            maxval = max(maxval,maxvals[j]);
            delete plot3d_thr_threads[j];
         }
         delete [] zz;
      }
      else
      {
         for (iter = distro_solute.begin(); iter != distro_solute.end(); iter++)
         {
            for (i=0; i<x_resolution; i++)
            {
               for (j=0; j<y_resolution; j++)
               {
                  z[i][j] += (*iter).c * exp(-pow((x[i] - (*iter).s), 2.0)/(pow(2.0 * ssigma, 2.0)))
                     * exp(-pow((y[j] - (*iter).k), 2.0)/(pow(2.0 * fsigma, 2.0)));
                  maxval = max( maxval, z[i][j] );
                  //               cout << "z[" << i << "][" << j << "]: " << z[i][j] << ", maxval: " << maxval <<endl;
               }
            }
            count++;
            progress->setProgress(count);
         }
      }
   }
   for (i=0; i<x_resolution; i++)
   {
      x[i] = smin + i * sstep;
      for (j=0; j<y_resolution; j++)
      {
         if (z[i][j] !=0)
         {
            z[i][j] = 1 + (gradient.size()-1) * z[i][j]/maxval;
         }
      }
   }
   //   cout << "i: " << i << ", j: " << j << ", size: " << gradient.size() << ", Max: " << maxval << endl;
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
   for (k=0; k<gradient.size(); k++)
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
      symbol.setPen(gradient[k]);
      symbol.setBrush(gradient[k]);
      plot->setCurveSymbol(curve[k], symbol);
      plot->setCurveStyle(curve[k], QwtCurve::NoCurve);
      plot->setCurveData(curve[k], xval, yval, count);
      plot->setTitle(fi.fileName() + cell_info + str1);
   }
   plot->setCanvasBackground(gradient[0]);
   plot->replot();
   delete [] xval;
   delete [] yval;
   for ( unsigned int i = 0; i < x_resolution; i++ ) delete [] z[i];
   delete [] z;
   delete [] x;
   delete [] y;

   //   cout << distro_type << endl;
   switch (distro_type)
   {
   case 1:
      {
         str = htmlDir + "/cofs_pseudo3d_fs" + cell_info + ".";
         break;
      }
   case 2:
      {
         str = htmlDir + "/fe_pseudo3d_fs" + cell_info + ".";
         break;
      }
   case 3:
      {
         str = htmlDir + "/sa2d_pseudo3d_fs" + cell_info + ".";
         break;
      }
   case 4:
      {
         str = htmlDir + "/ga_mc_pseudo3d_fs" + cell_info + ".";
         break;
      }
   case 5:
      {
         str = htmlDir + "/sa2d_mc_pseudo3d_fs" + cell_info + ".";
         break;
      }
   case 6:
      {
         str = htmlDir + "/ga_pseudo3d_fs" + cell_info + ".";
         break;
      }
   case 7:
      {
         str = htmlDir + "/global_pseudo3d_fs" + cell_info + ".";
         break;
      }
   case 8:
      {
         str = htmlDir + "/sa2d_mw_pseudo3d_fs" + cell_info + ".";
         break;
      }
   case 9:
      {
         str = htmlDir + "/ga_mw_pseudo3d_fs" + cell_info + ".";
         break;
      }
   case 10:
      {
         str = htmlDir + "/sa2d_mw_mc_pseudo3d_fs" + cell_info + ".";
         break;
      }
   case 11:
      {
         str = htmlDir + "/ga_mw_mc_pseudo3d_fs" + cell_info + ".";
         break;
      }
   }//   cout << str << endl;
   QPixmap p;
   US_Pixmap *pm;
   pm = new US_Pixmap();
   p = QPixmap::grabWidget(plot, 2, 2, plot->width() - 4, plot->height() - 4);
   pm->save_file(str, p);
}

void US_GA_Initialize::highlight_solute(int j)
{
   highlight_solute(j, true);
}

void US_GA_Initialize::highlight_solute(int j, bool flag)
{
   QString str;
   limits.clear();
   limits.resize(GA_Solute.size());
   if (flag)
   {
      plot->clear();
      plot->setAxisTitle(QwtPlot::xBottom, tr("Sedimentation Coefficient corrected for water at 20ºC"));
      plot->setAxisTitle(QwtPlot::yLeft, tr("Frictional Ratio f/f0"));
      plot->setAxisAutoScale(QwtPlot::xBottom);
      plot->setAxisAutoScale(QwtPlot::yLeft);
   }
   double temp_x[2], temp_y[2];
   unsigned int box_border[4], i;
   QPen p;
   p.setWidth(1);
   for (i=0; i<GA_Solute.size(); i++)
   {
      if (i == (unsigned int) j)
      {
         p.setColor(Qt::red);
      }
      else
      {
         p.setColor(Qt::yellow);
      }
      temp_x[0] = GA_Solute[i].s_min;
      temp_x[1] = GA_Solute[i].s_min;
      temp_y[0] = GA_Solute[i].ff0_min;
      temp_y[1] = GA_Solute[i].ff0_max;
      box_border[0] = plot->insertCurve(str.sprintf("s-min border for solute %d", i + 1));
      plot->setCurvePen(box_border[0], p);
      plot->setCurveStyle(box_border[0], QwtCurve::Lines);
      plot->setCurveData(box_border[0], temp_x, temp_y, 2);

      temp_x[0] = GA_Solute[i].s_min;
      temp_x[1] = GA_Solute[i].s_max;
      temp_y[0] = GA_Solute[i].ff0_max;
      temp_y[1] = GA_Solute[i].ff0_max;
      box_border[1] = plot->insertCurve(str.sprintf("ff0-max border for solute %d", i + 1));
      plot->setCurvePen(box_border[1], p);
      plot->setCurveStyle(box_border[1], QwtCurve::Lines);
      plot->setCurveData(box_border[1], temp_x, temp_y, 2);

      temp_x[0] = GA_Solute[i].s_max;
      temp_x[1] = GA_Solute[i].s_max;
      temp_y[0] = GA_Solute[i].ff0_max;
      temp_y[1] = GA_Solute[i].ff0_min;
      box_border[2] = plot->insertCurve(str.sprintf("s-max border solute %d", i + 1));
      plot->setCurvePen(box_border[2], p);
      plot->setCurveStyle(box_border[2], QwtCurve::Lines);
      plot->setCurveData(box_border[2], temp_x, temp_y, 2);

      temp_x[0] = GA_Solute[i].s_min;
      temp_x[1] = GA_Solute[i].s_max;
      temp_y[0] = GA_Solute[i].ff0_min;
      temp_y[1] = GA_Solute[i].ff0_min;
      box_border[3] = plot->insertCurve(str.sprintf("ff0-min border for solute %d", i + 1));
      plot->setCurvePen(box_border[3], p);
      plot->setCurveStyle(box_border[3], QwtCurve::Lines);
      plot->setCurveData(box_border[3], temp_x, temp_y, 2);
   }
   plot->replot();
}

void US_GA_Initialize::select_1dim()
{
   plot->setOutlineStyle(Qwt::Cross);
   dimension = 1;
   cnt_s_range->setEnabled(false);
   cnt_k_range->setEnabled(false);
   cb_1dim->setChecked(true);
   cb_2dim->setChecked(false);
   cb_3dim->setChecked(false);
   pb_reset_peaks->setEnabled(false);
   pb_shrink->setEnabled(false);
   pb_save->setEnabled(false);
   reset_peaks();
   plot_1dim();
}

void US_GA_Initialize::select_2dim()
{
   plot->setOutlineStyle(Qwt::Rect);
   dimension = 2;
   cnt_s_range->setEnabled(true);
   cnt_k_range->setEnabled(true);
   cb_1dim->setChecked(false);
   cb_2dim->setChecked(true);
   cb_3dim->setChecked(false);
   pb_reset_peaks->setEnabled(false);
   pb_save->setEnabled(false);
   reset_peaks();
   plot_2dim();
}

void US_GA_Initialize::select_3dim()
{
   plot->setOutlineStyle(Qwt::Rect);
   dimension = 3;
   cnt_s_range->setEnabled(true);
   cnt_k_range->setEnabled(true);
   cb_1dim->setChecked(false);
   cb_2dim->setChecked(false);
   cb_3dim->setChecked(true);
   pb_reset_peaks->setEnabled(false);
   pb_save->setEnabled(false);
   reset_peaks();
   pb_replot3d->setEnabled(true);
   plot_3dim();
}

void US_GA_Initialize::assign_peaks()
{
   if (initial_solutes == 0 && distro_type == 0 )
   {
      QMessageBox::warning(this, tr("UltraScan Warning"),
                           tr("Please note:\n\nPlease select at least one initial solute!"),
                           QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      return;
   }
   reset_peaks();
   lb_solutes->clear();
   //cout << "distro_type: " << distro_type << endl;
   if (distro_type == 0)
   {
      QString str;
      float integral=0.0, range, temp, sum;
      unsigned int i, j;
      list <Solute>::iterator j1, j2, j3;
      for (j1 = distro_solute.begin(); j1 != distro_solute.end(); j1++)
      {
         integral += (*j1).c;
      }

      j1 = distro_solute.begin();

      while ((*j1).c < 1.0e-2)
      {
         j1++;
      }

      j2 = distro_solute.end();
      j2--;  // Can't dereference while at end()

      while ((*j2).c < 1.0e-2)
      {
         j2--;
      }

      integral = integral/initial_solutes;
      //cout << "integral: " << integral << endl;
      range = ((*j2).s - (*j1).s)/20.0;
      //cout << "range: " << range << endl;
      temp = (*j1).s - range;
      if (temp < 0)
      {
         temp =  (float) 1.0e-16;
      }
      j3 = distro_solute.begin();
      GA_Solute.resize(initial_solutes);
      for (i=0; i<initial_solutes-1; i++)
      {
         GA_Solute[i].s_min = temp;
         sum = 0.0;
         while (sum < integral)
         {
            sum += (*j3).c;
            j3++;
         }
         j3--;
         GA_Solute[i].s_max = (*j3).s;
         temp = GA_Solute[i].s_max;
         GA_Solute[i].s = (GA_Solute[i].s_max + GA_Solute[i].s_min)/2.0;
         GA_Solute[i].ff0_min = ff0_min;
         GA_Solute[i].ff0_max = ff0_max;
         GA_Solute[i].ff0 = (GA_Solute[i].ff0_max + GA_Solute[i].ff0_min)/2.0;
      }
      j = initial_solutes-1;
      GA_Solute[j].s_min = GA_Solute[j-1].s_max;
      GA_Solute[j].s_max = (*j2).s + range;
      GA_Solute[j].s = (GA_Solute[j].s_max + GA_Solute[j].s_min)/2.0;
      GA_Solute[j].ff0_min = ff0_min;
      GA_Solute[j].ff0_max = ff0_max;
      GA_Solute[j].ff0 = (GA_Solute[j].ff0_max + GA_Solute[j].ff0_min)/2.0;
      plot_1dim();
      plot_buckets();
   }
   if (distro_type > 0)
   {
      calc_distro();
      if (dimension == 1)
      {
         plot_1dim();
      }
      else if (dimension == 2)
      {
         plot_2dim();
      }
      else if (dimension == 3)
      {
         plot_3dim();
      }
      plot_buckets();
   }
   pb_reset_peaks->setEnabled(true);
   pb_save->setEnabled(true);
}

void US_GA_Initialize::reset_peaks_replot()
{
   reset_peaks();
   if (distro_type == 0)
   {
      dimension = 1;
   }
   switch (dimension)
   {
   case 1:
      {
         plot_1dim();
         break;
      }
   case 2:
      {
         plot_2dim();
         break;
      }
   case 3:
      {
         plot_3dim();
         break;
      }
   }
}

void US_GA_Initialize::reset_peaks()
{
   zoom = false;
   current_solute = 0;
   plot->clear();
   GA_Solute.clear();
   MC_solute.clear();
   limits.clear();
   limits.resize(initial_solutes);
   lb_solutes->clear();
   pb_reset_peaks->setEnabled(false);
   pb_save->setEnabled(false);
   cnt_initial_solutes->disconnect();
   cnt_initial_solutes->setValue(initial_solutes);
   connect(cnt_initial_solutes, SIGNAL(valueChanged(double)), SLOT(update_initial_solutes(double)));
}

void US_GA_Initialize::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/ga_initialize.html");
}

void US_GA_Initialize::update_initial_solutes(double val)
{
   //   reset_peaks();
   initial_solutes = (unsigned int) val;
   limits.resize(initial_solutes);
   GA_Solute.resize(initial_solutes);
}

void US_GA_Initialize::calc_distro()
{
   GA_Solute.clear();
   bucket temp_bucket;
   bucket temp_bucket2;
   list <bucket> storage;
   int index1, index2;
   distro_solute.sort();
   vector <bucket>::iterator i;
   list <Solute>::iterator j;
   list <bucket>::iterator k;

   j = distro_solute.begin();
   k = storage.begin();

   temp_bucket.s = (*j).s;
   temp_bucket.s_min = temp_bucket.s - s_range;
   if (temp_bucket.s_min < 0.1)
   {
      temp_bucket.s_min =  (float) 0.1;
   }
   temp_bucket.s_max = temp_bucket.s + s_range;
   temp_bucket.ff0 = (*j).k;
   temp_bucket.ff0_min = temp_bucket.ff0 - k_range;
   if (temp_bucket.ff0_min < 1.0)
   {
      temp_bucket.ff0_min = 1.0;
   }
   temp_bucket.ff0_max = temp_bucket.ff0 + k_range;
   temp_bucket.status = 0;
   temp_bucket.conc = (*j).c;
   storage.clear();
   storage.push_back(temp_bucket);
   j++;
   index1 = 0;
   while (j != distro_solute.end())
   {
      storage.sort(); // take the filled storage vector (perhaps containing some new solutes) and sort on "s"
      GA_Solute.clear(); // empty the GA vector
      for (k = storage.begin(); k != storage.end(); k++)
      {
         GA_Solute.push_back(*k); // repopulate GA solute vector with
      }
      i = GA_Solute.begin();

      // now set the default values:
      temp_bucket.s         = (*j).s;
      temp_bucket.s_min      = (*j).s - s_range;
      if (temp_bucket.s_min < 0.1)
      {
         temp_bucket.s_min =  (float) 0.1;
      }
      temp_bucket.s_max      = (*j).s + s_range;
      temp_bucket.ff0       = (*j).k;
      temp_bucket.ff0_min   = (*j).k - k_range;
      if (temp_bucket.ff0_min < 1.0)
      {
         temp_bucket.ff0_min = 1.0;
      }
      temp_bucket.ff0_max   = (*j).k + k_range;
      temp_bucket.conc      = (*j).c;
      temp_bucket.status    = 0;
      index1++;
      //cerr << "Index 1: " << index1 << endl;
      // if there are any overlaps with lower or equal s-value buckets
      // the default values get trimmed back to the non-overlapping region.
      // If there are s-value offsets, a new solute gets added.
      index2 = 0;
      //cerr << "starting with i: " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n";
      //cerr << "    temp_bucket: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n\n";
      while (i != GA_Solute.end())
      {
         index2++;
         //cerr << "Index 2: " << index2 << endl;
         // s values are equal, f values overlap (new solute has higher f)
         //cerr << "continuing with i: " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n";
         //cerr << "      temp_bucket: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n\n";
         if (((*i).s_min == temp_bucket.s_min) // case 1
             &&  ((*i).s_max == temp_bucket.s_max)
             &&  ((*i).ff0_max > temp_bucket.ff0_min))
         {
            temp_bucket.ff0_min   = (*i).ff0_max;
            temp_bucket.ff0       = temp_bucket.ff0_min + (temp_bucket.ff0_max - temp_bucket.ff0_min)/2.0;
            temp_bucket.status   = 1;
            //cerr << "Case 1: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
            //cerr << "        " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
         }
         else if (((*i).s_min == temp_bucket.s_min) // case 2
                  && ((*i).s_max == temp_bucket.s_max)
                  && (temp_bucket.ff0_max < (*i).ff0_max)
                  && (temp_bucket.ff0_max > (*i).ff0_min)
                  && (temp_bucket.ff0_min < (*i).ff0_min))
         {
            temp_bucket.ff0_max   = (*i).ff0_min;
            temp_bucket.ff0       = temp_bucket.ff0_min + (temp_bucket.ff0_max - temp_bucket.ff0_min)/2.0;
            temp_bucket.status   = 1;
            //cerr << "Case 2: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
            //cerr << "        " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
         }
         // f values are equal, s values overlap
         else if (((*i).ff0_min == temp_bucket.ff0_min)
                  && ((*i).ff0_max == temp_bucket.ff0_max)
                  && ((*i).s_max > temp_bucket.s_min))
         {
            temp_bucket.s_min      = (*i).s_max;
            temp_bucket.s          = temp_bucket.s_min + (temp_bucket.s_max - temp_bucket.s_min)/2.0;
            temp_bucket.status   = 1;
            //cerr << "Case 3: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
            //cerr << "        " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
         }
         else if ((temp_bucket.s_min   < (*i).s_max) // s-overlap lower right
                  && (temp_bucket.ff0_max < (*i).ff0_max)
                  && (temp_bucket.ff0_max >= (*i).ff0_min)
                  && (temp_bucket.ff0_min <= (*i).ff0_min))
         {
            temp_bucket2.s_min   = temp_bucket.s_min;
            if (temp_bucket2.s_min < 0.1)
            {
               temp_bucket2.s_min =  (float) 0.1;
            }
            temp_bucket2.s_max   = (*i).s_max;
            temp_bucket2.s         = temp_bucket2.s_min + (temp_bucket2.s_max - temp_bucket2.s_min)/2.0;
            temp_bucket2.ff0_min   = temp_bucket.ff0_min;
            if (temp_bucket2.ff0_min < 1.0)
            {
               temp_bucket2.ff0_min = 1.0;
            }
            temp_bucket2.ff0_max   = (*i).ff0_min;
            temp_bucket2.ff0       = temp_bucket2.ff0_min + (temp_bucket2.ff0_max - temp_bucket2.ff0_min)/2.0;
            temp_bucket2.conc      = (*i).conc;
            temp_bucket2.status    = 2;
            //cerr << "pushing back: " << temp_bucket2.s_min << ", " << temp_bucket2.s_max << " -- "  << temp_bucket2.ff0_min << ", " << temp_bucket2.ff0_max << "\n";
            storage.push_back(temp_bucket2);
            temp_bucket.s_min      = (*i).s_max;
            temp_bucket.status   = 1;
            //cerr << "Case 4a: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
            //cerr << "Case 4b: " << temp_bucket2.s_min << ", " << temp_bucket2.s_max << " -- "  << temp_bucket2.ff0_min << ", " << temp_bucket2.ff0_max << "\n";
            //cerr << "         " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
         }
         else if ((temp_bucket.s_min   <  (*i).s_max) // s-overlap upper right
                  && (temp_bucket.ff0_max >  (*i).ff0_max)
                  && (temp_bucket.ff0_min >= (*i).ff0_min)
                  && (temp_bucket.ff0_min <=  (*i).ff0_max))
         {
            temp_bucket2.s_min   = temp_bucket.s_min;
            if (temp_bucket2.s_min < 0.1)
            {
               temp_bucket2.s_min =  (float) 0.1;
            }
            temp_bucket2.s_max   = (*i).s_max;
            temp_bucket2.s         = temp_bucket2.s_min + (temp_bucket2.s_max - temp_bucket2.s_min)/2.0;
            temp_bucket2.ff0_min   = (*i).ff0_max;
            if (temp_bucket2.ff0_min < 1.0)
            {
               temp_bucket2.ff0_min = 1.0;
            }
            temp_bucket2.ff0_max   = temp_bucket.ff0_max;
            temp_bucket2.ff0       = temp_bucket2.ff0_min + (temp_bucket2.ff0_max - temp_bucket2.ff0_min)/2.0;
            temp_bucket2.conc      = (*i).conc;
            temp_bucket2.status    = 2;
            //cerr << "pushing back: " << temp_bucket2.s_min << ", " << temp_bucket2.s_max << " -- "  << temp_bucket2.ff0_min << ", " << temp_bucket2.ff0_max << "\n";
            storage.push_back(temp_bucket2);
            temp_bucket.s_min      = (*i).s_max;
            temp_bucket.status   = 1;
            //cerr << "Case 5a: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
            //cerr << "Case 5b: " << temp_bucket2.s_min << ", " << temp_bucket2.s_max << " -- "  << temp_bucket2.ff0_min << ", " << temp_bucket2.ff0_max << "\n";
            //cerr << "         " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
         }
         else if ((temp_bucket.s_min   < (*i).s_max) // s-overlap middle
                  && (temp_bucket.ff0_max < (*i).ff0_max)
                  && (temp_bucket.ff0_min > (*i).ff0_min))
         {
            temp_bucket.s_min      = (*i).s_max;
            temp_bucket.status   = 1;
            //cerr << "Case 6: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
            //cerr << "        " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
         }
         else if ((temp_bucket.s_min   < (*i).s_max) // s-overlap, new solute bucket is larger
                  && (temp_bucket.ff0_max > (*i).ff0_max)
                  && (temp_bucket.ff0_min < (*i).ff0_min))
         {
            temp_bucket2.s_min   = temp_bucket.s_min;
            if (temp_bucket2.s_min < 0.1)
            {
               temp_bucket2.s_min =  (float) 0.1;
            }
            temp_bucket2.s_max   = (*i).s_max;
            temp_bucket2.s         = temp_bucket2.s_min + (temp_bucket2.s_max - temp_bucket2.s_min)/2.0;
            temp_bucket2.ff0_min   = temp_bucket.ff0_min;
            if (temp_bucket2.ff0_min < 1.0)
            {
               temp_bucket2.ff0_min = 1.0;
            }
            temp_bucket2.ff0_max   = (*i).ff0_min;
            temp_bucket2.ff0       = temp_bucket2.ff0_min + (temp_bucket2.ff0_max - temp_bucket2.ff0_min)/2.0;
            temp_bucket2.conc      = (*i).conc;
            temp_bucket2.status    = 2;
            //cerr << "pushing back: " << temp_bucket2.s_min << ", " << temp_bucket2.s_max << " -- "  << temp_bucket2.ff0_min << ", " << temp_bucket2.ff0_max << "\n";
            storage.push_back(temp_bucket2);
            temp_bucket2.s_min   = temp_bucket.s_min;
            if (temp_bucket2.s_min < 0.1)
            {
               temp_bucket2.s_min =  (float) 0.1;
            }
            //cerr << "Case 8a: " << temp_bucket2.s_min << ", " << temp_bucket2.s_max << " -- "  << temp_bucket2.ff0_min << ", " << temp_bucket2.ff0_max << "\n";
            //cerr << "         " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
            temp_bucket2.s_max   = (*i).s_max;
            temp_bucket2.s         = temp_bucket2.s_min + (temp_bucket2.s_max - temp_bucket2.s_min)/2.0;
            temp_bucket2.ff0_min   = (*i).ff0_min;
            if (temp_bucket2.ff0_min < 1.0)
            {
               temp_bucket2.ff0_min = 1.0;
            }
            temp_bucket2.ff0_max   = temp_bucket.ff0_max;
            temp_bucket2.ff0       = temp_bucket2.ff0_min + (temp_bucket2.ff0_max - temp_bucket2.ff0_min)/2.0;
            temp_bucket2.conc      = (*i).conc;
            temp_bucket2.status    = 2;
            //cerr << "pushing back: " << temp_bucket2.s_min << ", " << temp_bucket2.s_max << " -- "  << temp_bucket2.ff0_min << ", " << temp_bucket2.ff0_max << "\n";
            storage.push_back(temp_bucket2);
            temp_bucket.s_min      = (*i).s_max;
            temp_bucket.status   = 1;
            //cerr << "Case 8b: " << temp_bucket2.s_min << ", " << temp_bucket2.s_max << " -- "  << temp_bucket2.ff0_min << ", " << temp_bucket2.ff0_max << "\n";
            //cerr << "Case 7c: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
            //cerr << "         " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
         }
         if (temp_bucket.s_min < 0.1)
         {
            temp_bucket.s_min =  (float) 0.1;
         }
         if (temp_bucket.ff0_min < 1.0)
         {
            temp_bucket.ff0_min = 1.0;
         }
         i++;
      }
      //cerr << "pushing back: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
      storage.push_back(temp_bucket);
      j++;
   }
   GA_Solute.clear();
   for (k = storage.begin(); k != storage.end(); k++)
   {
      GA_Solute.push_back(*k); // repopulate GA solute vector with
   }
   initial_solutes = GA_Solute.size();
   cnt_initial_solutes->disconnect();
   cnt_initial_solutes->setValue(initial_solutes);
   connect(cnt_initial_solutes, SIGNAL(valueChanged(double)), SLOT(update_initial_solutes(double)));
   if (dimension == 1)
   {
      plot_1dim();
   }
   else if (dimension == 2)
   {
      plot_2dim();
   }
   else if (dimension == 3)
   {
      plot_3dim();
   }
}

void US_GA_Initialize::load_color()
{
   QString line, filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc",
                                                         "*.map", 0);
   QFile f(filename);
   QColor col;
   gradient.clear();
   int r, g, b;
   float val;
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
         gradient.push_back(col);
      }
      f.close();
   }
}

void US_GA_Initialize::update_resolution(double val)
{
   resolution = (float) val;
}

void US_GA_Initialize::update_x_resolution(double val)
{
   x_resolution = (unsigned int) val;
}

void US_GA_Initialize::update_y_resolution(double val)
{
   y_resolution = (unsigned int) val;
}

void US_GA_Initialize::update_x_pixel(double val)
{
   x_pixel = (unsigned int) val;
}

void US_GA_Initialize::update_y_pixel(double val)
{
   y_pixel = (unsigned int) val;
}

void US_GA_Initialize::update_k_range(double val)
{
   k_range = (float) val;
   reset_peaks();
   calc_distro();
}

void US_GA_Initialize::update_s_range(double val)
{
   s_range = (float) val;
   reset_peaks();
   calc_distro();
}

void US_GA_Initialize::update_ff0_min(double val)
{
   ff0_min = (float) val;
}

void US_GA_Initialize::update_ff0_max(double val)
{
   ff0_max = (float) val;
}

void US_GA_Initialize::edit_solute(int index)
{
   QString str;
   GA_Solute[index].ff0_min = ff0_min;
   GA_Solute[index].ff0_max = ff0_max;
   GA_Solute[index].ff0 = (GA_Solute[index].ff0_min + GA_Solute[index].ff0_max)/2.0;
   str.sprintf("Solute Bin %d: s_min=%4.2e, s_max=%4.2e, f/f0_min=%4.2f, f/f0_max=%4.2f",
               index + 1,
               GA_Solute[index].s_min,
               GA_Solute[index].s_max,
               GA_Solute[index].ff0_min,
               GA_Solute[index].ff0_max);
   lb_solutes->changeItem(str, index);
}

void US_GA_Initialize::getMouseReleased(const QMouseEvent &e)
{
   QString str;
   bucket temp_solute;
   //cout << "zoom: " << zoom << endl;
   if (zoom)
   {
      p2.x = plot->invTransform(QwtPlot::xBottom, e.x());
      p2.y = plot->invTransform(QwtPlot::yLeft, e.y());
      if (p2.y < 1.0)
      {
         p2.y = 1.0;
      }
      if (p1.y < 1.0)
      {
         p1.y = 1.0;
      }
      if (p1.x > p2.x)
      {
         temp_solute.s_min = p2.x;
         temp_solute.s_max = p1.x;
         if(p1.y > p2.y)
         {
            temp_solute.ff0_min = p2.y;
            temp_solute.ff0_max = p1.y;
         }
         else
         {
            temp_solute.ff0_min = p1.y;
            temp_solute.ff0_max = p2.y;
         }
      }
      else
      {
         temp_solute.s_min = p1.x;
         temp_solute.s_max = p2.x;
         if(p1.y > p2.y)
         {
            temp_solute.ff0_min = p2.y;
            temp_solute.ff0_max = p1.y;
         }
         else
         {
            temp_solute.ff0_min = p1.y;
            temp_solute.ff0_max = p2.y;
         }
      }
      GA_Solute.push_back(temp_solute);
      highlight_solute(GA_Solute.size() - 1, false);
      str.sprintf("Solute Bin %d: s_min=%4.2e, s_max=%4.2e, f/f0_min=%4.2f, f/f0_max=%4.2f",
                  GA_Solute.size(),
                  GA_Solute[GA_Solute.size() - 1].s_min,
                  GA_Solute[GA_Solute.size() - 1].s_max,
                  GA_Solute[GA_Solute.size() - 1].ff0_min,
                  GA_Solute[GA_Solute.size() - 1].ff0_max);
      lb_solutes->insertItem(str);
      initial_solutes = GA_Solute.size();
      cnt_initial_solutes->disconnect();
      cnt_initial_solutes->setValue(initial_solutes);
      connect(cnt_initial_solutes, SIGNAL(valueChanged(double)), SLOT(update_initial_solutes(double)));
      vector <struct SimulationComponent>::iterator comp_it;
      MC_solute.clear();
      MC_solute.resize(GA_Solute.size());
      for (unsigned int i=0; i<GA_Solute.size(); i++)
      {
         MC_solute[i].clear();
         for (comp_it = component.begin(); comp_it != component.end(); comp_it++)
         {
            if ((*comp_it).s * 1.0e13 >= GA_Solute[i].s_min && (*comp_it).s * 1.0e13 <= GA_Solute[i].s_max
                &&  (*comp_it).f_f0 >= GA_Solute[i].ff0_min && (*comp_it).f_f0 <= GA_Solute[i].ff0_max)
            {
               MC_solute[i].push_back(*comp_it);
            }
         }
      }
      pb_shrink->setEnabled(true);
      pb_save->setEnabled(true);
      return;
   }
   if (initial_solutes < 1)
   {
      QMessageBox::warning(this, tr("UltraScan Warning"),
                           tr("Please note:\n\nPlease select at least one initial solute!"),
                           QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      return;
   }

   //   cout << "Current solute: " << current_solute << ", initial solutes: " << initial_solutes << endl;
   if (current_solute >= initial_solutes) // the user wants another bucket
   {
      initial_solutes++;
      //cnt_initial_solutes->disconnect();
      cnt_initial_solutes->setValue(initial_solutes);
      //connect(cnt_initial_solutes, SIGNAL(valueChanged(double)), SLOT(update_initial_solutes(double)));
   }
   double s = (plot->invTransform(QwtPlot::xBottom, e.x()));
   if (current_solute > 0 && s < GA_Solute[current_solute-1].s_max)
   {
      s = GA_Solute[current_solute-1].s_max;
      /*
        QMessageBox::information(this, tr("Attention:"),
        tr("Your selected s-value overlaps the previous bin.\n"
        "Overlapping bins will cause unnecessary calculations\n"
        "in the genetic algorithm calculation.\n\n"
        "Resetting the lower s-value limit to: ")
        + str.sprintf("%e", GA_Solute[current_solute-1].s_max) + " s");
      */
   }
   double temp_x[3];
   double temp_y[3];
   temp_x[0] = s;
   temp_x[1] = s;
   temp_x[2] = s;

   temp_y[0] = 0.0;
   temp_y[1] = plot->axisScale(QwtPlot::yLeft)->hBound();
   temp_y[2] = plot->axisScale(QwtPlot::yLeft)->hBound();
   if(!minmax)
   {
      limits[current_solute].line1 =
         plot->insertCurve(str.sprintf("Lower Limit for solute %d", current_solute + 1));
      plot->setCurvePen(limits[current_solute].line1, QPen(red, 2));
      plot->setCurveStyle(limits[current_solute].line1, QwtCurve::Lines);
      plot->setCurveData(limits[current_solute].line1, temp_x, temp_y, 3);
      GA_Solute[current_solute].s_min = s;
      GA_Solute[current_solute].ff0_min = ff0_min;
      minmax = true;
   }
   else
   {
      if (s < GA_Solute[current_solute].s_min)
      {
         QMessageBox::information(this, tr("Attention:"), tr("The upper s-value limit should"
                                                             "\nbe higher than the lower limit,\nwhich is: ")
                                  + str.sprintf("%e", GA_Solute[current_solute].s_min) + " s");
         return;
      }
      GA_Solute[current_solute].s_max = s;
      GA_Solute[current_solute].s = GA_Solute[current_solute].s_min + (s - GA_Solute[current_solute].s_min)/2.0;
      plot->removeCurve(limits[current_solute].line1);

      temp_x[0] = GA_Solute[current_solute].s_min;
      temp_x[1] = GA_Solute[current_solute].s_min;
      temp_x[2] = GA_Solute[current_solute].s;

      temp_y[0] = 0.0;
      temp_y[1] = plot->axisScale(QwtPlot::yLeft)->hBound()/2.0;
      temp_y[2] = 3.0 * plot->axisScale(QwtPlot::yLeft)->hBound()/4.0;

      limits[current_solute].line1 =
         plot->insertCurve(str.sprintf("Lower Limit for solute %d", current_solute + 1));
      plot->setCurvePen(limits[current_solute].line1, QPen(red, 2));
      plot->setCurveStyle(limits[current_solute].line1, QwtCurve::Lines);
      plot->setCurveData(limits[current_solute].line1, temp_x, temp_y, 3);

      temp_x[0] = s;
      temp_x[1] = s;

      limits[current_solute].line2 =
         plot->insertCurve(str.sprintf("Upper Limit for solute %d", current_solute + 1));
      plot->setCurveData(limits[current_solute].line2, temp_x, temp_y, 3);
      plot->setCurvePen(limits[current_solute].line2, QPen(red, 2, SolidLine));
      plot->setCurveStyle(limits[current_solute].line2, QwtCurve::Lines);
      GA_Solute[current_solute].s_max = s;
      GA_Solute[current_solute].ff0_min = ff0_min;
      GA_Solute[current_solute].ff0_max = ff0_max;
      GA_Solute[current_solute].ff0 = (GA_Solute[current_solute].ff0_min + GA_Solute[current_solute].ff0_max)/2.0;
      GA_Solute[current_solute].s = GA_Solute[current_solute].s_min;
      minmax = false;
      str.sprintf("Solute Bin %d: s_min=%4.2e, s_max=%4.2e, f/f0_min=%4.2f, f/f0_max=%4.2f",
                  current_solute + 1,
                  GA_Solute[current_solute].s_min,
                  GA_Solute[current_solute].s_max,
                  GA_Solute[current_solute].ff0_min,
                  GA_Solute[current_solute].ff0_max);
      lb_solutes->insertItem(str);
      current_solute ++;
      vector <struct SimulationComponent>::iterator comp_it;
      MC_solute.clear();
      MC_solute.resize(GA_Solute.size());
     // cout << component.size() << endl;
      for (unsigned int i=0; i<GA_Solute.size(); i++)
      {
         MC_solute[i].clear();
         for (comp_it = component.begin(); comp_it != component.end(); comp_it++)
         {
            if ((*comp_it).s * 1.0e13 >= GA_Solute[i].s_min && (*comp_it).s * 1.0e13 <= GA_Solute[i].s_max
                  &&  (*comp_it).f_f0 >= GA_Solute[i].ff0_min && (*comp_it).f_f0 <= GA_Solute[i].ff0_max)
            {
               MC_solute[i].push_back(*comp_it);
            }
         }
      }
      pb_reset_peaks->setEnabled(true);
      pb_save->setEnabled(true);
   }
   plot->replot();
}

void US_GA_Initialize::getMousePressed(const QMouseEvent &e)
{
   if (zoom)
   {
      p1.x = plot->invTransform(QwtPlot::xBottom, e.x());
      p1.y = plot->invTransform(QwtPlot::yLeft, e.y());
   }
}

void US_GA_Initialize::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_GA_Initialize::save()
{
   QString str, s1, filename;
   vector <QString> line;
   QFile f;
   unsigned int i, j;
   float sum;
   vector <double> val, conc;
   struct MonteCarloStats stats;
   line.clear();
   conc.clear();

   for (i=0; i<GA_Solute.size(); i++)
   {
      if ((GA_Solute[i].s_max - GA_Solute[i].s_min < 1.0e-16)
          && (GA_Solute[i].ff0_max - GA_Solute[i].ff0_min < 1.0e-4))
      {
         QMessageBox::warning(this, tr("UltraScan Warning"),
                              tr("Please note:\n\nBin #") + str.sprintf("%d", i+1) + tr(" does not have a finite width!\n\nSkipping..."),
                              QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      }
      else
      {
         line.push_back(str.sprintf("%6.4f, %6.4f, %6.4f, %6.4f",
                                    GA_Solute[i].s_min, GA_Solute[i].s_max, GA_Solute[i].ff0_min, GA_Solute[i].ff0_max));
      }
   }
   filename = id + cell_info;
   OneLiner ol_descr(tr("Please confirm or\nmodify the file name\nfor the GA distribution:"));
   ol_descr.show();
   ol_descr.parameter1->setText(filename);
   if (ol_descr.exec())
   {
      filename = ol_descr.string;
   }
   else
   {
      return;
   }

   if (line.size() > 0)
   {
      f.setName(filename + ".gadistro.dat");
      f.open(IO_WriteOnly);
      QTextStream ts(&f);
      ts << line.size() << endl;
      for (i=0; i<line.size(); i++)
      {
         ts << line[i] << endl;
      }
      f.close();
   }
   if (monte_carlo) // we also want the stats written to file.
   {
      f.setName(id + ".ga_stats" + cell_info);
      f.open(IO_WriteOnly);
      QTextStream ts(&f);
      ts << "***************************************************************************\n\n";
      ts << "Monte Carlo Analysis Statistical Results (from Genetic Algorithm Analysis):\n";
      ts << "\n***************************************************************************\n\n";
      ts << "Summary:\n";
      if (MC_solute.size() < 1)
      {
         QMessageBox::warning(this, tr("UltraScan Warning"),
                              tr("Please note:\n\nThere are no Solute bins defined!\n" ) +
                              tr("Please define some buckets before continuing."),
                              QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
         return;
      }
      float conc_sum=0.0;
      for (i=0; i<GA_Solute.size(); i++)
      {
         ts << "\nSolute " << i + 1 << ":\n";
         if (MC_solute[i].size() < 3) // we want at least 3 points to calculate a statistical distribution
         {
            ts << "\nThis solute bin does not have sufficient points to\ncalculate meaningful statistics.\n";
            ts << "\nAverage Molecular Weight: ";
            sum = 0.0;
            for (j=0; j<MC_solute[i].size(); j++)
            {
               sum += MC_solute[i][j].mw;
            }
            ts << sum/MC_solute[i].size() << endl;
            ts << "\nAverage Concentration: ";
            sum = 0.0;
            for (j=0; j<MC_solute[i].size(); j++)
            {
               sum += MC_solute[i][j].concentration * monte_carlo_iterations; // should be replaced by the number of Monte Carlo iterations
            }
            ts << sum/MC_solute[i].size() << endl;
            ts << "\nAverage Frictional Ratio: ";
            sum = 0.0;
            for (j=0; j<MC_solute[i].size(); j++)
            {
               sum += MC_solute[i][j].f_f0;
            }
            ts << sum/MC_solute[i].size() << "\n\n";
         }
         else
         {
            val.clear();
            for (j=0; j<MC_solute[i].size(); j++)
            {
               val.push_back(MC_solute[i][j].mw);
            }
            calc_stats(&stats, val, "Molecular Weight");
            ts << s1.sprintf(tr("Molecular weight:\t %6.4e (%6.4e, %6.4e)\n"), stats.mode_center, stats.conf95low, stats.conf95high);   // the standard error of the distribution
            val.clear();
            //            cout << "Monte carlo iterations: " << monte_carlo_iterations << endl;
            float total=0.0;
            for (j=0; j<MC_solute[i].size(); j++)
            {
               // multiply with the total number of MC iterations to scale back to normal concentration
               val.push_back(MC_solute[i][j].concentration * monte_carlo_iterations);
               total += MC_solute[i][j].concentration;
            }
            calc_stats(&stats, val, "Concentration");
            ts << s1.sprintf(tr("Concentration:\t\t %6.4e (%6.4e, %6.4e)\n"), stats.mode_center, stats.conf95low, stats.conf95high);   // the standard error of the distribution
            ts << s1.sprintf(tr("Total Concentration:\t\t %6.4e\n"), total);   // the total concentration of the solute
            conc.push_back(total); // add each solute's concentration to a vector to keep track of all of them for summaries
            conc_sum += total; // keep track of the total concentration
            val.clear();
            for (j=0; j<MC_solute[i].size(); j++)
            {
               val.push_back(MC_solute[i][j].s);
            }
            calc_stats(&stats, val, "Sedimentation Coefficient");
            ts << s1.sprintf(tr("Sedimentation Coeff.:\t %6.4e (%6.4e, %6.4e)\n"), stats.mode_center, stats.conf95low, stats.conf95high);   // the standard error of the distribution
            val.clear();
            for (j=0; j<MC_solute[i].size(); j++)
            {
               val.push_back(MC_solute[i][j].D);
            }
            calc_stats(&stats, val, "Diffusion Coefficient");
            ts << s1.sprintf(tr("Diffusion Coeff.:\t %6.4e (%6.4e, %6.4e)\n"), stats.mode_center, stats.conf95low, stats.conf95high);   // the standard error of the distribution
            val.clear();
            for (j=0; j<MC_solute[i].size(); j++)
            {
               val.push_back(MC_solute[i][j].f_f0);
            }
            calc_stats(&stats, val, "Frictional Ratio, f/f0");
            ts << s1.sprintf(tr("Frictional Ratio, f/f0:\t %6.4e (%6.4e, %6.4e)\n"), stats.mode_center, stats.conf95low, stats.conf95high);   // the standard error of the distribution
         }
      }
      ts << "\n\nRelative Concentrations:\n\n";
      ts << "Total concentration: " << conc_sum << " OD\n";
      for (i=0; i<conc.size(); i++)
      {
         ts << "Relative percentage of Solute " << i+1 << ": " << 100 * conc[i]/conc_sum << " %\n";
      }
      ts << "\n\nDetailed Results:\n";
      for (i=0; i<GA_Solute.size(); i++)
      {
         ts << "\n***************************************************************************\nSolute " << i + 1 << ":";
         if (MC_solute[i].size() < 3) // we want at least 3 points to calculate a statistical distribution
         {
            QMessageBox::warning(this, tr("UltraScan Warning"),
                                 tr("Please note:\n\nBin #") + str.sprintf("%d", i+1) +
                                 tr(" does not have sufficient points to calculate a meaningful distribution\n\nSkipping..."),
                                 QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            ts << "\nThis solute bin does not have sufficient points to\ncalculate a meaningful distribution\n";
            ts << "\nMolecular Weight:\n";
            for (j=0; j<MC_solute[i].size(); j++)
            {
               ts << MC_solute[i][j].mw << endl;
            }
            ts << "\nConcentration:\n";
            for (j=0; j<MC_solute[i].size(); j++)
            {
               ts << MC_solute[i][j].concentration * monte_carlo_iterations << endl;
            }
            ts << "\nSedimentation Coefficient:\n";
            for (j=0; j<MC_solute[i].size(); j++)
            {
               ts << MC_solute[i][j].s << endl;
            }
            ts << "\nDiffusion Coefficient:\n";
            for (j=0; j<MC_solute[i].size(); j++)
            {
               ts << MC_solute[i][j].D << endl;
            }
            ts << "\nFrictional Ratio:\n";
            for (j=0; j<MC_solute[i].size(); j++)
            {
               ts << MC_solute[i][j].f_f0 << endl;
            }
         }
         else
         {
            val.clear();
            for (j=0; j<MC_solute[i].size(); j++)
            {
               val.push_back(MC_solute[i][j].mw);
            }
            ts << calc_stats(&stats, val, "Molecular Weight");
            val.clear();
            for (j=0; j<MC_solute[i].size(); j++)
            {
               // multiply with the total number of MC iterations to scale back to normal concentration
               val.push_back(MC_solute[i][j].concentration * monte_carlo_iterations);
            }
            ts << calc_stats(&stats, val, "Concentration");
            val.clear();
            for (j=0; j<MC_solute[i].size(); j++)
            {
               val.push_back(MC_solute[i][j].s);
            }
            ts << calc_stats(&stats, val, "Sedimentation Coefficient");
            val.clear();
            for (j=0; j<MC_solute[i].size(); j++)
            {
               val.push_back(MC_solute[i][j].D);
            }
            ts << calc_stats(&stats, val, "Diffusion Coefficient");
            val.clear();
            for (j=0; j<MC_solute[i].size(); j++)
            {
               val.push_back(MC_solute[i][j].f_f0);
            }
            ts << calc_stats(&stats, val, "Frictional Ratio, f/f0");
         }
      }
      f.close();
   }
}

QString US_GA_Initialize::calc_stats(struct MonteCarloStats *stats, vector <double> val, QString str)
{
   QString output, s1, s2;
   unsigned int i, j, points, bins=50;
   double *xplot, *yplot;
   double intercept, slope, sigma, corr;
   (*stats).low  =  (float)  9.9e30;
   (*stats).high =  (float) -9.9e30;
   (*stats).area = val.size();
   (*stats).points = val.size();
   xplot = new double [(*stats).points];
   yplot = new double [(*stats).points];
   double sum = 0.0, m2 = 0.0, m3 = 0.0, m4 = 0.0;
   for (i=0; i<(*stats).points; i++)
   {
      sum += val[i];
      (*stats).high = max((double)(*stats).high, val[i]);
      (*stats).low = min((double)(*stats).low, val[i]);
      xplot[i] = (double) i;
      yplot[i] = val[i];
   }
   //   cout << "low: " << (*stats).low << ", high: " << (*stats).high << endl;
   (*stats).mean = sum/(*stats).points;
   for (i=0; i<(*stats).points; i++)
   {
      m2 += pow((double) (val[i] - (*stats).mean), (double) 2.0);
      m3 += pow((double) (val[i] - (*stats).mean), (double) 3.0);
      m4 += pow((double) (val[i] - (*stats).mean), (double) 4.0);
   }
   m2 /= (*stats).points;
   m3 /= (*stats).points;
   m4 /= (*stats).points;
   (*stats).skew =  m3 / pow((double) m2, (double) (3.0/2.0));
   (*stats).kurtosis =  m4 / pow((double) m2, (double) 2.0) - 3.0;
   (*stats).median = (*stats).high - ((*stats).high - (*stats).low) / 2.0;
   linefit(&xplot, &yplot, &slope, &intercept, &sigma, &corr, (*stats).points);
   (*stats).correlation = (float) corr;
   (*stats).std_deviation = pow((double) m2, (double) 0.5);
   (*stats).std_error = (*stats).std_deviation / pow((double) (*stats).points, (double) 0.5);
   (*stats).variance = m2;
   (*stats).area = 0.0;
   (*stats).parameter_name = str;
   delete [] xplot;
   delete [] yplot;
   xplot = new double [bins];
   yplot = new double [bins];
   float binsize = ((*stats).high - (*stats).low)/bins;
   //cout << "binsize: " << binsize << endl;
   yplot[0] = 0.0;
   for (i=0; i<bins; i++)
   {
      points = 0;
      xplot[i] = (double) ((*stats).low + (binsize * i));
      yplot[i] = 0.0;
      for (j = 0; j<val.size(); j++)
      {
         //cout << "val[" << j << "]: " << val[j] << ", xplot1: " << xplot[i] << ", xplot2: " << xplot[i] +binsize << endl;
         if (val[j] >= xplot[i] && val[j] < (xplot[i] + binsize))
         {
            yplot[i] += 1.0;
         }
      }
      //cout << "yplot[" << i << "]: " << yplot[i] << endl;
      (*stats).area += yplot[i] * binsize;
   }
   double test = -1.0;
   int this_bin=0;
   for (i=0; i<bins; i++)
   {
      //      cout << "i: " << i << ", yplot: " << yplot[i] << ", test: " << test << endl;
      if (test < yplot[i])
      {
         test = yplot[i];
         this_bin = i;
      }
   }
   (*stats).mode1 = xplot[this_bin];
   (*stats).mode2 = xplot[this_bin] + binsize;
   (*stats).mode_center = ((*stats).mode1 + (*stats).mode2)/2;
   //cout << "This bin: " << this_bin << ", mode1: " << (*stats).mode1 << ", mode2: " << (*stats).mode1
   //<< ", mode center: " << (*stats).mode_center << endl;
   (*stats).conf99low = (*stats).mean - 2.576 * (*stats).std_deviation;
   (*stats).conf99high = (*stats).mean + 2.576 * (*stats).std_deviation;
   (*stats).conf95low = (*stats).mean - 1.96 * (*stats).std_deviation;
   (*stats).conf95high = (*stats).mean + 1.96 * (*stats).std_deviation;
   output  = tr("\n\nResults for the " + str + ":\n\n");
   output += tr("Maximum Value:             ") + s2.sprintf("%6.4e\n", (*stats).high);
   output += tr("Minimum Value:             ") + s2.sprintf("%6.4e\n", (*stats).low);
   output += tr("Mean Value:                ") + s2.sprintf("%6.4e\n", (*stats).mean);
   output += tr("Median Value:              ") + s2.sprintf("%6.4e\n", (*stats).median);
   output += tr("Skew Value:                ") + s2.sprintf("%6.4e\n", (*stats).skew);
   output += tr("Kurtosis Value:            ") + s2.sprintf("%6.4e\n", (*stats).kurtosis);
   output += tr("Lower Mode Limit:          ") + s2.sprintf("%6.4e\n", (*stats).mode1);
   output += tr("Upper Mode Limit:          ") + s2.sprintf("%6.4e\n", (*stats).mode2);
   output += tr("Mode Center:               ") + s2.sprintf("%6.4e\n", (*stats).mode_center);
   output += tr("95% Confidence Limits:     +") + s2.sprintf("%6.4e, -%6.4e\n",
                                                             ((*stats).conf95high - (*stats).mode_center), ((*stats).mode_center - (*stats).conf95low));
   output += tr("99% Confidence Limits:     +") + s2.sprintf("%6.4e, -%6.4e\n",
                                                             ((*stats).conf99high - (*stats).mode_center), ((*stats).mode_center - (*stats).conf99low));
   output += tr("Standard Deviation:        ") + s2.sprintf("%6.4e\n", (*stats).std_deviation);
   output += tr("Standard Error:            ") + s2.sprintf("%6.4e\n", (*stats).std_error);
   output += tr("Variance:                  ") + s2.sprintf("%6.4e\n", (*stats).variance);
   output += tr("Correlation Coefficient:   ") + s2.sprintf("%6.4e\n", (*stats).correlation);
   output += tr("Number of Bins:            ") + s2.sprintf("%6.4e\n", (float) bins);
   output += tr("Distribution Area:         ") + s2.sprintf("%6.4e\n", (*stats).area);
   s1.sprintf(tr(" %e (low),  %e (high)\n"), (*stats).conf95low, (*stats).conf95high);   // the standard error of the distribution
   output += tr("95 % Confidence Interval: ") + s1;
   s1.sprintf(tr(" %e (low),  %e (high)\n"), (*stats).conf99low, (*stats).conf99high);   // the standard error of the distribution
   output += tr("99 % Confidence Interval: ") + s1;
   delete [] xplot;
   delete [] yplot;
   return output;
}

void US_GA_Initialize::print()
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

void US_GA_Initialize::select_autolimit()
{
   if (cb_autolimit->isChecked())
   {
      autolimit = true;
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

void US_GA_Initialize::update_plot_smin(double val)
{
   plot_smin = val;
}

void US_GA_Initialize::update_plot_smax(double val)
{
   plot_smax = val;
}

void US_GA_Initialize::update_plot_fmin(double val)
{
   plot_fmin = val;
}

void US_GA_Initialize::update_plot_fmax(double val)
{
   plot_fmax = val;
}

