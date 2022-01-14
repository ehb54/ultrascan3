//! \file us_ddist_combine_main.cpp

#include "us_ddist_combine.h"
#include "us_license_t.h"
#include "us_license.h"

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_DDistr_Combine w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}


