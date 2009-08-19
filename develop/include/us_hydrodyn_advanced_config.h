#ifndef US_HYDRODYN_ADVANCED_CONFIG_H
#define US_HYDRODYN_ADVANCED_CONFIG_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qbuttongroup.h>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

struct advanced_config
{
   bool auto_view_pdb; // on (default) will automatically load pdb
   bool scroll_editor; // side scrolling of editor on/off
   bool auto_calc_somo; // automatically calc somo on load
   bool auto_show_hydro; // automatically show hydro calc
   bool debug_1;
   bool debug_2;
   bool debug_3;
   bool debug_4;
   bool debug_5;
   bool debug_6;
   bool debug_7;
};

class US_EXTERN US_Hydrodyn_AdvancedConfig : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_AdvancedConfig(struct advanced_config *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_AdvancedConfig();

   public:
      struct advanced_config *advanced_config;
      bool *advanced_config_widget;
      void *us_hydrodyn;
      US_Config *USglobal;

      QLabel *lbl_info;
      QCheckBox *cb_auto_view_pdb;
      QCheckBox *cb_scroll_editor;
      QCheckBox *cb_auto_calc_somo;
      QCheckBox *cb_auto_show_hydro;
      QCheckBox *cb_debug_1;
      QCheckBox *cb_debug_2;
      QCheckBox *cb_debug_3;
      QPushButton *pb_help;
      QPushButton *pb_cancel;

   private slots:
      
      void setupGUI();
      void set_auto_view_pdb();
      void set_scroll_editor();
      void set_auto_calc_somo();
      void set_auto_show_hydro();
      void set_debug_1();
      void set_debug_2();
      void set_debug_3();
      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif

