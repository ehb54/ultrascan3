//! \file us_analyte_main.cpp

#include <QApplication>

#include "us_analyte_gui.h"
#include "us_license.h"
#include "us_license_t.h"

//! \brief Main program for US_AnalyteGui. Loads translators and starts
//    the class US_AnalyteGui.

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.
  US_AnalyteGui* w = new US_AnalyteGui();
  w->show();                  //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}
