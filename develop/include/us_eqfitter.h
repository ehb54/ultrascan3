#ifndef US_EQFITTER_H
#define US_EQFITTER_H

#include "us_minimize.h"
#include "us_htmledit.h"
#include "us_eqmodelctrl.h"
#include "us_sassoc.h"
#include "us_editor.h"
#include "us_pixmap.h"
#include "us_extern.h"

struct opticalProfile
{

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

	vector <float> absorbance;
	vector <float> residual;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
};

class US_EXTERN US_EqFitter : public US_Minimize
{
	Q_OBJECT
	
	public:
		US_EqFitter(vector <struct EquilScan_FitProfile> *, vector <struct runinfo> *, 
		struct EquilRun_FitProfile *,	class US_EqModelControl **temp_model_control, unsigned int temp_model,
		bool temp_GUI, vector <QString> *temp_scan_info, 
		bool *temp_fitting_widget, QString *temp_projectName, float temp_od_limit, QWidget *p=0, const char *name = 0);
		
		~US_EqFitter();
		
		struct opticalProfile profile_230, profile_280, profile_220, profile_215, profile_210;
		struct opticalProfile profile_208, profile_205, profile_260, profile_262;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

		vector <struct EquilScan_FitProfile> *scanfit_vector;
		vector <struct runinfo> *run_information;
		vector <struct rotorInfo> rotor_list;
		vector <struct centerpieceInfo> cp_list;
		vector <unsigned int> logpoints_per_dataset;
		vector <float> mwpoints;
		vector <QString> *scan_info;
		
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
    
    struct EquilRun_FitProfile *runfit;
		class US_EqModelControl **model_control;
		US_Sassoc *sas_w;
		US_Pixmap *pm;
		unsigned int first_fitted_scan;
		QString *projectName;
		US_Editor *e;
		double **lncr2, **dcr2, **dlncr2;
		bool GUI, print_plot, initialized_1, initialized_2;
		QString htmlDir;
		int which_model;
		float concentration_average, od_limit;
		QwtPlot *histogram_plot;
		
	private slots:
	
		void cleanup();
		void updateRange(double);
		void saveFit();
		void saveFit(const QString &);
		int calc_jacobian();
		bool try_adjustment();
		
	public slots:
		void view_report();
		int calc_model(double *);
		bool fit_init();
		void guess_mapForward(double *);
		void parameter_mapBackward(double *);
		void parameter_addRandomNoise(double *, float);
		void write_data();
		void write_report();
		void plot_overlays();
		void plot_residuals();
		void plot_two();
		void plot_three();
		void plot_five();
		void plot_four();
		void plot_six();
		void print();
		void calc_integral();
		void endFit();
		bool createHtmlDir();
		void calc_dlncr2();
		void print_histogram();

		void startFit();
		int GLLS_global();
		int GLLS_individual();
		int NNLS_individual();
		
	signals:
	
		void dataSaved(const QString &, const int);	//needed to save a list of used scans
};

#endif

