#ifndef US_HYDRODYN_SAS_OPTIONS_HYDRATION_H
#define US_HYDRODYN_SAS_OPTIONS_HYDRATION_H

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

class US_EXTERN US_Hydrodyn_SasOptionsHydration : public QFrame
{
   Q_OBJECT

   public:
      friend class US_Hydrodyn;

      US_Hydrodyn_SasOptionsHydration(struct saxs_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_SasOptionsHydration();

   public:
      struct saxs_options *saxs_options;
      bool        *sas_options_hydration_widget;
      void        *us_hydrodyn;
      US_Config   *USglobal;

      QLabel      *lbl_info;

      QCheckBox   *cb_hydrate_pdb;

      QPushButton *pb_default_rotamer_filename;
      QLineEdit   *le_default_rotamer_filename;

      QLabel      *lbl_steric_clash_distance;
      QwtCounter  *cnt_steric_clash_distance;

      QLabel      *lbl_steric_clash_recheck_distance;
      QwtCounter  *cnt_steric_clash_recheck_distance;

      QCheckBox   *cb_alt_hydration;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

   private slots:
      
      void setupGUI();

      void set_hydrate_pdb();

      void default_rotamer_filename();

      void update_steric_clash_distance(double);
      void update_steric_clash_recheck_distance(double);

      void set_alt_hydration();

      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif

