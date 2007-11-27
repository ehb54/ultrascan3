#include "../include/us_combine.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();
	
	if(us_register->read())		//this is a correct file
	{
		delete us_register;
  		US_Combine *combine_dlg;
		combine_dlg = new US_Combine(1); //load as MW-value distribution, flag=1
		combine_dlg->setCaption("Combine Discrete Molecular Weight Distributions");
		combine_dlg->show();
		a.setMainWidget(combine_dlg);
		a.setDesktopSettingsAware(false);
	}
	else		
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
