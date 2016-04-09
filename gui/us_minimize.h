#ifndef US_MINIMIZE_H
#define US_MINIMIZE_H

#include "us_settings.h"
#include "us_widgets.h"
#include "us_plot.h"
#include "us_util.h"
#include "us_math2.h"
#include "us_matrix.h"

class US_GUI_EXTERN US_Minimize : public US_Widgets
{
	Q_OBJECT
	
	public:

		QVector <unsigned int> points_per_dataset;	
		US_Minimize(bool& temp_fitting_widget, bool temp_GUI);
		~US_Minimize();
      unsigned int parameters, iteration, function_evaluations, decompositions;
      unsigned long int points, runs, datasets, firstScan, maxIterations;
      unsigned int column1, column2, column3, span, totalSteps;
      int nlsMethod, return_value;
      unsigned int plotGroup;

		bool *fitting_widget, plotResiduals, showGuiFit, GUI, constrained;
      bool autoconverge;
      bool suspend_flag, aborted, converged, completed, first_plot, init_simulation;
      float *y_raw;
      float lambdaStart, lambdaStep, runs_percent, variance, tolerance;
      double **LL_transpose, *B, *y_guess, **jacobian, *guess,  *test_guess,
           **information_matrix, *y_delta;
      int buttonh;

		US_Settings* settings;
		US_Matrix*	matrix;
	
      QLabel	*lbl_header;
      QLabel	*lbl_iteration;
      QLabel	*lbl_variance;
      QLabel	*lbl_stddev;
      QLabel	*lbl_improvement;
      QLabel	*lbl_tolerance;
      QLabel	*lbl_evaluations;
      QLabel	*lbl_decompositions;
      QLabel	*lbl_currentLambda;
      QLabel	*lbl_parameters;
      QLabel	*lbl_datasets;
      QLabel	*lbl_points;
      QLabel	*lbl_constrained;
		QLabel	*lbl_autoconverge;
      QLabel	*lbl_showGuiFit;
      QCheckBox	*cb_showGuiFit;
      QLabel	*lbl_progress;
      QLabel	*lbl_nlsMethod;
      QLabel	*lbl_controls1;
      QLabel	*lbl_controls2;
		QLabel	*lbl_lambdaStart;
		QLabel	*lbl_lambdaStep;
		QLabel	*lbl_maxIterations;
		QLabel	*lbl_global;
      QLineEdit	*le_iteration;
		QLineEdit	*le_variance;
		QLineEdit	*le_stddev;
		QLineEdit	*le_improvement;
		QLineEdit	*le_evaluations;
		QLineEdit	*le_decompositions;
      QLineEdit	*le_currentLambda;
		QLineEdit	*le_parameters;
      QLineEdit	*le_datasets;
		QLineEdit	*le_points;
		QLineEdit	*le_lambdaStart;
      QLineEdit	*le_maxIterations;
      QLineEdit	*le_lambdaStep;
      QLineEdit	*le_tolerance;
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
      QPushButton *pb_close;
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
      
		QComboBox 	*cbb_nlsMethod;
		QButtonGroup	*bg1;
		QButtonGroup	*bg2;
		QButtonGroup	*bg3;		
		QwtCounter	*ct_unlabeled;
		QCheckBox	*ck_monitor;
		QRadioButton	*rb_linearN;
		QRadioButton	*rb_linearY;
		QRadioButton	*rb_autoN;
		QRadioButton	*rb_autoY;
		QComboBox	*cb_nlsalg;
		US_Plot*		plotLayout;
		QwtPlot	 	*data_plot;

	protected slots:
	float calc_testParameter(float **, float);
	float linesearch(float**, float);
	int calc_B();
	void updateQN(float **, float**);
	void closeEvent(QCloseEvent *);

	public slots:
	void update_nlsMethod(int);
	void update_fitDialog();
	int Fit();

//Virtual functions:

	protected slots:
	virtual float calc_residuals();
	virtual int calc_model(double *);
	virtual void cleanup();
	virtual bool fit_init();
	virtual void plot_residuals();
	virtual void plot_overlays();
	virtual int calc_jacobian();
	virtual void endFit();

	private slots:
	void setup_GUI();
	void save_Fit();

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
