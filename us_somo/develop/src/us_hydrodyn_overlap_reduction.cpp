#include "../include/us_hydrodyn_overlap_reduction.h"

#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
// Added by qt3to4:
#include <QFrame>
#include <QGridLayout>
#include <QLabel>

US_Hydrodyn_OR::US_Hydrodyn_OR(struct overlap_reduction *o_r,
                               bool *replicate_o_r_method,
                               vector<void *> *other_ORs, void *us_hydrodyn,
                               QWidget *p, const char *)
    : QFrame(p) {
  this->o_r = o_r;
  this->replicate_o_r_method = replicate_o_r_method;
  this->other_ORs = other_ORs;
  this->us_hydrodyn = us_hydrodyn;
  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setupGUI();
}

US_Hydrodyn_OR::~US_Hydrodyn_OR() {}

void US_Hydrodyn_OR::setupGUI() {
  int minHeight1 = 30;

  lbl_title =
      new QLabel(us_tr("Overlap reduction between " + (*o_r).title), this);
  Q_CHECK_PTR(lbl_title);
  lbl_title->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_title->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_title->setMinimumHeight(minHeight1);
  lbl_title->setPalette(PALET_FRAME);
  AUTFBACK(lbl_title);
  lbl_title->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize + 1, QFont::Bold));

  cb_remove = new QCheckBox(this);
  cb_remove->setText(us_tr(" Remove Overlaps "));
  cb_remove->setChecked((*o_r).remove_overlap);
  cb_remove->setEnabled(true);
  cb_remove->setMinimumHeight(minHeight1);
  cb_remove->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_remove->setPalette(PALET_NORMAL);
  AUTFBACK(cb_remove);
  connect(cb_remove, SIGNAL(clicked()), SLOT(set_remove()));

  cb_replicate_method = new QCheckBox(this);
  cb_replicate_method->setText(
      us_tr(" Same overlap reduction method of all bead types"));
  cb_replicate_method->setChecked(*replicate_o_r_method);
  cb_replicate_method->setEnabled(true);
  cb_replicate_method->setMinimumHeight(minHeight1);
  cb_replicate_method->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_replicate_method->setPalette(PALET_NORMAL);
  AUTFBACK(cb_replicate_method);
  connect(cb_replicate_method, SIGNAL(clicked()), SLOT(set_replicate_method()));

  cb_fuse = new QCheckBox(this);
  cb_fuse->setText(us_tr(" Fuse Beads that overlap by more than: "));
  cb_fuse->setChecked((*o_r).fuse_beads);
  cb_fuse->setMinimumHeight(minHeight1);
  cb_fuse->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_fuse->setPalette(PALET_NORMAL);
  AUTFBACK(cb_fuse);
  connect(cb_fuse, SIGNAL(clicked()), SLOT(set_fuse()));

  cnt_fuse = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_fuse);
  Q_CHECK_PTR(cnt_fuse);
  cnt_fuse->setRange(0, 100);
  cnt_fuse->setSingleStep(0.1);
  cnt_fuse->setValue((*o_r).fuse_beads_percent);
  cnt_fuse->setMinimumHeight(minHeight1);
  cnt_fuse->setNumButtons(3);
  cnt_fuse->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_fuse->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_fuse);
  connect(cnt_fuse, SIGNAL(valueChanged(double)), SLOT(update_fuse(double)));

  lbl_steps =
      new QLabel(us_tr(" Overlap Reduction\n Step Size (in %): "), this);
  Q_CHECK_PTR(lbl_steps);
  lbl_steps->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_steps->setMinimumHeight(minHeight1 + 20);
  lbl_steps->setPalette(PALET_LABEL);
  AUTFBACK(lbl_steps);
  lbl_steps->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize - 1, QFont::Bold));

  cb_hierarch = new QCheckBox(this);
  cb_hierarch->setText(
      us_tr(" Remove Overlaps hierarchically (larger -> smaller) "));
  cb_hierarch->setChecked((*o_r).remove_hierarch);
  cb_hierarch->setEnabled((*o_r).remove_overlap);
  cb_hierarch->setMinimumHeight(minHeight1);
  cb_hierarch->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_hierarch->setPalette(PALET_NORMAL);
  AUTFBACK(cb_hierarch);
  connect(cb_hierarch, SIGNAL(clicked()), SLOT(set_hierarch()));

  cnt_hierarch = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_hierarch);
  Q_CHECK_PTR(cnt_hierarch);
  cnt_hierarch->setRange(0, 100);
  cnt_hierarch->setSingleStep(0.1);
  cnt_hierarch->setValue((*o_r).remove_hierarch_percent);
  cnt_hierarch->setMinimumHeight(minHeight1);
  cnt_hierarch->setEnabled((*o_r).remove_overlap);
  cnt_hierarch->setNumButtons(3);
  cnt_hierarch->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_hierarch->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_hierarch);
  connect(cnt_hierarch, SIGNAL(valueChanged(double)),
          SLOT(update_hierarch(double)));

  cb_sync = new QCheckBox(this);
  cb_sync->setText(us_tr(" Remove Overlaps synchronously: "));
  cb_sync->setChecked((*o_r).remove_sync);
  cb_sync->setEnabled((*o_r).remove_overlap);
  cb_sync->setMinimumHeight(minHeight1);
  cb_sync->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_sync->setPalette(PALET_NORMAL);
  AUTFBACK(cb_sync);
  connect(cb_sync, SIGNAL(clicked()), SLOT(set_sync()));

  cnt_sync = new QwtCounter(this);
  US_Hydrodyn::sizeArrows(cnt_sync);
  Q_CHECK_PTR(cnt_sync);
  cnt_sync->setRange(0, 100);
  cnt_sync->setSingleStep(0.1);
  cnt_sync->setValue((*o_r).remove_sync_percent);
  cnt_sync->setMinimumHeight(minHeight1);
  cnt_sync->setEnabled((*o_r).remove_overlap);
  cnt_sync->setNumButtons(3);
  cnt_sync->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cnt_sync->setPalette(PALET_NORMAL);
  AUTFBACK(cnt_sync);
  connect(cnt_sync, SIGNAL(valueChanged(double)), SLOT(update_sync(double)));

  if ((*o_r).show_translate) {
    cb_translate = new QCheckBox(this);
    cb_translate->setText(us_tr(" Outward Translation "));
    cb_translate->setChecked((*o_r).translate_out);
    cb_translate->setEnabled((*o_r).remove_overlap);
    cb_translate->setMinimumHeight(minHeight1);
    cb_translate->setFont(QFont(USglobal->config_list.fontFamily,
                                USglobal->config_list.fontSize));
    cb_translate->setPalette(PALET_NORMAL);
    AUTFBACK(cb_translate);
    connect(cb_translate, SIGNAL(clicked()), SLOT(set_translate()));
  }

  int /* rows=6, columns = 2,*/ spacing = 2, j = 0, margin = 2;
  QGridLayout *background = new QGridLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);
  background->setSpacing(spacing);
  background->setContentsMargins(margin, margin, margin, margin);

  background->addWidget(lbl_title, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  background->addWidget(cb_fuse, j, 0);
  background->addWidget(cnt_fuse, j, 1);
  j++;
  background->addWidget(cb_remove, j, 0);
  j++;
  background->addWidget(lbl_steps, j, 1, 1 + (j + 1) - (j), 1 + (1) - (1));
  background->addWidget(cb_replicate_method, j + 1, 0);
  j += 2;
  background->addWidget(cb_sync, j, 0);
  background->addWidget(cnt_sync, j, 1);
  j++;
  background->addWidget(cb_hierarch, j, 0);
  background->addWidget(cnt_hierarch, j, 1);
  j++;

  if ((*o_r).show_translate) {
    background->addWidget(cb_translate, j, 0);
  }
  set_remove();  // initialize all widgets to proper enabled status
}

