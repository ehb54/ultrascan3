#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_asa.h"
// Added by qt3to4:
#include <QBoxLayout>
#include <QCloseEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#define SLASH "/"
#if defined(WIN32)
#undef SLASH
#define SLASH "\\"
#endif

// #define USE_H

US_Hydrodyn_SaxsOptions::US_Hydrodyn_SaxsOptions(
    struct saxs_options *saxs_options, bool *saxs_options_widget,
    bool *sas_options_saxs_widget, bool *sas_options_sans_widget,
    bool *sas_options_curve_widget, bool *sas_options_bead_model_widget,
    bool *sas_options_hydration_widget, bool *sas_options_guinier_widget,
    bool *sas_options_xsr_widget, bool *sas_options_misc_widget,
    bool *sas_options_experimental_widget, void *us_hydrodyn, QWidget *p,
    const char *)
    : QFrame(p) {
  this->saxs_options = saxs_options;

  this->saxs_options_widget = saxs_options_widget;

  this->sas_options_bead_model_widget = sas_options_bead_model_widget;
  this->sas_options_curve_widget = sas_options_curve_widget;
  this->sas_options_experimental_widget = sas_options_experimental_widget;
  this->sas_options_hydration_widget = sas_options_hydration_widget;
  this->sas_options_guinier_widget = sas_options_guinier_widget;
  this->sas_options_xsr_widget = sas_options_xsr_widget;
  this->sas_options_misc_widget = sas_options_misc_widget;
  this->sas_options_sans_widget = sas_options_sans_widget;
  this->sas_options_saxs_widget = sas_options_saxs_widget;

  this->us_hydrodyn = us_hydrodyn;

  *saxs_options_widget = true;

  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("US-SOMO SAS Options"));
  setupGUI();
  global_Xpos += 30;
  global_Ypos += 30;
  setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SaxsOptions::~US_Hydrodyn_SaxsOptions() {
  *saxs_options_widget = false;
}

