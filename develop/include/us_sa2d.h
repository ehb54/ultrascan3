#ifndef US_SA2D_H
#define US_SA2D_H

#include "us_dtactrl.h"
#include "us_extern.h"
#include "us_resplot.h"
#include "us_sa2dbase.h"
#include "us_sa2d_control.h"
#include "us_plotdata.h"
#include <qlabel.h>
#include <qcheckbox.h>
#include <qwt_symbol.h>
#include <cerrno>

class  US_EXTERN US_SA2D_W : public Data_Control_W
{
	Q_OBJECT
	
	public:
	
		US_SA2D_W(QWidget *p = 0, const char *name = 0);
		US_SA2D_W(double, double, QString, unsigned int, unsigned int, QWidget *p=0, const char *name=0);
		~US_SA2D_W();
		SA2D *sa2d;
		Simulation_values Simulation_Routine(Simulation_values);
		US_Sa2d_Control *SA_control;
		
	private:
	
		US_PlotData *additional_plots;
		US_ResidualPlot *resplot;
		US_Pixmap *pm;
		QPushButton *pb_control;
		QPushButton *pb_addplots;
		QPushButton *pb_loadfit;
		SA2d_control_variables sa2d_ctrl_vars;
		QLabel *lbl_info1;
		QLabel *lbl_info2;

		float rmsd;
		double old_variance;
		bool fit_status, fit_posBaseline, fit_negBaseline, control_window_flag, additional_plots_flag;
		unsigned int resolution, resolution_ff0;
		vector <double> s_distribution;
		vector <double> D_distribution;
		vector <double> ff0_distribution;
		vector <double> frequency;
		vector <double> s20w;
		vector <double> D20w;
		vector <double> mw;
		struct mfem_data residuals, model;
		float f_f0;
		unsigned int iteration;
		vector <double> ti_noise;
		vector <double> ri_noise;
		
	private slots:

		void sa2d_GUI();
		void setup_GUI();
		void enableButtons();
		void update_distribution();
		void control_window();
		void addplots();
		void clear_data(struct mfem_data *);
		float calc_residuals();
		void write_sa2d();
		void write_res();
		void calc_20W_distros();
		void calc_points();
		void fit();
		void loadfit();
		void loadfit(QString);
		void plot_graph();
		void update_result_vector();

// re-implemented Functions:

		void view();
		void help();
		void save();
		void second_plot();
};

#endif

