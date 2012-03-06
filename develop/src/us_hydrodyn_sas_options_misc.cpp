#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"

#define SLASH "/"
#if defined(WIN32)
#  undef SLASH
#  define SLASH "\\"
#endif

US_Hydrodyn_SasOptionsMisc::US_Hydrodyn_SasOptionsMisc(struct saxs_options *saxs_options, bool *sas_options_misc_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->sas_options_misc_widget = sas_options_misc_widget;
   this->saxs_options = saxs_options;
   this->us_hydrodyn = us_hydrodyn;
   *sas_options_misc_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO SAS Miscellaneous Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SasOptionsMisc::~US_Hydrodyn_SasOptionsMisc()
{
   *sas_options_misc_widget = false;
}

void US_Hydrodyn_SasOptionsMisc::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("US-SOMO SAS Miscellaneous Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

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

   cb_iqq_scale_chi2_fitting = new QCheckBox(this);
   cb_iqq_scale_chi2_fitting->setText(tr("Chi^2 fitting"));
   cb_iqq_scale_chi2_fitting->setEnabled(true);
   cb_iqq_scale_chi2_fitting->setChecked((*saxs_options).iqq_scale_chi2_fitting);
   cb_iqq_scale_chi2_fitting->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_scale_chi2_fitting->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq_scale_chi2_fitting, SIGNAL(clicked()), this, SLOT(set_iqq_scale_chi2_fitting()));

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

   pb_clear_mw_cache = new QPushButton(tr("Clear remembered molecular weights"), this);
   pb_clear_mw_cache->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_clear_mw_cache->setMinimumHeight(minHeight1);
   pb_clear_mw_cache->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_clear_mw_cache, SIGNAL(clicked()), SLOT(clear_mw_cache()));

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

   background->addWidget(pb_default_atom_filename, j, 0);
   background->addWidget(le_default_atom_filename, j, 1);
   j++;
   background->addWidget(pb_default_hybrid_filename, j, 0);
   background->addWidget(le_default_hybrid_filename, j, 1);
   j++;
   background->addWidget(pb_default_saxs_filename, j, 0);
   background->addWidget(le_default_saxs_filename, j, 1);
   j++;

   QHBoxLayout *hbl_iq_ask = new QHBoxLayout;
   hbl_iq_ask->addWidget(cb_iq_ask);
   hbl_iq_ask->addWidget(cb_iq_scale_ask);
   hbl_iq_ask->addWidget(cb_iqq_ask_target_grid);
   
   background->addMultiCellLayout(hbl_iq_ask, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_iq_scale = new QHBoxLayout;
   hbl_iq_scale->addWidget(cb_iq_scale_angstrom);
   hbl_iq_scale->addWidget(cb_iq_scale_nm);
   hbl_iq_scale->addWidget(cb_iqq_expt_data_contains_variances);
   background->addMultiCellLayout(hbl_iq_scale, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_various_1 = new QHBoxLayout;
   hbl_various_1->addWidget(cb_disable_iq_scaling);
   hbl_various_1->addWidget(cb_iqq_scale_chi2_fitting);
   background->addMultiCellLayout(hbl_various_1, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_various_2b = new QHBoxLayout;
   hbl_various_2b->addWidget(lbl_swh_excl_vol);
   hbl_various_2b->addWidget(le_swh_excl_vol);
   hbl_various_2b->addWidget(lbl_scale_excl_vol);
   hbl_various_2b->addWidget(cnt_scale_excl_vol);
   background->addMultiCellLayout(hbl_various_2b, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_various_2c = new QHBoxLayout;
   hbl_various_2c->addWidget( pb_clear_mw_cache );
   background->addMultiCellLayout(hbl_various_2c, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_iqq_scaling = new QHBoxLayout;
   hbl_iqq_scaling->addWidget(lbl_iqq_scale_min_maxq);
   hbl_iqq_scaling->addWidget(le_iqq_scale_minq);
   hbl_iqq_scaling->addWidget(le_iqq_scale_maxq);
   background->addMultiCellLayout(hbl_iqq_scaling, j, j, 0, 1);
   j++;

   background->addWidget( pb_help  , j, 0 );
   background->addWidget( pb_cancel, j, 1 );
}

void US_Hydrodyn_SasOptionsMisc::cancel()
{
   close();
}

void US_Hydrodyn_SasOptionsMisc::clear_mw_cache()
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

void US_Hydrodyn_SasOptionsMisc::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_sas_options_misc.html");
}

void US_Hydrodyn_SasOptionsMisc::closeEvent(QCloseEvent *e)
{
   *sas_options_misc_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_SasOptionsMisc::default_atom_filename()
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

void US_Hydrodyn_SasOptionsMisc::default_hybrid_filename()
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

void US_Hydrodyn_SasOptionsMisc::default_saxs_filename()
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


void US_Hydrodyn_SasOptionsMisc::set_iq_ask()
{
   (*saxs_options).iq_ask = cb_iq_ask->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsMisc::set_iq_scale_ask()
{
   (*saxs_options).iq_scale_ask = cb_iq_scale_ask->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsMisc::set_iqq_ask_target_grid()
{
   (*saxs_options).iqq_ask_target_grid = cb_iqq_ask_target_grid->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsMisc::set_iq_scale_angstrom()
{
   (*saxs_options).iq_scale_angstrom = cb_iq_scale_angstrom->isChecked();
   (*saxs_options).iq_scale_nm = !cb_iq_scale_angstrom->isChecked();
   cb_iq_scale_nm->setChecked((*saxs_options).iq_scale_nm);
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsMisc::set_iq_scale_nm()
{
   (*saxs_options).iq_scale_nm = cb_iq_scale_nm->isChecked();
   (*saxs_options).iq_scale_angstrom = !cb_iq_scale_nm->isChecked();
   cb_iq_scale_angstrom->setChecked((*saxs_options).iq_scale_angstrom);
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsMisc::set_iqq_expt_data_contains_variances()
{
   (*saxs_options).iqq_expt_data_contains_variances = cb_iqq_expt_data_contains_variances->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsMisc::set_disable_iq_scaling()
{
   (*saxs_options).disable_iq_scaling = cb_disable_iq_scaling->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsMisc::set_iqq_scale_chi2_fitting()
{
   (*saxs_options).iqq_scale_chi2_fitting = cb_iqq_scale_chi2_fitting->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsMisc::update_scale_excl_vol(double val)
{
   (*saxs_options).scale_excl_vol = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->update_iqq_suffix();
   }
}

void US_Hydrodyn_SasOptionsMisc::update_swh_excl_vol( const QString &str )
{
   (*saxs_options).swh_excl_vol = str.toFloat();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->update_iqq_suffix();
   }
}

void US_Hydrodyn_SasOptionsMisc::update_iqq_scale_minq( const QString &str )
{
   (*saxs_options).iqq_scale_minq = str.toFloat();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsMisc::update_iqq_scale_maxq( const QString &str )
{
   (*saxs_options).iqq_scale_maxq = str.toFloat();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
