#include "../include/us_license.h"
#include <qapplication.h>

int main (int argc, char **argv)
{
   QApplication a(argc, argv);
   US_License *license;
   license = new US_License();
   license->show();
   a.setMainWidget(license);
   a.setDesktopSettingsAware(false);
   return a.exec();
}
