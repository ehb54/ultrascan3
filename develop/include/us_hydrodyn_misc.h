#ifndef US_HYDRODYN_MISC_H
#define US_HYDRODYN_MISC_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include "us_db_tbl_vbar.h"

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

struct misc_options
{
	double vbar;
	double hydrovol;
	bool compute_vbar;		// true = compute
		 							// false = use user specified value
};

class US_EXTERN US_Hydrodyn_Misc : public QFrame
{
	Q_OBJECT

	public:
		US_Hydrodyn_Misc(struct misc_options *, bool *, QWidget *p = 0, const char *name = 0);
		~US_Hydrodyn_Misc();

	public:
		
		struct misc_options *misc;
		bool *misc_widget;
		
		US_Config *USglobal;
		US_Vbar_DB *vbar_dlg;
		
		QLabel *lbl_info;
		QLabel *lbl_vbar;
		QLabel *lbl_hydrovol;

		QPushButton *pb_help;
		QPushButton *pb_cancel;
		QPushButton *pb_vbar;

		QLineEdit *le_vbar;
		QCheckBox *cb_vbar;
		QwtCounter *cnt_hydrovol;

	private slots:
		
		void setupGUI();
		void set_vbar();
		void select_vbar();
		void update_vbar_signal(float, float);
		void update_vbar(const QString &);
		void update_hydrovol(double);

		void cancel();
		void help();
	
	protected slots:

		void closeEvent(QCloseEvent *);

	signals:

		void vbar_changed();
};



#endif

