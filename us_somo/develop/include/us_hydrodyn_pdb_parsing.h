#ifndef US_HYDRODYN_PDB_PARSING_H
#define US_HYDRODYN_PDB_PARSING_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
//#include <q3frame.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <set>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

// The residue names the "Skip solvent water molecules" parsing option discards.  Heavy water
// counts as solvent water just as light water does - DOD is the PDB chemical component for
// D2O, e.g. 5PTI.pdb - so both belong here and every PDB parsing path shares this one list.
//
// NOTE: "WAT" is deliberately absent.  That is SOMO's own explicit hydration water, which is
// meant to be kept, not discarded as solvent.

inline const set < QString > & pdb_parse_water_names()
{
   static const set < QString > water_names =
      {
         "HOH"    // water
         ,"DOD"   // heavy water, D2O
         ,"D2O"   // heavy water, non-standard spelling
         ,"SOL"
         ,"CIM"
      };
   return water_names;
}

inline bool pdb_parse_is_water( const QString & resName )
{
   return pdb_parse_water_names().count( resName.trimmed().toUpper() ) > 0;
}

// Deuterium atoms - the D-for-H analogue of the hydrogen test the readers apply.  Neutron
// structures name them D, DE, DZ1, DD21, ... , e.g. 5PTI.pdb.
//
// This takes the whole ATOM/HETATM line, not just the atom name, because the wwPDB element
// field ( columns 77-78 ) settles the question when it is present.  Without it a name test
// alone cannot tell deuterium from the second letter of a two-letter element - "CD  " is
// cadmium, not a deuterium - so the element field is preferred and the name is the fallback.

inline bool pdb_parse_is_deuterium( const QString & pdb_line )
{
   const QString element = pdb_line.mid( 76, 2 ).trimmed().toUpper();

   if ( !element.isEmpty() )
   {
      return element == "D";
   }

   const QString name = pdb_line.mid( 12, 4 ).trimmed().toUpper();

   return name.startsWith( "D" )                                          // D, DE, DZ1, DD21
      || ( name.size() > 1 && name[ 0 ].isDigit() && name[ 1 ] == 'D' );  // 1DG1
}

struct pdb_parsing
{
   bool skip_hydrogen;      // true = Skip hydrogen atoms (default: true)
   bool skip_deuterium;     // true = Skip deuterium atoms (default: true)
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
      
      QGroupBox *bg_misc;
      QGroupBox *bg_residues;
      QGroupBox *bg_atoms;

      QCheckBox *cb_skip_hydrogen;
      QCheckBox *cb_skip_deuterium;
      QCheckBox *cb_skip_water;
      QCheckBox *cb_alternate;
      QCheckBox *cb_find_sh;
      QLabel    *lbl_thresh_SS;
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
      void skip_deuterium();
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

