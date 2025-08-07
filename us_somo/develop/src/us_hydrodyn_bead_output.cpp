#include "../include/us_hydrodyn_bead_output.h"

#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
// Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>

US_Hydrodyn_Bead_Output::US_Hydrodyn_Bead_Output(
    struct bead_output_options *bead_output, bool *bead_output_widget,
    void *us_hydrodyn, QWidget *p, const char *)
    : QFrame(p) {
  this->bead_output = bead_output;
  this->bead_output_widget = bead_output_widget;
  this->us_hydrodyn = us_hydrodyn;
  *bead_output_widget = true;
  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("SOMO Bead Model Output Options"));
  setupGUI();
  global_Xpos += 30;
  global_Ypos += 30;
  setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Bead_Output::~US_Hydrodyn_Bead_Output() {
  *bead_output_widget = false;
}

void US_Hydrodyn_Bead_Output::setupGUI() {
  int minHeight1 = 30;

  lbl_info = new QLabel(us_tr("Bead Model Output Options:"), this);
  Q_CHECK_PTR(lbl_info);
  lbl_info->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_info->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_info->setMinimumHeight(minHeight1);
  lbl_info->setPalette(PALET_FRAME);
  AUTFBACK(lbl_info);
  lbl_info->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize + 1, QFont::Bold));

#if QT_VERSION < 0x040000
  bg_output = new QGroupBox(4, Qt::Horizontal, "Output Format:", this);
  bg_output->setExclusive(false);
  connect(bg_output, SIGNAL(clicked(int)), this, SLOT(select_output(int)));

  cb_somo_output = new QCheckBox(bg_output);
  cb_somo_output->setText(us_tr(" SOMO "));
  cb_somo_output->setEnabled(true);
  cb_somo_output->setChecked((*bead_output).output & US_HYDRODYN_OUTPUT_SOMO);
  cb_somo_output->setMinimumHeight(minHeight1);
  cb_somo_output->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_somo_output->setPalette(PALET_NORMAL);
  AUTFBACK(cb_somo_output);

  cb_beams_output = new QCheckBox(bg_output);
  cb_beams_output->setText(us_tr(" BEAMS "));
  cb_beams_output->setEnabled(true);
  cb_beams_output->setChecked((*bead_output).output & US_HYDRODYN_OUTPUT_BEAMS);
  cb_beams_output->setMinimumHeight(minHeight1);
  cb_beams_output->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_beams_output->setPalette(PALET_NORMAL);
  AUTFBACK(cb_beams_output);

  cb_hydro_output = new QCheckBox(bg_output);
  cb_hydro_output->setText(us_tr(" HYDRO "));
  cb_hydro_output->setEnabled(true);
  cb_hydro_output->setChecked((*bead_output).output & US_HYDRODYN_OUTPUT_HYDRO);
  cb_hydro_output->setMinimumHeight(minHeight1);
  cb_hydro_output->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_hydro_output->setPalette(PALET_NORMAL);
  AUTFBACK(cb_hydro_output);

  cb_grpy_output = new QCheckBox(bg_output);
  cb_grpy_output->setText(us_tr(" GRPY "));
  cb_grpy_output->setEnabled(true);
  cb_grpy_output->setChecked((*bead_output).output & US_HYDRODYN_OUTPUT_GRPY);
  cb_grpy_output->setMinimumHeight(minHeight1);
  cb_grpy_output->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_grpy_output->setPalette(PALET_NORMAL);
  AUTFBACK(cb_grpy_output);
  cb_grpy_output->hide();

  // bg_output->setButton((*bead_output).output);
