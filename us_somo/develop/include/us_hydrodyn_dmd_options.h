#ifndef US_HYDRODYN_DMD_OPTIONS_H
#define US_HYDRODYN_DMD_OPTIONS_H

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

struct DMD_Options {
      bool force_chem;

      float threshold_pb_pb;
      float threshold_pb_sc;
      float threshold_sc_sc;

      bool pdb_static_pairs;
};

class US_EXTERN US_Hydrodyn_DMD_Options : public QFrame {
      Q_OBJECT

   public:
      US_Hydrodyn_DMD_Options(DMD_Options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_DMD_Options();

   public:
      DMD_Options *dmd_options;
      bool *dmd_widget;
      void *us_hydrodyn;
      US_Config *USglobal;

      QLabel *lbl_info;
      QLabel *lbl_info_model_creation;

      QLabel *lbl_threshold_pb_pb;
      QLabel *lbl_threshold_pb_sc;
      QLabel *lbl_threshold_sc_sc;

      QCheckBox *cb_force_chem;
      QCheckBox *cb_pdb_static_pairs;

      QwtCounter *cnt_threshold_pb_pb;
      QwtCounter *cnt_threshold_pb_sc;
      QwtCounter *cnt_threshold_sc_sc;


      QPushButton *pb_help;
      QPushButton *pb_cancel;

   private slots:

      void setupGUI();

      void update_threshold_pb_pb(double);
      void update_threshold_pb_sc(double);
      void update_threshold_sc_sc(double);

      void set_pdb_static_pairs();
      void set_force_chem();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
