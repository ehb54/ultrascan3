#ifndef US_HYDRODYN_RESULTS_H
#define US_HYDRODYN_RESULTS_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qstring.h>
// #include <q3frame.h>
// Added by qt3to4:
#include <QCloseEvent>

#include "us_editor.h"
#include "us_util.h"

#if QT_VERSION >= 0x040000
#include "us3i_editor.h"
#include "us3i_gui_settings.h"
#endif

// standard C and C++ defs:

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <vector>

using namespace std;

struct hydro_results {
  QString name;
  QString method;
  unsigned int num_models;
  float total_beads;
  float total_beads_sd;
  float used_beads;
  float used_beads_sd;
  double mass;
  double s20w;
  double s20w_sd;
  double D20w;
  double D20w_sd;
  double viscosity;
  double viscosity_sd;
  double rs;
  double rs_sd;
  double rg;
  double rg_sd;
  double tau;
  double tau_sd;
  double vbar;
  double asa_rg_pos;
  double asa_rg_neg;
  double ff0;
  double ff0_sd;
  QString solvent_name;
  QString solvent_acronym;
  double temperature;
  double solvent_viscosity;
  double solvent_density;
  double pH;
};

class US_EXTERN US_Hydrodyn_Results : public QFrame {
  Q_OBJECT

 public:
  US_Hydrodyn_Results(struct hydro_results *, bool *, void *us_hydrodyn,
                      QWidget *p = 0, const char *name = 0);
  ~US_Hydrodyn_Results();

  static hydro_results
  hydro_results_initialized();  // returns initialized hydro_results

 private:
  struct hydro_results *results;
  bool *result_widget;
  TextEdit *e;

  US_Config *USglobal;

  QLabel *lbl_info;
  QLabel *lbl_name;
  QLineEdit *le_name;
  QLabel *lbl_method;
  QLineEdit *le_method;
  QLabel *lbl_total_beads;
  QLineEdit *le_total_beads;
  QLabel *lbl_used_beads;
  QLineEdit *le_used_beads;
  QLabel *lbl_mass;
  QLineEdit *le_mass;
  QLabel *lbl_D20w;
  QLineEdit *le_D20w;
  QLabel *lbl_s20w;
  QLineEdit *le_s20w;
  QLabel *lbl_viscosity;
  QLineEdit *le_viscosity;
  QLabel *lbl_rs;
  QLineEdit *le_rs;
  QLabel *lbl_rg;
  QLineEdit *le_rg;
  QLabel *lbl_tau;
  QLineEdit *le_tau;
  QLabel *lbl_vbar;
  QLineEdit *le_vbar;
  QLabel *lbl_ff0;
  QLineEdit *le_ff0;

  QPushButton *pb_help;
  QPushButton *pb_cancel;
  QPushButton *pb_load_results;
  QPushButton *pb_load_asa;
  QPushButton *pb_load_beadmodel;

  QString somo_dir;

  void *us_hydrodyn;

 public slots:

  void cancel();

 private slots:

  void setupGUI();
  void load_results();
  void load_asa();
  void load_beadmodel();
  void help();
  void view_file(const QString &);

 protected slots:

  void closeEvent(QCloseEvent *);
};

#endif
