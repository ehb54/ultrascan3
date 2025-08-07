#include "../include/us_hydrodyn_cluster_additional.h"

#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_best.h"
#include "../include/us_hydrodyn_cluster_bfnb.h"
#include "../include/us_hydrodyn_cluster_bfnb_nsa.h"
#include "../include/us_hydrodyn_cluster_dammin.h"
#include "../include/us_hydrodyn_cluster_oned.h"
// Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char> &operator<<(std::basic_ostream<char> &os,
                                            const QString &str) {
  return os << qPrintable(str);
}

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Additional::US_Hydrodyn_Cluster_Additional(
    void *us_hydrodyn, QWidget *p, const char *)
    : QDialog(p) {
  cluster_window = (void *)p;

  this->us_hydrodyn = us_hydrodyn;

  options_active =
      (((US_Hydrodyn *)us_hydrodyn)->cluster_additional_methods_options_active);
  options_selected = (((US_Hydrodyn *)us_hydrodyn)
                          ->cluster_additional_methods_options_selected);

  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("US-SOMO: Cluster: Other Methods"));

  load_save_path = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster" +
                   SLASH + "parameters";

  QDir dir1(load_save_path);
  if (!dir1.exists()) {
    dir1.mkdir(load_save_path);
  }

  QDir::setCurrent(load_save_path);

  setupGUI();

  global_Xpos += 30;
  global_Ypos += 30;

  setGeometry(global_Xpos, global_Ypos, 0, 0);
  update_enables();
}

US_Hydrodyn_Cluster_Additional::~US_Hydrodyn_Cluster_Additional() {}

