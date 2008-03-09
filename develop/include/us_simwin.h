#ifndef US_SIMWIN_H
#define US_SIMWIN_H

#include <qapp.h>
#include <math.h>
#include <qlabel.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <qfont.h>
#include <qlcdnum.h>
#include <qprogbar.h>
#include <qtimer.h>
#include <qwt_counter.h>
#include <qdatetime.h>

#include "us_matrix.h"
#include "us_vhwsim.h"
#include "us_smsim.h"
#include "us_dcdrsim.h"
#include "us_dcdtsim.h"
#include "us_displp.h"
#include "us_util.h"
#include "us_hydro.h"
#include "us_editor.h"
#include "us_mfem.h"
#include "us_clipdata.h"

extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN SpeedLegend_F : public QFrame
{
	Q_OBJECT
	public:
	   unsigned long int *speed;
		SpeedLegend_F(unsigned long int *, QWidget *p = 0, const char *name = 0);
		~SpeedLegend_F();
		QLabel *speed_lbl;
		QPushButton *pb_continue;
		QwtCounter *speed_counter;
		US_Config *USglobal;

	public slots:
		void update_speed(double);
};

class US_EXTERN dcdt_Window : public QWidget
{
	Q_OBJECT
	public:
		dcdt_Window(bool *, QWidget *p = 0, const char *name = 0);
		~dcdt_Window();
		bool *widget;
		QwtPlot *plt_dcdt;
		QPushButton *pb_close;
		US_Config *USglobal;

	public slots:
		void dcdtPlot(double **, double **, unsigned int);
		void quit();

	protected slots:
		void resizeEvent(QResizeEvent *e);
		void closeEvent(QCloseEvent *e);
};

class US_EXTERN MovieLegend_F : public QFrame
{
	public:
		MovieLegend_F(QWidget *p = 0, const char *name = 0);
		~MovieLegend_F();
		US_Config *USglobal;
		QLCDNumber  *time_lcd;
		QLCDNumber  *comp_lcd;
		QLabel *time_lbl;
		QLabel *comp_lbl;
		QPushButton *pb_speed;
};

class US_EXTERN ScanLegend_F : public QFrame
{
	public:
		ScanLegend_F(QWidget *p = 0, const char *name = 0);
		~ScanLegend_F();
		US_Config *USglobal;
		QLabel *scan_lbl;
		QLabel *progress_lbl;
		QLCDNumber  *scan_lcd;
		QProgressBar *progress;
		QPushButton *pb_dcdt;
};

class US_EXTERN SimWin : public QWidget
{
	Q_OBJECT

	private:

		QwtPlot *movie_plot;
		QwtPlot *scan_plot;

		US_Config *USglobal;
		SimControl_F *frmInp;
		MovieLegend_F *m_legend;
		ScanLegend_F *s_legend;
		vHW_Sim *vhw;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif    

		vector <float> speedTimeV;
		vector <unsigned int> speedV;

#ifdef WIN32
#pragma warning ( default: 4251 )
#endif

		SecondMoment *secm;
		dCdT *dcdt;
		dCdr *dcdr;
		bool init_simulation, init_inoise;
		unsigned int current_component;
		double current_time;
		float resolution;
		dcdt_Window *dcdt_W;
		DisplayPanel *display_p;
		SpeedLegend_F *sp_f;
		US_MovingFEM *mfem;

		unsigned int points;
		unsigned int lastscans;
		unsigned int *curve;
		unsigned int movie_scan;
		unsigned int point_index; // saves the max dimension of the current radius array 
		unsigned int scan_index;  // saves the max dimension of the current scan time array 
		bool stop_calculation, speedChanged, dcdtWindow, saveTimes;
		float *inoise, *rinoise;
		double *radius;
		double *dcdtData;
		double *oldScan;
		bool dcdt_flag;
		double **c;
		float omega_sqr;
		float temp1;
		float temp2;
		float maxconc;
		float scan_timer;
		float *scantimes;
		float temp_conc;
		float temp_k;
		float initial_concentration;

	public:	
		SimWin(QWidget *p = 0, const char *name = 0);
		~SimWin();
		unsigned int comp;
		int model;	// 1 = non-interacting, 2 = monomer-dimer, 3 = isomerization

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif    

		vector <struct component> components;
		vector <struct component> copy_components;

#ifdef WIN32
#pragma warning ( default: 4251 )
#endif

		struct simulation_parameters simparams;
		bool simflag;
		bool calc_flag;
		bool plateau_flag;
		struct mfem_initial initCvector;
		struct mfem_data fem_data;

	public slots:
		void simulate();
		void display_params();
		void quitthis();
		void stop_now();
		void component_reset();
		void component_init();
		void sim_help();
		void non_interacting();
		void monomer_dimer();
		void isomerization();
		void vhw_Analysis();
		void dcdt_Analysis();
		void dcdr_Analysis();
		void SecondMoment_Analysis();
		void change_speed();
		void dcdtOpen();
		void savescans();
		void save_ultrascan(const QString &);
		void save_xla(const QString &);
		
	protected slots:
		void init();
//		void resizeEvent(QResizeEvent *e);
		void closeEvent(QCloseEvent *e);
		void update_GUI(struct mfem_scan, std::vector <double>);

	private slots:
		void plot_scans();
		void show_speeds();
		void nextSpeed();
};

#endif

