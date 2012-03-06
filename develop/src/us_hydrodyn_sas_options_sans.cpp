#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"

#define SLASH "/"
#if defined(WIN32)
#  undef SLASH
#  define SLASH "\\"
#endif

US_Hydrodyn_SasOptionsSans::US_Hydrodyn_SasOptionsSans(struct saxs_options *saxs_options, bool *sas_options_sans_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->sas_options_sans_widget = sas_options_sans_widget;
   this->saxs_options = saxs_options;
   this->us_hydrodyn = us_hydrodyn;
   *sas_options_sans_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO SANS Computation Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SasOptionsSans::~US_Hydrodyn_SasOptionsSans()
{
   *sas_options_sans_widget = false;
}

void US_Hydrodyn_SasOptionsSans::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("US-SOMO SANS Computation Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_h_scat_len = new QLabel(tr(" H scattering length (*10^-12 cm): "), this);
   lbl_h_scat_len->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_h_scat_len->setMinimumHeight(minHeight1);
   lbl_h_scat_len->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_h_scat_len->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_h_scat_len = new QwtCounter(this);
   cnt_h_scat_len->setRange(-0.5, 0.5, 0.0001);
   cnt_h_scat_len->setValue((*saxs_options).h_scat_len);
   cnt_h_scat_len->setMinimumHeight(minHeight1);
   cnt_h_scat_len->setEnabled(true);
   cnt_h_scat_len->setNumButtons(3);
   cnt_h_scat_len->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_h_scat_len->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_h_scat_len, SIGNAL(valueChanged(double)), SLOT(update_h_scat_len(double)));

   lbl_d_scat_len = new QLabel(tr(" D scattering length (*10^-12 cm): "), this);
   lbl_d_scat_len->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_d_scat_len->setMinimumHeight(minHeight1);
   lbl_d_scat_len->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_d_scat_len->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_d_scat_len = new QwtCounter(this);
   cnt_d_scat_len->setRange(0.1, 1.0, 0.0001);
   cnt_d_scat_len->setValue((*saxs_options).d_scat_len);
   cnt_d_scat_len->setMinimumHeight(minHeight1);
   cnt_d_scat_len->setEnabled(true);
   cnt_d_scat_len->setNumButtons(3);
   cnt_d_scat_len->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_d_scat_len->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_d_scat_len, SIGNAL(valueChanged(double)), SLOT(update_d_scat_len(double)));

   lbl_h2o_scat_len_dens = new QLabel(tr(" H2O scattering length density (*10^-10 cm^2): "), this);
   lbl_h2o_scat_len_dens->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_h2o_scat_len_dens->setMinimumHeight(minHeight1);
   lbl_h2o_scat_len_dens->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_h2o_scat_len_dens->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_h2o_scat_len_dens = new QwtCounter(this);
   cnt_h2o_scat_len_dens->setRange(-0.9, 0.0, 0.001);
   cnt_h2o_scat_len_dens->setValue((*saxs_options).h2o_scat_len_dens);
   cnt_h2o_scat_len_dens->setMinimumHeight(minHeight1);
   cnt_h2o_scat_len_dens->setEnabled(true);
   cnt_h2o_scat_len_dens->setNumButtons(3);
   cnt_h2o_scat_len_dens->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_h2o_scat_len_dens->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_h2o_scat_len_dens, SIGNAL(valueChanged(double)), SLOT(update_h2o_scat_len_dens(double)));

   lbl_d2o_scat_len_dens = new QLabel(tr(" D2O scattering length density (*10^-10 cm^2): "), this);
   lbl_d2o_scat_len_dens->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_d2o_scat_len_dens->setMinimumHeight(minHeight1);
   lbl_d2o_scat_len_dens->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_d2o_scat_len_dens->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_d2o_scat_len_dens = new QwtCounter(this);
   cnt_d2o_scat_len_dens->setRange(0.0, 10.0, 0.001);
   cnt_d2o_scat_len_dens->setValue((*saxs_options).d2o_scat_len_dens);
   cnt_d2o_scat_len_dens->setMinimumHeight(minHeight1);
   cnt_d2o_scat_len_dens->setEnabled(true);
   cnt_d2o_scat_len_dens->setNumButtons(3);
   cnt_d2o_scat_len_dens->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_d2o_scat_len_dens->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_d2o_scat_len_dens, SIGNAL(valueChanged(double)), SLOT(update_d2o_scat_len_dens(double)));

   lbl_d2o_conc = new QLabel(tr(" Buffer D2O fraction (0 - 1): "), this);
   lbl_d2o_conc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_d2o_conc->setMinimumHeight(minHeight1);
   lbl_d2o_conc->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_d2o_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_d2o_conc = new QwtCounter(this);
   cnt_d2o_conc->setRange(0.0, 1.0, 0.01);
   cnt_d2o_conc->setValue((*saxs_options).d2o_conc);
   cnt_d2o_conc->setMinimumHeight(minHeight1);
   cnt_d2o_conc->setEnabled(true);
   cnt_d2o_conc->setNumButtons(3);
   cnt_d2o_conc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_d2o_conc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_d2o_conc, SIGNAL(valueChanged(double)), SLOT(update_d2o_conc(double)));

   lbl_frac_of_exch_pep = new QLabel(tr(" Fraction of non-exchanged peptide H (0 - 1): "), this);
   lbl_frac_of_exch_pep->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_frac_of_exch_pep->setMinimumHeight(minHeight1);
   lbl_frac_of_exch_pep->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_frac_of_exch_pep->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_frac_of_exch_pep = new QwtCounter(this);
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

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int rows=0, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
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

   background->addWidget( pb_help  , j, 0 );
   background->addWidget( pb_cancel, j, 1 );
}

void US_Hydrodyn_SasOptionsSans::cancel()
{
   close();
}

void US_Hydrodyn_SasOptionsSans::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_sas_options_sans.html");
}

void US_Hydrodyn_SasOptionsSans::closeEvent(QCloseEvent *e)
{
   *sas_options_sans_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_SasOptionsSans::update_h_scat_len(double val)
{
   (*saxs_options).h_scat_len = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSans::update_d_scat_len(double val)
{
   (*saxs_options).d_scat_len = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSans::update_h2o_scat_len_dens(double val)
{
   (*saxs_options).h2o_scat_len_dens = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSans::update_d2o_scat_len_dens(double val)
{
   (*saxs_options).d2o_scat_len_dens = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSans::update_d2o_conc(double val)
{
   (*saxs_options).d2o_conc = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSans::update_frac_of_exch_pep(double val)
{
   (*saxs_options).frac_of_exch_pep = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSans::set_sans_iq_native_debye()
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

void US_Hydrodyn_SasOptionsSans::set_sans_iq_native_fast_compute_pr()
{
   (*saxs_options).sans_iq_native_fast_compute_pr = cb_sans_iq_native_fast_compute_pr->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSans::set_sans_iq_native_hybrid()
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

void US_Hydrodyn_SasOptionsSans::set_sans_iq_native_hybrid2()
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

void US_Hydrodyn_SasOptionsSans::set_sans_iq_native_hybrid3()
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

void US_Hydrodyn_SasOptionsSans::set_sans_iq_hybrid_adaptive()
{
   (*saxs_options).sans_iq_hybrid_adaptive = cb_sans_iq_hybrid_adaptive->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->set_current_method_buttons();
   }
}

void US_Hydrodyn_SasOptionsSans::set_sans_iq_native_fast()
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

void US_Hydrodyn_SasOptionsSans::set_sans_iq_cryson()
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
