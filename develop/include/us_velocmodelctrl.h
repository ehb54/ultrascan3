#ifndef US_VELOCMODELCONTROL_H
#define US_VELOCMODELCONTROL_H


#include <qcheckbox.h>
#include <qframe.h>
#include <qpushbt.h>
#include <qradiobt.h>
#include <iostream>
#include <qmsgbox.h>
#include <qwt_counter.h>
#include <qfile.h>
#include <qfiledlg.h>
#include <qprogressbar.h>
#include "us_util.h"
#include "us_hydro.h"
#include "us_fefit.h"
#include "us_db_tbl_vbar.h"

class US_VelocModelControl : public QFrame
{
	Q_OBJECT

	public:

		US_VelocModelControl(bool *, 		// temp_model_widget
		int, 							// temp_components
		int, 							// temp_model, 
		vector <struct fefit_component> *, 		// temp_fef_comp
		struct fefit_run **, 	// temp_fef_run
		float *, 					// temp_viscosity_tb
		float *, 					// temp_density_tb
		struct runinfo, 			// run_inf
		unsigned int, 				// selected_cell
		unsigned int, 				// selected_lambda
		unsigned int, 				// selected_channel
		vector <float>,			// Vbar20
		QWidget *p = 0, const char *name = 0);
		~US_VelocModelControl();
		vector <struct fefit_component> *fef_comp;
		fefit_run **fef_run;
		struct runinfo run_inf;
		int selected_cell;
		int selected_lambda;
		int selected_channel;
		vector <float> const_Vbar20; // used to save original vbar20 (reset will restore this)
		vector <float> Vbar20; // new vbar20 value overridden by user 

		int model;
		int components;
		bool *model_widget;
		float vbar, temperature;
		float *density_tb, *viscosity_tb;
		bool vbar_from_seq;
		float baseline;
		float meniscus;
		float total_conc;
		float buoyancyw, buoyancyb;
		int current_component;
		US_Config *USglobal;
		US_Hydro3 *shape;
		US_Vbar_DB *vbar_dlg;
		QString newVal;

//Headers:

		QLabel *lbl_header;
		QLabel *lbl_sample;
		QLabel *lbl_sample_descr;
		QLabel *lbl_legend1;
		QLabel *lbl_legend2;
		QLabel *lbl_legend3;
		QLabel *lbl_legend4;
		QLabel *lbl_legend5;
		QLabel *lbl_legend6;
		QLabel *lbl_meniscus;
		QLabel *lbl_baseline;
		QLabel *lbl_slope;
		QLabel *lbl_stray;
		QLabel *lbl_simpoints;
		QLabel *lbl_component;
		QLabel *lbl_conc;
		QLabel *lbl_sed;
		QLabel *lbl_diff;
		QLabel *lbl_sigma;
		QLabel *lbl_delta;
		QLabel *lbl_percent;

		QLabel *lcd_meniscus;
		QLabel *lcd_baseline;
		QLabel *lcd_slope;
		QLabel *lcd_stray;
		QLabel *lcd_conc;
		QLabel *lcd_sed;
		QLabel *lcd_diff;
		QLabel *lcd_sigma;
		QLabel *lcd_delta;


//Results:

		QLabel *lbl_result;
		QLabel *lbl_mw;
		QPushButton *pb_vbar;
		QLabel *lbl1_s20w;
		QLabel *lbl1_d20w;
		QLabel *lbl2_s20w;
		QLabel *lbl2_d20w;

