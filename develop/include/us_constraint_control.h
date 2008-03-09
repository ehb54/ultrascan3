#ifndef US_CONSTRAINT_CONTROL_H
#define US_CONSTRAINT_CONTROL_H

#include <qframe.h>
#include <qwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>

#include "us_util.h"

class US_ConstraintControl : public QWidget
{
	Q_OBJECT
	
	public:
		
		US_ConstraintControl(float /*center*/, float * /*low*/, float * /*high*/, bool * /*fit*/, QWidget *parent=0, const char *name=0);
		~US_ConstraintControl();
		float *low, *high, center;
		bool *fit;
		QLineEdit *le_high;
		QLineEdit *le_low;
		QCheckBox *cb_fit;
		US_Config *USglobal;
		
	public slots:

		void set_default(float);
		
	private slots:

		void set_high(const QString &);
		void set_low(const QString &);
		void set_fit();
};

#endif
