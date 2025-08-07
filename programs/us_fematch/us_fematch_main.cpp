#include "us_fematch.h"
#include "us_license.h"
#include "us_license_t.h"

// main program
int
main (int argc, char *argv[])
{
  QApplication application (argc, argv);

#include "main1.inc"

  // License is OK.  Start up.

  US_FeMatch w;
  w.show ();                  //!< \memberof QWidget
  return application.exec (); //!< \memberof QApplication
}
