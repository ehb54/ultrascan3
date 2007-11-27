#include "../include/us_sysload.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read())      //this is a correct file
	{
		delete us_register;
		SysloadWin *sys_w;
	  	sys_w = new SysloadWin;	
		sys_w->setCaption("System Load");
		sys_w->start();
		sys_w->show();
		a.setMainWidget(sys_w);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
