#include "../include/us_equilspeed.h"
#include "../include/us_register.h"
#include <qapplication.h>

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read())      //this is a correct file
	{
		delete us_register;
	  	US_EquilSpeed *predict_dlg;
		predict_dlg = new US_EquilSpeed();
		predict_dlg->setCaption("Equilibrium Speed Prediction");
		predict_dlg->show();
		a.setMainWidget(predict_dlg);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
