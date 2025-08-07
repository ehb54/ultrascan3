#include "../include/us_hydrodyn_bd_options.h"

#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
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

US_Hydrodyn_BD_Options::US_Hydrodyn_BD_Options(BD_Options *bd_options,
                                               bool *bd_widget,
                                               void *us_hydrodyn, QWidget *p,
                                               const char *)
    : QFrame(p) {
  this->bd_options = bd_options;
  this->bd_widget = bd_widget;
  this->us_hydrodyn = us_hydrodyn;
  *bd_widget = true;
  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("SOMO BD Options"));
  setupGUI();
  global_Xpos += 30;
  global_Ypos += 30;
  setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_BD_Options::~US_Hydrodyn_BD_Options() { *bd_widget = false; }

void US_Hydrodyn_BD_Options::setupGUI() {
  QFont qf;
  QString str;

  int minHeight1 = 30;
  lbl_info = new QLabel(us_tr("BD Options:"), this);
  lbl_info->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_info->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_info->setMinimumHeight(minHeight1);
  lbl_info->setPalette(PALET_FRAME);
  AUTFBACK(lbl_info);
  lbl_info->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize + 1, QFont::Bold));

  lbl_info_model_creation = new QLabel(us_tr("Connector Discovery:"), this);
  lbl_info_model_creation->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_info_model_creation->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_info_model_creation->setMinimumHeight(minHeight1);
  lbl_info_model_creation->setPalette(PALET_FRAME);
  AUTFBACK(lbl_info_model_creation);
  lbl_info_model_creation->setFont(QFont(USglobal->config_list.fontFamily,
                                         USglobal->config_list.fontSize + 1,
                                         QFont::Bold));

  lbl_info_simulation_opts = new QLabel(us_tr("Simulation Parameters:"), this);
  lbl_info_simulation_opts->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_info_simulation_opts->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_info_simulation_opts->setMinimumHeight(minHeight1);
  lbl_info_simulation_opts->setPalette(PALET_FRAME);
  AUTFBACK(lbl_info_simulation_opts);
  lbl_info_simulation_opts->setFont(QFont(USglobal->config_list.fontFamily,
                                          USglobal->config_list.fontSize + 1,
                                          QFont::Bold));

  lbl_springs = new QLabel(us_tr("Connector Spring Definitions (spring "
                                 "constant in erg/cm^2, distance in cm):"),
                           this);
  lbl_springs->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_springs->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_springs->setMinimumHeight(minHeight1);
  lbl_springs->setPalette(PALET_FRAME);
  AUTFBACK(lbl_springs);
  lbl_springs->setFont(QFont(USglobal->config_list.fontFamily,
                             USglobal->config_list.fontSize + 1, QFont::Bold));

  lbl_bd_threshold_pb_pb = new QLabel(us_tr(" Threshold PB-PB (A): "), this);
  lbl_bd_threshold_pb_pb->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_bd_threshold_pb_pb->setMinimumHeight(minHeight1);
  lbl_bd_threshold_pb_pb->setPalette(PALET_LABEL);
  AUTFBACK(lbl_bd_threshold_pb_pb);
  lbl_bd_threshold_pb_pb->setFont(QFont(USglobal->config_list.fontFamily,
                                        USglobal->config_list.fontSize - 1,
                                        QFont::Bold));

  cnt_bd_threshold_pb_pb = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_bd_threshold_pb_pb);
  cnt_bd_threshold_pb_pb->setRange(0, 100);
  cnt_bd_threshold_pb_pb->setSingleStep(0.1);
  cnt_bd_threshold_pb_pb->setValue((*bd_options).threshold_pb_pb);
  cnt_bd_threshold_pb_pb->setMinimumHeight(minHeight1);
  cnt_bd_threshold_pb_pb->setMinimumWidth(150);
  cnt_bd_threshold_pb_pb->setEnabled(true);
  cnt_bd_threshold_pb_pb->setNumButtons(3);
  cnt_bd_threshold_pb_pb->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_bd_threshold_pb_pb->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_bd_threshold_pb_pb);
  connect(cnt_bd_threshold_pb_pb, SIGNAL(valueChanged(double)),
          SLOT(update_bd_threshold_pb_pb(double)));

  lbl_bd_threshold_pb_sc = new QLabel(us_tr(" Threshold PB-SC (A): "), this);
  lbl_bd_threshold_pb_sc->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_bd_threshold_pb_sc->setMinimumHeight(minHeight1);
  lbl_bd_threshold_pb_sc->setPalette(PALET_LABEL);
  AUTFBACK(lbl_bd_threshold_pb_sc);
  lbl_bd_threshold_pb_sc->setFont(QFont(USglobal->config_list.fontFamily,
                                        USglobal->config_list.fontSize - 1,
                                        QFont::Bold));

  cnt_bd_threshold_pb_sc = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_bd_threshold_pb_sc);
  cnt_bd_threshold_pb_sc->setRange(0, 100);
  cnt_bd_threshold_pb_sc->setSingleStep(0.1);
  cnt_bd_threshold_pb_sc->setValue((*bd_options).threshold_pb_sc);
  cnt_bd_threshold_pb_sc->setMinimumHeight(minHeight1);
  cnt_bd_threshold_pb_sc->setMinimumWidth(150);
  cnt_bd_threshold_pb_sc->setEnabled(true);
  cnt_bd_threshold_pb_sc->setNumButtons(3);
  cnt_bd_threshold_pb_sc->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_bd_threshold_pb_sc->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_bd_threshold_pb_sc);
  connect(cnt_bd_threshold_pb_sc, SIGNAL(valueChanged(double)),
          SLOT(update_bd_threshold_pb_sc(double)));

  lbl_bd_threshold_sc_sc = new QLabel(us_tr(" Threshold SC-SC (A): "), this);
  lbl_bd_threshold_sc_sc->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_bd_threshold_sc_sc->setMinimumHeight(minHeight1);
  lbl_bd_threshold_sc_sc->setPalette(PALET_LABEL);
  AUTFBACK(lbl_bd_threshold_sc_sc);
  lbl_bd_threshold_sc_sc->setFont(QFont(USglobal->config_list.fontFamily,
                                        USglobal->config_list.fontSize - 1,
                                        QFont::Bold));

  cnt_bd_threshold_sc_sc = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_bd_threshold_sc_sc);
  cnt_bd_threshold_sc_sc->setRange(0, 100);
  cnt_bd_threshold_sc_sc->setSingleStep(0.1);
  cnt_bd_threshold_sc_sc->setValue((*bd_options).threshold_sc_sc);
  cnt_bd_threshold_sc_sc->setMinimumHeight(minHeight1);
  cnt_bd_threshold_sc_sc->setMinimumWidth(150);
  cnt_bd_threshold_sc_sc->setEnabled(true);
  cnt_bd_threshold_sc_sc->setNumButtons(3);
  cnt_bd_threshold_sc_sc->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_bd_threshold_sc_sc->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_bd_threshold_sc_sc);
  connect(cnt_bd_threshold_sc_sc, SIGNAL(valueChanged(double)),
          SLOT(update_bd_threshold_sc_sc(double)));

#if QT_VERSION < 0x040000
  bg_bead_size_type =
      new QGroupBox(3, Qt::Vertical, " Bead sizes determined ", this);
  Q_CHECK_PTR(bg_bead_size_type);
  bg_bead_size_type->setExclusive(true);
  bg_bead_size_type->setAlignment(Qt::AlignHCenter);
  bg_bead_size_type->setInsideMargin(3);
  bg_bead_size_type->setInsideSpacing(0);
  connect(bg_bead_size_type, SIGNAL(clicked(int)), this,
          SLOT(set_bead_size_type(int)));

  cb_bead_size_type_1st = new QCheckBox(bg_bead_size_type);
  cb_bead_size_type_1st->setText(us_tr(" First model's beads "));
  cb_bead_size_type_1st->setEnabled(true);
  //   cb_bead_size_type_1st->setMinimumHeight(minHeight1);
  cb_bead_size_type_1st->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_bead_size_type_1st->setPalette(PALET_NORMAL);
  AUTFBACK(cb_bead_size_type_1st);

  cb_bead_size_type_min = new QCheckBox(bg_bead_size_type);
  cb_bead_size_type_min->setText(us_tr(" Minimum size "));
  cb_bead_size_type_min->setEnabled(true);
  //   cb_bead_size_type_min->setMinimumHeight(minHeight1);
  cb_bead_size_type_min->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_bead_size_type_min->setPalette(PALET_NORMAL);
  AUTFBACK(cb_bead_size_type_min);

  cb_bead_size_type_avg = new QCheckBox(bg_bead_size_type);
  cb_bead_size_type_avg->setText(us_tr(" Average size "));
  cb_bead_size_type_avg->setEnabled(true);
  //   cb_bead_size_type_avg->setMinimumHeight(minHeight1);
  cb_bead_size_type_avg->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_bead_size_type_avg->setPalette(PALET_NORMAL);
  AUTFBACK(cb_bead_size_type_avg);

  bg_bead_size_type->setButton(bd_options->bead_size_type);
#else
  bg_bead_size_type = new QGroupBox(" Bead sizes determined ");
  Q_CHECK_PTR(bg_bead_size_type);

  rb_bead_size_type_1st = new QRadioButton();
  rb_bead_size_type_1st->setText(us_tr(" First model's beads "));
  rb_bead_size_type_1st->setEnabled(true);
  //   rb_bead_size_type_1st->setMinimumHeight(minHeight1);
  rb_bead_size_type_1st->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_bead_size_type_1st->setPalette(PALET_NORMAL);
  AUTFBACK(rb_bead_size_type_1st);
  connect(rb_bead_size_type_1st, SIGNAL(clicked()), this,
          SLOT(set_bead_size_type()));

  rb_bead_size_type_min = new QRadioButton();
  rb_bead_size_type_min->setText(us_tr(" Minimum size "));
  rb_bead_size_type_min->setEnabled(true);
  //   rb_bead_size_type_min->setMinimumHeight(minHeight1);
  rb_bead_size_type_min->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_bead_size_type_min->setPalette(PALET_NORMAL);
  AUTFBACK(rb_bead_size_type_min);
  connect(rb_bead_size_type_min, SIGNAL(clicked()), this,
          SLOT(set_bead_size_type()));

  rb_bead_size_type_avg = new QRadioButton();
  rb_bead_size_type_avg->setText(us_tr(" Average size "));
  rb_bead_size_type_avg->setEnabled(true);
  //   rb_bead_size_type_avg->setMinimumHeight(minHeight1);
  rb_bead_size_type_avg->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_bead_size_type_avg->setPalette(PALET_NORMAL);
  AUTFBACK(rb_bead_size_type_avg);
  connect(rb_bead_size_type_avg, SIGNAL(clicked()), this,
          SLOT(set_bead_size_type()));

  {
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(rb_bead_size_type_1st);
    vbox->addWidget(rb_bead_size_type_min);
    vbox->addWidget(rb_bead_size_type_avg);
    bg_bead_size_type->setLayout(vbox);
  }

  switch (bd_options->bead_size_type) {
    case 0:
      rb_bead_size_type_1st->setChecked(true);
      break;
    case 1:
      rb_bead_size_type_min->setChecked(true);
      break;
    case 2:
      rb_bead_size_type_avg->setChecked(true);
      break;
    default:
      qDebug() << "bd options bead size type selection error";
      break;
  }
