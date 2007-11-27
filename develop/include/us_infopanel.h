#ifndef US_INFOPANEL_H
#define US_INFOPANEL_H

#include <qlineedit.h>
#include <qstring.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qlayout.h>

#include "us.h"
#include "us_util.h"

class US_InfoPanel : public QFrame
{
	Q_OBJECT
	
	public:
		US_InfoPanel(int, float *, QWidget *p=0, const char *name = 0);
		~US_InfoPanel();
		US_Config *USglobal;
		
		int xpos, ypos, border, buttonh;
		float *limit;
		QListBox *lb_model;
		QLabel *lbl_model1;
		QLabel *lbl_model2;
		QLabel *lbl_max_od1;
		QLabel *lbl_max_od2;
		QLineEdit *le_max_od;
		QLabel *lbl_status1;
		QLabel *lbl_status2;

	public slots:
		void setup_GUI();
		void set_model(int);	
		void update_limit(const QString &);
		
	signals:
		void limitChanged(float);
};

#endif

