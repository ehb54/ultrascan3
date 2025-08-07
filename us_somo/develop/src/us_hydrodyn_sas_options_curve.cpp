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

US_Hydrodyn_SasOptionsCurve::US_Hydrodyn_SasOptionsCurve(
    struct saxs_options *saxs_options, bool *sas_options_curve_widget,
    void *us_hydrodyn, QWidget *p, const char *)
    : QFrame(p) {
  this->sas_options_curve_widget = sas_options_curve_widget;
  this->saxs_options = saxs_options;
  this->us_hydrodyn = us_hydrodyn;
  *sas_options_curve_widget = true;
  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("US-SOMO SAS Curve Generation Options"));
  update_q();
  setupGUI();
  global_Xpos += 30;
  global_Ypos += 30;
  setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SasOptionsCurve::~US_Hydrodyn_SasOptionsCurve() {
  *sas_options_curve_widget = false;
}

void US_Hydrodyn_SasOptionsCurve::setupGUI() {
  int minHeight1 = 30;
  lbl_info = new QLabel(us_tr("US-SOMO SAS Curve Generation Options:"), this);
  lbl_info->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_info->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_info->setMinimumHeight(minHeight1);
  lbl_info->setPalette(PALET_FRAME);
  AUTFBACK(lbl_info);
  lbl_info->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize + 1, QFont::Bold));

  lbl_wavelength = new QLabel(us_tr(" Wavelength (Angstrom): "), this);
  lbl_wavelength->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_wavelength->setMinimumHeight(minHeight1);
  lbl_wavelength->setPalette(PALET_LABEL);
  AUTFBACK(lbl_wavelength);
  lbl_wavelength->setFont(QFont(USglobal->config_list.fontFamily,
                                USglobal->config_list.fontSize - 1,
                                QFont::Bold));

  cnt_wavelength = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_wavelength);
  cnt_wavelength->setRange(0, 10);
  cnt_wavelength->setSingleStep(0.01);
  cnt_wavelength->setValue((*saxs_options).wavelength);
  cnt_wavelength->setMinimumHeight(minHeight1);
  cnt_wavelength->setEnabled(true);
  cnt_wavelength->setNumButtons(3);
  cnt_wavelength->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_wavelength->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_wavelength);
  connect(cnt_wavelength, SIGNAL(valueChanged(double)),
          SLOT(update_wavelength(double)));

  lbl_start_angle = new QLabel(us_tr(" Starting Angle: "), this);
  lbl_start_angle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_start_angle->setMinimumHeight(minHeight1);
  lbl_start_angle->setPalette(PALET_LABEL);
  AUTFBACK(lbl_start_angle);
  lbl_start_angle->setFont(QFont(USglobal->config_list.fontFamily,
                                 USglobal->config_list.fontSize - 1,
                                 QFont::Bold));

  cnt_start_angle = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_start_angle);
  cnt_start_angle->setRange(0, 90);
  cnt_start_angle->setSingleStep(1.0f / SAXS_Q_ROUNDING);
  cnt_start_angle->setValue((*saxs_options).start_angle);
  cnt_start_angle->setMinimumHeight(minHeight1);
  cnt_start_angle->setEnabled(true);
  cnt_start_angle->setNumButtons(SAXS_Q_BUTTONS);
  cnt_start_angle->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_start_angle->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_start_angle);
  connect(cnt_start_angle, SIGNAL(valueChanged(double)),
          SLOT(update_start_angle(double)));

  lbl_end_angle = new QLabel(us_tr(" Ending Angle: "), this);
  lbl_end_angle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_end_angle->setMinimumHeight(minHeight1);
  lbl_end_angle->setPalette(PALET_LABEL);
  AUTFBACK(lbl_end_angle);
  lbl_end_angle->setFont(QFont(USglobal->config_list.fontFamily,
                               USglobal->config_list.fontSize - 1,
                               QFont::Bold));

  cnt_end_angle = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_end_angle);
  cnt_end_angle->setRange(0, 90);
  cnt_end_angle->setSingleStep(1.0f / SAXS_Q_ROUNDING);
  cnt_end_angle->setValue((*saxs_options).end_angle);
  cnt_end_angle->setMinimumHeight(minHeight1);
  cnt_end_angle->setEnabled(true);
  cnt_end_angle->setNumButtons(SAXS_Q_BUTTONS);
  cnt_end_angle->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_end_angle->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_end_angle);
  connect(cnt_end_angle, SIGNAL(valueChanged(double)),
          SLOT(update_end_angle(double)));

  lbl_delta_angle = new QLabel(us_tr(" Angle Stepsize: "), this);
  lbl_delta_angle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_delta_angle->setMinimumHeight(minHeight1);
  lbl_delta_angle->setPalette(PALET_LABEL);
  AUTFBACK(lbl_delta_angle);
  lbl_delta_angle->setFont(QFont(USglobal->config_list.fontFamily,
                                 USglobal->config_list.fontSize - 1,
                                 QFont::Bold));

  cnt_delta_angle = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_delta_angle);
  cnt_delta_angle->setRange(0.0001, 90);
  cnt_delta_angle->setSingleStep(1.0f / SAXS_Q_ROUNDING);
  cnt_delta_angle->setValue((*saxs_options).delta_angle);
  cnt_delta_angle->setMinimumHeight(minHeight1);
  cnt_delta_angle->setEnabled(true);
  cnt_delta_angle->setNumButtons(SAXS_Q_BUTTONS);
  cnt_delta_angle->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_delta_angle->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_delta_angle);
  connect(cnt_delta_angle, SIGNAL(valueChanged(double)),
          SLOT(update_delta_angle(double)));

  lbl_start_q = new QLabel(us_tr(" Starting q: "), this);
  lbl_start_q->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_start_q->setMinimumHeight(minHeight1);
  lbl_start_q->setPalette(PALET_LABEL);
  AUTFBACK(lbl_start_q);
  lbl_start_q->setFont(QFont(USglobal->config_list.fontFamily,
                             USglobal->config_list.fontSize - 1, QFont::Bold));

  cnt_start_q = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_start_q);
  cnt_start_q->setRange(0, 90);
  cnt_start_q->setSingleStep(1.0f / SAXS_Q_ROUNDING);
  cnt_start_q->setValue((*saxs_options).start_q);
  cnt_start_q->setMinimumHeight(minHeight1);
  cnt_start_q->setEnabled(true);
  cnt_start_q->setNumButtons(SAXS_Q_BUTTONS);
  cnt_start_q->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_start_q->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_start_q);
  connect(cnt_start_q, SIGNAL(valueChanged(double)),
          SLOT(update_start_q(double)));

  lbl_end_q = new QLabel(us_tr(" Ending q: "), this);
  lbl_end_q->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_end_q->setMinimumHeight(minHeight1);
  lbl_end_q->setPalette(PALET_LABEL);
  AUTFBACK(lbl_end_q);
  lbl_end_q->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize - 1, QFont::Bold));

  cnt_end_q = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_end_q);
  cnt_end_q->setRange(0, 90);
  cnt_end_q->setSingleStep(1.0f / SAXS_Q_ROUNDING);
  cnt_end_q->setValue((*saxs_options).end_q);
  cnt_end_q->setMinimumHeight(minHeight1);
  cnt_end_q->setEnabled(true);
  cnt_end_q->setNumButtons(SAXS_Q_BUTTONS);
  cnt_end_q->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_end_q->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_end_q);
  connect(cnt_end_q, SIGNAL(valueChanged(double)), SLOT(update_end_q(double)));

  lbl_delta_q = new QLabel(us_tr(" q Stepsize: "), this);
  lbl_delta_q->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_delta_q->setMinimumHeight(minHeight1);
  lbl_delta_q->setPalette(PALET_LABEL);
  AUTFBACK(lbl_delta_q);
  lbl_delta_q->setFont(QFont(USglobal->config_list.fontFamily,
                             USglobal->config_list.fontSize - 1, QFont::Bold));

  cnt_delta_q = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_delta_q);
  cnt_delta_q->setRange(0.0001, 90);
  cnt_delta_q->setSingleStep(1.0f / SAXS_Q_ROUNDING);
  cnt_delta_q->setValue((*saxs_options).delta_q);
  cnt_delta_q->setMinimumHeight(minHeight1);
  cnt_delta_q->setEnabled(true);
  cnt_delta_q->setNumButtons(SAXS_Q_BUTTONS);
  cnt_delta_q->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_delta_q->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_delta_q);
  connect(cnt_delta_q, SIGNAL(valueChanged(double)),
          SLOT(update_delta_q(double)));

  cb_normalize_by_mw = new QCheckBox(this);
  cb_normalize_by_mw->setText(
      us_tr(" Normalize P(r) vs r curve by molecular weight"));
  cb_normalize_by_mw->setEnabled(true);
  cb_normalize_by_mw->setChecked((*saxs_options).normalize_by_mw);
  cb_normalize_by_mw->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_normalize_by_mw->setPalette(PALET_NORMAL);
  AUTFBACK(cb_normalize_by_mw);
  connect(cb_normalize_by_mw, SIGNAL(clicked()), this,
          SLOT(set_normalize_by_mw()));

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

  // column grouping 2

  background->addWidget(lbl_wavelength, j, 0);
  background->addWidget(cnt_wavelength, j, 1);
  j++;
  background->addWidget(lbl_start_angle, j, 0);
  background->addWidget(cnt_start_angle, j, 1);
  j++;
  background->addWidget(lbl_end_angle, j, 0);
  background->addWidget(cnt_end_angle, j, 1);
  j++;
  background->addWidget(lbl_delta_angle, j, 0);
  background->addWidget(cnt_delta_angle, j, 1);
  j++;
  background->addWidget(lbl_start_q, j, 0);
  background->addWidget(cnt_start_q, j, 1);
  j++;
  background->addWidget(lbl_end_q, j, 0);
  background->addWidget(cnt_end_q, j, 1);
  j++;
  background->addWidget(lbl_delta_q, j, 0);
  background->addWidget(cnt_delta_q, j, 1);
  j++;

  QHBoxLayout *hbl_curve_opts = new QHBoxLayout;
  hbl_curve_opts->setContentsMargins(0, 0, 0, 0);
  hbl_curve_opts->setSpacing(0);
  hbl_curve_opts->addWidget(cb_normalize_by_mw);
  background->addLayout(hbl_curve_opts, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;

  background->addWidget(pb_help, j, 0);
  background->addWidget(pb_cancel, j, 1);

  setMinimumWidth(400);
}