#endif

  lbl_npadif = new QLabel(
      us_tr(" Number of consecutive steps without recalculating: "), this);
  lbl_npadif->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_npadif->setMinimumHeight(minHeight1);
  lbl_npadif->setPalette(PALET_LABEL);
  AUTFBACK(lbl_npadif);
  lbl_npadif->setFont(QFont(USglobal->config_list.fontFamily,
                            USglobal->config_list.fontSize - 1, QFont::Bold));

  cnt_npadif = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_npadif);
  cnt_npadif->setRange(1, 999);
  cnt_npadif->setSingleStep(1);
  cnt_npadif->setValue((*bd_options).npadif);
  cnt_npadif->setMinimumHeight(minHeight1);
  cnt_npadif->setMinimumWidth(150);
  cnt_npadif->setEnabled(true);
  cnt_npadif->setNumButtons(3);
  cnt_npadif->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_npadif->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_npadif);
  connect(cnt_npadif, SIGNAL(valueChanged(double)),
          SLOT(update_npadif(double)));

  lbl_nmol = new QLabel(us_tr(" Number of subtrajectories: "), this);
  lbl_nmol->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_nmol->setMinimumHeight(minHeight1);
  lbl_nmol->setPalette(PALET_LABEL);
  AUTFBACK(lbl_nmol);
  lbl_nmol->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize - 1, QFont::Bold));

  le_nmol = new QLineEdit(this);
  le_nmol->setObjectName("Nmol Line Edit");
  le_nmol->setText(str.sprintf("%d", (*bd_options).nmol));
  le_nmol->setAlignment(Qt::AlignVCenter);
  le_nmol->setPalette(PALET_NORMAL);
  AUTFBACK(le_nmol);
  le_nmol->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_nmol->setEnabled(true);
  connect(le_nmol, SIGNAL(textChanged(const QString &)),
          SLOT(update_nmol(const QString &)));

  lbl_nconf = new QLabel(us_tr(" Number of conformations to store: "), this);
  lbl_nconf->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_nconf->setMinimumHeight(minHeight1);
  lbl_nconf->setPalette(PALET_LABEL);
  AUTFBACK(lbl_nconf);
  lbl_nconf->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize - 1, QFont::Bold));

  le_nconf = new QLineEdit(this);
  le_nconf->setObjectName("Nconf Line Edit");
  le_nconf->setText(str.sprintf("%d", (*bd_options).nconf));
  le_nconf->setAlignment(Qt::AlignVCenter);
  le_nconf->setPalette(PALET_NORMAL);
  AUTFBACK(le_nconf);
  le_nconf->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_nconf->setEnabled(true);
  connect(le_nconf, SIGNAL(textChanged(const QString &)),
          SLOT(update_nconf(const QString &)));

  lbl_iseed = new QLabel(us_tr(" Random seed: "), this);
  lbl_iseed->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_iseed->setMinimumHeight(minHeight1);
  lbl_iseed->setPalette(PALET_LABEL);
  AUTFBACK(lbl_iseed);
  lbl_iseed->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize - 1, QFont::Bold));

  le_iseed = new QLineEdit(this);
  le_iseed->setObjectName("Iseed Line Edit");
  le_iseed->setText(str.sprintf("%d", (*bd_options).iseed));
  le_iseed->setAlignment(Qt::AlignVCenter);
  le_iseed->setPalette(PALET_NORMAL);
  AUTFBACK(le_iseed);
  le_iseed->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_iseed->setEnabled(true);
  connect(le_iseed, SIGNAL(textChanged(const QString &)),
          SLOT(update_iseed(const QString &)));

  cb_do_rr = new QCheckBox(this);
  cb_do_rr->setText(us_tr(" Use radial reduction "));
  cb_do_rr->setChecked((*bd_options).do_rr);
  cb_do_rr->setEnabled(true);
  //   cb_do_rr->setMinimumHeight(minHeight1);
  cb_do_rr->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_do_rr->setPalette(PALET_NORMAL);
  AUTFBACK(cb_do_rr);
  connect(cb_do_rr, SIGNAL(clicked()), SLOT(set_do_rr()));

  cb_force_chem = new QCheckBox(this);
  cb_force_chem->setText(us_tr(" Force chemical bonds as connectors "));
  cb_force_chem->setChecked((*bd_options).force_chem);
  cb_force_chem->setEnabled(true);
  // cb_force_chem->setMinimumHeight(minHeight1);
  cb_force_chem->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_force_chem->setPalette(PALET_NORMAL);
  AUTFBACK(cb_force_chem);
  connect(cb_force_chem, SIGNAL(clicked()), SLOT(set_force_chem()));

  cb_icdm = new QCheckBox(this);
  cb_icdm->setText(us_tr(" Center of mass reference frame "));
  cb_icdm->setChecked((*bd_options).icdm);
  cb_icdm->setEnabled(true);
  cb_icdm->setMinimumHeight(minHeight1);
  cb_icdm->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_icdm->setPalette(PALET_NORMAL);
  AUTFBACK(cb_icdm);
  connect(cb_icdm, SIGNAL(clicked()), SLOT(set_icdm()));

  cb_show_pdb = new QCheckBox(this);
  cb_show_pdb->setText(us_tr(" Show connection PDB "));
  cb_show_pdb->setChecked((*bd_options).show_pdb);
  cb_show_pdb->setEnabled(true);
  //   cb_show_pdb->setMinimumHeight(minHeight1);
  cb_show_pdb->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_show_pdb->setPalette(PALET_NORMAL);
  AUTFBACK(cb_show_pdb);
  connect(cb_show_pdb, SIGNAL(clicked()), SLOT(set_show_pdb()));

  cb_run_browflex = new QCheckBox(this);
  cb_run_browflex->setText(us_tr(" Run BrowFlex "));
  cb_run_browflex->setChecked((*bd_options).run_browflex);
  cb_run_browflex->setEnabled(true);
  //   cb_run_browflex->setMinimumHeight(minHeight1);
  cb_run_browflex->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_run_browflex->setPalette(PALET_NORMAL);
  AUTFBACK(cb_run_browflex);
  connect(cb_run_browflex, SIGNAL(clicked()), SLOT(set_run_browflex()));

  lbl_tprev = new QLabel(us_tr(" Previous heating time (s): "), this);
  lbl_tprev->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_tprev->setMinimumHeight(minHeight1);
  lbl_tprev->setPalette(PALET_LABEL);
  AUTFBACK(lbl_tprev);
  lbl_tprev->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize - 1, QFont::Bold));

  le_tprev = new QLineEdit(this);
  le_tprev->setObjectName("Tprev Line Edit");
  le_tprev->setText(str.sprintf("%4.2g", (*bd_options).tprev));
  le_tprev->setAlignment(Qt::AlignVCenter);
  le_tprev->setPalette(PALET_NORMAL);
  AUTFBACK(le_tprev);
  le_tprev->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_tprev->setEnabled(true);
  connect(le_tprev, SIGNAL(textChanged(const QString &)),
          SLOT(update_tprev(const QString &)));

  lbl_ttraj = new QLabel(us_tr(" Total time duration (s): "), this);
  lbl_ttraj->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_ttraj->setMinimumHeight(minHeight1);
  lbl_ttraj->setPalette(PALET_LABEL);
  AUTFBACK(lbl_ttraj);
  lbl_ttraj->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize - 1, QFont::Bold));

  le_ttraj = new QLineEdit(this);
  le_ttraj->setObjectName("Ttraj Line Edit");
  le_ttraj->setText(str.sprintf("%4.2g", (*bd_options).ttraj));
  le_ttraj->setAlignment(Qt::AlignVCenter);
  le_ttraj->setPalette(PALET_NORMAL);
  AUTFBACK(le_ttraj);
  le_ttraj->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_ttraj->setEnabled(true);
  connect(le_ttraj, SIGNAL(textChanged(const QString &)),
          SLOT(update_ttraj(const QString &)));

  lbl_deltat =
      new QLabel(us_tr(" Duration of each simulation step (s): "), this);
  lbl_deltat->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_deltat->setMinimumHeight(minHeight1);
  lbl_deltat->setPalette(PALET_LABEL);
  AUTFBACK(lbl_deltat);
  lbl_deltat->setFont(QFont(USglobal->config_list.fontFamily,
                            USglobal->config_list.fontSize - 1, QFont::Bold));

  le_deltat = new QLineEdit(this);
  le_deltat->setObjectName("Deltat Line Edit");
  le_deltat->setText(str.sprintf("%4.6g", (*bd_options).deltat));
  le_deltat->setAlignment(Qt::AlignVCenter);
  le_deltat->setPalette(PALET_NORMAL);
  AUTFBACK(le_deltat);
  le_deltat->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_deltat->setEnabled(true);
  connect(le_deltat, SIGNAL(textChanged(const QString &)),
          SLOT(update_deltat(const QString &)));

  lbl_chem_pb_pb = new QLabel(us_tr(" Chemical PB-PB: "), this);
  lbl_chem_pb_pb->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  lbl_chem_pb_pb->setMinimumHeight(minHeight1);
  lbl_chem_pb_pb->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chem_pb_pb);
  lbl_chem_pb_pb->setFont(QFont(USglobal->config_list.fontFamily,
                                USglobal->config_list.fontSize - 1,
                                QFont::Bold));

  lbl_chem_pb_sc = new QLabel(us_tr(" Chemical PB-SC: "), this);
  lbl_chem_pb_sc->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  lbl_chem_pb_sc->setMinimumHeight(minHeight1);
  lbl_chem_pb_sc->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chem_pb_sc);
  lbl_chem_pb_sc->setFont(QFont(USglobal->config_list.fontFamily,
                                USglobal->config_list.fontSize - 1,
                                QFont::Bold));

  lbl_chem_sc_sc = new QLabel(us_tr(" Chemical SC-SC: "), this);
  lbl_chem_sc_sc->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  lbl_chem_sc_sc->setMinimumHeight(minHeight1);
  lbl_chem_sc_sc->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chem_sc_sc);
  lbl_chem_sc_sc->setFont(QFont(USglobal->config_list.fontFamily,
                                USglobal->config_list.fontSize - 1,
                                QFont::Bold));

  lbl_pb_pb = new QLabel(us_tr(" PB-PB: "), this);
  lbl_pb_pb->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  lbl_pb_pb->setMinimumHeight(minHeight1);
  lbl_pb_pb->setPalette(PALET_LABEL);
  AUTFBACK(lbl_pb_pb);
  lbl_pb_pb->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize - 1, QFont::Bold));

  lbl_pb_sc = new QLabel(us_tr(" PB-SC: "), this);
  lbl_pb_sc->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  lbl_pb_sc->setMinimumHeight(minHeight1);
  lbl_pb_sc->setPalette(PALET_LABEL);
  AUTFBACK(lbl_pb_sc);
  lbl_pb_sc->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize - 1, QFont::Bold));

  lbl_sc_sc = new QLabel(us_tr(" SC-SC: "), this);
  lbl_sc_sc->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  lbl_sc_sc->setMinimumHeight(minHeight1);
  lbl_sc_sc->setPalette(PALET_LABEL);
  AUTFBACK(lbl_sc_sc);
  lbl_sc_sc->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize - 1, QFont::Bold));

  cb_compute_chem_pb_pb_force_constant = new QCheckBox(this);
  cb_compute_chem_pb_pb_force_constant->setText(us_tr(" Compute "));
  cb_compute_chem_pb_pb_force_constant->setChecked(
      (*bd_options).compute_chem_pb_pb_force_constant);
  cb_compute_chem_pb_pb_force_constant->setEnabled(true);
  cb_compute_chem_pb_pb_force_constant->setMinimumHeight(minHeight1);
  cb_compute_chem_pb_pb_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_chem_pb_pb_force_constant->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_chem_pb_pb_force_constant);
  connect(cb_compute_chem_pb_pb_force_constant, SIGNAL(clicked()),
          SLOT(set_compute_chem_pb_pb_force_constant()));

  cb_compute_chem_pb_pb_equilibrium_dist = new QCheckBox(this);
  cb_compute_chem_pb_pb_equilibrium_dist->setText(us_tr(" Compute "));
  cb_compute_chem_pb_pb_equilibrium_dist->setChecked(
      (*bd_options).compute_chem_pb_pb_equilibrium_dist);
  cb_compute_chem_pb_pb_equilibrium_dist->setEnabled(true);
  cb_compute_chem_pb_pb_equilibrium_dist->setMinimumHeight(minHeight1);
  cb_compute_chem_pb_pb_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_chem_pb_pb_equilibrium_dist->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_chem_pb_pb_equilibrium_dist);
  connect(cb_compute_chem_pb_pb_equilibrium_dist, SIGNAL(clicked()),
          SLOT(set_compute_chem_pb_pb_equilibrium_dist()));

  cb_compute_chem_pb_pb_max_elong = new QCheckBox(this);
  cb_compute_chem_pb_pb_max_elong->setText(us_tr(" Compute "));
  cb_compute_chem_pb_pb_max_elong->setChecked(
      (*bd_options).compute_chem_pb_pb_max_elong);
  cb_compute_chem_pb_pb_max_elong->setEnabled(true);
  cb_compute_chem_pb_pb_max_elong->setMinimumHeight(minHeight1);
  cb_compute_chem_pb_pb_max_elong->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_chem_pb_pb_max_elong->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_chem_pb_pb_max_elong);
  connect(cb_compute_chem_pb_pb_max_elong, SIGNAL(clicked()),
          SLOT(set_compute_chem_pb_pb_max_elong()));

  cb_compute_chem_pb_sc_force_constant = new QCheckBox(this);
  cb_compute_chem_pb_sc_force_constant->setText(us_tr(" Compute "));
  cb_compute_chem_pb_sc_force_constant->setChecked(
      (*bd_options).compute_chem_pb_sc_force_constant);
  cb_compute_chem_pb_sc_force_constant->setEnabled(true);
  cb_compute_chem_pb_sc_force_constant->setMinimumHeight(minHeight1);
  cb_compute_chem_pb_sc_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_chem_pb_sc_force_constant->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_chem_pb_sc_force_constant);
  connect(cb_compute_chem_pb_sc_force_constant, SIGNAL(clicked()),
          SLOT(set_compute_chem_pb_sc_force_constant()));

  cb_compute_chem_pb_sc_equilibrium_dist = new QCheckBox(this);
  cb_compute_chem_pb_sc_equilibrium_dist->setText(us_tr(" Compute "));
  cb_compute_chem_pb_sc_equilibrium_dist->setChecked(
      (*bd_options).compute_chem_pb_sc_equilibrium_dist);
  cb_compute_chem_pb_sc_equilibrium_dist->setEnabled(true);
  cb_compute_chem_pb_sc_equilibrium_dist->setMinimumHeight(minHeight1);
  cb_compute_chem_pb_sc_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_chem_pb_sc_equilibrium_dist->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_chem_pb_sc_equilibrium_dist);
  connect(cb_compute_chem_pb_sc_equilibrium_dist, SIGNAL(clicked()),
          SLOT(set_compute_chem_pb_sc_equilibrium_dist()));

  cb_compute_chem_pb_sc_max_elong = new QCheckBox(this);
  cb_compute_chem_pb_sc_max_elong->setText(us_tr(" Compute "));
  cb_compute_chem_pb_sc_max_elong->setChecked(
      (*bd_options).compute_chem_pb_sc_max_elong);
  cb_compute_chem_pb_sc_max_elong->setEnabled(true);
  cb_compute_chem_pb_sc_max_elong->setMinimumHeight(minHeight1);
  cb_compute_chem_pb_sc_max_elong->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_chem_pb_sc_max_elong->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_chem_pb_sc_max_elong);
  connect(cb_compute_chem_pb_sc_max_elong, SIGNAL(clicked()),
          SLOT(set_compute_chem_pb_sc_max_elong()));

  cb_compute_chem_sc_sc_force_constant = new QCheckBox(this);
  cb_compute_chem_sc_sc_force_constant->setText(us_tr(" Compute "));
  cb_compute_chem_sc_sc_force_constant->setChecked(
      (*bd_options).compute_chem_sc_sc_force_constant);
  cb_compute_chem_sc_sc_force_constant->setEnabled(true);
  cb_compute_chem_sc_sc_force_constant->setMinimumHeight(minHeight1);
  cb_compute_chem_sc_sc_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_chem_sc_sc_force_constant->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_chem_sc_sc_force_constant);
  connect(cb_compute_chem_sc_sc_force_constant, SIGNAL(clicked()),
          SLOT(set_compute_chem_sc_sc_force_constant()));

  cb_compute_chem_sc_sc_equilibrium_dist = new QCheckBox(this);
  cb_compute_chem_sc_sc_equilibrium_dist->setText(us_tr(" Compute "));
  cb_compute_chem_sc_sc_equilibrium_dist->setChecked(
      (*bd_options).compute_chem_sc_sc_equilibrium_dist);
  cb_compute_chem_sc_sc_equilibrium_dist->setEnabled(true);
  cb_compute_chem_sc_sc_equilibrium_dist->setMinimumHeight(minHeight1);
  cb_compute_chem_sc_sc_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_chem_sc_sc_equilibrium_dist->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_chem_sc_sc_equilibrium_dist);
  connect(cb_compute_chem_sc_sc_equilibrium_dist, SIGNAL(clicked()),
          SLOT(set_compute_chem_sc_sc_equilibrium_dist()));

  cb_compute_chem_sc_sc_max_elong = new QCheckBox(this);
  cb_compute_chem_sc_sc_max_elong->setText(us_tr(" Compute "));
  cb_compute_chem_sc_sc_max_elong->setChecked(
      (*bd_options).compute_chem_sc_sc_max_elong);
  cb_compute_chem_sc_sc_max_elong->setEnabled(true);
  cb_compute_chem_sc_sc_max_elong->setMinimumHeight(minHeight1);
  cb_compute_chem_sc_sc_max_elong->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_chem_sc_sc_max_elong->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_chem_sc_sc_max_elong);
  connect(cb_compute_chem_sc_sc_max_elong, SIGNAL(clicked()),
          SLOT(set_compute_chem_sc_sc_max_elong()));

  cb_compute_pb_pb_force_constant = new QCheckBox(this);
  cb_compute_pb_pb_force_constant->setText(us_tr(" Compute "));
  cb_compute_pb_pb_force_constant->setChecked(
      (*bd_options).compute_pb_pb_force_constant);
  cb_compute_pb_pb_force_constant->setEnabled(true);
  cb_compute_pb_pb_force_constant->setMinimumHeight(minHeight1);
  cb_compute_pb_pb_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_pb_pb_force_constant->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_pb_pb_force_constant);
  connect(cb_compute_pb_pb_force_constant, SIGNAL(clicked()),
          SLOT(set_compute_pb_pb_force_constant()));

  cb_compute_pb_pb_equilibrium_dist = new QCheckBox(this);
  cb_compute_pb_pb_equilibrium_dist->setText(us_tr(" Compute "));
  cb_compute_pb_pb_equilibrium_dist->setChecked(
      (*bd_options).compute_pb_pb_equilibrium_dist);
  cb_compute_pb_pb_equilibrium_dist->setEnabled(true);
  cb_compute_pb_pb_equilibrium_dist->setMinimumHeight(minHeight1);
  cb_compute_pb_pb_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_pb_pb_equilibrium_dist->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_pb_pb_equilibrium_dist);
  connect(cb_compute_pb_pb_equilibrium_dist, SIGNAL(clicked()),
          SLOT(set_compute_pb_pb_equilibrium_dist()));

  cb_compute_pb_pb_max_elong = new QCheckBox(this);
  cb_compute_pb_pb_max_elong->setText(us_tr(" Compute "));
  cb_compute_pb_pb_max_elong->setChecked((*bd_options).compute_pb_pb_max_elong);
  cb_compute_pb_pb_max_elong->setEnabled(true);
  cb_compute_pb_pb_max_elong->setMinimumHeight(minHeight1);
  cb_compute_pb_pb_max_elong->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_pb_pb_max_elong->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_pb_pb_max_elong);
  connect(cb_compute_pb_pb_max_elong, SIGNAL(clicked()),
          SLOT(set_compute_pb_pb_max_elong()));

  cb_compute_pb_sc_force_constant = new QCheckBox(this);
  cb_compute_pb_sc_force_constant->setText(us_tr(" Compute "));
  cb_compute_pb_sc_force_constant->setChecked(
      (*bd_options).compute_pb_sc_force_constant);
  cb_compute_pb_sc_force_constant->setEnabled(true);
  cb_compute_pb_sc_force_constant->setMinimumHeight(minHeight1);
  cb_compute_pb_sc_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_pb_sc_force_constant->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_pb_sc_force_constant);
  connect(cb_compute_pb_sc_force_constant, SIGNAL(clicked()),
          SLOT(set_compute_pb_sc_force_constant()));

  cb_compute_pb_sc_equilibrium_dist = new QCheckBox(this);
  cb_compute_pb_sc_equilibrium_dist->setText(us_tr(" Compute "));
  cb_compute_pb_sc_equilibrium_dist->setChecked(
      (*bd_options).compute_pb_sc_equilibrium_dist);
  cb_compute_pb_sc_equilibrium_dist->setEnabled(true);
  cb_compute_pb_sc_equilibrium_dist->setMinimumHeight(minHeight1);
  cb_compute_pb_sc_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_pb_sc_equilibrium_dist->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_pb_sc_equilibrium_dist);
  connect(cb_compute_pb_sc_equilibrium_dist, SIGNAL(clicked()),
          SLOT(set_compute_pb_sc_equilibrium_dist()));

  cb_compute_pb_sc_max_elong = new QCheckBox(this);
  cb_compute_pb_sc_max_elong->setText(us_tr(" Compute "));
  cb_compute_pb_sc_max_elong->setChecked((*bd_options).compute_pb_sc_max_elong);
  cb_compute_pb_sc_max_elong->setEnabled(true);
  cb_compute_pb_sc_max_elong->setMinimumHeight(minHeight1);
  cb_compute_pb_sc_max_elong->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_pb_sc_max_elong->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_pb_sc_max_elong);
  connect(cb_compute_pb_sc_max_elong, SIGNAL(clicked()),
          SLOT(set_compute_pb_sc_max_elong()));

  cb_compute_sc_sc_force_constant = new QCheckBox(this);
  cb_compute_sc_sc_force_constant->setText(us_tr(" Compute "));
  cb_compute_sc_sc_force_constant->setChecked(
      (*bd_options).compute_sc_sc_force_constant);
  cb_compute_sc_sc_force_constant->setEnabled(true);
  cb_compute_sc_sc_force_constant->setMinimumHeight(minHeight1);
  cb_compute_sc_sc_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_sc_sc_force_constant->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_sc_sc_force_constant);
  connect(cb_compute_sc_sc_force_constant, SIGNAL(clicked()),
          SLOT(set_compute_sc_sc_force_constant()));

  cb_compute_sc_sc_equilibrium_dist = new QCheckBox(this);
  cb_compute_sc_sc_equilibrium_dist->setText(us_tr(" Compute "));
  cb_compute_sc_sc_equilibrium_dist->setChecked(
      (*bd_options).compute_sc_sc_equilibrium_dist);
  cb_compute_sc_sc_equilibrium_dist->setEnabled(true);
  cb_compute_sc_sc_equilibrium_dist->setMinimumHeight(minHeight1);
  cb_compute_sc_sc_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_sc_sc_equilibrium_dist->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_sc_sc_equilibrium_dist);
  connect(cb_compute_sc_sc_equilibrium_dist, SIGNAL(clicked()),
          SLOT(set_compute_sc_sc_equilibrium_dist()));

  cb_compute_sc_sc_max_elong = new QCheckBox(this);
  cb_compute_sc_sc_max_elong->setText(us_tr(" Compute "));
  cb_compute_sc_sc_max_elong->setChecked((*bd_options).compute_sc_sc_max_elong);
  cb_compute_sc_sc_max_elong->setEnabled(true);
  cb_compute_sc_sc_max_elong->setMinimumHeight(minHeight1);
  cb_compute_sc_sc_max_elong->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_sc_sc_max_elong->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_sc_sc_max_elong);
  connect(cb_compute_sc_sc_max_elong, SIGNAL(clicked()),
          SLOT(set_compute_sc_sc_max_elong()));

#if QT_VERSION < 0x040000
  bg_inter =
      new QGroupBox(3, Qt::Vertical, "Type of simulation algorithm:", this);
  Q_CHECK_PTR(bg_inter);
  bg_inter->setExclusive(true);
  bg_inter->setAlignment(Qt::AlignHCenter);
  bg_inter->setInsideMargin(3);
  bg_inter->setInsideSpacing(0);
  connect(bg_inter, SIGNAL(clicked(int)), this, SLOT(set_inter(int)));

  cb_inter_no_hi = new QCheckBox(bg_inter);
  cb_inter_no_hi->setText(us_tr(" No hydrodynamic interaction (HI) "));
  cb_inter_no_hi->setEnabled(true);
  //   cb_inter_no_hi->setMinimumHeight(minHeight1);
  cb_inter_no_hi->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_inter_no_hi->setPalette(PALET_NORMAL);
  AUTFBACK(cb_inter_no_hi);

  cb_inter_os = new QCheckBox(bg_inter);
  cb_inter_os->setText(us_tr(" HI Oseen "));
  cb_inter_os->setEnabled(true);
  //   cb_inter_os->setMinimumHeight(minHeight1);
  cb_inter_os->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_inter_os->setPalette(PALET_NORMAL);
  AUTFBACK(cb_inter_os);

  cb_inter_mos = new QCheckBox(bg_inter);
  cb_inter_mos->setText(us_tr(" HI modified Oseen "));
  cb_inter_mos->setEnabled(true);
  //   cb_inter_mos->setMinimumHeight(minHeight1);
  cb_inter_mos->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_inter_mos->setPalette(PALET_NORMAL);
  AUTFBACK(cb_inter_mos);

  bg_inter->setButton(bd_options->inter);
