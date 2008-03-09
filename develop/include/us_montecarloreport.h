#ifndef US_MONTECARLOREPORT_H
#define US_MONTECARLOREPORT_H

#include "us.h"

#include "us_util.h"
#include "us_dtactrl.h"
#include <qobject.h>
#include <qfile.h>
#include <qmessagebox.h>

class US_EXTERN US_Report_MonteCarlo : public QWidget
{
	Q_OBJECT
	
	public:
	
		US_Report_MonteCarlo(bool, QWidget *p=0, const char *name=0);
		US_Report_MonteCarlo(QString, QWidget *p=0, const char *name=0);
		~US_Report_MonteCarlo();

	private:
		US_Config *USglobal;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

		vector <QString> parameter_name;
	
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

	public slots:
		void generate(QString);	
	private slots:
		void load();
		void view(QString);
};
#endif