void US_Hydrodyn_SasOptionsCurve::cancel() { close(); }

void US_Hydrodyn_SasOptionsCurve::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  // online_help->show_help("manual/somo/somo_sas_options_curve.html");
  online_help->show_help("manual/somo/somo_saxs_options.html");
}

void US_Hydrodyn_SasOptionsCurve::closeEvent(QCloseEvent *e) {
  *sas_options_curve_widget = false;
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

void US_Hydrodyn_SasOptionsCurve::update_wavelength(double val) {
  (*saxs_options).wavelength = (float)val;
  update_q();
  cnt_start_q->setValue(saxs_options->start_q);
  cnt_end_q->setValue(saxs_options->end_q);
  cnt_delta_q->setValue(saxs_options->delta_q);
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsCurve::update_start_angle(double val) {
  (*saxs_options).start_angle = (float)val;
  if (saxs_options->start_angle > saxs_options->end_angle) {
    saxs_options->start_angle = saxs_options->end_angle;
    cnt_start_angle->setValue(saxs_options->end_angle);
  }
  update_q();
  cnt_start_q->setValue(saxs_options->start_q);
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsCurve::update_end_angle(double val) {
  (*saxs_options).end_angle = (float)val;
  if (saxs_options->start_angle > saxs_options->end_angle) {
    saxs_options->end_angle = saxs_options->start_angle;
    cnt_end_angle->setValue(saxs_options->start_angle);
  }
  update_q();
  cnt_end_q->setValue(saxs_options->end_q);
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsCurve::update_delta_angle(double val) {
  (*saxs_options).delta_angle = (float)val;
  update_q();
  cnt_delta_q->setValue(saxs_options->delta_q);
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsCurve::update_q() {
  // note changes to this section should be updated in
  // US_Hydrodyn_Saxs::US_Hydrodyn_Saxs()
  if (saxs_options->wavelength == 0) {
    saxs_options->start_q = saxs_options->end_q = saxs_options->delta_q = 0;
  } else {
    saxs_options->start_q = 4.0 * M_PI *
                            sin(saxs_options->start_angle * M_PI / 360.0) /
                            saxs_options->wavelength;
    saxs_options->start_q =
        floor(saxs_options->start_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
    saxs_options->end_q = 4.0 * M_PI *
                          sin(saxs_options->end_angle * M_PI / 360.0) /
                          saxs_options->wavelength;
    saxs_options->end_q =
        floor(saxs_options->end_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
    saxs_options->delta_q = 4.0 * M_PI *
                            sin(saxs_options->delta_angle * M_PI / 360.0) /
                            saxs_options->wavelength;
    saxs_options->delta_q =
        floor(saxs_options->delta_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
  }
}

void US_Hydrodyn_SasOptionsCurve::update_start_q(double val) {
  (*saxs_options).start_q = (float)val;
  if (saxs_options->start_q > saxs_options->end_q) {
    saxs_options->start_q = saxs_options->end_q;
    cnt_start_q->setValue(saxs_options->end_q);
  }

  // note 360 / M_PI since 'theta' is 1/2 scattering angle

  saxs_options->start_angle =
      floor((asin(saxs_options->wavelength * saxs_options->start_q /
                  (4.0 * M_PI)) *
             360.0 / M_PI) *
                SAXS_Q_ROUNDING +
            0.5) /
      SAXS_Q_ROUNDING;
  cnt_start_angle->setValue(saxs_options->start_angle);

  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsCurve::update_end_q(double val) {
  (*saxs_options).end_q = (float)val;
  if (saxs_options->start_q > saxs_options->end_q) {
    saxs_options->end_q = saxs_options->start_q;
    cnt_end_q->setValue(saxs_options->start_q);
  }

  saxs_options->end_angle = floor((asin(saxs_options->wavelength *
                                        saxs_options->end_q / (4.0 * M_PI)) *
                                   360.0 / M_PI) *
                                      SAXS_Q_ROUNDING +
                                  0.5) /
                            SAXS_Q_ROUNDING;
  cnt_end_angle->setValue(saxs_options->end_angle);

  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsCurve::update_delta_q(double val) {
  (*saxs_options).delta_q = (float)val;

  saxs_options->delta_angle =
      floor((asin(saxs_options->wavelength * saxs_options->delta_q /
                  (4.0 * M_PI)) *
             360.0 / M_PI) *
                SAXS_Q_ROUNDING +
            0.5) /
      SAXS_Q_ROUNDING;
  cnt_delta_angle->setValue(saxs_options->delta_angle);

  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsCurve::set_normalize_by_mw() {
  (*saxs_options).normalize_by_mw = cb_normalize_by_mw->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
