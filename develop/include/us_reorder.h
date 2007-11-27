#ifndef US_REORDER_H
#define US_REORDER_H

#include <qwidget.h>
#include <qframe.h>
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
#include "us_util.h"


extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN US_ReOrder : public QFrame
{
	Q_OBJECT
	
	public:
		US_ReOrder( QWidget *p=0, const char *name=0 );
		~US_ReOrder();

	private:
		int selected_cell;
		QString directory, extension, extension1, extension2, flo_channel;
		QDir data_dir;
		int scan_count[8];
		int data_type; //0 = absorbance, 1 = interference, 2 = Fluorescence
		QComboBox *cmb_datatype;
		QLabel *absorbance;
		QLabel *interference;
		QLabel *lbl_header;
		QLabel *lbl_line1, *lbl_line2;
		QLabel *lbl_directory;
		QLabel *lbl_cell_details;
		QLabel *lbl_cell_contents;
		QLabel *lbl_progress;
		QLabel *lbl_lambda1;
		QLabel *lbl_lambda2;
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
		QPushButton *pb_order_cell;
		QPushButton *pb_order_all;
		QPushButton *pb_check_lambda;
		QPushButton *pb_help;
		QPushButton *pb_quit;
		US_Config *USglobal;

	protected slots:
		void setup_GUI();
		void closeEvent(QCloseEvent *e);

	private slots:
		void select_dir();
		void order_all();
		void order_cell();
		void show_cell(int);
		void update_datatype(int);
		void check_lambda();
		void reset();
		void help();
		void quit();
};


#endif

