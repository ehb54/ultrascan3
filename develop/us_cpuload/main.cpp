#include "../include/us_cpuload.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();
	
	if(us_register->read())		//this is a correct file
	{
		delete us_register;
 		CpuloadWin *cpu_w;
  		cpu_w = new CpuloadWin;	
		cpu_w->setCaption("CPU Status");
		cpu_w->show();
		a.setMainWidget(cpu_w);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
