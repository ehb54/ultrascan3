#ifndef US_EQUILMC_T_H
#define US_EQUILMC_T_H

#include <qapp.h>
#include <qstyle.h>
#include <qapp.h>
#include <qwidget.h>
#include <unistd.h>

#include "us.h"
#include "us_eqfitter.h"
#include "us_util.h"
#include "us_montecarlo.h"
#include "us_math.h"
#include "us_dtactrl.h"

class US_EXTERN US_EquilMC_T : public QFrame
{
	Q_OBJECT
	
	public:
	
//		US_EquilMC_T(QWidget *p = 0, const char *name = 0);
		US_EquilMC_T();
		~US_EquilMC_T();
		bool model_widget;
		bool monte_carlo_widget;
		bool fitting_widget;
		bool has_data[8];
		bool mouse_was_moved;
		bool float_parameters;
		bool scans_have_problems;
		bool show_messages;
		bool scans_initialized;
		vector <struct rotorInfo> rotor_list;
		vector <struct centerpieceInfo> cp_list;
		QString projectName, htmlDir;
		int model;
		unsigned int dataset, scans_in_list, first_fitted_scan, fitpoints, parameters;
		uint selected_scan;
		unsigned int new_start_index, new_stop_index;
		vector <struct EquilScan_FitProfile> scanfit_vector;
		struct EquilRun_FitProfile runfit;
		struct runinfo run_inf;
		float bottom[8], std_deviation;
		float od_limit;	//limit of the max. upper absorbance/fringe value
		vector <QString> filenames;
		vector <struct runinfo> run_information;
		vector <QString> scan_info;
		vector <float> residuals, original_data, yfit;
		US_MonteCarlo *monte_carlo_window;
		struct MonteCarlo mc;
		
// These parameters are needed to set up the fixed molecular weight distribution model:
		float mw_upperLimit, mw_lowerLimit, model3_vbar;
		bool model3_vbar_flag;

// These parameters are needed for the user-defined equilibrium models:
		float nmer_size1, nmer_size2;

		US_Config *USglobal;
		US_EqModelControl *model_control;
		US_ShowDetails *ShowDetails;
		US_SelectModel *SelectModel;
		US_InfoPanel *InfoPanel;
		US_EqFitter *eqFitter;
		RunDetails_F *run_details;
		int xpos, ypos, buttonw, buttonh, border;
		void read_init(const QString &);
		void monte_carlo();
		void monte_carlo_iterate();
		
	private slots:
		void load(const QString &);
		int assign_scanfit(unsigned int, unsigned int, unsigned int);
		void update_projectName(const QString &);
		bool createHtmlDir();
		void update_limit(float);
		void fitcontrol();
		void loadfit(const QString &);
		void setup_vectors();
};

#endif

