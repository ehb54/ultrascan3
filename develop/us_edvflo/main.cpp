#include "../include/us_edvflo.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read()) //this is a correct file
	{
		delete us_register;
		EditFloVeloc_Win *edvflo;
		edvflo = new EditFloVeloc_Win();
		edvflo->setCaption("Velocity Data Editing");
		edvflo->show();
		a.setMainWidget(edvflo);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
