#include "../include/us_hydrodyn_pdb_parsing.h"

#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
// Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>

US_Hydrodyn_PDB_Parsing::US_Hydrodyn_PDB_Parsing(struct pdb_parsing *pdb,
                                                 bool *pdb_parsing_widget,
                                                 void *us_hydrodyn, QWidget *p,
                                                 const char *)
    : QFrame(p) {
  this->pdb = pdb;
  this->pdb_parsing_widget = pdb_parsing_widget;
  this->us_hydrodyn = us_hydrodyn;
  *pdb_parsing_widget = true;
  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("SOMO PDB Parsing Options"));
  setupGUI();
  global_Xpos += 30;
  global_Ypos += 30;
  setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_PDB_Parsing::~US_Hydrodyn_PDB_Parsing() {
  *pdb_parsing_widget = false;
}

void US_Hydrodyn_PDB_Parsing::setupGUI() {
  int minHeight1 = 30;

  lbl_info = new QLabel(us_tr("SOMO PDB Parsing Options:"), this);
  Q_CHECK_PTR(lbl_info);
  lbl_info->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_info->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_info->setMinimumHeight(minHeight1);
  lbl_info->setPalette(PALET_FRAME);
  AUTFBACK(lbl_info);
  lbl_info->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize + 1, QFont::Bold));

  bg_misc = new QGroupBox("Miscellaneous parsing options:");

  cb_skip_hydrogen = new QCheckBox();
  cb_skip_hydrogen->setText(us_tr(" Skip hydrogen atoms"));
  cb_skip_hydrogen->setEnabled(false);
  cb_skip_hydrogen->setChecked((*pdb).skip_hydrogen);
  cb_skip_hydrogen->setMinimumHeight(minHeight1);
  cb_skip_hydrogen->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_skip_hydrogen->setPalette(PALET_NORMAL);
  AUTFBACK(cb_skip_hydrogen);
  connect(cb_skip_hydrogen, SIGNAL(clicked()), this, SLOT(skip_hydrogen()));

  cb_skip_water = new QCheckBox();
  cb_skip_water->setText(us_tr(" Skip solvent water molecules"));
  cb_skip_water->setEnabled(false);
  cb_skip_water->setChecked((*pdb).skip_water);
  cb_skip_water->setMinimumHeight(minHeight1);
  cb_skip_water->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_skip_water->setPalette(PALET_NORMAL);
  AUTFBACK(cb_skip_water);
  connect(cb_skip_water, SIGNAL(clicked()), this, SLOT(skip_water()));

  cb_alternate = new QCheckBox();
  cb_alternate->setText(us_tr(" Skip alternate conformations"));
  cb_alternate->setEnabled(false);
  cb_alternate->setChecked((*pdb).alternate);
  cb_alternate->setMinimumHeight(minHeight1);
  cb_alternate->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_alternate->setPalette(PALET_NORMAL);
  AUTFBACK(cb_alternate);
  connect(cb_alternate, SIGNAL(clicked()), this, SLOT(alternate()));

  cb_find_sh = new QCheckBox();
  cb_find_sh->setText(us_tr(" Find free SH, change residue coding"));
  cb_find_sh->setEnabled(true);
  cb_find_sh->setChecked((*pdb).find_sh);
  cb_find_sh->setMinimumHeight(minHeight1);
  cb_find_sh->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_find_sh->setPalette(PALET_NORMAL);
  AUTFBACK(cb_find_sh);
  connect(cb_find_sh, SIGNAL(clicked()), this, SLOT(find_sh()));

  lbl_thresh_SS =
      new QLabel(us_tr(" Disulfide distance threshold [A]: "), this);
  lbl_thresh_SS->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_thresh_SS->setMinimumWidth(220);
  lbl_thresh_SS->setMinimumHeight(minHeight1);
  lbl_thresh_SS->setPalette(PALET_LABEL);
  AUTFBACK(lbl_thresh_SS);
  lbl_thresh_SS->setFont(QFont(USglobal->config_list.fontFamily,
                               USglobal->config_list.fontSize - 1,
                               QFont::Bold));

  le_thresh_SS = new QLineEdit(this);
  le_thresh_SS->setObjectName("thresh_SS Line Edit");
  le_thresh_SS->setMinimumHeight(minHeight1);
  le_thresh_SS->setEnabled(cb_find_sh->isChecked());
  le_thresh_SS->setText(
      QString("%1").arg(((US_Hydrodyn *)us_hydrodyn)->gparams.count("thresh_SS")
                            ? ((US_Hydrodyn *)us_hydrodyn)->gparams["thresh_SS"]
                            : "2.3"));
  le_thresh_SS->setPalette(PALET_NORMAL);
  AUTFBACK(le_thresh_SS);
  le_thresh_SS->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_thresh_SS, SIGNAL(textChanged(const QString &)),
          SLOT(update_thresh_SS(const QString &)));

  cb_save_csv_on_load = new QCheckBox();
  cb_save_csv_on_load->setText(us_tr(" Save CSV on load PDB"));
  cb_save_csv_on_load->setEnabled(true);
  cb_save_csv_on_load->setChecked(
      ((US_Hydrodyn *)us_hydrodyn)->gparams.count("save_csv_on_load_pdb") &&
      ((US_Hydrodyn *)us_hydrodyn)->gparams["save_csv_on_load_pdb"] == "true");
  cb_save_csv_on_load->setMinimumHeight(minHeight1);
  cb_save_csv_on_load->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_save_csv_on_load->setPalette(PALET_NORMAL);
  AUTFBACK(cb_save_csv_on_load);
  connect(cb_save_csv_on_load, SIGNAL(clicked()), this,
          SLOT(save_csv_on_load()));

  cb_use_WAT_Tf = new QCheckBox();
  cb_use_WAT_Tf->setText(
      us_tr(" Use explicit WAT radii as specified in PDB the T-factor field"));
  cb_use_WAT_Tf->setEnabled(true);
  cb_use_WAT_Tf->setChecked(
      ((US_Hydrodyn *)us_hydrodyn)->gparams.count("use_WAT_Tf_pdb") &&
      ((US_Hydrodyn *)us_hydrodyn)->gparams["use_WAT_Tf_pdb"] == "true");
  cb_use_WAT_Tf->setMinimumHeight(minHeight1);
  cb_use_WAT_Tf->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_use_WAT_Tf->setPalette(PALET_NORMAL);
  AUTFBACK(cb_use_WAT_Tf);
  connect(cb_use_WAT_Tf, SIGNAL(clicked()), this, SLOT(use_WAT_Tf()));

  {
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(cb_skip_hydrogen);
    vbox->addWidget(cb_skip_water);
    vbox->addWidget(cb_alternate);
    vbox->addWidget(cb_find_sh);
    {
      QHBoxLayout *hbox = new QHBoxLayout;
      vbox->setContentsMargins(0, 0, 0, 0);
      vbox->setSpacing(0);
      hbox->addWidget(lbl_thresh_SS);
      hbox->addWidget(le_thresh_SS);
      vbox->addLayout(hbox);
    }
    vbox->addWidget(cb_save_csv_on_load);
    vbox->addWidget(cb_use_WAT_Tf);

    bg_misc->setLayout(vbox);
  }

  bg_residues = new QGroupBox("If non-coded residues are found:", this);
  bg_residues->setFlat(true);
  // bg_residues->setExclusive(true);
  bg_residues->setAlignment(Qt::AlignHCenter);
  // bg_residues->setInsideMargin(3);
  // bg_residues->setInsideSpacing(0);

  rb_residue_stop = new QRadioButton();
  rb_residue_stop->setText(us_tr(" List them and stop operation"));
  rb_residue_stop->setEnabled(true);
  rb_residue_stop->setMinimumHeight(minHeight1);
  rb_residue_stop->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_residue_stop->setPalette(PALET_NORMAL);
  AUTFBACK(rb_residue_stop);
  connect(rb_residue_stop, SIGNAL(clicked()), this, SLOT(residue()));

  rb_residue_skip = new QRadioButton();
  rb_residue_skip->setText(us_tr(" List them, skip residue and proceed"));
  rb_residue_skip->setEnabled(true);
  rb_residue_skip->setMinimumHeight(minHeight1);
  rb_residue_skip->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_residue_skip->setPalette(PALET_NORMAL);
  AUTFBACK(rb_residue_skip);
  connect(rb_residue_skip, SIGNAL(clicked()), this, SLOT(residue()));

  rb_residue_auto = new QRadioButton();
  rb_residue_auto->setText(
      us_tr(" Use automatic bead builder (approximate method)"));
  rb_residue_auto->setEnabled(true);
  rb_residue_auto->setMinimumHeight(minHeight1);
  rb_residue_auto->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_residue_auto->setPalette(PALET_NORMAL);
  AUTFBACK(rb_residue_auto);
  connect(rb_residue_auto, SIGNAL(clicked()), this, SLOT(residue()));

  {
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(rb_residue_stop);
    vbox->addWidget(rb_residue_skip);
    vbox->addWidget(rb_residue_auto);
    bg_residues->setLayout(vbox);
  }

  // bg_residues->setButton((*pdb).missing_residues);
  switch ((*pdb).missing_residues) {
    case 0:
      rb_residue_stop->setChecked(true);
      break;
    case 1:
      rb_residue_skip->setChecked(true);
      break;
    case 2:
      rb_residue_auto->setChecked(true);
      break;
    default:
      qDebug() << "pdb parsing missing residues selection error";
      break;
  }

  bg_atoms = new QGroupBox("If missing atoms within a residue are found:");
  bg_atoms->setFlat(true);
  // bg_atoms->setExclusive(true);
  bg_atoms->setAlignment(Qt::AlignHCenter);
  // bg_atoms->setInsideMargin(3);
  // bg_atoms->setInsideSpacing(0);

  rb_atom_stop = new QRadioButton();
  rb_atom_stop->setText(us_tr(" List them and stop operation"));
  rb_atom_stop->setEnabled(true);
  //   rb_atom_stop->setMinimumHeight(minHeight1);
  rb_atom_stop->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_atom_stop->setPalette(PALET_NORMAL);
  AUTFBACK(rb_atom_stop);
  connect(rb_atom_stop, SIGNAL(clicked()), this, SLOT(atom()));

  rb_atom_skip = new QRadioButton();
  rb_atom_skip->setText(us_tr(" List them, skip entire residue and proceed"));
  rb_atom_skip->setEnabled(true);
  //   rb_atom_skip->setMinimumHeight(minHeight1);
  rb_atom_skip->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_atom_skip->setPalette(PALET_NORMAL);
  AUTFBACK(rb_atom_skip);
  connect(rb_atom_skip, SIGNAL(clicked()), this, SLOT(atom()));

  rb_atom_auto = new QRadioButton();
  rb_atom_auto->setText(us_tr(" Use approximate method to generate bead"));
  rb_atom_auto->setEnabled(true);
  //   rb_atom_auto->setMinimumHeight(minHeight1);
  rb_atom_auto->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  rb_atom_auto->setPalette(PALET_NORMAL);
  AUTFBACK(rb_atom_auto);
  connect(rb_atom_auto, SIGNAL(clicked()), this, SLOT(atom()));

  // bg_atoms->setButton((*pdb).missing_atoms);
  switch ((*pdb).missing_atoms) {
    case 0:
      rb_atom_stop->setChecked(true);
      break;
    case 1:
      rb_atom_skip->setChecked(true);
      break;
    case 2:
      rb_atom_auto->setChecked(true);
      break;
    default:
      qDebug() << "pdb parsing missing atoms selection error";
      break;
  }

  {
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(rb_atom_stop);
    vbox->addWidget(rb_atom_skip);
    vbox->addWidget(rb_atom_auto);
    bg_atoms->setLayout(vbox);
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
  background->addWidget(bg_misc, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  background->addWidget(bg_residues, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  background->addWidget(bg_atoms, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;
  background->addWidget(pb_help, j, 0);
  background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_PDB_Parsing::skip_hydrogen() {
  (*pdb).skip_hydrogen = cb_skip_hydrogen->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_PDB_Parsing::skip_water() {
  (*pdb).skip_water = cb_skip_water->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_PDB_Parsing::alternate() {
  (*pdb).alternate = cb_alternate->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_PDB_Parsing::find_sh() {
  (*pdb).find_sh = cb_find_sh->isChecked();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
  le_thresh_SS->setEnabled(cb_find_sh->isChecked());
}

void US_Hydrodyn_PDB_Parsing::save_csv_on_load() {
  ((US_Hydrodyn *)us_hydrodyn)->gparams["save_csv_on_load_pdb"] =
      cb_save_csv_on_load->isChecked() ? "true" : "false";
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_PDB_Parsing::use_WAT_Tf() {
  ((US_Hydrodyn *)us_hydrodyn)->gparams["use_WAT_Tf_pdb"] =
      cb_use_WAT_Tf->isChecked() ? "true" : "false";
  // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_PDB_Parsing::residue() {
  if (rb_residue_stop->isChecked()) {
    return residue(0);
  }
  if (rb_residue_skip->isChecked()) {
    return residue(1);
  }
  if (rb_residue_auto->isChecked()) {
    return residue(2);
  }
}

void US_Hydrodyn_PDB_Parsing::residue(int val) {
  (*pdb).missing_residues = val;
  // if ( val ) {
  //    ((US_Hydrodyn *)us_hydrodyn)->misc.pb_rule_on = false;
  // }
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_PDB_Parsing::atom() {
  if (rb_atom_stop->isChecked()) {
    return atom(0);
  }
  if (rb_atom_skip->isChecked()) {
    return atom(1);
  }
  if (rb_atom_auto->isChecked()) {
    return atom(2);
  }
}

void US_Hydrodyn_PDB_Parsing::atom(int val) {
  (*pdb).missing_atoms = val;
  // if ( val ) {
  //    ((US_Hydrodyn *)us_hydrodyn)->misc.pb_rule_on = false;
  // }
  ((US_Hydrodyn *)us_hydrodyn)->set_disabled();
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_PDB_Parsing::cancel() { close(); }

void US_Hydrodyn_PDB_Parsing::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  online_help->show_help("manual/somo/somo_pdb_parsing.html");
}

void US_Hydrodyn_PDB_Parsing::closeEvent(QCloseEvent *e) {
  *pdb_parsing_widget = false;
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

void US_Hydrodyn_PDB_Parsing::update_thresh_SS(const QString &str) {
  ((US_Hydrodyn *)us_hydrodyn)->gparams["thresh_SS"] = str;
  ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
