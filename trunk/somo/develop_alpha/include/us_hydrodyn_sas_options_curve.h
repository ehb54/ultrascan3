#ifndef US_HYDRODYN_SAS_OPTIONS_CURVE_H
#define US_HYDRODYN_SAS_OPTIONS_CURVE_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3frame.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <q3buttongroup.h>
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

class US_EXTERN US_Hydrodyn_SasOptionsCurve : public Q3Frame
{
   Q_OBJECT

   public:
      friend class US_Hydrodyn;

      US_Hydrodyn_SasOptionsCurve(struct saxs_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_SasOptionsCurve();

   public:
      struct saxs_options *saxs_options;
      bool       *sas_options_curve_widget;
      void       *us_hydrodyn;
      US_Config  *USglobal;

      QLabel     *lbl_info;

      QLabel     *lbl_wavelength;
      QLabel     *lbl_start_angle;
      QLabel     *lbl_end_angle;
      QLabel     *lbl_delta_angle;
      QLabel     *lbl_start_q;
      QLabel     *lbl_end_q;
      QLabel     *lbl_delta_q;

      QwtCounter *cnt_wavelength;
      QwtCounter *cnt_start_angle;
      QwtCounter *cnt_end_angle;
      QwtCounter *cnt_delta_angle;
      QwtCounter *cnt_start_q;
      QwtCounter *cnt_end_q;
      QwtCounter *cnt_delta_q;

      QCheckBox  *cb_normalize_by_mw;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      void update_q();

   private slots:
      
      void setupGUI();

      void update_wavelength(double);      
      void update_start_angle(double);
      void update_end_angle(double);     
      void update_delta_angle(double);
      void update_start_q(double);
      void update_end_q(double);
      void update_delta_q(double);

      void set_normalize_by_mw();

      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};

// used for adjusting buttons/rounding for the q & angle
#define SAXS_Q_ROUNDING 10000.0f
#define SAXS_Q_BUTTONS 3

#endif
