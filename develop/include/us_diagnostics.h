#ifndef US_DIAGNOSTICS_H
#define US_DIAGNOSTICS_H

#include <qlabel.h>
#include <string.h>
#include <qfont.h>
#include <qfiledialog.h>
#include <qwt_plot.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qlayout.h>
#include "us_util.h"


extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN US_Diagnostics : public QFrame
{
	Q_OBJECT

	private:
		QString cell_info, scanmode, cell, temperature, time;
		bool converted;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

		vector <float> rad, abs;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

		QwtPlot *delta_plot;
		QwtPlot *scan_plot;
		US_Config *USglobal;
		QPushButton *pb_load;
		QPushButton *pb_print_delta;
		QPushButton *pb_print_scan;
		QPushButton *pb_convert;
		QPushButton *pb_help;
		QPushButton *pb_close;
		QLabel *banner1;
		QLabel *lbl_file1;
		QLabel *lbl_file2;
		QLabel *lbl_filesize1;
		QLabel *lbl_filesize2;
		QLabel *lbl_cell1;
		QLabel *lbl_cell2;
		QLabel *lbl_scanmode1;
		QLabel *lbl_scanmode2;
		QLabel *lbl_temperature1;
		QLabel *lbl_temperature2;
		QLabel *lbl_rpm1;
		QLabel *lbl_rpm2;
		QLabel *lbl_time1;
		QLabel *lbl_time2;
		QLabel *lbl_omega1;
		QLabel *lbl_omega2;
		QLabel *lbl_lambda1;
		QLabel *lbl_lambda2;
		QLabel *lbl_repetitions1;
		QLabel *lbl_repetitions2;
		QLabel *lbl_datapoints1;
		QLabel *lbl_datapoints2;
		QLabel *lbl_pt_density1;
		QLabel *lbl_pt_density2;

	public:	
		US_Diagnostics(QWidget *p = 0, const char *name = 0);
		~US_Diagnostics();

	public slots:
		void load();
		void load(const QString &);
		void quit();
		void print_delta();
		void print_scan();
		void plot_graph();
		void convert();
		void help();

	protected slots:
		void setup_GUI();
//		void resizeEvent(QResizeEvent *e);
		void closeEvent(QCloseEvent *e);
};

#endif

