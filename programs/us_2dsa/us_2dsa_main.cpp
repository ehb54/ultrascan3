//! \brief Main program. Loads translators and starts
//         the class US_2dsa.

#include "us_2dsa.h"
#include "us_license_t.h"
#include "us_license.h"


int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_2dsa w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}
