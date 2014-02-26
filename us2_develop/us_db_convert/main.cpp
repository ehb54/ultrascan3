#include "us_db_convert.h"
#include <qapplication.h>

int main (int argc, char **argv)
{
   QApplication a(argc, argv);

   US_Convert_DB *exp_db;
   exp_db = new US_Convert_DB();
   exp_db->all_done = false;
   exp_db->setCaption("Experimental Data Table");
   exp_db->show();
   a.setMainWidget(exp_db);
   a.setDesktopSettingsAware(false);
   return a.exec();
}
