#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_asa.h"
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

US_Hydrodyn_SasOptionsSans::US_Hydrodyn_SasOptionsSans(
    struct saxs_options *saxs_options, bool *sas_options_sans_widget,
    void *us_hydrodyn, QWidget *p, const char *)
    : QFrame(p) {
  this->sas_options_sans_widget = sas_options_sans_widget;
  this->saxs_options = saxs_options;
  this->us_hydrodyn = us_hydrodyn;
  *sas_options_sans_widget = true;
  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("US-SOMO SANS Computation Options"));
  setupGUI();
  global_Xpos += 30;
  global_Ypos += 30;
  setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SasOptionsSans::~US_Hydrodyn_SasOptionsSans() {
  *sas_options_sans_widget = false;
}

void US_Hydrodyn_SasOptionsSans::setupGUI() {
  started_in_expert_mode = U_EXPT;

  int minHeight1 = 30;
  lbl_info = new QLabel(us_tr("US-SOMO SANS Computation Options:"), this);
  lbl_info->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_info->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_info->setMinimumHeight(minHeight1);
  lbl_info->setPalette(PALET_FRAME);
  AUTFBACK(lbl_info);
  lbl_info->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize + 1, QFont::Bold));

  lbl_h_scat_len =
      new QLabel(us_tr(" H scattering length (*10^-12 cm): "), this);
  lbl_h_scat_len->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_h_scat_len->setMinimumHeight(minHeight1);
  lbl_h_scat_len->setPalette(PALET_LABEL);
  AUTFBACK(lbl_h_scat_len);
  lbl_h_scat_len->setFont(QFont(USglobal->config_list.fontFamily,
                                USglobal->config_list.fontSize - 1,
                                QFont::Bold));

  cnt_h_scat_len = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_h_scat_len);
  cnt_h_scat_len->setRange(-0.5, 0.5);
  cnt_h_scat_len->setSingleStep(0.0001);
  cnt_h_scat_len->setValue((*saxs_options).h_scat_len);
  cnt_h_scat_len->setMinimumHeight(minHeight1);
  cnt_h_scat_len->setEnabled(true);
  cnt_h_scat_len->setNumButtons(3);
  cnt_h_scat_len->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_h_scat_len->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_h_scat_len);
  connect(cnt_h_scat_len, SIGNAL(valueChanged(double)),
          SLOT(update_h_scat_len(double)));

  lbl_d_scat_len =
      new QLabel(us_tr(" D scattering length (*10^-12 cm): "), this);
  lbl_d_scat_len->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_d_scat_len->setMinimumHeight(minHeight1);
  lbl_d_scat_len->setPalette(PALET_LABEL);
  AUTFBACK(lbl_d_scat_len);
  lbl_d_scat_len->setFont(QFont(USglobal->config_list.fontFamily,
                                USglobal->config_list.fontSize - 1,
                                QFont::Bold));

  cnt_d_scat_len = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_d_scat_len);
  cnt_d_scat_len->setRange(0.1, 1.0);
  cnt_d_scat_len->setSingleStep(0.0001);
  cnt_d_scat_len->setValue((*saxs_options).d_scat_len);
  cnt_d_scat_len->setMinimumHeight(minHeight1);
  cnt_d_scat_len->setEnabled(true);
  cnt_d_scat_len->setNumButtons(3);
  cnt_d_scat_len->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_d_scat_len->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_d_scat_len);
  connect(cnt_d_scat_len, SIGNAL(valueChanged(double)),
          SLOT(update_d_scat_len(double)));

  lbl_h2o_scat_len_dens = new QLabel(
      us_tr(" H2O scattering length density (*10^10 cm^-2): "), this);
  lbl_h2o_scat_len_dens->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_h2o_scat_len_dens->setMinimumHeight(minHeight1);
  lbl_h2o_scat_len_dens->setPalette(PALET_LABEL);
  AUTFBACK(lbl_h2o_scat_len_dens);
  lbl_h2o_scat_len_dens->setFont(QFont(USglobal->config_list.fontFamily,
                                       USglobal->config_list.fontSize - 1,
                                       QFont::Bold));

  cnt_h2o_scat_len_dens = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_h2o_scat_len_dens);
  cnt_h2o_scat_len_dens->setRange(-0.9, 0.0);
  cnt_h2o_scat_len_dens->setSingleStep(0.001);
  cnt_h2o_scat_len_dens->setValue((*saxs_options).h2o_scat_len_dens);
  cnt_h2o_scat_len_dens->setMinimumHeight(minHeight1);
  cnt_h2o_scat_len_dens->setEnabled(true);
  cnt_h2o_scat_len_dens->setNumButtons(3);
  cnt_h2o_scat_len_dens->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_h2o_scat_len_dens->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_h2o_scat_len_dens);
  connect(cnt_h2o_scat_len_dens, SIGNAL(valueChanged(double)),
          SLOT(update_h2o_scat_len_dens(double)));

  lbl_d2o_scat_len_dens = new QLabel(
      us_tr(" D2O scattering length density (*10^10 cm^-2): "), this);
  lbl_d2o_scat_len_dens->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_d2o_scat_len_dens->setMinimumHeight(minHeight1);
  lbl_d2o_scat_len_dens->setPalette(PALET_LABEL);
  AUTFBACK(lbl_d2o_scat_len_dens);
  lbl_d2o_scat_len_dens->setFont(QFont(USglobal->config_list.fontFamily,
                                       USglobal->config_list.fontSize - 1,
                                       QFont::Bold));

  cnt_d2o_scat_len_dens = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_d2o_scat_len_dens);
  cnt_d2o_scat_len_dens->setRange(0.0, 10.0);
  cnt_d2o_scat_len_dens->setSingleStep(0.001);
  cnt_d2o_scat_len_dens->setValue((*saxs_options).d2o_scat_len_dens);
  cnt_d2o_scat_len_dens->setMinimumHeight(minHeight1);
  cnt_d2o_scat_len_dens->setEnabled(true);
  cnt_d2o_scat_len_dens->setNumButtons(3);
  cnt_d2o_scat_len_dens->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_d2o_scat_len_dens->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_d2o_scat_len_dens);
  connect(cnt_d2o_scat_len_dens, SIGNAL(valueChanged(double)),
          SLOT(update_d2o_scat_len_dens(double)));

  lbl_d2o_conc = new QLabel(us_tr(" Buffer D2O fraction (0 - 1): "), this);
  lbl_d2o_conc->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_d2o_conc->setMinimumHeight(minHeight1);
  lbl_d2o_conc->setPalette(PALET_LABEL);
  AUTFBACK(lbl_d2o_conc);
  lbl_d2o_conc->setFont(QFont(USglobal->config_list.fontFamily,
                              USglobal->config_list.fontSize - 1, QFont::Bold));

  cnt_d2o_conc = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_d2o_conc);
  cnt_d2o_conc->setRange(0.0, 1.0);
  cnt_d2o_conc->setSingleStep(0.01);
  cnt_d2o_conc->setValue((*saxs_options).d2o_conc);
  cnt_d2o_conc->setMinimumHeight(minHeight1);
  cnt_d2o_conc->setEnabled(true);
  cnt_d2o_conc->setNumButtons(3);
  cnt_d2o_conc->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_d2o_conc->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_d2o_conc);
  connect(cnt_d2o_conc, SIGNAL(valueChanged(double)),
          SLOT(update_d2o_conc(double)));

  lbl_frac_of_exch_pep =
      new QLabel(us_tr(" Fraction of non-exchanged peptide H (0 - 1): "), this);
  lbl_frac_of_exch_pep->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_frac_of_exch_pep->setMinimumHeight(minHeight1);
  lbl_frac_of_exch_pep->setPalette(PALET_LABEL);
  AUTFBACK(lbl_frac_of_exch_pep);
  lbl_frac_of_exch_pep->setFont(QFont(USglobal->config_list.fontFamily,
                                      USglobal->config_list.fontSize - 1,
                                      QFont::Bold));

  cnt_frac_of_exch_pep = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_frac_of_exch_pep);
  cnt_frac_of_exch_pep->setRange(0.0, 1.0);
  cnt_frac_of_exch_pep->setSingleStep(0.001);
  cnt_frac_of_exch_pep->setValue((*saxs_options).frac_of_exch_pep);
  cnt_frac_of_exch_pep->setMinimumHeight(minHeight1);
  cnt_frac_of_exch_pep->setEnabled(true);
  cnt_frac_of_exch_pep->setNumButtons(3);
  cnt_frac_of_exch_pep->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_frac_of_exch_pep->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_frac_of_exch_pep);
  connect(cnt_frac_of_exch_pep, SIGNAL(valueChanged(double)),
          SLOT(update_frac_of_exch_pep(double)));

  lbl_perdeuteration = new QLabel(us_tr(" Perdeuteration (0 - 1): "), this);
  lbl_perdeuteration->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_perdeuteration->setMinimumHeight(minHeight1);
  lbl_perdeuteration->setPalette(PALET_LABEL);
  AUTFBACK(lbl_perdeuteration);
  lbl_perdeuteration->setFont(QFont(USglobal->config_list.fontFamily,
                                    USglobal->config_list.fontSize - 1,
                                    QFont::Bold));

  cnt_perdeuteration = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_perdeuteration);
  cnt_perdeuteration->setRange(0.0, 1.0);
  cnt_perdeuteration->setSingleStep(0.001);
  cnt_perdeuteration->setValue(
      ((US_Hydrodyn *)us_hydrodyn)->gparams.count("perdeuteration")
          ? ((US_Hydrodyn *)us_hydrodyn)->gparams["perdeuteration"].toDouble()
          : 0e0);
  cnt_perdeuteration->setMinimumHeight(minHeight1);
  cnt_perdeuteration->setEnabled(true);
  cnt_perdeuteration->setNumButtons(3);
  cnt_perdeuteration->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_perdeuteration->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_perdeuteration);
  connect(cnt_perdeuteration, SIGNAL(valueChanged(double)),
          SLOT(update_perdeuteration(double)));

  lbl_sans_iq = new QLabel(us_tr(" I(q) method: "), this);
  lbl_sans_iq->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_sans_iq->setMinimumHeight(minHeight1);
  lbl_sans_iq->setPalette(PALET_LABEL);
  AUTFBACK(lbl_sans_iq);
  lbl_sans_iq->setFont(QFont(USglobal->config_list.fontFamily,
                             USglobal->config_list.fontSize - 1, QFont::Bold));

  cb_sans_iq_native_debye = new QCheckBox(this);
  cb_sans_iq_native_debye->setText(us_tr("F-DB "));
  cb_sans_iq_native_debye->setEnabled(true);
  cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
  cb_sans_iq_native_debye->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_sans_iq_native_debye->setPalette(PALET_NORMAL);
  AUTFBACK(cb_sans_iq_native_debye);
  connect(cb_sans_iq_native_debye, SIGNAL(clicked()), this,
          SLOT(set_sans_iq_native_debye()));

  if (started_in_expert_mode) {
    cb_sans_iq_native_hybrid = new QCheckBox(this);
    cb_sans_iq_native_hybrid->setText(us_tr("Hybrid "));
    cb_sans_iq_native_hybrid->setEnabled(true);
    cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
    cb_sans_iq_native_hybrid->setFont(QFont(USglobal->config_list.fontFamily,
                                            USglobal->config_list.fontSize));
    cb_sans_iq_native_hybrid->setPalette(PALET_NORMAL);
    AUTFBACK(cb_sans_iq_native_hybrid);
    connect(cb_sans_iq_native_hybrid, SIGNAL(clicked()), this,
            SLOT(set_sans_iq_native_hybrid()));

    cb_sans_iq_native_hybrid2 = new QCheckBox(this);
    cb_sans_iq_native_hybrid2->setText(us_tr("H2 "));
    cb_sans_iq_native_hybrid2->setEnabled(true);
    cb_sans_iq_native_hybrid2->setChecked(
        (*saxs_options).sans_iq_native_hybrid2);
    cb_sans_iq_native_hybrid2->setFont(QFont(USglobal->config_list.fontFamily,
                                             USglobal->config_list.fontSize));
    cb_sans_iq_native_hybrid2->setPalette(PALET_NORMAL);
    AUTFBACK(cb_sans_iq_native_hybrid2);
    connect(cb_sans_iq_native_hybrid2, SIGNAL(clicked()), this,
            SLOT(set_sans_iq_native_hybrid2()));

    cb_sans_iq_native_hybrid3 = new QCheckBox(this);
    cb_sans_iq_native_hybrid3->setText(us_tr("H3 "));
    cb_sans_iq_native_hybrid3->setEnabled(true);
    cb_sans_iq_native_hybrid3->setChecked(
        (*saxs_options).sans_iq_native_hybrid3);
    cb_sans_iq_native_hybrid3->setFont(QFont(USglobal->config_list.fontFamily,
                                             USglobal->config_list.fontSize));
    cb_sans_iq_native_hybrid3->setPalette(PALET_NORMAL);
    AUTFBACK(cb_sans_iq_native_hybrid3);
    connect(cb_sans_iq_native_hybrid3, SIGNAL(clicked()), this,
            SLOT(set_sans_iq_native_hybrid3()));

    cb_sans_iq_hybrid_adaptive = new QCheckBox(this);
    cb_sans_iq_hybrid_adaptive->setText(us_tr("Adaptive "));
    cb_sans_iq_hybrid_adaptive->setEnabled(true);
    cb_sans_iq_hybrid_adaptive->setChecked(
        (*saxs_options).sans_iq_hybrid_adaptive);
    cb_sans_iq_hybrid_adaptive->setFont(QFont(USglobal->config_list.fontFamily,
                                              USglobal->config_list.fontSize));
    cb_sans_iq_hybrid_adaptive->setPalette(PALET_NORMAL);
    AUTFBACK(cb_sans_iq_hybrid_adaptive);
    connect(cb_sans_iq_hybrid_adaptive, SIGNAL(clicked()), this,
            SLOT(set_sans_iq_hybrid_adaptive()));
  }

  cb_sans_iq_native_fast = new QCheckBox(this);
  cb_sans_iq_native_fast->setText(us_tr("Q-DB "));
  cb_sans_iq_native_fast->setEnabled(true);
  cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
  cb_sans_iq_native_fast->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_sans_iq_native_fast->setPalette(PALET_NORMAL);
  AUTFBACK(cb_sans_iq_native_fast);
  connect(cb_sans_iq_native_fast, SIGNAL(clicked()), this,
          SLOT(set_sans_iq_native_fast()));

  cb_sans_iq_native_fast_compute_pr = new QCheckBox(this);
  cb_sans_iq_native_fast_compute_pr->setText(us_tr("P(r) "));
  cb_sans_iq_native_fast_compute_pr->setEnabled(true);
  cb_sans_iq_native_fast_compute_pr->setChecked(
      (*saxs_options).sans_iq_native_fast_compute_pr);
  cb_sans_iq_native_fast_compute_pr->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_sans_iq_native_fast_compute_pr->setPalette(PALET_NORMAL);
  AUTFBACK(cb_sans_iq_native_fast_compute_pr);
  connect(cb_sans_iq_native_fast_compute_pr, SIGNAL(clicked()), this,
          SLOT(set_sans_iq_native_fast_compute_pr()));

  cb_sans_iq_cryson = new QCheckBox(this);
  cb_sans_iq_cryson->setText(us_tr("Cryson"));
  cb_sans_iq_cryson->setEnabled(true);
  cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);
  cb_sans_iq_cryson->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_sans_iq_cryson->setPalette(PALET_NORMAL);
  AUTFBACK(cb_sans_iq_cryson);
  connect(cb_sans_iq_cryson, SIGNAL(clicked()), this,
          SLOT(set_sans_iq_cryson()));

  lbl_cryson_sh_max_harmonics = new QLabel(
      started_in_expert_mode ? us_tr(" SH/Cryson: Maximum order of harmonics")
                             : us_tr(" SH/Cryson: Maximum order of harmonics"),
      this);
  lbl_cryson_sh_max_harmonics->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_cryson_sh_max_harmonics->setMinimumHeight(minHeight1);
  lbl_cryson_sh_max_harmonics->setPalette(PALET_LABEL);
  AUTFBACK(lbl_cryson_sh_max_harmonics);
  lbl_cryson_sh_max_harmonics->setFont(QFont(USglobal->config_list.fontFamily,
                                             USglobal->config_list.fontSize - 1,
                                             QFont::Bold));

  cnt_cryson_sh_max_harmonics = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_cryson_sh_max_harmonics);
  cnt_cryson_sh_max_harmonics->setRange(1, 75);
  cnt_cryson_sh_max_harmonics->setSingleStep(1);
  cnt_cryson_sh_max_harmonics->setValue((*saxs_options).sh_max_harmonics);
  cnt_cryson_sh_max_harmonics->setMinimumHeight(minHeight1);
  cnt_cryson_sh_max_harmonics->setEnabled(true);
  cnt_cryson_sh_max_harmonics->setNumButtons(2);
  cnt_cryson_sh_max_harmonics->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_cryson_sh_max_harmonics->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_cryson_sh_max_harmonics);
  connect(cnt_cryson_sh_max_harmonics, SIGNAL(valueChanged(double)),
          SLOT(update_cryson_sh_max_harmonics(double)));

  lbl_cryson_sh_fibonacci_grid_order = new QLabel(
      started_in_expert_mode ? us_tr(" Cryson: Order of Fibonacci grid")
                             : us_tr(" Cryson: Order of Fibonacci grid"),
      this);
  lbl_cryson_sh_fibonacci_grid_order->setAlignment(Qt::AlignLeft |
                                                   Qt::AlignVCenter);
  lbl_cryson_sh_fibonacci_grid_order->setMinimumHeight(minHeight1);
  lbl_cryson_sh_fibonacci_grid_order->setPalette(PALET_LABEL);
  AUTFBACK(lbl_cryson_sh_fibonacci_grid_order);
  lbl_cryson_sh_fibonacci_grid_order->setFont(
      QFont(USglobal->config_list.fontFamily,
            USglobal->config_list.fontSize - 1, QFont::Bold));

  cnt_cryson_sh_fibonacci_grid_order = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_cryson_sh_fibonacci_grid_order);
  cnt_cryson_sh_fibonacci_grid_order->setRange(2, 75);
  cnt_cryson_sh_fibonacci_grid_order->setSingleStep(1);
  cnt_cryson_sh_fibonacci_grid_order->setValue(
      (*saxs_options).sh_fibonacci_grid_order);
  cnt_cryson_sh_fibonacci_grid_order->setMinimumHeight(minHeight1);
  cnt_cryson_sh_fibonacci_grid_order->setEnabled(true);
  cnt_cryson_sh_fibonacci_grid_order->setNumButtons(2);
  cnt_cryson_sh_fibonacci_grid_order->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_cryson_sh_fibonacci_grid_order->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_cryson_sh_fibonacci_grid_order);
  connect(cnt_cryson_sh_fibonacci_grid_order, SIGNAL(valueChanged(double)),
          SLOT(update_cryson_sh_fibonacci_grid_order(double)));

  cb_cryson_manual_hs = new QCheckBox(this);
  cb_cryson_manual_hs->setText(
      us_tr(" Cryson: Contrast of hydration shell (*10^10 cm^-2):"));
  cb_cryson_manual_hs->setEnabled(true);
  cb_cryson_manual_hs->setChecked((*saxs_options).cryson_manual_hs);
  cb_cryson_manual_hs->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_cryson_manual_hs->setPalette(PALET_NORMAL);
  AUTFBACK(cb_cryson_manual_hs);
  connect(cb_cryson_manual_hs, SIGNAL(clicked()), this,
          SLOT(set_cryson_manual_hs()));

  //    lbl_cryson_hydration_shell_contrast = new QLabel( started_in_expert_mode
  //    ?

  //                                                      :
  //                                                      us_tr(" Cryson:
  //                                                      Contrast of hydration
  //                                                      shell (*10^10
  //                                                      cm^-2):") , this);
  //    lbl_cryson_hydration_shell_contrast->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  //    lbl_cryson_hydration_shell_contrast->setMinimumHeight(minHeight1);
  //    lbl_cryson_hydration_shell_contrast->setPalette( PALET_LABEL );
  //    lbl_cryson_hydration_shell_contrast->setFont(QFont(
  //    USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1,
  //    QFont::Bold));

  cnt_cryson_hydration_shell_contrast = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_cryson_hydration_shell_contrast);
  cnt_cryson_hydration_shell_contrast->setRange(-1, 8);
  cnt_cryson_hydration_shell_contrast->setSingleStep(0.0001);
  cnt_cryson_hydration_shell_contrast->setValue(
      (*saxs_options).cryson_hydration_shell_contrast);
  cnt_cryson_hydration_shell_contrast->setMinimumHeight(minHeight1);
  cnt_cryson_hydration_shell_contrast->setEnabled(true);
  cnt_cryson_hydration_shell_contrast->setNumButtons(3);
  cnt_cryson_hydration_shell_contrast->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_cryson_hydration_shell_contrast->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_cryson_hydration_shell_contrast);
  connect(cnt_cryson_hydration_shell_contrast, SIGNAL(valueChanged(double)),
          SLOT(update_cryson_hydration_shell_contrast(double)));

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

  int /* rows=0, columns = 2,*/ spacing = 2, j = 0, margin = 4;
  QGridLayout *background = new QGridLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);
  background->setSpacing(spacing);
  background->setContentsMargins(margin, margin, margin, margin);

  background->addWidget(lbl_info, j, 0, 1 + (j) - (j), 1 + (1) - (0));
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
  background->addWidget(lbl_perdeuteration, j, 0);
  background->addWidget(cnt_perdeuteration, j, 1);
  j++;

  QHBoxLayout *hbl_sans_iq = new QHBoxLayout;
  hbl_sans_iq->setContentsMargins(0, 0, 0, 0);
  hbl_sans_iq->setSpacing(0);
  hbl_sans_iq->addWidget(lbl_sans_iq);
  hbl_sans_iq->addWidget(cb_sans_iq_native_debye);
  if (started_in_expert_mode) {
    hbl_sans_iq->addWidget(cb_sans_iq_native_hybrid);
    hbl_sans_iq->addWidget(cb_sans_iq_native_hybrid2);
    hbl_sans_iq->addWidget(cb_sans_iq_native_hybrid3);
    hbl_sans_iq->addWidget(cb_sans_iq_hybrid_adaptive);
  }
  hbl_sans_iq->addWidget(cb_sans_iq_native_fast);
  hbl_sans_iq->addWidget(cb_sans_iq_native_fast_compute_pr);
  hbl_sans_iq->addWidget(cb_sans_iq_cryson);
  background->addLayout(hbl_sans_iq, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;

  background->addWidget(lbl_cryson_sh_max_harmonics, j, 0);
  background->addWidget(cnt_cryson_sh_max_harmonics, j, 1);
  j++;

  background->addWidget(lbl_cryson_sh_fibonacci_grid_order, j, 0);
  background->addWidget(cnt_cryson_sh_fibonacci_grid_order, j, 1);
  j++;

  background->addWidget(cb_cryson_manual_hs, j, 0);
  background->addWidget(cnt_cryson_hydration_shell_contrast, j, 1);
  j++;

  background->addWidget(pb_help, j, 0);
  background->addWidget(pb_cancel, j, 1);
  //    if ( !started_in_expert_mode )
  //    {
  //       cb_sans_iq_native_hybrid->hide();
  //       cb_sans_iq_native_hybrid2->hide();
  //       cb_sans_iq_native_hybrid3->hide();
  //       cb_sans_iq_hybrid_adaptive->hide();
  //    }

  update_enables();
  if (!cb_cryson_manual_hs->isChecked()) {
    recompute_contrast();
  }
}

