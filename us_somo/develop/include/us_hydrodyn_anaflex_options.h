#ifndef US_HYDRODYN_ANAFLEX_OPTIONS_H
#define US_HYDRODYN_ANAFLEX_OPTIONS_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
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

struct Anaflex_Options {
  bool run_anaflex;
  int nfrec;
  bool instprofiles;

  bool run_mode_1;
  bool run_mode_1_1;
  bool run_mode_1_2;
  bool run_mode_1_3;
  bool run_mode_1_4;
  bool run_mode_1_5;
  bool run_mode_1_7;
  bool run_mode_1_8;
  bool run_mode_1_12;
  bool run_mode_1_13;
  bool run_mode_1_14;
  bool run_mode_1_18;
  bool run_mode_1_20;
  bool run_mode_1_24;

  bool run_mode_2;
  bool run_mode_2_1;
  bool run_mode_2_2;
  bool run_mode_2_3;
  bool run_mode_2_4;
  bool run_mode_2_5;
  bool run_mode_2_7;
  bool run_mode_2_8;
  bool run_mode_2_12;
  bool run_mode_2_13;
  bool run_mode_2_14;
  bool run_mode_2_18;
  bool run_mode_2_20;
  bool run_mode_2_24;

  bool run_mode_3;
  bool run_mode_3_1;
  bool run_mode_3_5;
  int run_mode_3_5_iii;
  int run_mode_3_5_jjj;
  bool run_mode_3_9;
  bool run_mode_3_10;
  float run_mode_3_10_theta;
  float run_mode_3_10_refractive_index;
  float run_mode_3_10_lambda;
  bool run_mode_3_14;
  int run_mode_3_14_iii;
  int run_mode_3_14_jjj;
  bool run_mode_3_15;
  bool run_mode_3_16;

  bool run_mode_4;
  bool run_mode_4_1;
  bool run_mode_4_6;
  bool run_mode_4_7;
  bool run_mode_4_8;

  bool run_mode_9;

  int ntimc;
  float tmax;
};

class US_EXTERN US_Hydrodyn_Anaflex_Options : public QFrame {
  Q_OBJECT

 public:
  US_Hydrodyn_Anaflex_Options(Anaflex_Options *, bool *, void *, QWidget *p = 0,
                              const char *name = 0);
  ~US_Hydrodyn_Anaflex_Options();

 public:
  Anaflex_Options *anaflex_options;
  bool *anaflex_widget;
  void *us_hydrodyn;
  US_Config *USglobal;

  QLabel *lbl_info;
  QLabel *lbl_nfrec;
  QLabel *lbl_run_mode;
  QLabel *lbl_ntimc;
  QLabel *lbl_tmax;
  QLabel *lbl_deltat;
  QLabel *lbl_run_mode_3_5_iii_spacing;
  QLabel *lbl_run_mode_3_5_iii;
  QLabel *lbl_run_mode_3_5_jjj_spacing;
  QLabel *lbl_run_mode_3_5_jjj;
  QLabel *lbl_run_mode_3_10_theta_spacing;
  QLabel *lbl_run_mode_3_10_theta;
  QLabel *lbl_run_mode_3_10_refractive_index_spacing;
  QLabel *lbl_run_mode_3_10_refractive_index;
  QLabel *lbl_run_mode_3_10_lambda_spacing;
  QLabel *lbl_run_mode_3_10_lambda;
  QLabel *lbl_run_mode_3_14_iii_spacing;
  QLabel *lbl_run_mode_3_14_iii;
  QLabel *lbl_run_mode_3_14_jjj_spacing;
  QLabel *lbl_run_mode_3_14_jjj;

  QCheckBox *cb_run_anaflex;
  QCheckBox *cb_instprofiles;

  QCheckBox *cb_run_mode_1;
  QCheckBox *cb_run_mode_1_1;
  QCheckBox *cb_run_mode_1_2;
  QCheckBox *cb_run_mode_1_3;
  QCheckBox *cb_run_mode_1_4;
  QCheckBox *cb_run_mode_1_5;
  QCheckBox *cb_run_mode_1_7;
  QCheckBox *cb_run_mode_1_8;
  QCheckBox *cb_run_mode_1_12;
  QCheckBox *cb_run_mode_1_13;
  QCheckBox *cb_run_mode_1_14;
  QCheckBox *cb_run_mode_1_18;
  // QCheckBox *cb_run_mode_1_20;
  // QCheckBox *cb_run_mode_1_24;

