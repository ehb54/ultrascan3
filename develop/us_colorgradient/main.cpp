#include "../include/us_colorgradient.h"
#include "../include/us_register.h"


int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read()) //this is a correct file
	{
		delete us_register;
		US_ColorGradient *colorgradient; 
		colorgradient = new US_ColorGradient();
		colorgradient->setCaption("Color Gradient Generator:");
		colorgradient->show();
		a.setMainWidget(colorgradient);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