void US_Hydrodyn_OR::set_remove() {
  (*o_r).remove_overlap = cb_remove->isChecked();
  cnt_sync->setEnabled((*o_r).remove_overlap);
  cb_sync->setEnabled((*o_r).remove_overlap);
  cnt_hierarch->setEnabled((*o_r).remove_overlap);
  cb_hierarch->setEnabled((*o_r).remove_overlap);
  if ((*o_r).show_translate) {
    cb_translate->setEnabled((*o_r).remove_overlap);
  }
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_OR::replicate() {
  if (*replicate_o_r_method) {
    for (unsigned int i = 0; i < other_ORs->size(); i++) {
      ((US_Hydrodyn_OR *)((*other_ORs)[i]))
          ->cb_hierarch->setChecked(o_r->remove_hierarch);
      ((US_Hydrodyn_OR *)((*other_ORs)[i]))
          ->cb_sync->setChecked(o_r->remove_sync);
      ((US_Hydrodyn_OR *)((*other_ORs)[i]))->o_r->remove_hierarch =
          o_r->remove_hierarch;
      ((US_Hydrodyn_OR *)((*other_ORs)[i]))->o_r->remove_sync =
          o_r->remove_sync;
    }
  }
}

void US_Hydrodyn_OR::set_replicate_method() {
  *replicate_o_r_method = cb_replicate_method->isChecked();
  for (unsigned int i = 0; i < other_ORs->size(); i++) {
    ((US_Hydrodyn_OR *)((*other_ORs)[i]))
        ->cb_replicate_method->setChecked(*replicate_o_r_method);
  }
  replicate();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_OR::set_fuse() {
  if (cb_fuse->isChecked()) {
    (*o_r).fuse_beads = true;
  } else {
    (*o_r).fuse_beads = false;
  }
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_OR::set_hierarch() {
  if (cb_hierarch->isChecked()) {
    (*o_r).remove_hierarch = true;
    (*o_r).remove_sync = false;
  } else {
    (*o_r).remove_hierarch = false;
    (*o_r).remove_sync = true;
  }
  cb_sync->disconnect();
  cb_sync->setChecked((*o_r).remove_sync);
  connect(cb_sync, SIGNAL(clicked()), SLOT(set_sync()));
  replicate();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_OR::set_sync() {
  if (cb_sync->isChecked()) {
    (*o_r).remove_sync = true;
    (*o_r).remove_hierarch = false;
  } else {
    (*o_r).remove_sync = false;
    (*o_r).remove_hierarch = true;
  }
  cb_hierarch->disconnect();
  cb_hierarch->setChecked((*o_r).remove_hierarch);
  connect(cb_hierarch, SIGNAL(clicked()), SLOT(set_hierarch()));
  replicate();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_OR::set_translate() {
  if (cb_translate->isChecked()) {
    (*o_r).translate_out = true;
  } else {
    (*o_r).translate_out = false;
  }
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_OR::update_fuse(double val) {
  (*o_r).fuse_beads_percent = val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_OR::update_sync(double val) {
  (*o_r).remove_sync_percent = val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_OR::update_hierarch(double val) {
  (*o_r).remove_hierarch_percent = val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
