/* database application for registration */
#include "us_check.h"
#include <qapplication.h>


int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	
	US_Check *us_check;
	us_check = new US_Check();

	us_check->show();
	a.setMainWidget(us_check);
	a.setDesktopSettingsAware(false);
	return a.exec();
}
