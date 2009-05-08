#include "../include/us_cofdistro.h"
#include "../include/us_register.h"


int main (int argc, char **argv)
{
   QApplication a(argc, argv);
   US_Register *us_register;
   us_register = new US_Register();

   if(us_register->read()) //this is a correct file
   {
      delete us_register;
      US_CofDistro_W *cofdistro;
      cofdistro = new US_CofDistro_W();
      cofdistro->show();
      a.setMainWidget(cofdistro);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
