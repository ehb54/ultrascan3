#include "../include/us_laser.h"
#include "../include/us_register.h"
#include <qapp.h>

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read())      //this is a correct file
	{
		delete us_register;
		US_Laser *laser;
		laser = new US_Laser();	
		laser->setCaption("US_Laser");
		laser->show();
		a.setMainWidget(laser);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
