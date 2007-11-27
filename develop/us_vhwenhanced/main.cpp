#include "../include/us_vhwenhanced.h"
#include "../include/us_register.h"


int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read()) //this is a correct file
	{
		delete us_register;
		US_vhwEnhanced *vhw_enhanced; 
		vhw_enhanced = new US_vhwEnhanced();
		vhw_enhanced->setCaption("Enhanced van Holde - Weischet Analysis:");
		vhw_enhanced->show();
		a.setMainWidget(vhw_enhanced);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
