#include "../include/us_sa2d.h"
#include "../include/us_register.h"


int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read()) //this is a correct file
	{
		delete us_register;
		US_SA2D_W *sa2d;
		sa2d = new US_SA2D_W();
		sa2d->setCaption("2-dimensional Spectrum Analysis:");
		sa2d->show();
		a.setMainWidget(sa2d);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
