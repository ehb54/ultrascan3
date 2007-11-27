#ifndef US_SIMDLG_H
#define US_SIMDLG_H

#include <qwidget.h>
#include <qframe.h>
#include <qpushbt.h>
#include <qapp.h>
#include <iostream>
#include <qmsgbox.h>
#include <fstream>
#include <stdlib.h>
#include <locale.h>
#include <qpopmenu.h>
#include <qcombo.h>
#include <qtstream.h>
#include <qlistbox.h>
#include <qmenubar.h>
#include <string.h>
#include <qpopmenu.h>
#include <qkeycode.h>
#include <qwt_plot.h>
#include <qwt_counter.h>
#include <qwt_math.h>
#include <qdialog.h>
#include <qlined.h>
#include <qfile.h>
#include <qfiledlg.h>
#include "us_util.h"

extern int global_Xpos;
extern int global_Ypos;

struct component
{
	float conc;		// partial concentration
	float sed;		// sedimentation coefficient (s)
	float diff;		// diffusion coefficient (D)
	float sigma;	// concentration dependency factor for (s)
	float delta;	// concentration dependency factor for (D)
};

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

class SimControl_F : public QFrame
{
	Q_OBJECT

	public:
		US_Config *USglobal;

		QPushButton *config;
		QPushButton *load;
		QPushButton *simpar;
		QPushButton *display;
		QPushButton *start_sim;
		QPushButton *write_file;
		QPushButton *quit;
		QPushButton *vhw;
		QPushButton *secm;
		QPushButton *dcdt;
		QPushButton *dcdr;
		QPushButton *help;
		QPushButton *stop_now;
		QLabel *settings_F;
		QLabel *config_banner;
		QLabel *analysis_banner;
		QLabel *current_banner;
		QLabel *model_lb1;
		QLabel *model_lb2;
		QLabel *comp_lb1;
		QLabel *comp_lb2;
		QLabel *speed_lb1;
		QLabel *speed_lb2;
		QLabel *time_lb1;
		QLabel *time_lb2;
		QLabel *scans_lb1;
		QLabel *scans_lb2;
		QLabel *delta_t_lb1;
		QLabel *delta_t_lb2;
		QLabel *delta_r_lb1;
		QLabel *delta_r_lb2;
		bool *temp_simflag;
		SimControl_F(bool *simflag, unsigned int *comp, int *model, vector <struct component> *components, 
						struct simulation_parameters *simparams, QWidget *p = 0, const char *name = 0);
		~SimControl_F();

	public slots:
		void setmodel_T(QString);
		void setcomponents_T(const QString &);
		void setspeed_T(const QString &);
		void setduration_T(const QString &);
		void setscans_T(const QString &);
		void setdelta_t_T(const QString &);
		void setdelta_r_T(const QString &);

	signals:
		void clicked();

	public slots:
		void create_model();
		void load_model();
		void run_parameters();
	
	private slots:
		void setup_GUI();
		
	private:	
		void load_model(const QString &);
		int *temp_model;
		unsigned int *temp_comp;
		vector <struct component> *temp_components;
		struct simulation_parameters *temp_sp;
};

class CompDialog : public QDialog
{
	Q_OBJECT
	public:
		CompDialog(QWidget *parent=0, const char *name=0);
		~CompDialog();
		US_Config *USglobal;


		float conc;
		float sed;
		float diff;
		float sigma;
		float delta;
		QPushButton *next;
		QLabel *component_lbl;
		QLabel *model_lbl;
		QLineEdit *conc_le;
		QLineEdit *sed_le;
		QLineEdit *diff_le;
		QLineEdit *sigma_le;
		QLineEdit *delta_le;

	private:
		int molecule;
		QLabel *msg1_lbl;
		QLabel *sed_lbl;
		QLabel *diff_lbl;
		QLabel *conc_lbl;
		QLabel *sigma_lbl;
		QLabel *delta_lbl;
		QLabel *lbl_blank;
		QLabel *lbl_prolate;
		QLabel *lbl_oblate;
		QLabel *lbl_rod;
		QLabel *lbl_sphere;
		QPushButton *pb_simulateComponent;
		QPushButton *cancel;
		QString str;
		QCheckBox *cb_rod, *cb_oblate, *cb_sphere, *cb_prolate;

	protected slots:
		void closeEvent(QCloseEvent *e);
	
	private slots:
		void simulate_component();
		void update_component();
		void update_conc(const QString &);
		void update_sed(const QString &);
		void update_diff(const QString &);
		void update_sigma(const QString &);
		void update_delta(const QString &);
		void select_rod();
		void select_prolate();
		void select_oblate();
		void select_sphere();
};

class EquilDialog : public QDialog 
{
	Q_OBJECT
	public:
		EquilDialog(QWidget *parent=0, const char *name=0);
		~EquilDialog();
		US_Config *USglobal;

