#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_asa.h"
// Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>

#define SLASH "/"
#if defined(WIN32)
#undef SLASH
#define SLASH "\\"
#endif

US_Hydrodyn_SasOptionsXsr::US_Hydrodyn_SasOptionsXsr(
    struct saxs_options *saxs_options, bool *sas_options_xsr_widget,
    void *us_hydrodyn, QWidget *p, const char *)
    : QFrame(p) {
  this->sas_options_xsr_widget = sas_options_xsr_widget;
  this->saxs_options = saxs_options;
  this->us_hydrodyn = us_hydrodyn;
  *sas_options_xsr_widget = true;
  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("US-SOMO SAS Cross Sectional Fitting Options"));
  setupGUI();
  global_Xpos += 30;
  global_Ypos += 30;
  setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SasOptionsXsr::~US_Hydrodyn_SasOptionsXsr() {
  *sas_options_xsr_widget = false;
}

void US_Hydrodyn_SasOptionsXsr::setupGUI() {
  int minHeight1 = 30;
  int minHeight2 = 50;

  lbl_info =
      new QLabel(us_tr("US-SOMO SAS Cross Sectional Fitting Options:"), this);
  lbl_info->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_info->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_info->setMinimumHeight(minHeight1);
  lbl_info->setPalette(PALET_FRAME);
  AUTFBACK(lbl_info);
  lbl_info->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize + 1, QFont::Bold));

  lbl_xsr_symmop = new QLabel(us_tr(" Symmetry of dummy-atom model :\n( 1 = no "
                                    "symmetry, 2 = 2 fold symmetry )"),
                              this);
  lbl_xsr_symmop->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_xsr_symmop->setMinimumHeight(minHeight2);
  lbl_xsr_symmop->setPalette(PALET_LABEL);
  AUTFBACK(lbl_xsr_symmop);
  lbl_xsr_symmop->setFont(QFont(USglobal->config_list.fontFamily,
                                USglobal->config_list.fontSize - 1,
                                QFont::Bold));

  le_xsr_symmop = new QLineEdit(this);
  le_xsr_symmop->setObjectName("xsr_symmop Line Edit");
  le_xsr_symmop->setText(QString("%1").arg(saxs_options->xsr_symmop));
  le_xsr_symmop->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_xsr_symmop->setPalette(PALET_NORMAL);
  AUTFBACK(le_xsr_symmop);
  le_xsr_symmop->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_xsr_symmop->setMinimumHeight(minHeight2);
  le_xsr_symmop->setMinimumWidth(80);
  connect(le_xsr_symmop, SIGNAL(textChanged(const QString &)),
          SLOT(update_xsr_symmop(const QString &)));

  lbl_xsr_nx = new QLabel(
      us_tr(" Number of x grid points in the dummy atom model : "), this);
  lbl_xsr_nx->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_xsr_nx->setMinimumHeight(minHeight1);
  lbl_xsr_nx->setPalette(PALET_LABEL);
  AUTFBACK(lbl_xsr_nx);
  lbl_xsr_nx->setFont(QFont(USglobal->config_list.fontFamily,
                            USglobal->config_list.fontSize - 1, QFont::Bold));

  le_xsr_nx = new QLineEdit(this);
  le_xsr_nx->setObjectName("xsr_nx Line Edit");
  le_xsr_nx->setText(QString("%1").arg(saxs_options->xsr_nx));
  le_xsr_nx->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_xsr_nx->setPalette(PALET_NORMAL);
  AUTFBACK(le_xsr_nx);
  le_xsr_nx->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_xsr_nx, SIGNAL(textChanged(const QString &)),
          SLOT(update_xsr_nx(const QString &)));

  lbl_xsr_ny = new QLabel(
      us_tr(" Number of y grid points in the dummy atom model :"), this);
  lbl_xsr_ny->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_xsr_ny->setMinimumHeight(minHeight1);
  lbl_xsr_ny->setPalette(PALET_LABEL);
  AUTFBACK(lbl_xsr_ny);
  lbl_xsr_ny->setFont(QFont(USglobal->config_list.fontFamily,
                            USglobal->config_list.fontSize - 1, QFont::Bold));

  le_xsr_ny = new QLineEdit(this);
  le_xsr_ny->setObjectName("xsr_ny Line Edit");
  le_xsr_ny->setText(QString("%1").arg(saxs_options->xsr_ny));
  le_xsr_ny->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_xsr_ny->setPalette(PALET_NORMAL);
  AUTFBACK(le_xsr_ny);
  le_xsr_ny->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_xsr_ny, SIGNAL(textChanged(const QString &)),
          SLOT(update_xsr_ny(const QString &)));

  lbl_xsr_griddistance =
      new QLabel(us_tr(" Radius of each dummy atom (A) : "), this);
  lbl_xsr_griddistance->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_xsr_griddistance->setMinimumHeight(minHeight1);
  lbl_xsr_griddistance->setPalette(PALET_LABEL);
  AUTFBACK(lbl_xsr_griddistance);
  lbl_xsr_griddistance->setFont(QFont(USglobal->config_list.fontFamily,
                                      USglobal->config_list.fontSize - 1,
                                      QFont::Bold));

  le_xsr_griddistance = new QLineEdit(this);
  le_xsr_griddistance->setObjectName("xsr_griddistance Line Edit");
  le_xsr_griddistance->setText(
      QString("%1").arg(saxs_options->xsr_griddistance));
  le_xsr_griddistance->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_xsr_griddistance->setPalette(PALET_NORMAL);
  AUTFBACK(le_xsr_griddistance);
  le_xsr_griddistance->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_xsr_griddistance, SIGNAL(textChanged(const QString &)),
          SLOT(update_xsr_griddistance(const QString &)));

  lbl_xsr_ncomponents = new QLabel(
      us_tr(" Number of components with different scattering densities : "),
      this);
  lbl_xsr_ncomponents->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_xsr_ncomponents->setMinimumHeight(minHeight1);
  lbl_xsr_ncomponents->setPalette(PALET_LABEL);
  AUTFBACK(lbl_xsr_ncomponents);
  lbl_xsr_ncomponents->setFont(QFont(USglobal->config_list.fontFamily,
                                     USglobal->config_list.fontSize - 1,
                                     QFont::Bold));

  le_xsr_ncomponents = new QLineEdit(this);
  le_xsr_ncomponents->setObjectName("xsr_ncomponents Line Edit");
  le_xsr_ncomponents->setText(QString("%1").arg(saxs_options->xsr_ncomponents));
  le_xsr_ncomponents->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_xsr_ncomponents->setPalette(PALET_NORMAL);
  AUTFBACK(le_xsr_ncomponents);
  le_xsr_ncomponents->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_xsr_ncomponents, SIGNAL(textChanged(const QString &)),
          SLOT(update_xsr_ncomponents(const QString &)));

  lbl_xsr_compactness_weight =
      new QLabel(us_tr(" Compactness weight : "), this);
  lbl_xsr_compactness_weight->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_xsr_compactness_weight->setMinimumHeight(minHeight1);
  lbl_xsr_compactness_weight->setPalette(PALET_LABEL);
  AUTFBACK(lbl_xsr_compactness_weight);
  lbl_xsr_compactness_weight->setFont(QFont(USglobal->config_list.fontFamily,
                                            USglobal->config_list.fontSize - 1,
                                            QFont::Bold));

  le_xsr_compactness_weight = new QLineEdit(this);
  le_xsr_compactness_weight->setObjectName("xsr_compactness_weight Line Edit");
  le_xsr_compactness_weight->setText(
      QString("%1").arg(saxs_options->xsr_compactness_weight));
  le_xsr_compactness_weight->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_xsr_compactness_weight->setPalette(PALET_NORMAL);
  AUTFBACK(le_xsr_compactness_weight);
  le_xsr_compactness_weight->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_xsr_compactness_weight, SIGNAL(textChanged(const QString &)),
          SLOT(update_xsr_compactness_weight(const QString &)));

  lbl_xsr_looseness_weight = new QLabel(us_tr(" Looseness weight : "), this);
  lbl_xsr_looseness_weight->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_xsr_looseness_weight->setMinimumHeight(minHeight1);
  lbl_xsr_looseness_weight->setPalette(PALET_LABEL);
  AUTFBACK(lbl_xsr_looseness_weight);
  lbl_xsr_looseness_weight->setFont(QFont(USglobal->config_list.fontFamily,
                                          USglobal->config_list.fontSize - 1,
                                          QFont::Bold));

  le_xsr_looseness_weight = new QLineEdit(this);
  le_xsr_looseness_weight->setObjectName("xsr_looseness_weight Line Edit");
  le_xsr_looseness_weight->setText(
      QString("%1").arg(saxs_options->xsr_looseness_weight));
  le_xsr_looseness_weight->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_xsr_looseness_weight->setPalette(PALET_NORMAL);
  AUTFBACK(le_xsr_looseness_weight);
  le_xsr_looseness_weight->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_xsr_looseness_weight, SIGNAL(textChanged(const QString &)),
          SLOT(update_xsr_looseness_weight(const QString &)));

  lbl_xsr_temperature = new QLabel(us_tr(" Temperature : "), this);
  lbl_xsr_temperature->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_xsr_temperature->setMinimumHeight(minHeight1);
  lbl_xsr_temperature->setPalette(PALET_LABEL);
  AUTFBACK(lbl_xsr_temperature);
  lbl_xsr_temperature->setFont(QFont(USglobal->config_list.fontFamily,
                                     USglobal->config_list.fontSize - 1,
                                     QFont::Bold));

  le_xsr_temperature = new QLineEdit(this);
  le_xsr_temperature->setObjectName("xsr_temperature Line Edit");
  le_xsr_temperature->setText(QString("%1").arg(saxs_options->xsr_temperature));
  le_xsr_temperature->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_xsr_temperature->setPalette(PALET_NORMAL);
  AUTFBACK(le_xsr_temperature);
  le_xsr_temperature->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_xsr_temperature, SIGNAL(textChanged(const QString &)),
          SLOT(update_xsr_temperature(const QString &)));

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

  background->addWidget(lbl_xsr_symmop, j, 0);
  background->addWidget(le_xsr_symmop, j, 1);
  j++;
  background->addWidget(lbl_xsr_nx, j, 0);
  background->addWidget(le_xsr_nx, j, 1);
  j++;
  background->addWidget(lbl_xsr_ny, j, 0);
  background->addWidget(le_xsr_ny, j, 1);
  j++;
  background->addWidget(lbl_xsr_griddistance, j, 0);
  background->addWidget(le_xsr_griddistance, j, 1);
  j++;
  background->addWidget(lbl_xsr_ncomponents, j, 0);
  background->addWidget(le_xsr_ncomponents, j, 1);
  j++;
  background->addWidget(lbl_xsr_compactness_weight, j, 0);
  background->addWidget(le_xsr_compactness_weight, j, 1);
  j++;
  background->addWidget(lbl_xsr_looseness_weight, j, 0);
  background->addWidget(le_xsr_looseness_weight, j, 1);
  j++;
  background->addWidget(lbl_xsr_temperature, j, 0);
  background->addWidget(le_xsr_temperature, j, 1);
  j++;

  background->addWidget(pb_help, j, 0);
  background->addWidget(pb_cancel, j, 1);

  setMinimumWidth(400);
}

