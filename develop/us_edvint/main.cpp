#include "../include/us_edvint.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read())      //this is a correct file
	{
		delete us_register;
		EditIntVeloc_Win *edvint;
		edvint = new EditIntVeloc_Win;	
		edvint->setCaption("Edit Window for Velocity Interference Data");
		edvint->show();
		a.setMainWidget(edvint);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
