#include "../include/us_meniscus.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
   QApplication a(argc, argv);
   US_Register *us_register;
   us_register = new US_Register();

   if(us_register->read())      //this is a correct file
   {
      delete us_register;
      US_Meniscus *meniscus_w;
      meniscus_w = new US_Meniscus();   
      meniscus_w->setCaption("UltraScan Meniscus Fitter:");
      meniscus_w->show();
      a.setMainWidget(meniscus_w);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
