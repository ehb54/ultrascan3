#ifndef US_FEMATCH_RA_H
#define US_FEMATCH_RA_H

#include "us_dtactrl.h"
#include "us_extern.h"
#include "us_resplot.h"
#include "us_util.h"
#include "us_plotdata.h"
#include "us_3dsolutes.h"
#include "us_astfem_rsa.h"
#include "../3dplot/mesh2mainwindow.h"

//#include <qcheckbox.h>
//#include <qmessagebox.h>
//#include <qwt_symbol.h>

#ifndef WIN32
#include <unistd.h>
#endif
#include <math.h>
#include <float.h>
#include <cerrno>

using namespace std;

class  US_EXTERN US_FeMatchRa_W : public Data_Control_W
{
	Q_OBJECT

	public:

		US_FeMatchRa_W(QWidget *p = 0, const char *name = 0);
		~US_FeMatchRa_W();

	private:

		unsigned int monte_carlo_iterations, simpoints;
		SA2d_control_variables sa2d_ctrl_vars;
		int mesh, moving_grid;
		float band_volume;

#ifdef WIN32
		  #pragma warning ( disable: 4251 )
#endif

		vector <double> ri_noise, ti_noise;
		vector <struct mfem_data> simdata;

#ifdef WIN32
		  #pragma warning ( default: 4251 )
#endif

		QString analysis_type;

		QLabel *lbl_variance;
		QLabel *lbl_variance2;
		QLabel *lbl_bandVolume;
		QLabel *lbl_simpoints;

		QPushButton *pb_fit;
		QPushButton *pb_loadModel;

		QwtCounter *cnt_simpoints;
		QwtCounter *cnt_lamella;

		QComboBox *cmb_radialGrid;
		QComboBox *cmb_timeGrid;

		US_ResidualPlot *resplot;
		US_Pixmap *pm;
		float rmsd;
		int model;
		bool stopFlag, movieFlag;
		struct mfem_data residuals;
		struct ModelSystem ms;
		struct SimulationParameters sp;
		US_Astfem_RSA *astfem_rsa;

	private slots:

		void setup_GUI();
		void enableButtons();
		void clear_data(struct mfem_data *);
		float fit();
		void write_res();
		void load_model();
		void clearDisplay();
		void printError(const int &);
		void update_radialGrid(int);
		void update_timeGrid(int);
		void update_simpoints(double val);
		void update_lamella(double val);

// re-implemented Functions:

		void view();
		void help();
		void save();
};

#endif

