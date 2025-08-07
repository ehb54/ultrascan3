#include "../include/us_hydrodyn_overlap.h"

#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
// Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>

US_Hydrodyn_Overlap::US_Hydrodyn_Overlap(
    struct overlap_reduction *sidechain_overlap,
    struct overlap_reduction *mainchain_overlap,
    struct overlap_reduction *buried_overlap, bool *replicate_o_r_method_somo,
    struct overlap_reduction *grid_exposed_overlap,
    struct overlap_reduction *grid_buried_overlap,
    struct overlap_reduction *grid_overlap, bool *replicate_o_r_method_grid,
    double *overlap_tolerance, bool *overlap_widget, void *us_hydrodyn,
    QWidget *p, const char *)
    : QFrame(p) {
  this->sidechain_overlap = sidechain_overlap;
  this->mainchain_overlap = mainchain_overlap;
  this->buried_overlap = buried_overlap;
  this->replicate_o_r_method_somo = replicate_o_r_method_somo;
  this->grid_exposed_overlap = grid_exposed_overlap;
  this->grid_buried_overlap = grid_buried_overlap;
  this->grid_overlap = grid_overlap;
  this->replicate_o_r_method_grid = replicate_o_r_method_grid;
  this->overlap_widget = overlap_widget;
  this->overlap_tolerance = overlap_tolerance;
  this->us_hydrodyn = us_hydrodyn;
  *overlap_widget = true;
  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("SoMo Bead Overlap Reduction Options"));
  show_grid_only = false;
  setupGUI();
  global_Xpos += 30;
  global_Ypos += 30;
  setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Overlap::US_Hydrodyn_Overlap(
    struct overlap_reduction *grid_exposed_overlap,
    struct overlap_reduction *grid_buried_overlap,
    struct overlap_reduction *grid_overlap, bool *replicate_o_r_method_grid,
    double *overlap_tolerance, bool *overlap_widget, void *us_hydrodyn,
    QWidget *p, const char *)
    : QFrame(p) {
  this->grid_exposed_overlap = grid_exposed_overlap;
  this->grid_buried_overlap = grid_buried_overlap;
  this->grid_overlap = grid_overlap;
  this->replicate_o_r_method_grid = replicate_o_r_method_grid;
  this->overlap_widget = overlap_widget;
  this->overlap_tolerance = overlap_tolerance;
  this->us_hydrodyn = us_hydrodyn;
  *overlap_widget = true;
  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("Grid Bead Overlap Reduction Options"));
  show_grid_only = true;
  setupGUI();
  global_Xpos += 30;
  global_Ypos += 30;
  setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Overlap::~US_Hydrodyn_Overlap() { *overlap_widget = false; }

