#ifndef US_FINITE_SINGLE_H
#define US_FINITE_SINGLE_H

#include <qprogbar.h>

#include "us_selectmodel3.h"

#include "us_util.h"
#include "us_pixmap.h"
#include "us_dtactrl.h"
#include "us_finite2.h"
#include "us_selectmodel.h"
#include "us_montecarlo.h"
#include "us_montecarlostats.h"
#include "us_fefitter_single.h"
#include "us_velocmodelctrl.h"

class US_EXTERN US_Finite_single : public Data_Control_W
{
	Q_OBJECT
	
	public:
	
		US_Finite_single(QWidget *p = 0, const char *name = 0);
		~US_Finite_single();
		US_Editor *e;
		QPushButton *pb_create_model;
		QPushButton *pb_model_control;
		QPushButton *pb_fit_control;
		QPushButton *pb_load_fit;
		QPushButton *pb_save_fit;
		QLabel *residuals_lbl;
		QwtCounter *residuals_counter;
		US_VelocModelControl *model_control;
		US_FeFitter_Single *velocFitter;
		US_Pixmap *pm;
		vector <struct fefit_component> component_vector;
		fefit_run *runData;
		vector <struct rotorInfo> rotor_list;
		vector <struct centerpieceInfo> cp_list;
		vector <float> residuals_v;// vector of residuals for Monte Carlo
		vector <float> vbar_model;	// vbar array containing all vbar20's for a current cell and current channel
		bool fitting_widget;			// keeps track of the fitting widget's state
		bool model_widget;			// keeps track of the model widget's state
		bool monte_carlo_widget;	// keeps track of the Monte Carlo widget's state
		bool suspend_flag;			// flag to allow temporary suspension of fitting process
		bool completed;				// current Simulation event has completed if true
		bool aborted;					// true if fit has been aborted
		bool converged;				// true if the fit exited normally
		unsigned int components;	// number of components
		int model;						// selected model
		unsigned int parameters;	// number of parameters to be fitted
		bool constrained_fit;		// are we fitting constrained or not?
		float standard_deviation, variance;	// used for monte carlo and load_fit
		US_MonteCarlo *monte_carlo_window;
		struct MonteCarlo mc;
		QString fitName;
		Q_UINT16 sim_points;			// number of radial points/scan in the simulation ('points' is for raw data)
		double **F;						// the concentration matrix containing all simulated scans for the run, same size as absorbance 
		double **residuals;
		bool first_plot;				// keeps track of when to call the residuals from update_residuals
		QString model_id;

// Variables for monomer-dimer model:

		float **partial_c, *s_bar, *D_bar;

	private:
	
		bool model_defined, mem_initialized;
		double line_y[5][2];
		double line_x[2];
		uint line[5];
		int comp;
		int divisions;
		int count;
		int exclude;

	private slots:

		void write_fef();
		void reset();
		void change_tape_status();

// re-implemented Functions:

		void create_model();
		void cleanup();
		void select_model_control();
		void select_fit_control();
		void start_fit();
		void suspend_fit();
		void resume_fit();
		void cancel_fit();
		void load_fit();
		void load_fit(const QString &);
		void update_residuals(double);
		int plot_analysis();
		void update_plot(const unsigned int);
		void view();
		void help();
		void save();
		void second_plot();	// substitutes as virtual function for monte carlo
		void monte_carlo();
		void fit_init();
		void fit_converged();
		void fit_started();
		void updateButtons();
		void updateModelWindow();
		void updateMonteCarlo();
		
	protected slots:

		void closeEvent(QCloseEvent *);
};
#endif

