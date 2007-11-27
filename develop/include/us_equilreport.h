#ifndef US_EQUILREPORT_H
#define US_EQUILREPORT_H


#include "us.h"
#include "us_util.h"
#include "us_dtactrl.h"
#include <qobject.h>
#include <qfile.h>
#include <qmessagebox.h>

class US_EXTERN US_Report_Equil : public QWidget
{
	Q_OBJECT
	
	public:
	
		US_Report_Equil(bool flag, QWidget *p=0, const char *name=0);
		~US_Report_Equil();

	private:
		US_Config *USglobal;
		Data_Control_W *data_control;
	
		public slots:
		void generate(Data_Control_W *data_control);
		
	private slots:
		void load();
		void view(QString);
};
#endif

