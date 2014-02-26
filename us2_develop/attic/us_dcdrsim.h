#ifndef US_DCDRSIM_H
#define US_DCDRSIM_H

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

class dCdr_Legend_F : public QFrame
{
	Q_OBJECT

	public:
		dCdr_Legend_F(double radius, QWidget *p = 0, const char *name = 0);
		~dCdr_Legend_F();
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

class dCdr : public QWidget
{
	Q_OBJECT

	public:
		dCdr(double ***conc, struct simulation_parameters *simparams, 
				float **scan_t, QWidget *p = 0, const char *name = 0);
		~dCdr();
		double radius;

	public slots:
		void update_progress(int);

	private:
		QwtPlot *dcdr_plot;
		US_Config *USglobal;
		dCdr_Legend_F *dcdr_legend;
		US_Average *avg;
		vector <struct component> *temp_components;
		struct simulation_parameters *temp_sp;
		double ***c, omega, **dcdr_points, **sval;
		uint *plots;
		float **scantimes;
		unsigned int  points, i, j, count;
		bool test;
		bool setup_already;
		
	protected slots:
		void resizeEvent(QResizeEvent *e);
		void closeEvent(QCloseEvent *e);

	private slots:
		void recalc();
		void close_dcdr();
		void setups();
		void change_radius(double);
		void average();
		void cleanup();

};

#endif

