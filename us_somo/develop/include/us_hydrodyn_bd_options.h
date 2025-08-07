#ifndef US_HYDRODYN_BD_OPTIONS_H
#define US_HYDRODYN_BD_OPTIONS_H

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

struct BD_Options {
  float threshold_pb_pb;
  float threshold_pb_sc;
  float threshold_sc_sc;
  bool do_rr;
  bool force_chem;

  int bead_size_type;  // 0 = first model, 1 = minimum, 2 = average

  bool show_pdb;
  bool run_browflex;

  int nmol;
  double tprev;  // see browflex2a.pdf for more info
  double ttraj;  // see browflex2a.pdf for more info
  double deltat;
  int npadif;
  int nconf;
  int inter;
  int iorder;
  int iseed;
  int icdm;

  int chem_pb_pb_bond_type;
  bool compute_chem_pb_pb_force_constant;
  float chem_pb_pb_force_constant;
  bool compute_chem_pb_pb_equilibrium_dist;
  float chem_pb_pb_equilibrium_dist;
  bool compute_chem_pb_pb_max_elong;
  float chem_pb_pb_max_elong;

  int chem_pb_sc_bond_type;
  bool compute_chem_pb_sc_force_constant;
  float chem_pb_sc_force_constant;
  bool compute_chem_pb_sc_equilibrium_dist;
  float chem_pb_sc_equilibrium_dist;
  bool compute_chem_pb_sc_max_elong;
  float chem_pb_sc_max_elong;

  int chem_sc_sc_bond_type;
  bool compute_chem_sc_sc_force_constant;
  float chem_sc_sc_force_constant;
  bool compute_chem_sc_sc_equilibrium_dist;
  float chem_sc_sc_equilibrium_dist;
  bool compute_chem_sc_sc_max_elong;
  float chem_sc_sc_max_elong;

  int pb_pb_bond_type;
  bool compute_pb_pb_force_constant;
  float pb_pb_force_constant;
  bool compute_pb_pb_equilibrium_dist;
  float pb_pb_equilibrium_dist;
  bool compute_pb_pb_max_elong;
  float pb_pb_max_elong;

  int pb_sc_bond_type;
  bool compute_pb_sc_force_constant;
  float pb_sc_force_constant;
  bool compute_pb_sc_equilibrium_dist;
  float pb_sc_equilibrium_dist;
  bool compute_pb_sc_max_elong;
  float pb_sc_max_elong;

  int sc_sc_bond_type;
  bool compute_sc_sc_force_constant;
  float sc_sc_force_constant;
  bool compute_sc_sc_equilibrium_dist;
  float sc_sc_equilibrium_dist;
  bool compute_sc_sc_max_elong;
  float sc_sc_max_elong;
};

class US_EXTERN US_Hydrodyn_BD_Options : public QFrame {
  Q_OBJECT

 public:
  US_Hydrodyn_BD_Options(BD_Options *, bool *, void *, QWidget *p = 0,
                         const char *name = 0);
  ~US_Hydrodyn_BD_Options();

 public:
  BD_Options *bd_options;
  bool *bd_widget;
  void *us_hydrodyn;
  US_Config *USglobal;

  QLabel *lbl_info;
  QLabel *lbl_info_model_creation;
  QLabel *lbl_info_simulation_opts;
  QLabel *lbl_bd_threshold_pb_pb;
  QLabel *lbl_bd_threshold_pb_sc;
  QLabel *lbl_bd_threshold_sc_sc;
  QLabel *lbl_nmol;
  QLabel *lbl_npadif;
  QLabel *lbl_nconf;
  QLabel *lbl_iseed;
  QLabel *lbl_tprev;
  QLabel *lbl_ttraj;
  QLabel *lbl_deltat;
  QLabel *lbl_springs;

  QwtCounter *cnt_bd_threshold_pb_pb;
  QwtCounter *cnt_bd_threshold_pb_sc;
  QwtCounter *cnt_bd_threshold_sc_sc;

  QLineEdit *le_tprev;
  QLineEdit *le_ttraj;
  QLineEdit *le_deltat;
  QwtCounter *cnt_npadif;
  QLineEdit *le_nmol;
  QLineEdit *le_nconf;
  QLineEdit *le_iseed;

