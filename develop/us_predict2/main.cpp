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
		US_Hydro2 *hydro2_w;
		hydro2_w = new US_Hydro2();	
		hydro2_w->setCaption("Predict f and axial ratios for 4 basic shapes");
		hydro2_w->show();
		a.setMainWidget(hydro2_w);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
