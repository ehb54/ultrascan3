#ifndef US_HYDRODYN_SAS_OPTIONS_BEAD_MODEL_H
#define US_HYDRODYN_SAS_OPTIONS_BEAD_MODEL_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
// #include <q3frame.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qwt_counter.h>
// Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

// standard C and C++ defs:

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <vector>

using namespace std;

class US_EXTERN US_Hydrodyn_SasOptionsBeadModel : public QFrame {
  Q_OBJECT

 public:
  friend class US_Hydrodyn;

  US_Hydrodyn_SasOptionsBeadModel(struct saxs_options *, bool *, void *,
                                  QWidget *p = 0, const char *name = 0);
  ~US_Hydrodyn_SasOptionsBeadModel();

 public:
  struct saxs_options *saxs_options;

  bool *sas_options_bead_model_widget;
  void *us_hydrodyn;
  US_Config *USglobal;

  QLabel *lbl_info;

  QCheckBox *cb_compute_saxs_coeff_for_bead_models;
  QCheckBox *cb_compute_sans_coeff_for_bead_models;
  QCheckBox *cb_bead_model_rayleigh;
  QCheckBox *cb_bead_models_use_bead_radius_ev;
  QCheckBox *cb_bead_models_rho0_in_scat_factors;

  QLabel *lbl_dummy_saxs_name;
  QLineEdit *le_dummy_saxs_name;

  QCheckBox *cb_dummy_atom_pdbs_in_nm;
  QCheckBox *cb_iq_global_avg_for_bead_models;
  QCheckBox *cb_bead_models_use_quick_fitting;
  QCheckBox *cb_bead_models_use_gsm_fitting;
  QCheckBox *cb_apply_loaded_sf_repeatedly_to_pdb;
  QCheckBox *cb_bead_models_use_var_len_sf;
  QLabel *lbl_bead_models_var_len_sf_max;
  QValidator *le_bead_models_var_len_sf_max_qv;
  QLineEdit *le_bead_models_var_len_sf_max;

  QPushButton *pb_help;
  QPushButton *pb_cancel;

  void update_q();

  bool started_in_expert_mode;

 private slots:

  void setupGUI();

  void set_compute_saxs_coeff_for_bead_models();
  void set_compute_sans_coeff_for_bead_models();
  void set_bead_model_rayleigh();
  void set_bead_models_use_bead_radius_ev();
  void set_bead_models_rho0_in_scat_factors();

  void update_dummy_saxs_name(const QString &);

  void set_dummy_atom_pdbs_in_nm();
  void set_iq_global_avg_for_bead_models();
  void set_bead_models_use_quick_fitting();
  void set_bead_models_use_gsm_fitting();
  void set_apply_loaded_sf_repeatedly_to_pdb();
  void set_bead_models_use_var_len_sf();

  void update_bead_models_var_len_sf_max(const QString &);

  void cancel();
  void help();

 protected slots:

  void closeEvent(QCloseEvent *);
};

#endif
