#include "../include/us_hydrodyn.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
   QApplication a(argc, argv);
   US_Register *us_register;
   us_register = new US_Register();

   if(us_register->read())      //this is a correct file
   {
      delete us_register;
      US_Hydrodyn *hydrodyn;
      hydrodyn = new US_Hydrodyn();
      hydrodyn->setCaption("SoMo Solution Modeler");
      hydrodyn->show();
      a.setMainWidget(hydrodyn);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
