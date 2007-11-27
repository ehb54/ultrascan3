#ifndef US_RTVREQUESTSTATUS_H
#define US_RTVREQUESTSTATUS_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qfiledialog.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qlistbox.h>

#include "us_db_rtv_request.h"


class US_EXTERN US_DB_RequestStatus : public US_DB
{
	Q_OBJECT
	
	public:
		US_DB_RequestStatus(QWidget *p = 0, const char *name = "rtv_requeststatus");
		~US_DB_RequestStatus();
		
		QLabel *lbl_title;
		QLabel *lbl_s1;
		QLabel *lbl_s2;
		QLabel *lbl_s3;
		QLabel *lbl_s4;
		QLabel *lbl_s5;
		QLabel *lbl_s6;
		QLabel *lbl_s7;
		QCheckBox *cb1;
		QCheckBox *cb2;
		QCheckBox *cb3;
		QCheckBox *cb4;
		QCheckBox *cb5;
		QCheckBox *cb6;
		QCheckBox *cb7;
		QListBox *lb_status;
		QPushButton *pb_search;
		QPushButton *pb_update;
		QPushButton *pb_reset;
		QPushButton *pb_help;
		QPushButton *pb_cancel;
		bool s1, s2, s3, s4, s5, s6, s7;
		QString Str;
		int Item, RID;
		
	protected slots:
		void select_cb1();
		void select_cb2();
		void select_cb3();
		void select_cb4();
		void select_cb5();
		void select_cb6();
		void select_cb7();
		void show_status();
		void select_status(int);
		void update_status();
		void update_display(QString);
		void reset();
		void help();
		void quit();
		void resizeEvent(QResizeEvent *e);
		void closeEvent(QCloseEvent *e);
};

#endif


