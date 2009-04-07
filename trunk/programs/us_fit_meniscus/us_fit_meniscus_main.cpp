//! \file us_fit_meniscus_main.cpp

#include <QApplication>

#include "us_fit_meniscus.h"
#include "us_license_t.h"
#include "us_license.h"

//! \brief Main program for US_FitMeniscus. Loads translators and starts
//         the class US_FitMeniscus.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_FitMeniscus w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}