void US_Hydrodyn_Overlap::setupGUI() {
  int minHeight1 = 30;

  if (show_grid_only) {
    other_ORs.clear();
    grid_exposed_OR =
        new US_Hydrodyn_OR(grid_exposed_overlap, replicate_o_r_method_grid,
                           &other_ORs, us_hydrodyn, this);
    other_ORs.push_back((void *)grid_exposed_OR);
    grid_buried_OR =
        new US_Hydrodyn_OR(grid_buried_overlap, replicate_o_r_method_grid,
                           &other_ORs, us_hydrodyn, this);
    other_ORs.push_back((void *)grid_buried_OR);
    grid_OR = new US_Hydrodyn_OR(grid_overlap, replicate_o_r_method_grid,
                                 &other_ORs, us_hydrodyn, this);
    other_ORs.push_back((void *)grid_OR);
    lbl_info = new QLabel(us_tr("Grid Bead Overlap Reduction Options:"), this);
  } else {
    other_ORs.clear();
    sidechain_OR =
        new US_Hydrodyn_OR(sidechain_overlap, replicate_o_r_method_somo,
                           &other_ORs, us_hydrodyn, this);
    other_ORs.push_back((void *)sidechain_OR);
    mainchain_OR =
        new US_Hydrodyn_OR(mainchain_overlap, replicate_o_r_method_somo,
                           &other_ORs, us_hydrodyn, this);
    other_ORs.push_back((void *)mainchain_OR);
    buried_OR = new US_Hydrodyn_OR(buried_overlap, replicate_o_r_method_somo,
                                   &other_ORs, us_hydrodyn, this);
    other_ORs.push_back((void *)buried_OR);
    lbl_info = new QLabel(us_tr("SoMo Bead Overlap Reduction Options:"), this);
  }
  // buried_OR->cnt_fuse->setEnabled(false);
  // buried_OR->cb_fuse->setEnabled(false);

  Q_CHECK_PTR(lbl_info);
  lbl_info->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_info->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_info->setMinimumHeight(minHeight1);
  lbl_info->setPalette(PALET_FRAME);
  AUTFBACK(lbl_info);
  lbl_info->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize + 1, QFont::Bold));

  lbl_overlap_tolerance = new QLabel(us_tr(" Bead Overlap Tolerance: "), this);
  Q_CHECK_PTR(lbl_overlap_tolerance);
  lbl_overlap_tolerance->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_overlap_tolerance->setMinimumHeight(minHeight1);
  lbl_overlap_tolerance->setPalette(PALET_LABEL);
  AUTFBACK(lbl_overlap_tolerance);
  lbl_overlap_tolerance->setFont(QFont(USglobal->config_list.fontFamily,
                                       USglobal->config_list.fontSize - 1,
                                       QFont::Bold));

  cnt_overlap_tolerance = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_overlap_tolerance);
  Q_CHECK_PTR(cnt_overlap_tolerance);
  cnt_overlap_tolerance->setRange(0, 1);
  cnt_overlap_tolerance->setSingleStep(0.0001);
  cnt_overlap_tolerance->setValue(*overlap_tolerance);
  cnt_overlap_tolerance->setMinimumHeight(minHeight1);
  cnt_overlap_tolerance->setEnabled(true);
  cnt_overlap_tolerance->setNumButtons(3);
  cnt_overlap_tolerance->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_overlap_tolerance->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_overlap_tolerance);
  connect(cnt_overlap_tolerance, SIGNAL(valueChanged(double)),
          SLOT(update_overlap_tolerance(double)));

  tw_overlap = new QTabWidget(this);
  tw_overlap->setPalette(PALET_NORMAL);
  AUTFBACK(tw_overlap);
  if (show_grid_only) {
    tw_overlap->addTab(grid_exposed_OR, "Exposed grid beads");
    tw_overlap->addTab(grid_buried_OR, "Buried grid beads");
    tw_overlap->addTab(grid_OR, "Non-screened grid beads");
  } else {
    tw_overlap->addTab(sidechain_OR, "Exposed Side chain beads");
    tw_overlap->addTab(mainchain_OR, "Exposed Main and side chain beads");
    tw_overlap->addTab(buried_OR, "Buried beads");
  }

  pb_cancel = new QPushButton(us_tr("Close"), this);
  Q_CHECK_PTR(pb_cancel);
  pb_cancel->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize + 1));
  pb_cancel->setMinimumHeight(minHeight1);
  pb_cancel->setPalette(PALET_PUSHB);
  connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

  pb_help = new QPushButton(us_tr("Help"), this);
  Q_CHECK_PTR(pb_help);
  pb_help->setFont(QFont(USglobal->config_list.fontFamily,
                         USglobal->config_list.fontSize + 1));
  pb_help->setMinimumHeight(minHeight1);
  pb_help->setPalette(PALET_PUSHB);
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));

  int /* rows=11, columns = 2,*/ spacing = 2, j = 0, margin = 4;
  QGridLayout *background = new QGridLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);
  background->setSpacing(spacing);
  background->setContentsMargins(margin, margin, margin, margin);

  background->addWidget(lbl_info, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  background->addWidget(lbl_overlap_tolerance, j, 0);
  background->addWidget(cnt_overlap_tolerance, j, 1);
  j++;
  background->addWidget(tw_overlap, j, 0, 1 + (j + 6) - (j), 1 + (1) - (0));
  j += 8;
  background->addWidget(pb_help, j, 0);
  background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_Overlap::cancel() { close(); }

void US_Hydrodyn_Overlap::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  online_help->show_help(show_grid_only ? "manual/somo/somo_grid_overlap.html"
                                        : "manual/somo/somo_overlap.html");
}

void US_Hydrodyn_Overlap::closeEvent(QCloseEvent *e) {
  *overlap_widget = false;
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

void US_Hydrodyn_Overlap::update_overlap_tolerance(double val) {
  *overlap_tolerance = val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