void US_Hydrodyn_SaxsOptions::setupGUI() {
  started_in_expert_mode = U_EXPT;

  int minHeight1 = 30;
  lbl_info = new QLabel(us_tr("US-SOMO SAS Options:"), this);
  lbl_info->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_info->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_info->setMinimumHeight(minHeight1);
  lbl_info->setPalette(PALET_FRAME);
  AUTFBACK(lbl_info);
  lbl_info->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize + 1, QFont::Bold));

  pb_sas_options_saxs =
      new QPushButton(us_tr("SAXS computation options"), this);
  pb_sas_options_saxs->setFont(QFont(USglobal->config_list.fontFamily,
                                     USglobal->config_list.fontSize + 1));
  pb_sas_options_saxs->setMinimumHeight(minHeight1);
  pb_sas_options_saxs->setPalette(PALET_PUSHB);
  connect(pb_sas_options_saxs, SIGNAL(clicked()), SLOT(sas_options_saxs()));

  pb_sas_options_sans =
      new QPushButton(us_tr("SANS computation options"), this);
  pb_sas_options_sans->setFont(QFont(USglobal->config_list.fontFamily,
                                     USglobal->config_list.fontSize + 1));
  pb_sas_options_sans->setMinimumHeight(minHeight1);
  pb_sas_options_sans->setPalette(PALET_PUSHB);
  connect(pb_sas_options_sans, SIGNAL(clicked()), SLOT(sas_options_sans()));

  pb_sas_options_curve =
      new QPushButton(us_tr("Curve generation options"), this);
  pb_sas_options_curve->setFont(QFont(USglobal->config_list.fontFamily,
                                      USglobal->config_list.fontSize + 1));
  pb_sas_options_curve->setMinimumHeight(minHeight1);
  pb_sas_options_curve->setPalette(PALET_PUSHB);
  connect(pb_sas_options_curve, SIGNAL(clicked()), SLOT(sas_options_curve()));

  pb_sas_options_bead_model =
      new QPushButton(us_tr("Bead model options"), this);
  pb_sas_options_bead_model->setFont(QFont(USglobal->config_list.fontFamily,
                                           USglobal->config_list.fontSize + 1));
  pb_sas_options_bead_model->setMinimumHeight(minHeight1);
  pb_sas_options_bead_model->setPalette(PALET_PUSHB);
  connect(pb_sas_options_bead_model, SIGNAL(clicked()),
          SLOT(sas_options_bead_model()));

  if (started_in_expert_mode) {
    pb_sas_options_hydration =
        new QPushButton(us_tr("Hydration options"), this);
    pb_sas_options_hydration->setFont(QFont(
        USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
    pb_sas_options_hydration->setMinimumHeight(minHeight1);
    pb_sas_options_hydration->setPalette(PALET_PUSHB);
#if defined(USE_H)
    connect(pb_sas_options_hydration, SIGNAL(clicked()),
            SLOT(sas_options_hydration()));
#endif
  }

  pb_sas_options_guinier = new QPushButton(us_tr("Guinier options"), this);
  pb_sas_options_guinier->setFont(QFont(USglobal->config_list.fontFamily,
                                        USglobal->config_list.fontSize + 1));
  pb_sas_options_guinier->setMinimumHeight(minHeight1);
  pb_sas_options_guinier->setPalette(PALET_PUSHB);
  connect(pb_sas_options_guinier, SIGNAL(clicked()),
          SLOT(sas_options_guinier()));

  if (started_in_expert_mode) {
    pb_sas_options_xsr =
        new QPushButton(us_tr("Cross section fitting options"), this);
    pb_sas_options_xsr->setFont(QFont(USglobal->config_list.fontFamily,
                                      USglobal->config_list.fontSize + 1));
    pb_sas_options_xsr->setMinimumHeight(minHeight1);
    pb_sas_options_xsr->setPalette(PALET_PUSHB);
    connect(pb_sas_options_xsr, SIGNAL(clicked()), SLOT(sas_options_xsr()));
  }

  pb_sas_options_misc = new QPushButton(us_tr("Miscellaneous options"), this);
  pb_sas_options_misc->setFont(QFont(USglobal->config_list.fontFamily,
                                     USglobal->config_list.fontSize + 1));
  pb_sas_options_misc->setMinimumHeight(minHeight1);
  pb_sas_options_misc->setPalette(PALET_PUSHB);
  connect(pb_sas_options_misc, SIGNAL(clicked()), SLOT(sas_options_misc()));

  if (started_in_expert_mode) {
    pb_sas_options_experimental =
        new QPushButton(us_tr("Experimental code options"), this);
    pb_sas_options_experimental->setFont(QFont(
        USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
    pb_sas_options_experimental->setMinimumHeight(minHeight1);
    pb_sas_options_experimental->setPalette(PALET_PUSHB);
    connect(pb_sas_options_experimental, SIGNAL(clicked()),
            SLOT(sas_options_experimental()));
  }

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

  QBoxLayout *background = new QVBoxLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);

  background->addWidget(lbl_info);
  background->addWidget(pb_sas_options_saxs);
  background->addWidget(pb_sas_options_sans);
  background->addWidget(pb_sas_options_curve);
  background->addWidget(pb_sas_options_bead_model);
  if (started_in_expert_mode) {
    background->addWidget(pb_sas_options_hydration);
  }
  background->addWidget(pb_sas_options_guinier);
  if (started_in_expert_mode) {
    background->addWidget(pb_sas_options_xsr);
  }
  background->addWidget(pb_sas_options_misc);
  if (started_in_expert_mode) {
    background->addWidget(pb_sas_options_experimental);
  }

  QBoxLayout *helpclose = new QHBoxLayout;
  helpclose->setContentsMargins(0, 0, 0, 0);
  helpclose->setSpacing(0);

  helpclose->addWidget(pb_help);
  helpclose->addWidget(pb_cancel);

  background->addLayout(helpclose);

  setMinimumWidth(300);

  US_Hydrodyn::fixWinButtons(this);
}

void US_Hydrodyn_SaxsOptions::cancel() { close(); }

void US_Hydrodyn_SaxsOptions::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  online_help->show_help("manual/somo/somo_saxs_options.html");
}

void US_Hydrodyn_SaxsOptions::closeEvent(QCloseEvent *e) {
  *saxs_options_widget = false;
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

void US_Hydrodyn_SaxsOptions::sas_options_saxs() {
  if (*sas_options_saxs_widget) {
    if (((US_Hydrodyn *)us_hydrodyn)->sas_options_saxs_window->isVisible()) {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_saxs_window->raise();
    } else {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_saxs_window->show();
    }
    return;
  } else {
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_saxs_window =
        new US_Hydrodyn_SasOptionsSaxs(saxs_options, sas_options_saxs_widget,
                                       us_hydrodyn);
    US_Hydrodyn::fixWinButtons(
        ((US_Hydrodyn *)us_hydrodyn)->sas_options_saxs_window);
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_saxs_window->show();
  }
}

void US_Hydrodyn_SaxsOptions::sas_options_sans() {
  if (*sas_options_sans_widget) {
    if (((US_Hydrodyn *)us_hydrodyn)->sas_options_sans_window->isVisible()) {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_sans_window->raise();
    } else {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_sans_window->show();
    }
    return;
  } else {
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_sans_window =
        new US_Hydrodyn_SasOptionsSans(saxs_options, sas_options_sans_widget,
                                       us_hydrodyn);
    US_Hydrodyn::fixWinButtons(
        ((US_Hydrodyn *)us_hydrodyn)->sas_options_sans_window);
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_sans_window->show();
  }
}
void US_Hydrodyn_SaxsOptions::sas_options_curve() {
  if (*sas_options_curve_widget) {
    if (((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->isVisible()) {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->raise();
    } else {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->show();
    }
    return;
  } else {
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window =
        new US_Hydrodyn_SasOptionsCurve(saxs_options, sas_options_curve_widget,
                                        us_hydrodyn);
    US_Hydrodyn::fixWinButtons(
        ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window);
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_curve_window->show();
  }
}
void US_Hydrodyn_SaxsOptions::sas_options_bead_model() {
  if (*sas_options_bead_model_widget) {
    if (((US_Hydrodyn *)us_hydrodyn)
            ->sas_options_bead_model_window->isVisible()) {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_bead_model_window->raise();
    } else {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_bead_model_window->show();
    }
    return;
  } else {
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_bead_model_window =
        new US_Hydrodyn_SasOptionsBeadModel(
            saxs_options, sas_options_bead_model_widget, us_hydrodyn);
    US_Hydrodyn::fixWinButtons(
        ((US_Hydrodyn *)us_hydrodyn)->sas_options_bead_model_window);
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_bead_model_window->show();
  }
}
void US_Hydrodyn_SaxsOptions::sas_options_hydration() {
  if (*sas_options_hydration_widget) {
    if (((US_Hydrodyn *)us_hydrodyn)
            ->sas_options_hydration_window->isVisible()) {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_hydration_window->raise();
    } else {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_hydration_window->show();
    }
    return;
  } else {
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_hydration_window =
        new US_Hydrodyn_SasOptionsHydration(
            saxs_options, sas_options_hydration_widget, us_hydrodyn);
    US_Hydrodyn::fixWinButtons(
        ((US_Hydrodyn *)us_hydrodyn)->sas_options_hydration_window);
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_hydration_window->show();
  }
}
void US_Hydrodyn_SaxsOptions::sas_options_guinier() {
  if (*sas_options_guinier_widget) {
    if (((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_window->isVisible()) {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_window->raise();
    } else {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_window->show();
    }
    return;
  } else {
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_window =
        new US_Hydrodyn_SasOptionsGuinier(
            saxs_options, sas_options_guinier_widget, us_hydrodyn);
    US_Hydrodyn::fixWinButtons(
        ((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_window);
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_window->show();
  }
}
void US_Hydrodyn_SaxsOptions::sas_options_xsr() {
  if (*sas_options_xsr_widget) {
    if (((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window->isVisible()) {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window->raise();
    } else {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window->show();
    }
    return;
  } else {
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window =
        new US_Hydrodyn_SasOptionsXsr(saxs_options, sas_options_xsr_widget,
                                      us_hydrodyn);
    US_Hydrodyn::fixWinButtons(
        ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window);
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window->show();
  }
}
void US_Hydrodyn_SaxsOptions::sas_options_misc() {
  if (*sas_options_misc_widget) {
    if (((US_Hydrodyn *)us_hydrodyn)->sas_options_misc_window->isVisible()) {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_misc_window->raise();
    } else {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_misc_window->show();
    }
    return;
  } else {
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_misc_window =
        new US_Hydrodyn_SasOptionsMisc(saxs_options, sas_options_misc_widget,
                                       us_hydrodyn);
    US_Hydrodyn::fixWinButtons(
        ((US_Hydrodyn *)us_hydrodyn)->sas_options_misc_window);
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_misc_window->show();
  }
}
void US_Hydrodyn_SaxsOptions::sas_options_experimental() {
  if (*sas_options_experimental_widget) {
    if (((US_Hydrodyn *)us_hydrodyn)
            ->sas_options_experimental_window->isVisible()) {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_experimental_window->raise();
    } else {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_experimental_window->show();
    }
    return;
  } else {
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_experimental_window =
        new US_Hydrodyn_SasOptionsExperimental(
            saxs_options, sas_options_experimental_widget, us_hydrodyn);
    US_Hydrodyn::fixWinButtons(
        ((US_Hydrodyn *)us_hydrodyn)->sas_options_experimental_window);
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_experimental_window->show();
  }
}
