#ifndef US_GA_MODEL_EDITOR_H
#define US_GA_MODEL_EDITOR_H

#include "us_model_editor.h"
#include "us_constraint_control.h"

using namespace std;

class US_GAModelEditor : public US_ModelEditor
{
	Q_OBJECT
	
	public:
		US_GAModelEditor(struct ModelSystem*, struct ModelSystemConstraints*, QWidget* parent=0, const char* name=0);
		US_GAModelEditor(QWidget *parent=0, const char *name=0);
		~US_GAModelEditor();
		struct ModelSystem *ms;
		struct ModelSystemConstraints *msc;

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
		QwtCounter *cnt_lamella;
		
		QComboBox *cmb_radialGrid;
		QComboBox *cmb_timeGrid;

		QPushButton *pb_selectModel;
		QPushButton *pb_loadInit;
		QPushButton *pb_saveInit;
		QPushButton *pb_close;

		US_ConstraintControl *us_cc_mw;
		US_ConstraintControl *us_cc_f_f0;
		US_ConstraintControl *us_cc_conc;
		US_ConstraintControl *us_cc_keq;
		US_ConstraintControl *us_cc_koff;
		
	private slots:

		void load_constraints();
		void save_constraints();
		bool verify_constraints();
		void update_radialGrid(int);
		void update_timeGrid(int);
		void update_simpoints(double val);
		void update_lamella(double val);

// re-implemented functions:
		void setup_GUI();
		void help();
};

#endif