		float conc;
		float equil;
		float sed1;
		float sed2;
		float diff1;
		float diff2;
		float sigma1;
		float sigma2;
		float delta1;
		float delta2;
		QLabel *component_lbl1;
		QLabel *component_lbl2;
		QLabel *model_lbl;
		QLineEdit *conc_le;
		QLineEdit *equil_le;
		QLineEdit *sed1_le;
		QLineEdit *sed2_le;
		QLineEdit *diff1_le;
		QLineEdit *diff2_le;
		QLineEdit *sigma1_le;
		QLineEdit *sigma2_le;
		QLineEdit *delta1_le;
		QLineEdit *delta2_le;

	private:
		int molecule;
		QLabel *conc_lbl;
		QLabel *equil_lbl;
		QLabel *sed1_lbl;
		QLabel *diff1_lbl;
		QLabel *sigma1_lbl;
		QLabel *delta1_lbl;
		QLabel *msg1_lbl;
		QLabel *lbl_blank;
		QLabel *lbl_prolate;
		QLabel *lbl_oblate;
		QLabel *lbl_rod;
		QLabel *lbl_sphere;
		QPushButton *cancel;
		QPushButton *ok;
		QPushButton *pb_simulateComponent;
		QCheckBox *cb_rod, *cb_oblate, *cb_sphere, *cb_prolate;

	protected slots:
		void closeEvent(QCloseEvent *e);

	private slots:
		void update_conc(const QString &);
		void update_equil(const QString &);
		void update_sed1(const QString &);
		void update_sed2(const QString &);
		void update_diff1(const QString &);
		void update_diff2(const QString &);
		void update_sigma1(const QString &);
		void update_sigma2(const QString &);
		void update_delta1(const QString &);
		void update_delta2(const QString &);
		void simulate_component();
		void update_component();
		void select_rod();
		void select_prolate();
		void select_oblate();
		void select_sphere();
};

class SimDialog : public QDialog
{
	Q_OBJECT
	public:
		SimDialog(bool *simflag, struct simulation_parameters *simparams, QWidget *parent=0, const char *name=0);
		~SimDialog();
		US_Config *USglobal;

		QLabel *title_lbl;
		QLabel *duration_lbl1;
		QLabel *duration_lbl2;
		QLabel *delay_lbl1;
		QLabel *delay_lbl2;
		QLabel *speed_lbl1;
		QLabel *speed_lbl2;
		QLabel *delta_t_lbl;
		QLabel *delta_r_lbl;
		QLabel *meniscus_lbl;
		QLabel *bottom_lbl;
		QLabel *scans_lbl;
		QLabel *rnoise_lbl;
		QLabel *inoise_lbl;
		QLabel *rinoise_lbl;

		struct simulation_parameters *temp_sp;
		int hours;
		int minutes;
		int delay_hours;
		int delay_minutes;
		long speed;
		float delta_t;
		float delta_r;
		float meniscus;
		float bottom;
		int scans;

	private:
		QwtCounter *duration_counter_h;
		QwtCounter *duration_counter_m;
		QwtCounter *delay_counter_h;
		QwtCounter *delay_counter_m;
		QwtCounter *speed_counter;
		QwtCounter *delta_t_counter;
		QwtCounter *delta_r_counter;
		QwtCounter *meniscus_counter;
		QwtCounter *bottom_counter;
		QwtCounter *scans_counter;
		QwtCounter *rnoise_counter;
		QwtCounter *inoise_counter;
		QwtCounter *rinoise_counter;
		QPushButton *pb_cancel;
		QPushButton *pb_ok;
		QPushButton *pb_save;
		QPushButton *pb_load;
		QString str;

	protected slots:
		void setup_GUI();
		void closeEvent(QCloseEvent *e);
	
	private slots:
		void update_duration_h(double);
		void update_duration_m(double);
		void update_delay_h(double);
		void update_delay_m(double);
		void update_speed(double);
		void update_delta_t(double);
		void update_delta_r(double);
		void update_meniscus(double);
		void update_bottom(double);
		void update_scans(double);
		void update_rnoise(double);
		void update_inoise(double);
		void update_rinoise(double);
		void load();
		void load(const QString &);
		void save();
		void save(const QString &);
};

class ModelDialog : public QDialog 
{
	Q_OBJECT
	public:
		ModelDialog(unsigned int *comp, int *model, 
		vector <struct component> *components, QWidget *parent=0, 
		const char *name=0);
		~ModelDialog();
		QString *s1, *s2;
		US_Config *USglobal;
		
	private:
		QListBox *lb_model;
		QLabel *model_lbl;
		QLabel *comp_lbl;
		QwtCounter *comp_counter;
		QPushButton *assign;
		QPushButton *ok;
		QPushButton *cancel;
		QPushButton *help;
		QPushButton *load;
		QPushButton *savef;
		CompDialog *cd;
		vector <struct component> *temp_components;
		int *temp_model;
		unsigned int *temp_comp;

	signals:
		void textChanged(const QString &);
	
	public slots:
		void assign_comp(double);
		void load_model();
		void load_model(const QString &);

	protected slots:
		void closeEvent(QCloseEvent *e);
			
	private slots:
		void newmodel(int);
		void savefile();
		void savefile(const QString &);
		void model_help();
		void newcomponent();
};
#endif

