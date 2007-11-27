#ifndef US_3DSOLUTES_H
#define US_3DSOLUTES_H

#include "us.h"
#include "us_extern.h"
#include "us_util.h"
#include "us_mfem.h"
#include "us_plotdata.h"
#include "../3dplot/mesh2mainwindow.h"
#include "us_sa2d_control.h"

#include <qlabel.h>
#include <qgl.h>
#include <qframe.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qwt_counter.h>

class US_EXTERN US_3d_Solutes : public QFrame
{
	Q_OBJECT
	
	public:
		US_3d_Solutes(SA2d_control_variables *, bool *, QString, unsigned int *, unsigned int *, unsigned int, 
		unsigned int, double **, double *, struct mfem_data *, 
		vector <double> *, vector <double> *, QWidget *p=0, const char *name = 0);
		~US_3d_Solutes();

		SA2d_control_variables *sa2d_ctrl_vars;
		US_PlotData *residual_plots;

		vector <struct element_3D> solutes_3D;
		vector <struct solute_description> solutes;
		QString run_id;
		unsigned int *scans, *points, current_cell, current_lambda;
		double **absorbance, *radius, zAdjust, resolutionAdjust, alphaAdjust, betaAdjust;
		struct mfem_data *fem_model;
		vector <double> *ti_noise, *ri_noise;
		
		bool *window_3d_flag, plot_window_flag;
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
		bool widget3d_flag, residual_plots_flag;

		Mesh2MainWindow *mainwindow;
		QLabel *lbl_info;
		QLabel *lbl_dimension1;
		QLabel *lbl_dimension2;
		QLabel *lbl_mw;
		QLabel *lbl_s;
		QLabel *lbl_D;
		QLabel *lbl_f;
		QLabel *lbl_ff0;
		QLabel *lbl_parameter;
		QLabel *lbl_zAdjust;
		QLabel *lbl_betaAdjust;
		QLabel *lbl_alphaAdjust;
		QLabel *lbl_resolutionAdjust;

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

		QPushButton *pb_cancel;
		QPushButton *pb_help;
		QPushButton *pb_plot;
		QPushButton *pb_residuals;
		
		QwtCounter *cnt_zAdjust;
		QwtCounter *cnt_resolutionAdjust;
		QwtCounter *cnt_alphaAdjust;
		QwtCounter *cnt_betaAdjust;

		US_Config *USglobal;	 /*!< A US_Config reference. */

		
	public slots:
	
		void cancel();
		void help();

	private slots:
	
		void setup();
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
		void set_zAdjust(double);
		void set_resolutionAdjust(double);
		void set_alphaAdjust(double);
		void set_betaAdjust(double);
		void plot_residuals();
		
	public slots:
		void plot();
		
	protected slots:
		void setup_GUI();
		void closeEvent(QCloseEvent *);
		
};

#endif
