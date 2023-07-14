#ifndef US_HYDRODYN_SAS_OPTIONS_SAXS_H
#define US_HYDRODYN_SAS_OPTIONS_SAXS_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

class US_EXTERN US_Hydrodyn_SasOptionsSaxs : public QFrame
{
   Q_OBJECT

   public:
      friend class US_Hydrodyn;

      US_Hydrodyn_SasOptionsSaxs(struct saxs_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_SasOptionsSaxs();

   public:
      struct saxs_options *saxs_options;

      bool       *sas_options_saxs_widget;
      void       *us_hydrodyn;
      US_Config  *USglobal;

      QLabel     *lbl_info;

      QLabel     *lbl_water_e_density;
      QwtCounter *cnt_water_e_density;
      QLabel     *lbl_saxs_iq;
      QCheckBox  *cb_saxs_iq_native_debye;
      QCheckBox  *cb_saxs_iq_native_sh;
      QCheckBox  *cb_saxs_iq_native_hybrid;
      QCheckBox  *cb_saxs_iq_native_hybrid2;
      QCheckBox  *cb_saxs_iq_native_hybrid3;
      QCheckBox  *cb_saxs_iq_hybrid_adaptive;
      QCheckBox  *cb_saxs_iq_native_fast;
      QCheckBox  *cb_saxs_iq_native_fast_compute_pr;
      QCheckBox  *cb_saxs_iq_crysol;
      QCheckBox  *cb_saxs_iq_foxs;
      QCheckBox  *cb_saxs_iq_sastbx;

      QLabel     *lbl_fast_bin_size;
      QwtCounter *cnt_fast_bin_size;

      QLabel     *lbl_fast_modulation;
      QwtCounter *cnt_fast_modulation;

      QLabel     *lbl_hybrid2_q_points;
      QwtCounter *cnt_hybrid2_q_points;

      QLabel     *lbl_sh_max_harmonics;
      QwtCounter *cnt_sh_max_harmonics;

      QLabel     *lbl_sh_fibonacci_grid_order;
      QwtCounter *cnt_sh_fibonacci_grid_order;

      QLabel     *lbl_crysol_hydration_shell_contrast;
      QwtCounter *cnt_crysol_hydration_shell_contrast;

      QLabel     *lbl_crysol;
      QCheckBox  *cb_crysol_default_load_difference_intensity;
      QCheckBox  *cb_crysol_version_26;
      QCheckBox  *cb_crysol_version_3;
      QCheckBox  *cb_crysol_water_dummy_beads;
      QCheckBox  *cb_crysol_explicit_hydrogens;

      QLabel      *lbl_ra;
      QLineEdit   *le_ra;

      QLabel      *lbl_vol;
      QLineEdit   *le_vol;

      QPushButton *pb_crysol_target;
      QLineEdit   *le_crysol_target;

      QLabel       *lbl_sastbx_method;
      QGroupBox *bg_sastbx_method;
      QRadioButton *rb_sastbx_method_she;
      QRadioButton *rb_sastbx_method_debye;
      QRadioButton *rb_sastbx_method_zernike;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      void update_q();

      bool        started_in_expert_mode;

   private slots:
      
      void setupGUI();

      void update_water_e_density(double);
      void set_saxs_iq_native_debye();
      void set_saxs_iq_native_sh();
      void set_saxs_iq_native_hybrid();
      void set_saxs_iq_native_hybrid2();
      void set_saxs_iq_native_hybrid3();
      void set_saxs_iq_hybrid_adaptive();
      void set_saxs_iq_native_fast();
      void set_saxs_iq_native_fast_compute_pr();
      void set_saxs_iq_crysol();
      void set_saxs_iq_foxs();
      void set_saxs_iq_sastbx();

      void update_fast_bin_size(double);
      void update_fast_modulation(double);

      void update_hybrid2_q_points(double);

      void update_sh_max_harmonics(double);
      void update_sh_fibonacci_grid_order(double);
      void update_crysol_hydration_shell_contrast(double);
      void set_crysol_default_load_difference_intensity();
      void set_crysol_version_26();
      void set_crysol_version_3();
      void set_crysol_water_dummy_beads();
      void set_crysol_explicit_hydrogens();

      void crysol_target();

      void set_sastbx_method();
      void set_sastbx_method( int );

      void set_ra ( const QString & );
      void set_vol( const QString & );

      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif

