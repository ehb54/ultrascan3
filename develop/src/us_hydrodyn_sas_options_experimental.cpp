#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"

#define SLASH "/"
#if defined(WIN32)
#  undef SLASH
#  define SLASH "\\"
#endif

US_Hydrodyn_SasOptionsExperimental::US_Hydrodyn_SasOptionsExperimental(struct saxs_options *saxs_options, bool *sas_options_experimental_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->sas_options_experimental_widget = sas_options_experimental_widget;
   this->saxs_options = saxs_options;
   this->us_hydrodyn = us_hydrodyn;
   *sas_options_experimental_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO SAS Experimental Code Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SasOptionsExperimental::~US_Hydrodyn_SasOptionsExperimental()
{
   *sas_options_experimental_widget = false;
}

void US_Hydrodyn_SasOptionsExperimental::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("SOMO SAS Experimental Code Options: NB: This is for development testing"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_iqq_scale_linear_offset = new QCheckBox(this);
   cb_iqq_scale_linear_offset->setText(tr("Scale with linear offset"));
   cb_iqq_scale_linear_offset->setEnabled(true);
   cb_iqq_scale_linear_offset->setChecked((*saxs_options).iqq_scale_linear_offset);
   cb_iqq_scale_linear_offset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_scale_linear_offset->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq_scale_linear_offset, SIGNAL(clicked()), this, SLOT(set_iqq_scale_linear_offset()));

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

   cb_subtract_radius = new QCheckBox(this);
   cb_subtract_radius->setText(tr("Subtract radii for debye pairwise distance "));
   cb_subtract_radius->setEnabled(true);
   cb_subtract_radius->setChecked((*saxs_options).subtract_radius);
   cb_subtract_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_subtract_radius->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_subtract_radius, SIGNAL(clicked()), this, SLOT(set_subtract_radius()));

   cb_iqq_use_atomic_ff = new QCheckBox(this);
   cb_iqq_use_atomic_ff->setText(tr(" Explicit hydrogens"));
   cb_iqq_use_atomic_ff->setEnabled(true);
   cb_iqq_use_atomic_ff->setChecked((*saxs_options).iqq_use_atomic_ff);
   cb_iqq_use_atomic_ff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_use_atomic_ff->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq_use_atomic_ff, SIGNAL(clicked()), this, SLOT(set_iqq_use_atomic_ff()));

   cb_iqq_use_saxs_excl_vol = new QCheckBox(this);
   cb_iqq_use_saxs_excl_vol->setText(tr(" Use saxs exclulded volume"));
   cb_iqq_use_saxs_excl_vol->setEnabled(true);
   cb_iqq_use_saxs_excl_vol->setChecked((*saxs_options).iqq_use_saxs_excl_vol);
   cb_iqq_use_saxs_excl_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_use_saxs_excl_vol->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq_use_saxs_excl_vol, SIGNAL(clicked()), this, SLOT(set_iqq_use_saxs_excl_vol()));

   cb_use_somo_ff = new QCheckBox(this);
   cb_use_somo_ff->setText(tr(" Use somo.ff" ) );
   cb_use_somo_ff->setEnabled(true);
   cb_use_somo_ff->setChecked((*saxs_options).use_somo_ff);
   cb_use_somo_ff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_use_somo_ff->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_use_somo_ff, SIGNAL(clicked()), this, SLOT(set_use_somo_ff()));

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

   cb_iqq_scale_play = new QCheckBox(this);
   cb_iqq_scale_play->setText(tr(" Manually adjust scaling"));
   cb_iqq_scale_play->setEnabled(true);
   cb_iqq_scale_play->setChecked((*saxs_options).iqq_scale_play);
   cb_iqq_scale_play->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_scale_play->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq_scale_play, SIGNAL(clicked()), this, SLOT(set_iqq_scale_play()));

   cb_alt_ff = new QCheckBox(this);
   cb_alt_ff->setText(tr(" Alternate ff computation"));
   cb_alt_ff->setEnabled(true);
   cb_alt_ff->setChecked((*saxs_options).alt_ff);
   cb_alt_ff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_alt_ff->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_alt_ff, SIGNAL(clicked()), this, SLOT(set_alt_ff()));

   cb_five_term_gaussians = new QCheckBox(this);
   cb_five_term_gaussians->setText(tr(" 5 term Gaussians"));
   cb_five_term_gaussians->setEnabled(true);
   cb_five_term_gaussians->setChecked((*saxs_options).five_term_gaussians);
   cb_five_term_gaussians->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_five_term_gaussians->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_five_term_gaussians, SIGNAL(clicked()), this, SLOT(set_five_term_gaussians()));

   cb_iq_exact_q = new QCheckBox(this);
   cb_iq_exact_q->setText(tr(" Exact q"));
   cb_iq_exact_q->setEnabled(true);
   cb_iq_exact_q->setChecked((*saxs_options).iq_exact_q);
   cb_iq_exact_q->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iq_exact_q->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iq_exact_q, SIGNAL(clicked()), this, SLOT(set_iq_exact_q()));

   cb_compute_exponentials = new QCheckBox(this);
   cb_compute_exponentials->setText( tr( " Optionally compute exponentials on load SAXS curve" ) );
   cb_compute_exponentials->setEnabled(true);
   cb_compute_exponentials->setChecked((*saxs_options).compute_exponentials);
   cb_compute_exponentials->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_exponentials->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_exponentials, SIGNAL(clicked()), this, SLOT(set_compute_exponentials()));

   //    cnt_compute_exponential_terms = new QwtCounter(this);
   //    US_Hydrodyn::sizeArrows( cnt_compute_exponential_terms );
   //    cnt_compute_exponential_terms->setRange( 0, 20, 1 );
   //    cnt_compute_exponential_terms->setValue((*saxs_options).compute_exponential_terms);
   //    cnt_compute_exponential_terms->setMinimumHeight(minHeight1);
   //    cnt_compute_exponential_terms->setEnabled(true);
   //    cnt_compute_exponential_terms->setNumButtons(1);
   //    cnt_compute_exponential_terms->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //    cnt_compute_exponential_terms->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   //    connect(cnt_compute_exponential_terms, SIGNAL(valueChanged(double)), SLOT(update_compute_exponential_terms(double)));

   cb_multiply_iq_by_atomic_volume = new QCheckBox(this);
   cb_multiply_iq_by_atomic_volume->setText( tr( " Multiply Iq by atomic volume (for structure facture computations)" ) );
   cb_multiply_iq_by_atomic_volume->setEnabled(true);
   cb_multiply_iq_by_atomic_volume->setChecked((*saxs_options).multiply_iq_by_atomic_volume);
   cb_multiply_iq_by_atomic_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_multiply_iq_by_atomic_volume->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_multiply_iq_by_atomic_volume, SIGNAL(clicked()), this, SLOT(set_multiply_iq_by_atomic_volume()));

   lbl_ev_exp_mult = new QLabel(tr(" Excluded volume exponential multiplier "), this);
   lbl_ev_exp_mult->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_ev_exp_mult->setMinimumHeight(minHeight1);
   lbl_ev_exp_mult->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_ev_exp_mult->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_ev_exp_mult = new QLineEdit(this, "ev_exp_mult Line Edit");
   le_ev_exp_mult->setText( QString( "%1" ).arg( ( *saxs_options ).ev_exp_mult ) );
   le_ev_exp_mult->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_ev_exp_mult->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_ev_exp_mult->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_ev_exp_mult, SIGNAL(textChanged(const QString &)), SLOT(update_ev_exp_mult(const QString &)));

   cb_alt_sh1 = new QCheckBox(this);
   cb_alt_sh1->setText(tr(" SH Alt 1"));
   cb_alt_sh1->setEnabled(true);
   cb_alt_sh1->setChecked((*saxs_options).alt_sh1);
   cb_alt_sh1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_alt_sh1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_alt_sh1, SIGNAL(clicked()), this, SLOT(set_alt_sh1()));

   cb_alt_sh2 = new QCheckBox(this);
   cb_alt_sh2->setText(tr(" SH Alt 2"));
   cb_alt_sh2->setEnabled(true);
   cb_alt_sh2->setChecked((*saxs_options).alt_sh2);
   cb_alt_sh2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_alt_sh2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_alt_sh2, SIGNAL(clicked()), this, SLOT(set_alt_sh2()));

   pb_create_somo_ff = new QPushButton(tr("Create somo.ff"), this);
   pb_create_somo_ff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_create_somo_ff->setMinimumHeight(minHeight1);
   pb_create_somo_ff->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_create_somo_ff, SIGNAL(clicked()), SLOT(create_somo_ff()));

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

   int rows = 0, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_various_1 = new QHBoxLayout;
   hbl_various_1->addWidget(cb_iqq_scale_linear_offset);
   background->addMultiCellLayout(hbl_various_1, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_various_2 = new QHBoxLayout;
   hbl_various_2->addWidget(cb_autocorrelate);
   hbl_various_2->addWidget(cb_hybrid_radius_excl_vol);
   hbl_various_2->addWidget(cb_subtract_radius);
   background->addMultiCellLayout(hbl_various_2, j, j, 0, 1);
   j++;


   QHBoxLayout *hbl_various_2c = new QHBoxLayout;
   hbl_various_2c->addWidget( cb_iqq_use_atomic_ff );
   hbl_various_2c->addWidget( cb_iqq_use_saxs_excl_vol );
   hbl_various_2c->addWidget( cb_use_somo_ff );
   hbl_various_2c->addWidget( cb_iq_exact_q );
   background->addMultiCellLayout(hbl_various_2c, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_various_3 = new QHBoxLayout;
   
   hbl_various_3->addWidget(cb_iqq_scale_nnls);
   hbl_various_3->addWidget(cb_iqq_log_fitting);
   hbl_various_3->addWidget(cb_iqq_scale_play);
   hbl_various_3->addWidget(cb_alt_ff);
   hbl_various_3->addWidget(cb_five_term_gaussians);
   background->addMultiCellLayout(hbl_various_3, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_various_4 = new QHBoxLayout;
   
   hbl_various_4->addWidget( cb_compute_exponentials );
   //    hbl_various_4->addWidget( cnt_compute_exponential_terms );
   hbl_various_4->addWidget( cb_multiply_iq_by_atomic_volume );
   background->addMultiCellLayout(hbl_various_4, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_various_5 = new QHBoxLayout;
   hbl_various_5->addWidget( lbl_ev_exp_mult );
   hbl_various_5->addWidget( le_ev_exp_mult );
   background->addMultiCellLayout(hbl_various_5, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_various_6 = new QHBoxLayout;
   hbl_various_6->addWidget(cb_alt_sh1);
   hbl_various_6->addWidget(cb_alt_sh2);
   background->addMultiCellLayout(hbl_various_6, j, j, 0, 1);
   j++;

   background->addMultiCellWidget(pb_create_somo_ff, j, j, 0, 1);
   j++;

   background->addWidget( pb_help  , j, 0 );
   background->addWidget( pb_cancel, j, 1 );
}

void US_Hydrodyn_SasOptionsExperimental::cancel()
{
   close();
}

void US_Hydrodyn_SasOptionsExperimental::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   // online_help->show_help("manual/somo_sas_options_experimental.html");
   online_help->show_help("manual/somo_saxs_options.html");
}

void US_Hydrodyn_SasOptionsExperimental::closeEvent(QCloseEvent *e)
{
   *sas_options_experimental_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_SasOptionsExperimental::set_iqq_scale_linear_offset()
{
   (*saxs_options).iqq_scale_linear_offset = cb_iqq_scale_linear_offset->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_autocorrelate()
{
   (*saxs_options).autocorrelate = cb_autocorrelate->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_hybrid_radius_excl_vol()
{
   (*saxs_options).hybrid_radius_excl_vol = cb_hybrid_radius_excl_vol->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_subtract_radius()
{
   (*saxs_options).subtract_radius = cb_subtract_radius->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_iqq_use_atomic_ff()
{
   (*saxs_options).iqq_use_atomic_ff = cb_iqq_use_atomic_ff->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_iqq_use_saxs_excl_vol()
{
   (*saxs_options).iqq_use_saxs_excl_vol = cb_iqq_use_saxs_excl_vol->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_use_somo_ff()
{
   (*saxs_options).use_somo_ff = cb_use_somo_ff->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_iqq_scale_nnls()
{
   (*saxs_options).iqq_scale_nnls = cb_iqq_scale_nnls->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_iqq_log_fitting()
{
   (*saxs_options).iqq_log_fitting = cb_iqq_log_fitting->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_iqq_scale_play()
{
   (*saxs_options).iqq_scale_play = cb_iqq_scale_play->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_alt_ff()
{
   (*saxs_options).alt_ff = cb_alt_ff->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_five_term_gaussians()
{
   (*saxs_options).five_term_gaussians = cb_five_term_gaussians->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_iq_exact_q()
{
   (*saxs_options).iq_exact_q = cb_iq_exact_q->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_compute_exponentials()
{
   (*saxs_options).compute_exponentials = cb_compute_exponentials->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::update_compute_exponential_terms( double val )
{
   (*saxs_options).compute_exponential_terms = (unsigned int) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_multiply_iq_by_atomic_volume()
{
   (*saxs_options).multiply_iq_by_atomic_volume = cb_multiply_iq_by_atomic_volume->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();

}

void US_Hydrodyn_SasOptionsExperimental::set_alt_sh1()
{
   (*saxs_options).alt_sh1 = cb_alt_sh1->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_alt_sh2()
{
   (*saxs_options).alt_sh2 = cb_alt_sh2->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::create_somo_ff()
{
   QString errormsg = "";
   if ( !((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      errormsg = "The SAXS window must be open";
   } else {
      if ( !((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->create_somo_ff() )
      {
         errormsg = ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->errormsg;
      }
   }
   if ( !errormsg.isEmpty() )
   {
      QMessageBox::warning( this,
                            "US-SOMO: Create somo.ff",
                            errormsg );
   } else {
      QMessageBox::information( this,
                                "US-SOMO: Create somo.ff",
                                "somo.ff.new created" );
   }
}

void US_Hydrodyn_SasOptionsExperimental::update_ev_exp_mult( const QString &str )
{
   (*saxs_options).ev_exp_mult = str.toDouble();
}
