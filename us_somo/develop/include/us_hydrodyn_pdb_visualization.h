#ifndef US_HYDRODYN_PDB_VISUALIZATION_H
#define US_HYDRODYN_PDB_VISUALIZATION_H

// QT defs:

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
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


struct pdb_visualization {
      int visualization; // 0 = Default RASMOL settings
         // 1 = Space-filling, colors as bead colors
         // 2 = Custom script file
      QString filename; // custom file name
};

class US_EXTERN US_Hydrodyn_PDB_Visualization : public QFrame {
      Q_OBJECT

   public:
      US_Hydrodyn_PDB_Visualization(struct pdb_visualization *, bool *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_PDB_Visualization();

   public:
      struct pdb_visualization *pdb;
      bool *pdb_visualization_widget;

      US_Config *USglobal;

      QLabel *lbl_info;

      QPushButton *pb_help;
      QPushButton *pb_cancel;
      QPushButton *pb_filename;

      QGroupBox *bg_visualization;

      QCheckBox *cb_default;
      QCheckBox *cb_spacefilling;
      QCheckBox *cb_custom;

      QRadioButton *rb_default;
      QRadioButton *rb_spacefilling;
      QRadioButton *rb_custom;

      QLineEdit *le_filename;


   private slots:

      void setupGUI();
      void select_option();
      void select_option(int);
      void select_filename();
      void update_filename(const QString &);

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};


#endif