  QCheckBox *cb_run_mode_2;
  QCheckBox *cb_run_mode_2_1;
  QCheckBox *cb_run_mode_2_2;
  QCheckBox *cb_run_mode_2_3;
  QCheckBox *cb_run_mode_2_4;
  QCheckBox *cb_run_mode_2_5;
  QCheckBox *cb_run_mode_2_7;
  QCheckBox *cb_run_mode_2_8;
  QCheckBox *cb_run_mode_2_12;
  QCheckBox *cb_run_mode_2_13;
  QCheckBox *cb_run_mode_2_14;
  QCheckBox *cb_run_mode_2_18;
  // QCheckBox *cb_run_mode_2_20;
  // QCheckBox *cb_run_mode_2_24;

  QCheckBox *cb_run_mode_3;
  QCheckBox *cb_run_mode_3_1;
  QCheckBox *cb_run_mode_3_5;
  QCheckBox *cb_run_mode_3_9;
  QCheckBox *cb_run_mode_3_10;
  QCheckBox *cb_run_mode_3_14;
  QCheckBox *cb_run_mode_3_15;
  QCheckBox *cb_run_mode_3_16;

  QCheckBox *cb_run_mode_4;
  QCheckBox *cb_run_mode_4_1;
  QCheckBox *cb_run_mode_4_6;
  QCheckBox *cb_run_mode_4_7;
  QCheckBox *cb_run_mode_4_8;

  QCheckBox *cb_run_mode_9;

  QLineEdit *le_nfrec;
  QLineEdit *le_ntimc;
  QLineEdit *le_tmax;
  QLineEdit *le_deltat;
  QLineEdit *le_run_mode_3_5_iii;
  QLineEdit *le_run_mode_3_5_jjj;
  QLineEdit *le_run_mode_3_10_theta;
  QLineEdit *le_run_mode_3_10_refractive_index;
  QLineEdit *le_run_mode_3_10_lambda;
  QLineEdit *le_run_mode_3_14_iii;
  QLineEdit *le_run_mode_3_14_jjj;

  QPushButton *pb_help;
  QPushButton *pb_cancel;

 private:
  void setupGUI();

  void update_deltat();
  void update_enables();
  void update_ntimc_msg();

  QPalette label_font_ok;
  QPalette label_font_warning;

 private slots:

  void set_run_anaflex();
  void set_instprofiles();

  void set_run_mode_1();
  void set_run_mode_1_1();
  void set_run_mode_1_2();
  void set_run_mode_1_3();
  void set_run_mode_1_4();
  void set_run_mode_1_5();
  void set_run_mode_1_7();
  void set_run_mode_1_8();
  void set_run_mode_1_12();
  void set_run_mode_1_13();
  void set_run_mode_1_14();
  void set_run_mode_1_18();
  // void set_run_mode_1_20();
  // void set_run_mode_1_24();

  void set_run_mode_2();
  void set_run_mode_2_1();
  void set_run_mode_2_2();
  void set_run_mode_2_3();
  void set_run_mode_2_4();
  void set_run_mode_2_5();
  void set_run_mode_2_7();
  void set_run_mode_2_8();
  void set_run_mode_2_12();
  void set_run_mode_2_13();
  void set_run_mode_2_14();
  void set_run_mode_2_18();
  // void set_run_mode_2_20();
  // void set_run_mode_2_24();

  void set_run_mode_3();
  void set_run_mode_3_1();
  void set_run_mode_3_5();
  void set_run_mode_3_9();
  void set_run_mode_3_10();
  void set_run_mode_3_14();
  void set_run_mode_3_15();
  void set_run_mode_3_16();

  void set_run_mode_4();
  void set_run_mode_4_1();
  void set_run_mode_4_6();
  void set_run_mode_4_7();
  void set_run_mode_4_8();

  void set_run_mode_9();

  void update_nfrec(const QString &str);
  void update_ntimc(const QString &str);
  void update_tmax(const QString &str);
  void update_run_mode_3_5_iii(const QString &str);
  void update_run_mode_3_5_jjj(const QString &str);
  void update_run_mode_3_10_theta(const QString &str);
  void update_run_mode_3_10_refractive_index(const QString &str);
  void update_run_mode_3_10_lambda(const QString &str);
  void update_run_mode_3_14_iii(const QString &str);
  void update_run_mode_3_14_jjj(const QString &str);

  void cancel();
  void help();

 protected slots:

  void closeEvent(QCloseEvent *);
};

#endif
