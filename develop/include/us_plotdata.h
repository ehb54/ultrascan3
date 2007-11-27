#ifndef US_PLOTDATA_H
#define US_PLOTDATA_H

#include "us.h"
#include "us_extern.h"
#include "us_util.h"
#include "us_mfem.h"
#include "us_resplot.h"

#include <vector>
#include <string>

#include <qlabel.h>
#include <qframe.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qmessagebox.h>

#include <qwt_plot.h>


class US_EXTERN US_PlotData : public QFrame
{
	Q_OBJECT
	
	public:
	
		US_PlotData(QString, unsigned int *, unsigned int *, unsigned int, 
		unsigned int, bool *, double **, double *, struct mfem_data *, 
		vector <double> *, vector <double> *, QWidget *p = 0, const char *name = 0);
		~US_PlotData();
		US_Config *USglobal;
		US_ResidualPlot *resplot;
		QString run_id;

	private:

		unsigned int *scans, *points, current_cell, current_lambda;
		bool *widget_flag;
		bool raw;
		bool raw_sub_ti;
		bool raw_sub_ri;
		bool sim;
		bool sim_add_ti;
		bool sim_add_ri;
		bool resid;
		bool ti;
		bool ri;
		bool random;
		bool bitmap;

		struct mfem_data *model;
		double **absorbance, *radius;
		vector <double> *ti_noise;
		vector <double> *ri_noise;
		
		QwtPlot *data_plot, *resid_plot;

		QPushButton *pb_save;
		QPushButton *pb_cancel;

		QCheckBox *cb_raw;
		QCheckBox *cb_raw_sub_ti;
		QCheckBox *cb_raw_sub_ri;

		QCheckBox *cb_sim;
		QCheckBox *cb_sim_add_ti;
		QCheckBox *cb_sim_add_ri;

		QCheckBox *cb_resid;
		QCheckBox *cb_ti;
		QCheckBox *cb_ri;
		QCheckBox *cb_random;
		QCheckBox *cb_bitmap;

		QLabel *lbl_info;
		QLabel *lbl_variance1;
		QLabel *lbl_variance2;
		QLabel *lbl_rmsd1;
		QLabel *lbl_rmsd2;
		QLabel *lbl_raw;
		QLabel *lbl_raw_sub_ti;
		QLabel *lbl_raw_sub_ri;

		QLabel *lbl_sim;
		QLabel *lbl_sim_add_ti;
		QLabel *lbl_sim_add_ri;

		QLabel *lbl_resid;
		QLabel *lbl_ti;
		QLabel *lbl_ri;
		QLabel *lbl_random;
		QLabel *lbl_bitmap;

	private slots:

		void plot_raw();
		void plot_raw_sub_ti();
		void plot_raw_sub_ri();

		void plot_sim();
		void plot_sim_add_ti();
		void plot_sim_add_ri();

		void plot_resid();
		void plot_ti();
		void plot_ri();
		void plot_random();
		void plot_bitmap();

		void setup_GUI();
		void save();
		void cancel();

	public slots:

		void update_plot();

	protected slots:

		void closeEvent(QCloseEvent *e);

};

#endif

