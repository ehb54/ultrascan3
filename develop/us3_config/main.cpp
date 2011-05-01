#include "../include/us_config_gui.h"
#include <qapplication.h>

int main (int argc, char **argv)
{
   QApplication a(argc, argv);

   US_Config_GUI *configuration;
   configuration = new US_Config_GUI();
   configuration->show();
   a.setMainWidget(configuration);
   a.setDesktopSettingsAware(false);
   return a.exec();
}
