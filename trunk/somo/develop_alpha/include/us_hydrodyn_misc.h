#ifndef US_HYDRODYN_MISC_H
#define US_HYDRODYN_MISC_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3buttongroup.h>
#include <q3frame.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
//Added by qt3to4:
#include <QCloseEvent>
#ifndef NO_DB
#  include "us_db_tbl_vbar.h"
#endif

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

struct misc_options
{
   double vbar;
   double vbar_temperature;
   double hydrovol;
   bool compute_vbar;      // true = compute
                                // false = use user specified value
   bool pb_rule_on;      // true = move N back etc, false = do not

   double avg_hydration;
   double avg_mass;
   double avg_volume;
   double avg_radius;
   double avg_vbar;

   double target_e_density;  // non zero 
   double target_volume;
   bool   set_target_on_load_pdb;
   bool   equalize_radii;
   bool   hydro_supc;
   bool   hydro_zeno;
   bool   export_msroll;
};

class US_EXTERN US_Hydrodyn_Misc : public Q3Frame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Misc(struct misc_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Misc();

   public:

      struct misc_options *misc;
      bool *misc_widget;
                void *us_hydrodyn;

      US_Config *USglobal;
#ifndef NO_DB
      US_Vbar_DB *vbar_dlg;
#endif

      QLabel *lbl_info;
      QLabel *lbl_vbar;
      QLabel *lbl_vbar_temperature;
      QLabel *lbl_hydrovol;
      QLabel *lbl_avg_banner;
      QLabel *lbl_avg_radius;
      QLabel *lbl_avg_mass;
      QLabel *lbl_avg_hydration;
      QLabel *lbl_avg_volume;
      QLabel *lbl_avg_vbar;

      QPushButton *pb_help;
      QPushButton *pb_cancel;
      QPushButton *pb_vbar;

      QLineEdit *le_vbar;
      QLineEdit *le_vbar_temperature;
      QCheckBox *cb_vbar;
      QCheckBox *cb_pb_rule_on;
      QwtCounter *cnt_hydrovol;
      QwtCounter *cnt_avg_radius;
      QwtCounter *cnt_avg_mass;
      QwtCounter *cnt_avg_hydration;
      QwtCounter *cnt_avg_volume;
      QwtCounter *cnt_avg_vbar;

      QLabel    *lbl_bead_model_controls;
      QLabel    *lbl_target_e_density;
      QLineEdit *le_target_e_density;
      QLabel    *lbl_target_volume;
      QLineEdit *le_target_volume;
      QCheckBox *cb_set_target_on_load_pdb;
      QCheckBox *cb_equalize_radii;
      QLabel    *lbl_hydro_method;
      QCheckBox *cb_hydro_supc;
      QCheckBox *cb_hydro_zeno;
      QLabel    *lbl_other;
      QCheckBox *cb_export_msroll;

   private slots:

      void setupGUI();
      void set_vbar();
      void set_pb_rule_on();
      void select_vbar();
      void update_vbar_signal(float, float);
      void update_vbar(const QString &);
      void update_vbar_temperature(const QString &);
      void update_hydrovol(double);
      void update_avg_radius(double);
      void update_avg_mass(double);
      void update_avg_hydration(double);
      void update_avg_volume(double);
      void update_avg_vbar(double);
      
      void update_target_e_density(const QString &);
      void update_target_volume(const QString &);
      void set_set_target_on_load_pdb();
      void set_equalize_radii();

      void set_hydro_supc();
      void set_hydro_zeno();

      void set_export_msroll();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);

   signals:

      void vbar_changed();
};



#endif

