//! \brief Main program. Loads translators and starts
//         the class US_Convert.

#include "us_license.h"
#include "us_license_t.h"
#include "us_mwl_species_fit.h"


int main(int argc, char *argv[]) {
   QApplication application(argc, argv);

#include "main1.inc"

   // License is OK.  Start up.

   US_MwlSpeciesFit w;
   w.show(); //!< \memberof QWidget
   return application.exec(); //!< \memberof QApplication
}
