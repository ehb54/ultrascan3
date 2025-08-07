#ifndef US_HYDRODYN_BEAD_OUTPUT_H
#define US_HYDRODYN_BEAD_OUTPUT_H

// QT defs:

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstring.h>
// #include <q3frame.h>
#include <qcheckbox.h>
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

#define US_HYDRODYN_OUTPUT_SOMO (1 << 0)
#define US_HYDRODYN_OUTPUT_BEAMS (1 << 1)
#define US_HYDRODYN_OUTPUT_HYDRO (1 << 2)
#define US_HYDRODYN_OUTPUT_GRPY (1 << 3)

struct bead_output_options {
  int output;           // 0 = SOMO
                        // 1 = BEAMS
                        // 2 = HYDRO
                        // 3 = GRPY
  int sequence;         // 0 = as in original pdb file
                        // 1 = exposed sidechain -> exposed main chain -> buried
                        // 2 = include bead-original residue correspondence
  bool correspondence;  // true = include bead - original residue correspondence
                        // (default=true) can only be selected when BEAMS output
                        // format is selected
};

class US_EXTERN US_Hydrodyn_Bead_Output : public QFrame {
  Q_OBJECT

 public:
  US_Hydrodyn_Bead_Output(struct bead_output_options *, bool *, void *,
                          QWidget *p = 0, const char *name = 0);
  ~US_Hydrodyn_Bead_Output();

 public:
  struct bead_output_options *bead_output;
  bool *bead_output_widget;
  void *us_hydrodyn;

  US_Config *USglobal;

  QLabel *lbl_info;

  QPushButton *pb_help;
  QPushButton *pb_cancel;

  QGroupBox *bg_output;
  QGroupBox *bg_sequence;
  QGroupBox *bg_beams;

  QCheckBox *cb_somo_output;
  QCheckBox *cb_beams_output;
  QCheckBox *cb_hydro_output;
  QCheckBox *cb_grpy_output;

  QCheckBox *cb_pdb_sequence;
  QCheckBox *cb_chain_sequence;

  QRadioButton *rb_pdb_sequence;
  QRadioButton *rb_chain_sequence;

  QCheckBox *cb_correspondence;

 private slots:

  void setupGUI();
  void select_output_somo();
  void select_output_beams();
  void select_output_hydro();
  void select_output_grpy();
  void select_output(int);
  void select_sequence();
  void select_sequence(int);
  void select_correspondence();
  void cancel();
  void help();

 protected slots:

  void closeEvent(QCloseEvent *);
};

#endif
