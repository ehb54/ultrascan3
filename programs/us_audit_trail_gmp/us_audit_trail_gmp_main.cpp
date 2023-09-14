//! \file us_audit_trail_gmp_main.cpp

#include "us_audit_trail_gmp.h"
#include "us_license_t.h"
#include "us_license.h"


//! \brief Main program for US_audit_trail. Loads translators and starts
//         the class 

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_auditTrailGMP * w = new US_auditTrailGMP;
   w->show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

