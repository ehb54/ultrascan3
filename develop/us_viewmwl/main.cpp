#include "../include/us_viewmwl.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read())      //this is a correct file
	{
		delete us_register;
		US_ViewMWL *viewmwl;
		viewmwl = new US_ViewMWL();
		viewmwl->setCaption("View Multiwavelength Data");
		viewmwl->show();
		a.setMainWidget(viewmwl);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
