#include "../include/us_lncr2.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
   QApplication a(argc, argv);
   US_Register *us_register;
   us_register = new US_Register();

   if(us_register->read())      //this is a correct file
   {
      delete us_register;
      US_lncr2_W *lncr2_w;
      lncr2_w = new US_lncr2_W();   
      lncr2_w->setCaption("Equilibrium Analysis - ln(C) vs. r^2 Analysis");
      lncr2_w->show();
      a.setMainWidget(lncr2_w);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
