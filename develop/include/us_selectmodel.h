#ifndef US_SELECTMODEL_H
#define US_SELECTMODEL_H

#include <qlabel.h>
#include <qlistbox.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qstring.h>
#include <vector>
#include "us.h"
#include "us_util.h"


extern vector <QString> modelString;

class US_EXTERN US_SelectModel : public QDialog
{
	Q_OBJECT
	
	public:
		US_SelectModel(int *, bool, QWidget *p=0, const char *name = 0);
		~US_SelectModel();
		US_Config *USglobal;
		
		int xpos, ypos, border, buttonh, *model_selected;
		bool show_equation;
		QListBox *lb_model;
		QLabel *lbl_info;
		QPushButton *pb_cancel;
		QPushButton *pb_select;
		QPushButton *pb_help;
		
	public slots:
		void cancel();
		void help();
		void select_model();
		void select_model(int);
		void setup_GUI();

	protected slots:
		void closeEvent(QCloseEvent *);
};

#endif

