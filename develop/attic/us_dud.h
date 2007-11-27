#ifndef US_DUD_H
#define US_DUD_H

#include "us_buffer.h"
#include "us_buffer.h"
#include "us_util.h"

#include "us_fefit.h"

class US_Dud : public QFrame
{
	Q_OBJECT
	
	public:
	
		US_Dud::US_Dud(const unsigned int temp_model, bool *temp_fitting_widget, 
		QWidget *p=0, const char *name = 0);
		~US_Dud();
	
		unsigned int parameters, iteration, *order, model, func_eval;
		unsigned long int allpoints, runs;
		unsigned int datasets, firstScan;
		unsigned int column1, column2, column3, span;
		vector <unsigned int> points_per_dataset;
		bool *fitting_widget, plotGroup, plotResiduals;
		bool suspend_flag, aborted, converged, completed, first_plot, init_simulation;

		float **raw_y, *old_variance, *variance, **concentration, runs_percent;
		float **theta;					// Parameter matrix theta columns in first dimension (parameters+1), parameter count in second dimension
		float **delta_theta;
		float **F;						// the concentration matrix containing all simulated scans for the run, 
											// for all thetas, 1st dimension: parameters, 2nd dimension: scans * points 
		float **delta_F;				// difference matrix of above
		float *alpha;
		float alphaCutoff, dInit;
		unsigned int maxLoops;
		US_Config *configuration;

		QwtPlot *data_plot;
		QwtCounter *cnt_scan;
		QLabel *lbl_header;
		QLabel *lbl_iteration1;
		QLabel *lbl_iteration2;
		QLabel *lbl_variance1;
		QLabel *lbl_variance2;
		QLabel *lbl_stddev1;
		QLabel *lbl_stddev2;
		QLabel *lbl_improvement1;
		QLabel *lbl_improvement2;
		QLabel *lbl_best1;
		QLabel *lbl_best2;
		QLabel *lbl_evaluations1;
		QLabel *lbl_evaluations2;
		QLabel *lbl_parameters1;
		QLabel *lbl_parameters2;
		QLabel *lbl_datasets1;
		QLabel *lbl_datasets2;
		QLabel *lbl_points1;
		QLabel *lbl_points2;
		QLabel *lbl_progress;
		QLabel *lbl_status1;
		QLabel *lbl_blank;
		QLabel *lbl_status2;
		QLabel *lbl_status3;
		QLabel *lbl_status4;
		QLabel *lbl_status5;
		QLabel *lbl_controls;
		QLabel *lbl_alphaCutoff;
		QLabel *lbl_maxLoops;
		QLabel *lbl_dInit;
		QLineEdit *le_alphaCutoff;
		QLineEdit *le_maxLoops;
		QLineEdit *le_dInit;
		QProgressBar *pgb_progress;
		QRadioButton *bt_plotGroup;
		QRadioButton *bt_plotAll;
		QPushButton *pb_fit;
		QPushButton *pb_pause;
		QPushButton *pb_resume;
		QPushButton *pb_cancel;
		QPushButton *pb_help;
		QPushButton *pb_report;
		QPushButton *pb_overlays;
		QPushButton *pb_residuals;
		QPushButton *pb_saveFit;
		QPushButton *pb_print;

	private slots:

		void cancel();
		void update_plotGroup();
		void update_plotAll();
		void cancel_fit();
		void dud();
		void F_init();
		void swap();
		void calc_deltas(const unsigned int);
		void shift_all();
		void order_variance();
		void update_alphaCutoff(const QString &);
		void update_maxLoops(const QString &);
		void update_dInit(const QString &);
		void print();
				
	protected slots:
	
		void closeEvent(QCloseEvent *);	
		void resizeEvent(QResizeEvent *e);

// Virtual functions:

		virtual void help();
		virtual void cleanup_dud();
		virtual void cleanup_model();
		virtual void update_simulation_parameters(const unsigned int);
		virtual bool dud_init();
		virtual void assign_F(const unsigned int);
		virtual void calc_model(const unsigned int);
		virtual void calc_residuals(const unsigned int);
		virtual bool calc_alpha();		// a default version is implemented
		virtual void calc_diff(double **);
		virtual void write_data();
		virtual void overlays();
		virtual void residuals();
		virtual void saveFit();
		virtual void updateRange(double);
		virtual void resumeFit();
		virtual void suspendFit();
		
	public slots:	
		virtual void view_report();
		virtual void update_final_fit();
		
	signals:
	
		void hasConverged();
		void parametersUpdated();
		void newParameters();
		void fitSuspended();
		void fitResumed();
		void fittingWidgetClosed();
};

#endif
