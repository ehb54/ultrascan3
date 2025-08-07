//! \file programs/us_license/us_license.cpp
//! \anchor licenseMain
#include "us_license.h"

/*! \brief Main program for us_license.  Loads translators and starts
    the class us_license
*/
int main(int argc, char *argv[]) {
   QApplication application(argc, argv);

   // Set up language localization
   QString locale = QLocale::system().name();

   QTranslator translator1;
   translator1.load(QString("us_license_") + locale);
   application.installTranslator(&translator1);

   QTranslator translator2;
   translator2.load(QString("us_gui_") + locale);
   application.installTranslator(&translator2);

   QTranslator translator3;
   translator3.load(QString("us_utils_") + locale);
   application.installTranslator(&translator3);

   US_License w;
   w.show();
   return application.exec();
}
