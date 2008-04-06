#include "../include/us_fematch_ra.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();
	
	if(us_register->read())		//this is a correct file
	{
		delete us_register;
  		US_FeMatchRa_W *fematch_ra;
		fematch_ra = new US_FeMatchRa_W(); 
		fematch_ra->setCaption("Compare Experimental Data to Sums of Finite Element Solutions");
		fematch_ra->show();
		a.setMainWidget(fematch_ra);
		a.setDesktopSettingsAware(false);
	}
	else		
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
