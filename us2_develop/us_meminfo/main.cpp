#include "../include/us_meminfo.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
   QApplication a(argc, argv);
   US_Register *us_register;
   us_register = new US_Register();

   if(us_register->read())      //this is a correct file
   {
      delete us_register;
      MeminfoWin *mem_w;
      mem_w = new MeminfoWin;   
      mem_w->setCaption("Memory Information");
      mem_w->start();
      mem_w->show();
      a.setMainWidget(mem_w);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
