#include "../include/us_db_rst_equil.h"
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
		US_DB_RST_Equil *equil_result_db;
		equil_result_db = new US_DB_RST_Equil;
		equil_result_db->setCaption("Equilibrium Experimental Data Result Table");
		equil_result_db->show();
		a.setMainWidget(equil_result_db);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