#else
  bg_output = new QGroupBox("Output Format:");

  cb_somo_output = new QCheckBox();
  cb_somo_output->setText(us_tr(" SOMO "));
  cb_somo_output->setEnabled(true);
  cb_somo_output->setChecked((*bead_output).output & US_HYDRODYN_OUTPUT_SOMO);
  cb_somo_output->setMinimumHeight(minHeight1);
  cb_somo_output->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_somo_output->setPalette(PALET_NORMAL);
  AUTFBACK(cb_somo_output);
  connect(cb_somo_output, SIGNAL(clicked()), this, SLOT(select_output_somo()));

  cb_beams_output = new QCheckBox();
  cb_beams_output->setText(us_tr(" BEAMS "));
  cb_beams_output->setEnabled(true);
  cb_beams_output->setChecked((*bead_output).output & US_HYDRODYN_OUTPUT_BEAMS);
  cb_beams_output->setMinimumHeight(minHeight1);
  cb_beams_output->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_beams_output->setPalette(PALET_NORMAL);
  AUTFBACK(cb_beams_output);
  connect(cb_beams_output, SIGNAL(clicked()), this,
          SLOT(select_output_beams()));

  cb_hydro_output = new QCheckBox();
  cb_hydro_output->setText(us_tr(" HYDRO "));
  cb_hydro_output->setEnabled(true);
  cb_hydro_output->setChecked((*bead_output).output & US_HYDRODYN_OUTPUT_HYDRO);
  cb_hydro_output->setMinimumHeight(minHeight1);
  cb_hydro_output->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_hydro_output->setPalette(PALET_NORMAL);
  AUTFBACK(cb_hydro_output);
  connect(cb_hydro_output, SIGNAL(clicked()), this,
          SLOT(select_output_hydro()));

  cb_grpy_output = new QCheckBox();
  cb_grpy_output->setText(us_tr(" GRPY "));
  cb_grpy_output->setEnabled(true);
  cb_grpy_output->setChecked((*bead_output).output & US_HYDRODYN_OUTPUT_GRPY);
  cb_grpy_output->setMinimumHeight(minHeight1);
  cb_grpy_output->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_grpy_output->setPalette(PALET_NORMAL);
  AUTFBACK(cb_grpy_output);
  connect(cb_grpy_output, SIGNAL(clicked()), this, SLOT(select_output_grpy()));
  cb_grpy_output->hide();

  {
    QHBoxLayout *bl = new QHBoxLayout;
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(0);
    bl->addWidget(cb_somo_output);
    bl->addWidget(cb_beams_output);
    bl->addWidget(cb_hydro_output);
    bl->addWidget(cb_grpy_output);
    bg_output->setLayout(bl);
  }

#endif

#if QT_VERSION < 0x040000
  bg_sequence = new QGroupBox(2, Qt::Vertical, "Bead Sequence Format:", this);
  bg_sequence->setExclusive(true);
  connect(bg_sequence, SIGNAL(clicked(int)), this, SLOT(select_sequence(int)));

  cb_pdb_sequence = new QCheckBox(bg_sequence);
  cb_pdb_sequence->setText(us_tr(" as in original PDB  "));
  cb_pdb_sequence->setEnabled(true);
  cb_pdb_sequence->setMinimumHeight(minHeight1);
  cb_pdb_sequence->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_pdb_sequence->setPalette(PALET_NORMAL);
  AUTFBACK(cb_pdb_sequence);

  cb_chain_sequence = new QCheckBox(bg_sequence);
  cb_chain_sequence->setText(
      us_tr(" exposed side chain -> exposed main chain -> buried "));
  cb_chain_sequence->setEnabled(true);
  cb_chain_sequence->setMinimumHeight(minHeight1);
  cb_chain_sequence->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_chain_sequence->setPalette(PALET_NORMAL);
  AUTFBACK(cb_chain_sequence);

  bg_sequence->setButton((*bead_output).sequence);
#else
  bg_sequence = new QGroupBox("Bead Sequence Format:");

  rb_pdb_sequence = new QRadioButton();
  rb_pdb_sequence->setText(us_tr(" as in original PDB  "));
  rb_pdb_sequence->setEnabled(true);
  rb_pdb_sequence->setMinimumHeight(minHeight1);
  rb_pdb_sequence->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_pdb_sequence->setPalette(PALET_NORMAL);
  AUTFBACK(rb_pdb_sequence);
  connect(rb_pdb_sequence, SIGNAL(clicked()), this, SLOT(select_sequence()));

  rb_chain_sequence = new QRadioButton();
  rb_chain_sequence->setText(
      us_tr(" exposed side chain -> exposed main chain -> buried "));
  rb_chain_sequence->setEnabled(true);
  rb_chain_sequence->setMinimumHeight(minHeight1);
  rb_chain_sequence->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_chain_sequence->setPalette(PALET_NORMAL);
  AUTFBACK(rb_chain_sequence);
  connect(rb_chain_sequence, SIGNAL(clicked()), this, SLOT(select_sequence()));

  {
    QVBoxLayout *bl = new QVBoxLayout;
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(0);
    bl->addWidget(rb_pdb_sequence);
    bl->addWidget(rb_chain_sequence);
    bg_sequence->setLayout(bl);
  }

  switch ((*bead_output).sequence) {
    case 0:
      rb_pdb_sequence->setChecked(true);
      break;
    case 1:
      rb_chain_sequence->setChecked(true);
      break;
    default:
      qDebug() << "bead output sequence switch error";
      break;
  }
