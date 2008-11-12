#ifndef US_HYDRODYN_RESULTS_H
#define US_HYDRODYN_RESULTS_H

// QT defs:

#include <qlabel.h>
#include <qlineedit.h>
#include <qstring.h>
#include <qlayout.h>
#include <qframe.h>

#include "us_util.h"
#include "us_editor.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

using namespace std;

struct hydro_results
{
	float total_beads;
	float used_beads;
	double mass;
	double s20w;
	double s20w_sd;
	double D20w;
	double D20w_sd;
	double viscosity;
	double viscosity_sd;
	double rs;
	double rs_sd;
	double rg;
	double rg_sd;
	double tau;
	double tau_sd;
	double vbar;
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
		TextEdit *e;

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
		QLabel *lbl_tau;
		QLineEdit *le_tau;
		QLabel *lbl_vbar;
		QLineEdit *le_vbar;

		QPushButton *pb_help;
		QPushButton *pb_cancel;
		QPushButton *pb_load_results;
		QPushButton *pb_load_asa;
		QPushButton *pb_load_beadmodel;

		QString somo_dir;

	public slots:

		void cancel();

	private slots:

		void setupGUI();
		void load_results();
		void load_asa();
		void load_beadmodel();
		void help();
		void view_file(const QString &);

	protected slots:

		void closeEvent(QCloseEvent *);
};



#endif

