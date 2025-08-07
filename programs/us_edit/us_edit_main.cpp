//! \brief Main program for US_Edit. Loads translators and starts
//!        the class US_FitMeniscus.

#include "us_edit.h"
#include "us_license.h"
#include "us_license_t.h"

int main(int argc, char *argv[]) {
   QApplication application(argc, argv);

#include "main1.inc"

   // License is OK.  Start up.

   //US_Edit w("AUTO");
   US_Edit w;
   w.show(); //!< \memberof QWidget
   return application.exec(); //!< \memberof QApplication
}
