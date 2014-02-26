#include "../include/us_dcdrdat.h"
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
      dcdr_dat_W *dcdr_w;
      dcdr_w = new dcdr_dat_W();   
      dcdr_w->setCaption("Radial Derivative - dC/dr Analysis:");
      dcdr_w->show();
      a.setMainWidget(dcdr_w);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
