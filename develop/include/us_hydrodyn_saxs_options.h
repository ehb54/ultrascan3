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
      US_Hydrodyn_SaxsOptions(struct saxs_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_SaxsOptions();

   public:
      struct saxs_options *saxs_options;
      bool *saxs_options_widget;
      void *us_hydrodyn;
      US_Config *USglobal;

      QLabel *lbl_info;

      QLabel *lbl_saxs_options;
      QLabel *lbl_water_e_density;

      QLabel *lbl_sans_options;
      QLabel *lbl_h_scat_len;
      QLabel *lbl_d_scat_len;
      QLabel *lbl_h2o_scat_len_dens;
      QLabel *lbl_d2o_scat_len_dens;
      QLabel *lbl_d2o_conc;
      QLabel *lbl_frac_of_exch_pep;

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


      QCheckBox *cb_hydrate_pdb;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      void update_q();

   private slots:
      
      void setupGUI();

      void update_water_e_density(double);

      void update_h_scat_len(double);
      void update_d_scat_len(double);
      void update_h2o_scat_len_dens(double);
      void update_d2o_scat_len_dens(double);
      void update_d2o_conc(double);
      void update_frac_of_exch_pep(double);

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

      void set_hydrate_pdb();

      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};

// used for adjusting buttons/rounding for the q & angle
#define SAXS_Q_ROUNDING 1000.0f
#define SAXS_Q_BUTTONS 3

#endif

