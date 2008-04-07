#ifndef US_FEMATCH_RA_H
#define US_FEMATCH_RA_H

#include "us_dtactrl.h"
#include "us_extern.h"
#include "us_mfem.h"
#include "us_resplot.h"
#include "us_util.h"
#include "us_plotdata.h"
#include "us_3dsolutes.h"
#include "../3dplot/mesh2mainwindow.h"

#include <qcheckbox.h>
#include <qwt_symbol.h>
#include <cerrno>

#ifndef WIN32
#include <unistd.h>
#endif
#include <math.h>
#include <float.h>


using namespace std;

class  US_EXTERN US_FeMatchRa_W : public Data_Control_W
{
	Q_OBJECT

	public:

		US_FeMatchRa_W(QWidget *p = 0, const char *name = 0);
		~US_FeMatchRa_W();

	private:

		unsigned int monte_carlo_iterations;
		SA2d_control_variables sa2d_ctrl_vars;
		vector <double> ri_noise, ti_noise;
		QString analysis_type;
		QLabel *lbl_variance;
		QLabel *lbl_variance2;
		QPushButton *pb_fit;
		QPushButton *pb_loadModel;
		US_ResidualPlot *resplot;
		US_Pixmap *pm;
		float rmsd;
		int model;
		struct mfem_data residuals, fem_model;

	private slots:

		void setup_GUI();
		void enableButtons();
		void update_distribution();
		void clear_data(struct mfem_data *);
		void updateParameters();
		void fit();
		float calc_residuals();
		void write_cofs();
		void write_res();
		void calc_distros();
		void load_model();
		void load_model(const QString &);
		void clearDisplay();

// re-implemented Functions:

		void view();
		void help();
		void save();
};

#endif

