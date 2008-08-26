#ifndef US_HYDRODYN_BEAD_OUTPUT_H
#define US_HYDRODYN_BEAD_OUTPUT_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qframe.h>
#include <qcheckbox.h>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

struct bead_output_options
{
	int output; 		// 0 = SOMO
							// 1 = BEAMS
							// 2 = HYDRO
	int sequence;		// 0 = as in original pdb file
							// 1 = exposed sidechain -> exposed main chain -> buried
							// 2 = include bead-original residue correspondence
};

class US_EXTERN US_Hydrodyn_Bead_Output : public QFrame
{
	Q_OBJECT

	public:
		US_Hydrodyn_Bead_Output(struct bead_output_options *, bool *, QWidget *p = 0, const char *name = 0);
		~US_Hydrodyn_Bead_Output();

	public:
		
		struct bead_output_options *bead_output;
		bool *bead_output_widget;
		
		US_Config *USglobal;

		QLabel *lbl_info;

		QPushButton *pb_help;
		QPushButton *pb_cancel;
		
		QButtonGroup *bg_output;
		QButtonGroup *bg_sequence;

		QCheckBox *cb_somo_output;
		QCheckBox *cb_beams_output;
		QCheckBox *cb_hydro_output;
		QCheckBox *cb_pdb_sequence;
		QCheckBox *cb_chain_sequence;
		QCheckBox *cb_correspondence_sequence;
		
		
	private slots:
		
		void setupGUI();
		void select_output(int);
		void select_sequence(int);

		void cancel();
		void help();
	
	protected slots:

		void closeEvent(QCloseEvent *);
};



#endif

