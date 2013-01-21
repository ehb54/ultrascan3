#ifndef US_HYDRODYN_SAXS_HPLC_FIT_H
#define US_HYDRODYN_SAXS_HPLC_FIT_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>
#include <qtable.h>

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_hplc.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Fit : public QDialog
{
   Q_OBJECT

      friend class US_Hydrodyn_Saxs_Hplc;

   public:
      US_Hydrodyn_Saxs_Hplc_Fit(
                                US_Hydrodyn_Saxs_Hplc *hplc_win,
                                QWidget *p = 0, 
                                const char *name = 0
                                );
      ~US_Hydrodyn_Saxs_Hplc_Fit();

   private:
      US_Hydrodyn_Saxs_Hplc *hplc_win;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QCheckBox     *cb_fix_center;
      QLabel        *lbl_pct_center;
      QLineEdit     *le_pct_center;

      QCheckBox     *cb_fix_width;
      QLabel        *lbl_pct_width;
      QLineEdit     *le_pct_width;

      QCheckBox     *cb_fix_amplitude;
      QLabel        *lbl_pct_amplitude;
      QLineEdit     *le_pct_amplitude;
      
      QLabel        *lbl_fix_curves;
      QLineEdit     *le_fix_curves;

      QLabel        *lbl_epsilon;
      QLineEdit     *le_epsilon;

      QLabel        *lbl_iterations;
      QLineEdit     *le_iterations;

      QLabel        *lbl_population;
      QLineEdit     *le_population;

      QPushButton   *pb_lm;
      QPushButton   *pb_gsm_sd;
      QPushButton   *pb_gsm_ih;
      QPushButton   *pb_gsm_cg;
      QPushButton   *pb_ga;
      QPushButton   *pb_grid;

      QProgressBar  *progress;
      QPushButton   *pb_stop;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

      void          setupGUI();

      bool          running;

   private slots:

      void update_enables();

      void lm();
      void gsm_sd();
      void gsm_ih();
      void gsm_cg();
      void ga();
      void grid();

      void stop();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
