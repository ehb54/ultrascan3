//! \file us_analyte_main.cpp

#include <QApplication>

#include "us_analyte.h"
#include "us_license_t.h"
#include "us_license.h"

//! \brief Main program for US_Analyte. Loads translators and starts
//    the class US_Analyte.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );
  
   #include "main1.inc"

   // License is OK.  Start up.
   US_Analyte* w = new US_Analyte();
   w->show();                  //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}