		QLineEdit *le_meniscus;
		QLineEdit *le_baseline;
		QLineEdit *le_slope;
		QLineEdit *le_stray;
		QLineEdit *le_conc;
		QLineEdit *le_sed;
		QLineEdit *le_s20w;
		QLineEdit *le_diff;
		QLineEdit *le_d20w;
		QLineEdit *le_sigma;
		QLineEdit *le_delta;
		QLineEdit *le_mw;
		QLineEdit *le_vbar;
		QLineEdit *le_meniscus_range;
		QLineEdit *le_baseline_range;
		QLineEdit *le_slope_range;
		QLineEdit *le_stray_range;
		QLineEdit *le_conc_range;
		QLineEdit *le_sed_range;
		QLineEdit *le_diff_range;
		QLineEdit *le_sigma_range;
		QLineEdit *le_delta_range;
		QPushButton *pb_update;
		QPushButton *pb_model;
		QPushButton *pb_load;
		QPushButton *pb_quit;
		QPushButton *pb_noise;
		QPushButton *pb_help;
		QPushButton *pb_reset;
		QPushButton *pb_save;
		QPushButton *pb_reset_range;
		QwtCounter *ct_component;
		QwtCounter *ct_simpoints;
		QCheckBox *cb_meniscus_fix;
		QCheckBox *cb_baseline_fix;
		QCheckBox *cb_slope_fix;
		QCheckBox *cb_stray_fix;
		QCheckBox *cb_conc_fix;
		QCheckBox *cb_sed_fix;
		QCheckBox *cb_diff_fix;
		QCheckBox *cb_sigma_fix;
		QCheckBox *cb_delta_fix;
		QCheckBox *cb_mw_fix;
		QCheckBox *cb_vbar_fix;
		QCheckBox *cb_meniscus_float;
		QCheckBox *cb_baseline_float;
		QCheckBox *cb_slope_float;
		QCheckBox *cb_stray_float;
		QCheckBox *cb_conc_float;
		QCheckBox *cb_sed_float;
		QCheckBox *cb_diff_float;
		QCheckBox *cb_sigma_float;
		QCheckBox *cb_delta_float;
		QCheckBox *cb_mw_float;
		QCheckBox *cb_vbar_float;

	public slots:

		void update_labels();
		void update_meniscus(const QString &);
		void update_meniscus_range(const QString &);
		void update_meniscus_fix();
		void update_meniscus_float();
		void update_baseline(const QString &);
		void update_baseline_range(const QString &);
		void update_baseline_fix();
		void update_baseline_float();
		void update_slope(const QString &);
		void update_slope_range(const QString &);
		void update_slope_fix();
		void update_slope_float();
		void update_stray(const QString &);
		void update_stray_range(const QString &);
		void update_stray_fix();
		void update_stray_float();
		void assign_component(double);
		void assign_simpoints(double);
		void update_sed(const QString &);
		void update_sed_range(const QString &);
		void update_sed_fix();
		void update_sed_float();
		void update_diff(const QString &);
		void update_diff_range(const QString &);
		void update_diff_fix();
		void update_diff_float();
		void update_conc(const QString &);
		void update_conc_range(const QString &);
		void update_conc_fix();
		void update_conc_float();
		void update_sigma(const QString &);
		void update_sigma_range(const QString &);
		void update_sigma_fix();
		void update_sigma_float();
		void update_delta(const QString &);
		void update_delta_range(const QString &);
		void update_delta_fix();
		void update_delta_float();
		void update_mw(const QString &);
		void update_mw_fix();
		void update_mw_float();
		void update_model_vbar(const QString &);
		void update_vbar_lbl(float, float);
		void read_vbar();
		void update_vbar_fix();
		void update_vbar_float();
		void save_model();
		void save_model(const QString &);
		void load_model();
		void load_model(const QString &);
		void calc_mw(const int);
		void calc_vbar(const int);
		void calc_D(const int);
		void quit();
		void help();
		void show_model();
		void reset();
		void reset_range();
		void meniscus_red();
		void meniscus_green();
		void meniscus_dk_green();
		void baseline_red();
		void baseline_green();
		void baseline_dk_green();
		void slope_red();
		void slope_green();
		void slope_dk_green();
		void stray_red();
		void stray_green();
		void stray_dk_green();
		void sed_red();
		void sed_green();
		void sed_dk_green();
		void diff_red();
		void diff_green();
		void diff_dk_green();
		void conc_red();
		void conc_green();
		void conc_dk_green();
		void sigma_red();
		void sigma_green();
		void sigma_dk_green();
		void delta_red();
		void delta_green();
		void delta_dk_green();

	protected slots:
		void closeEvent(QCloseEvent *e);
};

#endif
