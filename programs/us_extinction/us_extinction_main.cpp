//! \file us_extinction_main.cpp
//
#include <QApplication>
#include "us_extinction_gui.h"
#include "us_license.h"
#include "us_license_t.h"

//! \brief Main program for US_EXTINCTION. Loads translators and starts
//         the class US_EXTINCTION

int main(int argc, char *argv[]) {
   QApplication application(argc, argv);

#include "main1.inc"

   // License is OK.  Start up.

   US_Extinction w;
   w.show(); //!< \memberof QWidget
   return application.exec(); //!< \memberof QApplication
}
