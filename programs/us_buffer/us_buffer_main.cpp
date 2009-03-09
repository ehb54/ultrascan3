//! \file us_buffer_main.cpp

#include <QApplication>

#include "us_buffer.h"
#include "us_license_t.h"
#include "us_license.h"

//! \brief Main program for US_Buffer. Loads translators and starts
//    the class US_Buffer.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );
  
   #include "main1.inc"

   // License is OK.  Start up.
   US_Buffer_DB* w = new US_Buffer_DB;
   w->show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}
