//! \file us_anapro_main.cpp

#include "us_analysis_profile.h"
#include "us_license.h"
#include "us_license_t.h"

//! \brief Main program for US_AnalysisProfileGui. Loads translators and starts
//         the class US_AnalysisProfileGui

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.

  US_AnalysisProfileGui* w = new US_AnalysisProfileGui;
  w->show();                  //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}
