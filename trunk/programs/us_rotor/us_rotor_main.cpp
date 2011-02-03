//! \file us_rotor_main.cpp

#include <QApplication>

#include "us_rotor_gui.h"
#include "us_license_t.h"
#include "us_license.h"

//! \brief Main program for US_RotorGui. Loads translators and starts
//         the class US_FitMeniscus.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_RotorGui w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}
