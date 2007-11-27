#include "../include/us_db_tbl_vbar.h"
#include "../include/us_register.h"
#include <qapplication.h>

int main (int argc, char **argv)
{
	QApplication a(argc, argv);
	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read())      //this is a correct file
	{
		delete us_register;
		US_Vbar_DB *vbar_db;
		float vbar=.72, temp=20, vbar20=.72;
		bool from_cell = false;
		vbar_db = new US_Vbar_DB(temp, &vbar, &vbar20,true,from_cell, -1);
		vbar_db->setCaption("V-bar Calculation");
		vbar_db->pb_ok->setText(" Close ");
		vbar_db->show();
		a.setMainWidget(vbar_db);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
