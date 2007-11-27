#ifndef US_LNCR2_H
#define US_LNCR2_H


#include "us_vhwsim.h"
#include "us_editor.h"
#include "us_util.h"
#include "us_extern.h"
#include "us_dtactrl.h"

#include <qprogbar.h>

class US_EXTERN US_lncr2_W : public Data_Control_W
{
	Q_OBJECT
	
	public:
	
		US_lncr2_W(QWidget *p = 0, const char *name = 0);
		~US_lncr2_W();
		US_Editor *e;
		
	private:

		QLabel *baseline_lbl;
		QwtCounter *baseline_counter;
		double span, interval;
		double baseline;
		double **tempx;
		double **tempy;
		int *newcount;
		float *mw;
		float *runs;
		double *slope;
		double *intercept;
		double *sigma;
		double *correl;
		uint *lines;
		uint *line_fits;
		double *lines_y;
		double *lines_x;

	private slots:

		void setup_GUI();
		void write_lncr2();
		void write_res();
		void update_baseline(double);
		
// re-implemented Functions:

		int plot_analysis();
		void view();
		void help();
		void save();
		void second_plot();
		void reset();
};

#endif

