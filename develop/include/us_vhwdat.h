#ifndef US_VHWDAT_H
#define US_VHWDAT_H

#include <qprogbar.h>
#include <qprinter.h>


#include "us_vhwsim.h"
#include "us_editor.h"
#include "us_util.h"
#include "us_pixmap.h"
#include "us_dtactrl.h"
#include "us_extern.h"

class  US_EXTERN vhw_dat_W : public Data_Control_W
{
	Q_OBJECT
	
	public:
	
		vhw_dat_W(QWidget *p = 0, const char *name = 0);
		~vhw_dat_W();
		US_Editor *e;
		Distribution *dis;
		
	private:
	
		QLabel *division_lbl;
		QPushButton *pb_groups;
		QwtCounter *division_counter;
		US_Pixmap *pm;
		unsigned int exclude, exclude_singlecurve, exclude_rangecurve;
		bool *excluded;
		bool groups_on;
		bool setup_already; // for garbage collection
		unsigned int groups;
		float start_y, max_sed;
		float stop_y;
		float C_zero;
		bool group_start;		//true if we are ready to select the first point of a group
		float *group_sval;
		float *group_percent;
		uint *sval_label;
		unsigned int divisions;
		double span, interval;
		double *tempx;
		double *tempy;
		double **sed_app;
		double *slope;
		double *intercept;
		double *boundary_fractions;
		uint *sa_points;
		uint *sa_lines;
		double *lines_y;
		double *lines_x;
		double sigma;
		double correl;

	private slots:

		void setup_GUI();
		void write_vhw();
		void write_dis();
		void write_res();
		void select_groups();
		void update_divisions(double);
		void getAnalysisPlotMouseReleased(const QMouseEvent &e);
		void getAnalysisPlotMousePressed(const QMouseEvent &e);
		void cleanup();
		void save_model();

// re-implemented Functions:

		int plot_analysis();
		int setups();
		void view();
		void help();
		void save();
		void second_plot();
		void excludeSingleSignal(unsigned int);
		void excludeRangeSignal(unsigned int);
};

#endif

