#ifndef US_SMSIM_H
#define US_SMSIM_H

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
#include "us_math.h"

class US_EXTERN SecondMoment_Legend_F : public QFrame
{
	Q_OBJECT

	public:
		SecondMoment_Legend_F(QWidget *p = 0, const char *name = 0);
		~SecondMoment_Legend_F();
		QPushButton *save;
		QPushButton *close;
		QProgressBar *progress;
		US_Config *USglobal;
	private:

		QLabel *progress_lbl;

	signals:
		void clicked();

};

class US_EXTERN SecondMoment : public QWidget
{
	Q_OBJECT

	public:
		SecondMoment(int mod, unsigned int *comps, unsigned int *pts, double ***conc,
						vector <struct component> *components,
						struct simulation_parameters *simparams, float **scan_t,
						float temp_conc, float temp_k, QWidget *p = 0, const char *name = 0);
		~SecondMoment();
		US_Config *USglobal;
		
	private:
		QwtPlot *secm_plot;
		SecondMoment_Legend_F *secm_legend;

		vector <struct component> *temp_components;
		struct simulation_parameters *temp_sp;
		double ***c, omega, *plateau, *smp, *sms, avg;
		double *plot1_x, *plot1_y, *plot2_x, *plot2_y, *plot3_x, *plot3_y;
		uint plot1, plot2, plot3;
		float **scantimes, equil_k, total_conc;
		unsigned int points, i, j, comp, exclude, divisions, count, model;

	protected slots:
		void resizeEvent(QResizeEvent *e);
		void closeEvent(QCloseEvent *e);
		
	private slots:
		void setups();
		void recalc();
		void save_secm();
		void save_secm(const QString &);
		void close_secm();
};

#endif