void US_Hydrodyn_SasOptionsSans::cancel() { close(); }

void US_Hydrodyn_SasOptionsSans::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  // online_help->show_help("manual/somo/somo_sas_options_sans.html");
  online_help->show_help("manual/somo/somo_saxs_options.html");
}

void US_Hydrodyn_SasOptionsSans::closeEvent(QCloseEvent *e) {
  *sas_options_sans_widget = false;
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

void US_Hydrodyn_SasOptionsSans::update_h_scat_len(double val) {
  (*saxs_options).h_scat_len = (float)val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSans::update_d_scat_len(double val) {
  (*saxs_options).d_scat_len = (float)val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSans::update_h2o_scat_len_dens(double val) {
  (*saxs_options).h2o_scat_len_dens = (float)val;
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  recompute_contrast();
}

void US_Hydrodyn_SasOptionsSans::update_d2o_scat_len_dens(double val) {
  (*saxs_options).d2o_scat_len_dens = (float)val;
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  recompute_contrast();
}

void US_Hydrodyn_SasOptionsSans::update_d2o_conc(double val) {
  (*saxs_options).d2o_conc = (float)val;
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  recompute_contrast();
}

void US_Hydrodyn_SasOptionsSans::update_frac_of_exch_pep(double val) {
  (*saxs_options).frac_of_exch_pep = (float)val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSans::update_perdeuteration(double val) {
  ((US_Hydrodyn *)us_hydrodyn)->gparams["perdeuteration"] =
      QString("%1").arg(val);
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSans::set_sans_iq_native_debye() {
  (*saxs_options).sans_iq_native_debye = cb_sans_iq_native_debye->isChecked();
  (*saxs_options).sans_iq_native_fast = !cb_sans_iq_native_debye->isChecked();
  (*saxs_options).sans_iq_native_hybrid = false;
  (*saxs_options).sans_iq_native_hybrid2 = false;
  (*saxs_options).sans_iq_native_hybrid3 = false;
  (*saxs_options).sans_iq_cryson = false;

  cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
  if (started_in_expert_mode) {
    cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
    cb_sans_iq_native_hybrid2->setChecked(
        (*saxs_options).sans_iq_native_hybrid2);
    cb_sans_iq_native_hybrid3->setChecked(
        (*saxs_options).sans_iq_native_hybrid3);
  }
  cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
  cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);

  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  if (((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget) {
    ((US_Hydrodyn *)us_hydrodyn)
        ->saxs_plot_window->set_current_method_buttons();
  }
}

void US_Hydrodyn_SasOptionsSans::set_sans_iq_native_fast_compute_pr() {
  (*saxs_options).sans_iq_native_fast_compute_pr =
      cb_sans_iq_native_fast_compute_pr->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsSans::set_sans_iq_native_hybrid() {
  (*saxs_options).sans_iq_native_hybrid = cb_sans_iq_native_hybrid->isChecked();
  (*saxs_options).sans_iq_native_debye = !cb_sans_iq_native_hybrid->isChecked();
  (*saxs_options).sans_iq_native_hybrid2 = false;
  (*saxs_options).sans_iq_native_hybrid3 = false;
  (*saxs_options).sans_iq_native_fast = false;
  (*saxs_options).sans_iq_cryson = false;

  cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
  if (started_in_expert_mode) {
    cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
    cb_sans_iq_native_hybrid2->setChecked(
        (*saxs_options).sans_iq_native_hybrid2);
    cb_sans_iq_native_hybrid3->setChecked(
        (*saxs_options).sans_iq_native_hybrid3);
  }
  cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
  cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);

  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  if (((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget) {
    ((US_Hydrodyn *)us_hydrodyn)
        ->saxs_plot_window->set_current_method_buttons();
  }
}

void US_Hydrodyn_SasOptionsSans::set_sans_iq_native_hybrid2() {
  (*saxs_options).sans_iq_native_hybrid2 =
      cb_sans_iq_native_hybrid2->isChecked();
  (*saxs_options).sans_iq_native_debye =
      !cb_sans_iq_native_hybrid2->isChecked();
  (*saxs_options).sans_iq_native_hybrid = false;
  (*saxs_options).sans_iq_native_hybrid3 = false;
  (*saxs_options).sans_iq_native_fast = false;
  (*saxs_options).sans_iq_cryson = false;

  cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
  if (started_in_expert_mode) {
    cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
    cb_sans_iq_native_hybrid2->setChecked(
        (*saxs_options).sans_iq_native_hybrid2);
    cb_sans_iq_native_hybrid3->setChecked(
        (*saxs_options).sans_iq_native_hybrid3);
  }
  cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
  cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);

  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  if (((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget) {
    ((US_Hydrodyn *)us_hydrodyn)
        ->saxs_plot_window->set_current_method_buttons();
  }
}

void US_Hydrodyn_SasOptionsSans::set_sans_iq_native_hybrid3() {
  (*saxs_options).sans_iq_native_hybrid3 =
      cb_sans_iq_native_hybrid3->isChecked();
  (*saxs_options).sans_iq_native_debye =
      !cb_sans_iq_native_hybrid3->isChecked();
  (*saxs_options).sans_iq_native_hybrid = false;
  (*saxs_options).sans_iq_native_hybrid2 = false;
  (*saxs_options).sans_iq_native_fast = false;
  (*saxs_options).sans_iq_cryson = false;

  cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
  if (started_in_expert_mode) {
    cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
    cb_sans_iq_native_hybrid2->setChecked(
        (*saxs_options).sans_iq_native_hybrid2);
    cb_sans_iq_native_hybrid3->setChecked(
        (*saxs_options).sans_iq_native_hybrid3);
  }
  cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
  cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);

  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  if (((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget) {
    ((US_Hydrodyn *)us_hydrodyn)
        ->saxs_plot_window->set_current_method_buttons();
  }
}

void US_Hydrodyn_SasOptionsSans::set_sans_iq_hybrid_adaptive() {
  (*saxs_options).sans_iq_hybrid_adaptive =
      cb_sans_iq_hybrid_adaptive->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  if (((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget) {
    ((US_Hydrodyn *)us_hydrodyn)
        ->saxs_plot_window->set_current_method_buttons();
  }
}

void US_Hydrodyn_SasOptionsSans::set_sans_iq_native_fast() {
  (*saxs_options).sans_iq_native_fast = cb_sans_iq_native_fast->isChecked();
  (*saxs_options).sans_iq_native_debye = !cb_sans_iq_native_fast->isChecked();
  (*saxs_options).sans_iq_native_hybrid = false;
  (*saxs_options).sans_iq_native_hybrid2 = false;
  (*saxs_options).sans_iq_native_hybrid3 = false;
  (*saxs_options).sans_iq_cryson = false;

  cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
  if (started_in_expert_mode) {
    cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
    cb_sans_iq_native_hybrid2->setChecked(
        (*saxs_options).sans_iq_native_hybrid2);
    cb_sans_iq_native_hybrid3->setChecked(
        (*saxs_options).sans_iq_native_hybrid3);
  }
  cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
  cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);

  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  if (((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget) {
    ((US_Hydrodyn *)us_hydrodyn)
        ->saxs_plot_window->set_current_method_buttons();
  }
}

void US_Hydrodyn_SasOptionsSans::set_sans_iq_cryson() {
  (*saxs_options).sans_iq_cryson = cb_sans_iq_cryson->isChecked();
  (*saxs_options).sans_iq_native_debye = !cb_sans_iq_cryson->isChecked();
  (*saxs_options).sans_iq_native_hybrid = false;
  (*saxs_options).sans_iq_native_hybrid2 = false;
  (*saxs_options).sans_iq_native_hybrid3 = false;
  (*saxs_options).sans_iq_native_fast = false;

  cb_sans_iq_native_debye->setChecked((*saxs_options).sans_iq_native_debye);
  if (started_in_expert_mode) {
    cb_sans_iq_native_hybrid->setChecked((*saxs_options).sans_iq_native_hybrid);
    cb_sans_iq_native_hybrid2->setChecked(
        (*saxs_options).sans_iq_native_hybrid2);
    cb_sans_iq_native_hybrid3->setChecked(
        (*saxs_options).sans_iq_native_hybrid3);
  }
  cb_sans_iq_native_fast->setChecked((*saxs_options).sans_iq_native_fast);
  cb_sans_iq_cryson->setChecked((*saxs_options).sans_iq_cryson);

  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  if (((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget) {
    ((US_Hydrodyn *)us_hydrodyn)
        ->saxs_plot_window->set_current_method_buttons();
  }
}

void US_Hydrodyn_SasOptionsSans::update_cryson_sh_max_harmonics(double val) {
  (*saxs_options).cryson_sh_max_harmonics = (unsigned int)val;
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  if (((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget) {
    ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->update_iqq_suffix();
  }
}

void US_Hydrodyn_SasOptionsSans::update_cryson_sh_fibonacci_grid_order(
    double val) {
  (*saxs_options).cryson_sh_fibonacci_grid_order = (unsigned int)val;
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  if (((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget) {
    ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->update_iqq_suffix();
  }
}

void US_Hydrodyn_SasOptionsSans::update_cryson_hydration_shell_contrast(
    double val) {
  (*saxs_options).cryson_hydration_shell_contrast = (float)val;
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  if (((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget) {
    ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->update_iqq_suffix();
  }
}

void US_Hydrodyn_SasOptionsSans::set_cryson_manual_hs() {
  (*saxs_options).cryson_manual_hs = cb_cryson_manual_hs->isChecked();
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  update_enables();
}

void US_Hydrodyn_SasOptionsSans::recompute_contrast() {
  cnt_cryson_hydration_shell_contrast->setValue(
      (*saxs_options).d2o_conc * (*saxs_options).d2o_scat_len_dens +
      (1e0 - (*saxs_options).d2o_conc) * (*saxs_options).h2o_scat_len_dens);
}

void US_Hydrodyn_SasOptionsSans::update_enables() {
  cnt_cryson_hydration_shell_contrast->setEnabled(
      cb_cryson_manual_hs->isChecked());
}
