#ifndef US_SA2D_CONTROL_H
#define US_SA2D_CONTROL_H

#include "us.h"
#include "us_extern.h"
#include "us_util.h"
#include "../3dplot/mesh2mainwindow.h"

#include <qlabel.h>
#include <qgl.h>
#include <qframe.h>
#include <qstring.h>
#include <qdialog.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qlayout.h>
#include <qprogressbar.h>

struct solute_description
{
	double s, D, ff0, f, c, mw;
};

class US_EXTERN US_Sa2d_Control : public QFrame
{
	Q_OBJECT
	
	public:
		US_Sa2d_Control(SA2d_control_variables *, bool *, QWidget *p=0, const char *name = 0);
		~US_Sa2d_Control();

		SA2d_control_variables *sa2d_ctrl_vars;
		vector <struct element_3D> solutes_3D;
		vector <struct solute_description> solutes;
		bool *control_window_flag, plot_window_flag;
		bool mw1_flag;
		bool mw2_flag;
		bool s1_flag;
		bool s2_flag;
		bool D1_flag;
		bool D2_flag;
		bool f1_flag;
		bool f2_flag;
		bool ff01_flag;
		bool ff02_flag;
		bool coalesce;
		bool clip;
		bool random;
		bool widget3d_flag;
		bool regularize;
		bool uniform;
		bool local_uniform;
		bool autoupdate;
		bool fit_ti;
		bool fit_ri;
		Mesh2MainWindow *mainwindow;

		QProgressBar *progress;
		QLabel *lbl_status;
		QLabel *lbl_fitcontrol;
		QLabel *lbl_plotcontrol;
		QLabel *lbl_info;
		QLabel *lbl_max_s;
		QLabel *lbl_min_s;
		QLabel *lbl_resolution_s;
		QLabel *lbl_max_ff0;
		QLabel *lbl_min_ff0;
		QLabel *lbl_resolution_ff0;
		QLabel *lbl_dimension1;
		QLabel *lbl_dimension2;
		QLabel *lbl_iteration1;
		QLabel *lbl_iteration2;
		QLabel *lbl_variance1;
		QLabel *lbl_variance2;
		QLabel *lbl_new_variance1;
		QLabel *lbl_new_variance2;
		QLabel *lbl_difference1;
		QLabel *lbl_difference2;
		QLabel *lbl_memory1;
		QLabel *lbl_memory2;
		QLabel *lbl_mw;
		QLabel *lbl_s;
		QLabel *lbl_D;
		QLabel *lbl_f;
		QLabel *lbl_ff0;
		QLabel *lbl_parameter;
		QLabel *lbl_uniform;
		QLabel *lbl_local_uniform;
		QLabel *lbl_random;
		QLabel *lbl_regularize;
		QLabel *lbl_coalesce;
		QLabel *lbl_uniform_steps;
		QLabel *lbl_clip_steps;
		QLabel *lbl_local_uniform_steps;
		QLabel *lbl_random_steps;
		QLabel *lbl_scaling_factor;
		QLabel *lbl_scaling_factor2;
		QLabel *lbl_distance;
		QLabel *lbl_metric;
//		QLabel *lbl_threshold;
		QLabel *lbl_clip;
		QLabel *lbl_regfactor;
		QLabel *lbl_optimization;
		QLabel *lbl_thread_count;

		QCheckBox *cb_fit_ti;
		QCheckBox *cb_fit_ri;
		QCheckBox *cb_autoupdate;
		QCheckBox *cb_posBaseline;
		QCheckBox *cb_negBaseline;
		QCheckBox *cb_ff01;
		QCheckBox *cb_ff02;
		QCheckBox *cb_mw1;
		QCheckBox *cb_mw2;
		QCheckBox *cb_s1;
		QCheckBox *cb_s2;
		QCheckBox *cb_D1;
		QCheckBox *cb_D2;
		QCheckBox *cb_f1;
		QCheckBox *cb_f2;

		QCheckBox *cb_uniform;
		QCheckBox *cb_local_uniform;
		QCheckBox *cb_random;
		QCheckBox *cb_regularize;
		QCheckBox *cb_coalesce;
		QCheckBox *cb_clip;

		QwtCounter *cnt_max_s;
		QwtCounter *cnt_min_s;
		QwtCounter *cnt_resolution_s;
		QwtCounter *cnt_max_ff0;
		QwtCounter *cnt_min_ff0;
		QwtCounter *cnt_resolution_ff0;
		QwtCounter *cnt_uniform_steps;
		QwtCounter *cnt_clip_steps;
		QwtCounter *cnt_local_uniform_steps;
		QwtCounter *cnt_random_steps;
		QwtCounter *cnt_scaling_factor2;
		QwtCounter *cnt_scaling_factor;
		QwtCounter *cnt_random_distance;
		QwtCounter *cnt_regfactor;
		QwtCounter *cnt_metric;
//		QwtCounter *cnt_threshold;
		QwtCounter *cnt_thread_count;

		double clip_steps;
		double uniform_steps;
		double local_uniform_steps;
		double random_steps;
		double scaling_factor;
		double scaling_factor2;
		double random_distance;
		double regfactor;
		double metric;
//		double threshold;
		double thread_count;

		QPushButton *pb_fit;
		QPushButton *pb_cancel;
		QPushButton *pb_help;
		QPushButton *pb_plot;
		QPushButton *pb_save;

		US_Config *USglobal;	 /*!< A US_Config reference. */

		int iteration;
		bool fit_status;
		
	public slots:
	
		void cancel();
		void help();
		void calc_memory();

	private slots:
	
		void update_max_s(double);
		void update_min_s(double);
		void update_max_ff0(double);
		void update_min_ff0(double);
		void update_resolution_s(double);
		void update_resolution_ff0(double);
		void update_uniform_steps(double);
		void update_clip_steps(double);
		void update_local_uniform_steps(double);
		void update_random_steps(double);
		void update_scaling_factor(double);
		void update_scaling_factor2(double);
		void update_random_distance(double);
		void update_regfactor(double);
		void update_metric(double);
		void update_thread_count(double);
//		void update_threshold(double);
		void set_posBaseline();
		void set_negBaseline();
		void set_mw1();
		void set_mw2();
		void set_s1();
		void set_s2();
		void set_D1();
		void set_D2();
		void set_f1();
		void set_f2();
		void set_ff01();
		void set_ff02();
		void set_random();
		void set_uniform();
		void set_local_uniform();
		void set_regularize();
		void set_coalesce();
		void set_clip();
		void set_autoupdate();
		void set_fit_ti();
		void set_fit_ri();
		
	public slots:
		void plot();
		
	protected slots:
		void setup_GUI();
		void closeEvent(QCloseEvent *);
		
	signals:
	
		void mem_changed(); 
		void distribution_changed(); 
};

#endif
