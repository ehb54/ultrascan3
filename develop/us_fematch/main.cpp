#include "../include/us_fematch.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();
	
	if(us_register->read())		//this is a correct file
	{
		delete us_register;
  		US_FeMatch_W *fematch;
		fematch = new US_FeMatch_W(); 
		fematch->setCaption("Compare Experimental Data to Sums of Finite Element Solutions");
		fematch->show();
		a.setMainWidget(fematch);
		a.setDesktopSettingsAware(false);
	}
	else		
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
