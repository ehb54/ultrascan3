#ifndef US_HYDRODYN_SAS_OPTIONS_BEAD_MODEL_H
#define US_HYDRODYN_SAS_OPTIONS_BEAD_MODEL_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qbuttongroup.h>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

class US_EXTERN US_Hydrodyn_SasOptionsBeadModel : public QFrame
{
   Q_OBJECT

   public:
      friend class US_Hydrodyn;

      US_Hydrodyn_SasOptionsBeadModel(struct saxs_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_SasOptionsBeadModel();

   public:
      struct saxs_options *saxs_options;

      bool        *sas_options_bead_model_widget;
      void        *us_hydrodyn;
      US_Config   *USglobal;

      QLabel      *lbl_info;

      QCheckBox   *cb_compute_saxs_coeff_for_bead_models;
      QCheckBox   *cb_compute_sans_coeff_for_bead_models;
      QCheckBox   *cb_bead_model_rayleigh;
      QCheckBox   *cb_bead_models_use_bead_radius_ev;

      QLabel      *lbl_dummy_saxs_name;
      QLineEdit   *le_dummy_saxs_name;

      QCheckBox   *cb_dummy_atom_pdbs_in_nm;
      QCheckBox   *cb_iq_global_avg_for_bead_models;
      QCheckBox   *cb_bead_models_use_quick_fitting;
      QCheckBox   *cb_bead_models_use_gsm_fitting;
      QCheckBox   *cb_apply_loaded_sf_repeatedly_to_pdb;
      QCheckBox   *cb_bead_models_use_var_len_sf;
      QLabel      *lbl_bead_models_var_len_sf_max;
      QValidator  *le_bead_models_var_len_sf_max_qv;
      QLineEdit   *le_bead_models_var_len_sf_max;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      void update_q();

   private slots:
      
      void setupGUI();

      void set_compute_saxs_coeff_for_bead_models();
      void set_compute_sans_coeff_for_bead_models();
      void set_bead_model_rayleigh();
      void set_bead_models_use_bead_radius_ev();

      void update_dummy_saxs_name( const QString & );

      void set_dummy_atom_pdbs_in_nm();
      void set_iq_global_avg_for_bead_models();
      void set_bead_models_use_quick_fitting();
      void set_bead_models_use_gsm_fitting();
      void set_apply_loaded_sf_repeatedly_to_pdb();
      void set_bead_models_use_var_len_sf();

      void update_bead_models_var_len_sf_max( const QString & );

      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif

