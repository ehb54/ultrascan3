//! \file us_manage_analytes.cpp

#include <QApplication>

#include "us_analyte_manager.h"
#include "us_license_t.h"
#include "us_license.h"

//! \brief Main program for US_AnalyteManager. Loads translators and starts
//    the class US_AnalyteManager.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );
  
   #include "main1.inc"

   // License is OK.  Start up.
   US_AnalyteManager* w = new US_AnalyteManager();
   w->show();                  //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}
