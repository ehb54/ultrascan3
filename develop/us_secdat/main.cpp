#include "../include/us_smdat.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
   QApplication a(argc, argv);
   US_Register *us_register;
   us_register = new US_Register();

   if(us_register->read()) //this is a correct file
   {
      delete us_register;
      sm_dat_W *sm_w;
      sm_w = new sm_dat_W();   
      sm_w->setCaption("Second Moment Analysis:");
      sm_w->show();
      a.setMainWidget(sm_w);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
