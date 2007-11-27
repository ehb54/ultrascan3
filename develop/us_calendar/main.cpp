#include "../include/us_calendar.h"
#include "../include/us_register.h"
#include<qapp.h>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();
	
	if(us_register->read()) //this is a correct file
        {
		delete us_register;
		US_Calendar *us_calendar;
		us_calendar = new US_Calendar();
		us_calendar->setCaption("Calendar");
		us_calendar->show();
		app.setMainWidget(us_calendar);
		app.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		app.setMainWidget(us_register);
		app.setDesktopSettingsAware(false);
	}
	return app.exec();
}