#else
  bg_inter = new QGroupBox("Type of simulation algorithm:");
  Q_CHECK_PTR(bg_inter);

  rb_inter_no_hi = new QRadioButton();
  rb_inter_no_hi->setText(us_tr(" No hydrodynamic interaction (HI) "));
  rb_inter_no_hi->setEnabled(true);
  //   rb_inter_no_hi->setMinimumHeight(minHeight1);
  rb_inter_no_hi->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_inter_no_hi->setPalette(PALET_NORMAL);
  AUTFBACK(rb_inter_no_hi);
  connect(rb_inter_no_hi, SIGNAL(clicked()), this, SLOT(set_inter()));

  rb_inter_os = new QRadioButton();
  rb_inter_os->setText(us_tr(" HI Oseen "));
  rb_inter_os->setEnabled(true);
  //   rb_inter_os->setMinimumHeight(minHeight1);
  rb_inter_os->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_inter_os->setPalette(PALET_NORMAL);
  AUTFBACK(rb_inter_os);
  connect(rb_inter_os, SIGNAL(clicked()), this, SLOT(set_inter()));

  rb_inter_mos = new QRadioButton();
  rb_inter_mos->setText(us_tr(" HI modified Oseen "));
  rb_inter_mos->setEnabled(true);
  //   rb_inter_mos->setMinimumHeight(minHeight1);
  rb_inter_mos->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_inter_mos->setPalette(PALET_NORMAL);
  AUTFBACK(rb_inter_mos);
  connect(rb_inter_mos, SIGNAL(clicked()), this, SLOT(set_inter()));

  {
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(rb_inter_no_hi);
    vbox->addWidget(rb_inter_os);
    vbox->addWidget(rb_inter_mos);
    bg_inter->setLayout(vbox);
  }

  switch (bd_options->inter) {
    case 0:
      rb_inter_no_hi->setChecked(true);
      break;
    case 1:
      rb_inter_os->setChecked(true);
      break;
    case 2:
      rb_inter_mos->setChecked(true);
      break;
    default:
      qDebug() << "bd options inter selection error";
      break;
  }
#endif

#if QT_VERSION < 0x040000
  bg_iorder = new QGroupBox(3, Qt::Vertical,
                            "Order of the simulation algorithm:", this);
  Q_CHECK_PTR(bg_iorder);
  bg_iorder->setExclusive(true);
  bg_iorder->setAlignment(Qt::AlignHCenter);
  bg_iorder->setInsideMargin(3);
  bg_iorder->setInsideSpacing(0);
  connect(bg_iorder, SIGNAL(clicked(int)), this, SLOT(set_iorder(int)));

  cb_iorder_em = new QCheckBox(bg_iorder);
  cb_iorder_em->setText(us_tr(" Ermak-McCammon 1st order "));
  cb_iorder_em->setEnabled(true);
  //   cb_iorder_em->setMinimumHeight(minHeight1);
  cb_iorder_em->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_iorder_em->setPalette(PALET_NORMAL);
  AUTFBACK(cb_iorder_em);

  cb_iorder_igt = new QCheckBox(bg_iorder);
  cb_iorder_igt->setText(
      us_tr(" Iniesta-Garcia de la Torre predictor-corrector"));
  cb_iorder_igt->setEnabled(true);
  //   cb_iorder_igt->setMinimumHeight(minHeight1);
  cb_iorder_igt->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_iorder_igt->setPalette(PALET_NORMAL);
  AUTFBACK(cb_iorder_igt);

  bg_iorder->setButton(bd_options->iorder);
#else
  bg_iorder = new QGroupBox("Order of the simulation algorithm:");
  Q_CHECK_PTR(bg_iorder);

  rb_iorder_em = new QRadioButton();
  rb_iorder_em->setText(us_tr(" Ermak-McCammon 1st order "));
  rb_iorder_em->setEnabled(true);
  //   rb_iorder_em->setMinimumHeight(minHeight1);
  rb_iorder_em->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_iorder_em->setPalette(PALET_NORMAL);
  AUTFBACK(rb_iorder_em);
  connect(rb_iorder_em, SIGNAL(clicked()), this, SLOT(set_iorder()));

  rb_iorder_igt = new QRadioButton();
  rb_iorder_igt->setText(
      us_tr(" Iniesta-Garcia de la Torre predictor-corrector"));
  rb_iorder_igt->setEnabled(true);
  //   rb_iorder_igt->setMinimumHeight(minHeight1);
  rb_iorder_igt->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_iorder_igt->setPalette(PALET_NORMAL);
  AUTFBACK(rb_iorder_igt);
  connect(rb_iorder_igt, SIGNAL(clicked()), this, SLOT(set_iorder()));

  {
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(rb_iorder_em);
    vbox->addWidget(rb_iorder_igt);
    bg_iorder->setLayout(vbox);
  }

  switch (bd_options->iorder) {
    case 0:
      rb_iorder_em->setChecked(true);
      break;
    case 1:
      rb_iorder_igt->setChecked(true);
      break;
    default:
      qDebug() << "bd options iorder selection error";
      break;
  }
#endif

  pb_dup_fraenkel = new QPushButton(us_tr("Replicate"), this);
  pb_dup_fraenkel->setFont(QFont(USglobal->config_list.fontFamily,
                                 USglobal->config_list.fontSize + 1));
  pb_dup_fraenkel->setMinimumHeight(minHeight1);
  pb_dup_fraenkel->setPalette(PALET_PUSHB);
  connect(pb_dup_fraenkel, SIGNAL(clicked()), SLOT(dup_fraenkel()));

#if QT_VERSION < 0x040000
  bg_chem_pb_pb_bond_types = new QGroupBox(4, Qt::Vertical, "Bond type:", this);
  qf = bg_chem_pb_pb_bond_types->font();
  qf.setPointSize(qf.pointSize() - 1);
  bg_chem_pb_pb_bond_types->setFont(qf);
  bg_chem_pb_pb_bond_types->setExclusive(true);
  bg_chem_pb_pb_bond_types->setAlignment(Qt::AlignHCenter);
  bg_chem_pb_pb_bond_types->setInsideMargin(3);
  bg_chem_pb_pb_bond_types->setInsideSpacing(0);
  connect(bg_chem_pb_pb_bond_types, SIGNAL(clicked(int)), this,
          SLOT(set_chem_pb_pb_bond_types(int)));

  cb_chem_pb_pb_bond_type_fraenkel = new QCheckBox(bg_chem_pb_pb_bond_types);
  cb_chem_pb_pb_bond_type_fraenkel->setText(us_tr(" Fraenkel (hard Hookean) "));
  cb_chem_pb_pb_bond_type_fraenkel->setEnabled(true);
  //   cb_chem_pb_pb_bond_type_fraenkel->setMinimumHeight(minHeight1);
  cb_chem_pb_pb_bond_type_fraenkel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_chem_pb_pb_bond_type_fraenkel->setPalette(PALET_NORMAL);
  AUTFBACK(cb_chem_pb_pb_bond_type_fraenkel);

  cb_chem_pb_pb_bond_type_hookean = new QCheckBox(bg_chem_pb_pb_bond_types);
  cb_chem_pb_pb_bond_type_hookean->setText(us_tr(" Hookean,Gaussian (soft) "));
  cb_chem_pb_pb_bond_type_hookean->setEnabled(true);
  //   cb_chem_pb_pb_bond_type_hookean->setMinimumHeight(minHeight1);
  cb_chem_pb_pb_bond_type_hookean->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_chem_pb_pb_bond_type_hookean->setPalette(PALET_NORMAL);
  AUTFBACK(cb_chem_pb_pb_bond_type_hookean);

  cb_chem_pb_pb_bond_type_fene = new QCheckBox(bg_chem_pb_pb_bond_types);
  cb_chem_pb_pb_bond_type_fene->setText(us_tr(" FENE "));
  cb_chem_pb_pb_bond_type_fene->setEnabled(true);
  //   cb_chem_pb_pb_bond_type_fene->setMinimumHeight(minHeight1);
  cb_chem_pb_pb_bond_type_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_chem_pb_pb_bond_type_fene->setPalette(PALET_NORMAL);
  AUTFBACK(cb_chem_pb_pb_bond_type_fene);

  cb_chem_pb_pb_bond_type_hard_fene = new QCheckBox(bg_chem_pb_pb_bond_types);
  cb_chem_pb_pb_bond_type_hard_fene->setText(us_tr(" Hard-FENE "));
  cb_chem_pb_pb_bond_type_hard_fene->setEnabled(true);
  //   cb_chem_pb_pb_bond_type_hard_fene->setMinimumHeight(minHeight1);
  cb_chem_pb_pb_bond_type_hard_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_chem_pb_pb_bond_type_hard_fene->setPalette(PALET_NORMAL);
  AUTFBACK(cb_chem_pb_pb_bond_type_hard_fene);

  bg_chem_pb_pb_bond_types->setButton(bd_options->chem_pb_pb_bond_type);
#else
  bg_chem_pb_pb_bond_types = new QGroupBox("Bond type:");

  rb_chem_pb_pb_bond_type_fraenkel = new QRadioButton();
  rb_chem_pb_pb_bond_type_fraenkel->setText(us_tr(" Fraenkel (hard Hookean) "));
  rb_chem_pb_pb_bond_type_fraenkel->setEnabled(true);
  //   rb_chem_pb_pb_bond_type_fraenkel->setMinimumHeight(minHeight1);
  rb_chem_pb_pb_bond_type_fraenkel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_chem_pb_pb_bond_type_fraenkel->setPalette(PALET_NORMAL);
  AUTFBACK(rb_chem_pb_pb_bond_type_fraenkel);
  connect(rb_chem_pb_pb_bond_type_fraenkel, SIGNAL(clicked()), this,
          SLOT(set_chem_pb_pb_bond_types()));

  rb_chem_pb_pb_bond_type_hookean = new QRadioButton();
  rb_chem_pb_pb_bond_type_hookean->setText(us_tr(" Hookean,Gaussian (soft) "));
  rb_chem_pb_pb_bond_type_hookean->setEnabled(true);
  //   rb_chem_pb_pb_bond_type_hookean->setMinimumHeight(minHeight1);
  rb_chem_pb_pb_bond_type_hookean->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_chem_pb_pb_bond_type_hookean->setPalette(PALET_NORMAL);
  AUTFBACK(rb_chem_pb_pb_bond_type_hookean);
  connect(rb_chem_pb_pb_bond_type_hookean, SIGNAL(clicked()), this,
          SLOT(set_chem_pb_pb_bond_types()));

  rb_chem_pb_pb_bond_type_fene = new QRadioButton();
  rb_chem_pb_pb_bond_type_fene->setText(us_tr(" FENE "));
  rb_chem_pb_pb_bond_type_fene->setEnabled(true);
  //   rb_chem_pb_pb_bond_type_fene->setMinimumHeight(minHeight1);
  rb_chem_pb_pb_bond_type_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_chem_pb_pb_bond_type_fene->setPalette(PALET_NORMAL);
  AUTFBACK(rb_chem_pb_pb_bond_type_fene);
  connect(rb_chem_pb_pb_bond_type_fene, SIGNAL(clicked()), this,
          SLOT(set_chem_pb_pb_bond_types()));

  rb_chem_pb_pb_bond_type_hard_fene = new QRadioButton();
  rb_chem_pb_pb_bond_type_hard_fene->setText(us_tr(" Hard-FENE "));
  rb_chem_pb_pb_bond_type_hard_fene->setEnabled(true);
  //   rb_chem_pb_pb_bond_type_hard_fene->setMinimumHeight(minHeight1);
  rb_chem_pb_pb_bond_type_hard_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_chem_pb_pb_bond_type_hard_fene->setPalette(PALET_NORMAL);
  AUTFBACK(rb_chem_pb_pb_bond_type_hard_fene);
  connect(rb_chem_pb_pb_bond_type_hard_fene, SIGNAL(clicked()), this,
          SLOT(set_chem_pb_pb_bond_types()));

  {
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(rb_chem_pb_pb_bond_type_fraenkel);
    vbox->addWidget(rb_chem_pb_pb_bond_type_hookean);
    vbox->addWidget(rb_chem_pb_pb_bond_type_fene);
    vbox->addWidget(rb_chem_pb_pb_bond_type_hard_fene);
    bg_chem_pb_pb_bond_types->setLayout(vbox);
  }

  switch (bd_options->chem_pb_pb_bond_type) {
    case 0:
      rb_chem_pb_pb_bond_type_fraenkel->setChecked(true);
      break;
    case 1:
      rb_chem_pb_pb_bond_type_hookean->setChecked(true);
      break;
    case 2:
      rb_chem_pb_pb_bond_type_fene->setChecked(true);
      break;
    case 3:
      rb_chem_pb_pb_bond_type_hard_fene->setChecked(true);
      break;
    default:
      qDebug() << "bd options chem_pb_pb_bond_type selection error";
      break;
  }

#endif

#if QT_VERSION < 0x040000
  bg_chem_pb_sc_bond_types = new QGroupBox(4, Qt::Vertical, "Bond type:", this);
  qf = bg_chem_pb_sc_bond_types->font();
  qf.setPointSize(qf.pointSize() - 1);
  bg_chem_pb_sc_bond_types->setFont(qf);
  bg_chem_pb_sc_bond_types->setExclusive(true);
  bg_chem_pb_sc_bond_types->setAlignment(Qt::AlignHCenter);
  bg_chem_pb_sc_bond_types->setInsideMargin(3);
  bg_chem_pb_sc_bond_types->setInsideSpacing(0);
  connect(bg_chem_pb_sc_bond_types, SIGNAL(clicked(int)), this,
          SLOT(set_chem_pb_sc_bond_types(int)));

  cb_chem_pb_sc_bond_type_fraenkel = new QCheckBox(bg_chem_pb_sc_bond_types);
  cb_chem_pb_sc_bond_type_fraenkel->setText(us_tr(" Fraenkel (hard Hookean) "));
  cb_chem_pb_sc_bond_type_fraenkel->setEnabled(true);
  //   cb_chem_pb_sc_bond_type_fraenkel->setMinimumHeight(minHeight1);
  cb_chem_pb_sc_bond_type_fraenkel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_chem_pb_sc_bond_type_fraenkel->setPalette(PALET_NORMAL);
  AUTFBACK(cb_chem_pb_sc_bond_type_fraenkel);

  cb_chem_pb_sc_bond_type_hookean = new QCheckBox(bg_chem_pb_sc_bond_types);
  cb_chem_pb_sc_bond_type_hookean->setText(us_tr(" Hookean,Gaussian (soft) "));
  cb_chem_pb_sc_bond_type_hookean->setEnabled(true);
  //   cb_chem_pb_sc_bond_type_hookean->setMinimumHeight(minHeight1);
  cb_chem_pb_sc_bond_type_hookean->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_chem_pb_sc_bond_type_hookean->setPalette(PALET_NORMAL);
  AUTFBACK(cb_chem_pb_sc_bond_type_hookean);

  cb_chem_pb_sc_bond_type_fene = new QCheckBox(bg_chem_pb_sc_bond_types);
  cb_chem_pb_sc_bond_type_fene->setText(us_tr(" FENE "));
  cb_chem_pb_sc_bond_type_fene->setEnabled(true);
  //   cb_chem_pb_sc_bond_type_fene->setMinimumHeight(minHeight1);
  cb_chem_pb_sc_bond_type_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_chem_pb_sc_bond_type_fene->setPalette(PALET_NORMAL);
  AUTFBACK(cb_chem_pb_sc_bond_type_fene);

  cb_chem_pb_sc_bond_type_hard_fene = new QCheckBox(bg_chem_pb_sc_bond_types);
  cb_chem_pb_sc_bond_type_hard_fene->setText(us_tr(" Hard-FENE "));
  cb_chem_pb_sc_bond_type_hard_fene->setEnabled(true);
  //   cb_chem_pb_sc_bond_type_hard_fene->setMinimumHeight(minHeight1);
  cb_chem_pb_sc_bond_type_hard_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_chem_pb_sc_bond_type_hard_fene->setPalette(PALET_NORMAL);
  AUTFBACK(cb_chem_pb_sc_bond_type_hard_fene);

  bg_chem_pb_sc_bond_types->setButton(bd_options->chem_pb_sc_bond_type);
#else
  bg_chem_pb_sc_bond_types = new QGroupBox("Bond type:");

  rb_chem_pb_sc_bond_type_fraenkel = new QRadioButton();
  rb_chem_pb_sc_bond_type_fraenkel->setText(us_tr(" Fraenkel (hard Hookean) "));
  rb_chem_pb_sc_bond_type_fraenkel->setEnabled(true);
  //   rb_chem_pb_sc_bond_type_fraenkel->setMinimumHeight(minHeight1);
  rb_chem_pb_sc_bond_type_fraenkel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_chem_pb_sc_bond_type_fraenkel->setPalette(PALET_NORMAL);
  AUTFBACK(rb_chem_pb_sc_bond_type_fraenkel);
  connect(rb_chem_pb_sc_bond_type_fraenkel, SIGNAL(clicked()), this,
          SLOT(set_chem_pb_sc_bond_types()));

  rb_chem_pb_sc_bond_type_hookean = new QRadioButton();
  rb_chem_pb_sc_bond_type_hookean->setText(us_tr(" Hookean,Gaussian (soft) "));
  rb_chem_pb_sc_bond_type_hookean->setEnabled(true);
  //   rb_chem_pb_sc_bond_type_hookean->setMinimumHeight(minHeight1);
  rb_chem_pb_sc_bond_type_hookean->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_chem_pb_sc_bond_type_hookean->setPalette(PALET_NORMAL);
  AUTFBACK(rb_chem_pb_sc_bond_type_hookean);
  connect(rb_chem_pb_sc_bond_type_hookean, SIGNAL(clicked()), this,
          SLOT(set_chem_pb_sc_bond_types()));

  rb_chem_pb_sc_bond_type_fene = new QRadioButton();
  rb_chem_pb_sc_bond_type_fene->setText(us_tr(" FENE "));
  rb_chem_pb_sc_bond_type_fene->setEnabled(true);
  //   rb_chem_pb_sc_bond_type_fene->setMinimumHeight(minHeight1);
  rb_chem_pb_sc_bond_type_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_chem_pb_sc_bond_type_fene->setPalette(PALET_NORMAL);
  AUTFBACK(rb_chem_pb_sc_bond_type_fene);
  connect(rb_chem_pb_sc_bond_type_fene, SIGNAL(clicked()), this,
          SLOT(set_chem_pb_sc_bond_types()));

  rb_chem_pb_sc_bond_type_hard_fene = new QRadioButton();
  rb_chem_pb_sc_bond_type_hard_fene->setText(us_tr(" Hard-FENE "));
  rb_chem_pb_sc_bond_type_hard_fene->setEnabled(true);
  //   rb_chem_pb_sc_bond_type_hard_fene->setMinimumHeight(minHeight1);
  rb_chem_pb_sc_bond_type_hard_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_chem_pb_sc_bond_type_hard_fene->setPalette(PALET_NORMAL);
  AUTFBACK(rb_chem_pb_sc_bond_type_hard_fene);
  connect(rb_chem_pb_sc_bond_type_hard_fene, SIGNAL(clicked()), this,
          SLOT(set_chem_pb_sc_bond_types()));

  {
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(rb_chem_pb_sc_bond_type_fraenkel);
    vbox->addWidget(rb_chem_pb_sc_bond_type_hookean);
    vbox->addWidget(rb_chem_pb_sc_bond_type_fene);
    vbox->addWidget(rb_chem_pb_sc_bond_type_hard_fene);
    bg_chem_pb_sc_bond_types->setLayout(vbox);
  }

  switch (bd_options->chem_pb_sc_bond_type) {
    case 0:
      rb_chem_pb_sc_bond_type_fraenkel->setChecked(true);
      break;
    case 1:
      rb_chem_pb_sc_bond_type_hookean->setChecked(true);
      break;
    case 2:
      rb_chem_pb_sc_bond_type_fene->setChecked(true);
      break;
    case 3:
      rb_chem_pb_sc_bond_type_hard_fene->setChecked(true);
      break;
    default:
      qDebug() << "bd options chem_pb_sc_bond_type selection error";
      break;
  }