  QGroupBox *bg_bead_size_type;
  QCheckBox *cb_bead_size_type_1st;
  QCheckBox *cb_bead_size_type_min;
  QCheckBox *cb_bead_size_type_avg;

  QRadioButton *rb_bead_size_type_1st;
  QRadioButton *rb_bead_size_type_min;
  QRadioButton *rb_bead_size_type_avg;

  QGroupBox *bg_inter;
  QCheckBox *cb_inter_no_hi;
  QCheckBox *cb_inter_os;
  QCheckBox *cb_inter_mos;

  QRadioButton *rb_inter_no_hi;
  QRadioButton *rb_inter_os;
  QRadioButton *rb_inter_mos;

  QGroupBox *bg_iorder;
  QCheckBox *cb_iorder_em;
  QCheckBox *cb_iorder_igt;
  QRadioButton *rb_iorder_em;
  QRadioButton *rb_iorder_igt;

  QCheckBox *cb_do_rr;
  QCheckBox *cb_force_chem;
  QCheckBox *cb_show_pdb;
  QCheckBox *cb_run_browflex;
  QCheckBox *cb_icdm;
  QCheckBox *cb_compute_chem_pb_pb_force_constant;
  QCheckBox *cb_compute_chem_pb_pb_equilibrium_dist;
  QCheckBox *cb_compute_chem_pb_pb_max_elong;
  QCheckBox *cb_compute_chem_pb_sc_force_constant;
  QCheckBox *cb_compute_chem_pb_sc_equilibrium_dist;
  QCheckBox *cb_compute_chem_pb_sc_max_elong;
  QCheckBox *cb_compute_chem_sc_sc_force_constant;
  QCheckBox *cb_compute_chem_sc_sc_equilibrium_dist;
  QCheckBox *cb_compute_chem_sc_sc_max_elong;
  QCheckBox *cb_compute_pb_pb_force_constant;
  QCheckBox *cb_compute_pb_pb_equilibrium_dist;
  QCheckBox *cb_compute_pb_pb_max_elong;
  QCheckBox *cb_compute_pb_sc_force_constant;
  QCheckBox *cb_compute_pb_sc_equilibrium_dist;
  QCheckBox *cb_compute_pb_sc_max_elong;
  QCheckBox *cb_compute_sc_sc_force_constant;
  QCheckBox *cb_compute_sc_sc_equilibrium_dist;
  QCheckBox *cb_compute_sc_sc_max_elong;

  QLabel *lbl_chem_pb_pb;
  QPushButton *pb_dup_fraenkel;
  QGroupBox *bg_chem_pb_pb_bond_types;
  QCheckBox *cb_chem_pb_pb_bond_type_fraenkel;
  QCheckBox *cb_chem_pb_pb_bond_type_hookean;
  QCheckBox *cb_chem_pb_pb_bond_type_fene;
  QCheckBox *cb_chem_pb_pb_bond_type_hard_fene;
  QRadioButton *rb_chem_pb_pb_bond_type_fraenkel;
  QRadioButton *rb_chem_pb_pb_bond_type_hookean;
  QRadioButton *rb_chem_pb_pb_bond_type_fene;
  QRadioButton *rb_chem_pb_pb_bond_type_hard_fene;
  QLabel *lbl_chem_pb_pb_force_constant;
  QLineEdit *le_chem_pb_pb_force_constant;
  QLabel *lbl_chem_pb_pb_equilibrium_dist;
  QLineEdit *le_chem_pb_pb_equilibrium_dist;
  QLabel *lbl_chem_pb_pb_max_elong;
  QLineEdit *le_chem_pb_pb_max_elong;

  QLabel *lbl_chem_pb_sc;
  QGroupBox *bg_chem_pb_sc_bond_types;
  QCheckBox *cb_chem_pb_sc_bond_type_fraenkel;
  QCheckBox *cb_chem_pb_sc_bond_type_hookean;
  QCheckBox *cb_chem_pb_sc_bond_type_fene;
  QCheckBox *cb_chem_pb_sc_bond_type_hard_fene;
  QRadioButton *rb_chem_pb_sc_bond_type_fraenkel;
  QRadioButton *rb_chem_pb_sc_bond_type_hookean;
  QRadioButton *rb_chem_pb_sc_bond_type_fene;
  QRadioButton *rb_chem_pb_sc_bond_type_hard_fene;
  QLabel *lbl_chem_pb_sc_force_constant;
  QLineEdit *le_chem_pb_sc_force_constant;
  QLabel *lbl_chem_pb_sc_equilibrium_dist;
  QLineEdit *le_chem_pb_sc_equilibrium_dist;
  QLabel *lbl_chem_pb_sc_max_elong;
  QLineEdit *le_chem_pb_sc_max_elong;

