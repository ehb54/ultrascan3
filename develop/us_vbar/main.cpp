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
		US_Vbar_DB *vbar_dlg;
		float vbar=.72, temp=20, vbar20=.72;
		vbar_dlg = new US_Vbar_DB(temp, &vbar, &vbar20, true,false,-1);
		vbar_dlg->setCaption("V-bar Calculation");
		vbar_dlg->pb_ok->setText(" Close ");
		vbar_dlg->show();
		a.setMainWidget(vbar_dlg);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
