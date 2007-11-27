#ifndef US_VELOCREPORT_H
#define US_VELOCREPORT_H

#include "us.h"

#include "us_util.h"
#include "us_dtactrl.h"
#include <qobject.h>
#include <qfile.h>
#include <qmessagebox.h>

class US_EXTERN US_Report_Veloc : public QWidget
{
	Q_OBJECT
	
	public:
	
		US_Report_Veloc(bool flag, QWidget *p=0, const char *name=0);
		~US_Report_Veloc();

	private:
		Data_Control_W *data_control;
		US_Config *USglobal;
		
	public slots:
		void write_file(Data_Control_W *data_control);
	
	private slots:
		void load();
		void view(QString);		
	
};

#endif

