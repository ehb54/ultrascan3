#include "../include/us_archive.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read())		//this is a correct license file
	{
		delete us_register;
  		US_Archive *us_archive;
   		us_archive = new US_Archive();
		us_archive->show();
   		a.setMainWidget(us_archive);
		a.setDesktopSettingsAware(false);		
	}
	else	// problem with license (either missing, incorrect or expired)	
	{
                a.setMainWidget(us_register);
                a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
