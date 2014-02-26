#include "../include/us_db_tbl_investigator.h"
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
      US_DB_TblInvestigator *tb_investigator;
      tb_investigator = new US_DB_TblInvestigator();
      tb_investigator->setCaption("Table of Investigator");
      tb_investigator->resize(405,460);
      tb_investigator->show();
      a.setMainWidget(tb_investigator);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
