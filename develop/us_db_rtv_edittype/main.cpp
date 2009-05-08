#include "../include/us_db_rtv_edittype.h"
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
      US_DB_RtvEditType *rtv_edittype;
      rtv_edittype = new US_DB_RtvEditType();
      rtv_edittype->show();
      a.setMainWidget(rtv_edittype);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
