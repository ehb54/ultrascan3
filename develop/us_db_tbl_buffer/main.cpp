#include "../include/us_db_tbl_buffer.h"
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
 		US_Buffer_DB *buffer_db;
	  	buffer_db = new US_Buffer_DB(false, -1);
		buffer_db->setCaption("Buffer Calculation");
		buffer_db->pb_accept->setText(" Close ");
		buffer_db->resize(500,500);
		buffer_db->show();
		a.setMainWidget(buffer_db);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}
