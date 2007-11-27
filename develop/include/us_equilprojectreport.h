#ifndef US_EQUILPROJECTREPORT_H
#define US_EQUILPROJECTREPORT_H

#include "us.h"

#include "us_util.h"
#include "us_dtactrl.h"
#include <qwidget.h>
#include <qfile.h>
#include <qmessagebox.h>

class US_EXTERN US_Report_EquilProject : public QWidget
{
	Q_OBJECT
	
	public:
	
		US_Report_EquilProject(bool flag, QWidget *p=0, const char *name=0);
		~US_Report_EquilProject();

	private:
		US_Config *USglobal;
		//QString fileName;

	public slots:
		void generate(QString);
	private slots:
		void load();
		void view(QString);
};
#endif

