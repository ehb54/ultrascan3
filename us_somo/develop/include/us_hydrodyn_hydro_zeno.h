#ifndef US_HYDRODYN_HYDRO_ZENO_H
#define US_HYDRODYN_HYDRO_ZENO_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

#include "../include/us_hydrodyn_hydro.h"

class US_EXTERN US_Hydrodyn_Hydro_Zeno : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Hydro_Zeno(struct hydro_options *, 
                             bool *, 
                             void *, 
                             QWidget *p = 0, const char *name = 0);
     ~US_Hydrodyn_Hydro_Zeno();

   public:

      struct hydro_options *hydro;
      bool *hydro_zeno_widget;
      void *us_hydrodyn;

      US_Config *USglobal;

      QLabel *lbl_info;

      QCheckBox *cb_zeno_zeno;
      QLabel    *lbl_zeno_zeno_steps;
      QLineEdit *le_zeno_zeno_steps;

      QCheckBox *cb_zeno_max_cap;
      QLabel    *lbl_zeno_max_cap;
      QLineEdit *le_zeno_max_cap;

      QCheckBox *cb_zeno_interior;
      QLabel    *lbl_zeno_interior_steps;
      QLineEdit *le_zeno_interior_steps;

      QCheckBox *cb_zeno_surface;
      QLabel    *lbl_zeno_surface_steps;
      QLineEdit *le_zeno_surface_steps;

      QLabel    *lbl_zeno_surface_thickness;
      QLineEdit *le_zeno_surface_thickness;

      QCheckBox *cb_zeno_surface_thickness_from_rg;
      QLabel    *lbl_zeno_surface_thickness_from_rg_a;
      QLineEdit *le_zeno_surface_thickness_from_rg_a;
      QLabel    *lbl_zeno_surface_thickness_from_rg_b;
      QLineEdit *le_zeno_surface_thickness_from_rg_b;
      QLabel    *lbl_zeno_surface_thickness_from_rg_c;
      QLineEdit *le_zeno_surface_thickness_from_rg_c;

      QLabel    *lbl_zeno_repeats;
      QLineEdit *le_zeno_repeats;
      
      QCheckBox *cb_zeno_cxx;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      void update_enables();

   private slots:

      void setupGUI();

      void set_zeno_zeno();
      void update_zeno_zeno_steps    ( const QString &str );

      void set_zeno_max_cap();
      void update_zeno_max_cap       ( const QString &str );

      void set_zeno_interior();
      void update_zeno_interior_steps( const QString &str );

      void set_zeno_surface();
      void update_zeno_surface_steps ( const QString &str );

      void update_zeno_surface_thickness ( const QString &str );

      void set_zeno_surface_thickness_from_rg();
      void update_zeno_surface_thickness_from_rg_a ( const QString &str );
      void update_zeno_surface_thickness_from_rg_b ( const QString &str );
      void update_zeno_surface_thickness_from_rg_c ( const QString &str );

      void set_zeno_cxx();

      void update_zeno_repeats ( const QString &str );

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif

