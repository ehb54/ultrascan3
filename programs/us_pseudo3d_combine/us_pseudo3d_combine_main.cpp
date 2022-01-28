//! \file us_pseudo3d_combine_main.cpp

#include "us_pseudo3d_combine.h"
#include "us_license_t.h"
#include "us_license.h"

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_Pseudo3D_Combine w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}
