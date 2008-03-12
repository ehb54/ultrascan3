#ifndef US_CONSTRAINT_CONTROL_H
#define US_CONSTRAINT_CONTROL_H

#include <qframe.h>
#include <qwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>

#include "us_util.h"
#include "us_femglobal.h"

class US_ConstraintControl : public QWidget
{
	Q_OBJECT
	
	public:
		
		US_ConstraintControl(QWidget *parent=0, const char *name=0);
		~US_ConstraintControl();
		struct constraint c;
		QLineEdit *le_high;
		QLineEdit *le_low;
		QCheckBox *cb_fit;
		US_Config *USglobal;
		
	public slots:

		void setDefault(float /*center*/, float /*fraction*/);
		void setEnabled(bool);
		
	private slots:

		void setHigh(const QString &);
		void setLow(const QString &);
		void setFit();

	signals:
		
		void constraintChanged(struct constraint);
};

#endif
