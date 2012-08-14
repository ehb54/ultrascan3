#ifndef US_HYDRODYN_SAS_OPTIONS_MISC_H
#define US_HYDRODYN_SAS_OPTIONS_MISC_H

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

class US_EXTERN US_Hydrodyn_SasOptionsMisc : public QFrame
{
   Q_OBJECT

   public:
      friend class US_Hydrodyn;

      US_Hydrodyn_SasOptionsMisc(struct saxs_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_SasOptionsMisc();

   public:
      struct saxs_options *saxs_options;

      bool        *sas_options_misc_widget;
      void        *us_hydrodyn;
      US_Config   *USglobal;

      QLabel      *lbl_info;

      QPushButton *pb_default_atom_filename;
      QLineEdit   *le_default_atom_filename;
      QPushButton *pb_default_hybrid_filename;
      QLineEdit   *le_default_hybrid_filename;
      QPushButton *pb_default_saxs_filename;
      QLineEdit   *le_default_saxs_filename;
      QPushButton *pb_default_ff_filename;
      QLineEdit   *le_default_ff_filename;

      QCheckBox   *cb_iq_ask;

      QCheckBox   *cb_iq_scale_ask;
      QCheckBox   *cb_iqq_ask_target_grid;
      QCheckBox   *cb_iq_scale_angstrom;
      QCheckBox   *cb_iq_scale_nm;
      QCheckBox   *cb_iqq_expt_data_contains_variances;
      QCheckBox   *cb_iqq_kratky_fit;
      QCheckBox   *cb_ignore_errors;

      QCheckBox   *cb_disable_iq_scaling;
      QCheckBox   *cb_iqq_scale_chi2_fitting;

      QLabel      *lbl_swh_excl_vol;
      QLineEdit   *le_swh_excl_vol;

      QLabel      *lbl_scale_excl_vol;
      QwtCounter  *cnt_scale_excl_vol;

      QCheckBox   *cb_use_iq_target_ev;
      QLineEdit   *le_iq_target_ev;

      QLabel      *lbl_iqq_scale_min_maxq;
      QLineEdit   *le_iqq_scale_maxq;
      QLineEdit   *le_iqq_scale_minq;

      QPushButton *pb_clear_mw_cache;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

   private slots:
      
      void setupGUI();

      void default_atom_filename();
      void default_hybrid_filename();
      void default_saxs_filename();
      void default_ff_filename();

      void set_iq_ask();

      void set_iq_scale_ask();
      void set_iqq_ask_target_grid();
      void set_iq_scale_angstrom();
      void set_iq_scale_nm();
      void set_iqq_expt_data_contains_variances();
      void set_iqq_kratky_fit();
      void set_ignore_errors();

      void set_disable_iq_scaling();
      void set_iqq_scale_chi2_fitting();
      void update_swh_excl_vol(const QString &);
      void update_scale_excl_vol(double);
      void set_use_iq_target_ev();
      void update_iq_target_ev(const QString &);
      void update_iqq_scale_minq(const QString &);
      void update_iqq_scale_maxq(const QString &);

      void clear_mw_cache();

      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif

