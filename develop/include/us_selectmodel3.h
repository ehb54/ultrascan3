#ifndef US_SELECTMODEL3_H
#define US_SELECTMODEL3_H

#include <qlabel.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qstring.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qmessagebox.h>
#include "us.h"
#include "us_util.h"
#include "us_db_tbl_vbar.h"


class US_EXTERN US_SelectModel3 : public QDialog
{
	Q_OBJECT
	
	public:
		US_SelectModel3(float *, float *, unsigned int *, float *, bool *, int,  QWidget *p=0, const char *name = 0);
		US_SelectModel3(unsigned int *, QWidget *p=0, const char *name=0);
		~US_SelectModel3();

		QLabel *lbl_info;
		QLabel *lbl_mwSlots;
		QLabel *lbl_mwLowerLimit;
		QLabel *lbl_mwUpperLimit;
		QLabel *lbl_commonVbar;
		QLineEdit *le_mwSlots;
		QLineEdit *le_mwLowerLimit;
		QLineEdit *le_mwUpperLimit;
		QLineEdit *le_vbar;
		QPushButton *pb_cancel;
		QPushButton *pb_ok;
		QPushButton *pb_help;
		QPushButton *pb_vbar;
		QCheckBox *cb_commonVbar;
		US_Config *USglobal;
		US_Vbar_DB *vbar_dlg;

		float *model3Vbar, *mwUpperLimit, *mwLowerLimit;
		int InvID;
		unsigned int *mwSlots;
		bool *model3VbarFlag;
		int mySize_x, mySize_y;
	
	public slots:
	
		void cancel();
		void check();
		void help();
		void update_mwSlots(const QString &);
		void update_mwUpperLimit(const QString &);
		void update_mwLowerLimit(const QString &);
		void change_commonVbar();
		void update_vbar(const QString &);
		void update_vbar_lbl(float, float);
		void read_vbar();
		void resize_me();
		
	protected slots:
		void closeEvent(QCloseEvent *);
};

#endif

