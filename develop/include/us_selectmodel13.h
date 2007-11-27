#ifndef US_SELECTMODEL13_H
#define US_SELECTMODEL13_H

#include <qlabel.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qmessagebox.h>
#include "us.h"
#include "us_util.h"


class US_EXTERN US_SelectModel13 : public QDialog
{
	Q_OBJECT
	
	public:
		US_SelectModel13(float *, float *, QWidget *p=0, const char *name = 0);
		~US_SelectModel13();

		QLabel *lbl_info;
		QLabel *lbl_stoich1;
		QLineEdit *le_stoich1;
		QLabel *lbl_stoich2;
		QLineEdit *le_stoich2;
		QPushButton *pb_cancel;
		QPushButton *pb_ok;
		QPushButton *pb_help;
		US_Config *USglobal;

		float *stoich1;
		float *stoich2;
	
	public slots:
	
		void cancel();
		void check();
		void help();
		void update_stoich1(const QString &);
		void update_stoich2(const QString &);
		
	protected slots:
		void closeEvent(QCloseEvent *);
};

#endif

