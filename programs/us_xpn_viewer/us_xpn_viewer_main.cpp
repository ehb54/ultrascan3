//! \file us_xpn_viewer_main.cpp

#include "us_license.h"
#include "us_license_t.h"
#include "us_xpn_viewer_gui.h"

//! \brief Main program for US_XpnDataViewer.

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.

  // US_XpnDataViewer ww("AUTO");
  US_XpnDataViewer ww;
  ww.show();                  //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}
