#include "../include/us_globallaser.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read())      //this is a correct file
	{
		delete us_register;
		US_GlobalLaser *global_ls;
		global_ls = new US_GlobalLaser();	
		global_ls->setCaption("Global Light Scattering:");
		global_ls->show();
		a.setMainWidget(global_ls);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
