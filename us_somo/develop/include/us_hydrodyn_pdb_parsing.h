#ifndef US_HYDRODYN_PDB_PARSING_H
#define US_HYDRODYN_PDB_PARSING_H

// QT defs:

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qstring.h>
//#include <q3frame.h>
#include <qcheckbox.h>
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


struct pdb_parsing {
      bool skip_hydrogen; // true = Skip hydrogen atoms (default: true)
      bool skip_water; // true = Skip solven water molecules (default: true)
      bool alternate; // true = Skip alternate conformations (default: true), else prompt for action
      bool find_sh; // true = find free SH, change residue coding (default: false)

      int missing_residues; // 0 = List them and stop operation
         // 1 = List them, skip residue and proceed
         // 2 = use automatic bead builder (approximate method, default)
      int missing_atoms; // 0 = List them and stop operation (fix with WHATIF)
         // 1 = List them, skip residue and proceed
         // 2 = use approximate method to generate beads (approximate method, default)
};

class US_EXTERN US_Hydrodyn_PDB_Parsing : public QFrame {
      Q_OBJECT

   public:
      US_Hydrodyn_PDB_Parsing(struct pdb_parsing *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_PDB_Parsing();

   public:
      struct pdb_parsing *pdb;
      bool *pdb_parsing_widget;
      void *us_hydrodyn;

      US_Config *USglobal;

      QLabel *lbl_info;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      QGroupBox *bg_misc;
      QGroupBox *bg_residues;
      QGroupBox *bg_atoms;

      QCheckBox *cb_skip_hydrogen;
      QCheckBox *cb_skip_water;
      QCheckBox *cb_alternate;
      QCheckBox *cb_find_sh;
      QLabel *lbl_thresh_SS;
      QLineEdit *le_thresh_SS;

      QCheckBox *cb_save_csv_on_load;
      QCheckBox *cb_residue_stop;
      QCheckBox *cb_residue_skip;
      QCheckBox *cb_residue_auto;
      QRadioButton *rb_residue_stop;
      QRadioButton *rb_residue_skip;
      QRadioButton *rb_residue_auto;
      QCheckBox *cb_atom_stop;
      QCheckBox *cb_atom_skip;
      QCheckBox *cb_atom_auto;
      QRadioButton *rb_atom_stop;
      QRadioButton *rb_atom_skip;
      QRadioButton *rb_atom_auto;

      QCheckBox *cb_use_WAT_Tf;

   private slots:

      void setupGUI();
      void update_thresh_SS(const QString &);
      void skip_hydrogen();
      void skip_water();
      void alternate();
      void find_sh();
      void save_csv_on_load();
      void residue();
      void residue(int);
      void atom();
      void use_WAT_Tf();
      void atom(int);
      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};


#endif
