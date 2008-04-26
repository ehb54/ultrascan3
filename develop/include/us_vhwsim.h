#ifndef US_VHWSIM_H
#define US_VHWSIM_H

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

#include "us_util.h"
#include "us_pixmap.h"
#include "us_math.h"
#include "us_selectplot.h"
#include "us_printfilter.h"

struct simulation_parameters
{
	unsigned long time;
	unsigned long delay;
	unsigned long speed;
	float delta_t;
	unsigned int delta_r;
	float resolution;
	float meniscus;
	float bottom;
	unsigned int scans;
	float rnoise;
	float inoise;
	float rinoise;
};

struct component
{
	float conc;		// partial concentration
	float sed;		// sedimentation coefficient (s)
	float diff;		// diffusion coefficient (D)
	float sigma;	// concentration dependency factor for (s)
	float delta;	// concentration dependency factor for (D)
};

class Extrapolation_Legend_F : public QFrame
{
	Q_OBJECT

	public:
		Extrapolation_Legend_F(int divisions, QWidget *p = 0, const char *name = 0);
		~Extrapolation_Legend_F();
		QPushButton *dist;
		QPushButton *save;
		QPushButton *close;
		QProgressBar *progress;
		QwtCounter *division_counter;
		US_Config *USglobal;

	private:
		QLabel *division_lbl;
		QLabel *progress_lbl;

	signals:
		void clicked();
		void valueChanged(double);

};

class Distribution_Legend_F : public QFrame
{
	Q_OBJECT
	public:
		Distribution_Legend_F(QWidget *p = 0, const char *name = 0);
		~Distribution_Legend_F();

	public:
		QPushButton *expand;
		QPushButton *compress;
		QPushButton *pb_histogram;
		QPushButton *pb_print;
		QPushButton *close;
		QPushButton *help;
		QPushButton *pb_histogram_plot;
		QPushButton *pb_envelope_plot;
		QLabel *lbl_sensitivity_envelope;
		QLabel *lbl_sensitivity_histogram;
		US_Config *USglobal;
		QwtCounter *ct_envelope;
		QwtCounter *ct_histogram;

	signals:
		void clicked();

};

class Distribution : public QWidget
{
	Q_OBJECT

	public:
		Distribution(QString temp_run_name, unsigned int *pts, double **boundary_fractions,
		double **intercepts, const QString temp_htmlDir, const QString temp_title, const int temp_cell,
		const int temp_lambda, QWidget *p = 0, const char *name = 0);
		~Distribution();

	private:
		bool histogram, histogram_is_hidden, envelope_is_hidden, close_flag;
		unsigned int divisions;
		float sensitivity, smoothval;
		uint dis_curve;
		QwtPlot *dis_plot;
		Distribution_Legend_F *dis_legend;
		int cell, lambda;
		QString htmlDir, title;
		double **sed;
		double **fractions, m_min, m_max, diff;
		US_Config *USglobal;
		US_Pixmap *pm;
		QString run_name;

	protected slots:
		void resizeEvent(QResizeEvent *e);

	public slots:
		void expand();
		void compress();
		void close_dis();
		void help();
		void show_histogram();
		void show_histogram_plot();
		void update_pixmap();
		void print_plot();
		void show_distribution();
		void closeEvent(QCloseEvent *e);
		void hide_histogram();
		void hide_envelope();
		void update_histogram(double);
		void update_envelope(double);
};

class vHW_Sim : public QWidget
{
	Q_OBJECT

	public:
                vHW_Sim(int mod, unsigned int *component, unsigned int *pts, int *run_flag, vector <struct component> *components,
						double ***conc, struct simulation_parameters *simparams, float **scan_t,
						float temp_conc, float temp_k, QWidget *p = 0, const char *name = 0);
		~vHW_Sim();

	private:
		int flag, model;
		float total_conc, equil_k;
		bool setup_already;
		QwtPlot *ext_plot;
		Distribution *dis;
		Extrapolation_Legend_F *ext_legend;
		vector <struct component> *temp_components;
		struct simulation_parameters *temp_sp;
		double ***c, **sed_app, omega, *plateau, interval, testy, *tempx, *tempy;
		double *slope, *intercept, correl, sigma, *lines_x, *lines_y, *boundary_fractions;
		float **scantimes, temp_r;
		unsigned int points, i, j, comp, exclude, divisions, count;
		void setups();
		uint *sa_points, *sa_lines;
		US_Config *USglobal;

	protected slots:
		void resizeEvent(QResizeEvent *e);
		void closeEvent(QCloseEvent *e);

	private slots:
		void recalc();
		void distrib();
		void update_div(double);
		void save_vhw();
		void save_vhw(const char *);
		void close_vhw();
		void cleanup();

};

#endif

