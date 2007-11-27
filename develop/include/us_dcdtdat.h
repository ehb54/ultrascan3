#ifndef US_DCDTDAT_H
#define US_DCDTDAT_H


#include <qprogbar.h>
#include "us_editor.h"
#include "us_util.h"
#include "us_pixmap.h"
#include "us_dtactrl.h"
#include "us_average.h"

class US_EXTERN dcdt_dat_W : public Data_Control_W
{
	Q_OBJECT
	
	public:
	
		dcdt_dat_W(QWidget *p = 0, const char *name = 0);
		~dcdt_dat_W();
		US_Editor *e;
		US_Average *avg;
		
	private:
		QRadioButton *bt_r, *bt_s;
		QLabel *sval_lbl;
		QLabel *blank_lbl;
		QString title;
		QwtCounter *sval_counter;
		US_Pixmap *pm;
		unsigned int exclude;
		double *frequency;
		double **dcdt_points;
		double **sval;
		unsigned int analysis_curve;
		unsigned int *plots;
		bool dcdt_plots;
		bool start;
		bool setup_already;
		double smin, smax;

	private slots:
		
		void setup_GUI();
		void write_dcdt_avg();
		void write_dcdt_scans();
		void write_dcdt_sval();
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

