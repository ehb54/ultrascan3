//! \file us_buffer_main.cpp

#include <QApplication>

#include "us_sassoc.h"
#include "us_license_t.h"
#include "us_license.h"

//! \brief Main program for US_Buffer. Loads translators and starts
//    the class US_Buffer.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );
  
   #include "main1.inc"

   // License is OK.  Start up.
   US_Sassoc w( 13.5, 35.0, 2.0, 4.0, "Simulation", 13, true );
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}
