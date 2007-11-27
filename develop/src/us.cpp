#include <qapplication.h>
#include <qstyle.h>
#include "../include/us_win.h"
#include "../include/us.h"
#include "../include/us_register.h"

int config=1;

int main (int argc, char **argv)
{
 int global_Xpos;
 int global_Ypos;

	global_Xpos = 50;
	global_Ypos = 50;

 	QApplication a(argc, argv);
	QTranslator translator(0);
	translator.load("../us/us_",".");
	a.installTranslator(&translator);
	
	US_Register *us_register;
	us_register = new US_Register();
	if(us_register->read())		//this is a correct license file
	{
		delete us_register;
		UsWin *w;
		w = new UsWin();
		a.setMainWidget(w);
		a.setDesktopSettingsAware(false);
		w->setCaption("UltraScan Analysis");
		w->move(global_Xpos, global_Ypos);
		w->show();
	}
	else		
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}

