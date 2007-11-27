#include "../include/us_finite1.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read()) //this is a correct file
	{
		delete us_register;
		US_Finite_W *finite1_w;
		finite1_w = new US_Finite_W();	
		finite1_w->setCaption("Finite Element Analysis:");
		finite1_w->show();
		a.setMainWidget(finite1_w);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
