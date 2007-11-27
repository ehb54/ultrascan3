#ifndef US_AVERAGE_H
#define US_AVERAGE_H

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
#include <qprinter.h>
#include "us_util.h"
#include "us_pixmap.h"

class US_Average_Legend_F : public QFrame
{
	Q_OBJECT
	public:
		US_Average_Legend_F(QWidget *p = 0, const char *name = 0);
		~US_Average_Legend_F();

	public:
		US_Config *USglobal;
		QPushButton *expand;
		QPushButton *compress;
		QPushButton *save;
		QPushButton *pb_print;
		QPushButton *close;

	signals:
		void clicked();
};

class US_Average : public QWidget
{
	Q_OBJECT

	public:
		US_Average(int pts, int sc, double ***xval, double ***yval, const QString temp_extension,
		const QString temp_htmldir, const QString temp_title,
		const int temp_cell, const int temp_lambda,	QWidget *p = 0, const char *name = 0);
		~US_Average();
		
		US_Pixmap *pm;
		US_Config *USglobal;
		int points, total_points, scans;
		uint avg_curve;
		QwtPlot *avg_plot;
		US_Average_Legend_F *avg_legend;
		bool *end_flag;
		double ***x, ***y, *sum, *sval, m_min, m_max, diff;
		QString extension;
		QString htmlDir;
		int cell, lambda;

	public slots:
		void expand();
		void compress();
		void close_dis();
		void save_avg();
		void save_avg(const QString &);
		void calc();
		void update_pixmap();
		void print_plot();

	signals:
		void status(int);

	protected slots:
		void resizeEvent(QResizeEvent *e);
		void closeEvent(QCloseEvent *e);

};

#endif

