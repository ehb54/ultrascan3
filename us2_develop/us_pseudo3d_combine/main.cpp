#include "../include/us_pseudo3d_combine.h"
#include "../include/us_register.h"


int main (int argc, char **argv)
{
   QApplication a(argc, argv);
   US_Register *us_register;
   us_register = new US_Register();

   if(us_register->read()) //this is a correct file
   {
      delete us_register;
      US_Pseudo3D_Combine *pseudo3d_combine; 
      pseudo3d_combine = new US_Pseudo3D_Combine();
      pseudo3d_combine->setCaption("Combine Pseudo-3D Distribution Overlays Module:");
      pseudo3d_combine->show();
      a.setMainWidget(pseudo3d_combine);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
