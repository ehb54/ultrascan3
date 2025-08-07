#ifndef US_HYDRODYN_SAS_OPTIONS_XSR_H
#define US_HYDRODYN_SAS_OPTIONS_XSR_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
//#include <q3frame.h>
#include <qlineedit.h>
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

class US_EXTERN US_Hydrodyn_SasOptionsXsr : public QFrame {
      Q_OBJECT

   public:
      friend class US_Hydrodyn;

      US_Hydrodyn_SasOptionsXsr(struct saxs_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_SasOptionsXsr();

   public:
      struct saxs_options *saxs_options;
      bool *sas_options_xsr_widget;
      void *us_hydrodyn;
      US_Config *USglobal;

      QLabel *lbl_info;


      QLabel *lbl_xsr_symmop;
      QLineEdit *le_xsr_symmop;
      QLabel *lbl_xsr_nx;
      QLineEdit *le_xsr_nx;
      QLabel *lbl_xsr_ny;
      QLineEdit *le_xsr_ny;
      QLabel *lbl_xsr_griddistance;
      QLineEdit *le_xsr_griddistance;
      QLabel *lbl_xsr_ncomponents;
      QLineEdit *le_xsr_ncomponents;
      QLabel *lbl_xsr_compactness_weight;
      QLineEdit *le_xsr_compactness_weight;
      QLabel *lbl_xsr_looseness_weight;
      QLineEdit *le_xsr_looseness_weight;
      QLabel *lbl_xsr_temperature;
      QLineEdit *le_xsr_temperature;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      void setupGUI();

   private slots:

      void update_xsr_symmop(const QString &);
      void update_xsr_nx(const QString &);
      void update_xsr_ny(const QString &);
      void update_xsr_griddistance(const QString &);
      void update_xsr_ncomponents(const QString &);
      void update_xsr_compactness_weight(const QString &);
      void update_xsr_looseness_weight(const QString &);
      void update_xsr_temperature(const QString &);

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
