//! \file us_extinction.cpp
//
#include <QApplication>
#include "us_extinction_gui.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_buffer_gui.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c) setData(a,b,c)
#define setSymbol(a) setSymbol(*a)
#endif
//
//! \brief Main program for US_EXTINCTION. Loads translators and starts
//         the class US_EXTINCTION

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_Extinction w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

