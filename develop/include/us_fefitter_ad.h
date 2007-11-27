#ifndef US_FEFITTER_AD_H
#define US_FEFITTER_AD_H

#include <qstring.h>
#include "us_minimize.h"
#include "us_htmledit.h"
#include "us_velocmodelctrl.h"
#include "us_fefitter_single.h"

class US_EXTERN US_FeFitter_AD : public US_Minimize
{
	Q_OBJECT
	
	public:
		US_FeFitter_AD(bool * /* fitting_widget */,
							bool /* GUI */,
							unsigned int /* components */,
							int /* model */,
							vector <struct fefit_component> *,
							struct fefit_run *,
							struct runinfo *,
							unsigned int, /* number of points for an individual scan */
							unsigned int, /* current cell */
							unsigned int, /* current lambda */
							float, /* boundary range */
							float, /* bounday position */
							float, /* density */
							float, /* density_tb */
							float, /* viscosity */
							float, /* viscosity_tb */
							float, /* centerpiece bottom */ 
							float *, /* variance */
							double **, /* absorbance scan matrix */
							double **, /* simulated solution */
							double **, /* residuals */
							double *, /* radius vector */
							QWidget *p=0,
							const char *name = 0);
		
		~US_FeFitter_AD();
		
		int model;
		short int tag;
		unsigned int scanpoints, cell, lambda, sim_points, components, all_sim_points, offset1, offset2;
		double **absorbance, *radius, **simulation_c, **resid;
		adouble *cu, *cv, *cw, sigma, delta, *ad_par, *right, *right_old;
		adouble *s_bar, *D_bar, **partial_c, **concentration, **left, *c_current, **left_temp;
		float **u, **v, **w, bd_position, bd_range, viscosity, density, viscosity_tb, density_tb;
		float **a1, **a2, **b, **b_orig;
		bool GUI, *fitting_widget, tape_construct, conc_dep_flag, init_flag, print_bw, print_inv;
		float *sim_radius, bottom;
		vector <struct fefit_component> *component_vector;
		struct fefit_run *runData;
		struct runinfo *run_inf;
		struct veloc_parameterIndex adParameterIndex;
		struct veloc_parameters veloc_par;
		US_Config *USglobal;
		QwtPlot *residuals_plot;
		QString htmlDir;

	private slots:
	
		void cleanup();
		int calc_jacobian();
		void parameter_map_to_component_vector(double *);
		void parameter_map_to_guess();
		void init_finite_element();
		float calc_residuals();
		void cleanup_finite_element();
		
	public slots:
		bool fit_init();
		int calc_model(double *);
		void view_report();
		void write_report();
		void endFit();
		void print();
		bool createHtmlDir();
		void saveFit();
		void saveFit(const QString &);
		void assign_orig();
		void check_constraints(double *);

	protected slots:

		void resizeEvent(QResizeEvent *e);
		
	signals:
	
		void dataSaved(const QString &, const int);	//needed to save a list of used scans
		void fitLoaded();
		
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
};

#endif

