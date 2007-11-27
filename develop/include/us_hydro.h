#ifndef US_HYDRO_H
#define US_HYDRO_H

#include <qwidget.h>
#include <qframe.h>
#include <qdialog.h>
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
#include <qlayout.h>
#include "us.h"

#include "us_util.h"
#include "us_dtactrl.h"

struct hydrocomp
{
	float sedcoeff;
	float diffcoeff;
	float f;
	float f_f0;
	float a;
	float b;
	float volume;
};

struct hydrosim
{
	float mw;
	float density;
	float viscosity;
	float vbar;
	float temperature;
	float axial_ratio;
	struct hydrocomp sphere;
	struct hydrocomp prolate;
	struct hydrocomp oblate;
	struct hydrocomp rod;
};

class US_EXTERN US_Hydro1 : public QDialog
{
	Q_OBJECT

	public:
		US_Hydro1(QWidget *p=0, const char *name = 0);
		US_Hydro1(struct hydrosim *, QWidget *p=0, const char *name = 0);
		~US_Hydro1();

		
	private:
		
		int xpos, ypos, width, height;
		float mw, ratio;
		double *prolate;
		double *oblate;
		double *rod;
		double *ratio_x;
		double vline_x[2];
		double vline_y[2];
		QPushButton *pb_vbar;
		QPushButton *pb_help;
		QPushButton *pb_close;
		QPushButton *pb_viscosity;
		QPushButton *pb_density;
		QLabel *lbl_mw;
		QLabel *lbl_temperature;
		QLabel *lbl_ratio;
		QLabel *lbl_text;
		QLabel *lbl_header[8];
		QLabel *lbl_sphere[8];
		QLabel *lbl_prolate[8];
		QLabel *lbl_oblate[8];
		QLabel *lbl_rod[8];
		QLineEdit *le_mw;
		QLineEdit *le_temperature;
		QLineEdit *le_density;
		QLineEdit *le_viscosity;
		QLineEdit *le_vbar;
		QLineEdit *le_ratio;
		QwtCounter *cnt_ratio;
		QwtPlot *plt;
		long cv_prolate, cv_oblate, cv_rod, cv_vline;
		Data_Control_W *data_control;
		US_Config *USglobal;

	public:
		struct hydrosim *allparams;

	signals:
		void clicked();
		void updated();

	protected:
//		void resizeEvent(QResizeEvent *e);
		void closeEvent(QCloseEvent *e);
		void GUI();
		void setup_GUI();
	
		
	public slots:
		void update_buffer_lbl(float, float);
		void update_density_lbl(const QString &);
		void update_viscosity_lbl(const QString &);
		void update_vbar_lbl(float, float);
		void update_vbar_lbl(const QString &);
		void update_temperature_lbl(const QString &);
		void update_mw_lbl(const QString &);
		void update_mw_lbl(float);
		void update_ratio_lbl(const QString &);
		void update_ratio_lbl(double);
		void update();
		void update(struct hydrosim *);
		void help();
		void quit();
		double calc_invtangent(double);
		void calc_models();
};

class US_EXTERN US_Hydro2 : public QFrame
{
	Q_OBJECT

	public:
		US_Hydro2(QWidget *p=0, const char *name = 0);
		~US_Hydro2();

		
	private:
		
		int xpos, ypos, width, height, flag;
		float mw, sed_coeff, diff_coeff, frict_coeff, ratio;
		QPushButton *pb_vbar;
		QPushButton *pb_update;
		QPushButton *pb_help;
		QPushButton *pb_close;
		QPushButton *pb_viscosity;
		QPushButton *pb_density;
		QPushButton *pb_mw_s;
		QPushButton *pb_mw_d;
		QPushButton *pb_s_d;
		QLabel *lbl_mw;
		QLabel *lbl_param1;
		QLabel *lbl_param2;
		QLabel *lbl_temperature;
		QLabel *lbl_text;
		QLabel *lbl_header[4];
		QLabel *lbl_sphere[4];
		QLabel *lbl_prolate[4];
		QLabel *lbl_oblate[4];
		QLabel *lbl_rod[4];
		QLabel *lbl_line1[4];
		QLabel *lbl_line2[4];
		QLabel *lbl_line3[4];
		QLineEdit *le_temperature;
		QLineEdit *le_density;
		QLineEdit *le_viscosity;
		QLineEdit *le_vbar;
		QLineEdit *le_param1;
		QLineEdit *le_param2;
		Data_Control_W *data_control;
		US_Config *USglobal;

	signals:

		void clicked();

		
	public slots:

		void update_buffer_lbl(float, float);
		void update_density_lbl(const QString &);
		void update_viscosity_lbl(const QString &);
		void update_vbar_lbl(float, float);
		void update_vbar_lbl(const QString &);
		void update_temperature_lbl(const QString &);
		void update_param1_lbl(const QString &);
		void update_param2_lbl(const QString &);
		void update();
		void do_mw_s();
		void do_mw_d();
		void do_s_d();
		void help();
		void quit();
		double calc_invtangent(double);
		bool check_valid(float, float);
		double root(int, float);

	protected slots:
		void setup_GUI();
		void closeEvent(QCloseEvent *e);
};

class US_Hydro3 : public QFrame
{
	Q_OBJECT

	public:
		US_Hydro3(int run_type, float *temp_s20w, float *temp_D20w, float *temp_mw, float *temp_vbar20,
		const QString header_str, const int temp_component, QWidget *p = 0, const char *name = 0);
		~US_Hydro3();
		float vol_per_molecule, rad_sphere, f0, frict_coeff, f_ratio;
		float ap, bp, ratio_p;
		float ao, bo, ratio_o;
		float ar, br, ratio_r;
		int component;
		
	private:
		
		float *mw, *s20w, *D20w, *vbar;
		QPushButton *pb_update;
		QPushButton *pb_help;
		QPushButton *pb_close;
		QLabel *lbl_header[4];
		QLabel *lbl_sphere[4];
		QLabel *lbl_prolate[4];
		QLabel *lbl_oblate[4];
		QLabel *lbl_rod[4];
		QLabel *lbl_line1[4];
		QLabel *lbl_line2[4];
		QLabel *lbl_line3[4];
		US_Config *USglobal;

		
	public slots:

		void calc();
		void update();
		void help();
		void quit();
		
	protected slots:
		void closeEvent(QCloseEvent *e);
};
#endif

