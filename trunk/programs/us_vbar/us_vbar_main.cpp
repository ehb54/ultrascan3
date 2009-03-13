//! \file us_vbar_main.cpp

#include <QApplication>

#include "us_vbar.h"
#include "us_license_t.h"
#include "us_license.h"

//! \brief Main program for US_Buffer. Loads translators and starts
//    the class US_Buffer.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );
  
   #include "main1.inc"

   // License is OK.  Start up.
   US_Vbar* w = new US_Vbar;
   w->show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}
