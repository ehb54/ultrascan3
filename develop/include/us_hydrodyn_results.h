#ifndef US_HYDRODYN_RESULTS_H
#define US_HYDRODYN_RESULTS_H

// QT defs:

#include <qlabel.h>
#include <qlineedit.h>
#include <qstring.h>
#include <qlayout.h>
#include <qframe.h>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

struct hydro_results
{
	unsigned int total_beads;
	unsigned int used_beads;
	double mass;
	double s20w;
	double D20w;
	double viscosity;
	double rs;
	double rg;
	double theta;
};

class US_EXTERN US_Hydrodyn_Results : public QFrame
{
	Q_OBJECT

	public:
		US_Hydrodyn_Results(struct hydro_results *, bool *, QWidget *p = 0, const char *name = 0);
		~US_Hydrodyn_Results();

	public:
		
		struct hydro_results *results;
		bool *result_widget;
		
		US_Config *USglobal;
		
		QLabel *lbl_info;
		QLabel *lbl_total_beads;
		QLineEdit *le_total_beads;
		QLabel *lbl_used_beads;
		QLineEdit *le_used_beads;
		QLabel *lbl_mass;
		QLineEdit *le_mass;
		QLabel *lbl_D20w;
		QLineEdit *le_D20w;
		QLabel *lbl_s20w;
		QLineEdit *le_s20w;
		QLabel *lbl_viscosity;
		QLineEdit *le_viscosity;
		QLabel *lbl_rs;
		QLineEdit *le_rs;
		QLabel *lbl_rg;
		QLineEdit *le_rg;
		QLabel *lbl_theta;
		QLineEdit *le_theta;

		QPushButton *pb_help;
		QPushButton *pb_cancel;

	private slots:
		
		void setupGUI();
		void cancel();
		void help();
	
	protected slots:

		void closeEvent(QCloseEvent *);
};



#endif

