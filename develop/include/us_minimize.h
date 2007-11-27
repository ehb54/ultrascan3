#ifndef US_MINIMIZE_H
#define US_MINIMIZE_H


//QT includes:

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qprogressbar.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qapplication.h>
#include <qcheckbox.h>

// QWT includes:

#include <qwt_plot.h>
#include <qwt_counter.h>
#include <float.h>


// UltraScan includes:

#include "us_util.h"
#include "us_math.h"
#include "us_selectplot.h"
#include "us_printfilter.h"



class US_EXTERN US_Minimize : public QFrame
{
	Q_OBJECT
	
	public:
	
		US_Minimize(bool *temp_fitting_widget, bool temp_GUI, QWidget *p=0, const char *name = 0);
		~US_Minimize();
	
		unsigned int parameters, iteration, function_evaluations, decompositions;
		unsigned long int points, runs, datasets, firstScan, maxIterations;
		unsigned int column1, column2, column3, span, totalSteps;
		int nlsMethod, return_value;
		unsigned int plotGroup;
// plotgroup = 0 means plot all scans, plotGroup=1 means plot 1 scan, plotGroup=5 means plot 5
		vector <unsigned int> points_per_dataset;
		bool *fitting_widget, plotResiduals, showGuiFit, GUI, constrained;
		bool autoconverge;
		bool suspend_flag, aborted, converged, completed, first_plot, init_simulation;
		float *y_raw;
		float lambdaStart, lambdaStep, runs_percent, variance, tolerance;
		double **LL_transpose, *B, *y_guess, **jacobian, *guess,  *test_guess, **information_matrix, *y_delta;
		int buttonh;

		US_Config *USglobal;

		QwtPlot *data_plot;
		QwtCounter *cnt_scan;
		QwtCounter *cnt_scan2;
		QLabel *lbl_header;
		QLabel *lbl_iteration1;
		QLabel *lbl_iteration2;
		QLabel *lbl_variance1;
		QLabel *lbl_variance2;
		QLabel *lbl_stddev1;
		QLabel *lbl_stddev2;
		QLabel *lbl_improvement1;
		QLabel *lbl_improvement2;
		QLabel *lbl_tolerance;
		QLabel *lbl_evaluations1;
		QLabel *lbl_evaluations2;
		QLabel *lbl_decompositions1;
		QLabel *lbl_decompositions2;
		QLabel *lbl_currentLambda1;
		QLabel *lbl_currentLambda2;
		QLabel *lbl_parameters1;
		QLabel *lbl_parameters2;
		QLabel *lbl_datasets1;
		QLabel *lbl_datasets2;
		QLabel *lbl_points1;
		QLabel *lbl_points2;
		QLabel *lbl_constrained;
		QLabel *lbl_autoconverge;
		QLabel *lbl_showGuiFit;
		QCheckBox *cb_showGuiFit;
		QLabel *lbl_progress;
		QLabel *lbl_status1;
		QLabel *lbl_status2;
		QLabel *lbl_status3;
		QLabel *lbl_status4;
		QLabel *lbl_nlsMethod;
		QLabel *lbl_controls1;
		QLabel *lbl_controls2;
		QLabel *lbl_lambdaStart;
		QLabel *lbl_maxIterations;
		QLabel *lbl_lambdaStep;
		QLineEdit *le_lambdaStart;
		QLineEdit *le_maxIterations;
		QLineEdit *le_lambdaStep;
		QLineEdit *le_tolerance;
		QProgressBar *pgb_progress;
		QRadioButton *bt_plotGroup;
		QRadioButton *bt_plotSingle;
		QRadioButton *bt_plotAll;
		QRadioButton *bt_constrained;
		QRadioButton *bt_unconstrained;
		QRadioButton *bt_autoconverge;
		QRadioButton *bt_manualconverge;
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
		QPushButton *pb_plottwo;
		QPushButton *pb_plotthree;
		QPushButton *pb_plotfour;
		QPushButton *pb_plotfive;
		QComboBox *cbb_nlsMethod;

				
		
	protected slots:
	
		void setup_GUI();
		void setup_GUI2();
		void cancelFit();
		void update_plotGroup();
		void update_plotAll();
		void update_plotSingle();
		void update_constrained();
		void update_unconstrained();
		void update_autoconverge();
		void update_manualconverge();
		void update_maxIterations(const QString &);
		void update_lambdaStart(const QString &);
		void update_lambdaStep(const QString &);
		void update_tolerance(const QString &);
		void change_showGuiFit();
		void closeEvent(QCloseEvent *);	
//		void resizeEvent(QResizeEvent *e);
		void resumeFit();
		void suspendFit();
		void help();
		int calc_B();
		float calc_testParameter(float **, float);
		float linesearch(float **, float);
		void updateQN(float **, float**);

	public slots:
	
		void update_nlsMethod(int);
		int Fit();
		void update_fitDialog();
		void cancel();
		void setGUI(bool);
		
// Virtual functions:

	protected slots:
	
		virtual float calc_residuals();
		virtual bool fit_init();
		virtual void cleanup();
		virtual int calc_model(double *);
		virtual void write_data();
		virtual void plot_overlays();
		virtual void plot_residuals();
		virtual void plot_two();
		virtual void plot_three();
		virtual void plot_four();
		virtual void plot_five();
		virtual void saveFit();
		virtual void print();
		virtual void updateRange(double);
		virtual int calc_jacobian();
		virtual void endFit();
		virtual bool try_adjustment();
				
	public slots:

		virtual void view_report();
		virtual void startFit();
		
	signals:
	
		void hasConverged();
		void currentStatus(const QString &);
		void parametersUpdated();
		void newParameters();
		void fitSuspended();
		void fitAborted();
		void fitResumed();
		void fitStarted();
		void fittingWidgetClosed();
};

#endif

