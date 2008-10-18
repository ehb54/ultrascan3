#ifndef US_GRIDCONTROL_T_H
#define US_GRIDCONTROL_T_H

#include "us.h"
#include "us_util.h"
#include "us_data_io.h"
#include "us_mfem.h"
#include "us_ga_gridcontrol.h"
#include "us_sa2d_gridcontrol.h"
#include "us_sa2dbase.h"
#include "us_cluster.h"
#include <qapp.h>

struct control_parameters
{
	vector <float> float_params;
	vector <int> int_params;
};

struct gene
{
	vector <Solute> component;
};

class US_EXTERN US_GridControl_T : public QObject
{
	Q_OBJECT
	
	public:
		US_GridControl_T(const QString &, const QString &, const QString &, QObject *p = 0, const char *name = 0);
		~US_GridControl_T();

		US_Config *USglobal;
		US_Cluster *cluster;
		US_Data_IO *dataIO;

		QDateTime timestamp;
		QString analysis_type, email;
		QString timestamp_string;
		QString gcfile;
		QString gridopt;
		QString system_name;

		struct channelData channel_data;
		struct control_parameters Control_Params;
		struct ga_data GA_Params;
		struct sa2d_data SA2D_Params;
		struct runinfo run_inf;
		struct US_DatabaseLogin db_login;
		struct correctionTerms corr_inf;
		struct hydrodynamicData hydro_inf;
		vector <struct rotorInfo> rotor_list;
		vector <struct mfem_data> experiment;
		vector <struct gene> solutions;
		vector <struct centerpieceInfo> cp_list;
		bool data_loaded, has_data[8];
		bool analysis_defined;
		bool fit_tinoise, fit_rinoise;
		bool fit_meniscus, use_iterative;
		unsigned int meniscus_gridpoints, max_iterations, points, total_points;
		unsigned int selected_cell;
		unsigned int selected_lambda;
		unsigned int selected_channel;

		vector <SimparamsExtra> simparams_extra;
		float meniscus_range;
		float regularization;
		
		vector <QString> constraints_full_text;
		vector <QString> simulation_parameters_full_text;
		SimulationParameters simulation_parameters;
		vector <SimulationParameters> simulation_parameters_vec;

		QString constraints_file_name;
		QString simulation_parameters_file_name;
		ModelSystem model_system;
		ModelSystemConstraints model_system_constraints;

	private slots:
	
		void add_experiment();
		void write_experiment();
		void write_solutes(const QString &);
		void update_email(const QString &);
		void write_ga_experiment();
		void write_sa2d_experiment();
};

#endif

