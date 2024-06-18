//! \file us_buffer_main.cpp

#include <QApplication>

#include "us_buffer_gui.h"
#include "us_license_t.h"
#include "us_license.h"

//! \brief Main program for US_Buffer. Loads translators and starts
//    the class US_Buffer.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );
  
   #include "main1.inc"

   // License is OK.  Start up.
   US_Buffer buf = US_Buffer();
   US_BufferGui* w = new US_BufferGui(true,buf,1);
   w->show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}
