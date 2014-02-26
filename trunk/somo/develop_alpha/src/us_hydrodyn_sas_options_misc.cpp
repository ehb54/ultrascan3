#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QCloseEvent>
#include <Q3GridLayout>
#include <Q3Frame>
#include <QLabel>

#define SLASH "/"
#if defined(WIN32)
#  undef SLASH
#  define SLASH "\\"
#endif

US_Hydrodyn_SasOptionsMisc::US_Hydrodyn_SasOptionsMisc(struct saxs_options *saxs_options, bool *sas_options_misc_widget, void *us_hydrodyn, QWidget *p, const char *name) : Q3Frame(p, name)
{
   this->sas_options_misc_widget = sas_options_misc_widget;
   this->saxs_options = saxs_options;
   this->us_hydrodyn = us_hydrodyn;
   *sas_options_misc_widget = true;
   USglobal=new US_Config();
   setPalette( PALET_FRAME );
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
   bool started_in_expert_mode = U_EXPT;

   int minHeight1 = 30;
   lbl_info = new QLabel(tr("US-SOMO SAS Miscellaneous Options:"), this);
   lbl_info->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   pb_default_atom_filename = new QPushButton(tr("Set Atom Definition File"), this);
   pb_default_atom_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_default_atom_filename->setMinimumHeight(minHeight1);
   pb_default_atom_filename->setPalette( PALET_PUSHB );
   connect(pb_default_atom_filename, SIGNAL(clicked()), SLOT(default_atom_filename()));
   
   le_default_atom_filename = new QLineEdit(this, "");
   le_default_atom_filename->setText(QFileInfo((*saxs_options).default_atom_filename).fileName());
   // le_default_atom_filename->setMinimumHeight(minHeight1);
   // le_default_atom_filename->setMaximumHeight(minHeight1);
   le_default_atom_filename->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_default_atom_filename->setPalette( PALET_EDIT );
   AUTFBACK( le_default_atom_filename );
   le_default_atom_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_default_atom_filename->setReadOnly(true);

   pb_default_hybrid_filename = new QPushButton(tr("Set Hybridization File"), this);
   pb_default_hybrid_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_default_hybrid_filename->setMinimumHeight(minHeight1);
   pb_default_hybrid_filename->setPalette( PALET_PUSHB );
   connect(pb_default_hybrid_filename, SIGNAL(clicked()), SLOT(default_hybrid_filename()));

   le_default_hybrid_filename = new QLineEdit(this, "");
   le_default_hybrid_filename->setText(QFileInfo((*saxs_options).default_hybrid_filename).fileName());
   // le_default_hybrid_filename->setMinimumHeight(minHeight1);
   // le_default_hybrid_filename->setMaximumHeight(minHeight1);
   le_default_hybrid_filename->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_default_hybrid_filename->setPalette( PALET_EDIT );
   AUTFBACK( le_default_hybrid_filename );
   le_default_hybrid_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_default_hybrid_filename->setReadOnly(true);

   pb_default_saxs_filename = new QPushButton(tr("Set SAXS Coefficients File"), this);
   pb_default_saxs_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_default_saxs_filename->setMinimumHeight(minHeight1);
   pb_default_saxs_filename->setPalette( PALET_PUSHB );
   connect(pb_default_saxs_filename, SIGNAL(clicked()), SLOT(default_saxs_filename()));

   le_default_saxs_filename = new QLineEdit(this, "");
   le_default_saxs_filename->setText(QFileInfo((*saxs_options).default_saxs_filename).fileName());
   // le_default_saxs_filename->setMinimumHeight(minHeight1);
   // le_default_saxs_filename->setMaximumHeight(minHeight1);
   le_default_saxs_filename->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_default_saxs_filename->setPalette( PALET_EDIT );
   AUTFBACK( le_default_saxs_filename );
   le_default_saxs_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_default_saxs_filename->setReadOnly(true);

   pb_default_ff_filename = new QPushButton(tr("Set SAXS Formfactor File"), this);
   pb_default_ff_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_default_ff_filename->setMinimumHeight(minHeight1);
   pb_default_ff_filename->setPalette( PALET_PUSHB );
   connect(pb_default_ff_filename, SIGNAL(clicked()), SLOT(default_ff_filename()));

   le_default_ff_filename = new QLineEdit(this, "");
   le_default_ff_filename->setText(QFileInfo((*saxs_options).default_ff_filename).fileName());
   le_default_ff_filename->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_default_ff_filename->setPalette( PALET_EDIT );
   AUTFBACK( le_default_ff_filename );
   le_default_ff_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_default_ff_filename->setReadOnly(true);

   // cb_iq_ask = new QCheckBox(this);
   // cb_iq_ask->setText(tr("Manually choose I(q) method"));
   // cb_iq_ask->setEnabled(true);
   // cb_iq_ask->setChecked((*saxs_options).iq_ask);
   // cb_iq_ask->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   // cb_iq_ask->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // connect(cb_iq_ask, SIGNAL(clicked()), this, SLOT(set_iq_ask()));

   cb_iq_scale_ask = new QCheckBox(this);
   cb_iq_scale_ask->setText(tr("Always ask 1/angstrom or 1/nm"));
   cb_iq_scale_ask->setEnabled(true);
   cb_iq_scale_ask->setChecked((*saxs_options).iq_scale_ask);
   cb_iq_scale_ask->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iq_scale_ask->setPalette( PALET_NORMAL );
   AUTFBACK( cb_iq_scale_ask );
   connect(cb_iq_scale_ask, SIGNAL(clicked()), this, SLOT(set_iq_scale_ask()));

   cb_iqq_ask_target_grid = new QCheckBox(this);
   cb_iqq_ask_target_grid->setText(tr("Calc I(q) ask for grid"));
   cb_iqq_ask_target_grid->setEnabled(true);
   cb_iqq_ask_target_grid->setChecked((*saxs_options).iqq_ask_target_grid);
   cb_iqq_ask_target_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_ask_target_grid->setPalette( PALET_NORMAL );
   AUTFBACK( cb_iqq_ask_target_grid );
   connect(cb_iqq_ask_target_grid, SIGNAL(clicked()), this, SLOT(set_iqq_ask_target_grid()));

   cb_iq_scale_angstrom = new QCheckBox(this);
   cb_iq_scale_angstrom->setText(tr("I(q) curves in 1/angstrom"));
   cb_iq_scale_angstrom->setEnabled(true);
   cb_iq_scale_angstrom->setChecked((*saxs_options).iq_scale_angstrom);
   cb_iq_scale_angstrom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iq_scale_angstrom->setPalette( PALET_NORMAL );
   AUTFBACK( cb_iq_scale_angstrom );
   connect(cb_iq_scale_angstrom, SIGNAL(clicked()), this, SLOT(set_iq_scale_angstrom()));

   cb_iq_scale_nm = new QCheckBox(this);
   cb_iq_scale_nm->setText(tr("I(q) curves in 1/nanometer"));
   cb_iq_scale_nm->setEnabled(true);
   cb_iq_scale_nm->setChecked((*saxs_options).iq_scale_nm);
   cb_iq_scale_nm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iq_scale_nm->setPalette( PALET_NORMAL );
   AUTFBACK( cb_iq_scale_nm );
   connect(cb_iq_scale_nm, SIGNAL(clicked()), this, SLOT(set_iq_scale_nm()));

   cb_iqq_expt_data_contains_variances = new QCheckBox(this);
   cb_iqq_expt_data_contains_variances->setText(tr("Expt. data in variance"));
   cb_iqq_expt_data_contains_variances->setEnabled(true);
   cb_iqq_expt_data_contains_variances->setChecked((*saxs_options).iqq_expt_data_contains_variances);
   cb_iqq_expt_data_contains_variances->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_expt_data_contains_variances->setPalette( PALET_NORMAL );
   AUTFBACK( cb_iqq_expt_data_contains_variances );
   connect(cb_iqq_expt_data_contains_variances, SIGNAL(clicked()), this, SLOT(set_iqq_expt_data_contains_variances()));

   cb_disable_iq_scaling = new QCheckBox(this);
   cb_disable_iq_scaling->setText(tr("Disable I(q) scaling"));
   cb_disable_iq_scaling->setEnabled(true);
   cb_disable_iq_scaling->setChecked((*saxs_options).disable_iq_scaling);
   cb_disable_iq_scaling->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_disable_iq_scaling->setPalette( PALET_NORMAL );
   AUTFBACK( cb_disable_iq_scaling );
   connect(cb_disable_iq_scaling, SIGNAL(clicked()), this, SLOT(set_disable_iq_scaling()));

   cb_iqq_scale_chi2_fitting = new QCheckBox(this);
   cb_iqq_scale_chi2_fitting->setText(tr("Chi^2 fitting"));
   cb_iqq_scale_chi2_fitting->setEnabled(true);
   cb_iqq_scale_chi2_fitting->setChecked((*saxs_options).iqq_scale_chi2_fitting);
   cb_iqq_scale_chi2_fitting->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_scale_chi2_fitting->setPalette( PALET_NORMAL );
   AUTFBACK( cb_iqq_scale_chi2_fitting );
   connect(cb_iqq_scale_chi2_fitting, SIGNAL(clicked()), this, SLOT(set_iqq_scale_chi2_fitting()));

   cb_iqq_kratky_fit = new QCheckBox(this);
   cb_iqq_kratky_fit->setText(tr("Kratky fit"));
   cb_iqq_kratky_fit->setEnabled(true);
   cb_iqq_kratky_fit->setChecked((*saxs_options).iqq_kratky_fit);
   cb_iqq_kratky_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq_kratky_fit->setPalette( PALET_NORMAL );
   AUTFBACK( cb_iqq_kratky_fit );
   connect(cb_iqq_kratky_fit, SIGNAL(clicked()), this, SLOT(set_iqq_kratky_fit()));

   cb_ignore_errors = new QCheckBox(this);
   cb_ignore_errors->setText(tr("Do not use experimental errors in Iq fits"));
   cb_ignore_errors->setEnabled(true);
   cb_ignore_errors->setChecked((*saxs_options).ignore_errors);
   cb_ignore_errors->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_ignore_errors->setPalette( PALET_NORMAL );
   AUTFBACK( cb_ignore_errors );
   connect(cb_ignore_errors, SIGNAL(clicked()), this, SLOT(set_ignore_errors()));

   lbl_swh_excl_vol = new QLabel(tr(" Excluded volume WAT [A^3]: "), this);
   lbl_swh_excl_vol->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_swh_excl_vol->setMinimumHeight(minHeight1);
   lbl_swh_excl_vol->setPalette( PALET_LABEL );
   AUTFBACK( lbl_swh_excl_vol );
   lbl_swh_excl_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_swh_excl_vol = new QLineEdit(this, "swh_excl_vol Line Edit");
   (*saxs_options).swh_excl_vol ? 
      le_swh_excl_vol->setText(QString("%1").arg((*saxs_options).swh_excl_vol)) :
      le_swh_excl_vol->setText("");
   // le_swh_excl_vol->setMinimumHeight(minHeight1);
   le_swh_excl_vol->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_swh_excl_vol->setPalette( PALET_NORMAL );
   AUTFBACK( le_swh_excl_vol );
   le_swh_excl_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_swh_excl_vol, SIGNAL(textChanged(const QString &)), SLOT(update_swh_excl_vol(const QString &)));

   lbl_scale_excl_vol = new QLabel(tr(" Excluded volume scaling: "), this);
   lbl_scale_excl_vol->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_scale_excl_vol->setMinimumHeight(minHeight1);
   lbl_scale_excl_vol->setPalette( PALET_LABEL );
   AUTFBACK( lbl_scale_excl_vol );
   lbl_scale_excl_vol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_scale_excl_vol = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_scale_excl_vol );
   cnt_scale_excl_vol->setRange(.1, 2, 0.001);
   cnt_scale_excl_vol->setValue((*saxs_options).scale_excl_vol);
   cnt_scale_excl_vol->setMinimumHeight(minHeight1);
   cnt_scale_excl_vol->setEnabled(true);
   cnt_scale_excl_vol->setNumButtons(3);
   cnt_scale_excl_vol->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_scale_excl_vol->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_scale_excl_vol );
   connect(cnt_scale_excl_vol, SIGNAL(valueChanged(double)), SLOT(update_scale_excl_vol(double)));

   if ( started_in_expert_mode )
   {
      cb_use_iq_target_ev = new QCheckBox(this);
      cb_use_iq_target_ev->setText(tr("Adjust I(q) computation to target total excluded volume (A^3):"));
      cb_use_iq_target_ev->setEnabled(true);
      cb_use_iq_target_ev->setChecked((*saxs_options).use_iq_target_ev);
      cb_use_iq_target_ev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_use_iq_target_ev->setPalette( PALET_NORMAL );
      AUTFBACK( cb_use_iq_target_ev );
      connect(cb_use_iq_target_ev, SIGNAL(clicked()), this, SLOT(set_use_iq_target_ev()));

      le_iq_target_ev = new QLineEdit(this, "iq_target_ev Line Edit");
      (*saxs_options).use_iq_target_ev ? 
         le_iq_target_ev->setText(QString("%1").arg((*saxs_options).iq_target_ev)) :
         le_iq_target_ev->setText("");
      // le_iq_target_ev->setMinimumHeight(minHeight1);
      le_iq_target_ev->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      le_iq_target_ev->setPalette( PALET_NORMAL );
      AUTFBACK( le_iq_target_ev );
      le_iq_target_ev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      connect(le_iq_target_ev, SIGNAL(textChanged(const QString &)), SLOT(update_iq_target_ev(const QString &)));

      cb_set_iq_target_ev_from_vbar = new QCheckBox(this);
      cb_set_iq_target_ev_from_vbar->setText(tr("Set target excluded volume from vbar"));
      cb_set_iq_target_ev_from_vbar->setEnabled(true);
      cb_set_iq_target_ev_from_vbar->setChecked((*saxs_options).set_iq_target_ev_from_vbar);
      cb_set_iq_target_ev_from_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_set_iq_target_ev_from_vbar->setPalette( PALET_NORMAL );
      AUTFBACK( cb_set_iq_target_ev_from_vbar );
      connect(cb_set_iq_target_ev_from_vbar, SIGNAL(clicked()), this, SLOT(set_set_iq_target_ev_from_vbar()));
   }

   lbl_iqq_scale_min_maxq = new QLabel(tr(" I(q) curve q range for scaling, NNLS and best fit (Angstrom) "), this);
   lbl_iqq_scale_min_maxq->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_iqq_scale_min_maxq->setMinimumHeight(minHeight1);
   lbl_iqq_scale_min_maxq->setPalette( PALET_LABEL );
   AUTFBACK( lbl_iqq_scale_min_maxq );
   lbl_iqq_scale_min_maxq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_iqq_scale_minq = new QLineEdit(this, "iqq_scale_minq Line Edit");
   (*saxs_options).iqq_scale_minq ? 
      le_iqq_scale_minq->setText(QString("%1").arg((*saxs_options).iqq_scale_minq)) :
      le_iqq_scale_minq->setText("");
   // le_iqq_scale_minq->setMinimumHeight(minHeight1);
   le_iqq_scale_minq->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_iqq_scale_minq->setPalette( PALET_NORMAL );
   AUTFBACK( le_iqq_scale_minq );
   le_iqq_scale_minq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_iqq_scale_minq, SIGNAL(textChanged(const QString &)), SLOT(update_iqq_scale_minq(const QString &)));

   le_iqq_scale_maxq = new QLineEdit(this, "iqq_scale_maxq Line Edit");
   (*saxs_options).iqq_scale_maxq ? 
      le_iqq_scale_maxq->setText(QString("%1").arg((*saxs_options).iqq_scale_maxq)) :
      le_iqq_scale_maxq->setText("");
   // le_iqq_scale_maxq->setMinimumHeight(minHeight1);
   le_iqq_scale_maxq->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_iqq_scale_maxq->setPalette( PALET_NORMAL );
   AUTFBACK( le_iqq_scale_maxq );
   le_iqq_scale_maxq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_iqq_scale_maxq, SIGNAL(textChanged(const QString &)), SLOT(update_iqq_scale_maxq(const QString &)));

   pb_clear_mw_cache = new QPushButton(tr("Clear remembered molecular weights"), this);
   pb_clear_mw_cache->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_clear_mw_cache->setMinimumHeight(minHeight1);
   pb_clear_mw_cache->setPalette( PALET_PUSHB );
   connect(pb_clear_mw_cache, SIGNAL(clicked()), SLOT(clear_mw_cache()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int rows = 0, columns = 2, spacing = 2, j=0, margin=4;
   Q3GridLayout *background=new Q3GridLayout(this, rows, columns, margin, spacing);

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
   background->addWidget(pb_default_ff_filename, j, 0);
   background->addWidget(le_default_ff_filename, j, 1);
   j++;

   Q3HBoxLayout *hbl_iq_ask = new Q3HBoxLayout;
   // hbl_iq_ask->addWidget(cb_iq_ask);
   hbl_iq_ask->addWidget(cb_iq_scale_ask);
   hbl_iq_ask->addWidget(cb_iqq_ask_target_grid);
   
   background->addMultiCellLayout(hbl_iq_ask, j, j, 0, 1);
   j++;

   Q3HBoxLayout *hbl_iq_scale = new Q3HBoxLayout;
   hbl_iq_scale->addWidget(cb_iq_scale_angstrom);
   hbl_iq_scale->addWidget(cb_iq_scale_nm);
   hbl_iq_scale->addWidget(cb_iqq_expt_data_contains_variances);
   background->addMultiCellLayout(hbl_iq_scale, j, j, 0, 1);
   j++;

   Q3HBoxLayout *hbl_various_1 = new Q3HBoxLayout;
   hbl_various_1->addWidget(cb_disable_iq_scaling);
   hbl_various_1->addWidget(cb_iqq_scale_chi2_fitting);
   background->addMultiCellLayout(hbl_various_1, j, j, 0, 1);
   j++;

   Q3HBoxLayout *hbl_various_1b = new Q3HBoxLayout;
   hbl_various_1b->addWidget(cb_iqq_kratky_fit);
   hbl_various_1b->addWidget(cb_ignore_errors);
   background->addMultiCellLayout(hbl_various_1b, j, j, 0, 1);
   j++;

   Q3HBoxLayout *hbl_various_2b = new Q3HBoxLayout;
   hbl_various_2b->addWidget(lbl_swh_excl_vol);
   hbl_various_2b->addWidget(le_swh_excl_vol);
   hbl_various_2b->addWidget(lbl_scale_excl_vol);
   hbl_various_2b->addWidget(cnt_scale_excl_vol);
   background->addMultiCellLayout(hbl_various_2b, j, j, 0, 1);
   j++;

   if ( started_in_expert_mode )
   {
      Q3HBoxLayout *hbl_various_2b2 = new Q3HBoxLayout;
      hbl_various_2b2->addWidget( cb_use_iq_target_ev );
      hbl_various_2b2->addWidget( le_iq_target_ev );
      background->addMultiCellLayout(hbl_various_2b2, j, j, 0, 1);
      j++;

      Q3HBoxLayout *hbl_various_2b3 = new Q3HBoxLayout;
      hbl_various_2b3->addWidget( cb_set_iq_target_ev_from_vbar );
      background->addMultiCellLayout(hbl_various_2b3, j, j, 0, 1);
      j++;
   }

   Q3HBoxLayout *hbl_various_2c = new Q3HBoxLayout;
   hbl_various_2c->addWidget( pb_clear_mw_cache );
   background->addMultiCellLayout(hbl_various_2c, j, j, 0, 1);
   j++;

   Q3HBoxLayout *hbl_iqq_scaling = new Q3HBoxLayout;
   hbl_iqq_scaling->addWidget(lbl_iqq_scale_min_maxq);
   hbl_iqq_scaling->addWidget(le_iqq_scale_minq);
   hbl_iqq_scaling->addWidget(le_iqq_scale_maxq);
   background->addMultiCellLayout(hbl_iqq_scaling, j, j, 0, 1);
   j++;

   background->addWidget( pb_help  , j, 0 );
   background->addWidget( pb_cancel, j, 1 );

   if ( !started_in_expert_mode )
   {
      pb_default_ff_filename->hide();
      le_default_ff_filename->hide();
   }
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
   // online_help->show_help("manual/somo_sas_options_misc.html");
   online_help->show_help("manual/somo_saxs_options.html");
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
   QString atom_filename = Q3FileDialog::getOpenFileName(USglobal->config_list.system_dir + SLASH + "etc", "*.atom *.ATOM", this);
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
   QString hybrid_filename = Q3FileDialog::getOpenFileName(USglobal->config_list.system_dir + SLASH + "etc", "*.hybrid *.HYBRID", this);
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
   QString saxs_filename = Q3FileDialog::getOpenFileName(USglobal->config_list.system_dir + SLASH + "etc", "*.saxs_atoms *.SAXS_ATOMS", this);
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

void US_Hydrodyn_SasOptionsMisc::default_ff_filename()
{
   QString ff_filename = Q3FileDialog::getOpenFileName(USglobal->config_list.system_dir + SLASH + "etc", "*.ff *.SAXS_ATOMS", this);
   if (ff_filename.isEmpty())
   {
      return;
   }
   else
   {
      (*saxs_options).default_ff_filename = ff_filename;
      le_default_ff_filename->setText( QFileInfo(ff_filename).fileName() );
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

void US_Hydrodyn_SasOptionsMisc::set_iqq_kratky_fit()
{
   (*saxs_options).iqq_kratky_fit = cb_iqq_kratky_fit->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsMisc::set_ignore_errors()
{
   (*saxs_options).ignore_errors = cb_ignore_errors->isChecked();
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

void US_Hydrodyn_SasOptionsMisc::set_use_iq_target_ev()
{
   (*saxs_options).use_iq_target_ev = cb_use_iq_target_ev->isChecked();
   if ( !cb_use_iq_target_ev->isChecked() && 
        (*saxs_options).scale_excl_vol != 1.0f )
   {
      QMessageBox::information( this,
                                tr("US-SOMO SAS Miscellaneous Options - Reset target scaling"), 
                                QString( tr( "The Excluded Volume Scaling which was %1 has been reset to 1.0" ) )
                                .arg( (*saxs_options).scale_excl_vol )
                                );
      cnt_scale_excl_vol->setValue( 1e0 );
      update_scale_excl_vol( 1e0 );
   }
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsMisc::set_set_iq_target_ev_from_vbar()
{
   (*saxs_options).set_iq_target_ev_from_vbar = cb_set_iq_target_ev_from_vbar->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsMisc::update_iq_target_ev( const QString &str )
{
   (*saxs_options).iq_target_ev = str.toFloat();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
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
