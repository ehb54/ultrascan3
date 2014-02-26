#include "../include/us_edeflo.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
   QApplication a(argc, argv);
   US_Register *us_register;
   us_register = new US_Register();

   if(us_register->read())      //this is a correct file
   {
      delete us_register;
      EditFloEquil_Win *edeflo;
      edeflo = new EditFloEquil_Win;   
      edeflo->setCaption("Edit Window for Equilibrium Fluorescence Data");
      edeflo->show();
      a.setMainWidget(edeflo);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
