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
   cb_iqq_scaled_fitting->setText(tr(" I(q) NNLS q^2*I fit"));
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
   background->addMultiCellLayout(hbl_various_2c, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_various_3 = new QHBoxLayout;
   
   hbl_various_3->addWidget(cb_iqq_scale_nnls);
   hbl_various_3->addWidget(cb_iqq_log_fitting);
   hbl_various_3->addWidget(cb_iqq_scaled_fitting);
   hbl_various_3->addWidget(cb_iqq_scale_play);
   background->addMultiCellLayout(hbl_various_3, j, j, 0, 1);
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
   online_help->show_help("manual/somo_sas_options_experimental.html");
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

void US_Hydrodyn_SasOptionsExperimental::set_iqq_scaled_fitting()
{
   (*saxs_options).iqq_scaled_fitting = cb_iqq_scaled_fitting->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_iqq_scale_play()
{
   (*saxs_options).iqq_scale_play = cb_iqq_scale_play->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
