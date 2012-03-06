#ifndef US_HYDRODYN_SAS_OPTIONS_SANS_H
#define US_HYDRODYN_SAS_OPTIONS_SANS_H

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

class US_EXTERN US_Hydrodyn_SasOptionsSans : public QFrame
{
   Q_OBJECT

   public:
      friend class US_Hydrodyn;

      US_Hydrodyn_SasOptionsSans(struct saxs_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_SasOptionsSans();

   public:
      struct saxs_options *saxs_options;
      bool       *sas_options_sans_widget;
      void       *us_hydrodyn;
      US_Config  *USglobal;

      QLabel     *lbl_info;

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

      QwtCounter *cnt_h_scat_len;
      QwtCounter *cnt_d_scat_len;
      QwtCounter *cnt_h2o_scat_len_dens;
      QwtCounter *cnt_d2o_scat_len_dens;
      QwtCounter *cnt_d2o_conc;
      QwtCounter *cnt_frac_of_exch_pep;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

   private slots:
      
      void setupGUI();

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

      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif

