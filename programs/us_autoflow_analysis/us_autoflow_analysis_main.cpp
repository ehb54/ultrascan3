//! \file us_autoflow_analysis_main.cpp

#include "us_autoflow_analysis.h"
#include "us_license.h"
#include "us_license_t.h"

//! \brief Main program for US_Analysis_auto. Loads translators and starts
//         the class US_Analysis_auto

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.

  US_Analysis_auto* w = new US_Analysis_auto;
  w->show();                  //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}