void US_Hydrodyn_SasOptionsXsr::cancel() { close(); }

void US_Hydrodyn_SasOptionsXsr::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  // online_help->show_help("manual/somo/somo_sas_options_xsr.html");
  online_help->show_help("manual/somo/somo_saxs_options.html");
}

void US_Hydrodyn_SasOptionsXsr::closeEvent(QCloseEvent *e) {
  *sas_options_xsr_widget = false;
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

void US_Hydrodyn_SasOptionsXsr::update_xsr_symmop(const QString &str) {
  saxs_options->xsr_symmop = str.toUInt();
}

void US_Hydrodyn_SasOptionsXsr::update_xsr_nx(const QString &str) {
  saxs_options->xsr_nx = str.toUInt();
}

void US_Hydrodyn_SasOptionsXsr::update_xsr_ny(const QString &str) {
  saxs_options->xsr_ny = str.toUInt();
}

void US_Hydrodyn_SasOptionsXsr::update_xsr_griddistance(const QString &str) {
  saxs_options->xsr_griddistance = str.toDouble();
}

void US_Hydrodyn_SasOptionsXsr::update_xsr_ncomponents(const QString &str) {
  saxs_options->xsr_ncomponents = str.toUInt();
}

void US_Hydrodyn_SasOptionsXsr::update_xsr_compactness_weight(
    const QString &str) {
  saxs_options->xsr_compactness_weight = str.toDouble();
}

void US_Hydrodyn_SasOptionsXsr::update_xsr_looseness_weight(
    const QString &str) {
  saxs_options->xsr_looseness_weight = str.toDouble();
}

void US_Hydrodyn_SasOptionsXsr::update_xsr_temperature(const QString &str) {
  saxs_options->xsr_temperature = str.toDouble();
}
