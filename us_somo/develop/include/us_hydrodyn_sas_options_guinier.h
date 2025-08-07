#ifndef US_HYDRODYN_SAS_OPTIONS_GUINIER_H
#define US_HYDRODYN_SAS_OPTIONS_GUINIER_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qwt_counter.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

//standard C and C++ defs:

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;

class US_EXTERN US_Hydrodyn_SasOptionsGuinier : public QFrame {
      Q_OBJECT

   public:
      friend class US_Hydrodyn;

      US_Hydrodyn_SasOptionsGuinier(struct saxs_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_SasOptionsGuinier();

   public:
      struct saxs_options *saxs_options;

      bool *sas_options_guinier_widget;
      void *us_hydrodyn;
      US_Config *USglobal;

      QLabel *lbl_guinier_and_cs_guinier;

      QLabel *lbl_qRgmax;
      QLineEdit *le_qRgmax;

      QLabel *lbl_qstart;
      QLineEdit *le_qstart;

      QLabel *lbl_qend;
      QLineEdit *le_qend;

      QLabel *lbl_cs_guinier;

      QLabel *lbl_cs_qRgmax;
      QLineEdit *le_cs_qRgmax;

      QLabel *lbl_Rt_qRtmax;
      QLineEdit *le_Rt_qRtmax;

      QLabel *lbl_cs_qstart;
      QLineEdit *le_cs_qstart;

      QLabel *lbl_cs_qend;
      QLineEdit *le_cs_qend;

      QCheckBox *cb_guinier_auto_fit;

      QLabel *lbl_pointsmin;
      QLineEdit *le_pointsmin;

      QLabel *lbl_pointsmax;
      QLineEdit *le_pointsmax;

      QCheckBox *cb_guinier_csv;
      QLineEdit *le_guinier_csv_filename;

      QCheckBox *cb_guinier_csv_save_data;

      QLabel *lbl_conc_header;

      QPushButton *pb_curve_conc;

      QLabel *lbl_conc;
      QLineEdit *le_conc;

      QLabel *lbl_psv;
      QLineEdit *le_psv;

      QCheckBox *cb_guinier_use_qRlimit;

      QCheckBox *cb_guinier_use_sd;

      QCheckBox *cb_guinier_outlier_reject;
      QLineEdit *le_guinier_outlier_reject_dist;

      QCheckBox *cb_use_cs_psv;
      QLineEdit *le_cs_psv;

      QCheckBox *cb_guinier_use_standards;

      QLabel *lbl_I0_exp;
      QLineEdit *le_I0_exp;

      QLabel *lbl_I0_theo;
      QLineEdit *le_I0_theo;

      QLabel *lbl_diffusion_len;
      QLineEdit *le_diffusion_len;

      QLabel *lbl_electron_nucleon_ratio;
      QLineEdit *le_electron_nucleon_ratio;

      QLabel *lbl_nucleon_mass;
      QLineEdit *le_nucleon_mass;

      QLabel *lbl_mwt_k;
      QLineEdit *le_mwt_k;

      QLabel *lbl_mwt_c;
      QLineEdit *le_mwt_c;

      QLabel *lbl_mwt_qmax;
      QLineEdit *le_mwt_qmax;

      QPushButton *pb_guinier;
      QPushButton *pb_cs_guinier;
      QPushButton *pb_Rt_guinier;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

   private slots:

      void setupGUI();

      void set_guinier_csv();
      void set_guinier_csv_save_data();

      void update_qRgmax(const QString &);
      void update_qstart(const QString &);
      void update_qend(const QString &);

      void update_cs_qRgmax(const QString &);
      void update_cs_qstart(const QString &);
      void update_cs_qend(const QString &);

      void update_Rt_qRtmax(const QString &);

      void set_guinier_auto_fit();
      void update_pointsmin(const QString &);
      void update_pointsmax(const QString &);
      void update_guinier_csv_filename(const QString &);

      void curve_conc();

      void update_conc(const QString &);
      void update_psv(const QString &);
      void set_use_cs_psv();
      void update_cs_psv(const QString &);

      void update_I0_exp(const QString &);
      void update_I0_theo(const QString &);
      void update_diffusion_len(const QString &);
      void update_electron_nucleon_ratio(const QString &);
      void update_nucleon_mass(const QString &);

      void update_mwt_k(const QString &);
      void update_mwt_c(const QString &);
      void update_mwt_qmax(const QString &);

      void set_guinier_outlier_reject();
      void update_guinier_outlier_reject_dist(const QString &);
      void set_guinier_use_sd();
      void set_guinier_use_qRlimit();
      void set_guinier_use_standards();

      void guinier();
      void cs_guinier();
      void Rt_guinier();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};
#endif
