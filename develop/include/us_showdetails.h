#ifndef US_SHOWDETAILS_H
#define US_SHOWDETAILS_H

#include <qpushbutton.h>
#include <vector>
#include <qstring.h>
#include <qlabel.h>
#include <qlistbox.h>
using namespace std;

#include "us.h"
#include "us_util.h"


class US_ShowDetails : public QDialog
{
	Q_OBJECT
	
	public:
		US_ShowDetails(vector <QString> *temp_filenames, int *, QWidget *p=0, const char *name = 0);
		~US_ShowDetails();
		US_Config *USglobal;
		
		int xpos, ypos, border, buttonh, *dataset_selected;
		vector <QString> *filenames;
		QListBox *lb_datasets;
		QLabel *lbl_info;
		QPushButton *pb_cancel;
		QPushButton *pb_show;
		
	public slots:
		void cancel();
		void show_dataset();
		void show_dataset(int);

	protected slots:
		void closeEvent(QCloseEvent *);
};

#endif
