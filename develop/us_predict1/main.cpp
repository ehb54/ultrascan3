#include "../include/us_hydro.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read())      //this is a correct file
	{
		delete us_register;
		US_Hydro1 *hydro1_w;
	  	hydro1_w = new US_Hydro1();	
		hydro1_w->setCaption("Modeling s, D and f from MW for 4 basic shapes:");
		hydro1_w->show();
		a.setMainWidget(hydro1_w);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
