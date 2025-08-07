#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_asa.h"
#include "qvalidator.h"
// Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

#define SLASH "/"
#if defined(WIN32)
#undef SLASH
#define SLASH "\\"
#endif

US_Hydrodyn_SasOptionsExperimental::US_Hydrodyn_SasOptionsExperimental(
    struct saxs_options *saxs_options, bool *sas_options_experimental_widget,
    void *us_hydrodyn, QWidget *p, const char *)
    : QFrame(p) {
  this->sas_options_experimental_widget = sas_options_experimental_widget;
  this->saxs_options = saxs_options;
  this->us_hydrodyn = us_hydrodyn;
  *sas_options_experimental_widget = true;
  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("US-SOMO SAS Experimental Code Options"));
  setupGUI();
  global_Xpos += 30;
  global_Ypos += 30;
  setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SasOptionsExperimental::~US_Hydrodyn_SasOptionsExperimental() {
  *sas_options_experimental_widget = false;
}

void US_Hydrodyn_SasOptionsExperimental::setupGUI() {
  int minHeight1 = 30;
  lbl_info = new QLabel(us_tr("SOMO SAS Experimental Code Options: NB: This is "
                              "for development testing"),
                        this);
  lbl_info->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_info->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_info->setMinimumHeight(minHeight1);
  lbl_info->setPalette(PALET_FRAME);
  AUTFBACK(lbl_info);
  lbl_info->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize + 1, QFont::Bold));

  cb_iqq_scale_linear_offset = new QCheckBox(this);
  cb_iqq_scale_linear_offset->setText(us_tr("Scale with linear offset"));
  cb_iqq_scale_linear_offset->setEnabled(true);
  cb_iqq_scale_linear_offset->setChecked(
      (*saxs_options).iqq_scale_linear_offset);
  cb_iqq_scale_linear_offset->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_iqq_scale_linear_offset->setPalette(PALET_NORMAL);
  AUTFBACK(cb_iqq_scale_linear_offset);
  connect(cb_iqq_scale_linear_offset, SIGNAL(clicked()), this,
          SLOT(set_iqq_scale_linear_offset()));

  cb_autocorrelate = new QCheckBox(this);
  cb_autocorrelate->setText(us_tr("Autocorrelate"));
  cb_autocorrelate->setEnabled(true);
  cb_autocorrelate->setChecked((*saxs_options).autocorrelate);
  cb_autocorrelate->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_autocorrelate->setPalette(PALET_NORMAL);
  AUTFBACK(cb_autocorrelate);
  connect(cb_autocorrelate, SIGNAL(clicked()), this, SLOT(set_autocorrelate()));

  cb_hybrid_radius_excl_vol = new QCheckBox(this);
  cb_hybrid_radius_excl_vol->setText(
      us_tr("Use hybrid radius for excluded volume"));
  cb_hybrid_radius_excl_vol->setEnabled(true);
  cb_hybrid_radius_excl_vol->setChecked((*saxs_options).hybrid_radius_excl_vol);
  cb_hybrid_radius_excl_vol->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_hybrid_radius_excl_vol->setPalette(PALET_NORMAL);
  AUTFBACK(cb_hybrid_radius_excl_vol);
  connect(cb_hybrid_radius_excl_vol, SIGNAL(clicked()), this,
          SLOT(set_hybrid_radius_excl_vol()));

  cb_subtract_radius = new QCheckBox(this);
  cb_subtract_radius->setText(
      us_tr("Subtract radii for debye pairwise distance "));
  cb_subtract_radius->setEnabled(true);
  cb_subtract_radius->setChecked((*saxs_options).subtract_radius);
  cb_subtract_radius->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_subtract_radius->setPalette(PALET_NORMAL);
  AUTFBACK(cb_subtract_radius);
  connect(cb_subtract_radius, SIGNAL(clicked()), this,
          SLOT(set_subtract_radius()));

  cb_iqq_use_atomic_ff = new QCheckBox(this);
  cb_iqq_use_atomic_ff->setText(us_tr(" Explicit hydrogens"));
  cb_iqq_use_atomic_ff->setEnabled(true);
  cb_iqq_use_atomic_ff->setChecked((*saxs_options).iqq_use_atomic_ff);
  cb_iqq_use_atomic_ff->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_iqq_use_atomic_ff->setPalette(PALET_NORMAL);
  AUTFBACK(cb_iqq_use_atomic_ff);
  connect(cb_iqq_use_atomic_ff, SIGNAL(clicked()), this,
          SLOT(set_iqq_use_atomic_ff()));

  cb_iqq_use_saxs_excl_vol = new QCheckBox(this);
  cb_iqq_use_saxs_excl_vol->setText(us_tr(" Use saxs exclulded volume"));
  cb_iqq_use_saxs_excl_vol->setEnabled(true);
  cb_iqq_use_saxs_excl_vol->setChecked((*saxs_options).iqq_use_saxs_excl_vol);
  cb_iqq_use_saxs_excl_vol->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_iqq_use_saxs_excl_vol->setPalette(PALET_NORMAL);
  AUTFBACK(cb_iqq_use_saxs_excl_vol);
  connect(cb_iqq_use_saxs_excl_vol, SIGNAL(clicked()), this,
          SLOT(set_iqq_use_saxs_excl_vol()));

  cb_use_somo_ff = new QCheckBox(this);
  cb_use_somo_ff->setText(us_tr(" Use somo.ff"));
  cb_use_somo_ff->setEnabled(true);
  cb_use_somo_ff->setChecked((*saxs_options).use_somo_ff);
  cb_use_somo_ff->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_use_somo_ff->setPalette(PALET_NORMAL);
  AUTFBACK(cb_use_somo_ff);
  connect(cb_use_somo_ff, SIGNAL(clicked()), this, SLOT(set_use_somo_ff()));

  cb_iqq_scale_nnls = new QCheckBox(this);
  cb_iqq_scale_nnls->setText(us_tr(" Use alternate scaling function"));
  cb_iqq_scale_nnls->setEnabled(true);
  cb_iqq_scale_nnls->setChecked((*saxs_options).iqq_scale_nnls);
  cb_iqq_scale_nnls->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_iqq_scale_nnls->setPalette(PALET_NORMAL);
  AUTFBACK(cb_iqq_scale_nnls);
  connect(cb_iqq_scale_nnls, SIGNAL(clicked()), this,
          SLOT(set_iqq_scale_nnls()));

  cb_iqq_log_fitting = new QCheckBox(this);
  cb_iqq_log_fitting->setText(us_tr(" I(q) NNLS log fit"));
  cb_iqq_log_fitting->setEnabled(true);
  cb_iqq_log_fitting->setChecked((*saxs_options).iqq_log_fitting);
  cb_iqq_log_fitting->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_iqq_log_fitting->setPalette(PALET_NORMAL);
  AUTFBACK(cb_iqq_log_fitting);
  connect(cb_iqq_log_fitting, SIGNAL(clicked()), this,
          SLOT(set_iqq_log_fitting()));

  cb_iqq_scale_play = new QCheckBox(this);
  cb_iqq_scale_play->setText(us_tr(" Manually adjust scaling"));
  cb_iqq_scale_play->setEnabled(true);
  cb_iqq_scale_play->setChecked((*saxs_options).iqq_scale_play);
  cb_iqq_scale_play->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_iqq_scale_play->setPalette(PALET_NORMAL);
  AUTFBACK(cb_iqq_scale_play);
  connect(cb_iqq_scale_play, SIGNAL(clicked()), this,
          SLOT(set_iqq_scale_play()));

  cb_alt_ff = new QCheckBox(this);
  cb_alt_ff->setText(us_tr(" Alternate ff computation"));
  cb_alt_ff->setEnabled(true);
  cb_alt_ff->setChecked((*saxs_options).alt_ff);
  cb_alt_ff->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_alt_ff->setPalette(PALET_NORMAL);
  AUTFBACK(cb_alt_ff);
  connect(cb_alt_ff, SIGNAL(clicked()), this, SLOT(set_alt_ff()));

  cb_five_term_gaussians = new QCheckBox(this);
  cb_five_term_gaussians->setText(us_tr(" 5 term Gaussians"));
  cb_five_term_gaussians->setEnabled(true);
  cb_five_term_gaussians->setChecked((*saxs_options).five_term_gaussians);
  cb_five_term_gaussians->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_five_term_gaussians->setPalette(PALET_NORMAL);
  AUTFBACK(cb_five_term_gaussians);
  connect(cb_five_term_gaussians, SIGNAL(clicked()), this,
          SLOT(set_five_term_gaussians()));

  cb_iq_exact_q = new QCheckBox(this);
  cb_iq_exact_q->setText(us_tr(" Exact q"));
  cb_iq_exact_q->setEnabled(true);
  cb_iq_exact_q->setChecked((*saxs_options).iq_exact_q);
  cb_iq_exact_q->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_iq_exact_q->setPalette(PALET_NORMAL);
  AUTFBACK(cb_iq_exact_q);
  connect(cb_iq_exact_q, SIGNAL(clicked()), this, SLOT(set_iq_exact_q()));

  cb_compute_exponentials = new QCheckBox(this);
  cb_compute_exponentials->setText(
      us_tr(" Optionally compute exponentials on load SAXS curve"));
  cb_compute_exponentials->setEnabled(true);
  cb_compute_exponentials->setChecked((*saxs_options).compute_exponentials);
  cb_compute_exponentials->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_exponentials->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_exponentials);
  connect(cb_compute_exponentials, SIGNAL(clicked()), this,
          SLOT(set_compute_exponentials()));

  //    cnt_compute_exponential_terms = new QwtCounter(this);
  //    US_Hydrodyn::sizeArrows( cnt_compute_exponential_terms );
  //    cnt_compute_exponential_terms->setRange( 0, 20);
  //    cnt_compute_exponential_terms->setSingleStep( 1 );
  //    cnt_compute_exponential_terms->setValue((*saxs_options).compute_exponential_terms);
  //    cnt_compute_exponential_terms->setMinimumHeight(minHeight1);
  //    cnt_compute_exponential_terms->setEnabled(true);
  //    cnt_compute_exponential_terms->setNumButtons(1);
  //    cnt_compute_exponential_terms->setFont(QFont(USglobal->config_list.fontFamily,
  //    USglobal->config_list.fontSize));
  //    cnt_compute_exponential_terms->setPalette( PALET_NORMAL );
  //    connect(cnt_compute_exponential_terms, SIGNAL(valueChanged(double)),
  //    SLOT(update_compute_exponential_terms(double)));

  cb_multiply_iq_by_atomic_volume = new QCheckBox(this);
  cb_multiply_iq_by_atomic_volume->setText(us_tr(
      " Multiply Iq by atomic volume (for structure facture computations)"));
  cb_multiply_iq_by_atomic_volume->setEnabled(true);
  cb_multiply_iq_by_atomic_volume->setChecked(
      (*saxs_options).multiply_iq_by_atomic_volume);
  cb_multiply_iq_by_atomic_volume->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_multiply_iq_by_atomic_volume->setPalette(PALET_NORMAL);
  AUTFBACK(cb_multiply_iq_by_atomic_volume);
  connect(cb_multiply_iq_by_atomic_volume, SIGNAL(clicked()), this,
          SLOT(set_multiply_iq_by_atomic_volume()));

  lbl_ev_exp_mult =
      new QLabel(us_tr(" Excluded volume exponential multiplier "), this);
  lbl_ev_exp_mult->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_ev_exp_mult->setMinimumHeight(minHeight1);
  lbl_ev_exp_mult->setPalette(PALET_LABEL);
  AUTFBACK(lbl_ev_exp_mult);
  lbl_ev_exp_mult->setFont(QFont(USglobal->config_list.fontFamily,
                                 USglobal->config_list.fontSize - 1,
                                 QFont::Bold));

  le_ev_exp_mult = new QLineEdit(this);
  le_ev_exp_mult->setObjectName("ev_exp_mult Line Edit");
  le_ev_exp_mult->setText(QString("%1").arg((*saxs_options).ev_exp_mult));
  le_ev_exp_mult->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_ev_exp_mult->setPalette(PALET_NORMAL);
  AUTFBACK(le_ev_exp_mult);
  le_ev_exp_mult->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_ev_exp_mult, SIGNAL(textChanged(const QString &)),
          SLOT(update_ev_exp_mult(const QString &)));

  cb_compute_chi2shannon = new QCheckBox(this);
  cb_compute_chi2shannon->setText(us_tr(" Compute Chi^2 Shannon"));
  cb_compute_chi2shannon->setEnabled(true);
  cb_compute_chi2shannon->setChecked(
      ((US_Hydrodyn *)us_hydrodyn)->gparams.count("compute_chi2shannon") &&
      ((US_Hydrodyn *)us_hydrodyn)->gparams["compute_chi2shannon"] == "1");
  cb_compute_chi2shannon->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_chi2shannon->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_chi2shannon);
  connect(cb_compute_chi2shannon, SIGNAL(clicked()), this,
          SLOT(set_compute_chi2shannon()));

  lbl_chi2shannon_dmax = new QLabel(us_tr(" Chi^2_Shannon Dmax : "), this);
  lbl_chi2shannon_dmax->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_chi2shannon_dmax->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chi2shannon_dmax);
  lbl_chi2shannon_dmax->setFont(QFont(USglobal->config_list.fontFamily,
                                      USglobal->config_list.fontSize - 1,
                                      QFont::Bold));

  le_chi2shannon_dmax = new QLineEdit(this);
  le_chi2shannon_dmax->setValidator(new QDoubleValidator(le_chi2shannon_dmax));
  ((QDoubleValidator *)le_chi2shannon_dmax->validator())
      ->setRange(1e0, 10000e0);
  le_chi2shannon_dmax->setText(QString("%1").arg(
      ((US_Hydrodyn *)us_hydrodyn)->gparams.count("chi2shannon_dmax")
          ? ((US_Hydrodyn *)us_hydrodyn)->gparams["chi2shannon_dmax"].toDouble()
          : 50e0));
  le_chi2shannon_dmax->setEnabled(true);
  le_chi2shannon_dmax->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_chi2shannon_dmax->setPalette(PALET_NORMAL);
  AUTFBACK(le_chi2shannon_dmax);
  connect(le_chi2shannon_dmax, SIGNAL(textChanged(const QString &)),
          SLOT(update_chi2shannon_dmax(const QString &)));

  lbl_chi2shannon_k = new QLabel(us_tr(" Chi^2_Shannon Iterations : "), this);
  lbl_chi2shannon_k->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_chi2shannon_k->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chi2shannon_k);
  lbl_chi2shannon_k->setFont(QFont(USglobal->config_list.fontFamily,
                                   USglobal->config_list.fontSize - 1,
                                   QFont::Bold));

  le_chi2shannon_k = new QLineEdit(this);
  le_chi2shannon_k->setValidator(new QIntValidator(le_chi2shannon_k));
  ((QIntValidator *)le_chi2shannon_k->validator())->setRange(5, 100000);
  le_chi2shannon_k->setText(QString("%1").arg(
      ((US_Hydrodyn *)us_hydrodyn)->gparams.count("chi2shannon_k")
          ? ((US_Hydrodyn *)us_hydrodyn)->gparams["chi2shannon_k"].toInt()
          : 1000));
  le_chi2shannon_k->setEnabled(true);
  le_chi2shannon_k->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_chi2shannon_k->setPalette(PALET_NORMAL);
  AUTFBACK(le_chi2shannon_k);
  connect(le_chi2shannon_k, SIGNAL(textChanged(const QString &)),
          SLOT(update_chi2shannon_k(const QString &)));

  cb_alt_sh1 = new QCheckBox(this);
  cb_alt_sh1->setText(us_tr(" SH Alt 1"));
  cb_alt_sh1->setEnabled(true);
  cb_alt_sh1->setChecked((*saxs_options).alt_sh1);
  cb_alt_sh1->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_alt_sh1->setPalette(PALET_NORMAL);
  AUTFBACK(cb_alt_sh1);
  connect(cb_alt_sh1, SIGNAL(clicked()), this, SLOT(set_alt_sh1()));

  cb_alt_sh2 = new QCheckBox(this);
  cb_alt_sh2->setText(us_tr(" SH Alt 2"));
  cb_alt_sh2->setEnabled(true);
  cb_alt_sh2->setChecked((*saxs_options).alt_sh2);
  cb_alt_sh2->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_alt_sh2->setPalette(PALET_NORMAL);
  AUTFBACK(cb_alt_sh2);
  connect(cb_alt_sh2, SIGNAL(clicked()), this, SLOT(set_alt_sh2()));

  cb_create_shd = new QCheckBox(this);
  cb_create_shd->setText(us_tr(" Create SHD file  on compute H3"));
  cb_create_shd->setEnabled(true);
  cb_create_shd->setChecked(
      ((US_Hydrodyn *)us_hydrodyn)->gparams.count("create_shd") &&
      ((US_Hydrodyn *)us_hydrodyn)->gparams["create_shd"] == "1");

  cb_create_shd->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_create_shd->setPalette(PALET_NORMAL);
  AUTFBACK(cb_create_shd);
  connect(cb_create_shd, SIGNAL(clicked()), this, SLOT(set_create_shd()));

  pb_create_somo_ff = new QPushButton(us_tr("Create somo.ff"), this);
  pb_create_somo_ff->setFont(QFont(USglobal->config_list.fontFamily,
                                   USglobal->config_list.fontSize + 1));
  pb_create_somo_ff->setMinimumHeight(minHeight1);
  pb_create_somo_ff->setPalette(PALET_PUSHB);
  connect(pb_create_somo_ff, SIGNAL(clicked()), SLOT(create_somo_ff()));

  pb_cancel = new QPushButton(us_tr("Close"), this);
  pb_cancel->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize + 1));
  pb_cancel->setMinimumHeight(minHeight1);
  pb_cancel->setPalette(PALET_PUSHB);
  connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

  pb_help = new QPushButton(us_tr("Help"), this);
  pb_help->setFont(QFont(USglobal->config_list.fontFamily,
                         USglobal->config_list.fontSize + 1));
  pb_help->setMinimumHeight(minHeight1);
  pb_help->setPalette(PALET_PUSHB);
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));

  int /* rows = 0, columns = 2,*/ spacing = 2, j = 0, margin = 4;
  QGridLayout *background = new QGridLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);
  background->setSpacing(spacing);
  background->setContentsMargins(margin, margin, margin, margin);

  background->addWidget(lbl_info, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;

  QHBoxLayout *hbl_various_1 = new QHBoxLayout;
  hbl_various_1->setContentsMargins(0, 0, 0, 0);
  hbl_various_1->setSpacing(0);
  hbl_various_1->addWidget(cb_iqq_scale_linear_offset);
  background->addLayout(hbl_various_1, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;

  QHBoxLayout *hbl_various_2 = new QHBoxLayout;
  hbl_various_2->setContentsMargins(0, 0, 0, 0);
  hbl_various_2->setSpacing(0);
  hbl_various_2->addWidget(cb_autocorrelate);
  hbl_various_2->addWidget(cb_hybrid_radius_excl_vol);
  hbl_various_2->addWidget(cb_subtract_radius);
  background->addLayout(hbl_various_2, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;

  QHBoxLayout *hbl_various_2c = new QHBoxLayout;
  hbl_various_2c->setContentsMargins(0, 0, 0, 0);
  hbl_various_2c->setSpacing(0);
  hbl_various_2c->addWidget(cb_iqq_use_atomic_ff);
  hbl_various_2c->addWidget(cb_iqq_use_saxs_excl_vol);
  hbl_various_2c->addWidget(cb_use_somo_ff);
  hbl_various_2c->addWidget(cb_iq_exact_q);
  background->addLayout(hbl_various_2c, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;

  QHBoxLayout *hbl_various_3 = new QHBoxLayout;
  hbl_various_3->setContentsMargins(0, 0, 0, 0);
  hbl_various_3->setSpacing(0);

  hbl_various_3->addWidget(cb_iqq_scale_nnls);
  hbl_various_3->addWidget(cb_iqq_log_fitting);
  hbl_various_3->addWidget(cb_iqq_scale_play);
  hbl_various_3->addWidget(cb_alt_ff);
  hbl_various_3->addWidget(cb_five_term_gaussians);
  background->addLayout(hbl_various_3, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;

  QHBoxLayout *hbl_various_4 = new QHBoxLayout;
  hbl_various_4->setContentsMargins(0, 0, 0, 0);
  hbl_various_4->setSpacing(0);

  hbl_various_4->addWidget(cb_compute_exponentials);
  //    hbl_various_4->addWidget( cnt_compute_exponential_terms );
  hbl_various_4->addWidget(cb_multiply_iq_by_atomic_volume);
  background->addLayout(hbl_various_4, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;

  QHBoxLayout *hbl_various_5 = new QHBoxLayout;
  hbl_various_5->setContentsMargins(0, 0, 0, 0);
  hbl_various_5->setSpacing(0);
  hbl_various_5->addWidget(lbl_ev_exp_mult);
  hbl_various_5->addWidget(le_ev_exp_mult);
  background->addLayout(hbl_various_5, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;

  QHBoxLayout *hbl_chi2shannon = new QHBoxLayout;
  hbl_chi2shannon->setContentsMargins(0, 0, 0, 0);
  hbl_chi2shannon->setSpacing(0);
  hbl_chi2shannon->addWidget(cb_compute_chi2shannon);
  hbl_chi2shannon->addWidget(lbl_chi2shannon_dmax);
  hbl_chi2shannon->addWidget(le_chi2shannon_dmax);
  hbl_chi2shannon->addWidget(lbl_chi2shannon_k);
  hbl_chi2shannon->addWidget(le_chi2shannon_k);
  background->addLayout(hbl_chi2shannon, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;

  QHBoxLayout *hbl_various_6 = new QHBoxLayout;
  hbl_various_6->setContentsMargins(0, 0, 0, 0);
  hbl_various_6->setSpacing(0);
  hbl_various_6->addWidget(cb_alt_sh1);
  hbl_various_6->addWidget(cb_alt_sh2);
  hbl_various_6->addWidget(cb_create_shd);
  background->addLayout(hbl_various_6, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;

  background->addWidget(pb_create_somo_ff, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;

  background->addWidget(pb_help, j, 0);
  background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_SasOptionsExperimental::cancel() { close(); }

void US_Hydrodyn_SasOptionsExperimental::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  // online_help->show_help("manual/somo/somo_sas_options_experimental.html");
  online_help->show_help("manual/somo/somo_saxs_options.html");
}

void US_Hydrodyn_SasOptionsExperimental::closeEvent(QCloseEvent *e) {
  *sas_options_experimental_widget = false;
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

void US_Hydrodyn_SasOptionsExperimental::set_iqq_scale_linear_offset() {
  (*saxs_options).iqq_scale_linear_offset =
      cb_iqq_scale_linear_offset->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_autocorrelate() {
  (*saxs_options).autocorrelate = cb_autocorrelate->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_hybrid_radius_excl_vol() {
  (*saxs_options).hybrid_radius_excl_vol =
      cb_hybrid_radius_excl_vol->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_subtract_radius() {
  (*saxs_options).subtract_radius = cb_subtract_radius->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_iqq_use_atomic_ff() {
  (*saxs_options).iqq_use_atomic_ff = cb_iqq_use_atomic_ff->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_iqq_use_saxs_excl_vol() {
  (*saxs_options).iqq_use_saxs_excl_vol = cb_iqq_use_saxs_excl_vol->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_use_somo_ff() {
  (*saxs_options).use_somo_ff = cb_use_somo_ff->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_iqq_scale_nnls() {
  (*saxs_options).iqq_scale_nnls = cb_iqq_scale_nnls->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_iqq_log_fitting() {
  (*saxs_options).iqq_log_fitting = cb_iqq_log_fitting->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_iqq_scale_play() {
  (*saxs_options).iqq_scale_play = cb_iqq_scale_play->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_alt_ff() {
  (*saxs_options).alt_ff = cb_alt_ff->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_five_term_gaussians() {
  (*saxs_options).five_term_gaussians = cb_five_term_gaussians->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_iq_exact_q() {
  (*saxs_options).iq_exact_q = cb_iq_exact_q->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_compute_exponentials() {
  (*saxs_options).compute_exponentials = cb_compute_exponentials->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::update_compute_exponential_terms(
    double val) {
  (*saxs_options).compute_exponential_terms = (unsigned int)val;
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_multiply_iq_by_atomic_volume() {
  (*saxs_options).multiply_iq_by_atomic_volume =
      cb_multiply_iq_by_atomic_volume->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_alt_sh1() {
  (*saxs_options).alt_sh1 = cb_alt_sh1->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_alt_sh2() {
  (*saxs_options).alt_sh2 = cb_alt_sh2->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsExperimental::set_create_shd() {
  ((US_Hydrodyn *)us_hydrodyn)->gparams["create_shd"] =
      cb_create_shd->isChecked() ? "1" : "0";
}

void US_Hydrodyn_SasOptionsExperimental::create_somo_ff() {
  QString errormsg = "";
  if (!((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget) {
    errormsg = "The SAXS window must be open";
  } else {
    if (!((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->create_somo_ff()) {
      errormsg = ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->errormsg;
    }
  }
  if (!errormsg.isEmpty()) {
    QMessageBox::warning(this, "US-SOMO: Create somo.ff", errormsg);
  } else {
    QMessageBox::information(this, "US-SOMO: Create somo.ff",
                             "somo.ff.new created");
  }
}

void US_Hydrodyn_SasOptionsExperimental::update_ev_exp_mult(
    const QString &str) {
  (*saxs_options).ev_exp_mult = str.toDouble();
}

void US_Hydrodyn_SasOptionsExperimental::set_compute_chi2shannon() {
  ((US_Hydrodyn *)us_hydrodyn)->gparams["compute_chi2shannon"] =
      cb_compute_chi2shannon->isChecked() ? "1" : "0";
}

void US_Hydrodyn_SasOptionsExperimental::update_chi2shannon_dmax(
    const QString &str) {
  ((US_Hydrodyn *)us_hydrodyn)->gparams["chi2shannon_dmax"] = str;
}

void US_Hydrodyn_SasOptionsExperimental::update_chi2shannon_k(
    const QString &str) {
  ((US_Hydrodyn *)us_hydrodyn)->gparams["chi2shannon_k"] = str;
}
