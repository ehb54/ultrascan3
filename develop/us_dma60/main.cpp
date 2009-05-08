#include "../include/us_dma60.h"
#include "../include/us_register.h"


int main (int argc, char **argv)
{
   QApplication a(argc, argv);
   US_Register *us_register;
   us_register = new US_Register();

   if(us_register->read()) //this is a correct file
   {
      delete us_register;
      US_DMA60 *dma60; 
      dma60 = new US_DMA60();
      dma60->setCaption("Anton Paar DMA 60 Data Acquisition Module:");
      dma60->show();
      a.setMainWidget(dma60);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
