#ifndef US_GA_MODEL_EDITOR_H
#define US_GA_MODEL_EDITOR_H

#include "us_model_editor.h"
#include "us_constraint_control.h"
#include "us_extern.h"
#include "us_modelselection.h"

using namespace std;

class US_EXTERN US_GAModelEditor : public US_ModelEditor
{
   Q_OBJECT
   
   public:
      US_GAModelEditor(struct ModelSystem *, QWidget *parent=0, const char *name=0);
      ~US_GAModelEditor();

      unsigned int current_assoc;

      struct ModelSystem *ms;
      struct ModelSystemConstraints msc;
      

      struct constraint c_mw;
      struct constraint c_conc;
      struct constraint c_f_f0;
      struct constraint c_keq;
      struct constraint c_koff;

      QLabel *lbl_constraints;
      QLabel *lbl_high;
      QLabel *lbl_low;
      QLabel *lbl_fit;
      QLabel *lbl_bandVolume;
      QLabel *lbl_simpoints;

      QwtCounter *cnt_simpoints;
      QwtCounter *cnt_band_volume;
      
      QComboBox *cmb_radialGrid;
      QComboBox *cmb_timeGrid;

      QPushButton *pb_selectModel;
      QPushButton *pb_loadInit;
      QPushButton *pb_saveInit;
      QPushButton *pb_close;

      US_ConstraintControl *cc_mw;
      US_ConstraintControl *cc_f_f0;
      US_ConstraintControl *cc_conc;
      US_ConstraintControl *cc_keq;
      US_ConstraintControl *cc_koff;
      
   private slots:

      void load_constraints();
      void save_constraints();
      void update_constraints(unsigned int);
      bool verify_constraints();
      void update_radialGrid(int);
      void update_timeGrid(int);
      void update_simpoints(double val);
      void update_band_volume(double val);
      void select_model();

      void mw_constraintChanged(struct constraint);
      void f_f0_constraintChanged(struct constraint);
      void conc_constraintChanged(struct constraint);
      void keq_constraintChanged(struct constraint);
      void koff_constraintChanged(struct constraint);
      void initialize_msc();

// re-implemented functions:
      void setup_GUI();
      void help();
};

#endif
