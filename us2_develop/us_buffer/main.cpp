#include "../include/us_db_tbl_buffer.h"
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
      US_Buffer_DB *buffer_dlg;
      buffer_dlg = new US_Buffer_DB(false, -1);
      buffer_dlg->setCaption("Buffer Calculation");
      buffer_dlg->pb_accept->setText(" Close ");
      buffer_dlg->show();
      a.setMainWidget(buffer_dlg);
      a.setDesktopSettingsAware(false);
   }
   else
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
