#ifndef US_HYDRODYN_HYDRO_H
#define US_HYDRODYN_HYDRO_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
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

struct hydro_options
{
   int unit;                   // exponent from units in meter (example: -10 = Angstrom, -9 = nanometers)
   QString solvent_name;
   QString solvent_acronym;
   double temperature;         // in C
   double solvent_viscosity;
   double solvent_density;
   bool reference_system;      // false: diffusion center, true: cartesian origin (default false)
   bool boundary_cond;         // false: stick, true: slip (default false)
   bool volume_correction;      // false: Automatic, true: manual (provide value)
   bool use_avg_for_volume;     // true: if volume_correction true, will replace volumes with averages
   double volume;               // volume correction
   bool mass_correction;      // false: Automatic, true: manual (provide value)
   double mass;               // mass correction
   bool bead_inclusion;         // false: exclude hidden beads; true: use all beads
   bool grpy_bead_inclusion;    // false: exclude hidden beads; true: use all beads
   bool rotational;            // false: include beads in volume correction for rotational diffusion, true: exclude
   bool viscosity;            // false: include beads in volume correction for intrinsic viscosity, true: exclude
   bool overlap_cutoff;         // false: same as in model building, true: enter manually
   double overlap;            // overlap cut off value if entered manually
   double pH;        

   bool manual_solvent_conditions;     // manual viscosity & density

   // zeno options

   bool zeno_zeno;
   bool zeno_interior;
   bool zeno_surface;

   // these are in thousands:
   unsigned int zeno_zeno_steps; 
   unsigned int zeno_interior_steps;
   unsigned int zeno_surface_steps;

   float zeno_surface_thickness;

   bool zeno_surface_thickness_from_rg;
   double zeno_surface_thickness_from_rg_a;
   double zeno_surface_thickness_from_rg_b;
   double zeno_surface_thickness_from_rg_c;
};

class US_EXTERN US_Hydrodyn_Hydro : public QFrame
{
   friend class US_Hydrodyn;

   Q_OBJECT

   public:
      US_Hydrodyn_Hydro(struct hydro_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Hydro();

   public:

      struct hydro_options *hydro;
      bool *hydro_widget;
                void *us_hydrodyn;

      US_Config *USglobal;

      QLabel *lbl_info;
      QLabel *lbl_unit;
      QLabel *lbl_reference_system;
      QLabel *lbl_volume;
      QLabel *lbl_mass;
      QLabel *lbl_overlap;
      QLabel *lbl_solvent_name;
      QLabel *lbl_solvent_defaults;
      QLabel *lbl_solvent_acronym;
      QLabel *lbl_temperature;
      QLabel *lbl_solvent_viscosity;
      QLabel *lbl_solvent_density;
      QLabel *lbl_tc_solvent_viscosity;
      QLabel *lbl_tc_solvent_density;
      QLabel *lbl_smi;
      QLabel *lbl_grpy;
      
      QGroupBox *bg_solvent_conditions;
      QGroupBox *bg_reference_system;
      QGroupBox *bg_boundary_cond;
      QGroupBox *bg_volume_correction;
      QGroupBox *bg_mass_correction;
      QGroupBox *bg_overlap;
      QGroupBox *bg_bead_inclusion;
      QGroupBox *bg_grpy_bead_inclusion;
      
      QGroupBox *bg_buried;

      QCheckBox *cb_solvent_defaults;
      QCheckBox *cb_diffusion_center;
      QCheckBox *cb_cartesian_origin;
      QCheckBox *cb_stick;
      QCheckBox *cb_slip;
      QCheckBox *cb_auto_volume;
      QCheckBox *cb_manual_volume;
      QCheckBox *cb_auto_mass;
      QCheckBox *cb_manual_mass;
      QCheckBox *cb_auto_overlap;
      QCheckBox *cb_manual_overlap;
      QCheckBox *cb_exclusion;
      QCheckBox *cb_inclusion;
      QCheckBox *cb_rotational;
      QCheckBox *cb_viscosity;
      QCheckBox *cb_manual_solvent_conditions;
      
      QRadioButton *rb_diffusion_center;
      QRadioButton *rb_cartesian_origin;
      QRadioButton *rb_stick;
      QRadioButton *rb_slip;
      QRadioButton *rb_auto_mass;
      QRadioButton *rb_manual_mass;
      QRadioButton *rb_auto_volume;
      QRadioButton *rb_auto_volume_avg;
      QRadioButton *rb_manual_volume;
      QRadioButton *rb_exclusion;
      QRadioButton *rb_inclusion;
      QRadioButton *rb_grpy_exclusion;
      QRadioButton *rb_grpy_inclusion;
      QRadioButton *rb_auto_overlap;
      QRadioButton *rb_manual_overlap;

      QwtCounter *cnt_unit;
      
      QLineEdit *le_solvent_name;
      QLineEdit *le_solvent_acronym;
      QLineEdit *le_temperature;
      QLineEdit *le_solvent_viscosity;
      QLineEdit *le_solvent_density;
      QLineEdit *le_tc_solvent_viscosity;
      QLineEdit *le_tc_solvent_density;

      QLineEdit *le_volume;
      QLineEdit *le_mass;
      QLineEdit *le_overlap;
      
      QPushButton *pb_help;
      QPushButton *pb_cancel;

   private slots:

      void setupGUI();

      void update_solvent_name(const QString &str);
      void update_solvent_acronym(const QString &str);
      void update_temperature(const QString &str, bool update_main = true );
      void update_solvent_viscosity(const QString &str);
      void update_solvent_density(const QString &str);

      void update_unit(double);
      void update_volume(const QString &str);
      void update_mass(const QString &str);
      void update_overlap(const QString &str);

      void select_reference_system();
      void select_reference_system(int);
      void select_boundary_cond();
      void select_boundary_cond(int);
      void select_volume_correction();
      void select_volume_correction(int);
      void select_mass_correction();
      void select_mass_correction(int);
      void select_overlap();
      void select_overlap(int);
      void select_bead_inclusion();
      void select_bead_inclusion(int);
      void select_grpy_bead_inclusion();
      void select_grpy_bead_inclusion(int);
      void set_solvent_defaults();
      void set_rotational();
      void set_viscosity();
      void set_manual_solvent_conditions();
      void check_solvent_defaults();
      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};



#endif

