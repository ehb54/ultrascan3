#ifndef US_DCDRDAT_H
#define US_DCDRDAT_H


#include "us_editor.h"
#include "us_util.h"
#include "us_pixmap.h"
#include "us_dtactrl.h"
#include "us_average.h"
#include <qprogbar.h>
#include <qwt_scldiv.h>

class US_EXTERN dcdr_dat_W : public Data_Control_W
{
	Q_OBJECT
	
	public:
	
		dcdr_dat_W(QWidget *p = 0, const char *name = 0);
		~dcdr_dat_W();
		US_Editor *e;
		US_Average *avg;
		
	private:
		QRadioButton *bt_r, *bt_s;
		QString title;
		QLabel *sval_lbl;
		QLabel *blank_lbl;
		QwtCounter *sval_counter;
		US_Pixmap *pm;
		unsigned int exclude;
		double *frequency;
		double **dcdr_points;
		double **sval;
		unsigned int analysis_curve;
		unsigned int *plots;
		bool dcdr_plots;
		bool start;
		bool setup_already;
		double smin, smax;

	private slots:

		void setup_GUI();
		void write_dcdr_avg();
		void write_dcdr_scans();
		void write_dcdr_sval();
		void write_res();
		void update_sval(double);
		void update_r_button();
		void update_s_button();
		void cleanup();

// Virtual Functions:

		int plot_analysis();
		int setups();
		void view();
		void help();
		void save();
		void second_plot();
};


#endif