#endif

#if QT_VERSION < 0x040000
  bg_chem_sc_sc_bond_types = new QGroupBox(4, Qt::Vertical, "Bond type:", this);
  qf = bg_chem_sc_sc_bond_types->font();
  qf.setPointSize(qf.pointSize() - 1);
  bg_chem_sc_sc_bond_types->setFont(qf);
  bg_chem_sc_sc_bond_types->setExclusive(true);
  bg_chem_sc_sc_bond_types->setAlignment(Qt::AlignHCenter);
  bg_chem_sc_sc_bond_types->setInsideMargin(3);
  bg_chem_sc_sc_bond_types->setInsideSpacing(0);
  connect(bg_chem_sc_sc_bond_types, SIGNAL(clicked(int)), this,
          SLOT(set_chem_sc_sc_bond_types(int)));

  cb_chem_sc_sc_bond_type_fraenkel = new QCheckBox(bg_chem_sc_sc_bond_types);
  cb_chem_sc_sc_bond_type_fraenkel->setText(us_tr(" Fraenkel (hard Hookean) "));
  cb_chem_sc_sc_bond_type_fraenkel->setEnabled(true);
  //   cb_chem_sc_sc_bond_type_fraenkel->setMinimumHeight(minHeight1);
  cb_chem_sc_sc_bond_type_fraenkel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_chem_sc_sc_bond_type_fraenkel->setPalette(PALET_NORMAL);
  AUTFBACK(cb_chem_sc_sc_bond_type_fraenkel);

  cb_chem_sc_sc_bond_type_hookean = new QCheckBox(bg_chem_sc_sc_bond_types);
  cb_chem_sc_sc_bond_type_hookean->setText(us_tr(" Hookean,Gaussian (soft) "));
  cb_chem_sc_sc_bond_type_hookean->setEnabled(true);
  //   cb_chem_sc_sc_bond_type_hookean->setMinimumHeight(minHeight1);
  cb_chem_sc_sc_bond_type_hookean->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_chem_sc_sc_bond_type_hookean->setPalette(PALET_NORMAL);
  AUTFBACK(cb_chem_sc_sc_bond_type_hookean);

  cb_chem_sc_sc_bond_type_fene = new QCheckBox(bg_chem_sc_sc_bond_types);
  cb_chem_sc_sc_bond_type_fene->setText(us_tr(" FENE "));
  cb_chem_sc_sc_bond_type_fene->setEnabled(true);
  //   cb_chem_sc_sc_bond_type_fene->setMinimumHeight(minHeight1);
  cb_chem_sc_sc_bond_type_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_chem_sc_sc_bond_type_fene->setPalette(PALET_NORMAL);
  AUTFBACK(cb_chem_sc_sc_bond_type_fene);

  cb_chem_sc_sc_bond_type_hard_fene = new QCheckBox(bg_chem_sc_sc_bond_types);
  cb_chem_sc_sc_bond_type_hard_fene->setText(us_tr(" Hard-FENE "));
  cb_chem_sc_sc_bond_type_hard_fene->setEnabled(true);
  //   cb_chem_sc_sc_bond_type_hard_fene->setMinimumHeight(minHeight1);
  cb_chem_sc_sc_bond_type_hard_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_chem_sc_sc_bond_type_hard_fene->setPalette(PALET_NORMAL);
  AUTFBACK(cb_chem_sc_sc_bond_type_hard_fene);

  bg_chem_sc_sc_bond_types->setButton(bd_options->chem_sc_sc_bond_type);
#else
  bg_chem_sc_sc_bond_types = new QGroupBox("Bond type:");

  rb_chem_sc_sc_bond_type_fraenkel = new QRadioButton();
  rb_chem_sc_sc_bond_type_fraenkel->setText(us_tr(" Fraenkel (hard Hookean) "));
  rb_chem_sc_sc_bond_type_fraenkel->setEnabled(true);
  //   rb_chem_sc_sc_bond_type_fraenkel->setMinimumHeight(minHeight1);
  rb_chem_sc_sc_bond_type_fraenkel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_chem_sc_sc_bond_type_fraenkel->setPalette(PALET_NORMAL);
  AUTFBACK(rb_chem_sc_sc_bond_type_fraenkel);
  connect(rb_chem_sc_sc_bond_type_fraenkel, SIGNAL(clicked()), this,
          SLOT(set_chem_sc_sc_bond_types()));

  rb_chem_sc_sc_bond_type_hookean = new QRadioButton();
  rb_chem_sc_sc_bond_type_hookean->setText(us_tr(" Hookean,Gaussian (soft) "));
  rb_chem_sc_sc_bond_type_hookean->setEnabled(true);
  //   rb_chem_sc_sc_bond_type_hookean->setMinimumHeight(minHeight1);
  rb_chem_sc_sc_bond_type_hookean->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_chem_sc_sc_bond_type_hookean->setPalette(PALET_NORMAL);
  AUTFBACK(rb_chem_sc_sc_bond_type_hookean);
  connect(rb_chem_sc_sc_bond_type_hookean, SIGNAL(clicked()), this,
          SLOT(set_chem_sc_sc_bond_types()));

  rb_chem_sc_sc_bond_type_fene = new QRadioButton();
  rb_chem_sc_sc_bond_type_fene->setText(us_tr(" FENE "));
  rb_chem_sc_sc_bond_type_fene->setEnabled(true);
  //   rb_chem_sc_sc_bond_type_fene->setMinimumHeight(minHeight1);
  rb_chem_sc_sc_bond_type_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_chem_sc_sc_bond_type_fene->setPalette(PALET_NORMAL);
  AUTFBACK(rb_chem_sc_sc_bond_type_fene);
  connect(rb_chem_sc_sc_bond_type_fene, SIGNAL(clicked()), this,
          SLOT(set_chem_sc_sc_bond_types()));

  rb_chem_sc_sc_bond_type_hard_fene = new QRadioButton();
  rb_chem_sc_sc_bond_type_hard_fene->setText(us_tr(" Hard-FENE "));
  rb_chem_sc_sc_bond_type_hard_fene->setEnabled(true);
  //   rb_chem_sc_sc_bond_type_hard_fene->setMinimumHeight(minHeight1);
  rb_chem_sc_sc_bond_type_hard_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_chem_sc_sc_bond_type_hard_fene->setPalette(PALET_NORMAL);
  AUTFBACK(rb_chem_sc_sc_bond_type_hard_fene);
  connect(rb_chem_sc_sc_bond_type_hard_fene, SIGNAL(clicked()), this,
          SLOT(set_chem_sc_sc_bond_types()));

  {
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(rb_chem_sc_sc_bond_type_fraenkel);
    vbox->addWidget(rb_chem_sc_sc_bond_type_hookean);
    vbox->addWidget(rb_chem_sc_sc_bond_type_fene);
    vbox->addWidget(rb_chem_sc_sc_bond_type_hard_fene);
    bg_chem_sc_sc_bond_types->setLayout(vbox);
  }

  switch (bd_options->chem_sc_sc_bond_type) {
    case 0:
      rb_chem_sc_sc_bond_type_fraenkel->setChecked(true);
      break;
    case 1:
      rb_chem_sc_sc_bond_type_hookean->setChecked(true);
      break;
    case 2:
      rb_chem_sc_sc_bond_type_fene->setChecked(true);
      break;
    case 3:
      rb_chem_sc_sc_bond_type_hard_fene->setChecked(true);
      break;
    default:
      qDebug() << "bd options chem_sc_sc_bond_type selection error";
      break;
  }

#endif

#if QT_VERSION < 0x040000
  bg_pb_pb_bond_types = new QGroupBox(4, Qt::Vertical, "Bond type:", this);
  qf = bg_pb_pb_bond_types->font();
  qf.setPointSize(qf.pointSize() - 1);
  bg_pb_pb_bond_types->setFont(qf);
  bg_pb_pb_bond_types->setExclusive(true);
  bg_pb_pb_bond_types->setAlignment(Qt::AlignHCenter);
  bg_pb_pb_bond_types->setInsideMargin(3);
  bg_pb_pb_bond_types->setInsideSpacing(0);
  connect(bg_pb_pb_bond_types, SIGNAL(clicked(int)), this,
          SLOT(set_pb_pb_bond_types(int)));

  cb_pb_pb_bond_type_fraenkel = new QCheckBox(bg_pb_pb_bond_types);
  cb_pb_pb_bond_type_fraenkel->setText(us_tr(" Fraenkel (hard Hookean) "));
  cb_pb_pb_bond_type_fraenkel->setEnabled(true);
  //   cb_pb_pb_bond_type_fraenkel->setMinimumHeight(minHeight1);
  cb_pb_pb_bond_type_fraenkel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_pb_pb_bond_type_fraenkel->setPalette(PALET_NORMAL);
  AUTFBACK(cb_pb_pb_bond_type_fraenkel);

  cb_pb_pb_bond_type_hookean = new QCheckBox(bg_pb_pb_bond_types);
  cb_pb_pb_bond_type_hookean->setText(us_tr(" Hookean,Gaussian (soft) "));
  cb_pb_pb_bond_type_hookean->setEnabled(true);
  //   cb_pb_pb_bond_type_hookean->setMinimumHeight(minHeight1);
  cb_pb_pb_bond_type_hookean->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_pb_pb_bond_type_hookean->setPalette(PALET_NORMAL);
  AUTFBACK(cb_pb_pb_bond_type_hookean);

  cb_pb_pb_bond_type_fene = new QCheckBox(bg_pb_pb_bond_types);
  cb_pb_pb_bond_type_fene->setText(us_tr(" FENE "));
  cb_pb_pb_bond_type_fene->setEnabled(true);
  //   cb_pb_pb_bond_type_fene->setMinimumHeight(minHeight1);
  cb_pb_pb_bond_type_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_pb_pb_bond_type_fene->setPalette(PALET_NORMAL);
  AUTFBACK(cb_pb_pb_bond_type_fene);

  cb_pb_pb_bond_type_hard_fene = new QCheckBox(bg_pb_pb_bond_types);
  cb_pb_pb_bond_type_hard_fene->setText(us_tr(" Hard-FENE "));
  cb_pb_pb_bond_type_hard_fene->setEnabled(true);
  //   cb_pb_pb_bond_type_hard_fene->setMinimumHeight(minHeight1);
  cb_pb_pb_bond_type_hard_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_pb_pb_bond_type_hard_fene->setPalette(PALET_NORMAL);
  AUTFBACK(cb_pb_pb_bond_type_hard_fene);

  bg_pb_pb_bond_types->setButton(bd_options->pb_pb_bond_type);
#else
  bg_pb_pb_bond_types = new QGroupBox("Bond type:");

  rb_pb_pb_bond_type_fraenkel = new QRadioButton();
  rb_pb_pb_bond_type_fraenkel->setText(us_tr(" Fraenkel (hard Hookean) "));
  rb_pb_pb_bond_type_fraenkel->setEnabled(true);
  //   rb_pb_pb_bond_type_fraenkel->setMinimumHeight(minHeight1);
  rb_pb_pb_bond_type_fraenkel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_pb_pb_bond_type_fraenkel->setPalette(PALET_NORMAL);
  AUTFBACK(rb_pb_pb_bond_type_fraenkel);
  connect(rb_pb_pb_bond_type_fraenkel, SIGNAL(clicked()), this,
          SLOT(set_pb_pb_bond_types()));

  rb_pb_pb_bond_type_hookean = new QRadioButton();
  rb_pb_pb_bond_type_hookean->setText(us_tr(" Hookean,Gaussian (soft) "));
  rb_pb_pb_bond_type_hookean->setEnabled(true);
  //   rb_pb_pb_bond_type_hookean->setMinimumHeight(minHeight1);
  rb_pb_pb_bond_type_hookean->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_pb_pb_bond_type_hookean->setPalette(PALET_NORMAL);
  AUTFBACK(rb_pb_pb_bond_type_hookean);
  connect(rb_pb_pb_bond_type_hookean, SIGNAL(clicked()), this,
          SLOT(set_pb_pb_bond_types()));

  rb_pb_pb_bond_type_fene = new QRadioButton();
  rb_pb_pb_bond_type_fene->setText(us_tr(" FENE "));
  rb_pb_pb_bond_type_fene->setEnabled(true);
  //   rb_pb_pb_bond_type_fene->setMinimumHeight(minHeight1);
  rb_pb_pb_bond_type_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_pb_pb_bond_type_fene->setPalette(PALET_NORMAL);
  AUTFBACK(rb_pb_pb_bond_type_fene);
  connect(rb_pb_pb_bond_type_fene, SIGNAL(clicked()), this,
          SLOT(set_pb_pb_bond_types()));

  rb_pb_pb_bond_type_hard_fene = new QRadioButton();
  rb_pb_pb_bond_type_hard_fene->setText(us_tr(" Hard-FENE "));
  rb_pb_pb_bond_type_hard_fene->setEnabled(true);
  //   rb_pb_pb_bond_type_hard_fene->setMinimumHeight(minHeight1);
  rb_pb_pb_bond_type_hard_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_pb_pb_bond_type_hard_fene->setPalette(PALET_NORMAL);
  AUTFBACK(rb_pb_pb_bond_type_hard_fene);
  connect(rb_pb_pb_bond_type_hard_fene, SIGNAL(clicked()), this,
          SLOT(set_pb_pb_bond_types()));

  {
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(rb_pb_pb_bond_type_fraenkel);
    vbox->addWidget(rb_pb_pb_bond_type_hookean);
    vbox->addWidget(rb_pb_pb_bond_type_fene);
    vbox->addWidget(rb_pb_pb_bond_type_hard_fene);
    bg_pb_pb_bond_types->setLayout(vbox);
  }

  switch (bd_options->chem_pb_pb_bond_type) {
    case 0:
      rb_pb_pb_bond_type_fraenkel->setChecked(true);
      break;
    case 1:
      rb_pb_pb_bond_type_hookean->setChecked(true);
      break;
    case 2:
      rb_pb_pb_bond_type_fene->setChecked(true);
      break;
    case 3:
      rb_pb_pb_bond_type_hard_fene->setChecked(true);
      break;
    default:
      qDebug() << "bd options chem_pb_pb_bond_type selection error";
      break;
  }
#endif

#if QT_VERSION < 0x040000
  bg_pb_sc_bond_types = new QGroupBox(4, Qt::Vertical, "Bond type:", this);
  qf = bg_pb_sc_bond_types->font();
  qf.setPointSize(qf.pointSize() - 1);
  bg_pb_sc_bond_types->setFont(qf);
  bg_pb_sc_bond_types->setExclusive(true);
  bg_pb_sc_bond_types->setAlignment(Qt::AlignHCenter);
  bg_pb_sc_bond_types->setInsideMargin(3);
  bg_pb_sc_bond_types->setInsideSpacing(0);
  connect(bg_pb_sc_bond_types, SIGNAL(clicked(int)), this,
          SLOT(set_pb_sc_bond_types(int)));

  cb_pb_sc_bond_type_fraenkel = new QCheckBox(bg_pb_sc_bond_types);
  cb_pb_sc_bond_type_fraenkel->setText(us_tr(" Fraenkel (hard Hookean) "));
  cb_pb_sc_bond_type_fraenkel->setEnabled(true);
  //   cb_pb_sc_bond_type_fraenkel->setMinimumHeight(minHeight1);
  cb_pb_sc_bond_type_fraenkel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_pb_sc_bond_type_fraenkel->setPalette(PALET_NORMAL);
  AUTFBACK(cb_pb_sc_bond_type_fraenkel);

  cb_pb_sc_bond_type_hookean = new QCheckBox(bg_pb_sc_bond_types);
  cb_pb_sc_bond_type_hookean->setText(us_tr(" Hookean,Gaussian (soft) "));
  cb_pb_sc_bond_type_hookean->setEnabled(true);
  //   cb_pb_sc_bond_type_hookean->setMinimumHeight(minHeight1);
  cb_pb_sc_bond_type_hookean->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_pb_sc_bond_type_hookean->setPalette(PALET_NORMAL);
  AUTFBACK(cb_pb_sc_bond_type_hookean);

  cb_pb_sc_bond_type_fene = new QCheckBox(bg_pb_sc_bond_types);
  cb_pb_sc_bond_type_fene->setText(us_tr(" FENE "));
  cb_pb_sc_bond_type_fene->setEnabled(true);
  //   cb_pb_sc_bond_type_fene->setMinimumHeight(minHeight1);
  cb_pb_sc_bond_type_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_pb_sc_bond_type_fene->setPalette(PALET_NORMAL);
  AUTFBACK(cb_pb_sc_bond_type_fene);

  cb_pb_sc_bond_type_hard_fene = new QCheckBox(bg_pb_sc_bond_types);
  cb_pb_sc_bond_type_hard_fene->setText(us_tr(" Hard-FENE "));
  cb_pb_sc_bond_type_hard_fene->setEnabled(true);
  //   cb_pb_sc_bond_type_hard_fene->setMinimumHeight(minHeight1);
  cb_pb_sc_bond_type_hard_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_pb_sc_bond_type_hard_fene->setPalette(PALET_NORMAL);
  AUTFBACK(cb_pb_sc_bond_type_hard_fene);

  bg_pb_sc_bond_types->setButton(bd_options->pb_sc_bond_type);
