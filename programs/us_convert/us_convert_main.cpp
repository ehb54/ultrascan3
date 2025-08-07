//! \file us_convert_main.cpp

#include "us_convert_gui.h"
#include "us_license.h"
#include "us_license_t.h"

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.

  // US_ConvertGui w("AUTO");
  US_ConvertGui w;
  w.show();                   //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}