  QLabel *lbl_chem_sc_sc;
  QGroupBox *bg_chem_sc_sc_bond_types;
  QCheckBox *cb_chem_sc_sc_bond_type_fraenkel;
  QCheckBox *cb_chem_sc_sc_bond_type_hookean;
  QCheckBox *cb_chem_sc_sc_bond_type_fene;
  QCheckBox *cb_chem_sc_sc_bond_type_hard_fene;
  QRadioButton *rb_chem_sc_sc_bond_type_fraenkel;
  QRadioButton *rb_chem_sc_sc_bond_type_hookean;
  QRadioButton *rb_chem_sc_sc_bond_type_fene;
  QRadioButton *rb_chem_sc_sc_bond_type_hard_fene;
  QLabel *lbl_chem_sc_sc_force_constant;
  QLineEdit *le_chem_sc_sc_force_constant;
  QLabel *lbl_chem_sc_sc_equilibrium_dist;
  QLineEdit *le_chem_sc_sc_equilibrium_dist;
  QLabel *lbl_chem_sc_sc_max_elong;
  QLineEdit *le_chem_sc_sc_max_elong;

  QLabel *lbl_pb_pb;
  QGroupBox *bg_pb_pb_bond_types;
  QCheckBox *cb_pb_pb_bond_type_fraenkel;
  QCheckBox *cb_pb_pb_bond_type_hookean;
  QCheckBox *cb_pb_pb_bond_type_fene;
  QCheckBox *cb_pb_pb_bond_type_hard_fene;
  QRadioButton *rb_pb_pb_bond_type_fraenkel;
  QRadioButton *rb_pb_pb_bond_type_hookean;
  QRadioButton *rb_pb_pb_bond_type_fene;
  QRadioButton *rb_pb_pb_bond_type_hard_fene;
  QLabel *lbl_pb_pb_force_constant;
  QLineEdit *le_pb_pb_force_constant;
  QLabel *lbl_pb_pb_equilibrium_dist;
  QLineEdit *le_pb_pb_equilibrium_dist;
  QLabel *lbl_pb_pb_max_elong;
  QLineEdit *le_pb_pb_max_elong;

  QLabel *lbl_pb_sc;
  QGroupBox *bg_pb_sc_bond_types;
  QCheckBox *cb_pb_sc_bond_type_fraenkel;
  QCheckBox *cb_pb_sc_bond_type_hookean;
  QCheckBox *cb_pb_sc_bond_type_fene;
  QCheckBox *cb_pb_sc_bond_type_hard_fene;
  QRadioButton *rb_pb_sc_bond_type_fraenkel;
  QRadioButton *rb_pb_sc_bond_type_hookean;
  QRadioButton *rb_pb_sc_bond_type_fene;
  QRadioButton *rb_pb_sc_bond_type_hard_fene;
  QLabel *lbl_pb_sc_force_constant;
  QLineEdit *le_pb_sc_force_constant;
  QLabel *lbl_pb_sc_equilibrium_dist;
  QLineEdit *le_pb_sc_equilibrium_dist;
  QLabel *lbl_pb_sc_max_elong;
  QLineEdit *le_pb_sc_max_elong;

  QLabel *lbl_sc_sc;
  QGroupBox *bg_sc_sc_bond_types;
  QCheckBox *cb_sc_sc_bond_type_fraenkel;
  QCheckBox *cb_sc_sc_bond_type_hookean;
  QCheckBox *cb_sc_sc_bond_type_fene;
  QCheckBox *cb_sc_sc_bond_type_hard_fene;
  QRadioButton *rb_sc_sc_bond_type_fraenkel;
  QRadioButton *rb_sc_sc_bond_type_hookean;
  QRadioButton *rb_sc_sc_bond_type_fene;
  QRadioButton *rb_sc_sc_bond_type_hard_fene;
  QLabel *lbl_sc_sc_force_constant;
  QLineEdit *le_sc_sc_force_constant;
  QLabel *lbl_sc_sc_equilibrium_dist;
  QLineEdit *le_sc_sc_equilibrium_dist;
  QLabel *lbl_sc_sc_max_elong;
  QLineEdit *le_sc_sc_max_elong;

