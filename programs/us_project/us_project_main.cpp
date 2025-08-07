//! \file us_project_main.cpp

#include <QApplication>

#include "us_license.h"
#include "us_license_t.h"
#include "us_project_gui.h"

//! \brief Main program for US_Project. Loads translators and starts
//    the class US_Project.

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.
  US_ProjectGui* w = new US_ProjectGui;
  w->show();                  //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}
