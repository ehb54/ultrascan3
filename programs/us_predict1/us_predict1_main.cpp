//! \file us_predict1_main.cpp

#include <QApplication>

#include "us_license.h"
#include "us_license_t.h"
#include "us_predict1.h"

//! \brief Main program for US_Predict1. Loads translators and starts
//         the class US_Predict1.

int main(int argc, char *argv[]) {
   QApplication application(argc, argv);

#include "main1.inc"

   // License is OK.  Start up.

   //US_Predict1::Hydrosim params;
   US_Hydrosim params;
   US_Predict1 w(params);
   w.show(); //!< \memberof QWidget
   return application.exec(); //!< \memberof QApplication
}
