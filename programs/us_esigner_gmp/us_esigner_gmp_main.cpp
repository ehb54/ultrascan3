//! \file us_esigner_gmp_main.cpp

#include "us_esigner_gmp.h"
#include "us_license_t.h"
#include "us_license.h"


//! \brief Main program for US_eSigner. Loads translators and starts
//         the class 

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   //US_eSignaturesGMP * w = new US_eSignaturesGMP;
   //US_eSignaturesGMP * w = new US_eSignaturesGMP( "AUTO" );
   US_eSignaturesGMP * w = new US_eSignaturesGMP( "SEPARATE" );
   w->show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

