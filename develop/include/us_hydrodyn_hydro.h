#ifndef US_HYDRODYN_HYDRO_H
#define US_HYDRODYN_HYDRO_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qlineedit.h>

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
   double volume;               // volume correction
   bool mass_correction;      // false: Automatic, true: manual (provide value)
   double mass;               // mass correction
   bool bead_inclusion;         // false: exclude hidden beads; true: use all beads
   bool rotational;            // false: include beads in volume correction for rotational diffusion, true: exclude
   bool viscosity;            // false: include beads in volume correction for intrinsic viscosity, true: exclude
   bool overlap_cutoff;         // false: same as in model building, true: enter manually
   double overlap;            // overlap cut off value if entered manually
};

class US_EXTERN US_Hydrodyn_Hydro : public QFrame
{
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
      
      QButtonGroup *bg_solvent_conditions;
      QButtonGroup *bg_reference_system;
      QButtonGroup *bg_boundary_cond;
      QButtonGroup *bg_volume_correction;
      QButtonGroup *bg_mass_correction;
      QButtonGroup *bg_overlap;
      QButtonGroup *bg_bead_inclusion;
      
      QGroupBox *gb_buried;

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
      
      QwtCounter *cnt_unit;
      
      QLineEdit *le_solvent_name;
      QLineEdit *le_solvent_acronym;
      QLineEdit *le_temperature;
      QLineEdit *le_solvent_viscosity;
      QLineEdit *le_solvent_density;

      QLineEdit *le_volume;
      QLineEdit *le_mass;
      QLineEdit *le_overlap;
      
      QPushButton *pb_help;
      QPushButton *pb_cancel;

   private slots:

      void setupGUI();

      void update_solvent_name(const QString &str);
      void update_solvent_acronym(const QString &str);
      void update_temperature(const QString &str);
      void update_solvent_viscosity(const QString &str);
      void update_solvent_density(const QString &str);

      void update_unit(double);
      void update_volume(const QString &str);
      void update_mass(const QString &str);
      void update_overlap(const QString &str);
      void select_reference_system(int);
      void select_boundary_cond(int);
      void select_volume_correction(int);
      void select_mass_correction(int);
      void select_overlap(int);
      void select_bead_inclusion(int);
      void set_solvent_defaults();
      void set_rotational();
      void set_viscosity();
      void check_solvent_defaults();
      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};



#endif