#endif

#if QT_VERSION < 0x040000
  bg_beams = new QGroupBox(1, Qt::Vertical, "BEAMS Format:", this);

  cb_correspondence = new QCheckBox(bg_beams);
  cb_correspondence->setText(
      us_tr(" include bead - original residue correspondence "));
  if ((*bead_output).output & US_HYDRODYN_OUTPUT_BEAMS) {
    cb_correspondence->setEnabled(true);
  } else {
    cb_correspondence->setEnabled(false);
  }
  cb_correspondence->setChecked((*bead_output).correspondence);
  cb_correspondence->setMinimumHeight(minHeight1);
  cb_correspondence->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_correspondence->setPalette(PALET_NORMAL);
  AUTFBACK(cb_correspondence);
  connect(cb_correspondence, SIGNAL(clicked()), this,
          SLOT(select_correspondence()));

#else
  bg_beams = new QGroupBox("BEAMS Format:");

  cb_correspondence = new QCheckBox();
  cb_correspondence->setText(
      us_tr(" include bead - original residue correspondence "));
  if ((*bead_output).output & US_HYDRODYN_OUTPUT_BEAMS) {
    cb_correspondence->setEnabled(true);
  } else {
    cb_correspondence->setEnabled(false);
  }
  cb_correspondence->setChecked((*bead_output).correspondence);
  cb_correspondence->setMinimumHeight(minHeight1);
  cb_correspondence->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_correspondence->setPalette(PALET_NORMAL);
  AUTFBACK(cb_correspondence);
  connect(cb_correspondence, SIGNAL(clicked()), this,
          SLOT(select_correspondence()));

  {
    QHBoxLayout *bl = new QHBoxLayout;
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(0);
    bl->addWidget(cb_correspondence);
    bg_beams->setLayout(bl);
  }
#endif

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

  int /* rows=10, columns = 2, */ spacing = 2, j = 0, margin = 4;
  QGridLayout *background = new QGridLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);
  background->setSpacing(spacing);
  background->setContentsMargins(margin, margin, margin, margin);

  background->addWidget(lbl_info, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  background->addWidget(bg_output, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  background->addWidget(bg_sequence, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  background->addWidget(bg_beams, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  background->addWidget(pb_help, j, 0);
  background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_Bead_Output::select_output_somo() { select_output(0); }

void US_Hydrodyn_Bead_Output::select_output_beams() { select_output(1); }

void US_Hydrodyn_Bead_Output::select_output_hydro() { select_output(2); }

void US_Hydrodyn_Bead_Output::select_output_grpy() { select_output(3); }

void US_Hydrodyn_Bead_Output::select_output(int val) {
  if ((*bead_output).output & 1 << val) {
    if (val == 1) {
      cb_correspondence->setEnabled(false);
    }
    (*bead_output).output &= ~(1 << val);
  } else {
    if (val == 1) {
      cb_correspondence->setEnabled(true);
    }
    (*bead_output).output |= (1 << val);
  }
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Bead_Output::select_sequence() {
  if (rb_pdb_sequence->isChecked()) {
    return select_sequence(0);
  }
  if (rb_chain_sequence->isChecked()) {
    return select_sequence(1);
  }
}

void US_Hydrodyn_Bead_Output::select_sequence(int val) {
  (*bead_output).sequence = val;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Bead_Output::select_correspondence() {
  (*bead_output).correspondence = cb_correspondence->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Bead_Output::cancel() { close(); }

void US_Hydrodyn_Bead_Output::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  online_help->show_help("manual/somo/somo_bead_output.html");
}

void US_Hydrodyn_Bead_Output::closeEvent(QCloseEvent *e) {
  *bead_output_widget = false;
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}