  QPushButton *pb_help;
  QPushButton *pb_cancel;

 private:
  QPalette label_font_ok;
  QPalette label_font_warning;

 private slots:

  void setupGUI();

  void update_enables();
  void update_labels();

  void update_bd_threshold_pb_pb(double);
  void update_bd_threshold_pb_sc(double);
  void update_bd_threshold_sc_sc(double);
  void set_do_rr();
  void set_force_chem();
  void set_show_pdb();
  void set_run_browflex();
  void set_icdm();
  void set_compute_chem_pb_pb_force_constant();
  void set_compute_chem_pb_pb_equilibrium_dist();
  void set_compute_chem_pb_pb_max_elong();
  void set_compute_chem_pb_sc_force_constant();
  void set_compute_chem_pb_sc_equilibrium_dist();
  void set_compute_chem_pb_sc_max_elong();
  void set_compute_chem_sc_sc_force_constant();
  void set_compute_chem_sc_sc_equilibrium_dist();
  void set_compute_chem_sc_sc_max_elong();
  void set_compute_pb_pb_force_constant();
  void set_compute_pb_pb_equilibrium_dist();
  void set_compute_pb_pb_max_elong();
  void set_compute_pb_sc_force_constant();
  void set_compute_pb_sc_equilibrium_dist();
  void set_compute_pb_sc_max_elong();
  void set_compute_sc_sc_force_constant();
  void set_compute_sc_sc_equilibrium_dist();
  void set_compute_sc_sc_max_elong();

  void update_tprev(const QString &str);
  void update_ttraj(const QString &str);
  void update_deltat(const QString &str);

  void update_nmol(const QString &str);
  void update_npadif(double);
  void update_nconf(const QString &str);
  void update_iseed(const QString &str);

  void set_bead_size_type();
  void set_bead_size_type(int);

  void set_iorder();
  void set_iorder(int);
  void set_inter();
  void set_inter(int);

  void set_chem_pb_pb_bond_types();
  void set_chem_pb_pb_bond_types(int);
  void set_chem_pb_sc_bond_types();
  void set_chem_pb_sc_bond_types(int);
  void set_chem_sc_sc_bond_types();
  void set_chem_sc_sc_bond_types(int);
  void set_pb_pb_bond_types();
  void set_pb_pb_bond_types(int);
  void set_pb_sc_bond_types();
  void set_pb_sc_bond_types(int);
  void set_sc_sc_bond_types();
  void set_sc_sc_bond_types(int);

  void update_chem_pb_pb_force_constant(const QString &str);
  void update_chem_pb_pb_equilibrium_dist(const QString &str);
  void update_chem_pb_pb_max_elong(const QString &str);
  void update_chem_pb_sc_force_constant(const QString &str);
  void update_chem_pb_sc_equilibrium_dist(const QString &str);
  void update_chem_pb_sc_max_elong(const QString &str);
  void update_chem_sc_sc_force_constant(const QString &str);
  void update_chem_sc_sc_equilibrium_dist(const QString &str);
  void update_chem_sc_sc_max_elong(const QString &str);
  void update_pb_pb_force_constant(const QString &str);
  void update_pb_pb_equilibrium_dist(const QString &str);
  void update_pb_pb_max_elong(const QString &str);
  void update_pb_sc_force_constant(const QString &str);
  void update_pb_sc_equilibrium_dist(const QString &str);
  void update_pb_sc_max_elong(const QString &str);
  void update_sc_sc_force_constant(const QString &str);
  void update_sc_sc_equilibrium_dist(const QString &str);
  void update_sc_sc_max_elong(const QString &str);

  void dup_fraenkel();

  void cancel();
  void help();

 protected slots:

  void closeEvent(QCloseEvent *);
};

#endif
