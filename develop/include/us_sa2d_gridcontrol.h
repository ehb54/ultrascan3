#ifndef US_SA2D_GRIDCONTROL_H
#define US_SA2D_GRIDCONTROL_H

#include "us.h"
#include "us_util.h"
#include <qwt_counter.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <vector>
#include <algorithm>

struct sa2d_data
{
	float ff0_min;
	float ff0_max;
	unsigned int ff0_resolution;
	float s_min;
	float s_max;
	float mw_min;
	float mw_max;
	bool fit_meniscus;
	unsigned int s_resolution;
	unsigned int grid_resolution;
	unsigned int max_mer;
	QString max_mer_string;
	float regularization, meniscus_range;
	unsigned int meniscus_gridpoints;
	unsigned int uniform_grid_repetition;
	bool use_iterative;
	unsigned int max_iterations;
	unsigned int monte_carlo;
};

class US_EXTERN US_SA2D_GridControl : public QDialog
{
	Q_OBJECT
	
	public:
	
		US_SA2D_GridControl(struct sa2d_data *, QWidget *p = 0, const char *name = 0);
		~US_SA2D_GridControl();
		struct sa2d_data *SA2D_Params;
		US_Config *USglobal;	 /*!< A US_Config reference. */
		vector <float> sval;
		vector <float> frequency;
		vector <float> ff0;

	private:
	
		QCheckBox *cb_meniscus;
		QCheckBox *cb_iterations;

		QLabel *lbl_info1;
		QLabel *lbl_ff0_min;
		QLabel *lbl_ff0_max;
		QLabel *lbl_ff0_resolution;
		QLabel *lbl_s_min;
		QLabel *lbl_s_max;
		QLabel *lbl_meniscus_range;
		QLabel *lbl_meniscus_gridpoints;
		QLabel *lbl_max_iterations;
		QLabel *lbl_s_resolution;
		QLabel *lbl_regularization;
		QLabel *lbl_uniform_grid_repetition;

		QwtCounter *cnt_ff0_min;
		QwtCounter *cnt_ff0_max;
		QwtCounter *cnt_meniscus_range;
		QwtCounter *cnt_meniscus_gridpoints;
		QwtCounter *cnt_max_iterations;
		QwtCounter *cnt_ff0_resolution;
		QwtCounter *cnt_s_min;
		QwtCounter *cnt_s_max;
		QwtCounter *cnt_s_resolution;
		QwtCounter *cnt_regularization;
		QwtCounter *cnt_uniform_grid_repetition;

		QPushButton *pb_help;
		QPushButton *pb_load_distro;
		QPushButton *pb_close;
		QPushButton *pb_reject;

	private slots:
	
		void update_ff0_min(double);
		void update_ff0_max(double);
		void update_ff0_resolution(double);
		void update_s_min(double);
		void update_s_max(double);
		void update_meniscus_range(double);
		void update_meniscus_gridpoints(double);
		void update_max_iterations(double);
		void update_s_resolution(double);
		void update_regularization(double);
		void update_uniform_grid_repetition(double);
		void load_distro();
		void set_meniscus();
		void set_iterations();
		void help();
		void reject();
		void accept();
		void setup_GUI();
		void closeEvent(QCloseEvent *);
};

#endif

