#ifndef US_HYDRODYN_SAXS_OPTIONS_H
#define US_HYDRODYN_SAXS_OPTIONS_H

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

class US_EXTERN US_Hydrodyn_SaxsOptions : public QFrame
{
   Q_OBJECT

   public:
      friend class US_Hydrodyn;

      US_Hydrodyn_SaxsOptions(struct saxs_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_SaxsOptions();

   public:
      struct saxs_options *saxs_options;
      bool *saxs_options_widget;
      void *us_hydrodyn;
      US_Config *USglobal;

      QLabel *lbl_info;

      QLabel     *lbl_saxs_options;
      QLabel     *lbl_water_e_density;
      QLabel     *lbl_saxs_iq;
      QCheckBox  *cb_saxs_iq_native_debye;
      QCheckBox  *cb_saxs_iq_native_hybrid;
      QCheckBox  *cb_saxs_iq_native_hybrid2;
      QCheckBox  *cb_saxs_iq_native_hybrid3;
      QCheckBox  *cb_saxs_iq_hybrid_adaptive;
      QCheckBox  *cb_saxs_iq_native_fast;
      QCheckBox  *cb_saxs_iq_native_fast_compute_pr;
      QCheckBox  *cb_saxs_iq_crysol;
      QCheckBox  *cb_saxs_iq_foxs;

      QLabel     *lbl_fast_bin_size;
      QwtCounter *cnt_fast_bin_size;

      QLabel     *lbl_fast_modulation;
      QwtCounter *cnt_fast_modulation;

      QLabel     *lbl_hybrid2_q_points;
      QwtCounter *cnt_hybrid2_q_points;

      QLabel     *lbl_crysol_max_harmonics;
      QwtCounter *cnt_crysol_max_harmonics;

      QLabel     *lbl_crysol_fibonacci_grid_order;
      QwtCounter *cnt_crysol_fibonacci_grid_order;

      QLabel     *lbl_crysol_hydration_shell_contrast;
      QwtCounter *cnt_crysol_hydration_shell_contrast;

      QCheckBox  *cb_crysol_default_load_difference_intensity;
      QCheckBox  *cb_crysol_version_26;

      QLabel     *lbl_sans_options;
      QLabel     *lbl_h_scat_len;
      QLabel     *lbl_d_scat_len;
      QLabel     *lbl_h2o_scat_len_dens;
      QLabel     *lbl_d2o_scat_len_dens;
      QLabel     *lbl_d2o_conc;
      QLabel     *lbl_frac_of_exch_pep;
      QLabel     *lbl_sans_iq;
      QCheckBox  *cb_sans_iq_native_debye;
      QCheckBox  *cb_sans_iq_native_hybrid;
      QCheckBox  *cb_sans_iq_native_hybrid2;
      QCheckBox  *cb_sans_iq_native_hybrid3;
      QCheckBox  *cb_sans_iq_hybrid_adaptive;
      QCheckBox  *cb_sans_iq_native_fast;
      QCheckBox  *cb_sans_iq_native_fast_compute_pr;
      QCheckBox  *cb_sans_iq_cryson;

      QLabel *lbl_guinier;
      QLabel *lbl_qRgmax;
      QLabel *lbl_qend;
      QLabel *lbl_pointsmin;
      QLabel *lbl_pointsmax;

      QLabel *lbl_curve;
      QLabel *lbl_wavelength;
      QLabel *lbl_start_angle;
      QLabel *lbl_end_angle;
      QLabel *lbl_delta_angle;
      QLabel *lbl_start_q;
      QLabel *lbl_end_q;
      QLabel *lbl_delta_q;

      QwtCounter *cnt_water_e_density;

      QwtCounter *cnt_h_scat_len;
      QwtCounter *cnt_d_scat_len;
      QwtCounter *cnt_h2o_scat_len_dens;
      QwtCounter *cnt_d2o_scat_len_dens;
      QwtCounter *cnt_d2o_conc;
      QwtCounter *cnt_frac_of_exch_pep;

      QCheckBox  *cb_guinier_csv;
      QLineEdit  *le_guinier_csv_filename;

      QwtCounter *cnt_qRgmax;
      QwtCounter *cnt_qend;
      QwtCounter *cnt_pointsmin;
      QwtCounter *cnt_pointsmax;

      QwtCounter *cnt_wavelength;
      QwtCounter *cnt_start_angle;
      QwtCounter *cnt_end_angle;
      QwtCounter *cnt_delta_angle;
      QwtCounter *cnt_start_q;
      QwtCounter *cnt_end_q;
      QwtCounter *cnt_delta_q;

      QCheckBox  *cb_hydrate_pdb;
      QCheckBox  *cb_normalize_by_mw;

      QLabel      *lbl_bead_model_control;
      QCheckBox   *cb_compute_saxs_coeff_for_bead_models;
      QCheckBox   *cb_compute_sans_coeff_for_bead_models;
      QCheckBox   *cb_bead_model_rayleigh;
      QPushButton *pb_default_atom_filename;
      QLineEdit   *le_default_atom_filename;
      QPushButton *pb_default_hybrid_filename;
      QLineEdit   *le_default_hybrid_filename;
      QPushButton *pb_default_saxs_filename;
      QLineEdit   *le_default_saxs_filename;
      QPushButton *pb_default_rotamer_filename;
      QLineEdit   *le_default_rotamer_filename;

      QLabel      *lbl_misc;

      QLabel      *lbl_steric_clash_distance;
      QwtCounter  *cnt_steric_clash_distance;

      QCheckBox   *cb_iq_ask;

      QCheckBox   *cb_iq_scale_ask;
      QCheckBox   *cb_iqq_ask_target_grid;
      QCheckBox   *cb_iq_scale_angstrom;
      QCheckBox   *cb_iq_scale_nm;
      QCheckBox   *cb_iqq_expt_data_contains_variances;

      QCheckBox   *cb_disable_iq_scaling;
      QCheckBox   *cb_iqq_scale_linear_offset;
      QCheckBox   *cb_iqq_scale_chi2_fitting;
      QCheckBox   *cb_autocorrelate;
      QCheckBox   *cb_hybrid_radius_excl_vol;

      QLabel      *lbl_swh_excl_vol;
      QLineEdit   *le_swh_excl_vol;

      QLabel      *lbl_scale_excl_vol;
      QwtCounter  *cnt_scale_excl_vol;

      QCheckBox   *cb_subtract_radius;

      QLabel      *lbl_iqq_scale_min_maxq;
      QLineEdit   *le_iqq_scale_maxq;
      QLineEdit   *le_iqq_scale_minq;

      QCheckBox   *cb_iqq_scale_nnls;
      QCheckBox   *cb_iqq_scale_play;

      QPushButton *pb_clear_mw_cache;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      void update_q();

   private slots:
      
      void setupGUI();

      void update_water_e_density(double);
      void set_saxs_iq_native_debye();
      void set_saxs_iq_native_hybrid();
      void set_saxs_iq_native_hybrid2();
      void set_saxs_iq_native_hybrid3();
      void set_saxs_iq_hybrid_adaptive();
      void set_saxs_iq_native_fast();
      void set_saxs_iq_native_fast_compute_pr();
      void set_saxs_iq_crysol();
      void set_saxs_iq_foxs();

      void update_fast_bin_size(double);
      void update_fast_modulation(double);

      void update_hybrid2_q_points(double);

      void update_crysol_max_harmonics(double);
      void update_crysol_fibonacci_grid_order(double);
      void update_crysol_hydration_shell_contrast(double);
      void set_crysol_default_load_difference_intensity();
      void set_crysol_version_26();

      void update_h_scat_len(double);
      void update_d_scat_len(double);
      void update_h2o_scat_len_dens(double);
      void update_d2o_scat_len_dens(double);
      void update_d2o_conc(double);
      void update_frac_of_exch_pep(double);

      void set_sans_iq_native_debye();
      void set_sans_iq_native_hybrid();
      void set_sans_iq_native_hybrid2();
      void set_sans_iq_native_hybrid3();
      void set_sans_iq_hybrid_adaptive();
      void set_sans_iq_native_fast();
      void set_sans_iq_native_fast_compute_pr();
      void set_sans_iq_cryson();

      void set_guinier_csv();
      void update_guinier_csv_filename(const QString &);
      void update_qRgmax(double);
      void update_qend(double);
      void update_pointsmin(double);
      void update_pointsmax(double);

      void update_wavelength(double);      
      void update_start_angle(double);
      void update_end_angle(double);     
      void update_delta_angle(double);
      void update_start_q(double);
      void update_end_q(double);
      void update_delta_q(double);

      void set_compute_saxs_coeff_for_bead_models();
      void set_compute_sans_coeff_for_bead_models();
      void set_bead_model_rayleigh();
      void default_atom_filename();
      void default_hybrid_filename();
      void default_saxs_filename();
      void default_rotamer_filename();

      void set_hydrate_pdb();
      void set_normalize_by_mw();

      void update_steric_clash_distance(double);
      void set_iq_ask();

      void set_iq_scale_ask();
      void set_iqq_ask_target_grid();
      void set_iq_scale_angstrom();
      void set_iq_scale_nm();
      void set_iqq_expt_data_contains_variances();

      void set_disable_iq_scaling();
      void set_iqq_scale_linear_offset();
      void set_iqq_scale_chi2_fitting();
      void set_autocorrelate();
      void set_hybrid_radius_excl_vol();
      void update_swh_excl_vol(const QString &);
      void update_scale_excl_vol(double);
      void set_subtract_radius();
      void update_iqq_scale_minq(const QString &);
      void update_iqq_scale_maxq(const QString &);
      void set_iqq_scale_nnls();
      void set_iqq_scale_play();

      void clear_mw_cache();

      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};

// used for adjusting buttons/rounding for the q & angle
#define SAXS_Q_ROUNDING 10000.0f
#define SAXS_Q_BUTTONS 3

#endif