void US_Hydrodyn_Cluster_Additional::setupGUI() {
  bool expert_mode = U_EXPT;

  int minHeight1 = 30;

  lbl_title =
      new QLabel(us_tr("    US-SOMO: Cluster: Other Methods    "), this);
  lbl_title->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_title->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_title->setMinimumHeight(minHeight1);
  lbl_title->setPalette(PALET_FRAME);
  AUTFBACK(lbl_title);
  lbl_title->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize, QFont::Bold));

  cb_bfnb = new QCheckBox(this);
  cb_bfnb->setText(us_tr(" Active "));
  cb_bfnb->setChecked(options_active.count("bfnb") && options_active["bfnb"]);
  cb_bfnb->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_bfnb->setPalette(PALET_NORMAL);
  AUTFBACK(cb_bfnb);
  cb_bfnb->setMinimumHeight(minHeight1);
  connect(cb_bfnb, SIGNAL(clicked()), this, SLOT(set_bfnb()));

  pb_bfnb = new QPushButton("BFNB", this);
  pb_bfnb->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_bfnb->setMinimumHeight(minHeight1);
  pb_bfnb->setPalette(PALET_PUSHB);
  connect(pb_bfnb, SIGNAL(clicked()), SLOT(bfnb()));

  cb_bfnb_nsa = new QCheckBox(this);
  cb_bfnb_nsa->setText(us_tr(" Active "));
  cb_bfnb_nsa->setChecked(options_active.count("bfnb_nsa") &&
                          options_active["bfnb_nsa"]);
  cb_bfnb_nsa->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_bfnb_nsa->setPalette(PALET_NORMAL);
  AUTFBACK(cb_bfnb_nsa);
  cb_bfnb_nsa->setMinimumHeight(minHeight1);
  connect(cb_bfnb_nsa, SIGNAL(clicked()), this, SLOT(set_bfnb_nsa()));

  pb_bfnb_nsa = new QPushButton("BFNB/NSA", this);
  pb_bfnb_nsa->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_bfnb_nsa->setMinimumHeight(minHeight1);
  pb_bfnb_nsa->setPalette(PALET_PUSHB);
  connect(pb_bfnb_nsa, SIGNAL(clicked()), SLOT(bfnb_nsa()));

  cb_best = new QCheckBox(this);
  cb_best->setText(us_tr(" Active "));
  cb_best->setChecked(options_active.count("best") && options_active["best"]);
  cb_best->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_best->setPalette(PALET_NORMAL);
  AUTFBACK(cb_best);
  cb_best->setMinimumHeight(minHeight1);
  connect(cb_best, SIGNAL(clicked()), this, SLOT(set_best()));

  pb_best = new QPushButton("BEST", this);
  pb_best->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_best->setMinimumHeight(minHeight1);
  pb_best->setPalette(PALET_PUSHB);
  connect(pb_best, SIGNAL(clicked()), SLOT(best()));

  cb_oned = new QCheckBox(this);
  cb_oned->setText(us_tr(" Active "));
  cb_oned->setChecked(options_active.count("oned") && options_active["oned"]);
  cb_oned->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_oned->setPalette(PALET_NORMAL);
  AUTFBACK(cb_oned);
  cb_oned->setMinimumHeight(minHeight1);
  connect(cb_oned, SIGNAL(clicked()), this, SLOT(set_oned()));

  pb_oned = new QPushButton("1d", this);
  pb_oned->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_oned->setMinimumHeight(minHeight1);
  pb_oned->setPalette(PALET_PUSHB);
  connect(pb_oned, SIGNAL(clicked()), SLOT(oned()));

  cb_csa = new QCheckBox(this);
  cb_csa->setText(us_tr(" Active "));
  cb_csa->setChecked(options_active.count("csa") && options_active["csa"]);
  cb_csa->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_csa->setPalette(PALET_NORMAL);
  AUTFBACK(cb_csa);
  cb_csa->setMinimumHeight(minHeight1);
  connect(cb_csa, SIGNAL(clicked()), this, SLOT(set_csa()));

  pb_csa = new QPushButton("CSA", this);
  pb_csa->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_csa->setMinimumHeight(minHeight1);
  pb_csa->setPalette(PALET_PUSHB);
  connect(pb_csa, SIGNAL(clicked()), SLOT(csa()));

  cb_dammin = new QCheckBox(this);
  cb_dammin->setText(us_tr(" Active "));
  cb_dammin->setChecked(options_active.count("dammin") &&
                        options_active["dammin"]);
  cb_dammin->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_dammin->setPalette(PALET_NORMAL);
  AUTFBACK(cb_dammin);
  cb_dammin->setMinimumHeight(minHeight1);
  connect(cb_dammin, SIGNAL(clicked()), this, SLOT(set_dammin()));

  pb_dammin = new QPushButton("DAMMIN", this);
  pb_dammin->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_dammin->setMinimumHeight(minHeight1);
  pb_dammin->setPalette(PALET_PUSHB);
  connect(pb_dammin, SIGNAL(clicked()), SLOT(dammin()));

  cb_dammif = new QCheckBox(this);
  cb_dammif->setText(us_tr(" Active "));
  cb_dammif->setChecked(options_active.count("dammif") &&
                        options_active["dammif"]);
  cb_dammif->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_dammif->setPalette(PALET_NORMAL);
  AUTFBACK(cb_dammif);
  cb_dammif->setMinimumHeight(minHeight1);
  connect(cb_dammif, SIGNAL(clicked()), this, SLOT(set_dammif()));

  pb_dammif = new QPushButton("DAMMIF", this);
  pb_dammif->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_dammif->setMinimumHeight(minHeight1);
  pb_dammif->setPalette(PALET_PUSHB);
  connect(pb_dammif, SIGNAL(clicked()), SLOT(dammif()));

  cb_gasbor = new QCheckBox(this);
  cb_gasbor->setText(us_tr(" Active "));
  cb_gasbor->setChecked(options_active.count("gasbor") &&
                        options_active["gasbor"]);
  cb_gasbor->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_gasbor->setPalette(PALET_NORMAL);
  AUTFBACK(cb_gasbor);
  cb_gasbor->setMinimumHeight(minHeight1);
  connect(cb_gasbor, SIGNAL(clicked()), this, SLOT(set_gasbor()));

  pb_gasbor = new QPushButton("GASBOR", this);
  pb_gasbor->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_gasbor->setMinimumHeight(minHeight1);
  pb_gasbor->setPalette(PALET_PUSHB);
  connect(pb_gasbor, SIGNAL(clicked()), SLOT(gasbor()));

  pb_cancel = new QPushButton(us_tr("Cancel"), this);
  pb_cancel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_cancel->setMinimumHeight(minHeight1);
  pb_cancel->setPalette(PALET_PUSHB);
  connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

  pb_help = new QPushButton(us_tr("Help"), this);
  pb_help->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_help->setMinimumHeight(minHeight1);
  pb_help->setPalette(PALET_PUSHB);
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));

  pb_ok = new QPushButton(us_tr("Close"), this);
  pb_ok->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  pb_ok->setMinimumHeight(minHeight1);
  pb_ok->setPalette(PALET_PUSHB);
  connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

  if (!expert_mode) {
    cb_bfnb->hide();
    pb_bfnb->hide();
    cb_bfnb_nsa->hide();
    pb_bfnb_nsa->hide();
    // cb_best->hide();
    // pb_best->hide();
    cb_oned->hide();
    pb_oned->hide();
    cb_csa->hide();
    pb_csa->hide();
    cb_dammin->hide();
    pb_dammin->hide();
    cb_dammif->hide();
    pb_dammif->hide();
    cb_gasbor->hide();
    pb_gasbor->hide();
  }

  // build layout

  QGridLayout *gl_options = new QGridLayout(0);
  gl_options->setContentsMargins(0, 0, 0, 0);
  gl_options->setSpacing(0);

  int j = 0;

  gl_options->addWidget(cb_bfnb, j, 0);
  gl_options->addWidget(pb_bfnb, j, 1);
  j++;

  gl_options->addWidget(cb_bfnb_nsa, j, 0);
  gl_options->addWidget(pb_bfnb_nsa, j, 1);
  j++;

  gl_options->addWidget(cb_best, j, 0);
  gl_options->addWidget(pb_best, j, 1);
  j++;

  gl_options->addWidget(cb_oned, j, 0);
  gl_options->addWidget(pb_oned, j, 1);
  j++;

  gl_options->addWidget(cb_csa, j, 0);
  gl_options->addWidget(pb_csa, j, 1);
  j++;

  gl_options->addWidget(cb_dammin, j, 0);
  gl_options->addWidget(pb_dammin, j, 1);
  j++;

  gl_options->addWidget(cb_dammif, j, 0);
  gl_options->addWidget(pb_dammif, j, 1);
  j++;

  gl_options->addWidget(cb_gasbor, j, 0);
  gl_options->addWidget(pb_gasbor, j, 1);
  j++;

  QHBoxLayout *hbl_bottom = new QHBoxLayout();
  hbl_bottom->setContentsMargins(0, 0, 0, 0);
  hbl_bottom->setSpacing(0);
  hbl_bottom->addSpacing(4);
  hbl_bottom->addWidget(pb_cancel);
  hbl_bottom->addSpacing(4);
  hbl_bottom->addWidget(pb_help);
  hbl_bottom->addSpacing(4);
  hbl_bottom->addWidget(pb_ok);
  hbl_bottom->addSpacing(4);

  QVBoxLayout *background = new QVBoxLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);
  background->addSpacing(4);
  background->addWidget(lbl_title);
  background->addSpacing(4);
  background->addLayout(gl_options);
  background->addSpacing(4);
  background->addLayout(hbl_bottom);
  background->addSpacing(4);
}

void US_Hydrodyn_Cluster_Additional::ok() {
  (((US_Hydrodyn *)us_hydrodyn)->cluster_additional_methods_options_active) =
      options_active;
  (((US_Hydrodyn *)us_hydrodyn)->cluster_additional_methods_options_selected) =
      options_selected;
  close();
}

void US_Hydrodyn_Cluster_Additional::cancel() { close(); }

void US_Hydrodyn_Cluster_Additional::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  online_help->show_help("manual/somo/somo_cluster_additional.html");
}

void US_Hydrodyn_Cluster_Additional::closeEvent(QCloseEvent *e) {
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

void US_Hydrodyn_Cluster_Additional::update_enables() {
  pb_bfnb->setEnabled(cb_bfnb->isChecked());
  pb_bfnb_nsa->setEnabled(cb_bfnb_nsa->isChecked());
  pb_best->setEnabled(cb_best->isChecked());
  pb_oned->setEnabled(cb_oned->isChecked());
  pb_csa->setEnabled(cb_csa->isChecked());
  pb_dammin->setEnabled(cb_dammin->isChecked());
  pb_dammif->setEnabled(cb_dammif->isChecked());
  pb_gasbor->setEnabled(cb_gasbor->isChecked());

  // disabled for now:
  // cb_bfnb      ->setEnabled( false );
  // cb_bfnb_nsa  ->setEnabled( false );
  cb_csa->setEnabled(false);
  // cb_dammin    ->setEnabled( false );
  cb_dammif->setEnabled(false);
  cb_gasbor->setEnabled(false);
}

void US_Hydrodyn_Cluster_Additional::set_bfnb() {
  options_active["bfnb"] = cb_bfnb->isChecked();
  update_enables();
}

void US_Hydrodyn_Cluster_Additional::bfnb() {
  cout << QString("current load save path %1\n").arg(load_save_path);
  map<QString, QString> parameters;
  if (options_selected.count("bfnb")) {
    parameters = options_selected["bfnb"];
  }
  QDir::setCurrent(load_save_path);
  US_Hydrodyn_Cluster_Bfnb *hc =
      new US_Hydrodyn_Cluster_Bfnb(us_hydrodyn, &parameters, this);
  US_Hydrodyn::fixWinButtons(hc);
  hc->exec();
  delete hc;
  options_selected["bfnb"] = parameters;
}

void US_Hydrodyn_Cluster_Additional::set_bfnb_nsa() {
  options_active["bfnb_nsa"] = cb_bfnb_nsa->isChecked();
  update_enables();
}

void US_Hydrodyn_Cluster_Additional::bfnb_nsa() {
  cout << QString("current load save path %1\n").arg(load_save_path);
  map<QString, QString> parameters;
  if (options_selected.count("bfnb_nsa")) {
    parameters = options_selected["bfnb_nsa"];
  }
  QDir::setCurrent(load_save_path);
  US_Hydrodyn_Cluster_Bfnb_Nsa *hc =
      new US_Hydrodyn_Cluster_Bfnb_Nsa(us_hydrodyn, &parameters, this);
  US_Hydrodyn::fixWinButtons(hc);
  hc->exec();
  delete hc;
  options_selected["bfnb_nsa"] = parameters;
}

void US_Hydrodyn_Cluster_Additional::set_best() {
  options_active["best"] = cb_best->isChecked();
  update_enables();
}

void US_Hydrodyn_Cluster_Additional::best() {
  cout << QString("current load save path %1\n").arg(load_save_path);
  map<QString, QString> parameters;
  if (options_selected.count("best")) {
    parameters = options_selected["best"];
  }
  QDir::setCurrent(load_save_path);
  US_Hydrodyn_Cluster_Best *hc =
      new US_Hydrodyn_Cluster_Best(us_hydrodyn, &parameters, this);
  US_Hydrodyn::fixWinButtons(hc);
  hc->exec();
  delete hc;
  options_selected["best"] = parameters;
}

void US_Hydrodyn_Cluster_Additional::set_oned() {
  options_active["oned"] = cb_oned->isChecked();
  update_enables();
}

void US_Hydrodyn_Cluster_Additional::oned() {
  QDir::setCurrent(load_save_path);
  map<QString, QString> parameters;
  if (options_selected.count("oned")) {
    parameters = options_selected["oned"];
  }
  US_Hydrodyn_Cluster_Oned *hc =
      new US_Hydrodyn_Cluster_Oned(us_hydrodyn, &parameters, this);
  US_Hydrodyn::fixWinButtons(hc);
  hc->exec();
  delete hc;
  options_selected["oned"] = parameters;
}

void US_Hydrodyn_Cluster_Additional::set_csa() {
  options_active["csa"] = cb_csa->isChecked();
  update_enables();
}

void US_Hydrodyn_Cluster_Additional::csa() { QDir::setCurrent(load_save_path); }

void US_Hydrodyn_Cluster_Additional::set_dammin() {
  options_active["dammin"] = cb_dammin->isChecked();
  update_enables();
}

void US_Hydrodyn_Cluster_Additional::dammin() {
  QDir::setCurrent(load_save_path);
  map<QString, QString> parameters;
  if (options_selected.count("dammin")) {
    parameters = options_selected["dammin"];
  }
  US_Hydrodyn_Cluster_Dammin *hc =
      new US_Hydrodyn_Cluster_Dammin(us_hydrodyn, &parameters, this);
  US_Hydrodyn::fixWinButtons(hc);
  hc->exec();
  delete hc;
  options_selected["dammin"] = parameters;
}

void US_Hydrodyn_Cluster_Additional::set_dammif() {
  options_active["dammif"] = cb_dammif->isChecked();
  update_enables();
}

void US_Hydrodyn_Cluster_Additional::dammif() {
  QDir::setCurrent(load_save_path);
}

void US_Hydrodyn_Cluster_Additional::set_gasbor() {
  options_active["gasbor"] = cb_gasbor->isChecked();
  update_enables();
}

void US_Hydrodyn_Cluster_Additional::gasbor() {
  QDir::setCurrent(load_save_path);
}
