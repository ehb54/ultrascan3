#ifndef US_DCDTSIM_H
#define US_DCDTSIM_H

#include <qwidget.h>
#include <qframe.h>
#include <qpushbt.h>
#include <iostream>
#include <qmsgbox.h>
#include <fstream>
#include <stdlib.h>
#include <locale.h>
#include <qtstream.h>
#include <string.h>
#include <qwt_plot.h>
#include <qwt_counter.h>
#include <qwt_math.h>
#include <qfile.h>
#include <qfiledlg.h>
#include <qprogbar.h>

#include "us_simdlg.h"
#include "us_util.h"
#include "us_average.h"
#include "us_math.h"

class dCdT_Legend_F : public QFrame
{
	Q_OBJECT

	public:
		dCdT_Legend_F(double radius, QWidget *p = 0, const char *name = 0);
		~dCdT_Legend_F();
		QPushButton *calc;
		QPushButton *average;
		QPushButton *close;
		QProgressBar *progress;
		QwtCounter *radius_counter;
		US_Config *USglobal;

	private:

		QLabel *progress_lbl;
		QLabel *radius_lbl;

	signals:
		void clicked();
		void valueChanged(double);
};

class dCdT : public QWidget
{
	Q_OBJECT

	public:
		dCdT(int temp_model, unsigned int *comps, vector <struct component> *components, double ***conc,
				struct simulation_parameters *simparams, float **scan_t, 
				float temp_conc, float temp_k, QWidget *p = 0, const char *name = 0);
		~dCdT();
		double radius;

	private:
		QwtPlot *dcdt_plot;
		dCdT_Legend_F *dcdt_legend;
		US_Average *avg;
		vector <struct component> *temp_components;
		struct simulation_parameters *temp_sp;
		double ***c, omega, **dcdt_points, **sval, *plateau;
		int model;
		unsigned int comp;
		unsigned int *plots;
		float **scantimes, total_conc, equil_k;
		unsigned int points, i, j, count;
		bool test;
		bool setup_already;
		US_Config *USglobal;

	protected slots:
		void resizeEvent(QResizeEvent *e);
		void closeEvent(QCloseEvent *e);

	private slots:
		void recalc();
		void close_dcdt();
		void setups();
		void change_radius(double);
		void average();
		void cleanup();

};

#endif

