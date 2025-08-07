#ifndef US_HYDRODYN_ASA_H
#define US_HYDRODYN_ASA_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstring.h>
// #include <q3frame.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qwt_counter.h>
// Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

// standard C and C++ defs:

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <vector>

using namespace std;

struct asa_options {
  float probe_radius;
  float probe_recheck_radius;
  float threshold;
  float threshold_percent;
  float grid_threshold;
  float grid_threshold_percent;
  bool calculation;
  bool recheck_beads;
  int method;  // 0 == surfracer , 1 == asab1
  float asab1_step;
  float hydrate_probe_radius;
  float hydrate_threshold;

  float vdw_grpy_probe_radius;
  float vdw_grpy_threshold_percent;

  bool vvv;
  float vvv_probe_radius;
  float vvv_grid_dR;
};

class US_EXTERN US_Hydrodyn_ASA : public QFrame {
  Q_OBJECT

 public:
  US_Hydrodyn_ASA(struct asa_options *, bool *, void *, QWidget *p = 0,
                  const char *name = 0);
  ~US_Hydrodyn_ASA();

 public:
  struct asa_options *asa;
  bool *asa_widget;
  void *us_hydrodyn;
  US_Config *USglobal;

  QLabel *lbl_info;
  QLabel *lbl_probe_radius;
  QLabel *lbl_probe_recheck_radius;
  QLabel *lbl_asa_threshold;
  QLabel *lbl_asa_threshold_percent;
  QLabel *lbl_asa_grid_threshold;
  QLabel *lbl_asa_grid_threshold_percent;
  QLabel *lbl_vdw_grpy_probe_radius;
  QLabel *lbl_vdw_grpy_threshold_percent;
  QLabel *lbl_hydrate_probe_radius;
  QLabel *lbl_hydrate_threshold;
  QLabel *lbl_asab1_step;

  QLabel *lbl_vvv_probe_radius;
  QLabel *lbl_vvv_grid_dR;

  QwtCounter *cnt_probe_radius;
  QwtCounter *cnt_probe_recheck_radius;
  QwtCounter *cnt_asa_threshold;
  QwtCounter *cnt_asa_threshold_percent;
  QwtCounter *cnt_asa_grid_threshold;
  QwtCounter *cnt_asa_grid_threshold_percent;
  QwtCounter *cnt_vdw_grpy_probe_radius;
  QwtCounter *cnt_vdw_grpy_threshold_percent;
  QwtCounter *cnt_hydrate_probe_radius;
  QwtCounter *cnt_hydrate_threshold;
  QwtCounter *cnt_asab1_step;
  QwtCounter *cnt_vvv_probe_radius;
  QwtCounter *cnt_vvv_grid_dR;

  QCheckBox *cb_asa_calculation;
  QCheckBox *cb_bead_check;

  QCheckBox *cb_surfracer;
  QCheckBox *cb_asab1;
  QRadioButton *rb_surfracer;
  QRadioButton *rb_asab1;
  QCheckBox *cb_vvv;

  QPushButton *pb_help;
  QPushButton *pb_cancel;

  QGroupBox *bg_asa_method;

 private slots:

  void setupGUI();
  void update_probe_radius(double);
  void update_probe_recheck_radius(double);
  void update_asa_threshold(double);
  void update_asa_threshold_percent(double);
  void update_asa_grid_threshold(double);
  void update_asa_grid_threshold_percent(double);
  void update_asab1_step(double);
  void update_vdw_grpy_probe_radius(double);
  void update_vdw_grpy_threshold_percent(double);
  void update_hydrate_probe_radius(double);
  void update_hydrate_threshold(double);
  void update_vvv_probe_radius(double);
  void update_vvv_grid_dR(double);
  void select_asa_method();
  void select_asa_method(int);
  void set_asa_calculation();
  void set_bead_check();
  void set_vvv();
  void cancel();
  void help();

 protected slots:

  void closeEvent(QCloseEvent *);
};

#endif