#else
  bg_pb_sc_bond_types = new QGroupBox("Bond type:");

  rb_pb_sc_bond_type_fraenkel = new QRadioButton();
  rb_pb_sc_bond_type_fraenkel->setText(us_tr(" Fraenkel (hard Hookean) "));
  rb_pb_sc_bond_type_fraenkel->setEnabled(true);
  //   rb_pb_sc_bond_type_fraenkel->setMinimumHeight(minHeight1);
  rb_pb_sc_bond_type_fraenkel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_pb_sc_bond_type_fraenkel->setPalette(PALET_NORMAL);
  AUTFBACK(rb_pb_sc_bond_type_fraenkel);
  connect(rb_pb_sc_bond_type_fraenkel, SIGNAL(clicked()), this,
          SLOT(set_pb_sc_bond_types()));

  rb_pb_sc_bond_type_hookean = new QRadioButton();
  rb_pb_sc_bond_type_hookean->setText(us_tr(" Hookean,Gaussian (soft) "));
  rb_pb_sc_bond_type_hookean->setEnabled(true);
  //   rb_pb_sc_bond_type_hookean->setMinimumHeight(minHeight1);
  rb_pb_sc_bond_type_hookean->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_pb_sc_bond_type_hookean->setPalette(PALET_NORMAL);
  AUTFBACK(rb_pb_sc_bond_type_hookean);
  connect(rb_pb_sc_bond_type_hookean, SIGNAL(clicked()), this,
          SLOT(set_pb_sc_bond_types()));

  rb_pb_sc_bond_type_fene = new QRadioButton();
  rb_pb_sc_bond_type_fene->setText(us_tr(" FENE "));
  rb_pb_sc_bond_type_fene->setEnabled(true);
  //   rb_pb_sc_bond_type_fene->setMinimumHeight(minHeight1);
  rb_pb_sc_bond_type_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_pb_sc_bond_type_fene->setPalette(PALET_NORMAL);
  AUTFBACK(rb_pb_sc_bond_type_fene);
  connect(rb_pb_sc_bond_type_fene, SIGNAL(clicked()), this,
          SLOT(set_pb_sc_bond_types()));

  rb_pb_sc_bond_type_hard_fene = new QRadioButton();
  rb_pb_sc_bond_type_hard_fene->setText(us_tr(" Hard-FENE "));
  rb_pb_sc_bond_type_hard_fene->setEnabled(true);
  //   rb_pb_sc_bond_type_hard_fene->setMinimumHeight(minHeight1);
  rb_pb_sc_bond_type_hard_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_pb_sc_bond_type_hard_fene->setPalette(PALET_NORMAL);
  AUTFBACK(rb_pb_sc_bond_type_hard_fene);
  connect(rb_pb_sc_bond_type_hard_fene, SIGNAL(clicked()), this,
          SLOT(set_pb_sc_bond_types()));

  {
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(rb_pb_sc_bond_type_fraenkel);
    vbox->addWidget(rb_pb_sc_bond_type_hookean);
    vbox->addWidget(rb_pb_sc_bond_type_fene);
    vbox->addWidget(rb_pb_sc_bond_type_hard_fene);
    bg_pb_sc_bond_types->setLayout(vbox);
  }

  switch (bd_options->chem_pb_sc_bond_type) {
    case 0:
      rb_pb_sc_bond_type_fraenkel->setChecked(true);
      break;
    case 1:
      rb_pb_sc_bond_type_hookean->setChecked(true);
      break;
    case 2:
      rb_pb_sc_bond_type_fene->setChecked(true);
      break;
    case 3:
      rb_pb_sc_bond_type_hard_fene->setChecked(true);
      break;
    default:
      qDebug() << "bd options chem_pb_sc_bond_type selection error";
      break;
  }
#endif

#if QT_VERSION < 0x040000
  bg_sc_sc_bond_types = new QGroupBox(4, Qt::Vertical, "Bond type:", this);
  qf = bg_sc_sc_bond_types->font();
  qf.setPointSize(qf.pointSize() - 1);
  bg_sc_sc_bond_types->setFont(qf);
  bg_sc_sc_bond_types->setExclusive(true);
  bg_sc_sc_bond_types->setAlignment(Qt::AlignHCenter);
  bg_sc_sc_bond_types->setInsideMargin(3);
  bg_sc_sc_bond_types->setInsideSpacing(0);
  connect(bg_sc_sc_bond_types, SIGNAL(clicked(int)), this,
          SLOT(set_sc_sc_bond_types(int)));

  cb_sc_sc_bond_type_fraenkel = new QCheckBox(bg_sc_sc_bond_types);
  cb_sc_sc_bond_type_fraenkel->setText(us_tr(" Fraenkel (hard Hookean) "));
  cb_sc_sc_bond_type_fraenkel->setEnabled(true);
  //   cb_sc_sc_bond_type_fraenkel->setMinimumHeight(minHeight1);
  cb_sc_sc_bond_type_fraenkel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_sc_sc_bond_type_fraenkel->setPalette(PALET_NORMAL);
  AUTFBACK(cb_sc_sc_bond_type_fraenkel);

  cb_sc_sc_bond_type_hookean = new QCheckBox(bg_sc_sc_bond_types);
  cb_sc_sc_bond_type_hookean->setText(us_tr(" Hookean,Gaussian (soft) "));
  cb_sc_sc_bond_type_hookean->setEnabled(true);
  //   cb_sc_sc_bond_type_hookean->setMinimumHeight(minHeight1);
  cb_sc_sc_bond_type_hookean->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_sc_sc_bond_type_hookean->setPalette(PALET_NORMAL);
  AUTFBACK(cb_sc_sc_bond_type_hookean);

  cb_sc_sc_bond_type_fene = new QCheckBox(bg_sc_sc_bond_types);
  cb_sc_sc_bond_type_fene->setText(us_tr(" FENE "));
  cb_sc_sc_bond_type_fene->setEnabled(true);
  //   cb_sc_sc_bond_type_fene->setMinimumHeight(minHeight1);
  cb_sc_sc_bond_type_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_sc_sc_bond_type_fene->setPalette(PALET_NORMAL);
  AUTFBACK(cb_sc_sc_bond_type_fene);

  cb_sc_sc_bond_type_hard_fene = new QCheckBox(bg_sc_sc_bond_types);
  cb_sc_sc_bond_type_hard_fene->setText(us_tr(" Hard-FENE "));
  cb_sc_sc_bond_type_hard_fene->setEnabled(true);
  //   cb_sc_sc_bond_type_hard_fene->setMinimumHeight(minHeight1);
  cb_sc_sc_bond_type_hard_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_sc_sc_bond_type_hard_fene->setPalette(PALET_NORMAL);
  AUTFBACK(cb_sc_sc_bond_type_hard_fene);

  bg_sc_sc_bond_types->setButton(bd_options->sc_sc_bond_type);
#else
  bg_sc_sc_bond_types = new QGroupBox("Bond type:");

  rb_sc_sc_bond_type_fraenkel = new QRadioButton();
  rb_sc_sc_bond_type_fraenkel->setText(us_tr(" Fraenkel (hard Hookean) "));
  rb_sc_sc_bond_type_fraenkel->setEnabled(true);
  //   rb_sc_sc_bond_type_fraenkel->setMinimumHeight(minHeight1);
  rb_sc_sc_bond_type_fraenkel->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_sc_sc_bond_type_fraenkel->setPalette(PALET_NORMAL);
  AUTFBACK(rb_sc_sc_bond_type_fraenkel);
  connect(rb_sc_sc_bond_type_fraenkel, SIGNAL(clicked()), this,
          SLOT(set_sc_sc_bond_types()));

  rb_sc_sc_bond_type_hookean = new QRadioButton();
  rb_sc_sc_bond_type_hookean->setText(us_tr(" Hookean,Gaussian (soft) "));
  rb_sc_sc_bond_type_hookean->setEnabled(true);
  //   rb_sc_sc_bond_type_hookean->setMinimumHeight(minHeight1);
  rb_sc_sc_bond_type_hookean->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_sc_sc_bond_type_hookean->setPalette(PALET_NORMAL);
  AUTFBACK(rb_sc_sc_bond_type_hookean);
  connect(rb_sc_sc_bond_type_hookean, SIGNAL(clicked()), this,
          SLOT(set_sc_sc_bond_types()));

  rb_sc_sc_bond_type_fene = new QRadioButton();
  rb_sc_sc_bond_type_fene->setText(us_tr(" FENE "));
  rb_sc_sc_bond_type_fene->setEnabled(true);
  //   rb_sc_sc_bond_type_fene->setMinimumHeight(minHeight1);
  rb_sc_sc_bond_type_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_sc_sc_bond_type_fene->setPalette(PALET_NORMAL);
  AUTFBACK(rb_sc_sc_bond_type_fene);
  connect(rb_sc_sc_bond_type_fene, SIGNAL(clicked()), this,
          SLOT(set_sc_sc_bond_types()));

  rb_sc_sc_bond_type_hard_fene = new QRadioButton();
  rb_sc_sc_bond_type_hard_fene->setText(us_tr(" Hard-FENE "));
  rb_sc_sc_bond_type_hard_fene->setEnabled(true);
  //   rb_sc_sc_bond_type_hard_fene->setMinimumHeight(minHeight1);
  rb_sc_sc_bond_type_hard_fene->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_sc_sc_bond_type_hard_fene->setPalette(PALET_NORMAL);
  AUTFBACK(rb_sc_sc_bond_type_hard_fene);
  connect(rb_sc_sc_bond_type_hard_fene, SIGNAL(clicked()), this,
          SLOT(set_sc_sc_bond_types()));

  {
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(rb_sc_sc_bond_type_fraenkel);
    vbox->addWidget(rb_sc_sc_bond_type_hookean);
    vbox->addWidget(rb_sc_sc_bond_type_fene);
    vbox->addWidget(rb_sc_sc_bond_type_hard_fene);
    bg_sc_sc_bond_types->setLayout(vbox);
  }

  switch (bd_options->chem_sc_sc_bond_type) {
    case 0:
      rb_sc_sc_bond_type_fraenkel->setChecked(true);
      break;
    case 1:
      rb_sc_sc_bond_type_hookean->setChecked(true);
      break;
    case 2:
      rb_sc_sc_bond_type_fene->setChecked(true);
      break;
    case 3:
      rb_sc_sc_bond_type_hard_fene->setChecked(true);
      break;
    default:
      qDebug() << "bd options chem_sc_sc_bond_type selection error";
      break;
  }
