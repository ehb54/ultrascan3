#ifndef US_HYDRODYN_PDB_PARSING_H
#define US_HYDRODYN_PDB_PARSING_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qframe.h>
#include <qcheckbox.h>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;


struct pdb_parsing
{
   bool skip_hydrogen;      // true = Skip hydrogen atoms (default: true)
   bool skip_water;         // true = Skip solven water molecules (default: true)
   bool alternate;         // true = Skip alternate conformations (default: true), else prompt for action
   bool find_sh;            // true = find free SH, change residue coding (default: false)

   int missing_residues;   // 0 = List them and stop operation
                           // 1 = List them, skip residue and proceed
                           // 2 = use automatic bead builder (approximate method, default)
   int missing_atoms;      // 0 = List them and stop operation (fix with WHATIF)
                           // 1 = List them, skip residue and proceed
                           // 2 = use approximate method to generate beads (approximate method, default)
};

class US_EXTERN US_Hydrodyn_PDB_Parsing : public QFrame
{
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
      
      QButtonGroup *bg_misc;
      QButtonGroup *bg_residues;
      QButtonGroup *bg_atoms;

      QCheckBox *cb_skip_hydrogen;
      QCheckBox *cb_skip_water;
      QCheckBox *cb_alternate;
      QCheckBox *cb_find_sh;
      QCheckBox *cb_residue_stop;
      QCheckBox *cb_residue_skip;
      QCheckBox *cb_residue_auto;
      QCheckBox *cb_atom_stop;
      QCheckBox *cb_atom_skip;
      QCheckBox *cb_atom_auto;
      
      
   private slots:
      
      void setupGUI();
      void skip_hydrogen();
      void skip_water();
      void alternate();
      void find_sh();
      void residue(int);
      void atom(int);
      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};



#endif

