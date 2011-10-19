#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"

#define SLASH "/"
#if defined(WIN32)
#  undef SLASH
#  define SLASH "\\"
#endif

US_Hydrodyn_SaxsOptions::US_Hydrodyn_SaxsOptions(struct saxs_options *saxs_options, bool *saxs_options_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->saxs_options_widget = saxs_options_widget;
   this->saxs_options = saxs_options;
   this->us_hydrodyn = us_hydrodyn;
   *saxs_options_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SOMO SAXS/SANS Simulation Options"));
   update_q();
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SaxsOptions::~US_Hydrodyn_SaxsOptions()
{
   *saxs_options_widget = false;
}

void US_Hydrodyn_SaxsOptions::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("SOMO SAXS/SANS Simulation Options:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_saxs_options = new QLabel(tr("SAXS Options:"), this);
   Q_CHECK_PTR(lbl_saxs_options);
   lbl_saxs_options->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_saxs_options->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_saxs_options->setMinimumHeight(minHeight1);
   lbl_saxs_options->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_saxs_options->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_water_e_density = new QLabel(tr(" Water electron density (e / A^3): "), this);
   Q_CHECK_PTR(lbl_water_e_density);
   lbl_water_e_density->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_water_e_density->setMinimumHeight(minHeight1);
   lbl_water_e_density->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_water_e_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_water_e_density = new QwtCounter(this);
   Q_CHECK_PTR(cnt_water_e_density);
   cnt_water_e_density->setRange(0, 10, 0.001);
   cnt_water_e_density->setValue((*saxs_options).water_e_density);
   cnt_water_e_density->setMinimumHeight(minHeight1);
   cnt_water_e_density->setEnabled(true);
   cnt_water_e_density->setNumButtons(3);
   cnt_water_e_density->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_water_e_density->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_water_e_density, SIGNAL(valueChanged(double)), SLOT(update_water_e_density(double)));

   lbl_saxs_iq = new QLabel(tr(" I(q) method: "), this);
   lbl_saxs_iq->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_saxs_iq->setMinimumHeight(minHeight1);
   lbl_saxs_iq->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_saxs_iq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cb_saxs_iq_native_debye = new QCheckBox(this);
   cb_saxs_iq_native_debye->setText(tr("Full "));
   cb_saxs_iq_native_debye->setEnabled(true);
   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_debye->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_native_debye->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_native_debye, SIGNAL(clicked()), this, SLOT(set_saxs_iq_native_debye()));

   cb_saxs_iq_native_hybrid = new QCheckBox(this);
   cb_saxs_iq_native_hybrid->setText(tr("Hybrid "));
   cb_saxs_iq_native_hybrid->setEnabled(true);
   cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
   cb_saxs_iq_native_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_native_hybrid->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_native_hybrid, SIGNAL(clicked()), this, SLOT(set_saxs_iq_native_hybrid()));

   cb_saxs_iq_native_hybrid2 = new QCheckBox(this);
   cb_saxs_iq_native_hybrid2->setText(tr("H2 "));
   cb_saxs_iq_native_hybrid2->setEnabled(true);
   cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
   cb_saxs_iq_native_hybrid2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_native_hybrid2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_native_hybrid2, SIGNAL(clicked()), this, SLOT(set_saxs_iq_native_hybrid2()));

   cb_saxs_iq_native_hybrid3 = new QCheckBox(this);
   cb_saxs_iq_native_hybrid3->setText(tr("H3 "));
   cb_saxs_iq_native_hybrid3->setEnabled(true);
   cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   cb_saxs_iq_native_hybrid3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_native_hybrid3->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_native_hybrid3, SIGNAL(clicked()), this, SLOT(set_saxs_iq_native_hybrid3()));

   cb_saxs_iq_hybrid_adaptive = new QCheckBox(this);
   cb_saxs_iq_hybrid_adaptive->setText(tr("Adaptive "));
   cb_saxs_iq_hybrid_adaptive->setEnabled(true);
   cb_saxs_iq_hybrid_adaptive->setChecked((*saxs_options).saxs_iq_hybrid_adaptive);
   cb_saxs_iq_hybrid_adaptive->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_hybrid_adaptive->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_hybrid_adaptive, SIGNAL(clicked()), this, SLOT(set_saxs_iq_hybrid_adaptive()));

   cb_saxs_iq_native_fast = new QCheckBox(this);
   cb_saxs_iq_native_fast->setText(tr("Fast "));
   cb_saxs_iq_native_fast->setEnabled(true);
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_native_fast->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_native_fast->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_native_fast, SIGNAL(clicked()), this, SLOT(set_saxs_iq_native_fast()));

   cb_saxs_iq_native_fast_compute_pr = new QCheckBox(this);
   cb_saxs_iq_native_fast_compute_pr->setText(tr("P(r) "));
   cb_saxs_iq_native_fast_compute_pr->setEnabled(true);
   cb_saxs_iq_native_fast_compute_pr->setChecked((*saxs_options).saxs_iq_native_fast_compute_pr);
   cb_saxs_iq_native_fast_compute_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_native_fast_compute_pr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_native_fast_compute_pr, SIGNAL(clicked()), this, SLOT(set_saxs_iq_native_fast_compute_pr()));

   cb_saxs_iq_foxs = new QCheckBox(this);
   cb_saxs_iq_foxs->setText(tr("FoXS"));
   cb_saxs_iq_foxs->setEnabled(true);
   cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);
   cb_saxs_iq_foxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_foxs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_foxs, SIGNAL(clicked()), this, SLOT(set_saxs_iq_foxs()));

   cb_saxs_iq_crysol = new QCheckBox(this);
   cb_saxs_iq_crysol->setText(tr("Crysol"));
   cb_saxs_iq_crysol->setEnabled(true);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   cb_saxs_iq_crysol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_iq_crysol->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_iq_crysol, SIGNAL(clicked()), this, SLOT(set_saxs_iq_crysol()));

   lbl_fast_bin_size = new QLabel(tr(" Fast Debye: Bin size"), this);
   lbl_fast_bin_size->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_fast_bin_size->setMinimumHeight(minHeight1);
   lbl_fast_bin_size->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_fast_bin_size->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_fast_bin_size = new QwtCounter(this);
   cnt_fast_bin_size->setRange(0.01, 100, 0.01);
   cnt_fast_bin_size->setValue((*saxs_options).fast_bin_size);
   cnt_fast_bin_size->setMinimumHeight(minHeight1);
   cnt_fast_bin_size->setEnabled(true);
   cnt_fast_bin_size->setNumButtons(2);
   cnt_fast_bin_size->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_fast_bin_size->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_fast_bin_size, SIGNAL(valueChanged(double)), SLOT(update_fast_bin_size(double)));

   lbl_fast_modulation = new QLabel(tr(" Fast Debye: Modulation"), this);
   lbl_fast_modulation->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_fast_modulation->setMinimumHeight(minHeight1);
   lbl_fast_modulation->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_fast_modulation->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_fast_modulation = new QwtCounter(this);
   cnt_fast_modulation->setRange(0.1, 0.5, 0.001);
   cnt_fast_modulation->setValue((*saxs_options).fast_modulation);
   cnt_fast_modulation->setMinimumHeight(minHeight1);
   cnt_fast_modulation->setEnabled(true);
   cnt_fast_modulation->setNumButtons(3);
   cnt_fast_modulation->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_fast_modulation->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_fast_modulation, SIGNAL(valueChanged(double)), SLOT(update_fast_modulation(double)));

   lbl_hybrid2_q_points = new QLabel(tr(" Hybrid, H2, H3: q points"), this);
   lbl_hybrid2_q_points->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_hybrid2_q_points->setMinimumHeight(minHeight1);
   lbl_hybrid2_q_points->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_hybrid2_q_points->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_hybrid2_q_points = new QwtCounter(this);
   cnt_hybrid2_q_points->setRange(3, 100, 1);
   cnt_hybrid2_q_points->setValue((*saxs_options).hybrid2_q_points);
   cnt_hybrid2_q_points->setMinimumHeight(minHeight1);
   cnt_hybrid2_q_points->setEnabled(true);
   cnt_hybrid2_q_points->setNumButtons(3);
   cnt_hybrid2_q_points->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_hybrid2_q_points->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_hybrid2_q_points, SIGNAL(valueChanged(double)), SLOT(update_hybrid2_q_points(double)));

   lbl_crysol_max_harmonics = new QLabel(tr(" Crysol: Maximum order of harmonics"), this);
   lbl_crysol_max_harmonics->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_crysol_max_harmonics->setMinimumHeight(minHeight1);
   lbl_crysol_max_harmonics->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_crysol_max_harmonics->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_crysol_max_harmonics = new QwtCounter(this);
   cnt_crysol_max_harmonics->setRange(10, 75, 1);
   cnt_crysol_max_harmonics->setValue((*saxs_options).crysol_max_harmonics);
   cnt_crysol_max_harmonics->setMinimumHeight(minHeight1);
   cnt_crysol_max_harmonics->setEnabled(true);
   cnt_crysol_max_harmonics->setNumButtons(2);
   cnt_crysol_max_harmonics->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_crysol_max_harmonics->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_crysol_max_harmonics, SIGNAL(valueChanged(double)), SLOT(update_crysol_max_harmonics(double)));

   lbl_crysol_fibonacci_grid_order = new QLabel(tr(" Crysol: Order of Fibonacci grid"), this);
   lbl_crysol_fibonacci_grid_order->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_crysol_fibonacci_grid_order->setMinimumHeight(minHeight1);
   lbl_crysol_fibonacci_grid_order->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_crysol_fibonacci_grid_order->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_crysol_fibonacci_grid_order = new QwtCounter(this);
   cnt_crysol_fibonacci_grid_order->setRange(10, 75, 1);
   cnt_crysol_fibonacci_grid_order->setValue((*saxs_options).crysol_fibonacci_grid_order);
   cnt_crysol_fibonacci_grid_order->setMinimumHeight(minHeight1);
   cnt_crysol_fibonacci_grid_order->setEnabled(true);
   cnt_crysol_fibonacci_grid_order->setNumButtons(2);
   cnt_crysol_fibonacci_grid_order->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_crysol_fibonacci_grid_order->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_crysol_fibonacci_grid_order, SIGNAL(valueChanged(double)), SLOT(update_crysol_fibonacci_grid_order(double)));

   lbl_crysol_hydration_shell_contrast = new QLabel(tr(" Crysol: Contrast of hydration shell (e / A^3):"), this);
   lbl_crysol_hydration_shell_contrast->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_crysol_hydration_shell_contrast->setMinimumHeight(minHeight1);
   lbl_crysol_hydration_shell_contrast->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_crysol_hydration_shell_contrast->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_crysol_hydration_shell_contrast = new QwtCounter(this);
   cnt_crysol_hydration_shell_contrast->setRange(-1, 1, 0.001);
   cnt_crysol_hydration_shell_contrast->setValue((*saxs_options).crysol_hydration_shell_contrast);
   cnt_crysol_hydration_shell_contrast->setMinimumHeight(minHeight1);
   cnt_crysol_hydration_shell_contrast->setEnabled(true);
   cnt_crysol_hydration_shell_contrast->setNumButtons(3);
   cnt_crysol_hydration_shell_contrast->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_crysol_hydration_shell_contrast->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_crysol_hydration_shell_contrast, SIGNAL(valueChanged(double)), SLOT(update_crysol_hydration_shell_contrast(double)));

   cb_crysol_default_load_difference_intensity = new QCheckBox(this);
   cb_crysol_default_load_difference_intensity->setText(tr("Crysol: automatically load difference intensity"));
   cb_crysol_default_load_difference_intensity->setEnabled(true);
   cb_crysol_default_load_difference_intensity->setChecked((*saxs_options).crysol_default_load_difference_intensity);
   cb_crysol_default_load_difference_intensity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_crysol_default_load_difference_intensity->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_crysol_default_load_difference_intensity, SIGNAL(clicked()), this, SLOT(set_crysol_default_load_difference_intensity()));

   cb_crysol_version_26 = new QCheckBox(this);
   cb_crysol_version_26->setText(tr("Crysol: support version 2.6"));
   cb_crysol_version_26->setEnabled(true);
   cb_crysol_version_26->setChecked((*saxs_options).crysol_version_26);
   cb_crysol_version_26->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_crysol_version_26->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_crysol_version_26, SIGNAL(clicked()), this, SLOT(set_crysol_version_26()));

   lbl_sans_options = new QLabel(tr("SANS Options:"), this);
   Q_CHECK_PTR(lbl_sans_options);
   lbl_sans_options->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_sans_options->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_sans_options->setMinimumHeight(minHeight1);
   lbl_sans_options->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_sans_options->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_h_scat_len = new QLabel(tr(" H scattering length (*10^-12 cm): "), this);
   Q_CHECK_PTR(lbl_h_scat_len);
   lbl_h_scat_len->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_h_scat_len->setMinimumHeight(minHeight1);
   lbl_h_scat_len->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_h_scat_len->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_h_scat_len = new QwtCounter(this);
   Q_CHECK_PTR(cnt_h_scat_len);
   cnt_h_scat_len->setRange(-0.5, 0.5, 0.0001);
   cnt_h_scat_len->setValue((*saxs_options).h_scat_len);
   cnt_h_scat_len->setMinimumHeight(minHeight1);
   cnt_h_scat_len->setEnabled(true);
   cnt_h_scat_len->setNumButtons(3);
   cnt_h_scat_len->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_h_scat_len->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_h_scat_len, SIGNAL(valueChanged(double)), SLOT(update_h_scat_len(double)));

   lbl_d_scat_len = new QLabel(tr(" D scattering length (*10^-12 cm): "), this);
   Q_CHECK_PTR(lbl_d_scat_len);
   lbl_d_scat_len->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_d_scat_len->setMinimumHeight(minHeight1);
   lbl_d_scat_len->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_d_scat_len->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_d_scat_len = new QwtCounter(this);
   Q_CHECK_PTR(cnt_d_scat_len);
   cnt_d_scat_len->setRange(0.1, 1.0, 0.0001);
   cnt_d_scat_len->setValue((*saxs_options).d_scat_len);
   cnt_d_scat_len->setMinimumHeight(minHeight1);
   cnt_d_scat_len->setEnabled(true);
   cnt_d_scat_len->setNumButtons(3);
   cnt_d_scat_len->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_d_scat_len->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_d_scat_len, SIGNAL(valueChanged(double)), SLOT(update_d_scat_len(double)));

   lbl_h2o_scat_len_dens = new QLabel(tr(" H2O scattering length density (*10^-10 cm^2): "), this);
   Q_CHECK_PTR(lbl_h2o_scat_len_dens);
   lbl_h2o_scat_len_dens->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_h2o_scat_len_dens->setMinimumHeight(minHeight1);
   lbl_h2o_scat_len_dens->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_h2o_scat_len_dens->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_h2o_scat_len_dens = new QwtCounter(this);
   Q_CHECK_PTR(cnt_h2o_scat_len_dens);
   cnt_h2o_scat_len_dens->setRange(-0.9, 0.0, 0.001);
   cnt_h2o_scat_len_dens->setValue((*saxs_options).h2o_scat_len_dens);
   cnt_h2o_scat_len_dens->setMinimumHeight(minHeight1);
   cnt_h2o_scat_len_dens->setEnabled(true);
   cnt_h2o_scat_len_dens->setNumButtons(3);
   cnt_h2o_scat_len_dens->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_h2o_scat_len_dens->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_h2o_scat_len_dens, SIGNAL(valueChanged(double)), SLOT(update_h2o_scat_len_dens(double)));

   lbl_d2o_scat_len_dens = new QLabel(tr(" D2O scattering length density (*10^-10 cm^2): "), this);
   Q_CHECK_PTR(lbl_d2o_scat_len_dens);
   lbl_d2o_scat_len_dens->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_d2o_scat_len_dens->setMinimumHeight(minHeight1);
   lbl_d2o_scat_len_dens->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_d2o_scat_len_dens->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_d2o_scat_len_dens = new QwtCounter(this);
   Q_CHECK_PTR(cnt_d2o_scat_len_dens);
   cnt_d2o_scat_len_dens->setRange(0.0, 10.0, 0.001);
   cnt_d2o_scat_len_dens->setValue((*saxs_options).d2o_scat_len_dens);
   cnt_d2o_scat_len_dens->setMinimumHeight(minHeight1);
   cnt_d2o_scat_len_dens->setEnabled(true);
   cnt_d2o_scat_len_dens->setNumButtons(3);
   cnt_d2o_scat_len_dens->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_d2o_scat_len_dens->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_d2o_scat_len_dens, SIGNAL(valueChanged(double)), SLOT(update_d2o_scat_len_dens(double)));

   lbl_d2o_conc = new QLabel(tr(" Buffer D2O fraction (0 - 1): "), this);
   Q_CHECK_PTR(lbl_d2o_conc);
   lbl_d2o_conc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_d2o_conc->setMinimumHeight(minHeight1);
   lbl_d2o_conc->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_d2o_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_d2o_conc = new QwtCounter(this);
   Q_CHECK_PTR(cnt_d2o_conc);
   cnt_d2o_conc->setRange(0.0, 1.0, 0.01);
   cnt_d2o_conc->setValue((*saxs_options).d2o_conc);
   cnt_d2o_conc->setMinimumHeight(minHeight1);
   cnt_d2o_conc->setEnabled(true);
   cnt_d2o_conc->setNumButtons(3);
   cnt_d2o_conc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_d2o_conc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_d2o_conc, SIGNAL(valueChanged(double)), SLOT(update_d2o_conc(double)));

   lbl_frac_of_exch_pep = new QLabel(tr(" Fraction of non-exchanged peptide H (0 - 1): "), this);
   Q_CHECK_PTR(lbl_frac_of_exch_pep);
   lbl_frac_of_exch_pep->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_frac_of_exch_pep->setMinimumHeight(minHeight1);
   lbl_frac_of_exch_pep->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_frac_of_exch_pep->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_frac_of_exch_pep = new QwtCounter(this);
   Q_CHECK_PTR(cnt_frac_of_exch_pep);
   cnt_frac_of_exch_pep->setRange(0.0, 1.0, 0.01);
   cnt_frac_of_exch_pep->setValue((*saxs_options).frac_of_exch_pep);
   cnt_frac_of_exch_pep->setMinimumHeight(minHeight1);
   cnt_frac_of_exch_pep->setEnabled(true);
   cnt_frac_of_exch_pep->setNumButtons(3);
   cnt_frac_of_exch_pep->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_frac_of_exch_pep->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_frac_of_exch_pep, SIGNAL(valueChanged(double)), SLOT(update_frac_of_exch_pep(double)));

   lbl_sans_iq = new QLabel(tr(" I(q) method: "), this);
   lbl_sans_iq->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_sans_iq->setMinimumHeight(minHeight1);
   lbl_sans_iq->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sans_iq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cb_sans_iq_native_debye = new QCheckBox(this);
   cb_sans_iq_native_debye->setText(tr("Full "));
   cb_sans_iq_native_debye->setEnabled(true);
   cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
   cb_sans_iq_native_debye->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_sans_iq_native_debye->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_sans_iq_native_debye, SIGNAL(clicked()), this, SLOT(set_sans_iq_native_debye()));

   cb_sans_iq_native_hybrid = new QCheckBox(this);
   cb_sans_iq_native_hybrid->setText(tr("Hybrid "));
   cb_sans_iq_native_hybrid->setEnabled(true);
   cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
   cb_sans_iq_native_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_sans_iq_native_hybrid->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_sans_iq_native_hybrid, SIGNAL(clicked()), this, SLOT(set_sans_iq_native_hybrid()));

   cb_sans_iq_native_hybrid2 = new QCheckBox(this);
   cb_sans_iq_native_hybrid2->setText(tr("H2 "));
   cb_sans_iq_native_hybrid2->setEnabled(true);
   cb_sans_iq_native_hybrid2->setChecked((*saxs_options).sans_iq_native_hybrid2);
   cb_sans_iq_native_hybrid2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_sans_iq_native_hybrid2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_sans_iq_native_hybrid2, SIGNAL(clicked()), this, SLOT(set_sans_iq_native_hybrid2()));

   cb_sans_iq_native_hybrid3 = new QCheckBox(this);
   cb_sans_iq_native_hybrid3->setText(tr("H3 "));
   cb_sans_iq_native_hybrid3->setEnabled(true);
   cb_sans_iq_native_hybrid3->setChecked((*saxs_options).sans_iq_native_hybrid3);
   cb_sans_iq_native_hybrid3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_sans_iq_native_hybrid3->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_sans_iq_native_hybrid3, SIGNAL(clicked()), this, SLOT(set_sans_iq_native_hybrid3()));

   cb_sans_iq_hybrid_adaptive = new QCheckBox(this);
   cb_sans_iq_hybrid_adaptive->setText(tr("Adaptive "));
   cb_sans_iq_hybrid_adaptive->setEnabled(true);
   cb_sans_iq_hybrid_adaptive->setChecked((*saxs_options).sans_iq_hybrid_adaptive);
   cb_sans_iq_hybrid_adaptive->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_sans_iq_hybrid_adaptive->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_sans_iq_hybrid_adaptive, SIGNAL(clicked()), this, SLOT(set_sans_iq_hybrid_adaptive()));

   cb_sans_iq_native_fast = new QCheckBox(this);
   cb_sans_iq_native_fast->setText(tr("Fast "));
   cb_sans_iq_native_fast->setEnabled(true);
   cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
   cb_sans_iq_native_fast->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_sans_iq_native_fast->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_sans_iq_native_fast, SIGNAL(clicked()), this, SLOT(set_sans_iq_native_fast()));

   cb_sans_iq_native_fast_compute_pr = new QCheckBox(this);
   cb_sans_iq_native_fast_compute_pr->setText(tr("P(r) "));
   cb_sans_iq_native_fast_compute_pr->setEnabled(true);
   cb_sans_iq_native_fast_compute_pr->setChecked((*saxs_options).sans_iq_native_fast_compute_pr);
   cb_sans_iq_native_fast_compute_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_sans_iq_native_fast_compute_pr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_sans_iq_native_fast_compute_pr, SIGNAL(clicked()), this, SLOT(set_sans_iq_native_fast_compute_pr()));

   cb_sans_iq_cryson = new QCheckBox(this);
   cb_sans_iq_cryson->setText(tr("Cryson"));
   cb_sans_iq_cryson->setEnabled(true);
   cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);
   cb_sans_iq_cryson->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_sans_iq_cryson->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_sans_iq_cryson, SIGNAL(clicked()), this, SLOT(set_sans_iq_cryson()));

   // -------------- guinier section ------------

   lbl_guinier = new QLabel(tr("Guinier Options:"), this);
   Q_CHECK_PTR(lbl_guinier);
   lbl_guinier->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_guinier->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_guinier->setMinimumHeight(minHeight1);
   lbl_guinier->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_guinier_csv = new QCheckBox(this);
   cb_guinier_csv->setText(tr(" Save Guinier results to csv file: "));
   cb_guinier_csv->setEnabled(true);
   cb_guinier_csv->setChecked((*saxs_options).guinier_csv);
   cb_guinier_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_guinier_csv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_guinier_csv, SIGNAL(clicked()), this, SLOT(set_guinier_csv()));

   le_guinier_csv_filename = new QLineEdit(this, "guinier_csv_filename Line Edit");
   le_guinier_csv_filename->setText((*saxs_options).guinier_csv_filename);
   // le_guinier_csv_filename->setMinimumHeight(minHeight1);
   le_guinier_csv_filename->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_csv_filename->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_guinier_csv_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_guinier_csv_filename, SIGNAL(textChanged(const QString &)), SLOT(update_guinier_csv_filename(const QString &)));

   lbl_qRgmax = new QLabel(tr(" Maximum q * Rg : "), this);
   Q_CHECK_PTR(lbl_qRgmax);
   lbl_qRgmax->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_qRgmax->setMinimumHeight(minHeight1);
   lbl_qRgmax->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_qRgmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_qRgmax = new QwtCounter(this);
   Q_CHECK_PTR(cnt_qRgmax);
   cnt_qRgmax->setRange(0.5, 3, 0.001);
   cnt_qRgmax->setValue((*saxs_options).qRgmax);
   cnt_qRgmax->setMinimumHeight(minHeight1);
   cnt_qRgmax->setEnabled(true);
   cnt_qRgmax->setNumButtons(SAXS_Q_BUTTONS);
   cnt_qRgmax->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_qRgmax->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_qRgmax, SIGNAL(valueChanged(double)), SLOT(update_qRgmax(double)));

   lbl_qend = new QLabel(tr(" Maximum q value for Guinier search : "), this);
   Q_CHECK_PTR(lbl_qend);
   lbl_qend->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_qend->setMinimumHeight(minHeight1);
   lbl_qend->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_qend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_qend = new QwtCounter(this);
   Q_CHECK_PTR(cnt_qend);
   cnt_qend->setRange(0, 1, 0.001);
   cnt_qend->setValue((*saxs_options).qend);
   cnt_qend->setMinimumHeight(minHeight1);
   cnt_qend->setEnabled(true);
   cnt_qend->setNumButtons(SAXS_Q_BUTTONS);
   cnt_qend->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_qend->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_qend, SIGNAL(valueChanged(double)), SLOT(update_qend(double)));

   lbl_pointsmin = new QLabel(tr(" Minimum number of points : "), this);
   Q_CHECK_PTR(lbl_pointsmin);
   lbl_pointsmin->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pointsmin->setMinimumHeight(minHeight1);
   lbl_pointsmin->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pointsmin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_pointsmin = new QwtCounter(this);
   Q_CHECK_PTR(cnt_pointsmin);
   cnt_pointsmin->setRange(4, 50, 1);
   cnt_pointsmin->setValue((*saxs_options).pointsmin);
   cnt_pointsmin->setMinimumHeight(minHeight1);
   cnt_pointsmin->setEnabled(true);
   cnt_pointsmin->setNumButtons(SAXS_Q_BUTTONS);
   cnt_pointsmin->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_pointsmin->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_pointsmin, SIGNAL(valueChanged(double)), SLOT(update_pointsmin(double)));

   lbl_pointsmax = new QLabel(tr(" Maximum number of points : "), this);
   Q_CHECK_PTR(lbl_pointsmax);
   lbl_pointsmax->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pointsmax->setMinimumHeight(minHeight1);
   lbl_pointsmax->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pointsmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_pointsmax = new QwtCounter(this);
   Q_CHECK_PTR(cnt_pointsmax);
   cnt_pointsmax->setRange(10, 100, 1);
   cnt_pointsmax->setValue((*saxs_options).pointsmax);
   cnt_pointsmax->setMinimumHeight(minHeight1);
   cnt_pointsmax->setEnabled(true);
   cnt_pointsmax->setNumButtons(SAXS_Q_BUTTONS);
   cnt_pointsmax->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_pointsmax->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_pointsmax, SIGNAL(valueChanged(double)), SLOT(update_pointsmax(double)));

   // -------------- curve generation section ------------

   lbl_curve = new QLabel(tr("Curve Generation Options:"), this);
   Q_CHECK_PTR(lbl_curve);
   lbl_curve->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_curve->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_curve->setMinimumHeight(minHeight1);
   lbl_curve->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_curve->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_wavelength = new QLabel(tr(" Wavelength (Angstrom): "), this);
   Q_CHECK_PTR(lbl_wavelength);
   lbl_wavelength->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_wavelength->setMinimumHeight(minHeight1);
   lbl_wavelength->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_wavelength->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_wavelength= new QwtCounter(this);
   Q_CHECK_PTR(cnt_wavelength);
   cnt_wavelength->setRange(0, 10, 0.01);
   cnt_wavelength->setValue((*saxs_options).wavelength);
   cnt_wavelength->setMinimumHeight(minHeight1);
   cnt_wavelength->setEnabled(true);
   cnt_wavelength->setNumButtons(3);
   cnt_wavelength->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_wavelength->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_wavelength, SIGNAL(valueChanged(double)), SLOT(update_wavelength(double)));

   lbl_start_angle = new QLabel(tr(" Starting Angle: "), this);
   Q_CHECK_PTR(lbl_start_angle);
   lbl_start_angle->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_start_angle->setMinimumHeight(minHeight1);
   lbl_start_angle->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_start_angle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_start_angle= new QwtCounter(this);
   Q_CHECK_PTR(cnt_start_angle);
   cnt_start_angle->setRange(0, 90, 1.0f/SAXS_Q_ROUNDING);
   cnt_start_angle->setValue((*saxs_options).start_angle);
   cnt_start_angle->setMinimumHeight(minHeight1);
   cnt_start_angle->setEnabled(true);
   cnt_start_angle->setNumButtons(SAXS_Q_BUTTONS);
   cnt_start_angle->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_start_angle->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_start_angle, SIGNAL(valueChanged(double)), SLOT(update_start_angle(double)));

   lbl_end_angle = new QLabel(tr(" Ending Angle: "), this);
   Q_CHECK_PTR(lbl_end_angle);
   lbl_end_angle->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_end_angle->setMinimumHeight(minHeight1);
   lbl_end_angle->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_end_angle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_end_angle= new QwtCounter(this);
   Q_CHECK_PTR(cnt_end_angle);
   cnt_end_angle->setRange(0, 90, 1.0f/SAXS_Q_ROUNDING);
   cnt_end_angle->setValue((*saxs_options).end_angle);
   cnt_end_angle->setMinimumHeight(minHeight1);
   cnt_end_angle->setEnabled(true);
   cnt_end_angle->setNumButtons(SAXS_Q_BUTTONS);
   cnt_end_angle->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_end_angle->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_end_angle, SIGNAL(valueChanged(double)), SLOT(update_end_angle(double)));

   lbl_delta_angle = new QLabel(tr(" Angle Stepsize: "), this);
   Q_CHECK_PTR(lbl_delta_angle);
   lbl_delta_angle->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_delta_angle->setMinimumHeight(minHeight1);
   lbl_delta_angle->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_delta_angle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_delta_angle= new QwtCounter(this);
   Q_CHECK_PTR(cnt_delta_angle);
   cnt_delta_angle->setRange(0.0001, 90, 1.0f/SAXS_Q_ROUNDING);
   cnt_delta_angle->setValue((*saxs_options).delta_angle);
   cnt_delta_angle->setMinimumHeight(minHeight1);
   cnt_delta_angle->setEnabled(true);
   cnt_delta_angle->setNumButtons(SAXS_Q_BUTTONS);
   cnt_delta_angle->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_delta_angle->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_delta_angle, SIGNAL(valueChanged(double)), SLOT(update_delta_angle(double)));

   lbl_start_q = new QLabel(tr(" Starting q: "), this);
   Q_CHECK_PTR(lbl_start_q);
   lbl_start_q->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_start_q->setMinimumHeight(minHeight1);
   lbl_start_q->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_start_q->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_start_q = new QwtCounter(this);
   Q_CHECK_PTR(cnt_start_q);
   cnt_start_q->setRange(0, 90, 1.0f/SAXS_Q_ROUNDING);
   cnt_start_q->setValue((*saxs_options).start_q);
   cnt_start_q->setMinimumHeight(minHeight1);
   cnt_start_q->setEnabled(true);
   cnt_start_q->setNumButtons(SAXS_Q_BUTTONS);
   cnt_start_q->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_start_q->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_start_q, SIGNAL(valueChanged(double)), SLOT(update_start_q(double)));

   lbl_end_q = new QLabel(tr(" Ending q: "), this);
   Q_CHECK_PTR(lbl_end_q);
   lbl_end_q->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_end_q->setMinimumHeight(minHeight1);
   lbl_end_q->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_end_q->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_end_q = new QwtCounter(this);
   Q_CHECK_PTR(cnt_end_q);
   cnt_end_q->setRange(0, 90, 1.0f/SAXS_Q_ROUNDING);
   cnt_end_q->setValue((*saxs_options).end_q);
   cnt_end_q->setMinimumHeight(minHeight1);
   cnt_end_q->setEnabled(true);
   cnt_end_q->setNumButtons(SAXS_Q_BUTTONS);
   cnt_end_q->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_end_q->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_end_q, SIGNAL(valueChanged(double)), SLOT(update_end_q(double)));

   lbl_delta_q = new QLabel(tr(" q Stepsize: "), this);
   Q_CHECK_PTR(lbl_delta_q);
   lbl_delta_q->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_delta_q->setMinimumHeight(minHeight1);
   lbl_delta_q->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_delta_q->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_delta_q = new QwtCounter(this);
   Q_CHECK_PTR(cnt_delta_q);
   cnt_delta_q->setRange(0.0001, 90, 1.0f/SAXS_Q_ROUNDING);
   cnt_delta_q->setValue((*saxs_options).delta_q);
   cnt_delta_q->setMinimumHeight(minHeight1);
   cnt_delta_q->setEnabled(true);
   cnt_delta_q->setNumButtons(SAXS_Q_BUTTONS);
   cnt_delta_q->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_delta_q->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_delta_q, SIGNAL(valueChanged(double)), SLOT(update_delta_q(double)));

   cb_hydrate_pdb = new QCheckBox(this);
   cb_hydrate_pdb->setText(tr(" Hydrate the Original Model (PDB files only)"));
   cb_hydrate_pdb->setEnabled(true);
   cb_hydrate_pdb->setChecked((*saxs_options).hydrate_pdb);
   cb_hydrate_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydrate_pdb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_hydrate_pdb, SIGNAL(clicked()), this, SLOT(set_hydrate_pdb()));
   set_hydrate_pdb();

   cb_normalize_by_mw = new QCheckBox(this);
   cb_normalize_by_mw->setText(tr(" Normalize P(r) vs r curve by molecular weight"));
   cb_normalize_by_mw->setEnabled(true);
   cb_normalize_by_mw->setChecked((*saxs_options).normalize_by_mw);
   cb_normalize_by_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_normalize_by_mw->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_normalize_by_mw, SIGNAL(clicked()), this, SLOT(set_normalize_by_mw()));

   lbl_bead_model_control = new QLabel(tr("Bead Model Control:"), this);
   lbl_bead_model_control->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_bead_model_control->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_bead_model_control->setMinimumHeight(minHeight1);
   lbl_bead_model_control->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_bead_model_control->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_compute_saxs_coeff_for_bead_models = new QCheckBox(this);
   cb_compute_saxs_coeff_for_bead_models->setText(tr(" Compute SAXS coefficients for bead models"));
   cb_compute_saxs_coeff_for_bead_models->setEnabled(true);
   cb_compute_saxs_coeff_for_bead_models->setChecked((*saxs_options).compute_saxs_coeff_for_bead_models);
   cb_compute_saxs_coeff_for_bead_models->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_saxs_coeff_for_bead_models->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_saxs_coeff_for_bead_models, SIGNAL(clicked()), this, SLOT(set_compute_saxs_coeff_for_bead_models()));

   cb_compute_sans_coeff_for_bead_models = new QCheckBox(this);
   cb_compute_sans_coeff_for_bead_models->setText(tr(" Compute SANS coefficients for bead models"));
   cb_compute_sans_coeff_for_bead_models->setEnabled(true);
   cb_compute_sans_coeff_for_bead_models->setChecked((*saxs_options).compute_sans_coeff_for_bead_models);
   cb_compute_sans_coeff_for_bead_models->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_sans_coeff_for_bead_models->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_sans_coeff_for_bead_models, SIGNAL(clicked()), this, SLOT(set_compute_sans_coeff_for_bead_models()));

   cb_bead_model_rayleigh = new QCheckBox(this);
   cb_bead_model_rayleigh->setText(tr(" Use Rayleigh (1911) for structure factors"));
   cb_bead_model_rayleigh->setEnabled(true);
   cb_bead_model_rayleigh->setChecked((*saxs_options).bead_model_rayleigh);
   cb_bead_model_rayleigh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_bead_model_rayleigh->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_bead_model_rayleigh, SIGNAL(clicked()), this, SLOT(set_bead_model_rayleigh()));

   pb_default_atom_filename = new QPushButton(tr("Set Atom Definition File"), this);
   pb_default_atom_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_default_atom_filename->setMinimumHeight(minHeight1);
   pb_default_atom_filename->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_default_atom_filename, SIGNAL(clicked()), SLOT(default_atom_filename()));
   
   le_default_atom_filename = new QLineEdit(this, "");
   le_default_atom_filename->setText(QFileInfo((*saxs_options).default_atom_filename).fileName());
   // le_default_atom_filename->setMinimumHeight(minHeight1);
   // le_default_atom_filename->setMaximumHeight(minHeight1);
   le_default_atom_filename->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_default_atom_filename->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   le_default_atom_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_default_atom_filename->setReadOnly(true);

   pb_default_hybrid_filename = new QPushButton(tr("Set Hybridization File"), this);
   pb_default_hybrid_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_default_hybrid_filename->setMinimumHeight(minHeight1);
   pb_default_hybrid_filename->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_default_hybrid_filename, SIGNAL(clicked()), SLOT(default_hybrid_filename()));

   le_default_hybrid_filename = new QLineEdit(this, "");
   le_default_hybrid_filename->setText(QFileInfo((*saxs_options).default_hybrid_filename).fileName());
   // le_default_hybrid_filename->setMinimumHeight(minHeight1);
   // le_default_hybrid_filename->setMaximumHeight(minHeight1);
   le_default_hybrid_filename->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_default_hybrid_filename->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   le_default_hybrid_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_default_hybrid_filename->setReadOnly(true);

   pb_default_saxs_filename = new QPushButton(tr("Set SAXS Coefficients File"), this);
   pb_default_saxs_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_default_saxs_filename->setMinimumHeight(minHeight1);
   pb_default_saxs_filename->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_default_saxs_filename, SIGNAL(clicked()), SLOT(default_saxs_filename()));

   le_default_saxs_filename = new QLineEdit(this, "");
   le_default_saxs_filename->setText(QFileInfo((*saxs_options).default_saxs_filename).fileName());
   // le_default_saxs_filename->setMinimumHeight(minHeight1);
   // le_default_saxs_filename->setMaximumHeight(minHeight1);
   le_default_saxs_filename->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_default_saxs_filename->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   le_default_saxs_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_default_saxs_filename->setReadOnly(true);

   pb_default_rotamer_filename = new QPushButton(tr("Set Hydrated Rotamer File"), this);
   pb_default_rotamer_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_default_rotamer_filename->setMinimumHeight(minHeight1);
   pb_default_rotamer_filename->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_default_rotamer_filename, SIGNAL(clicked()), SLOT(default_rotamer_filename()));

   le_default_rotamer_filename = new QLineEdit(this, "");
   le_default_rotamer_filename->setText(QFileInfo((*saxs_options).default_rotamer_filename).fileName());
   // le_default_rotamer_filename->setMinimumHeight(minHeight1);
   // le_default_rotamer_filename->setMaximumHeight(minHeight1);
   le_default_rotamer_filename->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_default_rotamer_filename->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   le_default_rotamer_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_default_rotamer_filename->setReadOnly(true);

   lbl_misc = new QLabel(tr("Miscellaneous Options:"), this);
   lbl_misc->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_misc->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_misc->setMinimumHeight(minHeight1);
   lbl_misc->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_misc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_steric_clash_distance = new QLabel(tr(" Steric clash minimum distance : "), this);
   lbl_steric_clash_distance->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_steric_clash_distance->setMinimumHeight(minHeight1);
   lbl_steric_clash_distance->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_steric_clash_distance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_steric_clash_distance = new QwtCounter(this);
   cnt_steric_clash_distance->setRange(0.0, 10, 0.1);
   cnt_steric_clash_distance->setValue((*saxs_options).steric_clash_distance);
   cnt_steric_clash_distance->setMinimumHeight(minHeight1);
   cnt_steric_clash_distance->setEnabled(true);
   cnt_steric_clash_distance->setNumButtons(2);
   cnt_steric_clash_distance->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_steric_clash_distance->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_steric_clash_distance, SIGNAL(valueChanged(double)), SLOT(update_steric_clash_distance(double)));

   cb_iq_ask = new QCheckBox(this);
   cb_iq_ask->setText(tr("Manually choose I(q) method"));
   cb_iq_ask->setEnabled(true);
   cb_iq_ask->setChecked((*saxs_options).iq_ask);
   cb_iq_ask->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iq_ask->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iq_ask, SIGNAL(clicked()), this, SLOT(set_iq_ask()));

   cb_iq_scale_ask = new QCheckBox(this);
   cb_iq_scale_ask->setText(tr("Always ask angstrom or nm"));
   cb_iq_scale_ask->setEnabled(true);
   cb_iq_scale_ask->setChecked((*saxs_options).iq_scale_ask);
   cb_iq_scale_ask->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iq_scale_ask->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iq_scale_ask, SIGNAL(clicked()), this, SLOT(set_iq_scale_ask()));

   cb_iqq_ask_target_grid = new QCheckBox(this);
   cb_iqq_ask_target_grid->setText(tr("Calc I(q) ask for grid"));
   cb_iqq_ask_target_grid->setEnabled(true);
   cb_iqq_ask_target_grid->setChecked((*saxs_options).iqq_ask_target_grid);
   cb_iqq_ask_target_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_ask_target_grid->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq_ask_target_grid, SIGNAL(clicked()), this, SLOT(set_iqq_ask_target_grid()));

   cb_iq_scale_angstrom = new QCheckBox(this);
   cb_iq_scale_angstrom->setText(tr("I(q) curves in angstrom"));
   cb_iq_scale_angstrom->setEnabled(true);
   cb_iq_scale_angstrom->setChecked((*saxs_options).iq_scale_angstrom);
   cb_iq_scale_angstrom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iq_scale_angstrom->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iq_scale_angstrom, SIGNAL(clicked()), this, SLOT(set_iq_scale_angstrom()));

   cb_iq_scale_nm = new QCheckBox(this);
   cb_iq_scale_nm->setText(tr("I(q) curves in nanometer"));
   cb_iq_scale_nm->setEnabled(true);
   cb_iq_scale_nm->setChecked((*saxs_options).iq_scale_nm);
   cb_iq_scale_nm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iq_scale_nm->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iq_scale_nm, SIGNAL(clicked()), this, SLOT(set_iq_scale_nm()));

   cb_iqq_expt_data_contains_variances = new QCheckBox(this);
   cb_iqq_expt_data_contains_variances->setText(tr("Expt. data in variance"));
   cb_iqq_expt_data_contains_variances->setEnabled(true);
   cb_iqq_expt_data_contains_variances->setChecked((*saxs_options).iqq_expt_data_contains_variances);
   cb_iqq_expt_data_contains_variances->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_expt_data_contains_variances->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq_expt_data_contains_variances, SIGNAL(clicked()), this, SLOT(set_iqq_expt_data_contains_variances()));

   cb_disable_iq_scaling = new QCheckBox(this);
   cb_disable_iq_scaling->setText(tr("Disable I(q) scaling"));
   cb_disable_iq_scaling->setEnabled(true);
   cb_disable_iq_scaling->setChecked((*saxs_options).disable_iq_scaling);
   cb_disable_iq_scaling->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_disable_iq_scaling->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_disable_iq_scaling, SIGNAL(clicked()), this, SLOT(set_disable_iq_scaling()));

   cb_iqq_scale_linear_offset = new QCheckBox(this);
   cb_iqq_scale_linear_offset->setText(tr("Scale with linear offset"));
   cb_iqq_scale_linear_offset->setEnabled(true);
   cb_iqq_scale_linear_offset->setChecked((*saxs_options).iqq_scale_linear_offset);
   cb_iqq_scale_linear_offset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_scale_linear_offset->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq_scale_linear_offset, SIGNAL(clicked()), this, SLOT(set_iqq_scale_linear_offset()));

   cb_iqq_scale_chi2_fitting = new QCheckBox(this);
   cb_iqq_scale_chi2_fitting->setText(tr("Chi^2 fitting"));
   cb_iqq_scale_chi2_fitting->setEnabled(true);
   cb_iqq_scale_chi2_fitting->setChecked((*saxs_options).iqq_scale_chi2_fitting);
   cb_iqq_scale_chi2_fitting->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_scale_chi2_fitting->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq_scale_chi2_fitting, SIGNAL(clicked()), this, SLOT(set_iqq_scale_chi2_fitting()));

   cb_autocorrelate = new QCheckBox(this);
   cb_autocorrelate->setText(tr("Autocorrelate"));
   cb_autocorrelate->setEnabled(true);
   cb_autocorrelate->setChecked((*saxs_options).autocorrelate);
   cb_autocorrelate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_autocorrelate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_autocorrelate, SIGNAL(clicked()), this, SLOT(set_autocorrelate()));

   cb_hybrid_radius_excl_vol = new QCheckBox(this);
   cb_hybrid_radius_excl_vol->setText(tr("Use hybrid radius for excluded volume"));
   cb_hybrid_radius_excl_vol->setEnabled(true);
   cb_hybrid_radius_excl_vol->setChecked((*saxs_options).hybrid_radius_excl_vol);
   cb_hybrid_radius_excl_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hybrid_radius_excl_vol->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_hybrid_radius_excl_vol, SIGNAL(clicked()), this, SLOT(set_hybrid_radius_excl_vol()));

   lbl_swh_excl_vol = new QLabel(tr(" Excluded volume SWH [A^3]: "), this);
   lbl_swh_excl_vol->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_swh_excl_vol->setMinimumHeight(minHeight1);
   lbl_swh_excl_vol->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_swh_excl_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_swh_excl_vol = new QLineEdit(this, "swh_excl_vol Line Edit");
   (*saxs_options).swh_excl_vol ? 
      le_swh_excl_vol->setText(QString("%1").arg((*saxs_options).swh_excl_vol)) :
      le_swh_excl_vol->setText("");
   // le_swh_excl_vol->setMinimumHeight(minHeight1);
   le_swh_excl_vol->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_swh_excl_vol->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_swh_excl_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_swh_excl_vol, SIGNAL(textChanged(const QString &)), SLOT(update_swh_excl_vol(const QString &)));

   lbl_scale_excl_vol = new QLabel(tr(" Excluded volume scaling: "), this);
   lbl_scale_excl_vol->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_scale_excl_vol->setMinimumHeight(minHeight1);
   lbl_scale_excl_vol->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_scale_excl_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_scale_excl_vol = new QwtCounter(this);
   cnt_scale_excl_vol->setRange(.5, 1.5, 0.001);
   cnt_scale_excl_vol->setValue((*saxs_options).scale_excl_vol);
   cnt_scale_excl_vol->setMinimumHeight(minHeight1);
   cnt_scale_excl_vol->setEnabled(true);
   cnt_scale_excl_vol->setNumButtons(3);
   cnt_scale_excl_vol->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_scale_excl_vol->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_scale_excl_vol, SIGNAL(valueChanged(double)), SLOT(update_scale_excl_vol(double)));

   cb_subtract_radius = new QCheckBox(this);
   cb_subtract_radius->setText(tr("Subtract radii for debye pairwise distance "));
   cb_subtract_radius->setEnabled(true);
   cb_subtract_radius->setChecked((*saxs_options).subtract_radius);
   cb_subtract_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_subtract_radius->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_subtract_radius, SIGNAL(clicked()), this, SLOT(set_subtract_radius()));

   lbl_iqq_scale_min_maxq = new QLabel(tr(" I(q) curve q range for scaling, NNLS and best fit (Angstrom) "), this);
   lbl_iqq_scale_min_maxq->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_iqq_scale_min_maxq->setMinimumHeight(minHeight1);
   lbl_iqq_scale_min_maxq->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_iqq_scale_min_maxq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_iqq_scale_minq = new QLineEdit(this, "iqq_scale_minq Line Edit");
   (*saxs_options).iqq_scale_minq ? 
      le_iqq_scale_minq->setText(QString("%1").arg((*saxs_options).iqq_scale_minq)) :
      le_iqq_scale_minq->setText("");
   // le_iqq_scale_minq->setMinimumHeight(minHeight1);
   le_iqq_scale_minq->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_iqq_scale_minq->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_iqq_scale_minq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_iqq_scale_minq, SIGNAL(textChanged(const QString &)), SLOT(update_iqq_scale_minq(const QString &)));

   le_iqq_scale_maxq = new QLineEdit(this, "iqq_scale_maxq Line Edit");
   (*saxs_options).iqq_scale_maxq ? 
      le_iqq_scale_maxq->setText(QString("%1").arg((*saxs_options).iqq_scale_maxq)) :
      le_iqq_scale_maxq->setText("");
   // le_iqq_scale_maxq->setMinimumHeight(minHeight1);
   le_iqq_scale_maxq->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_iqq_scale_maxq->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_iqq_scale_maxq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_iqq_scale_maxq, SIGNAL(textChanged(const QString &)), SLOT(update_iqq_scale_maxq(const QString &)));

   cb_iqq_scale_nnls = new QCheckBox(this);
   cb_iqq_scale_nnls->setText(tr(" Use alternate scaling function"));
   cb_iqq_scale_nnls->setEnabled(true);
   cb_iqq_scale_nnls->setChecked((*saxs_options).iqq_scale_nnls);
   cb_iqq_scale_nnls->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_scale_nnls->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq_scale_nnls, SIGNAL(clicked()), this, SLOT(set_iqq_scale_nnls()));

   cb_iqq_log_fitting = new QCheckBox(this);
   cb_iqq_log_fitting->setText(tr(" I(q) NNLS log fit"));
   cb_iqq_log_fitting->setEnabled(true);
   cb_iqq_log_fitting->setChecked((*saxs_options).iqq_log_fitting);
   cb_iqq_log_fitting->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_log_fitting->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq_log_fitting, SIGNAL(clicked()), this, SLOT(set_iqq_log_fitting()));

   cb_iqq_scaled_fitting = new QCheckBox(this);
   cb_iqq_scaled_fitting->setText(tr(" I(q) NNLS scaled fit"));
   cb_iqq_scaled_fitting->setEnabled(true);
   cb_iqq_scaled_fitting->setChecked((*saxs_options).iqq_scaled_fitting);
   cb_iqq_scaled_fitting->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_scaled_fitting->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq_scaled_fitting, SIGNAL(clicked()), this, SLOT(set_iqq_scaled_fitting()));

   cb_iqq_scale_play = new QCheckBox(this);
   cb_iqq_scale_play->setText(tr(" Manually adjust scaling"));
   cb_iqq_scale_play->setEnabled(true);
   cb_iqq_scale_play->setChecked((*saxs_options).iqq_scale_play);
   cb_iqq_scale_play->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_scale_play->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq_scale_play, SIGNAL(clicked()), this, SLOT(set_iqq_scale_play()));

   pb_clear_mw_cache = new QPushButton(tr("Clear remembered molecular weights"), this);
   pb_clear_mw_cache->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_clear_mw_cache->setMinimumHeight(minHeight1);
   pb_clear_mw_cache->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_clear_mw_cache, SIGNAL(clicked()), SLOT(clear_mw_cache()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int rows=9, columns = 4, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 3);
   j++;

   background->addMultiCellWidget(lbl_saxs_options, j, j, 0, 1);
   j++;
   background->addWidget(lbl_water_e_density, j, 0);
   background->addWidget(cnt_water_e_density, j, 1);
   j++;

   QHBoxLayout *hbl_saxs_iq = new QHBoxLayout;
   hbl_saxs_iq->addWidget(lbl_saxs_iq);
   hbl_saxs_iq->addWidget(cb_saxs_iq_native_debye);
   hbl_saxs_iq->addWidget(cb_saxs_iq_native_hybrid);
   hbl_saxs_iq->addWidget(cb_saxs_iq_native_hybrid2);
   hbl_saxs_iq->addWidget(cb_saxs_iq_native_hybrid3);
   hbl_saxs_iq->addWidget(cb_saxs_iq_hybrid_adaptive);
   hbl_saxs_iq->addWidget(cb_saxs_iq_native_fast);
   hbl_saxs_iq->addWidget(cb_saxs_iq_native_fast_compute_pr);
   hbl_saxs_iq->addWidget(cb_saxs_iq_foxs);
   hbl_saxs_iq->addWidget(cb_saxs_iq_crysol);
   background->addMultiCellLayout(hbl_saxs_iq, j, j, 0, 1);
   j++;

   background->addWidget(lbl_fast_bin_size, j, 0);
   background->addWidget(cnt_fast_bin_size, j, 1);
   j++;

   background->addWidget(lbl_fast_modulation, j, 0);
   background->addWidget(cnt_fast_modulation, j, 1);
   j++;

   background->addWidget(lbl_hybrid2_q_points, j, 0);
   background->addWidget(cnt_hybrid2_q_points, j, 1);
   j++;

   background->addWidget(lbl_crysol_max_harmonics, j, 0);
   background->addWidget(cnt_crysol_max_harmonics, j, 1);
   j++;

   background->addWidget(lbl_crysol_fibonacci_grid_order, j, 0);
   background->addWidget(cnt_crysol_fibonacci_grid_order, j, 1);
   j++;

   background->addWidget(lbl_crysol_hydration_shell_contrast, j, 0);
   background->addWidget(cnt_crysol_hydration_shell_contrast, j, 1);
   j++;

   QHBoxLayout *hbl_crysol = new QHBoxLayout;
   hbl_crysol->addWidget(cb_crysol_default_load_difference_intensity);
   hbl_crysol->addWidget(cb_crysol_version_26);
   background->addMultiCellLayout(hbl_crysol, j, j, 0, 1);
   j++;

   background->addMultiCellWidget(lbl_sans_options, j, j, 0, 1);
   j++;
   background->addWidget(lbl_h_scat_len, j, 0);
   background->addWidget(cnt_h_scat_len, j, 1);
   j++;
   background->addWidget(lbl_d_scat_len, j, 0);
   background->addWidget(cnt_d_scat_len, j, 1);
   j++;
   background->addWidget(lbl_h2o_scat_len_dens, j, 0);
   background->addWidget(cnt_h2o_scat_len_dens, j, 1);
   j++;
   background->addWidget(lbl_d2o_scat_len_dens, j, 0);
   background->addWidget(cnt_d2o_scat_len_dens, j, 1);
   j++;
   background->addWidget(lbl_d2o_conc, j, 0);
   background->addWidget(cnt_d2o_conc, j, 1);
   j++;
   background->addWidget(lbl_frac_of_exch_pep, j, 0);
   background->addWidget(cnt_frac_of_exch_pep, j, 1);
   j++;

   QHBoxLayout *hbl_sans_iq = new QHBoxLayout;
   hbl_sans_iq->addWidget(lbl_sans_iq);
   hbl_sans_iq->addWidget(cb_sans_iq_native_debye);
   hbl_sans_iq->addWidget(cb_sans_iq_native_hybrid);
   hbl_sans_iq->addWidget(cb_sans_iq_native_hybrid2);
   hbl_sans_iq->addWidget(cb_sans_iq_native_hybrid3);
   hbl_sans_iq->addWidget(cb_sans_iq_hybrid_adaptive);
   hbl_sans_iq->addWidget(cb_sans_iq_native_fast);
   hbl_sans_iq->addWidget(cb_sans_iq_native_fast_compute_pr);
   hbl_sans_iq->addWidget(cb_sans_iq_cryson);
   background->addMultiCellLayout(hbl_sans_iq, j, j, 0, 1);
   j++;

   background->addMultiCellWidget(lbl_guinier, j, j, 0, 1);
   j++;
   background->addWidget(cb_guinier_csv, j, 0);
   background->addWidget(le_guinier_csv_filename, j, 1);
   j++;
   background->addWidget(lbl_qRgmax, j, 0);
   background->addWidget(cnt_qRgmax, j, 1);
   j++;
   background->addWidget(lbl_qend, j, 0);
   background->addWidget(cnt_qend, j, 1);
   j++;
   background->addWidget(lbl_pointsmin, j, 0);
   background->addWidget(cnt_pointsmin, j, 1);
   j++;
   background->addWidget(lbl_pointsmax, j, 0);
   background->addWidget(cnt_pointsmax, j, 1);
   j++;

   // column grouping 2

   int k = 1;
   
   background->addMultiCellWidget(lbl_curve, k, k, 2, 3);
   k++;
   background->addWidget(lbl_wavelength, k, 2);
   background->addWidget(cnt_wavelength, k, 3);
   k++;
   background->addWidget(lbl_start_angle, k, 2);
   background->addWidget(cnt_start_angle, k, 3);
   k++;
   background->addWidget(lbl_end_angle, k, 2);
   background->addWidget(cnt_end_angle, k, 3);
   k++;
   background->addWidget(lbl_delta_angle, k, 2);
   background->addWidget(cnt_delta_angle, k, 3);
   k++;
   background->addWidget(lbl_start_q, k, 2);
   background->addWidget(cnt_start_q, k, 3);
   k++;
   background->addWidget(lbl_end_q, k, 2);
   background->addWidget(cnt_end_q, k, 3);
   k++;
   background->addWidget(lbl_delta_q, k, 2);
   background->addWidget(cnt_delta_q, k, 3);
   k++;

   QHBoxLayout *hbl_curve_opts = new QHBoxLayout;
   hbl_curve_opts->addWidget(cb_hydrate_pdb);
   hbl_curve_opts->addWidget(cb_normalize_by_mw);
   background->addMultiCellLayout(hbl_curve_opts, k, k, 2, 3);
   k++;

   background->addMultiCellWidget(lbl_bead_model_control, k, k, 2, 3);
   k++;

   QHBoxLayout *hbl_coeff_bead_model = new QHBoxLayout;
   hbl_coeff_bead_model->addWidget(cb_compute_saxs_coeff_for_bead_models);
   hbl_coeff_bead_model->addWidget(cb_compute_sans_coeff_for_bead_models);
   hbl_coeff_bead_model->addWidget(cb_bead_model_rayleigh);
   background->addMultiCellLayout(hbl_coeff_bead_model, k, k, 2, 3);
   k++;

   background->addWidget(pb_default_atom_filename, k, 2);
   background->addWidget(le_default_atom_filename, k, 3);
   k++;
   background->addWidget(pb_default_hybrid_filename, k, 2);
   background->addWidget(le_default_hybrid_filename, k, 3);
   k++;
   background->addWidget(pb_default_saxs_filename, k, 2);
   background->addWidget(le_default_saxs_filename, k, 3);
   k++;
   background->addWidget(pb_default_rotamer_filename, k, 2);
   background->addWidget(le_default_rotamer_filename, k, 3);
   k++;

   background->addMultiCellWidget(lbl_misc, k, k, 2, 3);
   k++;
   background->addWidget(lbl_steric_clash_distance, k, 2);
   background->addWidget(cnt_steric_clash_distance, k, 3);
   k++;
   
   QHBoxLayout *hbl_iq_ask = new QHBoxLayout;
   hbl_iq_ask->addWidget(cb_iq_ask);
   hbl_iq_ask->addWidget(cb_iq_scale_ask);
   hbl_iq_ask->addWidget(cb_iqq_ask_target_grid);
   
   background->addMultiCellLayout(hbl_iq_ask, k, k, 2, 3);
   k++;

   QHBoxLayout *hbl_iq_scale = new QHBoxLayout;
   hbl_iq_scale->addWidget(cb_iq_scale_angstrom);
   hbl_iq_scale->addWidget(cb_iq_scale_nm);
   hbl_iq_scale->addWidget(cb_iqq_expt_data_contains_variances);
   background->addMultiCellLayout(hbl_iq_scale, k, k, 2, 3);
   k++;

   QHBoxLayout *hbl_various_1 = new QHBoxLayout;
   hbl_various_1->addWidget(cb_disable_iq_scaling);
   hbl_various_1->addWidget(cb_iqq_scale_linear_offset);
   hbl_various_1->addWidget(cb_iqq_scale_chi2_fitting);
   background->addMultiCellLayout(hbl_various_1, k, k, 2, 3);
   k++;

   QHBoxLayout *hbl_various_2 = new QHBoxLayout;
   hbl_various_2->addWidget(cb_autocorrelate);
   hbl_various_2->addWidget(cb_hybrid_radius_excl_vol);
   hbl_various_2->addWidget(cb_subtract_radius);
   background->addMultiCellLayout(hbl_various_2, k, k, 2, 3);
   k++;

   QHBoxLayout *hbl_various_2b = new QHBoxLayout;
   hbl_various_2b->addWidget(lbl_swh_excl_vol);
   hbl_various_2b->addWidget(le_swh_excl_vol);
   hbl_various_2b->addWidget(lbl_scale_excl_vol);
   hbl_various_2b->addWidget(cnt_scale_excl_vol);
   background->addMultiCellLayout(hbl_various_2b, k, k, 2, 3);
   k++;

   background->addMultiCellWidget(pb_clear_mw_cache, k, k, 2, 3);
   k++;

   QHBoxLayout *hbl_iqq_scaling = new QHBoxLayout;
   hbl_iqq_scaling->addWidget(lbl_iqq_scale_min_maxq);
   hbl_iqq_scaling->addWidget(le_iqq_scale_minq);
   hbl_iqq_scaling->addWidget(le_iqq_scale_maxq);
   background->addMultiCellLayout(hbl_iqq_scaling, k, k, 2, 3);
   k++;

   QHBoxLayout *hbl_various_3 = new QHBoxLayout;
   
   hbl_various_3->addWidget(cb_iqq_scale_nnls);
   hbl_various_3->addWidget(cb_iqq_log_fitting);
   hbl_various_3->addWidget(cb_iqq_scaled_fitting);
   hbl_various_3->addWidget(cb_iqq_scale_play);
   background->addMultiCellLayout(hbl_various_3, k, k, 2, 3);
   k++;

   if ( k > j )
   {
      j = k;
   }

   background->addMultiCellWidget(pb_help, j, j, 0, 1);
   background->addMultiCellWidget(pb_cancel, j, j, 2, 3);

   setMinimumWidth(750);
}