#endif

  lbl_chem_pb_pb_force_constant =
      new QLabel(us_tr(" Hookean spring constant: "), this);
  lbl_chem_pb_pb_force_constant->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_chem_pb_pb_force_constant->setMinimumHeight(minHeight1);
  lbl_chem_pb_pb_force_constant->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chem_pb_pb_force_constant);
  lbl_chem_pb_pb_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily,
            USglobal->config_list.fontSize - 1, QFont::Bold));

  le_chem_pb_pb_force_constant = new QLineEdit(this);
  le_chem_pb_pb_force_constant->setObjectName(
      "Chem_Pb_Pb_Force_Constant Line Edit");
  le_chem_pb_pb_force_constant->setText(
      str.sprintf("%4.2f", (*bd_options).chem_pb_pb_force_constant));
  le_chem_pb_pb_force_constant->setAlignment(Qt::AlignVCenter);
  le_chem_pb_pb_force_constant->setPalette(PALET_NORMAL);
  AUTFBACK(le_chem_pb_pb_force_constant);
  le_chem_pb_pb_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_chem_pb_pb_force_constant->setEnabled(true);
  connect(le_chem_pb_pb_force_constant, SIGNAL(textChanged(const QString &)),
          SLOT(update_chem_pb_pb_force_constant(const QString &)));

  lbl_chem_pb_pb_equilibrium_dist =
      new QLabel(us_tr(" Equilibrium distance: "), this);
  lbl_chem_pb_pb_equilibrium_dist->setAlignment(Qt::AlignLeft |
                                                Qt::AlignVCenter);
  lbl_chem_pb_pb_equilibrium_dist->setMinimumHeight(minHeight1);
  lbl_chem_pb_pb_equilibrium_dist->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chem_pb_pb_equilibrium_dist);
  lbl_chem_pb_pb_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily,
            USglobal->config_list.fontSize - 1, QFont::Bold));

  le_chem_pb_pb_equilibrium_dist = new QLineEdit(this);
  le_chem_pb_pb_equilibrium_dist->setObjectName(
      "Chem_Pb_Pb_Equilibrium_Dist Line Edit");
  le_chem_pb_pb_equilibrium_dist->setText(
      str.sprintf("%4.2f", (*bd_options).chem_pb_pb_equilibrium_dist));
  le_chem_pb_pb_equilibrium_dist->setAlignment(Qt::AlignVCenter);
  le_chem_pb_pb_equilibrium_dist->setPalette(PALET_NORMAL);
  AUTFBACK(le_chem_pb_pb_equilibrium_dist);
  le_chem_pb_pb_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_chem_pb_pb_equilibrium_dist->setEnabled(true);
  connect(le_chem_pb_pb_equilibrium_dist, SIGNAL(textChanged(const QString &)),
          SLOT(update_chem_pb_pb_equilibrium_dist(const QString &)));

  lbl_chem_pb_pb_max_elong = new QLabel(us_tr(" Maximum elongation: "), this);
  lbl_chem_pb_pb_max_elong->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_chem_pb_pb_max_elong->setMinimumHeight(minHeight1);
  lbl_chem_pb_pb_max_elong->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chem_pb_pb_max_elong);
  lbl_chem_pb_pb_max_elong->setFont(QFont(USglobal->config_list.fontFamily,
                                          USglobal->config_list.fontSize - 1,
                                          QFont::Bold));

  le_chem_pb_pb_max_elong = new QLineEdit(this);
  le_chem_pb_pb_max_elong->setObjectName("Chem_Pb_Pb_Max_Elong Line Edit");
  le_chem_pb_pb_max_elong->setText(
      str.sprintf("%4.2f", (*bd_options).chem_pb_pb_max_elong));
  le_chem_pb_pb_max_elong->setAlignment(Qt::AlignVCenter);
  le_chem_pb_pb_max_elong->setPalette(PALET_NORMAL);
  AUTFBACK(le_chem_pb_pb_max_elong);
  le_chem_pb_pb_max_elong->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_chem_pb_pb_max_elong->setEnabled(true);
  connect(le_chem_pb_pb_max_elong, SIGNAL(textChanged(const QString &)),
          SLOT(update_chem_pb_pb_max_elong(const QString &)));

  lbl_chem_pb_sc_force_constant =
      new QLabel(us_tr(" Hookean spring constant: "), this);
  lbl_chem_pb_sc_force_constant->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_chem_pb_sc_force_constant->setMinimumHeight(minHeight1);
  lbl_chem_pb_sc_force_constant->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chem_pb_sc_force_constant);
  lbl_chem_pb_sc_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily,
            USglobal->config_list.fontSize - 1, QFont::Bold));

  le_chem_pb_sc_force_constant = new QLineEdit(this);
  le_chem_pb_sc_force_constant->setObjectName(
      "Chem_Pb_Sc_Force_Constant Line Edit");
  le_chem_pb_sc_force_constant->setText(
      str.sprintf("%4.2f", (*bd_options).chem_pb_sc_force_constant));
  le_chem_pb_sc_force_constant->setAlignment(Qt::AlignVCenter);
  le_chem_pb_sc_force_constant->setPalette(PALET_NORMAL);
  AUTFBACK(le_chem_pb_sc_force_constant);
  le_chem_pb_sc_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_chem_pb_sc_force_constant->setEnabled(true);
  connect(le_chem_pb_sc_force_constant, SIGNAL(textChanged(const QString &)),
          SLOT(update_chem_pb_sc_force_constant(const QString &)));

  lbl_chem_pb_sc_equilibrium_dist =
      new QLabel(us_tr(" Equilibrium distance: "), this);
  lbl_chem_pb_sc_equilibrium_dist->setAlignment(Qt::AlignLeft |
                                                Qt::AlignVCenter);
  lbl_chem_pb_sc_equilibrium_dist->setMinimumHeight(minHeight1);
  lbl_chem_pb_sc_equilibrium_dist->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chem_pb_sc_equilibrium_dist);
  lbl_chem_pb_sc_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily,
            USglobal->config_list.fontSize - 1, QFont::Bold));

  le_chem_pb_sc_equilibrium_dist = new QLineEdit(this);
  le_chem_pb_sc_equilibrium_dist->setObjectName(
      "Chem_Pb_Sc_Equilibrium_Dist Line Edit");
  le_chem_pb_sc_equilibrium_dist->setText(
      str.sprintf("%4.2f", (*bd_options).chem_pb_sc_equilibrium_dist));
  le_chem_pb_sc_equilibrium_dist->setAlignment(Qt::AlignVCenter);
  le_chem_pb_sc_equilibrium_dist->setPalette(PALET_NORMAL);
  AUTFBACK(le_chem_pb_sc_equilibrium_dist);
  le_chem_pb_sc_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_chem_pb_sc_equilibrium_dist->setEnabled(true);
  connect(le_chem_pb_sc_equilibrium_dist, SIGNAL(textChanged(const QString &)),
          SLOT(update_chem_pb_sc_equilibrium_dist(const QString &)));

  lbl_chem_pb_sc_max_elong = new QLabel(us_tr(" Maximum elongation: "), this);
  lbl_chem_pb_sc_max_elong->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_chem_pb_sc_max_elong->setMinimumHeight(minHeight1);
  lbl_chem_pb_sc_max_elong->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chem_pb_sc_max_elong);
  lbl_chem_pb_sc_max_elong->setFont(QFont(USglobal->config_list.fontFamily,
                                          USglobal->config_list.fontSize - 1,
                                          QFont::Bold));

  le_chem_pb_sc_max_elong = new QLineEdit(this);
  le_chem_pb_sc_max_elong->setObjectName("Chem_Pb_Sc_Max_Elong Line Edit");
  le_chem_pb_sc_max_elong->setText(
      str.sprintf("%4.2f", (*bd_options).chem_pb_sc_max_elong));
  le_chem_pb_sc_max_elong->setAlignment(Qt::AlignVCenter);
  le_chem_pb_sc_max_elong->setPalette(PALET_NORMAL);
  AUTFBACK(le_chem_pb_sc_max_elong);
  le_chem_pb_sc_max_elong->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_chem_pb_sc_max_elong->setEnabled(true);
  connect(le_chem_pb_sc_max_elong, SIGNAL(textChanged(const QString &)),
          SLOT(update_chem_pb_sc_max_elong(const QString &)));

  lbl_chem_sc_sc_force_constant =
      new QLabel(us_tr(" Hookean spring constant: "), this);
  lbl_chem_sc_sc_force_constant->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_chem_sc_sc_force_constant->setMinimumHeight(minHeight1);
  lbl_chem_sc_sc_force_constant->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chem_sc_sc_force_constant);
  lbl_chem_sc_sc_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily,
            USglobal->config_list.fontSize - 1, QFont::Bold));

  le_chem_sc_sc_force_constant = new QLineEdit(this);
  le_chem_sc_sc_force_constant->setObjectName(
      "Chem_Sc_Sc_Force_Constant Line Edit");
  le_chem_sc_sc_force_constant->setText(
      str.sprintf("%4.2f", (*bd_options).chem_sc_sc_force_constant));
  le_chem_sc_sc_force_constant->setAlignment(Qt::AlignVCenter);
  le_chem_sc_sc_force_constant->setPalette(PALET_NORMAL);
  AUTFBACK(le_chem_sc_sc_force_constant);
  le_chem_sc_sc_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_chem_sc_sc_force_constant->setEnabled(true);
  connect(le_chem_sc_sc_force_constant, SIGNAL(textChanged(const QString &)),
          SLOT(update_chem_sc_sc_force_constant(const QString &)));

  lbl_chem_sc_sc_equilibrium_dist =
      new QLabel(us_tr(" Equilibrium distance: "), this);
  lbl_chem_sc_sc_equilibrium_dist->setAlignment(Qt::AlignLeft |
                                                Qt::AlignVCenter);
  lbl_chem_sc_sc_equilibrium_dist->setMinimumHeight(minHeight1);
  lbl_chem_sc_sc_equilibrium_dist->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chem_sc_sc_equilibrium_dist);
  lbl_chem_sc_sc_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily,
            USglobal->config_list.fontSize - 1, QFont::Bold));

  le_chem_sc_sc_equilibrium_dist = new QLineEdit(this);
  le_chem_sc_sc_equilibrium_dist->setObjectName(
      "Chem_Sc_Sc_Equilibrium_Dist Line Edit");
  le_chem_sc_sc_equilibrium_dist->setText(
      str.sprintf("%4.2f", (*bd_options).chem_sc_sc_equilibrium_dist));
  le_chem_sc_sc_equilibrium_dist->setAlignment(Qt::AlignVCenter);
  le_chem_sc_sc_equilibrium_dist->setPalette(PALET_NORMAL);
  AUTFBACK(le_chem_sc_sc_equilibrium_dist);
  le_chem_sc_sc_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_chem_sc_sc_equilibrium_dist->setEnabled(true);
  connect(le_chem_sc_sc_equilibrium_dist, SIGNAL(textChanged(const QString &)),
          SLOT(update_chem_sc_sc_equilibrium_dist(const QString &)));

  lbl_chem_sc_sc_max_elong = new QLabel(us_tr(" Maximum elongation: "), this);
  lbl_chem_sc_sc_max_elong->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_chem_sc_sc_max_elong->setMinimumHeight(minHeight1);
  lbl_chem_sc_sc_max_elong->setPalette(PALET_LABEL);
  AUTFBACK(lbl_chem_sc_sc_max_elong);
  lbl_chem_sc_sc_max_elong->setFont(QFont(USglobal->config_list.fontFamily,
                                          USglobal->config_list.fontSize - 1,
                                          QFont::Bold));

  le_chem_sc_sc_max_elong = new QLineEdit(this);
  le_chem_sc_sc_max_elong->setObjectName("Chem_Sc_Sc_Max_Elong Line Edit");
  le_chem_sc_sc_max_elong->setText(
      str.sprintf("%4.2f", (*bd_options).chem_sc_sc_max_elong));
  le_chem_sc_sc_max_elong->setAlignment(Qt::AlignVCenter);
  le_chem_sc_sc_max_elong->setPalette(PALET_NORMAL);
  AUTFBACK(le_chem_sc_sc_max_elong);
  le_chem_sc_sc_max_elong->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_chem_sc_sc_max_elong->setEnabled(true);
  connect(le_chem_sc_sc_max_elong, SIGNAL(textChanged(const QString &)),
          SLOT(update_chem_sc_sc_max_elong(const QString &)));

  lbl_pb_pb_force_constant =
      new QLabel(us_tr(" Hookean spring constant: "), this);
  lbl_pb_pb_force_constant->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_pb_pb_force_constant->setMinimumHeight(minHeight1);
  lbl_pb_pb_force_constant->setPalette(PALET_LABEL);
  AUTFBACK(lbl_pb_pb_force_constant);
  lbl_pb_pb_force_constant->setFont(QFont(USglobal->config_list.fontFamily,
                                          USglobal->config_list.fontSize - 1,
                                          QFont::Bold));

  le_pb_pb_force_constant = new QLineEdit(this);
  le_pb_pb_force_constant->setObjectName("Pb_Pb_Force_Constant Line Edit");
  le_pb_pb_force_constant->setText(
      str.sprintf("%4.2f", (*bd_options).pb_pb_force_constant));
  le_pb_pb_force_constant->setAlignment(Qt::AlignVCenter);
  le_pb_pb_force_constant->setPalette(PALET_NORMAL);
  AUTFBACK(le_pb_pb_force_constant);
  le_pb_pb_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_pb_pb_force_constant->setEnabled(true);
  connect(le_pb_pb_force_constant, SIGNAL(textChanged(const QString &)),
          SLOT(update_pb_pb_force_constant(const QString &)));

  lbl_pb_pb_equilibrium_dist =
      new QLabel(us_tr(" Equilibrium distance: "), this);
  lbl_pb_pb_equilibrium_dist->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_pb_pb_equilibrium_dist->setMinimumHeight(minHeight1);
  lbl_pb_pb_equilibrium_dist->setPalette(PALET_LABEL);
  AUTFBACK(lbl_pb_pb_equilibrium_dist);
  lbl_pb_pb_equilibrium_dist->setFont(QFont(USglobal->config_list.fontFamily,
                                            USglobal->config_list.fontSize - 1,
                                            QFont::Bold));

  le_pb_pb_equilibrium_dist = new QLineEdit(this);
  le_pb_pb_equilibrium_dist->setObjectName("Pb_Pb_Equilibrium_Dist Line Edit");
  le_pb_pb_equilibrium_dist->setText(
      str.sprintf("%4.2f", (*bd_options).pb_pb_equilibrium_dist));
  le_pb_pb_equilibrium_dist->setAlignment(Qt::AlignVCenter);
  le_pb_pb_equilibrium_dist->setPalette(PALET_NORMAL);
  AUTFBACK(le_pb_pb_equilibrium_dist);
  le_pb_pb_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_pb_pb_equilibrium_dist->setEnabled(true);
  connect(le_pb_pb_equilibrium_dist, SIGNAL(textChanged(const QString &)),
          SLOT(update_pb_pb_equilibrium_dist(const QString &)));

  lbl_pb_pb_max_elong = new QLabel(us_tr(" Maximum elongation: "), this);
  lbl_pb_pb_max_elong->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_pb_pb_max_elong->setMinimumHeight(minHeight1);
  lbl_pb_pb_max_elong->setPalette(PALET_LABEL);
  AUTFBACK(lbl_pb_pb_max_elong);
  lbl_pb_pb_max_elong->setFont(QFont(USglobal->config_list.fontFamily,
                                     USglobal->config_list.fontSize - 1,
                                     QFont::Bold));

  le_pb_pb_max_elong = new QLineEdit(this);
  le_pb_pb_max_elong->setObjectName("Pb_Pb_Max_Elong Line Edit");
  le_pb_pb_max_elong->setText(
      str.sprintf("%4.2f", (*bd_options).pb_pb_max_elong));
  le_pb_pb_max_elong->setAlignment(Qt::AlignVCenter);
  le_pb_pb_max_elong->setPalette(PALET_NORMAL);
  AUTFBACK(le_pb_pb_max_elong);
  le_pb_pb_max_elong->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_pb_pb_max_elong->setEnabled(true);
  connect(le_pb_pb_max_elong, SIGNAL(textChanged(const QString &)),
          SLOT(update_pb_pb_max_elong(const QString &)));

  lbl_pb_sc_force_constant =
      new QLabel(us_tr(" Hookean spring constant: "), this);
  lbl_pb_sc_force_constant->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_pb_sc_force_constant->setMinimumHeight(minHeight1);
  lbl_pb_sc_force_constant->setPalette(PALET_LABEL);
  AUTFBACK(lbl_pb_sc_force_constant);
  lbl_pb_sc_force_constant->setFont(QFont(USglobal->config_list.fontFamily,
                                          USglobal->config_list.fontSize - 1,
                                          QFont::Bold));

  le_pb_sc_force_constant = new QLineEdit(this);
  le_pb_sc_force_constant->setObjectName("Pb_Sc_Force_Constant Line Edit");
  le_pb_sc_force_constant->setText(
      str.sprintf("%4.2f", (*bd_options).pb_sc_force_constant));
  le_pb_sc_force_constant->setAlignment(Qt::AlignVCenter);
  le_pb_sc_force_constant->setPalette(PALET_NORMAL);
  AUTFBACK(le_pb_sc_force_constant);
  le_pb_sc_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_pb_sc_force_constant->setEnabled(true);
  connect(le_pb_sc_force_constant, SIGNAL(textChanged(const QString &)),
          SLOT(update_pb_sc_force_constant(const QString &)));

  lbl_pb_sc_equilibrium_dist =
      new QLabel(us_tr(" Equilibrium distance: "), this);
  lbl_pb_sc_equilibrium_dist->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_pb_sc_equilibrium_dist->setMinimumHeight(minHeight1);
  lbl_pb_sc_equilibrium_dist->setPalette(PALET_LABEL);
  AUTFBACK(lbl_pb_sc_equilibrium_dist);
  lbl_pb_sc_equilibrium_dist->setFont(QFont(USglobal->config_list.fontFamily,
                                            USglobal->config_list.fontSize - 1,
                                            QFont::Bold));

  le_pb_sc_equilibrium_dist = new QLineEdit(this);
  le_pb_sc_equilibrium_dist->setObjectName("Pb_Sc_Equilibrium_Dist Line Edit");
  le_pb_sc_equilibrium_dist->setText(
      str.sprintf("%4.2f", (*bd_options).pb_sc_equilibrium_dist));
  le_pb_sc_equilibrium_dist->setAlignment(Qt::AlignVCenter);
  le_pb_sc_equilibrium_dist->setPalette(PALET_NORMAL);
  AUTFBACK(le_pb_sc_equilibrium_dist);
  le_pb_sc_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_pb_sc_equilibrium_dist->setEnabled(true);
  connect(le_pb_sc_equilibrium_dist, SIGNAL(textChanged(const QString &)),
          SLOT(update_pb_sc_equilibrium_dist(const QString &)));

  lbl_pb_sc_max_elong = new QLabel(us_tr(" Maximum elongation: "), this);
  lbl_pb_sc_max_elong->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_pb_sc_max_elong->setMinimumHeight(minHeight1);
  lbl_pb_sc_max_elong->setPalette(PALET_LABEL);
  AUTFBACK(lbl_pb_sc_max_elong);
  lbl_pb_sc_max_elong->setFont(QFont(USglobal->config_list.fontFamily,
                                     USglobal->config_list.fontSize - 1,
                                     QFont::Bold));

  le_pb_sc_max_elong = new QLineEdit(this);
  le_pb_sc_max_elong->setObjectName("Pb_Sc_Max_Elong Line Edit");
  le_pb_sc_max_elong->setText(
      str.sprintf("%4.2f", (*bd_options).pb_sc_max_elong));
  le_pb_sc_max_elong->setAlignment(Qt::AlignVCenter);
  le_pb_sc_max_elong->setPalette(PALET_NORMAL);
  AUTFBACK(le_pb_sc_max_elong);
  le_pb_sc_max_elong->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_pb_sc_max_elong->setEnabled(true);
  connect(le_pb_sc_max_elong, SIGNAL(textChanged(const QString &)),
          SLOT(update_pb_sc_max_elong(const QString &)));

  lbl_sc_sc_force_constant =
      new QLabel(us_tr(" Hookean spring constant: "), this);
  lbl_sc_sc_force_constant->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_sc_sc_force_constant->setMinimumHeight(minHeight1);
  lbl_sc_sc_force_constant->setPalette(PALET_LABEL);
  AUTFBACK(lbl_sc_sc_force_constant);
  lbl_sc_sc_force_constant->setFont(QFont(USglobal->config_list.fontFamily,
                                          USglobal->config_list.fontSize - 1,
                                          QFont::Bold));

  le_sc_sc_force_constant = new QLineEdit(this);
  le_sc_sc_force_constant->setObjectName("Sc_Sc_Force_Constant Line Edit");
  le_sc_sc_force_constant->setText(
      str.sprintf("%4.2f", (*bd_options).sc_sc_force_constant));
  le_sc_sc_force_constant->setAlignment(Qt::AlignVCenter);
  le_sc_sc_force_constant->setPalette(PALET_NORMAL);
  AUTFBACK(le_sc_sc_force_constant);
  le_sc_sc_force_constant->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_sc_sc_force_constant->setEnabled(true);
  connect(le_sc_sc_force_constant, SIGNAL(textChanged(const QString &)),
          SLOT(update_sc_sc_force_constant(const QString &)));

  lbl_sc_sc_equilibrium_dist =
      new QLabel(us_tr(" Equilibrium distance: "), this);
  lbl_sc_sc_equilibrium_dist->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_sc_sc_equilibrium_dist->setMinimumHeight(minHeight1);
  lbl_sc_sc_equilibrium_dist->setPalette(PALET_LABEL);
  AUTFBACK(lbl_sc_sc_equilibrium_dist);
  lbl_sc_sc_equilibrium_dist->setFont(QFont(USglobal->config_list.fontFamily,
                                            USglobal->config_list.fontSize - 1,
                                            QFont::Bold));

  le_sc_sc_equilibrium_dist = new QLineEdit(this);
  le_sc_sc_equilibrium_dist->setObjectName("Sc_Sc_Equilibrium_Dist Line Edit");
  le_sc_sc_equilibrium_dist->setText(
      str.sprintf("%4.2f", (*bd_options).sc_sc_equilibrium_dist));
  le_sc_sc_equilibrium_dist->setAlignment(Qt::AlignVCenter);
  le_sc_sc_equilibrium_dist->setPalette(PALET_NORMAL);
  AUTFBACK(le_sc_sc_equilibrium_dist);
  le_sc_sc_equilibrium_dist->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_sc_sc_equilibrium_dist->setEnabled(true);
  connect(le_sc_sc_equilibrium_dist, SIGNAL(textChanged(const QString &)),
          SLOT(update_sc_sc_equilibrium_dist(const QString &)));

  lbl_sc_sc_max_elong = new QLabel(us_tr(" Maximum elongation: "), this);
  lbl_sc_sc_max_elong->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_sc_sc_max_elong->setMinimumHeight(minHeight1);
  lbl_sc_sc_max_elong->setPalette(PALET_LABEL);
  AUTFBACK(lbl_sc_sc_max_elong);
  lbl_sc_sc_max_elong->setFont(QFont(USglobal->config_list.fontFamily,
                                     USglobal->config_list.fontSize - 1,
                                     QFont::Bold));

  le_sc_sc_max_elong = new QLineEdit(this);
  le_sc_sc_max_elong->setObjectName("Sc_Sc_Max_Elong Line Edit");
  le_sc_sc_max_elong->setText(
      str.sprintf("%4.2f", (*bd_options).sc_sc_max_elong));
  le_sc_sc_max_elong->setAlignment(Qt::AlignVCenter);
  le_sc_sc_max_elong->setPalette(PALET_NORMAL);
  AUTFBACK(le_sc_sc_max_elong);
  le_sc_sc_max_elong->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_sc_sc_max_elong->setEnabled(true);
  connect(le_sc_sc_max_elong, SIGNAL(textChanged(const QString &)),
          SLOT(update_sc_sc_max_elong(const QString &)));

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

  label_font_ok = USglobal->global_colors.cg_label;
  label_font_warning = USglobal->global_colors.cg_label_warn;

  update_enables();
  update_labels();

  QVBoxLayout *vbl_top = new QVBoxLayout;
  vbl_top->setContentsMargins(0, 0, 0, 0);
  vbl_top->setSpacing(0);

  vbl_top->addWidget(lbl_info);
  vbl_top->addSpacing(3);

  QHBoxLayout *hbl_method = new QHBoxLayout;
  hbl_method->setContentsMargins(0, 0, 0, 0);
  hbl_method->setSpacing(0);

  QGridLayout *gl_thresh = new QGridLayout;
  gl_thresh->setContentsMargins(0, 0, 0, 0);
  gl_thresh->setSpacing(0);

  int j = 0;
  gl_thresh->addWidget(lbl_info_model_creation, j, 0, 1 + (j) - (j),
                       1 + (1) - (0));
  j++;
  gl_thresh->addWidget(lbl_bd_threshold_pb_pb, j, 0);
  gl_thresh->addWidget(cnt_bd_threshold_pb_pb, j, 1);
  j++;
  gl_thresh->addWidget(lbl_bd_threshold_pb_sc, j, 0);
  gl_thresh->addWidget(cnt_bd_threshold_pb_sc, j, 1);
  j++;
  gl_thresh->addWidget(lbl_bd_threshold_sc_sc, j, 0);
  gl_thresh->addWidget(cnt_bd_threshold_sc_sc, j, 1);
  j++;
  gl_thresh->addWidget(cb_do_rr, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  gl_thresh->addWidget(cb_force_chem, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  gl_thresh->addWidget(cb_show_pdb, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  gl_thresh->addWidget(cb_run_browflex, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  gl_thresh->addWidget(bg_bead_size_type, j, 0, 1 + (j + 4) - (j),
                       1 + (1) - (0));
  j++;

  hbl_method->addLayout(gl_thresh);
  hbl_method->addSpacing(3);

  QVBoxLayout *vbl_method_group = new QVBoxLayout;
  vbl_method_group->setContentsMargins(0, 0, 0, 0);
  vbl_method_group->setSpacing(0);
  vbl_method_group->addWidget(lbl_info_simulation_opts);

  QHBoxLayout *hbl_method_group = new QHBoxLayout;
  hbl_method_group->setContentsMargins(0, 0, 0, 0);
  hbl_method_group->setSpacing(0);

  QVBoxLayout *vbl_method = new QVBoxLayout;
  vbl_method->setContentsMargins(0, 0, 0, 0);
  vbl_method->setSpacing(0);
  vbl_method->addWidget(bg_inter);
  vbl_method->addSpacing(10);
  vbl_method->addWidget(bg_iorder);
  vbl_method->addSpacing(10);
  vbl_method->addWidget(cb_icdm);

  hbl_method_group->addLayout(vbl_method);
  hbl_method_group->addSpacing(3);

  QGridLayout *gl_simu = new QGridLayout;
  gl_simu->setContentsMargins(0, 0, 0, 0);
  gl_simu->setSpacing(0);
  j = 0;
  gl_simu->addWidget(lbl_nmol, j, 0);
  gl_simu->addWidget(le_nmol, j, 1);
  j++;
  gl_simu->addWidget(lbl_tprev, j, 0);
  gl_simu->addWidget(le_tprev, j, 1);
  j++;
  gl_simu->addWidget(lbl_ttraj, j, 0);
  gl_simu->addWidget(le_ttraj, j, 1);
  j++;
  gl_simu->addWidget(lbl_deltat, j, 0);
  gl_simu->addWidget(le_deltat, j, 1);
  j++;
  gl_simu->addWidget(lbl_npadif, j, 0);
  gl_simu->addWidget(cnt_npadif, j, 1);
  j++;
  gl_simu->addWidget(lbl_nconf, j, 0);
  gl_simu->addWidget(le_nconf, j, 1);
  j++;
  gl_simu->addWidget(lbl_iseed, j, 0);
  gl_simu->addWidget(le_iseed, j, 1);
  j++;
  //   gl_simu->addWidget( cb_icdm , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0
  //   ) ); j++;

  hbl_method_group->addLayout(gl_simu);

  vbl_method_group->addLayout(hbl_method_group);

  hbl_method->addLayout(vbl_method_group);

  vbl_top->addLayout(hbl_method);
  vbl_top->addSpacing(3);
  vbl_top->addWidget(lbl_springs);

  QHBoxLayout *hbl_bonds = new QHBoxLayout;
  hbl_bonds->setContentsMargins(0, 0, 0, 0);
  hbl_bonds->setSpacing(0);

  QVBoxLayout *vbl_chem_pb_pb = new QVBoxLayout;
  vbl_chem_pb_pb->setContentsMargins(0, 0, 0, 0);
  vbl_chem_pb_pb->setSpacing(0);
  vbl_chem_pb_pb->addWidget(lbl_chem_pb_pb);
  vbl_chem_pb_pb->addWidget(bg_chem_pb_pb_bond_types);
  vbl_chem_pb_pb->addWidget(lbl_chem_pb_pb_force_constant);
  QHBoxLayout *hbl_chem_pb_pb_force_constant = new QHBoxLayout;
  hbl_chem_pb_pb_force_constant->setContentsMargins(0, 0, 0, 0);
  hbl_chem_pb_pb_force_constant->setSpacing(0);
  hbl_chem_pb_pb_force_constant->addWidget(
      cb_compute_chem_pb_pb_force_constant);
  hbl_chem_pb_pb_force_constant->addWidget(pb_dup_fraenkel);
  vbl_chem_pb_pb->addLayout(hbl_chem_pb_pb_force_constant);
  vbl_chem_pb_pb->addWidget(le_chem_pb_pb_force_constant);
  vbl_chem_pb_pb->addWidget(lbl_chem_pb_pb_equilibrium_dist);
  vbl_chem_pb_pb->addWidget(cb_compute_chem_pb_pb_equilibrium_dist);
  vbl_chem_pb_pb->addWidget(le_chem_pb_pb_equilibrium_dist);
  vbl_chem_pb_pb->addWidget(lbl_chem_pb_pb_max_elong);
  vbl_chem_pb_pb->addWidget(cb_compute_chem_pb_pb_max_elong);
  vbl_chem_pb_pb->addWidget(le_chem_pb_pb_max_elong);
  hbl_bonds->addLayout(vbl_chem_pb_pb);
  hbl_bonds->addSpacing(3);

  QVBoxLayout *vbl_chem_pb_sc = new QVBoxLayout;
  vbl_chem_pb_sc->setContentsMargins(0, 0, 0, 0);
  vbl_chem_pb_sc->setSpacing(0);
  vbl_chem_pb_sc->addWidget(lbl_chem_pb_sc);
  vbl_chem_pb_sc->addWidget(bg_chem_pb_sc_bond_types);
  vbl_chem_pb_sc->addWidget(lbl_chem_pb_sc_force_constant);
  vbl_chem_pb_sc->addWidget(cb_compute_chem_pb_sc_force_constant);
  vbl_chem_pb_sc->addWidget(le_chem_pb_sc_force_constant);
  vbl_chem_pb_sc->addWidget(lbl_chem_pb_sc_equilibrium_dist);
  vbl_chem_pb_sc->addWidget(cb_compute_chem_pb_sc_equilibrium_dist);
  vbl_chem_pb_sc->addWidget(le_chem_pb_sc_equilibrium_dist);
  vbl_chem_pb_sc->addWidget(lbl_chem_pb_sc_max_elong);
  vbl_chem_pb_sc->addWidget(cb_compute_chem_pb_sc_max_elong);
  vbl_chem_pb_sc->addWidget(le_chem_pb_sc_max_elong);
  hbl_bonds->addLayout(vbl_chem_pb_sc);
  hbl_bonds->addSpacing(3);

  QVBoxLayout *vbl_chem_sc_sc = new QVBoxLayout;
  vbl_chem_sc_sc->setContentsMargins(0, 0, 0, 0);
  vbl_chem_sc_sc->setSpacing(0);
  vbl_chem_sc_sc->addWidget(lbl_chem_sc_sc);
  vbl_chem_sc_sc->addWidget(bg_chem_sc_sc_bond_types);
  vbl_chem_sc_sc->addWidget(lbl_chem_sc_sc_force_constant);
  vbl_chem_sc_sc->addWidget(cb_compute_chem_sc_sc_force_constant);
  vbl_chem_sc_sc->addWidget(le_chem_sc_sc_force_constant);
  vbl_chem_sc_sc->addWidget(lbl_chem_sc_sc_equilibrium_dist);
  vbl_chem_sc_sc->addWidget(cb_compute_chem_sc_sc_equilibrium_dist);
  vbl_chem_sc_sc->addWidget(le_chem_sc_sc_equilibrium_dist);
  vbl_chem_sc_sc->addWidget(lbl_chem_sc_sc_max_elong);
  vbl_chem_sc_sc->addWidget(cb_compute_chem_sc_sc_max_elong);
  vbl_chem_sc_sc->addWidget(le_chem_sc_sc_max_elong);
  hbl_bonds->addLayout(vbl_chem_sc_sc);
  hbl_bonds->addSpacing(3);

  QVBoxLayout *vbl_pb_pb = new QVBoxLayout;
  vbl_pb_pb->setContentsMargins(0, 0, 0, 0);
  vbl_pb_pb->setSpacing(0);
  vbl_pb_pb->addWidget(lbl_pb_pb);
  vbl_pb_pb->addWidget(bg_pb_pb_bond_types);
  vbl_pb_pb->addWidget(lbl_pb_pb_force_constant);
  vbl_pb_pb->addWidget(cb_compute_pb_pb_force_constant);
  vbl_pb_pb->addWidget(le_pb_pb_force_constant);
  vbl_pb_pb->addWidget(lbl_pb_pb_equilibrium_dist);
  vbl_pb_pb->addWidget(cb_compute_pb_pb_equilibrium_dist);
  vbl_pb_pb->addWidget(le_pb_pb_equilibrium_dist);
  vbl_pb_pb->addWidget(lbl_pb_pb_max_elong);
  vbl_pb_pb->addWidget(cb_compute_pb_pb_max_elong);
  vbl_pb_pb->addWidget(le_pb_pb_max_elong);
  hbl_bonds->addLayout(vbl_pb_pb);
  hbl_bonds->addSpacing(3);

  QVBoxLayout *vbl_pb_sc = new QVBoxLayout;
  vbl_pb_sc->setContentsMargins(0, 0, 0, 0);
  vbl_pb_sc->setSpacing(0);
  vbl_pb_sc->addWidget(lbl_pb_sc);
  vbl_pb_sc->addWidget(bg_pb_sc_bond_types);
  vbl_pb_sc->addWidget(lbl_pb_sc_force_constant);
  vbl_pb_sc->addWidget(cb_compute_pb_sc_force_constant);
  vbl_pb_sc->addWidget(le_pb_sc_force_constant);
  vbl_pb_sc->addWidget(lbl_pb_sc_equilibrium_dist);
  vbl_pb_sc->addWidget(cb_compute_pb_sc_equilibrium_dist);
  vbl_pb_sc->addWidget(le_pb_sc_equilibrium_dist);
  vbl_pb_sc->addWidget(lbl_pb_sc_max_elong);
  vbl_pb_sc->addWidget(cb_compute_pb_sc_max_elong);
  vbl_pb_sc->addWidget(le_pb_sc_max_elong);
  hbl_bonds->addLayout(vbl_pb_sc);
  hbl_bonds->addSpacing(3);

  QVBoxLayout *vbl_sc_sc = new QVBoxLayout;
  vbl_sc_sc->setContentsMargins(0, 0, 0, 0);
  vbl_sc_sc->setSpacing(0);
  vbl_sc_sc->addWidget(lbl_sc_sc);
  vbl_sc_sc->addWidget(bg_sc_sc_bond_types);
  vbl_sc_sc->addWidget(lbl_sc_sc_force_constant);
  vbl_sc_sc->addWidget(cb_compute_sc_sc_force_constant);
  vbl_sc_sc->addWidget(le_sc_sc_force_constant);
  vbl_sc_sc->addWidget(lbl_sc_sc_equilibrium_dist);
  vbl_sc_sc->addWidget(cb_compute_sc_sc_equilibrium_dist);
  vbl_sc_sc->addWidget(le_sc_sc_equilibrium_dist);
  vbl_sc_sc->addWidget(lbl_sc_sc_max_elong);
  vbl_sc_sc->addWidget(cb_compute_sc_sc_max_elong);
  vbl_sc_sc->addWidget(le_sc_sc_max_elong);
  hbl_bonds->addLayout(vbl_sc_sc);

  vbl_top->addSpacing(3);
  vbl_top->addLayout(hbl_bonds);

  QHBoxLayout *hbl_buttons = new QHBoxLayout;
  hbl_buttons->setContentsMargins(0, 0, 0, 0);
  hbl_buttons->setSpacing(0);

  hbl_buttons->addWidget(pb_help);
  hbl_buttons->addWidget(pb_cancel);

  vbl_top->addSpacing(3);
  vbl_top->addLayout(hbl_buttons);

  QHBoxLayout *background = new QHBoxLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);
  background->addLayout(vbl_top);
}

void US_Hydrodyn_BD_Options::cancel() { close(); }

void US_Hydrodyn_BD_Options::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  online_help->show_help("manual/somo/somo_bd_options.html");
}

void US_Hydrodyn_BD_Options::closeEvent(QCloseEvent *e) {
  *bd_widget = false;
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

void US_Hydrodyn_BD_Options::update_bd_threshold_pb_pb(double val) {
  (*bd_options).threshold_pb_pb = (float)val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_bd_threshold_pb_sc(double val) {
  (*bd_options).threshold_pb_sc = (float)val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_bd_threshold_sc_sc(double val) {
  (*bd_options).threshold_sc_sc = (float)val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_do_rr() {
  (*bd_options).do_rr = cb_do_rr->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_force_chem() {
  (*bd_options).force_chem = cb_force_chem->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_icdm() {
  (*bd_options).icdm = cb_do_rr->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_show_pdb() {
  (*bd_options).show_pdb = cb_show_pdb->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_run_browflex() {
  (*bd_options).run_browflex = cb_run_browflex->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_bead_size_type() {
  if (rb_bead_size_type_1st->isChecked()) {
    return set_bead_size_type(0);
  }
  if (rb_bead_size_type_min->isChecked()) {
    return set_bead_size_type(1);
  }
  if (rb_bead_size_type_avg->isChecked()) {
    return set_bead_size_type(2);
  }
}
void US_Hydrodyn_BD_Options::set_bead_size_type(int val) {
  (*bd_options).bead_size_type = val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_pb_pb_force_constant() {
  (*bd_options).compute_chem_pb_pb_force_constant =
      cb_compute_chem_pb_pb_force_constant->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_pb_pb_equilibrium_dist() {
  (*bd_options).compute_chem_pb_pb_equilibrium_dist =
      cb_compute_chem_pb_pb_equilibrium_dist->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_pb_pb_max_elong() {
  (*bd_options).compute_chem_pb_pb_max_elong =
      cb_compute_chem_pb_pb_max_elong->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_pb_sc_force_constant() {
  (*bd_options).compute_chem_pb_sc_force_constant =
      cb_compute_chem_pb_sc_force_constant->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_pb_sc_equilibrium_dist() {
  (*bd_options).compute_chem_pb_sc_equilibrium_dist =
      cb_compute_chem_pb_sc_equilibrium_dist->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_pb_sc_max_elong() {
  (*bd_options).compute_chem_pb_sc_max_elong =
      cb_compute_chem_pb_sc_max_elong->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_sc_sc_force_constant() {
  (*bd_options).compute_chem_sc_sc_force_constant =
      cb_compute_chem_sc_sc_force_constant->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_sc_sc_equilibrium_dist() {
  (*bd_options).compute_chem_sc_sc_equilibrium_dist =
      cb_compute_chem_sc_sc_equilibrium_dist->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_sc_sc_max_elong() {
  (*bd_options).compute_chem_sc_sc_max_elong =
      cb_compute_chem_sc_sc_max_elong->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_pb_pb_force_constant() {
  (*bd_options).compute_pb_pb_force_constant =
      cb_compute_pb_pb_force_constant->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_pb_pb_equilibrium_dist() {
  (*bd_options).compute_pb_pb_equilibrium_dist =
      cb_compute_pb_pb_equilibrium_dist->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_pb_pb_max_elong() {
  (*bd_options).compute_pb_pb_max_elong =
      cb_compute_pb_pb_max_elong->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_pb_sc_force_constant() {
  (*bd_options).compute_pb_sc_force_constant =
      cb_compute_pb_sc_force_constant->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_pb_sc_equilibrium_dist() {
  (*bd_options).compute_pb_sc_equilibrium_dist =
      cb_compute_pb_sc_equilibrium_dist->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_pb_sc_max_elong() {
  (*bd_options).compute_pb_sc_max_elong =
      cb_compute_pb_sc_max_elong->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_sc_sc_force_constant() {
  (*bd_options).compute_sc_sc_force_constant =
      cb_compute_sc_sc_force_constant->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_sc_sc_equilibrium_dist() {
  (*bd_options).compute_sc_sc_equilibrium_dist =
      cb_compute_sc_sc_equilibrium_dist->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_sc_sc_max_elong() {
  (*bd_options).compute_sc_sc_max_elong =
      cb_compute_sc_sc_max_elong->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_inter() {
  if (rb_inter_no_hi->isChecked()) {
    return set_inter(0);
  }
  if (rb_inter_os->isChecked()) {
    return set_inter(1);
  }
  if (rb_inter_mos->isChecked()) {
    return set_inter(2);
  }
}

void US_Hydrodyn_BD_Options::set_inter(int val) {
  (*bd_options).inter = val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_iorder() {
  if (rb_iorder_em->isChecked()) {
    return set_iorder(0);
  }
  if (rb_iorder_igt->isChecked()) {
    return set_iorder(1);
  }
}
void US_Hydrodyn_BD_Options::set_iorder(int val) {
  (*bd_options).iorder = val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_tprev(const QString &str) {
  (*bd_options).tprev = str.toFloat();
  //   le_tprev->setText(QString("").sprintf("%4.2f",(*hydro).tprev));
  update_labels();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_ttraj(const QString &str) {
  (*bd_options).ttraj = str.toFloat();
  //   le_ttraj->setText(QString("").sprintf("%4.2f",(*hydro).ttraj));
  update_labels();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_deltat(const QString &str) {
  (*bd_options).deltat = str.toFloat();
  //   le_deltat->setText(QString("").sprintf("%4.2f",(*hydro).deltat));
  update_labels();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_npadif(double val) {
  (*bd_options).npadif = (int)val;
  update_labels();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_nconf(const QString &str) {
  (*bd_options).nconf = str.toInt();
  //   le_nconf->setText(QString("").sprintf("%4.2f",(*hydro).nconf));
  update_labels();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_nmol(const QString &str) {
  (*bd_options).nmol = str.toInt();
  //   le_nmol->setText(QString("").sprintf("%4.2f",(*hydro).nmol));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_iseed(const QString &str) {
  (*bd_options).iseed = str.toInt();
  //   le_iseed->setText(QString("").sprintf("%4.2f",(*hydro).iseed));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_chem_pb_pb_bond_types() {
  if (rb_chem_pb_pb_bond_type_fraenkel->isChecked()) {
    return set_chem_pb_pb_bond_types(0);
  }
  if (rb_chem_pb_pb_bond_type_hookean->isChecked()) {
    return set_chem_pb_pb_bond_types(1);
  }
  if (rb_chem_pb_pb_bond_type_fene->isChecked()) {
    return set_chem_pb_pb_bond_types(2);
  }
  if (rb_chem_pb_pb_bond_type_hard_fene->isChecked()) {
    return set_chem_pb_pb_bond_types(3);
  }
}

void US_Hydrodyn_BD_Options::set_chem_pb_pb_bond_types(int val) {
  (*bd_options).chem_pb_pb_bond_type = val;
  update_enables();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_chem_pb_sc_bond_types() {
  if (rb_chem_pb_sc_bond_type_fraenkel->isChecked()) {
    return set_chem_pb_sc_bond_types(0);
  }
  if (rb_chem_pb_sc_bond_type_hookean->isChecked()) {
    return set_chem_pb_sc_bond_types(1);
  }
  if (rb_chem_pb_sc_bond_type_fene->isChecked()) {
    return set_chem_pb_sc_bond_types(2);
  }
  if (rb_chem_pb_sc_bond_type_hard_fene->isChecked()) {
    return set_chem_pb_sc_bond_types(3);
  }
}

void US_Hydrodyn_BD_Options::set_chem_pb_sc_bond_types(int val) {
  (*bd_options).chem_pb_sc_bond_type = val;
  update_enables();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_chem_sc_sc_bond_types() {
  if (rb_chem_sc_sc_bond_type_fraenkel->isChecked()) {
    return set_chem_sc_sc_bond_types(0);
  }
  if (rb_chem_sc_sc_bond_type_hookean->isChecked()) {
    return set_chem_sc_sc_bond_types(1);
  }
  if (rb_chem_sc_sc_bond_type_fene->isChecked()) {
    return set_chem_sc_sc_bond_types(2);
  }
  if (rb_chem_sc_sc_bond_type_hard_fene->isChecked()) {
    return set_chem_sc_sc_bond_types(3);
  }
}

void US_Hydrodyn_BD_Options::set_chem_sc_sc_bond_types(int val) {
  (*bd_options).chem_sc_sc_bond_type = val;
  update_enables();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_pb_pb_bond_types() {
  if (rb_pb_pb_bond_type_fraenkel->isChecked()) {
    return set_pb_pb_bond_types(0);
  }
  if (rb_pb_pb_bond_type_hookean->isChecked()) {
    return set_pb_pb_bond_types(1);
  }
  if (rb_pb_pb_bond_type_fene->isChecked()) {
    return set_pb_pb_bond_types(2);
  }
  if (rb_pb_pb_bond_type_hard_fene->isChecked()) {
    return set_pb_pb_bond_types(3);
  }
}

void US_Hydrodyn_BD_Options::set_pb_pb_bond_types(int val) {
  (*bd_options).pb_pb_bond_type = val;
  update_enables();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_pb_sc_bond_types() {
  if (rb_pb_sc_bond_type_fraenkel->isChecked()) {
    return set_pb_sc_bond_types(0);
  }
  if (rb_pb_sc_bond_type_hookean->isChecked()) {
    return set_pb_sc_bond_types(1);
  }
  if (rb_pb_sc_bond_type_fene->isChecked()) {
    return set_pb_sc_bond_types(2);
  }
  if (rb_pb_sc_bond_type_hard_fene->isChecked()) {
    return set_pb_sc_bond_types(3);
  }
}

void US_Hydrodyn_BD_Options::set_pb_sc_bond_types(int val) {
  (*bd_options).pb_sc_bond_type = val;
  update_enables();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_sc_sc_bond_types() {
  if (rb_sc_sc_bond_type_fraenkel->isChecked()) {
    return set_sc_sc_bond_types(0);
  }
  if (rb_sc_sc_bond_type_hookean->isChecked()) {
    return set_sc_sc_bond_types(1);
  }
  if (rb_sc_sc_bond_type_fene->isChecked()) {
    return set_sc_sc_bond_types(2);
  }
  if (rb_sc_sc_bond_type_hard_fene->isChecked()) {
    return set_sc_sc_bond_types(3);
  }
}

void US_Hydrodyn_BD_Options::set_sc_sc_bond_types(int val) {
  (*bd_options).sc_sc_bond_type = val;
  update_enables();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_chem_pb_pb_force_constant(
    const QString &str) {
  (*bd_options).chem_pb_pb_force_constant = str.toFloat();
  //   le_chem_pb_pb_force_constant->setText(QString("").sprintf("%4.2f",(*hydro).chem_pb_pb_force_constant));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_chem_pb_pb_equilibrium_dist(
    const QString &str) {
  (*bd_options).chem_pb_pb_equilibrium_dist = str.toFloat();
  //   le_chem_pb_pb_equilibrium_dist->setText(QString("").sprintf("%4.2f",(*hydro).chem_pb_pb_equilibrium_dist));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_chem_pb_pb_max_elong(const QString &str) {
  (*bd_options).chem_pb_pb_max_elong = str.toFloat();
  //   le_chem_pb_pb_max_elong->setText(QString("").sprintf("%4.2f",(*hydro).chem_pb_pb_max_elong));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_chem_pb_sc_force_constant(
    const QString &str) {
  (*bd_options).chem_pb_sc_force_constant = str.toFloat();
  //   le_chem_pb_sc_force_constant->setText(QString("").sprintf("%4.2f",(*hydro).chem_pb_sc_force_constant));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_chem_pb_sc_equilibrium_dist(
    const QString &str) {
  (*bd_options).chem_pb_sc_equilibrium_dist = str.toFloat();
  //   le_chem_pb_sc_equilibrium_dist->setText(QString("").sprintf("%4.2f",(*hydro).chem_pb_sc_equilibrium_dist));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_chem_pb_sc_max_elong(const QString &str) {
  (*bd_options).chem_pb_sc_max_elong = str.toFloat();
  //   le_chem_pb_sc_max_elong->setText(QString("").sprintf("%4.2f",(*hydro).chem_pb_sc_max_elong));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_chem_sc_sc_force_constant(
    const QString &str) {
  (*bd_options).chem_sc_sc_force_constant = str.toFloat();
  //   le_chem_sc_sc_force_constant->setText(QString("").sprintf("%4.2f",(*hydro).chem_sc_sc_force_constant));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_chem_sc_sc_equilibrium_dist(
    const QString &str) {
  (*bd_options).chem_sc_sc_equilibrium_dist = str.toFloat();
  //   le_chem_sc_sc_equilibrium_dist->setText(QString("").sprintf("%4.2f",(*hydro).chem_sc_sc_equilibrium_dist));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_chem_sc_sc_max_elong(const QString &str) {
  (*bd_options).chem_sc_sc_max_elong = str.toFloat();
  //   le_chem_sc_sc_max_elong->setText(QString("").sprintf("%4.2f",(*hydro).chem_sc_sc_max_elong));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_pb_pb_force_constant(const QString &str) {
  (*bd_options).pb_pb_force_constant = str.toFloat();
  //   le_pb_pb_force_constant->setText(QString("").sprintf("%4.2f",(*hydro).pb_pb_force_constant));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_pb_pb_equilibrium_dist(const QString &str) {
  (*bd_options).pb_pb_equilibrium_dist = str.toFloat();
  //   le_pb_pb_equilibrium_dist->setText(QString("").sprintf("%4.2f",(*hydro).pb_pb_equilibrium_dist));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_pb_pb_max_elong(const QString &str) {
  (*bd_options).pb_pb_max_elong = str.toFloat();
  //   le_pb_pb_max_elong->setText(QString("").sprintf("%4.2f",(*hydro).pb_pb_max_elong));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_pb_sc_force_constant(const QString &str) {
  (*bd_options).pb_sc_force_constant = str.toFloat();
  //   le_pb_sc_force_constant->setText(QString("").sprintf("%4.2f",(*hydro).pb_sc_force_constant));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_pb_sc_equilibrium_dist(const QString &str) {
  (*bd_options).pb_sc_equilibrium_dist = str.toFloat();
  //   le_pb_sc_equilibrium_dist->setText(QString("").sprintf("%4.2f",(*hydro).pb_sc_equilibrium_dist));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_pb_sc_max_elong(const QString &str) {
  (*bd_options).pb_sc_max_elong = str.toFloat();
  //   le_pb_sc_max_elong->setText(QString("").sprintf("%4.2f",(*hydro).pb_sc_max_elong));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_sc_sc_force_constant(const QString &str) {
  (*bd_options).sc_sc_force_constant = str.toFloat();
  //   le_sc_sc_force_constant->setText(QString("").sprintf("%4.2f",(*hydro).sc_sc_force_constant));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_sc_sc_equilibrium_dist(const QString &str) {
  (*bd_options).sc_sc_equilibrium_dist = str.toFloat();
  //   le_sc_sc_equilibrium_dist->setText(QString("").sprintf("%4.2f",(*hydro).sc_sc_equilibrium_dist));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_sc_sc_max_elong(const QString &str) {
  (*bd_options).sc_sc_max_elong = str.toFloat();
  //   le_sc_sc_max_elong->setText(QString("").sprintf("%4.2f",(*hydro).sc_sc_max_elong));
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_enables() {
  switch (bd_options->chem_pb_pb_bond_type) {
    case 0:  // fraenkel (hard hookean)
      cb_compute_chem_pb_pb_equilibrium_dist->setEnabled(true);
      le_chem_pb_pb_equilibrium_dist->setEnabled(true);
      cb_compute_chem_pb_pb_max_elong->setEnabled(false);
      le_chem_pb_pb_max_elong->setEnabled(false);
      break;
    case 1:  // hookean, gaussian (soft)
      cb_compute_chem_pb_pb_equilibrium_dist->setEnabled(false);
      le_chem_pb_pb_equilibrium_dist->setEnabled(false);
      cb_compute_chem_pb_pb_max_elong->setEnabled(false);
      le_chem_pb_pb_max_elong->setEnabled(false);
      break;
    case 2:  // fene
      cb_compute_chem_pb_pb_equilibrium_dist->setEnabled(false);
      le_chem_pb_pb_equilibrium_dist->setEnabled(false);
      cb_compute_chem_pb_pb_max_elong->setEnabled(true);
      le_chem_pb_pb_max_elong->setEnabled(true);
      break;
    case 3:  // hard-fene
      cb_compute_chem_pb_pb_equilibrium_dist->setEnabled(true);
      le_chem_pb_pb_equilibrium_dist->setEnabled(true);
      cb_compute_chem_pb_pb_max_elong->setEnabled(true);
      le_chem_pb_pb_max_elong->setEnabled(true);
      break;
    default:
      break;
  }
  switch (bd_options->chem_pb_sc_bond_type) {
    case 0:  // fraenkel (hard hookean)
      cb_compute_chem_pb_sc_equilibrium_dist->setEnabled(true);
      le_chem_pb_sc_equilibrium_dist->setEnabled(true);
      cb_compute_chem_pb_sc_max_elong->setEnabled(false);
      le_chem_pb_sc_max_elong->setEnabled(false);
      break;
    case 1:  // hookean, gaussian (soft)
      cb_compute_chem_pb_sc_equilibrium_dist->setEnabled(false);
      le_chem_pb_sc_equilibrium_dist->setEnabled(false);
      cb_compute_chem_pb_sc_max_elong->setEnabled(false);
      le_chem_pb_sc_max_elong->setEnabled(false);
      break;
    case 2:  // fene
      cb_compute_chem_pb_sc_equilibrium_dist->setEnabled(false);
      le_chem_pb_sc_equilibrium_dist->setEnabled(false);
      cb_compute_chem_pb_sc_max_elong->setEnabled(true);
      le_chem_pb_sc_max_elong->setEnabled(true);
      break;
    case 3:  // hard-fene
      cb_compute_chem_pb_sc_equilibrium_dist->setEnabled(true);
      le_chem_pb_sc_equilibrium_dist->setEnabled(true);
      cb_compute_chem_pb_sc_max_elong->setEnabled(true);
      le_chem_pb_sc_max_elong->setEnabled(true);
      break;
    default:
      break;
  }
  switch (bd_options->chem_sc_sc_bond_type) {
    case 0:  // fraenkel (hard hookean)
      cb_compute_chem_sc_sc_equilibrium_dist->setEnabled(true);
      le_chem_sc_sc_equilibrium_dist->setEnabled(true);
      cb_compute_chem_sc_sc_max_elong->setEnabled(false);
      le_chem_sc_sc_max_elong->setEnabled(false);
      break;
    case 1:  // hookean, gaussian (soft)
      cb_compute_chem_sc_sc_equilibrium_dist->setEnabled(false);
      le_chem_sc_sc_equilibrium_dist->setEnabled(false);
      cb_compute_chem_sc_sc_max_elong->setEnabled(false);
      le_chem_sc_sc_max_elong->setEnabled(false);
      break;
    case 2:  // fene
      cb_compute_chem_sc_sc_equilibrium_dist->setEnabled(false);
      le_chem_sc_sc_equilibrium_dist->setEnabled(false);
      cb_compute_chem_sc_sc_max_elong->setEnabled(true);
      le_chem_sc_sc_max_elong->setEnabled(true);
      break;
    case 3:  // hard-fene
      cb_compute_chem_sc_sc_equilibrium_dist->setEnabled(true);
      le_chem_sc_sc_equilibrium_dist->setEnabled(true);
      cb_compute_chem_sc_sc_max_elong->setEnabled(true);
      le_chem_sc_sc_max_elong->setEnabled(true);
      break;
    default:
      break;
  }
  switch (bd_options->pb_pb_bond_type) {
    case 0:  // fraenkel (hard hookean)
      cb_compute_pb_pb_equilibrium_dist->setEnabled(true);
      le_pb_pb_equilibrium_dist->setEnabled(true);
      cb_compute_pb_pb_max_elong->setEnabled(false);
      le_pb_pb_max_elong->setEnabled(false);
      break;
    case 1:  // hookean, gaussian (soft)
      cb_compute_pb_pb_equilibrium_dist->setEnabled(false);
      le_pb_pb_equilibrium_dist->setEnabled(false);
      cb_compute_pb_pb_max_elong->setEnabled(false);
      le_pb_pb_max_elong->setEnabled(false);
      break;
    case 2:  // fene
      cb_compute_pb_pb_equilibrium_dist->setEnabled(false);
      le_pb_pb_equilibrium_dist->setEnabled(false);
      cb_compute_pb_pb_max_elong->setEnabled(true);
      le_pb_pb_max_elong->setEnabled(true);
      break;
    case 3:  // hard-fene
      cb_compute_pb_pb_equilibrium_dist->setEnabled(true);
      le_pb_pb_equilibrium_dist->setEnabled(true);
      cb_compute_pb_pb_max_elong->setEnabled(true);
      le_pb_pb_max_elong->setEnabled(true);
      break;
    default:
      break;
  }
  switch (bd_options->pb_sc_bond_type) {
    case 0:  // fraenkel (hard hookean)
      cb_compute_pb_sc_equilibrium_dist->setEnabled(true);
      le_pb_sc_equilibrium_dist->setEnabled(true);
      cb_compute_pb_sc_max_elong->setEnabled(false);
      le_pb_sc_max_elong->setEnabled(false);
      break;
    case 1:  // hookean, gaussian (soft)
      cb_compute_pb_sc_equilibrium_dist->setEnabled(false);
      le_pb_sc_equilibrium_dist->setEnabled(false);
      cb_compute_pb_sc_max_elong->setEnabled(false);
      le_pb_sc_max_elong->setEnabled(false);
      break;
    case 2:  // fene
      cb_compute_pb_sc_equilibrium_dist->setEnabled(false);
      le_pb_sc_equilibrium_dist->setEnabled(false);
      cb_compute_pb_sc_max_elong->setEnabled(true);
      le_pb_sc_max_elong->setEnabled(true);
      break;
    case 3:  // hard-fene
      cb_compute_pb_sc_equilibrium_dist->setEnabled(true);
      le_pb_sc_equilibrium_dist->setEnabled(true);
      cb_compute_pb_sc_max_elong->setEnabled(true);
      le_pb_sc_max_elong->setEnabled(true);
      break;
    default:
      break;
  }
  switch (bd_options->sc_sc_bond_type) {
    case 0:  // fraenkel (hard hookean)
      cb_compute_sc_sc_equilibrium_dist->setEnabled(true);
      le_sc_sc_equilibrium_dist->setEnabled(true);
      cb_compute_sc_sc_max_elong->setEnabled(false);
      le_sc_sc_max_elong->setEnabled(false);
      break;
    case 1:  // hookean, gaussian (soft)
      cb_compute_sc_sc_equilibrium_dist->setEnabled(false);
      le_sc_sc_equilibrium_dist->setEnabled(false);
      cb_compute_sc_sc_max_elong->setEnabled(false);
      le_sc_sc_max_elong->setEnabled(false);
      break;
    case 2:  // fene
      cb_compute_sc_sc_equilibrium_dist->setEnabled(false);
      le_sc_sc_equilibrium_dist->setEnabled(false);
      cb_compute_sc_sc_max_elong->setEnabled(true);
      le_sc_sc_max_elong->setEnabled(true);
      break;
    case 3:  // hard-fene
      cb_compute_sc_sc_equilibrium_dist->setEnabled(true);
      le_sc_sc_equilibrium_dist->setEnabled(true);
      cb_compute_sc_sc_max_elong->setEnabled(true);
      le_sc_sc_max_elong->setEnabled(true);
      break;
    default:
      break;
  }
}

void US_Hydrodyn_BD_Options::dup_fraenkel() {
  update_chem_pb_sc_force_constant(le_chem_pb_pb_force_constant->text());
  le_chem_pb_sc_force_constant->setText(le_chem_pb_pb_force_constant->text());
  cb_compute_chem_pb_sc_force_constant->setChecked(
      cb_compute_chem_pb_pb_force_constant->isChecked());
  update_chem_sc_sc_force_constant(le_chem_pb_pb_force_constant->text());
  le_chem_sc_sc_force_constant->setText(le_chem_pb_pb_force_constant->text());
  cb_compute_chem_sc_sc_force_constant->setChecked(
      cb_compute_chem_pb_pb_force_constant->isChecked());
  update_pb_pb_force_constant(le_chem_pb_pb_force_constant->text());
  le_pb_pb_force_constant->setText(le_chem_pb_pb_force_constant->text());
  cb_compute_pb_pb_force_constant->setChecked(
      cb_compute_chem_pb_pb_force_constant->isChecked());
  update_pb_sc_force_constant(le_chem_pb_pb_force_constant->text());
  le_pb_sc_force_constant->setText(le_chem_pb_pb_force_constant->text());
  cb_compute_pb_sc_force_constant->setChecked(
      cb_compute_chem_pb_pb_force_constant->isChecked());
  update_sc_sc_force_constant(le_chem_pb_pb_force_constant->text());
  le_sc_sc_force_constant->setText(le_chem_pb_pb_force_constant->text());
  cb_compute_sc_sc_force_constant->setChecked(
      cb_compute_chem_pb_pb_force_constant->isChecked());
}

#define TOLERANCE 1e-2

void US_Hydrodyn_BD_Options::update_labels() {
  double total_steps = bd_options->ttraj / bd_options->deltat;

  int digits = (int)log10(total_steps) - 5;
  digits = digits >= 0 ? digits : 0;
  if (fabs(total_steps * pow(0.1, digits) -
           (double)(int(total_steps * pow(0.1, digits) + 0.5))) < TOLERANCE) {
    cout << QString("total steps %1 int total_steps %2\n")
                .arg(total_steps)
                .arg(int(total_steps));
    cout << QString("").sprintf("%f %e %g\n", total_steps, total_steps,
                                (total_steps - int(total_steps)));
    lbl_deltat->setPalette(label_font_ok);
    lbl_deltat->setText(us_tr(" Duration of each simulation step (s): "));
  } else {
    cout << QString("total steps %1 int total_steps %2 diff %3\n")
                .arg(total_steps)
                .arg(int(total_steps))
                .arg(fabs((total_steps) - int(total_steps + 0.5)));
    cout << QString("").sprintf("%f %e %g\n", total_steps, total_steps,
                                (total_steps - int(total_steps)));
    lbl_deltat->setPalette(label_font_warning);
    lbl_deltat->setText(
        us_tr(" Duration of each simulation step (s)\n"
              " WARNING: does not divide total duration: "));
  }
  double max_conf = total_steps / bd_options->npadif;
  double check_nconf = max_conf / bd_options->nconf;
  if (fabs(check_nconf - int(check_nconf + 0.5)) < TOLERANCE) {
    cout << QString("check_nconf %1 int check_nconf %2\n")
                .arg(check_nconf)
                .arg(int(check_nconf));
    if (bd_options->nconf > max_conf) {
      lbl_nconf->setPalette(label_font_warning);
      lbl_nconf->setText(
          us_tr(QString(" Number of conformations to store (maximum %1)\n"
                        " WARNING: greater than maximum: ")
                    .arg((int)max_conf)));
    } else {
      lbl_nconf->setPalette(label_font_ok);
      lbl_nconf->setText(
          us_tr(QString(" Number of conformations to store (max %1): ")
                    .arg((int)max_conf)));
    }
  } else {
    lbl_nconf->setPalette(label_font_warning);
    lbl_nconf->setText(
        us_tr(QString(" Number of conformations to store (maximum %1)\n"
                      " WARNING: does not divide maximum")
                  .arg((int)max_conf)));
  }
}
