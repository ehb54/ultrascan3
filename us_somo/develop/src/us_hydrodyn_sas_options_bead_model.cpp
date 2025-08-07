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

US_Hydrodyn_SasOptionsBeadModel::US_Hydrodyn_SasOptionsBeadModel(
    struct saxs_options *saxs_options, bool *sas_options_bead_model_widget,
    void *us_hydrodyn, QWidget *p, const char *)
    : QFrame(p) {
  this->sas_options_bead_model_widget = sas_options_bead_model_widget;
  this->saxs_options = saxs_options;
  this->us_hydrodyn = us_hydrodyn;
  *sas_options_bead_model_widget = true;
  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("US-SOMO SAS Bead Model Options"));
  setupGUI();
  global_Xpos += 30;
  global_Ypos += 30;
  setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SasOptionsBeadModel::~US_Hydrodyn_SasOptionsBeadModel() {
  *sas_options_bead_model_widget = false;
}

void US_Hydrodyn_SasOptionsBeadModel::setupGUI() {
  started_in_expert_mode = U_EXPT;

  int minHeight1 = 30;
  lbl_info = new QLabel(us_tr("US-SOMO SAS Bead Model Options:"), this);
  lbl_info->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_info->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_info->setMinimumHeight(minHeight1);
  lbl_info->setPalette(PALET_FRAME);
  AUTFBACK(lbl_info);
  lbl_info->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize + 1, QFont::Bold));

  cb_compute_saxs_coeff_for_bead_models = new QCheckBox(this);
  cb_compute_saxs_coeff_for_bead_models->setText(
      us_tr(" Compute SAXS coefficients for bead models"));
  cb_compute_saxs_coeff_for_bead_models->setEnabled(true);
  cb_compute_saxs_coeff_for_bead_models->setChecked(
      (*saxs_options).compute_saxs_coeff_for_bead_models);
  cb_compute_saxs_coeff_for_bead_models->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_saxs_coeff_for_bead_models->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_saxs_coeff_for_bead_models);
  connect(cb_compute_saxs_coeff_for_bead_models, SIGNAL(clicked()), this,
          SLOT(set_compute_saxs_coeff_for_bead_models()));

  cb_compute_sans_coeff_for_bead_models = new QCheckBox(this);
  cb_compute_sans_coeff_for_bead_models->setText(
      us_tr(" Compute SANS coefficients for bead models"));
  cb_compute_sans_coeff_for_bead_models->setEnabled(true);
  cb_compute_sans_coeff_for_bead_models->setChecked(
      (*saxs_options).compute_sans_coeff_for_bead_models);
  cb_compute_sans_coeff_for_bead_models->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_compute_sans_coeff_for_bead_models->setPalette(PALET_NORMAL);
  AUTFBACK(cb_compute_sans_coeff_for_bead_models);
  connect(cb_compute_sans_coeff_for_bead_models, SIGNAL(clicked()), this,
          SLOT(set_compute_sans_coeff_for_bead_models()));

  cb_bead_model_rayleigh = new QCheckBox(this);
  cb_bead_model_rayleigh->setText(
      us_tr(" Use Rayleigh (1911) for structure factors"));
  cb_bead_model_rayleigh->setEnabled(true);
  cb_bead_model_rayleigh->setChecked((*saxs_options).bead_model_rayleigh);
  cb_bead_model_rayleigh->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_bead_model_rayleigh->setPalette(PALET_NORMAL);
  AUTFBACK(cb_bead_model_rayleigh);
  connect(cb_bead_model_rayleigh, SIGNAL(clicked()), this,
          SLOT(set_bead_model_rayleigh()));

  if (started_in_expert_mode) {
    cb_bead_models_use_bead_radius_ev = new QCheckBox(this);
    cb_bead_models_use_bead_radius_ev->setText(
        us_tr(" Use bead radius for excluded volume"));
    cb_bead_models_use_bead_radius_ev->setEnabled(true);
    cb_bead_models_use_bead_radius_ev->setChecked(
        (*saxs_options).bead_models_use_bead_radius_ev);
    cb_bead_models_use_bead_radius_ev->setFont(QFont(
        USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
    cb_bead_models_use_bead_radius_ev->setPalette(PALET_NORMAL);
    AUTFBACK(cb_bead_models_use_bead_radius_ev);
    connect(cb_bead_models_use_bead_radius_ev, SIGNAL(clicked()), this,
            SLOT(set_bead_models_use_bead_radius_ev()));

    cb_bead_models_rho0_in_scat_factors = new QCheckBox(this);
    cb_bead_models_rho0_in_scat_factors->setText(
        us_tr(" rho0 included in computed scattering factors"));
    cb_bead_models_rho0_in_scat_factors->setEnabled(true);
    cb_bead_models_rho0_in_scat_factors->setChecked(
        (*saxs_options).bead_models_rho0_in_scat_factors);
    cb_bead_models_rho0_in_scat_factors->setFont(QFont(
        USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
    cb_bead_models_rho0_in_scat_factors->setPalette(PALET_NORMAL);
    AUTFBACK(cb_bead_models_rho0_in_scat_factors);
    connect(cb_bead_models_rho0_in_scat_factors, SIGNAL(clicked()), this,
            SLOT(set_bead_models_rho0_in_scat_factors()));

    cb_iq_global_avg_for_bead_models = new QCheckBox(this);
    cb_iq_global_avg_for_bead_models->setText(us_tr(
        " Use global average scattering factors for loading bead models"));
    cb_iq_global_avg_for_bead_models->setEnabled(true);
    cb_iq_global_avg_for_bead_models->setChecked(
        (*saxs_options).iq_global_avg_for_bead_models);
    cb_iq_global_avg_for_bead_models->setFont(QFont(
        USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
    cb_iq_global_avg_for_bead_models->setPalette(PALET_NORMAL);
    AUTFBACK(cb_iq_global_avg_for_bead_models);
    connect(cb_iq_global_avg_for_bead_models, SIGNAL(clicked()), this,
            SLOT(set_iq_global_avg_for_bead_models()));

    cb_bead_models_use_quick_fitting = new QCheckBox(this);
    cb_bead_models_use_quick_fitting->setText(
        us_tr(" Use quick method for computing scattering factors"));
    cb_bead_models_use_quick_fitting->setEnabled(true);
    cb_bead_models_use_quick_fitting->setChecked(
        (*saxs_options).bead_models_use_quick_fitting);
    cb_bead_models_use_quick_fitting->setFont(QFont(
        USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
    cb_bead_models_use_quick_fitting->setPalette(PALET_NORMAL);
    AUTFBACK(cb_bead_models_use_quick_fitting);
    connect(cb_bead_models_use_quick_fitting, SIGNAL(clicked()), this,
            SLOT(set_bead_models_use_quick_fitting()));

    cb_bead_models_use_gsm_fitting = new QCheckBox(this);
    cb_bead_models_use_gsm_fitting->setText(
        us_tr(" Use gsm method for computing scattering factors"));
    cb_bead_models_use_gsm_fitting->setEnabled(true);
    cb_bead_models_use_gsm_fitting->setChecked(
        (*saxs_options).bead_models_use_gsm_fitting);
    cb_bead_models_use_gsm_fitting->setFont(QFont(
        USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
    cb_bead_models_use_gsm_fitting->setPalette(PALET_NORMAL);
    AUTFBACK(cb_bead_models_use_gsm_fitting);
    connect(cb_bead_models_use_gsm_fitting, SIGNAL(clicked()), this,
            SLOT(set_bead_models_use_gsm_fitting()));

    cb_apply_loaded_sf_repeatedly_to_pdb = new QCheckBox(this);
    cb_apply_loaded_sf_repeatedly_to_pdb->setText(
        us_tr(" Apply preloaded individual bead structure factors to bead "
              "model PDB"));
    cb_apply_loaded_sf_repeatedly_to_pdb->setEnabled(true);
    cb_apply_loaded_sf_repeatedly_to_pdb->setChecked(
        (*saxs_options).apply_loaded_sf_repeatedly_to_pdb);
    cb_apply_loaded_sf_repeatedly_to_pdb->setFont(QFont(
        USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
    cb_apply_loaded_sf_repeatedly_to_pdb->setPalette(PALET_NORMAL);
    AUTFBACK(cb_apply_loaded_sf_repeatedly_to_pdb);
    connect(cb_apply_loaded_sf_repeatedly_to_pdb, SIGNAL(clicked()), this,
            SLOT(set_apply_loaded_sf_repeatedly_to_pdb()));

    cb_bead_models_use_var_len_sf = new QCheckBox(this);
    cb_bead_models_use_var_len_sf->setText(
        us_tr(" Compute and use variable length scattering factors"));
    cb_bead_models_use_var_len_sf->setEnabled(true);
    cb_bead_models_use_var_len_sf->setChecked(
        (*saxs_options).bead_models_use_var_len_sf);
    cb_bead_models_use_var_len_sf->setFont(QFont(
        USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
    cb_bead_models_use_var_len_sf->setPalette(PALET_NORMAL);
    AUTFBACK(cb_bead_models_use_var_len_sf);
    connect(cb_bead_models_use_var_len_sf, SIGNAL(clicked()), this,
            SLOT(set_bead_models_use_var_len_sf()));

    lbl_bead_models_var_len_sf_max = new QLabel(
        us_tr(" Variable length scattering factors max length: "), this);
    lbl_bead_models_var_len_sf_max->setAlignment(Qt::AlignLeft |
                                                 Qt::AlignVCenter);
    lbl_bead_models_var_len_sf_max->setMinimumHeight(minHeight1);
    lbl_bead_models_var_len_sf_max->setPalette(PALET_LABEL);
    AUTFBACK(lbl_bead_models_var_len_sf_max);
    lbl_bead_models_var_len_sf_max->setFont(
        QFont(USglobal->config_list.fontFamily,
              USglobal->config_list.fontSize - 1, QFont::Bold));

    le_bead_models_var_len_sf_max = new QLineEdit(this);
    le_bead_models_var_len_sf_max->setObjectName("");
    le_bead_models_var_len_sf_max->setText(
        QString("%1").arg((*saxs_options).bead_models_var_len_sf_max));
    le_bead_models_var_len_sf_max->setAlignment(Qt::AlignLeft |
                                                Qt::AlignVCenter);
    le_bead_models_var_len_sf_max->setPalette(PALET_EDIT);
    AUTFBACK(le_bead_models_var_len_sf_max);
    le_bead_models_var_len_sf_max->setFont(QFont(
        USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
    le_bead_models_var_len_sf_max_qv = new QIntValidator(5, 25, this);
    le_bead_models_var_len_sf_max->setValidator(
        le_bead_models_var_len_sf_max_qv);
    connect(le_bead_models_var_len_sf_max, SIGNAL(textChanged(const QString &)),
            this, SLOT(update_bead_models_var_len_sf_max(const QString &)));

    lbl_dummy_saxs_name =
        new QLabel(us_tr(" Saxs name for dummy atom models: "), this);
    lbl_dummy_saxs_name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lbl_dummy_saxs_name->setMinimumHeight(minHeight1);
    lbl_dummy_saxs_name->setPalette(PALET_LABEL);
    AUTFBACK(lbl_dummy_saxs_name);
    lbl_dummy_saxs_name->setFont(QFont(USglobal->config_list.fontFamily,
                                       USglobal->config_list.fontSize - 1,
                                       QFont::Bold));

    le_dummy_saxs_name = new QLineEdit(this);
    le_dummy_saxs_name->setObjectName("");
    le_dummy_saxs_name->setText((*saxs_options).dummy_saxs_name);
    le_dummy_saxs_name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    le_dummy_saxs_name->setPalette(PALET_EDIT);
    AUTFBACK(le_dummy_saxs_name);
    le_dummy_saxs_name->setFont(QFont(USglobal->config_list.fontFamily,
                                      USglobal->config_list.fontSize));
    connect(le_dummy_saxs_name, SIGNAL(textChanged(const QString &)), this,
            SLOT(update_dummy_saxs_name(const QString &)));
  }

  cb_dummy_atom_pdbs_in_nm = new QCheckBox(this);
  cb_dummy_atom_pdbs_in_nm->setText(us_tr(" Dummy atom PDB's in NM "));
  cb_dummy_atom_pdbs_in_nm->setEnabled(true);
  cb_dummy_atom_pdbs_in_nm->setChecked((*saxs_options).dummy_atom_pdbs_in_nm);
  cb_dummy_atom_pdbs_in_nm->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_dummy_atom_pdbs_in_nm->setPalette(PALET_NORMAL);
  AUTFBACK(cb_dummy_atom_pdbs_in_nm);
  connect(cb_dummy_atom_pdbs_in_nm, SIGNAL(clicked()), this,
          SLOT(set_dummy_atom_pdbs_in_nm()));

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

  int /* rows = 0, columns = 2,*/ spacing = 2, j = 0, margin = 4;
  QGridLayout *background = new QGridLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);
  background->setSpacing(spacing);
  background->setContentsMargins(margin, margin, margin, margin);

  background->addWidget(lbl_info, j, 0, 1 + (j) - (j), 1 + (1) - (0));
  j++;

  background->addWidget(cb_compute_saxs_coeff_for_bead_models, j, 0,
                        1 + (j) - (j), 1 + (1) - (0));
  j++;
  background->addWidget(cb_compute_sans_coeff_for_bead_models, j, 0,
                        1 + (j) - (j), 1 + (1) - (0));
  j++;
  background->addWidget(cb_bead_model_rayleigh, j, 0, 1 + (j) - (j),
                        1 + (1) - (0));
  j++;
  if (started_in_expert_mode) {
    background->addWidget(cb_bead_models_use_bead_radius_ev, j, 0,
                          1 + (j) - (j), 1 + (1) - (0));
    j++;
    background->addWidget(cb_bead_models_rho0_in_scat_factors, j, 0,
                          1 + (j) - (j), 1 + (1) - (0));
    j++;
    background->addWidget(cb_iq_global_avg_for_bead_models, j, 0, 1 + (j) - (j),
                          1 + (1) - (0));
    j++;
    background->addWidget(cb_bead_models_use_quick_fitting, j, 0, 1 + (j) - (j),
                          1 + (1) - (0));
    j++;
    background->addWidget(cb_bead_models_use_gsm_fitting, j, 0, 1 + (j) - (j),
                          1 + (1) - (0));
    j++;
    background->addWidget(cb_apply_loaded_sf_repeatedly_to_pdb, j, 0,
                          1 + (j) - (j), 1 + (1) - (0));
    j++;
    background->addWidget(cb_bead_models_use_var_len_sf, j, 0, 1 + (j) - (j),
                          1 + (1) - (0));
    j++;

    background->addWidget(lbl_bead_models_var_len_sf_max, j, 0);
    background->addWidget(le_bead_models_var_len_sf_max, j, 1);
    j++;

    background->addWidget(lbl_dummy_saxs_name, j, 0);
    background->addWidget(le_dummy_saxs_name, j, 1);
    j++;
  }

  background->addWidget(cb_dummy_atom_pdbs_in_nm, j, 0, 1 + (j) - (j),
                        1 + (1) - (0));
  j++;

  background->addWidget(pb_help, j, 0);
  background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_SasOptionsBeadModel::cancel() { close(); }

void US_Hydrodyn_SasOptionsBeadModel::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  // online_help->show_help("manual/somo/somo_sas_options_bead_model.html");
  online_help->show_help("manual/somo/somo_saxs_options.html");
}

void US_Hydrodyn_SasOptionsBeadModel::closeEvent(QCloseEvent *e) {
  *sas_options_bead_model_widget = false;
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

void US_Hydrodyn_SasOptionsBeadModel::set_compute_saxs_coeff_for_bead_models() {
  (*saxs_options).compute_saxs_coeff_for_bead_models =
      cb_compute_saxs_coeff_for_bead_models->isChecked();
  if ((*saxs_options).compute_saxs_coeff_for_bead_models) {
    ((US_Hydrodyn *)us_hydrodyn)
        ->saxs_util->setup_saxs_maps(saxs_options->default_atom_filename,
                                     saxs_options->default_hybrid_filename,
                                     saxs_options->default_saxs_filename);
  }
  //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_compute_sans_coeff_for_bead_models() {
  (*saxs_options).compute_sans_coeff_for_bead_models =
      cb_compute_sans_coeff_for_bead_models->isChecked();
  //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_bead_model_rayleigh() {
  (*saxs_options).bead_model_rayleigh = cb_bead_model_rayleigh->isChecked();
  //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_bead_models_use_bead_radius_ev() {
  (*saxs_options).bead_models_use_bead_radius_ev =
      cb_bead_models_use_bead_radius_ev->isChecked();
  //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_bead_models_rho0_in_scat_factors() {
  (*saxs_options).bead_models_rho0_in_scat_factors =
      cb_bead_models_rho0_in_scat_factors->isChecked();
  //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_iq_global_avg_for_bead_models() {
  (*saxs_options).iq_global_avg_for_bead_models =
      cb_iq_global_avg_for_bead_models->isChecked();
  //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_bead_models_use_quick_fitting() {
  (*saxs_options).bead_models_use_quick_fitting =
      cb_bead_models_use_quick_fitting->isChecked();
  if (cb_bead_models_use_quick_fitting->isChecked()) {
    cb_bead_models_use_gsm_fitting->setChecked(false);
    (*saxs_options).bead_models_use_gsm_fitting = false;
  }
  //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_bead_models_use_gsm_fitting() {
  (*saxs_options).bead_models_use_gsm_fitting =
      cb_bead_models_use_gsm_fitting->isChecked();
  if (cb_bead_models_use_gsm_fitting->isChecked()) {
    cb_bead_models_use_quick_fitting->setChecked(false);
    (*saxs_options).bead_models_use_quick_fitting = false;
  }

  //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_apply_loaded_sf_repeatedly_to_pdb() {
  (*saxs_options).apply_loaded_sf_repeatedly_to_pdb =
      cb_apply_loaded_sf_repeatedly_to_pdb->isChecked();
  //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_bead_models_use_var_len_sf() {
  (*saxs_options).bead_models_use_var_len_sf =
      cb_bead_models_use_var_len_sf->isChecked();
  //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::update_bead_models_var_len_sf_max(
    const QString &qs) {
  (*saxs_options).bead_models_var_len_sf_max = qs.toUInt();
}

void US_Hydrodyn_SasOptionsBeadModel::update_dummy_saxs_name(const QString &) {
  bool ok = false;
  QString qs;
  if ((*saxs_options).dummy_saxs_names.size()) {
    qs = US_Static::getItem(
        us_tr("US-SOMO SAS Bead Model Options : Select dummy atom name"),
        us_tr("Select a defined dummy atom name"),
        (*saxs_options).dummy_saxs_names, 0, false, &ok, this);
  }
  if (ok && !qs.isEmpty()) {
    (*saxs_options).dummy_saxs_name = qs;
  }
  disconnect(le_dummy_saxs_name, SIGNAL(textChanged(const QString &)), 0, 0);
  le_dummy_saxs_name->setText((*saxs_options).dummy_saxs_name);
  connect(le_dummy_saxs_name, SIGNAL(textChanged(const QString &)), this,
          SLOT(update_dummy_saxs_name(const QString &)));
}

void US_Hydrodyn_SasOptionsBeadModel::set_dummy_atom_pdbs_in_nm() {
  (*saxs_options).dummy_atom_pdbs_in_nm = cb_dummy_atom_pdbs_in_nm->isChecked();
  //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
