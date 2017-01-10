#ifndef US_EXTINCTFITTER_GUI_H
#define US_EXTINCTFITTER_GUI_H

#include <QApplication>
#include <QtGui>
#include "us_editor.h"
#include "us_minimize.h"

//Structures needed for each file
struct Reading
{
   float lambda, od;
};

struct WavelengthScan
{
   QVector<Reading> v_readings;
   QString fileName, filePath;
   QString description;
	int pos, neg, runs;
};


class US_GUI_EXTERN US_ExtinctFitter : public US_Minimize
{
	Q_OBJECT
	
	public:

	US_ExtinctFitter(QVector <struct WavelengthScan> *, double*&, unsigned int&, unsigned int&, QString&, bool *);
	US_ExtinctFitter(QVector <struct WavelengthScan> *, double*&, unsigned int&, unsigned int&, QString&, bool *, bool);
	
	private: 
		QVector <struct WavelengthScan> *wls_v;
		QString projectName;
		US_Editor *e;
		QString htmlDir;
		unsigned int order;
	        
	
	private slots:
		void cleanup();
		int calc_jacobian();

	public slots:
		bool fit_init();
		int calc_model(double *);
		void view_report();
		void write_report();
		void plot_overlays();
		void plot_residuals();
      void updateRange(double scan);
      void endFit();
      void saveFit();
      void startFit();

   signals:

      void dataSaved(const QString &, const int);   //needed to save a list of used scans
};

#endif
