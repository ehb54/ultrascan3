#ifndef US_PSEUDOABS_H
#define US_PSEUDOABS_H

#include <qwidget.h>
#include <qframe.h>
#include <vector>
#include <qpushbt.h>
#include <qapp.h>
#include <iostream>
#include <qmsgbox.h>
#include <fstream>
#include <stdlib.h>
#include <locale.h>
#include <qtstream.h>
#include <qlistbox.h>
#include <qprogressbar.h>
#include <string.h>
#include <qfile.h>
#include <qfiledlg.h>
#include <qcombobox.h>
#include <qstrlist.h>
#include <qlayout.h>
#include <qwt_plot.h>
#include "us_util.h"
#include "us_ga_initialize.h"


extern int global_Xpos;
extern int global_Ypos;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

struct intensity_scan
{
	QString header2;
	QString filename;
	vector <float> radius, intensity[2], pseudoabs[2];
};

struct intensity_cell
{
	QString header1;
	QString path;
	unsigned int cell_number;
	vector <struct intensity_scan> scans;
};

class US_EXTERN US_PseudoAbs : public QFrame
{
	Q_OBJECT

	public:
		US_PseudoAbs( QWidget *p=0, const char *name=0 );
		~US_PseudoAbs();

	private:
		int selected_cell, selected_channel, reference_channel;
		QDir data_dir;
		bool review_channel1, review_channel2, ready_for_averaging, average_adjusted;
		int scan_count[8];
		struct vertex p1, p2;
		vector <double> average;
		vector <struct intensity_cell> icell;
		QString directory, extension, extension1, extension2;

		QLabel *absorbance;
		QLabel *interference;
		QLabel *lbl_header;
		QLabel *lbl_line1, *lbl_line2;
		QLabel *lbl_directory;
		QLabel *lbl_cell_details;
		QLabel *lbl_cell_contents;
		QLabel *lbl_progress;
		QLabel *lbl_cells1;
		QLabel *lbl_cells2;
		QLabel *lbl_scans1;
		QLabel *lbl_message;
		QLabel *lbl_first_scan1;
		QLabel *lbl_last_scan1;
		QLabel *lbl_scans2;
		QLabel *lbl_first_scan2;
		QLabel *lbl_last_scan2;
		QLabel *lbl_info1;
		QLabel *lbl_info2;
		QListBox *lb_cells;
		QProgressBar *pgb_progress;
		QPushButton *pb_select_dir;
		QPushButton *pb_convert_cell;
		QPushButton *pb_show1;
		QPushButton *pb_show2;
		QPushButton *pb_help;
		QPushButton *pb_markref;
		QPushButton *pb_quit;
		US_Config *USglobal;
		QwtPlot *abs_plot, *int_plot;

	protected slots:
		void setup_GUI();
		void closeEvent(QCloseEvent *e);
		void plotMousePressed(const QMouseEvent &);
		void plotMouseReleased(const QMouseEvent &);

	private slots:
		void select_dir();
		void convert_cell();
		void show_cell(int);
		void show_channel(int);
		void show1();
		void show2();
		void reset();
		void markref();
		void help();
		void quit();
};

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

#endif

