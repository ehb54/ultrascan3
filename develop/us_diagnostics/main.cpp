#include "../include/us_diagnostics.h"
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
      US_Diagnostics *us_diagnostics;
      us_diagnostics = new US_Diagnostics();
      us_diagnostics->show();
      a.setMainWidget(us_diagnostics);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
