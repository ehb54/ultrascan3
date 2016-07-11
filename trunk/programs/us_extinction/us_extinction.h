#ifndef US_EXTINCTION_H
#define US_EXTINCTION_H
//#include <QApplication>
//#include <QtGui>

#include "us_widgets.h"
#include "us_plot.h"
#include "us_minimize.h"
#include "us_extinctfitter.h"
#include "us_analyte_gui.h"

class US_Extinction : public US_Widgets
{
	Q_OBJECT

	public:
		US_Extinction();
		QVector <QString> filenames;
		US_ExtinctFitter *fitter;
		US_Disk_DB_Controls* disk_controls;
		US_Analyte currentAnalyte;
		QVector <double> lambda;
		QVector <double> extinction;
		float xmin, xmax;
		unsigned int order, parameters;
		double *fitparameters;
	   bool fitted, fitting_widget;

	private:
   QVector <WavelengthScan> v_wavelength;
   QVector <QwtPlotCurve*>  v_curve;
   QString projectName, filename_one, filename_two;
	unsigned int maxrange;
	float odCutoff, lambdaLimitLeft, lambdaLimitRight,lambda_min, lambda_max,
         pathlength, extinction_coefficient, factor, selected_wavelength;
	QLabel* 			lbl_gaussians;
	QLabel* 			lbl_peptide;
	QLabel*			lbl_wvinfo;
	QLabel*			lbl_associate;
	QLabel* 			lbl_cutoff;
	QLabel* 			lbl_lambda1;
	QLabel* 			lbl_lambda2;
	QLabel* 			lbl_pathlength;
	QLabel* 			lbl_coefficient;
	QListWidget*	lw_file_names;
	QPushButton* 	pb_addWavelength;
	QPushButton* 	pb_reset;
	QPushButton* 	pb_update;
	QPushButton* 	pb_perform;
	QPushButton* 	pb_calculate;
	QPushButton* 	pb_save;
	QPushButton* 	pb_view;
	QPushButton* 	pb_help;
	QPushButton* 	pb_close;
	QLineEdit*	 	le_associate;
	QLineEdit* 		le_odCutoff;
	QLineEdit* 		le_lambdaLimitLeft;
	QLineEdit* 		le_lambdaLimitRight;
	QLineEdit* 		le_pathlength;
	QLineEdit*		le_coefficient;
	QwtCounter* 	ct_gaussian;
	QwtCounter* 	ct_coefficient;
	QwtPlotCurve* 	changedCurve;
	US_Plot* 		plotLayout;
	QwtPlot* 		data_plot;
	QWidget*			p;

	private slots:
	bool 	loadScan(const QString&);
	bool 	isComment(const QString&);	
	void	add_wavelength	(void);
	void 	reading(QStringList);
	void	reset_scanlist (void);
	void 	update_data(void);
	void 	perform_global(void);
	void 	calculateE280(void);
	void 	save(void);
	void 	view_result(void);
	void 	help(void);
	void 	plot();
	void  calc_extinction();
	void	update_order(double);
	void 	listToCurve();
	bool 	deleteCurve();
	void	accessAnalyteExtinc(US_Analyte);
};
#endif	