void US_Hydrodyn_SaxsOptions::cancel()
{
   close();
}

void US_Hydrodyn_SaxsOptions::clear_mw_cache()
{
   if ( !((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw.size() &&
        !((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw_source.size() &&
        !((US_Hydrodyn *)us_hydrodyn)->dammix_match_remember_mw.size() )
   {
      QMessageBox::information( this,
                                "UltraScan",
                                tr("The molecular weight cache is already empty") );
   }      
          
   ((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw.clear();
   ((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw_source.clear();
   ((US_Hydrodyn *)us_hydrodyn)->dammix_match_remember_mw.clear();
}

void US_Hydrodyn_SaxsOptions::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_options.html");
}

void US_Hydrodyn_SaxsOptions::closeEvent(QCloseEvent *e)
{
   *saxs_options_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_SaxsOptions::update_water_e_density(double val)
{
   (*saxs_options).water_e_density = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_saxs_iq_native_debye()
{
   (*saxs_options).saxs_iq_native_debye = cb_saxs_iq_native_debye->isChecked();
   (*saxs_options).saxs_iq_native_fast = !cb_saxs_iq_native_debye->isChecked();
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_foxs = false;
   (*saxs_options).saxs_iq_crysol = false;

   cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
   cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
   cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}


void US_Hydrodyn_SaxsOptions::set_saxs_iq_native_hybrid()
{
   (*saxs_options).saxs_iq_native_hybrid = cb_saxs_iq_native_hybrid->isChecked();
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_native_hybrid->isChecked();
   (*saxs_options).saxs_iq_native_fast = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_foxs = false;
   (*saxs_options).saxs_iq_crysol = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
   cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::set_saxs_iq_native_hybrid2()
{
   (*saxs_options).saxs_iq_native_hybrid2 = cb_saxs_iq_native_hybrid2->isChecked();
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_native_hybrid2->isChecked();
   (*saxs_options).saxs_iq_native_fast = false;
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_foxs = false;
   (*saxs_options).saxs_iq_crysol = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
   cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::set_saxs_iq_native_hybrid3()
{
   (*saxs_options).saxs_iq_native_hybrid3 = cb_saxs_iq_native_hybrid3->isChecked();
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_native_hybrid3->isChecked();
   (*saxs_options).saxs_iq_native_fast = false;
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_foxs = false;
   (*saxs_options).saxs_iq_crysol = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
   cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::set_saxs_iq_hybrid_adaptive()
{
   (*saxs_options).saxs_iq_hybrid_adaptive = cb_saxs_iq_hybrid_adaptive->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::set_saxs_iq_native_fast()
{
   (*saxs_options).saxs_iq_native_fast = cb_saxs_iq_native_fast->isChecked();
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_native_fast->isChecked();
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_foxs = false;
   (*saxs_options).saxs_iq_crysol = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
   cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
   cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);
   cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::set_saxs_iq_native_fast_compute_pr()
{
   (*saxs_options).saxs_iq_native_fast_compute_pr = cb_saxs_iq_native_fast_compute_pr->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_saxs_iq_crysol()
{
   (*saxs_options).saxs_iq_crysol = cb_saxs_iq_crysol->isChecked();
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_crysol->isChecked();
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_native_fast = false;
   (*saxs_options).saxs_iq_foxs = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
   cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
   cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_foxs->setChecked((*saxs_options).saxs_iq_foxs);

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::set_saxs_iq_foxs()
{
   (*saxs_options).saxs_iq_foxs = cb_saxs_iq_foxs->isChecked();
   (*saxs_options).saxs_iq_native_debye = !cb_saxs_iq_foxs->isChecked();
   (*saxs_options).saxs_iq_native_hybrid = false;
   (*saxs_options).saxs_iq_native_hybrid2 = false;
   (*saxs_options).saxs_iq_native_hybrid3 = false;
   (*saxs_options).saxs_iq_native_fast = false;
   (*saxs_options).saxs_iq_crysol = false;

   cb_saxs_iq_native_debye->setChecked((*saxs_options).saxs_iq_native_debye);
   cb_saxs_iq_native_hybrid->setChecked((*saxs_options).saxs_iq_native_hybrid);
   cb_saxs_iq_native_hybrid2->setChecked((*saxs_options).saxs_iq_native_hybrid2);
   cb_saxs_iq_native_hybrid3->setChecked((*saxs_options).saxs_iq_native_hybrid3);
   cb_saxs_iq_native_fast->setChecked((*saxs_options).saxs_iq_native_fast);
   cb_saxs_iq_crysol->setChecked((*saxs_options).saxs_iq_crysol);

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::update_crysol_max_harmonics(double val)
{
   (*saxs_options).crysol_max_harmonics = (unsigned int) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->update_iqq_suffix();
   }
}

void US_Hydrodyn_SaxsOptions::update_crysol_fibonacci_grid_order(double val)
{
   (*saxs_options).crysol_fibonacci_grid_order = (unsigned int) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->update_iqq_suffix();
   }
}

void US_Hydrodyn_SaxsOptions::update_crysol_hydration_shell_contrast(double val)
{
   (*saxs_options).crysol_hydration_shell_contrast = (float) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->update_iqq_suffix();
   }
}

void US_Hydrodyn_SaxsOptions::set_crysol_default_load_difference_intensity()
{
   (*saxs_options).crysol_default_load_difference_intensity = cb_crysol_default_load_difference_intensity->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_crysol_version_26()
{
   (*saxs_options).crysol_version_26 = cb_crysol_version_26->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_h_scat_len(double val)
{
   (*saxs_options).h_scat_len = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_d_scat_len(double val)
{
   (*saxs_options).d_scat_len = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_h2o_scat_len_dens(double val)
{
   (*saxs_options).h2o_scat_len_dens = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_d2o_scat_len_dens(double val)
{
   (*saxs_options).d2o_scat_len_dens = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_d2o_conc(double val)
{
   (*saxs_options).d2o_conc = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_frac_of_exch_pep(double val)
{
   (*saxs_options).frac_of_exch_pep = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_sans_iq_native_debye()
{
   (*saxs_options).sans_iq_native_debye = cb_sans_iq_native_debye->isChecked();
   (*saxs_options).sans_iq_native_fast = !cb_sans_iq_native_debye->isChecked();
   (*saxs_options).sans_iq_native_hybrid = false;
   (*saxs_options).sans_iq_native_hybrid2 = false;
   (*saxs_options).sans_iq_native_hybrid3 = false;
   (*saxs_options).sans_iq_cryson = false;

   cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
   cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
   cb_sans_iq_native_hybrid2->setChecked((*saxs_options).sans_iq_native_hybrid2);
   cb_sans_iq_native_hybrid3->setChecked((*saxs_options).sans_iq_native_hybrid3);
   cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
   cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);

   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::set_sans_iq_native_fast_compute_pr()
{
   (*saxs_options).sans_iq_native_fast_compute_pr = cb_sans_iq_native_fast_compute_pr->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_sans_iq_native_hybrid()
{
   (*saxs_options).sans_iq_native_hybrid = cb_sans_iq_native_hybrid->isChecked();
   (*saxs_options).sans_iq_native_debye = !cb_sans_iq_native_hybrid->isChecked();
   (*saxs_options).sans_iq_native_hybrid2 = false;
   (*saxs_options).sans_iq_native_hybrid3 = false;
   (*saxs_options).sans_iq_native_fast = false;
   (*saxs_options).sans_iq_cryson = false;

   cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
   cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
   cb_sans_iq_native_hybrid2->setChecked((*saxs_options).sans_iq_native_hybrid2);
   cb_sans_iq_native_hybrid3->setChecked((*saxs_options).sans_iq_native_hybrid3);
   cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
   cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);

   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::set_sans_iq_native_hybrid2()
{
   (*saxs_options).sans_iq_native_hybrid2 = cb_sans_iq_native_hybrid2->isChecked();
   (*saxs_options).sans_iq_native_debye = !cb_sans_iq_native_hybrid2->isChecked();
   (*saxs_options).sans_iq_native_hybrid = false;
   (*saxs_options).sans_iq_native_hybrid3 = false;
   (*saxs_options).sans_iq_native_fast = false;
   (*saxs_options).sans_iq_cryson = false;

   cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
   cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
   cb_sans_iq_native_hybrid2->setChecked((*saxs_options).sans_iq_native_hybrid2);
   cb_sans_iq_native_hybrid3->setChecked((*saxs_options).sans_iq_native_hybrid3);
   cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
   cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);

   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::set_sans_iq_native_hybrid3()
{
   (*saxs_options).sans_iq_native_hybrid3 = cb_sans_iq_native_hybrid3->isChecked();
   (*saxs_options).sans_iq_native_debye = !cb_sans_iq_native_hybrid3->isChecked();
   (*saxs_options).sans_iq_native_hybrid = false;
   (*saxs_options).sans_iq_native_hybrid2 = false;
   (*saxs_options).sans_iq_native_fast = false;
   (*saxs_options).sans_iq_cryson = false;

   cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
   cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
   cb_sans_iq_native_hybrid2->setChecked((*saxs_options).sans_iq_native_hybrid2);
   cb_sans_iq_native_hybrid3->setChecked((*saxs_options).sans_iq_native_hybrid3);
   cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
   cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);

   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::set_sans_iq_hybrid_adaptive()
{
   (*saxs_options).sans_iq_hybrid_adaptive = cb_sans_iq_hybrid_adaptive->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::set_sans_iq_native_fast()
{
   (*saxs_options).sans_iq_native_fast = cb_sans_iq_native_fast->isChecked();
   (*saxs_options).sans_iq_native_debye = !cb_sans_iq_native_fast->isChecked();
   (*saxs_options).sans_iq_native_hybrid = false;
   (*saxs_options).sans_iq_native_hybrid2 = false;
   (*saxs_options).sans_iq_native_hybrid3 = false;
   (*saxs_options).sans_iq_cryson = false;

   cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
   cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
   cb_sans_iq_native_hybrid2->setChecked((*saxs_options).sans_iq_native_hybrid2);
   cb_sans_iq_native_hybrid3->setChecked((*saxs_options).sans_iq_native_hybrid3);
   cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
   cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);

   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::set_sans_iq_cryson()
{
   (*saxs_options).sans_iq_cryson = cb_sans_iq_cryson->isChecked();
   (*saxs_options).sans_iq_native_debye = !cb_sans_iq_cryson->isChecked();
   (*saxs_options).sans_iq_native_hybrid = false;
   (*saxs_options).sans_iq_native_hybrid2 = false;
   (*saxs_options).sans_iq_native_hybrid3 = false;
   (*saxs_options).sans_iq_native_fast = false;

   cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
   cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
   cb_sans_iq_native_hybrid2->setChecked((*saxs_options).sans_iq_native_hybrid2);
   cb_sans_iq_native_hybrid3->setChecked((*saxs_options).sans_iq_native_hybrid3);
   cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
   cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);

   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SaxsOptions::update_wavelength(double val)
{
   (*saxs_options).wavelength = (float) val;
   update_q();
   cnt_start_q->setValue(saxs_options->start_q);
   cnt_end_q->setValue(saxs_options->end_q);
   cnt_delta_q->setValue(saxs_options->delta_q);
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_start_angle(double val)
{
   (*saxs_options).start_angle = (float) val;
   if ( saxs_options->start_angle > 
        saxs_options->end_angle )
   {
      saxs_options->start_angle = saxs_options->end_angle;
      cnt_start_angle->setValue(saxs_options->end_angle);
   }
   update_q();
   cnt_start_q->setValue(saxs_options->start_q);
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_end_angle(double val)
{
   (*saxs_options).end_angle = (float) val;
   if ( saxs_options->start_angle > 
        saxs_options->end_angle )
   {
      saxs_options->end_angle = saxs_options->start_angle;
      cnt_end_angle->setValue(saxs_options->start_angle);
   }
   update_q();
   cnt_end_q->setValue(saxs_options->end_q);
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_delta_angle(double val)
{
   (*saxs_options).delta_angle = (float) val;
   update_q();
   cnt_delta_q->setValue(saxs_options->delta_q);
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_q()
{
   // note changes to this section should be updated in US_Hydrodyn_Saxs::US_Hydrodyn_Saxs()
   if ( saxs_options->wavelength == 0 )
   {
      saxs_options->start_q = 
         saxs_options->end_q = 
         saxs_options->delta_q = 0;
   }
   else
   {
      saxs_options->start_q = 4.0 * M_PI * 
         sin(saxs_options->start_angle * M_PI / 360.0) / 
         saxs_options->wavelength;
      saxs_options->start_q =  floor(saxs_options->start_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
      saxs_options->end_q = 4.0 * M_PI * 
         sin(saxs_options->end_angle * M_PI / 360.0) / 
         saxs_options->wavelength;
      saxs_options->end_q =  floor(saxs_options->end_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
      saxs_options->delta_q = 4.0 * M_PI * 
         sin(saxs_options->delta_angle * M_PI / 360.0) / 
         saxs_options->wavelength;
      saxs_options->delta_q =  floor(saxs_options->delta_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
   }         
}      

void US_Hydrodyn_SaxsOptions::update_start_q(double val)
{
   (*saxs_options).start_q = (float) val;
   if ( saxs_options->start_q > 
        saxs_options->end_q )
   {
      saxs_options->start_q = saxs_options->end_q;
      cnt_start_q->setValue(saxs_options->end_q);
   }

   // note 360 / M_PI since 'theta' is 1/2 scattering angle

   saxs_options->start_angle = 
      floor(
            (asin(saxs_options->wavelength * saxs_options->start_q / 
                  (4.0 * M_PI)) * 360.0 / M_PI) * SAXS_Q_ROUNDING + 0.5
            ) / SAXS_Q_ROUNDING;
   cnt_start_angle->setValue(saxs_options->start_angle);

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_end_q(double val)
{
   (*saxs_options).end_q = (float) val;
   if ( saxs_options->start_q > 
        saxs_options->end_q )
   {
      saxs_options->end_q = saxs_options->start_q;
      cnt_end_q->setValue(saxs_options->start_q);
   }

   saxs_options->end_angle = 
      floor(
            (asin(saxs_options->wavelength * saxs_options->end_q / 
                  (4.0 * M_PI)) * 360.0 / M_PI) * SAXS_Q_ROUNDING + 0.5
            ) / SAXS_Q_ROUNDING;
   cnt_end_angle->setValue(saxs_options->end_angle);

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_delta_q(double val)
{
   (*saxs_options).delta_q = (float) val;

   saxs_options->delta_angle = 
      floor(
            (asin(saxs_options->wavelength * saxs_options->delta_q / 
                  (4.0 * M_PI)) * 360.0 / M_PI) * SAXS_Q_ROUNDING + 0.5
            ) / SAXS_Q_ROUNDING;
   cnt_delta_angle->setValue(saxs_options->delta_angle);

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_hydrate_pdb()
{
   (*saxs_options).hydrate_pdb = cb_hydrate_pdb->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_normalize_by_mw()
{
   (*saxs_options).normalize_by_mw = cb_normalize_by_mw->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_iq_ask()
{
   (*saxs_options).iq_ask = cb_iq_ask->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_iq_scale_ask()
{
   (*saxs_options).iq_scale_ask = cb_iq_scale_ask->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_iqq_ask_target_grid()
{
   (*saxs_options).iqq_ask_target_grid = cb_iqq_ask_target_grid->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_iq_scale_angstrom()
{
   (*saxs_options).iq_scale_angstrom = cb_iq_scale_angstrom->isChecked();
   (*saxs_options).iq_scale_nm = !cb_iq_scale_angstrom->isChecked();
   cb_iq_scale_nm->setChecked((*saxs_options).iq_scale_nm);
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_iq_scale_nm()
{
   (*saxs_options).iq_scale_nm = cb_iq_scale_nm->isChecked();
   (*saxs_options).iq_scale_angstrom = !cb_iq_scale_nm->isChecked();
   cb_iq_scale_angstrom->setChecked((*saxs_options).iq_scale_angstrom);
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_iqq_expt_data_contains_variances()
{
   (*saxs_options).iqq_expt_data_contains_variances = cb_iqq_expt_data_contains_variances->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_disable_iq_scaling()
{
   (*saxs_options).disable_iq_scaling = cb_disable_iq_scaling->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_iqq_scale_linear_offset()
{
   (*saxs_options).iqq_scale_linear_offset = cb_iqq_scale_linear_offset->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_iqq_scale_chi2_fitting()
{
   (*saxs_options).iqq_scale_chi2_fitting = cb_iqq_scale_chi2_fitting->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_SaxsOptions::set_autocorrelate()
{
   (*saxs_options).autocorrelate = cb_autocorrelate->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_hybrid_radius_excl_vol()
{
   (*saxs_options).hybrid_radius_excl_vol = cb_hybrid_radius_excl_vol->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_scale_excl_vol(double val)
{
   (*saxs_options).scale_excl_vol = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->update_iqq_suffix();
   }
}

void US_Hydrodyn_SaxsOptions::set_subtract_radius()
{
   (*saxs_options).subtract_radius = cb_subtract_radius->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_swh_excl_vol( const QString &str )
{
   (*saxs_options).swh_excl_vol = str.toFloat();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->update_iqq_suffix();
   }
}

void US_Hydrodyn_SaxsOptions::update_iqq_scale_minq( const QString &str )
{
   (*saxs_options).iqq_scale_minq = str.toFloat();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_iqq_scale_maxq( const QString &str )
{
   (*saxs_options).iqq_scale_maxq = str.toFloat();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_iqq_scale_nnls()
{
   (*saxs_options).iqq_scale_nnls = cb_iqq_scale_nnls->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_iqq_log_fitting()
{
   (*saxs_options).iqq_log_fitting = cb_iqq_log_fitting->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_iqq_scaled_fitting()
{
   (*saxs_options).iqq_scaled_fitting = cb_iqq_scaled_fitting->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_iqq_scale_play()
{
   (*saxs_options).iqq_scale_play = cb_iqq_scale_play->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_guinier_csv()
{
   (*saxs_options).guinier_csv = cb_guinier_csv->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_guinier_csv_filename(const QString &str)
{
   (*saxs_options).guinier_csv_filename = str;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_qRgmax(double val)
{
   (*saxs_options).qRgmax = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_qend(double val)
{
   (*saxs_options).qend = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
void US_Hydrodyn_SaxsOptions::update_pointsmin(double val)
{
   (*saxs_options).pointsmin = (unsigned int) val;
   if ( (*saxs_options).pointsmax < (unsigned int) val )
   {
      (*saxs_options).pointsmax = (unsigned int) val;
      cnt_pointsmax->setValue(val);
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
void US_Hydrodyn_SaxsOptions::update_pointsmax(double val)
{
   (*saxs_options).pointsmax = (unsigned int) val;
   if ( (*saxs_options).pointsmin > (unsigned int) val )
   {
      (*saxs_options).pointsmin = (unsigned int) val;
      cnt_pointsmin->setValue(val);
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_fast_bin_size(double val)
{
   (*saxs_options).fast_bin_size = (float) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_fast_modulation(double val)
{
   (*saxs_options).fast_modulation = (float) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_hybrid2_q_points(double val)
{
   (*saxs_options).hybrid2_q_points = (unsigned int) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_steric_clash_distance(double val)
{
   (*saxs_options).steric_clash_distance = val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_compute_saxs_coeff_for_bead_models()
{
   (*saxs_options).compute_saxs_coeff_for_bead_models = cb_compute_saxs_coeff_for_bead_models->isChecked();
   if ( (*saxs_options).compute_saxs_coeff_for_bead_models )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_util->setup_saxs_maps( 
                                                               saxs_options->default_atom_filename ,
                                                               saxs_options->default_hybrid_filename ,
                                                               saxs_options->default_saxs_filename 
                                                               );
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_compute_sans_coeff_for_bead_models()
{
   (*saxs_options).compute_sans_coeff_for_bead_models = cb_compute_sans_coeff_for_bead_models->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_bead_model_rayleigh()
{
   (*saxs_options).bead_model_rayleigh = cb_bead_model_rayleigh->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::default_atom_filename()
{
   QString atom_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + SLASH + "etc", "*.atom *.ATOM", this);
   if (atom_filename.isEmpty())
   {
      return;
   }
   else
   {
      (*saxs_options).default_atom_filename = atom_filename;
      if ( (*saxs_options).compute_saxs_coeff_for_bead_models )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_util->setup_saxs_maps( 
                                                                  saxs_options->default_atom_filename ,
                                                                  saxs_options->default_hybrid_filename ,
                                                                  saxs_options->default_saxs_filename 
                                                                  );
      }
      le_default_atom_filename->setText( QFileInfo(atom_filename).fileName() );
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::default_hybrid_filename()
{
   QString hybrid_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + SLASH + "etc", "*.hybrid *.HYBRID", this);
   if (hybrid_filename.isEmpty())
   {
      return;
   }
   else
   {
      (*saxs_options).default_hybrid_filename = hybrid_filename;
      le_default_hybrid_filename->setText( QFileInfo(hybrid_filename).fileName() );
      if ( (*saxs_options).compute_saxs_coeff_for_bead_models )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_util->setup_saxs_maps( 
                                                                  saxs_options->default_atom_filename ,
                                                                  saxs_options->default_hybrid_filename ,
                                                                  saxs_options->default_saxs_filename 
                                                                  );
      }
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::default_saxs_filename()
{
   QString saxs_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + SLASH + "etc", "*.saxs_atoms *.SAXS_ATOMS", this);
   if (saxs_filename.isEmpty())
   {
      return;
   }
   else
   {
      (*saxs_options).default_saxs_filename = saxs_filename;
      le_default_saxs_filename->setText( QFileInfo(saxs_filename).fileName() );
      if ( (*saxs_options).compute_saxs_coeff_for_bead_models )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_util->setup_saxs_maps( 
                                                                  saxs_options->default_atom_filename ,
                                                                  saxs_options->default_hybrid_filename ,
                                                                  saxs_options->default_saxs_filename 
                                                                  );
      }
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::default_rotamer_filename()
{
   QString rotamer_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + SLASH + "etc", "*.hydrated_rotamer *.HYDRATED_ROTAMER", this);
   if (rotamer_filename.isEmpty())
   {
      return;
   }
   else
   {
      (*saxs_options).default_rotamer_filename = rotamer_filename;
      ((US_Hydrodyn *)us_hydrodyn)->rotamer_changed = true;      
      le_default_rotamer_filename->setText( QFileInfo(rotamer_filename).fileName() );
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
